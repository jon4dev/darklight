#include "../gui.h"

void GUI::CONTROLS::Button(const std::string& szName, const std::function<void(bool&)>& func)
{
	static bool bHolder = false;

	const std::vector<std::string> vSplit = UTILS::SplitStr(szName.c_str(), '#');
	const auto bInBound = UTILS::MouseInRegion(m_vOffset.x - 5, m_vOffset.y, 175, 15);
	const auto bActive = m_uBlocking == UTILS::Hash(szName.data());

	if (!m_bDisableBound) {
		if (!bActive && bInBound && UTILS::KeyPressed(VK_LBUTTON))
			m_uBlocking = UTILS::Hash(szName.data());
		else if (bActive && !UTILS::KeyDown(VK_LBUTTON)) {
			m_uBlocking = 0;
			func(bHolder);
		}
	}

	D::Rect(m_vOffset.x - 4, m_vOffset.y, 173, 13, bActive ? Color(100, 100, 100, CONTROLS::m_flMenuAlpha) : Color(40, 40, 40, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vOffset.x - 5, m_vOffset.y - 1, 175, 15, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
	D::String(m_vOffset.x - 4 + (175 / 2), m_vOffset.y + 6, D::uSmallFonts[G::iDPIScale], szName, 1, Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha));

	m_vOffset.y += 18;

	m_iLastControl = BUTTON;
}