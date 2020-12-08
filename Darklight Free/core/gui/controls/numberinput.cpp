#include "../gui.h"

const char* szNumbers[254] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, ("0"), ("1"), ("2"), ("3"), ("4"), ("5"), ("6"), ("7"), ("8"), ("9"), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, ("0"), ("1"), ("2"), ("3"), ("4"), ("5"), ("6"),
("7"), ("8"), ("9"), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

void GUI::CONTROLS::NumberField(const std::string& szName, std::string* pValue)
{
	if (!m_bInitialized)
		return;

	static bool bTakingText = false;
	static float flNextBlink = 0.0f;
	static std::string szCurrentItem;

	D::String(m_vOffset.x - 10, m_vOffset.y, D::uSmallFonts[G::iDPIScale], szName, false, Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	D::Rect(m_vOffset.x - 4, m_vOffset.y + 12, 173, 13, Color(40, 40, 40, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vOffset.x - 5, m_vOffset.y + 11, 175, 15, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));

	static auto bBuffer = false, bBuffer2 = false;

	auto bInBound = UTILS::MouseInRegion(m_vOffset.x + 20, m_vOffset.y + 16, 150, 18);

	if (!m_bDisableBound) {
		bool bSkip = false;
		if (UTILS::KeyPressed(VK_LBUTTON) && bInBound)
		{
			bBuffer ? (bTakingText = !bTakingText, szCurrentItem = szName, bBuffer = false) : NULL;
			bSkip = true;
		}
		else if (bInBound)
		{
			bBuffer = true;
		}

		if (bTakingText && szCurrentItem == szName)
		{
			if (UTILS::KeyPressed(VK_LBUTTON) && !bInBound)
				bTakingText = false;

			std::string szString = *pValue;
			for (int i = 0; i < 255; i++)
			{
				if (UTILS::KeyPressed(i))
				{
					if (i == VK_ESCAPE || i == VK_RETURN || i == VK_INSERT || i == VK_DELETE)
					{
						bTakingText = false;
						continue;
					}

					if (strlen(szString.c_str()) != 0)
					{
						if (GetAsyncKeyState(VK_BACK))
							*pValue = szString.substr(0, strlen(szString.c_str()) - 1);
					}

					if (strlen(szString.c_str()) < 30 && i != NULL &&
						szNumbers[i] != nullptr)
					{
						*pValue = szString + szNumbers[i];
						continue;
					}
				}
			}
		}
	}

	std::string szToRender = *pValue;
	if (GetTickCount64() >= flNextBlink)
		flNextBlink = GetTickCount64() + 800;
	if (bTakingText && szCurrentItem == szName && GetTickCount64() > (flNextBlink - 400))
		szToRender += "_";

	G::bTakingInput = bTakingText;

	D::String(m_vOffset.x + 5, m_vOffset.y + 14, D::uSmallFonts[G::iDPIScale], szToRender, false, bTakingText && szCurrentItem == szName ? Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha) : Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	m_iLastControl = NUMBERFIELD;
	m_vOffset.y += 30;
}