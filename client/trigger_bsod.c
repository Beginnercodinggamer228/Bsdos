#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

#define DEVICE_NAME L"\\\\.\\BSODDriver"

typedef struct _BSOD_DATA {
    DWORD ErrorCode;
    ULONG_PTR Param1;
    ULONG_PTR Param2;
    ULONG_PTR Param3;
    ULONG_PTR Param4;
} BSOD_DATA, *PBSOD_DATA;

#define IOCTL_BSOD_TRIGGER CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

int main(int argc, char* argv[]) {
    HANDLE hDevice;
    BSOD_DATA bsodData;
    DWORD bytesReturned;
    
    printf("=== BSOD Trigger Tool ===\n");
    
    if (argc < 2) {
        printf("Usage: %s <error_code> [param1] [param2] [param3] [param4]\n", argv[0]);
        printf("Example: %s 0xDEADBEEF\n", argv[0]);
        return 1;
    }
    
    hDevice = CreateFile(
        DEVICE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device. Error: %d\n", GetLastError());
        printf("Make sure driver is loaded!\n");
        return 1;
    }

    bsodData.ErrorCode = strtoul(argv[1], NULL, 0);
    bsodData.Param1 = argc > 2 ? strtoull(argv[2], NULL, 0) : 0;
    bsodData.Param2 = argc > 3 ? strtoull(argv[3], NULL, 0) : 0;
    bsodData.Param3 = argc > 4 ? strtoull(argv[4], NULL, 0) : 0;
    bsodData.Param4 = argc > 5 ? strtoull(argv[5], NULL, 0) : 0;
    
    printf("Triggering BSOD with code: 0x%X\n", bsodData.ErrorCode);
    printf("This will crash your system!\n");

    if (!DeviceIoControl(
        hDevice,
        IOCTL_BSOD_TRIGGER,
        &bsodData,
        sizeof(BSOD_DATA),
        NULL,
        0,
        &bytesReturned,
        NULL
    )) {
        printf("Failed to send IOCTL. Error: %d\n", GetLastError());
        CloseHandle(hDevice);
        return 1;
    }
    
    CloseHandle(hDevice);
    return 0;
}