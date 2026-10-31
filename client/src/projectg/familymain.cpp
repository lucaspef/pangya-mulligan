#include "minatl.h"
#include "familymain.h"
#include <algorithm>
#include <string>
#include "shareddoc.h"
#include "treasuredlg.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "frarea.h"
#include "frbutton.h"
#include "frviewer.h"
#include "frlistbox.h"
#include "frgraphicinterface.h"
#include "equipdlg.h"
#include "exhibition.h"
#include "playerdlg.h"
#include "bitmap.h"
#include "commonutil.h"
#include "mousecursor.h"
#include "inputmanager.h"
#include "rankingdlg.h"
#include "user_info.h"
#include "parttidlist.h"
#include "cardsystem.h"
#include "wview.h"
#include "mathconsts.h"
#include "anotherhand.inl"


unsigned long CFamilyMain::ms_itemslotBackup[4][10];

CFamilyMain::cFamily::cFamily()
	: m_pExhibition(NULL), m_pPartTidList(NULL), m_pStatBar(NULL), m_petKind(0)
{
	memset(&m_charInfo, 0, sizeof(m_charInfo));
	memset(&m_userEquip, 0, sizeof(m_userEquip));
	memset(&m_mascotInfo, 0, sizeof(m_mascotInfo));
	memset(&m_caddieInfo, 0, sizeof(m_caddieInfo));
	memset(&m_clubInfo, 0, sizeof(m_clubInfo));
}

CFamilyMain::cFamily::~cFamily()
{
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
	if (m_pPartTidList)
	{
		delete m_pPartTidList;
		m_pPartTidList = NULL;
	}
	if (m_pStatBar)
	{
		delete m_pStatBar;
		m_pStatBar = NULL;
	}
}

void CFamilyMain::cFamily::SetItemSlot(unsigned long* pItemSlot)
{
	memcpy(m_userEquip.tidItemSlot, pItemSlot, sizeof(m_userEquip.tidItemSlot));
}

void CFamilyMain::cFamily::ConvertUserInfo(sUserInfo& info)
{
	info.roomIndex = 0;
	info.caddieInfo = m_caddieInfo;
	info.charInfo = m_charInfo;
	sClubInfo clubInfo;
	clubInfo.guid = m_clubInfo.guid;
	clubInfo.tid = m_clubInfo.tid;
	memcpy(clubInfo.PCL, m_clubInfo.Common, sizeof(clubInfo.PCL));
	info.clubInfo = clubInfo;
	info.info = Doc()->m_myInfo.info;
	memcpy(info.mapStat, Doc()->m_myInfo.mapStat, sizeof(info.mapStat));
	info.mascotInfo = m_mascotInfo;
	info.stat = Doc()->m_myInfo.stat;
	info.trophy = Doc()->m_myInfo.trophy;
	info.userEquip = m_userEquip;
}

const WPoint& CFamilyMain::cFamily::GetFramePos() const
{
	return m_framePos;
}

const sUserEquip& CFamilyMain::cFamily::GetUserEquip() const
{
	return m_userEquip;
}

void CFamilyMain::cFamily::SetUserEquip(int category, unsigned long id)
{
	switch (category)
	{
	case CEquipHelper::CATEGORY_CHARACTER:
	{
		std::map<unsigned int, sCharacterInfo>::iterator it =
			Doc()->m_charMap.find(id);
		if (it != Doc()->m_charMap.end())
		{
			m_userEquip.guidChar = id;
			m_charInfo = it->second;
			if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/) &&
				IsLocalContent((localContentType_t)0x65 /*983: era 0x64; interpolado*/))
				CCardManager::Instance()->m_charInfo[m_petKind + 1] =
					m_charInfo;
			PetRebuild();
		}
		break;
	}
	case CEquipHelper::CATEGORY_MASCOT:
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			Doc()->m_mascotMap.find(id);
		if (it != Doc()->m_mascotMap.end())
		{
			m_userEquip.guidMascot = id;
			m_mascotInfo = it->second;
		}
		else
		{
			memset(&m_mascotInfo, 0, sizeof(m_mascotInfo));
		}
		SetStatus();
		break;
	}
	case CEquipHelper::CATEGORY_CADDIE:
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			Doc()->m_caddieMap.find(id);
		if (it != Doc()->m_caddieMap.end())
		{
			m_userEquip.guidCaddie = id;
			m_caddieInfo = it->second;
		}
		else
		{
			memset(&m_caddieInfo, 0, sizeof(m_caddieInfo));
		}
		SetStatus();
		break;
	}
	case CEquipHelper::CATEGORY_BALL:
		m_userEquip.tidBall = id;
		break;
	case CEquipHelper::CATEGORY_CLUB:
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			Doc()->m_clubSetMap.find(id);
		if (it != Doc()->m_clubSetMap.end())
		{
			m_userEquip.guidClubSet = id;
			m_clubInfo = it->second;
			SetStatus();
		}
		break;
	}
	}
}

void CFamilyMain::cFamily::Init(const WRect& rect, std::vector<int>& petList,
	unsigned char petKind)
{
	m_rect = rect;
	const Bitmap* pFrame = g_pFresh->GetBitmap("family_info_frm");
	if (pFrame)
	{
		m_framePos.x = m_rect.x + (m_rect.w - pFrame->Width()) * 0.5f;
		m_framePos.y = m_rect.y + m_rect.h - pFrame->Height();
	}

	m_modelPos.x = m_framePos.x + 45.0f;
	m_modelPos.y = m_framePos.y + 50.0f;
	m_petKind = petKind;

	MakePet(petList);
}

void CFamilyMain::cFamily::Process(float delta)
{
	if (!m_pExhibition)
		return;

	m_pStatBar->Process(delta);

	FrWnd* pShop =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop");
	if (pShop == NULL)
	{
		__rtti_obj = NULL;
		m_pExhibition->SetEnableRotate(true);
		if (g_pFresh->GetManager()->GetDesktop()->FindChildByName(
				"treasuremap") ||
			CRankingInfo::Instance()->GetDlg() ||
			(CUserInfo::Instance() && CUserInfo::Instance()->GetDlg()))
			m_pExhibition->SetEnableRotate(false);

		m_pExhibition->Process(delta, false);
	}
}

void CFamilyMain::cFamily::Render()
{
	FrWnd* pShop =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop");
	if (pShop == NULL)
	{
		__rtti_obj = NULL;
		if (!m_anotherHand.IsWorking(0) &&
			!CRankingInfo::Instance()->GetDlg() &&
			!(CUserInfo::Instance() && CUserInfo::Instance()->GetDlg()))
		{
			m_anotherHand.Enter();
			if (m_pExhibition)
				m_pExhibition->Display(0.0f, 0.0f);
			m_anotherHand.Leave();
		}
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;

	const Bitmap* pFrame = g_pFresh->GetBitmap("family_info_frm");
	if (pFrame)
	{
		float w = (float)pFrame->Width();
		float h = (float)pFrame->Height();
		pGDI->DrawTexture(pFrame, WRect(m_framePos.x, m_framePos.y, w, h),
			0xFFFFFFFF, 0);
	}

	const Bitmap* pCharFrame = g_pFresh->GetBitmap("family_info_char");
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(m_charInfo.tid);
	if (pItem && pCharFrame)
	{
		const Bitmap* pText =
			g_pFresh->GetBitmap(MakeStr("text_%s", pItem->Icon));
		if (pText)
		{
			WPoint pos = m_framePos;
			pos.y -= 30.0f;
			pos.x += (m_rect.w - pCharFrame->Width()) * 0.5f;
			pGDI->DrawTexture(pCharFrame,
				WRect(pos.x, pos.y, (float)pCharFrame->Width(),
					(float)pCharFrame->Height()),
				0xFFFFFFFF, 0);

			float x = (pCharFrame->Width() - pText->Width()) * 0.5f + pos.x;
			float y = (pCharFrame->Height() - pText->Height()) * 0.5f + pos.y;
			pGDI->DrawTexture(pText,
				WRect(x, y, (float)pText->Width(), (float)pText->Height()),
				0xFFFFFFFF, 0);
		}
	}

	m_pStatBar->Render(m_modelPos);
}

void CFamilyMain::cFamily::MakePet(std::vector<int>& petList)
{
	static int index = 0;

	index++;
	if (index >= (int)CSharedDoc::Instance()->m_charMap.size())
	{
		index = 0;
		std::random_shuffle(petList.begin(), petList.end());
	}

	int n = petList[index];
	std::map<unsigned int, sCharacterInfo>::iterator it =
		CSharedDoc::Instance()->m_charMap.begin();
	for (int i = 0; i < n; i++)
		++it;

	sCharacterInfo& charInfo = it->second;
	m_charInfo = charInfo;
	m_userEquip = CSharedDoc::Instance()->m_myInfo.userEquip;
	m_userEquip.guidChar = charInfo.guid;
	m_userEquip.guidCaddie = CEquipHelper::Instance()->GetEquipItemGuid(
		CEquipHelper::CATEGORY_CADDIE, true);
	m_userEquip.guidClubSet = CEquipHelper::Instance()->GetEquipItemGuid(
		CEquipHelper::CATEGORY_CLUB, true);
	m_userEquip.tidBall = CEquipHelper::Instance()->GetEquipItemTid(
		CEquipHelper::CATEGORY_BALL, true);
	m_userEquip.guidMascot = CEquipHelper::Instance()->GetEquipItemGuid(
		CEquipHelper::CATEGORY_MASCOT, true);

	std::map<unsigned int, sItemInfo>::iterator itClub =
		CSharedDoc::Instance()->m_clubSetMap.find(m_userEquip.guidClubSet);
	if (itClub != CSharedDoc::Instance()->m_clubSetMap.end())
		m_clubInfo = itClub->second;

	std::map<unsigned int, sCaddieInfo>::iterator itCaddie =
		CSharedDoc::Instance()->m_caddieMap.find(m_userEquip.guidCaddie);
	if (itCaddie != CSharedDoc::Instance()->m_caddieMap.end())
		m_caddieInfo = itCaddie->second;

	std::map<unsigned int, sMascotInfo>::iterator itMascot =
		CSharedDoc::Instance()->m_mascotMap.find(m_userEquip.guidMascot);
	if (itMascot != CSharedDoc::Instance()->m_mascotMap.end())
		m_mascotInfo = itMascot->second;

	if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/) &&
		IsLocalContent((localContentType_t)0x65 /*983: era 0x64; interpolado*/))
	{
		unsigned char slot = m_petKind + 1;
		CCardManager::Instance()->m_charInfo[slot] = m_charInfo;
		CCardManager::Instance()->m_playerSlot[slot] = slot;
	}

	SetStatus();

	m_anotherHand.Enter();

	m_pExhibition = new CExhibitionByHardwareTnL(m_rect.x + m_rect.w * 0.5f,
		m_rect.y + m_rect.h * 0.5f - 70.0f, 0.35f, false);
	m_pExhibition->SetArea(m_rect.w, m_rect.h);

	if (charInfo.tid == 0x4000005 || charInfo.tid == 0x4000009)
		m_pExhibition->SetDistance(11.0f, true, false);
	else
		m_pExhibition->SetDistance(13.0f, true, false);

	m_pPartTidList = new CPartTidList;
	if (!m_pPartTidList->SetTids(&charInfo, 0xFF) ||
		!m_pPartTidList->IsComboValid())
		m_pPartTidList->SetDefaultTids();

	m_pExhibition->SetModel(*m_pPartTidList, charInfo.tidAuxParts, 0, 3.0f, 0);
	if (Doc()->m_myInfo.info.angelicWingsEffect)
		m_pExhibition->AttachAngelWingFx();

	m_anotherHand.Leave();
}

void CFamilyMain::cFamily::SetStatus()
{
	if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/) &&
		IsLocalContent((localContentType_t)0x65 /*983: era 0x64; interpolado*/))
	{
		CCardManager::Instance()->CalcCardPeriodAndStatus();
		CCardManager::Instance()->SetPlayerIndex(m_petKind);
	}

	if (m_pStatBar)
	{
		delete m_pStatBar;
		m_pStatBar = NULL;
	}
	m_pStatBar = new CStatBar(9.0f, 6.0f);
	CItemManager* pItemManager = ItemManager();
	unsigned char level[5] = { 0 }, penalty[5] = { 0 }, capacity[5] = { 0 };

	for (int i = 0; i < 5; i++)
	{
		status_t status = pItemManager->GetLevelWithPenalty((eLvlType)i,
			m_charInfo.tid, m_charInfo.PCL, m_charInfo.tidParts,
			m_charInfo.tidAuxParts, m_clubInfo.tid, m_clubInfo.Common,
			m_caddieInfo.tid, CSharedDoc::Instance()->m_myInfo.stat.Level);
		level[i] = status.level;
		penalty[i] = status.penalty;
		capacity[i] = pItemManager->GetCapacity((eLvlType)i, m_charInfo.tid,
			m_charInfo.PCL, m_charInfo.tidParts, m_clubInfo.tid,
			m_clubInfo.Common, m_caddieInfo.tid,
			CSharedDoc::Instance()->m_myInfo.stat.Level);
	}

	m_pStatBar->SetDestTime(1.2f);
	for (int i = 0; i < 5; i++)
		m_pStatBar->SetRange((eLvlType)i, level[i], capacity[i], penalty[i]);
}

void CFamilyMain::cFamily::PetRebuild()
{
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
	if (m_pPartTidList)
	{
		delete m_pPartTidList;
		m_pPartTidList = NULL;
	}
	if (m_pStatBar)
	{
		delete m_pStatBar;
		m_pStatBar = NULL;
	}

	SetStatus();

	m_anotherHand.Enter();

	m_pExhibition = new CExhibitionByHardwareTnL(m_rect.x + m_rect.w * 0.5f,
		m_rect.y + m_rect.h * 0.5f - 70.0f, 0.35f, false);
	m_pExhibition->SetArea(m_rect.w, m_rect.h);

	if (m_charInfo.tid == 0x4000005 || m_charInfo.tid == 0x4000009)
		m_pExhibition->SetDistance(11.0f, true, false);
	else
		m_pExhibition->SetDistance(13.0f, true, false);

	m_pPartTidList = new CPartTidList;
	if (!m_pPartTidList->SetTids(&m_charInfo, 0xFF) ||
		!m_pPartTidList->IsComboValid())
		m_pPartTidList->SetDefaultTids();

	m_pExhibition->SetModel(*m_pPartTidList, m_charInfo.tidAuxParts, 0, 3.0f,
		0);
	if (Doc()->m_myInfo.info.angelicWingsEffect)
		m_pExhibition->AttachAngelWingFx();

	m_anotherHand.Leave();
}

IMPLEMENT_ACTOR(CFamilyMain, CTaskMain)

BEGIN_FRESH_MSGMAP(CFamilyMain, CTaskMain)
ON_FRESH_VI("FAMILY", FRCMD_INIT, CFamilyMain::OnInitFamilyMain)
ON_FRESH_VV("FAMILY", FRCMD_FINISH, CFamilyMain::OnInitFamilyFinish)
ON_FRESH_VI("FAMILY.pet_area1", FRCMD_INIT, CFamilyMain::OnInitAreaPet1)
ON_FRESH_VI("FAMILY.pet_area2", FRCMD_INIT, CFamilyMain::OnInitAreaPet2)
ON_FRESH_VI("FAMILY.pet_area3", FRCMD_INIT, CFamilyMain::OnInitAreaPet3)
ON_FRESH_VI("FAMILY.pet_area4", FRCMD_INIT, CFamilyMain::OnInitAreaPet4)
ON_FRESH_VI("FAMILY.globalgui", FRCMD_INIT, CFamilyMain::OnInitAreaGlobal)
ON_FRESH_VV("FAMILY.pet_area1", FRCMD_OWNERDRAW, CFamilyMain::OnDrawAreaPet1)
ON_FRESH_VV("FAMILY.pet_area2", FRCMD_OWNERDRAW, CFamilyMain::OnDrawAreaPet2)
ON_FRESH_VV("FAMILY.pet_area3", FRCMD_OWNERDRAW, CFamilyMain::OnDrawAreaPet3)
ON_FRESH_VV("FAMILY.pet_area4", FRCMD_OWNERDRAW, CFamilyMain::OnDrawAreaPet4)
ON_FRESH_VV("FAMILY.globalgui", FRCMD_OWNERDRAW, CFamilyMain::OnDrawAreaGlobal)
ON_FRESH_VI("FAMILY.start_btn", FRCMD_INIT, CFamilyMain::OnInitBtnStart)
ON_FRESH_VI("FAMILY.course_btn", FRCMD_INIT, CFamilyMain::OnInitBtnSelMap)
ON_FRESH_VI("FAMILY.limit_time_btn", FRCMD_INIT, CFamilyMain::OnInitBtnHoleType)
ON_FRESH_VI("FAMILY.hole_type_btn", FRCMD_INIT, CFamilyMain::OnInitBtnHoleNum)
ON_FRESH_VI("FAMILY.hole_num_btn", FRCMD_INIT, CFamilyMain::OnInitBtnTimeLimit)
ON_FRESH_VI("FAMILY.create_pet3", FRCMD_INIT, CFamilyMain::OnInitCreatePet3)
ON_FRESH_VI("FAMILY.create_pet4", FRCMD_INIT, CFamilyMain::OnInitCreatePet4)
ON_FRESH_VI("FAMILY.delete_pet3", FRCMD_INIT, CFamilyMain::OnInitDeletePet3)
ON_FRESH_VI("FAMILY.delete_pet4", FRCMD_INIT, CFamilyMain::OnInitDeletePet4)
ON_FRESH_VV("FAMILY.start_btn", FRCMD_LBUTTONDOWN, CFamilyMain::OnDownBtnStart)
ON_FRESH_VV("FAMILY.course_btn", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownBtnSelMap)
ON_FRESH_VV("FAMILY.limit_time_btn", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownBtnHoleType)
ON_FRESH_VV("FAMILY.hole_type_btn", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownBtnHoleNum)
ON_FRESH_VV("FAMILY.hole_num_btn", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownBtnTimeLimit)
ON_FRESH_VV("FAMILY.create_pet3", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownCreatePet3)
ON_FRESH_VV("FAMILY.create_pet4", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownCreatePet4)
ON_FRESH_VV("FAMILY.delete_pet3", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownDeletePet3)
ON_FRESH_VV("FAMILY.delete_pet4", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownDeletePet4)
ON_FRESH_VV("FAMILY.start_btn", FRCMD_OWNERDRAW, CFamilyMain::OnDrawBtnStart)
ON_FRESH_VV("FAMILY.course_btn", FRCMD_OWNERDRAW, CFamilyMain::OnDrawBtnSelMap)
ON_FRESH_VV("FAMILY.limit_time_btn", FRCMD_OWNERDRAW,
	CFamilyMain::OnDrawBtnHoleType)
ON_FRESH_VV("FAMILY.hole_type_btn", FRCMD_OWNERDRAW,
	CFamilyMain::OnDrawBtnHoleNum)
ON_FRESH_VV("FAMILY.hole_num_btn", FRCMD_OWNERDRAW,
	CFamilyMain::OnDrawBtnTimeLimit)
ON_FRESH_VI("FAMILY.pet_equip1", FRCMD_INIT, CFamilyMain::OnInitListEquip1)
ON_FRESH_VI("FAMILY.pet_equip2", FRCMD_INIT, CFamilyMain::OnInitListEquip2)
ON_FRESH_VI("FAMILY.pet_equip3", FRCMD_INIT, CFamilyMain::OnInitListEquip3)
ON_FRESH_VI("FAMILY.pet_equip4", FRCMD_INIT, CFamilyMain::OnInitListEquip4)
ON_FRESH_VI("FAMILY.pet_equip1", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownListEquip1)
ON_FRESH_VI("FAMILY.pet_equip2", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownListEquip2)
ON_FRESH_VI("FAMILY.pet_equip3", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownListEquip3)
ON_FRESH_VI("FAMILY.pet_equip4", FRCMD_LBUTTONDOWN,
	CFamilyMain::OnDownListEquip4)
ON_FRESH_VI("FAMILY.pet_equip1", FRCMD_OWNERDRAW, CFamilyMain::OnDrawListEquip1)
ON_FRESH_VI("FAMILY.pet_equip2", FRCMD_OWNERDRAW, CFamilyMain::OnDrawListEquip2)
ON_FRESH_VI("FAMILY.pet_equip3", FRCMD_OWNERDRAW, CFamilyMain::OnDrawListEquip3)
ON_FRESH_VI("FAMILY.pet_equip4", FRCMD_OWNERDRAW, CFamilyMain::OnDrawListEquip4)
ON_FRESH_VI("FAMILY.ubar", FRCMD_INIT, CTaskMain::OnUnderBarInit)
ON_FRESH_VI("FAMILY.ubar.back", FRCMD_INIT, CTaskMain::OnUnderBar_BackInit)
ON_FRESH_VV("FAMILY.ubar.back", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_BackUp)
ON_FRESH_VI("FAMILY.ubar.back", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.back", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.front", FRCMD_INIT, CTaskMain::OnUnderBar_FrontInit)
ON_FRESH_VV("FAMILY.ubar.front", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_FrontUp)
ON_FRESH_VI("FAMILY.ubar.front", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.front", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.shop", FRCMD_INIT, CTaskMain::OnUnderBar_ShopInit)
ON_FRESH_VV("FAMILY.ubar.shop", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_ShopUp)
ON_FRESH_VI("FAMILY.ubar.shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.shop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.myroom", FRCMD_INIT, CTaskMain::OnUnderBar_MyRoomInit)
ON_FRESH_VV("FAMILY.ubar.myroom", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MyRoomUp)
ON_FRESH_VI("FAMILY.ubar.myroom", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.myroom", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (C40.3, s110): "ubar.achievement" no lugar do "ubar.itemstorage" (mapa do 983: 00651e30 / 006582f0 / hover)
ON_FRESH_VI("FAMILY.ubar.achievement", FRCMD_INIT, CTaskMain::OnUnderBar_AchievementInit983)
ON_FRESH_VV("FAMILY.ubar.achievement", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_AchievementUp983)
ON_FRESH_VI("FAMILY.ubar.achievement", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.achievement", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.gift", FRCMD_INIT, CTaskMain::OnUnderBar_GiftInit)
ON_FRESH_VV("FAMILY.ubar.gift", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_GiftUp)
ON_FRESH_VI("FAMILY.ubar.gift", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.gift", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.cookie", FRCMD_INIT, CTaskMain::OnUnderBar_CookieInit)
ON_FRESH_VV("FAMILY.ubar.cookie", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_CookieUp)
ON_FRESH_VI("FAMILY.ubar.cookie", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.cookie", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.game", FRCMD_INIT, CTaskMain::OnUnderBar_GameInit)
ON_FRESH_VV("FAMILY.ubar.game", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_GameUp)
ON_FRESH_VI("FAMILY.ubar.game", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.game", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.server", FRCMD_INIT, CTaskMain::OnUnderBar_ServerInit)
ON_FRESH_VV("FAMILY.ubar.server", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_ServerUp)
ON_FRESH_VI("FAMILY.ubar.server", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.server", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.exit", FRCMD_INIT, CTaskMain::OnUnderBar_ExitInit)
ON_FRESH_VV("FAMILY.ubar.exit", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_ExitUp)
ON_FRESH_VI("FAMILY.ubar.exit", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.ubar.exit", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.ubar.tabbtn_b", FRCMD_INIT,
	CTaskMain::OnUnderBar_TabBtnInit)
ON_FRESH_VV("FAMILY.ubar.tabbtn_b", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_TabBtnUp)
ON_FRESH_VI("FAMILY.utab", FRCMD_INIT, CTaskMain::OnUnderTab_Init)
ON_FRESH_VI("FAMILY.utab.magicbox", FRCMD_INIT,
	CTaskMain::OnUnderBar_MagicBoxInit)
ON_FRESH_VV("FAMILY.utab.magicbox", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_MagicBoxUp)
ON_FRESH_VI("FAMILY.utab.magicbox", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.magicbox", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s106): "utab.combine" e "utab.gacha" (mapa do 983: init, LBUTTONDOWN, hover)
ON_FRESH_VI("FAMILY.utab.combine", FRCMD_INIT, CTaskMain::OnUnderBar_CombineInit983)
ON_FRESH_VV("FAMILY.utab.combine", FRCMD_LBUTTONDOWN, CTaskMain::OnUnderBar_CombineUp983)
ON_FRESH_VI("FAMILY.utab.combine", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.combine", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.utab.gacha", FRCMD_INIT, CTaskMain::OnUnderBar_GachaInit983)
ON_FRESH_VV("FAMILY.utab.gacha", FRCMD_LBUTTONDOWN, CTaskMain::OnUnderBar_GachaUp983)
ON_FRESH_VI("FAMILY.utab.gacha", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.gacha", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.utab.bongdarishop", FRCMD_INIT,
	CTaskMain::OnUnderBar_BongdariShopInit)
ON_FRESH_VV("FAMILY.utab.bongdarishop", FRCMD_LBUTTONDOWN,
	CTaskMain::OnUnderBar_BongdariShopUp)
ON_FRESH_VI("FAMILY.utab.bongdarishop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.bongdarishop", FRCMD_HOVEROFF,
	CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.utab.ranking", FRCMD_INIT,
	CTaskMain::OnUnderBar_RankingInit)
ON_FRESH_VV("FAMILY.utab.ranking", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_RankingUp)
ON_FRESH_VI("FAMILY.utab.ranking", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.ranking", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.utab.guild", FRCMD_INIT, CTaskMain::OnUnderBar_GuildInit)
ON_FRESH_VV("FAMILY.utab.guild", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_GuildUp)
ON_FRESH_VI("FAMILY.utab.guild", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.guild", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.utab.notice", FRCMD_INIT, CTaskMain::OnUnderBar_NoticeInit)
ON_FRESH_VV("FAMILY.utab.notice", FRCMD_LBUTTONUP,
	CTaskMain::OnUnderBar_NoticeUp)
ON_FRESH_VI("FAMILY.utab.notice", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.notice", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s111): "utab.point_shop" (AlchemyDlg) e "utab.card_compose" (alquimia de cartas), LBUTTONUP
ON_FRESH_VI("FAMILY.utab.point_shop", FRCMD_INIT, CTaskMain::OnUnderBar_PointShopInit983)
ON_FRESH_VV("FAMILY.utab.point_shop", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_PointShopUp983)
ON_FRESH_VI("FAMILY.utab.point_shop", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.point_shop", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.utab.card_compose", FRCMD_INIT, CTaskMain::OnUnderBar_CardComposeInit983)
ON_FRESH_VV("FAMILY.utab.card_compose", FRCMD_LBUTTONUP, CTaskMain::OnUnderBar_CardComposeUp983)
ON_FRESH_VI("FAMILY.utab.card_compose", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.utab.card_compose", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
// 983 (s111): sem "utab.tikimagicbox", "utab.scratch", "utab.maket", "obar.messenger_alarm" e "obar.gift_alarm"
ON_FRESH_VI("FAMILY.obar", FRCMD_INIT, CTaskMain::OnOverBar_Init)
ON_FRESH_VI("FAMILY.obar.title", FRCMD_INIT, CTaskMain::OnOverBar_TitleInit)
ON_FRESH_VI("FAMILY.obar.site", FRCMD_INIT, CTaskMain::OnOverBar_SiteInit)
ON_FRESH_VI("FAMILY.obar.char", FRCMD_INIT, CTaskMain::OnOverBar_CharInit)
ON_FRESH_VV("FAMILY.obar.char", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CharDown)
ON_FRESH_VI("FAMILY.obar.char", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharOwnerDraw)
ON_FRESH_VI("FAMILY.obar.caddie", FRCMD_INIT, CTaskMain::OnOverBar_CaddieInit)
ON_FRESH_VV("FAMILY.obar.caddie", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_CaddieDown)
ON_FRESH_VI("FAMILY.obar.caddie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieOwnerDraw)
ON_FRESH_VI("FAMILY.obar.club", FRCMD_INIT, CTaskMain::OnOverBar_ClubInit)
ON_FRESH_VV("FAMILY.obar.club", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_ClubDown)
ON_FRESH_VI("FAMILY.obar.club", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubOwnerDraw)
ON_FRESH_VI("FAMILY.obar.aztec", FRCMD_INIT, CTaskMain::OnOverBar_AztecInit)
ON_FRESH_VV("FAMILY.obar.aztec", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_AztecDown)
ON_FRESH_VI("FAMILY.obar.aztec", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecOwnerDraw)
ON_FRESH_VI("FAMILY.obar.mascot", FRCMD_INIT, CTaskMain::OnOverBar_MascotInit)
ON_FRESH_VV("FAMILY.obar.mascot", FRCMD_LBUTTONDOWN,
	CTaskMain::OnOverBar_MascotDown)
ON_FRESH_VI("FAMILY.obar.mascot", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotOwnerDraw)
ON_FRESH_VI("FAMILY.obar.char_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CharSelInit)
ON_FRESH_VI("FAMILY.obar.char_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CharSelOwnerDraw)
ON_FRESH_VI("FAMILY.obar.caddie_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_CaddieSelInit)
ON_FRESH_VI("FAMILY.obar.caddie_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_CaddieSelOwnerDraw)
ON_FRESH_VI("FAMILY.obar.club_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_ClubSelInit)
ON_FRESH_VI("FAMILY.obar.club_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_ClubSelOwnerDraw)
ON_FRESH_VI("FAMILY.obar.aztec_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_AztecSelInit)
ON_FRESH_VI("FAMILY.obar.aztec_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_AztecSelOwnerDraw)
ON_FRESH_VI("FAMILY.obar.mascot_sel", FRCMD_INIT,
	CTaskMain::OnOverBar_MascotSelInit)
ON_FRESH_VI("FAMILY.obar.mascot_sel", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_MascotSelOwnerDraw)
ON_FRESH_VI("FAMILY.obar.pang_cookie", FRCMD_INIT,
	CTaskMain::OnOverBar_PangCookieInit)
ON_FRESH_VI("FAMILY.obar.pang_cookie", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_PangCookieOwnerDraw)
ON_FRESH_VI("FAMILY.obar.onelinemsg_req", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineReqBtnInit)
ON_FRESH_VV("FAMILY.obar.onelinemsg_req", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OneLineReqBtnUp)
ON_FRESH_VI("FAMILY.obar.onelineview", FRCMD_INIT,
	CTaskMain::OnOverBar_OneLineViewInit)
ON_FRESH_VI("FAMILY.obar.onelineview", FRCMD_OWNERDRAW,
	CTaskMain::OnOverBar_OneLineViewOwnerDraw)
ON_FRESH_VI("FAMILY.obar.spcard", FRCMD_INIT,
	CTaskMain::OnOverBar_SpecialCardInit)
ON_FRESH_VV("FAMILY.obar.spcard", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_SpecialCardUp)
ON_FRESH_VI("FAMILY.obar.spcard", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.obar.spcard", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.obar.messenger", FRCMD_INIT,
	CTaskMain::OnOverBar_MessengerInit)
ON_FRESH_VV("FAMILY.obar.messenger", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MessengerUp)
ON_FRESH_VI("FAMILY.obar.messenger", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.obar.messenger", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.obar.myinfo", FRCMD_INIT, CTaskMain::OnOverBar_MyInfoInit)
ON_FRESH_VV("FAMILY.obar.myinfo", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_MyInfoUp)
ON_FRESH_VI("FAMILY.obar.myinfo", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.obar.myinfo", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.obar.option", FRCMD_INIT, CTaskMain::OnOverBar_OptionInit)
ON_FRESH_VV("FAMILY.obar.option", FRCMD_LBUTTONUP,
	CTaskMain::OnOverBar_OptionUp)
ON_FRESH_VI("FAMILY.obar.option", FRCMD_HOVERON, CTaskMain::OnBarHoverOn)
ON_FRESH_VI("FAMILY.obar.option", FRCMD_HOVEROFF, CTaskMain::OnBarHoverOff)
ON_FRESH_VI("FAMILY.obar.messenger_bubble", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleInit)
ON_FRESH_VI("FAMILY.obar.messenger_bubble", FRCMD_OWNERDRAW,
	CTaskMain::OnMessengerBubbleOwnerDraw)
ON_FRESH_VI("FAMILY.obar.messenger_desc", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleEditInit)
ON_FRESH_VI("FAMILY.obar.tooltip", FRCMD_OWNERDRAW,
	CTaskMain::OnBarTooltipOwnerDraw)
END_FRESH_MSGMAP()

unsigned char CFamilyMain::m_mapIndex = 0xff;

void CFamilyMain::OnInitFamilyMain(int param)
{
	TurnOff3DBackground();
	CSharedDoc::Instance()->m_gameMode = 1;

	m_families.reserve(4);
	m_areas.reserve(5);
	m_buttons.reserve(9);
	m_equipLists.reserve(4);
	m_families.assign(4, (cFamily*)NULL);
	m_areas.assign(5, (FrArea*)NULL);
	m_buttons.assign(9, (FrButton*)NULL);
	m_equipLists.assign(4, (FrListBox*)NULL);

	sUserInfo info;
	for (int i = 1; i < 4; i++)
		CSharedDoc::Instance()->m_userInfo[i] =
			CSharedDoc::Instance()->m_userInfo[0];

	m_timeLimit = 0;
	if (m_mapIndex == 0xff)
		m_mapIndex = rand() % 22; // 983 00b1bfa0 (645: 20)
	if (m_mapIndex == 0x11 || m_mapIndex == 0xc)
		m_mapIndex = 0;

	while (ItemManager()->FindCourse(0x28000000 | m_mapIndex) == NULL)
	{
		m_mapIndex++;
		if (m_mapIndex >= 22)
			m_mapIndex = 0;
	}

	m_holeNum = 3;
	m_holeType = 0;
	m_timeLimit = 0;
	CSharedDoc::Instance()->m_golfGame.gameType = 0;
	strcpy(CSharedDoc::Instance()->m_golfGame.gameTypeName,
		CSharedDoc::Instance()->m_gameTypeInfo[0].name);
	CSharedDoc::Instance()->m_bGameOver = false;

	int count = (int)CSharedDoc::Instance()->m_charMap.size();
	m_petList.reserve(count);
	for (int i = 0; i < count; i++)
		m_petList.push_back(i);
	std::random_shuffle(m_petList.begin(), m_petList.end());
}

void CFamilyMain::OnInitFamilyFinish()
{
	m_pOverBarTitle->SetBgImg("title_family_renew");

	for (int i = 0; i < 2; i++)
	{
		m_families[i] = new cFamily;
		m_families[i]->Init(m_areas[i]->GetRect(), m_petList, (unsigned char)i);

		cFamily* pFamily = m_families[i];
		FrListBox* pList = m_equipLists[i];
		pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
			CEquipHelper::CATEGORY_CHARACTER, pFamily->GetUserEquip()));
		pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
			CEquipHelper::CATEGORY_CADDIE, pFamily->GetUserEquip()));
		pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
			CEquipHelper::CATEGORY_CLUB, pFamily->GetUserEquip()));
		pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
			CEquipHelper::CATEGORY_BALL, pFamily->GetUserEquip()));
		pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
			CEquipHelper::CATEGORY_MASCOT, pFamily->GetUserEquip()));
		pList->AddItem(NULL);

		WRect rect = pList->GetRect();
		rect.x = pFamily->GetFramePos().x;
		rect.y = pFamily->GetFramePos().y;
		pList->SetRect(rect);
	}

	TurnOffControls();
	m_pUnderBarBongdariShop->Enable(true);
}

void CFamilyMain::TurnOffControls()
{
	m_pUnderBarMagicBox->Enable(false);
	m_pUnderBarShop->Enable(false);
	m_pUnderBarMyRoom->Enable(false);
	m_pUnderBarItemStorage->Enable(false);
	m_pUnderBarCookie->Enable(false);
	m_pUnderBarServer->Enable(false);
	m_pUnderBarRanking->Enable(false);
	m_pUnderBarGuild->Enable(false);
	m_pOverBarSpecialCard->Enable(false);
	m_pUnderBarGift->Enable(false);
	m_pUnderBarGame->Enable(false);
	// 983 00b16c40: +0x70 gacha e +0x74 combine (FAMILY.utab)
	m_pUnderBarScratch->Enable(false);
	m_pUnderBarCombine->Enable(false);

	m_pOverBarChar->SetVisible(false);
	m_pOverBarCaddie->SetVisible(false);
	m_pOverBarAztec->SetVisible(false);
	m_pOverBarMascot->SetVisible(false);
	m_pOverBarClub->SetVisible(false);
}

void CFamilyMain::OnInitAreaPet1(int param)
{
	m_areas[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CFamilyMain::OnInitAreaPet2(int param)
{
	m_areas[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CFamilyMain::OnInitAreaPet3(int param)
{
	m_areas[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CFamilyMain::OnInitAreaPet4(int param)
{
	m_areas[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CFamilyMain::OnInitAreaGlobal(int param)
{
	m_areas[4] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CFamilyMain::OnDrawAreaPet1()
{
	if (g_pFresh->GetManager()->GetGDI() == 0)
		return;

	if (m_families[0])
		m_families[0]->Render();
}

void CFamilyMain::OnDrawAreaPet2()
{
	if (g_pFresh->GetManager()->GetGDI() == 0)
		return;

	if (m_families[1])
		m_families[1]->Render();
}

void CFamilyMain::OnDrawAreaPet3()
{
	if (g_pFresh->GetManager()->GetGDI() == 0)
		return;

	if (m_families[2])
		m_families[2]->Render();
}

void CFamilyMain::OnDrawAreaPet4()
{
	if (g_pFresh->GetManager()->GetGDI() == 0)
		return;

	if (m_families[3])
		m_families[3]->Render();
}

void CFamilyMain::OnDrawAreaGlobal()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
}

void CFamilyMain::OnInitBtnStart(int param)
{
	m_buttons[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CFamilyMain::OnInitBtnSelMap(int param)
{
	m_buttons[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CFamilyMain::OnInitBtnHoleType(int param)
{
	m_buttons[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_buttons[2]->SetPushDelay(0.1f);
}

void CFamilyMain::OnInitBtnHoleNum(int param)
{
	m_buttons[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_buttons[3]->SetPushDelay(0.1f);
}

void CFamilyMain::OnInitBtnTimeLimit(int param)
{
	m_buttons[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_buttons[4]->SetPushDelay(0.1f);
}

void CFamilyMain::OnInitCreatePet3(int param)
{
	m_buttons[5] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CFamilyMain::OnInitCreatePet4(int param)
{
	m_buttons[6] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CFamilyMain::OnInitDeletePet3(int param)
{
	m_buttons[7] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_buttons[7]->SetVisible(false);
}

void CFamilyMain::OnInitDeletePet4(int param)
{
	m_buttons[8] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_buttons[8]->SetVisible(false);
}

void CFamilyMain::OnDownBtnSelMap()
{
	if (m_pSelMapDlg == 0)
	{
		m_pSelMapDlg = CreateForm<FrTreasureCourse>(g_pFresh->GetManager(),
			this, "treasuremap", 0);
		if (m_pSelMapDlg)
		{
			m_pSelMapDlg->SetSelectMap(m_mapIndex);
			m_pSelMapDlg->SetMapIndexChanged(false);
			m_pSelMapDlg->EnableDrag(false);
			m_pSelMapDlg->EnableRandomBtn(false);

			m_pSelMapDlg->Open((FRESH_PFN_RESULT)&CFamilyMain::OnResultSelMap,
				3);
		}
	}
}

bool CFamilyMain::OnResultSelMap(int, FrForm*)
{
	if (m_pSelMapDlg)
	{
		m_mapIndex = m_pSelMapDlg->GetCurMapIdx();
		SetCurMap(m_mapIndex);
		m_pSelMapDlg = 0;
	}
	return true;
}

void CFamilyMain::OnDownBtnHoleType()
{
	m_holeType++;

	if (m_holeType > 3)
		m_holeType = 0;

	Doc()->m_holeType = m_holeType;
}

void CFamilyMain::OnDownBtnHoleNum()
{
	unsigned char holes[4] = { 3, 6, 9, 18 };

	for (int i = 0; i < 4; ++i)
	{
		if (holes[i] == m_holeNum)
		{
			++i;
			if (i >= 4)
				i = 0;

			m_holeNum = holes[i];
			break;
		}
	}

	Doc()->m_golfGame.holes = m_holeNum;
}

void CFamilyMain::OnDownBtnTimeLimit()
{
	unsigned long limits[6] = { 0, 30000, 40000, 50000, 60000, 90000 };

	for (int i = 0; i < 6; ++i)
	{
		if (limits[i] == m_timeLimit)
		{
			++i;
			if (i >= 6)
				i = 0;

			m_timeLimit = limits[i];
			break;
		}
	}

	Doc()->m_golfGame.shotTimeLimit = m_timeLimit;
}

void CFamilyMain::OnDownCreatePet3()
{
	if (m_bCreatingPet)
		return;

	m_bCreatingPet = true;
	m_buttons[5]->SetVisible(false);

	m_families[2] = new cFamily;
	m_families[2]->Init(m_areas[2]->GetRect(), m_petList, 2);

	cFamily* pFamily = m_families[2];
	FrListBox* pList = m_equipLists[2];
	m_equipLists[2]->SetVisible(true);
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_CHARACTER, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_CADDIE, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_CLUB, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_BALL, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_MASCOT, pFamily->GetUserEquip()));
	pList->AddItem(NULL);

	WRect rect = pList->GetRect();
	rect.x = pFamily->GetFramePos().x;
	rect.y = pFamily->GetFramePos().y;
	pList->SetRect(rect);

	m_bCreatingPet = false;
	m_buttons[7]->SetVisible(true);
}

void CFamilyMain::OnDownCreatePet4()
{
	if (m_bCreatingPet)
		return;

	m_bCreatingPet = true;
	m_buttons[6]->SetVisible(false);

	m_families[3] = new cFamily;
	m_families[3]->Init(m_areas[3]->GetRect(), m_petList, 3);

	cFamily* pFamily = m_families[3];
	FrListBox* pList = m_equipLists[3];
	m_equipLists[3]->SetVisible(true);
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_CHARACTER, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_CADDIE, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_CLUB, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_BALL, pFamily->GetUserEquip()));
	pList->AddItem(CEquipHelper::Instance()->GetItemCommon(
		CEquipHelper::CATEGORY_MASCOT, pFamily->GetUserEquip()));
	pList->AddItem(NULL);

	WRect rect = pList->GetRect();
	rect.x = pFamily->GetFramePos().x;
	rect.y = pFamily->GetFramePos().y;
	pList->SetRect(rect);

	m_bCreatingPet = false;
	m_buttons[8]->SetVisible(true);
}

void CFamilyMain::OnDownDeletePet3()
{
	cFamily* family = m_families[2];
	delete family;
	m_families[2] = 0;

	m_buttons[7]->SetVisible(false);
	m_buttons[5]->SetVisible(true);
	m_buttons[5]->EnableHover(true, 0.0f);
	m_equipLists[2]->ClearItem();
	m_equipLists[2]->SetVisible(false);
}

void CFamilyMain::OnDownDeletePet4()
{
	cFamily* family = m_families[3];
	delete family;
	m_families[3] = 0;

	m_buttons[8]->SetVisible(false);
	m_buttons[6]->SetVisible(true);
	m_buttons[6]->EnableHover(true, 0.0f);
	m_equipLists[3]->ClearItem();
	m_equipLists[3]->SetVisible(false);
}

void CFamilyMain::OnDrawBtnStart()
{
}

void CFamilyMain::OnDrawBtnSelMap()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
		return;

	IFF_STRUCT::sCourse* course =
		ItemManager()->FindCourse(0x28000000 | m_mapIndex);
	if (course == 0)
		return;

	std::string name("mid_");
	name += course->c.Icon;
	const Bitmap* icon = g_pFresh->GetBitmap(name.c_str());

	if (icon)
	{
		WRect rect = m_buttons[1]->GetRect();
		rect.x += (rect.w - icon->Width() + 1.0f) * 0.5f;
		rect.y += (rect.h - icon->Height() + 1.0f) * 0.5f;
		rect.w = (float)icon->Width();
		rect.h = (float)icon->Height();

		pGDI->DrawTexture(icon, rect, 0xFFFFFFFF, 0);
	}
}

void CFamilyMain::OnDrawBtnHoleType()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
		return;

	const WRect& rect = m_buttons[2]->GetRect();

	pGDI->SetTextColor(0xFFFFFFFF, 0xFF000000);
	pGDI->SetTextStyle(4);

	std::string holeTypes[4] = { "FRONT", "BACK", "RANDOM", "SHUFFLE" };
	pGDI->PrintText(WPoint(rect.x + 15.0f, rect.y + 2.0f), 0,
		MakeStr(K2L_Compatibility("\xc1\xf8\xc7\xe0\xb9\xe6\xbd\xc4:%s"),
			holeTypes[m_holeType].c_str()),
		0);
	pGDI->SetTextStyle(0);
}

void CFamilyMain::OnDrawBtnHoleNum()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
		return;

	const WRect& rect = m_buttons[3]->GetRect();

	pGDI->SetTextColor(0xFFFFFFFF, 0xFF000000);
	pGDI->SetTextStyle(4);
	pGDI->PrintText(WPoint(rect.x + 15.0f, rect.y + 2.0f), 0,
		MakeStr(K2L_Compatibility("\xc8\xa6 \xbc\xf6 : %d \xc8\xa6"), m_holeNum), 0);
	pGDI->SetTextStyle(0);
}

void CFamilyMain::OnDrawBtnTimeLimit()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == 0)
		return;

	const WRect& rect = m_buttons[4]->GetRect();

	pGDI->SetTextColor(0xFFFFFFFF, 0xFF000000);
	pGDI->SetTextStyle(4);
	unsigned long seconds = m_timeLimit / 1000;
	pGDI->PrintText(WPoint(rect.x + 15.0f, rect.y + 2.0f), 0,
		seconds == 0
			? K2L_Compatibility("\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3:\xb9\xab\xc1\xa6\xc7\xd1")
			: MakeStr(K2L_Compatibility("\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3:%d\xc3\xca"), seconds),
		0);
	pGDI->SetTextStyle(0);
}

void CFamilyMain::OnInitListEquip1(int param)
{
	m_equipLists[0] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CFamilyMain::OnInitListEquip2(int param)
{
	m_equipLists[1] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CFamilyMain::OnInitListEquip3(int param)
{
	m_equipLists[2] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CFamilyMain::OnInitListEquip4(int param)
{
	m_equipLists[3] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CFamilyMain::OnDownListEquip1(int)
{
	OnDownEquipListBox(0);
}

void CFamilyMain::OnDownListEquip2(int)
{
	OnDownEquipListBox(1);
}

void CFamilyMain::OnDownListEquip3(int)
{
	OnDownEquipListBox(2);
}

void CFamilyMain::OnDownListEquip4(int)
{
	OnDownEquipListBox(3);
}

void CFamilyMain::OnDownEquipListBox(int index)
{
	FrListItem* pItem = m_equipLists[index]->GetSelected();
	if (pItem == NULL)
		return;

	bool bOpen = false;
	if (pItem->pData == NULL)
	{
		if (pItem->idx == 5)
		{
			m_curPet = index;
			FrEquipDlg* pDlg = CreateForm<FrEquipDlg>(g_pFresh->GetManager(),
				this, "equip", NULL);
			if (pDlg)
			{
				pDlg->Init(ms_itemslotBackup[m_curPet], true);
				pDlg->Open((FRESH_PFN_RESULT)&CFamilyMain::OnItemEquipResult,
					3);
			}
		}
		else
		{
			switch (pItem->idx)
			{
			case 4:
				if (CSharedDoc::Instance()->m_mascotMap.size())
					bOpen = true;
				break;
			case 1:
				if (CSharedDoc::Instance()->m_caddieMap.size())
					bOpen = true;
				break;
			}
		}
	}
	else
	{
		bOpen = true;
	}

	if (m_pEquipBar == NULL && bOpen)
	{
		m_pCurItem = pItem;
		m_curPet = index;
		m_pEquipBar = CreateForm<FrSubEquipBar>(g_pFresh->GetManager(), this,
			"EQUIPBAR_SUB", NULL);
		if (m_pEquipBar)
		{
			FrSubEquipBar::eCategory category =
				FrSubEquipBar::CATEGORY_CHARACTER;
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
			m_pEquipBar->Open(
				(FRESH_PFN_RESULT)&CFamilyMain::OnResultSubEquipBar, category,
				pos, FrSubEquipBar::SUBTYPE_2);

			WRect rect = m_pEquipBar->GetRect();
			rect.y -= rect.h;
			if (rect.x + rect.w + 5.0f > g_view->GetWidth())
				rect.x = g_view->GetWidth() - rect.w - 5.0f;
			m_pEquipBar->SetRect(rect);
		}
	}
}

bool CFamilyMain::OnItemEquipResult(int result, FrForm* pForm)
{
	if (result == 1 || result == 2)
	{
		cFamily* family = m_families[m_curPet];
		family->SetItemSlot(((FrEquipDlg*)pForm)->GetMyItemSlot());

		memcpy(ms_itemslotBackup[m_curPet],
			((FrEquipDlg*)pForm)->GetMyItemSlot(),
			sizeof(ms_itemslotBackup[0]));
	}
	return true;
}

bool CFamilyMain::OnResultSubEquipBar(int result, FrForm* pForm)
{
	if (result == 1)
	{
		cFamily* pFamily = m_families[m_curPet];
		IFF_ITEM_COMMON* pResult = m_pEquipBar->GetResult();
		if (pResult)
		{
			int group = pResult->TypeId >> 26;
			if (group == CEquipHelper::CATEGORY_BALL)
			{
				pFamily->SetUserEquip(group, pResult->TypeId);
				m_pCurItem->pData = pResult;
			}
			else
			{
				pFamily->SetUserEquip(group,
					m_pEquipBar->GetLastSelectedGuid());
				m_pCurItem->pData = pResult;
			}
		}
		else
		{
			switch (m_pEquipBar->GetCategory())
			{
			case FrSubEquipBar::CATEGORY_MASCOT:
				pFamily->SetUserEquip(CEquipHelper::CATEGORY_MASCOT, 0);
				m_pCurItem->pData = NULL;
				break;
			case FrSubEquipBar::CATEGORY_CADDIE:
				pFamily->SetUserEquip(CEquipHelper::CATEGORY_CADDIE, 0);
				m_pCurItem->pData = NULL;
				break;
			}
		}
	}

	m_pEquipBar = NULL;
	m_pCurItem = NULL;
	return true;
}

void CFamilyMain::OnDrawListEquip1(int param)
{
	OnDrawEquipListBox((FrListItem*)param);
}

void CFamilyMain::OnDrawListEquip2(int param)
{
	OnDrawEquipListBox((FrListItem*)param);
}

void CFamilyMain::OnDrawListEquip3(int param)
{
	OnDrawEquipListBox((FrListItem*)param);
}

void CFamilyMain::OnDrawListEquip4(int param)
{
	OnDrawEquipListBox((FrListItem*)param);
}

void CFamilyMain::OnDrawEquipListBox(FrListItem* pItem)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pItem == 0 || pGDI == 0)
		return;

	const Bitmap* bar;
	IFF_ITEM_COMMON* item = (IFF_ITEM_COMMON*)pItem->pData;
	if (item)
	{
		bar = g_pFresh->GetBitmap(MakeStr("bar_%s", item->Icon));
	}
	else
	{
		bar = g_pFresh->GetBitmap("bar_no");
	}

	if (bar == 0)
		return;

	WRect rect(pItem->pos.x + 5.0f, pItem->pos.y + 5.0f,
		(float)bar->Width() + 0.5f, (float)bar->Height() + 0.5f);
	if (pItem->idx != 5)
	{
		pGDI->DrawTexture(bar, rect, 0xFFFFFFFF, 0);
	}
	else
	{
		const Bitmap* button = g_pFresh->GetBitmap("gameroom_equip_item_bn");
		if (button)
			pGDI->DrawTexture(button, rect, 0xFFFFFFFF, 0);
	}

	if (rect.IsInRect(
			WPoint(g_input->GetMousePoint().x, g_input->GetMousePoint().y)))
	{
		const Bitmap* frame = g_pFresh->GetBitmap("select_frame");
		if (frame)
			pGDI->DrawTexture(frame,
				WRect(rect.x - 3.0f, rect.y - 3.0f, rect.w + 6.0f,
					rect.h + 6.0f),
				0xFFFFFFFF, 0);
	}
}

void CFamilyMain::OnDownBtnStart()
{
	Doc()->m_golfGame.shotTimeLimit = m_timeLimit;
	SetCurMap(m_mapIndex);
	Doc()->m_golfGame.gameType = 0;
	strcpy(Doc()->m_golfGame.gameTypeName, Doc()->m_gameTypeInfo[0].name);
	unsigned char holes = m_holeNum;
	Doc()->m_golfGame.holes = holes;
	unsigned char holeType = m_holeType;
	Doc()->m_holeType = holeType;
	Doc()->m_slotList.clear();

	if ((((m_mapIndex > 0x7f && m_mapIndex != 0xfd) ? m_mapIndex - 0x80
													: m_mapIndex) |
			0x28000000) == 17 ||
		(((m_mapIndex > 0x7f && m_mapIndex != 0xfd) ? m_mapIndex - 0x80
													: m_mapIndex) |
			0x28000000) == 12)
	{
		m_mapIndex = 0;
		SetCurMap(0);
	}

	for (int i = 0; i < 4; ++i)
	{
		sSlotInfo slot;
		memset(&slot, 0, sizeof(slot));
		memset(&Doc()->m_userInfo[i], 0, sizeof(sUserInfo));

		cFamily* pFamily = m_families[i];
		if (pFamily == NULL)
		{
			slot.dwGuid = (unsigned long)-1;
			memset(ms_itemslotBackup[i], 0, sizeof(ms_itemslotBackup[i]));
			Doc()->m_slotList.push_back(slot);
			continue;
		}

		pFamily->ConvertUserInfo(Doc()->m_userInfo[i]);
		slot.connectionRank = i + 1;
		slot.dwGuid = i;
		slot.level = Doc()->m_myInfo.stat.Level;
		strncpy(slot.sNick, MakeStr("PLAYER%d", i + 1), 21);
		strcpy(Doc()->m_userInfo[i].info.sNick, MakeStr("PLAYER%d", i + 1));
		memcpy(ms_itemslotBackup[i], Doc()->m_userInfo[i].userEquip.tidItemSlot,
			sizeof(ms_itemslotBackup[i]));

		IFF_STRUCT::sCaddie* pCaddie =
			ItemManager()->FindCaddie(Doc()->m_userInfo[i].caddieInfo.tid);
		if (pCaddie != NULL &&
			(Doc()->m_userInfo[i].stat.Level < pCaddie->c.Level ||
				(Doc()->m_userInfo[i].caddieInfo.Rent_flag &&
					!Doc()->m_userInfo[i].caddieInfo.Remain_Date &&
					pCaddie->MonthlyFee > 0)))
		{
			Doc()->m_userInfo[i].caddieInfo.tid = 0;
		}

		IFF_STRUCT::sClubSet* pClubSet =
			ItemManager()->FindClubSet(Doc()->m_userInfo[i].clubInfo.tid);
		if ((pClubSet != NULL &&
				Doc()->m_userInfo[i].stat.Level < pClubSet->c.Level &&
				!pClubSet->c.IsUnderLvl) ||
			(Doc()->m_userInfo[i].stat.Level > pClubSet->c.Level &&
				pClubSet->c.IsUnderLvl))
		{
			Doc()->m_userInfo[i].clubInfo.tid = 0x10000000;
		}

		Doc()->m_slotList.push_back(slot);
	}

	switch (Doc()->m_holeType)
	{
	case 2:
	{
		int hole = rand() % 18;
		for (int i = 0; i < 18; ++i)
		{
			Doc()->m_holeOrder[i] = hole++ + 1;
			if (hole >= 18)
				hole = 0;
		}
	}
	break;

	case 0:
	{
		for (int i = 0; i < 18; ++i)
			Doc()->m_holeOrder[i] = i + 1;
	}
	break;

	case 1:
	{
		int hole = 9;
		for (int i = 0; i < 18; ++i)
		{
			Doc()->m_holeOrder[i] = hole++ + 1;
			if (hole >= 18)
				hole = 0;
		}
	}
	break;

	case 3:
	{
		for (unsigned char i = 0; i < 18; ++i)
			Doc()->m_holeOrder[i] = i + 1;
		std::random_shuffle(Doc()->m_holeOrder, Doc()->m_holeOrder + 17);
	}
	break;
	}

	for (int i = 0; i < 18; ++i)
		Doc()->m_courseMap[i] = rand() % 3;

	Doc()->m_gameMode = 1;
	CMouseCursor::Instance()->SetMode(1);
	CMouseCursor::Instance()->SetActive(false);
	CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);

	WSendPacket packet((enumClientPacket)0x5a);
	packet.Encode1(1);
	packet.Send(TO_GAME);
}

CFamilyMain::CFamilyMain()
	: m_pSelMapDlg(NULL), m_pEquipBar(NULL), m_bCreatingPet(false)
{
	m_curPet = 0;
	CSharedDoc::Instance()->m_gameMode = 1;
	g_pFresh->GetManager()->SetExclusiveKey(true);
}

CFamilyMain::~CFamilyMain()
{
	g_pFresh->GetManager()->SetExclusiveKey(false);
	this << MsgObject(0, M983(0x2), 0, 0, 0, 0, 0);
}

void CFamilyMain::OnPreLoadInit()
{
	CTaskMain::OnPreLoadInit();

	CMouseCursor::Instance()->SetAutoHide(false);
}

void CFamilyMain::OnLoad()
{
	CTaskMain::OnLoad();
}

void CFamilyMain::OnInit()
{
	CTaskMain::OnInit();
}

void CFamilyMain::OnDestroy()
{
	std::for_each(m_families.begin(), m_families.end(), Delete_Object());
	m_families.clear();
	m_areas.clear();
	m_buttons.clear();

	CTaskMain::OnDestroy();
}

void CFamilyMain::OnProcess(float delta)
{
	for (std::vector<cFamily*>::iterator it = m_families.begin();
		it != m_families.end(); ++it)
	{
		if (*it)
			(*it)->Process(delta);
	}

	CTaskMain::OnProcess(delta);
}

void CFamilyMain::OnDisplay()
{
}

void CFamilyMain::ReFresh(bool)
{
}

void CFamilyMain::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0:
	{
		if (g_pFresh == NULL)
			return;

		const char* layout = (const char*)msg.param1;
		if (CUserInfo::Instance())
			CUserInfo::Instance()->Close();
		g_pFresh->OpenLayout(layout, this, true);
		break;
	}

	case M983(0x1):
	case M983(0x2):
	case M983(0xe2):
	case M983(0xe3):
		break;

	default:
		HandleMsgCommon(msg);
		break;
	}
}
