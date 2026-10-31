#include "minatl.h"
#include "vc10_layout.h"
#define REBANG_GOLFTASK_CPP
#include "golftask.h"
#include "golfdoc.h"

// in the order the original emits their inlines (before approachdesc.h)
#include "mousecursor.h"
#include "clientsetting.h"
#include "messengerdlg.h"
#include "cardsystem.h"
#include "actor.h"
#include "approachdesc.h"
#include "autotarget.h"
#include "bonusticket.h"
#include "capturedbg.h"
#include "chatmsg.h"
#include "club.h"
#include "commonutil.h"
#include "exhibition.h"
#include "fresh.h"
#include "frdesktop.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "golfball.h"
#include "guilddefine.h"
#include "hackingmanager.h"
#include "headicon.h"
#include "inputmanager.h"
#include "logininfo.h"
#include "matchhistory.h"
#include "optiondlg.h"
#include "packet.h"
#include "projectg.h"
#include "quadtree.h"
#include "quitwindow.h"
#include "jp/extraresource.h"
#include "jp/uiframesetmanager.h"
extern WView* g_view; // 00f02458
#include "shareddoc.h"
#include "soundmanager.h"
#include "wind.h"
#include "s5/classicserver/classicserver.h"
#include "wview.h"
#include "../shared/localize.h"
#include "mathconsts.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B1399 temporario


extern Fresh* g_pFresh;
extern WView* g_view;
extern int g_iRecvPacketType;

int IsMassGame();
unsigned char __fastcall GetHoleIndex(unsigned char hole);
unsigned char __fastcall MyIndex();
void __fastcall RequestUccInfo(sCharacterInfo& charInfo);
void __fastcall RequestUccInfo(sUserInfo& info);

// shareddoc.inl has these three only for the clientsetting PCH variant.
inline void SetHoleType(unsigned char type)
{
	Doc()->m_holeType = type;
}

inline void SetShotTimeLimit(unsigned long limit)
{
	Doc()->m_golfGame.shotTimeLimit = limit;
}

inline void SetGameTimeLimit(unsigned long limit)
{
	Doc()->m_golfGame.gameTimeLimit = limit;
}

// eTeamName: golfdoc.h has it only for the golfrule PCH variant.
enum eTeamWinner
{
	TEAMWINNER_RED,
	TEAMWINNER_BLUE
};

#pragma pack(push, 1)
struct sShotSyncData
{
	unsigned char data[0x32];	// 983 0069f67c: bloco de 0x32 (era 0x2e no 645)
};

struct sShotResultData
{
	unsigned long uid;
	unsigned char data[0x22];	// 983 0069fff5: bloco de 0x26 = sShotResultMsg (era 0x25 no 645)
};

struct sHoleResultData
{
	unsigned long uid;
	unsigned char data[0x1d];
};
// Player snapshot sent to the gallery (0x40 -> 0x96).
// Level byte of IFF_ITEM_COMMON (the bit-field pair after Name).
struct sItemLevel
{
	unsigned char Level : 7;
	unsigned char IsUnderLvl : 1;
};

struct sGalleryVector
{
	float x;
	float y;
	float z;
};

struct sGalleryPlayerData
{
	float angle;
	sGalleryVector startPos;
	sGalleryVector pos;
	unsigned char stroke[18];
	unsigned char putt[18];
	__int64 pang[18];
	int score;
	unsigned char totalStroke;
	__int64 totalPang;
	unsigned char itemNum;
	unsigned long item;
	unsigned long windAbility983;	// 983: +0xe2, novo -> sPlayerData::windAbility983 (bloco de 0xfb; era 0xf7 no 645)
	float gauge;
	unsigned char bunker;
	unsigned char flag;
	unsigned char powerShot;
	unsigned char club;
	unsigned long frame;
	unsigned char state;
	__int64 bonusPang;
};
#pragma pack(pop)

// The original header of these is unknown; only their .data bytes are known.
static char s_base64Encode[64] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
	'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1',
	'2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };
static int s_base64Decode[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52,
	53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
	37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static const char* s_guildActorClass = "CGuildActor";
static const char* s_guildActorName = "GuildActor";
static const char* s_lineFeed = "\n";
static const char* s_gatewayActorClass = "CGatewayActor";
static const char* s_gatewayActorName = "Gateway";

IMPLEMENT_OBJECT(CGolfTask, CTask)

CGolfTask::CGolfTask()
{
	m_bLobbyScreenSize = false;
	IActor::m_defaultSkipLayer = 0x622021;
	m_exitDelay = -10.0f;
	PushWind983(); // 983 0069ecaf
}

CGolfTask::~CGolfTask()
{
	PopWind983(); // 983 0069ea37
}

void CGolfTask::Init(const char* wallPaper)
{
	CTask::Init(NULL);
	// 983 (0069d180): mascara de modos em que a janela de sair nao abre (CQuitWindow+0x34, 006efb00); Grand Prix 0x14/0x15 soma 0x20000
	if (CQuitWindow::Instance())
		CQuitWindow::Instance()->SetBlockedModes983(
			(unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2 ? 0x22030 : 0);
}

// 983 (0069dc40): 1a parte da lista de atores do Register (metodo proprio). Em relacao ao 645: entram
// CTutorGuide, CDummyPlayerActor, ShotReview e CAIPlayer (Grand Prix); saem CFx e CGuildActor; CClock, CGrid, CGuideLine,
// CBubble e CGuideBook vao para a 2a parte; NPC 0x2001 -> 0x222001, Spot 0x20 -> 0x2020, Screen 0x2020 no Grand Prix.
void CGolfTask::RegisterActors983()
{
	AddActor("CGallery", "Gallery", 0, "");
	AddActor("CCamera", "Camera", 0x20, "");
	AddActor("CTutorGuide", "TutorGuide", 0x200, "");
	AddActor("CGolfBg", "GolfBg", 0x22001, "");
	AddActor("CGolfPlayer", "Player", 0x22000, "");
	AddActor("CGolfCaddie", "Caddie", 0x2000, "");
	AddActor("CMascot", "Mascot", 0x2000, "");
	AddActor("CNPC", "NPC", 0x222001, "");
	AddActor("CSpot", "Spot", 0x2020, "");
	AddActor("CBall", "Ball", 0x2000, "");
	AddActor("CTrajectory", "Trajectory", 0, "");
	AddActor("CParticleEffect", "Effect", 0x20001, "");
	AddActor("CAmbientSound", "Ambient", 1, "");
	AddActor("CDummyPlayerActor", "DummyPlayer", 0x22000, "");
	AddActor("CLetter", "Letter", 0x20020, "");
	if ((unsigned)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)	// Grand Prix
		AddActor("CScreen", "Screen", 0x2020, "");
	else
		AddActor("CScreen", "Screen", 0x20, "");
	AddActor("CSpin", "Spin", 0, "");
	AddActor("CLogo", "Logo", 1, "");
	AddActor("CScore", "Score", 0x222020, "");
	AddActor("CItemWindow", "Item", 0x2000, "");
	AddActor(s_gatewayActorClass, s_gatewayActorName, 0, GetRTTI()->GetName());
	AddActor("ShotPreview", "ShotPreview", 0, "");
	AddActor("ShotReview", "ShotReview", 0, "");
	if ((unsigned)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
		AddActor("CAIPlayer", "AIPlayer", 0x2000, "");
}

// 983 (0069e4c0): 2a parte. Na sala "hole in one" (tipo 11) nao ha bolha/grade/guia/relogio/guia de jogo/tutorial/
// cutin/itens do chao/tapete; sempre CReactionObjectMng e o vento classico (CClassicWind ou o boneco CNoClassicWind).
void CGolfTask::RegisterActorsExtra983()
{
	if (Doc()->m_golfGame.gameType != GAME_TYPE_HOLEINONE983)
	{
		AddActor("CBubble", "Bubble", 0x20, "");
		AddActor("CGrid", "Grid", 0x20, "");
		AddActor("CGuideLine", "GuideLine", 0x20, "");
		AddActor("CClock", "Clock", 0x20, "");
		AddActor("CGuideBook", "GuideBook", 0x22020, "");
		// 983 (0069e687): o tutorial e o tipo de jogo, nao a tarefa preservada; o 645 pedia os nomes ao "TutorialMain" (M983(0x14e))
		if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
			AddActor("CTutorial", "Tutorial", 0x20, "");
		else
			AddActor("CCutin", "Cutin", 0, "");
		if (DetermineWhetherToAddActor_GroundItemMan())
			AddActor("GroundItemMan", "GroundItemMan", 0, "");
		AddActor("CMagicCarpetHelper", "MagicCarpetHelper", 0, "");
	}
	AddActor("CReactionObjectMng", "ReactionObject", 0x202001, "");
	if (S5::CLASSICSRV::IsClassicGame983() &&
		Doc()->m_golfGame.gameType != GAME_TYPE_TUTORIAL_BASIC &&
		Doc()->m_golfGame.gameType != GAME_TYPE_TUTORIAL_ADV &&
		Doc()->m_playingMode != 2)
		AddActor("CClassicWind", "ClassicWind", 0x20, "");
	else
		AddActor("CNoClassicWind", "ClassicWind", 0, "");
}

// 983 (0069e8d0)
void CGolfTask::Register()
{
	SetMainActor(AddActor("CGolfRule", "GolfRule", 0x20021, ""));
	RegisterActors983();
	RegisterActorsExtra983();
	// mapa 0x40 (sem o bit 0x80 de "aleatorio", exceto 0xfd): jogo de massa
	unsigned char map = Doc()->m_roomInfo.mapType;
	if (((map > 0x7f && map != 0xfd) ? map - 0x80 : map) == 0x40)
		AddActor("ShotTrajectoryForMassGame", "ShotTrajectoryForMassGame", 0, "");
}

void CGolfTask::Load()
{
	IActor::m_pLayer = &GOLFDOC()->m_gameMode;

	new CQuitWindow;

	SetLoadingBackGround(CExtraResource::Instance()->GetLoading(2));

	g_pFresh->GetDesktop()->SetViewFocus(true);

	CChatMsg::Instance()->ResetUiPosition();
	CUiFrameSetManager::Instance()->ReloadImages983(g_view); // 983 0069db10

	CTask::Load();
}

void CGolfTask::Destroy()
{
	CTask::Destroy();

	g_pFresh->GetDesktop()->SetWallPaper("gameroom_background.jpg", true);
	g_pFresh->GetDesktop()->SetBgColor(0xffffffff);
	CMouseCursor::Instance()->SetTempActive(false);

	if (CQuitWindow::Instance())
		delete CQuitWindow::Instance();
}

void CGolfTask::Process(float delta)
{
	if (HackingManager::Instance())
		HackingManager::Instance()->ResetCrypticValue(20);

	if (Doc()->m_playingMode == 1 && g_input->GetDown("ESCAPE", true) == 1 &&
		!g_pFresh->GetManager()->HasEscKeyWindow())
	{
		// 983 0069d1c0: o 983 nao consulta nem avisa o ator "Item" ao fechar com ESC
		if (CChatMsg::Instance()->IsOpened())
		{
			CChatMsg::Instance()->Close(false, true);
		}
		else
		{
			if (CMessengerInfo::Instance()->Close(false))
			{
				if (CQuitWindow::Instance())
					CQuitWindow::Instance()->Process();
			}
		}
	}

	Doc()->m_chatManager.CheckPenaltyTime();

	if (!CChatMsg::Instance()->IsOpened() &&
		Doc()->m_golfGame.gameType != GAME_TYPE_TUTORIAL_BASIC &&
		Doc()->m_golfGame.gameType != GAME_TYPE_TUTORIAL_ADV &&
		!g_pFresh->GetManager()->IsKeyExclusive() &&
		(g_input->GetDown("ENTER", true) || g_input->GetDown("PADENTER", true)))
	{
		if ((g_input->Get("LCONTROL", true) ||
				g_input->Get("RCONTROL", true)) &&
			(g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true)))
		{
			GetActor("GolfRule") << MsgObject(NULL, M983(0x37),
				(int)K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4"), 0, 0, 0, 0);
		}
		else if (g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true))
		{
			GetActor("GolfRule") << MsgObject(NULL, M983(0x37),
				(int)K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"), 0, 0, 0, 0);
		}
		else if (g_input->Get("LCONTROL", true) ||
			g_input->Get("RCONTROL", true))
		{
			GetActor("GolfRule") << MsgObject(NULL, M983(0x65), 0, 0, 0, 0, 0);
		}

		CChatMsg::Instance()->Open(true, true);
	}

	if (g_input->GetDown("\xb8\xc5\xc5\xa9\xb7\xce\xc3\xa2", true) &&
		!g_input->Get("LALT", true) && !g_input->Get("RALT", true) &&
		!g_input->Get("LCONTROL", true) && !g_input->Get("RCONTROL", true))
	{
		FrOptionDlg* pDlg = CreateForm<FrOptionDlg>(g_pFresh->GetManager(),
			this, "option", NULL);
		if (pDlg)
		{
			pDlg->Open(NULL, 1);
			pDlg->SetApplyScrSize(true);
			pDlg->SetCloseResult((eFormRet)2);
			pDlg->OpenTab((FrOptionDlg::eTabType)3);
		}
	}

	float step = delta;
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2 &&
		Doc()->m_replayState == 4)
	{
		step = 0.0f;
	}

	CTask::Process(step);
	g_pFresh->OnProcess(delta);
	CChatMsg::Instance()->ProcessWindow(delta);

	if (m_exitDelay > 0.0f)
	{
		m_exitDelay -= delta;
		if (m_exitDelay < 0.1f)
		{
			m_exitDelay = -10.0f;
			Doc()->m_myInfo.roomIndex = 0xffff;
			CProjectG::Instance()->FromGolfToLobbySettings();
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
				false);
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE",
				0, 0, 0);
			// 983 0069d1c0: aviso 0x8b (p1=4, p2=1 nos tipos de jogo 0x14/0x15) antes do 0x1a; o texto de ida e sempre "TOPPAGE"
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x8b, 4,
				(unsigned)(Doc()->m_roomInfo.realGameType - 0x14) <= 1 ? 1 : 0, 0, 0);
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", M983(0x1a), 0, 0, 0, 0);

			// 983: um so PostMsg depois do switch (a mensagem 0x23 leva o texto escolhido)
			const char* exitMsg = NULL;
			switch (Doc()->m_lobbyExitReason)
			{
			case 1:
				if (Doc()->m_golfGame.gameType != GAME_TYPE_NEW_APPROACH)
					exitMsg = "\xc1\xa2\xbc\xd3\xc0\xcc \xba\xd2\xbe\xc8\xc7\xcf\xbf\xa9 \xb0\xd4\xc0\xd3\xbf\xa1\xbc\xad \xc5\xf0\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.";
				break;
			case 2:
				exitMsg = "\xc5\xb8\xc0\xd3\xbe\xc6\xbf\xf4 3\xb9\xf8\xc0\xb8\xb7\xce \xbd\xc7\xb0\xdd\xb5\xc7\xbe\xee \xc5\xf0\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.";
				break;
			case 4:
				exitMsg = "\xb1\xe2\xba\xea\xbe\xf7 3\xb9\xf8\xc0\xb8\xb7\xce \xbd\xc7\xb0\xdd\xb5\xc7\xbe\xee \xc5\xf0\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.";
				break;
			case 8:
				exitMsg = "\xb0\xd4\xc0\xd3\xc1\xf8\xc7\xe0\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xc6\xce\xc0\xcc \xbe\xf8\xbe\xee\xbc\xad \xc5\xf0\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.";
				break;
			case 0x1000:
				exitMsg = "\xc5\xb8\xc0\xd3\xbe\xc6\xbf\xf4\xc0\xcc \xb5\xc7\xbe\xee\xb5\xb5 \xbd\xc7\xb0\xdd\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xc6 \xc5\xf0\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.";
				break;
			case 0x2000:
				exitMsg = "\xc0\xd4\xc0\xe5 \xc6\xbc\xc4\xcf\xc0\xcc \xba\xce\xc1\xb7\xc7\xcf\xbf\xa9 \xc5\xf0\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.";
				break;
			}

			if (exitMsg)
				CTaskManager::Instance()->PostMsg(NULL, "Lobby", M983(0x23),
					(int)K2L_Compatibility(exitMsg), 0, 0, 0);
		}
	}
}

void CGolfTask::Display()
{
	CTask::Display();

	if (GOLFDOC()->m_gameMode & 0x400000)
	{
		GetActor("Score") << MsgObject(NULL, M983(0x12f), 3, 0, 0, 0, 0);
		GetActor("GolfRule") << MsgObject(NULL, M983(0x78), 2, 0, 0, 0, 0);
	}

	if (g_pFresh->GetDesktop())
		g_pFresh->GetDesktop()->OnDisplay(true, false, true, true);

	if (!CChatMsg::Instance()->HasBeenPrinted())
	{
		CChatMsg::Instance()->PrintInScreen();
		CChatMsg::Instance()->DrawChatWindow();
		CChatMsg::Instance()->PrintInChatWindow();
	}

	if (!CCapturedBg::Instance()->IsCaptured() ||
		GOLFDOC()->m_tutorialMode != 15)
	{
		g_pFresh->OnDisplay(false);
	}

	if (GOLFDOC()->m_gameMode & 0x10)
	{
		if (CCapturedBg::Instance())
			CCapturedBg::Instance()->Capture();
	}

	if ((GOLFDOC()->m_gameMode & 1) && OnlinePlay() && !IsMassGame())
		DrawLoadingInfo(g_pFresh);
}

void CGolfTask::OnDisplayWhileLoading()
{
	if (0)
		((CGolfTask*)0)->CGolfTask::Process(*(float*)0);
	CTask::OnDisplayWhileLoading();
	OnlinePlay();
}

void CGolfTask::UpdateLoadingInfo(unsigned long uid, unsigned char progress)
{
	std::list<sSlotInfo>::iterator it =
		std::find(Doc()->m_slotList.begin(), Doc()->m_slotList.end(), uid);

	if (it != Doc()->m_slotList.end())
		m_loadingInfo[it->dwGuid] = progress;
}

void CGolfTask::DrawLoadingInfo(Fresh* pFresh)
{
	FrGraphicInterface* pGDI = pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	float y = g_view->GetHeight() - 100.0f;

	std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
	int offset = 0;
	for (; it != Doc()->m_slotList.end(); ++it)
	{
		sSlotInfo& slot = *it;
		if (slot.dwGuid == MyGuid(false))
			continue;

		unsigned char progress = m_loadingInfo[slot.dwGuid];

		pGDI->SetTextColor(0xffffffff, 0xff000000);
		pGDI->SetTextStyle(2);

		char* text;
		if (progress == 10)
		{
			text = MakeStr(
				K2L_Compatibility("%s\xb4\xd4\xc0\xba \xb7\xce\xb5\xf9 \xbf\xcf\xb7\xe1 \xc0\xd4\xb4\xcf\xb4\xd9."),
				slot.sNick);
		}
		else
		{
			text = MakeStr(
				K2L_Compatibility("%s\xb4\xd4\xc0\xba \xc7\xf6\xc0\xe7 %d%% \xb7\xce\xb5\xf9 \xc1\xdf \xc0\xd4\xb4\xcf\xb4\xd9."),
				slot.sNick, progress * 10);
		}

		pFresh->GetManager()->PrintText(WPoint(20.0f, y + offset), 0, text,
			-1.0f, 0xffffffff);
		offset += 20;
	}
}

void CGolfTask::OnInitFinished()
{
	if (OnlinePlay())
	{
		WSendPacket packet((enumClientPacket)0x11);
		// 983 0069d6f2: nos tipos de jogo 11 a 14 o pacote leva um dword pedido ao ator "Gateway" (mensagem 0x7535,
		// sub 9, ponteiro do valor); nos outros tipos segue vazio como no 645 (lote 9 da C-PKT)
		if ((unsigned)(Doc()->m_roomInfo.realGameType - 11) <= 3)	// doc+0x109b8
		{
			unsigned long value983 = 0;
			AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 9, (int)&value983, 0, 0, 0);
			packet.Encode4(value983);
		}
		packet.Send(TO_GAME);
	}

	if (CChatMsg::Instance())
	{
		CChatMsg::Instance()->Close(true, true);
		CChatMsg::Instance()->EnableCaretMove(true);
	}

	GetActor("GolfRule") << MsgObject(NULL, M983(0x29), 0, 0, 0, 0, 0);

	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 11:
		{
			{
				WSendPacket packet((enumClientPacket)0x3f);
				packet.Encode4(Doc()->m_userInfo[0].info.dwGuid);
				packet.Send(TO_GAME);
			}
			break;
		}
		}
	}

	// 983 0069d847: o aviso ao Gateway (0x7532, sub 0xa) e incondicional: tambem sem o bit 1 de dwIdentity e nos outros gameType
	GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7532, 0xa, 0, 0, 0, 0);
}

// 983 0062fe70 (CSharedDoc, stdcall ret 8; so o 2o parametro e lido): false se algum rival que nao e o `oid`
// (e sem bGrandPrixAI983) esta em estado diferente de 1/3. Nome meu.
static bool AllOtherRivalsSettled983(unsigned char /*hole*/, unsigned long oid)
{
	std::vector<sRivalData>& list = CSharedDoc::Instance()->m_rivalList;
	for (size_t i = 0; i < list.size(); i++)
	{
		const sRivalData& r = list[i];
		if (r.bGrandPrixAI983 != 0 || r.oid == oid)
			continue;
		if (r.state != 3 && r.state != 1)
			return false;
	}
	return true;
}

int CGolfTask::OnPacket(WReceivedPacket& packet)
{
	unsigned short type = packet.Decode2();

	g_iRecvPacketType = type;
	// PKT (frente C, sessao C15): os 64 rotulos abaixo estavam com os numeros de pacote do KR 645 - a renumeracao da sessao 61
	// (f40710c) so tinha alcancado a copia morta do hibrido (#ifndef REBANG_SEMANTIC_ONLY, acima), que o build do 983 nao compila.
	// Agora sao os do 983, na mesma ordem da copia morta (out/PKT/s8/renumeracao_golftask.tsv).
	switch (type)
	{
	case 0xf4: // 645: 0xf2
	{
		unsigned char bOn = packet.Decode1();
		GetActor("Screen") << MsgObject(NULL, M983(0x1cf), bOn, 0, 0, 0, 0);
		break;
	}

	case 0xc8: // 645: 0xc6
	{
		// PKT (frente C, sessao C15): forma do 983 (caso 0xc8, 0069efde) - so o saldo; a conferencia "[Pang Hacker]" do 645
		// (conteudo local 0x87) nao existe mais no exe
		__int64 pang = packet.Decode8();
		Doc()->m_myInfo.stat.i64Pang = pang;
		Doc()->m_userInfo[MyIndex()].stat.i64Pang = pang;
		break;
	}

	case 0xc7: // 645: 0xc5
	{
		float speed;
		packet.DecodeBuffer(&speed, sizeof(speed));
		CProjectG::Instance()->SetGameSpeed(speed);

		// B773: 983 (caso 0xc7, 006a39a0): le o guid e descarta; NAO consome o item 0x1a000011 da lista local (era do 645)
		packet.Decode4();

		GetActor("Letter") << MsgObject(NULL, M983(0x193), 0, 0, 0, 0, 0);
		break;
	}

	case 0x115: // 645: 0x113
	{
		if (!IsLocalContent((localContentType_t)0x33 /*983: era 0x32; interpolado*/))
			break;

		char ticket[100] = { 0 };
		std::string name;
		packet.DecodeStr(name);
		packet.DecodeBuffer(ticket, sizeof(ticket));
		CBonusTicket::Instance().SetTicketFromServer(name,
			(unsigned char*)ticket, sizeof(ticket));
		break;
	}

	case 0x54: // 645: 0x52
	case 0x67: // 645: 0x65
		if (CMessengerInfo::Instance()->GetMyStatus() != 3)
			CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)4);

		GetActor("GolfRule") << MsgObject(NULL, M983(0x8f), 1, 0, 0, 0, 0);
		break;

	case 0x53: // 645: 0x51
	{
		// 983 (0069f55a): no Grand Prix (tipo 0x14/0x15) avisa o AIPlayer do buraco atual; depois do contagem de rivais
		unsigned char realType = Doc()->m_roomInfo.realGameType;
		if (realType >= 0x14 && realType <= 0x15)
			GetActor("AIPlayer")
				<< MsgObject(NULL, 0x346, Doc()->m_pGolfDoc->m_currentHole, 0, 0, 0, 0);

		unsigned long uid = packet.Decode4();

		if (IsMassGame())
		{
			GetActor("GolfRule") << MsgObject(NULL, 0x9c, 0, 0, 0, 0, 0);
		}
		else
		{
			unsigned char index = Doc()->GetIndex(uid);
			if (Doc()->AreWePlayingTogether(uid))
				GetActor("GolfRule") << MsgObject(NULL, 0x9c, index, 0, 0, 0, 0);

			if (IsLocalContent((localContentType_t)0x29 /*983: era 0x28; interpolado*/))
				CMatchHistory::Instance()->RequestHistory();
		}

		GetActor("GolfRule") << MsgObject(NULL, 0x32d, 0, 0, 0, 0, 0);

		IActor* gateway = AfxGetTask()->GetActor(s_gatewayActorName);
		gateway << MsgObject(NULL, 0x753b, 2, 0, 0, 0, 0);
		gateway << MsgObject(NULL, 0x753b, 3, 0, 0, 0, 0);

		unsigned int count = 0;
		GetActor("AIPlayer") << MsgObject(NULL, 0x349, (int)&count, 0, 0, 0, 0);
		if (count != 0 && count <= Doc()->m_rivalList.size())
			GetActor("AIPlayer") << MsgObject(NULL, 0x347, 0, 0, 0, 0, 0);
		break;
	}

	case 0x5b: // 645: 0x59
	{
		unsigned char a = packet.Decode1();
		unsigned char color = packet.Decode1();
		CCardManager::Instance()->SetWindColor(color);
		unsigned char b = packet.Decode1();
		unsigned char c = packet.Decode1();
		unsigned char d = packet.Decode1();

		Wind().SetActive(true);
		Wind().Init(a, b, c, d);
		break;
	}

	case 0x55: // 645: 0x53
	{
		unsigned long uid = packet.Decode4();
		sShotSyncData sync;
		packet.DecodeBuffer(&sync, sizeof(sync));
		// 983 (0069f683/0069f6ec): ClassicWind (0x358, &packet, 1) e GroundItemMan (0x2f3, &packet) recebem o PACOTE
		// (ebx), nao o uid; com o uid o CNoClassicWind fazia Decode4 em 0x186a1 -> queda no eco da tacada com bot (B1307)
		GetActor("ClassicWind") << MsgObject(NULL, 0x358, (int)&packet, 1, 0, 0, 0);
		packet.GetRemainLength();

		GetActor("GroundItemMan")
			<< MsgObject(NULL, M983(0x279), (int)&packet, 0, 0, 0, 0);

		if (Doc()->AreWePlayingTogether(uid))
		{
			*(sShotSyncData*)GOLFDOC()->m_shotSync983 = sync;
			GetActor("GolfRule") << MsgObject(NULL, M983(0x7a), 0, 0, 0, 0, 0);
		}
		break;
	}

	case 0x56: // 645: 0x54
	{
		unsigned long uid = packet.Decode4();
		float angle;
		packet.DecodeBuffer(&angle, sizeof(angle));

		if (uid != Doc()->m_myInfo.info.dwGuid &&
			Doc()->AreWePlayingTogether(uid))
			GetActor("Camera")
				<< MsgObject(NULL, M983(0xf2), (int)&angle, 0, 0, 0, 0);
		break;
	}

	case 0x57: // 645: 0x55
	{
		unsigned long uid = packet.Decode4();
		sRemoteGauge983 power;
		packet.DecodeBuffer(&power, sizeof(power));

		if (uid != Doc()->m_myInfo.info.dwGuid &&
			Doc()->AreWePlayingTogether(uid))
		{
			GOLFDOC()->m_remoteGauge983 = power;
			GetActor("GolfRule") << MsgObject(NULL, M983(0x7b), 0, 0, 0, 0, 0);
		}
		break;
	}

	case 0x58: // 645: 0x56
	{
		unsigned long uid = packet.Decode4();
		unsigned char motion = packet.Decode1();

		if (uid != Doc()->m_myInfo.info.dwGuid &&
			Doc()->AreWePlayingTogether(uid))
			GetActor("Player") << MsgObject(NULL, M983(0x11f), motion, 0, 0, 0, 0);
		break;
	}

	case 0x59: // 645: 0x57
	{
		unsigned long uid = packet.Decode4();
		unsigned char club = packet.Decode1();

		if (uid != Doc()->m_myInfo.info.dwGuid &&
			Doc()->AreWePlayingTogether(uid))
		{
			GolfClub().SetClub(club);
			g_audio->PlaySfx("\xc5\xac\xb7\xb4\xb1\xb3\xc3\xbc");
		}
		break;
	}

	case 0x5a: // 645: 0x58
	{
		unsigned long uid = packet.Decode4();
		unsigned long item = packet.Decode4();

		IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/);

		if (OnlinePlay())
		{
			unsigned long owner = packet.Decode4();
			if (Doc()->AreWePlayingTogether(owner))
				GetActor("Item")
					<< MsgObject(NULL, M983(0x19f), owner, uid, item, 0, 0);
		}
		else
		{
			GetActor("Item") << MsgObject(NULL, M983(0x19f), -1, uid, item, 0, 0);
		}
		break;
	}

	case 0x5c: // 645: 0x5a
	{
		unsigned long uid = packet.Decode4();

		if (Doc()->AreWePlayingTogether(uid))
		{
			GetActor("GolfRule") << MsgObject(NULL, M983(0x7c), uid, 0, 0, 0, 0);
			GetActor("TutorGuide") << MsgObject(NULL, M983(0x7c), 0, 0, 0, 0, 0); // 983 0069fbe0
		}
		break;
	}

	case 0x5d: // 645: 0x5b
	{
		unsigned long uid = packet.Decode4();
		unsigned short icon = packet.Decode2();

		if (uid != Doc()->m_myInfo.info.dwGuid &&
			Doc()->AreWePlayingTogether(uid))
		{
			if (CHeadIcon::Instance())
				CHeadIcon::Instance()->SetIcon(uid, icon);
		}
		break;
	}

	case 0x5e: // 645: 0x5c
	{
		unsigned long uid = packet.Decode4();

		if (uid != Doc()->m_myInfo.info.dwGuid &&
			Doc()->AreWePlayingTogether(uid))
			GetActor("Caddie") << MsgObject(NULL, M983(0x1da), 0, 0, 0, 0, 0);
		break;
	}

	case 0x63: // 645: 0x61
	{
		unsigned long uid = packet.Decode4();

		if (Doc()->AreWePlayingTogether(uid))
		{
			if (Doc()->m_golfGame.gameType == 7)
				GOLFDOC()->m_skinsNextTurnWord983 = packet.Decode2();

			GetActor("GolfRule") << MsgObject(NULL, M983(0x7d), uid, 0, 0, 0, 0);
		}
		break;
	}

	case 0x65: // 645: 0x63
		if (Doc()->m_golfGame.gameType == 7)
			GOLFDOC()->m_skinsHoleWinner = packet.Decode4();

		GetActor("GolfRule") << MsgObject(NULL, M983(0x7e), 0, 0, 0, 0, 0);
		break;

	case 0x8f: // 645: 0x8d
	{
		unsigned long uid = packet.Decode4();

		if (Doc()->AreWePlayingTogether(uid))
			GetActor("GolfRule") << MsgObject(NULL, M983(0x83), 0, 0, 0, 0, 0);
		break;
	}

	case 0x64: // 645: 0x62
	{
		sShotResultData result;
		packet.DecodeBuffer(&result, sizeof(result));

		if (Doc()->AreWePlayingTogether(result.uid))
		{
			unsigned char index =
				IsMassGame() ? 0 : Doc()->GetIndex(result.uid);

			*(sShotResultData*)&GOLFDOC()->m_shotResultMsg[index] = result;
			GetActor("GolfRule") << MsgObject(NULL, M983(0x79), 0, 0, 0, 0, 0);
		}
		break;
	}

	case 0x6c: // 645: 0x6a
	{
		if (!IsMassGame())
			break;

		unsigned long uid = packet.Decode4();
		unsigned char state = packet.Decode1();
		sRivalData* pRival = &Doc()->m_rivalList[Doc()->GetIndex(uid)];

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		switch (state)
		{
		case 3:
			for (std::vector<sRivalData>::iterator it =
					 Doc()->m_rivalList.begin();
				it != Doc()->m_rivalList.end(); ++it)
			{
				if ((*it).state != 3)
					pRival->quitOrder--;
			}

			GetActor("Score")
				<< MsgObject(NULL, M983(0x133), pRival->hole - 1, 0, 0, 0, 0);
			break;

		case 2:
			for (std::vector<sRivalData>::iterator it =
					 Doc()->m_rivalList.begin();
				it != Doc()->m_rivalList.end(); ++it)
			{
				if ((*it).state == 0)
					pRival->finishOrder--;
			}

			if (uid ==
				((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
						? Doc()->m_myInfo.info.dwGalleryGuid
						: Doc()->m_myInfo.info.dwGuid))
			{
				if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) > 1) // 983 006a02d3: fora do Grand Prix (doc+0x109b8)
					GOLFDOC()->m_bNetGameEnd = true;
				GOLFDOC()->m_bApproach794 = false;
				if (CMessengerInfo::Instance()->GetMyStatus() != 3)
					CMessengerInfo::Instance()->SetMyStatus(
						(MSN::eStatusType)4);
			}
			break;
		}

		pRival->state = state;
		if (state == 3 && (unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2)
		{
			if (AllOtherRivalsSettled983(pRival->hole, pRival->oid))
				GetActor("GolfRule") << MsgObject(NULL, 0xc9, 0, 0, 0, 0, 0);
			GetActor("Score") << MsgObject(NULL, 0x171, Doc()->GetIndex(pRival->oid), 0, 0, 0, 0);
		}
		GetActor("Score") << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
		break;
	}

	case 0x6d: // 645: 0x6b
	{
		if (!IsMassGame())
			break;

		unsigned long uid = packet.Decode4();
		unsigned char hole = packet.Decode1();
		unsigned char stroke = packet.Decode1();
		int score = packet.Decode4();
		__int64 pang = packet.Decode8();
		__int64 bonusPang = packet.Decode8();
		int bHoleOut = packet.Decode1();

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		Doc()->LogGstUpdateHole(uid, hole, score, pang);

		std::vector<sRivalData>::iterator end = Doc()->m_rivalList.end();
		std::vector<sRivalData>::iterator it =
			std::find_if(Doc()->m_rivalList.begin(), end, sFindRival(uid));
		if (it == end)
			break;

		if (bHoleOut == 1)
		{
			(*it).holeStroke[hole - 1] = stroke - (*it).totalStroke;
			(*it).holeScore[hole - 1] = score - (*it).totalScore;
			(*it).holePang[hole - 1] = pang - (*it).totalPang;
			(*it).totalStroke = stroke;
			(*it).totalScore = score;
			(*it).hole = Doc()->m_holeOrder[GetHoleIndex(hole)];
		}
		(*it).totalPang = pang;
		(*it).totalBonusPang = bonusPang;

		if (Doc()->m_golfGame.gameType == 6)
		{
			GetActor("GolfRule") << MsgObject(NULL, M983(0x5e), uid, hole, 0, 0, 0);

			if (Doc()->m_teamPlayerGuid[0] == uid ||
				Doc()->m_teamPlayerGuid[1] == uid)
				GetActor("GolfRule") << MsgObject(NULL, M983(0x9f), uid, 0, 0, 0, 0);
		}

		GetActor("Score") << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);

		if (bHoleOut != 1)
			break;

		GetActor("Score") << MsgObject(NULL, M983(0x136), hole, 0, 0, 0, 0);
		GetActor("Score") << MsgObject(NULL, M983(0x137), 0, 0, 0, 0, 0);
		GetActor("Spot") << MsgObject(NULL, M983(0xf9), Doc()->GetIndex(uid),
			(*it).hole, 0, 0, 0);

		// 983 (caso 0x6d, 006a08f3): Grand Prix (realGameType 0x14-0x15) - se os outros rivais ja encerraram, avisa o GolfRule (0xc9);
		// marca o rival (state 1) e manda 0x171/0x177/0x2b ao Score.
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2)
		{
			if (AllOtherRivalsSettled983((*it).hole, (*it).oid))
				GetActor("GolfRule") << MsgObject(NULL, 0xc9, 0, 0, 0, 0, 0);
			(*it).state = 1;
			AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x171, Doc()->GetIndex(uid), 0, 0, 0, 0);
			AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x177, Doc()->GetIndex(uid), 0, 0, 0, 0);
			AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x2b, 0, 0, 0, 0, 0);
		}
		break;
	}

	case 0x6e: // 645: 0x6c
	{
		// PKT (frente C, sessao C15): forma do 983 (caso 0x6e, 006a09ce) - dword novo depois da posicao (sRivalData+0x1a4),
		// modo 0xb (mensagem 0x129 ao Spot e 0x2b ao Score a cada 10 pacotes) e, no Grand Prix, aviso de bola do jogador
		// local perto do buraco (0x171, 0x177 ao Score)
		if (!IsMassGame())
			break;

		unsigned long uid = packet.Decode4();
		unsigned char shotType = packet.Decode1(); // 983: tambem e o buraco passado ao GetHoleData
		float pos[2];
		packet.DecodeBuffer(pos, sizeof(pos));
		unsigned long value983 = packet.Decode4();

		unsigned long distance = 0;
		int time = 0;
		if (Doc()->m_golfGame.gameType == 10)
		{
			distance = packet.Decode4();
			time = packet.Decode4();
		}
		unsigned short value = packet.Decode2();

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		unsigned char index = Doc()->GetIndex(uid);

		if (Doc()->m_golfGame.gameType == 10)
		{
			Doc()->m_rivalList[index].approachTime = time;
			Doc()->m_rivalList[index].approachResultDistance = distance;
			if (time == -1)
				Doc()->m_rivalList[index].approachDistance = -1;

			GetActor("Score")
				<< MsgObject(NULL, M983(0x2a), index == MyIndex(), 0, 0, 0, 0);
			GetActor("Spot") << MsgObject(NULL, M983(0xf8), index, shotType, value, 0, 0);
		}
		else if (Doc()->m_golfGame.gameType == 11)
		{
			static int s_count983; // 00e78fa8 (so este caso usa)
			if (s_count983 % 10 == 0)
				GetActor("Score") << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
			s_count983++;
			GetActor("Spot") << MsgObject(NULL, 0x129, index, value, 0, 0, 0);
		}
		else
			GetActor("Spot") << MsgObject(NULL, M983(0xf8), index, shotType, value, 0, 0);

		Doc()->m_rivalList[index].ballPos = WVector(pos[0], 0.0f, pos[1]);
		C_ASSERT(offsetof(sRivalData, shotSpecial983) == 0x1a4 && offsetof(sRivalData, state) == 0x36);
		Doc()->m_rivalList[index].shotSpecial983 = value983;

		if (IsGrandPrixGameType983(Doc()->m_roomInfo.realGameType) // 0046e6d0
			&& Doc()->m_myInfo.info.dwGuid == uid)
		{
			float CourseScale983_00470040(float value); // 00470040: medida na escala do campo (corpo na jp/pktviews983.cpp, C68)
			WVector diff = Doc()->m_rivalList[index].ballPos - Doc()->m_pGolfDoc->GetHoleData(shotType).pin;
			diff.y = 0.0f;
			float radius = GolfBall().m_radius;
			if (diff.Magnitude() < CourseScale983_00470040(radius) * 2.0f)
			{
				AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x171, index, 0, 0, 0, 0);
				AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x177, index, 0, 0, 0, 0);
				Doc()->m_rivalList[index].state = 1;
			}
		}
		break;
	}

	case 0x79: // 645: 0x77
		// PKT (frente C, sessao C15): forma do 983 (caso 0x79, 006a0de8) - modo 0xb sem dados proprios, guilda so com bonus e
		// pontos, bloco de 0x18 de +0x106e4 fora dos modos 6 e 0xa, Grand Prix sem a mensagem de aproximacao
		if (Doc()->m_holeStatistics[0].dwExp == -1)
		{
			if (Doc()->m_golfGame.gameType == 10)
			{
				unsigned long uid = packet.Decode4();
				if (uid != -1)
				{
					std::map<unsigned long, sBriefUserInfo>::iterator it =
						Doc()->m_briefUserInfoMap.find(uid);
					if (it != Doc()->m_briefUserInfoMap.end())
					{
						WSendPacket send((enumClientPacket)0x2f);
						send.Encode4(it->second.dwUid);
						send.Encode1(5);
						send.Send(TO_GAME);
					}
				}
			}
			else if (Doc()->m_golfGame.gameType != 11)
			{
				Doc()->m_holeStatistics[0].dwExp = packet.Decode4();
				Doc()->m_roomType = packet.Decode4();
				Doc()->m_eventType = packet.Decode1();
				Doc()->m_eventFlag = packet.Decode1();

				if (Doc()->m_golfGame.gameType == 6)
				{
					Doc()->m_guildBonusPang = packet.Decode4();
					Doc()->m_guildPoint = packet.Decode4();
				}
			}

			if (Doc()->m_golfGame.gameType != 11)
				packet.DecodeBuffer(Doc()->m_awardItem, sizeof(Doc()->m_awardItem));
			if (Doc()->m_golfGame.gameType != 6 && Doc()->m_golfGame.gameType != 10)
				packet.DecodeBuffer(&Doc()->m_guildRoundScore, 0x18);
		}

		if (Doc()->m_golfGame.gameType == 10)
			break;
		if (IsGrandPrixGameType983(Doc()->m_roomInfo.realGameType)) // 0046e6d0
			break;

		if (GOLFDOC()->m_bApproach794)
			GetActor("GolfRule") << MsgObject(NULL, M983(0x8f), 1, 0, 0, 0, 0);
		break;

	case 0x61: // 645: 0x5f
	{
		unsigned long uid = packet.Decode4();

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		std::map<unsigned long, sRoomSlot>::iterator itRoom =
			Doc()->m_roomSlotMap.find(uid);
		if (itRoom != Doc()->m_roomSlotMap.end())
		{
			if ((*itRoom).second.pExhibition)
			{
				delete (*itRoom).second.pExhibition;
				(*itRoom).second.pExhibition = NULL;
			}
			Doc()->m_roomSlotMap.erase(itRoom);
		}

		std::list<sSlotInfo>* pList = &Doc()->m_slotList;
		std::list<sSlotInfo>::iterator itSlot;
		itSlot = std::find(pList->begin(), pList->end(), uid);
		if (itSlot != pList->end())
		{
			unsigned char slot = (*itSlot).connectionRank;
			pList->erase(itSlot);

			for (std::list<sSlotInfo>::iterator it = pList->begin();
				it != pList->end(); it++)
			{
				if ((*it).connectionRank > slot)
					(*it).connectionRank--;
			}
		}

		if (!IsMassGame())
			GetActor("GolfRule") << MsgObject(NULL, M983(0x82), uid, 0, 0, 0, 0);
		break;
	}

	case 0x7c: // 645: 0x7a
	{
		std::list<sSlotInfo>* pList = &Doc()->m_slotList;
		unsigned long uid = packet.Decode4();
		packet.Decode2();

		for (std::list<sSlotInfo>::iterator it = pList->begin();
			it != pList->end(); it++)
		{
			(*it).bMaster = (*it).dwGuid == uid;
		}
		break;
	}

	case 0x7e: // 645: 0x7c
		Doc()->m_lobbyExitReason = packet.Decode2();
		m_exitDelay = 5.0f;
		break;

	case 0x60: // 645: 0x5e
	{
		char holeInfo[12];
		packet.DecodeBuffer(holeInfo, sizeof(holeInfo));

		GetActor("GolfRule")
			<< MsgObject(NULL, M983(0x8d), (int)holeInfo, 0, 0, 0, 0);
		break;
	}

	case 0x8a: // 645: 0x88
		GetActor("GolfRule") << MsgObject(NULL, M983(0x93), 0, 0, 0, 0, 0);
		break;

	case 0x8b: // 645: 0x89
	{
		unsigned long uid = packet.Decode4();
		int bPause = packet.Decode1();

		if (bPause)
		{
			CChatMsg::Instance()->AddChatMsg(
				MakeStr(
					K2L_Compatibility("%s\xb4\xd4\xc0\xcc \xb0\xd4\xc0\xd3\xc0\xbb "
					"\xc0\xe1\xbd\xc3 "
					"\xc1\xdf\xc1\xf6\xbd\xc3\xc4\xd7\xbd\xc0\xb4\xcf\xb4\xd9."),
					Doc()->m_userInfo[Doc()->GetIndex(uid)].info.sNick),
				0xffff7878, false, false);
		}
		else
		{
			CChatMsg::Instance()->AddChatMsg(
				MakeStr(K2L_Compatibility("%s\xb4\xd4\xc0\xcc \xc1\xdf\xc1\xf6\xb5\xc8 "
						"\xb0\xd4\xc0\xd3\xc0\xbb "
						"\xc0\xe7\xb0\xb3\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
					Doc()->m_userInfo[Doc()->GetIndex(uid)].info.sNick),
				0xffff7878, false, false);
		}

		GetActor("GolfRule") << MsgObject(NULL, M983(0x84), bPause,
			(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xc0\xcc \xc0\xe1\xbd\xc3 "
				 "\xc1\xdf\xb4\xdc\xb5\xc8 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9.\n"
				 "\xb0\xd4\xc0\xd3\xc0\xbb \xb0\xe8\xbc\xd3\xc7\xcf\xb1\xe2 "
				 "\xc0\xa7\xc7\xd8\xbc\xad\xb4\xc2 \xc8\xae\xc0\xce\xc0\xbb "
				 "\xb4\xad\xb7\xaf\xc1\xd6\xbc\xbc\xbf\xe4."),
			0, 0, 0);
		break;
	}

	case 0x8c: // 645: 0x8a
	{
		// 983 (0069eef0, caso 0x8c): tudo sob gameType != 0xb (Hole-in-One)
		if (Doc()->m_golfGame.gameType == GAME_TYPE_HOLEINONE983)
			break;
		if (CMessengerInfo::Instance()->GetMyStatus() != 3)
			CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)4);

		unsigned char order = Doc()->m_rivalList.size();

		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); ++it)
		{
			if ((*it).state != 0)
				continue;

			(*it).finishOrder = order;
			(*it).state = 2;
			(*it).holePang[(*it).hole - 1] = 0;
			(*it).holeStroke[(*it).hole - 1] =
				Doc()->m_holePar[(*it).hole - 1] + 5;
			(*it).holeScore[(*it).hole - 1] = 5;
			(*it).totalScore += 5;
			order--;

			while (
				(*it).hole != Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1])
			{
				(*it).hole = Doc()->m_holeOrder[GetHoleIndex((*it).hole)];
				(*it).holePang[(*it).hole - 1] = 0;
				(*it).holeStroke[(*it).hole - 1] =
					Doc()->m_holePar[(*it).hole - 1] + 5;
				(*it).holeScore[(*it).hole - 1] = 5;
				(*it).totalScore += 5;
			}
		}
		break;
	}

	case 0x84: // 645: 0x82
	{
		unsigned char flags = packet.Decode1();
		unsigned char bGM = flags >> 7;
		flags &= 0x7f;
		bool bSent = flags == 0;
		unsigned long color = 0xffd2a2ff;
		if (bGM)
			color = 0xff6eecfb;

		std::string name;
		std::string text;
		std::string msg;
		packet.DecodeStr(name);
		packet.DecodeStr(msg);

		if (bSent)
		{
			text = Doc()->m_myInfo.info.sNick;
			text += ">";
			text += name;
		}
		else
		{
			if (!COption::Instance()->gIsWhisperAllowed())
				break;

			std::list<std::string>::iterator ignoreIt = std::find(
				Doc()->m_ignoreList.begin(), Doc()->m_ignoreList.end(), name);
			if (ignoreIt != Doc()->m_ignoreList.end())
				break;

			unsigned long dummy;
			GetActor("GolfRule")
				<< MsgObject(NULL, M983(0x3f), (int)&dummy, 0, 0, 0, 0);
			GetActor("GolfRule")
				<< MsgObject(NULL, M983(0x38), (int)name.c_str(), 0, 0, 0, 0);
			text = name + ">" + Doc()->m_myInfo.info.sNick;
		}

		Doc()->m_chatManager.FilteringHack(msg.c_str(), false);
		text += " : \b'";
		text += msg;
		text += "'";
		CChatMsg::Instance()->AddChatMsg(text, color, false, true);
		break;
	}

	case 0x48: // 645: 0x46
	{
		std::list<sSlotInfo>* pList = &Doc()->m_slotList;
		unsigned char sub = packet.Decode1();
		packet.Decode2();

		switch (sub)
		{
		case 0:
		case 5:
		{
			if (sub == 0)
				pList->clear();

			unsigned char count = packet.Decode1();
			sSlotInfo slotInfo;
			sCharacterInfo charInfo;
			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&slotInfo, sizeof(slotInfo));
				pList->push_back(slotInfo);
				packet.DecodeBuffer(&charInfo, sizeof(charInfo));
				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
					RequestUccInfo(charInfo);
			}
			break;
		}

		case 1:
		{
			sSlotInfo slotInfo;
			packet.DecodeBuffer(&slotInfo, sizeof(slotInfo));

			if (!IsMassGame())
			{
				sRoomSlot roomSlot;
				sCharacterInfo charInfo;
				packet.DecodeBuffer(&charInfo, sizeof(charInfo));
				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
					RequestUccInfo(charInfo);

				roomSlot.charInfo = charInfo;
				Doc()->m_roomSlotMap[slotInfo.dwGuid] = roomSlot;
			}

			std::list<sSlotInfo>::iterator it;
			it = std::find(pList->begin(), pList->end(), slotInfo.dwGuid);
			if (it == pList->end())
				pList->push_back(slotInfo);
			break;
		}

		case 2:
		{
			unsigned long guid = packet.Decode4();

			std::map<unsigned long, sRoomSlot>::iterator slotIt =
				Doc()->m_roomSlotMap.find(guid);
			if (slotIt != Doc()->m_roomSlotMap.end())
			{
				if ((*slotIt).second.pExhibition)
				{
					delete (*slotIt).second.pExhibition;
					(*slotIt).second.pExhibition = NULL;
				}
				Doc()->m_roomSlotMap.erase(slotIt);
			}

			std::list<sSlotInfo>::iterator it;
			it = std::find(pList->begin(), pList->end(), guid);
			if (it != pList->end())
			{
				unsigned char rank = (*it).connectionRank;
				pList->erase(it);

				for (it = pList->begin(); it != pList->end(); it++)
				{
					if ((*it).connectionRank > rank)
						(*it).connectionRank--;
				}
			}
			break;
		}

		case 3:
		{
			sSlotInfo slotInfo;
			unsigned long guid = packet.Decode4();
			packet.DecodeBuffer(&slotInfo, sizeof(slotInfo));

			std::list<sSlotInfo>::iterator it;
			it = std::find(pList->begin(), pList->end(), guid);

			std::map<unsigned long, sRoomSlot>::iterator slotIt =
				Doc()->m_roomSlotMap.find(guid);
			if (slotIt != Doc()->m_roomSlotMap.end())
			{
				(*slotIt).second.bGachaWing = slotInfo.angelicWingsEffect;
				(*slotIt).second.bAngelWing = slotInfo.angelicWings;
			}
			break;
		}
		}

		Doc()->m_roomInfo.nUserNum = pList->size();
		break;
	}

	case 0x78: // 645: 0x76
	{
		unsigned long uid = packet.Decode4();
		int state = packet.Decode1();

		std::list<sSlotInfo>::iterator it =
			std::find(Doc()->m_slotList.begin(), Doc()->m_slotList.end(), uid);
		if (it != Doc()->m_slotList.end())
			(*it).bReady = state == 0;
		break;
	}

	case 0x90: // 645: 0x8e
		GetActor("GolfRule") << MsgObject(NULL, M983(0x94), 0, 0, 0, 0, 0);
		break;

	case 0x9e: // 645: 0x9c
	{
		unsigned char weather = packet.Decode1();
		packet.Decode1();
		packet.Decode1();
		GetActor("GolfBg") << MsgObject(NULL, M983(0x142), weather, 0, 0, 0, 0);
		break;
	}

	case 0x66: // 645: 0x64
	case 0x91: // 645: 0x8f
	case 0xb3: // 645: 0xb1
		if (!GOLFDOC()->m_bNetGameEnd)
		{
			if (CMessengerInfo::Instance()->GetMyStatus() != 3)
				CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)4);

			unsigned char count = packet.Decode1();

			if (Doc()->m_golfGame.gameType == 7)
			{
				GOLFDOC()->m_skinsHoleWinner = packet.Decode4();
				GOLFDOC()->m_skinsPenaltyUid = packet.Decode4();
			}

			for (int i = 0; i < count; i++)
			{
				sHoleResultData result;
				packet.DecodeBuffer(&result, sizeof(result));

				if (Doc()->AreWePlayingTogether(result.uid))
				{
					((sHoleResultData*)GOLFDOC()
							->m_holeResult983)[Doc()->GetIndex(result.uid)] =
						result;
				}
			}

			if (type == 0x91 /* 645: 0x8f */)
			{
				TEAM(0)->score = packet.Decode1();
				TEAM(1)->score = packet.Decode1();
				eTeamWinner winner = (eTeamWinner)packet.Decode1();
				*(eTeamWinner*)&GOLFDOC()->m_teamNum = winner;
			}
		}

		GetActor("GolfRule") << MsgObject(NULL, M983(0x80), 0, 0, 0, 0, 0);
		break;

	case 0xab: // 645: 0xa9
	{
		// PKT (frente C, sessao C15): ramo novo do 983 (caso 0xab, 006a14c9) com o conteudo local 0x92
		if (IsLocalContent((localContentType_t)0x92))
		{
			unsigned char index = packet.Decode1();
			unsigned long value = packet.Decode4();
			C_ASSERT_VC10(offsetof(CSharedDoc, m_holePrizeItem983) == 0x1145c && offsetof(CSharedDoc, m_holePrizeXmas983) == 0x11468);
			Doc()->m_holePrizeXmas983[index] = packet.Decode1() ? 1 : 0;
			Doc()->m_holePrizeItem983[index] = value;
			break;
		}
		unsigned char count = packet.Decode1();
		for (int i = 0; i < count; i++)
		{
			unsigned long uid = packet.Decode4();
			sWinningPrize prize;
			packet.DecodeBuffer(&prize, sizeof(prize));
			Doc()->SetWinningPrize(uid, prize);
		}
		break;
	}

	case 0x92: // 645: 0x90
		GetActor("GolfRule") << MsgObject(NULL, M983(0x95), 0, 0, 0, 0, 0);
		break;

	case 0xb0: // 645: 0xae
	{
		std::string name;
		std::string msg;
		std::string text;
		packet.DecodeStr(name);
		packet.DecodeStr(msg);

		if (COption::Instance()->gIsWhisperAllowed())
		{
			std::list<std::string>::iterator it = std::find(
				Doc()->m_ignoreList.begin(), Doc()->m_ignoreList.end(), name);
			if (it == Doc()->m_ignoreList.end())
			{
				Doc()->m_chatManager.FilteringHack(msg.c_str(), false);
				text = name + ">" + K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4");
				text += " : \b";
				text += msg;
				CChatMsg::Instance()->AddChatMsg(text, 0xff00c6ff, false, true);
			}
		}
		break;
	}

	case 0x94: // 645: 0x92
		GOLFDOC()->m_bStopRequest = packet.Decode1() == 1;

		if (!GOLFDOC()->IsReported())
			break;

		if (GOLFDOC()->m_bStopRequest)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xbf\xa1 \xb4\xeb\xc7\xd8\xbc\xad \xc0\xcc\xb9\xcc \xbd\xc5\xb0\xed\xb0\xa1 \xc1\xa2\xbc\xf6\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 4);
			GOLFDOC()->m_bReport = false;
		}
		else
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xbd\xc5\xb0\xed\xb0\xa1 \xc1\xa4\xbb\xf3\xc0\xfb\xc0\xb8\xb7\xce \xc1\xa2\xbc\xf6 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.\n\n\xc6\xaf\xc1\xa4 \xbb\xf3\xb4\xeb\xc0\xc7 \xb4\xeb\xc8\xad\xb8\xa6 \xc2\xf7\xb4\xdc \xc7\xcf\xb7\xc1\xb8\xe9 \xbb\xf3\xb4\xeb \xc1\xa4\xba\xb8\xc3\xa2\xc0\xbb \xb8\xb6\xbf\xec\xbd\xba \xb4\xf5\xba\xed \xc5\xac\xb8\xaf \xc8\xc4 \\c0xffff0000\\c[\xb4\xeb\xc8\xad \xc2\xf7\xb4\xdc \xbe\xc6\xc0\xcc\xc4\xdc]\\c0xff000000\\c\xc0\xbb \xbc\xb1\xc5\xc3 \xc7\xcf\xbd\xc3\xb8\xe9 \xb5\xcb\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 0);
		}
		break;

	case 0x9c: // 645: 0x9a
	{
		// 983 (006a1b25): esvazia a lista antes de encher
		GOLFDOC()->m_inputList983.clear();
		unsigned char count = packet.Decode1();
		for (unsigned char i = 0; i < count; i++)
		{
			unsigned long id = packet.Decode4();
			GOLFDOC()->m_inputList983.push_back(id);
		}

		if (OnlinePlay())
			GetActor("Cutin") << MsgObject(NULL, 0x30c, 8, 0, 0, 0, 0); // 983: 0x30c (o 645 mandava M983(0x78))
		break;
	}

	case 0x99: // 645: 0x97
		if (!(GOLFDOC()->m_gameMode & 0x30e3))
		{
			unsigned char count = packet.Decode1();
			std::vector<unsigned long> uidList;
			uidList.reserve(count);

			for (unsigned char n = 0; n < count; n++)
			{
				unsigned long uid = packet.Decode4();
				uidList.push_back(uid);
			}

			WSendPacket send((enumClientPacket)0x40);
			sPlayerData* pPlayer = PLAYER(0);

			sGalleryPlayerData data;
			data.club = GolfClub().GetIndex();
			data.frame = GetPVS().GetFrame();
			data.angle = GOLFDOC()->m_cameraData.angle;
			data.bunker = pPlayer->bunker;
			data.gauge = (float)float2int(pPlayer->gauge);
			data.itemNum = pPlayer->itemNum;
			data.item = pPlayer->item;
			data.flag = pPlayer->flag;
			data.startPos.x = pPlayer->startPos.x;
			data.startPos.y = pPlayer->startPos.y;
			data.startPos.z = pPlayer->startPos.z;
			data.pos.x = pPlayer->pos.x;
			data.pos.y = pPlayer->pos.y;
			data.pos.z = pPlayer->pos.z;
			data.powerShot = GolfClub().PowerShotType();
			data.bonusPang = pPlayer->bonusPang;
			for (int i = 0; i < 18; i++)
			{
				data.stroke[i] = pPlayer->stroke[i];
				data.putt[i] = pPlayer->putt[i];
				data.pang[i] = pPlayer->pang[i];
			}

			if (IsMassGame())
				data.state = GolfBall().m_playFrame != -1;
			else
				data.state = pPlayer->state;

			send.EncodeBuffer(&data, sizeof(data));
			send.Encode1(count);
			for (unsigned char k = 0; k < count; k++)
				send.Encode4(uidList[k]);
			send.Send(TO_GAME);
			break;
		}
		break;

	case 0xbe: // 645: 0xbc
		GetActor("GolfRule") << MsgObject(NULL, M983(0x9e), 1, 0, 0, 0, 0);
		break;

	case 0xbc: // 645: 0xba
	{
		if (GalleryMode())
		{
			unsigned char result = packet.Decode1();
			if (result == 0xb)
			{
				CChatMsg::Instance()->AddChatMsg(
					K2L_Compatibility("\xbe\xcb\xb8\xb2 : \xb5\xd1\xb7\xaf\xba\xb8\xb1\xe2\xb0\xa1 \xb0\xa1\xb4\xc9\xc7\xd1 \xb0\xd4\xc0\xd3\xc1\xdf\xc0\xce \xb9\xe6\xc0\xbb \xc3\xa3\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
					0xffff7878, false, false);
				break;
			}
			else if (result == 0x10)
			{
				CChatMsg::Instance()->AddChatMsg(
					K2L_Compatibility("\xbe\xcb\xb8\xb2 : \xb7\xce\xb5\xf9\xb5\xc7\xb1\xe2 \xc0\xfc\xbf\xa1 \xb0\xd4\xc0\xd3\xc0\xcc \xc1\xbe\xb7\xe1\xb5\xc9 \xbf\xec\xb7\xc1\xb0\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
					0xffff7878, false, false);
				break;
			}
			else
			{
				packet.DecodeBuffer(&Doc()->m_roomInfo, sizeof(sRoomInfo));
				Doc()->m_expRate = packet.Decode4();

				unsigned char slotNum = packet.Decode1();
				Doc()->m_slotList.clear();
				unsigned char i;
				for (i = 0; i < slotNum; i++)
				{
					sSlotInfo slotInfo;
					packet.DecodeBuffer(&slotInfo, sizeof(slotInfo));
					Doc()->m_slotList.push_back(slotInfo);
				}

				if (IsMassGame(Doc()->m_roomInfo.gameType))
				{
					packet.DecodeBuffer(&Doc()->m_userInfo[0],
						sizeof(sUserInfo));
					if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
						RequestUccInfo(Doc()->m_userInfo[0]);
					Doc()->m_myInfo.info.dwGalleryGuid =
						Doc()->m_userInfo[0].info.dwGuid;
				}
				else
				{
					if (slotNum > 3)
						break;

					for (i = 0; i < slotNum; i++)
					{
						packet.DecodeBuffer(&Doc()->m_userInfo[i],
							sizeof(sUserInfo));
						if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
							RequestUccInfo(Doc()->m_userInfo[i]);
					}
				}

				SetCurMap(packet.Decode1());
				SetGameType(packet.Decode1());
				SetHoleType(packet.Decode1());
				SetHoles(packet.Decode1());
				Doc()->m_roomType = packet.Decode4();
				SetShotTimeLimit(packet.Decode4());
				SetGameTimeLimit(packet.Decode4());

				for (unsigned char hole = 0; hole < GetHoles(); hole++)
				{
					// PKT (frente C, sessao C15): 983 (caso 0xbc, 006a2130) - o mapa do buraco vem no pacote, nao e mais % 3
					Doc()->m_holeRandom[hole] = packet.Decode4();
					Doc()->m_courseMap[hole] = packet.Decode1();
					Doc()->m_holeOrder[hole] = packet.Decode1();
				}

				GOLFDOC()->m_currentHole = packet.Decode1();
				Doc()->m_roomSetupStage = 0;

				if (AfxGetTask()->IsInited())
				{
					AfxGetTask()->SetInited(false);
					AfxGetTask()->Reload();
				}

				GetActor("GolfBg") << MsgObject(NULL, M983(0x140), 0, 0, 0, 0, 0);
				GetActor("GolfBg") << MsgObject(NULL, M983(0x13e), 0, 0, 0, 0, 0);
				GetActor("Screen") << MsgObject(NULL, M983(0x49), 0, 0, 0, 0, 0);
				break;
			}
		}
		else
		{
			sSlotInfo slotInfo;
			packet.DecodeBuffer(&slotInfo, sizeof(slotInfo));
			Doc()->m_slotList2.clear();
			Doc()->m_slotList2.push_back(slotInfo);
			Doc()->m_roomInfo.nGalleryNum = 1;
			GetActor("GolfRule") << MsgObject(NULL, M983(0x9e), 0, 0, 0, 0, 0);
			IsMyTurn(true);
			break;
		}
		break;
	}

	case 0xbd: // 645: 0xbb
		if (GalleryMode())
		{
			unsigned char weather = packet.Decode1();
			packet.Decode1();
			packet.Decode1();
			GetActor("GolfBg") << MsgObject(NULL, M983(0x142), weather, 1, 0, 0, 0);

			unsigned char windPower = packet.Decode1();
			unsigned char windAngle = packet.Decode1();
			Wind().Init(windPower, windAngle, 0, 1);

			unsigned long remainTime = Doc()->m_golfGame.shotTimeLimit;
			remainTime -= packet.Decode4();
			unsigned long currentUid = packet.Decode4();
			unsigned char playerNum = packet.Decode1();

			for (unsigned char index = 0; index < playerNum; index++)
			{
				sGalleryPlayerData data;
				packet.DecodeBuffer(&data, sizeof(data));

				PLAYER(index)->bunker = data.bunker;
				PLAYER(index)->gauge = data.gauge;
				PLAYER(index)->itemNum = data.itemNum;
				PLAYER(index)->item = data.item;
				PLAYER(index)->windAbility983 = data.windAbility983;	// 983 006a0f.. (caso 0xbd): dword novo do bloco -> jogador+0x4c0
				PLAYER(index)->flag = data.flag;
				PLAYER(index)->startPos.x = data.startPos.x;
				PLAYER(index)->startPos.y = data.startPos.y;
				PLAYER(index)->startPos.z = data.startPos.z;
				PLAYER(index)->pos.x = data.pos.x;
				PLAYER(index)->pos.y = data.pos.y;
				PLAYER(index)->pos.z = data.pos.z;
				for (unsigned char i = 0; i < 18; i++)
				{
					PLAYER(index)->stroke[i] = data.stroke[i];
					PLAYER(index)->putt[i] = data.putt[i];
					PLAYER(index)->pang[i] = (int)data.pang[i];
				}

				PLAYER(index)->oid = Doc()->m_userInfo[index].info.dwGuid;
				PLAYER(index)->teeOrder = 0;
				PLAYER(index)->score = data.score;
				PLAYER(index)->totalStroke = data.totalStroke;
				PLAYER(index)->totalPang = (int)data.totalPang;
				PLAYER(index)->bonusPang = (int)data.bonusPang;
				PLAYER(index)->index = index;
				PLAYER(index)->holeNum =
					GetHoleIndex(GOLFDOC()->m_currentHole) - 1;
				PLAYER(index)->state = data.state;

				if (PLAYER(index)->oid == currentUid)
				{
					GOLFDOC()->m_currentPlayer = index;
					GolfBall().m_pos = PLAYER(index)->pos;
					GolfBall().m_meshIndex =
						GetPVS().GetMeshIndex(GolfBall().m_pos, false);
					GameCamera().angle = data.angle;
					GolfClub().SetApproachMode();
					GolfClub().SetClub(data.club);
					GolfClub().SetPowerShot(data.powerShot);
					GetPVS().SetFrame(data.frame);
				}
			}

			GetActor("GolfRule") << MsgObject(NULL, M983(0x78), 0x100000, 0, 0, 0, 0);
			GOLFDOC()->m_remainTime = remainTime;
			break;
		}
		break;

	case 0x98: // 645: 0x96
		if (GalleryMode())
		{
			if (!IsMassGame())
			{
				Doc()->m_myInfo.info.dwGalleryGuid = packet.Decode4();
				break;
			}

			packet.DecodeBuffer(&Doc()->m_userInfo[0], sizeof(sUserInfo));
			if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
				RequestUccInfo(Doc()->m_userInfo[0]);

			IFF_STRUCT::sCaddie* pCaddie =
				ItemManager()->FindCaddie(Doc()->m_userInfo[0].caddieInfo.tid);
			if ((pCaddie &&
					Doc()->m_userInfo[0].stat.Level < pCaddie->c.Level) ||
				(Doc()->m_userInfo[0].caddieInfo.Rent_flag &&
					Doc()->m_userInfo[0].caddieInfo.Remain_Date == 0 &&
					pCaddie->MonthlyFee > 0))
			{
				Doc()->m_userInfo[0].caddieInfo.tid = 0;
			}

			IFF_STRUCT::sClubSet* pClubSet =
				ItemManager()->FindClubSet(Doc()->m_userInfo[0].clubInfo.tid);
			if (pClubSet)
			{
				sItemLevel level = *(sItemLevel*)&pClubSet->c.Icon[-1];
				if ((level.Level > Doc()->m_userInfo[0].stat.Level &&
						!level.IsUnderLvl) ||
					(Doc()->m_userInfo[0].stat.Level > level.Level &&
						level.IsUnderLvl))
				{
					Doc()->m_userInfo[0].clubInfo.tid = 0x10000000;
				}
			}

			unsigned long itemSlot[10];
			memcpy(itemSlot, Doc()->m_userInfo[0].userEquip.tidItemSlot,
				sizeof(itemSlot));

			unsigned char slotNum = 0;
			for (unsigned char i = 0; i < 10; i++)
			{
				if (itemSlot[i])
					Doc()->m_userInfo[0].userEquip.tidItemSlot[slotNum++] =
						itemSlot[i];
			}
			for (; slotNum < 10; slotNum++)
				Doc()->m_userInfo[0].userEquip.tidItemSlot[slotNum] = 0;

			Doc()->m_myInfo.info.dwGalleryGuid =
				Doc()->m_userInfo[0].info.dwGuid;
			if (CHeadIcon::Instance())
				CHeadIcon::Instance()->SetLevel();

			unsigned char hole = packet.Decode1();
			unsigned char weather = packet.Decode1();
			packet.Decode1();
			packet.Decode1();
			unsigned char windPower = packet.Decode1();
			unsigned char windAngle = packet.Decode1();
			unsigned char windType = packet.Decode1();

			sGalleryPlayerData data;
			packet.DecodeBuffer(&data, sizeof(data));

			GolfClub();
			GetActor("Player") << MsgObject(NULL, M983(0x49), 0, 0, 0, 0, 0);
			GetActor("Screen") << MsgObject(NULL, M983(0x49), 0, 0, 0, 0, 0);
			GetActor("Player") << MsgObject(NULL, M983(0x11c),
				Doc()->m_userInfo[0].clubInfo.tid, 0, 0, 0, 0);
			GetActor("Ball") << MsgObject(NULL, M983(0x4c),
				Doc()->m_userInfo[0].userEquip.tidBall, 0, 0, 0, 0);
			GetActor("Screen") << MsgObject(NULL, M983(0x4c),
				Doc()->m_userInfo[0].userEquip.tidBall, 0, 0, 0, 0);
			GetActor("Caddie")
				<< MsgObject(NULL, M983(0x4a), Doc()->m_userInfo[0].caddieInfo.tid,
					   Doc()->m_userInfo[0].caddieInfo.Level - 1,
					   Doc()->m_userInfo[0].caddieInfo.tidPart, 0, 0);
			GetActor("Mascot")
				<< MsgObject(NULL, M983(0x4e), Doc()->m_userInfo[0].mascotInfo.guid,
					   (int)&Doc()->m_userInfo[0].mascotInfo, 0, 0, 0);
			GOLFDOC()->SetPlayerLevel(0);
			GetActor("GolfBg") << MsgObject(NULL, M983(0x142), weather, 1, 0, 0, 0);
			GOLFDOC()->m_currentHole = hole;
			GetActor("GolfBg") << MsgObject(NULL, M983(0x1e8), 0, 0, 0, 0, 0);
			Wind().Init(windPower, windAngle, windType, 1);

			sPlayerData* pPlayer = PLAYER(0);
			pPlayer->bunker = data.bunker;
			pPlayer->gauge = data.gauge;
			pPlayer->itemNum = data.itemNum;
			pPlayer->item = data.item;
			pPlayer->windAbility983 = data.windAbility983;	// 983 006a0f.. (caso 0xbd): dword novo do bloco -> jogador+0x4c0
			pPlayer->flag = data.flag;
			pPlayer->startPos.x = data.startPos.x;
			pPlayer->startPos.y = data.startPos.y;
			pPlayer->startPos.z = data.startPos.z;
			pPlayer->pos.x = data.pos.x;
			pPlayer->pos.y = data.pos.y;
			pPlayer->pos.z = data.pos.z;
			for (unsigned char h = 0; h < 18; h++)
			{
				pPlayer->stroke[h] = data.stroke[h];
				pPlayer->putt[h] = data.putt[h];
				pPlayer->pang[h] = (int)data.pang[h];
			}

			GolfBall().m_pos = pPlayer->pos;
			GolfBall().m_meshIndex =
				GetPVS().GetMeshIndex(GolfBall().m_pos, false);

			switch (data.state)
			{
			case 0:
				GameCamera().angle = data.angle;
				GolfClub().SetApproachMode();
				break;

			case 2:
				GolfBall().m_pos = GetTeeShotPos();
				GolfBall().m_pos.y += GolfBall().m_radius;
				pPlayer->pos = GolfBall().m_pos;
				pPlayer->startPos = GolfBall().m_pos;

			case 1:
				GolfClub().SelectBestClub();
				GOLFDOC()->m_cameraData.angle = CalcDeltaAngle(
					GOLFDOC()->m_pAutoTarget->GetPoint(NULL) - GolfBall().m_pos,
					WVector::UNIT_POS_Z);
				break;
			}

			pPlayer->oid = Doc()->m_myInfo.info.dwGalleryGuid;
			pPlayer->teeOrder = 0;
			pPlayer->score =
				Doc()->m_rivalList[Doc()->GetIndex(pPlayer->oid)].totalScore;
			pPlayer->totalStroke =
				Doc()->m_rivalList[Doc()->GetIndex(pPlayer->oid)].totalStroke;
			pPlayer->totalPang = (int)data.pang[hole - 1] +
				Doc()->m_rivalList[Doc()->GetIndex(pPlayer->oid)].totalPang;
			pPlayer->bonusPang = (int)data.bonusPang;
			pPlayer->index =
				Doc()->m_rivalList[Doc()->GetIndex(pPlayer->oid)].order;
			pPlayer->holeNum = GetHoleIndex(hole) - 1;
			pPlayer->state = 0;

			GolfClub().SetClub(data.club);
			GolfClub().SetPowerShot(data.powerShot);

			GetActor("GolfRule") << MsgObject(NULL, M983(0x78), 0x100000, 0, 0, 0, 0);

			GetPVS().SetFrame(data.frame);
			break;
		}
		break;

	case 0x9b: // 645: 0x99
	{
		if (!IsMassGame())
			break;

		unsigned long uid = packet.Decode4();
		Doc()->m_rivalList[Doc()->GetIndex(uid)].observerState =
			packet.Decode4();
		break;
	}

	case 0x97: // 645: 0x95
		Doc()->m_roomType = packet.Decode4();
		break;

	case 0xa3: // 645: 0xa1
	{
		unsigned long uid = packet.Decode4();
		unsigned char percent = packet.Decode1();
		if (uid != Doc()->m_myInfo.info.dwGuid)
			UpdateLoadingInfo(uid, percent);
		break;
	}

	case 0xa4: // 645: 0xa2
		if (IsMyTurn(false))
		{
			unsigned short count = packet.Decode2();
			unsigned long typeId;
			if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
				typeId = packet.Decode4();
			else
				typeId = 0x1a000000;

			std::list<sItemInfo>::iterator it = std::find(
				Doc()->m_myItemList.begin(), Doc()->m_myItemList.end(), typeId);
			if (it != Doc()->m_myItemList.end())
			{
				(*it).Common[0] = count;
				if (count == 0)
					Doc()->m_myItemList.erase(it);
			}
		}

		GetActor("GolfRule") << MsgObject(NULL, M983(0x97), 0, 0, 0, 0, 0);
		break;

	case 0xae: // 645: 0xac
	{
		std::list<sSlotInfo>& slotList = Doc()->m_slotList2;

		switch (packet.Decode1())
		{
		case 2:
		{
			unsigned long uid = packet.Decode4();
			std::list<sSlotInfo>::iterator it;
			it = std::find(slotList.begin(), slotList.end(), uid);
			if (it != slotList.end())
				slotList.erase(it);
			break;
		}

		case 1:
		{
			sSlotInfo slot;
			packet.DecodeBuffer(&slot, sizeof(slot));
			std::list<sSlotInfo>::iterator it;
			it = std::find(slotList.begin(), slotList.end(), slot.dwGuid);
			if (it == slotList.end())
				slotList.push_back(slot);
			break;
		}

		case 0:
		{
			slotList.clear();

			unsigned char count = packet.Decode1();
			sSlotInfo slot;
			for (int i = count; i > 0; i--)
			{
				packet.DecodeBuffer(&slot, sizeof(slot));
				slotList.push_back(slot);
			}
			break;
		}
		}

		Doc()->m_roomInfo.nGalleryNum = slotList.size();
		break;
	}

	case 0xac: // 645: 0xaa
	{
		unsigned long uid = packet.Decode4();
		unsigned char chatBlock = packet.Decode1();
		if (uid != Doc()->m_myInfo.info.dwGuid &&
			Doc()->AreWePlayingTogether(uid))
			PLAYER(Doc()->GetIndex(uid))->chatBlock = chatBlock;
		break;
	}

	case 0xad: // 645: 0xab
	{
		std::string name;
		packet.DecodeStr(name);
		unsigned char channel = packet.Decode1();
		char buf[128];

		std::list<sChannelInfo>::iterator it = std::find(
			Doc()->m_channelList.begin(), Doc()->m_channelList.end(), channel);
		if (it == Doc()->m_channelList.end())
		{
			sprintf(buf,
				K2L_Compatibility("%s\xb4\xd4\xc0\xba [%s]\xbf\xa1\xbc\xad \xc3\xa3\xc0\xbb "
				"\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				name.c_str(), Doc()->m_curGameServer.name);
		}
		else
		{
			sprintf(buf,
				K2L_Compatibility("%s\xb4\xd4\xc0\xba [%s]\xbf\xa1 \xc1\xa2\xbc\xd3\xc7\xd8 "
				"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				name.c_str(), it->Name);
		}

		CChatMsg::Instance()->AddChatMsg(buf, 0xffff7878, false, false);
		break;
	}

	case 0xc2: // 645: 0xc0
	{
		unsigned long uid = packet.Decode4();
		Doc()->m_guildScore[0] = packet.Decode2();
		Doc()->m_guildScore[1] = packet.Decode2();
		unsigned char point = packet.Decode1();
		unsigned char rivalPoint = packet.Decode1();

		sRivalData* pData = &Doc()->m_rivalList[Doc()->GetIndex(uid)];
		sRivalData* pRival = Doc()->GetRival(uid);
		if (pData == NULL || pRival == NULL)
			break;

		GetActor("Score") << MsgObject(NULL, M983(0x57), Doc()->m_guildScore[0],
			Doc()->m_guildScore[1], 0, 0, 0);
		GetActor("Screen") << MsgObject(NULL, M983(0x1ce), Doc()->m_guildScore[0],
			Doc()->m_guildScore[1], 0, 0, 0);

		pData->guildPoint = point;
		pRival->guildPoint = rivalPoint;
		break;
	}

	case 0xcc: // 645: 0xca
	{
		sPrizeInfo prize[0x800 / sizeof(sPrizeInfo)];	// 983 006a4ae0: 0x80 elementos de 0x10 (era 0x600 no 645)
		memset(prize, 0, sizeof(prize));

		unsigned long uid = packet.Decode4();
		int count = packet.Decode1();
		if (count > 0)
		{
			packet.DecodeBuffer(prize, sizeof(prize));
			Doc()->SetExtPrize(uid, count, prize);
		}
		break;
	}

	case 0x14f: // 645: 0x147
	{
		char desc[128] = "\0";
		unsigned char missionType = packet.Decode1();
		unsigned char prizeType = 0;
		unsigned char winnerType = 0;

		if (missionType != 0)
		{
			prizeType = packet.Decode1();
			winnerType = packet.Decode1();
			unsigned long param1 = packet.Decode4();
			unsigned long param2 = packet.Decode4();
			std::string param("\0");
			packet.DecodeStr(param);

			CApproachDesc approach(missionType, param1, param2, param.c_str());
			const char* pDesc = approach.GetDesc();
			if (pDesc)
				strcpy(desc, pDesc);
		}

		GOLFDOC()->SetApproachMissionType((eApproachMissionType)missionType);
		GOLFDOC()->SetApproachPrizeType((eApproachPrizeType)prizeType);
		GOLFDOC()->SetApproachWinnerType((eApproachWinnerType)winnerType);
		GOLFDOC()->SetApproachMissionDesc(desc);
		break;
	}

	case 0x150: // 645: 0x148
	{
		unsigned long winner = -1;

		GOLFDOC()->ClearApproachCurHoleResult();

		unsigned char count = packet.Decode1();
		for (unsigned char i = 0; i < count; i++)
		{
			sApproachResultData result;
			packet.DecodeBuffer(&result, sizeof(result));

			if (result.byRank == 1)
				winner = result.dwGUID;

			if (result.dwGUID == Doc()->m_myInfo.info.dwGuid)
			{
				GOLFDOC()->AddApproachTotalPrize(result.dwPrizeCount);
				GOLFDOC()->AddApproachTotalPrize(result.byLuckPrize);
				GOLFDOC()->AddApproachTotalPrize(result.byRankPrize);
			}

			GOLFDOC()->SetApproachCurHoleResult(result);
		}

		if (winner != -1)
		{
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				Doc()->m_briefUserInfoMap.find(winner);
			if (it != Doc()->m_briefUserInfoMap.end())
			{
				WSendPacket send((enumClientPacket)0x2f);
				send.Encode4(it->second.dwUid);
				send.Encode1(5);
				send.Send(TO_GAME);
				Doc()->SetUserInfoMod(1);
			}
		}
		break;
	}

	case 0x14e: // 645: 0x146
	{
		GOLFDOC()->ClearApproachGameResult();

		unsigned char count = packet.Decode1();
		for (unsigned char i = 0; i < count; i++)
		{
			sApproachResultData result;
			packet.DecodeBuffer(&result, sizeof(result));
			GOLFDOC()->SetApproachGameResult(result);
		}
		break;
	}

	case 0x151: // 645: 0x149
	{
		if (CMessengerInfo::Instance()->GetMyStatus() != 3)
			CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)4);

		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); ++it)
		{
			if ((*it).state == 0)
				(*it).state = 2;
		}

		GetActor("GolfRule") << MsgObject(NULL, M983(0xa6), 0, 0, 0, 0, 0);
		Doc()->SetAppGiftDownloadState((CSharedDoc::eAppGiftDownloadState)2);
		break;
	}

	case 0x18d: // 645: 0x192
	{
		std::string texName;

		if (packet.Decode1() == 1)
		{
			CUTIN_ANI_STYPEINFO info;
			unsigned long skinId = packet.Decode4();
			packet.Decode4();
			packet.DecodeBuffer(&info, sizeof(info));

			if (COption::Instance()->gGetCutinFlag() == 1)
			{
				IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(skinId);
				if (pSkin == NULL) // 983 (006a3f65): sem a pele, sai sem aviso
					break;
				IFF_STRUCT::S5::sCutinInformation* pCutin =
					ItemManager()->FindCutinInformation(pSkin->c.TypeId);
				if (pCutin != NULL)
				{
					char name[64] = { 0 };
					texName = pCutin->Char_Tex;
					// 983 (006a3fad): aviso de sucesso a regra; "Cutin" 0x30c com 0xc (o 645 mandava M983(0x78) com 4)
					GetActor("GolfRule") << MsgObject(NULL, M983(0x24d), 0,
						(int)"GST_RESPONSE_CUTIN_SUCCESS", 0, 0, 0);
					GetActor("Cutin") << MsgObject(NULL, 0x30c, 0xc, 0, 0, 0, 0);

					strcmpi(texName.c_str(), info.szCharTex);
					sprintf(name, "%s", texName.c_str());

					GetActor("Cutin") << MsgObject(NULL, M983(0x24c), (int)name,
						(int)&info, pCutin->Condition, skinId, 0);
					break;
				}
			}
		}
		else
		{
			if (packet.Decode2() == 0)
			{
				GetActor("GolfRule") << MsgObject(NULL, M983(0x24d), 0,
					(int)"NON CUTIN_MODE", 0, 0, 0);
			}
			else
			{
				GetActor("GolfRule") << MsgObject(NULL, M983(0x24d), 0,
					(int)"GST_RESPONSE_CUTIN_FAILED", 0, 0, 0);
			}
			break;
		}

		GetActor("GolfRule") << MsgObject(NULL, M983(0x24d), 0,
			(int)"GST_RESPONSE_CUTIN_FAILED", 0, 0, 0);
		break;
	}

	// ---- PKT (frente C, sessao C15, lote 8): casos que so existem no 983 (numeros de pacote e de mensagem do 983) ----
	case 0x1ec:
	{
		// 983 006a4434: devolve os 8 bytes recebidos no pacote 0x12d
		double value = 0;
		packet.DecodeBuffer(&value, 8);
		WSendPacket send((enumClientPacket)0x12d);
		send.EncodeBuffer(&value, 8);
		send.Send((eSendTo)0);
		break;
	}

	case 0x1ee:
		// 983 006a424f
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 1, (int)&packet, 0, 0, 0);
		break;

	case 0x1f0:
		// 983 006a4291
		AfxGetTask()->GetActor("Effect") << MsgObject(NULL, 0x1ef, 1, 0, 0, 0, 0);
		break;

	case 0x1f1:
		// 983 006a42d3
		AfxGetTask()->GetActor("Effect") << MsgObject(NULL, 0x1ef, 0, 0, 0, 0, 0);
		break;

	case 0x1f2:
		// 983 006a42e5
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 0xc, 0, 0, 0, 0);
		break;

	case 0x1f3:
		// 983 006a4329
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xc5, 0, 0, 0, 0, 0));
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 0x18, (int)&packet, 0, 0, 0);
		AfxPostMsg(NULL, "Lobby", 0x2e, 0, 0, 0, 0);
		AfxPostMsg(NULL, "Lobby", 0xf, 0, 0, 0, 0);
		break;

	case 0x1f4:
	{
		// 983 006a43e8
		int value = packet.Decode4();
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 0xf, value, 0, 0, 0);
		break;
	}

	case 0x1f5:
		// 983 006a44a0
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 0x10, (int)&packet, 0, 0, 0);
		break;

	case 0x1f7:
	{
		// 983 006a3ac3: trajetoria da tacada de outro jogador no jogo em massa (bloco de 0x57), so no buraco atual
		unsigned long guid = packet.Decode4();
		unsigned char hole = packet.Decode1();
		unsigned char data[0x57];
		packet.DecodeBuffer(data, 0x57);
		if (Doc()->m_myInfo.info.dwGuid == guid)
			break;
		if (Doc()->m_pGolfDoc->m_currentHole != hole)
			break;
		GetActor("ShotTrajectoryForMassGame") << MsgObject(NULL, 0x312, (int)data, 0, 0, 0, 0);
		break;
	}

	case 0x200:
	{
		// 983 006a4147: diferenca entre o valor do servidor e o do Gateway (sub 0x17 devolve em milesimos); de 3 em diante
		// avisa a tela (0x318) e o Gateway (sub 0x16), limitada a +-7
		int value = packet.Decode4();
		int local = 0;
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 0x17, (int)&local, 0, 0, 0);
		int diff = value - local / 1000;
		if (abs(diff) < 3)
			break;
		if (diff > 7)
			diff = 7;
		else if (diff < -7)
			diff = -7;
		AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x318, diff, 0, 0, 0, 0);
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7535, 0x16, diff, 0, 0, 0);
		break;
	}

	case 0x201:
		// 983 006a410a
		GetActor("GolfRule") << MsgObject(NULL, 0x93, 2, 0, 0, 0, 0);
		break;

	case 0x203:
	{
		// 983 006a44b4
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int value1 = packet.Decode4();
		int value2 = packet.Decode4();
		jp983_rastro("B1399 0x203 uid=%d tid=%08x", value1, value2); // temporario: queda dono12 16:32
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7532, 0x19, value1, value2, 0, 0);
		jp983_rastro("B1399 0x203 fim");
		break;
	}

	case 0x236:
	{
		// 983 006a4538
		int value = packet.Decode4();
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7532, 0x18, value, 0, 0, 0);
		break;
	}

	// resposta do controlador de itens (0x753b): tira a espera; erro != 0 vira a mensagem 0x27, senao o pacote segue
	// para o Gateway com o sub-codigo do caso. O 983 tem um corpo por caso (nao funde)
	case 0x237: // 983 006a4566 (sub 0x14)
	{
		SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int err = packet.Decode4();
		if (err != 0)
		{
			SendMsgToMainActor(MsgObject(NULL, 0x27, err, 0, 0, 0, 0));
			break;
		}
		GetActor(s_gatewayActorName) << MsgObject(NULL, 0x753b, 0x14, (int)&packet, 0, 0, 0);
		break;
	}

	case 0x24c: // 983 006a4619 (sub 0x15)
	{
		SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int err = packet.Decode4();
		if (err == 0)
			GetActor(s_gatewayActorName) << MsgObject(NULL, 0x753b, 0x15, (int)&packet, 0, 0, 0);
		else
			SendMsgToMainActor(MsgObject(NULL, 0x27, err, 0, 0, 0, 0));
		break;
	}

	case 0x265: // 983 006a4952 (sub 0x16)
	{
		SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int err = packet.Decode4();
		if (err == 0)
			GetActor(s_gatewayActorName) << MsgObject(NULL, 0x753b, 0x16, (int)&packet, 0, 0, 0);
		else
			SendMsgToMainActor(MsgObject(NULL, 0x27, err, 0, 0, 0, 0));
		break;
	}

	case 0x266: // 983 006a49c0 (sub 0x13)
	{
		SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int err = packet.Decode4();
		if (err == 0)
			GetActor(s_gatewayActorName) << MsgObject(NULL, 0x753b, 0x13, (int)&packet, 0, 0, 0);
		else
			SendMsgToMainActor(MsgObject(NULL, 0x27, err, 0, 0, 0, 0));
		break;
	}

	case 0x26b: // 983 006a4a2e (sub 0x18)
	{
		SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int err = packet.Decode4();
		if (err == 0)
			GetActor(s_gatewayActorName) << MsgObject(NULL, 0x753b, 0x18, (int)&packet, 0, 0, 0);
		else
			SendMsgToMainActor(MsgObject(NULL, 0x27, err, 0, 0, 0, 0));
		break;
	}

	case 0x280: // 983 006a4a99 (sub 0x19)
	{
		SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		int err = packet.Decode4();
		if (err == 0)
			GetActor(s_gatewayActorName) << MsgObject(NULL, 0x753b, 0x19, (int)&packet, 0, 0, 0);
		else
			SendMsgToMainActor(MsgObject(NULL, 0x27, err, 0, 0, 0, 0));
		break;
	}

	case 0x255:
	{
		// 983 006a4687
		AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x173, 0, 0, 0, 0, 0);
		break;
	}

	case 0x258:
	{
		// 983 006a4797
		int err = packet.Decode4();
		if (err != 0)
		{
			SendMsgToMainActor(MsgObject(NULL, 0x27, err, 0, 0, 0, 0));
			break;
		}
		AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0xc9, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x174, 0, 0, 0, 0, 0);
		GetActor(s_gatewayActorName) << MsgObject(NULL, 0x753e, 5, (int)&packet, 0, 0, 0);
		GetActor("DummyPlayer") << MsgObject(NULL, 0x345, 0, 0, 0, 0, 0);
		break;
	}

	case 0x259:
	{
		// 983 006a4892; 0046e6d0 = tipo de sala 0x14 ou 0x15 (Grand Prix)
		if (packet.Decode4() != 0)
			break;
		GetActor("GolfRule") << MsgObject(NULL, 0xc8, 0, 0, 0, 0, 0);
		unsigned char realType = Doc()->m_roomInfo.realGameType;
		if (realType >= 0x14 && realType <= 0x15)
			AfxGetTask()->GetMainActor()->SendMsg(NULL, "Screen", 0x248, 0, 0, 0, 0);
		break;
	}

	case 0x25c:
	{
		// 983 006a4913: guarda um dword no global 00e7519c (lido pelo FrGrandPrixAwardDlg, 0054d110; ajuste 0069cd90)
		extern unsigned long g_gpAwardMatchTid983;
		if (packet.Decode4() != 0)
			break;
		unsigned char realType = Doc()->m_roomInfo.realGameType;
		if (realType >= 0x14 && realType <= 0x15)
			g_gpAwardMatchTid983 = packet.Decode4();
		break;
	}

	case 0x27e: // 983 006a471a
	{
		int value = packet.Decode4();
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7532, 0x1b, value, 0, 0, 0);
		break;
	}

	case 0x27f: // 983 006a4768
	{
		int value = packet.Decode4();
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7532, 0x1d, value, 0, 0, 0);
		break;
	}

	case 0x281: // 983 006a4748
	{
		int value = packet.Decode4();
		AfxGetTask()->GetActor(s_gatewayActorName) << MsgObject(NULL, 0x7532, 0x1f, value, 0, 0, 0);
		break;
	}

	default:
		return OnPacketCommon(packet);
	}
	return 1;
}

bool CGolfTask::DetermineWhetherToAddActor_GroundItemMan()
{
	bool bAdd = false;
	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
	case 1:
	// 983 (0069d8b0) nao tem o caso 3 (o 645 tinha)
	case 4:
	case 5:
	case 6:
		if (Doc()->m_playingMode != 2 && Doc()->m_gameMode != 1)
			bAdd = true;
		break;
	}
	return bAdd;
}
