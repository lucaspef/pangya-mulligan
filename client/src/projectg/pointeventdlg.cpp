#include "minatl.h"
#include "pointeventdlg.h"
#include "frbutton.h"
#include "frarea.h"
#include "frviewer.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "shareddoc.h"
#include "packet.h"
#include "../shared/localize.h"


extern Fresh* g_pFresh;

unsigned short g_prevCommodity = 10;
FrPointEventDlg* g_pPointEventDlg = NULL;
CCommodityTable g_CommodityTable;
CCommodityStatusManager g_CommodityStatusManager;

CCommodityItem::CCommodityItem()
{
	m_typeID = 0;
	m_value = 0;
	m_itemType = (eItemType)0;
}

CCommodityItem::CCommodityItem(const CCommodityItem& rhs)
{
	m_typeID = rhs.m_typeID;
	m_value = rhs.m_value;
	m_itemType = rhs.m_itemType;
}

CCommodityItem::CCommodityItem(unsigned long typeID, unsigned long value,
	eItemType itemType)
{
	m_typeID = typeID;
	m_value = value;
	m_itemType = itemType;
}

void CCommodityItem::SetTypeID(unsigned long typeID)
{
	m_typeID = typeID;
}

void CCommodityItem::SetValue(unsigned long value)
{
	m_value = value;
}

void CCommodityItem::SetItemType(eItemType itemType)
{
	m_itemType = itemType;
}

unsigned long CCommodityItem::GetTypeID() const
{
	return m_typeID;
}

unsigned long CCommodityItem::GetValue() const
{
	return m_value;
}

CCommodityItem::eItemType CCommodityItem::GetItemType() const
{
	return m_itemType;
}

const CCommodityItem& CCommodityItem::operator=(const CCommodityItem& rhs)
{
	m_typeID = rhs.m_typeID;
	m_value = rhs.m_value;
	m_itemType = rhs.m_itemType;
	return *this;
}

CCommodity::CCommodity()
{
	m_itemCount = 0;
	m_cost = 0;
}

CCommodity::CCommodity(const CCommodity& rhs)
{
	m_itemCount = rhs.m_itemCount;
	for (int i = 0; i < m_itemCount; ++i)
		m_item[i] = rhs.GetCommodityItem(i);
	m_cost = rhs.m_cost;
}

CCommodity::CCommodity(const CCommodityItem& item, unsigned short cost)
{
	m_itemCount = 1;
	m_item[0] = item;
	m_cost = cost;
}

void CCommodity::SetCommodityCost(unsigned short cost)
{
	m_cost = cost;
}

unsigned short CCommodity::GetCommodityItemCount() const
{
	return m_itemCount;
}

unsigned short CCommodity::GetCommodityCost() const
{
	return m_cost;
}

bool CCommodity::InsertCommodityItem(const CCommodityItem& item)
{
	if (m_itemCount >= MAX_ITEM_COUNT)
		return false;

	m_item[m_itemCount] = item;
	++m_itemCount;
	return true;
}

const CCommodityItem& CCommodity::GetCommodityItem(unsigned short index) const
{
	return m_item[index];
}

const CCommodity& CCommodity::operator=(const CCommodity& rhs)
{
	m_itemCount = rhs.m_itemCount;
	for (int i = 0; i < m_itemCount; ++i)
		m_item[i] = rhs.GetCommodityItem(i);
	m_cost = rhs.m_cost;
	return *this;
}

CCommodityTable::CCommodityTable()
{
	InitializeTable();
}

bool CCommodityTable::InitializeTable()
{
	CCommodity commodity[MAX_COMMODITY_COUNT];

	commodity[0].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[0].SetCommodityCost(600);
	commodity[1].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[1].SetCommodityCost(600);
	commodity[2].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[2].SetCommodityCost(600);
	commodity[3].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[3].SetCommodityCost(600);
	commodity[4].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[4].SetCommodityCost(600);
	commodity[5].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[5].SetCommodityCost(600);
	commodity[6].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[6].SetCommodityCost(600);
	commodity[7].InsertCommodityItem(
		CCommodityItem(0, 0, (CCommodityItem::eItemType)4));
	commodity[7].SetCommodityCost(300);
	commodity[8].InsertCommodityItem(
		CCommodityItem(0x1a000010, 3000, (CCommodityItem::eItemType)1));
	commodity[8].SetCommodityCost(150);
	commodity[9].InsertCommodityItem(
		CCommodityItem(0x1a00003f, 1, (CCommodityItem::eItemType)5));
	commodity[9].SetCommodityCost(100);

	for (int i = 0; i < MAX_COMMODITY_COUNT; ++i)
		m_commodity[i] = commodity[i];
	return true;
}

const CCommodity& CCommodityTable::GetCommodity(unsigned short index) const
{
	if (index > MAX_COMMODITY_COUNT)
		index = 0;
	return m_commodity[index];
}

CCommodityStatusManager::CCommodityStatusManager()
{
	m_pStatus = NULL;
	InitializeStatus();
}

CCommodityStatusManager::~CCommodityStatusManager()
{
	if (m_pStatus)
	{
		delete[] m_pStatus;
		m_pStatus = NULL;
	}
}

void CCommodityStatusManager::InitializeStatus()
{
	if (!m_pStatus)
	{
		m_pStatus = new sCommodityStatus[MAX_COMMODITY_COUNT];
		for (int i = 0; i < MAX_COMMODITY_COUNT; ++i)
		{
			m_pStatus[i].myCount = 0;
			m_pStatus[i].totalCount = 0;
			m_pStatus[i].status = 0;
		}
	}

	m_updateType[8] = UPDATE_NONE;
	m_updateType[0] = UPDATE_MAX;
	m_updateType[1] = UPDATE_MAX;
	m_updateType[2] = UPDATE_MAX;
	m_updateType[3] = UPDATE_MAX;
	m_updateType[4] = UPDATE_MAX;
	m_updateType[5] = UPDATE_MAX;
	m_updateType[6] = UPDATE_MAX;
	m_updateType[7] = UPDATE_MAX;
	m_updateType[9] = UPDATE_MAX;
}

unsigned long CCommodityStatusManager::GetStatus(unsigned short index) const
{
	return m_pStatus[index].status;
}

unsigned long CCommodityStatusManager::GetMyCount(unsigned short index) const
{
	return m_pStatus[index].myCount;
}

unsigned long CCommodityStatusManager::GetTotalCount(unsigned short index) const
{
	return m_pStatus[index].totalCount;
}

CCommodityStatusManager::eUpdateType CCommodityStatusManager::GetUpdateType(
	unsigned short index) const
{
	return m_updateType[index];
}

void CCommodityStatusManager::SetStatus(unsigned short index,
	unsigned long status)
{
	m_pStatus[index].status = status;
}

void CCommodityStatusManager::SetMyCount(unsigned short index,
	unsigned long count)
{
	m_pStatus[index].myCount = count;
}

void CCommodityStatusManager::SetTotalCount(unsigned short index,
	unsigned long count)
{
	m_pStatus[index].totalCount = count;
}

void CCommodityStatusManager::SetData(unsigned short index,
	unsigned long status, unsigned long myCount, unsigned long totalCount)
{
	m_pStatus[index].status = status;
	m_pStatus[index].myCount = myCount;
	m_pStatus[index].totalCount = totalCount;
}

void CCommodityStatusManager::UpdateTotalCount(unsigned short index,
	unsigned long count)
{
	switch (m_updateType[index])
	{
	case UPDATE_MAX:
		if (m_pStatus[index].totalCount <= count)
			m_pStatus[index].totalCount = count;
		break;
	case UPDATE_MIN:
		if (m_pStatus[index].totalCount >= count ||
			m_pStatus[index].totalCount == 0)
			m_pStatus[index].totalCount = count;
		break;
	case UPDATE_REPLACE:
		m_pStatus[index].totalCount = count;
		break;
	}
}

void CCommodityStatusManager::UpdateStatus(unsigned short index,
	unsigned long status)
{
	m_pStatus[index].status = status;
}

void CCommodityStatusManager::UpdateMyCount(unsigned short index,
	unsigned long count)
{
	m_pStatus[index].myCount = count;
}

CCommodityButton::CCommodityButton()
{
	m_buttonType = BUTTON_NONE;
	m_pButton = NULL;
	m_pCover = NULL;
	m_coverPos = WPoint(0.0f, 0.0f);
}

CCommodityButton::~CCommodityButton()
{
}

CCommodityButton::eCommodityButtonType CCommodityButton::GetButtonType() const
{
	return m_buttonType;
}

bool CCommodityButton::IsVisible() const
{
	return m_pButton->IsVisible();
}

bool CCommodityButton::BindButton(FrButton* pButton)
{
	if (!pButton)
		return false;

	m_pButton = pButton;
	m_pButton->SetPushDelay(0.1f);
	return true;
}

void CCommodityButton::SetCover(const char* name)
{
	if (m_pButton)
	{
		m_pCover = NULL;
		m_pCover = g_pFresh->GetBitmap(name);
		m_coverPos.x = m_pButton->GetRect().x;
		m_coverPos.y = m_pButton->GetRect().y;
	}
}

void CCommodityButton::ChangeButtonType(eCommodityButtonType type)
{
	if (m_pButton)
	{
		switch (type)
		{
		case BUTTON_GIFT1:
			m_pButton->SetButtonImg("btn_pointevent_gift1_n", FrButton::NORMAL);
			m_pButton->SetButtonImg("btn_pointevent_gift1_o", FrButton::OVER);
			m_pButton->SetButtonImg("btn_pointevent_gift1_d",
				FrButton::PRESSED);
			m_buttonType = BUTTON_GIFT1;
			break;
		case BUTTON_GIFT2:
			m_pButton->SetButtonImg("btn_pointevent_gift2_n", FrButton::NORMAL);
			m_pButton->SetButtonImg("btn_pointevent_gift2_o", FrButton::OVER);
			m_pButton->SetButtonImg("btn_pointevent_gift2_d",
				FrButton::PRESSED);
			m_buttonType = BUTTON_GIFT2;
			break;
		case BUTTON_CONTRIBUTION:
			m_buttonType = BUTTON_CONTRIBUTION;
			break;
		}
	}
}

void CCommodityButton::ButtonSwitch(bool bEnable)
{
	if (m_pButton)
		m_pButton->SetVisible(bEnable);
}

void CCommodityButton::DrawCover()
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi && m_pButton->IsVisible() && m_pCover)
	{
		m_coverPos.x = m_pButton->GetRect().x;
		m_coverPos.y = m_pButton->GetRect().y;
		int space = gdi->SetSpace(0);
		gdi->DrawTexture(m_pCover,
			WRect(m_coverPos.x, m_coverPos.y, (float)m_pCover->Width(),
				(float)m_pCover->Height()),
			0xffffffff, 0);
		gdi->SetSpace(space);
	}
}

CCommoditySlot::CCommoditySlot()
{
	m_slotIndex = 0;
	m_slotStatus = SLOT_ENABLE;
	m_pSoldOutMark = NULL;
	m_pGiftMark = NULL;
	m_myCount = 0;
	m_totalCount = 0;
	m_myCountPos = WPoint(0.0f, 0.0f);
	m_totalCountPos = WPoint(0.0f, 0.0f);
	m_markPos = WPoint(0.0f, 0.0f);
	m_linkCount = 0;
	for (int i = 0; i < MAX_COMMODITY_SLOT_COUNT; ++i)
		m_pLink[i] = NULL;
}

CCommoditySlot::~CCommoditySlot()
{
}

void CCommoditySlot::InitSlot(sCommoditySlotData& data)
{
	m_slotIndex = data.slotIndex;
	m_slotStatus = SLOT_ENABLE;
	m_pSoldOutMark = g_pFresh->GetBitmap(data.soldOutMark);
	m_pGiftMark = g_pFresh->GetBitmap(data.giftMark);
	m_countFlag = data.countFlag;
	m_markFlag = data.markFlag;
	m_myCountPos.x = data.myCountPos.x;
	m_myCountPos.y = data.myCountPos.y;
	m_totalCountPos.x = data.totalCountPos.x;
	m_totalCountPos.y = data.totalCountPos.y;
	m_markPos.x = data.markPos.x;
	m_markPos.y = data.markPos.y;
	BindButton(data.pButton);
	SetCover(data.cover);
	ChangeButtonType(data.buttonType);
	m_linkCount = 0;
	for (int i = 0; i < MAX_COMMODITY_SLOT_COUNT; ++i)
		m_pLink[i] = NULL;
}

void CCommoditySlot::SetMyCount(unsigned long count)
{
	m_myCount = count;
}

void CCommoditySlot::SetTotalCount(unsigned long count)
{
	m_totalCount = count;
}

void CCommoditySlot::SetSlotStatus(eSlotStatus status)
{
	m_slotStatus = status;
	switch (status)
	{
	case SLOT_ENABLE:
		ButtonSwitch(true);
		break;
	case SLOT_RECEIVED:
		ButtonSwitch(false);
		return;
	case SLOT_SOLDOUT:
	case SLOT_DISABLE:
		ButtonSwitch(false);
		break;
	}
}

unsigned short CCommoditySlot::GetSlotIndex() const
{
	return m_slotIndex;
}

CCommoditySlot::eSlotStatus CCommoditySlot::GetSlotStatus() const
{
	return m_slotStatus;
}

WRect CCommoditySlot::GetButtonRect() const
{
	WRect rect = m_pButton->GetRect();
	return rect;
}

void CCommoditySlot::AddSlotLink(const CCommoditySlot* pSlot)
{
	if (pSlot && m_linkCount < MAX_COMMODITY_SLOT_COUNT)
	{
		m_pLink[m_linkCount] = pSlot;
		++m_linkCount;
	}
}

void CCommoditySlot::Process()
{
	for (int i = 0; i < m_linkCount; ++i)
	{
		if (m_pLink[i] && m_pLink[i]->m_slotStatus == SLOT_RECEIVED)
		{
			m_slotStatus = SLOT_DISABLE;
			ButtonSwitch(false);
		}
	}
}

void CCommoditySlot::DrawSlot(WPoint pos)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;

	int space = gdi->SetSpace(0);
	gdi->SetTextStyle(1);
	switch (m_countFlag)
	{
	case 1:
		gdi->Print(WPoint(pos.x + m_myCountPos.x, pos.y + m_myCountPos.y), 2,
			"%d", m_myCount);
		break;
	case 2:
		gdi->Print(WPoint(pos.x + m_totalCountPos.x, pos.y + m_totalCountPos.y),
			2, "%d", m_totalCount);
		break;
	case 3:
		gdi->Print(WPoint(pos.x + m_myCountPos.x, pos.y + m_myCountPos.y), 2,
			"%d", m_myCount);
		gdi->Print(WPoint(pos.x + m_totalCountPos.x, pos.y + m_totalCountPos.y),
			2, "%d", m_totalCount);
		break;
	}
	gdi->SetTextStyle(0);

	switch (m_slotStatus)
	{
	case SLOT_SOLDOUT:
		if (m_pSoldOutMark && (m_markFlag & 1))
			gdi->DrawTexture(m_pSoldOutMark,
				WRect(pos.x + m_markPos.x, pos.y + m_markPos.y,
					(float)m_pSoldOutMark->Width(),
					(float)m_pSoldOutMark->Height()),
				0xffffffff, 0);
		break;
	case SLOT_RECEIVED:
		if (m_pGiftMark && (m_markFlag & 2))
			gdi->DrawTexture(m_pGiftMark,
				WRect(pos.x + m_markPos.x, pos.y + m_markPos.y,
					(float)m_pGiftMark->Width(), (float)m_pGiftMark->Height()),
				0xffffffff, 0);
		break;
	}
	gdi->SetSpace(space);
}

IMPLEMENT_OBJECT(FrPointEventNoticeDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrPointEventNoticeDlg, FrForm)

ON_FRESH_VI("yes_button", FRCMD_INIT, FrPointEventNoticeDlg::OnInitBtnYes)
ON_FRESH_VV("yes_button", FRCMD_LBUTTONUP,
	FrPointEventNoticeDlg::OnLBDownBtnYes)
ON_FRESH_VI("no_button", FRCMD_INIT, FrPointEventNoticeDlg::OnInitBtnNo)
ON_FRESH_VV("no_button", FRCMD_LBUTTONUP, FrPointEventNoticeDlg::OnLBDownBtnNo)
ON_FRESH_VI("draw_area", FRCMD_INIT, FrPointEventNoticeDlg::OnInitDrawArea)
ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW,
	FrPointEventNoticeDlg::OnOwnerDrawDrawArea)

END_FRESH_MSGMAP()

FrPointEventNoticeDlg::FrPointEventNoticeDlg()
{
	m_pBtnYes = NULL;
	m_pBtnNo = NULL;
	m_pDrawArea = NULL;
	m_commodityIndex = 0;
	m_pItemImage = NULL;
	m_type = 0;
}

FrPointEventNoticeDlg::~FrPointEventNoticeDlg()
{
}

void FrPointEventNoticeDlg::OnInitBtnYes(int param)
{
	m_pBtnYes = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnYes->SetPushDelay(0.0f);
}

void FrPointEventNoticeDlg::OnLBDownBtnYes()
{
	Close(FrOK, true);
}

void FrPointEventNoticeDlg::OnInitBtnNo(int param)
{
	m_pBtnNo = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnNo->SetPushDelay(0.0f);
}

void FrPointEventNoticeDlg::OnLBDownBtnNo()
{
	Close(FrCANCEL, true);
}

void FrPointEventNoticeDlg::SetCommodity(int index, unsigned long type)
{
	m_commodityIndex = index;
	if (index + 1 < 10)
		m_pItemImage =
			g_pFresh->GetBitmap(MakeStr("pointevent_item_0%d", index + 1));
	else
		m_pItemImage =
			g_pFresh->GetBitmap(MakeStr("pointevent_item_%d", index + 1));
	m_type = type;
}

void FrPointEventNoticeDlg::OnInitDrawArea(int param)
{
	m_pDrawArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPointEventNoticeDlg::OnOwnerDrawDrawArea(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;

	int space = gdi->SetSpace(0);
	WRect rect = m_rect;
	if (m_pItemImage)
		gdi->DrawTexture(m_pItemImage,
			WRect(rect.x + 48.0f, rect.y + 41.0f, (float)m_pItemImage->Width(),
				(float)m_pItemImage->Height()),
			0xffffffff, 0);
	gdi->SetTextStyle(1);
	switch (m_type)
	{
	case 0:
		gdi->Print(WPoint(rect.x + 69.0f, rect.y + 153.0f), 0,
			K2L_Compatibility("\xc0\xbb \xc1\xf6\xb1\xde\xb9\xde\xc0\xb8\xbd\xc3\xb0\xda\xbd\xc0"
			"\xb4\xcf\xb1\xee?"));
		break;
	case 1:
		gdi->Print(WPoint(rect.x + 69.0f, rect.y + 153.0f), 0,
			K2L_Compatibility("\xc0\xbb \xc0\xc0\xb8\xf0\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf"
			"\xb1\xee?"));
		break;
	case 2:
		gdi->Print(WPoint(rect.x + 53.0f, rect.y + 153.0f), 0,
			K2L_Compatibility("\xc0\xbb \xc1\xf6\xb1\xde\xb9\xde\xb0\xed 100\xc6\xf7\xc0\xce\xc6"
			"\xae \xb1\xe2\xba\xce\xc7\xd5\xb4\xcf\xb4\xd9."));
		break;
	}
	gdi->SetTextStyle(0);
	gdi->SetSpace(space);
}

bool FrPointEventNoticeDlg::OnInit()
{
	return true;
}

IMPLEMENT_OBJECT(FrPointEventDescDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrPointEventDescDlg, FrForm)

ON_FRESH_VI("desc_view", FRCMD_INIT, FrPointEventDescDlg::OnInitDescViewer)

END_FRESH_MSGMAP()

FrPointEventDescDlg::FrPointEventDescDlg()
{
	m_pDescViewer = NULL;
	memset(m_szDescImage, 0, sizeof(m_szDescImage));
}

FrPointEventDescDlg::~FrPointEventDescDlg()
{
}

void FrPointEventDescDlg::SetDescImage(const char* filename)
{
	if (m_pDescViewer)
	{
		memset(m_szDescImage, 0, sizeof(m_szDescImage));
		strcpy(m_szDescImage, filename);
		m_pDescViewer->ShowScrollBar(true);
		m_pDescViewer->Open(m_szDescImage);
	}
}

void FrPointEventDescDlg::OnInitDescViewer(int param)
{
	m_pDescViewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

IMPLEMENT_OBJECT(FrPointEventDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrPointEventDlg, FrForm)

ON_FRESH_VI("desc_button", FRCMD_INIT, FrPointEventDlg::OnInitDescButton)
ON_FRESH_VV("desc_button", FRCMD_LBUTTONUP, FrPointEventDlg::OnLBDownDescButton)
ON_FRESH_VI("commodity_button01", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity01)
ON_FRESH_VV("commodity_button01", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity01)
ON_FRESH_VI("commodity_button02", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity02)
ON_FRESH_VV("commodity_button02", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity02)
ON_FRESH_VI("commodity_button03", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity03)
ON_FRESH_VV("commodity_button03", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity03)
ON_FRESH_VI("commodity_button04", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity04)
ON_FRESH_VV("commodity_button04", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity04)
ON_FRESH_VI("commodity_button05", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity05)
ON_FRESH_VV("commodity_button05", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity05)
ON_FRESH_VI("commodity_button06", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity06)
ON_FRESH_VV("commodity_button06", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity06)
ON_FRESH_VI("commodity_button07", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity07)
ON_FRESH_VV("commodity_button07", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity07)
ON_FRESH_VI("commodity_button08", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity08)
ON_FRESH_VV("commodity_button08", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity08)
ON_FRESH_VI("commodity_button09", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnCommodity09)
ON_FRESH_VV("commodity_button09", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnCommodity09)
ON_FRESH_VI("data_area", FRCMD_OWNERDRAW, FrPointEventDlg::OnOwnerDrawDataArea)
ON_FRESH_VI("contribution_area", FRCMD_OWNERDRAW,
	FrPointEventDlg::OnOwnerDrawContribution)
ON_FRESH_VI("contribution_desc", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnDescContribution)
ON_FRESH_VV("contribution_desc", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnDescContribution)
ON_FRESH_VI("contribution_button", FRCMD_INIT,
	FrPointEventDlg::OnInitBtnContribution)
ON_FRESH_VV("contribution_button", FRCMD_LBUTTONUP,
	FrPointEventDlg::OnLBDownBtnContribution)

END_FRESH_MSGMAP()

FrPointEventDlg::FrPointEventDlg()
{
	m_elapsed = 0.0f;
	m_pDescButton = NULL;
	m_pDescDlg = NULL;
	m_pNoticeDlg = NULL;
	m_myPoint = 0;
	for (int i = 0; i < MAX_COMMODITY_SLOT_COUNT; ++i)
		m_pBtnCommodity[i] = NULL;
	g_pPointEventDlg = this;
	m_contribution = 0.0f;
	m_pBtnDescContribution = NULL;
	m_pContributionGage = NULL;
	m_pContributionGageBg = NULL;
	memset(m_pContributionFrame, 0, sizeof(m_pContributionFrame));
	m_pIconSuccess = NULL;
}

FrPointEventDlg::~FrPointEventDlg()
{
	g_pPointEventDlg = NULL;
}

bool FrPointEventDlg::OnPointEventDescDlgResult(int result, FrForm* pForm)
{
	m_pDescDlg = NULL;
	return true;
}

void FrPointEventDlg::UnSelectCommodity()
{
	m_selectCommodity = 10;
}

void FrPointEventDlg::OnInitDescButton(int param)
{
	if (!m_pDescButton)
		m_pDescButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrPointEventDlg::OnLBDownDescButton()
{
	if (!m_pDescDlg)
		m_pDescDlg = CreateForm<FrPointEventDescDlg>(g_pFresh->GetManager(),
			this, "point_event_desc", NULL);
	if (m_pDescDlg)
	{
		m_pDescDlg->SetDescImage("point_event_explanation.jpg");
		m_pDescDlg->SetIconRect(m_pDescButton->GetRect());
		m_pDescDlg->Open(
			(FRESH_PFN_RESULT)&FrPointEventDlg::OnPointEventDescDlgResult, 17);
	}
}

void FrPointEventDlg::OnInitBtnCommodity01(int param)
{
	m_pBtnCommodity[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[0]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity02(int param)
{
	m_pBtnCommodity[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[1]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity03(int param)
{
	m_pBtnCommodity[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[2]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity04(int param)
{
	m_pBtnCommodity[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[3]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity05(int param)
{
	m_pBtnCommodity[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[4]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity06(int param)
{
	m_pBtnCommodity[5] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[5]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity07(int param)
{
	m_pBtnCommodity[6] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[6]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity08(int param)
{
	m_pBtnCommodity[7] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[7]->SetVisible(false);
}

void FrPointEventDlg::OnInitBtnCommodity09(int param)
{
	m_pBtnCommodity[8] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBtnCommodity[8]->SetVisible(false);
}

void FrPointEventDlg::OnLBDownBtnCommodity01()
{
	Confirm(0);
}

void FrPointEventDlg::OnLBDownBtnCommodity02()
{
	Confirm(1);
}

void FrPointEventDlg::OnLBDownBtnCommodity03()
{
	Confirm(2);
}

void FrPointEventDlg::OnLBDownBtnCommodity04()
{
	Confirm(3);
}

void FrPointEventDlg::OnLBDownBtnCommodity05()
{
	Confirm(4);
}

void FrPointEventDlg::OnLBDownBtnCommodity06()
{
	Confirm(5);
}

void FrPointEventDlg::OnLBDownBtnCommodity07()
{
	Confirm(6);
}

void FrPointEventDlg::OnLBDownBtnCommodity08()
{
	Confirm(7);
}

void FrPointEventDlg::OnLBDownBtnCommodity09()
{
	Confirm(8);
}

void FrPointEventDlg::OnOwnerDrawDataArea(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		WPoint origin = WPoint(m_rect.x, m_rect.y);
		int space = gdi->SetSpace(0);
		gdi->SetTextStyle(1);
		gdi->Print(WPoint(origin.x + 507.0f, origin.y + 73.0f), 2, "%d",
			GetMyPoint());
		gdi->SetTextStyle(0);
		gdi->SetSpace(space);
		for (int i = 0; i < MAX_COMMODITY_SLOT_COUNT; ++i)
			m_commoditySlot[i].DrawSlot(origin);
	}
}

bool FrPointEventDlg::OnPointEventNoticeDlgResult(int result, FrForm* pForm)
{
	m_pNoticeDlg = NULL;
	if (result == FrOK)
	{
		WSendPacket packet((enumClientPacket)0x8d);
		packet.Encode4(m_selectCommodity);
		packet.Send(TO_GAME);
		return true;
	}
	m_selectCommodity = 10;
	return true;
}

bool FrPointEventDlg::OnInit()
{
	SendUpdateDataPacket();

	CCommoditySlot::sCommoditySlotData data[MAX_COMMODITY_SLOT_COUNT];

	data[0].slotIndex = 0;
	data[0].countFlag = 3;
	data[0].markFlag = 0;
	data[0].soldOutMark = "pointevent_soldout_mark";
	data[0].giftMark = "pointevent_gift_mark";
	data[0].myCountPos.x = 166.0f;
	data[0].myCountPos.y = 285.0f;
	data[0].totalCountPos.x = 166.0f;
	data[0].totalCountPos.y = 297.0f;
	data[0].markPos.x = 130.0f;
	data[0].markPos.y = 252.0f;
	data[0].pButton = m_pBtnCommodity[0];
	data[0].cover = "btn_pointevent_white";
	data[0].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[1].slotIndex = 1;
	data[1].countFlag = 3;
	data[1].markFlag = 0;
	data[1].soldOutMark = "pointevent_soldout_mark";
	data[1].giftMark = "pointevent_gift_mark";
	data[1].myCountPos.x = 330.0f;
	data[1].myCountPos.y = 285.0f;
	data[1].totalCountPos.x = 330.0f;
	data[1].totalCountPos.y = 297.0f;
	data[1].markPos.x = 295.0f;
	data[1].markPos.y = 252.0f;
	data[1].pButton = m_pBtnCommodity[1];
	data[1].cover = "btn_pointevent_white";
	data[1].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[2].slotIndex = 2;
	data[2].countFlag = 3;
	data[2].markFlag = 0;
	data[2].soldOutMark = "pointevent_soldout_mark";
	data[2].giftMark = "pointevent_gift_mark";
	data[2].myCountPos.x = 494.0f;
	data[2].myCountPos.y = 285.0f;
	data[2].totalCountPos.x = 494.0f;
	data[2].totalCountPos.y = 297.0f;
	data[2].markPos.x = 459.0f;
	data[2].markPos.y = 252.0f;
	data[2].pButton = m_pBtnCommodity[2];
	data[2].cover = "btn_pointevent_white";
	data[2].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[3].slotIndex = 3;
	data[3].countFlag = 3;
	data[3].markFlag = 0;
	data[3].soldOutMark = "pointevent_soldout_mark";
	data[3].giftMark = "pointevent_gift_mark";
	data[3].myCountPos.x = 166.0f;
	data[3].myCountPos.y = 391.0f;
	data[3].totalCountPos.x = 166.0f;
	data[3].totalCountPos.y = 403.0f;
	data[3].markPos.x = 130.0f;
	data[3].markPos.y = 358.0f;
	data[3].pButton = m_pBtnCommodity[3];
	data[3].cover = "btn_pointevent_white";
	data[3].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[4].slotIndex = 4;
	data[4].countFlag = 3;
	data[4].markFlag = 0;
	data[4].soldOutMark = "pointevent_soldout_mark";
	data[4].giftMark = "pointevent_gift_mark";
	data[4].myCountPos.x = 330.0f;
	data[4].myCountPos.y = 391.0f;
	data[4].totalCountPos.x = 330.0f;
	data[4].totalCountPos.y = 403.0f;
	data[4].markPos.x = 295.0f;
	data[4].markPos.y = 358.0f;
	data[4].pButton = m_pBtnCommodity[4];
	data[4].cover = "btn_pointevent_white";
	data[4].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[5].slotIndex = 5;
	data[5].countFlag = 3;
	data[5].markFlag = 0;
	data[5].soldOutMark = "pointevent_soldout_mark";
	data[5].giftMark = "pointevent_gift_mark";
	data[5].myCountPos.x = 494.0f;
	data[5].myCountPos.y = 391.0f;
	data[5].totalCountPos.x = 494.0f;
	data[5].totalCountPos.y = 403.0f;
	data[5].markPos.x = 459.0f;
	data[5].markPos.y = 358.0f;
	data[5].pButton = m_pBtnCommodity[5];
	data[5].cover = "btn_pointevent_white";
	data[5].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[6].slotIndex = 6;
	data[6].countFlag = 3;
	data[6].markFlag = 0;
	data[6].soldOutMark = "pointevent_soldout_mark";
	data[6].giftMark = "pointevent_gift_mark";
	data[6].myCountPos.x = 166.0f;
	data[6].myCountPos.y = 496.0f;
	data[6].totalCountPos.x = 166.0f;
	data[6].totalCountPos.y = 508.0f;
	data[6].markPos.x = 130.0f;
	data[6].markPos.y = 462.0f;
	data[6].pButton = m_pBtnCommodity[6];
	data[6].cover = "btn_pointevent_white";
	data[6].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[7].slotIndex = 7;
	data[7].countFlag = 3;
	data[7].markFlag = 0;
	data[7].soldOutMark = "pointevent_soldout_mark";
	data[7].giftMark = "pointevent_gift_mark";
	data[7].myCountPos.x = 330.0f;
	data[7].myCountPos.y = 496.0f;
	data[7].totalCountPos.x = 330.0f;
	data[7].totalCountPos.y = 508.0f;
	data[7].markPos.x = 295.0f;
	data[7].markPos.y = 462.0f;
	data[7].pButton = m_pBtnCommodity[7];
	data[7].cover = "btn_pointevent_white";
	data[7].buttonType = CCommodityButton::BUTTON_GIFT2;

	data[8].slotIndex = 8;
	data[8].countFlag = 0;
	data[8].markFlag = 0;
	data[8].soldOutMark = "pointevent_soldout_mark";
	data[8].giftMark = "pointevent_gift_mark";
	data[8].myCountPos.x = 494.0f;
	data[8].myCountPos.y = 496.0f;
	data[8].totalCountPos.x = 494.0f;
	data[8].totalCountPos.y = 508.0f;
	data[8].markPos.x = 459.0f;
	data[8].markPos.y = 462.0f;
	data[8].pButton = m_pBtnCommodity[8];
	data[8].cover = "btn_pointevent_white";
	data[8].buttonType = CCommodityButton::BUTTON_GIFT1;

	data[9].slotIndex = 9;
	data[9].countFlag = 0;
	data[9].markFlag = 0;
	data[9].soldOutMark = "pointevent_soldout_mark";
	data[9].giftMark = "pointevent_gift_mark";
	data[9].myCountPos.x = 494.0f;
	data[9].myCountPos.y = 496.0f;
	data[9].totalCountPos.x = 494.0f;
	data[9].totalCountPos.y = 508.0f;
	data[9].markPos.x = 459.0f;
	data[9].markPos.y = 462.0f;
	data[9].pButton = m_pBtnCommodity[9];
	data[9].cover = "contribution_btn_white";
	data[9].buttonType = CCommodityButton::BUTTON_CONTRIBUTION;

	for (int i = 0; i < MAX_COMMODITY_SLOT_COUNT; ++i)
		m_commoditySlot[i].InitSlot(data[i]);

	m_selectCommodity = 10;

	if (IsLocalContent((localContentType_t)0x17 /*983: era 0x16; interpolado*/))
	{
		SetRect(WRect(m_rect.x - 50.0f, m_rect.y, m_rect.w, m_rect.h));
		m_pContributionGage = g_pFresh->GetBitmap("contribution_gage");
		m_pContributionGageBg =
			g_pFresh->GetBitmap("contribution_gage_background");
		m_pContributionFrame[0] = g_pFresh->GetBitmap("contribution_frame_01");
		m_pContributionFrame[1] = g_pFresh->GetBitmap("contribution_frame_02");
		m_pIconSuccess = g_pFresh->GetBitmap("icon_success");
	}
	return true;
}

void FrPointEventDlg::OnProc(const float delta)
{
	m_elapsed += delta;
	if (m_elapsed > 10.0f)
	{
		SendUpdateDataPacket();
		m_elapsed = 0.0f;
	}

	m_myPoint = Doc()->m_myInfo.info.dwPointPointEvent;

	int count = 10;
	if (!IsLocalContent((localContentType_t)0x17 /*983: era 0x16; interpolado*/))
		count = 9;

	for (int i = 0; i < count; ++i)
	{
		CCommodity commodity = g_CommodityTable.GetCommodity(i);
		m_commoditySlot[i].SetMyCount(g_CommodityStatusManager.GetMyCount(i));
		m_commoditySlot[i].SetTotalCount(
			g_CommodityStatusManager.GetTotalCount(i));

		bool bShort = false;
		if (m_myPoint >= commodity.GetCommodityCost())
		{
			m_commoditySlot[i].SetSlotStatus(CCommoditySlot::SLOT_ENABLE);
		}
		else
		{
			m_commoditySlot[i].SetSlotStatus(CCommoditySlot::SLOT_DISABLE);
			bShort = true;
		}

		unsigned long status = g_CommodityStatusManager.GetStatus(i);
		if (status == 0)
		{
			if (!bShort)
				m_commoditySlot[i].SetSlotStatus(CCommoditySlot::SLOT_ENABLE);
		}
		else if (status == 1)
		{
			m_commoditySlot[i].SetSlotStatus(CCommoditySlot::SLOT_RECEIVED);
		}
		else if (status == 2)
		{
			m_commoditySlot[i].SetSlotStatus(CCommoditySlot::SLOT_SOLDOUT);
		}
		m_commoditySlot[i].Process();
	}
}

void FrPointEventDlg::Confirm(unsigned long index)
{
	unsigned short prev = m_selectCommodity;
	g_prevCommodity = prev;
	if (prev != 10)
	{
		FrForm* notify =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		notify->SetTimeLimit(3.0f);
		notify->SetMessage(
			K2L_Compatibility(" \xc0\xe1\xbd\xc3\xc8\xc4\xbf\xa1 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5 "
			"\xc7\xd8\xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0. "),
			false);
		notify->Open(NULL, 0);
		return;
	}

	m_selectCommodity = (unsigned short)index;
	if (!m_pNoticeDlg)
		m_pNoticeDlg = CreateForm<FrPointEventNoticeDlg>(g_pFresh->GetManager(),
			this, "point_event_notice", NULL);
	if (m_pNoticeDlg)
	{
		unsigned long type = 0;
		switch (m_commoditySlot[index].GetButtonType())
		{
		case CCommodityButton::BUTTON_GIFT2:
			type = 1;
			break;
		case CCommodityButton::BUTTON_CONTRIBUTION:
			type = 2;
			break;
		case CCommodityButton::BUTTON_GIFT1:
		case CCommodityButton::BUTTON_NONE:
			type = 0;
			break;
		}
		m_pNoticeDlg->SetCommodity(m_selectCommodity, type);
		m_pNoticeDlg->SetIconRect(
			m_commoditySlot[m_selectCommodity].GetButtonRect());
		m_pNoticeDlg->Open(
			(FRESH_PFN_RESULT)&FrPointEventDlg::OnPointEventNoticeDlgResult,
			17);
	}
}

__forceinline unsigned long RbPointEventUID()
{
	return Doc()->m_myInfo.info.dwUID;
}
void FrPointEventDlg::ResultMessage(unsigned long result)
{
	FrForm* notify =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
	WRect origin = notify->GetRect();
	notify->SetRect(WRect(origin.x, origin.y, origin.w, origin.h));
	notify->SetTimeLimit(12.0f);
	if (m_selectCommodity >= 10)
		return;

	switch (result)
	{
	case 0:
		switch (m_commoditySlot[m_selectCommodity].GetButtonType())
		{
		case CCommodityButton::BUTTON_GIFT1:
		{
			notify->SetMessage(
				K2L_Compatibility("\xbc\xba\xb0\xf8\xc0\xfb\xc0\xb8\xb7\xce \xbb\xf3\xc7\xb0\xc0"
				"\xbb \xc1\xf6\xb1\xde \xb9\xde\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			WSendPacket packet(33);
			const char* nick = Doc()->m_myInfo.info.sNick;
			packet.EncodeStr(std::string(nick));
			unsigned long uid = Doc()->m_myInfo.info.dwUID;
			packet.Encode4(uid);
			packet.Send(TO_MSN);
		}
		break;
		case CCommodityButton::BUTTON_GIFT2:
			notify->SetMessage(
				K2L_Compatibility("\xbc\xba\xb0\xf8\xc0\xfb\xc0\xb8\xb7\xce \xbb\xf3\xc7\xb0\xc0"
				"\xbb \xc0\xc0\xb8\xf0 \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			break;
		case CCommodityButton::BUTTON_CONTRIBUTION:
		{
			notify->SetMessage(
				K2L_Compatibility("\xbc\xba\xb0\xf8\xc0\xfb\xc0\xb8\xb7\xce \xb1\xe2\xba\xce "
				"\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			WSendPacket packet(33);
			packet.EncodeStr(std::string(Doc()->m_myInfo.info.sNick));
			packet.Encode4(RbPointEventUID());
			packet.Send(TO_MSN);
		}
		break;
		}
		LocalDataUpdate(m_selectCommodity);
		break;
	case 1:
	case 4:
		switch (m_commoditySlot[m_selectCommodity].GetButtonType())
		{
		case CCommodityButton::BUTTON_CONTRIBUTION:
			notify->SetMessage(
				K2L_Compatibility("\xc6\xf7\xc0\xce\xc6\xae\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4"
				"\xcf\xb4\xd9."),
				false);
			break;
		case CCommodityButton::BUTTON_GIFT2:
			notify->SetMessage(
				K2L_Compatibility("\xc6\xf7\xc0\xce\xc6\xae\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4"
				"\xcf\xb4\xd9."),
				false);
			break;
		case CCommodityButton::BUTTON_GIFT1:
			notify->SetMessage(
				K2L_Compatibility("\xc6\xf7\xc0\xce\xc6\xae\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4"
				"\xcf\xb4\xd9."),
				false);
			break;
		}
		break;
	case 2:
	case 3:
		switch (m_commoditySlot[m_selectCommodity].GetButtonType())
		{
		case CCommodityButton::BUTTON_CONTRIBUTION:
			notify->SetMessage(
				K2L_Compatibility("\xb1\xe2\xba\xce \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf"
				"\xb4\xd9. \xc0\xe1\xbd\xc3 \xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3"
				"\xb5\xb5 \xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"),
				false);
			break;
		case CCommodityButton::BUTTON_GIFT2:
			notify->SetMessage(
				K2L_Compatibility("\xc0\xcc\xb9\xcc \xc0\xc0\xb8\xf0\xc7\xd1 \xbb\xf3\xc7\xb0\xc0"
				"\xd4\xb4\xcf\xb4\xd9."),
				false);
			break;
		case CCommodityButton::BUTTON_GIFT1:
			notify->SetMessage(
				K2L_Compatibility("\xc0\xcc\xb9\xcc \xc1\xf6\xb1\xde\xb9\xde\xc0\xba \xbb\xf3\xc7"
				"\xb0\xc0\xd4\xb4\xcf\xb4\xd9."),
				false);
			break;
		}
		break;
	}
	notify->Open(NULL, 0);
	m_selectCommodity = 10;
}

void FrPointEventDlg::SendUpdateDataPacket()
{
	WSendPacket packet((enumClientPacket)0x8e);
	packet.Send(TO_GAME);
}

void FrPointEventDlg::LocalDataUpdate(unsigned short index)
{
	unsigned long myCount = g_CommodityStatusManager.GetMyCount(index);
	unsigned long totalCount = g_CommodityStatusManager.GetTotalCount(index);
	switch (m_commoditySlot[index].GetButtonType())
	{
	case CCommodityButton::BUTTON_GIFT1:
		++myCount;
		if (totalCount > 0)
			--totalCount;
		g_CommodityStatusManager.SetMyCount(index, myCount);
		g_CommodityStatusManager.SetTotalCount(index, totalCount);
		break;
	case CCommodityButton::BUTTON_GIFT2:
		g_CommodityStatusManager.SetMyCount(index, myCount + 1);
		g_CommodityStatusManager.SetTotalCount(index, totalCount + 1);
		break;
	case CCommodityButton::BUTTON_CONTRIBUTION:
		totalCount += 100;
		g_CommodityStatusManager.SetMyCount(index, myCount + 1);
		g_CommodityStatusManager.SetTotalCount(index, totalCount);
		break;
	}

	CCommodity commodity = g_CommodityTable.GetCommodity(index);
	m_myPoint -= commodity.GetCommodityCost();
}

void FrPointEventDlg::OnOwnerDrawContribution(int param)
{
	if (!IsLocalContent((localContentType_t)0x17 /*983: era 0x16; interpolado*/))
		return;

	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;

	int space = gdi->SetSpace(0);
	WPoint pos(m_rect.x, m_rect.y);
	WPoint unused;
	int mark = 0;
	float height;

	m_contribution = (float)g_CommodityStatusManager.GetTotalCount(9);
	unsigned int value = (unsigned int)(m_contribution / 100.0); // 983 00b0d75f: fdiv, 0.01f truncava 1 abaixo
	if (value < m_iPangDouble1)
	{
		height = (float)value * 0.00121666666f;
	}
	else if (value < m_iExpDouble)
	{
		value -= m_iPangDouble1;
		height = (float)value * 0.000521428592f + 36.5f;
		if (value < 100)
			mark = 2;
	}
	else if (value < m_iPangDouble2)
	{
		value -= m_iExpDouble;
		height = (float)value * 0.000304166664f + 73.0f;
		if (value < 100)
			mark = 3;
	}
	else if (value < m_iMaxContribution)
	{
		value -= m_iPangDouble2;
		height = (float)value * 36.5 / 80000.0 + 109.5; // 983 00b0d837
		if (value < 100)
			mark = 4;
	}
	else
	{
		height = 146.0f;
		mark = 5;
	}

	float x = pos.x + 604.0f;
	gdi->DrawTexture(m_pContributionGageBg,
		WRect(x, (pos.y + 372.0f) - 146.0f, (float)m_pContributionGage->Width(),
			146.0f),
		0xffffffff, 0);
	gdi->DrawTexture(m_pContributionGage,
		WRect(x, (pos.y + 373.0f) - height, (float)m_pContributionGage->Width(),
			height + 1.0f),
		0xffffffff, 0);
	gdi->DrawTexture(m_pContributionFrame[0],
		WRect(pos.x + 522.0f, pos.y + 130.0f,
			(float)m_pContributionFrame[0]->Width(),
			(float)m_pContributionFrame[0]->Height()),
		0xffffffff, 0);
	gdi->DrawTexture(m_pContributionFrame[1],
		WRect(pos.x + 522.0f, pos.y + 386.0f,
			(float)m_pContributionFrame[1]->Width(),
			(float)m_pContributionFrame[1]->Height()),
		0xffffffff, 0);
	if (mark)
	{
		gdi->DrawTexture(m_pIconSuccess,
			WRect(pos.x + 617.0f, (pos.y + 366.0f) - (float)mark * 36.5f,
				(float)m_pIconSuccess->Width(),
				(float)m_pIconSuccess->Height()),
			0xffffffff, 0);
	}
	gdi->SetSpace(space);
}

void FrPointEventDlg::OnInitBtnDescContribution(int param)
{
	m_pBtnDescContribution = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsLocalContent((localContentType_t)0x17 /*983: era 0x16; interpolado*/))
	{
		m_pBtnDescContribution->SetVisible(false);
		m_pBtnDescContribution->Enable(false);
	}
	else
	{
		m_pBtnDescContribution->SetPushDelay(0.0f);
	}
}

void FrPointEventDlg::OnLBDownBtnDescContribution()
{
	if (IsLocalContent((localContentType_t)0x17 /*983: era 0x16; interpolado*/))
	{
		if (!m_pDescDlg)
			m_pDescDlg = CreateForm<FrPointEventDescDlg>(g_pFresh->GetManager(),
				this, "point_event_desc", NULL);
		if (m_pDescDlg)
		{
			m_pDescDlg->SetDescImage("point_event_explanation_02.jpg");
			m_pDescDlg->SetIconRect(m_pBtnDescContribution->GetRect());
			m_pDescDlg->Open(
				(FRESH_PFN_RESULT)&FrPointEventDlg::OnPointEventDescDlgResult,
				17);
		}
	}
}

void FrPointEventDlg::OnInitBtnContribution(int param)
{
	m_pBtnCommodity[9] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsLocalContent((localContentType_t)0x17 /*983: era 0x16; interpolado*/))
	{
		m_pBtnCommodity[9]->SetVisible(false);
	}
	else
	{
		m_pBtnCommodity[0]->SetVisible(false);
		m_pBtnCommodity[9]->SetPushDelay(0.0f);
	}
}

void FrPointEventDlg::OnLBDownBtnContribution()
{
	if (IsLocalContent((localContentType_t)0x17 /*983: era 0x16; interpolado*/))
		Confirm(9);
}
