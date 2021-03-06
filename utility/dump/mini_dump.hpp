#ifndef __WIN32_DEBUG_EXCEPT_MINIDUMP_HPP
#define __WIN32_DEBUG_EXCEPT_MINIDUMP_HPP
#include <Windows.h>
#include <DbgHelp.h>
#include <Shlwapi.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "shlwapi.lib")

namespace win32
{
	namespace debug
	{
		// -----------------------------------------------
		// class MiniDump

		class mini_dump
		{
		public:
			mini_dump()
			{
				::SetUnhandledExceptionFilter(filter);
				//DisableSetUnhandledExceptionFilter();
			}

		private:
			static void dump(HANDLE hFile, PEXCEPTION_POINTERS excpInfo)
			{
				if (excpInfo == NULL)
				{
					__try
					{
						::RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
					}
					__except(dump(hFile, GetExceptionInformation()),
							EXCEPTION_CONTINUE_EXECUTION)
					{}}
				else
				{
					MINIDUMP_EXCEPTION_INFORMATION eInfo;
					eInfo.ThreadId = ::GetCurrentThreadId();
					eInfo.ExceptionPointers = excpInfo;
					eInfo.ClientPointers = FALSE;
					::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(),
							hFile, MiniDumpNormal, excpInfo ? &eInfo : NULL, NULL, NULL);
				}
			}

			static LONG WINAPI filter(PEXCEPTION_POINTERS pExcept)
			{
				TCHAR szDir[MAX_PATH] = {0};
				TCHAR szExe[MAX_PATH] = {0};
				::GetModuleFileName(NULL, szDir, _countof(szDir));

				LPTSTR pFile = ::PathFindFileName(szDir);
				LPTSTR pExt = ::PathFindExtension(szDir);
				if (pFile != szDir && pExt != NULL)
				{
					*pExt = 0;
					lstrcpy(szExe, pFile);
				}
				else
				{
					lstrcpy(szExe, TEXT("Crash"));
				}

				::PathRemoveFileSpec(szDir);
				::PathAddBackslash(szDir);
				lstrcat(szDir, TEXT("Dump\\"));
				::CreateDirectory(szDir, NULL);

				SYSTEMTIME st = {0};
				::GetLocalTime(&st);
				TCHAR szModuleName[MAX_PATH] = {0};
				wsprintf(szModuleName, TEXT("%s%s-%04d%02d%02d%02d%02d%02d.dmp"), szDir, szExe,
						st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

				HANDLE hFile = ::CreateFile(szModuleName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					dump(hFile, pExcept);
					::CloseHandle(hFile);
				}
				return 0;
			}

			void DisableSetUnhandledExceptionFilter()
			{
				void* addr = (void*)GetProcAddress(LoadLibraryW(L"kernel32.dll"),
					"SetUnhandledExceptionFilter");

				if (addr)
				{
					unsigned char code[16];
					int size = 0;

					code[size++] = 0x33;
					code[size++] = 0xC0;
					code[size++] = 0xC2;
					code[size++] = 0x04;
					code[size++] = 0x00;

					DWORD dwOldFlag, dwTempFlag;
					VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
					WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
					VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
				}
			}
		};
	}
}

#endif
