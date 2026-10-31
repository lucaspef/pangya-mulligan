#pragma once

#include <list>
#include <vector>
#include "singleton.h"
#include "wmath.h"
#include "wtypes.h"
#include "frform.h"
#include "objectfactory.h"
#include "../shared/globalnetworkdefine.h"

// HACK: the COMDAT order of realmyroommainui.cpp (PCH inlines x its own functions and
// end-of-TU templates) depends on the amount of IL this header adds to its TU. The
// three dialogs, the FrCmdTarget base of CItemStorage (second vtable: an adjustor
// thunk at the end of every including TU), GetStoredPang and the WPoint of the
// drag&drop record are only needed by itemstorage.cpp, which defines
// REBANG_ITEMSTORAGE_FULL; other units get a layout-compatible slim view.
#ifdef REBANG_SEMANTIC_ONLY
#define REBANG_ITEMSTORAGE_FULL
#endif

class CIndexControl;
class FrArea;
class FrButton;
class FrEdit;
class FrListBox;
class WTitleFont;
struct FrListItem;
struct sItemInfo;

template <class T, class P>
bool TSafeNew(P& p, unsigned int count)
{
	bool ret = false;

	if (p == NULL && count > 0)
	{
		p = new T[count];
		if (p)
			ret = true;
	}

	return ret;
}

template <class P>
void TSafeDelete(P& p)
{
	if (p)
	{
		delete[] p;
		p = NULL;
	}
}

class FrItemStorageNotice : public FrForm
{
public:
	DECLARE_OBJECT(FrItemStorageNotice)
	FrItemStorageNotice() { }
	virtual ~FrItemStorageNotice() { }

	virtual void OnProc(const float dt);
	virtual void OnDraw();

protected:
	void OnLBDownConfirm();
	void OnOwnerDrawPrintOut(int param);

private:
	DECLARE_FRESH_MSGMAP()
};

class FrItemStorageAccessPanel : public FrForm
{
public:
	enum eFormType
	{
		FORMTYPE_FIRST,
		FORMTYPE_INPUT,
		FORMTYPE_CHANGE,
		FORMTYPE_LOCK,
		FORMTYPE_UNLOCK
	};

	DECLARE_OBJECT(FrItemStorageAccessPanel)
	FrItemStorageAccessPanel();
	virtual ~FrItemStorageAccessPanel();

	void SetFormType(eFormType formType)
	{
		if (0) // HACK: emission order (waits for the late Fresh inlines like the original)
			UseDblClick(false);
		m_formType = formType;
		switch (formType)
		{
		case FORMTYPE_FIRST:
			SetCaption(
				"\xc3\xd6\xc3\xca \xc6\xd0\xbd\xba\xbf\xf6\xb5\xe5\xb8\xa6 \xbc\xb3\xc1\xa4\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.");
			SetDesc(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 4\xc0\xda\xb8\xae \xc0\xd4\xb7\xc2.");
			m_inputStep = 1;
			break;
		case FORMTYPE_INPUT:
			SetCaption("\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xc0\xd4\xb7\xc2");
			SetDesc(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 4\xc0\xda\xb8\xae \xc0\xd4\xb7\xc2.");
			m_inputStep = 1;
			// 983 00604550 case 1: so na entrada de senha aparece o botao de configuracao (troca de senha)
			if (m_pOptionBn)
				m_pOptionBn->SetVisible(true);
			break;
		case FORMTYPE_CHANGE:
			SetCaption("\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xba\xaf\xb0\xe6");
			SetDesc(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 4\xc0\xda\xb8\xae \xc0\xd4\xb7\xc2.");
			m_inputStep = 0;
			break;
		case FORMTYPE_LOCK:
			SetCaption("\xc0\xe1\xb1\xdd\xbc\xb3\xc1\xa4");
			SetDesc(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 4\xc0\xda\xb8\xae \xc0\xd4\xb7\xc2.");
			m_inputStep = 1;
			break;
		case FORMTYPE_UNLOCK:
			SetCaption("\xc0\xe1\xb1\xdd\xc7\xd8\xc1\xa6");
			SetDesc(
				"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 4\xc0\xda\xb8\xae \xc0\xd4\xb7\xc2.");
			m_inputStep = 1;
			break;
		}
	}
	eFormType GetFormType() const { return m_formType; }
	void SendPasswordToServer();

protected:
	template <class T>
	bool TBindControl(T*& pControl, int param)
	{
		pControl = DYNAMIC_CAST(T, (FrWnd*)param);
		return pControl ? true : false;
	}

	bool DiffPassword();
	bool InputPassword(unsigned char number);
	void SetButtonImage(FrButton* pButton, unsigned char number);

	void OnInitPrintOut(int param);
	void OnOwnerDrawPrintOut(int param);
	void OnInitNumberSlot0(int param);
	void OnInitNumberSlot1(int param);
	void OnInitNumberSlot2(int param);
	void OnInitNumberSlot3(int param);
	void OnInitNumberSlot4(int param);
	void OnInitNumberSlot5(int param);
	void OnInitNumberSlot6(int param);
	void OnInitNumberSlot7(int param);
	void OnInitNumberSlot8(int param);
	void OnInitNumberSlot9(int param);
	void OnLBDownNumberSlot0();
	void OnLBDownNumberSlot1();
	void OnLBDownNumberSlot2();
	void OnLBDownNumberSlot3();
	void OnLBDownNumberSlot4();
	void OnLBDownNumberSlot5();
	void OnLBDownNumberSlot6();
	void OnLBDownNumberSlot7();
	void OnLBDownNumberSlot8();
	void OnLBDownNumberSlot9();
	void OnInitConfirm(int param);
	void OnLBDownConfirm();
	void OnInitCancel(int param);
	void OnLBDownCancel();
	void OnInitOption983(int param);
	void OnLBDownOption983();
	void OnHoverOnOption983(int param);
	void OnHoverOffOption983(int param);

	eFormType m_formType;
	int m_inputStep;
	char m_password[32];
	char m_newPassword[32];
	char m_confirmPassword[32];
	unsigned char m_numberSlot[10];
	FrArea* m_pPrintOut;
	FrButton* m_pNumberSlotBn[10];
	FrButton* m_pConfirmBn;
	FrButton* m_pCancelBn;
	FrButton* m_pOptionBn; // 983 +0x1b8 "btn_option"

private:
	DECLARE_FRESH_MSGMAP()
};

#ifdef REBANG_ITEMSTORAGE_FULL
class FrItemCounter : public FrForm
{
public:
	DECLARE_OBJECT(FrItemCounter)
	FrItemCounter();
	virtual ~FrItemCounter();

	void SetStoredItemInfo(const sStoredItemInfo& info)
	{
		// ORDER: dead reference to a late PCH inline, so this inline is emitted at
		// the end of the main pass as in the original.
		if (0)
		{
			GetRect();
		}
		m_storedItemInfo = info;
	}
	const sStoredItemInfo& GetStoredItemInfo() const
	{
		return m_storedItemInfo;
	}
	void SetType(unsigned long type)
	{
		// ORDER: dead reference to a late PCH inline, so this inline is emitted at
		// the end of the main pass as in the original.
		if (0)
		{
			GetRect();
		}
		m_type = type;

		switch (type)
		{
		case 0:
			SetCaption(K2L_Compatibility(
				"\276\306\300\314\305\333 \272\270\260\374\307\317\261\342"));
			SetDesc(K2L_Compatibility(
				"\272\270\260\374\307\322 \276\306\300\314\305\333\300\307 "
				"\274\366\267\256\300\273 \300\324\267\302\307\317\274\274\277\344."));
			break;
		case 1:
			SetCaption(K2L_Compatibility(
				"\276\306\300\314\305\333 \260\241\301\256\277\300\261\342"));
			SetDesc(K2L_Compatibility(
				"\260\241\301\256\277\303 \276\306\300\314\305\333\300\307 "
				"\274\366\267\256\300\273 \300\324\267\302\307\317\274\274\277\344."));
			break;
		}
	}
	unsigned long GetType() const { return m_type; }

protected:
	void OnInitDrawArea(int param);
	void OnOwnerDrawDrawArea(int param);
	void OnInitInput(int param);
	void OnInitConfirm(int param);
	void OnLBDownConfirm();
	void OnInitCancel(int param);
	void OnLBDownCancel();

	FrArea* m_pDrawArea;
	FrEdit* m_pInput;
	FrButton* m_pConfirmBn;
	FrButton* m_pCancelBn;
	unsigned long m_type;
	sStoredItemInfo m_storedItemInfo;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrItemConfirm : public FrForm
{
public:
	DECLARE_OBJECT(FrItemConfirm)
	FrItemConfirm();
	virtual ~FrItemConfirm();

	void SetStoredItemInfo(const sStoredItemInfo& info)
	{
		// ORDER: dead reference to a late PCH inline, so this inline is emitted at
		// the end of the main pass as in the original.
		if (0)
		{
			GetRect();
		}
		m_storedItemInfo = info;
	}
	const sStoredItemInfo& GetStoredItemInfo() const
	{
		return m_storedItemInfo;
	}
	void SetType(unsigned long type)
	{
		// ORDER: dead reference to a late PCH inline, so this inline is emitted at
		// the end of the main pass as in the original.
		if (0)
		{
			GetRect();
		}
		m_type = type;

		switch (type)
		{
		case 0:
			SetCaption(K2L_Compatibility(
				"\276\306\300\314\305\333 \272\270\260\374\307\317\261\342"));
			SetDesc(K2L_Compatibility("\276\306\300\314\305\333\300\273 \303\242\260\355\277\241 "
					"\272\270\260\374\307\325\264\317\264\331."));
			break;
		case 1:
			SetCaption(K2L_Compatibility(
				"\276\306\300\314\305\333 \260\241\301\256\277\300\261\342"));
			SetDesc(K2L_Compatibility(
				"\276\306\300\314\305\333\300\273 \303\242\260\355\277\241\274\255 "
				"\260\241\301\256\277\311\264\317\264\331."));
			break;
		}
	}
	unsigned long GetType() const { return m_type; }

protected:
	void OnInitDrawArea(int param);
	void OnOwnerDrawDrawArea(int param);
	void OnInitConfirm(int param);
	void OnLBDownConfirm();
	void OnInitCancel(int param);
	void OnLBDownCancel();

	FrArea* m_pDrawArea;
	FrButton* m_pConfirmBn;
	FrButton* m_pCancelBn;
	unsigned long m_type;
	sStoredItemInfo m_storedItemInfo;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrItemStoragePangBank : public FrForm
{
public:
	enum eType
	{
		TYPE_DEPOSIT,
		TYPE_WITHDRAW
	};

	DECLARE_OBJECT(FrItemStoragePangBank)
	FrItemStoragePangBank();
	virtual ~FrItemStoragePangBank();

	void SetStoredPang(__int64 pang) { m_storedPang = pang; }
	__int64 GetStoredPang() const { return m_storedPang; }
	void SetMyPang(__int64 pang) { m_myPang = pang; }
	__int64 GetMyPang() const { return m_myPang; }
	void SetType(unsigned long type);
	unsigned long GetType() const { return m_type; }

protected:
	void OnInitDrawArea(int param);
	void OnOwnerDrawDrawArea(int param);
	void OnInitInput(int param);
	bool OnInputEnterKey(int param);
	void OnInitConfirm(int param);
	void OnLBDownConfirm();
	void OnInitCancel(int param);
	void OnLBDownCancel();

	FrArea* m_pDrawArea;
	FrEdit* m_pInput;
	FrButton* m_pConfirmBn;
	FrButton* m_pCancelBn;
	unsigned long m_type;
	__int64 m_storedPang;
	__int64 m_myPang;

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
	friend bool __fastcall
	IsMannerPlayer(); // HACK: literal host, see itemstorage.cpp
#endif
};
#else
class FrItemCounter;
class FrItemConfirm;

class FrItemStoragePangBank
{
public:
	enum eType
	{
		TYPE_DEPOSIT,
		TYPE_WITHDRAW
	};
};
#endif

struct sStoredItemDragDrop
{
	enum MODE
	{
		MODE_NONE,
		MODE_PUSH,
		MODE_POP
	};

	MODE mode;
#ifdef REBANG_ITEMSTORAGE_FULL
	WPoint pos;
#else
	unsigned long pos[2];
#endif
	unsigned long index;
	const sItemInfo* pItemInfo;
	sStoredItemInfo stored; // 0x14: the storage slot and the item
};

#ifdef REBANG_ITEMSTORAGE_FULL
class CItemStorage : public WSingleton<CItemStorage>, public FrCmdTarget
#else
class CItemStorage : public WSingleton<CItemStorage>
#endif
{
#ifndef REBANG_ITEMSTORAGE_FULL
protected:
	unsigned long m_cmdTarget[sizeof(FrCmdTarget) / sizeof(unsigned long)];
#endif

public:
	CItemStorage();
	virtual ~CItemStorage();

	void InitIndexControl(unsigned long totalCount, unsigned long countPerPage);
	void IndexControlDraw(WPoint& pos);
	void IndexControlClick();
	void IndexControlNext();
	void IndexControlPrev();
	bool NeedPageIndexer();
	void SetDragDropItem(sStoredItemDragDrop& dragDrop);
	unsigned long GetDragingItemIndex(sStoredItemDragDrop::MODE mode);
	void Lock();
	void UnLock();
	void AddFrontUI(FrWnd* pWnd);
	void AddBackGroundUI(FrWnd* pWnd, bool bVisibleGroup);
	void SetVisibleFrontUI(bool bVisible);
	void SetVisibleBackGroundUI(bool bVisible);
	void SetEnableFrontUI(bool bEnable);
	void SetEnableBackGroundUI(bool bEnable);
	void Print(float x, float y, const char* text, float spacing, int align,
		float scale, unsigned long color);
	// sem posicao: so o realmyroommainui.cpp (E31) chama; posicao que reproduz o fundo do 645 (B289.1)
	void DrawBackgroundImage() { DrawBackgroundImage(WPoint(60.0f, 154.0f)); }
	void DrawBackgroundImage(const WPoint& pos);
	void SetDrag(sStoredItemDragDrop::MODE mode);
	void ClearStoredItemInfo();
	void AddStoredItemInfo(const sStoredItemInfo& info);
	bool OnResultPangBank(int ret, FrForm* pForm);
	bool OnResultItemCounter(int ret, FrForm* pForm);
	bool OnResultItemConfirm(int ret, FrForm* pForm);
	bool CheckEquiped(unsigned long guid);
	void DragDropItemDraw();
	void OpenPangBank(FrItemStoragePangBank::eType type);
	void DrawStoredItemInfo(const FrListBox*& pListBox, const FrListItem& item);
	void ReqPopStoredItemInfo(const sStoredItemInfo& info);
	void ReqPopStoredItemInfo(const FrListItem& item);
	bool CheckStoredItemInfo(sStoredItemInfo& stored, const sItemInfo& info);
	void ReqPushStoredItemInfo(const sItemInfo& info);
	void ReqPushStoredItemInfo(const FrListItem& item);
	void MakeStoredItemInfo(FrListBox*& pListBox);
#ifdef REBANG_ITEMSTORAGE_FULL
	__int64 GetStoredPang() const { return m_storedPang; }
#endif
	bool IsLock() const { return m_lock; }
	unsigned long GetTotalPage() const { return m_totalPage; }
	unsigned long GetCurPage() const { return m_curPage; }
	CIndexControl* GetIndexControl() { return m_indexControl; }
	void SetItemStorageArea(WRect rect) { m_itemStorageArea = rect; }
	void SetItemListArea(WRect rect) { m_itemListArea = rect; }
	bool IsVisible983() const { return m_visible; } // tradedlg 0xec: janela do armazem aberta

protected:
	bool CheckDrag();
	bool CheckDrop();

	bool m_visible;
	bool m_lock;
	std::list<FrWnd*> m_frontUI;
	std::list<FrWnd*> m_backGroundUI;
	std::list<FrWnd*> m_backGroundVisibleUI;
	WTitleFont* m_pTitleFont;
	__int64 m_storedPang;
	unsigned long m_totalPage;
	unsigned long m_curPage;
	CIndexControl* m_indexControl;
	std::vector<sStoredItemInfo*> m_storedItemInfo;
	std::vector<sItemInfo*> m_itemInfo;
	FrItemStoragePangBank* m_pPangBank;
	FrItemCounter* m_pItemCounter;
	FrItemConfirm* m_pItemConfirm;
	WRect m_itemStorageArea;
	WRect m_itemListArea;
	sStoredItemDragDrop m_dragDrop;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnresultpangbankFunclets(); // HACK: EH funclets of the naked OnResultPangBank (nakedify)
#endif
#ifdef REBANG_ITEMSTORAGE_FULL
	friend struct RbItemStorageLayout; // A727
#endif
};
#ifdef REBANG_ITEMSTORAGE_FULL
struct RbItemStorageLayout // A727: os offsets que as visoes parciais (realmyroomtask.h, taskmain.cpp, tradedlg.cpp) usam, nos dois compiladores
{
	C_ASSERT(offsetof(CItemStorage, m_visible) == sizeof(WSingleton<CItemStorage>) + 4);
	C_ASSERT(offsetof(CItemStorage, m_storedPang) == sizeof(WSingleton<CItemStorage>) + 8 + 3 * sizeof(std::list<void*>) + 4);
};
#endif
