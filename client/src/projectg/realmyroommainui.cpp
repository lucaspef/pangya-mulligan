#define REBANG_REALMYROOMMAINUI_CPP
#include "minatl.h"
#include "jp/docofs983.h" // B1460: Doc()+0x8cb -> DOCPTR983
// the inline functions of these headers open the unit's .text in this order
#include "clientsetting.h"
#include "realmyroommain.h"
#include "realmyroom_avatar.h"
#include "exhibition.h"
#include "fortunedlg.h"
#include "fortuneopendlg.h"
#include "enchantdlg.h"
#include "caddieinfodlg.h"
#include "panggiftdlg.h"
#include "ccardbook.h"
#include "post_senddlg.h"
#include "uccdrawdlg.h"
#include "itemstorage.h"
#include "s5/rentalextensiondlg.h"
#include "frrandomboxprogress.h"
#include "commonutil.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frwnd.h"
#include "fredit.h"
#include "exhibition.h"
#include "cardsystem.h"
#include "ccardbook.h"
#include "itemstorage.h"
#include "realmyroomdata.h"
#include "realmyroom_avatar.h"
#include "quadtree.h"
#include "resultdlg.h"
#include "contentsdoc.h"
#include "carddlg.h"
#include "shareddoc.h"
#include "parttidlist.h"
#include "uccdrawdlg.h"
#include "projectg.h"
#include "taskmain.h"
#include "cardremove.h"
#include "s5/utilities.h"
#include "jp/shareddocjp.h" // PKT (frente C, lote 37): IsControlServerService983 (0060d460)
#include "jp/statushelper.h"
#include "levelupitemdlg.h"
#include "guildactor.h"
#include "gatewayactor.h"
#include "s5/supplypack.h"
#include "panggiftdlg.h"
#include "../shared/globalgamedefine.h"
#include "taskmanager.h"
#include "avatar_task.h"
#include "frwndinl.h"
#include "frcontextmenuctrl.h"
#include "frgaugebar.h"
#include "frviewer.h"
#include "frtabbutton.h"
#include "frstatic.h"
#include "frlistbox.h"
#include "wbone.h"
#include "rankingdlg.h"
#include "user_info.h"
#include "frgroupbox.h"
#include "frbutton.h"
#include "frarea.h"
#include "frscrollbar.h"
#include "frform.h"
#include "frdesktop.h"
#include "packet.h"
#include "packets/realmyroompackets.h"	// PKT (frente C)
#include "wresrcmng.h"
#include "woverlay.h"
#include "tooltip.h"
#include "returngiftdlg.h"
#include "notedlg.h"
#include "post_recivedlg.h"
#include "mascotinfodlg.h"
#include "uccmanager.h"
#include "uccclothes.h"
#include "realmyroom_authoritydlg.h"
#include "fortunedlg.h"
#include "fortuneopendlg.h"
#include "downgradedlg.h"
#include "furniture_ability.h"
#include "frrandomboxprogress.h"
#include "replaymodedlg.h"
#include "deleteitem.h"
#include "ucccopydlg.h"
#include "post_senddlg.h"
#include "setitemdlg.h"
#include "frienddlg.h"
#include "mascot.h"
#include "indexcontrol.h"
extern WMatrix g_camera;
#include "enchantdlg.h"
#include "userinfoform.h"
#include "logininfo.h"
#include "clientsetting.h"
#include "taskmanager.h"
#include "caddieinfodlg.h"
#include "s5/rentalextensiondlg.h"
#include "../shared/localize.h"
#include "mousecursor.h"
#include "codetinker.h"
#include "jp/clubsetresetwnd.h" // B89: reset do taco (FrClubSetResetWnd, frente B)
#include "vc10_layout.h"

// A731: crus do 983 (VC10) -> membros; o cl novo desloca o bloco 0x1xxxx do CSharedDoc
C_ASSERT_VC10(offsetof(CSharedDoc, m_replayTargetUID) == 0x10708 && offsetof(CSharedDoc, m_rmrAuthority) == 0x1070c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_realEquip) == 0x1103c && offsetof(CSharedDoc, m_bItemStoragePin) == 0x110c8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip) == 0xa26 && offsetof(CSharedDoc, m_myInfo.userEquip.tidSkin) == 0xa76 && offsetof(CSharedDoc, m_myInfo.userEquip.guidMascot) == 0xa8e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_ballList) == 0x3750 && offsetof(CSharedDoc, m_cardAbilityList) == 0x37ec); // A758: ex-Doc()+0x3750/+0x37ec (cabeÃ§a da std::list no +0 nos 2 compiladores)
C_ASSERT(sizeof(sRealMyRoomAuthority) == 0x6b);


// the unit starts its .rdata with this pair (no symbol in the original; same pair as shoptask.cpp/hatmanager.cpp)
extern const float g_realMyRoomMainUiFloatPair[2] = { 3.402823466e+38f,
	1.0e-5f };

std::string __fastcall GenerateIconFilename(unsigned long typeId,
	const char* uccIndex);

int float2int(float f);

extern Fresh* g_pFresh;

void __fastcall ReviseItemIconPoint(unsigned long tid, WPoint& pt);
void __fastcall SetMainTabImg(const char** const names, int index);
void __fastcall SetSubTabImg(const char** const names, int index);
bool __fastcall EmptyCompares(const void* a, const void* b);

namespace S5
{
	bool __fastcall UnderCategory(unsigned long tidChar, unsigned long tid,
		unsigned long flag);
	bool __fastcall ISCutinItem(unsigned long tid);
	namespace UTIL
	{
		int __fastcall GetEquipSkinIndex(eEquipSkin& index, unsigned long tid);
	}
}
bool __fastcall IsAngelWing(unsigned long tid);

void __fastcall ReviseItemIconPoint(unsigned long tid, WPoint& pt)
{
	float x;
	switch (tid >> 26)
	{
	case 1:
	case 2:
	case 8:
	case 9:
	case 15:
	case 16:
	case 28:
		pt.x += 7.0f;
		pt.y += 2.0f;
		return;
	default:
		x = pt.x;
		break;
	case 11:
		pt.x += 9.0f;
		pt.y += 12.0f;
		return;
	case 14:
		x = pt.x;
		if ((tid & 0x3c00000) == 0x800000)
		{
			pt.x = x + 7.0f;
			pt.y += 12.0f;
			return;
		}
		break;
	case 31:
		pt.x += 16.0f;
		pt.y += 12.0f;
		return;
	case 18:
		pt.x += 21.0f;
		pt.y += 12.0f;
		return;
	}
	pt.x = x + 16.0f;
	pt.y += 27.0f;
}

BEGIN_FRESH_MSGMAP(CRealMyRoomMain, CTaskMain)
ON_FRESH_VI("REALMYROOM", FRCMD_INIT, CRealMyRoomMain::OnMyRoomInit)
ON_FRESH_VV("REALMYROOM", FRCMD_FINISH, CRealMyRoomMain::OnMyRoomFinish)
ON_FRESH_VI("REALMYROOMVISITOR", FRCMD_INIT, CRealMyRoomMain::OnMyRoomInit)
ON_FRESH_VV("REALMYROOMVISITOR", FRCMD_FINISH, CRealMyRoomMain::OnMyRoomFinish)
ON_FRESH_VI("REALMYROOM.obar", FRCMD_INIT, CTaskMain::OnOverBar_Init)
ON_FRESH_VI("REALMYROOM.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)
ON_FRESH_VI("REALMYROOM.ubar", FRCMD_INIT, CTaskMain::OnUnderBarInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar", FRCMD_INIT, CTaskMain::OnOverBar_Init)
ON_FRESH_VI("REALMYROOMVISITOR.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)
ON_FRESH_VI("REALMYROOMVISITOR.ubar", FRCMD_INIT, CTaskMain::OnUnderBarInit)
ON_FRESH_VI("REALMYROOM.middle_ui.item_list", FRCMD_INIT,
	CRealMyRoomMain::OnItemListInit)
ON_FRESH_VI("REALMYROOM.middle_ui.item_list", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnItemListOwnerDraw)
ON_FRESH_VV("REALMYROOM.middle_ui.item_list", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnItemListBtnDown)
ON_FRESH_VV("REALMYROOM.middle_ui.item_list", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnItemListRBtnDown)
ON_FRESH_VV("REALMYROOM.middle_ui.item_list", FRCMD_MOUSEMOVE,
	CRealMyRoomMain::OnItemListMouseMove)
ON_FRESH_VV("REALMYROOM.middle_ui.item_list", FRCMD_DBLCLICK,
	CRealMyRoomMain::OnItemListDClick)
ON_FRESH_VI("REALMYROOM.middle_ui.tab_main", FRCMD_INIT,
	CRealMyRoomMain::OnTabMainInit)
ON_FRESH_VV("REALMYROOM.middle_ui.tab_main", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnTabMainButtonUp)
ON_FRESH_VI("REALMYROOM.middle_ui.tab_sub", FRCMD_INIT,
	CRealMyRoomMain::OnTabSubInit)
ON_FRESH_VV("REALMYROOM.middle_ui.tab_sub", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnTabSubButtonUp)
ON_FRESH_VI("REALMYROOM.middle_ui.gift_page", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnGiftPageOwnerDraw)
ON_FRESH_VV("REALMYROOM.middle_ui.click_area_list", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnGiftPageLBDown)
ON_FRESH_VI("REALMYROOM.middle_ui", FRCMD_INIT, CRealMyRoomMain::OnMiddleUIInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui", FRCMD_INIT,
	CRealMyRoomMain::OnMiddleUIInit)
ON_FRESH_VI("REALMYROOM.middle_ui.tabtxt1", FRCMD_INIT,
	CRealMyRoomMain::OnTabTxt1Init)
ON_FRESH_VI("REALMYROOM.middle_ui.tabtxt2", FRCMD_INIT,
	CRealMyRoomMain::OnTabTxt2Init)
ON_FRESH_VI("REALMYROOM.middle_ui.tabtxt2", FRCMD_INIT,
	CRealMyRoomMain::OnTabTxt3Init)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selected_name", FRCMD_INIT,
	CRealMyRoomMain::OnSelectedNameInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.tabtxt1", FRCMD_INIT,
	CRealMyRoomMain::OnTabTxt1Init)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.tabtxt2", FRCMD_INIT,
	CRealMyRoomMain::OnTabTxt2Init)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.tabtxt2", FRCMD_INIT,
	CRealMyRoomMain::OnTabTxt3Init)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selected_name", FRCMD_INIT,
	CRealMyRoomMain::OnSelectedNameInit)
ON_FRESH_VI("REALMYROOM.middle_ui.pet", FRCMD_INIT, CRealMyRoomMain::OnPetInit)
ON_FRESH_VI("REALMYROOM.middle_ui.pet", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnPetOwnerDraw)
ON_FRESH_VI("REALMYROOM.middle_ui.skin_pet", FRCMD_INIT,
	CRealMyRoomMain::OnSkinPetInit)
ON_FRESH_VI("REALMYROOM.middle_ui.skin_pet2", FRCMD_INIT,
	CRealMyRoomMain::OnSkinPet2Init)
ON_FRESH_VI("REALMYROOM.middle_ui.item_desc_bg", FRCMD_INIT,
	CRealMyRoomMain::OnItemDescBgInit)
ON_FRESH_VI("REALMYROOM.middle_ui.item_desc_bg", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnItemDescBgOwnerDraw)
ON_FRESH_VI("REALMYROOM.middle_ui.item_desc", FRCMD_INIT,
	CRealMyRoomMain::OnItemDescInit)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox", FRCMD_INIT,
	CRealMyRoomMain::OnItemSlotBoxInit)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.equip_cancel", FRCMD_INIT,
	CRealMyRoomMain::OnEquipCancelInit)
ON_FRESH_VV("REALMYROOM.middle_ui.sbox.equip_cancel", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnEquipCancelUp)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.equip_cancel", FRCMD_HOVERON,
	CRealMyRoomMain::OnTooltipHoverOn)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.equip_cancel", FRCMD_HOVEROFF,
	CRealMyRoomMain::OnTooltipHoverOff)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.equip_reset", FRCMD_INIT,
	CRealMyRoomMain::OnEquipResetInit)
ON_FRESH_VV("REALMYROOM.middle_ui.sbox.equip_reset", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnEquipResetUp)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.equip_reset", FRCMD_HOVERON,
	CRealMyRoomMain::OnTooltipHoverOn)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.equip_reset", FRCMD_HOVEROFF,
	CRealMyRoomMain::OnTooltipHoverOff)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.item_slot", FRCMD_INIT,
	CRealMyRoomMain::OnItemSlotInit)
ON_FRESH_VI("REALMYROOM.middle_ui.sbox.item_slot", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnItemSlotOwnerDraw)
ON_FRESH_VV("REALMYROOM.middle_ui.sbox.item_slot", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnItemSlotBtnDown)
ON_FRESH_VV("REALMYROOM.middle_ui.sbox.item_slot", FRCMD_DBLCLICK,
	CRealMyRoomMain::OnItemSlotDClick)
ON_FRESH_VI("REALMYROOM.middle_ui.gift_bg", FRCMD_INIT,
	CRealMyRoomMain::OnGiftBgInit)
ON_FRESH_VI("REALMYROOM.middle_ui.gift_desc", FRCMD_INIT,
	CRealMyRoomMain::OnGiftDescInit)
ON_FRESH_VI("REALMYROOM.middle_ui.furniture_arrange", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureArrangeInit)
ON_FRESH_VV("REALMYROOM.middle_ui.furniture_arrange", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurnitureArrangeButtonUP)
ON_FRESH_VI("REALMYROOM.middle_ui.st", FRCMD_INIT,
	CRealMyRoomMain::OnStatBoxInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selframe", FRCMD_INIT,
	CRealMyRoomMain::OnSelFrameInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selitemicon", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemIconInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selitem", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemInit)
ON_FRESH_VV("REALMYROOM.middle_ui.st.selitem", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnSelItemUp)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selitem", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnSelItemOwnerDraw)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selitemicon", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnSelItemIconOwnerDraw)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selitem_prev", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemPrevInit)
ON_FRESH_VV("REALMYROOM.middle_ui.st.selitem_prev", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnSelItemPrevUp)
ON_FRESH_VI("REALMYROOM.middle_ui.st.selitem_next", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemNextInit)
ON_FRESH_VV("REALMYROOM.middle_ui.st.selitem_next", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnSelItemNextUp)
ON_FRESH_VI("REALMYROOM.middle_ui.st.stat_part", FRCMD_INIT,
	CRealMyRoomMain::OnStatPartInit)
ON_FRESH_VV("REALMYROOM.middle_ui.st.stat_part", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnStatPartUp)
ON_FRESH_VI("REALMYROOM.middle_ui.st.stat_part", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnStatPartOwnerDraw)
ON_FRESH_VI("REALMYROOM.middle_ui.st.stat_all", FRCMD_INIT,
	CRealMyRoomMain::OnStatAllInit)
ON_FRESH_VV("REALMYROOM.middle_ui.st.stat_all", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnStatAllUp)
ON_FRESH_VI("REALMYROOM.middle_ui.st.stat_all", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnStatAllOwnerDraw)
ON_FRESH_VI("REALMYROOM.middle_ui.st.stat_frame", FRCMD_INIT,
	CRealMyRoomMain::OnStatFrameInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.stat_text", FRCMD_INIT,
	CRealMyRoomMain::OnStatTextInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.powerbar", FRCMD_INIT,
	CRealMyRoomMain::OnPowerBarInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.controlbar", FRCMD_INIT,
	CRealMyRoomMain::OnControlBarInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.impactbar", FRCMD_INIT,
	CRealMyRoomMain::OnImpactBarInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.spinbar", FRCMD_INIT,
	CRealMyRoomMain::OnSpinBarInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.curvebar", FRCMD_INIT,
	CRealMyRoomMain::OnCurveBarInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.epower", FRCMD_INIT,
	CRealMyRoomMain::OnPowerEditInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.econtrol", FRCMD_INIT,
	CRealMyRoomMain::OnControlEditInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.eimpact", FRCMD_INIT,
	CRealMyRoomMain::OnImpactEditInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.espin", FRCMD_INIT,
	CRealMyRoomMain::OnSpinEditInit)
ON_FRESH_VI("REALMYROOM.middle_ui.st.ecurve", FRCMD_INIT,
	CRealMyRoomMain::OnCurveEditInit)
ON_FRESH_VI("REALMYROOM.middle_ui.upgrade", FRCMD_INIT,
	CRealMyRoomMain::OnUpgradeInit)
ON_FRESH_VV("REALMYROOM.middle_ui.upgrade", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUpgradeUp)
ON_FRESH_VI("REALMYROOM.middle_ui.upgrade", FRCMD_HOVERON,
	CRealMyRoomMain::OnTooltipHoverOn)
ON_FRESH_VI("REALMYROOM.middle_ui.upgrade", FRCMD_HOVEROFF,
	CRealMyRoomMain::OnTooltipHoverOff)
ON_FRESH_VI("REALMYROOM.middle_ui.downgrade", FRCMD_INIT,
	CRealMyRoomMain::OnDowngradeInit)
ON_FRESH_VV("REALMYROOM.middle_ui.downgrade", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnDowngradeUp)
ON_FRESH_VI("REALMYROOM.middle_ui.downgrade", FRCMD_HOVERON,
	CRealMyRoomMain::OnTooltipHoverOn)
ON_FRESH_VI("REALMYROOM.middle_ui.downgrade", FRCMD_HOVEROFF,
	CRealMyRoomMain::OnTooltipHoverOff)
ON_FRESH_VI("REALMYROOM.rmenu", FRCMD_INIT, CRealMyRoomMain::OnRightMenuInit)
ON_FRESH_VV("REALMYROOM.rmenu", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnRightMenuLBtnDown)
ON_FRESH_VV("REALMYROOM.rmenu", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnRightMenuRBtnDown)
ON_FRESH_VI("REALMYROOM.ubar.back", FRCMD_INIT, CTaskMain::OnUnderBar_BackInit)
ON_FRESH_VV("REALMYROOM.ubar.back", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUnderBar_BackUp)
ON_FRESH_VI("REALMYROOM.ubar.back", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.back", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.front", FRCMD_INIT,
	CTaskMain::OnUnderBar_FrontInit)
ON_FRESH_VV("REALMYROOM.ubar.front", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_FrontUp)
ON_FRESH_VI("REALMYROOM.ubar.front", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.front", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.shop", FRCMD_INIT, CTaskMain::OnUnderBar_ShopInit)
ON_FRESH_VV("REALMYROOM.ubar.shop", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ShopUp)
ON_FRESH_VI("REALMYROOM.ubar.shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.shop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.myroom", FRCMD_INIT,
	CTaskMain::OnUnderBar_MyRoomInit)
ON_FRESH_VV("REALMYROOM.ubar.myroom", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MyRoomUp)
ON_FRESH_VI("REALMYROOM.ubar.myroom", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.myroom", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.itemstorage", FRCMD_INIT,
	CTaskMain::OnUnderBar_ItemStorageInit)
ON_FRESH_VV("REALMYROOM.ubar.itemstorage", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUnderBar_ItemStorageUp)
ON_FRESH_VI("REALMYROOM.ubar.itemstorage", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.itemstorage", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VV("REALMYROOM.ubar.itemstorage", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUnderBar_ItemStorageUp)
ON_FRESH_VI("REALMYROOM.ubar.gift", FRCMD_INIT, CTaskMain::OnUnderBar_GiftInit)
ON_FRESH_VV("REALMYROOM.ubar.gift", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUnderBar_GiftUp)
ON_FRESH_VI("REALMYROOM.ubar.gift", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.gift", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.cookie", FRCMD_INIT,
	CTaskMain::OnUnderBar_CookieInit)
ON_FRESH_VV("REALMYROOM.ubar.cookie", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_CookieUp)
ON_FRESH_VI("REALMYROOM.ubar.cookie", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.cookie", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.game", FRCMD_INIT, CTaskMain::OnUnderBar_GameInit)
ON_FRESH_VV("REALMYROOM.ubar.game", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GameUp)
ON_FRESH_VI("REALMYROOM.ubar.game", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.game", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.server", FRCMD_INIT,
	CTaskMain::OnUnderBar_ServerInit)
ON_FRESH_VV("REALMYROOM.ubar.server", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ServerUp)
ON_FRESH_VI("REALMYROOM.ubar.server", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.server", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.exit", FRCMD_INIT, CTaskMain::OnUnderBar_ExitInit)
ON_FRESH_VV("REALMYROOM.ubar.exit", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ExitUp)
ON_FRESH_VI("REALMYROOM.ubar.exit", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.ubar.exit", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.ubar.tabbtn_b", FRCMD_INIT,
	CTaskMain::OnUnderBar_TabBtnInit)
ON_FRESH_VV("REALMYROOM.ubar.tabbtn_b", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TabBtnUp)
ON_FRESH_VI("REALMYROOM.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)
ON_FRESH_VI("REALMYROOM.utab.magicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_MagicBoxInit)
ON_FRESH_VV("REALMYROOM.utab.magicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MagicBoxUp)
ON_FRESH_VI("REALMYROOM.utab.magicbox", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.magicbox", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.utab.tikimagicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_TikiMagicBoxInit)
ON_FRESH_VV("REALMYROOM.utab.tikimagicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TikiMagicBoxUp)
ON_FRESH_VI("REALMYROOM.utab.tikimagicbox", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.tikimagicbox", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.utab.scratch", FRCMD_INIT,
	CTaskMain::OnUnderBar_ScratchInit)
ON_FRESH_VV("REALMYROOM.utab.scratch", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ScratchUp)
ON_FRESH_VI("REALMYROOM.utab.scratch", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.scratch", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.utab.bongdarishop", FRCMD_INIT,
	CTaskMain::OnUnderBar_BongdariShopInit)
ON_FRESH_VV("REALMYROOM.utab.bongdarishop", FRCMD_LBUTTONDOWN,
	CTaskMain::OnUnderBar_BongdariShopUp)
ON_FRESH_VI("REALMYROOM.utab.bongdarishop", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.bongdarishop", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.utab.ranking", FRCMD_INIT,
	CTaskMain::OnUnderBar_RankingInit)
ON_FRESH_VV("REALMYROOM.utab.ranking", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_RankingUp)
ON_FRESH_VI("REALMYROOM.utab.ranking", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.ranking", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.utab.guild", FRCMD_INIT,
	CTaskMain::OnUnderBar_GuildInit)
ON_FRESH_VV("REALMYROOM.utab.guild", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GuildUp)
ON_FRESH_VI("REALMYROOM.utab.guild", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.guild", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.utab.notice", FRCMD_INIT,
	CTaskMain::OnUnderBar_NoticeInit)
ON_FRESH_VV("REALMYROOM.utab.notice", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_NoticeUp)
ON_FRESH_VI("REALMYROOM.utab.notice", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.notice", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.utab.maket", FRCMD_INIT,
	CTaskMain::OnUnderBar_MaketInit)
ON_FRESH_VV("REALMYROOM.utab.maket", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MaketUp)
ON_FRESH_VI("REALMYROOM.utab.maket", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.utab.maket", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.middle_ui.chatbg", FRCMD_INIT,
	CRealMyRoomMain::OnEffectViewInit)
ON_FRESH_VI("REALMYROOM.middle_ui.chatbg", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnEffectViewOnwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.title", FRCMD_INIT, CTaskMain::OnOverBar_TitleInit)
ON_FRESH_VI("REALMYROOM.obar.site", FRCMD_INIT, CTaskMain::OnOverBar_SiteInit)
ON_FRESH_VI("REALMYROOM.obar.char", FRCMD_INIT, CTaskMain::OnOverBar_CharInit)
ON_FRESH_VV("REALMYROOM.obar.char", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CharDown)
ON_FRESH_VI("REALMYROOM.obar.char", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.caddie", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieInit)
ON_FRESH_VV("REALMYROOM.obar.caddie", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CaddieDown)
ON_FRESH_VI("REALMYROOM.obar.caddie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.club", FRCMD_INIT, CTaskMain::OnOverBar_ClubInit)
ON_FRESH_VV("REALMYROOM.obar.club", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_ClubDown)
ON_FRESH_VI("REALMYROOM.obar.club", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.aztec", FRCMD_INIT, CTaskMain::OnOverBar_AztecInit)
ON_FRESH_VV("REALMYROOM.obar.aztec", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_AztecDown)
ON_FRESH_VI("REALMYROOM.obar.aztec", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.mascot", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotInit)
ON_FRESH_VV("REALMYROOM.obar.mascot", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MascotDown)
ON_FRESH_VI("REALMYROOM.obar.mascot", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.char_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CharSelInit)
ON_FRESH_VI("REALMYROOM.obar.char_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharSelOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.caddie_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieSelInit)
ON_FRESH_VI("REALMYROOM.obar.caddie_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieSelOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.club_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_ClubSelInit)
ON_FRESH_VI("REALMYROOM.obar.club_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubSelOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.aztec_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecSelInit)
ON_FRESH_VI("REALMYROOM.obar.aztec_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecSelOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.mascot_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotSelInit)
ON_FRESH_VI("REALMYROOM.obar.mascot_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotSelOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.pang_cookie", FRCMD_INIT,
	CTaskMain::OnOverBar_PangCookieInit)
ON_FRESH_VI("REALMYROOM.obar.pang_cookie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_PangCookieOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.onelinemsg_req", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineReqBtnInit)
ON_FRESH_VV("REALMYROOM.obar.onelinemsg_req", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OneLineReqBtnUp)
ON_FRESH_VI("REALMYROOM.obar.onelineview", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineViewInit)
ON_FRESH_VI("REALMYROOM.obar.onelineview", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_OneLineViewOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.spcard", FRCMD_INIT,
	CTaskMain::OnOverBar_SpecialCardInit)
ON_FRESH_VV("REALMYROOM.obar.spcard", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_SpecialCardUp)
ON_FRESH_VI("REALMYROOM.obar.spcard", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.obar.spcard", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.obar.messenger", FRCMD_INIT,
	CTaskMain::OnOverBar_MessengerInit)
ON_FRESH_VV("REALMYROOM.obar.messenger", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MessengerUp)
ON_FRESH_VI("REALMYROOM.obar.messenger", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.obar.messenger", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.obar.myinfo", FRCMD_INIT,
	CTaskMain::OnOverBar_MyInfoInit)
ON_FRESH_VV("REALMYROOM.obar.myinfo", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MyInfoUp)
ON_FRESH_VI("REALMYROOM.obar.myinfo", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.obar.myinfo", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.obar.option", FRCMD_INIT,
	CTaskMain::OnOverBar_OptionInit)
ON_FRESH_VV("REALMYROOM.obar.option", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OptionUp)
ON_FRESH_VI("REALMYROOM.obar.option", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOM.obar.option", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOM.obar.messenger_bubble", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleInit)
ON_FRESH_VI("REALMYROOM.obar.messenger_bubble", FRCMD_OWNERDRAW,
	CTaskMain::OnMessengerBubbleOwnerDraw)
ON_FRESH_VI("REALMYROOM.obar.messenger_desc", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleEditInit)
ON_FRESH_VI("REALMYROOM.obar.messenger_alarm", FRCMD_INIT,
	CTaskMain::OnMessengerAlarmBtnInit)
ON_FRESH_VV("REALMYROOM.obar.messenger_alarm", FRCMD_LBUTTONUP,
	CTaskMain::OnMessengerAlarmBtnUp)
ON_FRESH_VI("REALMYROOM.obar.gift_alarm", FRCMD_INIT,
	CTaskMain::OnMessengerGiftAlarmBtnInit)
ON_FRESH_VV("REALMYROOM.obar.gift_alarm", FRCMD_LBUTTONUP,
	CTaskMain::OnMessengerGiftAlarmBtnUp)
ON_FRESH_VI("REALMYROOM.obar.tooltip", FRCMD_OWNERDRAW,
	CTaskMain::OnBarTooltipOwnerDraw)
ON_FRESH_VI("REALMYROOM.ftab_main", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureTabMainInit)
ON_FRESH_VV("REALMYROOM.ftab_main", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurnitureTabMainButtonUp)
ON_FRESH_VI("REALMYROOM.fitem_list", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureListInit)
ON_FRESH_VI("REALMYROOM.fitem_list", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnFurnitureListOwnerDraw)
ON_FRESH_VV("REALMYROOM.fitem_list", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnFurnitureListBtnDown)
ON_FRESH_VI("REALMYROOM.farrorup", FRCMD_INIT,
	CRealMyRoomMain::OnFurniturePupInit)
ON_FRESH_VV("REALMYROOM.farrorup", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurniturePupButtonUp)
ON_FRESH_VI("REALMYROOM.farrordown", FRCMD_INIT,
	CRealMyRoomMain::OnFurniturePdownInit)
ON_FRESH_VV("REALMYROOM.farrordown", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurniturePdownButtonUp)
ON_FRESH_VI("REALMYROOM.flist_num", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureListNumInit)
ON_FRESH_VV("REALMYROOM.flist_num", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnFurnitureListNumBtnUp)
ON_FRESH_BI("REALMYROOM.flist_num", FRCMD_ENTERKEY,
	CRealMyRoomMain::OnFurnitureListEnterKey)
ON_FRESH_VI("REALMYROOM.flist_maxnum", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureListMaxNumInit)
ON_FRESH_VI("REALMYROOM.Btn_flistnum", FRCMD_INIT,
	CRealMyRoomMain::OnFurniture_BtnListNumInit)
ON_FRESH_VV("REALMYROOM.Btn_flistnum", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnFurniture_BtnListNumBtnUp)
ON_FRESH_VI("REALMYROOM.freset", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureResetInit)
ON_FRESH_VV("REALMYROOM.freset", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurnitureResetButtonUp)
ON_FRESH_VI("REALMYROOM.fback", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackInit)
ON_FRESH_VV("REALMYROOM.fback", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurnitureBackButtonUp)
ON_FRESH_VI("REALMYROOM.fsave", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureSaveInit)
ON_FRESH_VV("REALMYROOM.fsave", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurnitureSaveButtonUp)
ON_FRESH_VI("REALMYROOM.fout", FRCMD_INIT, CRealMyRoomMain::OnFurnitureOutInit)
ON_FRESH_VV("REALMYROOM.fout", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnFurnitureOutButtonUp)
ON_FRESH_VI("REALMYROOM.ftab_background01", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit1)
ON_FRESH_VI("REALMYROOM.ftab_background02", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit2)
ON_FRESH_VI("REALMYROOM.ftab_background03", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit3)
ON_FRESH_VI("REALMYROOM.ftab_background04", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit4)
ON_FRESH_VI("REALMYROOM.ftab_background05", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit5)
ON_FRESH_VI("REALMYROOM.ftab_background06", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit6)
ON_FRESH_VI("REALMYROOM.ftab_background07", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit7)
ON_FRESH_VI("REALMYROOM.ftab_background08", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureBackGroundInit8)
ON_FRESH_VI("REALMYROOM.ftab_scrollimg01", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureScrollInit1)
ON_FRESH_VI("REALMYROOM.ftab_scrollimg02", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureScrollInit2)
ON_FRESH_VI("REALMYROOM.ftab_scrollimg03", FRCMD_INIT,
	CRealMyRoomMain::OnFurnitureScrollInit3)
ON_FRESH_VI("REALMYROOM.Btn_Menu", FRCMD_INIT, CRealMyRoomMain::OnBtnMenuInit)
ON_FRESH_VV("REALMYROOM.Btn_Menu", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OOnBtnMenuButtonUP)
ON_FRESH_VI("REALMYROOM.Btn_MenuList", FRCMD_INIT,
	CRealMyRoomMain::OnBtn_MenuListInit)
ON_FRESH_VI("REALMYROOM.Btn_MenuList", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnBtn_MenuListOwnerDraw)
ON_FRESH_VV("REALMYROOM.Btn_MenuList", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnBtn_MenuListBtnDown)
ON_FRESH_VV("REALMYROOM.Btn_MenuList", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnBtn_MenuListRBtnDown)
ON_FRESH_VV("REALMYROOM.Btn_MenuList", FRCMD_MOUSEMOVE,
	CRealMyRoomMain::OnBtn_MenuListMouseMove)
ON_FRESH_VV("REALMYROOM.Btn_MenuList", FRCMD_DBLCLICK,
	CRealMyRoomMain::OnBtn_MenuListDClick)
ON_FRESH_VI("REALMYROOM.item_hide_desc_bg", FRCMD_INIT,
	CRealMyRoomMain::OnItemDescBgInit2)
ON_FRESH_VI("REALMYROOM.item_hide_desc_bg", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnItemDescBgOwnerDraw2)
ON_FRESH_VI("REALMYROOM.item_hide_desc", FRCMD_INIT,
	CRealMyRoomMain::OnItemDescInit2)
ON_FRESH_VI("REALMYROOM.item_hide_name", FRCMD_INIT,
	CRealMyRoomMain::OnItemNameInit)
ON_FRESH_VI("REALMYROOM.card_book", FRCMD_INIT, CRealMyRoomMain::OnCardBookInit)
ON_FRESH_VI("REALMYROOM.card_book", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnCardBookOwnerDraw)
ON_FRESH_VI("REALMYROOM.cardpacklist", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackListInit)
ON_FRESH_VI("REALMYROOM.cardpacklist", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnCardPackListDraw)
ON_FRESH_VV("REALMYROOM.cardpacklist", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackListDown)
ON_FRESH_VV("REALMYROOM.cardpacklist", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackListRbUp)
ON_FRESH_VI("REALMYROOM.cardpage_index", FRCMD_INIT,
	CRealMyRoomMain::OnCardBookIndexInit)
ON_FRESH_VV("REALMYROOM.cardpage_index", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardBookIndexLBDown)
ON_FRESH_VV("REALMYROOM.cardpage_index", FRCMD_MOUSEMOVE,
	CRealMyRoomMain::OnCardBookIndexMouseOver)
ON_FRESH_VI("REALMYROOM.card_property", FRCMD_INIT,
	CRealMyRoomMain::OnCardPropertyInit)
ON_FRESH_VI("REALMYROOM.card_pack", FRCMD_INIT,
	CRealMyRoomMain::OnTabCardPackInit)
ON_FRESH_VV("REALMYROOM.card_pack", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnTabCardPackLBDown)
ON_FRESH_VI("REALMYROOM.card_use", FRCMD_INIT, CRealMyRoomMain::OnCardUseInit)
ON_FRESH_VV("REALMYROOM.card_use", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardUseLBDown)
ON_FRESH_VI("REALMYROOM.card_volume1", FRCMD_INIT,
	CRealMyRoomMain::OnTabCardVolume1Init)
ON_FRESH_VV("REALMYROOM.card_volume1", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnTabCardVolume1LBDown)
ON_FRESH_VI("REALMYROOM.card_volume2", FRCMD_INIT,
	CRealMyRoomMain::OnTabCardVolume2Init)
ON_FRESH_VV("REALMYROOM.card_volume2", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnTabCardVolume2LBDown)
ON_FRESH_VI("REALMYROOM.card_volume3", FRCMD_INIT,
	CRealMyRoomMain::OnTabCardVolume3Init)
ON_FRESH_VV("REALMYROOM.card_volume3", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnTabCardVolume3LBDown)
ON_FRESH_VI("REALMYROOM.card_volume4", FRCMD_INIT,
	CRealMyRoomMain::OnTabCardVolume4Init)
ON_FRESH_VV("REALMYROOM.card_volume4", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnTabCardVolume4LBDown)
ON_FRESH_VI("REALMYROOM.cardpack_slot1", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit1)
ON_FRESH_VI("REALMYROOM.cardpack_slot2", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit2)
ON_FRESH_VI("REALMYROOM.cardpack_slot3", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit3)
ON_FRESH_VI("REALMYROOM.cardpack_slot4", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit4)
ON_FRESH_VI("REALMYROOM.cardpack_slot5", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit5)
ON_FRESH_VI("REALMYROOM.cardpack_slot6", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit6)
ON_FRESH_VI("REALMYROOM.cardpack_slot7", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit7)
ON_FRESH_VI("REALMYROOM.cardpack_slot8", FRCMD_INIT,
	CRealMyRoomMain::OnCardPackInit8)
ON_FRESH_VV("REALMYROOM.cardpack_slot1", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown1)
ON_FRESH_VV("REALMYROOM.cardpack_slot2", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown2)
ON_FRESH_VV("REALMYROOM.cardpack_slot3", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown3)
ON_FRESH_VV("REALMYROOM.cardpack_slot4", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown4)
ON_FRESH_VV("REALMYROOM.cardpack_slot5", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown5)
ON_FRESH_VV("REALMYROOM.cardpack_slot6", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown6)
ON_FRESH_VV("REALMYROOM.cardpack_slot7", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown7)
ON_FRESH_VV("REALMYROOM.cardpack_slot8", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardPackLBDown8)
ON_FRESH_VV("REALMYROOM.cardpack_slot1", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown1)
ON_FRESH_VV("REALMYROOM.cardpack_slot2", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown2)
ON_FRESH_VV("REALMYROOM.cardpack_slot3", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown3)
ON_FRESH_VV("REALMYROOM.cardpack_slot4", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown4)
ON_FRESH_VV("REALMYROOM.cardpack_slot5", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown5)
ON_FRESH_VV("REALMYROOM.cardpack_slot6", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown6)
ON_FRESH_VV("REALMYROOM.cardpack_slot7", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown7)
ON_FRESH_VV("REALMYROOM.cardpack_slot8", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardPackRBDown8)
ON_FRESH_VI("REALMYROOM.card_slot1", FRCMD_INIT, CRealMyRoomMain::OnCardInit1)
ON_FRESH_VI("REALMYROOM.card_slot2", FRCMD_INIT, CRealMyRoomMain::OnCardInit2)
ON_FRESH_VI("REALMYROOM.card_slot3", FRCMD_INIT, CRealMyRoomMain::OnCardInit3)
ON_FRESH_VI("REALMYROOM.card_slot4", FRCMD_INIT, CRealMyRoomMain::OnCardInit4)
ON_FRESH_VI("REALMYROOM.card_slot5", FRCMD_INIT, CRealMyRoomMain::OnCardInit5)
ON_FRESH_VI("REALMYROOM.card_slot6", FRCMD_INIT, CRealMyRoomMain::OnCardInit6)
ON_FRESH_VI("REALMYROOM.card_slot7", FRCMD_INIT, CRealMyRoomMain::OnCardInit7)
ON_FRESH_VI("REALMYROOM.card_slot8", FRCMD_INIT, CRealMyRoomMain::OnCardInit8)
ON_FRESH_VI("REALMYROOM.card_slot9", FRCMD_INIT, CRealMyRoomMain::OnCardInit9)
ON_FRESH_VI("REALMYROOM.card_slot10", FRCMD_INIT, CRealMyRoomMain::OnCardInit10)
ON_FRESH_VI("REALMYROOM.card_slot11", FRCMD_INIT, CRealMyRoomMain::OnCardInit11)
ON_FRESH_VI("REALMYROOM.card_slot12", FRCMD_INIT, CRealMyRoomMain::OnCardInit12)
ON_FRESH_VV("REALMYROOM.card_slot1", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown1)
ON_FRESH_VV("REALMYROOM.card_slot2", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown2)
ON_FRESH_VV("REALMYROOM.card_slot3", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown3)
ON_FRESH_VV("REALMYROOM.card_slot4", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown4)
ON_FRESH_VV("REALMYROOM.card_slot5", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown5)
ON_FRESH_VV("REALMYROOM.card_slot6", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown6)
ON_FRESH_VV("REALMYROOM.card_slot7", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown7)
ON_FRESH_VV("REALMYROOM.card_slot8", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown8)
ON_FRESH_VV("REALMYROOM.card_slot9", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown9)
ON_FRESH_VV("REALMYROOM.card_slot10", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown10)
ON_FRESH_VV("REALMYROOM.card_slot11", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown11)
ON_FRESH_VV("REALMYROOM.card_slot12", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnCardLBDown12)
ON_FRESH_VV("REALMYROOM.card_slot1", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown1)
ON_FRESH_VV("REALMYROOM.card_slot2", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown2)
ON_FRESH_VV("REALMYROOM.card_slot3", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown3)
ON_FRESH_VV("REALMYROOM.card_slot4", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown4)
ON_FRESH_VV("REALMYROOM.card_slot5", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown5)
ON_FRESH_VV("REALMYROOM.card_slot6", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown6)
ON_FRESH_VV("REALMYROOM.card_slot7", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown7)
ON_FRESH_VV("REALMYROOM.card_slot8", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown8)
ON_FRESH_VV("REALMYROOM.card_slot9", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown9)
ON_FRESH_VV("REALMYROOM.card_slot10", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown10)
ON_FRESH_VV("REALMYROOM.card_slot11", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown11)
ON_FRESH_VV("REALMYROOM.card_slot12", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnCardRBDown12)
ON_FRESH_VI("REALMYROOMVISITOR.obar.title", FRCMD_INIT,
	CTaskMain::OnOverBar_TitleInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.site", FRCMD_INIT,
	CTaskMain::OnOverBar_SiteInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.char", FRCMD_INIT,
	CTaskMain::OnOverBar_CharInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.char", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CharDown)
ON_FRESH_VI("REALMYROOMVISITOR.obar.char", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.caddie", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.caddie", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CaddieDown)
ON_FRESH_VI("REALMYROOMVISITOR.obar.caddie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.club", FRCMD_INIT,
	CTaskMain::OnOverBar_ClubInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.club", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_ClubDown)
ON_FRESH_VI("REALMYROOMVISITOR.obar.club", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.aztec", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.aztec", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_AztecDown)
ON_FRESH_VI("REALMYROOMVISITOR.obar.aztec", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.mascot", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.mascot", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MascotDown)
ON_FRESH_VI("REALMYROOMVISITOR.obar.mascot", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.char_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CharSelInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.char_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharSelOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.caddie_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieSelInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.caddie_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieSelOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.club_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_ClubSelInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.club_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubSelOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.aztec_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecSelInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.aztec_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecSelOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.mascot_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotSelInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.mascot_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotSelOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.pang_cookie", FRCMD_INIT,
	CTaskMain::OnOverBar_PangCookieInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.pang_cookie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_PangCookieOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.onelinemsg_req", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineReqBtnInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.onelinemsg_req", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OneLineReqBtnUp)
ON_FRESH_VI("REALMYROOMVISITOR.obar.onelineview", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineViewInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.onelineview", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_OneLineViewOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.spcard", FRCMD_INIT,
	CTaskMain::OnOverBar_SpecialCardInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.spcard", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_SpecialCardUp)
ON_FRESH_VI("REALMYROOMVISITOR.obar.spcard", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.obar.spcard", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.obar.messenger", FRCMD_INIT,
	CTaskMain::OnOverBar_MessengerInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.messenger", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MessengerUp)
ON_FRESH_VI("REALMYROOMVISITOR.obar.messenger", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.obar.messenger", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.obar.myinfo", FRCMD_INIT,
	CTaskMain::OnOverBar_MyInfoInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.myinfo", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MyInfoUp)
ON_FRESH_VI("REALMYROOMVISITOR.obar.myinfo", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.obar.myinfo", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.obar.option", FRCMD_INIT,
	CTaskMain::OnOverBar_OptionInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.option", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OptionUp)
ON_FRESH_VI("REALMYROOMVISITOR.obar.option", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.obar.option", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.obar.messenger_bubble", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.messenger_bubble", FRCMD_OWNERDRAW,
	CTaskMain::OnMessengerBubbleOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.obar.messenger_desc", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleEditInit)
ON_FRESH_VI("REALMYROOMVISITOR.obar.messenger_alarm", FRCMD_INIT,
	CTaskMain::OnMessengerAlarmBtnInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.messenger_alarm", FRCMD_LBUTTONUP,
	CTaskMain::OnMessengerAlarmBtnUp)
ON_FRESH_VI("REALMYROOMVISITOR.obar.gift_alarm", FRCMD_INIT,
	CTaskMain::OnMessengerGiftAlarmBtnInit)
ON_FRESH_VV("REALMYROOMVISITOR.obar.gift_alarm", FRCMD_LBUTTONUP,
	CTaskMain::OnMessengerGiftAlarmBtnUp)
ON_FRESH_VI("REALMYROOMVISITOR.obar.tooltip", FRCMD_OWNERDRAW,
	CTaskMain::OnBarTooltipOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.back", FRCMD_INIT,
	CTaskMain::OnUnderBar_BackInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.back", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUnderBar_BackUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.back", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.back", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.front", FRCMD_INIT,
	CTaskMain::OnUnderBar_FrontInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.front", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_FrontUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.front", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.front", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.shop", FRCMD_INIT,
	CTaskMain::OnUnderBar_ShopInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.shop", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ShopUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.shop", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.shop", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.myroom", FRCMD_INIT,
	CTaskMain::OnUnderBar_MyRoomInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.myroom", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MyRoomUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.myroom", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.myroom", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.itemstorage", FRCMD_INIT,
	CTaskMain::OnUnderBar_ItemStorageInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.itemstorage", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUnderBar_ItemStorageUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.itemstorage", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.itemstorage", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.gift", FRCMD_INIT,
	CTaskMain::OnUnderBar_GiftInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.gift", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUnderBar_GiftUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.gift", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.gift", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.cookie", FRCMD_INIT,
	CTaskMain::OnUnderBar_CookieInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.cookie", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_CookieUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.cookie", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.cookie", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.game", FRCMD_INIT,
	CTaskMain::OnUnderBar_GameInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.game", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GameUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.game", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.game", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.server", FRCMD_INIT,
	CTaskMain::OnUnderBar_ServerInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.server", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ServerUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.server", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.server", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.exit", FRCMD_INIT,
	CTaskMain::OnUnderBar_ExitInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.exit", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ExitUp)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.exit", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.exit", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.ubar.tabbtn_b", FRCMD_INIT,
	CTaskMain::OnUnderBar_TabBtnInit)
ON_FRESH_VV("REALMYROOMVISITOR.ubar.tabbtn_b", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TabBtnUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st", FRCMD_INIT,
	CRealMyRoomMain::OnStatBoxInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selframe", FRCMD_INIT,
	CRealMyRoomMain::OnSelFrameInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selitemicon", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemIconInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selitem", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemInit)
ON_FRESH_VV("REALMYROOMVISITOR.middle_ui.st.selitem", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnSelItemUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selitem", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnSelItemOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selitemicon", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnSelItemIconOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selitem_prev", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemPrevInit)
ON_FRESH_VV("REALMYROOMVISITOR.middle_ui.st.selitem_prev", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnSelItemPrevUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.selitem_next", FRCMD_INIT,
	CRealMyRoomMain::OnSelItemNextInit)
ON_FRESH_VV("REALMYROOMVISITOR.middle_ui.st.selitem_next", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnSelItemNextUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.stat_part", FRCMD_INIT,
	CRealMyRoomMain::OnStatPartInit)
ON_FRESH_VV("REALMYROOMVISITOR.middle_ui.st.stat_part", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnStatPartUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.stat_part", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnStatPartOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.stat_all", FRCMD_INIT,
	CRealMyRoomMain::OnStatAllInit)
ON_FRESH_VV("REALMYROOMVISITOR.middle_ui.st.stat_all", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnStatAllUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.stat_all", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnStatAllOwnerDraw)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.stat_frame", FRCMD_INIT,
	CRealMyRoomMain::OnStatFrameInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.stat_text", FRCMD_INIT,
	CRealMyRoomMain::OnStatTextInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.powerbar", FRCMD_INIT,
	CRealMyRoomMain::OnPowerBarInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.controlbar", FRCMD_INIT,
	CRealMyRoomMain::OnControlBarInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.impactbar", FRCMD_INIT,
	CRealMyRoomMain::OnImpactBarInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.spinbar", FRCMD_INIT,
	CRealMyRoomMain::OnSpinBarInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.curvebar", FRCMD_INIT,
	CRealMyRoomMain::OnCurveBarInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.epower", FRCMD_INIT,
	CRealMyRoomMain::OnPowerEditInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.econtrol", FRCMD_INIT,
	CRealMyRoomMain::OnControlEditInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.eimpact", FRCMD_INIT,
	CRealMyRoomMain::OnImpactEditInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.espin", FRCMD_INIT,
	CRealMyRoomMain::OnSpinEditInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.st.ecurve", FRCMD_INIT,
	CRealMyRoomMain::OnCurveEditInit)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.upgrade", FRCMD_INIT,
	CRealMyRoomMain::OnUpgradeInit)
ON_FRESH_VV("REALMYROOMVISITOR.middle_ui.upgrade", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnUpgradeUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.upgrade", FRCMD_HOVERON,
	CRealMyRoomMain::OnTooltipHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.upgrade", FRCMD_HOVEROFF,
	CRealMyRoomMain::OnTooltipHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.downgrade", FRCMD_INIT,
	CRealMyRoomMain::OnDowngradeInit)
ON_FRESH_VV("REALMYROOMVISITOR.middle_ui.downgrade", FRCMD_LBUTTONUP,
	CRealMyRoomMain::OnDowngradeUp)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.downgrade", FRCMD_HOVERON,
	CRealMyRoomMain::OnTooltipHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.middle_ui.downgrade", FRCMD_HOVEROFF,
	CRealMyRoomMain::OnTooltipHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)
ON_FRESH_VI("REALMYROOMVISITOR.utab.magicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_MagicBoxInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.magicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MagicBoxUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.magicbox", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.magicbox", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab.tikimagicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_TikiMagicBoxInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.tikimagicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TikiMagicBoxUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.tikimagicbox", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.tikimagicbox", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab.scratch", FRCMD_INIT,
	CTaskMain::OnUnderBar_ScratchInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.scratch", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ScratchUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.scratch", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.scratch", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab.bongdarishop", FRCMD_INIT,
	CTaskMain::OnUnderBar_BongdariShopInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.bongdarishop", FRCMD_LBUTTONDOWN,
	CTaskMain::OnUnderBar_BongdariShopUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.bongdarishop", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.bongdarishop", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab.maket", FRCMD_INIT,
	CTaskMain::OnUnderBar_MaketInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.maket", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MaketUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.maket", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.maket", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab.ranking", FRCMD_INIT,
	CTaskMain::OnUnderBar_RankingInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.ranking", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_RankingUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.ranking", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.ranking", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab.guild", FRCMD_INIT,
	CTaskMain::OnUnderBar_GuildInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.guild", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_GuildUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.guild", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.guild", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.utab.notice", FRCMD_INIT,
	CTaskMain::OnUnderBar_NoticeInit)
ON_FRESH_VV("REALMYROOMVISITOR.utab.notice", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_NoticeUp)
ON_FRESH_VI("REALMYROOMVISITOR.utab.notice", FRCMD_HOVERON,
	CTaskMain::OnBarHoverOn)
ON_FRESH_VI("REALMYROOMVISITOR.utab.notice", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("REALMYROOMVISITOR.rmenu", FRCMD_INIT,
	CRealMyRoomMain::OnRightMenuInit)
ON_FRESH_VV("REALMYROOMVISITOR.rmenu", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnRightMenuLBtnDown)
ON_FRESH_VV("REALMYROOMVISITOR.rmenu", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnRightMenuRBtnDown)
ON_FRESH_VI("REALMYROOM.item_storage_bgi", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStorageBGI)
ON_FRESH_VI("REALMYROOM.item_storage_bgi", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnOwnerDrawItemStorageBGI)
ON_FRESH_VI("REALMYROOM.item_storage_pang", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStoragePang)
ON_FRESH_VI("REALMYROOM.item_storage_pang", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnOwnerDrawItemStoragePang)
ON_FRESH_VI("REALMYROOM.pang_minus", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStoragePangMinus)
ON_FRESH_VV("REALMYROOM.pang_minus", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnLBDownItemStoragePangMinus)
ON_FRESH_VI("REALMYROOM.pang_plus", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStoragePangPlus)
ON_FRESH_VV("REALMYROOM.pang_plus", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnLBDownItemStoragePangPlus)
ON_FRESH_VI("REALMYROOM.item_storage_list", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStorageList)
ON_FRESH_VI("REALMYROOM.item_storage_list", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnOwnerDrawItemStorageList)
ON_FRESH_VV("REALMYROOM.item_storage_list", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnLBDownItemStorageList)
ON_FRESH_VV("REALMYROOM.item_storage_list", FRCMD_RBUTTONDOWN,
	CRealMyRoomMain::OnRBDownItemStorageList)
ON_FRESH_VV("REALMYROOM.item_storage_list", FRCMD_DBLCLICK,
	CRealMyRoomMain::OnDBClickItemStorageList)
ON_FRESH_VV("REALMYROOM.item_storage_list", FRCMD_MOUSEMOVE,
	CRealMyRoomMain::OnMouseMoveItemStorageList)
ON_FRESH_VI("REALMYROOM.item_storage_index", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStorageIndexMsg)
ON_FRESH_VI("REALMYROOM.item_storage_index", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnOwnerDrawItemStorageIndexMsg)
ON_FRESH_VV("REALMYROOM.item_storage_index", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnLBDownItemStorageIndexMsg)
ON_FRESH_VI("REALMYROOM.item_storage_page_next", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStoragePageNext)
ON_FRESH_VV("REALMYROOM.item_storage_page_next", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnLBDownItemStoragePageNext)
ON_FRESH_VI("REALMYROOM.item_storage_page_prev", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStoragePagePrev)
ON_FRESH_VV("REALMYROOM.item_storage_page_prev", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnLBDownItemStoragePagePrev)
ON_FRESH_VI("REALMYROOM.item_storage_close", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStorageClose)
ON_FRESH_VV("REALMYROOM.item_storage_close", FRCMD_LBUTTONDOWN,
	CRealMyRoomMain::OnLBDownItemStorageClose)
ON_FRESH_VI("REALMYROOM.item_storage_dragdrop", FRCMD_INIT,
	CRealMyRoomMain::OnInitItemStorageDragDrop)
ON_FRESH_VI("REALMYROOM.item_storage_dragdrop", FRCMD_OWNERDRAW,
	CRealMyRoomMain::OnOwnerDrawItemStorageDragDrop)
END_FRESH_MSGMAP()

void __fastcall SetMainTabImg(const char** const names, int index)
{
	names[0] = MakeStr("stab_%d_n", index);
	names[1] = MakeStr("stab_%d_d", index);
	names[2] = MakeStr("stab_%d_d", index);
}

void __fastcall SetSubTabImg(const char** const names, int index)
{
	names[0] = MakeStr("stext_%d_n", index);
	names[1] = MakeStr("stext_%d_n", index);
	names[2] = MakeStr("stext_%d_n", index);
}

bool __fastcall EmptyCompares(const void* a, const void* b)
{
	if (a != NULL && b == NULL)
		return true;
	return false;
}

extern int g_lobbyModal983; // 00e78cac

void CRealMyRoomMain::OnAnotherHand(int key, void* param)
{
	if (key == 0)
		ShowPet(*(unsigned long*)param);
}

void CRealMyRoomMain::OpenLayout(const char* name)
{
	WSendPacket packet(0x5c);
	packet.Send(TO_GAME);
	g_pFresh->OpenLayout(name, this, false);
}

void CRealMyRoomMain::CloseLayout()
{
	if (m_pFrWnd_694)
		m_pFrWnd_694->Close(true);
	g_pFresh->CloseLayout();
}

void CRealMyRoomMain::ShowPet(unsigned long tid)
{
	if (0) // HACK: keeps the inline naked LoadMyRoomImages emitted (nakedify)
		((CRealMyRoomMain*)0)->LoadMyRoomImages();
	if (tid == 0)
		return;
	m_hand.Enter(); // m_hand.m_csUser (era this+0x810 = layout do 645)
	if (m_pFrWnd_314 == NULL)
	{
		m_hand.Leave();
		return;
	}
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition != NULL)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
	WRect* pRect = (WRect*)((char*)m_pFrWnd_314 + 0x20);
	float x = pRect->x;
	float y = pRect->y;
	float w = pRect->w;
	float h = pRect->h;
	bool bOk = false;
	pExhibition = NULL;
	switch (tid >> 26)
	{
	case 1:
		if (m_n_77C == 13 || m_n_77C == 14 || m_n_77C == 15)
		{
			pExhibition =
				new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f, 0.35f, true);
			pExhibition->SetArea(w, h);
			pExhibition->SetDistanceRange(16.0f, 20.0f);
			pExhibition->SetDistance(18.0f, true, false);
		}
		else
		{
			pExhibition = new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f + 15.0f,
				0.35f, true);
			pExhibition->SetArea(w, h);
			pExhibition->SetDistanceRange(8.0f, 12.0f);
			pExhibition->SetDistance(10.0f, true, false);
		}
		if (IsLocalContent(S4_UCC) && m_pFrUccDrawDlg_45C != NULL &&
			m_pFrUccDrawDlg_45C->m_pItemInfo != NULL)
		{
			char* pUcc = (char*)m_pFrUccDrawDlg_45C->m_pItemInfo;
			unsigned long ucc = *(unsigned long*)(pUcc + 4);
			char name[9];
			strncpy(name, pUcc + 0x52, 9);
			sItemInfo* pInfo = GetMyItemInfo(ucc, name, 0);
			char uccNames[24][9];
			pExhibition->SetAutoRecoverCamera(false);
			strcpy(uccNames[(ucc >> 13) & 0x1f], name);
			CPartTidList parts = CSharedDoc::Instance()->GetMyPartTidList();
			unsigned short flag = 0;
			unsigned long uid = 0;
			if (pInfo != NULL)
			{
				flag = *(unsigned short*)((char*)pInfo + 0x5c);
				uid = *(unsigned long*)pInfo;
			}
			parts.SetTid(ucc, uid, name, flag);
			bOk = pExhibition->SetModel(parts,
				CSharedDoc::Instance()->GetMyAuxPartTidList(), NULL, 3.0f, 0);
		}
		else
		{
			bOk = pExhibition->SetModel(
				CSharedDoc::Instance()->GetMyPartTidList(),
				CSharedDoc::Instance()->GetMyAuxPartTidList(), NULL, 3.0f, 0);
		}
		break;
	case 4:
		pExhibition = new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f, 0.0f, true);
		pExhibition->SetArea(w, h);
		pExhibition->SetDistance(7.0f, true, false); // 983 0060f0xx: d09150 = 7.0
		bOk = pExhibition->SetModel(tid);
		break;
	case 5:
		pExhibition = new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f, 0.0f, true);
		pExhibition->SetArea(w, h);
		pExhibition->SetDistance(1.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	case 6:
		pExhibition = new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f, 0.0f, true);
		pExhibition->SetArea(w, h);
		pExhibition->SetDistance(6.0f, true, false);
		pExhibition->SetDistanceRange(5.0f, 7.0f);
		bOk = pExhibition->SetModel(tid);
		break;
	case 7:
	case 8:
		if (g_lobbyModal983 == 0) // 983: sem a sala 3D (00e78cac == 0) nao ha previa de mascote/caddie
			goto done;
		pExhibition =
			new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f + 15.0f, 0.35f, true);
		pExhibition->SetArea(w, h);
		pExhibition->SetDistanceRange(9.0f, 13.0f);
		pExhibition->SetDistance(11.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	case 0xb:
		pExhibition = new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f, 0.0f, true);
		pExhibition->SetArea(w, h);
		pExhibition->SetDistance(18.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	case 0x10:
		pExhibition =
			new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f + 15.0f, 0.35f, true);
		pExhibition->SetArea(w, h);
		pExhibition->SetDistanceRange(4.0f, 8.0f);
		pExhibition->SetDistance(6.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	case 0x12:
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			pExhibition =
				new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f, 0.0f, true);
			pExhibition->SetArea(w * 0.5f, h * 0.5f);
			pExhibition->SetDistance(15.0f, true, false);
			bOk = pExhibition->SetModel(tid);
		}
		break;
	case 0x1f:
		pExhibition = new CExhibitionByHardwareTnL(x + w * 0.5f, y + h * 0.5f, 0.0f, true);
		pExhibition->SetArea(w, h);
		pExhibition->SetDistanceRange(5.0f, 7.0f);
		if ((tid & 0x3c00000) == 0xc00000)
			pExhibition->SetDistance(5.0f, true, false);
		else
			pExhibition->SetDistance(6.0f, true, false);
		bOk = pExhibition->SetModel(tid);
		break;
	default:
		goto done;
	}
	if (!bOk)
	{
		if (pExhibition != NULL)
			delete pExhibition;
	}
	else
	{
		pExhibition->SetControlFlag(0x1f); // 983: [obj+0x564] = 0x1f
		m_pExhibition = pExhibition;
	}
done:
	m_hand.Leave();
}

void CRealMyRoomMain::HidePet(bool bWait)
{
	if (bWait)
		m_hand.WaitUntilWorkDone(9999);
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition != NULL)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
}

void CRealMyRoomMain::OnMyRoomInit(int param)
{
	m_trophyList.clear();
	SetConstantBGM("myroom.mp3", false, false);
	TurnOff3DBackground();
}

void CRealMyRoomMain::OnMyRoomFinish()
{
	m_pOverBarTitle->SetBgImg("title_myroom");
	m_pFrWnd_314 = m_pFrArea_378;
	Reinitialize(false);
	if (Doc()->m_bRefreshCamera)
		HandleMsg(MsgObject(NULL, M983(0xaa), 1, 0, 0, 0, 0));
}

void CRealMyRoomMain::Reinitialize(bool bKeepCaddieMascot)
{
	if (Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
	{
		ChangeCharacter(Doc()->m_myInfo.userEquip.guidChar);
		ChangeClubset(Doc()->m_myInfo.userEquip.guidClubSet);
		ChangeAztec(Doc()->m_myInfo.userEquip.tidBall);
		if (!bKeepCaddieMascot)
		{
			ChangeCaddie(Doc()->m_myInfo.userEquip.guidCaddie);
			ChangeMascot(Doc()->m_myInfo.userEquip.guidMascot);
		}
		ChangeMainTab((mainTab_t)m_roomMode, true);
		ChangeSubTab((subTab_t)m_n_77C, true);
		m_n_774 = (2 < m_roomMode);
		SetLevelBar();
		m_selTab_t_780 = (selTab_t)0;

		struct sSkinSet
		{
			unsigned long tid[6];
		};
		sSkinSet* pSet = (sSkinSet*)&m_n_B78;
		*pSet = *(sSkinSet*)Doc()->m_myInfo.userEquip.tidSkin;
		unsigned long* pSkin = pSet->tid;
		int i = 6;
		do
		{
			if (*pSkin != 0)
			{
				IFF_STRUCT::sSkin* pInfo = ItemManager()->FindSkin(*pSkin);
				if (pInfo != NULL && (*pSkin & 0x3c00000) == 0)
				{
					m_n_B98 = pInfo->HScroll;
					m_n_B9C = pInfo->VScroll;
					m_scrollPos[0] = 0.0f;
					m_scrollPos[1] = 0.0f;
				}
			}
			pSkin++;
			i--;
		} while (i != 0);
	}
}

void CRealMyRoomMain::ReinitializePC(ePCBangItem item)
{
	if (item == PCBANGITEM_CADDIE)
		ChangeCaddie(Doc()->m_realEquip.guidCaddie);
	else if (item == PCBANGITEM_MASCOT)
		ChangeMascot(Doc()->m_realEquip.guidMascot);
}

void CRealMyRoomMain::InitMyRoomDialogs()
{
	m_n_424 = 0;
	m_pOptionDlg = NULL;
	m_pKbdManualDlg = NULL;
	m_pServerDlg = NULL;
	m_pGiftDlg = NULL;
	m_pWaitDlg = NULL;
	m_pNotifyOneDlg = NULL;
	m_fKbdManualTime = -10.0f;
	m_pFrEnchantDlg_428 = NULL;
	m_pFrDowngradeDlg_430 = NULL;
	m_pFrButton_390 = NULL;
	m_pFrDowngradeDlg_42C = NULL;
	m_pFrPangGiftDlg_434 = NULL;
	m_pFrMascotInfoDlg_438 = NULL;
	m_pFrNoteDlg_43C = NULL;
	m_pFrFortuneDlg_440 = NULL;
	m_pFortuneOpenDlg = NULL;
	m_pFrReplayModeDlg_448 = NULL;
	m_pCPost_ReciveDlg_474 = NULL;
	m_pCPost_SendDlg_478 = NULL;
	m_pCRealMyRoom_AuthorityDlg_47C = NULL;
	if (IsLocalContent(S4_FURNITURE_ABILITY))
	{
		m_pCFurniture_AbilityDlg_480 = NULL;
	}
	m_n_DA0 = 0;
	m_b_2C4 = true;
	m_pFrUccDrawDlg_45C = NULL;
	m_pFrUccCopyDlg_460 = NULL;
	m_pFrUniteResultDlg_464 = NULL;
	m_pFrCardUseDlg_44C = NULL;
	m_pFrCardRemove_468 = NULL;
	m_pFrItemStorageNotice_D90 = NULL;
	m_pFrItemStorageAccessPanel_D94 = NULL;
	m_b_D98 = false;
	m_n_D9C = 0;
	m_pFrArea_DA4 = NULL;
	m_pFrArea_DA8 = NULL;
	m_pFrButton_DAC = NULL;
	m_pFrButton_DB0 = NULL;
	m_pFrListBox_DB4 = NULL;
	m_pFrListBox_DB8 = NULL;
	m_pFrButton_DBC = NULL;
	m_pFrButton_DC0 = NULL;
	m_pFrButton_DC4 = NULL;
	m_pFrArea_DC8 = NULL;
	SetItemStorageVisible(false);
}

void CRealMyRoomMain::InitMyRoomControls()
{
	m_pFrGroupBox_2E4 = 0;
	m_pFrStatic_2E8 = 0;
	m_pFrViewer_2EC = 0;
	m_pFrViewer_2F0 = 0;
	m_pFrTabButton_2F4 = 0;
	m_pFrTabButton_2F8 = 0;
	m_n_2FC = 0;
	m_n_300 = 0;
	m_pFrListBox_304 = 0;
	m_pFrArea_308 = 0;
	m_pFrArea_30C = 0;
	m_pFrArea_310 = 0;
	m_pFrWnd_314 = 0;
	m_pExhibition = 0;
	m_pFrButton_324 = 0;
	m_pFrGaugeBar_32C = 0;
	m_pFrGaugeBar_330 = 0;
	m_pFrGaugeBar_334 = 0;
	m_pFrGaugeBar_338 = 0;
	m_pFrGaugeBar_33C = 0;
	m_pFrStatic_340 = 0;
	m_pFrStatic_344 = 0;
	m_pFrStatic_348 = 0;
	m_pFrStatic_34C = 0;
	m_pFrStatic_350 = 0;
	m_pFrButton_354 = 0;
	m_pFrButton_358 = 0;
	m_n_B78 = 0;
	m_n_B7C = 0;
	m_n_B80 = 0;
	m_n_B84 = 0;
	m_n_B88 = 0;
	m_n_B8C = 0;
	m_n_B98 = 0;
	m_n_B9C = 0;
	m_scrollPos[0] = 0;
	m_scrollPos[1] = 0;
	m_pFrArea_378 = 0;
	m_pFrArea_370 = 0;
	m_pFrListBox_37C = 0;
	m_pFrButton_388 = 0;
	m_pFrButton_38C = 0;
	m_pFrGroupBox_384 = 0;
	m_b_784 = 0;
	m_pFrWnd_398 = 0;
	m_n_318 = 0;
	m_pFrArea_320 = 0;
	m_pFrGroupBox_328 = 0;
	m_pFrArea_31C = 0;
	m_pBitmap_4D4 = 0;
	m_pBitmap_54C = 0;
	m_pBitmap_558 = 0;
	m_pBitmap_55C = 0;
	m_pFrButton_394 = 0;
	m_pFrArea_35C = 0;
	m_pFrEdit_360 = 0;
	m_pFrViewer_364 = 0;
	m_pFrEdit_368 = 0;
	m_pFrViewer_39C = 0;
	m_pFrEdit_3A0 = 0;
	m_pFrButton_380 = 0;
	m_pFrArea_684 = 0;
	m_n_688 = 0;
	m_pFrArea_374 = 0;
	m_pFrArea_3A4 = 0;
	m_pFrArea_3A8 = 0;
	m_pFrWnd_654 = 0;
	m_pFrWnd_658 = 0;
	m_pFrWnd_660 = 0;
	m_pFrWnd_664 = 0;
	m_pFrWnd_668 = 0;
	m_pFrWnd_66C = 0;
	m_pFrWnd_670 = 0;
	m_pFrWnd_674 = 0;
	m_pFrWnd_678 = 0;
	m_pFrWnd_67C = 0;
	m_pFrWnd_680 = 0;
	m_pFrWnd_210 = 0;
	m_pFrTabButton_3AC = 0;
	m_pFrListBox_3B0 = 0;
	m_n_D4C = 0;
	m_n_D50 = 0;
	*(unsigned long*)&m_b_D54 = 0x1010101;
	m_b_D58 = true;
	m_b_D57 = false;
	m_pFrButton_3CC = 0;
	m_pFrButton_3D0 = 0;
	m_pFrButton_3D4 = 0;
	m_pFrButton_3D8 = 0;
	m_pFrArea_3DC = 0;
	m_pFrArea_3E0 = 0;
	m_pFrArea_3E4 = 0;
	m_pFrArea_3E8 = 0;
	m_pFrArea_3EC = 0;
	m_pFrArea_3F0 = 0;
	m_pFrArea_3F4 = 0;
	m_pFrArea_3F8 = 0;
	m_pFrArea_3FC = 0;
	m_pFrArea_400 = 0;
	m_pFrArea_404 = 0;
	m_pFrEdit_3C0 = 0;
	m_pFrEdit_3C4 = 0;
	m_pFrButton_3C8 = 0;
	m_pFrButton_3B8 = 0;
	m_pFrButton_3BC = 0;
	m_pFrListBox_3B4 = 0;
	m_n_408 = 0;
	m_n_40C = 0;
	m_n_410 = 0;
	m_n_414 = 0;
	m_n_418 = 0;
	m_n_41C = 0;
	m_b_D44 = 0;
	m_b_D45 = true;
	m_b_D46 = 0;
	m_n_D5C = 0;
}

void CRealMyRoomMain::InitMyRoomVariables()
{
	m_b_6D8 = false;
	m_tidCaddie = 0;
	m_n_754 = 0;
	m_n_758 = 0;
	m_n_75C = 0;
	m_n_760 = 0;
	m_tidChar = 0;
	m_n_74C = 0;
	m_n_764 = 0;
	m_tidAztec = 0;
	m_pCharStat = NULL;
	m_pParts = NULL;
	if (IsLocalContent(S4_UCC))
	{
		memset(&m_partUid, 0, 0x60);
	}
	m_pAuxParts = NULL;
	m_n_774 = 0;
	m_roomMode = 0;
	m_n_77C = 0x15;
	m_pClubStat = NULL;
	m_selTab_t_780 = (selTab_t)0;
	m_n_B74 = 0;
	m_giftTotal = 0;
	m_giftCount = 0;
	m_giftSel = *(unsigned long*)((char*)&CSharedDoc::m_cardPackMode);
	m_giftSel = (unsigned int)*((unsigned char*)&CSharedDoc::m_cardPackSlot);
	m_giftPage = 0;
	m_giftPrevRect = WRect(0, 0, 0, 0);
	m_giftSlotRect[0] = WRect(0, 0, 0, 0);
	m_giftSlotRect[1] = WRect(0, 0, 0, 0);
	m_giftSlotRect[2] = WRect(0, 0, 0, 0);
	m_giftSlotRect[3] = WRect(0, 0, 0, 0);
	m_giftSlotRect[4] = WRect(0, 0, 0, 0);
	m_giftSlotRect[5] = WRect(0, 0, 0, 0);
	m_pFrArea_684 = NULL;
	m_n_688 = 0;
	m_pFrArea_374 = NULL;
	m_pFrArea_3A4 = NULL;
	m_pFrArea_3A8 = NULL;
	m_pFrWnd_654 = NULL;
	m_pFrWnd_658 = NULL;
	m_pFrWnd_660 = NULL;
	m_pFrWnd_664 = NULL;
	m_pFrWnd_668 = NULL;
	m_pFrWnd_66C = NULL;
	m_pFrWnd_670 = NULL;
	m_pFrWnd_674 = NULL;
	m_pFrWnd_678 = NULL;
	m_pFrWnd_67C = NULL;
	m_pFrWnd_680 = NULL;
	m_b_C26 = false;
	m_b_C24 = false;
	m_eFurniture_Tab_7A0 = (eFurniture_Tab)0;
	m_n_7A4 = 0;
	m_pBitmap_650 = NULL;
	m_b_C25 = true;
	m_n_7A8 = 1;
}

void CRealMyRoomMain::LoadMyRoomImages()
{
	extern const char* dot_files[5][2]; // de shopmain.cpp (o ramo de bytes usa o apelido rb_shop_dot_files)
	const char** ppFile = &dot_files[0][0];
	const Bitmap** ppImg = &m_pStatusBarImg[0][0];
	do
	{
		for (int n = 2; n != 0; n--)
			*ppImg++ = g_pFresh->RegisterBitmap(*ppFile++);
	} while (ppFile < &dot_files[5][0]);
	m_pBitmap_4C8 = g_pFresh->RegisterBitmap("select_up");
	m_pBitmap_4CC = g_pFresh->RegisterBitmap("select_mid");
	m_pBitmap_4D0 = g_pFresh->RegisterBitmap("select_low");
	m_pBitmap_4D4 = g_pFresh->RegisterBitmap("select_empty");
	int num = 1;
	const Bitmap** ppSel = &m_pSelectImg[0][1];
	for (int left = 5; left != 0; left--)
	{
		ppSel[-1] = g_pFresh->RegisterBitmap(MakeStr("select_%d_n", num));
		ppSel[0] = g_pFresh->RegisterBitmap(MakeStr("select_%d_o", num));
		num++;
		ppSel += 3;
	}
	m_pBitmap_54C = g_pFresh->RegisterBitmap(MakeStr("select_divide"));
	m_pBitmap_4D8 = g_pFresh->RegisterBitmap("select_cursor1");
	m_pBitmap_4DC = g_pFresh->RegisterBitmap("select_cursor0");
	m_pStatTabImg[0][0] = g_pFresh->RegisterBitmap("stat_char_n");
	m_pStatTabImg[0][1] = g_pFresh->RegisterBitmap("stat_char_o");
	m_pStatTabImg[0][2] = g_pFresh->RegisterBitmap("stat_char_d");
	m_pStatTabImg[1][0] = g_pFresh->RegisterBitmap("stat_club_n");
	m_pStatTabImg[1][1] = g_pFresh->RegisterBitmap("stat_club_o");
	m_pStatTabImg[1][2] = g_pFresh->RegisterBitmap("stat_club_d");
	m_pStatTabImg[2][0] = g_pFresh->RegisterBitmap("stat_caddie_n");
	m_pStatTabImg[2][1] = g_pFresh->RegisterBitmap("stat_caddie_o");
	m_pStatTabImg[2][2] = g_pFresh->RegisterBitmap("stat_caddie_d");
	m_pStatTabImg[3][0] = g_pFresh->RegisterBitmap("stat_all_n");
	m_pStatTabImg[3][1] = g_pFresh->RegisterBitmap("stat_all_o");
	m_pStatTabImg[3][2] = g_pFresh->RegisterBitmap("stat_all_d");
	m_pBitmap_484 = g_pFresh->RegisterBitmap("item_base_n");
	m_pBitmap_488 = g_pFresh->RegisterBitmap("item_base_o");
	m_pBitmap_48C = g_pFresh->RegisterBitmap("item_base_d");
	m_pBitmap_490 = g_pFresh->RegisterBitmap("item_base_empty");
	m_pBitmap_9F0 = g_pFresh->RegisterBitmap("giftmove_btn_n");
	m_pBitmap_9F4 = g_pFresh->RegisterBitmap("giftmove_btn_o");
	{
		float w = (float)m_pBitmap_9F0->Width();
		float h = (float)m_pBitmap_9F0->Height();
		m_WRect_9E0.x = 103.0f;
		m_WRect_9E0.y = 66.0f;
		m_WRect_9E0.w = w;
		m_WRect_9E0.h = h;
	}
	m_pBitmap_A60 = g_pFresh->RegisterBitmap("answer_btn_n");
	m_pBitmap_A64 = g_pFresh->RegisterBitmap("answer_btn_o");
	{
		float w = (float)m_pBitmap_A60->Width();
		float h = (float)m_pBitmap_A60->Height();
		m_WRect_A50.x = 144.0f;
		m_WRect_A50.y = 66.0f;
		m_WRect_A50.w = w;
		m_WRect_A50.h = h;
	}
	m_pBitmap_A0C = g_pFresh->RegisterBitmap("equip_btn_n");
	m_pBitmap_A10 = g_pFresh->RegisterBitmap("equip_btn_o");
	{
		float w = (float)m_pBitmap_A0C->Width();
		float h = (float)m_pBitmap_A0C->Height();
		m_WRect_9FC.x = 103.0f;
		m_WRect_9FC.y = 66.0f;
		m_WRect_9FC.w = w;
		m_WRect_9FC.h = h;
	}
	m_pBitmap_A28 = g_pFresh->RegisterBitmap("delete_btn_n");
	m_pBitmap_A2C = g_pFresh->RegisterBitmap("delete_btn_o");
	{
		float w = (float)m_pBitmap_A28->Width();
		float h = (float)m_pBitmap_A28->Height();
		m_WRect_A18.x = 144.0f;
		m_WRect_A18.y = 66.0f;
		m_WRect_A18.w = w;
		m_WRect_A18.h = h;
	}
	m_pBitmap_A44 = g_pFresh->RegisterBitmap("reemploy_btn_n");
	m_pBitmap_A48 = g_pFresh->RegisterBitmap("reemploy_btn_o");
	{
		float w = (float)m_pBitmap_A44->Width();
		float h = (float)m_pBitmap_A44->Height();
		m_WRect_A34.x = 144.0f;
		m_WRect_A34.y = 66.0f;
		m_WRect_A34.w = w;
		m_WRect_A34.h = h;
	}
	m_pBitmap_A7C = g_pFresh->RegisterBitmap("clock");
	m_pBitmap_A80 = g_pFresh->RegisterBitmap("clock");
	{
		float w = (float)m_pBitmap_A7C->Width();
		float h = (float)m_pBitmap_A7C->Height();
		m_WRect_A6C.x = 32.0f;
		m_WRect_A6C.y = 30.0f;
		m_WRect_A6C.w = w;
		m_WRect_A6C.h = h;
	}
	if (IsLocalContent(S4_REPLAY_SYSTEM))
	{
		m_pBitmap_AB4 = g_pFresh->RegisterBitmap("replaymode_btn_n");
		m_pBitmap_AB8 = g_pFresh->RegisterBitmap("replaymode_btn_o");
		{
			float w = (float)m_pBitmap_AB4->Width();
			float h = (float)m_pBitmap_AB4->Height();
			m_WRect_AA4.x = 144.0f;
			m_WRect_AA4.y = 66.0f;
			m_WRect_AA4.w = w;
			m_WRect_AA4.h = h;
		}
		m_pBitmap_AD0 = g_pFresh->RegisterBitmap("replaydel_btn_n");
		m_pBitmap_AD4 = g_pFresh->RegisterBitmap("replaydel_btn_o");
		{
			float w = (float)m_pBitmap_AD0->Width();
			float h = (float)m_pBitmap_AD0->Height();
			m_WRect_AC0.x = 103.0f;
			m_WRect_AC0.y = 66.0f;
			m_WRect_AC0.w = w;
			m_WRect_AC0.h = h;
		}
	}
	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		m_pBitmap_AEC = g_pFresh->RegisterBitmap("card_btn_n");
		m_pBitmap_AF0 = g_pFresh->RegisterBitmap("card_btn_o");
		{
			float w = (float)m_pBitmap_AEC->Width();
			float h = (float)m_pBitmap_AEC->Height();
			m_WRect_ADC.x = 144.0f;
			m_WRect_ADC.y = 66.0f;
			m_WRect_ADC.w = w;
			m_WRect_ADC.h = h;
		}
	}
	if (IsLocalContent(S4_UCC))
	{
		m_pBitmap_B24 = g_pFresh->RegisterBitmap("ucc_draw_n");
		m_pBitmap_B28 = g_pFresh->RegisterBitmap("ucc_draw_o");
		m_pBitmap_B5C = g_pFresh->RegisterBitmap("ucc_redraw_btn_n");
		m_pBitmap_B60 = g_pFresh->RegisterBitmap("ucc_redraw_btn_o");
		m_pBitmap_B40 = g_pFresh->RegisterBitmap("ucc_copy_n");
		m_pBitmap_B44 = g_pFresh->RegisterBitmap("ucc_copy_o");
		{
			float w = (float)m_pBitmap_B24->Width();
			float h = (float)m_pBitmap_B24->Height();
			m_WRect_C28.x = 103.0f;
			m_WRect_C28.y = 66.0f;
			m_WRect_C28.w = w;
			m_WRect_C28.h = h;
		}
		{
			float w = (float)m_pBitmap_B24->Width();
			float h = (float)m_pBitmap_B24->Height();
			m_WRect_C38.x = 144.0f;
			m_WRect_C38.y = 66.0f;
			m_WRect_C38.w = w;
			m_WRect_C38.h = h;
		}
		m_pBitmap_5C0 = g_pFresh->RegisterBitmap("ucc_state_original");
		m_pBitmap_5C4 = g_pFresh->RegisterBitmap("ucc_state_copied");
		m_pBitmap_5C8 = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
	}
	if (IsLocalContent(S4_REAL_MYROOM))
	{
		m_pBitmap_B08 = g_pFresh->RegisterBitmap("rmr_arrange_btn_n");
		m_pBitmap_B0C = g_pFresh->RegisterBitmap("rmr_arrange_btn_o");
		{
			float w = (float)m_pBitmap_B08->Width();
			float h = (float)m_pBitmap_B08->Height();
			m_WRect_AF8.x = 144.0f;
			m_WRect_AF8.y = 66.0f;
			m_WRect_AF8.w = w;
			m_WRect_AF8.h = h;
		}
	}
	m_pBitmap_550 = g_pFresh->RegisterBitmap("i_male");
	m_pBitmap_554 = g_pFresh->RegisterBitmap("i_female");
	m_pBitmap_558 = g_pFresh->RegisterBitmap("master_sign");
	m_pBitmap_55C = g_pFresh->RegisterBitmap("team_base");
	m_pSlotImg[0] = g_pFresh->RegisterBitmap("item_slot_n");
	m_pSlotImg[1] = g_pFresh->RegisterBitmap("item_slot_o");
	m_pSlotImg[2] = g_pFresh->RegisterBitmap("item_slot_d");
	m_pBitmap_56C = g_pFresh->RegisterBitmap("item_slot_empty");
	m_pBitmap_570 = g_pFresh->RegisterBitmap("itemdesc_01");
	m_pBitmap_574 = g_pFresh->RegisterBitmap("itemdesc_02");
	m_pBitmap_5AC = g_pFresh->RegisterBitmap("gift_arrow_white_2");
	m_pBitmap_5B0 = g_pFresh->RegisterBitmap("gift_number_01");
	m_pBitmap_5B4 = g_pFresh->RegisterBitmap("gift_number_02");
	m_pBitmap_5B8 = g_pFresh->RegisterBitmap("gift_number_03");
	m_pBitmap_5BC = g_pFresh->RegisterBitmap("gift_arrow_white");
	m_pBitmap_640 = g_pFresh->RegisterBitmap("rmr_arg_item_base_n");
	m_pBitmap_644 = g_pFresh->RegisterBitmap("rmr_arg_item_base_d");
	m_pBitmap_648 = g_pFresh->RegisterBitmap("rmr_arg_item_base_s");
	m_pBitmap_64C = g_pFresh->RegisterBitmap("rmr_arg_item_base_empty");
	m_pBitmap_588 = g_pFresh->RegisterBitmap("rmr_tip_frm_01");
	m_pBitmap_58C = g_pFresh->RegisterBitmap("rmr_tip_frm_02");
	m_pBitmap_590 = g_pFresh->RegisterBitmap("rmr_tip_frm_03");
	m_pBitmap_594 = g_pFresh->RegisterBitmap("rmr_tip_frm_04");
	m_pBitmap_598 = g_pFresh->RegisterBitmap("rmr_tip_frm_05");
	m_pBitmap_59C = g_pFresh->RegisterBitmap("rmr_tip_frm_06");
	m_pBitmap_5A0 = g_pFresh->RegisterBitmap("rmr_tip_frm_07");
	m_pBitmap_5A4 = g_pFresh->RegisterBitmap("rmr_tip_frm_08");
	m_pBitmap_5A8 = g_pFresh->RegisterBitmap("rmr_tip_frm_09");
	m_pBitmap_5CC = g_pFresh->RegisterBitmap("rmr_f_wardrobe");
	m_pBitmap_5D0 = g_pFresh->RegisterBitmap("rmr_f_post");
	m_pBitmap_5D4 = g_pFresh->RegisterBitmap("rmr_f_arrange");
	m_pBitmap_5D8 = g_pFresh->RegisterBitmap("rmr_f_shop");
	m_pBitmap_5DC = g_pFresh->RegisterBitmap("rmr_f_shop");
	m_pBitmap_5E0 = g_pFresh->RegisterBitmap("rmr_f_album");
	m_pBitmap_5E4 = g_pFresh->RegisterBitmap("rmr_f_tv");
	m_pBitmap_5E8 = g_pFresh->RegisterBitmap("rmr_f_visitors_book");
	m_pBitmap_5EC = g_pFresh->RegisterBitmap("rmr_f_authority");
	m_pBitmap_5FC = g_pFresh->RegisterBitmap("rmr_f_close");
	m_pBitmap_5F0 = g_pFresh->RegisterBitmap("rmr_f_camera");
	m_pBitmap_5F4 = g_pFresh->RegisterBitmap("rmr_f_hide");
	m_pBitmap_600 = g_pFresh->RegisterBitmap("rmr_f_wardrobe_text");
	m_pBitmap_604 = g_pFresh->RegisterBitmap("rmr_f_post_text");
	m_pBitmap_608 = g_pFresh->RegisterBitmap("rmr_f_arrange_text");
	m_pBitmap_60C = g_pFresh->RegisterBitmap("rmr_f_shop_text");
	m_pBitmap_610 = g_pFresh->RegisterBitmap("rmr_f_shop_text");
	m_pBitmap_614 = g_pFresh->RegisterBitmap("rmr_f_album_text");
	m_pBitmap_618 = g_pFresh->RegisterBitmap("rmr_f_tv_text");
	m_pBitmap_61C = g_pFresh->RegisterBitmap("rmr_f_visitors_book_text");
	m_pBitmap_620 = g_pFresh->RegisterBitmap("rmr_f_authority_text");
	m_pBitmap_630 = g_pFresh->RegisterBitmap("rmr_f_close_text");
	m_pBitmap_624 = g_pFresh->RegisterBitmap("rmr_f_camera_text");
	m_pBitmap_628 = g_pFresh->RegisterBitmap("rmr_f_hide_text");
	if (IsLocalContent(S4_ITEM_STORAGE))
	{
		m_pBitmap_5F8 = g_pFresh->RegisterBitmap("rmr_f_secbox");
		m_pBitmap_62C = g_pFresh->RegisterBitmap("rmr_f_secbox_text");
	}
	m_pBitmap_634 = g_pFresh->RegisterBitmap("rmr_menu_o");
	m_pBitmap_638 = g_pFresh->RegisterBitmap("rmr_menu_d");
	m_pBitmap_650 = g_pFresh->RegisterBitmap("mark_begin");
}

void CRealMyRoomMain::LoadItemImages()
{
	{
		for (std::map<unsigned int, IFF_STRUCT::sChar>::iterator it =
				 ItemManager()->m_CharMap.begin();
			it != ItemManager()->m_CharMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		std::map<unsigned int, IFF_STRUCT::sPart>::iterator it;
		for (it = ItemManager()->m_PartMap.begin();
			it != ItemManager()->m_PartMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end() &&
				((*it).second.c.TypeId & 0x600) == 0)
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator it;
		for (it = ItemManager()->m_CaddieMap.begin();
			it != ItemManager()->m_CaddieMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sCadItem>::iterator it =
				 ItemManager()->m_CadItemMap.begin();
			it != ItemManager()->m_CadItemMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sItem>::iterator it =
				 ItemManager()->m_ItemMap.begin();
			it != ItemManager()->m_ItemMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sClubSet>::iterator it =
				 ItemManager()->m_ClubSetMap.begin();
			it != ItemManager()->m_ClubSetMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sBall>::iterator it =
				 ItemManager()->m_BallMap.begin();
			it != ItemManager()->m_BallMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sSetItem>::iterator it =
				 ItemManager()->m_SetItemMap.begin();
			it != ItemManager()->m_SetItemMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sSkin>::iterator it =
				 ItemManager()->m_SkinMap.begin();
			it != ItemManager()->m_SkinMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sHairStyle>::iterator it =
				 ItemManager()->m_HairStyleMap.begin();
			it != ItemManager()->m_HairStyleMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		std::map<unsigned int, IFF_STRUCT::sAuxPart>::iterator it;
		for (it = ItemManager()->m_AuxPartMap.begin();
			it != ItemManager()->m_AuxPartMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sQuestDrop>::iterator it =
				 ItemManager()->m_QuestDropMap.begin();
			it != ItemManager()->m_QuestDropMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sQuest>::iterator it =
				 ItemManager()->m_QuestMap.begin();
			it != ItemManager()->m_QuestMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.TypeId, pBitmap));
			}
		}
	}
	{
		for (std::map<unsigned int, IFF_STRUCT::sMascot>::iterator it =
				 ItemManager()->m_MascotMap.begin();
			it != ItemManager()->m_MascotMap.end(); ++it)
		{
			if (m_iconMap.find((*it).second.c.TypeId) == m_iconMap.end())
			{
				const Bitmap* pBitmap =
					g_pFresh->RegisterBitmap((*it).second.c.Icon);
				if (pBitmap)
					m_iconMap.insert(
						std::map<unsigned long, const Bitmap*>::value_type(
							(*it).second.c.TypeId, pBitmap));
			}
		}
	}
}

void CRealMyRoomMain::CloseAllDialogs()
{
	FrWnd* pWnd = DYNAMIC_CAST(FrWnd,
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop"));
	if (pWnd)
		pWnd->Close(true);
	CTaskMain::CloseAllDialogs();
}

void CRealMyRoomMain::OnMiddleUIInit(int param)
{
	m_pFrGroupBox_2E4 = DYNAMIC_CAST(FrGroupBox, (FrWnd*)param);
}

void CRealMyRoomMain::OnTabTxt1Init(int param)
{
	m_pFrArea_308 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnTabTxt2Init(int param)
{
	m_pFrArea_30C = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnTabTxt3Init(int param)
{
	m_pFrArea_310 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnSelectedNameInit(int param)
{
	m_pFrStatic_2E8 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

__forceinline void CRealMyRoomMain::HidePetNow()
{
	m_hand.WaitUntilWorkDone(9999);
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition != NULL)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
}

void CRealMyRoomMain::ChangeSubTab(subTab_t tab, bool bForce)
{
	if (!bForce && m_n_77C == tab)
		return;
	int cur = m_n_77C;
	bForce = false;
	if (((cur == 0x15 || cur <= 4 || cur == 0x16 || cur == 0x1a ||
			 cur == 0x10 || cur == 0x21) &&
			(tab == 0xd || tab == 0xe || tab == 0xf)) ||
		((cur == 0xd || cur == 0xe || cur == 0xf || cur == 0x21) &&
			m_roomMode == 0) ||
		(m_roomMode == 0 && cur == 0x10))
		bForce = true;
	if (tab == 5)
		tab = (subTab_t)1;
	m_n_77C = tab;
	m_b_6D8 = 0;
	BuildDisplayStand(tab);
	SetLevelBar();
	m_pFrEdit_360->ClearLine();
	m_pFrTabButton_2F8->Select(m_n_77C);
	if (m_pFrViewer_39C)
		m_pFrViewer_39C->SetVisible(false);
	if (m_pFrEdit_3A0)
		m_pFrEdit_3A0->SetVisible(false);
	m_pFrGroupBox_328->SetVisible(false);
	if (!m_pFrListBox_304->IsVisible())
	{
		m_pFrListBox_304->SetVisible(true);
		SetCardBookVisible(false);
	}
	switch (tab)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 0x15:
	case 0x16:
	case 0x1a:
		m_pFrListBox_304->SetMultiSelect(true);
		if (bForce || m_n_D4C != 1)
			SafeLoadPet(&m_tidChar, bForce);
		m_pFrGroupBox_328->SetVisible(true);
		break;
	case 6:
		if (m_pFrStatic_2E8)
		{
			m_pFrStatic_2E8->SetVisible(true);
			m_pFrStatic_2E8->SetCaption11(
				"\xbc\xb1\xc5\xc3\xc7\xd1 \xc5\xac\xb7\xb4");
		}
		m_pFrListBox_304->SetMultiSelect(false);
		ChangeClubset(m_tidClub);
		m_pFrGroupBox_328->SetVisible(true);
		break;
	case 7:
		m_pFrListBox_304->SetMultiSelect(false);
		ChangeAztec(m_tidAztec);
		break;
	case 8:
	case 9:
case_hide:
		m_pFrListBox_304->SetMultiSelect(false);
		HidePetNow();
		break;
	case 10:
		m_pFrListBox_304->SetMultiSelect(false);
		m_pFrGroupBox_328->SetVisible(true);
		break;
	case 0xb:
	case 0x18:
	case 0x19:
	case 0x21:
		break;
	case 0xc:
	case 0x13:
		m_pFrListBox_304->SetMultiSelect(false);
		break;
	case 0xd:
	case 0xe:
	case 0xf:
	case 0x10:
		m_pFrListBox_304->SetMultiSelect(false);
		SafeLoadPet(&m_tidChar, bForce);
		break;
	case 0x11:
		if (!IsLocalContent((localContentType_t)0x61 /*983: era 0x60; medido*/))
		{
			m_pFrListBox_304->SetMultiSelect(false);
			BuildDisplayStand((subTab_t)0x11);
			HidePetNow();
			if (m_pFrViewer_39C)
				m_pFrViewer_39C->SetVisible(true);
			if (m_pFrEdit_3A0)
				m_pFrEdit_3A0->SetVisible(true);
			if (IsLocalCountry(0x5b))
			{
				m_giftTotal = *((unsigned char*)&CSharedDoc::m_cardPackItem);
				m_giftSel = *((unsigned char*)&CSharedDoc::m_cardPackSlot);
			}
			else
			{
				unsigned int a = *(unsigned int*)((char*)&CSharedDoc::m_cardPackPageNum);
				unsigned int b = *(unsigned int*)((char*)&CSharedDoc::m_cardPackCurPage);
				if (a != 0 && b != 0)
				{
					m_giftTotal = b / a;
					if (b % a != 0)
						m_giftTotal = b / a + 1;
				}
				m_giftSel = *(unsigned int*)((char*)&CSharedDoc::m_cardPackMode);
			}
		}
		break;
	case 0x12:
	case 0x14:
		m_pFrListBox_304->SetMultiSelect(false);
		break;
	case 0x17:
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
			goto case_hide;
		break;
	case 0x1b:
		if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
		{
			m_pFrListBox_304->SetMultiSelect(false);
			m_pFrListBox_304->SetVisible(false);
			HidePetNow();
			SetCardBookVisible(true);
		}
		break;
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
		if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		{
			m_pFrListBox_304->SetMultiSelect(false);
			if (m_pFrButton_380)
				m_pFrButton_380->SetVisible(true);
			goto case_hide;
		}
		break;
	case 0x22:
		TurnOffItemStatus();
		goto case_hide;
	}
	if (m_n_77C == 8)
	{
		if (m_pFrGroupBox_384)
			m_pFrGroupBox_384->SetVisible(true);
	}
	else if (m_pFrGroupBox_384)
		m_pFrGroupBox_384->SetVisible(false);
	bool bShow;
	switch (m_n_77C)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 0x15:
	case 0x16:
	case 0x1a:
		m_pFrButton_390->SetVisible(true);
		bShow = true;
		break;
	default:
		m_pFrButton_390->SetVisible(false);
		bShow = false;
	}
	m_pFrButton_394->SetVisible(bShow);
	if (m_n_77C > 0xc && m_n_77C < 0x10)
		m_pFrWnd_314 = m_pFrArea_370;
	else if (m_n_77C == 0x10)
		m_pFrWnd_314 = m_pFrArea_374;
	else
		m_pFrWnd_314 = m_pFrArea_378;
	SetItemDesc();
	if (m_pFrListBox_304)
	{
		FrScrollBar* pScroll = m_pFrListBox_304->GetScrollBar();
		if (pScroll)
			pScroll->SetCurTopRow(0);
	}
}

void CRealMyRoomMain::SetPetAreaRect()
{
	int sub = m_n_77C;
	if (sub >= 13)
	{
		if (sub > 15)
		{
			if (sub == 16)
			{
				m_pFrWnd_314 = m_pFrArea_374;
				return;
			}
		}
		else
		{
			m_pFrWnd_314 = m_pFrArea_370;
			return;
		}
	}
	m_pFrWnd_314 = m_pFrArea_378;
}

// (a macro: as a __forceinline helper it was also emitted out of line)
#define MakeSubTabImg(names, index) \
	do \
	{ \
		(names)[0] = MakeStr("stext_%d_n", (index)); \
		(names)[1] = MakeStr("stext_%d_n", (index)); \
		(names)[2] = MakeStr("stext_%d_n", (index)); \
	} while (0)

void CRealMyRoomMain::ChangeMainTab(mainTab_t tab, bool bForce)
{
	if (bForce)
		m_pFrTabButton_2F4->Select(m_roomMode);
	else if (m_roomMode == tab)
		return;
	m_roomMode = tab;
	m_pFrTabButton_2F8->ClearTab();
	const char* names[3];
	names[1] = NULL;
	names[2] = NULL;
	names[0] = NULL;
	if (m_pFrArea_684)
		m_pFrArea_684->SetVisible(true);
	if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar)
		CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(false);
	switch (m_roomMode)
	{
	case 0:
	{
		m_pFrArea_308->SetBgImg("mt_character");
		TurnOnItemSelect();
		SetSubTabImg(names, 0x16);
		m_pFrTabButton_2F8->AddTab(names, 0x15);
		for (int i = 2; i < 6; i++)
		{
			MakeSubTabImg(names, i);
			m_pFrTabButton_2F8->AddTab(names, i - 1);
		}
		if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
		{
			SetSubTabImg(names, 0x1b);
			m_pFrTabButton_2F8->AddTab(names, 0x1a);
		}
		MakeSubTabImg(names, 0x17);
		m_pFrTabButton_2F8->AddTab(names, 0x16);
		break;
	}
	case 1:
	{
		m_pFrArea_308->SetBgImg("mt_item");
		if (m_pFrStatic_2E8)
			m_pFrStatic_2E8->SetVisible(false);
		TurnOffItemSelect();
		for (int i = 7; i < 11; i++)
		{
			MakeSubTabImg(names, i);
			m_pFrTabButton_2F8->AddTab(names, i - 1);
		}
		if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
		{
			SetSubTabImg(names, 0x1c);
			m_pFrTabButton_2F8->AddTab(names, 0x1b);
		}
		break;
	}
	case 2:
		m_pFrArea_308->SetBgImg("mt_caddie");
		TurnOffItemSelect();
		MakeSubTabImg(names, 0xb);
		m_pFrTabButton_2F8->AddTab(names, 10);
		break;
	case 3:
		m_pFrArea_308->SetBgImg("mt_mascot");
		TurnOffItemSelect();
		MakeSubTabImg(names, 0xd);
		m_pFrTabButton_2F8->AddTab(names, 0xc);
		break;
	case 4:
	{
		m_pFrArea_308->SetBgImg("mt_skin");
		TurnOffItemSelect();
		for (int i = 0xe; i < 0x12; i++)
		{
			MakeSubTabImg(names, i);
			m_pFrTabButton_2F8->AddTab(names, i - 1);
		}
		MakeSubTabImg(names, 0x22);
		m_pFrTabButton_2F8->AddTab(names, 0x21);
		MakeSubTabImg(names, 0x23);
		m_pFrTabButton_2F8->AddTab(names, 0x22);
		break;
	}
	case 5:
	{
		if (IsLocalContent((localContentType_t)0x5c /*983: era 0x5b; interpolado*/))
		{
			if (!*((char*)&CSharedDoc::m_bRmrTabPending))
			{
				WSendPacket packet(0x95); // JP 983 (s63): 0x92 no KR 645
				packet.Encode4(1);
				packet.Send(TO_GAME);
			}
			else
			{
				m_pFrArea_308->SetBgImg("mt_mybox");
				TurnOffItemSelect();
				for (unsigned long i =
						 IsLocalContent((localContentType_t)0x61 /*983: era 0x60; medido*/) + 0x11;
					i < 0x15; i++)
				{
					SetSubTabImg(names, i + 1);
					m_pFrTabButton_2F8->AddTab(names, i);
				}
				if (IsLocalContent((localContentType_t)0x14))
				{
					SetSubTabImg(names, 0x19);
					m_pFrTabButton_2F8->AddTab(names, 0x18);
				}
				if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
				{
					SetSubTabImg(names, 0x18);
					m_pFrTabButton_2F8->AddTab(names, 0x17);
				}
			}
		}
		else
		{
			m_pFrArea_308->SetBgImg("mt_mybox");
			TurnOffItemSelect();
			for (unsigned long i =
					 IsLocalContent((localContentType_t)0x61 /*983: era 0x60; medido*/) + 0x11;
				i < 0x15; i++)
			{
				SetSubTabImg(names, i + 1);
				m_pFrTabButton_2F8->AddTab(names, i);
			}
			if (IsLocalContent((localContentType_t)0x14))
			{
				SetSubTabImg(names, 0x19);
				m_pFrTabButton_2F8->AddTab(names, 0x18);
			}
			if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
			{
				SetSubTabImg(names, 0x18);
				m_pFrTabButton_2F8->AddTab(names, 0x17);
			}
		}
		break;
	}
	case 6:
	{
		m_pFrArea_308->SetBgImg("mt_interior");
		TurnOffItemSelect();
		SetSubTabImg(names, 0x16);
		m_pFrTabButton_2F8->AddTab(names, 0x20);
		for (int i = 0x1d; i < 0x21; i++)
		{
			MakeSubTabImg(names, i);
			m_pFrTabButton_2F8->AddTab(names, i - 1);
		}
		break;
	}
	}
	m_pFrEdit_360->SetVisible(false);
	m_pFrArea_35C->SetVisible(false);
	m_pUnderBarBack->Enable(false);
	if (m_pExhibition == NULL)
		ChangeSubTab((subTab_t)m_n_77C, true);
}

void CRealMyRoomMain::ChangeStatTab(statTab_t tab)
{
	m_n_774 = tab;
	SetLevelBar();
}

void CRealMyRoomMain::ChangeSelTab(selTab_t tab)
{
	m_selTab_t_780 = tab;
}

void CRealMyRoomMain::CharacterViewControl(bool bView)
{
	if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar)
	{
		if (bView == true)
		{
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetState(2);
		}
		else
		{
			CRealMyRoom_AvatarManager::Instance()
				->m_pAvatar->ChangeCharacterInfo();
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetState(0);
		}
		CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(!bView);
	}
}

void CRealMyRoomMain::ChangeCharacter(unsigned long tidChar)
{
	typedef std::map<unsigned int, sCharacterInfo>::iterator CharIter;

	CharIter it = Doc()->m_charMap.find(tidChar);
	if (it == Doc()->m_charMap.end())
		return;

	// the original hands the map node itself (key first) to BuildMyPartTidList
	sCharacterInfo& info = (sCharacterInfo&)*it;
	char* pNode = (char*)&*it;
	m_pParts = (unsigned long*)(pNode + 8);
	m_tidChar = it->first;
	if (IsLocalContent(S4_ITEM_STORAGE))
		memcpy(&m_partUid, pNode + 0x15c, 0x60);
	m_pAuxParts = (unsigned long*)(pNode + 0x70);
	m_pCharStat = pNode + 0x6a;
	m_n_74C = it->second.tid;
	Doc()->m_myInfo.userEquip.guidChar = it->second.tid;

	if (Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
	{
		if (m_roomMode == 0)
		{
			Doc()->BuildMyPartTidList(info);
			BuildDisplayStand((subTab_t)m_n_77C);
			m_hand.WaitUntilWorkDone(9999);
			if (m_pExhibition == NULL ||
				*(unsigned long*)((char*)m_pExhibition + 0x498) != m_tidChar)
			{
				m_hand.Work(static_cast<CHandOwner*>(this), 0, &m_tidChar,
					true, -1);
			}
			CRealMyRoom_Avatar* pAvatar =
				CRealMyRoom_AvatarManager::Instance()->FindAvatarByGuid(
					*(int*)DOCPTR983(0x81a));
			sCharacterInfo& curInfo =
				Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
			if (pAvatar &&
				*(int*)((char*)&pAvatar->GetCharacterInfo() + 4) != *(int*)((char*)&curInfo + 4))
			{
				unsigned long uid = *(unsigned long*)((char*)pAvatar + 0xdc);
				{
					CRealMyRoom_AvatarBurden burden(uid,
						pAvatar->GetCharacterInfo(),
						pAvatar->GetPetFrame());
					((CRealMyRoomTask*)GetTask())->RemoveAvatarBurden(burden);
				}
				pAvatar->SetCharactorInfo(curInfo);
				{
					CRealMyRoom_AvatarBurden burden(uid, curInfo, NULL);
					((CRealMyRoomTask*)GetTask())->AddAvatarBurden(burden);
				}
				if (m_b_D45 == 0)
				{
					CFxSequence* pSeq =
						CFx::Instance()->OpenSequence("Talk_Room.seq", true);
					if (pSeq)
					{
						WVector pos;
						pos.x = *(float*)((char*)pAvatar + 0xf4);
						pos.y = *(float*)((char*)pAvatar + 0xf8) + 5.0f;
						pos.z = *(float*)((char*)pAvatar + 0xfc);
						pSeq->m_pos = pos;
					}
				}
			}
			if (m_pFrViewer_2EC)
				m_pFrViewer_2EC->Enable(true);
			m_n_774 = 0;
			SetLevelBar();
			m_b_6D8 = 0;
			if (m_pFrButton_358 && m_pFrButton_354)
			{
				m_pFrButton_358->SetVisible(false);
				m_pFrButton_354->SetVisible(false);
				std::map<unsigned int, sCharacterInfo>& chars =
					Doc()->m_charMap;
				std::map<unsigned int, sCharacterInfo>::const_iterator cur =
					chars.begin();
				while (cur != chars.end())
				{
					if (cur->second.tid == tidChar)
					{
						++cur;
						if (cur != chars.end())
							m_pFrButton_358->SetVisible(true);
						break;
					}
					++cur;
				}
				std::map<unsigned int, sCharacterInfo>::const_iterator back =
					chars.end();
				while (back != chars.begin())
				{
					std::map<unsigned int, sCharacterInfo>::const_iterator
						prev = back;
					--prev;
					if (prev->second.tid == tidChar)
					{
						--back;
						if (back != chars.begin())
							m_pFrButton_354->SetVisible(true);
						break;
					}
					--back;
				}
			}
			const char* pName;
			if (m_tidChar != 0 &&
				(pName = Doc()->m_itemManager.GetItemName(m_tidChar)) != NULL &&
				m_pFrStatic_2E8)
			{
				m_pFrStatic_2E8->SetCaption11(pName);
			}
		}
		else if (m_roomMode == 4)
		{
			Doc()->BuildMyPartTidList(info);
			BuildDisplayStand((subTab_t)m_n_77C);
			SafeLoadPet(&m_tidChar, false);
		}
		else if (m_n_774 == 1)
		{
			SetLevelBar();
		}
		if (m_pFrListBox_304)
		{
			FrScrollBar* pScroll = m_pFrListBox_304->GetScrollBar();
			if (pScroll)
				pScroll->SetCurTopRow(0);
		}
	}
	else if (m_roomMode == 0)
	{
		Doc()->BuildMyPartTidList(info);
		CRealMyRoom_Avatar* pAvatar =
			CRealMyRoom_AvatarManager::Instance()->FindAvatarByGuid(
				*(int*)DOCPTR983(0x81a));
		sCharacterInfo& curInfo =
			Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		if (pAvatar &&
			*(int*)((char*)&pAvatar->GetCharacterInfo() + 4) != *(int*)((char*)&curInfo + 4))
		{
			unsigned long uid = *(unsigned long*)((char*)pAvatar + 0xdc);
			{
				CRealMyRoom_AvatarBurden burden(uid,
					pAvatar->GetCharacterInfo(),
					pAvatar->GetPetFrame());
				((CRealMyRoomTask*)GetTask())->RemoveAvatarBurden(burden);
			}
			pAvatar->SetCharactorInfo(curInfo);
			{
				CRealMyRoom_AvatarBurden burden(uid, curInfo, NULL);
				((CRealMyRoomTask*)GetTask())->AddAvatarBurden(burden);
			}
			if (m_b_D45 == 0)
			{
				CFxSequence* pSeq =
					CFx::Instance()->OpenSequence("Talk_Room.seq", true);
				if (pSeq)
				{
					WVector pos;
					pos.x = *(float*)((char*)pAvatar + 0xf4);
					pos.y = *(float*)((char*)pAvatar + 0xf8) + 5.0f;
					pos.z = *(float*)((char*)pAvatar + 0xfc);
					pSeq->m_pos = pos;
				}
			}
		}
	}
}

bool CRealMyRoomMain::ChangeCaddie(unsigned long guid)
{
	typedef std::map<unsigned int, sCaddieInfo>::const_iterator CaddieIter;
	bool result = false;
	CaddieIter it = Doc()->m_caddieMap.find(guid);
	if (it == Doc()->m_caddieMap.end())
	{
		m_tidCaddie = 0;
		m_n_754 = 0;
		// 983 0061c960: zera doc+0xa26 (m_myInfo.userEquip), nao o m_realEquip (0x1103c)
		Doc()->m_myInfo.userEquip.guidCaddie = 0;
		if (m_n_77C == 10)
		{
			m_hand.WaitUntilWorkDone(9999);
			// 983 00605530: apaga a vitrine, zera o ponteiro e segue (o nosso saia com
			// m_pExhibition pendurado, sem desmarcar a lista nem limpar a ficha)
			if (m_pExhibition)
			{
				delete m_pExhibition;
				m_pExhibition = NULL;
			}
			m_pFrListBox_304->UnselectAllItem();
			if (m_pFrViewer_2EC)
				m_pFrViewer_2EC->Enable(true);
			m_n_774 = 0;
			SetLevelBar();
			if (m_pFrStatic_2E8)
			{
				m_pFrStatic_2E8->SetCaption11("");
			}
		}
		if (guid == 0)
			Doc()->m_realEquip.guidCaddie = 0;
	}
	else
	{
		m_n_754 = (*it).second.guid;
		m_tidCaddie = (*it).second.tid;
		m_n_758 =
			(*it).second.tidPart ? (*it).second.tidPart : (*it).second.tid;
		IFF_STRUCT::sCaddie* pCaddie =
			Doc()->m_itemManager.FindCaddie(m_tidCaddie);
		if (pCaddie && pCaddie->c.Level <= Doc()->m_myInfo.stat.Level)
		{
			// 983 0061c960: doc+0xa26 (userEquip, lido pelo BuildDisplayStand) e 0x1103c
			Doc()->m_myInfo.userEquip.guidCaddie = (*it).second.guid;
			Doc()->m_realEquip.guidCaddie = (*it).second.guid;
			result = true;
		}
		if (m_n_77C == 10)
		{
			BuildDisplayStand((subTab_t)10);
			m_hand.WaitUntilWorkDone(9999);
			if (m_pExhibition == NULL ||
				*(int*)((char*)m_pExhibition + 0x498) != (int)m_n_758)
				m_hand.Work(this, 0, &m_n_758, true, -1);
			if (m_pFrViewer_2EC)
				m_pFrViewer_2EC->Enable(true);
			m_n_774 = 0;
			SetLevelBar();
			m_b_6D8 = false;
			m_pFrButton_358->Enable(false);
			m_pFrButton_354->Enable(false);
			CaddieIter itEnd = Doc()->m_caddieMap.end();
			CaddieIter itNext = Doc()->m_caddieMap.begin();
			while (itNext != itEnd)
			{
				if ((*itNext).second.guid == guid)
				{
					++itNext;
					if (itNext != itEnd)
						m_pFrButton_358->Enable(true);
					break;
				}
				++itNext;
			}
			CaddieIter itPrev = Doc()->m_caddieMap.end();
			CaddieIter itBegin = Doc()->m_caddieMap.begin();
			if (itPrev != itBegin)
			{
				for (;;)
				{
					CaddieIter tmp = itPrev;
					--tmp;
					if ((*tmp).second.guid == guid)
						break;
					--itPrev;
					if (itPrev == itBegin)
						goto done;
				}
				--itPrev;
				if (itPrev != itBegin)
					m_pFrButton_354->Enable(true);
			}
		}
done:
		if (result)
		{
			const char* pName = Doc()->m_itemManager.GetItemName(m_tidCaddie);
			if (pName && m_pFrStatic_2E8)
			{
				m_pFrStatic_2E8->SetCaption11(pName);
			}
		}
	}
	return result;
}

void CRealMyRoomMain::ChangeMascot(unsigned long guid)
{
	typedef std::map<unsigned int, sMascotInfo>::const_iterator MascotIter;
	MascotIter it = Doc()->m_mascotMap.find(guid);
	if (it == Doc()->m_mascotMap.end())
	{
		m_n_75C = 0;
		m_n_760 = 0;
		// 983 0061cb70: zera doc+0xa8e (m_myInfo.userEquip), nao o m_realEquip (0x110a4)
		Doc()->m_myInfo.userEquip.guidMascot = 0;
		if (m_n_77C == 0xc)
		{
			m_hand.WaitUntilWorkDone(9999);
			// 983 00605530: apaga a vitrine, zera o ponteiro e segue (o nosso saia com
			// m_pExhibition pendurado, sem desmarcar a lista nem enviar os slots)
			if (m_pExhibition)
			{
				delete m_pExhibition;
				m_pExhibition = NULL;
			}
			m_pFrListBox_304->UnselectAllItem();
		}
		if (guid == 0)
			Doc()->m_realEquip.guidMascot = 0;
	}
	else
	{
		m_n_75C = (*it).second.tid;
		m_n_760 = (*it).second.guid;
		// 983 0061cb70: doc+0xa8e (userEquip, lido pelo BuildDisplayStand) e 0x110a4
		Doc()->m_myInfo.userEquip.guidMascot = (*it).second.guid;
		Doc()->m_realEquip.guidMascot = (*it).second.guid;
		if (m_n_77C == 0xc)
		{
			BuildDisplayStand((subTab_t)0xc);
			m_hand.WaitUntilWorkDone(9999);
			if (m_pExhibition == NULL ||
				*(int*)((char*)m_pExhibition + 0x498) != m_n_75C)
				m_hand.Work(this, 0, (unsigned long*)&m_n_75C, true, -1);
			if (m_pFrViewer_2EC)
				m_pFrViewer_2EC->Enable(false);
			m_n_774 = 1;
			SetLevelBar();
			m_b_6D8 = false;
			m_pFrButton_354->Enable(false);
			m_pFrButton_358->Enable(false);
			MascotIter itEnd = Doc()->m_mascotMap.end();
			MascotIter itNext = Doc()->m_mascotMap.begin();
			while (itNext != itEnd)
			{
				if ((*itNext).second.guid == guid)
				{
					++itNext;
					if (itNext != itEnd)
						m_pFrButton_358->Enable(true);
					break;
				}
				++itNext;
			}
			MascotIter itPrev = Doc()->m_mascotMap.end();
			MascotIter itBegin = Doc()->m_mascotMap.begin();
			if (itPrev != itBegin)
			{
				for (;;)
				{
					MascotIter tmp = itPrev;
					--tmp;
					if ((*tmp).second.guid == guid)
						break;
					--itPrev;
					if (itPrev == itBegin)
						goto done;
				}
				--itPrev;
				if (itPrev != itBegin)
					m_pFrButton_354->Enable(true);
			}
		}
	}
done:
	if (!IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
		Doc()->SendEquipItemSlotByMascot();
	else
		Doc()->SendEquipItemSlot();
}

int CRealMyRoomMain::ChangeClubset(unsigned long guid)
{
	typedef std::map<unsigned int, sItemInfo>::const_iterator ClubIter;
	sItemInfo* pInfo;
	{
		ClubIter it = Doc()->m_clubSetMap.find(guid);
		if (it == Doc()->m_clubSetMap.end())
			return 0;
		pInfo = (sItemInfo*)&(*it).second;
	}
	m_tidClub = pInfo->tid;
	m_n_764 = pInfo->guid;
	m_pClubStat = (short*)pInfo->Common;
	int result = 0;
	IFF_STRUCT::sClubSet* pClubSet =
		Doc()->m_itemManager.FindClubSet(m_tidClub);
	if (pClubSet)
	{
		int level = pClubSet->c.Level;
		if (pClubSet->c.IsUnderLvl)
		{
			if (level < Doc()->m_myInfo.stat.Level)
			{
				for (ClubIter p = Doc()->m_clubSetMap.begin();
					p != Doc()->m_clubSetMap.end(); ++p)
				{
					if ((*p).second.tid == 0x10000000)
					{
						Doc()->m_myInfo.userEquip.guidClubSet =
							(*p).second.guid;
						m_tidClub = (*p).second.tid;
						m_n_764 = (*p).second.guid;
						m_pClubStat = (short*)(*p).second.Common;
						break;
					}
				}
				result = 2;
			}
			else
			{
				Doc()->m_myInfo.userEquip.guidClubSet = pInfo->guid;
				result = 1;
			}
		}
		else if (Doc()->m_myInfo.stat.Level >= level)
		{
			Doc()->m_myInfo.userEquip.guidClubSet = pInfo->guid;
			result = 1;
		}
	}
	if (m_n_77C == 6)
	{
		m_hand.WaitUntilWorkDone(9999);
		if (m_pExhibition == NULL ||
			*(unsigned long*)((char*)m_pExhibition + 0x498) != m_tidClub)
			m_hand.Work(this, 0, &m_tidClub, true, -1);
		if (m_pFrViewer_2EC)
			m_pFrViewer_2EC->Enable(true);
		m_n_774 = 0;
		SetLevelBar();
		m_b_6D8 = false;
		m_pFrButton_354->Enable(false);
		m_pFrButton_358->Enable(false);
		ClubIter itEnd = Doc()->m_clubSetMap.end();
		ClubIter itNext = Doc()->m_clubSetMap.begin();
		while (itNext != itEnd)
		{
			if ((*itNext).second.guid == guid)
			{
				++itNext;
				if (itNext != itEnd)
					m_pFrButton_358->Enable(true);
				break;
			}
			++itNext;
		}
		ClubIter itPrev = Doc()->m_clubSetMap.end();
		ClubIter itBegin = Doc()->m_clubSetMap.begin();
		if (itPrev != itBegin)
		{
			for (;;)
			{
				ClubIter tmp = itPrev;
				--tmp;
				if ((*tmp).second.guid == guid)
					break;
				--itPrev;
				if (itPrev == itBegin)
					return result;
			}
			--itPrev;
			if (itPrev != itBegin)
				m_pFrButton_354->Enable(true);
		}
	}
	return result;
}

struct sRbListNode
{
	sRbListNode* next;
	sRbListNode* prev;
	unsigned long pad;
	unsigned long tid;
};

void CRealMyRoomMain::ChangeAztec(unsigned long tid)
{
	m_tidAztec = tid;
	Doc()->m_myInfo.userEquip.tidBall = tid;
	if (m_n_77C == 7)
	{
		m_hand.WaitUntilWorkDone(9999);
		if (m_pExhibition == NULL ||
			*(unsigned long*)((char*)m_pExhibition + 0x498) != m_tidAztec)
			m_hand.Work(this, 0, &m_tidAztec, true, -1);
		if (m_pFrViewer_2EC)
			m_pFrViewer_2EC->Enable(false);
		m_n_774 = 1;
		SetLevelBar();
		m_b_6D8 = false;
		m_pFrButton_354->Enable(false);
		m_pFrButton_358->Enable(false);
		sRbListNode* pHead = *(sRbListNode**)&Doc()->m_ballList;
		for (sRbListNode* p = pHead->next; p != pHead; p = p->next)
		{
			if (p->tid == tid)
			{
				if (p->next != pHead)
					m_pFrButton_358->Enable(true);
				break;
			}
		}
		sRbListNode* pPrev = *(sRbListNode**)&Doc()->m_ballList;
		sRbListNode* pFirst = pPrev->next;
		if (pPrev != pFirst)
		{
			sRbListNode* pNode;
			while ((pNode = pPrev->prev)->tid != tid)
			{
				pPrev = pNode;
				if (pNode == pFirst)
					return;
			}
			if (pPrev->prev != pFirst)
				m_pFrButton_354->Enable(true);
		}
	}
}

const Bitmap* CRealMyRoomMain::GetItemIcon(unsigned long tid)
{
	const Bitmap* pBitmap = NULL;
	std::map<unsigned long, const Bitmap*>::iterator it = m_iconMap.find(tid);
	if (it == m_iconMap.end())
	{
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(tid);
		if (pItem)
		{
			pBitmap = g_pFresh->RegisterBitmap(pItem->Icon);
			if (pBitmap)
				m_iconMap.insert(
					std::map<unsigned long, const Bitmap*>::value_type(
						pItem->TypeId, pBitmap));
		}
		return pBitmap;
	}
	return (*it).second;
}

void CRealMyRoomMain::DrawSelItem(FrGraphicInterface* pGI, unsigned long tid)
{
	WRect rect = m_pFrArea_320->GetRect();
	const Bitmap* pBitmap = GetItemIcon(tid);
	if (pBitmap)
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
}

void CRealMyRoomMain::DrawCharacterList(FrGraphicInterface* pGI, unsigned long)
{
	if (!m_b_6D8)
		return;
	if (!Doc()->m_charMap.empty())
	{
		int count = (int)Doc()->m_charMap.size();
		int drawn = 0;
		int cols;
		if (count < 0x11)
		{
			if (count < 5)
				cols = (1 < count) + 1;
			else
				cols = 3;
		}
		else
			cols = 4;
		int rows = (count % cols != 0) + count / cols;
		float curX = 18.0f;
		float rowsH = (float)rows * 50.0f + (float)(rows - 1);
		float curY = 302.0f - rowsH;
		WRect frame(13.0f, curY - 5.0f,
			(float)cols * 46.0f + (float)(cols - 1) + 10.0f, rowsH + 10.0f);
		pGI->DrawTexture(m_pBitmap_4C8, frame, 0xffffffff, 0);
		WRect selRect(0.0f, 0.0f, 0.0f, 0.0f);
		WRect hotRect(0.0f, 0.0f, 0.0f, 0.0f);
		std::map<unsigned int, sCharacterInfo>::const_iterator it =
			Doc()->m_charMap.begin();
		for (; it != Doc()->m_charMap.end(); ++it)
		{
			const Bitmap* pIcon = GetItemIcon((*it).second.tid);
			if (pIcon)
			{
				drawn++;
				WRect icon(curX, curY, 46.0f, 50.0f);
				pGI->DrawTexture(pIcon, icon, 0xffffffff, 0);
				if (drawn % cols == 0)
				{
					curX = 18.0f;
					curY += 51.0f;
				}
				else
					curX += 47.0f;
				if ((unsigned long)m_n_74C == (*it).second.guid)
					selRect = WRect(icon.x - 4.0f, icon.y - 4.0f, icon.w + 8.0f,
						icon.h + 8.0f);
				const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
				if (icon.x <= mouse.x && mouse.x <= icon.x + icon.w &&
					icon.y <= mouse.y && mouse.y <= icon.y + icon.h)
				{
					hotRect = WRect(icon.x - 4.0f, icon.y - 4.0f, icon.w + 8.0f,
						icon.h + 8.0f);
					if (g_input->GetButton(LEFT_BUTTON) == 1 &&
						!m_hand.IsWorking(0))
						ChangeCharacter((*it).second.guid);
				}
			}
		}
		if (selRect.w != 0.0f)
			pGI->DrawTexture(m_pBitmap_4D8, selRect, 0xffffffff, 0);
		if (hotRect.w != 0.0f)
			pGI->DrawTexture(m_pBitmap_4DC, hotRect, 0xffffffff, 0);
	}
	if (m_b_6D8 && g_input->GetButton(LEFT_BUTTON) == 1)
	{
		WRect area = m_pFrButton_324->GetRect();
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (mouse.x < area.x || area.x + area.w <= mouse.x || mouse.y < area.y)
			m_b_6D8 = false;
		else if (area.y + area.h < mouse.y)
			m_b_6D8 = false;
	}
}

void CRealMyRoomMain::DrawCaddieList(FrGraphicInterface* pGI, unsigned long)
{
	if (!m_b_6D8)
		return;
	if (!Doc()->m_caddieMap.empty())
	{
		int count = (int)Doc()->m_caddieMap.size();
		int drawn = 0;
		int cols;
		if (count < 0x11)
		{
			if (count < 5)
				cols = (1 < count) + 1;
			else
				cols = 3;
		}
		else
			cols = 4;
		int rows = (count % cols != 0) + count / cols;
		float curX = 18.0f;
		float rowsH = (float)rows * 50.0f + (float)(rows - 1);
		float curY = 302.0f - rowsH;
		WRect frame(13.0f, curY - 5.0f,
			(float)cols * 46.0f + (float)(cols - 1) + 10.0f, rowsH + 10.0f);
		pGI->DrawTexture(m_pBitmap_4C8, frame, 0xffffffff, 0);
		WRect selRect(0.0f, 0.0f, 0.0f, 0.0f);
		WRect hotRect(0.0f, 0.0f, 0.0f, 0.0f);
		std::map<unsigned int, sCaddieInfo>::const_iterator it =
			Doc()->m_caddieMap.begin();
		for (; it != Doc()->m_caddieMap.end(); ++it)
		{
			const Bitmap* pIcon = GetItemIcon((*it).second.tid);
			if (pIcon)
			{
				drawn++;
				WRect icon(curX, curY, 46.0f, 50.0f);
				pGI->DrawTexture(pIcon, icon, 0xffffffff, 0);
				if (drawn % cols == 0)
				{
					curX = 18.0f;
					curY += 51.0f;
				}
				else
					curX += 47.0f;
				if ((unsigned long)m_n_754 == (*it).second.guid)
					selRect = WRect(icon.x - 4.0f, icon.y - 4.0f, icon.w + 8.0f,
						icon.h + 8.0f);
				const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
				if (icon.x <= mouse.x && mouse.x <= icon.x + icon.w &&
					icon.y <= mouse.y && mouse.y <= icon.y + icon.h)
				{
					hotRect = WRect(icon.x - 4.0f, icon.y - 4.0f, icon.w + 8.0f,
						icon.h + 8.0f);
					if (g_input->GetButton(LEFT_BUTTON) == 1 &&
						!m_hand.IsWorking(0))
						ChangeCaddie((*it).second.guid);
				}
			}
		}
		if (selRect.w != 0.0f)
			pGI->DrawTexture(m_pBitmap_4D8, selRect, 0xffffffff, 0);
		if (hotRect.w != 0.0f)
			pGI->DrawTexture(m_pBitmap_4DC, hotRect, 0xffffffff, 0);
	}
	if (m_b_6D8 && g_input->GetButton(LEFT_BUTTON) == 1)
	{
		WRect area = m_pFrButton_324->GetRect();
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (mouse.x < area.x || area.x + area.w <= mouse.x || mouse.y < area.y)
			m_b_6D8 = false;
		else if (area.y + area.h < mouse.y)
			m_b_6D8 = false;
	}
}

void CRealMyRoomMain::DrawMascotList(FrGraphicInterface* pGI, unsigned long)
{
	if (!m_b_6D8)
		return;
	if (!Doc()->m_mascotMap.empty())
	{
		int count = (int)Doc()->m_mascotMap.size();
		int drawn = 0;
		int cols;
		if (count < 0x11)
		{
			if (count < 5)
				cols = (1 < count) + 1;
			else
				cols = 3;
		}
		else
			cols = 4;
		int rows = (count % cols != 0) + count / cols;
		float curX = 18.0f;
		float rowsH = (float)rows * 50.0f + (float)(rows - 1);
		float curY = 302.0f - rowsH;
		WRect frame(13.0f, curY - 5.0f,
			(float)cols * 46.0f + (float)(cols - 1) + 10.0f, rowsH + 10.0f);
		pGI->DrawTexture(m_pBitmap_4C8, frame, 0xffffffff, 0);
		WRect selRect(0.0f, 0.0f, 0.0f, 0.0f);
		WRect hotRect(0.0f, 0.0f, 0.0f, 0.0f);
		std::map<unsigned int, sMascotInfo>::const_iterator it =
			Doc()->m_mascotMap.begin();
		for (; it != Doc()->m_mascotMap.end(); ++it)
		{
			const Bitmap* pIcon = GetItemIcon((*it).second.tid);
			if (pIcon)
			{
				drawn++;
				WRect icon(curX, curY, 46.0f, 50.0f);
				pGI->DrawTexture(pIcon, icon, 0xffffffff, 0);
				if (drawn % cols == 0)
				{
					curX = 18.0f;
					curY += 51.0f;
				}
				else
					curX += 47.0f;
				if ((unsigned long)m_n_760 == (*it).second.guid)
					selRect = WRect(icon.x - 4.0f, icon.y - 4.0f, icon.w + 8.0f,
						icon.h + 8.0f);
				const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
				if (icon.x <= mouse.x && mouse.x <= icon.x + icon.w &&
					icon.y <= mouse.y && mouse.y <= icon.y + icon.h)
				{
					hotRect = WRect(icon.x - 4.0f, icon.y - 4.0f, icon.w + 8.0f,
						icon.h + 8.0f);
					if (g_input->GetButton(LEFT_BUTTON) == 1 &&
						!m_hand.IsWorking(0))
						ChangeMascot((*it).second.guid);
				}
			}
		}
		if (selRect.w != 0.0f)
			pGI->DrawTexture(m_pBitmap_4D8, selRect, 0xffffffff, 0);
		if (hotRect.w != 0.0f)
			pGI->DrawTexture(m_pBitmap_4DC, hotRect, 0xffffffff, 0);
	}
	if (m_b_6D8 && g_input->GetButton(LEFT_BUTTON) == 1)
	{
		WRect area = m_pFrButton_324->GetRect();
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (mouse.x < area.x || area.x + area.w <= mouse.x || mouse.y < area.y)
			m_b_6D8 = false;
		else if (area.y + area.h < mouse.y)
			m_b_6D8 = false;
	}
}

void CRealMyRoomMain::DrawAztecList(FrGraphicInterface* pGI, unsigned long)
{
	if (!m_b_6D8)
		return;
	int count = (int)Doc()->m_ballList.size();
	if (count != 0)
	{
		int drawn = 0;
		int row = 1;
		int rows = (count % 4 != 0) + count / 4;
		float curX = 18.0f;
		float curY = 407.0f - (float)rows * 50.0f + (float)(rows - 1) * 5.0f;
		float frameY = curY - 12.0f;
		float frameH = (float)rows * 50.0f + 15.0f;
		WRect r;
		r.x = 6.0f;
		r.y = frameY;
		r.w = 223.0f;
		r.h = 12.0f;
		pGI->DrawTexture(m_pBitmap_4C8, r, 0xffffffff, 0);
		r.y = frameY + 12.0f;
		r.h = frameH;
		r.x = 6.0f;
		r.w = 223.0f;
		pGI->DrawTexture(m_pBitmap_4CC, r, 0xffffffff, 0);
		r.y = frameH + frameY + 12.0f;
		r.x = 6.0f;
		r.w = 223.0f;
		r.h = 20.0f;
		pGI->DrawTexture(m_pBitmap_4D0, r, 0xffffffff, 0);
		std::list<sItemInfo>::const_iterator it;
		for (it = Doc()->m_ballList.begin(); it != Doc()->m_ballList.end();
			++it)
		{
			unsigned long tid = (*it).tid;
			const Bitmap* pIcon = GetItemIcon(tid);
			if (pIcon)
			{
				drawn++;
				WRect icon;
				icon.x = curX;
				icon.y = curY;
				icon.w = 46.0f;
				icon.h = 50.0f;
				pGI->DrawTexture(pIcon, icon, 0xffffffff, 0);
				if (drawn % 4 == 0)
				{
					row++;
					curY += 55.0f;
					curX = 18.0f;
				}
				else
					curX += 51.0f;
				r.x = icon.x - 6.0f;
				r.y = icon.y - 6.0f;
				r.w = icon.w + 12.0f;
				r.h = icon.h + 12.0f;
				if (m_tidAztec == tid)
					pGI->DrawTexture(m_pBitmap_4D8, r, 0xffffffff, 0);
				const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
				if (icon.x <= mouse.x && mouse.x <= icon.x + icon.w &&
					icon.y <= mouse.y && mouse.y <= icon.y + icon.h)
				{
					pGI->DrawTexture(m_pBitmap_4DC, r, 0xffffffff, 0);
					if (g_input->GetButton(LEFT_BUTTON) == 1 &&
						!m_hand.IsWorking(0))
						ChangeAztec(tid);
				}
			}
		}
		if (rows == row)
		{
			int fill = 4 - drawn % 4;
			if (fill != 0 && 0 < fill)
			{
				do
				{
					r.x = curX;
					r.y = curY;
					r.w = 46.0f;
					r.h = 50.0f;
					pGI->DrawTexture(m_pBitmap_4D4, r, 0xffffffff, 0);
					fill--;
					curX += 51.0f;
				} while (fill != 0);
			}
		}
	}
	if (m_b_6D8 && g_input->GetButton(LEFT_BUTTON) == 1)
	{
		WRect area = m_pFrButton_324->GetRect();
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (mouse.x < area.x || area.x + area.w <= mouse.x || mouse.y < area.y)
			m_b_6D8 = false;
		else if (area.y + area.h < mouse.y)
			m_b_6D8 = false;
	}
}

void CRealMyRoomMain::DrawClubList(FrGraphicInterface* pGI, unsigned long)
{
	if (!m_b_6D8)
		return;
	if (!Doc()->m_clubSetMap.empty())
	{
		int count = (int)Doc()->m_clubSetMap.size();
		int drawn = 0;
		int row = 1;
		int rows = (count % 4 != 0) + count / 4;
		float curX = 18.0f;
		float curY = 407.0f - (float)rows * 50.0f + (float)(rows - 1) * 5.0f;
		float frameY = curY - 12.0f;
		float frameH = (float)rows * 50.0f + 15.0f;
		WRect r;
		r.x = 6.0f;
		r.y = frameY;
		r.w = 223.0f;
		r.h = 12.0f;
		pGI->DrawTexture(m_pBitmap_4C8, r, 0xffffffff, 0);
		r.y = frameY + 12.0f;
		r.h = frameH;
		r.x = 6.0f;
		r.w = 223.0f;
		pGI->DrawTexture(m_pBitmap_4CC, r, 0xffffffff, 0);
		r.y = frameH + frameY + 12.0f;
		r.x = 6.0f;
		r.w = 223.0f;
		r.h = 20.0f;
		pGI->DrawTexture(m_pBitmap_4D0, r, 0xffffffff, 0);
		std::map<unsigned int, sItemInfo>::const_iterator it;
		for (it = Doc()->m_clubSetMap.begin(); it != Doc()->m_clubSetMap.end();
			++it)
		{
			const Bitmap* pIcon = GetItemIcon((*it).second.tid);
			if (pIcon)
			{
				drawn++;
				WRect icon;
				icon.x = curX;
				icon.y = curY;
				icon.w = 46.0f;
				icon.h = 50.0f;
				pGI->DrawTexture(pIcon, icon, 0xffffffff, 0);
				if (drawn % 4 != 0)
					curX += 51.0f;
				else
				{
					row++;
					curY += 55.0f;
					curX = 18.0f;
				}
				r.x = icon.x - 6.0f;
				r.y = icon.y - 6.0f;
				r.w = icon.w + 12.0f;
				r.h = icon.h + 12.0f;
				if (m_n_764 == (*it).second.guid)
					pGI->DrawTexture(m_pBitmap_4D8, r, 0xffffffff, 0);
				const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
				if (icon.x <= mouse.x && mouse.x <= icon.x + icon.w &&
					icon.y <= mouse.y && mouse.y <= icon.y + icon.h)
				{
					pGI->DrawTexture(m_pBitmap_4DC, r, 0xffffffff, 0);
					if (g_input->GetButton(LEFT_BUTTON) == 1 &&
						!m_hand.IsWorking(0))
						ChangeClubset((*it).second.guid);
				}
			}
		}
		if (rows == row)
		{
			int fill = 4 - drawn % 4;
			if (fill != 0 && 0 < fill)
			{
				do
				{
					r.x = curX;
					r.y = curY;
					r.w = 46.0f;
					r.h = 50.0f;
					pGI->DrawTexture(m_pBitmap_4D4, r, 0xffffffff, 0);
					fill--;
					curX += 51.0f;
				} while (fill != 0);
			}
		}
	}
	if (m_b_6D8 && g_input->GetButton(LEFT_BUTTON) == 1)
	{
		WRect area = m_pFrButton_324->GetRect();
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (mouse.x < area.x || area.x + area.w <= mouse.x || mouse.y < area.y)
			m_b_6D8 = false;
		else if (area.y + area.h < mouse.y)
			m_b_6D8 = false;
	}
}

void CRealMyRoomMain::OnPetInit(int param)
{
	m_pFrArea_378 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnSkinPetInit(int param)
{
	m_pFrArea_370 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnSkinPet2Init(int param)
{
	m_pFrArea_374 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::DrawSkinFrame()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;
	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pBitmap;
	if (m_n_B78 != 0 && (pSkin = ItemManager()->FindSkin(m_n_B78)) != NULL &&
		(pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
	{
		WRect* pArea = (WRect*)((char*)m_pFrWnd_314 + 0x20);
		WRect src;
		WRect dst;
		float areaX = pArea->x;
		dst.y = pArea->y;
		float areaW = pArea->w;
		float endY = pArea->h;
		float texW = (float)pBitmap->Width();
		float texH = (float)pBitmap->Height();
		if (m_scrollPos[0] > texW)
			m_scrollPos[0] = m_scrollPos[0] - texW;
		else if (m_scrollPos[0] < 0.0f)
			m_scrollPos[0] = texW + m_scrollPos[0];
		if (m_scrollPos[1] > texH)
			m_scrollPos[1] = m_scrollPos[1] - texH;
		else if (m_scrollPos[1] < 0.0f)
			m_scrollPos[1] = texH + m_scrollPos[1];
		src.x = m_scrollPos[0];
		src.w = texW - m_scrollPos[0];
		src.y = m_scrollPos[1];
		src.h = texH - m_scrollPos[1];
		endY = endY + dst.y;
		dst.x = areaX;
		dst.w = src.w;
		dst.h = src.h;
		float endY2 = endY;
		if (dst.y < endY)
		{
			float endX = areaW + areaX;
			do
			{
				float endX2 = endX;
				src.h = dst.h;
				dst.x = areaX;
				if (areaX < endX)
				{
					do
					{
						src.w = dst.w;
						pGI->DrawTexture(pBitmap, src, dst, 0xffffffff, 0);
						dst.x = dst.w + dst.x;
						dst.w = endX - dst.x;
						if (texW <= endX - dst.x)
							dst.w = texW;
						src.x = 0;
					} while (dst.x < endX2);
				}
				dst.y = dst.h + dst.y;
				src.w = texW - m_scrollPos[0];
				dst.h = endY2 - dst.y;
				if (texH <= dst.h)
					dst.h = texH;
				src.x = m_scrollPos[0];
				src.y = 0;
				src.h = dst.h;
				dst.x = areaX;
				dst.w = src.w;
			} while (dst.y < endY);
		}
	}
	CAnotherHand* pHand = &m_hand;
	if (!pHand->IsWorking(0) && *((bool*)this + 0x122) == false &&
		CRankingInfo::Instance()->GetDlg() == NULL &&
		(CUserInfo::Instance() == NULL ||
			CUserInfo::Instance()->GetDlg() == NULL))
	{
		pHand->Enter();
		if (m_pExhibition)
			m_pExhibition->Display(0.0f, 0.0f);
		pHand->m_bUserLock = false;
		LeaveCriticalSection((LPCRITICAL_SECTION)((char*)pHand + 0x30));
	}
	if (m_n_B7C != 0 && (pSkin = ItemManager()->FindSkin(m_n_B7C)) != NULL &&
		(pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
		pGI->DrawTexture(pBitmap, *(WRect*)((char*)m_pFrWnd_314 + 0x20),
			0xffffffff, 0);
	if (m_n_B80 != 0 && (pSkin = ItemManager()->FindSkin(m_n_B80)) != NULL &&
		(pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
		pGI->DrawTexture(pBitmap, *(WRect*)((char*)m_pFrWnd_314 + 0x20),
			0xffffffff, 0);
}

void CRealMyRoomMain::DrawSkinSlot()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;
	WRect rect(116.0f, 308.0f, (float)m_pBitmap_55C->Width(),
		(float)m_pBitmap_55C->Height());
	pGI->DrawTexture(m_pBitmap_55C, rect, 0xffffffff, 0);
	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pBitmap;
	if (m_n_B84 != 0 && (pSkin = ItemManager()->FindSkin(m_n_B84)) != NULL &&
		(pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
	{
		WRect rect(116.0f, 308.0f, (float)pBitmap->Width(),
			(float)pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(m_tidChar);
	if (pChar != NULL && (pBitmap = g_pFresh->GetBitmap(pChar->Data)) != NULL)
	{
		WRect rect(124.0f, 344.0f, (float)pBitmap->Width(),
			(float)pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	if (m_n_B88 != 0 && (pSkin = ItemManager()->FindSkin(m_n_B88)) != NULL &&
		(pBitmap = g_pFresh->GetBitmap(pSkin->Tex)) != NULL)
	{
		WRect rect(124.0f, 344.0f, (float)pBitmap->Width(),
			(float)pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	pBitmap = (&m_pBitmap_550)[(*(unsigned char*)DOCPTR983(0x832) >> 1) & 1];
	{
		WRect rect(121.0f, 406.0f, (float)pBitmap->Width(),
			(float)pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	const char* pszIcon;
	if (m_n_B8C == 0 || (pSkin = ItemManager()->FindSkin(m_n_B8C)) == NULL)
		pszIcon = MakeStr("level_%03d", *(unsigned char*)DOCPTR983(0x91d) + 1);
	else
		pszIcon = pSkin->Tex;
	pBitmap = g_pFresh->GetBitmap(pszIcon);
	if (pBitmap != NULL)
	{
		WRect rect(206.0f - (int)(pBitmap->Width() * 0.5f),
			386.0f - (int)(pBitmap->Height() * 0.5f), pBitmap->Width(),
			pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	{
		WRect rect((int)(179.5f - m_pBitmap_558->Width() * 0.5f), 313.0f,
			m_pBitmap_558->Width(), m_pBitmap_558->Height());
		pGI->DrawTexture(m_pBitmap_558, rect, 0xffffffff, 0);
	}
	pGI->SetTextColor(0xffffffff, 0xff808080);
	pGI->SetTextStyle(2);
	g_pFresh->GetManager()->PrintText(WPoint(139.0f, 415.0f), 0,
		DOCPTR983(0x7b8), -1.0f, 0xffffffff);
}

void CRealMyRoomMain::OnPetOwnerDraw(int)
{
	if (m_roomMode == 4)
	{
		switch (m_n_77C)
		{
		case 0xd:
		case 0xe:
		case 0xf:
			DrawSkinFrame();
			return;
		case 0x10:
		case 0x22:
			DrawSkinSlot();
			return;
		case 0x21:
			DrawCutinFrame();
			return;
		}
	}
	else
	{
		if (m_n_77C == 0x1b && m_pFrListBox_DCC->GetSelected() &&
			m_pFrListBox_DCC->GetSelected()->pData && m_pExhibition)
		{
			sCards* pInfo = (sCards*)m_pFrListBox_DCC->GetSelected()->pData;
			if ((pInfo->tid & 0x3c00000) == 0x1000000 && pInfo->cardCount < 10)
			{
				WBone* pRoot = m_pExhibition->GetModel()->GetRootBone();
				WBone* pBone = pRoot->FindBone("card_pack_Dummy", 0);
				if (pBone)
					pBone->SetInvisible(false, true);
				int i = 1;
				if (1 < 10 - pInfo->cardCount)
				{
					do
					{
						pBone = pRoot->FindBone(
							MakeStr("card_pack%02d_Dummy", i), 0);
						if (pBone)
							pBone->SetInvisible(false, true);
						i++;
					} while (i < 10 - pInfo->cardCount);
				}
			}
		}
		DisplayExhibition983();
	}
}

// 983 0060d9b0: fim do OnPetOwnerDraw como funcao propria; tambem chamada
// pelo FrUccDrawDlg::OnDraw 007e2150 (personagem por cima do ucc_draw).
void CRealMyRoomMain::DisplayExhibition983()
{
	CAnotherHand* pHand = &m_hand;
	// 983: a flag da loja do bongdari saiu do CTaskMain +0x122 para o CSharedDoc +0x11410 (ver tutorialmain.cpp)
	if (!pHand->IsWorking(0) && !CSharedDoc::Instance()->m_bBongdariShopOpen &&
		CRankingInfo::Instance()->GetDlg() == NULL &&
		(CUserInfo::Instance() == NULL ||
			CUserInfo::Instance()->GetDlg() == NULL))
	{
		pHand->Enter();
		if (m_pExhibition)
			m_pExhibition->Display(0.0f, 0.0f);
		pHand->Leave();
	}
}

void CRealMyRoomMain::OnFurnitureArrangeInit(int param)
{
	m_pFrButton_380 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_380)
		m_pFrButton_380->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureArrangeButtonUP()
{
	if (m_b_D57 == false)
	{
		ControlUiHideMode(4);
		return;
	}
	ControlUiHideMode(2);
}

void CRealMyRoomMain::OnItemDescBgInit2(int param)
{
	m_pFrViewer_364 = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pFrViewer_364)
		m_pFrViewer_364->SetVisible(false);
}

extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)
void CRealMyRoomMain::OnItemDescBgOwnerDraw2(int)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI)
	{
		float w0 = (float)m_pBitmap_588->Width();
		float h0 = (float)m_pBitmap_588->Height();
		float h1 = (float)m_pBitmap_58C->Height();
		float w2 = (float)m_pBitmap_590->Width();
		_ReadWriteBarrier();
		float h2 = (float)m_pBitmap_590->Height();
		float w3 = (float)m_pBitmap_594->Width();
		float w5 = (float)m_pBitmap_59C->Width();
		float w6 = (float)m_pBitmap_5A0->Width();
		_ReadWriteBarrier();
		float h6 = (float)m_pBitmap_5A0->Height();
		float h7 = (float)m_pBitmap_5A4->Height();
		float w8 = (float)m_pBitmap_5A8->Width();
		_ReadWriteBarrier();
		float h8 = (float)m_pBitmap_5A8->Height();
		float editH = m_pFrEdit_36C->GetRect().h;
		float lineH = (float)m_pFrEdit_368->GetLineHeight();
		lineH = (float)(m_pFrEdit_368->GetLineNum() + 1) * (float)(int)lineH;
		float editW = m_pFrEdit_368->GetRect().w;
		float totalH = lineH + editH;
		float baseX = m_f_6A0 - m_pFrViewer_364->GetRect().w;
		float baseY = 500.0f - totalH;
		WRect rc;
		rc.x = baseX + 10.0f;
		rc.y = baseY + 15.0f;
		rc.w = editW;
		rc.h = editH;
		m_pFrEdit_36C->SetRect(rc);
		rc.y = baseY + 45.0f;
		rc.w = editW;
		rc.h = lineH;
		m_pFrEdit_368->SetRect(rc);
		float innerW = editW + 10.0f;
		WRect r;
		r.x = baseX;
		r.y = baseY;
		r.w = w0;
		r.h = h0;
		pGI->DrawTexture(m_pBitmap_588, r, 0xffffffff, 0);
		r.x = w0 + baseX;
		r.y = baseY;
		r.w = innerW - (w2 + w0);
		r.h = h1;
		pGI->DrawTexture(m_pBitmap_58C, r, 0xffffffff, 0);
		r.y = baseY;
		r.x = (innerW - w2) + baseX;
		r.w = w2;
		r.h = h2;
		pGI->DrawTexture(m_pBitmap_590, r, 0xffffffff, 0);
		r.y = h1 + baseY;
		r.x = baseX;
		r.h = totalH - (h6 + h0);
		r.w = w3;
		pGI->DrawTexture(m_pBitmap_594, r, 0xffffffff, 0);
		r.x = w3 + baseX;
		r.w = innerW - (w5 + w3);
		r.h = totalH - (h7 + h1);
		pGI->DrawTexture(m_pBitmap_598, r, 0xffffffff, 0);
		r.x = (innerW - w5) + baseX;
		r.y = h2 + baseY;
		r.h = totalH - (h8 + h2);
		r.w = w5;
		pGI->DrawTexture(m_pBitmap_59C, r, 0xffffffff, 0);
		r.x = baseX;
		r.y = (totalH - h6) + baseY;
		r.w = w6;
		r.h = h6;
		pGI->DrawTexture(m_pBitmap_5A0, r, 0xffffffff, 0);
		r.x = w6 + baseX;
		r.y = (totalH - h7) + baseY;
		r.w = innerW - (w8 + w6);
		r.h = h7;
		pGI->DrawTexture(m_pBitmap_5A4, r, 0xffffffff, 0);
		r.x = (innerW - w8) + baseX;
		r.y = (totalH - h8) + baseY;
		r.w = w8;
		r.h = h8;
		pGI->DrawTexture(m_pBitmap_5A8, r, 0xffffffff, 0);
	}
}

void CRealMyRoomMain::OnItemDescBgInit(int param)
{
	m_pFrArea_35C = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnItemDescBgOwnerDraw(int)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI)
	{
		if (m_pFrEdit_360->GetLineNum() > 0)
		{
			WRect rect;
			rect.w = (float)m_pBitmap_570->Width();
			rect.h = (float)m_pBitmap_570->Height();
			float w2 = (float)m_pBitmap_574->Width();
			float h2 = (float)m_pBitmap_574->Height();
			float y = m_WPoint_788.y + 320.0f;
			rect.x = m_WPoint_788.x + 6.0f;
			rect.y = y;
			pGI->DrawTexture(m_pBitmap_570, rect, 0xffffffff, 0);
			rect.x = m_WPoint_788.x + 262.0f;
			rect.y = y;
			rect.w = w2;
			rect.h = h2;
			pGI->DrawTexture(m_pBitmap_574, rect, 0xffffffff, 0);
		}
	}
}

void CRealMyRoomMain::OnItemDescInit2(int param)
{
	m_pFrEdit_368 = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pFrEdit_368)
	{
		m_pFrEdit_368->SetVisible(false);
		const WRect& rect = m_pFrEdit_368->GetRect();
		m_f_798 = rect.x;
		m_f_79C = rect.h + rect.y;
		m_f_6A0 = m_f_798;
		m_f_6A4 = m_f_79C;
	}
}

void CRealMyRoomMain::OnItemNameInit(int param)
{
	m_pFrEdit_36C = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pFrEdit_36C)
		m_pFrEdit_36C->SetVisible(false);
}

void CRealMyRoomMain::OnItemDescInit(int param)
{
	m_pFrEdit_360 = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pFrEdit_360)
	{
		m_pFrEdit_360->SetVisible(false);
		const WRect& rect = m_pFrEdit_360->GetRect();
		m_f_790 = rect.x;
		m_f_794 = rect.h + rect.y;
	}
}

void CRealMyRoomMain::SetItemDesc()
{
	m_pFrEdit_360->ClearLine();
	switch (m_n_77C)
	{
	case 6:
	case 10:
		m_WPoint_788 = WPoint(0.0f, 30.0f);
		return;
	case 7:
	case 9:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 23:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
		m_WPoint_788 = WPoint(0.0f, 120.0f);
		return;
	default:
		m_WPoint_788 = WPoint(0.0f, 0.0f);
		return;
	}
}

void CRealMyRoomMain::OnItemSlotBoxInit(int param)
{
	m_pFrGroupBox_384 = DYNAMIC_CAST(FrGroupBox, (FrWnd*)param);
	if (m_pFrGroupBox_384)
	{
		m_pFrGroupBox_384->SetVisible(false);
		if (IsLocalContent(S4_ITEM_STORAGE) == true && CItemStorage::Instance())
			CItemStorage::Instance()->AddBackGroundUI(m_pFrGroupBox_384, false);
	}
}

void CRealMyRoomMain::OnEquipCancelInit(int param)
{
	m_pFrButton_388 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pFrButton_388->SetPushDelay(0.0f);
	m_pFrButton_388->EnableHover(true, 0.0f);
	if (IsLocalContent(S4_ITEM_STORAGE) == true && CItemStorage::Instance())
		CItemStorage::Instance()->AddBackGroundUI(m_pFrButton_388, false);
}

void CRealMyRoomMain::OnEquipCancelUp()
{
	if (m_pFrListBox_37C)
	{
		FrListItem* pSel = m_pFrListBox_37C->GetSelected();
		if (pSel)
		{
			FrListBox::ITEM_LIST::iterator it;
			for (it = m_pFrListBox_37C->m_itemList.begin();
				it != m_pFrListBox_37C->m_itemList.end() && *it != pSel; ++it)
			{
			}
			for (++it; it != m_pFrListBox_37C->m_itemList.end(); ++it)
			{
				if ((*it)->pData && *(int*)(*it)->pData)
				{
					m_pFrListBox_37C->SelectItem(*it, true);
					break;
				}
			}
			if (pSel->pData)
				UnequipItem((unsigned long*)pSel->pData);
		}
	}
	__rtti_obj = NULL;
	CIconToolTip* pTip = CIconToolTip::GetInstance();
	pTip = CIconToolTip::GetInstance();
	pTip->ClearToolTip();
}

void CRealMyRoomMain::OnEquipResetInit(int param)
{
	m_pFrButton_38C = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pFrButton_38C->SetPushDelay(0.0f);
	m_pFrButton_38C->EnableHover(true, 0.0f);
	if (IsLocalContent(S4_ITEM_STORAGE) == true && CItemStorage::Instance())
		CItemStorage::Instance()->AddBackGroundUI(m_pFrButton_38C, false);
}

void CRealMyRoomMain::OnEquipResetUp()
{
	if (m_pFrListBox_37C)
	{
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pFrListBox_37C->m_itemList.begin();
			it != m_pFrListBox_37C->m_itemList.end(); ++it)
		{
			if ((*it)->pData)
				UnequipItem((unsigned long*)(*it)->pData);
		}
	}
	__rtti_obj = NULL;
	CIconToolTip* pTip = CIconToolTip::GetInstance();
	pTip = CIconToolTip::GetInstance();
	pTip->ClearToolTip();
}

void CRealMyRoomMain::OnItemSlotInit(int param)
{
	m_pFrListBox_37C = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CRealMyRoomMain::OnItemSlotOwnerDraw(int param)
{
	FrListItem* pListItem = (FrListItem*)param;
	if (pListItem)
	{
		FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
		unsigned long* pData = (unsigned long*)pListItem->pData;
		if (pData)
		{
			unsigned long tid = *pData;
			int state = 0;
			if (pListItem->selected)
				state = 2;
			else if (pListItem->underCursor)
				state = 1;
			if (tid == 0)
				state = 0;
			const Bitmap* pSlot = m_pSlotImg[state];
			if (pSlot)
			{
				WRect rect;
				rect.x = pListItem->pos.x;
				rect.y = pListItem->pos.y;
				rect.w = (float)pSlot->Width();
				rect.h = (float)pSlot->Height();
				pGI->DrawTexture(pSlot, rect, 0xffffffff, 0);
			}
			IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(tid);
			if (pItem)
			{
				const Bitmap* pIcon = g_pFresh->GetBitmap(pItem->c.Icon);
				if (pIcon)
				{
					WRect rect;
					rect.x = pListItem->pos.x + 2.0f;
					rect.y = pListItem->pos.y + 4.0f;
					rect.w = (float)pIcon->Width();
					rect.h = (float)pIcon->Height();
					pGI->DrawTexture(pIcon, rect, 0xffffffff, 0);
				}
			}
		}
	}
}

void CRealMyRoomMain::OnItemSlotBtnDown()
{
	FrListItem* pItem = m_pFrListBox_37C->GetItemUnderCursor();
}

void CRealMyRoomMain::OnItemSlotDClick()
{
	if (IsLocalContent(S4_ITEM_STORAGE) && IsItemStorageVisible())
		return;
	FrListItem* pItem = m_pFrListBox_37C->GetItemUnderCursor();
	if (pItem == NULL)
		return;
	unsigned long* pData = (unsigned long*)pItem->pData;
	if (pData)
		UnequipItem(pData);
}

void CRealMyRoomMain::OnGiftBgInit(int param)
{
	m_pFrViewer_39C = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pFrViewer_39C)
	{
		m_pFrViewer_39C->SetVisible(false);
		m_pFrViewer_39C->SetMouseEvent(false);
	}
}

void CRealMyRoomMain::OnGiftDescInit(int param)
{
	m_pFrEdit_3A0 = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pFrEdit_3A0)
		m_pFrEdit_3A0->SetVisible(false);
}

void CRealMyRoomMain::SetGiftDescString(const char* from, const char* msg,
	const char* date, const char* time)
{
	if (0) // HACK: emission order (the original waits one wave for a late Fresh inline)
		m_pFrEdit_3A0->GetLineHeight();
	m_pFrEdit_3A0->ClearLine();
	if (from == NULL)
	{
		m_pFrEdit_3A0->AddText(
			MakeStr(
				"\xb5\xb5\xc2\xf8\xc7\xd1 \xbc\xb1\xb9\xb0\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
				from),
			false, true);
	}
	else
	{
		m_pFrEdit_3A0->AddText(
			MakeStr(
				"\\c0xffffffff,t\\c\xb9\xde\xb4\xc2\xc0\xcc\\c0xffffffff,n\\c : %s\xb4\xd4\xb2\xb2\xbc\xad \xbc\xb1\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbb\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.",
				from),
			false, true);
		m_pFrEdit_3A0->AddText(
			MakeStr("\\c0xffffffff,t\\c\xb3\xbb\xbf\xeb\\c0xffffffff,n\\c : %s",
				msg),
			false, true);
		m_pFrEdit_3A0->AddText(
			MakeStr(
				"\\c0xffffffff,t\\c\xba\xb8\xb3\xbd\xb3\xaf\xc2\xa5\\c0xffffffff,n\\c : %s",
				date),
			false, true);
		m_pFrEdit_3A0->AddText(
			MakeStr(
				"\\c0xffffffff,t\\c\xba\xb8\xb3\xbd\xbd\xc3\xb0\xa3\\c0xffffffff,n\\c : %s",
				time),
			false, true);
	}
}

void CRealMyRoomMain::OnStatFrameInit(int param)
{
	m_pFrArea_3A4 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnStatTextInit(int param)
{
	m_pFrArea_3A8 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnPowerBarInit(int param)
{
	FrGaugeBar* pBar = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pFrGaugeBar_32C = pBar;
	pBar->SetRange(0, 50, 0);
}

void CRealMyRoomMain::OnControlBarInit(int param)
{
	FrGaugeBar* pBar = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pFrGaugeBar_330 = pBar;
	pBar->SetRange(0, 30, 0);
}

void CRealMyRoomMain::OnImpactBarInit(int param)
{
	FrGaugeBar* pBar = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pFrGaugeBar_334 = pBar;
	pBar->SetRange(0, 30, 0);
}

void CRealMyRoomMain::OnSpinBarInit(int param)
{
	FrGaugeBar* pBar = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pFrGaugeBar_338 = pBar;
	pBar->SetRange(0, 30, 0);
}

void CRealMyRoomMain::OnCurveBarInit(int param)
{
	FrGaugeBar* pBar = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pFrGaugeBar_33C = pBar;
	pBar->SetRange(0, 30, 0);
}

void CRealMyRoomMain::TurnOnItemSelect()
{
	m_pFrButton_324->SetVisible(true);
	m_pFrButton_354->SetVisible(true);
	m_pFrButton_358->SetVisible(true);
}

void CRealMyRoomMain::TurnOffItemSelect()
{
	m_pFrButton_324->SetVisible(false);
	m_pFrButton_354->SetVisible(false);
	m_pFrButton_358->SetVisible(false);
}

void CRealMyRoomMain::TurnOnStat()
{
	m_pFrGroupBox_328->SetVisible(true);
}

void CRealMyRoomMain::TurnOffStat()
{
	m_pFrGroupBox_328->SetVisible(false);
}

void CRealMyRoomMain::OnSelFrameInit(int param)
{
	m_pFrArea_31C = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnStatBoxInit(int param)
{
	m_pFrGroupBox_328 = DYNAMIC_CAST(FrGroupBox, (FrWnd*)param);
}

void CRealMyRoomMain::OnSelItemIconInit(int param)
{
	m_pFrArea_320 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnSelItemIconOwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;

	switch (m_n_77C)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 0x15:
	case 0x16:
	case 0x1a:
		DrawSelItem(pGI, m_tidChar);
		DrawCharacterList(pGI, m_tidChar);
		break;
	case 6:
		DrawSelItem(pGI, m_tidClub);
		DrawClubList(pGI, m_tidClub);
		break;
	case 7:
		DrawSelItem(pGI, m_tidAztec);
		DrawAztecList(pGI, m_tidAztec);
		break;
	case 10:
		DrawSelItem(pGI, m_tidCaddie);
		DrawCaddieList(pGI, m_tidCaddie);
		break;
	case 0xc:
		DrawSelItem(pGI, m_n_75C);
		DrawMascotList(pGI, m_n_75C);
		break;
	}
}

void CRealMyRoomMain::OnSelItemInit(int param)
{
	m_pFrButton_324 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_324)
		m_pFrButton_324->EnableBlink(true);
}

void CRealMyRoomMain::OnSelItemOwnerDraw(int param)
{
}

void CRealMyRoomMain::OnSelItemUp()
{
	m_b_6D8 = (m_b_6D8 == 0);
}

void CRealMyRoomMain::OnSelItemPrevInit(int param)
{
	m_pFrButton_354 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_354)
		m_pFrButton_354->SetPushDelay(0.1f);
}

void CRealMyRoomMain::OnSelItemPrevUp()
{
	// HACK: emission order (templates are compiled in instantiation order; a template waits one pass for a
	// callee that is instantiated after it)
	__assume(&*--(*(std::map<unsigned int, sCharacterInfo>::iterator*)0) != 0);
	__assume(&*--(*(std::list<sItemInfo>::iterator*)0) != 0);
	// m_hand ocupado com a chave 0 (era this+0x7f0/0x7f8/0x82c = layout do 645)
	if (m_hand.IsWorking(0))
		return;
	switch (m_n_77C)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 0x15:
	case 0x16:
	case 0x1a:
	{
		for (std::map<unsigned int, sCharacterInfo>::reverse_iterator it =
				 Doc()->m_charMap.rbegin();
			it != Doc()->m_charMap.rend(); ++it)
		{
			if ((*it).second.guid == m_n_74C && ++it != Doc()->m_charMap.rend())
			{
				ChangeCharacter((*it).second.guid);
				return;
			}
		}
		break;
	}
	case 10:
	{
		std::map<unsigned int, sCaddieInfo>::reverse_iterator it =
			Doc()->m_caddieMap.rbegin();
		if (it != Doc()->m_caddieMap.rend())
		{
			for (;;)
			{
				if ((*it).second.guid == m_n_754)
				{
					++it;
					if (it != Doc()->m_caddieMap.rend())
						break;
				}
				++it;
				if (!(it != Doc()->m_caddieMap.rend()))
					return;
			}
			ChangeCaddie((*it).second.guid);
		}
		break;
	}
	case 0xc:
	{
		std::map<unsigned int, sMascotInfo>::reverse_iterator it =
			Doc()->m_mascotMap.rbegin();
		if (it != Doc()->m_mascotMap.rend())
		{
			for (;;)
			{
				if ((*it).second.guid == m_n_760)
				{
					++it;
					if (it != Doc()->m_mascotMap.rend())
						break;
				}
				++it;
				if (!(it != Doc()->m_mascotMap.rend()))
					return;
			}
			ChangeMascot((*it).second.guid);
		}
		break;
	}
	case 6:
	{
		std::map<unsigned int, sItemInfo>::reverse_iterator it =
			Doc()->m_clubSetMap.rbegin();
		if (it != Doc()->m_clubSetMap.rend())
		{
			for (;;)
			{
				if ((*it).second.guid == m_n_764)
				{
					++it;
					if (it != Doc()->m_clubSetMap.rend())
						break;
				}
				++it;
				if (!(it != Doc()->m_clubSetMap.rend()))
					return;
			}
			ChangeClubset((*it).second.guid);
		}
		break;
	}
	case 7:
	{
		std::list<sItemInfo>::reverse_iterator it = Doc()->m_ballList.rbegin();
		if (it != Doc()->m_ballList.rend())
		{
			for (;;)
			{
				if ((*it).tid == m_tidAztec)
				{
					++it;
					if (it != Doc()->m_ballList.rend())
						break;
				}
				++it;
				if (!(it != Doc()->m_ballList.rend()))
					return;
			}
			ChangeAztec((*it).tid);
		}
		break;
	}
	}
}

void CRealMyRoomMain::OnSelItemNextInit(int param)
{
	m_pFrButton_358 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_358)
		m_pFrButton_358->SetPushDelay(0.1f);
}

void CRealMyRoomMain::OnSelItemNextUp()
{
	// m_hand ocupado com a chave 0 (era this+0x7f0/0x7f8/0x82c = layout do 645)
	if (m_hand.IsWorking(0))
		return;
	switch (m_n_77C)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 0x15:
	case 0x16:
	case 0x1a:
	{
		std::map<unsigned int, sCharacterInfo>::const_iterator it =
			Doc()->m_charMap.begin();
		if (it != Doc()->m_charMap.end())
		{
			for (;;)
			{
				if ((*it).second.guid == m_n_74C)
				{
					++it;
					if (it != Doc()->m_charMap.end())
						break;
				}
				++it;
				if (it == Doc()->m_charMap.end())
					return;
			}
			ChangeCharacter((*it).second.guid);
		}
		break;
	}
	case 10:
	{
		std::map<unsigned int, sCaddieInfo>::const_iterator it =
			Doc()->m_caddieMap.begin();
		if (it != Doc()->m_caddieMap.end())
		{
			for (;;)
			{
				if ((*it).second.guid == m_n_754)
				{
					++it;
					if (it != Doc()->m_caddieMap.end())
						break;
				}
				++it;
				if (it == Doc()->m_caddieMap.end())
					return;
			}
			ChangeCaddie((*it).second.guid);
		}
		break;
	}
	case 0xc:
	{
		std::map<unsigned int, sMascotInfo>::const_iterator it =
			Doc()->m_mascotMap.begin();
		if (it != Doc()->m_mascotMap.end())
		{
			for (;;)
			{
				if ((*it).second.guid == m_n_760)
				{
					++it;
					if (it != Doc()->m_mascotMap.end())
						break;
				}
				++it;
				if (it == Doc()->m_mascotMap.end())
					return;
			}
			ChangeMascot((*it).second.guid);
		}
		break;
	}
	case 6:
	{
		std::map<unsigned int, sItemInfo>::const_iterator it =
			Doc()->m_clubSetMap.begin();
		if (it != Doc()->m_clubSetMap.end())
		{
			for (;;)
			{
				if ((*it).second.guid == m_n_764)
				{
					++it;
					if (it != Doc()->m_clubSetMap.end())
						break;
				}
				++it;
				if (it == Doc()->m_clubSetMap.end())
					return;
			}
			ChangeClubset((*it).second.guid);
		}
		break;
	}
	case 7:
	{
		sRbListNode* pHead = *(sRbListNode**)&Doc()->m_ballList;
		sRbListNode* p = pHead->next;
		if (p != pHead)
		{
			for (;;)
			{
				if (p->tid == m_tidAztec)
				{
					p = p->next;
					if (p != pHead)
						break;
				}
				p = p->next;
				if (p == pHead)
					return;
			}
			ChangeAztec(p->tid);
		}
		break;
	}
	}
}

void CRealMyRoomMain::OnStatPartInit(int param)
{
	m_pFrViewer_2EC = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void CRealMyRoomMain::OnStatPartUp()
{
	m_n_774 = 0;
	SetLevelBar();
}

void CRealMyRoomMain::OnStatPartOwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;
	int state = 0;
	WRect rect = m_pFrViewer_2EC->GetRect();
	if (m_pFrViewer_2EC->IsVisible())
	{
		if (m_n_774 == 0)
			state = 2;
		else if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			state = 1;
		if (m_n_B74 < 3)
			pGI->DrawTexture(m_pStatTabImg[m_n_B74][state], rect, 0xffffffff,
				0);
	}
}

void CRealMyRoomMain::OnStatAllInit(int param)
{
	m_pFrViewer_2F0 = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void CRealMyRoomMain::OnStatAllUp()
{
	m_n_774 = 1;
	SetLevelBar();
}

void CRealMyRoomMain::OnStatAllOwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;
	int state = 0;
	WRect rect = m_pFrViewer_2F0->GetRect();
	if (m_pFrViewer_2F0->IsVisible())
	{
		if (m_n_774 == 1)
			state = 2;
		else if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			state = 1;
		pGI->DrawTexture(m_pStatTabImg[3][state], rect, 0xffffffff, 0);
	}
}

void CRealMyRoomMain::OnTabMainInit(int param)
{
	m_pFrTabButton_2F4 = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	if (m_pFrTabButton_2F4)
	{
		const char* imgNames[3];
		imgNames[0] = NULL;
		imgNames[1] = NULL;
		imgNames[2] = NULL;
		for (int i = 1; i < 6; i++)
		{
			imgNames[0] = MakeStr("stab_%d_n", i);
			imgNames[1] = MakeStr("stab_%d_d", i);
			imgNames[2] = MakeStr("stab_%d_d", i);
			m_pFrTabButton_2F4->AddTab(imgNames, i - 1);
		}
		imgNames[0] = MakeStr("stab_%d_n", 7);
		imgNames[1] = MakeStr("stab_%d_d", 7);
		imgNames[2] = MakeStr("stab_%d_d", 7);
		m_pFrTabButton_2F4->AddTab(imgNames, 6);
		imgNames[0] = MakeStr("stab_%d_n", 6);
		imgNames[1] = MakeStr("stab_%d_d", 6);
		imgNames[2] = MakeStr("stab_%d_d", 6);
		m_pFrTabButton_2F4->AddTab(imgNames, 5);
	}
}

void CRealMyRoomMain::OnTabMainButtonUp()
{
	if (m_pFrTabButton_2F4->GetSelected() == NULL)
		return;
	mainTab_t tab = (mainTab_t)m_pFrTabButton_2F4->GetSelected()->index;
	if (m_hand.IsWorking(0))
	{
		m_pFrTabButton_2F4->Select(m_roomMode);
		return;
	}
	ChangeMainTab(tab, false);
	switch (tab)
	{
	case 0:
		ChangeSubTab((subTab_t)0x15, false);
		ChangeCharacter(Doc()->m_myInfo.userEquip.guidChar);
		break;
	case 1:
		ChangeSubTab((subTab_t)6, false);
		break;
	case 2:
		ChangeSubTab((subTab_t)10, false);
		ChangeCaddie(Doc()->m_myInfo.userEquip.guidCaddie);
		break;
	case 3:
		ChangeSubTab((subTab_t)0xc, false);
		ChangeMascot(Doc()->m_myInfo.userEquip.guidMascot);
		break;
	case 4:
		ChangeSubTab((subTab_t)0xd, false);
		TurnOffItemStatus();
		break;
	case 5:
	{
		if (IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
			ChangeSubTab((subTab_t)0x12, false);
		else
			ChangeSubTab((subTab_t)0x11, false);
		TurnOffItemStatus();
		m_hand.WaitUntilWorkDone(9999);
		CExhibition* pExhibition = m_pExhibition;
		if (pExhibition != NULL)
		{
			delete pExhibition;
			m_pExhibition = NULL;
		}
		break;
	}
	case 6:
		ChangeSubTab((subTab_t)0x20, false);
		TurnOffItemStatus();
		break;
	}
	if (tab == 6)
		m_pFrButton_380->SetVisible(true);
	else
		m_pFrButton_380->SetVisible(false);
	Net_EquipDresserParts();
}

void CRealMyRoomMain::OnTabSubInit(int param)
{
	m_pFrTabButton_2F8 = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
}

void CRealMyRoomMain::OnTabSubButtonUp()
{
	if (m_pFrTabButton_2F8->GetSelected())
		ChangeSubTab((subTab_t)m_pFrTabButton_2F8->GetSelected()->index, false);
}

void CRealMyRoomMain::OnGiftPageOwnerDraw(int)
{
	if (m_n_77C != 0x11)
		return;
	int k = 0;
	const Bitmap** pBmp = &m_pBitmap_5AC;
	do
	{
		if (*pBmp == 0)
			return;
		k++;
		pBmp++;
	} while (k < 5);
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;
	unsigned int page = m_giftPage;
	unsigned int count = 0;
	unsigned int base = page * 5;
	bool bLocal = IsLocalContent((localContentType_t)0x5c /*983: era 0x5b; interpolado*/);
	if (bLocal)
	{
		unsigned int total = *((unsigned char*)&CSharedDoc::m_cardPackItem);
		m_giftTotal = total;
		count = *((unsigned char*)&CSharedDoc::m_cardPackSlot);
		if (base < total)
		{
			count = total - page * 5;
			if (count > 4)
			{
				count = 5;
				goto draw;
			}
		}
		else if (base == total)
		{
			count = 1;
			goto check;
		}
		else if (total < base)
			count = total % 5;
		if (count != 1)
			goto draw;
	}
	else
	{
		unsigned int total = *(unsigned int*)((char*)&CSharedDoc::m_cardPackPageNum);
		unsigned int val = *(unsigned int*)((char*)&CSharedDoc::m_cardPackCurPage);
		if (total != 0 && val != 0)
		{
			unsigned int pages = val / total;
			m_giftTotal = pages;
			if (pages * total < val)
				m_giftTotal = pages + 1;
		}
		total = m_giftTotal;
		if (base < total)
		{
			count = total - page * 5;
			if (count > 4)
			{
				count = 5;
				goto draw;
			}
			if (count != 1)
				goto draw;
		}
		else if (base != total)
		{
			if (base <= total)
				goto draw;
			count = total % 5;
			if (count != 1)
				goto draw;
		}
		else
			count = 1;
	}
check:
	if (m_giftPage == 0)
		return;
draw:
	m_giftCount = count;
	WRect r;
	WRect nav;
	const Bitmap* pBmp2;
	float cur;
	unsigned int cell = m_pBitmap_5B4->Width();
	float y0 = 577.0f - (float)(((count + 2) >> 1) * cell);
	if (count & 1)
		y0 = y0 - (float)(cell >> 1);
	nav.x = y0 - (float)m_pBitmap_5B0->Width();
	nav.y = 517.0f;
	nav.w = (float)m_pBitmap_5B0->Width();
	nav.h = (float)m_pBitmap_5B0->Height();
	cur = nav.x;
	pGI->DrawTexture(m_pBitmap_5B0, nav, 0xffffffff, 0);
	r.x = (float)m_pBitmap_5B0->Width() + cur;
	r.y = 517.0f;
	r.w = (float)m_pBitmap_5B4->Width();
	r.h = (float)m_pBitmap_5B4->Height();
	cur = r.x;
	pGI->DrawTexture(m_pBitmap_5B4, r, 0xffffffff, 0);
	r.x = ((float)(m_pBitmap_5B4->Width() >> 1) + r.x) -
		(float)(m_pBitmap_5AC->Width() >> 1);
	r.y = (((float)(m_pBitmap_5B4->Height() >> 1) + r.y) -
			  (float)(m_pBitmap_5AC->Height() >> 1)) -
		0.5f;
	r.w = (float)m_pBitmap_5AC->Width();
	r.h = (float)m_pBitmap_5AC->Height();
	m_giftPrevRect = r;
	pGI->DrawTexture(m_pBitmap_5AC, r,
		m_giftPage == 0 ? 0xff999999 : 0xffffffff, 0);
	pBmp2 = m_pBitmap_5B4;
	cur = (float)pBmp2->Width() + cur;
	unsigned int i = 0;
	r.x = cur;
	if (count != 0)
	{
		WRect* pSlot = m_giftSlotRect;
		do
		{
			r.y = 517.0f;
			r.w = (float)pBmp2->Width();
			r.h = (float)pBmp2->Height();
			r.x = cur;
			pGI->DrawTexture(pBmp2, r, 0xffffffff, 0);
			unsigned int half = m_pBitmap_5B4->Width() >> 1;
			r.x = (float)half + r.x;
			r.y = r.y + 3.0f;
			*pSlot = r;
			pSlot->x = pSlot->x - 5.0f;
			pSlot->y = pSlot->y - 5.0f;
			pGI->SetTextColor(0xffffffff, 0xffffffff);
			if (count < m_giftSel)
			{
				m_giftSel = count;
				if (IsLocalContent((localContentType_t)0x5c /*983: era 0x5b; interpolado*/))
					*((unsigned char*)&CSharedDoc::m_cardPackSlot) =
						(unsigned char)m_giftSel;
				else
					*(unsigned int*)((char*)&CSharedDoc::m_cardPackMode) =
						(unsigned char)m_giftSel;
			}
			unsigned int cursel = i + 1;
			if (cursel == m_giftSel)
				pGI->SetTextStyle(1);
			else
				pGI->SetTextColor(0xff99ccff, 0xffffffff);
			WPoint pt(r.x, r.y);
			pGI->Print(pt, 1, "%d", i + m_giftPage * 5 + 1);
			pGI->SetTextStyle(0);
			pBmp2 = m_pBitmap_5B4;
			cur = (float)pBmp2->Width() + cur;
			i = cursel;
			pSlot++;
			r.x = cur;
		} while (i < count);
	}
	r.y = 517.0f;
	r.w = (float)pBmp2->Width();
	r.h = (float)pBmp2->Height();
	nav.x = r.x;
	cur = r.x;
	pGI->DrawTexture(pBmp2, nav, 0xffffffff, 0);
	r.x = ((float)(m_pBitmap_5B4->Width() >> 1) + r.x) -
		(float)(m_pBitmap_5BC->Width() >> 1);
	r.y = (((float)(m_pBitmap_5B4->Height() >> 1) + r.y) -
			  (float)(m_pBitmap_5BC->Height() >> 1)) -
		0.5f;
	r.w = (float)m_pBitmap_5BC->Width();
	r.h = (float)m_pBitmap_5BC->Height();
	m_giftSlotRect[count] = r;
	pGI->DrawTexture(m_pBitmap_5BC, r,
		(m_giftPage * 5 < m_giftTotal && m_giftTotal - m_giftPage * 5 > 5)
			? 0xffffffff
			: 0xff999999,
		0);
	nav.x = (float)m_pBitmap_5B4->Width() + cur;
	nav.y = 517.0f;
	nav.w = (float)m_pBitmap_5B8->Width();
	nav.h = (float)m_pBitmap_5B8->Height();
	pGI->DrawTexture(m_pBitmap_5B8, nav, 0xffffffff, 0);
	IsLocalContent((localContentType_t)0x5c /*983: era 0x5b; interpolado*/);
	if (*((unsigned char*)&CSharedDoc::m_bCardPackRebuild) == 1)
	{
		*((unsigned char*)&CSharedDoc::m_bCardPackRebuild) = 0;
		BuildDisplayStand((subTab_t)0x11);
	}
	WVector mouse = g_input->GetMousePoint();
	bool bHit = false;
	int idx = 0;
	if (m_giftPrevRect.x <= mouse.x &&
		mouse.x <= m_giftPrevRect.w + m_giftPrevRect.x &&
		m_giftPrevRect.y <= mouse.y &&
		mouse.y <= m_giftPrevRect.h + m_giftPrevRect.y)
	{
		idx = 1;
		bHit = true;
	}
	if (m_giftCount != 0)
	{
		int n = m_giftCount;
		WRect* p = m_giftSlotRect;
		idx = idx + n;
		do
		{
			if (p->x <= mouse.x && mouse.x <= p->w + p->x && p->y <= mouse.y &&
				mouse.y <= p->y + p->h)
				bHit = true;
			p++;
			n--;
		} while (n != 0);
	}
	WRect* pLast = &m_giftSlotRect[idx];
	if ((pLast->x <= mouse.x && mouse.x <= pLast->w + pLast->x &&
			pLast->y <= mouse.y && mouse.y <= pLast->h + pLast->y) ||
		bHit)
		CMouseCursor::Instance()->SetCursor(1);
}

void CRealMyRoomMain::OnGiftPageLBDown()
{
	if (m_n_77C != 0x11)
		return;
	if (!IsLocalContent((localContentType_t)0x5c /*983: era 0x5b; interpolado*/) &&
		!(*(unsigned int*)((char*)&CSharedDoc::m_cardPackCurPage) >
			*(unsigned int*)((char*)&CSharedDoc::m_cardPackPageNum)))
		return;
	static DWORD s_lastTick = GetTickCount();
	if (GetTickCount() - s_lastTick > 350)
	{
		s_lastTick = GetTickCount();
		FrWndManager* pManager = g_pFresh->GetManager();
		const WPoint& mouse = pManager->GetMousePos();
		unsigned long pageIndex = 0;
		unsigned short slot = 0;
		if (m_giftPrevRect.IsInRect(mouse))
		{
			if (m_giftPage > 0)
			{
				m_giftPage--;
				m_giftSel = 1;
				pageIndex = m_giftPage * 5 + 1;
			}
			slot = 1;
		}
		if (m_giftCount > 0)
		{
			unsigned int i = 1;
			do
			{
				const WRect& rc = m_giftSlotRect[i - 1];
				if (mouse.x >= rc.x && mouse.x <= rc.w + rc.x &&
					mouse.y >= rc.y && mouse.y <= rc.h + rc.y && m_giftSel != i)
				{
					pageIndex = i + m_giftPage * 5;
					m_giftSel = i;
				}
				slot++;
				i++;
			} while (i - 1 < m_giftCount);
		}
		const WRect& rcNext = m_giftSlotRect[slot];
		if (mouse.x >= rcNext.x && mouse.x <= rcNext.w + rcNext.x &&
			mouse.y >= rcNext.y && mouse.y <= rcNext.h + rcNext.y)
		{
			unsigned int page = m_giftPage;
			if (page * 5 < m_giftTotal && m_giftTotal - page * 5 > 5)
			{
				m_giftPage = page + 1;
				m_giftSel = 1;
				pageIndex = m_giftPage * 5 + 1;
			}
		}
		if (pageIndex != 0)
		{
			WSendPacket packet((enumClientPacket)0x95); // JP 983 (s63): 0x92 no KR 645
			packet.Encode4(pageIndex);
			packet.Send(TO_GAME);
		}
	}
	if (!IsLocalContent((localContentType_t)0x5c /*983: era 0x5b; interpolado*/))
		*(unsigned int*)((char*)&CSharedDoc::m_cardPackMode) = m_giftSel;
}

void CRealMyRoomMain::OnCardBookInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pBackground, pArea);
	}
}

void CRealMyRoomMain::OnCardBookOwnerDraw(int param)
{
	if (m_pCardBook)
		m_pCardBook->Draw();
}

void CRealMyRoomMain::SetCardBookVisible(bool bVisible)
{
	if (m_pCardBook)
	{
		m_pCardBook->Show(bVisible);
		m_b_D88 = bVisible;
	}
	if (m_pFrListBox_DCC)
	{
		m_pFrListBox_DCC->SetVisible(bVisible);
		m_pFrListBox_DCC->UnselectAllItem();
		CExhibition* pExhibition = m_pExhibition;
		if (pExhibition != NULL)
		{
			delete pExhibition;
			m_pExhibition = NULL;
		}
	}
}

void CRealMyRoomMain::BuildCardPackList()
{
	if (m_pFrListBox_DCC)
	{
		m_pFrListBox_DCC->ClearItem();
		std::list<sCards>::iterator it;
		for (it = Doc()->m_cardList.begin(); it != Doc()->m_cardList.end();
			++it)
		{
			sCards* pCard = &(*it);
			unsigned int type = (pCard->tid >> 22) & 0xf;
			if (type == 4 || type == 3)
				m_pFrListBox_DCC->AddItem(pCard);
		}
		if (Doc()->m_cardList.size() & 1)
			m_pFrListBox_DCC->AddItem(NULL);
		int nEmpty = 6 - m_pFrListBox_DCC->GetCurrentItemSize(true);
		for (int i = 0; i < nEmpty; i++)
			m_pFrListBox_DCC->AddItem(NULL);
		m_pFrListBox_DCC->UseRightButton(true);
		m_pFrListBox_DCC->UseDummy(true);
		if (m_pExhibition)
		{
			delete m_pExhibition;
			m_pExhibition = NULL;
		}
	}
}

void CRealMyRoomMain::OnCardPackListInit(int param)
{
	m_pFrListBox_DCC = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	BuildCardPackList();
	if (m_pFrListBox_DCC)
		m_pFrListBox_DCC->SetVisible(false);
}

void CRealMyRoomMain::OnCardPackListDraw(int param)
{
	if (!m_pFrListBox_DCC || m_n_77C != 0x1b || !param)
		return;
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	pGI->SetTextStyle(0);
	const Bitmap* pBitmap;
	if (!pItem->pData)
		pBitmap = m_pBitmap_490;
	else
	{
		if (!pItem->selected)
		{
			pGI->SetTextColor(0xff000000, 0xffffffff);
			if (!pItem->underCursor)
				pBitmap = m_pBitmap_484;
			else
				pBitmap = m_pBitmap_488;
		}
		else
		{
			pBitmap = m_pBitmap_48C;
			pGI->SetTextColor(0xffff0000, 0xffffffff);
		}
	}
	if (pBitmap)
	{
		WRect dst(pItem->pos.x, pItem->pos.y,
			(float)m_pFrListBox_DCC->GetItemWidth(),
			(float)m_pFrListBox_DCC->GetItemHeight());
		WRect src(0.0f, 0.0f, (float)(unsigned)m_pFrListBox_DCC->GetItemWidth(),
			(float)(unsigned)m_pFrListBox_DCC->GetItemHeight());
		pGI->DrawTexture(pBitmap, src, dst, 0xffffffff, 0);
	}
	sCards* pCard = (sCards*)pItem->pData;
	if (pCard)
	{
		const char* name =
			K2L_Compatibility("\xbe\xcb \xbc\xf6 \xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb");
		const char* icon = "caddie_no";
		IFF_STRUCT::sCard* pInfo = ItemManager()->FindCard(pCard->tid);
		if (pInfo)
			icon = pInfo->c.Icon;
		pBitmap = g_pFresh->GetBitmap(icon);
		float w = (float)m_pFrListBox_DCC->GetItemWidth();
		if (pBitmap)
		{
			WRect rect(pItem->pos.x + 12.0f, pItem->pos.y + 12.0f,
				(float)pBitmap->Width(), (float)pBitmap->Height());
			pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
		}
		WPoint pt(w - 12.0f + pItem->pos.x, pItem->pos.y + 12.0f);
		pGI->Print11(pt, 2, "%s", name);
		int h = pGI->GetFontHeight11();
		pt = WPoint(w - 12.0f + pItem->pos.x, (float)h + 17.0f + pItem->pos.y);
		if ((pCard->tid & 0x3c00000) == 0x1000000)
			pGI->Print11(pt, 2,
				MakeStr(K2L_Compatibility("\xb3\xb2\xc0\xba \xc4\xab\xb5\xe5 \xc6\xd1 %d\xb0\xb3"),
					pCard->cardCount));
		else
			pGI->Print11(pt, 2, MakeStr("%d\xb0\xb3", pCard->cardCount));
		if (m_pCardBook && pItem == m_pFrListBox_DCC->GetSelected())
			m_pCardBook->DrawCardPackHelp(pCard->tid);
	}
}

void CRealMyRoomMain::OnCardPackListDown()
{
	if (m_pFrListBox_DCC == NULL || m_n_77C != 0x1b)
		return;
	FrListItem* pItem = m_pFrListBox_DCC->GetItemUnderCursor();
	if (pItem == NULL || pItem->pData == NULL)
		return;
	if (m_hand.IsWorking(0))
	{
		m_pFrListBox_DCC->ToggleItem(pItem);
		return;
	}
	if (pItem->selected)
	{
		g_audio->PlaySfx("ui_desktop_icon_click");
		sCards* pCard = (sCards*)pItem->pData;
		if (m_pCardBook)
			m_pCardBook->m_pUseButton->SetVisible(true);
		ShowPet(pCard->tid);
		return;
	}
	m_pFrListBox_DCC->SelectItem(pItem, true);
}

void CRealMyRoomMain::OnCardPackListRbUp()
{
	if (m_pFrListBox_DCC == NULL || m_n_77C != 0x1b)
		return;
	FrListItem* pItem = m_pFrListBox_DCC->GetItemUnderCursor();
	if (pItem == NULL || pItem->pData == NULL)
		return;
	if (m_hand.IsWorking(0))
	{
		m_pFrListBox_DCC->ToggleItem(pItem);
		return;
	}
	if (pItem->selected)
	{
		OnCardPackListDown();
		OpenInformation((unsigned long*)pItem->pData + 1, NULL, NULL);
		return;
	}
	m_pFrListBox_DCC->ToggleItem(pItem);
	sCards* pCard = (sCards*)pItem->pData;
	OpenInformation(&pCard->tid, NULL, NULL);
}

void CRealMyRoomMain::OnCardBookIndexInit(int param)
{
	FrListBox* pListBox = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (pListBox && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pIndexList, pListBox);
	}
}

void CRealMyRoomMain::OnCardBookIndexLBDown()
{
	if (m_pCardBook)
		m_pCardBook->PageIndexBehavior(g_pFresh->GetManager()->GetMousePos(),
			1);
}

void CRealMyRoomMain::OnCardBookIndexMouseOver()
{
	if (m_pCardBook)
		m_pCardBook->PageIndexBehavior(g_pFresh->GetManager()->GetMousePos(),
			2);
}

void CRealMyRoomMain::OnCardUseInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pUseButton, pButton);
	}
}

template <class T>
T* FindFrForm(const char* id)
{
	FrWnd* pWnd = g_pFresh->GetDesktop()->FindChildForm(id);
	if (pWnd)
		return DYNAMIC_CAST(T, pWnd);
	return NULL;
}

void CRealMyRoomMain::OnCardUseLBDown()
{
	if (m_pFrListBox_DCC->IsVisible())
	{
		FrListItem* pItem = m_pFrListBox_DCC->GetSelected();
		if (pItem != NULL && pItem->pData != NULL)
		{
			unsigned long* pCard = (unsigned long*)pItem->pData;
			FrCardBoxDlg* pDlg = FindFrForm<FrCardBoxDlg>("FrCardBoxDlg");
			if (pDlg == NULL)
			{
				pDlg = CreateForm<FrCardBoxDlg>(g_pFresh->GetManager(), this,
					"FrCardBoxDlg", NULL);
				pDlg->SetCardInfo(pCard[1], pCard[0], pCard[5]);
				pDlg->Open(
					(FRESH_PFN_RESULT)&CRealMyRoomMain::OnCardBoxDlgResult, 1);
				if (m_pExhibition)
				{
					delete m_pExhibition;
					m_pExhibition = NULL;
				}
			}
		}
	}
	else if (m_pCardBook != NULL)
	{
		m_pCardBook->UseCard();
	}
}

bool CRealMyRoomMain::OnCardBoxDlgResult(int result, FrForm* pForm)
{
	return true;
}

void CRealMyRoomMain::OnCardPropertyInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pProperty, pArea);
	}
}

void CRealMyRoomMain::OnTabCardPackInit(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pTabButton[0], pButton);
		pButton->SetPushDelay(0.0f);
	}
}

void CRealMyRoomMain::OnTabCardPackLBDown()
{
	if (m_pCardBook)
	{
		m_pCardBook->ButtonTabControl(0);
		m_pCardBook->ShowSlotControl(true);
	}
	if (m_pFrListBox_DCC)
	{
		m_pFrListBox_DCC->SetVisible(true);
		m_pFrListBox_DCC->UnselectAllItem();
	}
}

void CRealMyRoomMain::OnTabCardVolume1Init(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pTabButton[1], pButton);
		pButton->SetPushDelay(0.0f);
	}
}

void CRealMyRoomMain::OnTabCardVolume1LBDown()
{
	if (m_pCardBook)
	{
		m_pCardBook->ButtonTabControl(1);
		m_pCardBook->ShowSlotControl(true);
	}
	if (m_pFrListBox_DCC)
		m_pFrListBox_DCC->SetVisible(false);
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition != NULL)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
}

void CRealMyRoomMain::OnTabCardVolume2Init(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pTabButton[2], pButton);
		pButton->SetPushDelay(0.0f);
	}
}

void CRealMyRoomMain::OnTabCardVolume2LBDown()
{
	if (m_pCardBook)
	{
		m_pCardBook->ButtonTabControl(2);
		m_pCardBook->ShowSlotControl(true);
	}
	if (m_pFrListBox_DCC)
		m_pFrListBox_DCC->SetVisible(false);
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition != NULL)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
}

void CRealMyRoomMain::OnTabCardVolume3Init(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pTabButton[3], pButton);
		pButton->SetPushDelay(0.0f);
	}
}

void CRealMyRoomMain::OnTabCardVolume3LBDown()
{
	if (m_pCardBook)
	{
		m_pCardBook->ButtonTabControl(3);
		m_pCardBook->ShowSlotControl(true);
	}
	if (m_pFrListBox_DCC)
		m_pFrListBox_DCC->SetVisible(false);
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition != NULL)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
}

void CRealMyRoomMain::OnTabCardVolume4Init(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton && m_pCardBook)
	{
		m_pCardBook->BoundControl(m_pCardBook->m_pTabButton[4], pButton);
		pButton->SetPushDelay(0.0f);
	}
}

void CRealMyRoomMain::OnTabCardVolume4LBDown()
{
	if (m_pCardBook)
	{
		m_pCardBook->ButtonTabControl(4);
		m_pCardBook->ShowSlotControl(true);
	}
	if (m_pFrListBox_DCC)
		m_pFrListBox_DCC->SetVisible(false);
	CExhibition* pExhibition = m_pExhibition;
	if (pExhibition != NULL)
	{
		delete pExhibition;
		m_pExhibition = NULL;
	}
}

void CRealMyRoomMain::BoundCardPackPage(FrButton* pButton, unsigned char page)
{
	pButton->SetVisible(false);
	pButton->Enable(false);
}

void CRealMyRoomMain::OnCardPackInit1(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 0);
}

void CRealMyRoomMain::OnCardPackInit2(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 1);
}

void CRealMyRoomMain::OnCardPackInit3(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 2);
}

void CRealMyRoomMain::OnCardPackInit4(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 3);
}

void CRealMyRoomMain::OnCardPackInit5(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 4);
}

void CRealMyRoomMain::OnCardPackInit6(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 5);
}

void CRealMyRoomMain::OnCardPackInit7(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 6);
}

void CRealMyRoomMain::OnCardPackInit8(int param)
{
	BoundCardPackPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 7);
}

void CRealMyRoomMain::CardPackBehavior(unsigned char slot, unsigned char type)
{
	CCardBook* pCardBook = m_pCardBook;
	if (pCardBook == NULL || m_b_D98)
		return;
	CCardPackPage* pPage = pCardBook->GetCardPackPage();
	if (pPage == NULL)
		return;
	pCardBook->SetSelectedSlot(slot);
	pPage->SelectSlot(type, slot);
	if (m_pCardBook->m_pUseButton)
	{
		if (pPage->IsSelectedSlot())
		{
			m_pCardBook->m_pUseButton->SetVisible(true);
			return;
		}
		m_pCardBook->m_pUseButton->SetVisible(false);
	}
}

void CRealMyRoomMain::OnCardPackLBDown1()
{
	CardPackBehavior(0, 1);
}

void CRealMyRoomMain::OnCardPackLBDown2()
{
	CardPackBehavior(1, 1);
}

void CRealMyRoomMain::OnCardPackLBDown3()
{
	CardPackBehavior(2, 1);
}

void CRealMyRoomMain::OnCardPackLBDown4()
{
	CardPackBehavior(3, 1);
}

void CRealMyRoomMain::OnCardPackLBDown5()
{
	CardPackBehavior(4, 1);
}

void CRealMyRoomMain::OnCardPackLBDown6()
{
	CardPackBehavior(5, 1);
}

void CRealMyRoomMain::OnCardPackLBDown7()
{
	CardPackBehavior(6, 1);
}

void CRealMyRoomMain::OnCardPackLBDown8()
{
	CardPackBehavior(7, 1);
}

void CRealMyRoomMain::OnCardPackRBDown1()
{
	CardPackBehavior(0, 2);
}

void CRealMyRoomMain::OnCardPackRBDown2()
{
	CardPackBehavior(1, 2);
}

void CRealMyRoomMain::OnCardPackRBDown3()
{
	CardPackBehavior(2, 2);
}

void CRealMyRoomMain::OnCardPackRBDown4()
{
	CardPackBehavior(3, 2);
}

void CRealMyRoomMain::OnCardPackRBDown5()
{
	CardPackBehavior(4, 2);
}

void CRealMyRoomMain::OnCardPackRBDown6()
{
	CardPackBehavior(5, 2);
}

void CRealMyRoomMain::OnCardPackRBDown7()
{
	CardPackBehavior(6, 2);
}

void CRealMyRoomMain::OnCardPackRBDown8()
{
	CardPackBehavior(7, 2);
}

void CRealMyRoomMain::BoundCardPage(FrButton* pButton, unsigned char slot)
{
	if (pButton && m_pCardBook)
	{
		CCardPage* pPage = m_pCardBook->GetCardPage();
		if (pPage)
			pPage->BoundSlotControl(slot, pButton);
		pButton->UseRightButton(true);
		if (IsLocalContent(S4_ITEM_STORAGE) == true && CItemStorage::Instance())
			CItemStorage::Instance()->AddBackGroundUI(pButton, false);
	}
}

void CRealMyRoomMain::OnCardInit1(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 0);
}

void CRealMyRoomMain::OnCardInit2(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 1);
}

void CRealMyRoomMain::OnCardInit3(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 2);
}

void CRealMyRoomMain::OnCardInit4(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 3);
}

void CRealMyRoomMain::OnCardInit5(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 4);
}

void CRealMyRoomMain::OnCardInit6(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 5);
}

void CRealMyRoomMain::OnCardInit7(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 6);
}

void CRealMyRoomMain::OnCardInit8(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 7);
}

void CRealMyRoomMain::OnCardInit9(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 8);
}

void CRealMyRoomMain::OnCardInit10(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 9);
}

void CRealMyRoomMain::OnCardInit11(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 10);
}

void CRealMyRoomMain::OnCardInit12(int param)
{
	BoundCardPage(DYNAMIC_CAST(FrButton, (FrWnd*)param), 11);
}

void CRealMyRoomMain::CardBehavior(unsigned char slot, unsigned char type)
{
	CCardBook* pCardBook = m_pCardBook;
	if (pCardBook == NULL || m_b_D98)
		return;
	CCardPage* pPage = pCardBook->GetCardPage();
	if (pPage == NULL)
		return;
	pCardBook->SetSelectedSlot(slot);
	pPage->SelectSlot(type, slot);
	if (m_pCardBook->m_pUseButton)
	{
		if (pPage->IsSelectedSlot())
		{
			if (pPage->GetCardSlot()[slot].GetCards()->tid == 0x7c800026)
			{
				m_pCardBook->m_pUseButton->SetVisible(false);
				return;
			}
			m_pCardBook->m_pUseButton->SetVisible(true);
			return;
		}
		m_pCardBook->m_pUseButton->SetVisible(false);
	}
}

void CRealMyRoomMain::OnCardLBDown1()
{
	CardBehavior(0, 1);
}

void CRealMyRoomMain::OnCardLBDown2()
{
	CardBehavior(1, 1);
}

void CRealMyRoomMain::OnCardLBDown3()
{
	CardBehavior(2, 1);
}

void CRealMyRoomMain::OnCardLBDown4()
{
	CardBehavior(3, 1);
}

void CRealMyRoomMain::OnCardLBDown5()
{
	CardBehavior(4, 1);
}

void CRealMyRoomMain::OnCardLBDown6()
{
	CardBehavior(5, 1);
}

void CRealMyRoomMain::OnCardLBDown7()
{
	CardBehavior(6, 1);
}

void CRealMyRoomMain::OnCardLBDown8()
{
	CardBehavior(7, 1);
}

void CRealMyRoomMain::OnCardLBDown9()
{
	CardBehavior(8, 1);
}

void CRealMyRoomMain::OnCardLBDown10()
{
	CardBehavior(9, 1);
}

void CRealMyRoomMain::OnCardLBDown11()
{
	CardBehavior(10, 1);
}

void CRealMyRoomMain::OnCardLBDown12()
{
	CardBehavior(11, 1);
}

void CRealMyRoomMain::OnCardRBDown1()
{
	CardBehavior(0, 2);
}

void CRealMyRoomMain::OnCardRBDown2()
{
	CardBehavior(1, 2);
}

void CRealMyRoomMain::OnCardRBDown3()
{
	CardBehavior(2, 2);
}

void CRealMyRoomMain::OnCardRBDown4()
{
	CardBehavior(3, 2);
}

void CRealMyRoomMain::OnCardRBDown5()
{
	CardBehavior(4, 2);
}

void CRealMyRoomMain::OnCardRBDown6()
{
	CardBehavior(5, 2);
}

void CRealMyRoomMain::OnCardRBDown7()
{
	CardBehavior(6, 2);
}

void CRealMyRoomMain::OnCardRBDown8()
{
	CardBehavior(7, 2);
}

void CRealMyRoomMain::OnCardRBDown9()
{
	CardBehavior(8, 2);
}

void CRealMyRoomMain::OnCardRBDown10()
{
	CardBehavior(9, 2);
}

void CRealMyRoomMain::OnCardRBDown11()
{
	CardBehavior(10, 2);
}

void CRealMyRoomMain::OnCardRBDown12()
{
	CardBehavior(11, 2);
}

void CRealMyRoomMain::OnItemListInit(int param)
{
	m_pFrListBox_304 = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pFrListBox_304)
	{
		m_pFrListBox_304->UseRightButton(true);
		m_pFrListBox_304->UseDummy(true);
		m_pFrListBox_304->SetPushSound(NULL);
		m_pFrListBox_304->AddNoRButtonRect(m_WRect_9E0, 0);
		m_pFrListBox_304->AddNoRButtonRect(m_WRect_A50, 4);
		m_pFrListBox_304->AddNoRButtonRect(m_WRect_9FC, 1);
		m_pFrListBox_304->AddNoRButtonRect(m_WRect_A18, 2);
		m_pFrListBox_304->AddNoRButtonRect(m_WRect_A34, 3);
		if (IsLocalContent(S4_REPLAY_SYSTEM))
		{
			m_pFrListBox_304->AddNoRButtonRect(m_WRect_AA4, 7);
			m_pFrListBox_304->AddNoRButtonRect(m_WRect_AC0, 8);
		}
		if (IsLocalContent(S4_CARD_SYSTEM))
			m_pFrListBox_304->AddNoRButtonRect(m_WRect_ADC, 9);
		if (IsLocalContent(S4_REAL_MYROOM))
			m_pFrListBox_304->AddNoRButtonRect(m_WRect_AF8, 10);
		if (IsLocalContent(S4_ITEM_STORAGE) == true && CItemStorage::Instance())
			CItemStorage::Instance()->SetItemListArea(
				m_pFrListBox_304->GetRect());
	}
}

void CRealMyRoomMain::DrawStatus(FrGraphicInterface* pGI, FrListItem* pItem,
	short* const status, short* const addStatus)
{
	float y = 4.0f;
	for (int i = 0; i < 5; i++)
	{
		const Bitmap* pAddImg = m_pStatusBarImg[i][1];
		const Bitmap* pImg = m_pStatusBarImg[i][0];
		int nStatus = status[i];
		int nAdd = addStatus[i];
		if (nAdd != 0 || nStatus != 0)
		{
			float x = 86.0f;
			for (int j = 0; j < nStatus; j++)
			{
				pGI->DrawTexture(pImg,
					WRect(x + pItem->pos.x, y + pItem->pos.y, 5.0f, 6.0f),
					0xffffffff, 0);
				x -= 6.0f;
			}
			for (int k = 0; k < nAdd; k++)
			{
				pGI->DrawTexture(pAddImg,
					WRect(x + pItem->pos.x, y + pItem->pos.y, 5.0f, 6.0f),
					0xffffffff, 0);
				x -= 6.0f;
			}
			y += 6.0f;
		}
	}
}

void CRealMyRoomMain::DrawStatus2(FrGraphicInterface* pGI, FrListItem* pItem,
	short* const status, short* const addStatus, float offset)
{
	float y = 0.0f;
	for (int i = 0; i < 5; i++)
	{
		if (status[i] != 0 || addStatus[i] != 0)
		{
			pGI->SetTextColor(0xff0000ff, 0xffffffff);
			WPoint pt(offset + 182.0f, y + offset + 36.0f);
			pGI->Print11(pt + pItem->pos, 2,
				MakeStatusString(i, status[i], addStatus[i]));
			y += 14.0f;
		}
	}
}

void CRealMyRoomMain::DrawStatusCard(FrGraphicInterface* pGI, FrListItem* pItem,
	short* const status, float offset)
{
	float y = 0.0f;
	for (int i = 0; i < 5; i++)
	{
		if (status[i] != 0)
		{
			pGI->SetTextColor(0xff0000ff, 0xffffffff);
			WPoint pt(offset + 182.0f, y + offset + 36.0f);
			pGI->Print11(pt + pItem->pos, 2,
				MakeCardStatusString(i, status[i]));
			y += 14.0f;
		}
	}
}

// run 45: OnItemListOwnerDraw (ghidra.c "OnItemListOwnerDraw @", orig_dis __pg_c_00337b)
#define IsOverItemBtn(pItem, rc) \
	((pItem)->underCursor && \
		((rc) + (pItem)->pos).IsInRect(g_pFresh->GetManager()->GetMousePos()))

#define DrawItemBtn(pGI, pItem, rc, pBitmap) \
	(pGI)->DrawTexture((pBitmap), ((rc) + (pItem)->pos) + WPoint(0.0f, 0.0f), \
		0xffffffff, 0)

void CRealMyRoomMain::OnItemListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;

	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	const Bitmap* pBack;
	if (pItem->pData == NULL)
	{
		pBack = m_pBitmap_490;
	}
	else
	{
		unsigned long color;
		if (!pItem->selected)
		{
			color = 0xff000000;
			if (!pItem->underCursor)
				pBack = m_pBitmap_484;
			else
				pBack = m_pBitmap_488;
		}
		else
		{
			pBack = m_pBitmap_48C;
			color = 0xffff0000;
		}
		pGI->SetTextColor(color, 0xffffffff);
	}
	pGI->SetTextStyle(0);
	if (pBack != NULL)
	{
		WRect rect(pItem->pos.x, pItem->pos.y, (float)pBack->Width(),
			(float)pBack->Height());
		pGI->DrawTexture(pBack, rect, 0xffffffff, 0);
	}

	sItemInfo* pInfo = (sItemInfo*)pItem->pData;
	if (pInfo == NULL)
		return;

	unsigned long tid = 0;
	const char* name =
		K2L_Compatibility("\xbe\xcb\xbc\xf6\xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb");
	const char* iconName = "caddie_no";
	int count = -1;
	bool bUsable = false;
	bool bCaddieRest = false;
	bool bD5 = false;
	unsigned char grade = 0;
	int days = 0;
	int hours = 0;
	bool bPartDays = false;
	int partDays = 0;
	int partHours = 0;
	unsigned char pcbang = 0;
	unsigned char timeFlag = 0;
	const Bitmap* pIconOverride = NULL;
	unsigned long courseId = 0;
	SYSTEMTIME st;

	switch (m_n_77C)
	{
	case 8:
	{
		tid = pInfo->tid;
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
		if (pCommon != NULL)
		{
			name = pCommon->Name;
			iconName = pCommon->Icon;
			bUsable = !pCommon->IsCash;
			count = pInfo->Common[0];
			grade = pInfo->ItemType;
			unsigned long* pEquip = Doc()->m_myInfo.userEquip.tidItemSlot;
			for (int i = 0; i < 10; i++)
			{
				if (tid == pEquip[i])
					count--;
			}
		}
		break;
	}
	default:
	{
		tid = pInfo->tid;
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
		if (pCommon != NULL)
		{
			name = pCommon->Name;
			iconName = pCommon->Icon;
			bUsable = !pCommon->IsCash;
			count = pInfo->Common[0];
			grade = pInfo->ItemType;
			days = pInfo->HourRemain / 0x18;
			hours = pInfo->HourRemain % 0x18;
		}
		if (IsLocalContent(S4_UCC) && IsUccClothes(pInfo->tid))
		{
			name = GetUccItemName(pInfo);
			pIconOverride = GetUccItemIcon(pInfo);
			if (m_n_77C == 0x15 && (pInfo->status & 1) == 0)
			{
				IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tid);
				if (pPart != NULL)
				{
					int cat = pPart->Category;
					if (cat == 7 || cat == 9)
					{
						WRect& rcCopy = (&m_WRect_C28)[cat != 7];
						if (IsOverItemBtn(pItem, rcCopy))
						{
							DrawItemBtn(pGI, pItem, rcCopy, m_pBitmap_B44);
							CIconToolTip::GetInstance()->SetToolTip(
								UCC_TOOLTIP_COPY);
						}
						else
						{
							DrawItemBtn(pGI, pItem, rcCopy, m_pBitmap_B40);
						}
					}
					if (cat == 7 || cat == 8)
					{
						const Bitmap* pNormal;
						const Bitmap* pHover;
						eBarButton tip;
						if ((pInfo->status & 2) == 0)
						{
							pNormal = m_pBitmap_B24;
							pHover = m_pBitmap_B28;
							tip = UCC_TOOLTIP_DRAW;
						}
						else
						{
							pNormal = m_pBitmap_B5C;
							pHover = m_pBitmap_B60;
							tip = UCC_TOOLTIP_REDRAW;
						}
						if (IsOverItemBtn(pItem, m_WRect_C38))
						{
							DrawItemBtn(pGI, pItem, m_WRect_C38, pHover);
							CIconToolTip::GetInstance()->SetToolTip(tip);
						}
						else
						{
							DrawItemBtn(pGI, pItem, m_WRect_C38, pNormal);
						}
					}
				}
			}
		}
		break;
	}
	case 10:
	{
		sCaddieInfo* pCad = (sCaddieInfo*)pInfo;
		tid = pCad->tid;
		IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(tid);
		if (pCaddie != NULL)
		{
			name = pCaddie->c.Name;
			days = pCad->Remain_Date;
			iconName = pCaddie->c.Icon;
			count = 1;
			if (pCad->Rent_flag && pCaddie->MonthlyFee != 0)
				grade = 1;
			if ((pCad->tidPart & 0x1fff) != 0)
			{
				partDays = pCad->Remain_Partdate / 0x18;
				partHours = (unsigned int)pCad->Remain_Partdate % 0x18;
				bPartDays = true;
			}
		}
		if (pCad->Rent_flag && pCad->Remain_Date == 0 && pCad->tidPart == 0 &&
			pCaddie->MonthlyFee != 0)
			bCaddieRest = true;
		break;
	}
	case 12:
	{
		sMascotInfo* pMascot = (sMascotInfo*)pInfo;
		tid = pMascot->tid;
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
		if (pCommon != NULL)
		{
			name = pCommon->Name;
			days = pMascot->Remain_Date / 0x18;
			hours = (unsigned int)pMascot->Remain_Date % 0x18;
			iconName = pCommon->Icon;
			count = 1;
			timeFlag = pCommon->TimeFlag;
		}
		pcbang = REBANG_983_PCBANG_MASCOT(*pMascot);
		break;
	}
	case 0x11:
		if (!IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
		{
			sGiftInfo* pGift = (sGiftInfo*)pItem->pData;
			tid = pGift->tid;
			IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
			if (pCommon != NULL)
			{
				count = pGift->Arg0;
				name = pCommon->Name;
				iconName = pCommon->Icon;
				grade = pGift->ItemType;
			}
			if (pGift->sFromID[0] != '@')
				bD5 = true;
		}
		break;
	case 0x12:
	{
		tid = pInfo->tid & 0xfffffff0;
		unsigned long matchIdx = pInfo->tid & 0xf;
		IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(tid);
		if (pMatch != NULL)
		{
			name = pMatch->Name;
			count = *(int*)((char*)pInfo + 8);
			iconName = pMatch->Icon[matchIdx];
		}
		break;
	}
	case 0x14:
	{
		tid = *(unsigned long*)pInfo;
		IFF_STRUCT::sQuest* pQuest = ItemManager()->FindQuest(tid);
		if (pQuest != NULL)
		{
			name = pQuest->Name;
			iconName = pQuest->Icon;
			count = 1;
		}
		break;
	}
	case 0x17:
		if (IsLocalContent(S4_REPLAY_SYSTEM))
		{
			sRecordedItemInfo* pRecorded = (sRecordedItemInfo*)pItem->pData;
			tid = pRecorded->itemInfo.tid;
			courseId = pRecorded->course;
			st = pRecorded->date;
			IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
			if (pCommon != NULL)
			{
				name = pCommon->Name;
				iconName = pCommon->Icon;
				count = 1;
			}
		}
		break;
	case 0x1a:
	{
		tid = pInfo->tid;
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
		if (pCommon != NULL)
		{
			iconName = pCommon->Icon;
			bUsable = !pCommon->IsCash;
			count = 1;
			grade = 0;
			days = 0;
			hours = 0;
		}
		name = GetUccItemName(pInfo);
		pIconOverride = GetUccItemIcon(pInfo);
		if ((pInfo->status & 1) == 0)
		{
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tid);
			if (pPart != NULL)
			{
				int cat = pPart->Category;
				if (cat == 7 || cat == 9)
				{
					WRect& rcCopy = (&m_WRect_C28)[cat != 7];
					if (IsOverItemBtn(pItem, rcCopy))
					{
						DrawItemBtn(pGI, pItem, rcCopy, m_pBitmap_B44);
						CIconToolTip::GetInstance()->SetToolTip(
							UCC_TOOLTIP_COPY);
					}
					else
					{
						DrawItemBtn(pGI, pItem, rcCopy, m_pBitmap_B40);
					}
				}
				if (cat == 7 || cat == 8)
				{
					const Bitmap* pNormal;
					const Bitmap* pHover;
					eBarButton tip;
					if ((pInfo->status & 2) == 0)
					{
						pNormal = m_pBitmap_B24;
						pHover = m_pBitmap_B28;
						tip = UCC_TOOLTIP_DRAW;
					}
					else
					{
						pNormal = m_pBitmap_B5C;
						pHover = m_pBitmap_B60;
						tip = UCC_TOOLTIP_REDRAW;
					}
					if (IsOverItemBtn(pItem, m_WRect_C38))
					{
						DrawItemBtn(pGI, pItem, m_WRect_C38, pHover);
						CIconToolTip::GetInstance()->SetToolTip(tip);
					}
					else
					{
						DrawItemBtn(pGI, pItem, m_WRect_C38, pNormal);
					}
				}
			}
		}
		break;
	}
	case 0x1b:
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			sCards* pCards = (sCards*)pItem->pData;
			if (pCards->tid != 0)
			{
				IFF_STRUCT::sCard* pCard =
					ItemManager()->FindCard(pCards->tid);
				if (pCard != NULL)
				{
					tid = pCards->tid;
					count = pCards->cardCount;
					name = pCard->c.Name;
					iconName = pCard->c.Icon;
				}
			}
		}
		break;
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
	{
		tid = pInfo->tid;
		IFF_STRUCT::sFurniture* pFurniture;
		if ((tid & 0xfc000000) == 0x48000000 &&
			(pFurniture = ItemManager()->FindFurniture(tid)) != NULL)
		{
			name = pFurniture->c.Name;
			iconName = pFurniture->c.Icon;
			count = 1;
		}
		break;
	}
	}

	const Bitmap* pIcon = g_pFresh->GetBitmap(iconName);
	if (pIconOverride != NULL)
		pIcon = pIconOverride;
	if (CItemStorage::Instance() != NULL &&
		CItemStorage::Instance()->GetDragingItemIndex(
			sStoredItemDragDrop::MODE_PUSH) == (unsigned long)pItem->idx)
		pIcon = NULL;

	WPoint iconPos;
	if (S5::ISCutinItem(tid))
		iconPos = WPoint(pItem->pos.x, pItem->pos.y - 12.0f);
	else
		iconPos = WPoint(pItem->pos.x, pItem->pos.y);
	ReviseItemIconPoint(tid, iconPos);
	if (pIcon != NULL)
	{
		WRect rect(iconPos.x, iconPos.y, (float)pIcon->Width(),
			(float)pIcon->Height());
		pGI->DrawTexture(pIcon, rect, 0xffffffff, 0);
	}

	if (grade > 1 && tid != 0x1a000042 && grade != 6)
		pGI->DrawTexture(m_pBitmap_A7C,
			(m_WRect_A6C + iconPos) + WPoint(0.0f, 0.0f), 0xffffffff, 0);
	if (bCaddieRest)
	{
		const Bitmap* pRest = g_pFresh->GetBitmap("caddie_rest");
		WRect rect(iconPos.x, iconPos.y, (float)pRest->Width(),
			(float)pRest->Height());
		pGI->DrawTexture(pRest, rect, 0xffffffff, 0);
	}

	pGI->SetTextStyle(0);
	if (m_n_77C == 0x12)
	{
		char buf[100];
		memset(buf, 0, sizeof(buf));
		strcpy(buf, name);
		char* pText = buf;
		int line = 0;
		char* pNewLine = strstr(pText, "\n");
		while (pNewLine != NULL)
		{
			*pNewLine = '\0';
			pGI->Print11(WPoint(pItem->pos.x + 182.0f,
							 (float)line * 15.0f + 7.0f + pItem->pos.y),
				2, pText);
			line++;
			pText = pNewLine + 1;
			pNewLine = strstr(pText, "\n");
		}
		pGI->Print11(WPoint(pItem->pos.x + 182.0f,
						 (float)line * 15.0f + 7.0f + pItem->pos.y),
			2, pText);
		sprintf(buf, "%d", count);
		pGI->Print11(WPoint(pItem->pos.x + 180.0f, pItem->pos.y + 67.0f), 2,
			buf);
	}
	else
	{
		pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 7.0f), 2,
			"%s", name);
	}

	if (IsLocalContent(S4_UCC) && IsUccClothes(tid))
	{
		sItemInfo* pUcc = (sItemInfo*)pItem->pData;
		float fx = pItem->pos.x + 51.0f;
		float fy = pItem->pos.y + 50.0f;
		unsigned char status = pUcc->status;
		const Bitmap* pMark = NULL;
		if ((status & 1) != 0 && pUcc->Seq == 1 && m_pBitmap_5C0 != NULL)
			pMark = m_pBitmap_5C0;
		else if ((status & 1) != 0 && pUcc->Seq > 1 && m_pBitmap_5C4 != NULL)
			pMark = m_pBitmap_5C4;
		else if ((status & 2) != 0 && m_pBitmap_5C8 != NULL)
			pMark = m_pBitmap_5C8;
		if (pMark != NULL)
		{
			WRect rect(fx, fy, (float)pMark->Width(), (float)pMark->Height());
			pGI->DrawTexture(pMark, rect, 0xffffffff, 0);
		}
		if ((pUcc->status & 1) != 0)
		{
			pGI->SetTextColor(0xff9e2d3e, 0xffffffff);
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 21.0f), 2,
				pUcc->CopierNick);
			pGI->SetTextColor(0xff000000, 0xffffffff);
		}
	}

	char bHandled = 0;
	GetActor("ItemBuff") << MsgObject(NULL, M983(0x246), 0, (int)&bHandled,
		(int)pItem, (int)&g_pFresh->GetManager()->GetMousePos(), tid);

	int btnKind = m_n_77C;
	if (bHandled != 0)
		btnKind = 0x23;

	WPoint pos1(pItem->pos.x + 182.0f, pItem->pos.y + 23.0f);
	WPoint pos2(pos1.x, pos1.y + 16.0f);

	switch (btnKind)
	{
	case 8:
		if (IsOverItemBtn(pItem, m_WRect_9FC))
		{
			DrawItemBtn(pGI, pItem, m_WRect_9FC, m_pBitmap_A10);
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_EQUIP);
		}
		else
		{
			DrawItemBtn(pGI, pItem, m_WRect_9FC, m_pBitmap_A0C);
		}
		if (bUsable)
		{
			if (IsOverItemBtn(pItem, m_WRect_A18))
			{
				DrawItemBtn(pGI, pItem, m_WRect_A18, m_pBitmap_A2C);
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_DELETE);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_A18, m_pBitmap_A28);
			}
		}
		break;
	case 9:
	{
		bool bOpenBtn = false;
		bool bItemBtn = false;
		switch (tid)
		{
		case 0x1a0000bc:
		case 0x1a0000d6:
		case 0x1a000105:
		case 0x1a00012f:
		case 0x1a000130:
		case 0x1a000131:
		case 0x1a00014b:
		case 0x1a000003:
		case 0x1a00003e:
		case 0x1a000048:
		case 0x1a000052:
		case 0x1a000054:
			bOpenBtn = true;
			break;
		case 0x1a00003b:
			if (IsOverItemBtn(pItem, m_WRect_9E0))
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F4);
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_HANDSEL);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F0);
			}
			break;
		default:
			bItemBtn = true;
			break;
		}
		if (bOpenBtn)
		{
			if (IsOverItemBtn(pItem, m_WRect_9E0))
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F4);
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_OPENGIFT);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F0);
			}
		}
		else if (bItemBtn)
		{
			IFF_STRUCT::sItem* pSItem = ItemManager()->FindItem(tid);
			if (pSItem != NULL && pSItem->IsRandomBox())
			{
				if (IsOverItemBtn(pItem, m_WRect_9E0))
				{
					DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F4);
					CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_OPENGIFT);
				}
				else
				{
					DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F0);
				}
			}
			if (FrCardRemove::IsCardRemoveItem(tid))
			{
				if (IsOverItemBtn(pItem, m_WRect_9E0))
					DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F4);
				else
					DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F0);
			}
		}
		break;
	}
	case 10:
		if (bCaddieRest)
		{
			if (IsOverItemBtn(pItem, m_WRect_A34))
			{
				DrawItemBtn(pGI, pItem, m_WRect_A34, m_pBitmap_A48);
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_REEMPLOY);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_A34, m_pBitmap_A44);
			}
		}
		break;
	case 0x11:
		if (!IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
		{
			if (IsOverItemBtn(pItem, m_WRect_9E0))
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F4);
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_MOVEGIFT);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F0);
			}
			if (bD5)
			{
				if (IsOverItemBtn(pItem, m_WRect_A50))
				{
					DrawItemBtn(pGI, pItem, m_WRect_A50, m_pBitmap_A64);
					CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_REPLY);
				}
				else
				{
					DrawItemBtn(pGI, pItem, m_WRect_A50, m_pBitmap_A60);
				}
			}
		}
		break;
	case 0x17:
		if (IsLocalContent(S4_REPLAY_SYSTEM))
		{
			if (IsOverItemBtn(pItem, m_WRect_AA4))
				DrawItemBtn(pGI, pItem, m_WRect_AA4, m_pBitmap_AB8);
			else
				DrawItemBtn(pGI, pItem, m_WRect_AA4, m_pBitmap_AB4);
			if (IsOverItemBtn(pItem, m_WRect_AC0))
				DrawItemBtn(pGI, pItem, m_WRect_AC0, m_pBitmap_AD4);
			else
				DrawItemBtn(pGI, pItem, m_WRect_AC0, m_pBitmap_AD0);
		}
		break;
	case 0x18:
		if (tid == 0x1a000042 && IsLocalContent(S3_CADDIE_REPORT))
		{
			if (IsOverItemBtn(pItem, m_WRect_9E0))
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F4);
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_REPORT);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_9E0, m_pBitmap_9F0);
			}
		}
		break;
	case 0x1b:
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			if (IsOverItemBtn(pItem, m_WRect_ADC))
			{
				DrawItemBtn(pGI, pItem, m_WRect_ADC, m_pBitmap_AF0);
				CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_CARD);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_ADC, m_pBitmap_AEC);
			}
		}
		break;
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			if (IsOverItemBtn(pItem, m_WRect_AF8))
			{
				DrawItemBtn(pGI, pItem, m_WRect_AF8, m_pBitmap_B0C);
				CIconToolTip::GetInstance()->SetToolTip(RMR_RM_DECO);
			}
			else
			{
				DrawItemBtn(pGI, pItem, m_WRect_AF8, m_pBitmap_B08);
			}
		}
		break;
	}

	if (count == -1)
	{
		pGI->Print11(pos1, 2, "%d", tid);
		return;
	}

	unsigned int kind = tid >> 26;
	switch (kind)
	{
	case 2:
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(tid);
		if (pPart != NULL)
		{
			DrawStatus2(pGI, pItem, pPart->Attr,
				pPart->Slot, 0.0f);
			if (m_n_77C != 0x11)
				DrawPartsSlot(pGI, pItem);
		}
		sItemInfo* pPartInfo = (sItemInfo*)pItem->pData;
		if (pPartInfo != NULL && pPartInfo->ItemType == 6)
		{
			if (!pPartInfo->Expired)
			{
				const Bitmap* pClock =
					g_pFresh->GetBitmap("myroom_rental_clock");
				if (pClock != NULL)
				{
					WRect rect(pItem->pos.x + 70.0f, pItem->pos.y + 43.0f,
						pClock->Width(), pClock->Height());
					pGI->DrawTexture(pClock, rect, 0xffffffff, 0);
				}
			}
			else
			{
				const Bitmap* pExpired =
					g_pFresh->GetBitmap("rental_item_expired");
				if (pExpired == NULL)
					return;
				WPoint pt(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f);
				WRect rect(pt, pExpired->Width(), pExpired->Height());
				pGI->DrawTexture(pExpired, rect, 0xffffffff, 0);
				return;
			}
		}
		else if (pPartInfo->ItemType < 2)
		{
			return;
		}
		pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
		if (days > 0)
		{
			pGI->Print11(pos1, 2,
				K2L_Compatibility("\xb3\xb2\xc0\xba\xb1\xe2\xb0\xa3 : %d\xc0\xcf %d\xbd\xc3\xb0\xa3"),
				days, hours);
			return;
		}
		pGI->SetTextColor(0xffff0000, 0xffffffff);
		pGI->Print11(pos1, 2,
			K2L_Compatibility("\xb3\xb2\xc0\xba\xb1\xe2\xb0\xa3 : %d\xbd\xc3\xb0\xa3"), hours);
		return;
	}
	case 4:
	{
		int total = 0;
		if (m_n_77C != 0x11)
		{
			short* pCommon = ((sItemInfo*)pItem->pData)->Common;
			for (unsigned int slot = 0; slot < 5; slot++)
			{
				for (int k = 0; k < pCommon[slot]; k++)
				{
					IFF_STRUCT::sEnchant* pEnchant = ItemManager()->FindEnchant(
						k | ((slot | 0x340) << 0x14));
					if (pEnchant != NULL)
						total += pEnchant->ReqdPang;
				}
			}
		}
		pGI->SetTextColor(0xff009000, 0xffffffff);
		pGI->Print11(pos1, 2,
			K2L_Compatibility("\xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xc7\xd1 \xba\xf1\xbf\xeb:"));
		pGI->Print11(pos2, 2, K2L_Compatibility("%d\xc6\xce"), total);
		return;
	}
	case 5:
		if (!ItemManager()->IsBasicAztecFamily(tid))
			pGI->Print11(pos1, 2, "%d\xb0\xb3", count);
		break;
	case 6:
		if (grade > 1)
		{
			if (tid == 0x1a000042 && IsLocalContent(S3_CADDIE_REPORT))
			{
				const _SYSTEMTIME& date = ((sItemInfo*)pItem->pData)->ItemDate;
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(pos1, 2, K2L_Compatibility("%d\xb3\xe2 %d\xbf\xf9 %d\xc0\xcf"),
					date.wYear,
					date.wMonth,
					date.wDay);
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(pos2, 2, K2L_Compatibility(" %d\xbd\xc3 %d \xba\xd0"),
					date.wHour,
					date.wMinute);
				return;
			}
			if (days == 0 && hours == 0)
			{
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(pos1, 2, "\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3:");
				pGI->SetTextColor(0xffff0000, 0xffffffff);
				pGI->Print11(pos2, 2, "1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8");
				return;
			}
			pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
			pGI->Print11(pos1, 2, "\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3:");
			pGI->Print11(pos2, 2, "%d\xc0\xcf %d\xbd\xc3\xb0\xa3", days, hours);
			return;
		}
		if (IsLocalContent(S4_REPLAY_SYSTEM) && m_n_77C == 0x17)
		{
			IFF_STRUCT::sCourse* pCourse =
				ItemManager()->FindCourse(courseId | 0x28000000);
			if (pCourse != NULL)
			{
				WPoint posName(pItem->pos.x + 182.0f, pItem->pos.y + 22.0f);
				pGI->Print11(posName, 2, pCourse->c.Name);
			}
			char date[52];
			char time[52];
			date[0] = '\0';
			memset(date + 1, 0, 0x31);
			time[0] = '\0';
			memset(time + 1, 0, 0x31);
			wsprintfA(date, "%d\xb3\xe2%02d\xbf\xf9%02d\xc0\xcf", st.wYear,
				st.wMonth, st.wDay);
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 33.0f), 2,
				date);
			wsprintfA(time, "%02d\xbd\xc3%02d\xba\xd0", st.wHour, st.wMinute);
			pGI->Print11(WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 44.0f), 2,
				time);
			return;
		}
		if (tid != 0x1a000006)
		{
			if (tid == 0x1a000010)
				pGI->Print11(pos1, 2, K2L_Compatibility("%d\xc6\xce"), count);
			else
				pGI->Print11(pos1, 2, "%d\xb0\xb3", count);
			return;
		}
		break;
	case 7:
		if (grade == 1)
		{
			if (days == 0 && hours == 0)
			{
				pGI->SetTextColor(0xffff0000, 0xffffffff);
				pGI->Print11(pos1, 2, K2L_Compatibility("\xc8\xde\xb0\xa1\xc1\xdf"));
				return;
			}
			pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
			pGI->Print11(pos1, 2,
				K2L_Compatibility("\xb3\xb2\xc0\xba \xb0\xed\xbf\xeb\xb1\xe2\xb0\xa3:"));
			pGI->Print11(pos2, 2, "%d\xc0\xcf", days);
			if (bPartDays)
			{
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(WPoint(pos2.x, pos2.y + 16.0f), 2,
					K2L_Compatibility("\xb2\xd9\xb9\xcc\xb1\xe2 \xb3\xb2\xc0\xba \xb1\xe2\xb0\xa3:"));
				pGI->Print11(WPoint(pos2.x, pos2.y + 32.0f), 2,
					"%d\xc0\xcf %d\xbd\xc3\xb0\xa3", partDays, partHours);
				return;
			}
		}
		else if (bPartDays)
		{
			pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
			pGI->Print11(pos1, 2,
				K2L_Compatibility("\xb2\xd9\xb9\xcc\xb1\xe2 \xb3\xb2\xc0\xba \xb1\xe2\xb0\xa3:"));
			pGI->Print11(pos2, 2, "%d\xc0\xcf %d\xbd\xc3\xb0\xa3", partDays,
				partHours);
		}
		break;
	case 0xe:
	{
		if ((tid & 0x3c00000) != 0x1800000)
			return;
		Title* pTitle = g_TitleManager->GetTitle((eTitle)(tid & 0x3fffff));
		if (pTitle == NULL)
			return;
		if (pTitle->CanEquip())
			return;
		const Bitmap* pNoFit = g_pFresh->GetBitmap("do not fit");
		if (pNoFit == NULL)
			return;
		WPoint pt(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f);
		WRect rect(pt, pNoFit->Width(), pNoFit->Height());
		pGI->DrawTexture(pNoFit, rect, 0xffffffff, 0);
		break;
	}
	case 0x10:
		if (m_n_77C != 0x11 && pcbang == 0)
		{
			if (timeFlag == 0)
			{
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(pos1, 2, "\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3:");
				pGI->Print11(pos2, 2, K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"), days, hours);
				return;
			}
			if (days == 0 && hours == 0)
			{
				pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
				pGI->Print11(pos1, 2, "\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3:");
				pGI->SetTextColor(0xffff0000, 0xffffffff);
				pGI->Print11(pos2, 2, "1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8");
				return;
			}
			pGI->SetTextColor(0xff4d4d4d, 0xffffffff);
			pGI->Print11(pos1, 2, "\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3:");
			pGI->Print11(pos2, 2, "%d\xc0\xcf %d\xbd\xc3\xb0\xa3", hours, days);
			return;
		}
		break;
	case 0x12:
		if (!IsLocalContent(S4_REAL_MYROOM))
			return;
		if (kind != 0x12)
			return;
		if (!GetArrangement(tid, pInfo->guid))
			return;
		{
			const Bitmap* pCheck = g_pFresh->GetBitmap("rmr_arg_item_check");
			WRect rect(pItem->pos.x + 8.0f, pItem->pos.y + 62.0f,
				pCheck->Width(), pCheck->Height());
			pGI->DrawTexture(pCheck, rect, 0xffffffff, 0);
		}
		break;
	case 0x1c:
		if ((tid & 0x1f0000) == 0)
			pGI->Print11(pos1, 2, "%d\xb0\xb3", count);
		break;
	case 0x1f:
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			pGI->Print11(pos1, 2, "%d\xb0\xb3", count);
			IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(
				*(unsigned long*)((char*)pItem->pData + 4));
			DrawStatusCard(pGI, pItem, pCard->COM, 0.0f);
			return;
		}
		break;
	}
}

void CRealMyRoomMain::MoveGift(sGiftInfo* pGift)
{
	IFF_ITEM_COMMON* pItem = Doc()->m_itemManager.FindCommonItem(pGift->tid);
	if (!Doc()->ConfirmInsertItem(pGift->tid, pGift->Arg0))
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		WRect rect(pForm->GetRect());
		pForm->SetRect(rect);
		pForm->SetTimeLimit(12.0f);
		pForm->SetMessage(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xba\xb8\xc0\xaf\xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb1\xb8\xc0\xd4 \xb6\xc7\xb4\xc2 \xbf\xc5\xb1\xe6 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 0);
		return;
	}
	switch (pGift->tid >> 26)
	{
	case 8:
	{
		std::map<unsigned int, sCaddieInfo>& caddies = Doc()->m_caddieMap;
		std::map<unsigned int, sCaddieInfo>::const_iterator it;
		std::map<unsigned int, sCaddieInfo>::const_iterator itEnd =
			caddies.end();
		for (it = caddies.begin(); it != itEnd; ++it)
		{
			unsigned long tidCaddie = (pGift->tid >> 21 & 0x1f) | 0x1c000000;
			if ((*it).second.tid == tidCaddie)
			{
				IFF_STRUCT::sCaddie* pCaddie =
					Doc()->m_itemManager.FindCaddie((*it).second.tid);
				if ((*it).second.Remain_Date == 0 && pCaddie != NULL &&
					pCaddie->MonthlyFee != 0 && !pCaddie->c.IsCash)
				{
					this << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc8\xde\xb0\xa1\xc1\xdf\xc0\xce \xc4\xb3\xb5\xf0\xc0\xd4\xb4\xcf\xb4\xd9. \xc0\xe7\xb0\xed\xbf\xeb \xc8\xc4 \xbc\xb1\xb9\xb0 \xbf\xc5\xb1\xe2\xb1\xe2\xb8\xa6 \xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4"),
						0, 0, 0, 0);
					return;
				}
				break;
			}
		}
		break;
	}
	case 0xe:
		break;
	case 5:
	case 6:
		if (pGift->tid == 0x1a000010)
		{
			m_pFrPangGiftDlg_434 = CreateForm<FrPangGiftDlg>(
				g_pFresh->GetManager(), this, "panggift", NULL);
			m_pFrPangGiftDlg_434->SetItemInfo(pGift);
			m_pFrPangGiftDlg_434->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnPangGiftDlgResult, 1);
			return;
		}
		break;
	case 2:
	case 4:
		if ((pItem->IsCash) && GetItemNumInMyPocket(pGift->tid) != 0)
		{
			FrReturnGiftDlg* pDlg = CreateForm<FrReturnGiftDlg>(
				g_pFresh->GetManager(), this, "notify_yesno", NULL);
			unsigned long price = pItem->SalePrice;
			if (price == 0 || price >= pItem->Price)
				price = pItem->Price;
			pDlg->SetMessage(
				MakeStr(
					"\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \\c0xffff0000\\c\xc4\xed\xc5\xb0\xbe\xc6\xc0\xcc\xc5\xdb\\c0xff000000\\c\xc0\xd4\xb4\xcf\xb4\xd9.\n\\c0xffff0000\\c%d \xc4\xed\xc5\xb0\\c0xff000000\\c\xb7\xce \xc8\xaf\xba\xd2\xb9\xde\xc0\xbb \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n\xc4\xed\xc5\xb0\xb7\xce \xc0\xfc\xc8\xaf\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?",
					price, 0),
				false);
			pDlg->m_giftIdx = pGift->guid;
			pDlg->m_typeId = pGift->tid;
			pDlg->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnReturnGiftDlgResult, 0);
			return;
		}
		break;
	default:
		if ((pItem->IsCash) && GetItemNumInMyPocket(pGift->tid) != 0)
		{
			FrReturnGiftDlg* pDlg = CreateForm<FrReturnGiftDlg>(
				g_pFresh->GetManager(), this, "notify_yesno", NULL);
			unsigned long price = pItem->SalePrice;
			if (price == 0 || price >= pItem->Price)
				price = pItem->Price;
			pDlg->SetMessage(
				MakeStr(
					"\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \\c0xffff0000\\c\xc4\xed\xc5\xb0\xbe\xc6\xc0\xcc\xc5\xdb\\c0xff000000\\c\xc0\xd4\xb4\xcf\xb4\xd9.\n\\c0xffff0000\\c%d \xc4\xed\xc5\xb0\\c0xff000000\\c\xb7\xce \xc8\xaf\xba\xd2\xb9\xde\xc0\xbb \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n\xc4\xed\xc5\xb0\xb7\xce \xc0\xfc\xc8\xaf\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?",
					price, 0),
				false);
			pDlg->m_giftIdx = pGift->guid;
			pDlg->m_typeId = pGift->tid;
			pDlg->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnReturnGiftDlgResult, 0);
			return;
		}
		break;
	}
	m_psGiftInfo_7AC = pGift;
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
	pForm->SetMessage(
		"\xbc\xb1\xb9\xb0\xc0\xbb \xbf\xc5\xb1\xe2\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?",
		false);
	pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomMain::OnMoveGiftDlgResult, 1);
}

bool CRealMyRoomMain::OnCardRemoveDlgResult(int result, FrForm* pForm)
{
	m_pFrCardRemove_468 = NULL;
	return true;
}

void CRealMyRoomMain::OpenTikiReport(sItemInfo* pItem)
{
	if (IsLocalContent(S3_CADDIE_REPORT) && pItem)
	{
		if (pItem->guid == m_n_7D0)
		{
			HandleMsg(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc0\xaf\xc8\xbf\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xba \xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae \xc0\xd4\xb4\xcf\xb4\xd9. "),
				0, 0, 0, 0));
			return;
		}
		m_n_7D0 = pItem->guid;
		m_n_7D4 = pItem->tid;
		m_n_7D8 = pItem->Common[1] * 0x8000 + pItem->Common[2];
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		pForm->SetMessage(
			K2L_Compatibility("30\xc0\xce \xb4\xeb\xc8\xb8 \xb1\xe2\xb7\xcf\xc0\xce \xc6\xbc\xc5\xb0 \xb8\xae\xc6\xf7\xc6\xae\xb8\xa6 \xbf\xad\xb6\xf7 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			false);
		pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomMain::OnOpenTikiReport, 1);
	}
}

void __fastcall OnOpenedBoxDlg(unsigned long tid, FRESH_PFN_RESULT pFnResult,
	FrCmdTarget* pTarget)
{
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), pTarget,
		"notify_yesno", NULL);
	pForm->SetMessage(
		"\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xad\xbe\xee \xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?",
		false);
	pForm->Open(pFnResult, 1);
}

void CRealMyRoomMain::OpenNewYearMoney(sItemInfo* pInfo)
{
	if (pInfo)
	{
		m_n_7B0 = pInfo->guid;
		OnOpenedBoxDlg(pInfo->tid,
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnOpenNewYearMoneyDlgResult,
			this);
	}
}

void CRealMyRoomMain::OpenEventGiftBox(sItemInfo* pInfo)
{
	if (pInfo)
	{
		CGiftBoxEvent* pEvent;
		if (CContentsDoc::Instance()->GetContainer((localContentType_t)0x1e,
				pEvent))
		{
			m_n_7B4 = pInfo->guid;
			pEvent->SetOpenedBoxTID(pInfo->tid);
			if (pEvent->GetOpenedBoxTID() != 0)
				OnOpenedBoxDlg(pInfo->tid,
					(FRESH_PFN_RESULT)&CRealMyRoomMain::
						OnOpenEventGiftDlgResult,
					this);
		}
	}
}

void CRealMyRoomMain::OpenS4EventGiftBox(sItemInfo* pInfo)
{
	if (pInfo)
	{
		CGiftBoxEvent* pEvent;
		if (CContentsDoc::Instance()->GetContainer((localContentType_t)0x1e,
				pEvent))
		{
			m_n_7BC = pInfo->tid;
			pEvent->SetOpenedBoxTID(pInfo->tid);
			if (pEvent->GetOpenedBoxTID() != 0)
				OnOpenedBoxDlg(pInfo->tid,
					(FRESH_PFN_RESULT)&CRealMyRoomMain::
						OnOpenS4EventGiftDlgResult,
					this);
		}
	}
}

void CRealMyRoomMain::OnItemListBtnDown()
{
	// node of the item-attached card list at CSharedDoc+0x124c (std::list<>, value at +8)
	struct sAttachCardNode
	{
		sAttachCardNode* pNext;
	};
	FrListItem* pItem = m_pFrListBox_304->GetItemUnderCursor();
	if (pItem == NULL)
		return;
	if (m_b_D98 == false)
	{
		if (m_hand.IsWorking(0))
			goto toggle;
		if (m_n_77C == 0x16)
		{
			m_pFrListBox_304->ToggleItem(pItem);
			return;
		}
	}
	else
	{
		if (m_hand.IsWorking(0))
			goto toggle;
	}
	if (IsLocalContent(S4_ITEM_STORAGE) == true && m_b_D98 == true)
	{
		m_pFrListBox_304->ToggleItem(pItem);
		CItemStorage* pStorage = CItemStorage::Instance();
		if (pStorage == NULL)
			return;
		sStoredItemInfo stored;
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		if (pStorage->CheckStoredItemInfo(stored, *pInfo) != true)
			return;
		sStoredItemDragDrop drag;
		memset(&drag, 0, sizeof(drag));
		drag.mode = sStoredItemDragDrop::MODE_NONE;
		drag.index = pItem->idx;
		drag.pItemInfo = pInfo;
		drag.stored = stored;
		struct sUccIdx
		{
			char s[9];
		};
		*(sUccIdx*)((char*)&drag + 0x43) = *(sUccIdx*)pInfo->UccIndex;
		pStorage->SetDragDropItem(drag);
		return;
	}
	if (m_roomMode == 0 &&
		(CRealMyRoom_AvatarManager::Instance()->m_pAvatar == NULL ||
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->m_pPetFrame ==
				NULL))
	{
toggle:
		m_pFrListBox_304->ToggleItem(pItem);
		return;
	}
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	switch (m_n_77C)
	{
	case 8:
	{
		if ((m_WRect_9FC + pItem->pos).IsInRect(mouse) && pItem->pData != NULL)
		{
			EquipItem((sItemInfo*)pItem->pData);
			return;
		}
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		if ((m_WRect_A18 + pItem->pos).IsInRect(mouse) && pInfo != NULL &&
			!ItemManager()->FindCommonItem(pInfo->tid)->IsCash)
		{
			unsigned int tid = pInfo->tid;
			unsigned int nEquipped = 0;
			if ((tid & 0x2000000) == 0)
			{
				nEquipped = (tid == Doc()->m_myInfo.userEquip.tidItemSlot[0]);
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[1])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[2])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[3])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[4])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[5])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[6])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[7])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[8])
					nEquipped++;
				if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[9])
					nEquipped++;
			}
			short nCount = pInfo->Common[0];
			if (0 < (int)(nCount - nEquipped))
			{
				FrDeleteItemDlg* pDlg = CreateForm<FrDeleteItemDlg>(
					g_pFresh->GetManager(), this, "delete", NULL);
				pDlg->SetDelItem(pInfo->tid, nCount - nEquipped);
				pDlg->Open(NULL, 1);
			}
		}
		break;
	}
	case 9:
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		char bHandled = 0;
		GetActor("ItemBuff") << MsgObject(NULL, M983(0x246), 1, (int)&bHandled,
			(int)pItem, (int)&mouse, pInfo->tid);
		if (bHandled == 0)
		{
			if ((m_WRect_9E0 + pItem->pos).IsInRect(mouse))
				ExecuteMiniButtonDown(pInfo);
		}
		break;
	}
	case 10:
	{
		if ((m_WRect_A34 + pItem->pos).IsInRect(mouse))
		{
			sCaddieInfo* pInfo = (sCaddieInfo*)pItem->pData;
			if (pInfo != NULL)
			{
				IFF_STRUCT::sCaddie* pCaddie =
					ItemManager()->FindCaddie(pInfo->tid);
				if (pInfo->Rent_flag && pInfo->Remain_Date == 0 &&
					pInfo->tidPart == 0 && pCaddie->MonthlyFee != 0)
					OpenCaddieInfo(*pInfo);
			}
		}
		break;
	}
	case 0x11:
		if (!IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
		{
			if ((m_WRect_9E0 + pItem->pos).IsInRect(mouse) &&
				pItem->pData != NULL)
				MoveGift((sGiftInfo*)pItem->pData);
			sGiftInfo* pGift = (sGiftInfo*)pItem->pData;
			if ((m_WRect_A50 + pItem->pos).IsInRect(mouse) && pGift != NULL &&
				pGift->sFromID[0] != '@')
			{
				m_pFrNoteDlg_43C = CreateForm<FrNoteDlg>(g_pFresh->GetManager(),
					this, "note", NULL);
				m_pFrNoteDlg_43C->SetNick(pGift->sFromID, pGift->sMsg);
				m_pFrNoteDlg_43C->Open(
					(FRESH_PFN_RESULT)&CRealMyRoomMain::OnNoteDlgResult, 1);
			}
		}
		break;
	case 0x17:
		if (IsLocalContent(S4_REPLAY_SYSTEM))
		{
			if ((m_WRect_AA4 + pItem->pos).IsInRect(mouse))
			{
				sRecordedItemInfo* pReplay = (sRecordedItemInfo*)pItem->pData;
				if (pReplay->bValid == 1)
				{
					if (m_pFrReplayModeDlg_448 == NULL)
					{
						m_pFrReplayModeDlg_448 = CreateForm<FrReplayModeDlg>(
							g_pFresh->GetManager(), this, "replaymode", NULL);
						Doc()->SetFileName(pReplay->fileName);
						m_pFrReplayModeDlg_448->SetReplayMode(1);
						m_pFrReplayModeDlg_448->Open(
							(FRESH_PFN_RESULT)&CRealMyRoomMain::
								OnReplayModeOpenDlgResult,
							1);
					}
				}
				else
				{
					(IActor*)this << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xb9\xf6\xc1\xaf\xc0\xcc \xb3\xb7\xbe\xc6\xbc\xad \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0);
				}
			}
			sRecordedItemInfo* pReplay = (sRecordedItemInfo*)pItem->pData;
			if ((m_WRect_AC0 + pItem->pos).IsInRect(mouse) &&
				m_pFrReplayModeDlg_448 == NULL)
			{
				m_pFrReplayModeDlg_448 = CreateForm<FrReplayModeDlg>(
					g_pFresh->GetManager(), this, "replaytape_del", NULL);
				Doc()->SetFileName(pReplay->fileName);
				m_pFrReplayModeDlg_448->SetReplayMode(2);
				m_pFrReplayModeDlg_448->Open(
					(FRESH_PFN_RESULT)&CRealMyRoomMain::
						OnReplayDelOpenDlgResult,
					1);
			}
		}
		break;
	case 0x18:
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		if (pInfo != NULL)
		{
			if ((m_WRect_9E0 + pItem->pos).IsInRect(mouse) &&
				pInfo->tid == 0x1a000042)
				OpenTikiReport(pInfo);
		}
		break;
	}
	case 0x1b:
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			sCards* pCard = (sCards*)pItem->pData;
			m_fba0 = pCard->tid;
			if ((m_WRect_ADC + pItem->pos).IsInRect(mouse))
			{
				IFF_STRUCT::sCard* pInfo = ItemManager()->FindCard(pCard->tid);
				if (pInfo == NULL)
				{
					(IActor*)this << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc0\xaf\xc8\xbf\xc7\xd1 \xc4\xab\xb5\xe5\xb0\xa1 \xbe\xc6\xb4\xcf\xb9\xc7\xb7\xce \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0);
				}
				else
				{
					if (pInfo->c.Final)
					{
						unsigned int type = (pCard->tid >> 0x16) & 0xf;
						if (type == 3)
						{
							m_pFrOpenCardPackDlg_454 =
								CreateForm<FrOpenCardPackDlg>(
									g_pFresh->GetManager(), this,
									"cardpack_open", NULL);
							m_pFrOpenCardPackDlg_454->m_guid = pCard->uid;
							m_pFrOpenCardPackDlg_454->m_tid = pCard->tid;
							m_pFrOpenCardPackDlg_454->Open(
								(FRESH_PFN_RESULT)&CRealMyRoomMain::
									OnCardPackDlgResult,
								1);
							break;
						}
						char bUsed = 0;
						if (type == 2)
						{
							sAttachCardNode* pHead =
								*(sAttachCardNode**)&Doc()->m_cardAbilityList[0];
							sAttachCardNode* pNode = pHead->pNext;
							if (pNode != pHead)
							{
								do
								{
									if (*(int*)((char*)pNode + 0x44) == 2 &&
										*(unsigned long*)((char*)pNode +
											0x18) ==
											*(unsigned short*)((char*)ItemManager()
																   ->FindCard(
																	   pCard
																		   ->tid) +
												0xc4))
									{
										(IActor*)this << MsgObject(NULL, M983(0x23),
											(int)K2L_Compatibility("\xc0\xcc \xc4\xab\xb5\xe5\xc0\xc7 \xc8\xbf\xb0\xfa\xb4\xc2 \xc0\xcc\xb9\xcc \xc0\xfb\xbf\xeb\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
											0, 0, 0, 0);
										bUsed = 1;
									}
									pNode = pNode->pNext;
								} while (pNode != pHead);
								if (bUsed != 0)
									break;
							}
						}
						m_pFrCardUseDlg_44C = CreateForm<FrCardUseDlg>(
							g_pFresh->GetManager(), this, "usecard", NULL);
						m_pFrCardUseDlg_44C->m_guid = pCard->uid;
						m_pFrCardUseDlg_44C->m_tid = pCard->tid;
						m_pFrCardUseDlg_44C->Open(
							(FRESH_PFN_RESULT)&CRealMyRoomMain::OnCardDlgResult,
							1);
					}
					else
					{
						(IActor*)this << MsgObject(NULL, M983(0x23),
							(int)K2L_Compatibility("\xc0\xaf\xc8\xbf\xc7\xd1 \xc4\xab\xb5\xe5\xb0\xa1 \xbe\xc6\xb4\xcf\xb9\xc7\xb7\xce \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
							0, 0, 0, 0);
					}
				}
			}
		}
		break;
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
		if (IsLocalContent(S4_REAL_MYROOM) && pItem->pData != NULL)
		{
			if ((m_WRect_AF8 + pItem->pos).IsInRect(mouse))
			{
				if (m_b_D57 == false)
					ControlUiHideMode(4);
				else
					ControlUiHideMode(2);
			}
		}
	}
	sItemInfo* pInfo;
	if (IsLocalContent(S4_UCC) && (m_n_77C == 0x15 || m_n_77C == 0x1a) &&
		(pInfo = (sItemInfo*)pItem->pData) != NULL)
	{
		{
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tid);
			if (pPart != NULL)
			{
				if ((pPart->Category == 7 || pPart->Category == 8) &&
					(pInfo->status & 1) == 0)
				{
					if ((m_WRect_C38 + pItem->pos).IsInRect(mouse) &&
						IsUccClothes(pInfo->tid) && m_pFrUccDrawDlg_45C == NULL)
					{
						CIconToolTip::GetInstance()->ClearToolTip();
						m_pFrUccDrawDlg_45C = CreateForm<FrUccDrawDlg>(
							g_pFresh->GetManager(), this, "ucc_draw", NULL);
						m_pFrUccDrawDlg_45C->SetUccDrawInfo((eDrawMode)0, pInfo,
							false);
						if (!m_pFrUccDrawDlg_45C->IsInitialized())
						{
							(IActor*)this << MsgObject(NULL, M983(0x23),
								(int)MakeStr(
									K2L_Compatibility("\xbc\xad\xb9\xf6\xb7\xce\xba\xce\xc5\xcd \xc7\xca\xbf\xe4\xc7\xd1 \xc1\xa4\xba\xb8\xb8\xa6 \xbe\xf2\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9(\xc4\xda\xb5\xe5:%d)"),
									2),
								0, 0, 0, 0);
						}
						else
						{
							WPoint pos(373.0f, 72.0f);
							m_pFrUccDrawDlg_45C->Open(
								(FRESH_PFN_RESULT)&CRealMyRoomMain::
									OnUccDrawDlgResult,
								pos, 1);
							m_pFrUccDrawDlg_45C->SetMyRoom983(this); // 983 0x621d21
							ShowPet(Doc()->GetMyPartTidList().m_charTid);
							m_pFrListBox_304->SetVisible(false);
						}
					}
				}
				if ((pPart->Category == 7 || pPart->Category == 9) &&
					(pInfo->status & 1) == 0)
				{
					if (((pPart->Category != 7 ? m_WRect_C38 : m_WRect_C28) +
							pItem->pos)
							.IsInRect(mouse) &&
						IsUccClothes(pInfo->tid) && m_pFrUccCopyDlg_460 == NULL)
					{
						CIconToolTip::GetInstance()->ClearToolTip();
						m_pFrListBox_304->SetVisible(false);
						m_pFrUccCopyDlg_460 = CreateForm<FrUccCopyDlg>(
							g_pFresh->GetManager(), this, "ucc_copy", NULL);
						m_pFrUccCopyDlg_460->SetItem(pInfo);
						WPoint pos(359.0f, 71.0f);
						m_pFrUccCopyDlg_460->Open(
							(FRESH_PFN_RESULT)&CRealMyRoomMain::
								OnUccCopyDlgResult,
							pos, 1);
					}
				}
			}
		}
		if (m_pFrListBox_304 != NULL)
		{
			if ((pInfo->status & 1) == 0 && IsUccClothes(pInfo->tid))
			{
unselect:
				m_pFrListBox_304->UnselectItem(pItem);
				return;
			}
			if (IsUccClothes(pInfo->tid))
			{
				if (!UccManager()->IsDownloadComplete(pInfo->tid,
						pInfo->UccIndex))
					goto unselect;
			}
		}
	}
	if (pItem->selected != false)
	{
		SelectItemList(pItem);
		return;
	}
	UnselectItemList(pItem);
}

void CRealMyRoomMain::OnItemListMouseMove()
{
	if (m_pFrListBox_304->GetItemUnderCursor() == NULL)
		return;
	int bBusy = m_b_D98 ? m_hand.IsWorking(0)
						: (m_hand.IsWorking(0) || m_n_77C == 0x16);
	if (bBusy)
		return;
	if (IsLocalContent(S4_ITEM_STORAGE) == true && CItemStorage::Instance())
		CItemStorage::Instance()->SetDrag(sStoredItemDragDrop::MODE_PUSH);
}

void CRealMyRoomMain::OnItemListRBtnDown()
{
	if (m_pFrListBox_304 == NULL)
		return;
	FrListItem* pItem = m_pFrListBox_304->GetItemUnderCursor();
	if (pItem == NULL)
		return;
	if (m_hand.IsWorking(0) ||
		(m_roomMode == 0 &&
			(CRealMyRoom_AvatarManager::Instance()->m_pAvatar == NULL ||
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->m_pPetFrame ==
					NULL)))
		goto toggle;
	if (m_b_D98)
	{
open:
		m_pFrListBox_304->ToggleItem(pItem);
		OpenInformation(pItem);
		return;
	}
	switch (m_n_77C)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 0x15:
	case 0x1a:
		if (pItem->pData == NULL)
			return;
		{
			sItemInfo* pInfo = (sItemInfo*)pItem->pData;
			if (pInfo->ItemType == 6 && pInfo->Expired)
				goto open;
		}
	}
	if (m_n_77C == 9)
	{
		if (pItem->pData == NULL)
			return;
		if (((sItemInfo*)pItem->pData)->tid != 0x1a000004)
			OpenInformation(pItem);
		else
		{
			if (m_pFrFortuneDlg_440 == NULL)
			{
				IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(0x1a000004);
				m_pFrFortuneDlg_440 = CreateForm<FrFortuneDlg>(
					g_pFresh->GetManager(), this, "fortune", NULL);
				m_pFrFortuneDlg_440->SetMessage(pDesc ? pDesc->Desc : NULL, 0);
				m_pFrFortuneDlg_440->Open(
					(FRESH_PFN_RESULT)&CRealMyRoomMain::OnFortuneDlgResult, 1);
			}
		}
	}
	else
		OpenInformation(pItem);
	if (m_n_77C == 0x16)
		goto toggle;
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		if (IsUccClothes(pInfo->tid))
		{
			if ((pInfo->status & 1) == 0)
				goto toggle;
			if (!UccManager()->IsDownloadComplete(pInfo->tid, pInfo->UccIndex))
				goto toggle;
		}
	}
	if (pItem->selected)
	{
		SelectItemList(pItem);
		return;
	}
toggle:
	m_pFrListBox_304->ToggleItem(pItem);
}

void CRealMyRoomMain::OnItemListDClick()
{
	if (IsLocalContent(S4_ITEM_STORAGE) && m_b_D98)
	{
		if (IsLocalContent(S4_ITEM_STORAGE) == true && m_b_D98 == true &&
			CItemStorage::Instance() && !CItemStorage::Instance()->IsLock() &&
			m_pFrListBox_304->GetItemUnderCursor())
		{
			CItemStorage::Instance()->ReqPushStoredItemInfo(
				*m_pFrListBox_304->GetItemUnderCursor());
		}
	}
	else
	{
		FrListItem* pItem = m_pFrListBox_304->GetItemUnderCursor();
		if (pItem && m_n_77C == 8)
			EquipItem((sItemInfo*)pItem->pData);
	}
}

void CRealMyRoomMain::OnPowerEditInit(int param)
{
	m_pFrStatic_340 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CRealMyRoomMain::OnControlEditInit(int param)
{
	m_pFrStatic_344 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CRealMyRoomMain::OnImpactEditInit(int param)
{
	m_pFrStatic_348 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CRealMyRoomMain::OnSpinEditInit(int param)
{
	m_pFrStatic_34C = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CRealMyRoomMain::OnCurveEditInit(int param)
{
	m_pFrStatic_350 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CRealMyRoomMain::OnUpgradeInit(int param)
{
	m_pFrButton_390 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pFrButton_390->SetVisible(false);
	m_pFrButton_390->EnableHover(true, 0.0f);
}

void CRealMyRoomMain::OnUpgradeUp()
{
	IFF_STRUCT::sClubSet* pClubSet =
		Doc()->m_itemManager.FindClubSet(m_tidClub);
	if (pClubSet)
	{
		if ((pClubSet->c.Level > Doc()->m_myInfo.stat.Level &&
				!pClubSet->c.IsUnderLvl) ||
			(pClubSet->c.Level < Doc()->m_myInfo.stat.Level &&
				pClubSet->c.IsUnderLvl))
		{
			HandleMsg(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xca\xbd\xc3\xbf\xc0."),
				0, 0, 0, 0));
			return;
		}
	}
	m_pFrDowngradeDlg_42C = CreateForm<FrDowngradeDlg>(g_pFresh->GetManager(),
		this, "downgrade", NULL);
	if (m_roomMode == 0)
		m_pFrDowngradeDlg_42C->SetInfo(m_tidChar, m_pCharStat, m_pParts,
			m_pAuxParts, 0);
	else if (m_roomMode == 1)
		m_pFrDowngradeDlg_42C->SetInfo(Doc()->m_myInfo.userEquip.guidClubSet,
			0);
	m_pFrDowngradeDlg_42C->Open(
		(FRESH_PFN_RESULT)&CRealMyRoomMain::OnUpgradeDlgResult, 3);
	OnTooltipHoverOff(0);
}

void CRealMyRoomMain::OnDowngradeInit(int param)
{
	m_pFrButton_394 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pFrButton_394->SetVisible(false);
	m_pFrButton_394->EnableHover(true, 0.0f);
}

void CRealMyRoomMain::OnDowngradeUp()
{
	IFF_STRUCT::sClubSet* pClubSet =
		Doc()->m_itemManager.FindClubSet(m_tidClub);
	if (pClubSet)
	{
		if ((pClubSet->c.Level > Doc()->m_myInfo.stat.Level &&
				!pClubSet->c.IsUnderLvl) ||
			(pClubSet->c.Level < Doc()->m_myInfo.stat.Level &&
				pClubSet->c.IsUnderLvl))
		{
			HandleMsg(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xca\xbd\xc3\xbf\xc0."),
				0, 0, 0, 0));
			return;
		}
	}
	m_pFrDowngradeDlg_42C = CreateForm<FrDowngradeDlg>(g_pFresh->GetManager(),
		this, "downgrade", NULL);
	if (m_roomMode == 0)
		m_pFrDowngradeDlg_42C->SetInfo(m_tidChar, m_pCharStat, m_pParts,
			m_pAuxParts, 1);
	else if (m_roomMode == 1)
		m_pFrDowngradeDlg_42C->SetInfo(Doc()->m_myInfo.userEquip.guidClubSet,
			1);
	m_pFrDowngradeDlg_42C->Open(
		(FRESH_PFN_RESULT)&CRealMyRoomMain::OnUpgradeDlgResult, 3);
	OnTooltipHoverOff(0);
}

void CRealMyRoomMain::OnTooltipHoverOn(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pButton == m_pFrButton_390)
		CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_UPGRADE);
	else if (pButton == m_pFrButton_394)
		CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_DOWNGRADE);
	else if (pButton == m_pFrButton_388)
		CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_EQUIPCANCEL);
	else if (pButton == m_pFrButton_38C)
		CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_EQUIPRESET);
}

void CRealMyRoomMain::OnTooltipHoverOff(int param)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	int nTip = CIconToolTip::GetInstance()->GetCurTipIndex();
	bool bClear = false;
	switch (nTip)
	{
	case TOOLTIP_UPGRADE:
		if (pButton == m_pFrButton_390)
			bClear = true;
		break;
	case TOOLTIP_DOWNGRADE:
		if (pButton == m_pFrButton_394)
			bClear = true;
		break;
	case TOOLTIP_EQUIPCANCEL:
		if (pButton == m_pFrButton_388)
			bClear = true;
		break;
	case TOOLTIP_EQUIPRESET:
		if (pButton == m_pFrButton_38C)
			bClear = true;
		break;
	}
	if (pButton == NULL || bClear)
		CIconToolTip::GetInstance()->ClearToolTip();
}

void CRealMyRoomMain::GetTotalStatus(unsigned char* const level,
	unsigned char* const capacity, unsigned char* const penalty)
{
	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(0xff);
		CCardManager::Instance()->CalcCardPeriodAndStatus();
	}
	// D: All Stats pelo 983 (005e3cc0): inclui mascote e PCL da conta (o 645 omitia: Curve 7/7 x 8/9 do original)
	CStatusHelper helper;
	unsigned long tidMascot = 0;
	std::map<unsigned int, sMascotInfo>::iterator itMascot = Doc()->m_mascotMap.find(Doc()->m_myInfo.userEquip.guidMascot);
	if (itMascot != Doc()->m_mascotMap.end())
		tidMascot = itMascot->second.tid;
	sCharacterInfo* pChar = Doc()->FindChar983(0xffffffff);
	for (int i = 0; i < 5; i++)
	{
		int cur = helper.GetStatusOf983(i, m_tidChar, m_pParts, m_pAuxParts, m_tidClub, m_tidCaddie, tidMascot);
		int pen = helper.GetPenaltyOf983(i, m_tidChar, m_pParts, m_pAuxParts, m_tidClub, m_tidCaddie, tidMascot);
		level[i] = (unsigned char)cur;
		penalty[i] = (unsigned char)pen;
		capacity[i] = (unsigned char)(cur + helper.GetFreeSlotOf983(i, m_pParts, m_tidChar, pChar ? (int)pChar->mastery : 0));
	}
}

void CRealMyRoomMain::GetCharacterStatus(unsigned char* const level,
	unsigned char* const capacity)
{
	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(0xff);
		CCardManager::Instance()->CalcCardPeriodAndStatus();
	}
	if (ItemManager()->FindChar(m_tidChar) == NULL)
		return;
	for (int i = 0; i < 5; i++)
	{
		unsigned char item =
			ItemManager()->GetItemLevel((eLvlType)i, m_pParts, m_pAuxParts);
		capacity[i] = item +
			ItemManager()->GetCharCapacity((eLvlType)i, m_tidChar, m_pParts,
				m_pAuxParts, Doc()->m_myInfo.stat.Level);
		level[i] = item +
			ItemManager()->GetCharLevel((eLvlType)i, m_tidChar, m_pCharStat);
	}
}

void CRealMyRoomMain::GetCaddieStatus(unsigned char* const stat,
	unsigned char* const)
{
	IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(m_tidCaddie);
	if (pCaddie)
	{
		stat[0] += (char)pCaddie->Attr[0];
		stat[1] += (char)pCaddie->Attr[1];
		stat[2] += (char)pCaddie->Attr[2];
		stat[3] += (char)pCaddie->Attr[3];
		stat[4] += (char)pCaddie->Attr[4];
	}
}

void CRealMyRoomMain::GetClubSetStatus(unsigned char* const stat,
	unsigned char* const slot)
{
	std::map<unsigned int, sItemInfo>::iterator it =
		Doc()->m_clubSetMap.find(m_n_764);
	if (it != Doc()->m_clubSetMap.end())
	{
		stat[0] = (unsigned char)(*it).second.Common[0];
		stat[1] = (unsigned char)(*it).second.Common[1];
		stat[2] = (unsigned char)(*it).second.Common[2];
		stat[3] = (unsigned char)(*it).second.Common[3];
		stat[4] = (unsigned char)(*it).second.Common[4];
	}
	IFF_STRUCT::sClubSet* pClubSet = ItemManager()->FindClubSet(m_tidClub);
	if (pClubSet)
	{
		for (int i = 0; i < 5; i++)
		{
			slot[i] += (char)pClubSet->Slot[i];
			stat[i] += (char)pClubSet->Attr[i];
		}
	}
}

void CRealMyRoomMain::SetLevelBar()
{
	unsigned char status[5] = { 0 };
	unsigned char addStatus[5] = { 0 };
	unsigned char etc[5] = { 0 };
	FrGaugeBar** ppBar = &m_pFrGaugeBar_32C;
	int i = 5;
	do
	{
		if (*ppBar)
		{
			(*ppBar)->SetDestPos(0, false);
			(*ppBar)->SetExpand(0);
		}
		ppBar++;
		i--;
	} while (i != 0);
	switch (m_n_774)
	{
	case 0:
		switch (m_n_77C)
		{
		case 1:
		case 2:
		case 3:
		case 4:
		case 0x15:
		case 0x16:
		case 0x1a:
			GetCharacterStatus(status, addStatus);
			m_n_B74 = 0;
			break;
		case 6:
			GetClubSetStatus(status, addStatus);
			m_n_B74 = 1;
			break;
		case 10:
			GetCaddieStatus(status, addStatus);
			m_n_B74 = 2;
			break;
		}
		break;
	case 1:
		GetTotalStatus(status, addStatus, etc);
		break;
	}
	i = 0;
	ppBar = &m_pFrGaugeBar_32C;
	do
	{
		if (*ppBar)
		{
			unsigned char base = status[i];
			(*ppBar)->SetDestPos(base, false);
			(*ppBar)->SetExpand(addStatus[i] - base);
		}
		FrStatic* pStatic = *(FrStatic**)(ppBar + 5);
		if (pStatic)
			pStatic->SetCaption11(
				MakeStatusString3(status[i], addStatus[i], etc[i]));
		i++;
		ppBar++;
	} while (i < 5);
}

// A422: sPart do 983 (0x238) = 645 + 0x30 antes destes campos; o codigo lia 0x1b4/0x1be/0x1f8/0x1fa/0x200 (645)
C_ASSERT(offsetof(IFF_STRUCT::sPart, Attr) == 0x1e4 && offsetof(IFF_STRUCT::sPart, Slot) == 0x1ee);
C_ASSERT(offsetof(IFF_STRUCT::sPart, CharacterSlot) == 0x228 && offsetof(IFF_STRUCT::sPart, CaddieSlot) == 0x22a);
C_ASSERT(offsetof(IFF_STRUCT::sPart, Beginner) == 0x234);
void CRealMyRoomMain::BuildDisplayStand(subTab_t subTab)
{
	typedef std::list<FrListItem*>::iterator ListIter;
	typedef std::list<sItemInfo>::iterator ItemIter;
	typedef std::map<unsigned int, sCharacterInfo>::iterator CharIter;

	if (m_pFrListBox_304 == NULL)
		return;
	m_pFrListBox_304->ClearItem();
	switch (subTab)
	{
	case 1:
	case 2:
	case 3:
	{
		unsigned long flag = CTaskMain::GetCategoryFlag(subTab);
		for (ItemIter it = Doc()->m_partsList.begin();
			it != Doc()->m_partsList.end(); ++it)
		{
			if (S5::UnderCategory(m_tidChar, (*it).tid, flag))
			{
				IFF_STRUCT::sPart* pPart = ItemManager()->FindPart((*it).tid);
				if (pPart != NULL)
				{
					(*it).CharacterSlotNum =
						(unsigned short)pPart->CharacterSlot;
					(*it).CaddieSlotNum =
						(unsigned short)pPart->CaddieSlot;
				}
				m_pFrListBox_304->AddItem(&(*it));
			}
		}
		unsigned long* pUids = (unsigned long*)&m_partUid;
		for (int i = 0; i < 24; i++)
		{
			ListIter li = m_pFrListBox_304->m_itemList.begin();
			if (li != m_pFrListBox_304->m_itemList.end())
			{
				unsigned long guidChar = Doc()->m_myInfo.userEquip.guidChar;
				CharIter ciEnd = Doc()->m_charMap.end();
				do
				{
					FrListItem* pItem = *li;
					unsigned long* pData = (unsigned long*)pItem->pData;
					CharIter ci = Doc()->m_charMap.find(guidChar);
					if (ci != ciEnd)
					{
						unsigned long tid = pData[1];
						bool bMatch;
						if ((*ci).second.ItemIdList[(tid >> 13) & 0x1f] == 0)
							bMatch = (tid == m_pParts[i]);
						else
						{
							if (tid != m_pParts[i])
								goto next1;
							bMatch = (pData[0] == pUids[i]);
						}
						if (bMatch)
						{
							m_pFrListBox_304->SelectItem(pItem, false);
							break;
						}
					}
next1:
					++li;
				} while (li != m_pFrListBox_304->m_itemList.end());
			}
		}
		break;
	}
	case 4:
	{
		unsigned long flag = CTaskMain::GetCategoryFlag(subTab);
		for (ItemIter it = Doc()->m_partsList.begin();
			it != Doc()->m_partsList.end(); ++it)
		{
			if (S5::UnderCategory(m_tidChar, (*it).tid, flag) &&
				(!IsAngelWing((*it).tid) ||
					(IsMannerPlayer() && 0x14 < Doc()->m_myInfo.stat.Level)))
			{
				m_pFrListBox_304->AddItem(&(*it));
			}
		}
		unsigned long* pUids = (unsigned long*)&m_partUid;
		for (int i = 0; i < 24; i++)
		{
			ListIter li = m_pFrListBox_304->m_itemList.begin();
			if (li != m_pFrListBox_304->m_itemList.end())
			{
				unsigned long guidChar = Doc()->m_myInfo.userEquip.guidChar;
				CharIter ciEnd = Doc()->m_charMap.end();
				do
				{
					FrListItem* pItem = *li;
					unsigned long* pData = (unsigned long*)pItem->pData;
					CharIter ci = Doc()->m_charMap.find(guidChar);
					if (ci != ciEnd)
					{
						unsigned long tid = pData[1];
						bool bMatch;
						if ((*ci).second.ItemIdList[(tid >> 13) & 0x1f] == 0)
							bMatch = (tid == m_pParts[i]);
						else
						{
							if (tid != m_pParts[i])
								goto next4;
							bMatch = (pData[0] == pUids[i]);
						}
						if (bMatch)
						{
							IFF_STRUCT::sPart* pPart =
								ItemManager()->FindPart(pData[1]);
							if (pPart != NULL)
							{
								((sItemInfo*)pData)->CharacterSlotNum =
									(unsigned short)pPart->CharacterSlot;
								((sItemInfo*)pData)->CaddieSlotNum =
									(unsigned short)pPart->CaddieSlot;
							}
							m_pFrListBox_304->SelectItem(pItem, false);
							break;
						}
					}
next4:
					++li;
				} while (li != m_pFrListBox_304->m_itemList.end());
			}
		}
		for (ItemIter it = Doc()->m_auxPartList.begin();
			it != Doc()->m_auxPartList.end(); ++it)
		{
			m_pFrListBox_304->AddItem(&(*it));
		}
		for (int i = 0; i < 5; i++)
		{
			ListIter li = m_pFrListBox_304->m_itemList.begin();
			if (li != m_pFrListBox_304->m_itemList.end())
			{
				do
				{
					if (((sItemInfo*)(*li)->pData)->tid == m_pAuxParts[i])
					{
						m_pFrListBox_304->SelectItem(*li, false);
						break;
					}
					++li;
				} while (li != m_pFrListBox_304->m_itemList.end());
			}
		}
		break;
	}
	case 5:
	case 0xb:
	case 0x19:
		break;
	case 6:
	{
		std::map<unsigned int, sItemInfo>::const_iterator it =
			Doc()->m_clubSetMap.begin();
		if (it != Doc()->m_clubSetMap.end())
		{
			do
			{
				const sItemInfo& info = (*it).second;
				FrListItem* pItem = m_pFrListBox_304->AddItem((void*)&info);
				if (m_n_764 == 0)
				{
					if (Doc()->m_myInfo.userEquip.guidClubSet == info.guid)
					{
						m_n_764 = info.guid;
						m_pFrListBox_304->SelectItem(pItem, true);
					}
				}
				else if (m_n_764 == (int)info.guid)
					m_pFrListBox_304->SelectItem(pItem, true);
				++it;
			} while (it != Doc()->m_clubSetMap.end());
		}
		break;
	}
	case 7:
		for (ItemIter it = Doc()->m_ballList.begin();
			it != Doc()->m_ballList.end(); ++it)
		{
			FrListItem* pItem = m_pFrListBox_304->AddItem(&(*it));
			if (Doc()->m_myInfo.userEquip.tidBall == (*it).tid)
				m_pFrListBox_304->SelectItem(pItem, true);
		}
		break;
	case 8:
	{
		for (ItemIter it = Doc()->m_myItemList.begin();
			it != Doc()->m_myItemList.end(); ++it)
		{
			if (ItemManager()->GetCouponKind((*it).tid) == 0)
			{
				unsigned int tid = (*it).tid;
				if ((tid & 0x2000000) == 0)
				{
					unsigned int nEquipped =
						(tid == Doc()->m_myInfo.userEquip.tidItemSlot[0]);
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[1])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[2])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[3])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[4])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[5])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[6])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[7])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[8])
						nEquipped++;
					if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[9])
						nEquipped++;
					unsigned int nCount = (*it).Common[0];
					if (nCount != nEquipped && -1 < (int)(nCount - nEquipped))
					{
						FrListItem* pItem = m_pFrListBox_304->AddItem(&(*it));
						pItem->AddNoRBtnIdx(1);
						IFF_ITEM_COMMON* pCommon =
							ItemManager()->FindCommonItem((*it).tid);
						if (pCommon != NULL && !pCommon->IsCash)
							pItem->AddNoRBtnIdx(2);
					}
				}
			}
		}
		if (m_pFrListBox_37C != NULL)
		{
			m_pFrListBox_37C->ClearItem();
			int nSlot = Doc()->GetNumAddItemSlotByMascot() + 8;
			int nAdd = nSlot;
			if (IsLocalContent(S4_NT_SC_ITEMSLOT))
			{
				if (IsLocalContent(S4_CARD_SYSTEM))
				{
					CCardManager::Instance()->SetPlayerIndex(0xff);
					CCardManager::Instance()->CalcCardPeriodAndStatus();
					int nCardSlot =
						CCardManager::Instance()->GetCardPeriodSlot();
					if (0 < nCardSlot)
						nAdd = nSlot + nCardSlot;
				}
				for (int i = 0; i < 10; i++)
				{
					if (i < nAdd)
						m_pFrListBox_37C->AddItem(
							&Doc()->m_myInfo.userEquip.tidItemSlot[i]);
					else
						UnequipItem(&Doc()->m_myInfo.userEquip.tidItemSlot[i]);
				}
				int nSize = m_pFrListBox_37C->GetCurrentItemSize(true);
				if (nSize < 10)
				{
					nSize = 10 - nSize;
					do
					{
						m_pFrListBox_37C->AddItem(NULL);
						nSize--;
					} while (nSize != 0);
				}
			}
			else
			{
				if (IsLocalContent(S4_CARD_SYSTEM))
				{
					CCardManager::Instance()->SetPlayerIndex(0xff);
					CCardManager::Instance()->CalcCardPeriodAndStatus();
					nSlot += CCardManager::Instance()->GetCardPeriodSlot();
					nAdd = nSlot;
				}
				if (0 < nSlot)
				{
					int i = 0;
					int n = nAdd;
					do
					{
						m_pFrListBox_37C->AddItem(
							&Doc()->m_myInfo.userEquip.tidItemSlot[i]);
						i++;
						n--;
					} while (n != 0);
				}
				int nSize = m_pFrListBox_37C->GetCurrentItemSize(true);
				if (nSize < 10)
				{
					nSize = 10 - nSize;
					do
					{
						m_pFrListBox_37C->AddItem(NULL);
						nSize--;
					} while (nSize != 0);
				}
			}
		}
		break;
	}
	case 9:
		for (ItemIter it = Doc()->m_myItemList.begin();
			it != Doc()->m_myItemList.end(); ++it)
		{
			if (ItemManager()->GetCouponKind((*it).tid) == 0 &&
				!ItemManager()->IsNonVisibleItem((*it).tid) &&
				((*it).Common[2] & 0x200) != 0)
			{
				FrListItem* pItem = m_pFrListBox_304->AddItem(&(*it));
				CCodeTinker tinker;
				bool bOpen = tinker.IsNeedOpenMiniButton((*it).tid);
				if (bOpen)
					pItem->AddNoRBtnIdx(0);
			}
		}
		break;
	case 10:
	{
		std::map<unsigned int, sCaddieInfo>::const_iterator it =
			Doc()->m_caddieMap.begin();
		if (it != Doc()->m_caddieMap.end())
		{
			do
			{
				const sCaddieInfo& info = (*it).second;
				FrListItem* pItem = m_pFrListBox_304->AddItem((void*)&info);
				IFF_STRUCT::sCaddie* pCaddie =
					ItemManager()->FindCaddie(info.tid);
				if ((*(unsigned long*)((char*)&info + 0xc) & 0x200) != 0 &&
					*(short*)((char*)&info + 0xe) == 0 &&
					*(unsigned long*)((char*)&info + 0x10) == 0 &&
					*(int*)((char*)pCaddie + 0x90) != 0)
				{
					pItem->AddNoRBtnIdx(3);
				}
				if (m_n_754 == 0)
				{
					if (Doc()->m_myInfo.userEquip.guidCaddie == info.guid)
					{
						m_n_754 = info.guid;
						m_pFrListBox_304->SelectItem(pItem, true);
					}
				}
				else if (m_n_754 == (int)info.guid)
					m_pFrListBox_304->SelectItem(pItem, true);
				++it;
			} while (it != Doc()->m_caddieMap.end());
		}
		break;
	}
	case 0xc:
	{
		std::map<unsigned int, sMascotInfo>::const_iterator it =
			Doc()->m_mascotMap.begin();
		if (it != Doc()->m_mascotMap.end())
		{
			do
			{
				const sMascotInfo& info = (*it).second;
				FrListItem* pItem = m_pFrListBox_304->AddItem((void*)&info);
				if (Doc()->m_myInfo.userEquip.guidMascot ==
					*(unsigned long*)&info)
					m_pFrListBox_304->SelectItem(pItem, true);
				++it;
			} while (it != Doc()->m_mascotMap.end());
		}
		break;
	}
	case 0xd:
	case 0xe:
	case 0xf:
	case 0x10:
	case 0x22:
	{
		unsigned long flag = CTaskMain::GetCategoryFlag(subTab);
		for (ItemIter it = Doc()->m_cutinList.begin();
			it != Doc()->m_cutinList.end(); ++it)
		{
			if (flag == 5 || ((*it).tid >> 0x16 & 0xf) == flag)
			{
				FrListItem* pItem = m_pFrListBox_304->AddItem(&(*it));
				eEquipSkin skin;
				if (S5::UTIL::GetEquipSkinIndex(skin, (*it).tid) != 0 &&
					Doc()->m_myInfo.userEquip.tidSkin[skin] == (*it).tid)
					m_pFrListBox_304->SelectItem(pItem, true);
			}
		}
		break;
	}
	case 0x11:
		if (!IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
		{
			if (Doc()->m_giftList.empty())
			{
				m_pFrEdit_3A0->ClearLine();
				m_pFrEdit_3A0->AddText(
					MakeStr(
						"\xb5\xb5\xc2\xf8\xc7\xd1 \xbc\xb1\xb9\xb0\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
						0),
					false, true);
			}
			else
			{
				std::list<sGiftInfo>::iterator it = Doc()->m_giftList.begin();
				if (it != Doc()->m_giftList.end())
				{
					do
					{
						FrListItem* pItem = m_pFrListBox_304->AddItem(&(*it));
						pItem->AddNoRBtnIdx(0);
						if (*((char*)&(*it) + 0xa) != '@')
							pItem->AddNoRBtnIdx(4);
						++it;
					} while (it != Doc()->m_giftList.end());
				}
			}
		}
		break;
	case 0x12:
	{
		m_trophyList.clear();
		for (int i = 0; i < 13; i++)
		{
			for (int j = 2; j >= 0; j--)
			{
				if (Doc()->m_myInfo.trophy.Trophy[i][j] != 0 ||
					Doc()->m_oldSeasonTrophy.Trophy[i][j] != 0)
				{
					sSpecialTrophy trophy;
					trophy.guid = 0;
					trophy.tid = ((i | 0x2c00) << 16) | j;
					trophy.count = Doc()->m_myInfo.trophy.Trophy[i][j] +
						Doc()->m_oldSeasonTrophy.Trophy[i][j];
					m_trophyList.push_back(trophy);
				}
			}
		}
		for (std::list<sSpecialTrophy>::iterator it =
				 Doc()->m_specialTrophyList.begin();
			it != Doc()->m_specialTrophyList.end(); ++it)
		{
			m_trophyList.push_back(*it);
		}
		for (std::list<sSpecialTrophy>::iterator it =
				 Doc()->m_oldSpecialTrophyList.begin();
			it != Doc()->m_oldSpecialTrophyList.end(); ++it)
		{
			m_trophyList.push_back(*it);
		}
		for (std::list<sSpecialTrophy>::reverse_iterator it =
				 m_trophyList.rbegin();
			it != m_trophyList.rend(); ++it)
		{
			m_pFrListBox_304->AddItem(&(*it));
		}
		break;
	}
	case 0x13:
		for (ItemIter it = Doc()->m_myItemList.begin();
			it != Doc()->m_myItemList.end(); ++it)
		{
			if (ItemManager()->GetCouponKind((*it).tid) != 0)
				m_pFrListBox_304->AddItem(&(*it));
		}
		break;
	case 0x14:
		for (std::list<sQuest>::iterator it = Doc()->m_questList.begin();
			it != Doc()->m_questList.end(); ++it)
		{
			m_pFrListBox_304->AddItem(&(*it));
		}
		break;
	case 0x15:
	{
		unsigned long flag = CTaskMain::GetCategoryFlag(subTab);
		for (ItemIter it = Doc()->m_partsList.begin();
			it != Doc()->m_partsList.end(); ++it)
		{
			if (S5::UnderCategory(m_tidChar, (*it).tid, flag))
			{
				if (IsLocalContent(S4_CARD_SYSTEM))
				{
					IFF_STRUCT::sPart* pPart =
						ItemManager()->FindPart((*it).tid);
					if (pPart != NULL)
					{
						(*it).CharacterSlotNum =
							(unsigned short)pPart->CharacterSlot;
						(*it).CaddieSlotNum =
							(unsigned short)pPart->CaddieSlot;
					}
				}
				m_pFrListBox_304->AddItem(&(*it));
			}
		}
		unsigned long* pUids = (unsigned long*)&m_partUid;
		for (int i = 0; i < 24; i++)
		{
			ListIter li = m_pFrListBox_304->m_itemList.begin();
			if (li != m_pFrListBox_304->m_itemList.end())
			{
				unsigned long guidChar = Doc()->m_myInfo.userEquip.guidChar;
				CharIter ciEnd = Doc()->m_charMap.end();
				do
				{
					FrListItem* pItem = *li;
					unsigned long* pData = (unsigned long*)pItem->pData;
					CharIter ci = Doc()->m_charMap.find(guidChar);
					if (ci != ciEnd)
					{
						unsigned long tid = pData[1];
						bool bMatch;
						if ((*ci).second.ItemIdList[(tid >> 13) & 0x1f] == 0)
							bMatch = (tid == m_pParts[i]);
						else
						{
							if (tid != m_pParts[i])
								goto next15;
							bMatch = (pData[0] == pUids[i]);
						}
						if (bMatch)
						{
							m_pFrListBox_304->SelectItem(pItem, false);
							break;
						}
					}
next15:
					++li;
				} while (li != m_pFrListBox_304->m_itemList.end());
			}
		}
		for (ItemIter it = Doc()->m_auxPartList.begin();
			it != Doc()->m_auxPartList.end(); ++it)
		{
			m_pFrListBox_304->AddItem(&(*it));
		}
		for (int i = 0; i < 5; i++)
		{
			ListIter li = m_pFrListBox_304->m_itemList.begin();
			if (li != m_pFrListBox_304->m_itemList.end())
			{
				do
				{
					FrListItem* pItem = *li;
					if (pItem != NULL && pItem->pData != NULL &&
						((sItemInfo*)pItem->pData)->tid == m_pAuxParts[i])
					{
						m_pFrListBox_304->SelectItem(pItem, false);
						break;
					}
					++li;
				} while (li != m_pFrListBox_304->m_itemList.end());
			}
		}
		if (IsLocalContent(S4_UCC))
		{
			for (ItemIter it = Doc()->m_uccAztecList.begin();
				it != Doc()->m_uccAztecList.end(); ++it)
			{
				if (S5::UnderCategory(m_tidChar, (*it).tid, flag))
				{
					if (IsLocalContent(S4_CARD_SYSTEM))
					{
						IFF_STRUCT::sPart* pPart =
							ItemManager()->FindPart((*it).tid);
						if (pPart != NULL)
						{
							(*it).CharacterSlotNum =
								(unsigned short)pPart->CharacterSlot;
							(*it).CaddieSlotNum =
								(unsigned short)pPart->CaddieSlot;
						}
					}
					m_pFrListBox_304->AddItem(&(*it));
				}
			}
			for (int i = 0; i < 24; i++)
			{
				ListIter li = m_pFrListBox_304->m_itemList.begin();
				if (li != m_pFrListBox_304->m_itemList.end())
				{
					do
					{
						FrListItem* pItem = *li;
						unsigned long* pData = (unsigned long*)pItem->pData;
						if (IsLocalContent(S4_CARD_SYSTEM))
						{
							CharIter ci = Doc()->m_charMap.find(
								Doc()->m_myInfo.userEquip.guidChar);
							if (ci != Doc()->m_charMap.end())
							{
								unsigned long tid = pData[1];
								if ((*ci).second.ItemIdList[(tid >> 13) &
										0x1f] == 0)
								{
									if (tid == m_pParts[i])
									{
										m_pFrListBox_304->SelectItem(pItem,
											false);
										break;
									}
								}
								else if (tid == m_pParts[i] &&
									pData[0] == pUids[i])
								{
									m_pFrListBox_304->SelectItem(pItem, false);
									break;
								}
							}
						}
						else if (pData[1] == m_pParts[i])
						{
							m_pFrListBox_304->SelectItem(pItem, false);
							break;
						}
						++li;
					} while (li != m_pFrListBox_304->m_itemList.end());
				}
			}
		}
		break;
	}
	case 0x16:
	{
		CTaskMain::GetCategoryFlag(subTab);
		for (ItemIter it = Doc()->m_partsList.begin();
			it != Doc()->m_partsList.end(); ++it)
		{
			if (((*it).tid & 0x600) == 0)
			{
				std::map<unsigned int, sCharacterInfo>::const_iterator ci =
					Doc()->m_charMap.begin();
				if (ci != Doc()->m_charMap.end())
				{
					unsigned int key = (*it).tid >> 0x12 & 0xff;
					do
					{
						if (key == ((*ci).second.tid & 0x3ffffff))
							break;
						++ci;
					} while (ci != Doc()->m_charMap.end());
					if (ci != Doc()->m_charMap.end())
						continue;
				}
				if (IsLocalContent(S4_CARD_SYSTEM))
				{
					IFF_STRUCT::sPart* pPart =
						ItemManager()->FindPart((*it).tid);
					if (pPart != NULL)
					{
						(*it).CharacterSlotNum =
							(unsigned short)pPart->CharacterSlot;
						(*it).CaddieSlotNum =
							(unsigned short)pPart->CaddieSlot;
					}
				}
				m_pFrListBox_304->AddItem(&(*it));
			}
		}
		if (IsLocalContent(S4_UCC))
		{
			ItemIter it = Doc()->m_uccAztecList.begin();
			if (it != Doc()->m_uccAztecList.end())
			{
				do
				{
					if (((*it).tid & 0x600) == 0)
					{
						std::map<unsigned int, sCharacterInfo>::const_iterator
							ci = Doc()->m_charMap.begin();
						if (ci != Doc()->m_charMap.end())
						{
							do
							{
								if ((((*it).tid >> 0x12) & 0xff) ==
									((*ci).second.tid & 0x3ffffff))
									break;
								++ci;
							} while (ci != Doc()->m_charMap.end());
							if (ci != Doc()->m_charMap.end())
								goto next16;
						}
						m_pFrListBox_304->AddItem(&(*it));
					}
next16:
					++it;
				} while (it != Doc()->m_uccAztecList.end());
			}
		}
		break;
	}
	case 0x17:
		if (IsLocalContent(S4_REPLAY_SYSTEM))
		{
			Doc()->ReplaySavFileCheck();
			for (std::list<sRecordedItemInfo>::iterator it =
					 Doc()->m_recordedItemList.begin();
				it != Doc()->m_recordedItemList.end(); ++it)
			{
				m_pFrListBox_304->AddItem(&(*it));
			}
		}
		break;
	case 0x18:
		if (IsLocalContent(S3_CADDIE_REPORT))
		{
			for (ItemIter it = Doc()->m_myItemList.begin();
				it != Doc()->m_myItemList.end(); ++it)
			{
				if (((*it).tid & 0x2000000) != 0 && (*it).tid == 0x1a000042)
				{
					FrListItem* pItem = m_pFrListBox_304->AddItem(&(*it));
					pItem->AddNoRBtnIdx(0);
				}
			}
		}
		break;
	case 0x1a:
	{
		unsigned long flag = CTaskMain::GetCategoryFlag(subTab);
		for (ItemIter it = Doc()->m_uccAztecList.begin();
			it != Doc()->m_uccAztecList.end(); ++it)
		{
			if (S5::UnderCategory(m_tidChar, (*it).tid, flag))
			{
				if (IsLocalContent(S4_CARD_SYSTEM))
				{
					IFF_STRUCT::sPart* pPart =
						ItemManager()->FindPart((*it).tid);
					if (pPart != NULL)
					{
						(*it).CharacterSlotNum =
							(unsigned short)pPart->CharacterSlot;
						(*it).CaddieSlotNum =
							(unsigned short)pPart->CaddieSlot;
					}
				}
				m_pFrListBox_304->AddItem(&(*it));
			}
		}
		unsigned long* pUids = (unsigned long*)&m_partUid;
		for (int i = 0; i < 24; i++)
		{
			ListIter li = m_pFrListBox_304->m_itemList.begin();
			if (li != m_pFrListBox_304->m_itemList.end())
			{
				do
				{
					FrListItem* pItem = *li;
					unsigned long* pData = (unsigned long*)pItem->pData;
					if (IsLocalContent(S4_CARD_SYSTEM))
					{
						CharIter ci = Doc()->m_charMap.find(
							Doc()->m_myInfo.userEquip.guidChar);
						if (ci != Doc()->m_charMap.end())
						{
							unsigned long tid = pData[1];
							if ((*ci).second.ItemIdList[(tid >> 13) & 0x1f] ==
								0)
							{
								if (tid == m_pParts[i])
								{
									m_pFrListBox_304->SelectItem(pItem, false);
									break;
								}
							}
							else if (tid == m_pParts[i] && pData[0] == pUids[i])
							{
								m_pFrListBox_304->SelectItem(pItem, false);
								break;
							}
						}
					}
					else if (pData[1] == m_pParts[i])
					{
						m_pFrListBox_304->SelectItem(pItem, false);
						break;
					}
					++li;
				} while (li != m_pFrListBox_304->m_itemList.end());
			}
		}
		break;
	}
	case 0x1b:
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			for (std::list<sCards>::iterator it = Doc()->m_cardList.begin();
				it != Doc()->m_cardList.end(); ++it)
			{
				m_pFrListBox_304->AddItem(&(*it));
			}
		}
		BuildCardPackList();
		if (IsLocalContent(S4_CARD_SYSTEM) && m_pCardBook != NULL &&
			m_pCardBook->IsShow())
		{
			m_pCardBook->ReBuildCurrentPage();
		}
		break;
	case 0x1c:
	case 0x1d:
	case 0x1e:
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			unsigned long flag = CTaskMain::GetCategoryFlag(subTab);
			for (std::list<sFurniture_List>::iterator it =
					 Doc()->m_furnitureList.begin();
				it != Doc()->m_furnitureList.end(); ++it)
			{
				if ((((*it).typeId >> 0xb) & 0x7ff) == flag)
					m_pFrListBox_304->AddItem(&(*it));
			}
		}
		break;
	case 0x1f:
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			for (std::list<sFurniture_List>::iterator it =
					 Doc()->m_furnitureList.begin();
				it != Doc()->m_furnitureList.end(); ++it)
			{
				unsigned int type = ((*it).typeId >> 0xb) & 0x7ff;
				if (type < 7 || 9 < type)
					m_pFrListBox_304->AddItem(&(*it));
			}
		}
		break;
	case 0x20:
		for (std::list<sFurniture_List>::iterator it =
				 Doc()->m_furnitureList.begin();
			it != Doc()->m_furnitureList.end(); ++it)
		{
			m_pFrListBox_304->AddItem(&(*it));
		}
		break;
	case 0x21:
		for (ItemIter it = Doc()->m_cutinList.begin();
			it != Doc()->m_cutinList.end(); ++it)
		{
			if (S5::ISCutinItem((*it).tid))
			{
				IFF_STRUCT::S5::sCutinInformation* pCutin =
					ItemManager()->FindCutinInformation((*it).tid);
				if (pCutin != NULL &&
					((m_tidChar & 0x3ffffff) ==
							*(unsigned long*)((char*)pCutin + 0x18) ||
						99 < (int)*(unsigned long*)((char*)pCutin + 0x18)))
				{
					FrListItem* pItem = m_pFrListBox_304->AddItem(&(*it));
					eEquipSkin skin;
					if (S5::UTIL::GetEquipSkinIndex(skin, (*it).tid) != 0 &&
						Doc()->m_myInfo.userEquip.tidSkin[skin] == (*it).tid)
						m_pFrListBox_304->SelectItem(pItem, true);
				}
			}
		}
		break;
	default:
		break;
	}
	m_pFrListBox_304->ResetTopRow();
	int nSize = m_pFrListBox_304->GetCurrentItemSize(true);
	if (nSize < 8)
	{
		nSize = 8 - nSize;
		do
		{
			m_pFrListBox_304->AddItem(NULL);
			nSize--;
		} while (nSize != 0);
	}
	if (subTab != 0x12)
		m_pFrListBox_304->SortItem(EmptyCompares);
}

void CRealMyRoomMain::ShowMainModel()
{
	SetLevelBar();
	if (m_pExhibition)
	{
		m_pExhibition->ResetCameraRecoverTime();
		m_pExhibition->AttachFx(); // 983: sem ParseBoundingBoxScript
	}
	m_hand.Enter(); // m_hand.m_csUser (era this+0x810 = layout do 645)
	if (m_pExhibition)
		m_pExhibition->GetPetFrame()->Refresh(
			&CSharedDoc::Instance()->GetMyPartTidList());
	if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar)
	{
		CRealMyRoom_AvatarManager::Instance()->m_pAvatar->ChangeCharacterInfo();
		CPetFrame* pPet =
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->m_pPetFrame;
		if (pPet)
			pPet->Refresh(&CSharedDoc::Instance()->GetMyPartTidList());
	}
	m_hand.Leave();
}

void CRealMyRoomMain::DrawSelectItemList(FrListItem* pItem)
{
	if (pItem->pData != NULL)
	{
		IFF_STRUCT::sDesc* pDesc =
			ItemManager()->FindDesc(((unsigned long*)pItem->pData)[1]);
		m_pFrEdit_368->SetVisible(true);
		m_f_6A0 = pItem->pos.x;
		m_f_6A4 = pItem->pos.y;
		m_f_798 = pItem->pos.x;
		m_f_79C = pItem->pos.y;
		m_pFrEdit_368->ClearLine();
		if (pDesc)
		{
			m_pFrEdit_368->AddText(pDesc->Desc, false, true);
			int nLine = m_pFrEdit_368->GetLineNum();
			FrEdit* pEdit = m_pFrEdit_368;
			WRect rect;
			float h = (float)pEdit->GetLineHeight() * (float)nLine;
			WRect cur = pEdit->GetRect();
			rect.x = m_WPoint_788.x + m_f_798;
			rect.y = (m_f_79C + m_WPoint_788.y) - h;
			rect.w = cur.w;
			rect.h = h;
			pEdit->SetRect(rect);
			m_pFrEdit_368->GetScrollBar()->ScrollToFirst();
			return;
		}
		m_pFrEdit_368->SetVisible(false);
		m_pFrViewer_364->SetVisible(false);
		return;
	}
	m_pFrEdit_368->SetVisible(false);
}

bool CRealMyRoomMain::SelectItemList(unsigned long id)
{
	if (!m_pFrListBox_304)
		return false;
	FrListBox::ITEM_LIST& list = m_pFrListBox_304->m_itemList;
	FrListBox::ITEM_LIST::iterator it = list.begin();
	if (it != list.end())
	{
		while (
			!(*it)->pData || *(unsigned long*)((char*)(*it)->pData + 4) != id)
		{
			++it;
			if (it == list.end())
				return false;
		}
		if (*it)
		{
			SelectItemList(*it);
			return true;
		}
	}
	return false;
}

bool CRealMyRoomMain::SelectItemList(FrListItem* pItem)
{
	const char* pszMsg;
	switch (m_n_77C)
	{
	case 4:
	case 21:
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		g_audio->PlaySfx("item_select_cloth");
		if ((pInfo->tid & 0xfc000000) == 0x70000000)
		{
			unsigned int index = (pInfo->tid >> 21) & 0x1f;
			unsigned long old = m_pAuxParts[index];
			m_pAuxParts[index] = pInfo->tid;
			if (m_pExhibition)
				m_pExhibition->GetPetFrame()->SetAuxPart(
					(unsigned char)(pInfo->tid >> 21) & 0x1f, pInfo->tid);
			else if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar &&
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->GetPetFrame())
				CRealMyRoom_AvatarManager::Instance()
					->m_pAvatar->GetPetFrame()
					->SetAuxPart((unsigned char)(pInfo->tid >> 21) & 0x1f,
						pInfo->tid);
			if (old != 0)
			{
				FrListBox::ITEM_LIST::iterator it;
				for (it = m_pFrListBox_304->m_itemList.begin();
					it != m_pFrListBox_304->m_itemList.end(); ++it)
				{
					FrListItem* pOther = *it;
					if (pOther->pData && pOther->selected &&
						((sItemInfo*)pOther->pData)->tid == old)
						m_pFrListBox_304->UnselectItem(pOther);
				}
			}
			ShowMainModel();
			return true;
		}
	}
	case 1:
	case 2:
	case 3:
	case 26:
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tid);
		if (pPart == NULL)
			return false;
		if (pInfo->ItemType == 6 && pInfo->Expired)
		{
			m_pFrListBox_304->ToggleItem(pItem);
			S5::FrRentalExtensionDlg* pDlg =
				CreateForm<S5::FrRentalExtensionDlg>(g_pFresh->GetManager(),
					this, "FrRentalExtensionDlg", NULL);
			pDlg->Initialize(pInfo);
			pDlg->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnRentalExtensionDlgResult,
				3);
			return false;
		}
		if (IsAngelWing(pInfo->tid) && !IsMannerPlayer())
		{
			if (pItem->selected)
				m_pFrListBox_304->ToggleItem(pItem);
			(IActor*)this << MsgObject(NULL, M983(0x23),
				(int)"\xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xc0\xbd", 0, 0,
				0, 0);
			return false;
		}
		bool bUnder = pPart->c.IsUnderLvl;
		if (!bUnder && Doc()->m_myInfo.stat.Level < pPart->c.Level)
			pszMsg =
				K2L_Compatibility("\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc2\xf8\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.");
		else if (bUnder == true && Doc()->m_myInfo.stat.Level > pPart->c.Level)
			pszMsg =
				K2L_Compatibility("\xc0\xcc\xb9\xcc \xc2\xf8\xbf\xeb\xb0\xa1\xb4\xc9\xc7\xd1 \xb7\xb9\xba\xa7\xc0\xbb \xb3\xd1\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xbe\xc6\xc0\xcc\xc5\xdb\xbf\xa1 \xc7\xa5\xbd\xc3\xb5\xc8 \xb7\xb9\xba\xa7 \xc0\xcc\xc7\xcf\xb8\xb8 \xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
		else
		{
			CPartTidList& parts = Doc()->GetMyPartTidList();
			if (!parts.CanSetPart(pInfo->tid))
			{
				m_pFrListBox_304->UnselectItem(pItem);
				if (pPart->EquippableWith[0] == 0)
				{
					(IActor*)this << MsgObject(NULL, M983(0x23),
						(int)"\\c0xffff0000\\c\xbb\xf3\xc7\xb0\xc0\xbb \xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
						0, 0, 0, 0);
					return false;
				}
				char szMsg[256];
				sprintf(szMsg,
					"\xba\xbb \xbb\xf3\xc7\xb0\xc0\xba \xc7\xf6\xc0\xe7 \xc0\xc7\xbb\xf3\xbf\xa1\xb4\xc2 \xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
				(IActor*)this << MsgObject(NULL, M983(0x23), (int)szMsg, 0, 0, 0, 0);
				return false;
			}
			g_audio->PlaySfx("item_select_cloth");
			unsigned long overlapped = parts.GetOverlapped(pInfo->tid);
			if (overlapped)
			{
				FrListBox::ITEM_LIST::iterator it;
				for (it = m_pFrListBox_304->m_itemList.begin();
					it != m_pFrListBox_304->m_itemList.end(); ++it)
				{
					FrListItem* pOther = *it;
					sItemInfo* pOtherInfo = (sItemInfo*)pOther->pData;
					if (pOtherInfo && pOther->selected && pOther != pItem &&
						parts.IsEquipped(pOtherInfo->tid, pOtherInfo->guid) &&
						(overlapped & parts.GetOverlapped(pOtherInfo->tid)))
					{
						m_pFrListBox_304->UnselectItem(*it);
						UnselectItemList(*it);
					}
				}
			}
			CPartTidList avatarParts = Doc()->GetMyPartTidList();
			unsigned long result = 0;
			if (m_pExhibition && m_pExhibition->GetPetFrame())
				result = m_pExhibition->GetPetFrame()->SetPart(&parts,
					pInfo->tid, pInfo->guid);
			if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar &&
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->GetPetFrame())
				CRealMyRoom_AvatarManager::Instance()
					->m_pAvatar->GetPetFrame()
					->SetPart(&avatarParts, pInfo->tid, pInfo->guid);
			if (result == 0)
				(IActor*)this << MsgObject(NULL, M983(0x23),
					(int)"\xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9",
					0, 0, 0, 0);
			else
			{
				sCharacterInfo& info =
					Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
				for (int i = 0; i < parts.m_partNum; ++i)
				{
					info.tidParts[i] = parts.m_tid[i];
					info.ItemIdList[i] = parts.m_uid[i];
				}
			}
			ShowMainModel();
			m_pParts = parts.m_tid;
			memcpy(&m_partUid, parts.m_uid, sizeof(parts.m_uid));
			return true;
		}
		if (pItem->selected)
			m_pFrListBox_304->ToggleItem(pItem);
		(IActor*)this << MsgObject(NULL, M983(0x23), (int)pszMsg, 0, 0, 0, 0);
		return false;
	}
	default:
		break;
	case 6:
	{
		int result = ChangeClubset(((sItemInfo*)pItem->pData)->guid);
		if (result == 0)
			(IActor*)this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc2\xf8\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0);
		else if (result == 2)
			(IActor*)this << MsgObject(NULL, M983(0x23),
				(int)"\xc2\xf8\xbf\xeb \xb7\xb9\xba\xa7\xc0\xbb \xc3\xca\xb0\xfa\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0);
		g_audio->PlaySfx("item_select_club");
		return true;
	}
	case 7:
		ChangeAztec(((sItemInfo*)pItem->pData)->tid);
		g_audio->PlaySfx("item_select_aztec");
		return true;
	case 8:
	case 9:
	case 23:
	case 24:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	{
		unsigned long* pTid = &((sItemInfo*)pItem->pData)->tid;
		m_hand.WaitUntilWorkDone(9999);
		if (m_pExhibition == NULL ||
			*(unsigned long*)((char*)m_pExhibition + 0x498) != *pTid)
			m_hand.Work(static_cast<CHandOwner*>(this), 0, pTid, true, -1);
		break;
	}
	case 10:
		if (!ChangeCaddie(((sItemInfo*)pItem->pData)->guid))
			(IActor*)this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb7\xb9\xba\xa7\xc0\xcc \xc3\xe6\xba\xd0\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb7\xb9\xba\xa7\xbe\xf7 \xc8\xc4\xbf\xa1 \xc2\xf8\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0);
		g_audio->PlaySfx("ui_desktop_icon_click");
		return true;
	case 12:
		ChangeMascot(((sItemInfo*)pItem->pData)->guid);
		g_audio->PlaySfx("ui_desktop_icon_click");
		return true;
	case 13:
	case 14:
	case 15:
	case 16:
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		eEquipSkin index;
		if (S5::UTIL::GetEquipSkinIndex(index, pInfo->tid))
		{
			(&m_n_B78)[index] = pInfo->tid;
			g_audio->PlaySfx("ui_desktop_icon_click");
		}
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(pInfo->tid);
		if (pSkin == NULL)
			return true;
		if ((pInfo->tid & 0x3c00000) == 0 && m_n_B78 != 0)
		{
			m_n_B98 = pSkin->HScroll;
			m_n_B9C = pSkin->VScroll;
			m_scrollPos[0] = 0;
			m_scrollPos[1] = 0;
		}
		Doc()->m_myInfo.userEquip.tidSkin[index] = pInfo->tid;
		return true;
	}
	case 17:
	{
		if (IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
			return true;
		char* pGift = (char*)pItem->pData;
		char date[32] = { 0 };
		char time[32] = { 0 };
		sscanf(pGift + 0xda, "%s %s", date, time);
		char* pDot = strchr(time, '.');
		if (pDot)
			*pDot = 0;
		SetGiftDescString(pGift + 10, pGift + 0x8a, date, time);
		break;
	}
	case 18:
	case 19:
		SafeLoadPet(&((sItemInfo*)pItem->pData)->tid, false);
	case 27:
		g_audio->PlaySfx("ui_desktop_icon_click");
		return true;
	case 22:
		return true;
	case 33:
		if (!S5::ISCutinItem(((sItemInfo*)pItem->pData)->tid))
			return true;
		break;
	case 34:
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		Title* pTitle =
			g_TitleManager->GetTitle((eTitle)(pInfo->tid & 0x3fffff));
		if (pTitle && !pTitle->CanEquip())
		{
			if (pItem->selected)
			{
				m_pFrListBox_304->ToggleItem(pItem);
				m_pFrListBox_304->SelectItem(
					m_pFrListBox_304->GetPreviousSelected(), true);
			}
			(IActor*)this << MsgObject(NULL, M983(0x23),
				(int)"\xc2\xf8\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xb4\xc2 \xc1\xb6\xb0\xc7\xc0\xbb \xb0\xae\xc3\xdf\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9",
				0, 0, 0, 0);
			return false;
		}
		eEquipSkin index;
		if (S5::UTIL::GetEquipSkinIndex(index, pInfo->tid))
		{
			(&m_n_B78)[index] = pInfo->tid;
			g_audio->PlaySfx("ui_desktop_icon_click");
		}
		if (ItemManager()->FindSkin(pInfo->tid) == NULL)
			return true;
		Doc()->m_myInfo.userEquip.tidSkin[index] = pInfo->tid;
		return true;
	}
	}
	g_audio->PlaySfx("ui_desktop_icon_click");
	return true;
}

bool CRealMyRoomMain::UnselectItemFromItemList(unsigned long tid)
{
	FrListBox* pListBox = m_pFrListBox_304;
	if (pListBox == NULL)
		return false;
	if ((tid & 0xfc000000) != 0x08000000)
		return false;
	FrListBox::ITEM_LIST::iterator it;
	for (it = pListBox->m_itemList.begin(); it != pListBox->m_itemList.end();
		++it)
	{
		sItemInfo* pInfo = (sItemInfo*)(*it)->pData;
		if (pInfo && pInfo->tid == tid)
		{
			pListBox->UnselectItem(*it);
			return true;
		}
	}
	return false;
}

bool CRealMyRoomMain::UnselectItemList(unsigned long id)
{
	if (!m_pFrListBox_304)
		return false;
	FrListBox::ITEM_LIST& list = m_pFrListBox_304->m_itemList;
	FrListBox::ITEM_LIST::iterator it = list.begin();
	if (it != list.end())
	{
		while (
			!(*it)->pData || *(unsigned long*)((char*)(*it)->pData + 4) != id)
		{
			++it;
			if (it == list.end())
				return false;
		}
		if (*it)
		{
			UnselectItemList(*it);
			return true;
		}
	}
	return false;
}

void CRealMyRoomMain::UnselectItemList(FrListItem* pItem)
{
	switch (m_n_77C)
	{
	case 4:
	case 21:
	{
		unsigned long tid = ((sItemInfo*)pItem->pData)->tid;
		if ((tid & 0xfc000000) == 0x70000000)
		{
			m_pAuxParts[(tid >> 21) & 0x1f] = 0;
			if (m_pExhibition)
				m_pExhibition->GetPetFrame()->SetAuxPart(
					(unsigned char)(((sItemInfo*)pItem->pData)->tid >> 21) &
						0x1f,
					0);
			else if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar &&
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->GetPetFrame())
				CRealMyRoom_AvatarManager::Instance()
					->m_pAvatar->GetPetFrame()
					->SetAuxPart(
						(unsigned char)(((sItemInfo*)pItem->pData)->tid >> 21) &
							0x1f,
						0);
			ShowMainModel();
			return;
		}
	}
	case 1:
	case 2:
	case 3:
	case 26:
	{
		sItemInfo* pInfo = (sItemInfo*)pItem->pData;
		CPartTidList& parts = Doc()->GetMyPartTidList();
		CPartTidList avatarParts = Doc()->GetMyPartTidList();
		unsigned long* pTids = parts.m_tid;
		unsigned long oldTids[24];
		memcpy(oldTids, pTids, sizeof(oldTids));
		if (parts.IsEquipped(pInfo->tid, pInfo->guid))
		{
			unsigned long result = 0;
			if (m_pExhibition && m_pExhibition->GetPetFrame())
				result = m_pExhibition->GetPetFrame()->SetPart(&parts,
					pInfo->tid, pInfo->guid);
			if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar &&
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->GetPetFrame())
				result = CRealMyRoom_AvatarManager::Instance()
							 ->m_pAvatar->GetPetFrame()
							 ->SetPart(&avatarParts, pInfo->tid, pInfo->guid);
			if (result)
			{
				sCharacterInfo& info =
					Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
				for (int i = 0; i < parts.m_partNum; ++i)
				{
					info.tidParts[i] = parts.m_tid[i];
					info.ItemIdList[i] = parts.m_uid[i];
				}
				m_pParts = pTids;
			}
		}
		else
			m_pFrListBox_304->SelectItem(pItem, true);
		for (int i = 0; i < 24; ++i)
		{
			unsigned long tid = oldTids[i];
			if (tid != 0 && pInfo->tid != tid && pTids[i] != tid && tid != 2 &&
				tid != 3)
				UnselectItemFromItemList(tid);
		}
		if (IsLocalContent(S4_UCC) && IsUccClothes(pInfo->tid))
		{
			unsigned long tid = pInfo->tid;
			sCharacterInfo& info =
				Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
			if (m_pExhibition && m_pExhibition->GetPetFrame())
				m_pExhibition->GetPetFrame()->SetPart(&parts,
					((info.tid << 5 | ((tid >> 13) & 0x1f)) << 13) | 0x8000400,
					0);
			memcpy(&m_partUid, parts.m_uid, sizeof(parts.m_uid));
		}
		ShowMainModel();
		return;
	}
	default:
		m_pFrListBox_304->SelectItem(pItem, true);
		break;
	case 10:
		ChangeCaddie(0);
		TurnOffItemStatus();
		return;
	case 12:
		ChangeMascot(0);
		return;
	case 13:
	case 14:
	case 15:
	case 16:
	case 34:
	{
		eEquipSkin index;
		if (S5::UTIL::GetEquipSkinIndex(index, ((sItemInfo*)pItem->pData)->tid))
		{
			(&m_n_B78)[index] = 0;
			Doc()->m_myInfo.userEquip.tidSkin[index] = 0;
			return;
		}
		break;
	}
	case 22:
	case 33:
		break;
	}
}

int CRealMyRoomMain::GetItemNumInMyPocket(unsigned long tid)
{
	int count = 0;
	if (tid == 0 || (tid & 0xfc000000) == 0x4000000)
	{
		std::map<unsigned int, sCharacterInfo>::const_iterator it;
		for (it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
		{
			if (ItemManager()->FindCommonItem((*it).second.tid) &&
				(tid == 0 || (*it).second.tid == tid))
				count++;
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x8000000)
	{
		std::list<sItemInfo>::iterator it;
		for (it = Doc()->m_partsList.begin(); it != Doc()->m_partsList.end();
			++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(tid == 0 || (*it).tid == tid))
				count++;
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x18000000)
	{
		std::list<sItemInfo>::iterator it;
		for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
			++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(tid == 0 || (*it).tid == tid))
				count++;
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x1c000000)
	{
		std::map<unsigned int, sCaddieInfo>::iterator it;
		for (it = Doc()->m_caddieMap.begin(); it != Doc()->m_caddieMap.end();
			++it)
		{
			if (ItemManager()->FindCommonItem((*it).second.tid) &&
				(tid == 0 || (*it).second.tid == tid))
				count++;
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x20000000)
		count = 0;
	if (tid == 0 || (tid & 0xfc000000) == 0x3c000000)
	{
		std::map<unsigned int, sCharacterInfo>::iterator it;
		for (it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
		{
			IFF_STRUCT::sHairStyle* pHair = ItemManager()->FindHairStyle(tid);
			if (pHair && pHair->cHairID == (*it).second.hairClr &&
				(pHair->cCharID | 0x4000000) == (*it).second.tid)
				count++;
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x10000000)
	{
		std::map<unsigned int, sItemInfo>::iterator it;
		for (it = Doc()->m_clubSetMap.begin(); it != Doc()->m_clubSetMap.end();
			++it)
		{
			if (ItemManager()->FindCommonItem((*it).second.tid) &&
				(tid == 0 || (*it).second.tid == tid))
				count++;
		}
	}
	if (tid == 0 || (tid & 0xfc000000) == 0x14000000)
	{
		std::list<sItemInfo>::iterator it;
		for (it = Doc()->m_ballList.begin(); it != Doc()->m_ballList.end();
			++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(tid == 0 || (*it).tid == tid))
				count++;
		}
	}
	unsigned long type = tid >> 26;
	if (type == 9 && (tid & 0x3e00000) == 0x200000)
	{
		IFF_STRUCT::sSetItem* pSet = ItemManager()->FindSetItem(tid);
		unsigned char i = 0;
		if (pSet->nElems > 0)
		{
			do
			{
				if (tid == 0 || (pSet->ElemIds[i] & 0xfc000000) != 0x4000000)
				{
					std::list<sItemInfo>::iterator it;
					for (it = Doc()->m_partsList.begin();
						it != Doc()->m_partsList.end(); ++it)
					{
						if (ItemManager()->FindCommonItem((*it).tid) &&
							(tid == 0 || (*it).tid == pSet->ElemIds[i]))
							count++;
					}
				}
				i++;
			} while (i < pSet->nElems);
		}
	}
	if (tid == 0 || type == 0xe)
	{
		std::list<sItemInfo>::iterator it;
		for (it = Doc()->m_cutinList.begin(); it != Doc()->m_cutinList.end();
			++it)
		{
			if (ItemManager()->FindCommonItem((*it).tid) &&
				(tid == 0 || (*it).tid == tid))
				count++;
		}
	}
	return count;
}

void CRealMyRoomMain::Net_EquipCharacter()
{
	if (m_userEquip.guidChar != Doc()->m_myInfo.userEquip.guidChar)
	{
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(5);
		packet.Encode4(Doc()->m_myInfo.userEquip.guidChar);
		packet.Send(TO_GAME);
	}
}

// PKT (frente C, lote 20): global 00e78cac (nome da frente B em jp/descimageonlyicon.cpp; logo depois do
// CLobbyState983::ms_pInstance, 0 ou 1; o 983 tem o leitor 00604990 nesta unidade)
extern int g_lobbyModal983;
void CRealMyRoomMain::Net_EquipDresserParts()
{
	typedef std::map<unsigned int, sCharacterInfo> CharMap;
	CharMap& mine = m_mapChar;
	CharMap::const_iterator itDoc = Doc()->m_charMap.begin();
	CharMap::const_iterator it = mine.begin();
	for (; it != mine.end(); ++it)
	{
		if (itDoc == Doc()->m_charMap.end())
			return;
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar((*it).second.tid);
		if (pChar)
		{
			unsigned int nParts = pChar->nAcsries + pChar->nParts;
			bool bDiff = false;
			for (int i = 0; i < (int)nParts; i++)
			{
				if ((*itDoc).second.tidParts[i] != (*it).second.tidParts[i] ||
					(*itDoc).second.ItemIdList[i] != (*it).second.ItemIdList[i])
				{
					bDiff = true;
					break;
				}
			}
			int j = 0;
			for (; j < 5; j++)
			{
				if ((*it).second.tidAuxParts[j] !=
					(*itDoc).second.tidAuxParts[j])
					break;
			}
			if (j < 5 || bDiff)
			{
				sCharacterInfo info;
				memcpy(&info, &(*itDoc).second, sizeof(sCharacterInfo));
				for (int i = 0; i < (int)nParts; i++)
				{
					info.tidParts[i] = (*itDoc).second.tidParts[i];
					info.ItemIdList[i] = (*itDoc).second.ItemIdList[i];
				}
				for (; (int)nParts < 24; nParts++)
				{
					info.tidParts[nParts] = 0;
					info.ItemIdList[nParts] = 0;
				}
				for (int k = 0; k < 5; k++)
					info.tidAuxParts[k] = (*itDoc).second.tidAuxParts[k];
				WSendPacket packet(0x20);
				packet.Encode1(0);
				packet.EncodeBuffer(&info, sizeof(sCharacterInfo));
				packet.Send(TO_GAME);
				// PKT (frente C, lote 20): o 983 (00613796) avisa a troca do personagem equipado quando o global
				// 00e78cac vale 1 (pacote 0xc, o mesmo do avatar_main)
				if (g_lobbyModal983 == 1 &&
					Doc()->m_myInfo.userEquip.guidChar == info.guid)
				{
					WSendPacket change(0xc);
					change.Encode1(4);
					change.Encode4(info.guid);
					change.Send(TO_GAME);
				}
			}
		}
		++itDoc;
	}
}

// PKT (frente C, lote 20): tipos 9 e 10 do pacote 0x20 (so no 983); declaracao em include/packets/realmyroompackets.h
// 0060e810 (metodo desde a A s98, pedido C27)
void CRealMyRoomMain::Net_EquipCutIn983()
{
	sCharacterInfo* pChar = CSharedDoc::Instance()->FindChar983(0xffffffff);
	if (pChar)
	{
		WSendPacket packet(0x20);
		packet.Encode1(9);
		packet.Encode4(pChar->guid);
		packet.EncodeBuffer(pChar->cutIn, sizeof(pChar->cutIn));
		packet.Send(TO_GAME);
	}
}

// 0060e6c0 (metodo desde a A s98, pedido C27)
void CRealMyRoomMain::Net_EquipPoster983()
{
	if (memcmp(m_userEquip.poster, CSharedDoc::Instance()->m_myInfo.userEquip.poster,
		sizeof(CSharedDoc::Instance()->m_myInfo.userEquip.poster)) != 0)
	{
		WSendPacket packet(0x20);
		packet.Encode1(10);
		packet.EncodeBuffer(CSharedDoc::Instance()->m_myInfo.userEquip.poster,
			sizeof(CSharedDoc::Instance()->m_myInfo.userEquip.poster));
		packet.Send(TO_GAME);
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x182, 0, 0, 0, 0, 0); // 983 0060e7c7: numero do 983 (cartazes do ntLobbyBg)
	}
}

void CRealMyRoomMain::Net_EquipCaddie()
{
	CSharedDoc* pDoc = Doc();
	if (m_userEquip.guidCaddie != pDoc->m_myInfo.userEquip.guidCaddie)
	{
		if (pDoc->m_myInfo.userEquip.guidCaddie)
		{
			std::map<unsigned int, sCaddieInfo>::iterator it =
				pDoc->m_caddieMap.find(pDoc->m_myInfo.userEquip.guidCaddie);
			if (it == pDoc->m_caddieMap.end())
				return;
		}
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(1);
		packet.Encode4(Doc()->m_myInfo.userEquip.guidCaddie);
		packet.Send(TO_GAME);
	}
}

void CRealMyRoomMain::Net_EquipItem()
{
	int i;
	for (i = 0; i < 10; i++)
	{
		if (m_userEquip.tidItemSlot[i] !=
			Doc()->m_myInfo.userEquip.tidItemSlot[i])
			break;
	}
	if (i != 10)
	{
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(2);
		packet.EncodeBuffer(Doc()->m_myInfo.userEquip.tidItemSlot,
			sizeof(Doc()->m_myInfo.userEquip.tidItemSlot));
		packet.Send(TO_GAME);
	}
}

void CRealMyRoomMain::Net_EquipGear()
{
	if (m_userEquip.tidBall != Doc()->m_myInfo.userEquip.tidBall ||
		m_userEquip.guidClubSet != Doc()->m_myInfo.userEquip.guidClubSet)
	{
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(3);
		packet.Encode4(Doc()->m_myInfo.userEquip.tidBall);
		packet.Encode4(Doc()->m_myInfo.userEquip.guidClubSet);
		packet.Send(TO_GAME);
	}
}

void CRealMyRoomMain::Net_EquipSkin()
{
	int i;
	for (i = 0; i < 6; ++i)
	{
		if (m_userEquip.tidSkin[i] != Doc()->m_myInfo.userEquip.tidSkin[i])
			break;
	}
	if (i == 6)
		return;
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_cutinList.begin(); it != Doc()->m_cutinList.end(); ++it)
	{
		eEquipSkin index;
		if (S5::UTIL::GetEquipSkinIndex(index, (*it).tid) &&
			(*it).ItemType == 1 &&
			(*it).tid == Doc()->m_myInfo.userEquip.tidSkin[index])
			(*it).ItemType = 2;
	}
	WSendPacket packet(0x20);
	packet.Encode1(4);
	packet.EncodeBuffer(Doc()->m_myInfo.userEquip.tidSkin,
		sizeof(Doc()->m_myInfo.userEquip.tidSkin));
	packet.Send(TO_GAME);
}

// PKT (frente C, lote 37): RFWnd do my room do 983 (classe e global declarados no realmyroommain.cpp pela A)
class CRFWnd983;
extern CRFWnd983* g_pMyRoomCardWnd983; // 00e78154

// PKT (frente C, lote 37): 983 0060d460 - LBUTTONUP do "updown_group.mastery_m" (mapa 00cd2320; o nosso mapa ainda nao tem o
// grupo). Servico 0x40000000 suspenso -> so o aviso; com a janela de cartas aberta nao faz nada; senao avisa o ator principal,
// liga o +0x11411 e pede as conquistas (0x157) com o meu UID.
void CRealMyRoomMain::OnMasteryMUp983()
{
	if (DOCJP()->IsControlServerService983(0x40000000))
	{
		S5::UTIL::NotifyServiceStopped983(0x40000000);
		return;
	}
	if (g_pMyRoomCardWnd983 != NULL)
		return;
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	Doc()->m_goStopMode = true;
	WSendPacket packet((enumClientPacket)0x157);
	packet.Encode4(Doc()->m_myInfo.info.dwUID);
	packet.Send(TO_GAME);
}

void CRealMyRoomMain::Net_StartPeriodItem(unsigned long uid)
{
	WSendPacket packet(0x20);
	packet.Encode1(7);
	packet.Encode4(uid);
	packet.Send(TO_GAME);
}

void CRealMyRoomMain::Net_EquipMascot()
{
	sMascotInfo* pMascot = Doc()->GetMyEquippedMascotInfo();
	if (pMascot)
	{
		if (ItemManager()->CompareSystemTime(pMascot->endDate,
				Doc()->GetServerTime()) <= 0 &&
			!REBANG_983_PCBANG_MASCOT(*pMascot))
			Doc()->m_myInfo.userEquip.guidMascot = 0;
	}
	CSharedDoc* pDoc = Doc();
	if (m_userEquip.guidMascot != pDoc->m_myInfo.userEquip.guidMascot)
	{
		if (pDoc->m_myInfo.userEquip.guidMascot)
		{
			std::map<unsigned int, sMascotInfo>::iterator it =
				pDoc->m_mascotMap.find(pDoc->m_myInfo.userEquip.guidMascot);
			if (it == pDoc->m_mascotMap.end())
				return;
		}
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(8);
		packet.Encode4(Doc()->m_myInfo.userEquip.guidMascot);
		packet.Send(TO_GAME);
	}
}

bool CRealMyRoomMain::OnEnchantDlgResult(int result, FrForm* pForm)
{
	m_pFrEnchantDlg_428 = NULL;
	return true;
}

bool CRealMyRoomMain::OnPangGiftDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		sGiftInfo* pInfo = ((FrPangGiftDlg*)pForm)->GetItemInfo();
		WSendPacket packet((enumClientPacket)0x24);
		packet.Encode4(pInfo->guid);
		packet.Encode4(pInfo->tid);
		packet.Send(TO_GAME);
		Sleep(50);
	}
	m_pFrPangGiftDlg_434 = NULL;
	return true;
}

bool CRealMyRoomMain::OnReturnGiftDlgResult(int result, FrForm* pForm)
{
	if (result != 1)
		return true;
	FrReturnGiftDlg* pDlg = DYNAMIC_CAST(FrReturnGiftDlg, pForm);
	if (pDlg)
	{
		WSendPacket packet((enumClientPacket)0x4d);
		packet.Encode4(pDlg->m_giftIdx);
		packet.Encode4(pDlg->m_typeId);
		packet.Send(TO_GAME);
	}
	return true;
}

bool CRealMyRoomMain::OnMoveGiftDlgResult(int result, FrForm* pForm)
{
	if (result == 1 && m_psGiftInfo_7AC)
	{
		m_b_C25 = 0;
		HandleMsg(MsgObject(NULL, M983(0x24),
			(int)K2L_Compatibility("\xbc\xb1\xb9\xb0 \xbf\xc5\xb1\xe2\xb1\xe2\xb8\xa6 \xbf\xe4\xc3\xbb\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
		HandleMsg(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)0x24);
		packet.Encode4(m_psGiftInfo_7AC->guid);
		packet.Encode4(m_psGiftInfo_7AC->tid);
		packet.Send(TO_GAME);
	}
	return true;
}

bool CRealMyRoomMain::OnMascotInfoDlgResult(int result, FrForm* pForm)
{
	m_pFrMascotInfoDlg_438 = NULL;
	return true;
}

bool CRealMyRoomMain::OnUpgradeDlgResult(int result, FrForm* pForm)
{
	m_pFrDowngradeDlg_42C = NULL;
	return true;
}

bool CRealMyRoomMain::OnNoteDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		WSendPacket packet((enumClientPacket)0x3c);
		packet.Encode2(0x111);
		packet.Encode4(0xffffffff);
		packet.EncodeStr(std::string(m_pFrNoteDlg_43C->GetNote()));
		packet.Encode1(1);
		packet.Send(TO_GAME);
	}
	m_pFrNoteDlg_43C = NULL;
	return true;
}

bool CRealMyRoomMain::OnFunritureSaveMsgdlgResult(int nResult, FrForm* pForm)
{
	SetCameraDisplayType(0);
	return true;
}

bool CRealMyRoomMain::OnOfflineShopEditMsgDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		m_n_D5C = 1;
		WSendPacketSem983 packet((enumClientPacket)0xc4); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(Doc()->m_replayTargetUID);
		packet.Send(TO_GAME);
	}
	return true;
}

bool CRealMyRoomMain::OnFurnitrueListDeleteDlgResult(int result, FrForm* pForm)
{
	if (result != 1)
		return true;
	FrListItem* pItem = m_pFrListBox_3B0->GetItemUnderCursor();
	if (pItem == NULL)
	{
		FrForm* pNotify =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pNotify->SetMessage(
			"\xbb\xe8\xc1\xa6\xc7\xd2 \xb0\xa1\xb1\xb8\xb8\xa6 \xc3\xa3\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.",
			false);
		pNotify->Open(NULL, 3);
		return true;
	}
	else
	{
		sFurniture_List* pFurniture = (sFurniture_List*)pItem->pData;
		pItem->selected = false;
		if (pFurniture)
		{
			m_pFrListBox_3B0->UnselectItem(pItem);
			std::list<sFurniture_List>::iterator it;
			for (it = Doc()->m_furnitureList.begin();
				it != Doc()->m_furnitureList.end(); ++it)
			{
				if ((*it).id == pFurniture->id)
					(*it).bArrange = 0;
			}
			CRealMyRoomData::Instance()->ListFurnitureSelect(pFurniture);
			CRealMyRoomData::Instance()->SetState(6);
			return true;
		}
		FrForm* pNotify =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pNotify->SetMessage(
			"\xbb\xe8\xc1\xa6\xc7\xd2 \xb0\xa1\xb1\xb8\xb8\xa6 \xc3\xa3\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.",
			false);
		pNotify->Open(NULL, 3);
	}
	return true;
}

bool CRealMyRoomMain::OnCaddieInfoDlgResult(int result, FrForm* pForm)
{
	if (result == 2 && m_pFrWnd_398)
	{
		m_pFrWnd_398->Enable(true);
		if (pForm)
		{
			sCaddieInfo info = ((FrCaddieInfoDlg*)pForm)->GetCaddieInfo();
			CaddieIterator it;
			for (it = ItemManager()->m_CaddieMap.begin();
				it != ItemManager()->m_CaddieMap.end(); ++it)
			{
				if ((*it).second.c.TypeId == info.tid)
				{
					if ((*it).second.c.Level > Doc()->m_myInfo.stat.Level)
						m_pFrWnd_398->Enable(false);
					break;
				}
			}
		}
	}
	return true;
}

bool CRealMyRoomMain::OnOpenTikiReport(int result, FrForm* pForm)
{
	if (!IsLocalContent(S3_CADDIE_REPORT))
		return false;
	if (result == 1)
	{
		if (m_n_7D0 && m_n_7D8)
		{
			WSendPacket packet((enumClientPacket)0xab); // JP 983 (s63): 0xa3 no KR 645
			packet.Encode4(m_n_7D0);
			packet.Encode4(m_n_7D8);
			packet.Send(TO_GAME);
			HandleMsg(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
	}
	else
	{
		m_n_7D0 = 0;
	}
	m_n_7D8 = 0;
	return true;
}

bool CRealMyRoomMain::OnOpenNewYearMoneyDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (m_n_7B0)
		{
			// PKT (frente C, lote 20): o 983 manda pela tarefa ao ator principal (+0x30)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			WSendPacket packet((enumClientPacket)0x93);
			packet.Encode4(m_n_7B0);
			packet.Send(TO_GAME);
		}
		m_n_7B0 = 0;
		return true;
	}
	m_n_7B0 = 0;
	return true;
}

bool CRealMyRoomMain::OnOpenEventGiftDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (m_n_7B4)
		{
			// PKT (frente C, lote 20): o 983 manda pela tarefa ao ator principal (+0x30)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			WSendPacket packet((enumClientPacket)0x94);
			packet.Encode4(m_n_7B4);
			packet.Send(TO_GAME);
		}
		m_n_7B4 = 0;
		return true;
	}
	m_n_7B4 = 0;
	return true;
}

bool CRealMyRoomMain::OnOpenS4EventGiftDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (m_n_7BC)
		{
			// PKT (frente C, lote 20): o 983 manda pela tarefa ao ator principal (+0x30)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			WSendPacket packet((enumClientPacket)0xb2);
			packet.Encode4(m_n_7BC);
			packet.Send(TO_GAME);
		}
		m_n_7BC = 0;
		return true;
	}
	m_n_7BC = 0;
	return true;
}

void CRealMyRoomMain::OpenCaddieInfo(const sCaddieInfo& info)
{
	FrWnd* pWnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("caddie_info");
	if (pWnd)
		return;
	FrCaddieInfoDlg* pDlg = DYNAMIC_CAST(FrCaddieInfoDlg, pWnd);
	if (pDlg == NULL)
		pDlg = CreateForm<FrCaddieInfoDlg>(g_pFresh->GetManager(), this,
			"caddie_info", NULL);
	if (pDlg)
	{
		pDlg->SetCaddieInfo(info);
		pDlg->Open((FRESH_PFN_RESULT)&CRealMyRoomMain::OnCaddieInfoDlgResult,
			3);
		if (m_pFrWnd_398)
			m_pFrWnd_398->Enable(false);
	}
}

void CRealMyRoomMain::OpenInformation(FrListItem* pItem)
{
	if (pItem != NULL)
	{
		m_f_698 = pItem->pos.x - 300.0f;
		m_f_69C = pItem->pos.y - 450.0f;
	}
	switch (m_n_77C)
	{
	case 10:
		OpenCaddieInfo(*(sCaddieInfo*)pItem->pData);
		break;
	case 12:
	{
		sMascotInfo* pInfo = (sMascotInfo*)pItem->pData;
		m_pFrMascotInfoDlg_438 = CreateForm<FrMascotInfoDlg>(
			g_pFresh->GetManager(), this, "mascot_info", NULL);
		m_pFrMascotInfoDlg_438->SetMascotInfo(*pInfo,
			m_pExhibition ? m_pExhibition->GetModel() : NULL);
		m_pFrMascotInfoDlg_438->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnMascotInfoDlgResult, 1);
		break;
	}
	case 0x11:
		if (!IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
			OpenInformation((unsigned long*)pItem->pData + 1,
				(sGiftInfo*)pItem->pData, NULL);
		break;
	default:
		OpenInformation((unsigned long*)pItem->pData + 1, NULL,
			(unsigned long*)pItem->pData);
		break;
	}
}

void CRealMyRoomMain::OpenInformation(unsigned long* pTid, sGiftInfo* pGift,
	unsigned long* pUid)
{
	if (g_pFresh->GetManager()->GetDesktop()->FindChildByName("information"))
		return;
	__rtti_obj = NULL;
	if (g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"information_setitem"))
		return;
	__rtti_obj = NULL;
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(*pTid);
	unsigned long uid = pUid ? *pUid : 0;
	if (pItem == NULL)
		return;
	FrForm* pForm;
	if ((pItem->TypeId & 0xfc000000) == 0x24000000)
	{
		FrSetItemDlg* pSetItem = CreateForm<FrSetItemDlg>(
			g_pFresh->GetManager(), this, "information_setitem", NULL);
		if (pSetItem == NULL)
			return;
		pSetItem->SetSetItem(pItem->TypeId);
		pForm = pSetItem;
	}
	else
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "information",
			NULL);
		if (pForm == NULL)
			return;
		FrArea* pPortrait =
			DYNAMIC_CAST(FrArea, pForm->FindChildByName("portrait"));
		if (pPortrait)
		{
			if (IsLocalContent(S4_UCC) && IsLocalContent(S4_GUID_BASE) &&
				uid != 0 && IsUccClothes(*pTid))
			{
				const Bitmap* pIcon = GetUccItemIcon(uid);
				if (pIcon)
					pPortrait->SetBgImg(pIcon);
			}
			else
				pPortrait->SetBgImg(pItem->Icon);
		}
		FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
		FrArea* pLevel = DYNAMIC_CAST(FrArea, pForm->FindChildByName("level"));
		bool bTime = false;
		if (pName && pLevel)
		{
			char name[128] = { 0 };
			if (IsLocalContent(S4_UCC) && IsLocalContent(S4_GUID_BASE) &&
				uid != 0 && IsUccClothes(*pTid))
				strncpy(name, GetUccItemName(uid), sizeof(name));
			else
				strncpy(name, pItem->Name, sizeof(name));
			const char* fmt;
			if (IsAngelWing(pItem->TypeId) && !IsMannerPlayer())
				fmt =
					K2L_Compatibility("\xc0\xcc\xb8\xa7: %s \\c0xffff0000\\c(\xc2\xf8\xbf\xeb\xba\xd2\xb0\xa1)\\c0xff000000\\c\n\n");
			else
				fmt = K2L_Compatibility("\xc0\xcc\xb8\xa7: %s\n\n");
			pName->AddText(MakeStr(fmt, name), false, true);
			pName->AddText(pItem->IsUnderLvl
					? K2L_Compatibility("\xb7\xb9\xba\xa7:                \xc0\xcc\xc7\xcf\n")
					: K2L_Compatibility("\xb7\xb9\xba\xa7:                \xc0\xcc\xbb\xf3\n"),
				false, true);
			pLevel->SetBgImg(MakeStr("level_%03d", pItem->Level + 1));

			unsigned char type = 0;
			bool bExpired = false;
			int days;
			int hours;
			bool bFound = false;
			if (pGift)
			{
				if (pItem->TimeFlag)
				{
					days = *(unsigned short*)((char*)pGift + 8);
					type = 1;
					hours = 0;
					bFound = true;
				}
			}
			else
			{
				std::list<sItemInfo>::iterator it;
				for (it = Doc()->m_cutinList.begin();
					it != Doc()->m_cutinList.end(); ++it)
				{
					if (&(*it).tid == pTid)
						break;
				}
				if (it == Doc()->m_cutinList.end())
				{
					for (it = Doc()->m_myItemList.begin();
						it != Doc()->m_myItemList.end(); ++it)
					{
						if (&(*it).tid == pTid)
							break;
					}
				}
				if (it == Doc()->m_myItemList.end())
				{
					for (it = Doc()->m_partsList.begin();
						it != Doc()->m_partsList.end(); ++it)
					{
						if (&(*it).tid == pTid)
							break;
					}
					if (it != Doc()->m_partsList.end())
						bFound = true;
				}
				else
					bFound = true;
				if (bFound)
				{
					days = (*it).HourRemain / 24;
					hours = (*it).HourRemain % 24;
					type = (*it).ItemType;
					bExpired = (*it).Expired;
				}
			}
			if (bFound)
			{
				if (type > 0)
				{
					bTime = true;
					pName->AddText("\n", false, true);
				}
				if (type == 1)
					pName->AddText(
						MakeStr(
							K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3"),
							days, hours),
						false, true);
				else if (type > 1 && type <= 5)
				{
					if (bExpired)
						pName->AddText(
							K2L_Compatibility("\\c0xffff0000\\c\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3\\c0xff000000\\c\xc0\xcc \\c0xffff0000\\c\xb8\xb8\xb7\xe1\\c0xff000000\\c\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.\n"),
							false, true);
					else if (IsLocalContent(S3_CADDIE_REPORT) &&
						pItem->TypeId == 0x1a000042)
						pName->AddText(
							MakeStr(
								"\xbf\xad\xb6\xf7 \xb0\xa1\xb4\xc9 \xb1\xe2\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3\n",
								days, hours),
							false, true);
					else if (days == 0 && hours == 0)
						pName->AddText(
							K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: 1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n"),
							false, true);
					else
						pName->AddText(
							MakeStr(
								K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3\n"),
								days, hours),
							false, true);
				}
			}
		}
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			char desc[512] = { 0 };
			FrEdit* pAbility =
				DYNAMIC_CAST(FrEdit, pForm->FindChildByName("avility"));
			FrStatic* pStatic = DYNAMIC_CAST(FrStatic,
				pForm->FindChildByName("static_avility"));
			if (pAbility)
			{
				WRect rc;
				if (bTime == true)
				{
					pAbility->AddText("\n\n", false, true);
					rc = pStatic->GetRect();
					rc.y += 25.0f;
				}
				else
				{
					rc = pStatic->GetRect();
					rc.y -= 3.0f;
				}
				pStatic->SetRect(rc);
				IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(*pTid);
				if (pPart)
				{
					for (int i = 0; i < 5; ++i)
					{
						if (pPart->Attr[i] != 0 || pPart->Slot[i] != 0)
							pAbility->AddText(
								MakeStr("\\c0xff0000ff\\c%s\\c0xff000000\\c",
									MakeStatusString(i, pPart->Attr[i],
										pPart->Slot[i])),
								false, true);
					}
				}
				// 983 00499210: cartas de atributo do taco = m_cardAbilityList[0] (+0x37ec), nao m_cardList
				std::list<sSCardAvilityPeriodInfo>::iterator it;
				for (it = Doc()->m_cardAbilityList[0].begin();
					it != Doc()->m_cardAbilityList[0].end(); ++it)
				{
					if ((*it).partsTid != *pTid || (*it).partsUid != uid)
						continue;
					if ((*it).cardType == 0)
					{
						IFF_STRUCT::sCard* pCard =
							ItemManager()->FindCard((*it).tid);
						if (pCard)
						{
							for (int i = 0; i < 5; ++i)
							{
								if (pCard->COM[i] > 0)
									pAbility->AddText(
										MakeStr(
											"\\c0xff60ee0e\\c%s\\c0xff000000\\c",
											MakeCardStatusString(i,
												pCard->COM[i])),
										false, true);
							}
						}
					}
					else if ((*it).cardType == 1)
					{
						IFF_STRUCT::sDesc* pDesc =
							ItemManager()->FindDesc((*it).tid);
						if (pDesc)
						{
							strcpy(desc, pDesc->Desc);
							for (int i = 0; i < 512; ++i)
							{
								if (desc[i] == '\n')
								{
									desc[i] = 0;
									break;
								}
							}
							pAbility->AddText(
								MakeStr("\\c0xffff0000\\c%s\\c0xff000000\\c",
									desc),
								false, true);
						}
					}
				}
			}
		}
		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pItem->TypeId);
		if (pDesc)
			pForm->SetMessage(pDesc->Desc, false);
	}
	pForm->Open(NULL, 3);
}

void CRealMyRoomMain::EquipItem(sItemInfo* pItem)
{
	int nSlot = 10;
	if (IsLocalContent((localContentType_t)3))
		nSlot = Doc()->GetNumAddItemSlotByMascot() + 8;
	if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
	{
		CCardManager::Instance()->SetPlayerIndex(0xff);
		CCardManager::Instance()->CalcCardPeriodAndStatus();
		int nPeriod = CCardManager::Instance()->GetCardPeriodSlot();
		if (nPeriod > 0)
			nSlot += nPeriod;
		else
		{
			if (Doc()->GetNumAddItemSlotByMascot() == 1)
			{
				Doc()->m_myInfo.userEquip.tidItemSlot[9] = 0;
			}
			else
			{
				Doc()->m_myInfo.userEquip.tidItemSlot[8] = 0;
				Doc()->m_myInfo.userEquip.tidItemSlot[9] = 0;
			}
		}
	}
	int i = 0;
	while (i < nSlot)
	{
		if (Doc()->m_myInfo.userEquip.tidItemSlot[i] == 0)
			break;
		i++;
	}
	if (i == nSlot)
	{
		(IActor*)this << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xb4\xf5 \xc0\xcc\xbb\xf3 \xc0\xe5\xc2\xf8\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
			0, 0, 0, 0);
	}
	else
	{
		Doc()->m_myInfo.userEquip.tidItemSlot[i] = pItem->tid;
		unsigned long tid = pItem->tid;
		int nEquipped = 0;
		unsigned long* pSlot = Doc()->m_myInfo.userEquip.tidItemSlot;
		for (int n = 10; n != 0; n--, pSlot++)
		{
			if (tid == *pSlot)
				nEquipped++;
		}
		if (pItem->Common[0] - nEquipped <= 0)
		{
			m_hand.WaitUntilWorkDone(9999);
			if (m_pExhibition)
			{
				delete m_pExhibition;
				m_pExhibition = NULL;
			}
			FrListBox::ITEM_LIST::iterator it;
			for (it = m_pFrListBox_304->m_itemList.begin();
				it != m_pFrListBox_304->m_itemList.end(); ++it)
			{
				if ((*it)->pData &&
					pItem->tid == ((sItemInfo*)(*it)->pData)->tid)
				{
					m_pFrListBox_304->_DelItem(it);
					break;
				}
			}
			m_pFrListBox_304->SortItem(EmptyCompares);
		}
	}
}

void CRealMyRoomMain::UnequipItem(unsigned long* pTid)
{
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
		++it)
	{
		if ((*it).tid == *pTid && !IsInItemList(*pTid))
		{
			m_pFrListBox_304->AddItem(&*it);
			m_pFrListBox_304->SortItem(EmptyCompares);
		}
	}
	*pTid = 0;
}

bool CRealMyRoomMain::IsInItemList(unsigned long tid)
{
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pFrListBox_304->m_itemList.begin();
		it != m_pFrListBox_304->m_itemList.end(); ++it)
	{
		sItemInfo* pInfo = (sItemInfo*)(*it)->pData;
		if (pInfo && pInfo->tid == tid)
			return true;
	}
	return false;
}

void CRealMyRoomMain::SafeLoadPet(unsigned long* pTids, bool bForce)
{
	m_hand.WaitUntilWorkDone(9999);
	if (bForce || m_pExhibition == NULL || m_pExhibition->GetTypeId() != *pTids)
		m_hand.Work(this, 0, pTids, true, -1);
}

void CRealMyRoomMain::TurnOnItemStatus()
{
	if (m_pFrViewer_2EC)
		m_pFrViewer_2EC->Enable(true);
	m_n_774 = 0;
	SetLevelBar();
}

void CRealMyRoomMain::TurnOffItemStatus()
{
	if (m_pFrViewer_2EC)
		m_pFrViewer_2EC->Enable(false);
	m_n_774 = 1;
	SetLevelBar();
}

void CRealMyRoomMain::AutoScroll()
{
	if (m_pFrListBox_304)
	{
		FrScrollBar* pScroll = m_pFrListBox_304->GetScrollBar();
		if (pScroll)
			pScroll->SetCurTopRow(0);
	}
}

bool CRealMyRoomMain::OnFortuneOpenDlgResult(int result, FrForm* pForm)
{
	std::list<sItemInfo>::iterator it = Doc()->m_myItemList.end();
	if (m_pFortuneOpenDlg)
	{
		unsigned long tid = m_pFortuneOpenDlg->GetTypeId();
		for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
			++it)
		{
			if ((*it).tid == tid)
				break;
		}
	}
	m_pFortuneOpenDlg = NULL;
	if (it != Doc()->m_myItemList.end())
	{
		(*it).Common[0]--;
		if ((*it).Common[0] <= 0)
			Doc()->m_myItemList.erase(it);
	}
	HandleMsg(MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
	return true;
}

bool CRealMyRoomMain::OnFortuneDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		// PKT (frente C, lote 37): 983 0060fa40 - pela tarefa ao ator principal e pacote 0xb2 (o 983 nao manda 0x59 em lugar
		// nenhum; o 0xb2 e o mesmo do OnOpenS4EventGiftDlgResult). O 983 abre "fortune_new" e guarda a janela em +0x20c.
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		{
			WSendPacket packet((enumClientPacket)0xb2);
			packet.Encode4(m_pFrFortuneDlg_440->GetTypeId());
			packet.Send(TO_GAME);
		}
		m_pFrFortuneDlg_440 = NULL;
		return true;
	}
	m_pFrFortuneDlg_440 = NULL;
	return true;
}

// WIP (run 40): HandleMsg, part 1 (cases up to 0xc6 + a few simple ones). The remaining cases
// (0x12d, 0x5b..0x68 helpers, 0xc9..0xe4+, >= 0x12e) are still to be transcribed from ghidra.c.
void CRealMyRoomMain::HandleMsg(const MsgObject& msg)
{
	if (g_CurrentTime < msg.time)
	{
		StoreDelayedMsg(msg);
		return;
	}
	if (msg.message == M983(0xe4))
	{
		sSlotInfo* pSlot = (sSlotInfo*)msg.param1;
		sCharacterInfo* pChar = (sCharacterInfo*)msg.param2;
		CRealMyRoom_Avatar* pNew =
			CRealMyRoom_AvatarManager::Instance()->CreateAvatar(*pSlot, *pChar,
				this);
		{
			CRealMyRoom_AvatarBurden burden(pSlot->dwGuid, *pChar, NULL);
			((CRealMyRoomTask*)GetTask())->AddAvatarBurden(burden);
		}
		WPolySoup* pSoup = *(WPolySoup**)((char*)Doc()->m_pGolfDoc + 8); // A729: membro (cl novo 0x10cc4)
		if ((int)pSlot->dwGuid != *(int*)DOCPTR983(0x81a))
		{
			char* pPoint = (char*)pSoup->FindPoint(
				MakeStr("\xb8\xb6\xc0\xcc\xb7\xeb\xbd\xc3\xc0\xdb\xc1\xa1"));
			if (pPoint == NULL)
				pPoint = (char*)pSoup->FindPoint(
					MakeStr("\xb4\xeb\xc8\xad\xb9\xe6\xbd\xc3\xc0\xdb"));
			WVector pos = *(WVector*)(pPoint + 0x24);
			pNew->SetPosition(pos);
			pNew->SetAngle(pSlot->location[2]);
			return;
		}
		CRealMyRoom_AvatarManager::Instance()->m_pAvatar = pNew;
		Doc()->m_myInfo.userEquip.guidChar =
			*(unsigned long*)((char*)pChar + 4);
		Doc()->SendTradeCloseShop();
		char* pPoint = (char*)pSoup->FindPoint(
			MakeStr("\xb8\xb6\xc0\xcc\xb7\xeb\xbd\xc3\xc0\xdb\xc1\xa1"));
		if (pPoint == NULL)
			pPoint = (char*)pSoup->FindPoint(
				MakeStr("\xb4\xeb\xc8\xad\xb9\xe6\xbd\xc3\xc0\xdb"));
		WVector pos = *(WVector*)(pPoint + 0x24);
		pos = GetPVS().GetGroundPoint(pos, false, NULL, false);
		CRealMyRoom_Avatar* pMine =
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar;
		pMine->SetPosition(pos);
		pMine->SetAngle(0.0f);
		pMine->Stop();
		m_v220 = pMine->m_vPos;
		m_v220.y += 4.6f;
		m_v220.z -= 46.0f;
		if (Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
		{
			SetUIState(1);
			if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar != NULL)
			{
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetState(2);
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(
					false);
			}
			SetCameraDisplayType(1);
			return;
		}
		SetUIState(0);
		SetCameraDisplayType(4);
		if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar == NULL)
			return;
		CRealMyRoom_AvatarManager::Instance()->m_pAvatar->ChangeCharacterInfo();
		CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetState(0);
		CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(true);
		return;
	}
	if (msg.message == M983(0xe5))
	{
		CRealMyRoom_Avatar* pAvatar =
			CRealMyRoom_AvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		if (pAvatar == NULL)
			return;
		CRealMyRoom_Avatar*& pMine =
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar;
		if (pMine != NULL)
		{
			if (strcmp(pAvatar->GetIDName(), pMine->GetIDName()) == 0)
			{
				Doc()->SendTradeCloseShop();
				pMine->SubState(0x7e);
				pMine = NULL;
			}
			const char* nick = pAvatar->GetNickName();
			unsigned long guid = *(unsigned long*)((char*)pAvatar + 0xdc);
			if (IsLocalContent((localContentType_t)10))
			{
				sBriefUserInfo& user =
					Doc()->m_briefUserInfoMap.find(guid)->second;
				sBriefUserInfo info = user;
				if (info.IsIdentity(4))
				{
					info = user;
					if (info.state & 1)
						CChatMsg::Instance()->AddChatMsg(
							MakeStr(
								K2L_Compatibility("%s \xb4\xd4\xc0\xcc \xc5\xf0\xc0\xe5\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
								nick),
							0xffff00, false, false);
				}
				else
					CChatMsg::Instance()->AddChatMsg(
						MakeStr(
							K2L_Compatibility("%s \xb4\xd4\xc0\xcc \xc5\xf0\xc0\xe5\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
							nick),
						0xffff00, false, false);
			}
			else
				CChatMsg::Instance()->AddChatMsg(
					MakeStr(
						K2L_Compatibility("%s \xb4\xd4\xc0\xcc \xc5\xf0\xc0\xe5\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
						nick),
					0xffff00, false, false);
		}
		bool bFx = true;
		if (IsLocalContent((localContentType_t)10))
		{
			sBriefUserInfo& user =
				Doc()
					->m_briefUserInfoMap
					.find(*(unsigned long*)((char*)pAvatar + 0xdc))
					->second;
			sBriefUserInfo info = user;
			if (info.IsIdentity(4))
			{
				info = user;
				if (!(info.state & 1))
					bFx = false;
			}
		}
		if (bFx)
		{
			CFxSequence* pSeq =
				CFx::Instance()->OpenSequence("Talk_Room.seq", true);
			if (pSeq != NULL)
			{
				WVector fxPos(pAvatar->m_vPos.x, pAvatar->m_vPos.y + 5.0f,
					pAvatar->m_vPos.z);
				pSeq->m_pos = fxPos;
			}
		}
		CRealMyRoom_AvatarBurden burden(
			*(unsigned long*)((char*)pAvatar + 0xdc),
			pAvatar->GetCharacterInfo(),
			pAvatar->GetPetFrame());
		((CRealMyRoomTask*)GetTask())->RemoveAvatarBurden(burden);
		CRealMyRoom_AvatarManager::Instance()->DeleteAvatar(
			(const char*)pAvatar);
		return;
	}
	if (msg.message == M983(0xca))
	{
		sItemInfo item;
		item.guid = msg.param1;
		item.tid = msg.param2;
		item.ItemType = msg.param4;
		item.gift_flag = 1;
		item.HourRemain = msg.param3 * 0x18;
		if (IsLocalContent(S4_UCC) && IsUccClothes(item.tid))
		{
			strncpy(item.UccIndex, ((sBuyItemResult*)msg.param5)->UccIndex, 9);
			item.Seq = 1;
			Doc()->m_uccItemList.push_back(item);
			return;
		}
		Doc()->AddMyPartsList(item);
		return;
	}

	if (msg.message == M983(0x5b))
	{
		CRealMyRoom_Avatar* pAvatar =
			CRealMyRoom_AvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		if (pAvatar == NULL)
			return;
		if (pAvatar ==
			*(CRealMyRoom_Avatar**)((char*)
										CRealMyRoom_AvatarManager::Instance() +
				0xa4))
			return;
		switch (msg.param2)
		{
		case 5:
			pAvatar->SetAction(msg.param3);
			return;
		case 6:
		{
			WVector pos;
			pos.x = pAvatar->GetTargetPosition().x + **(float**)msg.param3;
			pos.y = pAvatar->GetTargetPosition().y;
			pos.z = pAvatar->GetTargetPosition().z + **(float**)msg.param4;
			pAvatar->SetTargetPosition(pos);
			pAvatar->SetPendingTargetAngle(**(float**)msg.param5);
			return;
		}
		case 7:
			pAvatar->ChangeMotion(4, (const char*)msg.param3, 0.2f);
			return;
		case 4:
		{
			float x = **(float**)msg.param3;
			float z = **(float**)msg.param4;
			WVector pos;
			pos = GetPVS().GetGroundPoint(WVector(x, 0.0f, z), false, NULL,
				false);
			pAvatar->SetPosition(pos);
			pAvatar->SetAngle(**(float**)msg.param5);
			if (!IsLocalContent((localContentType_t)10))
			{
				CFxSequence* pSeq =
					CFx::Instance()->OpenSequence("Talk_Room.seq", true);
				if (pSeq == NULL)
					return;
				WVector fxPos(pos.x, pos.y + 5.0f, pos.z);
				pSeq->m_pos = fxPos;
				return;
			}
			std::map<unsigned long, sBriefUserInfo>& users =
				Doc()->m_briefUserInfoMap;
			sBriefUserInfo info = users.find(msg.param1)->second;
			if (!info.IsIdentity(4))
				return;
			info = users.find(msg.param1)->second;
			if (!(info.state & 1))
				return;
			CFxSequence* pSeq =
				CFx::Instance()->OpenSequence("Talk_Room.seq", true);
			if (pSeq == NULL)
				return;
			WVector fxPos(pAvatar->m_vPos.x, pAvatar->m_vPos.y + 5.0f,
				pAvatar->m_vPos.z);
			pSeq->m_pos = fxPos;
			return;
		}
		case 8:
			pAvatar->SetState(msg.param3);
			return;
		default:
			return;
		}
	}

	if (msg.message == M983(0x215))
	{
		bool bSave = msg.param1 == 1;
		unsigned long typeId = *(unsigned long*)msg.param2;
		const char* uccIndex = (const char*)msg.param3;
		if (m_pFrUccDrawDlg_45C == NULL)
			return;
		if (m_pFrUccDrawDlg_45C->m_pItemInfo->tid != typeId)
			return;
		if (strcmp(m_pFrUccDrawDlg_45C->m_pItemInfo->UccIndex, uccIndex) != 0)
			return;
		m_pFrUccDrawDlg_45C->m_bWaiting = false;
		if (bSave)
		{
			Bitmap* pBack =
				m_pFrUccDrawDlg_45C->m_pClothes[1]->GetTexture32BPP(true);
			Bitmap* pFront =
				m_pFrUccDrawDlg_45C->m_pClothes[0]->GetTexture32BPP(true);
			UccManager()->SetUccTempClothes(typeId, uccIndex, pFront, pBack);
			m_pFrUccDrawDlg_45C->SetSaved(true);
			return;
		}
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce \xc0\xc7\xbb\xf3\xc0\xc7 \xc0\xd3\xbd\xc3 \xc0\xfa\xc0\xe5\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 1);
		return;
	}

	if (msg.message == M983(0x205))
	{
		if (IsLocalContent((localContentType_t)0x14) &&
			m_pFrUniteResultDlg_464 == NULL)
		{
			_SYSTEMTIME time = *(_SYSTEMTIME*)msg.param1;
			m_pFrUniteResultDlg_464 = CreateForm<FrUniteResultDlg>(
				g_pFresh->GetManager(), this, "uniteresult", NULL);
			m_pFrUniteResultDlg_464->EnableDrag(false);
			m_pFrUniteResultDlg_464->SetTikiReportState(time);
			WPoint pos;
			pos.x = 68.0f;
			pos.y = 48.0f;
			m_pFrUniteResultDlg_464->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnResultDlg_Report, pos, 1);
		}
		return;
	}

	if (msg.message == M983(0x245))
	{
		if (!IsLocalContent(S4_FURNITURE_ABILITY))
		{
			FrForm* pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pNotify->SetMessage(
				K2L_Compatibility("\xb3\xad\xb7\xce \xb1\xe2\xb4\xc9\xc0\xba \xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. "),
				false);
			pNotify->Open(NULL, 3);
			return;
		}
		unsigned long tid = msg.param1;
		unsigned long count = msg.param2;
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(tid);
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_item", NULL);
		if (pForm == NULL)
			return;
		FrArea* pIcon =
			DYNAMIC_CAST(FrArea, pForm->FindChildByName("itemicon"));
		FrEdit* pName =
			DYNAMIC_CAST(FrEdit, pForm->FindChildByName("itemname"));
		if (pIcon != NULL)
		{
			const char* bg;
			if (pItem != NULL)
				bg = pItem->Icon;
			else if (tid == 0)
				bg = "hide";
			else
			{
				WRect rect = pIcon->GetRect();
				rect.x += 15.0f;
				rect.y += 7.0f;
				pIcon->SetRect(rect);
				bg = "item1_16";
			}
			pIcon->SetBgImg(bg);
		}
		if (pName != NULL)
		{
			const char* text;
			if (pItem != NULL)
				text = pItem->Name;
			else if (tid == 0)
				text = "none";
			else
				text = MakeStr(K2L_Compatibility("%d\xc6\xce"), count);
			pName->AddText(text, false, true);
			pName->AddText("\n", false, true);
		}
		pForm->SetMessage(
			MakeStr(
				K2L_Compatibility("\xb0\xa1\xb1\xb8\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbf\xa9 %s \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb %d\xb0\xb3 \xc8\xb9\xb5\xe6\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				pItem->Name, count),
			false);
		pForm->Open(NULL, 2);
		return;
	}

	switch (msg.message)
	{
	case 0:
		if (g_pFresh != NULL)
			g_pFresh->OpenLayout((const char*)msg.param1, this, false);
		return;
	case M983(0x3):
	case M983(0xaa):
	case M983(0xc0):
	case M983(0xc1):
	case M983(0xd5):
	case M983(0xd6):
	case M983(0xd7):
	case M983(0xd8):
	case M983(0xd9):
	case M983(0xda):
	case M983(0xdb):
	case M983(0xdc):
	case M983(0xde):
	case M983(0xe8):
	case M983(0xe9):
		return;
	case M983(0x16):
		m_b_208 = msg.param1 != 0;
		return;
	case M983(0x17):
		*(unsigned int*)msg.param1 = m_b_208;
		return;
	case M983(0x12d):
	{
		FrLevelupItemForm* pForm = CreateForm<FrLevelupItemForm>(
			g_pFresh->GetManager(), this, "levelupitem", NULL);
		if (pForm == NULL)
			return;
		pForm->SetData((sLevelUpDone*)msg.param1);
		pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomMain::OnLevelupFormResult, 3);
		m_levelupForms.push_back(pForm);
		return;
	}
	case M983(0x48):
		if (!IsLocalContent(S4_REAL_MYROOM))
			return;
		if (m_pCPost_SendDlg_478 == NULL)
			return;
		m_pCPost_SendDlg_478->UpdateFriendList();
		return;
	case M983(0x49):
		m_hand.WaitUntilWorkDone(9999);
		ChangeCharacter(*(unsigned long*)(msg.param2 + 4));
		return;
	case M983(0x4a):
		m_hand.WaitUntilWorkDone(9999);
		ChangeCaddie(*(unsigned long*)msg.param2);
		SetLevelBar();
		return;
	case M983(0x4b):
		m_hand.WaitUntilWorkDone(9999);
		ChangeClubset(*(unsigned long*)msg.param2);
		SetLevelBar();
		return;
	case M983(0x4c):
		m_hand.WaitUntilWorkDone(9999);
		ChangeAztec(msg.param2);
		return;
	case M983(0x4e):
	{
		unsigned long* pInfo = (unsigned long*)msg.param2;
		if (Doc()->m_myInfo.userEquip.guidMascot != *pInfo)
		{
			m_hand.WaitUntilWorkDone(9999);
			ChangeMascot(*pInfo);
			Doc()->m_myInfo.userEquip.guidMascot = *pInfo;
		}
		if (m_pExhibition != NULL &&
			*(int*)((char*)m_pExhibition + 0xa8) != 0 && pInfo[1] != 0)
		{
			IFF_STRUCT::sMascot* pMascot =
				Doc()->m_itemManager.FindMascot(pInfo[1]);
			if (pMascot != NULL)
			{
				WPuppet* pPuppet = NULL;
				if (m_pExhibition != NULL)
					pPuppet = *(WPuppet**)((char*)m_pExhibition + 0xa8);
				CMascot::ChangeMascotMsgInTex((const char*)((char*)pInfo + 0xd),
					pMascot, pPuppet, MakeStr("myroom%02d"));
			}
		}
		if (*((char*)pInfo + 0x3e) != 0)
			COption::Instance()->mSetPCBangMascotMsg(
				(const char*)((char*)pInfo + 0xd));
		BuildDisplayStand((subTab_t)m_n_77C);
		return;
	}
	case M983(0x50):
	{
		if (IsLocalContent(S4_REAL_MYROOM) && m_pCPost_SendDlg_478 != NULL)
			m_pCPost_SendDlg_478->SetReceiverInfo(msg.param1,
				*(sPangYaUserInfo*)msg.param2);
		if (CUserInfo::Instance() == NULL)
			return;
		FrUserInfoForm* pForm = CUserInfo::Instance()->GetDlg();
		if (pForm == NULL)
			return;
		if (!CUserInfo::Instance()->IsVisible())
			return;
		// A858: RAW 645 +0x2554 (m_pAddFriendDlg) e info +0x16/+0x109 -> membros, como D620 na taskmain (983 +0x5d20, dwUID +0x129)
		if (!pForm->IsOpenAddFriendDlg())
			return;
		pForm->SetRequestFriendData(((sPangYaUserInfo*)msg.param2)->sNick,
			((sPangYaUserInfo*)msg.param2)->dwUID);
		return;
	}
	case M983(0x68):
	{
		if (CLoginInfo::Instance()->IsPcBang() == 0)
		{
			Reinitialize(false);
		}
		else if (GetFlags(1))
		{
			if (msg.param1 != 0)
				ReinitializePC((ePCBangItem)msg.param1);
			else
				Reinitialize(true);
		}
		Net_EquipCharacter();
		Net_EquipDresserParts();
		Net_EquipCaddie();
		Net_EquipItem();
		Net_EquipGear();
		Net_EquipMascot();
		Net_EquipCutIn983(); // PKT (frente C, lote 20): 983 00624374
		return;
	}
	case M983(0xc3):
		*(int*)msg.param1 = m_roomMode;
		*(int*)msg.param2 = m_n_77C;
		return;
	case M983(0xc4):
		if (msg.param1 < 7)
		{
			if (msg.param1 == 5)
				m_roomMode = 6;
			else
				m_roomMode = msg.param1;
		}
		else
		{
			m_roomMode = 0;
			m_n_77C = 1;
		}
		if (msg.param2 == 0)
		{
			m_n_77C = 0x15;
			return;
		}
		if (msg.param1 == 6)
		{
			m_n_77C = 0x20;
			return;
		}
		m_n_77C = msg.param2;
		return;
	case M983(0xc6):
		switch (msg.param1)
		{
		case 0:
			if (IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
			{
				SelectMenu(1);
				return;
			}
			m_roomMode = 5;
			m_n_77C = 0x11;
			ChangeMainTab((mainTab_t)5, true);
			ChangeSubTab((subTab_t)0x11, true);
			TurnOffItemStatus();
			break;
		case 1:
reset_to_character:
			m_roomMode = 0;
			m_n_77C = 0x15;
			ChangeMainTab((mainTab_t)0, true);
			ChangeSubTab((subTab_t)0x15, true);
			ChangeCharacter(Doc()->m_myInfo.userEquip.guidChar);
			return;
		case 2:
			if (!IsLocalContent(S4_REPLAY_SYSTEM))
				return;
			m_roomMode = 5;
			m_n_77C = 0x17;
			ChangeMainTab((mainTab_t)5, true);
			ChangeSubTab((subTab_t)0x17, true);
			break;
		case 3:
			if (!IsLocalContent(S4_UCC))
				return;
			m_roomMode = 0;
			m_n_77C = 0x1a;
			ChangeMainTab((mainTab_t)0, true);
			ChangeSubTab((subTab_t)0x1a, true);
			ChangeCharacter(Doc()->m_myInfo.userEquip.guidChar);
			return;
		case 4:
			if (!IsLocalContent(S4_CARD_SYSTEM))
				return;
			m_roomMode = 1;
			m_n_77C = 0x1b;
			ChangeMainTab((mainTab_t)1, true);
			ChangeSubTab((subTab_t)0x1b, true);
			break;
		case 5:
			if (!IsLocalContent(S4_REAL_MYROOM))
				return;
			m_roomMode = 6;
			m_n_77C = 0x20;
			ChangeMainTab((mainTab_t)6, true);
			ChangeSubTab((subTab_t)0x20, true);
			m_hand.WaitUntilWorkDone(9999);
			if (m_pExhibition == NULL)
			{
				OnTabMainButtonUp();
				return;
			}
			delete m_pExhibition;
			m_pExhibition = NULL; // 983 00605530: apaga e zera [this+0x584] (D773)
			return;
		case 6:
			if (IsLocalContent(S4_ITEM_STORAGE))
			{
				SelectMenu(0xb);
				m_roomMode = 0;
				m_n_77C = 0x15;
				ChangeMainTab((mainTab_t)0, true);
				ChangeSubTab((subTab_t)0x15, true);
				ChangeCharacter(Doc()->m_myInfo.userEquip.guidChar);
				return;
			}
			goto reset_to_character;
		default:
			return;
		}
		m_hand.WaitUntilWorkDone(9999);
		if (m_pExhibition == NULL)
			return;
		delete m_pExhibition;
		m_pExhibition = NULL; // 983 00605530: apaga e zera [this+0x584] (D773)
		return;
	case M983(0xc7):
		// 983: fecha a janela de opções (CTaskMain+0x154)
		C_ASSERT(offsetof(CRealMyRoomMain, m_pOptionDlg) == 0x154);
		if (m_pOptionDlg == NULL)
			return;
		(*(FrForm**)&m_pOptionDlg)->Close((eFormRet)1, true);
		m_pOptionDlg = NULL;
		return;
	case M983(0xc8):
	{
		sCharacterInfo info;
		info.guid = msg.param1;
		info.tid = msg.param2;
		info.gift_flag = 1;
		Doc()->m_itemManager.GetDefCombo(info.tid, info.tidParts);
		Doc()->m_charMap.insert(
			std::pair<const unsigned int, sCharacterInfo>(info.guid, info));
		return;
	}
	case M983(0xc9):
	{
		unsigned long tidChar = Doc()->m_myInfo.userEquip.guidChar;
		Doc()->m_charMap[tidChar];
		IFF_STRUCT::sHairStyle* pHair =
			Doc()->m_itemManager.FindHairStyle(msg.param2);
		if (pHair == NULL)
			return;
		std::map<unsigned int, sCharacterInfo>::iterator it;
		for (it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
		{
			if ((*it).second.tid == (pHair->cCharID | 0x4000000))
			{
				(*it).second.hairClr = pHair->cHairID;
				break;
			}
		}
		if (!IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
			return;
		CPartTidList& tids = Doc()->GetMyPartTidList();
		(*(CPetFrame**)((char*)m_pExhibition + 0xa4))
			->SetHairClr(&tids, pHair->cHairID);
		return;
	}
	case M983(0xcb):
	{
		sCaddieInfo caddie;
		memset(&caddie, 0, sizeof(caddie));
		caddie.guid = msg.param1;
		caddie.tid = msg.param2;
		caddie.gift_flag = 1;
		caddie.Rent_flag = 1;
		caddie.Remain_Date = 0x1e;
		if (caddie.tid > 0x1c00000b && caddie.tid < 0x1c00000f)
		{
			caddie.Level = (unsigned char)msg.param5;
			caddie.Exp = msg.param4;
		}
		Doc()->m_caddieMap.insert(
			std::pair<const unsigned int, sCaddieInfo>(msg.param1, caddie));
		return;
	}
	case M983(0xcc):
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			Doc()->m_caddieMap.find(msg.param1);
		if (it == Doc()->m_caddieMap.end())
			return;
		(*it).second.tidPart = msg.param2;
		(*it).second.Remain_Partdate = (unsigned short)msg.param3;
		if (msg.param3 > (*it).second.Remain_Date)
			(*it).second.Remain_Date = (unsigned short)msg.param3;
		return;
	}
	case M983(0xcd):
	{
		sMascotInfo mascot;
		memset(&mascot, 0, sizeof(mascot));
		mascot.guid = msg.param1;
		mascot.tid = msg.param2;
		strcpy(mascot.szMsg, "PANGYA!");
		mascot.Remain_Date = ((sBuyItemResult*)msg.param5)->Time;
		memcpy(&mascot.endDate, &((sBuyItemResult*)msg.param5)->endDate, sizeof(_SYSTEMTIME));
		Doc()->AddMascot(&mascot);
		return;
	}
	case M983(0xce):
	{
		unsigned long tid = msg.param2;
		if (tid == 0x1a000010)
		{
			*(__int64*)DOCPTR983(0x91e) += msg.param3;
			return;
		}
		unsigned short count = 0;
		int period = 0;
		unsigned char type = 0;
		std::list<sItemInfo>* pList;
		switch (tid >> 26)
		{
		case 5:
			count = ((sBuyItemResult*)msg.param5)->Count;
			period = msg.param3;
			pList = &Doc()->m_ballList;
			type = (unsigned char)msg.param4;
			break;
		case 6:
			count = ((sBuyItemResult*)msg.param5)->Count;
			period = msg.param3;
			pList = &Doc()->m_myItemList;
			type = (unsigned char)msg.param4;
			break;
		case 0x1c:
		{
			unsigned long* pAux = (unsigned long*)msg.param5;
			if (Doc()->m_itemManager.FindAuxPart(*pAux) == NULL)
				return;
			count = ((sBuyItemResult*)pAux)->Count;
			pList = &Doc()->m_auxPartList;
			break;
		}
		case 0x1d:
			if (Doc()->m_itemManager.FindQuestDrop(tid) == NULL)
				return;
			count = 1;
			pList = &Doc()->m_questDropList;
			break;
		default:
			return;
		}
		if (msg.param4 == 0)
		{
			std::list<sItemInfo>::iterator it;
			for (it = pList->begin(); it != pList->end(); ++it)
			{
				if ((*it).guid != (unsigned long)msg.param1)
					continue;
				int add = 0;
				if (type != 0)
				{
					add = period;
					if (type != 3)
					{
						if (type == 4)
							add = period * 0x18;
						else
						{
							add = 0;
							if (type == 5)
								add = period * 0x2d0;
						}
					}
					Net_StartPeriodItem((*it).guid);
				}
				(*it).HourRemain += add;
				(*it).Common[0] = count;
				if ((*it).ItemType != 0 && (*it).HourRemain > 0)
					Net_StartPeriodItem((*it).guid);
				BuildDisplayStand((subTab_t)m_n_77C);
				return;
			}
		}
		sItemInfo item;
		item.guid = msg.param1;
		item.tid = msg.param2;
		item.Common[0] = 1;
		if (count != 0)
			item.Common[0] = count;
		item.gift_flag = 1;
		item.ItemType = type;
		if (type != 0)
		{
			if (type == 3)
				item.HourRemain = period;
			else if (type == 4)
				item.HourRemain = period * 0x18;
			else if (type == 5)
				item.HourRemain = period * 0x2d0;
			Net_StartPeriodItem(item.guid);
			item.ItemType = 2;
		}
		pList->push_back(item);
		BuildDisplayStand((subTab_t)m_n_77C);
		return;
	}
	case M983(0xcf):
	{
		sItemInfo item;
		item.guid = msg.param1;
		item.tid = msg.param2;
		Doc()->m_clubSetMap.insert(
			std::pair<const unsigned int, sItemInfo>(item.guid, item));
		return;
	}
	case M983(0xd0):
	{
		sItemInfo item;
		item.guid = msg.param1;
		item.ItemType = msg.param4;
		item.HourRemain = msg.param3 * 0x18;
		item.tid = msg.param2;
		Doc()->m_cutinList.push_back(item);
		return;
	}
	case M983(0xd1):
	{
		int guid = msg.param1;
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pFrListBox_304->m_itemList.begin();
			it != m_pFrListBox_304->m_itemList.end(); ++it)
		{
			int* pData = (int*)(*it)->pData;
			if (pData == NULL)
				break;
			if (*pData == guid)
			{
				m_pFrListBox_304->_DelItem(it);
				break;
			}
		}
		m_pFrListBox_304->SortItem(EmptyCompares);
		for (std::list<sGiftInfo>::iterator git = Doc()->m_giftList.begin();
			git != Doc()->m_giftList.end(); ++git)
		{
			if ((*git).guid == (unsigned long)guid)
			{
				Doc()->m_giftList.erase(git);
				break;
			}
		}
		m_b_C25 = 1;
		return;
	}
	case M983(0xd2):
		if (m_pFrEnchantDlg_428 != NULL)
			m_pFrEnchantDlg_428->SetResultCode((unsigned char)msg.param1);
		SetLevelBar();
		return;
	case M983(0xd3):
		if (m_pFrDowngradeDlg_42C != NULL)
			m_pFrDowngradeDlg_42C->SetLevelBar();
		if (m_pFrDowngradeDlg_430 != NULL)
			m_pFrDowngradeDlg_430->SetLevelBar();
		SetLevelBar();
		return;
	case M983(0xd4):
		if (m_pFrEnchantDlg_428 != NULL)
			return;
		m_pFrEnchantDlg_428 = CreateForm<FrEnchantDlg>(g_pFresh->GetManager(),
			this, "enchant", NULL);
		m_pFrEnchantDlg_428->SetType((unsigned char)msg.param1,
			(unsigned char)msg.param2);
		m_pFrEnchantDlg_428->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnEnchantDlgResult, 1);
		return;
	case M983(0xdf):
		Net_EquipDresserParts();
		Net_EquipCharacter();
		Net_EquipCaddie();
		Net_EquipItem();
		Net_EquipGear();
		Net_EquipSkin();
		Net_EquipMascot();
		Net_EquipCutIn983(); // PKT (frente C, lote 20): 983 006243b1
		return;
	case M983(0xe0):
	{
		unsigned long tidCard = msg.param2;
		unsigned long guid = msg.param1;
		if (Doc()->m_itemManager.FindCard(tidCard) == NULL)
			return;
		unsigned short count = ((sBuyItemResult*)msg.param5)->Count;
		std::list<sCards>::iterator it;
		for (it = Doc()->m_cardList.begin(); it != Doc()->m_cardList.end();
			++it)
		{
			if ((*it).tid == tidCard)
			{
				(*it).cardCount += count;
				return;
			}
		}
		sCards card;
		memset(&card, 0, sizeof(card));
		card.uid = guid;
		card.tid = tidCard;
		card.cardCount = count;
		Doc()->m_cardList.push_back(card);
		return;
	}
	case M983(0xe1):
		BuildDisplayStand((subTab_t)m_n_77C);
		return;
	case M983(0xdd):
		BuildDisplayStand((subTab_t)m_n_77C);
		return;
	case M983(0x1fc):
	case M983(0x211):
	case M983(0x232):
		return;
	case M983(0x204):
		if (IsLocalContent((localContentType_t)0x14))
		{
			*(unsigned long*)msg.param1 = m_n_7D0;
			m_n_7D0 = 0;
		}
		return;
	case M983(0x209):
		m_n_284 = msg.param1;
		if (msg.param1 == 1)
		{
			m_f274 = (float)msg.param2;
			m_f278 = (float)msg.param3;
		}
		else if (msg.param1 == 2)
		{
			m_f27c = (float)msg.param2;
			if (m_f27c < -90.0f)
				m_f27c = -90.0f;
			m_f280 = (float)msg.param3;
			if (45.0f < m_f280)
				m_f280 = 45.0f;
		}
		return;
	case M983(0x20a):
	{
		WVector pos;
		pos.x = (float)msg.param1;
		pos.z = (float)msg.param2;
		char name[32];
		strcpy(name, (const char*)msg.param3);
		pos.y = 0.0f;
		WPolySoup* pSoup = *(WPolySoup**)((char*)Doc()->m_pGolfDoc + 8); // A729: membro (cl novo 0x10cc4)
		pSoup->SetAdditionModel(name, pos);
		pSoup->EndLoad(false);
		return;
	}
	case M983(0x20b):
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			if (m_b_D45 != 0)
				ChangeSubTab((subTab_t)0x15, false);
			if (Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
				SetMiddleUIControl(m_b_D45 == 0);
			CharacterViewControl(m_b_D45 != 0);
		}
		// fall through
	case M983(0x220):
		if (IsLocalContent(S4_REAL_MYROOM) &&
			m_pCRealMyRoom_AuthorityDlg_47C != NULL)
		{
			delete m_pCRealMyRoom_AuthorityDlg_47C;
			m_pCRealMyRoom_AuthorityDlg_47C = NULL;
		}
		return;
	case M983(0x21a):
		if (m_pCardBook != NULL && m_pCardBook->IsShow() == true)
			m_pCardBook->ReBuildCurrentPage();
		BuildCardPackList();
		return;
	case M983(0x229):
		if (m_pCardBook != NULL)
			m_pCardBook->ReBuildCurrentPage();
		BuildCardPackList();
		return;
	case M983(0x22a):
		*(int*)DOCPTR983(0x919) = msg.param1;
		*DOCPTR983(0x91d) = (char)msg.param2;
		return;
	case M983(0x22b):
		g_camera.xm = (float)msg.param1;
		g_camera.ym = (float)msg.param2;
		g_camera.zm = (float)msg.param3;
		return;
	case M983(0x22f):
		if (!g_pFresh->IsCurrentLayout("REALMYROOM"))
			return;
		SetListMenu();
		BuildDisplayStand((subTab_t)m_n_77C);
		return;
	case M983(0x233):
		if (IsLocalContent(S4_ITEM_STORAGE))
			SetItemStorageVisible(true);
		return;
	case M983(0x234):
		if (IsLocalContent(S4_ITEM_STORAGE))
		{
			SetItemStorageVisible(false);
			if (m_pFrItemStorageAccessPanel_D94 != NULL)
				m_pFrItemStorageAccessPanel_D94->Close((eFormRet)2, true);
		}
		return;
	case M983(0x235):
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenItemStorageAccessPanel(1);
		return;
	case M983(0x236):
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenItemStorageNotice();
		return;
	case M983(0x237):
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenItemStorageAccessPanel(2);
		return;
	case M983(0x238):
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenItemStorageAccessPanel(3);
		return;
	case M983(0x239):
		if (IsLocalContent(S4_ITEM_STORAGE))
			OpenItemStorageAccessPanel(4);
		return;
	case M983(0x23a):
		if (IsLocalContent(S4_ITEM_STORAGE))
		{
			AfxGetTask()->GetActor("RealMyRoom")
				<< MsgObject(NULL, M983(0x233), 0, 0, 0, 0, 0);
			if (m_b_D98 == 1)
				BuildDisplayStand((subTab_t)m_n_77C);
		}
		return;
	case M983(0x23b):
	{
		CItemStorage* pStorage = CItemStorage::Instance();
		if (IsLocalContent(S4_ITEM_STORAGE) && pStorage != NULL &&
			m_pFrListBox_DB4 != NULL)
		{
			pStorage->InitIndexControl(pStorage->GetTotalPage(), 5);
			CIndexControl* pIndex = pStorage->GetIndexControl();
			if (pIndex != NULL)
				pIndex->SelectIndex(pIndex->GetCurIndexCount());
		}
		return;
	}
	case M983(0x23c):
	{
		CItemStorage* pStorage = CItemStorage::Instance();
		if (IsLocalContent(S4_ITEM_STORAGE) && pStorage != NULL &&
			m_pFrListBox_DB4 != NULL)
		{
			pStorage->InitIndexControl(pStorage->GetTotalPage(), 5);
			if (pStorage->GetIndexControl() != NULL)
				pStorage->GetIndexControl()->SelectIndex(0);
		}
		return;
	}
	case M983(0x23d):
		if (IsLocalContent(S4_ITEM_STORAGE) == true && m_b_D98 == true &&
			CItemStorage::Instance())
		{
			unsigned long page = CItemStorage::Instance()->GetCurPage();
			WSendPacket packet((enumClientPacket)0xcd); // JP 983 (s63): 0xcf no KR 645
			packet.Encode4(99);
			packet.Encode2((unsigned char)page);
			packet.Send(TO_GAME);
		}
		return;
	case M983(0x23e):
		if (IsLocalContent(S4_ITEM_STORAGE) == true && m_b_D98 == true &&
			CItemStorage::Instance())
		{
			unsigned long page = CItemStorage::Instance()->GetCurPage();
			if (page == 0)
				page = 1;
			WSendPacket packet((enumClientPacket)0xcd); // JP 983 (s63): 0xcf no KR 645
			packet.Encode4(99);
			packet.Encode2((unsigned char)page);
			packet.Send(TO_GAME);
		}
		return;
	case M983(0x23f):
		if (msg.param1 == 0)
		{
			if (msg.param2 == 0x17b)
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\xc6\xd0\xbd\xba\xbf\xf6\xb5\xe5\xb0\xa1 \xbc\xb3\xc1\xa4\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
					0, 0, 0, 0));
		}
		else if (msg.param1 == 0x64)
		{
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)"\xbd\xc7\xc6\xd0", 0, 0, 0, 0));
		}
		else if (msg.param1 == 0x65)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x66)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.", 0, 0,
				0, 0));
		}
		else if (msg.param1 == 0x67 || msg.param1 == 0x68 || msg.param1 == 0x6a)
			return;
		else if (msg.param1 == 0x6b)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc1\xa4\xbb\xf3 \xc3\xb3\xb8\xae \xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			AfxGetTask()->GetActor("RealMyRoom")
				<< MsgObject(NULL, M983(0x234), 0, 0, 0, 0, 0);
		}
		else if (msg.param1 == 0x6c)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc3\xb3\xb8\xae\xc7\xd2\xbc\xf6 \xc0\xd6\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb \xc3\xd6\xb4\xeb \xb0\xb3\xbc\xf6\xb8\xa6 \xc3\xca\xb0\xfa\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x6d)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc3\xa2\xb0\xed\xbf\xa1 \xba\xb8\xb0\xfc\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x6e)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb1\xe2\xb0\xa3\xc1\xa6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xc3\xa2\xb0\xed\xbf\xa1 \xb3\xd6\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x6f)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc0\xd4\xb7\xc2\xc7\xcf\xbd\xc5 \xb1\xdd\xbe\xd7\xc0\xcc \xba\xb8\xc0\xaf\xc7\xd1 \xb1\xdd\xbe\xd7\xba\xb8\xb4\xd9 \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x70)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0));
			AfxGetTask()->GetActor("RealMyRoom")
				<< MsgObject(NULL, M983(0x234), 0, 0, 0, 0, 0);
		}
		else if (msg.param1 == 0x71)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc3\xa2\xb0\xed\xb0\xa1 \xc0\xcc\xb9\xcc \xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x72)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xba\xb8\xc0\xaf\xc1\xdf\xc0\xce \xc3\xa2\xb0\xed\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x73)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc6\xed\xc1\xfd\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."), 0, 0,
				0, 0));
		}
		else if (msg.param1 == 0x74)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\xc3\xa2\xb0\xed\xb0\xa1 \xc0\xe1\xb1\xdd\xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0));
			AfxGetTask()->GetActor("RealMyRoom")
				<< MsgObject(NULL, M983(0x234), 0, 0, 0, 0, 0);
		}
		else if (msg.param1 == 0x75)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc6\xd0\xbd\xba\xbf\xf6\xb5\xe5\xb0\xa1 \xc6\xb2\xb7\xc8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x69)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc6\xd0\xbd\xba\xbf\xf6\xb5\xe5\xb4\xc2 \xbc\xfd\xc0\xda 4\xc0\xda\xb8\xae\xb7\xce\xb8\xb8 \xbc\xb3\xc1\xa4 \xb0\xa1\xb4\xc9 \xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x76)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb \xbc\xf6\xb7\xae \xbf\xa1\xb7\xaf"),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x77)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc0\xd4\xb7\xc2\xc7\xcf\xbd\xc5 \xb1\xdd\xbe\xd7\xc0\xcc \xba\xb8\xc0\xaf\xc7\xd1 \xb1\xdd\xbe\xd7\xba\xb8\xb4\xd9 \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x78)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"1\xc8\xb8 \xc0\xd4,\xc3\xe2\xb1\xdd \xc7\xd2\xbc\xf6\xc0\xd6\xb4\xc2 \xb9\xfc\xc0\xa7\xb8\xa6 \xc3\xca\xb0\xfa\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x79)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)"\xbf\xac\xbc\xd3\xb5\xc8 \xbc\xfd\xc0\xda\xb7\xce \xbc\xb3\xc1\xa4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0));
		}
		else if (msg.param1 == 0x7a)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)" \xb8\xb6\xc0\xcc\xb7\xeb \xc3\xd6\xb4\xeb \xba\xb8\xc0\xaf \xbc\xf6\xb7\xae\xc0\xbb \xc3\xca\xb0\xfa\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0));
		}
		return;
	case M983(0x242):
		if (IsInItemList(msg.param1))
			*(unsigned char*)msg.param2 = 1;
		else
			*(unsigned char*)msg.param2 = 0;
		return;
	case 0x2bf: // 983 (era M983(0x244)): reabilita botoes do correio apos 0x0211/0x0212 (C157.1)
	case M983(0x244):
		if (m_pCPost_ReciveDlg_474 != NULL)
			m_pCPost_ReciveDlg_474->EnableMiniBtns(msg.param1 != 0);
		return;
	case M983(0x212):
		if (m_pFrUccDrawDlg_45C != NULL)
			m_pFrUccDrawDlg_45C->Upload();
		return;
	case M983(0x213):
	{
		unsigned long typeId = *(unsigned long*)msg.param1;
		const char* uccIndex = (const char*)msg.param2;
		const char* errCode = (const char*)msg.param3;
		const char* errNames[6] = { "", "ERR_FILE", "ERR_UCC", "ERR_KEY",
			"ERR_USER", "ERR_SRVVAR" };
		int errIdx = 0;
		for (int i = 0; i < 6; i++)
		{
			if (strcmpi(errNames[i], errCode) == 0)
			{
				errIdx = i;
				break;
			}
		}
		if (m_pFrUccDrawDlg_45C != NULL &&
			m_pFrUccDrawDlg_45C->m_pItemInfo->tid == typeId &&
			strcmp(m_pFrUccDrawDlg_45C->m_pItemInfo->UccIndex, uccIndex) == 0)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				MakeStr(
					K2L_Compatibility("\xbe\xf7\xb7\xce\xb5\xe5\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.(\xc4\xda\xb5\xe5:%d)"),
					errIdx + 10),
				false);
			pForm->Open(NULL, 1);
			m_pFrUccDrawDlg_45C->CloseWaitingForm();
		}
		return;
	}
	case M983(0x214):
	{
		bool bSuccess = msg.param1 != 0;
		unsigned long id = *(unsigned long*)msg.param2;
		unsigned long typeId = *(unsigned long*)msg.param3;
		const char* uccIndex = (const char*)msg.param4;
		const char* uccName = (const char*)msg.param5;
		if (m_pFrUccDrawDlg_45C == NULL)
			return;
		if (m_pFrUccDrawDlg_45C->m_pItemInfo->tid != typeId)
			return;
		if (strcmp(m_pFrUccDrawDlg_45C->m_pItemInfo->UccIndex, uccIndex) != 0)
			return;
		m_pFrUccDrawDlg_45C->m_bWaiting = false;
		if (bSuccess)
		{
			for (int i = 0; i < 2; i++)
			{
				char* pClothes = (char*)m_pFrUccDrawDlg_45C->m_pClothes[i];
				Bitmap* pBitmap = *(Bitmap**)(pClothes + 8);
				const char* texName = pClothes + 0x8c;
				w_texlist* pTex = g_resrcmng->FindTexture(texName);
				if (pTex == NULL)
					g_resrcmng->UploadTexture(texName, pBitmap, 0, NULL);
				else
					g_resrcmng->FixTexture(pTex->texhandle, pBitmap, 0, NULL);
			}
			sUccClothes clothes;
			GetSystemTime(&clothes.date);
			clothes.id = id;
			clothes.downloadCount = 1;
			clothes.typeId = typeId;
			strncpy(clothes.uccIndex, uccIndex, 9);
			strncpy(clothes.uccName, uccName, 0x29);
			std::map<std::string, Bitmap*>::iterator itIcon =
				UccManager()->m_icon.find(
					GenerateIconFilename(typeId, uccIndex));
			if (itIcon != UccManager()->m_icon.end())
				clothes.icon = itIcon->second;
			UccManager()->AddClothes(1, clothes, true, false);
			std::list<sItemInfo>& itemList = Doc()->m_uccItemList;
			for (std::list<sItemInfo>::iterator it = itemList.begin();
				it != itemList.end(); ++it)
			{
				if (it->tid == typeId && strcmp(it->UccIndex, uccIndex) == 0)
				{
					it->status = (it->status & 0xfd) | 1;
					break;
				}
			}
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce \xc0\xc7\xbb\xf3\xc0\xcc \xbf\xcf\xbc\xba\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 1);
			m_pFrUccDrawDlg_45C->Close((eFormRet)1, true);
			return;
		}
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce \xc0\xc7\xbb\xf3 \xb5\xee\xb7\xcf\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 1);
		return;
	}
	case M983(0x219):
	{
		bool bSuccess = msg.param2 == 1;
		unsigned long tid = msg.param1;
		// D892: o 983 (00623500 case 0x296) nao pisca botao aqui; o bloco do 645 lia this+0x48/+0x15c e
		// CCardManager+0xd48 (offsets do 645: no nosso layout = m_caddieCard, nos dois cl)
		if (!bSuccess)
		{
			if (m_pFrCardUseDlg_44C == NULL)
			{
				FrCardUseDlg* pDlg = CreateForm<FrCardUseDlg>(
					g_pFresh->GetManager(), this, "usecard_result", NULL);
				pDlg->m_tid = tid; // 983 piVar8[0x45]
				pDlg->Open(NULL, 0);
			}
			if (m_pCardBook != NULL)
				m_pCardBook->ReBuildCurrentPage();
		}
		return;
	}
	// 983 (pedido C32, s102): 0x221/0x223/0x224/0x225/0x226 (0x29f/0x2a1..0x2a4 do 983) sairam: no 983 o switch deste HandleMsg
	// (00623500, tabela 00626820/0062677c, faixa 0x279..0x320) nao os tem; o correio e do CTaskMain::HandleMsgCommon.
	case M983(0x228):
	{
		unsigned long id = msg.param1;
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		const char* text = NULL;
		switch (id)
		{
		case 0x122:
			if (msg.param2 != 0)
				text =
					"\xb8\xb6\xc0\xcc\xb7\xeb\xc0\xc7 \xb1\xc7\xc7\xd1\xc0\xbb \xbc\xb3\xc1\xa4\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
			else
				text =
					"\xb8\xb6\xc0\xcc\xb7\xeb\xc0\xc7 \xb1\xc7\xc7\xd1\xc0\xbb \xbc\xb3\xc1\xa4\xc7\xcf\xb4\xc2\xb5\xa5 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
			break;
		case 0x123:
			text =
				K2L_Compatibility("\xb8\xb6\xc0\xcc\xb7\xeb\xbf\xa1 \xc0\xd6\xb4\xc2 \xb0\xa1\xb1\xb8 \xc1\xa4\xba\xb8\xb8\xa6 \xb0\xa1\xc1\xae\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x124:
		{
			int result = msg.param2;
			delete pForm;
			FrForm* pSaveForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			if (result == 0)
				pSaveForm->SetMessage(
					K2L_Compatibility("\xb8\xb6\xc0\xcc\xb7\xeb\xbf\xa1 \xb0\xa1\xb1\xb8\xb8\xa6 \xb3\xf5\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.. "),
					false);
			else
				pSaveForm->SetMessage(
					K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb9\xe8\xc4\xa1\xb8\xa6 \xc0\xfa\xc0\xe5\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
			pSaveForm->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnFunritureSaveMsgdlgResult,
				3);
			break;
		}
		case 0x125:
			text =
				K2L_Compatibility("\xb8\xb6\xc0\xcc\xb7\xeb\xbf\xa1 \xb5\xe9\xbe\xee\xb0\xa5 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x13f:
			switch (msg.param2)
			{
			case 1:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc6\xed\xc1\xf6 \xbc\xf6\xbd\xc5\xc0\xbb \xb0\xc5\xba\xce\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			case 2:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xc0\xaf\xc0\xfa\xb8\xa6 \xc3\xa3\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			case 3:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc0\xcc\xb9\xcc \xbc\xd2\xc0\xaf\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9.");
				break;
			case 4:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbc\xd2\xc0\xaf\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb \xbc\xf6\xb7\xae\xba\xb8\xb4\xd9 \xb4\xf5 \xb8\xb9\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xba\xb8\xb3\xbb\xb7\xc1\xb0\xed \xbd\xc3\xb5\xb5\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9..");
				break;
			case 5:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbc\xd2\xc0\xaf\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xba\xb8\xb3\xbb\xb7\xc1\xb0\xed \xbd\xc3\xb5\xb5\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			case 6:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc0\xcc\xb9\xcc \xc6\xed\xc1\xf6\xb7\xce \xb9\xde\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9.");
				break;
			case 7:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc3\xb7\xba\xce\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			case 8:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xc6\xed\xc1\xf6 \xb9\xdf\xbc\xdb\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			case 9:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xc7 \xbf\xec\xc6\xed\xc7\xd4\xc0\xcc \xb0\xa1\xb5\xe6 \xc2\xf7\xbc\xad \xc7\xf6\xc0\xe7\xb4\xc2 \xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xb9\xde\xc0\xbb \xbc\xf6 \xbe\xf8\xb4\xc2 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9.");
				break;
			case 10:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xc7\xd4 \xb1\xe2\xb4\xc9\xc0\xc7 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.");
				break;
			case 0x16:
				text =
					K2L_Compatibility("\xb8\xde\xc0\xcf \xc3\xb7\xba\xce \xb0\xa1\xb4\xc9\xc7\xd1 \xc3\xd6\xb4\xeb \xb0\xb3\xbc\xf6\xb8\xa6 \xc3\xca\xb0\xfa\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			default:
				text =
					K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbb\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			}
			break;
		case 0x141:
			text =
				K2L_Compatibility("\xbf\xec\xc6\xed\xc7\xd4\xc0\xc7 \xc6\xed\xc1\xf6\xb5\xe9\xc0\xbb \xc0\xd0\xbe\xee\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x143:
			text =
				K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xc7 \xb3\xbb\xbf\xeb\xc0\xbb \xc0\xd0\xbe\xee\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x145:
			text =
				"\xbf\xec\xc6\xed\xb9\xb0\xbf\xa1 \xc3\xb7\xba\xce\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xc5\xb1\xe2\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
			break;
		case 0x15d:
			if (msg.param2 == 9)
				text =
					K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc6\xf7\xc7\xd4\xb5\xc7\xbe\xee \xc0\xd6\xbe\xee \xc1\xf6\xbf\xec\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			else
				text =
					"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xc1\xf6\xbf\xec\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
			break;
		}
		if (text != NULL)
			pForm->SetMessage(text, false);
		if (id != 0x124)
			pForm->Open(NULL, 0);
		AfxGetTask()->GetActor("RealMyRoom")
			<< MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
		return;
	}
	case M983(0x266):
		UnselectItemList((unsigned long)msg.param1);
		return;
	case M983(0x267):
		SelectItemList((unsigned long)msg.param1);
		return;
	case M983(0x26c):
		m_mapChar = // era this+0x968 = layout do 645
			Doc()->m_charMap;
		return;
	default:
		CTaskMain::HandleMsgCommon(msg);
		return;
	}
}

bool CRealMyRoomMain::OnReplayModeOpenDlgResult(int result, FrForm* pForm)
{
	if (0) // HACK: emission order (the original waits for an inline chain that leaves no reloc; anchor of the previous wave)
		OnCardPackDlgResult(0, NULL);
	m_pFrReplayModeDlg_448 = NULL;
	if (result == 1)
	{
		memset(&Doc()->m_roomInfo, 0, sizeof(sRoomInfo));
		Doc()->m_playingMode = (eReplayModeType)2;
		Doc()->m_golfGame.gameType = 0;
		strcpy(Doc()->m_golfGame.gameTypeName, Doc()->m_gameTypeInfo[0].name);
		CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);
	}
	return true;
}

bool CRealMyRoomMain::OnReplayDelOpenDlgResult(int result, FrForm* pForm)
{
	m_pFrReplayModeDlg_448 = NULL;
	if (result == 1)
		BuildDisplayStand((subTab_t)0x17);
	return true;
}

void CRealMyRoomMain::OnFurniturePupInit(int param)
{
	m_pFrButton_3B8 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_3B8)
		m_pFrButton_3B8->SetVisible(false);
}

void CRealMyRoomMain::OnFurniturePupButtonUp()
{
	m_n_7A4--;
	int nMax = m_n_7A8 - 1;
	m_n_7A4 = (m_n_7A4 < 0) ? 0 : ((m_n_7A4 > nMax) ? nMax : m_n_7A4);
	m_pFrEdit_3C0->ClearLine();
	m_pFrEdit_3C4->ClearLine();
	m_pFrEdit_3C0->AddText(MakeStr("%3d", m_n_7A4 + 1), false, true);
	m_pFrEdit_3C4->AddText(MakeStr("%3d", m_n_7A8), false, true);
	BuildDisplayStandFurniture();
}

void CRealMyRoomMain::OnFurniturePdownInit(int param)
{
	m_pFrButton_3BC = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_3BC)
		m_pFrButton_3BC->SetVisible(false);
}

void CRealMyRoomMain::OnFurniturePdownButtonUp()
{
	m_n_7A4++;
	int nMax = m_n_7A8 - 1;
	m_n_7A4 = (m_n_7A4 < 0) ? 0 : ((m_n_7A4 > nMax) ? nMax : m_n_7A4);
	m_pFrEdit_3C0->ClearLine();
	m_pFrEdit_3C4->ClearLine();
	m_pFrEdit_3C0->AddText(MakeStr("%3d", m_n_7A4 + 1), false, true);
	m_pFrEdit_3C4->AddText(MakeStr("%3d", m_n_7A8), false, true);
	BuildDisplayStandFurniture();
}

void CRealMyRoomMain::OnFurnitureListNumInit(int param)
{
	m_pFrEdit_3C0 = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pFrEdit_3C0)
	{
		m_pFrEdit_3C0->SetCharLimit(2, false);
		m_pFrEdit_3C0->SetVisible(false);
	}
}

void CRealMyRoomMain::OnFurnitureListNumBtnUp()
{
	if (m_pFrEdit_3C0->IsVisible())
		m_pFrEdit_3C0->SetKeyEvent(true);
}

bool CRealMyRoomMain::OnFurnitureListEnterKey(int key)
{
	OnFurniture_BtnListNumBtnUp();
	return false;
}

void CRealMyRoomMain::OnFurnitureListMaxNumInit(int param)
{
	m_pFrEdit_3C4 = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pFrEdit_3C4)
		m_pFrEdit_3C4->SetVisible(false);
}

void CRealMyRoomMain::OnFurniture_BtnListNumInit(int param)
{
	m_pFrButton_3C8 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_3C8)
		m_pFrButton_3C8->SetVisible(false);
}

void CRealMyRoomMain::OnFurniture_BtnListNumBtnUp()
{
	m_n_7A4 = atoi(m_pFrEdit_3C0->GetLine(1, false)) - 1;
	int nMax = m_n_7A8 - 1;
	m_n_7A4 = (m_n_7A4 < 0) ? 0 : ((m_n_7A4 > nMax) ? nMax : m_n_7A4);
	m_pFrEdit_3C0->ClearLine();
	m_pFrEdit_3C4->ClearLine();
	m_pFrEdit_3C0->AddText(MakeStr("%3d", m_n_7A4 + 1), false, true);
	m_pFrEdit_3C4->AddText(MakeStr("%3d", m_n_7A8), false, true);
	if (m_pFrEdit_3C0->IsVisible())
		m_pFrEdit_3C0->SetKeyEvent(true);
	BuildDisplayStandFurniture();
}

void CRealMyRoomMain::OnFurnitureResetInit(int param)
{
	m_pFrButton_3CC = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_3CC)
		m_pFrButton_3CC->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureResetButtonUp()
{
	CRealMyRoomData::Instance()->ResetList(false);
	std::list<sFurniture_List>::iterator it;
	for (it = Doc()->m_furnitureList.begin();
		it != Doc()->m_furnitureList.end(); ++it)
		(*it).bArrange = 0;
	BuildDisplayStandFurniture();
}

void CRealMyRoomMain::OnFurnitureBackInit(int param)
{
	m_pFrButton_3D0 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_3D0)
		m_pFrButton_3D0->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackButtonUp()
{
	std::list<sFurniture_List> backup;
	std::list<sFurniture_List>::iterator it;
	for (it = Doc()->m_furnitureList.begin();
		it != Doc()->m_furnitureList.end(); ++it)
		(*it).bArrange = 0;
	CRealMyRoomData::Instance()->ResotreList();
	BuildDisplayStandFurniture();
}

void CRealMyRoomMain::OnFurnitureSaveInit(int param)
{
	m_pFrButton_3D4 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_3D4)
		m_pFrButton_3D4->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureSaveButtonUp()
{
	if (Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
	{
		*(WMatrix*)((char*)m_n_C4C + 0x20) = g_camera;
		*(float*)((char*)m_n_C4C + 0x50) = g_view->GetFOV(); // 983: GetFOV de verdade (00b4fa30)
		m_clipNear = g_view->GetClipNearValue();
		m_clipFar = g_view->GetClipFarValue();
		SetCameraDisplayType(2);
		if (CRealMyRoomData::Instance()->SaveModel(false) &&
			Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
		{
			RMR_UpdateDB();
			CRealMyRoomData::Instance()->SaveModel(true);
		}
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			MakeStr(
				"\xc7\xf6\xc0\xe7\xb4\xc2 \xc0\xfa\xc0\xe5\xc7\xd2\xbc\xf6 \xbe\xf8\xb4\xc2 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 3);
	}
}

void CRealMyRoomMain::OnFurnitureOutInit(int param)
{
	m_pFrButton_3D8 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_3D8)
		m_pFrButton_3D8->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureOutButtonUp()
{
	CFxSequence* pSeq = m_pFxSeq;
	if (pSeq->m_bActive)
	{
		pSeq->m_bActive = false;
		for (std::map<CFxSpray*, CFxSpray*>::const_iterator it =
				 pSeq->m_spray.begin();
			it != pSeq->m_spray.end(); ++it)
			(*it).second->m_bActive = false;
	}
	if (m_n_D50 == 1 && m_roomMode == 6)
	{
		ControlUiHideMode(1);
		return;
	}
	ControlUiHideMode(2);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit1(int param)
{
	m_pFrArea_3DC = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3DC)
		m_pFrArea_3DC->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit2(int param)
{
	m_pFrArea_3E0 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3E0)
		m_pFrArea_3E0->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit3(int param)
{
	m_pFrArea_3E4 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3E4)
		m_pFrArea_3E4->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit4(int param)
{
	m_pFrArea_3E8 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3E8)
		m_pFrArea_3E8->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit5(int param)
{
	m_pFrArea_3EC = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3EC)
		m_pFrArea_3EC->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit6(int param)
{
	m_pFrArea_3F0 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3F0)
		m_pFrArea_3F0->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit7(int param)
{
	m_pFrArea_3F4 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3F4)
		m_pFrArea_3F4->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureBackGroundInit8(int param)
{
	m_pFrArea_3F8 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3F8)
		m_pFrArea_3F8->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureScrollInit1(int param)
{
	m_pFrArea_3FC = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_3FC)
		m_pFrArea_3FC->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureScrollInit2(int param)
{
	m_pFrArea_400 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_400)
		m_pFrArea_400->SetVisible(false);
}

void CRealMyRoomMain::OnFurnitureScrollInit3(int param)
{
	m_pFrArea_404 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pFrArea_404)
		m_pFrArea_404->SetVisible(false);
}

void CRealMyRoomMain::BuildDisplayStandFurniture()
{
	int added = 0;
	int skipped = 0;
	unsigned int typeFilter = 0xffffffff;
	if (m_pFrListBox_3B0 == NULL)
		return;
	m_pFrListBox_3B0->ClearItem();
	std::list<sFurniture_List>::iterator it;
	if (m_eFurniture_Tab_7A0 == 0)
	{
		for (it = Doc()->m_furnitureList.begin();
			it != Doc()->m_furnitureList.end(); ++it)
		{
			skipped++;
			if (skipped > m_n_7A4 * 11)
			{
				FrListItem* pItem = m_pFrListBox_3B0->AddItem(&(*it));
				added++;
				if ((*it).bArrange)
					m_pFrListBox_3B0->SelectItem(pItem, true);
				if (added >= 11)
					break;
			}
		}
	}
	else if (m_eFurniture_Tab_7A0 == 4)
	{
		for (it = Doc()->m_furnitureList.begin();
			it != Doc()->m_furnitureList.end(); ++it)
		{
			int type = ((*it).typeId >> 11) & 0x7ff;
			if (((type >= 0 && type < 7) || type >= 10) &&
				(skipped++, added < 11) && skipped > m_n_7A4 * 11)
			{
				FrListItem* pItem = m_pFrListBox_3B0->AddItem(&(*it));
				added++;
				if ((*it).bArrange)
					m_pFrListBox_3B0->SelectItem(pItem, true);
			}
		}
	}
	else
	{
		if (m_eFurniture_Tab_7A0 == 1)
			typeFilter = 7;
		else if (m_eFurniture_Tab_7A0 == 2)
			typeFilter = 9;
		else if (m_eFurniture_Tab_7A0 == 3)
			typeFilter = 8;
		for (it = Doc()->m_furnitureList.begin();
			it != Doc()->m_furnitureList.end(); ++it)
		{
			if ((((*it).typeId >> 11) & 0x7ff) == typeFilter &&
				(skipped++, added < 10) && skipped > m_n_7A4 * 11)
			{
				FrListItem* pItem = m_pFrListBox_3B0->AddItem(&(*it));
				added++;
				if ((*it).bArrange)
					m_pFrListBox_3B0->SelectItem(pItem, true);
			}
		}
	}
	if (0 < 11 - added % 11)
	{
		int blank = 11 - added % 11;
		do
		{
			m_pFrListBox_3B0->AddItem(NULL);
			blank--;
		} while (blank != 0);
	}
	if (m_eFurniture_Tab_7A0 == 0)
		m_n_7A8 = Doc()->m_furnitureList.size() / 11;
	else
		m_n_7A8 = skipped / 11;
	if (Doc()->m_furnitureList.size() % 11 != 0)
	{
		m_n_7A8++;
		if (m_n_7A8 == 1)
		{
			m_pFrButton_3B8->Enable(false);
			m_pFrButton_3BC->Enable(false);
		}
		else
		{
			m_pFrButton_3B8->Enable(true);
			m_pFrButton_3BC->Enable(true);
		}
	}
	else if (m_n_7A8 == 0)
	{
		m_n_7A8 = 1;
		m_pFrButton_3B8->Enable(false);
		m_pFrButton_3BC->Enable(false);
	}
	if (m_n_7A4 == 0)
		m_pFrButton_3B8->Enable(false);
	if (m_n_7A4 + 1 == m_n_7A8)
		m_pFrButton_3BC->Enable(false);
	m_pFrEdit_3C0->ClearLine();
	m_pFrEdit_3C0->AddText(MakeStr("%3d", m_n_7A4 + 1), false, true);
	m_pFrEdit_3C4->ClearLine();
	m_pFrEdit_3C4->AddText(MakeStr("%3d", m_n_7A8), false, true);
}

void CRealMyRoomMain::OnFurnitureTabMainInit(int param)
{
	m_pFrTabButton_3AC = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	if (m_pFrTabButton_3AC)
	{
		for (int i = 0; i < 5; i++)
			m_pFrTabButton_3AC->AddTab(MakeStr("rmr_arg_tab%d", i + 1), i,
				NULL);
		m_pFrTabButton_3AC->Select(m_eFurniture_Tab_7A0);
		m_pFrTabButton_3AC->SetVisible(false);
	}
}

void CRealMyRoomMain::OnFurnitureTabMainButtonUp()
{
	if (m_pFrTabButton_3AC->GetSelected())
		ChangeFMainTab((eFurniture_Tab)m_pFrTabButton_3AC->GetSelected()->index,
			false);
}

void CRealMyRoomMain::ChangeFMainTab(eFurniture_Tab tab, bool bForce)
{
	if (m_eFurniture_Tab_7A0 != tab)
		m_n_7A4 = 0;
	m_eFurniture_Tab_7A0 = tab;
	if (tab != 2 && tab != 3 && tab != 1)
	{
		m_pFrListBox_3B0->SetMultiSelect(true);
		BuildDisplayStandFurniture();
		return;
	}
	m_pFrListBox_3B0->SetMultiSelect(false);
	BuildDisplayStandFurniture();
}

void CRealMyRoomMain::OnFurnitureListInit(int param)
{
	m_pFrListBox_3B0 = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pFrListBox_3B0)
	{
		m_pFrListBox_3B0->SetVisible(false);
		m_pFrListBox_3B0->UseDummy(true);
		m_pFrListBox_3B0->SetMultiSelect(true);
	}
}

void CRealMyRoomMain::OnFurnitureListOwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	float offsetY = 0.0f;
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
	{
		if (m_pFrEdit_36C && m_pFrEdit_36C->IsVisible())
			m_pFrEdit_36C->SetElementFadeOut(true);
		if (m_pFrEdit_368 && m_pFrEdit_368->IsVisible())
			m_pFrEdit_368->SetElementFadeOut(true);
		if (m_pFrViewer_364 && m_pFrViewer_364->IsVisible())
			m_pFrViewer_364->SetElementFadeOut(true);
		return;
	}
	const Bitmap* pBitmap;
	if (!pItem->pData)
		pBitmap = m_pBitmap_64C;
	else
	{
		unsigned long color;
		if (pItem->selected)
			color = 0xffff0000;
		else
		{
			if (pItem->underCursor)
				offsetY = 3.0f;
			color = 0xff000000;
		}
		pBitmap = m_pBitmap_640;
		pGI->SetTextColor(color, 0xffffffff);
	}
	pGI->SetTextStyle(0);
	if (pBitmap)
	{
		WRect rect(pItem->pos.x, pItem->pos.y - offsetY,
			(float)pBitmap->Width(), (float)pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	void* pData = pItem->pData;
	if (!pData)
		return;
	IFF_STRUCT::sFurniture* pFurniture =
		ItemManager()->FindFurniture(*(unsigned long*)((char*)pData + 4));
	if (pFurniture)
	{
		pBitmap = g_pFresh->GetBitmap(pFurniture->c.Icon);
		if (pBitmap)
		{
			WRect rect(pItem->pos.x + 3.0f, pItem->pos.y + 3.0f - offsetY,
				(float)pBitmap->Width() * 0.58f,
				(float)pBitmap->Height() * 0.58f);
			pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
		}
	}
	if (pItem->selected && m_pBitmap_648)
	{
		pBitmap = m_pBitmap_648;
		WRect rect(pItem->pos.x, pItem->pos.y - offsetY,
			(float)pBitmap->Width(), (float)pBitmap->Height());
		pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}
	if (pItem->underCursor)
	{
		if (m_pFrEdit_36C && !m_pFrEdit_36C->IsVisible())
			m_pFrEdit_36C->SetElementFadeOut(false);
		if (m_pFrEdit_368 && !m_pFrEdit_368->IsVisible())
			m_pFrEdit_368->SetElementFadeOut(false);
		if (m_pFrViewer_364 && !m_pFrViewer_364->IsVisible())
			m_pFrViewer_364->SetElementFadeOut(false);
		IFF_STRUCT::sDesc* pDesc =
			ItemManager()->FindDesc(*(unsigned long*)((char*)pData + 4));
		char buf[512];
		sprintf(buf, " [%s]", pFurniture->c.Name);
		m_pFrEdit_368->ClearLine();
		m_pFrEdit_36C->ClearLine();
		m_pFrEdit_36C->AddText(buf, false, true);
		if (pDesc)
			m_pFrEdit_368->AddText(pDesc->Desc, false, true);
		m_f_6A0 = pItem->pos.x;
		m_f_6A4 = pItem->pos.y;
	}
}

void CRealMyRoomMain::OnFurnitureListBtnDown()
{
	FrListItem* pItem = m_pFrListBox_3B0->GetItemUnderCursor();
	if (pItem)
	{
		sFurniture_List* pFurniture = (sFurniture_List*)pItem->pData;
		if (pItem->selected == false &&
			((pFurniture->typeId >> 11) & 0x7ff) != 8 &&
			((pFurniture->typeId >> 11) & 0x7ff) != 9)
		{
			pItem->selected = true;
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify_yesno", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xbc\xb1\xc5\xc3\xc7\xd1 \xb0\xa1\xb1\xb8\xc0\xc7 \xbc\xb3\xc4\xa1\xb8\xa6 \xc1\xa6\xb0\xc5\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
				false);
			pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomMain::
							OnFurnitrueListDeleteDlgResult,
				0);
			return;
		}
		std::list<sFurniture_List>::iterator it;
		for (it = Doc()->m_furnitureList.begin();
			it != Doc()->m_furnitureList.end(); ++it)
		{
			unsigned int type = ((*it).typeId >> 11) & 0x7ff;
			if ((type == 8 || type == 9) &&
				((pFurniture->typeId >> 11) & 0x7ff) == type &&
				(*it).id != pFurniture->id)
				(*it).bArrange = 0;
		}
		pFurniture->bArrange = 1;
		unsigned int type = (pFurniture->typeId >> 11) & 0x7ff;
		if (type == 0xe)
		{
			IFF_STRUCT::sFurniture* pDesc =
				ItemManager()->FindFurniture(pFurniture->typeId);
			pFurniture->x = pDesc->Pos[0];
			pFurniture->y = pDesc->Pos[1];
			pFurniture->z = pDesc->Pos[2];
			pFurniture->r = pDesc->Pos[3];
		}
		if (CRealMyRoomData::Instance()->AddModel(*pFurniture))
		{
			pFurniture->bArrange = 1;
			if (type == 0xe && CRealMyRoomData::Instance()->m_pSelModelInfo)
			{
				WVector pos = g_view->Projection(
					*(WVector*)(*(char**)((char*)CRealMyRoomData::Instance()
											  ->m_pSelModelInfo +
									8) +
						0x34));
				g_input->SetMousePoint(pos.x, pos.y);
			}
			if (m_eFurniture_Tab_7A0 == 0 && (type == 8 || type == 9))
			{
				int nSize = m_pFrListBox_3B0->GetCurrentItemSize(true);
				for (int i = 0; i < nSize; i++)
				{
					FrListItem* pOther = m_pFrListBox_3B0->GetItem(i);
					if (pOther && pOther != pItem && pOther->pData &&
						((((sFurniture_List*)pOther->pData)->typeId >> 11) &
							0x7ff) == type &&
						pOther->selected)
						m_pFrListBox_3B0->UnselectItem(pOther);
				}
			}
		}
		else
		{
			pFurniture->bArrange = 0;
			pItem->selected = false;
		}
	}
}

void CRealMyRoomMain::OnBtnMenuInit(int param)
{
	m_pFrButton_420 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CRealMyRoomMain::OOnBtnMenuButtonUP()
{
	m_pFrListBox_3B4->SetVisible(true);
}

void CRealMyRoomMain::OnBtn_MenuListInit(int param)
{
	m_pFrListBox_3B4 = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pFrListBox_3B4)
	{
		m_pFrListBox_3B4->SetVisible(false);
		m_pFrListBox_3B4->UseDummy(true);
		m_pFrListBox_3B4->SetMultiSelect(false);
		SetListMenu();
	}
}

void CRealMyRoomMain::SetListMenu()
{
	if (m_pFrListBox_3B4)
	{
		m_pFrListBox_3B4->ClearItem();
	}
	int hide[13];
	hide[0] = 5;
	hide[1] = 6;
	hide[2] = 7;
	hide[3] = 4;
	int nHide = 4;
	if (!IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/) ||
		Doc()->IsControlServerService(0x80000) != 0)
	{
		hide[4] = 3;
		nHide = 5;
	}
	if (Doc()->IsControlServerService(0x40000) != 0)
	{
		hide[nHide] = 1;
		nHide++;
	}
	if (!IsLocalContent((localContentType_t)0x70 /*983: era 0x71; medido*/))
	{
		hide[nHide] = 0xb;
		nHide++;
	}
	switch (m_n_D4C)
	{
	case 0:
	case 1:
		hide[nHide] = 9;
		nHide++;
		hide[nHide] = 10;
		nHide++;
		break;
	case 2:
		hide[nHide] = 9;
		nHide++;
		hide[nHide] = 10;
		nHide++;
	case 4:
		hide[nHide] = 0xb;
		nHide++;
		break;
	}
	std::list<sFurniture_List>::iterator it;
	for (it = Doc()->m_furnitureList.begin();
		it != Doc()->m_furnitureList.end(); ++it)
	{
		if (((*it).typeId & 0x3ff800) == 0x2800 && (*it).bArrange)
			goto found;
	}
	hide[nHide] = 3;
	nHide++;
found:
	if (m_pFrListBox_3B4)
	{
		for (int menu = 0; menu <= 0xc; menu++)
		{
			if (menu == 0xc)
				goto pad;
			int k = 0;
			if (nHide > 0)
			{
				do
				{
					if (hide[k] == menu)
						goto skip;
					k++;
				} while (k < nHide);
			}
			m_pFrListBox_3B4->AddItem((void*)(menu + 1));
skip:;
		}
		return;
pad:
		int nPad = 2 - m_pFrListBox_3B4->GetCurrentItemSize(true) % 3;
		for (; nPad > 0; nPad--)
		{
			m_pFrListBox_3B4->AddItem(NULL);
		}
		m_pFrListBox_3B4->AddItem((void*)0xd);
	}
}

void CRealMyRoomMain::OnBtn_MenuListOwnerDraw(int param)
{
	if (m_pFrListBox_3B4 && m_pFrListBox_3B4->IsVisible())
	{
		FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
		FrListItem* pItem = (FrListItem*)param;
		if (pItem && pItem->pData)
		{
			int idx = (int)pItem->pData - 1;
			const Bitmap* pBitmap = (&m_pBitmap_5CC)[idx];
			if (pBitmap)
			{
				WRect rect(pItem->pos.x, pItem->pos.y, (float)pBitmap->Width(),
					(float)pBitmap->Height());
				pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
			}
			if ((pItem->selected || pItem->underCursor) && idx != 12)
			{
				pBitmap = (&m_pBitmap_600)[idx];
				if (pBitmap)
				{
					WRect rect(pItem->pos.x, pItem->pos.y,
						(float)pBitmap->Width(), (float)pBitmap->Height());
					pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
				}
			}
			if (pItem->selected && idx != 12)
				pBitmap = m_pBitmap_638;
			else if (pItem->underCursor)
				pBitmap = m_pBitmap_634;
			else
				return;
			if (pBitmap)
			{
				WRect rect(pItem->pos.x, pItem->pos.y, (float)pBitmap->Width(),
					(float)pBitmap->Height());
				pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
			}
		}
	}
}

void CRealMyRoomMain::OnBtn_MenuListBtnDown()
{
	if (m_pFrListBox_3B4 == NULL)
		return;
	FrListItem* pItem = m_pFrListBox_3B4->GetItemUnderCursor();
	if (pItem == NULL)
		return;
	int menu = (int)pItem->pData;
	if (menu != 0)
		SelectMenu((unsigned char)(menu - 1));
}

void CRealMyRoomMain::OnBtn_MenuListRBtnDown()
{
}

void CRealMyRoomMain::OnBtn_MenuListMouseMove()
{
}

void CRealMyRoomMain::OnBtn_MenuListDClick()
{
}

void CRealMyRoomMain::SelectMenu(unsigned char menu)
{
	switch (menu)
	{
	case 0:
		if (m_b_D54 == false)
		{
			ControlUiHideMode(1);
			if (m_b_D88 == 1)
			{
				SetCardBookVisible(true);
				return;
			}
			break;
		}
		ControlUiHideMode(2);
		SetCardBookVisible(false);
		return;
	case 1:
		if (Doc()->IsControlServerService(0x40000))
		{
			(IActor*)this << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xbf\xec\xc6\xed\xc7\xd4 \xb1\xe2\xb4\xc9\xc0\xc7 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			return;
		}
		if (m_pCPost_ReciveDlg_474 == NULL)
		{
			m_pCPost_ReciveDlg_474 = CreateForm<CPost_ReciveDlg>(
				g_pFresh->GetManager(), this, "postbox", NULL);
			FrForm::ms_bHasTail = false;
			m_pCPost_ReciveDlg_474->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnPostDlgResult, 0x81);
			m_pCPost_ReciveDlg_474->SetFadeout(false);
			return;
		}
		break;
	case 2:
		if (m_b_D57 == false)
			ControlUiHideMode(4);
		else
			ControlUiHideMode(2);
		SetCardBookVisible(false);
		m_eFurniture_Tab_7A0 = (eFurniture_Tab)0;
		return;
	case 8:
		if (m_pCRealMyRoom_AuthorityDlg_47C == NULL)
		{
			m_pCRealMyRoom_AuthorityDlg_47C =
				CreateForm<CRealMyRoom_AuthorityDlg>(g_pFresh->GetManager(),
					this, "realmyroom_authority", NULL);
			FrForm::ms_bHasTail = false;
			m_pCRealMyRoom_AuthorityDlg_47C->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnAuthDlgResult, 0x81);
			m_pCRealMyRoom_AuthorityDlg_47C->SetFadeout(false);
			return;
		}
		break;
	case 9:
		if (m_b_D57 == true)
		{
			SetCameraDisplayType(5);
			return;
		}
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xb0\xa1\xb1\xb8 \xb9\xe8\xc4\xa1 \xb8\xf0\xb5\xe5\xbf\xa1\xbc\xad\xb8\xb8 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xb4\xc2 \xb1\xe2\xb4\xc9\xc0\xd4\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 3);
		}
		return;
	case 10:
		if (m_b_D57 != true)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xb0\xa1\xb1\xb8 \xb9\xe8\xc4\xa1 \xb8\xf0\xb5\xe5\xbf\xa1\xbc\xad\xb8\xb8 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xb4\xc2 \xb1\xe2\xb4\xc9\xc0\xd4\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 3);
			return;
		}
		if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar)
		{
			if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar->m_bHide)
			{
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(
					false);
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->m_vPos =
					m_v2c8;
				CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetPosition(
					m_v2c8);
				return;
			}
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->m_vPos = m_v2c8;
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetPosition(
				m_v2c8);
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(true);
			return;
		}
		break;
	case 11:
		if (IsLocalContent(S4_ITEM_STORAGE))
		{
			if (m_b_D98)
			{
				AfxGetTask()->GetActor("RealMyRoom")
					<< MsgObject(NULL, M983(0x234), 0, 0, 0, 0, 0);
				return;
			}
			WSendPacket packet((enumClientPacket)0xd3);
			packet.Send(TO_GAME);
			Net_EquipDresserParts();
			Net_EquipCharacter();
			Net_EquipCaddie();
			Net_EquipItem();
			Net_EquipGear();
			Net_EquipSkin();
			Net_EquipMascot();
			if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
				Doc()->SendEquipItemSlot();
			else
				Doc()->SendEquipItemSlotByMascot();
			Net_EquipCutIn983(); // PKT (frente C, lote 20): 983 00622815
			return;
		}
		break;
	case 12:
		if (m_pFrListBox_3B4)
			m_pFrListBox_3B4->SetVisible(false);
	}
}

void CRealMyRoomMain::SetUIState(unsigned long flag)
{
	if (m_n_D4C & flag)
		m_n_D4C &= ~flag;
	else
		m_n_D4C |= flag;
	CRealMyRoomData::Instance()->m_pSelModelInfo = NULL;
	ChangeFlags(flag);
}

void CRealMyRoomMain::ChangeFlags(unsigned long flag)
{
	memset(&m_b_D54, 0, 5);
	switch (flag)
	{
	case 1:
		m_b_D54 = !m_b_D54;
		break;
	case 2:
		m_b_D55 = !m_b_D55;
		break;
	case 4:
		m_b_D56 = !m_b_D56;
		m_b_D54 = m_b_D56;
		m_b_D55 = m_b_D56;
		break;
	case 8:
		m_b_D57 = !m_b_D57;
		break;
	}
}

bool CRealMyRoomMain::GetFlags(unsigned long flag)
{
	switch (flag)
	{
	case 1:
		return m_b_D54;
	case 2:
		return m_b_D55;
	default:
		return false;
	case 4:
		return m_b_D56;
	case 8:
		return m_b_D57;
	}
}

void CRealMyRoomMain::UIDisplayControl(bool bVisible)
{
	SetOverAndUnderBarVisible(bVisible);
	if (bVisible)
		SetCameraDisplayType(1);
	else
		SetCameraDisplayType(4);
}

void CRealMyRoomMain::FurnitureDisplayControl(bool bShow)
{
	if (bShow)
	{
		BuildDisplayStandFurniture();
	}
	bool bFade = !bShow;
	if (m_pFrListBox_3B0)
	{
		m_pFrListBox_3B0->SetVisible(bShow);
	}
	if (m_pFrTabButton_3AC)
	{
		m_pFrTabButton_3AC->SetElementFadeOut(bFade);
	}
	if (m_pFrButton_3CC)
	{
		m_pFrButton_3CC->SetElementFadeOut(bFade);
	}
	if (m_pFrButton_3D0)
	{
		m_pFrButton_3D0->SetElementFadeOut(bFade);
	}
	if (m_pFrButton_3D4)
	{
		m_pFrButton_3D4->SetElementFadeOut(bFade);
	}
	if (m_pFrButton_3D8)
	{
		m_pFrButton_3D8->SetElementFadeOut(bFade);
	}
	if (m_pFrButton_3B8)
	{
		m_pFrButton_3B8->SetElementFadeOut(bFade);
	}
	if (m_pFrButton_3BC)
	{
		m_pFrButton_3BC->SetElementFadeOut(bFade);
	}
	if (m_pFrEdit_3C0)
	{
		m_pFrEdit_3C0->SetVisible(bShow);
		m_pFrEdit_3C0->ClearLine();
		m_pFrEdit_3C0->AddText(MakeStr("%3d", m_n_7A4 + 1), false, true);
	}
	if (m_pFrEdit_3C4)
	{
		m_pFrEdit_3C4->SetVisible(bShow);
		m_pFrEdit_3C4->ClearLine();
		m_pFrEdit_3C4->AddText(MakeStr("%3d", m_n_7A8), false, true);
	}
	if (m_pFrButton_3C8)
	{
		m_pFrButton_3C8->SetElementFadeOut(bFade);
	}
	FrArea** ppArea = &m_pFrArea_3DC;
	for (int i = 8; i != 0; i--)
	{
		if (*ppArea)
		{
			(*ppArea)->SetElementFadeOut(bFade);
		}
		ppArea++;
	}
	ppArea = &m_pFrArea_3FC;
	for (int j = 3; j != 0; j--)
	{
		if (*ppArea)
		{
			(*ppArea)->SetElementFadeOut(bFade);
		}
		ppArea++;
	}
	m_pFrTabButton_3AC->Select(m_eFurniture_Tab_7A0);
}

void CRealMyRoomMain::PostDlgCreate()
{
	if (m_pCPost_ReciveDlg_474 == NULL)
	{
		m_pCPost_ReciveDlg_474 = CreateForm<CPost_ReciveDlg>(
			g_pFresh->GetManager(), this, "postbox", NULL);
		FrForm::ms_bHasTail = false;
		m_pCPost_ReciveDlg_474->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnPostDlgResult, 0x81);
		m_pCPost_ReciveDlg_474->SetFadeout(false);
	}
	else
	{
		m_pCPost_ReciveDlg_474->OnInit();
	}
}

void CRealMyRoomMain::ControlUiHideMode(int mode)
{
	bool bHide;
	FrWnd** ppWnd;
	int i;
	if (mode != m_n_D4C && mode != 5)
		m_n_D50 = m_n_D4C;
	CFxSequence* pSeq = m_pFxSeq;
	if (pSeq->m_bActive)
	{
		pSeq->m_bActive = false;
		for (std::map<CFxSpray*, CFxSpray*>::const_iterator it =
				 pSeq->m_spray.begin();
			it != pSeq->m_spray.end(); ++it)
			(*it).second->m_bActive = false;
	}
	switch (mode)
	{
	case 1:
		SetUIState(1);
		ShowOverAndUnderBar(true);
		ppWnd = (FrWnd**)&m_pOverBar;
		for (i = 3; i != 0; i--, ppWnd++)
		{
			if (*ppWnd && (*ppWnd)->IsVisible())
				(*ppWnd)->SetElementFadeOut(false);
		}
		SetCameraDisplayType(1);
		m_b_D45 = true;
		SetMiddleUIVisible(true);
		FurnitureDisplayControl(false);
		if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar)
		{
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetState(2);
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(false);
		}
	default:
end:
		m_n_D4C = mode;
		SetListMenu();
		return;
	case 2:
		SetUIState(2);
		ShowOverAndUnderBar(true);
		ppWnd = (FrWnd**)&m_pOverBar;
		for (i = 3; i != 0; i--, ppWnd++)
		{
			if (*ppWnd && (*ppWnd)->IsVisible())
				(*ppWnd)->SetElementFadeOut(false);
		}
		m_b_D45 = 0;
		SetMiddleUIVisible(false);
		bHide = false;
		break;
	case 4:
		m_pFxSeq->SetActive(true, true);
		SetUIState(8);
		ppWnd = (FrWnd**)&m_pOverBar;
		for (i = 3; i != 0; i--, ppWnd++)
		{
			if (*ppWnd && (*ppWnd)->IsVisible())
				(*ppWnd)->SetElementFadeOut(true);
		}
		ShowOverAndUnderBar(false);
		m_b_D45 = 0;
		SetMiddleUIVisible(false);
		bHide = true;
		break;
	case 5:
		if (m_n_D50 == 1)
			ControlUiHideMode((m_roomMode != 6) + 1);
		else
			ControlUiHideMode(2);
		return;
	}
	FurnitureDisplayControl(bHide);
	SetCameraDisplayType(4);
	if (CRealMyRoom_AvatarManager::Instance()->m_pAvatar == NULL)
		goto end;
	CRealMyRoom_AvatarManager::Instance()->m_pAvatar->ChangeCharacterInfo();
	CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetState(0);
	CRealMyRoom_AvatarManager::Instance()->m_pAvatar->SetHide(true);
	goto end;
}

void CRealMyRoomMain::SetOverAndUnderBarVisible(bool bVisible)
{
	bool bHide = !bVisible;
	if (bVisible)
		ShowOverAndUnderBar(true);
	FrWnd** ppWnd = (FrWnd**)&m_pOverBar;
	for (int i = 0; i < 3; i++, ppWnd++)
	{
		if (*ppWnd && (*ppWnd)->IsVisible())
			(*ppWnd)->SetElementFadeOut(bHide);
	}
	if (bHide)
		ShowOverAndUnderBar(false);
}

void CRealMyRoomMain::SetMiddleUIControl(bool bVisible)
{
	if (bVisible)
		SetCameraDisplayType(1);
	else
		SetCameraDisplayType(4);
}

void CRealMyRoomMain::SetMiddleUIVisible(bool bVisible)
{
	FrWnd* pWnd = m_pFrGroupBox_2E4;
	if (bVisible)
	{
		if (pWnd)
			pWnd->SetElementFadeOut(!bVisible);
		ChangeMainTab((mainTab_t)m_roomMode, true);
		return;
	}
	if (pWnd && pWnd->IsVisible())
		pWnd->SetElementFadeOut(!bVisible);
}

void CRealMyRoomMain::Furniture_ShowUI(bool bShow)
{
	bool bHide = !bShow;
	if (m_pFrWnd_654)
		m_pFrWnd_654->SetElementFadeOut(bHide);
	if (m_pFrWnd_658)
		m_pFrWnd_658->SetElementFadeOut(bHide);
	if (m_pFrWnd_660)
		m_pFrWnd_660->SetElementFadeOut(bHide);
	if (m_pFrWnd_664)
		m_pFrWnd_664->SetElementFadeOut(bHide);
	if (m_pFrWnd_668)
		m_pFrWnd_668->SetElementFadeOut(bHide);
	if (m_pFrWnd_66C)
		m_pFrWnd_66C->SetElementFadeOut(bHide);
	if (m_pFrWnd_670)
		m_pFrWnd_670->SetElementFadeOut(bHide);
	if (m_pFrWnd_674)
		m_pFrWnd_674->SetElementFadeOut(bHide);
	if (m_pFrWnd_678)
		m_pFrWnd_678->SetElementFadeOut(bHide);
	if (m_pFrWnd_67C)
		m_pFrWnd_67C->SetElementFadeOut(bHide);
	if (m_pFrWnd_680)
		m_pFrWnd_680->SetElementFadeOut(bHide);
}

void CRealMyRoomMain::Furniture_ShowUI_Left(bool bShow)
{
	if (m_pFrWnd_670)
		m_pFrWnd_670->Enable(bShow);
	if (m_pFrWnd_674)
		m_pFrWnd_674->Enable(bShow);
	if (m_pFrWnd_678)
		m_pFrWnd_678->Enable(bShow);
	if (m_pFrWnd_67C)
		m_pFrWnd_67C->Enable(bShow);
	if (m_pFrWnd_680)
		m_pFrWnd_680->Enable(bShow);
}

void CRealMyRoomMain::OnEffectViewInit(int param)
{
	m_pFrArea_684 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CRealMyRoomMain::OnEffectViewOnwnerDraw(int param)
{
	if (g_pFresh->GetManager()->GetGDI())
	{
		WRect rect = m_pFrArea_684->GetRect();
		WOverlay::DrawBox(g_view, rect, 0, 0x18000000, 0.0f);
	}
}

void CRealMyRoomMain::OnRightMenuInit(int param)
{
	m_pFrWnd_210 = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
	if (m_pFrWnd_210)
	{
		m_pFrWnd_210->SetVisible(false);
		m_pFrWnd_210->SetFixed(true);
	}
}

void CRealMyRoomMain::OnRightMenuLBtnDown()
{
	// HACK: dead references hosted by the last function of the original emission order
	if (0) // HACK: keeps the inline naked HandleMsg emitted (nakedify)
		((CRealMyRoomMain*)0)->CRealMyRoomMain::HandleMsg(*(const MsgObject*)0);
	if (0) // HACK: keeps the inline naked DrawPartsSlot emitted (nakedify)
		((CRealMyRoomMain*)0)
			->DrawPartsSlot(*(FrGraphicInterface**)0, *(FrListItem**)0);
	// 983: bloco `if (0)` de WSingleton<...>::IsInstantiated retirado (classes não declaradas no ramo semântico)
	// HACK: MoveGift/LoadMyRoomImages are inline naked functions whose callers are naked too:
	// a dead call keeps them emitted
	if (0)
	{
		MoveGift(NULL);
		LoadMyRoomImages();
	}
	// HACK: inlines/templates only referenced by naked functions with an in-.text jump table
	// (no dead C copy possible there): dead references keep their COMDATs emitted
	if (0)
	{
		CreateForm<S5::FrRentalExtensionDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrFortuneDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrPangGiftDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrReturnGiftDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrDeleteItemDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrUccDrawDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrReplayModeDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrOpenCardPackDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrUccCopyDlg>(NULL, NULL, NULL, NULL);
		m_pFrStatic_2E8->SetCaption11(NULL);
		m_pFrListBox_304->GetPreviousSelected();
		m_pExhibition->SetAutoRecoverCamera(false);
		m_pFrUccDrawDlg_45C->IsInitialized();
		WRect rc(WPoint(), 0u, 0u);
		// (HandleMsg)
		CRealMyRoom_Avatar* pAvatar =
			CRealMyRoom_AvatarManager::Instance()->m_pAvatar;
		pAvatar->GetIDName();
		pAvatar->GetNickName();
		pAvatar->GetTargetPosition();
		pAvatar->SubState(0x7e);
		COption::Instance()->mSetPCBangMascotMsg(NULL);
		m_pFrEnchantDlg_428->SetResultCode(0);
		m_pCPost_SendDlg_478->UpdateFriendList();
		CItemStorage::Instance()->GetCurPage();
		CItemStorage::Instance()->GetIndexControl();
		CreateForm<FrUniteResultDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrEnchantDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrCardUseDlg>(NULL, NULL, NULL, NULL);
		Doc()->m_clubSetMap.insert(std::pair<const unsigned int, sItemInfo>(
			m_userEquip.tidSkin[0], *(sItemInfo*)NULL));
		for (std::list<sGiftInfo>::iterator git = Doc()->m_giftList.begin();
			git != Doc()->m_giftList.end(); ++git)
			Doc()->m_giftList.erase(git);
		UccManager()->m_icon.find(*(std::string*)NULL);
	}
	if (IsLocalContent(S4_ITEM_STORAGE) == true && m_b_D98 == true)
		return;
	const char* pszMenu =
		((FrContextMenuCtrl*)m_pFrWnd_210)->GetSelectMenuText();
	if (IsLocalContent(S4_FURNITURE_ABILITY))
	{
		if (strcmp(pszMenu, "\xb0\xa1\xb1\xb8\xbb\xe7\xbf\xeb") == 0)
		{
			FurnitureUse();
			return;
		}
		if (strcmp(pszMenu, "\xb0\xa1\xb1\xb8\xb1\xb8\xb5\xbf") == 0)
		{
			FurnitureRun();
			return;
		}
	}
	if (strcmp(pszMenu, "\xb0\xa1\xb1\xb8\xbc\xb1\xc5\xc3") == 0)
	{
		FurnitureFunctionChange();
		return;
	}
	if (strcmp(pszMenu, "\xb0\xa1\xb1\xb8\xc0\xcc\xb5\xbf") == 0)
	{
		CRealMyRoomData::Instance()->SetState(2);
		return;
	}
	if (strcmp(pszMenu, "\xb0\xa1\xb1\xb8\xc8\xb8\xc0\xfc") == 0)
	{
		CRealMyRoomData::Instance()->SetState(3);
		return;
	}
	if (strcmp(pszMenu, "\xb0\xa1\xb1\xb8\xc1\xa6\xb0\xc5") == 0)
	{
		FrListItem* pItem = m_pFrListBox_3B0->FindItem(
			CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo);
		if (pItem != NULL)
			m_pFrListBox_3B0->UnselectItem(pItem);
		std::list<sFurniture_List>::iterator it;
		for (it = Doc()->m_furnitureList.begin();
			it != Doc()->m_furnitureList.end(); ++it)
		{
			if ((*it).id ==
				CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->m_pad0)
				(*it).bArrange = 0;
		}
		CRealMyRoomData::Instance()->SetState(6);
	}
}

void CRealMyRoomMain::OnRightMenuRBtnDown()
{
	bool bLocal = IsLocalContent(S4_ITEM_STORAGE);
	if (bLocal != true || m_b_D98 != bLocal)
		m_pFrWnd_210->SetVisible(false);
}

void CRealMyRoomMain::UpdateRightMenu()
{
	if (CRealMyRoomData::Instance()->m_pSelModelInfo != NULL)
	{
		int nType =
			(CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->tid >> 11) &
			0x7ff;
		if (nType >= 7 && nType <= 9)
		{
			m_pFrWnd_210->SetVisible(false);
			return;
		}
		bool bOwner =
			Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID ||
			!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME);
		if (!bOwner && (nType == 5 || nType == 13))
		{
			if (CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->tid ==
					0x48006820 ||
				CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->tid ==
					0x48006821)
				m_pFrWnd_210->SetVisible(false);
			else
				((FrContextMenuCtrl*)m_pFrWnd_210)
					->SetMenuList(
						"\xb0\xa1\xb1\xb8\xbb\xe7\xbf\xeb@rmr_i_use|"
						"\xb0\xa1\xb1\xb8\xbc\xb1\xc5\xc3@rmr_i_select");
		}
		else if (bOwner && m_b_D57)
		{
			((FrContextMenuCtrl*)m_pFrWnd_210)
				->SetMenuList("\xb0\xa1\xb1\xb8\xbb\xe7\xbf\xeb@rmr_i_use|"
							  "\xb0\xa1\xb1\xb8\xc0\xcc\xb5\xbf@rmr_i_move|"
							  "separator|"
							  "\xb0\xa1\xb1\xb8\xc8\xb8\xc0\xfc@rmr_i_rotate|"
							  "separator|"
							  "\xb0\xa1\xb1\xb8\xc1\xa6\xb0\xc5@rmr_i_delete");
		}
		else if (bOwner &&
			ItemManager()
					->FindFurniture(CRealMyRoomData::Instance()
							->m_pSelModelInfo->pInfo->tid)
					->IsFunction != 0)
		{
			((FrContextMenuCtrl*)m_pFrWnd_210)
				->SetMenuList("\xb0\xa1\xb1\xb8\xbb\xe7\xbf\xeb@rmr_i_use|"
							  "\xb0\xa1\xb1\xb8\xbc\xb1\xc5\xc3@rmr_i_select");
		}
		else
			m_pFrWnd_210->SetVisible(false);

		if (IsLocalContent(S4_FURNITURE_ABILITY) && nType == 13)
		{
			if (IsLocalContent(S4_FURNITURE_ABILITY))
			{
				if (CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->tid ==
						0x48006820 ||
					CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->tid ==
						0x48006821)
					((FrContextMenuCtrl*)m_pFrWnd_210)
						->UnableMenuItem("\xb0\xa1\xb1\xb8\xbb\xe7\xbf\xeb");
				else
					((FrContextMenuCtrl*)m_pFrWnd_210)
						->UnableMenuItem("\xb0\xa1\xb1\xb8\xbc\xb1\xc5\xc3");
			}
		}
		else
			((FrContextMenuCtrl*)m_pFrWnd_210)
				->UnableMenuItem("\xb0\xa1\xb1\xb8\xbb\xe7\xbf\xeb");
	}
	else
		m_pFrWnd_210->SetVisible(false);
}

void CRealMyRoomMain::PopupRightMenu(int x, int y)
{
	if (Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID ||
		!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		m_pFrWnd_210->MoveWindow(WPoint((float)x, (float)y));
		m_pFrWnd_210->SetVisible(true);
	}
	else
	{
		if (!g_pFresh->IsCurrentLayout("REALMYROOMVISITOR"))
		{
			UpdateRightMenu();
			return;
		}
		m_pFrWnd_210->MoveWindow(WPoint((float)x, (float)y));
		m_pFrWnd_210->SetVisible(true);
	}
	m_pFrWnd_210->SetTopmost(true);
	UpdateRightMenu();
}

// JP 983 (00605eb0)
bool CRealMyRoomMain::OnClubSetResetDone983(int result, FrForm* pForm)
{
	if (FrClubSetResetWnd::s_pInstance)
	{
		delete FrClubSetResetWnd::s_pInstance;
		FrClubSetResetWnd::s_pInstance = NULL;
	}
	return true;
}

// JP 983 (00606cb0): itens de reset do taco (a mesma lista do 00499c50 da frente B)
static bool IsClubSetResetItem983(unsigned long tid)
{
	switch (tid)
	{
	case 0x1a000247:
	case 0x1a00024b:
		return true;
	}
	return false;
}

// JP 983 (0061aae0): sem a opcao (0x1a000003), o presente 0x1a00003e/0x1a000048, os itens da guilda e o FrCardRemove; o
// presente S4 e 0x1a000055 (645: 0x1a000054); no fim, o item de reset do taco (FrClubSetResetWnd, frente B).
void CRealMyRoomMain::ExecuteMiniButtonDown(sItemInfo* pItem)
{
	switch (pItem->tid)
	{
	case 0x1a000055:
	case 0x1a0000bc:
		OpenS4EventGiftBox(pItem);
		return;
	case 0x1a00003b:
		if (IsLocalContent(S3_NEWYEAR_MONEY))
		{
			OpenNewYearMoney(pItem);
			return;
		}
		break;
	case 0x1a00003e:
	case 0x1a000048:
		break;
	case 0x1a000052:
		if (IsLocalContent((localContentType_t)0x25 /*983: era 0x24; interpolado*/))
		{
			OpenEventXmasGiftBox(pItem);
			return;
		}
		break;
	case 0x1a000105:
	case 0x1a00014b:
		if (g_pFresh->GetDesktop()->FindChildForm("FrSupplyPackDlg") == NULL)
		{
			S5::FrSupplyPackDlg* pDlg = CreateForm<S5::FrSupplyPackDlg>(
				g_pFresh->GetManager(), NULL, "FrSupplyPackDlg", NULL);
			pDlg->Initialize(pItem->tid);
			pDlg->EnableDrag(false);
			pDlg->Open(NULL, 0);
			return;
		}
		break;
	case 0x1a00015b:
	{
		IActor* pActor = GetActor(s_gatewayActor[1]);
		MsgObject msg(NULL, M983(0x276), 0, 0, 0, 0, 0);
		if (pActor)
		{
			((CRealMyRoomMain*)pActor)
				->HandleMsg(msg); // protected virtual, same slot
			return;
		}
		break;
	}
	default:
	{
		IFF_STRUCT::sItem* pInfo = ItemManager()->FindItem(pItem->tid);
		if (pInfo && pInfo->IsRandomBox())
			OpenNewRandomBox(pItem);
		if (IsClubSetResetItem983(pItem->tid))
		{
			if (FrClubSetResetWnd::s_pInstance)
			{
				FrClubSetResetWnd::s_pInstance->Start983(pItem->tid, 0);
				return;
			}
			if (FrClubSetResetWnd* pWnd = FrClubSetResetWnd::Create983(this))
				pWnd->Start983(pItem->tid, 0);
		}
		break;
	}
	}
}

void CRealMyRoomMain::OnUnderBar_BackUp()
{
	bool bLocal = IsLocalContent(S4_ITEM_STORAGE);
	if (bLocal == true && m_b_D98 == bLocal)
	{
		SetItemStorageVisible(false);
		return;
	}
	CTaskMain::OnUnderBar_BackUp();
}

void CRealMyRoomMain::FurnitureUse()
{
	FrForm* pForm;
	if (!IsLocalContent(S4_FURNITURE_ABILITY))
	{
		pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xb3\xad\xb7\xce \xb1\xe2\xb4\xc9\xc0\xba \xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. "),
			false);
	}
	else
	{
		sRealMyRoomModelInfo* pInfo =
			CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo;
		unsigned long typeId = pInfo->tid;
		if ((typeId & 0xfc000000) == 0x48000000 &&
			(typeId & 0x3ff800) == 0x6800)
		{
			unsigned long id = pInfo->m_pad0;
			if (!CRealMyRoomData::Instance()->IsArragementFurniture(id))
			{
				if (*(unsigned long*)DOCPTR983(0x8cb) ==
					Doc()->m_replayTargetUID)
				{
					pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
						"notify", NULL);
					pForm->SetMessage(
						K2L_Compatibility("\xb9\xe8\xc4\xa1\xb5\xc7\xc1\xf6 \xbe\xca\xc0\xba \xb0\xa1\xb1\xb8\xc0\xd4\xb4\xcf\xb4\xd9. \xb9\xe8\xc4\xa1\xc7\xd1 \xb4\xd9\xc0\xbd \xc0\xfa\xc0\xe5\xb9\xf6\xc6\xb0\xc0\xbb \xb4\xad\xb7\xaf \xc0\xfa\xc0\xe5\xc7\xcf\xbd\xc5 \xc8\xc4\xbf\xa1 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
						false);
				}
				else
				{
					WSendPacket packet((enumClientPacket)0xd7); // JP 983 (s63): 0xd9 no KR 645
					packet.Encode4(id);
					packet.Encode4(0xffffffff);
					packet.Send(TO_GAME);
					return;
				}
			}
			else
			{
				WSendPacket packet((enumClientPacket)0xd7); // JP 983 (s63): 0xd9 no KR 645
				packet.Encode4(id);
				packet.Encode4(0xffffffff);
				packet.Send(TO_GAME);
				return;
			}
		}
		else
		{
			pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify",
				NULL);
			pForm->SetMessage(
				"\xb0\xa1\xb1\xb8 \xbb\xe7\xbf\xeb \xb9\xd7 \xb1\xb8\xb5\xbf \xb8\xde\xb4\xba \xbf\xad\xb1\xe2\xb0\xa1 \xba\xd2\xb0\xa1\xb4\xc9\xc7\xd1 \xb0\xa1\xb1\xb8\xc0\xd4\xb4\xcf\xb4\xd9.",
				false);
		}
	}
	pForm->Open(NULL, 3);
}

void CRealMyRoomMain::FurnitureRun()
{
	if (!IsLocalContent(S4_FURNITURE_ABILITY))
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xb3\xad\xb7\xce \xb1\xe2\xb4\xc9\xc0\xba \xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. "),
			false);
		pForm->Open(NULL, 3);
		return;
	}
	if (m_pCFurniture_AbilityDlg_480 == NULL)
	{
		m_pCFurniture_AbilityDlg_480 = CreateForm<CFurniture_AbilityDlg>(
			g_pFresh->GetManager(), this, "myitem_choice", NULL);
		FrForm::ms_bHasTail = false;
		m_pCFurniture_AbilityDlg_480->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnFurniture_AbilityDlgResult,
			0x81);
		m_pCFurniture_AbilityDlg_480->SetFadeout(false);
	}
}

void CRealMyRoomMain::FurnitureFunctionChange()
{
	short func =
		ItemManager()
			->FindFurniture(
				CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->tid)
			->IsFunction;
	if (func > 100)
	{
		if (Doc()->m_myInfo.info.dwUID != Doc()->m_replayTargetUID &&
			WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		{
			SelectMenu(4);
			return;
		}
		SelectMenu(3);
		return;
	}
	switch (func)
	{
	case 1:
		SelectMenu(0);
		break;
	case 2:
		PostDlgCreate();
		break;
	case 5:
		if (IsLocalContent(S4_ITEM_STORAGE) == true)
			SelectMenu(11);
		break;
	}
}

void CRealMyRoomMain::OnUnderBar_ItemStorageUp()
{
	if (IsLocalContent(S4_ITEM_STORAGE) == true)
		SelectMenu(11);
	CIconToolTip::GetInstance()->ClearToolTip();
}

void CRealMyRoomMain::OnUnderBar_MyRoomUp()
{
	if (g_pFresh->IsCurrentLayout("REALMYROOMVISITOR"))
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			"\xb4\xd9\xb8\xa5\xc0\xaf\xc0\xfa\xc0\xc7 \xb9\xe6\xc0\xbb \xb9\xe6\xb9\xae \xc1\xdf\xc0\xce \xbb\xf3\xc5\xc2\xbf\xa1\xbc\xad\xb4\xc2 \xb8\xb6\xc0\xcc\xb7\xeb\xc0\xb8\xb7\xce \xb0\xa5 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. ",
			false);
		pForm->Open(NULL, 3);
		return;
	}
	CTaskMain::OnUnderBar_MyRoomUp();
}

void CRealMyRoomMain::OnUnderBar_GiftUp()
{
	if (g_pFresh->IsCurrentLayout("REALMYROOMVISITOR"))
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			"\xb4\xd9\xb8\xa5\xc0\xaf\xc0\xfa\xc0\xc7 \xb9\xe6\xc0\xbb \xb9\xe6\xb9\xae \xc1\xdf\xc0\xce \xbb\xf3\xc5\xc2\xbf\xa1\xbc\xad\xb4\xc2 \xbc\xb1\xb9\xb0\xc7\xd4\xc0\xb8\xb7\xce \xb0\xa5 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. ",
			false);
		pForm->Open(NULL, 3);
		return;
	}
	SelectMenu(1);
}

bool CRealMyRoomMain::OnPostDlgResult(int result, FrForm* pForm)
{
	if (pForm)
		FrForm::ms_bHasTail = true;
	SetListMenu();
	m_pCPost_ReciveDlg_474 = NULL;
	return true;
}

bool CRealMyRoomMain::OnFurniture_AbilityDlgResult(int result, FrForm* pForm)
{
	if (pForm != NULL)
		FrForm::ms_bHasTail = true;
	FrForm* pNotify;
	if (result == 1)
	{
		unsigned long tid =
			CRealMyRoomData::Instance()->m_pSelModelInfo->pInfo->tid;
		if ((tid & 0xfc000000) == 0x48000000 && (tid & 0x3ff800) == 0x6800)
		{
			if (m_pCFurniture_AbilityDlg_480 != NULL)
			{
				m_pCFurniture_AbilityDlg_480->GetItemInfo(NULL);
				pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", NULL);
				pNotify->SetMessage(
					"\xb0\xa1\xb1\xb8 \xb1\xb8\xb5\xbf\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbe\xf2\xbe\xee\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.",
					false);
				pNotify->Open(NULL, 3);
				m_pCFurniture_AbilityDlg_480 = NULL;
				return true;
			}
			pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify",
				NULL);
			pNotify->SetMessage(
				"\xb0\xa1\xb1\xb8 \xb1\xb8\xb5\xbf\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbe\xf2\xbe\xee\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.",
				false);
		}
		else
		{
			pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify",
				NULL);
			pNotify->SetMessage(
				"\xb0\xa1\xb1\xb8 \xbb\xe7\xbf\xeb \xb9\xd7 \xb1\xb8\xb5\xbf \xb8\xde\xb4\xba \xbf\xad\xb1\xe2\xb0\xa1 \xba\xd2\xb0\xa1\xb4\xc9\xc7\xd1 \xb0\xa1\xb1\xb8\xc0\xd4\xb4\xcf\xb4\xd9.",
				false);
		}
	}
	else
	{
		pNotify =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pNotify->SetMessage(
			"\xb0\xa1\xb1\xb8\xbf\xa1 \xbe\xc6\xc0\xcc\xc5\xdb \xc3\xdf\xb0\xa1\xc7\xcf\xb1\xe2\xb8\xa6 \xc1\xdf\xb4\xdc\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.",
			false);
	}
	pNotify->Open(NULL, 3);
	m_pCFurniture_AbilityDlg_480 = NULL;
	return true;
}

bool CRealMyRoomMain::OnPostSendDlgResult(int result, FrForm* pForm)
{
	if (pForm)
		FrForm::ms_bHasTail = true;
	if (result == 1)
	{
		if (m_pCPost_SendDlg_478)
		{
			WSendPacket packet((enumClientPacket)0xc3); // JP 983 (s63): 0xbb no KR 645
			packet.Encode4(*(unsigned long*)DOCPTR983(0x8cb));
			packet.Encode4(m_pCPost_SendDlg_478->GetSendUID());
			packet.Encode1(
				(unsigned char)m_pCPost_SendDlg_478->GetSendListSize());
			packet.Encode8(m_pCPost_SendDlg_478->GetSendServiceCharge());
			packet.EncodeStr(std::string(""));
			packet.EncodeStr(m_pCPost_SendDlg_478->GetLetterMsg());
			sMailIncludeItem item;
			std::list<sMailIncludeItem>* pList =
				m_pCPost_SendDlg_478->GetSendList();
			std::list<sMailIncludeItem>::iterator it;
			for (it = pList->begin(); it != pList->end(); it++)
			{
				item = *it;
				packet.EncodeBuffer(&item, 0x35);
			}
			packet.Send(TO_GAME);
			m_pCPost_SendDlg_478 = NULL;
		}
	}
	else
	{
		m_pCPost_ReciveDlg_474 = NULL;
		m_pCPost_SendDlg_478 = NULL;
		m_pCPost_ReciveDlg_474 = CreateForm<CPost_ReciveDlg>(
			g_pFresh->GetManager(), this, "postbox", NULL);
		FrForm::ms_bHasTail = false;
		m_pCPost_ReciveDlg_474->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnPostDlgResult, 0x81);
		m_pCPost_ReciveDlg_474->SetFadeout(false);
	}
	return true;
}

bool CRealMyRoomMain::OnPostSendDlg2Result(int result, FrForm* pForm)
{
	m_pCPost_SendDlg_478 = NULL;
	if (result == 1)
	{
		m_pCPost_SendDlg_478 = CreateForm<CPost_SendDlg>(g_pFresh->GetManager(),
			this, "write_post", NULL);
		FrForm::ms_bHasTail = false;
		m_pCPost_SendDlg_478->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnPostSendDlgResult, 0x81);
		m_pCPost_SendDlg_478->SetFadeout(false);
	}
	return true;
}

bool CRealMyRoomMain::OnAuthDlgResult(int result, FrForm* pForm)
{
	if (pForm)
		FrForm::ms_bHasTail = true;
	if (result == 1 && m_pCRealMyRoom_AuthorityDlg_47C != NULL)
	{
		bool bAuth = m_pCRealMyRoom_AuthorityDlg_47C->GetAuthSetting();
		m_pCRealMyRoom_AuthorityDlg_47C->GetPassWord();
		CSharedDoc* pDoc = Doc();
		if (strlen(pDoc->m_rmrAuthority.password) == 0 && bAuth == true)
		{
			FrForm* pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"rmr_notify", NULL);
			pNotify->SetMessage(
				K2L_Compatibility("\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xc0\xd4\xb7\xc2\xc0\xcc \xb5\xc7\xc1\xf6 \xbe\xca\xbe\xc6\xbc\xad \xb1\xc7\xc7\xd1 \xbc\xb3\xc1\xa4\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pNotify->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnAuthDlgFaileResult, 2);
		}
		else
		{
			pDoc->m_rmrAuthority.bPrivate =
				m_pCRealMyRoom_AuthorityDlg_47C->GetAuthSetting();
			WSendPacket packet((enumClientPacket)0xb8);
			packet.EncodeBuffer((char*)&Doc()->m_rmrAuthority, sizeof(sRealMyRoomAuthority));
			packet.Send(TO_GAME);
		}
	}
	SetListMenu();
	m_pCRealMyRoom_AuthorityDlg_47C = NULL;
	return true;
}

bool CRealMyRoomMain::OnAuthDlgFaileResult(int result, FrForm* pForm)
{
	if (result == 1)
		SelectMenu(8);
	return true;
}

bool CRealMyRoomMain::GetArrangement(unsigned long tid, unsigned long id)
{
	if ((tid & 0xfc000000) != 0x48000000)
		return false;
	cModel* pModel = CRealMyRoomData::Instance()->m_ArrangeList.Start();
	while (true)
	{
		if (pModel == NULL)
			return false;
		if (pModel && pModel->pInfo->id == id)
			break;
		pModel = CRealMyRoomData::Instance()->m_ArrangeList.Next();
	}
	return true;
}

void CRealMyRoomMain::RMR_UpdateDB()
{
	std::list<sFurniture_List> saveList;
	if (Doc()->m_furnitureList.size() == 0)
	{
		return;
	}
	saveList.clear();
	std::list<sFurniture_List>::iterator it = Doc()->m_furnitureList.begin();
	while (it != Doc()->m_furnitureList.end())
	{
		IFF_STRUCT::sFurniture* pFurniture =
			Doc()->m_itemManager.FindFurniture((*it).typeId);
		if (pFurniture == NULL)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				"\xc7\xf6\xc0\xe7\xb4\xc2 \xc0\xfa\xc0\xe5\xc7\xd2\xbc\xf6 \xbe\xf8\xb4\xc2 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9.",
				false);
			pForm->Open(NULL, 3);
		}
		else
		{
			(*it).bArrange = 0;
			cModel* pModel = CRealMyRoomData::Instance()->GetModelData((*it).id,
				(*it).typeId);
			if (pModel == NULL ||
				(pModel->pInfo->typeId & 0xfc000000) != 0x48000000)
			{
				if (CRealMyRoomData::Instance()->IsModelData_SaveList((*it).id,
						(*it).typeId))
				{
					sFurniture_List entry;
					entry.id = (*it).id;
					entry.typeId = (*it).typeId;
					entry.reserved = 0;
					entry.x = (*it).x;
					entry.y = (*it).y;
					entry.z = (*it).z;
					entry.r = (*it).r;
					entry.bArrange = 0;
					saveList.push_back(entry);
				}
			}
			else
			{
				pModel->pInfo->bArrange = 1;
				(*it).bArrange = 1;
				sFurniture_List entry = *pModel->pInfo;
				if (pModel->m_pModelData != NULL)
				{
					entry.x = pModel->m_pModelData->pos.x; // A874: era cModelEntity +0xd0 (645)
					entry.y = pModel->m_pModelData->pos.y; // A874: era cModelEntity +0xd0 (645)
					entry.z = pModel->m_pModelData->pos.z; // A874: era cModelEntity +0xd0 (645)
				}
				int nChange =
					CRealMyRoomData::Instance()->IsFunritureValueChange(
						*pModel);
				if (nChange == -1 || (nChange > 0 && nChange < 3))
				{
					saveList.push_back(entry);
				}
			}
		}
		++it;
	}
	if (saveList.size() == 0)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			"\xba\xaf\xb0\xe6\xc7\xd1 \xb3\xbb\xbf\xeb\xc0\xcc \xbe\xf8\xc0\xb8\xb9\xc7\xb7\xce \xc0\xfa\xc0\xe5\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9.",
			false);
		pForm->Open(NULL, 3);
	}
	else
	{
		WSendPacket packet(0xb6); // JP 983 (s63): 0xae no KR 645
		packet.Encode2((unsigned short)saveList.size());
		for (std::list<sFurniture_List>::iterator p = saveList.begin();
			p != saveList.end(); ++p)
		{
			packet.EncodeBuffer(&*p, 0x1b);
		}
		packet.Send(TO_GAME);
	}
}

void CRealMyRoomMain::LoadCardImages()
{
	for (std::map<unsigned int, IFF_STRUCT::sCard>::iterator it =
			 ItemManager()->m_CardMap.begin();
		it != ItemManager()->m_CardMap.end(); ++it)
	{
		if (m_cardImgMap.find((*it).second.c.TypeId) == m_cardImgMap.end())
		{
			char szName[64];
			sprintf(szName, "%s.png", (*it).second.Image);
			WOverlay* pImg = g_resrcmng->GetOverlay(szName, 0x80000000);
			if (pImg)
				m_cardImgMap.insert(
					std::map<unsigned int, WOverlay*>::value_type(
						(*it).second.c.TypeId, pImg));
		}
	}
}

void CRealMyRoomMain::DeleteCardImages()
{
	for (std::map<unsigned int, WOverlay*>::iterator it = m_cardImgMap.begin();
		it != m_cardImgMap.end(); ++it)
	{
		if (g_resrcmng && (*it).second)
		{
			g_resrcmng->Release((*it).second);
			(*it).second = NULL;
		}
	}
}

WOverlay* CRealMyRoomMain::GetCardImg(unsigned long tid)
{
	std::map<unsigned int, WOverlay*>::iterator it = m_cardImgMap.find(tid);
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(tid);
	if (it == m_cardImgMap.end())
	{
		pCard = ItemManager()->FindCard(tid);
		if (pCard)
		{
			char szName[64];
			sprintf(szName, "%s.png", pCard->Image);
			WOverlay* pImg = g_resrcmng->GetOverlay(szName, 0);
			if (pImg)
				m_cardImgMap.insert(
					std::map<unsigned int, WOverlay*>::value_type(
						pCard->c.TypeId, pImg));
			return pImg;
		}
	}
	else
		return (*it).second;
	return NULL;
}

void CRealMyRoomMain::DrawCard()
{
	if (m_fba0 == 0)
		return;
	if (ItemManager()->FindCard(m_fba0) == NULL)
		return;
	WOverlay* pImg = GetCardImg(m_fba0);
	if (pImg)
	{
		WRect dst(92.0f, 117.0f, 248.0, 499.0);
		WRect src(0.0f, 0.0f, 1.0f, 1.0f);
		pImg->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
	}
}

namespace
{
	// node of the item-attached card list at CSharedDoc+0x124c (std::list<>, value at +8)
	struct sAttachCardNode
	{
		sAttachCardNode* pNext;
		sAttachCardNode* pPrev;
		unsigned long uid;
		unsigned long cardTid;
		unsigned long itemTid;
		unsigned long itemGuid;
		unsigned long pad18[2];
		int slot;
		unsigned long pad24[14];
		int kind;
	};
}

void CRealMyRoomMain::DrawPartsSlot(FrGraphicInterface* pGI, FrListItem* pItem)
{
	sItemInfo* pInfo = (sItemInfo*)pItem->pData;
	if (pInfo == NULL)
		return;
	if (pGI == NULL)
	{
		pGI = g_pFresh->GetManager()->GetGDI();
		if (pGI == NULL)
			return;
	}

	float xChar = 5.0f;
	float xCaddie = 5.0f;
	if (pInfo->CharacterSlotNum == 0 && pInfo->CaddieSlotNum == 0 &&
		m_pBitmap_650 != NULL)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tid);
		if (pPart != NULL && pPart->Beginner == 1)
		{
			WRect rect(pItem->pos.x + 5.0f, pItem->pos.y + 65.0f,
				(float)m_pBitmap_650->Width(), (float)m_pBitmap_650->Height());
			pGI->DrawTexture(m_pBitmap_650, rect, 0xffffffff, 0);
		}
	}

	if (pInfo->CharacterSlotNum < 0 || pInfo->CharacterSlotNum > 4)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tid);
		if (pPart == NULL)
		{
			pInfo->CharacterSlotNum = 0;
		}
		else
		{
			pInfo->CharacterSlotNum = (unsigned short)pPart->CharacterSlot;
			if (pInfo->CharacterSlotNum > 4)
				pInfo->CharacterSlotNum = 0;
		}
	}
	for (int slot = 1;
		pInfo->CharacterSlotNum != 0 && pInfo->CharacterSlotNum != 0xffff &&
		slot < (int)pInfo->CharacterSlotNum + 1;
		slot++)
	{
		sAttachCardNode* pHead = *(sAttachCardNode**)&Doc()->m_cardAbilityList[0];
		bool bFound = false;
		sAttachCardNode* pNode = pHead->pNext;
		if (pNode != pHead)
		{
			do
			{
				if (pInfo->tid == pNode->itemTid &&
					pInfo->guid == pNode->itemGuid)
				{
					IFF_STRUCT::sCard* pCard =
						ItemManager()->FindCard(pNode->cardTid);
					if (pCard != NULL && slot == pNode->slot &&
						pNode->kind == 0)
					{
						bFound = true;
						const Bitmap* pBitmap =
							g_pFresh->GetBitmap(pCard->SlotImg);
						if (pBitmap != NULL)
						{
							WRect rect(xChar + pItem->pos.x,
								pItem->pos.y + 53.0f, (float)pBitmap->Width(),
								(float)pBitmap->Height());
							pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
							xChar += 16.0f;
						}
					}
				}
				pNode = pNode->pNext;
			} while (pNode != pHead);
			if (bFound)
				continue;
		}
		const Bitmap* pBitmap = g_pFresh->GetBitmap("card_character_empty");
		if (pBitmap != NULL)
		{
			WRect rect(xChar + pItem->pos.x, pItem->pos.y + 53.0f,
				(float)pBitmap->Width(), (float)pBitmap->Height());
			pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
			xChar += 16.0f;
		}
	}

	if (pInfo->CaddieSlotNum < 0 || pInfo->CaddieSlotNum > 4)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tid);
		if (pPart == NULL)
		{
			pInfo->CaddieSlotNum = 0;
		}
		else
		{
			pInfo->CaddieSlotNum = (unsigned short)pPart->CaddieSlot;
			if (pInfo->CaddieSlotNum > 4)
				pInfo->CaddieSlotNum = 0;
		}
	}
	for (int slot = 1; pInfo->CaddieSlotNum != 0 &&
		pInfo->CaddieSlotNum != 0xffff && slot < (int)pInfo->CaddieSlotNum + 1;
		slot++)
	{
		sAttachCardNode* pHead = *(sAttachCardNode**)&Doc()->m_cardAbilityList[0];
		bool bFound = false;
		sAttachCardNode* pNode = pHead->pNext;
		if (pNode != pHead)
		{
			do
			{
				if (pInfo->tid == pNode->itemTid &&
					pInfo->guid == pNode->itemGuid)
				{
					IFF_STRUCT::sCard* pCard =
						ItemManager()->FindCard(pNode->cardTid);
					if (pCard != NULL && slot == pNode->slot &&
						pNode->kind == 1)
					{
						bFound = true;
						const Bitmap* pBitmap =
							g_pFresh->GetBitmap(pCard->SlotImg);
						if (pBitmap != NULL)
						{
							WRect rect(xCaddie + pItem->pos.x,
								pItem->pos.y + 70.0f, (float)pBitmap->Width(),
								(float)pBitmap->Height());
							pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
							xCaddie += 16.0f;
						}
					}
				}
				pNode = pNode->pNext;
			} while (pNode != pHead);
			if (bFound)
				continue;
		}
		const Bitmap* pBitmap = g_pFresh->GetBitmap("card_caddie_empty");
		if (pBitmap != NULL)
		{
			WRect rect(xCaddie + pItem->pos.x, pItem->pos.y + 70.0f,
				(float)pBitmap->Width(), (float)pBitmap->Height());
			pGI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
			xCaddie += 16.0f;
		}
	}
}

bool CRealMyRoomMain::OnCardDlgResult(int result, FrForm* pForm)
{
	if (result == 1 && pForm != NULL)
	{
		FrOpenCardPackDlg* pCard = (FrOpenCardPackDlg*)pForm;
		unsigned int type = (pCard->m_tid >> 22) & 0xf;
		if (type == 2)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			WSendPacket packet((enumClientPacket)0xbd);
			packet.Encode4(pCard->m_tid);
			packet.Send(TO_GAME);
		}
		else if (type == 0 || type == 1)
		{
			m_pFrItemAttachCardDlg_450 = CreateForm<FrItemAttachCardDlg>(
				g_pFresh->GetManager(), this, "itemattachcarddlg", NULL);
			m_pFrItemAttachCardDlg_450->m_cardGuid = pCard->m_guid;
			m_pFrItemAttachCardDlg_450->m_cardTid = pCard->m_tid;
			m_pFrItemAttachCardDlg_450->m_cardType = (pCard->m_tid >> 22) & 0xf;
			m_pFrItemAttachCardDlg_450->Open(
				(FRESH_PFN_RESULT)&CRealMyRoomMain::OnItemAttachCardDlgResult,
				1);
		}
	}
	m_pFrCardUseDlg_44C = NULL;
	return true;
}

bool CRealMyRoomMain::OnCardPackDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		FrOpenCardPackDlg* pDlg = (FrOpenCardPackDlg*)pForm;
		if (pDlg == NULL || (pDlg->m_tid & 0x3c00000) != 0xc00000)
		{
			m_pFrOpenCardPackDlg_454 = NULL;
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			{
				WSendPacket packet((enumClientPacket)0xca);
				packet.Encode4(pDlg->m_tid);
				packet.Encode4(pDlg->m_guid);
				packet.Send(TO_GAME);
			}
			m_pFrOpenCardPackDlg_454 = NULL;
		}
	}
	else
	{
		m_pFrOpenCardPackDlg_454 = NULL;
	}
	return true;
}

#pragma pack(push, 1)
struct sItemAttachCardReq
{
	unsigned long guid;
	unsigned long typeId;
	unsigned long x;
	unsigned long y;
	unsigned long slot;
	unsigned long reserved0;
	unsigned long reserved1;
	unsigned long reserved2;
	unsigned long reserved3;
	unsigned long reserved4;
	unsigned long reserved5;
	unsigned long reserved6;
	unsigned long reserved7;
	unsigned long reserved8;
	unsigned short reserved9;
};
#pragma pack(pop)

bool CRealMyRoomMain::OnItemAttachCardDlgResult(int result, FrForm* pForm)
{
	if (pForm != NULL && result == 1)
	{
		FrItemAttachCardDlg* pDlg = (FrItemAttachCardDlg*)pForm;
		if (pDlg->m_slot == 0xff)
		{
			(IActor*)this << MsgObject(NULL, M983(0x23),
				(int)"\xc6\xc4\xc3\xf7\xc0\xc7 \xbd\xbd\xb7\xd4\xc0\xcc \xbc\xb1\xc5\xc3\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9.",
				0, 0, 0, 0);
			return true;
		}
		unsigned int kind = (pDlg->m_cardTid >> 22) & 0xf;
		if ((kind == 0 && pDlg->m_slot > 0 && pDlg->m_slot < 5) ||
			(kind == 1 && pDlg->m_slot > 4 && pDlg->m_slot < 9))
		{
			sItemAttachCardReq req;
			req.guid = pDlg->m_cardGuid;
			req.typeId = pDlg->m_cardTid;
			req.x = pDlg->m_partTid;
			req.y = pDlg->m_partGuid;
			req.slot = pDlg->m_slot;
			WSendPacket packet((enumClientPacket)0xc8);
			packet.EncodeBuffer(&req, 0x3a);
			packet.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			return true;
		}
		(IActor*)this << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xbd\xbd\xb7\xd4\xb0\xfa \xc4\xab\xb5\xe5\xc0\xc7 \xc5\xb8\xc0\xd4\xc0\xcc \xb4\xde\xb6\xf3\xbc\xad \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
	}
	return true;
}

bool CRealMyRoomMain::OnResultDlg_Report(int result, FrForm* pForm)
{
	m_pFrUniteResultDlg_464 = NULL;
	return true;
}

bool CRealMyRoomMain::OnUccDrawDlgResult(int nResult, FrForm* pForm)
{
	m_pFrUccDrawDlg_45C = NULL;
	unsigned long charTid = Doc()->GetMyPartTidList().m_charTid;
	m_pFrListBox_304->SetVisible(true);
	if (g_view)
		ShowPet(charTid);
	return true;
}

bool CRealMyRoomMain::OnUccCopyDlgResult(int result, FrForm* pForm)
{
	m_pFrUccCopyDlg_460 = NULL;
	m_pFrListBox_304->SetVisible(true);
	return true;
}

void CRealMyRoomMain::OpenEventXmasGiftBox(sItemInfo* pInfo)
{
	if (pInfo)
	{
		CGiftBoxEvent* pEvent;
		if (CContentsDoc::Instance()->GetContainer((localContentType_t)0x25,
				pEvent))
		{
			m_n_7B8 = pInfo->guid;
			pEvent->SetOpenedBoxTID(pInfo->tid);
			if (pEvent->GetOpenedBoxTID() != 0)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify_yesno", NULL);
				pForm->SetMessage(
					K2L_Compatibility("\xc5\xa9\xb8\xae\xbd\xba\xb8\xb6\xbd\xba \xbc\xb1\xb9\xb0 \xbb\xf3\xc0\xda\xb8\xa6 \xbf\xad\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
					false);
				pForm->Open((FRESH_PFN_RESULT)&CRealMyRoomMain::
								OnOpenEventXmasGiftDlgResult,
					1);
			}
		}
	}
}

bool CRealMyRoomMain::OnOpenEventXmasGiftDlgResult(int result, FrForm* pForm)
{
	return true;
}

bool __fastcall CheckRandomBoxTypeID(unsigned long tid)
{
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(tid);
	if (pItem != NULL && pItem->IsRandomBox())
		return true;
	return false;
}

void CRealMyRoomMain::OpenNewRandomBox(sItemInfo* pItem)
{
	if (pItem == NULL)
		return;
	unsigned long tid = pItem->tid;
	IFF_STRUCT::sItem* pIffItem = ItemManager()->FindItem(tid);
	if (pIffItem == NULL || !pIffItem->RandomBox)
	{
		std::string msg(
			K2L_Compatibility("\xbf\xad \xbc\xf6 \xbe\xf8\xb4\xc2 \xbc\xb1\xb9\xb0 \xbb\xf3\xc0\xda\xc0\xd4\xb4\xcf\xb4\xd9."));
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x26), (int)msg.c_str(), 0, 0, 0, 0));
	}
	else
	{
		m_n_7BC = pItem->tid;
		OnOpenedBoxDlg(pItem->tid,
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnOpenNewRandomBoxDlgResult,
			this);
	}
}

bool CRealMyRoomMain::OnOpenNewRandomBoxDlgResult(int result, FrForm* pForm)
{
	if (result == 1 && m_n_7BC)
	{
		FrRandomBoxProgress* pDlg = CreateForm<FrRandomBoxProgress>(
			g_pFresh->GetManager(), this, "newRandomBox", NULL);
		if (pDlg == NULL)
			return false;
		pDlg->SetBoxTid(m_n_7BC);
		const char* pName = ItemManager()->GetItemName(m_n_7BC);
		if (pName)
			pDlg->SetExplane(MakeStr(
				K2L_Compatibility("%\xbc\xb1\xc5\xc3\xc7\xcf\xbd\xc5 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xa9\xb4\xc2 \xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
				pName));
		pDlg->SetFixed(true);
		pDlg->Open(NULL, 1);
	}
	m_n_7BC = 0;
	return true;
}

void CRealMyRoomMain::OpenItemStorageNotice()
{
	if (m_pFrItemStorageNotice_D90 == NULL)
		m_pFrItemStorageNotice_D90 = CreateForm<FrItemStorageNotice>(
			g_pFresh->GetManager(), this, "item_storage_notice", NULL);
	if (m_pFrItemStorageNotice_D90)
	{
		m_pFrItemStorageNotice_D90->SetCaption(K2L_Compatibility("\xbe\xcb\xb8\xb2"));
		m_pFrItemStorageNotice_D90->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnResultItemStorageNotice, 9);
	}
}

bool CRealMyRoomMain::OnResultItemStorageNotice(int result, FrForm* pForm)
{
	OpenItemStorageAccessPanel(0);
	m_pFrItemStorageNotice_D90 = NULL;
	return true;
}

void CRealMyRoomMain::OpenItemStorageAccessPanel(unsigned char type)
{
	if (m_pFrItemStorageAccessPanel_D94 == NULL)
		m_pFrItemStorageAccessPanel_D94 = CreateForm<FrItemStorageAccessPanel>(
			g_pFresh->GetManager(), this, "item_storage_access_panel", NULL);
	if (m_pFrItemStorageAccessPanel_D94)
	{
		WRect rect = m_pFrItemStorageAccessPanel_D94->GetRect();
		rect.x = 100.0f;
		rect.y = 75.0f;
		m_pFrItemStorageAccessPanel_D94->SetRect(rect);
		m_pFrItemStorageAccessPanel_D94->SetFormType(
			(FrItemStorageAccessPanel::eFormType)type);
		m_pFrItemStorageAccessPanel_D94->Open(
			(FRESH_PFN_RESULT)&CRealMyRoomMain::OnResultItemStorageAccessPanel,
			9);
	}
}

bool CRealMyRoomMain::OnResultItemStorageAccessPanel(int result, FrForm* pForm)
{
	bool bRet = true;
	if (pForm)
	{
		FrItemStorageAccessPanel* pPanel = (FrItemStorageAccessPanel*)pForm;
		if (result == 1)
			pPanel->SendPasswordToServer();
		else if (pPanel->GetFormType() ==
			FrItemStorageAccessPanel::FORMTYPE_FIRST)
			bRet = false;
	}
	m_pFrItemStorageAccessPanel_D94 = NULL;
	return bRet;
}

void CRealMyRoomMain::OnInitItemStorageBGI(int param)
{
	m_pFrArea_DA4 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	CItemStorage* pStorage = CItemStorage::Instance();
	if (pStorage)
	{
		pStorage->AddFrontUI(m_pFrArea_DA4);
		FrWnd** ppBar = (FrWnd**)&m_pUnderBarBack;
		int i = 0;
		do
		{
			if (*ppBar && i != 3)
				pStorage->AddBackGroundUI(*ppBar, false);
			ppBar++;
			i++;
		} while (i < 0x15);
		pStorage->AddBackGroundUI(m_pOverBarCaddie, false);
		pStorage->AddBackGroundUI(m_pOverBarCaddieSel, false);
		pStorage->AddBackGroundUI(m_pOverBarAztec, false);
		pStorage->AddBackGroundUI(m_pOverBarAztecSel, false);
		pStorage->AddBackGroundUI(m_pOverBarClub, false);
		pStorage->AddBackGroundUI(m_pOverBarClubSel, false);
		pStorage->AddBackGroundUI(m_pOverBarMascot, false);
		pStorage->AddBackGroundUI(m_pOverBarMascotSel, false);
		pStorage->AddBackGroundUI(m_pOverBarOneLineReqBtn, false);
		pStorage->AddBackGroundUI(m_pUnderBarTabBtn, false);
		pStorage->AddBackGroundUI(m_pFrButton_390, false);
		pStorage->AddBackGroundUI(m_pFrButton_394, false);
		pStorage->AddBackGroundUI(m_pFrButton_324, false);
		pStorage->AddBackGroundUI(m_pFrArea_378, true);
		pStorage->AddBackGroundUI(m_pFrButton_380, false);
		pStorage->AddBackGroundUI(m_pFrButton_420, true);
		pStorage->AddBackGroundUI(m_pFrListBox_3B4, true);
	}
	if (!Doc()->m_bItemStoragePin && IsLocalContent(S4_ITEM_STORAGE))
	{
		if (m_b_D98 == 0)
		{
			if (!Doc()->m_bItemStoragePin)
			{
				WSendPacket packet((enumClientPacket)0xd3);
				packet.Send(TO_GAME);
				Net_EquipDresserParts();
				Net_EquipCharacter();
				Net_EquipCaddie();
				Net_EquipItem();
				Net_EquipGear();
				Net_EquipSkin();
				Net_EquipMascot();
				if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
					Doc()->SendEquipItemSlot();
				else
					Doc()->SendEquipItemSlotByMascot();
				Net_EquipCutIn983(); // PKT (frente C, lote 20): 983 00615f11
			}
		}
		else
		{
			AfxGetTask()->GetActor("RealMyRoom")
				<< MsgObject(NULL, M983(0x234), 0, 0, 0, 0, 0);
		}
	}
}

void CRealMyRoomMain::OnOwnerDrawItemStorageBGI(int param)
{
	if (CItemStorage::Instance())
		CItemStorage::Instance()->DrawBackgroundImage();
}

void CRealMyRoomMain::OnInitItemStoragePang(int param)
{
	m_pFrArea_DA8 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (CItemStorage::Instance())
		CItemStorage::Instance()->AddFrontUI(m_pFrArea_DA8);
}

void CRealMyRoomMain::OnOwnerDrawItemStoragePang(int param)
{
}

void CRealMyRoomMain::OnInitItemStoragePangMinus(int param)
{
	m_pFrButton_DAC = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_DAC)
	{
		m_pFrButton_DAC->SetPushDelay(0.1f);
		if (CItemStorage::Instance())
			CItemStorage::Instance()->AddFrontUI(m_pFrButton_DAC);
	}
}

void CRealMyRoomMain::OnLBDownItemStoragePangMinus()
{
	if (CItemStorage::Instance() && !CItemStorage::Instance()->IsLock())
		CItemStorage::Instance()->OpenPangBank(
			FrItemStoragePangBank::TYPE_WITHDRAW);
}

void CRealMyRoomMain::OnInitItemStoragePangPlus(int param)
{
	m_pFrButton_DB0 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_DB0)
	{
		m_pFrButton_DB0->SetPushDelay(0.1f);
		if (CItemStorage::Instance())
			CItemStorage::Instance()->AddFrontUI(m_pFrButton_DB0);
	}
}

void CRealMyRoomMain::OnLBDownItemStoragePangPlus()
{
	if (CItemStorage::Instance() && !CItemStorage::Instance()->IsLock())
		CItemStorage::Instance()->OpenPangBank(
			FrItemStoragePangBank::TYPE_DEPOSIT);
}

void CRealMyRoomMain::OnInitItemStorageList(int param)
{
	m_pFrListBox_DB4 = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pFrListBox_DB4)
	{
		m_pFrListBox_DB4->UseDblClick(true);
		m_pFrListBox_DB4->UseRightButton(true);
		CItemStorage* pStorage = CItemStorage::Instance();
		if (pStorage)
		{
			pStorage->AddFrontUI(m_pFrListBox_DB4);
			pStorage->SetItemStorageArea(m_pFrListBox_DB4->GetRect());
		}
	}
}

void CRealMyRoomMain::OnOwnerDrawItemStorageList(int param)
{
	if (CItemStorage::Instance() && param)
		CItemStorage::Instance()->DrawStoredItemInfo(
			(const FrListBox*&)m_pFrListBox_DB4, *(FrListItem*)param);
}

void CRealMyRoomMain::OnLBDownItemStorageList()
{
	if (IsLocalContent(S4_ITEM_STORAGE) != true || m_b_D98 != true ||
		m_pFrListBox_DB4 == NULL)
		return;
	FrListItem* pItem = m_pFrListBox_DB4->GetItemUnderCursor();
	CItemStorage* pStorage = CItemStorage::Instance();
	if (pStorage == NULL || pStorage->IsLock() || pItem == NULL)
		return;
	sStoredItemInfo* pInfo = (sStoredItemInfo*)pItem->pData;
	if (pInfo)
	{
		sStoredItemDragDrop drag;
		memset(&drag, 0, sizeof(drag));
		drag.index = pItem->idx;
		drag.mode = sStoredItemDragDrop::MODE_NONE;
		drag.stored = *pInfo;
		if (CItemStorage::Instance())
			CItemStorage::Instance()->SetDragDropItem(drag);
	}
	m_pFrListBox_DB4->SelectItem(pItem, true);
}

void CRealMyRoomMain::OnRBDownItemStorageList()
{
	if (IsLocalContent(S4_ITEM_STORAGE) == true && m_b_D98 == true &&
		m_pFrListBox_DB4 && CItemStorage::Instance() &&
		!CItemStorage::Instance()->IsLock())
	{
		FrListItem* pItem = m_pFrListBox_DB4->GetItemUnderCursor();
		if (pItem)
		{
			sStoredItemInfo* pInfo = (sStoredItemInfo*)pItem->pData;
			if (pInfo)
			{
				unsigned long tid = pInfo->GetTypeID();
				unsigned long uid = pInfo->GetGuid();
				OpenInformation(&tid, NULL, &uid);
			}
			m_pFrListBox_DB4->SelectItem(pItem, true);
		}
	}
}

void CRealMyRoomMain::OnDBClickItemStorageList()
{
	if (IsLocalContent(S4_ITEM_STORAGE) == true && m_b_D98 == true &&
		m_pFrListBox_DB4 && CItemStorage::Instance() &&
		!CItemStorage::Instance()->IsLock())
	{
		FrListItem* pItem = m_pFrListBox_DB4->GetItemUnderCursor();
		if (pItem)
		{
			m_pFrListBox_DB4->SelectItem(pItem, true);
			if (CItemStorage::Instance())
				CItemStorage::Instance()->ReqPopStoredItemInfo(*pItem);
		}
	}
}

void CRealMyRoomMain::OnMouseMoveItemStorageList()
{
	if (CItemStorage::Instance())
		CItemStorage::Instance()->SetDrag(sStoredItemDragDrop::MODE_POP);
}

void CRealMyRoomMain::OnInitItemStorageIndexMsg(int param)
{
	m_pFrListBox_DB8 = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (CItemStorage::Instance())
		CItemStorage::Instance()->AddFrontUI(m_pFrListBox_DB8);
}

void CRealMyRoomMain::OnOwnerDrawItemStorageIndexMsg(int param)
{
	if (CItemStorage::Instance())
	{
		WPoint pos(165.0f, 386.0f);
		CItemStorage::Instance()->IndexControlDraw(pos);
	}
}

void CRealMyRoomMain::OnLBDownItemStorageIndexMsg()
{
	if (CItemStorage::Instance() && !CItemStorage::Instance()->IsLock())
		CItemStorage::Instance()->IndexControlClick();
}

void CRealMyRoomMain::OnInitItemStoragePageNext(int param)
{
	m_pFrButton_DBC = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_DBC)
	{
		m_pFrButton_DBC->SetPushDelay(0.1f);
		m_pFrButton_DBC->SetVisible(false);
	}
}

void CRealMyRoomMain::OnLBDownItemStoragePageNext()
{
	if (CItemStorage::Instance() && !CItemStorage::Instance()->IsLock())
		CItemStorage::Instance()->IndexControlNext();
}

void CRealMyRoomMain::OnInitItemStoragePagePrev(int param)
{
	m_pFrButton_DC0 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_DC0)
	{
		m_pFrButton_DC0->SetPushDelay(0.1f);
		m_pFrButton_DC0->SetVisible(false);
	}
}

void CRealMyRoomMain::OnLBDownItemStoragePagePrev()
{
	if (CItemStorage::Instance() && !CItemStorage::Instance()->IsLock())
		CItemStorage::Instance()->IndexControlPrev();
}

void CRealMyRoomMain::OnInitItemStorageClose(int param)
{
	m_pFrButton_DC4 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pFrButton_DC4)
		m_pFrButton_DC4->SetPushDelay(0.1f);
	if (CItemStorage::Instance())
		CItemStorage::Instance()->AddFrontUI(m_pFrButton_DC4);
}

void CRealMyRoomMain::OnLBDownItemStorageClose()
{
	if (CItemStorage::Instance() && !CItemStorage::Instance()->IsLock())
		SetItemStorageVisible(false);
	if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		m_pUnderBarGame->Enable(false);
		m_pUnderBarServer->Enable(false);
	}
}

void CRealMyRoomMain::OnInitItemStorageDragDrop(int param)
{
	m_pFrArea_DC8 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (CItemStorage::Instance())
		CItemStorage::Instance()->AddFrontUI(m_pFrArea_DC8);
}

// JP 983 (pedido B89): repassadores dos dialogos do reset do taco
bool CRealMyRoomMain::OnClubSetResetSelected983(int result, FrForm* pForm)
{
	if (FrClubSetResetWnd::s_pInstance)
		return FrClubSetResetWnd::s_pInstance->OnSelectResult983(result, pForm);
	return true;
}

bool CRealMyRoomMain::OnClubSetResetQuestion983(int result, FrForm* pForm)
{
	if (FrClubSetResetWnd::s_pInstance)
		return FrClubSetResetWnd::s_pInstance->OnQuestionResult983(result, pForm);
	return true;
}

bool CRealMyRoomMain::OnClubSetResetProgress983(int result, FrForm* pForm)
{
	if (FrClubSetResetWnd::s_pInstance)
		return FrClubSetResetWnd::s_pInstance->OnProgressResult983(result, pForm);
	return true;
}

bool CRealMyRoomMain::OnClubSetResetCloseQuestion983(int result, FrForm* pForm)
{
	if (FrClubSetResetWnd::s_pInstance)
		return FrClubSetResetWnd::s_pInstance->OnCloseQuestion983(result, pForm);
	return true;
}

void CRealMyRoomMain::OnOwnerDrawItemStorageDragDrop(int param)
{
	if (CItemStorage::Instance())
		CItemStorage::Instance()->DragDropItemDraw();
}

void CRealMyRoomMain::SetItemStorageVisible(bool bVisible)
{
	if (IsLocalContent(S4_ITEM_STORAGE) == true)
	{
		m_b_D98 = bVisible;
		CItemStorage* pStorage = CItemStorage::Instance();
		if (pStorage)
		{
			if (bVisible == true)
			{
				pStorage->SetVisibleFrontUI(true);
				pStorage->SetEnableBackGroundUI(false);
				pStorage->SetVisibleBackGroundUI(false);
				pStorage->InitIndexControl(pStorage->GetTotalPage(), 5);
				if (m_pFrListBox_DB4 == NULL)
					return;
				pStorage->MakeStoredItemInfo(m_pFrListBox_DB4);
				if (m_pFrButton_DBC == NULL)
					return;
				if (m_pFrButton_DC0 == NULL)
					return;
				if (pStorage->NeedPageIndexer() == true)
				{
					m_pFrButton_DBC->SetVisible(true);
					m_pFrButton_DC0->SetVisible(true);
					return;
				}
			}
			else
			{
				pStorage->SetVisibleFrontUI(false);
				pStorage->SetEnableBackGroundUI(true);
				pStorage->SetVisibleBackGroundUI(true);
				if (m_pFrListBox_DB4)
					m_pFrListBox_DB4->ClearItem();
				pStorage->ClearStoredItemInfo();
				FrButton* pBtn = m_pFrButton_DBC;
				if (pBtn == NULL)
					return;
				if (m_pFrButton_DC0 == NULL)
					return;
				pBtn->SetVisible(false);
				m_pFrButton_DC0->SetVisible(false);
				return;
			}
			m_pFrButton_DBC->SetVisible(false);
			m_pFrButton_DC0->SetVisible(false);
		}
	}
}

void CRealMyRoomMain::DrawCutinFrame()
{
	if (m_n_77C != 0x21)
		return;
	FrListItem* pItem = m_pFrListBox_304->GetSelected();
	if (pItem == NULL)
		return;
	sItemInfo* pInfo = (sItemInfo*)pItem->pData;
	if (pInfo == NULL)
		return;
	WOverlay* pImg = GetCutinImg(pInfo->tid);
	if (pImg)
	{
		WRect dst(72.0f, 67.0f, (float)pImg->GetWidth(),
			(float)pImg->GetHeight());
		WRect src(0.0f, 0.0f, 1.0f, 1.0f);
		pImg->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
	}
}

WOverlay* CRealMyRoomMain::GetCutinImg(unsigned long tid)
{
	WOverlay* pImg;
	std::map<unsigned int, WOverlay*>::iterator it = m_cutinImgMap.find(tid);
	if (it == m_cutinImgMap.end())
	{
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(tid);
		if (pSkin)
		{
			char szName[64];
			sprintf(szName, "%s.jpg", pSkin->Tex);
			pImg = g_resrcmng->GetOverlay(szName, 0);
			if (pImg)
				m_cutinImgMap.insert(
					std::map<unsigned int, WOverlay*>::value_type(
						pSkin->c.TypeId, pImg));
		}
		else
			return NULL;
	}
	else
		pImg = (*it).second;
	return pImg;
}

void CRealMyRoomMain::DeleteCutinImages()
{
	for (std::map<unsigned int, WOverlay*>::iterator it = m_cutinImgMap.begin();
		it != m_cutinImgMap.end(); ++it)
	{
		if (g_resrcmng && (*it).second)
		{
			g_resrcmng->Release((*it).second);
			(*it).second = NULL;
		}
	}
}

bool CRealMyRoomMain::OnRentalExtensionDlgResult(int result, FrForm* pForm)
{
	S5::FrRentalExtensionDlg* pDlg =
		DYNAMIC_CAST(S5::FrRentalExtensionDlg, pForm);
	if (result == 3)
	{
		(IActor*)this << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		const sItemInfo* pItem = pDlg->GetItemInfo();
		WSendPacket packet((enumClientPacket)0xe6);
		packet.Encode4(pItem->guid);
		packet.Send(TO_GAME);
	}
	else if (result == 4)
	{
		(IActor*)this << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		const sItemInfo* pItem = pDlg->GetItemInfo();
		WSendPacket packet((enumClientPacket)0xe7);
		packet.Encode4(pItem->guid);
		packet.Send(TO_GAME);
	}
	return true;
}
