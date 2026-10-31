// JP 983: unidade 00510a10–00517140 (FrMassTypeRoomDlg e FrGPTypeRoomDlg). Fonte reconstruída do 983: a sala de 30 jogadores
// (o `OnGameRoomExt_*`/`OnGameRoomExtRes_*` do CLobbyMain do 645) e a sala do Grand Prix. Ver include/jp/gptyperoomdlg.h.
// Sessão B69 (1º lote): mapas de mensagens, construtores, destrutores, registros, OnProc e os handlers simples; os desenhos,
// as listas e o grandprix_room_ui.lua ficam para os próximos lotes (declarados, sem corpo).
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "icontentsdoc.h"
#include "taskmanager.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frviewer.h"
#include "frbutton.h"
#include "frgroupbox.h"
#include "frcombobox.h"
#include "frlistbox.h"
#include "frgaugebar.h"
#include "frframe.h"
#include "frgraphicinterface.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "bitmap.h"
#include "titlefontex.h"
#include "scoredlg.h"
#include "netresourcemanager.h"
#include "commonutil.h"
#include "gproomtimersteward.h"
#include "lobbyjp.h"
#include "user_info.h"
#include "packet.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"
#include "gamestringmanager.h"
#include "grandprix983.h"
#include "gptyperoomdlg.h"
#include "lobbycontainer983.h"

extern Fresh* g_pFresh; // 00f02340

#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60
static const char* s_gateway = "Gateway"; // 00e09454

#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))
#define DOC983_U32(off) (*(unsigned long*)DOCPTR983(off))
// documento 0xa5 do CContentsDoc (m_pDoc): jogador escolhido (gameroomdlg.cpp)
#define ROOMDOC_SEL_OID(p) (static_cast<CLobbyContainer983*>(p)->m_selOID) // A844: era +0x0c cru
#define ROOMDOC_SEL_UID(p) (static_cast<CLobbyContainer983*>(p)->m_selUID) // A844: era +0x10 cru
static inline int float2int(float f) // no 983 sai embutida (`call` 00c3c320 = conversão do CRT), como na gameroomdlg.cpp
{
	return (int)f;
}

#define ROOMDOC_MASTER(p) (static_cast<CLobbyContainer983*>(p)->m_master) // A844: era +0x14 cru
#define ROOMDOC_B18(p) (static_cast<CLobbyContainer983*>(p)->m_b18) // A844: era +0x18 cru

// CSharedDoc+0x8cb: uid da conta; CSharedDoc+0x832: asas nos bits 5 e 6 no 983 (o cabeçalho da A ainda tem 3 e 4; pedido B47).
// Mesmas visões da gameroomdlg.cpp.
struct sInfoFlags983
{
	unsigned char low : 5;
	unsigned char angelicWings : 1;
	unsigned char angelicWingsEffect : 1;
	unsigned char high : 1;
};
#define MYFLAGS983() (*(sInfoFlags983*)DOCPTR983(0x832))
#define DOC983_MYUID() (*(unsigned long*)DOCPTR983(0x8cb))

// 008a9ae0: o `CUserInfo::SetInfo` do 983 tem dois argumentos a mais (zero aqui). PROVISÓRIO: pedido B → A (B44).
class CUserInfoJp
{
public:
	void SetInfo983(unsigned long uid, unsigned long guid, bool bCheckTime, bool bOption, bool bMsnControl,
		bool bControlOff, std::string nickname, int a8, int a9);
};

class FrWnd983
{
public:
	static void SetF1ab38_983(); // 00be7950 (o mesmo empréstimo da gameroomdlg.cpp)
};

// linha das listas "ranking"/"guildranking" = sRivalData / sGuildMatchup do CSharedDoc (shareddoc.h)
C_ASSERT(offsetof(sRivalData, oid) == 0x4 && offsetof(sRivalData, state) == 0x36);
C_ASSERT(offsetof(sGuildMatchup, uid) == 0x1);

// ------------------------------------------------------------------------------------------------------------------------
// FrMassTypeRoomDlg

IMPLEMENT_OBJECT(FrMassTypeRoomDlg, FrGameRoomDlg)

BEGIN_FRESH_MSGMAP(FrMassTypeRoomDlg, FrGameRoomDlg)

ON_FRESH_VI("invite", FRCMD_INIT, FrMassTypeRoomDlg::OnGuildInviteInit)
ON_FRESH_VV("invite", FRCMD_LBUTTONUP, FrMassTypeRoomDlg::OnGuildInviteLBtnUp)
ON_FRESH_VI("group_30s.room_event", FRCMD_OWNERDRAW, FrMassTypeRoomDlg::OnEventOwnerDraw)
ON_FRESH_VI("group_30s.user", FRCMD_INIT, FrMassTypeRoomDlg::OnRoomUserInit)
ON_FRESH_VI("group_30s.user", FRCMD_OWNERDRAW, FrMassTypeRoomDlg::OnRoomUserOwnerDraw)
ON_FRESH_VV("group_30s.user", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnUserLButtonDown)
ON_FRESH_VV("group_30s.user", FRCMD_RBUTTONUP, FrGameRoomDlg::OnUserRButtonUp)
ON_FRESH_VV("group_30s.user", FRCMD_DBLCLICK, FrGameRoomDlg::OnUserDblClick)
ON_FRESH_VI("group_30s.30_team1", FRCMD_INIT, FrMassTypeRoomDlg::OnTeamTab1Init)
ON_FRESH_VI("group_30s.30_team2", FRCMD_INIT, FrMassTypeRoomDlg::OnTeamTab2Init)
ON_FRESH_VI("group_30s.30_guild1", FRCMD_INIT, FrMassTypeRoomDlg::OnGuildTab1Init)
ON_FRESH_VI("group_30s.30_guild2", FRCMD_INIT, FrMassTypeRoomDlg::OnGuildTab2Init)
ON_FRESH_VI("group_30s.30_team_num", FRCMD_INIT, FrMassTypeRoomDlg::OnTeamNumInit)
ON_FRESH_VI("group_30s.30_team_num", FRCMD_OWNERDRAW, FrMassTypeRoomDlg::OnTeamNumOwnerDraw)
ON_FRESH_VI("group_30s.30_guild_num", FRCMD_INIT, FrMassTypeRoomDlg::OnGuildNumInit)
ON_FRESH_VI("group_30s.30_guild_num", FRCMD_OWNERDRAW, FrMassTypeRoomDlg::OnGuildNumOwnerDraw)
ON_FRESH_VI("group_30s.countdown", FRCMD_INIT, FrMassTypeRoomDlg::OnCountdownInit)
ON_FRESH_VI("group_result.user_info", FRCMD_INIT, FrMassTypeRoomDlg::OnUserInfoInit)
ON_FRESH_VV("group_result.user_info", FRCMD_LBUTTONDOWN, FrMassTypeRoomDlg::OnUserInfoLBtnUp)
ON_FRESH_VI("group_result.rankframe", FRCMD_INIT, FrMassTypeRoomDlg::OnFrameInit)
ON_FRESH_VI("group_result.score_gb", FRCMD_INIT, FrMassTypeRoomDlg::OnGuildScoreGaugeInit)
ON_FRESH_VI("group_result.rankingtab", FRCMD_INIT, FrMassTypeRoomDlg::OnRoomUserRankTabInit)
ON_FRESH_VI("group_result.ranking", FRCMD_INIT, FrMassTypeRoomDlg::OnRoomUserRankInit)
ON_FRESH_VI("group_result.ranking", FRCMD_OWNERDRAW, FrMassTypeRoomDlg::OnRoomUserRankOwnerDraw)
ON_FRESH_VV("group_result.ranking", FRCMD_LBUTTONDOWN, FrMassTypeRoomDlg::OnRoomUserRankLBtnUp)
ON_FRESH_VV("group_result.ranking", FRCMD_RBUTTONUP, FrMassTypeRoomDlg::OnRoomUserRankRBtnUp)
ON_FRESH_VI("group_result.guildranking", FRCMD_INIT, FrMassTypeRoomDlg::OnRoomGuildRankInit)
ON_FRESH_VI("group_result.guildranking", FRCMD_OWNERDRAW, FrMassTypeRoomDlg::OnRoomGuildRankOwnerDraw)
ON_FRESH_VV("group_result.guildranking", FRCMD_LBUTTONDOWN, FrMassTypeRoomDlg::OnRoomGuildRankLBtnUp)
ON_FRESH_VV("group_result.guildranking", FRCMD_RBUTTONUP, FrMassTypeRoomDlg::OnRoomGuildRankRBtnUp)
ON_FRESH_VI("group_result.score", FRCMD_INIT, FrMassTypeRoomDlg::OnScoreInit)
ON_FRESH_VV("group_result.score", FRCMD_LBUTTONDOWN, FrMassTypeRoomDlg::OnScoreBtnUp)

END_FRESH_MSGMAP()

// 00510a20
void FrMassTypeRoomDlg::ResetRoomMass983()
{
	ResetRoom983();
}

// 00510a40
void FrMassTypeRoomDlg::HideControls983()
{
	OpenNewRecordForm();
	if (m_pMap)
		m_pMap->SetVisible(false);
	if (m_pMapPrev)
		m_pMapPrev->SetVisible(false);
	if (m_pMapNext)
		m_pMapNext->SetVisible(false);
	if (m_pMapHoleCombo)
		m_pMapHoleCombo->SetVisible(false);
	if (m_pStart)
		m_pStart->SetVisible(false);
	if (m_pReady)
		m_pReady->SetVisible(false);
	if (m_pRoomOption)
		m_pRoomOption->SetVisible(false);
	if (m_pShortGame)
		m_pShortGame->SetVisible(false);
	if (m_pCaddie)
		m_pCaddie->SetVisible(false);
	if (m_pClub)
		m_pClub->SetVisible(false);
	if (m_pAztec)
		m_pAztec->SetVisible(false);
	if (m_pMascot)
		m_pMascot->SetVisible(false);
	if (m_pEquipItem)
		m_pEquipItem->SetVisible(false);
}

// 00510fd0
FrMassTypeRoomDlg::~FrMassTypeRoomDlg()
{
}

// 00511060
void FrMassTypeRoomDlg::OnTeamTab1Init(int param)
{
	m_pTeamTab[0] = DYNAMIC_CAST(FrArea, param);
}

// 005110d0
void FrMassTypeRoomDlg::OnTeamTab2Init(int param)
{
	m_pTeamTab[1] = DYNAMIC_CAST(FrArea, param);
}

// 00511140
void FrMassTypeRoomDlg::OnGuildTab1Init(int param)
{
	m_pGuildTab[0] = DYNAMIC_CAST(FrArea, param);
}

// 005111b0
void FrMassTypeRoomDlg::OnGuildTab2Init(int param)
{
	m_pGuildTab[1] = DYNAMIC_CAST(FrArea, param);
}

// 00511220
void FrMassTypeRoomDlg::OnTeamNumInit(int param)
{
	m_pTeamNum = DYNAMIC_CAST(FrArea, param);
}

// 00511290
void FrMassTypeRoomDlg::OnGuildInviteInit(int param)
{
	m_pGuildInvite = DYNAMIC_CAST(FrButton, param);
}

// 00511300
void FrMassTypeRoomDlg::OnCountdownInit(int param)
{
	m_pCountdown = DYNAMIC_CAST(FrArea, param);

	if (m_pCountdown)
	{
		m_pCountdown->SetVisible(false);
	}
}

// 00511380
void FrMassTypeRoomDlg::OnUserInfoInit(int param)
{
	m_pUserInfo = DYNAMIC_CAST(FrButton, param);

	if (m_pUserInfo)
	{
		m_pUserInfo->Enable(false);
	}
}

// 00511400
void FrMassTypeRoomDlg::OnFrameInit(int param)
{
	m_pExtFrame = DYNAMIC_CAST(FrFrame, param);
}

// 00511470 (645: OnGameRoomExtRes_RoomUserRankLBtnUp)
void FrMassTypeRoomDlg::OnRoomUserRankLBtnUp()
{
	FrListItem* item = m_pRoomUserRank->GetItemUnderCursor();

	if (!item)
		return;

	sRivalData* rival = (sRivalData*)item->pData;
	if (!rival)
		return;

	ROOMDOC_SEL_OID(m_pDoc) = rival->oid;
	ROOMDOC_SEL_UID(m_pDoc) = rival->uid;

	if (m_pScoreDlg)
	{
		m_pScoreDlg->SetPlayer(ROOMDOC_SEL_OID(m_pDoc));
		return;
	}

	if (m_pScore)
		m_pScore->Enable(true);

	if (m_pUserInfo)
		m_pUserInfo->Enable(true);
}

// 005114f0 (645: OnGameRoomExtRes_RoomGuildRankLBtnUp)
void FrMassTypeRoomDlg::OnRoomGuildRankLBtnUp()
{
	FrListItem* item = m_pRoomGuildRank->GetItemUnderCursor();

	if (!item)
		return;

	sGuildMatchup* matchup = (sGuildMatchup*)item->pData;

	if (!matchup)
		return;

	if (m_pScoreDlg)
	{
		m_pScoreDlg->SetGuildPlayer(matchup->uid[0]);
		return;
	}

	if (m_pScore)
		m_pScore->Enable(true);
}

// 00511540
void FrMassTypeRoomDlg::OnScoreInit(int param)
{
	m_pScore = DYNAMIC_CAST(FrButton, param);
	if (m_pScore)
		m_pScore->Enable(false);
}

// 00511c90: estado 8 (partida em andamento) — o título mostra o tempo até o fim (CSharedDoc+0x1057c + +0x106a0, em ms)
void FrMassTypeRoomDlg::OnProc(const float deltaTime)
{
	FrGameRoomDlg::OnProc(deltaTime);
	if (m_state == 8)
	{
		unsigned long end = DOC983_U32(0x106a0) + DOC983_U32(0x1057c);
		if (end > (unsigned long)g_CurrentTime)
		{
			unsigned long sec = (end - g_CurrentTime) / 1000;
			SetRoomTitle983(MakeStr("%02d:%02d", sec / 60, sec % 60));
			return;
		}
		SetRoomTitle983("00:00");
	}
}

// 00511d20
void FrMassTypeRoomDlg::ResetIdle983()
{
	m_bIdle = false;
	g_mouse->ResetInputTime();
	g_ime->ResetInputTime();
	m_bReady = false;
	m_bLockControls = false;
	m_bCanStart = false;

	if (Doc()->m_myInfo.info.dwIdentity == 2)
	{
		m_bReady = true;
		m_bLockControls = true;
	}

	m_bRoomStateReq = false;
	ROOMDOC_SEL_OID(m_pDoc) = 0xffffffff;
	ROOMDOC_SEL_UID(m_pDoc) = 0xffffffff;
}

// 00511d90: sala do Grand Prix — campo, título, botões, número de salas pelo "Gateway" (aviso 0x753e / 4 com &m_p49c), aviso
// "pronto" (0x4f) ao ator principal quando o horário já abriu, barras de atributo e o som do lobby
void FrMassTypeRoomDlg::SetGPRoom983()
{
	SetCourse983(Doc()->m_itemManager.HasGrandPrixSpecialHole983(Doc()->m_roomInfo.gpTicket983) ? 0x11 : Doc()->m_roomInfo.mapType);
	SetRoomTitle983(NULL);
	m_pStart->Enable(false);
	m_pStart->SetVisible(false);
	m_pReady->Enable(true);
	m_pReady->SetVisible(true);
	m_p49c = 0;
	AfxGetTask()->GetActor(s_gateway) << MsgObject(NULL, 0x753e, 4, 0x1a000264, (int)&m_p49c, 0, 0);
	if (m_pTimeBoard983->m_bLimited == true)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x4f, 0, 0, 0, 0, 0));
		EnableSettingButton(false);
	}
	SetLevelBar();
	g_audio->PlaySfx("gp_lobby_amb", NULL, 1, "gp_lobby_amb");
}

// 00511f10 (645: CLobbyMain::OnGameRoom_EventOwnerDraw; o 983 pula o modo 11 e pede +0x6a da sala)
void FrMassTypeRoomDlg::OnEventOwnerDraw(int param)
{
	if (Doc()->m_roomInfo.gameType != 11 && Doc()->m_roomInfo.bAdminMaster && Doc()->m_roomInfo.nUserLimit > 99)
	{
		FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
		if (pDevice)
		{
			if (m_pRoomEvent02Bmp == NULL)
				m_pRoomEvent02Bmp = g_pFresh->GetBitmap("room_event02");
			if (m_pRoomEvent02Bmp)
				pDevice->DrawTexture(m_pRoomEvent02Bmp, WRect(8.0f, 45.0f, 43.0f, 32.0f), 0xffffffff, false);
		}
	}
}

// 00511fc0 (645: CLobbyMain::OnGameRoomExt_TeamNumOwnerDraw; o 983 só olha o modo 1 e troca "대회" por "숏게임" no bit 2)
void FrMassTypeRoomDlg::OnTeamNumOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;

	WRect rect = m_pTeamNum->GetRect();

	pGDI->SetTextStyle(0);

	if (Doc()->m_roomInfo.gameType == GAME_TYPE_TEAM)
	{
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		g_pFresh->GetManager()->PrintText(
			WPoint(rect.x + 209.0f, rect.y + 12.0f), 0,
			MakeStr("(%d)", m_redTeam.size()), -1.0f, 0xffffffff);

		pGDI->SetTextColor(0xff0060ff, 0xffffffff);
		g_pFresh->GetManager()->PrintText(
			WPoint(rect.x + 458.0f, rect.y + 12.0f), 0,
			MakeStr("(%d)", m_blueTeam.size()), -1.0f, 0xffffffff);
	}
	else
	{
		pGDI->SetTextColor(0xffffffff, 0xffffffff);
		if (Doc()->m_roomInfo.roomFlags983 & 2)
			g_pFresh->GetManager()->PrintText(
				WPoint(rect.x + 109.0f, rect.y + 12.0f), 0, K2L_Compatibility("\xbc\xf4\xb0\xd4\xc0\xd3"),
				-1.0f, 0xffffffff);
		else
			g_pFresh->GetManager()->PrintText(
				WPoint(rect.x + 109.0f, rect.y + 12.0f), 0, K2L_Compatibility("\xb4\xeb\xc8\xb8"),
				-1.0f, 0xffffffff);

		float offset = 209.0f;
		if (Doc()->m_slotList.size() >= 100)
			offset = 200.0f;

		g_pFresh->GetManager()->PrintText(
			WPoint(offset + rect.x, rect.y + 12.0f), 0,
			MakeStr("(%d)", Doc()->m_slotList.size()), -1.0f, 0xffffffff);
	}
}

// 005121d0
void FrMassTypeRoomDlg::OnGuildNumInit(int param)
{
	m_pGuildNum = DYNAMIC_CAST(FrArea, param);
	if (DOC983_U8(0x1094c) != 6)
		m_pGuildNum->SetVisible(false);
}

// 00512250 (645: CLobbyMain::OnGameRoomExt_GuildNumOwnerDraw; o "GAMEROOM_EXTRES" virou o estado 8 da sala)
void FrMassTypeRoomDlg::OnGuildNumOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;

	WRect rect = m_pTeamNum->GetRect();
	pGDI->SetTextStyle(1);

	if (Doc()->m_roomInfo.GuildInfo.nGuildID[0])
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			Doc()->m_roomInfo.GuildInfo.nGuildID[0], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[0]);

		if (pEmblem)
			pGDI->DrawTexture(pEmblem,
				WRect(rect.x + 24.0f, rect.y + 8.0f, (float)pEmblem->Width(),
					(float)pEmblem->Height()),
				0xffffffff, 0);
	}

	pGDI->SetTextColor(0xffff0000, 0xffffffff);
	g_pFresh->GetManager()->PrintText(WPoint(rect.x + 60.0f, rect.y + 12.0f), 0,
		Doc()->m_roomInfo.GuildInfo.szName[0], -1.0f, 0xffffffff);
	g_pFresh->GetManager()->PrintText(WPoint(rect.x + 209.0f, rect.y + 12.0f),
		0, MakeStr("(%d)", m_redTeam.size()), -1.0f, 0xffffffff);

	if (Doc()->m_roomInfo.GuildInfo.nGuildID[1])
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			Doc()->m_roomInfo.GuildInfo.nGuildID[1], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[1]);

		if (pEmblem)
			pGDI->DrawTexture(pEmblem,
				WRect(rect.x + 273.0f, rect.y + 8.0f, (float)pEmblem->Width(),
					(float)pEmblem->Height()),
				0xffffffff, 0);
	}

	pGDI->SetTextColor(0xff0060ff, 0xffffffff);
	g_pFresh->GetManager()->PrintText(WPoint(rect.x + 309.0f, rect.y + 12.0f),
		0, Doc()->m_roomInfo.GuildInfo.szName[1], -1.0f, 0xffffffff);
	g_pFresh->GetManager()->PrintText(WPoint(rect.x + 458.0f, rect.y + 12.0f),
		0, MakeStr("(%d)", m_blueTeam.size()), -1.0f, 0xffffffff);

	if (m_state == 8)
	{
		g_pFresh->GetManager()->PrintText(
			WPoint(rect.x + 180.0f, rect.y + 12.0f), 0,
			MakeStr("%d", Doc()->m_guildScore[0]), -1.0f, 0xffffffff);
		g_pFresh->GetManager()->PrintText(
			WPoint(rect.x + 429.0f, rect.y + 12.0f), 0,
			MakeStr("%d", Doc()->m_guildScore[1]), -1.0f, 0xffffffff);
	}
}

// 005125a0
void FrMassTypeRoomDlg::OnGuildScoreGaugeInit(int param)
{
	m_pGuildScoreGauge = DYNAMIC_CAST(FrGaugeBar, param);

	m_pGuildScoreGauge->SetVisible(false);
}

// 00512610
void FrMassTypeRoomDlg::OnRoomUserRankTabInit(int param)
{
	m_pRoomUserRankTab = DYNAMIC_CAST(FrArea, param);
	if (m_pRoomUserRankTab)
	{
		if (DOC983_U8(0x1094c) == 10)
		{
			((FrArea*)m_pRoomUserRankTab)->SetBgImg("approach_tab");
		}
	}
}

// 005160a0 (645: o miolo do CLobbyMain::OnGameRoomExt_Finish): o meu boneco no 1º lugar, campo e título
void FrMassTypeRoomDlg::InitMySlot983()
{
	if (m_b18d)
	{
		sRoomSlot slot;
		slot.pArea = m_pPet[0];
		slot.pExhibition = NULL;
		slot.dwUserUID = 0;
		slot.partTidList = Doc()->GetMyPartTidList();
		slot.charInfo = Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		slot.bAngelWing = MYFLAGS983().angelicWings;
		slot.bGachaWing = MYFLAGS983().angelicWingsEffect;
		slot.dwUserUID = DOC983_MYUID();
		Doc()->m_roomSlotMap[MyGuid(false)] = slot;

		ShowPet(MyGuid(false));
	}

	SetCourse983(Doc()->m_roomInfo.mapType);
	SetRoomTitle983(NULL);
}

// 005162c0 (645: CLobbyMain::OnGameRoomExt_Init/OnGameRoomExtRes_Init): grupo de 30, resultado no estado 8
bool FrMassTypeRoomDlg::OnInit()
{
	if (m_3e0 == 0)
		m_pGroup30s->SetVisible(true);

	switch (m_state)
	{
	case 4:
	case 5:
		InitMySlot983();
		goto open_bg;
	case 8:
		m_pGroupResult->SetVisible(true);
		HideControls983();
	open_bg:
		if (m_pBg)
			m_pBg->Open("gameroom_background.tga");
		break;
	}

	ROOMDOC_B18(m_pDoc) = false;
	if (!m_pTreasureGift)
		m_resultDelay = 1.0f;
	if (m_pBackGround)
		m_pBackGround->SetVisible(false);
	FrWnd983::SetF1ab38_983();
	if (CLobbyState983::Instance983())
		AfxPostMsg(NULL, "Lobby", 0x59, 10, 0, 0, 0);
	return FrForm::OnInit();
}

// 00512ee0
FrMassTypeRoomDlg::FrMassTypeRoomDlg()
{
	m_pGroup30s = NULL;
	m_pGroupGp = NULL;
	if (m_3e0 == 0)
	{
		if (DOCJP()->GetState983() == 8)
		{
			ResetIdle983();
			return;
		}
		ResetRoomMass983();
	}
}

// 00513270 (645: CLobbyMain::OnGameRoomExt_RoomUserOwnerDraw). No 983: só a guerra de guildas tem as cores de equipe,
// sem a escada (ladder_%03d), e o anfitrião do Grand Prix (bit 0x8000) troca o nível por "gp_host"
void FrMassTypeRoomDlg::OnRoomUserOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	sSlotInfo* pSlot = NULL;
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	pSlot = (sSlotInfo*)pItem->pData;

	if (IsLocalContent(S4_INVITE_FRIEND))
	{
		if (pSlot)
		{
			if (pSlot->IsInvite)
			{
				pDevice->DrawTexture(m_pInviteBmp,
					WRect(pItem->pos.x, pItem->pos.y,
						(float)m_pInviteBmp->Width(),
						(float)m_pInviteBmp->Height()),
					0xffffffff, 0);
				return;
			}
		}
	}

	pDevice->DrawTexture(m_pPlayerBaseBmp[0],
		WRect(pItem->pos.x, pItem->pos.y, (float)m_pPlayerBaseBmp[1]->Width(),
			(float)m_pPlayerBaseBmp[1]->Height()),
		0xffffffff, 0);

	if (pItem->selected)
		pDevice->Box(WRect(pItem->pos.x, pItem->pos.y,
						 (float)m_pUser->GetItemWidth() + 15.0f,
						 (float)m_pUser->GetItemHeight()),
			0x7f265785, 0, 0.0f);

	if (!pSlot)
		return;

	WRect rect(0.0f, 0.0f, 0.0f, 0.0f);

	int index;
	if (!pSlot->bReady && !pSlot->bMaster)
	{
		index = 0;
	}
	else
	{
		int n = (pSlot->bMaster) ? 1 : 2;
		index = n;

		if (Doc()->m_roomInfo.gameType == GAME_TYPE_GUILD_MATCH)
		{
			if (!pSlot->bTeam)
				index += 2;
			else
				index += 4;
		}
	}

	const Bitmap* pIcon = m_pSituBmp[index];
	rect = WRect(pItem->pos.x + 10.0f, pItem->pos.y + 5.0f,
		(float)pIcon->Width(), (float)pIcon->Height());
	pDevice->DrawTexture(pIcon, rect, 0xffffffff, 0);

	if (pSlot->angelicWings)
		pIcon = m_pGenderBmp[8 + pSlot->gender % 2];
	else if (pSlot->manner)
		pIcon = m_pGenderBmp[6 + pSlot->gender % 2];
	else
		pIcon = m_pGenderBmp[pSlot->gender];

	rect = WRect(pItem->pos.x + 30.0f, pItem->pos.y, (float)pIcon->Width(),
		(float)pIcon->Height());
	pDevice->DrawTexture(pIcon, rect, 0xffffffff, 0);

	if (m_pDiveBmp && pSlot->bSleep)
		pDevice->DrawTexture(m_pDiveBmp,
			WRect(pItem->pos.x + 34.0f, pItem->pos.y + 2.0f,
				(float)m_pDiveBmp->Width(), (float)m_pDiveBmp->Height()),
			0xffffffff, 0);

	const Bitmap* pLevel;
	if (pSlot->dwTitle)
	{
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(pSlot->dwTitle);
		if (pSkin)
			pLevel = g_pFresh->GetBitmap(pSkin->c.Icon);
		else
			pLevel =
				g_pFresh->GetBitmap(MakeStr("level_%03d", pSlot->level + 1));
	}
	else
	{
		pLevel = g_pFresh->GetBitmap(MakeStr("level_%03d", pSlot->level + 1));
	}
	if (pSlot->dwIdentity & 0x8000)
		pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");
	if (pLevel)
	{
		pDevice->DrawTexture(pLevel,
			WRect(pItem->pos.x + 80.0f - float2int(pLevel->Width() * 0.5f),
				pItem->pos.y + 13.0f - float2int(pLevel->Height() * 0.5f),
				(float)pLevel->Width(), (float)pLevel->Height()),
			0xffffffff, 0);
	}

	if (pSlot->GuildId && !pSlot->IsIdentity(0x14))
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			pSlot->GuildId, pSlot->dwEmblemVer983);

		if (pEmblem)
			pDevice->DrawTexture(pEmblem,
				WRect(pItem->pos.x + 115.0f, pItem->pos.y + 7.0f - 6.0f,
					(float)pEmblem->Width(), (float)pEmblem->Height()),
				0xffffffff, 0);
	}

	unsigned long color =
		(unsigned char)(pSlot->dwIdentity & 0x14) ? 0x50ffffff : 0xffffffff;

	if (pItem->underCursor)
	{
		pDevice->SetTextColor(0xffffffff, 0xff124371);
		pDevice->SetTextStyle(2);
	}
	else
	{
		pDevice->SetTextColor(0xffffffff, 0xffffffff);
		pDevice->SetTextStyle(0);
	}

	if (!CProjectG::Instance()->HidePrivacy())
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 140.0f, pItem->pos.y + 7.0f), 0, pSlot->sNick,
			-1.0f, color);

	if ((!Doc()->m_bGameOver ||
			(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1)) &&
		MyGuid(false) == ROOMDOC_MASTER(m_pDoc) && !pSlot->bMaster &&
		Doc()->m_roomInfo.gameType != GAME_TYPE_GUILD_MATCH)
		pDevice->DrawTexture(m_pKickBmp,
			WRect(pItem->pos.x + 226.0f, pItem->pos.y + 7.0f - 2.0f,
				(float)m_pKickBmp->Width(), (float)m_pKickBmp->Height()),
			pItem->underCursor ? 0xffffffff : 0x80ffffff, 0);
}

bool __fastcall RankUserCompare(const void* a, const void* b); // 0059f440 (unidade vizinha, não escrita)
bool __fastcall ApproachRankUserCompare(const void* a, const void* b); // 0059f3e0 (idem)

// 00513910 (645: CLobbyMain::OnGameRoomExtRes_RoomUserRankInit; os nomes das imagens das equipes mudaram)
void FrMassTypeRoomDlg::OnRoomUserRankInit(int param)
{
	m_pRoomUserRank = DYNAMIC_CAST(FrListBox, param);
	if (!m_pRoomUserRank)
		return;

	if (Doc()->m_golfGame.gameType == GAME_TYPE_GUILD_MATCH)
	{
		m_pRoomUserRank->SetVisible(false);
		return;
	}

	m_pRoomUserRank->UseRightButton(true);

	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); it++)
		m_pRoomUserRank->AddItem(&(*it));

	if (m_pRoomUserRank->GetScrollBar())
		m_pRoomUserRank->GetScrollBar()->SetGuideVisible(true);

	if (Doc()->m_golfGame.gameType == GAME_TYPE_NEW_APPROACH)
		m_pRoomUserRank->SortItem(ApproachRankUserCompare);
	else
		m_pRoomUserRank->SortItem(RankUserCompare);

	m_pTeamRankBmp[0] = g_pFresh->RegisterBitmap("30in_team_redteam_rank");
	m_pTeamRankBmp[1] = g_pFresh->RegisterBitmap("30in_team_blueteam_rank");
	m_pRankBgBmp = g_pFresh->RegisterBitmap("30in_rank");

	m_pRankBmp[0] = g_pFresh->RegisterBitmap("rank_gold");
	m_pRankBmp[1] = g_pFresh->RegisterBitmap("rank_silver");
	m_pRankBmp[2] = g_pFresh->RegisterBitmap("rank_bronze");
	m_pRankBmp[3] = g_pFresh->RegisterBitmap("rank_noaward");

	m_rankViewKind = 0;
}

unsigned char __fastcall GetHoleIndex(unsigned char hole); // 0046f4f0
int __fastcall GetMedalIndex983(unsigned long oid); // 0059fc40 (645: CLobbyMain::GetMedalIndex; no 983 é livre, unidade vizinha)

// 00513ac0 (645: CLobbyMain::OnGameRoomExtRes_RoomUserRankOwnerDraw). No 983: sem o ramo das equipes de 30; a medalha só na
// sala de evento (+0x6a) de 100 ou mais, fora dos modos 10 e 11; "%d홀" passa pelo K2L
void FrMassTypeRoomDlg::OnRoomUserRankOwnerDraw(int param)
{
	if (param == NULL)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	WRect rect(pItem->pos.x, pItem->pos.y,
		(float)m_pRoomUserRank->GetItemWidth(),
		(float)m_pRoomUserRank->GetItemHeight());

	if (!pGDI)
		return;

	sRivalData* pRival = (sRivalData*)pItem->pData;

	if (ROOMDOC_SEL_OID(m_pDoc) == pRival->oid)
		pGDI->Box(WRect(rect.x, rect.y + 2.0f, rect.w + 20.0f, rect.h - 2.0f),
			0x999b9dff, 0, 0.0f);

	int space = pGDI->SetSpace(0);

	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xffffffff, 0xffffffff);

	if (m_pRankBgBmp)
		pGDI->DrawTexture(m_pRankBgBmp,
			WRect(rect.x + 5.0f, rect.y + 7.0f - 1.0f,
				(float)m_pRankBgBmp->Width(),
				(float)m_pRankBgBmp->Height()),
			0xffffffff, 0);

	if (pRival->state == 3)
		pGDI->Print(WPoint(pItem->pos.x + 12.0f, pItem->pos.y + 7.0f), 1,
			"-");
	else if (Doc()->m_roomInfo.gameType == GAME_TYPE_NEW_APPROACH &&
		pRival->approachResultDistance == -1)
		pGDI->Print(WPoint(pItem->pos.x + 12.0f, pItem->pos.y + 7.0f), 1,
			"-");
	else
		pGDI->Print(WPoint(pItem->pos.x + 12.0f, pItem->pos.y + 7.0f), 1,
			"%d", pRival->rank);

	if (Doc()->m_roomInfo.gameType != GAME_TYPE_NEW_APPROACH && Doc()->m_roomInfo.gameType != 11 &&
		Doc()->m_roomInfo.bAdminMaster && Doc()->m_roomInfo.nUserLimit > 99 &&
		pRival->state != 3 && pRival->hole != Doc()->m_holeOrder[0])
	{
		int medal = GetMedalIndex983(pRival->oid);

		if (medal >= 0 && medal <= 2)
		{
			if (pRival->flag > 3)
				medal = 3;

			pGDI->DrawTexture(m_pRankBmp[medal],
				WRect(rect.x - 5.0f, rect.y + 7.0f - 2.0f,
					(float)m_pRankBmp[medal]->Width(),
					(float)m_pRankBmp[medal]->Height()),
				0xffffffff, 0);
		}
	}

	if (pRival->guildUID)
	{
		const Bitmap* pEmblem =
			NetResourceManager::Instance()->GetEmblem983(pRival->guildUID, pRival->guildEmblemVer983);

		if (pEmblem)
		{
			pGDI->DrawTexture(pEmblem,
				WRect(pItem->pos.x + 24.0f, pItem->pos.y + 1.0f,
					(float)pEmblem->Width(), (float)pEmblem->Height()),
				0xffffffff, 0);
		}
	}

	if (pRival->oid == MyGuid(true))
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
	else if (pRival->state == 3)
		pGDI->SetTextColor(0xff808080, 0xffffffff);
	else
		pGDI->SetTextColor(0xff000000, 0xffffffff);

	if (CProjectG::Instance()->HidePrivacy() == false)
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 47.0f, pItem->pos.y + 7.0f), 0,
			pRival->nickname, 80.0f, 0xffffffff);

	if (Doc()->m_roomInfo.gameType == GAME_TYPE_NEW_APPROACH)
	{
		if (pRival->approachTime == -1)
		{
			pGDI->Print(WPoint(pItem->pos.x + 142.0f, pItem->pos.y + 7.0f), 2,
				"Out");
			pGDI->Print(WPoint(pItem->pos.x + 188.0f, pItem->pos.y + 7.0f), 2,
				"-");
		}
		else
		{
			if (pRival->approachResultDistance == -1)
				pGDI->Print(WPoint(pItem->pos.x + 142.0f, pItem->pos.y + 7.0f),
					2, "Out");
			else
				pGDI->Print(WPoint(pItem->pos.x + 142.0f, pItem->pos.y + 7.0f),
					2, "%d.%dy", pRival->approachResultDistance / 10,
					pRival->approachResultDistance % 10);

			pGDI->Print(WPoint(pItem->pos.x + 207.0f, pItem->pos.y + 7.0f), 2,
				"%d.%03d", pRival->approachTime / 1000,
				pRival->approachTime % 1000);
		}
	}
	else
	{
		pGDI->Print(WPoint(pItem->pos.x + 147.0f, pItem->pos.y + 7.0f), 2, "%d",
			pRival->totalScore);
		pGDI->Print(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 7.0f), 2,
			"%I64d", pRival->totalPang);

		if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
			pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 7.0f), 2,
				"End");
		else if (pRival->state == 3)
			pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 7.0f), 2,
				"Out");
		else
			pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 7.0f), 2,
				K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pRival->hole));
	}

	pGDI->SetSpace(space);
}

// 005140c0 (645: CLobbyMain::OnGameRoomExtRes_RoomGuildRankInit)
void FrMassTypeRoomDlg::OnRoomGuildRankInit(int param)
{
	m_pRoomGuildRank = DYNAMIC_CAST(FrListBox, param);
	if (!m_pRoomGuildRank)
		return;

	if (Doc()->m_golfGame.gameType != GAME_TYPE_GUILD_MATCH)
	{
		m_pRoomGuildRank->SetVisible(false);
		return;
	}

	m_pRoomGuildRank->UseRightButton(true);

	for (std::vector<sGuildMatchup>::iterator it = Doc()->m_guildMatchupList.begin();
		it != Doc()->m_guildMatchupList.end(); ++it)
		m_pRoomGuildRank->AddItem(&*it);

	if (m_pRoomGuildRank->GetScrollBar())
		m_pRoomGuildRank->GetScrollBar()->SetGuideVisible(true);

	m_pTeamRankBmp[0] = NetResourceManager::Instance()->GetEmblem983(
		Doc()->m_roomInfo.GuildInfo.nGuildID[0], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[0]);
	m_pTeamRankBmp[1] = NetResourceManager::Instance()->GetEmblem983(
		Doc()->m_roomInfo.GuildInfo.nGuildID[1], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[1]);

	m_pRankBgBmp = g_pFresh->RegisterBitmap("30in_rank_02");
}

// 005169b0 (645: CLobbyMain::OnGameRoomExtRes_RoomGuildRankOwnerDraw; "%d 조" pelo K2L)
void FrMassTypeRoomDlg::OnRoomGuildRankOwnerDraw(int param)
{
	if (param == NULL)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	WRect rect(pItem->pos.x + 2.0f, pItem->pos.y,
		(float)m_pRoomGuildRank->GetItemWidth() - 10.0f,
		(float)m_pRoomGuildRank->GetItemHeight());

	if (!pGDI)
		return;

	int space = pGDI->SetSpace(0);

	sGuildMatchup* pMatchup = (sGuildMatchup*)pItem->pData;

	if (ROOMDOC_SEL_OID(m_pDoc) == pMatchup->uid[0] || ROOMDOC_SEL_OID(m_pDoc) == pMatchup->uid[1])
		pGDI->Box(WRect(rect.x, rect.y + 2.0f, rect.w + 20.0f, rect.h - 2.0f),
			0x999b9dff, 0, 0.0f);

	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xffffffff, 0xffffffff);

	if (m_pRankBgBmp)
	{
		pGDI->DrawTexture(m_pRankBgBmp,
			WRect(rect.x - 3.0f, rect.y + 6.0f, (float)m_pRankBgBmp->Width(),
				(float)m_pRankBgBmp->Height()),
			0xffffffff, 0);
		pGDI->Print(WPoint(pItem->pos.x + 13.0f, pItem->pos.y + 10.0f), 1,
			K2L_Compatibility("%d \xc1\xb6"), pMatchup->team);
	}

	sRivalData* pRival;

	pRival = &Doc()->m_rivalList[Doc()->GetIndex(pMatchup->uid[0])];

	if (m_pTeamRankBmp[0])
		pGDI->DrawTexture(m_pTeamRankBmp[0],
			WRect(pItem->pos.x + 34.0f, pItem->pos.y + 3.0f,
				(float)(m_pTeamRankBmp[0]->Width() / 2),
				(float)(m_pTeamRankBmp[0]->Height() / 2)),
			0xffffffff, 0);

	if (pRival->oid == MyGuid(true))
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
	else if (pRival->state == 3)
		pGDI->SetTextColor(0xff808080, 0xffffffff);
	else
		pGDI->SetTextColor(0xff000000, 0xffffffff);

	if (CProjectG::Instance()->HidePrivacy() == false)
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 47.0f, pItem->pos.y + 3.0f), 0,
			pRival->nickname, 80.0f, 0xffffffff);

	pGDI->Print(WPoint(pItem->pos.x + 147.0f, pItem->pos.y + 3.0f), 2, "%d",
		pRival->guildPoint);
	pGDI->Print(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 3.0f), 2, "%I64d",
		pRival->totalPang);

	if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
		pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 3.0f), 2,
			"End");
	else if (pRival->state == 3)
		pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 3.0f), 2,
			"Out");
	else
		pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 3.0f), 2,
			K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pRival->hole));

	pRival = &Doc()->m_rivalList[Doc()->GetIndex(pMatchup->uid[1])];

	if (m_pTeamRankBmp[1])
		pGDI->DrawTexture(m_pTeamRankBmp[1],
			WRect(pItem->pos.x + 34.0f, pItem->pos.y + 21.0f,
				(float)(m_pTeamRankBmp[1]->Width() / 2),
				(float)(m_pTeamRankBmp[1]->Height() / 2)),
			0xffffffff, 0);

	if (pRival->oid == MyGuid(true))
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
	else if (pRival->state == 3)
		pGDI->SetTextColor(0xff808080, 0xffffffff);
	else
		pGDI->SetTextColor(0xff000000, 0xffffffff);

	if (CProjectG::Instance()->HidePrivacy() == false)
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 47.0f, pItem->pos.y + 20.0f), 0,
			pRival->nickname, 80.0f, 0xffffffff);

	pGDI->Print(WPoint(pItem->pos.x + 147.0f, pItem->pos.y + 20.0f), 2, "%d",
		pRival->guildPoint);
	pGDI->Print(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 20.0f), 2, "%I64d",
		pRival->totalPang);

	if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
		pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 20.0f), 2,
			"End");
	else if (pRival->state == 3)
		pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 20.0f), 2,
			"Out");
	else
		pGDI->Print(WPoint(pItem->pos.x + 220.0f, pItem->pos.y + 20.0f), 2,
			K2L_Compatibility("%d\xc8\xa6"), GetHoleIndex(pRival->hole));

	pGDI->SetSpace(space);
}


// 00514200 (645: CLobbyMain::OnGameRoom_ScoreBtnUp)
void FrMassTypeRoomDlg::OnScoreBtnUp()
{
	if (m_pScoreDlg)
		return;

	if (Doc()->m_golfGame.gameType == GAME_TYPE_GUILD_MATCH)
	{
		m_pScoreDlg = CreateForm<FrScoreDlg>(g_pFresh->GetManager(), this, "score_guild", NULL);
		m_pScoreDlg->Open((FRESH_PFN_RESULT)&FrMassTypeRoomDlg::OnDlg160Result983, 1);
		m_pScoreDlg->SetGuildPlayer(ROOMDOC_SEL_OID(m_pDoc));
		m_pScoreDlg->SetVisibleCloseBtn(false);
	}
	else
	{
		m_pScoreDlg = CreateForm<FrScoreDlg>(g_pFresh->GetManager(), this, "score", NULL);
		m_pScoreDlg->Open((FRESH_PFN_RESULT)&FrMassTypeRoomDlg::OnDlg160Result983, 1);

		if (ROOMDOC_SEL_OID(m_pDoc) == (unsigned long)-1)
		{
			ROOMDOC_SEL_OID(m_pDoc) = MyGuid(false);
			ROOMDOC_SEL_UID(m_pDoc) = DOC983_MYUID();
		}

		m_pScoreDlg->SetPlayer(ROOMDOC_SEL_OID(m_pDoc));
		m_pScoreDlg->SetVisibleCloseBtn(true);
	}
}

// 005153d0 (645: CLobbyMain::OnGameRoomExtRes_RoomGuildRankRBtnUp)
void FrMassTypeRoomDlg::OnRoomGuildRankRBtnUp()
{
	OnRoomGuildRankLBtnUp();
	OnScoreBtnUp();
}

// 00515810 (645: CLobbyMain::OnGameRoomExt_UserInfoLBtnUp)
void FrMassTypeRoomDlg::OnUserInfoLBtnUp()
{
	if (m_rankViewKind == 0)
	{
		if (ROOMDOC_SEL_OID(m_pDoc) == (unsigned long)-1)
		{
			if (m_pUserInfo)
				m_pUserInfo->Enable(false);
			return;
		}

		if (CUserInfo::IsInstantiated())
		{
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				Doc()->m_briefUserInfoMap.find(ROOMDOC_SEL_OID(m_pDoc));
			if (it != Doc()->m_briefUserInfoMap.end())
			{
				((CUserInfoJp*)CUserInfo::Instance())->SetInfo983(it->second.dwUid, ROOMDOC_SEL_OID(m_pDoc), true,
					true, false, false, it->second.sNick, 0, 0);
			}
		}
	}
}

// 005158c0 (645: CLobbyMain::OnGameRoomExtRes_RoomUserRankRBtnUp)
void FrMassTypeRoomDlg::OnRoomUserRankRBtnUp()
{
	FrListItem* item = m_pRoomUserRank->GetItemUnderCursor();

	if (!item)
		return;

	sRivalData* rival = (sRivalData*)item->pData;

	if (!rival)
		return;

	ROOMDOC_SEL_OID(m_pDoc) = rival->oid;
	ROOMDOC_SEL_UID(m_pDoc) = rival->uid;

	if (m_pScoreDlg)
		m_pScoreDlg->SetPlayer(ROOMDOC_SEL_OID(m_pDoc));
	else
	{
		if (m_pScore)
			m_pScore->Enable(true);
	}

	if (CUserInfo::IsInstantiated() && rival->state != 3)
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(rival->oid);
		if (it != Doc()->m_briefUserInfoMap.end())
			it->second.roomIndex = Doc()->m_roomInfo.roomGuid;

		((CUserInfoJp*)CUserInfo::Instance())->SetInfo983(it->second.dwUid, rival->oid, true, true,
			false, false, it->second.sNick, 0, 0);
	}
}

// 00515090 (645: CLobbyMain::OnGameRoomExt_RoomUserInit). Novo no 983: com mais de 9 "티키리포트" (0x1a000042) não
// conferidos o ator principal recebe o aviso 0x23 com o texto
void FrMassTypeRoomDlg::OnRoomUserInit(int param)
{
	m_pUser = DYNAMIC_CAST(FrListBox, param);
	if (m_pUser)
	{
		m_pUser->UseRightButton(true);
		m_pUser->UseDummy(true);

		m_pInviteBmp = (Bitmap*)m_pUser->GetBitmap("30_player_wait");

		m_pGenderBmp[0] = (Bitmap*)m_pUser->GetBitmap("i_male");
		m_pGenderBmp[1] = (Bitmap*)m_pUser->GetBitmap("i_female");
		m_pGenderBmp[2] = (Bitmap*)m_pUser->GetBitmap("i_male_02");
		m_pGenderBmp[3] = (Bitmap*)m_pUser->GetBitmap("i_female_02");
		m_pGenderBmp[4] = (Bitmap*)m_pUser->GetBitmap("i_male_03");
		m_pGenderBmp[5] = (Bitmap*)m_pUser->GetBitmap("i_female_03");
		m_pGenderBmp[6] = (Bitmap*)m_pUser->GetBitmap("i_male_manner");
		m_pGenderBmp[7] = (Bitmap*)m_pUser->GetBitmap("i_female_manner");
		m_pGenderBmp[8] = (Bitmap*)m_pUser->GetBitmap("i_male_angel");
		m_pGenderBmp[9] = (Bitmap*)m_pUser->GetBitmap("i_female_angel");

		m_pSituBmp[0] = (Bitmap*)m_pUser->GetBitmap("30in_situ03");
		m_pSituBmp[1] = (Bitmap*)m_pUser->GetBitmap("30in_situ01");
		m_pSituBmp[2] = (Bitmap*)m_pUser->GetBitmap("30in_situ02");
		m_pSituBmp[3] = (Bitmap*)m_pUser->GetBitmap("30in_situ_red_cp");
		m_pSituBmp[4] = (Bitmap*)m_pUser->GetBitmap("30in_situ_red");
		m_pSituBmp[5] = (Bitmap*)m_pUser->GetBitmap("30in_situ_blue_cp");
		m_pSituBmp[6] = (Bitmap*)m_pUser->GetBitmap("30in_situ_blue");
		m_pDiveBmp = (Bitmap*)m_pUser->GetBitmap("dive");

		m_pPlayerBaseBmp[0] = (Bitmap*)g_pFresh->RegisterBitmap("30_noplayer_base");
		m_pPlayerBaseBmp[1] = (Bitmap*)g_pFresh->RegisterBitmap("30_player_base");

		m_pKickBmp = (Bitmap*)g_pFresh->RegisterBitmap("kick");

		ROOMDOC_SEL_OID(m_pDoc) = (unsigned long)-1;
		ROOMDOC_SEL_UID(m_pDoc) = (unsigned long)-1;

		if (m_b18d)
			MakeRoomUserList();

		unsigned int count = 0;
		for (std::list<sItemInfo>::const_iterator it = Doc()->GetConstMyItemList().begin();
			it != Doc()->GetConstMyItemList().end(); ++it)
		{
			sItemInfo info = *it;
			if (info.tid == 0x1a000042)
				count++;
		}
		if (count >= 10)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
				(int)K2L_Compatibility("\xb9\xcc\xc8\xae\xc0\xce \xc6\xbc\xc5\xb0\xb8\xae\xc6\xf7\xc6\xae\xc0\xc7 \xba\xb8\xc0\xaf\xb7\xae\xc0\xcc 10\xb0\xb3\xb8\xa6 \xc3\xca\xb0\xfa \xc7\xcf\xbf\xb4\xc0\xb8\xb9\xc7\xb7\xce \xb4\xf5\xc0\xcc\xbb\xf3 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9 \xc6\xbc\xc5\xb0\xb8\xae\xc6\xf7\xc6\xae\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc3\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad\xb4\xc2 \xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xb8\xae\xc6\xf7\xc6\xae\xb8\xa6 \xc8\xae\xc0\xce\xc7\xcf\xbf\xa9 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."), 0, 0, 0, 0));
	}
}

// 00516390 (645: CLobbyMain::OnGameRoomExt_GuildInviteLBtnUp; o 983 tirou o m_guildInviteDelay e o "notify" de sucesso
// abre com 0x800)
void FrMassTypeRoomDlg::OnGuildInviteLBtnUp()
{
	if (Doc()->m_roomInfo.gameType == GAME_TYPE_GUILD_MATCH)
	{
		std::map<unsigned long, unsigned int> guildCount;
		bool bFound = false;

		std::map<unsigned long, sBriefUserInfo>::iterator it;
		for (it = Doc()->m_briefUserInfoMap.begin();
			it != Doc()->m_briefUserInfoMap.end(); ++it)
		{
			if (it->second.m_GuildId &&
				Doc()->m_myInfo.info.dwGuildId != it->second.m_GuildId &&
				!it->second.bSleep && it->second.roomIndex == 0xffff)
			{
				guildCount[it->second.m_GuildId]++;
			}
		}

		std::vector<unsigned long> guildList;

		for (std::map<unsigned long, unsigned int>::iterator itCount =
				 guildCount.begin();
			itCount != guildCount.end(); ++itCount)
		{
			if (Doc()->m_roomInfo.nUserNum <= itCount->second)
			{
				guildList.push_back(itCount->first);
				bFound = true;
			}
		}

		if (!bFound)
		{
			m_pNotifyDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			m_pNotifyDlg->SetMessage(
				MakeStr(
					K2L_Compatibility("\xc3\xca\xb4\xeb\xc7\xd2 \xbb\xf3\xb4\xeb \xb1\xe6\xb5\xe5\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					it->second.sNick),
				false);
			m_pNotifyDlg->Open(NULL, 0);

			FrButton* pCancel =
				DYNAMIC_CAST(FrButton, m_pNotifyDlg->FindChildByName("cancel"));
			return;
		}

		{
			unsigned int count = guildList.size();

			for (it = Doc()->m_briefUserInfoMap.begin();
				it != Doc()->m_briefUserInfoMap.end(); ++it)
			{
				for (unsigned int i = 0; i < count; i++)
				{
					if (it->second.m_GuildId == guildList[i])
					{
						if (IsLocalContent(S4_INVITE_FRIEND))
						{
							WSendPacket packet((enumClientPacket)0xba);
							packet.EncodeStr(it->second.sNick);
							packet.Encode4(it->second.dwUid);
							packet.Send((eSendTo)0);
						}
						else
						{
							WSendPacket packet((enumClientPacket)0x29);
							packet.Encode4(it->second.dwUid);
							packet.Send((eSendTo)0);
						}
					}
				}
			}

			m_pNotifyDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			m_pNotifyDlg->SetMessage(
				MakeStr(
					K2L_Compatibility("\xc3\xca\xb4\xeb \xb8\xde\xbd\xc3\xc1\xf6\xb8\xa6 \xba\xb8\xb3\xc2\xbd\xc0\xb4\xcf\xb4\xd9"),
					it->second.sNick),
				false);
			m_pNotifyDlg->Open(NULL, 0x800);

			FrButton* pCancel =
				DYNAMIC_CAST(FrButton, m_pNotifyDlg->FindChildByName("cancel"));
			if (pCancel)
				pCancel->SetVisible(false);
		}
	}
	else
	{
		OnInvite();
	}
}

// ------------------------------------------------------------------------------------------------------------------------
// FrGPTypeRoomDlg

// leitura por nome fora de linha (00470f20 float) — ver holeinonemissiondlg.cpp
struct LuaHelper983 : lua_system::lua_table_helper
{
	template <typename T>
	T Get(const char* name);
};
template <typename T>
T LuaHelper983::Get(const char* name)
{
	return table.get<T>(name);
}
#define LUA983_GET(tbl, type, name) ((LuaHelper983&)(tbl)).Get<type>(name)

bool __fastcall ConnectionUserCompare(const void* a, const void* b); // 0059f3c0 (unidade vizinha, não escrita)

// leitura por índice do lua_table_helper do 983 (como na normallobbyroom.cpp): o float sai embutido, o texto (004d5ba0) e a
// tabela filha (004d5250) por `call` — definições em dummyplayeractor.cpp
struct LuaIdx983 : lua_system::lua_table_helper
{
	void Push983(int index);
	template <typename T>
	T GetByIndex(int index);
	lua_system::lua_table_helper GetTableByIndex(int index); // 004d5250
};
__forceinline void LuaIdx983::Push983(int index)
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
__forceinline T LuaIdx983::GetByIndex(int index)
{
	Push983(index);
	return lua_tinker::pop<T>(state);
}
template <>
const char* LuaIdx983::GetByIndex<const char*>(int index); // 004d5ba0
#define LUAIDX983(tbl) (*(LuaIdx983*)&(tbl))

IMPLEMENT_OBJECT(FrGPTypeRoomDlg, FrMassTypeRoomDlg)

BEGIN_FRESH_MSGMAP(FrGPTypeRoomDlg, FrMassTypeRoomDlg)

ON_FRESH_VI("group_gp.user", FRCMD_INIT, FrGameRoomDlg::OnUserInit)
ON_FRESH_VI("group_gp.user", FRCMD_OWNERDRAW, FrGPTypeRoomDlg::OnGPUserOwnerDraw)
ON_FRESH_VV("group_gp.user", FRCMD_RBUTTONUP, FrGameRoomDlg::OnUserRButtonUp)
ON_FRESH_VI("group_gp.gp_detail", FRCMD_INIT, FrGPTypeRoomDlg::OnGPDetailInit)
ON_FRESH_VI("group_gp.gp_detail", FRCMD_HOVERON, FrGPTypeRoomDlg::OnGPDetailHoverOn)
ON_FRESH_VI("group_gp.gp_detail", FRCMD_HOVEROFF, FrGPTypeRoomDlg::OnGPDetailHoverOff)
ON_FRESH_VI("group_gp.grandprixinfo", FRCMD_INIT, FrGPTypeRoomDlg::OnGPInfoInit)
ON_FRESH_VV("group_gp.grandprixinfo", FRCMD_OWNERDRAW, FrGPTypeRoomDlg::OnGPInfoOwnerDraw)
ON_FRESH_VV("group_gp.game_start_waiting", FRCMD_INIT, FrGPTypeRoomDlg::OnWaitingInit)
ON_FRESH_VV("group_gp.game_start_waiting", FRCMD_OWNERDRAW, FrGPTypeRoomDlg::OnWaitingOwnerDraw)

END_FRESH_MSGMAP()

// 00512f80: quadro de horário do Grand Prix e grandprix_room_ui.lua (5 bitmaps pelo nome, 11 posições)
void FrGPTypeRoomDlg::Init983()
{
	m_bIdle = false;
	m_bReady = false;
	m_bLockControls = false;
	m_bCanStart = false;
	m_bAutoStart = false;
	m_bRoomStateReq = false;
	m_pStartTip = NULL;
	m_pReadyTip = NULL;
	g_mouse->ResetInputTime();
	g_ime->ResetInputTime();
	if (!m_pTimeBoard983)
	{
		m_pTimeBoard983 = new sGPTimeBoard983;
		m_pTimeBoard983->Init983();
	}

	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
	{
		script->RunFile("grandprix_room_ui.lua");
		const Bitmap** ppBmp = m_pGPUiBmp983;
		memset(ppBmp, 0, sizeof(m_pGPUiBmp983));
		{
			lua_system::lua_table_helper tbl = script->GetTable("grandprixroomuiname");
			int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
			for (int i = 1; i <= count; i++, ppBmp++)
			{
				const char* name = LUAIDX983(tbl).GetByIndex<const char*>(i);
				if (name)
					*ppBmp = g_pFresh->GetBitmap(name);
			}
		}
		{
			lua_system::lua_table_helper tbl = script->GetTable("grandprixroomuioffset");
			int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
			sGPPoint983* pOffset = m_gpUiOffset983;
			for (int i = 1; i <= count; i++, pOffset++)
			{
				float x = LUAIDX983(LUAIDX983(tbl).GetTableByIndex(i)).GetByIndex<float>(1);
				float y = LUAIDX983(LUAIDX983(tbl).GetTableByIndex(i)).GetByIndex<float>(2);
				pOffset->x = x;
				pOffset->y = y;
			}
		}
		lua_system::lua_script_manger::GetInstance()->DeleteScript(script);
	}
}

// 00511020
void FrGPTypeRoomDlg::Leave983()
{
	g_audio->StopSfx("gp_lobby_amb");
	if (m_pTimeBoard983)
	{
		delete m_pTimeBoard983;
		m_pTimeBoard983 = NULL;
	}
}

// 005115c0
FrGPTypeRoomDlg::~FrGPTypeRoomDlg()
{
	Leave983();
}

// 005116b0: a dica do "gp_detail" acompanha o cursor (FWF_TOOLTIPS)
void FrGPTypeRoomDlg::OnGPDetailHoverOn(int param)
{
	m_pGPDetail983->m_nFlags.Enable(FWF_TOOLTIPS);
}

// 005116c0
void FrGPTypeRoomDlg::OnGPDetailHoverOff(int param)
{
	m_pGPDetail983->m_nFlags.Disable(FWF_TOOLTIPS);
}

// 005143b0: lista de jogadores do Grand Prix — o desenho da sala de 30 (00513270) com o ícone de situação só "pronto"/"não
// pronto" (0/2), sem nível para a IA (sSlotInfo+0x159) e com o ícone da IA (+0x3c0) no lugar do sexo
void FrGPTypeRoomDlg::OnGPUserOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	sSlotInfo* pSlot = NULL;
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	pSlot = (sSlotInfo*)pItem->pData;

	if (IsLocalContent(S4_INVITE_FRIEND))
	{
		if (pSlot)
		{
			if (pSlot->IsInvite)
			{
				pDevice->DrawTexture(m_pInviteBmp,
					WRect(pItem->pos.x, pItem->pos.y,
						(float)m_pInviteBmp->Width(),
						(float)m_pInviteBmp->Height()),
					0xffffffff, 0);
				return;
			}
		}
	}

	pDevice->DrawTexture(m_pPlayerBaseBmp[0],
		WRect(pItem->pos.x, pItem->pos.y, (float)m_pPlayerBaseBmp[1]->Width(),
			(float)m_pPlayerBaseBmp[1]->Height()),
		0xffffffff, 0);

	if (pItem->selected)
		pDevice->Box(WRect(pItem->pos.x, pItem->pos.y,
						 (float)m_pUser->GetItemWidth() + 15.0f,
						 (float)m_pUser->GetItemHeight()),
			0x7f265785, 0, 0.0f);

	if (!pSlot)
		return;

	int index;
	if (!pSlot->bReady && !pSlot->bMaster)
		index = 0;
	else
		index = 2;

	WRect rect(0.0f, 0.0f, 0.0f, 0.0f);

	const Bitmap* pIcon = m_pSituBmp[index];
	rect = WRect(pItem->pos.x + 10.0f, pItem->pos.y + 5.0f,
		(float)pIcon->Width(), (float)pIcon->Height());
	pDevice->DrawTexture(pIcon, rect, 0xffffffff, 0);

	if (pSlot->angelicWings)
		pIcon = m_pGenderBmp[8 + pSlot->gender % 2];
	else if (pSlot->manner)
		pIcon = m_pGenderBmp[6 + pSlot->gender % 2];
	else
		pIcon = m_pGenderBmp[pSlot->gender];
	if (pSlot->bGrandPrixAI983 && m_3c0)
		pIcon = (const Bitmap*)m_3c0;

	if (pIcon)
	{
		rect = WRect(pItem->pos.x + 30.0f, pItem->pos.y, (float)pIcon->Width(),
			(float)pIcon->Height());
		pDevice->DrawTexture(pIcon, rect, 0xffffffff, 0);
	}

	if (m_pDiveBmp && pSlot->bSleep)
		pDevice->DrawTexture(m_pDiveBmp,
			WRect(pItem->pos.x + 34.0f, pItem->pos.y + 2.0f,
				(float)m_pDiveBmp->Width(), (float)m_pDiveBmp->Height()),
			0xffffffff, 0);

	const Bitmap* pLevel;
	if (pSlot->dwTitle)
	{
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(pSlot->dwTitle);
		if (pSkin)
			pLevel = g_pFresh->GetBitmap(pSkin->c.Icon);
		else
			pLevel =
				g_pFresh->GetBitmap(MakeStr("level_%03d", pSlot->level + 1));
	}
	else
	{
		pLevel = g_pFresh->GetBitmap(MakeStr("level_%03d", pSlot->level + 1));
	}
	if (pSlot->dwIdentity & 0x8000)
		pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");
	if (!pSlot->bGrandPrixAI983 && pLevel)
	{
		pDevice->DrawTexture(pLevel,
			WRect(pItem->pos.x + 80.0f - float2int(pLevel->Width() * 0.5f),
				pItem->pos.y + 13.0f - float2int(pLevel->Height() * 0.5f),
				(float)pLevel->Width(), (float)pLevel->Height()),
			0xffffffff, 0);
	}

	if (pSlot->GuildId && !pSlot->IsIdentity(0x14))
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			pSlot->GuildId, pSlot->dwEmblemVer983);

		if (pEmblem)
			pDevice->DrawTexture(pEmblem,
				WRect(pItem->pos.x + 115.0f, pItem->pos.y + 7.0f - 6.0f,
					(float)pEmblem->Width(), (float)pEmblem->Height()),
				0xffffffff, 0);
	}

	unsigned long color =
		(unsigned char)(pSlot->dwIdentity & 0x14) ? 0x50ffffff : 0xffffffff;

	if (pItem->underCursor)
	{
		pDevice->SetTextColor(0xffffffff, 0xff124371);
		pDevice->SetTextStyle(2);
	}
	else
	{
		pDevice->SetTextColor(0xffffffff, 0xffffffff);
		pDevice->SetTextStyle(0);
	}

	if (!CProjectG::Instance()->HidePrivacy())
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 140.0f, pItem->pos.y + 7.0f), 0, pSlot->sNick,
			-1.0f, color);

	if ((!Doc()->m_bGameOver ||
			(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1)) &&
		MyGuid(false) == ROOMDOC_MASTER(m_pDoc) && !pSlot->bMaster &&
		Doc()->m_roomInfo.gameType != GAME_TYPE_GUILD_MATCH)
		pDevice->DrawTexture(m_pKickBmp,
			WRect(pItem->pos.x + 226.0f, pItem->pos.y + 7.0f - 2.0f,
				(float)m_pKickBmp->Width(), (float)m_pKickBmp->Height()),
			pItem->underCursor ? 0xffffffff : 0x80ffffff, 0);
}

// 00511660 (sem chamador no 983; nome meu): desce a janela `dy` em y
void __stdcall MoveWndY983(FrWnd* pWnd, float dy)
{
	WRect rc(pWnd->GetRect());
	rc.y += dy;
	pWnd->SetRect(rc);
}

// 00515ac0: dica do "gp_detail" (m_artifactTip) — nome do Grand Prix, curso, buraco inicial e número de buracos, regra
// especial (0055ecc0) e os prêmios (1º lugar do GrandPrixRankReward.iff e os cinco prêmios do registro)
void FrGPTypeRoomDlg::OnGPDetailInit(int param)
{
	m_pGPDetail983 = DYNAMIC_CAST(FrButton, param);
	if (!m_pGPDetail983)
		return;
	m_pGPDetail983->EnableHover(true, 0.0f);

	IFF983::sGrandPrixData* pData = Doc()->m_itemManager.FindGrandPrixData983(Doc()->m_roomInfo.gpTypeId983);
	if (!pData)
		return;
	const sGPInfo983* pInfo = (const sGPInfo983*)pData;

	std::string text;
	text = MakeStr("%s : %s\n", GetGameString(0x7543), pInfo->name);
	if (Doc()->m_itemManager.HasGrandPrixSpecialHole983(pInfo->ticket) == true)
		text.append(GetGameString(0x7555));
	else if (pInfo->course == 0xff)
		text.append(K2L_Compatibility("\xc4\xda\xbd\xba : RANDOM"));
	else
	{
		unsigned long course = pInfo->course;
		if (course > 0x7f && course != 0xfd)
			course -= 0x80;
		IFF_STRUCT::sCourse* pCourse = Doc()->m_itemManager.FindCourse(course | 0x28000000);
		if (pCourse)
			text += MakeStr(K2L_Compatibility("\xc4\xda\xbd\xba : %s"), pCourse->c.Name);
		else
			text += K2L_Compatibility("\xc4\xda\xbd\xba : \xb8\xf0\xb8\xa7");
	}
	text.append("\n");

	int startHole = pInfo->startHole;
	unsigned char nHole = pInfo->nHole;
	if (startHole == 2)
		text.append(MakeStr(K2L_Compatibility("\xbd\xc3\xc0\xdb\xc8\xa6 : \xb7\xa3\xb4\xfd \xc8\xa6 / \xc8\xa6 \xbc\xf6 : %d \xc8\xa6"),
			nHole));
	else if (startHole == 3)
		text.append(MakeStr(K2L_Compatibility("\xbd\xc3\xc0\xdb\xc8\xa6 : \xbc\xc5\xc7\xc3 \xc8\xa6 / \xc8\xa6 \xbc\xf6 : %d \xc8\xa6"),
			nHole));
	else
		text.append(MakeStr(K2L_Compatibility("\xbd\xc3\xc0\xdb\xc8\xa6 : %d \xc8\xa6 / \xc8\xa6 \xbc\xf6 : %d \xc8\xa6"),
			startHole != 0 ? 10 : 1, nHole));
	text.append("\n");

	std::string rule = GPRuleText983(pInfo);
	if (rule.size() != 0)
	{
		text += rule;
		text.append("\n");
	}

	std::string prizes;
	std::vector<IFF983::sGrandPrixRankReward>* pRank = Doc()->m_itemManager.FindGrandPrixRankReward983(pInfo->ticket);
	if (pRank && pRank->begin() != pRank->end())
	{
		IFF983::sGrandPrixRankReward first = pRank->at(0);
		const unsigned long* pTid = (const unsigned long*)((const char*)&first + 0xc);
		for (unsigned int i = 0; i < 5; i++)
		{
			const char* name;
			if (pTid[i] && (name = Doc()->m_itemManager.GetItemName(pTid[i])) != NULL)
			{
				prizes.append(name);
				prizes.append(", ");
			}
		}
	}
	for (int i = 0; i < 5; i++)
	{
		const char* name;
		if (pInfo->prize[i] && (name = Doc()->m_itemManager.GetItemName(pInfo->prize[i])) != NULL)
		{
			prizes.append(name);
			prizes.append(", ");
		}
	}
	if (prizes.size() != 0)
	{
		std::string::size_type pos = prizes.find_last_of(",");
		if (pos != std::string::npos)
			prizes.erase(pos - 1);
		text.append(MakeStr(GetGameString(0x7546), prizes.c_str()));
	}
	m_artifactTip.assign(text, 0, std::string::npos);
}

// 005116d0
void FrGPTypeRoomDlg::OnGPInfoInit(int param)
{
	m_pGPInfo983 = DYNAMIC_CAST(FrArea, param);
}

// 005126d0: quadro do Grand Prix (grandprix_room_ui.lua) — fundo, horário (casas de começo e de fim com AM/PM), equipamento
// pedido, "fechado" e o número de salas
void FrGPTypeRoomDlg::OnGPInfoOwnerDraw()
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	FrArea* pArea = m_pGPInfo983;
	if (DOCJP()->GetGPChannel983())
	{
		const Bitmap* pBmp = m_pGPUiBmp983[0];
		pDevice->DrawTexture(pBmp,
			WRect(m_gpUiOffset983[0].x + pArea->GetRect().x, m_gpUiOffset983[0].y + pArea->GetRect().y,
				(float)pBmp->Width(), (float)pBmp->Height()),
			0xffffffff, 0);
		pDevice->DrawTexture(pBmp,
			WRect(m_gpUiOffset983[1].x + pArea->GetRect().x, m_gpUiOffset983[1].y + pArea->GetRect().y,
				(float)pBmp->Width(), (float)pBmp->Height()),
			0xffffffff, 0);
		pDevice->DrawTexture(m_pGPUiBmp983[1],
			WRect(m_gpUiOffset983[2].x + pArea->GetRect().x, m_gpUiOffset983[2].y + pArea->GetRect().y,
				(float)m_pGPUiBmp983[1]->Width(), (float)m_pGPUiBmp983[1]->Height()),
			0xffffffff, 0);
		pDevice->DrawTexture(m_pGPUiBmp983[2],
			WRect(m_gpUiOffset983[3].x + pArea->GetRect().x, m_gpUiOffset983[3].y + pArea->GetRect().y,
				(float)m_pGPUiBmp983[2]->Width(), (float)m_pGPUiBmp983[2]->Height()),
			0xffffffff, 0);
		if (m_pTimeBoard983 && m_158)
		{
			m_pTimeBoard983->DrawTime983(m_gpUiOffset983[7].x + pArea->GetRect().x,
				m_gpUiOffset983[7].y + pArea->GetRect().y);
			pBmp = !m_pTimeBoard983->IsStartPM983() ? m_pGPUiBmp983[3] : m_pGPUiBmp983[4];
			pDevice->DrawTexture(pBmp,
				WRect(m_gpUiOffset983[4].x + pArea->GetRect().x, m_gpUiOffset983[4].y + pArea->GetRect().y,
					(float)pBmp->Width(), (float)pBmp->Height()),
				0xffffffff, 0);
			pBmp = !m_pTimeBoard983->IsEndPM983() ? m_pGPUiBmp983[3] : m_pGPUiBmp983[4];
			pDevice->DrawTexture(pBmp,
				WRect(m_gpUiOffset983[5].x + pArea->GetRect().x, m_gpUiOffset983[5].y + pArea->GetRect().y,
					(float)pBmp->Width(), (float)pBmp->Height()),
				0xffffffff, 0);
			m_pTimeBoard983->DrawLimited983(m_gpUiOffset983[9].x + pArea->GetRect().x,
				m_gpUiOffset983[9].y + pArea->GetRect().y);
		}
	}
	if (m_pTimeBoard983)
		m_pTimeBoard983->DrawClosed983(m_gpUiOffset983[10].x + pArea->GetRect().x,
			m_gpUiOffset983[10].y + pArea->GetRect().y);
	if (m_158)
		((WTitleFontEx*)m_158)->Print(m_gpUiOffset983[8].x + pArea->GetRect().x,
			pArea->GetRect().y - 11.0 + m_gpUiOffset983[8].y, MakeStr(" %d", m_p49c), (eTITLEFONTALINE)2, 0.0f, 1.0f);
}

// 00514340
FrGPTypeRoomDlg::FrGPTypeRoomDlg()
{
	Init983();
}

// 00514a80: grandprix_ai_player.lua — quanto esperar até a partida e entre uma IA e a próxima na lista
void FrGPTypeRoomDlg::OnWaitingInit()
{
	ROOMDOC_B18(m_pDoc) = false;
	m_bAI983 = false;
	m_lastSlotAddTime983 = 0;
	m_waitStartTime983 = 0;
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
	{
		script->RunFile("grandprix_ai_player.lua");
		lua_system::lua_table_helper tbl = script->GetTable("ai_player_waiting_room_slot_add_table");
		m_startWaitSec983 = LUA983_GET(tbl, float, "game_start_waiting_time_sec");
		m_slotAddSec983 = LUA983_GET(tbl, float, "add_slot_time_sec");
		lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
	}
}

// 00515470: na espera do Grand Prix as IAs entram na lista uma a uma (pelo lugar) e, vencido o prazo, o ator principal
// recebe 0x86 (começar) e a espera recomeça
void FrGPTypeRoomDlg::OnWaitingOwnerDraw()
{
	if (ROOMDOC_B18(m_pDoc))
	{
		unsigned long elapsed = g_CurrentTime - m_waitStartTime983;
		unsigned long startMs = (unsigned long)(m_startWaitSec983 * 1000.0f);
		unsigned long addMs = (unsigned long)(1000.0f * m_slotAddSec983);
		if (g_CurrentTime - m_lastSlotAddTime983 >= addMs && m_bAI983)
		{
			unsigned int rank = m_pUser->GetCurrentItemSize(false) + 1;
			for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin(); it != Doc()->m_slotList.end(); ++it)
			{
				if ((*it).connectionRank == rank)
				{
					for (std::list<FrListItem*>::iterator item = m_pUser->m_itemList.begin();
						item != m_pUser->m_itemList.end(); ++item)
					{
						if ((*item)->pData == NULL)
						{
							m_pUser->_DelItem(item);
							m_pUser->AddItem(&(*it));
							m_pUser->SortItem(ConnectionUserCompare);
							break;
						}
					}
					m_lastSlotAddTime983 = g_CurrentTime;
				}
			}
		}
		if (elapsed >= startMs)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x86, 0, 0, 0, 0, 0));
			OnWaitingInit();
		}
	}
}

// 00516fc0: só o grupo do Grand Prix; mapa, combo, "gp_detail" e "grandprixinfo" sobem
bool FrGPTypeRoomDlg::OnInit()
{
	m_pGroupGp->SetVisible(true);
	SetGPRoom983();
	m_pTimeBoard983->SetFont983((WTitleFontEx*)m_154);
	if (m_pBg)
		m_pBg->Open("gameroom_background_gp.tga");
	if (m_pGuildInvite)
		m_pGuildInvite->SetVisible(false);
	if (m_pChangeTeam)
		m_pChangeTeam->SetVisible(false);
	if (m_pRoomOption)
		m_pRoomOption->SetVisible(false);
	{
		WRect rc(m_pMap->GetRect());
		rc.y -= 15.0;
		m_pMap->SetRect(rc);
	}
	{
		WRect rc(m_pMapHoleCombo->GetRect());
		rc.y -= 38.0;
		m_pMapHoleCombo->SetRect(rc);
	}
	{
		WRect rc(m_pGPDetail983->GetRect());
		rc.y -= 15.0;
		m_pGPDetail983->SetRect(rc);
	}
	{
		WRect rc(m_pGPInfo983->GetRect());
		rc.y -= 40.0;
		m_pGPInfo983->SetRect(rc);
	}
	return FrMassTypeRoomDlg::OnInit();
}
