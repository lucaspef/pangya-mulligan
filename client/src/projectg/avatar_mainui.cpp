#define REBANG_AVATAR_MAINUI_CPP
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "frarea.h"
#include "frbutton.h"
#include "fredit.h"
#include "frframe.h"
#include "frcombobox.h"
#include "frscrollbar.h"
#include "frcontextmenuctrl.h"
#include "wresrcmng.h"
#include "wview.h"
#include "../shared/localize.h"
#include "packet.h"
#include "commonutil.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "quitwindow.h"
#include "taskmanager.h"
#include "chatmsg.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "user_info.h"
#include "rankingdlg.h"
#include "messengerdlg.h"
#include "tradedlg.h"

// HACK: in the original this is an inline of shareddoc.h (PCH); the unit emits its own copy.
inline unsigned long CSharedDoc::GetTradeUID()
{
	if (0)
		CSharedDoc::Instance();
	return m_tradeUID;
}

// 983: o deslocamento vertical e recalculado em cada Init (altura da visao - 600 + y); a 645 guardava em m_offsetY
#define MOVE_OFFSET983(w, r) \
	do { \
		float oy_ = (float)(600.0 - (r).y); \
		float vh_ = g_view ? *(float*)((char*)g_view + VC10_OFS_BO(0x1a0)) : 600.0f; \
		(w)->MoveWindow(WPoint((r).x, vh_ - oy_)); \
	} while (0)

#include "avatar_avatar.h"
#include "jukeboxdlg.h"
#include "jp/lobbyjp.h" // CUiPointer983 (00e73e4c)
#include "jp/createcharacteroption.h" // LoadPortraits983 (004ce960)
#include "jp/avatarmotiondlg.h" // D641: lista das emocoes do Lounge (007b1860)

// D641: tabela de animacoes por personagem (CSharedDoc+0x10c24); visao da jp/avatarmotiondlg.cpp
class CLobbyMotionTable983
{
public:
	int Count983(const unsigned long* pTid); // 009a3c80
};
#define LOBBYMOTION983() ((CLobbyMotionTable983*)&CSharedDoc::Instance()->m_chatManager)
C_ASSERT_VC10(offsetof(CSharedDoc, m_chatManager) == 0x10c24); // A748: era cru Doc()+0x10c24
#include "soundmanager.h"

extern CSoundManager* g_audio;
#include "avatarchatlogdlg.h"
#include "avataruserlistdlg.h"
#include "avatartipdlg.h"
#include "notedlg.h"
#include "clientsetting.h"
#include "emoticondlg.h"
#include "reportdlg.h"
#include "gatewayactor.h"
#include "avatar_main.h"


#define STR_EVERYONE "\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"
#define STR_MENU_TRADE "1:1\xb0\xc5\xb7\xa1"
#define STR_MENU_VISIT_SHOP "\xbb\xf3\xc1\xa1\xb1\xb8\xb0\xe6"
#define STR_MENU_FOLLOW "\xb5\xfb\xb6\xf3\xb0\xa1\xb1\xe2"
#define STR_MENU_FRIEND "\xc4\xa3\xb1\xb8\xbf\xe4\xc3\xbb"
#define STR_MENU_WHISPER "\xb1\xd3\xbc\xd3\xb8\xbb"
#define STR_MENU_NOTE "\xc2\xca\xc1\xf6"
#define STR_MENU_INFO "\xc1\xa4\xba\xb8\xba\xb8\xb1\xe2"
#define STR_MENU_KICK "\xb0\xad\xc1\xa6\xc5\xf0\xc0\xe5"
#define STR_MENU_EDIT_SHOP "\xbb\xf3\xc1\xa1\xc6\xed\xc1\xfd"
#define STR_MENU_MAKE_SHOP "\xbb\xf3\xc1\xa1\xb8\xb8\xb5\xe9\xb1\xe2"
#define STR_MENU_ACHIEVEMENT "\xbe\xf7\xc0\xfb\xba\xb8\xb1\xe2" // 983 00d263e0 (PKT, frente C, lote 37)

// HACK: avatar_main.cpp (HandleMsg) and avatar_avatar.cpp (IsClickable) own these literals as statics of their
// naked functions; a second, genuine COMDAT from this unit would stay in the image, so they are borrowed here.
#define STR_BTN_JUKEBOX_N "btn_jukebox_n"
#define STR_BTN_JUKEBOX_O "btn_jukebox_o"
#define TRADE_RANGE 19.2f

extern Fresh* g_pFresh;

void __fastcall SetTradeResultCode(int code, int msg, sTradeItem* pItem);

BEGIN_FRESH_MSGMAP(ntAvatarChatMain, CTaskMain)
ON_FRESH_VI("AVATARCHAT_MAIN", FRCMD_INIT,
	ntAvatarChatMain::OnAvatarChatMainInit)
ON_FRESH_VV("AVATARCHAT_MAIN", FRCMD_FINISH,
	ntAvatarChatMain::OnAvatarChatMainFinish)
ON_FRESH_VI("AVATARCHAT_MAIN.portrait", FRCMD_INIT,
	ntAvatarChatMain::OnPortraitInit)
ON_FRESH_VI("AVATARCHAT_MAIN.portrait_mask_area", FRCMD_INIT,
	ntAvatarChatMain::OnPortraitMaskAreaInit)
ON_FRESH_VI("AVATARCHAT_MAIN.portrait_mask", FRCMD_INIT,
	ntAvatarChatMain::OnPortraitMaskInit)
ON_FRESH_VV("AVATARCHAT_MAIN.portrait_mask", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnPortraitMaskBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.portrait_tipbtn", FRCMD_INIT,
	ntAvatarChatMain::OnPortraitTipButtonInit)
ON_FRESH_VV("AVATARCHAT_MAIN.portrait_tipbtn", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnPortraitTipButtonBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.portrait_jukeboxbtn_area", FRCMD_INIT,
	ntAvatarChatMain::OnPortraitJukeBoxButtonAreaInit)
ON_FRESH_VI("AVATARCHAT_MAIN.portrait_jukeboxbtn", FRCMD_INIT,
	ntAvatarChatMain::OnPortraitJukeBoxButtonInit)
ON_FRESH_VV("AVATARCHAT_MAIN.portrait_jukeboxbtn", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnPortraitJukeBoxButtonBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.portrait_chatlogbtn", FRCMD_INIT,
	ntAvatarChatMain::OnPortraitChatLogButtonInit)
ON_FRESH_VV("AVATARCHAT_MAIN.portrait_chatlogbtn", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnPortraitChatLogButtonBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.userlistbtn", FRCMD_INIT,
	ntAvatarChatMain::OnAvatarUserListButtonInit)
ON_FRESH_VI("AVATARCHAT_MAIN.motion_btn_start", FRCMD_INIT,
	ntAvatarChatMain::OnMotionBtnInit983)
ON_FRESH_VV("AVATARCHAT_MAIN.motion_btn_start", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnMotionBtnUp983) // D641: 983 00e19b78 -> 007b1950
ON_FRESH_VV("AVATARCHAT_MAIN.userlistbtn", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnAvatarUserListButtonBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.chatframe", FRCMD_INIT,
	ntAvatarChatMain::OnChatFrameInit)
ON_FRESH_VI("AVATARCHAT_MAIN.chatview", FRCMD_INIT,
	ntAvatarChatMain::OnChatViewInit)
ON_FRESH_VI("AVATARCHAT_MAIN.emoticon", FRCMD_INIT,
	ntAvatarChatMain::OnEmoticonInit)
ON_FRESH_VV("AVATARCHAT_MAIN.emoticon", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnEmoticonBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.language", FRCMD_INIT,
	ntAvatarChatMain::OnLanguageInit)
ON_FRESH_VI("AVATARCHAT_MAIN.report", FRCMD_INIT,
	ntAvatarChatMain::OnReportInit)
ON_FRESH_VV("AVATARCHAT_MAIN.report", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnReportBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.chattarget", FRCMD_INIT,
	ntAvatarChatMain::OnChatTargetInit)
ON_FRESH_VI("AVATARCHAT_MAIN.chattarget", FRCMD_LBUTTONDOWN,
	ntAvatarChatMain::OnChatTargetBtnDown)
ON_FRESH_BI("AVATARCHAT_MAIN.chattarget", FRCMD_ENTERKEY,
	ntAvatarChatMain::OnChatTargetEnterKey)
ON_FRESH_VI("AVATARCHAT_MAIN.chatinput", FRCMD_INIT,
	ntAvatarChatMain::OnChatInputInit)
ON_FRESH_BI("AVATARCHAT_MAIN.chatinput", FRCMD_ENTERKEY,
	ntAvatarChatMain::OnChatInputEnterKey)
ON_FRESH_VI("AVATARCHAT_MAIN.rmenu", FRCMD_INIT,
	ntAvatarChatMain::OnRightMenuInit)
ON_FRESH_VV("AVATARCHAT_MAIN.rmenu", FRCMD_LBUTTONDOWN,
	ntAvatarChatMain::OnRightMenuLBtnDown)
ON_FRESH_VV("AVATARCHAT_MAIN.rmenu", FRCMD_RBUTTONDOWN,
	ntAvatarChatMain::OnRightMenuRBtnDown)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar", FRCMD_INIT, CTaskMain::OnUnderBarInit)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.back", FRCMD_INIT,
	CTaskMain::OnUnderBar_BackInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.back", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_BackUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.back", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.back", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.front", FRCMD_INIT,
	CTaskMain::OnUnderBar_FrontInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.front", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_FrontUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.front", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.front", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.shop", FRCMD_INIT,
	CTaskMain::OnUnderBar_ShopInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.shop", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ShopUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.shop", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.myroom", FRCMD_INIT,
	CTaskMain::OnUnderBar_MyRoomInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.myroom", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MyRoomUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.myroom", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.myroom", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.itemstorage", FRCMD_INIT,
	CTaskMain::OnUnderBar_ItemStorageInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.itemstorage", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ItemStorageUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.itemstorage", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.itemstorage", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.gift", FRCMD_INIT,
	CTaskMain::OnUnderBar_GiftInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.gift", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GiftUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.gift", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.gift", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.cookie", FRCMD_INIT,
	CTaskMain::OnUnderBar_CookieInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.cookie", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_CookieUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.cookie", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.cookie", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.game", FRCMD_INIT,
	CTaskMain::OnUnderBar_GameInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.game", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GameUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.game", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.game", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.server", FRCMD_INIT,
	CTaskMain::OnUnderBar_ServerInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.server", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ServerUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.server", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.server", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.exit", FRCMD_INIT,
	CTaskMain::OnUnderBar_ExitInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.exit", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ExitUp)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.exit", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.exit", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.ubar.tabbtn", FRCMD_INIT,
	CTaskMain::OnUnderBar_TabBtnInit)
ON_FRESH_VV("AVATARCHAT_MAIN.ubar.tabbtn", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TabBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.magicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_MagicBoxInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.magicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MagicBoxUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.magicbox", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.magicbox", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.tikimagicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_TikiMagicBoxInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.tikimagicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TikiMagicBoxUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.tikimagicbox", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.tikimagicbox", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.scratch", FRCMD_INIT,
	CTaskMain::OnUnderBar_ScratchInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.scratch", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ScratchUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.scratch", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.scratch", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.bongdarishop", FRCMD_INIT,
	CTaskMain::OnUnderBar_BongdariShopInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.bongdarishop", FRCMD_LBUTTONDOWN,
	CTaskMain::OnUnderBar_BongdariShopUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.bongdarishop", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.bongdarishop", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.maket", FRCMD_INIT,
	CTaskMain::OnUnderBar_MaketInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.maket", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MaketUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.maket", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.maket", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.ranking", FRCMD_INIT,
	CTaskMain::OnUnderBar_RankingInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.ranking", FRCMD_LBUTTONUP,
	ntAvatarChatMain::OnUnderBar_RankingUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.ranking", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.ranking", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.guild", FRCMD_INIT,
	CTaskMain::OnUnderBar_GuildInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.guild", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GuildUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.guild", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.guild", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.notice", FRCMD_INIT,
	CTaskMain::OnUnderBar_NoticeInit)
ON_FRESH_VV("AVATARCHAT_MAIN.utab.notice", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_NoticeUp)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.notice", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.utab.notice", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.title", FRCMD_INIT,
	CTaskMain::OnOverBar_TitleInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar", FRCMD_INIT, CTaskMain::OnOverBar_Init)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.bar1", FRCMD_INIT,
	CTaskMain::OnOverBar_Bar1Init)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.bar2", FRCMD_INIT,
	CTaskMain::OnOverBar_Bar2Init)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.bar3", FRCMD_INIT,
	CTaskMain::OnOverBar_Bar3Init)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.frame_1", FRCMD_INIT,
	CTaskMain::OnOverBar_Frame1Init)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.frame_2", FRCMD_INIT,
	CTaskMain::OnOverBar_Frame2Init)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.frame_3", FRCMD_INIT,
	CTaskMain::OnOverBar_Frame3Init)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.site", FRCMD_INIT,
	CTaskMain::OnOverBar_SiteInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.char", FRCMD_INIT,
	CTaskMain::OnOverBar_CharInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.char", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CharDown)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.char", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.caddie", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.caddie", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CaddieDown)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.caddie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.club", FRCMD_INIT,
	CTaskMain::OnOverBar_ClubInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.club", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_ClubDown)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.club", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.aztec", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.aztec", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_AztecDown)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.aztec", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.mascot", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.mascot", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MascotDown)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.mascot", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.char_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CharSelInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.char_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharSelOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.caddie_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieSelInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.caddie_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieSelOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.club_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_ClubSelInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.club_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubSelOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.aztec_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecSelInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.aztec_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecSelOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.mascot_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotSelInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.mascot_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotSelOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.pang_cookie", FRCMD_INIT,
	CTaskMain::OnOverBar_PangCookieInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.pang_cookie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_PangCookieOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.onelinemsg_req", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineReqBtnInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.onelinemsg_req", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OneLineReqBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.onelineview", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineViewInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.onelineview", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_OneLineViewOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.spcard", FRCMD_INIT,
	CTaskMain::OnOverBar_SpecialCardInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.spcard", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_SpecialCardUp)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.spcard", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.spcard", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.messenger", FRCMD_INIT,
	CTaskMain::OnOverBar_MessengerInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.messenger", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MessengerUp)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.messenger", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.messenger", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.myinfo", FRCMD_INIT,
	CTaskMain::OnOverBar_MyInfoInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.myinfo", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MyInfoUp)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.myinfo", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.myinfo", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.option", FRCMD_INIT,
	CTaskMain::OnOverBar_OptionInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.option", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OptionUp)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.option", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.option", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.messenger_bubble", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.messenger_bubble", FRCMD_OWNERDRAW,
	CTaskMain::OnMessengerBubbleOwnerDraw)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.messenger_desc", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleEditInit)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.messenger_alarm", FRCMD_INIT,
	CTaskMain::OnMessengerAlarmBtnInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.messenger_alarm", FRCMD_LBUTTONUP,
	CTaskMain::OnMessengerAlarmBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.gift_alarm", FRCMD_INIT,
	CTaskMain::OnMessengerGiftAlarmBtnInit)
ON_FRESH_VV("AVATARCHAT_MAIN.obar.gift_alarm", FRCMD_LBUTTONUP,
	CTaskMain::OnMessengerGiftAlarmBtnUp)
ON_FRESH_VI("AVATARCHAT_MAIN.obar.tooltip", FRCMD_OWNERDRAW,
	CTaskMain::OnBarTooltipOwnerDraw)
END_FRESH_MSGMAP()

void ntAvatarChatMain::InitUI()
{
	// 983 007afe80: os 15 retratos "?_def.tga" saem do CreateCharacterOption (004ce960)
	ResetFrWndPointers();

	m_uiHideMode = 0;
	CreateCharacterOption::Instance()->LoadPortraits983(m_pPortraitBmp);
}

void ntAvatarChatMain::ResetFrWndPointers()
{
	(void)&g_HUGE;
	(void)&g_EPSILON;
	m_pJukeBoxBtn = NULL;
	m_pPortrait = NULL;
	m_pPortraitMask = NULL;
	m_pChatView = NULL;
	m_pChatFrame = NULL;
	m_pChatTarget = NULL;
	m_pChatInput = NULL;
	m_pEmoticonBtn = NULL;
	m_pLanguage = NULL;
	m_pReportBtn = NULL;
	m_pEmoticonDlg = NULL;
	m_pReportDlg = NULL;
	m_pNoteDlg = NULL;
	m_pAddFriendDlg = NULL;
	m_pDlg49c = NULL;
	m_pPasswordDlg = NULL;
	m_pTradeDlg = NULL;
	m_pJukeBoxDlg = NULL;
	m_pAvatarTipDlg = NULL;
	m_pUserListDlg = NULL;
	m_pChatLogDlg = NULL;
	m_pUserListBtn = NULL;
	m_pTipBtn = NULL;
	m_pChatLogBtn = NULL;
	m_pDlg4c8 = NULL;
	m_pWnd4c4 = NULL;
	m_pRightMenu = NULL;
}

void ntAvatarChatMain::OnAvatarChatMainInit(int param)
{
}

void ntAvatarChatMain::OnAvatarChatMainFinish()
{
	if (m_pOverBarTitle)
		m_pOverBarTitle->SetBgImg("title_chat");
	C_ASSERT_VC10(offsetof(CSharedDoc, m_bRefreshCamera) == 0x10d80); // B1470: era cru +0x10d80 (anda no cl 14.44)
	if (CSharedDoc::Instance()->m_bRefreshCamera)
		this << MsgObject(NULL, M983(0xaa), 1, 0, 0, 0, 0);
}

void ntAvatarChatMain::OnPortraitInit(int param)
{
	m_pPortrait = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pPortrait)
	{
		WRect rect = m_pPortrait->GetRect();
		MOVE_OFFSET983(m_pPortrait, rect);
	}

	if (ntAvatarManager::Instance()->m_pMyAvatar)
	{
		int index =
			ntAvatarManager::Instance()->m_pMyAvatar->m_charTypeId & 0x3ffffff;
		if (index >= 0 &&
			index < sizeof(m_pPortraitBmp) / sizeof(m_pPortraitBmp[0]) &&
			m_pPortrait)
			m_pPortrait->SetBgImg(m_pPortraitBmp[index]);
	}
}

void ntAvatarChatMain::OnPortraitMaskAreaInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
	{
		pArea->SetVisible(true);
		WRect rect = pArea->GetRect();
		MOVE_OFFSET983(pArea, rect);
	}
}

void ntAvatarChatMain::OnPortraitMaskInit(int param)
{
	m_pPortraitMask = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pPortraitMask)
	{
		m_pPortraitMask->SetVisible(true);
		WRect rect = m_pPortraitMask->GetRect();
		MOVE_OFFSET983(m_pPortraitMask, rect);
	}
}

void ntAvatarChatMain::OnPortraitMaskBtnUp()
{
	if (CChatMsg::Instance()->IsClosed())
		CChatMsg::Instance()->Open(true, true);
	else
		CChatMsg::Instance()->Close(true, true);
}

void ntAvatarChatMain::OnPortraitTipButtonInit(int param)
{
	m_pTipBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pTipBtn)
	{
		WRect rect = m_pTipBtn->GetRect();
		MOVE_OFFSET983(m_pTipBtn, rect);

		if (COption::Instance()->gGetAvatarNewBie())
			OnPortraitTipButtonBtnUp();
	}
}

bool ntAvatarChatMain::OnAvatarTipDlgResult(int result, FrForm* pForm)
{
	if (m_pTipBtn)
		m_pTipBtn->SetStatus(FrButton::NORMAL);
	m_pAvatarTipDlg = NULL;
	return true;
}

void ntAvatarChatMain::OnPortraitTipButtonBtnUp()
{
	if (m_pAvatarTipDlg)
	{
		m_pAvatarTipDlg->Close(true);
		return;
	}

	if (m_pTipBtn)
	{
		m_pTipBtn->SetStatus(FrButton::PRESSED);

		if (CChatMsg::Instance()->IsOpened())
			CChatMsg::Instance()->Close(true, true);

		m_pAvatarTipDlg = CreateForm<FrAvatarTipDlg>(g_pFresh->GetManager(),
			this, "avatartip", NULL);
		m_pAvatarTipDlg->EnableDrag(false);
		m_pAvatarTipDlg->Open(
			(FRESH_PFN_RESULT)&ntAvatarChatMain::OnAvatarTipDlgResult,
			WPoint(10.0f, 65.0f), 0x15);
		m_pAvatarTipDlg->SetIconRect(m_pTipBtn->GetRect());
	}
}

void ntAvatarChatMain::OnPortraitJukeBoxButtonAreaInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
	{
		pArea->SetVisible(true);
		WRect rect = pArea->GetRect();
		MOVE_OFFSET983(pArea, rect);
	}
}

void ntAvatarChatMain::OnPortraitJukeBoxButtonInit(int param)
{
	C_ASSERT_VC10(offsetof(ntAvatarChatMain, m_pJukeBoxBtn) == 0x4b4); // D701: 983 007b02b8/007b02e5
	m_pJukeBoxBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pJukeBoxBtn)
	{
		WRect rect = m_pJukeBoxBtn->GetRect();
		MOVE_OFFSET983(m_pJukeBoxBtn, rect);
		m_pJukeBoxBtn->SetPushDelay(0.0f);

		// D701: 983 007b02d2 invertido em relacao ao lobby (00ab773f): tocando (byte 00e1a160) = "_n"
		m_pJukeBoxBtn->SetButtonImg(FrJukeBoxDlg::m_preserve_isPlaying
				? STR_BTN_JUKEBOX_N
				: STR_BTN_JUKEBOX_O,
			FrButton::NORMAL);
	}
}

void ntAvatarChatMain::OnPortraitJukeBoxButtonBtnUp()
{
	ToggleJukeBox();
}

bool ntAvatarChatMain::OnJukeBoxDlgResult(int result, FrForm* pForm)
{
	if (m_pJukeBoxBtn)
		m_pJukeBoxBtn->SetStatus(FrButton::NORMAL);
	m_pJukeBoxDlg = NULL;
	return true;
}

void ntAvatarChatMain::ToggleJukeBox()
{
	if (m_pJukeBoxDlg)
	{
		m_pJukeBoxDlg->Close(true);
		return;
	}

	if (m_pJukeBoxBtn)
	{
		// 983 007b14f0: sem SetIconRect; seleciona a faixa que toca (nome em g_audio+0x24), SetFixed(false)+foco
		m_pJukeBoxDlg = CreateForm<FrJukeBoxDlg>(g_pFresh->GetManager(), this,
			"jukeboxdlg", NULL);
		FrJukeBoxDlg::SelectBGMByName(g_audio->GetBGMName());
		m_pJukeBoxDlg->SetFixed(false);
		m_pJukeBoxDlg->SetViewFocus(true);
		m_pJukeBoxDlg->Open(
			(FRESH_PFN_RESULT)&ntAvatarChatMain::OnJukeBoxDlgResult, 5);
		m_pJukeBoxDlg->OnChangePlayPauseButton();
		CloseAllDialogs(); // 983 007b14f0: slot +0x3c = 007afcf0, nao OnProcess
	}
}

void ntAvatarChatMain::OnPortraitChatLogButtonInit(int param)
{
	m_pChatLogBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pChatLogBtn)
	{
		WRect rect = m_pChatLogBtn->GetRect();
		MOVE_OFFSET983(m_pChatLogBtn, rect);
		m_pChatLogBtn->SetPushDelay(0.0f);
	}
}

void ntAvatarChatMain::OnPortraitChatLogButtonBtnUp()
{
	ToggleAvatarChatLog();
}

bool ntAvatarChatMain::OnAvatarChatLogDlgResult(int result, FrForm* pForm)
{
	if (m_pChatLogBtn)
		m_pChatLogBtn->SetStatus(FrButton::NORMAL);
	m_pChatLogDlg = NULL;
	return true;
}

void ntAvatarChatMain::ToggleAvatarChatLog()
{
	if (m_pChatLogDlg)
	{
		m_pChatLogDlg->Close(true);
		return;
	}

	if (m_pChatLogBtn)
	{
		m_pChatLogDlg = CreateForm<FrAvatarChatLogDlg>(g_pFresh->GetManager(),
			this, "avatarchatlogdlg", NULL);
		m_pChatLogDlg->SetFixed(true);
		m_pChatLogDlg->Open(
			(FRESH_PFN_RESULT)&ntAvatarChatMain::OnAvatarChatLogDlgResult,
			0x15);
		m_pChatLogDlg->SetIconRect(m_pChatLogBtn->GetRect());

		if (!CChatMsg::Instance()->IsClosed())
			CChatMsg::Instance()->Close(true, true);
	}
}

void ntAvatarChatMain::OnAvatarUserListButtonInit(int param)
{
	m_pUserListBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUserListBtn)
	{
		WRect rect = m_pUserListBtn->GetRect();
		m_pUserListBtn->MoveWindow(WPoint(g_view->GetWidth() - 38.0f, rect.y));
		m_pUserListBtn->SetPushDelay(0.5f);
	}
}

// 983: o botao redondo de emocoes do Lounge ("motion_btn_start") e ancorado a direita (largura viva - (800 - x do recurso)); d495-1: ficava em 783
void ntAvatarChatMain::OnMotionBtnInit983(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!pBtn)
		return;
	m_pWnd4c4 = pBtn; // D641: 983 007b17bc (+0x4c4 = o botao)
	pBtn->MoveWindow(WPoint(g_view->GetWidth() - 48.0f, 105.0f)); // 983 007b17c2
	pBtn->SetPushDelay(0.0f); // 983 007b17f2: +0x104 = 0, +0x108 = 0.1
	// 983 007b180e: sem emocoes na tabela do personagem o botao fica desabilitado
	ntAvatar* me = ntAvatarManager::m_pInstance ? ntAvatarManager::m_pInstance->m_pMyAvatar : NULL;
	if (me)
	{
		unsigned long tid = me->GetCharacterInfo().tid & 0x3ffffff;
		if (!LOBBYMOTION983()->Count983(&tid))
			pBtn->Enable(false);
	}
}

// D641 (983 007b1950): o clique abre a lista das emocoes; com ela aberta, fecha deslizando
void ntAvatarChatMain::OnMotionBtnUp983()
{
	if (m_pDlg4c8)
		((FrAvatarMotionDlg*)m_pDlg4c8)->m_slide = 4;
	else
		OpenMotionDlg983();
}

// D794 (983 007ad0ff, fim do caso 0x48 do HandleMsg): trocou o personagem -> a lista de emocoes aberta e refeita com a tabela
// do novo personagem e fecha; o botao so fica habilitado se o personagem tem emocoes (a lista ficava com as do anterior)
void ntAvatarChatMain::RefreshMotionUi983()
{
	if (m_pDlg4c8)
	{
		unsigned long tid = ntAvatarManager::Instance()->m_pMyAvatar->GetCharacterInfo().tid & 0x3ffffff; // 983 [+0x489]
		FrAvatarMotionDlg* pDlg = (FrAvatarMotionDlg*)m_pDlg4c8;
		pDlg->Refresh983(&tid);
		pDlg->m_bPlayed = false; // +0x168
		pDlg->m_nFlags.Enable(FWF_VIEWFOCUS); // +0x1c |= 8
		pDlg->Close(true); // vtbl+8
	}
	if (m_pWnd4c4)
	{
		unsigned long tid = ntAvatarManager::Instance()->m_pMyAvatar->GetCharacterInfo().tid & 0x3ffffff;
		m_pWnd4c4->Enable(LOBBYMOTION983()->Count983(&tid) != 0); // vtbl+0x18
	}
}

// 983 007afb30 / 007afb70
void ntAvatarChatMain::SetMotionBtnImg983(bool on)
{
	FrButton* pBtn = (FrButton*)m_pWnd4c4;
	if (!pBtn)
		return;
	pBtn->SetButtonImg(on ? "motionchat_btn_on_Normal" : "motionchat_btn_off_Normal", FrButton::NORMAL);
	pBtn->SetButtonImg(on ? "motionchat_btn_on_Hover" : "motionchat_btn_off_Hover", FrButton::OVER);
	pBtn->SetButtonImg(on ? "motionchat_btn_on_Hover" : "motionchat_btn_off_Hover", FrButton::PRESSED);
}

// 983 007b1860: so com o meu boneco; janela "motiondlg" FLAG_CHAT em (largura - 80, 140)
void ntAvatarChatMain::OpenMotionDlg983()
{
	ntAvatar* me = ntAvatarManager::m_pInstance ? ntAvatarManager::m_pInstance->m_pMyAvatar : NULL;
	if (!me)
		return;
	SetMotionBtnImg983(true);
	WPoint pos((g_view ? g_view->GetWidth() : 800.0f) - 80.0f, 140.0f);
	FrAvatarMotionDlg* pDlg = CreateForm<FrAvatarMotionDlg>(g_pFresh->GetManager(), this, "motiondlg", NULL);
	m_pDlg4c8 = pDlg;
	pDlg->EnableDrag(false);
	pDlg->SetFixed(true);
	pDlg->m_flags = FrAvatarMotionDlg::FLAG_CHAT;
	unsigned long tid = me->GetCharacterInfo().tid & 0x3ffffff;
	pDlg->SetupMotions983(&tid);
	pDlg->Open((FRESH_PFN_RESULT)&ntAvatarChatMain::OnMotionDlgResult983, pos, 5);
}

// 983 007afbb0
bool ntAvatarChatMain::OnMotionDlgResult983(int result, FrForm* pForm)
{
	if (m_pWnd4c4)
		((FrButton*)m_pWnd4c4)->SetStatus(FrButton::NORMAL);
	m_pDlg4c8 = NULL;
	SetMotionBtnImg983(false);
	return true;
}

void ntAvatarChatMain::PlaceUserListBtn983(float w)
{
	if (!m_pUserListBtn)
		return;
	WRect r = m_pUserListBtn->GetRect();
	if (r.x != w - 38.0f)
		m_pUserListBtn->MoveWindow(WPoint(w - 38.0f, r.y));
}

void ntAvatarChatMain::OnAvatarUserListButtonBtnUp()
{
	ToggleAvatarUserList();
}

bool ntAvatarChatMain::OnAvatarUserListDlgResult(int result, FrForm* pForm)
{
	if (m_pUserListBtn)
		m_pUserListBtn->SetStatus(FrButton::NORMAL);
	m_pUserListDlg = NULL;
	return true;
}

void ntAvatarChatMain::ToggleAvatarUserList()
{
	if (m_pUserListDlg)
	{
		m_pUserListDlg->Close(true);
		return;
	}

	if (m_pUserListBtn)
	{
		m_pUserListDlg = CreateForm<FrAvatarUserListDlg>(g_pFresh->GetManager(),
			this, "avataruserlistdlg", NULL);

		WPoint pos(g_view->GetWidth() - 252.0f, 108.0f);

		m_pUserListDlg->EnableDrag(false);
		m_pUserListDlg->SetFixed(true);
		m_pUserListDlg->Open(
			(FRESH_PFN_RESULT)&ntAvatarChatMain::OnAvatarUserListDlgResult, pos,
			0x15);
		m_pUserListDlg->SetIconRect(m_pUserListBtn->GetRect());
		m_pUserListDlg->UpdateUserList();
		if (m_pDlg4c8)
			m_pDlg4c8->Close(true);
	}
}

void ntAvatarChatMain::OnChatFrameInit(int param)
{
	m_pChatFrame = DYNAMIC_CAST(FrFrame, (FrWnd*)param);
	if (m_pChatFrame)
	{
		m_pChatFrame->SetVisible(CChatMsg::Instance()->IsOpened());

		WRect rect = m_pChatFrame->GetRect();
		MOVE_OFFSET983(m_pChatFrame, rect);
	}
}

void ntAvatarChatMain::OnChatViewInit(int param)
{
	m_pChatView = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pChatView)
	{
		m_pChatView->SetVisible(true);
		m_pChatView->HidePrivacy(true);

		WRect rect = m_pChatView->GetRect();
		MOVE_OFFSET983(m_pChatView, rect);

		m_pChatView->GetScrollBar()->Resize(8, 1, true);
		m_pChatView->SetWheelEvent(false);
		m_chatViewLines = 8;

		this << MsgObject(NULL, M983(0x63), 0, 0, 0, 0, 0);
	}
}

void ntAvatarChatMain::OnChatTargetInit(int param)
{
	// ORDER: emitted one wave later in the original
	if (0)
		OnChatInputInit(0);
	m_pChatTarget = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
	if (m_pChatTarget)
	{
		m_pChatTarget->SetListDir983(FrComboBox::LISTDIR983_ABOVE);
		m_pChatTarget->SetVisible(CChatMsg::Instance()->IsOpened());
		CSharedDoc::Instance()->InitReservedChatList(m_pChatTarget,
			(eReservedChatPartner)2);

		WRect rect = m_pChatTarget->GetRect();
		MOVE_OFFSET983(m_pChatTarget, rect);
	}
}

void ntAvatarChatMain::OnChatTargetBtnDown(int param)
{
	if (m_pChatInput)
		m_pChatInput->SetKeyFocus(true);
}

bool ntAvatarChatMain::OnChatTargetEnterKey(int param)
{
	// ORDER: emitted one wave later in the original
	if (0)
		OnReportResult(0, NULL);
	if (m_pChatTarget && strlen(m_pChatTarget->GetLine(1, false)) == 0)
		m_pChatTarget->SetLine(1, K2L_Compatibility(STR_EVERYONE), 0, false, 0);

	if (m_pChatInput)
		m_pChatInput->SetKeyFocus(true);

	return false;
}

void ntAvatarChatMain::OnChatInputInit(int param)
{
	m_pChatInput = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pChatInput)
	{
		m_pChatInput->SetVisible(CChatMsg::Instance()->IsOpened());

		WRect rect = m_pChatInput->GetRect();
		MOVE_OFFSET983(m_pChatInput, rect);
	}
}

bool ntAvatarChatMain::OnChatInputEnterKey(int param)
{
	// ORDER: emitted one wave later in the original
	if (0)
		OnEmoticonInit(0);
	const char* text = (const char*)param;

	if (text == NULL || text[0] == '\0')
	{
		CChatMsg::Instance()->Close(true, true);
		return true;
	}

	// the chat window is hidden
	if (!CSharedDoc::Instance()->m_pGolfDoc->m_bShowChat)
	{
		CChatMsg::Instance()->AddChatMsg(
			K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xb4\xeb\xc8\xad\xc3\xa2\xc0\xbb \xbc\xfb\xb1\xe4 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9."),
			0xffff7878, true, false);
		return true;
	}

	if (!CSharedDoc::Instance()->UTIL_SendChatMessage(m_pChatTarget, text,
			false))
	{
		// wrong whisper partner, back to "everyone"
		m_pChatTarget->SetLine(1, K2L_Compatibility(STR_EVERYONE), 0, false, 0);
		CChatMsg::Instance()->AddChatMsg(
			K2L_Compatibility("\xb1\xd3\xb8\xbb \xbb\xf3\xb4\xeb\xb0\xa1 \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \"\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4\"\xb7\xce \xba\xaf\xb0\xe6\xc7\xd5\xb4\xcf\xb4\xd9."),
			0xffff7878, true, false);
	}

	if (!OnlinePlay())
	{
		std::string msg(
			ntAvatarManager::Instance()->m_pMyAvatar->GetNickName());
		msg += " : \b";
		msg += text;
		CChatMsg::Instance()->AddChatMsg(msg, 0xffffcc00, false, false);

		ntAvatarManager::Instance()->m_pMyAvatar->SetBalloonText(text);

		int face = CSharedDoc::Instance()->m_chatManager.GetFaceIndex(text);
		if (face != 5)
			ntAvatarManager::Instance()->m_pMyAvatar->SetChatFacial(face);
	}

	return true;
}

void ntAvatarChatMain::OnEmoticonInit(int param)
{
	m_pEmoticonBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pEmoticonBtn)
	{
		m_pEmoticonBtn->SetVisible(CChatMsg::Instance()->IsOpened());

		WRect rect = m_pEmoticonBtn->GetRect();
		MOVE_OFFSET983(m_pEmoticonBtn, rect);
	}
}

void ntAvatarChatMain::OnEmoticonBtnUp()
{
	if (m_pEmoticonDlg == NULL)
	{
		m_pEmoticonDlg = CreateForm<FrEmoticonDlg>(g_pFresh->GetManager(), this,
			"emoticon", NULL);
		if (m_pEmoticonDlg == NULL)
			return;

		WPoint pos(0.0f, 0.0f);
		if (m_pEmoticonBtn)
		{
			const WRect& rect = m_pEmoticonBtn->GetRect();
			pos.x = rect.x - 200.0f;
			pos.y = rect.y - 260.0f;
		}

		if (!g_pFresh->GetManager()->IsKeyExclusive())
			g_pFresh->GetManager()->SetExclusiveKey(true);

		m_pEmoticonDlg->Open(
			(FRESH_PFN_RESULT)&ntAvatarChatMain::OnEmoticonResult, pos, 3);
	}
	else
	{
		m_pEmoticonDlg->Close(FrCANCEL, true);
	}
}

bool ntAvatarChatMain::OnEmoticonResult(int result, FrForm* pForm)
{
	// ORDER: emitted one wave later in the original
	if (0)
		OnLanguageInit(0);
	if (!CChatMsg::Instance()->IsOpened())
		g_pFresh->GetManager()->SetExclusiveKey(false);

	if (m_pEmoticonDlg && result == FrOK)
	{
		FrEmoticonDlg* pDlg = DYNAMIC_CAST(FrEmoticonDlg, pForm);
		if (pDlg)
		{
			const char* emoticon = pDlg->GetSelectedIcon();
			if (emoticon)
			{
				if (m_pChatInput && m_pChatInput->IsVisible())
				{
					const char* front = m_pChatInput->GetEditText_Front();
					const char* comp = m_pChatInput->GetEditText_Comp();
					const char* end = m_pChatInput->GetEditText_End();

					if (CChatMsg::Instance()->GetMaskedFont()->GetTextWidthOld983(
							g_view,
							MakeStr("%s%s%s%s", front, comp, emoticon, end)) <
						m_pChatInput->GetWidthLimit())
					{
						m_pChatInput->SetLine(1,
							MakeStr("%s%s%s%s", front, comp, emoticon, end), 0,
							false, 0);
					}
					m_pChatInput->SetKeyFocus(true);
				}
				else
				{
					CSharedDoc::Instance()->m_chatManager.SendChatMessage(
						emoticon, NULL, false);

					if (!OnlinePlay())
					{
						std::string msg(CSharedDoc::Instance()
								->m_userInfo[CSharedDoc::Instance()
										->m_pGolfDoc->m_currentPlayer]
								.info.sNick);
						msg += " : \b";
						msg += emoticon;
						CChatMsg::Instance()->AddChatMsg(msg, 0xffffcc00, false,
							false);
					}
				}
			}
		}
	}

	m_pEmoticonDlg = NULL;
	return true;
}

void ntAvatarChatMain::OnLanguageInit(int param)
{
	m_pLanguage = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pLanguage)
	{
		m_pLanguage->SetVisible(CChatMsg::Instance()->IsOpened());

		if (g_ime->IsAlphaNumericMode())
			m_pLanguage->SetBgImg("chat_eng");
		else
			m_pLanguage->SetBgImg("chat_kor");

		WRect rect = m_pLanguage->GetRect();
		MOVE_OFFSET983(m_pLanguage, rect);
	}
}

void ntAvatarChatMain::OnReportInit(int param)
{
	m_pReportBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pReportBtn)
	{
		m_pReportBtn->SetVisible(CChatMsg::Instance()->IsOpened());

		WRect rect = m_pReportBtn->GetRect();
		MOVE_OFFSET983(m_pReportBtn, rect);
	}
}

void ntAvatarChatMain::OnReportBtnUp()
{
	m_pReportDlg =
		CreateForm<FrReportDlg>(g_pFresh->GetManager(), this, "report", NULL);

	std::string msg;
	msg =
		K2L_Compatibility("1. \xb4\xeb\xc8\xad\xc3\xa2\xc0\xc7 \xc1\xf6\xb3\xad \xb3\xbb\xbf\xeb\xb5\xb5 \xb8\xf0\xb5\xce \xbc\xad\xb9\xf6\xbf\xa1 \xb1\xe2\xb7\xcf\xb5\xcb\xb4\xcf\xb4\xd9.\n");
	msg +=
		K2L_Compatibility("2. \xc1\xb6\xc0\xdb, \xc7\xe3\xc0\xa7, \xb9\xdd\xba\xb9\xbd\xc5\xb0\xed\xb4\xc2 \xc3\xb3\xb9\xfa\xc0\xbb \xb9\xde\xbd\xc0\xb4\xcf\xb4\xd9.\n");
	msg +=
		K2L_Compatibility("3. \xbd\xc5\xb0\xed\xc7\xcf\xbd\xc5 \xb3\xbb\xbf\xeb\xc0\xba 48\xbd\xc3\xb0\xa3 \xc0\xcc\xb3\xbb\xbf\xa1 \xc3\xb3\xb8\xae\xb5\xcb\xb4\xcf\xb4\xd9.");

	m_pReportDlg->SetMessage(msg.c_str(), false);
	m_pReportDlg->Open((FRESH_PFN_RESULT)&ntAvatarChatMain::OnReportResult, 3);
}

bool ntAvatarChatMain::OnReportResult(int result, FrForm* pForm)
{
	m_pReportDlg = NULL;
	return true;
}

static ntAvatarChatMain* s_menuOwner = NULL;
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
void ntAvatarChatMain::OnContextMenuFreed(void* menu)
{
	if (s_menuOwner && s_menuOwner->m_pRightMenu == menu)
	{
		// A819: no 983 o rmenu vive com o Lounge; aqui e liberado e o menu do botao direito nunca mais abre (dono16 19:35)
		void* frames[12];
		typedef WORD (WINAPI* CaptureFn)(DWORD, DWORD, void**, DWORD*);
		CaptureFn capture = (CaptureFn)GetProcAddress(GetModuleHandleA("kernel32.dll"), "RtlCaptureStackBackTrace");
		int n = capture ? capture(0, 12, frames, NULL) : 0;
		char line[300];
		int len = _snprintf(line, 80, "A819 rmenu liberado %p |", menu);
		for (int i = 0; i < n && len > 0 && len < 260; i++)
			len += _snprintf(line + len, sizeof(line) - len, " %p", frames[i]);
		jp983_rastro("%s", line);
		s_menuOwner->m_pRightMenu = NULL;
	}
}

extern "C" void __cdecl jp983_ctxmenu_owner_gone(ntAvatarChatMain* owner) // D503
{
	if (s_menuOwner == owner)
		s_menuOwner = NULL;
}

extern "C" void __cdecl jp983_ctxmenu_freed(void* menu)
{
	ntAvatarChatMain::OnContextMenuFreed(menu);
}

void ntAvatarChatMain::OnRightMenuInit(int param)
{
	s_menuOwner = this;
	m_pRightMenu = DYNAMIC_CAST(FrContextMenuCtrl, (FrWnd*)param);
	jp983_rastro("A819 rmenu init %p wnd=%p", m_pRightMenu, (void*)param);
	if (m_pRightMenu)
	{
		m_pRightMenu->SetVisible(false);
		m_pRightMenu->SetFixed(true);
	}
}

void ntAvatarChatMain::OnNoteToFriend()
{
	if (CSharedDoc::Instance()->m_myInfo.stat.i64Pang < 10)
	{
		// "sending a note needs %d pang"
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			MakeStr(
				K2L_Compatibility("\xc2\xca\xc1\xf6\xb8\xa6 \xba\xb8\xb3\xbb\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad\xb4\xc2 %d\xc6\xce\xc0\xcc \xc7\xca\xbf\xe4\xc7\xd5\xb4\xcf\xb4\xd9."),
				10),
			false);
		pForm->SetTimeLimit(5.0f);
		pForm->SetFixed(true);

		WPoint pos((g_view->GetWidth() - pForm->GetRect().w) * 0.5f,
			(g_view->GetHeight() - pForm->GetRect().h) * 0.5f);
		pForm->Open(NULL, pos, 3);
		return;
	}

	ntAvatar* pAvatar =
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
	if (m_pNoteDlg == NULL && pAvatar)
	{
		m_pNoteDlg =
			CreateForm<FrNoteDlg>(g_pFresh->GetManager(), this, "note", NULL);
		m_pNoteDlg->SetNick(pAvatar->m_nickName, NULL);
		m_pNoteDlg->SetFixed(true);

		WPoint pos((g_view->GetWidth() - m_pNoteDlg->GetRect().w) * 0.5f,
			(g_view->GetHeight() - m_pNoteDlg->GetRect().h) * 0.5f);
		m_pNoteDlg->Open((FRESH_PFN_RESULT)&ntAvatarChatMain::OnNoteDlgResult,
			pos, 1);

		WRect rect = m_pNoteDlg->GetRect();
		m_pNoteDlg->Adjust(rect);
		m_pNoteDlg->MoveWindow(WPoint(rect.x, rect.y));
	}

	if (!g_pFresh->GetManager()->IsKeyExclusive())
		g_pFresh->GetManager()->SetExclusiveKey(true);
}

bool ntAvatarChatMain::OnNoteDlgResult(int result, FrForm* pForm)
{
	if (result == FrOK &&
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName))
	{
		WSendPacket packet((enumClientPacket)0x3c);
		packet.Encode2(0x111);

		packet.Encode4(m_targetUid);
		packet.EncodeStr(std::string(m_pNoteDlg->GetNote()));
		packet.Encode1(0);
		packet.Send(TO_GAME);
	}

	m_pNoteDlg = NULL;
	return true;
}

void ntAvatarChatMain::OnAddFriend()
{
	ntAvatar* pAvatar =
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
	if (pAvatar)
	{
		if (!CMessengerInfo::Instance()->IsVisible())
		{
			CMessengerInfo::Instance()->Open(true);
			if (CMessengerInfo::Instance()->IsVisible() == true)
			{
				FrMessengerDlg* pDlg =
					CMessengerInfo::Instance()->GetMessengerDlg();
				if (pDlg)
				{
					pDlg->OpenAddFriendNickDlg();
					CMessengerInfo::Instance()->SetTargetNick(
						pAvatar->m_nickName);
				}
			}
		}
	}

	if (!g_pFresh->GetManager()->IsKeyExclusive())
		g_pFresh->GetManager()->SetExclusiveKey(true);
}

bool ntAvatarChatMain::OnAddFriendDlgResult(int result, FrForm* pForm)
{
	m_pAddFriendDlg = NULL;
	return true;
}

void ntAvatarChatMain::OnAvatarInfo()
{
	ntAvatar* pAvatar =
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
	if (pAvatar)
	{
		if (CUserInfo::Instance())
		{
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				CSharedDoc::Instance()->m_briefUserInfoMap.find(
					pAvatar->m_guid);
			if (it != CSharedDoc::Instance()->m_briefUserInfoMap.end())
			{
				CUserInfo::Instance()->SetInfo(it->second.dwUid,
					pAvatar->m_guid, true, true, false, false,
					it->second.sNick);
			}
		}
	}
}

void ntAvatarChatMain::OnFollowAvatar()
{
	if (ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName) &&
		(ntAvatarManager::Instance()->m_pMyAvatar->m_state & 0x7e) == 0)
	{
		ntAvatarManager::Instance()->m_pMyAvatar->Follow(m_targetNickName);
	}
}

void ntAvatarChatMain::OnKickAvatar()
{
	ntAvatar* pAvatar =
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
	if (ntAvatarManager::Instance()->m_pMyAvatar->m_canKick && pAvatar)
	{
		WSendPacket packet((enumClientPacket)0x26);
		packet.Encode4(pAvatar->m_uid);
		packet.Send(TO_GAME);
	}
}

// 983 007b0d20 (pedido C58.2): para o boneco, pede as missoes do dia (0x151, como o lobby em 00593990) e manda a
// mensagem 1 a si mesmo
void ntAvatarChatMain::OnToppage_OneDayQuestDown983()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
		ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	WSendPacket packet((enumClientPacket)0x151);
	packet.Send(TO_GAME);
	this << MsgObject(this, M983(0x1), 0, 0, 0, 0, 0);
}

// PKT (frente C, lote 37): 983 007b1a80 - pede as conquistas do avatar do menu (pacote 0x157 com o UID, o mesmo campo do
// OnKickAvatar) e manda a mensagem 1 ao ator principal
void ntAvatarChatMain::OnAchievementAvatar983()
{
	ntAvatar* pAvatar =
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
	if (pAvatar == NULL)
		return;

	WSendPacket packet((enumClientPacket)0x157);
	packet.Encode4(pAvatar->m_uid);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

bool ntAvatarChatMain::OnTradeDlgResult(int result, FrForm* pForm)
{
	m_pTradeDlg = NULL;
	return true;
}

void ntAvatarChatMain::OnOpenTradeShop()
{
	if (CSharedDoc::Instance()->IsControlServerService(0x20))
	{
		// "the personal shop is under maintenance"
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		return;
	}

	// 983 (007b1b50): sem a consulta de venda ao gateway (0x26d 0x10/0x11)
	ntAvatarManager::Instance()->m_pMyAvatar->Stop();

	if (CSharedDoc::Instance()->m_myInfo.stat.Level == 0)
	{
		SetTradeResultCode(0x19, 0xe3, NULL); // 983 (007b1bda)
		return;
	}

	switch (CSharedDoc::Instance()->m_stateTrade)
	{
	case 0:
		CSharedDoc::Instance()->SendTradeEditShop();
		break;
	case 1:
	case 3:
	case 4:
		if (m_pTradeDlg == NULL)
		{
			m_pTradeDlg = CreateForm<FrTradeDlg>(g_pFresh->GetManager(), this,
				"tradedlg", NULL);
			if (m_pTradeDlg)
			{
				// 983 (007b1b50, s84): sem a consulta de venda em pacote ao gateway; abre com 0xc1
				m_pTradeDlg->Open(
					(FRESH_PFN_RESULT)&ntAvatarChatMain::OnTradeDlgResult,
					0xc1);
				m_pTradeDlg->OpenTradeShop(
					CSharedDoc::Instance()->GetTradeUID());
				m_pTradeDlg->SetFadeout(false);
			}
			CSharedDoc::Instance()->SendTradeShowVisitor();
			CSharedDoc::Instance()->SendTradeIncome();
		}
		break;
	case 2:
		if (!CSharedDoc::Instance()->SendTradeEditShop())
			SetTradeResultCode(0, 0xe3, NULL); // 983 (007b1cb1)
		break;
	}
}

void ntAvatarChatMain::OnEnterTradeShop()
{
	// 983 (007b2280): sem a consulta de venda ao gateway (0x26d 0x10/0x11)
	ntAvatar* pAvatar =
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
	if (pAvatar)
	{
		ntAvatar* pMyAvatar = ntAvatarManager::Instance()->m_pMyAvatar;
		WVector diff = pAvatar->m_position - pMyAvatar->m_position;
		float dist = diff.Magnitude();
		if (TRADE_RANGE > dist)
		{
			ntAvatarManager::Instance()->m_pMyAvatar->Stop();
			if (CSharedDoc::Instance()->m_stateTrade == 0)
			{
				CSharedDoc::Instance()->SendTradeEnterShop(pAvatar->m_uid);
			}
			else
			{
				// "you cannot use another shop while yours is open"
				CChatMsg::Instance()->AddChatMsg(
					K2L_Compatibility("\xbb\xf3\xc1\xa1\xc0\xbb \xb0\xb3\xbc\xb3\xc1\xdf\xbf\xa1\xb4\xc2 \xb4\xd9\xb8\xa5 \xbb\xf3\xc1\xa1\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					0xffff00, true, false);
			}
		}
		else
		{
			// "go closer to enter the personal shop"
			CChatMsg::Instance()->AddChatMsg(
				K2L_Compatibility("\xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1\xbf\xa1 \xc0\xd4\xc0\xe5\xc7\xcf\xbd\xc3\xb7\xc1\xb8\xe9 \xb4\xf5 \xb0\xa1\xb1\xee\xc0\xcc \xb0\xa1\xbe\xdf \xc7\xd5\xb4\xcf\xb4\xd9."),
				0xffff00, true, false);
		}
	}
}

bool ntAvatarChatMain::OnPasswordDlgResult(int result, FrForm* pForm)
{
	// ORDER: emitted one wave later in the original
	if (0)
		ntAvatarChatMain::OnUnderBar_ShopUp();
	if (result == FrOK)
	{
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
		if (m_pPasswordDlg)
		{
			FrWnd* pWnd = m_pPasswordDlg->FindChildByName("password");
			FrEdit* pEdit = DYNAMIC_CAST(FrEdit, pWnd);

			std::string password;
			if (pEdit)
			{
				const char* text = pEdit->GetLine(1, false);
				password = text;
				this << MsgObject(NULL, M983(0x27), (int)text, 0, 0, 0, 0);
			}

			FrWnd* pOk = m_pPasswordDlg->FindChildByName("ok");
			if (pOk)
				pOk->Enable(false);

			WSendPacket packet((enumClientPacket)9);
			packet.Encode2(CSharedDoc::Instance()->m_roomInfo.roomGuid);
			packet.EncodeStr(password);
			packet.Send(TO_GAME);

			this << MsgObject(NULL, M983(0x16), 0, 0, 0, 0, 0);
			this << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		}
	}

	m_pPasswordDlg = NULL;
	return true;
}

void ntAvatarChatMain::OnRightMenuLBtnDown()
{
	if (m_pRightMenu == NULL)
		return;

	m_pRightMenu->SetVisible(false);
	const char* text = m_pRightMenu->GetSelectMenuText();

	if (IsLocalContent((localContentType_t)0x76 /*983: era 0x77; medido em 007b1d3a/007b2e32*/) &&
		strcmp(text, K2L_Compatibility(STR_MENU_TRADE)) == 0)
	{
		ntAvatar* pAvatar =
			ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
		if (pAvatar)
		{
			SendMsg(NULL, "PrivateTrade", M983(0x247), 0, pAvatar->m_guid,
				(int)pAvatar->m_nickName, 0);
			return;
		}
	}

	if (strcmp(text, K2L_Compatibility(STR_MENU_FOLLOW)) == 0)
	{
		OnFollowAvatar();
	}
	else if (strcmp(text, K2L_Compatibility(STR_MENU_INFO)) == 0)
	{
		OnAvatarInfo();
	}
	else if (strcmp(text, K2L_Compatibility(STR_MENU_WHISPER)) == 0)
	{
		ntAvatar* pAvatar =
			ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
		if (pAvatar)
			this << MsgObject(NULL, M983(0x37), (int)pAvatar->m_nickName, 0, 0, 0, 0);
	}
	else if (strcmp(text, K2L_Compatibility(STR_MENU_FRIEND)) == 0)
	{
		OnAddFriend();
	}
	else if (strcmp(text, K2L_Compatibility(STR_MENU_NOTE)) == 0)
	{
		OnNoteToFriend();
	}
	else if (strcmp(text, K2L_Compatibility(STR_MENU_MAKE_SHOP)) == 0 ||
		strcmp(text, K2L_Compatibility(STR_MENU_EDIT_SHOP)) == 0)
	{
		OnOpenTradeShop();
	}
	else if (strcmp(text, K2L_Compatibility(STR_MENU_VISIT_SHOP)) == 0)
	{
		OnEnterTradeShop();
	}
	else if (strcmp(text, K2L_Compatibility(STR_MENU_KICK)) == 0)
	{
		OnKickAvatar();
	}
	// PKT (frente C, lote 37): item do 983 (007b315b) depois do de expulsar
	else if (strcmp(text, K2L_Compatibility(STR_MENU_ACHIEVEMENT)) == 0)
	{
		OnAchievementAvatar983();
	}
}

void ntAvatarChatMain::OnRightMenuRBtnDown()
{
	if (m_pRightMenu)
		m_pRightMenu->SetVisible(false);
}

void ntAvatarChatMain::UpdateRightMenu()
{
	// ORDER: emitted one wave later in the original
	if (0)
		ntAvatarChatMain::OnUnderBar_ShopUp();
	ntAvatar* pAvatar =
		ntAvatarManager::Instance()->FindAvatarByNickName(m_targetNickName);
	if (pAvatar)
	{
		if (pAvatar != ntAvatarManager::Instance()->m_pMyAvatar)
		{
			if (ntAvatarManager::Instance()->m_pMyAvatar->m_canKick)
			{
				m_pRightMenu->SetMenuList(K2L_Compatibility(STR_MENU_VISIT_SHOP
					"@store|separator|" STR_MENU_FOLLOW
					"@follow|" STR_MENU_FRIEND
					"@friend_request|" STR_MENU_WHISPER
					"@whisper|" STR_MENU_NOTE "@chat_note|" STR_MENU_INFO
					"@information|separator|" STR_MENU_KICK
					"@exit|" STR_MENU_ACHIEVEMENT "@achievement")); // 983 00d26298
			}
			else if (pAvatar->m_canKick == true)
			{
				m_pRightMenu->SetMenuList(K2L_Compatibility(
					STR_MENU_FOLLOW "@follow|" STR_MENU_WHISPER "@whisper"));
			}
			else if (IsLocalContent((localContentType_t)0x76 /*983: era 0x77; medido em 007b1d3a/007b2e32*/))
			{
				m_pRightMenu->SetMenuList(K2L_Compatibility(
					STR_MENU_TRADE "@store|separator|" STR_MENU_VISIT_SHOP
								   "@store|separator|" STR_MENU_FOLLOW
								   "@follow|" STR_MENU_FRIEND
								   "@friend_request|" STR_MENU_WHISPER
								   "@whisper|" STR_MENU_NOTE
								   "@chat_note|" STR_MENU_INFO "@information"));
			}
			else
			{
				m_pRightMenu->SetMenuList(K2L_Compatibility(STR_MENU_VISIT_SHOP
					"@store|separator|" STR_MENU_FOLLOW
					"@follow|" STR_MENU_FRIEND
					"@friend_request|" STR_MENU_WHISPER
					"@whisper|" STR_MENU_NOTE "@chat_note|" STR_MENU_INFO
					"@information|" STR_MENU_ACHIEVEMENT "@achievement")); // 983 00d26158
			}

			if (!(pAvatar->m_state & 0x80))
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_VISIT_SHOP));

			const WVector& myPos =
				ntAvatarManager::Instance()->m_pMyAvatar->GetPosition();
			WVector diff = myPos - pAvatar->m_position;
			if (diff.Magnitude() > 64.0f)
			{
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_FOLLOW));
				if (IsLocalContent((localContentType_t)0x76 /*983: era 0x77; medido em 007b1d3a/007b2e32*/))
					m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_TRADE));
			}

			if (ntAvatarManager::Instance()->m_pMyAvatar->m_state & 0x80)
			{
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_VISIT_SHOP));
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_FOLLOW));
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_FRIEND));
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_NOTE));
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_INFO));
				m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_KICK));
				if (IsLocalContent((localContentType_t)0x76 /*983: era 0x77; medido em 007b1d3a/007b2e32*/))
					m_pRightMenu->UnableMenuItem(K2L_Compatibility(STR_MENU_TRADE));
			}
		}
		else
		{
			if (CSharedDoc::Instance()->m_stateTrade != 0)
				m_pRightMenu->SetMenuList(K2L_Compatibility(STR_MENU_EDIT_SHOP "@store"));
			else
				m_pRightMenu->SetMenuList(K2L_Compatibility(STR_MENU_MAKE_SHOP
					"@store_make|separator|" STR_MENU_INFO
					"@information|" STR_MENU_ACHIEVEMENT "@achievement")); // 983 00d260b0
		}
	}
	else
	{
		m_pRightMenu->SetVisible(false);
	}
}

void ntAvatarChatMain::PopupRightMenu(int x, int y)
{
	jp983_rastro("A819 popup rmenu=%p x=%d y=%d", m_pRightMenu, x, y);
	if (m_pRightMenu)
	{
		m_pRightMenu->MoveWindow(WPoint((float)x, (float)y));
		m_pRightMenu->SetVisible(true);
		m_pRightMenu->SetTopmost(true);
		UpdateRightMenu();
	}
}

void ntAvatarChatMain::ChangePortraitImage(int index)
{
	if (index >= 0 &&
		index < sizeof(m_pPortraitBmp) / sizeof(m_pPortraitBmp[0]))
	{
		if (m_pPortrait)
			m_pPortrait->SetBgImg(m_pPortraitBmp[index]);
	}
}

void ntAvatarChatMain::CloseAllDialogs()
{
	if (m_pAvatarTipDlg)
		m_pAvatarTipDlg->Close(true);
	if (m_pUserListDlg)
		m_pUserListDlg->Close(true);
	if (m_pChatLogDlg)
		m_pChatLogDlg->Close(true);
	if (m_pDlg4c8)
		m_pDlg4c8->Close(true);
	CTaskMain::CloseAllDialogs();
}

void ntAvatarChatMain::OnUnderBar_ShopUp()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
		ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	CTaskMain::OnUnderBar_ShopUp();
}

void ntAvatarChatMain::OnUnderBar_MyRoomUp()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
		ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	CTaskMain::OnUnderBar_MyRoomUp();
}

void ntAvatarChatMain::OnUnderBar_CookieUp()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
		ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	CTaskMain::OnUnderBar_CookieUp();
}

void ntAvatarChatMain::OnUnderBar_RankingUp()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
		ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	CTaskMain::OnUnderBar_RankingUp();
}

// 983 007b0d00 / 007b0dd0 (vtable 00d25eb4/00d25ec0, slots 20 e 23): para o boneco e chama o do CTaskMain, como os de cima
void ntAvatarChatMain::OnUnderBar_GuildUp()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
		ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	CTaskMain::OnUnderBar_GuildUp();
}

void ntAvatarChatMain::OnUnderBar_ClubUp983()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
		ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	CTaskMain::OnUnderBar_ClubUp983();
}

void ntAvatarChatMain::ProcessUI(float deltaTime)
{
	static bool s_bAlphaNumeric = true;

	if (g_pFresh->GetManager()->GetModalForm())
	{
		ntAvatar* pMyAvatar = ntAvatarManager::Instance()->m_pMyAvatar;
		if (pMyAvatar->IsMoving())
			pMyAvatar->Stop();
	}

	if (!CQuitWindow::Instance()->IsActive())
	{
		if ((g_input->Get("LCONTROL", false) ||
				g_input->Get("RCONTROL", false)) &&
			g_input->GetDown("HIDE", false))
		{
			m_uiHideMode = (m_uiHideMode + 1) % 3;
			g_pFresh->show = m_uiHideMode != 2;
			CUiPointer983::Instance983()->m_show40 = m_uiHideMode != 2;
			CMouseCursor::Instance()->Toggle(m_uiHideMode != 2);
			ShowOverAndUnderBar(m_uiHideMode != 1);
			if (m_uiHideMode == 2 && !CChatMsg::Instance()->IsClosed())
				CChatMsg::Instance()->Close(true, true);
			ntAvatarManager::Instance()->m_overlayDetail = m_uiHideMode == 0;
		}
	}
	else
	{
		m_uiHideMode = 0;
		g_pFresh->show = true;
		CUiPointer983::Instance983()->m_show40 = m_uiHideMode != 2;
	}

	if ((g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) &&
		g_input->GetDown("EMOTICON", false) &&
		CRankingInfo::Instance()->GetDlg() == NULL &&
		!CMessengerInfo::Instance()->IsVisible())
	{
		OnEmoticonBtnUp();
	}

	if (m_pLanguage && g_ime->IsAlphaNumericMode() != s_bAlphaNumeric)
	{
		s_bAlphaNumeric = g_ime->IsAlphaNumericMode();
		if (s_bAlphaNumeric)
			m_pLanguage->SetBgImg("chat_eng");
		else
			m_pLanguage->SetBgImg("chat_kor");
	}

	if (m_pChatInput && m_pChatTarget && !CChatMsg::Instance()->IsClosed())
	{
		if (g_input->GetDown("PAGE_UP", true))
		{
			if (m_pChatInput->m_nFlags.GetFlag(FWF_KEYFOCUS) &&
				CSharedDoc::Instance()->m_chatHistory.size() > 0)
			{
				const char* text = m_pChatInput->GetLine(1, false);
				std::list<std::string>::iterator it =
					std::find(CSharedDoc::Instance()->m_chatHistory.begin(),
						CSharedDoc::Instance()->m_chatHistory.end(), text);
				if (it != CSharedDoc::Instance()->m_chatHistory.end())
				{
					if (it != CSharedDoc::Instance()->m_chatHistory.begin())
					{
						--it;
						CChatMsg::Instance()->SetChatText((*it).c_str(), false);
					}
				}
				else
				{
					CChatMsg::Instance()->SetChatText(
						CSharedDoc::Instance()->m_chatHistory.back().c_str(),
						false);
				}
			}
			else if (m_pChatTarget->m_nFlags.GetFlag(FWF_KEYFOCUS) &&
				CSharedDoc::Instance()->m_reservedChatList.size() > 0)
			{
				CChatMsg::Instance()->SetChatText(
					CSharedDoc::Instance()->GetPrevChatTarget(
						m_pChatTarget->GetLine(1, false)),
					false);
			}
		}
		else if (g_input->GetDown("PAGE_DOWN", true))
		{
			if (m_pChatInput->m_nFlags.GetFlag(FWF_KEYFOCUS) &&
				CSharedDoc::Instance()->m_chatHistory.size() > 0)
			{
				const char* text = m_pChatInput->GetLine(1, false);
				std::list<std::string>::iterator it =
					std::find(CSharedDoc::Instance()->m_chatHistory.begin(),
						CSharedDoc::Instance()->m_chatHistory.end(), text);
				if (it != CSharedDoc::Instance()->m_chatHistory.end())
				{
					++it;
					if (it != CSharedDoc::Instance()->m_chatHistory.end())
						CChatMsg::Instance()->SetChatText((*it).c_str(), false);
					else
						CChatMsg::Instance()->SetChatText("", false);
				}
			}
			else if (m_pChatTarget->m_nFlags.GetFlag(FWF_KEYFOCUS) &&
				CSharedDoc::Instance()->m_reservedChatList.size() > 0)
			{
				CChatMsg::Instance()->SetChatText(
					CSharedDoc::Instance()->GetNextChatTarget(
						m_pChatTarget->GetLine(1, false)),
					false);
			}
		}
	}

	if ((g_input->Get("LCONTROL", true) || g_input->Get("RCONTROL", true)) &&
		g_input->GetDown("JUKEBOX", true))
	{
		ToggleJukeBox();
	}

	if (ntAvatarManager::Instance()->m_pMyAvatar->m_state & 0x80)
	{
		TurnOffUnderTabBar();
		EnableOverAndUnderBar(false);
	}
	else
	{
		EnableOverAndUnderBar(true);
	}
}
