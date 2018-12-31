/*
	SelectMyParent
	Author： Didier Stevens
	Link：https://blog.didierstevens.com/2009/11/22/quickpost-selectmyparent-or-playing-with-the-windows-process-tree/
	start a program and select its parent process	
*/

#include "stdafx.h"

#include <windows.h>
#pragma comment(lib,"advapi32.lib")
void DisplayErrorMessage(LPTSTR pszMessage, DWORD dwLastError)
{
	HLOCAL hlErrorMessage = NULL;
	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (PTSTR) &hlErrorMessage, 0, NULL))
	{
		_tprintf(TEXT("%s: %s"), pszMessage, (PCTSTR) LocalLock(hlErrorMessage));
		LocalFree(hlErrorMessage);
	}
}

BOOL CurrentProcessAdjustToken(void)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES sTP;

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sTP.Privileges[0].Luid))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		sTP.PrivilegeCount = 1;
		sTP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(hToken, 0, &sTP, sizeof(sTP), NULL, NULL))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		CloseHandle(hToken);
		return TRUE;
	}
	return FALSE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	STARTUPINFOEX sie = {sizeof(sie)};
	PROCESS_INFORMATION pi;
	SIZE_T cbAttributeListSize = 0;
	PPROC_THREAD_ATTRIBUTE_LIST pAttributeList = NULL;
	HANDLE hParentProcess = NULL;
	DWORD dwPid = 0;

	_putts(TEXT("SelectMyParent v0.0.0.1: start a program with a selected parent process"));
	_putts(TEXT("Source code put in public domain by Didier Stevens, no Copyright"));
	_putts(TEXT("https://DidierStevens.com"));
	_putts(TEXT("Use at your own risk\n"));
	if (argc != 3)
		_putts(TEXT("usage: SelectMyParent program pid"));
	else
	{
		dwPid = _tstoi(argv[2]);
		if (0 == dwPid)
		{
			_putts(TEXT("Invalid pid"));
			return 0;
		}
		InitializeProcThreadAttributeList(NULL, 1, 0, &cbAttributeListSize);
		pAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST) HeapAlloc(GetProcessHeap(), 0, cbAttributeListSize);
		if (NULL == pAttributeList)
		{
			DisplayErrorMessage(TEXT("HeapAlloc error"), GetLastError());
			return 0;
		}
		if (!InitializeProcThreadAttributeList(pAttributeList, 1, 0, &cbAttributeListSize))
		{
			DisplayErrorMessage(TEXT("InitializeProcThreadAttributeList error"), GetLastError());
			return 0;
		}
		CurrentProcessAdjustToken();
		hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (NULL == hParentProcess)
		{
			DisplayErrorMessage(TEXT("OpenProcess error"), GetLastError());
			return 0;
		}
		if (!UpdateProcThreadAttribute(pAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hParentProcess, sizeof(HANDLE), NULL, NULL))
		{
			DisplayErrorMessage(TEXT("UpdateProcThreadAttribute error"), GetLastError());
			return 0;
		}
		sie.lpAttributeList = pAttributeList;
		if (!CreateProcess(NULL, argv[1], NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &sie.StartupInfo, &pi))
		{
			DisplayErrorMessage(TEXT("CreateProcess error"), GetLastError());
			return 0;
		}
		_tprintf(TEXT("Process created: %d\n"), pi.dwProcessId);
		DeleteProcThreadAttributeList(pAttributeList);
		CloseHandle(hParentProcess);
	}

	return 0;
}
