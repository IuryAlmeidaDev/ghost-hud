#include "ui.h"
#include <stdio.h>

static HFONT hFontTitle = NULL;
static HFONT hFontLabel = NULL;
static HFONT hFontValue = NULL;
static HFONT hFontMini = NULL;

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
    hFontTitle = CreateFontA(
        -8, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    hFontLabel = CreateFontA(
        -8, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    hFontValue = CreateFontA(
        -11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    hFontMini = CreateFontA(
        -8, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );
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

static void draw_metric_slot(HDC memDC, int x, int y, int w, int h, 
                             const char *label, const char *value, int pct, COLORREF val_color) {
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
    RECT rLbl = { x, y + 1, x + w, y + 10 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, label, -1, &rLbl, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // 3. Valor principal
    RECT rVal = { x, y + 10, x + w, y + 21 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, val_color);
    DrawTextA(memDC, value, -1, &rVal, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // 4. Mini barra de progresso no rodapé do slot
    int g_w = w - 6;
    int g_x = x + 3;
    int g_y = y + h - 3;
    draw_gauge_bar(memDC, g_x, g_y, g_w, 2, pct, val_color);
}

static void render_mini_mode(HDC memDC, int width, int height, const HardwareMetrics *m, int timer_left) {
    RECT rHdr = { 2, 2, width - 2, 12 };
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle);
    SetTextColor(memDC, COLOR_TITLE);

    RECT rTitle = { 4, 1, width - 16, 12 };
    if (timer_left > 0) {
        char tbuf[16];
        snprintf(tbuf, sizeof(tbuf), "%02d:%02d", timer_left / 60, timer_left % 60);
        SetTextColor(memDC, COLOR_YELLOW);
        DrawTextA(memDC, tbuf, -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    } else {
        DrawTextA(memDC, "HUD", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    }

    // Botão [+]
    int btn_x = width - 14;
    int btn_y = 2;
    HPEN hPenPlus = CreatePen(PS_SOLID, 1, COLOR_TITLE);
    HPEN oldPenBtn = (HPEN)SelectObject(memDC, hPenPlus);
    MoveToEx(memDC, btn_x + 1, btn_y + 4, NULL);
    LineTo(memDC, btn_x + 8, btn_y + 4);
    MoveToEx(memDC, btn_x + 4, btn_y + 1, NULL);
    LineTo(memDC, btn_x + 4, btn_y + 8);
    SelectObject(memDC, oldPenBtn);
    DeleteObject(hPenPlus);

    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, 12, NULL);
    LineTo(memDC, width - 2, 12);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    int cpu = m ? m->cpu_usage : 0;
    int gpu = m ? m->gpu_usage : 0;
    int vram = m ? m->vram_usage : 0;
    int ram = m ? m->ram_usage : 0;
    int ping = m ? m->ping_ms : 0;

    char buf[16];

    // 5 linhas compactas no mini mode
    int y_start = 13;
    int row_h = 10;

    // CPU
    RECT rL1 = { 4, y_start, 24, y_start + row_h };
    RECT rV1 = { 24, y_start, width - 4, y_start + row_h };
    SelectObject(memDC, hFontLabel); SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "CPU", -1, &rL1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%d%%", cpu);
    SelectObject(memDC, hFontMini); SetTextColor(memDC, get_status_color(cpu, 0));
    DrawTextA(memDC, buf, -1, &rV1, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // GPU
    y_start += row_h;
    RECT rL2 = { 4, y_start, 24, y_start + row_h };
    RECT rV2 = { 24, y_start, width - 4, y_start + row_h };
    SelectObject(memDC, hFontLabel); SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "GPU", -1, &rL2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%d%%", gpu);
    SelectObject(memDC, hFontMini); SetTextColor(memDC, get_status_color(gpu, m ? m->gpu_temp : 0));
    DrawTextA(memDC, buf, -1, &rV2, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // VRAM
    y_start += row_h;
    RECT rL3 = { 4, y_start, 24, y_start + row_h };
    RECT rV3 = { 24, y_start, width - 4, y_start + row_h };
    SelectObject(memDC, hFontLabel); SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "VRAM", -1, &rL3, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%d%%", vram);
    SelectObject(memDC, hFontMini); SetTextColor(memDC, get_status_color(vram, 0));
    DrawTextA(memDC, buf, -1, &rV3, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // RAM
    y_start += row_h;
    RECT rL4 = { 4, y_start, 24, y_start + row_h };
    RECT rV4 = { 24, y_start, width - 4, y_start + row_h };
    SelectObject(memDC, hFontLabel); SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "RAM", -1, &rL4, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%d%%", ram);
    SelectObject(memDC, hFontMini); SetTextColor(memDC, get_status_color(ram, 0));
    DrawTextA(memDC, buf, -1, &rV4, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // PING
    y_start += row_h;
    RECT rL5 = { 4, y_start, 24, y_start + row_h };
    RECT rV5 = { 24, y_start, width - 4, y_start + row_h };
    SelectObject(memDC, hFontLabel); SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "PING", -1, &rL5, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%dms", ping);
    SelectObject(memDC, hFontMini); SetTextColor(memDC, get_ping_color(ping));
    DrawTextA(memDC, buf, -1, &rV5, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
}

static void render_normal_mode(HDC memDC, int width, int height, const HardwareMetrics *m, int timer_left) {
    // 1. Cabeçalho Integrado com Relógio / Timer de Buff
    RECT rHdr = { 2, 2, width - 2, 13 };
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle);

    // Título à esquerda
    SetTextColor(memDC, COLOR_TITLE);
    RECT rTitle = { 6, 1, 62, 13 };
    DrawTextA(memDC, "GHOST HUD", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Relógio Digital / Timer de Buffs
    char timeBuf[32];
    RECT rClock = { 65, 1, width - 20, 13 };
    if (timer_left > 0) {
        snprintf(timeBuf, sizeof(timeBuf), "\x95  BUFF: %02d:%02d", timer_left / 60, timer_left % 60);
        SetTextColor(memDC, COLOR_YELLOW);
    } else {
        SYSTEMTIME st;
        GetLocalTime(&st);
        snprintf(timeBuf, sizeof(timeBuf), "\x95  %02d:%02d", st.wHour, st.wMinute);
        SetTextColor(memDC, COLOR_TIME);
    }
    DrawTextA(memDC, timeBuf, -1, &rClock, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Botão discreto [-] de minimizar no canto superior direito
    int btn_x = width - 16;
    int btn_y = 2;
    HPEN hPenMinus = CreatePen(PS_SOLID, 1, COLOR_TITLE);
    HPEN oldPenBtn = (HPEN)SelectObject(memDC, hPenMinus);
    MoveToEx(memDC, btn_x + 2, btn_y + 4, NULL);
    LineTo(memDC, btn_x + 9, btn_y + 4);
    SelectObject(memDC, oldPenBtn);
    DeleteObject(hPenMinus);

    // Linha divisória horizontal separando cabeçalho
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, 13, NULL);
    LineTo(memDC, width - 2, 13);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    // 2. Cinco Slots Modulares de Status: CPU, GPU, VRAM, RAM, PING
    // Largura total: width = 240. Margens: 4px cada lado. Espaço útil = 232px.
    // 5 slots de 44px com 3px de espaçamento entre eles:
    // Slot 1: x = 4
    // Slot 2: x = 51
    // Slot 3: x = 98
    // Slot 4: x = 145
    // Slot 5: x = 192
    int slot_w = 44;
    int slot_h = 26;
    int slot_y = 15;

    char buf[32];

    // --- SLOT 1: CPU ---
    int cpu = m ? m->cpu_usage : 0;
    COLORREF color_cpu = get_status_color(cpu, 0);
    snprintf(buf, sizeof(buf), "%d%%", cpu);
    draw_metric_slot(memDC, 4, slot_y, slot_w, slot_h, "CPU", buf, cpu, color_cpu);

    // --- SLOT 2: GPU ---
    int gpu = m ? m->gpu_usage : 0;
    int gpu_temp = m ? m->gpu_temp : 0;
    COLORREF color_gpu = get_status_color(gpu, gpu_temp);
    if (gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%d\xB0" "C", gpu_temp); // Foco na temperatura e carga
    } else {
        snprintf(buf, sizeof(buf), "%d%%", gpu);
    }
    draw_metric_slot(memDC, 51, slot_y, slot_w, slot_h, "GPU", buf, gpu, color_gpu);

    // --- SLOT 3: VRAM ---
    int vram_pct = m ? m->vram_usage : 0;
    COLORREF color_vram = get_status_color(vram_pct, 0);
    if (m && m->vram_used_mb > 0) {
        float vram_gb = (float)m->vram_used_mb / 1024.0f;
        snprintf(buf, sizeof(buf), "%.1fG", vram_gb);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", vram_pct);
    }
    draw_metric_slot(memDC, 98, slot_y, slot_w, slot_h, "VRAM", buf, vram_pct, color_vram);

    // --- SLOT 4: RAM ---
    int ram = m ? m->ram_usage : 0;
    COLORREF color_ram = get_status_color(ram, 0);
    snprintf(buf, sizeof(buf), "%d%%", ram);
    draw_metric_slot(memDC, 145, slot_y, slot_w, slot_h, "RAM", buf, ram, color_ram);

    // --- SLOT 5: PING ---
    int ping = m ? m->ping_ms : 0;
    COLORREF color_ping = get_ping_color(ping);
    if (ping > 0) {
        snprintf(buf, sizeof(buf), "%dms", ping);
    } else {
        snprintf(buf, sizeof(buf), "--ms");
    }
    int ping_gauge = (ping > 0) ? ((ping * 100) / 120) : 0;
    draw_metric_slot(memDC, 192, slot_y, slot_w, slot_h, "PING", buf, ping_gauge, color_ping);
}

void ui_render(HDC hdc, int width, int height, const HardwareMetrics *m, BOOL minimized, int timer_left) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    // 1. Clipping com cantos sutis de 3px
    HRGN hClip = CreateRoundRectRgn(0, 0, width + 1, height + 1, 3, 3);
    SelectClipRgn(memDC, hClip);

    // 2. Fundo principal
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG);
    RECT rcAll = { 0, 0, width, height };
    FillRect(memDC, &rcAll, hBrushBg);
    DeleteObject(hBrushBg);

    // 3. Renderiza conteúdo
    if (minimized) {
        render_mini_mode(memDC, width, height, m, timer_left);
    } else {
        render_normal_mode(memDC, width, height, m, timer_left);
    }

    // 4. Remove clipping para desenhar a borda
    SelectClipRgn(memDC, NULL);
    DeleteObject(hClip);

    // 5. Alerta Térmico: se GPU >= 82°C, borda pisca em vermelho suave de proteção
    BOOL is_hot = (m && m->gpu_temp >= 82);
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, is_hot ? COLOR_BORDER_HOT : COLOR_BORDER);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenBorder);

    RoundRect(memDC, 0, 0, width, height, 3, 3);

    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenBorder);

    // Double buffering sem flicker
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

void ui_cleanup(void) {
    if (hFontTitle) {
        DeleteObject(hFontTitle);
        hFontTitle = NULL;
    }
    if (hFontLabel) {
        DeleteObject(hFontLabel);
        hFontLabel = NULL;
    }
    if (hFontValue) {
        DeleteObject(hFontValue);
        hFontValue = NULL;
    }
    if (hFontMini) {
        DeleteObject(hFontMini);
        hFontMini = NULL;
    }
}
