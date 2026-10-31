#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "ghostmain.h"
#define REBANG_GHOSTMAIN_CPP
#include "ghostmanager.h"
#include "ghosthandler.h"
#include "parttidlist.h"
#include "partsinfo.h"
#include "playerdlg.h"
#include "uccmanager.h"
#include "rankingdlg.h"
#include "user_info.h"
#include "userinfoform.h"
#include "inputmanager.h"
#include "clientsetting.h"
#include "equipdlg.h"
#include "taskmanager.h"
#include "frdesktop.h"
#include "frarea.h"
#include "frlistbox.h"
#include "frgraphicinterface.h"
#include "packet.h"
#include "shareddoc.h"
#include "cardsystem.h"
#include "commonutil.h"
#include "mousecursor.h"
#include "type_convert.h"
// HACK: last, so that their inlines come after the PCH ones (WSize/WRect) in emission order
#include "exhibition.h"
#include "ghostdocument.h"


// HACK: "-%d" belongs to ghostgamedlg.obj (linked first), which holds it as a local static of
// its naked FrRecentMatch::OnDrawBgArea; a genuine literal here would not be discarded
#define GHOST_FMT_MINUS_D "-%d"

// HACK: the original has g_HUGE and g_EPSILON (wmath.h) at the top of its .rdata
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

class FrRecycleDlg : public FrForm
{
	DECLARE_OBJECT(FrRecycleDlg)
};

class FrBongdariShopDlg : public FrForm
{
	DECLARE_OBJECT(FrBongdariShopDlg)
};

struct GHOST_GAME_DATA : public GHOST_GAME_BRIEF
{
};

IMPLEMENT_ACTOR(CGhostMain, CTaskMain)
BEGIN_FRESH_MSGMAP(CGhostMain, CTaskMain)
ON_FRESH_VI("GHOST", FRCMD_INIT, CGhostMain::OnInitGhostMain)
ON_FRESH_VV("GHOST", FRCMD_FINISH, CGhostMain::OnInitGhostFinish)
ON_FRESH_VI("GHOST.start", FRCMD_INIT, CGhostMain::OnInitStart)
ON_FRESH_VV("GHOST.start", FRCMD_LBUTTONDOWN, CGhostMain::OnDownStart)
ON_FRESH_VI("GHOST.pet_user", FRCMD_INIT, CGhostMain::OnInitPlayerZone)
ON_FRESH_VV("GHOST.pet_user", FRCMD_OWNERDRAW, CGhostMain::OnDrawPlayerZone)
ON_FRESH_VI("GHOST.pet_ghost", FRCMD_INIT, CGhostMain::OnInitGhostZone)
ON_FRESH_VV("GHOST.pet_ghost", FRCMD_OWNERDRAW, CGhostMain::OnDrawGhostZone)
ON_FRESH_VI("GHOST.tip_ghost", FRCMD_INIT, CGhostMain::OnInitTipZone)
ON_FRESH_VV("GHOST.tip_ghost", FRCMD_OWNERDRAW, CGhostMain::OnDrawTipZone)
ON_FRESH_VI("GHOST.user_stat", FRCMD_INIT, CGhostMain::OnInitAreaUserStat)
ON_FRESH_VV("GHOST.user_stat", FRCMD_OWNERDRAW, CGhostMain::OnDrawAreaUserStat)
ON_FRESH_VI("GHOST.ghost_stat", FRCMD_INIT, CGhostMain::OnInitAreaGhostStat)
ON_FRESH_VV("GHOST.ghost_stat", FRCMD_OWNERDRAW,
	CGhostMain::OnDrawAreaGhostStat)
ON_FRESH_VI("GHOST.select_course", FRCMD_INIT, CGhostMain::OnInitSelectCourse)
ON_FRESH_VV("GHOST.select_course", FRCMD_LBUTTONDOWN,
	CGhostMain::OnDownSelectCourse)
ON_FRESH_VI("GHOST.user_equip", FRCMD_INIT, CGhostMain::OnInitUserEquip)
ON_FRESH_VI("GHOST.user_equip", FRCMD_OWNERDRAW, CGhostMain::OnDrawUserEquip)
ON_FRESH_VV("GHOST.user_equip", FRCMD_LBUTTONDOWN, CGhostMain::OnDownUserEquip)
ON_FRESH_VI("GHOST.ghost_equip", FRCMD_INIT, CGhostMain::OnInitGhostEquip)
ON_FRESH_VI("GHOST.ghost_equip", FRCMD_OWNERDRAW, CGhostMain::OnDrawGhostEquip)
ON_FRESH_VI("GHOST.ubar", FRCMD_INIT, CTaskMain::OnUnderBarInit)
ON_FRESH_VI("GHOST.ubar.back", FRCMD_INIT, CTaskMain::OnUnderBar_BackInit)
ON_FRESH_VV("GHOST.ubar.back", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_BackUp)
ON_FRESH_VI("GHOST.ubar.back", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.back", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.front", FRCMD_INIT, CTaskMain::OnUnderBar_FrontInit)
ON_FRESH_VV("GHOST.ubar.front", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_FrontUp)
ON_FRESH_VI("GHOST.ubar.front", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.front", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.shop", FRCMD_INIT, CTaskMain::OnUnderBar_ShopInit)
ON_FRESH_VV("GHOST.ubar.shop", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_ShopUp)
ON_FRESH_VI("GHOST.ubar.shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.shop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.myroom", FRCMD_INIT, CTaskMain::OnUnderBar_MyRoomInit)
ON_FRESH_VV("GHOST.ubar.myroom", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MyRoomUp)
ON_FRESH_VI("GHOST.ubar.myroom", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.myroom", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (C40.3, s110): "ubar.achievement" no lugar do "ubar.itemstorage" (mapa do 983: 00651e30 / 006582f0 / hover)
ON_FRESH_VI("GHOST.ubar.achievement", FRCMD_INIT, CTaskMain::OnUnderBar_AchievementInit983)
ON_FRESH_VV("GHOST.ubar.achievement", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_AchievementUp983)
ON_FRESH_VI("GHOST.ubar.achievement", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.achievement", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.gift", FRCMD_INIT, CTaskMain::OnUnderBar_GiftInit)
ON_FRESH_VV("GHOST.ubar.gift", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_GiftUp)
ON_FRESH_VI("GHOST.ubar.gift", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.gift", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.cookie", FRCMD_INIT, CTaskMain::OnUnderBar_CookieInit)
ON_FRESH_VV("GHOST.ubar.cookie", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_CookieUp)
ON_FRESH_VI("GHOST.ubar.cookie", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.cookie", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.game", FRCMD_INIT, CTaskMain::OnUnderBar_GameInit)
ON_FRESH_VV("GHOST.ubar.game", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_GameUp)
ON_FRESH_VI("GHOST.ubar.game", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.game", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.server", FRCMD_INIT, CTaskMain::OnUnderBar_ServerInit)
ON_FRESH_VV("GHOST.ubar.server", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ServerUp)
ON_FRESH_VI("GHOST.ubar.server", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.server", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.exit", FRCMD_INIT, CTaskMain::OnUnderBar_ExitInit)
ON_FRESH_VV("GHOST.ubar.exit", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_ExitUp)
ON_FRESH_VI("GHOST.ubar.exit", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.ubar.exit", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.ubar.tabbtn_b", FRCMD_INIT, CTaskMain::OnUnderBar_TabBtnInit)
ON_FRESH_VV("GHOST.ubar.tabbtn_b", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TabBtnUp)
ON_FRESH_VI("GHOST.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)
ON_FRESH_VI("GHOST.utab.magicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_MagicBoxInit)
ON_FRESH_VV("GHOST.utab.magicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MagicBoxUp)
ON_FRESH_VI("GHOST.utab.magicbox", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.magicbox", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s106): "utab.combine" e "utab.gacha" (mapa do 983: init, LBUTTONDOWN, hover)
ON_FRESH_VI("GHOST.utab.combine", FRCMD_INIT, CTaskMain::OnUnderBar_CombineInit983)
ON_FRESH_VV("GHOST.utab.combine", FRCMD_LBUTTONDOWN, CTaskMain::OnUnderBar_CombineUp983)
ON_FRESH_VI("GHOST.utab.combine", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.combine", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.utab.gacha", FRCMD_INIT, CTaskMain::OnUnderBar_GachaInit983)
ON_FRESH_VV("GHOST.utab.gacha", FRCMD_LBUTTONDOWN, CTaskMain::OnUnderBar_GachaUp983)
ON_FRESH_VI("GHOST.utab.gacha", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.gacha", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.utab.bongdarishop", FRCMD_INIT,
	CTaskMain::OnUnderBar_BongdariShopInit)
ON_FRESH_VV("GHOST.utab.bongdarishop", FRCMD_LBUTTONDOWN,
	CTaskMain::OnUnderBar_BongdariShopUp)
ON_FRESH_VI("GHOST.utab.bongdarishop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.bongdarishop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s111): "utab.point_shop" (AlchemyDlg) e "utab.card_compose" (alquimia de cartas), LBUTTONUP
ON_FRESH_VI("GHOST.utab.point_shop", FRCMD_INIT, CTaskMain::OnUnderBar_PointShopInit983)
ON_FRESH_VV("GHOST.utab.point_shop", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_PointShopUp983)
ON_FRESH_VI("GHOST.utab.point_shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.point_shop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.utab.card_compose", FRCMD_INIT, CTaskMain::OnUnderBar_CardComposeInit983)
ON_FRESH_VV("GHOST.utab.card_compose", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_CardComposeUp983)
ON_FRESH_VI("GHOST.utab.card_compose", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.card_compose", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s111): sem "utab.tikimagicbox", "utab.scratch", "utab.maket", "obar.messenger_alarm" e "obar.gift_alarm"
ON_FRESH_VI("GHOST.utab.ranking", FRCMD_INIT, CTaskMain::OnUnderBar_RankingInit)
ON_FRESH_VV("GHOST.utab.ranking", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_RankingUp)
ON_FRESH_VI("GHOST.utab.ranking", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.ranking", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.utab.guild", FRCMD_INIT, CTaskMain::OnUnderBar_GuildInit)
ON_FRESH_VV("GHOST.utab.guild", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_GuildUp)
ON_FRESH_VI("GHOST.utab.guild", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.guild", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.utab.notice", FRCMD_INIT, CTaskMain::OnUnderBar_NoticeInit)
ON_FRESH_VV("GHOST.utab.notice", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_NoticeUp)
ON_FRESH_VI("GHOST.utab.notice", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.utab.notice", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.obar", FRCMD_INIT, CTaskMain::OnOverBar_Init)
ON_FRESH_VI("GHOST.obar.title", FRCMD_INIT, CTaskMain::OnOverBar_TitleInit)
ON_FRESH_VI("GHOST.obar.site", FRCMD_INIT, CTaskMain::OnOverBar_SiteInit)
ON_FRESH_VI("GHOST.obar.char", FRCMD_INIT, CTaskMain::OnOverBar_CharInit)
ON_FRESH_VV("GHOST.obar.char", FRCMD_LBUTTONDOWN, CTaskMain::OnOverBar_CharDown)
ON_FRESH_VI("GHOST.obar.char", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharOwnerDraw)
ON_FRESH_VI("GHOST.obar.caddie", FRCMD_INIT, CTaskMain::OnOverBar_CaddieInit)
ON_FRESH_VV("GHOST.obar.caddie", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CaddieDown)
ON_FRESH_VI("GHOST.obar.caddie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieOwnerDraw)
ON_FRESH_VI("GHOST.obar.club", FRCMD_INIT, CTaskMain::OnOverBar_ClubInit)
ON_FRESH_VV("GHOST.obar.club", FRCMD_LBUTTONDOWN, CTaskMain::OnOverBar_ClubDown)
ON_FRESH_VI("GHOST.obar.club", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubOwnerDraw)
ON_FRESH_VI("GHOST.obar.aztec", FRCMD_INIT, CTaskMain::OnOverBar_AztecInit)
ON_FRESH_VV("GHOST.obar.aztec", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_AztecDown)
ON_FRESH_VI("GHOST.obar.aztec", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecOwnerDraw)
ON_FRESH_VI("GHOST.obar.mascot", FRCMD_INIT, CTaskMain::OnOverBar_MascotInit)
ON_FRESH_VV("GHOST.obar.mascot", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MascotDown)
ON_FRESH_VI("GHOST.obar.mascot", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotOwnerDraw)
ON_FRESH_VI("GHOST.obar.char_sel", FRCMD_INIT, CTaskMain::OnOverBar_CharSelInit)
ON_FRESH_VI("GHOST.obar.char_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharSelOwnerDraw)
ON_FRESH_VI("GHOST.obar.caddie_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieSelInit)
ON_FRESH_VI("GHOST.obar.caddie_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieSelOwnerDraw)
ON_FRESH_VI("GHOST.obar.club_sel", FRCMD_INIT, CTaskMain::OnOverBar_ClubSelInit)
ON_FRESH_VI("GHOST.obar.club_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubSelOwnerDraw)
ON_FRESH_VI("GHOST.obar.aztec_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecSelInit)
ON_FRESH_VI("GHOST.obar.aztec_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecSelOwnerDraw)
ON_FRESH_VI("GHOST.obar.mascot_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotSelInit)
ON_FRESH_VI("GHOST.obar.mascot_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotSelOwnerDraw)
ON_FRESH_VI("GHOST.obar.pang_cookie", FRCMD_INIT,
	CTaskMain::OnOverBar_PangCookieInit)
ON_FRESH_VI("GHOST.obar.pang_cookie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_PangCookieOwnerDraw)
ON_FRESH_VI("GHOST.obar.onelinemsg_req", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineReqBtnInit)
ON_FRESH_VV("GHOST.obar.onelinemsg_req", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OneLineReqBtnUp)
ON_FRESH_VI("GHOST.obar.onelineview", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineViewInit)
ON_FRESH_VI("GHOST.obar.onelineview", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_OneLineViewOwnerDraw)
ON_FRESH_VI("GHOST.obar.spcard", FRCMD_INIT,
	CTaskMain::OnOverBar_SpecialCardInit)
ON_FRESH_VV("GHOST.obar.spcard", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_SpecialCardUp)
ON_FRESH_VI("GHOST.obar.spcard", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.obar.spcard", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.obar.messenger", FRCMD_INIT,
	CTaskMain::OnOverBar_MessengerInit)
ON_FRESH_VV("GHOST.obar.messenger", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MessengerUp)
ON_FRESH_VI("GHOST.obar.messenger", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.obar.messenger", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.obar.myinfo", FRCMD_INIT, CTaskMain::OnOverBar_MyInfoInit)
ON_FRESH_VV("GHOST.obar.myinfo", FRCMD_LBUTTONUP, CTaskMain::OnOverBar_MyInfoUp)
ON_FRESH_VI("GHOST.obar.myinfo", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.obar.myinfo", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.obar.option", FRCMD_INIT, CTaskMain::OnOverBar_OptionInit)
ON_FRESH_VV("GHOST.obar.option", FRCMD_LBUTTONUP, CTaskMain::OnOverBar_OptionUp)
ON_FRESH_VI("GHOST.obar.option", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("GHOST.obar.option", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("GHOST.obar.messenger_bubble", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleInit)
ON_FRESH_VI("GHOST.obar.messenger_bubble", FRCMD_OWNERDRAW,
	CTaskMain::OnMessengerBubbleOwnerDraw)
ON_FRESH_VI("GHOST.obar.messenger_desc", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleEditInit)
ON_FRESH_VI("GHOST.obar.tooltip", FRCMD_OWNERDRAW,
	CTaskMain::OnBarTooltipOwnerDraw)
END_FRESH_MSGMAP()

std::string __stdcall GetIconName(const unsigned long& typeId)
{
	std::string name("w_");
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeId);
	if (pItem)
		name += pItem->Icon;
	else
		name = "w_caddie_no";
	return name;
}

void __stdcall GetEquipIcons(std::vector<std::string>& icons,
	const sUserInfo& info)
{
	icons.clear();
	icons.reserve(6);
	icons.assign(6, "");

	icons[0] = GetIconName(info.charInfo.tid);
	icons[1] = GetIconName(info.caddieInfo.tid);
	icons[2] = GetIconName(info.clubInfo.tid);
	icons[3] = GetIconName(info.userEquip.tidBall);
	icons[4] = GetIconName(info.mascotInfo.tid);
	icons[5] = std::string("gameroom_equip_item_bn");
}

class CGhostMain::cImgBinder
{
public:
	struct sImgInfo
	{
		sImgInfo(const std::string& strImage, const WRect& rect);

		std::string m_strImage;
		WRect m_rect;
	};

	cImgBinder(int nReserve);
	~cImgBinder();

	void AddImage(std::string strImage, const WPoint& pos);
	__forceinline WPoint GetPos();
	void SetPosX(float x);
	void AddPos(float dx, float dy);
	void AddPos(const WPoint& delta);
	void Render(unsigned char alpha);

	std::vector<sImgInfo> m_vecImage;
};

CGhostMain::cImgBinder::sImgInfo::sImgInfo(const std::string& strImage,
	const WRect& rect)
	: m_strImage(strImage), m_rect(rect)
{
}

CGhostMain::cImgBinder::cImgBinder(int nReserve)
{
	m_vecImage.reserve(nReserve);
}

CGhostMain::cImgBinder::~cImgBinder()
{
}

void CGhostMain::cImgBinder::AddImage(std::string strImage, const WPoint& pos)
{
	const Bitmap* pBitmap = g_pFresh->GetBitmap(strImage.c_str());
	if (pBitmap)
		m_vecImage.push_back(sImgInfo(strImage,
			WRect(pos,
				WSize((float)pBitmap->Width(), (float)pBitmap->Height()))));
}

__forceinline WPoint CGhostMain::cImgBinder::GetPos()
{
	WPoint pos(0.0f, 0.0f);
	if (!m_vecImage.empty())
		pos = WPoint(m_vecImage[0].m_rect.x, m_vecImage[0].m_rect.y);
	return pos;
}

void CGhostMain::cImgBinder::SetPosX(float x)
{
	if (0)
		((CGhostMain::cImgBinder*)0)
			->CGhostMain::cImgBinder::AddPos(*(float*)0, *(float*)0);
	float curX = 0.0f;
	if (!m_vecImage.empty())
		curX = m_vecImage[0].m_rect.x;

	for (std::vector<sImgInfo>::iterator it = m_vecImage.begin();
		it != m_vecImage.end(); ++it)
		(*it).m_rect.x += x - curX;
}

void CGhostMain::cImgBinder::AddPos(float dx, float dy)
{
	std::vector<sImgInfo>::iterator it = m_vecImage.begin();
	for (; it != m_vecImage.end(); ++it)
	{
		(*it).m_rect.x += dx;
		(*it).m_rect.y += dy;
	}
}

void CGhostMain::cImgBinder::AddPos(const WPoint& delta)
{
	std::vector<sImgInfo>::iterator it = m_vecImage.begin();
	for (; it != m_vecImage.end(); ++it)
	{
		(*it).m_rect.x += delta.x;
		(*it).m_rect.y += delta.y;
	}
}

void CGhostMain::cImgBinder::Render(unsigned char alpha)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	std::vector<sImgInfo>::iterator it = m_vecImage.begin();
	for (; it != m_vecImage.end(); ++it)
	{
		const Bitmap* pBitmap = g_pFresh->GetBitmap((*it).m_strImage.c_str());
		if (pBitmap)
			pGDI->DrawTexture(pBitmap, (*it).m_rect, (alpha << 24) | 0xffffff,
				0);
	}
}

CGhostMain::CGhostMain()
	: m_pUserPet(NULL),
	  m_pGhostPet(NULL),
	  m_pUserParts(NULL),
	  m_pGhostParts(NULL),
	  m_ghostAlpha(0.0f),
	  m_bGhostVisible(false),
	  m_pUserStatImage(NULL),
	  m_pGhostStatImage(NULL),
	  m_userStatAlpha(0.0f),
	  m_userStatStopX(0.0f),
	  m_ghostStatStopX(0.0f),
	  m_userStatSpeed(0.0f),
	  m_ghostStatSpeed(0.0f),
	  m_bShowStat(false),
	  m_statTime(0.0f),
	  m_statSpeedTime(0.0f),
	  m_pSelectedIcon(NULL),
	  m_equipAlpha(0.0f),
	  m_pEquipDlg(NULL),
	  m_pUserStat(NULL),
	  m_pGhostStat(NULL),
	  m_pCourseBitmap(NULL),
	  m_pTipImage(NULL)
{
	Initialize();
}

CGhostMain::~CGhostMain()
{
}

void CGhostMain::Initialize()
{
	CSharedDoc::Instance()->m_gameMode = 5;
	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
	{
		WSendPacket packet((enumClientPacket)0xc1);
		packet.Encode1(2);
		packet.Send(TO_GAME);
	}

	memset(CSharedDoc::Instance()->m_userInfo, 0,
		sizeof(CSharedDoc::Instance()->m_userInfo));
	CCardManager::Instance()->CalcCardPeriodAndStatus();

	m_areas.reserve(5);
	m_areas.assign(5, (FrArea*)NULL);

	m_pUserParts = new CPartTidList;
	m_pGhostParts = new CPartTidList;

	m_pTipImage = new cImgBinder(2);
	m_pTipImage->AddImage("ghost_main_tip1", WPoint(0.0f, 0.0f));
	m_pTipImage->AddImage("ghost_main_tip2", WPoint(256.0f, 0.0f));
	m_pTipImage->AddPos(200.0f, -120.0f);
	m_tipTime = 5.0f;
	m_bTipHide = false;

	m_pUserStatImage = new cImgBinder(2);
	m_pUserStatImage->AddImage("ghost_user_stat1", WPoint(0.0f, 0.0f));
	m_pUserStatImage->AddImage("ghost_user_stat2", WPoint(256.0f, 0.0f));
	m_pUserStatImage->AddPos(-(g_view->GetWidth() * 0.5f),
		g_view->GetHeight() - 162.0f - 77.0f);

	m_pGhostStatImage = new cImgBinder(2);
	m_pGhostStatImage->AddImage("ghost_ghost_stat1", WPoint(0.0f, 0.0f));
	m_pGhostStatImage->AddImage("ghost_ghost_stat2", WPoint(256.0f, 0.0f));
	m_pGhostStatImage->AddPos(g_view->GetWidth(),
		g_view->GetHeight() - 162.0f - 77.0f);

	m_userStatStopX = 17.0f;
	m_ghostStatStopX = g_view->GetWidth() - 129.0f - 256.0f - 17.0f;
	m_userStatSpeed = g_view->GetWidth() * 0.5f + 17.0f;
	m_ghostStatSpeed = 402.0f;

	m_pUserStat = BuildStatusBar(0);
	m_pGhostStat = BuildStatusBar(1);
}

C_ASSERT(offsetof(sUserInfo, charInfo) == 0x2f52);
C_ASSERT(offsetof(sUserInfo, clubInfo) == 0x316c);
C_ASSERT(offsetof(sUserInfo, mascotInfo) == 0x3188);
C_ASSERT(offsetof(sUserInfo, stat.Level) == 0x17d);

CStatBar* CGhostMain::BuildStatusBar(int index)
{
	sUserInfo* pInfo = &CSharedDoc::Instance()->m_userInfo[index];

	if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
	{
		if (index == 1)
			CCardManager::Instance()->m_charInfo[0] =
				CSharedDoc::Instance()->m_userInfo[1].charInfo;
		CCardManager::Instance()->SetPlayerIndex(index);
	}

	CStatBar* pBar = new CStatBar(10.0f, 5.0f);

	unsigned char level[5] = { 0, 0, 0, 0, 0 };
	unsigned char penalty[5] = { 0, 0, 0, 0, 0 };
	unsigned char capacity[5] = { 0, 0, 0, 0, 0 };

	// 983 00823330: um sCapacityParam983 com partes aux, slots ganhos e guid do taco e o mascote do sUserInfo
	// (o adaptador antigo zerava esses campos e passava aux NULL ao GetCapacity)
	for (int i = 0; i < 5; i++)
	{
		sCapacityParam983 param;
		param.type = i;
		param.tidChar = pInfo->charInfo.tid;
		param.m_8 = (unsigned long)pInfo->charInfo.PCL;
		param.tidParts = pInfo->charInfo.tidParts;
		param.tidAuxParts = pInfo->charInfo.tidAuxParts;
		param.tidClub = pInfo->clubInfo.tid;
		param.clubAttrUp = pInfo->clubInfo.PCL;
		param.clubSlotUp = pInfo->clubInfo.slotUp983;
		param.guidClub = pInfo->clubInfo.guid;
		param.tidCaddie = pInfo->caddieInfo.tid;
		param.m_28 = pInfo->mascotInfo.tid;
		param.level = pInfo->stat.Level;
		param.m_30 = 0;
		status_t status = ItemManager()->GetLevelWithPenalty(&param);
		level[i] = status.level;
		penalty[i] = status.penalty;
		capacity[i] = ItemManager()->GetCapacity(&param);
	}

	pBar->SetDestTime(1.2f);
	for (int i = 0; i < 5; i++)
		pBar->SetRange((eLvlType)i, level[i], capacity[i], penalty[i]);

	return pBar;
}

void CGhostMain::StatProcess(float dt)
{
	m_pUserStat->Process(dt);
	m_pGhostStat->Process(dt);
}

CExhibition* CGhostMain::MakePet(FrArea* pArea, CPartTidList* pParts, int index)
{
	if (0)
		((CGhostMain*)0)->CGhostMain::BuildStatusBar(*(int*)0);
	m_hand.Enter();
	if (pArea == NULL)
	{
		m_hand.Leave();
		return NULL;
	}

	WRect rect = pArea->GetRect();
	CExhibition* pPet = new CExhibitionByHardwareTnL(rect.w * 0.5f + rect.x,
		rect.h * 0.5f + rect.y + 15.0f, 0.35f, false);
	pPet->SetArea(rect.w, rect.h);

	sUserInfo* pInfo = &CSharedDoc::Instance()->m_userInfo[index];
	sCharacterInfo* pChar = &pInfo->charInfo;
	switch (pChar->tid)
	{
	case 0x4000000:
		pPet->SetDistance(11.0f, true, false);
		break;
	case 0x4000005:
		pPet->SetDistance(9.0f, true, false);
		break;
	default:
		pPet->SetDistance(10.0f, true, false);
		break;
	}

	if (!pParts->SetTids(pChar, 0xff) || !pParts->IsComboValid())
		pParts->SetDefaultTids();

	pPet->SetModel(*pParts, pChar->tidAuxParts, NULL, 3.0f, 0);
	if (pInfo->info.angelicWingsEffect)
		pPet->AttachAngelWingFx();

	m_hand.Leave();
	return pPet;
}

void CGhostMain::HidePet(bool bWait)
{
	// HACK: the original emits the CAnotherHand::IsWorkDone COMDAT (anotherhand.inl) here
	if (0)
		m_hand.IsWorkDone();

	if (bWait)
		m_hand.WaitUntilWorkDone(9999);

	if (m_pUserPet)
	{
		delete m_pUserPet;
		m_pUserPet = NULL;
	}
	if (m_pGhostPet)
	{
		delete m_pGhostPet;
		m_pGhostPet = NULL;
	}
}

void CGhostMain::OnPreLoadInit()
{
	CTaskMain::OnPreLoadInit();
}

void CGhostMain::OnLoad()
{
	CTaskMain::OnLoad();
}

void CGhostMain::OnInit()
{
	CTaskMain::OnInit();
}

void CGhostMain::OnDestroy()
{
	m_areas.clear();

	if (m_pUserPet)
	{
		delete m_pUserPet;
		m_pUserPet = NULL;
	}
	if (m_pGhostPet)
	{
		delete m_pGhostPet;
		m_pGhostPet = NULL;
	}
	if (m_pUserParts)
	{
		delete m_pUserParts;
		m_pUserParts = NULL;
	}
	if (m_pGhostParts)
	{
		delete m_pGhostParts;
		m_pGhostParts = NULL;
	}
	if (m_pTipImage)
	{
		delete m_pTipImage;
		m_pTipImage = NULL;
	}
	if (m_pUserStatImage)
	{
		delete m_pUserStatImage;
		m_pUserStatImage = NULL;
	}
	if (m_pGhostStatImage)
	{
		delete m_pGhostStatImage;
		m_pGhostStatImage = NULL;
	}
	if (m_pUserStat)
	{
		delete m_pUserStat;
		m_pUserStat = NULL;
	}
	if (m_pGhostStat)
	{
		delete m_pGhostStat;
		m_pGhostStat = NULL;
	}

	CTaskMain::OnDestroy();
}

void CGhostMain::OnProcess(float dt)
{
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
		RefreshAllUccClothes();

	OnProcTip(dt);

	if (m_userStatAlpha < 255.0f)
	{
		m_userStatAlpha += dt * 204.0f;
		if (m_userStatAlpha > 255.0f)
			m_userStatAlpha = 255.0f;
	}

	if (m_bShowStat)
	{
		StatProcess(dt);
		if (m_equipAlpha < 255.0f)
		{
			m_equipAlpha += dt * 510.0f;
			if (m_equipAlpha > 255.0f)
				m_equipAlpha = 255.0f;
		}
	}
	else
	{
		if (m_statTime < 3.0f)
			m_statTime += dt;

		if (m_statSpeedTime < 4.0f)
			m_statSpeedTime += dt * 1.2f;

		if (m_pUserStatImage->GetPos().x < m_userStatStopX)
		{
			m_pUserStatImage->AddPos(
				m_userStatSpeed * m_statTime * m_statTime * dt, 0.0f);
			if (m_pUserStatImage->GetPos().x > m_userStatStopX)
				m_pUserStatImage->SetPosX(m_userStatStopX);
		}

		if (m_pGhostStatImage->GetPos().x > m_ghostStatStopX)
		{
			m_pGhostStatImage->AddPos(
				-(m_ghostStatSpeed * m_statSpeedTime * dt), 0.0f);
			if (m_pGhostStatImage->GetPos().x < m_ghostStatStopX)
				m_pGhostStatImage->SetPosX(m_ghostStatStopX);
		}

		if (m_pUserStatImage->GetPos().x >= m_userStatStopX &&
			m_pGhostStatImage->GetPos().x <= m_ghostStatStopX)
			m_bShowStat = true;
	}

	if (g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop"))
		return;

	bool bEquipOpen = DYNAMIC_CAST(FrEquipDlg,
						  g_pFresh->GetManager()->GetDesktop()->FindChildByName(
							  "equip")) != NULL;
	bool bRecycleOpen =
		DYNAMIC_CAST(FrRecycleDlg,
			g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"recycledlg")) != NULL;

	if (g_input->GetDown("ESCAPE", true) && !bEquipOpen && !bRecycleOpen)
	{
		OnExit();
		return;
	}

	if (!m_hand.IsWorking(0))
	{
		bool bRotate = !bEquipOpen && !bRecycleOpen;
		if (m_pUserPet)
		{
			m_pUserPet->SetEnableRotate(bRotate);
			m_pUserPet->Process(dt, false);
		}

		if (m_pGhostPet)
		{
			if (m_ghostAlpha < 255.0f)
			{
				m_ghostAlpha += dt * 127.5f;
				if (m_ghostAlpha > 255.0f)
					m_ghostAlpha = 255.0f;
			}

			if (!m_bGhostVisible && m_ghostAlpha > 32.0f)
				m_bGhostVisible = true;

			m_pGhostPet->SetAlpha((unsigned char)m_ghostAlpha);

			if (!bEquipOpen && !bRecycleOpen)
				m_pGhostPet->Process(dt, false);
		}
	}

	CTaskMain::OnProcess(dt);
}

void CGhostMain::OnDisplay()
{
	if (0)
		((CGhostMain*)0)->CGhostMain::HidePet(*(bool*)0);
}

void CGhostMain::OnExit()
{
	CSharedDoc::Instance()->m_gameMode = 0;
	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
	{
		WSendPacket packet((enumClientPacket)0xc1);
		packet.Encode1(1);
		packet.Send(TO_GAME);
	}

	CSharedDoc::Instance()->m_myInfo.roomIndex = 0xffff;
	CSharedDoc::Instance()->m_roomInfo = sRoomInfo();

	WSendPacket packet(0xc2);
	packet.Encode1(10);
	packet.Encode4(CSharedDoc::Instance()->m_userInfo[1].info.dwUID);
	packet.Encode1(m_ghostSlot);
	packet.Send(TO_GAME);

	CGhostManager::Instance()->ReleaseDocument();
	// 983 00823700: so troca de task com o flag (+0x10b28, je 00823864) e vai a "TOPPAGE";
	// sem ROOMLIST e sem o else do 645 (unica ref a CLobbyTask/TOPPAGE na faixa ghost)
	if (CSharedDoc::Instance()->m_bReturnLobbyNotice)
	{
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0,
			0, 0);
	}
}

void CGhostMain::OnInitGhostFinish()
{
	m_pOverBarTitle->SetBgImg("title_ghost_main");
	m_pOverBarPangCookie->SetVisible(false);

	GetEquipIcons(m_userIcons, CSharedDoc::Instance()->m_userInfo[0]);
	m_pUserEquip->AddItem(&m_userIcons[0]);
	m_pUserEquip->AddItem(&m_userIcons[1]);
	m_pUserEquip->AddItem(&m_userIcons[2]);
	m_pUserEquip->AddItem(&m_userIcons[3]);
	m_pUserEquip->AddItem(&m_userIcons[4]);
	m_pUserEquip->AddItem(&m_userIcons[5]);

	GetEquipIcons(m_ghostIcons, CSharedDoc::Instance()->m_userInfo[1]);
	m_pGhostEquip->AddItem(&m_ghostIcons[0]);
	m_pGhostEquip->AddItem(&m_ghostIcons[1]);
	m_pGhostEquip->AddItem(&m_ghostIcons[2]);
	m_pGhostEquip->AddItem(&m_ghostIcons[3]);
	m_pGhostEquip->AddItem(&m_ghostIcons[4]);
	m_pGhostEquip->AddItem(&m_ghostIcons[5]);

	m_pUnderBarShop->Enable(false);
	m_pUnderBarMyRoom->Enable(false);
	m_pUnderBarRanking->Enable(false);
	m_pUnderBarGift->Enable(false);

	WSendPacket packet(0xc2);
	packet.Encode1(1);
	packet.Encode4(CSharedDoc::Instance()->m_userInfo[1].info.dwUID);
	packet.EncodeStr(CSharedDoc::Instance()->m_userInfo[1].info.sNick);
	packet.Encode1(m_ghostSlot);
	packet.Send(TO_GAME);
}

void CGhostMain::OnInitStart(int param)
{
}

void CGhostMain::OnDownStart()
{
	if (IsFocusToMain())
	{
		CGhostDocument* pDoc = CGhostManager::Instance()->GetDocument();
		if (pDoc->IsInvalidate())
		{
			CGhostHandler::Instance()->OpenNotifyDlg(8);
			return;
		}

		std::list<sSlotInfo>& slotList = CSharedDoc::Instance()->m_slotList;
		slotList.clear();

		// HACK: the original zero-fills both slots without running sSlotInfo's
		// constructor
		char userBuf[sizeof(sSlotInfo)];
		char ghostBuf[sizeof(sSlotInfo)];
		sSlotInfo& userSlot = *(sSlotInfo*)userBuf;
		sSlotInfo& ghostSlot = *(sSlotInfo*)ghostBuf;
		memset(&userSlot, 0, sizeof(sSlotInfo));
		memset(&ghostSlot, 0, sizeof(sSlotInfo));

		userSlot.connectionRank = 1;
		userSlot.level = CSharedDoc::Instance()->m_myInfo.stat.Level;
		userSlot.dwGuid = CSharedDoc::Instance()->m_myInfo.info.dwGuid;
		strcpy(userSlot.sNick, CSharedDoc::Instance()->m_myInfo.info.sNick);
		slotList.push_back(userSlot);

		type_convert(pDoc->GetUserData(), &ghostSlot);
		slotList.push_back(ghostSlot);

		const GHOST_GAME_DATA* pGame = pDoc->GetGameData();
		SetCurMap(pGame->m_map);

		CSharedDoc::Instance()->m_golfGame.gameType = 0;
		strcpy(CSharedDoc::Instance()->m_golfGame.gameTypeName,
			CSharedDoc::Instance()->m_gameTypeInfo[0].name);
		CSharedDoc::Instance()->m_holeType = pGame->m_holeType;
		CSharedDoc::Instance()->m_golfGame.holes = 18;
		CSharedDoc::Instance()->m_golfGame.shotTimeLimit = 0;
		CSharedDoc::Instance()->m_golfGame.gameTimeLimit = 0;

		for (int i = 0; i < 18; i++)
		{
			CSharedDoc::Instance()->m_holeRandom[i] = pGame->m_holeScore[i];
			CSharedDoc::Instance()->m_courseMap[i] =
				(unsigned char)(CSharedDoc::Instance()->m_holeRandom[i] % 3);
			CSharedDoc::Instance()->m_holeOrder[i] = pGame->m_holePar[i];
		}

		CMouseCursor::Instance()->SetMode(1);
		CMouseCursor::Instance()->SetActive(false);
		CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);

		WSendPacket packet(0xc2);
		packet.Encode1(2);
		packet.Send(TO_GAME);
	}
}

void CGhostMain::OnInitPlayerZone(int param)
{
	m_areas[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);

	FrArea* pArea = m_areas[0];
	pArea->SetVisible(true);

	m_pUserPet = MakePet(pArea, m_pUserParts, 0);
}

void CGhostMain::OnDrawPlayerZone()
{
	FrWnd* pShop =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop");
	if (pShop != NULL || IS_KINDOF(FrBongdariShopDlg, pShop) ||
		IS_KINDOF(FrRecycleDlg, pShop))
		return;

	if (m_hand.IsWorking(0))
		return;
	if (CRankingInfo::Instance()->GetDlg() == NULL &&
		(CUserInfo::Instance() == NULL ||
			CUserInfo::Instance()->GetDlg() == NULL))
	{
		m_hand.Enter();
		if (m_pUserPet)
			m_pUserPet->Display(0.0f, 0.0f);
		m_hand.Leave();
	}
}

void CGhostMain::OnInitSelectCourse(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	CGhostDocument* pDoc = CGhostManager::Instance()->GetDocument();
	if (pDoc == NULL)
		return;

	IFF_STRUCT::sCourse* pCourse =
		ItemManager()->FindCourse(pDoc->GetGameData()->m_map | 0x28000000);
	if (pCourse == NULL)
		return;

	std::string name("mid_");
	name += pCourse->c.Icon;
	m_pCourseBitmap = g_pFresh->GetBitmap(name.c_str());

	m_courseRect = pButton->GetRect();
	m_courseRect.x += (float)abs(
		(int)(((float)m_pCourseBitmap->Width() - m_courseRect.w) * 0.5f));
	m_courseRect.y += (float)abs(
		(int)(((float)m_pCourseBitmap->Height() - m_courseRect.h) * 0.5f));
	m_courseRect.w = (float)m_pCourseBitmap->Width();
	m_courseRect.h = (float)m_pCourseBitmap->Height();
}

void CGhostMain::OnDownSelectCourse()
{
}

void CGhostMain::OnInitUserEquip(int param)
{
	m_pUserEquip = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CGhostMain::OnDrawUserEquip(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL || pGDI == NULL)
		return;

	const Bitmap* pIcon =
		g_pFresh->GetBitmap(((std::string*)pItem->pData)->c_str());
	if (pIcon == NULL)
		return;

	WRect rect(pItem->pos.x, pItem->pos.y, (float)pIcon->Width(),
		(float)pIcon->Height());

	pGDI->DrawTexture(pIcon, rect,
		((unsigned char)m_equipAlpha << 24) | 0xffffff, 0);

	if (rect.IsInRect(
			WPoint(g_input->GetMousePoint().x, g_input->GetMousePoint().y)))
	{
		const Bitmap* pFrame = g_pFresh->GetBitmap("select_frame");
		if (pFrame)
			pGDI->DrawTexture(pFrame,
				WRect(rect.x - 3.0f, rect.y - 3.0f, rect.w + 6.0f,
					rect.h + 6.0f),
				0xffffffff, 0);
	}
}

void CGhostMain::OnDownUserEquip()
{
	if (!m_bShowStat)
		return;

	FrListItem* pItem = m_pUserEquip->GetSelected();
	if (pItem == NULL)
		return;

	if (pItem->idx == 5)
	{
		FrEquipDlg* pDlg =
			CreateForm<FrEquipDlg>(g_pFresh->GetManager(), this, "equip", NULL);
		if (pDlg)
		{
			pDlg->Init(CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot,
				true);

			WRect rect(pDlg->GetRect());
			rect.x = g_view->GetWidth() - rect.w - 17.0f;
			rect.y = (g_view->GetHeight() - rect.h) * 0.5f;
			pDlg->SetRect(rect);
			pDlg->EnableDrag(false);
			pDlg->Open((FRESH_PFN_RESULT)&CGhostMain::CallBackEquipItemResult,
				3);
		}
		return;
	}

	bool bEnable = true;
	switch (pItem->idx)
	{
	case 4:
		if (CSharedDoc::Instance()->m_mascotMap.size() == 0)
			bEnable = false;
		break;
	case 1:
		if (CSharedDoc::Instance()->m_caddieMap.size() == 0)
			bEnable = false;
		break;
	}
	if (!bEnable)
		return;

	FrSubEquipBar* pBar = CreateForm<FrSubEquipBar>(g_pFresh->GetManager(),
		this, "EQUIPBAR_SUB", NULL);
	if (pBar)
	{
		m_pSelectedIcon = pItem;

		FrSubEquipBar::eCategory category = FrSubEquipBar::CATEGORY_CHARACTER;
		WPoint pos = pItem->pos;
		switch (pItem->idx)
		{
		case 4:
			category = FrSubEquipBar::CATEGORY_MASCOT;
			break;
		case 1:
			category = FrSubEquipBar::CATEGORY_CADDIE;
			break;
		case 0:
			category = FrSubEquipBar::CATEGORY_CHARACTER;
			break;
		case 2:
			category = FrSubEquipBar::CATEGORY_CLUB;
			break;
		case 3:
			category = FrSubEquipBar::CATEGORY_BALL;
			break;
		}

		pBar->Open((FRESH_PFN_RESULT)&CGhostMain::OnResultSubEquipBar, category,
			pos, (FrSubEquipBar::eSubType)1);

		WRect rect(pBar->GetRect());
		rect.y -= rect.h;
		if (rect.x + rect.w + 5.0f > g_view->GetWidth())
			rect.x = g_view->GetWidth() - rect.w - 5.0f;
		pBar->SetRect(rect);
	}
}

bool CGhostMain::OnResultSubEquipBar(int result, FrForm* pForm)
{
	// JP 983 008257c0 (B1215): = 983; nome do icone em *(this+0x3f8)->pData
	C_ASSERT_VC10(offsetof(CGhostMain, m_pSelectedIcon) == 0x3f8);
	if (result == 1)
	{
		FrSubEquipBar* pBar = (FrSubEquipBar*)pForm;
		IFF_ITEM_COMMON* pItem = pBar->GetResult();
		if (pItem != NULL)
		{
			unsigned long typeId = pItem->TypeId;
			unsigned long group = typeId >> 26;
			if (group == 5)
				SendPacketEquipResult((PANGYA_ITEM_GROUP)group, typeId);
			else
				SendPacketEquipResult((PANGYA_ITEM_GROUP)group,
					pBar->GetLastSelectedGuid());
		}
		else if (pBar->GetCategory() == FrSubEquipBar::CATEGORY_CADDIE)
		{
			// JP 983 (008257c0; PKT lote 31): pelo SendPacketEquipResult (00822800), como os outros ramos
			SendPacketEquipResult((PANGYA_ITEM_GROUP)7, 0);
			unsigned long none = 0;
			*(std::string*)m_pSelectedIcon->pData = GetIconName(none);
		}
		else if (pBar->GetCategory() == FrSubEquipBar::CATEGORY_MASCOT)
		{
			// JP 983 (008257c0; PKT lote 31): pelo SendPacketEquipResult (00822800), como os outros ramos
			SendPacketEquipResult((PANGYA_ITEM_GROUP)0x10, 0);
			unsigned long none = 0;
			*(std::string*)m_pSelectedIcon->pData = GetIconName(none);
		}
	}
	return true;
}

void CGhostMain::SetUserEquip(PANGYA_ITEM_GROUP group, unsigned long id)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	sUserInfo* pInfo = &pDoc->m_userInfo[0];

	switch (group)
	{
	case 1:
	{
		std::map<unsigned int, sCharacterInfo>::iterator it =
			pDoc->m_charMap.find(id);
		if (it == pDoc->m_charMap.end())
			return;

		pDoc->m_myInfo.userEquip.guidChar = id;
		CSharedDoc::Instance()->BuildMyPartTidList(it->second);
		pInfo->userEquip.guidChar = id;
		pInfo->charInfo = it->second;
		UserPetRebuild();

		if (m_pUserStat)
		{
			delete m_pUserStat;
			m_pUserStat = NULL;
		}
		m_pUserStat = BuildStatusBar(0);
		m_userIcons[0] = GetIconName(pInfo->charInfo.tid);
		break;
	}

	case 7:
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			pDoc->m_caddieMap.find(id);
		if (it == pDoc->m_caddieMap.end())
		{
			pDoc->m_myInfo.userEquip.guidCaddie = 0;
			memset(&pInfo->caddieInfo, 0, sizeof(pInfo->caddieInfo));
		}
		else
		{
			pDoc->m_myInfo.userEquip.guidCaddie = id;
			pInfo->userEquip.guidCaddie = id;
			pInfo->caddieInfo = it->second;
		}

		if (m_pUserStat)
		{
			delete m_pUserStat;
			m_pUserStat = NULL;
		}
		m_pUserStat = BuildStatusBar(0);
		m_userIcons[1] = GetIconName(pInfo->caddieInfo.tid);
		break;
	}

	case 4:
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			pDoc->m_clubSetMap.find(id);
		if (it == pDoc->m_clubSetMap.end())
			return;

		pDoc->m_myInfo.userEquip.guidClubSet = id;
		pInfo->userEquip.guidClubSet = id;
		pInfo->clubInfo.guid = it->second.guid;
		memcpy(pInfo->clubInfo.PCL, it->second.Common,
			sizeof(pInfo->clubInfo.PCL));
		pInfo->clubInfo.tid = it->second.tid;

		if (m_pUserStat)
		{
			delete m_pUserStat;
			m_pUserStat = NULL;
		}
		m_pUserStat = BuildStatusBar(0);
		m_userIcons[2] = GetIconName(pInfo->clubInfo.tid);
		break;
	}

	case 5:
		pDoc->m_myInfo.userEquip.tidBall = id;
		pInfo->userEquip.tidBall = id;
		m_userIcons[3] = GetIconName(id);
		break;

	case 16:
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			pDoc->m_mascotMap.find(id);
		if (it == pDoc->m_mascotMap.end())
		{
			pDoc->m_myInfo.userEquip.guidMascot = 0;
			memset(&pInfo->mascotInfo, 0, sizeof(pInfo->mascotInfo));
		}
		else
		{
			pDoc->m_myInfo.userEquip.guidMascot = id;
			pInfo->userEquip.guidMascot = id;
			pInfo->mascotInfo = it->second;
		}

		if (m_pUserStat)
		{
			delete m_pUserStat;
			m_pUserStat = NULL;
		}
		m_pUserStat = BuildStatusBar(0);
		m_userIcons[4] = GetIconName(pInfo->mascotInfo.tid);
		break;
	}
	}
}

void CGhostMain::UserPetRebuild()
{
	if (m_pUserPet)
	{
		delete m_pUserPet;
		m_pUserPet = NULL;
	}
	if (m_pUserParts)
	{
		delete m_pUserParts;
		m_pUserParts = NULL;
	}

	FrArea* pArea = m_areas[0];
	m_pUserParts = new CPartTidList;
	m_pUserPet = MakePet(pArea, m_pUserParts, 0);
}

bool CGhostMain::CallBackEquipItemResult(int result, FrForm* pForm)
{
	if (0)
		((CGhostMain*)0)
			->CGhostMain::OnResultSubEquipBar(*(int*)0, *(FrForm**)0);
	if (pForm)
	{
		m_pEquipDlg = pForm;
		unsigned long* pItemSlot =
			CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot;
		memcpy(pItemSlot, ((FrEquipDlg*)pForm)->GetMyItemSlot(),
			sizeof(CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot));
		memcpy(CSharedDoc::Instance()->m_userInfo[0].userEquip.tidItemSlot,
			pItemSlot,
			sizeof(CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot));
	}

	m_pEquipDlg = NULL;
	HandleMsg(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));

	WSendPacket packet(0x20);
	packet.Encode1(2);
	packet.EncodeBuffer(CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot,
		sizeof(CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot));
	packet.Send(TO_GAME);

	return true;
}

bool CGhostMain::IsFocusToMain()
{
	if (!g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop"))
	{
		__rtti_obj = NULL;
		return true;
	}

	return false;
}

void CGhostMain::OnInitGhostEquip(int param)
{
	m_pGhostEquip = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CGhostMain::OnDrawGhostEquip(int param)
{
	if (!m_bShowStat)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL || pGDI == NULL)
		return;

	const Bitmap* pIcon =
		g_pFresh->GetBitmap(((std::string*)pItem->pData)->c_str());
	const Bitmap* pHide = g_pFresh->GetBitmap("ghost_equipicon_hide");
	if (pIcon)
		pGDI->DrawTexture(pIcon,
			WRect(pItem->pos.x, pItem->pos.y, (float)pIcon->Width(),
				(float)pIcon->Height()),
			((unsigned char)m_equipAlpha << 24) | 0xffffff, 0);

	// the hide mask is drawn with the icon's size, as in the original
	if (pHide)
		pGDI->DrawTexture(pHide,
			WRect(pItem->pos.x, pItem->pos.y, (float)pIcon->Width(),
				(float)pIcon->Height()),
			((unsigned char)m_equipAlpha << 24) | 0xffffff, 0);

	m_pTitleFont->Print(g_view, m_courseRect.x + 40.0f, m_courseRect.y - 40.0f,
		MakeStr(GHOST_FMT_MINUS_D, m_ghostScore), 0,
		((unsigned char)m_equipAlpha << 24) | 0xffffff, NULL);
	float width = m_pTitleFont->Print(g_view, m_courseRect.x + 40.0f,
		m_courseRect.y - 25.0f, MakeStr("%d", m_ghostPang), 0,
		((unsigned char)m_equipAlpha << 24) | 0xffffff, NULL); // 983 00823e6e

	const Bitmap* pPang = g_pFresh->GetBitmap("ghost_pang");
	if (pPang)
		pGDI->DrawTexture(pPang,
			WRect(width + m_courseRect.x + 50.0f, m_courseRect.y - 25.0f,
				(float)pPang->Width(), (float)pPang->Height()),
			((unsigned char)m_equipAlpha << 24) | 0xffffff, 0);

	if (m_pCourseBitmap)
		pGDI->DrawTexture(m_pCourseBitmap, m_courseRect, 0xffffffff, 0);
}

void CGhostMain::OnInitGhostZone(int param)
{
	m_areas[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);

	FrArea* pArea = m_areas[1];
	pArea->SetVisible(true);
	m_pGhostPet = MakePet(pArea, m_pGhostParts, 1);
	m_pGhostPet->SetAlpha(0);
}

void CGhostMain::OnDrawGhostZone()
{
	if (IS_KINDOF(FrBongdariShopDlg,
			g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"bongdari_shop")) ||
		IS_KINDOF(FrRecycleDlg,
			g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"recycledlg")) ||
		IS_KINDOF(FrEquipDlg,
			g_pFresh->GetManager()->GetDesktop()->FindChildByName("equip")))
		return;

	if (m_hand.IsWorking(0))
		return;
	if (CRankingInfo::Instance()->GetDlg() == NULL &&
		(CUserInfo::Instance() == NULL ||
			CUserInfo::Instance()->GetDlg() == NULL))
	{
		m_hand.Enter();
		if (m_pGhostPet && m_bGhostVisible)
			m_pGhostPet->Display(0.0f, 0.0f);
		m_hand.Leave();
	}
}

void CGhostMain::OnInitTipZone(int param)
{
	m_areas[4] = DYNAMIC_CAST(FrArea, (FrWnd*)param);

	m_tipAlpha = 0.0f;
}

void CGhostMain::OnDrawTipZone()
{
	m_pTipImage->Render((unsigned char)m_tipAlpha);
}

void CGhostMain::OnProcTip(float dt)
{
	// JP 983 00824490 (B1215): mesma logica; offsets dos membros
	C_ASSERT_VC10(offsetof(CGhostMain, m_bShowStat) == 0x3e4);
	C_ASSERT_VC10(offsetof(CGhostMain, m_tipAlpha) == 0x460);
	C_ASSERT_VC10(offsetof(CGhostMain, m_bTipHide) == 0x464);
	C_ASSERT_VC10(offsetof(CGhostMain, m_tipTime) == 0x468);
	if (!m_bShowStat)
		return;

	float y = m_pTipImage->GetPos().y;
	if (!m_bTipHide)
	{
		if (y < 30.0f)
			m_pTipImage->AddPos(WPoint(0.0f, dt * 240.0f));

		if (m_tipAlpha < 240.0f)
		{
			m_tipAlpha += dt * 240.0f;
			if (m_tipAlpha > 240.0f)
				m_tipAlpha = 240.0f;
		}

		if (m_tipAlpha == 240.0f && m_pTipImage->GetPos().y >= 30.0f)
		{
			m_tipTime -= dt;
			if (m_tipTime < 0.0f)
			{
				m_tipTime = 5.0f;
				m_bTipHide = true;
			}
		}
	}
	else
	{
		if (y > -120.0f)
			m_pTipImage->AddPos(WPoint(0.0f, dt * -240.0f));

		if (m_tipAlpha > 0.0f)
		{
			m_tipAlpha -= dt * 0.0f;
			if (m_tipAlpha < 0.0f)
				m_tipAlpha = 0.0f;
		}
	}
}

void CGhostMain::OnInitAreaUserStat(int param)
{
	m_areas[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CGhostMain::OnDrawAreaUserStat()
{
	m_pUserStatImage->Render((unsigned char)m_userStatAlpha);
	if (!m_bShowStat)
		return;

	WRect rect = m_areas[2]->GetRect();
	m_pUserStat->Render(WPoint(rect.x + 86.0f, rect.y + 86.0f));

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	CSharedDoc* pDoc = CSharedDoc::Instance();
	int level = pDoc->m_userInfo[0].stat.Level + 1;
	const Bitmap* pBitmap =
		g_pFresh->GetBitmap(MakeStr("level_%03d", level < 0x47 ? level : 0x47));
	if (pBitmap == NULL)
		return;

	pGDI->DrawTexture(pBitmap,
		WRect(rect.x + 30.0f, rect.y + 3.0f, (float)pBitmap->Width(),
			(float)pBitmap->Height()),
		0xffffffff, 0);

	pGDI->SetTextColor(0xffffffff, 0xff000000);
	pGDI->SetTextStyle(2);

	g_pFresh->GetManager()->PrintText(
		WPoint((float)pBitmap->Width() + rect.x + 35.0f, rect.y + 5.0f), 0,
		CSharedDoc::Instance()->m_userInfo[0].info.sNick, -1.0f, 0xffffffff);
	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xffffffff, 0xffffffff);
}

void CGhostMain::OnInitAreaGhostStat(int param)
{
	m_areas[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CGhostMain::OnDrawAreaGhostStat()
{
	m_pGhostStatImage->Render((unsigned char)m_userStatAlpha);
	if (g_pFresh->GetManager()->GetGDI() == NULL || !m_bShowStat)
		return;

	WRect rect = m_areas[3]->GetRect();
	m_pGhostStat->Render(WPoint(rect.x + 210.0f, rect.y + 86.0f));

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	CSharedDoc* pDoc = CSharedDoc::Instance();
	int level = pDoc->m_userInfo[1].stat.Level + 1;
	const Bitmap* pBitmap =
		g_pFresh->GetBitmap(MakeStr("level_%03d", level < 0x47 ? level : 0x47));
	if (pBitmap == NULL)
		return;

	pGDI->DrawTexture(pBitmap,
		WRect(rect.x + 230.0f, rect.y + 3.0f, (float)pBitmap->Width(),
			(float)pBitmap->Height()),
		0xffffffff, 0);

	pGDI->SetTextColor(0xffffffff, 0xff000000);
	pGDI->SetTextStyle(2);

	g_pFresh->GetManager()->PrintText(
		WPoint((float)pBitmap->Width() + rect.x + 235.0f, rect.y + 5.0f), 0,
		CSharedDoc::Instance()->m_userInfo[1].info.sNick, -1.0f, 0xffffffff);
	pGDI->SetTextStyle(0);
	pGDI->SetTextColor(0xffffffff, 0xffffffff);
}

void CGhostMain::OnInitGhostMain(int param)
{
	TurnOff3DBackground();
}

void CGhostMain::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0:
		if (g_pFresh == NULL)
			return;

		{
			const char* pLayout = (const char*)msg.param1;
			g_pFresh->OpenLayout(pLayout, this, true);
			switch (msg.param2)
			{
			case 1:
				CGhostHandler::Instance()->OpenNotifyDlg(5);
				break;
			}
			break;
		}

	case M983(0x49):
	{
		const sCharacterInfo* pChar = (const sCharacterInfo*)msg.param2;
		SetUserEquip((PANGYA_ITEM_GROUP)1, pChar->guid);
		break;
	}

	case M983(0x4a):
	{
		const sCaddieInfo* pCaddie = (const sCaddieInfo*)msg.param2;
		SetUserEquip((PANGYA_ITEM_GROUP)7, pCaddie->guid);
		break;
	}

	case M983(0x4b):
	{
		const sItemInfo* pClubSet = (const sItemInfo*)msg.param2;
		SetUserEquip((PANGYA_ITEM_GROUP)4, pClubSet->guid);
		break;
	}

	case M983(0x4c):
		SetUserEquip((PANGYA_ITEM_GROUP)5, msg.param2);
		break;

	case M983(0x4e):
	{
		const sMascotInfo* pMascot = (const sMascotInfo*)msg.param2;
		if (REBANG_983_PCBANG_MASCOT(*pMascot))
			COption::Instance()->mSetPCBangMascotMsg(pMascot->szMsg);
		SetUserEquip((PANGYA_ITEM_GROUP)16, pMascot->guid);
		break;
	}

	default:
		HandleMsgCommon(msg);
		break;
	}
}

void CGhostMain::SendPacketEquipResult(PANGYA_ITEM_GROUP group,
	unsigned long id)
{
	WSendPacket packet(0x0b);
	unsigned char kind = 0;
	switch (group)
	{
	case 1:
		kind = 4;
		break;
	case 7:
		kind = 1;
		break;
	case 4:
		kind = 3;
		break;
	case 5:
		kind = 2;
		break;
	case 16:
		kind = 5;
		break;
	}
	packet.Encode1(kind);
	packet.Encode4(id);
	packet.Send(TO_GAME);
}

// HACK: the original reads the part lists through inline accessors of CPartTidList (PCH
// inlines, not emitted); dllimport + __forceinline is expanded and never emitted out of line
__declspec(dllimport) __forceinline unsigned long GetPartTid(
	const CPartTidList* pParts, unsigned char part)
{
	return pParts->m_tid[part];
}

__declspec(dllimport) __forceinline unsigned long GetPartUid(
	const CPartTidList* pParts, unsigned char part)
{
	return pParts->m_uid[part];
}

void CGhostMain::RefreshAllUccClothes()
{
	if (0)
		((CGhostMain*)0)
			->CGhostMain::SendPacketEquipResult(*(PANGYA_ITEM_GROUP*)0,
				*(unsigned long*)0);
	sUccRefreshItem item;
	for (int i = 0; i < 10; i++)
	{
		if (!UccManager()->GetRefreshItem(item))
			return;

		if (m_pUserPet && m_pUserPet->GetPetFrame() && m_pUserParts)
		{
			CPartTidList* pParts = m_pUserParts;
			unsigned char part = (unsigned char)((item.typeId >> 13) & 0x1f);
			if (GetPartTid(pParts, part) == item.typeId)
			{
				sUccClothes* pClothes =
					UccManager()->FindClothes(GetPartUid(pParts, part));
				if (pClothes && strcmpi(pClothes->uccIndex, item.uccIndex) == 0)
				{
					// JP 983 00823070 (B1215): 1x CPetFrame::SetPart 008b0960 (so as texturas UCC)
					m_pUserPet->GetPetFrame()->SetPart983(pClothes);
				}
			}
		}

		if (m_pGhostPet && m_pGhostPet->GetPetFrame() && m_pGhostParts)
		{
			CPartTidList* pParts = m_pGhostParts;
			unsigned char part = (unsigned char)((item.typeId >> 13) & 0x1f);
			if (GetPartTid(pParts, part) == item.typeId)
			{
				sUccClothes* pClothes =
					UccManager()->FindClothes(GetPartUid(pParts, part));
				if (pClothes && strcmpi(pClothes->uccIndex, item.uccIndex) == 0)
				{
					// JP 983 00823070 (B1215): 1x CPetFrame::SetPart 008b0960 (so as texturas UCC)
					m_pGhostPet->GetPetFrame()->SetPart983(pClothes);
				}
			}
		}
	}
}
