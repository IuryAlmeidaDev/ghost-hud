#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>

#include "hardware.h"
#include "ui.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_HOTKEY_F9            1
#define ID_HOTKEY_CTRL_F9       2
#define ID_HOTKEY_CTRL_SHIFT_F9 3
#define ID_HOTKEY_F8            4
#define ID_HOTKEY_CTRL_F8       5
#define ID_TIMER_TICK           1

#define IDM_TOGGLE_LOCK  101
#define IDM_TOGGLE_SIZE  102
#define IDM_RESET_POS    103
#define IDM_QUIT         104

// Definições nativas do Shell NotifyIcon (dispensa shellapi.h)
#define NIM_ADD         0x00000000
#define NIM_MODIFY      0x00000001
#define NIM_DELETE      0x00000002

#define NIF_MESSAGE     0x00000001
#define NIF_ICON        0x00000002
#define NIF_TIP         0x00000004

#ifndef LWA_ALPHA
#define LWA_ALPHA 0x00000002
#endif

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

typedef struct _NOTIFYICONDATAW {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    WCHAR szTip[128];
} NOTIFYICONDATAW, *PNOTIFYICONDATAW;

typedef BOOL (WINAPI *pfn_Shell_NotifyIconW)(DWORD dwMessage, PNOTIFYICONDATAW lpData);
typedef BOOL (WINAPI *pfn_SetLayeredWindowAttributes)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

static pfn_Shell_NotifyIconW fn_Shell_NotifyIconW = NULL;
static pfn_SetLayeredWindowAttributes fn_SetLayeredWindowAttributes = NULL;
static HMODULE hShell32 = NULL;

static HWND g_hWnd = NULL;
static BOOL g_locked = FALSE;
static BOOL g_minimized = FALSE;
static HardwareMetrics g_metrics = {0};
static NOTIFYICONDATAW g_nid = {0};
static char g_iniPath[MAX_PATH] = {0};

static void get_ini_path(void) {
    GetModuleFileNameA(NULL, g_iniPath, MAX_PATH);
    char *lastSlash = strrchr(g_iniPath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
        strcat(g_iniPath, "ghosthud.ini");
    } else {
        strcpy(g_iniPath, "ghosthud.ini");
    }
}

static void save_config(int x, int y, BOOL locked, BOOL minimized) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", x);
    WritePrivateProfileStringA("HUD", "x", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", y);
    WritePrivateProfileStringA("HUD", "y", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", locked ? 1 : 0);
    WritePrivateProfileStringA("HUD", "locked", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", minimized ? 1 : 0);
    WritePrivateProfileStringA("HUD", "minimized", buf, g_iniPath);
}

static void update_tray_tip(void) {
    if (fn_Shell_NotifyIconW && g_nid.hWnd) {
        if (g_locked) {
            wcscpy(g_nid.szTip, L"GhostHUD - Modo Jogo (Travado) [F9 / F8]");
        } else {
            wcscpy(g_nid.szTip, L"GhostHUD - Modo Ajuste (Arrastável) [F9 / F8]");
        }
        fn_Shell_NotifyIconW(NIM_MODIFY, &g_nid);
    }
}

static void set_window_locked(HWND hWnd, BOOL locked) {
    g_locked = locked;
    LONG_PTR style = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
    if (g_locked) {
        style |= WS_EX_TRANSPARENT;
    } else {
        style &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLongPtrW(hWnd, GWL_EXSTYLE, style);

    RECT r;
    GetWindowRect(hWnd, &r);
    save_config(r.left, r.top, g_locked, g_minimized);

    update_tray_tip();
    InvalidateRect(hWnd, NULL, TRUE);
}

static void set_window_minimized(HWND hWnd, BOOL min) {
    g_minimized = min;
    int target_w = g_minimized ? HUD_MINI_WIDTH : HUD_WIDTH;
    int target_h = g_minimized ? HUD_MINI_HEIGHT : HUD_HEIGHT;

    RECT r;
    GetWindowRect(hWnd, &r);
    SetWindowPos(hWnd, HWND_TOPMOST, r.left, r.top, target_w, target_h, SWP_SHOWWINDOW);

    save_config(r.left, r.top, g_locked, g_minimized);
    InvalidateRect(hWnd, NULL, TRUE);
}

static void show_tray_menu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, IDM_TOGGLE_LOCK, 
                g_locked ? L"Desbloquear p/ Mover (Modo Ajuste) [F9]" : L"Bloquear Cliques (Modo Jogo) [F9]");
    InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_STRING, IDM_TOGGLE_SIZE, 
                g_minimized ? L"Maximizar HUD (Normal) [F8]" : L"Minimizar HUD (Quadradinho) [F8]");
    InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_STRING, IDM_RESET_POS, L"Resetar Posição");
    InsertMenuW(hMenu, 3, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenuW(hMenu, 4, MF_BYPOSITION | MF_STRING, IDM_QUIT, L"Sair");

    SetForegroundWindow(hWnd);
    TrackPopupMenuEx(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, hWnd, NULL);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            hardware_init();
            ui_init();
            hardware_update(&g_metrics);
            SetTimer(hWnd, ID_TIMER_TICK, 1000, NULL);
            return 0;
        }

        // Permite arrastar clicando em qualquer ponto do HUD no Modo Ajuste
        case WM_NCHITTEST: {
            if (!g_locked) {
                return HTCAPTION;
            }
            return HTTRANSPARENT;
        }

        case WM_SETCURSOR: {
            if (!g_locked) {
                SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                return TRUE;
            }
            return DefWindowProcW(hWnd, msg, wParam, lParam);
        }

        case WM_HOTKEY: {
            if (wParam == ID_HOTKEY_F9 || wParam == ID_HOTKEY_CTRL_F9 || wParam == ID_HOTKEY_CTRL_SHIFT_F9) {
                set_window_locked(hWnd, !g_locked);
            } else if (wParam == ID_HOTKEY_F8 || wParam == ID_HOTKEY_CTRL_F8) {
                set_window_minimized(hWnd, !g_minimized);
            }
            return 0;
        }

        case WM_TIMER: {
            if (wParam == ID_TIMER_TICK) {
                hardware_update(&g_metrics);
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            GetClientRect(hWnd, &rc);
            ui_render(hdc, rc.right - rc.left, rc.bottom - rc.top, &g_metrics, g_locked, g_minimized);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_LBUTTONDBLCLK: {
            if (!g_locked) {
                set_window_minimized(hWnd, !g_minimized);
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            if (!g_locked) {
                ReleaseCapture();
                SendMessageW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
            return 0;
        }

        case WM_MOVE: {
            RECT r;
            GetWindowRect(hWnd, &r);
            save_config(r.left, r.top, g_locked, g_minimized);
            return 0;
        }

        case WM_TRAYICON: {
            if (lParam == WM_RBUTTONUP) {
                show_tray_menu(hWnd);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                set_window_minimized(hWnd, !g_minimized);
            } else if (lParam == WM_LBUTTONUP) {
                set_window_locked(hWnd, !g_locked);
            }
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDM_TOGGLE_LOCK:
                    set_window_locked(hWnd, !g_locked);
                    break;
                case IDM_TOGGLE_SIZE:
                    set_window_minimized(hWnd, !g_minimized);
                    break;
                case IDM_RESET_POS: {
                    int w = g_minimized ? HUD_MINI_WIDTH : HUD_WIDTH;
                    int h = g_minimized ? HUD_MINI_HEIGHT : HUD_HEIGHT;
                    SetWindowPos(hWnd, HWND_TOPMOST, 120, 80, w, h, SWP_SHOWWINDOW);
                    save_config(120, 80, g_locked, g_minimized);
                    break;
                }
                case IDM_QUIT:
                    DestroyWindow(hWnd);
                    break;
            }
            return 0;
        }

        case WM_DESTROY: {
            KillTimer(hWnd, ID_TIMER_TICK);
            UnregisterHotKey(hWnd, ID_HOTKEY_F9);
            UnregisterHotKey(hWnd, ID_HOTKEY_CTRL_F9);
            UnregisterHotKey(hWnd, ID_HOTKEY_CTRL_SHIFT_F9);
            UnregisterHotKey(hWnd, ID_HOTKEY_F8);
            UnregisterHotKey(hWnd, ID_HOTKEY_CTRL_F8);
            if (fn_Shell_NotifyIconW) {
                fn_Shell_NotifyIconW(NIM_DELETE, &g_nid);
            }
            if (hShell32) {
                FreeLibrary(hShell32);
                hShell32 = NULL;
            }
            hardware_cleanup();
            ui_cleanup();
            PostQuitMessage(0);
            return 0;
        }

        default:
            return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!hInstance) {
        hInstance = GetModuleHandleW(NULL);
    }

    get_ini_path();

    // Carrega User32 e Shell32 dinamicamente
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        fn_SetLayeredWindowAttributes = (pfn_SetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    }

    hShell32 = LoadLibraryA("shell32.dll");
    if (hShell32) {
        fn_Shell_NotifyIconW = (pfn_Shell_NotifyIconW)GetProcAddress(hShell32, "Shell_NotifyIconW");
    }

    // Configurações salvas
    int start_x = GetPrivateProfileIntA("HUD", "x", 120, g_iniPath);
    int start_y = GetPrivateProfileIntA("HUD", "y", 80, g_iniPath);
    g_locked = GetPrivateProfileIntA("HUD", "locked", 0, g_iniPath);
    g_minimized = GetPrivateProfileIntA("HUD", "minimized", 0, g_iniPath);

    // Registro da classe da janela (com CS_DBLCLKS para detectar clique duplo)
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"GhostHudWindowClass";
    RegisterClassExW(&wc);

    // Estilos de janela: Sem bordas (WS_POPUP), Topmost, Layered, Fora do Alt+Tab
    DWORD exStyle = WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
    if (g_locked) {
        exStyle |= WS_EX_TRANSPARENT;
    }

    int init_w = g_minimized ? HUD_MINI_WIDTH : HUD_WIDTH;
    int init_h = g_minimized ? HUD_MINI_HEIGHT : HUD_HEIGHT;

    g_hWnd = CreateWindowExW(
        exStyle,
        wc.lpszClassName,
        L"GhostHUD",
        WS_POPUP,
        start_x, start_y, init_w, init_h,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd) {
        DWORD err = GetLastError();
        char buf[128];
        snprintf(buf, sizeof(buf), "Falha ao inicializar janela GhostHUD (Erro: %lu)", err);
        MessageBoxA(NULL, buf, "GhostHUD", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Aplica opacidade translúcida de 90% (230 / 255)
    if (fn_SetLayeredWindowAttributes) {
        fn_SetLayeredWindowAttributes(g_hWnd, 0, 230, LWA_ALPHA);
    }

    // Registra atalhos globais F9 (Travar/Destravar) e F8 (Minimizar/Maximizar)
    RegisterHotKey(g_hWnd, ID_HOTKEY_F9, 0, VK_F9);
    RegisterHotKey(g_hWnd, ID_HOTKEY_CTRL_F9, MOD_CONTROL, VK_F9);
    RegisterHotKey(g_hWnd, ID_HOTKEY_CTRL_SHIFT_F9, MOD_CONTROL | MOD_SHIFT, VK_F9);

    RegisterHotKey(g_hWnd, ID_HOTKEY_F8, 0, VK_F8);
    RegisterHotKey(g_hWnd, ID_HOTKEY_CTRL_F8, MOD_CONTROL, VK_F8);

    // Registra ícone na bandeja do sistema
    if (fn_Shell_NotifyIconW) {
        ZeroMemory(&g_nid, sizeof(g_nid));
        g_nid.cbSize = sizeof(NOTIFYICONDATAW);
        g_nid.hWnd = g_hWnd;
        g_nid.uID = 1;
        g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        g_nid.uCallbackMessage = WM_TRAYICON;
        g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        update_tray_tip();
        fn_Shell_NotifyIconW(NIM_ADD, &g_nid);
    }

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    // Loop de mensagens Win32 tradicional (0% CPU)
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

// Ponto de entrada padrão
int main(int argc, char **argv) {
    return WinMain(GetModuleHandleW(NULL), NULL, GetCommandLineA(), SW_SHOW);
}
