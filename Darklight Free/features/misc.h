#pragma once

#include "../common.h"
#include "../sdk/datatypes/usercmd.h"
#include "../sdk/interfaces/igameeventmanager.h"
#include "../sdk/entity.h"

class CMiscellaneous
{
public:
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket);

	void CorrectMovement(CBaseEntity* pLocal, CUserCmd* pCmd, QAngle& angOldViewPoint);

private:
	void Bunnyhop(CUserCmd* pCmd, CBaseEntity* pLocal);
	void ClantagChanger(CUserCmd* pCmd, CBaseEntity* pLocal);
	void MouseDelta(CUserCmd* pCmd, CBaseEntity* pLocal);
};

extern CMiscellaneous g_Misc;