#include "ui.h"
#include <stdio.h>

static HFONT hFontLabel = NULL;
static HFONT hFontValue = NULL;
static HFONT hFontSmall = NULL;
static HFONT hFontMini = NULL;

// Paleta de Cores Impeccable (Dark Minimalist Cyberpunk)
#define COLOR_BG          RGB(11, 15, 23)      // Fundo obsidiana profundo
#define COLOR_BORDER_LOCK RGB(33, 38, 45)      // Borda sutil grafite metálico
#define COLOR_BORDER_ADJ  RGB(0, 229, 255)     // Borda ciano neon elétrico
#define COLOR_DIVIDER     RGB(26, 32, 44)      // Divisor elegante
#define COLOR_LABEL       RGB(139, 148, 158)   // Titânio fosco para rótulos
#define COLOR_TRACK       RGB(22, 27, 34)      // Fundo dos mini gauges

// Escala de Cores Progressiva de 4 Estágios solicitada pelo usuário:
// Verde (<55%) -> Amarelo (55-71%) -> Laranja (72-84%) -> Vermelho Crítico (>=85%)
#define COLOR_GREEN       RGB(0, 230, 118)     // 🟢 Normal / Saudável
#define COLOR_YELLOW      RGB(255, 234, 0)     // 🟡 Médio / Moderado
#define COLOR_ORANGE      RGB(255, 145, 0)     // 🟠 Alto / Atenção
#define COLOR_RED         RGB(255, 59, 48)     // 🔴 Crítico / Alerta Máximo
#define COLOR_CYAN        RGB(0, 229, 255)     // Ciano elétrico

static COLORREF get_status_color(int usage_pct, int temp_c) {
    int score = usage_pct;
    // Eleva a gravidade se a temperatura estiver alta
    if (temp_c >= 83) {
        if (score < 85) score = 85;
    } else if (temp_c >= 75) {
        if (score < 73) score = 73;
    } else if (temp_c >= 66) {
        if (score < 56) score = 56;
    }

    if (score >= 85) return COLOR_RED;       // 🔴 Vermelho (>=85% ou 90% crítico)
    if (score >= 72) return COLOR_ORANGE;    // 🟠 Laranja (72% a 84%)
    if (score >= 55) return COLOR_YELLOW;    // 🟡 Amarelo (55% a 71%)
    return COLOR_GREEN;                      // 🟢 Verde (<55%)
}

void ui_init(void) {
    // Fonte dos rótulos superiores (CPU, GPU, RAM)
    hFontLabel = CreateFontA(
        -10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte principal dos números (Segoe UI bold)
    hFontValue = CreateFontA(
        -16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte menor para sub-valores
    hFontSmall = CreateFontA(
        -12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte condensada para o modo mini (quadradinho)
    hFontMini = CreateFontA(
        -11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );
}

static void draw_gauge_bar(HDC hdc, int x, int y, int width, int height, int pct, COLORREF color) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    // Trilha de fundo
    HBRUSH hBrushTrack = CreateSolidBrush(COLOR_TRACK);
    RECT rTrack = { x, y, x + width, y + height };
    FillRect(hdc, &rTrack, hBrushTrack);
    DeleteObject(hBrushTrack);

    // Barra de preenchimento
    int fill_w = (width * pct) / 100;
    if (fill_w > 0) {
        HBRUSH hBrushFill = CreateSolidBrush(color);
        RECT rFill = { x, y, x + fill_w, y + height };
        FillRect(hdc, &rFill, hBrushFill);
        DeleteObject(hBrushFill);
    }
}

static void render_mini_mode(HDC memDC, int width, int height, const HardwareMetrics *m, BOOL locked) {
    int cpu = m ? m->cpu_usage : 0;
    int gpu = m ? m->gpu_usage : 0;
    int gpu_t = m ? m->gpu_temp : 0;
    int ram = m ? m->ram_usage : 0;

    COLORREF col_cpu = get_status_color(cpu, 0);
    COLORREF col_gpu = get_status_color(gpu, gpu_t);
    COLORREF col_ram = get_status_color(ram, 0);

    char buf[32];
    SetBkMode(memDC, TRANSPARENT);

    // Linha 1: CPU
    RECT rL1 = { 8, 7, 22, 21 };
    RECT rV1 = { 24, 7, width - 8, 21 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "C", -1, &rL1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", cpu);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_cpu);
    DrawTextA(memDC, buf, -1, &rV1, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // Linha 2: GPU
    RECT rL2 = { 8, 21, 22, 35 };
    RECT rV2 = { 24, 21, width - 8, 35 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "G", -1, &rL2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", gpu);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_gpu);
    DrawTextA(memDC, buf, -1, &rV2, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // Linha 3: RAM
    RECT rL3 = { 8, 35, 22, 49 };
    RECT rV3 = { 24, 35, width - 8, 49 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "R", -1, &rL3, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", ram);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_ram);
    DrawTextA(memDC, buf, -1, &rV3, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
}

static void render_normal_mode(HDC memDC, int width, int height, const HardwareMetrics *m, BOOL locked) {
    // Divisores verticais sutis entre as 3 colunas
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);

    int col_w = width / 3;
    int col1_x = col_w;
    int col2_x = col_w * 2;

    MoveToEx(memDC, col1_x, 8, NULL);
    LineTo(memDC, col1_x, height - 8);

    MoveToEx(memDC, col2_x, 8, NULL);
    LineTo(memDC, col2_x, height - 8);

    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    int gauge_w = col_w - 28;
    int gauge_y = height - 10;
    int gauge_h = 3;

    // --- COLUNA 1: CPU ---
    int cpu_usage = m ? m->cpu_usage : 0;
    COLORREF color_cpu = get_status_color(cpu_usage, 0);

    RECT rLabel1 = { 0, 7, col1_x, 20 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "CPU", -1, &rLabel1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    char buf[64];
    snprintf(buf, sizeof(buf), "%d%%", cpu_usage);
    RECT rVal1 = { 0, 20, col1_x, 40 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_cpu);
    DrawTextA(memDC, buf, -1, &rVal1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    draw_gauge_bar(memDC, (col1_x - gauge_w) / 2, gauge_y, gauge_w, gauge_h, cpu_usage, color_cpu);

    // --- COLUNA 2: GPU ---
    int gpu_usage = m ? m->gpu_usage : 0;
    int gpu_temp = m ? m->gpu_temp : 0;
    COLORREF color_gpu = get_status_color(gpu_usage, gpu_temp);

    RECT rLabel2 = { col1_x, 7, col2_x, 20 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "GPU", -1, &rLabel2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    if (gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%d%%  %d\xB0" "C", gpu_usage, gpu_temp);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", gpu_usage);
    }
    RECT rVal2 = { col1_x, 20, col2_x, 40 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_gpu);
    DrawTextA(memDC, buf, -1, &rVal2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    draw_gauge_bar(memDC, col1_x + (col_w - gauge_w) / 2, gauge_y, gauge_w, gauge_h, gpu_usage, color_gpu);

    // --- COLUNA 3: RAM ---
    int ram_usage = m ? m->ram_usage : 0;
    COLORREF color_ram = get_status_color(ram_usage, 0);

    RECT rLabel3 = { col2_x, 7, width, 20 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "RAM", -1, &rLabel3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", ram_usage);
    RECT rVal3 = { col2_x, 20, width, 40 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_ram);
    DrawTextA(memDC, buf, -1, &rVal3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    draw_gauge_bar(memDC, col2_x + (col_w - gauge_w) / 2, gauge_y, gauge_w, gauge_h, ram_usage, color_ram);
}

void ui_render(HDC hdc, int width, int height, const HardwareMetrics *m, BOOL locked, BOOL minimized) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG);
    HPEN hPenBorder = CreatePen(PS_SOLID, locked ? 1 : 2, locked ? COLOR_BORDER_LOCK : COLOR_BORDER_ADJ);

    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrushBg);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenBorder);

    RoundRect(memDC, 0, 0, width, height, 14, 14);

    if (minimized) {
        render_mini_mode(memDC, width, height, m, locked);
    } else {
        render_normal_mode(memDC, width, height, m, locked);
    }

    // Indicador luminoso de Modo Ajuste (LED Ciano)
    if (!locked) {
        HBRUSH hBrushDot = CreateSolidBrush(COLOR_CYAN);
        HPEN hPenDot = CreatePen(PS_SOLID, 1, COLOR_CYAN);
        SelectObject(memDC, hBrushDot);
        SelectObject(memDC, hPenDot);
        if (minimized) {
            Ellipse(memDC, width - 11, 6, width - 6, 11);
        } else {
            Ellipse(memDC, 9, 8, 15, 14);
        }
        DeleteObject(hBrushDot);
        DeleteObject(hPenDot);
    }

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    SelectObject(memDC, oldBmp);
    DeleteObject(hBrushBg);
    DeleteObject(hPenBorder);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

void ui_cleanup(void) {
    if (hFontLabel) {
        DeleteObject(hFontLabel);
        hFontLabel = NULL;
    }
    if (hFontValue) {
        DeleteObject(hFontValue);
        hFontValue = NULL;
    }
    if (hFontSmall) {
        DeleteObject(hFontSmall);
        hFontSmall = NULL;
    }
    if (hFontMini) {
        DeleteObject(hFontMini);
        hFontMini = NULL;
    }
}
