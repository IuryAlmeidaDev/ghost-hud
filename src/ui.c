#include "ui.h"
#include <stdio.h>

static HFONT hFontTitle = NULL;
static HFONT hFontLabel = NULL;
static HFONT hFontValue = NULL;
static HFONT hFontMini = NULL;

// Paleta de Cores Fiel ao Padrão de Interface do Jogo (Aion / NCSoft)
#define COLOR_BG          RGB(15, 17, 21)      // Fundo obsidian slate escuro do jogo (#0F1115)
#define COLOR_HEADER_BG   RGB(19, 23, 30)      // Fundo do cabeçalho integrado (#13171E)
#define COLOR_SLOT_BG     RGB(22, 27, 36)      // Fundo dos módulos/slots de status (#161B24)
#define COLOR_SLOT_BORDER RGB(34, 42, 54)      // Borda interna de 1px dos slots (#222A36)
#define COLOR_BORDER      RGB(42, 50, 64)      // Borda metálica sutil da janela (#2A3240)
#define COLOR_DIVIDER     RGB(28, 34, 44)      // Separador horizontal sutil (#1C222C)
#define COLOR_TRACK       RGB(14, 17, 23)      // Trilha escura rebaixada das barras de progresso (#0E1117)

#define COLOR_TITLE       RGB(220, 226, 236)   // Branco suave dos títulos e ícones (#DCE2EC)
#define COLOR_LABEL       RGB(124, 138, 156)   // Cinza aço dos rótulos in-game (#7C8A9C)

// Escala de Cores Vibrante do HUD do Jogo:
#define COLOR_GREEN       RGB(46, 213, 115)    // 🟢 Verde esmeralda brilhante (#2ED573 - idêntico aos buffs)
#define COLOR_YELLOW      RGB(255, 190, 118)   // 🟡 Ouro suave (#FFBE76)
#define COLOR_ORANGE      RGB(255, 121, 63)    // 🟠 Laranja quente (#FF793F)
#define COLOR_RED         RGB(255, 71, 87)     // 🔴 Vermelho carmesim (#FF4757 - idêntico à barra de HP)

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

void ui_init(void) {
    // Fonte da barra de título (-8 Bold Segoe UI)
    hFontTitle = CreateFontA(
        -8, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte dos rótulos CPU, GPU, RAM (-8 Semibold Segoe UI)
    hFontLabel = CreateFontA(
        -8, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte dos números de alta visibilidade (-11 Bold Segoe UI)
    hFontValue = CreateFontA(
        -11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    // Fonte para o modo minimizado compacto (-8 Bold Segoe UI)
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

// Renderiza um slot/cartão modular com acabamento de 1px idêntico aos botões do jogo
static void draw_metric_slot(HDC memDC, int x, int y, int w, int h, 
                             const char *label, const char *value, int pct, COLORREF val_color) {
    // 1. Fundo do slot
    HBRUSH hBrushSlot = CreateSolidBrush(COLOR_SLOT_BG);
    HPEN hPenSlot = CreatePen(PS_SOLID, 1, COLOR_SLOT_BORDER);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, hBrushSlot);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenSlot);

    RoundRect(memDC, x, y, x + w, y + h, 2, 2);

    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(hBrushSlot);
    DeleteObject(hPenSlot);

    // 2. Rótulo (CPU, GPU, RAM)
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
    int g_w = w - 8;
    int g_x = x + 4;
    int g_y = y + h - 3;
    draw_gauge_bar(memDC, g_x, g_y, g_w, 2, pct, val_color);
}

static void render_mini_mode(HDC memDC, int width, int height, const HardwareMetrics *m) {
    // Cabeçalho mini
    RECT rHdr = { 2, 2, width - 2, 12 };
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle);
    SetTextColor(memDC, COLOR_TITLE);

    // Título à esquerda
    RECT rTitle = { 4, 1, width - 16, 12 };
    DrawTextA(memDC, "HUD", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Botão discreto [+] de maximizar no canto superior direito
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

    // Linha divisória horizontal do cabeçalho
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, 12, NULL);
    LineTo(memDC, width - 2, 12);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    int cpu = m ? m->cpu_usage : 0;
    int gpu = m ? m->gpu_usage : 0;
    int ram = m ? m->ram_usage : 0;

    COLORREF col_cpu = get_status_color(cpu, 0);
    COLORREF col_gpu = get_status_color(gpu, m ? m->gpu_temp : 0);
    COLORREF col_ram = get_status_color(ram, 0);

    char buf[16];

    // Linha 1: CPU
    RECT rL1 = { 4, 13, 20, 22 };
    RECT rV1 = { 20, 13, width - 4, 22 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "CPU", -1, &rL1, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%d%%", cpu);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_cpu);
    DrawTextA(memDC, buf, -1, &rV1, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // Linha 2: GPU
    RECT rL2 = { 4, 23, 20, 32 };
    RECT rV2 = { 20, 23, width - 4, 32 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "GPU", -1, &rL2, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%d%%", gpu);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_gpu);
    DrawTextA(memDC, buf, -1, &rV2, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

    // Linha 3: RAM
    RECT rL3 = { 4, 33, 20, 42 };
    RECT rV3 = { 20, 33, width - 4, 42 };
    SelectObject(memDC, hFontLabel);
    SetTextColor(memDC, COLOR_LABEL);
    DrawTextA(memDC, "RAM", -1, &rL3, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    snprintf(buf, sizeof(buf), "%d%%", ram);
    SelectObject(memDC, hFontMini);
    SetTextColor(memDC, col_ram);
    DrawTextA(memDC, buf, -1, &rV3, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
}

static void render_normal_mode(HDC memDC, int width, int height, const HardwareMetrics *m) {
    // 1. Cabeçalho Integrado Estilo Jogo (Topo 13px)
    RECT rHdr = { 2, 2, width - 2, 13 };
    HBRUSH hBrushHdr = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &rHdr, hBrushHdr);
    DeleteObject(hBrushHdr);

    SetBkMode(memDC, TRANSPARENT);
    SelectObject(memDC, hFontTitle);

    // Título limpo à esquerda: "GHOST HUD"
    SetTextColor(memDC, COLOR_TITLE);
    RECT rTitle = { 6, 1, width - 20, 13 };
    DrawTextA(memDC, "GHOST HUD", -1, &rTitle, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    // Botão discreto [-] de minimizar no canto superior direito
    int btn_x = width - 16;
    int btn_y = 2;
    HPEN hPenMinus = CreatePen(PS_SOLID, 1, COLOR_TITLE);
    HPEN oldPenBtn = (HPEN)SelectObject(memDC, hPenMinus);
    MoveToEx(memDC, btn_x + 2, btn_y + 4, NULL);
    LineTo(memDC, btn_x + 9, btn_y + 4);
    SelectObject(memDC, oldPenBtn);
    DeleteObject(hPenMinus);

    // Linha divisória horizontal separando o cabeçalho do conteúdo
    HPEN hPenDiv = CreatePen(PS_SOLID, 1, COLOR_DIVIDER);
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenDiv);
    MoveToEx(memDC, 2, 13, NULL);
    LineTo(memDC, width - 2, 13);
    SelectObject(memDC, oldPen);
    DeleteObject(hPenDiv);

    // 2. Três Slots Modulares Individuais (Estilo Cartões de Interface do Jogo)
    // Largura total: width = 192. Espaço útil = 186px.
    // 3 slots de 58px cada com 4px de espaçamento entre eles:
    // Slot 1: x = 4  (4 a 62)
    // Slot 2: x = 67 (67 a 125)
    // Slot 3: x = 130 (130 a 188)
    int slot_w = 58;
    int slot_h = 26;
    int slot_y = 15;

    char buf[32];

    // --- SLOT 1: CPU ---
    int cpu_usage = m ? m->cpu_usage : 0;
    COLORREF color_cpu = get_status_color(cpu_usage, 0);
    snprintf(buf, sizeof(buf), "%d%%", cpu_usage);
    draw_metric_slot(memDC, 4, slot_y, slot_w, slot_h, "CPU", buf, cpu_usage, color_cpu);

    // --- SLOT 2: GPU ---
    int gpu_usage = m ? m->gpu_usage : 0;
    int gpu_temp = m ? m->gpu_temp : 0;
    COLORREF color_gpu = get_status_color(gpu_usage, gpu_temp);
    if (gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%d%% %d\xB0", gpu_usage, gpu_temp);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", gpu_usage);
    }
    draw_metric_slot(memDC, 67, slot_y, slot_w, slot_h, "GPU", buf, gpu_usage, color_gpu);

    // --- SLOT 3: RAM ---
    int ram_usage = m ? m->ram_usage : 0;
    COLORREF color_ram = get_status_color(ram_usage, 0);
    snprintf(buf, sizeof(buf), "%d%%", ram_usage);
    draw_metric_slot(memDC, 130, slot_y, slot_w, slot_h, "RAM", buf, ram_usage, color_ram);
}

void ui_render(HDC hdc, int width, int height, const HardwareMetrics *m, BOOL minimized) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    // 1. Cria região com cantos sutis de 3px (perfeitamente alinhado aos painéis do jogo)
    HRGN hClip = CreateRoundRectRgn(0, 0, width + 1, height + 1, 3, 3);
    SelectClipRgn(memDC, hClip);

    // 2. Preenche fundo principal
    HBRUSH hBrushBg = CreateSolidBrush(COLOR_BG);
    RECT rcAll = { 0, 0, width, height };
    FillRect(memDC, &rcAll, hBrushBg);
    DeleteObject(hBrushBg);

    // 3. Renderiza conteúdo específico (Normal ou Mini)
    if (minimized) {
        render_mini_mode(memDC, width, height, m);
    } else {
        render_normal_mode(memDC, width, height, m);
    }

    // 4. Remove o clipping para desenhar a borda externa
    SelectClipRgn(memDC, NULL);
    DeleteObject(hClip);

    // 5. Desenha a borda metálica elegante e discreta (1px constante, estilo MMORPG)
    HPEN hPenBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memDC, GetStockObject(NULL_BRUSH));
    HPEN oldPen = (HPEN)SelectObject(memDC, hPenBorder);

    RoundRect(memDC, 0, 0, width, height, 3, 3);

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
