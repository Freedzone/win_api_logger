#include <Windows.h>
#include <conio.h>
#include <iostream>

#include "detours.h"

#define DLL_NAME "API_Logger.dll"
using namespace std;

void Error(char* funcName)
{
	char * szError;
	static char szErrorBuf[256];

	DWORD dwError = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		(LPTSTR)&szError,
		0, NULL );

	_snprintf(szErrorBuf, sizeof(szErrorBuf), "\n%s exits with error: %s\n", funcName, szError);
	printf("%s", szErrorBuf);
	LocalFree(szError);
	_getch();
	exit(dwError);
}

int main(int _Argc, char ** _Argv)
{
	char szExepath[MAX_PATH];
	DWORD res = 0;
	STARTUPINFO StartupInfo = {0};
	PROCESS_INFORMATION ProcessInformation;

	ZeroMemory(szExepath, sizeof(szExepath));

	if(_Argc <= 1) // no input parameters, ask user to input
	{
		cout << "EXE path: "; 
		cin.getline(szExepath, MAX_PATH);
	}
	else 
	{
		strcpy_s(szExepath, MAX_PATH, _Argv[1]);
	}

	res = DetourCreateProcessWithDll(szExepath, NULL, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE, 
										NULL, NULL, &StartupInfo, &ProcessInformation, DLL_NAME, NULL );
	if(res == 0)
		Error("main->DetourCreateProcessWithDll");
	
	return 0;
}