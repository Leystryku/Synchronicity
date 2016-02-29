#pragma once

#include "hooks.h"
char synchronicityfolder[MAX_PATH];

int lua_hookfuncref = 0;
int lua_vectorfuncref = 0;
int lua_anglefuncref = 0;

#define getebp() stack *_bp; __asm mov _bp, ebp;

struct stack
{
	stack *next;
	char  *ret;

	template<typename T> inline T arg(unsigned int i)
	{
		return *(T *)((void **)this + i + 2);
	}
};

m_hooks *hooks;

mstudiohitboxset_t* ientity__hboxsets[0xFFFF];
matrix3x4_t ientity__matrixs[0xFFFF][128];
int ientity__boneaccess[0xFFFF];

ILuaInterface *menuluastate = 0;

bool luaset_DoVisNoRecoil = false;
HANDLE lua_Thread = 0;
int lua_ThreadID;

void AntiAntiAimProxyX(const CRecvProxyData *pData, void *ent, void *pOut)
{
	if (!pOut)
		return;

	float val = pData->m_Value.m_Float;

	char shit[25];
	sprintf(shit, "K: %f", val);
	MessageBoxA(NULL, shit, "kkk", MB_OK);

	//lua call
	*(float*)pOut = val;
}

void AntiAntiAimProxyY(const CRecvProxyData *pData, void *ent, void *pOut)
{
	if (!pOut)
		return;

	float val = pData->m_Value.m_Float;


	char shit[25];
	sprintf(shit, "K: %f", val);
	MessageBoxA(NULL, shit, "kkk", MB_OK);

	//lua call
	*(float*)pOut = val;

}


void* cacheddiff[1000];

bool __stdcall hooked_InPrediction(void)
{
	bool ret = g_pPrediction->InPrediction();

	if (!luaset_DoVisNoRecoil)
		return ret;

	void*vesi;
	__asm mov vesi, esi;

	if (!g_pLocalEnt || g_pLocalEnt != vesi)
		return ret;

	if (!vesi)
		return ret;

	void*cret = _ReturnAddress();

	int diff = utils::memory::FindSubSize(cret, 800);

	if (diff < 315 || diff > 370)
		return ret;

	static int calcviewcount = 0;
	static QAngle bakPunch;


	calcviewcount++;

	if (calcviewcount == 3)
	{
		g_pLocalEnt->GetViewPunch().x = bakPunch.x;
		g_pLocalEnt->GetViewPunch().y = bakPunch.y;
		g_pLocalEnt->GetViewPunch().z = bakPunch.z;

		bakPunch.x = 0;
		bakPunch.y = 0;
		bakPunch.z = 0;

		calcviewcount = 0;
	}

	if (calcviewcount == 2)
	{
		QAngle curPunch = g_pLocalEnt->GetViewPunch();
		bakPunch.x = curPunch.x;
		bakPunch.y = curPunch.y;
		bakPunch.z = curPunch.z;
		g_pLocalEnt->GetViewPunch().x = 0;
		g_pLocalEnt->GetViewPunch().y = 0;
		g_pLocalEnt->GetViewPunch().z = 0;

	}

	return ret;
}

typedef bool(__fastcall *OrigCreateMove)(void*thisptr, void*edx, float flInputSampleTime, CUserCmd *pCmd);
OrigCreateMove CreateMoveFn;

bool __fastcall hooked_CreateMove(void*thisptr, void*edx, float flInputSampleTime, CUserCmd* pCmd)
{
	if (!flInputSampleTime || !pCmd || !lua_hookfuncref || menuluastate->Top())
		return CreateMoveFn(thisptr, edx, flInputSampleTime, pCmd);

	GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)mlua->NewUserdata(sizeof(GarrysMod::Lua::UserData));


	if (!ud)
	{
		return CreateMoveFn(thisptr, edx, flInputSampleTime, pCmd);
	}

	ud->type = GarrysMod::Lua::Type::USERCMD;
	ud->data = pCmd;



	menuluastate->ReferencePush(lua_hookfuncref);
	menuluastate->PushString("OnCreateMove");

	menuluastate->PushUserdata(ud);
	menuluastate->CreateMetaTableType("CUserCmd", GarrysMod::Lua::Type::USERCMD);

	menuluastate->SetMetaTable(-2);

	if (menuluastate->PCall(2, 0, 0))
	{
		MessageBoxA(NULL, menuluastate->GetString(-1), "Synchronicity - Error", MB_OK);
		menuluastate->Pop();

	}
	menuluastate->Pop();

	return CreateMoveFn(thisptr, edx, flInputSampleTime, pCmd);
}

typedef void*(__fastcall *OrigRunStringEx)(void*thisptr, int edx, const char* filename, const char *path, const char *stringToRun, bool run, bool showErrors, bool bunkwn);
OrigRunStringEx RunStringExFn;

void* __fastcall hooked_RunStringEx(ILuaInterface*lua, int edx, const char* filename, const char *path, const char *stringToRun, bool run, bool showErrors, bool bunkwn)
{

	if (lua_hookfuncref && filename && stringToRun)
	{


		//menuluastate->PushLuaObject(lua_hookfunc);
		menuluastate->ReferencePush(lua_hookfuncref);
		menuluastate->PushString("OnRunString");
		menuluastate->PushString(filename);
		menuluastate->PushString(stringToRun);
		menuluastate->PushBool(run);
		menuluastate->PushBool(showErrors);

		if (menuluastate->PCall(5, 1, 0))
		{
			MessageBoxA(NULL, menuluastate->GetString(-1), "Synchronicity - Error", MB_OK);
			menuluastate->Pop();

			return RunStringExFn(lua, edx, filename, path, stringToRun, run, showErrors, bunkwn);
		}

		const char *newcode = 0;

		if (menuluastate->IsType(-1, GarrysMod::Lua::Type::STRING))
		{
			newcode = menuluastate->GetString(-1);

			if (newcode && strlen(newcode) != 0)
			{
				stringToRun = newcode;
			}
			else{
				return (void*)1;
			}


		}

		menuluastate->Pop();
	}

	return RunStringExFn(lua, edx, filename, path, stringToRun, run, showErrors, bunkwn);
}

typedef void*(__fastcall *OrigCreateLuaInterface)(void*thisptr, int edx, unsigned char cstate, bool renew);
OrigCreateLuaInterface CreateLuaInterfaceFn;


void* __fastcall hooked_CreateLuaInterface(void*thisptr, int edx, unsigned char cstate, bool renew)
{
	void*state = CreateLuaInterfaceFn(thisptr, edx, cstate, renew);

	///VMTHook *hookedstate = new VMTHook(state);
	//RunStringExFn = (OrigRunStringEx)hookedstate->hookFunction(107, hooked_RunStringEx)//crash reason here

	return state;
}

//lua

//lua funcs
int lua_SetHookFunc(lua_State* state)
{
	if (!mlua->IsType(1, GarrysMod::Lua::Type::FUNCTION) || !mlua->IsType(2, GarrysMod::Lua::Type::FUNCTION) || !mlua->IsType(3, GarrysMod::Lua::Type::FUNCTION))
	{
		return 1;
	}

	mlua->Push(1);
	lua_hookfuncref = mlua->ReferenceCreate();

	mlua->Push(2);
	lua_vectorfuncref = mlua->ReferenceCreate();

	mlua->Push(3);
	lua_anglefuncref = mlua->ReferenceCreate();

	return 1;
}

int lua_SetVisualRecoil(lua_State* state)
{
	if (!mlua->IsType(1, GarrysMod::Lua::Type::BOOL))
	{
		return 1;
	}

	luaset_DoVisNoRecoil = mlua->GetBool(1);
	g_pLocalEnt = (IEntity*)g_pEntList->GetClientEntity(g_pEngine->GetLocalPlayer());

	return 1;
}

int lua_SetViewAngles(lua_State *state)
{
	if (!mlua->IsType(1, GarrysMod::Lua::Type::ANGLE))
	{
		return 1;
	}


	g_pEngine->SetViewAngles(*(QAngle*)(LUA->GetUserdata(1))->data);

	return 1;
}

int lua_VectorAngles(lua_State *state)
{
	if (!mlua->IsType(1, GarrysMod::Lua::Type::VECTOR))
	{
		return 1;
	}


	Vector &vec = *(Vector*)(LUA->GetUserdata(1))->data;
	QAngle ang(0, 0, 0);

	VectorAnglesR(vec, ang);
	LUA_PUSHANG(ang, nice);

	return 1;
}

int lua_NormalizeAngles(lua_State *state)
{
	if (!mlua->IsType(1, GarrysMod::Lua::Type::ANGLE))
	{
		return 1;
	}

	QAngle &ang = *(QAngle*)(LUA->GetUserdata(1))->data;
	NormalizeAnglesR(ang);

	//LUA_PUSHANG(ang, nice);
	return 1;
}



int lua_IsConnected(lua_State* state)
{
	LUA->PushBool(g_pEngine->IsConnected());
	return 1;
}


int lua_IsDrawingLoadingScreen(lua_State* state)
{
	LUA->PushBool(g_pEngine->IsDrawingLoadingImage());
	return 1;
}

int lua_IsInGame(lua_State* state)
{
	LUA->PushBool(g_pEngine->IsInGame());
	return 1;
}

bool lua_TraceFilter(IHandleEntity *handleent)
{
	IEntity *ent = static_cast<IEntity*>(handleent);

	if (ent&&ent->entindex()==g_pEngine->GetLocalPlayer())
		return false;

	return true;
}

CTraceFilter filter;

bool IsVisible(Vector& startvec, Vector& endvec, IEntity *ent)
{
	trace_t tr;
	Ray_t pRay;

	pRay.Init(startvec, endvec);
	filter.CustomCollisionFunc = &lua_TraceFilter;

	g_pEngineTrace->TraceRay(pRay, 0x46004003, &filter, &tr); /*0x4600400B */

	if (tr.fraction == 1.0f)
	{
		return true;
	}

	if (tr.m_pEnt && ent)
	{
		if (tr.m_pEnt == ent)
		{
			return true;
		}
	}

	return false;
}

int lua_IsVisible(lua_State* state)
{
	if (!g_pEngine->IsInGame())
		return 1;

	if (!LUA->IsType(1, GarrysMod::Lua::Type::VECTOR) || !LUA->IsType(2, GarrysMod::Lua::Type::VECTOR))
		return 1;


	Vector &startvec = *reinterpret_cast<Vector*>(mlua->GetUserdata(1));
	Vector &endvec = *reinterpret_cast<Vector*>(mlua->GetUserdata(2));

	IEntity *ent = 0;

	if (LUA->IsType(3, GarrysMod::Lua::Type::ENTITY))
	{
		GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*) LUA->GetUserdata(3);
		ent = (IEntity*)ud->data;

		if (!ent || !ent->GetClientClass() || !ent->GetClientClass()->m_pNetworkName)
			ent = 0;
	}

	LUA->PushBool(IsVisible(startvec, endvec, ent));
	return 1;
}

int HitGroupDamage(int iGroup)
{
	if (iGroup == 1) //head
		return 5;

	if (iGroup == 2)//chest
		return 4;

	if (iGroup == 3)//stomach
		return 3;

	if (iGroup == 4 || iGroup == 5)//arms
		return 2;

	if (iGroup == 6 || iGroup == 7)//legs
		return 1;

	return 1;
}

bool CalculateAimVector(IEntity *ent, Vector &vecEyePos, Vector &HitPos, int hitgroup)
{

	HitPos.Zero();

	mstudiohitboxset_t *hboxset = ent->GetHBoxSet();

	if (!hboxset)
		return false;

	matrix3x4_t *matrix = ent->GetMatrix();

	if (!matrix)
		return false;

	

	int numhitboxes = hboxset->numhitboxes;

	if (hitgroup == 8)
	{
		srand((unsigned int)time(NULL));
		hitgroup = (rand() % 7);
	}

	bool foundpos = false;
	int groupdmg = -1;
	Vector newhitpos;

	for (int i = 0; i < numhitboxes; i++)
	{
		mstudiobbox_t* hitbox = hboxset->pHitbox(i);

		if (!hitbox || hitbox->group > 7 || hitgroup && hitbox->group != hitgroup)
			continue;

		Vector middle = ((hitbox->bbmin + hitbox->bbmax) * .5f);


		VectorTransformR(middle, matrix[hitbox->bone], newhitpos);

		if (hitbox->group == 1)
			newhitpos.z -= 3;

		if (!IsVisible(vecEyePos, newhitpos, ent))
			continue;

		foundpos = true;

		int tmpgroupdmg = HitGroupDamage(hitbox->group);

		if (hitgroup)
		{
			HitPos = newhitpos;
			return true;
		}

		if (tmpgroupdmg == 5)
		{
			HitPos = newhitpos;
			return true;
		}

		if (tmpgroupdmg>groupdmg)
		{
			HitPos = newhitpos;
			groupdmg = tmpgroupdmg;
		}

	}

	return foundpos;
}

int lua_CalculateAimVector(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::ENTITY))
	{
		LUA->PushNil();
		return 1;
	}

	if (!LUA->IsType(2, GarrysMod::Lua::Type::VECTOR))
	{
		LUA->PushNil();
		return 1;
	}

	if (!LUA->IsType(3, GarrysMod::Lua::Type::NUMBER))
	{
		LUA->PushNil();
		return 1;
	}

	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;


	Vector* eyepos = (Vector*)(LUA->GetUserdata(2))->data;

	if (!eyepos)
		return 1;

	int hitgroup = LUA->GetNumber(3);

	Vector HitPos(0, 0, 0);

	if (!CalculateAimVector(pEnt, *eyepos, HitPos, hitgroup) || HitPos.IsZero())
	{
		HitPos.Zero();
		LUA->PushNil();
		return 1;
	}

	LUA_PUSHVEC(HitPos, vecud);


	return 1;
}

int lua_CreateFolder(lua_State* state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING))
	{
		return 1;
	}

	std::string tofolder = synchronicityfolder;
	tofolder.append(LUA->GetString(1));

	utils::file::DirCreate(tofolder);
	return 1;
}

int lua_ReadFromSN(lua_State* state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING))
	{
		return 1;
	}

	std::string tofile = synchronicityfolder;
	tofile.append(LUA->GetString(1));

	std::string read = "";

	if (!utils::file::FileRead(tofile, read))
	{
		return 1;
	}


	LUA->PushString(read.c_str());
	return 1;
}

int lua_WriteToSN(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING) || !LUA->IsType(2, GarrysMod::Lua::Type::STRING))
	{
		return 1;
	}

	std::string tofile = synchronicityfolder;
	tofile.append(LUA->GetString(1));

	if (!utils::file::FileEdit(tofile, LUA->GetString(2)))
	{
		return 1;
	}

	LUA->PushBool(true);

	return 1;
}

int lua_RunString(lua_State* state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING))
	{
		LUA->PushString("no lua!");
		return 1;
	}

	if (LUA->IsType(2, GarrysMod::Lua::Type::BOOL) && LUA->GetBool(2) == true)
	{
		LUA->PushString(g_pLuaShared->RunCode(menuluastate, LUA->GetString(1), "[C]"));
		return 1;
	}
	else{
		if (g_pLuaShared->GetLuaInterface(0))
		{
			LUA->PushString(g_pLuaShared->RunCode(g_pLuaShared->GetLuaInterface(0), LUA->GetString(1), "[C]"));
			return 1;
		}
		else{
			LUA->PushString("no client state!");
			return 1;
		}
	}

	LUA->PushString("weird behaviour!");

	return 1;
}
int lua_RunCommand(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::ENTITY))
	{
		return 1;
	}

	if (!LUA->IsType(2, GarrysMod::Lua::Type::USERCMD))
	{
		return 1;
	}

	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt || !pEnt->GetClientClass() || !pEnt->GetClientClass()->m_pNetworkName)
		return 1;

	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(2)->data;

	if (!pCmd)
		return 1;

	int *pFlags = pEnt->GetFlags();
	int Flags = *pFlags;

	QAngle vecPunchAng(g_pLocalEnt->GetViewPunch());
	float frametime = g_pGlobals->frametime;
	float curtime = g_pGlobals->curtime;
	bool attack1 = false;
	bool attack2 = false;

	if ((pCmd->buttons & IN_ATTACK))
	{
		attack1 = true;
		pCmd->buttons &= ~IN_ATTACK;
	}

	if ((pCmd->buttons & IN_ATTACK2))
	{
		attack2 = true;
		pCmd->buttons &= ~IN_ATTACK2;
	}

	g_pPrediction->RunCommand(pEnt, pCmd, 0);

	if (attack1)
		pCmd->buttons |= IN_ATTACK;

	if (attack2)
		pCmd->buttons |= IN_ATTACK2;

	*pFlags = Flags;
	g_pLocalEnt->GetViewPunch().x = vecPunchAng.x;
	g_pLocalEnt->GetViewPunch().y = vecPunchAng.y;
	g_pLocalEnt->GetViewPunch().z = vecPunchAng.z;

	g_pGlobals->frametime = frametime;
	g_pGlobals->curtime = curtime;

	return 1;
}


int lua_ClientCommand(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING))
	{
		return 1;
	}

	g_pEngine->ClientCmd_Unrestricted(LUA->GetString(1));
	return 1;
}

int lua_Disconnect(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING))
	{
		return 1;
	}

	if (!g_pEngine->IsInGame())
		return 1;

	INetChannel *chan = (INetChannel*)g_pEngine->GetNetChannelInfo();

	char dcbuf[255];

	bf_write disconnect(dcbuf, sizeof(dcbuf));
	disconnect.WriteUBitLong(1, 6);
	disconnect.WriteString(LUA->GetString(1));

	chan->SendData(disconnect);



	return 1;
}

int lua_GetMap(lua_State *state)
{
	LUA->PushString(g_pEngine->GetLevelName());
	return 1;
}

int lua_GetIP(lua_State *state)
{
	INetChannelInfo* chan = g_pEngine->GetNetChannelInfo();

	const char* ip = 0;

	if (chan)
		ip = chan->GetAddress();

	if (ip)
		LUA->PushString(ip);
	else
		LUA->PushString("0.0.0.0");

	return 1;
}


int lua_GetMaxEnts(lua_State *state)
{
	LUA->PushNumber(g_pEntList->GetHighestEntityIndex());
	return 1;
}

int lua_GetMaxPlayers(lua_State *state)
{
	LUA->PushNumber(g_pGlobals->maxClients);
	return 1;
}

int lua_GetEntity(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::NUMBER))
	{
		LUA->PushNil();
		return 1;
	}

	IEntity *ent = (IEntity*)g_pEntList->GetClientEntity(LUA->GetNumber(1));

	if (!ent)
	{
		LUA->PushNil();
		return 1;
	}

	ClientClass *cc = ent->GetClientClass();

	if (!cc || !cc->m_pNetworkName)
	{
		LUA->PushNil();
		return 1;
	}

	int len = strlen(cc->m_pNetworkName);

	if (1>len||len>255)
	{
		LUA->PushNil();
		return 1;
	}

	GarrysMod::Lua::UserData* ud = (GarrysMod::Lua::UserData*)LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
	ud->type = GarrysMod::Lua::Type::ENTITY;
	ud->data = ent;

	LUA->CreateMetaTableType("Entity", GarrysMod::Lua::Type::ENTITY);
	LUA->SetMetaTable(-2);



	return 1;
}

int lua_GetLocalPlayer(lua_State *state)
{
	IEntity *ent = (IEntity*)g_pEntList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!ent)
	{
		LUA->PushNil();
		return 1;
	}

	GarrysMod::Lua::UserData* ud = LUA->NewUserdata(sizeof(GarrysMod::Lua::UserData));
	
	if (!ud)
	{
		LUA->PushNil();
		return 1;
	}

	ud->type = GarrysMod::Lua::Type::ENTITY;
	ud->data = ent;

	LUA->CreateMetaTableType("Player", GarrysMod::Lua::Type::ENTITY);
	LUA->SetMetaTable(-2);

	return 1;
}

int lua_GetLocalPlayerIndex(lua_State *state)
{
	LUA->PushNumber(g_pEngine->GetLocalPlayer());
	return 1;
}

int lua_GetViewAngles(lua_State *state)
{
	QAngle va(0, 0, 0);
	g_pEngine->GetViewAngles(va);

	LUA_PUSHANG(va, angud);

	return 1;
}



int lua_SetName(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING))
		return 1;

	if (!g_pEngine->IsInGame())
		return 1;

	INetChannel* chan = (INetChannel*)g_pEngine->GetNetChannelInfo();

	if (!chan)
		return 1;

	char sendbuf[1024];
	bf_write pck(sendbuf, 1024);
	pck.WriteUBitLong(5, 6);
	pck.WriteByte(1);
	pck.WriteString("name");
	pck.WriteString(LUA->GetString(1));

	return chan->SendData(pck);
}

int lua_SetConVar(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING) || !LUA->IsType(2, GarrysMod::Lua::Type::STRING))
		return 1;

	ConVar *var = g_pCVar->FindVar(LUA->GetString(1));

	if (var)
	{
		var->SetValue(LUA->GetString(2));
		return 1;
	}

	return 1;
}

//metatables
static int lua_mtent__eq(lua_State* state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::USERDATA) || !LUA->IsType(2, GarrysMod::Lua::Type::USERDATA))
	{
		LUA->PushBool(false);
		return 1;
	}

	GarrysMod::Lua::UserData* ud = LUA->GetUserdata(1);
	if (!ud)
	{
		LUA->PushBool(false);
		return 1;
	}

	GarrysMod::Lua::UserData* ud2 = LUA->GetUserdata(2);
	if (!ud2)
	{
		LUA->PushBool(false);
		return 1;
	}

	if (ud->data == ud2->data)
	{
		LUA->PushBool(true);
		return 1;
	}

	LUA->PushBool(false);
	return 1;
}



static int lua_mtent__tostring(lua_State* state)
{
	if (!state || !LUA->IsType(1, GarrysMod::Lua::Type::USERDATA))
		return 1;

	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	char ret[64];

	sprintf(ret, "Data [%x]", (char*)pEnt);
	LUA->PushString(ret);

	return 1;
}

int lua_usercmd_GetCommandNumber(lua_State *state)
{
	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	LUA->PushNumber(pCmd->command_number);
	return 1;
}

int lua_usercmd_SetCommandNumber(lua_State *state)
{
	if (!LUA->IsType(2, GarrysMod::Lua::Type::NUMBER))
	{
		return 1;
	}

	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	pCmd->command_number = LUA->GetNumber(2);
	return 1;
}

int lua_usercmd_GetTickCount(lua_State *state)
{
	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	LUA->PushNumber(pCmd->tick_count);
	return 1;
}

int lua_usercmd_SetTickCount(lua_State *state)
{
	if (!LUA->IsType(2, GarrysMod::Lua::Type::NUMBER))
	{
		return 1;
	}

	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	pCmd->tick_count = LUA->GetNumber(2);
	return 1;
}

int lua_usercmd_GetViewAngles(lua_State *state)
{
	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	LUA_PUSHANG(pCmd->viewangles, angud);
	return 1;
}

int lua_usercmd_SetViewAngles(lua_State *state)
{
	if (!LUA->IsType(2, GarrysMod::Lua::Type::ANGLE))
	{
		return 1;
	}

	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	pCmd->viewangles = *(QAngle*)(LUA->GetUserdata(2))->data;
	return 1;
}

int lua_usercmd_GetMove(lua_State *state)
{
	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	LUA_PUSHANG(pCmd->move, angud);
	return 1;
}

int lua_usercmd_SetMove(lua_State *state)
{
	if (!LUA->IsType(2, GarrysMod::Lua::Type::ANGLE))
	{
		return 1;
	}

	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	QAngle &moveang = *reinterpret_cast<QAngle*>(mlua->GetUserdata(2));


	pCmd->move = moveang;
	return 1;
}

int lua_usercmd_GetButtons(lua_State *state)
{
	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	LUA->PushNumber(pCmd->buttons);
	return 1;
}

int lua_usercmd_SetButtons(lua_State *state)
{
	if (!LUA->IsType(2, GarrysMod::Lua::Type::NUMBER))
	{
		return 1;
	}

	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	pCmd->buttons = LUA->GetNumber(2);
	return 1;
}

int lua_usercmd_GetImpulse(lua_State *state)
{
	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	LUA->PushNumber(pCmd->impulse);
	return 1;
}

int lua_usercmd_SetImpulse(lua_State *state)
{
	if (!LUA->IsType(2, GarrysMod::Lua::Type::NUMBER))
	{
		return 1;
	}

	CUserCmd* pCmd = (CUserCmd*)LUA->GetUserdata(1)->data;
	if (!pCmd)
		return 1;

	pCmd->impulse = LUA->GetNumber(2);
	return 1;
}

//entities
int lua_ent_IsDormant(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	LUA->PushBool(pEnt->IsDormant());
	return 1;
}

int lua_ent_IsAlive(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
	{
		LUA->PushBool(false);
		return 1;
	}

	if (pEnt->GetHealth() > 0 && pEnt->GetLifeState() == 0)
	{
		LUA->PushBool(true);
	}else{
		LUA->PushBool(false);
	}


	return 1;
}


int lua_ent_GetClass(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	LUA->PushString(pEnt->GetClassname());
	return 1;
}

int lua_ent_GetNetworkClass(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt || !pEnt->GetClientClass() || !pEnt->GetClientClass()->m_pNetworkName)
		return 1;

	LUA->PushString(pEnt->GetClientClass()->m_pNetworkName);

	return 1;
}


int lua_ent_EntIndex(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt || !pEnt->GetClientClass() || !pEnt->GetClientClass()->m_pNetworkName)
		return 1;

	LUA->PushNumber(pEnt->entindex());

	return 1;
}

int lua_ent_GetPos(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt || !pEnt->GetClientClass() || !pEnt->GetClientClass()->m_pNetworkName)
		return 1;

	Vector pos = pEnt->GetPosition();

	LUA_PUSHVEC(pos, udvec);

	return 1;
}


int lua_ent_GetAbsPos(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt || !pEnt->GetClientClass() || !pEnt->GetClientClass()->m_pNetworkName)
		return 1;

	Vector pos = pEnt->GetAbsPosition();

	LUA_PUSHVEC(pos, udvec);

	return 1;
}

int lua_ent_GetHealth(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	LUA->PushNumber(pEnt->GetHealth());

	return 1;
}

int lua_ent_GetColor(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	int r, g, b, a;
	Color &col = pEnt->GetColor();
	col.GetColor(r, g, b, a);

	LUA->PushNumber(r);
	LUA->PushNumber(g);
	LUA->PushNumber(b);
	LUA->PushNumber(a);

	return 1;
}

int lua_ent_GetNextAttack(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	if (LUA->IsType(2, GarrysMod::Lua::Type::BOOL) && LUA->GetBool(2))
		LUA->PushNumber(pEnt->GetNextSecondaryAttack());
	else
		LUA->PushNumber(pEnt->GetNextPrimaryAttack());

	return 1;
}

int lua_ent_GetClip(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	if (LUA->IsType(2, GarrysMod::Lua::Type::BOOL) && LUA->GetBool(2))
		LUA->PushNumber(pEnt->GetClip2());
	else
		LUA->PushNumber(pEnt->GetClip1());

	return 1;
}

player_info_s plInfo;

int lua_ent_ToPlayer(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::ENTITY))
	{
		LUA->PushNil();
		return 1;
	}

	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt || !pEnt->GetClientClass())
	{
		LUA->PushNil();
		return 1;
	}

	g_pEngine->GetPlayerInfo(pEnt->entindex(), &plInfo);

	if (!plInfo.name)
	{
		LUA->PushNil();
		return 1;
	}

	LUA->CreateMetaTableType("Player", GarrysMod::Lua::Type::ENTITY);
	LUA->SetMetaTable(-2);

	return 1;
}

int lua_pent_GetFlags(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	LUA->PushNumber(*pEnt->GetFlags());

	return 1;
}

int lua_pent_GetNick(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	char *nick = 0;

	player_info_s pinfo;

	if (!g_pEngine->GetPlayerInfo(pEnt->entindex(), &pinfo))
		return 1;

	nick = pinfo.name;

	if (nick)
		LUA->PushString(nick);

	return 1;
}

int lua_pent_GetFriendID(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	unsigned int id = 0;

	player_info_s pinfo;

	if (!g_pEngine->GetPlayerInfo(pEnt->entindex(), &pinfo))
		return 1;

	id = pinfo.friendsID;

	if (id)
		LUA->PushNumber(id);

	return 1;
}

int lua_pent_GetEyePos(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	Vector vec = pEnt->GetEyePosition();

	LUA_PUSHVEC(vec, udvec);

	return 1;
}

int lua_pent_GetAbsEyePos(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	Vector vec = pEnt->GetAbsEyePosition();

	LUA_PUSHVEC(vec, udvec);

	return 1;
}

int lua_pent_GetTickBase(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	LUA->PushNumber(pEnt->GetTickBase());

	return 1;
}

int lua_pent_GetViewPunch(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	LUA_PUSHANG(pEnt->GetViewPunch(), udang);

	return 1;
}

int lua_pent_GetWeapon(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	int wepindex = pEnt->GetActiveWeaponIndex();

	if (!wepindex)
		return 1;

	LUA->PushNumber(wepindex);



	return 1;
}

int lua_pent_GetTeam(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;


	LUA->PushNumber(pEnt->GetTeamIndex());



	return 1;
}



int lua_pent_GetObserverMode(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;


	LUA->PushNumber(pEnt->GetObserverMode());



	return 1;
}



int lua_pent_GetObserverTarget(lua_State *state)
{
	IEntity* pEnt = (IEntity*)LUA->GetUserdata(1)->data;

	if (!pEnt)
		return 1;

	IEntity *target = pEnt->GetObserverTarget();

	if (target)
		LUA->PushNumber(target->entindex());


	return 1;
}


char centstoboneupdate[23][100];
int	ientstoboneupdate = 0;

int lua_AddBoneUpdate(lua_State *state)
{
	if (!LUA->IsType(1, GarrysMod::Lua::Type::STRING))
	{
		LUA->PushNil();
		return 1;
	}

	if (!ientstoboneupdate)
	{
		memset(centstoboneupdate, 0, sizeof(centstoboneupdate));
	}

	ientstoboneupdate++;

	strcpy(centstoboneupdate[ientstoboneupdate], LUA->GetString(1));

	return 1;

}

void __stdcall hooked_FrameStageNotify(ClientFrameStage_t stage)
{
	if (stage == 5 && ientstoboneupdate&&g_pEngine->IsInGame())
	{


		int highest = g_pEntList->GetHighestEntityIndex();

		if (!highest)
			return g_pClient->FrameStageNotify(stage);


		for (int i = 0; highest >i ; i++)
		{
			IEntity *ent = (IEntity*)g_pEntList->GetClientEntity(i);

			if (!ent)
				continue;

			ClientClass *cc = ent->GetClientClass();

			if (!cc)
				continue;
		
			if (!cc->m_pNetworkName)
				continue;

			int len = strlen(cc->m_pNetworkName);

			if (len == 0 || len > 0xFF)
				continue;

			for (int a = 1; a < ientstoboneupdate; a++)
			{
				if (centstoboneupdate[a] )
				{
					if (strstr(cc->m_pNetworkName, centstoboneupdate[a]))
					{
						ent->UpdateBones();
						break;
					}
				}

			}

		}
	}

	return g_pClient->FrameStageNotify(stage);
}

std::string m_hooks::HookStuff()
{
	BOOL success = SHGetSpecialFolderPathA(NULL, synchronicityfolder, CSIDL_MYDOCUMENTS, false);
	if (!success)
	{
		return "SHGetSpecialFolderPath failed!\n";
	}
	strcat(synchronicityfolder, "\\Synchronicity");

	CreateDirectoryA(synchronicityfolder, NULL);

	strcat(synchronicityfolder, "\\");

	memset(ientity__matrixs, 0, sizeof(ientity__matrixs));
	memset(ientity__hboxsets, 0, sizeof(ientity__hboxsets));

	VMTHook *engineHook = new VMTHook(g_pEngine);
	g_pEngine = (IVEngineClient*)&engineHook->m_pOriginalVTable;

	VMTHook *clientHook = new VMTHook(g_pClient);
	g_pClient = (IBaseClientDLL*)&clientHook->m_pOriginalVTable;

	clientHook->hookFunction(35, hooked_FrameStageNotify);

	//VMTHook *predHook = new VMTHook(g_pPrediction);
	//g_pPrediction = (IPred*)&predHook->m_pOriginalVTable;


	void**vclient = *(void***)g_pClient;
	void* clientmode = **(void***)((char*)vclient[10] + 0x5);

	VMTHook *modeHook = new VMTHook(clientmode);
	CreateMoveFn = (OrigCreateMove)modeHook->hookFunction(21, hooked_CreateMove);


	//predHook->hookFunction(14, hooked_InPrediction);


	VMTHook *luasharedhook = new VMTHook(g_pLuaShared->realluashared);
	CreateLuaInterfaceFn = (OrigCreateLuaInterface)luasharedhook->hookFunction(4, hooked_CreateLuaInterface);

	void*vluastate = g_pLuaShared->GetLuaInterface(2);
	ILuaInterface *lua = (ILuaInterface*)vluastate;
	menuluastate = lua;
	
	Sleep(3);

	lua->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);

	lua->CreateTable();

	//table start

	//folder stuff

	lua->PushCFunction(lua_IsConnected);
	lua->SetField(-2, "IsConnected");

	lua->PushCFunction(lua_IsDrawingLoadingScreen);
	lua->SetField(-2, "IsDrawingLoadingScreen");

	lua->PushCFunction(lua_IsInGame);
	lua->SetField(-2, "IsInGame");

	lua->PushCFunction(lua_IsVisible);
	lua->SetField(-2, "IsVisible");

	lua->PushCFunction(lua_CalculateAimVector);
	lua->SetField(-2, "CalculateAimVector");

	lua->PushCFunction(lua_CreateFolder);
	lua->SetField(-2, "CreateFolder");

	lua->PushCFunction(lua_ReadFromSN);
	lua->SetField(-2, "FileRead");

	lua->PushCFunction(lua_WriteToSN);
	lua->SetField(-2, "FileWrite");

	lua->PushCFunction(lua_RunString);
	lua->SetField(-2, "RunString");

	lua->PushCFunction(lua_RunCommand);
	lua->SetField(-2, "RunCommand");

	lua->PushCFunction(lua_ClientCommand);
	lua->SetField(-2, "ClientCommand");

	lua->PushCFunction(lua_Disconnect);
	lua->SetField(-2, "Disconnect");

	lua->PushCFunction(lua_GetMap);
	lua->SetField(-2, "GetMap");

	lua->PushCFunction(lua_GetIP);
	lua->SetField(-2, "GetIP");

	lua->PushCFunction(lua_GetMaxEnts);
	lua->SetField(-2, "GetMaxEnts");

	lua->PushCFunction(lua_GetMaxPlayers);
	lua->SetField(-2, "GetMaxPlayers");

	lua->PushCFunction(lua_GetEntity);
	lua->SetField(-2, "GetEntity");

	lua->PushCFunction(lua_GetLocalPlayer);
	lua->SetField(-2, "GetLocalPlayer");

	lua->PushCFunction(lua_GetLocalPlayerIndex);
	lua->SetField(-2, "GetLocalPlayerIndex");

	lua->PushCFunction(lua_GetViewAngles);
	lua->SetField(-2, "GetViewAngles");

	lua->PushCFunction(lua_SetName);
	lua->SetField(-2, "SetName");

	lua->PushCFunction(lua_SetConVar);
	lua->SetField(-2, "SetConVar");

	lua->PushCFunction(lua_SetHookFunc);
	lua->SetField(-2, "SetHookFunc");

	lua->PushCFunction(lua_SetVisualRecoil);
	lua->SetField(-2, "SetVisualRecoil");

	lua->PushCFunction(lua_SetViewAngles);
	lua->SetField(-2, "SetViewAngles");

	lua->PushCFunction(lua_VectorAngles);
	lua->SetField(-2, "VectorAngles");

	lua->PushCFunction(lua_NormalizeAngles);
	lua->SetField(-2, "NormalizeAngles");

	lua->PushCFunction(lua_AddBoneUpdate);
	lua->SetField(-2, "AddBoneUpdate");


	//table end

	lua->SetField(-2, "SN");


	lua->Pop();

	//create cusercmd
	lua->CreateMetaTableType("CUserCmd", GarrysMod::Lua::Type::USERCMD);
	ILuaObject *cmdmetaT = lua->GetMetaTableObject("CUserCmd", GarrysMod::Lua::Type::USERCMD);
	//normal stuff

	lua->PushCFunction(lua_mtent__eq);
	lua->SetField(-2, "__eq");

	lua->PushLuaObject(cmdmetaT);
	lua->SetField(-2, "__index");

	lua->PushLuaObject(cmdmetaT);
	lua->SetField(-2, "__newindex");

	lua->PushCFunction(lua_mtent__tostring);
	lua->SetField(-2, "__tostring");

	int ref = lua->ReferenceCreate();

	lua->ReferencePush(ref);

	//funcs
	lua->PushCFunction(lua_usercmd_GetCommandNumber);
	lua->SetField(-2, "GetCommandNumber");

	lua->PushCFunction(lua_usercmd_SetCommandNumber);
	lua->SetField(-2, "SetCommandNumber");

	lua->PushCFunction(lua_usercmd_GetTickCount);
	lua->SetField(-2, "GetTickCount");

	lua->PushCFunction(lua_usercmd_SetTickCount);
	lua->SetField(-2, "SetTickCount");

	lua->PushCFunction(lua_usercmd_GetViewAngles);
	lua->SetField(-2, "GetViewAngles");

	lua->PushCFunction(lua_usercmd_SetViewAngles);
	lua->SetField(-2, "SetViewAngles");

	lua->PushCFunction(lua_usercmd_GetMove);
	lua->SetField(-2, "GetMove");

	lua->PushCFunction(lua_usercmd_SetMove);
	lua->SetField(-2, "SetMove");

	lua->PushCFunction(lua_usercmd_GetButtons);
	lua->SetField(-2, "GetButtons");

	lua->PushCFunction(lua_usercmd_SetButtons);
	lua->SetField(-2, "SetButtons");

	lua->PushCFunction(lua_usercmd_GetImpulse);
	lua->SetField(-2, "GetImpulse");

	lua->PushCFunction(lua_usercmd_SetImpulse);
	lua->SetField(-2, "SetImpulse");

	lua->ReferenceFree(ref);

	lua->Pop();

	//create entitys
	lua->CreateMetaTableType("Entity", GarrysMod::Lua::Type::ENTITY);
	ILuaObject *entmetaT = lua->GetMetaTableObject("Entity", GarrysMod::Lua::Type::ENTITY);
	//normal stuff

	lua->PushCFunction(lua_mtent__eq);
	lua->SetField(-2, "__eq");

	lua->PushLuaObject(entmetaT);
	lua->SetField(-2, "__index");

	lua->PushLuaObject(entmetaT);
	lua->SetField(-2, "__newindex");

	lua->PushCFunction(lua_mtent__tostring);
	lua->SetField(-2, "__tostring");

	ref = lua->ReferenceCreate();

	lua->ReferencePush(ref);

	//funcs
	lua->PushCFunction(lua_ent_IsDormant);
	lua->SetField(-2, "IsDormant");

	lua->PushCFunction(lua_ent_IsAlive);
	lua->SetField(-2, "IsAlive");

	lua->PushCFunction(lua_ent_GetClass);
	lua->SetField(-2, "GetClass");

	lua->PushCFunction(lua_ent_GetNetworkClass);
	lua->SetField(-2, "GetNetworkClass");

	lua->PushCFunction(lua_ent_EntIndex);
	lua->SetField(-2, "EntIndex");

	lua->PushCFunction(lua_ent_GetPos);
	lua->SetField(-2, "GetPos");

	lua->PushCFunction(lua_ent_GetAbsPos);
	lua->SetField(-2, "GetAbsPos");

	lua->PushCFunction(lua_ent_GetHealth);
	lua->SetField(-2, "GetHealth");

	lua->PushCFunction(lua_ent_GetColor);
	lua->SetField(-2, "GetColor");

	lua->PushCFunction(lua_ent_GetNextAttack);
	lua->SetField(-2, "GetNextAttack");

	lua->PushCFunction(lua_ent_GetClip);
	lua->SetField(-2, "GetClip");

	lua->PushCFunction(lua_ent_ToPlayer);
	lua->SetField(-2, "ToPlayer");


	lua->ReferenceFree(ref);

	lua->Pop();

	//create players
	lua->CreateMetaTableType("Player", GarrysMod::Lua::Type::ENTITY);
	ILuaObject *pmetaT = lua->GetMetaTableObject("Player", GarrysMod::Lua::Type::ENTITY);

	//normal stuff

	lua->PushCFunction(lua_mtent__eq);
	lua->SetField(-2, "__eq");

	lua->PushLuaObject(pmetaT);
	lua->SetField(-2, "__index");

	lua->PushLuaObject(pmetaT);
	lua->SetField(-2, "__newindex");

	lua->PushCFunction(lua_mtent__tostring);
	lua->SetField(-2, "__tostring");

	ref = lua->ReferenceCreate();

	lua->ReferencePush(ref);

	//funcs

	lua->PushCFunction(lua_pent_GetNick);
	lua->SetField(-2, "GetNick");

	lua->PushCFunction(lua_pent_GetFriendID);
	lua->SetField(-2, "GetFriendNick");

	lua->PushCFunction(lua_pent_GetFlags);
	lua->SetField(-2, "GetFlags");

	lua->PushCFunction(lua_pent_GetEyePos);
	lua->SetField(-2, "GetEyePos");

	lua->PushCFunction(lua_pent_GetAbsEyePos);
	lua->SetField(-2, "GetAbsEyePos");

	lua->PushCFunction(lua_pent_GetTickBase);
	lua->SetField(-2, "GetTickBase");

	lua->PushCFunction(lua_pent_GetViewPunch);
	lua->SetField(-2, "GetViewPunch");

	lua->PushCFunction(lua_pent_GetWeapon);
	lua->SetField(-2, "GetWeapon");

	lua->PushCFunction(lua_pent_GetTeam);
	lua->SetField(-2, "GetTeam");

	lua->PushCFunction(lua_pent_GetObserverTarget);
	lua->SetField(-2, "GetObserverTarget");

	lua->PushCFunction(lua_pent_GetObserverMode);
	lua->SetField(-2, "GetObserverMode");

	lua->ReferenceFree(ref);

	lua->Pop();

	Sleep(2);
	g_pLuaShared->RunCode(lua, "local pmeta = FindMetaTable('Player') for k,v in pairs(FindMetaTable('Entity')) do if(not pmeta[k]) then pmeta[k] = v end end", "[C]");
	Sleep(2);

	std::string readluafile = "";
	utils::file::FileRead("C://Synchronicity.lua", readluafile);
	char*luaerrors = g_pLuaShared->RunCode(lua, readluafile.c_str(), "[C]");
	if (luaerrors&&strlen(luaerrors)!=0)
		MessageBoxA(NULL, luaerrors, "k", MB_OK);

	/*
	bool a = HookNetvar("DT_HL2MP_Player", "m_angEyeAngles[0]", AntiAntiAimProxyX);
	bool b = HookNetvar("DT_HL2MP_Player", "m_angEyeAngles[1]", AntiAntiAimProxyY);

	if (a && b)
	Msg("Hooked eyeangles!\n");
	*/

	return "";
}