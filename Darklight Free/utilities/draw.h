#pragma once
// used: std::function
#include <functional>

// used: winapi, directx, imgui, fmt includes
#include "../common.h"
// used: color
#include "../sdk/datatypes/color.h"
// used: vector
#include "../sdk/datatypes/vector.h"

#include "../sdk/interfaces/isurface.h"
#include "../global.h"

/*
 * DRAW
 * ready rendering framework
 */
namespace D
{
	inline unsigned long uVerdana[5];
	inline unsigned long uSmallFonts[5];
	inline unsigned long uVerdanaBig[5];
	inline unsigned long uMenuFont[5];
	inline unsigned long uTrebuchetMS[5];

	inline int ScaleDPI(int i)
	{
		return i * (1.0f + (0.25f * G::iDPIScale));
	}

	inline int ScaleDPI(int i, int x)
	{
		return i * (1.0f + (0.25f * x));
	}

	void Initialize();

	void OutlinedRect(int x, int y, int w, int h, Color col);
	void Line(int x, int y, int x1, int y1, Color col);
	void Rect(int x, int y, int w, int h, Color col);
	void String(int x, int y, Color col, unsigned long font, const char* text, ...);
	void String(int, int, unsigned long, std::string, int, Color);

	void RectFade(int x, int y, int x1, int y1, Color first, Color second, bool = false, bool = false);

	RECT GetTextSize(unsigned long font, const char* text);

	void TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color col);
	void DrawFilledTriangle(std::vector<Vector> points, Color colour);
	// Get
	/* create fonts, set styles etc */
	void Setup(IDirect3DDevice9* pDevice, unsigned int uFontFlags = 0x0);
	/* shutdown imgui */
	void Destroy();

	// Extra
	/* converts 3d game world space to screen space */
	bool WorldToScreen(const Vector& vecOrigin, Vector2D& vecScreen);

	// Values
	/* directx init state */
	inline bool	bInitialized = false;
}

//struct Vertice_t
//{
//	float x, y, z, rhw;
//	int c;
//};
//
//class CDirectRender
//{
//public:
//	CDirectRender(IDirect3DDevice9* device)
//	{
//		this->pDevice = device;
//		this->pVertexShader = pVertexShader;
//
//		device->GetViewport(&screenSize);
//	}
//
//	IDirect3DVertexShader9* pVertexShader;
//	IDirect3DDevice9* pDevice;
//	D3DVIEWPORT9 screenSize;
//
//	void SetRenderStates();
//
//	int GetTextWidth(const char*, LPD3DXFONT);
//	void Text(float, float, LPD3DXFONT, std::string, Color, bool = false, int = 0, Color = Color());
//
//	void Line(Vector2D, Vector2D, Color);
//	void LineInit(float, float, float, float, Color);
//
//	void Rect(Vector2D, Vector2D, Color);
//	void RectInit(float, float, float, float, Color);
//
//	void Outline(Vector2D, Vector2D, Color);
//	void OutlineInit(float, float, float, float, Color);
//
//	void GradientVertical(Vector2D, Vector2D, Color, Color);
//	void GradientVerticalInit(float, float, float, float, Color, Color);
//
//	void GradientHorizontal(Vector2D, Vector2D, Color, Color);
//	void GradientHorizontalInit(float, float, float, float, Color, Color);
//
//	inline void SetViewport(D3DVIEWPORT9 vp)
//	{
//		pDevice->SetViewport(&vp);
//	}
//
//	inline D3DVIEWPORT9 GetViewport()
//	{
//		D3DVIEWPORT9 vp;
//		pDevice->GetViewport(&vp);
//		return vp;
//	}
//
//	struct
//	{
//		LPD3DXFONT mainFont;
//		LPD3DXFONT secondFont;
//	} fonts;
//};
//
//extern CDirectRender* g_pDrawList;