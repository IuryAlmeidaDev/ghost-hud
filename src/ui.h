#ifndef UI_H
#define UI_H

#include <windows.h>
#include "hardware.h"

typedef enum {
    HUD_SIZE_SMALL = 0,
    HUD_SIZE_MEDIUM = 1,
    HUD_SIZE_LARGE = 2
} HudSize;

typedef enum {
    HUD_ORIENT_HORIZONTAL = 0,
    HUD_ORIENT_VERTICAL = 1
} HudOrientation;

typedef enum {
    HUD_THEME_OBSIDIAN = 0,
    HUD_THEME_BLOODMOON = 1,
    HUD_THEME_CYBERNEON = 2,
    HUD_THEME_ROYALGOLD = 3
} HudTheme;

typedef struct {
    BOOL show_cpu;
    BOOL show_gpu;
    BOOL show_vram;
    BOOL show_ram;
    BOOL show_ping;
    BOOL show_power;
    BOOL show_clock;
    BOOL show_counter;
    int counter_val;
    int counter_label_type; // 0: FARM, 1: KILLS, 2: BOSS, 3: RUNS
    HudSize size;
    HudOrientation orientation;
    HudTheme theme;
    int alpha;
    BOOL ghost_mode;
    BOOL auto_fade;
    BOOL auto_game_mode;
} HudConfig;

#define UI_IS_GEAR_BTN(x, y, width, header_h) ( \
    ((x) >= ((width) - 22)) && ((x) <= ((width) - 2)) && ((y) >= 0) && ((y) <= (header_h)) \
)

void ui_init(void);
void ui_get_window_size(const HudConfig *cfg, int *out_width, int *out_height, int *out_header_h);
void ui_render(HDC hdc, int width, int height, const HardwareMetrics *metrics, const HudConfig *config);
int ui_hit_test_counter(int mouseX, int mouseY, int width, int height, const HudConfig *config);
void ui_cleanup(void);

#endif // UI_H
