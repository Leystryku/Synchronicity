#pragma once

#include "synchronicity.h"
#include "antidbg.h"

size_t authhash = 0;
int Synchronicity::Init()
{
	if (Initiated)
		return 0;

	Initiated = true;

	utils::file::DirCreate("C:\\Synchronicity");
	utils::file::DirCreate("C:\\Synchronicity\\log");

	utils::file::WriteToLog("Initializing Synchronicity...\n");
	
	std::string init = InitializeSdk();

	if (init != "")
	{
		utils::file::WriteToLog("Could not initialize Synchronicity == sdk!\n");
		utils::file::WriteToLog(init.c_str());
		MessageBoxA(NULL, init.c_str(), "!ERROR!", MB_OK);
		return 0;
	}
	DoAntiDebug();

	init = hooks->HookStuff();

	if (init != "")
	{
		utils::file::WriteToLog("Could not initialize Synchronicity == hooks!\n");
		utils::file::WriteToLog(init.c_str());
		MessageBoxA(NULL, init.c_str(), "!ERROR!", MB_OK);
		return 0;
	}

	
	int ScrW, ScrH = 0;
	g_pEngine->GetScreenSize(ScrW, ScrH);

	g_pEngine->ClientCmd("showconsole");
	//Msg("[Synchronicity] Initialized successfully!\n");
	//Msg("This build was compiled at %s\n", __TIME__);

	utils::file::WriteToLog("Synchronicity Initialized successfully!\n");

	int icock = 0;
	while (1)
	{
		ThinkAntiDebug();
		OutputDebugStringA("%s%s");
		_sleep(1);
	}

	return 1;
}

int DllInit()
{
	DoAntiDebug();
	Synchronicity::Init();
	return 0;
}

bool antidbg_done = false;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{

	if (reason == DLL_PROCESS_ATTACH)
	{

		DoAntiDebug();

		if (!antidbg_done)
		{
			ExitProcess(0);
			exit(0);
			return 0;
		}

		HANDLE thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DllInit, NULL, CREATE_SUSPENDED, 0);
		HideThread(thread);
		Sleep(5);
		ResumeThread(thread);
		DisableThreadLibraryCalls(hModule);
	}

	return TRUE;
}

