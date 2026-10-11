#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

#include "hardware.h"
#include "ui.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_HOTKEY_F9 1
#define ID_TIMER_TICK 1

#define IDM_TOGGLE_LOCK  101
#define IDM_RESET_POS    102
#define IDM_QUIT         103

static HWND g_hWnd = NULL;
static BOOL g_locked = TRUE;
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

    InvalidateRect(hWnd, NULL, TRUE);
}

static void show_tray_menu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, IDM_TOGGLE_LOCK, 
                g_locked ? L"Modo Ajuste (Destravar arrasto) [F9]" : L"Modo Jogo (Travar click-through) [F9]");
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

        case WM_HOTKEY: {
            if (wParam == ID_HOTKEY_F9) {
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
            } else if (lParam == WM_LBUTTONDBLCLK) {
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
            Shell_NotifyIconW(NIM_DELETE, &g_nid);
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

    // Configurações salvas
    int start_x = GetPrivateProfileIntA("HUD", "x", 120, g_iniPath);
    int start_y = GetPrivateProfileIntA("HUD", "y", 80, g_iniPath);
    g_locked = GetPrivateProfileIntA("HUD", "locked", 1, g_iniPath);

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
    SetLayeredWindowAttributes(g_hWnd, 0, 225, LWA_ALPHA);

    // Registra HotKey global F9
    RegisterHotKey(g_hWnd, ID_HOTKEY_F9, 0, VK_F9);

    // Registra ícone na bandeja do sistema
    ZeroMemory(&g_nid, sizeof(g_nid));
    g_nid.cbSize = sizeof(NOTIFYICONDATAW);
    g_nid.hWnd = g_hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy(g_nid.szTip, L"GhostHUD - Monitor Gamer (F9 para alternar)");
    Shell_NotifyIconW(NIM_ADD, &g_nid);

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
