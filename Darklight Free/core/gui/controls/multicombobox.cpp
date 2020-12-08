#include "../gui.h"

void GUI::CONTROLS::Multibox(const std::string& szName, const std::vector<std::string> vecItems, std::deque<bool>* pValue)
{
	if (!m_bInitialized)
		return;

	std::deque<bool> vecValue = *pValue;

	D::String(m_vOffset.x - 10, m_vOffset.y, D::uSmallFonts[G::iDPIScale], szName, false, Color(150, 150, 150, CONTROLS::m_flMenuAlpha));

	D::Rect(m_vOffset.x - 4, m_vOffset.y + 12, 173, 13, Color(40, 40, 40, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vOffset.x - 5, m_vOffset.y + 11, 175, 15, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));

	auto bInBound = UTILS::MouseInRegion(m_vOffset.x - 5, m_vOffset.y + 11, 175, 15);
	static auto bBuffer = false, bBuffer2 = false;

	if (!m_bDisableBound && !m_bHoldingSlider) {
		bool bSkip = false;
		if (UTILS::KeyPressed(VK_LBUTTON) && bInBound)
		{
			bBuffer ? (uInfo.m_bOpen = !uInfo.m_bOpen, uInfo.m_szName = szName, uInfo.m_flAlpha = 0.0f, bBuffer = false) : NULL;
			bSkip = true;
		}
		else if (bInBound)
		{
			bBuffer = true;
		}

		if (uInfo.m_bOpen && uInfo.m_szName == szName)
		{
			int iClickedInBounds = 0;

			for (int i = 0; i < vecItems.size(); i++)
			{
				bool bMouseInComBound = UTILS::MouseInRegion(m_vOffset.x - 4, m_vOffset.y + 28 + (i * 12), 175, 13);

				if (UTILS::KeyDown(VK_LBUTTON) && bMouseInComBound) {
					bBuffer2 ? (vecValue[i] = !vecValue[i], bBuffer2 = false) : NULL;
					iClickedInBounds++;
				}
				else if (bMouseInComBound) {
					bBuffer2 = true;
				}
			}

			*pValue = vecValue;

			if ((UTILS::KeyPressed(VK_LBUTTON) && iClickedInBounds <= 0 && !bSkip) || m_bCloseBoxes)
				uInfo.m_bOpen = false;
		}

		if ((bInBound || uInfo.m_bOpen) && uInfo.m_szName == szName)
		{
			uInfo.m_iPrevControl = MULTIBOX;
			uInfo.m_bItems = pValue;
			uInfo.m_vItems.assign(vecItems.begin(), vecItems.end());
			uInfo.m_vItems = vecItems;
			uInfo.m_vOffset = { m_vOffset.x, m_vOffset.y + 15 };
			uInfo.m_bCopyPaste = false;

			m_bDisableBound = true;
		}
	}

	for (auto i = 5; i >= 2; --i) {
		auto iOffset = 5 - i;

		auto bIsOpen = uInfo.m_bOpen && uInfo.m_szName == szName;
		RECT rDraw = {
			m_vOffset.x + 155 + iOffset,
			bIsOpen ? m_vOffset.y + 18 - iOffset : m_vOffset.y + 18 + iOffset,
			m_vOffset.x + 155 + iOffset + std::clamp(i - iOffset, 0, 5),
			bIsOpen ? m_vOffset.y + 18 - iOffset : m_vOffset.y + 18 + iOffset,
		};

		D::Line(rDraw.left, rDraw.top, rDraw.right, rDraw.bottom, Color(115, 115, 115, CONTROLS::m_flMenuAlpha));
	}

	std::string ss;
	for (auto n = 0; n < vecItems.size(); ++n) {
		auto format_len = ss.length() < 30;
		if ((vecValue[n] && format_len)) {
			if (ss.length() > 0)
				ss += (",  ");

			ss += (vecItems[n]);
		}
		else if (!format_len) {
			ss.resize(30);
			ss += ("..");
			break;
		}
	}

	if (!ss.length())
		ss += "...";

	D::String(m_vOffset.x + 5, m_vOffset.y + 14, D::uSmallFonts[G::iDPIScale], ss, false, Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha));

	m_iLastControl = MULTIBOX;
	m_vOffset.y += 30;
}

void GUI::CONTROLS::MultiboxFocus()
{
	if (uInfo.m_iPrevControl == MULTIBOX)
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

				D::Rect(uInfo.m_vOffset.x - 4, uInfo.m_vOffset.y + 13, 173, (uInfo.m_vItems.size() * 12) + 1, Color(40, 40, 40, uInfo.m_flAlpha * 255));

				for (int i = 0; i < uInfo.m_vItems.size(); i++)
				{
					const std::deque<bool> vecValue = *uInfo.m_bItems;
					D::String(uInfo.m_vOffset.x + 6, uInfo.m_vOffset.y + 15 + (i * 12), D::uSmallFonts[G::iDPIScale], uInfo.m_vItems.at(i), false, vecValue[i] ? Color(CONTROLS::m_cDefaultMenuCol, uInfo.m_flAlpha * 255) : Color(150, 150, 150, uInfo.m_flAlpha * 255));
				}

				D::OutlinedRect(uInfo.m_vOffset.x - 5, uInfo.m_vOffset.y + 12, 175, (uInfo.m_vItems.size() * 12) + 3, Color(0, 0, 0, uInfo.m_flAlpha * 255));
			}
	}
	}
}