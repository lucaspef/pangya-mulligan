#pragma once

#include "frform.h"
#include "../shared/classdefine.h"
#include "../shared/globalgamedefine.h"

class FrArea;
class FrButton;
class FrGaugeBar;
class FrEdit;
class WTitleFont;

class FrCompoundDlg : public FrForm
{
	DECLARE_OBJECT(FrCompoundDlg)

	FrCompoundDlg();
	virtual ~FrCompoundDlg();

	virtual void OnProc(const float time);

	void SetCompound(IFF_STRUCT::sQuest& quest);
	void SetCompoundRes(unsigned char result, sItemInfo& itemInfo);
	void SetDlgText(const char* text);

protected:
	void OnYesInit(int param);
	void OnYesBtnUp();
	void OnNoInit(int param);
	void OnNoBtnUp();
	void OnOKInit(int param);
	void OnOKBtnUp();
	void OnGaugeBarInit(int param);
	void OnTextEditInit(int param);
	void OnCompResInit(int param);
	void OnCompResOwnerDraw();

	IFF_STRUCT::sQuest m_quest;
	sItemInfo m_resultItem;
	FrArea* m_pCompRes;
	bool m_bShowResult;
	FrButton* m_pBtn[3];
	FrGaugeBar* m_pGaugeBar;
	FrEdit* m_pTextEdit;
	unsigned char m_result;
	unsigned long m_unused2b0;
	WTitleFont* m_pFont;

	DECLARE_FRESH_MSGMAP()
};
