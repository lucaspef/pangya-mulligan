// JP 983: unidade 00486700–0048f0a0 (alquimia de cartas: FrCardComposeDlg, FrCardComposeCheckDlg, FrCardComposeAboutDlg e
// FrCardComposeResultDlg, nessa ordem no fonte pelo inicializador estático). Classes do 839/983, sem par no 645; ver
// include/jp/cardcomposedlg.h. Sessão B92 (F3.7.10, 1º lote): Check, About e Result; a FrCardComposeDlg (janela principal,
// 0x570) e os moldes do começo da unidade (00486700–00488d60) ficam para a sessão seguinte. O compilador gera: os ajustes
// 0048c590/0048c5a0, 00489b10/00489b20, 0048ec90/0048eca0, o construtor e o destrutor implícitos da CPvsFxBox (0048e9b0 /
// 0048eae0) e os CreateForm<…> (0048d0a0, 0048b970, 0048ee50; chamados pela FrCardComposeDlg).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frtextbutton.h"
#include "frviewer.h"
#include "commonutil.h"
#include "wresrcmng.h"
#include "wview.h"
#include "fxbox.h"
#include "fx.h"
#include "frlistbox.h"
#include "frscrollbar.h"
#include "wfont.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "binstr.h"
#include "../../shared/itemmanager.h"
#include "cardcomposedlg.h"
#include "avatar_avatar.h"

extern Fresh* g_pFresh; // 00f02340
extern const _WRECT g_rectUnit983_e4296c; // 00e4296c ({0, 0, 1, 1})
extern const WVector g_vZero983_f041c4; // 00f041c4

// CFxBox::Finished (0096d7f0) só é visível na fxbox.cpp: visão com o nome no jp.tsv
class CFxBoxJp983 : public CFxBox
{
public:
	bool Finished();
};

// ---------------------------------------------------------------------------------------------------------------------
// Regras, cache de figuras e FrCardComposeDlg (sessão B93)

static char* s_bongFont[] = { "[font_bong.jpg" }; // 00e0416c

// Visão do FrWnd para o m_pScrBar (protegido) da lista: o 983 lê +0x90 direto
class FrWndScrJp983 : public FrWnd
{
public:
	FrScrollBar* GetScrBar983() { return m_pScrBar; }
};

// 00488d60 (GetMessageMap); mapa 00e03948 (32 entradas)
BEGIN_FRESH_MSGMAP(FrCardComposeDlg, FrForm)
ON_FRESH_VI("my_card_list", FRCMD_INIT, FrCardComposeDlg::OnListInit983)
ON_FRESH_VI("my_card_list", FRCMD_OWNERDRAW, FrCardComposeDlg::OnListDraw983)
ON_FRESH_VV("my_card_list", FRCMD_LBUTTONDOWN, FrCardComposeDlg::OnListDown983)
ON_FRESH_VI("compose_card", FRCMD_INIT, FrCardComposeDlg::OnComposeInit983)
ON_FRESH_VV("compose_card", FRCMD_LBUTTONDOWN, FrCardComposeDlg::OnComposeDown983)
ON_FRESH_VI("compose_card_btn1", FRCMD_INIT, FrCardComposeDlg::OnSlot1Init983)
ON_FRESH_VV("compose_card_btn1", FRCMD_LBUTTONUP, FrCardComposeDlg::OnSlot1Up983)
ON_FRESH_VV("compose_card_btn1", FRCMD_OWNERDRAW, FrCardComposeDlg::OnSlot1Draw983)
ON_FRESH_VI("compose_card_btn2", FRCMD_INIT, FrCardComposeDlg::OnSlot2Init983)
ON_FRESH_VV("compose_card_btn2", FRCMD_LBUTTONUP, FrCardComposeDlg::OnSlot2Up983)
ON_FRESH_VV("compose_card_btn2", FRCMD_OWNERDRAW, FrCardComposeDlg::OnSlot2Draw983)
ON_FRESH_VI("compose_card_btn3", FRCMD_INIT, FrCardComposeDlg::OnSlot3Init983)
ON_FRESH_VV("compose_card_btn3", FRCMD_LBUTTONUP, FrCardComposeDlg::OnSlot3Up983)
ON_FRESH_VV("compose_card_btn3", FRCMD_OWNERDRAW, FrCardComposeDlg::OnSlot3Draw983)
ON_FRESH_VI("about", FRCMD_INIT, FrCardComposeDlg::OnAboutInit983)
ON_FRESH_VV("about", FRCMD_LBUTTONDOWN, FrCardComposeDlg::OnAboutDown983)
ON_FRESH_VI("whole_card_button", FRCMD_INIT, FrCardComposeDlg::OnWholeInit983)
ON_FRESH_VV("whole_card_button", FRCMD_LBUTTONDOWN, FrCardComposeDlg::OnWholeDown983)
ON_FRESH_VI("normal_card_button", FRCMD_INIT, FrCardComposeDlg::OnNormalInit983)
ON_FRESH_VV("normal_card_button", FRCMD_LBUTTONDOWN, FrCardComposeDlg::OnNormalDown983)
ON_FRESH_VI("rare_card_button", FRCMD_INIT, FrCardComposeDlg::OnRareInit983)
ON_FRESH_VV("rare_card_button", FRCMD_LBUTTONDOWN, FrCardComposeDlg::OnRareDown983)
ON_FRESH_VI("super_rare_card_button", FRCMD_INIT, FrCardComposeDlg::OnSuperRareInit983)
ON_FRESH_VV("super_rare_card_button", FRCMD_LBUTTONDOWN, FrCardComposeDlg::OnSuperRareDown983)
ON_FRESH_VI("print_pang_area", FRCMD_INIT, FrCardComposeDlg::OnPangAreaInit983)
ON_FRESH_VI("print_pang_area", FRCMD_OWNERDRAW, FrCardComposeDlg::OnPangAreaDraw983)
ON_FRESH_VI("MagicBox", FRCMD_INIT, FrCardComposeDlg::OnMagicBoxInit983)
ON_FRESH_VV("MagicBox", FRCMD_LBUTTONUP, FrCardComposeDlg::OnMagicBoxUp983)
ON_FRESH_VI("CardCompose", FRCMD_INIT, FrCardComposeDlg::OnCardComposeInit983)
ON_FRESH_VV("CardCompose", FRCMD_LBUTTONUP, FrCardComposeDlg::OnCardComposeUp983)
ON_FRESH_VI("Exchange", FRCMD_INIT, FrCardComposeDlg::OnExchangeInit983)
ON_FRESH_VV("Exchange", FRCMD_LBUTTONUP, FrCardComposeDlg::OnExchangeUp983)
END_FRESH_MSGMAP()

// 0048e400
FrCardComposeDlg::FrCardComposeDlg()
	: m_unused138983(-1), m_topRow(0)
{
	for (int i = 0; i < 3; ++i)
	{
		m_sel.m_tid[i] = 0;
		m_sel.m_pOverlay[i] = NULL;
	}
	InitFont983();
	m_seq = "";
	m_pMagicBox = NULL;
	m_pCardCompose = NULL;
	m_pExchange = NULL;
}

IMPLEMENT_OBJECT(FrCardComposeDlg, FrForm)

// 0048dd20
FrCardComposeDlg::~FrCardComposeDlg()
{
	m_items.clear();
	if (g_resrcmng && m_sel.m_pRes)
	{
		g_resrcmng->Release(m_sel.m_pRes);
		m_sel.m_pRes = NULL;
	}
	m_cache.Clear983();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

// 00488e20
void FrCardComposeDlg::LoadBitmaps983()
{
	m_pListOverBmp = g_pFresh->GetBitmap("cardmix_btn_list_my_o");
	m_pSlotOverBmp = g_pFresh->GetBitmap("cardmix_btn_slot_my_o");
	m_pSlotDownBmp = g_pFresh->GetBitmap("cardmix_btn_slot_my_d");
}

// 00488e70
void FrCardComposeDlg::SetPriceRect983()
{
	m_iconRc.w = 21.5f;
	m_iconRc.h = 26.5f;
}

// 00489d30
void FrCardComposeDlg::SetListBmpSize983()
{
	if (m_pListOverBmp)
	{
		m_listOverRc.w = (float)m_pListOverBmp->Width();
		m_listOverRc.h = (float)m_pListOverBmp->Height();
	}
}

// 00489d80
void FrCardComposeDlg::SetSlotBmpSize983()
{
	if (m_pSlotOverBmp)
	{
		m_slotRc.w = (float)m_pSlotOverBmp->Width();
		m_slotRc.h = (float)m_pSlotOverBmp->Height();
	}
}

// 0048bb90
void FrCardComposeDlg::InitFont983()
{
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_bongFont;
	info.fontw = 9;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 32;
	info.pCharSet = "1234567890,-";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
	MakePriceStrA(m_pangStr, Doc()->m_myInfo.stat.i64Pang);
	MakePriceStrA(m_priceStr, 0);
}

// 0048e5c0
bool FrCardComposeDlg::OnInit()
{
	LoadBitmaps983();
	SetPriceRect983();
	SetListBmpSize983();
	SetSlotBmpSize983();
	if (FillList983(Doc()->m_cardList, 3))
		m_pList->SortItem(CompareItem983);
	m_pCardCompose->SetStatus((FrButton::eButMode)2);
	// A299: 00e73e80 = ntAvatarManager::m_pInstance (g_pAvatarMotion983 era só declaração = nulo no exe; estado em +0xf0 no nosso)
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pAvatar)
			pAvatar->SetState(pAvatar->m_state983 | 0x20);
		ntAvatarManager::m_pInstance->m_bActive = false;
	}
	return true;
}

// 0048de90: as cartas do inventário entram no mapa (ou atualizam a quantidade); na lista só as do filtro
bool FrCardComposeDlg::FillList983(std::list<sCards>& cards, int filter)
{
	for (std::list<sCards>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
		sCards* pCard = &*it;
		if (!pCard)
			continue;
		IFF_STRUCT::sCard* pIff = ItemManager()->FindCard(pCard->tid);
		if (!pIff)
			continue;
		sCardComposeItem983* pItem = NULL;
		std::map<unsigned long, sCardComposeItem983>::iterator found = m_items.find(pCard->tid);
		if (found == m_items.end())
		{
			std::pair<std::map<unsigned long, sCardComposeItem983>::iterator, bool> ret =
				m_items.insert(std::pair<const unsigned long, sCardComposeItem983>(pCard->tid, sCardComposeItem983(*pCard, *pIff)));
			if (ret.second == true)
				pItem = &ret.first->second;
		}
		else
		{
			pItem = &found->second;
			pItem->card.cardCount = pCard->cardCount;
		}
		if (pCard->tid && pIff->RareType != 3)
		{
			unsigned long type = (pCard->tid >> 22) & 0xf;
			if (type != 3 && type != 4 && (filter == 3 || pIff->RareType == filter))
				m_pList->AddItem(pItem);
		}
	}
	return m_pList->GetCurrentItemSize(true) > 0;
}

// 00488d70
int FrCardComposeDlg::TypeOrder983(const IFF_STRUCT::sCard* p)
{
	unsigned long type = (p->c.TypeId >> 22) & 0xf;
	if (type == 2)
		return 0;
	if (type == 0)
		return 1;
	if (type == 1)
		return 2;
	return type == 5 ? 3 : -1;
}

// 00488db0: raridade, tipo, typeId
bool FrCardComposeDlg::CompareItem983(const void* a, const void* b)
{
	const sCardComposeItem983* p1 = (const sCardComposeItem983*)a;
	const sCardComposeItem983* p2 = (const sCardComposeItem983*)b;
	if (p1->iff.RareType > p2->iff.RareType)
		return false;
	if (p1->iff.RareType < p2->iff.RareType)
		return true;
	int o1 = TypeOrder983(&p1->iff);
	int o2 = TypeOrder983(&p2->iff);
	if (o1 > o2)
		return false;
	if (o1 < o2)
		return true;
	return p1->card.tid <= p2->card.tid;
}

// 00488ea0
int FrCardComposeDlg::AddSlot983(const sCardComposeItem983* p)
{
	if (p->card.cardCount < 1)
		return -1;
	int slot = -1;
	int same = 0;
	for (int i = 2; i >= 0; --i)
	{
		if (m_sel.m_tid[i] == p->card.tid)
			++same;
		if (m_sel.m_tid[i] == 0)
			slot = i;
	}
	if (slot != -1 && same < p->card.cardCount)
	{
		m_sel.m_tid[slot] = p->card.tid;
		++m_sel.m_count;
		return slot;
	}
	return -1;
}

// 00488f30
bool FrCardComposeDlg::ClearSlot983(int slot)
{
	if (m_sel.m_tid[slot] == 0)
		return false;
	m_sel.m_tid[slot] = 0;
	--m_sel.m_count;
	return true;
}

// 00488f60
int FrCardComposeDlg::GetLeft983(const sCardComposeItem983* p)
{
	int n = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (m_sel.m_tid[i] == p->card.tid)
			++n;
	}
	return p->card.cardCount - n;
}

// 00488fa0
bool FrCardComposeDlg::SetNewCard983(const unsigned long& tid)
{
	if (tid == 0)
		return false;
	m_sel.m_newTid = tid;
	return true;
}

// 00488fc0
void FrCardComposeDlg::RemoveSlot983(int slot)
{
	if (ClearSlot983(slot))
	{
		if (!m_rule.IsFull(m_sel.m_count))
			m_pCompose->Enable(false);
		MakePriceStrA(m_priceStr, m_rule.CalcPrice(m_sel.m_tid));
		m_sel.m_pOverlay[slot] = NULL;
		m_pSlot[slot]->SetVisible(false);
	}
}

// 00489040: resposta da confirmação
bool FrCardComposeDlg::OnCheckResult983(int result, FrForm* pForm)
{
	if (result == FrYES)
	{
		__int64 price = m_rule.CalcPrice(m_sel.m_tid);
		if (m_rule.CanCompose(m_sel.m_count, price, m_sel.m_tid))
		{
			m_rule.Send983(price, m_sel.m_tid);
			m_pCompose->Enable(false);
		}
	}
	m_pCheckDlg = NULL;
	return true;
}

// 00489dd0
void FrCardComposeDlg::OnListInit983(int param)
{
	m_pList = KINDOF_CAST983(FrListBox, param);
	m_pList->UseDblClick(false);
}

// 00489e50
void FrCardComposeDlg::OnPangAreaInit983(int param)
{
	m_pPangArea = KINDOF_CAST983(FrArea, param);
}

// 00489ec0
void FrCardComposeDlg::OnMagicBoxInit983(int param)
{
	m_pMagicBox = KINDOF_CAST983(FrButton, param);
	if (m_pMagicBox)
		m_pMagicBox->SetVisible(true);
	if (m_pMagicBox)
		m_pMagicBox->SetPushDelay(0.01f);
	if (m_pMagicBox)
		m_pMagicBox->EnableHover(true, 0.0f);
}

// 00489f70
void FrCardComposeDlg::OnCardComposeInit983(int param)
{
	m_pCardCompose = KINDOF_CAST983(FrButton, param);
	if (m_pCardCompose)
		m_pCardCompose->SetVisible(true);
	if (m_pCardCompose)
		m_pCardCompose->SetPushDelay(0.01f);
	if (m_pCardCompose)
		m_pCardCompose->EnableHover(true, 0.0f);
}

// 0048a020
void FrCardComposeDlg::OnExchangeInit983(int param)
{
	m_pExchange = KINDOF_CAST983(FrButton, param);
	if (m_pExchange)
		m_pExchange->SetVisible(true);
	if (m_pExchange)
		m_pExchange->SetPushDelay(0.01f);
	if (m_pExchange)
		m_pExchange->EnableHover(true, 0.0f);
}

// 0048a0d0: pang do jogador, alinhado à direita
void FrCardComposeDlg::PrintPang983(FrGraphicInterface* pGI)
{
	m_pFont->Print(g_view, m_pPangArea->GetRect().x + 155.0 - strlen(m_pangStr) * (m_pFont->GetFontInfo()->fontw - 1.0),
		m_pPangArea->GetRect().y + 5.0, m_pangStr, 0, 0xffffffff, NULL);
}

// 0048a160: preço da alquimia
void FrCardComposeDlg::PrintPrice983(FrGraphicInterface* pGI)
{
	m_pFont->Print(g_view, m_pPangArea->GetRect().x + 155.0 - strlen(m_priceStr) * (m_pFont->GetFontInfo()->fontw - 1.0),
		m_pPangArea->GetRect().y + 31.0, m_priceStr, 0, 0xffffffff, NULL);
}

// 0048a1f0
void FrCardComposeDlg::OnComposeInit983(int param)
{
	m_pCompose = KINDOF_CAST983(FrButton, param);
	m_pCompose->Enable(false);
}

// 0048a260
void FrCardComposeDlg::DrawItemIcon983(FrListItem* pItem, FrGraphicInterface* pGI, const sCardComposeItem983* p)
{
	const Bitmap* pBmp = g_pFresh->GetBitmap(p->iff.c.Icon);
	if (pBmp)
	{
		m_iconRc.x = m_pList->GetRect().x + 10.0;
		m_iconRc.y = pItem->pos.y + 2.5;
		pGI->DrawTexture(pBmp, m_iconRc, 0xffffffff, 0);
	}
}

// 0048a2c0
void FrCardComposeDlg::DrawItemName983(FrListItem* pItem, const sCardComposeItem983* p)
{
	WPoint pos;
	pos.x = pItem->pos.x + 35.0;
	pos.y = pItem->pos.y + 10.0;
	g_pFresh->GetManager()->PrintText(pos, 0, p->iff.c.Name, 102.0f, 0xffffffff);
}

// 0048a310
void FrCardComposeDlg::DrawItemLeft983(FrListItem* pItem, int left)
{
	WPoint pos;
	pos.x = pItem->pos.x + 161.0;
	pos.y = pItem->pos.y + 10.0;
	g_pFresh->GetManager()->PrintText(pos, 2, MakeStr("x%d", left), -1.0f, 0xffffffff);
}

// 0048a370
void FrCardComposeDlg::OnSlot1Init983(int param)
{
	m_pSlot[0] = KINDOF_CAST983(FrButton, param);
	m_pSlot[0]->SetVisible(false);
}

// 0048a3f0
void FrCardComposeDlg::OnSlot2Init983(int param)
{
	m_pSlot[1] = KINDOF_CAST983(FrButton, param);
	m_pSlot[1]->SetVisible(false);
}

// 0048a470
void FrCardComposeDlg::OnSlot3Init983(int param)
{
	m_pSlot[2] = KINDOF_CAST983(FrButton, param);
	m_pSlot[2]->SetVisible(false);
}

// 0048a4f0: figura da carta na casa
void FrCardComposeDlg::DrawSlotOverlay983(int slot)
{
	FrButton* pBtn = m_pSlot[slot];
	if (pBtn)
	{
		WRect rc;
		rc.x = pBtn->GetRect().x;
		rc.y = pBtn->GetRect().y;
		rc.x += 1.5;
		rc.y += 1.5;
		rc.w = 173.0f;
		rc.h = 352.0f;
		m_sel.m_pOverlay[slot]->Render(g_view, g_rectUnit983_e4296c, rc, 0, 0xffffffff, 0.0f, 0);
	}
}

// 0048a580: moldura da casa sob o cursor (1) ou apertada (2)
void FrCardComposeDlg::DrawSlotFrame983(int slot, FrButton* pBtn, FrGraphicInterface* pGI)
{
	m_slotRc.x = m_pSlot[slot]->GetRect().x;
	m_slotRc.y = m_pSlot[slot]->GetRect().y;
	if (pBtn->GetStatus() == 1)
		pGI->DrawTexture(m_pSlotOverBmp, m_slotRc, 0xffffffff, 0);
	else if (pBtn->GetStatus() == 2)
		pGI->DrawTexture(m_pSlotDownBmp, m_slotRc, 0xffffffff, 0);
}

// 0048a5f0
void FrCardComposeDlg::DrawSlot983(int slot)
{
	FrButton* pBtn = m_pSlot[slot];
	FrGraphicInterface* pGI;
	if (pBtn && (pGI = g_pFresh->GetManager()->GetGDI()) != NULL && m_sel.m_pOverlay[slot])
	{
		DrawSlotOverlay983(slot);
		if (pBtn->GetStatus() != 0)
			DrawSlotFrame983(slot, pBtn, pGI);
	}
}

// 0048a650
void FrCardComposeDlg::OnAboutInit983(int param)
{
	m_pAbout = KINDOF_CAST983(FrButton, param);
}

// 0048a6c0
void FrCardComposeDlg::OnWholeInit983(int param)
{
	m_pFilter[3] = KINDOF_CAST983(FrButton, param);
	m_pFilter[3]->SetStatus((FrButton::eButMode)2);
}

// 0048a740
void FrCardComposeDlg::OnNormalInit983(int param)
{
	m_pFilter[0] = KINDOF_CAST983(FrButton, param);
}

// 0048a7b0
void FrCardComposeDlg::OnRareInit983(int param)
{
	m_pFilter[1] = KINDOF_CAST983(FrButton, param);
}

// 0048a820
void FrCardComposeDlg::OnSuperRareInit983(int param)
{
	m_pFilter[2] = KINDOF_CAST983(FrButton, param);
}

// 0048bc40
void FrCardComposeDlg::OnPangAreaDraw983(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI && m_pPangArea)
	{
		PrintPang983(pGI);
		PrintPrice983(pGI);
	}
}

// 0048bc80: "MagicBox" (mensagem 0x338 ao CTaskMain)
void FrCardComposeDlg::OnMagicBoxUp983()
{
	OnFreshCancel();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x338, 0, 0, 0, 0, 0));
}

// 0048bce0: "Exchange" (mensagem 0x2e3)
void FrCardComposeDlg::OnExchangeUp983()
{
	OnFreshCancel();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2e3, 0, 0, 0, 0, 0));
}

// 0048bd40
void FrCardComposeDlg::OnListDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	FrGraphicInterface* pGI;
	const sCardComposeItem983* p;
	if (pItem && (pGI = g_pFresh->GetManager()->GetGDI()) != NULL && (p = (const sCardComposeItem983*)pItem->pData) != NULL)
	{
		if (pItem->underCursor && m_pListOverBmp)
		{
			m_listOverRc.x = m_pList->GetRect().x + 2.0;
			m_listOverRc.y = pItem->pos.y + 1.0;
			pGI->DrawTexture(m_pListOverBmp, m_listOverRc, 0xffffffff, 0);
		}
		DrawItemIcon983(pItem, pGI, p);
		DrawItemName983(pItem, p);
		DrawItemLeft983(pItem, GetLeft983(p));
	}
}

// 0048be10
void FrCardComposeDlg::OnAboutDown983()
{
	FrCardComposeAboutDlg* pDlg = CreateForm<FrCardComposeAboutDlg>(g_pFresh->GetManager(), this, "card_compose_about", NULL);
	if (pDlg)
		pDlg->Open(NULL, 3);
}

// 0048d6f0
void FrCardComposeDlg::SetSlotImage983(const sCardComposeItem983* p, int slot)
{
	std::string name(p->iff.Image);
	name += ".png";
	m_sel.m_pOverlay[slot] = m_cache.Get983(name.c_str());
}

// 0048d7c0
void FrCardComposeDlg::SetSeq983(const int& rarity)
{
	if (rarity == -1)
		return;
	switch (rarity)
	{
	case 0:
		m_seq = "card_compose_N.seq";
		break;
	case 1:
		m_seq = "card_compose_R.seq";
		break;
	case 2:
		m_seq = "card_compose_SR.seq";
		break;
	case 3:
		m_seq = "card_compose_SC.seq";
		break;
	}
}

// 0048da90: carta da lista para a primeira casa livre
void FrCardComposeDlg::OnListDown983()
{
	FrListItem* pItem = m_pList->GetItemUnderCursor();
	if (!pItem)
		return;
	const sCardComposeItem983* p = (const sCardComposeItem983*)pItem->pData;
	if (!p)
		return;
	if (m_rule.IsFull(m_sel.m_count))
	{
		FrForm* pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		if (pDlg)
		{
			// "Não há casa livre. Aperte o botão de juntar as cartas."
			pDlg->SetMessage(K2L_Compatibility("\272\363 \275\275\267\324\300\314 \276\370\275\300\264\317\264\331. \304\253\265\345 "
				"\307\325\274\272 \271\366\306\260\300\273 \264\255\267\257\301\326\274\274\277\344."), false);
			pDlg->Open(NULL, 3);
		}
		return;
	}
	int slot = AddSlot983(p);
	if (slot != -1)
	{
		SetSlotImage983(p, slot);
		m_pSlot[slot]->SetVisible(true);
		MakePriceStrA(m_priceStr, m_rule.CalcPrice(m_sel.m_tid));
		if (m_rule.IsFull(m_sel.m_count))
			m_pCompose->Enable(true);
	}
}

// 0048e000: confirmação com as 3 figuras
void FrCardComposeDlg::OpenCheck983()
{
	m_pCheckDlg = CreateForm<FrCardComposeCheckDlg>(g_pFresh->GetManager(), this, "card_compose_check", NULL);
	if (m_pCheckDlg)
	{
		m_pCheckDlg->Open((FRESH_PFN_RESULT)&FrCardComposeDlg::OnCheckResult983, 1);
		std::string names[3];
		for (int i = 0; i < 3; ++i)
		{
			IFF_STRUCT::sCard* pIff = ItemManager()->FindCard(m_sel.m_tid[i]);
			if (!pIff)
				return;
			names[i] = pIff->Image;
		}
		m_pCheckDlg->SetImages983(names);
		m_pCheckDlg->OpenImages983();
	}
}

// 0048e160: depois do resultado: casas vazias, pang e lista refeitos, na mesma linha
void FrCardComposeDlg::Reset983()
{
	for (int i = 0; i < 3; ++i)
	{
		m_sel.m_tid[i] = 0;
		m_pSlot[i]->SetVisible(false);
	}
	MakePriceStrA(m_pangStr, Doc()->m_myInfo.stat.i64Pang);
	MakePriceStrA(m_priceStr, 0);
	m_pList->ClearItem();
	if (FillList983(Doc()->m_cardList, m_sel.m_filter))
		m_pList->SortItem(CompareItem983);
	m_sel.m_count = 0;
	m_pList->ResetTopRow();
	((FrWndScrJp983*)m_pList)->GetScrBar983()->SetCurTopRow(m_topRow);
}

// 0048e230
bool FrCardComposeDlg::OnResultClose983(int result, FrForm* pForm)
{
	if (result == 1 || result == 2)
		Reset983();
	m_pResultDlg = NULL;
	return true;
}

// 0048e260
void FrCardComposeDlg::SetFilter983(int filter)
{
	if (m_sel.m_filter != filter)
	{
		m_pFilter[m_sel.m_filter]->SetStatus((FrButton::eButMode)0);
		m_sel.m_filter = filter;
		m_pList->ClearItem();
		if (FillList983(Doc()->m_cardList, filter))
			m_pList->SortItem(CompareItem983);
	}
}

// 0048e650: juntar: pang suficiente → confirmação
void FrCardComposeDlg::OnComposeDown983()
{
	if (!m_rule.IsFull(m_sel.m_count))
		return;
	if (Doc()->m_myInfo.stat.i64Pang < m_rule.CalcPrice(m_sel.m_tid))
	{
		FrForm* pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		if (pDlg)
		{
			// "O pang que você tem não basta."
			pDlg->SetMessage(K2L_Compatibility("\272\270\300\257\307\317\260\355 \300\326\264\302 \306\316\300\314 \272\316\301\267\307"
				"\325\264\317\264\331"), false);
			pDlg->Open(NULL, 3);
		}
		return;
	}
	float row = ((FrWndScrJp983*)m_pList)->GetScrBar983()->GetCurTopRow() + 0.5;
	m_topRow = (int)row;
	OpenCheck983();
}

// 0048ef20: resultado (0x326 do CTaskMain)
void FrCardComposeDlg::OpenResult983()
{
	m_pResultDlg = CreateForm<FrCardComposeResultDlg>(g_pFresh->GetManager(), this, "result_card_compose", NULL);
	if (m_pResultDlg)
	{
		m_pResultDlg->SetCard983(m_sel.m_newTid);
		m_pResultDlg->PlayFx983(m_seq.c_str());
		m_pResultDlg->SetOwner983(this);
		m_pResultDlg->ShowCard983();
		m_pResultDlg->Open((FRESH_PFN_RESULT)&FrCardComposeDlg::OnResultClose983, 1);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// FrCardComposeCheckDlg

// 004890d0 (GetMessageMap); mapa 00e03950 (5 entradas)
BEGIN_FRESH_MSGMAP(FrCardComposeCheckDlg, FrForm)
ON_FRESH_VI("card_compose_viewer1", FRCMD_INIT, FrCardComposeCheckDlg::OnViewer1Init983)
ON_FRESH_VI("card_compose_viewer2", FRCMD_INIT, FrCardComposeCheckDlg::OnViewer2Init983)
ON_FRESH_VI("card_compose_viewer3", FRCMD_INIT, FrCardComposeCheckDlg::OnViewer3Init983)
ON_FRESH_VV("accept_compose", FRCMD_LBUTTONUP, FrCardComposeCheckDlg::OnAcceptUp983)
ON_FRESH_VV("cancel_compose", FRCMD_LBUTTONUP, FrCardComposeCheckDlg::OnCancelUp983)
END_FRESH_MSGMAP()

// 0048c500
FrCardComposeCheckDlg::FrCardComposeCheckDlg()
{
}

IMPLEMENT_OBJECT(FrCardComposeCheckDlg, FrForm)

// 0048c5b0
FrCardComposeCheckDlg::~FrCardComposeCheckDlg()
{
}

// 0048a890
void FrCardComposeCheckDlg::OnViewer1Init983(int param)
{
	m_pViewer[0] = KINDOF_CAST983(FrViewer, param);
}

// 0048a900
void FrCardComposeCheckDlg::OnViewer2Init983(int param)
{
	m_pViewer[1] = KINDOF_CAST983(FrViewer, param);
}

// 0048a970
void FrCardComposeCheckDlg::OnViewer3Init983(int param)
{
	m_pViewer[2] = KINDOF_CAST983(FrViewer, param);
}

// 0048dbb0
void FrCardComposeCheckDlg::SetImages983(const std::string* names)
{
	for (int i = 0; i < 3; i++)
	{
		m_image[i] = names[i];
		m_image[i] += ".png";
	}
}

// 0048be60
void FrCardComposeCheckDlg::OpenImages983()
{
	for (int i = 0; i < 3; i++)
	{
		if (!m_pViewer[i])
			return;
		m_pViewer[i]->Open(m_image[i].c_str());
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// FrCardComposeAboutDlg

// 00489100 (GetMessageMap); mapa 00e03958 (17 entradas)
BEGIN_FRESH_MSGMAP(FrCardComposeAboutDlg, FrForm)
ON_FRESH_VI("about_viewer", FRCMD_INIT, FrCardComposeAboutDlg::OnViewerInit983)
ON_FRESH_VI("index01", FRCMD_INIT, FrCardComposeAboutDlg::OnIndex01Init983)
ON_FRESH_VV("index01", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnIndex01Up983)
ON_FRESH_VI("index02", FRCMD_INIT, FrCardComposeAboutDlg::OnIndex02Init983)
ON_FRESH_VV("index02", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnIndex02Up983)
ON_FRESH_VI("index03", FRCMD_INIT, FrCardComposeAboutDlg::OnIndex03Init983)
ON_FRESH_VV("index03", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnIndex03Up983)
ON_FRESH_VI("index04", FRCMD_INIT, FrCardComposeAboutDlg::OnIndex04Init983)
ON_FRESH_VV("index04", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnIndex04Up983)
ON_FRESH_VI("index05", FRCMD_INIT, FrCardComposeAboutDlg::OnIndex05Init983)
ON_FRESH_VV("index05", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnIndex05Up983)
ON_FRESH_VI("index06", FRCMD_INIT, FrCardComposeAboutDlg::OnIndex06Init983)
ON_FRESH_VV("index06", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnIndex06Up983)
ON_FRESH_VI("backward", FRCMD_INIT, FrCardComposeAboutDlg::OnBackwardInit983)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnBackwardUp983)
ON_FRESH_VI("forward", FRCMD_INIT, FrCardComposeAboutDlg::OnForwardInit983)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrCardComposeAboutDlg::OnForwardUp983)
END_FRESH_MSGMAP()

// 00489110
FrCardComposeAboutDlg::FrCardComposeAboutDlg()
	: m_page(0), m_pBackward(NULL), m_pForward(NULL)
{
	for (int i = 0; i < 6; i++)
		m_pIndex[i] = NULL;
}

IMPLEMENT_OBJECT(FrCardComposeAboutDlg, FrForm)

// 004891a0
void FrCardComposeAboutDlg::SetPage983(int page)
{
	if ((unsigned int)page > 5)
		return;

	switch (page)
	{
	case 0:
		m_pBackward->Enable(false);
		m_pForward->Enable(true);
		break;
	case 5:
		m_pForward->Enable(false);
		m_pBackward->Enable(true);
		break;
	default:
		m_pBackward->Enable(true);
		m_pForward->Enable(true);
		break;
	}

	for (int i = 0; i < 6; i++)
		m_pIndex[i]->Unselect(0);
	m_pIndex[page]->Select(0);

	if (m_pViewer)
		m_pViewer->Open(MakeStr("help_cardmix_%02d.jpg", m_page + 1));
}

// 0048aaa0
void FrCardComposeAboutDlg::OnViewerInit983(int param)
{
	m_pViewer = KINDOF_CAST983(FrViewer, param);
}

// 0048ab10
void FrCardComposeAboutDlg::OnIndex01Init983(int param)
{
	m_pIndex[0] = KINDOF_CAST983(FrTextButton, param);
}

// 0048aba0
void FrCardComposeAboutDlg::OnIndex02Init983(int param)
{
	m_pIndex[1] = KINDOF_CAST983(FrTextButton, param);
}

// 0048ac30
void FrCardComposeAboutDlg::OnIndex03Init983(int param)
{
	m_pIndex[2] = KINDOF_CAST983(FrTextButton, param);
}

// 0048acc0
void FrCardComposeAboutDlg::OnIndex04Init983(int param)
{
	m_pIndex[3] = KINDOF_CAST983(FrTextButton, param);
}

// 0048ad50
void FrCardComposeAboutDlg::OnIndex05Init983(int param)
{
	m_pIndex[4] = KINDOF_CAST983(FrTextButton, param);
}

// 0048ade0
void FrCardComposeAboutDlg::OnIndex06Init983(int param)
{
	m_pIndex[5] = KINDOF_CAST983(FrTextButton, param);
}

// 0048ae70
void FrCardComposeAboutDlg::OnBackwardInit983(int param)
{
	m_pBackward = KINDOF_CAST983(FrButton, param);
}

// 0048af00
void FrCardComposeAboutDlg::OnForwardInit983(int param)
{
	m_pForward = KINDOF_CAST983(FrButton, param);
}

// 0048af90
bool FrCardComposeAboutDlg::OnInit()
{
	SetPage983(0);
	if (ntAvatarManager::m_pInstance) // A299: 00e73e80
		ntAvatarManager::m_pInstance->m_bActive = false;
	return FrForm::OnInit();
}

// ---------------------------------------------------------------------------------------------------------------------
// FrCardComposeResultDlg

// 00489250 (GetMessageMap); mapa 00e03960 (5 entradas)
BEGIN_FRESH_MSGMAP(FrCardComposeResultDlg, FrForm)
ON_FRESH_VI("synthesized_card", FRCMD_INIT, FrCardComposeResultDlg::OnCardInit983)
ON_FRESH_VI("fx_player", FRCMD_INIT, FrCardComposeResultDlg::OnFxPlayerInit983)
ON_FRESH_VV("fx_player", FRCMD_OWNERDRAW, FrCardComposeResultDlg::OnFxPlayerDraw983)
ON_FRESH_VI("ok", FRCMD_INIT, FrCardComposeResultDlg::OnOkInit983)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrCardComposeResultDlg::OnOkUp983)
END_FRESH_MSGMAP()

// 0048ecb0
FrCardComposeResultDlg::FrCardComposeResultDlg()
	: m_camera(WMatrix::IDENTITY), m_rc(g_rectUnit983_e4296c), m_timer(-1.0f)
{
	m_fxBox.Init(g_resrcmng);
	m_fxBox.SetCamera(&g_camera);
	m_fxBox.m_frameTime = 33.333332f;
	float scale = g_view->SCREEN_YS / 600.0;
	float dist = scale * -30.0;
	m_camera.pivot = m_camera.za * dist;
}

IMPLEMENT_OBJECT(FrCardComposeResultDlg, FrForm)

// 0048ebc0
FrCardComposeResultDlg::~FrCardComposeResultDlg()
{
	m_fxBox.ClearAll(false);
}

// 0048afc0
void FrCardComposeResultDlg::OnCardInit983(int param)
{
	m_pCard = KINDOF_CAST983(FrViewer, param);
	m_pCard->SetVisible(false);
}

// 0048b030
void FrCardComposeResultDlg::OnFxPlayerInit983(int param)
{
	m_rc = KINDOF_CAST983(FrArea, param)->GetRect();
	m_fxBox.SetScreenCenter(m_rc.x, m_rc.y);
}

// 0048b0e0
void FrCardComposeResultDlg::OnProc(const float deltaTime)
{
	FrForm::OnProc(deltaTime);
	if (((CFxBoxJp983*)&m_fxBox)->Finished())
		return;

	float ms = deltaTime * 1000.0;
	m_fxBox.ProcessAttacher(ms);
	m_fxBox.Process(ms);

	if (m_timer > 0.0f)
		m_timer += deltaTime;
	if (!m_pCard->IsVisible() && m_timer > 2.0f)
		m_pCard->SetVisible(true);
	if (m_timer > 4.0f)
	{
		m_pOk->Enable(true);
		m_timer = -1.0f;
	}
}

// 0048b1c0
void FrCardComposeResultDlg::OnFxPlayerDraw983()
{
	if (!((CFxBoxJp983*)&m_fxBox)->Finished())
	{
		bool bFog = g_view->SetFogEnable(false);
		g_view->SetCamera(m_camera);
		float oldNear = g_view->clip_near;
		WView::PROJECTION_MODE mode = g_view->GetProjectionMode();
		float oldFar = g_view->clip_far;
		float oldFov = g_view->GetFOV();
		g_view->SetProjectionMode((WView::PROJECTION_MODE)0);
		g_view->SetFOV(1.5707964f);
		g_view->SetClip(0.5f, 50.0f, false);
		g_view->SetClippingArea(m_rc);
		g_view->UpdateCamera();
		m_fxBox.Display(g_view);
		g_view->Flush(1, 1);
		g_view->SetProjectionMode(mode);
		g_view->SetClip(oldNear, oldFar, false);
		g_view->SetFOV(oldFov);
		WRect rc;
		rc.x = 0.0f;
		rc.y = 0.0f;
		rc.w = g_view->SCREEN_XS;
		rc.h = g_view->SCREEN_YS;
		g_view->SetClippingArea(rc);
		g_view->SetScreenCenter(g_view->SCREEN_XS * 0.5, g_view->SCREEN_YS * 0.5);
		g_view->UpdateCamera();
		g_view->SetFogEnable(bFog);
	}
}

// 0048b380
void FrCardComposeResultDlg::PlayFx983(const char* seqName)
{
	CFxSequence* pSeq = m_fxBox.LoadSequence(seqName, true, NULL);
	if (pSeq && m_pCard)
	{
		const WRect& rc = m_pCard->GetRect();
		pSeq->m_attachX = (float)(int)(float)(rc.x + rc.w * 0.5);
		pSeq->m_attachY = (float)(int)(float)(rc.y + rc.h * 0.5);
		pSeq->m_pos = g_vZero983_f041c4;
		m_fxBox.AddSequence(pSeq);
	}
}

// 00489260
void FrCardComposeResultDlg::SetOwner983(FrCardComposeDlg* pOwner)
{
	m_pOwner = pOwner;
}

// 0048b420
void FrCardComposeResultDlg::OnOkInit983(int param)
{
	m_pOk = KINDOF_CAST983(FrButton, param);
	m_pOk->Enable(false);
}

// 0048bea0
void FrCardComposeResultDlg::ShowCard983()
{
	if (m_pCard && m_pCard->Open(m_image.c_str()))
		m_timer = 1.0f;
}

// 0048d850
void FrCardComposeResultDlg::SetCard983(const unsigned long& typeId)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(typeId);
	if (pCard)
	{
		m_image.assign(pCard->Image, strlen(pCard->Image));
		m_image.append(".png");
	}
}
