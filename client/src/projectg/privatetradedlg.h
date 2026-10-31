#pragma once

#include <string>
#include <vector>
#include "frform.h"
#include "titlefontex.h"

class FrEdit;
class WRect;
class Bitmap;

struct sTradeDropItem
{
	sTradeDropItem()
		: itemId(0), typeId(0), count(1)
	{
	}
	sTradeDropItem(unsigned long i, unsigned long t)
		: itemId(i), typeId(t), count(1)
	{
	}

	unsigned long itemId;
	unsigned long typeId;
	int count;
};

class CTradeDragDrop
{
	friend class FrPrivateTrade;

public:
	CTradeDragDrop();
	virtual ~CTradeDragDrop();
	virtual bool Pop();

	static bool __fastcall IsPoped();
	static void __fastcall Cancel();
	static void __fastcall Drop(CTradeDragDrop* target);
	const sTradeDropItem* GetDropItem();

protected:
	virtual bool DropItemUp() = 0;
	virtual bool DropItemDown() = 0;
	virtual void DropItemMoved() = 0;
	virtual void SetPopItem(const sTradeDropItem& item);

	static sTradeDropItem* m_pItem;

private:
	static CTradeDragDrop* m_pCaller;
};

class FrNumEdit : public FrForm
{
public:
	DECLARE_OBJECT(FrNumEdit)

	FrNumEdit();
	virtual ~FrNumEdit();

	unsigned long GetValue() const { return m_value; }
	void SetReturnParam(void* p) { m_returnParam = p; }
	void* GetReturnParam() { return m_returnParam; }
	void SetValue(unsigned long value);
	void SetEditRange(unsigned long min, unsigned long max);
	void OnViewDraw();
	void OnEditInit(int param);
	bool OnEditEnter(int param);

protected:
	virtual void OnProc(const float deltaTime);

private:
	FrEdit* m_pEdit;
	unsigned long m_value;
	void* m_returnParam;
	unsigned long m_min;
	unsigned long m_max;

	DECLARE_FRESH_MSGMAP()
};

class FrPangEdit : public FrForm
{
public:
	DECLARE_OBJECT(FrPangEdit)

	FrPangEdit();
	virtual ~FrPangEdit();

	void SetTexRate(unsigned char rate) { m_texRate = rate; }
	__int64 GetTex() { return m_pang * m_texRate / 100; }
	__int64 GetPangValue() const { return m_pang; }
	void SetPangValue(__int64 pang);
	void OnViewDraw();
	void OnEditInit(int param);
	bool OnEditEnter(int param);

protected:
	virtual void OnProc(const float deltaTime);

private:
	FrEdit* m_pEdit;
	int m_pad114; // 983: alinhamento do __int64 m_pang (+0x118); ctor 008d03a0 e metodos 008d0070..008d5f00 nao tocam +0x114
	__int64 m_pang;
	unsigned char m_texRate;

	DECLARE_FRESH_MSGMAP()
};

class EncapsulatedPang : public CWindFont
{
public:
	enum eStyle
	{
		STYLE_NONE = 0,
		STYLE_BAR = 2,
		STYLE_ICON = 4,
		STYLE_HOVER = 8
	};

	EncapsulatedPang();
	virtual ~EncapsulatedPang();

	__int64 GetPang() const { return m_pang; }
	const WRect* GetRectPtr() const { return m_pRect; }
	void SetPang(__int64 pang) { m_pang = pang; }
	void SetRect(const WRect& rect);
	int IsStyle(eStyle style);
	int SubPang(__int64 pang);
	void AddPangBarStyle(eStyle style);

protected:
	void DrawPang();

private:
	__int64 m_pang;
	WRect* m_pRect;
	int m_style;
};

struct sWarpItem;
struct FrListItem;
class FrListBox;
class FrButton;

enum eSetItemButton
{
	SET_ITEM_BUTTON_PART = 0,
	SET_ITEM_BUTTON_CLUB = 1,
	SET_ITEM_BUTTON_ITEM = 2,
	SET_ITEM_BUTTON_CARD = 3
};

enum eSetSortType
{
	SET_SORT_TYPE_DEFAULT = 0
};

class FrMyWareHouse : public FrForm,
					  public CTradeDragDrop,
					  public EncapsulatedPang
{
public:
	DECLARE_OBJECT(FrMyWareHouse)

	FrMyWareHouse();
	virtual ~FrMyWareHouse();

	virtual bool OnInit();
	virtual void OnProc(const float deltaTime);

	bool SubListItemCount(unsigned long guid, unsigned long typeId, int count);
	bool AddListItemCount(unsigned long guid, unsigned long typeId, int count);
	int GetListItemCount(unsigned long guid, unsigned long typeId);
	bool AddListItem(unsigned long guid, unsigned long typeId, int count);
	bool SetListItem(unsigned long guid, unsigned long typeId, int count);
	bool EraseListItem(unsigned long guid, unsigned long typeId);

protected:
	virtual bool DropItemUp();
	virtual bool DropItemDown();
	virtual void DropItemMoved();

	void OnViewDraw();
	void OnListInit(int param);
	void OnListDraw(int param);
	void OnListDblClick();
	void OnSortInit(int param);
	void OnSortDraw(int param);
	void OnSortDown(int param);
	void OnPartBtnInit(int param);
	void OnClubBtnInit(int param);
	void OnItemBtnInit(int param);
	void OnCardBtnInit(int param);
	void OnPartBtnDown();
	void OnClubBtnDown();
	void OnItemBtnDown();
	void OnCardBtnDown();

private:
	FrListItem* FindListItem(unsigned long guid, unsigned long typeId);
	void SetItemList(eSetItemButton button, eSetSortType sort);
	void InitSortList(eSetItemButton button);
	void SetButtonStyle(int param, eSetItemButton button);
	void UnPushedButtonAll();

	std::vector<sWarpItem*> m_warpItems;
	std::vector<FrButton*> m_buttons;
	FrListBox* m_pItemList;
	FrListBox* m_pSortList;
	eSetItemButton m_curButton;

	DECLARE_FRESH_MSGMAP()
};

struct _sPrivateTradeCardInfo
{
	unsigned long typeId;
	unsigned long id;
	int effect;
	int effectValue;
	int slot;
	int kind;
};

struct _sPrivateTradeItemSingle
{
	unsigned long itemId;
	unsigned long typeId;
	unsigned short count;
	unsigned char m_pad0a[2]; // alinhamento antes de cards (+0x0c)
	_sPrivateTradeCardInfo cards[10]; // 983: 10 cartas (slotStats em 0xfc; Formator 008d6fb0)
	short slotStats[5];
	unsigned char m_pad106[2]; // alinhamento do fim (slotStats +0xfc..+0x105; sizeof 0x108)
};

struct _sPrivateTradeItem
{
	__int64 pang;
	_sPrivateTradeItemSingle items[4];
};

class FrTradeToolTip : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeToolTip)

	FrTradeToolTip();
	virtual ~FrTradeToolTip();

	void SetDragDraw(int bDraw) { m_bDragDraw = bDraw; }
	void SetDragItem(const sTradeDropItem* pItem);
	void SetToolTip(_sPrivateTradeItemSingle* pItem);

private:
	void MakeFormatText(const std::string& text,
		std::vector<std::string>& lines, int param);

protected:
	void OnViewDraw();
	void OnTextListInit(int param);
	void OnTextListDraw(int param);

	virtual void OnProc(const float deltaTime);

private:
	float m_alpha;
	FrListBox* m_pTextList;
	const Bitmap* m_pIcon;
	std::vector<std::string> m_textLines;
	_sPrivateTradeItemSingle* m_pItem;
	int m_bDragDraw;
	const sTradeDropItem* m_pDragItem;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbMakeformattextFunclets(); // HACK: EH funclets of the naked MakeFormatText (nakedify)
#endif
};

class FrPrivateTradeWnd : public FrForm,
						  public CTradeDragDrop,
						  public EncapsulatedPang
{
public:
	DECLARE_OBJECT(FrPrivateTradeWnd)

	FrPrivateTradeWnd();
	virtual ~FrPrivateTradeWnd();

	virtual bool OnInit();
	void OnUpdateTradeItem(const _sPrivateTradeItem& items);

	void SetNick(const std::string& nick) { m_nick = nick; }

	void EnableActiveControl(bool bEnable) { m_bActiveControl = bEnable; }
	void SetAvatarIndex(int index);
	void GetTradeItems(_sPrivateTradeItem& items);
	void DoConfirm();
	void DoPermit();
	void DoTradeUnLock();
	void SubItem(const sTradeDropItem* pItem);
	void AddItem(const sTradeDropItem* pItem);
	bool OnResultEditNum(int result, FrForm* pForm);
	bool OnResultEditPang(int result, FrForm* pForm);

protected:
	virtual bool DropItemUp();
	virtual bool DropItemDown();
	virtual void DropItemMoved();

	void OnViewDraw();
	void OnListInit(int param);
	void OnListDraw(int param);
	void OnListDown();
	void OnListDblClick();
	void OnNumEditBtn01Init(int param);
	void OnNumEditBtn01Down();
	void OnNumEditBtn02Init(int param);
	void OnNumEditBtn02Down();
	void OnNumEditBtn03Init(int param);
	void OnNumEditBtn03Down();
	void OnNumEditBtn04Init(int param);
	void OnNumEditBtn04Down();

	virtual void OnProc(const float deltaTime);

	void OnSendItemUpdate();
	void OpenNumEditWnd(int index);
	void OpenPangEditWnd();

private:
	std::string m_nick;
	int m_avatarIndex;
	FrListBox* m_pItemList;
	FrPangEdit* m_pPangEditWnd;
	std::vector<FrButton*> m_buttons;
	FrNumEdit* m_pNumEditWnd;
	bool m_bActiveControl;
	char m_tradeState;
	_sPrivateTradeItem* m_pTradeItem;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnlistdrawFunclets(); // HACK: EH funclets of the naked OnListDraw (nakedify)
#endif
};

class FrPrivateTrade : public FrForm, public EncapsulatedPang
{
	friend class FrPrivateTradeWnd;
	friend class FrMyWareHouse;

public:
	DECLARE_OBJECT(FrPrivateTrade)

	FrPrivateTrade();
	virtual ~FrPrivateTrade();

	void SetCompanion(const std::string& nickName, unsigned long guid,
		unsigned long typeId);
	void GetMyTradeItems(_sPrivateTradeItem& items);
	void SetTradeUnLockAll();
	void ButtonContolByConfirmCancel();
	void UpdateFrTradeItem(const _sPrivateTradeItem& items);
	void FinalFrWait(const _sPrivateTradeItem& items);
	void FinalFrOK(const _sPrivateTradeItem& items);
	void MoveItem(const char* from, const void* pData);

	virtual bool OnInit();

protected:
	void OnViewDraw();
	void OnInitConfirmBtn(int param);
	void OnInitPermitBtn(int param);
	void OnInitCancelBtn(int param);
	void OnDownConfirmBtn();
	void OnDownPermitBtn();
	void OnDownCancelBtn();

	virtual void OnProc(const float deltaTime);

private:
	FrPrivateTradeWnd* m_pCompanionTradeWnd;
	FrPrivateTradeWnd* m_pMyTradeWnd;
	FrTradeToolTip* m_pToolTip;
	FrMyWareHouse* m_pWareHouse;
	const sTradeDropItem* m_pDragItem;
	FrButton* m_pConfirmBtn;
	FrButton* m_pPermitBtn;
	FrButton* m_pCancelBtn;
	std::string m_companionNick;
	unsigned long m_companionGuid;
	unsigned long m_companionTypeId;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOninitFunclets(); // HACK: EH funclets of the naked OnInit (nakedify)
#endif
};
