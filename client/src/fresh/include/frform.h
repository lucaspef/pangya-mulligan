#pragma once
#include "rtti.h"
#include "frcmdtarget.h"
#include "frelement.h"
#include "frwnd.h"
#ifndef REBANG_FRFORM_IN_PCH // HACK: the ProjectG PCH includes frframe.h later (see stdafx.h)
#include "frframe.h"
#endif
#include "../../../include/engine/wtypes.h"

enum eFormRet
{
	FrNONE,
	FrOK,
	FrCANCEL,
	FrYES,
	FrNO
};

class FrWndManager;

template <class T>
T* CreateForm(FrWndManager* pManager, FrCmdTarget* pCmdDest,
	const char* lpszTemplateID, FrWnd* pParent = NULL)
{
	T* pForm = new T;

	FrForm* pRet = pForm->_Init(pManager, pCmdDest, lpszTemplateID, pParent);
	if (pRet)
		return DYNAMIC_CAST(T, pRet);

	delete pForm;
	return NULL;
}

class FrEdit;
class FrFrame;
class FrGuiItem;
class FrStatic;

enum eFormFlags
{
	FrESCAPE = 0x1,
	FrENTER = 0x2,
	FrMODALESS = 0x4,
	FrNOTAIL = 0x8,
	FrICON = 0x10,
	FrRET_NONE = 0x20,
	FrRET_OK = 0x40,
	FrRET_CANCEL = 0x80,
	FrNOSOUND = 0x100,
	FrEXCLUSIVE = 0x200,
	FrRESIZABLE = 0x400
};

// 983: alinhamento de 16 (sizeof de toda tela derivada é múltiplo de 16; o membro que o causa não foi identificado)
class __declspec(align(16)) FrForm : public FrWnd
{
	friend class FrWndManager;

public:
	static const WRTTI m_RTTI;
#ifdef REBANG_FRFORM_GETRTTI_LATE // HACK: valentineeventdlg emission order (PCH inline in the original)
	virtual const WRTTI* GetRTTI() const;
#else
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }
#endif

	FrForm();
	virtual ~FrForm();
	void Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent);
	FrForm* _Init(FrWndManager* pManager, FrCmdTarget* pCmdDest,
		const char* lpszTemplateID, FrWnd* pParent);
	virtual bool Open(FRESH_PFN_RESULT pFnResult, unsigned long flag);
	virtual bool Open(FRESH_PFN_RESULT pFnResult, const WPoint& pos,
		unsigned long flag);
	virtual bool Close(eFormRet result, bool bFade);
	virtual bool Close(bool bFade);
	void SetResultCallback(FRESH_PFN_RESULT pFnResult);
	virtual void SetCaption(const char* caption);
	virtual void SetDesc(const char* desc);
	void ResetPosition983(WPoint offset); // 983: 00be6d70 (posicao de criacao do layout + deslocamento)
	virtual void SetMessage(const char* msg, bool spaceAlign);
	virtual void EnableDrag(bool drag);
	virtual void SetTimeLimit(float limit)
	{
		m_timeLimit = limit;
		m_dt = 0.0f;
	}
#ifdef REBANG_FRFORM_IN_PCH
	void DescHidePrivacy();
#else
	void DescHidePrivacy() { m_pBaseFrm->HidePrivacy(true); }
#endif
	bool GetFlag(unsigned long flag) { return (m_flag & flag) ? true : false; }
	void SetFlag(unsigned long flag) { m_flag = flag; }
	void EnableFlag(unsigned long flag) { m_flag |= flag; }
	void DisableFlag(unsigned long flag) { m_flag &= ~flag; }
	void SetCloseResult(eFormRet ret) { m_defaultRetCode = ret; }
	void Adjust(WRect& rtRect);
	void SetFrameCaptionFocus(bool bEnable);
#ifdef REBANG_FRFORM_IN_PCH
	void SetCaptionOffset(bool offset);
#else
	void SetCaptionOffset(bool offset) { m_pBaseFrm->SetCaptionOffset(offset); }
#endif
	void SetMessageControlByType(enumGuiType type, FrWnd* pChild);
	void DrawModalBack983(float alpha); // 983: FrModalRenderer 00be7cf0 (OnDraw 00be62d0; DoFadeDisplay 00be63d0 = FrWnd::DoFadeDisplay chama aqui)
	virtual bool OnInit();

protected:
	virtual void OnDraw();
	virtual void OnProc(const float deltaTime);
	virtual void OnOK();
	virtual void OnCancel();

public:
	virtual void OnMouseMove(const WPoint& mousePos);
	virtual bool OnLButtonDown(const WPoint& mousePos);
	virtual bool OnLButtonUp(const WPoint& mousePos);
	virtual void OnSetCursor(bool bInClient, const WPoint& mousePos);
	virtual void SetRect(const WRect& rect);
	virtual void SetIconRect(const WRect& rect);
	virtual void SetClientRect(const WRect& rect);

protected:
	bool m_canDrag;
	bool m_bCaptionDown;
	bool m_bResizeBtnDown;
	unsigned long m_flag;
	eFormRet m_retCode;
	FrCmdTarget* m_pCmdDest;
	FrFrame* m_pBaseFrm;
	FrStatic* m_pMsgStatic;
	FrEdit* m_pMsgEdit;
	float m_timeLimit;
	float m_dt;
	eFormRet m_defaultRetCode;
	int m_pad983_f8; // 983 +0xf8: sem acesso (ctor/Open/Close; out/F5/layouts/FrForm.tsv)
	FRESH_PFN_RESULT m_pFnResult;

private:
	bool IsTitleBarArea(const WPoint& pos);
	virtual bool IsResizeBtnArea(const WPoint& pos);
	static WPoint ms_oldPos;

protected:
	void OnFreshOkay();
	void OnFreshCancel();
	void OnFreshYes();
	void OnFreshNo();

	DECLARE_FRESH_MSGMAP()

public:
	static bool ms_bHasTail;
	static bool ms_bExclusive;
	static bool ms_bModalLock983;
	static void EnableTail(bool enable) { ms_bHasTail = enable; }
};
