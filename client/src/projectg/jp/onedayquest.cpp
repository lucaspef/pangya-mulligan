// JP 983: unidade 005176f0–0051c9e0, missão diária ("_questoneday"): FrOnedayQuest, FrOnedayQuestComplete e FrOnedayMentionSelect;
// ver include/jp/onedayquest.h. Sessão B98 (F3.7.10, 7º lote: `quests_presenca`). A F3.7.1 cortava em onedaymentionselect
// (00517a40–0051c0c0) e onedayquest (0051c0c0–0051c9e0); é uma unidade só (os três mapas de mensagens e as vtables misturam os dois
// pedaços). A unidade foi compilada com LTCG no 983 (005185f0 recebe o typeid em eax): critério FUNCIONAL. O resto da faixa são
// operadores das lambdas (005177a0…00517a20, 00517b90, 00519040…), instâncias da STL (vector/list/for_each/find_if) e os
// CreateForm<FrScratchResultDlg> (005193e0) / CreateForm<FrOnedayMentionSelect> (005194b0), que o compilador gera sozinho.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frstatic.h"
#include "frlistbox.h"
#include "frgraphicinterface.h"
#include "commonutil.h"
#include "wresrcmng.h"
#include "../../shared/itemmanager.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "messagemanager.h"
#include "scratchdlg.h"
#include "ucclibrary.h"
#include "lua983.h"
#include "questitem.h"
#include "questdocument.h"
#include "onedayquest.h"
#include <algorithm>

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

extern Fresh* g_pFresh; // 00f02340

using namespace _quest;

namespace
{
// visão local do CItemManager: 00a238f0 junta os typeids do ArtifactManaInfo.iff (+0x348) num vetor. Empréstimo.
class CItemManagerOnedayJp983 : public CItemManager
{
public:
	void GetArtifactManaIds983(std::vector<unsigned long>& ids); // 00a238f0
};

// 00a238f0: percorre o ArtifactManaInfo.iff (+0x348) em ordem e empilha o typeid de cada registro (node+0x18 = chave)
void CItemManagerOnedayJp983::GetArtifactManaIds983(std::vector<unsigned long>& ids)
{
	for (std::map<unsigned int, IFF983::sArtifactManaInfo>::iterator it = m_ArtifactManaInfoMap983.begin();
		it != m_ArtifactManaInfoMap983.end(); ++it)
		ids.push_back(it->first);
}

// campos dos registros .iff do 983 ainda sem nome (IFF983_REC só tem bFinal/TypeId)
inline const char* IffName983(const void* p) { return (const char*)p + 8; }
inline unsigned long IffDword983(const void* p, int offset) { return *(const unsigned long*)((const char*)p + offset); }

const char* const QUEST_SOLVED = "\304\371\275\272\306\256 \307\330\260\341"; // "퀘스트 해결"
const char* const QUEST_NONE = "\304\371\275\272\306\256 \276\370\300\275"; // "퀘스트 없음"
const char* const WRONG_ITEM_REQUEST = "\300\337\270\370\265\310 \276\306\300\314\305\333 \301\244\272\270\270\246 "
	"\277\344\303\273\307\317\277\264\275\300\264\317\264\331."; // "잘못된 아이템 정보를 요청하였습니다."

const unsigned long DAILY_TYPEID = 0x78800001; // a missão diária do "last_gift" / medidor total
}

namespace _gateway
{
namespace _questoneday
{
// 005185f0: missão do QuestItem.iff (0x78…) dos subtipos 1 e 2 (bits 22..25)
static bool IsOnedayQuestType983(unsigned long typeId)
{
	if ((typeId & 0xfc000000) != 0x78000000)
		return false;
	return ((typeId >> 22) & 0xf) - 1 <= 1;
}

// 00517740 (GetMessageMap); mapa 00e09b3c, inicializador 00cccc40
BEGIN_FRESH_MSGMAP(FrOnedayQuest, FrForm)
ON_FRESH_VI("futurework", FRCMD_INIT, FrOnedayQuest::OnFutureWorkInit983)
ON_FRESH_VI("futurework", FRCMD_OWNERDRAW, FrOnedayQuest::OnFutureWorkDraw983)
ON_FRESH_VI("currentwork", FRCMD_INIT, FrOnedayQuest::OnCurrentWorkInit983)
ON_FRESH_VI("currentwork", FRCMD_OWNERDRAW, FrOnedayQuest::OnCurrentWorkDraw983)
ON_FRESH_VI("req_today_quest", FRCMD_INIT, FrOnedayQuest::OnReqTodayInit983)
ON_FRESH_VV("req_today_quest", FRCMD_LBUTTONUP, FrOnedayQuest::OnReqTodayUp983)
ON_FRESH_VI("giveup_quest", FRCMD_INIT, FrOnedayQuest::OnGiveUpInit983)
ON_FRESH_VV("giveup_quest", FRCMD_LBUTTONUP, FrOnedayQuest::OnGiveUpUp983)
ON_FRESH_VI("quest_btn1", FRCMD_INIT, FrOnedayQuest::OnQuestBtn1Init983)
ON_FRESH_VI("quest_btn2", FRCMD_INIT, FrOnedayQuest::OnQuestBtn2Init983)
ON_FRESH_VI("quest_btn3", FRCMD_INIT, FrOnedayQuest::OnQuestBtn3Init983)
ON_FRESH_VV("quest_btn1", FRCMD_LBUTTONUP, FrOnedayQuest::OnQuestBtn1Up983)
ON_FRESH_VV("quest_btn2", FRCMD_LBUTTONUP, FrOnedayQuest::OnQuestBtn2Up983)
ON_FRESH_VV("quest_btn3", FRCMD_LBUTTONUP, FrOnedayQuest::OnQuestBtn3Up983)
ON_FRESH_VI("last_gift", FRCMD_INIT, FrOnedayQuest::OnLastGiftInit983)
ON_FRESH_VV("last_gift", FRCMD_LBUTTONUP, FrOnedayQuest::OnLastGiftUp983)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrOnedayQuest::OnCloseUp983)
ON_FRESH_VI("draw_gauge", FRCMD_OWNERDRAW, FrOnedayQuest::OnDrawGauge983)
END_FRESH_MSGMAP()

// 00517770
void FrOnedayQuest::OnProc(const float deltaTime)
{
	for (int i = 0; i < 4; i++)
		m_gauge[i].Process(deltaTime);
}

// 00517950
void FrOnedayQuest::AddEmptyItems983(int list)
{
	if (list < 2)
	{
		for (int i = 0; i < 3; i++)
			m_pList[list]->AddItem(NULL);
	}
}

// 005179e0
bool FrOnedayQuest::OnScratchResult983(int ret, FrForm* pForm)
{
	return true;
}

// 005179f0
void FrOnedayQuest::OnCloseUp983()
{
	Close(FrOK, true);
}

// 00517a00 (GetMessageMap); mapa 00e09b44, inicializador 00ccceb0
BEGIN_FRESH_MSGMAP(FrOnedayQuestComplete, FrForm)
ON_FRESH_VI("subject", FRCMD_INIT, FrOnedayQuestComplete::OnSubjectInit983)
ON_FRESH_VI("gift", FRCMD_INIT, FrOnedayQuestComplete::OnGiftInit983)
ON_FRESH_VI("bg1", FRCMD_INIT, FrOnedayQuestComplete::OnBg1Init983)
ON_FRESH_VV("bg1", FRCMD_OWNERDRAW, FrOnedayQuestComplete::OnBg1Draw983)
ON_FRESH_VI("bg2", FRCMD_INIT, FrOnedayQuestComplete::OnBg2Init983)
END_FRESH_MSGMAP()

// 00517a40 (GetMessageMap); mapa 00e09b4c, inicializador 00cccfb0
BEGIN_FRESH_MSGMAP(FrOnedayMentionSelect, FrForm)
ON_FRESH_VI("arin_yes", FRCMD_INIT, FrOnedayMentionSelect::OnYesInit983)
ON_FRESH_VI("arin_no", FRCMD_INIT, FrOnedayMentionSelect::OnNoInit983)
ON_FRESH_VV("arin_yes", FRCMD_LBUTTONUP, FrOnedayMentionSelect::OnYesUp983)
ON_FRESH_VV("arin_no", FRCMD_LBUTTONUP, FrOnedayMentionSelect::OnNoUp983)
END_FRESH_MSGMAP()

// 00517a50
FrOnedayMentionSelect::FrOnedayMentionSelect()
{
	m_pYes = NULL;
	m_pNo = NULL;
}

// 00517a90
FrOnedayMentionSelect::~FrOnedayMentionSelect()
{
}

// 00518630
void FrOnedayQuest::OnFutureWorkInit983(int param)
{
	m_pList[0] = KINDOF_CAST983(FrListBox, param);
}

// 005186a0: nome da missão de amanhã na linha (ou "퀘스트 없음")
void FrOnedayQuest::OnFutureWorkDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem)
	{
		FrGraphicInterface* pGI = GetGDI();
		const sQuestSlot983* pSlot = (const sQuestSlot983*)pItem->pData;
		FrListBox* pList = m_pList[0];
		WPoint pt;
		pt.x = pList->GetRect().x;
		if (pSlot)
		{
			pt.y = (float)(pItem->idx * pList->GetItemHeight() + pList->GetRect().y + 5.0);
			pGI->PrintText(pt, 0, IffName983(pSlot->pIff), NULL);
		}
		else
		{
			pt.y = (float)(pItem->idx * pList->GetItemHeight() + pList->GetRect().y + 5.0);
			pGI->PrintText(pt, 0, K2L_Compatibility(QUEST_NONE), NULL);
		}
	}
}

// 00518750
void FrOnedayQuest::OnCurrentWorkInit983(int param)
{
	m_pList[1] = KINDOF_CAST983(FrListBox, param);
}

// 005187c0: nome da missão de hoje ("퀘스트 해결" se já resolvida, "퀘스트 없음" na linha vazia)
void FrOnedayQuest::OnCurrentWorkDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem)
	{
		FrGraphicInterface* pGI = GetGDI();
		const sQuestSlot983* pSlot = (const sQuestSlot983*)pItem->pData;
		FrListBox* pList = m_pList[1];
		WPoint pt;
		pt.x = pList->GetRect().x;
		const char* text;
		if (pSlot)
		{
			if (pSlot->pItem)
			{
				pt.y = (float)(pItem->idx * pList->GetItemHeight() + pList->GetRect().y + 5.0);
				pGI->PrintText(pt, 0, IffName983(pSlot->pIff), NULL);
				return;
			}
			text = QUEST_SOLVED;
		}
		else
			text = QUEST_NONE;
		pt.y = (float)(pItem->idx * pList->GetItemHeight() + pList->GetRect().y + 5.0);
		pGI->PrintText(pt, 0, K2L_Compatibility(text), NULL);
	}
}

// 00518890
void FrOnedayQuest::OnReqTodayInit983(int param)
{
	m_pReqToday = KINDOF_CAST983(FrButton, param);
}

// 00518900
void FrOnedayQuest::OnGiveUpInit983(int param)
{
	m_pGiveUp = KINDOF_CAST983(FrButton, param);
}

// 00518970
void FrOnedayQuest::OnQuestBtn1Init983(int param)
{
	m_pQuestBtn[0] = KINDOF_CAST983(FrButton, param);
	m_pQuestBtn[0]->Enable(false);
}

// 005189e0
void FrOnedayQuest::OnQuestBtn2Init983(int param)
{
	m_pQuestBtn[1] = KINDOF_CAST983(FrButton, param);
	m_pQuestBtn[1]->Enable(false);
}

// 00518a50
void FrOnedayQuest::OnQuestBtn3Init983(int param)
{
	m_pQuestBtn[2] = KINDOF_CAST983(FrButton, param);
	m_pQuestBtn[2]->Enable(false);
}

// 00518ac0
void FrOnedayQuest::OnLastGiftInit983(int param)
{
	m_pLastGift = KINDOF_CAST983(FrButton, param);
	m_pLastGift->Enable(false);
}

// 00518b30
bool FrOnedayQuestComplete::HasTypes983() const
{
	return m_typeIds.empty() ? false : true;
}

// 00518b40
void FrOnedayQuestComplete::MoveUp983()
{
	WRect rc = GetRect();
	rc.y = (float)(rc.y - 75.0);
	SetRect(rc);
}

// 00518b90
void FrOnedayQuestComplete::MoveDown983()
{
	WRect rc = GetRect();
	rc.y = (float)(rc.y + 75.0);
	SetRect(rc);
}

// 00518be0
void FrOnedayQuestComplete::OnSubjectInit983(int param)
{
	m_pSubject = KINDOF_CAST983(FrStatic, param);
}

// 00518c50
void FrOnedayQuestComplete::OnGiftInit983(int param)
{
	m_pGift = KINDOF_CAST983(FrStatic, param);
	m_pGift->SetVisible(false);
}

// 00518cc0
void FrOnedayQuestComplete::OnBg1Init983(int param)
{
	m_pBg1 = KINDOF_CAST983(FrArea, param);
}

// 00518d30: ícones das conquistas lado a lado (50 cada), centrados na janela, 70 abaixo do topo
void FrOnedayQuestComplete::OnBg1Draw983()
{
	if (m_icons.empty())
		return;
	FrGraphicInterface* pGI = GetGDI();
	int count = (int)m_icons.size();
	float x = (float)((GetRect().x + GetRect().x + GetRect().w) * 0.5 - count * 50.0 * 0.5);
	for (int i = 0; i < count; i++)
	{
		const Bitmap* pBmp = m_icons[i];
		WRect rc(x, (float)(GetRect().y + 70.0), (float)pBmp->Width(), (float)pBmp->Height());
		pGI->DrawTexture(pBmp, rc, 0xffffffff, 0);
		x = (float)pBmp->Width() + x;
	}
}

// 00518e30
void FrOnedayQuestComplete::OnBg2Init983(int param)
{
	m_pBg2 = KINDOF_CAST983(FrArea, param);
}

// 00518f60
void FrOnedayMentionSelect::OnYesInit983(int param)
{
	m_pYes = KINDOF_CAST983(FrButton, param);
}

// 00518fd0
void FrOnedayMentionSelect::OnNoInit983(int param)
{
	m_pNo = KINDOF_CAST983(FrButton, param);
}

// 00519810: a missão `pItem` (de hoje) foi paga: medidor cheio com a figura do prêmio, botão desligado e a janela do prêmio
void FrOnedayQuest::OnQuestRewarded983(CQuestItem* pItem)
{
	sQuestSlot983* pSlot = m_pDoc->FindCurrentByUid983(pItem->GetUid983());
	if (!pSlot)
		return;
	FrListItem* pListItem = m_pList[1]->FindItem(pSlot);
	if (!pListItem)
		return;
	int index = pListItem->idx;
	CGaugeBar983& gauge = m_gauge[index];
	gauge.Reset();
	gauge.SetImages("quest_daily_reward.tga", "quest_gauge1.tga");
	gauge.m_b49 = false;
	gauge.SetValue(100.0f);
	if (index < 0)
		index = 0;
	else if (index > 2)
		index = 2;
	m_pQuestBtn[index]->Enable(false);
	pSlot->pItem = NULL;

	sBonusBall ball[5];
	memset(ball, 0, sizeof(ball));
	ball[0].count = (int)IffDword983(pSlot->pIff, 0xfc);
	ball[0].typeId = IffDword983(pSlot->pIff, 0xf4);
	ball[0].unused10 = 2;
	FrScratchResultDlg* pDlg = CreateForm<FrScratchResultDlg>(g_pFresh->GetManager(), this, "scratchresultdlg", NULL);
	const WRect& rc = pDlg->GetRect();
	pDlg->SetRect(WRect(rc.x, rc.y, rc.w, (float)(rc.h - 70.0)));
	pDlg->Open((FRESH_PFN_RESULT)&FrOnedayQuest::OnScratchResult983, 0xc2);
	pDlg->SetFadeout(false);
	pDlg->SetResultItem(ball, 1);

	if (!m_pQuestBtn[0]->IsEnabled() && !m_pQuestBtn[1]->IsEnabled() && !m_pQuestBtn[2]->IsEnabled())
		m_pGiveUp->Enable(false);
}

// 005199e0: pacote 0x153 (pegar o prêmio) com a missão da linha `index` de hoje
void FrOnedayQuest::SendReward983(int index)
{
	FrListItem* pListItem = m_pList[1]->GetItem(index);
	if (pListItem && ((const sQuestSlot983*)pListItem->pData)->pItem)
	{
		WSendPacket packet((enumClientPacket)0x153);
		packet.Encode4(1);
		packet.Encode4(((const sQuestSlot983*)pListItem->pData)->pItem->GetUid983());
		packet.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	}
}

// 00519ad0: resposta do "pegar o prêmio?" de um quest_btn
bool FrOnedayQuest::OnQuestBtnResult983(int ret, FrForm* pForm)
{
	if (ret != 1)
	{
		m_selBtn = -1;
		return true;
	}
	if (m_selBtn != -1)
	{
		SendReward983(m_selBtn);
		m_selBtn = -1;
	}
	else
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)K2L_Compatibility(WRONG_ITEM_REQUEST), 0, 0, 0, 0));
	if (m_pLastGift && !m_pLastGift->IsEnabled() && m_gauge[3].m_target == 100.0)
		m_gauge[3].SetValue(0.0f);
	return true;
}

// 00519bc0: os 4 medidores e as datas ("datetoday" sempre, "dateproc" só com missões de hoje)
void FrOnedayQuest::OnDrawGauge983(int param)
{
	for (int i = 0; i < 4; i++)
		m_gauge[i].Draw();
	FrGraphicInterface* pGI = GetGDI();
	unsigned long color = pGI->GetTextColor();
	pGI->SetTextColor(0xffffffff, 0xff000000);
	pGI->SetTextStyle(2);
	pGI->PrintText(WPoint(m_todayX + GetRect().x, m_todayY + GetRect().y), 0, m_todayDate.c_str(), NULL);
	if (m_pDoc->HasCurrent983())
		pGI->PrintText(WPoint(m_procX + GetRect().x, m_procY + GetRect().y), 0, m_procDate.c_str(), NULL);
	pGI->SetTextColor(color, 0xffffffff);
	pGI->SetTextStyle(0);
}

// 0051a1c0: listas pelo documento; sem missões de amanhã o "pedir" desliga; sem as de hoje, desistir/botões/medidores zeram
void FrOnedayQuest::RefreshLists983()
{
	CQuestDocument* pDoc = m_pDoc;
	FrListBox* pList = m_pList[0];
	pList->ClearItem();
	std::for_each(pDoc->m_future.begin(), pDoc->m_future.end(), [pList](sQuestSlot983& slot) { pList->AddItem(&slot); });
	pList = m_pList[1];
	pList->ClearItem();
	std::for_each(pDoc->m_current.begin(), pDoc->m_current.end(), [pList](sQuestSlot983& slot) { pList->AddItem(&slot); });
	if (pDoc->m_future.empty())
	{
		m_pReqToday->Enable(false);
		AddEmptyItems983(0);
	}
	else
		m_pReqToday->Enable(true);
	if (!pDoc->m_current.empty())
	{
		m_pGiveUp->Enable(true);
		return;
	}
	m_pGiveUp->Enable(false);
	m_pQuestBtn[0]->Enable(false);
	m_pQuestBtn[1]->Enable(false);
	m_pQuestBtn[2]->Enable(false);
	AddEmptyItems983(1);
	for (int i = 0; i < 3; i++)
		m_gauge[i].Reset();
	for (int i = 0; i < 3; i++)
		m_gauge[i].SetImages("quest_gauge1_mask.tga", "quest_gauge1.tga");
}

// 0051a320
void FrOnedayQuest::RefreshGauges983()
{
	CQuestDocument* pDoc = m_pDoc;
	for (std::vector<sQuestSlot983>::iterator it = pDoc->m_current.begin(); it != pDoc->m_current.end(); ++it)
	{
		if (!it->pItem)
			continue;
		FrListItem* pListItem = m_pList[1]->FindItem(&*it);
		if (!pListItem)
			continue;
		int index = pListItem->idx;
		if (it->pItem->GetState983() == 4)
		{
			if (index < 0)
				index = 0;
			else if (index > 2)
				index = 2;
			m_pQuestBtn[index]->Enable(true);
		}
	}
	for (std::vector<sQuestSlot983>::iterator it = pDoc->m_current.begin(); it != pDoc->m_current.end(); ++it)
	{
		if (!it->pCount || !it->pItem)
			continue;
		float value = it->pItem->GetProgress983(0);
		if (value < 0.0f)
			value = 0.0f;
		else if (value > 100.0)
			value = 100.0f;
		FrListItem* pListItem = m_pList[1]->FindItem(&*it);
		if (pListItem)
			m_gauge[pListItem->idx].SetValue(value);
	}
	if (m_dailies.empty())
		return;
	std::vector<CQuestItem*>::iterator daily = std::find_if(m_dailies.begin(), m_dailies.end(),
		[](CQuestItem* p) { return p->GetTypeId983() == DAILY_TYPEID; });
	if (daily == m_dailies.end())
		return;
	float value = (*daily)->GetProgress10_983(0);
	for (std::vector<sQuestGiftBtn983>::iterator it = m_giftBtns.begin(); it != m_giftBtns.end(); ++it)
	{
		if (!it->pBtn)
			return;
		if (it->pBtn->IsEnabled())
			value = 100.0f;
	}
	if (value < 0.0f)
		value = 0.0f;
	else if (value > 100.0)
		value = 100.0f;
	m_gauge[3].SetValue(value);
}

// 0051a4e0
void FrOnedayQuest::OnDailyRewarded983(CQuestItem* pItem)
{
	for (std::vector<sQuestGiftBtn983>::iterator it = m_giftBtns.begin(); it != m_giftBtns.end(); ++it)
	{
		if (it->typeId == pItem->GetTypeId983())
		{
			if (!pItem->IsActive983())
			{
				it->pBtn->Enable(false);
				it->pBtn->SetButtonImg("quest_daily_btn_gift_Hover", FrButton::NORMAL);
			}
			return;
		}
	}
}

// 0051a550: 0x152 (pedir as de amanhã: todas) / 0x154 (desistir das de hoje: só as não resolvidas), com os uids
void FrOnedayQuest::SendQuestList983(int packetId, bool bFuture)
{
	std::vector<sQuestSlot983>& slots = bFuture ? m_pDoc->m_future : m_pDoc->m_current;
	WSendPacket packet((enumClientPacket)packetId);
	if (packetId == 0x154)
	{
		int count = 0;
		std::for_each(slots.begin(), slots.end(), [&count](sQuestSlot983& slot) {
			if (slot.pItem)
				++count;
		});
		packet.Encode4(count);
		std::for_each(slots.begin(), slots.end(), [&packet](sQuestSlot983& slot) {
			if (slot.pItem)
				packet.Encode4(slot.pItem->GetUid983());
		});
	}
	else
	{
		packet.Encode4(slots.size());
		std::for_each(slots.begin(), slots.end(), [&packet](sQuestSlot983& slot) {
			packet.Encode4(slot.pItem->GetUid983());
		});
	}
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// 0051a690: pacote 0x153 com a diária `typeId`
void FrOnedayQuest::SendDailyReward983(unsigned long typeId)
{
	std::vector<CQuestItem*>::iterator it = std::find_if(m_dailies.begin(), m_dailies.end(),
		[typeId](CQuestItem* p) { return typeId == p->GetTypeId983(); });
	if (it == m_dailies.end())
		return;
	WSendPacket packet((enumClientPacket)0x153);
	packet.Encode4(1);
	packet.Encode4((*it)->GetUid983());
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// 0051a790
void FrOnedayQuest::ShowQuestMention983()
{
	FrOnedayMentionSelect* pDlg = CreateForm<FrOnedayMentionSelect>(g_pFresh->GetManager(), this, "oneday_quest_mention", NULL);
	pDlg->SetMessage(_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2c3), false);
	pDlg->Open((FRESH_PFN_RESULT)&FrOnedayQuest::OnQuestBtnResult983, 3);
}

// 0051a810
bool FrOnedayQuest::HasActiveDaily983(unsigned long typeId)
{
	return std::find_if(m_dailies.begin(), m_dailies.end(),
		[typeId](CQuestItem* p) { return p->GetTypeId983() == typeId && p->IsActive983(); }) != m_dailies.end();
}

// 0051a840
bool FrOnedayQuest::OnGiveUpResult983(int ret, FrForm* pForm)
{
	if (ret == 1)
		SendQuestList983(0x154, false);
	return true;
}

// 0051a860: resposta do "pegar o prêmio?" do last_gift: medidor total zera, 0x153 e a janela do prêmio (0x1a0001c4 × 1)
bool FrOnedayQuest::OnGiftResult983(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		if (m_giftTypeId)
		{
			m_gauge[3].SetValue(0.0f);
			SendDailyReward983(m_giftTypeId);
			m_giftTypeId = 0;
		}
		else
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)K2L_Compatibility(WRONG_ITEM_REQUEST), 0, 0, 0, 0));
		sBonusBall ball;
		ball.typeId = 0x1a0001c4;
		ball.count = 1;
		ball.unused10 = 2;
		FrScratchResultDlg* pDlg = CreateForm<FrScratchResultDlg>(g_pFresh->GetManager(), this, "scratchresultdlg", NULL);
		const WRect& rc = pDlg->GetRect();
		pDlg->SetRect(WRect(rc.x, rc.y, rc.w, (float)(rc.h - 70.0)));
		pDlg->Open((FRESH_PFN_RESULT)&FrOnedayQuest::OnScratchResult983, 0xc2);
		pDlg->SetFadeout(false);
		pDlg->SetResultItem(&ball, 1);
	}
	else
		m_giftTypeId = 0;
	return true;
}

// 0051a9d0: "desistir?" (0x2be)
void FrOnedayQuest::OnGiveUpUp983()
{
	FrOnedayMentionSelect* pDlg = CreateForm<FrOnedayMentionSelect>(g_pFresh->GetManager(), this, "oneday_quest_mention", NULL);
	pDlg->SetMessage(_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2be), false);
	pDlg->Open((FRESH_PFN_RESULT)&FrOnedayQuest::OnGiveUpResult983, 3);
}

// 0051aa50
void FrOnedayQuest::OnQuestBtn1Up983()
{
	m_selBtn = 0;
	ShowQuestMention983();
}

// 0051aa60
void FrOnedayQuest::OnQuestBtn2Up983()
{
	m_selBtn = 1;
	ShowQuestMention983();
}

// 0051aa70
void FrOnedayQuest::OnQuestBtn3Up983()
{
	m_selBtn = 2;
	ShowQuestMention983();
}

// 0051ad00: liga o botão de prêmio das diárias cuja contagem (CSharedDoc+0x3840) chegou à pedida e que estão COMPLETE
void FrOnedayQuest::RefreshGiftButtons983()
{
	for (std::vector<CQuestItem*>::iterator it = m_dailies.begin(); it != m_dailies.end(); ++it)
	{
		CQuestItem* pItem = *it;
		std::list<sQuestCount983>& counts = Doc()->m_questCounts983;
		std::list<sQuestCount983>::iterator c = std::find_if(counts.begin(), counts.end(),
			[pItem](sQuestCount983& count) { return pItem->GetCounterUid983(0) == count.uid; });
		if (c == Doc()->m_questCounts983.end() || !c->bValid)
			continue;
		IFF983::sQuestItem* pIff = ItemManager()->FindQuestItem983(pItem->GetTypeId983());
		if (!pIff)
			continue;
		IFF983::sQuestStuff* pStuff = ItemManager()->FindQuestStuff983(IffDword983(pIff, 0xcc));
		if (!pStuff || (long)IffDword983(pStuff, 0xd4) > (long)c->count || pItem->GetState983() != 4)
			continue;
		unsigned long typeId = pItem->GetTypeId983();
		std::vector<sQuestGiftBtn983>::iterator b = std::find_if(m_giftBtns.begin(), m_giftBtns.end(),
			[typeId](sQuestGiftBtn983& btn) { return btn.typeId == typeId; });
		if (b != m_giftBtns.end())
			b->pBtn->Enable(true);
	}
}

// 0051ae10: "pegar o prêmio?" (0x2c3) do last_gift
void FrOnedayQuest::ShowGiftMention983()
{
	FrOnedayMentionSelect* pDlg = CreateForm<FrOnedayMentionSelect>(g_pFresh->GetManager(), this, "oneday_quest_mention", NULL);
	pDlg->SetMessage(_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2c3), false);
	pDlg->Open((FRESH_PFN_RESULT)&FrOnedayQuest::OnGiftResult983, 3);
}

// 0051ae90
void FrOnedayQuest::OnGiftBtn983(unsigned long typeId)
{
	if (HasActiveDaily983(typeId))
	{
		m_pLastGift->Enable(false);
		ShowGiftMention983();
	}
}

// 0051b050
void FrOnedayQuest::OnLastGiftUp983()
{
	m_giftTypeId = DAILY_TYPEID;
	OnGiftBtn983(DAILY_TYPEID);
}

// 0051b070
FrOnedayQuestComplete::FrOnedayQuestComplete()
	: m_count(0)
	, m_pSubject(NULL)
	, m_pGift(NULL)
	, m_mode(0)
{
	m_pBg1 = NULL;
	m_pBg2 = NULL;
}

// 0051b120
FrOnedayQuestComplete::~FrOnedayQuestComplete()
{
}

// 0051b3f0: resposta do "pedir as de amanhã?": 0x152 e a data do andamento vira a de hoje
bool FrOnedayQuest::OnReqTodayResult983(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		SendQuestList983(0x152, true);
		m_procDate = m_todayDate;
	}
	return true;
}

// 0051b430: com missões de hoje ainda abertas, aviso (0x2bd); senão "pedir?" (0x2bc)
void FrOnedayQuest::OnReqTodayUp983()
{
	if (m_pDoc->HasCurrent983())
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2bd), 0, 0, 0, 0));
		return;
	}
	FrOnedayMentionSelect* pDlg = CreateForm<FrOnedayMentionSelect>(g_pFresh->GetManager(), this, "oneday_quest_mention", NULL);
	pDlg->SetMessage(_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2bc), false);
	pDlg->Open((FRESH_PFN_RESULT)&FrOnedayQuest::OnReqTodayResult983, 3);
}

IMPLEMENT_OBJECT(FrOnedayQuestComplete, FrForm)

// 0051b5e0: 5 s; conquista (typeid 0x4c…) troca os fundos, junta os ícones e desce o texto/mostra o prêmio
bool FrOnedayQuestComplete::OnInit()
{
	SetTimeLimit(5.0f);
	if (!m_typeIds.empty())
	{
		if ((m_typeIds[0] & 0xfc000000) == 0x4c000000)
			m_mode = 1;
		if (m_mode == 1)
		{
			m_pBg1->SetBgImg("challenge_complete_bg1");
			m_pBg2->SetBgImg("challenge_complete_bg2");
			for (std::vector<unsigned long>::iterator it = m_typeIds.begin(); it != m_typeIds.end(); ++it)
			{
				IFF983::sAchievement* pAch = ItemManager()->FindAchievement983(*it);
				const Bitmap* pBmp;
				if (pAch && (pBmp = g_pFresh->GetBitmap((const char*)pAch + 0x49)) != NULL)
					m_icons.push_back(pBmp);
			}
			const WRect& rcSubject = m_pSubject->GetRect();
			m_pSubject->SetRect(WRect(rcSubject.x, (float)(rcSubject.y + 15.0), rcSubject.w, rcSubject.h));
			m_pGift->SetVisible(true);
			const WRect& rcGift = m_pGift->GetRect();
			m_pGift->SetRect(WRect(rcGift.x, (float)(rcGift.y + 18.0), rcGift.w, rcGift.h));
		}
	}
	return FrForm::OnInit();
}

// 0051b760: "  <missão> <0x2c0>" por missão (e o total 0x2bf com mais de uma) no "subject"
void FrOnedayQuestComplete::SetQuests983(const std::vector<CQuestItem*>& quests)
{
	for (std::vector<CQuestItem*>::const_iterator it = quests.begin(); it != quests.end(); ++it)
	{
		IFF983::sQuestItem* pIff = ItemManager()->FindQuestItem983((*it)->GetTypeId983());
		if (!pIff)
			return;
		++m_count;
		m_text += MakeStr("  %s %s", IffName983(pIff),
			_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2c0));
		m_typeIds.push_back((*it)->GetTypeId983());
	}
	if (m_count > 1)
		m_text += MakeStr(_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2bf), m_count);
	m_pSubject->SetCaption(m_text.c_str());
}

// 0051b950: "subject" com as concluídas (fora as diárias) e "gift" com os prêmios: pang somado ("%d 점") e itens ("%s %d개 획득"),
// sem os artefatos de mana
void FrOnedayQuestComplete::SetDone983(const std::vector<sQuestDone983>& done)
{
	std::vector<unsigned long> manaIds;
	((CItemManagerOnedayJp983*)ItemManager())->GetArtifactManaIds983(manaIds);
	if (done.empty())
		return;
	std::string pangText;
	std::string itemText;
	int pang = 0;
	for (std::vector<sQuestDone983>::const_iterator it = done.begin(); it != done.end(); ++it)
	{
		sQuestDone983 d = *it;
		if ((d.typeId & 0xfc000000) == 0x78000000)
			continue;
		IFF983::sQuestStuff* pStuff = ItemManager()->FindQuestStuff983(d.stuffId);
		if (!pStuff)
			return;
		++m_count;
		m_text += MakeStr("  %s %s", IffName983(pStuff),
			_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2c0));
		for (int i = 0; i < 3; i++)
		{
			unsigned long typeId = IffDword983(pStuff, 0xe8 + i * 4);
			if (!typeId)
				continue;
			int count = (int)IffDword983(pStuff, 0xf4 + i * 4);
			IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeId);
			if (typeId == 0x6c000001)
			{
				if (pang == 0)
					pangText = pItem->Name;
				pang += count;
			}
			else if (std::find(manaIds.begin(), manaIds.end(), typeId) == manaIds.end())
			{
				itemText += MakeStr(K2L_Compatibility("%s %d\260\263 \310\271\265\346"), pItem->Name, count); // "%s %d개 획득"
				itemText += "  ";
			}
		}
		m_typeIds.push_back(d.typeId);
	}
	if (m_count > 1)
		m_text += MakeStr(_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2c5), m_count);
	if (!m_typeIds.empty())
		m_pSubject->SetCaption(m_text.c_str());
	std::string gift;
	if (!pangText.empty())
	{
		pangText += " ";
		pangText += MakeStr(K2L_Compatibility("%d \301\241"), pang); // "%d 점"
		gift += pangText;
	}
	if (!itemText.empty())
	{
		if (!gift.empty())
			gift += "  ";
		gift += itemText;
	}
	if (m_pGift)
		m_pGift->SetCaption(gift.c_str());
}

// 0051c0c0
FrOnedayQuest::FrOnedayQuest()
{
	m_pList[0] = NULL;
	m_pList[1] = NULL;
	m_pQuestBtn[0] = NULL;
	m_pQuestBtn[1] = NULL;
	m_pQuestBtn[2] = NULL;
	m_pReqToday = NULL;
	m_pGiveUp = NULL;
	m_pLastGift = NULL;
	m_pad983_130 = 0;
	m_pad983_268 = 0;
	m_pRewardOverlay = NULL;
	m_pDoc = NULL;
	m_selBtn = -1;
	m_giftTypeId = 0;
}

// 0051c210
FrOnedayQuest::~FrOnedayQuest()
{
	if (g_resrcmng && m_pRewardOverlay)
	{
		g_resrcmng->Release(m_pRewardOverlay);
		m_pRewardOverlay = NULL;
	}
	AfxPostMsg(NULL, "AvatarChat", 0x58, 0, 0, 0, 0);
}

IMPLEMENT_OBJECT(FrOnedayQuest, FrForm)

// 0051c610: medidores pela tabela "quest_ui_config", datas do documento, diárias do jogador e o estado das listas
bool FrOnedayQuest::OnInit()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("onedayquest.lua");
	lua_system::lua_table_helper tbl = script->GetTable("quest_ui_config");
	float procX = (float)LUA983_GET(tbl, int, "procgaugex");
	float procY = (float)LUA983_GET(tbl, int, "procgaugey");
	float totalX = (float)LUA983_GET(tbl, int, "totalgaugex");
	float totalY = (float)LUA983_GET(tbl, int, "totalgaugey");
	procX = GetRect().x + procX;
	procY = procY + GetRect().y;
	totalX = GetRect().x + totalX;
	totalY = totalY + GetRect().y;
	m_todayX = LUA983_GET(tbl, float, "datetodayx");
	m_todayY = LUA983_GET(tbl, float, "datetodayy");
	m_procX = LUA983_GET(tbl, float, "dateprocx");
	m_procY = LUA983_GET(tbl, float, "dateprocy");
	sQuestGiftBtn983 btn;
	btn.typeId = DAILY_TYPEID;
	btn.pBtn = m_pLastGift;
	m_giftBtns.push_back(btn);
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);

	m_gauge[0].Init("quest_gauge1_mask.tga", "quest_gauge1.tga", WRect(procX, procY, 0.0f, 100.0f), 0, true, 1.0f, true, true);
	m_gauge[1].Init("quest_gauge1_mask.tga", "quest_gauge1.tga", WRect(procX, (float)(procY + 26.0), 0.0f, 100.0f), 0, true,
		1.0f, true, true);
	m_gauge[2].Init("quest_gauge1_mask.tga", "quest_gauge1.tga", WRect(procX, (float)(procY + 52.0), 0.0f, 100.0f), 0, true,
		1.0f, true, true);
	m_gauge[3].Init("quest_gauge2_mask.tga", "quest_gauge2.tga", WRect(totalX, totalY, 0.0f, 100.0f), 0, true, 1.0f, true, true);
	m_gauge[0].Reset();
	m_gauge[1].Reset();
	m_gauge[2].Reset();
	m_gauge[3].Reset();
	m_pRewardOverlay = g_resrcmng->GetOverlay("quest_daily_reward.tga", 0);

	CQuestDocument* pDoc = m_pDoc;
	if (pDoc)
	{
		if (pDoc->m_todayDate.wYear)
			m_todayDate = MakeStr("%d-%d-%d", pDoc->m_todayDate.wYear, pDoc->m_todayDate.wMonth, pDoc->m_todayDate.wDay);
		if (pDoc->m_procDate.wYear)
			m_procDate = MakeStr("%d-%d-%d", pDoc->m_procDate.wYear, pDoc->m_procDate.wMonth, pDoc->m_procDate.wDay);
	}

	// 0051bec0: as missões do jogador (CSharedDoc+0x384c) de tipo diário cujo typeid está na lista
	unsigned long ids[1] = { DAILY_TYPEID };
	std::vector<CQuestItem*>& dailies = m_dailies;
	std::for_each(Doc()->m_quests983.begin(), Doc()->m_quests983.end(), [&dailies, &ids](CQuestItem& quest) {
		if (IsOnedayQuestType983(quest.GetTypeId983()))
		{
			for (unsigned int i = 0; i < sizeof(ids) / sizeof(ids[0]); i++)
			{
				if (quest.GetTypeId983() == ids[i])
				{
					dailies.push_back(&quest);
					break;
				}
			}
		}
	});

	if (m_pDoc)
	{
		RefreshLists983();
		RefreshGiftButtons983();
		RefreshGauges983();
	}
	return FrForm::OnInit();
}

IMPLEMENT_OBJECT(FrOnedayMentionSelect, FrForm)
}
}
