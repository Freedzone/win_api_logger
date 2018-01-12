// dllmain.cpp : Defines the entry point for the DLL application.
#include "API_Logger.h"
#include "CLogger.h"

#define NO_PROLOGUE_SECTION "# noprologue"

using namespace std;

const char* aLibrariesWin10[]  = {"KERNELBASE", NULL};
const char* aLibrariesWin7[]   = {"KERNELBASE", "kernel32", NULL};
const char* aLibrariesWinXP[]  = {"kernel32", "ADVAPI32", "USER32", NULL};
const char** pLibraries;

HANDLE hInitThread;
HANDLE hMainThread;

vector<string> TargetApis;
CLogger* pLogger;

void SingleHandler(int id)
{
	if(pLogger->bLogOn == FALSE)
		return;
	
	SYSTEMTIME systime;

	pLogger->bLogOn = FALSE;

	GetSystemTime(&systime);
	pLogger->WriteLog("[%d:%.2d:%.2d %dms]%s",	systime.wHour, systime.wMinute, systime.wSecond, 
											systime.wMilliseconds, TargetApis[id].data() );
	pLogger->bLogOn = TRUE;
}

void GenerateStub(DWORD addr, int id)
{
	DWORD dwOldProtect, dwBkupProtect; 
	DWORD dwRealAddr, dwBackAddr;
	DWORD dwAddressOfSingleHandler = (DWORD)&SingleHandler;
	BYTE oldData[5];
	char* HandlerPointer = (char*)VirtualAlloc(0, 30, MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE); // allocate memory for 1 handler
	
	memcpy(oldData, (void*)addr, 5);	// store old data

	if(oldData[0] == '\x68' || oldData[0] == '\x6A')
	{
		addr += 2;
		memcpy(oldData, (void*)addr, 5);	// store new old data
		cout << "Special prologue at " << hex << addr << " ";
	}

	HandlerPointer[0] = 0x60; // pushad
	HandlerPointer[1] = 0x68; // push word
	*(DWORD*)&HandlerPointer[2]  = id;
	HandlerPointer[6] = 0xe8; // call 
	*(DWORD*)&HandlerPointer[7] = dwAddressOfSingleHandler - (DWORD)&HandlerPointer[6] - 5;
	HandlerPointer[11] = 0x5d; // POP EBP (clean after function)
	HandlerPointer[12] = 0x61; // popad
	memcpy(&HandlerPointer[13], oldData, 5); // restore displaced bytes
	HandlerPointer[18] = 0xe9; // jmp
	dwBackAddr = (DWORD) (addr - (DWORD)&HandlerPointer[18]); // calc real jmp back
	memcpy(&HandlerPointer[19], &dwBackAddr, 4); // write address

	//############## Debug print stub address
	cout << endl << TargetApis[id] << " " << hex << addr << " " << (void*)HandlerPointer << endl;
	//#########################################

	// Calculate real jmp to
	dwRealAddr = (DWORD) ( (DWORD)HandlerPointer - addr) - 5;
	VirtualProtect((void*)addr, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy((void*)addr, "\xE9", 1);	// JMP
	memcpy((void*)(addr + 0x1), &dwRealAddr, 4 );

	VirtualProtect((void*)addr, 5, dwOldProtect, &dwBkupProtect);
}

void SetHooks()
{
	ifstream infile;
	string szLibName;
	string szApiName;
	string szFolder;
	HMODULE hDll;
	int i = 0, apiCounter = 0;

	// Set libraries depending on windows version
	OSVERSIONINFO version;
	version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if( GetVersionEx(&version) == 0 )
		return;

	if(version.dwMajorVersion == 5)
		pLibraries = aLibrariesWinXP;
	else if(version.dwMajorVersion == 6)
		pLibraries = aLibrariesWin7;
	else if(version.dwMajorVersion == 10)
		pLibraries = aLibrariesWin10;
	else
	{
		MessageBox(NULL, "OS version is not supported!", "Error", MB_ICONERROR);
		return;
	}

	// Set hooks
	while(pLibraries[i] != NULL)
	{
		string szBuf; 

		szLibName = pLibraries[i];

		szBuf = szLibName + ".dll";
		hDll = GetModuleHandle(szBuf.data());

		szBuf = szLibName + ".txt";
		infile.open(szBuf.data());

		i++;

		// if error while opening file or can't get module handle
		if( hDll == NULL || !infile.good() ) 
		{
			infile.close();
			continue; 
		}

		// read APIs from file
		while( getline(infile, szBuf) )
		{
			if(szBuf[0] == '>')
				continue;

			// find API in dll
			DWORD addr = (DWORD)GetProcAddress(hDll, szBuf.data());

			if(addr == NULL)
				continue;

			// add API name to list
			TargetApis.push_back(szBuf);

			// Make stub 
			GenerateStub(addr, apiCounter);
			apiCounter++;
		}
		infile.close();
	}
}

void SetHooksRoutine() 
{
	SuspendThread(hMainThread);

	pLogger->bLogOn = FALSE;

	SetHooks();

	pLogger->bLogOn = TRUE;

	ResumeThread(hMainThread);
}

void OnInit() 
{
	hMainThread = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());
	hInitThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SetHooksRoutine, NULL, 0, NULL);
	if(hInitThread == NULL) 
	{
		ExitProcess(123);
	}
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		pLogger = new CLogger("out.txt");
		OnInit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	} 
	return TRUE;
}