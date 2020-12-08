#include "gui.h"

void GUI::Run()
{
	if (UTILS::KeyPressed(VK_DELETE) ||
		UTILS::KeyPressed(VK_INSERT))
	{
		m_bOpened = !m_bOpened;

		if (G::pLocal && !G::pLocal->IsAlive() || !I::Engine->IsInGame())
			I::InputSystem->EnableInput(!m_bOpened);
	}

	static float flMenuAlpha = 0.0f;
	CONTROLS::m_flMenuAlpha = flMenuAlpha * 255.0f;

	if (!m_bOpened)
	{
		if (G::bTakingInput)
			G::bTakingInput = false;

		CONTROLS::m_bCloseBoxes = false;

		flMenuAlpha -= (1.0f / 0.2f) * I::Globals->flFrameTime;
		CONTROLS::uInfo.m_flAlpha = flMenuAlpha;
		if (flMenuAlpha <= 0.01f)
		{
			flMenuAlpha = 0.0f;
			return;
		}
	}
	else 
	{
		flMenuAlpha += (1.0f / 0.2f) * I::Globals->flFrameTime;
		if (flMenuAlpha >= 0.9f)
			flMenuAlpha = 1.0f;
	}

	CONTROLS::BeginWindow({ (float)C::Get<int>(Vars.iMenuX), (float)C::Get<int>(Vars.iMenuY)}, { 600, 281 });
	{
		switch (CONTROLS::m_iSelectedTab)
		{
		case 0: // AIMBOT
		{
			CONTROLS::BeginGroupbox(_("AIMBOT"));
			{
				CONTROLS::Keybind(_("AIMBOT  KEY"), &C::Get<int>(Vars.iAimbotKey), &C::Get<int>(Vars.iAimbotKeyMethod));
				CONTROLS::Checkbox(_("ENABLE"), &(C::Get<bool>(Vars.bEnable)));
				CONTROLS::Combobox(_("HITBOX  SELECTION"), { _("HEAD"), _("CHEST"), _("PELVIS"), _("NEAREST") }, &(C::Get<int>(Vars.iHitboxSelection)));
				CONTROLS::Slider(_("FOV"), &C::Get<float>(Vars.fRegularFOV), 0.0f, 25.0f);
				CONTROLS::Slider(_("SMOOTHING"), &C::Get<float>(Vars.fSmoothing), 1.0f, 20.0f);
				CONTROLS::Slider(_("RECOIL  CONTROL"), &C::Get<float>(Vars.fRCS), 0.0f, 100.0f);
			}
			CONTROLS::EndGroupbox();

			CONTROLS::PopGroupboxY();

			CONTROLS::BeginGroupbox(_("TRIGGERBOT"), 1);
			{
				CONTROLS::Keybind(_("TRIGGERBOT  KEY"), &C::Get<int>(Vars.iTriggerbotKey), &C::Get<int>(Vars.iTriggerbotKeyMethod));
				CONTROLS::Checkbox(_("ENABLE  TRIGGERBOT"), &C::Get<bool>(Vars.bTriggerbot));

				CONTROLS::Checkbox(_("AUTOWALL"), &C::Get<bool>(Vars.bTriggerbotAutowall));
				CONTROLS::Slider(_("MINIMUM  DAMAGE"), &C::Get<float>(Vars.flTriggerbotAutowallMinDmg), 0, 100);
			}
			CONTROLS::EndGroupbox();
		}
		break;
		case 1: // VISUALS
		{
			CONTROLS::BeginGroupbox(_("PLAYERS"));
			{
				CONTROLS::Colorpicker(_("box_col"), &C::Get<Color>(Vars.colBox), &C::Get<Color>(Vars.colBox).arrColor.at(3));
				CONTROLS::Checkbox(_("BOX"), &C::Get<bool>(Vars.bBox));
				CONTROLS::Colorpicker(_("name_col"), &C::Get<Color>(Vars.colName), &C::Get<Color>(Vars.colName).arrColor.at(3));
				CONTROLS::Checkbox(_("NAME"), &C::Get<bool>(Vars.bName));
				CONTROLS::Checkbox(_("HEALTH"), &C::Get<bool>(Vars.bHealth));
				CONTROLS::Colorpicker(_("weapon_col"), &C::Get<Color>(Vars.colWeapon), &C::Get<Color>(Vars.colWeapon).arrColor.at(3));
				CONTROLS::Checkbox(_("WEAPON"), &C::Get<bool>(Vars.bWeapon));
				CONTROLS::Colorpicker(_("ammo_col"), &C::Get<Color>(Vars.colAmmo), &C::Get<Color>(Vars.colAmmo).arrColor.at(3));
				CONTROLS::Checkbox(_("AMMO"), &C::Get<bool>(Vars.bAmmo));
				CONTROLS::Colorpicker(_("glow_col"), &C::Get<Color>(Vars.colGlow), &C::Get<Color>(Vars.colGlow).arrColor.at(3), true);
				CONTROLS::Checkbox(_("GLOW"), &C::Get<bool>(Vars.bGlow));
			}
			CONTROLS::EndGroupbox();

			CONTROLS::PopGroupboxY();
				
			CONTROLS::BeginGroupbox(_("MISC"), 1);
			{
				CONTROLS::Checkbox(_("C4  TIMER"), &C::Get<bool>(Vars.bBombTimer));

				CONTROLS::Colorpicker(_("grenade_prediction"), &C::Get<Color>(Vars.colGrenadePrediction), &C::Get<Color>(Vars.colGrenadePrediction).arrColor.at(3));
				CONTROLS::Checkbox(_("GRENADE  PREDICTION"), &C::Get<bool>(Vars.bGrenadePrediction));
				
				CONTROLS::Checkbox(_("SPECTATOR  LIST"), &C::Get<bool>(Vars.bSpectatorList));
				CONTROLS::Checkbox(_("HITMARKER"), &C::Get<bool>(Vars.bHitmarker));
			}
			CONTROLS::EndGroupbox();
		}
		break;
		case 2: // MISC
		{
			CONTROLS::BeginGroupbox(_("MISC"));
			{
				CONTROLS::Combobox(_("BUNNYHOP"), { _("OFF"), _("PERFECT"), _("LEGIT") }, & C::Get<int>(Vars.iBunnyHop));
				if (C::Get<int>(Vars.iBunnyHop) == 2)
					CONTROLS::Slider(_("BUNNYHOP  HITCHANCE"), &C::Get<float>(Vars.flBunnyhopHitchance), 0.0f, 100.0f);

				CONTROLS::Checkbox(_("CLANTAG  SPAMMER"), &C::Get<bool>(Vars.bClantagChanger));
				CONTROLS::Checkbox(_("ENABLE  BACKTRACK"), &C::Get<bool>(Vars.bBacktrack));

				CONTROLS::Combobox(_("KNIFE MODEL"), {
					_("DEFAULT"),
					_("BAYONET"),
					_("FLIP  KNIFE"),
					_("GUT  KNIFE"),
					_("KARAMBIT"),
					_("M9  BAYONET"),
					_("HUNTSMAN  KNIFE"),
					_("FALCHION  KNIFE"),
					_("BOWIE  KNIFE"),
					_("BUTTERFLY  KNIFE"),
					_("SHADOW  DAGGERS"),
					_("URSUS  KNIFE"),
					_("NAVAJA  KNIFE"),
					_("STILETTO  KNIFE"),
					_("TALON  KNIFE"),
					_("CLASSIC  KNIFE"),
					_("OUTDOOR  KNIFE"),
					_("SKELETON  KNIFE"),
					_("CORD  KNIFE"),
					_("CANIS  KNIFE")
					}, &C::Get<int>(Vars.iKnifeModel));

				static int iKnifeModel = 0;
				if (iKnifeModel != C::Get<int>(Vars.iKnifeModel))
				{
					iKnifeModel = C::Get<int>(Vars.iKnifeModel);
					U::ForceFullUpdate();
				}
			}
			CONTROLS::EndGroupbox();
				
			CONTROLS::PopGroupboxY();
				
			CONTROLS::BeginGroupbox(_("CONFIGS"), 1);
			{
				CONTROLS::Button(_("SAVE  CONFIG"), [&](bool& holder) {
					C::Save("default.cfg");
					holder = !holder;
					});
				
				CONTROLS::Button(_("LOAD  CONFIG"), [&](bool& holder) {
					C::Load("default.cfg");
					U::ForceFullUpdate();
					holder = !holder;
					});
			}
			CONTROLS::EndGroupbox();
		}
		break;
		default:
			break;
		}
	}
	CONTROLS::EndWindow();
}