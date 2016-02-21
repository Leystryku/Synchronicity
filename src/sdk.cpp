#include "sdk.h"

IBaseClientDLL *g_pClient = 0;
IVEngineClient *g_pEngine = 0;
IPred *g_pPrediction = 0;
IClientEntityList *g_pEntList = 0;
IEngineTrace *g_pEngineTrace = 0;
IVModelInfoClient *g_pModelInfo = 0;
ICvar *g_pCVar = 0;
IEntity *g_pLocalEnt = 0;
CGlobalVars *g_pGlobals = 0;

std::string InitializeSdk()
{
	HMODULE g_ClientDLL = GetModuleHandleA("client.dll");
	HMODULE g_EngineDLL = GetModuleHandle("engine.dll");
	HMODULE g_VstdLibDLL = GetModuleHandle("vstdlib.dll");
	HMODULE g_LuaSharedDLL = GetModuleHandle("lua_shared.dll");

	if (!g_ClientDLL || !g_EngineDLL || !g_VstdLibDLL || !g_LuaSharedDLL)
	{
		Sleep(200);
		return InitializeSdk();
	}

	CreateInterfaceFn g_ClientFactory = (CreateInterfaceFn)GetProcAddress(g_ClientDLL, "CreateInterface");
	CreateInterfaceFn g_EngineFactory = (CreateInterfaceFn)GetProcAddress(g_EngineDLL, "CreateInterface");
	CreateInterfaceFn g_VstdFactory = (CreateInterfaceFn)GetProcAddress(g_VstdLibDLL, "CreateInterface");
	CreateInterfaceFn g_LuaFactory = (CreateInterfaceFn)GetProcAddress(g_LuaSharedDLL, "CreateInterface");
	const char *has_Failed = NULL;

	if (!g_ClientFactory || !g_EngineFactory || !g_VstdFactory || !g_LuaFactory)
	{
		has_Failed = "Could not get a factory";
		MessageBox(NULL, has_Failed, "grr", MB_OK);

		return 0;
	}

	//Get Interfaces
	g_pClient = (IBaseClientDLL*)g_ClientFactory("VClient017", 0);
	g_pEngine = (IVEngineClient*)g_EngineFactory("VEngineClient015", 0);
	g_pEntList = (IClientEntityList*)g_ClientFactory("VClientEntityList003", 0);
	g_pEngineTrace = (IEngineTrace*)g_EngineFactory("EngineTraceClient003", 0);
	g_pModelInfo = (IVModelInfoClient*)g_EngineFactory("VModelInfoClient006", 0);
	g_pCVar = (ICvar*)g_VstdFactory("VEngineCvar004", 0);
	g_pPrediction = (IPred*)g_ClientFactory("VClientPrediction001", 0);
	void *pLuaShared = (void*)g_LuaFactory("LUASHARED003", 0);
	g_pLuaShared = new ILuaShared;

	if (!g_pClient)
	{
		return "g_pClient == NULL";
	}

	if (!g_pEngine)
	{
		return "g_pEngine == NULL";
	}


	if (!g_pEntList)
	{
		return "g_pEntList == NULL";
	}

	if (!g_pEngineTrace)
	{
		return "g_pEngineTrace == NULL";
	}

	if (!g_pModelInfo)
	{
		return "g_pModelInfo == NULL";
	}

	if (!g_pCVar)
	{
		return "g_pCVar == NULL";
	}

	if (!g_pPrediction)
	{
		return "g_pPrediction == NULL";
	}

	if (!pLuaShared)
	{
		return "pLuaShared == NULL";
	}

	g_pLuaShared->InitShared(pLuaShared);

	void *vglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod((void*)g_pClient, 0), "\x89\x0D", 0x300, 2);

	if (!vglobals)
		vglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod((void*)g_pClient, 0), "\x51\xA3", 0x300, 2);

	if (!vglobals)
		vglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod((void*)g_pClient, 0), "\x08\xA3", 0x300, 2);


	if (!vglobals)
	{
		return "no g_pGlobals!";
	}

	g_pGlobals = **(CGlobalVars***)(vglobals);

	return "";

}

void VectorTransformR(const Vector &vecIn, const matrix3x4_t &matrixIn, Vector &vecOut)
{
	Vector vecInNew = vecIn;
	vecOut.x = vecInNew.Dot(Vector(matrixIn[0][0], matrixIn[0][1], matrixIn[0][2])) + matrixIn[0][3];
	vecOut.y = vecInNew.Dot(Vector(matrixIn[1][0], matrixIn[1][1], matrixIn[1][2])) + matrixIn[1][3];
	vecOut.z = vecInNew.Dot(Vector(matrixIn[2][0], matrixIn[2][1], matrixIn[2][2])) + matrixIn[2][3];
}