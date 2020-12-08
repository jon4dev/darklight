#pragma once
#include <deque>
#include "../common.h"
#include "../sdk/datatypes/vector.h"
#include "../sdk/datatypes/color.h"
#include "../sdk/entity.h"
#include "../sdk/interfaces/igameeventmanager.h"
#include "../core/interfaces.h"

struct BBox_t {
	BBox_t() = default;
	BBox_t(float _flX, float _flY, float _flW, float _flH)
	{
		flX = _flX;
		flY = _flY;
		flW = _flW;
		flH = _flH;
	}

	float flX;
	float flY;
	float flW;
	float flH;
};

class CVisuals
{
public:
	void Run();

	void Glow(CBaseEntity* pLocal);

	void OverrideView(CViewSetup* setup);
	void Paint();

	void Event(IGameEvent* pEvent, const FNV1A_t uNameHash);
	void Hitmarker();

	float m_flHitmarkerTime = 0.0f;
	bool m_bGrabbingSpectatorList;

private:
	std::vector<Vector> vecPath;
	std::vector<Vector> vecCollision;
	int type = 0;
	int act = 0;

	void Setup(Vector& vecSrc, Vector& vecThrow, QAngle viewangles);
	void Simulate(CViewSetup* setup);

	int Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval);
	bool CheckDetonate(const Vector& vecThrow, const Trace_t& tr, int tick, float interval);

	void TraceHull(Vector& src, Vector& end, Trace_t& tr);
	void AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground);
	void PushEntity(Vector& src, const Vector& move, Trace_t& tr);
	void ResolveFlyCollisionCustom(Trace_t& tr, Vector& vecVelocity, float interval);
	int PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);

	void CollectEntities();

	void PaintPlayers(CBaseEntity* pEntity);
	void PaintC4(CBaseEntity* pEntity);

	bool CreateBBox(CBaseEntity* pEntity, BBox_t& bbox);

	bool WorldToScreen(const Vector& vOrigin, Vector& vScreen);

	void SpectatorList(CBaseEntity* pEntity);
	void SpectatorFrame();

	Vector2D m_vecPos = { 250, 250 };
	Vector2D m_vecSize;
	int m_iTotalSpectators;
};

extern CVisuals g_Visuals;