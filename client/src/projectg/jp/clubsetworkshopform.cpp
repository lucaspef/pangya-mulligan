// JP 983: unidade 004ee570–004f3110 (ClubSetWorkShopForm, base dos diálogos da oficina de tacos). Classe do 839/983, sem
// par no 645; ver include/jp/clubsetworkshopform.h. Sessão B76 (F3.7.9, 1º lote). Copiada da FrFormBase (formbase.cpp) e
// acertada pelo desmonte. Não escritos: os moldes de map/vector/shared_ptr que o compilador gera sozinho.
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "bitmap.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frframe.h"
#include "frbutton.h"
#include "frstatic.h"
#include "frtextbutton.h"
#include "frarea.h"
#include "frlistbox.h"
#include "fredit.h"
#include "frcombobox.h"
#include "frgaugebar.h"
#include "frviewer.h"
#include "frtabbutton.h"
#include "frgroupbox.h"
#include "lobbyjp.h"
#include "lua983.h"
class WView;
extern WView* g_pMainView983; // 00f02458
extern HWND g_hwnd; // 00f0245c
#include "clubsetworkshopform.h"

// visão da FrWnd: o nome por 00be76b0 (`lea eax, [ecx + 0x64]`, fora de linha no 983) e o tipo do item (+0xc8, privado lá)
class FrWndFormBaseJp : public FrWnd
{
public:
	const std::string& GetName983(); // 00be76b0
};
#define WNDGUITYPE983(p) ((p)->GetGuiType983()) // D871: era +0xc8 cru (VC10); no cl 14.44 a FrWnd e menor (std::string)

#define FORMGDI983() (FRMGR_GDI983(FRESHMGR983()))
#define VIEWW983() (*(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x19c)))
#define VIEWH983() (*(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x1a0)))

// 004ee580 (GetMessageMap); mapa 00e07948, 7 entradas (passo 0x20; os ponteiros vêm do inicializador 00ccac70, o dump só tem
// os nomes): FRCMD_INIT dos 5 controles -> 004ee9d0 (vtbl+0x98), clique do "cancel2" -> 00be5ee0 (vtbl+0x88 = OnCancel),
// OWNERDRAW do "image" -> 004ee9c0 (vtbl+0xa0 = DrawBitmaps983)
#define ON_FRESH_FORMCTRL983(name) \
	{ (FRESH_PFN)(void (FrCmdTarget::*)(IObject*))(void (ClubSetWorkShopForm::*)(IObject*)) \
		&ClubSetWorkShopForm::OnCtrlCreated983, name, FRCMD_INIT, FrFuncVI },

BEGIN_FRESH_MSGMAP(ClubSetWorkShopForm, FrForm)
ON_FRESH_FORMCTRL983("ok")
ON_FRESH_FORMCTRL983("cancel")
ON_FRESH_FORMCTRL983("cancel2")
ON_FRESH_FORMCTRL983("message")
ON_FRESH_FORMCTRL983("image")
ON_FRESH_VV("cancel2", FRCMD_LBUTTONUP, ClubSetWorkShopForm::OnCancel)
ON_FRESH_VV("image", FRCMD_OWNERDRAW, ClubSetWorkShopForm::DrawBitmaps983)
END_FRESH_MSGMAP()

// 004ee570
FrWnd* ClubSetWorkShopForm::OnCtrlCreated983(IObject* pObj)
{
	return AddCtrlByType983(pObj);
}

// 004ee590
void ClubSetWorkShopForm::OnProc(const float deltaTime)
{
	FrForm::OnProc(deltaTime);
}

// 004ee5b0: só repassa (as figuras saem pelo OWNERDRAW do "image", no mapa)
void ClubSetWorkShopForm::OnDraw()
{
	FrForm::OnDraw();
}

// 004ee5c0
void ClubSetWorkShopForm::OnOK()
{
	FrForm::OnOK();
}

// 004ee5d0
void ClubSetWorkShopForm::OnCancel()
{
	FrForm::OnCancel();
}

// 0049aae0
bool ClubSetWorkShopForm::Close(bool bFade)
{
	return FrForm::Close(bFade);
}

// 0049aaf0
bool ClubSetWorkShopForm::Close(eFormRet result, bool bFade)
{
	return FrForm::Close(result, bFade);
}

// 004eea80: centra na área cliente da janela, limitada ao tamanho da vista principal (800 × 600 sem ela)
void ClubSetWorkShopForm::SetSize983(float w, float h, bool bFlag)
{
	RECT rc;
	::GetClientRect(g_hwnd, &rc);
	int cw = rc.right - rc.left;
	int ch = rc.bottom - rc.top;
	WView* pView = g_pMainView983;
#define VW983() (pView ? *(float*)((char*)pView + VC10_OFS_BO(0x19c)) : 800.0f)
#define VH983() (pView ? *(float*)((char*)pView + VC10_OFS_BO(0x1a0)) : 600.0f)
	if ((float)cw > VW983())
		cw = (int)VW983();
	if ((float)ch > VH983())
		ch = (int)VH983();
	SetRect(WRect(cw / 2 - w * 0.5, ch / 2 - h * 0.5, w, h));
	m_b110 = bFlag;
}

// 004ef7f0: arruma os controles de nome fixo
void ClubSetWorkShopForm::Layout983()
{
	FrWnd* pWnd = GetCtrl983<FrWnd>("cancel2");
	if (pWnd)
		pWnd->SetRect(WRect(m_rect.w + m_rect.x - 53.0, m_rect.y, pWnd->GetRect().w, pWnd->GetRect().h));

	pWnd = GetCtrl983<FrWnd>("message");
	if (pWnd)
		pWnd->SetRect(WRect(pWnd->GetRect().x + 5.0, pWnd->GetRect().y - 5.0, m_rect.w - 17.0, pWnd->GetRect().h));

	FrWnd* pOk = GetCtrl983<FrWnd>("ok");
	FrWnd* pCancel = GetCtrl983<FrWnd>("cancel");
	const WRect& rcOk = pOk ? pOk->GetRect() : WRect(0.0f, 0.0f, 0.0f, 0.0f);
	float okW = rcOk.w;
	float okH = rcOk.h;
	const WRect& rcCancel = pCancel ? pCancel->GetRect() : WRect(0.0f, 0.0f, 0.0f, 0.0f);
	float cancelW = rcCancel.w;
	float cancelH = rcCancel.h;

	float x = m_rect.w * 0.5 - (okW + cancelW + m_rect.w * 0.1) * 0.5;
	if (!pCancel)
		x = x + 14.0;
	if (pOk)
	{
		pOk->SetRect(WRect(x + m_rect.x, m_rect.h + m_rect.y - 69.0, okW, okH));
		if (pCancel)
			pCancel->SetRect(WRect(pOk->GetRect().x + okW + m_rect.w * 0.1, pOk->GetRect().y, cancelW, cancelH));
	}
}

// 004efa40
bool ClubSetWorkShopForm::ShowCtrl983(ClubSetWorkShopForm* pForm, std::string name, bool bShow)
{
	if (pForm)
	{
		FrWnd* pWnd = pForm->GetCtrl983<FrWnd>(name);
		if (pWnd)
		{
			pWnd->SetVisible(bShow);
			return true;
		}
	}
	return false;
}

// 004efb00
bool ClubSetWorkShopForm::EnableCtrl983(ClubSetWorkShopForm* pForm, std::string name, bool bEnable)
{
	if (pForm)
	{
		FrWnd* pWnd = pForm->GetCtrl983<FrWnd>(name);
		if (pWnd)
		{
			pWnd->Enable(bEnable);
			return true;
		}
	}
	return false;
}

// 004efbc0
bool ClubSetWorkShopForm::SetCtrlToolTip983(std::string name, std::string text)
{
	FrWnd* pWnd = GetCtrl983<FrWnd>(name.c_str());
	if (pWnd)
	{
		pWnd->SetToolTipText(text.c_str());
		return true;
	}
	return false;
}

// 004efd20
bool ClubSetWorkShopForm::OffsetCtrl983(const std::string& name, float x, float y)
{
	FrWnd* pWnd = GetCtrl983<FrWnd>(name);
	if (pWnd)
	{
		pWnd->SetRect(WRect(pWnd->GetRect().x + x, pWnd->GetRect().y + y, pWnd->GetRect().w, pWnd->GetRect().h));
		return true;
	}
	return false;
}

// 004efdc0: a fila de figuras, centrada na largura do form, com a base alinhada embaixo, a partir do canto do "image"
void ClubSetWorkShopForm::DrawBitmaps983()
{
	if (m_bitmaps.size() <= 0)
		return;
	FrGraphicInterface* pGDI = FORMGDI983();
	FrWnd* pImage = GetCtrl983<FrWnd>("image");
	if (pGDI && pImage)
	{
		float x0 = m_rect.w * 0.5 - m_bmpTotalW * 0.5;
		for (unsigned i = 0; i < m_bitmaps.size(); i++)
		{
			sBitmapInfo* pInfo = m_bitmaps[i].get();
			Bitmap* pBmp = pInfo->pBmp;
			if (pBmp)
			{
				unsigned h = pBmp->Height();
				unsigned w = pBmp->Width();
				float y = m_bmpMaxH - h + pImage->GetRect().y + pInfo->y;
				float x = pImage->GetRect().x + x0 + pInfo->x;
				pGDI->DrawTexture(pBmp, WRect(x, y, w, h), 0xffffffff, 0);
			}
		}
	}
}

// 004f0110
bool ClubSetWorkShopForm::OnInit()
{
	Layout983();
	return FrForm::OnInit();
}

// 004f0120
bool ClubSetWorkShopForm::OffsetCtrlLua983(std::string name, lua_system::lua_table_helper* pTbl, std::string key)
{
	float x = ((LuaHelper983*)pTbl)->GetChild983(key.c_str()).GetByIndex(1);
	float y = ((LuaHelper983*)pTbl)->GetChild983(key.c_str()).GetByIndex(2);
	return OffsetCtrl983(name, x, y);
}

// 004f0230
bool ClubSetWorkShopForm::OffsetCtrlLua983(std::string name, lua_system::lua_table_helper* pTbl, std::string keyX,
	std::string keyY)
{
	float x = ((LuaHelper983*)pTbl)->Get<float>(keyX.c_str());
	float y = ((LuaHelper983*)pTbl)->Get<float>(keyY.c_str());
	return OffsetCtrl983(name, x, y);
}

// 004f1360
ClubSetWorkShopForm::~ClubSetWorkShopForm()
{
	m_ctrls.clear();
	m_bitmaps.clear();
	m_bmpTotalW = 0.0f;
	m_bmpMaxH = 0.0f;
	m_bmpNextX = 0.0f;
}

// 004f1470: no fim da fila; com figura mais alta, desce o "message" e aumenta o form
ClubSetWorkShopForm::BitmapInfoPtr ClubSetWorkShopForm::AddBitmap983(Bitmap* pBmp)
{
	if (!pBmp)
		return BitmapInfoPtr();
	m_bmpTotalW = (float)pBmp->Width() + 10.0 + m_bmpTotalW;
	if (m_bmpMaxH < (float)pBmp->Height())
	{
		m_bmpMaxH = (float)pBmp->Height();
		float h = 0.0f;
		float bottom = 0.0f;
		FrWnd* pImage = GetCtrl983<FrWnd>("image");
		if (pImage)
		{
			h = m_bmpMaxH + 10.0 + 0.0;
			pImage->SetRect(WRect(pImage->GetRect().x, pImage->GetRect().y + 50.0, pImage->GetRect().w, h));
			bottom = pImage->GetRect().y + h;
			h = h + 50.0;
		}
		FrWnd* pMsg = GetCtrl983<FrWnd>("message");
		if (pMsg)
		{
			pMsg->SetRect(WRect(pMsg->GetRect().x, bottom + 10.0, pMsg->GetRect().w, pMsg->GetRect().h));
			h = pMsg->GetRect().h + h;
		}
		h = h + 70.0;
		if (h > m_rect.h && !m_b110)
			SetSize983(m_rect.w, h, false);
	}
	BitmapInfoPtr pInfo(new sBitmapInfo);
	pInfo->pBmp = pBmp;
	pInfo->x = m_bmpNextX;
	pInfo->y = 0.0f;
	m_bmpNextX = (float)pBmp->Width() + 10.0 + m_bmpNextX;
	m_bitmaps.push_back(pInfo);
	return pInfo;
}

// 004f1770
ClubSetWorkShopForm::BitmapInfoPtr ClubSetWorkShopForm::AddOverlayBitmap983(Bitmap* pBase, Bitmap* pBmp,
	const WPoint& offset)
{
	if (!pBase || !pBmp)
		return BitmapInfoPtr();
	BitmapInfoPtr* pFound = NULL;
	for (unsigned i = 0; i < m_bitmaps.size(); i++)
	{
		if (m_bitmaps[i]->pBmp == pBase)
		{
			pFound = &m_bitmaps[i];
			break;
		}
	}
	if (!pFound)
		return BitmapInfoPtr();
	BitmapInfoPtr pInfo(new sBitmapInfo);
	pInfo->pBmp = pBmp;
	pInfo->x = (*pFound)->x + offset.x;
	pInfo->y = (*pFound)->y + offset.y;
	pInfo->bOverlay = true;
	m_bitmaps.push_back(pInfo);
	return pInfo;
}

// 004f1a60
ClubSetWorkShopForm::ClubSetWorkShopForm()
{
	C_ASSERT_VC10(offsetof(ClubSetWorkShopForm, m_ctrls) == 0x114);
	C_ASSERT_VC10(offsetof(ClubSetWorkShopForm, m_bitmaps) == 0x124);
	C_ASSERT_VC10(offsetof(ClubSetWorkShopForm, m_bmpTotalW) == 0x134);
	m_ctrls.clear();
	m_bitmaps.clear();
	m_bmpTotalW = 0.0f;
	m_b110 = false;
	m_bmpMaxH = 0.0f;
	m_bmpNextX = 0.0f;
}

// 004f2e60
FrWnd* ClubSetWorkShopForm::AddCtrlByType983(IObject* pObj)
{
	FrWnd* pWnd = KINDOF_CAST983(FrWnd, pObj);
	if (pWnd)
	{
		const std::string& name = ((FrWndFormBaseJp*)pWnd)->GetName983();
		switch (WNDGUITYPE983(pWnd))
		{
		case GI_FORM:
			return AddCtrl983<FrForm>(name, pObj);
		case GI_FRAME:
			return AddCtrl983<FrFrame>(name, pObj);
		case GI_BUTTON:
			return AddCtrl983<FrButton>(name, pObj);
		case GI_STATIC:
			return AddCtrl983<FrStatic>(name, pObj);
		case GI_TEXTBUTTON:
			return AddCtrl983<FrTextButton>(name, pObj);
		case GI_AREA:
			return AddCtrl983<FrArea>(name, pObj);
		case GI_LISTBOX:
			return AddCtrl983<FrListBox>(name, pObj);
		case GI_EDIT:
			return AddCtrl983<FrEdit>(name, pObj);
		case GI_COMBOBOX:
			return AddCtrl983<FrComboBox>(name, pObj);
		case GI_GAUGEBAR:
			return AddCtrl983<FrGaugeBar>(name, pObj);
		case GI_GAUGEBAREX:
			return AddCtrl983<FrGaugeBarEx>(name, pObj);
		case GI_GAUGEBARIMAGE:
			return AddCtrl983<FrGaugeBarImage>(name, pObj);
		case GI_VIEWER:
			return AddCtrl983<FrViewer>(name, pObj);
		case GI_CONTEXTMENU:
			return AddCtrl983<FrWnd>(name, pObj);
		case GI_TABBUTTON:
			return AddCtrl983<FrTabButton>(name, pObj);
		case GI_GROUPBOX:
			return AddCtrl983<FrGroupBox>(name, pObj);
		case GI_COMBOCTLEX:
			return AddCtrl983<FrComboCtlEx>(name, pObj);
		}
	}
	return NULL;
}

IMPLEMENT_OBJECT(ClubSetWorkShopForm, FrForm)
