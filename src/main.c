#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>

#include "hardware.h"
#include "ui.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TIMER_TICK           1

#define IDM_TOGGLE_CPU      201
#define IDM_TOGGLE_GPU      202
#define IDM_TOGGLE_VRAM     203
#define IDM_TOGGLE_RAM      204
#define IDM_TOGGLE_PING     205
#define IDM_TOGGLE_CLOCK    206

#define IDM_SIZE_SMALL      210
#define IDM_SIZE_MEDIUM     211
#define IDM_SIZE_LARGE      212

#define IDM_AUTOSTART       220
#define IDM_RESET_POS       221
#define IDM_QUIT            222

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

// Registry APIs dinâmicas para autostart (compatível com TCC sem advapi32.def)
typedef LONG (WINAPI *pfn_RegOpenKeyExA)(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
typedef LONG (WINAPI *pfn_RegSetValueExA)(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData);
typedef LONG (WINAPI *pfn_RegDeleteValueA)(HKEY hKey, LPCSTR lpValueName);
typedef LONG (WINAPI *pfn_RegQueryValueExA)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
typedef LONG (WINAPI *pfn_RegCloseKey)(HKEY hKey);

static pfn_Shell_NotifyIconW fn_Shell_NotifyIconW = NULL;
static pfn_SetLayeredWindowAttributes fn_SetLayeredWindowAttributes = NULL;
static HMODULE hShell32 = NULL;

static pfn_RegOpenKeyExA fn_RegOpenKeyExA = NULL;
static pfn_RegSetValueExA fn_RegSetValueExA = NULL;
static pfn_RegDeleteValueA fn_RegDeleteValueA = NULL;
static pfn_RegQueryValueExA fn_RegQueryValueExA = NULL;
static pfn_RegCloseKey fn_RegCloseKey = NULL;
static HMODULE hAdvapi32 = NULL;

static HWND g_hWnd = NULL;
static HudConfig g_config = {
    .show_cpu = TRUE,
    .show_gpu = TRUE,
    .show_vram = TRUE,
    .show_ram = TRUE,
    .show_ping = TRUE,
    .show_clock = TRUE,
    .size = HUD_SIZE_SMALL,
    .alpha = 230
};

static HardwareMetrics g_metrics = {0};
static NOTIFYICONDATAW g_nid = {0};
static char g_iniPath[MAX_PATH] = {0};

static BOOL g_isDragging = FALSE;
static POINT g_dragStartMouse = {0};
static POINT g_dragStartWin = {0};

static void init_advapi32(void) {
    if (!hAdvapi32) {
        hAdvapi32 = LoadLibraryA("advapi32.dll");
        if (hAdvapi32) {
            fn_RegOpenKeyExA = (pfn_RegOpenKeyExA)GetProcAddress(hAdvapi32, "RegOpenKeyExA");
            fn_RegSetValueExA = (pfn_RegSetValueExA)GetProcAddress(hAdvapi32, "RegSetValueExA");
            fn_RegDeleteValueA = (pfn_RegDeleteValueA)GetProcAddress(hAdvapi32, "RegDeleteValueA");
            fn_RegQueryValueExA = (pfn_RegQueryValueExA)GetProcAddress(hAdvapi32, "RegQueryValueExA");
            fn_RegCloseKey = (pfn_RegCloseKey)GetProcAddress(hAdvapi32, "RegCloseKey");
        }
    }
}

static BOOL is_autostart_enabled(void) {
    init_advapi32();
    if (!fn_RegOpenKeyExA || !fn_RegQueryValueExA || !fn_RegCloseKey) return FALSE;
    HKEY hKey;
    if (fn_RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD type = 0;
        DWORD cbData = 0;
        LONG res = fn_RegQueryValueExA(hKey, "GhostHUD", NULL, &type, NULL, &cbData);
        fn_RegCloseKey(hKey);
        return (res == ERROR_SUCCESS);
    }
    return FALSE;
}

static void toggle_autostart(void) {
    init_advapi32();
    if (!fn_RegOpenKeyExA || !fn_RegCloseKey) return;
    HKEY hKey;
    if (fn_RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE | KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (is_autostart_enabled()) {
            if (fn_RegDeleteValueA) fn_RegDeleteValueA(hKey, "GhostHUD");
        } else {
            char exePath[MAX_PATH];
            GetModuleFileNameA(NULL, exePath, MAX_PATH);
            if (fn_RegSetValueExA) {
                fn_RegSetValueExA(hKey, "GhostHUD", 0, REG_SZ, (const BYTE *)exePath, (DWORD)(strlen(exePath) + 1));
            }
        }
        fn_RegCloseKey(hKey);
    }
}

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

static void load_config(HudConfig *cfg) {
    cfg->show_cpu = GetPrivateProfileIntA("HUD", "show_cpu", 1, g_iniPath);
    cfg->show_gpu = GetPrivateProfileIntA("HUD", "show_gpu", 1, g_iniPath);
    cfg->show_vram = GetPrivateProfileIntA("HUD", "show_vram", 1, g_iniPath);
    cfg->show_ram = GetPrivateProfileIntA("HUD", "show_ram", 1, g_iniPath);
    cfg->show_ping = GetPrivateProfileIntA("HUD", "show_ping", 1, g_iniPath);
    cfg->show_clock = GetPrivateProfileIntA("HUD", "show_clock", 1, g_iniPath);
    cfg->size = (HudSize)GetPrivateProfileIntA("HUD", "size", HUD_SIZE_SMALL, g_iniPath);
    if (cfg->size < 0 || cfg->size > 2) cfg->size = HUD_SIZE_SMALL;
    cfg->alpha = GetPrivateProfileIntA("HUD", "alpha", 230, g_iniPath);
    if (cfg->alpha < 100) cfg->alpha = 100;
    if (cfg->alpha > 255) cfg->alpha = 255;
}

static void save_config(const HudConfig *cfg, int x, int y) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", x);
    WritePrivateProfileStringA("HUD", "x", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", y);
    WritePrivateProfileStringA("HUD", "y", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->show_cpu ? 1 : 0);
    WritePrivateProfileStringA("HUD", "show_cpu", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->show_gpu ? 1 : 0);
    WritePrivateProfileStringA("HUD", "show_gpu", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->show_vram ? 1 : 0);
    WritePrivateProfileStringA("HUD", "show_vram", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->show_ram ? 1 : 0);
    WritePrivateProfileStringA("HUD", "show_ram", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->show_ping ? 1 : 0);
    WritePrivateProfileStringA("HUD", "show_ping", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->show_clock ? 1 : 0);
    WritePrivateProfileStringA("HUD", "show_clock", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->size);
    WritePrivateProfileStringA("HUD", "size", buf, g_iniPath);
    snprintf(buf, sizeof(buf), "%d", cfg->alpha);
    WritePrivateProfileStringA("HUD", "alpha", buf, g_iniPath);
}

static void update_tray_tip(void) {
    if (fn_Shell_NotifyIconW && g_nid.hWnd) {
        wcscpy(g_nid.szTip, L"GhostHUD - Monitor Gamer MMORPG");
        fn_Shell_NotifyIconW(NIM_MODIFY, &g_nid);
    }
}

static void apply_layout_and_save(HWND hWnd) {
    int w, h, hh;
    ui_get_window_size(&g_config, &w, &h, &hh);

    RECT r;
    GetWindowRect(hWnd, &r);
    SetWindowPos(hWnd, HWND_TOPMOST, r.left, r.top, w, h, SWP_SHOWWINDOW);

    HRGN hRgn = CreateRoundRectRgn(0, 0, w + 1, h + 1, 3, 3);
    SetWindowRgn(hWnd, hRgn, TRUE);

    save_config(&g_config, r.left, r.top);
    InvalidateRect(hWnd, NULL, TRUE);
}

static void show_settings_menu(HWND hWnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();

    // 1. Módulos Visíveis (Opções diretas com checkmarks)
    InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING | (g_config.show_cpu ? MF_CHECKED : MF_UNCHECKED), 
                IDM_TOGGLE_CPU, L"CPU");
    InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_STRING | (g_config.show_gpu ? MF_CHECKED : MF_UNCHECKED), 
                IDM_TOGGLE_GPU, L"GPU (Temperatura)");
    InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_STRING | (g_config.show_vram ? MF_CHECKED : MF_UNCHECKED), 
                IDM_TOGGLE_VRAM, L"VRAM (Mem\u00F3ria de V\u00EDdeo)");
    InsertMenuW(hMenu, 3, MF_BYPOSITION | MF_STRING | (g_config.show_ram ? MF_CHECKED : MF_UNCHECKED), 
                IDM_TOGGLE_RAM, L"RAM");
    InsertMenuW(hMenu, 4, MF_BYPOSITION | MF_STRING | (g_config.show_ping ? MF_CHECKED : MF_UNCHECKED), 
                IDM_TOGGLE_PING, L"Ping (Lat\u00EAncia)");
    InsertMenuW(hMenu, 5, MF_BYPOSITION | MF_STRING | (g_config.show_clock ? MF_CHECKED : MF_UNCHECKED), 
                IDM_TOGGLE_CLOCK, L"Rel\u00F3gio no Cabe\u00E7alho");

    InsertMenuW(hMenu, 6, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

    // 2. Submenu de Seleção de Tamanho
    HMENU hSizeSub = CreatePopupMenu();
    AppendMenuW(hSizeSub, MF_STRING | (g_config.size == HUD_SIZE_SMALL ? MF_CHECKED : MF_UNCHECKED), 
                IDM_SIZE_SMALL, L"Pequeno (Compacto)");
    AppendMenuW(hSizeSub, MF_STRING | (g_config.size == HUD_SIZE_MEDIUM ? MF_CHECKED : MF_UNCHECKED), 
                IDM_SIZE_MEDIUM, L"M\u00E9dio");
    AppendMenuW(hSizeSub, MF_STRING | (g_config.size == HUD_SIZE_LARGE ? MF_CHECKED : MF_UNCHECKED), 
                IDM_SIZE_LARGE, L"Grande");
    InsertMenuW(hMenu, 7, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSizeSub, L"Tamanho do HUD");

    InsertMenuW(hMenu, 8, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenuW(hMenu, 9, MF_BYPOSITION | MF_STRING | (is_autostart_enabled() ? MF_CHECKED : MF_UNCHECKED), 
                IDM_AUTOSTART, L"Iniciar com o Windows");
    InsertMenuW(hMenu, 10, MF_BYPOSITION | MF_STRING, IDM_RESET_POS, L"Resetar Posi\u00E7\u00E3o");
    InsertMenuW(hMenu, 11, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenuW(hMenu, 12, MF_BYPOSITION | MF_STRING, IDM_QUIT, L"Sair");

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

        case WM_NCHITTEST: {
            return HTCLIENT;
        }

        case WM_SETCURSOR: {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);

            RECT r;
            GetWindowRect(hWnd, &r);
            int win_w = r.right - r.left;
            int cur_w, cur_h, cur_header_h;
            ui_get_window_size(&g_config, &cur_w, &cur_h, &cur_header_h);

            if (UI_IS_GEAR_BTN(pt.x, pt.y, win_w, cur_header_h)) {
                SetCursor(LoadCursor(NULL, IDC_HAND));
            } else {
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            }
            return TRUE;
        }

        case WM_MOUSEWHEEL: {
            short zDelta = (short)HIWORD(wParam);
            if (zDelta > 0) {
                g_config.alpha += 15;
                if (g_config.alpha > 255) g_config.alpha = 255;
            } else if (zDelta < 0) {
                g_config.alpha -= 15;
                if (g_config.alpha < 100) g_config.alpha = 100;
            }
            if (fn_SetLayeredWindowAttributes) {
                fn_SetLayeredWindowAttributes(hWnd, 0, (BYTE)g_config.alpha, LWA_ALPHA);
            }
            RECT r;
            GetWindowRect(hWnd, &r);
            save_config(&g_config, r.left, r.top);
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
            ui_render(hdc, rc.right - rc.left, rc.bottom - rc.top, &g_metrics, &g_config);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_RBUTTONUP: {
            show_settings_menu(hWnd);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int mouseX = (short)LOWORD(lParam);
            int mouseY = (short)HIWORD(lParam);

            RECT r;
            GetWindowRect(hWnd, &r);
            int win_w = r.right - r.left;
            int cur_w, cur_h, cur_header_h;
            ui_get_window_size(&g_config, &cur_w, &cur_h, &cur_header_h);

            // Clique na Engrenagem [⚙] abre o menu de personalização
            if (UI_IS_GEAR_BTN(mouseX, mouseY, win_w, cur_header_h)) {
                show_settings_menu(hWnd);
                return 0;
            }

            // Inicia arraste imediato e suave
            g_isDragging = TRUE;
            SetCapture(hWnd);
            GetCursorPos(&g_dragStartMouse);
            g_dragStartWin.x = r.left;
            g_dragStartWin.y = r.top;
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (g_isDragging) {
                POINT pt;
                GetCursorPos(&pt);
                int nx = g_dragStartWin.x + (pt.x - g_dragStartMouse.x);
                int ny = g_dragStartWin.y + (pt.y - g_dragStartMouse.y);
                SetWindowPos(hWnd, HWND_TOPMOST, nx, ny, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            if (g_isDragging) {
                g_isDragging = FALSE;
                ReleaseCapture();
                RECT r;
                GetWindowRect(hWnd, &r);
                save_config(&g_config, r.left, r.top);
            }
            return 0;
        }

        case WM_MOVE: {
            RECT r;
            GetWindowRect(hWnd, &r);
            save_config(&g_config, r.left, r.top);
            return 0;
        }

        case WM_TRAYICON: {
            if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
                show_settings_menu(hWnd);
            }
            return 0;
        }

        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDM_TOGGLE_CPU:
                    g_config.show_cpu = !g_config.show_cpu;
                    apply_layout_and_save(hWnd);
                    break;
                case IDM_TOGGLE_GPU:
                    g_config.show_gpu = !g_config.show_gpu;
                    apply_layout_and_save(hWnd);
                    break;
                case IDM_TOGGLE_VRAM:
                    g_config.show_vram = !g_config.show_vram;
                    apply_layout_and_save(hWnd);
                    break;
                case IDM_TOGGLE_RAM:
                    g_config.show_ram = !g_config.show_ram;
                    apply_layout_and_save(hWnd);
                    break;
                case IDM_TOGGLE_PING:
                    g_config.show_ping = !g_config.show_ping;
                    apply_layout_and_save(hWnd);
                    break;
                case IDM_TOGGLE_CLOCK:
                    g_config.show_clock = !g_config.show_clock;
                    apply_layout_and_save(hWnd);
                    break;

                case IDM_SIZE_SMALL:
                    g_config.size = HUD_SIZE_SMALL;
                    apply_layout_and_save(hWnd);
                    break;
                case IDM_SIZE_MEDIUM:
                    g_config.size = HUD_SIZE_MEDIUM;
                    apply_layout_and_save(hWnd);
                    break;
                case IDM_SIZE_LARGE:
                    g_config.size = HUD_SIZE_LARGE;
                    apply_layout_and_save(hWnd);
                    break;

                case IDM_AUTOSTART:
                    toggle_autostart();
                    break;

                case IDM_RESET_POS: {
                    int w, h, hh;
                    ui_get_window_size(&g_config, &w, &h, &hh);
                    SetWindowPos(hWnd, HWND_TOPMOST, 120, 80, w, h, SWP_SHOWWINDOW);
                    HRGN hRgn = CreateRoundRectRgn(0, 0, w + 1, h + 1, 3, 3);
                    SetWindowRgn(hWnd, hRgn, TRUE);
                    save_config(&g_config, 120, 80);
                    InvalidateRect(hWnd, NULL, TRUE);
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
            if (fn_Shell_NotifyIconW) {
                fn_Shell_NotifyIconW(NIM_DELETE, &g_nid);
            }
            if (hShell32) {
                FreeLibrary(hShell32);
                hShell32 = NULL;
            }
            if (hAdvapi32) {
                FreeLibrary(hAdvapi32);
                hAdvapi32 = NULL;
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
    load_config(&g_config);

    // Carrega User32 e Shell32 dinamicamente
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        fn_SetLayeredWindowAttributes = (pfn_SetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    }

    hShell32 = LoadLibraryA("shell32.dll");
    if (hShell32) {
        fn_Shell_NotifyIconW = (pfn_Shell_NotifyIconW)GetProcAddress(hShell32, "Shell_NotifyIconW");
    }

    // Configurações salvas de coordenadas
    int start_x = GetPrivateProfileIntA("HUD", "x", 120, g_iniPath);
    int start_y = GetPrivateProfileIntA("HUD", "y", 80, g_iniPath);

    // Registro da classe da janela
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"GhostHudWindowClass";
    RegisterClassExW(&wc);

    // Estilos de janela: Sem bordas (WS_POPUP), Topmost, Layered, Fora do Alt+Tab
    DWORD exStyle = WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW;

    int init_w, init_h, init_header_h;
    ui_get_window_size(&g_config, &init_w, &init_h, &init_header_h);

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

    HRGN hRgn = CreateRoundRectRgn(0, 0, init_w + 1, init_h + 1, 3, 3);
    SetWindowRgn(g_hWnd, hRgn, TRUE);

    // Aplica opacidade translúcida carregada do INI
    if (fn_SetLayeredWindowAttributes) {
        fn_SetLayeredWindowAttributes(g_hWnd, 0, (BYTE)g_config.alpha, LWA_ALPHA);
    }

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
