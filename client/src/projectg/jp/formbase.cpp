// JP 983: unidade 004fb5f0–004ff080 (FrFormBase). Classe do 839/983, sem par no 645; ver include/jp/formbase.h.
// Sessão B54. Não escritas: 004fd3e0 e 004fd4f0 (sem chamador nem citação no exe; leem dois números de um texto e chamam
// MoveCtrl983) e os moldes de vector/shared_ptr que o compilador gera sozinho.
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "taskmanager.h"
#include "golftask.h"
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
class WView;
extern WView* g_pMainView983; // 00f02458
#include "formbase.h"

// bloco de 0x344 bytes que 004927b0 devolve por referência (objeto de 00e73f00) e a 004fba30 copia inteiro; só leio a
// largura e a altura (+0xc, +0x10). Destrutor fora de linha 00491b40. Dono não levantado (faixa 00491900–00492800).
struct sScreenCfg983
{
	~sScreenCfg983(); // 00491b40
	int m_00[3];
	int width; // +0xc
	int height; // +0x10
	int m_14[0xcc];
};
C_ASSERT(sizeof(sScreenCfg983) == 0x344);
sScreenCfg983::~sScreenCfg983() {} // 00491b40: so `ret` no 983
class CScreenCfgOwner983
{
public:
	const sScreenCfg983& Get983(); // 004927b0
};
extern CScreenCfgOwner983* g_pScreenCfgOwner983; // 00e73f00
extern const WRect g_rcZero983; // 00f14668

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

static inline float ViewWidth983()
{
	return g_pMainView983 ? VIEWW983() : 800.0f;
}

static inline float ViewHeight983()
{
	return g_pMainView983 ? VIEWH983() : 600.0f;
}

// 004fb5f0 (GetMessageMap); mapa 00e08570, 8 entradas (passo 0x20; os ponteiros vêm do inicializador 00ccb7b0, o dump só tem
// os nomes): FRCMD_INIT dos 6 controles -> 004fedd0, clique do "cancel2" -> 00be5ee0 (vtbl+0x88 = OnCancel 004fdc80),
// OWNERDRAW do "image" -> 004fb8b0 (vtbl+0xa4 = DrawBitmaps983 004fd050). Vazio, a derivada não guardava esses controles.
#define ON_FRESH_BASECTRL983(name) \
	{ (FRESH_PFN)(FRESH_PFN_VI)(void (FrCmdTarget::*)(IObject*))&FrFormBase::OnCtrlCreated983, name, FRCMD_INIT, FrFuncVI },

BEGIN_FRESH_MSGMAP(FrFormBase, FrForm)
ON_FRESH_BASECTRL983("ok")
ON_FRESH_BASECTRL983("cancel")
ON_FRESH_BASECTRL983("cancel2")
ON_FRESH_BASECTRL983("message")
ON_FRESH_BASECTRL983("image")
ON_FRESH_BASECTRL983("caption")
ON_FRESH_VV("cancel2", FRCMD_LBUTTONUP, FrFormBase::OnCancel)
ON_FRESH_VV("image", FRCMD_OWNERDRAW, FrFormBase::DrawBitmaps983)
END_FRESH_MSGMAP()

// 004fb600
bool FrFormBase::Open(FRESH_PFN_RESULT pFnResult, unsigned long flag)
{
	PreOpen983();
	return FrForm::Open(pFnResult, flag);
}

// 004fb640
void FrFormBase::OnProc(const float deltaTime)
{
	if (m_pSubForm)
		m_pSubForm->OnProc(deltaTime);
}

// 004fb670
void FrFormBase::SetSubForm983(FrFormBase* pForm)
{
	if (pForm)
		m_pSubForm = pForm;
}

// 004fb690
void FrFormBase::SetMessage(const char* msg, bool spaceAlign)
{
	FrForm::SetMessage(msg, spaceAlign);
}

// 004fba30: tamanho da tela pela configuração; fora da partida, limitado ao tamanho da vista principal
void FrFormBase::UpdateScreenSize983()
{
	sScreenCfg983 cfg = g_pScreenCfgOwner983->Get983();
	m_screenW = (float)cfg.width;
	m_screenH = (float)cfg.height;
	if (!IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		if (m_screenW > ViewWidth983())
			m_screenW = ViewWidth983();
		if (m_screenH > ViewHeight983())
			m_screenH = ViewHeight983();
	}
}

// 004fbd50
void FrFormBase::SetSize983(float w, float h, bool bFlag)
{
	UpdateScreenSize983();
	FrForm::SetRect(WRect(m_screenW * 0.5 - w * 0.5, m_screenH * 0.5 - h * 0.5, w, h));
	m_b148 = bFlag;
}

// 004fc7b0
bool FrFormBase::ShowCtrl983(FrFormBase* pForm, std::string name, bool bShow)
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

// 004fc870
bool FrFormBase::EnableCtrl983(FrFormBase* pForm, std::string name, bool bEnable)
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

// 004fc930
bool FrFormBase::FadeOutCtrl983(FrFormBase* pForm, std::string name, bool bOut)
{
	if (pForm)
	{
		FrWnd* pWnd = pForm->GetCtrl983<FrWnd>(name);
		if (pWnd)
		{
			pWnd->SetElementFadeOut(bOut);
			return true;
		}
	}
	return false;
}

// 004fc9f0
bool FrFormBase::SetCtrlToolTip983(std::string name, std::string text)
{
	FrWnd* pWnd = GetCtrl983<FrWnd>(name.c_str());
	if (pWnd)
	{
		pWnd->SetToolTipText(text.c_str());
		return true;
	}
	return false;
}

// 004fcb50: arruma os controles de nome fixo e ajusta a altura do form ao conteúdo.
// Lida de UM desmonte; a conta do `x` dos botões (o `- 0.5` final) é a parte menos segura.
void FrFormBase::Layout983()
{
	FrWnd* pWnd = GetCtrl983<FrWnd>("cancel2");
	if (pWnd)
		pWnd->SetRect(WRect(m_rect.w + m_rect.x - 53.0, m_rect.y, pWnd->GetRect().w, pWnd->GetRect().h));

	WRect rcCaption = g_rcZero983;
	WRect rcImage = g_rcZero983;
	WRect rcMsg;
	pWnd = GetCtrl983<FrWnd>("caption");
	if (pWnd)
		rcCaption = pWnd->GetRect();

	bool bNoBitmap = m_bitmaps.size() == 0;
	FrWnd* pImage = GetCtrl983<FrWnd>("image");
	if (pImage)
	{
		pImage->SetRect(WRect(m_rect.x + 3.0, m_rect.y + rcCaption.h + 10.0, m_rect.w - 12.0,
			bNoBitmap ? 0.0 : m_bmpMaxH + 10.0));
		rcImage = pImage->GetRect();
	}

	FrEdit* pEdit = GetCtrl983<FrEdit>("message");
	if (pEdit)
	{
		int lines = pEdit->GetLineNum();
		pEdit->SetRect(WRect(rcImage.x + 3.0, (bNoBitmap ? 40.0 : rcImage.h + 10.0) + rcImage.y, rcImage.w + 2.0,
			lines * 20.0 + 20.0));
		rcMsg = pEdit->GetRect();
	}

	float h = rcImage.y - rcCaption.y + rcCaption.h;
	h = h + rcImage.h;
	h = h + (rcMsg.x - rcImage.x);
	h = h + rcMsg.h;
	h = h + 70.0;
	if (h < 250.0 || 280.0f > h)
	{
		int n = 280;
		if (!(0.0f < rcImage.h))
			n = 250;
		h = (float)n;
	}
	SetSize983(m_rect.w, h, m_b148);

	FrWnd* pOk = GetCtrl983<FrWnd>("ok");
	FrWnd* pCancel = GetCtrl983<FrWnd>("cancel");
	const WRect& rcOk = pOk ? pOk->GetRect() : WRect(0.0f, 0.0f, 0.0f, 0.0f);
	float okW = rcOk.w;
	float okH = rcOk.h;
	const WRect& rcCancel = pCancel ? pCancel->GetRect() : WRect(0.0f, 0.0f, 0.0f, 0.0f);
	float cancelW = rcCancel.w;
	float cancelH = rcCancel.h;

	float x = m_rect.w * 0.5 - (okW + cancelW + m_rect.w * 0.1) * 0.5 - 0.5;
	if (!pCancel)
		x = x + 14.0;
	if (pOk)
	{
		pOk->SetRect(WRect(x + m_rect.x, m_rect.h + m_rect.y - 70.0, okW, okH));
		if (pCancel)
			pCancel->SetRect(WRect(okW + pOk->GetRect().x + m_rect.w * 0.1, pOk->GetRect().y, cancelW, cancelH));
	}
}

// 004fcfb0
bool FrFormBase::MoveCtrl983(const std::string& name, float x, float y)
{
	FrWnd* pWnd = GetCtrl983<FrWnd>(name);
	if (pWnd)
	{
		pWnd->SetRect(WRect(x, y, pWnd->GetRect().w, pWnd->GetRect().h));
		return true;
	}
	return false;
}

// 004fd050: a fila de figuras, centrada na largura do form, com a base alinhada embaixo, a partir do canto do "image"
void FrFormBase::DrawBitmaps983()
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
				pGDI->DrawTexture(pBmp, WRect(pImage->GetRect().x + x0 + pInfo->x,
					m_bmpMaxH - h + pImage->GetRect().y + pInfo->y, w, h), 0xffffffff, 0);
			}
		}
	}
}

// 004fd380
bool FrFormBase::OnInit()
{
	SetSize983(350.0f, 250.0f, false);
	Layout983();
	if (m_pSubForm && m_pSubForm->PreOpen983())
		m_pSubForm->OnInit();
	return FrForm::OnInit();
}

// 004fdb60
FrFormBase::~FrFormBase()
{
	m_pSubForm = NULL;
	m_ctrls.clear();
	m_bitmaps.clear();
	m_bmpTotalW = 0.0f;
	m_bmpMaxH = 0.0f;
	m_bmpNextX = 0.0f;
}

// 004fdc60
void FrFormBase::OnDraw()
{
	FrForm::OnDraw();
}

// 004fdc70
void FrFormBase::OnOK()
{
	FrForm::OnOK();
}

// 004fdc80
void FrFormBase::OnCancel()
{
	FrForm::OnCancel();
}

// 004fdc90
bool FrFormBase::Close(bool bFade)
{
	return FrForm::Close(bFade);
}

// 004fdca0
bool FrFormBase::Close(eFormRet result, bool bFade)
{
	return FrForm::Close(result, bFade);
}

// 004fdcb0
void FrFormBase::ClearSubForm983()
{
	m_pSubForm = NULL;
}

// 004fdcc0
bool FrFormBase::PreOpen983()
{
	return true;
}

// 004fe910
FrFormBase::FrFormBase()
{
	C_ASSERT_VC10(offsetof(FrFormBase, m_ctrls) == 0x110);
	C_ASSERT_VC10(offsetof(FrFormBase, m_bitmaps) == 0x120);
	C_ASSERT_VC10(offsetof(FrFormBase, m_pSubForm) == 0x144);
	m_pSubForm = NULL;
	m_ctrls.clear();
	m_bitmaps.clear();
	m_bmpTotalW = 0.0f;
	m_bmpMaxH = 0.0f;
	m_b148 = false;
	m_bmpNextX = 0.0f;
	SetCaption(K2L_Compatibility("\xbe\xcb\xb8\xb2"));
	UpdateScreenSize983();
}

// 004fea90
FrFormBase::BitmapInfoPtr FrFormBase::AddBitmap983(Bitmap* pBmp)
{
	if (!pBmp)
		return BitmapInfoPtr();
	m_bmpTotalW = (float)pBmp->Width() + 10.0 + m_bmpTotalW;
	if (m_bmpMaxH < (float)pBmp->Height())
		m_bmpMaxH = (float)pBmp->Height();
	BitmapInfoPtr pInfo(new sBitmapInfo);
	pInfo->pBmp = pBmp;
	pInfo->x = m_bmpNextX;
	pInfo->y = 0.0f;
	m_bmpNextX = (float)pBmp->Width() + 10.0 + m_bmpNextX;
	m_bitmaps.push_back(pInfo);
	return pInfo;
}

// 004fec30
FrFormBase::BitmapInfoPtr FrFormBase::AddOverlayBitmap983(Bitmap* pBase, Bitmap* pBmp, const WPoint& offset)
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

// 004fedd0
void FrFormBase::OnCtrlCreated983(IObject* pObj)
{
	FrWnd* pWnd = DYNAMIC_CAST(FrWnd, pObj);
	if (!pWnd)
		return;
	const std::string& name = ((FrWndFormBaseJp*)pWnd)->GetName983();
	switch (WNDGUITYPE983(pWnd))
	{
	case GI_FORM:
		AddCtrl983<FrForm>(name, pObj);
		break;
	case GI_FRAME:
		AddCtrl983<FrFrame>(name, pObj);
		break;
	case GI_BUTTON:
		AddCtrl983<FrButton>(name, pObj);
		break;
	case GI_STATIC:
		AddCtrl983<FrStatic>(name, pObj);
		break;
	case GI_TEXTBUTTON:
		AddCtrl983<FrTextButton>(name, pObj);
		break;
	case GI_AREA:
		AddCtrl983<FrArea>(name, pObj);
		break;
	case GI_LISTBOX:
		AddCtrl983<FrListBox>(name, pObj);
		break;
	case GI_EDIT:
		AddCtrl983<FrEdit>(name, pObj);
		break;
	case GI_COMBOBOX:
		AddCtrl983<FrComboBox>(name, pObj);
		break;
	case GI_GAUGEBAR:
		AddCtrl983<FrGaugeBar>(name, pObj);
		break;
	case GI_GAUGEBAREX:
		AddCtrl983<FrGaugeBarEx>(name, pObj);
		break;
	case GI_GAUGEBARIMAGE:
		AddCtrl983<FrGaugeBarImage>(name, pObj);
		break;
	case GI_VIEWER:
		AddCtrl983<FrViewer>(name, pObj);
		break;
	case GI_CONTEXTMENU:
		AddCtrl983<FrWnd>(name, pObj);
		break;
	case GI_TABBUTTON:
		AddCtrl983<FrTabButton>(name, pObj);
		break;
	case GI_GROUPBOX:
		AddCtrl983<FrGroupBox>(name, pObj);
		break;
	case GI_COMBOCTLEX:
		AddCtrl983<FrComboCtlEx>(name, pObj);
		break;
	}
}

IMPLEMENT_OBJECT(FrFormBase, FrForm)
