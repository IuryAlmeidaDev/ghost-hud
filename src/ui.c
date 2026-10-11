#include "ui.h"
#include <stdio.h>

static HFONT hFontLabel = NULL;
static HFONT hFontValue = NULL;
static HFONT hFontSmall = NULL;

// Paleta de Cores Impeccable (Dark Minimalist Cyberpunk)
#define COLOR_BG          RGB(11, 15, 23)      // Fundo obsidiana profundo
#define COLOR_BORDER_LOCK RGB(33, 38, 45)      // Borda sutil grafite metálico
#define COLOR_BORDER_ADJ  RGB(0, 229, 255)     // Borda ciano neon elétrico
#define COLOR_DIVIDER     RGB(26, 32, 44)      // Divisor elegante
#define COLOR_LABEL       RGB(139, 148, 158)   // Titânio fosco para rótulos
#define COLOR_TRACK       RGB(22, 27, 34)      // Fundo dos mini gauges

#define COLOR_GOOD        RGB(0, 230, 118)     // Verde esmeralda vivo (<72°C)
#define COLOR_WARM        RGB(255, 179, 0)     // Âmbar quente (72-82°C)
#define COLOR_HOT         RGB(255, 82, 82)     // Coral elétrico (>83°C)
#define COLOR_CYAN        RGB(0, 229, 255)     // Ciano de destaque

static COLORREF get_thermal_color(int val) {
    if (val >= 83) return COLOR_HOT;
    if (val >= 72) return COLOR_WARM;
    return COLOR_GOOD;
}

void ui_init(void) {
    // Fonte dos rótulos superiores (CPU, GPU, RAM)
    hFontLabel = CreateFontA(
        -10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte principal dos números (Segoe UI bold de alta legibilidade)
    hFontValue = CreateFontA(
        -16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte menor para sub-valores e graus
    hFontSmall = CreateFontA(
        -12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );
}

static void draw_gauge_bar(HDC hdc, int x, int y, int width, int height, int pct, COLORREF color) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    // Fundo da barra (trilha escura)
    HBRUSH hBrushTrack = CreateSolidBrush(COLOR_TRACK);
    RECT rTrack = { x, y, x + width, y + height };
    FillRect(hdc, &rTrack, hBrushTrack);
    DeleteObject(hBrushTrack);

    // Barra de preenchimento proporcional
    int fill_w = (width * pct) / 100;
    if (fill_w > 0) {
        HBRUSH hBrushFill = CreateSolidBrush(color);
        RECT rFill = { x, y, x + fill_w, y + height };
        FillRect(hdc, &rFill, hBrushFill);
        DeleteObject(hBrushFill);
    }
}

void ui_render(HDC hdc, int width, int height, const HardwareMetrics *m, BOOL locked) {
    // Cria buffer de memória (Double Buffering perfeito)
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    // Fundo da cápsula com cantos arredondados
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG);
    HPEN hPenBorder = CreatePen(PS_SOLID, locked ? 1 : 2, locked ? COLOR_BORDER_LOCK : COLOR_BORDER_ADJ);

    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrushBg);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenBorder);

    RoundRect(memDC, 0, 0, width, height, 16, 16);

    // Divisores verticais sutis entre as colunas
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    SelectObject(memDC, hPenDiv);

    int col_w = width / 3;
    int col1_x = col_w;
    int col2_x = col_w * 2;

    MoveToEx(memDC, col1_x, 8, NULL);
    LineTo(memDC, col1_x, height - 8);

    MoveToEx(memDC, col2_x, 8, NULL);
    LineTo(memDC, col2_x, height - 8);

    SetBkMode(memDC, TRANSPARENT);

    int gauge_w = col_w - 28;
    int gauge_y = height - 10;
    int gauge_h = 3;

    // ==========================================
    // COLUNA 1: CPU
    // ==========================================
    int cpu_usage = m ? m->cpu_usage : 0;
    COLORREF color_cpu = get_thermal_color(cpu_usage);

    // Rótulo "CPU"
    RECT rLabel1 = { 0, 7, col1_x, 20 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "CPU", -1, &rLabel1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Valor "XX%"
    char buf[64];
    snprintf(buf, sizeof(buf), "%d%%", cpu_usage);
    RECT rVal1 = { 0, 20, col1_x, 40 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_cpu);
    DrawTextA(memDC, buf, -1, &rVal1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Mini Gauge bar CPU
    draw_gauge_bar(memDC, (col1_x - gauge_w) / 2, gauge_y, gauge_w, gauge_h, cpu_usage, color_cpu);

    // ==========================================
    // COLUNA 2: GPU
    // ==========================================
    int gpu_usage = m ? m->gpu_usage : 0;
    int gpu_temp = m ? m->gpu_temp : 0;
    COLORREF color_gpu = get_thermal_color(gpu_temp > 0 ? gpu_temp : gpu_usage);

    // Rótulo "GPU"
    RECT rLabel2 = { col1_x, 7, col2_x, 20 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "GPU", -1, &rLabel2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Valor "%d%  %d°C"
    if (gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%d%%  %d\xB0" "C", gpu_usage, gpu_temp);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", gpu_usage);
    }
    RECT rVal2 = { col1_x, 20, col2_x, 40 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_gpu);
    DrawTextA(memDC, buf, -1, &rVal2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Mini Gauge bar GPU
    draw_gauge_bar(memDC, col1_x + (col_w - gauge_w) / 2, gauge_y, gauge_w, gauge_h, gpu_usage, color_gpu);

    // ==========================================
    // COLUNA 3: RAM
    // ==========================================
    int ram_usage = m ? m->ram_usage : 0;
    COLORREF color_ram = get_thermal_color(ram_usage);

    // Rótulo "RAM"
    RECT rLabel3 = { col2_x, 7, width, 20 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "RAM", -1, &rLabel3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Valor "XX%"
    snprintf(buf, sizeof(buf), "%d%%", ram_usage);
    RECT rVal3 = { col2_x, 20, width, 40 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_ram);
    DrawTextA(memDC, buf, -1, &rVal3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Mini Gauge bar RAM
    draw_gauge_bar(memDC, col2_x + (col_w - gauge_w) / 2, gauge_y, gauge_w, gauge_h, ram_usage, color_ram);

    // Indicador luminoso de estado no canto superior esquerdo
    if (!locked) {
        // Ponto de luz ciano no modo ajuste
        HBRUSH hBrushDot = CreateSolidBrush(COLOR_CYAN);
        HPEN hPenDot = CreatePen(PS_SOLID, 1, COLOR_CYAN);
        SelectObject(memDC, hBrushDot);
        SelectObject(memDC, hPenDot);
        Ellipse(memDC, 9, 8, 15, 14);
        DeleteObject(hBrushDot);
        DeleteObject(hPenDot);
    }

    // Copia do buffer para o DC final em alta velocidade
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // Limpeza de recursos
    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    SelectObject(memDC, oldBmp);
    DeleteObject(hBrushBg);
    DeleteObject(hPenBorder);
    DeleteObject(hPenDiv);
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
}
