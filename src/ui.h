#ifndef UI_H
#define UI_H

#include <windows.h>
#include "hardware.h"

// Dimensões compactas e perfeitamente proporcionais ao HUD in-game
#define HUD_WIDTH        192
#define HUD_HEIGHT       44

#define HUD_MINI_WIDTH   52
#define HUD_MINI_HEIGHT  44

#define UI_IS_TOGGLE_BTN(x, y, min) ( \
    (min) ? ((x) >= (HUD_MINI_WIDTH - 18) && (x) <= (HUD_MINI_WIDTH - 2) && (y) >= 1 && (y) <= 15) : \
            ((x) >= (HUD_WIDTH - 20) && (x) <= (HUD_WIDTH - 2) && (y) >= 1 && (y) <= 15) \
)

void ui_init(void);
void ui_render(HDC hdc, int width, int height, const HardwareMetrics *metrics, BOOL minimized);
void ui_cleanup(void);

#endif // UI_H
