#pragma once

#include "frform.h"
#include "bitmap.h"
#include <vector>

class FrArea;
class FrListBox;
class FrButton;
class FrEdit;
class FrTextButton;
class FrViewer;
class WTitleFont;

struct sBonusBall
{
	unsigned long unused0;
	unsigned long typeId;
	unsigned long guid;
	int count;
	unsigned char unused10;
	unsigned char kind;
	unsigned short value;
};

class FrScratchResultDlg : public FrForm
{
	DECLARE_OBJECT(FrScratchResultDlg)

public:
	FrScratchResultDlg();
	virtual ~FrScratchResultDlg();

	void SetResultItem(sBonusBall* pBall, int count);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float deltaTime);

	void OnScratchResultDlgInit(int param);
	void OnResultItemIconInit(int param);
	void OnResultItemIconOwnerDraw(int param);
	void OnResultItemNameInit(int param);
	void OnResultItemName2ndInit983(int param);
	void OnMsg01Init983(int param);
	void OnMsg02Init983(int param);
	void OnOkInit983(int param);
	void AdjustLine(int count, FrWnd* pWnd);
	void SetResultName983(int line, int index);

	FrListBox* m_pIcon;		// +0x110
	int m_curIndex983;		// +0x114 linha do topo da lista ja escrita (-1 no ctor)
	FrEdit* m_pName[2];		// +0x118 resultitem_name, +0x11c resultitem_name_2nd
	FrWnd* m_pWnd120;		// +0x120
	FrWnd* m_pWnd124;		// +0x124 (o 983 nao inicializa)
	FrWnd* m_pWnd128;		// +0x128
	WTitleFont* m_pFont;	// +0x12c
	std::vector<sBonusBall> m_balls;	// +0x130
	int m_count;			// +0x140
	int m_pad983_144[3]; // 983: sem leitura/escrita (ctor 008eaaf0 nao zera; +0x144..0x14c na regiao sao de FrScratchDlg/FrTradeEditDlg); sizeof 0x150 (008eb243)

	DECLARE_FRESH_MSGMAP()
};

class FrScratchDlg : public FrForm
{
	DECLARE_OBJECT(FrScratchDlg)

public:
	FrScratchDlg();
	virtual ~FrScratchDlg();

	void SetEnableRequestScratchItem(bool bEnable);
	void EnableNextPageButton(bool bEnable);
	void EnableScratchSerialNumber(bool bEnable);
	void ClearScratchItem();
	void ClearDataCheckScratchItem();
	void CheckScratchItemRect(const WPoint& pos);
	void ProcessChangeScratchCardNumber(float deltaTime);
	void InitChangeScratchCardNumber(int index);
	void ResetBmpScratchCover(int index);
	void OpenScratchCover(const WPoint& pos);
	void RequestScratchItem();
	void SetRectScratchItem();
	void SetRectCheckScratchItemRectByIndex(int index, WPoint offset);
	void DrawScratchItem();
	void DrawChangeScratchCardNumber();
	void SetBmpColsedScratchCover();
	void ProcessResultText(float deltaTime);
	void SetBmpScratchItem();
	void SetRectCheckScratchItemRect();
	bool OnConfirmResult(int result, FrForm* form);
	void InitScratchCard();
	void OpenScratchResultDlg();
	void SetBmpResultText(int index);
	void ProcessCheckScratchItemRect(float deltaTime);
	void SendScratchSerialNumber();
	void UpdateListMyItem(sBonusBall* pBall);
	void ResponseScratchSerialNumber(int result, unsigned long guid);
	void SetScratchItem(int count, sBonusBall* pBall);
	void GetItemStartPos983(WPoint* pPos);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float deltaTime);
	virtual void OnMouseMove(const WPoint& pos);
	virtual bool OnLButtonDown(const WPoint& pos);
	virtual bool OnLButtonUp(const WPoint& pos);
	virtual void OnCancel();

	void OnScratchDlgInit(int param);
	void OnScratchBottomInit(int param);
	void OnScratchBottomOwnerDraw();
	void OnScratchCover01Init(int param);
	void OnScratchCover01OwnerDraw();
	void OnScratchCover02Init(int param);
	void OnScratchCover02OwnerDraw();
	void OnScratchCardInit(int param);
	bool OnScratchCardEnterKey(int param);
	void OnScratchNextPageBnInit(int param);
	void OnScratchNextPageBnUp();
	void OnSerialNumberInit(int param);
	bool OnSerialNumberEnterKey(int param);
	void OnSerialNumberBnInit(int param);
	void OnSerialNumberBnUp();
	void OnAboutBnInit(int param);
	void OnAboutBnUp();
	void OnCloseBnInit(int param);
	void OnCloseBnUp();

	FrArea* m_pCover[2];
	FrEdit* m_pScratchCard;
	FrButton* m_pNextPage;
	FrEdit* m_pSerial;
	FrButton* m_pSerialBn;
	WTitleFont* m_pFont;
	const Bitmap* m_pCoverBmp[2];
	const Bitmap* m_pCloseCover;
	Bitmap m_bmpCover[2];
	const Bitmap* m_pEraser;
	const Bitmap* m_pMinus;
	const Bitmap* m_pPlus;
	// 983: os itens viraram listas (ate 11 itens em fila; construtor 008ec050)
	std::vector<const Bitmap*> m_pItemBmp;	// +0x16c
	std::vector<WRect> m_rcItem;			// +0x17c
	WRect m_rcEraser;
	WPoint m_ptOffset;
	bool m_bPressed;
	int m_scratchCardCount;
	sBonusBall m_ball[11];
	int m_ballCount;
	int m_response;
	bool m_bEnableRequest;
	bool m_bScratchEnabled;
	bool m_bSerialEnabled;
	bool m_bResultPending;
	int m_changeIndex[2];
	float m_changePos[2];
	int m_changeAlpha[2];
	float m_resultTimer;
	std::vector<std::vector<bool> > m_bChecked;		// +0x2b0 (4 quadrantes por item)
	std::vector<std::vector<WRect> > m_checkRect;	// +0x2c0

	DECLARE_FRESH_MSGMAP()
};

class FrScratchAboutDlg : public FrForm
{
	DECLARE_OBJECT(FrScratchAboutDlg)

public:
	FrScratchAboutDlg();
	virtual ~FrScratchAboutDlg();

	void SetImgAboutIndex(int index);

protected:
	virtual bool OnInit();

	void OnAboutViewerInit(int param);
	void OnIndex01Init(int param);
	void OnIndex01BnUp();
	void OnIndex02Init(int param);
	void OnIndex02BnUp();
	void OnIndex03Init(int param);
	void OnIndex03BnUp();
	void OnIndex04Init(int param);
	void OnIndex04BnUp();
	void OnIndex05Init(int param);
	void OnIndex05BnUp();
	void OnBackwardBnInit(int param);
	void OnBackwardBnUp();
	void OnForwardBnInit(int param);
	void OnForwardBnUp();
	void OnOkBnInit(int param);
	void OnOkBnUp();

	FrTextButton* m_pIndex[5];
	int m_index;
	FrButton* m_pBackward;
	FrButton* m_pForward;
	FrViewer* m_pViewer;
	unsigned long m_unused134;

	DECLARE_FRESH_MSGMAP()
};
