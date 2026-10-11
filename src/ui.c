#include "ui.h"
#include <stdio.h>

static HFONT hFontTitle[3] = {NULL, NULL, NULL};
static HFONT hFontLabel[3] = {NULL, NULL, NULL};
static HFONT hFontValue[3] = {NULL, NULL, NULL};

// Paleta base e térmica
#define COLOR_DIVIDER     RGB(28, 34, 44)      // Separador horizontal sutil (#1C222C)
#define COLOR_TRACK       RGB(14, 17, 23)      // Trilha escura rebaixada das barras (#0E1117)
#define COLOR_BORDER_HOT  RGB(255, 71, 87)     // Borda de alerta térmico para GPU quente (#FF4757)

// Escala de Cores Vibrante do HUD do Jogo:
#define COLOR_GREEN       RGB(46, 213, 115)    // 🟢 Verde esmeralda (#2ED573)
#define COLOR_YELLOW      RGB(255, 190, 118)   // 🟡 Ouro suave (#FFBE76)
#define COLOR_ORANGE      RGB(255, 121, 63)    // 🟠 Laranja quente (#FF793F)
#define COLOR_RED         RGB(255, 71, 87)     // 🔴 Vermelho carmesim (#FF4757)

typedef struct {
    COLORREF bg;
    COLORREF header_bg;
    COLORREF slot_bg;
    COLORREF slot_border;
    COLORREF border;
    COLORREF title;
    COLORREF label;
    COLORREF time;
} ThemeColors;

static ThemeColors get_theme_colors(HudTheme theme) {
    ThemeColors tc;
    switch (theme) {
        case HUD_THEME_BLOODMOON:
            tc.bg          = RGB(18, 11, 14);
            tc.header_bg   = RGB(26, 14, 19);
            tc.slot_bg     = RGB(32, 17, 23);
            tc.slot_border = RGB(56, 25, 33);
            tc.border      = RGB(96, 32, 45);
            tc.title       = RGB(250, 205, 215);
            tc.label       = RGB(180, 115, 128);
            tc.time        = RGB(210, 145, 158);
            break;

        case HUD_THEME_CYBERNEON:
            tc.bg          = RGB(9, 13, 21);
            tc.header_bg   = RGB(13, 19, 31);
            tc.slot_bg     = RGB(17, 26, 42);
            tc.slot_border = RGB(28, 48, 76);
            tc.border      = RGB(0, 190, 225);
            tc.title       = RGB(0, 240, 255);
            tc.label       = RGB(105, 155, 198);
            tc.time        = RGB(140, 190, 230);
            break;

        case HUD_THEME_ROYALGOLD:
            tc.bg          = RGB(17, 16, 13);
            tc.header_bg   = RGB(25, 23, 17);
            tc.slot_bg     = RGB(31, 28, 20);
            tc.slot_border = RGB(62, 54, 34);
            tc.border      = RGB(195, 160, 75);
            tc.title       = RGB(250, 230, 175);
            tc.label       = RGB(170, 150, 110);
            tc.time        = RGB(215, 190, 140);
            break;

        case HUD_THEME_OBSIDIAN:
        default:
            tc.bg          = RGB(15, 17, 21);
            tc.header_bg   = RGB(19, 23, 30);
            tc.slot_bg     = RGB(22, 27, 36);
            tc.slot_border = RGB(34, 42, 54);
            tc.border      = RGB(42, 50, 64);
            tc.title       = RGB(220, 226, 236);
            tc.label       = RGB(124, 138, 156);
            tc.time        = RGB(148, 162, 180);
            break;
    }
    return tc;
}

static COLORREF get_status_color(int usage_pct, int temp_c) {
    int score = usage_pct;
    if (temp_c >= 83) {
        if (score < 85) score = 85;
    } else if (temp_c >= 75) {
        if (score < 73) score = 73;
    } else if (temp_c >= 66) {
        if (score < 56) score = 56;
    }

    if (score >= 85) return COLOR_RED;
    if (score >= 72) return COLOR_ORANGE;
    if (score >= 55) return COLOR_YELLOW;
    return COLOR_GREEN;
}

static COLORREF get_ping_color(int ms) {
    if (ms <= 0) return RGB(124, 138, 156);
    if (ms <= 45) return COLOR_GREEN;
    if (ms <= 80) return COLOR_YELLOW;
    if (ms <= 130) return COLOR_ORANGE;
    return COLOR_RED;
}

static COLORREF get_power_color(int watts) {
    if (watts <= 140) return COLOR_GREEN;
    if (watts <= 210) return COLOR_YELLOW;
    if (watts <= 265) return COLOR_ORANGE;
    return COLOR_RED;
}

void ui_init(void) {
    // 0: Pequeno (Compacto)
    hFontTitle[0] = CreateFontA(-8, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontLabel[0] = CreateFontA(-8, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontValue[0] = CreateFontA(-11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    // 1: Médio
    hFontTitle[1] = CreateFontA(-10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontLabel[1] = CreateFontA(-9, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontValue[1] = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    // 2: Grande
    hFontTitle[2] = CreateFontA(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontLabel[2] = CreateFontA(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    hFontValue[2] = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
}

void ui_cleanup(void) {
    for (int i = 0; i < 3; i++) {
        if (hFontTitle[i]) { DeleteObject(hFontTitle[i]); hFontTitle[i] = NULL; }
        if (hFontLabel[i]) { DeleteObject(hFontLabel[i]); hFontLabel[i] = NULL; }
        if (hFontValue[i]) { DeleteObject(hFontValue[i]); hFontValue[i] = NULL; }
    }
}

void ui_get_window_size(const HudConfig *cfg, int *out_width, int *out_height, int *out_header_h) {
    if (!cfg) return;

    int active_slots = 0;
    if (cfg->show_cpu) active_slots++;
    if (cfg->show_gpu) active_slots++;
    if (cfg->show_vram) active_slots++;
    if (cfg->show_ram) active_slots++;
    if (cfg->show_ping) active_slots++;
    if (cfg->show_power) active_slots++;
    if (cfg->show_counter) active_slots++;

    int slot_w, slot_h, slot_gap, header_h, pad_x, pad_y;
    int min_hdr_w;

    switch (cfg->size) {
        case HUD_SIZE_MEDIUM:
            slot_w = 56;
            slot_h = 33;
            slot_gap = 4;
            header_h = 16;
            pad_x = 4;
            pad_y = 4;
            min_hdr_w = cfg->show_clock ? 160 : 95;
            break;
        case HUD_SIZE_LARGE:
            slot_w = 68;
            slot_h = 40;
            slot_gap = 5;
            header_h = 19;
            pad_x = 5;
            pad_y = 5;
            min_hdr_w = cfg->show_clock ? 190 : 115;
            break;
        case HUD_SIZE_SMALL:
        default:
            slot_w = 44;
            slot_h = 26;
            slot_gap = 3;
            header_h = 13;
            pad_x = 3;
            pad_y = 3;
            min_hdr_w = cfg->show_clock ? 130 : 80;
            break;
    }

    int final_w, final_h;
    if (cfg->orientation == HUD_ORIENT_VERTICAL) {
        final_w = (pad_x * 2) + slot_w;
        if (final_w < 56) final_w = 56;
        final_h = header_h + (pad_y * 2) + (active_slots * slot_h) + ((active_slots > 1 ? active_slots - 1 : 0) * slot_gap);
    } else {
        int total_slots_w = (pad_x * 2) + (active_slots * slot_w) + ((active_slots > 1 ? active_slots - 1 : 0) * slot_gap);
        final_w = (total_slots_w > min_hdr_w) ? total_slots_w : min_hdr_w;
        final_h = (active_slots > 0) ? (header_h + slot_h + pad_y + 2) : (header_h + 3);
    }

    if (out_width) *out_width = final_w;
    if (out_height) *out_height = final_h;
    if (out_header_h) *out_header_h = header_h;
}

static void draw_gauge_bar(HDC hdc, int x, int y, int width, int height, int pct, COLORREF color) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    HBRUSH hBrushTrack = CreateSolidBrush(COLOR_TRACK);
    RECT rTrack = { x, y, x + width, y + height };
    FillRect(hdc, &rTrack, hBrushTrack);
    DeleteObject(hBrushTrack);

    int fill_w = (width * pct) / 100;
    if (fill_w > 0) {
        HBRUSH hBrushFill = CreateSolidBrush(color);
        RECT rFill = { x, y, x + fill_w, y + height };
        FillRect(hdc, &rFill, hBrushFill);
        DeleteObject(hBrushFill);
    }
}

static void draw_gear_icon(HDC hdc, int cx, int cy, int r, COLORREF color, COLORREF hdr_bg) {
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, hPen);

    // 8 dentes radiantes
    MoveToEx(hdc, cx - r, cy, NULL); LineTo(hdc, cx + r + 1, cy);
    MoveToEx(hdc, cx, cy - r, NULL); LineTo(hdc, cx + r + 1, cy);
    int d = (r * 7) / 10;
    MoveToEx(hdc, cx - d, cy - d, NULL); LineTo(hdc, cx + d + 1, cy + d + 1);
    MoveToEx(hdc, cx - d, cy + d, NULL); LineTo(hdc, cx + d + 1, cy - d - 1);

    // Anel / furo central da engrenagem
    HBRUSH hBrushHdr = CreateSolidBrush(hdr_bg);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrushHdr);
    int inner_r = (r > 4) ? 3 : 2;
    Ellipse(hdc, cx - inner_r, cy - inner_r, cx + inner_r + 1, cy + inner_r + 1);

    SelectObject(hdc, oldBrush);
    DeleteObject(hBrushHdr);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
}

static void draw_metric_slot(HDC memDC, int x, int y, int w, int h, 
                             const char *label, const char *value, int pct, COLORREF val_color, 
                             HudSize size, const ThemeColors *tc) {
    int s_idx = (size >= 0 && size <= 2) ? size : 0;

    // 1. Fundo e contorno do slot
    HBRUSH hBrushSlot = CreateSolidBrush(tc->slot_bg);
    HPEN hPenSlot = CreatePen(PS_SOLID, 1, tc->slot_border);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrushSlot);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenSlot);

    RoundRect(memDC, x, y, x + w, y + h, 2, 2);

    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(hBrushSlot);
    DeleteObject(hPenSlot);

    // 2. Rótulo superior
    int lbl_h = (s_idx == 0) ? 9 : ((s_idx == 1) ? 11 : 13);
    RECT rLbl = { x, y + 1, x + w, y + 1 + lbl_h };
    SelectObject(memDC, hFontLabel[s_idx]);
    SetTextColor(memDC, tc->label);
    DrawTextA(memDC, label, -1, &rLbl, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // 3. Valor principal
    int val_y = y + lbl_h + 1;
    int bar_h = (s_idx == 0) ? 2 : ((s_idx == 1) ? 3 : 4);
    int val_bottom = y + h - bar_h - 2;
    RECT rVal = { x, val_y, x + w, val_bottom };
    SelectObject(memDC, hFontValue[s_idx]);
    SetTextColor(memDC, val_color);
    DrawTextA(memDC, value, -1, &rVal, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // 4. Barra de progresso no rodapé do slot
    int g_w = w - 6;
    int g_x = x + 3;
    int g_y = y + h - bar_h - 1;
    draw_gauge_bar(memDC, g_x, g_y, g_w, bar_h, pct, val_color);
}

void ui_render(HDC hdc, int width, int height, const HardwareMetrics *metrics, const HudConfig *config) {
    if (!config) return;

    HudSize size = (config->size >= 0 && config->size <= 2) ? config->size : HUD_SIZE_SMALL;
    int s_idx = size;
    ThemeColors tc = get_theme_colors(config->theme);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 1. Fundo geral do Tema
    HBRUSH hBrushBg = CreateSolidBrush(tc.bg);
    RECT rClient = { 0, 0, width, height };
    FillRect(memDC, &rClient, hBrushBg);
    DeleteObject(hBrushBg);

    int pad_x, pad_y, slot_gap, header_h;
    switch (size) {
        case HUD_SIZE_MEDIUM:
            pad_x = 4; pad_y = 4; slot_gap = 4; header_h = 16;
            break;
        case HUD_SIZE_LARGE:
            pad_x = 5; pad_y = 5; slot_gap = 5; header_h = 19;
            break;
        case HUD_SIZE_SMALL:
        default:
            pad_x = 3; pad_y = 3; slot_gap = 3; header_h = 13;
            break;
    }

    // 2. Cabeçalho Integrado
    RECT rHdr = { 2, 2, width - 2, header_h };
    HBRUSH hBrushHdr = CreateSolidBrush(tc.header_bg);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle[s_idx]);

    // Título
    SetTextColor(memDC, tc.title);
    if (config->orientation == HUD_ORIENT_VERTICAL) {
        RECT rTitle = { pad_x + 1, 1, width - 20, header_h };
        DrawTextA(memDC, "HUD", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    } else {
        int title_w = (s_idx == 0) ? 58 : ((s_idx == 1) ? 72 : 88);
        RECT rTitle = { pad_x + 2, 1, pad_x + 2 + title_w, header_h };
        DrawTextA(memDC, "GHOST HUD", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

        if (config->show_clock) {
            SYSTEMTIME st;
            GetLocalTime(&st);
            char timeBuf[32];
            snprintf(timeBuf, sizeof(timeBuf), "\x95  %02d:%02d", st.wHour, st.wMinute);
            SetTextColor(memDC, tc.time);
            RECT rClock = { pad_x + 2 + title_w + 4, 1, width - 22, header_h };
            DrawTextA(memDC, timeBuf, -1, &rClock, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }
    }

    // Ícone da Engrenagem [⚙]
    int gear_r = (s_idx == 0) ? 4 : ((s_idx == 1) ? 5 : 6);
    int gear_cx = width - 11;
    int gear_cy = header_h / 2 + 1;
    draw_gear_icon(memDC, gear_cx, gear_cy, gear_r, tc.title, tc.header_bg);

    // Divisória horizontal sutil
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, header_h, NULL);
    LineTo(memDC, width - 2, header_h);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    // 3. Renderização dos Módulos (Horizontal ou Vertical)
    int active_slots = 0;
    if (config->show_cpu) active_slots++;
    if (config->show_gpu) active_slots++;
    if (config->show_vram) active_slots++;
    if (config->show_ram) active_slots++;
    if (config->show_ping) active_slots++;
    if (config->show_power) active_slots++;
    if (config->show_counter) active_slots++;

    if (active_slots > 0) {
        char buf[32];

        if (config->orientation == HUD_ORIENT_VERTICAL) {
            // Empilhamento Vertical
            int slot_w = width - (pad_x * 2);
            int slot_h = (size == HUD_SIZE_SMALL) ? 26 : ((size == HUD_SIZE_MEDIUM) ? 33 : 40);
            int cur_y = header_h + pad_y;

            #define DRAW_SLOT_V(label, val_str, pct, val_col) do { \
                draw_metric_slot(memDC, pad_x, cur_y, slot_w, slot_h, label, val_str, pct, val_col, size, &tc); \
                cur_y += slot_h + slot_gap; \
            } while(0)

            if (config->show_cpu) {
                int cpu = metrics ? metrics->cpu_usage : 0;
                snprintf(buf, sizeof(buf), "%d%%", cpu);
                DRAW_SLOT_V("CPU", buf, cpu, get_status_color(cpu, 0));
            }
            if (config->show_gpu) {
                int gpu = metrics ? metrics->gpu_usage : 0;
                int temp = metrics ? metrics->gpu_temp : 0;
                snprintf(buf, sizeof(buf), "%d\xB0" "C", temp);
                DRAW_SLOT_V("GPU", buf, gpu, get_status_color(gpu, temp));
            }
            if (config->show_vram) {
                int vram = metrics ? metrics->vram_usage : 0;
                snprintf(buf, sizeof(buf), "%d%%", vram);
                DRAW_SLOT_V("VRAM", buf, vram, get_status_color(vram, 0));
            }
            if (config->show_ram) {
                int ram = metrics ? metrics->ram_usage : 0;
                snprintf(buf, sizeof(buf), "%d%%", ram);
                DRAW_SLOT_V("RAM", buf, ram, get_status_color(ram, 0));
            }
            if (config->show_ping) {
                int ping = metrics ? metrics->ping_ms : 0;
                if (ping > 0) snprintf(buf, sizeof(buf), "%dms", ping);
                else snprintf(buf, sizeof(buf), "--");
                DRAW_SLOT_V("PING", buf, ping > 150 ? 100 : (ping * 100) / 150, get_ping_color(ping));
            }
            if (config->show_power) {
                int watts = metrics ? metrics->gpu_power_w : 0;
                snprintf(buf, sizeof(buf), "%dW", watts);
                DRAW_SLOT_V("PWR", buf, (watts * 100) / 290, get_power_color(watts));
            }
            if (config->show_counter) {
                const char *lbl = (config->counter_label_type == 1) ? "KILLS" : 
                                  ((config->counter_label_type == 2) ? "BOSS" : 
                                  ((config->counter_label_type == 3) ? "RUNS" : "FARM"));
                snprintf(buf, sizeof(buf), "%d", config->counter_val);
                DRAW_SLOT_V(lbl, buf, (config->counter_val % 100), COLOR_YELLOW);
            }
            #undef DRAW_SLOT_V

        } else {
            // Linha Horizontal
            int slot_y = header_h + 2;
            int slot_h = height - slot_y - pad_y;
            int total_gaps = (active_slots - 1) * slot_gap;
            int available_w = width - (pad_x * 2) - total_gaps;
            int slot_w = available_w / active_slots;
            int extra_pixels = available_w % active_slots;

            int cur_x = pad_x;

            #define DRAW_SLOT_H(label, val_str, pct, val_col) do { \
                int w_this = slot_w + (extra_pixels-- > 0 ? 1 : 0); \
                draw_metric_slot(memDC, cur_x, slot_y, w_this, slot_h, label, val_str, pct, val_col, size, &tc); \
                cur_x += w_this + slot_gap; \
            } while(0)

            if (config->show_cpu) {
                int cpu = metrics ? metrics->cpu_usage : 0;
                snprintf(buf, sizeof(buf), "%d%%", cpu);
                DRAW_SLOT_H("CPU", buf, cpu, get_status_color(cpu, 0));
            }
            if (config->show_gpu) {
                int gpu = metrics ? metrics->gpu_usage : 0;
                int temp = metrics ? metrics->gpu_temp : 0;
                snprintf(buf, sizeof(buf), "%d\xB0" "C", temp);
                DRAW_SLOT_H("GPU", buf, gpu, get_status_color(gpu, temp));
            }
            if (config->show_vram) {
                int vram = metrics ? metrics->vram_usage : 0;
                snprintf(buf, sizeof(buf), "%d%%", vram);
                DRAW_SLOT_H("VRAM", buf, vram, get_status_color(vram, 0));
            }
            if (config->show_ram) {
                int ram = metrics ? metrics->ram_usage : 0;
                snprintf(buf, sizeof(buf), "%d%%", ram);
                DRAW_SLOT_H("RAM", buf, ram, get_status_color(ram, 0));
            }
            if (config->show_ping) {
                int ping = metrics ? metrics->ping_ms : 0;
                if (ping > 0) snprintf(buf, sizeof(buf), "%dms", ping);
                else snprintf(buf, sizeof(buf), "--");
                DRAW_SLOT_H("PING", buf, ping > 150 ? 100 : (ping * 100) / 150, get_ping_color(ping));
            }
            if (config->show_power) {
                int watts = metrics ? metrics->gpu_power_w : 0;
                snprintf(buf, sizeof(buf), "%dW", watts);
                DRAW_SLOT_H("PWR", buf, (watts * 100) / 290, get_power_color(watts));
            }
            if (config->show_counter) {
                const char *lbl = (config->counter_label_type == 1) ? "KILLS" : 
                                  ((config->counter_label_type == 2) ? "BOSS" : 
                                  ((config->counter_label_type == 3) ? "RUNS" : "FARM"));
                snprintf(buf, sizeof(buf), "%d", config->counter_val);
                DRAW_SLOT_H(lbl, buf, (config->counter_val % 100), COLOR_YELLOW);
            }
            #undef DRAW_SLOT_H
        }
    }

    // 4. Borda Externa (com Alerta Térmico se GPU >= 82°C)
    COLORREF borderColor = tc.border;
    if (metrics && metrics->gpu_temp >= 82) {
        borderColor = COLOR_BORDER_HOT;
    }
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, borderColor);
    HPEN oldPenOuter = (HPEN)SelectObject(memDC, hPenBorder);
    HBRUSH oldBrushOuter = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));

    RoundRect(memDC, 0, 0, width, height, 4, 4);

    SelectObject(memDC, oldBrushOuter);
    SelectObject(memDC, oldPenOuter);
    DeleteObject(hPenBorder);

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

int ui_hit_test_counter(int mouseX, int mouseY, int width, int height, const HudConfig *config) {
    if (!config || !config->show_counter) return 0;

    HudSize size = (config->size >= 0 && config->size <= 2) ? config->size : HUD_SIZE_SMALL;
    int pad_x, pad_y, slot_gap, header_h;
    switch (size) {
        case HUD_SIZE_MEDIUM: pad_x = 4; pad_y = 4; slot_gap = 4; header_h = 16; break;
        case HUD_SIZE_LARGE:  pad_x = 5; pad_y = 5; slot_gap = 5; header_h = 19; break;
        case HUD_SIZE_SMALL:
        default:              pad_x = 3; pad_y = 3; slot_gap = 3; header_h = 13; break;
    }

    int counter_index = -1;
    int total_slots = 0;
    if (config->show_cpu) total_slots++;
    if (config->show_gpu) total_slots++;
    if (config->show_vram) total_slots++;
    if (config->show_ram) total_slots++;
    if (config->show_ping) total_slots++;
    if (config->show_power) total_slots++;
    if (config->show_counter) {
        counter_index = total_slots;
        total_slots++;
    }

    if (counter_index < 0 || total_slots == 0) return 0;

    if (config->orientation == HUD_ORIENT_VERTICAL) {
        int slot_w = width - (pad_x * 2);
        int slot_h = (size == HUD_SIZE_SMALL) ? 26 : ((size == HUD_SIZE_MEDIUM) ? 33 : 40);
        int target_y = header_h + pad_y + (counter_index * (slot_h + slot_gap));
        if (mouseX >= pad_x && mouseX <= pad_x + slot_w &&
            mouseY >= target_y && mouseY <= target_y + slot_h) {
            return 1;
        }
    } else {
        int slot_y = header_h + 2;
        int slot_h = height - slot_y - pad_y;
        int total_gaps = (total_slots - 1) * slot_gap;
        int available_w = width - (pad_x * 2) - total_gaps;
        int base_w = available_w / total_slots;
        int extra = available_w % total_slots;

        int cur_x = pad_x;
        for (int i = 0; i < total_slots; i++) {
            int w_this = base_w + (i < extra ? 1 : 0);
            if (i == counter_index) {
                if (mouseX >= cur_x && mouseX <= cur_x + w_this &&
                    mouseY >= slot_y && mouseY <= slot_y + slot_h) {
                    return 1;
                }
                break;
            }
            cur_x += w_this + slot_gap;
        }
    }
    return 0;
}
