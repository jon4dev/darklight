#include "../gui.h"

void GUI::CONTROLS::Text(const std::string& szName, const bool bUseCustomCol)
{
	if (!m_bInitialized)
		return;

	D::String(m_vOffset.x - 10, m_vOffset.y, D::uSmallFonts[G::iDPIScale], szName, false, bUseCustomCol ? Color(255, 215, 0, CONTROLS::m_flMenuAlpha) : Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	m_vOffset.y += 14;

	m_iLastControl = TEXT;
}