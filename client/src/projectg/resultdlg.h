#pragma once

#include "frform.h"
#include "../shared/globalgamedefine.h"
#include "shareddoc.h"

class CExhibition;
class CPartTidList;
class FrScoreDlg;
class FrTreasureScore;
class FrTreasureGift;
class FrAwardInfoDlg;

class FrResultDlg : public FrForm
{
	DECLARE_OBJECT(FrResultDlg)

	FrResultDlg();
	virtual ~FrResultDlg();

protected:
	virtual bool OnInit();
	virtual void OnProc(const float delta);

	void ResetVars();
	void FindWinner();
	void ShowPet();

	void OnStageInit(int param);
	void OnStageOwnerDraw();
	void OnTrophyInit(int param);
	void OnMatchStaticInit(int param);
	void OnMatchInit(int param);
	void OnIdInit(int param);
	void OnIdOwnerDraw(int param);
	void OnScoreStaticInit(int param);
	void OnScoreInit(int param);
	void OnPangInit(int param);
	void OnBonusStaticInit(int param);
	void OnBonusInit(int param);
	void OnGetItemInit(int param);
	void OnGetItemOwnerDraw();
	void OnLevelInit(int param);
	void OnExpInit(int param);
	void OnExpGaugeBarInit(int param);
	void OnLevelUpInit(int param);

	sRivalData m_winner;
	int m_startLevel;
	int m_level;
	int m_exp;
	int m_addExp;
	float m_expTime;
	bool m_bExpDone;
	FrArea* m_pStage;
	FrArea* m_pLevel;
	FrStatic* m_pExp;
	FrGaugeBar* m_pExpGaugeBar;
	FrArea* m_pLevelUp;
	FrArea* m_pGetItem;
	FrArea* m_pId;
	CExhibition* m_pExhibition;
	CPartTidList* m_pPartTidList;
	WRect m_stageRect;
	WTitleFont* m_pTitleFont;
	WOverlay* m_pHalloweenOverlay;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbShowpetFunclets(); // HACK: EH funclets of the naked ShowPet (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOninitFunclets(); // HACK: EH funclets of the naked OnInit (nakedify)
#endif
};

class FrResultTeamDlg : public FrResultDlg
{
	DECLARE_OBJECT(FrResultTeamDlg)

	FrResultTeamDlg() { }
	virtual ~FrResultTeamDlg() { }

protected:
	virtual bool OnInit();

	void OnStageInit(int param);
	void OnStageOwnerDraw();
	void OnTrophyInit(int param);
	void OnMatchTypeInit(int param);
	void OnMatchInit(int param);
	void OnIdInit(int param);
	void OnIdOwnerDraw(int param);
	void OnRankStaticInit(int param);
	void OnResultStaticInit(int param);
	void OnResultInit(int param);
	void OnWinLoseInit(int param);
	void OnScoreInit(int param);
	void OnPangInit(int param);
	void OnBonusStaticInit(int param);
	void OnBonusInit(int param);
	void OnGetItemInit(int param);
	void OnGetItemOwnerDraw();
	void OnLevelInit(int param);
	void OnExpInit(int param);
	void OnExpGaugeBarInit(int param);
	void OnLevelUpInit(int param);
	void OnRedInit(int param);
	void OnRedOwnerDraw();
	void OnBlueInit(int param);
	void OnBlueOwnerDraw();
	void OnRedScoreInit(int param);
	void OnBlueScoreInit(int param);
	void OnRedPangInit(int param);
	void OnBluePangInit(int param);

	FrArea* m_pTeamId;
	FrArea* m_pRed;
	FrArea* m_pBlue;
	const Bitmap* m_pRedEmblem;
	const Bitmap* m_pBlueEmblem;
	const Bitmap* m_pGuildTrophy;
	sGuildRoomInfo m_guildInfo;

	DECLARE_FRESH_MSGMAP()
};

class IContentsDataContainer;
class FrUniteResultDlg : public FrForm
{
	DECLARE_OBJECT(FrUniteResultDlg)

	FrUniteResultDlg();
	virtual ~FrUniteResultDlg();

	virtual bool Close(bool bOK);
	bool CheckChildDlgVisible();
	bool SaveGMEvent();
	bool GetDone() { return m_bDone; }
	void SetTikiReportState(_SYSTEMTIME& time);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float delta);

	void LoadBitmapA();
	void SetResultDlgType();
	void SetDataToRankListBox();
	void FindWinner();
	void SetPrizeResult();
	int GetMedalIndex(const sRivalData* pRival);
	bool IsSameRival(const sRivalData* a, const sRivalData* b);
	bool CanShowPremium10983(const sRivalData* pRival); // 00ae7bf0 (nome meu)
	bool SetTooltip(WRect rect, int index);

	void OnCancelButtonInit(int param);
	void OnAwardInfoInit(int param);
	void OnAwardInfoClick();
	void OnMyInfoAreaInit(int param);
	void OnMyInfoAreaDraw(int param);
	void OnExpGaugeBarInit(int param);
	void OnExpAreaInit(int param);
	void OnExpAreaOwnerDraw(int param);
	void OnRankListBoxInit(int param);
	void OnRankListOwnerDraw(int param);
	void OnRankListLBtnUp();
	void OnRankListRBtnUp();
	void OnDescAreaInit(int param);
	void OnDescAreaOwnerDraw(int param);
	void OnAwardAreaInit(int param);
	void OnAwardAreaOwnerDraw(int param);
	void OnItemAreaInit(int param);
	void OnItemAreaOwnerDraw(int param);
	void OnTeamResultInit(int param);
	void OnTeamResultOwnerDraw(int param);
	void OnTooltipOwnerDraw(int param);
	void OnNextLevelTextInit(int param);

	bool OnAwardInfoDlgResult(int result, FrForm* form);
	bool OnScoreDlgResult(int result, FrForm* form);
	bool OnTreasureScoreDlgResult(int result, FrForm* form);
	bool OnTreasureGiftDlgResult(int result, FrForm* form);

	std::map<unsigned long, int> m_prizeMap;
	struct sUniteResultControls
	{
		// HACK: dllimport + __forceinline: the only way an inline ctor is never emitted out of line;
		// the body is in resultdlg.cpp (lobbymain includes this header: its IL must not grow).
		__declspec(dllimport) __forceinline sUniteResultControls();
		FrButton* m_pCancelBtn;
		FrButton* m_pAwardInfoBtn;
		FrListBox* m_pRankList;
		FrArea* m_pMyInfoArea;
		FrAwardInfoDlg* m_pAwardInfoDlg;
		FrScoreDlg* m_pScoreDlg;
		FrTreasureScore* m_pTreasureScore;
		FrTreasureGift* m_pTreasureGift;
		FrGaugeBar* m_pExpGaugeBar;
		FrArea* m_pExpArea;
		FrArea* m_pDescArea;
		FrArea* m_pAwardArea;
		FrArea* m_pItemArea;
		FrArea* m_pTeamResult;
		FrEdit* m_pNextLevelText;
		tagWTITLEFONT m_angelFontInfo;
		WTitleFont* m_pAngelFont;
		tagWTITLEFONT m_windFontInfo;
		WTitleFont* m_pWindFont;
		const Bitmap* m_pRankBack[3];
		const Bitmap* m_pLevelIcon;
		const Bitmap* m_pEmblem;
		const Bitmap* m_pPcBonus;
		const Bitmap* m_pPremium10; // 983 +0x1bc "premium+10"
		const Bitmap* m_pAssistTitle; // 983 +0x1c0 "assist_title"
		const Bitmap* m_pMascot02;
		const Bitmap* m_pMascot01;
		const Bitmap* m_pMascot09;
		const Bitmap* m_pMascot10;
		const Bitmap* m_pMascot11;
		const Bitmap* m_pMascot12;
		const Bitmap* m_pMascot39;
		const Bitmap* m_pMascot40;
		const Bitmap* m_pBonusMa;
		const Bitmap* m_pBonusMa2;
		const Bitmap* m_pMedal[4];
		const Bitmap* m_pPrizeGray[6];
		const Bitmap* m_pPrize[6];
		const Bitmap* m_pIconZero;
		const Bitmap* m_pCurTooltip;
		const Bitmap* m_pTooltip[7];
	} m_ctrl;
	int m_resultDlgType;
	float m_expTime;
	bool m_bLevelUp;
	int m_level;
	int m_exp;
	int m_maxExp;
	int m_addExp;
	bool m_bDone;
	sRivalData m_winner;
	unsigned long m_selectedUID;
	float m_teamAniTime;
	float m_elapsedTime;
	int m_myRank;
	bool m_bTikiReport;
	_SYSTEMTIME m_tikiReportTime;
	unsigned char m_tikiReportIndex;
	bool m_bRankTooltip;
	bool m_bAwardTooltip;

	DECLARE_FRESH_MSGMAP()
public:
	IContentsDataContainer* m_pHoleInOneData; // 983 +0x450: CContentsDoc::GetContainer(0xaa), lido no OnInit
	const Bitmap* m_pHoleInOneTrophy[3]; // 983 +0x454 holeinone_trophy_gold/silver/bronze
};

class FrAwardInfoDlg : public FrForm
{
	DECLARE_OBJECT(FrAwardInfoDlg)

	FrAwardInfoDlg();
	virtual ~FrAwardInfoDlg();

protected:
	virtual bool OnInit();
	virtual void OnProc(const float delta);

	float m_elapsedTime;

	DECLARE_FRESH_MSGMAP()
};
