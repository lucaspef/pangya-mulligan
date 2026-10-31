#pragma once

#include "frform.h"

class FrComboCtlEx;
#include <vector>

#include "../shared/globalgamedefine.h"

class CSimpleUI;

class FrNextHole : public FrForm
{
	DECLARE_OBJECT(FrNextHole)

	enum eState
	{
	};

	FrNextHole();
	virtual ~FrNextHole();

	virtual bool OnInit();

	void SetState(eState state);
	void SetNextHole(int hole);
	void SetOutFrameVisible(bool bVisible);
	WPoint GetCalcPoint();

protected:
	void OnAreaNextHole_Init(int param);
	void OnAreaNextHole_OwnerDraw(int param);
	void OnArea_OutFrame_Init(int param);

	WOverlay* m_pHoleOverlay[3];
	WRect m_uvRect;
	FrArea* m_pNextHoleArea;
	std::vector<FrArea*> m_outFrames;
	int m_overlayIndex;
	eState m_state;
	int m_texWidth;
	int m_texHeight;

	DECLARE_FRESH_MSGMAP()
};

class FrTreasureGauge : public FrForm
{
	DECLARE_OBJECT(FrTreasureGauge)

	enum eState
	{
	};

	FrTreasureGauge();
	virtual ~FrTreasureGauge();

	virtual bool OnInit();

	void SetState(eState state);
	void SetTPoint();
	void SetOutFrameVisible(bool bVisible);
	WPoint GetCalcPoint();

protected:
	void OnImageBarInit(int param);
	void OnArea_Arrow_Init(int param);
	void OnArea_Arrow_OwnerDraw(int param);
	void OnArea_OutFrame_Init(int param);

	FrGaugeBarImage* m_pImageBar;
	FrArea* m_pArrowArea;
	std::vector<FrArea*> m_outFrames;
	eState m_state;
	const Bitmap* m_pArrowBmp;

	DECLARE_FRESH_MSGMAP()
};

class FrTreasureCourse : public FrForm
{
	DECLARE_OBJECT(FrTreasureCourse)

	enum eALPHA
	{
		eALPHA_INC,
		eALPHA_DEC,
	};

	struct sCloud
	{
		WRect rect;
		float speed;
		int pad14;
		float scale;
		unsigned char alpha;
		bool bEntered;

		sCloud()
			: rect(0.0f, 0.0f, 1.0f, 1.0f),
			  speed(0.0f),
			  pad14(0),
			  scale(0.0f),
			  alpha(0),
			  bEntered(false)
		{
		}
	};

	FrTreasureCourse();
	virtual ~FrTreasureCourse();

	void SetCurMapIdx(unsigned char idx);
	unsigned char GetCurMapIdx();
	bool GetMapIndexChanged() const;
	void SetMapIndexChanged(bool bChanged);
	void SetMapGauge();
	void SelectMapBtn(unsigned char map);
	void SetSelectMap(unsigned char map);

	void LockRandom(bool bLock) { m_bLockRandom = bLock; }
	void EnableRandomBtn(bool bEnable) { m_pRandomBtn->Enable(bEnable); }

protected:
	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void OnAreaSelectMapInit(int param);
	void OnBtnMap_ALL_Init(int param);
	void OnBtnMap_Random_Init(int param);
	void OnViewerF1(int param);
	void OnViewerF2(int param);
	void OnImageBar(int param);
	void OnGauge_SmallBar(int param);
	void OnGauge_SmallBar_OwnerDraw(int param);
	void OnViewer_CurrMap_Init(int param);
	void OnArea_Explane_Init(int param);
	void OnArea_Explane_OwnerDraw(int param);
	void OnInit_Cloud_Area(int param);
	void OnOwnerDraw_Cloud_Area(int param);

	void OnBtnMap_Lagoon_Down();
	void OnBtnMap_Water_Down();
	void OnBtnMap_Sepia_Down();
	void OnBtnMap_Hill_Down();
	void OnBtnMap_WizWiz_Down();
	void OnBtnMap_West_Down();
	void OnBtnMap_Moon_Down();
	void OnBtnMap_Silvia_Down();
	void OnBtnMap_Cannon_Down();
	void OnBtnMap_White_Down();
	void OnBtnMap_Shining_Down();
	void OnBtnMap_Pink_Down();
	void OnBtnMap_Inferno_Down();
	void OnBtnMap_IceSpa_Down();
	void OnBtnMap_SeaWay_Down();
	void OnBtnMap_Panda_Down();
	void OnBtnMap_Wizcity_Down();
	void OnBtnMap_IceInferno_Down(); // 983 008406d0 (demon_land_map)
	void OnBtnMap_AbbotMine_Down();
	void OnBtnMap_Luna_Down();
	void OnBtnMap_Random_Down();

	void OnBtnMap_Lagoon_OwnerDraw(int param);
	void OnBtnMap_Water_OwnerDraw(int param);
	void OnBtnMap_Sepia_OwnerDraw(int param);
	void OnBtnMap_Hill_OwnerDraw(int param);
	void OnBtnMap_WizWiz_OwnerDraw(int param);
	void OnBtnMap_West_OwnerDraw(int param);
	void OnBtnMap_Moon_OwnerDraw(int param);
	void OnBtnMap_Silvia_OwnerDraw(int param);
	void OnBtnMap_Cannon_OwnerDraw(int param);
	void OnBtnMap_White_OwnerDraw(int param);
	void OnBtnMap_Shining_OwnerDraw(int param);
	void OnBtnMap_Pink_OwnerDraw(int param);
	void OnBtnMap_Inferno_OwnerDraw(int param);
	void OnBtnMap_IceSpa_OwnerDraw(int param);
	void OnBtnMap_SeaWay_OwnerDraw(int param);
	void OnBtnMap_Panda_OwnerDraw(int param);
	void OnBtnMap_Wizcity_OwnerDraw(int param);
	void OnBtnMap_IceInferno_OwnerDraw(int param); // 983 0083f840
	void OnBtnMap_AbbotMine_OwnerDraw(int param);
	void OnBtnMap_Luna_OwnerDraw(int param);

private:
	void OpenSelectedMap(unsigned char map);
	void OpenCurrentMap(int map);
	bool OpenGauge(WPoint& pos, int index);
	void DrawCurrentMap(FrGraphicInterface* pGI, int map);
	void DrawMapBtn(eMapType map);
	void InitCloud();
	void ReInitCloud(int index);
	void ProcCloud(float dt);
	void CheckArea(int index, float dt);
	void ShiftAlpha(int index, float dt, eALPHA mode);

protected:
	std::vector<FrButton*> m_mapBtns;
	FrButton* m_pRandomBtn;
	FrArea* m_pSelectMapArea;
	FrViewer* m_pViewerF1;
	FrViewer* m_pViewerF2;
	FrGaugeBarImage* m_pImageBar;
	FrGaugeBarImage* m_pSmallBar;
	WPoint m_gaugePos[22]; // 983: 22
	FrViewer* m_pCurrMapViewer;
	FrArea* m_pExplaneArea;
	FrArea* m_pCloudArea;
	int m_unused1f4; // 983: sem acesso (FrTreasureCourse +0x1f4)
	unsigned char m_curMapIdx;
	unsigned char m_unused1f9; // 983: so zerado no ctor 00841712
	bool m_bMapIndexChanged;
	bool m_bLockRandom;
	unsigned char m_unused1fc; // 983: so zerado no ctor 00841706
	int m_unused200; // 983: sem acesso (+0x200)
	int m_gaugeIndex;
	int m_cloudNum;
	int m_btnIndex;
	const Bitmap* m_pMapBmp[22]; // 983: 22
	const Bitmap* m_pRandomSmallBmp;
	int m_unused26c; // 983: so zerado no ctor 008416cc
	const Bitmap* m_pCloudBmp;
	const Bitmap* m_pGaugeTipBmp;
	const Bitmap* m_pGaugeHighBmp;
	const Bitmap* m_pGaugeLowBmp;
	const Bitmap* m_pMapLockBmp;
	float m_unused284; // 983: so fldz/fst no ctor 008416c6
	float m_unused288; // 983: so fstp no ctor 008416d2
	WRect m_orgRect;
	unsigned long m_gauge[22]; // 983: 22
	WOverlay* m_pCloudOverlay;
	std::vector<sCloud> m_clouds;

	DECLARE_FRESH_MSGMAP()
};

class FrTreasureScore : public FrForm
{
	DECLARE_OBJECT(FrTreasureScore)

	FrTreasureScore();
	virtual ~FrTreasureScore();

	void SetState(bool bShow);
	void SetTPoint();

protected:
	virtual bool OnInit();

	void OnGBarImage_ImageBar_Init(int param);
	void OnView_Backgound_Init(int param);
	void OnArea_Team_Init(int param);

private:
	WPoint GetCalcPoint();
	void IsTeamMode();

protected:
	FrGaugeBarImage* m_pImageBar;
	FrViewer* m_pBackground;
	FrArea* m_pTeamArea;
	bool m_bShow;
	const Bitmap* m_pTeamBlueBmp;
	const Bitmap* m_pTeamRedBmp;

	DECLARE_FRESH_MSGMAP()
};

class CTFrame
{
public:
	CTFrame();
	CTFrame(unsigned char frameCount);
	~CTFrame();

	void AddFrame(FrArea* pFrame);
	bool SetFrameWidth(float width);
	bool SetFrameHeight(float height);

private:
	void Reset()
	{
		if (m_frames.size())
			m_frames.clear();
	}

	unsigned char m_frameCount;
	std::vector<FrArea*> m_frames;
};

class CBoxOpener
{
public:
	enum eSeqProc
	{
		eSEQ_NONE,
		eSEQ_INIT,
		eSEQ_CREATE,
		eSEQ_MYITEM_OPEN,
		eSEQ_COMPLETE,
		eSEQ_BOX_NONE,
		eSEQ_END,
		eSEQ_FINISH,
	};

	CBoxOpener();
	~CBoxOpener();

	bool OnInit();
	void OnProc(const float dt);
	void DrawBoxArea();
	void DrawGiftList(FrListItem* pItem);

	struct sTGift
	{
		int index;
		float frame;
		unsigned long typeId;
		unsigned short quantity;
		bool bMine;
		bool bItem;
		int state;

#ifdef REBANG_TREASUREDLG_CPP
		// HACK: emission order (treasuredlg needs the ctor before Reset); the other
		// includers keep the original body, their COMDAT order depends on this IL.
		sTGift()
		{
			index = -1;
			frame = 0.0f;
			typeId = 0;
			quantity = 0;
			bMine = false;
			bItem = false;
			state = 0;
		}
#else
		sTGift() { Reset(); }
#endif
		void Reset()
		{
			index = -1;
			frame = 0.0f;
			typeId = 0;
			quantity = 0;
			bMine = false;
			bItem = false;
			state = 0;
		}
	};

private:
	void ChangeSequence(eSeqProc seq);
	void ProcInit();
	void ProcCreate();
	void ProcMyItemOpen();
	void ProcComplete();
	void ProcBoxNone();
	void ProcEnd();
	unsigned char DecideTRBox();
	void GetItemOffset(WPoint& pos, unsigned long typeId, bool bIcon);
	sTGift* GetGiftItem(int index);

public:
	std::vector<sTGift> m_gifts;
	FrListBox* m_pGiftList;
	FrArea* m_pBoxArea;
	void (CBoxOpener::*m_pfnProc)();
	CSimpleUI* m_pOpenAni;
	const Bitmap* m_pBoxCloseBmp;
	const Bitmap* m_pBoxOpenBmp;
	const Bitmap* m_pMark1Bmp;
	const Bitmap* m_pMark2Bmp;
	const Bitmap* m_pMsgBmp;
	const Bitmap* m_pBoxBmp[5];
	eSeqProc m_reserveSeq;
	eSeqProc m_nextSeq;
	eSeqProc m_curSeq;
	unsigned char m_boxType;
	int m_unused58; // 983: sem acesso (CBoxOpener +0x58; ctor 00841870)
	int m_progress;
	int m_giftNum;
	int m_slotNum;
	unsigned long m_delay;
	unsigned long m_lastTime;
	float m_dt;
	bool m_bApproach;
	WTitleFont* m_pFont;
};

class FrTreasureGift : public FrForm
{
	friend class
		FrTreasureScore; // HACK: emission order (dead reference in IsTeamMode)
	DECLARE_OBJECT(FrTreasureGift)

	FrTreasureGift();
	virtual ~FrTreasureGift();

	void SetState(bool bShow);
	void SetPointByFrameHeight();
	WPoint GetCalcPoint();

	void SetApproachMode() { m_boxOpener.m_bApproach = true; }

protected:
	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void OnInit_RangeArea_Out(int param);
	void OnInit_RangeArea_In(int param);
	void OnInit_GridArea(int param);
	void OnOwnerDraw_GridArea(int param);
	void OnInit_Gift_List(int param);
	void OnOwnerDraw_Gift_List(int param);
	void OnInit_Box_Area(int param);
	void OnOwnerDraw_Box_Area(int param);

	CTFrame m_outRange;
	CTFrame m_inRange;
	CTFrame m_unusedRange;
	FrArea* m_pGridArea;
	unsigned char m_colNum;
	unsigned char m_rowNum;
	bool m_bShow;
	CBoxOpener m_boxOpener;

	DECLARE_FRESH_MSGMAP()
};

class FrTestDlg_combo : public FrForm
{
	DECLARE_OBJECT(FrTestDlg_combo)

	FrTestDlg_combo();
	virtual ~FrTestDlg_combo();

protected:
	virtual bool OnInit();

	FrComboCtlEx* m_pCombo;

	DECLARE_FRESH_MSGMAP()
};
