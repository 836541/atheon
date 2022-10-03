#include <Windows.h>
#include <stdio.h>
#include "C:\Users\Admin\Desktop\Drivers\Sacerdote\BackupData.h"

int Error(const char* msg) {
	printf("%s (error=%u)\n", msg, GetLastError());
	return 1;
}

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		printf("Usage: client.exe <MaxSizeOfBackupFiles> <ProtectionValue>\n");
		return 0;
	}

	int MaxFileSize = atoi(argv[1]);
	int Protection = atoi(argv[2]);

	HANDLE hDevice = CreateFile(L"\\\\.\\sacerdote", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE) {
		Error("Failed to Open Device\n");
	}

	BackupData data;
	data.MaxSize = MaxFileSize;
	data.Protect = Protection;

	DWORD bWritten;

	BOOL success = WriteFile(hDevice, &data, sizeof(data), &bWritten, nullptr);
	if (!success) {
		Error("Failed to communicate with Sacerdote\n");
	}

	printf("Data sent to Minifilter with success\n");
	CloseHandle(hDevice);
}




