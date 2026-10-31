typedef char z_rivit[sizeof(std::vector<sRivalData>::iterator)];
#include "exhibition.h"
#include "vc10_layout.h"
#include "../../include/packets/browserpackets.h" // C33: bingo
#include "clientsetting.h"
#include "invitedlg.h"
#include "userlistdlg.h"
#include "resultdlg.h"
#include "serverdlg.h"
class FrMailBoxDlg;
#include "eventprizedlg.h"
#include "mailboxdlg.h"
#include "messengerdlg.h"
#include "valentineeventdlg.h"
#include "pointeventdlg.h"
#include "netresourcemanager.h"
#include "treasuredlg.h"
#include "topiconmanager.h"
#include "lobbymain.h"
#include "user_info.h"
#include "chatmsg.h"
#include "buddymanager.h"
#include "cardsystem.h"
#include "contentsdoc.h"
#include "makeroomdlg.h"
#include "levelupitemdlg.h"
#include "standinginline.h"
#include "guilddefine.h"
#include "tooltip.h"
#include "skinnedtheme.h"
#include "gamedatadb.h"
#include "mousecursor.h"
#include "browser.h"
#include "carddlg.h" // FrSPCardBuffDlg (CloseAllDialogs 983)
#include "jp/webpageconnector.h" // 983 (s112): CWebPageConnector::RequestOpen983 (botoes rankup/guildrank da TOPPAGE)
#include "jp/gameroomdlg.h" // 983 (A174): o HandleMsg repassa as msgs da sala ao m_pGameRoomDlg983 (+0x780)
#include "jp/taskform.h" // 983 (B449): FrTaskForm::OnMsg983 (CLobbyMain::HandleMsg repassa as msgs ao form da widget)
#include "projectg.h"
#include "familymain.h"
#include "rankingdlg.h"
#include "logindlg.h"
#include "detailedroominfodlg.h"
#include "scoredlg.h"
#include "passworddlg.h"
#include "noticedlg.h"
#include "emoticondlg.h"
#include "jukeboxdlg.h"
#include "reportdlg.h"
#include "newrecorddlg.h"
#include "parannickdlg.h"
#include "caddiewarningdlg.h"
#include "equipdlg.h"
#include "worldtoureventdlg.h"
#include "frfindeborteventdlg.h"
#include "frhalloweendlg.h"
#include "christamseventdlg.h"
#include "christmassockeventdlg.h"
#include "frticketexchangedlg.h"
#include "dlgquickstartsetting.h"
#include "createnickdlg.h"
#include "missioneventdlg.h"
#include "bingoeventdlg.h"
#include "birthdayeventdlg.h"
#include "changeroominfodlg.h"
#include "guildnoticedlg.h"
#include "onelinereqdlg.h"
#include "frcombobox.h"
#include "frdesktop.h"
#include "uccmanager.h"
#include "partsinfo.h"
#include "../shared/s5/sharedutilities.h"
#include "optiondlg.h"
#include "itembuffactor.h"
#include "intrusion.h"
#include "../shared/sharedtables.h"
#include "s5/utilities.h"
#include "guildactor.h"
#include "userinfoform.h"
#include "caddieinfodlg.h"
#include "mathconsts.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.stat.Level) == 0x91d && offsetof(CSharedDoc, m_bPremiumEndNotice983) == 0x1160a); // A738

// HACK: 0.0005f is defined in tutorialmain.cpp as a function-local static aliased to
// __real@3a03126f; using the literal here would emit a competing COMDAT that all the
// other objects would bind to.
extern "C" __declspec(selectany) const float rb_real_3a03126f = 0.0005f; // C64.4: o estático de função do rebang não existe mais
// HACK: 107.0f is defined in tutorialmain.cpp as a function-local static aliased to
// __real@42d60000; using the literal here would emit a competing COMDAT that all the
// other objects would bind to.
extern "C" __declspec(selectany) const float rb_real_42d60000 = 107.0f; // C64.4: o estático de função do rebang não existe mais

// HACK: reuse gmtoolkit's original literal storage after nakedification.
#define rb_gmt_literal__04JOAJLKL "\xb8\xf0\xb5\xce"

// HACK
template <>
inline int Abs(int value)
{
	return value > 0 ? value : -value;
}

inline WRect WRect::operator+(WPoint& point) const
{
	return WRect(x + point.x, y + point.y, w, h);
}

int float2int(float f);

// A826: era uma visao local do CLoginInfo com Instance() fora de linha -> 8 chamadas a ?Instance@CLoginInfo@@SI..
// (NeedsChangeNick, OpenPCBangNoticeForm, OnToppage_*); o 983 inline new+ctor 00b2aee0 (A775) em todas.
#include "logininfo.h"

#include "intrusion.h"
#include "jp/shareddocjp.h" // PKT lote 21: GetState983 (0062bf90)
namespace S5 { void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); } // 00b307f0 (D643)
#include "powergauge.h"
#include "jp/gaugebar983.h" // 983 (s113): medidores da tela CREATE
#include "jp/lobbyavataractor.h" // 983 (s114): CExhibitionByHardwareTnL983 (boneco da tela CREATE)
#include "jp/usetikireportdlg.h" // 983 (s114): "notify_tiki" (HandleMsg 0x8b caso 0xc)
#include "jp/lua983.h" // 983 (s113): char_select_free.lua
#include "jp/singlemodestarter.h" // 983 (C-PKT lote 49): "single_mode_start"
#include "jp/grandprixroomdlg.h" // 983 (C-PKT lote 49): IsGoodMannerRate983 / IsNoMannerPart983
#include "jp/multiplayroomdlg.h" // 983 (C-PKT lote 50): "roomlist" (HandleMsg 0x8b caso 4)
#include "jp/stroketyperoomdlg.h" // 983 (C-PKT lote 50): "gameroom_main" (OpenGameRoomDlg983)
#include "jp/gptyperoomdlg.h" // 983 (C-PKT lote 50): FrMassTypeRoomDlg / FrGPTypeRoomDlg
#include "jp/singletypedlg.h" // 983 (C-PKT lote 50): "gameroom_single"
#include "jp/lobbyroom.h" // 983 (C-PKT lote 50): CLobbyRoomSelector983 (+0x778)
#include "titlefontex.h" // 983 00ab9800: fontes do Grand Prix (+0x784..0x7a0)
#include "jp/gproomtimersteward.h" // CGrandPrixClock (00e74e8c)
#include "jp/lobbyjp.h" // 983 (C-PKT lote 50): CLobbyState983 (00e78ca8)
#include "jp/avatarmotiondlg.h"
#include "jp/refresh/rflobbydesktop.h" // 983 (B159.2): TOPPAGE = RFLobbyDesktop
#include "jp/refresh/rfscenemanager.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // A803 temporario
#include "jp/bgmplaylist.h"
#include "jp/holeinonescoreinfo.h" // CRoomCheer983 (msg 0x314)
// D870: a visão CUiPointer983 (jp/lobbyjp.h) cai sobre o RFSceneManager nos dois compiladores (vs2022-7: m_pRF +0x28 cru
// = m_pInterface no cl novo -> IsModalExist em lixo no CanPickPet983)
C_ASSERT(offsetof(CUiPointer983, m_pRF) == offsetof(RFSceneManager, m_pManager));
C_ASSERT(offsetof(CUiPointer983, m_pButtons) == offsetof(RFSceneManager, m_pInput));
C_ASSERT(offsetof(CUiPointer983, m_show40) == offsetof(RFSceneManager, m_bDraw));
C_ASSERT(offsetof(CLobbyState983, m_state) == offsetof(RFLobbyDesktop, m_state));
C_ASSERT(offsetof(CLobbyState983, m_bLocked) == offsetof(RFLobbyDesktop, m_bLocked));
C_ASSERT(offsetof(CLobbyState983, m_bCursorBusy) == offsetof(RFLobbyDesktop, m_bCursorBusy));
C_ASSERT(offsetof(CLobbyState983, m_bOverPet) == offsetof(RFLobbyDesktop, m_bOverPet));
C_ASSERT(offsetof(CLobbyState983, m_bChanged) == offsetof(RFLobbyDesktop, m_bChanged));

// A174/B215: no 983 o CLobbyMain::HandleMsg (00abc140) repassa as msgs da sala (645 0x7..0x12, 0x29..0x2f, 0x46, 0x5a, 0x60;
// 983 0x87) a metodos da janela da sala (+0x780); o CLobbyMain e friend da FrGameRoomDlg (include/jp/gameroomdlg.h).
#define GameRoomDlg983() (static_cast<FrGameRoomDlg*>(m_pGameRoomDlg983))

int __fastcall IsMassGame(); // 983 00502f20 (o do CSharedDoc, sem argumento)

// HACK: realmyroommain owns this shared literal in the original image.
extern "C" const char rb_myroomTaskTitle[];

// HACK: frform.h is part of the PCH (its FrForm::EnableFlag is a late PCH inline in tutorial), which
// removed its inline bodies from this TU; this unused filler keeps the TU IL offsets, so the PCH inline
// sGuildRoomInfo::Reset still merges before CreateForm<FrLoginDlg> (window: ~120..250 increments).
namespace TUPAD_LOBBYMAIN
{
	struct Pad
	{
		int v;
		void g()
		{
			ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 ILFILL4 ILFILL4
				ILFILL4 v += 1;
		}
	};
}

class cTopEventIcon
{
public:
	cTopEventIcon(eTopIconID id)
		: m_id(id)
	{
	}

	void Prepare(FrButton* button);

	eTopIconID m_id;
};

void cTopEventIcon::Prepare(FrButton* button)
{
	if (button)
	{
		button->SetPushSound("ui_desktop_icon_click");
		button->EnableHover(true, 0.5f);
		// 983 005933c0: m_id + (IsPcBang ? 1 : 0); o 645 usava m_id/m_id - 1 (icone 80 px a esquerda)
		int index = CLoginInfo::Instance()->IsPcBang() ? m_id + 1 : m_id;
		WRect rect;
		rect.x = (float)(80 * index) + 1.0f;
		rect.y = 50.0f;
		rect.w = 80.0f;
		rect.h = 80.0f;
		button->SetRect(rect);
	}
}

std::string g_strCate[5] = { "\xb4\xeb\xc0\xfc", "\xb4\xeb\xc8\xb8",
	"\xb9\xe8\xc6\xb2", "\xb4\xeb\xc8\xad", rb_gmt_literal__04JOAJLKL };

std::string g_strGameType[8] = { "\xbd\xba\xc6\xae\xb7\xce\xc5\xa9",
	"\xb8\xc5\xc4\xa1", "30\xc0\xce\xb4\xeb\xc8\xb8",
	"30\xc0\xce\xc6\xc0\xc0\xfc", "\xb1\xe6\xb5\xe5\xb4\xeb\xc0\xfc",
	"\xc6\xce\xb9\xe8\xc6\xb2", "\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1",
	"\xb4\xeb\xc8\xad" };

int GetCateByGameType(int gameType)
{
	switch (gameType)
	{
	case GAME_TYPE_30S:
		return 1;
	case GAME_TYPE_30S_TEAM:
		return 1;
	case GAME_TYPE_GUILD_MATCH:
		return 1;
	case GAME_TYPE_SKINS:
	case GAME_TYPE_APPROACH:
	case GAME_TYPE_NEW_APPROACH:
		return 2;
	case GAME_TYPE_AVATARCHAT:
		return 3;
	case GAME_TYPE_HOLEINONE983: // 983 0059f130: categoria 4 (normallobbyroom filtra por ela)
		return 4;
	case GAME_TYPE_STROKE:
		return 0;
	case GAME_TYPE_TEAM:
		return 0;
	case GAME_TYPE_MATCH:
		return 0;
	}
	return 0;
}

long GetShotTimeLimitByGameType(int gameType)
{
	switch (gameType)
	{
	case GAME_TYPE_AVATARCHAT:
	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
		return 0;
	case GAME_TYPE_STROKE:
		return 40000;
	case GAME_TYPE_TEAM:
		return 40000;
	case GAME_TYPE_MATCH:
		return 40000;
	case GAME_TYPE_APPROACH:
		return 40000;
	case GAME_TYPE_NEW_APPROACH:
		return 40000;
	}
	return 40000;
}

long GetGameTimeLimitByGameType(int gameType)
{
	switch (gameType)
	{
	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
		return 2400000;
	case GAME_TYPE_APPROACH:
	case GAME_TYPE_NEW_APPROACH:
		return 40000;
	case GAME_TYPE_STROKE:
		return 0;
	case GAME_TYPE_TEAM:
		return 0;
	case GAME_TYPE_MATCH:
		return 0;
	case GAME_TYPE_AVATARCHAT:
		return 0;
	}
	return 0;
}

unsigned char GetRandGameTypeByCate(int cate)
{
	switch (cate)
	{
	case 1:
		return GAME_TYPE_30S;
	case 2:
		return GAME_TYPE_NEW_APPROACH;
	case 3:
		return GAME_TYPE_AVATARCHAT;
	case 0:
		return GAME_TYPE_STROKE;
	case 4:
		return GAME_TYPE_STROKE;
	}
	return GAME_TYPE_STROKE;
}

static const char* GetGameTypeText(unsigned char gameType)
{
	switch (gameType)
	{
	case GAME_TYPE_STROKE:
		return K2L_Compatibility("\xbd\xba\xc6\xae\xb7\xce\xc5\xa9");
	case GAME_TYPE_AVATARCHAT:
		return K2L_Compatibility("\xc3\xa4\xc6\xc3\xb9\xe6");
	case GAME_TYPE_30S:
		return K2L_Compatibility("\xb4\xeb\xc8\xb8");
	case GAME_TYPE_30S_TEAM:
		return K2L_Compatibility("\xb4\xeb\xc8\xb8\xc6\xc0\xc0\xfc");
	case GAME_TYPE_GUILD_MATCH:
		return K2L_Compatibility("\xb1\xe6\xb5\xe5\xb4\xeb\xc0\xfc");
	case GAME_TYPE_SKINS:
		return K2L_Compatibility("\xc6\xce\xb9\xe8\xc6\xb2");
	case GAME_TYPE_REALMYROOM:
		return K2L_Compatibility(rb_myroomTaskTitle);
	case GAME_TYPE_NEW_APPROACH:
		return K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1");
	}
	return "";
}

bool ApproachRankUserCompare(const void* a, const void* b);
bool RankUserCompare(const void* a, const void* b);

bool ApproachRankUserCompare(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;

	if (pA->quitOrder > pB->quitOrder)
		return true;

	if (pA->quitOrder < pB->quitOrder)
		return false;

	if (pA->approachResultDistance < pB->approachResultDistance)
		return true;

	if (pA->approachResultDistance > pB->approachResultDistance)
		return false;

	if (pA->approachTime > pB->approachTime)
		return true;

	if (pA->approachTime < pB->approachTime)
		return false;

	return pA->order < pB->order;
}

bool RankUserCompare(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;

	if (pA->quitOrder > pB->quitOrder)
		return true;

	if (pA->quitOrder < pB->quitOrder)
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

static int s_roomCate = 4;
static int s_roomGameType = -1;

IMPLEMENT_ACTOR(CLobbyMain, CTaskMain)
BEGIN_FRESH_MSGMAP(CLobbyMain, CTaskMain)
ON_FRESH_VI("LOGIN", FRCMD_INIT, CLobbyMain::OnLogin_Init)
ON_FRESH_VV("LOGIN", FRCMD_FINISH, CLobbyMain::OnLogin_Finish)
ON_FRESH_VI("LOGIN.copyright1", FRCMD_INIT, CLobbyMain::OnLogin_Copyright1Init983)
ON_FRESH_VI("LOGIN.copyright2", FRCMD_INIT, CLobbyMain::OnLogin_Copyright2Init983)
ON_FRESH_VI("BLANK", FRCMD_INIT, CLobbyMain::OnBlank_Init)
ON_FRESH_VV("BLANK", FRCMD_FINISH, CLobbyMain::OnBlank_Finish)

ON_FRESH_VI("SECOND_PWD", FRCMD_INIT, CLobbyMain::OnSecond_Pwd_Init)
ON_FRESH_VV("SECOND_PWD", FRCMD_FINISH, CLobbyMain::OnSecond_Pwd_Finish)

ON_FRESH_VI("TOPPAGE", FRCMD_INIT, CLobbyMain::OnToppage_Init)
ON_FRESH_VV("TOPPAGE", FRCMD_FINISH, CLobbyMain::OnToppage_Finish)
ON_FRESH_VV("TOPPAGE", FRCMD_DESTROY, CLobbyMain::OnToppage_Destroy)
ON_FRESH_VI("TOPPAGE.gameplay", FRCMD_INIT, CLobbyMain::OnToppage_GameInit)
ON_FRESH_VV("TOPPAGE.gameplay", FRCMD_LBUTTONUP, CLobbyMain::OnToppage_GameUp)
ON_FRESH_VI("TOPPAGE.family", FRCMD_INIT, CLobbyMain::OnToppage_FamilyInit)
ON_FRESH_VV("TOPPAGE.family", FRCMD_LBUTTONUP, CLobbyMain::OnToppage_FamilyUp)
ON_FRESH_VI("TOPPAGE.pangyastudy", FRCMD_INIT,
	CLobbyMain::OnToppage_PangysStudyInit)
ON_FRESH_VV("TOPPAGE.pangyastudy", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_PangysStudyUp)
ON_FRESH_VI("TOPPAGE.premium_service_end_notice", FRCMD_INIT, CLobbyMain::OnToppage_PremiumEndNoticeInit983)
ON_FRESH_VI("TOPPAGE.premium_service_end_notice", FRCMD_OWNERDRAW, CLobbyMain::OnToppage_PremiumEndNoticeOwnerDraw983) // D644 (983 00e0d180)
ON_FRESH_VI("TOPPAGE.PremiumService", FRCMD_INIT, CLobbyMain::OnToppage_PremiumServiceInit983)
ON_FRESH_VV("TOPPAGE.PremiumService", FRCMD_LBUTTONDOWN, CLobbyMain::OpenPremiumService983) // D643 (983 00e0d440)
ON_FRESH_VV("TOPPAGE.PremiumService", FRCMD_HOVERON, CLobbyMain::OnToppage_PremiumServiceHoverOn983)
ON_FRESH_VV("TOPPAGE.PremiumService", FRCMD_HOVEROFF, CLobbyMain::OnToppage_PremiumServiceHoverOff983)
ON_FRESH_VV("TOPPAGE.family", FRCMD_HOVERON, CLobbyMain::OnToppage_FamilyHoverOn983) // D643 (983 00e0d260..00e0d300)
ON_FRESH_VV("TOPPAGE.family", FRCMD_HOVEROFF, CLobbyMain::OnToppage_HoverOff983)
ON_FRESH_VV("TOPPAGE.pangyastudy", FRCMD_HOVERON, CLobbyMain::OnToppage_PangyaStudyHoverOn983)
ON_FRESH_VV("TOPPAGE.pangyastudy", FRCMD_HOVEROFF, CLobbyMain::OnToppage_HoverOff983)
ON_FRESH_VV("TOPPAGE.notice", FRCMD_HOVERON, CLobbyMain::OnToppage_NoticeHoverOn983)
ON_FRESH_VV("TOPPAGE.notice", FRCMD_HOVEROFF, CLobbyMain::OnToppage_HoverOff983)

ON_FRESH_VI("TOPPAGE.notice", FRCMD_INIT, CLobbyMain::OnToppage_NoticeInit)
ON_FRESH_VI("TOPPAGE.toppage_change_bg_btn", FRCMD_INIT, CLobbyMain::OnToppage_ChangeBgInit983)
ON_FRESH_VV("TOPPAGE.toppage_change_bg_btn", FRCMD_LBUTTONUP, CLobbyMain::OnToppage_ChangeBgUp983)
ON_FRESH_VV("TOPPAGE.notice", FRCMD_LBUTTONDOWN, CLobbyMain::OnToppage_NoticeUp)
ON_FRESH_VI("TOPPAGE.oneday_quest", FRCMD_INIT, CLobbyMain::OnToppage_OneDayQuestInit983)
ON_FRESH_VV("TOPPAGE.oneday_quest", FRCMD_LBUTTONDOWN, CLobbyMain::OnToppage_OneDayQuestDown983)
ON_FRESH_VI("TOPPAGE.server", FRCMD_INIT, CLobbyMain::OnToppage_ServerInit)
ON_FRESH_VV("TOPPAGE.server", FRCMD_LBUTTONDOWN, CLobbyMain::OnToppage_ServerUp)
ON_FRESH_VI("TOPPAGE.pcbang", FRCMD_INIT, CLobbyMain::OnToppage_PcbangInit)
ON_FRESH_VV("TOPPAGE.pcbang", FRCMD_LBUTTONDOWN, CLobbyMain::OnToppage_PcbangUp)
ON_FRESH_VI("TOPPAGE.rankup", FRCMD_INIT, CLobbyMain::OnToppage_RankUpInit983)
ON_FRESH_VV("TOPPAGE.rankup", FRCMD_LBUTTONDOWN, CLobbyMain::OnToppage_RankUpDown983)
ON_FRESH_VI("TOPPAGE.guildrank", FRCMD_INIT, CLobbyMain::OnToppage_GuildRankInit983)
ON_FRESH_VV("TOPPAGE.guildrank", FRCMD_LBUTTONDOWN, CLobbyMain::OnToppage_GuildRankDown983)
ON_FRESH_VI("TOPPAGE.motion_btn_start", FRCMD_INIT, CLobbyMain::OnToppage_MotionBtnInit983)
ON_FRESH_VV("TOPPAGE.motion_btn_start", FRCMD_LBUTTONUP, CLobbyMain::OnToppage_MotionBtnUp983)
ON_FRESH_VV("TOPPAGE.motion_btn_start", FRCMD_HOVERON, CLobbyMain::OnToppage_MotionBtnHoverOn983) // D642
ON_FRESH_VV("TOPPAGE.motion_btn_start", FRCMD_HOVEROFF, CLobbyMain::OnToppage_MotionBtnHoverOff983)
ON_FRESH_VI("TOPPAGE.portrait_jukeboxbtn", FRCMD_INIT, CLobbyMain::OnToppage_JukeboxBtnInit983)
ON_FRESH_VV("TOPPAGE.portrait_jukeboxbtn", FRCMD_LBUTTONUP, CLobbyMain::OnToppage_JukeboxBtnUp983)
ON_FRESH_VI("TOPPAGE.rental_expired", FRCMD_INIT,
	CLobbyMain::OnToppage_RentalExpiredAlramInit)
ON_FRESH_VV("TOPPAGE.rental_expired", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_RentalExpiredAlramUp)
ON_FRESH_VI("TOPPAGE.event1", FRCMD_INIT, CLobbyMain::OnToppage_Event1Init)
ON_FRESH_VV("TOPPAGE.event1", FRCMD_LBUTTONDOWN, CLobbyMain::OnToppage_Event1Up)

ON_FRESH_VI("TOPPAGE.new_2", FRCMD_INIT, CLobbyMain::OnToppage_NewInit)
ON_FRESH_VI("TOPPAGE.new_2", FRCMD_OWNERDRAW,
	CLobbyMain::OnToppage_NewOwnerDraw)

ON_FRESH_VI("TOPPAGE.TopIcon1", FRCMD_INIT, CLobbyMain::OnToppage_TopIcon1Init)
ON_FRESH_VV("TOPPAGE.TopIcon1", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_TopIcon1Up)

ON_FRESH_VI("TOPPAGE.TopIcon2", FRCMD_INIT, CLobbyMain::OnToppage_TopIcon2Init)
ON_FRESH_VV("TOPPAGE.TopIcon2", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_TopIcon2Up)

ON_FRESH_VI("TOPPAGE.TopIcon3", FRCMD_INIT, CLobbyMain::OnToppage_TopIcon3Init)
ON_FRESH_VV("TOPPAGE.TopIcon3", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_TopIcon3Up)

ON_FRESH_VI("TOPPAGE.TopIcon4", FRCMD_INIT, CLobbyMain::OnToppage_TopIcon4Init)
ON_FRESH_VV("TOPPAGE.TopIcon4", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_TopIcon4Up)

ON_FRESH_VI("TOPPAGE.TopIcon5", FRCMD_INIT, CLobbyMain::OnToppage_TopIcon5Init)
ON_FRESH_VV("TOPPAGE.TopIcon5", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_TopIcon5Up)

ON_FRESH_VI("TOPPAGE.TopIcon6", FRCMD_INIT, CLobbyMain::OnToppage_TopIcon6Init)
ON_FRESH_VV("TOPPAGE.TopIcon6", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_TopIcon6Up)

ON_FRESH_VI("TOPPAGE.pointevent", FRCMD_INIT,
	CLobbyMain::OnToppage_PointEventInit)
ON_FRESH_VV("TOPPAGE.pointevent", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_PointEventDown)

ON_FRESH_VI("TOPPAGE.valevent_btn", FRCMD_INIT,
	CLobbyMain::OnToppage_ValEventInit)
ON_FRESH_VV("TOPPAGE.valevent_btn", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_ValEventDown)

ON_FRESH_VI("TOPPAGE.worldtourevent", FRCMD_INIT,
	CLobbyMain::OnToppage_WorldTourEventInit)
ON_FRESH_VV("TOPPAGE.worldtourevent", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_WorldTourEventDown)

ON_FRESH_VI("TOPPAGE.findebortevent", FRCMD_INIT,
	CLobbyMain::OnToppage_FindEbortEventInit)
ON_FRESH_VV("TOPPAGE.findebortevent", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_FindEbortEventDown)

ON_FRESH_VI("TOPPAGE.halloween2007event", FRCMD_INIT,
	CLobbyMain::OnToppage_Halloween2007Init)
ON_FRESH_VV("TOPPAGE.halloween2007event", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_Halloween2007EvenDown)

ON_FRESH_VI("TOPPAGE.Christmas2007Event", FRCMD_INIT,
	CLobbyMain::OnToppage_Christmas2007EventInit)
ON_FRESH_VV("TOPPAGE.Christmas2007Event", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_Christmas2007EventDown)

ON_FRESH_VI("TOPPAGE.ticketexchage", FRCMD_INIT,
	CLobbyMain::OnToppage_TicketExchangeInit)
ON_FRESH_VV("TOPPAGE.ticketexchage", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_TicketExchangeDown)

ON_FRESH_VI("TOPPAGE.MissionEvent", FRCMD_INIT,
	CLobbyMain::OnToppage_MissionEventInit)
ON_FRESH_VV("TOPPAGE.MissionEvent", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_MissionEventDown)

ON_FRESH_VI("TOPPAGE.BingoEvent", FRCMD_INIT,
	CLobbyMain::OnToppage_BingoEventInit)
ON_FRESH_VV("TOPPAGE.BingoEvent", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_BingoEventDown)

ON_FRESH_VI("TOPPAGE.KoohBirthdayEvent", FRCMD_INIT,
	CLobbyMain::OnToppage_KoohBirthdayEventInit)
ON_FRESH_VV("TOPPAGE.KoohBirthdayEvent", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_KoohBirthdayEventDown)

ON_FRESH_VI("TOPPAGE.treasureisland", FRCMD_INIT,
	CLobbyMain::OnToppage_TreasureIslandInit)
ON_FRESH_VV("TOPPAGE.treasureisland", FRCMD_LBUTTONDOWN,
	CLobbyMain::OnToppage_TreasureIslandLBDown)

ON_FRESH_VI("TOPPAGE.ubar", FRCMD_INIT, CTaskMain::OnUnderBarInit)
ON_FRESH_VI("TOPPAGE.ubar.back", FRCMD_INIT, CTaskMain::OnUnderBar_BackInit)
ON_FRESH_VV("TOPPAGE.ubar.back", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_BackUp)
ON_FRESH_VI("TOPPAGE.ubar.back", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.back", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.front", FRCMD_INIT, CTaskMain::OnUnderBar_FrontInit)
ON_FRESH_VV("TOPPAGE.ubar.front", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_FrontUp)
ON_FRESH_VI("TOPPAGE.ubar.front", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.front", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.shop", FRCMD_INIT, CTaskMain::OnUnderBar_ShopInit)
ON_FRESH_VV("TOPPAGE.ubar.shop", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ShopUp)
ON_FRESH_VI("TOPPAGE.ubar.shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.shop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.myroom", FRCMD_INIT, CTaskMain::OnUnderBar_MyRoomInit)
ON_FRESH_VV("TOPPAGE.ubar.myroom", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MyRoomUp)
ON_FRESH_VI("TOPPAGE.ubar.myroom", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.myroom", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (C40.3, s110): "ubar.achievement" no lugar do "ubar.itemstorage" (mapa do 983: 00651e30 / 006582f0 / hover)
ON_FRESH_VI("TOPPAGE.ubar.achievement", FRCMD_INIT, CTaskMain::OnUnderBar_AchievementInit983)
ON_FRESH_VV("TOPPAGE.ubar.achievement", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_AchievementUp983)
ON_FRESH_VI("TOPPAGE.ubar.achievement", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.achievement", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.gift", FRCMD_INIT, CTaskMain::OnUnderBar_GiftInit)
// 983: LBUTTONUP (mapa 00ccf510, codigo 4)
ON_FRESH_VV("TOPPAGE.ubar.gift", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GiftUp)
ON_FRESH_VI("TOPPAGE.ubar.gift", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.gift", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.cookie", FRCMD_INIT, CTaskMain::OnUnderBar_CookieInit)
ON_FRESH_VV("TOPPAGE.ubar.cookie", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_CookieUp)
ON_FRESH_VI("TOPPAGE.ubar.cookie", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.cookie", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.game", FRCMD_INIT, CTaskMain::OnUnderBar_GameInit)
ON_FRESH_VV("TOPPAGE.ubar.game", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GameUp)
ON_FRESH_VI("TOPPAGE.ubar.game", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.game", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.server", FRCMD_INIT, CTaskMain::OnUnderBar_ServerInit)
ON_FRESH_VV("TOPPAGE.ubar.server", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ServerUp)
ON_FRESH_VI("TOPPAGE.ubar.server", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.server", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.exit", FRCMD_INIT, CTaskMain::OnUnderBar_ExitInit)
ON_FRESH_VV("TOPPAGE.ubar.exit", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ExitUp)
ON_FRESH_VI("TOPPAGE.ubar.exit", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.ubar.exit", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.ubar.tabbtn", FRCMD_INIT, CTaskMain::OnUnderBar_TabBtnInit)
ON_FRESH_VV("TOPPAGE.ubar.tabbtn", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TabBtnUp)

ON_FRESH_VI("TOPPAGE.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)

ON_FRESH_VI("TOPPAGE.utab.magicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_MagicBoxInit)
ON_FRESH_VV("TOPPAGE.utab.magicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MagicBoxUp)
ON_FRESH_VI("TOPPAGE.utab.magicbox", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.magicbox", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s106): "utab.combine" e "utab.gacha" (mapa do 983: init, LBUTTONDOWN, hover)
ON_FRESH_VI("TOPPAGE.utab.combine", FRCMD_INIT, CTaskMain::OnUnderBar_CombineInit983)
ON_FRESH_VV("TOPPAGE.utab.combine", FRCMD_LBUTTONDOWN, CTaskMain::OnUnderBar_CombineUp983)
ON_FRESH_VI("TOPPAGE.utab.combine", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.combine", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.utab.gacha", FRCMD_INIT, CTaskMain::OnUnderBar_GachaInit983)
ON_FRESH_VV("TOPPAGE.utab.gacha", FRCMD_LBUTTONDOWN, CTaskMain::OnUnderBar_GachaUp983)
ON_FRESH_VI("TOPPAGE.utab.gacha", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.gacha", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)

ON_FRESH_VI("TOPPAGE.utab.bongdarishop", FRCMD_INIT,
	CTaskMain::OnUnderBar_BongdariShopInit)
ON_FRESH_VV("TOPPAGE.utab.bongdarishop", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_BongdariShopUp)
ON_FRESH_VI("TOPPAGE.utab.bongdarishop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.bongdarishop", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.utab.ranking", FRCMD_INIT,
	CTaskMain::OnUnderBar_RankingInit)
ON_FRESH_VV("TOPPAGE.utab.ranking", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_RankingUp)
ON_FRESH_VI("TOPPAGE.utab.ranking", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.ranking", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.utab.guild", FRCMD_INIT, CTaskMain::OnUnderBar_GuildInit)
ON_FRESH_VV("TOPPAGE.utab.guild", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GuildUp)
ON_FRESH_VI("TOPPAGE.utab.guild", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.guild", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.utab.notice", FRCMD_INIT, CTaskMain::OnUnderBar_NoticeInit)
ON_FRESH_VV("TOPPAGE.utab.notice", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_NoticeUp)
ON_FRESH_VI("TOPPAGE.utab.notice", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.notice", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s111): "utab.point_shop" (AlchemyDlg) e "utab.card_compose" (alquimia de cartas), LBUTTONUP
ON_FRESH_VI("TOPPAGE.utab.point_shop", FRCMD_INIT, CTaskMain::OnUnderBar_PointShopInit983)
ON_FRESH_VV("TOPPAGE.utab.point_shop", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_PointShopUp983)
ON_FRESH_VI("TOPPAGE.utab.point_shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.point_shop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.utab.card_compose", FRCMD_INIT, CTaskMain::OnUnderBar_CardComposeInit983)
ON_FRESH_VV("TOPPAGE.utab.card_compose", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_CardComposeUp983)
ON_FRESH_VI("TOPPAGE.utab.card_compose", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.utab.card_compose", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s111): sem "utab.tikimagicbox", "utab.scratch", "utab.maket", "obar.messenger_alarm" e "obar.gift_alarm"

ON_FRESH_VI("TOPPAGE.obar", FRCMD_INIT, CTaskMain::OnOverBar_Init)
ON_FRESH_VI("TOPPAGE.obar.title", FRCMD_INIT, CTaskMain::OnOverBar_TitleInit)
ON_FRESH_VI("TOPPAGE.obar.site", FRCMD_INIT, CTaskMain::OnOverBar_SiteInit)
ON_FRESH_VI("TOPPAGE.obar.char", FRCMD_INIT, CTaskMain::OnOverBar_CharInit)
ON_FRESH_VV("TOPPAGE.obar.char", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CharDown)
ON_FRESH_VI("TOPPAGE.obar.char", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.caddie", FRCMD_INIT, CTaskMain::OnOverBar_CaddieInit)
ON_FRESH_VV("TOPPAGE.obar.caddie", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CaddieDown)
ON_FRESH_VI("TOPPAGE.obar.caddie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.club", FRCMD_INIT, CTaskMain::OnOverBar_ClubInit)
ON_FRESH_VV("TOPPAGE.obar.club", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_ClubDown)
ON_FRESH_VI("TOPPAGE.obar.club", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.aztec", FRCMD_INIT, CTaskMain::OnOverBar_AztecInit)
ON_FRESH_VV("TOPPAGE.obar.aztec", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_AztecDown)
ON_FRESH_VI("TOPPAGE.obar.aztec", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.mascot", FRCMD_INIT, CTaskMain::OnOverBar_MascotInit)
ON_FRESH_VV("TOPPAGE.obar.mascot", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MascotDown)
ON_FRESH_VI("TOPPAGE.obar.mascot", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotOwnerDraw)
// 983 (s111): "obar.eqitem" (itens equipados)
ON_FRESH_VI("TOPPAGE.obar.eqitem", FRCMD_INIT, CTaskMain::OnOverBar_EqItemInit983)
ON_FRESH_VV("TOPPAGE.obar.eqitem", FRCMD_LBUTTONDOWN, CTaskMain::OnOverBar_EqItemDown983)
ON_FRESH_VI("TOPPAGE.obar.eqitem", FRCMD_OWNERDRAW, CTaskMain::OnOverBar_EqItemOwnerDraw983)
ON_FRESH_VI("TOPPAGE.obar.char_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CharSelInit)
ON_FRESH_VI("TOPPAGE.obar.char_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharSelOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.caddie_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieSelInit)
ON_FRESH_VI("TOPPAGE.obar.caddie_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieSelOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.club_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_ClubSelInit)
ON_FRESH_VI("TOPPAGE.obar.club_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubSelOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.aztec_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecSelInit)
ON_FRESH_VI("TOPPAGE.obar.mascot_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotSelInit)
ON_FRESH_VI("TOPPAGE.obar.mascot_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotSelOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.pang_cookie", FRCMD_INIT,
	CTaskMain::OnOverBar_PangCookieInit)
ON_FRESH_VI("TOPPAGE.obar.onelinemsg_req", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineReqBtnInit)
ON_FRESH_VV("TOPPAGE.obar.onelinemsg_req", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_OneLineReqBtnUp)
ON_FRESH_VI("TOPPAGE.obar.onelineview", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineViewInit)
ON_FRESH_VI("TOPPAGE.obar.onelineview", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_OneLineViewOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.spcard", FRCMD_INIT,
	CTaskMain::OnOverBar_SpecialCardInit)
ON_FRESH_VV("TOPPAGE.obar.spcard", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_SpecialCardUp)
ON_FRESH_VI("TOPPAGE.obar.spcard", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.obar.spcard", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.obar.messenger", FRCMD_INIT,
	CTaskMain::OnOverBar_MessengerInit)
ON_FRESH_VV("TOPPAGE.obar.messenger", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MessengerUp)
ON_FRESH_VI("TOPPAGE.obar.messenger", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.obar.messenger", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.obar.myinfo", FRCMD_INIT, CTaskMain::OnOverBar_MyInfoInit)
ON_FRESH_VV("TOPPAGE.obar.myinfo", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MyInfoUp)
ON_FRESH_VI("TOPPAGE.obar.myinfo", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.obar.myinfo", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("TOPPAGE.obar.option", FRCMD_INIT, CTaskMain::OnOverBar_OptionInit)
ON_FRESH_VV("TOPPAGE.obar.option", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_OptionUp)
ON_FRESH_VI("TOPPAGE.obar.option", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("TOPPAGE.obar.option", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)

ON_FRESH_VI("TOPPAGE.ubar.undertab_tooltip", FRCMD_INIT,
	CTaskMain::OnUnderBar_TabToolTipInit)
ON_FRESH_VI("TOPPAGE.obar.messenger_bubble", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleInit)
ON_FRESH_VI("TOPPAGE.obar.messenger_bubble", FRCMD_OWNERDRAW,
	CTaskMain::OnMessengerBubbleOwnerDraw)
ON_FRESH_VI("TOPPAGE.obar.messenger_desc", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleEditInit)
ON_FRESH_VI("TOPPAGE.obar.tooltip", FRCMD_OWNERDRAW,
	CTaskMain::OnBarTooltipOwnerDraw)
ON_FRESH_VI("TOPPAGE.treasure_alarm", FRCMD_INIT,
	CTaskMain::OnTreasureAlarmInit)
ON_FRESH_VV("TOPPAGE.treasure_alarm", FRCMD_LBUTTONDOWN,
	CTaskMain::OnTreasureAlarmUp)

ON_FRESH_VI("TOPPAGE.FirstConnect", FRCMD_OWNERDRAW,
	CLobbyMain::OnFirstConnectOwnerDraw)

ON_FRESH_VV("TOPPAGE.Go_UCCShop", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_UCCShopBtnUp)
ON_FRESH_VV("TOPPAGE.Go_CardShop", FRCMD_LBUTTONUP,
	CLobbyMain::OnToppage_CardShopBtnUp)

ON_FRESH_VI("SERVERLIST", FRCMD_INIT, CLobbyMain::OnServer_Init)
ON_FRESH_VV("SERVERLIST", FRCMD_FINISH, CLobbyMain::OnServer_Finish)
















ON_FRESH_VI("CREATE", FRCMD_INIT, CLobbyMain::OnCreate_Init)
ON_FRESH_VV("CREATE", FRCMD_FINISH, CLobbyMain::OnCreate_Finish)

ON_FRESH_VV("CREATE", FRCMD_DESTROY, CLobbyMain::OnCreate_Destroy)
ON_FRESH_VI("CREATE.pet", FRCMD_INIT, CLobbyMain::OnCreatePetInit)
ON_FRESH_VV("CREATE.pet", FRCMD_OWNERDRAW, CLobbyMain::OnCreatePetOwnerDraw)
ON_FRESH_VI("CREATE.char", FRCMD_INIT, CLobbyMain::OnCreate_CharInit)
ON_FRESH_VI("CREATE.charprev", FRCMD_INIT, CLobbyMain::OnCreate_CharPrevInit)
ON_FRESH_VI("CREATE.charnext", FRCMD_INIT, CLobbyMain::OnCreate_CharNextInit)
ON_FRESH_VI("CREATE.hair", FRCMD_INIT, CLobbyMain::OnCreate_HairInit)
ON_FRESH_VI("CREATE.hairprev", FRCMD_INIT, CLobbyMain::OnCreate_HairPrevInit)
ON_FRESH_VV("CREATE.hairprev", FRCMD_LBUTTONUP,
	CLobbyMain::OnCreate_HairPrevBtnUp)
ON_FRESH_VI("CREATE.hairnext", FRCMD_INIT, CLobbyMain::OnCreate_HairNextInit)
ON_FRESH_VV("CREATE.hairnext", FRCMD_LBUTTONUP,
	CLobbyMain::OnCreate_HairNextBtnUp)
ON_FRESH_VI("CREATE.shirt", FRCMD_INIT, CLobbyMain::OnCreate_ShirtsInit)
ON_FRESH_VI("CREATE.shirtprev", FRCMD_INIT, CLobbyMain::OnCreate_ShirtsPrevInit)
ON_FRESH_VV("CREATE.shirtprev", FRCMD_LBUTTONUP,
	CLobbyMain::OnCreate_ShirtsPrevBtnUp)
ON_FRESH_VI("CREATE.shirtnext", FRCMD_INIT, CLobbyMain::OnCreate_ShirtsNextInit)
ON_FRESH_VV("CREATE.shirtnext", FRCMD_LBUTTONUP,
	CLobbyMain::OnCreate_ShirtsNextBtnUp)
ON_FRESH_VV("CREATE.create", FRCMD_LBUTTONUP, CLobbyMain::OnCreate_CreateBtnUp)
ON_FRESH_VI("CREATE.desc", FRCMD_INIT, CLobbyMain::OnCreate_DescInit)
ON_FRESH_VI("CREATE.warning", FRCMD_INIT, CLobbyMain::OnCreate_WarningInit)
ON_FRESH_VI("CREATE.powerbar", FRCMD_INIT, CLobbyMain::On_PowerBarInit)
ON_FRESH_VI("CREATE.controlbar", FRCMD_INIT, CLobbyMain::On_ControlBarInit)
ON_FRESH_VI("CREATE.accuratebar", FRCMD_INIT, CLobbyMain::On_AccuracyBarInit)
ON_FRESH_VI("CREATE.spinbar", FRCMD_INIT, CLobbyMain::On_SpinBarInit)

ON_FRESH_VI("CREATE.curvebar", FRCMD_INIT, CLobbyMain::On_CurveBarInit)
ON_FRESH_VI("CREATE.character_list", FRCMD_INIT, CLobbyMain::OnCreate_CharacterListInit983)
ON_FRESH_VI("CREATE.character_list", FRCMD_LBUTTONDOWN, CLobbyMain::OnCreate_CharacterListDown983)
ON_FRESH_VI("CREATE.character_list", FRCMD_OWNERDRAW, CLobbyMain::OnCreate_CharacterListOwnerDraw983)
ON_FRESH_VI("CREATE.draw_area", FRCMD_INIT, CLobbyMain::OnCreate_DrawAreaInit983)
ON_FRESH_VI("CREATE.draw_area", FRCMD_OWNERDRAW, CLobbyMain::OnCreate_DrawAreaOwnerDraw983)
ON_FRESH_VI("CREATE.char_name_image", FRCMD_INIT, CLobbyMain::OnCreate_CharNameImageInit983)

END_FRESH_MSGMAP()

bool FinishUserCompare(const void* a, const void* b)
{
	if (!a)
		return false;

	if (!b)
		return true;

	const sSlotInfo* pA = (const sSlotInfo*)a;
	const sSlotInfo* pB = (const sSlotInfo*)b;

	return Doc()->m_rivalList[Doc()->GetIndex(pA->dwGuid)].finishOrder <
		Doc()->m_rivalList[Doc()->GetIndex(pB->dwGuid)].finishOrder;
}

bool ConnectionUserCompare(const void* a, const void* b)
{
	if (!a)
		return false;

	if (!b)
		return true;

	const sSlotInfo* pA = (const sSlotInfo*)a;
	const sSlotInfo* pB = (const sSlotInfo*)b;

	return pA->connectionRank < pB->connectionRank;
}

CLobbyMain::CLobbyMain()
{
	Doc()->m_shopName = "";

	m_pRoomListDoc983 = NULL;
	m_pRoomSelector983 = NULL; // 983: zerado no construtor (as fontes de +0x784 sao shared_ptr)

	m_pLoginDlg = NULL;
	m_pWaitDlg = NULL;
	m_pCreateChar = NULL;
	m_pCreateDesc = NULL;
	m_pCreateHair = NULL;
	m_pCreateShirt = NULL;
	m_pPointEventDlg = NULL;
	m_pWorldTourEventDlg = NULL;
	m_pFindEbortEventDlg = NULL;
	m_pQuickStartDlg = NULL;
	m_bOverBarFlag0 = false;
	m_bOverBarFlag1 = false;
	m_fKbdManualTime = -10.0f;
	m_roomPassword[0] = 0;
	m_pTitleFont = NULL;
	m_bCaddieWarning = false;
	memset(m_caddieWarning, 0, sizeof(m_caddieWarning));

	ResetControl();

	m_bLoginBlock = false;

	m_autoStartTime = 15.0f;
	m_bCanStart = false;

	m_pToppagePointEvent = NULL;

	for (int i = 0; i < 3; i++)
		m_pLoginBmp[i] = NULL;

	m_pNewBmp = NULL;

	m_reserved3b0 = 0;

	m_pToppageWorldTourEvent = NULL;

	m_reserved3cc = 0;

	m_pToppageHalloween2007Event = NULL;
	m_pHalloweenEventDlg = NULL;
	m_pHalloweenEventGiftDlg = NULL;

	m_pToppageChristmas2007Event = NULL;
	m_pChristmasEventDlg = NULL;

	m_pCreatePetArea983 = NULL;

	m_pToppageTicketExchange = NULL;
	m_pTicketExchangeDlg = NULL;

	m_pToppageMissionEvent = NULL;
	m_pMissionEventDlg = NULL;

	m_pToppageBingoEvent = NULL;
	m_pBingoEventDlg = NULL;

	m_pToppageEvent1 = NULL;

	m_reserved1e4 = 0;

	m_pQuickResultDlg = NULL;

	m_pReservedDlg490 = NULL;
	m_pCreateNickDlg = NULL;
	m_topBtnIndex = 0;
	m_event1Dir = 1.0f;
	// 983 00aba679..00aba6bb: zerados no fim do construtor (B186.1: sem isto o OpenGameRoomDlg983 lia lixo em +0x77c/+0x780)
	for (int i = 0; i < 5; i++)
		m_pStatGauge983[i] = NULL;
	m_pMotionDlg983 = NULL;
	m_pMotionBtn983 = NULL;
	m_pToppageJukeboxBtn983 = NULL;
	m_pJukeBoxDlg983 = NULL;
	m_pToppagePremiumService983 = NULL;
	m_pPremiumServiceDlg983 = NULL; // B433: sem isto o CloseAllDialogs lia lixo em +0x438 (AV 008d0abe a243/d276/c179)
	m_pRoomListForm983 = 0;
	m_pGameRoomDlg983 = NULL;
	m_pUseTikiReportDlg983 = NULL; // D726: 983 00aba5b7 (lido em HandleMsg 0x8b caso 0xc antes de criar "notify_tiki")
	m_reserved474 = 0; // 983 00aba52d
	// 983: sem CIconManager::ClearInformation (o contador de icones de fora do 645 nao existe; 00aba400)
}

CLobbyMain::~CLobbyMain()
{
	for (std::map<unsigned long, sRoomSlot>::iterator it =
			 Doc()->m_roomSlotMap.begin();
		it != Doc()->m_roomSlotMap.end(); it++)
	{
		if (it->second.pExhibition)
		{
			delete it->second.pExhibition;
			it->second.pExhibition = NULL;
		}
	}

	this << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
}

// A173 (temporario, pedido A173.3 a B): o construtor do CLobbyRoomSelector983 (LoadRoomListLayout983 da B, normallobbyroom.cpp)
// fecha o jogo com codigo 1 (robo a173-3 e a173-7, este ja com o GetByIndex do B182). Tirar este define quando a B corrigir.
// #define A173_SEM_SELETOR983 (A175: B215 guardou o LoadRoomListLayout983; teste a175 sem o define)
// 983 00ab9800: alem do cursor, o seletor de salas (lista de salas do CContentsDoc 0xa5), a aba de salas "nao lida"
// (CSharedDoc+0x11634 = 2), as fontes do Grand Prix (o relogio usa a grande), as imagens do relogio, o fundo 3D e o
// mapa do cenario do lobby ("west", estrelas 5) mandado ao LobbyBg antes do OnLoad dele
void CLobbyMain::OnPreLoadInit()
{
	CTaskMain::OnPreLoadInit();

	CMouseCursor::Instance()->SetAutoHide(false);

	sRoomListDoc983* pRoomListDoc = (sRoomListDoc983*)CContentsDoc::Instance()->GetContainer((localContentType_t)0xa5);
	if (pRoomListDoc)
		m_pRoomListDoc983 = pRoomListDoc;

	Doc()->m_roomListRequest983 = 2; // 0062bf60

#ifndef A173_SEM_SELETOR983
	CLobbyRoomSelector983* pSelector = new CLobbyRoomSelector983(m_pRoomListDoc983);
	if (pSelector != m_pRoomSelector983)
	{
		delete m_pRoomSelector983;
		m_pRoomSelector983 = pSelector;
	}
#endif

	m_pGPTimerFont983 = std::tr1::shared_ptr<CGrandPrixGameRoomTimerFont>(new CGrandPrixGameRoomTimerFont);
	m_pGPTimerBigFont983 = std::tr1::shared_ptr<CGrandPrixGameRoomTimerBigFont>(new CGrandPrixGameRoomTimerBigFont);
	m_pGPTicketFont983 = std::tr1::shared_ptr<CGrandPrixGameRoomTicketFont>(new CGrandPrixGameRoomTicketFont);
	m_pGPDiffLevelFont983 = std::tr1::shared_ptr<CGrandPrixDiffLevelFont>(new CGrandPrixDiffLevelFont);

	CGrandPrixClock::Instance()->m_pFont = m_pGPTimerBigFont983.get();
	CGrandPrixClock::Instance()->LoadBitmaps983();

	TurnOn3DBackground();

	AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, M983(0x13e), 5, (int)"west", 0, 0, 0); // 0x17a
}

// 983 00ab5a70: depois do OnLoad da base, o LoadScript983 do seletor de salas (005a06b0) carrega os bitmaps e
// posicoes da lista do Grand Prix (00599cd0); sem isto a aba Grand Prix desenha Bitmap NULL e fecha (a215-2: AV 009a17cc
// TexCacheManager::Add <- CGrandPrixLobbyRoom::OnRoomListOwnerDraw)
void CLobbyMain::OnLoad()
{
	CTaskMain::OnLoad();
	if (m_pRoomSelector983)
		m_pRoomSelector983->LoadScript983();
}

void CLobbyMain::OnInit()
{
	g_pFresh->GetManager()->SetExclusiveKey(true);

	CTaskMain::OnInit();

	WPolySoup::sCamera* camera =
		GOLFDOC()->m_pPolySoup->FindCamera("DefaultCamera");
	if (camera)
	{
		g_camera = camera->mat;
		g_view->SetFOV(camera->fov * g_DEGTORAD);
	}

	m_bgCameraDir = -1.0f;
	m_bBgCameraPause = false;
	m_bgCameraStep = -1;
	m_bgCameraDuration = 0.0f;
	m_bgCameraTime = 0.0f;


	for (int i = 0; i < 3; i++)
	{
		m_pLoginBmp[i] = (Bitmap*)g_pFresh->GetBitmap(MakeStr("login_%02d", i));
	}

	m_pNewBmp = (Bitmap*)g_pFresh->GetBitmap("tap_new2");
}

void CLobbyMain::OnDestroy()
{
	CTaskMain::OnDestroy();
}

// 983 00ab5be0: balanco da camera do lobby em pernas de 10 s (a primeira 5 s), rampa de 2 s, pausa de 1 s; o passo
// (-1 no OnInit) inverte a direcao quando != 0 (o 645 andava 80 s a 0.3 por segundo e ia ate os cantos)
void CLobbyMain::ProcessLobbyBgCamera(float dt)
{
	if (Doc()->m_bBackgroundVideo || !Doc()->m_bLobbyBgCameraSway983)
		return;

	if (m_bgCameraDuration == 0.0f)
		m_bgCameraDuration = 5.0f;

	if (m_bBgCameraPause)
	{
		m_bgCameraTime += dt;
		if (m_bgCameraTime > 1.0f)
		{
			if (m_bgCameraStep)
				m_bgCameraDir *= -1.0;
			m_bBgCameraPause = false;
			m_bgCameraStep = (int)(m_bgCameraStep + m_bgCameraDir);
			m_bgCameraTime = 0.0f;
			m_bgCameraDuration = 10.0f;
		}
	}
	else if (m_bgCameraTime <= m_bgCameraDuration)
	{
		float ratio = 1.0f;
		m_bgCameraTime += dt;
		if (m_bgCameraTime < 2.0f)
			ratio = m_bgCameraTime * 0.5;
		else if (m_bgCameraTime > 8.0f)
			ratio = 1.0f - (m_bgCameraTime - 8.0f) * 0.5;
		g_camera.pivot += g_camera.xa * m_bgCameraDir * dt * 0.15f * ratio;
	}
	else
	{
		m_bgCameraTime = 0.0f;
		m_bBgCameraPause = true;
	}
}

void CLobbyMain::ProcessAdministrator(float dt)
{
	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
	{
	}
}

void CLobbyMain::ProcessDialogKeyboard(float dt)
{
	if (m_pNoticeDlg)
	{
		if (g_input->GetDown("\xc1\xc2", false))
			m_pNoticeDlg->Prev();
		else if (g_input->GetDown("\xbf\xec", false))
			m_pNoticeDlg->Next();
	}

	if (m_pPCBangNoticeDlg)
	{
		if (g_input->GetDown("\xc1\xc2", false))
			m_pPCBangNoticeDlg->Prev();
		else if (g_input->GetDown("\xbf\xec", false))
			m_pPCBangNoticeDlg->Next();
	}

	if (m_pGuildNoticeDlg)
	{
		if (g_input->GetDown("\xc1\xc2", false))
			((FrGuildNoticeDlg*)m_pGuildNoticeDlg)->Prev();
		else if (g_input->GetDown("\xbf\xec", false))
			((FrGuildNoticeDlg*)m_pGuildNoticeDlg)->Next();
	}
}

void CLobbyMain::ProcessLobbyPetSync(float dt)
{
	for (std::map<unsigned long, sRoomSlot>::iterator it =
			 Doc()->m_roomSlotMap.begin();
		it != Doc()->m_roomSlotMap.end(); it++)
	{
		if (it->second.pExhibition)
		{
			it->second.pExhibition->Process(dt, false);

			// JP 983 (00ab8d60; PKT lote 21): o estado da sala (CSharedDoc 0062bf90) no lugar do nome do layout
			if (it->first == MyGuid(false) && DOCJP()->GetState983() == 3)
			{
				if (it->second.pExhibition->IsDirectionChanged())
				{
					float dir = it->second.pExhibition->GetDirection();

					WSendPacket packet((enumClientPacket)0x63);
					packet.Encode1(0);
					packet.EncodeBuffer(&dir, 4);
					packet.Send(TO_GAME);
				}

				if (it->second.pExhibition->IsNextMotion())
				{
					WSendPacket packet((enumClientPacket)0x63);
					packet.Encode1(1);
					packet.EncodeStr(
						std::string(it->second.pExhibition->GetMotionName()));
					packet.Send(TO_GAME);
				}
			}
		}
	}
}

void CLobbyMain::ProcessLobbyUI(float dt)
{
	if (!strcmpi(g_pFresh->GetManager()->GetLayoutID(), "LOGIN") &&
		!m_bLoginBlock)
	{
		if (m_pLoginDlg)
		{
			if ((FrLoginDlg::GetState() == (eLoginState)0 ||
					FrLoginDlg::GetState() == (eLoginState)5) &&
				timeGetTime() - g_mouse->GetLastInputTime() > 60000 &&
				timeGetTime() - g_ime->GetLastInputTime() > 60000)
			{
				m_pLoginDlg->SetFadeout(true);

				CMouseCursor::Instance()->SetActive(false);
			}
		}
		else
		{
			if ((g_mouse->IsUpdated() || g_ime->IsUpdated()) && !g_bQuit)
			{
				m_pLoginDlg = CreateForm<FrLoginDlg>(g_pFresh->GetManager(),
					this, "login", NULL);
				m_pLoginDlg->Open(
					(FRESH_PFN_RESULT)&CLobbyMain::OnLoginDlgResult, 0x80a); // 983 00ab7b28
				CMouseCursor::Instance()->SetActive(true);
			}
		}
	}

	// 983 00ab7bc2: icone de idioma do chat do lobby e do recado pelo IME aberto/fechado (0099f050, 00f02fb4)
	static bool s_bImeOpened = CChatMsg::Instance()->IME_IsImeOpened();
	if (CChatMsg::Instance()->IME_IsImeOpened() != s_bImeOpened)
	{
		s_bImeOpened = CChatMsg::Instance()->IME_IsImeOpened();
		if (m_pOnelineReqDlg)
			m_pOnelineReqDlg->SetLanguage(s_bImeOpened); // 983 008983c0: nao zero = chat_eng
		if (m_pRoomListForm983)
			((FrChatDlg*)m_pRoomListForm983)->SetLanguage983(s_bImeOpened);
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetLanguage983(s_bImeOpened);
	}

	if (IsLocalContent((localContentType_t)0x19 /*983: era 0x18; interpolado*/) &&
		!IsLocalContent((localContentType_t)0x49 /*983: era 0x48; interpolado*/))
	{
		if (m_pToppageEvent1)
		{
			if (m_pToppageEvent1->IsVisible())
			{
				WRect rect;
				rect = m_pToppageEvent1->GetRect();
				rect.y += dt * m_event1Dir * 8.0f;

				if ((rect.y > m_event1Rect.y + 15.0f && m_event1Dir > 0.0f) ||
					(rect.y < m_event1Rect.y - 15.0f && m_event1Dir < 0.0f))
					m_event1Dir *= -1.0f;

				m_pToppageEvent1->MoveWindow(WPoint(rect.x, rect.y));
			}
		}
	}

	// 983 00ab7dc1: sem isso os medidores da criacao de personagem ficam em 0 (Draw usa m_target)
	if (g_pFresh->IsCurrentLayout("CREATE"))
	{
		for (int i = 0; i < 5; i++)
		{
			if (m_pStatGauge983[i])
			{
				float gaugeDt = dt > 0.02f ? 0.02f : dt;
				m_pStatGauge983[i]->Process(gaugeDt);
			}
		}
	}
}

// 983 (00ab5e00): estado 4 no lugar do layout GAMEROOM_EXT; so os avisos, o 983 nao chama mais o Start aqui
void CLobbyMain::ProcessAutoStart(float dt)
{
	sRoomInfo* pRoom = &Doc()->m_roomInfo;

	if (m_autoStartTime > 0.0f && DOCJP()->GetState983() == 4 &&
		(m_bCanStart || m_bAutoStart) &&
		pRoom->nUserNum >= Max((Doc()->m_curGameServer.property & 2) ? 5 : 10,
							   pRoom->nUserLimit * 3 / 4) &&
		Doc()->m_roomInfo.gameType != GAME_TYPE_GUILD_MATCH &&
		!pRoom->bAdminMaster)
	{
		m_autoStartTime -= dt;

		const char* text;
		if (m_autoStartTime < 6.0f && m_autoStartTime + dt > 6.0f)
			text = K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xc2\xfc\xbf\xa9\xc0\xce\xbf\xf8\xc0\xcc \xc0\xfc\xbf\xf8 \xc1\xd8\xba\xf1\xbf\xcf\xb7\xe1\xb0\xa1 \xb5\xc7\xbe\xfa\xc0\xb8\xb9\xc7\xb7\xce 5\xc3\xca \xc8\xc4\xbf\xa1 \xb0\xd4\xc0\xd3\xc0\xbb \xc0\xda\xb5\xbf \xbd\xc3\xc0\xdb\xc7\xd5\xb4\xcf\xb4\xd9.");
		else if (m_autoStartTime < 5.0f && m_autoStartTime + dt > 5.0f)
			text = K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xb0\xd4\xc0\xd3\xbd\xc3\xc0\xdb 5\xc3\xca\xc0\xfc\xc0\xd4\xb4\xcf\xb4\xd9.");
		else if (m_autoStartTime < 4.0f && m_autoStartTime + dt > 4.0f)
			text = K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xb0\xd4\xc0\xd3\xbd\xc3\xc0\xdb 4\xc3\xca\xc0\xfc\xc0\xd4\xb4\xcf\xb4\xd9.");
		else if (m_autoStartTime < 3.0f && m_autoStartTime + dt > 3.0f)
			text = K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xb0\xd4\xc0\xd3\xbd\xc3\xc0\xdb 3\xc3\xca\xc0\xfc\xc0\xd4\xb4\xcf\xb4\xd9.");
		else if (m_autoStartTime < 2.0f && m_autoStartTime + dt > 2.0f)
			text = K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xb0\xd4\xc0\xd3\xbd\xc3\xc0\xdb 2\xc3\xca\xc0\xfc\xc0\xd4\xb4\xcf\xb4\xd9.");
		else if (m_autoStartTime < 1.0f && m_autoStartTime + dt > 1.0f)
		{
			m_bAutoStart = false;
			text = K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xb0\xd4\xc0\xd3\xbd\xc3\xc0\xdb 1\xc3\xca\xc0\xfc\xc0\xd4\xb4\xcf\xb4\xd9.");
		}
		else
			return;

		this << MsgObject(NULL, 3, (int)text, 0xffff7878, 0, 0, 0);
	}
	else
	{
		if (Doc()->m_roomInfo.gameType == GAME_TYPE_GUILD_MATCH)
			m_autoStartTime = 6.0f;
		else
			m_autoStartTime = 15.0f;
	}
}

void CLobbyMain::OnProcess(float dt)
{
	CTaskMain::OnProcess(dt);

	ProcessLobbyBgCamera(dt);
	ProcessLobbyPetSync(dt);
	ReanchorRightIcons983();

	if (IsLocalContent(S3_GM_TOOLKIT))
		ProcessAdministrator(dt);

	ProcessDialogKeyboard(dt);
	ProcessLobbyUI(dt);
	ProcessAutoStart(dt);

	if (IsLocalContent((localContentType_t)0x4d /*983: era 0x4c; interpolado*/) &&
		g_pFresh->IsCurrentLayout("TOPPAGE"))
		AnimateWebEventButton(dt);

	if (IsLocalContent(S4_UCC))
	{
		RefreshAllUccClothes();
	}

	ProcessPremiumEndNotice983(dt);
}

// 983 (cauda TOPPAGE do 00ab7a70, OnProcess 00ab8f60): com o byte CSharedDoc+0x1160a ligado e o aviso escondido, mostra-o
// (topo) e reinicia o fade (alpha 1, 0.2/s); visivel, o alpha cai ate 0 (o OWNERDRAW 00597160 o desliga em 0).
void CLobbyMain::ProcessPremiumEndNotice983(float dt)
{
	if (strcmpi(g_pFresh->GetManager()->GetLayoutID(), "TOPPAGE") || !m_pToppagePremiumEndNotice983)
		return;
	if (!m_pToppagePremiumEndNotice983->IsVisible() && Doc()->m_bPremiumEndNotice983)
	{
		m_pToppagePremiumEndNotice983->SetVisible(true);
		m_pToppagePremiumEndNotice983->Enable(true);
		m_pToppagePremiumEndNotice983->SetTopmost(true);
		*(float*)&m_premiumEndNoticeAlpha983 = 1.0f;
		*(float*)&m_premiumEndNoticeFadeSpeed983 = 0.2f;
	}
	if (m_pToppagePremiumEndNotice983->IsVisible())
	{
		float& alpha = *(float*)&m_premiumEndNoticeAlpha983;
		alpha -= *(float*)&m_premiumEndNoticeFadeSpeed983 * dt;
		if (alpha < 0.0f)
			alpha = 0.0f;
	}
}

void CLobbyMain::AnimateWebEventButton(float dt)
{
	static float s_time;
	static int s_index;
	static int s_frame;
	s_time += dt;
	if (s_time > 0.3f)
	{
		s_time = 0.0f;
		int frames[4] = { 1, 2, 3, 2 };

		s_index++;
		if (s_index > 3)
			s_index = 0;

		s_frame = frames[s_index];
		char buf[4] = { 0 };
		std::string name = "xmas_main_i";
		name += itoa(s_frame, buf, 10);

		if (m_pToppageChristmas2007Event)
		{
			m_pToppageChristmas2007Event->SetButtonImg(name.c_str(),
				FrButton::NORMAL);
			m_pToppageChristmas2007Event->SetButtonImg(name.c_str(),
				FrButton::OVER);
		}
	}
}

void CLobbyMain::OnDisplay()
{
}

void CLobbyMain::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x5b):
	{
		std::map<unsigned long, sRoomSlot>::iterator it =
			Doc()->m_roomSlotMap.find(msg.param1);
		if (it == Doc()->m_roomSlotMap.end())
			return;

		if (Doc()->m_roomSlotMap[msg.param1].pExhibition == NULL)
			return;

		switch (msg.param2)
		{
		case 0:
			Doc()->m_roomSlotMap[msg.param1].pExhibition->SetDirection(
				*(float*)msg.param3);
			break;

		case 1:
			Doc()->m_roomSlotMap[msg.param1].pExhibition->SetMotion(
				(const char*)msg.param3, false, 0.0f);
			break;
		}
	}
	break;

	case M983(0x59):
	{
		std::map<unsigned long, sRoomSlot>::iterator it;
		for (it = Doc()->m_roomSlotMap.begin();
			it != Doc()->m_roomSlotMap.end(); ++it)
		{
			if (it->second.pExhibition)
			{
				delete it->second.pExhibition;
				it->second.pExhibition = NULL;
			}
		}
		Doc()->m_roomSlotMap.clear();

		if (m_pCreatePetArea983)
			m_pCreatePetArea983->SetMouseEvent(false);
	}
	break;

	case M983(0x5a):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->AddRoomSlot983((sSlotInfo*)msg.param1, (sCharacterInfo*)msg.param2); // 0050d700
		break;

	case M983(0x44):
		OpenEventPrizeForm((unsigned char)msg.param1);
		break;

	case 0:
	{
		if (g_pFresh == NULL)
			return;

		const char* layout = (const char*)msg.param1;

		if (g_pFresh->IsCurrentLayout(layout))
			return;

		CloseAllDialogs();
		ResetControl();

		// 983 (00abc402..00abc4a0; B159.2): a TOPPAGE do lobby e a area de trabalho da RF (RFLobbyDesktop, lobby_desktop.xml);
		// as outras paginas apagam a area de trabalho. Sem SetWhisper e sem CheckTikiReport (do 645).
		if (!stricmp(layout, "TOPPAGE"))
		{
			g_lobbyModal983 = 0;
			if (RFLobbyDesktop::Instance983())
				RFLobbyDesktop::Instance983()->OnShowDesktop983(true); // vt+0x88
			else
			{
				RFLobbyDesktop* pDesktop = RFLobbyDesktop::Create983(NULL);
				if (pDesktop)
					pDesktop->OnShowDesktop983(true);
			}
		}
		else if (RFLobbyDesktop::Instance983())
			RFSceneManager::GetManager983()->DeleteDesktop();

		g_pFresh->OpenLayout(layout, this, false);
		if (!stricmp(layout, "LOGIN"))
			CBGMPlayList983::Instance()->PlayDefault983(true); // 00abc476
		CMouseCursor::Instance()->Toggle(true);
	}
	break;

	case M983(0x3):
		// 983 00abc4a5: a linha vai para a lista de salas (+0x77c) e para a sala (+0x780), ambas FrChatDlg (00490d10);
		// o m_pChatView do 645 nao existe no lobby do 983 (D17: a linha nao aparecia no chat da lista de salas)
		if (m_pRoomListForm983)
			((FrChatDlg*)m_pRoomListForm983)->AddChatLine983((const char*)msg.param1, msg.param2);
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->AddChatLine983((const char*)msg.param1, msg.param2);
		break;

	case M983(0x60):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OnChatFace983(std::string((const char*)msg.param1), std::string((const char*)msg.param2)); // 0050d280
		break;

	// 983 00abc140: 0xa..0xd vao para a lista de salas aberta (+0x77c) e para a FrUserListDlg (+0x424)
	case M983(0xa):
	{
		sBriefUserInfo* pUser = (sBriefUserInfo*)msg.param1;
		bool me = pUser->dwGuid == MyGuid(false);
		if (me && (Doc()->m_myInfo.info.dwIdentity & 4) && (pUser->state & 0x400))
			break;
		if (!me && !(Doc()->m_myInfo.info.dwIdentity & 4) && (pUser->dwIdentity & 0x14) && !(pUser->state & 1))
			break;
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->AddUser983(pUser);
		if (m_pUserListDlg)
		{
			m_pUserListDlg->AddUser(pUser);
			m_pUserListDlg->SortUserList();
		}
	}
	break;

	case M983(0xc):
	{
		sBriefUserInfo* pUser = (sBriefUserInfo*)msg.param1;
		if (pUser->dwGuid == MyGuid(false) && (Doc()->m_myInfo.info.dwIdentity & 4) && (pUser->state & 0x400))
			break;
		if (m_pUserListDlg)
			m_pUserListDlg->DelUser(pUser, msg.param2);
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->DelUser983(pUser, msg.param2);
	}
	break;

	case M983(0xb):
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->ClearUserList983();
		break;

	// 983 00abc64c/00abc66a/00abc688: sala nova, sala que saiu e limpar vão à lista aberta (+0x77c: 0051d060/0051d090/
	// 0051d0c0 -> CNormalLobbyRoom::AddRoom983/DelRoom983); a ROOMLIST do 645 não existe no 983 (C457)
	case M983(0x4):
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->Room1f0_983(msg.param1);
		break;

	case M983(0x5):
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->Room220_983(msg.param1);
		break;

	case M983(0x6):
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->ClearRoomList983();
		break;

	case M983(0xd):
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->PadRoomList983(msg.param1);
		break;

	case 0x87: // 983 00abc140: Grand Prix, sem par no 645
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->AddGrandPrixAI983(); // 0050bc80
		break;

	case M983(0x7):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OnEnterUser983((sSlotInfo*)msg.param1, (sCharacterInfo*)msg.param2); // 0050d4c0
		break;

	case M983(0x8):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OnLeaveUser983((void*)msg.param1, (unsigned long)msg.param2, msg.param3); // 00509440
		break;

	case M983(0x9):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OnChangeWings983((sSlotInfo*)msg.param1); // 00509700
		break;

	case M983(0xe):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			{
			pDlg->MakeRoomUserList(); // 0050eb20
			pDlg->UpdateMaster983(); // 0050c760
		}
		break;

	case M983(0xf):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->UpdateMaster983(); // 0050c760
		break;

	case M983(0x10):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetRoomTitle983(NULL); // 00508a50
		break;

	case M983(0x11):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetCourse983((unsigned char)msg.param1); // 0050d120
		break;

	case M983(0x12):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->ReLoadHoleItem(msg.param1); // 005034e0
		break;

	case M983(0x16):
		m_bCreateRoom = msg.param1 != 0;
		break;

	case M983(0x17):
		*(int*)msg.param1 = m_bCreateRoom;
		break;

	case M983(0x18):
	{
		for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
			it != Doc()->m_slotList.end(); ++it)
			it->bSleep = 0;

		Doc()->m_bGameOver = false;

		CMouseCursor::Instance()->SetMode(1);
		CMouseCursor::Instance()->SetActive(false);
		CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);
	}
	break;

	// 983 (00abe199, s109): mensagem nova do 983 (sem par no 645). O pacote 0x52 (inicio de jogo, 009c2ad2) desvia para a
	// espera do Grand Prix (msg 0x753e/0xd); a janela (FrGPTypeRoomDlg::OnWaitingOwnerDraw 005155db) manda 0x86 no fim da
	// contagem e aqui se faz o resto do 0x52 (o mesmo do fim do caso no OnPacketCommon); cauda comum com o 0x18 (00abc826).
	case 0x86:
	{
		if (Doc()->m_golfGame.gameType != 0xa && IsMassGame())
			Doc()->m_approachStartTime = g_CurrentTime;

		if (IsMassGame())
			CPowerGauge::Instance()->ResetRandTable983();

		Doc()->m_roomSetupStage = 0;
		Doc()->LogGstUpdateHole(0xffffffff, 0xff, -1, -1);

		if (IsLocalContent((localContentType_t)0x18))
			Doc()->SetInitWearHalloweenItem();

		if (CMessengerInfo::Instance()->GetMyStatus() != 3)
			CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)0);

		for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
			it != Doc()->m_slotList.end(); ++it)
			it->bSleep = 0;

		Doc()->m_bGameOver = false;

		GetActor("Lobby") << MsgObject(NULL, 0x314, 0, 0, 0, 0, 0); // 983: numero sem par no 645

		CMouseCursor::Instance()->SetMode(1);
		CMouseCursor::Instance()->SetActive(false);
		COption::Instance()->vSetMinimumScreenSize(0, 0x320, 0x258);
		CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);
	}
	break;

	case M983(0x1e):
		if (m_pLoginDlg)
		{
			// 983 00abc84c: 0x1c traz o codigo do ErrorCodeInfo em param2
			if (msg.param1 == 0x1c)
				FrLoginDlg::SetErrorState983(msg.param2);
			else
				FrLoginDlg::SetState((eLoginState)msg.param1);
			return;
		}

		if (msg.param1 == 13)
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"CREATE",
				0, 0, 0);
		break;

	case M983(0x1f):
		if (m_pServerDlg)
			m_pServerDlg->SetState((gsLoginState_t)msg.param1);
		break;

	case M983(0x30):
	{
		if (!IsLocalContent(S4_INVITE_FRIEND))
			return;

		if (m_pInviteDlg || Doc()->m_myInfo.roomIndex != 0xffff ||
			!COption::Instance()->gIsInvitationAllowed())
			return;

		m_pInviteDlg =
			CreateForm<FrInviteDlg>(g_pFresh->GetManager(), this, "invite");
		m_pInviteDlg->SetInvaiteInfo(msg.param1, (unsigned char)msg.param2,
			(unsigned short)msg.param3, msg.param4, MyUID());
		std::string* pName = (std::string*)msg.param5;
		m_pInviteDlg->SetMessage(
			MakeStr(
				K2L_Compatibility("(%s)\xb4\xd4\xb2\xb2\xbc\xad \xb4\xe7\xbd\xc5\xc0\xbb "
				"\xc3\xca\xb4\xeb\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.\n"
				"\xc3\xca\xb4\xeb\xbf\xa1 \xc0\xc0\xc7\xcf\xbd\xc3\xb0\xda"
				"\xbd\xc0\xb4\xcf\xb1\xee?"),
				pName->c_str(), 0),
			false);
		m_pInviteDlg->SetTimeLimit(5.0f);
		m_pInviteDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnInviteDlgResult, 1);
	}
	break;


	case M983(0x23):
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
			(FrCmdTarget*)this, "notify");
		pForm->SetMessage((const char*)msg.param1, false);
		// 983 00abc140 (case 0x23): tempo limite so com param2 == 0; Open(NULL, 0x803) e nada depois (sem SetTopmost/
		// FindNextTopFocus); o 3 deixava o veu escuro na compra da loja (C374.2)
		if (msg.param2 == 0)
			pForm->SetTimeLimit(5.0f);
		pForm->Open(NULL, 0x803);
	}
	break;

	case M983(0x25):
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
			(FrCmdTarget*)this, "notify");
		pForm->SetMessage((const char*)msg.param1, false);
		pForm->Open((FRESH_PFN_RESULT)&CLobbyMain::OnNotifyQuitDlgResult, 0x803);
	}
	break;

	case M983(0x27):
		strcpy(m_roomPassword, (const char*)msg.param1);
		break;

	case M983(0x28):
		*(char**)msg.param1 = m_roomPassword;
		break;

	case M983(0x29):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OnReadyState983((unsigned long)msg.param1, msg.param2); // 0050ca30
		break;

	case M983(0x46):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetTeamChangeBtn983((unsigned long)msg.param1); // 00507330
		break;

	case M983(0x49):
	{
		// 983 (00abc140, caso 0x48): antes de tudo repassa ao form da widget (My Room: troca de personagem)
		if (FrTaskForm* pWidgetForm = AfxGetTask()->GetWidgetForm983())
			pWidgetForm->OnMsg983(msg);
		unsigned long guid = msg.param1;
		sCharacterInfo* pCharInfo = (sCharacterInfo*)msg.param2;
		jp983_rastro("A803 lobby 0x49 user=%08x char=%08x tid=%08x form=%p", guid, pCharInfo->guid, pCharInfo->tid,
			AfxGetTask()->GetWidgetForm983());
		// 983 (00abd016..00abd02e): so a sala trata (0050d390); o 645 gravava guidChar/BuildMyPartTidList aqui
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OnChangeChar983(guid, pCharInfo);
	}
	break;

	case M983(0x4a):
	{
		unsigned long guid = msg.param1;
		if (guid != MyGuid(true))
			return;

		Doc()->m_userInfo[0].caddieInfo = *(sCaddieInfo*)msg.param2;
		Doc()->m_myInfo.userEquip.guidCaddie =
			Doc()->m_userInfo[0].caddieInfo.guid;
		// 983 00abc140: as barras sao da sala (0050b570), o SetLevelBar do lobby so serve ao CREATE
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetLevelBar();
		// 983 (00abd0ba): depois do LevelBar repassa ao form da widget (My Room: seleciona o caddie)
		if (FrTaskForm* pWidgetForm = AfxGetTask()->GetWidgetForm983())
			pWidgetForm->OnMsg983(msg);
	}
	break;

	case M983(0x4b):
	{
		unsigned long guid = msg.param1;
		if (guid != MyGuid(true))
			return;

		Doc()->m_userInfo[0].clubInfo = *(sClubInfo*)msg.param2;
		Doc()->m_myInfo.userEquip.guidClubSet =
			Doc()->m_userInfo[0].clubInfo.guid;
		// 983 00abc140: as barras sao da sala (0050b570), o SetLevelBar do lobby so serve ao CREATE
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetLevelBar();
	}
	break;

	case M983(0x4c):
	{
		unsigned long guid = msg.param1;
		if (guid != MyGuid(true))
			return;

		Doc()->m_myInfo.userEquip.tidBall = msg.param2;
		Doc()->m_userInfo[0].userEquip.tidBall = msg.param2;
		// 983 00abc140: as barras sao da sala (0050b570), o SetLevelBar do lobby so serve ao CREATE
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetLevelBar();
	}
	break;

	case M983(0x4e):
	{
		unsigned long guid = msg.param1;
		if (guid != MyGuid(true))
			return;

		Doc()->m_userInfo[0].mascotInfo = *(sMascotInfo*)msg.param2;
		Doc()->m_myInfo.userEquip.guidMascot =
			Doc()->m_userInfo[0].mascotInfo.guid;
		// 983 00abc140: as barras sao da sala (0050b570), o SetLevelBar do lobby so serve ao CREATE
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetLevelBar();
	}
	break;

	case M983(0x2b):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetLockControls983(msg.param1); // 00503720
		break;

	case M983(0x2c):
		Doc()->m_underBarMask |= msg.param1;

		if (Doc()->m_underBarMask == 0x7f) // 983 00abd223; bits 1,2,4,0x10,0x20,0x40: ninguem manda 8 (morto nos 2)
			Doc()->SortAllMyItemList();
		break;

	case M983(0x2a):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SortRank(); // 00505640
		break;

	case M983(0x2d):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OpenResult983(); // 005070c0
		break;

	case M983(0x2e):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->m_b18d = false; // 004ff290(0)
		break;

	case M983(0x32):
		if (m_pUserListDlg)
			m_pUserListDlg->MakeUserList(Doc()->m_briefUserInfoMap);
		break;

	case M983(0x33):
		// 983 00abd2ab: sala comum (param1 0) e Grand Prix vão os dois à lista aberta (+0x77c, 0051fe80); a lista
		// ROOMLIST do 645 no CLobbyMain não existe no 983
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->OpenDtRoomDlg983(msg);
		break;

	case M983(0x34):
		memcpy(Doc()->m_myInfo.userEquip.tidItemSlot, (void*)msg.param1,
			sizeof(Doc()->m_myInfo.userEquip.tidItemSlot));
		break;

	case M983(0x35):
		if (IsLocalContent(S3_PARAN_CHANNELING))
		{
			OpenParanNickChange();
			return;
		}
		OpenCaddieWarningForm();
		break;

	case M983(0x37):
	{
		if (CUserInfo::Instance()->IsVisible())
			CUserInfo::Instance()->Close();

		if (m_pUserListDlg)
			m_pUserListDlg->Close(true);

		// 983 00abc140 caso 0x37: o sussurro vai ao chat da lista de salas e da sala (FrChatDlg 0048fcb0)
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->SetChatTarget983((const char*)msg.param1);
		if (m_pGameRoomDlg983)
			GameRoomDlg983()->SetChatTarget983((const char*)msg.param1);
	}
	break;

	case M983(0x38):
		// 983 00abc140 caso 0x38 (0048fd00)
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->AddWhisperPartner983((const char*)msg.param1);
		if (m_pGameRoomDlg983)
			GameRoomDlg983()->AddWhisperPartner983((const char*)msg.param1);
		break;

	case M983(0x47):
		// 983 00abc140 caso 0x46 (0048fd20)
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->ResetChatTarget983();
		if (m_pGameRoomDlg983)
			GameRoomDlg983()->ResetChatTarget983();
		break;

	case M983(0x3a):
	{
		if (!(Doc()->m_curChannel.Type & 2))
			return;

		std::list<sChannelInfo>::iterator it = Doc()->m_channelList.begin();

		for (; it != Doc()->m_channelList.end(); ++it)
		{
		}
	}
	break;

	case M983(0x2f):
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->SetTeamControls983(); // 00507380
		break;

	case M983(0x3b):
	{
		switch ((unsigned char)msg.param1)
		{
		case 2:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc1\xa4\xbf\xf8\xc0\xcc \xc3\xca\xb0\xfa "
					 "\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			break;

		case 3:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xb4\xc2 "
					 "\xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			break;

		case 4:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 "
					 "\xc6\xb2\xb7\xc8\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			break;

		case 7:
		case 9:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb9\xe6\xb8\xb8\xb5\xe9\xb1\xe2 \xbf\xe4\xc3\xbb\xc0\xcc "
					 "\xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			break;

		case 8:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb0\xd4\xc0\xd3 \xc1\xf8\xc7\xe0\xc1\xdf\xc0\xce "
					 "\xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			break;

		case 5:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb7\xb9\xba\xa7\xbf\xa1\xbc\xad\xb4\xc2 \xb9\xe6\xc0\xbb \xb8\xb8\xb5\xe9\xb0\xc5\xb3\xaa \xb9\xe6\xbf\xa1 \xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;

		case 13:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1 \xb0\xa1\xc0\xd4\xc7\xd8\xbe\xdf "
					 "\xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;

		case 18:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc8\xaf\xbb\xf3\xc0\xc7 \xc6\xce\xbe\xdf\xbc\xb6\xbf\xa1\xbc\xad\xb4\xc2 \xb9\xe6 \xb0\xb3\xbc\xb3\xc0\xcc \xba\xd2\xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;

		case 12:
		{
			// 983 00abd5xx (HandleMsg, caso 0xc): sem o sim/nao do canal 0x80 do 645
			if ((Doc()->m_curChannel.Type & 0x400) &&
				Doc()->m_myInfo.stat.Level < 11 &&
				!Doc()->m_myInfo.info.IsIdentity(4))
			{
				this << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xc0\xd6\xb4\xc2 \xb9\xe6\xc0\xbb \xc3\xa3\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
					0, 0, 0, 0);
				return;
			}

			WSendPacket packet((enumClientPacket)8);
			packet.Encode1(0);

			unsigned char gameType = 0x17; // JP 983 (00abd734): 0x10 no KR 645
			unsigned char map = FirstMap();

			if (Doc()->m_curChannel.Type & 2)
			{
				packet.Encode4(0);
				packet.Encode4(2400000);

				packet.Encode1(Doc()->m_gameTypeInfo[4].maxPlayer);

				packet.Encode1(4);
				packet.Encode1(18);
			}
			// JP 983 (00abd7a9, C-PKT lote 10): o ramo do canal 0x80 (tipo 3, 9 buracos) do KR 645 nao existe.
			else if (Doc()->m_curChannel.Type & 0x4000)
			{
				packet.Encode4(40000);
				packet.Encode4(0);

				packet.Encode1(Doc()->m_gameTypeInfo[7].maxPlayer);

				packet.Encode1(7);
				packet.Encode1(6);
			}
			else
			{
				gameType = GetRandGameTypeByCate(m_quickStartCate);

				packet.Encode4(GetShotTimeLimitByGameType(gameType));
				packet.Encode4(GetGameTimeLimitByGameType(gameType));

				packet.Encode1(GetGameTypeInfo(gameType)->maxPlayer);

				packet.Encode1(gameType);
				packet.Encode1(GetGameTypeInfo(gameType)->holes);

				if (gameType == 10)
					map = 0x7f;
			}

			packet.Encode1(map);
			packet.Encode1(gameType == 10 ? 2 : 0);
			packet.Encode4(0);	// 983 00abd8f7: dword novo antes do titulo (lote 9 da C-PKT)
			packet.EncodeStr(std::string(Doc()->GetDefaultTitle(gameType)));
			packet.EncodeStr(std::string(""));
			packet.Encode4(0);	// 983 00abd95c: dword novo no fim (o FrMakeRoomDlg manda aqui o artefato da sala)
			packet.Send(TO_GAME);

			this << MsgObject(NULL, M983(0x16), 1, 0, 0, 0, 0);
		}
			return;

		case 15:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc6\xce\xb9\xe8\xc6\xb2\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xc6\xce\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			break;

		case 17:
			this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1\xb0\xd4\xc0\xd3\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xc6\xce\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;
		}

		// 983 00abda54: recusada a entrada, a lista volta a aceitar clique (0051ed90)
		if (m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->EnableRoomEnterControls(true);
	}
	break;

	case M983(0x3d):
	{
		std::list<sSlotInfo>::iterator it = std::find(Doc()->m_slotList.begin(),
			Doc()->m_slotList.end(), msg.param1);
		if (it == Doc()->m_slotList.end())
			return;

		it->bSleep = msg.param2 == 1;
	}
	break;

	case M983(0x3e):
		OpenCreateInfo(msg.param1, (const char*)msg.param2);
		break;

	case M983(0x3f):
		*(const char**)msg.param1 =
			NULL;
		break;

	case M983(0x40):
		if (IsLocalContent(S3_PARAN_CHANNELING))
		{
			if (m_pParanNickDlg)
			{
				if (msg.param1 == 0)
					m_pParanNickDlg->CloseDlg();
				else
					m_pParanNickDlg->EnableControls(true);
			}
		}

		if (m_pOptionDlg)
		{
			if (msg.param1 == 0)
			{
				m_pOptionDlg->CloseDlg();
				return;
			}
			m_pOptionDlg->EnableChangeNickControls(true);
		}
		break;

	case M983(0x51):
	{
		FrUserInfoForm* pForm = USERINFODLG();
		if (pForm == NULL)
			return;

		if (CUserInfo::Instance()->IsVisible() == true)
		{
			if (pForm->IsOpenAddFriendDlg())
			{
				pForm->SetRequestFriendData((const char*)msg.param1,
					msg.param2);
			}
		}

		FrUserListDlg* pDlg = m_pUserListDlg;
		if (pDlg == NULL)
			return;

		if (!pDlg->IsOpenAddFriendDlg())
			return;

		pDlg->SetRequestFriendData((const char*)msg.param1, msg.param2);
	}
	break;

	case M983(0x52):
	{
		FrUserInfoForm* pForm = USERINFODLG();
		if (pForm)
			return;

		if (CUserInfo::Instance()->IsVisible() == true)
		{
			if (pForm->IsOpenAddFriendDlg())
			{
				pForm->SetRequestFriendData((const char*)msg.param1,
					msg.param2);
			}
		}
	}
	break;

	case M983(0x53):
		// 983 00abc140 caso 0x60: a janela e da sala (005033b0), so com a sala aberta
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->OpenNewRecordForm();
		break;

	case M983(0x56):
		// 983 (00abdc12): sem CTutorialTask - volta ao TOPPAGE e abre o widget do tutorial (0x88 so do 983)
		Doc()->m_bTutorialResume = false;
		CloseAllDialogs();

		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88 /*so do 983*/,
			(int)"CTutorialWidget", (int)"TOPPAGE", 0, 0);
		break;

	case M983(0x58):
		m_bAutoStart = true;
		break;

	case M983(0x5c):
	{
		FrCaddieInfoDlg* pDlg = (FrCaddieInfoDlg*)g_pFresh->GetManager()
									->GetDesktop()
									->FindChildByName("caddie_info");
		if (pDlg == NULL)
		{
			__rtti_obj = NULL;
			return;
		}

		pDlg->EndContract(msg.param1);
	}
	break;

	case M983(0x5e):
	{
		unsigned long guid = msg.param1;

		if (Doc()->m_teamPlayerGuid[0] != guid &&
			Doc()->m_teamPlayerGuid[1] != guid)
			return;

		sRivalData* pRival1 = GUILDMATCHUP(0);
		sRivalData* pRival2 = GUILDMATCHUP(1);

		if (pRival1 == NULL || pRival2 == NULL)
			return;

		int hole = msg.param2;

		if (pRival1->holeStroke[hole - 1] == 0 ||
			pRival2->holeStroke[hole - 1] == 0)
			return;

		char szMsg[1024];
		if (pRival1->holeStroke[hole - 1] < pRival2->holeStroke[hole - 1])
			sprintf(szMsg,
				K2L_Compatibility("%s\xb4\xd4\xc0\xcc %d\xc8\xa6\xbf\xa1\xbc\xad \xbd\xc2\xb8\xae\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9"),
				pRival1->nickname, hole);
		else if (pRival1->holeStroke[hole - 1] > pRival2->holeStroke[hole - 1])
			sprintf(szMsg,
				K2L_Compatibility("%s\xb4\xd4\xc0\xba %d\xc8\xa6\xbf\xa1\xbc\xad \xc6\xd0\xb9\xe8\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9"),
				pRival1->nickname, hole);
		else
			sprintf(szMsg,
				K2L_Compatibility("%d\xc8\xa6\xc0\xba \xb9\xab\xbd\xc2\xba\xce\xb0\xa1 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
				hole);

		this << MsgObject(NULL, M983(0x3), (int)szMsg, 0xffff7878, 0, 0, 0);
	}
	break;

	case M983(0x5f):
		// 983 00abddc9: o dono da sala vai para o doc da lista de salas (+0x14, lido pelo FrGameRoomDlg/FrSingleTypeDlg 0x0e x 0x0d)
		if (m_pRoomListDoc983)
			m_pRoomListDoc983->m_master = msg.param1;
		break;

	case M983(0xb3):
	{
		if (!Doc()->m_bBingoEvent)
			return;

		char szUrl[1024];
		sprintf(szUrl, "www.pangya.com/pang_event_bingo.asp?userid=%s", MyId());

		CBrowser::Instance()->OpenWeb983(szUrl, false, 0xc); // 983 (00abde18): navegador do 983 com o tipo 0xc (pacote 0xa1)
		Doc()->m_bBingoEvent = false;
	}
	break;

	// 983 (00abc8b4, s106; pedido C35): mensagem nova do 983 (sem par no 645) que abre as janelas do lobby pelo param1
	// (tabela 00abecf8). Feitos: 0 servidor, 1 criar sala, 2 partida rapida, 3 senha, 5 equipar. Faltam (dependem do
	// CLobbyMain no layout do 983 e das janelas novas da frente B): 4 (lista de salas / Grand Prix pelo tipo em param2,
	// "roomlist"/"grandprix_list", resultado 00ab6620), 6..0xb (00ab6cc0(3,4,5,8,6,7): "gameroom_main"/"gameroom_single")
	// e 0xc ("notify_tiki", resultado 00ab2fa0). 4 e 6..0xb: C-PKT lote 50.
	case 0x8b:
		switch (msg.param1)
		{
		case 0:
			// o 983 nao manda o 0x43 nem testa o canal aqui; o resultado e o do CTaskMain (00657150)
			m_pServerDlg = CreateForm<FrServerDlg>(g_pFresh->GetManager(), this, "server", NULL);
			m_pServerDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnServerDlgResult, // = CTaskMain::OnServerDlgResult
				WPoint(m_pServerDlg->GetRect().x, 50.0f), FrICON);
			m_pServerDlg->SetIconRect(m_pUnderBarServer->GetRect());
			break;
		case 1:
			m_pMakeRoomDlg = CreateForm<FrMakeRoomDlg>(g_pFresh->GetManager(), this, "makeroom", NULL);
			m_pMakeRoomDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnMakeRoomDlgResult, FrESCAPE | FrENTER);
			break;
		case 2:
			m_pQuickStartDlg = CreateForm<FrQuickStartDlg>(g_pFresh->GetManager(), this, "quick_start", NULL);
			if (m_pQuickStartDlg)
				m_pQuickStartDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnQuickStartDlgResult,
					WPoint(547.0f, 158.0f), 0xc0);
			break;
		case 3:
			if (m_pPasswordDlg)
				break;
			m_pPasswordDlg = CreateForm<FrPasswordDlg>(g_pFresh->GetManager(), this, "password", NULL);
			m_pPasswordDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnPasswordDlgResult, FrESCAPE | FrENTER);
			break;
		case 4:
		{
			// 983 (00abca66; C-PKT lote 50, pedido C56.2): lista de salas pelo tipo em param2 (CSharedDoc+0x11630, 0062bf40/50:
			// 0 = salas comuns "roomlist", 1 = Grand Prix "grandprix_list"); resultado OnRoomListDlgResult983 (00ab6620)
			CloseAllDialogs(); // vtable +0x3c = 00ab6480
			if (DOCJP()->GetRoomListKind983() == msg.param2 && m_pRoomListForm983)
				break;

			if (m_pRoomListForm983)
			{
				((FrForm*)m_pRoomListForm983)->Close((eFormRet)1, true);
				CLobbyState983* pState = CLobbyState983::Instance983();
				if (pState && pState->m_state == 9)
					CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x59 /*983; sem par no 645*/, 1, 0, 0, 0);
			}
			if (m_pGameRoomDlg983 && DOCJP()->GetState983() == 8)
				m_pGameRoomDlg983->Close((eFormRet)0, true);

			Doc()->m_roomListKind983 = msg.param2; // 0062bf40
			switch (DOCJP()->GetRoomListKind983())
			{
			case 0:
				m_pRoomListForm983 = (unsigned long)CreateForm<FrMultiplayRoomDlg>(g_pFresh->GetManager(), this, "roomlist", NULL);
				Doc()->m_lobbyState983 = 1; // 0062bf80
				break;
			case 1:
			{
				SetConstantBGM("bgm_grandprix_lobby.mp3", false, true);
				FrGrandPrixRoomDlg* pGrandPrixDlg =
					CreateForm<FrGrandPrixRoomDlg>(g_pFresh->GetManager(), this, "grandprix_list", NULL);
				m_pRoomListForm983 = (unsigned long)pGrandPrixDlg;
				// 0051ce50: os objetos de +0x794, +0x784 e +0x79c (OnPreLoadInit 00ab9800)
				pGrandPrixDlg->Set154_983((int)m_pGPTicketFont983.get(), (int)m_pGPTimerFont983.get(), (int)m_pGPDiffLevelFont983.get());
				Doc()->m_lobbyState983 = 2;
				break;
			}
			}

			FrRoomListDlg* pRoomListDlg = (FrRoomListDlg*)m_pRoomListForm983;
			WRect rect = pRoomListDlg->GetRect();
			rect.y += 35.0f;
			pRoomListDlg->SetRect(rect);
			pRoomListDlg->EnableDrag(false);
			pRoomListDlg->SetRoom983(m_pRoomSelector983);
			pRoomListDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnRoomListDlgResult983, 1);
			break;
		}
		case 5:
			OpenEquipDlg983(); // 00abcc38 (era o CLobbyMain::OnGameRoom_EquipItemDown do 645)
			break;
		// 983 (00abcc42..00abcc85; C-PKT lote 50): janela da sala pelo estado (00ab6cc0)
		case 6:
			OpenGameRoomDlg983(3);
			break;
		case 7:
			OpenGameRoomDlg983(4);
			break;
		case 8:
			OpenGameRoomDlg983(5);
			break;
		case 9:
			OpenGameRoomDlg983(8);
			break;
		case 0xa:
			OpenGameRoomDlg983(6);
			break;
		case 0xb:
			OpenGameRoomDlg983(7);
			break;
		case 0xc:
			// 983 (00abcc8a; pedido C47.2): "notify_tiki" com a quantidade de relatorios do Tiki (item 0x1a000041)
			if (!m_pUseTikiReportDlg983)
			{
				int count = 0;
				std::list<sItemInfo>::iterator itItem;
				for (itItem = Doc()->m_myItemList.begin(); itItem != Doc()->m_myItemList.end(); ++itItem)
				{
					if (itItem->tid == 0x1a000041)
						break;
				}
				if (itItem != Doc()->m_myItemList.end() && itItem->Common[0] > 0)
					count = itItem->Common[0];

				m_pUseTikiReportDlg983 = CreateForm<FrUseTikiReportDlg>(g_pFresh->GetManager(), this, "notify_tiki", NULL);
				m_pUseTikiReportDlg983->SetNotice983(K2L_Compatibility(
					"\\c0xffff0000\\c\xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae\\c0xff000000\\c\xb8\xa6 "
					"\xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc3\xb8\xe9,\n\xb4\xd9\xb8\xa5 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb5\xe9\xc0\xc7 "
					"\xbf\xcf\xc1\xd6\xb8\xa6 \xb1\xe2\xb4\xd9\xb8\xae\xc1\xf6 \xbe\xca\xb0\xed\n\xb4\xeb\xc8\xb8\xb8\xa6 "
					"\xb3\xa1\xb3\xbe \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n\n\n\\c0xffff0000\\c\xc6\xbc\xc5\xb0 "
					"\xb8\xae\xc6\xf7\xc6\xae\\c0xff000000\\c\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"));
				m_pUseTikiReportDlg983->SetAmount983(MakeStr(K2L_Compatibility(
					"\xc7\xf6\xc0\xe7\xbc\xf6\xb7\xae\n%d\xb0\xb3"), count));
				m_pUseTikiReportDlg983->Open((FRESH_PFN_RESULT)&CLobbyMain::OnUseTikiReportResult983, 1);
			}
			break;
		}
		break;

	case M983(0x61):
	{
		m_bCaddieWarning = true;
		int count = msg.param1;
		memset(m_caddieWarning, 0, sizeof(sCaddieInfo));
		memcpy(m_caddieWarning, (void*)msg.param2, count * sizeof(sCaddieInfo));
	}
	break;

	case M983(0x1f7):
		break;

	// 983 (00abe4ef): RFLobbyDesktop::SetScreen983 (005fbe70) avisa a troca de tela; fecha a janela das emocoes (00ab4500)
	// 983 (00abc140): janela das emocoes conforme o boneco do Lounge (LobbyAvatarActor manda ao clicar nele ou no vazio)
	case 0x192:
		if (!m_pMotionDlg983)
			break;
		if (msg.param1 != -1)
		{
			m_pMotionDlg983->Refresh983((const unsigned long*)&msg.param1);
			m_pMotionDlg983->m_selected = -1;
			break;
		}
		// -1 = nenhum boneco: fecha como o 0x1a0
	case 0x1a0:
		if (m_pMotionDlg983)
			m_pMotionDlg983->m_slide = 4;
		break;

	case 0x195:
		if (m_pMotionDlg983)
			m_pMotionDlg983->m_bPlaying = msg.param1 != 0;
		break;

	case 0x196:
		if (msg.param1 == 1 && m_pMotionDlg983)
		{
			int count = (int)m_pMotionDlg983->m_allMotions.size();
			m_pMotionDlg983->PlayLobby983(&count);
		}
		break;

	case 0x19c:
		if (m_pMotionDlg983)
			m_pMotionDlg983->SelectMotion983((const char*)msg.param1, &msg.param2);
		break;

	case 0x19d: // 983 00abe3f9: param1 != 1 = botao desabilitado
	{
		const bool on = m_pMotionBtn983 && m_pMotionDlg983;
		const bool enabled = msg.param1 == 1;
		const char* normal = on ? "motionchat_btn_on_Normal" : "motionchat_btn_off_Normal";
		const char* hover = on ? "motionchat_btn_on_Hover" : "motionchat_btn_off_Hover";
		m_pMotionBtn983->SetButtonImg(enabled ? normal : "motionchat_btn_Disable", FrButton::NORMAL);
		m_pMotionBtn983->SetButtonImg(enabled ? hover : "motionchat_btn_Disable", FrButton::OVER);
		m_pMotionBtn983->SetButtonImg(enabled ? hover : "motionchat_btn_Disable", FrButton::PRESSED);
	}
	break;

	case 0x19e:
		if (m_pMotionDlg983)
			*(int*)msg.param1 = (int)m_pMotionDlg983->m_allMotions.size();
		break;

	case 0x19f:
		if (m_pMotionDlg983)
			*(const char**)msg.param2 = m_pMotionDlg983->GetMotion983(msg.param1);
		break;

	case M983(0x201):
	{
		if (!IsLocalContent((localContentType_t)0x16 /*983: era 0x15; medido*/))
			return;

		if (m_pPointEventDlg == NULL)
			m_pPointEventDlg = CreateForm<FrPointEventDlg>(
				g_pFresh->GetManager(), this, "point_event_dlg");

		if (m_pPointEventDlg == NULL)
			return;

		m_pPointEventDlg->SetMyPoint(Doc()->m_myInfo.info.dwPointPointEvent);
		m_pPointEventDlg->SetIconRect(m_pToppagePointEvent->GetRect());
		m_pPointEventDlg->Open(
			(FRESH_PFN_RESULT)&CLobbyMain::OnPointEventDlgResult, 0x11);
		m_pPointEventDlg->SetDesc(
			K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc6\xf7\xc0\xce\xc6\xae\xb8\xa6 \xc8\xae\xc0\xce\xc7\xcf\xbd\xc3\xb0\xed \xc0\xcc\xba\xa5\xc6\xae\xbf\xa1 \xc2\xfc\xbf\xa9\xc7\xcf\xbc\xbc\xbf\xe4."));
	}
	break;

	case M983(0x21c):
	{
		if (!IsLocalContent(S4_MATCHING_SYSTEM))
			return;

		std::string name = *(std::string*)msg.param1;
		const char* mode = GetGameTypeText(*(unsigned char*)msg.param2);
		unsigned long uid = *(unsigned long*)msg.param3;

		if (m_pInviteDlg == NULL && Doc()->m_userInfoMod == 0)
		{
			m_pInviteDlg =
				CreateForm<FrInviteDlg>(g_pFresh->GetManager(), this, "invite");
			m_pInviteDlg->SetInvaiteInfo(0, 0, 0, uid, MyUID());
			m_pInviteDlg->SetMessage(
				MakeStr(
					K2L_Compatibility("(%s)\xb4\xd4\xb2\xb2\xbc\xad %s\xb8\xf0\xb5\xe5 \xb8\xc5\xc4\xaa \xbf\xe4\xc3\xbb\xc0\xbb\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.\n\xb8\xc5\xc4\xaa\xbf\xa1 \xc0\xc0\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
					name.c_str(), mode),
				false);
			m_pInviteDlg->SetTimeLimit(5.0f);
			m_pInviteDlg->Open(
				(FRESH_PFN_RESULT)&CLobbyMain::OnMatchingDlgResult, 1);
		}
		else
		{
			WSendPacket packet((enumClientPacket)0xc0);
			packet.Encode4(uid);
			packet.Encode1(0);
			packet.Send(TO_GAME);
		}
	}
	break;

	case M983(0x21d):
	{
		if (!IsLocalContent(S4_MATCHING_SYSTEM))
			return;

		if (m_pQuickResultDlg)
		{
			int result = msg.param1;
			m_pQuickResultDlg->SetContents((const char*)msg.param2);

			if (result == 0)
				m_pQuickResultDlg->SetOkbtnEnable(true);
		}

		Doc()->m_userInfoMod = 0;
	}
	break;

	case M983(0x230):
		if (m_pUserListDlg)
			m_pUserListDlg->CloseInviteDelayDlg();
		break;

	case M983(0x240):
	{
		if (!IsLocalContent((localContentType_t)0x71 /*983: era 0x72; medido*/))
			return;

		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
			(FrCmdTarget*)this, "notify");
		WRect rect = pForm->GetRect();
		pForm->SetRect(WRect(rect.x, rect.y, rect.w, rect.h));
		pForm->SetTimeLimit(12.0f);
		pForm->SetMessage(
			K2L_Compatibility("-\\c0xffff0000\\c\xb0\xe6\xc7\xb0 \xc0\xcc\xba\xa5\xc6\xae\\c0xff000000\\c\xbf\xa1 \xc0\xda\xb5\xbf\xc0\xb8\xb7\xce \xc0\xc0\xb8\xf0\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
			false);
		pForm->Open(NULL, 0);
	}
	break;

	case M983(0x249):
		if (msg.param1 == 1)
		{
			if (m_pCreateNickDlg == NULL)
			{
				m_pCreateNickDlg = CreateForm<FrCreateNickDlg>(
					g_pFresh->GetManager(), this, "createnick");
				if (m_pCreateNickDlg)
					m_pCreateNickDlg->Open(
						(FRESH_PFN_RESULT)&CLobbyMain::OnCreateNickResult, 0);
			}
			else
			{
				m_pCreateNickDlg->SetNick((const char*)msg.param2);
			}
		}
		else if (msg.param1 == 0)
		{
			if (m_pCreateNickDlg)
			{
				strcpy(MyNick(), (const char*)msg.param2);
				m_pCreateNickDlg->CloseDlg(1);
				m_pCreateNickDlg = NULL;
			}
		}
		else if (msg.param1 == 3)
		{
			if (!IsLocalContent(S4_NT_NICKNAME_CHANGE))
				return;

			if (m_pCreateNickDlg)
				return;

			m_pCreateNickDlg = CreateForm<FrCreateNickDlg>(
				g_pFresh->GetManager(), this, "createnick");
			if (m_pCreateNickDlg)
			{
				m_pCreateNickDlg->SetNick(MyNick());
				m_pCreateNickDlg->Open(
					(FRESH_PFN_RESULT)&CLobbyMain::OnCreateNickResult, 0);
			}
		}
		break;

	case M983(0x6d): // 983 00abeab9: nada (Reponse 00b08050 sem chamador)
		break;

	case M983(0x6e):
		if (!IsLocalContent(S4_NT_NICKNAME_CHANGE))
			return;

		if (msg.param1 == 0)
		{
			if (m_pCreateNickDlg)
				m_pCreateNickDlg->ReceiveCheckCode(msg.param2);
		}
		break;

	case M983(0x6f):
		if (!IsLocalContent(S4_NT_EVENT_MISSION))
			return;

		if (msg.param1 == 0)
		{
			m_pMissionEventDlg = CreateForm<FrMissionEventDlg>(
				g_pFresh->GetManager(), (FrCmdTarget*)this, "mission_event");
			if (m_pMissionEventDlg)
			{
				m_pMissionEventDlg->Open(
					(FRESH_PFN_RESULT)&CLobbyMain::OnMissionEventResult, 1);
				m_pMissionEventDlg->EnableDrag(false);
			}
		}
		else if (msg.param1 == 1)
		{
			const char* text = NULL;
			switch (msg.param2)
			{
			case 0:
				text =
					K2L_Compatibility("\xbb\xf3\xc7\xb0\xc0\xcc \xbf\xec\xc6\xed\xc0\xb8\xb7\xce \xc0\xfc\xb4\xde\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xbf\xec\xc6\xed\xc7\xd4\xc0\xbb \xc8\xae\xc0\xce\xc7\xcf\xbc\xbc\xbf\xe4.");
				break;
			case 1:
				text =
					K2L_Compatibility("\xc0\xcc\xb9\xcc \xb9\xde\xc0\xb8\xbc\xcc\xb3\xd7\xbf\xe4.");
				break;
			case 2:
				text =
					K2L_Compatibility("\xbb\xf3\xc7\xb0\xc0\xcc \xc0\xfc\xb4\xde\xc0\xcc \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xc7\xd1\xb9\xf8 \xbd\xc5\xc3\xbb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.");
				break;
			}

			this << MsgObject(NULL, M983(0x23), (int)text, 0, 0, 0, 0);
		}
		break;

	case M983(0x70):
		if (!IsLocalContent(S4_NT_EVENT_BINGO))
			return;

		if (msg.param1 == 0)
		{
			m_pBingoEventDlg = CreateForm<FrBingoEventDlg>(
				g_pFresh->GetManager(), (FrCmdTarget*)this, "bingo_event");
			if (m_pBingoEventDlg)
			{
				m_pBingoEventDlg->Open(
					(FRESH_PFN_RESULT)&CLobbyMain::OnBingoEventResult, 1);
				m_pBingoEventDlg->EnableDrag(false);

				m_pBingoEventDlg->SetBingoData(
					(GlobalEnum::sBingoInfo*)msg.param2);
			}
		}
		else if (msg.param1 == 1)
		{
			if (m_pBingoEventDlg)
			{
				m_pBingoEventDlg->SetBingoData(
					(GlobalEnum::sBingoInfo*)msg.param2);
				m_pBingoEventDlg->SetSelectedData(msg.param3, msg.param4,
					*(unsigned int*)msg.param5);
			}
		}
		else if (msg.param1 == 2)
		{
			if (m_pBingoEventDlg)
			{
				unsigned int* pCount = (unsigned int*)msg.param3;
				m_pBingoEventDlg->SetGiftData(
					(std::vector<unsigned long>*)msg.param2, *pCount);
			}
		}
		break;

	case M983(0x41):
		LogOut(0, "CLobbyMain::HandleMsg %d : %d %d %d %d %d\n", msg.message,
			msg.param1, msg.param2, msg.param3, msg.param4, msg.param5);
		break;

	case M983(0x6b):
		LogOut(0, "CLobbyMain::HandleMsg %d : %d %d %d %d %d\n", msg.message,
			msg.param1, msg.param2, msg.param3, msg.param4, msg.param5);
		break;

	case M983(0xb2):
		LogOut(0, "CLobbyMain::HandleMsg %d : %d %d %d %d %d\n", msg.message,
			msg.param1, msg.param2, msg.param3, msg.param4, msg.param5);
		break;

	case M983(0xb4):
		LogOut(0, "CLobbyMain::HandleMsg %d : %d %d %d %d %d\n", msg.message,
			msg.param1, msg.param2, msg.param3, msg.param4, msg.param5);
		break;

	case M983(0x211):
		LogOut(0, "CLobbyMain::HandleMsg %d : %d %d %d %d %d\n", msg.message,
			msg.param1, msg.param2, msg.param3, msg.param4, msg.param5);
		break;

	case M983(0x21b):
		LogOut(0, "CLobbyMain::HandleMsg %d : %d %d %d %d %d\n", msg.message,
			msg.param1, msg.param2, msg.param3, msg.param4, msg.param5);
		break;

	// JP 983 (00abea7b): 0x355 (taskmanager 009d693f, resposta do pacote 0x184) = FUN_00ab3290 -> 004ff210 da sala (+0x780):
	// religa o botao "assist" desligado no OnAssist e o deixa apertado (param1 == 1) ou solto; sem isto o botao travava
	// 983 00abc140: casos com remetente vivo no nosso fonte que nao tinham receptor (D968, out/D968)
	case 0x76: // 983 (+0x770; jukeboxdlg M983(0x69)): imagem do botao do jukebox
		if (m_pToppageJukeboxBtn983)
			m_pToppageJukeboxBtn983->SetButtonImg(msg.param1 == 1 ? "btn_jukebox_o" : "btn_jukebox_n", (FrButton::eButMode)0);
		break;

	case 0x77: // 983 (RFLobbyDesktop): BGM do lobby ao abrir/fechar as telas; 00ab5a50 = estado 9 da lista de GP pedida
		if (CLobbyState983::ms_pInstance && !(CLobbyState983::ms_pInstance->m_state == 9 && Doc()->m_roomListRequest983 == 1))
		{
			if (msg.param1 == 0)
				CBGMPlayList983::Instance()->Play983();
			else
				CBGMPlayList983::Instance()->PlayDefault983(false);
		}
		break;

	case 0x80: // 983 (taskmanager 009c3d27): botoes de categoria da lista de salas comuns
		if (m_pRoomListForm983 && Doc()->m_roomListKind983 == 0)
			((FrRoomListDlg*)m_pRoomListForm983)->SetCateBtnByServerType983();
		break;

	case 0x8c: // 983: abas da lista de salas
		if (m_pRoomListForm983)
		{
			((FrRoomListDlg*)m_pRoomListForm983)->ShowTabGpRoom983();
			((FrRoomListDlg*)m_pRoomListForm983)->SetTabs983();
		}
		break;

	case 0x352: // 983 (CGrandPrixDocument 005565ee): lista do GP refeita, a janela aberta no canal refaz as linhas
		if (msg.param1 == (int)Doc()->m_gpChannel983 && m_pRoomListForm983)
			((FrRoomListDlg*)m_pRoomListForm983)->Room310_983(msg.param1);
		break;

	case 0x314: // 983 (taskmanager 009c2bb7): sala 004ff2e0
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
		{
			if (pDlg->m_pCheer)
				pDlg->m_pCheer->SetRestoreBgm983(msg.param1 == 1);
		}
		break;

	case 0x31c: // 983 (lobbytask): sala 00503430 = botao da posicao fixa do copo
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
			pDlg->Fn503430_983(msg.param1);
		break;

	case 0x355:
		if (FrGameRoomDlg* pDlg = GameRoomDlg983())
		{
			pDlg->SetAssistBtn983(msg.param1 == 1);
		}
		break;

	// JP 983 (00abe0c7, C-PKT lote 10): mensagem 0x84 (numero do 983; nao existe no KR 645). Pede a lista de salas
	// do tipo guardado em CSharedDoc+0x11634 (0062bf70): 1 -> pacote 0x176, 0 -> pacote 0x81, os dois sem campos;
	// depois marca 2 (0062bf60) e manda a mensagem 1 a si mesma.
	case 0x84:
	{
		switch (Doc()->m_roomListRequest983)
		{
		case 1:
		{
			WSendPacket packet((enumClientPacket)0x176);
			packet.Send(TO_GAME);
		}
		break;
		case 0:
		{
			WSendPacket packet((enumClientPacket)0x81);
			packet.Send(TO_GAME);
		}
		break;
		}
		Doc()->m_roomListRequest983 = 2;
		this << MsgObject(NULL, 1, 0, 0, 0, 0, 0);
	}
	break;

	default:
	{
		// 983 (00abc140 switchD_caseD_1): a widget da tarefa (My Room/Loja) trata primeiro; falso -> HandleMsgCommon
		FrTaskForm* pWidgetForm = AfxGetTask()->GetWidgetForm983();
		if (pWidgetForm == NULL || !pWidgetForm->OnMsg983(msg))
			HandleMsgCommon(msg);
	}
	}
}
void CLobbyMain::OpenPCBangNoticeForm()
{
	if (m_pPCBangNoticeDlg == NULL && CLoginInfo::Instance()->IsPcBang())
	{
		OnToppage_NoticeUp();
	}
	else if (Doc()->m_bShowNotice)
	{
		OnToppage_NoticeUp();
	}
}

bool CLobbyMain::NeedsChangeNick()
{
	if (CLoginInfo::Instance()->IsWebLogin() &&
		Doc()->m_myInfo.info.nChannelingFlag == 1 &&
		!strcmp(Doc()->m_myInfo.info.sNick, Doc()->m_myInfo.info.sDisplayID))
		return true;

	return false;
}

void CLobbyMain::OpenParanNickChange()
{
	if (NeedsChangeNick() == true)
	{
		m_pParanNickDlg = CreateForm<FrParanNickDlg>(g_pFresh->GetManager(),
			this, "changenick_paran");
		m_pParanNickDlg->SetMessage(
			K2L_Compatibility("\xc6\xce\xbe\xdf\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xb4\xeb\xc8\xad\xb8\xed\xc0\xbb \xc0\xd4\xb7\xc2\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."),
			false);
		m_pParanNickDlg->Open(
			(FRESH_PFN_RESULT)&CLobbyMain::OnParanNickChangeResult, 0);
	}
	else
	{
		OpenCaddieWarningForm();
	}
}

void CLobbyMain::OpenCaddieWarningForm()
{
	if (m_bCaddieWarning && Doc()->IsOpenFirstLoginAlarm())
	{
		m_bCaddieWarning = false;

		for (int i = 0; i < 12; i++)
		{
			if (m_caddieWarning[i].guid != 0)
			{
				sCaddieInfo info = m_caddieWarning[i];
				memset(&m_caddieWarning[i], 0, sizeof(sCaddieInfo));

				if (info.guid)
				{
					m_pCaddieWarningDlg = CreateForm<FrCaddieWarningDlg>(
						g_pFresh->GetManager(), this, "caddie_warning");
					m_pCaddieWarningDlg->SetCaddieInfo(info);
					m_pCaddieWarningDlg->Open(
						(FRESH_PFN_RESULT)&CLobbyMain::OnCaddieWarningDlgResult,
						0x40);
				}
				return;
			}
		}
	}
	else
	{
		OpenNoteForm();
	}
}

void CLobbyMain::OpenNoteForm()
{
	if (Doc()->IsOpenFirstLoginAlarm() && Doc()->m_noteList.size() > 0)
	{
		if (m_pMailBoxDlg == NULL)
		{
			m_pMailBoxDlg = CreateForm<FrMailBoxDlg>(g_pFresh->GetManager(),
				this, "mailbox");
			m_pMailBoxDlg->EnableDrag(false);
			m_pMailBoxDlg->Open(
				(FRESH_PFN_RESULT)&CLobbyMain::OnMailBoxDlgResult, 0);
		}
	}
}

void CLobbyMain::OpenEventPrizeForm(unsigned char id)
{
	if (m_pEventPrizeDlg[id] == NULL)
	{
		m_pEventPrizeDlg[id] = CreateForm<FrEventPrizeDlg>(
			g_pFresh->GetManager(), this, "notice_popup");
		m_pEventPrizeDlg[id]->SetEvent(id);
		m_pEventPrizeDlg[id]->Open(
			(FRESH_PFN_RESULT)&CLobbyMain::OnEventDlgResult, 3);
	}
}


void CLobbyMain::OnToppage_Init(int param)
{
	CTaskMain::OnInit();

	Doc()->ClearUserInfoTimeMap(0xffffffff);
	memset(Doc()->m_userInfo, 0, sizeof(sUserInfo) * 4);

	TurnOn3DBackground();

	if (IsLocalContent(S4_TOPPAGE_BTN_LINEUP))
	{
		m_topBtnIndex = CLoginInfo::Instance()->IsPcBang() ? 2 : 1;
	}
}

void CLobbyMain::OnToppage_Finish()
{
	if (Doc()->m_bAutoRefresh)
	{
		if (IsLocalContent(S3_PARAN_CHANNELING))
		{
			Doc()->m_bAutoRefresh = false;
			OpenEventPrizeForm(5);

			if (Doc()->m_bTutorialRedirect == true)
			{
				this << MsgObject(NULL, M983(0x3e), 0x1a000006,
					(int)"[\xc7\xc7\xc7\xc9 \xbc\xb1\xb9\xb0 \xb8\xde\xc0\xcf]\n\n\xbe\xc8\xb3\xe7~ \xc0\xfc \xb4\xe7\xbd\xc5\xc0\xbb \xbf\xa9\xb1\xe2 \xb5\xa5\xb8\xae\xb0\xed \xbf\xc2 \xbd\xc3\xb0\xa3\xc0\xc7 \xbf\xe4\xc1\xa4 \xc7\xc7\xc7\xc9\xc0\xcc\xb6\xf3\xb0\xed \xc7\xd5\xb4\xcf\xb4\xd9. \xb9\xe6\xb1\xdd \xbf\xb9\xbb\xdb \xc4\xb3\xb8\xaf\xc5\xcd\xb8\xa6 \xbc\xb1\xc5\xc3\xc7\xdf\xb3\xd7\xbf\xe4. \xb0\xe8\xbc\xd3 \xc0\xdf \xba\xb8\xbb\xec\xc6\xec\xc1\xd6\xbc\xbc\xbf\xe4.\n\n\xbe\xc6~ \xb1\xd7\xb8\xae\xb0\xed \xc0\xcc\xb0\xc5 \xb9\xde\xc0\xb8\xbc\xbc\xbf\xe4. \xb4\xe7\xbd\xc5\xc0\xcc \xc6\xce\xbe\xdf\xbf\xa1 \xba\xb8\xb4\xd9 \xbd\xb1\xb0\xd4 \xc0\xfb\xc0\xc0\xc7\xcf\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad \xb5\xe5\xb8\xae\xb4\xc2 \xc7\xe0\xbf\xee\xc0\xc7 \xb8\xf1\xb0\xc9\xc0\xcc\xc0\xd4\xb4\xcf\xb4\xd9.\n\xc0\xcc \xc7\xe0\xbf\xee\xc0\xc7 \xb8\xf1\xb0\xc9\xc0\xcc\xb4\xc2 \xb4\xe7\xbd\xc5\xc0\xcc \xb8\xb8\xb5\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xc0\xc7 \\c0xffff0000\\c\xc1\xa4\xc8\xae\xb5\xb5\\c0xff000000\\c\xb8\xa6 \xb3\xf4\xc7\xf4\xc1\xd6\xb0\xed \\c0xffff0000\\c\xc6\xce\xbe\xdf\xc0\xd3\xc6\xd1\xc6\xae\xc1\xb8\\c0xff000000\\c\xc0\xbb \xb3\xd0\xc7\xf4\xc1\xd6\xb4\xc2 \xbf\xaa\xc7\xd2\xc0\xbb \xc7\xd1\xb4\xe4\xb4\xcf\xb4\xd9.\n\xc7\xcf\xb3\xaa \xc1\xd6\xc0\xc7\xc7\xd2 \xb0\xcd\xc0\xba\xbf\xe4~ \xb4\xe7\xbd\xc5\xc0\xc7 \xbd\xc7\xb7\xc2\xc0\xcc \xbe\xee\xb4\xc0 \xc1\xa4\xb5\xb5 \xbe\xc8\xc1\xa4\xb5\xc7\xb4\xc2 \xb7\xb9\xba\xa7, \xc1\xef \\c0xffff0000\\c\xc1\xd6\xb4\xcf\xbe\xee"
						 "E\\c0xff000000\\c\xb0\xa1 \xb5\xc7\xb8\xe9 \xc8\xbf\xb0\xfa\xb0\xa1 \xbe\xf8\xbe\xee\xc1\xf6\xb4\xcf\xb1\xee\xbf\xe4~ \xbe\xcb\xbe\xc6\xb5\xce\xbd\xc3\xb0\xed\xbf\xe4.\n\n\xb1\xd7\xb8\xae\xb0\xed \xb4\xeb\xc0\xfc\xb8\xf0\xb5\xe5\xbf\xa1 \xb0\xa1\xb1\xe2 \xc0\xfc\xbf\xa1 \xc7\xd1\xb9\xf8 \xc8\xa5\xc0\xda \xbf\xac\xbd\xc0\xc7\xcf\xb0\xed \xbd\xcd\xb4\xd9\xb8\xe9 \xc6\xa9\xc5\xe4\xb8\xae\xbe\xf3 \xb8\xf0\xb5\xe5\xbf\xa1 \xb5\xe9\xbe\xee\xb0\xa1\xba\xb8\xbc\xbc\xbf\xe4.\n\xc2\xf7\xb1\xd9\xc2\xf7\xb1\xd9 \xb8\xf0\xb5\xce \xb9\xe8\xbf\xec\xbd\xc3\xb8\xe9 \xbc\xb1\xb9\xb0\xb5\xb5 \xc1\xd8\xb4\xd9\xb0\xed \xc7\xcf\xb3\xd7\xbf\xe4. ^^\n\n\xb1\xd7\xb7\xb3 \xb4\xe3\xbf\xa1 \xb6\xc7 \xba\xc1\xbf\xe4. \xbe\xc8\xb3\xe7",
					0, 0, 0);
				Doc()->m_bTutorialRedirect = false;
			}
		}
		else
		{
			Doc()->m_bAutoRefresh = false;
			OpenEventPrizeForm(2);
		}
	}

	if (Doc()->m_bRefreshCamera)
		this << MsgObject(NULL, M983(0xaa), 1, 0, 0, 0, 0);
}

void CLobbyMain::OnToppage_Destroy()
{
}

void CLobbyMain::OnToppage_GameInit(int param)
{
	m_pToppageGame = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppageGame)
	{
		m_pToppageGame->SetPushSound("ui_desktop_icon_click");
		m_pToppageGame->EnableHover(true, 0.5f);
	}
}

void CLobbyMain::OnToppage_GameUp()
{
	OnUnderBar_GameUp();
}

void CLobbyMain::OnToppage_FamilyInit(int param)
{
	m_pToppageFamily = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppageFamily)
	{
		m_pToppageFamily->SetPushSound("ui_desktop_icon_click");
		m_pToppageFamily->EnableHover(true, 0.5f);
	}
}

// 983: o LBUTTONUP do "TOPPAGE.family" e o 00595f60 (tabela da message map em 00e0d200/00e0d210: "TOPPAGE.family", 4)
// = abre a janela do Practice (single_mode_start), como o btn_practice do RFLobbyDesktop. O 645 rodava aqui o jogo local
// "Family" (CFamilyTask), que o 983 nao tem.
void CLobbyMain::OnToppage_FamilyUp()
{
	OpenSingleModeStarter983();
}

// 983 (00596030): acima do nivel 5 (CSharedDoc+0x91d) o TUTORIAL some (o 00593760 e o Init do "family")
void CLobbyMain::OnToppage_PangysStudyInit(int param)
{
	m_pToppagePangyaStudy = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppagePangyaStudy)
	{
		if (Doc()->m_myInfo.stat.Level > 5)
		{
			m_pToppagePangyaStudy->SetVisible(false);
			m_pToppagePangyaStudy->Enable(false);
			return;
		}
		m_pToppagePangyaStudy->SetPushSound("ui_desktop_icon_click");
		m_pToppagePangyaStudy->EnableHover(true, 0.5f);
	}
}

// 983 (005937f0): o tutorial e o widget "CTutorialWidget" do lobby (o 983 nao tem CTutorialTask)
// 983 (00596290): o botao "P" (top_premium_btn) nasce escondido; com premium (bit 14 do dwIdentity) avisa o Lobby
void CLobbyMain::OnToppage_PremiumServiceInit983(int param)
{
	m_pToppagePremiumService983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!m_pToppagePremiumService983)
		return;
	m_pToppagePremiumService983->SetVisible(false);
	if ((Doc()->m_myInfo.info.dwIdentity >> 14) & 1)
		GetActor("Lobby") << MsgObject(NULL, M983(0xcb), 0, 0, 0, 0, 0);
}

// 983 (00596180): aviso de fim do premium, visivel so com o byte CSharedDoc+0x1160a (taskmanager); o desenho e o 00597160 (OWNERDRAW, D644)
void CLobbyMain::OnToppage_PremiumEndNoticeInit983(int param)
{
	m_pToppagePremiumEndNotice983 = (FrWnd*)param; // 983: DYNAMIC_CAST pela RTTI 00f1ab44
	if (!m_pToppagePremiumEndNotice983)
		return;
	*(float*)&m_premiumEndNoticeAlpha983 = 1.0f;
	*(float*)&m_premiumEndNoticeFadeSpeed983 = 0.2f;
	bool on = Doc()->m_bPremiumEndNotice983 != 0;
	m_pToppagePremiumEndNotice983->SetVisible(on);
	m_pToppagePremiumEndNotice983->Enable(on);
	m_pToppagePremiumEndNotice983->SetTopmost(true);
	m_premiumEndNoticeBmp983[0] = (unsigned long)g_pFresh->GetBitmap("Premium service_close_00");
	m_premiumEndNoticeBmp983[1] = (unsigned long)g_pFresh->GetBitmap("Premium service_close_01");
	m_premiumEndNoticeBmp983[2] = (unsigned long)g_pFresh->GetBitmap("Premium service_close_02");
	m_premiumEndNoticeBmp983[3] = (unsigned long)g_pFresh->GetBitmap("Premium service_base");
}

// 983 (00597160): faixa base + 3 bitmaps "Premium service_close_0x" centrados na tela com o alpha +0x27c; texto do doc
// (+0x1160c) no centro do aviso a 85% da altura, estilo 2 em branco e estilo 0 em preto; alpha <= 0 desliga o byte e o aviso.
void CLobbyMain::OnToppage_PremiumEndNoticeOwnerDraw983(int)
{
	if (!m_pToppagePremiumEndNotice983 || !Doc()->m_bPremiumEndNotice983)
		return;
	const float alpha = *(float*)&m_premiumEndNoticeAlpha983;
	const Bitmap* b0 = (const Bitmap*)m_premiumEndNoticeBmp983[0];
	const Bitmap* b1 = (const Bitmap*)m_premiumEndNoticeBmp983[1];
	const Bitmap* b2 = (const Bitmap*)m_premiumEndNoticeBmp983[2];
	const Bitmap* base = (const Bitmap*)m_premiumEndNoticeBmp983[3];
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (b0 && b1 && b2 && base)
	{
		unsigned long color = ((int)(alpha * 255.0) << 24) | 0xFFFFFF;
		float w = g_view->GetWidth();
		float h = g_view->GetHeight();
		float w0 = b0->Width(), w1 = b1->Width(), w2 = b2->Width();
		float h0 = b0->Height();
		float x = w * 0.5 - (w2 + w1 + w0) * 0.5;
		float y = h * 0.5 - h0 * 0.5;
		pGDI->DrawTexture(base, WRect(0.0f, y - 20.0, w, h0 + 40.0), color, 0);
		pGDI->DrawTexture(b0, WRect(x, y, w0, h0), color, 0);
		pGDI->DrawTexture(b1, WRect(w0 + x, y, w1, b1->Height()), color, 0);
		pGDI->DrawTexture(b2, WRect(w1 + w0 + x, y, w2, b2->Height()), color, 0);
	}
	const char* text = Doc()->m_premiumEndNoticeText983.c_str();
	const WRect& rc = m_pToppagePremiumEndNotice983->GetRect();
	WPoint pt(rc.w * 0.5 + rc.x, rc.h * 0.85f + rc.y);
	pGDI->SetTextStyle(2);
	pGDI->SetTextColor(((int)(alpha * 255.0) << 24) | 0xFFFFFF, 0xFFFFFFFF);
	pGDI->Print(pt, 1, text);
	pGDI->SetTextStyle(0);
	pGDI->SetTextColor((int)(alpha * 255.0) << 24, 0xFFFFFFFF);
	pGDI->Print(pt, 1, text);
	if (alpha <= 0.0f)
	{
		Doc()->m_bPremiumEndNotice983 = 0;
		m_pToppagePremiumEndNotice983->SetVisible(false);
		m_pToppagePremiumEndNotice983->Enable(false);
	}
}

void CLobbyMain::OnToppage_PangysStudyUp()
{
	CloseAllDialogs();

	HandleMsg(MsgObject(NULL, 0x88 /*so do 983*/, (int)"CTutorialWidget",
		(int)"TOPPAGE", 0, 0, 0));
}

// 983 005951b0: botao preso a direita da tela (WView+0x19c - 97, y 158)
void CLobbyMain::OnToppage_ChangeBgInit983(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pBtn != NULL)
		pBtn->MoveWindow(WPoint(g_view->GetWidth() - 97.0f, 158.0f));
}

// 983 00595230: troca o fundo do lobby (LobbyAvatarActor 0x1a2)
void CLobbyMain::OnToppage_ChangeBgUp983()
{
	AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x1a2, 0, 0, 0, 0, 0);
}

void CLobbyMain::OnToppage_NoticeInit(int param)
{
	m_pToppageNotice = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppageNotice)
	{
		m_pToppageNotice->SetPushSound("ui_desktop_icon_click");
		m_pToppageNotice->EnableHover(true, 0.5f);
	}
}

// 983 (00593900): "TOPPAGE.oneday_quest" (missoes diarias)
void CLobbyMain::OnToppage_OneDayQuestInit983(int param)
{
	m_pToppageOneDayQuest = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppageOneDayQuest)
	{
		m_pToppageOneDayQuest->SetPushSound("ui_desktop_icon_click");
		m_pToppageOneDayQuest->EnableHover(true, 0.5f);
	}
}

// 983 (00593990): pede as missoes (pacote 0x151, o mesmo do quest_accept do console) e manda a msg 1 a si mesmo
void CLobbyMain::OnToppage_OneDayQuestDown983()
{
	{
		WSendPacket packet((enumClientPacket)0x151);
		packet.Send(TO_GAME);
	}
	this << MsgObject(this, M983(0x1), 0, 0, 0, 0, 0);
}

// 983 (005949d0): "TOPPAGE.rankup" (sem checar o ponteiro antes do Show, como no 983)
void CLobbyMain::OnToppage_RankUpInit983(int param)
{
	m_pToppageRankUp983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pToppageRankUp983->SetVisible(true);
	m_pToppageRankUp983->SetPushSound("ui_desktop_icon_click");
	m_pToppageRankUp983->EnableHover(true, 0.5f);
	if (m_pToppageRankUp983)
	{
		m_pToppageRankUp983->SetButtonImg("main_icon_ppn_n", FrButton::NORMAL);
		m_pToppageRankUp983->SetButtonImg("main_icon_ppn_o", FrButton::OVER);
	}
}

// 983 (00594a90): pagina do tutorial de novatos (CWebPageConnector, tipo 10)
void CLobbyMain::OnToppage_RankUpDown983()
{
	CWebPageConnector* pConnector = CWebPageConnector::Instance();
	if (pConnector)
	{
		CloseAllDialogs();
		if (COption::Instance())
			COption::Instance()->SetScreenSize983(800, 600);
		pConnector->RequestOpen983(
			"https://www5.pangya.jp/on_client/201012_NoviceTutorial/login.aspx?ID=%s&KID=%s", true, 10);
		if (g_input)
			g_input->SetMousePoint(400.0f, 325.0f);
		OnBarHoverOff(0);
	}
}

// 983 (00594b00): "TOPPAGE.guildrank" (sem checar o ponteiro, como no 983)
void CLobbyMain::OnToppage_GuildRankInit983(int param)
{
	m_pToppageGuildRank983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pToppageGuildRank983->SetPushSound("ui_desktop_icon_click");
	m_pToppageGuildRank983->EnableHover(true, 0.5f);
	m_pToppageGuildRank983->SetPushDelay(0.0f);
	m_pToppageGuildRank983->UseDblClick(false);
}

// 983 (00ab45c0/00ab4620): os icones de emocao e de musica ficam presos a direita da tela. O recurso traz o
// retangulo na base de 800 de largura; x = largura da vista (WView+0x19c) - (800 - x do recurso), y do recurso.

static void AnchorRight983(FrWnd* pWnd, const WRect& rect)
{
	float width = g_view ? g_view->GetWidth() : 800.0f;
	pWnd->MoveWindow(WPoint(width - (float)(800.0 - rect.x), rect.y));
}

void CLobbyMain::AnchorMotionBtn983()
{
	C_ASSERT_VC10(offsetof(CLobbyMain, m_pMotionBtn983) == 0x4b0);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_pMotionDlg983) == 0x4b4);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_roomPassword) == 0x4c8);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_bCreateRoom) == 0x5cc);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_bCanStart) == 0x5d0);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_quickStartCate) == 0x5d1);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_motionBtnRect983) == 0x750);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_jukeboxBtnRect983) == 0x760);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_pToppageJukeboxBtn983) == 0x770);
	C_ASSERT_VC10(offsetof(CLobbyMain, m_pJukeBoxDlg983) == 0x774);
	if (m_pMotionBtn983)
		AnchorRight983(m_pMotionBtn983, m_motionBtnRect983);
}

void CLobbyMain::AnchorJukeboxBtn983()
{
	if (m_pToppageJukeboxBtn983)
		AnchorRight983(m_pToppageJukeboxBtn983, m_jukeboxBtnRect983);
}

// 983 (00ab7780, 3a chamada do OnProcess 00ab8f60): reancora os icones da direita quando a largura da tela muda
void CLobbyMain::ReanchorRightIcons983()
{
	static float s_width = COption::Instance()->GetBarX983();
	if (COption::Instance()->GetBarX983() != s_width)
	{
		AnchorMotionBtn983();
		AnchorJukeboxBtn983();
		s_width = COption::Instance()->GetBarX983();
	}
}

// 983 (00ab7430): "TOPPAGE.motion_btn_start"
void CLobbyMain::OnToppage_MotionBtnInit983(int param)
{
	m_pMotionBtn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!m_pMotionBtn983)
		return;
	m_pMotionBtn983->UseDblClick(false);
	m_pMotionBtn983->SetPushDelay(0.0f);
	m_pMotionBtn983->EnableHover(true, 0.0f);
	m_motionBtnRect983 = m_pMotionBtn983->GetRect();
	AnchorMotionBtn983();
	SetMotionBtnOnImg983();
}

// D642 (983 00ab7510 / 00ab7590): dica "tooltip_motion_ui" (0x1a) sobre o botao de emocoes
void CLobbyMain::OnToppage_MotionBtnHoverOn983()
{
	TOOLTIP()->SetToolTip(TOOLTIP_MOTION_UI);
}

void CLobbyMain::OnToppage_MotionBtnHoverOff983()
{
	TOOLTIP()->ClearToolTip();
}

// D643 (983 00596d60 / 00596e60 / 00596f60): dicas do "family" (0x18 tooltip_single_play), "pangyastudy" (0x19 tooltip_tutorial)
// e "notice" (0x12 tooltip_notice); os tres HOVEROFF (00596de0/00596ee0/00596fe0) so limpam a dica
void CLobbyMain::OnToppage_FamilyHoverOn983()
{
	TOOLTIP()->SetToolTip(TOOLTIP_SINGLE_PLAY);
}

void CLobbyMain::OnToppage_PangyaStudyHoverOn983()
{
	TOOLTIP()->SetToolTip(TOOLTIP_TUTORIAL);
}

void CLobbyMain::OnToppage_NoticeHoverOn983()
{
	TOOLTIP()->SetToolTip(TOOLTIP_NOTICE);
}

void CLobbyMain::OnToppage_HoverOff983()
{
	TOOLTIP()->ClearToolTip();
}

// D643 (983 005978b0): dica do botao "P" = tempo que falta do item premium 0x1a100002 (fim em +0x20 do item, horas do 006333c0)
void CLobbyMain::OnToppage_PremiumServiceHoverOn983()
{
	if (!m_pToppagePremiumService983)
		return;
	const std::list<sItemInfo>& items = Doc()->GetConstMyItemList();
	std::list<sItemInfo>::const_iterator it;
	for (it = items.begin(); it != items.end(); ++it)
		if (it->typeId == 0x1a100002)
			break;
	if (it == items.end())
		return;
	SYSTEMTIME end = { 0 };
	S5::SecToSysTime983(&end, *(__int64*)((char*)&*it + 0x20)); // 983: +0x28/+0x2c do no da lista
	__int64 hours = DOCJP()->GetRemainHours983(&end);
	WORD d = 0, h = 0;
	if (hours != 0)
	{
		d = (WORD)(hours / 24);
		h = (WORD)(hours - d * 24);
	}
	std::string tip;
	if (d == 0 && h == 0)
		tip = K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: 1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n"); // d005b0
	else
		tip = MakeStr(K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), d, h); // d0ba30
	m_pToppagePremiumService983->SetToolTipText(tip);
}

// D643 (983 00597a40): HOVEROFF esvazia a dica do botao
void CLobbyMain::OnToppage_PremiumServiceHoverOff983()
{
	if (m_pToppagePremiumService983)
		m_pToppagePremiumService983->SetToolTipText(std::string(""));
}

// 983 (00ab3620)
void CLobbyMain::SetMotionBtnOnImg983()
{
	m_pMotionBtn983->SetButtonImg("motionchat_btn_on_Normal", FrButton::NORMAL);
	m_pMotionBtn983->SetButtonImg("motionchat_btn_on_Hover", FrButton::OVER);
	m_pMotionBtn983->SetButtonImg("motionchat_btn_on_Hover", FrButton::PRESSED);
}

// 983 (00ab8600): com a janela aberta o clique so a fecha deslizando (00ab4500)
void CLobbyMain::OnToppage_MotionBtnUp983()
{
	m_motionBtnClicked983 = true;
	if (m_pMotionDlg983)
		m_pMotionDlg983->m_slide = 4;
	else
		OpenMotionDlg983();
}

// 983 (00ab8430): lista das emocoes a esquerda do botao; so com o boneco do lobby carregado (msg 0x198) e personagem valido
void CLobbyMain::OpenMotionDlg983()
{
	MsgObject msg(NULL, 0x198, 0, 0, 0, 0, 0);
	AfxGetTask()->GetActor("LobbyAvatar") << msg;
	if (!msg.param1)
		return;
	unsigned long charType = RFLobbyDesktop::Instance983() ? RFLobbyDesktop::Instance983()->GetCharType983() : 0;
	if (charType == (unsigned long)-1)
		return;
	SetMotionBtnOnImg983();
	const WRect& rc = m_pMotionBtn983->GetRect();
	WPoint pos(rc.x - 32.0f, rc.y + 35.0f);
	m_pMotionDlg983 = CreateForm<FrAvatarMotionDlg>(g_pFresh->GetManager(), this, "motiondlg", NULL);
	m_pMotionDlg983->EnableDrag(false);
	m_pMotionDlg983->SetFixed(true);
	m_pMotionDlg983->m_flags = FrAvatarMotionDlg::FLAG_LOBBY;
	m_pMotionDlg983->SetupMotions983(&charType);
	m_pMotionDlg983->Open((FRESH_PFN_RESULT)&CLobbyMain::OnMotionDlgResult983, pos, 5);
	m_pMotionBtn983->EnableHover(false, 0.0f);
	m_pMotionBtn983->SetButtonImg("motionchat_btn_on_Normal", FrButton::NORMAL);
	CIconToolTip::GetInstance()->ClearToolTip();
}

// 983 (00ab7610)
bool CLobbyMain::OnMotionDlgResult983(int result, FrForm* pForm)
{
	if (m_pMotionBtn983)
	{
		m_pMotionBtn983->SetStatus(FrButton::NORMAL);
		m_pMotionBtn983->EnableHover(true, 0.0f);
	}
	m_motionBtnClicked983 = true;
	m_pMotionDlg983 = NULL;
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x19d, 1, 0, 0, 0, 0);
	return true;
}

// 983 (00ab76b0): "TOPPAGE.portrait_jukeboxbtn"
void CLobbyMain::OnToppage_JukeboxBtnInit983(int param)
{
	m_pToppageJukeboxBtn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!m_pToppageJukeboxBtn983)
		return;
	m_jukeboxBtnRect983 = m_pToppageJukeboxBtn983->GetRect();
	AnchorJukeboxBtn983();
	m_pToppageJukeboxBtn983->SetButtonImg(
		FrJukeBoxDlg::m_preserve_isPlaying ? "btn_jukebox_o" : "btn_jukebox_n", FrButton::NORMAL);
	m_pToppageJukeboxBtn983->SetButtonImg("btn_jukebox_o_o", FrButton::OVER);
}

// 983 (00ab7820): abre o jukebox (o botao some enquanto a janela esta aberta) e fecha a das emocoes
void CLobbyMain::OnToppage_JukeboxBtnUp983()
{
	if (m_pJukeBoxDlg983)
	{
		m_pJukeBoxDlg983->Close(true);
		return;
	}
	if (!m_pToppageJukeboxBtn983)
		return;
	m_pJukeBoxDlg983 = CreateForm<FrJukeBoxDlg>(g_pFresh->GetManager(), this, "jukeboxdlg", NULL);
	FrJukeBoxDlg::SelectBGMByName(g_audio->GetBGMName());
	m_pJukeBoxDlg983->Open((FRESH_PFN_RESULT)&CLobbyMain::OnJukeBoxDlgResult983, 1);
	m_pJukeBoxDlg983->SetFixed(true);
	m_pToppageJukeboxBtn983->SetVisible(false);
	if (m_pMotionDlg983)
		m_pMotionDlg983->Close(true);
}

// 983 (00ab36a0)
bool CLobbyMain::OnJukeBoxDlgResult983(int result, FrForm* pForm)
{
	if (m_pToppageJukeboxBtn983)
	{
		m_pToppageJukeboxBtn983->SetStatus(FrButton::NORMAL);
		m_pToppageJukeboxBtn983->SetVisible(true);
	}
	m_pJukeBoxDlg983 = NULL;
	return true;
}

// 983 (00594bb0): pagina do clube (tipo 10)
void CLobbyMain::OnToppage_GuildRankDown983()
{
	CWebPageConnector* pConnector = CWebPageConnector::Instance();
	if (pConnector)
	{
		CloseAllDialogs();
		if (COption::Instance())
			COption::Instance()->SetScreenSize983(800, 600);
		pConnector->RequestOpen983(
			"https://www5.pangya.jp/on_client/club/login.aspx?ID=%s&KID=%s", true, 10);
		if (g_input)
			g_input->SetMousePoint(400.0f, 325.0f);
		OnBarHoverOff(0);
	}
}

void CLobbyMain::OnToppage_NoticeUp()
{
	CTaskMain::OnUnderBar_NoticeUp(); // 983 (005938d0): chamada direta (virtual desde o 983)

	if (m_pToppageNotice && m_pNoticeDlg)
		m_pNoticeDlg->SetIconRect(m_pToppageNotice->GetRect());
}

void CLobbyMain::OnToppage_PcbangInit(int param)
{
	// 983 (00593a30): o JP nao tem PC bang: so esconde o botao (o 00593900 citado antes era o "oneday_quest", s112)
	m_pToppagePcbang = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppagePcbang)
		m_pToppagePcbang->SetVisible(false);
}

void CLobbyMain::OnToppage_PcbangUp()
{
	CloseAllDialogs();
	m_pPCBangNoticeDlg =
		CreateForm<FrNoticeDlg>(g_pFresh->GetManager(), this, "notice_popup");
	m_pPCBangNoticeDlg->LoadInitFile("pcbang.ini");
	m_pPCBangNoticeDlg->SetIconRect(m_pToppagePcbang->GetRect());
	m_pPCBangNoticeDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnPCBangDlgResult,
		0x11);
}

void CLobbyMain::OnToppage_RentalExpiredAlramInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		if (!S5::RENTALPARTS::IsExistExpiredRentalItem(Doc()->m_partsList))
		{
			pButton->Enable(false);
			pButton->SetVisible(false);
		}
	}
}

void CLobbyMain::OnToppage_RentalExpiredAlramUp()
{
	std::list<sItemInfo> rentalItems;
	S5::RENTALPARTS::CollectRentalItem(rentalItems, Doc()->m_partsList);

	int count = 0;
	for (std::list<sItemInfo>::iterator it = rentalItems.begin();
		it != rentalItems.end(); ++it)
	{
		if (it->Expired)
			count++;
	}

	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");

	pForm->SetMessage(
		MakeStr(
			K2L_Compatibility("\xb1\xe2\xb0\xa3\xc0\xcc \xb8\xb8\xb7\xe1\xb5\xc8 \xc0\xc7\xbb\xf3\xc0\xcc <%d>\xb0\xb3 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n\xb8\xb6\xc0\xcc\xb7\xeb\xbf\xa1\xbc\xad \xc8\xae\xc0\xce\xc7\xd2 \xbc\xf6 \xc0\xd6\xc0\xb8\xb8\xe7 \xb1\xe2\xb0\xa3\xc0\xbb \xbf\xac\xc0\xe5\xc7\xcf\xb0\xc5\xb3\xaa \xbb\xe8\xc1\xa6\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
			count),
		false);

	FrButton* pCancel =
		DYNAMIC_CAST(FrButton, pForm->FindChildByName("cancel"));
	if (pCancel)
		pCancel->SetVisible(false);

	FrButton* pOk = DYNAMIC_CAST(FrButton, pForm->FindChildByName("ok"));

	pForm->Open(NULL, 3); // 983 00597fe0
}

void CLobbyMain::OnToppage_Event1Init(int param)
{
	m_pToppageEvent1 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppageEvent1)
	{
		m_pToppageEvent1->SetPushSound("ui_desktop_icon_click");
		m_pToppageEvent1->EnableHover(true, 0.5f);
		m_pToppageEvent1->SetVisible(false);

		if (IsLocalContent((localContentType_t)0x19 /*983: era 0x18; interpolado*/))
		{
			m_pToppageEvent1->SetVisible(true);
		}

		m_event1Rect = m_pToppageEvent1->GetRect();
	}
}

void CLobbyMain::OnToppage_Event1Up()
{
	if (IsLocalContent((localContentType_t)0x19 /*983: era 0x18; interpolado*/))
	{
		CloseAllDialogs();

		if (IsLocalContent((localContentType_t)0x49 /*983: era 0x48; interpolado*/))
			m_pHolesEventDlg = CreateForm<ntHolesEventDlg>(
				g_pFresh->GetManager(), this, "holes_EntranceExam");
		else
			m_pHolesEventDlg = CreateForm<ntHolesEventDlg>(
				g_pFresh->GetManager(), this, "holes_event");

		m_pHolesEventDlg = CreateForm<ntHolesEventDlg>(g_pFresh->GetManager(),
			this, "holes_event");
		m_pHolesEventDlg->Open(
			(FRESH_PFN_RESULT)&CLobbyMain::OnHolesEventDlgResult, 0x11);
		m_pHolesEventDlg->SetIconRect(m_pToppageEvent1->GetRect());
	}
}

void CLobbyMain::OnToppage_NewInit(int param)
{
	m_pToppageNew = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CLobbyMain::OnToppage_NewOwnerDraw(int param)
{
	if (m_pToppagePangyaStudy == NULL)
		return;

	if (m_pNewBmp == NULL)
		return;

	WRect rect;
	rect = m_pToppagePangyaStudy->GetRect();

	g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pNewBmp,
		WRect(rect.x + 58.0f, rect.y + 12.0f, (float)m_pNewBmp->Width(),
			(float)m_pNewBmp->Height()),
		0xffffffff, 0);
}

void CLobbyMain::OnFirstConnectOwnerDraw(int param)
{
	if (m_pToppagePointEvent == NULL)
		return;

	if (!Doc()->m_bToppageAd)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI == NULL)
		return;

	for (int i = 0; i < 3; i++)
	{
		if (m_pLoginBmp[i] == NULL)
			return;
	}

	WRect rect = m_pToppagePointEvent->GetRect();
	rect.y += rect.h - 10.0f;

	if (m_pLoginBmp[0])
	{
		pGDI->DrawTexture(m_pLoginBmp[0],
			WRect(rect.x, rect.y, rect.h - 15.0f,
				(float)m_pLoginBmp[0]->Height()),
			0xffffffff, 0);
		rect.y += (float)m_pLoginBmp[0]->Height();
	}

	if (m_pLoginBmp[1])
	{
		pGDI->DrawTexture(m_pLoginBmp[1],
			WRect(rect.x, rect.y, rect.h - 15.0f,
				(float)m_pLoginBmp[1]->Height() + 18.0f),
			0xffffffff, 0);
		rect.y = rect.y + (float)m_pLoginBmp[1]->Height() + 18.0f;
	}

	if (m_pLoginBmp[2])
	{
		pGDI->DrawTexture(m_pLoginBmp[2],
			WRect(rect.x, rect.y, rect.h - 15.0f,
				(float)m_pLoginBmp[2]->Height()),
			0xffffffff, 0);
		rect.y -= (float)(m_pLoginBmp[0]->Height() + m_pLoginBmp[1]->Height()) +
			18.0f;
	}

	float textX = rect.x + 3.0f;
	pGDI->Print(WPoint(textX, rect.y + 8.0f), 0,
		K2L_Compatibility("\xc3\xe2\xbc\xae\xc6\xf7\xc0\xce\xc6\xae"));
	pGDI->Print(WPoint(textX, rect.y + 19.0f), 0,
		K2L_Compatibility("25 \xc6\xf7\xc0\xce\xc6\xae"));
	pGDI->Print(WPoint(textX, rect.y + 30.0f), 0, K2L_Compatibility("\xc8\xb9\xb5\xe6!!"));
}

void CLobbyMain::OnToppage_PointEventInit(int param)
{
	m_pToppagePointEvent = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pToppagePointEvent == NULL)
		return;

	if (IsLocalContent((localContentType_t)0x16 /*983: era 0x15; medido*/))
	{
		m_pToppagePointEvent->SetPushSound("ui_desktop_icon_click");
		m_pToppagePointEvent->EnableHover(true, 0.5f);
		m_pToppagePointEvent->SetVisible(true);
		m_pToppagePointEvent->SetPushDelay(0.0f);

		if (IsLocalContent(S4_TOPPAGE_BTN_LINEUP))
		{
			WRect rect = m_pToppagePointEvent->GetRect();
			float x = GetTopPageBtnRect();
			rect.x = x;
			m_pToppagePointEvent->SetRect(rect);
		}
		else if (!CLoginInfo::Instance()->IsPcBang())
		{
			WRect pcbangRect = m_pToppagePcbang->GetRect();
			WRect rect = m_pToppagePointEvent->GetRect();
			rect.x = pcbangRect.x;
			rect.y = pcbangRect.y - 2.0f;
			m_pToppagePointEvent->SetRect(rect);
		}
		else
		{
			WRect rect = m_pToppagePointEvent->GetRect();
			rect.x = 160.0f;
			rect.y -= 2.0f;
			m_pToppagePointEvent->SetRect(rect);
		}
	}
	else
	{
		m_pToppagePointEvent->SetVisible(false);
	}
}

void CLobbyMain::OnToppage_PointEventDown()
{
	if (IsLocalContent((localContentType_t)0x16 /*983: era 0x15; medido*/))
	{
		CloseAllDialogs();
		WSendPacket packet((enumClientPacket)0x98);
		packet.Send(TO_GAME);

		Doc()->m_pendingMenu = 2;
		Doc()->m_bToppageAd = false;
	}
}

void CLobbyMain::OnToppage_ValEventInit(int param)
{
	m_pToppageValEvent = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pToppageValEvent)
	{
		m_pToppageValEvent->SetVisible(
			IsLocalContent((localContentType_t)0x78));
		if (m_pToppageValEvent->IsVisible())
		{
			m_pToppageValEvent->SetPushDelay(0.1f);

			cTopEventIcon icon((eTopIconID)8);
			icon.Prepare(m_pToppageValEvent);
		}
	}
}

void CLobbyMain::OnToppage_ValEventDown()
{
	if (IsLocalContent((localContentType_t)0x78) == true)
	{
		WSendPacket packet((enumClientPacket)0xda); // JP 983 (s63): 0xdc no KR 645
		packet.Send(TO_GAME);
	}
}

void CLobbyMain::OnToppage_WorldTourEventInit(int param)
{
	m_pToppageWorldTourEvent = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pToppageWorldTourEvent == NULL)
		return;

	if (IsLocalContent((localContentType_t)0x1f /*983: era 0x1e; medido*/))
	{
		m_pToppageWorldTourEvent->SetPushSound("ui_desktop_icon_click");
		m_pToppageWorldTourEvent->EnableHover(true, 0.5f);
		m_pToppageWorldTourEvent->SetVisible(true);
		m_pToppageWorldTourEvent->SetPushDelay(0.0f);

		bool bPcBang = CLoginInfo::Instance()->IsPcBang() != 0;
		bool bPointEvent = IsLocalContent((localContentType_t)0x16 /*983: era 0x15; medido*/);

		WRect pcbangRect = m_pToppagePcbang->GetRect();
		WRect pointRect = m_pToppagePointEvent->GetRect();
		WRect rect = m_pToppageWorldTourEvent->GetRect();

		rect.y += 2.0f;

		if (IsLocalContent(S4_TOPPAGE_BTN_LINEUP))
		{
			WRect btnRect = m_pToppageWorldTourEvent->GetRect();
			float x = GetTopPageBtnRect();
			btnRect.x = x;
			m_pToppageWorldTourEvent->SetRect(btnRect);
		}
		else if (bPcBang && bPointEvent)
		{
			rect.x = pointRect.x + pointRect.w;
			m_pToppageWorldTourEvent->SetRect(rect);
		}
		else if (bPcBang || bPointEvent)
		{
			if (bPcBang)
			{
				rect.x = pcbangRect.x + pcbangRect.w;
				m_pToppageWorldTourEvent->SetRect(rect);
			}
			else
			{
				rect.x = pointRect.x + pointRect.w;
				m_pToppageWorldTourEvent->SetRect(rect);
			}
		}
	}
	else
	{
		m_pToppageWorldTourEvent->SetVisible(false);
		m_pToppageWorldTourEvent->Enable(false);
	}
}

void CLobbyMain::OnToppage_WorldTourEventDown()
{
	if (!IsLocalContent((localContentType_t)0x1f /*983: era 0x1e; medido*/))
		return;

	if (m_pToppageWorldTourEvent == NULL)
		return;

	CloseAllDialogs();

	CWorldTourEvent* pEvent =
		(CWorldTourEvent*)CContentsDoc::Instance()->GetContainer(
			(localContentType_t)0x1f);
	if (pEvent == NULL)
		return;

	if (m_pWorldTourEventDlg)
	{
		m_pWorldTourEventDlg->SetMapFlag(pEvent->GetMapFlag());
		return;
	}

	m_pWorldTourEventDlg = CreateForm<FrWorldTourEventDlg>(
		g_pFresh->GetManager(), this, "world_tour_event_dlg");

	if (m_pWorldTourEventDlg == NULL)
		return;

	m_pWorldTourEventDlg->SetMapFlag(pEvent->GetMapFlag());
	m_pWorldTourEventDlg->SetIconRect(m_pToppageWorldTourEvent->GetRect());
	m_pWorldTourEventDlg->Open(
		(FRESH_PFN_RESULT)&CLobbyMain::OnWorldTourEventDlgResult, 0x11);
}

void CLobbyMain::OnToppage_FindEbortEventInit(int param)
{
	m_pToppageFindEbortEvent = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pToppageFindEbortEvent == NULL)
		return;

	if (IsLocalContent((localContentType_t)0x21 /*983: era 0x20; interpolado*/))
	{
		m_pToppageFindEbortEvent->SetVisible(true);
		m_pToppageFindEbortEvent->Enable(true);
		m_pToppageFindEbortEvent->SetPushDelay(0.0f);

		bool bPcBang = CLoginInfo::Instance()->IsPcBang() != 0;
		bool bPointEvent = IsLocalContent((localContentType_t)0x16 /*983: era 0x15; medido*/);
		bool bWorldTour = IsLocalContent((localContentType_t)0x1f /*983: era 0x1e; medido*/);

		WRect rect = m_pToppageFindEbortEvent->GetRect();

		if (bPcBang)
			rect.x +=
				m_pToppagePcbang->GetRect().x + m_pToppagePcbang->GetRect().w;

		if (bPointEvent)
			rect.x += m_pToppagePointEvent->GetRect().x +
				m_pToppagePointEvent->GetRect().w;

		if (bWorldTour)
			rect.x += m_pToppageWorldTourEvent->GetRect().x +
				m_pToppageWorldTourEvent->GetRect().w;

		rect.y -= 5.0f;
		m_pToppageFindEbortEvent->SetRect(rect);
	}
	else
	{
		m_pToppageFindEbortEvent->SetVisible(false);
		m_pToppageFindEbortEvent->Enable(false);
	}
}

void CLobbyMain::OnToppage_FindEbortEventDown()
{
	if (!IsLocalContent((localContentType_t)0x21 /*983: era 0x20; interpolado*/))
		return;

	if (m_pToppageFindEbortEvent == NULL)
		return;

	CloseAllDialogs();

	if (m_pFindEbortEventDlg)
		return;

	m_pFindEbortEventDlg = CreateForm<FrFindEbortEventDlg>(
		g_pFresh->GetManager(), this, "find_ebort_event_dlg");

	if (m_pFindEbortEventDlg == NULL)
		return;

	m_pFindEbortEventDlg->SetIconRect(m_pToppageFindEbortEvent->GetRect());
	m_pFindEbortEventDlg->Open(
		(FRESH_PFN_RESULT)&CLobbyMain::OnFindEbortEventDlgResult, 0x11);
}

void CLobbyMain::OnToppage_Halloween2007Init(int param)
{
	m_pToppageHalloween2007Event = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pToppageHalloween2007Event)
	{
		if (IsLocalContent((localContentType_t)0x22 /*983: era 0x21; interpolado*/))
		{
			if (CContentsDoc::Instance()->GetContainer(
					(localContentType_t)0x22))
			{
				m_pToppageHalloween2007Event->SetVisible(true);
				m_pToppageHalloween2007Event->Enable(true);
			}
			else
			{
				m_pToppageHalloween2007Event->SetVisible(false);
				m_pToppageHalloween2007Event->Enable(false);
			}

			cTopEventIcon icon((eTopIconID)9);
			icon.Prepare(m_pToppageHalloween2007Event);
		}
		else
		{
			m_pToppageHalloween2007Event->SetVisible(false);
			m_pToppageHalloween2007Event->Enable(false);
		}
	}
}

void CLobbyMain::OnToppage_Halloween2007EvenDown()
{
	if (!IsLocalContent((localContentType_t)0x22 /*983: era 0x21; interpolado*/))
		return;

	if (m_pToppageHalloween2007Event == NULL)
		return;

	CloseAllDialogs();

	if (m_pHalloweenEventDlg)
		return;

	m_pHalloweenEventDlg = CreateForm<FrHalloweenEventDlg>(
		g_pFresh->GetManager(), this, "halloween2007_event_dlg");

	if (m_pHalloweenEventDlg == NULL)
		return;

	m_pHalloweenEventDlg->SetIconRect(m_pToppageHalloween2007Event->GetRect());
	m_pHalloweenEventDlg->Open(
		(FRESH_PFN_RESULT)&CLobbyMain::OnHalloweenEventDlgResult, 0x11);
}
void CLobbyMain::OnToppage_Christmas2007EventInit(int param)
{
	m_pToppageChristmas2007Event = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	CChristmasEvent* pEvent;
	if (IsLocalContent((localContentType_t)0x4d /*983: era 0x4c; interpolado*/) &&
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x4d,
			pEvent) &&
		pEvent->GetFlag())
	{
		m_pToppageChristmas2007Event->SetVisible(true);
		m_pToppageChristmas2007Event->Enable(true);

		cTopEventIcon((eTopIconID)7).Prepare(m_pToppageChristmas2007Event);
	}
	else
	{
		m_pToppageChristmas2007Event->SetVisible(false);
		m_pToppageChristmas2007Event->Enable(false);
	}
}

void CLobbyMain::OnToppage_Christmas2007EventDown()
{
	if (IsLocalContent((localContentType_t)0x4d /*983: era 0x4c; interpolado*/))
	{
		if (m_pToppageChristmas2007Event)
		{
			CloseAllDialogs();

			if (m_pChristmasEventDlg == NULL)
			{
				m_pChristmasEventDlg = CreateForm<FrChristmasEventDlg>(
					g_pFresh->GetManager(), this, "christmas2007_event_dlg");

				if (m_pChristmasEventDlg)
				{
					m_pChristmasEventDlg->SetIconRect(
						m_pToppageChristmas2007Event->GetRect());
					m_pChristmasEventDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::
												   OnChristmasEventDlgResult,
						0x11);
				}
			}
		}
	}
}

void CLobbyMain::OnToppage_TicketExchangeInit(int param)
{
	m_pToppageTicketExchange = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	CTicketExchange* pEvent;
	if (IsLocalContent((localContentType_t)0x77 /*983: era 0x7a; medido 005943a0*/) &&
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x77,
			pEvent) &&
		pEvent->GetFlag())
	{
		m_pToppageTicketExchange->SetVisible(true);
		m_pToppageTicketExchange->Enable(true);

		cTopEventIcon((eTopIconID)10).Prepare(m_pToppageTicketExchange);

		m_pToppageTicketExchange->SetPushSound("ui_desktop_icon_click");
		m_pToppageTicketExchange->EnableHover(true, 0.5f);
		m_pToppageTicketExchange->SetVisible(true);
		m_pToppageTicketExchange->SetPushDelay(0.0f);

		if (IsLocalContent(S4_TOPPAGE_BTN_LINEUP))
		{
			WRect rect = m_pToppageTicketExchange->GetRect();
			float x = GetTopPageBtnRect();
			rect.x = x;
			m_pToppageTicketExchange->SetRect(rect);
		}
	}
	else
	{
		m_pToppageTicketExchange->SetVisible(false);
		m_pToppageTicketExchange->Enable(false);
	}
}

void CLobbyMain::OnToppage_TicketExchangeDown()
{
	if (IsLocalContent((localContentType_t)0x77 /*983: era 0x7a; medido 005968c0*/))
	{
		if (m_pToppageTicketExchange)
		{
			CloseAllDialogs();

			if (m_pTicketExchangeDlg == NULL)
			{
				m_pTicketExchangeDlg = CreateForm<FrTicketExchangeDlg>(
					g_pFresh->GetManager(), this, "ticket_exchage_dlg");

				if (m_pTicketExchangeDlg)
				{
					m_pTicketExchangeDlg->SetIconRect(
						m_pTicketExchangeDlg->GetRect());
					m_pTicketExchangeDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::
												   OnTicketExchangeDlgResult,
						0x11);
				}
			}
		}
	}
}

void CLobbyMain::OnToppage_MissionEventInit(int param)
{
	m_pToppageMissionEvent = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	cTopEventIcon((eTopIconID)13).Prepare(m_pToppageMissionEvent);

	if (!IsLocalContent((localContentType_t)0x80 /*983: era 0x83; medido*/))
	{
		m_pToppageMissionEvent->Enable(false);
		m_pToppageMissionEvent->SetVisible(false);
	}
}

void CLobbyMain::OnToppage_MissionEventDown()
{
	if (IsLocalContent((localContentType_t)0x80 /*983: era 0x83; medido*/))
	{
		WSendPacket packet((enumClientPacket)0xe1);
		packet.Send(TO_GAME);

		this << MsgObject(this, M983(0x1), 0, 0, 0, 0, 0);
	}
}

void CLobbyMain::OnToppage_BingoEventInit(int param)
{
	m_pToppageBingoEvent = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	cTopEventIcon((eTopIconID)12).Prepare(m_pToppageBingoEvent);

	if (!IsLocalContent((localContentType_t)0x82 /*983: era 0x85; medido*/))
	{
		m_pToppageBingoEvent->Enable(false);
		m_pToppageBingoEvent->SetVisible(false);
	}
}

void CLobbyMain::OnToppage_BingoEventDown()
{
	if (IsLocalContent((localContentType_t)0x82 /*983: era 0x85; medido*/))
	{
		WSendPacket packet((enumClientPacket)0xe3);
		packet.Encode4(0);
		packet.Send(TO_GAME);

		this << MsgObject(this, M983(0x1), 0, 0, 0, 0, 0);
	}
}

void CLobbyMain::OnToppage_KoohBirthdayEventInit(int param)
{
	m_pToppageKoohBirthdayEvent = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	std::string name("");
	if (IsLocalContent((localContentType_t)0x8b /*983: era 0x8e; medido no Down 00594f90*/))
	{
		name = "KoohBirthdayEvent";
	}

	if (IsLocalContent((localContentType_t)0x91 /*983: era 0x94; interpolado (-3)*/))
	{
		name = "HanaBirthdayEvent";
	}
	else if (IsLocalContent((localContentType_t)0x98 /*983: era 0x9b; interpolado (-3)*/))
	{
		name = "ArinBirthdayEvent";
	}

	if (name != "")
	{
		CGDBitmapBox* pBox =
			(CGDBitmapBox*)CGameDataDB::Instance()->GetGameData(
				GAMEDATA_BITMAPBOX, name.c_str());
		m_pToppageKoohBirthdayEvent->SetButtonImg(
			pBox->GetFileName("LobbyIconNormal"), FrButton::NORMAL);
		m_pToppageKoohBirthdayEvent->SetButtonImg(
			pBox->GetFileName("LobbyIconOver"), FrButton::OVER);
		m_pToppageKoohBirthdayEvent->SetButtonImg(
			pBox->GetFileName("LobbyIconBlink"), FrButton::BLINK);

		cTopEventIcon((eTopIconID)6).Prepare(m_pToppageKoohBirthdayEvent);
	}
	else
	{
		m_pToppageKoohBirthdayEvent->Enable(false);
		m_pToppageKoohBirthdayEvent->SetVisible(false);
	}
}

void CLobbyMain::OnToppage_KoohBirthdayEventDown()
{
	// 983 (00594f90): só com o conteúdo 0x8b
	if (!IsLocalContent((localContentType_t)0x8b))
		return;

	WSendPacket packet(0xed); // JP 983 (s63): 0xef no KR 645
	packet.Send(TO_GAME);
	this << MsgObject(this, M983(0x1), 0, 0, 0, 0, 0);
}

void CLobbyMain::OnToppage_TreasureIslandInit(int param)
{
	m_pToppageTreasureIsland = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	cTopEventIcon((eTopIconID)11).Prepare(m_pToppageTreasureIsland);

	if (!IsLocalContent((localContentType_t)0x89 /*983: era 0x8c; medido 00594810*/))
	{
		m_pToppageTreasureIsland->Enable(false);
		m_pToppageTreasureIsland->SetVisible(false);
	}
}

void CLobbyMain::OnToppage_TreasureIslandLBDown()
{
	if (IsLocalContent((localContentType_t)0x89 /*983: era 0x8c; mesmo conteúdo do Init*/))
	{
		ShellExecuteA(NULL, NULL,
			"http://qa.pangya.gametree.co.kr/events/bomul/PangyaBomul.aspx",
			NULL, NULL, SW_SHOWMAXIMIZED);
	}
}

void CLobbyMain::OnToppage_ServerInit(int param)
{
	m_pToppageServer = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToppageServer)
	{
		m_pToppageServer->SetPushSound("ui_desktop_icon_click");
		m_pToppageServer->EnableHover(true, 0.5f);
	}
}

void CLobbyMain::OnToppage_ServerUp()
{
	CloseAllDialogs();
	OnUnderBar_ServerUp();
}

void CLobbyMain::CloseAllDialogs()
{
	if (m_pOptionDlg)
		m_pOptionDlg->Close(true);

	if (m_pPCBangNoticeDlg)
		m_pPCBangNoticeDlg->Close(true);
	// 983 00ab6480: também o cartão especial (+0x188) e o Premium Service (+0x438)
	if (m_pSpecialCardDlg)
		m_pSpecialCardDlg->Close(FrCANCEL, true);
	if (m_pPremiumServiceDlg983)
		m_pPremiumServiceDlg983->Close(true);

	if (CMessengerInfo::Instance()->IsVisible())
		CMessengerInfo::Instance()->Toggle();

	if (CUserInfo::IsInstantiated())
		CUserInfo::Instance()->Close();

	if (m_pServerDlg)
		m_pServerDlg->Close(false);

	if (m_pPointEventDlg)
	{
		m_pPointEventDlg->Close(true);
	}

	if (m_pCreateNickDlg)
	{
		m_pCreateNickDlg->CloseDlg(0);
	}

	CIconToolTip::GetInstance()->ClearToolTip();

	CTaskMain::CloseAllDialogs();
}

void CLobbyMain::OnServer_Init(int param)
{
	CTaskMain::OnInit();
	TurnOn3DBackground();
}

void CLobbyMain::OnServer_Finish()
{
	if (m_pServerDlg == NULL)
	{
		m_pServerDlg =
			CreateForm<FrServerDlg>(g_pFresh->GetManager(), this, "server");

		m_pServerDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnServerDlgResult,
			0x10);
	}
}

// 983 0098cbb0 (commonutil; sem par no 645): corta `str` em `maxLen` bytes sem partir um caractere de 2 bytes do Shift-JIS
static void TruncateSjis983(char* str, int maxLen)
{
	if (!str || strlen(str) <= (size_t)maxLen)
		return;

	int i = 0;
	while (i < maxLen)
	{
		unsigned char c = (unsigned char)str[i];
		if ((c >= 0x80 && c < 0xa0) || (unsigned char)(c + 0x20) <= 0x1c)
		{
			if (i + 1 >= maxLen)
				break;
			i += 2;
		}
		else
		{
			i++;
		}
	}
	str[i] = 0;
}

// 983 00641650 (metodo do CSharedDoc sem par no 645; tambem chamado pelo FrMakeRoomDlg 00ae26c0): campo sorteado para a sala
// de pratica. Entram os campos da tabela cuja dificuldade (Course.iff) cabe no nivel do jogador; o sorteio usa o peso pela
// POSICAO no vetor dos aceitos (como o 983 faz), nao pelo indice da tabela. Sem nenhum campo: 0x14.
unsigned char PickRandomCourse983()
{
	static const unsigned char s_course[21] = { // 00d15fe8
		0x14, 0x15, 0x12, 0x13, 0x10, 0x0f, 0x0e, 0x0d, 0x0b, 0x08, 0x0a, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09, 0x7f };
	static const int s_weight[21] = { // 00d15ef8
		100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 0, 0 };

	unsigned char level = Doc()->m_myInfo.stat.Level;
	char maxDifficulty;
	if (level <= 10)
		maxDifficulty = 2;
	else if ((unsigned char)(level - 11) < 15)
		maxDifficulty = 3;
	else
		maxDifficulty = ((unsigned char)(level - 26) > 0x13) + 4;

	std::vector<unsigned long> courses;
	std::vector<int> starts;
	int total = 0;
	for (int i = 0; i < 21; i++)
	{
		if (s_course[i] == 0x7f || s_weight[i] <= 0)
			continue;

		IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(s_course[i] | 0x28000000);
		if (pCourse && (char)pCourse->Difficulty <= maxDifficulty)
		{
			courses.push_back(s_course[i]);
			starts.push_back(total + 1);
			total += s_weight[i];
		}
	}

	if (total == 0)
		return 0x14;

	int pick = rand() % total + 1;
	for (unsigned int j = 0; j < starts.size(); j++)
	{
		if (starts[j] <= pick && pick <= s_weight[j] + starts[j])
			return (unsigned char)courses[j];
	}
	return (unsigned char)courses[0];
}

// 983 00595f60 (chamado pelo botao do modo solo do RFLobbyDesktop, 005f7540)
void CLobbyMain::OpenSingleModeStarter983()
{
	CIconToolTip::GetInstance()->ClearToolTip();

	FrSingleModeStarter* pForm =
		CreateForm<FrSingleModeStarter>(g_pFresh->GetManager(), this, "single_mode_start", NULL);
	pForm->Open((FRESH_PFN_RESULT)&CLobbyMain::OnSingleModeStartResult983, 0x21);
}

// 983 00abaa10: resultado da "single_mode_start". Modo 0 = percurso, 4 = repeticao (buraco fixo), 6 = chip-in; monta a sala de
// 1 jogador (pacote 0x8, mesmo formato do FrMakeRoomDlg::MakeRoom) com senha aleatoria, depois de preparar a lista de salas
// como o FrMultiplayRoomDlg::InitRoomList983 (005173c0, aqui em linha).
bool CLobbyMain::OnSingleModeStartResult983(int result, FrForm* form)
{
	if (result != FrYES)
		return true;

	FrSingleModeStarter* pStarter = DYNAMIC_CAST(FrSingleModeStarter, form);
	if (!pStarter)
		return true;

	unsigned char mode = pStarter->GetMode983();
	switch (mode)
	{
	case 0:
	case 6:
		Doc()->m_roomInfo.holeType = 0;
		break;

	case 4:
		Doc()->m_roomInfo.holeType = 4;
		DOCJP()->SetFixHole983(1);
		Doc()->m_bFixHoleRepeat983 = 1;
		break;

	default:
		return true;
	}

	Doc()->m_roomInfo.nHole = 0x13;
	Doc()->m_bRebuildUnderBar = false;
	SendMsg(this, "Lobby", M983(0xeb), 0, 0, 0, 0);

	Doc()->m_roomInfo.gameType = 0x17;
	Doc()->m_roomInfo.realGameType = 0x17;

	for (std::map<unsigned long, sRoomSlot>::iterator it =
			 Doc()->m_roomSlotMap.begin();
		it != Doc()->m_roomSlotMap.end(); ++it)
	{
		if (it->second.pExhibition)
		{
			delete (CExhibition983*)it->second.pExhibition; // 983: destrutor virtual (como na FrMultiplayRoomDlg)
			it->second.pExhibition = NULL;
		}
	}
	Doc()->m_roomSlotMap.clear();

	if (IsLocalContent(S4_UCC))
	{
		NetResourceManager::Instance()->SetClearGarbage();
	}

	if (!IsGoodMannerRate983())
	{
		CPartTidList& partTidList = Doc()->GetMyPartTidList();
		sCharacterInfo& charInfo =
			Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];

		for (int i = 0; i < partTidList.m_partNum; i++)
		{
			if (IsNoMannerPart983(charInfo.tidParts[i]))
			{
				charInfo.tidParts[i] = 0;
				charInfo.ItemIdList[i] = 0;
			}

			if (IsNoMannerPart983(partTidList.m_tid[i]))
				partTidList.m_tid[i] = 0;
		}
	}

	std::string title;
	std::string password;
	bool bPublic = false;
	unsigned long shotTime = 0;
	unsigned long gameTime;
	unsigned char maxPlayer = 1;
	unsigned char gameType;
	unsigned char holeNum;
	unsigned char course;
	unsigned char holeType = Doc()->m_roomInfo.holeType;

	title = K2L_Compatibility("\x31\xc0\xce \xbf\xac\xbd\xc0 \xb8\xf0\xb5\xe5"); // "1-in yeonseup modeu" (pratica de 1 jogador)

	char szRand[8] = { 0 };
	char szPassword[256] = { 0 };
	_snprintf_s(szRand, 8, _TRUNCATE, "%07d", rand());
	S5::UTIL::Base64Encode(szRand, (int)strlen(szRand), szPassword);
	TruncateSjis983(szPassword, 8);
	password = szPassword;

	if (mode == 6)
	{
		gameTime = 1800000;
		gameType = 0x0e;
		holeNum = 1;
		course = 0x40;
	}
	else
	{
		gameTime = 2100000;
		gameType = 0x13;
		holeNum = 0x12;
		course = PickRandomCourse983();
	}

	WSendPacket packet((enumClientPacket)8);
	packet.Encode1(bPublic);
	packet.Encode4(shotTime);
	packet.Encode4(gameTime);
	packet.Encode1(maxPlayer);
	packet.Encode1(gameType);
	packet.Encode1(holeNum);
	packet.Encode1(course);
	packet.Encode1(holeType);
	if (holeType == 4)
	{
		packet.Encode1(DOCJP()->GetFixHole983());
		packet.Encode4(Doc()->m_bFixHoleRepeat983 ? 7 : 6);
	}
	packet.Encode4(0);
	packet.EncodeStr(title);
	packet.EncodeStr(password);
	packet.Encode4(0);
	packet.Send(TO_GAME);

	return true;
}

// 983 00ab6620: resultado da lista de salas do 983 (FrMultiplayRoomDlg "roomlist" ou FrGrandPrixRoomDlg "grandprix_list", em
// m_pRoomListForm983; abertas pelo HandleMsg 0x8b caso 4). Ao sair da lista avisa o servidor pelo tipo da lista (0062bf50: 0 = salas
// comuns, pacote 0x82 como o SendRoomListExitPacket; 1 = Grand Prix, 0x177), passa a mensagem 1 ao proprio HandleMsg e zera o
// estado de +0x11638 (0062bf80).
bool CLobbyMain::OnRoomListDlgResult983(int result, FrForm* form)
{
	m_pRoomListForm983 = 0;
	if (result == 0)
		return true;

	int type = DOCJP()->GetRoomListKind983();
	if (type == 0)
	{
		WSendPacket packet((enumClientPacket)0x82);
		packet.Send(TO_GAME);
	}
	else if (type == 1)
	{
		WSendPacket packet((enumClientPacket)0x177);
		packet.Send(TO_GAME);
	}

	HandleMsg(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	Doc()->m_lobbyState983 = 0;
	return true;
}


// 983: CSharedDoc 0063d300 (fica no shareddoc.cpp do 983; aqui ate a A levar): apaga os bonecos das vagas da sala e esvazia
// o mapa (o mesmo que o OnSingleModeStartResult983 faz embutido)
static void ClearRoomSlotMap983()
{
	for (std::map<unsigned long, sRoomSlot>::iterator it = Doc()->m_roomSlotMap.begin(); it != Doc()->m_roomSlotMap.end(); ++it)
	{
		if (it->second.pExhibition)
		{
			delete (CExhibition983*)it->second.pExhibition; // destrutor virtual
			it->second.pExhibition = NULL;
		}
	}
	Doc()->m_roomSlotMap.clear();
}
// 983 00ab6cc0 (HandleMsg 0x8b casos 6..0xb): fecha a lista de salas e a janela da sala aberta, grava o estado (CSharedDoc+0x11638,
// 0062bf80) e abre a janela da sala desse estado: 3 = FrStrokeTypeRoomDlg, 4/5 = FrMassTypeRoomDlg, 6 = FrGPTypeRoomDlg (todas
// "gameroom_main"), 7 = FrSingleTypeDlg ("gameroom_single", centrada pela largura da tela), 8 = FrMassTypeRoomDlg sem Open(1).
// Resultado OnGameRoomDlgResult983 (00ab6700).
void CLobbyMain::OpenGameRoomDlg983(int state)
{
	unsigned long openFlag = 0;
	if (m_pRoomListForm983)
		((FrForm*)m_pRoomListForm983)->Close((eFormRet)0, false);
	if (m_pGameRoomDlg983)
	{
		m_pGameRoomDlg983->Close((eFormRet)0, false);
		m_pGameRoomDlg983 = NULL;
	}

	Doc()->m_lobbyState983 = state; // 0062bf80
	switch (state)
	{
	case 3:
		m_pGameRoomDlg983 = CreateForm<FrStrokeTypeRoomDlg>(g_pFresh->GetManager(), this, "gameroom_main", NULL);
		openFlag = 1;
		break;
	case 4:
	case 5:
		m_pGameRoomDlg983 = CreateForm<FrMassTypeRoomDlg>(g_pFresh->GetManager(), this, "gameroom_main", NULL);
		openFlag = 1;
		break;
	case 6:
		m_pGameRoomDlg983 = CreateForm<FrGPTypeRoomDlg>(g_pFresh->GetManager(), this, "gameroom_main", NULL);
		openFlag = 1;
		break;
	case 7:
	{
		m_pGameRoomDlg983 = CreateForm<FrSingleTypeDlg>(g_pFresh->GetManager(), this, "gameroom_single", NULL);
		WRect rect = m_pGameRoomDlg983->GetRect();
		rect.x += (float)((g_view->GetWidth() - 800.0) * 0.3); // 983: WView+0x19c
		m_pGameRoomDlg983->SetRect(rect);
		openFlag = 2;
		goto skipOffset;
	}
	case 8:
		m_pGameRoomDlg983 = CreateForm<FrMassTypeRoomDlg>(g_pFresh->GetManager(), this, "gameroom_main", NULL);
		break;
	}

	{
		WRect rect = m_pGameRoomDlg983->GetRect();
		rect.y += 35.0f;
		m_pGameRoomDlg983->SetRect(rect);
	}
skipOffset:
	FrGameRoomDlg* pDlg = (FrGameRoomDlg*)m_pGameRoomDlg983;
	pDlg->Set154_983((int)m_pGPTimerFont983.get(), (int)m_pGPTicketFont983.get()); // 004ff300: objetos de +0x784 e +0x794
	pDlg->SetState983(state); // 004ff200
	m_pGameRoomDlg983->EnableDrag(false);
	m_pGameRoomDlg983->Open((FRESH_PFN_RESULT)&CLobbyMain::OnGameRoomDlgResult983, openFlag);
}

// 983 00ab6700: resultado da janela da sala (OpenGameRoomDlg983). Com resultado != 0 reabre a entrada da lista de salas e sai
// da sala pelo estado (CSharedDoc+0x11638, 0062bf90): 3..5 = SendExitRoom983 e estado 1; 6 (Grand Prix) = SendExitGPRoom983 e
// estado 2 (se o Grand Prix ainda vale: byte +0x10488 zerado ou 0062bf30 != 0); 7 (modo solo) = desfaz a sala de repeticao,
// liga o sussurro da tarefa, apaga o buraco fixo (0xff), SendExitRoom983 e estado 0. Depois esvazia os bonecos das vagas
// (CSharedDoc 0063d300) e passa a mensagem 1 ao proprio HandleMsg.
bool CLobbyMain::OnGameRoomDlgResult983(int result, FrForm* form)
{
	m_pGameRoomDlg983 = NULL;
	if (result == 0)
		return true;

	if (m_pRoomListForm983)
		((FrRoomListDlg*)m_pRoomListForm983)->EnableRoomEnterControls(true);

	switch (DOCJP()->GetState983())
	{
	case 3:
	case 4:
	case 5:
		S5::UTIL::SendExitRoom983(0);
		Doc()->m_lobbyState983 = 1;
		break;
	case 6:
		if (!Doc()->m_bGameOver /*+0x10488*/ || DOCJP()->GetGPChannel983() != 0)
		{
			S5::UTIL::SendExitGPRoom983(0);
			Doc()->m_lobbyState983 = 2;
		}
		break;
	case 7:
		if (Doc()->m_roomInfo.holeType == 4)
			Doc()->m_roomInfo.holeType = 0;
		(CTaskManager::Instance() ? CTaskManager::Instance()->GetCurrentTask() : NULL)->SetWhisper(false);
		DOCJP()->SetFixHole983(0xff);
		Doc()->m_bFixHoleRepeat983 = 0;
		S5::UTIL::SendExitRoom983(0);
		Doc()->m_lobbyState983 = 0;
		break;
	}

	ClearRoomSlotMap983(); // CSharedDoc 0063d300
	HandleMsg(MsgObject(this, 1, 0, 0, 0, 0, 0));
	return true;
}

bool CLobbyMain::OnNewRecordDlgResult(int result, FrForm* form)
{
	m_pNewRecordDlg = NULL;
	return true;
}

void CLobbyMain::OnLogin_Init(int param)
{
	CTaskMain::OnInit();

	if (NET()->IsConnected(WNetworkSystem::NET_MSN))
	{
		CMessengerInfo::Instance()->Close(true);
		NET()->ForceShutDown(WNetworkSystem::NET_MSN);
	}

	Doc()->ClearAll();
	Buddy()->ClearBuddy();

	TurnOn3DBackground();
}

// 983 (005934b0): copyright no canto inferior direito da tela real, 126 pixels antes do copyright2
void CLobbyMain::OnLogin_Copyright1Init983(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	const Bitmap* pBmp = pArea->GetBgImg();
	if (pBmp)
	{
		WRect rect(g_view->GetWidth() - (float)pBmp->Width() - 126.0, g_view->GetHeight() - (float)pBmp->Height(),
			(float)pBmp->Width(), (float)pBmp->Height());
		pArea->SetRect(rect);
	}
}

// 983 (005935c0): mesmo canto, encostado na direita
void CLobbyMain::OnLogin_Copyright2Init983(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	const Bitmap* pBmp = pArea->GetBgImg();
	if (pBmp)
	{
		WRect rect(g_view->GetWidth() - (float)pBmp->Width(), g_view->GetHeight() - (float)pBmp->Height(),
			(float)pBmp->Width(), (float)pBmp->Height());
		pArea->SetRect(rect);
	}
}

extern WInputDev* g_mouse;

void CLobbyMain::OnLogin_Finish()
{
	g_mouse->ResetInputTime();
	g_ime->ResetInputTime();
	CMouseCursor::Instance()->SetActive(true);

	// 983 (00595d20): sem o CheckUIHack do 645 (MD5 de [left_ht.jpg/[middle.jpg/[right_wind_b.jpg); o
	// formulário de login abre direto. Com ele, a caixa "notify" (texto coreano) bloqueava o login (F4 c91-2).
	m_pLoginDlg = CreateForm<FrLoginDlg>(g_pFresh->GetManager(),
		(CTaskMain*)this, "login");
	m_pLoginDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnLoginDlgResult, 10);

	CCardManager::Instance()->InitCardBuffDlgPosition();
	CItemBuff::ResetBuffList();
}

void CLobbyMain::OnBlank_Init(int param)
{
	CTaskMain::OnInit();
}

void CLobbyMain::OnBlank_Finish()
{
	unsigned char change = Doc()->GetForceNicknameChange();
	if (change == 1 || change == 2)
	{
		GetActor("Lobby") << MsgObject(NULL, M983(0x249), 1, 0, 0, 0, 0);
	}
}

void CLobbyMain::OnSecond_Pwd_Init(int param)
{
	CTaskMain::OnInit();
}

void CLobbyMain::OnSecond_Pwd_Finish()
{
}

bool CLobbyMain::OnMakeRoomDlgResult(int result, FrForm* form)
{
	if (result == 1)
	{
		if (m_pMakeRoomDlg)
		{
			if (LatestVersion() && !m_pMakeRoomDlg->MakeRoom(false))
				return false;
		}
	}
	else if (m_pRoomListForm983)
	{
		// 983 00ab2f40: religa os controles da lista de salas (+0x77c, 0051ed90), que o botao "makeroom" dela
		// desligou antes do 0x8b/1; os do 645 (CLobbyMain) deixavam a barra da lista travada (A820, dono16 103-105)
		((FrRoomListDlg*)m_pRoomListForm983)->EnableRoomEnterControls(true);
	}

	m_pMakeRoomDlg = NULL;
	return true;
}

bool CLobbyMain::OnEventDlgResult(int result, FrForm* form)
{
	FrEventPrizeDlg* pDlg = (FrEventPrizeDlg*)form;
	m_pEventPrizeDlg[pDlg->GetEvent()] = NULL;

	return true;
}

bool CLobbyMain::OnMailBoxDlgResult(int result, FrForm* form)
{
	m_pMailBoxDlg = NULL;

	return true;
}

extern bool g_bQuit;

bool CLobbyMain::OnNotifyQuitDlgResult(int result, FrForm* form)
{
	g_bQuit = true;
	return true;
}

bool CLobbyMain::OnLoginDlgResult(int result, FrForm* form)
{
	if (result == 1)
	{
		if (m_pLoginDlg)
		{
			m_pLoginDlg->Login();
			m_pLoginDlg->SetCloseResult(FrNONE);
			return false;
		}
	}
	else
	{
		if (result == 2)
			g_bQuit = true;
		LogOut(0, "OnLoginDlgResult\n");
		m_pLoginDlg = NULL;
	}

	return true;
}

bool CLobbyMain::OnVoteDlgResult(int result, FrForm* form)
{
	WSendPacket packet(0x27);

	if (result == 1)
		packet.Encode1(0);
	else
		packet.Encode1(1);

	packet.Send(TO_GAME);

	return true;
}

bool CLobbyMain::OnInviteDlgResult(int result, FrForm* form)
{
	if (m_pInviteDlg == NULL)
		return false;

	if (result == 1)
	{
		if (IsLocalContent(S4_INVITE_FRIEND))
		{
			if (m_pInviteDlg->GetSvrGUID() == Doc()->m_gameServerUID &&
				m_pInviteDlg->GetChannelIdx() == Doc()->m_curChannel.Uid)
			{
				std::list<sRoomInfo>::iterator it =
					std::find(Doc()->m_roomList.begin(),
						Doc()->m_roomList.end(), m_pInviteDlg->GetRoomIdx());
				// 983 ab9275: sala dos tipos 4/5 tambem recusa quando o byte +0x91d do CSharedDoc e 0
				if (it == Doc()->m_roomList.end() ||
					((it->gameType == 4 || it->gameType == 5) &&
						Doc()->m_myInfo.stat.Level == 0))
				{
					this << MsgObject(this, M983(0x23),
						(int)K2L_Compatibility("\xc3\xca\xb4\xeb\xb9\xde\xc0\xba \xb9\xe6\xc0\xcc \xc1\xf6\xb1\xdd \xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0);
				}
				else
				{
					FrWnd* pConsent = form->FindChildByName("b_consent");
					if (pConsent)
						pConsent->Enable(false);

					WSendPacket packet((enumClientPacket)9);
					packet.Encode2(m_pInviteDlg->GetRoomIdx());
					packet.EncodeStr("");
					packet.Send(TO_GAME);

					this << MsgObject(NULL, M983(0x16), 0, 0, 0, 0, 0);
					this << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
				}
			}
			else
			{
				sFriend* pFriend =
					Buddy()->GetBuddy(m_pInviteDlg->GetFromUID());

				if (pFriend)
				{
					if (AfxGetTask()->IsCurrentActor983("realmyroom")) // 983 00ab932e: nao ha ator RealMyRoom, o form e widget
						(IActor*)this << MsgObject(NULL, M983(0xdf), 0, 0, 0, 0, 0);

					Doc()->m_gameServerList.clear();
					Doc()->m_channelList.clear();

					WSendPacket packet((enumClientPacket)0x43);
					packet.Send(TO_GAME);

					Doc()->SetGoWithModeStart(pFriend->Uid);
					Doc()->SetGoWithMode(2);
				}
			}
		}
	}
	else if (IsLocalContent(S4_INVITE_FRIEND))
	{
		Doc()->ClearDirectMoveRoomInfo();	// 983 0062bcc0, antes da comparacao
		if (m_pInviteDlg->GetSvrGUID() == Doc()->m_gameServerUID &&
			m_pInviteDlg->GetChannelIdx() == Doc()->m_curChannel.Uid)
		{
			Doc()->SetInviteMode(0);
		}
	}

	m_pInviteDlg = NULL;
	return true;
}

bool CLobbyMain::OnPasswordDlgResult(int result, FrForm* form)
{
	if (result == 1)
	{
		if (m_pPasswordDlg)
		{
			FrWnd* pWnd = m_pPasswordDlg->FindChildByName("password");

			FrEdit* pEdit = DYNAMIC_CAST(FrEdit, pWnd);

			std::string password;
			if (pEdit)
			{
				const char* text = pEdit->GetLine(1, false);
				password = text;
				HandleMsg(MsgObject(NULL, M983(0x27), (int)text, 0, 0, 0, 0));
			}

			FrWnd* pOk = m_pPasswordDlg->FindChildByName("ok");
			if (pOk)
				pOk->Enable(false);

			WSendPacket packet((enumClientPacket)9);
			packet.Encode2(Doc()->m_roomInfo.roomGuid);
			packet.EncodeStr(password);
			packet.Send(TO_GAME);

			HandleMsg(MsgObject(NULL, M983(0x16), 0, 0, 0, 0, 0));
			HandleMsg(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
	}

	m_pPasswordDlg = NULL;
	return true;
}

bool CLobbyMain::OnPCBangDlgResult(int result, FrForm* form)
{
	m_pPCBangNoticeDlg = NULL;

	if (Doc()->m_bShowNotice)
	{
		OnToppage_NoticeUp();
	}

	return true;
}
// 983 (00596370, botao premium da barra via 005f5fd0): como o PcbangUp do 645, sem SetIconRect
void CLobbyMain::OpenPremiumService983()
{
	CloseAllDialogs();
	FrNoticeDlg* pDlg = CreateForm<FrNoticeDlg>(g_pFresh->GetManager(), this, "notice_popup");
	m_pPremiumServiceDlg983 = pDlg;
	if (pDlg)
	{
		pDlg->LoadInitFile("premiumservice.ini");
		pDlg->Open((FRESH_PFN_RESULT)&CLobbyMain::OnPremiumServiceDlgResult983, 0x11); // 983 005963b8
	}
}

// 983 (00ab6860)
bool CLobbyMain::OnPremiumServiceDlgResult983(int result, FrForm* form)
{
	m_pPremiumServiceDlg983 = NULL;
	if (Doc()->m_bShowNotice)
		OnToppage_NoticeUp();
	return true;
}

bool CLobbyMain::OnCreateInfoResult(int result, FrForm* form)
{
	return true;
}

bool CLobbyMain::OnParanNickChangeResult(int result, FrForm* form)
{
	m_pParanNickDlg = NULL;

	if (Doc()->m_bAutoRefresh)
	{
		Doc()->m_bAutoRefresh = false;
		OpenEventPrizeForm(2);
	}

	OpenCaddieWarningForm();

	return true;
}

bool CLobbyMain::OnCaddieWarningDlgResult(int result, FrForm* form)
{
	m_pCaddieWarningDlg = NULL;
	OpenNoteForm();
	return true;
}

extern bool g_bQuit;

bool CLobbyMain::OnHolesEventDlgResult(int result, FrForm* form)
{
	m_pHolesEventDlg = NULL;
	return true;
}

bool CLobbyMain::OnPointEventDlgResult(int result, FrForm* form)
{
	m_pPointEventDlg = NULL;
	return true;
}

bool CLobbyMain::OnWorldTourEventDlgResult(int result, FrForm* form)
{
	m_pWorldTourEventDlg = NULL;
	return true;
}

bool CLobbyMain::OnFindEbortEventDlgResult(int result, FrForm* form)
{
	m_pFindEbortEventDlg = NULL;
	return true;
}

bool CLobbyMain::OnHalloweenEventDlgResult(int result, FrForm* form)
{
	m_pHalloweenEventDlg = NULL;
	return true;
}

bool CLobbyMain::OnChristmasEventDlgResult(int result, FrForm* form)
{
	m_pChristmasEventDlg = NULL;
	return true;
}

bool CLobbyMain::OnTicketExchangeDlgResult(int result, FrForm* form)
{
	m_pTicketExchangeDlg = NULL;
	return true;
}

bool CLobbyMain::OnQuickStartDlgResult(int result, FrForm* form)
{
	switch (result)
	{
	case 1:
		if (m_pQuickResultDlg == NULL)
		{
			m_pQuickResultDlg = CreateForm<FrQuickResultDlg>(
				g_pFresh->GetManager(), this, "quick_result");
			if (m_pQuickResultDlg)
			{
				m_pQuickResultDlg->Open(
					(FRESH_PFN_RESULT)&CLobbyMain::OnQuickResultDlgResult,
					0x40);
				m_pQuickResultDlg->SetContents(MakeStr(
					K2L_Compatibility("\xb8\xc5\xc4\xaa \xb4\xeb\xbb\xf3\xc0\xbb \xb0\xcb\xbb\xf6 \xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.")));
			}
		}
		break;
	}

	m_pQuickStartDlg = NULL;

	return true;
}

bool CLobbyMain::OnQuickResultDlgResult(int result, FrForm* form)
{
	m_pQuickResultDlg = NULL;
	return true;
}

void CLobbyMain::OpenLayout(const char* name)
{
	this << MsgObject(NULL, 0, (int)name, 0, 0, 0, 0);
}

void CLobbyMain::ResetControl()
{
	m_pToppageGame = NULL;
	m_pToppageFamily = NULL;
	m_pToppagePangyaStudy = NULL;
	m_pToppageReserved = NULL;
	m_pOverBarOption = NULL;
	m_pToppageNotice = NULL;
	m_pToppageServer = NULL;
	m_pInviteDlg = NULL;
	m_pUserListDlg = NULL;
	m_pPasswordDlg = NULL;
	m_pOptionDlg = NULL;
	m_pServerDlg = NULL;
	m_pGuildNoticeDlg = NULL;
	m_pEquipDlg = NULL;
	m_pMailBoxDlg = NULL;
	m_pPCBangNoticeDlg = NULL;
	m_pNewRecordDlg = NULL;
	m_pParanNickDlg = NULL;
	m_pCaddieWarningDlg = NULL;
	m_pDlg150 = NULL;
	m_pOnelineReqDlg = NULL;
	memset(m_pEventPrizeDlg, 0, sizeof(m_pEventPrizeDlg));
	memset(m_pStatBar, 0, sizeof(m_pStatBar));
	memset(m_pStatEdit, 0, sizeof(m_pStatEdit));
	m_createCharType = 0;
	m_createHairColor = 0;
	m_createShirtColor = 0;
	memset(m_pStatGauge983, 0, sizeof(m_pStatGauge983)); // 983: 00aba679
	m_pGuildNoticeObj = NULL;
	m_pToppageEvent1 = NULL;
	// D726: o resto do ResetControl do 983 (00ab3120) que o nosso (645) nao fazia. Sem isto, depois de trocar de pagina, a
	// lista de salas (+0x77c, fechada junto com a pagina) e os botoes/janelas de +0x234/+0x278/+0x438 ficavam apontando
	// para janelas ja destruidas (HandleMsg 0x0b..0x91 chamava AddUser983/OpenDtRoomDlg983 na lista morta).
	m_pToppageOneDayQuest = NULL;
	*(float*)&m_premiumEndNoticeAlpha983 = 1.0f;
	*(float*)&m_premiumEndNoticeFadeSpeed983 = 0.2f; // [00d097cc]
	m_pToppagePremiumEndNotice983 = NULL;
	memset(m_premiumEndNoticeBmp983, 0, sizeof(m_premiumEndNoticeBmp983));
	m_pRemovedDtRoomDlg983 = NULL;
	m_pPremiumServiceDlg983 = NULL;
	m_reserved4bc = 0;
	m_pRoomListForm983 = 0;
}

void CLobbyMain::SortRank()
{
	if (Doc()->m_roomInfo.gameType == GAME_TYPE_NEW_APPROACH)
	{
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); it++)
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
					it2 != Doc()->m_rivalList.end(); it2++)
				{
					if (it == it2 || it2->state == 3)
						continue;

					if (it->approachResultDistance >
							it2->approachResultDistance ||
						(it->approachResultDistance ==
								it2->approachResultDistance &&
							it->approachTime < it2->approachTime))
						it->rank++;
				}
			}
		}
	}
	else
	{
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); it++)
		{
			if (it->state == 3)
				continue;

			it->rank = 1;
			for (std::vector<sRivalData>::iterator it2 =
					 Doc()->m_rivalList.begin();
				it2 != Doc()->m_rivalList.end(); it2++)
			{
				if (it == it2 || it2->state == 3)
					continue;

				if (it->totalScore > it2->totalScore ||
					(it->totalScore == it2->totalScore &&
						it->totalPang < it2->totalPang))
					it->rank++;
			}
		}
	}
}

void CLobbyMain::SetLevelBar()
{
	unsigned long tidChar;
	const char* pStat;
	const unsigned long* pParts;
	const unsigned long* pAuxParts;
	unsigned long tidClub;
	const short* pClubStat = NULL;
	unsigned long tidCaddie;
	unsigned char level = 0;

	// so o layout CREATE chega aqui: GAMEROOM/GAMEROOM_EXT do 645 nao existem no lobby.xml JP (C700/C702)
	if (!g_pFresh->IsCurrentLayout("CREATE"))
		return;

	tidChar = m_createCharType | 0x4000000;
	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(tidChar);
	pStat = pChar->PCL;
	pParts = NULL;
	pAuxParts = NULL;
	tidClub = 0;
	tidCaddie = 0;

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(255);
		CCardManager::Instance()->CalcCardPeriodAndStatus();
	}

	for (unsigned char i = 0; i < 5; i++)
	{
		status_t status =
			ItemManager()->GetLevelWithPenalty((eLvlType)i, tidChar, pStat,
				pParts, pAuxParts, tidClub, pClubStat, tidCaddie, level);
		int nLevel = status.level;
		int nPenalty = status.penalty;
		unsigned char nCapacity = ItemManager()->GetCapacity((eLvlType)i,
			tidChar, pStat, pParts, tidClub, pClubStat, tidCaddie, level);

		m_pStatBar[i]->SetDestPos(nLevel, false);
		m_pStatBar[i]->SetExpand(nCapacity - nLevel);
		if (m_pStatEdit[i])
			m_pStatEdit[i]->SetCaption11(
				MakeStatusString3(nLevel, nCapacity, nPenalty));
	}
}

void CLobbyMain::OpenCreateInfo(unsigned long typeId, const char* text)
{
	std::string message;
	if (text)
		message = text;

	FrWnd* pWnd = g_pFresh->GetDesktop()->FindChildByName("information_large");
	if (pWnd == NULL)
		;
	else
		return;

	__rtti_obj = NULL;
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "information_large");

	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeId);

	FrArea* pPortrait =
		DYNAMIC_CAST(FrArea, pForm->FindChildByName("portrait"));
	if (pItem && pPortrait)
		pPortrait->SetBgImg(pItem->Icon);

	FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
	FrArea* pLevel = DYNAMIC_CAST(FrArea, pForm->FindChildByName("level"));
	if (pItem)
	{
		if (pName && pLevel)
		{
			if (pItem->IsUnderLvl)
			{
				pLevel->SetBgImg(MakeStr("level_%03d", pItem->Level + 1));
				pName->AddText(
					MakeStr(
						K2L_Compatibility("\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:              \xc0\xcc\xc7\xcf"),
						pItem->Name),
					false, true);
			}
			else
			{
				pLevel->SetBgImg(MakeStr("level_%03d", pItem->Level + 1));
				pName->AddText(
					MakeStr(
						K2L_Compatibility("\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:              \xc0\xcc\xbb\xf3"),
						pItem->Name),
					false, true);
			}
		}

		if (text == NULL)
		{
			IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pItem->TypeId);
			if (pDesc)
			{
				if (message[0])
					message += "\n\n";

				message += "\\c0xffff0000\\c";
				message += pDesc->Desc;
			}
		}
	}

	pForm->SetMessage(message.c_str(), false);
	pForm->Open(text ? (FRESH_PFN_RESULT)&CLobbyMain::OnCreateInfoResult : NULL,
		FrESCAPE | FrENTER);
}

bool LatestVersion983();

bool CLobbyMain::LatestVersion()
{
	return LatestVersion983(); // 983 0059fa80: "JP.R7.983.00" x Doc+0x10d08 (645: "645.00" x m_packetNickname)
}

// 983 0059fa80 (o LatestVersion do 645 fora da classe; chamado pelo FrGameRoomDlg/FrRoomListDlg da B): versão do servidor
// maior que a do cliente = aviso 0x23 ao ator principal. O 983 lê a string em CSharedDoc+0x10d08 (no nosso header
// m_chatModeName, gravado pelo taskmanager 009c3b0c). A174: era stub (Start Game
// do dono da sala não mandava o 0x0e).
C_ASSERT_VC10(offsetof(CSharedDoc, m_chatModeName) == 0x10d08); // A175: o campo que o 983 le (taskmanager 009c3b0c grava)
bool LatestVersion983()
{
	if (Doc()->m_chatModeName > "JP.R7.983.00")
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)K2L_Compatibility("\xc3\xd6\xbd\xc5 \xb9\xf6\xc1\xaf\xc0\xb8\xb7\xce \xbe\xf7\xb5\xa5\xc0\xcc\xc6\xae\xb0\xa1 \xc7\xca\xbf\xe4\xc7\xd5\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
		return false;
	}
	return true;
}

bool RoomStateCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
		return true;

	if (pRoom1->bPublic == false && pRoom2->bPublic == true)
		return false;

	if (pRoom1->bPublic == true && pRoom2->bPublic == false)
		return true;

	if (pRoom1->bAvailable == false && pRoom2->bAvailable == true)
		return false;

	if (pRoom1->bAvailable == true && pRoom2->bAvailable == false)
		return true;

	if (pRoom1->bAvailable == true && pRoom2->bAvailable == true)
	{
		if (pRoom1->nUserNum == pRoom1->nUserLimit &&
			pRoom2->nUserNum < pRoom2->nUserLimit)
			return false;

		if (pRoom1->nUserNum < pRoom1->nUserLimit &&
			pRoom2->nUserNum == pRoom2->nUserLimit)
			return true;

		if (pRoom1->bSleep == true && pRoom2->bSleep == false)
			return false;

		if (pRoom1->bSleep == false && pRoom2->bSleep == true)
			return true;

		if (pRoom1->bIntrusion == false && pRoom2->bIntrusion == true)
			return false;

		if (pRoom1->bIntrusion == true && pRoom2->bIntrusion == false)
			return true;
	}

	return false;
}

bool RoomStateRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
	{
		if (pRoom2->bPublic == false && pRoom1->bPublic == true)
			return true;

		return true;
	}

	if (pRoom2->bPublic == false && pRoom1->bPublic == true)
		return false;

	if (pRoom2->bPublic == true && pRoom1->bPublic == false)
		return true;

	if (pRoom2->bAvailable == false && pRoom1->bAvailable == true)
		return false;

	if (pRoom2->bAvailable == true && pRoom1->bAvailable == false)
		return true;

	if (pRoom2->bAvailable == true && pRoom1->bAvailable == true)
	{
		if (pRoom2->nUserNum == pRoom2->nUserLimit &&
			pRoom1->nUserNum < pRoom1->nUserLimit)
			return false;

		if (pRoom2->nUserNum < pRoom2->nUserLimit &&
			pRoom1->nUserNum == pRoom1->nUserLimit)
			return true;

		if (pRoom2->bSleep == true && pRoom1->bSleep == false)
			return false;

		if (pRoom2->bSleep == false && pRoom1->bSleep == true)
			return true;

		if (pRoom2->bIntrusion == false && pRoom1->bIntrusion == true)
			return false;

		if (pRoom2->bIntrusion == true && pRoom1->bIntrusion == false)
			return true;
	}

	return false;
}

bool RoomNoCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
		return true;

	return pRoom1->roomGuid < pRoom2->roomGuid;
}

bool RoomNoRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
		return true;

	return pRoom1->roomGuid > pRoom2->roomGuid;
}

bool RoomHoleCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
		return true;

	return pRoom1->nHole < pRoom2->nHole;
}

bool RoomHoleRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
		return true;

	return pRoom1->nHole > pRoom2->nHole;
}

bool RoomCourseCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
		return true;

	return pRoom1->mapType < pRoom2->mapType;
}

bool RoomCourseRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit != 100 && pRoom2->nUserLimit == 100)
		return false;

	if (pRoom1->nUserLimit == 100 && pRoom2->nUserLimit != 100)
		return true;

	return pRoom1->mapType > pRoom2->mapType;
}

bool RoomTitleCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	return lstrcmpi(pRoom1->title, pRoom2->title) < 0;
}

bool RoomTitleRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	return lstrcmpi(pRoom1->title, pRoom2->title) > 0;
}

bool RoomGameTypeCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->gameType == pRoom2->gameType)
		return pRoom1->tidMatch > pRoom2->tidMatch;

	return pRoom1->gameType < pRoom2->gameType;
}

bool RoomGameTypeRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->gameType == pRoom2->gameType)
		return pRoom1->tidMatch > pRoom2->tidMatch;

	return pRoom1->gameType > pRoom2->gameType;
}

bool RoomUserCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit > pRoom2->nUserLimit)
		return false;

	if (pRoom1->nUserLimit < pRoom2->nUserLimit)
		return true;

	return pRoom1->nUserNum <= pRoom2->nUserNum ? true : false;
}

bool RoomUserRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)b;

	if (pRoom1->nUserLimit > pRoom2->nUserLimit)
		return true;

	if (pRoom1->nUserLimit < pRoom2->nUserLimit)
		return false;

	return pRoom1->nUserNum <= pRoom2->nUserNum ? true : false;
}

bool RoomGuildCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sRoomInfo* pRoom1 = (const sRoomInfo*)a;
	const sRoomInfo* pRoom2 = (const sRoomInfo*)a;

	int g0 = pRoom1->GuildInfo.nGuildID[0];
	int g1 = pRoom1->GuildInfo.nGuildID[1];
	int myGuild = Doc()->m_myInfo.info.dwGuildId;

	return min(Abs(g0 - myGuild), Abs(g1 - myGuild)) >
		min(Abs(g0 - myGuild), Abs(g1 - myGuild));
}

bool UserGenderCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return pUser1->gender < pUser2->gender;
}

bool UserGenderRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return pUser1->gender > pUser2->gender;
}

bool UserLevelCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return pUser1->level < pUser2->level;
}

bool UserLevelRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return pUser1->level > pUser2->level;
}

bool UserGuildCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return pUser1->m_GuildId < pUser2->m_GuildId;
}

bool UserGuildRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return pUser1->m_GuildId > pUser2->m_GuildId;
}

bool UserNicknameCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return lstrcmpi(pUser1->sNick, pUser2->sNick) < 0;
}

bool UserNicknameRevCompare(const void* a, const void* b)
{
	if (a == NULL)
		return false;

	if (b == NULL)
		return true;

	const sBriefUserInfo* pUser1 = (const sBriefUserInfo*)a;
	const sBriefUserInfo* pUser2 = (const sBriefUserInfo*)b;

	return lstrcmpi(pUser1->sNick, pUser2->sNick) > 0;
}

const char* RoomListGetGameCategory(unsigned char gameType)
{
	switch (gameType)
	{
	case GAME_TYPE_STROKE:
	case GAME_TYPE_TEAM:
		return K2L_Compatibility("\xb4\xeb\xc0\xfc");

	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
		return K2L_Compatibility("\xb4\xeb\xc8\xb8");

	case GAME_TYPE_SKINS:
	case GAME_TYPE_APPROACH:
	case GAME_TYPE_NEW_APPROACH:
		return K2L_Compatibility("\xb9\xe8\xc6\xb2");

	case GAME_TYPE_AVATARCHAT:
		return K2L_Compatibility("\xb4\xeb\xc8\xad");
	default:
		return NULL;
	}
}

unsigned long DecideRoomColor(sRoomInfo* pRoom)
{
	if (pRoom->bAvailable && pRoom->bPublic)
	{
		if (pRoom->realGameType != GAME_TYPE_USEMAX)
		{
			if (!pRoom->bIntrusion)
				return 0xffffffff;
			else
				return WSingleton<CIntrusion>::Instance()->GetActiveColor();
		}
		else
			return 0;
	}
	else
		return pRoom->realGameType == GAME_TYPE_USEMAX ? 0 : 0xb2ffffff;
}

int float2int(float f);

void CLobbyMain::InitControlsSelectedUserInfo()
{
	m_subSelTypeId = 0;
	m_subSelItemId = 0;

}

void CLobbyMain::OpenNewRecordForm()
{
	if (Doc()->m_newRecordState != 0xff && m_pNewRecordDlg == NULL)
	{
		m_pNewRecordDlg = CreateForm<FrNewRecordDlg>(g_pFresh->GetManager(),
			this, "new_record", NULL);
		m_pNewRecordDlg->Open(
			(FRESH_PFN_RESULT)&CLobbyMain::OnNewRecordDlgResult, 0);
		Doc()->m_newRecordState = 0xff;
	}

	if (IsLocalContent((localContentType_t)0x19 /*983: era 0x18; interpolado*/))
	{
		if (Doc()->m_newRecordCourse == -1)
			Doc()->m_newRecordCourse = Doc()->m_myInfo.stat.eventFlag;

		if (Doc()->m_newRecordCourse != Doc()->m_myInfo.stat.eventFlag)
		{
			if (IsLocalContent((localContentType_t)0x49 /*983: era 0x48; interpolado*/))
				m_pHolesEventDlg = CreateForm<ntHolesEventDlg>(
					g_pFresh->GetManager(), this, "holes_EntranceExam", NULL);
			else
				m_pHolesEventDlg = CreateForm<ntHolesEventDlg>(
					g_pFresh->GetManager(), this, "holes_event", NULL);

			m_pHolesEventDlg->Open(
				(FRESH_PFN_RESULT)&CLobbyMain::OnHolesEventDlgResult, 0x11);
			Doc()->m_newRecordCourse = Doc()->m_myInfo.stat.eventFlag;
		}
	}

	if (IsLocalContent((localContentType_t)0x22 /*983: era 0x21; interpolado*/))
	{
		CHalloweenEvent* pEvent =
			(CHalloweenEvent*)CContentsDoc::Instance()->GetContainer(
				(localContentType_t)0x22);
		if (pEvent)
		{
			if (pEvent->GetShowGiftDlg())
			{
				if (m_pHalloweenEventGiftDlg == NULL)
				{
					m_pHalloweenEventGiftDlg =
						CreateForm<FrHalloweenEventGiftDlg>(
							g_pFresh->GetManager(), this,
							"halloween2007_event_gift_dlg", NULL);
					if (m_pHalloweenEventGiftDlg)
					{
						m_pHalloweenEventGiftDlg->Open(NULL, 0x11);
						m_pHalloweenEventGiftDlg = NULL;
					}
				}
				pEvent->SetShowGiftDlg(false);
			}
		}
	}
}

// HACK: these literals belong to headicon.obj in the original (it precedes lobbymain in the link order),
// but headicon's naked CHeadIcon::DisplayOverPlayer emits them as function-local statics; refer to those
// so lobbymain does not contribute its own copies.
extern "C" __declspec(selectany) const char rb_hi_redteam_rank[] = "30in_team_redteam_rank"; // C64.4: o estático de função do rebang não existe mais
extern "C" __declspec(selectany) const char rb_hi_blueteam_rank[] = "30in_team_blueteam_rank"; // C64.4: o estático de função do rebang não existe mais
extern "C" __declspec(selectany) const char rb_hi_30in_rank[] = "30in_rank"; // C64.4: o estático de função do rebang não existe mais
extern "C" __declspec(selectany) const char rb_hi_30in_rank_02[] = "30in_rank_02"; // C64.4: o estático de função do rebang não existe mais
extern "C" __declspec(selectany) const char rb_hi_fmt_jo[] = "%d \xc1\xb6"; // C64.4: o estático de função do rebang não existe mais

// 983: ponteiros globais (00e36bc0 / 00e36bbc) com o arquivo e a tabela da tela de criar personagem
const char* g_charSelectLua983 = "char_select_free.lua";
const char* g_charSelectTable983 = "character_select_free_table";

// 983 (00abaf40): zera a escolha, refaz os cabelos de cada personagem (HairStyle.iff), le as posicoes do
// char_select_free.lua e cria os 5 medidores da tela
void CLobbyMain::OnCreate_Init(int param)
{
	CTaskMain::OnInit();

	m_createCharType = 0;
	m_createHairColor = 0;
	m_createShirtColor = 0;
	for (int i = 0; i < 15; i++)
	{
		m_createHair983[i].clear();
		m_createHair983[i].reserve(5);
	}

	lua_system::lua_script* script =
		lua_system::lua_script_manger::GetInstance()->CreateScript(true);
	if (script && script->RunFile(g_charSelectLua983))
	{
		lua_system::lua_table_helper tbl = script->GetTable(g_charSelectTable983);
		m_statusPos983 = WPoint(LUA983_POS(tbl, "status_pos", 1),
			LUA983_POS(tbl, "status_pos", 2));
		m_hairColorPos983 = WPoint(LUA983_POS(tbl, "hair_color_pos", 1),
			LUA983_POS(tbl, "hair_color_pos", 2));
	}

	char name[32] = "";
	for (int i = 0; i < 5; i++)
	{
		m_pStatGauge983[i] = new CGaugeBar983;
		sprintf(name, "chselect_gauge_%d.tga", i + 1);
		m_pStatGauge983[i]->Init("chselect_gauge_frame.tga", name,
			WRect(m_statusPos983.x, (float)(i * 15) + m_statusPos983.y, 0, 13.0f),
			0, true, 2.0f, true, true);
	}

	// cabelos a venda (sem cash e com preco < 10000) no vetor do personagem
	std::map<unsigned int, IFF_STRUCT::sHairStyle>::iterator it;
	for (it = ItemManager()->m_HairStyleMap.begin();
		 it != ItemManager()->m_HairStyleMap.end(); ++it)
	{
		if (!it->second.c.IsCash && it->second.c.Price < 10000)
			m_createHair983[it->second.cCharID].push_back(it->second);
	}

	// personagem sem nenhum: fica o cabelo 0 dele
	for (int i = 0; i < 15; i++)
	{
		if (!m_createHair983[i].empty())
			continue;

		for (it = ItemManager()->m_HairStyleMap.begin();
			 it != ItemManager()->m_HairStyleMap.end(); ++it)
		{
			if (it->second.cCharID == i && it->second.cHairID == 0)
			{
				m_createHair983[i].push_back(it->second);
				break;
			}
		}
	}
}

void CLobbyMain::OnCreate_Finish()
{
}

// 983 (00ab9670): tambem apaga o boneco do meu lugar e os 5 medidores
void CLobbyMain::OnCreate_Destroy()
{
	std::map<unsigned long, sRoomSlot>::iterator it =
		Doc()->m_roomSlotMap.find(MyGuid(false));
	if (it != Doc()->m_roomSlotMap.end() && it->second.pExhibition)
	{
		delete (CExhibition983*)it->second.pExhibition; // 983: destrutor virtual
		it->second.pExhibition = NULL;
	}

	if (m_pCreateNickDlg)
	{
		m_pCreateNickDlg->CloseDlg(1);
		m_pCreateNickDlg = NULL;
	}

	for (int i = 0; i < 5; i++)
	{
		if (m_pStatGauge983[i])
		{
			delete m_pStatGauge983[i];
			m_pStatGauge983[i] = NULL;
		}
	}
}

// 983 (00abc070): so guarda a area e escolhe o 1o personagem (o resto mora no SelectCreateChar983 00abbb90)
void CLobbyMain::OnCreatePetInit(int param)
{
	m_pCreatePetArea983 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	SelectCreateChar983(0);
}

void CLobbyMain::OnCreatePetOwnerDraw()
{
	if (Doc()->m_bBongdariShopOpen)
		return;

	std::map<unsigned long, sRoomSlot>::iterator it =
		Doc()->m_roomSlotMap.find(MyGuid(false));
	if (it != Doc()->m_roomSlotMap.end())
	{
		CExhibition* pExhibition =
			Doc()->m_roomSlotMap[MyGuid(false)].pExhibition;

		if (pExhibition)
			Doc()->m_roomSlotMap[MyGuid(false)].pExhibition->Display(0.0f,
				0.0f);
	}
}

void CLobbyMain::OnCreate_CharInit(int param)
{
	m_pCreateChar = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool CLobbyMain::SetFirstChar(CPartTidList* pTids)
{
	IFF_STRUCT::sChar* pChar =
		ItemManager()->FindChar(0x4000000 | m_createCharType);

	if (pChar == NULL)
		return false;

	pTids->m_charTid = 0x4000000 | m_createCharType;
	pTids->m_defPartNum = pChar->nParts;
	pTids->m_partNum = pChar->nParts + pChar->nAcsries;
	pTids->m_hairColor = 0;
	pTids->m_shirtsColor = 0;

	pTids->SetDefaultTids();

	if (m_pCreateChar)
		m_pCreateChar->SetLine(1, pChar->c.Name, 0, false, 0);

	if (m_pCreateDesc)
	{
		m_pCreateDesc->ClearLine();

		IFF_STRUCT::sDesc* pDesc =
			ItemManager()->FindDesc(0x4000000 | m_createCharType);
		if (pDesc)
			m_pCreateDesc->AddText(pDesc->Desc, false, true);
	}

	return true;
}

// 983 (00ab3bd0): o personagem sai da lista ("CREATE.character_list"); o botao fica escondido
void CLobbyMain::OnCreate_CharPrevInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		pButton->UseDblClick(false);
		pButton->SetPushDelay(0.0f);
		pButton->SetVisible(false);
	}
}

// 983 (00ab3c50): o personagem sai da lista ("CREATE.character_list"); o botao fica escondido
void CLobbyMain::OnCreate_CharNextInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		pButton->UseDblClick(false);
		pButton->SetPushDelay(0.0f);
		pButton->SetVisible(false);
	}
}

void CLobbyMain::OnCreate_HairInit(int param)
{
	m_createHairColor = 0;
	m_pCreateHair = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pCreateHair)
		m_pCreateHair->SetLine(1,
			K2L_Compatibility("\xb8\xd3\xb8\xae\xbb\xf6"
			"1"),
			0, false, 0);
}

void CLobbyMain::OnCreate_HairPrevInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		pButton->SetPushDelay(0.0f);
		pButton->UseDblClick(false); // 983 00ab3d70: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

// 983 (00abb800): o indice anda no vetor de cabelos do personagem; o boneco recebe o cHairID do registro
void CLobbyMain::OnCreate_HairPrevBtnUp()
{
	CPartTidList* pTids = &Doc()->GetMyPartTidList();

	m_createHairColor = (m_createHairColor != 0
							 ? m_createHairColor
							 : (unsigned char)m_createHair983[m_createCharType].size()) -
		1;
	unsigned char hair =
		m_createHair983[m_createCharType][m_createHairColor].cHairID;

	if (Doc()->m_roomSlotMap[MyGuid(false)].pExhibition)
		Doc()
			->m_roomSlotMap[MyGuid(false)]
			.pExhibition->GetPetFrame()
			->SetHairClr983(pTids, hair, 0, 1);
	else
		pTids->m_hairColor = m_createHairColor;
}

void CLobbyMain::OnCreate_HairNextInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		pButton->SetPushDelay(0.0f);
		pButton->UseDblClick(false); // 983 00ab3df0: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

// 983 (00abb8e0)
void CLobbyMain::OnCreate_HairNextBtnUp()
{
	CPartTidList* pTids = &Doc()->GetMyPartTidList();

	if (m_createHairColor == m_createHair983[m_createCharType].size() - 1)
		m_createHairColor = 0;
	else
		++m_createHairColor;
	unsigned char hair =
		m_createHair983[m_createCharType][m_createHairColor].cHairID;

	if (Doc()->m_roomSlotMap[MyGuid(false)].pExhibition)
		Doc()
			->m_roomSlotMap[MyGuid(false)]
			.pExhibition->GetPetFrame()
			->SetHairClr983(pTids, hair, 0, 1);
	else
		pTids->m_hairColor = m_createHairColor;
}

void CLobbyMain::OnCreate_ShirtsInit(int param)
{
	m_pCreateShirt = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_createShirtColor = 0;
	if (m_pCreateShirt)
		m_pCreateShirt->SetLine(1,
			K2L_Compatibility("\xbc\xc5\xc3\xf7\xbb\xf6"
			"1"),
			0, false, 0);
}

void CLobbyMain::OnCreate_ShirtsPrevInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		pButton->SetPushDelay(0.0f);
		pButton->UseDblClick(false); // 983 00ab3f10: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void CLobbyMain::OnCreate_ShirtsPrevBtnUp()
{
	CPartTidList* pTids = &Doc()->GetMyPartTidList();

	m_createShirtColor = (m_createShirtColor == 0) ? 2 : --m_createShirtColor;

	if (m_pCreateShirt)
		m_pCreateShirt->SetLine(1,
			MakeStr(K2L_Compatibility("\xbc\xc5\xc3\xf7\xbb\xf6%d"), m_createShirtColor + 1), 0,
			false, 0);

	if (Doc()->m_roomSlotMap[MyGuid(false)].pExhibition)
		Doc()
			->m_roomSlotMap[MyGuid(false)]
			.pExhibition->GetPetFrame()
			->SetShirtsClr(pTids, m_createShirtColor);
	else
		pTids->m_shirtsColor = m_createShirtColor;
}

void CLobbyMain::OnCreate_ShirtsNextInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton)
	{
		pButton->SetPushDelay(0.0f);
		pButton->UseDblClick(false); // 983 00ab3f90: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void CLobbyMain::OnCreate_ShirtsNextBtnUp()
{
	CPartTidList* pTids = &Doc()->GetMyPartTidList();

	m_createShirtColor = (m_createShirtColor == 2) ? 0 : ++m_createShirtColor;

	if (m_pCreateShirt)
		m_pCreateShirt->SetLine(1,
			MakeStr(K2L_Compatibility("\xbc\xc5\xc3\xf7\xbb\xf6%d"), m_createShirtColor + 1), 0,
			false, 0);

	if (Doc()->m_roomSlotMap[MyGuid(false)].pExhibition)
		Doc()
			->m_roomSlotMap[MyGuid(false)]
			.pExhibition->GetPetFrame()
			->SetShirtsClr(pTids, m_createShirtColor);
	else
		pTids->m_shirtsColor = m_createShirtColor;
}

// 983 (00aba0c0): pede confirmacao antes de criar (C43.1/C45.2)
void CLobbyMain::OnCreate_CreateBtnUp()
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	std::string text = K2L_Compatibility(
		"\\c0xffff0000\\c* \xc7\xd1 \xb9\xf8 \xbc\xb1\xc5\xc3\xc7\xd1 \xc4\xb3\xb8\xaf\xc5\xcd\xb4\xc2 \xb3\xaa\xc1\xdf\xbf\xa1 \xb9\xd9\xb2\xdc \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.\\c0xff000000\\c");
	text += "\n";
	text += K2L_Compatibility("  \xbd\xc5\xc1\xdf\xc8\xf7 \xbc\xb1\xc5\xc3\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.");
	pForm->SetMessage(text.c_str(), false);
	pForm->Open((FRESH_PFN_RESULT)&CLobbyMain::OnCreateConfirmResult983, 1);
}

// 983 (00ab6fc0): com "sim" manda o 0x8 com 6 bytes (personagem, cHairID do cabelo escolhido, camisa) ao login
bool CLobbyMain::OnCreateConfirmResult983(int result, FrForm* pForm)
{
	if (result == 1)
	{
#pragma pack(push, 1)
		struct
		{
			unsigned long charTid;
			unsigned char hair;
			unsigned char shirts;
		} info;
#pragma pack(pop)

		info.charTid = 0x4000000 | m_createCharType;
		info.hair = m_createHair983[m_createCharType][m_createHairColor].cHairID;
		info.shirts = m_createShirtColor;

		WSendPacket packet(8);
		packet.EncodeBuffer(&info, 6);
		packet.Send(TO_LOGIN);

		Doc()->m_bAutoRefresh = false;

		HandleMsg(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}
	return true;
}

// 983 (00ab8130): a caixa vai para a "desc_pos" do char_select_free.lua
void CLobbyMain::OnCreate_DescInit(int param)
{
	m_pCreateDesc = DYNAMIC_CAST(FrEdit, (FrWnd*)param);

	lua_system::lua_script* script =
		lua_system::lua_script_manger::GetInstance()->CreateScript(true);
	if (script && script->RunFile(g_charSelectLua983))
	{
		lua_system::lua_table_helper tbl = script->GetTable(g_charSelectTable983);
		float x = LUA983_POS(tbl, "desc_pos", 1);
		float y = LUA983_POS(tbl, "desc_pos", 2);
		WRect rect = m_pCreateDesc->GetRect();
		m_pCreateDesc->SetRect(WRect(x, y, rect.w, rect.h));
	}
}

// 983 (00ab8850): lista dos personagens (menos os 11..15, que sao as outras roupas de 0/1/3), posta no meio da tela
// na altura "character_select_listbox_pos" do Lua
void CLobbyMain::OnCreate_CharacterListInit983(int param)
{
	m_pCreateCharacterList983 = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (!m_pCreateCharacterList983)
		return;

	m_pCreateCharacterList983->SetVisible(true);
	m_pCreateCharacterList983->Resize(50, 50);

	const Bitmap* pBmp = NULL;
	std::map<unsigned int, IFF_STRUCT::sChar>::iterator it;
	for (it = ItemManager()->m_CharMap.begin(); it != ItemManager()->m_CharMap.end(); ++it)
	{
		unsigned int idx = it->second.c.TypeId & 0x3ffffff;
		if (idx < 11 || idx > 15)
		{
			m_pCreateCharacterList983->AddItem(&it->second);
			pBmp = g_pFresh->GetBitmap(it->second.c.Icon);
		}
	}
	if (pBmp)
		m_pCreateCharacterList983->SetItemSize(pBmp->Width(), pBmp->Height());

	WRect rect(0.0f, 0.0f, 0.0f, 0.0f);
	lua_system::lua_script* script =
		lua_system::lua_script_manger::GetInstance()->CreateScript(true);
	if (script && script->RunFile(g_charSelectLua983))
	{
		lua_system::lua_table_helper tbl = script->GetTable(g_charSelectTable983);
		rect.x += (float)((LuaHelper983&)tbl).GetChild983("character_select_listbox_pos").GetIntByIndex983(1);
		rect.y += (float)((LuaHelper983&)tbl).GetChild983("character_select_listbox_pos").GetIntByIndex983(2);
	}

	int n = m_pCreateCharacterList983->GetCurrentItemSize(true);
	rect.w = (float)(pBmp->Width() * n + n * 3 + 15.0);
	rect.h = (float)pBmp->Height();
	rect.x = (float)(int)(400.0 - rect.w * 0.5);
	m_pCreateCharacterList983->SetRect(rect);
	m_pCreateCharacterList983->Resize(pBmp->Width() + 3, pBmp->Height());
}

// 983 (00abc0e0): escolhe o personagem da linha e troca a descricao
void CLobbyMain::OnCreate_CharacterListDown983(int param)
{
	FrListItem* pItem = m_pCreateCharacterList983->GetSelected();
	if (!pItem)
		return;

	IFF_STRUCT::sChar* pChar = (IFF_STRUCT::sChar*)pItem->pData;
	SelectCreateChar983((unsigned char)pChar->c.TypeId);
	m_pCreateDesc->ClearLine();
	m_pCreateDesc->AddText(ItemManager()->FindDesc(pChar->c.TypeId)->Desc, false, true);
}

// 983 (00ab70b0): rosto (0/1/3 com o rosto de 11/12/14), moldura e o cursor da escolha / do mouse
void CLobbyMain::OnCreate_CharacterListOwnerDraw983(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem || !pGI)
		return;

	IFF_STRUCT::sChar* pChar = (IFF_STRUCT::sChar*)pItem->pData;
	if (!pChar)
		return;

	const Bitmap* pFace = NULL;
	unsigned long idx = pChar->c.TypeId & 0x3ffffff;
	if (idx != 2 && idx < 4)
	{
		IFF_STRUCT::sChar* pAlt = ItemManager()->FindChar((idx + 11) | 0x4000000);
		if (pAlt)
			pFace = g_pFresh->GetBitmap(pAlt->c.Icon);
	}
	else
	{
		pFace = g_pFresh->GetBitmap(pChar->c.Icon);
	}

	const Bitmap* pFrame = g_pFresh->GetBitmap("chselect_ch_frame");
	WRect rect(0.0f, 0.0f, 0.0f, 0.0f);
	if (pFace)
	{
		rect = WRect(pItem->pos.x, pItem->pos.y, (float)pFace->Width(), (float)pFace->Height());
		pGI->DrawTexture(pFace, rect, 0xffffffff, 0);
	}
	if (pFrame)
	{
		rect = WRect(pItem->pos.x, pItem->pos.y, (float)pFrame->Width(), (float)pFrame->Height());
		pGI->DrawTexture(pFrame, rect, 0xffffffff, 0);
	}

	rect.x -= 3.0;
	rect.y -= 3.0;

	const Bitmap* pCursor;
	if (pItem->idx == m_createCharType)
		pCursor = g_pFresh->GetBitmap("chselect_ch_cursor_Select");
	else if (pItem->underCursor)
		pCursor = g_pFresh->GetBitmap("chselect_ch_cursor_Hover");
	else
		return;

	if (pCursor)
	{
		rect.w = (float)pCursor->Width();
		rect.h = (float)pCursor->Height();
		pGI->DrawTexture(pCursor, rect, 0xffffffff, 0);
	}
}

// 983 (00ab4330): so o DYNAMIC_CAST (o resultado nao e guardado)
void CLobbyMain::OnCreate_DrawAreaInit983(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 983 (00ab4380): figura do cabelo escolhido na "hair_color_pos" e os 5 medidores com o valor ao lado
void CLobbyMain::OnCreate_DrawAreaOwnerDraw983(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;

	std::vector<IFF_STRUCT::sHairStyle>& hair = m_createHair983[m_createCharType];
	if (m_createHairColor < hair.size())
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(hair[m_createHairColor].c.Icon);
		if (pBmp)
			pGI->DrawTexture(pBmp, WRect(m_hairColorPos983.x, m_hairColorPos983.y,
				(float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	}

	for (int i = 0; i < 5; i++)
	{
		pGI->SetTextColor(0xff11375c, 0xffffffff);
		pGI->SetTextStyle(0);

		CGaugeBar983* pGauge = m_pStatGauge983[i];
		if (!pGauge)
			continue;

		pGauge->Draw();
		WPoint pos(pGauge->m_x, pGauge->m_y);
		pos.x += pGauge->m_w + 5.0;
		pos.y -= 2.0;
		g_pFresh->GetManager()->PrintText(pos, 0, MakeStr("%d", (int)pGauge->m_target), -1.0f, 0xffffffff);
	}
}

// 983 (00ab82a0): a figura do nome vai para a "character_name_pos" do Lua
void CLobbyMain::OnCreate_CharNameImageInit983(int param)
{
	m_pCreateCharNameImage983 = DYNAMIC_CAST(FrArea, (FrWnd*)param);

	lua_system::lua_script* script =
		lua_system::lua_script_manger::GetInstance()->CreateScript(true);
	if (script && script->RunFile(g_charSelectLua983))
	{
		lua_system::lua_table_helper tbl = script->GetTable(g_charSelectTable983);
		WPoint pos(LUA983_POS(tbl, "character_name_pos", 1), LUA983_POS(tbl, "character_name_pos", 2));
		WRect rect = m_pCreateCharNameImage983->GetRect();
		rect.x = pos.x;
		rect.y = pos.y;
		m_pCreateCharNameImage983->SetRect(rect);
	}
}

void __fastcall Fn0b304b0_983(unsigned long uid, sCharacterInfo* pInfo); // 00b304b0 (corpo NAO lido; frente B)

// 983 (00abbb90): troca o personagem da tela CREATE. 0/1/3 viram 11/12/14 (as versoes do JP); o cabelo e o 1o com
// cHairID 0 no vetor do personagem; figura "chselect_txt_name_%d"; boneco novo (CExhibitionByHardwareTnL983, flag 0x1f)
// no meu lugar; medidores com o PCL do Character.iff
void CLobbyMain::SelectCreateChar983(unsigned char index)
{
	if (m_createCharType >= 15)
		return;

	if (index != 2 && index < 4)
		index += 11;
	m_createCharType = index;
	m_createHairColor = 0;

	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(0x4000000 | m_createCharType);

	for (unsigned int i = 0; i < m_createHair983[m_createCharType].size(); i++)
	{
		if (m_createHair983[m_createCharType][i].cHairID == 0)
		{
			m_createHairColor = (unsigned char)i;
			break;
		}
	}

	if (m_pCreateCharNameImage983)
	{
		// D758: 983 00abbc80 = buffer de 0x20 em ebp-0x30 (cookie em ebp-0x10); "chselect_txt_name_%d" tem 20 B e o
		// name[16] do port estourava 4 B na pilha a cada troca de personagem na tela CREATE.
		char name[32];
		sprintf(name, "chselect_txt_name_%d", m_createCharType + 1);
		m_pCreateCharNameImage983->SetBgImg(name);
	}

	SetFirstChar(&Doc()->GetMyPartTidList());

	sRoomSlot slot;
	slot.pArea = m_pCreatePetArea983;
	slot.dwUserUID = 0;
	slot.pExhibition = NULL;
	slot.partTidList = Doc()->GetMyPartTidList();
	slot.bAngelWing = 0;
	slot.bGachaWing = 0;
	slot.charInfo.tid = slot.partTidList.m_charTid;
	slot.charInfo.hairClr = slot.partTidList.m_hairColor;
	slot.charInfo.shirtsClr = slot.partTidList.m_shirtsColor;
	slot.charInfo.gift_flag = 1;
	memcpy(slot.charInfo.tidParts, slot.partTidList.m_tid, sizeof(slot.charInfo.tidParts));

	std::map<unsigned long, sRoomSlot>::iterator it = Doc()->m_roomSlotMap.find(MyGuid(false));
	if (it != Doc()->m_roomSlotMap.end() && it->second.pExhibition)
	{
		delete (CExhibition983*)it->second.pExhibition; // 983: destrutor virtual (o lugar guarda o boneco do 983)
		it->second.pExhibition = NULL;
	}
	Doc()->m_roomSlotMap[MyGuid(false)] = slot;

	if (slot.pExhibition)
	{
		delete (CExhibition983*)slot.pExhibition;
		slot.pExhibition = NULL;
	}

	SetLevelBar();

	if (!g_pFresh->GetManager()->IsValidWindow(slot.pArea))
		return;

	slot.pArea->SetMouseEvent(true);

	if (!slot.partTidList.SetTids(&slot.charInfo, 0xff) || !slot.partTidList.IsComboValid())
	{
		slot.partTidList.m_charTid = slot.charInfo.tid;
		slot.partTidList.SetDefaultTids();
	}
	Doc()->BuildMyPartTidList(slot.charInfo);

	WRect rect = slot.pArea->GetRect();
	Fn0b304b0_983(slot.dwUserUID, &slot.charInfo);

	// o lugar ainda tem o tipo do 645 (shareddoc.h); o boneco e o do 983
	slot.pExhibition = (CExhibition*)new CExhibitionByHardwareTnL983(
		(float)(rect.x + rect.w * 0.5), (float)(0.5 * rect.h + rect.y), 0.35f, slot.dwUserUID, false);
	((CExhibition983*)slot.pExhibition)->SetDistanceRange(10.0f, 14.0f);
	((CExhibition983*)slot.pExhibition)->SetDistance(12.0f, true, false);
	((CExhibition983*)slot.pExhibition)->SetArea(rect.w, rect.h);
	((CExhibition983*)slot.pExhibition)->SetModel(slot.partTidList, slot.charInfo.tidAuxParts, NULL, 3.0f, MyGuid(false));
	((CExhibition983*)slot.pExhibition)->m_flags = 0x1f;
	Doc()->m_roomSlotMap[MyGuid(false)] = slot;

	for (int i = 0; i < 5; i++)
	{
		m_pStatGauge983[i]->Reset();
		m_pStatGauge983[i]->SetValue((float)pChar->PCL[i]);
	}
}

// 983 (00ab2fa0): resultado da "notify_tiki" (HandleMsg 0x8b caso 0xc); 1 e 4 fecham a janela da sala
bool CLobbyMain::OnUseTikiReportResult983(int result, FrForm* pForm)
{
	m_pUseTikiReportDlg983 = NULL;
	if (result == 1)
	{
		if (m_pGameRoomDlg983)
			m_pGameRoomDlg983->Close((eFormRet)1, false);
	}
	else if (result == 4)
	{
		if (m_pGameRoomDlg983)
			m_pGameRoomDlg983->Close((eFormRet)0, false);
	}
	return true;
}

void CLobbyMain::OnCreate_WarningInit(int param)
{
	FrEdit* pEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);

	pEdit->AddText(
		K2L_Compatibility("\\c0xffff0000\\c* \xc7\xd1 \xb9\xf8 \xbc\xb1\xc5\xc3\xc7\xd1 \xc4\xb3\xb8\xaf\xc5\xcd\xb4\xc2 \xb3\xaa\xc1\xdf\xbf\xa1 \xb9\xd9\xb2\xdc \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.\\c0xff000000\\c"),
		false, true);
	pEdit->AddText(
		K2L_Compatibility("  \xbd\xc5\xc1\xdf\xc8\xf7 \xbc\xb1\xc5\xc3\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."),
		false, true);
}

void CLobbyMain::On_PowerBarInit(int param)
{
	m_pStatBar[0] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pStatBar[0]->SetRange(0, 50, 0);
}

void CLobbyMain::On_ControlBarInit(int param)
{
	m_pStatBar[1] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pStatBar[1]->SetRange(0, 30, 0);
}

void CLobbyMain::On_AccuracyBarInit(int param)
{
	m_pStatBar[2] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pStatBar[2]->SetRange(0, 30, 0);
}

void CLobbyMain::On_SpinBarInit(int param)
{
	m_pStatBar[3] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pStatBar[3]->SetRange(0, 30, 0);
}

void CLobbyMain::On_CurveBarInit(int param)
{
	m_pStatBar[4] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pStatBar[4]->SetRange(0, 30, 0);
}

bool CLobbyMain::OnCreateNickResult(int result, FrForm* form)
{
	m_pCreateNickDlg = NULL;

	if (result == 0)
	{
	}
	else if (result == 1)
	{
		if (IsLocalContent(S4_NT_NICKNAME_CHANGE) && m_pServerDlg)
			m_pServerDlg->ClickDisableList(true);
	}

	return true;
}

bool CLobbyMain::OnMissionEventResult(int result, FrForm* form)
{
	if (!IsLocalContent(S4_NT_EVENT_MISSION))
		return false;

	m_pMissionEventDlg = NULL;

	return true;
}

bool CLobbyMain::OnBingoEventResult(int result, FrForm* form)
{
	if (!IsLocalContent(S4_NT_EVENT_BINGO))
		return false;

	m_pBingoEventDlg = NULL;

	return true;
}

bool CLobbyMain::OnMatchingDlgResult(int result, FrForm* form)
{
	if (!IsLocalContent(S4_MATCHING_SYSTEM))
		return false;

	if (m_pInviteDlg == NULL)
		return false;

	bool bAccept = false;

	switch (result)
	{
	case 1:
		bAccept = true;
		break;

	case 2:
		break;

	default:
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xb8\xc5\xc4\xaa\xc0\xcc \xc3\xeb\xbc\xd2\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		break;
	}

	WSendPacket packet((enumClientPacket)0xc0);
	packet.Encode4(m_pInviteDlg->GetFromUID());
	packet.Encode1(bAccept);
	packet.Send(TO_GAME);

	m_pInviteDlg = NULL;

	return true;
}

void CLobbyMain::RefreshAllUccClothes()
{
	if (CUserInfo::Instance()->GetDlg())
		return;

	sUccRefreshItem item;

	for (int i = 0; i < 10; i++)
	{
		if (!UccManager()->GetRefreshItem(item))
			break;
	}
}

void CLobbyMain::OnToppage_UCCShopBtnUp()
{
	// 983 00594950: loja como widget do lobby (0x88 so do 983), sem a CShopTask nem a aba 0xc6 do 645
	HandleMsg(MsgObject(NULL, 0x88 /*so do 983*/, (int)"CShopMain", (int)"TOPPAGE", 0, 0, 0));
}

void CLobbyMain::OnToppage_CardShopBtnUp()
{
	// 983 00594990: loja como widget do lobby (0x88 so do 983), sem a CShopTask nem a aba 0xc6 do 645
	HandleMsg(MsgObject(NULL, 0x88 /*so do 983*/, (int)"CShopMain", (int)"TOPPAGE", 0, 0, 0));
}

float CLobbyMain::GetTopPageBtnRect()
{
	if (IsLocalContent(S4_TOPPAGE_BTN_LINEUP))
	{
		float x;
		if (CLoginInfo::Instance()->IsPcBang())
			x = (float)m_topBtnIndex * 80.0f;
		else
			x = (float)m_topBtnIndex * 80.0f;
		m_topBtnIndex++;
		return x;
	}

	return 0.0f;
}

void CLobbyMain::OnToppage_TopIcon1Init(int param)
{
	CIconManager::Instance()->InitalizeTopIcon(0, param);
}

void CLobbyMain::OnToppage_TopIcon1Up()
{
	CloseAllDialogs();
	CIconManager::Instance()->OnButtonDown(0);
}

void CLobbyMain::OnToppage_TopIcon2Init(int param)
{
	CIconManager::Instance()->InitalizeTopIcon(1, param);
}

void CLobbyMain::OnToppage_TopIcon2Up()
{
	CloseAllDialogs();
	CIconManager::Instance()->OnButtonDown(1);
}

void CLobbyMain::OnToppage_TopIcon3Init(int param)
{
	CIconManager::Instance()->InitalizeTopIcon(2, param);
}

void CLobbyMain::OnToppage_TopIcon3Up()
{
	CloseAllDialogs();
	CIconManager::Instance()->OnButtonDown(2);
}

void CLobbyMain::OnToppage_TopIcon4Init(int param)
{
	CIconManager::Instance()->InitalizeTopIcon(3, param);
}

void CLobbyMain::OnToppage_TopIcon4Up()
{
	CloseAllDialogs();
	CIconManager::Instance()->OnButtonDown(3);
}

void CLobbyMain::OnToppage_TopIcon5Init(int param)
{
	CIconManager::Instance()->InitalizeTopIcon(4, param);
}

void CLobbyMain::OnToppage_TopIcon5Up()
{
	CloseAllDialogs();
	CIconManager::Instance()->OnButtonDown(4);
}

void CLobbyMain::OnToppage_TopIcon6Init(int param)
{
	CIconManager::Instance()->InitalizeTopIcon(5, param);
}

void CLobbyMain::OnToppage_TopIcon6Up()
{
	CloseAllDialogs();
	CIconManager::Instance()->OnButtonDown(5);
}
