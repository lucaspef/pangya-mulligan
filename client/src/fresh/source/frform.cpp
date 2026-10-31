#include <stdio.h>
#include "vc10_layout.h"
#include <string.h>
#include "frform.h"
#include "frframe.h"
#include "frstatic.h"
#include "frtext.h"
#include "fredit.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "frgraphicinterface.h"
#include "frelement.h"
#include "soundmanager.h"
#include "wlocalize.h"

extern WView* g_view;

bool FrForm::ms_bHasTail = true;
bool FrForm::ms_bExclusive = false;
bool FrForm::ms_bModalLock983 = false; // 983: 00f1ab38 (lido tambem por 00be7950/00be7cf0 do FrWnd)
WPoint FrForm::ms_oldPos;

IObject* FrFormMakeInstance()
{
	return new FrForm;
}

struct __sFrForm
{
	__sFrForm()
	{
		ObjectFactory().AddObjectFunctor(FrFormMakeInstance, "FrForm");
	}
};

const WRTTI FrForm::m_RTTI("FrForm", &FrWnd::m_RTTI);
static __sFrForm __implFrForm;

BEGIN_FRESH_MSGMAP(FrForm, FrCmdTarget)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrForm::OnFreshOkay)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrForm::OnFreshCancel)
ON_FRESH_VV("yes", FRCMD_LBUTTONUP, FrForm::OnFreshOkay)
ON_FRESH_VV("no", FRCMD_LBUTTONUP, FrForm::OnFreshCancel)
END_FRESH_MSGMAP()

void FrForm::OnFreshCancel()
{
	OnCancel();
}

void FrForm::OnFreshOkay()
{
	OnOK();
}

FrForm::FrForm()
	: m_canDrag(true),
	  m_bCaptionDown(false),
	  m_bResizeBtnDown(false),
	  m_pBaseFrm(NULL),
	  m_pMsgStatic(NULL),
	  m_pMsgEdit(NULL),
	  m_timeLimit(-1.0f)
{
	// mapa de layout do 983 (docs/LAYOUTS.md, A193): ctor 00be5f00, Open 00be6520, Close 00be6098
	C_ASSERT_VC10(sizeof(FrForm) == 0x110 && offsetof(FrForm, m_canDrag) == 0xd0 && offsetof(FrForm, m_flag) == 0xd4 && offsetof(FrForm, m_timeLimit) == 0xec && offsetof(FrForm, m_pFnResult) == 0x100);
	C_ASSERT(offsetof(FrForm, m_modalPos) == 0xc); // L0
	C_ASSERT(offsetof(FrForm, m_modalOn) == 0x18); // L1
	C_ASSERT_VC10(offsetof(FrForm, m_wndAlpha2) == 0x84); // L2
	C_ASSERT_VC10(offsetof(FrForm, m_fadeTime) == 0x94); // L3
	C_ASSERT_VC10(offsetof(FrForm, m_rect) == 0x34); // L4
	// A206: FrWnd inteiro (base de todas as janelas) medido no 983 (out/F5/layouts/FrWnd.tsv, 22 campos conferem pelo uso;
	// +0x98 m_iconRect = SetIconRect 00be61e0, +0xc8 m_guiType983 = DoCreate 00be3757); sizeof(FrWnd) = 0xd0 (m_canDrag)
	C_ASSERT(offsetof(FrForm, m_nFlags) == 0x1c && offsetof(FrForm, m_pParentWnd) == 0x20 && offsetof(FrForm, m_pOwner) == 0x24 && offsetof(FrForm, m_childList) == 0x28);
	C_ASSERT_VC10(offsetof(FrForm, m_dwStyle) == 0x44 && offsetof(FrForm, m_wndText) == 0x48 && offsetof(FrForm, m_wndName) == 0x64 && offsetof(FrForm, m_wndAlpha) == 0x80);
	C_ASSERT_VC10(offsetof(FrForm, m_nRefID) == 0x88 && offsetof(FrForm, m_pWndManager) == 0x8c && offsetof(FrForm, m_pScrBar) == 0x90 && offsetof(FrForm, m_iconRect) == 0x98);
	C_ASSERT_VC10(offsetof(FrForm, m_szPushSound) == 0xa8 && offsetof(FrForm, m_dblClickTimeout) == 0xac && offsetof(FrForm, m_dblClicked) == 0xb0 && offsetof(FrForm, m_dblClickPos) == 0xb4);
	C_ASSERT_VC10(offsetof(FrForm, m_hoverOn) == 0xbc && offsetof(FrForm, m_hoverTime) == 0xc0 && offsetof(FrForm, m_accHoverTime) == 0xc4);
	C_ASSERT_VC10(sizeof(FrWnd) == 0xd0); // +0xc8 m_guiType983 e +0xcc m_pToolTipData sao privados: cabem so entre +0xc8 e 0xd0
	// FrForm: os 13 campos com uso conferem (out/F5/layouts/FrForm.tsv); +0xf8 m_pad983_f8 sem acesso no 983 (ctor/Open/Close)
	C_ASSERT_VC10(offsetof(FrForm, m_bCaptionDown) == 0xd1 && offsetof(FrForm, m_bResizeBtnDown) == 0xd2 && offsetof(FrForm, m_retCode) == 0xd8 && offsetof(FrForm, m_pCmdDest) == 0xdc);
	C_ASSERT_VC10(offsetof(FrForm, m_pBaseFrm) == 0xe0 && offsetof(FrForm, m_pMsgStatic) == 0xe4 && offsetof(FrForm, m_pMsgEdit) == 0xe8 && offsetof(FrForm, m_dt) == 0xf0 && offsetof(FrForm, m_defaultRetCode) == 0xf4);
	m_flag = 0;
	m_defaultRetCode = FrNONE;
}

FrForm::~FrForm()
{
}

void FrForm::Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent)
{
	m_pFnResult = NULL;

	FrElementDoc* pDoc = pManager->GetDocument();
	FrElementForm* pElement = pDoc->GetForm(item.m_resource);
	if (pElement == NULL)
		return;

	std::map<std::string, std::string>& param = item.m_param;

	if (param.find("drag") != param.end())
	{
		int tf;
		sscanf(param["drag"].c_str(), "%d", &tf);
		EnableDrag(tf != 0);
	}

	if (param.find("modal") != param.end() ||
		(param.find("modal") != param.end() && param["modal"] == "false"))
		m_flag |= FrMODALESS;

	WRect rect((float)item.m_rect.left, (float)item.m_rect.top,
		(float)pElement->m_bgSize.w, (float)pElement->m_bgSize.h);
	m_pCmdDest = m_pOwner;
	Create("Form", item.m_name.c_str(), pManager, 0x21, rect, pParent);

	FrWnd* pWnd =
		pManager->DoCreate(pElement->m_bgFrame, pManager, this, m_pOwner);
	m_pBaseFrm = DYNAMIC_CAST(FrFrame, pWnd);
	SetCaption(pElement->m_caption.c_str());
	SetDesc(pElement->m_desc.c_str());

	std::list<FrGuiItem*>::iterator it;
	for (it = pElement->m_guiList.begin(); it != pElement->m_guiList.end();
		++it)
	{
		FrWnd* pChild = pManager->DoCreate(**it, pManager, this, m_pOwner);
		switch ((*it)->m_type)
		{
		case GI_STATIC:
		case GI_EDIT:
			if ((*it)->m_name == "message")
				SetMessageControlByType((*it)->m_type, pChild);
			break;
		}
	}

	SetViewFocus(false);

	// 983 (00be743d..00be746a, fim de Init): area do escurecimento = desktop (+0xc/+0x10) e +0x14 = FrEmoticon;
	// sem isto o form criado por Init (ex.: Gift/G1) nunca escurecia a cena (DrawModalBack983 sai com m_modalEmoticon983 == 0)
	FrDesktop* pDesktop = pManager->GetDesktop();
	m_modalPos = WPoint(pDesktop->GetRect().w, pDesktop->GetRect().h);
	m_modalEmoticon983 = (int)pManager->GetEmoticon();
}

#ifdef JP983_RASTRO
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
#endif
// 983: FrModalRenderer 00be7cf0 (chamada por OnDraw 00be62d0 e DoFadeDisplay 00be63d0): caixa preta do tamanho do desktop atras
// da form modal; o 1o desenho depois de ms_bModalLock983 so desliga o modal (o bloqueio e de um desenho).
void FrForm::DrawModalBack983(float alpha)
{
#ifdef JP983_RASTRO
	jp983_rastro("DrawModalBack983 form=%s modalOn=%d lock=%d unk=%d a=%.2f", m_wndName.c_str(), (int)m_modalOn, (int)ms_bModalLock983, m_modalEmoticon983, alpha);
#endif
	if (m_modalEmoticon983 == 0)
		return;
	if (ms_bModalLock983)
	{
		ms_bModalLock983 = false;
		m_modalOn = false;
		return;
	}
	if (m_modalOn)
	{
		WRect rc(0.0f, 0.0f, m_modalPos.x, m_modalPos.y);
		float a = (float)(alpha * 110.0);
		a = a / 255.0f;
		WOverlay::DrawBox(g_view, rc, 0, FrALPHA(0xff000000, a), 0.0f);
	}
}

void FrForm::SetMessageControlByType(enumGuiType type, FrWnd* pChild)
{
	if (type == GI_STATIC)
	{
		m_pMsgStatic = DYNAMIC_CAST(FrStatic, pChild);
	}
	else if (type == GI_EDIT)
	{
		m_pMsgEdit = DYNAMIC_CAST(FrEdit, pChild);
	}
}

FrForm* FrForm::_Init(FrWndManager* pManager, FrCmdTarget* pCmdDest,
	const char* lpszTemplateID, FrWnd* pParent)
{
	m_pFnResult = NULL;

	FrElementDoc* pDoc = pManager->GetDocument();
	FrElementForm* pForm = pDoc->GetForm(lpszTemplateID);
	if (pForm == NULL)
		return NULL;

	pManager->SetKeyFocus(NULL, true);

	WSize s((float)pForm->m_bgSize.w, (float)pForm->m_bgSize.h);
	WPoint pos = pManager->GetCreatePosition(s);
	WRect rect(pos, s);

	SetOwner(this);
	m_pCmdDest = pCmdDest;

	if (pParent)
		Create("Form", lpszTemplateID, pManager, 0x21, rect, pParent);
	else
		Create("Form", lpszTemplateID, pManager, 0x21, rect,
			(FrWnd*)pManager->GetDesktop());

	FrWnd* pWnd = pManager->DoCreate(pForm->m_bgFrame, pManager, this, this);
	m_pBaseFrm = DYNAMIC_CAST(FrFrame, pWnd);
	// 983 (00be6a20): a legenda passa pela tabela coreano -> japones (00b4ec60)
	const char* caption = K2L_Compatibility(pForm->m_caption.c_str());
	SetCaption(caption);
	SetDesc(pForm->m_desc.c_str());

	std::list<FrGuiItem*>::iterator it;
	for (it = pForm->m_guiList.begin(); it != pForm->m_guiList.end(); ++it)
	{
		FrWnd* pChild = pManager->DoCreate(**it, pManager, this, this);
		if ((*it)->m_name == "message")
			SetMessageControlByType((*it)->m_type, pChild);
	}

	// 983: fim de _Init (00be6cab): area do escurecimento = tamanho do desktop; +0x14 = o FrEmoticon do gerenciador
	FrDesktop* pDesktop = pManager->GetDesktop();
	m_modalPos = WPoint(pDesktop->GetRect().w, pDesktop->GetRect().h);
	m_modalEmoticon983 = (int)pManager->GetEmoticon();

	return this;
}

bool FrForm::Open(FRESH_PFN_RESULT pFnResult, unsigned long flag)
{
	if (!WndManager()->IsValidWindow(this))
		return false;

	m_pFnResult = pFnResult;
	m_flag = flag;

	if (ms_bExclusive)
		SetVisible(false);

	SetViewFocus(!IsFixed() && !ms_bExclusive);
	OnInit();

	if (!(m_flag & FrNOSOUND))
		g_audio->PlaySfx("ui_dialog_open");

	if (m_flag & FrEXCLUSIVE)
		ms_bExclusive = true;

	if (flag & 0x800) // 983: flag 0x800 (= FWF_FADEOUT_EX no m_nFlags) marca o form com FWF_UNK4000_983
		m_nFlags.Enable(FWF_UNK4000_983);

	bool modeless = (flag & FrMODALESS) != 0;
	if (!modeless)
		SetFadeout(false);

	WndManager()->CountForms983();

	// 983: 00be7930 (liga o modal quando o byte global 00f1ab38 está desligado)
	if (!modeless && !(flag & 0x800) && !ms_bModalLock983)
		m_modalOn = true;

	return true;
}

bool FrForm::Open(FRESH_PFN_RESULT pFnResult, const WPoint& pos,
	unsigned long flag)
{
	MoveWindow(pos);
	Open(pFnResult, flag);

	SetFadeout(false);

	return true;
}

bool FrForm::Close(bool bFade)
{
	return Close(m_defaultRetCode, bFade);
}

bool FrForm::Close(eFormRet result, bool bFade)
{
	WndManager()->ResetKey();

	m_retCode = result;
	if (m_pFnResult)
	{
		if (!(m_pCmdDest->*m_pFnResult)(result, this))
			return false;
	}

	if (!(m_flag & FrICON))
		SetIconRect(m_rect);

	if (!(m_flag & FrNOSOUND))
		g_audio->PlaySfx("ui_dialog_close");

	if (m_flag & FrEXCLUSIVE)
		ms_bExclusive = false;

	return FrWnd::Close(ms_bHasTail ? bFade : false);
}

void FrForm::SetResultCallback(FRESH_PFN_RESULT pFnResult)
{
	m_pFnResult = pFnResult;
}

void FrForm::SetCaption(const char* caption)
{
	if (m_pBaseFrm)
		m_pBaseFrm->SetCaption(caption);
}

void FrForm::SetDesc(const char* desc)
{
	if (m_pBaseFrm)
		m_pBaseFrm->SetDesc(desc);
}

// 983: 00be6d70 (COption 00494bb0, quando a resolucao muda)
void FrForm::ResetPosition983(WPoint offset)
{
	FrElementDoc* pDoc = m_pWndManager->GetDocument();
	FrElementForm* pElement = pDoc->GetForm(m_wndName.c_str());
	if (pElement)
	{
		WSize size((float)pElement->m_bgSize.w, (float)pElement->m_bgSize.h);
		WPoint pos = m_pWndManager->GetCreatePosition(size);
		pos += offset;
		SetRect(WRect(pos, size));
	}
}

// F4 (C92.2): rastro provisório no jp983_stubs.log que o robô traz (tools/v6/f4stub/falta983.cpp):
// texto e quem chamou, para achar a origem de cada caixa "notify". Tirar quando o login passar.
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
extern "C" void* _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)

void FrForm::SetMessage(const char* msg, bool spaceAlign)
{
#ifdef JP983_RASTRO
	jp983_rastro("FrForm::SetMessage ret=%p form=%s msg=%.120s", _ReturnAddress(),
		m_wndName.c_str(), msg ? msg : "(null)");
#endif
	if (m_pMsgStatic)
		m_pMsgStatic->SetCaption(msg);

	if (m_pMsgEdit)
	{
		m_pMsgEdit->ClearLine();
		m_pMsgEdit->AddText(msg, spaceAlign, true);
	}
}

void FrForm::EnableDrag(bool drag)
{
	m_canDrag = drag;
}

bool FrForm::OnInit()
{
	if (ms_bHasTail && !(m_flag & FrNOTAIL))
	{
		if (!(m_flag & FrICON))
			SetIconRect(m_rect);
		m_nFlags.Enable(FWF_FADING);
	}

	if (m_flag & FrRET_NONE)
		m_defaultRetCode = FrNONE;
	else if (m_flag & FrRET_OK)
		m_defaultRetCode = FrOK;
	else if (m_flag & FrRET_CANCEL)
		m_defaultRetCode = FrCANCEL;

	return true;
}

void FrForm::SetIconRect(const WRect& rect)
{
	// 983: (1 - 0) * 0.5 e 0 calculados em x87 (fld1; fldz; fsub), sem dobrar a constante
	float zero = 0.0f;
	double half = (1.0f - zero) * 0.5;
	m_iconRect.x = (float)(rect.x + rect.w * half);
	m_iconRect.y = (float)(rect.y + rect.h * half);
	m_iconRect.w = rect.w * zero;
	m_iconRect.h = rect.h * zero;
}

void FrForm::OnOK()
{
	if (!(m_flag & FrNOSOUND))
		g_audio->PlaySfx("ui_button_ok_click");

	Close(FrOK, true);
}

void FrForm::OnCancel()
{
	if (!(m_flag & FrNOSOUND))
		g_audio->PlaySfx("ui_button_cancel_click");

	Close(FrCANCEL, true);
}

void FrForm::OnDraw()
{
	if (!(m_flag & FrMODALESS))
		DrawModalBack983(m_wndAlpha2);
	FrWnd::OnDraw();
}

void FrForm::OnProc(const float deltaTime)
{
	FrWnd::OnProc(deltaTime);

	if (m_timeLimit > 0.0f)
	{
		m_dt += deltaTime;
		if (m_dt > m_timeLimit)
			OnCancel();
	}
}

bool FrForm::IsTitleBarArea(const WPoint& pos)
{
	WRect topArea(m_rect.x, m_rect.y, m_rect.w, 20.0f);

	return topArea.IsInRect(pos);
}

bool FrForm::IsResizeBtnArea(const WPoint& pos)
{
	WRect btnArea(m_rect.x + m_rect.w - 16.0f, m_rect.y + m_rect.h - 16.0f,
		16.0f, 16.0f);

	return false;
}

void FrForm::OnSetCursor(bool bInClient, const WPoint& mousePos)
{
	if (!IsViewFocused())
		return;

	if (IsTitleBarArea(mousePos) && m_canDrag)
		SetCursor(1);
}

void FrForm::OnMouseMove(const WPoint& mousePos)
{
	if (GetCapture() == this)
	{
		WPoint delta = mousePos - ms_oldPos;
		delta.x = (float)(int)delta.x;
		delta.y = (float)(int)delta.y;

		if (m_canDrag && m_bCaptionDown)
		{
			WRect dr(m_rect.TopLeft() + delta, m_rect.Size());
			WndManager()->ConfineRect(dr);
			FrWnd::MoveWindow(dr.TopLeft());
		}

		if (m_canDrag && m_bResizeBtnDown)
		{
			WRect dr(m_rect.TopLeft(), m_rect.Size() + WSize(delta.x, delta.y));
			WndManager()->ConfineRect(dr);
			SetRect(dr);
		}

		ms_oldPos = mousePos;
	}
}

bool FrForm::OnLButtonDown(const WPoint& mousePos)
{
	if (IsViewFocused())
	{
		if (IsTitleBarArea(mousePos))
		{
			m_bCaptionDown = true;
			ms_oldPos = mousePos;
			SetCapture();
		}

		if (IsResizeBtnArea(mousePos))
		{
			m_bResizeBtnDown = true;
			ms_oldPos = mousePos;
			SetCapture();
		}
	}

	FindNextTopFocus(false);

	return false;
}

bool FrForm::OnLButtonUp(const WPoint& mousePos)
{
	ReleaseCapture();

	m_bCaptionDown = false;
	m_bResizeBtnDown = false;

	return false;
}

void FrForm::SetRect(const WRect& rect)
{
	FrWnd::SetRect(rect);

	if (m_pBaseFrm)
		m_pBaseFrm->SetRect(rect);
}

void FrForm::SetClientRect(const WRect& rect)
{
	FrWnd::SetClientRect(rect);

	if (m_pBaseFrm)
		m_pBaseFrm->SetClientRect(rect);
}

void FrForm::Adjust(WRect& rtRect)
{
	float fWidth = g_view->GetWidth();
	float fHeight = g_view->GetHeight();

	if ((rtRect.x + rtRect.w) > fWidth)
		rtRect.x = fWidth - rtRect.w;

	if ((rtRect.y + rtRect.h) > fHeight)
		rtRect.y = fHeight - rtRect.h;
}

void FrForm::SetFrameCaptionFocus(bool bEnable)
{
	if (m_pBaseFrm)
		m_pBaseFrm->SetCaptionFocus(bEnable);
}
