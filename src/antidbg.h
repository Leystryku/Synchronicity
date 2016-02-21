#pragma once

#include <windows.h>

extern bool antidbg_done;

#define NTSTATUS LONG

typedef NTSTATUS(NTAPI *pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);
pNtSetInformationThread NtSetThreadInfo;

#define niceasm() \
__asm mov eax, 523423; \
__asm mov ecx, eax; \
__asm mov ebx, ecx; \
__asm mov edx, eax; \
__asm mov ebx, ecx; \
__asm pushad; \
__asm xor ebx, ebx; \
__asm xor edx, edx; \
__asm popad; \
__asm call [ecx]


#define nicecrash()  ExitProcess(0); exit(0); niceasm(); __asm nop;__asm nop;__asm nop;__asm nop;__asm nop;__asm nop;__asm nop;__asm nop;__asm nop;__asm nop; niceasm();



inline bool InitNTFuncs()
{
	HMODULE ntlib = LoadLibraryA("ntdll.dll");

	NtSetThreadInfo = (pNtSetInformationThread)
		GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")),
		"NtSetInformationThread");

	if (!NtSetThreadInfo)
		return false;

	return true;
}
inline bool HideThread(HANDLE hThread)
{

	NTSTATUS Status;

	// Set the thread info

	Status = NtSetThreadInfo(hThread, 0x11, 0, 0);

	if (Status != 0x00000000)
		return false;
	else
		return true;
}

inline void nicercrash()
{
	nicecrash();
}

long double fags = 0;


inline void ThinkAntiDebug()
{

	OutputDebugStringA("%s%s");
	fags = (2 ^ 63) - 0.5;
	fags = 111111111111111111111111111111111111111111111111111111111111111.1;
	fags -= 1.f;

	__try{
		__asm mov ebx, dword ptr[eax];//if not debugged it will raise an exception cause eax will be 0 or 1
		//MessageBoxA(NULL, "k0", "kk", MB_OK);
		nicecrash();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}

	unsigned int var;
	__asm
	{
		MOV EAX, FS:[0x30];
		MOV EAX, [EAX + 0x18];
		MOV EAX, [EAX + 0x0c];
		MOV var, EAX
	}

	if (var != 2)
	{
		//MessageBoxA(NULL, "k1", "kk", MB_OK);
		nicecrash();
	}

	unsigned long NtGlobalFlags = 0;

	__asm {

		mov eax, fs:[30h];
		mov eax, [eax + 68h];
		mov NtGlobalFlags, eax;
	}

	// 0x70 =  FLG_HEAP_ENABLE_TAIL_CHECK |
	//         FLG_HEAP_ENABLE_FREE_CHECK | 
	//         FLG_HEAP_VALIDATE_PARAMETERS
	if (NtGlobalFlags & 0x70)
	{
		//MessageBoxA(NULL, "k2", "kk", MB_OK);
		nicecrash();
	}


	if (IsDebuggerPresent())
	{
		//MessageBoxA(NULL, "k3", "kk", MB_OK);
		nicecrash();
	}

	BOOL result;
	
	CheckRemoteDebuggerPresent(GetCurrentProcess(), &result);

	if (result)
	{
		//MessageBoxA(NULL, "k4", "kk", MB_OK);
		nicecrash();
	}







}

inline void DoAntiDebug()
{

	bool bsuccess = false;

	ThinkAntiDebug();

	unsigned int time1 = 0;
	unsigned int time2 = 0;
	__asm
	{
		RDTSC
			MOV time1, EAX
			RDTSC
			MOV time2, EAX

	}
	if ((time2 - time1) > 100)
	{
		nicecrash();
	}

	unsigned char* p = (unsigned char*)GetProcAddress(GetModuleHandle("ntdll.dll"), "DbgUiRemoteBreakin");

	unsigned long xxx = 0;
	VirtualProtect(p, 6, PAGE_EXECUTE_READWRITE, &xxx);

	*p = 0x68;
	*(unsigned long*)(p + 1) = (unsigned long)GetProcAddress(GetModuleHandle("kernel32.dll"), "ExitProcess");
	*(p + 5) = 0xC3;
	VirtualProtect(p, 6, xxx, &xxx);

	InitNTFuncs();

	bsuccess = HideThread(GetCurrentThread());

	if (!bsuccess)
	{
		nicecrash();
	}

	antidbg_done = bsuccess;

}