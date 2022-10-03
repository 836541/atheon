
#include <fltKernel.h>
#include <dontuse.h>

#define DRIVER_TAG 'deaD'
#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

extern "C" NTSTATUS ZwTerminateProcess(HANDLE ProcessHandle, NTSTATUS ExitStatus);
NTSTATUS ObOpenObjectByPointer(
    _In_ PVOID Object,                        // OBJETO
    _In_ ULONG HandleAttributes,              // FLAGS -> use OBJ_KERNEL_HANDLE para validacao em todos context e uso exclusivo pelo kernel
    _In_opt_ PACCESS_STATE PassedAccessState, // NULL
    _In_ ACCESS_MASK DesiredAccess,           // 0, pois em kernelmode VC TEM O ACESSO FULL
    _In_opt_ POBJECT_TYPE ObjectType,         // Forcar um check pra ver se o objeto passado corresponde ao tipo designado
    _In_ KPROCESSOR_MODE AccessMode,          // KernelMode
    _Out_ PHANDLE Handle);                    // HANDLE OUT
FLT_PREOP_CALLBACK_STATUS PreSet(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, PVOID*);
FLT_PREOP_CALLBACK_STATUS PreCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, PVOID*);
FLT_PREOP_CALLBACK_STATUS PreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID*);
PFLT_FILTER gFilterHandle;
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
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
behelitInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
behelitInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
behelitInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
behelitUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
behelitInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
behelitPreOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

VOID
behelitOperationStatusCallback (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
    );

FLT_POSTOP_CALLBACK_STATUS
behelitPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
behelitPreOperationNoPostOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

BOOLEAN
behelitDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
    );

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, behelitUnload)
#pragma alloc_text(PAGE, behelitInstanceQueryTeardown)
#pragma alloc_text(PAGE, behelitInstanceSetup)
#pragma alloc_text(PAGE, behelitInstanceTeardownStart)
#pragma alloc_text(PAGE, behelitInstanceTeardownComplete)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, 0, PreCreate, nullptr},       // Pre Create -> PRÉ-DELETE E PRÉ-RENAME
    { IRP_MJ_WRITE, 0, PreWrite, nullptr},         // post write -> PÓS-ENCRYPTION
    { IRP_MJ_SET_INFORMATION, 0, PreSet, nullptr}, // Pre Set Info -> PRÉ-DELETE
    { IRP_MJ_OPERATION_END }
};


CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags
    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks
    behelitUnload,                           //  MiniFilterUnload
    behelitInstanceSetup,                    //  InstanceSetup
    behelitInstanceQueryTeardown,            //  InstanceQueryTeardown
    behelitInstanceTeardownStart,            //  InstanceTeardownStart
    behelitInstanceTeardownComplete,         //  InstanceTeardownComplete
    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent
};



NTSTATUS
behelitInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("behelit!behelitInstanceSetup: Entered\n") );

    return STATUS_SUCCESS;
}


NTSTATUS
behelitInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("behelit!behelitInstanceQueryTeardown: Entered\n") );

    return STATUS_SUCCESS;
}


VOID
behelitInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("behelit!behelitInstanceTeardownStart: Entered\n") );
}


VOID
behelitInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("behelit!behelitInstanceTeardownComplete: Entered\n") );
}

void BehelitUnloadDriver(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\behelit");
    
    IoDeleteSymbolicLink(&symLink);
    IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS BehelitOpenClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


const int MaxDecoys = 2;
WCHAR* Decoys[MaxDecoys] = { L"aabehelit", L"zzbehelit"};
int malware = 0;

bool isDecoy(PCUNICODE_STRING extension) {
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

    for (int i = 0; i < MaxDecoys; i++) {
        if (Decoys[i] != nullptr) {
            bool checkExt = wcsstr(buffer, Decoys[i]);
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
    ENTRY POINT do Driver
*************************************************************************/

NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    NTSTATUS status;
    UNREFERENCED_PARAMETER(RegistryPath);
    KdPrint(("Behelit Entry Point entered!\n"));

    DriverObject->DriverUnload = BehelitUnloadDriver;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = BehelitOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = BehelitOpenClose;

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\behelit");
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\behelit");
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
behelitUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("behelit!behelitUnload: Entered\n") );

    FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}

bool checkCounter() {
    malware++;
    int b = 5;
    if (malware >= b) {
        // malware = 0;
        // cancelops = 1;
        return true;
    }
    return false;
}

int cancelops = 0;

NTSTATUS killProcess(ULONG pid) {
    NTSTATUS status;
    HANDLE hProcess;
    // auto process = PsGetThreadProcessId(Data->Thread);
    HANDLE process = ULongToHandle(pid);
    PEPROCESS thehandle;
    status = PsLookupProcessByProcessId(process, &thehandle);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    status = ObOpenObjectByPointer(thehandle, 0, nullptr, 0, NULL, KernelMode, &hProcess);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    status = ZwTerminateProcess(hProcess, STATUS_ACCESS_DENIED);
    if (!NT_SUCCESS(status)) {
        ZwClose(hProcess);
        return status;
    }
    ZwClose(hProcess);

    return status;
}
/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS PreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID *CompletionContext){
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    NTSTATUS status;
    auto ok = FLT_PREOP_SUCCESS_NO_CALLBACK;
    auto fail = FLT_PREOP_COMPLETE;
    auto& params = Data->Iopb->Parameters.Create;
    ULONG pid = FltGetRequestorProcessId(Data);
    PFLT_FILE_NAME_INFORMATION file;

    if (Data->RequestorMode == KernelMode) {
        return ok;
    }

    //if (cancelops == 1) {
    //    KdPrint(("Seu computador foi atacado por um ransomware. Ops protegidas\n"));
    //    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
    //    return fail;
    //}

    if (params.Options & FILE_DELETE_ON_CLOSE) { // LIBERE O FILENAMEINFO PF
        KdPrint(("pid (%lu) trying to delete decoy file\n", pid));
        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &file);
        if (!NT_SUCCESS(status)) {
            return ok;
        }
        if (!isDecoy(&file->Name)) {
            FltReleaseFileNameInformation(file);
            return ok;
        }
        if (checkCounter()) {
            status = killProcess(pid);
            KdPrint(("SEXOO\n"));
            if (!NT_SUCCESS(status)) {
               KdPrint(("failed to kill pid %lu (0x%08X)\n", pid, status));
            }
             else {
               KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
               KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
               KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
               KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
               KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
               KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
               KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));

            }
        }
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        FltReleaseFileNameInformation(file);
        return fail;
    }
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS PreSet(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    NTSTATUS status;
    auto ok = FLT_PREOP_SUCCESS_NO_CALLBACK;
    auto fail = FLT_PREOP_COMPLETE;
    ULONG pid = FltGetRequestorProcessId(Data);
    PFLT_FILE_NAME_INFORMATION file;
    auto& params = Data->Iopb->Parameters.SetFileInformation;

    if (Data->RequestorMode == KernelMode) {
        return ok;
    }
    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &file);
    if (!NT_SUCCESS(status)) {
        return ok;
    }
    if (!isDecoy(&file->Name)) {
        FltReleaseFileNameInformation(file);
        return ok;
    }
    if (params.FileInformationClass == FileRenameInformation) {
        KdPrint(("Blocked a decoy from renaming\n"));
        FltReleaseFileNameInformation(file);
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        return fail;
    }

    if (params.FileInformationClass != FileDispositionInformation && params.FileInformationClass != FileDispositionInformationEx) {
        auto info = (FILE_DISPOSITION_INFORMATION*)params.InfoBuffer;
        if (info->DeleteFile) {
            Data->IoStatus.Status = STATUS_ACCESS_DENIED;
            if (checkCounter()) {
                KdPrint(("Sexo\n"));
                status = killProcess(pid);
                if (!NT_SUCCESS(status)) {
                   KdPrint(("failed to kill pid %lu (0x%08X)\n", pid, status));
                }
                else {
                    KdPrint(("killed pid %lu (0x%08X)\n", pid, status));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
                    KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));

                }
            }
            KdPrint(("MALWARE COUNTER: %i\n", malware));
            FltReleaseFileNameInformation(file);
            }
            return fail;
        }
    FltReleaseFileNameInformation(file);
    return ok;
}

FLT_PREOP_CALLBACK_STATUS PreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    NTSTATUS status;
    auto ok = FLT_PREOP_SUCCESS_NO_CALLBACK;
    auto fail = FLT_PREOP_COMPLETE;
    ULONG pid = FltGetRequestorProcessId(Data);
    PFLT_FILE_NAME_INFORMATION file;
    auto& params = Data->Iopb->Parameters.Write;
    auto writelen = params.Length;
    ULONG userdecoywrite = 1023;
   
    if (Data->RequestorMode == KernelMode) {
        return ok;
    }
    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &file);
    if (!NT_SUCCESS(status)) {
        return ok;
    }
    if (!isDecoy(&file->Name)) {
        FltReleaseFileNameInformation(file);
        return ok;
    }
    if (writelen < userdecoywrite) {
        FltReleaseFileNameInformation(file);
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        KdPrint(("Please dont write in decoy file\n"));
        return fail;
    }
    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
    KdPrint(("not allowing suspicious write in decoy\n"));
    if (checkCounter()) {
        KdPrint(("Sexo\n"));
        status = killProcess(pid);
        if (!NT_SUCCESS(status)) {
            KdPrint(("failed to kill pid %lu (0x%08X)\n", pid, status));
        }
        if (NT_SUCCESS(status)) {
           KdPrint(("killed it\n"));
           KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
           KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
           KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
           KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
           KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));
           KdPrint(("MUAHAHAHAHAHAHAHAHAHAHAHAH killed pid %lu\n", pid));

        }
    }
    FltReleaseFileNameInformation(file);
    return fail;
}

