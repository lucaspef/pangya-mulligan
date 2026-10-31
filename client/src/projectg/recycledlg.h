#pragma once
#include "vc10_layout.h"

#include "frform.h"
#include "objectfactory.h"
#include <map>
#include <list>

class CExhibition;
class CSimpleUI;
class WTitleFont;
class Bitmap;
class FrArea;
class FrButton;
class FrEdit;
class FrGraphicInterface;
class FrListBox;
class FrTextButton;
class FrSelectPartDlg;
class FrViewer;
struct sItemInfo;
struct FrListItem;

struct sRecycleItem;

// 983: o valor do mapa m_haveItemMap (4 bytes): bit31 conhecido, 0 tem o item, 1 nivel, 2 vencida, 3 agendada, 30 no periodo
struct sRecycleStatus983
{
	// 983 le os bits um a um como campo de bits (mov al,[ecx]; and al,1 / shr eax,N; and al,1)
	struct sBits
	{
		unsigned long owned : 1;        // 0x1 IsHaveItem983
		unsigned long levelLow : 1;     // 0x2 nivel abaixo do pedido
		unsigned long periodOver : 1;   // 0x4 fora do periodo (ja passou)
		unsigned long periodNotYet : 1; // 0x8 periodo ainda nao comecou
		unsigned long : 26;
		unsigned long inPeriod : 1;     // 0x40000000
		unsigned long known : 1;        // 0x80000000 Set ja rodou
	};
	union
	{
		unsigned long flags;
		sBits bits;
	};

	sRecycleStatus983();
	bool IsKnown() const;
	bool IsEnabled() const;
	bool Test(unsigned int kind) const;
	void Set(const sRecycleItem* pItem);
};

// Recycle (item mix) dialog. Size 0x630, the `new` of FrRecycleDlgMakeInstance.
// Only the members this unit's written functions use are named so far.
class FrRecycleDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrRecycleDlg)

	// A small icon drawn over a recycle list item (OnRecycleItemListInit).
	class MiniBtn
	{
	public:
		MiniBtn();

		WRect rect;            // 0x0
		const Bitmap* pBitmap; // 0x10
		char _pad14[0x4];
	};

	FrRecycleDlg();
	virtual ~FrRecycleDlg();

	void ClickRecycleItem();
	void DrawArrow();
	void DrawRecycleItem();
	void ControlRecycleNumber(int);
	void OpenInformation(const sRecycleItem*);
	void SetListRecycleItem(int, int);
	void SendDataRecycleItem(int, int);
	void SetCadieMessage(const char* msg);
	bool SetEventMessage(int level);
	void InitDataCharacterSelect();
	void InitListCharacterSelect();
	void EnableTotalControls(bool enable);
	void ResetMix983(); // 983 008db940 (C54.1): fim da mistura sem resultado (caso 0x22f com código != 0)
	void EnableMixControls(bool enable);
	void SetMaxEnableRecycle(int);
	void InitPositionItemData();
	void DrawItem(WPoint pos, unsigned long tid, int num, unsigned char type,
		const Bitmap* pBitmap, unsigned long color);
	void SetDataTotalItem(int index);
	void SetResultRecycleItem(bool success, std::list<sRecycleItem>& items,
		short index);
	void SelectLevel983(int level); // 983 008e3600: botao do nivel 0..4 (msg 0x339 do CTaskMain)

private:
	void ReInitializeHaveItem();
	void AddHaveItemMap(const sRecycleItem* pItem);
	const sRecycleStatus983& GetRecycleStatus983(const sRecycleItem* pItem) const;
	bool IsUsableParts(sItemInfo& item);

	DECLARE_FRESH_MSGMAP()

protected:
	virtual bool OnInit();
	virtual void OnProc(const float dt);
	void OnRecycleDlgInit(int);
	void OnAboutBnInit(int);
	void OnAboutBnUp();
	void OnCharacterSelectListLBnUp();
	void OnCloseBnUp();
	void OnRecycleItemListLBnUp();
	bool OnRecycleNumberEnterKey(int);
	void OnArrowMinimumBnDown();
	void OnArrowLeftBnUp();
	void OnArrowRightBnUp();
	void OnAreaPictureInit(int);
	void OnAreaPictureOwnerDraw();
	void OnArrowRightBnDown();
	virtual void OnCancel();
	bool OnSelectPartDlgResult(int ret, FrForm* pForm);
	// 983 (s102): mistura com confirmacao (OnMixBnUp 008de6a0)
	void StartMix983();                                     // 008dbdd0
	bool OnMixConfirmResult983(int ret, FrForm* pForm);     // 008dbe10 (item do grupo 0x1c)
	bool OnClubSetMixConfirmResult983(int ret, FrForm* pForm); // 008dd910 (taco entre os ingredientes)
	void SetLabels983(int itemWidth, int itemHeight);       // 008dd9a0 (rotulos do item da lista; OnInit)
	void OnCloseBnInit(int);
	void OnLevelLowBnInit(int);
	void OnLevelLowBnUp();
	void OnLevelMidBnInit(int);
	void OnLevelMidBnUp();
	void OnLevelHighBnInit(int);
	void OnLevelHighBnUp();
	void OnLevelSpecialBnUp();
	void OnLevelEventBnUp();
	void OnMixBnInit(int);
	void OnMixBnUp();
	void OnArrowMinimumBnInit(int);
	void OnArrowLeftBnInit(int);
	void OnArrowRightBnInit(int);
	void OnArrowMaximumBnInit(int);
	void OnRecycleItemNameInit(int);
	void OnCardSlotResultInit(int);
	// 983 (s102): controles novos do mapa de mensagens 00e2a930
	void OnMagicBoxBnInit983(int);
	void OnMagicBoxBnUp983();
	void OnCardComposeBnInit983(int);
	void OnCardComposeBnUp983();
	void OnExchangeBnInit983(int);
	void OnExchangeBnUp983();
	void OnBackCenterInit983(int);
	void OnBackBottomInit983(int);
	void OnLevelEventBnInit(int);
	void OnCharacterSelectListInit(int);
	void OnRecycleItemListRBnUp();
	void OnRecycleItemListInit(int);
	void OnRecycleNumberInit(int);
	void OnArrowLeftBnDown();
	void OnSelectPartInit(int);
	void OnSelectPartLBDown();
	void OnLevelSpecialBnInit(int);
	void OnCharacterSelectListOwnerDraw(int);
	void OnArrowMaximumBnDown();
	void OnCadieMessageInit(int);
	void OnRecycleItemListOwnerDraw(int);

	FrButton* m_pCloseBn;              // 0x110
	FrButton* m_pLevelLowBn;           // 0x114
	FrButton* m_pLevelMidBn;           // 0x118
	FrButton* m_pLevelHighBn;          // 0x11C
	FrButton* m_pLevelSpecialBn;       // 0x120
	FrButton* m_pLevelEventBn;         // 0x124
	FrListBox* m_pCharacterSelectList; // 0x128
	FrListBox* m_pRecycleItemList;     // 0x12C
	FrEdit* m_pRecycleNumber;          // 0x130
	FrButton* m_pMixBn;                // 0x134
	FrButton* m_pArrowMinimumBn;       // 0x138
	FrButton* m_pArrowLeftBn;          // 0x13C
	FrButton* m_pArrowRightBn;         // 0x140
	FrButton* m_pArrowMaximumBn;       // 0x144
	FrEdit* m_pRecycleItemName;        // 0x148
	FrEdit* m_pCadieMsg;               // 0x14C, the caddie's message box
	WTitleFont*
		m_pTitleFont; // 0x150, released through g_resrcmng by the destructor
	const Bitmap* m_pItemBnNormal; // 0x154, RECYCLEDLG "recycle_item_bn_normal"
	const Bitmap* m_pItemBnClick;  // 0x158, RECYCLEDLG "recycle_item_bn_click"
	CSimpleUI* m_pArrowUI;         // 0x15C, draws the two arrows (DrawArrow)
	// 983 (s102): layout medido no construtor 008e15f0 (sizeof 0x810, push do new em 00654a6a/008e2613), no destrutor 008e1390,
	// no mapa de mensagens 00e2a930 (inicializador 00ce3fa0) e nos acessos da unidade (out/F3.5/s102/disp_recycle.txt).
	// Sairam do 645: m_listLevel (0x160), m_pExhibition (0x510, o modelo "shop_cadie") e m_pCardSlot ("card_slot", 0x554).
	unsigned long m_characterTid
		[15]; // 0x160, the character type ids of the character select list (983: 15; InitDataCharacterSelect 008db740)
	int m_selectedIdx; // 0x19C, the selected recipe's index (-1: none; ClickRecycleItem)
	int m_characterIdx; // 0x1A0, the selected character list item's index
	int m_level;        // 0x1A4, the recycle level shown (low 0 .. event 4)
	sRecycleItem m_resultItem; // 0x1A8, the selected recipe's result item
	sRecycleItem
		m_needItem[4]; // 0x218, the ingredients the selected recipe needs
	sRecycleItem m_haveItem[4]; // 0x3D8, the ingredients the player put in
	int m_recycleNumber; // 0x598, how many times to recycle (the "recyclenumber" edit)
	int m_maxEnableRecycle; // 0x59C, how many times the recipe can be made
	sRecycleItem
		m_mixedItem; // 0x5A0, cleared when a mix starts and when m_mixResultTime runs out
	WPoint m_itemPos
		[10]; // 0x610, item slot positions: result, 4 needed, 4 owned, one more (InitPositionItemData 008dd0e0)
	bool m_arrowLeftDown;  // 0x660, cleared when the left arrow is released
	bool m_arrowRightDown; // 0x661, cleared when the right arrow is released
	char _pad662[0x2];
	float m_arrowHoldTime; // 0x664, reset when an arrow is pressed
	bool m_mixing;         // 0x668, set by OnMixBnUp
	char _pad669[0x3];
	int m_arrowState[2]; // 0x66C, per arrow: 0 "RIGHT", 1 "RIGHT_S" (DrawArrow)
	float m_arrowBlinkTime
		[2]; // 0x674, per arrow: time since its state last toggled (OnProc)
	float
		m_mixSendDelay; // 0x67C, seconds before the mix request is sent (OnMixBnUp: 1.5)
	float m_mixResultTime; // 0x680, seconds left before m_mixedItem is cleared
	float
		m_cadieMsgTime; // 0x684, seconds left on the caddie's message (SetEventMessage: 5)
	unsigned long
		m_selectedTid; // 0x688, the selected recipe's type id (ClickRecycleItem)
	unsigned long
		m_selectedPart;  // 0x68C, FrSelectPartDlg's result (0 when cancelled)
	bool m_partSelected; // 0x690
	bool m_unused545;    // 0x691, only cleared by the constructor
	char _pad692[0x2];
	int m_unused548;                   // 0x694, only cleared by the constructor
	int m_unused54C;                   // 0x698, only cleared by the constructor
	unsigned long m_resultColor983;    // 0x69C, 983: -1 no construtor; 0xffffffff/0xff707070 no ClickRecycleItem (008e2860), lida no 008dfc50
	FrButton* m_pSelectPartBn;         // 0x6A0 ("select_part", 008dd870)
	FrArea* m_pCardSlotResult;         // 0x6A4 ("card_slotresult", 008dd930)
	FrSelectPartDlg* m_pSelectPartDlg; // 0x6A8, the open part selection dialog
	FrButton* m_pMagicBoxBn983;        // 0x6AC, 983: "MagicBox" (008dc3f0)
	FrButton* m_pCardComposeBn983;     // 0x6B0, 983: "CardCompose" (008dc4a0; LBUp 008de490 = msg 0x324)
	FrButton* m_pExchangeBn983;        // 0x6B4, 983: "Exchange" (008dc550; LBUp 008de4f0 = msg 0x2e3)
	FrArea* m_pBackCenter983;          // 0x6B8, 983: "back_center_01" (008dc600)
	FrArea* m_pBackBottom983;          // 0x6BC, 983: "back_bottom_01" (008dc670)
	char _pad6C0[0x48];
	MiniBtn m_deleteBtn; // 0x708, "delete" icon of the recycle list items (OnRecycleItemListInit 008dcaa0)
	MiniBtn m_clockBtn;  // 0x720, "clock" icon of the recycle list items
	char _pad738[0x48];
	std::map<const sRecycleItem*, sRecycleStatus983> m_haveItemMap; // 0x780 (0x10; cabeca +0x784, tamanho +0x788)
	// 983: 5 rotulos do item da lista (008dd9a0): "recycle_text_owned", "level_001", "recycle_expired", "recycle_scheduled",
	// "recycle_in_period"; o construtor poe a cor 0xff000000 (nome meu)
	struct sLabel983
	{
		sLabel983();

		WRect rect;            // 0x0
		const Bitmap* pBitmap; // 0x10
		unsigned long color;   // 0x14
	};
	sLabel983 m_label983[5]; // 0x790
	char _pad808[0x8];
};

typedef char rb_size_check_0x18[sizeof(FrRecycleDlg::MiniBtn) == 0x18 ? 1 : -1];
C_ASSERT_VC10(sizeof(FrRecycleDlg) == 0x810); // 983 (s102)

// Recycle help dialog: five index buttons, backward/forward buttons and a
// viewer showing help_cadie_%02d.jpg. Size 0x138, the `new` of
// FrRecycleAboutDlgMakeInstance.
class FrRecycleAboutDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrRecycleAboutDlg)

	FrRecycleAboutDlg();
	virtual ~FrRecycleAboutDlg();

	void SetImgAboutIndex(int);

	DECLARE_FRESH_MSGMAP()

protected:
	virtual bool OnInit();
	void OnAboutInit(int);
	void OnAboutViewerInit(int);
	void OnIndex01Init(int);
	void OnIndex01BnUp();
	void OnIndex02Init(int);
	void OnIndex02BnUp();
	void OnIndex03Init(int);
	void OnIndex03BnUp();
	void OnIndex04Init(int);
	void OnIndex04BnUp();
	void OnIndex05Init(int);
	void OnIndex05BnUp();
	void OnBackwardBnInit(int);
	void OnBackwardBnUp();
	void OnForwardBnInit(int);
	void OnForwardBnUp();
	void OnOkBnInit(int);
	void OnOkBnUp();

	FrTextButton* m_pIndexBn[5]; // 0x110 ("index01".."index05")
	int m_aboutIndex;            // 0x124, the shown help page (0-4)
	FrButton* m_pBackwardBn;     // 0x128
	FrButton* m_pForwardBn;      // 0x12C
	FrViewer* m_pAboutViewer;    // 0x130
	char _pad134[0x4];
};

// 983: sizeof(FrRecycleAboutDlg) do 645 (0x138) não vale aqui (FrForm alinhada a 16; tamanho do 983 não medido)

// Part selection list of the recycle dialog. Size 0x120, the `new` of
// FrSelectPartDlgMakeInstance.
class FrSelectPartDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrSelectPartDlg)

	FrSelectPartDlg();
	virtual ~FrSelectPartDlg();

	void SetPartTID(unsigned long);
	void DrawStatus2(FrGraphicInterface* pGI, FrListItem* pItem,
		short* const status, short* const slotStatus, float x);

	DECLARE_FRESH_MSGMAP()

protected:
	void OnInitListBox(int);
	void OnLBUpListBox();
	void OnOwnerDrawListBox(int);

	FrListBox* m_pListBox;   // 0x110
	unsigned long m_partTid; // 0x114, set by SetPartTID

public:
	unsigned long GetPartTID983() const { return m_partTid; } // 983: lido em linha pelo OnSelectPartDlgResult (008dbcf0)
	unsigned long
		m_result; // 0x118, the chosen part, read by FrRecycleDlg::OnSelectPartDlgResult

protected:
	char _pad11C[0x4];
};

C_ASSERT_VC10(sizeof(FrSelectPartDlg) == 0x120);
