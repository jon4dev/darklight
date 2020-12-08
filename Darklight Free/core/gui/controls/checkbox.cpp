#include "../gui.h"

void GUI::CONTROLS::Checkbox(const std::string& szName, bool* pValue)
{
	if (!m_bInitialized)
		return;

	const std::vector<std::string> vSplit = UTILS::SplitStr(szName.c_str(), '#');

	const auto bInBound = UTILS::MouseInRegion(m_vOffset.x + 168, m_vOffset.y + 1, 10, 10);
	const auto bActive = m_uBlocking == UTILS::Hash(szName.data());

	if (!m_bDisableBound) {
		if (!bActive && bInBound && UTILS::KeyPressed(VK_LBUTTON))
			m_uBlocking = UTILS::Hash(szName.data());
		else if (bActive && !UTILS::KeyDown(VK_LBUTTON)) {
			m_uBlocking = 0;
			*pValue = !*pValue;
		}
	}

	D::RectFade(m_vOffset.x + 170, m_vOffset.y + 3, 6, 6, *pValue ? Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha) : Color(75, 75, 75, CONTROLS::m_flMenuAlpha), Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vOffset.x + 169, m_vOffset.y + 2, 8, 8, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
	D::String(m_vOffset.x - 10, m_vOffset.y, D::uSmallFonts[G::iDPIScale], vSplit[0], false, *pValue ? Color(150, 150, 150, CONTROLS::m_flMenuAlpha) : Color(100, 100, 100, CONTROLS::m_flMenuAlpha));

	m_vOffset.y += 14;

	m_iLastControl = CHECKBOX;
}