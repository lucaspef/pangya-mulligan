#pragma once

#include <list>
#include "frform.h"

class FrArea;
class FrListBox;
class FrGraphicInterface;
class Bitmap;
struct FrListItem;
struct sItemInfo;

class FrCardRemove : public FrForm
{
public:
	DECLARE_OBJECT(FrCardRemove)

	FrCardRemove();
	virtual ~FrCardRemove();

	static bool IsCardRemoveItem(unsigned long tid);
	void SetCardRemoveItem(unsigned long tid, unsigned long guid);

protected:
	void DrawInDlg(const char* name, float x, float y);
	void DrawTexture(const char* name, float x, float y, unsigned long color);
	void PrintText(const char* text, float x, float y, unsigned long color);
	bool CheckEquiped(unsigned long tid, unsigned long guid);
	bool SetCurPartUid(unsigned long uid);
	void BuildCurPartCardList();
	void OnDrawAreaInit(int param);
	void OnDrawAreaOwnerDraw(int param);
	void OnItemListInit(int param);
	void OnItemListOwnerDraw(int param);
	void OnItemListDBClick();
	void OnItemListRBDown();
	void OnConfirmInit(int param);
	void OnConfirmLBDown();
	bool OnUseCardRemoveConfirm(int result, FrForm* form);
	void OnCancelInit(int param);
	void OnCancelLBDown();

	FrArea* m_pDrawArea;
	FrListBox* m_pItemList;
	unsigned long m_removeItemTid;
	unsigned long m_removeItemGuid;
	unsigned long m_curPartTid;
	unsigned long m_curPartUid;
	std::list<unsigned long> m_cardList;

private:
	DECLARE_FRESH_MSGMAP()
public:
	// 983 007fe440 OnInit: card_remove_bg_1..6 (+0x134) e itemlist_button_n/o (+0x14c/+0x150); sizeof 0x160
	// (push antes do operator new em 007fff80). Fica no fim: o resto do layout do 983 nao foi medido.
	const Bitmap* m_pBitmap983[8];

protected:
	virtual bool OnInit();
};

class FrCardRemoveConfirm : public FrForm
{
public:
	DECLARE_OBJECT(FrCardRemoveConfirm)

	FrCardRemoveConfirm();
	virtual ~FrCardRemoveConfirm();

	void SetPart(unsigned long tid, unsigned long uid)
	{
		m_partTid = tid;
		m_partUid = uid;
	}

protected:
	void OnDrawAreaOwnerDraw(int param);
	void OnConfirmInit(int param);
	void OnConfirmLBDown();
	void OnCancelInit(int param);
	void OnCancelLBDown();

	unsigned long m_partTid;
	unsigned long m_partUid;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrCardRemoveResult : public FrForm
{
public:
	DECLARE_OBJECT(FrCardRemoveResult)

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	enum eRESULT
	{
	};

#endif
	FrCardRemoveResult();
	virtual ~FrCardRemoveResult();

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	void SetRemover(unsigned long tid) { m_itemTid = tid; }
	void SetResult(eRESULT result) { m_state = result; }
	void PushList(unsigned long cardTid) { m_cardList.push_back(cardTid); }

#else
	// Fora do pch do taskmanager os setters inline nao entram: m_state/m_cardList sao preenchidos de fora.
#endif
	int m_state;
	std::list<unsigned long> m_cardList;

protected:
	virtual bool OnInit();
	virtual void OnProc(const float deltaTime);

	void OnDrawAreaOwnerDraw(int param);
	void OnConfirmInit(int param);
	void OnConfirmLBDown();

	const Bitmap* m_pPartBitmap;
	const Bitmap* m_pMaskBitmap;
	WRect m_partRect;
	WRect m_maskRect;
	int m_phase;

public:
	unsigned long m_itemTid;

private:
	DECLARE_FRESH_MSGMAP()
};
