#ifndef BSOD_DRIVER_H
#define BSOD_DRIVER_H

// Типы BSOD ошибок
#define BSOD_CRITICAL_PROCESS_DIED    0xEF
#define BSOD_SYSTEM_SERVICE_EXCEPTION 0x3B
#define BSOD_PAGE_FAULT_IN_NONPAGED   0x50
#define BSOD_IRQL_NOT_LESS_OR_EQUAL   0x0A
#define BSOD_KERNEL_DATA_INPAGE       0x7A
#define BSOD_CUSTOM_ERROR             0xDEAD

// Структура для передачи данных в драйвер
typedef struct _BSOD_DATA {
    ULONG ErrorCode;
    ULONG_PTR Param1;
    ULONG_PTR Param2;
    ULONG_PTR Param3;
    ULONG_PTR Param4;
} BSOD_DATA, *PBSOD_DATA;

// IOCTL коды
#define IOCTL_BSOD_TRIGGER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_STATUS   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Расширение устройства
typedef struct _DEVICE_EXTENSION {
    ULONG Status;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#endif