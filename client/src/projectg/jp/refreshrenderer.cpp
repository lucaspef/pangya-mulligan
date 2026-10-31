// JP 983: unidade 005f5340–005f5df0 (RefreshRenderer, o RFRenderer do jogo, e as funções soltas de desenho). O mapa da F3.7.1
// ia até 005f69a0 e a punha em "lojas_pontos": 005f5df0 em diante (singletons do lobby, RFFormDesktopBase<RFWndEx>, iteradores
// de map, "Premium service_*" 005f6470) é o começo da rflobbydesktop.cpp. Inicializador 00cd21e0 = o s_rfEmptyText983 do
// rfinterface.h (não usado aqui).
#include "refreshrenderer.h"
#include "refreshtexture.h"
#include "refresh/rfinterface.h"
#include "refresh/rfscenemanager.h"
#include "refresh/rftextmanager.h"
#include "refresh/rftexturemanager.h"
#include "engine/woverlay.h"
#include "engine/bitmap.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "frwndmanager.h"

extern WView* g_pMainView983; // 00f02458
extern const WRect g_rectUnitW983; // 00e4296c (retângulo de origem padrão; o g_rectUnit983_e4296c das outras unidades)

// 00f02340 = g_pFresh: Fresh::manager->GetGDI983() (D865: era +0x40 -> +0x84 cru do VC10; anda no VS 2022)
struct sF02340View983;
extern sF02340View983* g_pF02340View983; // 00f02340

std::string WideToMultiByte983(const std::wstring& text); // 005e4d60 (atl.cpp)

// 005f5340
void RefreshRenderer::DeleteTexture(RFTexture* texture)
{
	if (texture)
		delete texture;
}

// slot 3: 005f5360 (jp.tsv à mão: três sobrecargas de Draw)
void RefreshRenderer::Draw(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10)
{
}

// slot 2: 005f5370
void RefreshRenderer::Draw(int a1, int a2, int a3)
{
}

// 005f53d0
RefreshRenderer::RefreshRenderer()
{
	Init983();
}

// 005f5430
RefreshRenderer::~RefreshRenderer()
{
	Init983();
}

// 005f5490
void RefreshRenderer::ResetClip983()
{
	RFSize size = RFSceneManager::GetManager983()->GetRoot983()->GetSize();
	WRect rect(0.0f, 0.0f, size.width, size.height);
	m_clip = rect;
	m_screenClip = rect;
}

// 005f5500
void DrawBox983(const _WRECT& rect, unsigned long color)
{
	WOverlay::DrawBox(g_pMainView983, WRect(rect), 0, color, 0.001f);
}

// 005f5560
void DrawBoxAt983(const WPoint& pos, const WSize& size, unsigned long color)
{
	WOverlay::DrawBox(g_pMainView983, WRect(pos.x, pos.y, size.w, size.h), 0, color, 0.001f);
}

// 005f55c0
void DrawBitmap983(const Bitmap* pBitmap, const WPoint& pos, const WSize& size, float alpha)
{
	if (g_pF02340View983 && FRMGR_GDI983(((Fresh*)g_pF02340View983)->manager))
	{
		WSize drawSize(size.w, size.h);
		if (drawSize.w == 0.0f && drawSize.h == 0.0f)
		{
			drawSize.w = (float)pBitmap->Width();
			drawSize.h = (float)pBitmap->Height();
		}
		WRect dst(pos.x, pos.y, drawSize.w, drawSize.h);
		WRect src(0.0f, 0.0f, pBitmap->Width(), pBitmap->Height());
		FRMGR_GDI983(((Fresh*)g_pF02340View983)->manager)->DrawTexture(pBitmap, src, dst, ((unsigned long)(alpha * 255.0) << 24) | 0xffffff, 0);
	}
}

// 005f56f0
void DrawOverlay983(WOverlay* pOverlay, const WPoint& pos, const WSize& size, float alpha, bool bScreenClip)
{
	if (pOverlay && g_pMainView983)
	{
		WSize drawSize(size.w, size.h);
		if (drawSize.w == 0.0f && drawSize.h == 0.0f)
		{
			drawSize.w = (float)pOverlay->GetWidth();
			drawSize.h = (float)pOverlay->GetHeight();
		}
		pOverlay->SetClippingArea(g_pMainView983,
			bScreenClip ? &RFSceneManager::GetRenderer983()->m_screenClip : &RFSceneManager::GetRenderer983()->m_clip);
		WRect dst(pos.x, pos.y, drawSize.w, drawSize.h);
		pOverlay->Render(g_pMainView983, drawSize.w < 4.0f ? RefreshRenderer::g_rectZero983 : g_rectUnitW983, dst, 0,
			FrALPHA(0xffffffff, alpha), 0.0f, 0);
	}
}

// 005f5810
void DrawTextureHandle983(int handle, const WPoint& pos, const WSize& size, float alpha)
{
	if (g_pMainView983)
	{
		WRect dst(pos.x, pos.y, size.w, size.h);
		g_pMainView983->Draw2DTexture(g_rectUnitW983, dst, handle, ((unsigned long)(alpha * 255.0) << 24) | 0xffffff, 0);
	}
}

// slot 4: 005f5900
void RefreshRenderer::Draw(RFTexture* texture, const RFVector2& pos, const RFSize& size, float alpha, int a5, int a6)
{
	if (texture && ((RefreshTexture*)texture)->m_info.m_pOverlay)
		DrawOverlay983(((RefreshTexture*)texture)->m_info.m_pOverlay, WPoint(pos.x, pos.y), WSize(size.width, size.height), alpha, false);
}

// 005f5950
void RefreshRenderer::PushClip983(const RFRect& rect)
{
	WRect clip(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
	m_clip = clip;
	m_screenClip = clip;
	if (RFTextManager::Instance())
		RFTextManager::Instance()->SetClippingArea983(&m_clip);
	++m_clipDepth;
}

// 005f59d0
void RefreshRenderer::PopClip983()
{
	if (m_clipDepth > 0)
	{
		RFSize size = RFSceneManager::GetManager983()->GetRoot983()->GetSize();
		WRect rect(0.0f, 0.0f, size.width, size.height);
		m_clip = rect;
		if (RFTextManager::Instance())
			RFTextManager::Instance()->SetClippingArea983(&rect);
		--m_clipDepth;
	}
}

// 005f5a80
void RefreshRenderer::FillRectangle(unsigned long color, float x, float y, float w, float h, int a6)
{
	if (g_pMainView983)
	{
		bool bClip;
		if (m_clip.w == 0.0f || m_clip.h == 0.0f)
			bClip = false;
		else
		{
			bClip = true;
			g_pMainView983->SetClippingArea(m_clip);
		}
		DrawBoxAt983(WPoint(x, y), WSize(w, h), color);
		if (bClip)
			g_pMainView983->SetClippingArea(WRect(0.0f, 0.0f, g_pMainView983->GetWidth(), g_pMainView983->GetHeight()));
	}
}

// 005f5b50
void DrawNamedOverlay983(const char* name, const WPoint& pos, const WSize& size, float alpha)
{
	if (RFTextureManager::Instance())
	{
		WOverlay* pOverlay = RFTextureManager::Instance()->Find983(name);
		if (pOverlay)
			DrawOverlay983(pOverlay, WPoint(pos.x, pos.y), WSize(size.w, size.h), alpha, true);
	}
}

// 005f5c50
RFTexture* RefreshRenderer::CreateTexture(const std::wstring& name)
{
	if (name.size() == 0 || !RFTextureManager::Instance())
		return NULL;
	std::string file = WideToMultiByte983(name.c_str());
	WOverlay* pOverlay = RFTextureManager::Instance()->Find983(file.c_str());
	if (pOverlay)
		return new RefreshTexture(pOverlay, m_owner);
	return NULL;
}
