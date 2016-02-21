//Engine Stuff
#pragma once
#pragma comment(lib, "tier0.lib")

#define GMMODULELEAN

#include "stdafx.h"
#include <cbase.h>
#include <checksum_md5.h>
#include <in_buttons.h>
#include <inetchannel.h>
#include <inetmsghandler.h>
#include <iclient.h>

#include <input.h>
#include <igamemovement.h>
#include "Lua\Interface.h"
#include "Lua\iluashared.h"

class IPred;
class IEntity;

extern IEntity *g_pLocalEnt;
extern IBaseClientDLL *g_pClient;
extern IVEngineClient *g_pEngine;
extern IPred *g_pPrediction;
extern IClientEntityList *g_pEntList;
extern IEngineTrace *g_pEngineTrace;
extern IVModelInfoClient *g_pModelInfo;
extern ICvar *g_pCVar;
extern CGlobalVars *g_pGlobals;

extern std::string(InitializeSdk)();

static void**& getvtable(void* inst, size_t offset = 0)
{
	return *reinterpret_cast<void***>((size_t)inst + offset);
}

static const void** getvtable(const void* inst, size_t offset = 0)
{
	return *reinterpret_cast<const void***>((size_t)inst + offset);
}
template< typename Fn >

static Fn getvfunc(const void* inst, size_t index, size_t offset = 0)
{
	return reinterpret_cast<Fn>(getvtable(inst, offset)[index]);
}


static int CrawlTableForOffset(RecvTable *pTable, const char *name)
{
	int add_offset = 0;

	for (int i = 0; i < pTable->m_nProps; i++) {
		RecvProp *prop = &pTable->m_pProps[i];

		if (prop->m_pDataTable && prop->m_pDataTable->m_nProps > 0) {
			int offset = CrawlTableForOffset(prop->m_pDataTable, name);

			if (offset) {
				add_offset += prop->m_Offset + offset;
			}
		}
		const char *prop_name = prop->m_pVarName;

		int length = strlen(prop_name);
		if (strcmp(prop_name + length - 3, "[0]")) {
			length -= 3; //don't copy array index
		}
		if (strncmp(prop_name, name, length)) {
			continue;
		}
		return prop->m_Offset + add_offset;
	}
	return 0;
}

static int GetNetOffset(char* pClassName, char* pVarName, RecvTable* pTable)
{
	int offset = 0;

	for (int i = 0; i < pTable->m_nProps; i++)
	{
		RecvProp* pProp = &pTable->m_pProps[i];

		if (!pProp)
			continue;

		RecvTable* pChild = pProp->m_pDataTable;

		if (pChild && pChild->m_nProps)
			offset += GetNetOffset(pClassName, pVarName, pChild);

		if (!strcmp(pProp->m_pVarName, pVarName))
		{
			if (strstr(pClassName, "*") || !strcmp(pTable->m_pNetTableName, pClassName))
			{
				offset += pProp->m_Offset;
				break;
			}

		}
	}

	return offset;
}

static int GetNetvarOffset(char* pClassName, char* pVarName)
{
	ClientClass* clientClass = g_pClient->GetAllClasses();

	if (!clientClass)
		return NULL;

	for (; clientClass; clientClass = clientClass->m_pNext)
	{
		RecvTable* recvTable = clientClass->m_pRecvTable;

		if (!recvTable)
			continue;

		if (!recvTable->m_nProps)
			continue;

		if (!strstr(recvTable->m_pNetTableName, "DT_"))
			continue;

		int offset = GetNetOffset(pClassName, pVarName, recvTable);

		if (offset)
			return offset;
	}

	return NULL;
}

extern void VectorTransformR(const Vector &vecIn, const matrix3x4_t &matrixIn, Vector &vecOut);

inline void NormalizeAnglesR(QAngle &vecAngles)
{
	for (int iAxis = 0; iAxis < 3; iAxis++)
	{
		while (vecAngles[iAxis] > 180.f)
			vecAngles[iAxis] -= 360.f;

		while (vecAngles[iAxis] < -180.f)
			vecAngles[iAxis] += 360.f;
	}
}

inline void NormalizeVectorsR(Vector &vecAngles)
{
	for (int iAxis = 0; iAxis < 3; iAxis++)
	{
		while (vecAngles[iAxis] > 180.f)
			vecAngles[iAxis] -= 360.f;

		while (vecAngles[iAxis] < -180.f)
			vecAngles[iAxis] += 360.f;
	}
}

#define pi 3.1415925f

inline float Rad2Deg(const float &rad)
{
	return rad * (180.f / pi);
}

inline float Deg2Rad(const float &deg)
{
	return deg * (pi / 180.f);
}

inline static void VectorAnglesR(const Vector &vecDirection, QAngle &vecAngles)
{
	if (vecDirection.x == 0 && vecDirection.y == 0)
	{
		if (vecDirection.z > 0.f)
			vecAngles.x = 270.f;
		else
			vecAngles.x = 90.f;

		vecAngles.y = 0.f;
	}
	else
	{
		vecAngles.x = Rad2Deg(atan2(-vecDirection.z, sqrtf(vecDirection.x * vecDirection.x + vecDirection.y * vecDirection.y)));
		vecAngles.y = Rad2Deg(atan2(vecDirection.y, vecDirection.x));

		if (vecAngles.x < 0.f)
			vecAngles.x += 360.f;

		if (vecAngles.y < 0.f)
			vecAngles.y += 360.f;
	}

	vecAngles.z = 0.f;
	NormalizeAnglesR(vecAngles);
}

inline static void VectorAnglesR(const Vector &vecDirection, const Vector &vecUp, Vector &vecAngles)
{
	Vector vecLeft;
	CrossProduct(vecUp, vecDirection, vecLeft);
	NormalizeVectorsR(vecLeft);

	float flLength2D = sqrtf((vecDirection.x * vecDirection.x) + (vecDirection.y * vecDirection.y));
	vecAngles.x = Rad2Deg(atan2(-vecDirection.z, flLength2D));

	if (flLength2D > .001f)
	{
		vecAngles.y = Rad2Deg(atan2(vecDirection.y, vecDirection.x));

		// todo: if > 84 || < -84, sqrt / dunno if this is needed
		float flUp = ((vecLeft.y * vecDirection.x) - (vecLeft.x * vecDirection.y));
		vecAngles.z = Rad2Deg(atan2(vecLeft.z, flUp));
	}
	else
	{
		vecAngles.y = Rad2Deg(atan2(-vecLeft.x, vecLeft.y));
		vecAngles.z = 0.f;
	}

	NormalizeVectorsR(vecAngles);
}

class IPred
{
public:

	inline bool InPrediction()
	{
		typedef bool(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(this, 14)(this);
	}

	inline void RunCommand(IEntity *ent, CUserCmd* pCmd, void* mv)
	{
		typedef void(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(this, 17)(this);
	}
};

#define clientrenderable() (void*)((char*)this + 0x4)
#define clientnetworkable() (void*)((char*)this + 0x8)

extern mstudiohitboxset_t* ientity__hboxsets[0xFFFF];
extern matrix3x4_t ientity__matrixs[0xFFFF][128];

typedef const char* (__thiscall* origGetClassname)(void* thisptr);
static origGetClassname GetClassnameFn;
typedef model_t*(__thiscall* GetModelFn)(void*);
class IEntity : public C_BaseEntity
{

public:
	inline bool UpdateBones()
	{
		int index = entindex();
		
		if (!index)
			return false;

		model_t *mdl = GetModel();
		if (!mdl)
			return false;

		if (index > 0xFFFF)
			return false;

		studiohdr_t* stdhdr = g_pModelInfo->GetStudiomodel(mdl);

		if (!stdhdr)
			return false;

		
		if (!SetupBones(ientity__matrixs[index], 128, 0x00000100, GetSimulationTime()))
			return false;

		
		mstudiohitboxset_t *hitbox = stdhdr->pHitboxSet(0);
		if (!hitbox)
			return false;

		ientity__hboxsets[index] = hitbox;

		if (!ientity__hboxsets[index])
			return false;

		return true;
	}

	inline mstudiohitboxset_t* GetHBoxSet()
	{
		int index = entindex();
		if (!index)
			return 0;

		mstudiohitboxset_t* set = ientity__hboxsets[index];

		return set;
	}

	inline matrix3x4_t* GetMatrix()
	{
		int index = entindex();
		
		if (!index)
			return 0;

		return ientity__matrixs[index];
	}


	inline QAngle& GetViewPunch()
	{
		return *(QAngle*)((char*)(this) + 0x22A0);
	}

	inline Vector& GetPosition()
	{
		static int offset = GetNetvarOffset("DT_BaseEntity", "m_vecOrigin");
		return *(Vector*)((char*)(this) + offset);
	}

	inline Vector& GetAbsPosition()
	{
		typedef Vector&(__thiscall* OriginalFn)(void*); 
		return getvfunc<OriginalFn>(clientrenderable(), 1)(clientrenderable());

	}


	inline bool SetupBones(matrix3x4_t* matrix, int bones, int mask, float time)
	{
		static auto OriginalFn = (bool(__thiscall *)(void *, matrix3x4_t *, int, int, float))utils::memory::FindSubStart(utils::memory::FindString(GetModuleHandleA("client.dll"), "*** ERROR: Bone access not allowed (entity %i:%s)\n"));

		return OriginalFn(clientrenderable(), matrix, bones, mask, time);
	}

	inline model_t* GetModel()
	{
		typedef model_t*(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(clientrenderable(), 9)(clientrenderable());
	}

	inline bool IsDormant()
	{
		typedef bool(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(clientnetworkable(), 8)(clientnetworkable());
	}

	inline int entindex()
	{
		typedef int (__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(clientnetworkable(), 9)(clientnetworkable());
	}

	inline ClientClass* GetClientClass()
	{
		typedef ClientClass*(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(clientnetworkable(), 2)(clientnetworkable());
	}

	inline BYTE GetLifeState()
	{
		static int offset = GetNetvarOffset("DT_BasePlayer", "m_lifeState");
		return *(BYTE*)(this + offset);
	}

	inline Vector& GetVelocity()
	{
		return *(Vector*)((char*)(this) + 0xF4);
	}

	inline float &GetNextPrimaryAttack()
	{
		static int offset = GetNetvarOffset("*", "m_flNextPrimaryAttack");

		return *(float*)((char*)(this) + offset);
	}

	inline float &GetNextSecondaryAttack()
	{
		static int offset = GetNetvarOffset("*", "m_flNextSecondaryAttack");

		return *(float*)((char*)(this) + offset);
	}

	inline int &GetClip1()
	{
		static int offset = GetNetvarOffset("*", "m_iClip1");

		return *(int*)((char*)(this) + offset);
	}

	inline int &GetClip2()
	{
		static int offset = GetNetvarOffset("*", "m_iClip1");

		return *(int*)((char*)(this) + offset);
	}

	inline int &GetTeamIndex()
	{
		return *(int*)((char*)(this) + 0x9C);
	}

	inline Color& GetColor()
	{
		return *(Color*)((char*)(this) + 0x58);
	}

	inline int& GetTickBase()
	{
		return *(int*)((char*)(this) + 0x2558);
	}

	inline Vector& EyeAnglesP()
	{ 
		return *(Vector*)((char*)(this) + 0x2558);
	}

	inline int& GetHealth()
	{
		return *(int*)((char*)(this) + 0x90);
	}

	inline int& GetSimulationTime()
	{
		static int offset = GetNetvarOffset("*", "m_flSimulationTime");

		return *(int*)((char*)(this) + offset);
	}

	inline int* GetFlags()
	{
		return (int*)((char*)(this) + 0x350);
	}

	inline Vector GetEyePosition()
	{
		static int offset = GetNetvarOffset("*", "m_vecViewOffset[0]");
		Vector m_vecViewOffset = *(Vector*)((char*)(this) + offset);

		return GetPosition() + m_vecViewOffset;
	}

	inline Vector GetAbsEyePosition()
	{
		static int offset = GetNetvarOffset("*", "m_vecViewOffset[0]");
		Vector m_vecViewOffset = *(Vector*)((char*)(this) + offset);

		return GetAbsPosition() + m_vecViewOffset;
	}

	inline HANDLE GetActiveWeaponH()
	{
		static int offset = GetNetvarOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");

		return *(HANDLE*)((char*)(this) + offset); // needs proper offset
	}

	inline int IEntity::GetActiveWeaponIndex()
	{
		HANDLE hWeapon = GetActiveWeaponH();

		if (!hWeapon)
			return NULL;

		int iWeapon = (unsigned long)hWeapon & 0xFFF;

		return iWeapon;
	}

	inline IEntity* GetObserverTarget()
	{
		static int offset = GetNetvarOffset("*", "m_hObserverTarget");

		HANDLE hdnl = *(HANDLE*)((char*)(this) + offset);

		return (IEntity*)g_pEntList->GetClientEntityFromHandle(hdnl);
	}

	inline int GetObserverMode()
	{
		static int offset = GetNetvarOffset("*", "m_iObserverMode");

		return *(int*)((char*)(this) + offset);
	}

	inline const char* GetClassname()
	{

		if (!GetClassnameFn)
		{
			void* ref = (char*)utils::memory::FindString(GetModuleHandleA("client.dll"), "C_BaseAnimating::SetupBones") + 82;
			GetClassnameFn = (origGetClassname)utils::memory::CalcAbsAddress(ref);

		}

		return GetClassnameFn(this);
	}

};

static bool CrawlTableForHook(RecvTable *pTable, const char *name, void* hook)
{
	bool ret = false;

	for (int i = 0; i < pTable->m_nProps; i++)
	{
		RecvProp *pProp = &pTable->m_pProps[i];

		if (pProp->m_pDataTable && pProp->m_pDataTable->m_nProps > 0)
		{
			ret = CrawlTableForHook(pProp->m_pDataTable, name, hook);
		}


		if (strcmp(name, pProp->m_pVarName))
		{
			pProp->m_ProxyFn = hook;
			ret = true;
		}

	}
	return ret;
}

static int HookNetvar(const char *table, const char *name, void*hook)
{
	ClientClass *pClass = g_pClient->GetAllClasses();
	bool ret = false;

	for (; pClass; pClass = pClass->m_pNext)
	{
		RecvTable *pTable = pClass->m_pRecvTable;

		if (!pTable)
			continue;

		if (pTable->m_nProps <= 1)
			continue;

		if (table && stricmp(table, pTable->m_pNetTableName))
			continue;

		bool bResult = CrawlTableForHook(pTable, name, hook);

		if (bResult)
			ret = true;
	}
	return ret;
}
