#include "../src/hardware.h"
#include <stdio.h>
#include <windows.h>

int main(void) {
    printf("[TEST] Inicializando modulo de hardware...\n");
    hardware_init();

    // Dorme 300ms para calcular delta de CPU
    Sleep(300);

    HardwareMetrics m;
    hardware_update(&m);

    printf("[RESULTADO]\n");
    printf("CPU Usage: %d%%\n", m.cpu_usage);
    printf("RAM Usage: %d%% (%d MB / %d MB)\n", m.ram_usage, m.ram_used_mb, m.ram_total_mb);
    printf("GPU Usage: %d%%\n", m.gpu_usage);
    printf("GPU Temp : %d C\n", m.gpu_temp);
    printf("VRAM     : %d MB / %d MB\n", m.vram_used_mb, m.vram_total_mb);

    hardware_cleanup();

    if (m.ram_usage > 0 && m.ram_total_mb > 0) {
        printf("[SUCESSO] Teste de hardware passou com exito!\n");
        return 0;
    }

    printf("[FALHA] Falha ao coletar RAM\n");
    return 1;
}
