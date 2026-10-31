#pragma once

#include "frform.h"
#ifdef REBANG_TASKMAIN_PCH
#include "fredit.h"
#endif

class FrEdit;
class FrArea;
class FrButton;
class FrEmoticonDlg;

class FrOnelineReqChkOutDlg : public FrForm
{
	DECLARE_OBJECT(FrOnelineReqChkOutDlg)

	FrOnelineReqChkOutDlg();
	virtual ~FrOnelineReqChkOutDlg();

protected:
	void OnExplainInit(int param);
	void OnYesInit(int param);

	DECLARE_FRESH_MSGMAP()
};

class FrOnelineReqDlg : public FrForm
{
#ifdef REBANG_GOLFRULE_PCH
	friend class CGolfRule;
#endif
	DECLARE_OBJECT(FrOnelineReqDlg)

	FrOnelineReqDlg();
	virtual ~FrOnelineReqDlg();

	void SetLanguage(bool english);
#ifdef REBANG_TASKMAIN_PCH // inline in the original header; other includers keep their IL
	const char* GetInputString() { return m_pChatInput->GetLine(1, false); }
#endif

protected:
	void OnExplainInit(int param);
	void OnChatInputInit(int param);
	bool OnChatEnterKey(int param);
	void OnEmoticonInit(int param);
	void OnEmoticonBtnUp();
	bool OnEmoticonResult(int result, FrForm* pForm);
	void OnLanguageInit(int param);
	void OnYesInit(int param);

	FrEdit* m_pChatInput;
	FrArea* m_pLanguage;
	FrButton* m_pEmoticon;
	FrEmoticonDlg* m_pEmoticonDlg;

	DECLARE_FRESH_MSGMAP()
};
