#ifndef NVML_LITE_H
#define NVML_LITE_H

#include <windows.h>

#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

typedef int nvmlReturn_t;
typedef void* nvmlDevice_t;

typedef struct {
    unsigned int gpu;
    unsigned int memory;
} nvmlUtilization_t;

typedef struct {
    unsigned long long total;
    unsigned long long free;
    unsigned long long used;
} nvmlMemory_t;

typedef nvmlReturn_t (*pfn_nvmlInit)(void);
typedef nvmlReturn_t (*pfn_nvmlShutdown)(void);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetHandleByIndex)(unsigned int index, nvmlDevice_t *device);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetUtilizationRates)(nvmlDevice_t device, nvmlUtilization_t *utilization);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetTemperature)(nvmlDevice_t device, int sensorType, unsigned int *temp);
typedef nvmlReturn_t (*pfn_nvmlDeviceGetMemoryInfo)(nvmlDevice_t device, nvmlMemory_t *memory);

#endif // NVML_LITE_H
