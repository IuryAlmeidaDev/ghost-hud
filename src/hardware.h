#ifndef HARDWARE_H
#define HARDWARE_H

#include <windows.h>

typedef struct {
    int cpu_usage;       // 0 a 100 %
    int gpu_usage;       // 0 a 100 %
    int gpu_temp;        // Temperatura em °C
    int vram_used_mb;    // VRAM usada em MB
    int vram_total_mb;   // VRAM total em MB
    int vram_usage;      // 0 a 100 %
    int ram_usage;       // 0 a 100 %
    int ram_used_mb;     // RAM usada em MB
    int ram_total_mb;    // RAM total em MB
    int ping_ms;         // Latência de rede em ms (tempo real)
    int gpu_power_w;     // Consumo da GPU em Watts (TDP real)
} HardwareMetrics;

void hardware_init(void);
void hardware_update(HardwareMetrics *out_metrics);
void hardware_cleanup(void);

#endif // HARDWARE_H
