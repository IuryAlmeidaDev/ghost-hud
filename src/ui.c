#include "ui.h"
#include <stdio.h>

static HFONT hFontTitle = NULL;
static HFONT hFontLabel = NULL;
static HFONT hFontValue = NULL;
static HFONT hFontMini = NULL;

// Paleta de Cores Inspirada no Estilo MMORPG / Dark Navy Slate do Print
#define COLOR_BG          RGB(13, 17, 26)      // Fundo azul-marinho profundo do jogo
#define COLOR_HEADER_BG   RGB(19, 25, 36)      // Fundo do cabeçalho integrado
#define COLOR_BORDER_LOCK RGB(40, 50, 68)      // Borda metálica sutil estilo interface MMO
#define COLOR_BORDER_ADJ  RGB(0, 229, 255)     // Borda ciano neon no modo ajuste
#define COLOR_DIVIDER     RGB(30, 38, 54)      // Divisores elegantes de 1px
#define COLOR_TRACK       RGB(24, 30, 42)      // Trilha escura das barras de carga

#define COLOR_TITLE       RGB(180, 195, 215)   // Título do cabeçalho (GhostHUD)
#define COLOR_LABEL       RGB(126, 142, 159)   // Rótulos secundários (CPU, GPU, RAM)

// Escala de Cores Progressiva de 4 Estágios (Combinando com a interface do jogo):
// Verde Turquesa (<55%) -> Amarelo Ouro (55-71%) -> Laranja (72-84%) -> Vermelho Coral (>=85%)
#define COLOR_GREEN       RGB(32, 201, 151)    // 🟢 Verde estilo poção/barra MMO
#define COLOR_YELLOW      RGB(255, 212, 59)    // 🟡 Amarelo ouro
#define COLOR_ORANGE      RGB(255, 146, 43)    // 🟠 Laranja quente
#define COLOR_RED         RGB(255, 107, 107)   // 🔴 Vermelho crítico (90%+)
#define COLOR_CYAN        RGB(0, 229, 255)     // Ciano elétrico de ajuste

static COLORREF get_status_color(int usage_pct, int temp_c) {
    int score = usage_pct;
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
    // Fonte da barra de título superior
    hFontTitle = CreateFontA(
        -9, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte dos rótulos (CPU, GPU, RAM)
    hFontLabel = CreateFontA(
        -10, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte dos números principais de alta definição
    hFontValue = CreateFontA(
        -15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte para o modo minimizado condensado
    hFontMini = CreateFontA(
        -10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
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

static void render_mini_mode(HDC memDC, int width, int height, const HardwareMetrics *m, BOOL locked) {
    // Cabeçalho mini
    RECT rHdr = { 2, 2, width - 2, 17 };
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle);
    SetTextColor(memDC, COLOR_TITLE);
    RECT rTitle = { 6, 2, width - 6, 17 };
    DrawTextA(memDC, locked ? "HUD [F8]" : "HUD *", -1, &rTitle, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    // Linha divisória horizontal do cabeçalho
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, 17, NULL);
    LineTo(memDC, width - 2, 17);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    int cpu = m ? m->cpu_usage : 0;
    int gpu = m ? m->gpu_usage : 0;
    int gpu_t = m ? m->gpu_temp : 0;
    int ram = m ? m->ram_usage : 0;

    COLORREF col_cpu = get_status_color(cpu, 0);
    COLORREF col_gpu = get_status_color(gpu, gpu_t);
    COLORREF col_ram = get_status_color(ram, 0);

    char buf[32];

    // Linha 1: CPU
    RECT rL1 = { 6, 19, 28, 33 };
    RECT rV1 = { 28, 19, width - 6, 33 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "CPU", -1, &rL1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", cpu);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_cpu);
    DrawTextA(memDC, buf, -1, &rV1, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // Linha 2: GPU
    RECT rL2 = { 6, 34, 28, 48 };
    RECT rV2 = { 28, 34, width - 6, 48 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "GPU", -1, &rL2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", gpu);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_gpu);
    DrawTextA(memDC, buf, -1, &rV2, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // Linha 3: RAM
    RECT rL3 = { 6, 49, 28, 63 };
    RECT rV3 = { 28, 49, width - 6, 63 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "RAM", -1, &rL3, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", ram);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_ram);
    DrawTextA(memDC, buf, -1, &rV3, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
}

static void render_normal_mode(HDC memDC, int width, int height, const HardwareMetrics *m, BOOL locked) {
    // 1. Cabeçalho Integrado Estilo Jogo (Topo 19px)
    RECT rHdr = { 2, 2, width - 2, 19 };
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle);

    // Título e ícone à esquerda
    SetTextColor(memDC, COLOR_TITLE);
    RECT rTitle = { 10, 2, 150, 19 };
    DrawTextA(memDC, "GHOST HUD  \x95  DESEMPENHO", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Indicadores e atalhos à direita no cabeçalho
    RECT rControls = { width - 150, 2, width - 10, 19 };
    SetTextColor(memDC, locked ? COLOR_LABEL : COLOR_CYAN);
    if (locked) {
        DrawTextA(memDC, "[JOGO] F9:MOVER  F8:MINI", -1, &rControls, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
    } else {
        DrawTextA(memDC, "[AJUSTE] ARRASTE AQUI", -1, &rControls, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
    }

    // Linha divisória horizontal separando o cabeçalho do conteúdo
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, 19, NULL);
    LineTo(memDC, width - 2, 19);

    // Divisores verticais entre as 3 colunas
    int col_w = (width - 4) / 3;
    int col1_x = 2 + col_w;
    int col2_x = 2 + (col_w * 2);

    MoveToEx(memDC, col1_x, 22, NULL);
    LineTo(memDC, col1_x, height - 6);

    MoveToEx(memDC, col2_x, 22, NULL);
    LineTo(memDC, col2_x, height - 6);

    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    int gauge_w = col_w - 24;
    int gauge_y = height - 9;
    int gauge_h = 3;

    // --- COLUNA 1: CPU ---
    int cpu_usage = m ? m->cpu_usage : 0;
    COLORREF color_cpu = get_status_color(cpu_usage, 0);

    RECT rLabel1 = { 2, 21, col1_x, 33 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "CPU", -1, &rLabel1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    char buf[64];
    snprintf(buf, sizeof(buf), "%d%%", cpu_usage);
    RECT rVal1 = { 2, 33, col1_x, 52 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_cpu);
    DrawTextA(memDC, buf, -1, &rVal1, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    draw_gauge_bar(memDC, 2 + (col_w - gauge_w) / 2, gauge_y, gauge_w, gauge_h, cpu_usage, color_cpu);

    // --- COLUNA 2: GPU ---
    int gpu_usage = m ? m->gpu_usage : 0;
    int gpu_temp = m ? m->gpu_temp : 0;
    COLORREF color_gpu = get_status_color(gpu_usage, gpu_temp);

    RECT rLabel2 = { col1_x, 21, col2_x, 33 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "GPU", -1, &rLabel2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    if (gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%d%%   %d\xB0" "C", gpu_usage, gpu_temp);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", gpu_usage);
    }
    RECT rVal2 = { col1_x, 33, col2_x, 52 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_gpu);
    DrawTextA(memDC, buf, -1, &rVal2, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    draw_gauge_bar(memDC, col1_x + (col_w - gauge_w) / 2, gauge_y, gauge_w, gauge_h, gpu_usage, color_gpu);

    // --- COLUNA 3: RAM ---
    int ram_usage = m ? m->ram_usage : 0;
    COLORREF color_ram = get_status_color(ram_usage, 0);

    RECT rLabel3 = { col2_x, 21, width - 2, 33 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "RAM", -1, &rLabel3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    snprintf(buf, sizeof(buf), "%d%%", ram_usage);
    RECT rVal3 = { col2_x, 33, width - 2, 52 };
    SelectObject(memDC, hFontValue);
    SetTextColor(memDC, color_ram);
    DrawTextA(memDC, buf, -1, &rVal3, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    draw_gauge_bar(memDC, col2_x + (col_w - gauge_w) / 2, gauge_y, gauge_w, gauge_h, ram_usage, color_ram);
}

void ui_render(HDC hdc, int width, int height, const HardwareMetrics *m, BOOL locked, BOOL minimized) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    // 1. Cria região com cantos arredondados para clipar o fundo e o cabeçalho perfeitamente
    HRGN hClip = CreateRoundRectRgn(0, 0, width + 1, height + 1, 8, 8);
    SelectClipRgn(memDC, hClip);

    // 2. Preenche fundo principal
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG);
    RECT rcAll = { 0, 0, width, height };
    FillRect(memDC, &rcAll, hBrushBg);
    DeleteObject(hBrushBg);

    // 3. Renderiza conteúdo específico (Normal ou Mini)
    if (minimized) {
        render_mini_mode(memDC, width, height, m, locked);
    } else {
        render_normal_mode(memDC, width, height, m, locked);
    }

    // 4. Remove o clipping para desenhar a borda externa
    SelectClipRgn(memDC, NULL);
    DeleteObject(hClip);

    // 5. Desenha a borda metálica arredondada estilo MMORPG
    HPEN hPenBorder = CreatePen(PS_SOLID, locked ? 1 : 2, locked ? COLOR_BORDER_LOCK : COLOR_BORDER_ADJ);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenBorder);

    RoundRect(memDC, 0, 0, width, height, 8, 8);

    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenBorder);

    // Copia buffer para tela (Double Buffering sem flicker)
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
