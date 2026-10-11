#include "hardware.h"
#include "nvml_lite.h"
#include <stdio.h>

// Ponteiros dinâmicos para kernel32
typedef BOOL (WINAPI *pfn_GetSystemTimes)(LPFILETIME, LPFILETIME, LPFILETIME);
typedef BOOL (WINAPI *pfn_GlobalMemoryStatusEx)(LPMEMORYSTATUSEX);

static pfn_GetSystemTimes fn_GetSystemTimes = NULL;
static pfn_GlobalMemoryStatusEx fn_GlobalMemoryStatusEx = NULL;

// Estado estático para cálculo de CPU
static ULONGLONG prev_idle_time = 0;
static ULONGLONG prev_kernel_time = 0;
static ULONGLONG prev_user_time = 0;
static BOOL cpu_calibrated = FALSE;

// Estado dinâmico do NVML (NVIDIA)
static HMODULE hNvml = NULL;
static nvmlDevice_t gpu_device = NULL;
static pfn_nvmlInit fn_nvmlInit = NULL;
static pfn_nvmlShutdown fn_nvmlShutdown = NULL;
static pfn_nvmlDeviceGetHandleByIndex fn_nvmlDeviceGetHandleByIndex = NULL;
static pfn_nvmlDeviceGetUtilizationRates fn_nvmlDeviceGetUtilizationRates = NULL;
static pfn_nvmlDeviceGetTemperature fn_nvmlDeviceGetTemperature = NULL;
static pfn_nvmlDeviceGetMemoryInfo fn_nvmlDeviceGetMemoryInfo = NULL;
static BOOL nvml_ready = FALSE;

// Estruturas e ponteiros dinâmicos para ICMP (Ping de Rede)
typedef struct {
    unsigned long Address;
    unsigned long Status;
    unsigned long RoundTripTime;
    unsigned short DataSize;
    unsigned short Reserved;
    void *Data;
    struct {
        unsigned char Ttl;
        unsigned char Tos;
        unsigned char Flags;
        unsigned char OptionsSize;
        unsigned char *OptionsData;
    } Options;
} ICMP_ECHO_REPLY;

typedef HANDLE (WINAPI *pfn_IcmpCreateFile)(VOID);
typedef BOOL   (WINAPI *pfn_IcmpCloseHandle)(HANDLE);
typedef DWORD  (WINAPI *pfn_IcmpSendEcho)(HANDLE, unsigned long, LPVOID, WORD, LPVOID, LPVOID, DWORD, DWORD);

static HMODULE hIcmp = NULL;
static pfn_IcmpCreateFile fn_IcmpCreateFile = NULL;
static pfn_IcmpCloseHandle fn_IcmpCloseHandle = NULL;
static pfn_IcmpSendEcho fn_IcmpSendEcho = NULL;

static volatile BOOL g_pingRunning = TRUE;
static HANDLE g_hPingThread = NULL;
static volatile int g_currentPing = 0;

static DWORD WINAPI PingBackgroundThread(LPVOID lpParam) {
    if (!fn_IcmpCreateFile || !fn_IcmpSendEcho || !fn_IcmpCloseHandle) return 0;

    HANDLE hFile = fn_IcmpCreateFile();
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    char sendData[32] = "GHOSTHUD_PING";
    char replyBuf[sizeof(ICMP_ECHO_REPLY) + 64];

    // 1.1.1.1 (Cloudflare DNS - menor latência global) em ordem de bytes de rede
    unsigned long targetIp = 0x01010101; 

    while (g_pingRunning) {
        DWORD replies = fn_IcmpSendEcho(hFile, targetIp, sendData, (WORD)sizeof(sendData), 
                                        NULL, replyBuf, (DWORD)sizeof(replyBuf), 800);
        if (replies > 0) {
            ICMP_ECHO_REPLY *reply = (ICMP_ECHO_REPLY *)replyBuf;
            if (reply->Status == 0) {
                g_currentPing = (int)reply->RoundTripTime;
            }
        }
        Sleep(1500); // Mede ping a cada 1.5s em segundo plano (0% lag na interface)
    }

    fn_IcmpCloseHandle(hFile);
    return 0;
}

static ULONGLONG file_time_to_ull(const FILETIME *ft) {
    ULARGE_INTEGER uli;
    uli.LowPart = ft->dwLowDateTime;
    uli.HighPart = ft->dwLowDateTime ? ft->dwHighDateTime : ft->dwHighDateTime;
    uli.LowPart = ft->dwLowDateTime;
    uli.HighPart = ft->dwHighDateTime;
    return uli.QuadPart;
}

void hardware_init(void) {
    // Carrega funções do kernel32
    HMODULE hKernel = GetModuleHandleA("kernel32.dll");
    if (hKernel) {
        fn_GetSystemTimes = (pfn_GetSystemTimes)GetProcAddress(hKernel, "GetSystemTimes");
        fn_GlobalMemoryStatusEx = (pfn_GlobalMemoryStatusEx)GetProcAddress(hKernel, "GlobalMemoryStatusEx");
    }

    // Calibração inicial de CPU
    if (fn_GetSystemTimes) {
        FILETIME idle, kernel, user;
        if (fn_GetSystemTimes(&idle, &kernel, &user)) {
            prev_idle_time = file_time_to_ull(&idle);
            prev_kernel_time = file_time_to_ull(&kernel);
            prev_user_time = file_time_to_ull(&user);
            cpu_calibrated = TRUE;
        }
    }

    // Carregamento dinâmico da nvml.dll
    hNvml = LoadLibraryA("nvml.dll");
    if (!hNvml) {
        hNvml = LoadLibraryA("C:\\Windows\\System32\\nvml.dll");
    }

    if (hNvml) {
        fn_nvmlInit = (pfn_nvmlInit)GetProcAddress(hNvml, "nvmlInit_v2");
        if (!fn_nvmlInit) fn_nvmlInit = (pfn_nvmlInit)GetProcAddress(hNvml, "nvmlInit");

        fn_nvmlShutdown = (pfn_nvmlShutdown)GetProcAddress(hNvml, "nvmlShutdown");

        fn_nvmlDeviceGetHandleByIndex = (pfn_nvmlDeviceGetHandleByIndex)GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex_v2");
        if (!fn_nvmlDeviceGetHandleByIndex) fn_nvmlDeviceGetHandleByIndex = (pfn_nvmlDeviceGetHandleByIndex)GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex");

        fn_nvmlDeviceGetUtilizationRates = (pfn_nvmlDeviceGetUtilizationRates)GetProcAddress(hNvml, "nvmlDeviceGetUtilizationRates");
        fn_nvmlDeviceGetTemperature = (pfn_nvmlDeviceGetTemperature)GetProcAddress(hNvml, "nvmlDeviceGetTemperature");
        fn_nvmlDeviceGetMemoryInfo = (pfn_nvmlDeviceGetMemoryInfo)GetProcAddress(hNvml, "nvmlDeviceGetMemoryInfo");

        if (fn_nvmlInit && fn_nvmlDeviceGetHandleByIndex && fn_nvmlDeviceGetUtilizationRates && fn_nvmlDeviceGetTemperature) {
            if (fn_nvmlInit() == NVML_SUCCESS) {
                if (fn_nvmlDeviceGetHandleByIndex(0, &gpu_device) == NVML_SUCCESS) {
                    nvml_ready = TRUE;
                }
            }
        }
    }

    // Carregamento dinâmico de ICMP para medição de Ping
    hIcmp = LoadLibraryA("icmp.dll");
    if (!hIcmp) {
        hIcmp = LoadLibraryA("iphlpapi.dll");
    }
    if (hIcmp) {
        fn_IcmpCreateFile = (pfn_IcmpCreateFile)GetProcAddress(hIcmp, "IcmpCreateFile");
        fn_IcmpCloseHandle = (pfn_IcmpCloseHandle)GetProcAddress(hIcmp, "IcmpCloseHandle");
        fn_IcmpSendEcho = (pfn_IcmpSendEcho)GetProcAddress(hIcmp, "IcmpSendEcho");

        if (fn_IcmpCreateFile && fn_IcmpSendEcho && fn_IcmpCloseHandle) {
            g_pingRunning = TRUE;
            g_hPingThread = CreateThread(NULL, 0, PingBackgroundThread, NULL, 0, NULL);
        }
    }
}

void hardware_update(HardwareMetrics *out) {
    if (!out) return;

    // 1. Cálculo de uso de CPU
    if (fn_GetSystemTimes) {
        FILETIME idle, kernel, user;
        if (fn_GetSystemTimes(&idle, &kernel, &user)) {
            ULONGLONG current_idle = file_time_to_ull(&idle);
            ULONGLONG current_kernel = file_time_to_ull(&kernel);
            ULONGLONG current_user = file_time_to_ull(&user);

            if (cpu_calibrated) {
                ULONGLONG idle_diff = current_idle - prev_idle_time;
                ULONGLONG kernel_diff = current_kernel - prev_kernel_time;
                ULONGLONG user_diff = current_user - prev_user_time;
                ULONGLONG total_sys = kernel_diff + user_diff;

                if (total_sys > 0) {
                    ULONGLONG busy = (total_sys > idle_diff) ? (total_sys - idle_diff) : 0;
                    out->cpu_usage = (int)((busy * 100) / total_sys);
                    if (out->cpu_usage > 100) out->cpu_usage = 100;
                    if (out->cpu_usage < 0) out->cpu_usage = 0;
                } else {
                    out->cpu_usage = 0;
                }
            } else {
                out->cpu_usage = 0;
                cpu_calibrated = TRUE;
            }

            prev_idle_time = current_idle;
            prev_kernel_time = current_kernel;
            prev_user_time = current_user;
        }
    } else {
        out->cpu_usage = 0;
    }

    // 2. Cálculo de uso de RAM
    if (fn_GlobalMemoryStatusEx) {
        MEMORYSTATUSEX mem;
        mem.dwLength = sizeof(MEMORYSTATUSEX);
        if (fn_GlobalMemoryStatusEx(&mem)) {
            out->ram_usage = (int)mem.dwMemoryLoad;
            out->ram_total_mb = (int)(mem.ullTotalPhys / (1024 * 1024));
            out->ram_used_mb = (int)((mem.ullTotalPhys - mem.ullAvailPhys) / (1024 * 1024));
        } else {
            out->ram_usage = 0;
            out->ram_used_mb = 0;
            out->ram_total_mb = 0;
        }
    } else {
        MEMORYSTATUS ms;
        ms.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus(&ms);
        out->ram_usage = (int)ms.dwMemoryLoad;
        out->ram_total_mb = (int)(ms.dwTotalPhys / (1024 * 1024));
        out->ram_used_mb = (int)((ms.dwTotalPhys - ms.dwAvailPhys) / (1024 * 1024));
    }

    // 3. Cálculo de GPU e VRAM via NVML
    if (nvml_ready && gpu_device) {
        nvmlUtilization_t util;
        if (fn_nvmlDeviceGetUtilizationRates(gpu_device, &util) == NVML_SUCCESS) {
            out->gpu_usage = (int)util.gpu;
        } else {
            out->gpu_usage = 0;
        }

        unsigned int temp = 0;
        if (fn_nvmlDeviceGetTemperature(gpu_device, NVML_TEMPERATURE_GPU, &temp) == NVML_SUCCESS) {
            out->gpu_temp = (int)temp;
        } else {
            out->gpu_temp = 0;
        }

        if (fn_nvmlDeviceGetMemoryInfo) {
            nvmlMemory_t vram;
            if (fn_nvmlDeviceGetMemoryInfo(gpu_device, &vram) == NVML_SUCCESS) {
                out->vram_used_mb = (int)(vram.used / (1024 * 1024));
                out->vram_total_mb = (int)(vram.total / (1024 * 1024));
                if (out->vram_total_mb > 0) {
                    out->vram_usage = (int)((vram.used * 100) / vram.total);
                } else {
                    out->vram_usage = 0;
                }
            }
        }
    } else {
        out->gpu_usage = 0;
        out->gpu_temp = 0;
        out->vram_used_mb = 0;
        out->vram_total_mb = 0;
        out->vram_usage = 0;
    }

    // 4. Ping em tempo real da thread de background
    out->ping_ms = g_currentPing;
}

void hardware_cleanup(void) {
    g_pingRunning = FALSE;
    if (g_hPingThread) {
        WaitForSingleObject(g_hPingThread, 200);
        CloseHandle(g_hPingThread);
        g_hPingThread = NULL;
    }
    if (hIcmp) {
        FreeLibrary(hIcmp);
        hIcmp = NULL;
    }
    if (nvml_ready && fn_nvmlShutdown) {
        fn_nvmlShutdown();
        nvml_ready = FALSE;
    }
    if (hNvml) {
        FreeLibrary(hNvml);
        hNvml = NULL;
    }
}
