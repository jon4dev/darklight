#include "aimbot.h"
#include "../utilities/math.h"
#include "../core/variables.h"
#include "../core/interfaces.h"
#include "../core/gui/gui.h"
#include "../utilities.h"
#include "backtrack.h"
#include "misc.h"

CAimbot g_Aimbot;

QAngle angOldAng;

void CAimbot::Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket)
{
	ClearAimbot();

	static bool bToggled = false;

	m_pCmd = pCmd;
	if (!m_pCmd)
		return;

	m_pLocal = pLocal;
	if (!m_pLocal || !m_pLocal->IsAlive())
		return;

	m_pLocalWeapon = m_pLocal->GetWeapon();
	if (!m_pLocalWeapon)
		return;

	if (!(m_pCmd->iButtons & IN_ATTACK))
		angOldAng.x = angOldAng.y = angOldAng.z = 0;

	short nDefinitionIndex = *m_pLocalWeapon->GetItemDefinitionIndex();
	m_pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);
	if (!m_pWeaponData || !m_pWeaponData->IsGun())
		return;

	WeaponSettings();

	if (!m_bEnable ||
		!GUI::UTILS::KeybindMethod(C::Get<int>(Vars.iAimbotKey), C::Get<int>(Vars.iAimbotKeyMethod), &bToggled))
		return;

	m_vLocalPosition = m_pLocal->GetEyePosition();

	if (!m_pLocal->CanShoot((CWeaponCSBase*)m_pLocalWeapon))
		return;

	if (m_flRCS > 0)
		RCS();

	AimbotTarget_t target = GetBestPlayer();
	if (target.pEntity)
		MoveViewAngles(target);
}

AimbotTarget_t CAimbot::GetBestPlayer()
{
	for (int i = 1; i < I::Globals->nMaxClients; i++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);
		if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant())
			continue;

		if (!m_pLocal->IsEnemy(pEntity) || !pEntity->IsPlayer() || pEntity->HasImmunity())
			continue;

		Vector vHitbox = GetBestHitbox(pEntity);
		if (vHitbox.IsZero())
			continue;

		static auto pWeaponRecoilScale = I::ConVar->FindVar(_("weapon_recoil_scale"));

		QAngle angCalc = M::CalcAngle(m_vLocalPosition, vHitbox);
		float flFOV = M::GetFOV(m_pCmd->angViewPoint + m_pLocal->GetPunch() * pWeaponRecoilScale->GetFloat(), angCalc);
		
		m_vTargets.push_back({ pEntity, flFOV, angCalc, vHitbox });
	}

	if (m_vTargets.empty())
		return { };

	std::sort(m_vTargets.begin(), m_vTargets.end(), [](const AimbotTarget_t& a, const AimbotTarget_t& b) -> bool {
		return a.flFOV < b.flFOV;
	});

	for (auto& i : m_vTargets)
	{
		if (i.pEntity && !i.angAngleTo.IsZero())
		{
			if (i.flFOV < m_flFOV)
				return i;
		}
	}

	return { };
}

int CAimbot::GetNearestHitbox(CBaseEntity* pEntity)
{
	int iBestHitbox = -1;
	auto flBestFOV = 180.0f;

	for (auto i = 0; i < HITBOX_MAX; i++)
	{
		Vector vHitbox = pEntity->GetHitboxPosition(i);
		if (vHitbox.IsZero())
			continue;

		QAngle angCalc = M::CalcAngle(m_vLocalPosition, vHitbox);
		float flFOV = M::GetFOV(m_pCmd->angViewPoint + m_pLocal->GetPunch() * 2.0f, angCalc);
		if (flFOV < flBestFOV)
		{
			flBestFOV = flFOV;
			iBestHitbox = i;
		}
	}

	return iBestHitbox;
}

Vector CAimbot::GetBestHitbox(CBaseEntity* pEntity)
{
	Vector vecReturn;
	int iBestHitbox = -1;
	float flBestFOV = 180.0f;
	float flBestSimTime = -1;

	switch (m_iHitboxSelection)
	{
	case 0:
		iBestHitbox = HITBOX_HEAD;
		break;
	case 1:
		iBestHitbox = HITBOX_CHEST;
		break;
	case 2:
		iBestHitbox = HITBOX_PELVIS;
		break;
	case 3:
		iBestHitbox = this->GetNearestHitbox(pEntity);
		break;
	default:
		break;
	}

	vecReturn = pEntity->GetHitboxPosition(iBestHitbox);

	if (!g_Backtrack.vRecords[pEntity->GetIndex()].empty() &&
		C::Get<bool>(Vars.bBacktrack))
	{
		auto vecBacktrack = g_Backtrack.vRecords[pEntity->GetIndex()];

		int iTicks = 0;
		for (auto i = 0; i < vecBacktrack.size() && iTicks < C::Get<int>(Vars.iBacktrackMax); i++) {
			iTicks++;

			Vector vecBacktrackHitbox = pEntity->GetBacktrackHitboxPosition(iBestHitbox, i);
			if (vecBacktrackHitbox.IsZero())
				continue;

			QAngle angCalc = M::CalcAngle(m_vLocalPosition, vecBacktrackHitbox);
			float flFOV = M::GetFOV(m_pCmd->angViewPoint + m_pLocal->GetPunch() * 2.0f, angCalc);

			if (flFOV < flBestFOV && vecBacktrack.at(i).bIsValid) {
				flBestFOV = flFOV;
				flBestSimTime = vecBacktrack.at(i).flSimTime;
				vecReturn = vecBacktrackHitbox;
			}
		}

		m_flSimTime = flBestSimTime;
	}

	return m_pLocal->IsVisible(pEntity, vecReturn) ? vecReturn : Vector();
}


void CAimbot::MoveViewAngles(AimbotTarget_t target)
{
	QAngle angDest = target.angAngleTo;
	QAngle angSrc = m_pCmd->angViewPoint;

	if (m_flRCS > 0)
		angDest -= m_pLocal->GetPunch() * m_flRCS / 50.0f;

	QAngle angDelta = angSrc - angDest;
	angDelta.Clamp();

	if (!angDelta.IsZero())
	{
		angDest = angSrc - angDelta / m_flSmoothing;
	}

	angDest.Clamp();

	m_pCmd->angViewPoint = angDest;

	I::Engine->SetViewAngles(m_pCmd->angViewPoint);

	if (m_pCmd->iButtons & IN_ATTACK && m_flSimTime != -1 &&
		C::Get<bool>(Vars.bBacktrack)) {
		m_pCmd->iTickCount = M_TIME_TO_TICKS(m_flSimTime);
	}
}

void CAimbot::RCS()
{
	static auto pWeaponRecoilScale = I::ConVar->FindVar(_("weapon_recoil_scale"));

	if (m_pCmd->iButtons & IN_ATTACK) 
	{
		QAngle angPunch = m_pLocal->GetPunch() * pWeaponRecoilScale->GetFloat();

		m_pCmd->angViewPoint += (angOldAng - angPunch);

		angOldAng = angPunch;
	}
}

void CAimbot::ClearAimbot()
{
	if (!m_vTargets.empty())
		m_vTargets.clear();

	m_pCmd = nullptr;
	m_pLocal = nullptr;
	m_pLocalWeapon = nullptr;
	m_pWeaponData = nullptr;
	m_vLocalPosition = Vector();
	m_vEyeAngles = QAngle();
}

void CAimbot::WeaponSettings()
{
	m_bEnable = C::Get<bool>(Vars.bEnable);
	m_bSilentAim = C::Get<bool>(Vars.bSilentEnable);
	m_flFOV = C::Get<float>(Vars.fRegularFOV);
	m_flSilentFOV = C::Get<float>(Vars.fSilentFOV);
	m_iHitboxSelection = C::Get<int>(Vars.iHitboxSelection);
	m_flRCS = C::Get<float>(Vars.fRCS);
	m_flSmoothing = C::Get<float>(Vars.fSmoothing);
}