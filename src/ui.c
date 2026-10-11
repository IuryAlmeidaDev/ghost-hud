#include "ui.h"
#include <stdio.h>

static HFONT hFontTitle[3] = {NULL, NULL, NULL};
static HFONT hFontLabel[3] = {NULL, NULL, NULL};
static HFONT hFontValue[3] = {NULL, NULL, NULL};

// Paleta de Cores Obsidian Slate do Jogo
#define COLOR_BG          RGB(15, 17, 21)      // Fundo obsidian slate escuro (#0F1115)
#define COLOR_HEADER_BG   RGB(19, 23, 30)      // Fundo do cabeçalho (#13171E)
#define COLOR_SLOT_BG     RGB(22, 27, 36)      // Fundo dos módulos/slots (#161B24)
#define COLOR_SLOT_BORDER RGB(34, 42, 54)      // Borda interna de 1px dos slots (#222A36)
#define COLOR_BORDER      RGB(42, 50, 64)      // Borda metálica padrão da janela (#2A3240)
#define COLOR_BORDER_HOT  RGB(255, 71, 87)     // Borda de alerta térmico para GPU quente (#FF4757)
#define COLOR_DIVIDER     RGB(28, 34, 44)      // Separador horizontal sutil (#1C222C)
#define COLOR_TRACK       RGB(14, 17, 23)      // Trilha escura rebaixada das barras (#0E1117)

#define COLOR_TITLE       RGB(220, 226, 236)   // Branco suave (#DCE2EC)
#define COLOR_LABEL       RGB(124, 138, 156)   // Cinza aço dos rótulos (#7C8A9C)
#define COLOR_TIME        RGB(148, 162, 180)   // Cinza claro para o relógio (#94A2B4)

// Escala de Cores Vibrante do HUD do Jogo:
#define COLOR_GREEN       RGB(46, 213, 115)    // 🟢 Verde esmeralda (#2ED573)
#define COLOR_YELLOW      RGB(255, 190, 118)   // 🟡 Ouro suave (#FFBE76)
#define COLOR_ORANGE      RGB(255, 121, 63)    // 🟠 Laranja quente (#FF793F)
#define COLOR_RED         RGB(255, 71, 87)     // 🔴 Vermelho carmesim (#FF4757)

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
    if (ms <= 0) return COLOR_LABEL;
    if (ms <= 45) return COLOR_GREEN;
    if (ms <= 80) return COLOR_YELLOW;
    if (ms <= 130) return COLOR_ORANGE;
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

    int total_slots_w = (pad_x * 2) + (active_slots * slot_w) + ((active_slots > 1 ? active_slots - 1 : 0) * slot_gap);
    int final_w = (total_slots_w > min_hdr_w) ? total_slots_w : min_hdr_w;
    int final_h = (active_slots > 0) ? (header_h + slot_h + pad_y + 2) : (header_h + 3);

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

static void draw_gear_icon(HDC hdc, int cx, int cy, int r, COLORREF color) {
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, hPen);

    // 8 dentes radiantes
    MoveToEx(hdc, cx - r, cy, NULL); LineTo(hdc, cx + r + 1, cy);
    MoveToEx(hdc, cx, cy - r, NULL); LineTo(hdc, cx, cy + r + 1);
    int d = (r * 7) / 10;
    MoveToEx(hdc, cx - d, cy - d, NULL); LineTo(hdc, cx + d + 1, cy + d + 1);
    MoveToEx(hdc, cx - d, cy + d, NULL); LineTo(hdc, cx + d + 1, cy - d - 1);

    // Anel / furo central da engrenagem
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrushHdr);
    int inner_r = (r > 4) ? 3 : 2;
    Ellipse(hdc, cx - inner_r, cy - inner_r, cx + inner_r + 1, cy + inner_r + 1);

    SelectObject(hdc, oldBrush);
    DeleteObject(hBrushHdr);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
}

static void draw_metric_slot(HDC memDC, int x, int y, int w, int h, 
                             const char *label, const char *value, int pct, COLORREF val_color, HudSize size) {
    int s_idx = (size >= 0 && size <= 2) ? size : 0;

    // 1. Fundo e contorno do slot
    HBRUSH hBrushSlot = CreateSolidBrush(COLOR_SLOT_BG);
    HPEN hPenSlot = CreatePen(PS_SOLID, 1, COLOR_SLOT_BORDER);
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
    SetTextColor(memDC, COLOR_LABEL);
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

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // 1. Fundo geral Obsidian Slate (#0F1115)
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG);
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

    // 2. Cabeçalho Integrado (#13171E)
    RECT rHdr = { 2, 2, width - 2, header_h };
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle[s_idx]);

    // Título à esquerda
    SetTextColor(memDC, COLOR_TITLE);
    int title_w = (s_idx == 0) ? 58 : ((s_idx == 1) ? 72 : 88);
    RECT rTitle = { pad_x + 2, 1, pad_x + 2 + title_w, header_h };
    DrawTextA(memDC, "GHOST HUD", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Relógio Digital (se ativado)
    if (config->show_clock) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        char timeBuf[32];
        snprintf(timeBuf, sizeof(timeBuf), "\x95  %02d:%02d", st.wHour, st.wMinute);
        SetTextColor(memDC, COLOR_TIME);
        RECT rClock = { pad_x + 2 + title_w + 4, 1, width - 22, header_h };
        DrawTextA(memDC, timeBuf, -1, &rClock, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    }

    // Ícone da Engrenagem [⚙] no canto superior direito
    int gear_r = (s_idx == 0) ? 4 : ((s_idx == 1) ? 5 : 6);
    int gear_cx = width - 11;
    int gear_cy = header_h / 2 + 1;
    draw_gear_icon(memDC, gear_cx, gear_cy, gear_r, COLOR_TITLE);

    // Linha divisória horizontal sutil (#1C222C)
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, header_h, NULL);
    LineTo(memDC, width - 2, header_h);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    // 3. Renderização Dinâmica dos Módulos/Slots
    int active_slots = 0;
    if (config->show_cpu) active_slots++;
    if (config->show_gpu) active_slots++;
    if (config->show_vram) active_slots++;
    if (config->show_ram) active_slots++;
    if (config->show_ping) active_slots++;

    if (active_slots > 0) {
        int slot_y = header_h + 2;
        int slot_h = height - slot_y - pad_y;
        int total_gaps = (active_slots - 1) * slot_gap;
        int available_w = width - (pad_x * 2) - total_gaps;
        int slot_w = available_w / active_slots;
        int extra_pixels = available_w % active_slots;

        int cur_x = pad_x;
        char buf[32];

        // CPU
        if (config->show_cpu) {
            int w_this = slot_w + (extra_pixels-- > 0 ? 1 : 0);
            int cpu = metrics ? metrics->cpu_usage : 0;
            snprintf(buf, sizeof(buf), "%d%%", cpu);
            draw_metric_slot(memDC, cur_x, slot_y, w_this, slot_h, 
                             "CPU", buf, cpu, get_status_color(cpu, 0), size);
            cur_x += w_this + slot_gap;
        }

        // GPU
        if (config->show_gpu) {
            int w_this = slot_w + (extra_pixels-- > 0 ? 1 : 0);
            int gpu = metrics ? metrics->gpu_usage : 0;
            int temp = metrics ? metrics->gpu_temp : 0;
            snprintf(buf, sizeof(buf), "%d\xB0" "C", temp);
            draw_metric_slot(memDC, cur_x, slot_y, w_this, slot_h, 
                             "GPU", buf, gpu, get_status_color(gpu, temp), size);
            cur_x += w_this + slot_gap;
        }

        // VRAM
        if (config->show_vram) {
            int w_this = slot_w + (extra_pixels-- > 0 ? 1 : 0);
            int vram = metrics ? metrics->vram_usage : 0;
            snprintf(buf, sizeof(buf), "%d%%", vram);
            draw_metric_slot(memDC, cur_x, slot_y, w_this, slot_h, 
                             "VRAM", buf, vram, get_status_color(vram, 0), size);
            cur_x += w_this + slot_gap;
        }

        // RAM
        if (config->show_ram) {
            int w_this = slot_w + (extra_pixels-- > 0 ? 1 : 0);
            int ram = metrics ? metrics->ram_usage : 0;
            snprintf(buf, sizeof(buf), "%d%%", ram);
            draw_metric_slot(memDC, cur_x, slot_y, w_this, slot_h, 
                             "RAM", buf, ram, get_status_color(ram, 0), size);
            cur_x += w_this + slot_gap;
        }

        // PING
        if (config->show_ping) {
            int w_this = slot_w + (extra_pixels-- > 0 ? 1 : 0);
            int ping = metrics ? metrics->ping_ms : 0;
            if (ping > 0) snprintf(buf, sizeof(buf), "%dms", ping);
            else snprintf(buf, sizeof(buf), "--");
            draw_metric_slot(memDC, cur_x, slot_y, w_this, slot_h, 
                             "PING", buf, ping > 150 ? 100 : (ping * 100) / 150, get_ping_color(ping), size);
            cur_x += w_this + slot_gap;
        }
    }

    // 4. Borda Externa (com Alerta Térmico se GPU >= 82°C)
    COLORREF borderColor = COLOR_BORDER;
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
