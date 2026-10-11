#include "ui.h"
#include <stdio.h>

static HFONT hFontBadge = NULL;
static HFONT hFontValue = NULL;
static HFONT hFontHint = NULL;

// Paleta de Cores (Impeccable Dark Frosted Glass)
#define COLOR_BG          RGB(13, 17, 23)
#define COLOR_BORDER_LOCK RGB(48, 54, 61)
#define COLOR_BORDER_ADJ  RGB(0, 229, 255)
#define COLOR_DIVIDER     RGB(33, 38, 45)
#define COLOR_BADGE       RGB(139, 148, 158)
#define COLOR_GOOD        RGB(0, 230, 118)
#define COLOR_WARM        RGB(255, 171, 0)
#define COLOR_HOT         RGB(255, 82, 82)
#define COLOR_CYAN        RGB(0, 229, 255)

static COLORREF get_thermal_color(int val) {
    if (val >= 83) return COLOR_HOT;
    if (val >= 72) return COLOR_WARM;
    return COLOR_GOOD;
}

void ui_init(void) {
    // Fonte dos rótulos (CPU, GPU, RAM)
    hFontBadge = CreateFontA(
        -10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte dos números tabulares (Consolas)
    hFontValue = CreateFontA(
        -13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas"
    );

    // Fonte para dicas de modo
    hFontHint = CreateFontA(
        -9, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );
}

void ui_render(HDC hdc, int width, int height, const HardwareMetrics *m, BOOL locked) {
    // Cria buffer de memória para evitar qualquer flicker
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    // Fundo da cápsula com cantos arredondados
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG);
    HPEN hPenBorder = CreatePen(PS_SOLID, locked ? 1 : 2, locked ? COLOR_BORDER_LOCK : COLOR_BORDER_ADJ);

    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrushBg);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenBorder);

    RoundRect(memDC, 0, 0, width, height, 14, 14);

    // Divisores verticais sutis entre as colunas
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    SelectObject(memDC, hPenDiv);

    int col1_x = width / 3;
    int col2_x = (width * 2) / 3;

    MoveToEx(memDC, col1_x, 6, NULL);
    LineTo(memDC, col1_x, height - 6);

    MoveToEx(memDC, col2_x, 6, NULL);
    LineTo(memDC, col2_x, height - 6);

    SetBkMode(memDC, TRANSPARENT);

    // --- Coluna 1: CPU ---
    RECT rBadge1 = { 0, 5, col1_x, 18 };
    RECT rVal1   = { 0, 19, col1_x, height - 5 };

    SelectObject(memDC, hFontBadge);
    SetTextColor(memDC, COLOR_BADGE);
    DrawTextA(memDC, "CPU", -1, &rBadge1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    char buf[64];
    snprintf(buf, sizeof(buf), "%d%%", m ? m->cpu_usage : 0);
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, get_thermal_color(m ? m->cpu_usage : 0));
    DrawTextA(memDC, buf, -1, &rVal1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // --- Coluna 2: GPU ---
    RECT rBadge2 = { col1_x, 5, col2_x, 18 };
    RECT rVal2   = { col1_x, 19, col2_x, height - 5 };

    SelectObject(memDC, hFontBadge);
    SetTextColor(memDC, COLOR_BADGE);
    DrawTextA(memDC, "GPU", -1, &rBadge2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    int gpu_temp = m ? m->gpu_temp : 0;
    if (gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%d%% %d C", m ? m->gpu_usage : 0, gpu_temp);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", m ? m->gpu_usage : 0);
    }
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, get_thermal_color(gpu_temp > 0 ? gpu_temp : (m ? m->gpu_usage : 0)));
    DrawTextA(memDC, buf, -1, &rVal2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // --- Coluna 3: RAM ---
    RECT rBadge3 = { col2_x, 5, width, 18 };
    RECT rVal3   = { col2_x, 19, width, height - 5 };

    SelectObject(memDC, hFontBadge);
    SetTextColor(memDC, COLOR_BADGE);
    DrawTextA(memDC, "RAM", -1, &rBadge3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", m ? m->ram_usage : 0);
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, get_thermal_color(m ? m->ram_usage : 0));
    DrawTextA(memDC, buf, -1, &rVal3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Se estiver no Modo Ajuste, exibe pequena indicação de arrasto
    if (!locked) {
        RECT rHint = { 0, height - 12, width, height - 1 };
        SelectObject(memDC, hFontHint);
        SetTextColor(memDC, COLOR_CYAN);
        DrawTextA(memDC, "ARRASTE  |  F9 P/ TRAVAR", -1, &rHint, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    }

    // Copia buffer de memória para o HDC da tela em uma única chamada rápida
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // Limpeza de recursos GDI temporários
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
    if (hFontBadge) {
        DeleteObject(hFontBadge);
        hFontBadge = NULL;
    }
    if (hFontValue) {
        DeleteObject(hFontValue);
        hFontValue = NULL;
    }
    if (hFontHint) {
        DeleteObject(hFontHint);
        hFontHint = NULL;
    }
}
