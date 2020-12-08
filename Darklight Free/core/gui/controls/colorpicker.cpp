#include "../gui.h"

void GUI::CONTROLS::Colorpicker(const std::string& szName, Color* pColor, uint8_t* pAlpha, const bool bAlpha, const int iColumn)
{
	if (!m_bInitialized)
		return;

	bool bMouseInBound = UTILS::MouseInRegion(m_vOffset.x + 143, m_vOffset.y, 25, 10);
	static auto bBuffer = false;
	bool bSkip = false;

	if (UTILS::KeyDown(VK_CONTROL) && UTILS::KeyDown(0x43) && bMouseInBound)
		m_cCopiedColor = *pColor;

	if (UTILS::KeyDown(VK_CONTROL) && UTILS::KeyDown(0x56) && bMouseInBound)
		*pColor = m_cCopiedColor;
		
	if (UTILS::KeyPressed(VK_LBUTTON) && bMouseInBound)
	{
		bBuffer ? (uInfo.m_bOpen = !uInfo.m_bOpen, uInfo.m_szName = szName, uInfo.m_flAlpha = 0.0f, bBuffer = false) : NULL;
		bSkip = true;
	}
	else if (bMouseInBound)
	{
		bBuffer = true;
	}

	auto bInColorBox = UTILS::MouseInRegion(uInfo.m_vOffset.x, uInfo.m_vOffset.y, bAlpha ? 165 : 150, 150);
	if (uInfo.m_bOpen && uInfo.m_szName == szName)
	{
		if ((UTILS::KeyDown(VK_LBUTTON) && !(bMouseInBound || bInColorBox)) && !bSkip || m_bCloseBoxes && !m_bHoldingSlider)
			uInfo.m_bOpen = false;
	}
		
	if (uInfo.m_bOpen && uInfo.m_szName == szName)
	{
		if (m_bCloseBoxes)
			uInfo.m_bOpen = false;
		
		uInfo.m_iPrevControl = COLORPICKER;
		uInfo.m_pColor = pColor;
		uInfo.m_iAlpha = pAlpha;
		uInfo.m_iColumn = iColumn;
		uInfo.m_vOffset.x = m_vOffset.x + 148;
		uInfo.m_vOffset.y = m_vOffset.y + 15;
		uInfo.m_bAlphaSlider = bAlpha;
		m_bDisableBound = true;
	}

	D::RectFade(m_vOffset.x + 148, m_vOffset.y + 3, 13, 6, *pColor, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vOffset.x + 147, m_vOffset.y + 2, 15, 8, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
		
	m_iLastControl = COLORPICKER;
}

std::unique_ptr<Color[]> pGradient = nullptr;

void ColorSpectrum(int x, int y, int w, int h)
{
	if (!pGradient)
	{
		pGradient = std::make_unique<Color[]>(w * h);
		float hue, sat = 0.99f, lum = 1.0f;
		for (int i = 0; i < w; i++)
		{
			for (int j = 0; j < h; j++)
			{
				*(Color*)(pGradient.get() + j + i * h) = Color::HSLToRGB(hue, sat, lum);

				hue += (1.f / w);
			}

			lum -= (1.f / h);
			hue = 0.f;
		}
	}
}

Color GetOfficialColor(int x, int y)
{
	return *(Color*)(pGradient.get() + x + y * 150);
}

void GUI::CONTROLS::ColorpickerFocus()
{
	if (uInfo.m_iPrevControl == COLORPICKER)
	{
		if (uInfo.m_bOpen)
		{
			if (m_bCloseBoxes)
				uInfo.m_bOpen = false;

			uInfo.m_flAlpha += (1.0f / 0.2f) * I::Globals->flFrameTime;
			if (uInfo.m_flAlpha >= 0.9f)
				uInfo.m_flAlpha = 1.0f;

			const Vector2D vecPosition = uInfo.m_vOffset;
			const Vector2D vecRelative = { UTILS::m_vMousePos.x - vecPosition.x, UTILS::m_vMousePos.y - vecPosition.y };
			static Vector2D vecSize = { 150, 150 };

			D::Rect(vecPosition.x - 4, vecPosition.y - 4, uInfo.m_bAlphaSlider ? 173 : 158, 158, Color(20, 20, 20, uInfo.m_flAlpha * 255));
			D::OutlinedRect(vecPosition.x - 4, vecPosition.y - 4, uInfo.m_bAlphaSlider ? 173 : 158, 158, Color(0, 0, 0, uInfo.m_flAlpha * 255));
			D::OutlinedRect(vecPosition.x - 3, vecPosition.y - 3, uInfo.m_bAlphaSlider ? 171 : 156, 156, Color(50, 50, 50, uInfo.m_flAlpha * 255));

			ColorSpectrum(vecPosition.x, vecPosition.y, vecSize.x, vecSize.y);

			colorPicker.Draw(vecPosition.x, vecPosition.y, Color(255, 255, 255, uInfo.m_flAlpha * 255));

			static Vector2D vecMousPos;

			if (UTILS::KeyDown(VK_LBUTTON) &&
				UTILS::MouseInRegion(vecPosition.x, vecPosition.y, vecSize.x, vecSize.y))
			{
				*uInfo.m_pColor =
					Color(
						GetOfficialColor(vecRelative.x, vecRelative.y).r(),
						GetOfficialColor(vecRelative.x, vecRelative.y).g(),
						GetOfficialColor(vecRelative.x, vecRelative.y).b(),
						*uInfo.m_iAlpha
					);

				vecMousPos = UTILS::m_vMousePos;
			}

			if (vecMousPos.x < vecPosition.x)
				vecMousPos.x = vecPosition.x;
			if (vecMousPos.y < vecPosition.y)
				vecMousPos.y = vecPosition.y;
			if (vecMousPos.x + 4 > vecPosition.x + vecSize.x)
				vecMousPos.x = vecPosition.x + vecSize.x - 4;
			if (vecMousPos.y + 4 > vecPosition.y + vecSize.y)
				vecMousPos.y = vecPosition.y + vecSize.y - 4;

			D::Rect(vecMousPos.x + 1, vecMousPos.y + 1, 2, 2, Color(124, 159, 222, uInfo.m_flAlpha * 255));
			D::OutlinedRect(vecMousPos.x, vecMousPos.y, 4, 4, Color(0, 0, 0, uInfo.m_flAlpha * 255));

			if (uInfo.m_bAlphaSlider)
			{
				// Create our Alpha slider position.
				const Vector2D vecAlphaPos = { vecPosition.x + vecSize.x + 3, vecPosition.y };
				const Vector2D vecAlphaSize = { 12, vecSize.y };

				if (UTILS::KeyDown(VK_LBUTTON) && UTILS::MouseInRegion(vecAlphaPos.x, vecAlphaPos.y, vecAlphaSize.x, vecAlphaSize.y))
				{
					*uInfo.m_iAlpha = 255 * (UTILS::m_vMousePos.y - vecAlphaPos.y) / vecAlphaSize.y;

					if (*uInfo.m_iAlpha < 2)
						*uInfo.m_iAlpha = 0;

					if (*uInfo.m_iAlpha > 252)
						*uInfo.m_iAlpha = 255;
				}

				const float flValueMod = (*uInfo.m_iAlpha / 255.0f) * vecAlphaSize.y;
				transparentBackground.Draw(vecAlphaPos.x, vecAlphaPos.y, Color(255, 255, 255, uInfo.m_flAlpha * 255));
				D::RectFade(vecAlphaPos.x, vecAlphaPos.y, vecAlphaSize.x, vecAlphaSize.y, Color(*uInfo.m_pColor, 0), Color(*uInfo.m_pColor, uInfo.m_flAlpha * 255), false, true);

				float flYPos = flValueMod;
				if (flValueMod >= vecAlphaSize.y - 3)
					flYPos = vecAlphaSize.y - 3;

				D::Rect(vecAlphaPos.x + 1, vecAlphaPos.y + flYPos + 1, vecAlphaSize.x - 2, 1, Color(124, 159, 222, uInfo.m_flAlpha * 255));
				D::OutlinedRect(vecAlphaPos.x, vecAlphaPos.y + flYPos, vecAlphaSize.x, 3, Color(0, 0, 0, uInfo.m_flAlpha * 255));
				D::OutlinedRect(vecAlphaPos.x, vecAlphaPos.y, vecAlphaSize.x, vecAlphaSize.y, Color(0, 0, 0, uInfo.m_flAlpha * 255));
			}
		}
	}
}