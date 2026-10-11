#ifndef UI_H
#define UI_H

#include <windows.h>
#include "hardware.h"

#define HUD_WIDTH        320
#define HUD_HEIGHT       66

#define HUD_MINI_WIDTH   72
#define HUD_MINI_HEIGHT  66

#define UI_IS_TOGGLE_BTN(x, y, min) ( \
    (min) ? ((x) >= (HUD_MINI_WIDTH - 20) && (x) <= (HUD_MINI_WIDTH - 3) && (y) >= 2 && (y) <= 16) : \
            ((x) >= (HUD_WIDTH - 24) && (x) <= (HUD_WIDTH - 4) && (y) >= 2 && (y) <= 18) \
)

void ui_init(void);
void ui_render(HDC hdc, int width, int height, const HardwareMetrics *metrics, BOOL minimized);
void ui_cleanup(void);

#endif // UI_H
