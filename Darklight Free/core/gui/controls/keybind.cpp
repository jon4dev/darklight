#include "../gui.h"

const char* szKeyStrings[254] = {
	("-"),("M1"),("M2"),("BRK"),("M3"),("M4"),("M5"),
	("-"),("BACK"),("TAB"),("-"),("-"),("-"),("ENT"),("-"),("-"),("SHFT"),
	("CTRL"),("ALT"),("PAUSE"),("CAPS"),("-"),("-"),("-"),("-"),("-"),("-"),
	("ESC"),("-"),("-"),("-"),("-"),("SPCE"),("PGUP"),("PGDN"),("END"),("HOME"),("LFT"),
	("UP"),("RGHT"),("DWN"),("-"),("PRNT"),("-"),("PRNTSCR"),("INS"),("DEL"),("-"),("0"),("1"),
	("2"),("3"),("4"),("5"),("6"),("7"),("8"),("9"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("A"),("B"),("C"),("D"),("E"),("F"),("G"),("H"),("I"),("J"),("K"),("L"),("M"),("N"),("O"),("P"),("Q"),("R"),("S"),("T"),("U"),
	("V"),("W"),("X"),("Y"),("Z"),("LWIN"),("RWIN"),("-"),("-"),("-"),("NUM0"),("NUM1"),
	("NUM2"),("NUM3"),("NUM4"),("NUM5"),("NUM6"),("NUM7"),("NUM8"),("NUM9"),("*"),("+"),(""),("-"),("."),("/"),("F1"),("F2"),("F3"),
	("F4"),("F5"),("F6"),("F7"),("F8"),("F9"),("F10"),("F11"),("F12"),("F13"),("F14"),("F15"),("F16"),("F17"),("F18"),("F19"),("F20"),
	("F21"),("F22"),("F23"),("F24"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("NUMLCK"),("SCRLLCK"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("-"),("-"),("-"),("-"),("-"),("-"),("LSHFT"),("RSHFT"),("LCTRL"),
	("RCTRL"),("LMENU"),("RMENU"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("-"),("-"),("NTRK"),("PTRK"),("STOP"),("PLAY"),("-"),("-"),
	("-"),("-"),("-"),("-"),(";"),("+"),("),"),("-"),("."),("/?"),("~"),("-"),("-"),
	("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("-"),("-"),("-"),("-"),("-"),("{"),("\\|"),("}"),("'\""),("-"),
	("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),("-"),
	("-"),("-")
};

void GUI::CONTROLS::Keybind(const std::string& szName, int* pBind, int* pMethod, bool bAllowMethod)
{
	if (!m_bInitialized)
		return;

	static std::string szSelectedName;
	static auto bIsGettingKey = false;

	static auto flTimeClicked = 0.0f;
	const char* key_name = "-";
	static auto good_key_name = false;
	char name_buffer[128];

	if (bIsGettingKey && szSelectedName == szName) {
		key_name = "-";
		for (int i = 0; i < 255; i++) {
			if (UTILS::KeyPressed(i)) {
				*pBind = i == VK_ESCAPE ? -1 : i;
				bIsGettingKey = false;
				flTimeClicked = 0.0f;
			}
		}
	}
	else {
		if (*pBind >= 0) {
			key_name = szKeyStrings[*pBind];
			if (key_name) {
				good_key_name = true;
			}
			else {
				if (GetKeyNameText(*pBind << 16, name_buffer, 127)) {
					key_name = name_buffer;
					good_key_name = true;
				}
			}
		}

		if (!good_key_name) {
			key_name = "-";
		}
	}

	static std::vector<std::string> vItems = { _("ALWAYS ON"), _("HOLD"), _("TOGGLE"), _("FORCE OFF") };

	std::string szBuiltKeyName;
	szBuiltKeyName += _("[");
	szBuiltKeyName += key_name;
	szBuiltKeyName += _("]");

	auto tSize = D::GetTextSize(D::uSmallFonts[G::iDPIScale], szBuiltKeyName.c_str());

	D::String(m_vOffset.x + 165 - tSize.right, m_vOffset.y + 1, D::uSmallFonts[G::iDPIScale], szBuiltKeyName, false, Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	bool bMouseInBound = UTILS::MouseInRegion(m_vOffset.x + 165 - tSize.right, m_vOffset.y + 1, tSize.right, 13);
	if (UTILS::KeyPressed(VK_LBUTTON) && bMouseInBound && !bIsGettingKey)
	{
		bIsGettingKey = true;
		szSelectedName = szName;
		flTimeClicked = I::Globals->flCurrentTime;
	}

	if (bAllowMethod)
	{
		bool bSkip = false;

		if (UTILS::KeyPressed(VK_RBUTTON) && bMouseInBound && !bIsGettingKey)
		{
			uInfo.m_bOpen = !uInfo.m_bOpen, uInfo.m_szName = szName, uInfo.m_flAlpha = 0.0f;
			bSkip = true;
		}

		if (uInfo.m_bOpen && uInfo.m_szName == szName)
		{
			int iClickedInBounds = 0;

			for (int i = 0; i < vItems.size(); i++)
			{
				bool bMouseInComBound = UTILS::MouseInRegion(m_vOffset.x + 165 - tSize.right, m_vOffset.y + 13 + (i * 12), 75, 13);

				if (UTILS::KeyDown(VK_LBUTTON) && bMouseInComBound) {
					*pMethod = i;
					iClickedInBounds++;
				}
			}

			if ((UTILS::KeyPressed(VK_LBUTTON) && iClickedInBounds <= 0 && !bSkip) || m_bCloseBoxes)
				uInfo.m_bOpen = false;
		}

		if ((bMouseInBound || uInfo.m_bOpen) && uInfo.m_szName == szName)
		{
			uInfo.m_iPrevControl = KEYBIND;
			uInfo.m_iItem = *pMethod;
			uInfo.m_vItems.assign(vItems.begin(), vItems.end());
			uInfo.m_vItems = vItems;
			uInfo.m_vOffset = { m_vOffset.x + 165 - tSize.right, m_vOffset.y };
			uInfo.m_bCopyPaste = false;

			m_bDisableBound = true;
		}
	}

	m_iLastControl = KEYBIND;
}

void GUI::CONTROLS::KeybindFocus()
{
	if (uInfo.m_iPrevControl == KEYBIND)
	{
		if (uInfo.m_vItems.size() > 0)
		{
			if (uInfo.m_bOpen)
			{
				if (m_bCloseBoxes)
					uInfo.m_bOpen = false;

				uInfo.m_flAlpha += (1.0f / 0.2f) * I::Globals->flFrameTime;
				if (uInfo.m_flAlpha >= 0.9f)
					uInfo.m_flAlpha = 1.0f;

				D::Rect(uInfo.m_vOffset.x, uInfo.m_vOffset.y + 13, 75, (uInfo.m_vItems.size() * 12) + 1, Color(40, 40, 40, uInfo.m_flAlpha * 255));
				D::OutlinedRect(uInfo.m_vOffset.x - 1, uInfo.m_vOffset.y + 12, 77, (uInfo.m_vItems.size() * 12) + 3, Color(0, 0, 0, uInfo.m_flAlpha * 255));

				for (int i = 0; i < uInfo.m_vItems.size(); i++)
					D::String(uInfo.m_vOffset.x + 10, uInfo.m_vOffset.y + 15 + (i * 12), D::uSmallFonts[G::iDPIScale], uInfo.m_vItems.at(i), false, uInfo.m_iItem == i ? Color(CONTROLS::m_cDefaultMenuCol, uInfo.m_flAlpha * 255) : Color(150, 150, 150, uInfo.m_flAlpha * 255));
			}
		}
	}
}