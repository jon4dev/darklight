#include "../gui.h"

static int iSliderSize = 175;

void GUI::CONTROLS::Slider(const std::string& szName, int* pValue, int iMin, int iMax)
{
	if (!m_bInitialized)
		return;

	static bool bTakingInput = false, bBuffer = false;
	static std::string szCurrentName;

	D::String(m_vOffset.x - 10, m_vOffset.y, D::uSmallFonts[G::iDPIScale], szName, false, Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	const auto bInBound = UTILS::MouseInRegion(m_vOffset.x - 7, m_vOffset.y + 11, iSliderSize + 3, 4);

	const float flMinMaxDelta = iMax - iMin;

	if (!m_bDisableBound)
	{
		bool bSkip = false;
		if (UTILS::KeyPressed(VK_LBUTTON) && bInBound)
		{
			bBuffer ? (bTakingInput = !bTakingInput, szCurrentName = szName, bBuffer = false) : NULL;
			bSkip = true;
		}
		else if (bInBound)
		{
			bBuffer = true;
		}

		if (UTILS::KeyDown(VK_LBUTTON) && bTakingInput && szCurrentName == szName)
		{
			*pValue = iMin + flMinMaxDelta * (UTILS::m_vMousePos.x - (m_vOffset.x - 6)) / iSliderSize;

			if (*pValue < iMin)
				*pValue = iMin;
			if (*pValue > iMax)
				*pValue = iMax;
		}
		else if (!UTILS::KeyDown(VK_LBUTTON) && !bInBound && bTakingInput)
		{
			bTakingInput = false;
		}

		m_bHoldingSlider = bTakingInput;
	}

	const float flValueMinDelta = *pValue - iMin;
	const float flValueMod = (flValueMinDelta / flMinMaxDelta) * iSliderSize;

	D::Rect(m_vOffset.x - 5, m_vOffset.y + 11, iSliderSize - 1, 5, Color(40, 40, 40, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vOffset.x - 6, m_vOffset.y + 11, iSliderSize, 6, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
	D::RectFade(m_vOffset.x - 5, m_vOffset.y + 12, flValueMod, 4, Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha), Color(0, 0, 0, CONTROLS::m_flMenuAlpha));

	const std::string szValue = std::to_string(*pValue);
	const auto valueSize = D::GetTextSize(D::uSmallFonts[G::iDPIScale], szValue.c_str());
	auto flX = m_vOffset.x - 5 + flValueMod;
	if (flX > m_vOffset.x - 5 + iSliderSize - valueSize.right)
		flX = m_vOffset.x - 5 + iSliderSize - valueSize.right;

	D::String(flX, m_vOffset.y + 15, D::uSmallFonts[G::iDPIScale], std::to_string(*pValue), false, Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	m_vOffset.y += 23;

	m_iLastControl = SLIDER;
}
void GUI::CONTROLS::Slider(const std::string& szName, float* pValue, float flMin, float flMax)
{
	if (!m_bInitialized)
		return;

	static bool bTakingInput = false, bBuffer = false;
	static std::string szCurrentName;

	D::String(m_vOffset.x - 10, m_vOffset.y, D::uSmallFonts[G::iDPIScale], szName, false, Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	const auto bInBound = UTILS::MouseInRegion(m_vOffset.x - 7, m_vOffset.y + 11, iSliderSize + 3, 4);

	const float flMinMaxDelta = flMax - flMin;

	if (!m_bDisableBound)
	{
		bool bSkip = false;
		if (UTILS::KeyPressed(VK_LBUTTON) && bInBound)
		{
			bBuffer ? (bTakingInput = !bTakingInput, szCurrentName = szName, bBuffer = false) : NULL;
			bSkip = true;
		}
		else if (bInBound)
		{
			bBuffer = true;
		}

		if (UTILS::KeyDown(VK_LBUTTON) && bTakingInput && szCurrentName == szName)
		{
			*pValue = flMin + flMinMaxDelta * (UTILS::m_vMousePos.x - (m_vOffset.x - 6)) / iSliderSize;

			if (*pValue < flMin)
				*pValue = flMin;
			if (*pValue > flMax)
				*pValue = flMax;
		}
		else if (!UTILS::KeyDown(VK_LBUTTON) && !bInBound && bTakingInput)
		{
			bTakingInput = false;
		}

		m_bHoldingSlider = bTakingInput;
	}

	const float flValueMinDelta = *pValue - flMin;
	const float flValueMod = (flValueMinDelta / flMinMaxDelta) * iSliderSize;

	D::Rect(m_vOffset.x - 5, m_vOffset.y + 11, iSliderSize - 1, 5, Color(40, 40, 40, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vOffset.x - 6, m_vOffset.y + 11, iSliderSize, 6, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
	D::RectFade(m_vOffset.x - 5, m_vOffset.y + 12, flValueMod, 4, Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha), Color(0, 0, 0, CONTROLS::m_flMenuAlpha));

	const std::string szValue = std::to_string((int)*pValue);
	const auto valueSize = D::GetTextSize(D::uSmallFonts[G::iDPIScale], szValue.c_str());
	auto flX = m_vOffset.x - 5 + flValueMod;
	if (flX > m_vOffset.x - 5 + iSliderSize - valueSize.right)
		flX = m_vOffset.x - 5 + iSliderSize - valueSize.right;

	D::String(flX, m_vOffset.y + 15, D::uSmallFonts[G::iDPIScale], szValue, false, Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	m_vOffset.y += 23;

	m_iLastControl = SLIDER;
}