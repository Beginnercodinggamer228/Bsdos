#include <ntddk.h>
#include <wdm.h>
#include "bsod_driver.h"

#define DEVICE_NAME L"\\Device\\BSODDriver"
#define SYMLINK_NAME L"\\DosDevices\\BSODDriver"

// Функция для вызова BSOD
NTSTATUS TriggerBSOD(ULONG errorCode, ULONG_PTR param1, ULONG_PTR param2, ULONG_PTR param3, ULONG_PTR param4) {
    DbgPrint("BSOD Driver: Triggering BSOD with code 0x%X\n", errorCode);
    
    // Вызываем BSOD
    KeBugCheckEx(errorCode, param1, param2, param3, param4);
    
    return STATUS_SUCCESS;
}

// Обработка IOCTL запросов
NTSTATUS DeviceIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    PVOID inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    
    DbgPrint("BSOD Driver: IOCTL received: 0x%X\n", ioControlCode);
    
    switch (ioControlCode) {
        case IOCTL_BSOD_TRIGGER:
            if (inputBufferLength >= sizeof(BSOD_DATA)) {
                PBSOD_DATA bsodData = (PBSOD_DATA)inputBuffer;
                DbgPrint("BSOD Driver: Triggering BSOD with code: 0x%X\n", bsodData->ErrorCode);
                
                // Вызываем BSOD с переданными параметрами
                TriggerBSOD(
                    bsodData->ErrorCode,
                    bsodData->Param1,
                    bsodData->Param2,
                    bsodData->Param3,
                    bsodData->Param4
                );
            }
            break;
            
        case IOCTL_GET_STATUS:
            // Просто возвращаем статус
            break;
            
        default:
            return STATUS_INVALID_DEVICE_REQUEST;
    }
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// Создание устройства
NTSTATUS CreateDevice(PDRIVER_OBJECT DriverObject) {
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject = NULL;
    UNICODE_STRING deviceName, symlinkName;
    
    RtlInitUnicodeString(&deviceName, DEVICE_NAME);
    RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
    
    // Создаём устройство
    status = IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_EXTENSION),
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject
    );
    
    if (!NT_SUCCESS(status)) {
        DbgPrint("BSOD Driver: Failed to create device: 0x%X\n", status);
        return status;
    }
    
    // Создаём символьную ссылку
    status = IoCreateSymbolicLink(&symlinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        DbgPrint("BSOD Driver: Failed to create symlink: 0x%X\n", status);
        return status;
    }
    
    // Настраиваем устройство
    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    
    DbgPrint("BSOD Driver: Device created successfully\n");
    return STATUS_SUCCESS;
}

// Удаление устройства
VOID DeleteDevice(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symlinkName;
    RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
    IoDeleteSymbolicLink(&symlinkName);
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("BSOD Driver: Device deleted\n");
}

// Точка входа драйвера
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    NTSTATUS status;
    
    DbgPrint("BSOD Driver: Loading...\n");
    
    // Настраиваем обработчики
    DriverObject->DriverUnload = DeleteDevice;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceIoControl;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIoControl;
    
    // Создаём устройство
    status = CreateDevice(DriverObject);
    if (!NT_SUCCESS(status)) {
        DbgPrint("BSOD Driver: Failed to create device\n");
        return status;
    }
    
    DbgPrint("BSOD Driver: Loaded successfully!\n");
    return STATUS_SUCCESS;
}