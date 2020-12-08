#include "../gui.h"

void GUI::CONTROLS::BeginGroupbox(const std::string& szName, const int iColumn)
{
	if (!m_bInitialized)
		return;

	m_iColumn = iColumn;

	// Help dynamically position groupboxes.
	m_vOldGroupboxPos.x = m_iColumn ? 363 : 145;

	// Offsets into the groupbox for controls.
	m_vOffset.x = m_vPos.x + 37 + m_vOldGroupboxPos.x;
	m_vOffset.y = m_vPos.y + m_vOldGroupboxPos.y + 7;

	m_szGroupboxName = szName;
}
void GUI::CONTROLS::EndGroupbox()
{
	if (!m_bInitialized)
		return;

	auto flOffsetFromLeft = m_vPos.x + 21;
	auto flOffsetFromTop = m_vPos.y + m_vOldGroupboxPos.y - 3;
	auto flOffsetFromLastControl = m_vOffset.y - (flOffsetFromTop + 16);

	D::OutlinedRect(flOffsetFromLeft + m_vOldGroupboxPos.x - 1,
		flOffsetFromTop - 1,
		202,
		flOffsetFromLastControl + 27,
		Color(0, 0, 0, CONTROLS::m_flMenuAlpha));

	D::OutlinedRect(flOffsetFromLeft + m_vOldGroupboxPos.x,
		flOffsetFromTop,
		200,
		flOffsetFromLastControl + 25,
		Color(50, 50, 50, CONTROLS::m_flMenuAlpha));

	D::Rect(flOffsetFromLeft + m_vOldGroupboxPos.x + 101 - D::GetTextSize(D::uSmallFonts[G::iDPIScale], m_szGroupboxName.c_str()).right,
		flOffsetFromTop - 1,
		D::GetTextSize(D::uSmallFonts[G::iDPIScale], m_szGroupboxName.c_str()).right * 2,
		3,
		Color(20, 20, 20, CONTROLS::m_flMenuAlpha));

	D::String(flOffsetFromLeft + m_vOldGroupboxPos.x + 101,
		flOffsetFromTop - 5,
		D::uSmallFonts[G::iDPIScale], m_szGroupboxName.c_str(),
		4,
		Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	// Add onto the bottom of the last groupbox to 
	// evenly space new groupboxs from old ones.
	m_vOldGroupboxPos.y += flOffsetFromLastControl + 45;
}