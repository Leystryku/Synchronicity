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


	return 1;
}

int DllInit()
{
	Synchronicity::Init();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{

	if (reason == DLL_PROCESS_ATTACH)
	{
		HANDLE thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DllInit, NULL, CREATE_SUSPENDED, 0);
		DisableThreadLibraryCalls(hModule);
		Sleep(5);
		ResumeThread(thread);
	}

	return TRUE;
}

