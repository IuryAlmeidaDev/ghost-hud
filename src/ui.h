#ifndef UI_H
#define UI_H

#include <windows.h>
#include "hardware.h"

#define HUD_WIDTH        320
#define HUD_HEIGHT       66

#define HUD_MINI_WIDTH   72
#define HUD_MINI_HEIGHT  66

void ui_init(void);
void ui_render(HDC hdc, int width, int height, const HardwareMetrics *metrics, BOOL locked, BOOL minimized);
void ui_cleanup(void);

#endif // UI_H
