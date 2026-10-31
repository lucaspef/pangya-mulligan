#pragma once
#include "refresh/rfrenderer.h"
#include "engine/wtypes.h"

class WOverlay;

// JP 983: implementação do RFRenderer pelo jogo (RTTI RefreshRenderer, vtable 00d1307c, 0x2c bytes — `push 0x2c` do 00604160,
// guardado em +0x30 do RFSceneManager). Texturas = WOverlay do RFTextureManager desenhados no g_pMainView983; recorte em dois
// retângulos (+0x08 o da RF, +0x18 o da tela) e no FrGraphicInterface do texto. Unidade 005f5340–005f5df0 (refreshrenderer.cpp).
class RefreshRenderer : public RFRenderer
{
public:
	RefreshRenderer(); // 005f53d0
	virtual ~RefreshRenderer(); // 005f5430 (destrutor de apagar 005f58d0)

	void ResetClip983(); // 005f5490 (chamada pelo RFSceneManager 006035b0)

	virtual RFTexture* CreateTexture(const std::wstring& name); // 005f5c50
	virtual void DeleteTexture(RFTexture* texture); // 005f5340
	virtual void Draw(RFTexture* texture, const RFVector2& pos, const RFSize& size, float alpha, int a5, int a6); // 005f5900
	virtual void Draw(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10); // 005f5360
	virtual void Draw(int a1, int a2, int a3); // 005f5370
	virtual void PushClip983(const RFRect& rect); // 005f5950
	virtual void PopClip983(); // 005f59d0
	virtual void FillRectangle(unsigned long color, float x, float y, float w, float h, int a6); // 005f5a80

private:
	void Init983()
	{
		m_owner = 0;
		m_clip = g_rectZero983;
		m_screenClip = g_rectZero983;
		m_clipDepth = 0;
	}

public:
	static const WRect g_rectZero983; // 00f14668 (o g_lobbyRectZero983 da frente A)

	int m_owner; // +0x04: repassado a cada RefreshTexture (+4)
	WRect m_clip; // +0x08: recorte pedido pela RF (PushClip983)
	WRect m_screenClip; // +0x18: recorte da tela inteira
	int m_clipDepth; // +0x28: PushClip983 − PopClip983
};

// desenho solto (__fastcall do /Gr; 005f5560 também é chamada pela RFModalRenderer 005e58f0)
void DrawBox983(const _WRECT& rect, unsigned long color); // 005f5500
void DrawBoxAt983(const WPoint& pos, const WSize& size, unsigned long color); // 005f5560
class Bitmap;
void DrawBitmap983(const Bitmap* pBitmap, const WPoint& pos, const WSize& size, float alpha); // 005f55c0
void DrawOverlay983(WOverlay* pOverlay, const WPoint& pos, const WSize& size, float alpha, bool bScreenClip); // 005f56f0
void DrawTextureHandle983(int handle, const WPoint& pos, const WSize& size, float alpha); // 005f5810
void DrawNamedOverlay983(const char* name, const WPoint& pos, const WSize& size, float alpha); // 005f5b50
