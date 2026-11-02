#include "minatl.h"
#include "vc10_layout.h"
#include <math.h>
#include "post_recivedlg.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frlistbox.h"
#include "frbutton.h"
#include "frarea.h"
#include "fredit.h"
#include "frscrollbar.h"
#include "fresh.h"
#include "projectg.h"
#include "wresrcmng.h"
#include "shareddoc.h"
#include "tooltip.h"
#include "returngiftdlg.h"
#include "ucclibrary.h"
#include "taskmanager.h"
#include "packet.h"
#include "commonutil.h"
#include "transferableitem.h"
#include "../shared/localize.h"
#include "jp/gamestringmanager.h" // GetGameString 00540a10
#include "jp/refresh/rfscenemanager.h" // SetRFMouseFocus983 00603660

// This overload is used here; keep its declaration out of the shared PCH.
const Bitmap* GetUccItemIcon(unsigned long typeId, const char* uccIndex);

// 983: 005b1c60 / 005b1c70 (funcoes locais, usercall): tipo de periodo 1..6 e sua duracao em segundos
static __declspec(noinline) bool PostItemIsTimed(int type)
{
	if (type <= 0)
		return false;
	if (type > 6)
		return false;
	return true;
}

static __declspec(noinline) int PostTimeToSeconds(int type, int count)
{
	switch (type)
	{
	case 2:
	case 3:
		return count * 3600;
	case 1:
	case 4:
	case 6:
		return count * 86400;
	case 5:
		return count * 2592000;
	}
	return 0;
}



extern Fresh* g_pFresh;
extern WResourceManager* g_resrcmng;

// ---------------------------------------------------------------------------
// CPost_ReciveDlg
// ---------------------------------------------------------------------------

IMPLEMENT_OBJECT(CPost_ReciveDlg, FrForm)

BEGIN_FRESH_MSGMAP(CPost_ReciveDlg, FrForm)
ON_FRESH_VI("recive_post", FRCMD_INIT, CPost_ReciveDlg::OnRecivePostInit)
ON_FRESH_VV("recive_post", FRCMD_LBUTTONUP, CPost_ReciveDlg::OnRecivePostBtnUp)
ON_FRESH_VI("send_post", FRCMD_INIT, CPost_ReciveDlg::OnSendPostInit)
ON_FRESH_VV("send_post", FRCMD_LBUTTONUP, CPost_ReciveDlg::OnSendPostBtnUp)
ON_FRESH_VI("receive_1page", FRCMD_INIT, CPost_ReciveDlg::OnReceive1PageInit) // 983 msgmap e11920 (A236)
ON_FRESH_VV("receive_1page", FRCMD_LBUTTONUP, CPost_ReciveDlg::OnReceive1PageBtnUp)
ON_FRESH_VI("delete_letter", FRCMD_INIT, CPost_ReciveDlg::OnDeleteAllLetterInit)
ON_FRESH_VV("delete_letter", FRCMD_LBUTTONUP,
	CPost_ReciveDlg::OnDeleteAllLetterBtnUp)
ON_FRESH_VI("write_letter", FRCMD_INIT, CPost_ReciveDlg::OnWriteLetterInit)
ON_FRESH_VV("write_letter", FRCMD_LBUTTONUP,
	CPost_ReciveDlg::OnWriteLetterBtnUp)
ON_FRESH_VI("post_slot", FRCMD_INIT, CPost_ReciveDlg::OnPostListInit)
ON_FRESH_VI("post_slot", FRCMD_OWNERDRAW, CPost_ReciveDlg::OnPostListOwnerDraw)
ON_FRESH_VV("post_slot", FRCMD_LBUTTONDOWN, CPost_ReciveDlg::OnPostListBtnDown)
ON_FRESH_VV("post_slot", FRCMD_RBUTTONDOWN, CPost_ReciveDlg::OnPostListRBtnDown)
ON_FRESH_VV("post_slot", FRCMD_MOUSEMOVE, CPost_ReciveDlg::OnPostListMouseMove)
ON_FRESH_VV("post_slot", FRCMD_DBLCLICK, CPost_ReciveDlg::OnPostListDClick)
ON_FRESH_VI("page_list", FRCMD_INIT, CPost_ReciveDlg::OnPageListInit)
ON_FRESH_VI("page_list", FRCMD_OWNERDRAW, CPost_ReciveDlg::OnPageListOwnerDraw)
ON_FRESH_VV("page_list", FRCMD_LBUTTONDOWN, CPost_ReciveDlg::OnPageListBtnDown)
ON_FRESH_VI("btn_minus", FRCMD_INIT, CPost_ReciveDlg::OnPageDownInit)
ON_FRESH_VV("btn_minus", FRCMD_LBUTTONUP, CPost_ReciveDlg::OnPageDownBtnUp)
ON_FRESH_VI("btn_pluse", FRCMD_INIT, CPost_ReciveDlg::OnPageUpInit)
ON_FRESH_VV("btn_pluse", FRCMD_LBUTTONUP, CPost_ReciveDlg::OnPageUpBtnUp)
ON_FRESH_BI("Edit_Page", FRCMD_ENTERKEY, CPost_ReciveDlg::OnEditPageEndterKey)
ON_FRESH_VI("tooltip", FRCMD_INIT, CPost_ReciveDlg::OnToolTipInit)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW, CPost_ReciveDlg::OnToolTipOnwerDraw)
ON_FRESH_VI("index_area", FRCMD_INIT, CPost_ReciveDlg::OnIndexAreaInit)
ON_FRESH_VV("index_area", FRCMD_OWNERDRAW,
	CPost_ReciveDlg::OnIndexAreaOwnerDraw)
ON_FRESH_VI("index_list", FRCMD_INIT, CPost_ReciveDlg::OnIndexListInit)
ON_FRESH_VV("index_list", FRCMD_LBUTTONDOWN, CPost_ReciveDlg::OnIndexListLBDown)
ON_FRESH_VV("index_list", FRCMD_OWNERDRAW,
	CPost_ReciveDlg::OnIndexListMouseMove)
END_FRESH_MSGMAP()

CPost_ReciveDlg::CPost_ReciveDlg()
{
	m_pContents = NULL;
	m_curIndex = 1;
	m_curPage = 1;
	m_pSelItem = NULL;
	m_question = -1;
	m_pageDiv = 1;

	tagWTITLEFONT fontInfo;
	memset(&fontInfo, 0, sizeof(fontInfo));
	static char* s_fontFile[] = { "[font_wind.jpg" };
	fontInfo.filename = s_fontFile;
	fontInfo.fontw = 16;
	fontInfo.fonth = 16;
	fontInfo.numPages = 1;
	fontInfo.texw = 128;
	fontInfo.texh = 64;
	fontInfo.pCharSet = "1234567890ym%-.?/:,";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&fontInfo);

	m_pIndexArea = NULL;
	m_pIndexList = NULL;
	m_indexPageCount = 0;
	m_indexPage = 0;
	m_indexSel = 0;
	m_indexHover = 0;
	m_indexCount = 0;
	m_pIndexRects = NULL;
	m_pDeleteAll = NULL;
	m_pWriteLetter = NULL;
	m_pPostList = NULL; // 983 005b669f (+0x124 = 0): EnablePostList983/ReceivePage983 testam NULL; sem isto liam lixo
	C_ASSERT_VC10(offsetof(CPost_ReciveDlg, m_pPostList) == 0x124);
	m_reserveIdx = -1;
	// 983 005b66bb: estado do "receber a pagina" (A236)
	m_pageSent983 = 0;
	m_pageDone983 = 0;
	m_pageFailed983 = 0;
	m_bReceivingPage983 = false;
	C_ASSERT_VC10(offsetof(CPost_ReciveDlg, m_bReceivingPage983) == 0x204);
	C_ASSERT_VC10(offsetof(CPost_ReciveDlg, m_pageFailed983) == 0x210);

	OnLoadImage();
	OnLoadPostList();
	if (0) // HACK: emission order only (no code): compiled after OnPostDescDlgCreate as in the original
		OnPostDescDlgCreate();
}

CPost_ReciveDlg::~CPost_ReciveDlg()
{
	if (m_pTitleFont)
		m_pTitleFont->Erase();
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
	if (m_pIndexRects)
	{
		delete[] m_pIndexRects;
		m_pIndexRects = NULL;
	}
}

void CPost_ReciveDlg::OnRecivePostInit(int param)
{
	m_pRecivePost = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CPost_ReciveDlg::OnRecivePostBtnUp()
{
}

void CPost_ReciveDlg::OnSendPostInit(int param)
{
	m_pSendPost = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CPost_ReciveDlg::OnSendPostBtnUp()
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
	pForm->SetMessage(K2L_Compatibility("\xb0\xf8\xbb\xe7\xc1\xdf!!! \xb3\xaa\xc1\xdf\xbf\xa1 "
					  "\xc0\xcc\xbf\xeb\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4~"),
		false);
	pForm->Open(NULL, 3);
}

// 983 005b1e90: botao "receive_1page" (+0x11c)
void CPost_ReciveDlg::OnReceive1PageInit(int param)
{
	m_pReceive1Page = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 005b53b0: pergunta (texto 0x30ed0) antes de receber os itens da pagina
void CPost_ReciveDlg::OnReceive1PageBtnUp()
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	pForm->SetMessage(GetGameString(0x30ed0), false);
	pForm->Open((FRESH_PFN_RESULT)&CPost_ReciveDlg::OnReceive1PageResult983, 3);
}

// 983 005b5000: "sim" -> zera a contagem, trava o mouse da cena e manda os pedidos
bool CPost_ReciveDlg::OnReceive1PageResult983(int result, FrForm* pForm)
{
	if (result == 1)
	{
		m_pageSent983 = 0;
		m_pageDone983 = 0;
		m_pageFailed983 = 0;
		m_bReceivingPage983 = true;
		RFSceneManager::Instance()->SetRFMouseFocus983(true);
		ReceivePage983();
	}
	return true;
}

// 983 005b45b0: 0x146 para cada carta da lista com item conhecido; nenhuma -> desfaz o estado
void CPost_ReciveDlg::ReceivePage983()
{
	if (!m_pPostList)
		return;
	for (int i = 0; i < m_pPostList->GetCurrentItemSize(true); i++)
	{
		FrListItem* item = m_pPostList->GetItem(i);
		if (!item || !item->pData)
			continue;
		sMailInfoBrief* mail = (sMailInfoBrief*)item->pData;
		if (mail->itemCount <= 0 || !ItemManager()->FindCommonItem(mail->item.dwTID))
			continue;
		ToServerSendData(item, 0x146);
		m_pageSent983++;
	}
	if (m_pageSent983 == 0)
	{
		m_bReceivingPage983 = false;
		RFSceneManager::Instance()->SetRFMouseFocus983(false);
	}
}

// 983 005b4660: resposta de cada 0x146 (CTaskMain 0x322); na ultima, um aviso so (tudo ok ou "falhou (%d)")
void CPost_ReciveDlg::OnReceivePageItemResult983(bool ok)
{
	m_pageDone983++;
	if (!ok)
		m_pageFailed983++;
	if (m_pageSent983 != m_pageDone983)
		return;

	const char* text;
	if (m_pageFailed983 == 0)
		text = K2L_Compatibility("\xc6\xed\xc1\xf6\xbf\xa1 \xc6\xf7\xc7\xd4\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb "
			"\xb8\xb6\xc0\xcc\xb7\xeb\xc0\xb8\xb7\xce \xc0\xcc\xb5\xbf\xbd\xc3\xc4\xd7\xbd\xc0\xb4\xcf\xb4\xd9."); // 983 00d0dfc4
	else
		text = MakeStr(K2L_Compatibility("\xc3\xb3\xb8\xae\xbf\xa1 \xbd\xc7\xc6\xd0 \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.(%d)"),
			m_pageFailed983); // 983 00d0cfe0
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)text, 0, 0, 0, 0));

	m_pageSent983 = 0;
	m_pageDone983 = 0;
	m_pageFailed983 = 0;
	m_bReceivingPage983 = false;
	RFSceneManager::Instance()->SetRFMouseFocus983(false);
}

void CPost_ReciveDlg::OnDeleteAllLetterInit(int param)
{
	m_pDeleteAll = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// ---------------------------------------------------------------------------
// delete_letter_check
// ---------------------------------------------------------------------------

inline delete_letter_check::delete_letter_check()
	: count(0)
{
}

void delete_letter_check::operator()(const sMailInfoBrief& mail)
{
	if (mail.itemCount == 0 && count < 100)
	{
		count++;
		idList.push_back(mail.id);
	}
}

void CPost_ReciveDlg::OnDeleteAllLetterBtnUp()
{
	sMailBoxListForPage* page =
		WSingleton<CTransferableItem>::Instance()->GetMailBoxPageList();
	delete_letter_check check = std::for_each(page->m_lMailList.begin(),
		page->m_lMailList.end(), delete_letter_check());

	FrForm* pForm;
	if (check.count != 0)
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		pForm->SetMessage(
			MakeStr(
				K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc3\xb7\xba\xce\xb5\xc7\xc1\xf6 \xbe\xca\xc0\xba \xc7\xf6\xc0\xe7 \xc6\xe4\xc0\xcc\xc1\xf6\xc0\xc7 \xc6\xed\xc1\xf6(%d\xb0\xb3)\xb8\xa6 \xb8\xf0\xb5\xce \xbb\xe8\xc1\xa6 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?\n\n\\c0xffff0000\\c(\xc1\xd6\xc0\xc7 : \xb3\xbb\xbf\xeb\xc0\xbb \xc0\xd0\xc1\xf6 \xbe\xca\xc0\xba \xc6\xed\xc1\xf6\xb5\xb5 \xb8\xf0\xb5\xce \xbb\xe8\xc1\xa6 \xb5\xcb\xb4\xcf\xb4\xd9.)\\c0xff000000\\c"),
				check.count, 0),
			false);
		pForm->Open((FRESH_PFN_RESULT)&CPost_ReciveDlg::OnDeleteAllLetterResult,
			3);
	}
	else
	{
		pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xbb\xe8\xc1\xa6\xc7\xd2 \xc6\xed\xc1\xf6\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 3);
	}
	m_reserveIdx = -1;
}

bool CPost_ReciveDlg::OnDeleteAllLetterResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		sMailBoxListForPage* page =
			WSingleton<CTransferableItem>::Instance()->GetMailBoxPageList();
		delete_letter_check check = std::for_each(page->m_lMailList.begin(),
			page->m_lMailList.end(), delete_letter_check());

		WSendPacket packet(0x147); // JP 983 (s62): 0xbe no KR 645; par 005b6d30
		packet.Encode4(check.count);
		for (std::list<unsigned long>::iterator it = check.idList.begin();
			it != check.idList.end(); ++it)
			packet.Encode4(*it);
		packet.Encode4(m_curPage);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		EnableMiniBtns(false);
		m_pSelItem = NULL;
	}
	return true;
}

void CPost_ReciveDlg::OnWriteLetterInit(int param)
{
	m_pWriteLetter = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 (005b3510, C-PKT lote 25): sem o bloqueio do conteudo 8 do 645; o "write_post" e aberto pelo CTaskMain (mensagem
// 0x29f no HandleMsgCommon), entao a mensagem vai ao ator principal da tarefa, nao ao "RealMyRoom".
void CPost_ReciveDlg::OnWriteLetterBtnUp()
{
	AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x221), 0, 0, 0, 0, 0);
	SetFadeout(true);
	Close(true); // 983 005b3510: vtable +8, sem som nem resultado do OnOK
}

void CPost_ReciveDlg::OnPageListInit(int param)
{
	m_pPageList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pPageList)
	{
		m_pPageList->ClearItem();
		for (int i = 1; i < 12; i++)
		{
			FrListItem* item = m_pPageList->AddItem((void*)(m_pageDiv * i));
			if (i == 1)
				m_pPageList->SelectItem(item, true);
		}
		m_pPageList->SetMultiSelect(false);
		m_pPageList->SetVisible(false); // 983 005b1fe0: slot +0x1c, a lista fica escondida
	}
}

void CPost_ReciveDlg::OnPageListOwnerDraw(int param)
{
	// 983: page_list fica escondido (OnPageListInit 005b1fe0); a paginacao visivel e so o index_area (C157.1)
	return;
	FrListItem* item = (FrListItem*)param;
	if (item)
	{
		int page = (int)item->pData;
		FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
		float dx = 7.5f;
		float dy = 0.0f;
		if (page != 0)
		{
			if (page > m_totalCount)
				item->selected = false;
			gfx->SetTextColor(0xff959494, 0xffffffff);
			if (item->selected)
			{
				gfx->SetTextStyle(1);
				gfx->SetTextColor(0xff000000, 0xffffffff);
			}
			else if (item->underCursor)
			{
				dy = -0.5f;
				dx = 7.0f;
			}
			else
				gfx->SetTextStyle(0);

			if (page != m_totalCount)
			{
				gfx->Print(WPoint(dx + item->pos.x, dy + item->pos.y), 1, "%d ",
					page);
				gfx->SetTextStyle(0);
				gfx->SetTextColor(0xff959494, 0xffffffff);
				gfx->Print(WPoint(item->pos.x + 12.0f, item->pos.y), 1, "|");
			}
			else
				gfx->Print(WPoint(dx + item->pos.x, dy + item->pos.y), 1, "%d",
					page);
		}
	}
}

void CPost_ReciveDlg::OnPageListBtnDown()
{
	FrListItem* sel = m_pPageList->GetItemUnderCursor();
	if (sel == NULL)
		return;
	int page = (int)sel->pData;
	int total = m_totalCount;
	if (page < 1)
		page = 1;
	else if (page > total)
		page = total;
	m_curIndex = page;
	OnLoadPostList();
}

void CPost_ReciveDlg::OnPostListInit(int param)
{
	m_pPostList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pPostList)
	{
		m_pPostList->SetMultiSelect(false);
		m_pPostList->UseDummy(true);
		m_pPostList->SetPushSound(NULL);
		m_pPostList->AddNoRButtonRect(*(WRect*)&m_miniBtn[1].rect, 1);
		m_pPostList->AddNoRButtonRect(*(WRect*)&m_miniBtn[0].rect, 0);
	}
}

void CPost_ReciveDlg::OnPostListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;

	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	const Bitmap* base;
	if (!item->pData)
	{
		base = m_pItemBaseEmpty;
	}
	else
	{
		unsigned long textColor;
		if (item->selected)
		{
			base = m_pItemBaseD;
			textColor = 0xffff0000;
		}
		else
		{
			textColor = 0xff000000;
			base = item->underCursor ? m_pItemBaseO : m_pItemBaseN;
		}
		gfx->SetTextColor(textColor, 0xffffffff);
	}
	gfx->SetTextStyle(0);
	if (base)
	{
		WRect rect(item->pos.x + 0.0f, item->pos.y + 0.0f, base->Width(), base->Height());
		gfx->DrawTexture(base, rect, 0xffffffff, 0);
	}

	if (!item->pData)
		return;

	const char* name = K2L_Compatibility("\xc6\xed\xc1\xf6");
	float iconX = 0.0f;
	float iconY = 0.0f;
	sMailInfoBrief* mail = (sMailInfoBrief*)item->pData;
	if (!mail)
		return;
	unsigned long tid = mail->item.dwTID;
	const Bitmap* icon;
	int count;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(tid);
	if (common && mail->itemCount != 0)
	{
		name = common->Name;
		count = ((tid & 0xfc000000) == 0x24000000) ? mail->itemCount
		                                           : mail->item.iCount;
		icon = g_pFresh->GetBitmap(common->Icon);
	}
	else
	{
		icon = mail->bRead ? m_pOpenMail : m_pArrivalMail;
		count = 0;
	}

	if (icon)
	{
		switch (tid >> 26)
		{
		case 1:
		case 2:
		case 8:
		case 0xf:
		case 0x10:
		case 0x1c:
			iconX = 7.0f;
			iconY = 2.0f;
			break;
		case 0xb:
			iconX = 9.0f;
			iconY = 12.0f;
			break;
		case 0xe:
			if ((tid & 0x3c00000) == 0x800000)
			{
				iconX = 7.0f;
				iconY = 0.0f;
			}
			else
			{
				iconX = 16.0f;
				iconY = 27.0f;
			}
			break;
		default:
			iconX = 16.0f;
			iconY = 12.0f;
			break;
		}
	}
	if (IsLocalContent(S4_UCC) && IsUccClothes(tid) && mail->itemCount > 0)
		icon = GetUccItemIcon(mail->item.dwTID, mail->item.csUCCIndex);
	if (icon && gfx)
	{
		WRect rect(item->pos.x + iconX + 0.0f, item->pos.y + iconY + 0.0f, icon->Width(),
			icon->Height());
		gfx->DrawTexture(icon, rect, 0xffffffff, 0);
	}

	miniBtn_t btn = (mail->itemCount > 0) ? m_miniBtn[0] : m_miniBtn[1];
	WRect btnRect(m_miniBtn[1].rect.x + item->pos.x,
		m_miniBtn[1].rect.y + item->pos.y, m_miniBtn[1].rect.w,
		m_miniBtn[1].rect.h);
	if (!btn.bEnable)
	{
		gfx->DrawTexture(btn.pNormal, WRect(btnRect.x + 0.0f, btnRect.y + 0.0f, btnRect.w, btnRect.h), 0x88ffffff, 0);
	}
	else if (!item->underCursor ||
		!btnRect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		gfx->DrawTexture(btn.pNormal, WRect(btnRect.x + 0.0f, btnRect.y + 0.0f, btnRect.w, btnRect.h), 0xffffffff, 0);
	}
	else
	{
		gfx->DrawTexture(btn.pOver, WRect(btnRect.x + 0.0f, btnRect.y + 0.0f, btnRect.w, btnRect.h), 0xffffffff, 0);
		CIconToolTip::GetInstance()->SetToolTip(RMR_DELETE);
	}

	if (PostItemIsTimed(mail->item.btItemType))
	{
		gfx->SetTextStyle(2);
		gfx->SetTextColor(0xff000000, 0xffffffff);
		WPoint pos(item->pos.x + 72.0f, item->pos.y + 72.0f);
		IFF_ITEM_COMMON* timed;
		if ((tid & 0xfc000000) == 0x40000000 &&
			(timed = ItemManager()->FindCommonItem(tid)) != NULL &&
			timed->TimeFlag == 0)
		{
			gfx->Print(WPoint(item->pos.x + 72.0f + 0.0f, item->pos.y + 72.0f + 0.0f), 2, K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"));
		}
		else
		{
			int secs = PostTimeToSeconds(mail->item.btItemType,
				mail->item.iTimeCount);
			int hours = (int)ceil((double)secs / 3600.0);
			int days = hours / 24;
			int rem = hours % 24;
			if (days == 0 && rem == 0)
				gfx->Print(WPoint(item->pos.x + 72.0f + 0.0f, item->pos.y + 72.0f + 0.0f), 2,
					K2L_Compatibility("1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n"));
			else if (days != 0 && rem == 0)
				gfx->Print(WPoint(item->pos.x + 72.0f + 0.0f, item->pos.y + 72.0f + 0.0f), 2, K2L_Compatibility("%d\xc0\xcf"), days);
			else
				gfx->Print(WPoint(item->pos.x + 72.0f + 0.0f, item->pos.y + 72.0f + 0.0f), 2,
					K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), days,
					rem);
		}
	}
	else if (count > 1 && tid != 0x1a000080 && tid != 0x1a000081 &&
		tid != 0x1a000082)
	{
		const char* digits = MakeStr("%d", count);
		float x = item->pos.x + 72.0f;
		for (unsigned int i = 0; i < strlen(digits); ++i)
			x -= m_pTitleFont->GetCharWidth(g_view, digits[i] - '0');
		m_pTitleFont->Print(g_view, x, item->pos.y + 42.0f, digits, 0,
			0xffffffff, NULL);
	}

	if (mail->itemCount == -1)
		gfx->Print(WPoint(item->pos.x + 182.0f + 0.0f, item->pos.y + 23.0f + 0.0f), 2,
			K2L_Compatibility("%d"), tid);
	else if (mail->itemCount > 1)
		gfx->Print(WPoint(item->pos.x + 182.0f + 0.0f, item->pos.y + 23.0f + 0.0f), 2,
			K2L_Compatibility(" + %d\xb0\xb3"), mail->itemCount - 1);
	gfx->SetTextStyle(0);
	gfx->Print(WPoint(item->pos.x + 182.0f + 0.0f, item->pos.y + 7.0f + 0.0f), 2, "%s", name);
}


void CPost_ReciveDlg::OnReset()
{
	m_curIndex = 1;
	OnLoadPostList();
}

void CPost_ReciveDlg::DrawStatus2(FrGraphicInterface* gfx, FrListItem* item,
	short a[5], short b[5], float x)
{
	float yoff = 0.0f;
	for (int i = 0; i < 5; i++)
	{
		if (a[i] != 0 || b[i] != 0)
		{
			gfx->SetTextColor(0xff0000ff, 0xffffffff);
			WPoint pos(x + 182.0f, yoff + x + 36.0f);
			pos += item->pos;
			gfx->Print(pos, 2, MakeStatusString(i, a[i], b[i]));
			yoff += 14.0f;
		}
	}
}

const char* CPost_ReciveDlg::MakeStatusString(int a, int b, int c)
{
	const char* names[5] = { K2L_Compatibility("\xc6\xc4\xbf\xf6"),
		K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1"),
		K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5"),
		K2L_Compatibility("\xbd\xba\xc7\xc9"),
		K2L_Compatibility("\xc4\xbf\xba\xea") };
	const char* slot = K2L_Compatibility("\xbd\xbd\xb7\xd4");
	if (b != 0)
	{
		if (c != 0)
			return MakeStr("%s+%d/%s+%d", names[a], b, slot, c);
		return MakeStr("%s+%d", names[a], b);
	}
	if (c != 0)
		return MakeStr("%s%s+%d", names[a], slot, c);
	return "";
}

void CPost_ReciveDlg::OnPostListBtnDown()
{
	FrListItem* sel = m_pPostList->GetItemUnderCursor();
	if (!sel || !sel->pData)
		return;
	m_pSelItem = sel;
	sMailInfoBrief* mail = (sMailInfoBrief*)sel->pData;
	if (!mail)
		return;

	WRect btn;
	btn.x = sel->pos.x + m_miniBtn[1].rect.x;
	btn.y = sel->pos.y + m_miniBtn[1].rect.y;
	btn.w = m_miniBtn[1].rect.w;
	btn.h = m_miniBtn[1].rect.h;
	if (btn.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		if (mail->itemCount > 0)
		{
			if (!ItemManager()->FindCommonItem(mail->item.dwTID))
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
					this, "notify", NULL);
				pForm->SetMessage(
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xbf\xa1 "
						"\xc3\xb7\xba\xce\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb"
						"\xc0\xba \xc7\xf6\xc0\xe7 \xbf\xc5\xb1\xe6\xbc\xf6 "
						"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				pForm->Open(NULL, 3);
				m_reserveIdx = sel->idx;
				return;
			}
			if (m_miniBtn[0].bEnable)
				Question(0);
		}
		else if (m_miniBtn[1].bEnable)
			Question(1);
		// 983 (005b5dc8): dentro do mini botao nao abre a carta mesmo com o botao desligado; so guarda o indice
		m_reserveIdx = sel->idx;
		return;
	}
	m_question = -1;
	ToServerSendData(sel, 0x144);
}

void CPost_ReciveDlg::OnPostListRBtnDown()
{
}

void CPost_ReciveDlg::Question(int type)
{
	if (type != -1)
	{
		m_question = type;
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		if (type == 0)
			pForm->SetMessage(K2L_Compatibility(
				"\xbc\xb1\xb9\xb0\xc0\xbb "
				"\xb8\xb6\xc0\xcc\xb7\xeb\xc0\xb8\xb7\xce "
				"\xbf\xc5\xb1\xe2\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
				false);
		else
			pForm->SetMessage(K2L_Compatibility(
				"\xc6\xed\xc1\xf6\xb8\xa6 "
				"\xbb\xe8\xc1\xa6\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
				false);
		pForm->Open((FRESH_PFN_RESULT)&CPost_ReciveDlg::OnPostQuestionResult,
			2);
		EnableMiniBtns(false);
		EnablePostList983(false);
	}
}

bool CPost_ReciveDlg::OnPostQuestionResult(int result, FrForm* pForm)
{
	EnableMiniBtns(true);
	EnablePostList983(true);
	// 983 005b50a0: ramo result == 1 primeiro, cancelamento no fim
	if (result == 1)
	{
		if (m_question != 0)
			ToServerSendData(m_pSelItem, 0x147);
		else
			ToServerSendData(m_pSelItem, 0x146);
		return true;
	}
	m_pSelItem = NULL;
	m_reserveIdx = -1;
	return true;
}

// JP 983 (005b14d0, sem nome; metodo novo do 983): liga/desliga a lista de cartas (m_pPostList, +0x124). Estatica ate a A
// declarar no cabecalho (pedido C -> A); chamada tambem em 005b50a0, 005b5430 e CTaskMain::HandleMsgCommon (0065f1e9).
void CPost_ReciveDlg::EnablePostList983(bool enable)
{
	if (m_pPostList)
		m_pPostList->Enable(enable);
}

void CPost_ReciveDlg::ToServerSendData(FrListItem* item, unsigned long cmd)
{
	if (!item || !item->pData)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba "
			"\xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb"
			"\xc0\xd4\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 3);
		return;
	}

	unsigned long* data = (unsigned long*)item->pData;
	// JP 983 (005b3d74, C-PKT lote 10): o id ja vem do 983 (0x144 ler, 0x146 receber, 0x147 apagar);
	// so o 0x147 leva o 1 e a pagina; os outros mandam so o id da carta.
	if (cmd == 0x147)
	{
		WSendPacket packet((enumClientPacket)cmd); // par 005b6d30
		packet.Encode4(1);
		packet.Encode4(*data);
		packet.Encode4(m_curPage);
		packet.Send(TO_GAME);
	}
	else
	{
		WSendPacket packet(cmd);
		packet.Encode4(*data);
		packet.Send(TO_GAME);
	}

	// 983 (005b3df0): pela tarefa ao ator principal (vtable+0x30), nao por GetActor("RealMyRoom"); e a lista de cartas
	// tambem fica desligada ate a resposta (005b14d0)
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	EnableMiniBtns(false);
	EnablePostList983(false);
	if (cmd != 0x144)
		m_pSelItem = NULL;
}

void CPost_ReciveDlg::OnPostListMouseMove()
{
}

void CPost_ReciveDlg::OnPostDescDlgCreate()
{
	sMailInfo* mail = WSingleton<CTransferableItem>::Instance()->GetMailInfo();
	if (mail)
	{
		m_pContents = CreateForm<CPost_Contents>(g_pFresh->GetManager(), this,
			"post_contents", NULL);
		if (m_pContents)
		{
			FrForm::ms_bHasTail = false;
			m_pContents->SetData(mail);
			m_pContents->Open(
				(FRESH_PFN_RESULT)&CPost_ReciveDlg::OnPostMessageResult, 1);
			m_pContents->SetFadeout(true);
		}
	}
}

bool CPost_ReciveDlg::OnPostMessageResult(int result, FrForm* pForm)
{
	// 983 005b1430: dado lido de novo e testado antes de marcar a carta como lida
	if (!m_pSelItem || !m_pSelItem->pData)
		return true;
	sMailInfoBrief* mail = (sMailInfoBrief*)m_pSelItem->pData;
	if (!mail)
		return true;
	m_pSelItem->selected = false;
	mail->bRead = true;
	m_pSelItem = NULL;
	return true;
}

void CPost_ReciveDlg::OnPageDownInit(int param)
{
	m_pPageDown = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CPost_ReciveDlg::OnPageDownBtnUp()
{
	// 983 005b3ee0: teste sem sinal (jbe) e pagina relida depois do BuildIndexPage
	if ((unsigned int)m_indexPage > 0)
	{
		m_indexPage--;
		BuildIndexPage(m_indexPage);
		int page = m_indexPage;
		m_indexSel = 0;
		WSendPacket packet(0x143); // JP 983 (s62): 0xbc no KR 645; par 005b3ee0/005b3fd0
		packet.Encode4(page * 5 + 1);
		packet.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		EnableMiniBtns(false);
	}
}

void CPost_ReciveDlg::OnPageUpInit(int param)
{
	m_pPageUp = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CPost_ReciveDlg::OnPageUpBtnUp()
{
	// 983 005b3fd0: incremento no proprio membro e pagina relida depois do BuildIndexPage
	if ((unsigned int)(m_indexPage + 1) < (unsigned int)m_indexPageCount)
	{
		m_indexPage++;
		BuildIndexPage(m_indexPage);
		int page = m_indexPage;
		m_indexSel = 0;
		WSendPacket packet(0x143); // JP 983 (s62): 0xbc no KR 645; par 005b3ee0/005b3fd0
		packet.Encode4(page * 5 + 1);
		packet.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		EnableMiniBtns(false);
	}
}

void CPost_ReciveDlg::OnPostListDClick()
{
}

bool CPost_ReciveDlg::OnEditPageEndterKey(int key)
{
	return true;
}

int CPost_ReciveDlg::GetItemNumInMyPocket(unsigned long typeId)
{
	int count = 0;

	if (typeId == 0 || (typeId & 0xfc000000) == 0x4000000)
	{
		for (std::map<unsigned int, sCharacterInfo>::const_iterator it =
				 Doc()->m_charMap.begin();
			it != Doc()->m_charMap.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).second.tid) &&
				(typeId == 0 || (*it).second.tid == typeId))
				count++;
		}
	}

	if (typeId == 0 || (typeId & 0xfc000000) == 0x8000000)
	{
		for (std::list<sItemInfo>::iterator it = Doc()->m_partsList.begin();
			it != Doc()->m_partsList.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(typeId == 0 || (*it).tid == typeId))
				count++;
		}
	}

	if (typeId == 0 || (typeId & 0xfc000000) == 0x18000000)
	{
		for (std::list<sItemInfo>::iterator it = Doc()->m_myItemList.begin();
			it != Doc()->m_myItemList.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(typeId == 0 || (*it).tid == typeId))
				count++;
		}
	}

	if (typeId == 0 || (typeId & 0xfc000000) == 0x1c000000)
	{
		for (std::map<unsigned int, sCaddieInfo>::const_iterator it =
				 Doc()->m_caddieMap.begin();
			it != Doc()->m_caddieMap.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).second.tid) &&
				(typeId == 0 || (*it).second.tid == typeId))
				count++;
		}
	}

	// the original resets the running total here (mascots are not counted)
	if (typeId == 0 || (typeId & 0xfc000000) == 0x20000000)
		count = 0;

	if (typeId == 0 || (typeId & 0xfc000000) == 0x3c000000)
	{
		for (std::map<unsigned int, sCharacterInfo>::const_iterator it =
				 Doc()->m_charMap.begin();
			it != Doc()->m_charMap.end(); ++it)
		{
			IFF_STRUCT::sHairStyle* hair = ItemManager()->FindHairStyle(typeId);
			if (hair && hair->cHairID == (*it).second.hairClr &&
				((unsigned char)hair->cCharID | 0x4000000) == (*it).second.tid)
				count++;
		}
	}

	if (typeId == 0 || (typeId & 0xfc000000) == 0x10000000)
	{
		for (std::map<unsigned int, sItemInfo>::const_iterator it =
				 Doc()->m_clubSetMap.begin();
			it != Doc()->m_clubSetMap.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).second.tid) &&
				(typeId == 0 || (*it).second.tid == typeId))
				count++;
		}
	}

	if (typeId == 0 || (typeId & 0xfc000000) == 0x14000000)
	{
		for (std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
			it != Doc()->m_ballList.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(typeId == 0 || (*it).tid == typeId))
				count++;
		}
	}

	unsigned long group = typeId >> 26;
	if (group == 9 && (typeId & 0x3e00000) == 0x200000)
	{
		IFF_STRUCT::sSetItem* set = ItemManager()->FindSetItem(typeId);
		for (unsigned char i = 0; i < set->nElems; ++i)
		{
			if (typeId == 0 || (set->ElemIds[i] & 0xfc000000) != 0x4000000)
			{
				for (std::list<sItemInfo>::iterator it =
						 Doc()->m_partsList.begin();
					it != Doc()->m_partsList.end(); ++it)
				{
					if (ItemManager()->FindCommonItem((*it).tid) &&
						(typeId == 0 || (*it).tid == set->ElemIds[i]))
						count++;
				}
			}
		}
	}

	if (typeId == 0 || group == 0xe)
	{
		for (std::list<sItemInfo>::iterator it = Doc()->m_cutinList.begin();
			it != Doc()->m_cutinList.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(typeId == 0 || (*it).tid == typeId))
				count++;
		}
	}
	return count;
}

bool CPost_ReciveDlg::OnReturnGiftDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		FrReturnGiftDlg* dlg = DYNAMIC_CAST(FrReturnGiftDlg, pForm);
		if (dlg)
		{
			WSendPacket packet(0x4d);
			packet.Encode4(dlg->m_giftIdx);
			packet.Encode4(dlg->m_typeId);
			packet.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			EnableMiniBtns(false);
		}
	}
	return true;
}

bool CPost_ReciveDlg::OnMoveGiftDlgResult(int result, FrForm* pForm)
{
	if (result == 1 && m_pMoveItem)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xbc\xb1\xb9\xb0 \xbf\xc5\xb1\xe2\xb1\xe2\xb8\xa6 "
			"\xbf\xe4\xc3\xbb\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 3);
		WSendPacket packet(0x24);
		packet.Encode4(*(unsigned long*)m_pMoveItem->pData);
		packet.Encode4(*((unsigned long*)m_pMoveItem->pData + 1));
		packet.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		EnableMiniBtns(false);
	}
	return true;
}

void CPost_ReciveDlg::OnLoadImage()
{
	m_miniBtn[0].pNormal = g_pFresh->RegisterBitmap("rmr_giftmove_btn_n");
	m_miniBtn[0].pOver = g_pFresh->RegisterBitmap("rmr_giftmove_btn_o");
	_WRECT rect0;
	rect0.x = 103.0f;
	rect0.y = 66.0f;
	rect0.w = (float)(unsigned long)m_miniBtn[0].pNormal->bi->bmiHeader.biWidth;
	rect0.h =
		(float)(unsigned long)m_miniBtn[0].pNormal->bi->bmiHeader.biHeight;
	m_miniBtn[0].rect = rect0;
	m_miniBtn[0].bEnable = true;

	m_miniBtn[1].pNormal = g_pFresh->RegisterBitmap("rmr_delete_btn_n");
	m_miniBtn[1].pOver = g_pFresh->RegisterBitmap("rmr_delete_btn_o");
	_WRECT rect1;
	rect1.x = 144.0f;
	rect1.y = 66.0f;
	rect1.w = (float)(unsigned long)m_miniBtn[1].pNormal->bi->bmiHeader.biWidth;
	rect1.h =
		(float)(unsigned long)m_miniBtn[1].pNormal->bi->bmiHeader.biHeight;
	m_miniBtn[1].rect = rect1;
	m_miniBtn[1].bEnable = true;

	m_pItemBaseN = g_pFresh->RegisterBitmap("item_base_n");
	m_pItemBaseO = g_pFresh->RegisterBitmap("item_base_o");
	m_pItemBaseD = g_pFresh->RegisterBitmap("item_base_d");
	m_pItemBaseEmpty = g_pFresh->RegisterBitmap("item_base_empty");
	m_pArrivalMail = g_pFresh->RegisterBitmap("rmr_Arrival_mail");
	m_pOpenMail = g_pFresh->RegisterBitmap("rmr_Open_mail");
	if (0) // HACK: emission order only (no code): references a late template so it is compiled one wave late as in the original
		m_mailList.m_lMailList.size();
}

void CPost_ReciveDlg::OnToolTipInit(int param)
{
	m_pToolTipArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CPost_ReciveDlg::OnToolTipOnwerDraw(int param)
{
	m_toolTipMgr.Process();
	// 983 005b4330: so os tips da propria lista (RMR_DELETE/RMR_RM_WITHDRAWAL = 0x4a/0x4b);
	// o 0x40..0x41 era a numeracao do 645
	int tip = CIconToolTip::GetInstance()->GetCurTipIndex();
	if (tip == RMR_DELETE || tip == RMR_RM_WITHDRAWAL)
		CIconToolTip::GetInstance()->ClearToolTip();
}

void CPost_ReciveDlg::BuildIndexPage(int page)
{
	m_indexCount = 0;
	if (m_pIndexRects)
	{
		delete[] m_pIndexRects;
		m_pIndexRects = NULL;
	}

	if (m_pageDiv < 1)
	{
		m_indexPageCount = 0;
		m_indexCount = 0;
		return;
	}

	int rem = m_totalCount % 5;
	m_indexPageCount = m_totalCount / 5;
	if (rem != 0)
		m_indexPageCount = m_totalCount / 5 + 1;

	if (m_totalCount - page * 5 >= 5)
		m_indexCount = 5;
	else
	{
		if (rem != 0)
			m_indexCount = rem;
		if (m_indexCount < 1)
			return;
	}

	m_pIndexRects = new _WRECT[m_indexCount];
	float half = m_indexCount * 30.0f * 0.5f;
	float base = 243.0f - half;
	if (m_pIndexRects)
	{
		for (unsigned int i = 0; i < m_indexCount; i++)
		{
			WRect rc(i * 30.0f + base, 300.0f, i * 30.0f + base + 30.0f, 310.0f);
			m_pIndexRects[i] = rc;
		}
	}
}


void CPost_ReciveDlg::EnableMiniBtns(bool enable)
{
	for (int i = 0; i < 2; i++)
		m_miniBtn[i].bEnable = enable;

	if (m_pDeleteAll)
		m_pDeleteAll->Enable(enable);
	if (m_pWriteLetter)
		m_pWriteLetter->Enable(enable);
}

void CPost_ReciveDlg::OnIndexAreaInit(int param)
{
	m_pIndexArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CPost_ReciveDlg::OnIndexAreaOwnerDraw()
{
	if (m_pIndexRects)
	{
		FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
		if (gfx)
		{
			for (int i = 0; i < m_indexCount; i++)
			{
				float x = m_rect.x;
				float y = m_rect.y;
				if (i == m_indexSel)
				{
					gfx->SetTextColor(0xffff0000, 0xffffffff);
					gfx->SetTextStyle(1);
				}
				else if (i == m_indexHover)
				{
					gfx->SetTextColor(0xffaa1111, 0xffffffff);
					gfx->SetTextStyle(1);
				}
				else
				{
					gfx->SetTextColor(0xff000000, 0xffffffff);
					gfx->SetTextStyle(0);
				}
				WPoint pos;
				pos.x = x + m_pIndexRects[i].x + 15.0f;
				pos.y = y + m_pIndexRects[i].y;
				gfx->Print(pos, 1, "%d", m_indexPage * 5 + i + 1);
				if (i != 0)
				{
					gfx->SetTextColor(0xffaaaaaa, 0xffffffff);
					gfx->SetTextStyle(0);
					pos.x = x + m_pIndexRects[i].x;
					pos.y = y + m_pIndexRects[i].y;
					gfx->Print(pos, 1, "|");
				}
				gfx->SetTextColor(0xff000000, 0xffffffff);
				gfx->SetTextStyle(0);
			}
		}
	}
}

void CPost_ReciveDlg::OnIndexListInit(int param)
{
	m_pIndexList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CPost_ReciveDlg::OnIndexListLBDown()
{
	if (m_indexCount != 0 && m_pIndexRects)
	{
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		for (unsigned char i = 0; i < m_indexCount; i++)
		{
			WPoint pos(m_rect.x, m_rect.y);
			const _WRECT& src = m_pIndexRects[i];
			WRect rc(src.x, src.y, src.w, src.h);
			rc.x += pos.x;
			rc.y += pos.y;
			rc.w += pos.x;
			rc.h += pos.y;
			if (mouse.x >= rc.x && mouse.x <= rc.w && mouse.y >= rc.y && mouse.y <= rc.h)
			{
				m_indexSel = i;
				WSendPacket packet(0x143);
				packet.Encode4(m_indexPage * 5 + 1 + i);
				packet.Send(TO_GAME);
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
				EnableMiniBtns(false);
				return;
			}
		}
	}
}

void CPost_ReciveDlg::OnIndexListMouseMove()
{
	if (m_indexCount != 0 && m_pIndexRects)
	{
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		for (unsigned char i = 0; i < m_indexCount; i++)
		{
			WPoint pos(m_rect.x, m_rect.y);
			const _WRECT& src = m_pIndexRects[i];
			WRect rc(src.x, src.y, src.w, src.h);
			rc.x += pos.x;
			rc.y += pos.y;
			rc.w += pos.x;
			rc.h += pos.y;
			if (mouse.x >= rc.x && mouse.x <= rc.w && mouse.y >= rc.y && mouse.y <= rc.h)
			{
				m_indexHover = i;
				return;
			}
		}
		m_indexHover = 0xff;
	}
}

void CPost_ReciveDlg::OnLoadPostList()
{
	WSendPacket packet(0x143);
	packet.Encode4(m_curPage);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	EnableMiniBtns(false);
}

void CPost_ReciveDlg::SetLoadPostList()
{
	sMailBoxListForPage* page =
		WSingleton<CTransferableItem>::Instance()->GetMailBoxPageList();
	// Original reads +0x10 as the page count and +0x0c as the selected page.
	m_totalCount = page->iCurPage;
	m_curPage = page->iTotalPage;
	m_pPageList->ClearItem();
	for (int i = 1; i <= m_totalCount; i++)
	{
		FrListItem* item = m_pPageList->AddItem((void*)(m_pageDiv * i));
		if (i == m_curPage)
			m_pPageList->SelectItem(item, true);
	}
	SetLoadPostList(&page->m_lMailList);
	BuildIndexPage(m_indexPage);
	if ((unsigned)m_totalCount < (unsigned)(m_indexPage * 5 + 1 + m_indexSel) &&
		m_totalCount != 0)
	{
		// 983 (005b6b00): vai para a ultima pagina que sobrou (o 645 so decrementava e podia ficar fora do indice)
		m_indexSel = m_indexCount - 1;
		WSendPacket packet(0x143); // JP 983 (s62): 0xbc no KR 645; par 005b3ee0/005b3fd0
		packet.Encode4(m_indexSel + 1);
		packet.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		EnableMiniBtns(false);
	}
}


void CPost_ReciveDlg::SetLoadPostList(std::list<sMailInfoBrief>* list)
{
	m_pPostList->ClearItem();
	sMailInfoBrief dummy;
	if (list->size() != 0)
	{
		for (std::list<sMailInfoBrief>::iterator it = list->begin();
			it != list->end(); ++it)
		{
			if (IsLocalContent(S4_CARD_SYSTEM))
			{
				IFF_STRUCT::sPart* part;
				if ((it->item.dwTID & 0xfc000000) == 0x8000000 &&
					it->itemCount > 0 &&
					(part = CSharedDoc::Instance()->m_itemManager.FindPart(
						 it->item.dwTID)) != NULL)
				{
					it->item.CharacterSlotNum = part->CharacterSlot;
					it->item.CaddieSlotNum = part->CaddieSlot;
					it->item.NpcSlotNum983 = part->NpcSlot983; // 983 005b6801: +0xb4 <- sPart+0x22c
				}
				else
				{
					it->item.CharacterSlotNum = 0;
					it->item.CaddieSlotNum = 0;
					it->item.NpcSlotNum983 = 0;
				}
			}
			FrListItem* item = m_pPostList->AddItem(&*it);
			item->AddNoRBtnIdx(0);
			item->AddNoRBtnIdx(1);
		}
		for (int i = (int)list->size(); i < 20; i++)
			m_pPostList->AddItem(NULL);
		m_pPostList->ResetPrevTopRow(); // 983: [+0xec] = -1
		int row = (m_reserveIdx - 1) / 2;
		if (m_reserveIdx == -1 || row < 0)
			m_pPostList->GetScrollBar()->ScrollToFirst();
		else
			m_pPostList->GetScrollBar()->SetCurTopRow(row);
		m_reserveIdx = -1;
	}
}

void CPost_ReciveDlg::OnDeleteItem(FrListItem* item)
{
	ToServerSendData(item, 0x147);
}

// ---------------------------------------------------------------------------
// CPost_MessgeDlg
// ---------------------------------------------------------------------------

IMPLEMENT_OBJECT(CPost_MessgeDlg, FrForm)

BEGIN_FRESH_MSGMAP(CPost_MessgeDlg, FrForm)
ON_FRESH_VI("recive_user", FRCMD_INIT,
	CPost_MessgeDlg::OnPostMessage_ReciveUserInit)
ON_FRESH_VI("letter_contents", FRCMD_INIT,
	CPost_MessgeDlg::OnPostMessage_LetterContentsInit)
ON_FRESH_VI("letter_date", FRCMD_INIT,
	CPost_MessgeDlg::OnPostMessage_LetterDateInit)
END_FRESH_MSGMAP()

CPost_MessgeDlg::CPost_MessgeDlg()
{
	m_pReciveUser = NULL;
	m_pLetterContents = NULL;
	m_pLetterDate = NULL;
}

CPost_MessgeDlg::~CPost_MessgeDlg()
{
}

void CPost_MessgeDlg::OnPostMessage_ReciveUserInit(int param)
{
	m_pReciveUser = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_MessgeDlg::OnPostMessage_LetterContentsInit(int param)
{
	m_pLetterContents = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_MessgeDlg::OnPostMessage_LetterDateInit(int param)
{
	m_pLetterDate = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_MessgeDlg::OnPostDescFrame(bool bShow)
{
	m_pReciveUser->SetElementFadeOut(!bShow);
	m_pLetterContents->SetElementFadeOut(!bShow);
	m_pLetterDate->SetElementFadeOut(!bShow);
}

void CPost_MessgeDlg::SetGiftDescString(const char* a, const char* b,
	const char* c, const char* d)
{
	if (m_pReciveUser->GetLine(1, false))
		m_pReciveUser->ClearLine();
	if (m_pLetterContents->GetLine(1, false))
		m_pLetterContents->ClearLine();
	if (m_pLetterDate->GetLine(1, false))
		m_pLetterDate->ClearLine();

	if (a == NULL)
	{
		m_pLetterContents->AddText(
			MakeStr(K2L_Compatibility("\xb5\xb5\xc2\xf8\xc7\xd1 \xbf\xec\xc6\xed\xb9\xb0\xc0\xcc "
					"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")),
			false, true);
		return;
	}

	m_pReciveUser->AddText(MakeStr(K2L_Compatibility("%s"), a), false, true);
	if (strlen(b) > 2)
		m_pLetterContents->AddText(MakeStr(K2L_Compatibility("%s"), b), false, true);
	else
		m_pLetterContents->AddText(
			MakeStr(
				K2L_Compatibility("\xb5\xb5\xc2\xf8\xc7\xd1 \xbf\xec\xc6\xed\xb9\xb0\xbf\xa1 "
				"\xb3\xbb\xbf\xeb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")),
			false, true);
	m_pLetterDate->AddText(
		MakeStr(K2L_Compatibility("%s \\c0xff7a6625\\c(%s)\\c0xff000000\\c"), c, d), false, true);
}

void CPost_MessgeDlg::OnOK()
{
	SetFadeout(true);
	FrForm::OnOK();
}

// ---------------------------------------------------------------------------
// CPost_Contents
// ---------------------------------------------------------------------------

IMPLEMENT_OBJECT(CPost_Contents, FrForm)

BEGIN_FRESH_MSGMAP(CPost_Contents, FrForm)
ON_FRESH_VI("recive_user", FRCMD_INIT,
	CPost_Contents::OnPostMessage_ReciveUserInit)
ON_FRESH_VI("send_user", FRCMD_INIT, CPost_Contents::OnPostMessage_SendUserInit)
ON_FRESH_VI("letter_contents", FRCMD_INIT,
	CPost_Contents::OnPostMessage_LetterContentsInit)
ON_FRESH_VI("letter_date", FRCMD_INIT,
	CPost_Contents::OnPostMessage_LetterDateInit)
ON_FRESH_VI("post_slot", FRCMD_INIT, CPost_Contents::OnPostListInit)
ON_FRESH_VI("post_slot", FRCMD_OWNERDRAW, CPost_Contents::OnPostListOwnerDraw)
ON_FRESH_VV("post_slot", FRCMD_LBUTTONDOWN, CPost_Contents::OnPostListBtnDown)
ON_FRESH_VV("post_slot", FRCMD_RBUTTONDOWN, CPost_Contents::OnPostListRBtnDown)
ON_FRESH_VV("post_slot", FRCMD_MOUSEMOVE, CPost_Contents::OnPostListMouseMove)
ON_FRESH_VV("post_slot", FRCMD_DBLCLICK, CPost_Contents::OnPostListDClick)
END_FRESH_MSGMAP()

CPost_Contents::CPost_Contents()
{
	tagWTITLEFONT fontInfo;
	memset(&fontInfo, 0, sizeof(fontInfo));
	static char* s_fontFile[] = { "[font_wind.jpg" };
	fontInfo.filename = s_fontFile;
	fontInfo.fontw = 16;
	fontInfo.fonth = 16;
	fontInfo.numPages = 1;
	fontInfo.texw = 128;
	fontInfo.texh = 64;
	fontInfo.pCharSet = "1234567890ym%-.?/:,";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&fontInfo);
}

CPost_Contents::~CPost_Contents()
{
	if (m_pTitleFont)
		m_pTitleFont->Erase();
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

void CPost_Contents::OnPostMessage_ReciveUserInit(int param)
{
	m_pReciveUser = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_Contents::OnPostMessage_SendUserInit(int param)
{
	m_pSendUser = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_Contents::OnPostMessage_LetterContentsInit(int param)
{
	m_pLetterContents = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_Contents::OnPostMessage_LetterDateInit(int param)
{
	m_pLetterDate = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pLetterDate)
		m_pLetterDate->AddText(K2L_Compatibility("2008-02-01 16:10"), false,
			true);
}

void CPost_Contents::OnPostListInit(int param)
{
	m_pPostList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CPost_Contents::OnPostListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;

	FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
	gfx->SetTextStyle(0);
	if (!item->pData)
		return;

	K2L_Compatibility("\xc6\xed\xc1\xf6");
	float iconY = 0.0f;
	float iconX = 0.0f;
	const char* iconName = "item0_13";
	sMailIncludeItem* mail = (sMailIncludeItem*)item->pData;
	if (!mail)
		return;
	unsigned long tid = mail->dwTID;
	int count;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(tid);
	if (common)
	{
		count = mail->iCount;
		iconName = common->Icon;
	}
	else
	{
		count = 0;
	}
	const Bitmap* icon = g_pFresh->GetBitmap(iconName);

	if (icon)
	{
		switch (tid >> 26)
		{
		case 1:
		case 2:
		case 8:
		case 9:
		case 0xf:
		case 0x10:
		case 0x1c:
			iconX = 7.0f;
			iconY = 2.0f;
			break;
		case 0xb:
			iconX = 9.0f;
			iconY = 12.0f;
			break;
		case 0xe:
			if ((tid & 0x3c00000) == 0x800000)
			{
				iconX = 7.0f;
				iconY = 0.0f;
				break;
			}
		default:
			iconX = 16.0f;
			iconY = 27.0f;
			break;
		case 0x1f:
			iconX = 16.0f;
			iconY = 12.0f;
			break;
		}
	}

	unsigned long group = tid >> 26;
	if (group == 2)
	{
		IFF_STRUCT::sChar* chr =
			ItemManager()->FindChar(((tid >> 18) & 0xff) | 0x4000000);
		if (chr)
		{
			const Bitmap* face =
				g_pFresh->GetBitmap(MakeStr("s_%s", chr->c.Icon));
			if (face)
			{
				WRect rect(item->pos.x + 3.0f, item->pos.y + 3.0f,
					(float)face->Width(), (float)face->Height());
				gfx->DrawTexture(face, rect, 0xffffffff, 0);
			}
		}
	}
	if (IsLocalContent(S4_UCC) && tid != 0 && IsUccClothes(tid))
		icon = GetUccItemIcon(mail->dwTID, mail->csUCCIndex);
	if (icon && gfx)
	{
		WRect rect(item->pos.x + iconX + 0.0f, item->pos.y + iconY + 0.0f,
			(float)icon->Width(), (float)icon->Height());
		gfx->DrawTexture(icon, rect, 0xffffffff, 0);
	}

	if (PostItemIsTimed(mail->btItemType) == true)
	{
		gfx->SetTextStyle(2);
		gfx->SetTextColor(0xff000000, 0xffffffff);
		WPoint zero(0.0f, 0.0f);
		IFF_ITEM_COMMON* timed;
		const char* text = NULL;
		if (group == 0x10 &&
			(timed = ItemManager()->FindCommonItem(tid)) != NULL &&
			timed->TimeFlag == 0)
		{
			text = "\xb9\xab\xc1\xa6\xc7\xd1";
		}
		else
		{
			int secs = PostTimeToSeconds(mail->btItemType, mail->iTimeCount);
			int hours = (int)ceil((double)secs / 3600.0);
			int days = hours / 24;
			int rem = hours % 24;
			if (days == 0 && rem == 0)
			{
				text = "1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n";
			}
			else if (days != 0 && rem == 0)
			{
				WPoint pt(item->pos.x + 72.0f, item->pos.y + 72.0f);
				gfx->Print(pt + zero, 2, K2L_Compatibility("%d\xc0\xcf"), days);
			}
			else
			{
				WPoint pt(item->pos.x + 72.0f, item->pos.y + 72.0f);
				gfx->Print(pt + zero, 2,
					K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), days, rem);
			}
		}
		if (text)
		{
			WPoint pt(item->pos.x + 72.0f, item->pos.y + 72.0f);
			gfx->Print(pt + zero, 2, K2L_Compatibility(text));
		}
	}
	else if (count > 1)
	{
		const char* digits = MakeStr("%d", count);
		float x = item->pos.x + 72.0f;
		for (unsigned int i = 0; i < strlen(digits); ++i)
			x -= m_pTitleFont->GetCharWidth(g_view, digits[i] - '0');
		m_pTitleFont->Print(g_view, x, item->pos.y + 60.0f, digits, 0,
			0xffffffff, NULL);
	}
}


void CPost_Contents::OnPostListBtnDown()
{
}

void CPost_Contents::OnPostListRBtnDown()
{
}

void CPost_Contents::OnPostListMouseMove()
{
}

void CPost_Contents::OnPostListDClick()
{
}

void CPost_Contents::SetData(sMailInfo* mail)
{
	if (m_pSendUser)
	{
		m_pSendUser->ClearLine();
		if (strlen(mail->szFromNick) != 0)
			m_pSendUser->AddText(MakeStr(K2L_Compatibility("%s"), mail->szFromNick), false, true);
	}
	if (m_pReciveUser)
	{
		m_pReciveUser->ClearLine();
		if (strlen(Doc()->m_myInfo.info.sNick) != 0)
			m_pReciveUser->AddText(MakeStr(K2L_Compatibility("%s"), Doc()->m_myInfo.info.sNick),
				false, true);
	}
	if (m_pLetterContents)
	{
		m_pLetterContents->ClearLine();
		if (strlen(mail->szContent) != 0)
		{
			m_pLetterContents->AddText(MakeStr(K2L_Compatibility("%s"), mail->szContent), false,
				true);
			// 983 (005b5ef3): volta o texto da carta para o topo
			m_pLetterContents->GetScrollBar()->ScrollToFirst();
		}
	}
	if (m_pLetterDate)
	{
		m_pLetterDate->ClearLine();
		if (strlen(mail->szDate) != 0)
			m_pLetterDate->AddText(MakeStr(K2L_Compatibility("%s"), mail->szDate), false, true);
	}
	SetData(&mail->m_lMailItemList);
}

void CPost_Contents::SetData(std::list<sMailIncludeItem>* items)
{
	m_pPostList->ClearItem();
	if (items->size() != 0)
	{
		for (std::list<sMailIncludeItem>::iterator it = items->begin();
			it != items->end(); ++it)
		{
			if (IsLocalContent(S4_CARD_SYSTEM) &&
				(it->dwTID & 0xfc000000) == 0x8000000)
			{
				IFF_STRUCT::sPart* part = ItemManager()->FindPart(it->dwTID);
				if (!part)
				{
					it->CharacterSlotNum = 0;
					it->CaddieSlotNum = 0;
					it->NpcSlotNum983 = 0;
				}
				else
				{
					it->CharacterSlotNum = part->CharacterSlot;
					it->CaddieSlotNum = part->CaddieSlot;
					it->NpcSlotNum983 = part->NpcSlot983; // 983 005b5a5f: +0x1f <- sPart+0x22c
				}
			}
			m_pPostList->AddItem(&*it);
		}
	}
}
