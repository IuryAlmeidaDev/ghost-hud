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
#define ID_TIMER_TICK           1

#define IDM_TOGGLE_LOCK  101
#define IDM_RESET_POS    102
#define IDM_QUIT         103

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

static void save_config(int x, int y, BOOL locked) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", x);
    WritePrivateProfileStringA("HUD", "x", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", y);
    WritePrivateProfileStringA("HUD", "y", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", locked ? 1 : 0);
    WritePrivateProfileStringA("HUD", "locked", buf, g_iniPath);
}

static void update_tray_tip(void) {
    if (fn_Shell_NotifyIconW && g_nid.hWnd) {
        if (g_locked) {
            wcscpy(g_nid.szTip, L"GhostHUD - Modo Jogo (Travado) [F9]");
        } else {
            wcscpy(g_nid.szTip, L"GhostHUD - Modo Ajuste (Arrastável) [F9]");
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
    save_config(r.left, r.top, g_locked);

    update_tray_tip();
    InvalidateRect(hWnd, NULL, TRUE);
}

static void show_tray_menu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, IDM_TOGGLE_LOCK, 
                g_locked ? L"Desbloquear p/ Mover (Modo Ajuste) [F9]" : L"Bloquear Cliques (Modo Jogo) [F9]");
    InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_STRING, IDM_RESET_POS, L"Resetar Posição");
    InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenuW(hMenu, 3, MF_BYPOSITION | MF_STRING, IDM_QUIT, L"Sair");

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
            ui_render(hdc, rc.right - rc.left, rc.bottom - rc.top, &g_metrics, g_locked);
            EndPaint(hWnd, &ps);
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
            save_config(r.left, r.top, g_locked);
            return 0;
        }

        case WM_TRAYICON: {
            if (lParam == WM_RBUTTONUP) {
                show_tray_menu(hWnd);
            } else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONUP) {
                set_window_locked(hWnd, !g_locked);
            }
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDM_TOGGLE_LOCK:
                    set_window_locked(hWnd, !g_locked);
                    break;
                case IDM_RESET_POS:
                    SetWindowPos(hWnd, HWND_TOPMOST, 120, 80, HUD_WIDTH, HUD_HEIGHT, SWP_SHOWWINDOW);
                    save_config(120, 80, g_locked);
                    break;
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

    // Configurações salvas (Inicia em Modo Ajuste = 0 por padrão se for primeira vez para facilitar posicionamento)
    int start_x = GetPrivateProfileIntA("HUD", "x", 120, g_iniPath);
    int start_y = GetPrivateProfileIntA("HUD", "y", 80, g_iniPath);
    g_locked = GetPrivateProfileIntA("HUD", "locked", 0, g_iniPath);

    // Registro da classe da janela
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
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

    g_hWnd = CreateWindowExW(
        exStyle,
        wc.lpszClassName,
        L"GhostHUD",
        WS_POPUP,
        start_x, start_y, HUD_WIDTH, HUD_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd) {
        return 1;
    }

    // Aplica opacidade translúcida de 88% (225 / 255)
    if (fn_SetLayeredWindowAttributes) {
        fn_SetLayeredWindowAttributes(g_hWnd, 0, 225, LWA_ALPHA);
    }

    // Registra atalhos globais de alta compatibilidade
    RegisterHotKey(g_hWnd, ID_HOTKEY_F9, 0, VK_F9);
    RegisterHotKey(g_hWnd, ID_HOTKEY_CTRL_F9, MOD_CONTROL, VK_F9);
    RegisterHotKey(g_hWnd, ID_HOTKEY_CTRL_SHIFT_F9, MOD_CONTROL | MOD_SHIFT, VK_F9);

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
