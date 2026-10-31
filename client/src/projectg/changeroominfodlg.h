#pragma once

#include "frform.h"
#include "../shared/globalgamedefine.h"

enum eHoleType;
class FrViewer;
class FrTreasureCourse;
class CRepeatHoleFont;

#include "itemcursor983.h" // CItemCursor983 (membro +0x1e0)

// Layout do 983 (sizeof 0x2e0; construtor 00acae20, tabela de mensagens 00e37390 preenchida em 00ced140).
class FrChangeRoomInfoDlg : public FrForm
{
	DECLARE_OBJECT(FrChangeRoomInfoDlg)

	FrChangeRoomInfoDlg();
	virtual ~FrChangeRoomInfoDlg();

	virtual bool OnInit();
	virtual void OnProc(const float dt);

protected:
	void OnTextInit(int param);
	void OnTitleInit(int param);
	void OnPasswordInit(int param);
	void OnMapPrevInit(int param);
	void OnMapPrevUp();
	void OnMapNextInit(int param);
	void OnMapNextUp();
	void OnGameTypeInit(int param);
	void DrawGameTypeButton(FrGraphicInterface* gi, int type, WRect rect,
		bool bEnable);
	void OnGameTypeOwnerDraw(int param);
	void OnUserLimitInit(int param);
	void DrawUserLimitButton(FrGraphicInterface* gi, int limit, WRect rect);
	void OnUserLimitOwnerDraw(int param);
	void OnHoleInit(int param);
	void DrawHoleButton(FrGraphicInterface* gi, int hole, WRect rect);
	void OnHoleOwnerDraw(int param);
	void OnTimeLimitInit(int param);
	void DrawTimeLimitButton(FrGraphicInterface* gi, int time, WRect rect);
	void OnTimeLimitOwnerDraw(int param);
	void OnCourseInit(int param);
	bool OnMapSelectDlgResult(int result, FrForm* form);
	void OnCourseUp();
	void OnCourseOwnerDraw(int param);
	void OnHoleTypeInit(int param);
	void DrawHoleTypeButton(FrGraphicInterface* gi, eHoleType type, WRect rect,
		bool bEnable);
	void OnHoleTypeOwnerDraw(int param);
	void OnOkBtnUp();
	void ChangeGameType(int type, bool bUnused, bool bKeepValues);
	void ChangeUserLimit(int limit);
	void ChangeHole(int hole);
	void ChangeTimeLimit(int time);
	void ChangeCourse(int course);
	void ChangeHoleType(eHoleType type);
	bool IsBtnEnable();
public:
	void SetRepeat983(bool bFixHole, unsigned char holecupPos, CRepeatHoleFont* pFont); // 00acac50 (B43: gameroomdlg)
protected:
	bool CheckArtifact983(bool bUpdateUI, int notify); // 00acb620: artefato escolhido nao serve para a sala (00479bc0)
	void OnSingleCtrlInit983(int param); // 00aca360 "cover" e "single_ctrl"
	void OnRepeatInit983(int param); // 00aca3f0 "single_ctrl.repeat"
	void OnFixHoleInit983(int param); // 00aca480 "single_ctrl.repeat.fix_hole"
	void OnFixHolecupPosInit983(int param); // 00aca5b0 "single_ctrl.repeat.fix_holecup_pos"
	void OnFixHolecupPosUp983(); // 00aca640
	void OnFixHoleUp983(); // 00acb550
	void OnFixHoleOwnerDraw983(); // 00aca510
	bool OnHoleSelected983(int ret, FrForm* pForm); // 00acadc0
	void RefreshFixHolecupPos983(); // 00ac9300: single_font_on/off nos 3 modos
	void OnMapCtrlInit983(int param); // 00aca660 "map_ctrl"
	void OnTextMapInit983(int param); // 00aca710 "map_ctrl.text_map"
	void OnArtifactCtrlInit983(int param); // 00aca780 "artifact_ctrl"
	bool ArtifactShow983(bool bShow); // 00ac9380
	void RefreshArtifact983(); // 00acb9c0
	void RefreshArtiButtons983(unsigned long tid); // 00acc0a0
	void SetArtifactArea983(bool bShow, bool bArtifact); // 00acc120
	void SetRoomFlags983(unsigned long flags); // 00ac9270: grava +0x1d4 e acende o classictype_btn (bit 0)
	void OnArtiFrameInit983(int param); // 00aca7f0 "artifact_ctrl.arti_frame"
	void OnArtiIconInit983(int param); // 00aca860 "artifact_ctrl.arti_icon"
	void OnArtiBanInit983(int param); // 00aca8d0 "artifact_ctrl.arti_ban"
	void OnArtiNameInit983(int param); // 00aca940 "artifact_ctrl.arti_name"
	void OnArtiDescInit983(int param); // 00aca9b0 "artifact_ctrl.arti_desc"
	void OnArtiPrevInit983(int param); // 00acaa20 "artifact_ctrl.arti_prev"
	void OnArtiPrevUp983(); // 00acbfe0
	void OnArtiNextInit983(int param); // 00acaab0 "artifact_ctrl.arti_next"
	void OnArtiNextUp983(); // 00acc040
	void OnClassicTypeInit983(int param); // 00acab40 "classictype_btn"
	void OnClassicTypeUp983(); // 00acac20

	FrEdit* m_pTitle; // +0x110
	FrEdit* m_pPassword; // +0x114
	FrArea* m_pText; // +0x118
	FrWnd* m_pMapCtrl; // +0x11c (983: so zerado no construtor e lido no ChangeGameType; o "map_ctrl" grava +0x150)
	FrButton* m_pMapPrev; // +0x120
	FrButton* m_pMapNext; // +0x124
	FrArea* m_pGameType; // +0x128
	FrArea* m_pUserLimit; // +0x12c
	FrArea* m_pHole; // +0x130
	FrArea* m_pTimeLimit; // +0x134
	FrViewer* m_pCourse; // +0x138
	FrArea* m_pHoleType; // +0x13c
	FrWnd* m_pSingleCtrl983; // +0x140 "single_ctrl" (e "cover", mesmo tratador 00aca360)
	FrWnd* m_pRepeat983; // +0x144 "single_ctrl.repeat"
	FrWnd* m_pFixHole983; // +0x148 "single_ctrl.repeat.fix_hole"
	FrWnd* m_pFixHolecupPos983; // +0x14c "single_ctrl.repeat.fix_holecup_pos"
	FrWnd* m_pMapCtrl983; // +0x150 "map_ctrl"
	FrWnd* m_pTextMap983; // +0x154 "map_ctrl.text_map"
	FrWnd* m_pArtifactCtrl983; // +0x158 "artifact_ctrl"
	FrWnd* m_pArtiFrame983; // +0x15c "artifact_ctrl.arti_frame"
	FrWnd* m_pArtiIcon983; // +0x160 "artifact_ctrl.arti_icon"
	FrWnd* m_pArtiBan983; // +0x164 "artifact_ctrl.arti_ban"
	FrWnd* m_pArtiName983; // +0x168 "artifact_ctrl.arti_name"
	FrWnd* m_pArtiDesc983; // +0x16c "artifact_ctrl.arti_desc"
	FrWnd* m_pArtiPrev983; // +0x170 "artifact_ctrl.arti_prev"
	FrWnd* m_pArtiNext983; // +0x174 "artifact_ctrl.arti_next"
	FrWnd* m_pClassicTypeBtn983; // +0x178 "classictype_btn"
	const Bitmap* m_pSelectBtnN; // +0x17c
	const Bitmap* m_pSelectBtnO; // +0x180
	const Bitmap* m_pSelectBtnD; // +0x184
	const Bitmap* m_pHoleTypeBtn[4][2]; // +0x188
	const Bitmap* m_pMapSelectO; // +0x1a8
	FrTreasureCourse* m_pMapSelectDlg; // +0x1ac
	int m_gameTypeGroup; // +0x1b0
	int m_gameType; // +0x1b4
	int m_userLimit; // +0x1b8
	int m_timeLimit; // +0x1bc
	int m_hole; // +0x1c0
	int m_course; // +0x1c4
	unsigned char m_holeType; // +0x1c8
	long m_gameTime; // +0x1cc (983: com sinal; imul/sar no OnOkBtnUp)
	long m_shotTime; // +0x1d0
	unsigned long m_roomFlags983; // +0x1d4 (00ac9270; bit 0 servico 0x20000000, bit 1 forca o tipo 4; pacote 0xa campo 0xe; sala +0xc9)
	unsigned char m_fixHole983; // +0x1d8 (CSharedDoc 00633580; pacote 0xa campo 0xb)
	bool m_bFixHolecupPos983; // +0x1d9 (00aca640; pacote 0xa campo 0xc: 7/6)
	float m_btnDelay; // +0x1dc
	CItemCursor983 m_artifact983; // +0x1e0 (lista do CSharedDoc+0x375c; pacote 0xa campo 0xd; sala +0xc5)
	bool m_bRepeat983; // +0x1f0 (SetRepeat983)
	sRoomInfo m_roomInfo; // +0x1f1 (0xdd, empacotada)
	CRepeatHoleFont* m_pRepeatHoleFont983; // +0x2d0 (SetRepeat983)
	char m_pad983_2d4[0xc]; // +0x2d4 (983: push 0x2e0 em 00acb783/005028aa; scan 00ac9000-00acd600 e vtables d4a55c/4c/44 sem +0x2d4..+0x2df)

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnokbtnupFunclets(); // HACK: EH funclets of the naked OnOkBtnUp (nakedify)
#endif
};

struct FrChangeRoomInfoInvalidPwd
{
	bool bInvalid;

	FrChangeRoomInfoInvalidPwd()
		: bInvalid(false)
	{
	}
	void operator()(char c)
	{
		if (c == ' ')
			bInvalid = true;
	}
};
