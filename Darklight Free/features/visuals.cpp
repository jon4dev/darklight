#include <mutex>

#include "visuals.h"
#include "../global.h"
#include "../sdk/datatypes/keyvalues.h"
#include "../core/variables.h"
#include "../core/gui/gui.h"
#include "../core/hooks.h"
#include "../utilities/draw.h"
#include "../utilities/math.h"
#include "../utilities.h"
#include "backtrack.h"
#include "misc.h"
#include "../global.h"

CVisuals g_Visuals;

void CVisuals::Event(IGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (!pEvent || !G::pLocal)
		return;

	switch (uNameHash)
	{
		case FNV1A::HashConst(("player_hurt")):
		{
			const auto pAttacker = (CBaseEntity*)I::ClientEntityList->GetClientEntity(I::Engine->GetPlayerForUserID(pEvent->GetInt(_("attacker"))));
			if (!pAttacker)
				return;

			const auto pVictim = (CBaseEntity*)I::ClientEntityList->GetClientEntity(I::Engine->GetPlayerForUserID(pEvent->GetInt(_("userid"))));
			if (!pVictim)
				return;

			if (pAttacker == G::pLocal && pVictim != G::pLocal)
			{
				if (C::Get<bool>(Vars.bHitmarker))
				{
					m_flHitmarkerTime = 1.0f;
					I::Surface->PlaySoundSurface(_("buttons\\arena_switch_press_02.wav"));
				}
			}
		} break;
		default: break;
	}
}

void CVisuals::Hitmarker()
{
	if (!G::pLocal || !G::pLocal->IsAlive())
		return;

	const Vector2D vecCenter(G::vecDisplaySize.x / 2, G::vecDisplaySize.y / 2);

	if (m_flHitmarkerTime > 0.0f) {
		D::Line(vecCenter.x - 10, vecCenter.y + 10, vecCenter.x - 4, vecCenter.y + 4, Color(255, 255, 255, m_flHitmarkerTime * 255));
		D::Line(vecCenter.x + 10, vecCenter.y + 10, vecCenter.x + 4, vecCenter.y + 4, Color(255, 255, 255, m_flHitmarkerTime * 255));
		D::Line(vecCenter.x - 10, vecCenter.y - 10, vecCenter.x - 4, vecCenter.y - 4, Color(255, 255, 255, m_flHitmarkerTime * 255));
		D::Line(vecCenter.x + 10, vecCenter.y - 10, vecCenter.x + 4, vecCenter.y - 4, Color(255, 255, 255, m_flHitmarkerTime * 255));

		m_flHitmarkerTime -= 1.0f / 0.5f * I::Globals->flFrameTime;
	}
}

void CVisuals::Run()
{
	SpectatorFrame();

	if (!G::pLocal)
		return;

	if (m_iTotalSpectators > 0)
		m_iTotalSpectators = 0;

	CollectEntities();
	Paint();
	Hitmarker();
}

void CVisuals::CollectEntities()
{
	for (int i = 1; i < I::ClientEntityList->GetHighestEntityIndex(); i++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);
		if (!pEntity)
			continue;

		if (!pEntity->IsAlive())
			SpectatorList(pEntity);

		if (pEntity->IsPlayer())
		{
			if (!pEntity->IsAlive() || !G::pLocal->IsEnemy(pEntity) || pEntity->IsDormant())
				continue;

			PaintPlayers(pEntity);
		}

		if (!pEntity->IsDormant() && !pEntity->IsPlayer())
			if (pEntity->GetClientClass()->nClassID == EClassIndex::CPlantedC4)
				PaintC4(pEntity);
	}
}

void CVisuals::SpectatorList(CBaseEntity* pEntity)
{
	PlayerInfo_t playerInfo;
	if (!I::Engine->GetPlayerInfo(pEntity->GetIndex(), &playerInfo))
		return;

	CBaseHandle hOBSTarget = pEntity->GetObserverTargetHandle();
	if (!hOBSTarget)
		return;

	CBaseEntity* pObserved = I::ClientEntityList->Get<CBaseEntity>(hOBSTarget);
	if (!pObserved)
		return;

	std::string szName = playerInfo.szName;

	std::transform(szName.begin(), szName.end(), szName.begin(), toupper);

	if (pObserved == G::pLocal && G::pLocal->IsAlive() && C::Get<bool>(Vars.bSpectatorList))
		D::String(m_vecPos.x + 16, m_vecPos.y + 40 + (m_iTotalSpectators++ * 11), D::uSmallFonts[G::iDPIScale], szName, false, Color(150, 150, 150));
}

void CVisuals::SpectatorFrame()
{
	if (!C::Get<bool>(Vars.bSpectatorList))
		return;

	m_vecSize = { 152, float(57 + m_iTotalSpectators * 11) };

	auto vecPos = m_vecPos = Vector2D((float)C::Get<int>(Vars.iSpectatorX), (float)C::Get<int>(Vars.iSpectatorY));

	static bool bDrag = false;
	static Vector2D vecLastMousePos;
	static Vector2D vecLastMenuPos;

	if (GUI::UTILS::KeyDown(VK_LBUTTON) && GUI::m_bOpened && !GUI::m_bGrabbingCheatGUI)
	{
		if (!bDrag && GUI::UTILS::MouseInRegion(m_vecPos.x, m_vecPos.y, m_vecSize.x, m_vecSize.y) &&
			!GUI::UTILS::MouseInRegion(m_vecPos.x, m_vecPos.y + 20, m_vecSize.x, m_vecSize.y - 20))
		{
			bDrag = true;
			vecLastMousePos = GUI::UTILS::m_vMousePos;
			vecLastMenuPos = m_vecPos;
		}

		if (bDrag)
		{
			C::Get<int>(Vars.iSpectatorX) = vecLastMenuPos.x + (GUI::UTILS::m_vMousePos.x - vecLastMousePos.x);
			C::Get<int>(Vars.iSpectatorY) = vecLastMenuPos.y + (GUI::UTILS::m_vMousePos.y - vecLastMousePos.y);
		}
	}
	else
		bDrag = false;

	m_bGrabbingSpectatorList = bDrag;

	if (C::Get<int>(Vars.iSpectatorX) < 0)
		C::Get<int>(Vars.iSpectatorX) = 0;

	if (C::Get<int>(Vars.iSpectatorY) < 0)
		C::Get<int>(Vars.iSpectatorY) = 0;

	if ((C::Get<int>(Vars.iSpectatorX) + m_vecSize.x) > G::vecDisplaySize.x)
		C::Get<int>(Vars.iSpectatorX) = G::vecDisplaySize.x - m_vecSize.x;

	if ((C::Get<int>(Vars.iSpectatorY) + m_vecSize.y) > G::vecDisplaySize.y)
		C::Get<int>(Vars.iSpectatorY) = G::vecDisplaySize.y - m_vecSize.y;

	std::string szName = _("SPECTATORS  LIST  (") + std::to_string(m_iTotalSpectators) + _(")");

	D::Rect(m_vecPos.x, m_vecPos.y, m_vecSize.x, m_vecSize.y, Color(20, 20, 20));
	D::OutlinedRect(m_vecPos.x + 1, m_vecPos.y + 1, m_vecSize.x - 2, 21, Color(50, 50, 50));
	D::Rect(m_vecPos.x, m_vecPos.y + 23, m_vecSize.x, 1, Color(GUI::CONTROLS::m_cDefaultMenuCol));
	D::OutlinedRect(m_vecPos.x + 1, m_vecPos.y + 1, m_vecSize.x - 2, m_vecSize.y - 2, Color(50, 50, 50));
	D::OutlinedRect(m_vecPos.x, m_vecPos.y, m_vecSize.x, m_vecSize.y, Color(0, 0, 0));
	D::String(m_vecPos.x + 6, m_vecPos.y + 6, D::uSmallFonts[G::iDPIScale], szName.c_str(), false, Color(147, 147, 155));

	D::OutlinedRect(m_vecPos.x + 8, m_vecPos.y + 32, 136, 15 + m_iTotalSpectators * 11, Color(50, 50, 50));
	D::OutlinedRect(m_vecPos.x + 7, m_vecPos.y + 31, 138, 15 + m_iTotalSpectators * 11 + 2, Color(0, 0, 0));
}

void CVisuals::PaintPlayers(CBaseEntity* pEntity)
{
	BBox_t bbox;
	if (!CreateBBox(pEntity, bbox))
		return;

	if (C::Get<bool>(Vars.bBox))
	{
		D::OutlinedRect(bbox.flX, bbox.flY, bbox.flW, bbox.flH, pEntity->IsDormant() ? Color(110, 110, 110) : Color(C::Get<Color>(Vars.colBox), C::Get<Color>(Vars.colBox).a()));
		D::OutlinedRect(bbox.flX - 1, bbox.flY - 1, bbox.flW + 2, bbox.flH + 2, Color(0, 0, 0));
		D::OutlinedRect(bbox.flX + 1, bbox.flY + 1, bbox.flW - 2, bbox.flH - 2, Color(0, 0, 0));
	}

	if (C::Get<bool>(Vars.bName))
	{
		PlayerInfo_t playerInfo;
		I::Engine->GetPlayerInfo(pEntity->GetIndex(), &playerInfo);

		std::string player_name;
		if (playerInfo.bFakePlayer)
			player_name += _("BOT ");

		player_name += playerInfo.szName;

		auto name_text_size = D::GetTextSize(D::uVerdana[G::iDPIScale], player_name.c_str());
		D::String(bbox.flX + (bbox.flW / 2) - (name_text_size.right / 2), bbox.flY - 14, D::uVerdana[G::iDPIScale], player_name.c_str(), false, Color(C::Get<Color>(Vars.colName), C::Get<Color>(Vars.colName).a()));
	}

	if (C::Get<bool>(Vars.bHealth))
	{
		auto health = std::clamp(pEntity->GetHealth(), 0, 100);

		auto green = health * 2.55;
		auto red = 255 - green;

		auto bar = (bbox.flH / 100.0f) * health;
		auto delta = bbox.flH - bar;

		D::Rect(bbox.flX - 6, bbox.flY - 1, 2, bbox.flH + 2, pEntity->IsDormant() ? Color(80, 80, 80) : Color(80, 80, 80, 125));
		D::Rect(bbox.flX - 6, bbox.flY + delta - 1, 2, bar + 2, Color(red, green, 0));
		D::OutlinedRect(bbox.flX - 7, bbox.flY - 1, 4, bbox.flH + 2, Color(0, 0, 0));

		if (health < 100.0f) {
			D::String(bbox.flX - 6, bbox.flY + delta - 4, D::uSmallFonts[G::iDPIScale], std::to_string(health).c_str(), true, Color(255, 255, 255));
		}
	}

	CBaseCombatWeapon* pWeapon = pEntity->GetWeapon();
	if (pWeapon)
	{
		CCSWeaponData* pData = I::WeaponSystem->GetWeaponData(*pWeapon->GetItemDefinitionIndex());
		if (pData)
		{
			if (C::Get<bool>(Vars.bWeapon))
				D::String(bbox.flX + (bbox.flW / 2), C::Get<bool>(Vars.bAmmo) && !(pData->nWeaponType == WEAPONTYPE_C4 || pData->nWeaponType == WEAPONTYPE_GRENADE || pData->nWeaponType == WEAPONTYPE_KNIFE) ? (bbox.flY + bbox.flH) + 10 : (bbox.flY + bbox.flH) + 4, D::uSmallFonts[G::iDPIScale], pWeapon->GetWeaponName(), true, Color(C::Get<Color>(Vars.colWeapon), C::Get<Color>(Vars.colWeapon).a()));

			if (C::Get<bool>(Vars.bAmmo))
			{
				if (pData->nWeaponType == WEAPONTYPE_C4 ||
					pData->nWeaponType == WEAPONTYPE_GRENADE ||
					pData->nWeaponType == WEAPONTYPE_KNIFE) {
					return;
				}

				float bar_width = pWeapon->GetAmmo() * bbox.flW / pData->iMaxClip1;

				auto animlayer = pEntity->GetAnimationOverlays()[1];
				if (animlayer.pOwner) {
					auto activity = pEntity->GetSequenceActivity(animlayer.nSequence);
					if ((activity == 967 && animlayer.flWeight != 0.0f) &&
						animlayer.flCycle < 0.99) {
						bar_width = (animlayer.flCycle * bbox.flW) / 1.f;
					}
				}

				D::Rect(bbox.flX, bbox.flY + bbox.flH + 3, bbox.flW + 1, 2, pEntity->IsDormant() ? Color(80, 80, 80) : Color(80, 80, 80, 125));
				D::OutlinedRect(bbox.flX - 1, bbox.flY + bbox.flH + 2, bbox.flW + 2, 4, Color(0, 0, 0));
				D::Rect(bbox.flX, bbox.flY + bbox.flH + 3, bar_width, 2, Color(C::Get<Color>(Vars.colAmmo), C::Get<Color>(Vars.colAmmo).a()));

				if (pWeapon->GetAmmo() < pData->iMaxClip1 && pEntity->GetSequenceActivity(animlayer.nSequence) != 967) {
					D::String(bbox.flX + bar_width, bbox.flY + bbox.flH + 4, D::uSmallFonts[G::iDPIScale], std::to_string(pWeapon->GetAmmo()).c_str(), true, Color(255, 255, 255));
				}
			}
		}
	}
}

float BombArmor(float flDamage, float flArmorValue)
{
	if (flArmorValue > 0)
	{
		float flNew = flDamage * 0.5f;
		float flArmor = (flDamage - flNew) * 0.5f;

		if (flArmor > flArmorValue) 
		{
			flArmor = flArmorValue * (1.f / 0.5f);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}

	return flDamage;
}

void CVisuals::PaintC4(CBaseEntity* pEntity)
{
	const auto pC4 = (CPlantedC4*)pEntity;
	if (!pC4 || pC4->GetTimer(I::Globals->flCurrentTime) <= 0 || pC4->IsDefused())
		return;

	BBox_t bbox;
	const auto bInView = CreateBBox(pEntity, bbox);

	char buffer[256];
	sprintf(buffer, _("BOMB %s - %.1f S"), pC4->GetBombSite() ? _("B") : _("A"), pC4->GetTimer(I::Globals->flCurrentTime));

	const auto bombSize = D::GetTextSize(D::uVerdanaBig[G::iDPIScale], buffer);

	if (C::Get<bool>(Vars.bBombTimer))
	{
		D::String(G::vecDisplaySize.x - bombSize.right, C::Get<bool>(Vars.bWatermark) ? 30 : 0, D::uVerdanaBig[G::iDPIScale], buffer, false, Color(225, 0, 0));
		if (bInView)
			D::String(bbox.flX + (bbox.flW / 2), bbox.flY + (bbox.flH / 2), D::uSmallFonts[G::iDPIScale], buffer, 4, Color(225, 0, 0));
	}

	if (G::pLocal->IsAlive())
	{
		const float flHypDist = (pEntity->GetEyePosition() - G::pLocal->GetEyePosition()).Length(), flSigma = 583.333313F;
		const int iDamage = BombArmor((500.f * (exp(-flHypDist * flHypDist / (2.0f * flSigma * flSigma))) * 1.0f), G::pLocal->GetArmor());

		const bool bIsLethal = iDamage >= G::pLocal->GetHealth();
		const std::string szHPToStr = std::to_string(iDamage);
		const std::string szDamage = bIsLethal ? szHPToStr + _(" (LETHAL)") : szHPToStr;
		const Color lethalColor = bIsLethal ? Color(225, 0, 0) : Color(0, 225, 0);

		if (iDamage)
		{
			if (C::Get<bool>(Vars.bBombTimer))
			{
				D::String(G::vecDisplaySize.x - D::GetTextSize(D::uVerdanaBig[G::iDPIScale], szDamage.c_str()).right, C::Get<bool>(Vars.bWatermark) ? 30 + bombSize.bottom : bombSize.bottom, D::uVerdanaBig[G::iDPIScale], szDamage, 0, lethalColor);
				if (bInView)
					D::String(bbox.flX + (bbox.flW / 2), bbox.flY + (bbox.flH / 2) + 10, D::uSmallFonts[G::iDPIScale], szDamage, 4, lethalColor);
			}
		}
	}
}

bool CVisuals::CreateBBox(CBaseEntity* pEntity, BBox_t& BBox)
{
	const ICollideable* pCollideable = pEntity->GetCollideable();
	if (pCollideable == nullptr)
		return false;

	const Vector vecMin = pCollideable->OBBMins();
	const Vector vecMax = pCollideable->OBBMaxs();

	std::array<Vector, 8U> arrPoints =
	{
		Vector(vecMin.x, vecMin.y, vecMin.z),
		Vector(vecMin.x, vecMax.y, vecMin.z),
		Vector(vecMax.x, vecMax.y, vecMin.z),
		Vector(vecMax.x, vecMin.y, vecMin.z),
		Vector(vecMax.x, vecMax.y, vecMax.z),
		Vector(vecMin.x, vecMax.y, vecMax.z),
		Vector(vecMin.x, vecMin.y, vecMax.z),
		Vector(vecMax.x, vecMin.y, vecMax.z)
	};

	if (arrPoints.data() == nullptr)
		return false;

	const matrix3x4_t& matTransformed = pEntity->GetCoordinateFrame();

	float flLeft = std::numeric_limits<float>::max();
	float flTop = std::numeric_limits<float>::max();
	float flRight = -std::numeric_limits<float>::max();
	float flBottom = -std::numeric_limits<float>::max();

	std::array<Vector, 8U> arrScreen = { };
	for (std::size_t i = 0U; i < 8U; i++)
	{
		if (!WorldToScreen(M::VectorTransform(arrPoints.at(i), matTransformed), arrScreen.at(i)))
			return false;

		flLeft = std::min(flLeft, arrScreen.at(i).x);
		flTop = std::min(flTop, arrScreen.at(i).y);
		flRight = std::max(flRight, arrScreen.at(i).x);
		flBottom = std::max(flBottom, arrScreen.at(i).y);
	}

	BBox.flX = flLeft;
	BBox.flY = flTop;
	BBox.flW = flRight - flLeft;
	BBox.flH = flBottom - flTop;

	return true;
}

void CVisuals::Glow(CBaseEntity* pLocal)
{
	for (int i = 0; i < I::GlowManager->vecGlowObjectDefinitions.Count(); i++)
	{
		IGlowObjectManager::GlowObject_t& hGlowObject = I::GlowManager->vecGlowObjectDefinitions[i];
		if (hGlowObject.IsEmpty())
			continue;

		CBaseEntity* pEntity = hGlowObject.pEntity;
		if (pEntity == nullptr)
			continue;

		CBaseClient* pClientClass = pEntity->GetClientClass();
		if (pClientClass == nullptr)
			continue;

		const EClassIndex nIndex = pClientClass->nClassID;
		switch (nIndex)
		{
			case EClassIndex::CCSPlayer:
			{
				if (pEntity->IsDormant() || !pEntity->IsAlive())
					break;

				if (pLocal->IsEnemy(pEntity))
					hGlowObject.Set(C::Get<Color>(Vars.colGlow));
				break;
			}
			default:
			{
				hGlowObject.flAlpha = 0.0f;
				break;
			}
		}

		hGlowObject.bRenderWhenOccluded = true;
		hGlowObject.bRenderWhenUnoccluded = false;
	}
}

bool CVisuals::WorldToScreen(const Vector& vOrigin, Vector& vScreen) {
	return I::DebugOverlay->ScreenPosition(vOrigin, vScreen) != 1;
}

void CVisuals::OverrideView(CViewSetup* setup)
{
	if (!C::Get<bool>(Vars.bGrenadePrediction) ||
		!G::pLocal || !G::pLocal->IsAlive())
		return;

	const auto pWeapon = G::pLocal->GetWeapon();
	if (!pWeapon)
		return;

	const auto pWeaponData = I::WeaponSystem->GetWeaponData(*pWeapon->GetItemDefinitionIndex());
	if (!pWeaponData)
		return;

	if (pWeaponData->nWeaponType == WEAPONTYPE_GRENADE)
	{
		type = *pWeapon->GetItemDefinitionIndex();
		Simulate(setup);
	}
	else
		type = 0;
}

void CVisuals::Paint()
{
	if (!C::Get<bool>(Vars.bGrenadePrediction) ||
		!G::pLocal || !G::pLocal->IsAlive())
		return;

	const auto pWeapon = G::pLocal->GetWeapon();
	if (!pWeapon)
		return;

	const auto pWeaponData = I::WeaponSystem->GetWeaponData(*pWeapon->GetItemDefinitionIndex());
	if (!pWeaponData)
		return;

	const auto pGrenade = (CBaseCSGrenade*)pWeapon;
	if (!pGrenade)
		return;

	if (pWeaponData->nWeaponType == WEAPONTYPE_GRENADE) {
		if (!pGrenade->IsPinPulled())
		{
			if (pGrenade->GetThrowTime() == 0)
				return;
		}
	}
	else
		return;

	if (type && vecPath.size() > 1)
	{
		Vector vecNadeStart, vecNadeEnd;
		Vector vecPrev = vecPath[0];

		for (auto it = vecPath.begin(), end = vecPath.end(); it != end; ++it)
		{
			if (WorldToScreen(vecPrev, vecNadeStart) && WorldToScreen(*it, vecNadeEnd))
				D::Line((int)vecNadeStart.x, (int)vecNadeStart.y, (int)vecNadeEnd.x, (int)vecNadeEnd.y, C::Get<Color>(Vars.colGrenadePrediction));

			vecPrev = *it;
		}

		for (auto it = vecCollision.begin(), end = vecCollision.end(); it != end; ++it)
		{
			if (WorldToScreen(*it, vecNadeEnd))
				D::OutlinedRect(vecNadeEnd.x - 1, vecNadeEnd.y - 1, 3, 3, Color(255, 255, 255));
		}

		if (WorldToScreen(vecPrev, vecNadeEnd))
			D::OutlinedRect(vecNadeEnd.x - 1, vecNadeEnd.y - 1, 3, 3, Color(255, 0, 0));
	}
}

void CVisuals::Setup(Vector& vecSrc, Vector& vecThrow, QAngle viewangles)
{
	if (!G::pLocal || !G::pLocal->IsAlive())
		return;

	const auto pWeapon = G::pLocal->GetWeapon();
	if (!pWeapon)
		return;

	const auto pGrenade = (CBaseCSGrenade*)pWeapon;
	if (!pGrenade)
		return;

	QAngle angThrow = viewangles;
	float pitch = angThrow.x;

	if (pitch <= 90.0f)
	{
		if (pitch < -90.0f)
			pitch += 360.0f;
	}
	else
		pitch -= 360.0f;

	angThrow.x = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;

	float flVel = 750.0f * 0.9f;
	float b = pGrenade->GetThrowStrength();
	b = b * 0.7f; 
	b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	M::AngleVectors(angThrow, &vForward, &vRight, &vUp);

	vecSrc = G::pLocal->GetEyePosition();
	vecSrc.z += (pGrenade->GetThrowStrength() * 12.0f) - 12.0f;

	Trace_t tr;
	Vector vecDest = vecSrc;
	vecDest += vForward * 22.0f;

	TraceHull(vecSrc, vecDest, tr);

	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.vecEnd;
	vecSrc -= vecBack;

	vecThrow = G::pLocal->GetVelocity(); 
	vecThrow *= 1.25f;
	vecThrow += vForward * flVel;
}

void CVisuals::Simulate(CViewSetup* setup)
{
	Vector vecSrc, vecThrow;

	QAngle angles; 
	I::Engine->GetViewAngles(angles);

	Setup(vecSrc, vecThrow, angles);

	float interval = I::Globals->flIntervalPerTick;
	int logstep = (int)(0.05f / interval);
	int logtimer = 0;

	bool first = false;

	vecPath.clear();
	vecCollision.clear();
	for (unsigned int i = 0; i < 1500; ++i)
	{
		static int s = 0;

		if (!logtimer)
		{
			vecPath.push_back(vecSrc);
			first = true;
		}

		s = Step(vecSrc, vecThrow, i, interval);
		if ((s & 1)) break;
		if ((s & 2) || logtimer >= logstep) logtimer = 0;
		else ++logtimer;
	}

	vecPath.push_back(vecSrc);
}

int CVisuals::Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval)
{
	Vector move; AddGravityMove(move, vecThrow, interval, false);
	Trace_t tr; PushEntity(vecSrc, move, tr);

	int result = 0;
	if (CheckDetonate(vecThrow, tr, tick, interval))
		result |= 1;

	if (tr.flFraction != 1.0f)
	{
		result |= 2;
		ResolveFlyCollisionCustom(tr, vecThrow, interval);

		vecCollision.push_back(tr.vecEnd);
	}

	vecSrc = tr.vecEnd;

	return result;
}

bool CVisuals::CheckDetonate(const Vector& vecThrow, const Trace_t& tr, int tick, float interval)
{
	switch (type)
	{
	case WEAPON_SMOKEGRENADE:
	case WEAPON_DECOY:
		if (vecThrow.Length2D() < 0.1f)
		{
			int det_tick_mod = (int)(0.2f / interval);
			return !(tick % det_tick_mod);
		}
		return false;

	case WEAPON_MOLOTOV:
	case WEAPON_INCGRENADE:
		if (tr.flFraction != 1.0f && tr.plane.vecNormal.z > 0.7f)
			return true;

	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		return (float)tick * interval > 1.5f && !(tick % (int)(0.2f / interval));
	default:
		assert(false);
		return false;
	}
}

void CVisuals::TraceHull(Vector& src, Vector& end, Trace_t& tr)
{
	CTraceFilterWorldOnly filter;
	I::EngineTrace->TraceRay(Ray_t(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f)), 0x200400B, &filter, &tr);
}

void CVisuals::AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	Vector basevel(0.0f, 0.0f, 0.0f);
	move.x = (vel.x + basevel.x) * frametime;
	move.y = (vel.y + basevel.y) * frametime;

	if (onground)
		move.z = (vel.z + basevel.z) * frametime;
	else
	{
		float gravity = 800.0f * 0.4f;
		float newZ = vel.z - (gravity * frametime);
		move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;
		vel.z = newZ;
	}
}

void CVisuals::PushEntity(Vector& src, const Vector& move, Trace_t& tr)
{
	Vector vecAbsEnd = src;
	vecAbsEnd += move;
	TraceHull(src, vecAbsEnd, tr);
}

void CVisuals::ResolveFlyCollisionCustom(Trace_t& tr, Vector& vecVelocity, float interval)
{
	float flSurfaceElasticity = 1.0, flGrenadeElasticity = 0.45f;
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

	Vector vecAbsVelocity;
	PhysicsClipVelocity(vecVelocity, tr.plane.vecNormal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f;

	if (flSpeedSqr < flMinSpeedSqr)
	{
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	if (tr.plane.vecNormal.z > 0.7f)
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.flFraction) * interval);
		PushEntity(tr.vecEnd, vecAbsVelocity, tr);
	}
	else
		vecVelocity = vecAbsVelocity;
}

int CVisuals::PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float backoff, change, angle;
	int   i, blocked;

	blocked = 0;
	angle = normal[2];

	if (angle > 0) blocked |= 1;
	if (!angle) blocked |= 2;

	backoff = in.DotProduct(normal) * overbounce;
	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	return blocked;
}