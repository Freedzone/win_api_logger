#include <Windows.h>
#include <conio.h>
#include <iostream>
#include <WinReg.h>

using namespace std;

int main(int _Argc, char ** _Argv)
{
	char buf[256];
	DWORD bytes;
	DWORD oldProtect, backupProtect;
	HANDLE hFile; 
	MEMORY_BASIC_INFORMATION memInfo;
	STARTUPINFO startinfo;
	HKEY hkey;
	DWORD dwDisposition;

	printf("Press any key to start");
	_getch();

	hFile = CreateFile("TEST.bin", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	for (int i = 5; i > 0; i--)
		GetProcAddress(GetModuleHandle("kernel32.dll"), "GetStartupInfoW");

	VirtualProtect(GetCurrentProcess(), 100, PAGE_EXECUTE_READWRITE, &oldProtect);
	VirtualQuery(GetCurrentProcess(), &memInfo, 100);
	Sleep(100);
	VirtualProtect(GetCurrentProcess(), 100, oldProtect, &backupProtect);

	ReadFile(hFile, buf, 10, &bytes, NULL);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
		TEXT("SOFTWARE\\aaTestCompany\\testApp"), 
		0, NULL, 0, 
		KEY_WRITE, NULL, 
		&hkey, &dwDisposition);
	RegOpenKey(hkey, "SOFTWARE\\Classes", &hkey);
	GetStartupInfo(&startinfo);
	CloseHandle(hFile);
	DeleteFile("TEST.bin");
}