#pragma once

#include "frform.h"

class FrComboBox;
#include <string>
#include <vector>

class FrQuickStartSettingDlg;

struct sIntrest
{
	char group[33];
	unsigned char code;
	char name[33];
};

class FrQuickResultDlg : public FrForm
{
	DECLARE_OBJECT(FrQuickResultDlg)

	FrQuickResultDlg();
	virtual ~FrQuickResultDlg();

	void SetContents(const char* text);
	void SetOkbtnEnable(bool bEnable);

protected:
	void OnOkBtnInit(int param);
	void OnOkBtnUp();

	FrButton* m_pOkBtn;

	DECLARE_FRESH_MSGMAP()
};

class FrQuickStartDlg : public FrForm
{
	DECLARE_OBJECT(FrQuickStartDlg)

	FrQuickStartDlg();
	virtual ~FrQuickStartDlg();

	virtual bool OnInit();

protected:
	void OnStrokeBtnInit(int param);
	void OnTournamentBtnInit(int param);
	void OnBattleBtnInit(int param);
	void OnChatBtnInit(int param);
	void OnOptionBtnInit(int param);
	void OnStrokeStopInit(int param);
	void OnTournamenStopInit(int param);
	void OnBattleStopInit(int param);
	void OnChatStopInit(int param);
	void OnStrokeBtnUp();
	void OnTournamentBtnUp();
	void OnBattleBtnUp();
	void OnChatBtnUp();
	void OnOptionBtnUp();
	bool OnQuickStartSettingDlgResult(int result, FrForm* form);
	void SendRequestMatching(unsigned char type);

	FrButton* m_pStrokeBtn;
	FrButton* m_pTournamentBtn;
	FrButton* m_pBattleBtn;
	FrButton* m_pChatBtn;
	FrButton* m_pOptionBtn;
	FrArea* m_pStrokeStop;
	FrArea* m_pTournamentStop;
	FrArea* m_pBattleStop;
	FrArea* m_pChatStop;
	FrQuickStartSettingDlg* m_pSettingDlg;

	DECLARE_FRESH_MSGMAP()
};

class FrQuickStartSettingDlg : public FrForm
{
	DECLARE_OBJECT(FrQuickStartSettingDlg)

	FrQuickStartSettingDlg();
	virtual ~FrQuickStartSettingDlg();

	void UpdateData();

protected:
	void OnChkStrokeBtnInit(int param);
	void OnChkTournamentBtnInit(int param);
	void OnChkBattleBtnInit(int param);
	void OnChkChatBtnInit(int param);
	void OnChkStrokeBtnUp();
	void OnChkTournamentBtnUp();
	void OnChkBattleBtnUp();
	void OnChkChatBtnUp();
	void OnCmbCondition1BoxInit(int param);
	void OnCmbCondition2BoxInit(int param);
	void OnCmbCondition3BoxInit(int param);
#ifndef REBANG_SEMANTIC_ONLY
	// HACK: unwind funclets of the naked OnCmbConditionNBoxInit (see the .cpp)
	static void __cdecl RbCc1Funclets();
	static void __cdecl RbCc2Funclets();
	static void __cdecl RbCc3Funclets();
#endif

	FrButton* m_pChkStroke;
	FrButton* m_pChkTournament;
	FrButton* m_pChkBattle;
	FrButton* m_pChkChat;
	FrComboBox* m_pCondition[3];
	int m_bCheck[0x17]; // 983: 0x17 interesses (+0x12c)

	DECLARE_FRESH_MSGMAP()
public:
	char m_pad983_188[0x8]; // 983: sem leitura/escrita (ctor 00ad0af0 zera so +0x12c..+0x184; vtable d4aec4 so dtor/runtime class; 00acf000..00ad6000 sem +0x188..+0x18f); sizeof 0x190 (push em 00ad1552)
};

class CMatchingSystem : public WSingleton<CMatchingSystem>
{
public:
	CMatchingSystem();
	virtual ~CMatchingSystem();

	void Clear();
	int SetDisplayIntrest(const sIntrest& intrest);
	int GetDisplayIntrests(const std::string& group,
		std::vector<sIntrest>* out);
	const char* GetGroup(unsigned char code);
	const sIntrest* GetIntrestInfo(unsigned char code);
	unsigned char GetDisplayIntrestCode(const std::string& group,
		const std::string& name);
	std::string GetNameChecked(const std::string& group);

	int SetMyIntrest(unsigned char code);
	int CheckIntrestCode(unsigned char code);
	void ClearMyIntrests() { m_myIntrests.clear(); }

protected:
	std::vector<sIntrest> m_intrests;
	std::vector<unsigned char> m_myIntrests;
};
