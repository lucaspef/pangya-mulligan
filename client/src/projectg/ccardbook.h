#pragma once
#include "vc10_layout.h"

#include <vector>
#include "frform.h"
#include "wresrcmng.h"
#include "wfont.h"

class Bitmap;
class FrArea;
class FrButton;
class FrListBox;
class FrGraphicInterface;
class WOverlay;
struct IFF_ITEM_COMMON;
namespace IFF_STRUCT
{
	struct sCard;
}

class CRealMyRoomForm983;
class FrSpCardUseDlg;
class FrCardUseDlg;
class FrOpenCardPackDlg;
class FrItemAttachCardDlg;

class CCardSlot : public FrCmdTarget
{
public:
	CCardSlot();
	virtual ~CCardSlot();

	unsigned char IsPushed() const { return m_state; }
	void Select(unsigned char state);
	void SetPosition(WRect& rect);
	WRect GetPosition() const { return m_rect; }
	bool BoundControl(FrButton* pButton);
	void SetCard(unsigned long tid);
	void UseCard();
	void Draw(FrGraphicInterface* pGDI);
	void Show(bool bShow)
	{
		if (m_pButton && m_pCard && m_pCommon)
			m_pButton->SetVisible(bShow);
	}
	IFF_STRUCT::sCard* GetCardInfo() { return m_pCard; }
	sCards* GetCards() { return m_pItem; }

protected:
	bool OnCardPackDlgResult(int result, FrForm* pForm);
	bool OnSpCardDlgResult(int result, FrForm* pForm);
	bool OnCardDlgResult(int result, FrForm* pForm);
	bool OnItemAttachCardDlgResult(int result, FrForm* pForm);

public:
	unsigned char m_state;
	_WRECT m_rect;
	FrButton* m_pButton;
	IFF_STRUCT::sCard* m_pCard;
	IFF_ITEM_COMMON* m_pCommon;
	sCards* m_pItem;
	WOverlay* m_pOverlay;
	FrOpenCardPackDlg* m_pOpenCardPackDlg;
	FrCardUseDlg* m_pCardUseDlg;
	FrItemAttachCardDlg* m_pItemAttachCardDlg;
	FrSpCardUseDlg* m_pSpCardUseDlg;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnitemattachcardFunclets(); // HACK: EH funclets of the naked OnItemAttachCardDlgResult (nakedify)
#endif
};

class ICardBookPage
{
public:
	ICardBookPage()
	{
		m_pSlots = NULL;
		m_pNumberFont = NULL;
		// HACK: the original numbers the local static as scope 4.
		{
			{
				{
					static char* strBonusNumber = "[font_wind.jpg";
					WTITLEFONT info;
					memset(&info, 0, sizeof(info));
					info.filename = &strBonusNumber;
					info.fontw = 16;
					info.fonth = 16;
					info.numPages = 1;
					info.texw = 128;
					info.texh = 64;
					info.pCharSet = "1234567890ym%-.?/:";
					m_pNumberFont = g_resrcmng->GetTitleFont();
					m_pNumberFont->Create(&info);
				}
			}
		}
	}
	virtual ~ICardBookPage()
	{
		if (m_pSlots)
		{
			delete[] m_pSlots;
			m_pSlots = NULL;
		}
		m_pNumberFont->Erase();
		if (g_resrcmng && m_pNumberFont)
		{
			g_resrcmng->Release(m_pNumberFont);
			m_pNumberFont = NULL;
		}
	}

	virtual bool Init() = 0;
	virtual void Draw(FrGraphicInterface* pGDI) = 0;
	virtual void SetCard(unsigned char slot, unsigned long tid) { }
	virtual void ShowSlotControl(bool bShow) = 0;
	virtual void SelectSlot(unsigned char type, unsigned char slot) = 0;
	virtual bool IsSelectedSlot() = 0;

	bool BoundSlotControl(unsigned char slot, FrButton* pButton)
	{
		bool bResult = false;
		if (m_pSlots)
			bResult = m_pSlots[slot].BoundControl(pButton);
		return bResult;
	}
	CCardSlot* GetCardSlot() { return m_pSlots; }

	CCardSlot* m_pSlots;

protected:
	WTitleFont* m_pNumberFont;
};

class CCardPackPage : public ICardBookPage
{
public:
	CCardPackPage() { m_pCardPackList = NULL; }
	virtual ~CCardPackPage()
	{
		if (m_pCardPackList)
		{
			delete m_pCardPackList;
			m_pCardPackList = NULL;
		}
	}

	virtual bool Init();
	virtual void Draw(FrGraphicInterface* pGDI);
	virtual void SetCard(unsigned char slot, unsigned long tid);
	virtual void ShowSlotControl(bool bShow);
	virtual void SelectSlot(unsigned char type, unsigned char slot);
	std::vector<unsigned long>* GetCardPackList() { return m_pCardPackList; }

	virtual bool IsSelectedSlot()
	{
		bool bSelected = false;
		for (int i = 0; i < 8; i++)
		{
			if (m_pSlots && m_pSlots[i].IsPushed() == 1)
			{
				bSelected = true;
				break;
			}
		}
		return bSelected;
	}

protected:
	std::vector<unsigned long>* m_pCardPackList;
};

class CCardPage : public ICardBookPage
{
public:
	CCardPage();
	virtual ~CCardPage();

	virtual bool Init();
	virtual void Draw(FrGraphicInterface* pGDI);
	virtual void SetCard(unsigned char slot, unsigned long tid);
	virtual void ShowSlotControl(bool bShow);
	virtual void SelectSlot(unsigned char type, unsigned char slot);
	virtual bool IsSelectedSlot()
	{
		bool bSelected = false;
		for (int i = 0; i < 12; i++)
		{
			if (m_pSlots && m_pSlots[i].IsPushed() == 1)
			{
				bSelected = true;
				break;
			}
		}
		return bSelected;
	}

	std::vector<unsigned long>* GetCardList(unsigned char volume);
	unsigned long GetPageCount(unsigned char volume);

protected:
	void BuildCardList();
	void PageSeparate(std::vector<unsigned long>*& pList, unsigned char volume);

public:
	unsigned char m_volume;
	unsigned char m_page;
	std::vector<unsigned long>* m_pCardList[6]; // 983: volumes 1..5 (0x10..0x20) + um slot extra (0x24)
	const Bitmap* m_pBackground;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbPageseparateFunclets(); // HACK: EH funclets of the naked PageSeparate (nakedify)
#endif
};

#ifndef REBANG_CARDDLG_CPP // card dialogs: carddlg.cpp sees the full definitions of carddlg.h; here only the members
// used by the card book are declared here.
class FrSpCardUseDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrSpCardUseDlg)

	FrSpCardUseDlg();
	virtual ~FrSpCardUseDlg();

	void SetSpCardInfo(unsigned long& info);
	unsigned long& GetSpCardTid() { return m_spCardTid; }

protected:
	unsigned long m_unused110;
	unsigned long m_spCardTid;
	unsigned char m_unused118[0x8]; // 983 +0x118 m_pItem, +0x11c m_pText1 (C694: sem text2)

	DECLARE_FRESH_MSGMAP()
};

class FrCardUseDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrCardUseDlg)

	FrCardUseDlg();
	virtual ~FrCardUseDlg();

	unsigned long m_guid;
	unsigned long m_tid;

protected:
	unsigned char m_unused118[0x10];

	DECLARE_FRESH_MSGMAP()
};

class FrOpenCardPackDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrOpenCardPackDlg)

	FrOpenCardPackDlg();
	virtual ~FrOpenCardPackDlg();

	unsigned long m_guid;
	unsigned long m_tid;

protected:
	unsigned char m_unused118[0x8];

	DECLARE_FRESH_MSGMAP()
};

class FrItemAttachCardDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrItemAttachCardDlg)

	FrItemAttachCardDlg();
	virtual ~FrItemAttachCardDlg();

	unsigned long m_cardTid;
	unsigned long m_cardGuid;
	unsigned long m_cardType;
	unsigned long m_partTid;
	unsigned long m_partGuid;
	int m_slot;

protected:
	unsigned char m_unused128[0x288]; // A234: era 0x268 (sizeof 0x390): realmyroommainui fazia new de 0x390 e o ctor (carddlg) escreve 0x3b0

	DECLARE_FRESH_MSGMAP()
};
// A234: as declarações parciais têm de ter o sizeof das completas (carddlg.h) — o new é feito por quem inclui este .h
// (983: new 0x120 FrSpCardUseDlg 00801433; C694: nosso 0x120 sem m_pText2/m_unused124), 0x130 FrCardUseDlg, 0x120 FrOpenCardPackDlg, 0x3b0 FrItemAttachCardDlg 008086d3
C_ASSERT_VC10(sizeof(FrSpCardUseDlg) == 0x120 && sizeof(FrCardUseDlg) == 0x130 && sizeof(FrOpenCardPackDlg) == 0x120);
C_ASSERT_VC10(sizeof(FrItemAttachCardDlg) == 0x3b0 && offsetof(FrItemAttachCardDlg, m_cardTid) == 0x110 && offsetof(FrItemAttachCardDlg, m_slot) == 0x124);
#endif
class CCardBook
{
public:
	CCardBook();
	virtual ~CCardBook();

	void Show(bool bShow);
	void Draw();
	void DrawBackground(); // 007f33b0
	void ShowSlotControl(bool bShow);
	void UseCard();
	void CardSlotControl(unsigned char page, unsigned char slot,
		unsigned char type);
	void ButtonTabControl(unsigned char tab);
	void BuildCardTab(unsigned long volume);
	void BuildCardPackTab();
	void BuildCardPage(unsigned long page);
	void ReBuildCurrentPage();
	void PageIndexBehavior(const WPoint& pos, unsigned char type);
	void DrawCardPackHelp(unsigned long tid);

	template <class T>
	bool BoundControl(T& pControl, T pWnd)
	{
		pControl = pWnd;
		pControl->SetVisible(false);
		if (pControl != NULL)
			return true;
		return false;
	}
	CCardPackPage* GetCardPackPage() { return (CCardPackPage*)m_pPages[0]; }
	CCardPage* GetCardPage() { return (CCardPage*)m_pPages[1]; }
	void SetSelectedSlot(unsigned char slot) { m_selectedSlot = slot; }
	bool IsShow() const { return m_bShow; }

protected:
	void ClearVariables();
	void LoadImages();
	void ShowCardProperty(bool bShow);
	void DrawCard(FrGraphicInterface* pGDI);
	void DrawPageIndex(FrGraphicInterface* pGDI);

	bool m_bShow;
	bool m_bShowProperty;
	unsigned char m_tab;
	unsigned char m_pageCount;
	unsigned char m_curPage;
	unsigned char m_hoverPage;
	unsigned char m_selectedSlot;
	std::vector<unsigned long> m_unusedList;
	std::vector<unsigned long>* m_pCurCardList;
	std::vector<WRect> m_pageIndexRects;
	ICardBookPage* m_pPages[2];
	const Bitmap* m_pBgBitmap[4];
	const Bitmap* m_pCardPackBitmap[4];
	const Bitmap* m_pCollectionBitmap[4];
	FrArea* m_pBackground;
	FrArea* m_pProperty;
	FrButton* m_pTabButton[7]; // card pack tab + volumes (983: 7 botoes, 0x70..0x88)
	FrButton* m_pUseButton; // 983: +0x8c
	FrListBox* m_pIndexList;

	friend class CRealMyRoomMain; // binds the controls (BoundControl)
	friend class CRealMyRoomForm983; // My Room do 983 (jp/myroomwidget.cpp)
};
