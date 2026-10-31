#pragma once

#include <list>
#include "frform.h"
#include "tooltipmanager.h"
#include "transferableitem.h"
#include "../shared/globalgamedefine.h"

class FrButton;
class FrListBox;
class FrEdit;
class FrArea;
class FrGraphicInterface;
class WTitleFont;
class Bitmap;
class CPost_Contents;
struct FrListItem;
struct sMailInfo;
struct sMailInfoBrief;
struct sMailIncludeItem;

struct delete_letter_check
{
	delete_letter_check();

	void operator()(const sMailInfoBrief& mail);

	unsigned int count;
	std::list<unsigned long> idList;
};

class CPost_MessgeDlg : public FrForm
{
	DECLARE_OBJECT(CPost_MessgeDlg)

public:
	CPost_MessgeDlg();
	virtual ~CPost_MessgeDlg();

	void OnPostDescFrame(bool bShow);
	void OnPostMessage_ReciveUserInit(int param);
	void OnPostMessage_LetterContentsInit(int param);
	void OnPostMessage_LetterDateInit(int param);
	void SetGiftDescString(const char* a, const char* b, const char* c,
		const char* d);

protected:
	virtual void OnOK();

	unsigned long m_pad110[6];
	FrEdit* m_pReciveUser;
	FrEdit* m_pLetterContents;
	FrEdit* m_pLetterDate;
	unsigned long m_pad134;

	DECLARE_FRESH_MSGMAP()
};

class CPost_Contents : public FrForm
{
	DECLARE_OBJECT(CPost_Contents)

public:
	CPost_Contents();
	virtual ~CPost_Contents();

	void OnPostMessage_ReciveUserInit(int param);
	void OnPostMessage_SendUserInit(int param);
	void OnPostMessage_LetterContentsInit(int param);
	void OnPostMessage_LetterDateInit(int param);
	void OnPostListInit(int param);
	void OnPostListOwnerDraw(int param);
	void OnPostListBtnDown();
	void OnPostListRBtnDown();
	void OnPostListMouseMove();
	void OnPostListDClick();
	void SetData(std::list<sMailIncludeItem>* items);
	void SetData(sMailInfo* mail);

protected:
	void DrawPartsSlot(FrGraphicInterface* gfx, FrListItem* item);

	FrEdit* m_pSendUser;
	FrEdit* m_pReciveUser;
	FrEdit* m_pLetterContents;
	FrEdit* m_pLetterDate;
	FrListBox* m_pPostList;
	WTitleFont* m_pTitleFont;
	unsigned long m_pad128[2];

	DECLARE_FRESH_MSGMAP()
};

class CPost_ReciveDlg : public FrForm
{
	DECLARE_OBJECT(CPost_ReciveDlg)

public:
	struct miniBtn_t
	{
		miniBtn_t()
		{
			if (0) // HACK: emission order only: first item of wave 1 as in the original
				((IObject*)0)->DynamicCast(0);
		}

		_WRECT rect;
		const Bitmap* pNormal;
		const Bitmap* pOver;
		bool bEnable;
	};

	CPost_ReciveDlg();
	virtual ~CPost_ReciveDlg();

	bool OnPostMessageResult(int result, FrForm* pForm);
	void EnableMiniBtns(bool enable);
	void EnablePostList983(bool enable); // 983 005b14d0: liga/desliga m_pPostList
	void BuildIndexPage(int page);
	void ToServerSendData(FrListItem* item, unsigned long cmd);
	bool OnReturnGiftDlgResult(int result, FrForm* pForm);
	bool OnMoveGiftDlgResult(int result, FrForm* pForm);
	void OnLoadPostList();
	void OnReset();
	bool OnPostQuestionResult(int result, FrForm* pForm);
	void Question(int type);
	int GetItemNumInMyPocket(unsigned long typeId);
	void OnPostDescDlgCreate();
	void SetLoadPostList(std::list<sMailInfoBrief>* list);
	void SetLoadPostList();
	bool OnDeleteAllLetterResult(int result, FrForm* pForm);

protected:
	void OnRecivePostInit(int param);
	void OnRecivePostBtnUp();
	void OnSendPostInit(int param);
	void OnSendPostBtnUp();
	void OnDeleteAllLetterInit(int param);
	void OnDeleteAllLetterBtnUp();
	void OnWriteLetterInit(int param);
	void OnWriteLetterBtnUp();
	void OnPostListInit(int param);
	void OnPostListOwnerDraw(int param);
	void OnPostListBtnDown();
	void OnPostListRBtnDown();
	void OnPostListMouseMove();
	void OnPostListDClick();
	void OnPageListInit(int param);
	void OnPageListOwnerDraw(int param);
	void OnPageListBtnDown();
	void OnPageDownInit(int param);
	void OnPageDownBtnUp();
	void OnPageUpInit(int param);
	void OnPageUpBtnUp();
	bool OnEditPageEndterKey(int key);
	void OnToolTipInit(int param);
	void OnToolTipOnwerDraw(int param);
	void OnIndexAreaInit(int param);
	void OnIndexAreaOwnerDraw();
	void OnIndexListInit(int param);
	void OnIndexListLBDown();
	void OnIndexListMouseMove();
	__declspec(noinline) void
	OnLoadImage(); // HACK: emission order only (callee would be compiled before its caller otherwise)
	void OnDeleteItem(FrListItem* item);
	const char* MakeStatusString(int a, int b, int c);
	void DrawStatus2(FrGraphicInterface* gfx, FrListItem* item, short a[5],
		short b[5], float x);
	void DrawPartsSlot(FrGraphicInterface* gfx, FrListItem* item);

	FrButton* m_pRecivePost;
	FrButton* m_pSendPost;
	FrButton* m_pDeleteAll;
	FrButton* m_pReceive1Page; // 983: "receive_1page" 0x11c (005b1e90); nome meu
	FrButton* m_pWriteLetter;
	FrListBox* m_pPostList;
	FrListBox* m_pPageList;
	unsigned long m_pad128[2];
	FrButton* m_pPageDown;
	FrButton* m_pPageUp;
	unsigned long m_pad138;
	FrArea* m_pToolTipArea;
	miniBtn_t m_miniBtn[2];
	const Bitmap* m_pItemBaseN;
	const Bitmap* m_pItemBaseO;
	const Bitmap* m_pItemBaseD;
	const Bitmap* m_pItemBaseEmpty;
	const Bitmap* m_pArrivalMail;
	const Bitmap* m_pOpenMail;
	CToolTipManager m_toolTipMgr;
	FrListItem* m_pMoveItem;
	unsigned long m_pad1a4;
	int m_curIndex;
	sMailBoxListForPage m_mailList;
	CPost_Contents* m_pContents;
	int m_totalCount;
	int m_curPage;
	int m_pageDiv;
	FrListItem* m_pSelItem;
	int m_question;
	WTitleFont* m_pTitleFont;
	unsigned long m_pad1e0;
	FrArea* m_pIndexArea;
	FrListBox* m_pIndexList;
	int m_indexPageCount;
	int m_indexPage;
	unsigned char m_indexSel;
	unsigned char m_indexHover;
	unsigned char m_indexCount;
	_WRECT* m_pIndexRects;
	int m_reserveIdx;
	// 983 (A236): "receber a página" (botão receive_1page): 0x146 para cada carta da página com item, contagem das respostas
	bool m_bReceivingPage983; // +0x204: lido pelo CTaskMain (mensagem 0x321) para calar o aviso de cada carta
	int m_pageSent983;        // +0x208
	int m_pageDone983;        // +0x20c
	int m_pageFailed983;      // +0x210

public:
	void OnReceive1PageInit(int param);              // 983 005b1e90
	void OnReceive1PageBtnUp();                      // 983 005b53b0
	bool OnReceive1PageResult983(int result, FrForm* pForm); // 983 005b5000
	void ReceivePage983();                           // 983 005b45b0
	void OnReceivePageItemResult983(bool ok);        // 983 005b4660 (CTaskMain 0x322)
	bool IsReceivingPage983() const { return m_bReceivingPage983; } // CTaskMain 0x321 (0065f204)

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbBuildindexpageFunclets(); // HACK: EH funclets of the naked BuildIndexPage (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOndeleteallletteFunclets(); // HACK: EH funclets of the naked OnDeleteAllLetterResult (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOndeletealllette2Funclets(); // HACK: EH funclets of the naked OnDeleteAllLetterBtnUp (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbToserversenddataFunclets(); // HACK: EH funclets of the naked ToServerSendData (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpagedownbtnupFunclets(); // HACK: EH funclets of the naked OnPageDownBtnUp (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpageupbtnupFunclets(); // HACK: EH funclets of the naked OnPageUpBtnUp (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnreturngiftdlgrFunclets(); // HACK: EH funclets of the naked OnReturnGiftDlgResult (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnmovegiftdlgresFunclets(); // HACK: EH funclets of the naked OnMoveGiftDlgResult (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnindexlistlbdowFunclets(); // HACK: EH funclets of the naked OnIndexListLBDown (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSetloadpostlistFunclets(); // HACK: EH funclets of the naked SetLoadPostList (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnloadpostlistFunclets(); // HACK: EH funclets of the naked OnLoadPostList (nakedify)
#endif
};
