#ifndef ADL_LITE_H
#define ADL_LITE_H

#include <windows.h>

#define ADL_OK 0

typedef void* (*ADL_MAIN_MALLOC_CALLBACK)(int);

typedef struct {
    int iSize;
    int iEngineClock;
    int iMemoryClock;
    int iVddc;
    int iActivityPercent;   // 0 a 100% de uso da GPU
    int iCurrentPerformanceLevel;
    int iCurrentBusSpeed;
    int iCurrentBusLanes;
    int iMaximumBusLanes;
    int iReserved;
} ADLPMActivity;

typedef struct {
    int iSize;
    int iTemperature;       // milésimos de grau Celsius (ex: 62000 = 62°C)
} ADLTemperature;

typedef struct {
    int iAdapterIndex;
    char strUDID[256];
    int iBusNumber;
    int iDeviceNumber;
    int iFunctionNumber;
    int iVendorID;
    char strAdapterName[256];
    char strDisplayName[256];
    int iPresent;
    int iExist;
    char strDriverPath[256];
    char strDriverPathExt[256];
    char strPNPString[256];
    int iOSDisplayIndex;
} ADLAdapterInfo;

typedef int (*pfn_ADL_Main_Control_Create)(ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters);
typedef int (*pfn_ADL_Main_Control_Destroy)(void);
typedef int (*pfn_ADL_Adapter_NumberOfAdapters_Get)(int *numAdapters);
typedef int (*pfn_ADL_Adapter_AdapterInfo_Get)(ADLAdapterInfo *info, int size);
typedef int (*pfn_ADL_Overdrive5_Temperature_Get)(int iAdapterIndex, int iThermalControllerIndex, ADLTemperature *lpTemperature);
typedef int (*pfn_ADL_Overdrive5_CurrentActivity_Get)(int iAdapterIndex, ADLPMActivity *lpActivity);
typedef int (*pfn_ADL_Overdrive6_CurrentPower_Get)(int iAdapterIndex, int iCurrentType, int *iValue);

#endif // ADL_LITE_H
