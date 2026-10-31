// JP 983: unidade 004a39f0–004a55a0 (FrClubSetWorkshopSelector: escolha do taco da oficina, "selectclubset"; e os textos K2L
// da oficina, funções livres). Classe do 839/983, sem par no 645; ver include/jp/clubsetworkshopselector.h. Sessão B90 (F3.7.9,
// 15º lote). O compilador gera (e casam) os moldes da unidade: vector<sItem983> (push_back 004a4dc0, _Reserve 004a4b70,
// reserve 004a4a60, _Umove 004a40e0…), vector<unsigned long> (os do m_marked são os da unidade da FrClubSetResetWnd) e o
// MakeInstance.
#include "minatl.h"
#include "vc10_layout.h"
#include <algorithm>
#include "projectg.h"
#include "bitmap.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frlistbox.h"
#include "frbutton.h"
#include "frstatic.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "titlefontex.h"
#include "shareddoc.h"
#include "../../shared/itemmanager.h"
#include "lua983.h"
#include "clubsetstat.h"
#include "clubsetinfofont.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopselector.h"

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);

extern Fresh* g_pFresh; // 00f02340

IMPLEMENT_OBJECT(FrClubSetWorkshopSelector, FrForm)

// 004a39f0 (GetMessageMap); mapa 00e04d30
BEGIN_FRESH_MSGMAP(FrClubSetWorkshopSelector, FrForm)

ON_FRESH_VI("lt_item", FRCMD_INIT, FrClubSetWorkshopSelector::OnItemInit)
ON_FRESH_VI("lt_item", FRCMD_OWNERDRAW, FrClubSetWorkshopSelector::OnItemDraw)
ON_FRESH_VV("lt_item", FRCMD_LBUTTONUP, FrClubSetWorkshopSelector::OnItemClick)
ON_FRESH_VV("lt_item", FRCMD_DBLCLICK, FrClubSetWorkshopSelector::OnItemDblClick)
ON_FRESH_VI("lt_item", FRCMD_HOVERON, FrClubSetWorkshopSelector::OnItemHoverOn)
ON_FRESH_VI("lt_item", FRCMD_HOVEROFF, FrClubSetWorkshopSelector::OnItemHoverOff)
ON_FRESH_VI("bt_ok", FRCMD_INIT, FrClubSetWorkshopSelector::OnOkInit)
ON_FRESH_VV("bt_ok", FRCMD_LBUTTONUP, FrClubSetWorkshopSelector::OnOkClick)
ON_FRESH_VV("bt_cancel", FRCMD_LBUTTONUP, FrClubSetWorkshopSelector::OnCancelClick)
ON_FRESH_VI("static_notice", FRCMD_INIT, FrClubSetWorkshopSelector::OnNoticeInit)

END_FRESH_MSGMAP()

// 004a3c80
bool FrClubSetWorkshopSelector::IsMarked983(unsigned long guid)
{
	for (unsigned int i = 0; i < m_marked.size(); ++i)
	{
		if (m_marked[i] == guid)
			return true;
	}
	return false;
}

// 004a3cc0
void FrClubSetWorkshopSelector::OnItemInit(int param)
{
	m_pList = KINDOF_CAST983(FrListBox, param);
}

// 004a3d30: escolhe o taco sob o cursor (não os já escolhidos) e libera o "bt_ok"
void FrClubSetWorkshopSelector::OnItemClick()
{
	FrListItem* pItem = m_pList->GetItemUnderCursor();
	if (pItem)
	{
		sItem983* pData = (sItem983*)pItem->pData;
		if (pData && !IsMarked983(pData->pItem->guid))
		{
			m_selTid = pData->pItem->tid;
			m_selGuid = pData->pItem->guid;
			if (m_pOk)
				m_pOk->Enable(true);
		}
	}
}

// 004a3d90: escolhe e fecha com OK
void FrClubSetWorkshopSelector::OnItemDblClick()
{
	FrListItem* pItem = m_pList->GetItemUnderCursor();
	if (pItem)
	{
		sItem983* pData = (sItem983*)pItem->pData;
		if (pData && !IsMarked983(pData->pItem->guid))
		{
			m_selTid = pData->pItem->tid;
			m_selGuid = pData->pItem->guid;
			OnOkClick();
		}
	}
}

// 004a3de0
void FrClubSetWorkshopSelector::OnOkInit(int param)
{
	m_pOk = KINDOF_CAST983(FrButton, param);
	m_pOk->Enable(false);
}

// 004a3a40
void FrClubSetWorkshopSelector::OnOkClick()
{
	Close(FrOK, true);
}

// 004a3a50
void FrClubSetWorkshopSelector::OnCancelClick()
{
	Close(FrCANCEL, true);
}

// 004a3a20
void FrClubSetWorkshopSelector::OnItemHoverOn(int param)
{
}

// 004a3a30
void FrClubSetWorkshopSelector::OnItemHoverOff(int param)
{
}

// 004a3e50
void FrClubSetWorkshopSelector::OnNoticeInit(int param)
{
	m_pNotice = KINDOF_CAST983(FrStatic, param);
}

// 004a4120
void FrClubSetWorkshopSelector::LoadLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	bool ran = false;
	if (script)
		ran = script->RunFile("clubset_workshop.lua");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_selector");
	// D404: rastro da queda intermitente "index a number value" (d399-1): o script rodou? a tabela veio? onde esta na pilha?
	jp983_rastro("D404 selector kind=%d script=%p ran=%d ptr=%p idx=%d top=%d", (int)m_kind, script, (int)ran,
		tbl.table.m_obj ? tbl.table.m_obj->m_pointer : 0, tbl.table.m_obj ? tbl.table.m_obj->m_index : -1, lua_gettop(tbl.state));
	m_selectedImg = LUA983_GET(tbl, const char*, "selected_img_filename");
	m_hoverImg = LUA983_GET(tbl, const char*, "hover_img_filename");
	if (m_pNotice)
	{
		float x = LUA983_POS(tbl, "text_notice_pos", 1);
		float y = LUA983_POS(tbl, "text_notice_pos", 2);
		x += m_rect.x;
		y += m_rect.y;
		m_pNotice->SetRect(WRect(x, y, m_pNotice->GetRect().w, m_pNotice->GetRect().h));
	}
	m_valuePos[0] = 0;
	m_valuePos[1] = 0;
	switch (m_kind)
	{
	case KIND_LEVELUP:
	case KIND_RANKUP:
	case KIND_RESET:
		m_valuePos[0] = LUA983_POS(tbl, "level_img_pos", 1);
		m_valuePos[1] = LUA983_POS(tbl, "level_img_pos", 2);
		m_rankPos[0] = LUA983_POS(tbl, "rank_img_pos", 1);
		m_rankPos[1] = LUA983_POS(tbl, "rank_img_pos", 2);
		break;
	case KIND_EXP_FROM:
	case KIND_EXP_TO:
		m_valuePos[0] = LUA983_POS(tbl, "exp_imgfont_pos", 1);
		m_valuePos[1] = LUA983_POS(tbl, "exp_imgfont_pos", 2);
		break;
	case KIND_RECOVERY:
		m_valuePos[0] = LUA983_POS(tbl, "recovery_imgfont_pos", 1);
		m_valuePos[1] = LUA983_POS(tbl, "recovery_imgfont_pos", 2);
		break;
	}
	m_hoverPos[0] = LUA983_POS(tbl, "hover_img_pos", 1);
	m_hoverPos[1] = LUA983_POS(tbl, "hover_img_pos", 2);
	m_selectedPos[0] = LUA983_POS(tbl, "selected_img_pos", 1);
	m_selectedPos[1] = LUA983_POS(tbl, "selected_img_pos", 2);
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// 004a4540: ícone do taco; marca dos já escolhidos; realce sob o cursor (com o nome na dica) e do escolhido; nível e rank,
// experiência ou cancelamentos restantes, conforme o uso
void FrClubSetWorkshopSelector::OnItemDraw(int param)
{
	if (!param)
		return;
	FrListItem* pItem = (FrListItem*)param;
	sItem983* pData = (sItem983*)pItem->pData;
	const Bitmap* pBmp = g_pFresh->GetBitmap(pData->pClub->c.Icon);
	if (pBmp)
		GDI()->DrawTexture(pBmp, WRect(pItem->pos.x, pItem->pos.y, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	bool bMarked = IsMarked983(pData->pItem->guid);
	if (bMarked && (pBmp = g_pFresh->GetBitmap(m_selectedImg)) != NULL)
		GDI()->DrawTexture(pBmp, WRect(pItem->pos.x + m_selectedPos[0], pItem->pos.y + m_selectedPos[1], (float)pBmp->Width(),
			(float)pBmp->Height()), 0xffffffff, 0);
	FrListItem* pCur = m_pList->GetItemUnderCursor();
	if (pCur)
	{
		sItem983* pCurData = (sItem983*)pCur->pData;
		if (pCurData && pCurData->pItem->guid == pData->pItem->guid)
		{
			if (!bMarked && (pBmp = g_pFresh->GetBitmap(m_hoverImg)) != NULL)
				GDI()->DrawTexture(pBmp, WRect(pItem->pos.x + m_hoverPos[0], pItem->pos.y + m_hoverPos[1], (float)pBmp->Width(),
					(float)pBmp->Height()), 0xffffffff, 0);
			m_pList->SetToolTipText(pCurData->pClub->c.Name);
		}
	}
	else
		m_pList->SetToolTipText("");
	if (m_selGuid == pData->pItem->guid && (pBmp = g_pFresh->GetBitmap(m_hoverImg)) != NULL)
		GDI()->DrawTexture(pBmp, WRect(pItem->pos.x + m_hoverPos[0], pItem->pos.y + m_hoverPos[1], (float)pBmp->Width(),
			(float)pBmp->Height()), 0xffffffff, 0);
	switch (m_kind)
	{
	case KIND_LEVELUP:
	case KIND_RANKUP:
	case KIND_RESET:
	{
		int grade = GetMyClubGrade983(pData->pItem->guid);
		int step = GetMyClubStep983(pData->pItem->guid);
		if (grade != 5)
			m_pInfoFont->PrintLevel983(step, pItem->pos.x + m_valuePos[0], pItem->pos.y + m_valuePos[1], TFA_RIGHT, 0, 1.0f);
		m_pInfoFont->PrintRank983(grade, pItem->pos.x + m_rankPos[0], pItem->pos.y + m_rankPos[1], TFA_RIGHT, 0, 1.0f);
		break;
	}
	case KIND_EXP_FROM:
	case KIND_EXP_TO:
		m_pNumFont->Print(pItem->pos.x + m_valuePos[0], pItem->pos.y + m_valuePos[1], MakeStr("%d", pData->pItem->clubExp983),
			TFA_RIGHT, 0, 1.0f);
		break;
	case KIND_RECOVERY:
		m_pWindFont->Print(pItem->pos.x + m_valuePos[0], pItem->pos.y + m_valuePos[1],
			MakeStr("%d/%d", pData->pClub->nRecoveryMax983 - pData->pItem->recoveryUsed983, pData->pClub->nRecoveryMax983), TFA_RIGHT, 0, 1.0f);
		break;
	}
}

// 004a4bd0
FrClubSetWorkshopSelector::FrClubSetWorkshopSelector()
	: m_items()
	, m_pList(NULL)
	, m_kind(0)
	, m_selTid(0)
	, m_selGuid(0)
	, m_marked()
{
	// deslocamentos lidos no 983: OnInit 004a4ef0 (+0x110, +0x158), OnItemDraw 004a4540, chamadores 004c1f20/004c2060 (+0x158, +0x164)
	C_ASSERT_VC10(offsetof(FrClubSetWorkshopSelector, m_items) == 0x110 && offsetof(FrClubSetWorkshopSelector, m_pList) == 0x120);
	C_ASSERT_VC10(offsetof(FrClubSetWorkshopSelector, m_pInfoFont) == 0x134 && offsetof(FrClubSetWorkshopSelector, m_selectedPos) == 0x150);
	C_ASSERT_VC10(offsetof(FrClubSetWorkshopSelector, m_kind) == 0x158 && offsetof(FrClubSetWorkshopSelector, m_selGuid) == 0x160);
	C_ASSERT_VC10(offsetof(FrClubSetWorkshopSelector, m_marked) == 0x164 && offsetof(FrClubSetWorkshopSelector, m_selectedImg) == 0x178);
	m_rankPos[0] = 0;
	m_pad983_174 = 0;
	m_rankPos[1] = 0;
	m_pWindFont = NULL;
	m_pNumFont = NULL;
	m_pInfoFont = NULL;
	m_selectedImg = NULL;
	m_hoverImg = NULL;
}

// 004a4c90
FrClubSetWorkshopSelector::~FrClubSetWorkshopSelector()
{
	if (m_pWindFont)
	{
		delete m_pWindFont;
		m_pWindFont = NULL;
	}
	if (m_pNumFont)
	{
		delete m_pNumFont;
		m_pNumFont = NULL;
	}
	if (m_pInfoFont)
	{
		delete m_pInfoFont;
		m_pInfoFont = NULL;
	}
}

// 004a4ef0: lista os tacos do jogador que servem para o uso, as fontes, o aviso e o layout
bool FrClubSetWorkshopSelector::OnInit()
{
	std::map<unsigned int, sItemInfo>& clubs = Doc()->m_clubSetMap;
	for (std::map<unsigned int, sItemInfo>::iterator it = clubs.begin(); it != clubs.end(); ++it)
	{
		IFF_STRUCT::sClubSet* pIff = FindClubSetIff983(it->second.tid);
		if (!pIff)
			continue;
		if (m_kind != KIND_EXP_FROM)
		{
			bool ok;
			if (m_kind == KIND_EXP_TO)
			{
				if (pIff->nWorkshopType983 == -1)
					continue;
				ok = GetMyClubGrade983(it->second.guid) != 5;
			}
			else if (m_kind == KIND_RESET)
				ok = IsClubStepChanged983(it->second.guid) || IsClubGradeChanged983(it->second.guid);
			else
				ok = pIff->nWorkshopType983 != -1;
			if (!ok)
				continue;
		}
		sItem983 item = { &it->second, ItemManager()->FindClubSet(it->second.tid) };
		m_items.push_back(item);
	}
	FrListBox* pList = m_pList;
	for (sItem983* p = m_items.begin()._Ptr, *end = m_items.end()._Ptr; p != end; ++p)
		pList->AddItem(p);
	m_pWindFont = new CWindFont;
	m_pNumFont = new CSmallNumFont;
	m_pInfoFont = new CClubSetInfoFont;
	if (m_pNotice)
	{
		const char* text;
		switch (m_kind)
		{
		case KIND_LEVELUP: // "개조에 사용할 클럽세트를 선택합니다."
			text = "\260\263\301\266\277\241 \273\347\277\353\307\322 \305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\325\264\317\264\331.";
			break;
		case KIND_RANKUP: // "랭크업에 …"
			text = "\267\251\305\251\276\367\277\241 \273\347\277\353\307\322 \305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\325\264\317\264\331.";
			break;
		case KIND_EXP_FROM:
		case KIND_EXP_TO: // "이전에 …"
			text = "\300\314\300\374\277\241 \273\347\277\353\307\322 \305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\325\264\317\264\331.";
			break;
		case KIND_RECOVERY: // "회복에 …"
			text = "\310\270\272\271\277\241 \273\347\277\353\307\322 \305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\325\264\317\264\331.";
			break;
		case KIND_RESET: // "초기화에 …"
			text = "\303\312\261\342\310\255\277\241 \273\347\277\353\307\322 \305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\325\264\317\264\331.";
			break;
		default:
			goto layout;
		}
		m_pNotice->SetCaption(K2L_Compatibility(text));
	}
layout:
	LoadLayout983();
	return FrForm::OnInit();
}

// ---------------------------------------------------------------------------------------------------------------------------
// textos

// 004a5240
const char* __fastcall GetUpgradeStatText983(int count)
{
	switch (count)
	{
	case 0: return K2L_Compatibility("\271\253\300\333\300\247"); // 무작위
	case 1: return K2L_Compatibility("\301\244\310\256\265\265"); // 정확도
	case 2: return K2L_Compatibility("\304\277\272\352"); // 커브
	case 3: return K2L_Compatibility("\306\304\277\366"); // 파워
	case 4: return K2L_Compatibility("\275\272\307\311"); // 스핀
	case 5: return K2L_Compatibility("\304\301\306\256\267\321"); // 컨트롤
	}
	return "INVALID";
}

// 004a52b0
const char* __fastcall GetAddSlotStatText983(int index)
{
	switch (index)
	{
	case 0: return K2L_Compatibility("\301\244\310\256\265\265");
	case 1: return K2L_Compatibility("\304\277\272\352");
	case 2: return K2L_Compatibility("\306\304\277\366");
	case 3: return K2L_Compatibility("\275\272\307\311");
	case 4: return K2L_Compatibility("\304\301\306\256\267\321");
	}
	return "INVALID";
}

// 004a5310
const char* __fastcall ClubSetTypeName983(int type)
{
	switch (type)
	{
	case 0: return K2L_Compatibility("\271\353\267\261\275\272"); // 밸런스
	case 1: return K2L_Compatibility("\306\304\277\366"); // 파워
	case 2: return K2L_Compatibility("\304\301\306\256\267\321"); // 컨트롤
	case 3: return K2L_Compatibility("\275\272\307\311"); // 스핀
	case 4: return K2L_Compatibility("\275\272\306\344\274\310"); // 스페셜
	}
	return "INVALID";
}

// 004a5370
const char* __fastcall GetLevelUpNotice983(int r)
{
	switch (r)
	{
	case 0: // "'깨진 에보트 조각'카드 사용 매수에 따라 슬롯 추가 확률이 늘어납니다."
		return K2L_Compatibility("'\261\372\301\370 \277\241\272\270\306\256 \301\266\260\242'\304\253\265\345 \273\347\277\353 \270\305\274\366"
			"\277\241 \265\373\266\363 \275\275\267\324 \303\337\260\241 \310\256\267\374\300\314 \264\303\276\356\263\263\264\317\264\331.");
	case 1:
		return K2L_Compatibility("\275\275\267\324 \303\337\260\241\277\241 \273\347\277\353\307\321 \276\306\300\314\305\333\300\272 \260\341"
			"\301\244 \277\251\272\316\277\315 \260\374\260\350\276\370\300\314 \274\322\270\360\265\313\264\317\264\331.");
	case 2:
		return K2L_Compatibility("\277\241\276\356\263\252\300\314\306\256, \267\260\305\260 \277\241\276\356\263\252\300\314\306\256 \305\254"
			"\267\264\274\274\306\256\264\302 \260\263\301\266\270\246 \307\322 \274\366 \276\370\275\300\264\317\264\331.");
	}
	return GetLevelUpNotice983(r % 3);
}

// 004a53c0
const char* __fastcall GetRankUpNotice983(int r)
{
	switch (r)
	{
	case 0: // "'소렌'카드 사용 매수에 따라 랭크업시 올라가는 슬롯이 달라집니다."
		return K2L_Compatibility("'\274\322\267\273'\304\253\265\345 \273\347\277\353 \270\305\274\366\277\241 \265\373\266\363 \267\251\305\251"
			"\276\367\275\303 \277\303\266\363\260\241\264\302 \275\275\267\324\300\314 \264\336\266\363\301\375\264\317\264\331.");
	case 1:
		return K2L_Compatibility("\305\254\267\264 \267\251\305\251\276\367\277\241\264\302 \274\367\267\303\265\265\260\241 \307\312\277\344"
			"\307\325\264\317\264\331. \274\367\267\303\265\265\264\302 \260\324\300\323\300\273 \307\303\267\271\300\314\307\317\270\351 \310\271"
			"\265\346\307\322 \274\366 \300\326\275\300\264\317\264\331.");
	case 2:
		return K2L_Compatibility("\267\251\305\251\276\367\275\303 \301\301\300\272 \300\317\300\314 \300\317\276\356\263\257\301\366\265\265 "
			"\270\360\270\250\264\317\264\331. \260\370\271\346 \300\345\300\316\265\351\300\307 \270\273\277\241 \261\315\261\342\277\357\277\251"
			"\272\270\274\274\277\344.");
	}
	return GetRankUpNotice983(r % 3);
}

// 004a5410
const char* __fastcall GetCareNotice983(int r)
{
	switch (r)
	{
	case 0: // "숙련도 이전에는 'UCIM칩' 아이템이 필요합니다."
		return K2L_Compatibility("\274\367\267\303\265\265 \300\314\300\374\277\241\264\302 'UCIM\304\250' \276\306\300\314\305\333\300\314 "
			"\307\312\277\344\307\325\264\317\264\331.");
	case 1:
		return K2L_Compatibility("\305\254\267\264\274\274\306\256 \310\270\272\271\277\241\264\302, '\277\241\272\270\306\256 \260\255\310\255 "
			"\304\332\306\303\301\246'\260\241 \307\312\277\344\307\325\264\317\264\331.");
	case 2:
		return K2L_Compatibility("\274\367\267\303\265\265\270\246 \300\314\300\374\307\317\270\351, \301\367\301\242 \260\324\300\323\277\241 "
			"\276\262\301\366 \276\312\300\272 \305\254\267\264\274\274\306\256\265\265 \267\251\305\251\276\367\300\273 \307\322 \274\366 \300\326"
			"\275\300\264\317\264\331.");
	case 3:
		return K2L_Compatibility("\260\263\301\266 \303\353\274\322 \260\241\264\311 \310\275\274\366\270\246 \310\270\272\271\307\317\270\351, "
			"\305\254\267\264 \260\263\301\266 \260\341\260\372 \310\256\300\316 \310\304 \303\353\274\322\307\322 \274\366 \300\326\275\300\264\317"
			"\264\331.");
	}
	return GetCareNotice983(r % 4);
}

// 004a5470
const char* __fastcall GetStatName983(int stat)
{
	switch (stat)
	{
	case 0: return K2L_Compatibility("\306\304\277\366"); // 파워
	case 1: return K2L_Compatibility("\304\301\306\256\267\321"); // 컨트롤
	case 2: return K2L_Compatibility("\301\244\310\256\265\265"); // 정확도
	case 3: return K2L_Compatibility("\275\272\307\311"); // 스핀
	case 4: return K2L_Compatibility("\304\277\272\352"); // 커브
	}
	return "INVALID";
}

// 004a54d0
const char* __fastcall GetRankUpTipText983()
{
	switch (rand() % 6)
	{
	case 0:
		return K2L_Compatibility("\277\300\264\303\300\272 \270\301\304\241\270\246 \304\241\264\302 \274\325\270\300\300\314 \264\331\270\245 "
			"\263\257\260\372 \264\336\266\363. \263\252\277\241\260\324 \301\273 \264\365 \270\303\260\334\272\270\301\366 \276\312\260\332\263\252?");
	case 1:
		return K2L_Compatibility("\300\341\261\361\300\341\261\361. \300\314 \305\254\267\264\274\274\306\256, \264\331\270\245 \305\254\267\264"
			"\260\372\264\302 \264\331\270\245 \264\300\263\246\300\314 \265\345\264\302\265\245... \301\273 \264\365 \273\354\306\354\272\301\265\265"
			" \265\307\260\332\263\252?");
	case 2:
		return K2L_Compatibility("\310\357\271\314\267\323\261\272. \300\332\263\327\270\270 \307\343\266\364\307\321\264\331\270\351 \300\314 "
			"\305\254\267\264\274\274\306\256\300\307 \300\347\271\314\263\255 \301\241\300\273 \272\270\277\251\301\331 \274\366 \300\326\300\273 "
			"\260\315 \260\260\263\327.");
	case 3:
		return K2L_Compatibility("\300\314 \305\254\267\264\274\274\306\256\266\363\270\351 \273\365\267\323\260\324 \270\270\265\345\264\302 "
			"\302\312\300\314 \310\316\276\300 \301\301\300\273\301\366\265\265 \270\360\270\243\260\332\261\270\270\270. \276\356\266\247, \307\321"
			"\271\370 \273\365\267\316\277\356 \305\254\267\264\300\270\267\316 \271\331\262\343\272\270\260\332\263\252?");
	case 4:
		return K2L_Compatibility("\261\342\272\320\300\314 \300\314\273\363\307\330. \300\314\267\261 \263\257\300\272 \264\303 \271\272\260\241 "
			"\301\301\300\272 \300\317\300\314 \300\317\276\356\263\265\276\372\301\366. \300\332\263\327 \305\254\267\264\277\241\274\255 \301\301"
			"\300\272 \300\317\300\314 \273\375\261\342\267\301\263\252 \272\273\265\245, \301\273 \264\365 \270\303\260\334\272\270\260\332\263\252?");
	case 5:
		return K2L_Compatibility("\277\300\264\303\300\272 \263\273 \304\301\265\360\274\307\300\314 \303\326\260\355\301\266\276\337. \270\303"
			"\260\334\301\326\270\351 \300\332\263\327\300\307 \305\254\267\264\300\273 \303\326\260\355\267\316 \270\270\265\351\276\356 \301\331 "
			"\274\366\265\265 \300\326\300\273 \260\315 \260\260\276\306. \307\317\307\317\307\326.");
	}
	return "...";
}

// 004a5550: sem chamador no exe
const char* __fastcall GetFailTalkText983()
{
	switch (rand() % 3)
	{
	case 0: // "좋은 느낌이었는데, 아쉽구만. 허나 오늘만이 날은 아니겠지."
		return K2L_Compatibility("\301\301\300\272 \264\300\263\246\300\314\276\372\264\302\265\245, \276\306\275\261\261\270\270\270. \307\343"
			"\263\252 \277\300\264\303\270\270\300\314 \263\257\300\272 \276\306\264\317\260\332\301\366.");
	case 1:
		return K2L_Compatibility("\276\356\302\277 \274\366 \276\370\301\366. \264\331\300\275\277\241 \301\301\300\272 \261\342\310\270\264\302 "
			"\266\307 \277\303\260\305\266\363\260\355 \271\317\263\327.");
	case 2:
		return K2L_Compatibility("\261\327\267\241. \276\326\301\366\301\337\301\366\307\317\264\370 \305\254\267\264\274\274\306\256\270\246 \270"
			"\303\261\344\264\331\264\302\260\324 \275\254\277\356 \300\317\300\272 \276\306\264\317\301\366. \264\331\300\275\277\241 \266\307 \261"
			"\342\310\270\260\241 \300\326\300\273\260\311\274\274.");
	}
	return "...";
}

// 004a5590: "클럽 장인의 이야기에 따라보시겠습니까?"
const char* __fastcall GetLuckyChanceText983()
{
	return K2L_Compatibility("\305\254\267\264 \300\345\300\316\300\307 \300\314\276\337\261\342\277\241 \265\373\266\363\272\270\275\303\260\332"
		"\275\300\264\317\261\356?");
}
