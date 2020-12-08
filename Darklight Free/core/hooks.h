#pragma once
// used: winapi, directx, fmt includes
#include "../common.h"
// used: hook setup/destroy
#include "../utilities/detourhook.h"
// used: recvprop hook setup/destroy, recvproxydata
#include "netvar.h"
// used: baseclasses
#include "interfaces.h"

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

/*
* VTABLE INDEXES
* functions indexes in their virtual tables
*/
namespace VTABLE
{
	// work with namespace cuz if use enum class need additional convert to int
	enum
	{
		/* directx table */
		RESET = 16,
		PRESENT = 17,
		ENDSCENE = 42,
		RESETEX = 132,

		/* client table */
		FRAMESTAGENOTIFY = 37,

		/* panel table */
		PAINTTRAVERSE = 41,

		/* clientmode table */
		OVERRIDEVIEW = 18,
		OVERRIDEMOUSEINPUT = 23,
		CREATEMOVE = 24,
		GETVIEWMODELFOV = 35,
		DOPOSTSCREENEFFECTS = 44,

		/* modelrender table */
		DRAWMODELEXECUTE = 21,

		/* studiorender table */
		DRAWMODEL = 29,

		/* engine table */
		ISCONNECTED = 27,

		/* bsp query table */
		LISTLEAVESINBOX = 6,

		/* prediction table */
		RUNCOMMAND = 19,

		/* steamgamecoordinator table */
		SENDMESSAGE = 0,
		RETRIEVEMESSAGE = 2,

		/* sound table */
		EMITSOUND = 5,

		/* materialsystem table */
		OVERRIDECONFIG = 21,
		FINDMATERIAL = 84,

		/* renderview table */
		SCENEEND = 9,

		/* surface table */
		LOCKCURSOR = 67,
		PLAYSOUND = 82,

		/* gameevent table */
		FIREEVENT = 9,

		/* convar table */
		GETBOOL = 13,

		/* netchannel table */
		SENDNETMSG = 40,
		SENDDATAGRAM = 46,
	};
}

/*
 * DETOURS
 * detour hook managers
 */
namespace DTR
{
	inline CDetourHook Reset;
	inline CDetourHook Present;
	inline CDetourHook FrameStageNotify;
	inline CDetourHook OverrideView;
	inline CDetourHook OverrideMouseInput;
	inline CDetourHook OverrideConfig;
	inline CDetourHook CreateMove;
	inline CDetourHook SendNetMsg;
	inline CDetourHook SendDatagram;
	inline CDetourHook GetViewModelFOV;
	inline CDetourHook DoPostScreenEffects;
	inline CDetourHook IsConnected;
	inline CDetourHook ListLeavesInBox;
	inline CDetourHook PaintTraverse;
	inline CDetourHook DrawModel;
	inline CDetourHook RunCommand;
	inline CDetourHook SendMessageGC;
	inline CDetourHook RetrieveMessage;
	inline CDetourHook EmitSound;
	inline CDetourHook LockCursor;
	inline CDetourHook PlaySoundSurface;
	inline CDetourHook SvCheatsGetBool;
	inline CDetourHook FindMaterial;
	inline CDetourHook SceneEnd;
	inline CDetourHook FireEvent;
	inline CDetourHook DrawModelExecute;
	inline CDetourHook IsHLTV;
	inline CDetourHook IsPaused;
	inline CDetourHook LevelInitPostEntity;
	inline CDetourHook CsmShadowGetInt;
	inline CDetourHook WeaponDebugSpreadShow;
	inline CDetourHook CrosshairRecoil;
	inline CDetourHook ShouldDrawFog;
}

/*
 * HOOKS
 * swap functions with given pointers
 */
namespace H
{
	// Get
	bool	Setup();
	void	Restore();

	// Handlers
	/* [type][call]		hk[name] (args...) */
	long	D3DAPI		hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	long	D3DAPI		hkPresent(IDirect3DDevice9* pDevice, RECT* pSourceRect, RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion);
	bool	FASTCALL	hkCreateMove(IClientModeShared* thisptr, int edx, float flInputSampleTime, CUserCmd* pCmd);
	void	FASTCALL	hkPaintTraverse(ISurface* thisptr, int edx, unsigned int uPanel, bool bForceRepaint, bool bForce);
	void	FASTCALL	hkLockCursor(ISurface* thisptr, int edx);
	void	FASTCALL	hkFrameStageNotify(IBaseClientDll* thisptr, int edx, EClientFrameStage stage);
	void	FASTCALL	hkDrawModel(IStudioRender* thisptr, int edx, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags);
	bool	FASTCALL	hkIsConnected(IEngineClient* thisptr, int edx);
	bool	FASTCALL	hkSendNetMsg(INetChannel* thisptr, int edx, INetMessage* pMessage, bool bForceReliable, bool bVoice);
	void	FASTCALL	hkOverrideView(IClientModeShared* thisptr, int edx, CViewSetup* pSetup);
	int		FASTCALL	hkDoPostScreenEffects(IClientModeShared* thisptr, int edx, CViewSetup* pSetup);
	void	FASTCALL	hkRunCommand(IPrediction* thisptr, int edx, CBaseEntity* pEntity, CUserCmd* pCmd, IMoveHelper* pMoveHelper);
	int		FASTCALL	hkSendMessage(ISteamGameCoordinator* thisptr, int edx, std::uint32_t uMsgType, const void* pData, std::uint32_t uData);
	int		FASTCALL	hkRetrieveMessage(ISteamGameCoordinator* thisptr, int edx, std::uint32_t* puMsgType, void* pDest, std::uint32_t uDest, std::uint32_t* puMsgSize);
	bool	FASTCALL	hkSvCheatsGetBool(CConVar* thisptr, int edx);
	long	CALLBACK	hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void	FASTCALL	hkSceneEnd(void* _this);
	bool    FASTCALL    hkOverrideConfig(void* pThis, void* edx, MaterialSystemConfig_t* pConfig, bool bUpdate);
	bool	FASTCALL	hkIsHLTV(void* pThis, void* edx);
	void	FASTCALL	hkLevelInitPostEntity(void* pThis, void* edx);
}

/*
 * RECV VAR PROXY MANAGERS
 * proxy property hook managers
 */
namespace RVP
{
	inline std::shared_ptr<CRecvPropHook> BaseViewModel;
	inline std::shared_ptr<CRecvPropHook> Sequence;
}

/*
 * PROXIES
 * networkable property proxy swap functions
 */
namespace P
{
	// Get
	bool	Setup();
	void	Restore();

	// Handlers
	//void	BaseViewModel(const CRecvProxyData* pData, void* pStruct, void* pOut);
	void	Sequence(const CRecvProxyData* pData, void* pStruct, void* pOut);
}
