#pragma once

#ifndef REBANG_TASKMAIN_PCH // taskmain.cpp includes it after this header (IsOpenAddFriendDlg before GetDlg)
#include "user_info.h"
#endif
#include "anotherhand.h"

struct sUserInfoTime;
class CExhibition;
class CPartTidList;
class FrAddFriendDlg;
class WTitleFont;

class FrUserInfoForm : public FrForm, public CHandOwner
{
	DECLARE_OBJECT(FrUserInfoForm)

	enum eSeasonType
	{
		SEASON_CURRENT,
		SEASON_TOTAL,
	};

	enum eTabType
	{
		TAB_GENERAL,
		TAB_SPECIAL,
		TAB_RECORD,
		TAB_TROPHY,
		TAB_GUILD,
		TAB_NONE,
	};

public:
	FrUserInfoForm();
	virtual ~FrUserInfoForm();
	void SetRequestFriendData(const char*, unsigned long);
	void RecvedSeasonData(unsigned long);
	int IsTotalTab();
	virtual bool Close(eFormRet, bool);
	void RequestSeasonData(unsigned char);
	void Clear();
	bool OnAddFriendFormResult(int, FrForm*);
	void OnCustomProc(float);
	void SetContent(bool);
	void OnTitleSeasonVisible();
	void SetInfo(sUserInfoTime*);

protected:
	void OnCloseUp();
	void SetVisible(FrWnd** const ppWnd, int num, bool bVisible);
	void OnEditInit_HoverOff(int);
	void OnTitleSeasonBtnInit(int);
	void OnTitleTotalBtnInit(int);
	void SetTrophyStatic();
	void OnGeneralInfoInit(int);
	void OnSpecialInfoInit(int);
	void OnRecordInfoInit(int);
	void OnTrophyInfoInit(int);
	void OnGuildInfoInit(int);
	void OnGeneralBackInit(int);
	void OnRecCourseBackInit(int);
	void OnTroNormalBackInit(int);
	void OnTroSpecialBackInit(int);
	void OnGuildBackInit(int);
	void OnAchievementBtnInit983(int); // 983: 00899b60 (vazio)
	void OnAchievementBtnUp983(); // 983: 008a0540 (pacote 0x157)
	void OnEditInit_SI_AWARD_4(int); void OnEditInit_SI_AWARD_5(int); void OnEditInit_SI_AWARD_6(int); // 983: es0_4..6 (0089b4d0..)
	void OnEditInit_RA0(int); void OnEditInit_RA1(int); void OnEditInit_RA2(int); // 983: era0..5 (0089ba50..0089bcd0)
	void OnEditInit_RA3(int); void OnEditInit_RA4(int); void OnEditInit_RA5(int);
	void OnRecordAwardBtnInit(int); void OnRecordAwardBtnUp(); // 983: 0089c900 / 008a0650
	void OnGpNumInit0(int); void OnGpNumInit1(int); void OnGpBtnInit0(int); void OnGpBtnInit1(int); // 983: 0089e5e0..0089e760
	void OnGpBtnUp0(); void OnGpBtnUp1(); // 983: 008a3ca0 / 008a3cc0
	void OnGpBoxInit0(int); void OnGpBoxInit1(int); // 983: 0089e7e0 / 0089e860
	void OnGpNormalBoxOwnerDraw983(int); // 983: 008a6290 "listbox_gp_normal" FRCMD_OWNERDRAW
	void OnGpSpecialBoxOwnerDraw983(int); // 983: 008a0bc0 "listbox_gp_special" FRCMD_OWNERDRAW
	void OnEditInit_GI_ID(int);
	void OnEditInit_GI_SCHOOL(int);
	void OnEditInit_GI_CLUB(int);
	void OnEditInit_GI_EXP(int);
	void OnEditInit_GI_CONNSTATUS(int);
	void OnEditInit_GI_DISCONN(int);
	void OnEditInit_GI_COMBO(int);
	void OnEditInit_GI_QUIZLEVEL(int);
	void OnEditInit_SI_TROPHY_GOLD(int);
	void OnEditInit_SI_TROPHY_SILVER(int);
	void OnEditInit_SI_TROPHY_BRONZE(int);
	void OnEditInit_SI_MATCH_WIN_PERCENTAGE(int);
	void OnEditInit_SI_AVG_SCORE(int);
	void OnEditInit_SI_MEAN_SHOTTIME(int);
	void OnEditInit_SI_PANGYA_PERCENTAGE(int);
	void OnEditInit_SI_FAIRWAY_PERCENTAGE(int);
	void OnEditInit_SI_PUTT_PERCENTAGE(int);
	void OnEditInit_SI_OB_PERCENTAGE(int);
	void OnEditInit_SI_TOTAL_SKINS_PANG(int);
	void OnEditInit_RI_TOT_PLAYTIME(int);
	void OnEditInit_RI_TOT_SHOTS(int);
	void OnEditInit_RI_TOT_HOLES(int);
	void OnEditInit_RI_TOT_DRIVE(int);
	void OnEditInit_RI_EARTH_ROUND(int);
	void OnEditInit_RI_BEST_SCORE_5(int);
	void OnEditInit_RI_BEST_SCORE_4(int);
	void OnEditInit_RI_BEST_SCORE_3(int);
	void OnEditInit_RI_BEST_SCORE_2(int);
	void OnEditInit_RI_BEST_SCORE_1(int);
	void OnEditInit_RI_TOT_HOLEINONES(int);
	void OnEditInit_RI_TOT_ALBATROSS(int);
	void OnEditInit_RI_LONGEST_DRIVE(int);
	void OnEditInit_RI_LONGEST_PUTT(int);
	void OnEditInit_RI_LONGEST_CHIPIN(int);
	void OnEditInit_GUILD_NAME(int);
	void OnEditInit_GUILD_PANG(int);
	void OnEditInit_GUILD_MYPERCENT(int);
	void OnAreaInit_GI_Level(int);
	void OnAreaInit_GI_ClUB(int);
	void OnAreaInit_Question_mark(int);
	void OnAreaDraw_GI_ClUB(int);
	void OnRecordNormalBtnInit(int);
	void OnRecordCourseBtnInit(int);
	void OnRecordClassicBtnInit(int);
	void OnEditInit_SI_MATCH_HISTORY1(int);
	void OnEditInit_SI_MATCH_HISTORY2(int);
	void OnEditInit_SI_MATCH_HISTORY3(int);
	void OnEditInit_SI_MATCH_HISTORY4(int);
	void OnEditInit_SI_MATCH_HISTORY5(int);
	void OnSpecListboxInit(int);
	void OnMatchHistoryBtn1Init(int);
	void OnMatchHistoryBtn2Init(int);
	void OnMatchHistoryBtn3Init(int);
	void OnMatchHistoryBtn4Init(int);
	void OnMatchHistoryBtn5Init(int);
	void OnSpecListboxOwnerDraw(int);
	void OnRecTotalListboxInit(int);
	void OnRecTotalListboxOwnerDraw(int);
	void OnTrophyNormalNumInit(int);
	void OnTrophySpecialNumInit(int);
	void OnTrophyNormalBtnInit(int);
	void OnTrophySpecialBtnInit(int);
	void ChangeRecordType();
	void ChangeTrophyType();
	void OnListBoxInit_TRI_SPECIALBOX(int);
	void OnListBoxInit_TRI_NORMALBOX(int);
	void OnStaticInit_TRI_NOTROPHY(int);
	void OnMyGuildInit(int);
	void OnMyGuildOwnerDraw();
	void OnGuildTrophyListInit(int);
	void OnStaticInit_GUILD_NOTROPHY(int);
	void OnWhisperInit(int);
	void OnNickInit(int);
	void OnNickOwnerDraw(int);
	void OnFriendInit(int);
	void OnIgnoreInit(int);
	void RefreshAllUccClothes();
	void HidePet(bool);
	void OnPetInit(int);
	void OnPetLoadInit(int);
	void ShowPet();
	void OnCommentInit(int);
	void OnEditInit_HoverOn(int);
	void OnScoreListInit(int);
	void OnRecAwardBackInit(int);
	void OnBtn_RmrVisitorInit(int);
	void OnRecordNormalBtnUp();
	void OnMatchHistoryBtn1Up();
	void OnMatchHistoryBtn2Up();
	void OnMatchHistoryBtn3Up();
	void OnMatchHistoryBtn4Up();
	void OnMatchHistoryBtn5Up();
	void OnTrophyNormalBtnUp();
	void OnTrophySpecialBtnUp();
	void OnListBoxOwnerDraw_TRI_SPECIALBOX(int);
	void OnListBoxOwnerDraw_TRI_NORMALBOX(int);
	void OnGuildTrophyListOwnerDraw(int);
	void OnWhisperUp();
	virtual void OnProc(const float delta);
	virtual void OnAnotherHand(int, void*);
	unsigned long GetBestScoreColor(unsigned char);
	unsigned long GetMaxPangColor(unsigned char);
	unsigned long Get30sRecordColor(int);
	unsigned long GetColor(eTabType, unsigned long);
	void OnPetOwnerDraw();
	void SetVisible(eTabType, bool);
	void OnScoreListOwnerDraw(int);
	void OnGhostInit(int);
	void OnGhostdUp();
	void OnBtn_RmrVisitorBtnUp();
	void OnRecordCourseBtnUp();
	void OnRecordClassicBtnUp();
	void OnFriendUp();
	virtual bool OnInit();
	void SetFormA(eTabType);
	void OnGeneralInfoDown();
	void OnSpecialInfoDown();
	void OnRecordInfoDown();
	void OnTrophyInfoDown();
	void OnGuildInfoDown();
	void OnTitleSeasonBtnUp();
	void OnTitleTotalBtnUp();
	void OnIgnoreUp();
	void BuildTotalStat();

private:
	void SetIgnoreButtonImg(bool);
	void SelectContents(eSeasonType, sPangYaUserStatistics**,
		sTrophyStatistics**, std::list<sSpecialTrophy>**,
		std::list<sSpecialTrophy>** /*983: GP 0x34e0/0x3f4c*/,
		std::list<sGuildTrophy>**);
	unsigned long GetSeasonTextColor(eSeasonType);
	void UpdateLevelTextImg(unsigned char);
	void MatchHistoryUserBtnUp(int);
	void MsnDisconExeception(float, FrListItem*);
	void SetMatchHistoryContents();
	void UpdateEditSpecialInfo(eSeasonType, const sPangYaUserStatistics*,
		const sTrophyStatistics*, const std::list<sSpecialTrophy>* = NULL);
	void BuildGpTrophy983(const std::list<sSpecialTrophy>*); // 983 008a7f10
	int FillGpTrophyBox983(); // 983 008a5500
	int FillNormalTrophyBox983(const sTrophyStatistics*); // 983 00899c00
	int FillSpecialTrophyBox983(const std::list<sSpecialTrophy>*); // 983 008a3df0
	int FillGpSpecialTrophyBox983(int, const std::list<sSpecialTrophy>*); // 983 008a3e50
	void SetTrophyNum983(int, int); // 983 008a5ab0
	static bool __stdcall IsGpTrophyOfType983(int, unsigned long); // 983 008a1830 (ret 8)
	static void __stdcall GetGpTrophyGroupName983(int, unsigned long, std::string&); // 983 008a6040 (ret 0xc)
	void GetTrophyTextPos983(int, const WPoint&, WPoint&); // 983 0089f040 (sub-objeto 0x27c)
	static void __stdcall PrintTrophyGroupName983(FrGraphicInterface*, const WPoint&, const std::string&); // 983 008a1890 (ret 0xc)
	void SetDisconntextAlign();
	void SetDisconnQuestionMark(bool);
	void SetSpecListBoxItem(eSeasonType);
	void UpdateEditSchoolAndGuild(eSeasonType, const sPangYaUserInfo*);
	void UpdateEditExpAndMannerInfo(eSeasonType, const sPangYaUserStatistics*);
	void UpdateEditMapStatistics(eSeasonType, bool);
	void MergeMapStatistics983(const sMapStatistics*, const sMapStatistics*); // 983 008a1290
	void MergeSeasonMapStatistics983(const sMapStatistics*, const sMapStatistics*, sMapStatistics**, bool); // 983 008a15b0
	void MergeTotalMapStatistics983(sMapStatistics* const*, sMapStatistics* const*); // 983 008a1420
	void UpdateEditUserLocation();

public:
	typedef std::vector<sSpecialTrophy> sGpTrophy983; // 983: elemento de 16 B da lista de 0x34ec = vector (008a7490 libera +8 e zera +8/+0xc/+0x10); item de 0xc = trofeu GP (008a7f10)
	struct sSeasonStat
	{
		sPangYaUserStatistics statistics;
		sMapStatistics mapStatistics[22]; // 983: 22 (+0x109)
		sTrophyStatistics trophy;
		std::list<sSpecialTrophy> specialTrophy;
		std::list<sGuildTrophy> guildTrophy;
		std::list<sSpecialTrophy> gpSpecialTrophy983; // 983: +0x524, nó de 0x14; nome meu
		void Clear()
		{
			memset(&statistics, 0, sizeof(statistics));
			memset(mapStatistics, 0, sizeof(mapStatistics));
			for (int i = 0; i < 22; ++i) // 983: 22
			{
				mapStatistics[i].cBestScore = 0x7f;
			}
			memset(&trophy, 0, sizeof(trophy));
			specialTrophy.clear();
			guildTrophy.clear();
		}
	};

	void SetMsnControl(bool bSet) { m_bMsnControl = bSet; }
	void SetControlOff(bool bSet) { m_bControlOff = bSet; }

	bool IsOpenAddFriendDlg() const { return m_pAddFriendDlg ? true : false; }

protected:
	eSeasonType m_seasonType;
	eTabType m_tabType;
	FrButton* m_pTitleBtn[2];
	FrButton* m_pTabBtn[5];
	FrArea* m_pBackArea[7];
	FrEdit* m_pGeneralEdit[8];
	FrArea* m_pLevelArea;
	FrArea* m_pClubArea;
	FrArea* m_pQuestionMark;
	WRect m_disconnRect;
	FrEdit* m_pSpecialEdit[19]; // 983: 19; "es0_4".."es0_6" nos índices 3..5 (handlers de Init ainda não escritos)
	FrListBox* m_pSpecListBox;
	const Bitmap* m_pSpecialBack;
	const Bitmap* m_pSpecialBack00;
	FrButton* m_pMatchHistoryBtn[5];
	const Bitmap* m_pRecTotalBack[3];
	const Bitmap* m_pRecTotalBackNd;
	FrListBox* m_pRecTotalListBox;
	FrButton* m_pRecordBtn[4];
	int m_recordType;
	FrEdit* m_pRecordEdit[15];
	FrWnd* m_pRecAwardWnd[6];
	int m_trophyType;
	// 983: sub-objeto de troféus em 0x27c (13 ponteiros; métodos 00899940..00899a2f). PROVISÓRIO: campos soltos na ordem do 983,
	// sem a classe nem os setters fora de linha (onda 2); m_reserved270 saiu
	FrButton* m_pTrophyBtn[4]; // 983: 0x27c; [2], [3] = "btn_gp_normal", "btn_gp_special"
	FrListBox* m_pNormalTrophyBox; // 983: 0x28c
	FrListBox* m_pSpecialTrophyBox; // 983: 0x290
	FrListBox* m_pGpTrophyBox983[2]; // 983: 0x294 "listbox_gp_normal", "listbox_gp_special"; nome meu
	FrEdit* m_pNormalTrophyNum; // 983: 0x29c
	FrEdit* m_pSpecialTrophyNum; // 983: 0x2a0
	FrEdit* m_pGpTrophyNum983[2]; // 983: 0x2a4 "edit_gp_normal", "edit_gp_special"; nome meu
	FrStatic* m_pNoTrophy; // 983: 0x2ac
	FrEdit* m_pGuildEdit[3];
	FrStatic* m_pGuildNoTrophy;
	FrListBox* m_pGuildTrophyList;
	FrArea* m_pMyGuild;
	int m_reserved2a8;
	const Bitmap* m_pGuildIcon;
	unsigned char m_reserved2b0[0x2c]; // 983: 0x2c (PROVISÓRIO: "Btn_rmrvisitor" não existe no mapa do 983; o botão pode ter saído)
	FrButton* m_pRmrVisitorBtn;
	sUserInfo m_userInfo;
	std::list<sSpecialTrophy> m_specialTrophy;
	std::list<sGuildTrophy> m_guildTrophy;
	std::list<sSpecialTrophy> m_gpSpecialTrophy983; // 983: 0x34e0, nó de 0x14 (tipo pelo tamanho; nome meu)
	std::list<sGpTrophy983> m_gpTrophy983; // 983: 0x34ec, nó de 0x18 (elemento de 16 B; nome meu)
	sSeasonStat m_oldSeason;
	sSeasonStat m_totalSeason;
	sUserPosition m_position;
	sMapStatistics m_classicMapStatistics[22]; // 983: 0x3fa3, 22
	sMapStatistics m_oldClassicMapStatistics[22]; // 983: 0x4355
	sMapStatistics m_totalClassicMapStatistics[22]; // 983: 0x4707
	sMapStatistics* m_pBestScoreMap[5]; // 983: 0x4abc
	__int64 m_bestMaxPang983[5]; // 983: 0x4ad0, maior Pang por dificuldade (SortMapStat 008a1ed0); zerado so no construtor
#pragma pack(push, 1)
	struct sMapFlag983 { unsigned long v; unsigned char b; };
#pragma pack(pop)
	sMapFlag983 m_mapFlag983[22]; // 983: 0x4af8, 22 x 5 B zerados (008a77f0); nome meu
	sMapStatistics m_mapStatistics983[4][22]; // 983: 0x4b66, 0x4f18, 0x52ca, 0x567c (4 vetores de 22: [0] atual, [1]/[2] totais por lista, [3] total junto; 008a1d00)
	int m_guildPang;
	int m_guildPoint;
	FrButton* m_pFriendBtn;
	FrButton* m_pWhisperBtn;
	FrButton* m_pIgnoreBtn;
	FrArea* m_pPetArea;
	FrEdit* m_pComment;
	FrArea* m_pNickArea;
	const Bitmap* m_pMaleIcon;
	const Bitmap* m_pFemaleIcon;
	FrListBox* m_pScoreList;
	WTitleFont* m_pScoreFont;
	float m_skinScroll[2];
	int m_skinScrollType[2];
	bool m_bRefresh;
	bool m_bMsnControl;
	bool m_bControlOff;
	float m_petLoadTime;
	bool m_bChatMsgOpen;
	CExhibition* m_pExhibition;
	CPartTidList* m_pPartTidList;
	FrGaugeBar* m_pPetLoadGauge;
	CAnotherHand m_anotherHand;
	bool m_bMatchHistoryRequest;
	bool m_bUnknownLocation;
	float m_delta;
	bool m_bReserved244c;
	sUserMatchHistory m_matchHistory[5];
	FrAddFriendDlg* m_pAddFriendDlg;
	FrButton* m_pGhostBtn;
	unsigned long m_recvSeason;
	int m_requestSeason;
	bool m_bReserved983_5d30; // 983: byte zerado em 008a7c74
	int m_reserved983_5d34[3]; // 983: até o sizeof 0x5d40; não medidos

public:
	int IsRecvSeason(unsigned long season)
	{
		return (m_recvSeason & season) != 0;
	}

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbShowpetFunclets(); // HACK: EH funclets of the naked ShowPet (nakedify)
#endif
};

FrUserInfoForm* USERINFODLG();
