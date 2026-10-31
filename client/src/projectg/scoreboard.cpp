#include "minatl.h"
#include "scoreboard.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "frbutton.h"
#include "frarea.h"
#include "frgaugebar.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "projectg.h"
#include "shareddoc.h"
#include "user_info.h"
#include "intrusion.h"
#include "actor.h"
#include "contentsdoc.h"
#include "jp/holeinonegamedoc.h" // B1484: m_28 do conteiner 0xaa pelo membro (depois de std::map; cl 14.44 = +0x20)
#include "netresourcemanager.h"
#include "partsinfo.h"
#include "fx.h"
#include "parttidlist.h"
// WRect::WRect<float,float>(const WPoint&, float, float) is out of line in the original (template ctor
// not declared in wtypes.h); defined here under its mangled name.
// template<class W, class H> WRect::WRect(const WPoint& pt, W width, H height) : x(pt.x), y(pt.y), w(width), h(height) { }

// WRect::operator+(WRect&) is out of line in the original and not declared in wtypes.h;
// defined here under its mangled name.
// WRect WRect::operator+(WRect& r) const { return WRect(x + r.x, y + r.y, w + r.w, h + r.h); }

#include "exhibition.h"
#include "networksystem.h"
#include "wbone.h"
#include "fredit.h"
#include "soundmanager.h"
#include "simpleoverlay.h"
#include "scoredlg.h"
#include "treasuredlg.h"
#include "frdesktop.h"
#include "noticeimage.h"
#include "headicon.h"
#include "treasure.h"
#include "capturedbg.h"
#include "quitwindow.h"
#include "voiceitem.h"
#include "levelupitemdlg.h"
#include "../shared/sharedtables.h"
#include "clientsetting.h"
#include "jp/docofs983.h"
#include "jp/shareddocjp.h" // 983: IsTrajectoryPreviewOn (0063bba0)
#include "s5/classicserver/classicserver.h" // 983: IsClassicGame983/GetClassicExp983 (FrAwardForm::CalcResult*)
#include "../shared/LuaSystem/luascriptmanager.h" // 983: CScore::HandleMsg 0x172 (placar do Grand Prix)
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594 && offsetof(CSharedDoc, m_myInfo.info.sNick) == 0x7b8 && offsetof(CSharedDoc, m_myInfo.info.dwIdentity) == 0x812 && offsetof(CSharedDoc, m_myInfo.info.dwGalleryGuid) == 0x816 && offsetof(CSharedDoc, m_myInfo.info.dwGuid) == 0x81a); // A747: ex-crus
#include "../shared/LuaSystem/luascript.h"
#include "../shared/LuaSystem/luatablehelper.h"

// 983: leitura por nome fora de linha (00470f20 float), como em jp/holeinonemissiondlg.cpp
// 983: tabela filha por nome (0049a4e0) e leitura por indice embutida (copia de include/jp/lua983.h)
struct LuaChild983 : lua_system::lua_table_helper
{
	LuaChild983(lua_tinker::table value) : lua_system::lua_table_helper(value) { }
	float GetByIndex(int index); // 0049a000
	void Push983(int index)
	{
		if (table.m_obj)
		{
			lua_pushnumber(state, (double)index);
			lua_gettable(state, table.m_obj->m_index);
			if (lua_type(state, -1) != LUA_TNIL)
			{
				lua_remove(state, -1);
				lua_rawgeti(state, table.m_obj->m_index, index);
				return;
			}
			lua_remove(state, -1);
		}
		lua_pushnil(state);
	}
	template <typename T>
	T GetInl983(int index)
	{
		Push983(index);
		return lua_tinker::pop<T>(state);
	}
};
struct LuaHelper983 : lua_system::lua_table_helper
{
	template <typename T>
	T Get(const char* name);
	template <typename T>
	T GetChecked983(const char* name); // 00483880 (<const char*>; copia de include/jp/lua983.h)
	LuaChild983 GetChild983(const char* name); // 0049a4e0
};
// erro do gerenciador de scripts (00c1f480; a mesma visao de include/jp/lua983.h)
class LuaScriptManagerJp983
{
public:
	void Error983(const char* text);
};
template <typename T>
T LuaHelper983::GetChecked983(const char* name)
{
	if (table.m_obj)
	{
		lua_pushstring(state, name);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) == LUA_TNIL)
			lua_remove(state, -1);
		else
		{
			lua_remove(state, -1);
			goto found;
		}
	}
	char text[512];
	sprintf(text, "not found '%s' value.", name);
	((LuaScriptManagerJp983*)lua_system::lua_script_manger::GetInstance())->Error983(text);
found:
	return table.get<T>(name);
}
template <typename T>
T LuaHelper983::Get(const char* name)
{
	return table.get<T>(name);
}
#define LUA983_GET(tbl, type, name) ((LuaHelper983&)(tbl)).Get<type>(name)
#define LUA983_STR(tbl, name) ((LuaHelper983&)(tbl)).GetChecked983<const char*>(name)
unsigned char GetDifficulty(); // 006ad0f0 (club.cpp)


#define RB_STR_FRAMES4 "FRAMES4"
#define RB_STR_BLUETEAM_RANK "30in_team_blueteam_rank"
#define RB_STR_REDTEAM_RANK "30in_team_redteam_rank"
#define RB_STR_30IN_RANK_02 "30in_rank_02"
#define RB_STR_30IN_RANK "30in_rank"
#define RB_STR_FMT_JO "%d \xc1\xb6"

extern Fresh* g_pFresh;

const char* SCORE_PPL_DIR = "./score_ppl";

int __fastcall float2int(float f);
unsigned char MyIndex();
bool ApproachRankCompare(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;

	if (pB->quitOrder < pA->quitOrder)
		return true;
	if (pA->quitOrder < pB->quitOrder)
		return false;

	unsigned long dist = pA->approachDistance;
	unsigned long resultDist = pA->approachResultDistance;
	if (dist == resultDist)
	{
		if (pB->approachDistance != pB->approachResultDistance)
			return true;
	}
	else if (pB->approachDistance == pB->approachResultDistance)
		return false;

	if (dist == resultDist)
	{
		unsigned long resultDistB = pB->approachResultDistance;
		if (pB->approachDistance == resultDistB)
		{
			if (resultDist < resultDistB)
				return true;
			if (resultDist != resultDistB)
				return false;
		}
	}

	if (pB->approachTime < pA->approachTime)
		return true;
	if (pA->approachTime < pB->approachTime)
		return false;
	return pA->order < pB->order;
}

bool RankCompare(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;

	// 983 0077b860: cada chave decide nos dois sentidos antes da seguinte
	if (pA->quitOrder > pB->quitOrder)
		return true;
	if (pA->quitOrder < pB->quitOrder)
		return false;
	if (pA->rank < pB->rank)
		return true;
	if (pA->rank > pB->rank)
		return false;
	if (pA->totalScore < pB->totalScore)
		return true;
	if (pA->totalScore > pB->totalScore)
		return false;
	if (pA->totalPang > pB->totalPang)
		return true;
	if (pA->totalPang < pB->totalPang)
		return false;
	return pA->order < pB->order;
}

bool GrandPrixRankCompare983(const void* a, const void* b);
IMPLEMENT_ACTOR(FrRankForm, FrForm)

BEGIN_FRESH_MSGMAP(FrRankForm, FrForm)

ON_FRESH_VI("list", FRCMD_INIT, FrRankForm::OnRankInit)
ON_FRESH_VI("list", FRCMD_OWNERDRAW, FrRankForm::OnRankOwnerDraw)
ON_FRESH_VV("list", FRCMD_LBUTTONDOWN, FrRankForm::OnRankLBtnDown)
ON_FRESH_VV("list", FRCMD_RBUTTONUP, FrRankForm::OnRankRBtnUp)
ON_FRESH_VV("list", FRCMD_DBLCLICK, FrRankForm::OnRankDClick)
ON_FRESH_VV("center", FRCMD_LBUTTONUP, FrRankForm::OnCenterBtnUp)
ON_FRESH_VV("info", FRCMD_LBUTTONUP, FrRankForm::OnInfoBtnUp)
ON_FRESH_VV("size", FRCMD_LBUTTONUP, FrRankForm::OnSizeBtnUp)
ON_FRESH_VI("billboard", FRCMD_INIT, FrRankForm::OnBoardInit)
ON_FRESH_VI("bar01", FRCMD_INIT, FrRankForm::OnBar01Init)
ON_FRESH_VI("bar02", FRCMD_INIT, FrRankForm::OnBar02Init)

END_FRESH_MSGMAP()

FrRankForm::FrRankForm()
	: m_pList(NULL),
	  m_pBoard(NULL),
	  m_tieUid(0xffffffff),
	  m_lastUid(0xffffffff),
	  m_selOid(0xffffffff),
	  m_selAiId983(0),
	  m_leadNoticeHole983(0),
	  m_scrollCount(0)
{
	memset(m_holePlayers983, 0, sizeof(m_holePlayers983));
	// 983 (00781430): container 0xaa na sala 0xb
	if (Doc()->m_golfGame.gameType == 0xb)
	{
		void* pContainer = CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa);
		if (pContainer)
			m_pContainer983 = pContainer;
	}
	else
		m_pContainer983 = NULL;
}

void FrRankForm::OnProc(const float deltaTime)
{
	if (Doc()->m_golfGame.gameType == 10 &&
		(g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true)))
	{
		if (g_input->GetDown("PAGE_DOWN", true))
			m_pList->GetScrollBar()->ScrollDown(1);
		else if (g_input->GetDown("PAGE_UP", true))
			m_pList->GetScrollBar()->ScrollUp(1);
	}

	switch (m_captionType)
	{
	case 0:
	{
		if (Doc()->m_golfGame.gameType == 5)
		{
			m_pBoard->SetCaption(MakeStr(
				K2L_Compatibility("RED %d\xc1\xa1/%I64d\xc6\xce  vs  BLUE %d\xc1\xa1/%I64d\xc6\xce"),
				Doc()->m_teamResult[0].score, Doc()->m_teamResult[0].pang,
				Doc()->m_teamResult[1].score, Doc()->m_teamResult[1].pang));
		}
		else if (m_tieUid != 0xffffffff)
		{
			if (Doc()->m_golfGame.gameType == 10)
			{
				m_pBoard->SetCaption(MakeStr(
					K2L_Compatibility("\xc7\xf6\xc0\xe7 1\xc0\xa7 %s\xb4\xd4, \xb3\xb2\xc0\xba\xb0\xc5\xb8\xae : %d.%d y"),
					Doc()->m_rivalList[Doc()->GetIndex(m_tieUid)].nickname,
					Doc()->m_rivalList[Doc()->GetIndex(m_tieUid)]
							.approachResultDistance /
						10,
					Doc()->m_rivalList[Doc()->GetIndex(m_tieUid)]
							.approachResultDistance %
						10));
			}
			else
			{
				m_pBoard->SetCaption(MakeStr(
					K2L_Compatibility("\xc7\xf6\xc0\xe7 1\xc0\xa7 %s\xb4\xd4, \xbd\xba\xc4\xda\xbe\xee %d"),
					Doc()->m_rivalList[Doc()->GetIndex(m_tieUid)].nickname,
					Doc()->m_rivalList[Doc()->GetIndex(m_tieUid)].totalScore));
			}
		}
		else
		{
			m_pBoard->SetCaption(MakeStr(
				K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xc1\xdf\xc0\xce \xc0\xce\xbf\xf8\xc0\xba %d\xb8\xed\xc0\xd4\xb4\xcf\xb4\xd9."),
				m_memberCount));
		}
		m_captionPrio983 = 4;
	}
	break;
	case 1:
	{
		m_captionTime += deltaTime;
		if (m_captionTime > 10.0f)
		{
			m_captionTime = 0.0f;
			SetCaptionType(0);
		}
	}
	break;
	case 2:
	{
		WRect rect;
		m_pBoard->GetClientRect(rect);
		rect.x -= deltaTime * 30.0f;
		m_pBoard->SetClientRect(rect);
		if (rect.x + rect.w < g_view->GetWidth() - 239.0f)
		{
			m_scrollCount++;
			if (m_scrollCount == 2)
			{
				m_scrollCount = 0;
				SetCaptionType(0);
			}
			else
				SetCaptionType(2);
		}
	}
	break;
	}
}

void FrRankForm::OnRankInit(int param)
{
	m_pList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
		m_pList->AddItem(&*it);
	m_pList->UseRightButton(true);
	m_memberCount = Doc()->m_rivalList.size();
	m_holePlayers983[0] = Doc()->m_rivalList.size();
	SortRank();
	m_size = (Doc()->m_golfGame.gameType == 10) + 1;
	m_pMedalBmp[0] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_gold");
	m_pMedalBmp[1] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_silver");
	m_pMedalBmp[2] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_bronze");
	m_pMedalBmp[3] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_noaward");
	m_pTeamRankBmp[0] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_REDTEAM_RANK);
	m_pTeamRankBmp[1] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_BLUETEAM_RANK);
	m_pRankBgBmp =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_30IN_RANK);
	m_pad983_118 = 0;
	m_pPcIcon = g_pFresh->GetManager()->GetBitmap("golf", "premium_score");
}

// 983 0078c0f0: funcao livre __stdcall (ret 4, ecx sem uso) compartilhada por FrRankForm/FrRankForm2/CRankExData;
// a tabela contagem x posicao esta em 006335c0 (MedalByCountRank).
int __stdcall MedalByCountRank(int count, int rank)
{
	if (count <= 14)
		return rank == 1 ? 2 : 3;
	if (count <= 18)
	{
		if (rank == 1)
			return 1;
		return rank == 2 ? 2 : 3;
	}
	if (count <= 22)
	{
		if (rank == 1)
			return 0;
		if (rank == 2)
			return 1;
		return rank == 3 ? 2 : 3;
	}
	if (count <= 26)
	{
		if (rank == 1)
			return 0;
		if (rank == 2)
			return 1;
		return rank <= 4 ? 2 : 3;
	}
	if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
	{
		if (rank == 1)
			return 0;
		if (rank == 2)
			return 1;
		return rank == 3 ? 2 : 3;
	}
	if (rank == 1)
		return 0;
	if (rank <= 3)
		return 1;
	return rank <= 6 ? 2 : 3;
}

// 983 0078ccb0 (chamada por 0078cdc0): medalha pelo oid, igual a 0078da70 (sem variante de nickname nem de intrusao)
static unsigned __stdcall MedalByOid983(unsigned long oid);
static unsigned __stdcall MedalByRival983(const sRivalData* pRival);

void FrRankForm::OnRankOwnerDraw(int param)
{
	if (m_size == 0 || param == NULL)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	CSharedDoc* pDoc = Doc();
	float x = pItem->pos.x, y = pItem->pos.y;

	_WRECT rect;
	rect.x = x + 7.0f;
	rect.y = y;
	rect.w = (float)m_pList->GetItemWidth() - 10.0f;
	rect.h = (float)m_pList->GetItemHeight();

	int space = pGDI->SetSpace(0);
	sRivalData* pRival = (sRivalData*)pItem->pData;

	// 983: nas salas 0x14/0x15 o destaque tambem compara o id do time (+0x1b6) do rival de convidado (+0x1b4)
	if ((unsigned char)(pDoc->m_roomInfo.realGameType - 0x14) < 2
			? (m_selOid == pRival->oid && !pRival->bGrandPrixAI983) ||
				(m_selAiId983 == pRival->grandPrixAIId983 && pRival->bGrandPrixAI983)
			: m_selOid == pRival->oid)
		pGDI->Box(rect, 0x999B9DFF, 0, 0.0f);

	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xFFFFFFFF, 0xFFFFFFFF);

	unsigned char gameType = pDoc->m_golfGame.gameType;
	WPoint rankPos(x + 21.0f, y + 3.0f);
	if (gameType == 5)
	{
		const Bitmap* pTeam = m_pTeamRankBmp[pRival->team];
		if (pTeam)
			pGDI->DrawTexture(pTeam,
				WRect(x + 12.0f, y + 2.0f, (float)pTeam->Width(), (float)pTeam->Height()),
				0xFFFFFFFF, 0);
		pGDI->Print(rankPos, 1, "%d", pRival->rank);
	}
	else
	{
		if (m_pRankBgBmp)
			pGDI->DrawTexture(m_pRankBgBmp,
				WRect(x + 12.0f, y + 2.0f, (float)m_pRankBgBmp->Width(),
					(float)m_pRankBgBmp->Height()),
				0xFFFFFFFF, 0);
		if (pRival->state == 3 || (gameType == 10 && pRival->approachDistance == 0xFFFFFFFF))
			pGDI->Print(rankPos, 1, "-");
		else
			pGDI->Print(rankPos, 1, "%d", pRival->rank);
	}

	if (gameType != 10 &&
		(pDoc->m_roomInfo.gameType == 11 || !pDoc->m_roomInfo.bAdminMaster ||
			pDoc->m_roomInfo.nUserLimit < 100) &&
		pRival->hole != pDoc->m_holeOrder[0])
	{
		unsigned medal = MedalByRival983(pRival);
		if (medal < 3)
		{
			if (pRival->flag > 3)
				medal = 3;
			pGDI->DrawTexture(m_pMedalBmp[medal],
				WRect(x + 2.0f, y + 1.0f, (float)m_pMedalBmp[medal]->Width(),
					(float)m_pMedalBmp[medal]->Height()),
				0xFFFFFFFF, 0);
		}
	}

	unsigned long myUid = ((pDoc->m_myInfo.info.dwIdentity >> 1) & 1)
		? pDoc->m_myInfo.info.dwGalleryGuid
		: pDoc->m_myInfo.info.dwGuid;
	if (pRival->oid == myUid && !pRival->bGrandPrixAI983)
		pGDI->SetTextColor(gameType == 5 && pRival->team ? 0xFF0060FF : 0xFFFF0000, 0xFFFFFFFF);
	else
		pGDI->SetTextColor(pRival->state == 3 ? 0xFF808080 : 0xFF000000, 0xFFFFFFFF);

	if (pRival->guildUID)
	{
		const Bitmap* pEmblem =
			NetResourceManager::Instance()->GetEmblem983(pRival->guildUID, pRival->guildEmblemVer983);
		if (pEmblem)
			pGDI->DrawTexture(pEmblem,
				WRect(x + 33.0f, y + 3.0f - 6.0f, (float)pEmblem->Width(),
					(float)pEmblem->Height()),
				0xFFFFFFFF, 0);
	}

	if (CProjectG::Instance()->HidePrivacy() == false)
		g_pFresh->GetManager()->PrintText(WPoint(x + 60.0f, y + 3.0f), 0,
			pRival->nickname, 90.0f, 0xFFFFFFFF);

	if (gameType == 10)
	{
		if (pRival->approachTime == -1)
		{
			if (pRival->approachDistance == pRival->approachResultDistance)
				pGDI->Print(WPoint(x + 210.0f, y + 3.0f), 2, "Out");
			else
				pGDI->Print(WPoint(x + 202.0f, y + 3.0f), 2, "-");
			pGDI->Print(WPoint(x + 254.0f, y + 3.0f), 2, "-");
		}
		else
		{
			if (pRival->approachDistance == pRival->approachResultDistance)
			{
				if (pRival->approachResultDistance == 0xFFFFFFFF)
					pGDI->Print(WPoint(x + 210.0f, y + 3.0f), 2, "Out");
				else
					pGDI->Print(WPoint(x + 217.0f, y + 3.0f), 2, "%d.%d y",
						pRival->approachResultDistance / 10, pRival->approachResultDistance % 10);
			}
			else if (pRival->oid == pDoc->m_myInfo.info.dwGuid &&
				(pDoc->m_pGolfDoc->m_gameMode & 0x40))
				pGDI->Print(WPoint(x + 202.0f, y + 3.0f), 2, "-");
			else
				pGDI->Print(WPoint(x + 217.0f, y + 3.0f), 2, "%d.%d y",
					pRival->approachDistance / 10, pRival->approachDistance % 10);

			pGDI->Print(WPoint(x + 273.0f, y + 3.0f), 2, "%d.%03d",
				pRival->approachTime / 1000, pRival->approachTime % 1000);
		}
		pGDI->SetSpace(space);
		return;
	}

	pGDI->Print(WPoint(x + 179.0f, y + 3.0f), 2, "%d", pRival->totalScore);
	if (gameType == 11)
	{
		// 983: sala tiki; so o proprio jogador mostra os dois contadores
		if (pRival->oid == pDoc->m_myInfo.info.dwGuid)
		{
			pGDI->Print(WPoint(x + 225.0f, y + 3.0f), 2, "%d",
				pDoc->m_holeStatistics[pDoc->m_pGolfDoc->GetCurrentPlayer()].wHoleInOne);
			pGDI->Print(WPoint(x + 265.0f, y + 3.0f), 2, "%d",
				((_gateway::_golfgame::CHoleInOneGameDoc*)m_pContainer983)->m_28);
		}
		pGDI->SetSpace(space);
		return;
	}

	pGDI->Print(WPoint(x + 222.0f, y + 3.0f), 2, "%I64d", pRival->totalPang);
	WPoint holePos(x + 272.0f, y + 3.0f);
	if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
		pGDI->Print(holePos, 2, "End");
	else if (pRival->state == 3)
		pGDI->Print(holePos, 2, "Out");
	else
		pGDI->Print(holePos, 2, K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pRival->hole));

	if (m_pPcIcon && !pRival->bGrandPrixAI983 && (pRival->capability & 0x4000))
		pGDI->DrawTexture(m_pPcIcon,
			WRect(x + 259.0f, y - 4.0f, (float)m_pPcIcon->Width(), (float)m_pPcIcon->Height()),
			0xFFFFFFFF, 0);

	pGDI->SetSpace(space);
}

// 983 (007891a0): grava a selecao (Grand Prix: aiId em +0x130 e oid -1) e abre o info do rival
void FrRankForm::SetSel983(unsigned long oid, unsigned short aiId, unsigned char aiType)
{
	if (aiType)
	{
		m_selAiId983 = aiId;
		m_selOid = 0xffffffff;
	}
	else
	{
		m_selAiId983 = 0;
		m_selOid = oid;
	}
}

void FrRankForm::OnRankLBtnDown()
{
	// 983 007891a0: no Grand Prix a selecao passa por SetSel983 (0077ba30); cada ramo com o seu SetInfo
	if (!m_pList->GetItemUnderCursor())
		return;
	sRivalData* pRival = (sRivalData*)m_pList->GetItemUnderCursor()->pData;
	if (!pRival)
		return;
	switch (Doc()->m_roomInfo.realGameType)
	{
	case 0x14: case 0x15:
		SetSel983(pRival->oid, pRival->grandPrixAIId983, pRival->bGrandPrixAI983);
		if (CUserInfo::Instance() && pRival->state != 3)
			CUserInfo::Instance()->SetInfo(pRival->uid, m_selOid, false, true, false, false,
				pRival->nickname, pRival->bGrandPrixAI983, pRival->grandPrixAIId983);
		break;
	default:
		m_selOid = pRival->oid;
		if (CUserInfo::Instance() && pRival->state != 3)
			CUserInfo::Instance()->SetInfo(pRival->uid, m_selOid, false, true, false, false,
				pRival->nickname, 0, 0);
	}
}

void FrRankForm::OnRankRBtnUp()
{
	if (m_pList->GetItemUnderCursor())
	{
		sRivalData* pRival = (sRivalData*)m_pList->GetItemUnderCursor()->pData;
		if (pRival)
		{
			// 983 (00789830): nao grava m_selOid; passa aiType/aiId do rival (+0x1b4/+0x1b6)
			if (CUserInfo::Instance() && pRival->state != 3)
				CUserInfo::Instance()->SetInfo(pRival->uid, pRival->oid, false,
					true, false, false, pRival->nickname, pRival->bGrandPrixAI983,
					pRival->grandPrixAIId983);
		}
	}
}

void FrRankForm::OnRankDClick()
{
	OnRankRBtnUp();
}

void FrRankForm::OnBoardInit(int param)
{
	m_pBoard = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pBoard->HidePrivacy(true);
	m_pBoard->SetCaption("");
	SetCaptionType(0);
	m_captionPrio983 = 4;
	m_captionTime = 0.0f;
}

void FrRankForm::OnBar01Init(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
	{
		// 983: gameType 10 = approach, 11 = hole-in-one (holein_base_01)
		if (Doc()->m_golfGame.gameType == 10)
			pArea->SetBgImg("approach_base_01");
		else if (Doc()->m_golfGame.gameType == 11)
			pArea->SetBgImg("holein_base_01");
	}
}

void FrRankForm::OnBar02Init(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
	{
		// 983: gameType 10 = approach, 11 = hole-in-one (holein_base_02)
		if (Doc()->m_golfGame.gameType == 10)
			pArea->SetBgImg("approach_base_02");
		else if (Doc()->m_golfGame.gameType == 11)
			pArea->SetBgImg("holein_base_02");
	}
}

void FrRankForm::OnCenterBtnUp()
{
	Align();
}

void FrRankForm::Align()
{
	// 983 00788530: MyGuid(true) fora de linha (0047dd30), chamada uma vez se a lista nao
	// estiver vazia; linha do topo presa em [0, maxRow] numa chamada so
	unsigned long (__fastcall* const myGuid)(bool) = MyGuid;
	unsigned char size = m_size;
	if (!size)
		return;

	FrListBox* pList = m_pList;
	std::list<FrListItem*>& items = pList->m_itemList;
	const std::list<FrListItem*>::iterator end = items.end();
	std::list<FrListItem*>::iterator it = items.begin();
	int row = 0;
	if (it != end)
	{
		unsigned long myUid = myGuid(true);
		do
		{
			if (((sRivalData*)(*it)->pData)->oid == myUid)
				break;
			++it;
			row++;
		} while (it != end);
	}

	FrScrollBar* pScroll = pList->GetScrollBar();
	if (pScroll)
	{
		if (size == 1)
		{
			int maxRow = Doc()->m_rivalList.size() - 5;
			row -= 2;
			pScroll->SetCurTopRow(row < 0 ? 0 : (row > maxRow ? maxRow : row));
		}
		else
		{
			int maxRow = Doc()->m_rivalList.size() - 10;
			row -= 4;
			pScroll->SetCurTopRow(row < 0 ? 0 : (row > maxRow ? maxRow : row));
		}
	}
}

void FrRankForm::OnInfoBtnUp()
{
	std::map<unsigned long, sBriefUserInfo>::iterator it =
		Doc()->m_briefUserInfoMap.find(m_selOid);
	if (it != Doc()->m_briefUserInfoMap.end())
		CUserInfo::Instance()->SetInfo(it->second.dwUid, m_selOid, true, true,
			false, false, it->second.sNick);
}

void FrRankForm::OnSizeBtnUp()
{
	m_size = (m_size + 1) % 3;
	Resize();
}

void FrRankForm::Resize()
{
	if (m_pList)
	{
		WRect rect;
		m_pList->GetClientRect(rect);
		switch (m_size)
		{
		case 0:
			rect.h = 0.0f;
			break;
		case 1:
			rect.h = 90.0f;
			break;
		case 2:
			rect.h = 180.0f;
			break;
		}
		m_pList->SetClientRect(rect);
		rect.h += 70.0f;
		rect.y = 0.0f;
		rect.w = m_pBaseFrm->GetRect().w;
		m_pBaseFrm->SetClientRect(rect);
	}
}

void FrRankForm::SortRank()
{
	m_lastUid = 0xffffffff;
	m_tieUid = 0xffffffff;
	int tieCount = 0;
	if (Doc()->m_golfGame.gameType == 10)
	{
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); ++it)
		{
			if (it->state == 3)
			{
				it->rank = Doc()->m_rivalList.size() - it->quitOrder;
			}
			else
			{
				it->rank = 1;
				for (std::vector<sRivalData>::iterator it2 =
						 Doc()->m_rivalList.begin();
					it2 != Doc()->m_rivalList.end(); ++it2)
				{
					if (it == it2 || it2->state == 3)
						continue;

					if (it2->approachDistance == it2->approachResultDistance)
					{
						if (it->approachDistance ==
								it->approachResultDistance &&
							it->approachResultDistance <=
								it2->approachResultDistance)
						{
							if (it->approachResultDistance ==
									it2->approachResultDistance &&
								it->approachTime < it2->approachTime)
								it->rank++;
						}
						else
							it->rank++;
					}
					else if (it->approachDistance !=
							it->approachResultDistance &&
						it->approachTime < it2->approachTime)
						it->rank++;
				}
				if (it->rank == 1)
				{
					if (it->approachDistance == it->approachResultDistance &&
						it->approachResultDistance != 0xffffffff)
					{
						tieCount++;
						m_tieUid = it->oid;
					}
				}
				else if (it->rank == m_memberCount)
					m_lastUid = it->oid;
			}
		}
		m_pList->SortItem(ApproachRankCompare);
	}
	else
	{
		// 983 (00784bd0): Grand Prix (realGameType 0x14/0x15) so ordena por rank (0077b8f0)
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2)
		{
			m_pList->SortItem(GrandPrixRankCompare983);
			m_tieUid = 0xffffffff;
			return;
		}
		CSharedDoc* pDoc = Doc();
		memset(pDoc->m_teamResult, 0, sizeof(pDoc->m_teamResult));
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); ++it)
		{
			if (it->state == 3)
			{
				it->rank = Doc()->m_rivalList.size() - it->quitOrder;
			}
			else
			{
				it->rank = 1;
				for (std::vector<sRivalData>::iterator it2 =
						 Doc()->m_rivalList.begin();
					it2 != Doc()->m_rivalList.end(); ++it2)
				{
					if (it == it2 || it2->state == 3)
						continue;

					// 983: gameType 11/13 (hole-in-one) ordena pelo maior score
					bool hio = Doc()->m_roomInfo.realGameType == 11 ||
						Doc()->m_roomInfo.realGameType == 13;
					if ((hio ? it->totalScore < it2->totalScore
							 : it->totalScore > it2->totalScore) ||
						(it->totalScore == it2->totalScore &&
							it->totalPang < it2->totalPang))
						it->rank++;
				}
				if (it->rank == 1)
				{
					tieCount++;
					m_tieUid = it->oid;
				}
				else if (it->rank == m_memberCount)
					m_lastUid = it->oid;
			}
			Doc()->m_teamResult[it->team].score += it->rank;
			Doc()->m_teamResult[it->team].pang += it->totalPang;
		}
		m_pList->SortItem(RankCompare);
	}
	if (tieCount != 1)
		m_tieUid = 0xffffffff;
}

void FrRankForm::SetCaptionType(unsigned char type)
{
	m_captionType = type;
	if (m_pBoard)
	{
		WRect rect;
		m_pBoard->GetClientRect(rect);
		if (m_captionType == 2)
		{
			WRect clip = rect;
			clip.w = 219.0f;
			clip.h = 16.0f;
			m_pBoard->SetClippingArea(&clip);
			rect.x = 800.0f;
			rect.w = m_pBoard->GDI()->GetTextExtendOld983(m_pBoard->GetCaption());
			m_pBoard->SetClientRect(rect);
		}
		else
		{
			m_pBoard->SetClippingArea(NULL);
			WRect clip = rect;
			clip.w = 219.0f;
			clip.h = 16.0f;
			m_pBoard->SetClientRect(clip);
		}
	}
}

IMPLEMENT_ACTOR(FrRankForm2, FrForm)

BEGIN_FRESH_MSGMAP(FrRankForm2, FrForm)

ON_FRESH_VI("list", FRCMD_INIT, FrRankForm2::OnRankInit)
ON_FRESH_VI("list", FRCMD_OWNERDRAW, FrRankForm2::OnRankOwnerDraw)

END_FRESH_MSGMAP()

FrRankForm2::FrRankForm2()
	: m_pList(NULL)
{
}

void FrRankForm2::OnRankInit(int param)
{
	m_pList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
		m_pList->AddItem(&*it);
	SortRank();
	m_pMedalBmp[0] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_gold");
	m_pMedalBmp[1] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_silver");
	m_pMedalBmp[2] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_bronze");
	m_pMedalBmp[3] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_noaward");
	m_pTeamRankBmp[0] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_REDTEAM_RANK);
	m_pTeamRankBmp[1] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_BLUETEAM_RANK);
	m_pRankBgBmp =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_30IN_RANK);
	m_pad983_130 = 0;
	m_pPremiumIcon = g_pFresh->GetManager()->GetBitmap("golf", "premium_score");
}

void FrRankForm2::SortRank()
{
	CSharedDoc* pDoc = Doc();
	memset(pDoc->m_teamResult, 0, sizeof(pDoc->m_teamResult));
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
	{
		if (it->state == 3)
		{
			it->rank = Doc()->m_rivalList.size() - it->quitOrder;
		}
		else
		{
			it->rank = 1;
			for (std::vector<sRivalData>::iterator it2 =
					 Doc()->m_rivalList.begin();
				it2 != Doc()->m_rivalList.end(); ++it2)
			{
				if (it == it2 || it2->state == 3)
					continue;

				if (it->totalScore > it2->totalScore ||
					(it->totalScore == it2->totalScore &&
						it->totalPang < it2->totalPang))
					it->rank++;
			}
			Doc()->m_teamResult[it->team].score += it->rank;
			Doc()->m_teamResult[it->team].pang += it->totalPang;
		}
	}
	m_pList->SortItem(RankCompare);
}

void FrRankForm2::OnRankOwnerDraw(int param)
{
	if (param == NULL)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	int space = pGDI->SetSpace(0);
	FrListItem* pItem = (FrListItem*)param;
	sRivalData* pRival = (sRivalData*)pItem->pData;

	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xFFFFFFFF, 0xFFFFFFFF);

	if (Doc()->m_golfGame.gameType == 5)
	{
		if (m_pTeamRankBmp[pRival->team])
			pGDI->DrawTexture(m_pTeamRankBmp[pRival->team],
				WRect(pItem->pos.x + 12.0f, pItem->pos.y + 2.0f,
					(float)m_pTeamRankBmp[pRival->team]->Width(),
					(float)m_pTeamRankBmp[pRival->team]->Height()),
				0xFFFFFFFF, 0);
		pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1, "%d",
			pRival->rank);
	}
	else
	{
		if (m_pRankBgBmp)
			pGDI->DrawTexture(m_pRankBgBmp,
				WRect(pItem->pos.x + 12.0f, pItem->pos.y + 2.0f,
					(float)m_pRankBgBmp->Width(),
					(float)m_pRankBgBmp->Height()),
				0xFFFFFFFF, 0);
		if (pRival->state == 3)
			pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1,
				"-");
		else
			pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1,
				"%d", pRival->rank);
	}

	if (pRival->hole != Doc()->m_holeOrder[0])
	{
		int medal = MedalByOid983(pRival->oid);
		if (medal >= 0 && medal <= 2)
		{
			if (pRival->flag > 3)
				medal = 3;
			pGDI->DrawTexture(m_pMedalBmp[medal],
				WRect(pItem->pos.x + 2.0f, pItem->pos.y + 1.0f,
					(float)m_pMedalBmp[medal]->Width(),
					(float)m_pMedalBmp[medal]->Height()),
				0xFFFFFFFF, 0);
		}
	}

	unsigned long myUid = ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		? Doc()->m_myInfo.info.dwGalleryGuid
		: Doc()->m_myInfo.info.dwGuid;
	if (pRival->oid == myUid)
	{
		if (Doc()->m_golfGame.gameType == 5)
			pGDI->SetTextColor(pRival->team ? 0xFF0060FF : 0xFFFF0000,
				0xFFFFFFFF);
		else
			pGDI->SetTextColor(0xFFFF0000, 0xFFFFFFFF);
	}
	else if (pRival->state == 3)
		pGDI->SetTextColor(0xFF808080, 0xFFFFFFFF);
	else
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);

	if (pRival->guildUID)
	{
		const Bitmap* pEmblem =
			NetResourceManager::Instance()->GetEmblem983(pRival->guildUID, pRival->guildEmblemVer983);
		if (pEmblem)
			pGDI->DrawTexture(pEmblem,
				WRect(pItem->pos.x + 33.0f, pItem->pos.y - 3.0f,
					(float)pEmblem->Width(), (float)pEmblem->Height()),
				0xFFFFFFFF, 0);
	}

	if (CProjectG::Instance()->HidePrivacy() == false)
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 60.0f, pItem->pos.y + 3.0f), 0,
			pRival->nickname, 90.0f, 0xFFFFFFFF);

	pGDI->Print(WPoint(pItem->pos.x + 179.0f, pItem->pos.y + 3.0f), 2, "%d",
		pRival->totalScore);

	pGDI->Print(WPoint(pItem->pos.x + 222.0f, pItem->pos.y + 3.0f), 2, "%I64d",
		pRival->totalPang);

	if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			"End");
	else
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pRival->hole));

	if ((pRival->capability & 0x4000) && m_pPremiumIcon)
		pGDI->DrawTexture(m_pPremiumIcon,
			WRect(pItem->pos.x + 259.0f, pItem->pos.y + 3.0f,
				(float)m_pPremiumIcon->Width(), (float)m_pPremiumIcon->Height()),
			0xFFFFFFFF, 0);

	pGDI->SetSpace(space);
}

IMPLEMENT_ACTOR(FrRankForm3, FrForm)

BEGIN_FRESH_MSGMAP(FrRankForm3, FrForm)

ON_FRESH_VI("list", FRCMD_INIT, FrRankForm3::OnRankInit)
ON_FRESH_VI("list", FRCMD_OWNERDRAW, FrRankForm3::OnRankOwnerDraw)
ON_FRESH_VV("size", FRCMD_LBUTTONUP, FrRankForm3::OnSizeBtnUp)
ON_FRESH_VI("billboard", FRCMD_INIT, FrRankForm3::OnBoardInit)
ON_FRESH_VI("score_gb", FRCMD_INIT, FrRankForm3::OnScoreGaugeInit)
ON_FRESH_VI("score_info", FRCMD_INIT, FrRankForm3::OnScoreInfoInit)
ON_FRESH_VV("score_info", FRCMD_OWNERDRAW, FrRankForm3::OnScoreInfoOwnerDraw)

END_FRESH_MSGMAP()

FrRankForm3::FrRankForm3()
	: m_pList(NULL),
	  m_pBoard(NULL),
	  m_pScoreGauge(NULL),
	  m_pScoreInfo(NULL),
	  m_pad983_129(0),
	  m_scrollCount(0)
{
	m_selOid = 0xffffffff;
	m_size = 1;
	LoadBitmaps();
}

void FrRankForm3::OnProc(const float deltaTime)
{
	switch (m_captionType)
	{
	case 0:
		m_pBoard->SetCaption(MakeStr(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xc1\xdf\xc0\xce \xc0\xce\xbf\xf8\xc0\xba %d\xb8\xed\xc0\xd4\xb4\xcf\xb4\xd9."),
			m_memberCount));
		break;
	case 1:
		m_captionTime += deltaTime;
		if (m_captionTime > 10.0f)
		{
			m_captionTime = 0.0f;
			SetCaptionType(0);
		}
		break;
	case 2:
	{
		WRect rect;
		m_pBoard->GetClientRect(rect);
		rect.x -= deltaTime * 30.0f;
		m_pBoard->SetClientRect(rect);
		if (rect.x + rect.w < g_view->GetWidth() - 239.0f)
		{
			m_scrollCount++;
			if (m_scrollCount == 2)
			{
				m_scrollCount = 0;
				SetCaptionType(0);
			}
			else
				SetCaptionType(2);
		}
	}
	break;
	}
}

void FrRankForm3::OnRankInit(int param)
{
	m_pList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	for (std::vector<sGuildMatchup>::iterator it =
			 Doc()->m_guildMatchupList.begin();
		it != Doc()->m_guildMatchupList.end(); ++it)
		m_pList->AddItem(&*it);
	m_pList->UseRightButton(false);
	m_memberCount = Doc()->m_guildMatchupList.size() * 2;
	m_pRedEmblem = NetResourceManager::Instance()->GetEmblem983(
		Doc()->m_roomInfo.GuildInfo.nGuildID[0], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[0]);
	m_pBlueEmblem = NetResourceManager::Instance()->GetEmblem983(
		Doc()->m_roomInfo.GuildInfo.nGuildID[1], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[1]);
	m_pRankBgBmp =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_30IN_RANK_02);
	m_pad983_16c = 0;
	m_pPremiumIcon = g_pFresh->GetManager()->GetBitmap("golf", "premium_score");
	m_size = (m_size + 1) % 2;
	OnSizeBtnUp();
}

void FrRankForm3::OnRankOwnerDraw(int param)
{
	if (m_size == 0)
		return;
	if (param == NULL)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	_WRECT rect;
	rect.x = pItem->pos.x + 2.0f;
	rect.y = pItem->pos.y;
	rect.w = (float)m_pList->GetItemWidth();
	rect.h = (float)m_pList->GetItemHeight();

	int space = pGDI->SetSpace(0);
	sGuildMatchup* pMatch = (sGuildMatchup*)pItem->pData;

	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xFFFFFFFF, 0xFFFFFFFF);

	sRivalData* pRed = &Doc()->m_rivalList[Doc()->GetIndex(pMatch->uid[0])];
	sRivalData* pBlue = &Doc()->m_rivalList[Doc()->GetIndex(pMatch->uid[1])];

	if (Doc()->m_myInfo.info.dwGuid == pRed->oid ||
		Doc()->m_myInfo.info.dwGuid == pBlue->oid)
	{
		m_pList->SelectItem(pItem, true);
		pGDI->Box(rect, 0x999B9DFF);
	}

	if (m_pRankBgBmp)
	{
		pGDI->DrawTexture(m_pRankBgBmp,
			WRect(pItem->pos.x + 5.0f, pItem->pos.y + 8.0f,
				(float)m_pRankBgBmp->Width(), (float)m_pRankBgBmp->Height()),
			0xFFFFFFFF, 0);
		pGDI->Print(WPoint(pItem->pos.x + 18.0f, pItem->pos.y + 12.0f), 1,
			K2L_Compatibility(RB_STR_FMT_JO), pMatch->team);	// 983 0078d4a8
	}

	if (m_pRedEmblem)
		pGDI->DrawTexture(m_pRedEmblem,
			WRect(pItem->pos.x + 39.0f, pItem->pos.y + 3.0f,
				(float)(m_pRedEmblem->Width() / 2),
				(float)(m_pRedEmblem->Height() / 2)),
			0xFFFFFFFF, 0);

	unsigned long myUid = ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		? Doc()->m_myInfo.info.dwGalleryGuid
		: Doc()->m_myInfo.info.dwGuid;
	if (pRed->oid == myUid)
		pGDI->SetTextColor(0xFFFF0000, 0xFFFFFFFF);
	else if (pRed->state == 3)
		pGDI->SetTextColor(0xFF808080, 0xFFFFFFFF);
	else
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);

	if (CProjectG::Instance()->HidePrivacy() == false)
	{
		pGDI->SetTextColor(0xFFCD1836, 0xFFFFFFFF);
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 60.0f, pItem->pos.y + 3.0f), 0,
			pRed->nickname, 90.0f, 0xFFFFFFFF);
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	}

	pGDI->Print(WPoint(pItem->pos.x + 179.0f, pItem->pos.y + 3.0f), 2, "%d",
		pRed->guildPoint);

	pGDI->Print(WPoint(pItem->pos.x + 222.0f, pItem->pos.y + 3.0f), 2, "%I64d",
		pRed->totalPang);

	if (pRed->state == 2 || GetHoleIndex(pRed->hole) == 19)
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			"End");
	else if (pRed->state == 3)
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			"Out");
	else
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 3.0f), 2,
			K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pRed->hole));

	if ((pRed->capability & 0x4000) && m_pPremiumIcon)
		pGDI->DrawTexture(m_pPremiumIcon,
			WRect(pItem->pos.x + 259.0f, pItem->pos.y + 3.0f,
				(float)m_pPremiumIcon->Width(), (float)m_pPremiumIcon->Height()),
			0xFFFFFFFF, 0);

	if (m_pBlueEmblem)
		pGDI->DrawTexture(m_pBlueEmblem,
			WRect(pItem->pos.x + 39.0f, pItem->pos.y + 21.0f,
				(float)(m_pBlueEmblem->Width() / 2),
				(float)(m_pBlueEmblem->Height() / 2)),
			0xFFFFFFFF, 0);

	myUid = ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		? Doc()->m_myInfo.info.dwGalleryGuid
		: Doc()->m_myInfo.info.dwGuid;
	if (pBlue->oid == myUid)
		pGDI->SetTextColor(0xFFFF0000, 0xFFFFFFFF);
	else if (pBlue->state == 3)
		pGDI->SetTextColor(0xFF808080, 0xFFFFFFFF);
	else
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);

	if (CProjectG::Instance()->HidePrivacy() == false)
	{
		pGDI->SetTextColor(0xFF1D26AB, 0xFFFFFFFF);
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 60.0f, pItem->pos.y + 20.0f), 0,
			pBlue->nickname, 90.0f, 0xFFFFFFFF);
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	}

	pGDI->Print(WPoint(pItem->pos.x + 179.0f, pItem->pos.y + 20.0f), 2, "%d",
		pBlue->guildPoint);

	pGDI->Print(WPoint(pItem->pos.x + 222.0f, pItem->pos.y + 20.0f), 2, "%I64d",
		pBlue->totalPang);

	if (pBlue->state == 2 || GetHoleIndex(pBlue->hole) == 19)
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 20.0f), 2,
			"End");
	else if (pBlue->state == 3)
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 20.0f), 2,
			"Out");
	else
		pGDI->Print(WPoint(pItem->pos.x + 255.0f, pItem->pos.y + 20.0f), 2,
			K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pBlue->hole));

	if ((pBlue->capability & 0x4000) && m_pPremiumIcon)
		pGDI->DrawTexture(m_pPremiumIcon,
			WRect(pItem->pos.x + 259.0f, pItem->pos.y + 3.0f,
				(float)m_pPremiumIcon->Width(), (float)m_pPremiumIcon->Height()),
			0xFFFFFFFF, 0);

	_WPOINT from, to;
	from.x = pItem->pos.x + 1.0f;
	to.x = (float)m_pList->GetItemWidth() + pItem->pos.x;
	from.y = to.y = (float)m_pList->GetItemHeight() + pItem->pos.y;
	g_view->DrawLine2D(from, to, 0xFF000000, 0xFF000000, 0);

	pGDI->SetSpace(space);
}

void FrRankForm3::OnBoardInit(int param)
{
	m_pBoard = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pBoard->HidePrivacy(true);
	m_pBoard->SetCaption("");
	SetCaptionType(0);
	m_captionTime = 0.0f;
}

void FrRankForm3::OnScoreGaugeInit(int param)
{
	m_pScoreGauge = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pScoreGauge->SetRange(0, 10, 5);
}

void FrRankForm3::SetScore(int redScore, int blueScore)
{
	m_score[0] = redScore;
	m_score[1] = blueScore;
	redScore++;
	m_pScoreGauge->SetRange(0, redScore + blueScore + 1, redScore);
}

void FrRankForm3::DrawDigit(FrGraphicInterface* gdi, char* text, WPoint pos,
	unsigned long color, FrGDI::eTextAlign align, bool narrow)
{
	int len = strlen(text);
	const Bitmap* digits[16];
	int width = 0;
	if (align)
	{
		width = 0;
		for (int i = 0; i < len; ++i)
		{
			switch (text[i])
			{
			case '+':
				digits[i] = m_pPlus;
				break;
			case '-':
				digits[i] = m_pMinus;
				break;
			default:
				digits[i] = m_pDigit[text[i] - '0'];
				break;
			}
			width += digits[i]->Width() - (narrow ? (len < 4 ? 0 : 2) : 0);
		}
	}
	WRect rect;
	switch (align)
	{
	case 0:
		rect.x = (float)(int)pos.x;
		break;
	case 1:
		rect.x = (float)(int)(float)(pos.x - width * 0.5f);
		break;
	case 2:
		rect.x = (float)(int)(float)(pos.x - width);
		break;
	}
	rect.y = pos.y;
	for (int i = 0; i < len; ++i)
	{
		rect.w = (float)digits[i]->Width();
		rect.h = (float)digits[i]->Height();
		gdi->DrawTexture(digits[i], rect, color, 0);
		rect.x += rect.w - (narrow ? (len < 4 ? 0 : 2) : 0);
	}
}

void FrRankForm3::OnSizeBtnUp()
{
	if (m_pList)
	{
		m_size = (m_size + 1) % 2;
		WRect rect;
		m_pList->GetClientRect(rect);
		switch (m_size)
		{
		case 0:
			rect.h = 0.0f;
			break;
		case 1:
			rect.h = (float)m_pList->GetItemHeight() * 3.0f;
			break;
		}
		m_pList->SetClientRect(rect);
		rect.w = 296.0f;
		rect.y = 0.0f;
		rect.h += 113.0f;
		m_pBaseFrm->SetClientRect(rect);
	}
}

void FrRankForm3::SetCaptionType(unsigned char type)
{
	m_captionType = type;
	if (m_pBoard)
	{
		WRect rect;
		m_pBoard->GetClientRect(rect);
		if (m_captionType == 2)
		{
			WRect clip = rect;
			clip.w = 219.0f;
			clip.h = 16.0f;
			m_pBoard->SetClippingArea(&clip);
			rect.x = 800.0f;
			rect.w = m_pBoard->GDI()->GetTextExtendOld983(m_pBoard->GetCaption());
			m_pBoard->SetClientRect(rect);
		}
		else
		{
			m_pBoard->SetClippingArea(NULL);
			WRect clip = rect;
			clip.w = 219.0f;
			clip.h = 16.0f;
			m_pBoard->SetClientRect(clip);
		}
	}
}

void FrRankForm3::OnScoreInfoInit(int param)
{
	m_pScoreInfo = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pRedEmblem = NetResourceManager::Instance()->GetEmblem983(
		Doc()->m_roomInfo.GuildInfo.nGuildID[0], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[0]);
	m_pBlueEmblem = NetResourceManager::Instance()->GetEmblem983(
		Doc()->m_roomInfo.GuildInfo.nGuildID[1], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[1]);
	memset(m_score, 0, sizeof(m_score));
	m_pScoreInfo->SetVisible(true);
}

void FrRankForm3::OnScoreInfoOwnerDraw()
{
	// 983 (0077d9e0): x/y do retângulo somados em double; largura/altura sem sinal direto no WRect.
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	float x = m_pScoreInfo->GetRect().x;
	float y = m_pScoreInfo->GetRect().y;

	if (m_pRedEmblem)
		pGDI->DrawTexture(m_pRedEmblem,
			WRect(WPoint(x + 10.0, y + 1.0), m_pRedEmblem->Width(),
				m_pRedEmblem->Height()),
			0xFFFFFFFF, 0);
	if (m_pBlueEmblem)
		pGDI->DrawTexture(m_pBlueEmblem,
			WRect(WPoint(x + 260.0, y + 1.0), m_pBlueEmblem->Width(),
				m_pBlueEmblem->Height()),
			0xFFFFFFFF, 0);

	y += 3.0;
	DrawDigit(pGDI, MakeStr("%d", m_score[0]), WPoint(x + 50.0, y),
		0xFFFF0000, (FrGDI::eTextAlign)1, false);
	DrawDigit(pGDI, MakeStr("%d", m_score[1]), WPoint(x + 240.0, y),
		0xFF0000FF, (FrGDI::eTextAlign)1, false);
}

void FrRankForm3::LoadBitmaps()
{
	if (Doc()->m_golfGame.gameType == 6)
	{
		for (int i = 0; i < 10; ++i)
			m_pDigit[i] = g_pFresh->GetManager()->GetBitmap("SCORECARD",
				MakeStr("%d", i));
		m_pPlus = g_pFresh->GetManager()->GetBitmap("SCORECARD", "+");
		m_pMinus = g_pFresh->GetManager()->GetBitmap("SCORECARD", "-");
	}
}

IMPLEMENT_OBJECT(FrRankFormEx, FrForm)

BEGIN_FRESH_MSGMAP(FrRankFormEx, FrForm)

ON_FRESH_VI("stage", FRCMD_INIT, FrRankFormEx::OnStageAreaInit)
ON_FRESH_VV("stage", FRCMD_OWNERDRAW, FrRankFormEx::OnStageAreaOwnerDraw)
ON_FRESH_VI("list", FRCMD_INIT, FrRankFormEx::OnRankInit)
ON_FRESH_VI("list", FRCMD_OWNERDRAW, FrRankFormEx::OnRankOwnerDraw)
ON_FRESH_VI("billboard", FRCMD_INIT, FrRankFormEx::OnBoardInit)

END_FRESH_MSGMAP()

CRankExData::CRankExData()
	: m_pRival(NULL),
	  m_motionLength(3.0f),
	  m_time(0.0f),
	  m_leader(-1),
	  m_showLeader(-1),
	  m_pList(NULL),
	  m_pStage(NULL),
	  m_pBoard(NULL)
{
}

CRankExData::~CRankExData()
{
}

void CRankExData::InitAreaStage(FrArea* pArea)
{
	m_pStage = pArea;
}

void CRankExData::Clear()
{
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
	// 983 0077bb70: ClearRival inlinado
	if (m_pRival)
	{
		delete[] m_pRival;
		m_pRival = NULL;
	}
}

void CRankExData::OnProc(float deltaTime)
{
	m_time += deltaTime;
	if (m_leader == -1)
		SetLeader();
	else if (m_showLeader != -1)
		ShowLeader();
	else if (m_pExhibition)
		m_pExhibition->Process(deltaTime, false);
}

void CRankExData::CreateLeaderPet(const WRect& rect)
{
	m_pExhibition = new CExhibitionByHardwareTnL(rect.x + rect.w * 0.5f,
		rect.y + rect.h * 0.5f, 0.3f, false);
	m_pExhibition->SetDistance(22.5f /* 983: constante 00d2417c */, true, false);
	m_pExhibition->SetArea(rect.w, rect.h);
}

void CRankExData::SetActive(bool active)
{
	m_showLeader = -1;
	m_leader = -1;
	m_motionLength = 3.0f;
	m_time = 0.0f;
	if (active)
		SetLeader();
}

bool CRankExData::IsMotionFinished()
{
	if (0)
		((CRankExData*)0)->CRankExData::CreateLeaderPet(*(const WRect*)0);
	float limit = m_motionLength + 1.0f;
	if (limit > 10.0f)
		limit = 10.0f;
	return m_time > limit;
}

void CRankExData::ResetStage()
{
	if (m_pExhibition && m_pStage)
	{
		const WRect& rect = m_pStage->GetRect();
		m_pExhibition->SetCenter(rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f);
		m_pExhibition->SetDistance(22.5f /* 983: constante 00d2417c */, true, false);
	}
}

void CRankExData::InitRivalData()
{
	if (0)
		((FrRankFormEx*)0)->FrRankFormEx::SortRank();
	int count = Doc()->m_rivalList.size();
	m_pRival = new sRivalData[count];
	for (int i = 0; i < count; ++i)
	{
		m_pRival[i] = Doc()->m_rivalList[i];
		m_pList->AddItem(&m_pRival[i]);
	}
}

void CRankExData::InitMedalData()
{
	m_pMedalBmp[0] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_gold");
	m_pMedalBmp[1] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_silver");
	m_pMedalBmp[2] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_bronze");
	m_pMedalBmp[3] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, "rank_noaward");
	m_pTeamRankBmp[0] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_REDTEAM_RANK);
	m_pTeamRankBmp[1] =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_BLUETEAM_RANK);
	m_pRankBgBmp =
		g_pFresh->GetManager()->GetBitmap(RB_STR_FRAMES4, RB_STR_30IN_RANK);
	m_pad983_40 = 0;
	m_pZeroIcon = g_pFresh->GetManager()->GetBitmap("30s", "icon_zero");
	m_pPremiumIcon = g_pFresh->GetManager()->GetBitmap("30s", "premium_score");
}

// 983 0078ccb0 / 0078da70 (uma copia por lista): medalha pelo oid; quem saiu cedo (state 3 ate o 4o buraco) nao conta
static unsigned __stdcall MedalByOid983(unsigned long oid)
{
	if (Doc()->m_rivalList.size() < 10)
		return 3;
	unsigned char idx = Doc()->GetIndex(oid);
	if (idx != 0xff)
	{
		sRivalData& me = Doc()->m_rivalList[idx];
		if (me.flag <= 3)
		{
			unsigned char rank = me.rank, count = 0, ahead = 0;
			for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
				it != Doc()->m_rivalList.end(); ++it)
			{
				if (it->state != 3 || GetHoleIndex(it->hole) >= 4)
				{
					++count;
					if (it->state != 3 && it->rank < rank && it->flag > 3)
						++ahead;
				}
			}
			return MedalByCountRank(count, (unsigned char)(rank - ahead));
		}
	}
	return 3;
}

// 983 0078c0f0 (FrRankForm::OnRankOwnerDraw): com intrusao a contagem vem dos membros e ninguem e pulado; NPC do Grand Prix
// e achado pelo nickname (sem achar fica o indice 0, como no 983)
static unsigned __stdcall MedalByRival983(const sRivalData* pRival)
{
	unsigned char members = IsLocalContent((localContentType_t)47) ? CIntrusion::Instance()->GetCalcExpMemberSize() : 0;
	if ((members ? members : Doc()->m_rivalList.size()) < 10)
		return 3;
	unsigned char idx = 0;
	if ((unsigned)(Doc()->m_roomInfo.realGameType - 0x14) < 2 && pRival->bGrandPrixAI983)
	{
		for (unsigned i = 0; i < Doc()->m_rivalList.size(); ++i)
		{
			if (strncmp(Doc()->m_rivalList[i].nickname, pRival->nickname, 0x16) == 0)
			{
				idx = (unsigned char)i;
				break;
			}
		}
	}
	else
	{
		idx = Doc()->GetIndex(pRival->oid);
		if (idx == 0xff)
			return 3;
	}
	sRivalData& me = Doc()->m_rivalList[idx];
	if (me.flag > 3)
		return 3;
	unsigned char rank = me.rank, count = 0, ahead = 0;
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
	{
		if (members == 0 && it->state == 3 && GetHoleIndex(it->hole) <= 3)
			continue;
		++count;
		if (it->state != 3 && it->rank < rank && it->flag > 3)
			++ahead;
	}
	return MedalByCountRank(members ? members : count, (unsigned char)(rank - ahead));
}

// 983 0059fc40 (Grand Prix, gptyperoomdlg): com intrusao a contagem vem dos membros e ninguem e pulado
int __fastcall GetMedalIndex983(unsigned long oid)
{
	unsigned char members = IsLocalContent((localContentType_t)47) ? CIntrusion::Instance()->GetCalcExpMemberSize() : 0;
	if (Doc()->m_rivalList.size() < 10)
		return 3;
	unsigned char idx = Doc()->GetIndex(oid);
	if (idx == 0xff || Doc()->m_rivalList[idx].flag > 3)
		return 3;
	unsigned char rank = Doc()->m_rivalList[idx].rank, count = 0, ahead = 0;
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
	{
		if (members == 0 && it->state == 3 && GetHoleIndex(it->hole) <= 3)
			continue;
		++count;
		if (it->state != 3 && it->rank < rank && it->flag > 3)
			++ahead;
	}
	return MedalByCountRank(members ? members : count, (unsigned char)(rank - ahead));
}

void CRankExData::DrawRankItem(FrListItem* pItem)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	int space = pGDI->SetSpace(0);
	sRivalData* pRival = (sRivalData*)pItem->pData;

	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xFFFFFFFF, 0xFFFFFFFF);

	if (Doc()->m_golfGame.gameType == 5)
	{
		if (m_pTeamRankBmp[pRival->team])
			pGDI->DrawTexture(m_pTeamRankBmp[pRival->team],
				WRect(pItem->pos.x + 12.0f, pItem->pos.y + 2.0f,
					(float)m_pTeamRankBmp[pRival->team]->Width(),
					(float)m_pTeamRankBmp[pRival->team]->Height()),
				0xFFFFFFFF, 0);
		pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1, "%d",
			pRival->rank);
	}
	else
	{
		if (m_pRankBgBmp)
			pGDI->DrawTexture(m_pRankBgBmp,
				WRect(pItem->pos.x + 12.0f, pItem->pos.y + 2.0f,
					(float)m_pRankBgBmp->Width(),
					(float)m_pRankBgBmp->Height()),
				0xFFFFFFFF, 0);
		if (pRival->state == 3)
			pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1,
				"-");
		else
			pGDI->Print(WPoint(pItem->pos.x + 21.0f, pItem->pos.y + 3.0f), 1,
				"%d", pRival->rank);
	}

	if ((Doc()->m_roomInfo.gameType == 11 || !Doc()->m_roomInfo.bAdminMaster ||
			Doc()->m_roomInfo.nUserLimit < 100) &&
		pRival->hole != Doc()->m_holeOrder[0])
	{
		unsigned medal = MedalByOid983(pRival->oid);
		if (medal < 3)
		{
			if (pRival->flag > 3)
				medal = 3;
			pGDI->DrawTexture(m_pMedalBmp[medal],
				WRect(pItem->pos.x + 2.0f, pItem->pos.y + 1.0f,
					(float)m_pMedalBmp[medal]->Width(),
					(float)m_pMedalBmp[medal]->Height()),
				0xFFFFFFFF, 0);
		}
	}

	unsigned long myUid = ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		? Doc()->m_myInfo.info.dwGalleryGuid
		: Doc()->m_myInfo.info.dwGuid;
	// 983 (0078db80): cor escolhida em if/else, uma chamada so.
	unsigned long color;
	if (pRival->oid == myUid)
	{
		if (Doc()->m_golfGame.gameType == 5)
			color = pRival->team ? 0xFF0060FF : 0xFFFF0000;
		else
			color = 0xFFFF0000;
	}
	else if (pRival->state == 3)
		color = 0xFF808080;
	else
		color = 0xFF000000;
	pGDI->SetTextColor(color, 0xFFFFFFFF);

	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pLevel;
	if (pRival->titleTypeId &&
		(pSkin = ItemManager()->FindSkin(pRival->titleTypeId)) != NULL)
		pLevel = g_pFresh->GetManager()->GetBitmap("TITLES", pSkin->c.Icon);
	else
		pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS",
			MakeStr("level_%03d", pRival->level + 1));
	if (pRival->capability & 0x8000)
		pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");
	if (pLevel)
	{
		// 983: meias medidas em float antes do (int); x = pos.x + 35.0 + int.
		WRect rc;
		rc.w = (float)pLevel->Width();
		rc.h = (float)pLevel->Height();
		float hx = (float)((60.0 - rc.w) * 0.5);
		rc.x = (float)(pItem->pos.x + 35.0 + (int)hx);
		float hy = (float)(rc.h * 0.5);
		rc.y = (float)(pItem->pos.y + 8.0 - (int)hy);
		pGDI->DrawTexture(pLevel, rc, 0xFFFFFFFF, 0);
	}

	if (pRival->guildUID)
	{
		const Bitmap* pEmblem =
			NetResourceManager::Instance()->GetEmblem983(pRival->guildUID, pRival->guildEmblemVer983);
		if (pEmblem)
			pGDI->DrawTexture(pEmblem,
				WRect(pItem->pos.x + 100.0f, pItem->pos.y + 3.0f - 6.0f,
					(float)pEmblem->Width(), (float)pEmblem->Height()),
				0xFFFFFFFF, 0);
	}

	if (CProjectG::Instance()->HidePrivacy() == false)
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 127.0f, pItem->pos.y + 3.0f), 0,
			pRival->nickname, 90.0f, 0xFFFFFFFF);

	pGDI->Print(WPoint(pItem->pos.x + 240.0f, pItem->pos.y + 3.0f), 2, "%d",
		pRival->totalScore);

	pGDI->Print(WPoint(pItem->pos.x + 285.0f, pItem->pos.y + 3.0f), 2, "%I64d",
		pRival->totalPang);

	if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
		pGDI->Print(WPoint(pItem->pos.x + 323.0f, pItem->pos.y + 3.0f), 2,
			"End");
	else if (pRival->state == 3)
		pGDI->Print(WPoint(pItem->pos.x + 323.0f, pItem->pos.y + 3.0f), 2,
			"Out");
	else
		pGDI->Print(WPoint(pItem->pos.x + 323.0f, pItem->pos.y + 3.0f), 2,
			K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pRival->hole));

	if ((pRival->capability & 0x4000) && m_pPremiumIcon)
		pGDI->DrawTexture(m_pPremiumIcon,
			WRect(pItem->pos.x + 329.0f, pItem->pos.y - 3.0f,
				(float)m_pPremiumIcon->Width(), (float)m_pPremiumIcon->Height()),
			0xFFFFFFFF, 0);
	else if (m_pZeroIcon)
		pGDI->DrawTexture(m_pZeroIcon,
			WRect(pItem->pos.x + 331.0f, pItem->pos.y + 6.0f,
				(float)m_pZeroIcon->Width(), (float)m_pZeroIcon->Height()),
			0xFFFFFFFF, 0);

	pGDI->SetSpace(space);
}

// HACK: stands for direct reads of WView::SCREEN_XS / SCREEN_YS (protected; GetWidth() / GetHeight() return
// by value and are loaded too early); dllimport + forceinline is always inlined and never emitted out of line.
struct RbViewSize : public WView
{
	__declspec(dllimport) __forceinline const float& Width() const
	{
		return SCREEN_XS;
	}
	__declspec(dllimport) __forceinline const float& Height() const
	{
		return SCREEN_YS;
	}
};

void CRankExData::DrawStageArea()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;
	if (m_leader == -1 || m_showLeader != -1 || m_pExhibition == NULL)
		return;

	m_pExhibition->Display(0.0f, 0.0f);
	if (m_pExhibition->GetPetFrame() == NULL)
		return;
	WBone* pHead =
		m_pExhibition->GetPetFrame()->GetBonePet()->FindBone("Bip01 Head");
	if (pHead == NULL)
		return;

	const WVector& headPos = pHead->m_matrix.pivot;
	const WVector2D& center = m_pExhibition->GetCenter();
	// 983 0078e250: FOV fixo de 90° e projeção perspectiva (repostos no fim)
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	float fov = g_view->GetFOV();
	g_view->SetProjectionMode(WView::PERSPECTIVE);
	g_view->SetFOV(1.5707964f);
	g_view->UpdateCamera();

	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pLevel;
	if (Doc()->m_rivalList[Doc()->GetIndex(m_leader)].titleTypeId &&
		(pSkin = ItemManager()->FindSkin(
			 Doc()->m_rivalList[Doc()->GetIndex(m_leader)].titleTypeId)) !=
			NULL)
		pLevel = g_pFresh->GetManager()->GetBitmap("TITLES", pSkin->c.Icon);
	else
		pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS",
			MakeStr("level_%03d",
				Doc()->m_rivalList[Doc()->GetIndex(m_leader)].level + 1));
	// 983 0078e250: capability bit 15 (host do Grand Prix) troca o icone por gp_host
	if (Doc()->m_rivalList[Doc()->GetIndex(m_leader)].capability & 0x8000)
		pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");

	if (pLevel)
	{
		WVector screen = g_view->Projection(headPos);
		if (0.0f < screen.z && screen.z < 1.0f)
		{
			sRivalData* pRival = &Doc()->m_rivalList[Doc()->GetIndex(m_leader)];
			char* name = pRival->nickname;
			float textWidth = pGDI->GetTextExtendOld983(name);
			WSize size((float)pLevel->Width(), (float)pLevel->Height());
			WPoint pos;
			pos.x = (float)(int)(float)(screen.x + center.x -
				(size.w + ((RbViewSize*)g_view)->Width() + textWidth + 5.0f) *
					0.5f);
			pos.y = (float)(int)(float)(screen.y - 52.0f + center.y -
				((RbViewSize*)g_view)->Height() * 0.5f);
			pGDI->DrawTexture(pLevel,
				WRect(pos.x, pos.y + 8.0f - (int)(float)(size.h * 0.5f), size.w,
					size.h),
				0xFFFFFFFF, 0);

			WPoint textPos(pos.x + size.w + 5.0f, pos.y + 2.0f);
			pGDI->SetTextColor(0xFFFFFFFF, 0xFF000000);
			pGDI->SetTextStyle(2);
			if (pRival->guildUID)
			{
				const Bitmap* pEmblem =
					NetResourceManager::Instance()->GetEmblem983(
						pRival->guildUID, pRival->guildEmblemVer983);
				if (pEmblem)
					pGDI->DrawTexture(pEmblem,
						WRect(textPos.x, textPos.y - 6.0f,
							(float)pEmblem->Width(), (float)pEmblem->Height()),
						0xFFFFFFFF, 0);
				textPos.x += 27.0f;
			}
			if (CProjectG::Instance()->HidePrivacy() == false)
				g_pFresh->GetManager()->PrintText(textPos, 0, name, -1.0f,
					0xFFFFFFFF);
		}
	}

	g_view->SetProjectionMode(oldMode);
	g_view->SetFOV(fov);
	g_view->UpdateCamera();
}

void CRankExData::SetLeader()
{
	// 983 (0078e6d0): sem Doc() guardado; o tamanho da lista sai sempre do mesmo Doc.
	// O 983 soma begin + MyIndex()*0x1b8 depois do call (begin() + i, nao operator[]).
	if (GetHoleIndex((Doc()->m_rivalList.begin() + MyIndex())->hole) >
			GetHoleIndex(Doc()->m_pGolfDoc->m_currentHole) ||
		!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		std::vector<sRivalData>& rivals = Doc()->m_rivalList;
		int i;
		for (i = 0; i < (int)rivals.size(); i++)
		{
			sRivalData& src = rivals[i];
			m_pRival[i].state = src.state;
			m_pRival[i].totalPang = src.totalPang;
			m_pRival[i].totalScore = src.totalScore;
			m_pRival[i].hole = src.hole;
			m_pRival[i].rank = src.rank;
			m_pRival[i].quitOrder = src.quitOrder;
			m_pRival[i].finishOrder = src.finishOrder;
		}
		m_pList->SortItem(RankCompare);

		int count = 0;
		for (i = 0; i < (int)rivals.size(); i++)
		{
			if (m_pRival[i].rank == 1)
				count++;
		}
		if (count == 0)
			return;

		int pick = rand() % count;
		int n = (int)rivals.size();
		count = 0;
		for (i = 0; i < n; i++)
		{
			if (m_pRival[i].rank == 1)
			{
				if (count == pick)
				{
					m_leader = m_pRival[i].oid;
					n = i;
					break;
				}
				count++;
			}
		}
		i = n;
		if (m_leader == -1)
			return;

		if (!Doc()->m_userInfoMod)
		{
			std::map<unsigned long, sUserInfoTime>::iterator it;
			if (!Doc()->FindUserInfoTimeUID(m_pRival[i].uid, it))
			{
				WSendPacket packet(0x2f);
				packet.Encode4(m_pRival[i].uid);
				packet.Encode1(5);
				packet.Send(TO_GAME);
				Doc()->m_userInfoMod = 1;
			}
		}
		ShowLeader();

		char text[256];
		sprintf(text,
			K2L_Compatibility("\xc7\xf6\xc0\xe7 1\xc0\xa7 %s\xb4\xd4, \xbd\xba\xc4\xda\xbe\xee %d"),
			m_pRival[i].nickname, m_pRival[i].totalScore);
		m_pBoard->SetCaption(text);
	}
}

void CRankExData::ShowLeader()
{
	unsigned long leader = m_showLeader;
	std::map<unsigned long, sUserInfoTime>::iterator it;
	if (leader == -1)
		leader = m_leader;

	CSharedDoc* pDoc = Doc();
	std::map<unsigned long, sBriefUserInfo>::iterator brief =
		pDoc->m_briefUserInfoMap.find(leader);
	if (brief == pDoc->m_briefUserInfoMap.end() ||
		!pDoc->FindUserInfoTimeUID(brief->second.dwUid, it))
	{
		if (m_showLeader == -1)
		{
			m_showLeader = m_leader;
			return;
		}
	}
	else if (Doc()->m_userInfoMod != 1)
	{
		m_showLeader = -1;
		CPartTidList parts;
		sCharacterInfo* pCharInfo = &it->second.info.charInfo;
		parts.SetTids(pCharInfo, 255);
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(pCharInfo->tid);
		if (m_pExhibition)
		{
			m_pExhibition->SetDistance(22.5f /* 983: constante 00d2417c */, true,
				false);
			// 983 00789360: um literal so para a motion padrao (d18300); "1st_place" e
			// os nomes desconhecidos tambem caem nela.
			static const char s_first[] = "1\xb5\xee\xb8\xf0\xbc\xc7";
			if (IsLocalCountry(30))
			{
				if (!pChar || !pChar->Mtn30sWinner[0])
					m_pExhibition->SetModel(parts, pCharInfo->tidAuxParts, s_first, 0.5f, leader);
				else if (!_stricmp(pChar->Mtn30sWinner, "1st_place"))
					m_pExhibition->SetModel(parts, pCharInfo->tidAuxParts, s_first, 0.5f, leader);
				else if (!_stricmp(pChar->Mtn30sWinner, "2nd_place"))
					m_pExhibition->SetModel(parts, pCharInfo->tidAuxParts,
						"2\xb5\xee\xb8\xf0\xbc\xc7", 0.5f, leader);
				else
					m_pExhibition->SetModel(parts, pCharInfo->tidAuxParts, s_first, 0.5f, leader);
			}
			else
				m_pExhibition->SetModel(parts, pCharInfo->tidAuxParts,
					(pChar && pChar->Mtn30sWinner[0]) ? pChar->Mtn30sWinner : s_first,
					0.5f, leader);
			m_motionLength = m_pExhibition->GetInitialMotionLength();
		}
	}
}

void CRankExData::ClearRival()
{
	if (m_pRival)
	{
		delete[] m_pRival;
		m_pRival = NULL;
	}
}

void CRankExData::SetRivalData()
{
	if (0)
		((FrRankFormEx*)0)->FrRankFormEx::SortRank();
	int count = Doc()->m_rivalList.size();
	m_pRival = new sRivalData[count];
	m_pList->ClearItem();
	for (int i = 0; i < count; ++i)
	{
		m_pRival[i] = Doc()->m_rivalList[i];
		m_pList->AddItem(&m_pRival[i]);
	}
}

FrRankFormEx::FrRankFormEx()
{
}

FrRankFormEx::~FrRankFormEx()
{
}

void FrRankFormEx::OnStageAreaInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
	{
		char name[64] = "";
		sprintf(name, "30in_back_%s",
			Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL);
		pArea->SetBgImg(name);
		m_data.InitAreaStage(pArea);
		m_data.CreateLeaderPet(pArea->GetRect());
	}
}

void FrRankFormEx::OnStageAreaOwnerDraw()
{
	if (IsVisible())
	{
		char name[64] = "";
		sprintf(name, "30in_back_%s",
			Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL);
		m_data.m_pStage->SetBgImg(name);
		m_data.DrawStageArea();
	}
}

void FrRankFormEx::OnRankInit(int param)
{
	m_data.m_pList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_data.InitRivalData();
	m_data.InitMedalData();
}

void FrRankFormEx::SortRank()
{
	CSharedDoc* pDoc = Doc();
	memset(pDoc->m_teamResult, 0, sizeof(pDoc->m_teamResult));
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
	{
		if (it->state == 3)
		{
			it->rank = Doc()->m_rivalList.size() - it->quitOrder;
		}
		else
		{
			it->rank = 1;
			for (std::vector<sRivalData>::iterator it2 =
					 Doc()->m_rivalList.begin();
				it2 != Doc()->m_rivalList.end(); ++it2)
			{
				if (it == it2 || it2->state == 3)
					continue;

				if (it->totalScore > it2->totalScore ||
					(it->totalScore == it2->totalScore &&
						it->totalPang < it2->totalPang))
					it->rank++;
			}
			Doc()->m_teamResult[it->team].score += it->rank;
			Doc()->m_teamResult[it->team].pang += it->totalPang;
		}
	}
	m_data.m_pList->SortItem(RankCompare);
}

sRivalData* FrRankFormEx::GetRival()
{
	return m_data.m_pRival;
}

void FrRankFormEx::OnRankOwnerDraw(int param)
{
	if (param)
		m_data.DrawRankItem((FrListItem*)param);
}

void FrRankFormEx::OnBoardInit(int param)
{
	m_data.m_pBoard = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_data.m_pBoard->HidePrivacy(true);
}

void FrRankFormEx::Clear()
{
	m_data.Clear();
}

void FrRankFormEx::SetActive(bool active)
{
	m_data.SetActive(active);
	Enable(active);
	SetVisible(active);
}

void FrRankFormEx::SetLeader()
{
	m_data.SetLeader();
}

void FrRankFormEx::ShowLeader()
{
	m_data.ShowLeader();
}

bool FrRankFormEx::OnInit()
{
	Enable(false);
	SetVisible(false);
	return true;
}

void FrRankFormEx::OnProc(const float deltaTime)
{
	// 983 007927e0: alem de avancar o CRankExData, a lista rola com PAGE_UP/PAGE_DOWN
	// (a cada 0,05 s) e HOME volta para a linha do jogador (788ad0/788a20)
	if (!IsVisible())
		return;

	m_data.OnProc(deltaTime);
	FrListBox* list = m_data.m_pList;
	if (list->GetCurrentItemSize(true) <=
		(int)list->GetRect().h / list->GetItemHeight())
		return;

	static float s_keyTimer = 0.0f;
	s_keyTimer += deltaTime;
	if (s_keyTimer < 0.05f)
		return;
	s_keyTimer = 0.0f;

	if (g_input->GetDown("PAGE_UP", false))
		list->GetScrollBar()->ScrollUp(1);
	else if (g_input->GetDown("PAGE_DOWN", false))
		list->GetScrollBar()->ScrollDown(1);

	if (g_input->GetDown("HOME", false))
	{
		std::list<FrListItem*>& items = list->m_itemList;
		int row = 0;
		for (std::list<FrListItem*>::iterator it = items.begin();
			it != items.end(); ++it)
		{
			unsigned long myUid =
				((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
				? Doc()->m_myInfo.info.dwGalleryGuid
				: Doc()->m_myInfo.info.dwGuid;
			if (((sRivalData*)(*it)->pData)->oid == myUid)
				break;
			row++;
		}
		if (list->GetScrollBar())
		{
			int maxRow = Doc()->m_rivalList.size() - 10;
			row -= 4;
			if (row < 0)
				row = 0;
			else if (row > maxRow)
				row = maxRow;
			list->GetScrollBar()->SetCurTopRow(row);
		}
	}
}

bool FrRankFormEx::IsMotionFinished()
{
	return m_data.IsMotionFinished();
}

void FrRankFormEx::ResetStage()
{
	m_data.ResetStage();
}

void __stdcall SortPlayerOrder(int* order)
{
	if (Doc()->m_golfGame.gameType == 1)
	{
		if (Doc()->m_pGolfDoc->GetTeam(0)->score >
				Doc()->m_pGolfDoc->GetTeam(1)->score ||
			(Doc()->m_pGolfDoc->GetTeam(0)->score ==
					Doc()->m_pGolfDoc->GetTeam(1)->score &&
				(int)Doc()->m_pGolfDoc->GetTeam(1)->totalPang <=
					(int)Doc()->m_pGolfDoc->GetTeam(0)->totalPang))
		{
			order[0] = 0;
			order[1] = 1;
		}
		else
		{
			order[0] = 1;
			order[1] = 0;
		}
	}
	// 983 785670: sem ramo para gameType 3 (cai no else generico por ordem)
	else if (Doc()->m_golfGame.gameType == 7)
	{
		int count = 0;
		int rank;
		int i;
		for (rank = 0; rank < Doc()->m_pGolfDoc->m_playerNum; rank++)
		{
			for (i = 0; i < Doc()->m_pGolfDoc->m_playerNum; i++)
			{
				if (Doc()->m_pGolfDoc->GetPlayer(i)->state != 3 &&
					Doc()->m_pGolfDoc->GetPlayer(i)->rank == rank + 1)
					order[count++] = i;
			}
		}
		for (i = 0; i < Doc()->m_pGolfDoc->m_playerNum; i++)
		{
			if (Doc()->m_pGolfDoc->GetPlayer(i)->state == 3)
				order[count++] = i;
		}
	}
	else if (Doc()->m_golfGame.gameType == 6)
	{
		order[0] = 0;
		order[1] = 1;
	}
	else
	{
		for (unsigned char i = 0; i < Doc()->m_pGolfDoc->m_playerNum; i++)
		{
			if (Doc()->m_pGolfDoc->GetPlayer(i)->state != 3 ||
				(Doc()->m_gameMode != 1 && Doc()->m_gameMode != 5))
				order[Doc()->m_pGolfDoc->GetPlayer(i)->order - 1] = i;
		}
	}
}

IMPLEMENT_OBJECT(FrAwardForm, FrForm)

BEGIN_FRESH_MSGMAP(FrAwardForm, FrForm)

ON_FRESH_VI("tab0", FRCMD_INIT, FrAwardForm::OnTab0Init)
ON_FRESH_VI("tab1", FRCMD_INIT, FrAwardForm::OnTab1Init)
ON_FRESH_VI("userlist", FRCMD_INIT, FrAwardForm::OnUserListInit)
ON_FRESH_VI("userlist", FRCMD_OWNERDRAW, FrAwardForm::OnUserListOwnerDraw)
ON_FRESH_VI("expbar0", FRCMD_INIT, FrAwardForm::OnExpBar0Init)
ON_FRESH_VI("expbar1", FRCMD_INIT, FrAwardForm::OnExpBar1Init)
ON_FRESH_VI("expbar2", FRCMD_INIT, FrAwardForm::OnExpBar2Init)
ON_FRESH_VI("expbar3", FRCMD_INIT, FrAwardForm::OnExpBar3Init)
ON_FRESH_VI("exp0", FRCMD_INIT, FrAwardForm::OnExp0Init)
ON_FRESH_VI("exp1", FRCMD_INIT, FrAwardForm::OnExp1Init)
ON_FRESH_VI("exp2", FRCMD_INIT, FrAwardForm::OnExp2Init)
ON_FRESH_VI("exp3", FRCMD_INIT, FrAwardForm::OnExp3Init)
ON_FRESH_VI("expend0", FRCMD_INIT, FrAwardForm::OnExpEnd0Init)
ON_FRESH_VI("expend1", FRCMD_INIT, FrAwardForm::OnExpEnd1Init)
ON_FRESH_VI("expend2", FRCMD_INIT, FrAwardForm::OnExpEnd2Init)
ON_FRESH_VI("expend3", FRCMD_INIT, FrAwardForm::OnExpEnd3Init)
ON_FRESH_VI("level0", FRCMD_INIT, FrAwardForm::OnLevel0Init)
ON_FRESH_VI("level1", FRCMD_INIT, FrAwardForm::OnLevel1Init)
ON_FRESH_VI("level2", FRCMD_INIT, FrAwardForm::OnLevel2Init)
ON_FRESH_VI("level3", FRCMD_INIT, FrAwardForm::OnLevel3Init)
ON_FRESH_VI("levelup0", FRCMD_INIT, FrAwardForm::OnLevelUp0Init)
ON_FRESH_VI("levelup1", FRCMD_INIT, FrAwardForm::OnLevelUp1Init)
ON_FRESH_VI("levelup2", FRCMD_INIT, FrAwardForm::OnLevelUp2Init)
ON_FRESH_VI("levelup3", FRCMD_INIT, FrAwardForm::OnLevelUp3Init)
ON_FRESH_VI("match_res0", FRCMD_INIT, FrAwardForm::OnMatchRes0Init)
ON_FRESH_VI("match_res1", FRCMD_INIT, FrAwardForm::OnMatchRes1Init)
ON_FRESH_VI("GainedLadderPoint0", FRCMD_INIT,
	FrAwardForm::OnGainedLadderPoint0Init)
ON_FRESH_VI("GainedLadderPoint1", FRCMD_INIT,
	FrAwardForm::OnGainedLadderPoint1Init)
ON_FRESH_VI("LadderPoint0", FRCMD_INIT, FrAwardForm::OnLadderPoint0Init)
ON_FRESH_VI("LadderPoint1", FRCMD_INIT, FrAwardForm::OnLadderPoint1Init)
ON_FRESH_VI("nextlevel", FRCMD_INIT, FrAwardForm::OnNextLevelTextInit)

END_FRESH_MSGMAP()

FrAwardForm::FrAwardForm()
{
	m_pSfxName = NULL;
	m_animTimer = 0;
	m_pUserList = NULL;
	m_userCount = 0;
	memset(m_pExpBar, 0, sizeof(m_pExpBar));
	memset(m_pExp, 0, sizeof(m_pExp));
	memset(m_pExpEnd, 0, sizeof(m_pExpEnd));
	memset(m_pLevel, 0, sizeof(m_pLevel));
	memset(m_pLevelUp, 0, sizeof(m_pLevelUp));
	memset(m_pMatchRes, 0, sizeof(m_pMatchRes));
	memset(m_order, 0, sizeof(m_order));
	memset(m_level, 0, sizeof(m_level));
	memset(m_expGain, 0, sizeof(m_expGain));
	memset(m_curExp, 0, sizeof(m_curExp));
	m_expDone = 0;
	memset(m_pGainedLadderPoint, 0, sizeof(m_pGainedLadderPoint));
	memset(m_pLadderPoint, 0, sizeof(m_pLadderPoint));
	m_pNextLevel = NULL;
	m_giftWaitTime = 0;
	m_pBmp[2] = 0;
	m_pBmp[3] = 0;
	m_pBmp[4] = 0;
	m_pBmp[5] = 0;
	m_pBmp[6] = 0;
	m_pBmp[7] = 0;
	m_pBmp[8] = 0;
	m_pBmp[9] = 0;
	m_pBmp[10] = 0;
}

FrAwardForm::~FrAwardForm()
{
}

bool FrAwardForm::Close(bool bFade)
{
	if (m_pSfxName)
		g_audio->StopSfx(m_pSfxName);
	return FrForm::Close(bFade);
}

C_ASSERT_VC10(offsetof(CGolfDoc, m_teamNum) == 0xd0); // A749: ex-cru m_pGolfDoc+0xd0

void FrAwardForm::OnUserListInit(int param)
{
	m_pUserList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (Doc()->m_golfGame.gameType == 1)
	{
		m_userCount = Doc()->m_pGolfDoc->m_playerNum;
		int mode = Doc()->m_pGolfDoc->m_teamNum;
		unsigned char myTeam = (unsigned char)(((mode == 2) - 1) & mode);
		for (int i = 0; i < (int)m_userCount; i++)
		{
			unsigned char index = (unsigned char)i;
			if (Doc()->m_pGolfDoc->GetPlayer(index)->teammate == 0xff)
			{
				if (Doc()->m_pGolfDoc->GetPlayer(index)->team == myTeam)
				{
					m_order[0] = i;
					Doc()->m_pGolfDoc->GetPlayer(index)->rank = 1;
					Doc()->m_pGolfDoc->GetPlayer(index)->order = 1;
				}
				else
				{
					m_order[1] = i;
					Doc()->m_pGolfDoc->GetPlayer(index)->rank = 2;
					Doc()->m_pGolfDoc->GetPlayer(index)->order = 2;
				}
			}
			else if (Doc()->m_pGolfDoc->GetPlayer(index)->team != myTeam)
			{
				m_order[2 +
					(Doc()->m_pGolfDoc->GetPlayer(index)->teammate <= i)] = i;
				Doc()->m_pGolfDoc->GetPlayer(index)->rank =
					(Doc()->m_pGolfDoc->GetPlayer(index)->teammate <= i) + 3;
				Doc()->m_pGolfDoc->GetPlayer(index)->order =
					Doc()->m_pGolfDoc->GetPlayer(index)->rank;
			}
			else
			{
				m_order[Doc()->m_pGolfDoc->GetPlayer(index)->teammate <= i] = i;
				Doc()->m_pGolfDoc->GetPlayer(index)->rank =
					(Doc()->m_pGolfDoc->GetPlayer(index)->teammate <= i) + 1;
				Doc()->m_pGolfDoc->GetPlayer(index)->order =
					Doc()->m_pGolfDoc->GetPlayer(index)->rank;
			}
		}
	}
	else
	{
		m_userCount = Doc()->m_pGolfDoc->m_playerNum;
		SortPlayerOrder(m_order);
	}
	for (int i = 0; i < (int)m_userCount; i++)
		m_pUserList->AddItem((void*)m_order[i]);
}

void FrAwardForm::OnTab0Init(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardForm::OnTab1Init(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardForm::OnExpBar0Init(int param)
{
	m_pExpBar[0] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pExpBar[0]->SetVisible(false);
}

void FrAwardForm::OnExpBar1Init(int param)
{
	m_pExpBar[1] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pExpBar[1]->SetVisible(false);
}

void FrAwardForm::OnExpBar2Init(int param)
{
	m_pExpBar[2] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pExpBar[2]->SetVisible(false);
}

void FrAwardForm::OnExpBar3Init(int param)
{
	m_pExpBar[3] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pExpBar[3]->SetVisible(false);
}

void FrAwardForm::OnExp0Init(int param)
{
	m_pExp[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExp[0]->SetVisible(false);
}

void FrAwardForm::OnExp1Init(int param)
{
	m_pExp[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExp[1]->SetVisible(false);
}

void FrAwardForm::OnExp2Init(int param)
{
	m_pExp[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExp[2]->SetVisible(false);
}

void FrAwardForm::OnExp3Init(int param)
{
	m_pExp[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExp[3]->SetVisible(false);
}

void FrAwardForm::OnExpEnd0Init(int param)
{
	m_pExpEnd[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExpEnd[0]->SetVisible(false);
}

void FrAwardForm::OnExpEnd1Init(int param)
{
	m_pExpEnd[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExpEnd[1]->SetVisible(false);
}

void FrAwardForm::OnExpEnd2Init(int param)
{
	m_pExpEnd[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExpEnd[2]->SetVisible(false);
}

void FrAwardForm::OnExpEnd3Init(int param)
{
	m_pExpEnd[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pExpEnd[3]->SetVisible(false);
}

void FrAwardForm::OnLevel0Init(int param)
{
	m_pLevel[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevel[0]->SetVisible(false);
}

void FrAwardForm::OnLevel1Init(int param)
{
	m_pLevel[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevel[1]->SetVisible(false);
}

void FrAwardForm::OnLevel2Init(int param)
{
	m_pLevel[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevel[2]->SetVisible(false);
}

void FrAwardForm::OnLevel3Init(int param)
{
	m_pLevel[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevel[3]->SetVisible(false);
}

void FrAwardForm::OnLevelUp0Init(int param)
{
	m_pLevelUp[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevelUp[0]->SetVisible(false);
}

void FrAwardForm::OnLevelUp1Init(int param)
{
	m_pLevelUp[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevelUp[1]->SetVisible(false);
}

void FrAwardForm::OnLevelUp2Init(int param)
{
	m_pLevelUp[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevelUp[2]->SetVisible(false);
}

void FrAwardForm::OnLevelUp3Init(int param)
{
	m_pLevelUp[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pLevelUp[3]->SetVisible(false);
}

void FrAwardForm::OnMatchRes0Init(int param)
{
	m_pMatchRes[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (Doc()->m_golfGame.gameType == 0)
		m_pMatchRes[0]->SetVisible(false);
}

void FrAwardForm::OnMatchRes1Init(int param)
{
	m_pMatchRes[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (Doc()->m_golfGame.gameType == 0)
		m_pMatchRes[1]->SetVisible(false);
}

void FrAwardForm::OnGainedLadderPoint0Init(int param)
{
	m_pGainedLadderPoint[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrAwardForm::OnGainedLadderPoint1Init(int param)
{
	m_pGainedLadderPoint[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrAwardForm::OnLadderPoint0Init(int param)
{
	m_pLadderPoint[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrAwardForm::OnLadderPoint1Init(int param)
{
	m_pLadderPoint[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrAwardForm::OnNextLevelTextInit(int param)
{
	if (m_pNextLevel == NULL)
		m_pNextLevel = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

// [mode 0..4][cnt 0..18][rank 0..10]; valores do 983 (00d23078; o 645 tinha outra tabela)
static const int s_ladderTable[1045] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 1, 2, 1, 3, 2, 2, 4, 4, 3, 2,
	3, 2, 3, 2, 4, 3, 3, 5, 5, 4, 3,
	7, 6, 7, 6, 9, 8, 7, 11, 10, 9, 8,
	10, 7, 10, 7, 12, 10, 9, 15, 13, 12, 10,
	12, 9, 12, 9, 15, 13, 11, 18, 16, 14, 12,
	14, 10, 14, 10, 17, 15, 13, 21, 19, 17, 15,
	16, 12, 16, 12, 20, 17, 14, 25, 23, 20, 17,
	18, 13, 18, 13, 23, 20, 16, 28, 26, 22, 19,
	20, 15, 20, 15, 25, 22, 18, 31, 29, 25, 22,
	23, 17, 23, 17, 28, 24, 20, 35, 32, 28, 24,
	25, 18, 25, 18, 31, 27, 22, 38, 35, 30, 26,
	27, 20, 27, 20, 34, 29, 24, 42, 38, 33, 28,
	29, 21, 29, 21, 36, 31, 26, 45, 41, 36, 31,
	31, 23, 31, 23, 39, 33, 28, 48, 44, 39, 33,
	33, 24, 33, 24, 42, 36, 30, 52, 47, 41, 35,
	36, 26, 36, 26, 45, 38, 32, 55, 50, 44, 37,
	38, 28, 38, 28, 47, 40, 34, 58, 53, 47, 40,
	40, 29, 40, 29, 50, 43, 36, 62, 56, 49, 42,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 1, 2, 1, 3, 3, 2, 4, 4, 3, 3,
	3, 2, 3, 2, 4, 4, 3, 5, 5, 4, 4,
	8, 6, 8, 6, 9, 8, 7, 12, 11, 9, 8,
	10, 8, 10, 8, 12, 11, 9, 15, 14, 12, 11,
	12, 9, 12, 9, 15, 13, 11, 19, 17, 15, 13,
	14, 11, 14, 11, 18, 16, 13, 22, 20, 18, 16,
	17, 12, 17, 12, 21, 18, 15, 26, 24, 21, 18,
	19, 14, 19, 14, 24, 20, 17, 29, 27, 24, 20,
	21, 16, 21, 16, 26, 23, 19, 33, 30, 26, 23,
	23, 17, 23, 17, 29, 25, 21, 36, 33, 29, 25,
	26, 19, 26, 19, 32, 28, 23, 40, 37, 32, 28,
	28, 21, 28, 21, 35, 30, 25, 43, 40, 35, 30,
	30, 22, 30, 22, 38, 33, 27, 47, 43, 38, 33,
	32, 24, 32, 24, 41, 35, 29, 50, 46, 41, 35,
	35, 26, 35, 26, 43, 37, 31, 54, 49, 43, 37,
	37, 27, 37, 27, 46, 40, 34, 57, 53, 46, 40,
	39, 29, 39, 29, 49, 42, 36, 61, 56, 49, 42,
	41, 30, 41, 30, 52, 45, 38, 65, 59, 52, 45,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 3, 3, 2, 5, 4, 3, 3,
	3, 3, 3, 3, 4, 4, 3, 6, 5, 4, 4,
	8, 6, 8, 6, 10, 9, 7, 12, 11, 10, 9,
	10, 8, 10, 8, 13, 11, 10, 16, 15, 13, 11,
	13, 10, 13, 10, 16, 14, 12, 19, 18, 16, 14,
	15, 11, 15, 11, 19, 16, 14, 23, 21, 19, 16,
	17, 13, 17, 13, 22, 19, 16, 27, 25, 22, 19,
	19, 15, 19, 15, 25, 21, 18, 30, 28, 25, 22,
	22, 16, 22, 16, 27, 24, 20, 34, 31, 28, 24,
	24, 18, 24, 18, 30, 26, 22, 38, 35, 31, 27,
	26, 20, 26, 20, 33, 29, 25, 41, 38, 34, 29,
	29, 22, 29, 22, 36, 32, 27, 45, 42, 37, 32,
	31, 23, 31, 23, 39, 34, 29, 49, 45, 40, 34,
	33, 25, 33, 25, 42, 37, 31, 52, 48, 43, 37,
	36, 27, 36, 27, 45, 39, 33, 56, 52, 46, 40,
	38, 28, 38, 28, 48, 42, 35, 60, 55, 49, 42,
	40, 30, 40, 30, 51, 44, 37, 63, 58, 52, 45,
	43, 32, 43, 32, 54, 47, 40, 67, 62, 55, 47,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 2, 3, 2, 4, 3, 3, 5, 5, 4, 3,
	4, 3, 4, 3, 5, 4, 4, 6, 6, 5, 4,
	8, 7, 8, 7, 11, 9, 8, 13, 12, 11, 10,
	11, 8, 11, 8, 14, 12, 10, 17, 16, 14, 12,
	13, 10, 13, 10, 17, 15, 13, 21, 19, 17, 15,
	16, 12, 16, 12, 20, 18, 15, 25, 23, 21, 18,
	18, 14, 18, 14, 23, 20, 18, 29, 27, 24, 21,
	21, 16, 21, 16, 26, 23, 20, 33, 30, 27, 24,
	23, 18, 23, 18, 30, 26, 22, 37, 34, 30, 27,
	26, 20, 26, 20, 33, 29, 25, 41, 38, 34, 30,
	28, 22, 28, 22, 36, 31, 27, 45, 41, 37, 33,
	31, 23, 31, 23, 39, 34, 29, 49, 45, 40, 35,
	33, 25, 33, 25, 42, 37, 32, 53, 49, 44, 38,
	35, 27, 35, 27, 45, 40, 34, 57, 52, 47, 41,
	38, 29, 38, 29, 49, 43, 37, 61, 56, 50, 44,
	40, 31, 40, 31, 52, 45, 39, 65, 60, 53, 47,
	43, 33, 43, 33, 55, 48, 41, 69, 63, 57, 50,
	45, 35, 45, 35, 58, 51, 44, 72, 67, 60, 53,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	3, 2, 3, 2, 4, 4, 3, 6, 5, 5, 4,
	4, 3, 4, 3, 5, 5, 4, 7, 6, 6, 5,
	9, 7, 9, 7, 12, 10, 9, 14, 13, 12, 11,
	12, 9, 12, 9, 15, 13, 12, 19, 17, 16, 14,
	14, 11, 14, 11, 19, 17, 15, 23, 22, 20, 18,
	17, 14, 17, 14, 22, 20, 17, 27, 26, 23, 21,
	20, 16, 20, 16, 26, 23, 20, 32, 30, 27, 24,
	23, 18, 23, 18, 29, 26, 23, 36, 34, 31, 28,
	25, 20, 25, 20, 33, 29, 25, 41, 38, 34, 31,
	28, 22, 28, 22, 36, 32, 28, 45, 42, 38, 34,
	31, 24, 31, 24, 40, 35, 31, 50, 46, 42, 37,
	33, 26, 33, 26, 43, 38, 33, 54, 50, 46, 41,
	36, 28, 36, 28, 47, 41, 36, 58, 55, 49, 44,
	39, 30, 39, 30, 50, 44, 39, 63, 59, 53, 47,
	41, 32, 41, 32, 54, 48, 42, 67, 63, 57, 51,
	44, 34, 44, 34, 57, 51, 44, 72, 67, 60, 54,
	47, 37, 47, 37, 61, 54, 47, 76, 71, 64, 57,
	49, 39, 49, 39, 64, 57, 50, 80, 75, 68, 61,
};

// unused: the original emitted these wmath constants right after its tables
static bool IsValidAngle(float f)
{
	return f * g_DEGTORAD < g_HUGE && f > g_EPSILON;
}

#include "golfdocofs983.h" // B1455: GDOCPTR983 (crus CGolfDoc+0xNNN do 983)

// B1449: offset do 983 com indice em tempo de execucao, ancorado no membro m (offset v no VC10) -> anda junto no cl novo
#define SDOCM(T, m, v, off) (*(T*)((char*)&Doc()->m + ((off) - (v))))
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) == 0x38b0 && sizeof(sUserInfo) == 0x31c6 && offsetof(CSharedDoc, m_levelTable) == 0x10d84);
C_ASSERT_VC10(offsetof(CSharedDoc, m_teamPlayerGuid) == 0x10fd4 && offsetof(CSharedDoc, m_holeOrder) == 0x1051e);
#define SDOC983(T, off) (*(T*)DOCPTR983(off)) // B1448: so offset constante (indice em tempo de execucao = cadeia de ?:)
C_ASSERT_VC10(sizeof(sRivalData) == 0x1b8 && offsetof(sRivalData, totalPang) == 0x128 && offsetof(sRivalData, totalScore) == 0x14c && offsetof(sRivalData, rank) == 0x152); // B1560: ex-crus do 0x134

// the original repeats these moves inline (no helper functions in the unit)
#define SHIFT_WND_Y(pos, pWnd, dy) \
	do \
	{ \
		const WRect& shiftRect = (pWnd)->GetRect(); \
		pos = WPoint(shiftRect.x, (dy) + shiftRect.y); \
		(pWnd)->MoveWindow(pos); \
	} while (0)

#define SHIFT_MATCH_RES(ppRes) \
	for (int res = 0; res < 2; res++) \
	{ \
		WPoint resPos; \
		float resDy = res == 0 ? -18.0f : -43.0f; \
		FrArea* pRes = (ppRes)[res]; \
		float resY = resDy + pRes->GetRect().y; \
		resPos.x = pRes->GetRect().x; \
		resPos.y = resY; \
		pRes->MoveWindow(resPos); \
	}

__declspec(dllimport) __forceinline unsigned char MyIndexInline()
{
	bool bGM = (Doc()->m_myInfo.info.dwIdentity >> 1) & 1;
	if (bGM)
	{
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 0xb: // 983 0053a260 (MyIndex) inclui 0xb
			return Doc()->GetIndex(Doc()->m_myInfo.info.dwGalleryGuid);
		default:
			return Doc()->m_pGolfDoc->m_currentPlayer;
		}
	}
	return Doc()->GetIndex(Doc()->m_myInfo.info.dwGuid);
}

void FrAwardForm::OnUserListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;
	int item = (int)pItem->pData;
	if (item >= Doc()->m_pGolfDoc->m_playerNum)
		return;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	_WRECT rect;
	rect.x = pItem->pos.x;
	rect.y = pItem->pos.y;
	rect.w = (float)m_pUserList->GetItemWidth();
	rect.h = (float)m_pUserList->GetItemHeight();
	float adjY = 0.0f;
	unsigned char index = (unsigned char)item;
	if (Doc()->m_golfGame.gameType == 1)
	{
		bool bBox = false;
		if (item == MyIndex())
			bBox = true;
		else if (PLAYER(MyIndex())->teammate != 0xff &&
			item == PLAYER(MyIndex())->teammate)
			bBox = true;
		if (bBox)
			pGDI->Box(rect, 0x999B9DFF, 0, 0.0f);

		bool bSecond;
		if (Doc()->m_pGolfDoc->GetPlayer(MyIndex())->teammate == 0xff)
			bSecond = false;
		else
			bSecond = PLAYER(index)->teammate > item;
		float textY;
		if (m_userCount < 4)
			textY = 8.0f;
		else if (bSecond)
		{
			textY = 11.0f;
			adjY = 3.0f;
		}
		else
		{
			textY = 5.0f;
			adjY = -3.0f;
		}
		if (!CProjectG::Instance()->HidePrivacy())
			g_pFresh->GetManager()->PrintText(
				WPoint(pItem->pos.x + 98.0f, textY + pItem->pos.y), 0,
				(const char*)&SDOCM(char, m_userInfo, 0x38b0, item * 0x31c6 + 0x38c8), 70.0f, 0xFFFFFFFF);
		if (!bSecond)
		{
			float y = pItem->pos.y + 8.0f;
			if (m_userCount >= 4)
				y = pItem->pos.y - 6.0f;
			pGDI->Print(WPoint(pItem->pos.x + 194.0f, y), 2, "%d",
				TEAM(PLAYER(index)->team)->score);
			pGDI->Print(WPoint(pItem->pos.x + 237.0f, y), 2, "%d",
				(int)TEAM(PLAYER(index)->team)->totalPang);
			pGDI->SetTextColor(0xFF006493, 0xFFFFFFFF);
			pGDI->Print(WPoint(pItem->pos.x + 243.0f, y), 0, "(+%d)",
				(int)TEAM(PLAYER(index)->team)->bonusPang);
			pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		}
	}
	else
	{
		unsigned long myOid;
		if (((Doc()->m_myInfo.info.dwIdentity >> 1) & 1) == 0)
			myOid = Doc()->m_myInfo.info.dwGuid;
		else
			myOid = Doc()->m_myInfo.info.dwGalleryGuid;
		if (Doc()->m_pGolfDoc->GetPlayer(index)->oid == myOid)
			pGDI->Box(rect, 0x999B9DFF, 0, 0.0f);
		else if (PLAYER(index)->state == 3)
			pGDI->Box(rect, 0x80808080, 0, 0.0f);
		if (Doc()->m_golfGame.gameType == 0)
			pGDI->Print(WPoint(pItem->pos.x + 19.0f, pItem->pos.y + 8.0f), 1,
				"%d", PLAYER(index)->order);
		if (!CProjectG::Instance()->HidePrivacy())
			g_pFresh->GetManager()->PrintText(
				WPoint(pItem->pos.x + 98.0f, pItem->pos.y + 8.0f), 0,
				(const char*)&SDOCM(char, m_userInfo, 0x38b0, item * 0x31c6 + 0x38c8), 70.0f, 0xFFFFFFFF);
		if (Doc()->m_golfGame.gameType == 0)
		{
			WPoint pos(pItem->pos.x + 194.0f, pItem->pos.y + 8.0f);
			if (PLAYER(index)->score == 0)
				pGDI->Print(pos, 2, "0");
			else
				pGDI->Print(pos, 2, "%+d", PLAYER(index)->score);
		}
		else
		{
			WPoint pos(pItem->pos.x + 194.0f, pItem->pos.y + 8.0f);
			if (Doc()->m_golfGame.gameType != 7)
				pGDI->Print(pos, 2, "%d", PLAYER(index)->score);
			else
				pGDI->Print(pos, 2, "%I64d", PLAYER(index)->skinsTotalPang);
		}
		// 983: icone "assist" ao lado do proprio nome quando o item de assistencia esta ligado (0063bba0)
		if (Doc()->m_pGolfDoc->GetPlayer(index)->oid == Doc()->m_myInfo.info.dwGuid &&
			DOCJP()->IsTrajectoryPreviewOn() && m_pBmp[14] != NULL)
			pGDI->DrawTexture(m_pBmp[14],
				WRect(pItem->pos.x + 289.0f, pItem->pos.y + 7.0f,
					(float)m_pBmp[14]->Width(), (float)m_pBmp[14]->Height()),
				0xFFFFFFFF, 0);
		pGDI->Print(WPoint(pItem->pos.x + 237.0f, pItem->pos.y + 8.0f), 2, "%d",
			(int)Doc()->m_pGolfDoc->GetPlayer(index)->totalPang);
		pGDI->SetTextColor(0xFF006493, 0xFFFFFFFF);
		pGDI->Print(WPoint(pItem->pos.x + 243.0f, pItem->pos.y + 8.0f), 0,
			"(+%d)", (int)Doc()->m_pGolfDoc->GetPlayer(index)->bonusPang);
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	}

	bool bDrawn = false;
	std::list<sSlotInfo>& slotList = Doc()->m_slotList;
	std::list<sSlotInfo>::iterator it;
	for (it = slotList.begin(); it != slotList.end(); ++it)
	{
		if (it->dwGuid == SDOCM(int, m_userInfo, 0x38b0, item * 0x31c6 + 0x392a))
			break;
	}
	if (it != slotList.end())
	{
		sSlotInfo* pElem = &*it;
		// 983 0078f2d2: o mesmo icone e desenhado 10 vezes no mesmo lugar (contador 10, elemento fixo);
		// com alfa as bordas acumulam opacidade; 1 passada deixava o icone mais claro
		for (int pass = 0; pElem != NULL && pass < 10; pass++)
		{
			if (pElem->pangma[1] == 0 || m_pBmp[12] == NULL)
			{
				if (pElem->pangma[0] != 0 && m_pBmp[11] != NULL)
				{
					pGDI->DrawTexture(m_pBmp[11],
						WRect(pItem->pos.x + 474.0f, 3.0f + pItem->pos.y + adjY,
							(float)m_pBmp[11]->Width(),
							(float)m_pBmp[11]->Height()),
						0xFFFFFFFF, 0);
					bDrawn = true;
				}
			}
			else
			{
				pGDI->DrawTexture(m_pBmp[12],
					WRect(pItem->pos.x + 477.0f, 3.0f + pItem->pos.y + adjY,
						(float)m_pBmp[12]->Width(), (float)m_pBmp[12]->Height()),
					0xFFFFFFFF, 0);
				bDrawn = true;
			}
		}
	}
	if ((SDOCM(unsigned int, m_userInfo, 0x38b0, item * 0x31c6 + 0x3922) >> 14) & 1)
	{
		if (m_pBmp[2] != NULL)
			pGDI->DrawTexture(m_pBmp[2],
				WRect(pItem->pos.x + 412.0f, adjY + pItem->pos.y + 1.0f,
					(float)m_pBmp[2]->Width(), (float)m_pBmp[2]->Height()),
				0xFFFFFFFF, 0);
		bDrawn = true;
	}
	const Bitmap* pBmp;
	unsigned long mascotId = SDOCM(unsigned long, m_userInfo, 0x38b0, item * 0x31c6 + 0x6a3c);
	IFF_STRUCT::sMascot* pMascot;
	if (mascotId != 0 &&
		(pMascot = Doc()->m_itemManager.FindMascot(mascotId)) != NULL)
	{
		if (_strcmpi(pMascot->c.Name, K2L_Compatibility("\xc7\xcf\xbe\xe7\xc1\xa6\xb8\xae")) == 0)
			pBmp = m_pBmp[3];
		else if (_strcmpi(pMascot->c.Name,
					 K2L_Compatibility("\xb1\xee\xb8\xc1\xc1\xa6\xb8\xae")) == 0)
			pBmp = m_pBmp[4];
		else if (pMascot->c.TypeId == 0x40000009)
			pBmp = m_pBmp[5];
		else if (pMascot->c.TypeId == 0x4000000a)
			pBmp = m_pBmp[6];
		else if (pMascot->c.TypeId == 0x4000000b)
			pBmp = m_pBmp[7];
		else if (pMascot->c.TypeId == 0x4000000c)
			pBmp = m_pBmp[8];
		else if (pMascot->c.TypeId == 0x40000027)
			pBmp = m_pBmp[9];
		else if (pMascot->c.TypeId == 0x40000028)
			pBmp = m_pBmp[10];
		else
			goto noMascot;
		if (pBmp != NULL)
			pGDI->DrawTexture(pBmp,
				WRect(pItem->pos.x + 458.0f, adjY + pItem->pos.y + 1.0f,
					(float)pBmp->Width(), (float)pBmp->Height()),
				0xFFFFFFFF, 0);
		return;
	}
noMascot:
	if (bDrawn)
		return;
	pBmp = m_pBmp[13];
	if (pBmp != NULL)
		pGDI->DrawTexture(pBmp,
			WRect(pItem->pos.x + 442.0f, adjY + pItem->pos.y + 10.0f,
				(float)pBmp->Width(), (float)pBmp->Height()),
			0xFFFFFFFF, 0);
}

std::string BuildLevelUpDesc983(); // resultdlg.cpp (983 00643fe0)

bool FrAwardForm::OnInit()
{
	if (0)
		((CRankExData*)0)->CRankExData::SetLeader();
	WPoint pos;
	FrForm::OnInit();
	if (m_pUserList != NULL)
	{
		m_pUserList->EnableScrollBar(false);
		FrListBox* pList = m_pUserList;
		WRect rect = pList->GetRect();
		float itemH = (float)pList->GetItemHeight();
		float newH = itemH * m_userCount;
		float delta = newH - rect.h;
		rect.h = newH;
		pList->SetRect(rect);
		pos.x = m_rect.x;
		m_rect.h = delta + m_rect.h;
		pos.y = (g_view->GetHeight() - m_rect.h) + 5.0f;
		MoveWindow(pos);
		SetRect(m_rect);
		float nextH = m_pNextLevel->GetRect().h;
		pos.x = m_rect.x + 25.0f;
		pos.y = g_view->GetHeight() - nextH;
		m_pNextLevel->MoveWindow(pos);
	}
	if (Doc()->m_golfGame.gameType == 0)
	{
		CalcResult();
	}
	else if (Doc()->m_golfGame.gameType == 1)
	{
		float baseY = 0.0f;
		if (m_userCount < 4)
			baseY = -3.0f;
		for (int i = 0; i < 4; i++)
		{
			bool bFirst;
			if (PLAYER(m_order[i])->teammate != 0xff)
				bFirst = m_order[i] < (int)PLAYER(m_order[i])->teammate;
			else
				bFirst = true;
			SHIFT_WND_Y(pos, m_pExpBar[i],
				bFirst ? baseY + 3.0f : baseY - 3.0f);
			SHIFT_WND_Y(pos, m_pExp[i], bFirst ? baseY + 3.0f : baseY - 3.0f);
			SHIFT_WND_Y(pos, m_pExpEnd[i],
				bFirst ? baseY + 3.0f : baseY - 3.0f);
			{
				const WRect& levelRect = m_pLevel[i]->GetRect();
				pos = WPoint((float)(levelRect.x - 13.0),
					(bFirst ? 3.0f : -3.0f) + levelRect.y);
				pos = WPoint(levelRect.x,
					(bFirst ? baseY + 3.0f : baseY - 3.0f) + levelRect.y);
				m_pLevel[i]->MoveWindow(pos);
			}
			SHIFT_WND_Y(pos, m_pLevelUp[i],
				bFirst ? baseY + 3.0f : baseY - 3.0f);
		}
		if (*((unsigned char*)GDOCPTR983(GOLFDOC(), 0xd0)) == 2)
		{
			m_pMatchRes[0]->SetBgImg("draw");
			m_pMatchRes[1]->SetBgImg("draw");
		}
		else
		{
			m_pMatchRes[0]->SetBgImg("win");
			m_pMatchRes[1]->SetBgImg("lose");
		}
		if (m_userCount < 4)
			SHIFT_MATCH_RES(m_pMatchRes)
		CalcResult();
	}
	else if (Doc()->m_golfGame.gameType == 7)
	{
		sPlayerData* pRed = PLAYER(0);
		sPlayerData* pBlue = PLAYER(1);
		if (pRed->rank == pBlue->rank)
		{
			m_pMatchRes[0]->SetBgImg("draw");
			m_pMatchRes[1]->SetBgImg("draw");
			FrArea* pRes = m_pMatchRes[0];
			WPoint resPos;
			float resY = pRes->GetRect().y - 18.0f;
			resPos.x = pRes->GetRect().x;
			resPos.y = resY;
			pRes->MoveWindow(resPos);
			m_pMatchRes[1]->SetVisible(false);
			CalcResult();
		}
		else
		{
			m_pMatchRes[0]->SetBgImg("win");
			m_pMatchRes[1]->SetBgImg("lose");
			SHIFT_MATCH_RES(m_pMatchRes)
			CalcResult();
		}
	}

	m_pBmp[0] = NULL;
	m_pBmp[1] = NULL;
	m_pBmp[2] = g_pFresh->GetManager()->GetBitmap("golf", "premium+10");
	m_pBmp[3] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_01");
	m_pBmp[4] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_02");
	m_pBmp[5] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_09");
	m_pBmp[6] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_10");
	m_pBmp[7] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_11");
	m_pBmp[8] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_12");
	m_pBmp[9] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_39");
	m_pBmp[10] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_mascot_40");
	m_pBmp[11] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_ma");
	m_pBmp[12] = g_pFresh->GetManager()->GetBitmap("golf", "bonus_ma2");
	m_pBmp[13] = g_pFresh->GetManager()->GetBitmap("golf", "icon_zero_vs");
	m_pBmp[14] = g_pFresh->GetBitmap("assist_title");

	m_pNextLevel->SetVisible(true);
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (PLAYER((unsigned char)i)->oid == Doc()->m_myInfo.info.dwGuid)
		{
			if ((unsigned int)((int)GOLFDOC()->m_holeResult983[i].exp +
					SDOCM(int, m_userInfo, 0x38b0, 0x3a29 + i * 0x31c6)) >
				SDOCM(unsigned int, m_levelTable, 0x10d84, 0x10d84 + m_level[i] * 0x8))
			{
				m_pNextLevel->SetVisible(false);
			}
			else
			{
				std::string label = BuildLevelUpDesc983();
				m_pNextLevel->AddLine(label.c_str(), 0, false);
			}
		}
	}

	int result = 0;
	if (Doc()->m_golfGame.gameType == 1)
	{
		sPlayerData* pPlayer = PLAYER(MyIndexInline());
		sTeamData* pMyTeam = TEAM(pPlayer->team);
		pPlayer = PLAYER(MyIndexInline());
		sTeamData* pOtherTeam = TEAM(pPlayer->team == 0);
		if ((int)pMyTeam->score < (int)pOtherTeam->score)
			result = 2;
		pPlayer = PLAYER(MyIndexInline());
		pMyTeam = TEAM(pPlayer->team);
		pPlayer = PLAYER(MyIndexInline());
		pOtherTeam = TEAM(pPlayer->team == 0);
		if ((int)pMyTeam->score == (int)pOtherTeam->score)
			result = 1;
	}
	else if (Doc()->m_golfGame.gameType == 0)
	{
		int myScore = PLAYER(MyIndexInline())->score;
		int myPang = (int)PLAYER(MyIndexInline())->totalPang;
		int best = 0;
		for (int j = 0; j < GOLFDOC()->m_playerNum; j++)
		{
			if (j != MyIndexInline())
			{
				if (PLAYER(j)->score < PLAYER(best)->score ||
					(PLAYER(j)->score == PLAYER(best)->score &&
						PLAYER(j)->totalPang > PLAYER(best)->totalPang))
					best = j;
				if (PLAYER(j)->score < myScore ||
					(PLAYER(j)->score == myScore &&
						PLAYER(j)->totalPang > myPang))
					result = 2;
			}
		}
		if (result != 0 && PLAYER(best)->score == myScore &&
			PLAYER(best)->totalPang == myPang)
			result = 1;
	}
	else if (Doc()->m_golfGame.gameType == 7)
	{
		if (SDOC983(int, 0x81a) != *(int*)((char*)GDOCPTR983(GOLFDOC(), 0x1a4)))
			result = 2;
		else if (*(int*)((char*)GDOCPTR983(GOLFDOC(), 0x1a4)) == 0)
			result = 1;
	}

	CVoiceItem::eWaveState state;
	switch (result)
	{
	case 0:
		state = (CVoiceItem::eWaveState)0x40;
		break;
	case 2:
		state = (CVoiceItem::eWaveState)0x200;
		break;
	default:
		return true;
	}
	unsigned char userA = MyIndexInline();
	unsigned char userB = MyIndexInline();
	CVoiceItem::Instance()->PlaySoundA(
		SDOCM(unsigned long, m_userInfo, 0x38b0, userB * 0x31c6 + 0x6a20), state,
		SDOCM(unsigned int, m_userInfo, 0x38b0, userA * 0x31c6 + 0x6802) & 0x3ffffff);
	return true;
}

// 983 (00786040): modo pelo GetDifficulty (006ad0f0); na sala classica a exp sai do GetClassicExp983 (00b305d0)
// e nao da s_ladderTable; no modo time o lado compara o time do jogador com o mesmo campo (+0xd0) testado contra 2.
void FrAwardForm::CalcResult()
{
	char buf[32];
	CGolfDoc* pGolf = Doc()->m_pGolfDoc;
	int userOfs = 0;
	int expOfs = 0;
	int* pExp = (int*)m_expGain;
	for (int i = 0; i < pGolf->m_playerNum; ++i)
	{
		if (Doc()->m_golfGame.gameType == 1 ||
			pGolf->GetPlayer((unsigned char)i)->state != 3)
		{
			sPlayerData* pPlayer =
				Doc()->m_pGolfDoc->GetPlayer((unsigned char)i);
			int slot = pPlayer->order - 1;
			if (slot >= 0 && m_userCount > slot)
			{
				if (WNetworkSystem::Instance()->IsConnected(
						WNetworkSystem::NET_GAME) != 0)
				{
					pExp[-4] = SDOCM(unsigned char, m_userInfo, 0x38b0, 0x3a2d + expOfs);
					pExp[4] = SDOCM(int, m_userInfo, 0x38b0, 0x3a29 + expOfs);
					*pExp =
						*(short*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x347) + userOfs);
				}
				else
				{
					unsigned char cntTeam = 0;
					unsigned char cnt2 = 0;
					unsigned char cnt3 = 0;
					unsigned char cnt4 = 0;
					for (int j = 0; j < (int)SDOC983(unsigned char, 0x10699); ++j)
					{
						if (Doc()->m_golfGame.gameType == 0)
						{
							unsigned char kind = *(
								(unsigned char*)*(
									char**)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
								0x1f + SDOCM(unsigned char, m_holeOrder, 0x1051e, 0x1051e + j) * 0x38);
							switch (kind)
							{
							case 2:
								++cnt2;
								break;
							case 3:
								++cnt3;
								break;
							case 4:
								++cnt4;
								break;
							}
						}
						else if (Doc()->m_golfGame.gameType == 1)
						{
							if (*((char*)*(
									  char**)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
									0x1f + (unsigned char)j * 0x38) == 4)
								++cntTeam;
						}
					}
					pExp[-4] = SDOCM(unsigned char, m_userInfo, 0x38b0, 0x3a2d + expOfs);
					pExp[4] = SDOCM(int, m_userInfo, 0x38b0, 0x3a29 + expOfs);
					if (Doc()->m_golfGame.gameType == 0)
					{
						unsigned char diff = GetDifficulty() - 1;
						if (!S5::CLASSICSRV::IsClassicGame983())
						{
							int row = diff * 0x13;
							unsigned int rank = pPlayer->rank;
							*pExp =
								s_ladderTable[6 + ((cnt4 + row) * 0xb + rank)] +
								s_ladderTable[3 + ((cnt3 + row) * 0xb + rank)] +
								s_ladderTable[1 + ((cnt2 + row) * 0xb + rank)];
						}
						else
						{
							*pExp =
								S5::CLASSICSRV::GetClassicExp983(diff, cnt4, 4, pPlayer->rank - 1) +
								S5::CLASSICSRV::GetClassicExp983(diff, cnt3, 3, pPlayer->rank - 1) +
								S5::CLASSICSRV::GetClassicExp983(diff, cnt2, 2, pPlayer->rank - 1);
						}
					}
					else if (Doc()->m_golfGame.gameType == 1)
					{
						unsigned char side;
						int winTeam = *(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd0));
						if (winTeam == 2 || (unsigned int)pPlayer->team == (unsigned int)winTeam)
							side = 0;
						else
							side = 1;
						unsigned char diff = GetDifficulty() - 1;
						if (!S5::CLASSICSRV::IsClassicGame983())
							*pExp = s_ladderTable[(diff * 0x13 + cntTeam) * 0xb + side];
						else
							*pExp = S5::CLASSICSRV::GetClassicExp983(diff, cntTeam, 1, side);
					}
				}
				int gained = *pExp;
				pExp[4] = pExp[4] * 10;
				*pExp = gained * 10;
				int range = SDOCM(int, m_levelTable, 0x10d84, 0x10d84 + pExp[-4] * 0x8) * 10;
				if (gained * 10 == 0)
					((unsigned char*)&m_expDone)[i] = 1;
				if (m_pExpBar[slot] != NULL)
				{
					m_pExpBar[slot]->SetRange(0, range, 0);
					m_pExpBar[slot]->SetPos(pExp[4]);
				}
				if (m_pExp[slot] != NULL)
				{
					sprintf(buf, "%d", pExp[4] / 10);
					m_pExp[slot]->SetCaption(buf);
				}
				if (m_pExpEnd[slot] != NULL)
				{
					sprintf(buf, "/ %d", SDOCM(int, m_levelTable, 0x10d84, 0x10d84 + pExp[-4] * 0x8));
					m_pExpEnd[slot]->SetCaption(buf);
				}
				if (m_pLevel[slot] != NULL)
				{
					sprintf(buf, "level_%03d", pExp[-4] + 1);
					m_pLevel[slot]->SetBgImg(buf);
				}
				if (m_pExpBar[slot] != NULL)
					m_pExpBar[slot]->SetVisible(true);
				if (m_pExp[slot] != NULL)
					m_pExp[slot]->SetVisible(true);
				if (m_pExpEnd[slot] != NULL)
					m_pExpEnd[slot]->SetVisible(true);
				if (m_pLevel[slot] != NULL)
					m_pLevel[slot]->SetVisible(true);
			}
		}
		pGolf = Doc()->m_pGolfDoc;
		userOfs += 0x21;
		expOfs += 0x31c6;
		++pExp;
	}
}

// 983 (00786500): como o CalcResult (GetDifficulty, ramo classico pelo GetClassicExp983, lado do time por +0xd0).
void FrAwardForm::CalcResultMatch()
{
	char buf[32] = { 0 };
	CGolfDoc* pGolf = Doc()->m_pGolfDoc;
	int userOfs = 0;
	int expOfs = 0;
	for (int i = 0; i < pGolf->m_playerNum; ++i)
	{
		if (pGolf->GetPlayer((unsigned char)i)->state != 3)
		{
			sPlayerData* pPlayer =
				Doc()->m_pGolfDoc->GetPlayer((unsigned char)i);
			int slot = (pPlayer->order & 0xf) - 1;
			if (slot >= 0 && m_userCount > slot)
			{
				if (WNetworkSystem::Instance()->IsConnected(
						WNetworkSystem::NET_GAME) != 0)
				{
					m_curExp[i] = SDOCM(int, m_userInfo, 0x38b0, 0x3a80 + expOfs);
					m_curExp[i] +=
						(m_expGain[i] = *(short*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x347) + userOfs)); // 983: +0x347 (o 645 lia +0x32f)
				}
				else
				{
					unsigned char cntTeam = 0;
					unsigned char cnt2 = 0;
					unsigned char cnt3 = 0;
					unsigned char cnt4 = 0;
					for (int j = 0; j < (int)SDOC983(unsigned char, 0x10699); ++j)
					{
						if (Doc()->m_golfGame.gameType == 0)
						{
							unsigned char kind = *(
								(unsigned char*)*(
									char**)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
								0x1f + SDOCM(unsigned char, m_holeOrder, 0x1051e, 0x1051e + j) * 0x38);
							switch (kind)
							{
							case 2:
								++cnt2;
								break;
							case 3:
								++cnt3;
								break;
							case 4:
								++cnt4;
								break;
							}
						}
						else if (Doc()->m_golfGame.gameType == 1)
						{
							if (*((char*)*(
									  char**)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
									0x1f + (unsigned char)j * 0x38) == 4)
								++cntTeam;
						}
					}
					m_level[i] = SDOCM(unsigned char, m_userInfo, 0x38b0, 0x3a2d + expOfs);
					m_curExp[i] = SDOCM(int, m_userInfo, 0x38b0, 0x3a29 + expOfs);
					unsigned char diff = GetDifficulty() - 1;
					if (Doc()->m_golfGame.gameType == 0)
					{
						if (!S5::CLASSICSRV::IsClassicGame983())
						{
							int row = diff * 0x13;
							unsigned int rank = pPlayer->rank;
							m_expGain[i] =
								s_ladderTable[6 + ((cnt4 + row) * 0xb + rank)] +
								s_ladderTable[3 + ((cnt3 + row) * 0xb + rank)] +
								s_ladderTable[1 + ((cnt2 + row) * 0xb + rank)];
						}
						else
						{
							m_expGain[i] =
								S5::CLASSICSRV::GetClassicExp983(diff, cnt4, 4, pPlayer->rank - 1) +
								S5::CLASSICSRV::GetClassicExp983(diff, cnt3, 3, pPlayer->rank - 1) +
								S5::CLASSICSRV::GetClassicExp983(diff, cnt2, 2, pPlayer->rank - 1);
						}
					}
					else if (Doc()->m_golfGame.gameType == 1)
					{
						unsigned char side;
						int winTeam = *(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd0));
						if (winTeam == 2 || (unsigned int)pPlayer->team == (unsigned int)winTeam)
							side = 0;
						else
							side = 1;
						if (!S5::CLASSICSRV::IsClassicGame983())
							m_expGain[i] = s_ladderTable[(diff * 0x13 + cntTeam) * 0xb + side];
						else
							m_expGain[i] = S5::CLASSICSRV::GetClassicExp983(diff, cntTeam, 1, side);
					}
				}
				if (m_expGain[i] == 0)
					((unsigned char*)&m_expDone)[i] = 1;
				if (m_pGainedLadderPoint[slot] != NULL)
				{
					sprintf(buf, "%d", m_expGain[i]);
					m_pGainedLadderPoint[slot]->SetCaption(buf);
					m_pGainedLadderPoint[slot]->SetVisible(true);
				}
				if (m_pLadderPoint[slot] != NULL)
				{
					sprintf(buf, "%d", m_curExp[i]);
					m_pLadderPoint[slot]->SetCaption(buf);
					m_pLadderPoint[slot]->SetVisible(true);
				}
			}
		}
		pGolf = Doc()->m_pGolfDoc;
		userOfs += 0x21;
		expOfs += 0x31c6;
	}
}

void FrAwardForm::OnProc(const float deltaTime)
{
	char buf[32];
	m_animTimer += deltaTime;
	if (m_animTimer < 1.0f)
		return;
	if (m_pSfxName == NULL && ((unsigned char*)&m_expDone)[MyIndex()] == 0)
	{
		if ((int)m_expGain[MyIndex()] > 0 || (int)m_expGain[MyIndex()] < 0)
			m_pSfxName = "player_exp_up";
		if (m_pSfxName != NULL && !g_audio->IsPlaying(m_pSfxName))
			g_audio->PlaySfx(m_pSfxName, NULL, 1, m_pSfxName, NULL, 0.5f,
				200.0f);
	}
	if (!(m_animTimer - 1.0f < 0.01f))
	{
		int step = (int)(100.0f * (m_animTimer - 1.0f));
		m_animTimer = m_animTimer - (float)step / 100.0f;
		unsigned int i = 0;
		if ((unsigned int)Doc()->m_pGolfDoc->m_playerNum > 0)
		{
			int expOfs = 0;
			do
			{
				if (((unsigned char*)&m_expDone)[i] == 0 &&
					(Doc()->m_golfGame.gameType == 1 ||
						Doc()->m_pGolfDoc->GetPlayer((unsigned char)i)->state !=
							3))
				{
					int levelExp = SDOCM(int, m_levelTable, 0x10d84, 0x10d84 + m_level[i] * 0x8) * 10;
					int remain = (int)m_expGain[i];
					if (step > (remain > 0 ? remain : -remain))
					{
						((unsigned char*)&m_expDone)[i] = 1;
						remain = (int)m_expGain[i];
						step = remain > 0 ? remain : -remain;
						m_expGain[i] = 0;
						m_curExp[i] =
							m_curExp[i] + ((remain > 0) * 2 - 1) * step;
					}
					else
					{
						int delta = ((remain > 0) * 2 - 1) * step;
						m_curExp[i] = m_curExp[i] + delta;
						m_expGain[i] = remain - delta;
					}
					if ((int)m_curExp[i] < 0 &&
						((unsigned char*)&m_expDone)[i] == 0)
					{
						((unsigned char*)&m_expDone)[i] = 1;
						m_curExp[i] = 0;
					}
					int slot = (Doc()->m_pGolfDoc->GetPlayer((unsigned char)i)
									   ->order) -
						1;
					if (slot >= 0 && m_userCount > slot)
					{
						if ((int)m_curExp[i] >= levelExp)
						{
							g_audio->PlaySfx("level_up", NULL, 0, NULL, NULL,
								0.5f, 200.0f);
							m_curExp[i] -= levelExp;
							m_level[i] = m_level[i] + 1;
							if (m_pExpBar[slot] != NULL)
							{
								m_pExpBar[slot]->SetRange(0,
									SDOCM(int, m_levelTable, 0x10d84, 0x10d84 + m_level[i] * 0x8) * 10,
									0);
								m_pExpBar[slot]->SetPos(m_curExp[i]);
							}
							if (m_pExpEnd[slot] != NULL)
							{
								sprintf(buf, "/ %d",
									SDOCM(int, m_levelTable, 0x10d84, 0x10d84 + m_level[i] * 0x8));
								m_pExpEnd[slot]->SetCaption(buf);
							}
							if (m_pLevel[slot] != NULL)
							{
								sprintf(buf, "level_%03d", m_level[i] + 1);
								m_pLevel[slot]->SetBgImg(buf);
							}
							MsgObject msg(NULL, 0x272, i, 0, 0, 0, 0);
							AfxGetTask()->GetActor("Player") << msg;
						}
						else
						{
							if (m_pExpBar[slot] != NULL)
								m_pExpBar[slot]->SetPos(m_curExp[i]);
						}
						sprintf(buf, "%d", (int)m_curExp[i] / 10);
						m_pExp[slot]->SetCaption(buf);
						if (((unsigned char*)&m_expDone)[i] != 0)
						{
							if (m_level[i] !=
								SDOCM(unsigned char, m_userInfo, 0x38b0, 0x3a2d + expOfs))
							{
								if (m_pExp[slot] != NULL)
									m_pExp[slot]->SetVisible(false);
								if (m_pExpEnd[slot] != NULL)
									m_pExpEnd[slot]->SetVisible(false);
								if (m_pLevelUp[slot] != NULL)
									m_pLevelUp[slot]->SetVisible(true);
								int* head = SDOC983(int*, 0x109fc);
								for (int* node = (int*)*head; node != head;
									node = (int*)*node)
								{
									if (node[2] == SDOCM(int, m_userInfo, 0x38b0, 0x392a + expOfs))
									{
										*((unsigned char*)node + 0x5d) =
											(unsigned char)m_level[i];
										break;
									}
								}
								if (CHeadIcon::IsInstantiated())
								{
									unsigned char saved[128];
									int j;
									for (j = 0;
										j < (int)Doc()->m_pGolfDoc->m_playerNum;
										++j)
									{
										saved[j] = SDOCM(unsigned char, m_userInfo, 0x38b0,
											0x3a2d + j * 0x31c6);
										SDOCM(unsigned char, m_userInfo, 0x38b0,
											0x3a2d + j * 0x31c6) =
											(unsigned char)m_level[j];
									}
									CHeadIcon::Instance()->SetLevel();
									for (j = 0;
										j < (int)Doc()->m_pGolfDoc->m_playerNum;
										++j)
										SDOCM(unsigned char, m_userInfo, 0x38b0,
											0x3a2d + j * 0x31c6) = saved[j];
								}
							}
							if (i == MyIndexInline() && m_pSfxName != NULL)
								g_audio->StopSfx(m_pSfxName);
						}
					}
				}
				expOfs += 0x31c6;
				++i;
			} while ((int)i < (int)Doc()->m_pGolfDoc->m_playerNum);
		}
		m_giftWaitTime += deltaTime;
		if (CTHunter::Instance()->IsReceiveGiftPacket() && m_giftWaitTime > 2.0f)
			SDOC983(unsigned char, 0x10778) = 1;
	}
}

void ResizeScoreBar23(FrArea* bar2, FrArea* bar3, float x, float w);

// 983 006336a0 (CSharedDoc, sem nome): item escondido do jogador; o ctor do cAquireItem pula estes (B330)
// Global (nao static) para sair __stdcall "ret 4" com quadro ebp, como o 983; o tipo 6 e testado de novo em cada if.
__declspec(noinline) bool __stdcall IsHiddenItem983(unsigned long tid)
{
	if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1e00000)
		return true;
	if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x1f00000)
	{
		if (Doc()->m_myInfo.info.dwIdentity & 0x2804)
			return false;
		return true;
	}
	if (ItemManager()->IsNonVisibleItem(tid) == true)
		return true;
	return false;
}

cAquireItem::cAquireItem()
{
	m_maxTime = 8.0f;
	m_pFont = NULL;
	m_time = 0.0f;
	for (int i = 0; i < GetPlayerNum(); i++)
	{
		std::vector<unsigned long>& list = Doc()->m_acquireItemList[i];
		for (std::vector<unsigned long>::iterator it = list.begin();
			it != list.end(); ++it)
		{
			unsigned long key = *it;
			if (IsHiddenItem983(key))
				continue;
			std::map<unsigned long, int>::iterator found =
				m_slot[i].m_items.find(key);
			if (found == m_slot[i].m_items.end())
				m_slot[i].m_items.insert(std::make_pair(key, 1));
			else
				++(*found).second;
		}
	}
	m_pFrame[0] =
		g_pFresh->GetManager()->GetBitmap("golf", "itemframe_item_one");
	m_pFrame[1] =
		g_pFresh->GetManager()->GetBitmap("golf", "itemframe_item_two");
	m_pFrame[2] =
		g_pFresh->GetManager()->GetBitmap("golf", "itemframe_item_three");
	m_pBubble[0] = g_pFresh->GetBitmap("golf_text_bubble1");
	m_pBubble[1] = g_pFresh->GetBitmap("golf_text_bubble2");
	m_pBubble[2] = g_pFresh->GetBitmap("golf_text_bubble3");
	static char* s_fontFile[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	m_pFont = NULL;
	info.filename = s_fontFile;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
	m_pNumberUI = new CSimpleUI("[emoti_object.png");
	m_pNumberUI->Add("0", WRect(0.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("1", WRect(64.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("2", WRect(128.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("3", WRect(192.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("4", WRect(0.0f, 192.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("5", WRect(64.0f, 192.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("6", WRect(128.0f, 192.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("7", WRect(192.0f, 192.0f, 64.0f, 64.0f), false);
}

cAquireItem::~cAquireItem()
{
	if (m_pNumberUI)
	{
		delete m_pNumberUI;
		m_pNumberUI = NULL;
	}
	if (m_pFont)
		m_pFont->Erase();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
	for (int i = 0; i < 4; ++i)
	{
		for (std::map<unsigned long, WOverlay*>::iterator it =
				 m_icon[i].begin();
			it != m_icon[i].end(); ++it)
		{
			if (g_resrcmng && (*it).second)
			{
				g_resrcmng->Release((*it).second);
				(*it).second = NULL;
			}
		}
		m_icon[i].clear();
	}
}

void cAquireItem::Reset()
{
	m_time = 0.0f;
}

void cAquireItem::OnProc(float delta)
{
	if (m_time < m_maxTime)
		m_time += delta * 20.0f;
}

void cAquireItem::DisplayPrizeList(int index)
{
	std::map<unsigned long, int>& items = m_slot[index].m_items;
	unsigned int size = items.size();
	if (size <= 0)
		return;
	WVector pos = GetPlayer2DPos(index);
	if (pos.z <= 0.0f || 1.0f <= pos.z)
		return;
	float t = (m_maxTime < m_time) ? m_maxTime : m_time;
	unsigned long color = ((int)(t / m_maxTime * 255.0f) << 24) | 0xffffff;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI && m_pBubble[0])
	{
		float y = pos.y + 65.0f;
		pGDI->DrawTexture(m_pBubble[0],
			WRect(pos.x, y, (float)m_pBubble[0]->Width(),
				(float)m_pBubble[0]->Height()),
			color, 0);
		float w0 = (float)m_pBubble[0]->Width();
		unsigned int half = size >> 1;
		WRect dst(pos.x + w0, y, (float)m_pBubble[1]->Width() * (float)half,
			(float)m_pBubble[1]->Height());
		pGDI->DrawTexture(m_pBubble[1],
			WRect(0.0f, 0.0f, (float)m_pBubble[1]->Width(),
				(float)m_pBubble[1]->Height()),
			dst, color, 0);
		pGDI->DrawTexture(m_pBubble[2],
			WRect((float)m_pBubble[1]->Width() * (float)half + w0 + pos.x, y,
				(float)m_pBubble[2]->Width(), (float)m_pBubble[2]->Height()),
			color, 0);
	}
	float offset = 5.0f;
	for (std::map<unsigned long, int>::iterator it = items.begin();
		it != items.end(); ++it)
	{
		unsigned int tid = (*it).first;
		WOverlay* pOverlay = NULL;
		std::map<unsigned long, WOverlay*>::iterator found =
			m_icon[index].find(tid);
		if (found != m_icon[index].end())
			pOverlay = (*found).second;
		else
		{
			IFF_ITEM_COMMON* pInfo = ItemManager()->FindCommonItem(tid);
			if (pInfo)
			{
				// 983 (0078a8a0): so tipos 2/5/6 (o 645 aceitava 0x1d tambem)
				switch (tid >> 26)
				{
				case 2:
				case 5:
				case 6:
					pOverlay = g_resrcmng->GetOverlay(
						MakeStr("%s.tga", pInfo->Icon), 0);
					if (pOverlay)
						m_icon[index].insert(
							std::map<unsigned long, WOverlay*>::value_type(tid,
								pOverlay));
					break;
				}
			}
		}
		if (pOverlay)
		{
			WRect dst;
			dst.x = pos.x + offset;
			float y = pos.y + 82.0f;
			dst.y = y;
			dst.w = (float)pOverlay->GetWidth() * 0.65f;
			dst.h = (float)pOverlay->GetHeight() * 0.65f;
			pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f), dst, 0,
				color, 0.0f, 0);
			if ((*it).second > 1)
			{
				const char* str = MakeStr("%d", (*it).second);
				m_pFont->Print(g_view, pos.x + offset + 17.0f, y + 12.0f, str,
					0, color, NULL);
			}
			offset += 40.0f;
		}
	}
	if (m_pNumberUI && m_time < m_maxTime)
		m_pNumberUI->Render(MakeStr("%d", (int)m_time), pos.x - 64.0f,
			pos.y - 16.0f, 2.0f, 0);
}

WVector cAquireItem::GetPlayer2DPos(int index)
{
	struct
	{
		int pad[9];
		float pos[3];
	} out;
	MsgObject msg(NULL, 0x154, index, (int)&out, 0, 0, 0); // 983 (007824b0): 0x154 (0x124 no 645)
	AfxGetTask()->GetActor("Player") << msg;
	WVector p = g_view->Projection(*(WVector*)out.pos);
	return WVector(p.x, p.y, p.z);
}

IMPLEMENT_OBJECT(FrScoreForm, FrForm)

BEGIN_FRESH_MSGMAP(FrScoreForm, FrForm)
ON_FRESH_VI("bar2", FRCMD_INIT, FrScoreForm::OnBar2Init)
ON_FRESH_VI("bar3", FRCMD_INIT, FrScoreForm::OnBar3Init)
ON_FRESH_VI("front9btn", FRCMD_INIT, FrScoreForm::OnFront9BtnInit)
ON_FRESH_VV("front9btn", FRCMD_LBUTTONUP, FrScoreForm::OnFront9BtnUp)
ON_FRESH_VI("back9btn", FRCMD_INIT, FrScoreForm::OnBack9BtnInit)
ON_FRESH_VV("back9btn", FRCMD_LBUTTONUP, FrScoreForm::OnBack9BtnUp)
ON_FRESH_VI("shotbtn", FRCMD_INIT, FrScoreForm::OnShotBtnInit)
ON_FRESH_VV("shotbtn", FRCMD_LBUTTONUP, FrScoreForm::OnShotBtnUp)
ON_FRESH_VI("graphbtn", FRCMD_INIT, FrScoreForm::OnGraphBtnInit)
ON_FRESH_VV("graphbtn", FRCMD_LBUTTONUP, FrScoreForm::OnGraphBtnUp)
ON_FRESH_VI("versusbtn", FRCMD_INIT, FrScoreForm::OnVersusBtnInit)
ON_FRESH_VV("versusbtn", FRCMD_LBUTTONUP, FrScoreForm::OnVersusBtnUp)
ON_FRESH_VI("puttbtn", FRCMD_INIT, FrScoreForm::OnPuttBtnInit)
ON_FRESH_VV("puttbtn", FRCMD_LBUTTONUP, FrScoreForm::OnPuttBtnUp)
ON_FRESH_VI("pangbtn", FRCMD_INIT, FrScoreForm::OnPangBtnInit)
ON_FRESH_VV("pangbtn", FRCMD_LBUTTONUP, FrScoreForm::OnPangBtnUp)
ON_FRESH_VI("hole_list", FRCMD_INIT, FrScoreForm::OnHoleListInit)
ON_FRESH_VI("hole_list", FRCMD_OWNERDRAW, FrScoreForm::OnHoleListOwnerDraw)
ON_FRESH_VI("par_list", FRCMD_INIT, FrScoreForm::OnParListInit)
ON_FRESH_VI("par_list", FRCMD_OWNERDRAW, FrScoreForm::OnParListOwnerDraw)
ON_FRESH_VI("pang_list", FRCMD_INIT, FrScoreForm::OnPangListInit)
ON_FRESH_VI("pang_list", FRCMD_OWNERDRAW, FrScoreForm::OnPangListOwnerDraw)
ON_FRESH_VI("golfer_list", FRCMD_INIT, FrScoreForm::OnGolferListInit)
ON_FRESH_VI("golfer_list", FRCMD_OWNERDRAW, FrScoreForm::OnGolferListOwnerDraw)
ON_FRESH_VI("record_list", FRCMD_INIT, FrScoreForm::OnRecordListInit)
ON_FRESH_VI("record_list", FRCMD_OWNERDRAW, FrScoreForm::OnRecordListOwnerDraw)
ON_FRESH_VI("total_list", FRCMD_INIT, FrScoreForm::OnTotalListInit)
ON_FRESH_VI("total_list", FRCMD_OWNERDRAW, FrScoreForm::OnTotalListOwnerDraw)
ON_FRESH_VI("total", FRCMD_INIT, FrScoreForm::OnTotalInit)
ON_FRESH_VI("ppl", FRCMD_INIT, FrScoreForm::OnPPLInit)
ON_FRESH_VI("board1", FRCMD_INIT, FrScoreForm::OnBoard1Init)
ON_FRESH_VI("board2", FRCMD_INIT, FrScoreForm::OnBoard2Init)
ON_FRESH_VI("drawlines", FRCMD_INIT, FrScoreForm::OnDrawLinesInit)
ON_FRESH_VI("drawlines", FRCMD_OWNERDRAW, FrScoreForm::OnDrawLinesOwnerDraw)
ON_FRESH_VI("cancel", FRCMD_INIT, FrScoreForm::OnCancelBtnInit)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrScoreForm::OnCancelBtnUp)
ON_FRESH_VI("close", FRCMD_INIT, FrScoreForm::OnCloseBtnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrScoreForm::OnCloseBtnUp)
ON_FRESH_VI("blankpanel1", FRCMD_INIT, FrScoreForm::OnBlankPanel01Init)
ON_FRESH_VI("blankpanel1", FRCMD_OWNERDRAW,
	FrScoreForm::OnBlankPanel01OwnerDraw)
ON_FRESH_VI("blankpanel2", FRCMD_INIT, FrScoreForm::OnBlankPanel02Init)
ON_FRESH_VI("blankpanel2", FRCMD_OWNERDRAW,
	FrScoreForm::OnBlankPanel02OwnerDraw)
ON_FRESH_VI("blankpanel3", FRCMD_INIT, FrScoreForm::OnBlankPanel03Init)
ON_FRESH_VI("blankpanel3", FRCMD_OWNERDRAW,
	FrScoreForm::OnBlankPanel03OwnerDraw)
END_FRESH_MSGMAP()

FrScoreForm::FrScoreForm()
{
	m_state = STATE_NORMAL;
	m_pFront9Btn = NULL;
	m_pBack9Btn = NULL;
	m_pHoleList = NULL;
	m_pParList = NULL;
	m_bFront9 = true;
	m_pCancelBtn = NULL;
	m_pCloseBtn = NULL;
	m_pBar2 = NULL;
	m_pBar3 = NULL;
	m_pPPL = NULL;
	if (Doc()->m_golfGame.gameType == 7)
		m_type = 4;
	else if (Doc()->m_golfGame.gameType == 6 ||
		Doc()->m_golfGame.gameType == 1)
		m_type = 5;
	else
		m_type = 0;
	m_aniTime = 0;
	m_pPoint = NULL;
	memset(m_pTypeBtn, 0, sizeof(m_pTypeBtn));
	memset(m_pList, 0, sizeof(m_pList));
	memset(m_order, 0, sizeof(m_order));
	memset(m_pDigit, 0, sizeof(m_pDigit));
	m_pang = 0;
	m_bonusPang = 0;
	m_playerCount = 1;
	m_bShowBlankPanel = Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX;
	m_pBlankPanel[0] = NULL;
	m_pBlankPanel[1] = NULL;
	m_pBlankPanel[2] = NULL;
}

void FrScoreForm::LoadBitmaps()
{
	if (Doc()->m_golfGame.gameType == 1)
	{
		for (int i = 0; i < 2; i++)
		{
			const Bitmap* pBitmap = g_pFresh->GetManager()->GetBitmap(
				"SCORECARD", i == 0 ? "red" : "blue");
			if (pBitmap)
				m_bitmap.insert(
					std::map<unsigned long, const Bitmap*>::value_type(i,
						pBitmap));
		}
		m_pWin = g_pFresh->GetManager()->GetBitmap("SCORECARD", "win");
		m_pLose = g_pFresh->GetManager()->GetBitmap("SCORECARD", "lose");
		m_pDraw = g_pFresh->GetManager()->GetBitmap("SCORECARD", "draw");
		m_pNoResult = g_pFresh->GetManager()->GetBitmap("SCORECARD", "-");
		m_pQuestion =
			g_pFresh->GetManager()->GetBitmap("SCORECARD", "question");
	}
	else if (Doc()->m_golfGame.gameType == 6)
	{
		m_pWin = g_pFresh->GetManager()->GetBitmap("SCORECARD", "win");
		m_pLose = g_pFresh->GetManager()->GetBitmap("SCORECARD", "lose");
		m_pDraw = g_pFresh->GetManager()->GetBitmap("SCORECARD", "draw");
		m_pNoResult = g_pFresh->GetManager()->GetBitmap("SCORECARD", "-");
		m_pQuestion =
			g_pFresh->GetManager()->GetBitmap("SCORECARD", "question");
	}
	else
	{
		for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (PLAYER(i)->state == 3)
				continue;
			unsigned long charTid = Doc()->m_userInfo[i].charInfo.tid;
			if (m_bitmap.find(charTid) != m_bitmap.end())
				continue;
			IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(charTid);
			if (!pChar)
				continue;
			const Bitmap* pBitmap =
				g_pFresh->GetManager()->GetBitmap("ITEMS", pChar->c.Icon);
			if (pBitmap)
				m_bitmap.insert(
					std::map<unsigned long, const Bitmap*>::value_type(charTid,
						pBitmap));
		}
		char name[8];
		for (int rank = 0; rank < 4; rank++)
		{
			switch (rank)
			{
			case 0:
				strcpy(name, "1st");
				break;
			case 1:
				strcpy(name, "2nd");
				break;
			case 2:
				strcpy(name, "3rd");
				break;
			case 3:
				strcpy(name, "4th");
				break;
			}
			const Bitmap* pBitmap =
				g_pFresh->GetManager()->GetBitmap("SCORECARD", name);
			if (pBitmap)
				m_bitmap.insert(
					std::map<unsigned long, const Bitmap*>::value_type(rank,
						pBitmap));
			switch (rank)
			{
			case 0:
				strcpy(name, "1st_01");
				break;
			case 1:
				strcpy(name, "2nd_01");
				break;
			case 2:
				strcpy(name, "3rd_01");
				break;
			case 3:
				strcpy(name, "4th_01");
				break;
			}
			pBitmap = g_pFresh->GetManager()->GetBitmap("SCORECARD", name);
			if (pBitmap)
				m_bitmap.insert(
					std::map<unsigned long, const Bitmap*>::value_type(rank + 4,
						pBitmap));
		}
	}
	for (int i = 0; i < 10; i++)
		m_pDigit[i] =
			g_pFresh->GetManager()->GetBitmap("SCORECARD", MakeStr("%d", i));
	m_pPlus = g_pFresh->GetManager()->GetBitmap("SCORECARD", "+");
	m_pMinus = g_pFresh->GetManager()->GetBitmap("SCORECARD", "-");
	m_pPoint = g_pFresh->GetManager()->GetBitmap("SCORECARD", "point");
	m_pArrow = g_pFresh->GetManager()->GetBitmap("SCORECARD", "arrow");
	m_pUnused1f0_983 = NULL; // 983 78b1c9
	m_pPCIcon = g_pFresh->GetManager()->GetBitmap("golf", "premium_score");
}

void FrScoreForm::SetState(eState state)
{
	if (m_state == state)
		return;
	m_state = state;
	if (state == STATE_HIDE)
	{
		Enable(false);
		SetVisible(false);
	}
	else
	{
		Enable(true);
		SetVisible(true);
		if (Doc()->m_golfGame.holes == 18)
		{
			CGolfDoc* pGolfDoc = Doc()->m_pGolfDoc;
			m_bFront9 = GetHoleIndex(pGolfDoc->m_currentHole) < 10 ||
					pGolfDoc->m_bNoStat
				? true
				: false;
		}
		else
			m_bFront9 = true;
		SetFrontBackBtn();
	}
	if (m_pPPL)
		m_pPPL->SetVisible(m_bPPL); // 983 (007868c0): sem o teste de estado final do 645
}

// 983 (00782540): sem o caso 3 do 645 (largura 547); o 3 cai no padrao (612).
void FrScoreForm::Resize()
{
	float listWidth = 0.0f;
	float itemHeight = 0.0f;
	WRect rect = GetRect();
	if (m_pList[0])
	{
		listWidth = 139.0f;
		itemHeight = (float)m_pList[0]->GetItemHeight();
	}
	CSharedDoc* pDoc = Doc();
	switch (pDoc->m_golfGame.gameType)
	{
	case 1:
	case 6:
		rect.w = 547.0f;
		rect.h = itemHeight + itemHeight + listWidth;
		break;
	case 7:
		rect.w = 648.0f;
		rect.h = (float)((float)pDoc->m_pGolfDoc->m_playerNum * itemHeight +
					 listWidth) +
			22.0;
		break;
	default:
		rect.w = 612.0f;
		rect.h = (float)pDoc->m_pGolfDoc->GetCurrentPlayerNum() * itemHeight +
			listWidth;
		break;
	}
	ResizeScoreBar23(m_pBar2, m_pBar3, rect.x, rect.w);
	SetRect(rect);
	WPoint pos;
	pos.x = (float)(int)(float)((g_view->GetWidth() - rect.w) * 0.5);
	pos.y =
		(float)(int)(float)((itemHeight * 4.0 + listWidth - rect.h) * 0.5) +
		20.0;
	MoveWindow(pos);
}

void FrScoreForm::SortLists(bool force)
{
	// 983 (00786980): so o modo 0 tem ramo proprio; o 3 cai no else (mesma conta).
	if (Doc()->m_golfGame.gameType == 0)
		m_playerCount = Doc()->m_pGolfDoc->GetCurrentPlayerNum();
	else if (Doc()->m_golfGame.gameType == 1 || Doc()->m_golfGame.gameType == 6)
		m_playerCount = 2;
	else if (Doc()->m_golfGame.gameType == 7)
		m_playerCount = Doc()->m_pGolfDoc->m_playerNum;
	else
		m_playerCount = Doc()->m_pGolfDoc->GetCurrentPlayerNum();
	if (force)
		Resize();
	int i;
	for (i = 0; i < 3; i++)
	{
		if (m_pList[i])
			m_pList[i]->ClearItem();
	}
	SortPlayerOrder(m_order);
	for (i = 0; i < 3; i++)
	{
		if (m_pList[i])
		{
			for (int j = 0; j < m_playerCount; j++)
				m_pList[i]->AddItem((void*)m_order[j]);
		}
	}
}

void FrScoreForm::ChangeTurn()
{
	UpdatePang();
}

__declspec(dllimport) __forceinline sTeamData* GetTeamOf(CGolfDoc* pGolf,
	unsigned char team)
{
	return pGolf->GetTeam(team);
}

__declspec(dllimport) __forceinline unsigned char GetPlayerTeam(
	sPlayerData* pPlayer)
{
	return pPlayer->team;
}

#define RB_CURRENT_PLAYER(d) ((d)->GetCurrentPlayer())

void FrScoreForm::UpdatePang()
{
	if (0)
		((FrScoreForm*)0)->FrScoreForm::SortLists(*(bool*)0);
	CGolfDoc* pGolf = Doc()->m_pGolfDoc;
	if (Doc()->m_golfGame.gameType == 1)
	{
		sPlayerData* pPlayer = pGolf->GetPlayer(RB_CURRENT_PLAYER(pGolf));
		sTeamData* pTeam = GetTeamOf(Doc()->m_pGolfDoc, GetPlayerTeam(pPlayer));
		m_pang = (int)pTeam->pang[Doc()->m_pGolfDoc->m_currentHole - 1];
		pPlayer =
			Doc()->m_pGolfDoc->GetPlayer(RB_CURRENT_PLAYER(Doc()->m_pGolfDoc));
		pTeam = GetTeamOf(Doc()->m_pGolfDoc, GetPlayerTeam(pPlayer));
		m_bonusPang = (int)pTeam->totalPang;
	}
	else
	{
		sPlayerData* pPlayer = pGolf->GetPlayer(RB_CURRENT_PLAYER(pGolf));
		m_pang = (int)pPlayer->pang[Doc()->m_pGolfDoc->m_currentHole - 1];
		pPlayer =
			Doc()->m_pGolfDoc->GetPlayer(RB_CURRENT_PLAYER(Doc()->m_pGolfDoc));
		m_bonusPang = (int)pPlayer->totalPang;
	}
}

// 983 (00788d70): so repassa ao UpdatePang (o cl faz `jmp`); chamado pelo CScore::HandleMsg na mensagem 0x212
void FrScoreForm::UpdatePang983()
{
	UpdatePang();
}

void FrScoreForm::OnBar2Init(int param)
{
	m_pBar2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrScoreForm::OnBar3Init(int param)
{
	m_pBar3 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrScoreForm::OnFront9BtnInit(int param)
{
	m_pFront9Btn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (Doc()->m_golfGame.holes != 18)
		m_pFront9Btn->SetVisible(false);
}

void FrScoreForm::OnFront9BtnUp()
{
	if (Doc()->m_golfGame.holes == 18)
	{
		m_bFront9 = true;
		SetFrontBackBtn();
	}
}

void FrScoreForm::OnBack9BtnInit(int param)
{
	m_pBack9Btn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (Doc()->m_golfGame.holes != 18)
		m_pBack9Btn->SetVisible(false);
}

void FrScoreForm::OnBack9BtnUp()
{
	if (Doc()->m_golfGame.holes == 18)
	{
		m_bFront9 = false;
		SetFrontBackBtn();
	}
}

void FrScoreForm::OnShotBtnUp()
{
	if (Doc()->m_golfGame.gameType == 7)
	{
		if (m_type != 4)
		{
			m_type = 4;
			SetTypeBtn();
		}
	}
	else if (m_type != 0)
	{
		m_type = 0;
		SetTypeBtn();
	}
}

void FrScoreForm::OnGraphBtnUp()
{
	if (m_type != 1)
	{
		m_type = 1;
		SetTypeBtn();
	}
}

void FrScoreForm::OnVersusBtnUp()
{
	if (m_type != 5)
	{
		m_type = 5;
		SetTypeBtn();
	}
}

void FrScoreForm::OnPuttBtnUp()
{
	if (m_type != 2)
	{
		m_type = 2;
		SetTypeBtn();
	}
}

void FrScoreForm::OnPangBtnUp()
{
	if (m_type != 3)
	{
		m_type = 3;
		SetTypeBtn();
	}
}

void FrScoreForm::OnShotBtnInit(int param)
{
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pTypeBtn[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		if (m_pTypeBtn[4])
		{
			m_pTypeBtn[4]->SetButtonImg("skin_n", FrButton::NORMAL);
			m_pTypeBtn[4]->SetButtonImg("skin_o", FrButton::OVER);
			m_pTypeBtn[4]->SetButtonImg("skin_o", FrButton::PRESSED);
		}
	}
	else
		m_pTypeBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (Doc()->m_golfGame.gameType == 6 || Doc()->m_golfGame.gameType == 1)
		m_pTypeBtn[0]->SetVisible(false);
}

void FrScoreForm::OnGraphBtnInit(int param)
{
	m_pTypeBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pTypeBtn[1] &&
		(Doc()->m_golfGame.gameType == 6 || Doc()->m_golfGame.gameType == 1 ||
			Doc()->m_golfGame.gameType == 7))
		m_pTypeBtn[1]->SetVisible(false);
}

void FrScoreForm::OnPuttBtnInit(int param)
{
	m_pTypeBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pTypeBtn[2] &&
		(Doc()->m_golfGame.gameType == 6 || Doc()->m_golfGame.gameType == 1 ||
			Doc()->m_golfGame.gameType == 7))
		m_pTypeBtn[2]->SetVisible(false);
}

void FrScoreForm::OnPangBtnInit(int param)
{
	m_pTypeBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrScoreForm::OnVersusBtnInit(int param)
{
	m_pTypeBtn[5] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!m_pTypeBtn[5])
		return;
	if (Doc()->m_golfGame.gameType == 6 || Doc()->m_golfGame.gameType == 1)
	{
		if (Doc()->m_golfGame.gameType == 1)
		{
			WPoint pos = m_pTypeBtn[0]->GetRect().TopLeft();
			m_pTypeBtn[5]->MoveWindow(pos);
		}
		m_pTypeBtn[5]->SetVisible(true);
	}
	else
		m_pTypeBtn[5]->SetVisible(false);
}

void FrScoreForm::OnHoleListInit(int param)
{
	m_pHoleList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pHoleList)
		m_pHoleList->AddItem(NULL);
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pHoleList->SetItemSize(470, 22);
		const WRect& rect = m_pHoleList->GetRect();
		m_pHoleList->SetRect(
			WRect(WPoint(rect.x - 22.0f, rect.y + 0.0f), rect.w + 0.0f, rect.h + 0.0f));
	}
}

void FrScoreForm::OnHoleListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (m_bShowBlankPanel || !item)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pHoleList->GetItemWidth();
	float height = (float)m_pHoleList->GetItemHeight();
	// 983 (00782bb0): passo w / 9.0 em double; meio passo e a altura com 0.5/7.0 em double.
	double step = width / 9.0;
	WPoint pos;
	pos.x = (float)(step * 0.5 + item->pos.x);
	pos.y = (float)(0.5 * height + item->pos.y - 7.0);
	int start = m_bFront9 ? 0 : 9;
	int end = min(start + 9, Doc()->m_golfGame.holes);
	for (int i = start; i < end; i++)
	{
		DrawDigit(gdi, MakeStr("%d", Doc()->m_holeOrder[i]), pos, 0xffffffff,
			(FrGDI::eTextAlign)1, false);
		float next = (float)(pos.x + step);
		pos.x = (float)(int)next;
	}
}

void FrScoreForm::OnParListInit(int param)
{
	m_pParList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pParList)
		m_pParList->AddItem(NULL);
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pParList->SetItemSize(470, 22);
		const WRect& rect = m_pParList->GetRect();
		m_pParList->SetRect(
			WRect(WPoint(rect.x - 22.0f, rect.y + 0.0f), rect.w + 0.0f, rect.h + 0.0f));
	}
}

void FrScoreForm::OnParListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (m_bShowBlankPanel || !item)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pParList->GetItemWidth();
	float height = (float)m_pParList->GetItemHeight();
	// 983 (00782de0): passo w / 9.0 e meio passo em double, como no OnHoleListOwnerDraw.
	double step = width / 9.0;
	double half = step * 0.5;
	WPoint pos;
	pos.x = (float)(half + item->pos.x);
	pos.y = (float)(0.5 * height + item->pos.y - 7.0);
	int start = m_bFront9 ? 0 : 9;
	int end = min(start + 9, GetHoles());
	int total = 0;
	for (int i = start; i < end; i++)
	{
		total += GOLFDOC()->GetHoleData(Doc()->m_holeOrder[i]).par;
		DrawDigit(gdi,
			MakeStr("%d", GOLFDOC()->GetHoleData(Doc()->m_holeOrder[i]).par),
			pos, 0xff9ff5b8, (FrGDI::eTextAlign)1, false);
		float next = (float)(pos.x + step);
		pos.x = (float)(int)next;
	}
	switch (Doc()->m_golfGame.gameType)
	{
	// 983: so 1 e 6 ficam sem o total (o 3 desenha); a coluna do total trunca com (int).
	case 1:
	case 6:
		break;
	case 7:
	{
		float x = (float)(half + item->pos.x + 405.0 + 23.0 + 45.0);
		pos.x = (float)(int)x;
		DrawDigit(gdi, MakeStr("%d", total), pos, 0xff9ff5b8,
			(FrGDI::eTextAlign)1, false);
		break;
	}
	default:
	{
		float x = (float)(half + item->pos.x + 342.0 + 13.0);
		pos.x = (float)(int)x;
		DrawDigit(gdi, MakeStr("%d", total), pos, 0xff9ff5b8,
			(FrGDI::eTextAlign)1, false);
		break;
	}
	}
}

void FrScoreForm::OnPangListInit(int param)
{
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pPangList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
		m_pPangList->AddItem(NULL);
		m_pPangList->SetItemSize(470, 22);
	}
}

void FrScoreForm::OnBlankPanel01Init(int param)
{
	m_pBlankPanel[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBlankPanel[0])
		m_pBlankPanel[0]->SetVisible(m_bShowBlankPanel);
}

void FrScoreForm::OnBlankPanel01OwnerDraw(int param)
{
	if (m_pBlankPanel[0])
		m_pBlankPanel[0]->SetVisible(m_bShowBlankPanel);
}

void FrScoreForm::OnBlankPanel02Init(int param)
{
	m_pBlankPanel[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBlankPanel[1])
		m_pBlankPanel[1]->SetVisible(m_bShowBlankPanel);
}

void FrScoreForm::OnBlankPanel02OwnerDraw(int param)
{
	if (m_pBlankPanel[1])
		m_pBlankPanel[1]->SetVisible(m_bShowBlankPanel);
}

void FrScoreForm::OnBlankPanel03Init(int param)
{
	m_pBlankPanel[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBlankPanel[2])
		m_pBlankPanel[2]->SetVisible(m_bShowBlankPanel);
}

void FrScoreForm::OnBlankPanel03OwnerDraw(int param)
{
	if (m_pBlankPanel[2])
		m_pBlankPanel[2]->SetVisible(m_bShowBlankPanel);
}

void FrScoreForm::OnPangListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pPangList->GetItemWidth();
	int height = m_pPangList->GetItemHeight();
	// 983 (007830c0): passo w / 9.0 e meio passo em double, como no OnParListOwnerDraw.
	double step = width / 9.0;
	double half = step * 0.5;
	WPoint pos;
	pos.x = (float)(half + item->pos.x);
	pos.y = (float)(0.5 * height + item->pos.y - 7.0);
	int start = m_bFront9 ? 0 : 9;
	int end = min(start + 9, Doc()->m_golfGame.holes);
	for (int i = start; i < end; i++)
	{
		unsigned char carry =
			Doc()->m_pGolfDoc->GetHoleData(Doc()->m_holeOrder[i]).carry;
		unsigned long color = (carry ? 0xff0000 : 0) + 0xff000000;
		gdi->SetTextStyle(1);
		gdi->SetTextColor(color, 0xffffffff);
		gdi->PrintText(pos, 1,
			MakeStr("%I64d",
				(int)Doc()
					->m_pGolfDoc->GetHoleData(Doc()->m_holeOrder[i])
					.pang),
			NULL);
		float next = (float)(pos.x + step);
		pos.x = (float)(int)next;
	}
	if (Doc()->m_golfGame.gameType == 7)
	{
		// 983: total na coluna 405+23+45, truncado com (int) (era float2int).
		float x = (float)(half + item->pos.x + 405.0 + 23.0 + 45.0);
		pos.x = (float)(int)x;
		gdi->SetTextStyle(1);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->PrintText(pos, 1,
			MakeStr("%I64d", *(__int64*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x1c8))),
			NULL);
	}
}

void FrScoreForm::OnGolferListInit(int param)
{
	m_pList[0] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	for (int i = 0; i < m_playerCount; i++)
		m_pList[0]->AddItem((void*)m_order[i]);
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pList[0]->SetItemSize(103, 58);
		const WRect& rect = m_pList[0]->GetRect();
		m_pList[0]->SetRect(WRect(WPoint(rect.x - 1.0f, rect.y + 22.0f),
			WSize(rect.w + 0.0f, rect.h + 0.0f)));
	}
}

void FrScoreForm::OnGolferListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	unsigned char index = (unsigned char)(int)item->pData;
	unsigned long pos;
	for (pos = 0; (int)pos < m_playerCount; pos++)
	{
		if (m_order[pos] == index)
			break;
	}
	if (Doc()->m_golfGame.gameType == 1)
	{
		unsigned char me;
		if (OnlinePlay() == 0)
			me = *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189));
		else
			me = MyIndex();
		if (index == Doc()->m_pGolfDoc->GetPlayer(me)->team)
			gdi->Box(WRect(item->pos.x, item->pos.y,
						 (float)m_pList[0]->GetItemWidth(),
						 (float)m_pList[0]->GetItemHeight()),
				0x809b9dff);
		std::map<unsigned long, const Bitmap*>::iterator it =
			m_bitmap.find(index); // 983 00792d00: red/blue pela chave do time, nao pela posicao
		if (it != m_bitmap.end() && (*it).second)
		{
			const Bitmap* pBmp = (*it).second;
			gdi->DrawTexture(pBmp,
				WRect(item->pos.x, item->pos.y + 3.0f, (float)pBmp->Width(),
					(float)pBmp->Height()),
				0xffffffff, 0);
		}
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(0);
		float x = item->pos.x;
		float y = item->pos.y + 28.0f;
		for (unsigned char p = 0; p < Doc()->m_pGolfDoc->m_playerNum; p++)
		{
			if (Doc()->m_pGolfDoc->GetPlayer(p)->team != index)
				continue;
			if (Doc()->m_userInfo[p].info.dwGuildId != 0 &&
				!Doc()->m_userInfo[p].info.IsIdentity(0x14))
			{
				const Bitmap* pEmblem =
					NetResourceManager::Instance()->GetEmblem983(
						Doc()->m_userInfo[p].info.dwGuildId, Doc()->m_userInfo[p].info.dwEmblemVer);
				if (pEmblem)
					gdi->DrawTexture(pEmblem,
						WRect(x, y - 6.0f, (float)pEmblem->Width(),
							(float)pEmblem->Height()),
						0xffffffff, 0);
			}
			if (!CProjectG::Instance()->HidePrivacy())
				g_pFresh->GetManager()->PrintText(WPoint(x + 27.0f, y), 0,
					Doc()->m_userInfo[p].info.sNick, 100.0f, 0xffffffff);
			y += 16.0f;
		}
		return;
	}
	unsigned long boxColor = 0;
	if (OnlinePlay() == 0)
	{
		if (SDOC983(unsigned char, 0x10594) == 7 &&
			Doc()->m_pGolfDoc->GetPlayer(index)->state == 3)
			gdi->Box(WRect(item->pos.x, item->pos.y,
						 (float)m_pList[0]->GetItemWidth(),
						 (float)m_pList[0]->GetItemHeight()),
				0x80808080);
		if (index == *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
			boxColor = 0x809b9dff;
	}
	else
	{
		unsigned long myUid = ((Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
			? Doc()->m_myInfo.info.dwGalleryGuid
			: Doc()->m_myInfo.info.dwGuid;
		if (SDOC983(unsigned char, 0x10594) == 6)
			boxColor = 0x809b9dff;
		else if (SDOC983(unsigned char, 0x10594) == 7)
		{
			if (Doc()->m_pGolfDoc->GetPlayer(index)->state == 3)
				boxColor = 0x80808080;
			else if (Doc()->m_pGolfDoc->GetPlayer(index)->oid == myUid)
				boxColor = 0x809b9dff;
		}
		else
		{
			if (Doc()->m_pGolfDoc->GetPlayer(index)->oid == myUid)
				boxColor = 0x809b9dff;
			else if (Doc()->m_pGolfDoc->GetPlayer(index)->state == 3)
				boxColor = 0x80808080;
		}
	}
	if (boxColor)
		gdi->Box(WRect(item->pos.x, item->pos.y,
					 (float)m_pList[0]->GetItemWidth(),
					 (float)m_pList[0]->GetItemHeight()),
			boxColor);
	WPoint text(item->pos.x, item->pos.y + 36.0f);
	if (SDOC983(unsigned char, 0x10594) == 6)
	{
		unsigned char idx =
			Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4, 0x10fd4 + index * 0x4));
		int rec = (unsigned int)idx * 0x1b8 + SDOC983(int, 0x1042c);
		if (*(int*)(rec + 0x174) != 0)
		{
			const Bitmap* pEmblem =
				NetResourceManager::Instance()->GetEmblem983(
					*(int*)(rec + 0x174), *(int*)(rec + 0x178));
			if (pEmblem)
				gdi->DrawTexture(pEmblem,
					WRect(text.x, text.y - 6.0f, (float)pEmblem->Width(),
						(float)pEmblem->Height()),
					0xffffffff, 0);
		}
		text.x += 27.0f;
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(0);
		if (CProjectG::Instance()->HidePrivacy())
			g_pFresh->GetManager()->PrintText(text, 0, (char*)(rec + 0x8), 100.0f,
				0xffffffff);
	}
	else if (SDOC983(unsigned char, 0x10594) == 7)
	{
		if (Doc()->m_pGolfDoc->GetPlayer(index)->state == 3)
			pos += 4;
		std::map<unsigned long, const Bitmap*>::iterator it =
			m_bitmap.find(pos);
		if (it != m_bitmap.end() && (*it).second)
		{
			const Bitmap* pBmp = (*it).second;
			WRect rect(item->pos.x + 43.0f, item->pos.y + 12.0f,
				(float)pBmp->Width(), (float)pBmp->Height());
			gdi->DrawTexture(pBmp, rect, 0xffffffff, 0);
			if (m_pPCIcon &&
				((SDOCM(unsigned int, m_userInfo, 0x38b0, index * 0x31c6 + 0x3922) >> 14) & 1))
				gdi->DrawTexture(m_pPCIcon,
					WRect(item->pos.x, rect.y - 10.0f,
						(float)m_pPCIcon->Width(), (float)m_pPCIcon->Height()),
					0xffffffff, 0);
		}
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(0);
		text.x += 52.0f;
		if (!CProjectG::Instance()->HidePrivacy())
			g_pFresh->GetManager()->PrintText(text, 1,
				Doc()->m_userInfo[index].info.sNick, 100.0f, 0xffffffff);
	}
	else
	{
		std::map<unsigned long, const Bitmap*>::iterator it =
			m_bitmap.find(pos);
		if (it != m_bitmap.end() && (*it).second)
		{
			const Bitmap* pBmp = (*it).second;
			WRect rect(item->pos.x + 43.0f, item->pos.y + 12.0f,
				(float)pBmp->Width(), (float)pBmp->Height());
			gdi->DrawTexture(pBmp, rect, 0xffffffff, 0);
			if (m_pPCIcon &&
				((SDOCM(unsigned int, m_userInfo, 0x38b0, index * 0x31c6 + 0x3922) >> 14) & 1))
				gdi->DrawTexture(m_pPCIcon,
					WRect(item->pos.x, rect.y - 10.0f,
						(float)m_pPCIcon->Width(), (float)m_pPCIcon->Height()),
					0xffffffff, 0);
		}
		unsigned char dlgType = SDOC983(unsigned char, 0x109b8);
		bool hideTag = (dlgType == 12 || dlgType == 14 || dlgType == 19);
		if (Doc()->m_userInfo[index].info.dwGuildId != 0 &&
			!Doc()->m_userInfo[index].info.IsIdentity(0x14) && !hideTag)
		{
			const Bitmap* pEmblem =
				NetResourceManager::Instance()->GetEmblem983(
					Doc()->m_userInfo[index].info.dwGuildId, Doc()->m_userInfo[index].info.dwEmblemVer);
			if (pEmblem)
				gdi->DrawTexture(pEmblem,
					WRect(text.x, text.y - 6.0f, (float)pEmblem->Width(),
						(float)pEmblem->Height()),
					0xffffffff, 0);
		}
		text.x += 27.0f;
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(0);
		if (!CProjectG::Instance()->HidePrivacy() && !hideTag)
			g_pFresh->GetManager()->PrintText(text, 0,
				Doc()->m_userInfo[index].info.sNick, 100.0f, 0xffffffff);
	}
}

void FrScoreForm::OnRecordListInit(int param)
{
	m_pList[1] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pList[1])
	{
		for (int i = 0; i < m_playerCount; i++)
			m_pList[1]->AddItem((void*)m_order[i]);
	}
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pList[1]->SetItemSize(470, 58);
		const WRect& rect = m_pList[1]->GetRect();
		m_pList[1]->SetRect(WRect(WPoint(rect.x - 24.0f, rect.y + 22.0f),
			WSize(rect.w + 0.0f, rect.h + 0.0f)));
	}
}

void FrScoreForm::OnRecordListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	unsigned long index = (unsigned long)item->pData;
	if (Doc()->m_golfGame.gameType == 1)
	{
		// 983 00795c10: offline o "eu" e GolfDoc+0x189 (nao GetCurrentPlayerNum)
		if (OnlinePlay() != 0)
		{
			if (index == Doc()->m_pGolfDoc->GetPlayer(MyIndex())->team)
				g_pFresh->GetManager()->GetGDI()->Box(
					WRect(item->pos.x, item->pos.y, m_pList[1]->GetItemWidth(),
						m_pList[1]->GetItemHeight()),
					0x809b9dff);
		}
		else
		{
			if (index == Doc()->m_pGolfDoc->GetPlayer(
					*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))->team)
				g_pFresh->GetManager()->GetGDI()->Box(
					WRect(item->pos.x, item->pos.y, m_pList[1]->GetItemWidth(),
						m_pList[1]->GetItemHeight()),
					0x809b9dff);
		}
	}
	else if (OnlinePlay() != 0)
	{
		unsigned char player = (unsigned char)index;
		sPlayerData* pPlayer = Doc()->m_pGolfDoc->GetPlayer(player);
		unsigned long myUid =
			((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
			? Doc()->m_myInfo.info.dwGalleryGuid
			: Doc()->m_myInfo.info.dwGuid;
		if (pPlayer->oid == myUid)
			g_pFresh->GetManager()->GetGDI()->Box(
				WRect(item->pos.x, item->pos.y, m_pList[1]->GetItemWidth(),
					m_pList[1]->GetItemHeight()),
				0x809b9dff);
		else if (Doc()->m_pGolfDoc->GetPlayer(player)->state == 3)
			g_pFresh->GetManager()->GetGDI()->Box(
				WRect(item->pos.x, item->pos.y, m_pList[1]->GetItemWidth(),
					m_pList[1]->GetItemHeight()),
				0x80808080);
	}
	else
	{
		if (Doc()->m_pGolfDoc->GetPlayer((unsigned char)index)->state == 3)
			g_pFresh->GetManager()->GetGDI()->Box(
				WRect(item->pos.x, item->pos.y, m_pList[1]->GetItemWidth(),
					m_pList[1]->GetItemHeight()),
				0x80808080);
		if (index == *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
			g_pFresh->GetManager()->GetGDI()->Box(
				WRect(item->pos.x, item->pos.y, m_pList[1]->GetItemWidth(),
					m_pList[1]->GetItemHeight()),
				0x809b9dff);
	}
	switch (m_type)
	{
	case 0:
		DrawShot(item);
		break;
	case 1:
		DrawGraph(item);
		break;
	case 2:
		DrawPutt(item);
		break;
	case 3:
		DrawPang(item);
		break;
	case 4:
		DrawSkins(item);
		break;
	case 5:
		DrawVersus(item);
		break;
	}
}

// HACK: stands for WRect::operator+(WRect&), which wtypes.h (PCH) does not declare;
// dllimport + forceinline is always inlined and never emitted out of line.
struct RbRectAdd : public WRect
{
	__declspec(dllimport) __forceinline WRect operator+(WRect& r) const
	{
		return WRect(x + r.x, y + r.y, w + r.w, h + r.h);
	}
};

void FrScoreForm::OnTotalListInit(int param)
{
	m_pList[2] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (!m_pList[2])
		return;
	for (int i = 0; i < m_playerCount; i++)
		m_pList[2]->AddItem((void*)m_order[i]);
	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pList[2]->SetItemSize(63, 58);
		m_pList[2]->SetRect((const RbRectAdd&)m_pList[2]->GetRect() +
			WRect(104.0f, 22.0f, 0.0f, 0.0f));
	}
}

sTeamData* TEAM(unsigned char index);
sRivalData* GUILDMATCHUP(unsigned char index);

void FrScoreForm::OnTotalListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	int itemHeight = m_pList[2]->GetItemHeight();
	float width = (float)m_pList[2]->GetItemWidth();
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	float height = (float)itemHeight;
	unsigned long data = (unsigned long)item->pData;
	unsigned char index = (unsigned char)data;
	_WRECT rect;
	if (SDOC983(unsigned char, 0x10594) == 1)
	{
		unsigned char me;
		if (OnlinePlay() == 0)
			me = *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189));
		else
			me = MyIndex();
		if (data == Doc()->m_pGolfDoc->GetPlayer(me)->team)
		{
			rect.x = item->pos.x;
			rect.y = item->pos.y;
			rect.w = (float)m_pList[2]->GetItemWidth() * 0.5f;
			rect.h = (float)m_pList[2]->GetItemHeight();
			gdi->Box(rect, 0x809b9dff);
		}
	}
	else
	{
		unsigned long boxColor;
		bool draw = true;
		if (OnlinePlay() == 0)
		{
			if (Doc()->m_pGolfDoc->GetPlayer(index)->state == 3)
			{
				rect.x = item->pos.x;
				rect.y = item->pos.y;
				rect.w = (float)m_pList[2]->GetItemWidth();
				rect.h = (float)m_pList[2]->GetItemHeight();
				gdi->Box(rect, 0x80808080);
			}
			if (data != *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
				draw = false;
			boxColor = 0x809b9dff;
		}
		else
		{
			unsigned long myUid = ((Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
				? Doc()->m_myInfo.info.dwGalleryGuid
				: Doc()->m_myInfo.info.dwGuid;
			if (Doc()->m_pGolfDoc->GetPlayer(index)->oid == myUid)
				boxColor = 0x809b9dff;
			else if (Doc()->m_pGolfDoc->GetPlayer(index)->state == 3)
				boxColor = 0x80808080;
			else
				draw = false;
		}
		if (draw)
		{
			rect.x = item->pos.x;
			rect.y = item->pos.y;
			rect.w = (float)m_pList[2]->GetItemWidth();
			rect.h = (float)m_pList[2]->GetItemHeight();
			// 983 00793720: so o tipo 6 usa meia largura (o 3 nao)
			if (SDOC983(unsigned char, 0x10594) == 6)
				rect.w = rect.w * 0.5f;
			gdi->Box(rect, boxColor);
		}
	}
	float y = (height * 0.5f + item->pos.y) - 6.0f;
	float scale = 0.25f;
	if (SDOC983(unsigned char, 0x10594) == 7)
		scale = 0.5f;
	float x = width * scale + item->pos.x;
	WPoint pos(x, y);
	char* text;
	unsigned long value;
	switch (m_type)
	{
	case 1:
		if (SDOC983(unsigned char, 0x10594) == 6 ||
			SDOC983(unsigned char, 0x10594) == 1 ||
			SDOC983(unsigned char, 0x10594) == 7)
			break;
	case 0:
		if (SDOC983(unsigned char, 0x10594) == 6)
			DrawDigit(gdi, "?", pos, 0xffffffff, (FrGDI::eTextAlign)1, false);
		else
		{
			value = PLAYER(index)->totalStroke;
			DrawDigit(gdi, MakeStr("%d", value), pos, 0xffffffff,
				(FrGDI::eTextAlign)1, false);
		}
		break;
	case 2:
	{
		if (SDOC983(unsigned char, 0x10594) == 6 ||
			SDOC983(unsigned char, 0x10594) == 1 ||
			SDOC983(unsigned char, 0x10594) == 7)
			break;
		unsigned long puttTotal = 0;
		int holes = m_bFront9 ? 9 : 18;
		for (int j = 0; j < holes; j++)
			puttTotal += (unsigned char)PLAYER(index)->putt[j];
		DrawDigit(gdi, MakeStr("%d", puttTotal), pos, 0xffffffff,
			(FrGDI::eTextAlign)1, false);
		break;
	}
	case 3:
		if (SDOC983(unsigned char, 0x10594) == 1)
		{
			if (data ==
				PLAYER(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))->team)
				// 983 00793720: o total do jogador atual = +0x188 (totalPang do UpdatePang), nao o pang do buraco +0x180
				DrawDigit(gdi, MakeStr("%I64d", m_bonusPang), pos, 0xffffffff,
					(FrGDI::eTextAlign)1, false);
			else
				DrawDigit(gdi, MakeStr("%d", (int)TEAM(index)->totalPang), pos,
					0xffffffff, (FrGDI::eTextAlign)1, false);
		}
		else if (SDOC983(unsigned char, 0x10594) == 6)
			DrawDigit(gdi, MakeStr("%I64d", GUILDMATCHUP(index)->totalPang),
				pos, 0xffffffff, (FrGDI::eTextAlign)1, false);
		else if (data == *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
			DrawDigit(gdi, MakeStr("%I64d", m_bonusPang), pos, 0xffffffff,
				(FrGDI::eTextAlign)1, false);
		else
			DrawDigit(gdi, MakeStr("%d", (int)PLAYER(index)->totalPang), pos,
				0xffffffff, (FrGDI::eTextAlign)1, false);
		break;
	case 4:
		gdi->SetTextStyle(1);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->PrintText(pos, 1, MakeStr("%I64d", Doc()->m_pGolfDoc->GetPlayer(index)->skinsSumPang),
			NULL);
		break;
	case 5:
		if (SDOC983(unsigned char, 0x10594) == 6)
			DrawDigit(gdi, MakeStr("%d", GUILDMATCHUP(index)->guildPoint), pos,
				0xffffffff, (FrGDI::eTextAlign)1, false);
		else if (SDOC983(unsigned char, 0x10594) == 1)
			DrawDigit(gdi, MakeStr("%d", TEAM(index)->score), pos, 0xffffffff,
				(FrGDI::eTextAlign)1, false);
		else
		{
			value = PLAYER(index)->totalStroke;
			DrawDigit(gdi, MakeStr("%d", value), pos, 0xffffffff,
				(FrGDI::eTextAlign)1, false);
		}
		break;
	}
	if (SDOC983(unsigned char, 0x10594) == 0 || IsMassGame() != 0)
	{
		pos.x = width * 0.75f + item->pos.x;
		text = Doc()->m_pGolfDoc->GetPlayer(index)->score != 0 ? "%+d" : "%d";
		DrawDigit(gdi,
			MakeStr(text, Doc()->m_pGolfDoc->GetPlayer(index)->score), pos,
			0xffffffff, (FrGDI::eTextAlign)1, false);
	}
}

bool FrScoreForm::OnInit()
{
	Enable(m_state != STATE_HIDE);
	SetVisible(m_state != STATE_HIDE);
	if (Doc()->m_golfGame.gameType == 7)
	{
		// 983 0078b230: soma (0, 0, 36, 0) campo a campo (fldz)
		const WRect& rect = GetRect();
		SetRect(WRect(rect.x + 0.0f, rect.y + 0.0f, rect.w + 36.0, rect.h + 0.0f));
	}
	SortLists(false);
	LoadBitmaps();
	return true;
}

void FrScoreForm::OnDraw()
{
	if (Doc()->m_golfGame.gameType != 7)
	{
		WPoint from;
		WPoint to;
		from.x = to.x = m_rect.x + 129.0f;
		from.y = m_rect.y + 116.0f;
		to.y = (float)m_playerCount * 58.0f + from.y;
		if (Doc()->m_golfGame.gameType == 7)
		{
			to.x = from.x = to.x - 24.0f;
			from.y += 22.0f;
			to.y += 22.0f;
		}
		int i;
		for (i = 0; i < 10; i++)
		{
			g_view->DrawLine2D(from, to, 0xff000000, 0xff000000, 0);
			from.x += 38.0f;
			to.x += 38.0f;
			if (Doc()->m_golfGame.gameType == 7)
			{
				from.x += 9.0f;
				to.x += 9.0f;
			}
		}
		from.x += 27.0f;
		to.x += 27.0f;
		if (Doc()->m_golfGame.gameType == 7)
		{
			from.x -= 13.0f;
			to.x -= 13.0f;
		}
		g_view->DrawLine2D(from, to, 0xff000000, 0xff000000, 0);
		from.y = to.y = m_rect.y + 174.0f;
		if (Doc()->m_golfGame.gameType == 7)
			to.y = from.y = to.y + 22.0f;
		for (i = 0; i < m_playerCount - 1; i++)
		{
			from.x = m_rect.x + 3.0f;
			to.x = m_rect.x + 28.0f;
			g_view->DrawLine2D(from, to, 0, 0xff000000, 0);
			from.x = m_rect.w + m_rect.x - 36.0f;
			to.x = m_rect.w + m_rect.x - 11.0f;
			g_view->DrawLine2D(from, to, 0xff000000, 0, 0);
			from.x = m_rect.x + 28.0f;
			to.x = m_rect.w + m_rect.x - 36.0f;
			g_view->DrawLine2D(from, to, 0xff000000, 0xff000000, 0);
			from.y += 58.0f;
			to.y += 58.0f;
		}
	}
}

void FrScoreForm::SetFrontBackBtn()
{
	m_aniTime = 0;
	if (Doc()->m_golfGame.holes == 18)
	{
		if (m_pFront9Btn)
		{
			m_pFront9Btn->SetVisible(true);
			m_pFront9Btn->SetStatus(
				m_bFront9 ? FrButton::PRESSED : FrButton::NORMAL);
		}
		if (m_pBack9Btn)
		{
			m_pBack9Btn->SetVisible(true);
			m_pBack9Btn->SetStatus(
				m_bFront9 ? FrButton::NORMAL : FrButton::PRESSED);
		}
	}
	else
	{
		if (m_pFront9Btn)
			m_pFront9Btn->SetVisible(false);
		if (m_pBack9Btn)
			m_pBack9Btn->SetVisible(false);
	}
}

void FrScoreForm::OnProc(const float deltaTime)
{
	if (m_state != STATE_HIDE && m_aniTime < 5.0f)
		m_aniTime += deltaTime;
}

void FrScoreForm::SetTypeBtn()
{
	m_aniTime = 0;
	for (int i = 0; i < 6; i++)
	{
		if (m_pTypeBtn[i])
			m_pTypeBtn[i]->SetStatus(
				m_type == i ? FrButton::PRESSED : FrButton::NORMAL);
	}
}

void FrScoreForm::DrawDigit(FrGraphicInterface* gdi, char* text, WPoint pos,
	unsigned long color, FrGDI::eTextAlign align, bool narrow)
{
	int len = strlen(text);
	const Bitmap* digits[16];
	int width = 0;
	if (align)
	{
		width = 0;
		for (int i = 0; i < len; ++i)
		{
			switch (text[i])
			{
			case '+':
				digits[i] = m_pPlus;
				break;
			case '-':
				digits[i] = m_pMinus;
				break;
			case '?':
				digits[i] = m_pQuestion;
				break;
			default:
				digits[i] = m_pDigit[text[i] - '0'];
				break;
			}
			width += digits[i]->Width() - (narrow ? (len < 4 ? 0 : 2) : 0);
		}
	}
	WRect rect;
	switch (align)
	{
	case 0:
		rect.x = (float)(int)pos.x;
		break;
	case 1:
		rect.x = (float)(int)(float)(pos.x - width * 0.5f);
		break;
	case 2:
		rect.x = (float)(int)(float)(pos.x - width);
		break;
	}
	rect.y = pos.y;
	for (int i = 0; i < len; ++i)
	{
		rect.w = (float)digits[i]->Width();
		rect.h = (float)digits[i]->Height();
		gdi->DrawTexture(digits[i], rect, color, 0);
		rect.x += rect.w - (narrow ? (len < 4 ? 0 : 2) : 0);
	}
}

// 983: m_holeOrder guarda o buraco a partir de 1 -> stroke/putt/pang/skinsPang[ordem - 1]
// (DrawShot/Versus/Graph/Putt/Pang/Skins: +0x1c/+0x13c/+0x25c no 983; o nosso lia o buraco seguinte)
void FrScoreForm::DrawShot(FrListItem* item)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pList[1]->GetItemWidth();
	void* data = item->pData;
	int i = m_bFront9 ? 0 : 9;
	int shown = 1 - float2int(m_aniTime * -20.0f);
	int end;
	if ((shown > 8 ? 9 : shown) + i < SDOC983(unsigned char, 0x10699))
		end = (shown > 8 ? 9 : shown) + i;
	else
		end = SDOC983(unsigned char, 0x10699);
	float x = (float)float2int(width * (1.0f / 18.0f) + item->pos.x);
	float y = (float)float2int(
		0.5f * (float)m_pList[1]->GetItemHeight() + item->pos.y - 6.0f);
	if (SDOC983(unsigned char, 0x10594) == 6)
	{
		for (; i < end; i++)
		{
			unsigned char idx = Doc()->GetIndex(
				SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4, 0x10fd4 + ((unsigned int)data & 0xff) * 0x4));
			if (*(char*)(Doc()->m_holeOrder[i] + 0x7f +
					(unsigned int)idx * 0x1b8 + SDOC983(int, 0x1042c)) == 0)
			{
				if (i !=
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1)
					return;
			}
			char* text;
			unsigned long color;
			if (i ==
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1 &&
				*(char*)((unsigned int)Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
							 0x10fd4 + ((unsigned int)data & 0xff) * 4)) *
						0x1b8 +
					SDOC983(int, 0x1042c) + 0x36) != 1 &&
				((*(unsigned int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) &
					 0x22000) == 0 ||
					*((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x15c)) != 0))
			{
				color = 0xffffffff;
				text = "?";
			}
			else
			{
				unsigned char idx2 = Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
					0x10fd4 + ((unsigned int)data & 0xff) * 4));
				if (*(char*)(Doc()->m_holeOrder[i] + 0x7f +
						(unsigned int)idx2 * 0x1b8 + SDOC983(int, 0x1042c)) == 0 ||
					(*(int*)((unsigned int)Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
								 0x10fd4 + ((unsigned int)data & 0xff) * 4)) *
							 0x1b8 +
						 SDOC983(int, 0x1042c) + 0x4) != SDOC983(int, 0x81a) &&
						(int)GetHoleIndex(
							*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
								1 <
							i))
				{
					color = 0xffffffff;
					text = "?";
				}
				else
				{
					// 983 00793e60: par da tabela GolfDoc+0xd8 como nos outros modos (era +0xc8)
					unsigned char par =
						*(unsigned char*)(*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
							0x20 + (unsigned int)Doc()->m_holeOrder[i] * 0x38);
					unsigned int score =
						*(unsigned char*)(Doc()->m_holeOrder[i] + 0x7f +
							(unsigned int)Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
								0x10fd4 + ((unsigned int)data & 0xff) * 4)) *
								0x1b8 +
							SDOC983(int, 0x1042c));
					int diff = score - par;
					if (diff < -3)
						diff = -3;
					else if (diff > 3)
						diff = 3;
					switch (diff)
					{
					case 0:
						color = 0xffffffff;
						break;
					case 1:
						color = 0xffb9b9b9;
						break;
					case -4:
					case -3:
					case -2:
						color = 0xffff4f4f;
						break;
					case -1:
						color = 0xffffbd42;
						break;
					default:
						color = 0xff6f6f71;
					}
					text = MakeStr("%d", score);
				}
			}
			DrawDigit(gdi, text, WPoint(x, y), color, (FrGDI::eTextAlign)1,
				false);
			x = (float)float2int(width * (1.0f / 9.0f) + x);
		}
	}
	else
	{
		for (; i < end; i++)
		{
			unsigned char player = (unsigned char)(int)data;
			unsigned char hole = Doc()->m_holeOrder[i];
			if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
				(unsigned char)0)
			{
				if (i !=
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1)
					return;
			}
			char* text;
			unsigned long color;
			if (i ==
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1 &&
				Doc()->m_pGolfDoc->GetPlayer(player)->state != 1 &&
				((*(unsigned int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) &
					 0x22000) == 0 ||
					*((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x15c)) != 0))
			{
				color = 0xffffffff;
				text = "-";
			}
			else if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
				(unsigned char)0)
			{
				color = 0xffffffff;
				text = "-";
			}
			else
			{
				unsigned char par =
					*(unsigned char*)(*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
						0x20 + (unsigned int)hole * 0x38);
				unsigned int score = (unsigned char)Doc()
										 ->m_pGolfDoc->GetPlayer(player)
										 ->stroke[hole - 1];
				int diff = score - par;
				if (diff < -3)
					diff = -3;
				else if (diff > 3)
					diff = 3;
				switch (diff)
				{
				case 0:
					color = 0xffffffff;
					break;
				case 1:
					color = 0xffb9b9b9;
					break;
				case -4:
				case -3:
				case -2:
					color = 0xffff4f4f;
					break;
				case -1:
					color = 0xffffbd42;
					break;
				default:
					color = 0xff6f6f71;
				}
				text = MakeStr("%d", score);
			}
			DrawDigit(gdi, text, WPoint(x, y), color, (FrGDI::eTextAlign)1,
				false);
			x = (float)float2int(width * (1.0f / 9.0f) + x);
		}
	}
}

void FrScoreForm::DrawVersus(FrListItem* item)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pList[1]->GetItemWidth();
	void* data = item->pData;
	int i = m_bFront9 ? 0 : 9;
	int shown = 1 - float2int(m_aniTime * -20.0f);
	int end;
	if ((shown > 8 ? 9 : shown) + i < Doc()->m_golfGame.holes)
		end = (shown > 8 ? 9 : shown) + i;
	else
		end = Doc()->m_golfGame.holes;
	int x = float2int(width * (1.0f / 18.0f) + item->pos.x);
	float y = (float)float2int(
		0.5f * (float)m_pList[1]->GetItemHeight() + item->pos.y - 6.0f);
	for (; i < end; i++)
	{
		WPoint pos((float)x, y);
		unsigned char hole = Doc()->m_holeOrder[i];
		bool empty;
		if (SDOC983(unsigned char, 0x10594) == 1)
			empty = Doc()
						->m_pGolfDoc->GetTeam((unsigned char)(int)data)
						->stroke[hole - 1] == (unsigned char)0;
		else
		{
			unsigned char stroke;
			if (SDOC983(unsigned char, 0x10594) == 6)
			{
				unsigned char idx = Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
					0x10fd4 + ((unsigned int)data & 0xff) * 4));
				stroke = *(unsigned char*)(hole + 0x7f +
					(unsigned int)idx * 0x1b8 + SDOC983(int, 0x1042c));
			}
			else
				stroke = Doc()
							 ->m_pGolfDoc->GetPlayer((unsigned char)(int)data)
							 ->stroke[hole - 1];
			empty = stroke == 0;
		}
		if (empty)
		{
			if (i !=
				GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) - 1)
				return;
		}
		if (i ==
				GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
					1 &&
			(*(unsigned int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) & 0xfffddfff) !=
				0)
		{
			DrawDigit(gdi, "?", pos, 0xffffffff, (FrGDI::eTextAlign)1, false);
		}
		else
		{
			const Bitmap* pBmp;
			unsigned int result =
				*(unsigned char*)(*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
					0x1a + (unsigned int)hole * 0x38);
			switch (result)
			{
			case 0:
			case 1:
				pBmp = (data != (void*)result) ? m_pLose : m_pWin;
				break;
			case 2:
				pBmp = m_pDraw;
				break;
			case 3:
				pBmp = m_pQuestion;
				break;
			default:
				goto next;
			}
			if (pBmp)
			{
				gdi->DrawTexture(pBmp,
					WRect((float)float2int(pos.x - (float)pBmp->Width() * 0.5f),
						y, (float)pBmp->Width(), (float)pBmp->Height()),
					0xffffffff, 0);
			}
		}
next:
		x = float2int(width * (1.0f / 9.0f) + pos.x);
	}
}

void FrScoreForm::DrawGraph(FrListItem* item)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pList[1]->GetItemWidth();
	void* data = item->pData;
	float height = (float)m_pList[1]->GetItemHeight();
	WPoint from;
	from.y = 0.5f * height + item->pos.y;
	int first = m_bFront9 ? 0 : 9;
	float unit = (height - 5.0f) * (1.0f / 6.0f);
	int shown = 1 - float2int(m_aniTime * -20.0f);
	int end;
	if ((shown > 8 ? 9 : shown) + first < SDOC983(unsigned char, 0x10699))
		end = (shown > 8 ? 9 : shown) + first;
	else
		end = SDOC983(unsigned char, 0x10699);
	unsigned char player = (unsigned char)(int)data;
	float base = from.y;
	from.x = (float)float2int(item->pos.x);
	WPoint to(
		(float)float2int((float)(end - first) * width * (1.0f / 9.0f) + from.x),
		from.y);
	g_view->DrawLine2D(from, to, 0xff95ffb3, 0xff95ffb3, 0);
	int i = first;
	if (Doc()->m_pGolfDoc->GetPlayer(player)
				->stroke[Doc()->m_holeOrder[first] - 1] != (unsigned char)0 &&
		(Doc()->m_pGolfDoc->GetPlayer(player)->state != 0 ||
			first !=
				GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
					1 ||
			*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) == 0x2000))
	{
		int diff = (unsigned char)Doc()
					   ->m_pGolfDoc->GetPlayer(player)
					   ->stroke[Doc()->m_holeOrder[first] - 1] -
			*(unsigned char*)(*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) - 0x20 +
				(unsigned int)Doc()->m_holeOrder[first] * 0x38);
		if (diff < -3)
			diff = -3;
		else if (diff > 3)
			diff = 3;
		unsigned long color = GetScoreColor(diff);
		from.x = (float)float2int(width * (1.0f / 18.0f) + item->pos.x);
		from.y = (float)float2int((float)diff * unit + base);
		for (int j = first + 1; j < end; j++)
		{
			unsigned char hole = Doc()->m_holeOrder[j];
			if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
					(unsigned char)0 ||
				(Doc()->m_pGolfDoc->GetPlayer(player)->state == 0 &&
					j ==
						GetHoleIndex(
							*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
							1 &&
					*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) != 0x2000))
				break;
			diff = (unsigned char)Doc()
					   ->m_pGolfDoc->GetPlayer(player)
					   ->stroke[hole - 1] -
				*(unsigned char*)(*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
					0x20 + (unsigned int)hole * 0x38);
			if (diff < -3)
				diff = -3;
			else if (diff > 3)
				diff = 3;
			unsigned long nextColor = GetScoreColor(diff);
			to.x = (float)float2int(width * (1.0f / 9.0f) + from.x);
			to.y = (float)float2int((float)diff * unit + base);
			g_view->DrawLine2D(from, to, color, nextColor, 0);
			from.x = to.x;
			from.y = to.y;
			color = nextColor;
		}
	}
	if (m_pPoint)
	{
		WRect rect(0.0f, 0.0f, 4.0f, 4.0f);
		rect.x = (width * (1.0f / 18.0f) + item->pos.x) - 2.0f;
		for (i = first; i < end; i++)
		{
			rect.x = (float)float2int(rect.x);
			unsigned char hole = Doc()->m_holeOrder[i];
			if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
				(unsigned char)0)
				return;
			if (Doc()->m_pGolfDoc->GetPlayer(player)->state == 0 &&
				i ==
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1 &&
				*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) != 0x2000)
				return;
			int diff = (unsigned char)Doc()
						   ->m_pGolfDoc->GetPlayer(player)
						   ->stroke[hole - 1] -
				*(unsigned char*)(*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
					0x20 + (unsigned int)hole * 0x38);
			if (diff < -3)
				diff = -3;
			else if (diff > 3)
				diff = 3;
			rect.y = (float)float2int(((float)diff * unit + base) - 2.0f);
			gdi->DrawTexture(m_pPoint, rect, 0xff696969, 0);
			rect.x = width * (1.0f / 9.0f) + rect.x;
		}
	}
}

void FrScoreForm::DrawPutt(FrListItem* item)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pList[1]->GetItemWidth();
	unsigned char player = (unsigned char)(int)item->pData;
	int i = m_bFront9 ? 0 : 9;
	int shown = 1 - float2int(m_aniTime * -20.0f);
	int end;
	if ((shown > 8 ? 9 : shown) + i < Doc()->m_golfGame.holes)
		end = (shown > 8 ? 9 : shown) + i;
	else
		end = Doc()->m_golfGame.holes;
	float x = width * (1.0f / 18.0f) + item->pos.x;
	float y = (float)float2int(
		0.5f * (float)m_pList[1]->GetItemHeight() + item->pos.y - 6.0f);
	for (; i < end; i++)
	{
		x = (float)float2int(x);
		unsigned char hole = Doc()->m_holeOrder[i];
		if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
			(unsigned char)0)
		{
			if (i !=
				GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) - 1)
				return;
		}
		unsigned char putt = Doc()->m_pGolfDoc->GetPlayer(player)->putt[hole - 1];
		char* text;
		unsigned long color;
		if (i ==
				GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
					1 &&
			Doc()->m_pGolfDoc->GetPlayer(player)->state != 1 &&
			((*(unsigned int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) & 0x22000) ==
					0 ||
				*((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x15c)) != 0))
		{
			color = 0xffffffff;
			text = "-";
		}
		else if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
			(unsigned char)0)
		{
			color = 0xffffffff;
			text = "-";
		}
		else
		{
			color = 0xff40c967;
			text = MakeStr("%d", putt);
		}
		DrawDigit(gdi, text, WPoint(x, y), color, (FrGDI::eTextAlign)1, false);
		x = width * (1.0f / 9.0f) + x;
	}
}

void FrScoreForm::DrawPang(FrListItem* item)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pList[1]->GetItemWidth();
	void* data = item->pData;
	int i = m_bFront9 ? 0 : 9;
	int shown = 1 - float2int(m_aniTime * -20.0f);
	int end = SDOC983(unsigned char, 0x10699);
	if ((shown > 8 ? 9 : shown) + i < SDOC983(unsigned char, 0x10699))
		end = (shown > 8 ? 9 : shown) + i;
	float x = (float)float2int(width * (1.0f / 18.0f) + item->pos.x);
	float y = (float)float2int(
		0.5f * (float)m_pList[1]->GetItemHeight() + item->pos.y - 6.0f);
	unsigned char player = (unsigned char)(int)data;
	if (SDOC983(unsigned char, 0x10594) == 1)
	{
		for (; i < end; i++)
		{
			unsigned char hole = Doc()->m_holeOrder[i];
			if (Doc()->m_pGolfDoc->GetTeam(player)->stroke[hole - 1] ==
				(unsigned char)0)
			{
				if (i !=
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1)
					return;
			}
			__int64 pang;
			if (i ==
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1 &&
				data ==
					(void*)(unsigned int)Doc()
						->m_pGolfDoc
						->GetPlayer(
							*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
						->team)
				pang = m_pang;
			else
				pang = (int)Doc()->m_pGolfDoc->GetTeam(player)->pang[hole - 1];
			char* text;
			unsigned long color;
			bool narrow;
			if (Doc()->m_pGolfDoc->GetTeam(player)->stroke[hole - 1] ==
				(unsigned char)0)
			{
				narrow = false;
				color = 0xffffffff;
				text = "-";
			}
			else
			{
				narrow = true;
				color = 0xffe98919;
				text = MakeStr("%I64d", pang);
			}
			DrawDigit(gdi, text, WPoint(x, y), color, (FrGDI::eTextAlign)1,
				narrow);
			x = (float)float2int(width * (1.0f / 9.0f) + x);
		}
	}
	else if (SDOC983(unsigned char, 0x10594) == 6)
	{
		while (i < end)
		{
			unsigned char idx = Doc()->GetIndex(
				SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4, 0x10fd4 + ((unsigned int)data & 0xff) * 0x4));
			if (*(char*)(Doc()->m_holeOrder[i] + 0x7f +
					(unsigned int)idx * 0x1b8 + SDOC983(int, 0x1042c)) == 0 &&
				i !=
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1)
				break;
			__int64 pang;
			if (i ==
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1 &&
				(*(unsigned int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) &
					0xfffddfff) != 0)
				pang = m_pang;
			else
				pang = *(
					__int64*)((unsigned int)Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
								  0x10fd4 + ((unsigned int)data & 0xff) * 4)) *
						0x1b8 +
					SDOC983(int, 0x1042c) + 0x90 +
					(unsigned int)Doc()->m_holeOrder[i] * 8);
			char* text;
			unsigned long color;
			bool narrow;
			if (*(char*)(Doc()->m_holeOrder[i] + 0x7f +
					(unsigned int)Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
						0x10fd4 + ((unsigned int)data & 0xff) * 4)) *
						0x1b8 +
					SDOC983(int, 0x1042c)) == 0 ||
				(*(int*)((unsigned int)Doc()->GetIndex(SDOCM(unsigned long, m_teamPlayerGuid, 0x10fd4,
							 0x10fd4 + ((unsigned int)data & 0xff) * 4)) *
						 0x1b8 +
					 SDOC983(int, 0x1042c) + 0x4) != SDOC983(int, 0x81a) &&
					(int)GetHoleIndex(
						*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
							1 <
						i))
			{
				narrow = false;
				color = 0xffffffff;
				text = "?";
			}
			else
			{
				narrow = true;
				color = 0xffe98919;
				text = MakeStr("%d", pang);
			}
			DrawDigit(gdi, text, WPoint(x, y), color, (FrGDI::eTextAlign)1,
				narrow);
			i++;
			x = (float)float2int(width * (1.0f / 9.0f) + x);
		}
	}
	else
	{
		for (; i < end &&
			(Doc()->m_pGolfDoc->GetPlayer(player)
						->stroke[Doc()->m_holeOrder[i] - 1] != (unsigned char)0 ||
				i ==
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1);
			i++)
		{
			unsigned char hole = Doc()->m_holeOrder[i];
			__int64 pang;
			if (i ==
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) -
						1 &&
				data ==
					(void*)(unsigned int)*(
						(unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
				pang = m_pang;
			else
				pang = (int)Doc()->m_pGolfDoc->GetPlayer(player)->pang[hole - 1];
			char* text;
			unsigned long color;
			bool narrow;
			if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
				(unsigned char)0)
			{
				narrow = false;
				color = 0xffffffff;
				text = "-";
			}
			else
			{
				narrow = true;
				color = 0xffe98919;
				text = MakeStr("%d", pang);
			}
			DrawDigit(gdi, text, WPoint(x, y), color, (FrGDI::eTextAlign)1,
				narrow);
			x = (float)float2int(width * (1.0f / 9.0f) + x);
		}
	}
}

void FrScoreForm::DrawSkins(FrListItem* item)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	float width = (float)m_pList[1]->GetItemWidth();
	unsigned char player = (unsigned char)(int)item->pData;
	int i = m_bFront9 ? 0 : 9;
	int shown = 1 - float2int(m_aniTime * -20.0f);
	int end;
	if ((shown > 8 ? 9 : shown) + i < Doc()->m_golfGame.holes)
		end = (shown > 8 ? 9 : shown) + i;
	else
		end = Doc()->m_golfGame.holes;
	float y = (float)float2int(
		0.5f * (float)m_pList[1]->GetItemHeight() + item->pos.y - 6.0f);
	if (SDOC983(unsigned char, 0x10594) != 7)
		return;
	int x = float2int(width * (1.0f / 18.0f) + item->pos.x);
	for (; i < end; i++)
	{
		WPoint pos((float)x, y);
		unsigned char hole = Doc()->m_holeOrder[i];
		if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
			(unsigned char)0)
		{
			if (i !=
				GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) - 1)
				return;
		}
		bool dash = false;
		if (i == GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) - 1)
		{
			if (Doc()->m_pGolfDoc->GetPlayer(player)->state == 3)
			{
				if (Doc()->m_pGolfDoc->GetPlayer(player)->stroke[hole - 1] ==
					(unsigned char)0)
					dash = true;
			}
			else if (*((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x194)) == 0 && m_state != 3)
				dash = true;
		}
		if (dash)
			DrawDigit(gdi, "-", pos, 0xffffffff, (FrGDI::eTextAlign)1, false);
		else
		{
			int skins = Doc()->m_pGolfDoc->GetPlayer(player)->skinsPang[hole - 1];
			unsigned long color;
			if (Doc()->m_pGolfDoc->GetPlayer(player)->state == 3)
				color = 0xff282828;
			else if (*((char*)*(char**)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) -
						 0x19 + (unsigned int)hole * 0x38) == 0 ||
				skins <= 0)
				color = 0xff000000;
			else
				color = 0xffff0000;
			gdi->SetTextStyle(1);
			gdi->SetTextColor(color, 0xffffffff);
			if (skins == -1)
			{
				gdi->DrawTexture(m_pArrow,
					WRect(pos.x - (float)m_pArrow->Width() * 0.5f, pos.y,
						(float)m_pArrow->Width(), (float)m_pArrow->Height()),
					0xffffffff, 0);
			}
			else
				gdi->PrintText(pos, 1, MakeStr("%d", skins), NULL);
		}
		x = float2int(width * (1.0f / 9.0f) + pos.x);
	}
}

unsigned long FrScoreForm::GetScoreColor(int score)
{
	switch (score)
	{
	case -4:
	case -3:
	case -2:
		return 0xffff0000;
	case -1:
		return 0xffffbb4f;
	case 0:
		return 0xffffffff;
	case 1:
		return 0xff82c3ff;
	default:
		return 0xff0036ff;
	}
}

void FrScoreForm::OnTotalInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (!pArea)
		return;
	// 983 (00783970): header04 so nos tipos 1 e 6 (o 3 cai em header03)
	if (Doc()->m_golfGame.gameType == 1 || Doc()->m_golfGame.gameType == 6)
		pArea->SetBgImg("header04");
	else if (Doc()->m_golfGame.gameType == 7)
	{
		pArea->SetBgImg("header03_skins");
		const WRect& rect = pArea->GetRect();
		pArea->SetRect(WRect(rect.x - 52.0f, rect.y + 0.0f, rect.w + 0.0f, rect.h + 0.0f));
	}
	else
		pArea->SetBgImg("header03");
}

void FrScoreForm::OnPPLInit(int param)
{
	m_pPPL = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (!m_pPPL)
		return;
	// 983 (00783a60): so os tipos 1 e 6 deslocam (o 3 do 645 saiu)
	// o 983 soma o deslocamento (double) em x e 0 nos outros tres campos; ramos sem tail-merge (WPoint + WSize)
	const WRect& rect = m_pPPL->GetRect();
	if (Doc()->m_golfGame.gameType == 1 || Doc()->m_golfGame.gameType == 6)
	{
		m_pPPL->SetRect(WRect(WPoint(rect.x - 65.0, rect.y + 0.0f), WSize(rect.w + 0.0f, rect.h + 0.0f)));
	}
	else if (Doc()->m_golfGame.gameType == 7)
	{
		m_pPPL->SetRect(WRect(WPoint(rect.x + 43.0, rect.y + 0.0f), WSize(rect.w + 0.0f, rect.h + 0.0f)));
	}
	m_pPPL->SetVisible(false);
	m_bPPL = false;
}

void FrScoreForm::SetPPL(const char* name)
{
	if (!m_pPPL)
		return;
	const Bitmap* pOld = m_pPPL->GetBgImg();
	if (m_pPPL->SetBgImg(name))
	{
		m_bPPL = true;
		m_pPPL->SetVisible(true);
	}
	else
	{
		m_bPPL = false;
		m_pPPL->SetBgImg(pOld);
		m_pPPL->SetVisible(false);
	}
}

void FrScoreForm::OnBoard1Init(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea && Doc()->m_golfGame.gameType == 7)
	{
		pArea->SetBgImg("header01_skins");
		const WRect& rect = pArea->GetRect();
		pArea->SetRect(WRect(rect.x - 1.0f, rect.y, rect.w, rect.h));
	}
	else
		pArea->SetBgImg("header01");
}

void FrScoreForm::OnBoard2Init(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea && Doc()->m_golfGame.gameType == 7)
	{
		pArea->SetBgImg("header02_skins");
		const WRect& rect = pArea->GetRect();
		pArea->SetRect(WRect(rect.x - 32.0f, rect.y, rect.w, rect.h));
	}
	else
		pArea->SetBgImg("header02");
}

void FrScoreForm::OnDrawLinesInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (Doc()->m_golfGame.gameType != 7)
		pArea->SetVisible(false);
}

void FrScoreForm::OnDrawLinesOwnerDraw(int param)
{
	WPoint from;
	WPoint to;
	from.x = to.x = m_rect.x + 129.0f;
	from.y = m_rect.y + 126.0f;
	to.y = (float)m_playerCount * 58.0f + from.y;
	if (Doc()->m_golfGame.gameType == 7)
	{
		to.x = from.x = to.x - 22.0f;
		from.y += 22.0f;
		to.y += 22.0f;
	}
	int i;
	for (i = 0; i < 10; i++)
	{
		g_view->DrawLine2D(from, to, 0xff9c9c9c, 0xff9c9c9c, 0);
		from.x += 38.0f;
		to.x += 38.0f;
		if (Doc()->m_golfGame.gameType == 7)
		{
			from.x += 14.0f;
			to.x += 14.0f;
		}
	}
	from.x += 27.0f;
	to.x += 27.0f;
	if (Doc()->m_golfGame.gameType == 7)
	{
		from.x -= 13.0f;
		to.x -= 13.0f;
	}
	from.y = to.y = m_rect.y + 184.0f;
	if (Doc()->m_golfGame.gameType == 7)
		to.y = from.y = to.y + 22.0f;
	for (i = 0; i < m_playerCount - 1; i++)
	{
		from.x = m_rect.x + 3.0f;
		to.x = m_rect.x + 28.0f;
		g_view->DrawLine2D(from, to, 0xff9c9c9c, 0xff9c9c9c, 0);
		from.x = m_rect.x + m_rect.w - 36.0f;
		to.x = m_rect.x + m_rect.w - 11.0f;
		g_view->DrawLine2D(from, to, 0xff9c9c9c, 0xff9c9c9c, 0);
		from.x = m_rect.x + 28.0f;
		to.x = m_rect.x + m_rect.w - 36.0f;
		g_view->DrawLine2D(from, to, 0xff9c9c9c, 0xff9c9c9c, 0);
		from.y += 58.0f;
		to.y += 58.0f;
	}
}

void FrScoreForm::OnCancelBtnInit(int param)
{
	m_pCancelBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pCancelBtn->SetVisible(false);
}

void FrScoreForm::OnCancelBtnUp()
{
	Close(FrOK, true);
}

void FrScoreForm::OnCloseBtnInit(int param)
{
	// 983 (00784010): refaz o cast sempre, mostra o botao e, no estado 1 do jogo, move 65 px p/ a esquerda
	if (m_pCloseBtn)
		m_pCloseBtn = NULL;
	m_pCloseBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pCloseBtn)
	{
		m_pCloseBtn->SetVisible(true);
		if (*((unsigned char*)CSharedDoc::Instance() + DOCOFS983(0x10594)) == 1)
		{
			const WRect& r = m_pCloseBtn->GetRect();
			m_pCloseBtn->SetRect(WRect(r.x - 65.0, r.y, r.w, r.h));
		}
	}
}

void FrScoreForm::OnCloseBtnUp()
{
	// 983 (00787850): SetState(STATE_HIDE), nao so SetVisible(false) do 645 -- com m_state preso em
	// STATE_RESULT o proximo Tab (SetState(2)) saia cedo e o placar nao reabria
	SetState(STATE_HIDE);
}

// 983 (00791500): registro 0x1b8 da lista da sala pelo indice (o indice sai antes de ler o vetor)
static inline int RbRoomRec983(unsigned char i)
{
	return (unsigned int)i * 0x1b8 + SDOC983(int, 0x1042c);
}

void FrScoreForm::SetDesc()
{
	// 983 (00791500): every text goes through K2L_Compatibility (Korean -> JP)
	char desc[256];
	// 983 (00791500): tipos de jogo 0xc/0xe/0x13 nao montam descricao
	if (SDOC983(unsigned char, 0x109b8) == 0xc || SDOC983(unsigned char, 0x109b8) == 0xe ||
		SDOC983(unsigned char, 0x109b8) == 0x13)
		return;
	unsigned long leader = 0xffffffff;
	// 983: m_holeOrder guarda o buraco a partir de 1 -> pang[ordem - 1]
	switch (SDOC983(unsigned char, 0x10594))
	{
	case 0:
		for (unsigned char p = 0; p < Doc()->m_pGolfDoc->m_playerNum; p++)
		{
			if (Doc()->m_pGolfDoc->GetPlayer(p)->state != 3 &&
				Doc()->m_pGolfDoc->GetPlayer(p)->rank == 1)
			{
				if (leader != 0xffffffff)
				{
					sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb5\xe9\xc0\xc7 \xbd\xba\xc4\xda\xbe\xee\xb8\xa6 \xba\xb8\xbf\xa9\xc1\xd6\xb4\xc2 \xc4\xab\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9."));
					break;
				}
				leader = Doc()->m_pGolfDoc->GetPlayer(p)->oid;
				__int64 total = 0;
				unsigned char holes =
					GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c)));
				for (unsigned char k = 0; k < holes; k++)
				{
					if (k == holes - 1 &&
						p == *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
						total += m_pang;
					else
						total += (int)Doc()
									 ->m_pGolfDoc->GetPlayer(p)
									 ->pang[Doc()->m_holeOrder[k] - 1];
				}
				sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 %s\xb4\xd4\xc0\xcc \xbd\xba\xc4\xda\xbe\xee %d, \xc6\xce %I64d"), (const char*)&SDOCM(char, m_userInfo, 0x38b0, p * 0x31c6 + 0x38c8),
					Doc()->m_pGolfDoc->GetPlayer(p)->score, total);
				ConditionalStrCat(desc, K2L_Compatibility("\xb7\xce \xbc\xb1\xb5\xce\xc0\xd4\xb4\xcf\xb4\xd9."));
			}
		}
		if (leader != 0xffffffff)
			break;
		sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb5\xe9\xc0\xc7 \xbd\xba\xc4\xda\xbe\xee\xb8\xa6 \xba\xb8\xbf\xa9\xc1\xd6\xb4\xc2 \xc4\xab\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9."));
		break;
	case 4:
	case 5:
	case 9:
	case 10:
#define RB_ROOMREC(i) (SDOC983(int, 0x1042c) + (unsigned int)(i) * 0x1b8)
#define RB_LEADERREC RbRoomRec983(Doc()->GetIndex(leader))
		for (unsigned char n = 0; n < (unsigned int)((SDOC983(int, 0x10430) - SDOC983(int, 0x1042c)) / 0x1b8); n++)
		{
			int rec = RB_ROOMREC(n);
			if (*(char*)(rec + 0x152) == 1)
			{
				if (leader != 0xffffffff)
				{
					sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb5\xe9\xc0\xc7 \xbd\xba\xc4\xda\xbe\xee\xb8\xa6 \xba\xb8\xbf\xa9\xc1\xd6\xb4\xc2 \xc4\xab\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9."));
					break;
				}
				leader = *(unsigned long*)(rec + 0x4);
				if (SDOC983(unsigned char, 0x10594) == 10)
					sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 %s\xb4\xd4\xc0\xcc \xb3\xb2\xc0\xba\xb0\xc5\xb8\xae %d.%d y, \xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3 %d.%03d"),
						(char*)(RB_LEADERREC + 0x8),
						*(unsigned int*)(RB_LEADERREC + 0x190) / 10,
						*(unsigned int*)(RB_LEADERREC + 0x190) % 10,
						*(int*)(RB_LEADERREC + 0x194) / 1000,
						*(int*)(RB_LEADERREC + 0x194) % 1000);
				else if ((unsigned char)(SDOC983(unsigned char, 0x109b8) - 0x14) < 2)
				{
					// 983: Grand Prix (realGameType 0x14/0x15) com +0x1b4: usa o proprio registro
					if (*(char*)(rec + 0x1b4) != 0)
						sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 %s\xb4\xd4\xc0\xcc \xbd\xba\xc4\xda\xbe\xee %d, \xc6\xce %I64d"), (char*)(rec + 0x8),
							*(int*)(rec + 0x14c), *(__int64*)(rec + 0x128));
					else
						sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 %s\xb4\xd4\xc0\xcc \xbd\xba\xc4\xda\xbe\xee %d, \xc6\xce %I64d"), (char*)(RB_ROOMREC(n) + 0x8),
							*(int*)(RB_LEADERREC + 0x14c),
							*(__int64*)(RB_LEADERREC + 0x128));
				}
				else
					sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 %s\xb4\xd4\xc0\xcc \xbd\xba\xc4\xda\xbe\xee %d, \xc6\xce %I64d"),
						(char*)(RB_LEADERREC + 0x8),
						*(int*)(RB_LEADERREC + 0x14c),
						*(__int64*)(RB_LEADERREC + 0x128));
				ConditionalStrCat(desc, K2L_Compatibility("\xb7\xce \xbc\xb1\xb5\xce\xc0\xd4\xb4\xcf\xb4\xd9."));
			}
		}
#undef RB_ROOMREC
#undef RB_LEADERREC
		break;
	case 1:
		if (TEAM(0)->score == TEAM(1)->score)
		{
			__int64 total[2];
			total[0] = 0;
			total[1] = 0;
			unsigned char holes =
				GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c)));
			for (unsigned char t = 0; t < 2; t++)
			{
				for (unsigned char k = 0; k < holes; k++)
				{
					if (k == holes - 1 &&
						t == Doc()
								 ->m_pGolfDoc
								 ->GetPlayer(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x189)))
								 ->team)
						total[t] += m_pang;
					else
						total[t] += (int)Doc()
										->m_pGolfDoc->GetTeam(t)
										->pang[Doc()->m_holeOrder[k] - 1];
				}
			}
			// 983 (00791500): empate de pang tem texto proprio (0xd24aa0), nao o generico
			if (total[0] == total[1])
			{
				sprintf(desc, K2L_Compatibility("\xb5\xbf\xc1\xa1\xc0\xcf \xb0\xe6\xbf\xec\xbf\xa1\xb4\xc2 \xc6\xce\xc0\xb8\xb7\xce \xbc\xf8\xc0\xa7\xb8\xa6 \xb0\xe1\xc1\xa4\xc7\xd5\xb4\xcf\xb4\xd9."));
				break;
			}
			if (total[0] > total[1])
				sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbd\xba\xc4\xda\xbe\xee\xb4\xc2 \xb5\xbf\xc1\xa1\xc0\xcc\xc1\xf6\xb8\xb8, RED \xc6\xc0\xc0\xcc \xc6\xce %d"), total[0]);
			else
				sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbd\xba\xc4\xda\xbe\xee\xb4\xc2 \xb5\xbf\xc1\xa1\xc0\xcc\xc1\xf6\xb8\xb8, BLUE \xc6\xc0\xc0\xcc \xc6\xce %d"), total[1]);
			ConditionalStrCat(desc, K2L_Compatibility("\xb7\xce \xc0\xcc\xb1\xe2\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
		}
		else
		{
			if (TEAM(0)->score > TEAM(1)->score)
				sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 RED \xc6\xc0\xc0\xcc %d vs %d"), TEAM(0)->score, TEAM(1)->score);
			else
				sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 BLUE \xc6\xc0\xc0\xcc %d vs %d"), TEAM(1)->score, TEAM(0)->score);
			ConditionalStrCat(desc, K2L_Compatibility("\xb7\xce \xc0\xcc\xb1\xe2\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
		}
		break;
	default:
		sprintf(desc, K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb5\xe9\xc0\xc7 \xbd\xba\xc4\xda\xbe\xee\xb8\xa6 \xba\xb8\xbf\xa9\xc1\xd6\xb4\xc2 \xc4\xab\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9."));
		break;
	}
	FrForm::SetDesc(desc);
}

void FrScoreForm::ConditionalStrCat(char* dest, const char* src)
{
	strcat(dest, src); // 983 (0077c4b0): so concatena (a virgula condicional do 645 saiu)
}

IMPLEMENT_OBJECT(FrMatchItems, FrForm)

BEGIN_FRESH_MSGMAP(FrMatchItems, FrForm)
ON_FRESH_VI("view", FRCMD_INIT, FrMatchItems::OnViewInit)
ON_FRESH_VI("view", FRCMD_OWNERDRAW, FrMatchItems::OnViewOwnerDraw)
END_FRESH_MSGMAP()

FrMatchItems::FrMatchItems()
{
	m_pNumberUI = NULL;
}

FrMatchItems::~FrMatchItems()
{
	if (m_pNumberUI)
	{
		delete m_pNumberUI;
		m_pNumberUI = NULL;
	}
}

bool FrMatchItems::OnInit()
{
	m_x = (g_view->GetWidth() - 440.0f) * 0.5f;
	m_y = g_view->GetHeight() - 100.0f;
	m_pNumberUI = new CSimpleUI("[emoti_object.png");
	m_pNumberUI->Add("0", WRect(0.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("1", WRect(64.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("2", WRect(128.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("3", WRect(192.0f, 128.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("4", WRect(0.0f, 192.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("5", WRect(64.0f, 192.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("6", WRect(128.0f, 192.0f, 64.0f, 64.0f), false);
	m_pNumberUI->Add("7", WRect(192.0f, 192.0f, 64.0f, 64.0f), false);
	m_count = 0;
	m_bAnimate = true;
	m_bPlaySound = true;
	m_pad983_154 = 0;
	m_aniTime = -3.0f;
	return true;
}

void FrMatchItems::OnViewInit(int param)
{
	m_pView = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrMatchItems::OnViewOwnerDraw(int param)
{
	if (0)
		((CRankExData*)0)->CRankExData::SetLeader();
	unsigned char index = MyIndex();
	WRect pos;
	pos.x = m_x + 8.0f;
	pos.y = m_y + 28.0f;
	unsigned long* slot = Doc()->m_userInfo[index].userEquip.tidItemSlot;
	unsigned long* begin = slot;
	pos.w = pos.h = 54.0f;
	int failIndex = 7;
	WOverlay* pFail = NULL;
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(0x1800000d);
	if (pItem)
		pFail = g_resrcmng->GetOverlay(
			MakeStr("%s.tga?items.bres", pItem->c.Icon), 0);
	int count = 10;
	if (IsLocalContent((localContentType_t)3))
		count = 8;
	for (int i = 0; i < count; i++, slot++)
	{
		unsigned long tid = *slot;
		if (tid)
		{
			std::map<unsigned long, WOverlay*>::iterator it =
				m_icon.find(*slot);
			if (it == m_icon.end())
			{
				pItem = ItemManager()->FindItem(tid);
				if (pItem)
				{
					WOverlay* pOverlay = g_resrcmng->GetOverlay(
						MakeStr("%s.tga?items.bres", pItem->c.Icon), 0);
					if (pOverlay)
						m_icon.insert(
							std::map<unsigned long, WOverlay*>::value_type(
								*slot, pOverlay));
				}
			}
			if (*slot == 0x1800000d)
			{
				failIndex = i - 1;
				break;
			}
		}
	}
	for (int i = 0; i < count; i++)
	{
		if (begin[i])
		{
			std::map<unsigned long, WOverlay*>::iterator it =
				m_icon.find(begin[i]);
			if (it != m_icon.end())
			{
				WOverlay* pOverlay = (*it).second;
				if (i == failIndex && m_aniTime < 0.0f && m_bAnimate)
				{
					if (pFail)
						pFail->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
							pos, 0, 0xffffffff, 0.0f, 0);
				}
				else
					pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f), pos,
						0, 0xffffffff, 0.0f, 0);
			}
			pos.x += 45.0f;
		}
	}
	if (m_bAnimate)
	{
		m_aniTime += 0.1f;
		m_count = (int)(m_aniTime * 5.0f);
		if (m_count == 8)
		{
			m_aniTime = -3.0f;
			m_bAnimate = false;
			return;
		}
		if (m_count >= 0)
		{
			if (m_bPlaySound)
			{
				g_audio->PlaySfx("play_fail_item");
				m_bPlaySound = false;
			}
			if (m_pNumberUI)
				m_pNumberUI->Render(MakeStr("%d", m_count),
					(float)failIndex * 45.0f + m_x, m_y + 8.0f, 1.0f, 0);
		}
	}
}

// FrWnd::SetFixed is out of line in our Fresh headers; the original sets the
// FWS_FIXED style inline
struct FrFixedWnd : public FrWnd
{
	__declspec(dllimport) __forceinline void Fix()
	{
		m_dwStyle.Enable(FWS_FIXED);
	}
};

// the original keeps these two as out-of-line COMDATs (their callers are naked asm)

IMPLEMENT_ACTOR(CScore, IActor)

BEGIN_FRESH_MSGMAP(CScore, FrCmdTarget)
ON_FRESH_VV(".score.close", FRCMD_LBUTTONUP, CScore::OnCloseBtnUp)
END_FRESH_MSGMAP()

CScore::CScore()
	: m_state(0),
	  m_stateTimer(0),
	  m_scoreView(0),
	  m_bRank3Shown(1),
	  m_bRank3ByKey(1),
	  m_pad983_30(0),
	  m_pScore(NULL),
	  m_pRank(NULL),
	  m_pRank2(NULL),
	  m_pRank3(NULL),
	  m_pRankEx(NULL),
	  m_pGrandPrix(NULL),
	  m_pAward(NULL),
	  m_pItems(NULL),
	  m_pGuildScore(NULL),
	  m_pTreasureGauge(NULL),
	  m_pNextHole(NULL),
	  m_pTreasureScore(0),
	  m_pTreasureGift(0),
	  m_whiteFade(0),
	  m_pAquire(NULL)
{
	m_dispPriority = DISP_PRIORITY_5;
	// B1503: 983 00799f10 reserva/carrega 15 (score_notice_0..14 do JP); com 10, Nuri(R) (indice 11) lia fora do vetor no OnLoad
	m_images.reserve(15);
	LoadNoticeImageFile(MakeStr("%s/score_ppl.ini", SCORE_PPL_DIR), m_images,
		15, NULL);
	ImageSortByCharacterIndex(m_images);
}

void CScore::ImageSortByCharacterIndex(std::vector<sNoticeImage>& images)
{
	if (images.size() >= 15) // 983 00799c70: 15
	{
		std::vector<sNoticeImage> sorted;
		sorted.reserve(images.size());
		for (int i = 0; i < 15; ++i)
			sorted.push_back(sNoticeImage());
		for (unsigned int i = 0; i < images.size(); ++i)
		{
			const char* prefix = "score_notice_";
			int index = 0;
			sscanf(images[i].name.c_str() + strlen(prefix), "%d", &index);
			if (index < 15)
				sorted[index].name = images[i].name; // 983: std::string (00799c70)
		}
		images.assign(sorted.begin(), sorted.end());
	}
}

void CScore::OnDestroy()
{
	if (m_pRankEx)
	{
		m_pRankEx->Close(FrOK, true);
		m_pRankEx = NULL;
	}
	if (m_pGuildScore)
	{
		delete m_pGuildScore; // 983 794e90: vtable+8 (dtor deletor) em +0x54
		m_pGuildScore = NULL;
	}
	if (m_pAquire)
	{
		delete m_pAquire;
		m_pAquire = NULL;
	}
	if (m_pGrandPrix)
	{
		m_pGrandPrix->Close(FrOK, true);
		m_pGrandPrix = NULL;
	}
}

// 983: sala do Grand Prix (realGameType 0x14/0x15); OnLoad e HandleMsg.
// 983: a mesma conta da IsGrandPrixGameType983 (0046e6d0), aqui em linha (o placar nao chama a de fora)
static __forceinline bool IsGrandPrixGameTypeInl983(int realGameType)
{
	return realGameType >= 0x14 && realGameType <= 0x15;
}
static __forceinline bool IsGrandPrixRoom983()
{
	switch (Doc()->m_roomInfo.realGameType)
	{
	case 0x14:
	case 0x15:
		return true;
	}
	return false;
}
// 983: 007979e0. Diferencas do 645: o caso 11 entra com 4/5/6/9/10; sala 0x13 sem placar; casos 10 e 11 sem "rankex"; na sala
// do Grand Prix (realGameType 0x14/0x15) o "rankex" da lugar ao "grandprix_scoreboard" (m_pGrandPrix, criado uma vez so;
// em 640x480 fica em y = -10); "score_guild" com o botao fechar visivel e o cancelar escondido; caso 11 sem "score"; sem a
// FrMatchItems do caso 3 (ela nao existe no 983); no fim, a figura do personagem (score_ppl) que o 645 punha no HandleMsg.
void CScore::OnLoad()
{
	WPoint center;

	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 11:
		if (Doc()->m_golfGame.gameType == 6)
			break;
		if (Doc()->m_roomInfo.realGameType == 0x13)
			break;
		if ((Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe)
		{
			m_pRank2 = CreateForm<FrRankForm2>(g_pFresh->GetManager(), this,
				"rank2", NULL);
			((FrFixedWnd*)m_pRank2)->Fix();
			m_pRank2->EnableDrag(false);
			m_pRank2->Open(NULL, 0xc);
			m_pRank2->SetVisible(false);
		}
		else
		{
			m_pRank = CreateForm<FrRankForm>(g_pFresh->GetManager(), this,
				"rank", NULL);
			m_rankPos.x = g_view->GetWidth() - m_pRank->GetRect().w;
			m_rankPos.y = 40.0f;
			((FrFixedWnd*)m_pRank)->Fix();
			m_pRank->EnableDrag(false);
			m_pRank->Open(NULL, m_rankPos, 0xc);
			m_pRank->SetVisible(false);
			m_pRank->Resize();
		}
		if (Doc()->m_golfGame.gameType >= 10 && Doc()->m_golfGame.gameType <= 11)
			break;
		if (IsGrandPrixGameTypeInl983(Doc()->m_roomInfo.realGameType))
		{
				if (m_pGrandPrix)
					break;
				m_pGrandPrix = CreateForm<FrGrandPrixRankForm>(g_pFresh->GetManager(),
					this, "grandprix_scoreboard", NULL);
				m_pGrandPrix->EnableDrag(false);
				WRect rc = m_pGrandPrix->GetRect();
				WPoint pos;
				pos.x = (float)(int)(float)((g_view->GetWidth() - rc.w) * 0.5);
				pos.y = (float)(int)(float)((g_view->GetHeight() - rc.h) * 0.5);
				if (g_view->GetWidth() == 640.0 && g_view->GetHeight() == 480.0)
					pos.y = -10.0f;
				((FrFixedWnd*)m_pGrandPrix)->Fix();
				m_pGrandPrix->SetVisible(false);
				m_pGrandPrix->Open(
					(FRESH_PFN_RESULT)&CScore::OnGrandPrixRankFormResult, pos, 4);
		}
		else
		{
			m_pRankEx = CreateForm<FrRankFormEx>(g_pFresh->GetManager(), this,
				"rankex", NULL);
			m_pRankEx->EnableDrag(false);
			center.x = (float)(int)(float)((g_view->GetWidth() -
										  m_pRankEx->GetRect().w) *
				0.5);
			center.y = (float)(int)(float)((g_view->GetHeight() -
										  m_pRankEx->GetRect().h) *
				0.5);
			((FrFixedWnd*)m_pRankEx)->Fix();
			m_pRankEx->Open((FRESH_PFN_RESULT)&CScore::OnRankFormExResult,
				center, 4);
			m_pRankEx->SetVisible(false);
			m_pRankEx->ResetStage();
		}
	}
	if (Doc()->m_golfGame.gameType == 6)
	{
		m_pRank3 = CreateForm<FrRankForm3>(g_pFresh->GetManager(), this,
			"rank3", NULL);
		m_rankPos.x = g_view->GetWidth() - m_pRank3->GetRect().w + 5.0f;
		m_rankPos.y = 40.0f;
		((FrFixedWnd*)m_pRank3)->Fix();
		m_pRank3->EnableDrag(false);
		m_pRank3->Open(NULL, m_rankPos, 0xc);
		m_pRank3->SetVisible(false);
		m_pGuildScore = CreateForm<FrScoreDlg>(g_pFresh->GetManager(), this,
			"score_guild", NULL);
		WPoint pos;
		pos.x = (g_view->GetWidth() - m_pGuildScore->GetRect().w) * 0.5 + 0.5;
		pos.y = m_pGuildScore->GetRect().y;
		m_pGuildScore->EnableDrag(false);
		((FrFixedWnd*)m_pGuildScore)->Fix();
		m_pGuildScore->Open((FRESH_PFN_RESULT)&CScore::OnGuildScoreFormResult,
			pos, 0xc);
		m_pGuildScore->SetVisible(false);
		m_pGuildScore->SetVisibleCloseBtn(true);
		m_pGuildScore->SetVisibleCancelBtn(false);
		return;
	}
	if (Doc()->m_golfGame.gameType != 7)
	{
		if (Doc()->m_golfGame.gameType != 11)
		{
			m_pScore = CreateForm<FrScoreForm>(g_pFresh->GetManager(), this,
				"score", NULL);
			m_pScore->EnableDrag(false);
			((FrFixedWnd*)m_pScore)->Fix();
			m_pScore->Open(NULL, 0xc);
			m_pScore->Resize();
			m_pScore->SetState(FrScoreForm::STATE_HIDE);
			m_pScore->SetTypeBtn();
			m_pScore->SetVisible(false);
			m_pScore->SetCaptionOffset(true);
			m_pScore->DescHidePrivacy();
			m_pTreasureGauge = CreateForm<FrTreasureGauge>(g_pFresh->GetManager(),
				this, "tPointdlg", NULL);
			m_pTreasureGauge->EnableDrag(false);
			((FrFixedWnd*)m_pTreasureGauge)->Fix();
			m_pTreasureGauge->Open(NULL, 0xc);
			m_pTreasureGauge->SetState((FrTreasureGauge::eState)1);
			m_pNextHole = CreateForm<FrNextHole>(g_pFresh->GetManager(), this,
				"nexthole", NULL);
			m_pNextHole->EnableDrag(false);
			((FrFixedWnd*)m_pNextHole)->Fix();
			m_pNextHole->Open(NULL, 0xc);
			m_pNextHole->SetState((FrNextHole::eState)1);
		}
	}
	else
	{
		m_pScore = CreateForm<FrScoreForm>(g_pFresh->GetManager(), this,
			"score_skins", NULL);
		m_pScore->EnableDrag(false);
		((FrFixedWnd*)m_pScore)->Fix();
		m_pScore->Open(NULL, 0xc);
		m_pScore->Resize();
		m_pScore->SetState(FrScoreForm::STATE_HIDE);
		m_pScore->SetTypeBtn();
		m_pScore->SetVisible(false);
		m_pScore->SetCaptionOffset(true);
		m_pScore->DescHidePrivacy();
	}
	if (m_images.size())
	{
		sNoticeImage& image =
			m_images[Doc()->GetMyPartTidList().m_charTid & 0x3ffffff];
		if (image.name.size() && m_pScore)
			m_pScore->SetPPL(MakeStr("%s/%s", SCORE_PPL_DIR, image.name.c_str()));
	}
}

void CScore::OnReload()
{
	this << MsgObject(NULL, M983(0x131), 0, 0, 0, 0, 0);
	this << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
}

template FrAwardForm* CreateForm<FrAwardForm>(FrWndManager*, FrCmdTarget*,
	const char*, FrWnd*);
template FrTreasureScore* CreateForm<FrTreasureScore>(FrWndManager*,
	FrCmdTarget*, const char*, FrWnd*);
template FrTreasureGift* CreateForm<FrTreasureGift>(FrWndManager*, FrCmdTarget*,
	const char*, FrWnd*);

// FrScoreForm::SetDesc is a hybrid without epilogue (see its HACK): a direct call would make the compiler
// treat it as noreturn and drop the code after the call, so OnProcess calls it through this alias
__declspec(dllimport) __forceinline unsigned char RbGameType()
{
	return Doc()->m_golfGame.gameType;
}
struct RbScoreFormDesc
{
	void SetDesc();
};
#pragma comment(linker, \
	"/alternatename:?SetDesc@RbScoreFormDesc@@QAEXXZ=?SetDesc@FrScoreForm@@QAEXXZ")

void CScore::OnProcess(float delta)
{
	if (GOLFDOC()->m_tutorialMode < 7)
		return;
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2)
		return;

	if (m_pRank)
	{
		if (!CCapturedBg::Instance()->IsCaptured() &&
			!(GOLFDOC()->m_gameMode & 0x1e84))
		{
			bool bShow =
				g_pFresh->manager->GetDesktop()->IsWallPaperVisible() == false;
			if (RbGameType() == 10)
				bShow = false;
			m_pRank->SetVisible(bShow);
			if (bShow)
			{
				switch (GOLFDOC()->m_gameMode)
				{
				case 8:
				{
					float mouseX, mouseY;
					SendMsg(this, "Screen", M983(0x1c6), (int)&mouseX, (int)&mouseY,
						0, 0);
					float x = g_view->GetWidth() - m_pRank->GetRect().w;
					if (x - 60.0f > mouseX)
						m_rankPos.x =
							Max<float>(m_rankPos.x - delta * 600.0f, x);
					else
						m_rankPos.x = Min<float>(m_rankPos.x + delta * 600.0f,
							g_view->GetWidth());
					m_pRank->MoveWindow(m_rankPos);
					break;
				}
				case 0x10:
				case 0x20:
					break;
				case 0x40:
					if (Doc()->m_golfGame.gameType == 10)
					{
						m_rankPos.x = g_view->GetWidth() - m_pRank->GetRect().w;
						m_pRank->MoveWindow(m_rankPos);
					}
					else
					{
						m_rankPos.x = Min<float>(m_rankPos.x + delta * 600.0f,
							g_view->GetWidth());
						m_pRank->MoveWindow(m_rankPos);
					}
					break;
				case 0x100:
				case 0x40000:
					m_rankPos.x = Min<float>(m_rankPos.x + delta * 600.0f,
						g_view->GetWidth());
					m_pRank->MoveWindow(m_rankPos);
					break;
				default:
					m_pRank->SetVisible(false);
					break;
				}
			}
		}
		else if (Doc()->m_golfGame.gameType != 10 ||
			!(GOLFDOC()->m_gameMode & 0x880))
			m_pRank->SetVisible(false);
	}

	if (m_pRank3)
	{
		m_pRank3->SetVisible(m_bRank3Shown == 1);
		if (!CQuitWindow::Instance()->IsActive() &&
			!CUserInfo::Instance()->IsVisible() &&
			!CChatMsg::Instance()->IsOpened() &&
			!g_pFresh->GetManager()->IsKeyExclusive() &&
			g_input->GetDown("RANK", true) != 0 &&
			GOLFDOC()->m_tutorialMode >= 15)
		{
			if (m_bRank3Shown == 0)
			{
				if ((GOLFDOC()->m_gameMode & 0xfffffbfe) != 0)
				{
					m_bRank3ByKey = 1;
					m_bRank3Shown = 1;
				}
			}
			else
			{
				m_bRank3ByKey = 0;
				m_bRank3Shown = 0;
			}
		}
	}

	if (m_pScore && !CQuitWindow::Instance()->IsActive() &&
		!CUserInfo::Instance()->IsVisible() &&
		!CChatMsg::Instance()->IsOpened() &&
		!g_pFresh->GetManager()->IsKeyExclusive() &&
		g_input->GetDown("TAB", true) != 0 && g_input->Get("LALT", true) == 0 &&
		g_input->Get("RALT", true) == 0 &&
		g_input->Get("RCONTROL", true) == 0 &&
		g_input->Get("LCONTROL", true) == 0 && GOLFDOC()->m_tutorialMode >= 15 &&
		(GOLFDOC()->m_gameMode & 0xffffdfff) != 0)
	{
		if (m_scoreView == 0)
		{
			if ((GOLFDOC()->m_gameMode & 0xfffffbfe) != 0)
			{
				m_scoreView = 1;
				m_pScore->SetState((FrScoreForm::eState)2);
				((RbScoreFormDesc*)m_pScore)->SetDesc();
				if (SDOC983(unsigned char, 0x10594) != 7)
				{
					m_pTreasureGauge->SetState((FrTreasureGauge::eState)2);
					m_pNextHole->SetState((FrNextHole::eState)2);
				}
			}
		}
		else if (m_scoreView == 1)
		{
			m_scoreView = 0;
			m_pScore->SetState((FrScoreForm::eState)1);
			if (SDOC983(unsigned char, 0x10594) != 7)
			{
				m_pTreasureGauge->SetState((FrTreasureGauge::eState)1);
				m_pNextHole->SetState((FrNextHole::eState)1);
			}
		}
	}

	if (m_pGuildScore && !CQuitWindow::Instance()->IsActive() &&
		!CUserInfo::Instance()->IsVisible() &&
		!CChatMsg::Instance()->IsOpened() &&
		!g_pFresh->GetManager()->IsKeyExclusive() &&
		g_input->GetDown("TAB", true) != 0 && g_input->Get("LALT", true) == 0 &&
		g_input->Get("RALT", true) == 0 &&
		g_input->Get("RCONTROL", true) == 0 &&
		g_input->Get("LCONTROL", true) == 0 && GOLFDOC()->m_tutorialMode >= 15 &&
		(GOLFDOC()->m_gameMode & 0xffffdfff) != 0)
	{
		if (m_scoreView == 0)
		{
			if ((GOLFDOC()->m_gameMode & 0xfffffbfe) != 0)
			{
				m_scoreView = 1;
				m_pGuildScore->SetVisible(true);
				unsigned long guid = Doc()->m_myInfo.info.dwGuid;
				m_pGuildScore->SetGuildPlayer(guid);
				m_pGuildScore->SetVisibleCloseBtn(true); // 983 00794f00: push 1 (TAB mostra o Fechar)
				m_pGuildScore->SetVisibleCancelBtn(false);
			}
		}
		else if (m_scoreView == 1)
		{
			m_scoreView = 0;
			m_pGuildScore->SetVisible(false);
		}
	}

	// 983 00794f00: TAB opens/closes the grand prix scoreboard's detail windows (m_bShowWnd983, +0x423)
	if (m_pGrandPrix && !CQuitWindow::Instance()->IsActive() &&
		!CUserInfo::Instance()->IsVisible() &&
		!CChatMsg::Instance()->IsOpened() &&
		!g_pFresh->GetManager()->IsKeyExclusive() &&
		g_input->GetDown("TAB", true) != 0 && g_input->Get("LALT", true) == 0 &&
		g_input->Get("RALT", true) == 0 &&
		g_input->Get("RCONTROL", true) == 0 &&
		g_input->Get("LCONTROL", true) == 0 &&
		(unsigned char)(SDOC983(unsigned char, 0x109b8) - 0x14) < 2 &&
		m_pGrandPrix->IsVisible())
		m_pGrandPrix->Fn00782330_983(!m_pGrandPrix->m_bShowWnd983);

	// the score form went back to the closed state under us (983 FUN_0077c4a0: m_state == 2)
	if (m_scoreView == 1 && m_pScore && m_pScore->m_state != 2)
	{
		m_scoreView = 0;
		if (m_pTreasureGauge)
			m_pTreasureGauge->SetState((FrTreasureGauge::eState)1);
		if (m_pNextHole)
			m_pNextHole->SetState((FrNextHole::eState)1);
	}

	if (m_scoreView == 2)
	{
		if (m_whiteFade < 1.0f)
		{
			m_whiteFade += delta;
			float t = m_whiteFade;
			if (!(1.0f > t))
				t = 1.0f;
			m_whiteFade = t;
			unsigned char alpha = (unsigned char)(t * 255.0f);
			g_pFresh->manager->GetDesktop()->SetBgColor(
				(alpha << 24) | 0xffffff);
		}
	}
	else if (m_scoreView == 3)
	{
		if (m_whiteFade > 0.0f)
		{
			m_whiteFade -= delta;
			unsigned char alpha =
				(unsigned char)float2int(Max<float>(0.0f, m_whiteFade) * 255.0f);
			g_pFresh->manager->GetDesktop()->SetBgColor(
				(alpha << 24) | 0xffffff);
			if (m_whiteFade < 0.0f)
			{
				m_scoreView = 0;
				m_whiteFade = 0;
			}
		}
	}

	if (m_state == 1 && SDOC983(int, 0x1048c) == 0)
	{
		m_stateTimer += delta;
		if (m_stateTimer > 3.0f && !m_pAward)
		{
			m_pAward =
				CreateForm<FrAwardForm>(g_pFresh->manager, this, "award", NULL);
			((FrFixedWnd*)m_pAward)->Fix();
			m_pAward->Open(NULL, 0xc);
			m_pAward->EnableDrag(false);
			if (m_pAquire)
				m_pAquire->Reset();
			else
				m_pAquire = new cAquireItem;
		}
		if (m_pAquire)
			m_pAquire->OnProc(delta);
	}

	if (SDOC983(unsigned char, 0x10778) == 1 &&
		CTHunter::Instance()->IsReceiveGiftPacket())
	{
		m_stateTimer += delta;
		if (m_stateTimer > 2.0f)
		{
			if (!m_pTreasureScore)
			{
				m_pTreasureScore = CreateForm<FrTreasureScore>(g_pFresh->manager, this,
					"trscore", NULL);
				m_pTreasureScore->Open(NULL, 0xc);
				m_pTreasureScore->EnableDrag(false);
				((FrFixedWnd*)m_pTreasureScore)->Fix();
			}
			if (!m_pTreasureGift)
			{
				m_pTreasureGift = CreateForm<FrTreasureGift>(g_pFresh->manager, this,
					"trgift", NULL);
				m_pTreasureGift->Open(NULL, 0xc);
				m_pTreasureGift->EnableDrag(false);
				((FrFixedWnd*)m_pTreasureGift)->Fix();
			}
		}
	}
}

// 983: sala do Grand Prix (realGameType 0x14/0x15; o 983 testa `add eax, -0x14; cmp eax, 1`; em `switch` o nosso fica mais perto)
// ---------------------------------------------------------------------------------------------------------------------------
// 983: placar do Grand Prix (FrGrandPrixRankForm, "grandprix_scoreboard") e os dois objetos que ele contem. Nao existe no 645.
// Corpos pelo desmonte do 983 (sessao 132); os que ainda faltam estao marcados no scoreboard.h. Nomes meus.

// 983: WRect zerado do .bss (00f14668), o mesmo "retangulo vazio" do recycledlg.cpp
static WRect s_rcEmptyGp983;

sGrandPrixIssueBoard983::sGrandPrixIssueBoard983()
{
}

sGrandPrixIssueBoard983::~sGrandPrixIssueBoard983()
{
}

// 983: 00781b20 (so o 00782050 chama). Nome meu.
void sGrandPrixIssueBoard983::SetCaption983(FrStatic* pText)
{
	pText->SetCaption(K2L_Compatibility(m_text.c_str()));
}

bool sGrandPrixIssueBoard983::IsFirst983(int index)
{
	if (!Doc()->HasRival983(index))
		return false;
	if (Doc()->m_rivalList[index].rank == 1)
		return true;
	return false;
}

bool sGrandPrixIssueBoard983::IsBestHolePang983(int index, unsigned char hole)
{
	__int64 pang = Doc()->m_rivalList[index].holePang[hole - 1];
	for (unsigned int i = 0; i < Doc()->m_rivalList.size(); i++)
	{
		if (i != index && pang < Doc()->m_rivalList[i].holePang[hole - 1])
			return false;
	}
	return true;
}

bool sGrandPrixIssueBoard983::IsHoleScore983(int index, unsigned char hole, int score)
{
	return Doc()->m_rivalList[index].holeScore[hole] == score; // 983: [hole], nao [hole - 1]
}

const std::string& sGrandPrixIssueBoard983::GetText()
{
	return m_text;
}

void sGrandPrixIssueBoard983::GetArgText983(int index, int argType, std::string& text)
{
	if (!Doc()->HasRival983(index))
		return;
	switch (argType)
	{
	case 0:
		text = Doc()->m_rivalList[index].nickname;
		break;
	case 1:
		text = MakeStr("%3d", Doc()->m_rivalList[index].totalScore);
		break;
	case 2:
		text = MakeStr("%I64d", Doc()->m_rivalList[index].holePang[GetHoleIndex(*((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c))) - 1]);
		break;
	}
}

int sGrandPrixIssueBoard983::SelectType983(int index, unsigned char hole, sGrandPrixIssue983& issue)
{
	std::vector<int> types;
	if (IsFirst983(index))
		types.push_back(3);
	if (IsBestHolePang983(index, hole))
		types.push_back(2);
	int score = Doc()->m_rivalList[index].holeScore[hole - 1];
	unsigned char stroke = Doc()->m_pGolfDoc->m_pHoleData[hole - 1].par + score;
	if (stroke == 1)
		types.push_back(6);
	else
	{
		switch (score)
		{
		case -4:
			types.push_back(6);
			break;
		case -3:
			types.push_back(5);
			break;
		case -2:
			types.push_back(4);
			break;
		}
	}
	types.push_back(1);
	unsigned short pick = rand() % types.size();
	for (unsigned int i = 0; i < types.size(); i++)
	{
		if (types[i] == 6)
			return types[i];
	}
	return types[pick];
}

void sGrandPrixIssueBoard983::FindIssue983(int type, sGrandPrixIssue983& issue)
{
	for (unsigned int i = 0; i < m_list.size(); i++)
	{
		sGrandPrixIssue983 item = m_list[i];
		if (type == item.type)
		{
			issue = item;
			return;
		}
	}
}

void sGrandPrixIssueBoard983::MakeText(int index, unsigned char hole)
{
	sGrandPrixIssue983 issue;
	FindIssue983(SelectType983(index, hole, issue), issue);
	std::vector<std::string> args;
	for (unsigned int i = 0; i < issue.values.size(); i++)
	{
		std::string arg;
		GetArgText983(index, issue.values[i], arg);
		args.push_back(arg);
	}
	std::string text;
	switch (issue.values.size())
	{
	case 0:
		text = K2L_Compatibility(issue.text.c_str());
		break;
	case 1:
		text = MakeStr(K2L_Compatibility(issue.text.c_str()), args[0].c_str());
		break;
	case 2:
		text = MakeStr(K2L_Compatibility(issue.text.c_str()), args[0].c_str(), args[1].c_str());
		break;
	}
	m_text = text;
}

bool sGrandPrixIssueBoard983::Load(std::string filename)
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (!script)
		return false;
	if (!script->RunFile(filename.c_str()))
		return false;
	lua_system::lua_table_helper config = script->GetTable("grandprix_issue_board_config");
	int count = LUA983_GET(config, int, "issue_string_count");
	for (int i = 0; i < count; i++)
	{
		sGrandPrixIssue983 issue;
		lua_system::lua_table_helper tbl = script->GetTable(MakeStr("%s%02d", "grandprix_issue_board_string", i + 1));
		std::string text = LUA983_STR(tbl, "issue_string");
		issue.text = text;
		issue.type = LUA983_GET(tbl, int, "issue_string_type");
		int num;
		{
			LuaChild983 args = ((LuaHelper983&)tbl).GetChild983("argment_type");
			num = lua_tinker::call<int, lua_tinker::table>(args.state, "get_array_size", args.table);
		}
		for (int j = 0; j < num; j++)
			issue.values.push_back(((LuaHelper983&)tbl).GetChild983("argment_type").GetInl983<int>(j + 1));
		m_list.push_back(issue);
	}
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
	return true;
}

FrGrandPrixRankEffect983::FrGrandPrixRankEffect983()
{
}

FrGrandPrixRankEffect983::~FrGrandPrixRankEffect983()
{
}

void FrGrandPrixRankEffect983::Init()
{
	m_bmp58[0] = NULL;
	m_bmp58[1] = NULL;
	m_bmp58[2] = NULL;
	m_mode = 2;
	m_rectAC = s_rcEmptyGp983;
	m_rectBC = s_rcEmptyGp983;
}

bool FrGrandPrixRankEffect983::LoadLua983(std::string filename)
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (!script)
		return false;
	script->RunFile(filename.c_str());
	lua_system::lua_table_helper tbl = script->GetTable("grandprix_scoreborad_notice_table");
	m_name[0] = LUA983_STR(tbl, "text_bg_start_img");
	m_name[1] = LUA983_STR(tbl, "text_bg2_middle_img");
	m_name[2] = LUA983_STR(tbl, "text_bg3_end_img");
	m_names64[0].clear();
	m_names64[0].push_back(std::string(LUA983_STR(tbl, "hole_out_waiting_img1_file_name")));
	m_names64[0].push_back(std::string(LUA983_STR(tbl, "hole_out_waiting_img2_file_name")));
	m_names64[1].clear();
	m_names64[1].push_back(std::string(LUA983_STR(tbl, "next_hole_waiting_img1_file_name")));
	m_textPos983.x = LUA983_GET(tbl, float, "notice_text_img_pos_x");
	m_textPos983.y = LUA983_GET(tbl, float, "notice_text_img_pos_y");
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
	return true;
}

void FrGrandPrixRankEffect983::LoadBitmaps983()
{
	for (int i = 0; i < 3; i++)
		m_bmp58[i] = g_pFresh->GetBitmap(m_name[i].c_str());
	for (int j = 0; j < 2; j++)
	{
		m_bmps84[j].clear();
		for (unsigned int k = 0; k < m_names64[j].size(); k++)
			m_bmps84[j].push_back(g_pFresh->GetBitmap(m_names64[j][k].c_str()));
	}
}

void FrGrandPrixRankEffect983::Load()
{
	LoadLua983("grandprix_ingame_ui.lua");
	LoadBitmaps983();
}

void FrGrandPrixRankEffect983::Draw(FrGraphicInterface* pGDI)
{
	if (m_mode == 2)
		return;
	std::vector<const Bitmap*>& list = m_bmps84[m_mode];
	int count = list.size();
	WPoint pos(m_rectBC.x, m_rectBC.y);
	WRect rcStart(pos, m_bmp58[0]->Width(), m_bmp58[0]->Height());
	WRect rcMid(pos.x + m_bmp58[0]->Width(), pos.y, m_rectAC.w, m_bmp58[1]->Height());
	pGDI->DrawTexture(m_bmp58[0], rcStart, 0xffffffff, 0);
	pGDI->DrawTexture(m_bmp58[1], WRect(0.0f, 0.0f, m_bmp58[1]->Width(), m_bmp58[1]->Height()), rcMid, 0xffffffff, 0);
	pGDI->DrawTexture(m_bmp58[2], WRect(rcMid.Right(), rcMid.y, m_bmp58[2]->Width(), m_bmp58[2]->Height()), 0xffffffff, 0);
	float offset = 0.0f;
	for (int i = 0; i < count; i++)
	{
		const Bitmap* pBmp = list[i];
		WPoint pt(rcMid.x + offset, m_rectAC.y);
		pGDI->DrawTexture(pBmp, WRect(pt, pBmp->Width(), pBmp->Height()), 0xffffffff, 0);
		offset += pBmp->Width();
	}
}

void FrGrandPrixRankEffect983::Start(const WRect& rect)
{
	if (m_mode == 2)
		return;
	std::vector<const Bitmap*>& list = m_bmps84[m_mode];
	int count = list.size();
	int width = 0;
	for (int i = 0; i < count; i++)
		width += list[i]->Width();
	if (m_mode == 1)
		width += 50.0f;
	m_rectBC.x = rect.Right() - (m_bmp58[2]->Width() + m_bmp58[0]->Width() + width);
	m_rectBC.y = rect.Bottom() + m_textPos983.y;
	m_rectBC.w = m_bmp58[2]->Width() + m_bmp58[0]->Width() + width;
	m_rectBC.h = m_bmp58[0]->Height();
	m_rectAC.x = rect.Right() - (m_bmp58[2]->Width() + width);
	m_rectAC.y = m_rectBC.y + (m_rectBC.h * 0.5 - (float)(list[0]->Height() / 2));
	m_rectAC.w = width;
	m_rectAC.h = list[0]->Height();
}

IMPLEMENT_OBJECT(FrGrandPrixRankForm, FrForm)

// 983: inicializador 00cd59f0. Sem estas entradas os ponteiros +0x110..+0x134 ficavam NULL e o OnInit (Fn0077e660_983,
// m_pLimitedTimeWnd983) caia ao abrir o placar do Grand Prix; sem o OWNERDRAW da ranklist (0078a4f0) a lista ficava vazia.
BEGIN_FRESH_MSGMAP(FrGrandPrixRankForm, FrForm)

ON_FRESH_VI("ranklist", FRCMD_INIT, FrGrandPrixRankForm::OnRankListInit983)
ON_FRESH_VI("ranklist", FRCMD_OWNERDRAW, FrGrandPrixRankForm::OnRankListOwnerDraw983)
ON_FRESH_VV("ranklist", FRCMD_RBUTTONUP, FrGrandPrixRankForm::OnRankListRBtnUp983)
ON_FRESH_VV("ranklist", FRCMD_DBLCLICK, FrGrandPrixRankForm::OnRankListRBtnUp983)
ON_FRESH_VI("gamename", FRCMD_INIT, FrGrandPrixRankForm::OnGameNameInit983)
ON_FRESH_VI("issueboard", FRCMD_INIT, FrGrandPrixRankForm::OnIssueBoardInit983)
ON_FRESH_VI("hole_limited_time", FRCMD_INIT, FrGrandPrixRankForm::OnLimitedTimeInit983)
ON_FRESH_VI("hole_limited_time", FRCMD_OWNERDRAW, FrGrandPrixRankForm::OnLimitedTimeOwnerDraw983)
ON_FRESH_VI("bg1", FRCMD_INIT, FrGrandPrixRankForm::OnBg1Init983)
ON_FRESH_VI("bg2", FRCMD_INIT, FrGrandPrixRankForm::OnBg2Init983)
ON_FRESH_VI("bg3", FRCMD_INIT, FrGrandPrixRankForm::OnBg3Init983)
ON_FRESH_VI("bg4", FRCMD_INIT, FrGrandPrixRankForm::OnBg4Init983)
ON_FRESH_VI("bg5", FRCMD_INIT, FrGrandPrixRankForm::OnBg5Init983)
ON_FRESH_VI("bg6", FRCMD_INIT, FrGrandPrixRankForm::OnBg6Init983)

END_FRESH_MSGMAP()

void FrGrandPrixRankForm::OnRankListInit983(int param)
{
	m_pRankList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pRankList->UseRightButton(true);
}

void FrGrandPrixRankForm::OnRankListRBtnUp983()
{
	if (m_pRankList->GetItemUnderCursor())
	{
		sRivalData* pRival = (sRivalData*)m_pRankList->GetItemUnderCursor()->pData;
		if (pRival && CUserInfo::Instance() && pRival->state != 3)
			CUserInfo::Instance()->SetInfo(pRival->uid, pRival->oid, false, true, false, false, pRival->nickname,
				pRival->bGrandPrixAI983, pRival->grandPrixAIId983);
	}
}

// 983 0077bfb0: imagem do resultado do buraco (m_p338) pela diferenca para o par; 1 tacada = hole in one (0)
static int GrandPrixScoreIcon983(int score, char strokes)
{
	if (strokes == 1)
		return 0;
	switch (score)
	{
	case -4: return 0;
	case -3: return 1;
	case -2: return 2;
	case -1: return 3;
	case 0: return 4;
	case 1: return 5;
	case 2: return 6;
	case 3: return 7;
	case 4: return 8;
	default: return 9;
	}
}

// 983 0078a4f0 -> 0078a3f0: uma coluna por ajudante, nas posicoes m_point983 (grandprix_ingame_ui.lua)
void FrGrandPrixRankForm::OnRankListOwnerDraw983(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!param || !pGDI)
		return;
	FrListItem* pItem = (FrListItem*)param;
	const sRivalData* pRival = (const sRivalData*)pItem->pData;
	const WPoint& pos = pItem->pos;
	const char* raw = (const char*)pRival;

	int space = pGDI->SetSpace(0);
	pGDI->SetTextStyle(0);
	unsigned long color = pGDI->GetTextColor();
	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	if (pRival->state == 3)
		pGDI->SetTextColor(0xFF808080, 0xFFFFFFFF);

	// 0077e6c0: ordem
	pGDI->Print(WPoint(m_point983[0].x + pos.x, m_point983[0].y + pos.y), 1, "%d", pRival->rank);

	// 00785a40: fundo da ordem pelo tipo (007822d0)
	if (const Bitmap* pBmp = m_p1f8[GetRankType983(m_curHole983, pRival)])
		pGDI->DrawTexture(pBmp, WRect(pos.x + m_point983[1].x, pos.y + m_point983[1].y,
			(float)pBmp->Width(), (float)pBmp->Height()), 0xFFFFFFFF, 0);

	// 00781df0: titulo ou nivel (anfitriao = gp_host); 0077e710: emblema da guilda; convidado (+0x1b4) sem os dois
	if (!pRival->bGrandPrixAI983)
	{
		IFF_STRUCT::sSkin* pSkin;
		const Bitmap* pLevel;
		if (pRival->titleTypeId && (pSkin = ItemManager()->FindSkin(pRival->titleTypeId)) != NULL)
			pLevel = g_pFresh->GetManager()->GetBitmap("TITLES", pSkin->c.Icon);
		else
			pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS", MakeStr("level_%03d", pRival->level + 1));
		if (pRival->capability & 0x8000)
			pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");
		if (pLevel)
			pGDI->DrawTexture(pLevel, WRect(m_point983[2].x + pos.x, m_point983[2].y + pos.y,
				(float)pLevel->Width(), (float)pLevel->Height()), 0xFFFFFFFF, 0);
	}
	if (!pRival->bGrandPrixAI983 && pRival->guildUID)
	{
		if (const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(pRival->guildUID, pRival->guildEmblemVer983))
			pGDI->DrawTexture(pEmblem, WRect(m_point983[3].x + pos.x, m_point983[3].y + pos.y,
				(float)pEmblem->Width(), (float)pEmblem->Height()), 0xFFFFFFFF, 0);
	}

	// 0077e7d0: apelido
	if (!CProjectG::Instance()->HidePrivacy())
		g_pFresh->GetManager()->PrintText(WPoint(m_point983[4].x + pos.x, m_point983[4].y + pos.y), 0,
			pRival->nickname, 90.0f, 0xFFFFFFFF);

	// 00781f20: resultado do buraco atual (convidado: buraco m_holeIndex983); o 983 le por deslocamento a partir do buraco
	// (1-based: +0x34 + hole*4 = holeScore[hole-1], +0x7f + hole = holeStroke[hole-1])
	if (pRival->bGrandPrixAI983 || raw[0x7f + m_curHole983] || pRival->state == 3)
	{
		int score = pRival->bGrandPrixAI983 == 1 ? pRival->holeScore[m_holeIndex983] : *(const int*)(raw + 0x34 + m_curHole983 * 4);
		if (score > -5)
		{
			char strokes = *((const char*)*(char**)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) - 0x20 + m_curHole983 * 0x38) + (char)score;
			if (strokes)
			{
				int icon = GrandPrixScoreIcon983(score, strokes);
				if (pRival->state == 3)
					icon = 10;
				if (const Bitmap* pBmp = m_p338[icon])
					pGDI->DrawTexture(pBmp, WRect(m_point983[5].x + pos.x, m_point983[5].y + pos.y,
						(float)pBmp->Width(), (float)pBmp->Height()), 0xFFFFFFFF, 0);
			}
		}
	}

	// 0077e840 / 0077e890: total e Pang
	pGDI->Print(WPoint(m_point983[6].x + pos.x, m_point983[6].y + pos.y), 2, "%3d", pRival->totalScore);
	pGDI->Print(WPoint(m_point983[7].x + pos.x, m_point983[7].y + pos.y), 2, "%I64d(+%I64d)",
		pRival->totalPang, pRival->totalBonusPang);

	// 00789700: barra do personagem ("bar_<icone>"); o proprio jogador pelo personagem em uso (CSharedDoc +0xa2a)
	unsigned long charTid = 0;
	if (Doc()->m_myInfo.info.dwGuid == pRival->oid && !pRival->bGrandPrixAI983)
	{
		std::map<unsigned int, sCharacterInfo>::iterator it = Doc()->m_charMap.find(*(unsigned int*)&Doc()->m_myInfo.userEquip.guidChar);
		if (it != Doc()->m_charMap.end())
			charTid = it->second.tid;
	}
	else
		charTid = pRival->tidChar983;
	if (IFF_ITEM_COMMON* pInfo = ItemManager()->FindCommonItem(charTid))
	{
		if (const Bitmap* pBar = g_pFresh->GetBitmap(MakeStr("bar_%s", pInfo->Icon)))
			pGDI->DrawTexture(pBar, WRect(m_point983[8].x + pos.x, m_point983[8].y + pos.y,
				(float)pBar->Width(), (float)pBar->Height()), 0xFFFFFFFF, 0);
	}

	pGDI->SetTextColor(color, 0xFFFFFFFF);
	pGDI->SetSpace(space);

	// 00785af0: linha do proprio jogador selecionada e com o destaque de 3 pecas (m_p3b8) ate a largura da linha
	if (pRival->oid == Doc()->m_myInfo.info.dwGuid && !pRival->bGrandPrixAI983)
	{
		m_pRankList->SelectItem(pItem, true);
		const Bitmap* pLeft = m_p3b8[0];
		const Bitmap* pMid = m_p3b8[1];
		const Bitmap* pRight = m_p3b8[2];
		float y = pos.y + m_point3c4.y;
		float left = m_point3c4.x + pos.x;
		float midX = left + (float)pLeft->Width();
		float rightX = ((float)m_pRankList->GetItemWidth() + pos.x) - (float)pRight->Width();
		float h = (float)pLeft->Height();
		pGDI->DrawTexture(pLeft, WRect(left, y, (float)pLeft->Width(), h), 0xFFFFFFFF, 0);
		pGDI->DrawTexture(pMid, WRect(0.0f, 0.0f, (float)pMid->Width(), (float)pMid->Height()),
			WRect(midX, y, rightX - midX, h), 0xFFFFFFFF, 0);
		pGDI->DrawTexture(pRight, WRect(rightX, y, (float)pRight->Width(), (float)pRight->Height()), 0xFFFFFFFF, 0);
	}
}

void FrGrandPrixRankForm::OnGameNameInit983(int param)
{
	m_pWnd114 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	std::string title(Doc()->m_roomInfo.title);
	m_pWnd114->SetCaption(title.c_str());
}

void FrGrandPrixRankForm::OnIssueBoardInit983(int param)
{
	m_pIssueText = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	WRect rc = m_pIssueText->GetRect();
	rc.x = (float)(m_rect.w * 0.5 + m_rect.x);
	m_pIssueText->SetRect(rc);
	m_pIssueText->m_nFlags.Enable(0x1000);
}

void FrGrandPrixRankForm::OnLimitedTimeInit983(int param)
{
	m_pLimitedTimeWnd983 = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
}

// 983 0077e8f0: segundos que faltam (GetWaitTime) em "%02d" com a fonte de titulo, a direita da faixa do efeito (-50)
void FrGrandPrixRankForm::OnLimitedTimeOwnerDraw983(int)
{
	if (!m_effect.m_mode || !g_pFresh->GetManager()->GetGDI() || !m_pTitleFont)
		return;
	int minutes = 0;
	int seconds = (int)GetWaitTime();
	if (GetWaitTime() > 60.0)
		minutes = seconds / 60;
	MakeStr("%02d", minutes); // o 983 monta e descarta
	char* text = MakeStr("%02d", seconds);
	m_pos983.x = (float)(m_effect.m_rectAC.w + m_effect.m_rectAC.x - 50.0);
	m_pos983.y = m_effect.m_rectAC.y;
	m_pos983.x = m_offset983.x + m_pos983.x;
	m_pos983.y = m_offset983.y + m_pos983.y;
	m_pTitleFont->Print(g_view, m_pos983.x, m_pos983.y, text, 0, 0xffffffff, NULL);
}

void FrGrandPrixRankForm::OnBg1Init983(int param)
{
	m_pWnd983[0] = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
}

void FrGrandPrixRankForm::OnBg2Init983(int param)
{
	m_pWnd983[1] = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
}

void FrGrandPrixRankForm::OnBg3Init983(int param)
{
	m_pWnd983[2] = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
}

void FrGrandPrixRankForm::OnBg4Init983(int param)
{
	m_pWnd983[3] = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
}

void FrGrandPrixRankForm::OnBg5Init983(int param)
{
	m_pWnd983[4] = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
}

void FrGrandPrixRankForm::OnBg6Init983(int param)
{
	m_pWnd983[5] = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
}

void FrGrandPrixRankForm::ClearRankList()
{
	m_pRankList->ClearItem();
}

void FrGrandPrixRankForm::UpdateBlink983(float dt)
{
	m_blinkElapsed983 += dt;
	if (m_blinkElapsed983 >= m_blinkTime983)
	{
		m_blinkElapsed983 = 0.0f;
		m_bBlink983 = !m_bBlink983;
	}
}

bool FrGrandPrixRankForm::UpdateTimer983(float dt)
{
	m_timer983 += dt;
	if (m_timer983 > (double)0.05f) // 983: compara com o double de 0.05f (00d18d78)
	{
		m_timer983 = 0.0f;
		return false;
	}
	return true;
}

float FrGrandPrixRankForm::GetWaitTime()
{
	return m_waitTime;
}

void FrGrandPrixRankForm::Fn0077bef0_983()
{
	m_bNextHoleWait983 = true;
}

void FrGrandPrixRankForm::Fn0077bf00_983()
{
	m_bNextHoleWait983 = false;
}

bool FrGrandPrixRankForm::Fn0077bf10_983()
{
	return m_bNextHoleWait983;
}

void FrGrandPrixRankForm::Fn0077bf20_983()
{
	m_bGameOverWait983 = true;
}

void FrGrandPrixRankForm::ReleaseTitleFont983()
{
	if (m_pTitleFont)
		m_pTitleFont->Erase();
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

void FrGrandPrixRankForm::ResetSounds983()
{
	for (int i = 0; i < 7; i++)
		m_bSoundPlayed983[i] = false;
}

void FrGrandPrixRankForm::Init983()
{
	m_waitTime = 0.0f;
	m_pRankList = NULL;
	m_pIssueText = NULL;
	m_pos983 = WPoint(0.0f, 0.0f);
	m_blinkTime983 = 0.0f;
	m_blinkElapsed983 = 0.0f;
	m_pLimitedTimeWnd983 = NULL;
	m_pTitleFont = NULL;
	m_bNextHoleWait983 = false;
	m_bGameOverWait983 = false;
	m_bBlink983 = true;
	for (int i = 0; i < 3; i++)
		m_p1f8[i] = NULL;
	for (int i = 0; i < 11; i++)
		m_p338[i] = NULL;
	for (int i = 0; i < 3; i++)
		m_p3b8[i] = NULL;
	m_point3c4 = WPoint(0.0f, 0.0f);
	m_curHole983 = 1;
	m_holeIndex983 = 0;
	m_timer983 = 0.0f;
	m_bRankReady983 = false;
	m_bShowWnd983 = true;
	for (int i = 0; i < 6; i++)
		m_pWnd983[i] = NULL;
}

FrGrandPrixRankForm::FrGrandPrixRankForm()
{
	Init983();
	m_effect.Init();
}

FrGrandPrixRankForm::~FrGrandPrixRankForm()
{
	ClearRankList();
	ReleaseTitleFont983();
}

void FrGrandPrixRankForm::OnDraw()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI)
		m_effect.Draw(pGDI);
}

void FrGrandPrixRankForm::ScrollByKey983(float dt)
{
	if (UpdateTimer983(dt))
		return;
	if (g_input->Get("\xbb\xf3", false))
		m_pRankList->GetScrollBar()->ScrollUp(1);
	else if (g_input->Get("\xc7\xcf", false))
		m_pRankList->GetScrollBar()->ScrollDown(1);
}

void FrGrandPrixRankForm::ResetPoints983()
{
	for (int i = 0; i < 9; i++)
		m_point983[i] = WPoint(0.0f, 0.0f);
}

void FrGrandPrixRankForm::CreateTitleFont983()
{
	static char* s_fontFile[] = { "[font_condition_x2.png" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	m_pTitleFont = NULL;
	info.filename = s_fontFile;
	info.numPages = 1;
	info.fontw = 32;
	info.fonth = 32;
	info.texw = 256;
	info.texh = 64;
	info.pCharSet = "1234567890%";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);
	m_pTitleFont->SetScale(1.0f);
}

void FrGrandPrixRankForm::Fn0077e660_983()
{
	const WRect& rect = m_pLimitedTimeWnd983->GetRect();
	m_pos983 = WPoint(rect.x, rect.y);
	m_pos983.Offset(m_offset983);
}

void FrGrandPrixRankForm::Fn0077ea00_983()
{
	m_effect.Start(m_rect);
}

void FrGrandPrixRankForm::UpdateHole983()
{
	m_curHole983 = *((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c));
	m_holeIndex983 = GetHoleIndex(m_curHole983) - 1;
}

bool FrGrandPrixRankForm::LoadBitmaps983()
{
	m_p1f8[0] = g_pFresh->GetBitmap(m_name1a4[0].c_str());
	m_p1f8[1] = g_pFresh->GetBitmap(m_name1a4[1].c_str());
	m_p1f8[2] = g_pFresh->GetBitmap(m_name1a4[2].c_str());
	for (int i = 0; i < 11; i++)
		m_p338[i] = g_pFresh->GetBitmap(m_name204[i].c_str());
	for (int i = 0; i < 3; i++)
		m_p3b8[i] = g_pFresh->GetBitmap(m_name364[i].c_str());
	return true;
}

void FrGrandPrixRankForm::SetWaitTime(float t)
{
	if (Doc()->m_pGolfDoc->m_b195_983 == true)
		t = 5.0f;
	m_waitTime = t;
}

void FrGrandPrixRankForm::Fn00782070_983()
{
	for (unsigned int i = 0; i < Doc()->m_rivalList.size(); i++)
		Doc()->m_rivalList[i].prevRank983 = Doc()->m_rivalList[i].rank;
}

void FrGrandPrixRankForm::PlaySound983(int index, bool bPlay)
{
	if (index != 0)
	{
		if (index > 0 && index <= 6 && bPlay)
			g_audio->PlaySfx(m_soundList[index].c_str());
	}
	else if (bPlay)
		g_audio->PlayBGM(m_soundList[0].c_str(), false, true);
}

const char* FrGrandPrixRankForm::GetSoundName983(unsigned int index)
{
	if (index >= m_soundList.size())
		return NULL;
	return m_soundList[index].c_str();
}

void FrGrandPrixRankForm::PlayCountSound983(int count)
{
	if (!m_bSoundPlayed983[0] && count <= 0)
	{
		PlaySound983(2, true);
		m_bSoundPlayed983[0] = true;
	}
	if (!m_bSoundPlayed983[1] && count <= 1)
	{
		PlaySound983(1, true);
		m_bSoundPlayed983[1] = true;
	}
	if (!m_bSoundPlayed983[2] && count <= 2)
	{
		PlaySound983(1, true);
		m_bSoundPlayed983[2] = true;
	}
	if (!m_bSoundPlayed983[3] && count <= 3)
	{
		PlaySound983(1, true);
		m_bSoundPlayed983[3] = true;
	}
}

void FrGrandPrixRankForm::PlayRankSound983(int type)
{
	switch (type)
	{
	case 0:
		if (!m_bSoundPlayed983[4])
		{
			PlaySound983(4, true);
			m_bSoundPlayed983[4] = true;
		}
		break;
	case 1:
		if (!m_bSoundPlayed983[5])
		{
			PlaySound983(5, true);
			m_bSoundPlayed983[5] = true;
		}
		break;
	case 2:
		if (!m_bSoundPlayed983[6])
		{
			PlaySound983(6, true);
			m_bSoundPlayed983[6] = true;
		}
		break;
	}
}

int FrGrandPrixRankForm::GetRankType983(unsigned char hole, const sRivalData* pRival)
{
	int type;
	if (pRival->rank > pRival->prevRank983)
		type = 2;
	else
		type = pRival->rank < pRival->prevRank983 ? 0 : 1;
	if (GetHoleIndex(hole) == 1 || !m_bRankReady983)
		type = 1;
	return type;
}

namespace
{
// 983: 0077cf90 = std::for_each com este functor (SetVisible de cada janela pelo +0x423)
struct GpSetVisible983
{
	FrGrandPrixRankForm* pForm;
	GpSetVisible983(FrGrandPrixRankForm* p) : pForm(p) { }
	void operator()(FrWnd* pWnd) const
	{
		if (pWnd)
			pWnd->SetVisible(pForm->m_bShowWnd983);
	}
};
} // namespace

void FrGrandPrixRankForm::Fn00782330_983(bool b)
{
	if (m_bShowWnd983 == b)
		return;
	m_bShowWnd983 = b;
	std::for_each(m_pWnd983, m_pWnd983 + 6, GpSetVisible983(this));
	m_pRankList->SetVisible(m_bShowWnd983);
	m_pIssueText->SetVisible(m_bShowWnd983);
	m_pWnd114->SetVisible(m_bShowWnd983);
}

void FrGrandPrixRankForm::UpdateWait983(float dt)
{
	if (m_waitTime >= 0.0f && (m_bNextHoleWait983 || m_bGameOverWait983))
	{
		m_waitTime -= dt;
		PlayCountSound983((int)m_waitTime);
		if (m_waitTime <= 0.0f)
		{
			if (m_bNextHoleWait983)
			{
				ResetSounds983();
				Fn0077bf00_983();
				// 983: mensagem 0x33b ao GolfRule (numero do 983, escrito direto)
				AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0x33b, 0, 0, 0, 0, 0);
			}
			else if (m_bGameOverWait983)
			{
				Doc()->m_bGameOver = true; // 983: CSharedDoc +0x10488
				m_bGameOverWait983 = false;
			}
		}
	}
}

void FrGrandPrixRankForm::Show(bool bShow)
{
	Enable(bShow);
	SetVisible(bShow);
	PlaySound983(0, bShow);
	if (bShow)
	{
		UpdateHole983();
		if (!m_bRankReady983)
		{
			m_effect.m_mode = 0;
			m_effect.Start(m_rect);
		}
	}
}

void FrGrandPrixRankForm::OnProc(const float deltaTime)
{
	UpdateWait983(deltaTime);
	UpdateBlink983(deltaTime);
	ScrollByKey983(deltaTime);
}

void FrGrandPrixRankForm::Align983()
{
	std::list<FrListItem*>& items = m_pRankList->m_itemList;
	int row = 0;
	for (std::list<FrListItem*>::iterator it = items.begin(); it != items.end(); ++it)
	{
		sRivalData* pRival = (sRivalData*)(*it)->pData;
		if (pRival->oid == MyGuid(true) && !pRival->bGrandPrixAI983)
			break;
		row++;
	}

	FrScrollBar* pScroll = m_pRankList->GetScrollBar();
	if (pScroll)
	{
		int maxRow = Doc()->m_rivalList.size() - 8;
		int top = row - 4;
		if (top < 0)
			top = 0;
		else if (top > maxRow)
			top = maxRow;
		pScroll->SetCurTopRow(top);
	}
}

// 983: 0077b8f0, comparador do placar do Grand Prix (rank menor primeiro). Nome meu.
// O 983 testa os dois lados (empate = false); o "rank maior" vira o mesmo ramo do false (jae + xor al, al).
bool GrandPrixRankCompare983(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;
	if (pA->rank < pB->rank)
		return true;
	if (pA->rank > pB->rank)
		return false;
	return false;
}

void FrGrandPrixRankForm::RefreshRank()
{
	if (*(unsigned long*)((unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x158)) & 0x2000)
	{
		Doc()->CalcRivalRank983();
		m_pRankList->ClearItem();
		std::vector<sRivalData>& rivals = Doc()->m_rivalList;
		for (std::vector<sRivalData>::iterator it = rivals.begin(), end = rivals.end(); it != end; ++it)
		{
			if (it->state)
				m_pRankList->AddItem(&*it);
		}
		m_pRankList->SortItem(GrandPrixRankCompare983);
	}
	Align983();
}

void FrGrandPrixRankForm::Fn007896d0_983(unsigned short v)
{
	if (m_pRankList && Doc()->HasRival983(v))
		RefreshRank();
}

void FrGrandPrixRankForm::Fn0078eb00_983()
{
	m_bRankReady983 = true;
	// 983: CSharedDoc +0x812/+0x816/+0x81a (ver CScore::HandleMsg)
	// 983: Doc() lido uma vez e guardado (edi) para o m_rivalList
	CSharedDoc* pDoc = Doc();
	unsigned char index = pDoc->GetIndex(((pDoc->m_myInfo.info.dwIdentity >> 1) & 1) ? pDoc->m_myInfo.info.dwGalleryGuid : pDoc->m_myInfo.info.dwGuid);
	PlayRankSound983(GetRankType983(m_curHole983, &pDoc->m_rivalList[index]));
	m_effect.m_mode = 1;
	m_effect.Start(m_rect);
	AfxGetTask()->GetMainActor()->SendMsg(NULL, "Screen", 0x248, 0, 0, 0, 0);
}

bool FrGrandPrixRankForm::LoadUi983(std::string filename)
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (!script)
		return false;
	script->RunFile(filename.c_str());

	lua_system::lua_table_helper posTable = script->GetTable("grandprix_rank_item_pos_table");
	int count = lua_tinker::call<int, lua_tinker::table>(posTable.state, "get_array_size", posTable.table);
	WPoint* pPoint = m_point983;
	for (int i = 1; i <= count; i += 2)
	{
		float x = ((LuaChild983&)posTable).GetByIndex(i);
		float y = ((LuaChild983&)posTable).GetByIndex(i + 1);
		pPoint->x = x;
		pPoint->y = y;
		pPoint++;
	}

	lua_system::lua_table_helper waitTable = script->GetTable("grandprix_next_hole_waiting_time_table");
	SetWaitTime(LUA983_GET(waitTable, float, "NextHoleWaitingTime"));
	m_offset983.x = LUA983_GET(waitTable, float, "NextHoleWaitingTimePos_x");
	m_offset983.y = LUA983_GET(waitTable, float, "NextHoleWaitingTimePos_y");
	m_blinkTime983 = LUA983_GET(waitTable, float, "ColonFlickerTime");

	lua_system::lua_table_helper arrowTable = script->GetTable("grandprix_rank_arrow_img_name_table");
	m_name1a4[0] = LUA983_STR(arrowTable, "ranklist_arrow_Up_img_file_name");
	m_name1a4[1] = LUA983_STR(arrowTable, "ranklist_ranking_tie_img_file_name");
	m_name1a4[2] = LUA983_STR(arrowTable, "ranklist_arrow_down_img_file_name");

	lua_system::lua_table_helper holeTable = script->GetTable("grandprix_holein_record_img_file_name_table");
	m_name204[0] = LUA983_STR(holeTable, "holeinone_img_file_name");
	m_name204[1] = LUA983_STR(holeTable, "albatross_img_file_name");
	m_name204[2] = LUA983_STR(holeTable, "eagle_img_file_name");
	m_name204[3] = LUA983_STR(holeTable, "birdie_img_file_name");
	m_name204[4] = LUA983_STR(holeTable, "par_img_file_name");
	m_name204[5] = LUA983_STR(holeTable, "bogey_img_file_name");
	m_name204[6] = LUA983_STR(holeTable, "double_bogey_img_file_name");
	m_name204[7] = LUA983_STR(holeTable, "tripple_bogey_img_file_name");
	m_name204[8] = LUA983_STR(holeTable, "plus4_img_file_name");
	m_name204[9] = LUA983_STR(holeTable, "plus5_img_file_name");
	m_name204[10] = LUA983_STR(holeTable, "CourseOut_img_file_name");

	lua_system::lua_table_helper myRankTable = script->GetTable("grandprix_scoreboard_my_rank_img_table");
	m_name364[0] = LUA983_STR(myRankTable, "my_rank_start_img_file_name");
	m_name364[1] = LUA983_STR(myRankTable, "my_rank_middle_img_file_name");
	m_name364[2] = LUA983_STR(myRankTable, "my_rank_end_img_file_name");
	m_point3c4.x = LUA983_GET(myRankTable, float, "my_rank_mark_start_img_pos_x");
	m_point3c4.y = LUA983_GET(myRankTable, float, "my_rank_mark_start_img_pos_y");

	lua_system::lua_table_helper soundTable = script->GetTable("grandprix_scoreboard_sound_file_name_table");
	int soundCount = lua_tinker::call<int, lua_tinker::table>(soundTable.state, "get_array_size", soundTable.table);
	for (int i = 0; i < soundCount; i++)
	{
		std::string name = ((LuaChild983&)soundTable).GetInl983<const char*>(i + 1);
		m_soundList.push_back(name);
	}
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
	return true;
}

void FrGrandPrixRankForm::Fn00797980_983(unsigned short v)
{
	if (!m_pIssueText)
		return;
	m_issueBoard.MakeText(v, m_curHole983);
	m_pIssueText->SetCaption(K2L_Compatibility(m_issueBoard.GetText().c_str()));
}

// 983: 00782050, sem chamador nem referencia no 983 (o argumento nao e lido). Nome meu.
void FrGrandPrixRankForm::SetIssueText983(int)
{
	m_issueBoard.SetCaption983(m_pIssueText);
}

bool FrGrandPrixRankForm::OnInit()
{
	UpdateHole983();
	ResetPoints983();
	CreateTitleFont983();
	if (!LoadUi983("grandprix_ingame_ui.lua"))
		return false;
	if (!LoadBitmaps983())
		return false;
	m_issueBoard.Load("grandprix_issueboard.lua");
	m_effect.Load();
	Fn0077e660_983();
	Fn0077bf00_983();
	ResetSounds983();
	return true;
}

#define GRANDPRIX_ROOM983() IsGrandPrixRoom983()
// 983 00780360: (this+0x34)->flags(+0x44) & 1 = m_pScore visivel
bool CScore::IsScoreShown983() const
{
	return m_pScore && m_pScore->IsVisible();
}

void CScore::HandleMsg(const MsgObject& msg)
{
	// 983 (007980f0): numeros 0x164 e 0x16e..0x178 escritos direto (0x16e = UpdateRivals, o M983(0x13c) provisorio do 645; o resto
	// e do placar do Grand Prix, m_pGrandPrix). O 0x15e (levelupitem, M983(0x12d)) nao passa mais por aqui (so o CTaskMain cria a janela).
	switch (msg.message)
	{
	case M983(0x57):
		m_pRank3->SetScore(msg.param1, msg.param2);
		return;
	case M983(0x2a):
		if (!IsMassGame())
			return;
		if (m_pRank)
		{
			m_pRank->SortRank();
			if (msg.param1 != 0)
				m_pRank->Align();
		}
		if (m_pRank2)
			m_pRank2->SortRank();
		if (m_pRankEx)
			m_pRankEx->SortRank();
		if (m_pGrandPrix)
			m_pGrandPrix->RefreshRank();
		return;
	case M983(0x12e):
		if (msg.param1 == 1)
			CheckOpenedDlg();
		m_state = msg.param1;
		return;
	case M983(0x12f):
		if (GRANDPRIX_ROOM983())
			return;
		if (msg.param1 == 0)
		{
			m_scoreView = 0;
			if (IsMassGame() && m_pRankEx)
				m_pRankEx->SetActive(false);
			if (m_pScore)
				m_pScore->SetState((FrScoreForm::eState)1);
			if (m_pTreasureGauge)
				m_pTreasureGauge->SetState((FrTreasureGauge::eState)1);
			if (m_pNextHole)
				m_pNextHole->SetState((FrNextHole::eState)1);
			if (SDOC983(unsigned char, 0x10594) == 6)
			{
				if (m_pGuildScore)
					m_pGuildScore->SetVisible(false);
				if (m_bRank3ByKey == 1)
					m_bRank3Shown = 1;
			}
		}
		else if (msg.param1 == 1)
		{
			m_whiteFade = 0.0f;
			m_scoreView = 2;
			if (m_pRank)
				m_pRank->SetVisible(false);
			if (m_pRank3)
				m_pRank3->SetVisible(m_bRank3Shown == 1);
			if (IsMassGame() && Doc()->m_roomInfo.realGameType != 0x13)
			{
				if (m_pRankEx)
					m_pRankEx->SetActive(true);
				if (m_pScore)
					m_pScore->SetState((FrScoreForm::eState)1);
				if (SDOC983(unsigned char, 0x10594) != 5)
				{
					if (m_pTreasureGauge)
						m_pTreasureGauge->SetState((FrTreasureGauge::eState)3);
					if (m_pNextHole)
						m_pNextHole->SetState((FrNextHole::eState)3);
				}
				if (SDOC983(unsigned char, 0x10594) == 6)
				{
					if (m_pGuildScore)
					{
						m_pGuildScore->SetVisible(true);
						m_pGuildScore->SetGuildPlayer(SDOC983(unsigned long, 0x81a));
						m_pGuildScore->SetVisibleCloseBtn(false);
						m_pGuildScore->SetVisibleCancelBtn(false);
					}
					m_bRank3Shown = 0;
				}
			}
			else
			{
				if (m_pScore)
				{
					m_pScore->SetState((FrScoreForm::eState)3);
					m_pScore->SetDesc();
				}
				if (SDOC983(unsigned char, 0x10594) != 5)
				{
					if (m_pTreasureGauge)
						m_pTreasureGauge->SetState((FrTreasureGauge::eState)3);
					if (m_pNextHole)
						m_pNextHole->SetState((FrNextHole::eState)2);
				}
			}
		}
		else if (msg.param1 == 4 && m_pTreasureGauge)
			m_pTreasureGauge->SetTPoint();
		if (GRANDPRIX_ROOM983())
			m_bg.ScoreBgCommand(4);
		else
			m_bg.ScoreBgCommand(msg.param1);
		return;
	case M983(0x130):
		m_scoreView = 3;
		m_whiteFade = 1.0f;
		if (m_pRank && SDOC983(unsigned char, 0x10594) != 10)
			m_pRank->SetVisible(true);
		if (m_pRank3)
			m_pRank3->SetVisible(m_bRank3Shown == 1);
		if (m_pRankEx)
			m_pRankEx->SetActive(false);
		if (m_pGrandPrix)
			m_pGrandPrix->Show(false);
		if (m_pScore)
			m_pScore->SetState((FrScoreForm::eState)1);
		if (m_pItems)
			m_pItems->SetVisible(false);
		return;
	case M983(0x131):
		if (m_pScore)
			m_pScore->SortLists(true);
		if (m_pGrandPrix)
			m_pGrandPrix->RefreshRank();
		return;
	case M983(0x1a0):
		if (m_pScore)
			m_pScore->UpdatePang983();
		return;
	case M983(0x132):
		if (m_pScore)
			m_pScore->UpdatePang();
		return;
	case 0x164: // 983: placar do Grand Prix (realGameType 0x14/0x15)
		if (!GRANDPRIX_ROOM983())
			return;
		if (msg.param1 == 0)
		{
			m_scoreView = 0;
			if (m_pGrandPrix)
				m_pGrandPrix->Show(false);
			if (m_pScore)
				m_pScore->SetState((FrScoreForm::eState)1);
			if (m_pTreasureGauge)
				m_pTreasureGauge->SetState((FrTreasureGauge::eState)1);
			if (m_pNextHole)
				m_pNextHole->SetState((FrNextHole::eState)1);
		}
		else if (msg.param1 == 1)
		{
			m_scoreView = 2;
			if (m_pGrandPrix)
				m_pGrandPrix->Show(true);
			if (m_pScore)
				m_pScore->SetState((FrScoreForm::eState)1);
			if (m_pTreasureGauge)
				m_pTreasureGauge->SetState((FrTreasureGauge::eState)1);
			if (m_pNextHole)
				m_pNextHole->SetState((FrNextHole::eState)1);
		}
		return;
	case M983(0x133):
		if (SDOC983(unsigned char, 0x10594) == 4 && m_pRank)
		{
			--m_pRank->m_memberCount;
			--m_pRank->m_holePlayers983[msg.param1];
		}
		return;
	case M983(0x134):
		if (SDOC983(unsigned char, 0x10594) == 4 && m_pRank)
		{
			// m_rivalList (sRivalData 0x1b8): +0x152 rank, +0x14c totalScore, +0x128 totalPang (__int64; o 983 compara hi/lo em 2 dwords)
			unsigned char myRank =
				SDOC983(sRivalData*, 0x1042c)[(unsigned int)MyIndex()].rank;
			int score = SDOC983(sRivalData*, 0x1042c)[(unsigned int)MyIndex()].totalScore +
				msg.param1;
			const sRivalData* me = &SDOC983(sRivalData*, 0x1042c)[(unsigned int)MyIndex()];
			unsigned int pang = (unsigned int)me->totalPang;
			int tie = ((const int*)&me->totalPang)[1];
			unsigned char better = 1;
			for (const sRivalData* p = SDOC983(sRivalData*, 0x1042c); p != SDOC983(sRivalData*, 0x10430); ++p)
			{
				if (p->totalScore < score ||
					(score == p->totalScore && tie <= ((const int*)&p->totalPang)[1] &&
						(tie < ((const int*)&p->totalPang)[1] ||
							pang < (unsigned int)p->totalPang)))
					++better;
			}
			int diff = (int)myRank - (int)better;
			if (diff >= 3)
			{
				m_pRank->m_pBoard->SetCaption(MakeStr(K2L_Compatibility(
					"%s\xb4\xd4 \xc0\xcc\xb9\xf8\xbf\xa1 \xc8\xa6\xc0\xce\xc7\xcf\xb8\xe9 %d\xb8\xed\xc0\xbb \xc0\xcc\xb1\xe6 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
					Doc()->m_myInfo.info.sNick, diff));
				m_pRank->m_captionPrio983 = 1;
				m_pRank->SetCaptionType(2);
			}
		}
		return;
	case M983(0x135):
		if (SDOC983(unsigned char, 0x10594) != 4)
			return;
		if (m_pRank)
		{
			m_pRank->m_pBoard->SetCaption(K2L_Compatibility(
				"\xc0\xda~ \xb8\xb6\xc1\xf6\xb8\xb7 \xc8\xa6\xc0\xd4\xb4\xcf\xb4\xd9. \xb8\xb7\xc6\xc7 \xc6\xc4\xc0\xcc\xc6\xc3!"));
			m_pRank->m_captionPrio983 = 1;
			m_pRank->SetCaptionType(1);
		}
		if (m_pRank3)
		{
			m_pRank3->m_pBoard->SetCaption(K2L_Compatibility(
				"\xc0\xda~ \xb8\xb6\xc1\xf6\xb8\xb7 \xc8\xa6\xc0\xd4\xb4\xcf\xb4\xd9. \xb8\xb7\xc6\xc7 \xc6\xc4\xc0\xcc\xc6\xc3!"));
			m_pRank3->SetCaptionType(1);
		}
		return;
	case M983(0x136):
	{
		if (SDOC983(unsigned char, 0x10594) != 4)
			return;
		FrRankForm* pRank = m_pRank;
		if (!pRank)
			return;
		++pRank->m_holePlayers983[msg.param1];
		--m_pRank->m_holePlayers983[msg.param1 - 1];
		pRank = m_pRank;
		if (pRank->m_captionPrio983 < 3)
			return;
		unsigned char curHole = *(unsigned char*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x17c)); // 983: m_currentHole em +0x17c (645: +0x168)
		if (pRank->m_leadNoticeHole983 == curHole)
			return;
		unsigned char best = 0;
		unsigned char bestHole = 0;
		for (int i = 0; i < 0x13; ++i)
		{
			if (best < pRank->m_holePlayers983[i])
			{
				bestHole = (unsigned char)(i + 1);
				best = pRank->m_holePlayers983[i];
			}
		}
		if ((int)GetHoleIndex(bestHole) < (int)(GetHoleIndex(curHole) + 5))
			return;
		pRank->m_leadNoticeHole983 = curHole;
		m_pRank->m_pBoard->SetCaption(MakeStr(K2L_Compatibility(
			"%s\xb4\xd4 \xb8\xb9\xc0\xcc \xb5\xda\xc3\xc4\xc1\xae \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. \xbc\xad\xb5\xce\xb8\xa3\xbc\xbc\xbf\xe4."),
			Doc()->m_myInfo.info.sNick));
		m_pRank->m_captionPrio983 = 2;
		m_pRank->SetCaptionType(2);
		return;
	}
	case M983(0x137):
	{
		// 983: bandeira e uids em CSharedDoc +0x812/+0x816/+0x81a (645: 0x433*4/0x437*4/0x43b*4)
		if (SDOC983(unsigned char, 0x10594) != 4)
			return;
		FrRankForm* pRank = m_pRank;
		if (!pRank)
			return;
		if (pRank->m_captionPrio983 <= 3)
			return;
		bool bFlag = ((SDOC983(unsigned int, 0x812) >> 1) & 1) != 0;
		if (pRank->m_lastUid != (bFlag ? SDOC983(unsigned long, 0x816) : SDOC983(unsigned long, 0x81a)))
			return;
		unsigned char idx = Doc()->GetIndex(bFlag == true ? SDOC983(unsigned long, 0x816) : SDOC983(unsigned long, 0x81a));
		m_pRank->m_pBoard->SetCaption(MakeStr(K2L_Compatibility(
			"%s\xb4\xd4 \xb2\xc3\xc2\xee\xbf\xb9\xbf\xe4. \xc8\xfb\xb3\xbb\xbc\xbc\xbf\xe4."),
			(unsigned int)idx * 0x1b8 + SDOC983(int, 0x1042c) + 0x8));
		m_pRank->m_captionPrio983 = 3;
		m_pRank->SetCaptionType(1);
		return;
	}
	case M983(0x138):
		if (m_pRankEx)
		{
			*(bool*)msg.param1 = m_pRankEx->IsMotionFinished();
			return;
		}
		if (SDOC983(unsigned char, 0x10594) != 6)
		{
			*(bool*)msg.param1 = true;
			return;
		}
		if (m_whiteFade < 1.0f)
		{
			*(bool*)msg.param1 = false;
			return;
		}
		*(bool*)msg.param1 = true;
		return;
	case M983(0x139):
	{
		bool* pResult = (bool*)msg.param1;
		*pResult = m_scoreView != 0;
		if (m_pRank)
		{
			WRect rect;
			m_pRank->GetClientRect(rect);
			WPoint mouse(g_input->GetMousePoint().x,
				g_input->GetMousePoint().y);
			if (rect.IsInRect(mouse))
				*pResult = true;
		}
		if (!m_pRank3)
			return;
		WRect rect;
		m_pRank3->GetClientRect(rect);
		WPoint mouse(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
		if (!rect.IsInRect(mouse))
			return;
		*pResult = true;
		return;
	}
	case M983(0x13a):
	{
		// 983 (00798a56): faixa de buracos 0x1051e..0x1051d+n, buracos em CGolfDoc+0xd8, coluna 1 da tabela;
		// na sala classica a exp sai do GetClassicExp983.
		unsigned char first = SDOC983(unsigned char, 0x1051e);
		unsigned char cnt2 = 0, cnt3 = 0, cnt4 = 0;
		unsigned char last =
			SDOCM(unsigned char, m_holeOrder, 0x1051e, SDOC983(unsigned char, 0x10699) + 0x1051d);
		if (first <= last)
		{
			unsigned char* p = (unsigned char*)(*(int*)((char*)GDOCPTR983(Doc()->m_pGolfDoc, 0xd8)) - 0x1f +
				(unsigned int)first * 0x38);
			unsigned int n = (unsigned char)((last - first) + 1);
			do
			{
				switch (*p)
				{
				case 2:
					++cnt2;
					break;
				case 3:
					++cnt3;
					break;
				case 4:
					++cnt4;
					break;
				}
				p += 0x38;
				n = n - 1;
			} while (n != 0);
		}
		unsigned char diff = GetDifficulty() - 1;
		if (!S5::CLASSICSRV::IsClassicGame983())
		{
			int row = diff * 0x13;
			int exp = s_ladderTable[7 + ((cnt4 + row) * 0xb)];
			exp += s_ladderTable[4 + ((cnt3 + row) * 0xb)];
			exp += s_ladderTable[2 + ((cnt2 + row) * 0xb)];
			*(int*)msg.param1 = exp;
		}
		else
		{
			int exp = S5::CLASSICSRV::GetClassicExp983(diff, cnt4, 4, 0);
			exp += S5::CLASSICSRV::GetClassicExp983(diff, cnt3, 3, 0);
			*(int*)msg.param1 = S5::CLASSICSRV::GetClassicExp983(diff, cnt2, 2, 0) + exp;
		}
		return;
	}
	case M983(0x13b):
		if (!m_pRank2)
			return;
		switch (msg.param1)
		{
		case 1:
			m_pRank2->SetVisible(true);
			break;
		case 0:
			m_pRank2->SetVisible(false);
			break;
		case -1:
			m_pRank2->SetVisible(!m_pRank2->IsVisible());
			break;
		}
		return;
	case 0x16e: // 983 (00798ba9); no 645 era o 0x13c
		UpdateRivals();
		return;
	case 0x170: // 983: cria o placar do Grand Prix
	{
		if (m_pGrandPrix)
			return;
		m_pGrandPrix = CreateForm<FrGrandPrixRankForm>(g_pFresh->GetManager(), this,
			"grandprix_scoreboard", NULL);
		m_pGrandPrix->EnableDrag(false);
		FrGrandPrixRankForm* pForm = m_pGrandPrix;
		float w = pForm->GetRect().w;
		float h = pForm->GetRect().h;
		WPoint pos;
		pos.x = (float)(int)(float)((g_view->GetWidth() - w) * 0.5);
		pos.y = (float)(int)(float)((g_view->GetHeight() - h) * 0.5);
		((FrFixedWnd*)pForm)->Fix();
		m_pGrandPrix->SetVisible(false);
		m_pGrandPrix->Open((FRESH_PFN_RESULT)&CScore::OnGrandPrixRankFormResult, pos, 4);
		return;
	}
	case 0x171:
		if (m_pGrandPrix)
			m_pGrandPrix->Fn007896d0_983((unsigned short)msg.param1);
		return;
	case 0x177:
		if (m_pGrandPrix)
			m_pGrandPrix->Fn00797980_983((unsigned short)msg.param1);
		return;
	case 0x172: // 983: tempo de espera do proximo buraco ("grandprix_ingame_ui.lua")
	{
		if (!m_pGrandPrix)
			return;
		m_pGrandPrix->ClearRankList();
		lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
		if (!script)
			return;
		script->RunFile("grandprix_ingame_ui.lua");
		lua_system::lua_table_helper tbl = script->GetTable("grandprix_next_hole_waiting_time_table");
		m_pGrandPrix->SetWaitTime(LUA983_GET(tbl, float, "NextHoleWaitingTime"));
		m_pGrandPrix->Fn0077bf00_983();
		m_pGrandPrix->m_bRankReady983 = false;
		return;
	}
	case 0x173:
		if (m_pGrandPrix)
		{
			m_pGrandPrix->Fn0078eb00_983();
			m_pGrandPrix->Fn0077bef0_983();
			m_pGrandPrix->Fn00782330_983(true);
		}
		return;
	case 0x174:
		if (m_pGrandPrix)
		{
			m_pGrandPrix->Fn0078eb00_983();
			m_pGrandPrix->Fn0077bf20_983();
			m_pGrandPrix->Fn00782330_983(true);
		}
		return;
	case 0x175:
	{
		bool* pResult = (bool*)msg.param1;
		if (!m_pGrandPrix)
			return;
		if (m_pGrandPrix->Fn0077bf10_983() || m_pGrandPrix->GetWaitTime() != 0.0f)
			*pResult = true;
		else
			*pResult = false;
		return;
	}
	case 0x176:
		if (m_pGrandPrix)
			m_pGrandPrix->Fn00782070_983();
		return;
	case 0x178:
		if (m_pGrandPrix)
			*(WRect*)msg.param1 = m_pGrandPrix->m_pLimitedTimeWnd983->GetRect();
		return;
	case 0x271b:
	{
		WPoint pos;
		if (m_pRank)
		{
			pos.x = g_view->GetWidth() - m_pRank->GetRect().w;
			pos.y = 40.0f;
			m_pRank->MoveWindow(pos);
		}
		if (m_pRank2)
		{
			pos.x = (g_view->GetWidth() - m_pRank2->GetRect().w) * 0.5;
			pos.y = 40.0f;
			m_pRank->MoveWindow(pos);
		}
		if (m_pRank3)
		{
			pos.x = (g_view->GetWidth() - m_pRank3->GetRect().w) + 5.0;
			pos.y = 40.0f;
			m_pRank3->MoveWindow(pos);
		}
		if (m_pRankEx)
		{
			pos.x = (float)(int)(float)((g_view->GetWidth() - m_pRankEx->GetRect().w) * 0.5);
			pos.y = (float)(int)(float)((g_view->GetHeight() - m_pRankEx->GetRect().h) * 0.5);
			m_pRankEx->MoveWindow(pos);
			m_pRankEx->ResetStage();
		}
		if (m_pScore)
		{
			pos.x = (float)(int)(float)((g_view->GetWidth() - m_pScore->GetRect().w) * 0.5);
			pos.y = 20.0f;
			m_pScore->MoveWindow(pos);
		}
		if (m_pAward)
		{
			float barY = COption::Instance()->gGetBar_Y();
			pos.x = (float)(int)(float)((g_view->GetWidth() - m_pAward->GetRect().w) * 0.5);
			pos.y = (float)(barY - 100.0);
			m_pAward->MoveWindow(pos);
		}
		if (m_pGrandPrix)
		{
			pos.x = (float)(int)(float)((g_view->GetWidth() - m_pGrandPrix->GetRect().w) * 0.5);
			pos.y = (float)(int)(float)((g_view->GetHeight() - m_pGrandPrix->GetRect().h) * 0.5);
			if (g_view->GetWidth() == 640.0 && g_view->GetHeight() == 480.0)
				pos.y = -10.0f;
			m_pGrandPrix->MoveWindow(pos);
			m_pGrandPrix->Fn0077e660_983();
			m_pGrandPrix->Fn0077ea00_983();
		}
		return;
	}
	}
}

void CScore::OnDisplay()
{
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 11:
		break;
	default:
		if (m_state == 1 && m_pAquire && m_scoreView != 1)
		{
			for (int i = 0; i < Doc()->m_pGolfDoc->m_playerNum; ++i)
				m_pAquire->DisplayPrizeList(i);
		}
	}
}

void CScore::OnCloseBtnUp()
{
	m_scoreView = 0;
}

void CScore::CheckOpenedDlg()
{
	if (Doc()->m_golfGame.gameType == 10)
	{
		if (m_pRank)
		{
			m_pRank->Close(true);
			m_pRank = NULL;
		}
		return;
	}
	m_stateTimer = 0;
	m_scoreView = 3;
	m_whiteFade = 1.0f;
	if (m_levelup.size() != 0)
	{
		for (unsigned int i = 0; i < m_levelup.size(); ++i)
		{
			if (m_levelup[i])
			{
				m_levelup[i]->Close(FrNONE, true);
				m_levelup[i] = NULL;
			}
		}
		m_levelup.clear();
	}
	if (m_pScore)
		m_pScore->SetState(FrScoreForm::STATE_HIDE);
	if (m_pTreasureGauge)
		m_pTreasureGauge->SetState((FrTreasureGauge::eState)1);
	if (m_pNextHole)
		m_pNextHole->SetState((FrNextHole::eState)1);
	if (m_pAward)
	{
		m_pAward->Close(true);
		m_pAward = NULL;
	}
	if (m_pAquire)
	{
		delete m_pAquire;
		m_pAquire = NULL;
	}
}

// 983 (0077c540): resultado do placar do Grand Prix
bool CScore::OnGrandPrixRankFormResult(int result, FrForm* pForm)
{
	if (m_pGrandPrix)
	{
		m_pGrandPrix->ClearRankList();
		m_pGrandPrix = NULL;
	}
	return true;
}

bool CScore::OnRankFormExResult(int result, FrForm* pForm)
{
	if (m_pRankEx)
	{
		m_pRankEx->Clear();
		m_pRankEx = NULL;
	}
	return true;
}

bool CScore::UpdateRivals()
{
	FrListBox* pList;
	if (m_pRank && (pList = m_pRank->m_pList) != NULL)
	{
		pList->ClearItem();
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); ++it)
			pList->AddItem(&*it);
		pList->UseRightButton(true);
		m_pRank->m_memberCount = Doc()->m_rivalList.size();
		m_pRank->m_holePlayers983[0] = Doc()->m_rivalList.size();
		m_pRank->SortRank();
	}
	if (m_pRankEx)
	{
		if (m_pRankEx->m_data.m_pRival)
			m_pRankEx->m_data.ClearRival();
		m_pRankEx->m_data.SetRivalData();
	}
	return true;
}

CScore::CScoreBg::CScoreBg()
{
	m_pBitmap = NULL;
	m_bCaptured = false;
}

CScore::CScoreBg::~CScoreBg()
{
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
	m_bCaptured = false;
	g_pFresh->manager->GetDesktop()->LockWallPaper(false);
}

void CScore::CScoreBg::SetDesktopWallPaper(bool bShow, bool bDefault)
{
	if (m_bCaptured)
	{
		g_pFresh->GetManager()->GetDesktop()->SetWallPaperFromBitmap(m_pBitmap,
			bShow);
		if (m_pBitmap)
		{
			delete m_pBitmap;
			m_pBitmap = NULL;
		}
	}
	else if (bDefault && bShow)
	{
		g_pFresh->GetManager()->GetDesktop()->SetWallPaper(
			"scoreboard_background.jpg", bShow);
	}
}

void CScore::CScoreBg::ScoreBgCommand(int command)
{
	switch (command)
	{
	case 0:
		SetDesktopWallPaper(false, true); // 983 007840f0: (0,1)
		break;
	case 1:
	case 2:
		SetDesktopWallPaper(true, true);
		break;
	case 3:
		if (Doc()->m_pGolfDoc->m_gameMode == 0x400000)
		{
			if (m_pBitmap)
			{
				delete m_pBitmap;
				m_pBitmap = NULL;
			}
			m_pBitmap = new Bitmap((int)g_view->GetWidth(),
				(int)g_view->GetHeight(), 24);
			if (g_resrcmng->VideoReference()->Command(W_VDEV_CAPTURE_SCREEN, 0,
					(int)m_pBitmap) == 1)
			{
				m_bCaptured = true;
				g_pFresh->GetManager()->GetDesktop()->LockWallPaper(true);
			}
			else
			{
				if (m_pBitmap)
				{
					delete m_pBitmap;
					m_pBitmap = NULL;
				}
				m_bCaptured = false;
				g_pFresh->GetManager()->GetDesktop()->LockWallPaper(false);
			}
		}
		break;
	}
}

bool CScore::OnGuildScoreFormResult(int result, FrForm* pForm)
{
	if (m_pGuildScore)
		m_pGuildScore = NULL;
	return true;
}
