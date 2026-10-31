#pragma once

#include "frform.h"

class CSimpleUI;
#include <string>
#include <vector>
#include <stdlib.h>

class CGDTextBox;
class CGDBitmapBox;
class FrBirthdayEventHelpDlg;

struct MissionData
{
	int bComplete;
	unsigned long value;
	MissionData()
	{
		bComplete = 0;
		value = 0;
	}
};
struct BoardInfo
{
	WRect boardRect;
	WRect numberRect;
	const Bitmap* pNumberBmp;
};
class FrBirthdayEventDlg : public FrForm
{
	DECLARE_OBJECT(FrBirthdayEventDlg)

	FrBirthdayEventDlg();
	virtual ~FrBirthdayEventDlg();

	virtual bool OnInit();
	virtual void OnMouseMove(const WPoint& pos);

	void SetRemainGiftCount(unsigned long count);
	void SetEventName(std::string& name);
	void SetMissionData(unsigned long completeMask,
		unsigned long remainGiftCount);
	void SetMissionInfo(std::vector<unsigned long>& missions,
		std::vector<unsigned long>& values);

protected:
	virtual void OnProc(const float dt);

	void OnInitGiftButton(int param);
	void OnOwnerDrawBoardArea(int param);
	void OnInitBoardNumberArea(int param);
	void OnInitBoardBG1Area(int param);
	void OnLButtonDownHelpButton();
	void OnLButtonDownGiftButton();

private:
	void InitalizeCoordinates();
	void InitalizeImages();
	bool OnKoohBirthdayEventHelpDlg(int result, FrForm* form);
	char* MakeMapInfoText(int mission, char* buf);
	void MakeToolTipText(int mission);
	void CheckToolTip(const WPoint& pos);

	FrBirthdayEventHelpDlg* m_pHelpDlg;
	FrButton* m_pHelpBtn; // 983: "Help" FRCMD_INIT 0x114 (00b12c10); nome meu
	FrButton* m_pGiftBtn;
	FrArea* m_pBoardNumberArea;
	FrArea* m_pBoardBG1Area;
	CGDTextBox* m_pTextBox;
	CGDBitmapBox* m_pBitmapBox;
	div_t m_giftCount;
	float m_clickOffset;
	unsigned long m_remainGiftCount;
	unsigned long m_boardIndex;
	std::string m_eventName;
	MissionData m_mission[16];
	const Bitmap* m_pDateBmp;
	const Bitmap* m_pClickBmp;
	CSimpleUI* m_pGiftNumberUI;
	CSimpleUI* m_pBoardUI;
	BoardInfo m_board[16];
	WRect m_giftNumberRect;
	WRect m_dateRect;
	WRect m_clickRect;

	static sFRESH_ENTRY _MsgEntries[];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;
public:
	char m_pad983_fim[0x1f0]; // 983: sizeof 0x650 (push em 00b136a0). Classe do 983 = evento kooh2013, layout outro (ctor 00b13500: +0x130 float, +0x138 uid%10, std::string +0x13c, 25x12 bytes +0x158); codigo morto no 983 (D615), nao portado; out/A835
};

class FrBirthdayEventHelpDlg : public FrForm
{
	DECLARE_OBJECT(FrBirthdayEventHelpDlg)

	FrBirthdayEventHelpDlg();
	virtual ~FrBirthdayEventHelpDlg();

	void OpenHelpDlg(const char* filename);

protected:
	void OnInitHelpView(int param);

	FrViewer* m_pHelpView;

	DECLARE_FRESH_MSGMAP()
};

class FrBirthdayEventMissionDlg : public FrForm
{
	DECLARE_OBJECT(FrBirthdayEventMissionDlg)

	FrBirthdayEventMissionDlg();
	virtual ~FrBirthdayEventMissionDlg();

	virtual bool OnInit();

	void SetCompletedMissionInfo(unsigned long mission);
	void CalculatePos(unsigned long index, unsigned long count);

	void SetFadeOutFlag(bool bFadeOut) { m_bFadeOut = bFadeOut; }

protected:
	virtual void OnProc(const float dt);

	void OnInitMissionNumberArea(int param);

	FrArea* m_pMissionNumberArea;
	const Bitmap* m_pMissionNumberBmp[16];
	unsigned long m_completedMission;
	float m_fadeOutTime;
	bool m_bFadeOut;

	DECLARE_FRESH_MSGMAP()
public:
	char m_pad983_fim[0x30]; // 983: sizeof 0x190 (push em 00b130b0). Ctor 00b126b0: 25 bitmaps "event_kooh2013_num2_%d" em +0x114, +0x178/+0x17c = 0, byte +0x180; codigo morto no 983 (D615), nao portado; out/A835
};
