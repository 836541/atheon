#include <fltKernel.h>
#include <dontuse.h>
#include "BackupData.h"


const int ListSize = 12;
WCHAR* Extensions[ListSize] = { L"mp3", L"pdf", L"rtf", L"docx", L"pptx", L"mp4", L"exe", L"png", L"rar", L"zip", L"jpeg", L"jpg" };
int MaxSize = 30000;
int Protection = 2;
PFLT_FILTER gFilterHandle;
#define DRIVER_TAG 'PleD'
#define POOL_FLAG_PAGED                   0x0000000000000100UI64

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

FLT_PREOP_CALLBACK_STATUS PreSetInformation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, PVOID*);
FLT_PREOP_CALLBACK_STATUS PreCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, PVOID*);
FLT_PREOP_CALLBACK_STATUS PreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID*);
FLT_PREOP_CALLBACK_STATUS PreRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID*);

NTSTATUS backupFile(UNICODE_STRING sourcePath, UNICODE_STRING destPath, PFLT_INSTANCE FilterInstance, PFLT_FILTER FHandle)
{
    NTSTATUS status;
    HANDLE sourceHandle;
    HANDLE destHandle;
    OBJECT_ATTRIBUTES sourceOA;
    OBJECT_ATTRIBUTES destOA;
    ULONG len;
    PVOID buffer;
    IO_STATUS_BLOCK io_status;
    LARGE_INTEGER offset = { 0 };

    InitializeObjectAttributes(&sourceOA, &sourcePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);
    InitializeObjectAttributes(&destOA, &destPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    status = FltCreateFile(
        FHandle,
        FilterInstance,
        &sourceHandle,
        GENERIC_READ,
        &sourceOA,
        &io_status,
        nullptr,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        nullptr,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = FltCreateFile(
        FHandle,
        FilterInstance,
        &destHandle,
        GENERIC_WRITE,
        &destOA,
        &io_status,
        nullptr,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_CREATE,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK
    );
    if (!NT_SUCCESS(status))
    {
        ZwClose(sourceHandle);
        return status;
    }
    buffer = ExAllocatePool2(POOL_FLAG_PAGED, 4096, DRIVER_TAG);
    if (buffer == nullptr)
    {
        ZwClose(sourceHandle);
        ZwClose(destHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    while (true)
    {
        status = ZwReadFile(
            sourceHandle,
            NULL,
            NULL,
            NULL,
            &io_status,
            buffer,
            PAGE_SIZE,
            &offset,
            NULL
        );
        if (!NT_SUCCESS(status))
        {
            if (STATUS_END_OF_FILE == status)
            {
                status = STATUS_SUCCESS;
            }
            break;
        }
        len = (ULONG)io_status.Information;


        status = ZwWriteFile(
            destHandle,
            NULL,
            NULL,
            NULL,
            &io_status,
            buffer,
            len,
            &offset,
            NULL
        );
        if (!NT_SUCCESS(status))
        {
            break;
        }
        offset.QuadPart += len;

    }
    ExFreePool(buffer);
    ZwClose(sourceHandle);
    ZwClose(destHandle);
    return status;

}

ULONG_PTR OperationStatusCtx = 1;

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

ULONG gTraceFlags = 0;


#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

NTSTATUS
SacerdoteInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
);

VOID
SacerdoteInstanceTeardownStart(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
);

VOID
SacerdoteInstanceTeardownComplete(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
);

NTSTATUS
SacerdoteUnload(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
);

NTSTATUS
SacerdoteInstanceQueryTeardown(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
SacerdotePreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

VOID
SacerdoteOperationStatusCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
);

FLT_POSTOP_CALLBACK_STATUS
SacerdotePostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
SacerdotePreOperationNoPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

BOOLEAN
SacerdoteDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
);

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SacerdoteUnload)
#pragma alloc_text(PAGE, SacerdoteInstanceQueryTeardown)
#pragma alloc_text(PAGE, SacerdoteInstanceSetup)
#pragma alloc_text(PAGE, SacerdoteInstanceTeardownStart)
#pragma alloc_text(PAGE, SacerdoteInstanceTeardownComplete)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, 0, PreCreate, nullptr},
    // { IRP_MJ_SET_INFORMATION, 0, PreSetInformation, nullptr},
    { IRP_MJ_READ, 0, PreRead, nullptr},
    // { IRP_MJ_WRITE, 0, PreWrite, nullptr},
    { IRP_MJ_OPERATION_END }
};


CONST FLT_REGISTRATION FilterRegistration = {

    sizeof(FLT_REGISTRATION),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags
    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks
    SacerdoteUnload,                           //  MiniFilterUnload
    SacerdoteInstanceSetup,                    //  InstanceSetup
    SacerdoteInstanceQueryTeardown,            //  InstanceQueryTeardown
    SacerdoteInstanceTeardownStart,            //  InstanceTeardownStart
    SacerdoteInstanceTeardownComplete,         //  InstanceTeardownComplete
    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent
};



NTSTATUS
SacerdoteInstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    PAGED_CODE();

    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("Sacerdote!SacerdoteInstanceSetup: Entered\n"));

    return STATUS_SUCCESS;
}


NTSTATUS
SacerdoteInstanceQueryTeardown(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("Sacerdote!SacerdoteInstanceQueryTeardown: Entered\n"));

    return STATUS_SUCCESS;
}


VOID
SacerdoteInstanceTeardownStart(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("Sacerdote!SacerdoteInstanceTeardownStart: Entered\n"));
}


VOID
SacerdoteInstanceTeardownComplete(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("Sacerdote!SacerdoteInstanceTeardownComplete: Entered\n"));
}

NTSTATUS SacerdoteOpenClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

void UnloadDriver(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\sacerdote");

    IoDeleteSymbolicLink(&symLink);
    IoDeleteDevice(DriverObject->DeviceObject);
}


NTSTATUS SacerdoteWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    auto status = STATUS_SUCCESS;
    ULONG_PTR information = 0;

    auto IrpSp = IoGetCurrentIrpStackLocation(Irp);

    do {
        if (IrpSp->Parameters.Write.Length < sizeof(BackupData)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        auto data = static_cast<BackupData*>(Irp->UserBuffer);
        if (data == nullptr || 1 > data->Protect || data->Protect > 4) {
            status = STATUS_INVALID_PARAMETER;
            break;
        }

        information = sizeof(data);
        Protection = data->Protect;
        if (Protection != 3 || Protection != 4) {
            MaxSize = data->MaxSize;
            break;
        }

    } while (false);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    NTSTATUS status;
    UNREFERENCED_PARAMETER(RegistryPath);
    KdPrint(("Sacerdote Entry Point!\n"));

    DriverObject->DriverUnload = UnloadDriver;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = SacerdoteOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SacerdoteOpenClose;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = SacerdoteWrite;

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\sacerdote");
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\sacerdote");
    auto symCreated = false;
    PDEVICE_OBJECT DeviceObject;

    do {
        status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
        if (!NT_SUCCESS(status)) {
            KdPrint(("Failed to create Device Object (0x%08X)\n", status));
            break;
        }
        status = IoCreateSymbolicLink(&symLink, &deviceName);
        if (!NT_SUCCESS(status)) {
            KdPrint(("Failed to create Symbolic Link (0x%08X)", status));
            break;
        }
        symCreated = true;
        status = FltRegisterFilter(DriverObject, &FilterRegistration, &gFilterHandle);
        FLT_ASSERT(NT_SUCCESS(status));
        if (!NT_SUCCESS(status)) {
            KdPrint(("Failed to Register Driver to Filter Manager (0x%08X)", status));
            break;
        }
        status = FltStartFiltering(gFilterHandle);
        if (!NT_SUCCESS(status)) {
            KdPrint(("Failed to Start Filtering (0x%08X)", status));
            break;
        }
    } while (false);

    if (!NT_SUCCESS(status)) {
        if (DeviceObject) {
            IoDeleteDevice(DeviceObject);
        }
        if (symCreated) {
            IoDeleteSymbolicLink(&symLink);
        }
        if (gFilterHandle) {
            FltUnregisterFilter(gFilterHandle);
        }
    }
    return status;
}


NTSTATUS
SacerdoteUnload(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("Sacerdote!SacerdoteUnload: Entered\n"));

    FltUnregisterFilter(gFilterHandle);

    return STATUS_SUCCESS;
}



bool isBackupDir(PCUNICODE_STRING Dir) {
    ULONG BufferSize = 1024;
    if (Dir->MaximumLength > BufferSize) {
        return false;
    }

    auto buffer = (WCHAR*)ExAllocatePool2(POOL_FLAG_PAGED, BufferSize + sizeof(WCHAR), DRIVER_TAG);
    if (!buffer) {
        return false;
    }
    RtlZeroMemory(buffer, BufferSize + sizeof(WCHAR));
    wcsncpy_s(buffer, 1 + BufferSize / sizeof(WCHAR), Dir->Buffer, Dir->Length / sizeof(WCHAR));
    _wcslwr(buffer);

    bool checkDir = wcsstr(buffer, L"\\sacerdotebackupdir\\");
    ExFreePool(buffer);

    return checkDir;
}

bool isBackupable(PCUNICODE_STRING extension) {
    ULONG BufferSize = 1024;
    bool istrue = false;
    if (extension->MaximumLength > BufferSize) {
        return false;
    }

    auto buffer = (WCHAR*)ExAllocatePool2(POOL_FLAG_PAGED, BufferSize + sizeof(WCHAR), DRIVER_TAG);
    if (!buffer) {
        return false;
    }
    RtlZeroMemory(buffer, BufferSize + sizeof(WCHAR));
    wcsncpy_s(buffer, 1 + BufferSize / sizeof(WCHAR), extension->Buffer, extension->Length / sizeof(WCHAR));
    _wcslwr(buffer);

    for (int i = 0; i < ListSize; i++) {
        if (Extensions[i] != nullptr) {
            bool checkExt = wcsstr(buffer, Extensions[i]);
            if (checkExt) {
                istrue = true;
                break;
            }
        }
    }
    ExFreePool(buffer);

    return istrue;
}





/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS PreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(CompletionContext);
    NTSTATUS status;
    auto returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
    UNICODE_STRING BackupDir = RTL_CONSTANT_STRING(L"\\??\\C:\\sacerdotebackupdir\\");
    PFLT_FILE_NAME_INFORMATION fileNameInfo;
    POBJECT_NAME_INFORMATION ObjectNameInformation;


    if (Data->RequestorMode == KernelMode) {
        return returnStatus;
    }

    do {
        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &fileNameInfo);
        if (!NT_SUCCESS(status)) {
            break;
        }
        status = FltParseFileNameInformation(fileNameInfo);
        if (!NT_SUCCESS(status)) {
            // KdPrint(("Failed to Parse File Information (0x%08x)\n", status));
            FltReleaseFileNameInformation(fileNameInfo);
            break;
        }
        if (isBackupDir(&fileNameInfo->Name)) {
            if (Protection == 2 || Protection == 4) {
                if (Data->RequestorMode != KernelMode) {
                    KdPrint(("You shall NOT PASS: Sacerdote killed your handle to backupdir\n"));
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    FltReleaseFileNameInformation(fileNameInfo);
                    return FLT_PREOP_COMPLETE;
                }
            }
        }
    } while (false);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

// PreSet não foi utilizado 
FLT_PREOP_CALLBACK_STATUS PreSetInformation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(CompletionContext);
    NTSTATUS status;
    auto returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
    PFLT_FILE_NAME_INFORMATION fileNameInfo;
    POBJECT_NAME_INFORMATION ObjectNameInformation;
    UNICODE_STRING BackupDir = RTL_CONSTANT_STRING(L"\\??\\C:\\sacerdotebackupdir\\");

    if (Data->RequestorMode == KernelMode) {
        return returnStatus;
    }

    if (!MaxSize) {
        KdPrint(("Backup isnt working because User didnt input MaxSizetoBackup yet\n"));
        return returnStatus;
    }

    do {
        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &fileNameInfo);
        if (!NT_SUCCESS(status)) {
            // KdPrint(("Failed to Retrieve File Information (0x%08X)\n", status));
            break;
        }
        status = FltParseFileNameInformation(fileNameInfo);
        if (!NT_SUCCESS(status)) {
            FltReleaseFileNameInformation(fileNameInfo);
            // KdPrint(("Failed to Parse File Information (0x%08x)\n", status));
            break;
        }

        if (!isBackupable(&fileNameInfo->Name)) {
            FltReleaseFileNameInformation(fileNameInfo);
            return FLT_PREOP_SUCCESS_NO_CALLBACK;
        }

        auto& params = Data->Iopb->Parameters.SetFileInformation;
        if (params.FileInformationClass != FileDispositionInformation && params.FileInformationClass != FileDispositionInformationEx) {
            FltReleaseFileNameInformation(fileNameInfo);
            break;
        }
        auto info = (FILE_DISPOSITION_INFORMATION*)params.InfoBuffer;
        if (info->DeleteFile) {
            IoQueryFileDosDeviceName(Data->Iopb->TargetFileObject, &ObjectNameInformation);
            if (!ObjectNameInformation) {
                FltReleaseFileNameInformation(fileNameInfo);
                // KdPrint(("Failed to Retrieve Disk Symlink \n"));
                break;
            }
            auto dosName = ObjectNameInformation->Name;
            auto NameLength = (USHORT)dosName.MaximumLength + Data->Iopb->TargetFileObject->FileName.MaximumLength + 2;
            auto NameBuffer = ExAllocatePool2(POOL_FLAG_PAGED, NameLength, DRIVER_TAG);
            if (!NameBuffer) {
                FltReleaseFileNameInformation(fileNameInfo);
                ExFreePool(ObjectNameInformation);
                KdPrint(("Failed to Allocate Memory for FileDir\n"));
                break;
            }
            UNICODE_STRING NameString;
            NameString.Length = 0;
            NameString.MaximumLength = (USHORT)NameLength;
            NameString.Buffer = (PWCH)NameBuffer;

            UNICODE_STRING symString = RTL_CONSTANT_STRING(L"\\??\\");
            RtlCopyUnicodeString(&NameString, &symString);
            RtlAppendUnicodeStringToString(&NameString, &ObjectNameInformation->Name);

            auto destNameLength = (USHORT)dosName.MaximumLength + Data->Iopb->TargetFileObject->FileName.MaximumLength + 2;
            auto destNameBuffer = ExAllocatePool2(POOL_FLAG_PAGED, destNameLength, DRIVER_TAG);
            if (!destNameBuffer) {
                FltReleaseFileNameInformation(fileNameInfo);
                ExFreePool(ObjectNameInformation);
                ExFreePool(NameBuffer);
                KdPrint(("Failed to Allocate for Destination Dir at PreSet\n"));
                break;
            }
            UNICODE_STRING destNameString;
            destNameString.Length = 0;
            destNameString.MaximumLength = (USHORT)destNameLength;
            destNameString.Buffer = (PWCH)destNameBuffer;
            RtlCopyUnicodeString(&destNameString, &BackupDir);
            RtlAppendUnicodeStringToString(&destNameString, &fileNameInfo->FinalComponent);

            status = backupFile(NameString, destNameString, FltObjects->Instance, FltObjects->Filter);
            if (!NT_SUCCESS(status)) {
                KdPrint(("Failed to Backup at PreSetInfo (0x%08X)\n", status));
                FltReleaseFileNameInformation(fileNameInfo);
                ExFreePool(ObjectNameInformation);
                ExFreePool(NameBuffer);
                ExFreePool(destNameBuffer);
                break;
            }
            KdPrint(("Success backup SET_INFO: %wZ \n", &NameString));
            FltReleaseFileNameInformation(fileNameInfo);
            ExFreePool(ObjectNameInformation);
            ExFreePool(NameBuffer);
            ExFreePool(destNameBuffer);
        }
    } while (false);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
FLT_PREOP_CALLBACK_STATUS PreRead(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(CompletionContext);
    NTSTATUS status;
    auto returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
    PFLT_FILE_NAME_INFORMATION fileNameInfo;
    POBJECT_NAME_INFORMATION ObjectNameInformation;
    UNICODE_STRING BackupDir = RTL_CONSTANT_STRING(L"\\??\\C:\\sacerdotebackupdir\\");

    if (Data->RequestorMode == KernelMode) {
        return returnStatus;
    }

    if (!MaxSize) {
        KdPrint(("Please input a Max Size via Client.exe\n"));
    }

    do {
        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &fileNameInfo);
        if (!NT_SUCCESS(status)) {
            // KdPrint(("Failed to Retrieve File Information (0x%08X)\n", status));
            break;
        }
        status = FltParseFileNameInformation(fileNameInfo);
        if (!NT_SUCCESS(status)) {
            FltReleaseFileNameInformation(fileNameInfo);
            // KdPrint(("Failed to Parse File Information (0x%08x)\n", status));
            break;
        }

        if (!isBackupable(&fileNameInfo->Extension)) {
            FltReleaseFileNameInformation(fileNameInfo);
            return FLT_PREOP_SUCCESS_NO_CALLBACK;
        }

        IoQueryFileDosDeviceName(Data->Iopb->TargetFileObject, &ObjectNameInformation);
        if (!ObjectNameInformation) {
            FltReleaseFileNameInformation(fileNameInfo);
            // KdPrint(("Failed to Retrieve Disk Symlink \n"));
            break;
        }
        auto dosName = ObjectNameInformation->Name;
        auto NameLength = (USHORT)dosName.MaximumLength + Data->Iopb->TargetFileObject->FileName.MaximumLength + 2;
        auto NameBuffer = ExAllocatePool2(POOL_FLAG_PAGED, NameLength, DRIVER_TAG);
        if (!NameBuffer) {
            FltReleaseFileNameInformation(fileNameInfo);
            ExFreePool(ObjectNameInformation);
            KdPrint(("Failed to Allocate Memory for FileDir\n"));
            break;
        }
        UNICODE_STRING NameString;
        NameString.Length = 0;
        NameString.MaximumLength = (USHORT)NameLength;
        NameString.Buffer = (PWCH)NameBuffer;

        UNICODE_STRING symString = RTL_CONSTANT_STRING(L"\\??\\");
        RtlCopyUnicodeString(&NameString, &symString);
        RtlAppendUnicodeStringToString(&NameString, &ObjectNameInformation->Name);

        auto destNameLength = (USHORT)dosName.MaximumLength + Data->Iopb->TargetFileObject->FileName.MaximumLength + 2;
        auto destNameBuffer = ExAllocatePool2(POOL_FLAG_PAGED, destNameLength, DRIVER_TAG);
        if (!destNameBuffer) {
            FltReleaseFileNameInformation(fileNameInfo);
            ExFreePool(ObjectNameInformation);
            ExFreePool(NameBuffer);
            KdPrint(("Failed to Allocate for Destination Dir at PreRead\n"));
            break;
        }
        UNICODE_STRING destNameString;
        destNameString.Length = 0;
        destNameString.MaximumLength = (USHORT)destNameLength;
        destNameString.Buffer = (PWCH)destNameBuffer;
        RtlCopyUnicodeString(&destNameString, &BackupDir);
        RtlAppendUnicodeStringToString(&destNameString, &fileNameInfo->FinalComponent);

        status = backupFile(NameString, destNameString, FltObjects->Instance, FltObjects->Filter);
        if (!NT_SUCCESS(status)) {
            // KdPrint(("Failed to Backup at PreRead (0x%08X)\n", status));
            FltReleaseFileNameInformation(fileNameInfo);
            ExFreePool(ObjectNameInformation);
            ExFreePool(NameBuffer);
            ExFreePool(destNameBuffer);
            break;
        }
        KdPrint(("Success backup PRE_READ: %wZ \n", &NameString));
        FltReleaseFileNameInformation(fileNameInfo);
        ExFreePool(ObjectNameInformation);
        ExFreePool(NameBuffer);
        ExFreePool(destNameBuffer);
    } while (false);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

// Pre Write foi removido nessa versão
FLT_PREOP_CALLBACK_STATUS PreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(CompletionContext);
    NTSTATUS status;
    auto returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
    PFLT_FILE_NAME_INFORMATION fileNameInfo;
    POBJECT_NAME_INFORMATION ObjectNameInformation;
    UNICODE_STRING BackupDir = RTL_CONSTANT_STRING(L"\\??\\C:\\sacerdotebackupdir\\");

    if (Data->RequestorMode == KernelMode) {
        return returnStatus;
    }

    if (!MaxSize) {
        KdPrint(("Please input a Max Size via Client.exe\n"));
    }

    LARGE_INTEGER fileSize;
    LONGLONG a = 5;
    LONGLONG sizepart;
    status = FsRtlGetFileSize(FltObjects->FileObject, &fileSize);
    if (!NT_SUCCESS(status)) {
        return returnStatus;
    }

    sizepart = fileSize.QuadPart / 5;
    ULONG fileSizePlus = fileSize.QuadPart + sizepart;
    ULONG fileSizeMinus = fileSize.QuadPart - sizepart;

    auto& params = Data->Iopb->Parameters.Write;
    auto WriteLength = params.Length;
    KdPrint(("%lu,%lu,%lu,%lu\n", WriteLength, fileSize.QuadPart, fileSizePlus, fileSizeMinus));
    //if (fileSizeMinus > WriteLength || WriteLength > fileSizePlus) { 
    //   return returnStatus;
    //}

    do {
        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &fileNameInfo);
        if (!NT_SUCCESS(status)) {
            // KdPrint(("Failed to Retrieve File Information (0x%08X)\n", status));
            break;
        }
        status = FltParseFileNameInformation(fileNameInfo);
        if (!NT_SUCCESS(status)) {
            FltReleaseFileNameInformation(fileNameInfo);
            // KdPrint(("Failed to Parse File Information (0x%08x)\n", status));
            break;
        }

        //if (!isBackupable(&fileNameInfo->Name)) {
        //    FltReleaseFileNameInformation(fileNameInfo);
        //    return FLT_PREOP_SUCCESS_NO_CALLBACK;
        //}

        IoQueryFileDosDeviceName(Data->Iopb->TargetFileObject, &ObjectNameInformation);
        if (!ObjectNameInformation) {
            FltReleaseFileNameInformation(fileNameInfo);
            // KdPrint(("Failed to Retrieve Disk Symlink \n"));
            break;
        }
        auto dosName = ObjectNameInformation->Name;
        auto NameLength = (USHORT)dosName.MaximumLength + Data->Iopb->TargetFileObject->FileName.MaximumLength + 2;
        auto NameBuffer = ExAllocatePool2(POOL_FLAG_PAGED, NameLength, DRIVER_TAG);
        if (!NameBuffer) {
            FltReleaseFileNameInformation(fileNameInfo);
            ExFreePool(ObjectNameInformation);
            KdPrint(("Failed to Allocate Memory for FileDir\n"));
            break;
        }
        UNICODE_STRING NameString;
        NameString.Length = 0;
        NameString.MaximumLength = (USHORT)NameLength;
        NameString.Buffer = (PWCH)NameBuffer;

        UNICODE_STRING symString = RTL_CONSTANT_STRING(L"\\??\\");
        RtlCopyUnicodeString(&NameString, &symString);
        RtlAppendUnicodeStringToString(&NameString, &ObjectNameInformation->Name);

        auto destNameLength = (USHORT)dosName.MaximumLength + Data->Iopb->TargetFileObject->FileName.MaximumLength + 2;
        auto destNameBuffer = ExAllocatePool2(POOL_FLAG_PAGED, destNameLength, DRIVER_TAG);
        if (!destNameBuffer) {
            FltReleaseFileNameInformation(fileNameInfo);
            ExFreePool(ObjectNameInformation);
            ExFreePool(NameBuffer);
            KdPrint(("Failed to Allocate for Destination Dir at PreSet\n"));
            break;
        }
        UNICODE_STRING destNameString;
        destNameString.Length = 0;
        destNameString.MaximumLength = (USHORT)destNameLength;
        destNameString.Buffer = (PWCH)destNameBuffer;
        RtlCopyUnicodeString(&destNameString, &BackupDir);
        RtlAppendUnicodeStringToString(&destNameString, &fileNameInfo->FinalComponent);

        status = backupFile(NameString, destNameString, FltObjects->Instance, FltObjects->Filter);
        if (!NT_SUCCESS(status)) {
            // KdPrint(("Failed to Backup at PreWrite (0x%08X)\n", status));
            FltReleaseFileNameInformation(fileNameInfo);
            ExFreePool(ObjectNameInformation);
            ExFreePool(NameBuffer);
            ExFreePool(destNameBuffer);
            break;
        }
        // KdPrint(("Success backup PRE_WRITE: %wZ \n", &NameString));
        FltReleaseFileNameInformation(fileNameInfo);
        ExFreePool(ObjectNameInformation);
        ExFreePool(NameBuffer);
        ExFreePool(destNameBuffer);
    } while (false);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}