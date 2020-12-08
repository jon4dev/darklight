#include <array>

#include "hooks.h"
#include "../global.h"
#include "variables.h"
#include "../utilities/inputsystem.h"
#include "../utilities/logging.h"
#include "../utilities/draw.h"
#include "../utilities.h"

#include "../features/prediction.h"
#include "../features/triggerbot.h"
#include "../features/visuals.h"
#include "../features/misc.h"
#include "../features/aimbot.h"
#include "../features/backtrack.h"
#include "../features/skinchanger.h"

#include "gui/gui.h"
#include <windowsx.h>

bool H::Setup()
{
	if (MH_Initialize() != MH_OK)
		throw std::runtime_error(_("Failed MH_Initialize"));

	if (!DTR::FrameStageNotify.Create(MEM::GetVFunc(I::Client, VTABLE::FRAMESTAGENOTIFY), &hkFrameStageNotify))
		throw std::runtime_error(_("Failed OverrideView"));

	if (!DTR::OverrideView.Create(MEM::GetVFunc(I::ClientMode, VTABLE::OVERRIDEVIEW), &hkOverrideView))
		throw std::runtime_error(_("Failed OverrideView"));

	if (!DTR::CreateMove.Create(MEM::GetVFunc(I::ClientMode, VTABLE::CREATEMOVE), &hkCreateMove))
		throw std::runtime_error(_("Failed CreateMove"));

	if (!DTR::DoPostScreenEffects.Create(MEM::GetVFunc(I::ClientMode, VTABLE::DOPOSTSCREENEFFECTS), &hkDoPostScreenEffects))
		throw std::runtime_error(_("Failed DoPostScreenEffects"));

	if (!DTR::PaintTraverse.Create(MEM::GetVFunc(I::Panel, VTABLE::PAINTTRAVERSE), &hkPaintTraverse))
		throw std::runtime_error(_("Failed PaintTraverse"));

	if (!DTR::RunCommand.Create(MEM::GetVFunc(I::Prediction, VTABLE::RUNCOMMAND), &hkRunCommand))
		throw std::runtime_error(_("Failed RunCommand"));

	if (!DTR::LockCursor.Create(MEM::GetVFunc(I::Surface, VTABLE::LOCKCURSOR), &hkLockCursor))
		throw std::runtime_error(_("Failed LockCursor"));
	return true;
}

void H::Restore()
{
	DTR::FrameStageNotify.~CDetourHook();
	DTR::OverrideView.~CDetourHook();
	DTR::CreateMove.~CDetourHook();
	DTR::SendNetMsg.~CDetourHook();
	DTR::DoPostScreenEffects.~CDetourHook();
	DTR::PaintTraverse.~CDetourHook();
	DTR::RunCommand.~CDetourHook();
	DTR::LockCursor.~CDetourHook();

	#if 0
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	#endif

	MH_Uninitialize();
}

bool FASTCALL H::hkCreateMove(IClientModeShared* thisptr, int edx, float flInputSampleTime, CUserCmd* pCmd)
{
	static auto oCreateMove = DTR::CreateMove.GetOriginal<decltype(&hkCreateMove)>();
	oCreateMove(thisptr, edx, flInputSampleTime, pCmd);

	CBaseEntity* pLocal = G::pLocal = U::GetLocalPlayer();
	
	if (pCmd->iCommandNumber == 0 || !pLocal || !pCmd)
		return oCreateMove(thisptr, edx, flInputSampleTime, pCmd);

	if (oCreateMove(thisptr, edx, flInputSampleTime, pCmd))
		I::Prediction->SetLocalViewAngles(pCmd->angViewPoint);

	G::pCmd = pCmd;

	if (I::ClientState == nullptr || I::Engine->IsPlayingDemo())
		return oCreateMove(thisptr, edx, flInputSampleTime, pCmd);

	if (!G::pLocal || !G::pCmd)
		return oCreateMove(thisptr, edx, flInputSampleTime, pCmd);

	INetChannel* pNetChannel = (INetChannel*)I::ClientState->pNetChannel;

	const volatile auto vlBaseAddress = *(std::uintptr_t*)((std::uintptr_t)_AddressOfReturnAddress() - sizeof(std::uintptr_t));

	bool& bSendPacket = *(bool*)(vlBaseAddress - 0x1C);

	QAngle angOldViewPoint = pCmd->angViewPoint;

	G::cmd = *pCmd;

	if (GUI::m_bOpened)
	{
		if (pCmd->iButtons & IN_ATTACK)
			pCmd->iButtons &= ~IN_ATTACK;

		if (pCmd->iButtons & IN_SECOND_ATTACK)
			pCmd->iButtons &= ~IN_SECOND_ATTACK;
	}

	if (I::ClientState->iDeltaTick > 0)
		I::Prediction->Update(I::ClientState->iDeltaTick, I::ClientState->iDeltaTick > 0, I::ClientState->nLastCommandAck, I::ClientState->nLastOutgoingCommand + I::ClientState->iChokedCommands);

	g_Misc.Run(pCmd, pLocal, bSendPacket);

	const int iFlags = pLocal->GetFlags();

	g_Prediction.Start(pCmd, pLocal);
	{
		g_Backtrack.Run(pCmd, pLocal, bSendPacket);
		g_Aimbot.Run(pCmd, pLocal, bSendPacket);
		g_Triggerbot.Run(pCmd, pLocal);
	}
	g_Prediction.End(pCmd, pLocal);

	g_Misc.CorrectMovement(pLocal, pCmd, angOldViewPoint);

	if (pNetChannel != nullptr)
	{
		if (!DTR::SendNetMsg.IsHooked())
			DTR::SendNetMsg.Create(MEM::GetVFunc(pNetChannel, VTABLE::SENDNETMSG), H::hkSendNetMsg);
	}

	G::angRealView = pCmd->angViewPoint;
	G::bSendPacket = bSendPacket;

	return false;
}

void FASTCALL H::hkPaintTraverse(ISurface* thisptr, int edx, unsigned int uPanel, bool bForceRepaint, bool bForce)
{
	static auto oPaintTraverse = DTR::PaintTraverse.GetOriginal<decltype(&hkPaintTraverse)>();
	oPaintTraverse(thisptr, edx, uPanel, bForceRepaint, bForce);

	int iCurrentWidth, iCurrentHeight = 0;
	I::Engine->GetScreenSize(iCurrentWidth, iCurrentHeight);
	if (G::vecDisplaySize.x != iCurrentWidth || G::vecDisplaySize.y != iCurrentHeight)
	{
		G::vecDisplaySize.x = iCurrentWidth, G::vecDisplaySize.y = iCurrentHeight;
		D::Initialize();
	}

	auto nPanel = FNV1A::HashConst(I::Panel->GetName(uPanel));
	switch (nPanel)
	{
	case FNV1A::HashConst(("MatSystemTopPanel")):
	{
		GUI::UTILS::InitializeInput(_("Counter-Strike: Global Offensive"));

		g_Visuals.Run();
		GUI::Run();
	} break;
	default:
		break;
	}
}

void FASTCALL H::hkLockCursor(ISurface* thisptr, int edx)
{
	static auto oLockCursor = DTR::LockCursor.GetOriginal<decltype(&hkLockCursor)>();

	if (G::pLocal && G::pLocal->IsAlive() && I::Engine->IsInGame())
		I::InputSystem->EnableInput(true);

	if (GUI::m_bOpened)
		I::Surface->UnLockCursor();
	else
		oLockCursor(thisptr, edx);
}

void FASTCALL H::hkFrameStageNotify(IBaseClientDll* thisptr, int edx, EClientFrameStage stage)
{
	static auto oFrameStageNotify = DTR::FrameStageNotify.GetOriginal<decltype(&hkFrameStageNotify)>();

	if (!I::Engine->IsInGame())
		return oFrameStageNotify(thisptr, edx, stage);

	if (I::Engine->IsTakingScreenshot())
		return oFrameStageNotify(thisptr, edx, stage);

	static int iOriginalIndex = 0;

	if (!G::pLocal) 
	{
		iOriginalIndex = 0;
		return oFrameStageNotify(thisptr, edx, stage);
	}

	switch (stage)
	{
	case FRAME_RENDER_START:
	{
		/*
		 * start rendering the scene
		 * e.g. remove visual punch, thirdperson, other render/update stuff
		 */

		break;
	};
	case FRAME_NET_UPDATE_END:
	{
		/*
		 * received all packets, now do interpolation, prediction, etc
		 * e.g. backtrack stuff
		 */
		break;
	};
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
	{
		/*
		 * data has been received and we are going to start calling postdataupdate
		 * e.g. resolver or skinchanger and other visuals
		 */

		g_Skinchanger.Run();
		break;
	};
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
	{
		/*
		 * data has been received and called postdataupdate on all data recipients
		 * e.g. now we can modify interpolation, other lagcompensation stuff
		 */
		break;
	}
	default:
		break;
	}

	oFrameStageNotify(thisptr, edx, stage);
}

bool FASTCALL H::hkSendNetMsg(INetChannel* thisptr, int edx, INetMessage* pMessage, bool bForceReliable, bool bVoice)
{
	static auto oSendNetMsg = DTR::SendNetMsg.GetOriginal<decltype(&hkSendNetMsg)>();

	if (pMessage->GetGroup() == 9)
		bVoice = true;

	return oSendNetMsg(thisptr, edx, pMessage, bForceReliable, bVoice);
}

void FASTCALL H::hkOverrideView(IClientModeShared* thisptr, int edx, CViewSetup* pSetup)
{
	static auto oOverrideView = DTR::OverrideView.GetOriginal<decltype(&hkOverrideView)>();
	
	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oOverrideView(thisptr, edx, pSetup);

	G::vecCamera = pSetup->vecOrigin;

	if (!G::pLocal || !G::pLocal->IsAlive())
		return oOverrideView(thisptr, edx, pSetup);

	g_Visuals.OverrideView(pSetup);

	oOverrideView(thisptr, edx, pSetup);
}

int FASTCALL H::hkDoPostScreenEffects(IClientModeShared* thisptr, int edx, CViewSetup* pSetup)
{
	static auto oDoPostScreenEffects = DTR::DoPostScreenEffects.GetOriginal<decltype(&hkDoPostScreenEffects)>();

	if (!I::Engine->IsInGame() || !I::Engine->IsConnected())
		return oDoPostScreenEffects(thisptr, edx, pSetup);

	if (G::pLocal && I::GlowManager && C::Get<bool>(Vars.bGlow))
		g_Visuals.Glow(G::pLocal);

	return oDoPostScreenEffects(thisptr, edx, pSetup);
}

void FASTCALL H::hkRunCommand(IPrediction* thisptr, int edx, CBaseEntity* pEntity, CUserCmd* pCmd, IMoveHelper* pMoveHelper)
{
	static auto oRunCommand = DTR::RunCommand.GetOriginal<decltype(&hkRunCommand)>();
	oRunCommand(thisptr, edx, pEntity, pCmd, pMoveHelper);

	I::MoveHelper = pMoveHelper;
}

long CALLBACK H::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (G::bTakingInput)
		return 0;

	return CallWindowProcW(IPT::pOldWndProc, hWnd, uMsg, wParam, lParam);
}

std::vector<std::string> vWeaponNames = {
	_("models/weapons/v_knife_bayonet.mdl"),
	_("models/weapons/v_knife_flip.mdl"),
	_("models/weapons/v_knife_gut.mdl"),
	_("models/weapons/v_knife_karam.mdl"),
	_("models/weapons/v_knife_m9_bay.mdl"),
	_("models/weapons/v_knife_tactical.mdl"),
	_("models/weapons/v_knife_falchion_advanced.mdl"),
	_("models/weapons/v_knife_survival_bowie.mdl"),
	_("models/weapons/v_knife_butterfly.mdl"),
	_("models/weapons/v_knife_push.mdl"),
	_("models/weapons/v_knife_ursus.mdl") ,
	_("models/weapons/v_knife_gypsy_jackknife.mdl"),
	_("models/weapons/v_knife_stiletto.mdl"),
	_("models/weapons/v_knife_widowmaker.mdl"),
	_("models/weapons/v_knife_css.mdl"),
	_("models/weapons/v_knife_outdoor.mdl"),
	_("models/weapons/v_knife_skeleton.mdl"),
	_("models/weapons/v_knife_cord.mdl"),
	_("models/weapons/v_knife_canis.mdl"),
};

int random_sequence(int low, int high) {
	return rand() % (high - low + 1) + low;
}

std::unordered_map<std::string, int(*)(int)> animation_fix_map{
	{ _("models/weapons/v_knife_butterfly.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
			default:
				return sequence + 1;
			}
	} },
	{ _("models/weapons/v_knife_falchion_advanced.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_FALCHION_IDLE1;
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence - 1;
			}
	} },
	{ _("models/weapons/v_knife_css.mdl"), [](int sequence) -> int
	{
		switch (sequence)
		{
		case SEQUENCE_DEFAULT_LOOKAT01:
			return 15;
		default:
			return sequence;
		}
	} },
	{ _("models/weapons/v_knife_push.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_DAGGERS_IDLE1;
			case SEQUENCE_DEFAULT_LIGHT_MISS1:
			case SEQUENCE_DEFAULT_LIGHT_MISS2:
				return random_sequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
			case SEQUENCE_DEFAULT_HEAVY_HIT1:
			case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
			case SEQUENCE_DEFAULT_LOOKAT01:
				return sequence + 3;
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence + 2;
			}
	} },
	{ _("models/weapons/v_knife_survival_bowie.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_BOWIE_IDLE1;
			default:
				return sequence - 1;
			}
	} },
	{ _("models/weapons/v_knife_ursus.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
			default:
				return sequence + 1;
			}
	} },
	{ _("models/weapons/v_knife_stiletto.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(12, 13);
			default:
				return sequence;
			}
	} },
	{ _("models/weapons/v_knife_widowmaker.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(14, 15);
			default:
				return sequence;
			}
	} },
	{ _("models/weapons/v_knife_cord.mdl"), [](int sequence) -> int
	{
		switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
			default:
				return sequence + 1;
			}
	} },
	{ _("models/weapons/v_knife_canis.mdl"), [](int sequence) -> int
	{
			switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
			default:
				return sequence + 1;
			}
	} },
	{ _("models/weapons/v_knife_outdoor.mdl"), [](int sequence) -> int
	{
			switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
			default:
				return sequence + 1;
			}
	} },
	{ _("models/weapons/v_knife_skeleton.mdl"), [](int sequence) -> int
	{
			switch (sequence)
			{
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
			default:
				return sequence + 1;
			}
	} }
};

#pragma region proxies_get
bool P::Setup()
{
	RecvProp_t* pSequence = CNetvarManager::Get().mapProps[FNV1A::HashConst(_("CBaseViewModel->m_nSequence"))].pRecvProp;
	if (!pSequence)
		return false;

	RVP::Sequence = std::make_shared<CRecvPropHook>(pSequence, P::Sequence);

	return true;
}

void P::Restore()
{
	RVP::BaseViewModel->~CRecvPropHook();
}
#pragma endregion

#pragma region proxies_handlers
void P::Sequence(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	static auto oSequence = RVP::Sequence->GetOriginal();

	if (!G::pLocal || !G::pLocal->IsAlive())
	{
		oSequence(pData, pStruct, pOut);
		return;
	}

	CRecvProxyData* pProxyData = const_cast<CRecvProxyData*>(pData);
	CBaseViewModel* pViewModel = static_cast<CBaseViewModel*>(pStruct);

	if (pViewModel && pViewModel->GetOwnerHandle())
	{
		if (G::pLocal->GetWeapon() && C::Get<int>(Vars.iKnifeModel)) {
			auto pKnifeModel = I::ModelInfo->GetModel(pViewModel->GetModelIndex());
			auto szKnifeName = I::ModelInfo->GetModelName(pKnifeModel);

			if (animation_fix_map.count(szKnifeName))
				pProxyData->Value.Int = animation_fix_map.at(szKnifeName)(pProxyData->Value.Int);
		}
	}

	oSequence(pProxyData, pStruct, pOut);
}
#pragma endregion