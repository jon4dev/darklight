#include <random>
#include <algorithm>

#include "misc.h"
#include "../global.h"
#include "../core/variables.h"
#include "../core/interfaces.h"
#include "../utilities/math.h"
#include "../utilities.h"
#include "../core/gui/gui.h"
#include "prediction.h"

CMiscellaneous g_Misc;

void CMiscellaneous::Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket)
{
	MouseDelta(pCmd, pLocal);
	ClantagChanger(pCmd, pLocal);

	if (pLocal->IsAlive())
		Bunnyhop(pCmd, pLocal);
}

void CMiscellaneous::Bunnyhop(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static CConVar* sv_autobunnyhopping = I::ConVar->FindVar(_("sv_autobunnyhopping"));
	if (sv_autobunnyhopping->GetBool() || !C::Get<int>(Vars.iBunnyHop))
		return;

	if (pLocal->GetMoveType() == MOVETYPE_LADDER || pLocal->GetMoveType() == MOVETYPE_NOCLIP || pLocal->GetMoveType() == MOVETYPE_OBSERVER)
		return;

	static float flHitchance, flMax = 0;
	if (C::Get<int>(Vars.iBunnyHop) == 2)
	{
		flHitchance = C::Get<float>(Vars.flBunnyhopHitchance);
		flMax = 0;
	}
	else
		flHitchance = 100, flMax = 0;

	static int hops_hit = 0;
	if (!(pCmd->iButtons & IN_JUMP) ||
		flHitchance <= 0)
		return;

	if (!(pLocal->GetFlags() & FL_ONGROUND))
		pCmd->iButtons &= ~IN_JUMP;
	else if (
		(rand() % 100 > flHitchance) ||
		(flMax > 0 && hops_hit > flMax))
	{
		pCmd->iButtons &= ~IN_JUMP;
		hops_hit = 0;
	}
	else
		hops_hit++;
}

void CMiscellaneous::ClantagChanger(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static bool bShouldClear = false, bShouldNotClear = true;

	static float flPrevtime = 0;

	if (!I::Engine->IsConnected())
		flPrevtime = 0;

	if (C::Get<bool>(Vars.bClantagChanger))
	{
		static std::string starter = _("[$] darklight ");
		static std::string current(starter.size(), ' ');
		static int position = 0, string_size = starter.size();

		if (I::Globals->flCurrentTime - flPrevtime > 0.2f)
		{
			if (position < string_size)
			{
				if (position < string_size - 1)
					current[position] = '>';

				if (position)
					current[position - 1] = starter[position - 1];
			}
			else if (position >= string_size)
			{
				int cur = string_size - (position - string_size + 1);
				if (cur > string_size / 2 - 1)
					current[cur] = '<';
				else
					current[cur] = '>';
			}

			position++;

			if (position == string_size * 2)
				position = 0;

			U::SendClanTag(current.c_str(), _("$$$"));

			flPrevtime = I::Globals->flCurrentTime;
		}
	}
	else 
	{
		bShouldClear = true;
		if (bShouldClear && !bShouldNotClear)
		{
			U::SendClanTag(_(""), _(""));
			bShouldNotClear = true;
		}
	}
}

void CMiscellaneous::MouseDelta(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static QAngle angDeltaAngles;
	QAngle angDelta = pCmd->angViewPoint - angDeltaAngles;
	angDelta.Clamp();

	static auto pSensitivity = I::ConVar->FindVar(_("sensitivity"));
	if (!pSensitivity)
		return;

	if (angDelta.x != 0.f) {
		static auto pPitch = I::ConVar->FindVar(_("m_pitch"));
		if (!pPitch)
			return;

		int iFinalDeltaY = (int)((angDelta.x / pPitch->GetFloat()) / pSensitivity->GetFloat());
		if (iFinalDeltaY <= 32767) {
			if (iFinalDeltaY >= -32768) {
				if (iFinalDeltaY >= 1 || iFinalDeltaY < 0) {
					if (iFinalDeltaY <= -1 || iFinalDeltaY > 0)
						iFinalDeltaY = iFinalDeltaY;
					else
						iFinalDeltaY = -1;
				}
				else
					iFinalDeltaY = 1;
			}
			else
				iFinalDeltaY = 32768;
		}
		else
			iFinalDeltaY = 32767;

		pCmd->sMouseDeltaY = (short)iFinalDeltaY;
	}

	if (angDelta.y != 0.f) {
		static auto pYaw = I::ConVar->FindVar(_("m_yaw"));
		if (!pYaw)
			return;

		int iFinalDeltaX = (int)((angDelta.y / pYaw->GetFloat()) / pSensitivity->GetFloat());
		if (iFinalDeltaX <= 32767) {
			if (iFinalDeltaX >= -32768) {
				if (iFinalDeltaX >= 1 || iFinalDeltaX < 0) {
					if (iFinalDeltaX <= -1 || iFinalDeltaX > 0)
						iFinalDeltaX = iFinalDeltaX;
					else
						iFinalDeltaX = -1;
				}
				else
					iFinalDeltaX = 1;
			}
			else
				iFinalDeltaX = 32768;
		}
		else
			iFinalDeltaX = 32767;

		pCmd->sMouseDeltaX = (short)iFinalDeltaX;
	}

	angDeltaAngles = pCmd->angViewPoint;
}

void CMiscellaneous::CorrectMovement(CBaseEntity* pLocal, CUserCmd* pCmd, QAngle& angOldViewPoint)
{
	pCmd->angViewPoint.Normalize();
	pCmd->angViewPoint.Clamp();

	Vector vecForward, vecRight, vecUp, vecForwardOld, vecRightOld, vecUpOld;
	M::AngleVectors(G::cmd.angViewPoint, &vecForward, &vecRight, &vecUp);
	M::AngleVectors(angOldViewPoint, &vecForwardOld, &vecRightOld, &vecUpOld);

	Vector vecMoveData = { G::cmd.flForwardMove, G::cmd.flSideMove, G::cmd.flUpMove };
	if (!(pLocal->GetFlags() & FL_ONGROUND) && angOldViewPoint.z != 0.0f)
		vecMoveData.y = 0.0f;

	const float flFwdLenght = vecForward.Length2D();
	const float flRightLenght = vecRight.Length2D();
	const float flUpLenght = std::sqrtf(vecUp.z * vecUp.z);

	const Vector vecNormFwd = Vector(1.f / flFwdLenght * vecForward.x, 1.f / flFwdLenght * vecForward.y, 0.f);
	const Vector vecNormRight = Vector(1.f / flRightLenght * vecRight.x, 1.f / flRightLenght * vecRight.y, 0.f);
	const Vector vecNormUp = Vector(0.f, 0.f, 1.f / flUpLenght * vecUp.z);

	const float flFwdOldLenght = vecForwardOld.Length2D();
	const float flRightOldLenght = vecRightOld.Length2D();
	const float flUpOldLenght = std::sqrtf(vecUpOld.z * vecUpOld.z);

	const Vector vecNormFwdOld(1.f / flFwdOldLenght * vecForwardOld.x, 1.f / flFwdOldLenght * vecForwardOld.y, 0.f);
	const Vector vecNormRightOld(1.f / flRightOldLenght * vecRightOld.x, 1.f / flRightOldLenght * vecRightOld.y, 0.f);
	const Vector vecNormUpOld(0.f, 0.f, 1.f / flUpOldLenght * vecUpOld.z);

	const auto
		v22 = vecNormFwd.x * vecMoveData.x,
		v26 = vecNormFwd.y * vecMoveData.x,
		v28 = vecNormFwd.z * vecMoveData.x,
		v24 = vecNormRight.x * vecMoveData.y,
		v23 = vecNormRight.y * vecMoveData.y,
		v25 = vecNormRight.z * vecMoveData.y,
		v30 = vecNormUp.x * vecMoveData.z,
		v27 = vecNormUp.z * vecMoveData.z,
		v29 = vecNormUp.y * vecMoveData.z;

	const float x = vecNormFwdOld.x * v24 + vecNormFwdOld.y * v23 + vecNormFwdOld.z * v25
		+ (vecNormFwdOld.x * v22 + vecNormFwdOld.y * v26 + vecNormFwdOld.z * v28)
		+ (vecNormFwdOld.y * v30 + vecNormFwdOld.x * v29 + vecNormFwdOld.z * v27);

	const float y = vecNormRightOld.x * v24 + vecNormRightOld.y * v23 + vecNormRightOld.z * v25
		+ (vecNormRightOld.x * v22 + vecNormRightOld.y * v26 + vecNormRightOld.z * v28)
		+ (vecNormRightOld.x * v29 + vecNormRightOld.y * v30 + vecNormRightOld.z * v27);

	const float z = vecNormUpOld.x * v23 + vecNormUpOld.y * v24 + vecNormUpOld.z * v25
		+ (vecNormUpOld.x * v26 + vecNormUpOld.y * v22 + vecNormUpOld.z * v28)
		+ (vecNormUpOld.x * v30 + vecNormUpOld.y * v29 + vecNormUpOld.z * v27);

	pCmd->flForwardMove = std::clamp(x, -450.f, 450.f);
	pCmd->flSideMove = std::clamp(y, -450.f, 450.f);
	pCmd->flUpMove = std::clamp(z, -450.f, 450.f);
}