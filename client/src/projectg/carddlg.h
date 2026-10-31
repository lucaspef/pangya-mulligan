#pragma once
#ifndef REBANG_CARDDLG_CPP // HACK: other includers keep the partial declarations (same IL as before, see ORDER_MODEL)
#include <list>
#include "frform.h"

struct sSCardAvilityPeriodInfo;
class FrListBox;

class FrCardBoxDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrCardBoxDlg)
	FrCardBoxDlg();

	virtual void SetCardInfo(unsigned long tid, unsigned long guid,
		unsigned long arg);

	// Resto da classe do 983 sem acesso no nosso codigo: so reserva os bytes.
	char m_pad110[0x58];
};

class FrSPCardBuffDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrSPCardBuffDlg)

	FrSPCardBuffDlg();
	virtual ~FrSPCardBuffDlg();

	virtual bool OnInit();
	void SetBuffItem(const std::list<sSCardAvilityPeriodInfo>& cards,
		bool bClear);
	const WPoint& GetPosition() { return m_Pos; }
	int GetSpecialCardNum();
	// 983 00806700: linha do premium (item 0x1a100002) e altura da lista pelos cartoes (so premium)
	void UpdatePremium983();
	static void SetPosition(const WPoint& pos) { m_Pos = pos; }
	static void SetSpecialBuffStartTick(unsigned long tick)
	{
		m_SpecialBuffStartTick = tick;
	}

protected:
	virtual void OnProc(const float delta);

	void OnSPCardListInit(int param);
	void OnSPCardListOwnerDraw(int param);
	int GetRemainedSec(const sSCardAvilityPeriodInfo* info);

	FrListBox* m_pSPCardList;
	char m_premiumPeriodInfo[0x41]; // 983 +0x114: sSCardAvilityPeriodInfo da linha do premium (00806700); zerado em OnSPCardListInit
	char m_pad155[0xb]; // 983: sizeof 0x160

	static WPoint m_Pos;
	static unsigned long m_SpecialBuffStartTick;

	DECLARE_FRESH_MSGMAP()
};
#else
#include <list>
#include <vector>
#include "frform.h"

struct sSCardAvilityPeriodInfo;
struct sItemAttachCard;
class Bitmap;
class FrArea;
class FrButton;
class FrEdit;
class FrGraphicInterface;
class FrListBox;
class FrStatic;
class WOverlay;

class FrSpCardUseDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrSpCardUseDlg)

	FrSpCardUseDlg();
	virtual ~FrSpCardUseDlg();

	unsigned long& GetSpCardTid() { return m_spCardTid; }
	unsigned long GetOverlappedCard();
	void SetSpCardInfo(unsigned long& info);

protected:
	void OnItemInit(int param);
	void OnItemOwnerDraw(int param);
	void OnText1Init(int param);

	unsigned long m_overlappedCard; // 983 +0x110 (SetSpCardInfo 008069e0)
	unsigned long m_spCardTid; // 983 +0x114
	FrArea* m_pItem; // 983 +0x118 (OnItemInit 008014d0)
	FrStatic* m_pText1; // 983 +0x11c (OnText1Init 00801540); sizeof 0x120, sem text2 no msgmap 00e1d508

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
	virtual bool OnInit(); // 983: 008052a0
	void SetCardDesc983(); // 983: 00803b70
	void OnItemInit(int param);
	void OnItemOwnerDraw(int param);
	void OnCardDescInit(int param); // 983: 00801700, "card_desc"

	unsigned long m_partsGuid;
	unsigned long m_partsTid;
	FrArea* m_pItem;
	FrEdit* m_pCardDesc; // 983: +0x124

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
	void OnLevelImgInit(int param);
	void OnItemInit(int param);
	void OnItemOwnerDraw(int param);

	FrArea* m_pItem;
	FrArea* m_pLevelImg;

	DECLARE_FRESH_MSGMAP()
};

class FrOpenCardPackResultDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrOpenCardPackResultDlg)

	FrOpenCardPackResultDlg();
	virtual ~FrOpenCardPackResultDlg();

	unsigned long m_cardTid[3];
	unsigned long m_cardTid4;

protected:
	void OnItemInit(int param);
	void OnCardFrmNumInit(int param);
	void OnItemOwnerDraw(int param);

	FrArea* m_pItem;
	FrArea* m_pCardFrmNum;

	DECLARE_FRESH_MSGMAP()
};

class FrCardAttachDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrCardAttachDlg)

	FrCardAttachDlg();
	virtual ~FrCardAttachDlg();

	virtual bool OnInit(); // 983 00806a10
	bool OnSetDlgTxt();

	unsigned long m_cardGuid;
	unsigned long m_cardTid;
	unsigned long m_partGuid;
	unsigned long m_partTid;
	unsigned long m_charSlot;
	unsigned long m_caddieSlot;
	unsigned long m_npcSlot; // 983 +0x128

protected:
	void OnCardAvilityInit(int param);
	void OnCardAvilityOldInit(int param);
	void OnItemInit(int param);
	void OnItemOwnerDraw(int param);

	unsigned long m_unused12c;
	unsigned long m_unused130;
	FrArea* m_pItem;
	FrEdit* m_pCardAvility;
	FrEdit* m_pCardAvilityOld;
	unsigned long m_unused140; // 983 +0x140: Bitmap* do icone do 0x1a00018f (OnInit 00806a10, so taco); lido no OnItemOwnerDraw 00804420
	unsigned long m_unused144;
	std::string m_str148; // 983 +0x148: quantidade do 0x1a00018f ("%d\xb0\xb3", OnInit 00806a10)

	DECLARE_FRESH_MSGMAP()
public:
	char m_pad983_164[0xc]; // 983: sizeof 0x170 (std::string VC10 0x1c termina em +0x164; +0x164..+0x16f sem uso achado)
};

class CTFrameMove
{
public:
	CTFrameMove();
	CTFrameMove(unsigned char count);
	~CTFrameMove();

	void AddFrame(FrArea* pFrame);
	bool SetFrameWidth(float width);
	bool SetFrameHeight(float height);

private:
	void Reset()
	{
		if (!m_frames.empty())
		{
			m_frames.clear();
		}
	}

	unsigned char m_count;
	std::vector<FrArea*> m_frames;
};

class FrItemAttachCardDlg : public FrForm
{
public:
#ifdef REBANG_LAYOUT_CHECK
	static void CheckLayout983();
#endif
	int GetSlotKind983(int slot);
	bool IsInvalidSlot983(int slot);
	unsigned long GetSlotCard983(int slot);
	bool IsSlotAttached983(int slot);
	int GetSlotIndex983(int kind);
	void SetSlotInfo983(int kind, int idx, unsigned long card, bool attached);
	void DrawSlotTxt983(float x, float y, int idx, int kind);
	void DrawEmptySlot983(float x, float y, int idx);
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
	virtual bool OnInit();
	virtual void OnProc(const float delta);

	void initCardSlotBtnTxt();
	void OnAttachCardIconInit(int param);
	void OnAttachCardIconOwnerDraw();
	void OnAttachCardNameInit(int param);
	void OnAttachCardAbilityInit(int param);
	void OnAttachItemIconInit(int param);
	void OnAttachItemIconOwnerDraw();
	void OnAttachItemNameInit(int param);
	void OnAttachItemAbilityInit(int param);
	void OnAreaCharImgInit(int param);
	void OnItemListFoldInit(int param);
	void OnItemListUnfoldInit(int param);
	void OnItemListFoldLClick();
	void OnSelectPartsInit(int param);
	void OnSelectClubInit(int param);
	void OnSelectPartsLClick();
	void OnSelectClubLClick();
	void SetListCardUsableClub983();
	void OnOkLClick983();
	void OnItemListUnfoldLClick();
	void OnDlgFrm_04Init(int param);
	void OnDlgFrm_05Init(int param);
	void OnDlgFrm_06Init(int param);
	void OnDlgFrm_07Init(int param);
	void OnDlgFrm_08Init(int param);
	void OnDlgFrm_09Init(int param);
	bool OnCardAttachResult(int ret, FrForm* pForm);
	void OnOpenAttachDlg(int charSlot, int caddieSlot, int npcSlot);
	void SetCharacterAttatchBtnTxt(unsigned long tid, char* text);
	void SetCaddieAttatchBtnTxt(unsigned long tid, char* text);
	void DrawBtnCharSlotimg(float x, float y, unsigned long tid);
	void DrawBtnCaddieSlotimg(float x, float y, unsigned long tid);
	void OnCharSlotBtnInit_1(int param);
	void OnCharSlotBtnDown_1();
	void OnCharSlotBtnOwnerDraw_1();
	void OnCharSlotBtnInit_2(int param);
	void OnCharSlotBtnDown_2();
	void OnCharSlotBtnOwnerDraw_2();
	void OnCharSlotBtnInit_3(int param);
	void OnCharSlotBtnDown_3();
	void OnCharSlotBtnOwnerDraw_3();
	void OnCharSlotBtnInit_4(int param);
	void OnCharSlotBtnDown_4();
	void OnCharSlotBtnOwnerDraw_4();
	void OnCaddieSlotBtnInit_1(int param);
	void OnCaddieSlotBtnDown_1();
	void OnCaddieSlotBtnOwnerDraw_1();
	void OnCaddieSlotBtnInit_2(int param);
	void OnCaddieSlotBtnDown_2();
	void OnCaddieSlotBtnOwnerDraw_2();
	void OnCaddieSlotBtnInit_3(int param);
	void OnCaddieSlotBtnDown_3();
	void OnCaddieSlotBtnOwnerDraw_3();
	void OnCaddieSlotBtnInit_4(int param);
	void OnCaddieSlotBtnDown_4();
	void OnCaddieSlotBtnOwnerDraw_4();
	void SetListMyItem();
	void OnMyItemListInit(int param);
	void OnMyItemListLClick();
	void OnMyItemListRClick();
	void OnMyItemListOwnerDraw(int param);
	void SetListCardUsableMyItem();
	void DrawItem(WPoint pt, unsigned long tid, unsigned long guid);
	void SetCardInfo(unsigned long tid);
	void SetListEquipParts();
	bool IsEquipParts(unsigned long tid, unsigned long guid);
	void SetSelectMyItem(sItemAttachCard* pItem);
	void SetPartsSlot(int nChar, int nCaddie, int nNpc);
	void DisableSlotRows983(int nChar, int nCaddie, int nNpc); // 983 00800b80 (argumentos ignorados)
	void EnableSlotRows983(int nChar, int nCaddie, int nNpc); // 983 00800c20
	void SetPartsSlotBtnTxt();
	void DrawPartsSlot(WPoint pt, unsigned long tid, unsigned long guid,
		FrGraphicInterface* pGDI);
	void DrawPartsLine();
	void AgreeAttaching();
	bool OnAgreeAttachingResult(int ret, FrForm* pForm);
	void ResetCardSlotInfo();

	FrListBox* m_pMyItemList;
	FrArea* m_pAttachCardIcon;
	FrEdit* m_pAttachCardName;
	FrEdit* m_pAttachCardAbility;
	FrArea* m_pAttachItemIcon;
	FrEdit* m_pAttachItemName;
	FrEdit* m_pAttachItemAbility;
	FrArea* m_pAreaCharImg;
	FrButton* m_pItemListFold;
	FrButton* m_pItemListUnfold;
	// 983: 8 slots (0-3 personagem, 4-7 caddie), stride 0x3c a partir de +0x150
	struct sAttachSlot
	{
		FrButton* pBtn;
		char txt[50];
		bool attached;
		unsigned long card;
	};
	sAttachSlot m_slots[8];
	unsigned long m_unused330[3]; // +0x330..0x33c
	FrButton* m_pSelectParts; // +0x33c (983 00802720: "select_parts")
	FrButton* m_pSelectClub; // +0x340 (983 00802790: "select_club")
	int m_selectMode983; // +0x344: 0 = pecas, 1 = tacos (zerado no ctor)
	unsigned long m_unused348;
	FrCardAttachDlg* m_pCardAttachDlg; // +0x34c
	const Bitmap* m_pMyItemBaseClickBmp;
	const Bitmap* m_pMyItemBaseOverBmp;
	sItemAttachCard* m_pSelectedItem; // +0x358
	std::list<sItemAttachCard> m_myItemList;
	std::list<sItemAttachCard> m_equipParts;
	CTFrameMove m_cardSlotMove; // +0x374
	const Bitmap* m_pEmptySlotBmp[3]; // +0x388 (char, caddie, npc)
	WOverlay* m_pLineOverlay;
	bool m_bItemListFolded; // +0x398
	const Bitmap* m_pUccStateBmp[3]; // +0x39c

	DECLARE_FRESH_MSGMAP()
};

class FrCardBoxDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrCardBoxDlg)
	FrCardBoxDlg();

	virtual void SetCardInfo(unsigned long tid, unsigned long guid,
		unsigned long arg);

	// Resto da classe do 983 sem acesso no nosso codigo: so reserva os bytes.
	char m_pad110[0x58];
};

class FrSPCardBuffDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrSPCardBuffDlg)

	FrSPCardBuffDlg();
	virtual ~FrSPCardBuffDlg();

	virtual bool OnInit();
	void SetBuffItem(const std::list<sSCardAvilityPeriodInfo>& cards,
		bool bClear);
	const WPoint& GetPosition() { return m_Pos; }
	int GetSpecialCardNum();
	// 983 00806700: linha do premium (item 0x1a100002) e altura da lista pelos cartoes (so premium)
	void UpdatePremium983();
	static void SetPosition(const WPoint& pos) { m_Pos = pos; }
	static void SetSpecialBuffStartTick(unsigned long tick)
	{
		m_SpecialBuffStartTick = tick;
	}

protected:
	virtual void OnProc(const float delta);

	void OnSPCardListInit(int param);
	void OnSPCardListOwnerDraw(int param);
	int GetRemainedSec(const sSCardAvilityPeriodInfo* info);

	FrListBox* m_pSPCardList;
	char m_premiumPeriodInfo[0x41]; // 983 +0x114: sSCardAvilityPeriodInfo da linha do premium (00806700); zerado em OnSPCardListInit
	char m_pad155[0xb]; // 983: sizeof 0x160

	static WPoint m_Pos;
	static unsigned long m_SpecialBuffStartTick;

	DECLARE_FRESH_MSGMAP()
};
#endif
