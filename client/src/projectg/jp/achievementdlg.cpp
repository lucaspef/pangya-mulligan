// JP 983: unidade 004e11b0–004ea7a0, janela das conquistas "ach_main" (_gateway::_questoneday::FrAchievement) e os três pintores
// aninhados; ver include/jp/achievementdlg.h. Sessão B102 (F3.7.10, 11º lote). A F3.7.1 cortava em ahievementpainter
// (004e11b0–004e13a0) e achievement_p2 (004e13a0–004ea7a0); é uma unidade só. O resto da faixa são instâncias da STL (map/list/
// vector/sort dos elementos e das conquistas), operadores de comparação e o CreateForm, que o compilador gera sozinho.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "frscrollbar.h"
#include "frgraphicinterface.h"
#include "commonutil.h"
#include "../../shared/itemmanager.h"
#include "shareddoc.h"
#include "actor.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "titlefontex.h"
#include "ucclibrary.h"
#include "lua983.h"
#include "achievement.h"
#include "questdocument.h"
#include "achievementdlg.h"
#include <math.h>
#include <algorithm>

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

extern Fresh* g_pFresh; // 00f02340

// S5 (sharedutilities.cpp da frente A) — empréstimo, nome meu (ver gproomtimersteward.cpp)
namespace S5
{
void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); // 00b307f0
}

namespace
{
// QuestStuff.iff (0x10c) sem campos na A: visão local com o que a janela lê
struct sQuestStuffView983
{
	unsigned char m_pad983_000[8];
	char name[0x40]; // +0x08
	unsigned char m_pad983_048[0x8c];
	int goal; // +0xd4: contagem que cumpre a peça
	unsigned char m_pad983_0d8[0x10];
	unsigned long prizeIds[3]; // +0xe8: typeids das 3 posições do prêmio (0x6c000001 = ponto de conquista)
	int prizeCount[3]; // +0xf4: quantidades das 3 posições do prêmio
	unsigned char m_pad983_100[0xc];
};
C_ASSERT(sizeof(sQuestStuffView983) == 0x10c);
inline const sQuestStuffView983* StuffView983(const IFF983::sQuestStuff* p) { return (const sQuestStuffView983*)p; }

// visão local do FrListBox: 00bef7e0 = índice da primeira linha visível (−1 com a lista vazia). Empréstimo (frlistbox.cpp é da A).
class FrListBoxAchJp983 : public FrListBox
{
public:
	int GetTopIndex983(); // 00bef7e0
};

// 004e5170 / 004e51d0: o laço da troca de aba fica entre as mensagens 1 e 2 ao ator principal (um objeto de 1 byte na pilha)
struct sAchBusy983
{
	sAchBusy983(); // 004e5170
	~sAchBusy983(); // 004e51d0
};
}

// 00bef7e0 (frlistbox.cpp:358): sem itens (m_itemList.size() +0xd4 == 0) = -1; senao (*m_itemTop)->idx (+0xf0, no +0x8, +0x14)
int FrListBoxAchJp983::GetTopIndex983()
{
	C_ASSERT_VC10(offsetof(FrListBoxAchJp983, m_itemTop) == 0xf0);
	C_ASSERT(offsetof(FrListItem, idx) == 0x14);
	if (m_itemList.size() == 0)
		return -1;
	return (*m_itemTop)->idx;
}

// 004e5170
sAchBusy983::sAchBusy983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
}

// 004e51d0
sAchBusy983::~sAchBusy983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
}

// 004e2820 (o CQuestDocument usa como chave das buscas por categoria)
sAchievementView983::sAchievementView983()
{
	lastStuffId983 = 0;
	category = -1;
	unused5fc983 = 0;
	for (int i = 0; i < 10; i++)
	{
		memset(names[i], 0, sizeof(names[i]));
		stuffIds[i] = 0;
	}
}

namespace _gateway
{
namespace _questoneday
{
// 004e13a0 (GetMessageMap); mapa 00e0708c, inicializador 00cca5a0
BEGIN_FRESH_MSGMAP(FrAchievement, FrForm)
ON_FRESH_VI("btn_summary", FRCMD_INIT, FrAchievement::OnSummaryBtnInit983)
ON_FRESH_VI("btn_char", FRCMD_INIT, FrAchievement::OnCharBtnInit983)
ON_FRESH_VI("btn_gamemode", FRCMD_INIT, FrAchievement::OnGameModeBtnInit983)
ON_FRESH_VI("btn_adventure", FRCMD_INIT, FrAchievement::OnAdventureBtnInit983)
ON_FRESH_VI("btn_statistics", FRCMD_INIT, FrAchievement::OnStatisticsBtnInit983)
ON_FRESH_VI("btn_item", FRCMD_INIT, FrAchievement::OnItemBtnInit983)
ON_FRESH_VI("btn_system", FRCMD_INIT, FrAchievement::OnSystemBtnInit983)
ON_FRESH_VI("btn_challenge", FRCMD_INIT, FrAchievement::OnChallengeBtnInit983)
ON_FRESH_VI("btn_event", FRCMD_INIT, FrAchievement::OnEventBtnInit983)
ON_FRESH_VV("btn_summary", FRCMD_LBUTTONUP, FrAchievement::OnSummaryBtnUp983)
ON_FRESH_VV("btn_char", FRCMD_LBUTTONUP, FrAchievement::OnCharBtnUp983)
ON_FRESH_VV("btn_gamemode", FRCMD_LBUTTONUP, FrAchievement::OnGameModeBtnUp983)
ON_FRESH_VV("btn_adventure", FRCMD_LBUTTONUP, FrAchievement::OnAdventureBtnUp983)
ON_FRESH_VV("btn_statistics", FRCMD_LBUTTONUP, FrAchievement::OnStatisticsBtnUp983)
ON_FRESH_VV("btn_item", FRCMD_LBUTTONUP, FrAchievement::OnItemBtnUp983)
ON_FRESH_VV("btn_system", FRCMD_LBUTTONUP, FrAchievement::OnSystemBtnUp983)
ON_FRESH_VV("btn_challenge", FRCMD_LBUTTONUP, FrAchievement::OnChallengeBtnUp983)
ON_FRESH_VV("btn_event", FRCMD_LBUTTONUP, FrAchievement::OnEventBtnUp983)
ON_FRESH_VI("area_summary", FRCMD_INIT, FrAchievement::OnSummaryAreaInit983)
ON_FRESH_VI("area_summary", FRCMD_OWNERDRAW, FrAchievement::OnSummaryAreaDraw983)
ON_FRESH_VI("area_element_layer", FRCMD_INIT, FrAchievement::OnElementLayerInit983)
ON_FRESH_VI("area_element_layer", FRCMD_OWNERDRAW, FrAchievement::OnElementLayerDraw983)
ON_FRESH_VI("list_ach_recent", FRCMD_INIT, FrAchievement::OnRecentListInit983)
ON_FRESH_VV("list_ach_recent", FRCMD_MOUSEMOVE, FrAchievement::OnRecentListMove983)
ON_FRESH_VI("list_ach_element", FRCMD_INIT, FrAchievement::OnElementListInit983)
ON_FRESH_VI("list_ach_element", FRCMD_OWNERDRAW, FrAchievement::OnElementListDraw983)
ON_FRESH_VV("list_ach_element", FRCMD_LBUTTONUP, FrAchievement::OnElementListUp983)
ON_FRESH_VI("list_ach_item", FRCMD_INIT, FrAchievement::OnItemListInit983)
ON_FRESH_VV("list_ach_item", FRCMD_MOUSEMOVE, FrAchievement::OnItemListMove983)
END_FRESH_MSGMAP()

// 004e1450
void FrAchievement::UpdateCategoryGauges983()
{
	for (int i = 0; i < CATEGORY_COUNT; i++)
	{
		int point = m_pDoc->GetCategoryPoint983(i);
		m_categoryPoints[i] = point;
		int max = m_pDoc->m_categoryPoints[i];
		float rate;
		if (max)
		{
			rate = (float)point / max;
			rate = (float)(rate * 100.0);
			rate = (float)floor(rate);
		}
		else
			rate = 0.0f;
		m_categoryGauge[i].SetNow983(rate);
	}
}

// 004e14f0
int FrAchievement::FindRecentRow983(const WPoint& pt)
{
	for (int i = 0; i < m_recentCount; i++)
	{
		if (m_recentRows[i][0] < pt.x && pt.x < m_recentRows[i][1])
			return i;
	}
	return -1;
}

// altura da lista "area_element_layer" do 983 (constantes próprias da unidade em 00d04f48/00d04f50, não no conjunto __real@)
static const double ELEMENT_LAYER_HEIGHT983 = 626.0;
static const double PRIZE_ROW_HEIGHT983 = 63.0;

// 004e28d0: altura da lista de peças aberta com n peças (metade do que sobra da área)
float CalcLayerHeight983(int n)
{
	float x = (float)n;
	float h = (float)(PRIZE_ROW_HEIGHT983 * x + x * 3.0);
	h = (float)(ELEMENT_LAYER_HEIGHT983 - h);
	h = h <= 0.0 ? 0.0f : h;
	return (float)(h * 0.5);
}

// 004e2930
void FrAchievement::SetCategory983(int category)
{
	m_category = category;
	if (category == -1)
		m_mode = 0;
	else if (category >= 0 && category < CATEGORY_COUNT)
		m_mode = 1;

	if (m_mode == 0)
	{
		m_painters[0]->SetShow983(true);
		m_painters[1]->SetShow983(false);
		m_painters[2]->SetShow983(false);
	}
	else
	{
		m_painters[0]->SetShow983(false);
		m_painters[1]->SetShow983(true);
		m_painters[2]->SetShow983(true);
	}
}

// 004e29b0
void FrAchievement::CloseItem983(FrListItem* pItem)
{
	((sAchElement983*)pItem->pData)->m_bOpen = 0;
	m_pItemList->m_nFlags.Disable(FWF_TOOLTIPS);
	m_pItemList->ClearItem();
	m_pItemList->Enable(false); // 983 004e29e4: vtbl+0x18 (Enable), a lista nunca some
}

// posição da lista de peças ao lado da linha aberta (constantes próprias da unidade em 00d04f58/00d04f60)
static const double ITEM_LIST_OFFSET_Y983 = 70.0;
static const double ITEM_LIST_OFFSET_X983 = 9.0;

// 004e29f0: "list_ach_item" ao lado da linha, uma figura (m_bitmaps[16]) por peça
void FrAchievement::PlaceItemList983(FrListItem* pItem)
{
	const sAchElement983* pElem = (const sAchElement983*)pItem->pData;
	float n = (float)(int)pElem->prizes.size();
	const Bitmap* pBmp = m_bitmaps[16];
	WRect rect(pItem->pos.x + pElem->height + ITEM_LIST_OFFSET_X983, pItem->pos.y + ITEM_LIST_OFFSET_Y983, n * pBmp->Width() + n * 2.0f,
		(float)pBmp->Height());
	m_pItemList->SetRect(rect);
	m_pItemList->SetItemSize((int)rect.w, (int)rect.h);
}

// 004e2ac0
int FrAchievement::FindItemColumn983(float width, const WPoint& pt)
{
	const WRect& rect = m_pItemList->GetRect();
	for (int i = 1; i <= 10; i++)
	{
		if (pt.x < i * width + rect.x)
		{
			int col = i - 1;
			if (WRect(col * width + rect.x, rect.y, width, rect.h).IsInRect(pt))
				return col;
			return -1;
		}
	}
	return -1;
}

// 004e2b80
void FrAchievement::OnSummaryBtnInit983(int param)
{
	m_pTabBtn[0] = KINDOF_CAST983(FrButton, param);
}

// 004e2bf0
void FrAchievement::OnCharBtnInit983(int param)
{
	m_pTabBtn[1] = KINDOF_CAST983(FrButton, param);
}

// 004e2c60
void FrAchievement::OnGameModeBtnInit983(int param)
{
	m_pTabBtn[2] = KINDOF_CAST983(FrButton, param);
}

// 004e2cd0
void FrAchievement::OnAdventureBtnInit983(int param)
{
	m_pTabBtn[3] = KINDOF_CAST983(FrButton, param);
}

// 004e2d40
void FrAchievement::OnStatisticsBtnInit983(int param)
{
	m_pTabBtn[4] = KINDOF_CAST983(FrButton, param);
}

// 004e2db0
void FrAchievement::OnItemBtnInit983(int param)
{
	m_pTabBtn[5] = KINDOF_CAST983(FrButton, param);
}

// 004e2e20
void FrAchievement::OnSystemBtnInit983(int param)
{
	m_pTabBtn[6] = KINDOF_CAST983(FrButton, param);
}

// 004e2e90
void FrAchievement::OnChallengeBtnInit983(int param)
{
	m_pTabBtn[7] = KINDOF_CAST983(FrButton, param);
}

// 004e2f00
void FrAchievement::OnEventBtnInit983(int param)
{
	m_pTabBtn[8] = KINDOF_CAST983(FrButton, param);
	// 004e2f60: vtbl+0x18 = Enable(false) (aba Event fica apagada, nao some)
	m_pTabBtn[8]->Enable(false);
}

// 004e2f70
void FrAchievement::OnSummaryAreaInit983(int param)
{
	m_pSummaryArea = KINDOF_CAST983(FrArea, param);
}

// 004e2fe0
void FrAchievement::OnSummaryAreaDraw983(int param)
{
	if (m_painters[0]->m_bShow)
		m_painters[0]->Draw(param);
}

// 004e3000
void FrAchievement::OnElementLayerInit983(int param)
{
	m_pElementLayer = KINDOF_CAST983(FrArea, param);
}

// 004e3070
void FrAchievement::OnRecentListInit983(int param)
{
	m_pRecentList = KINDOF_CAST983(FrListBox, param);
}

// 004e30e0
void FrAchievement::OnElementListInit983(int param)
{
	m_pElementList = KINDOF_CAST983(FrListBox, param);
}

// 004e3150
void FrAchievement::OnElementListDraw983(int param)
{
	m_painters[1]->Draw(param);
}

// 004e3170
void FrAchievement::OnItemListInit983(int param)
{
	m_pItemList = KINDOF_CAST983(FrListBox, param);
	m_pItemList->Enable(false); // 983 004e31d7: vtbl+0x18 (Enable)
}

// constantes dos pintores (próprias da unidade em 00d04f68–00d04fc0, não no conjunto __real@)
static const double DATE_OFFSET_X983 = 37.0;
static const float DATE_SPACE983 = -0.5f;
static const double POINT_OFFSET_X983 = 40.0; // também o y da data
static const double POINT_OFFSET_Y983 = 15.0;
static const float POINT_SPACE983 = -5.0f;
static const double PRIZE_ICON_OFFSET_Y983 = 12.0;
static const double STUFF_NAME_OFFSET_Y983 = 22.0;
static const double COUNTER_OFFSET_Y983 = 51.0;
static const double COUNTER_OFFSET_X983 = 510.0;
static const double PRIZE_TEXT_OFFSET_Y983 = 47.0;
static const double RECENT_FRAME_OFFSET983 = 14.0;

// 004e31e0: fundo do resumo — figuras 5+6, 3+4 e 0+1+2 lado a lado nas posições 0, 1 e 2; depois os 8 medidores
void FrAchievement::CAhievementMainContents::DrawFrame983()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	const WRect& rect = m_pOwner->GetRect();
	float x = m_pOwner->m_pos[0][0] + rect.x;
	float y = m_pOwner->m_pos[0][1] + rect.y;
	const Bitmap* pBmp = m_pOwner->m_bitmaps[5];
	pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	float w = (float)pBmp->Width();
	pBmp = m_pOwner->m_bitmaps[6];
	pGI->DrawTexture(pBmp, WRect(w + x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);

	x = m_pOwner->m_pos[1][0] + rect.x;
	y = m_pOwner->m_pos[1][1] + rect.y;
	pBmp = m_pOwner->m_bitmaps[3];
	pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	w = (float)pBmp->Width();
	pBmp = m_pOwner->m_bitmaps[4];
	pGI->DrawTexture(pBmp, WRect(w + x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);

	x = m_pOwner->m_pos[2][0] + rect.x;
	y = m_pOwner->m_pos[2][1] + rect.y;
	pBmp = m_pOwner->m_bitmaps[0];
	pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	x = (float)pBmp->Width() + x;
	pBmp = m_pOwner->m_bitmaps[1];
	pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	w = (float)pBmp->Width();
	pBmp = m_pOwner->m_bitmaps[2];
	pGI->DrawTexture(pBmp, WRect(w + x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);

	for (int i = 0; i < CATEGORY_COUNT; i++)
		m_pOwner->m_categoryGauge[i].Draw();
}

// 004e3550: param NULL = cabeçalho da categoria (figuras 7+8+9, medidor total, pontos e título da categoria);
// senão a linha da conquista (FrListItem): figuras 10+11+12, nome da peça atual, contador, nome do QuestStuff, seta da lista
// de peças, figura da conquista com moldura (cumprida: pontos e data) e o medidor da linha
void FrAchievement::CAhievementDetailInfo::Draw(int param)
{
	if (!m_bShow)
		return;
	FrGraphicInterface* pGI = GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
	{
		const WRect& rect = m_pOwner->GetRect();
		float x = m_pOwner->m_pos[3][0] + rect.x;
		float y = m_pOwner->m_pos[3][1] + rect.y;
		const Bitmap* pBmp = m_pOwner->m_bitmaps[7];
		pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		x = (float)pBmp->Width() + x;
		pBmp = m_pOwner->m_bitmaps[8];
		pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		float w = (float)pBmp->Width();
		pBmp = m_pOwner->m_bitmaps[9];
		pGI->DrawTexture(pBmp, WRect(w + x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		m_pOwner->m_totalGauge.Draw();

		FrAchievement* pOwner = m_pOwner;
		int category = pOwner->m_category;
		if (category == -1)
			return;
		pGI->SetTextColor(0xffffffff, 0xffffffff);
		pGI->SetTextStyle(1);
		pGI->PrintText(WPoint(rect.x + pOwner->m_pos[13][0], pOwner->m_pos[13][1] + rect.y), 1,
			MakeStr("%d", m_pOwner->m_categoryPoints[category]), 0);
		pGI->SetTextStyle(0);
		pBmp = m_pOwner->m_challengeText[category];
		pGI->DrawTexture(pBmp, WRect(m_pOwner->m_pos[15][0] + rect.x, m_pOwner->m_pos[15][1] + rect.y, (float)pBmp->Width(),
			(float)pBmp->Height()), 0xffffffff, 0);
		return;
	}

	float itemX = pItem->pos.x;
	float itemY = pItem->pos.y;
	const Bitmap* pBmp = m_pOwner->m_bitmaps[10];
	float x = m_pOwner->m_pos[4][0] + itemX;
	pGI->DrawTexture(pBmp, WRect(x, itemY, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	x = (float)pBmp->Width() + x;
	pBmp = m_pOwner->m_bitmaps[11];
	pGI->DrawTexture(pBmp, WRect(x, itemY, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	float w = (float)pBmp->Width();
	pBmp = m_pOwner->m_bitmaps[12];
	pGI->DrawTexture(pBmp, WRect(w + x, itemY, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);

	sAchElement983* pElem = (sAchElement983*)pItem->pData;
	const IFF983::sAchievement* pIff = pElem->pIff;
	int counter = pElem->counter;
	if (counter > -1)
	{
		float nameX = m_pOwner->m_pos[5][0] + itemX;
		float nameY = m_pOwner->m_pos[5][1] + itemY;
		pGI->SetTextStyle(1);
		pGI->SetTextColor(0xff000000, 0xffffffff);
		pGI->PrintText(WPoint(nameX, nameY + 2.0), 0, AchievementView983(pIff)->names[counter], 0);
		pGI->SetTextColor(0xffffffff, 0xffffffff);
		pGI->PrintText(WPoint(itemX + COUNTER_OFFSET_X983, itemY + COUNTER_OFFSET_Y983), 1,
			pElem->pAch ? MakeStr("%d", pElem->pAch->GetCounterValue983(counter)) : "0", 0);
		pGI->SetTextStyle(0);
		pGI->SetTextColor(0xff000000, 0xffffffff);
		pGI->PrintText(WPoint(nameX, nameY + STUFF_NAME_OFFSET_Y983), 0, StuffView983(pElem->pStuff)->name, 0);
		pBmp = pElem->m_bOpen ? m_pOwner->m_bitmaps[24] : m_pOwner->m_bitmaps[23];
		pGI->DrawTexture(pBmp, WRect(m_pOwner->m_pos[6][0] + itemX - (float)pBmp->Width(),
			m_pOwner->m_pos[6][1] + itemY + PRIZE_ICON_OFFSET_Y983, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	}
	if (counter > 0)
	{
		if (pElem->pBitmap)
			pGI->DrawTexture(pElem->pBitmap, WRect(m_pOwner->m_pos[8][0] + itemX, m_pOwner->m_pos[8][1] + itemY,
				(float)pElem->pBitmap->Width(), (float)pElem->pBitmap->Height()), 0xffffffff, 0);
		pBmp = m_pOwner->m_bitmaps[22];
		pGI->DrawTexture(pBmp, WRect(m_pOwner->m_pos[9][0] + itemX, m_pOwner->m_pos[9][1] + itemY, (float)pBmp->Width(),
			(float)pBmp->Height()), 0xffffffff, 0);
		float px = m_pOwner->m_pos[6][0] + itemX;
		float py = m_pOwner->m_pos[6][1] + itemY;
		pBmp = m_pOwner->m_bitmaps[20];
		pGI->DrawTexture(pBmp, WRect(px, py, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		pGI->SetTextColor(0xff000000, 0xffffffff);
		m_pOwner->m_pUnitPointFont->Print(px + POINT_OFFSET_X983, py + POINT_OFFSET_Y983, MakeStr("%d", pElem->points),
			(eTITLEFONTALINE)1, POINT_SPACE983, 1.0f);
		m_pOwner->m_pDateFont->Print(px + DATE_OFFSET_X983, py + POINT_OFFSET_X983,
			MakeStr("%d.%d.%d", pElem->date.wYear, pElem->date.wMonth, pElem->date.wDay), (eTITLEFONTALINE)1, DATE_SPACE983, 1.0f);
	}
	else
	{
		if (pElem->pBitmap)
			pGI->DrawTexture(pElem->pBitmap, WRect(m_pOwner->m_pos[8][0] + itemX, m_pOwner->m_pos[8][1] + itemY,
				(float)pElem->pBitmap->Width(), (float)pElem->pBitmap->Height()), 0xffffffff, 0);
		pBmp = m_pOwner->m_bitmaps[21];
		pGI->DrawTexture(pBmp, WRect(m_pOwner->m_pos[9][0] + itemX, m_pOwner->m_pos[9][1] + itemY, (float)pBmp->Width(),
			(float)pBmp->Height()), 0xffffffff, 0);
	}
	int top = ((FrListBoxAchJp983*)m_pOwner->m_pElementList)->GetTopIndex983();
	CGaugeBar983& gauge = m_pOwner->m_gauge4b0[pItem->idx - top];
	gauge.Reset();
	gauge.SetNow983(pElem->progress);
	gauge.Draw();
}

// 004e3e10: peças da linha aberta — figuras 13+14+15 e, por peça, a figura do prêmio (cumpridas: 16/18 com a quantidade;
// as que faltam: 17/19), 18/19 quando há mais de uma quantidade
void FrAchievement::CAhievementPopupInfo::Draw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!m_bShow || !pItem || !pItem->pData)
		return;
	FrGraphicInterface* pGI = GetGDI();
	sAchElement983* pElem = (sAchElement983*)pItem->pData;
	if (!pElem->m_bOpen)
		return;
	float itemX = pItem->pos.x;
	float itemY = pItem->pos.y;
	const Bitmap* pBmp = m_pOwner->m_bitmaps[13];
	float x = m_pOwner->m_pos[14][0] + itemX;
	float y = m_pOwner->m_pos[14][1] + itemY;
	pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	x = (float)pBmp->Width() + x;
	pBmp = m_pOwner->m_bitmaps[14];
	pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	float w = (float)pBmp->Width();
	pBmp = m_pOwner->m_bitmaps[15];
	pGI->DrawTexture(pBmp, WRect(w + x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);

	float px = pElem->height + itemX + ITEM_LIST_OFFSET_X983;
	float py = itemY + ITEM_LIST_OFFSET_Y983;
	pGI->SetTextColor(0xffffffff, 0xffffffff);
	pGI->SetTextStyle(1);
	bool bDone = false;
	int n;
	if (pElem->lastCounter == pElem->prizes.size() - 1)
	{
		bDone = true;
		n = pElem->lastCounter + 1;
	}
	else
		n = pElem->counter;
	for (int i = 0; i < n; i++)
	{
		const sAchPrize983& prize = pElem->prizes[i];
		pBmp = !prize.bMulti ? m_pOwner->m_bitmaps[16] : m_pOwner->m_bitmaps[18];
		pGI->DrawTexture(pBmp, WRect(px, py, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		pGI->Print11(WPoint(px + 32.0, py + PRIZE_TEXT_OFFSET_Y983), 1, MakeStr("%d", prize.count[0]));
		px = (float)pBmp->Width() + 3.0 + px;
	}
	pGI->SetTextStyle(0);
	if (bDone)
		return;
	for (int i = pElem->counter; i < (int)pElem->prizes.size(); i++)
	{
		pBmp = !pElem->prizes[i].bMulti ? m_pOwner->m_bitmaps[17] : m_pOwner->m_bitmaps[19];
		pGI->DrawTexture(pBmp, WRect(px, py, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
		px = (float)pBmp->Width() + 3.0 + px;
	}
}

// 004e5230: fundo, pontos totais do jogador, as 6 conquistas recentes (figura + moldura m_bitmaps[22]) e os pontos por categoria
void FrAchievement::CAhievementMainContents::Draw(int param)
{
	if (!m_bShow)
		return;
	DrawFrame983();
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	const WRect& rect = m_pOwner->m_pSummaryArea->GetRect();
	m_pOwner->m_pTotalPointFont->Print(m_pOwner->m_pos[7][0] + rect.x, m_pOwner->m_pos[7][1] + rect.y,
		m_pOwner->m_totalPoint.c_str(), (eTITLEFONTALINE)2, 1.0f, 1.0f);
	float x = m_pOwner->m_pos[10][0] + rect.x;
	float y = m_pOwner->m_pos[10][1] + rect.y;
	for (int i = 0; i < RECENT_COUNT; i++)
	{
		const sAchRecent983& recent = m_pOwner->m_recent[i];
		if (recent.pAch)
		{
			if (const Bitmap* pBmp = recent.pBitmap)
			{
				pGI->DrawTexture(pBmp, WRect(x, y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
				if (const Bitmap* pFrame = m_pOwner->m_bitmaps[22])
					pGI->DrawTexture(pFrame, WRect(x - RECENT_FRAME_OFFSET983, y - RECENT_FRAME_OFFSET983,
						(float)pFrame->Width(), (float)pFrame->Height()), 0xffffffff, 0);
				x = (float)pBmp->Width() + 16.0 + x;
			}
		}
	}
	x = rect.x + m_pOwner->m_pos[12][0];
	y = m_pOwner->m_pos[12][1] + rect.y;
	pGI->SetTextColor(0xff000000, 0xffffffff);
	for (int i = 0; i < CATEGORY_COUNT; i++)
	{
		pGI->PrintText(WPoint(x, y), 1, MakeStr("%d", m_pOwner->m_categoryPoints[i]), 0);
		x = x + 76.0;
	}
}

// 004e6810: as linhas abertas que saíram da parte visível da lista fecham; as outras desenham as peças
void FrAchievement::OnElementLayerDraw983(int param)
{
	int top = ((FrListBoxAchJp983*)m_pElementList)->GetTopIndex983();
	if (top == -1)
		return;
	for (std::list<FrListItem*>::iterator it = m_openItems.begin(); it != m_openItems.end();)
	{
		FrListItem* pItem = *it;
		if (pItem->idx < top || pItem->idx >= top + 4)
		{
			CloseItem983(pItem);
			it = m_openItems.erase(it);
		}
		else
		{
			m_painters[2]->Draw((int)pItem);
			++it;
		}
	}
}

// 004e68b0
void FrAchievement::OnRecentListMove983()
{
	WVector mouse = g_input->GetMousePoint();
	WPoint pt(mouse.x, mouse.y);
	int row = FindRecentRow983(pt);
	if (row != -1 && row < RECENT_COUNT)
	{
		const sAchRecent983& recent = m_recent[row];
		const char* text = MakeStr("%s\n%d.%d.%d", AchievementView983(recent.pAch)->names[recent.nameIdx], recent.date.wYear,
			recent.date.wMonth, recent.date.wDay);
		m_pRecentList->SetToolTipText(std::string(text));
	}
}

// faixa de cada conquista recente na lista "list_ach_recent" (constantes próprias da unidade em 00d05020/00d05028)
static const double RECENT_ICON_WIDTH983 = 50.0;
static const double RECENT_ICON_STEP983 = 66.0;

// 004e6ed0: abre a lista de peças da linha clicada (fecha as outras) ou fecha a que já estava aberta
void FrAchievement::OnElementListUp983()
{
	FrListBox* pList = m_pElementList;
	FrListItem* pItem = pList->GetItemUnderCursor();
	sAchElement983* pElem = (sAchElement983*)pItem->pData;
	if (pElem->m_bOpen == 1)
	{
		CloseItem983(pItem);
		std::list<FrListItem*>::iterator it = std::find(m_openItems.begin(), m_openItems.end(), pItem);
		if (it != m_openItems.end())
			m_openItems.erase(it);
		return;
	}
	for (FrListBox::ITEM_LIST::iterator it = pList->m_itemList.begin(); it != pList->m_itemList.end(); ++it)
		((sAchElement983*)(*it)->pData)->m_bOpen = 0;
	pElem->m_bOpen = 1;
	m_openItems.clear();
	m_openItems.push_back(pItem);
	if (pElem->counter != -1)
	{
		m_pItemList->m_nFlags.Enable(FWF_TOOLTIPS);
		m_pItemList->ClearItem();
		m_pItemList->Enable(true); // 983 004e7040
		m_pItemList->SetToolTipText(std::string(""));
		m_itemTipColumn = -1;
		PlaceItemList983(pItem);
		m_pItemList->AddItem(pElem);
	}
}

// 004e69d0: dica da peça sob o cursor — nome, peça, data de quando cumpriu e o prêmio extra (item comum) se houver
void FrAchievement::OnItemListMove983()
{
	FrListItem* pItem = m_pItemList->GetItemUnderCursor();
	if (!pItem)
		return;
	WVector mouse = g_input->GetMousePoint();
	WPoint pt(mouse.x, mouse.y);
	int col = FindItemColumn983((float)m_bitmaps[16]->Width(), pt);
	if (col == -1 || m_itemTipColumn == col)
		return;
	sAchElement983* pElem = (sAchElement983*)pItem->pData;
	if ((unsigned)col >= pElem->prizes.size())
		return;
	std::vector<IFF983::sQuestStuff*>* pStuffs = m_pDoc->FindStuffs983(AchievementView983(pElem->pIff)->TypeId);
	if (!pStuffs)
		return;
	m_itemTipColumn = col;
	const IFF983::sQuestStuff* pStuff = pStuffs->at(col);
	bool bLast = false;
	if (pElem->lastCounter == (int)pElem->prizes.size() - 1)
		bLast = true;
	int done = bLast ? pElem->lastCounter + 1 : pElem->counter;
	auto name = [pElem](int i) -> const char*
	{
		if (!AchievementView983(pElem->pIff)->stuffIds[i])
			return NULL;
		return AchievementView983(pElem->pIff)->names[i];
	};
	if (col < done)
	{
		SYSTEMTIME date;
		if (pElem->pAch)
			S5::SecToSysTime983(&date, pElem->pAch->GetCounterTime983(col));
		else
			memset(&date, 0, sizeof(date));
		if (!pElem->prizes[col].count[1])
			m_pItemList->SetToolTipText(MakeStr("%s\n%s\n%d.%d.%d", name(col), StuffView983(pStuff)->name, date.wYear, date.wMonth,
				date.wDay));
		else
		{
			const IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(StuffView983(pStuff)->prizeIds[1]);
			if (!pCommon)
				m_pItemList->SetToolTipText(MakeStr("%s\n%s\n%d.%d.%d", name(col), StuffView983(pStuff)->name, date.wYear,
					date.wMonth, date.wDay));
			else
				m_pItemList->SetToolTipText(MakeStr("%s\n%s\n%d.%d.%d\n%s", name(col), StuffView983(pStuff)->name, date.wYear,
					date.wMonth, date.wDay, pCommon->Name));
		}
		m_pItemList->SetToolTipFrameStyle(0);
	}
	else
	{
		if (!pElem->prizes[col].count[1])
			m_pItemList->SetToolTipText(MakeStr("%s\n%s\n", name(col), StuffView983(pStuff)->name));
		else
			m_pItemList->SetToolTipText(MakeStr("%s\n%s\n%s", name(col), StuffView983(pStuff)->name,
				K2L_Compatibility("\xc3\xdf\xb0\xa1\xba\xb8\xbb\xf3 \xc0\xd6\xc0\xbd")));
		m_pItemList->SetToolTipFrameStyle(1);
	}
}

// 004e7bb0: as conquistas com contador cumprido, as 6 mais recentes (partial_sort só com mais de 6), e as faixas da lista
void FrAchievement::BuildRecent983()
{
	typedef std::pair<_achievement::CAchievement*, int> Recent;
	std::vector<Recent> recents;
	for (CQuestDocument::AchievementMap::iterator it = m_pDoc->m_achievements.begin(), end = m_pDoc->m_achievements.end(); it != end; ++it)
	{
		int latest = it->second.GetLatestCounter983();
		if (latest != -1)
			recents.push_back(Recent(&it->second, latest));
	}
	int count = (int)recents.size();
	if (count > RECENT_COUNT)
	{
		std::partial_sort(recents.begin(), recents.begin() + RECENT_COUNT, recents.end(), [](const Recent& a, const Recent& b)
			{ return a.first->GetCounterTime983(a.second) > b.first->GetCounterTime983(b.second); });
		count = RECENT_COUNT;
	}
	for (int i = 0; i < count; i++)
	{
		_achievement::CAchievement* pAch = recents[i].first;
		IFF983::sAchievement* pIff = ItemManager()->FindAchievement983(pAch->GetTypeId983());
		if (!pIff)
			continue;
		unsigned long id = pAch->GetCounterId2_983(recents[i].second);
		for (unsigned j = 0; j < 10; j++)
		{
			if (AchievementView983(pIff)->stuffIds[j] == id)
			{
				m_recent[i].pAch = pIff;
				m_recent[i].nameIdx = j;
				m_recent[i].pBitmap = g_pFresh->GetBitmap(AchievementView983(pIff)->bitmap);
				S5::SecToSysTime983(&m_recent[i].date, recents[i].first->GetCounterTime983(recents[i].second));
			}
		}
	}

	float left = m_pos[11][0] + m_rect.x;
	float top = m_pos[11][1] + m_rect.y;
	float height = m_pRecentList->GetRect().h;
	int n = 0;
	for (int i = 0; i < RECENT_COUNT; i++)
	{
		if (m_recent[i].pAch)
			n++;
	}
	float x = left;
	for (int i = 0; i < n; i++)
	{
		m_recentRows[i][0] = x;
		m_recentRows[i][1] = (float)(x + RECENT_ICON_WIDTH983);
		x = (float)(x + RECENT_ICON_STEP983);
	}
	float width = (float)(RECENT_ICON_STEP983 * n);
	m_recentCount = n;
	m_pRecentList->SetRect(WRect(left, top, width, height));
}

// 004e8100: as conquistas da categoria (Achievement.iff ordenado pela categoria no documento) em m_iffs
void FrAchievement::LoadElements983(int category)
{
	CQuestDocument* pDoc = m_pDoc;
	sAchievementView983 key;
	key.category = category;
	std::pair<std::vector<IFF983::sAchievement*>::iterator, std::vector<IFF983::sAchievement*>::iterator> range =
		std::equal_range(pDoc->m_achievementIffs.begin(), pDoc->m_achievementIffs.end(), (IFF983::sAchievement*)&key,
			[](const IFF983::sAchievement* a, const IFF983::sAchievement* b) { return AchievementView983(a)->category < AchievementView983(b)->category; });
	m_iffs.clear();
	if (range.first != range.second)
		m_iffs.assign(range.first, range.second);
}

// 004e8c20: m_elements montado de m_iffs — conquista do jogador, peça atual, pontos, progresso, data e prêmios
void FrAchievement::SortElements983()
{
	CQuestDocument* pDoc = m_pDoc;
	m_elements.clear();
	for (std::vector<IFF983::sAchievement*>::iterator it = m_iffs.begin(); it != m_iffs.end(); ++it)
	{
		const IFF983::sAchievement* pIff = *it;
		unsigned long typeId = AchievementView983(pIff)->TypeId;
		CQuestDocument::AchievementInfoMap::iterator info = pDoc->m_achievementInfos.find(typeId);
		if (info == pDoc->m_achievementInfos.end())
			continue;
		if (m_elements.find(typeId) != m_elements.end())
			continue;
		int stuffCount = (int)info->second.stuffs.size();
		_achievement::CAchievement* pAch = NULL;
		CQuestDocument::AchievementMap::iterator found = pDoc->m_achievements.find(typeId);
		if (found != pDoc->m_achievements.end())
			pAch = &found->second;

		sAchElement983 elem;
		elem.lastCounter = -1;
		if (pAch)
			elem.lastCounter = pAch->GetLastCounter983();
		elem.counter = -1;
		if (stuffCount - 1 == elem.lastCounter)
			elem.counter = elem.lastCounter;
		else if (pAch)
		{
			elem.counter = pAch->GetOpenCounter983();
			if (elem.counter == -1)
				continue;
			if (elem.counter <= elem.lastCounter)
				elem.counter = elem.lastCounter;
		}
		unsigned long stuffId = pAch ? pAch->GetCounterId983(elem.counter) : 0;

		std::vector<sAchPrize983> prizes;
		for (int i = 0; i < stuffCount; i++)
		{
			const IFF983::sQuestStuff* pStuff = info->second.stuffs[i];
			sAchPrize983 prize;
			int n = 0;
			for (int j = 0; j < 3; j++)
			{
				prize.count[j] = StuffView983(pStuff)->prizeCount[j];
				if (prize.count[j])
					n++;
			}
			prize.bMulti = n > 1;
			prizes.push_back(prize);
		}
		elem.prizes.swap(prizes);
		elem.pAch = pAch;
		elem.pIff = pIff;
		elem.m_bOpen = 0;
		elem.points = pDoc->GetPoint983(pAch);
		elem.maxPoints = info->second.points;
		if (stuffId)
		{
			for (std::vector<IFF983::sQuestStuff*>::iterator s = info->second.stuffs.begin(), end = info->second.stuffs.end(); s != end; ++s)
			{
				if ((*s)->TypeId == stuffId)
				{
					elem.pStuff = *s;
					break;
				}
			}
		}
		else
			elem.pStuff = info->second.stuffs[0];
		if (pAch)
		{
			int latest = pAch->GetLatestCounter983();
			if (latest != -1)
				S5::SecToSysTime983(&elem.date, pAch->GetCounterTime983(latest));
			float value = (float)pAch->GetCounterValue983(elem.counter);
			elem.progress = (float)(value / StuffView983(elem.pStuff)->goal * 100.0);
			if (elem.progress < 0.0)
				elem.progress = 0.0f;
			else if (elem.progress > 100.0)
				elem.progress = 100.0f;
		}
		else
			elem.progress = 0.0f;
		elem.height = CalcLayerHeight983((int)elem.prizes.size());
		elem.pBitmap = g_pFresh->GetBitmap(AchievementView983(elem.pIff)->bitmap);
		m_elements.insert(std::make_pair(AchievementView983(pIff)->TypeId, elem));
	}
}

// 004e92d0
void FrAchievement::SelectTab983(int tab)
{
	sAchBusy983 busy;
	for (std::list<FrListItem*>::iterator it = m_openItems.begin(); it != m_openItems.end(); ++it)
		CloseItem983(*it);
	m_pElementList->ClearItem();
	m_openItems.clear();

	for (int i = 0; i < TAB_COUNT; i++)
		m_pTabBtn[i]->SetStatus(FrButton::NORMAL);
	m_pTabBtn[tab]->SetStatus(FrButton::PRESSED);

	int category = -1;
	switch (tab)
	{
	case 1: category = 1; break;
	case 2: category = 0; break;
	case 3: category = 3; break;
	case 4: category = 2; break;
	case 5: category = 4; break;
	case 6: category = 6; break;
	case 7: category = 5; break;
	case 8: category = 7; break;
	}
	if (tab == 0)
	{
		m_pRecentList->m_nFlags.Enable(FWF_TOOLTIPS);
		m_pRecentList->SetVisible(true);
	}
	else
	{
		m_pRecentList->m_nFlags.Disable(FWF_TOOLTIPS);
		m_pRecentList->SetVisible(false);
	}
	SetCategory983(category);
	if (category == -1)
		return;

	LoadElements983(category);
	SortElements983();
	for (ElementMap::iterator it = m_elements.begin(); it != m_elements.end(); ++it)
		m_pElementList->AddItem(&it->second);
	m_pElementList->GetScrollBar()->ScrollToFirst();

	int max = m_pDoc->m_categoryPoints[category];
	float rate;
	if (max)
	{
		rate = (float)m_categoryPoints[category] / max;
		rate = (float)(rate * 100.0);
		rate = (float)floor(rate);
	}
	else
		rate = 0.0f;
	m_totalGauge.SetNow983(rate);
}

// 004e96d0
FrAchievement::~FrAchievement()
{
	if (m_pDateFont)
	{
		delete m_pDateFont;
		m_pDateFont = NULL;
	}
	if (m_pUnitPointFont)
	{
		delete m_pUnitPointFont;
		m_pUnitPointFont = NULL;
	}
	if (m_pTotalPointFont)
	{
		delete m_pTotalPointFont;
		m_pTotalPointFont = NULL;
	}
	for (std::vector<CAhievementPainter*>::iterator it = m_painters.begin(); it != m_painters.end(); ++it)
		delete *it;
	AfxPostMsg(NULL, "AvatarChat", 0x58, 0, 0, 0, 0);
}


// 004e9960: pintores, fontes, tabelas de achievement.lua (figuras, posições, medidores), resumo e a aba 0
bool FrAchievement::OnInit()
{
	m_painters.push_back(new CAhievementMainContents(this));
	m_painters.push_back(new CAhievementDetailInfo(this));
	m_painters.push_back(new CAhievementPopupInfo(this));
	m_pTotalPointFont = new CAchieveTotalPointFont;
	m_pUnitPointFont = new CAchieveUnitEarnPoint;
	m_pDateFont = new CAchievementDate;

	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("achievement.lua");
	{
		lua_system::lua_table_helper tbl = script->GetTable("ach_bitmap_list");
		int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
		for (int i = 1; i <= count; i++)
		{
			const char* bmpName = ((LuaChild983&)tbl).GetInl983<const char*>(i);
			m_bitmaps[i - 1] = g_pFresh->GetBitmap(bmpName);
		}
	}
	for (int i = 0; i < CATEGORY_COUNT; i++)
		m_challengeText[i] = g_pFresh->GetBitmap(MakeStr("Challenge_text_%d", i));
	{
		lua_system::lua_table_helper tbl = script->GetTable("ach_position");
		int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
		float* pos = &m_pos[0][0];
		for (int i = 1; i <= count; i += 2)
		{
			float x = ((LuaChild983&)tbl).GetInl983<float>(i);
			float y = ((LuaChild983&)tbl).GetInl983<float>(i + 1);
			pos[0] = x;
			pos[1] = y;
			pos += 2;
		}
	}
	{
	lua_system::lua_table_helper tbl = script->GetTable("ach_ui_etc");
	float x = ((LuaChild983&)tbl).GetInl983<float>(1);
	float y = ((LuaChild983&)tbl).GetInl983<float>(2);
	float step = ((LuaChild983&)tbl).GetInl983<float>(3);
	for (int i = 0; i < CATEGORY_COUNT; i++)
	{
		m_categoryGauge[i].Init("challenges_guage1_mask.tga", "challenges_guage1.tga",
			WRect(i * step + (x + GetRect().x), y + GetRect().y, 0.0f, 100.0f), 0, true, 1.0f, true, true);
		m_categoryGauge[i].m_target = 0.0f;
		m_categoryGauge[i].m_b20 = false;
	}
	x = ((LuaChild983&)tbl).GetInl983<float>(4);
	y = ((LuaChild983&)tbl).GetInl983<float>(5);
	step = ((LuaChild983&)tbl).GetInl983<float>(6);
	for (int i = 0; i < 4; i++)
	{
		m_gauge4b0[i].Init("challenges_guage3_mask.tga", "challenges_guage3.tga",
			WRect(x + GetRect().x, i * step + (y + GetRect().y), 0.0f, 100.0f), 0, true, 1.0f, true, true);
		m_gauge4b0[i].m_target = 0.0f;
		m_gauge4b0[i].m_b20 = false;
	}
	x = ((LuaChild983&)tbl).GetInl983<float>(7);
	y = ((LuaChild983&)tbl).GetInl983<float>(8);
	m_totalGauge.Init("challenges_guage2_mask.tga", "challenges_guage2.tga", WRect(x + GetRect().x, y + GetRect().y, 0.0f, 100.0f),
		0, true, 1.0f, true, true);
	}
	lua_system::lua_script_manger::GetInstance()->DeleteScript(script);

	BuildRecent983();
	UpdateCategoryGauges983();
	m_totalPoint = MakeStr("%d", m_pDoc->GetAchievementPoint983());
	SelectTab983(0);
	return FrForm::OnInit();
}

// 004ea420
FrAchievement::FrAchievement()
{
	m_pTabBtn[0] = NULL;
	m_pTabBtn[1] = NULL;
	m_pTabBtn[2] = NULL;
	m_pTabBtn[3] = NULL;
	m_pTabBtn[4] = NULL;
	m_pTabBtn[5] = NULL;
	m_pTabBtn[6] = NULL;
	m_pTabBtn[7] = NULL;
	m_pTabBtn[8] = NULL;
	m_pElementList = NULL;
	m_pItemList = NULL;
	m_pRecentList = NULL;
	memset(m_bitmaps, 0, sizeof(m_bitmaps));
	m_challengeText[0] = NULL;
	m_challengeText[1] = NULL;
	m_challengeText[2] = NULL;
	m_challengeText[3] = NULL;
	m_challengeText[4] = NULL;
	m_challengeText[5] = NULL;
	m_challengeText[6] = NULL;
	m_challengeText[7] = NULL;
	memset(m_pos, 0, sizeof(m_pos));
	memset(m_recent, 0, sizeof(m_recent));
	memset(m_recentRows, 0, sizeof(m_recentRows));
	m_categoryPoints[0] = 0;
	m_categoryPoints[1] = 0;
	m_categoryPoints[2] = 0;
	m_categoryPoints[3] = 0;
	m_categoryPoints[4] = 0;
	m_categoryPoints[5] = 0;
	m_categoryPoints[6] = 0;
	m_categoryPoints[7] = 0;
	m_unused748983 = 0;
	m_itemTipColumn = 0;
	m_recentCount = 0;
	m_pTotalPointFont = NULL;
	m_pUnitPointFont = NULL;
	m_pDateFont = NULL;
	m_pDoc = NULL;
}

IMPLEMENT_OBJECT(FrAchievement, FrForm)
}
}

// 00b307f0 (sharedutilities.cpp:318): segundos (32 bits) -> FILETIME (+11644473600 s, em 100 ns) -> hora local -> SYSTEMTIME
void S5::SecToSysTime983(SYSTEMTIME* pOut, __int64 sec)
{
	FILETIME ft, local;
	__int64 v = ((__int64)(int)sec + 11644473600LL) * 10000000; // 00b30780
	ft.dwLowDateTime = (DWORD)v;
	ft.dwHighDateTime = (DWORD)(v >> 32);
	FileTimeToLocalFileTime(&ft, &local);
	FileTimeToSystemTime(&local, pOut);
}
