#include "triggerbot.h"
#include "../core/variables.h"
#include "../sdk/definitions.h"
#include "../utilities/math.h"
#include "../utilities/inputsystem.h"
#include "../core/interfaces.h"
#include "../utilities.h"
#include "backtrack.h"
#include "autowall.h"
#include "../core/gui/gui.h"
#include "misc.h"

CTriggerBot g_Triggerbot;

void CTriggerBot::Run(CUserCmd* pCmd, CBaseEntity* pLocal)
{
    static bool bToggled = false;
    if (!C::Get<bool>(Vars.bTriggerbot) ||
        !GUI::UTILS::KeybindMethod(C::Get<int>(Vars.iTriggerbotKey), C::Get<int>(Vars.iTriggerbotKeyMethod), &bToggled))
    {
        timer.Reset();
        return;
    }

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();
	if (pWeapon == nullptr)
		return;

	short nDefinitionIndex = *pWeapon->GetItemDefinitionIndex();
	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);
	if (pWeaponData == nullptr || !pWeaponData->IsGun())
		return;

	QAngle angView = pCmd->angViewPoint;
	angView += pLocal->GetPunch() * 2.0f;

	Trace_t trace = { };
	Vector vecStart, vecEnd, vecForward;
	M::AngleVectors(angView, &vecForward);

	vecStart = pLocal->GetEyePosition();
	vecForward *= pWeaponData->flRange;
	vecEnd = vecStart + vecForward;

    Ray_t ray(vecStart, vecEnd);
    CTraceFilter filter(pLocal);
    I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);

    if (trace.pHitEntity &&
        !pLocal->IsEnemy(trace.pHitEntity) &&
        !trace.pHitEntity->IsPlayer() &&
        C::Get<bool>(Vars.bTriggerbotAutowall))
    {
        float flMinDmg = C::Get<float>(Vars.flTriggerbotAutowallMinDmg);
        if (flMinDmg > 100)
            flMinDmg -= 100;

        FireBulletData_t data = { };
        const float flDamage = g_AutoWall.GetDamage(pLocal, vecEnd, data);
        if (flDamage >= flMinDmg)
            trace = data.enterTrace;
    }

    if (!trace.pHitEntity &&
        !pLocal->IsEnemy(trace.pHitEntity) ||
        trace.pHitEntity->HasImmunity() ||
        !trace.pHitEntity->IsPlayer() ||
        !trace.pHitEntity->IsAlive())
    {
        timer.Reset();
        return;
    }

	if (trace.iHitGroup == HITGROUP_HEAD || trace.iHitGroup == HITGROUP_CHEST || trace.iHitGroup == HITGROUP_STOMACH)
	{
        if (!Hitchance(trace.pHitEntity, pWeapon, C::Get<float>(Vars.flTriggerbotHitchance)))
            return;

        if (C::Get<int>(Vars.iTriggerbotDelay) > 0)
            if (timer.Elapsed() < C::Get<int>(Vars.iTriggerbotDelay))
                return;

		if (pLocal->CanShoot((CWeaponCSBase*)pWeapon))
			pCmd->iButtons |= IN_ATTACK;
	}

    if (C::Get<bool>(Vars.bBacktrack))
    {
        float best_fov = 180.0f;
        auto best_target = (CBaseEntity*)nullptr;

        for (int i = 1; i < I::ClientEntityList->GetMaxEntities(); i++)
        {
            CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);
            if (!pEntity || !pEntity->IsAlive() || pEntity->IsDormant() || !pEntity->IsPlayer() ||
                !pLocal->IsEnemy(pEntity) || pEntity->GetClientClass()->nClassID != EClassIndex::CCSPlayer)
                continue;

            auto backtrack = g_Backtrack.vRecords[i];
            if (backtrack.empty())
                continue;

            auto hitbox = pEntity->GetHitboxPosition(HITBOX_HEAD);
            if (hitbox.IsZero()) { continue; }

            auto fov = M::GetFOV(angView, M::CalcAngle(pLocal->GetEyePosition(), hitbox));
            if (fov < best_fov) {
                best_fov = fov;
                best_target = pEntity;
            }
        }

        if (best_target) {
            if (auto record = TargetRecord(best_target, pWeapon, vecStart, vecEnd); record) {
                if (AcceptedInaccuracy(pWeapon) < C::Get<float>(Vars.flTriggerbotHitchance))
                    return;

                if (C::Get<int>(Vars.iTriggerbotDelay) > 0)
                    if (timer.Elapsed() < C::Get<int>(Vars.iTriggerbotDelay))
                        return;

                if (pLocal->CanShoot((CWeaponCSBase*)pWeapon))
                {
                    pCmd->iButtons |= IN_ATTACK;
                    pCmd->iTickCount = TIME_TO_TICKS(g_Backtrack.vRecords[best_target->GetIndex()].at(*record).flSimTime);
                }
            }
        }
    }
}

bool CTriggerBot::Hitchance(CBaseEntity* pEntity, CBaseCombatWeapon* pWeapon, float flMinimum)
{
    static float flTraceAmount = 256.0f;
    const int iMinimumHits = (int)(256.0f * (flMinimum / 100.0f));
    const Vector vecStart = G::pLocal->GetEyePosition();
    int iCurrentHits = 0;

    QAngle qViewangles;
    I::Engine->GetViewAngles(qViewangles);

    const auto pWeaponData = I::WeaponSystem->GetWeaponData(*pWeapon->GetItemDefinitionIndex());
    if (!pWeaponData)
        return false;

    const float flInaccuracy = pWeapon->GetInaccuracy(), flSpread = pWeapon->GetSpread();

    Vector vecEnd, vecForward, vecRight, vecUp;
    QAngle qShotAng;
    Trace_t trace;

    for (int i = 0; i < (int)flTraceAmount; i++)
    {
        M::RandomSeed(i);

        float multiplier1 = M::RandomFloat(0.f, 1.f);
        float angle1 = M::RandomFloat(0.f, M_2PI);

        float multiplier2 = M::RandomFloat(0.f, 1.f);
        float angle2 = M::RandomFloat(0.f, M_2PI);

        float spreadOffsetX = cos(angle1) * (multiplier1 * flInaccuracy) + cos(angle2) * (multiplier2 * flSpread);
        float spreadOffsetY = sin(angle1) * (multiplier1 * flInaccuracy) + sin(angle2) * (multiplier2 * flSpread);

        qShotAng = qViewangles + QAngle(spreadOffsetX, spreadOffsetY, 0);

        M::AngleVectors(qShotAng, &vecForward, &vecRight, &vecUp);

        vecEnd = vecStart + (vecForward + vecRight + vecUp).Normalized() * pWeaponData->flRange;

        I::EngineTrace->ClipRayToEntity(Ray_t(vecStart, vecEnd), MASK_SHOT, pEntity, &trace);

        if (trace.pHitEntity == pEntity &&
            (trace.iHitGroup >= HITGROUP_HEAD && (trace.iHitGroup <= HITGROUP_RIGHTLEG) || (trace.iHitGroup == HITGROUP_GEAR)))
            iCurrentHits++;

        if (iCurrentHits >= iMinimumHits)
            return true;

        if (((int)flTraceAmount - i + iCurrentHits) < iMinimumHits)
            return false;
    }

    return false;
}

float CTriggerBot::AcceptedInaccuracy(CBaseCombatWeapon* pWeapon)
{
    float flInaccuracy = pWeapon->GetInaccuracy();
    if (flInaccuracy == 0)
        flInaccuracy = 0.0000001;

    return 1 / flInaccuracy;
}

std::optional<Vector> CTriggerBot::IntersectionPoint(Vector& start, Vector& end, Vector& mins, Vector& maxs, float radius) {
    auto sphereRayIntersection = [start, end, radius](Vector center) -> std::optional<Vector> {
        auto direction = (end - start).Normalized();

        auto q = center - start;
        auto v = q.DotProduct(direction);
        auto d = radius * radius - (q.LengthSqr() - v * v);

        if (d < FLT_EPSILON)
            return {};

        return start + direction * (v - std::sqrt(d));
    };

    auto delta = (maxs - mins).Normalized();
    for (size_t i{}; i < std::floor(mins.DistTo(maxs)); ++i)
    {
        if (auto intersection = sphereRayIntersection(mins + delta * float(i)); intersection)
            return intersection;
    }

    if (auto intersection = sphereRayIntersection(maxs); intersection)
        return intersection;

    return {};
}

std::optional<int> CTriggerBot::TargetRecord(CBaseEntity* entity, CBaseCombatWeapon* weapon, Vector& start, Vector& end) {
    auto hdr = I::ModelInfo->GetStudioModel(entity->GetModel());
    if (!hdr) return {};

    auto set = hdr->GetHitboxSet(0);
    if (!set)  return {};
    int iTicks = 0;

    for (auto j = 0; j < g_Backtrack.vRecords[entity->GetIndex()].size() && iTicks < C::Get<int>(Vars.iBacktrackMax); j++) {
        iTicks++;
        for (size_t i{}; i < set->nHitboxes; ++i)
        {
            auto hitbox = set->GetHitbox(i);
            if (!hitbox || hitbox->flRadius == -1.f)
                continue;

            Vector mins, maxs;
            mins = M::VectorTransform(hitbox->vecBBMin, g_Backtrack.vRecords[entity->GetIndex()].at(j).matrix[hitbox->iBone]);
            maxs = M::VectorTransform(hitbox->vecBBMax, g_Backtrack.vRecords[entity->GetIndex()].at(j).matrix[hitbox->iBone]);

            if (mins.IsZero() ||
                maxs.IsZero()) {
                continue;
            }

            if (auto intersection = IntersectionPoint(start, end, mins, maxs, hitbox->flRadius); intersection) {
                Ray_t ray;
                Trace_t trace;
                CTraceFilterSkipEntity filter(U::GetLocalPlayer());

                ray.Init(start, *intersection);

                I::EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);

                if (!trace.DidHit())
                    return j;
            }
        }
    }
    return {};
}
