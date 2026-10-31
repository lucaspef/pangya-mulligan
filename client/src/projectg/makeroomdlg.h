#pragma once

#include "frform.h"

class FrTabButton;
class FrStatic;
enum eHoleType;
class FrTreasureCourse;

#include "itemcursor983.h"

class FrMakeRoomDlg : public FrForm
{
	DECLARE_OBJECT(FrMakeRoomDlg)

	FrMakeRoomDlg();
	virtual ~FrMakeRoomDlg();

	virtual bool OnInit();
	virtual void OnProc(const float dt);

	bool MakeRoom(bool bQuick);
	void SetClassicFlags983(unsigned long flags); // 00add000
	void ApplyClassicFlags983(); // 00adcfb0
	void DisableTypeBtn983(unsigned long bit); // 00adcf20

protected:
	void OnMakeVsInit(int param);
	void OnMakeVsDown();
	void OnMakeMassInit(int param);
	void OnMakeMassDown();
	void OnMakeBattleInit(int param);
	void OnMakeBattleDown();
	void OnMakeChatInit(int param);
	void OnMakeChatDown();
	void OnMakeChaosInit(int param);
	void OnMakeChaosDown();
	void OnMakeEventInit(int param);
	void OnMakeEventDown();
	void OnPictureInit(int param);
	void OnTextInit(int param);
	void OnMapCtrlInit(int param);
	void OnMapCtrlTextInit(int param);
	void OnArtifactCtrlInit(int param);
	void OnArtiFrameInit(int param);
	void OnArtiIconInit(int param);
	void OnArtiBanInit(int param);
	void OnArtiNameInit(int param);
	void OnArtiDescInit(int param);
	void OnArtiPrevInit(int param);
	void OnArtiNextInit(int param);
	void OnArtiPrevUp();
	void OnArtiNextUp();
	void OnClassicTypeBtnInit(int param); // 00adea20
	void OnClassicTypeBtnUp(); // 00adeaa0
	void RefreshArtifact983(); // 00adf400
	void RefreshArtiButtons983(); // 00adf9e0
	void SetArtifactArea983(bool bShow, bool bChat); // 00adfa70
	void ApplyTypeMask983(unsigned long mask); // 00ae2460 (mascara 1,2,4,8,0xe); 00adcf20 no modo 0x80
	void OnTitleInit(int param);
	bool OnTitleEnterKey(int param);
	void OnPasswordInit(int param);
	void OnMapPrevInit(int param);
	void OnMapPrevUp();
	void OnMapNextInit(int param);
	void OnMapNextUp();
	void OnMapPrevChatInit(int param);
	void OnMapNextChatInit(int param);
	void SetDefaultTitle();
	void ChangeGameType(unsigned char type, bool bInit);
	void ChangeUserLimit(unsigned char limit);
	void ChangeHole(unsigned char hole);
	bool UpdateArtifact983(bool bUpdateBan, int notifyMode); // 983 00adec00
	void ChangeTimeLimit(unsigned long time);
	void ChangeCourse(unsigned char course);
	void ChangeHoleType(eHoleType type);
	void OnGameTypeInit(int param);
	void OnGameTypeButtonUp();
	void OnGameTypeOwnerDraw(int param);
	void OnUserLimitInit(int param);
	void OnUserLimitButtonUp();
	void OnUserLimitOwnerDraw(int param);
	void OnHoleInit(int param);
	void OnHoleButtonUp();
	void OnHoleOwnerDraw(int param);
	void OnTimeLimitInit(int param);
	void OnTimeLimitButtonUp();
	void OnTimeLimitOwnerDraw(int param);
	void OnCourseInit(int param);
	bool OnMapSelectDlgResult(int result, FrForm* form);
	void OnCourseUp();
	void OnCourseOwnerDraw(int param);
	void OnHoleTypeInit(int param);
	void DrawHoleTypeButton(FrGraphicInterface* gi, eHoleType type, WRect rect,
		bool bEnable);
	void OnHoleTypeOwnerDraw(int param);
	void OnCourseChatInit(int param);
	void OnCourseChatUp();
	void OnHoleChatInit(int param);
	void OnHoleChatOwnerDraw(int param);
	void OnHoleMinInit(int param);
	void OnHoleMinUp();
	void OnHolePrevInit(int param);
	void OnHolePrevUp();
	void OnHoleNextInit(int param);
	void OnHoleNextUp();
	void OnHoleMaxInit(int param);
	void OnHoleMaxUp();
	void OnOverGauge_Init(int param);
	bool IsBtnEnable();

	FrButton* m_pMakeBtn[6]; // 983: [5] = "make_event" 0x124 (00aded00)
	FrViewer* m_pPicture;
	FrEdit* m_pTitle;
	FrArea* m_pText;
	FrEdit* m_pPassword;
	unsigned long m_pad983_138; // 983: sem acesso na classe (00adc000..00ae3000, out/A847)
	FrButton* m_pMapPrev;
	FrButton* m_pMapNext;
	FrButton* m_pMapPrevChat;
	FrButton* m_pMapNextChat;
	FrTabButton* m_pGameType;
	FrTabButton* m_pUserLimit;
	FrTabButton* m_pHole;
	FrTabButton* m_pTimeLimit;
	FrButton* m_pClassicTypeBtn; // 983: "classictype_btn" 0x15c (00adea20); nome meu, tipo não medido
	FrViewer* m_pCourse;
	FrArea* m_pHoleType;
	FrViewer* m_pCourseChat;
	FrArea* m_pHoleChat;
	FrButton* m_pHoleMin;
	FrButton* m_pHolePrev;
	FrButton* m_pHoleNext;
	FrButton* m_pHoleMax;
	FrWnd* m_pMapCtrl; // 983: "map_ctrl" 0x180 (00ade540); nomes meus, tipos não medidos
	FrWnd* m_pMapCtrlText; // "map_ctrl.text_map" 0x184
	FrWnd* m_pArtifactCtrl; // "artifact_ctrl" 0x188
	FrWnd* m_pArtiFrame; // 0x18c
	FrWnd* m_pArtiIcon; // 0x190
	FrWnd* m_pArtiBan; // 0x194
	FrStatic* m_pArtiName; // 0x198
	FrStatic* m_pArtiDesc; // 0x19c
	FrWnd* m_pArtiPrev; // 0x1a0
	FrWnd* m_pArtiNext; // 0x1a4
	FrGaugeBarImage* m_pOverGauge;
	const Bitmap* m_pHoleTypeBtn[4][2];
	const Bitmap* m_pMapSelectO;
	unsigned long m_pad983_1d0; // 983: sem acesso na classe (00adc000..00ae3000, out/A847)
	FrTreasureCourse* m_pMapSelectDlg;
	int m_gameTypeGroup;
	unsigned char m_gameType;
	unsigned char m_userLimit;
	unsigned long m_timeLimit;
	unsigned char m_hole;
	unsigned char m_course;
	unsigned char m_holeType;
	int m_chatHole;
	unsigned long m_gameTime;
	unsigned long m_shotTime;
	float m_courseX;
	float m_btnDelay;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbMakeroomFunclets(); // HACK: EH funclets of the naked MakeRoom (nakedify)
#endif
public:
	CItemCursor983 m_artifact983; // +0x1fc (983 00adead0: lista do CSharedDoc+0x375c)
	unsigned long m_classicFlags; // +0x20c (983 00add000 SetClassicFlags: bit0 -> status do classictype_btn 0x15c); sizeof 0x210
};

namespace nsInvalidCheck
{
	struct _invalidPwd
	{
		bool bInvalid;

		_invalidPwd()
			: bInvalid(false)
		{
		}
		void operator()(char c)
		{
			if (c == ' ')
				bInvalid = true;
		}
	};
}
