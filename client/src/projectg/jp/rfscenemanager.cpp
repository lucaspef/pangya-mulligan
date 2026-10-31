// JP 983: unidade 006030c0–006044a0 — a RefreshTexture (textura da RF feita de um WOverlay) e o RFSceneManager (o gerente de
// cena que liga a biblioteca Refresh ao jogo: cria a RefreshInterface, o RefreshRenderer, o RFManager e a RefreshInputHandler,
// repassa mouse/teclado/desenho e cuida do cursor e do foco). O mapa da F3.7.1 cortava em 00603d90 (refreshtexture.cpp /
// rfscenemanager.cpp), mas é uma unidade só: os métodos do gerente estão dos dois lados do corte, o destrutor do
// WSingleton<RFSceneManager> (00603480, desenrolar do construtor 00604090) e o _Ref_count<RefreshInputHandler> (006037f0…)
// estão antes dele, e não há inicializador entre os dois. 006044a0 em diante é o FrItemStorageNotice. Muitas funções curtas
// sem chamador (embutidas em todo lugar) estão no 983 como cópias fora de linha; as que têm corpo próprio vão aqui com o VA.
#include "refreshtexture.h"
#include "refreshinterface.h"
#include "refreshrenderer.h"
#include "refresh/rfscenemanager.h"
#include "rftooltiphelper.h"
#include "topicon.h"
#include "taskwidget.h"
#include "engine/woverlay.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "vc10_layout.h"

extern HWND g_hWnd983; // 00f0245c
extern const float g_fRFScreenW983; // 00d05628 (= 600: a altura; nomes trocados, ver B129)
extern const float g_fRFScreenH983; // 00d0562c (= 800: a largura)
extern const WVector g_vZero983_f041c4; // 00f041c4

// 00f02340 = g_pFresh. D865: o +0x40 do VC10 é Fresh::manager e as marcas de foco da RF são campos do FrWndManager
// (0xa9/0xaa/0xac do 983); o FrElementDoc do Fresh tem STL, então os offsets crus andam no VS 2022
struct sF02340Rf983;
extern sF02340Rf983* g_pF02340Rf983; // 00f02340
C_ASSERT_VC10(offsetof(Fresh, manager) == 0x40);

// ---------------------------------------------------------------------------------------------------------------------
// RefreshTexture

// 006030c0
unsigned long RefreshTexture::GetWidth()
{
	return (unsigned long)m_info.m_src[2];
}

// 006030f0
unsigned long RefreshTexture::GetHeight()
{
	return (unsigned long)m_info.m_src[3];
}

// 00603160
RefreshTexture::RefreshTexture(WOverlay* pOverlay, int owner)
{
	if (pOverlay)
	{
		m_info.m_src[0] = 0.0f;
		m_info.m_src[1] = 0.0f;
		m_info.m_owner = owner;
		m_info.m_pOverlay = pOverlay;
		m_info.m_src[2] = (float)pOverlay->GetWidth();
		m_info.m_src[3] = (float)pOverlay->GetHeight();
	}
}

// 006031e0
RefreshTexture::~RefreshTexture()
{
}

// ---------------------------------------------------------------------------------------------------------------------
// cópias fora de linha de embutidas de outras classes (o 983 as tem aqui, sem chamador)

// 006034a0
bool HasMouseCursor983()
{
	return g_pMouseCursor983 != NULL;
}

// 006037a0: a camada da janela não é a que a tela do jogo deixa para a RF
bool __fastcall IsOtherLayer983(RFWnd* pDesktop)
{
	FrWndManager* pScreen;
	if (!g_pF02340Rf983 || (pScreen = ((Fresh*)g_pF02340Rf983)->manager) == NULL)
		return false;
	if (static_cast<RFWndEx*>(pDesktop)->GetImageSet983() > 0)
		return static_cast<RFWndEx*>(pDesktop)->GetImageSet983() != FRMGR_FORMCOUNT983(pScreen);
	return FRMGR_FORMCOUNT983(pScreen) != 0;
}

// ---------------------------------------------------------------------------------------------------------------------
// RFSceneManager

// 00603290
void RFSceneManager::PreDraw983()
{
	if (m_bDraw && m_pManager)
		m_pManager->InjectPreDraw();
}

// 006032b0
void RFSceneManager::Draw983()
{
	if (m_bDraw && m_pManager)
		m_pManager->InjectDraw();
}

// 006032d0
bool RFSceneManager::InjectLButtonDown(float x, float y)
{
	if (m_pManager)
		return m_pManager->InjectLButtonDown(x, y);
	return false;
}

// 00603300
bool RFSceneManager::InjectLButtonUp(float x, float y)
{
	if (m_pManager)
		return m_pManager->InjectLButtonUp(x, y);
	return false;
}

// 00603330
bool RFSceneManager::InjectLButtonDblClick(float x, float y)
{
	if (m_pManager)
		return m_pManager->InjectLButtonDblClick(x, y);
	return false;
}

// 00603360
bool RFSceneManager::InjectRButtonDown(float x, float y)
{
	if (m_pManager)
		return m_pManager->InjectRButtonDown(x, y);
	return false;
}

// 00603390
bool RFSceneManager::InjectRButtonUp(float x, float y)
{
	if (m_pManager)
		return m_pManager->InjectRButtonUp(x, y);
	return false;
}

// 006033c0
bool RFSceneManager::InjectMouseMove(float x, float y)
{
	if (m_pManager)
		return m_pManager->InjectMouseMove(x, y);
	return false;
}

// 006033f0
bool RFSceneManager::InjectMouseWheel(int delta, float x, float y)
{
	if (m_pManager)
		return m_pManager->InjectMouseWheel(delta, x, y);
	return false;
}

// 00603420
bool RFSceneManager::InjectKeyDown(unsigned int key, unsigned int flags)
{
	if (m_pManager)
		return m_pManager->InjectKeyDown(key, flags);
	return false;
}

// 00603440
bool RFSceneManager::InjectKeyUp(unsigned int key, unsigned int flags)
{
	if (m_pManager)
		return m_pManager->InjectKeyUp(key, flags);
	return false;
}

// 006035b0
void RFSceneManager::SetResolution983(float w, float h)
{
	if (m_pManager)
	{
		m_pManager->GetRoot983()->SetSize(RFSize(w, h));
		m_pManager->GetRoot983()->UpdateLayout();
		m_pRenderer->ResetClip983();
		if (CIconManager::Instance())
			CIconManager::Instance()->UpdateLayout983();
	}
}

// 00603610
void RFSceneManager::SetRFFocus983(bool focus)
{
	FrWndManager* pScreen;
	if (g_pF02340Rf983 && (pScreen = ((Fresh*)g_pF02340Rf983)->manager) != NULL)
	{
		FRMGR_RFKEY983(pScreen) = focus;
		FRMGR_RFMOUSE983(((Fresh*)g_pF02340Rf983)->manager) = focus;
	}
}

// 00603640
void RFSceneManager::SetRFKeyFocus983(bool focus)
{
	FrWndManager* pScreen;
	if (g_pF02340Rf983 && (pScreen = ((Fresh*)g_pF02340Rf983)->manager) != NULL)
		FRMGR_RFKEY983(pScreen) = focus;
}

// 00603660
void RFSceneManager::SetRFMouseFocus983(bool focus)
{
	FrWndManager* pScreen;
	if (g_pF02340Rf983 && (pScreen = ((Fresh*)g_pF02340Rf983)->manager) != NULL)
		FRMGR_RFMOUSE983(pScreen) = focus;
}

// 00603680
bool RFSceneManager::HasRFFocus983()
{
	FrWndManager* pScreen;
	if (g_pF02340Rf983 && (pScreen = ((Fresh*)g_pF02340Rf983)->manager) != NULL && (FRMGR_RFKEY983(pScreen) || FRMGR_RFMOUSE983(pScreen)))
		return true;
	return false;
}

// 006036b0
bool RFSceneManager::HasRFKeyFocus983()
{
	FrWndManager* pScreen;
	if (g_pF02340Rf983 && (pScreen = ((Fresh*)g_pF02340Rf983)->manager) != NULL)
		return FRMGR_RFKEY983(pScreen);
	return false;
}

// 006036d0
bool RFSceneManager::HasRFMouseFocus983()
{
	FrWndManager* pScreen;
	if (g_pF02340Rf983 && (pScreen = ((Fresh*)g_pF02340Rf983)->manager) != NULL)
		return FRMGR_RFMOUSE983(pScreen);
	return false;
}

// 006036f0
bool RFSceneManager::IsCursorLocked983()
{
	if (g_pMouseCursor983)
		return g_pMouseCursor983->m_bLocked983;
	return false;
}

// 00603710
void RFSceneManager::SetCursor983(int cursor, bool lock)
{
	if (g_pMouseCursor983)
	{
		g_pMouseCursor983->m_bLocked983 = false;
		g_pMouseCursor983->SetCursor983(cursor, 1);
		g_pMouseCursor983->m_bLocked983 = lock;
		m_bCursorSet = lock;
	}
}

// 00603750
int RFSceneManager::GetCursor983()
{
	if (g_pMouseCursor983)
		return g_pMouseCursor983->GetCursor983();
	return 0;
}

// 00603850
void RFSceneManager::OnWndMsg983(unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SETFOCUS:
		if (Instance() && Instance()->m_pManager)
			Instance()->m_pManager->RestoreInputMethod();
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_LBUTTONUP:
		if (m_pInput)
			m_pInput->PushMessage983(msg, wParam, lParam);
		break;
	}
}

// 006038b0
WVector* RFSceneManager::GetPos983()
{
	if (m_pInput)
		return &m_pInput->m_mouse.pos;
	return (WVector*)&g_vZero983_f041c4;
}

// 006038d0
float RFSceneManager::GetWheel983()
{
	if (m_pInput)
		return m_pInput->m_mouse.wheel;
	return 0.0f;
}

// 006038e0
int RFSceneManager::GetButtons983()
{
	if (m_pInput)
		return m_pInput->GetButtons983();
	return 0;
}

// 006038f0
void RFSceneManager::SetInputBlocked983(bool blocked)
{
	if (Instance() && Instance()->m_pInput)
		Instance()->m_pInput->SetBlocked983(blocked);
}

// 00603920
bool RFSceneManager::IsInputBlocked983()
{
	if (Instance() && Instance()->m_pInput)
		return Instance()->m_pInput->m_bBlocked;
	return false;
}

// 00603940
bool RFSceneManager::IsTyping983()
{
	if (GetCursor983() == 0xc)
		return true;
	RefreshInputHandler* pInput;
	if (m_pManager && (pInput = m_pInput.get()) != NULL)
	{
		RFVector2 pos;
		pos.x = pInput->m_mouse.pos.x;
		pos.y = pInput->m_mouse.pos.y;
		RFWnd* pWnd = m_pManager->HitTest2(pos, true, true);
		if (pWnd && pWnd->GetType() == 0xb && m_pManager->GetFocusWnd983() == pWnd)
			return true;
	}
	return false;
}

// 006039a0: a RF só fica com a entrada quando não há janela modal dela aberta na camada da tela do jogo
void RFSceneManager::UpdateFocus983()
{
	RFWnd* pDesktop = (Instance() ? Instance()->m_pManager : NULL)->GetDesktop();
	Fresh* pGame = (Fresh*)g_pF02340Rf983;
	FrWndManager* pScreen;
	if (pDesktop)
	{
		if (!pGame)
			return;
		if ((pScreen = pGame->manager) != NULL)
		{
			bool other;
			if (static_cast<RFWndEx*>(pDesktop)->GetImageSet983() > 0)
				other = static_cast<RFWndEx*>(pDesktop)->GetImageSet983() != FRMGR_FORMCOUNT983(pScreen);
			else
				other = FRMGR_FORMCOUNT983(pScreen) != 0;
			if (other)
				return;
		}
	}
	if (pGame && (pScreen = pGame->manager) != NULL)
	{
		if (FRMGR_FORMCOUNT983(pScreen) && !(Instance() ? Instance()->m_pManager : NULL)->IsModalExist())
		{
			if (!IsInputBlocked983())
				SetInputBlocked983(true);
			if (HasRFFocus983())
				SetRFFocus983(false);
			return;
		}
		if (IsInputBlocked983())
			SetInputBlocked983(false);
	}
}

// 00603b90: cursor conforme a janela da RF debaixo do mouse
void RFSceneManager::UpdateCursor983()
{
	if (IsInputBlocked983())
		return;
	CMouseCursorJp983* pCursor = g_pMouseCursor983;
	if (!pCursor)
		return;
	RefreshInputHandler* pInput = m_pInput.get();
	if (!pInput)
		return;
	if (pCursor->m_bLocked983)
		return;

	RFVector2 pos;
	pos.x = pInput->m_mouse.pos.x;
	pos.y = pInput->m_mouse.pos.y;
	int cursor = 0;
	RFWnd* pWnd = m_pManager->HitTest2(pos, true, true);
	if (pWnd)
	{
		switch (pWnd->GetType())
		{
		case 3:
		case 4:
		case 5:
			cursor = pWnd->IsWindowVisible(false) ? 1 : 0;
			break;
		case 7:
			cursor = 0;
			break;
		case 0xb:
			if (pWnd->IsFocusable() || pWnd == m_pManager->GetFocusWnd983())
				cursor = 0xc;
			else
				cursor = 0;
			break;
		}
		if (pWnd->IsDragable())
			cursor = pWnd->GetType() != 3 ? 8 : 0;
	}
	if (GetCursor983() != cursor)
		SetCursor983(cursor, false);
}

// 00603ca0
void RFSceneManager::SetFocusAll983(bool focus)
{
	SetRFFocus983(focus);
	SetInputBlocked983(focus);
}

// 00603cc0
BOOL RFSceneManager::IsFocusBlocked983()
{
	if (HasRFFocus983() && IsInputBlocked983())
		return TRUE;
	return FALSE;
}

// 00603d90
RFSceneManager::~RFSceneManager()
{
	delete RFTooltipHelper::Instance983();
	if (m_pManager)
	{
		delete m_pManager;
		m_pManager = NULL;
	}
	if (m_pInterface)
	{
		delete m_pInterface;
		m_pInterface = NULL;
	}
	if (m_pRenderer)
	{
		delete m_pRenderer;
		m_pRenderer = NULL;
	}
	m_3c = 0;
}

// 00603e80
void RFSceneManager::Update983(float delta)
{
	if (m_pManager)
		m_pManager->InjectUpdate(delta);
	if (m_pInput)
		m_pInput->Update983(delta);
	UpdateCursor983();
	UpdateFocus983();
}

// 00604090
RFSceneManager::RFSceneManager()
{
	m_pManager = NULL;
	m_pInterface = NULL;
	m_pRenderer = NULL;
	m_pInput = std::tr1::shared_ptr<RefreshInputHandler>((RefreshInputHandler*)NULL);
	m_bDraw = true;
	m_41 = false;
	m_bCursorSet = false;
	m_3c = 0;
}

// 00604160
bool RFSceneManager::Init983()
{
	m_pInterface = new RefreshInterface;
	m_pRenderer = new RefreshRenderer;
	m_pManager = RFManager::New983(); // 983 new(0x98) + 00417490; D856: sizeof da lib
	m_pInput = std::tr1::shared_ptr<RefreshInputHandler>(new RefreshInputHandler);
	m_pInput->Initialize983();
	m_pManager->SetInterface(m_pInterface);
	m_pManager->SetRenderer(m_pRenderer);
	m_pManager->InitializeInputMethod(g_hWnd983, true);
	// 983 006042bf..006042d7: largura = [00d0562c] 800, altura = [00d05628] 600 (D627: era 600x800)
	SetResolution983(g_fRFScreenH983, g_fRFScreenW983);
	m_pManager->SetVisualEffect(true);
	new RFTooltipHelper;
	return true;
}

// 00604320: há janela da RF (que não seja o próprio desktop) debaixo de (x, y)?
bool RFSceneManager::IsOverWnd983(float x, float y)
{
	if (Instance() && Instance()->m_pManager)
	{
		std::wstring desktopName;
		std::wstring wndName;
		RFVector2 pos;
		pos.x = x;
		pos.y = y;
		RFWnd* pWnd = Instance()->m_pManager->HitTest2(pos, true, true);
		if (pWnd)
		{
			wndName = pWnd->GetName983();
			RFWnd* pDesktop = (Instance() ? Instance()->m_pManager : NULL)->GetDesktop();
			if (pDesktop)
				desktopName = pDesktop->GetName983();
			if (desktopName.compare(wndName) != 0)
				return true;
		}
	}
	return false;
}
