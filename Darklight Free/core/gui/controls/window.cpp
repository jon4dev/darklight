#include "../gui.h"
#include <algorithm>

void GUI::CONTROLS::BeginWindow(const Vector2D vPos, const Vector2D vSize)
{
	if (!m_bInitialized)
	{
		m_vSize = vSize;
		m_vPos = vPos;
		m_iSelectedTab = 0;
		m_vTabs = { _("AIMBOT"), _("VISUALS"), _("MISC") };

		darklightLogo = CTextureHolder(CTextureHolder::DecodeResourceImage(IDB_PNG1, 184, 184), 184, 184);
		colorPicker = CTextureHolder(CTextureHolder::DecodeResourceImage(IDB_PNG2, 150, 150), 150, 150);
		transparentBackground = CTextureHolder(CTextureHolder::DecodeResourceImage(IDB_PNG3, 12, 150), 12, 150);

		m_bInitialized = !m_bInitialized;
	}

	CONTROLS::m_cDefaultMenuCol = C::Get<bool>(Vars.bCustomMenuCol) ? CONTROLS::m_cMenuColor : CONTROLS::m_cDefaultMenuColBackup;

	auto vecPos = m_vPos = Vector2D((float)C::Get<int>(Vars.iMenuX), (float)C::Get<int>(Vars.iMenuY));

	static bool bDrag = false;
	static Vector2D vecLastMousePos;
	static Vector2D vecLastMenuPos;

	if (UTILS::KeyDown(VK_LBUTTON) && !g_Visuals.m_bGrabbingSpectatorList)
	{
		if (!bDrag && UTILS::MouseInRegion(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y) &&
			!UTILS::MouseInRegion(m_vPos.x, m_vPos.y + 20, m_vSize.x, m_vSize.y - 20))
		{
			bDrag = true;
			vecLastMousePos = UTILS::m_vMousePos;
			vecLastMenuPos = m_vPos;
		}

		if (bDrag)
		{
			m_bDisableBound = true;
			m_bCloseBoxes = true;

			C::Get<int>(Vars.iMenuX) = vecLastMenuPos.x + (UTILS::m_vMousePos.x - vecLastMousePos.x);
			C::Get<int>(Vars.iMenuY) = vecLastMenuPos.y + (UTILS::m_vMousePos.y - vecLastMousePos.y);
		}
	}
	else
		bDrag = false;

	m_bGrabbingCheatGUI = bDrag;

	if (C::Get<int>(Vars.iMenuX) < 0)
		C::Get<int>(Vars.iMenuX) = 0;

	if (C::Get<int>(Vars.iMenuY) < 0)
		C::Get<int>(Vars.iMenuY) = 0;

	if ((C::Get<int>(Vars.iMenuX) + m_vSize.x) > G::vecDisplaySize.x)
		C::Get<int>(Vars.iMenuX) = G::vecDisplaySize.x - m_vSize.x;

	if ((C::Get<int>(Vars.iMenuY) + m_vSize.y) > G::vecDisplaySize.y)
		C::Get<int>(Vars.iMenuY) = G::vecDisplaySize.y - m_vSize.y;

	const std::string szName = _("DARKLIGHT  |  WELCOME  FREE  USER");

	D::Rect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y, Color(20, 20, 20, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 2, 21, Color(50, 50, 50, CONTROLS::m_flMenuAlpha));
	D::Rect(m_vPos.x, m_vPos.y + 23, m_vSize.x, 1, Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vPos.x + 1, m_vPos.y + 1, m_vSize.x - 2, m_vSize.y - 2, Color(50, 50, 50, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vPos.x, m_vPos.y, m_vSize.x, m_vSize.y, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));
	D::String(m_vPos.x + 6, m_vPos.y + 6, D::uSmallFonts[G::iDPIScale], szName, false, Color(147, 147, 155, CONTROLS::m_flMenuAlpha));
	darklightLogo.Draw(m_vPos.x - 2, m_vPos.y, Color(255, 255, 255, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vPos.x + 8, m_vPos.y + 32, 145, 130, Color(50, 50, 50, CONTROLS::m_flMenuAlpha));
	D::OutlinedRect(m_vPos.x + 7, m_vPos.y + 31, 147, 132, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));

	if (!m_vTabs.empty())
	{
		static int iCurrentTab = m_iSelectedTab;
		if (m_iSelectedTab != iCurrentTab)
		{
			m_bCloseBoxes = true;
			iCurrentTab = m_iSelectedTab;
		}

		D::OutlinedRect(m_vPos.x + 8, m_vPos.y + 170, 145, 24 * m_vTabs.size(), Color(50, 50, 50, CONTROLS::m_flMenuAlpha));
		D::OutlinedRect(m_vPos.x + 7, m_vPos.y + 169, 147, 24 * m_vTabs.size() + 2, Color(0, 0, 0, CONTROLS::m_flMenuAlpha));

		for (int i = 0; i < m_vTabs.size(); i++)
		{
			const RECT tabArea = { m_vPos.x + 16, m_vPos.y + 178 + (i * 20), 129, 16 };

			if (UTILS::MouseInRegion(tabArea.left, tabArea.top, tabArea.right, tabArea.bottom) &&
				UTILS::KeyPressed(VK_LBUTTON))
				m_iSelectedTab = i;

			D::RectFade(tabArea.left, tabArea.top, tabArea.right, tabArea.bottom, m_iSelectedTab == i ? Color(75, 75, 75, 100) : Color(35, 35, 35, 100), Color(0, 0, 0, 0), true);
			D::Line(tabArea.left, tabArea.top, tabArea.left, tabArea.top + tabArea.bottom, Color(CONTROLS::m_cDefaultMenuCol, CONTROLS::m_flMenuAlpha));
			D::String(tabArea.left + 10, tabArea.top + 3, D::uSmallFonts[G::iDPIScale], m_vTabs[i], false, m_iSelectedTab == i ? Color(150, 150, 150, CONTROLS::m_flMenuAlpha) : Color(85, 85, 85, CONTROLS::m_flMenuAlpha));
		}
	}
}
void GUI::CONTROLS::EndWindow()
{
	PushFocus();
	ResetVars();
}

void GUI::CONTROLS::ResetVars()
{
	m_iColumn = 0;

	m_iLastControl = NONE;
	m_bCloseBoxes = false;
	m_bDisableBound = false;
	m_vOffset = Vector2D();
	m_vGroupboxPos = Vector2D();
	m_vOldGroupboxPos = { 19, 35 };
	m_szGroupboxName = _("");

	if (!m_vecColorPickers.empty())
		m_vecColorPickers.clear();
}

void GUI::CONTROLS::PopGroupboxY()
{
	if (!m_bInitialized)
		return;

	m_vOldGroupboxPos.y = 35;
}

void GUI::CONTROLS::PushFocus()
{
	ComboboxFocus();
	MultiboxFocus();
	KeybindFocus();
	ColorpickerFocus();
}