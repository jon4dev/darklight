#include "common.h"
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

#include "global.h"
#include "core/variables.h"
#include "utilities.h"
#include "utilities/logging.h"
#include "utilities/math.h"
#include "utilities/inputsystem.h"
#include "utilities/draw.h"
#include "core/netvar.h"
#include "core/config.h"
#include "core/hooks.h"
#include "features/visuals.h"
#include "core/gui/gui.h"
#include "../Dependencies/json/json.hpp"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "libcurl_a.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "normaliz.lib")

using json = nlohmann::json;

DWORD WINAPI OnDllAttach(LPVOID lpParameter)
{
	try
	{
		while (GetModuleHandle(SERVERBROWSER_DLL) == nullptr)
			std::this_thread::sleep_for(200ms);

		//ForceBluescreen(STATUS_ASSERTION_FAILURE);

		#if DEBUG_CONSOLE
		if (!L::Attach(_("Darklight Debug Console")))
			throw std::runtime_error(_("Failed to attach console"));

		L::Print(_("Console opened"));
		#else
		L::ofsFile.open(C::GetWorkingPath().append(_("debug.log")), std::ios::out | std::ios::trunc);
		#endif

		G::szUsername = _("Darklight");

		if (!I::Setup())
			throw std::runtime_error(_("Failed to capture interfaces"));

		L::Print(_("Interfaces captured"));

		if (strcmp(I::Engine->GetProductVersionString(), _("1.37.7.0")) != 0)
			throw std::runtime_error(_("Bad client version."));

		#if DEBUG_CONSOLE
		if (strcmp(I::Engine->GetProductVersionString(), _("1.37.7.0")) != 0)
		{
			L::PushConsoleColor(FOREGROUND_RED | FOREGROUND_YELLOW);
			L::Print(fmt::format(_("[Warning] Version doesnt match! current CS:GO version: {}"), I::Engine->GetProductVersionString()));
			L::PopConsoleColor();
		}
		#endif

		if (!CNetvarManager::Get().Setup(_("netvars.txt")))
			throw std::runtime_error(_("Failed to initialize netvars"));

		L::Print(fmt::format(_("Found [{:d}] props in [{:d}] tables"), CNetvarManager::Get().iStoredProps, CNetvarManager::Get().iStoredTables));

		/*CNetvarManager::Get().GrabOffsets();
		L::Print(XorStr("Grab offsets successful"));*/

		if (!M::Setup())
			throw std::runtime_error(_("Failed to get math exports"));

		L::Print(_("Math exports loaded"));

		if (!IPT::Setup())
			throw std::runtime_error(_("Failed to set window messages processor"));

		L::Print(_("InputSystem setup complete"));

		//U::EntityListener.Setup();
		//L::Print(XorStr("Entity Listener initialized"));

		U::EventListener.Setup(
			{ 
				_("player_hurt"),
				_("item_purchase"),
				_("player_given_c4"),
				_("bomb_beginplant"),
				_("bomb_abortplant"),
				_("bomb_planted"),
				_("bomb_begindefuse"),
				_("bomb_abortdefuse"),
				_("player_death")
			});
		L::Print(_("Events registered"));

		if (!H::Setup())
			throw std::runtime_error(_("Failed initialize hooks"));

		L::Print(_("Hooks setup complete"));

		if (!P::Setup())
			throw std::runtime_error(_("Failed initialize proxies"));

		L::Print(_("Proxies applied"));

		D::Initialize();

		if (!C::Setup(_("default.cfg")))
		{
			L::PushConsoleColor(FOREGROUND_RED);
			L::Print(_("[Error] Failed setup and/or load default configuration"));
			L::PopConsoleColor();
		}
		else
			L::Print(_("Default config loaded"));

		L::PushConsoleColor(FOREGROUND_MAGENTA);
		L::Print(_("Darklight successfully loaded"));
		L::PopConsoleColor();
		I::GameConsole->Clear();

		I::ConVar->ConsoleColorPrintf(Color(GUI::CONTROLS::m_cDefaultMenuCol), _("Darklight Free - Build date: %s / %s\n"), __DATE__, __TIME__);
		I::ConVar->ConsoleColorPrintf(Color(255, 255, 255), _("["));
		I::ConVar->ConsoleColorPrintf(Color(GUI::CONTROLS::m_cDefaultMenuCol), _("Darklight"));
		I::ConVar->ConsoleColorPrintf(Color(255, 255, 255), _("]"));
		I::ConVar->ConsoleColorPrintf(Color(255, 255, 255), fmt::format(_(" Welcome free user.\n")).c_str());
	}
	catch (const std::runtime_error& ex)
	{
		MessageBox(nullptr, ex.what(), nullptr, MB_OK | MB_ICONERROR | MB_TOPMOST);
		FreeLibraryAndExitThread((HMODULE)lpParameter, EXIT_SUCCESS);
	}

	return 1UL;
}

DWORD WINAPI OnDllDetach(LPVOID lpParameter)
{
	while (!IPT::IsKeyReleased(VK_F8))
		std::this_thread::sleep_for(500ms);

	#if DEBUG_CONSOLE
	L::Detach();
	#else
	if (L::ofsFile.is_open())
		L::ofsFile.close();
	#endif

	U::EntityListener.Destroy();

	U::EventListener.Destroy();

	I::ConVar->FindVar(_("crosshair"))->SetValue(true);

	#if 0
	P::Restore();
	#endif

	H::Restore();

	IPT::Restore();

	FreeLibraryAndExitThread((HMODULE)lpParameter, EXIT_SUCCESS);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		if (GetModuleHandle(_("csgo.exe")) == nullptr)
			return FALSE;

		DisableThreadLibraryCalls(hModule);

		const auto hThread = CreateThread(nullptr, 0U, OnDllAttach, hModule, 0UL, nullptr);
		if (!hThread)
			return FALSE;

		G::hDll = hModule;
		
		CloseHandle(hThread);

		return TRUE;
	}

	return FALSE;
}
