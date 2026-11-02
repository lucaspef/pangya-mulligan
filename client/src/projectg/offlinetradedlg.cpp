#include "minatl.h"
#include "offlinetradedlg.h"
#include "frcombobox.h"
#include "fredit.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "frarea.h"
#include "frviewer.h"
#include "frgraphicinterface.h"
#include "standinginline.h"
#include "binstr.h"
#include "tooltip.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "../shared/localize.h"
#include "projectg.h"



extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrOfflineTradeFinishEditDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrOfflineTradeFinishEditDlg, FrForm)

ON_FRESH_VI("tradefinisheditdlg", FRCMD_INIT,
	FrOfflineTradeFinishEditDlg::OnTradeFinishEditDlgInit)
ON_FRESH_VI("trade_item_list", FRCMD_INIT,
	FrOfflineTradeFinishEditDlg::OnTradeItemListInit)
ON_FRESH_VI("trade_item_list", FRCMD_OWNERDRAW,
	FrOfflineTradeFinishEditDlg::OnTradeItemListOwnerDraw)
ON_FRESH_VV("trade_item_list", FRCMD_LBUTTONUP,
	FrOfflineTradeFinishEditDlg::OnTradeItemListLBtnUp)
ON_FRESH_VI("userid", FRCMD_INIT, FrOfflineTradeFinishEditDlg::OnUserIdInit)
ON_FRESH_VI("trade_title", FRCMD_INIT, FrOfflineTradeFinishEditDlg::OnTitleInit)
ON_FRESH_VI("ok", FRCMD_INIT, FrOfflineTradeFinishEditDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrOfflineTradeFinishEditDlg::OnOkBnUp)
ON_FRESH_VI("tooltip", FRCMD_INIT, FrOfflineTradeFinishEditDlg::OnToolTipInit)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW,
	FrOfflineTradeFinishEditDlg::OnToolTipOnwerDraw)

END_FRESH_MSGMAP()

FrOfflineTradeFinishEditDlg::FrOfflineTradeFinishEditDlg()
{
	m_pSelectedSlot = NULL;
	m_extendBtnN.pBitmap = g_pFresh->GetBitmap("rmr_extend_btn_n");
	m_extendBtnN.rect = WRect(96.0f, 66.0f, m_extendBtnN.pBitmap->Width(),
		m_extendBtnN.pBitmap->Height());
	m_extendBtnN.color = 0xffffffff;
	m_extendBtnO.pBitmap = g_pFresh->GetBitmap("rmr_extend_btn_o");
	m_extendBtnO.rect = WRect(96.0f, 66.0f, m_extendBtnO.pBitmap->Width(),
		m_extendBtnO.pBitmap->Height());
	m_extendBtnO.color = 0xffffffff;
	m_returnN.pBitmap = g_pFresh->GetBitmap("rmr_return_n");
	m_returnN.rect = WRect(144.0f, 66.0f, m_returnN.pBitmap->Width(),
		m_returnN.pBitmap->Height());
	m_returnN.color = 0xffffffff;
	m_returnO.pBitmap = g_pFresh->GetBitmap("rmr_return_o");
	m_returnO.rect = WRect(144.0f, 66.0f, m_returnO.pBitmap->Width(),
		m_returnO.pBitmap->Height());
	m_returnO.color = 0xffffffff;
	m_deleteBtnN.pBitmap = g_pFresh->GetBitmap("delete_btn_n");
	m_deleteBtnN.rect = WRect(144.0f, 66.0f, m_deleteBtnN.pBitmap->Width(),
		m_deleteBtnN.pBitmap->Height());
	m_deleteBtnN.color = 0xffffffff;
	m_deleteBtnO.pBitmap = g_pFresh->GetBitmap("delete_btn_o");
	m_deleteBtnO.rect = WRect(144.0f, 66.0f, m_deleteBtnO.pBitmap->Width(),
		m_deleteBtnO.pBitmap->Height());
	m_deleteBtnO.color = 0xffffffff;
	m_buyBtnN.pBitmap = g_pFresh->GetBitmap("buy_btn_n");
	m_buyBtnN.rect = WRect(144.0f, 66.0f, m_buyBtnN.pBitmap->Width(),
		m_buyBtnN.pBitmap->Height());
	m_buyBtnN.color = 0xffffffff;
	m_buyBtnO.pBitmap = g_pFresh->GetBitmap("buy_btn_o");
	m_buyBtnO.rect = WRect(144.0f, 66.0f, m_buyBtnO.pBitmap->Width(),
		m_buyBtnO.pBitmap->Height());
	m_buyBtnO.color = 0xffffffff;
	m_delete.pBitmap = g_pFresh->GetBitmap("delete");
	m_delete.rect = WRect(144.0f, 35.0f, m_delete.pBitmap->Width(),
		m_delete.pBitmap->Height());
	m_tradeState1.pBitmap = g_pFresh->GetBitmap("rmr_trade_state1");
	m_tradeState1.rect = WRect(70.0f, 35.0f, m_tradeState1.pBitmap->Width(),
		m_tradeState1.pBitmap->Height());
	m_tradeState2.pBitmap = g_pFresh->GetBitmap("rmr_trade_state2");
	m_tradeState2.rect = WRect(70.0f, 35.0f, m_tradeState2.pBitmap->Width(),
		m_tradeState2.pBitmap->Height());
	m_tradeState3.pBitmap = g_pFresh->GetBitmap("rmr_trade_state3");
	m_tradeState3.rect = WRect(70.0f, 35.0f, m_tradeState3.pBitmap->Width(),
		m_tradeState3.pBitmap->Height());
	m_itemUploadN.pBitmap = g_pFresh->GetBitmap("rmr_item_upload_n");
	m_itemUploadN.rect = WRect(144.0f, 66.0f, m_itemUploadN.pBitmap->Width(),
		m_itemUploadN.pBitmap->Height());
	m_itemUploadN.color = 0xffffffff;
	m_itemUploadO.pBitmap = g_pFresh->GetBitmap("rmr_item_upload_o");
	m_itemUploadO.rect = WRect(144.0f, 66.0f, m_itemUploadO.pBitmap->Width(),
		m_itemUploadO.pBitmap->Height());
	m_itemUploadO.color = 0xffffffff;
}

FrOfflineTradeFinishEditDlg::~FrOfflineTradeFinishEditDlg()
{
}

void FrOfflineTradeFinishEditDlg::OnTradeItemListOwnerDraw(int param)
{
	FrTradeFinishEditDlg::OnTradeItemListOwnerDraw(param);
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	FrListItem* item = (FrListItem*)param;
	if (gdi && item && item->pData)
	{
		const Bitmap* slotBmp = g_pFresh->GetBitmap("rmr_item_slot");
		sStallSlot* slot = (sStallSlot*)item->pData;
		gdi->SetTextStyle(0);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		if (slot->IsValid == 0)
		{
			if (slotBmp)
			{
				gdi->DrawTexture(slotBmp,
					WRect(item->pos.x, item->pos.y, slotBmp->Width(),
						slotBmp->Height()),
					0xffffffff, 0);
			}
			gdi->Print(WPoint(item->pos.x + 182.0f, item->pos.y + 7.0f), 2,
				"\xbb\xf3\xc1\xa1 \xbe\xc6\xc0\xcc\xc5\xdb \xbd\xbd\xb7\xd4 \xb1\xb8\xb8\xc5");
			if (item->underCursor &&
				(m_buyBtnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_buyBtnO.pBitmap,
					m_buyBtnO.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
					0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x1b);
			}
			else
			{
				WRect rect = m_buyBtnN.rect + item->pos;
				gdi->DrawTexture(m_buyBtnN.pBitmap, rect, 0xffffffff, 0);
			}
			return;
		}
		__int64 remain = 0;
		if (!slot->IsPermanence)
		{
			FILETIME slotTime;
			SystemTimeToFileTime(&slot->time, &slotTime);
			FILETIME serverTime;
			SystemTimeToFileTime(&Doc()->GetServerTime(), &serverTime);
			__int64 diff = ((unsigned __int64)slotTime.dwHighDateTime -
							   (unsigned __int64)serverTime.dwHighDateTime) *
					0x100000000 -
				serverTime.dwLowDateTime + slotTime.dwLowDateTime;
			remain = diff / 60 / 60 / 10000000;
			gdi->DrawTexture(m_tradeState3.pBitmap,
				m_tradeState3.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
				0);
			if (remain >= 0 && diff >= 0)
			{
				gdi->SetTextColor(0xffffffff, 0xffffffff);
				if (remain / 24 >= 1)
				{
					WPoint pos(m_tradeState3.rect.x + item->pos.x,
						m_tradeState3.rect.y + item->pos.y);
					gdi->Print(WPoint(pos.x + 55.0f, pos.y + 6.0f), 2,
						"%d\xc0\xcf", remain / 24 + 1);
				}
				else
				{
					WPoint pos(m_tradeState3.rect.x + item->pos.x,
						m_tradeState3.rect.y + item->pos.y);
					gdi->Print(WPoint(pos.x + 55.0f, pos.y + 6.0f), 2,
						"%d\xbd\xc3", remain);
				}
			}
			else
			{
				gdi->SetTextColor(0xffffffff, 0xffffffff);
				WPoint pos(m_tradeState3.rect.x + item->pos.x,
					m_tradeState3.rect.y + item->pos.y);
				gdi->Print(WPoint(pos.x + 55.0f, pos.y + 6.0f), 2,
					"\xb1\xe2\xb0\xa3\xb8\xb8\xb7\xe1");
			}
			gdi->SetTextColor(0xff000000, 0xffffffff);
			if (item->underCursor &&
				(m_extendBtnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_extendBtnO.pBitmap,
					m_extendBtnO.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x53);
			}
			else
			{
				gdi->DrawTexture(m_extendBtnN.pBitmap,
					m_extendBtnN.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
			}
		}
		if (slot->itemData.dwTid == 0)
		{
			if (slotBmp)
			{
				gdi->DrawTexture(slotBmp,
					WRect(item->pos.x, item->pos.y, slotBmp->Width(),
						slotBmp->Height()),
					0xffffffff, 0);
			}
			gdi->Print(WPoint(item->pos.x + 182.0f, item->pos.y + 7.0f), 2,
				"\xbb\xf3\xc1\xa1 \xbe\xc6\xc0\xcc\xc5\xdb \xbd\xbd\xb7\xd4");
			if (item->underCursor &&
				(m_itemUploadN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_itemUploadO.pBitmap,
					m_itemUploadO.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x54);
			}
			else
			{
				gdi->DrawTexture(m_itemUploadN.pBitmap,
					m_itemUploadN.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
			}
			return;
		}
		if (!slot->IsPermanence && remain < 0)
		{
			if (item->underCursor &&
				(m_returnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_returnO.pBitmap,
					m_returnO.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
					0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x55);
			}
			else
			{
				gdi->DrawTexture(m_returnN.pBitmap,
					m_returnN.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
					0);
			}
		}
		else
		{
			if (item->underCursor &&
				(m_deleteBtnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_deleteBtnO.pBitmap,
					m_deleteBtnO.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x52);
			}
			else
			{
				gdi->DrawTexture(m_deleteBtnN.pBitmap,
					m_deleteBtnN.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
			}
		}
	}
}

void FrOfflineTradeFinishEditDlg::OnTradeItemListLBtnUp()
{
	FrListItem* item = m_pTradeItemList->GetItemUnderCursor();
	if (item == NULL)
	{
		return;
	}
	FrWndManager* manager = g_pFresh->GetManager();
	sStallSlot* slot = (sStallSlot*)item->pData;
	unsigned char valid = slot->IsValid;
	const WPoint& mouse = manager->GetMousePos();
	bool buy = false;
	if (valid == 0)
	{
		WRect rect(item->pos.x + m_buyBtnN.rect.x,
			item->pos.y + m_buyBtnN.rect.y, m_buyBtnN.rect.w, m_buyBtnN.rect.h);
		if (rect.IsInRect(mouse))
		{
			buy = true;
		}
		else
		{
			WRect rectOver(item->pos.x + m_buyBtnO.rect.x,
				item->pos.y + m_buyBtnO.rect.y, m_buyBtnO.rect.w,
				m_buyBtnO.rect.h);
			if (rectOver.IsInRect(mouse))
			{
				buy = true;
			}
		}
	}
	if (!buy)
	{
		if (valid != 1)
		{
			return;
		}
		if (slot->itemData.dwTid == 0)
		{
			WRect rect(m_itemUploadN.rect.x + item->pos.x,
				item->pos.y + m_itemUploadN.rect.y, m_itemUploadN.rect.w,
				m_itemUploadN.rect.h);
			if (rect.IsInRect(mouse) ||
				(m_itemUploadO.rect + item->pos).IsInRect(mouse))
			{
				m_pSelectedSlot = item;
				FrForm::OnOK();
				return;
			}
		}
		if (valid != 1)
		{
			return;
		}
		WRect rect(item->pos.x + m_extendBtnN.rect.x,
			item->pos.y + m_extendBtnN.rect.y, m_extendBtnN.rect.w,
			m_extendBtnN.rect.h);
		if (!rect.IsInRect(mouse))
		{
			WRect rectOver(item->pos.x + m_extendBtnO.rect.x,
				item->pos.y + m_extendBtnO.rect.y, m_extendBtnO.rect.w,
				m_extendBtnO.rect.h);
			if (!rectOver.IsInRect(mouse))
			{
				return;
			}
		}
	}
	CBuyingSlotDlg* dlg =
		CreateForm<CBuyingSlotDlg>(manager, this, "buyslotdlg");
	dlg->SetSlotNum(item->idx + 1);
	dlg->Open(NULL, 1);
}

void FrOfflineTradeFinishEditDlg::SetListTradeItem(std::list<sTradeItem>* list)
{
	sStall* stall = Doc()->GetStallItem();
	m_pTradeItemList->ClearItem();
	for (int i = 0; i < stall->maxSlotNum; i++)
	{
		m_pTradeItemList->AddItem(&stall->slotList[i]);
	}
}

void FrOfflineTradeFinishEditDlg::GetSelectSlotInfo(int& index, int& valid)
{
	if (m_pSelectedSlot == NULL)
	{
		index = valid = 0;
		return;
	}
	sStallSlot* slot = (sStallSlot*)m_pSelectedSlot->pData;
	index = slot->stallKey;
	valid = slot->Idx;
}

void FrOfflineTradeFinishEditDlg::OnToolTipInit(int param)
{
	m_pToolTip = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrOfflineTradeFinishEditDlg::OnToolTipOnwerDraw(int param)
{
	CIconToolTip::GetInstance()->Render();
}

IMPLEMENT_OBJECT(COfflineTradeDlg, FrForm)

BEGIN_FRESH_MSGMAP(COfflineTradeDlg, FrForm)

ON_FRESH_VI("AreaSub_Nick", FRCMD_INIT, COfflineTradeDlg::OnAreaSubNickInit)
ON_FRESH_VI("AreaSub_Nick", FRCMD_OWNERDRAW,
	COfflineTradeDlg::OnAreaSubNickOwnerDraw)
ON_FRESH_VV("AreaSub_Nick", FRCMD_LBUTTONUP,
	COfflineTradeDlg::OnAreaSubNickLBtnUP)
ON_FRESH_VI("AreaSub_Price", FRCMD_INIT, COfflineTradeDlg::OnAreaSubPriceInit)
ON_FRESH_VI("AreaSub_Price", FRCMD_OWNERDRAW,
	COfflineTradeDlg::OnAreaSubPriceOwnerDraw)
ON_FRESH_VV("AreaSub_Price", FRCMD_LBUTTONUP,
	COfflineTradeDlg::OnAreaSubPriceLBtnUP)
ON_FRESH_VI("AreaSub_ItemName", FRCMD_INIT,
	COfflineTradeDlg::OnAreaSubItemNameInit)
ON_FRESH_VI("AreaSub_ItemName", FRCMD_OWNERDRAW,
	COfflineTradeDlg::OnAreaSubItemNameOwnerDraw)
ON_FRESH_VV("AreaSub_ItemName", FRCMD_LBUTTONUP,
	COfflineTradeDlg::OnAreaSubItemNameLBtnUP)
ON_FRESH_VI("Edit_ItemSearch", FRCMD_INIT, COfflineTradeDlg::OnItemSearchInit)
ON_FRESH_BI("Edit_ItemSearch", FRCMD_ENTERKEY,
	COfflineTradeDlg::OnItemSearchEnterKey)
ON_FRESH_VV("Edit_ItemSearch", FRCMD_LBUTTONUP,
	COfflineTradeDlg::OnItemSearchLBtnUp)
ON_FRESH_VI("BtnMain_search", FRCMD_INIT, COfflineTradeDlg::OnBtnMainSearchInit)
ON_FRESH_VV("BtnMain_search", FRCMD_LBUTTONUP,
	COfflineTradeDlg::OnBtnMainSearchUp)
ON_FRESH_VI("num_maxpage", FRCMD_INIT, COfflineTradeDlg::OnEditMaxPageInit)
ON_FRESH_VI("num_curpage", FRCMD_INIT, COfflineTradeDlg::OnEditCurrentPageInit)
ON_FRESH_BI("num_curpage", FRCMD_ENTERKEY,
	COfflineTradeDlg::OnEditCurrentPageEnterKey)
ON_FRESH_VI("BtnPage_Prev", FRCMD_INIT, COfflineTradeDlg::OnBtnPrevInit)
ON_FRESH_VV("BtnPage_Prev", FRCMD_LBUTTONUP, COfflineTradeDlg::OnBtnPrevBtnUp)
ON_FRESH_VI("BtnPage_Next", FRCMD_INIT, COfflineTradeDlg::OnBtnNextInit)
ON_FRESH_VV("BtnPage_Next", FRCMD_LBUTTONUP, COfflineTradeDlg::OnBtnNextBtnUp)
ON_FRESH_VI("BtnPage_go", FRCMD_INIT, COfflineTradeDlg::OnBtnGoPageInit)
ON_FRESH_VV("BtnPage_go", FRCMD_LBUTTONUP, COfflineTradeDlg::OnBtnGoPageBtnUp)
ON_FRESH_VI("BtnRefresh", FRCMD_INIT, COfflineTradeDlg::OnBtnRefreshInit)
ON_FRESH_VV("BtnRefresh", FRCMD_LBUTTONUP, COfflineTradeDlg::OnBtnRefreshBtnUp)
ON_FRESH_VI("BtnSub_Quick", FRCMD_INIT, COfflineTradeDlg::OnBtnDetailInit)
ON_FRESH_VV("BtnSub_Quick", FRCMD_LBUTTONUP, COfflineTradeDlg::OnBtnDetailBtnUp)
ON_FRESH_VI("offlinetrade_list", FRCMD_INIT, COfflineTradeDlg::OnMaketListInit)
ON_FRESH_VI("offlinetrade_list", FRCMD_OWNERDRAW,
	COfflineTradeDlg::OnMaketListOwnerDraw)
ON_FRESH_VV("offlinetrade_list", FRCMD_LBUTTONDOWN,
	COfflineTradeDlg::OnMaketListBtnDown)
ON_FRESH_VI("Edit_Msg", FRCMD_INIT, COfflineTradeDlg::OnEditMsgInit)
ON_FRESH_VV("Edit_Msg", FRCMD_LBUTTONUP, COfflineTradeDlg::OnEditMsgLBtnUp)
ON_FRESH_VI("tooltip", FRCMD_INIT, COfflineTradeDlg::OnToolTipInit)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW, COfflineTradeDlg::OnToolTipOnwerDraw)

END_FRESH_MSGMAP()

COfflineTradeDlg::COfflineTradeDlg()
{
	m_pAreaSubNick = NULL;
	m_pAreaSubPrice = NULL;
	m_pAreaSubItemName = NULL;
	m_pItemSearch = NULL;
	m_pMaketList = NULL;
	m_pCurPage = NULL;
	m_pMaxPage = NULL;
	m_pBtnPrev = NULL;
	m_pBtnNext = NULL;
	m_pBtnGoPage = NULL;
	m_pBtnMainSearch = NULL;
	m_pBtnRefresh = NULL;
	m_pStandingInLine = NULL;
	m_pBtnDetail = NULL;
	m_pEditMsg = NULL;
	m_maxPage = 0;
	m_curPage = 0;
	m_sortType = 1;
	m_searchOrder = 0;
	m_searchString.clear();
	SetSearchString(std::string(
		"\xb9\xb0\xb0\xc7\xc0\xbb \xb0\xf1\xb6\xf3\xba\xb8\xb7\xc5"));
	m_searchFlag645 = false;
	m_tradeMode = 0;
	m_moveBtn.pBitmap = g_pFresh->GetBitmap("rmr_btn_move_n");
	m_moveBtn.rect = WRect(490.0f, 1.0f, m_moveBtn.pBitmap->Width(),
		m_moveBtn.pBitmap->Height());
	m_moveOverBtn.pBitmap = g_pFresh->GetBitmap("rmr_btn_move_o");
	m_moveOverBtn.rect = WRect(490.0f, 1.0f, m_moveOverBtn.pBitmap->Width(),
		m_moveOverBtn.pBitmap->Height());
}

COfflineTradeDlg::~COfflineTradeDlg()
{
}

bool COfflineTradeDlg::OnInit()
{
	if (m_pItemSearch)
	{
		m_pItemSearch->SetFontColor(0xffd1d1d0);
		m_pItemSearch->ClearLine();
		m_pItemSearch->AddLine(
			"\xb9\xb0\xb0\xc7\xc0\xbb \xb0\xf1\xb6\xf3\xba\xb8\xb7\xc5", 0,
			false);
		m_pItemSearch->SetKeyFocus(true);
	}
	int result = 0;
	CTask* task = AfxGetTask();
	MsgObject msg(NULL, 0x231, (int)&result, 2, 3, 0, 0);
	task->SendMsgToMainActor(msg);
	m_tradeMode = result;
	if (result == 0)
	{
		m_pEditMsg->SetVisible(true);
		m_pEditMsg->AddText(
			"\x20\x2a\x20\xc7\xf6\xc0\xe7 \xbb\xf3\xc5\xc2\xbf\xa1\xbc\xad\xb4\xc2 \xb4\xd9\xb8\xa5 \xbb\xe7\xb6\xf7\xc0\xc7 \xb8\xb6\xc0\xcc\xb7\xeb\xc0\xbb \xb9\xe6\xb9\xae \xc7\xcf\xbd\xc7\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
			false, true);
		return true;
	}
	m_pEditMsg->SetVisible(false);
	return true;
}

void COfflineTradeDlg::OnProc(const float dt)
{
	FrForm::OnProc(dt);
	CIconToolTip::GetInstance()->ClearToolTip();
}

void COfflineTradeDlg::OnAreaSubNickInit(int param)
{
	m_pAreaSubNick = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pAreaSubNick)
	{
		const WRect& cur = m_pAreaSubNick->GetRect();
		WRect rect(cur.x, cur.y, cur.w + 55.0f, cur.h);
		m_pAreaSubNick->SetRect(rect);
	}
}

void COfflineTradeDlg::OnAreaSubNickOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	gdi->SetTextColor(0xffffffff, 0xffffffff);
	WRect rect = m_pAreaSubNick->GetRect();
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	if (mouse.x >= rect.x && mouse.x <= rect.x + rect.w && mouse.y >= rect.y &&
		mouse.y <= rect.y + rect.h)
	{
		gdi->SetTextColor(0xffffffff, 0xff000000);
		gdi->SetTextStyle(2);
	}
	else
	{
		gdi->SetTextStyle(0);
	}
	gdi->Print(WPoint(rect.x + 10.0f, rect.y), 0, K2L_Compatibility("\xb4\xd0\xb3\xd7\xc0\xd3"));
}

void COfflineTradeDlg::OnAreaSubNickLBtnUP()
{
	m_sortType = 4;
	m_searchOrder = 0;
	m_curPage = 1;
	if (SetSearchString(std::string("")))
	{
		RequestMaketList();
	}
}

void COfflineTradeDlg::OnAreaSubPriceInit(int param)
{
	m_pAreaSubPrice = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pAreaSubPrice)
	{
		const WRect& cur = m_pAreaSubPrice->GetRect();
		WRect rect(cur.x, cur.y, cur.w + 35.0f, cur.h);
		m_pAreaSubPrice->SetRect(rect);
	}
}

void COfflineTradeDlg::OnAreaSubPriceOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	gdi->SetTextColor(0xffffffff, 0xffffffff);
	WRect rect = m_pAreaSubPrice->GetRect();
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	if (mouse.x >= rect.x && mouse.x <= rect.x + rect.w && mouse.y >= rect.y &&
		mouse.y <= rect.y + rect.h)
	{
		gdi->SetTextColor(0xffffffff, 0xff000000);
		gdi->SetTextStyle(2);
	}
	else
	{
		gdi->SetTextStyle(0);
	}
	gdi->Print(WPoint(rect.x + 10.0f, rect.y), 0, "\xb0\xa1\xb0\xdd");
}

void COfflineTradeDlg::OnAreaSubPriceLBtnUP()
{
	m_sortType = 3;
	m_searchOrder = 0;
	m_curPage = 1;
	if (SetSearchString(std::string("")))
	{
		RequestMaketList();
	}
}

void COfflineTradeDlg::OnAreaSubItemNameInit(int param)
{
	m_pAreaSubItemName = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pAreaSubItemName)
	{
		const WRect& cur = m_pAreaSubItemName->GetRect();
		WRect rect(cur.x, cur.y, cur.w + 75.0f, cur.h);
		m_pAreaSubItemName->SetRect(rect);
	}
}

void COfflineTradeDlg::OnAreaSubItemNameOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	gdi->SetTextColor(0xffffffff, 0xffffffff);
	WRect rect = m_pAreaSubItemName->GetRect();
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	if (mouse.x >= rect.x && mouse.x <= rect.x + rect.w && mouse.y >= rect.y &&
		mouse.y <= rect.y + rect.h)
	{
		gdi->SetTextColor(0xffffffff, 0xff000000);
		gdi->SetTextStyle(2);
	}
	else
	{
		gdi->SetTextStyle(0);
	}
	gdi->Print(WPoint(rect.x + 10.0f, rect.y), 0,
		"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc\xb8\xa7");
}

void COfflineTradeDlg::OnAreaSubItemNameLBtnUP()
{
	m_sortType = 1;
	m_searchOrder = 0;
	m_curPage = 1;
	if (SetSearchString(std::string("")))
	{
		RequestMaketList();
	}
}

void COfflineTradeDlg::OnMaketListInit(int param)
{
	m_pMaketList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void COfflineTradeDlg::SetListItem()
{
	char text[32];
	std::list<sSaleItem>* list = Doc()->GetMaketList();
	if (m_pMaketList)
	{
		m_pMaketList->ClearItem();
		for (std::list<sSaleItem>::iterator it = list->begin();
			it != list->end(); it++)
		{
			m_pMaketList->AddItem(&(*it));
		}
	}
}

inline WRect WRect::operator+(WPoint& point) const
{
	return WRect(x + point.x, y + point.y, w, h);
}

void COfflineTradeDlg::OnMaketListOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	FrListItem* item = (FrListItem*)param;
	if (gdi && item)
	{
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(0);
		sSaleItem* sale = (sSaleItem*)item->pData;
		const Bitmap* select = g_pFresh->GetBitmap("maketlist_select");
		if ((item->selected || item->underCursor) && select)
		{
			gdi->DrawTexture(select,
				WRect(item->pos.x, item->pos.y - 5.0f, 520.0f, 23.0f),
				0xffffffff, 0);
		}
		if (sale->bOpen == 1)
		{
			gdi->Print(item->pos + WPoint(470.0f, 1.0f), 2,
				K2L_Compatibility("\xb0\xf8  \xb0\xb3"));
		}
		else
		{
			gdi->SetTextColor(0xffd1d1d0, 0xffffffff);
			gdi->Print(item->pos + WPoint(470.0f, 1.0f), 2,
				K2L_Compatibility("\xba\xf1\xb0\xf8\xb0\xb3"));
		}
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(sale->typeId);
		if (common)
		{
			gdi->Print(item->pos + WPoint(160.0f, 1.0f), 2, "%s", common->Name);
		}
		gdi->Print(item->pos + WPoint(250.0f, 1.0f), 2, "%d", sale->price);
		gdi->Print(item->pos + WPoint(280.0f, 1.0f), 2, "%d", sale->count);
		gdi->Print(item->pos + WPoint(400.0f, 1.0f), 2, "%s", sale->nickname);
		if (m_tradeMode != 0)
		{
			if (item->underCursor &&
				(m_moveBtn.rect + item->pos)
					.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				// HACK: dead copy of the else-branch rectangle. It emits no code, but without it the
				// operands of the hit test `rect.x + pos.x` come out swapped (fld pos.x; fadd rect.x).
				m_moveBtn.rect + item->pos + WPoint(0.0f, 0.0f);
				gdi->DrawTexture(m_moveOverBtn.pBitmap,
					m_moveOverBtn.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x1b);
			}
			else
			{
				// HACK: see above (same dead expression).
				m_moveBtn.rect + item->pos + WPoint(0.0f, 0.0f);
				gdi->DrawTexture(m_moveBtn.pBitmap,
					m_moveBtn.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
					0);
			}
		}
		gdi->SetTextColor(0xff000000, 0xffffffff);
	}
}

void COfflineTradeDlg::OnMaketListBtnDown()
{
	FrListItem* item = m_pMaketList->GetItemUnderCursor();
	if (item && !g_pFresh->IsCurrentLayout("REALMYROOM") &&
		!g_pFresh->IsCurrentLayout("REALMYROOMVISITOR"))
	{
		sSaleItem* sale = (sSaleItem*)item->pData;
		WRect rect(item->pos.x, item->pos.y, m_moveBtn.rect.w,
			m_moveBtn.rect.h);
		rect.x += m_moveBtn.rect.x;
		rect.y += m_moveBtn.rect.y;
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (!rect.IsInRect(mouse))
		{
			rect.x = item->pos.x;
			rect.y = item->pos.y;
			rect.x += m_moveOverBtn.rect.x;
			rect.y += m_moveOverBtn.rect.y;
			rect.w = m_moveOverBtn.rect.w;
			rect.h = m_moveOverBtn.rect.h;
			if (!rect.IsInRect(mouse))
			{
				return;
			}
		}
		Doc()->m_replayTargetUID = sale->ownerUID;
		if (Doc()->m_replayTargetUID == -1)
		{
			FrForm* form =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
			form->SetMessage(
				"\xb9\xe6\xb9\xae\xc7\xd2 \xc0\xaf\xc0\xfa\xc0\xc7 \xc1\xa4\xba\xb8\xb8\xa6 \xbe\xf2\xbe\xee\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.",
				false);
			form->Open(NULL, 3);
			return;
		}
		if (Doc()->m_myInfo.info.dwUID == Doc()->m_replayTargetUID)
		{
			Doc()->m_rmrMode = 1;
		}
		else
		{
			Doc()->m_rmrMode = 5;
		}
		MsgObject msg(NULL, 0x222, 0, 0, 0, 0, 0);
		AfxGetTask()->SendMsgToMainActor(msg);
	}
}

void COfflineTradeDlg::OnItemSearchInit(int param)
{
	m_pItemSearch = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pItemSearch)
	{
		m_pItemSearch->SetFontColor(0xffd1d1d0);
		m_pItemSearch->ClearLine();
		m_pItemSearch->AddLine(
			"\xb9\xb0\xb0\xc7\xc0\xbb \xb0\xf1\xb6\xf3\xba\xb8\xb7\xc5", 0,
			false);
		m_pItemSearch->SetKeyFocus(true);
	}
}

bool COfflineTradeDlg::OnItemSearchEnterKey(int param)
{
	OnAreaSubItemNameLBtnUP();
	return true;
}

void COfflineTradeDlg::OnItemSearchLBtnUp()
{
	std::string text = m_pItemSearch->GetLine(1, false);
	std::string empty;
	empty.clear();
	const char* str = text.c_str();
	if (stricmp("\xb9\xb0\xb0\xc7\xc0\xbb \xb0\xf1\xb6\xf3\xba\xb8\xb7\xc5",
			str) == 0)
	{
		m_pItemSearch->SetFontColor(0xff000000);
		m_pItemSearch->ClearLine();
		m_pItemSearch->AddLine("", 0, false);
		m_pItemSearch->SetKeyFocus(true);
	}
}

void COfflineTradeDlg::OnBtnMainSearchInit(int param)
{
	m_pBtnMainSearch = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void COfflineTradeDlg::OnBtnMainSearchUp()
{
	m_sortType = 1;
	m_searchOrder = 0;
	m_curPage = 1;
	if (SetSearchString(std::string("")))
	{
		RequestMaketList();
	}
}

void COfflineTradeDlg::OnEditMaxPageInit(int param)
{
	m_pMaxPage = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pMaxPage)
	{
		m_pMaxPage->AddLine(MakeStr("%d", m_maxPage), 0, false);
	}
}

void COfflineTradeDlg::OnEditCurrentPageInit(int param)
{
	m_pCurPage = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pCurPage)
	{
		m_pCurPage->AddLine(MakeStr("%d", m_curPage), 0, false);
	}
}

bool COfflineTradeDlg::OnEditCurrentPageEnterKey(int param)
{
	return true;
}

void COfflineTradeDlg::SetPageData(int curPage, int maxPage)
{
	if (0)
		((COfflineTradeDlg*)0)
			->COfflineTradeDlg::OnEditCurrentPageInit(*(int*)0);
	if (curPage != -1)
	{
		m_curPage = curPage;
		m_pBtnDetail->SetVisible(false);
	}
	m_maxPage = maxPage;
	if (m_curPage < 1)
	{
		m_curPage = 1;
	}
	if (m_curPage > maxPage)
	{
		m_curPage = maxPage;
	}
	if (m_pCurPage)
	{
		m_pCurPage->ClearLine();
		m_pCurPage->AddLine(MakeStr("%d", m_curPage), 0, false);
	}
	if (m_pMaxPage)
	{
		m_pMaxPage->ClearLine();
		m_pMaxPage->AddLine(MakeStr("%d", m_maxPage), 0, false);
	}
}

void COfflineTradeDlg::RequestMaketList()
{
	if (0)
		((COfflineTradeDlg*)0)->COfflineTradeDlg::OnMaketListBtnDown();
	if (stricmp("\xb9\xb0\xb0\xc7\xc0\xbb \xb0\xf1\xb6\xf3\xba\xb8\xb7\xc5",
			m_searchString.c_str()) == 0)
	{
		m_searchString.clear();
	}
	if (m_searchString.size() != 0)
	{
		if (!IsLocalContent(S4_OFFLINE_SHOP))
		{
			return;
		}
		WSendPacketSem983 packet((enumClientPacket)0xc9); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode1(m_sortType);
		packet.Encode1(m_searchOrder);
		packet.EncodeStr(m_searchString);
		packet.Encode2(m_curPage);
		packet.Send(TO_GAME);
		CTask* task = AfxGetTask();
		MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
		task->SendMsgToMainActor(msg);
		m_pBtnDetail->SetVisible(true);
	}
	else
	{
		if (!IsLocalContent(S4_OFFLINE_SHOP))
		{
			return;
		}
		WSendPacketSem983 packet((enumClientPacket)0xca); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode2(m_curPage);
		packet.Send(TO_GAME);
		CTask* task = AfxGetTask();
		MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
		task->SendMsgToMainActor(msg);
	}
}

void COfflineTradeDlg::OnBtnPrevInit(int param)
{
	m_pBtnPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void COfflineTradeDlg::OnBtnPrevBtnUp()
{
	m_curPage = m_curPage - 1;
	SetPageData(m_curPage, m_maxPage);
	RequestMaketList();
}

void COfflineTradeDlg::OnBtnNextInit(int param)
{
	m_pBtnNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void COfflineTradeDlg::OnBtnNextBtnUp()
{
	m_curPage = m_curPage + 1;
	SetPageData(m_curPage, m_maxPage);
	RequestMaketList();
}

void COfflineTradeDlg::OnBtnGoPageInit(int param)
{
	m_pBtnGoPage = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void COfflineTradeDlg::OnBtnGoPageBtnUp()
{
	m_curPage = atoi(m_pCurPage->GetLine(1, false));
	SetPageData(m_curPage, m_maxPage);
	RequestMaketList();
}

void COfflineTradeDlg::OnBtnRefreshInit(int param)
{
	m_pBtnRefresh = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void COfflineTradeDlg::OnBtnRefreshBtnUp()
{
	SetPageData(m_curPage = 1, m_maxPage);
	m_sortType = 1;
	m_searchOrder = 0;
	m_searchString.clear();
	SetSearchString(std::string(
		"\xb9\xb0\xb0\xc7\xc0\xbb \xb0\xf1\xb6\xf3\xba\xb8\xb7\xc5"));
	m_searchFlag645 = false;
	m_tradeMode = 0;
	m_pEditMsg->SetVisible(true);
	m_pEditMsg->AddText(
		"\x20\x2a\x20\xc7\xf6\xc0\xe7 \xbb\xf3\xc5\xc2\xbf\xa1\xbc\xad\xb4\xc2 \xb4\xd9\xb8\xa5 \xbb\xe7\xb6\xf7\xc0\xc7 \xb8\xb6\xc0\xcc\xb7\xeb\xc0\xbb \xb9\xe6\xb9\xae \xc7\xcf\xbd\xc7\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
		false, true);
	if (IsLocalContent(S4_OFFLINE_SHOP))
	{
		WSendPacketSem983 packet((enumClientPacket)0xca); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode2(m_curPage);
		packet.Send(TO_GAME);
		m_pItemSearch->SetFontColor(0xffd1d1d0);
		m_pItemSearch->ClearLine();
		m_pItemSearch->AddLine(
			"\xb9\xb0\xb0\xc7\xc0\xbb \xb0\xf1\xb6\xf3\xba\xb8\xb7\xc5", 0,
			false);
		m_pItemSearch->SetKeyFocus(true);
		CTask* task = AfxGetTask();
		MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
		task->SendMsgToMainActor(msg);
	}
	m_pBtnDetail->SetVisible(false);
}

void COfflineTradeDlg::OnBtnDetailInit(int param)
{
	m_pBtnDetail = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pBtnDetail)
	{
		m_pBtnDetail->SetVisible(false);
	}
}

void COfflineTradeDlg::OnBtnDetailBtnUp()
{
	WRect rect = m_pBtnDetail->GetRect();
	if (!m_pStandingInLine)
	{
		m_pStandingInLine = CreateForm<CStandingInLine>(g_pFresh->GetManager(),
			this, "arraydlg");
		if (m_pStandingInLine)
		{
			m_pStandingInLine->Open(
				(FRESH_PFN_RESULT)&COfflineTradeDlg::OnStandingInLineDlgResult,
				WPoint(rect.x + 10.0f, rect.y), 1);
		}
	}
}

void COfflineTradeDlg::OnEditMsgInit(int param)
{
	m_pEditMsg = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pEditMsg)
	{
		m_pEditMsg->SetVisible(false);
	}
}

void COfflineTradeDlg::OnEditMsgLBtnUp()
{
}

void COfflineTradeDlg::OnToolTipInit(int param)
{
	m_pToolTip = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void COfflineTradeDlg::OnToolTipOnwerDraw(int param)
{
	CIconToolTip::GetInstance()->Render();
}

bool COfflineTradeDlg::SetSearchString(std::string search)
{
	if (search.size() == 0)
	{
		std::string text;
		if (m_pItemSearch)
		{
			text = m_pItemSearch->GetLine(1, false);
		}
		if (text.size() == 0)
		{
			FrForm* form =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
			form->SetMessage(
				"\xb0\xcb\xbb\xf6\xc7\xcf\xb7\xc1\xb4\xc2 \xb4\xdc\xbe\xee\xb0\xa1 \xbf\xc3\xb9\xd9\xb8\xa3\xc1\xf6 \xbe\xca\xb0\xc5\xb3\xaa \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xc0\xd4\xb7\xc2\xc7\xcf\xbc\xbc\xbf\xe4",
				false);
			form->Open(NULL, 3);
			m_searchString.clear();
			return false;
		}
		SetSearchString(text);
	}
	else
	{
		m_searchString = search;
	}
	return true;
}

bool COfflineTradeDlg::OnStandingInLineDlgResult(int result, FrForm* form)
{
	int selectNum = 0;
	if (result == 1)
	{
		if (m_pStandingInLine)
		{
			selectNum = m_pStandingInLine->GetSelectNum();
		}
	}
	switch (selectNum)
	{
	case 0:
		m_searchOrder = 0;
		break;
	case 1:
		m_searchOrder = 10;
		break;
	case 2:
		m_searchOrder = 11;
		break;
	case 3:
		m_searchOrder = 12;
		break;
	case 4:
		m_searchOrder = 13;
		break;
	case 5:
		m_searchOrder = 14;
		break;
	case 6:
		m_searchOrder = 15;
		break;
	case 7:
		m_searchOrder = 16;
		break;
	case 8:
		m_searchOrder = 17;
		break;
	case 9:
		m_searchOrder = 18;
		break;
	case 11:
		m_searchOrder = 21;
		break;
	case 12:
		m_searchOrder = 23;
		break;
	case 13:
		m_searchOrder = 22;
		break;
	case 14:
		m_searchOrder = 24;
		break;
	case 15:
		m_searchOrder = 19;
		break;
	}
	m_curPage = 1;
	RequestMaketList();
	m_pStandingInLine = NULL;
	return true;
}

IMPLEMENT_OBJECT(CBuyingSlotDlg, FrForm)

BEGIN_FRESH_MSGMAP(CBuyingSlotDlg, FrForm)

ON_FRESH_VI("days", FRCMD_INIT, CBuyingSlotDlg::OnDaysInit)
ON_FRESH_VV("days", FRCMD_LBUTTONDOWN, CBuyingSlotDlg::OnDaysBtnDown)
ON_FRESH_VI("my_money", FRCMD_INIT, CBuyingSlotDlg::OnMyMoneyInit)
ON_FRESH_VI("money_left", FRCMD_INIT, CBuyingSlotDlg::OnMoneyLeftInit)
ON_FRESH_VI("price", FRCMD_INIT, CBuyingSlotDlg::OnPriceInit)
ON_FRESH_VI("checkout", FRCMD_INIT, CBuyingSlotDlg::OnCheckOutInit)
ON_FRESH_VV("checkout", FRCMD_LBUTTONDOWN, CBuyingSlotDlg::OnCheckOutBtnDown)
ON_FRESH_VI("slot_price", FRCMD_INIT, CBuyingSlotDlg::OnSlotPriceInit)

END_FRESH_MSGMAP()

struct sdays_t
{
	const char* name;
	int days;
};
static sdays_t sDay[] = { // 983 init 00ceb350: nome = K2L
	{ K2L_Compatibility("1\xc0\xcf"),                (0, 1)   },
	{ K2L_Compatibility("7\xc0\xcf"),                (0, 7)   },
	{ K2L_Compatibility("15\xc0\xcf"),               (0, 15)  },
	{ K2L_Compatibility("30\xc0\xcf"),               (0, 30)  },
	{ K2L_Compatibility("365\xc0\xcf"),              (0, 365) },
	{ K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"), (0, 0)   },
};

CBuyingSlotDlg::CBuyingSlotDlg()
{
	m_pDays = NULL;
	m_pMyMoney = NULL;
	m_pMoneyLeft = NULL;
	m_pPrice = NULL;
	m_pCheckOut = NULL;
	m_pSlotPrice = NULL;
	m_price = 0;
}

CBuyingSlotDlg::~CBuyingSlotDlg()
{
}

void CBuyingSlotDlg::OnDaysInit(int param)
{
	m_pDays = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
	if (m_pDays)
	{
		m_pDays->ClearList();
		IFF_STRUCT::sFurniture* furniture =
			ItemManager()->FindFurniture(0x4800601e);
		for (int i = 0; i < 4; i++)
		{
			if (furniture->COM[i] != 0)
			{
				m_pDays->AddString(sDay[i].name);
			}
		}
		m_pDays->SetLine(1, sDay[0].name, 0, false, 0);
		m_price = furniture->COM[0];
		m_pSlotPrice->ClearLine();
		m_pSlotPrice->AddText(MakeStr("%d \xc6\xce", m_price), false, true);
	}
}

void CBuyingSlotDlg::OnDaysBtnDown()
{
	IFF_STRUCT::sFurniture* furniture =
		ItemManager()->FindFurniture(0x4800601e);
	const char* text = m_pDays->GetLine(1, false);
	for (int i = 0; i < 5; i++)
	{
		if (stricmp(sDay[i].name, text) == 0)
		{
			m_price = furniture->COM[i];
			m_pSlotPrice->ClearLine();
			m_pSlotPrice->AddText(MakeStr("%d \xc6\xce", m_price), false, true);
		}
	}
	RecalculatePang();
}

void CBuyingSlotDlg::OnPriceInit(int param)
{
	m_pPrice = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pPrice)
	{
		char buf[32];
		MakePriceStrA(buf, m_price);
		strcat(buf, K2L_Compatibility("\xc6\xce"));
		m_pPrice->SetLine(1, buf, 0, false, 0);
	}
}

void CBuyingSlotDlg::OnMyMoneyInit(int param)
{
	m_pMyMoney = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pMyMoney)
	{
		char buf[32];
		MakePriceStrA(buf, Doc()->m_myInfo.stat.i64Pang);
		strcat(buf, K2L_Compatibility("\xc6\xce"));
		m_pMyMoney->SetLine(1, buf, 0, false, 0);
	}
}

void CBuyingSlotDlg::OnMoneyLeftInit(int param)
{
	m_pMoneyLeft = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pMoneyLeft)
	{
		char buf[32];
		__int64 left = Doc()->m_myInfo.stat.i64Pang - m_price;
		if (left < 0)
		{
			MakePriceStrA(buf, m_price - Doc()->m_myInfo.stat.i64Pang);
			strcat(buf, K2L_Compatibility("\xc6\xce \xba\xce\xc1\xb7")); // 983 00a9ce23/00aac80b
			m_pMoneyLeft->SetLine(1, buf, 0xffff0000, false, 0);
		}
		else
		{
			MakePriceStrA(buf, left);
			strcat(buf, K2L_Compatibility("\xc6\xce"));
			m_pMoneyLeft->SetLine(1, buf, 0, false, 0);
		}
	}
}

void CBuyingSlotDlg::RecalculatePang()
{
	char buf[32];
	if (m_pPrice)
	{
		MakePriceStrA(buf, m_price);
		strcat(buf, K2L_Compatibility("\xc6\xce"));
		m_pPrice->SetLine(1, buf, 0, false, 0);
	}
	if (m_pMyMoney)
	{
		MakePriceStrA(buf, Doc()->m_myInfo.stat.i64Pang);
		strcat(buf, K2L_Compatibility("\xc6\xce"));
		m_pMyMoney->SetLine(1, buf, 0, false, 0);
	}
	if (m_pMoneyLeft)
	{
		__int64 left = Doc()->m_myInfo.stat.i64Pang - m_price;
		if (left < 0)
		{
			MakePriceStrA(buf, m_price - Doc()->m_myInfo.stat.i64Pang);
			strcat(buf, K2L_Compatibility("\xc6\xce \xba\xce\xc1\xb7")); // 983 00a9ce23/00aac80b
			m_pMoneyLeft->SetLine(1, buf, 0xffff0000, false, 0);
		}
		else
		{
			MakePriceStrA(buf, left);
			strcat(buf, K2L_Compatibility("\xc6\xce"));
			m_pMoneyLeft->SetLine(1, buf, 0, false, 0);
		}
	}
}

void CBuyingSlotDlg::OnCheckOutInit(int param)
{
	m_pCheckOut = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CBuyingSlotDlg::OnCheckOutBtnDown()
{
	IFF_STRUCT::sFurniture* furniture =
		ItemManager()->FindFurniture(0x4800601e);
	const char* text = m_pDays->GetLine(1, false);
	int days = 0;
	int i;
	for (i = 0; i < 5; i++)
	{
		if (stricmp(sDay[i].name, text) == 0)
		{
			days = sDay[i].days;
			break;
		}
	}
	sStall* stall = Doc()->GetStallItem();
	if (furniture->COM[i] <= Doc()->m_myInfo.stat.i64Pang)
	{
		if (IsLocalContent(S4_OFFLINE_SHOP))
		{
			WSendPacketSem983 packet((enumClientPacket)0xc5); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(stall->stallKey);
			packet.Encode4(0x4800601e);
			packet.Encode1(m_slotNum);
			packet.Encode4(days * 1440);
			packet.Send(TO_GAME);
			CTask* task = AfxGetTask();
			MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
			task->SendMsgToMainActor(msg);
		}
	}
	else
	{
		FrForm* form =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
		form->SetMessage(
			"\xbc\xd2\xc1\xf6\xc7\xcf\xbd\xc5 \xb1\xdd\xbe\xd7\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9. \xbd\xbd\xb7\xd4\xc0\xbb \xb1\xb8\xb8\xc5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
			false);
		form->Open(NULL, 3);
	}
	FrForm::OnOK();
}

void CBuyingSlotDlg::OnSlotPriceInit(int param)
{
	m_pSlotPrice = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pSlotPrice)
	{
		m_pSlotPrice->ClearLine();
		m_pSlotPrice->AddText(MakeStr("%d \xc6\xce", m_price), false, true);
	}
}

IMPLEMENT_OBJECT(COfflineShopEditDlg, FrForm)

BEGIN_FRESH_MSGMAP(COfflineShopEditDlg, FrForm)

ON_FRESH_VI("areaBg01", FRCMD_INIT, COfflineShopEditDlg::OnAreaBg01Init)
ON_FRESH_VI("areaBg02", FRCMD_INIT, COfflineShopEditDlg::OnAreaBg02Init)
ON_FRESH_VI("areaBg03", FRCMD_INIT, COfflineShopEditDlg::OnAreaBg03Init)
ON_FRESH_VI("areaBg04", FRCMD_INIT, COfflineShopEditDlg::OnAreaBg04Init)
ON_FRESH_VI("areaBg05", FRCMD_INIT, COfflineShopEditDlg::OnAreaBg05Init)
ON_FRESH_VI("edit_slot", FRCMD_INIT, COfflineShopEditDlg::OnStallSlotNumInit)
ON_FRESH_VI("edit_slotstate", FRCMD_INIT, COfflineShopEditDlg::OnSlotNumInit)
ON_FRESH_VI("tradeeditdlg", FRCMD_INIT, COfflineShopEditDlg::OnTradeEditDlgInit)
ON_FRESH_VI("my_item_list", FRCMD_INIT, COfflineShopEditDlg::OnMyItemListInit)
ON_FRESH_VV("my_item_list", FRCMD_LBUTTONDOWN,
	COfflineShopEditDlg::OnMyItemListLClick)
ON_FRESH_VV("my_item_list", FRCMD_RBUTTONDOWN,
	COfflineShopEditDlg::OnMyItemListRClick)
ON_FRESH_VI("my_item_list", FRCMD_OWNERDRAW,
	COfflineShopEditDlg::OnMyItemListOwnerDraw)
ON_FRESH_VI("trade_item_list", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeItemListInit)
ON_FRESH_VV("trade_item_list", FRCMD_LBUTTONDOWN,
	COfflineShopEditDlg::OnTradeItemListLClick)
ON_FRESH_VI("trade_item_list", FRCMD_OWNERDRAW,
	COfflineShopEditDlg::OnTradeItemListOwnerDraw)
ON_FRESH_VI("visitor", FRCMD_INIT, COfflineShopEditDlg::OnVisitorInit)
ON_FRESH_VI("userid", FRCMD_INIT, COfflineShopEditDlg::OnUserIdInit)
ON_FRESH_VI("trade_title", FRCMD_INIT, COfflineShopEditDlg::OnTitleInit)
ON_FRESH_VI("trade_title_edit", FRCMD_INIT,
	COfflineShopEditDlg::OnTitleEditInit)
ON_FRESH_BI("trade_title_edit", FRCMD_ENTERKEY,
	COfflineShopEditDlg::OnTitleEditEnterKey)
ON_FRESH_VI("trade_title_edit_bn", FRCMD_INIT,
	COfflineShopEditDlg::OnTitleEditBnInit)
ON_FRESH_VV("trade_title_edit_bn", FRCMD_LBUTTONUP,
	COfflineShopEditDlg::OnTitleEditBnUp)
ON_FRESH_VI("trade_myitem_icon", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeMyItemIconInit)
ON_FRESH_VV("trade_myitem_icon", FRCMD_OWNERDRAW,
	COfflineShopEditDlg::OnTradeMyItemIconOwnerDraw)
ON_FRESH_VI("trade_myitem_name", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeMyItemNameInit)
ON_FRESH_VI("trade_myitem_ability", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeMyItemAbilityInit)
ON_FRESH_VI("trade_myitem_number", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeMyItemNumberInit)
ON_FRESH_VI("trade_myitem_price", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeMyItemPriceInit)
ON_FRESH_VV("trade_myitem_price", FRCMD_LBUTTONUP,
	COfflineShopEditDlg::OnTradeMyItemPriceClick)
ON_FRESH_VI("trade_myitem_ok_bn", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeMyItemOkBnInit)
ON_FRESH_VV("trade_myitem_ok_bn", FRCMD_LBUTTONUP,
	COfflineShopEditDlg::OnTradeMyItemOkBnUp)
ON_FRESH_VI("trade_myitem_cancel_bn", FRCMD_INIT,
	COfflineShopEditDlg::OnTradeMyItemCancelBnInit)
ON_FRESH_VV("trade_myitem_cancel_bn", FRCMD_LBUTTONUP,
	COfflineShopEditDlg::OnTradeMyItemCancelBnUp)
ON_FRESH_VI("open", FRCMD_INIT, COfflineShopEditDlg::OnOpenBnInit)
ON_FRESH_VV("open", FRCMD_LBUTTONUP, COfflineShopEditDlg::OnOpenBnUp)
ON_FRESH_VI("close", FRCMD_INIT, COfflineShopEditDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, COfflineShopEditDlg::OnCloseBnUp)
ON_FRESH_VI("exit", FRCMD_INIT, COfflineShopEditDlg::OnExitBnInit)
ON_FRESH_VV("exit", FRCMD_LBUTTONUP, COfflineShopEditDlg::OnExitBnUp)
ON_FRESH_VI("tooltip", FRCMD_INIT, COfflineShopEditDlg::OnToolTipInit)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW, COfflineShopEditDlg::OnToolTipOnwerDraw)

END_FRESH_MSGMAP()

COfflineShopEditDlg::COfflineShopEditDlg()
{
	m_pAreaBg01 = NULL;
	m_pAreaBg02 = NULL;
	m_pAreaBg03 = NULL;
	m_pAreaBg04 = NULL;
	m_pAreaBg05 = NULL;
	m_pShopCloseBn = NULL;
	m_pStallSlotNum = NULL;
	m_pSlotNum = NULL;
	m_usedSlotCount = 0;
	m_selectData[0] = 0;
	m_selectData[1] = 0;
	m_pFinishEditDlg = NULL;
	m_closing = 0;
	m_buyBtnN.pBitmap = g_pFresh->GetBitmap("buy_btn_n");
	m_buyBtnN.rect = WRect(120.0f, 30.0f, m_buyBtnN.pBitmap->Width(),
		m_buyBtnN.pBitmap->Height());
	m_buyBtnO.pBitmap = g_pFresh->GetBitmap("buy_btn_o");
	m_buyBtnO.rect = WRect(120.0f, 30.0f, m_buyBtnO.pBitmap->Width(),
		m_buyBtnO.pBitmap->Height());
	m_tradeItemDelBnNormal.pBitmap =
		g_pFresh->GetBitmap("trade_item_del_bn_normal");
	m_tradeItemDelBnNormal.rect =
		WRect(120.0f, 30.0f, m_tradeItemDelBnNormal.pBitmap->Width(),
			m_tradeItemDelBnNormal.pBitmap->Height());
	m_tradeItemDelBnOver.pBitmap =
		g_pFresh->GetBitmap("trade_item_del_bn_over");
	m_tradeItemDelBnOver.rect =
		WRect(120.0f, 30.0f, m_tradeItemDelBnOver.pBitmap->Width(),
			m_tradeItemDelBnOver.pBitmap->Height());
}

COfflineShopEditDlg::~COfflineShopEditDlg()
{
	if (m_closing == 0)
	{
		if (IsLocalContent(S4_OFFLINE_SHOP))
		{
		}
	}
}

void COfflineShopEditDlg::OnAreaBg01Init(int param)
{
	m_pAreaBg01 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pAreaBg01)
	{
		m_pAreaBg01->SetBgImg("rmr_trade_edit_back_01");
	}
}

void COfflineShopEditDlg::OnAreaBg02Init(int param)
{
	m_pAreaBg02 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pAreaBg02)
	{
		m_pAreaBg02->SetBgImg("rmr_trade_edit_back_02");
	}
}

void COfflineShopEditDlg::OnAreaBg03Init(int param)
{
	m_pAreaBg03 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pAreaBg03)
	{
		m_pAreaBg03->SetBgImg("rmr_trade_edit_back_03");
	}
}

void COfflineShopEditDlg::OnAreaBg04Init(int param)
{
	m_pAreaBg04 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pAreaBg04)
	{
		m_pAreaBg04->SetBgImg("rmr_trade_edit_back_04");
	}
}

void COfflineShopEditDlg::OnAreaBg05Init(int param)
{
	m_pAreaBg05 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pAreaBg05)
	{
		m_pAreaBg05->SetVisible(false);
	}
}

void COfflineShopEditDlg::OnStallSlotNumInit(int param)
{
	m_pStallSlotNum = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void COfflineShopEditDlg::OnSlotNumInit(int param)
{
	m_pSlotNum = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void COfflineShopEditDlg::OnTradeItemListInit(int param)
{
	m_pShopItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (IsLocalContent(S4_OFFLINE_SHOP) && Doc()->GetTradeMode() == 1)
	{
		sStall* stall = Doc()->GetStallItem();
		for (int i = 0; i < stall->maxSlotNum; i++)
		{
			m_pShopItemList->AddItem(&stall->slotList[i]);
		}
	}
}

void COfflineShopEditDlg::OnTradeItemListLClick()
{
	FrListItem* item = m_pShopItemList->GetItemUnderCursor();
	if (item == NULL || item->pData == NULL)
	{
		return;
	}
	item->selected = !item->selected;
}

// HACK: this literal is owned by the naked
// FrOfflineTradeFinishEditDlg::OnTradeItemListOwnerDraw (local static); a
// genuine literal here would be emitted a second time.
#define OTD_STR_STALL_SLOT \
	"\xbb\xf3\xc1\xa1 \xbe\xc6\xc0\xcc\xc5\xdb \xbd\xbd\xb7\xd4"

void COfflineShopEditDlg::OnTradeItemListOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	FrListItem* item = (FrListItem*)param;
	if (gdi && item)
	{
		sStallSlot* slot = (sStallSlot*)item->pData;
		if (m_pBase[2])
		{
			WRect rect(item->pos.x + 1.0f, item->pos.y + 1.0f,
				(float)m_pBase[2]->Width(), (float)m_pBase[2]->Height());
			gdi->DrawTexture(m_pBase[2], rect, 0xffffffff, 0);
		}
		if (slot->IsValid == 1 && slot->itemData.dwTid == 0)
		{
			gdi->Print(WPoint(item->pos.x + 10.0f, item->pos.y + 13.0f), 0,
				OTD_STR_STALL_SLOT);
		}
		if (slot->IsValid == 0 && slot->itemData.dwTid == 0)
		{
			gdi->Print(WPoint(item->pos.x + 10.0f, item->pos.y + 13.0f), 0,
				"\xbb\xf3\xc1\xa1 \xbe\xc6\xc0\xcc\xc5\xdb \xbd\xbd\xb7\xd4 \xb1\xb8\xb8\xc5  ");
		}
		else if (slot->itemData.dwTid == 0x4800581f ||
			slot->itemData.dwTid == 0x4800601e)
		{
			gdi->Print(WPoint(item->pos.x + 10.0f, item->pos.y + 13.0f), 0,
				OTD_STR_STALL_SLOT);
		}
		else if (slot->itemData.dwTid != 0)
		{
			IFF_ITEM_COMMON* common =
				ItemManager()->FindCommonItem(slot->itemData.dwTid);
			if (m_pBase[2])
			{
				WRect rect(item->pos.x + 1.0f, item->pos.y + 1.0f,
					(float)m_pBase[2]->Width(), (float)m_pBase[2]->Height());
				gdi->DrawTexture(m_pBase[2], rect, 0xffffffff, 0);
			}
			char price[32] = "";
			MakePriceStrA(price, slot->itemData.i64Price);
			if (slot->itemData.iNum == 1)
			{
				MakeStr(K2L_Compatibility("%s\xc6\xce"), price);
			}
			else
			{
				MakeStr(K2L_Compatibility("\xb0\xb3\xb4\xe7%s\xc6\xce"), price);
			}
			gdi->Print(WPoint(item->pos.x + 10.0f, item->pos.y + 13.0f), 0,
				"%s", common->Name);
		}
	}
}

void COfflineShopEditDlg::OnVisitorInit(int param)
{
	m_pVisitor = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (IsLocalContent(S4_OFFLINE_SHOP) && Doc()->GetTradeMode() == 1)
	{
		m_pVisitor->SetVisible(false);
	}
}

void COfflineShopEditDlg::OnTradeMyItemOkBnUp()
{
	if (m_pBuyItem == NULL)
	{
		return;
	}
	sTradeItem item = *m_pBuyItem;
	int num = atoi(m_pMyItemNumber->GetLine(1, false));
	char buf[10] = { 0 };
	const char* text = m_pMyItemPrice->GetLine(1, false);
	char* dst = buf;
	for (int i = 0; i < 9; i++)
	{
		char ch = text[i];
		if (ch == '\0')
		{
			break;
		}
		if (ch >= '0' && ch <= '9')
		{
			*dst++ = ch;
		}
		else if (ch != ',')
		{
			buf[0] = '\0';
			break;
		}
	}
	__int64 price = atoi(buf);
	for (std::list<sTradeItem>::iterator it = m_listTradeItem.begin();
		it != m_listTradeItem.end(); ++it)
	{
		if (&*it == m_pBuyItem)
		{
			item.iNum = it->iNum;
			for (std::list<sTradeItem>::iterator my = m_listMyItem.begin();
				my != m_listMyItem.end(); ++my)
			{
				if (my->dwGuid == m_pBuyItem->dwGuid)
				{
					item.iNum += my->iNum;
					break;
				}
			}
			break;
		}
	}
	if (num < 1 || item.iNum < num)
	{
		SetTradeResultCode(10, 0xe1, NULL);
		return;
	}
	if (price <= 0)
	{
		SetTradeResultCode(11, 0xe1, NULL);
		return;
	}
	if (Doc()->GetStallItem()->stallKey != m_selectData[0])
	{
		FrForm* notify =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
		notify->SetMessage(
			"\xc7\xf6\xc0\xe7 \xc1\xf8\xbf\xad\xb4\xeb\xc0\xc7 \xc1\xa4\xba\xb8\xb0\xa1 \xc0\xcf\xc4\xa1\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb5\xee\xb7\xcf \xbd\xc7\xc6\xd0\xc0\xd4\xb4\xcf\xb4\xd9.",
			false);
		notify->Open(NULL, 3);
		m_selectData[0] = 0;
		m_selectData[1] = 0;
		OnExitBnUp();
		return;
	}
	item.iNum = num;
	item.i64Price = price;
	if (!IsLocalContent(S4_OFFLINE_SHOP))
	{
		return;
	}
	WSendPacketSem983 packet((enumClientPacket)0xc7); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
	packet.Encode4(m_selectData[0]);
	packet.Encode2(1);
	packet.Encode1(m_selectData[1]);
	packet.EncodeBuffer(&item, sizeof(item));
	packet.Send(TO_GAME);
	CTask* task = AfxGetTask();
	MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
	task->SendMsgToMainActor(msg);
}

void COfflineShopEditDlg::SetSelectData(unsigned long idx, unsigned long valid)
{
	m_selectData[0] = idx;
	m_selectData[1] = valid;
}

void COfflineShopEditDlg::OnOpenBnInit(int param)
{
	m_pShopOpenBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pShopOpenBn)
	{
		m_pShopOpenBn->SetVisible(false);
	}
}

void COfflineShopEditDlg::OnCloseBnInit(int param)
{
	m_pShopCloseBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pShopCloseBn)
	{
		m_pShopCloseBn->SetVisible(false);
	}
}

void COfflineShopEditDlg::OnExitBnUp()
{
	m_closing = 1;
	COfflineShopDlg* dlg = CreateForm<COfflineShopDlg>(g_pFresh->GetManager(),
		this, "offlinetradedlg");
	if (dlg)
	{
		dlg->Open(NULL, 0xc0);
		dlg->OpenTradeShop(Doc()->m_replayTargetUID);
		dlg->SetListTradeItem(false);
		dlg->SetFadeout(false);
	}
	OnFreshCancel();
}

bool COfflineShopEditDlg::OnInit()
{
	FrForm::EnableDrag(false);
	SetListEnableTradingMyItem();
	SetListTradingItem();
	SetListMyItem();
	SetListTradeItem();
	SetTradeMyItem(NULL);
	SetControlsTradeMyItem(NULL, false);
	return true;
}

void COfflineShopEditDlg::InitAllUI()
{
	FrForm::EnableDrag(false);
	SetListEnableTradingMyItem();
	SetListTradingItem();
	SetListMyItem();
	SetListTradeItem();
	SetTradeMyItem(NULL);
	SetControlsTradeMyItem(NULL, false);
}

void COfflineShopEditDlg::OnProc(const float dt)
{
	FrForm::OnProc(dt);
	CIconToolTip::GetInstance()->ClearToolTip();
}

// HACK: the original discards this unit's copy of "%2d/%2d" in favour of the
// one in avatar_main.obj, which our avatar_main.cpp emits as a local static of
// a naked function; reference that one.
#define OTD_STR_SLOT_COUNT "%2d/%2d"

void COfflineShopEditDlg::SetListTradeItem()
{
	FrTradeEditDlg::SetListTradeItem();
	if (m_pShopItemList)
	{
		m_pShopItemList->ClearItem();
		m_usedSlotCount = 0;
		sStall* stall = Doc()->GetStallItem();
		if (!stall)
		{
			std::list<sStall>::iterator it = Doc()->m_listStallItem.begin();
			stall = &(*it);
		}
		if (!stall)
		{
			return;
		}
		for (int i = 0; i < stall->maxSlotNum; i++)
		{
			if (stall->slotList[i].Idx)
			{
				m_pShopItemList->AddItem(&stall->slotList[i]);
			}
			if (stall->slotList[i].itemData.dwTid != 0)
			{
				m_usedSlotCount++;
			}
		}
		if (m_pStallSlotNum)
		{
			m_pStallSlotNum->ClearLine();
			m_pStallSlotNum->AddLine(MakeStr("%d", stall->maxSlotNum), 0,
				false);
		}
		if (m_pSlotNum)
		{
			m_pSlotNum->ClearLine();
			m_pSlotNum->AddLine(
				MakeStr(OTD_STR_SLOT_COUNT, m_usedSlotCount, stall->useSlotNum),
				0, false);
		}
	}
}

void COfflineShopEditDlg::SetSlotData()
{
	m_usedSlotCount = 0;
	sStall* stall = Doc()->GetStallItem();
	if (!stall)
	{
		std::list<sStall>::iterator it = Doc()->m_listStallItem.begin();
		stall = &(*it);
	}
	if (!stall)
	{
		return;
	}
	for (int i = 0; i < stall->maxSlotNum; i++)
	{
		if (stall->slotList[i].itemData.dwTid != 0)
		{
			m_usedSlotCount++;
		}
	}
	if (m_pStallSlotNum)
	{
		m_pStallSlotNum->ClearLine();
		m_pStallSlotNum->AddLine(MakeStr("%d", stall->maxSlotNum), 0, false);
	}
	if (m_pSlotNum)
	{
		m_pSlotNum->ClearLine();
		m_pSlotNum->AddLine(
			MakeStr(OTD_STR_SLOT_COUNT, m_usedSlotCount, stall->useSlotNum), 0,
			false);
	}
}

bool COfflineShopEditDlg::IsCheckCardPats(unsigned long partsUid,
	unsigned long partsTid)
{
	std::list<sSCardAvilityPeriodInfo>& cards = Doc()->m_cardAbilityList[0];
	std::list<sSCardAvilityPeriodInfo>::iterator it;
	for (it = cards.begin(); it != cards.end(); ++it)
	{
		if ((*it).partsTid == partsTid && (*it).partsUid == partsUid)
		{
			break;
		}
	}
	return it == cards.end() ? true : false;
}

bool COfflineShopEditDlg::OnOfflineEquitCancelDlgResult(int result,
	FrForm* form)
{
	if (result == 1)
	{
		FrListItem* item = m_pShopItemList->GetItemUnderCursor();
		if (item)
		{
			if (IsLocalContent(S4_OFFLINE_SHOP))
			{
				WSendPacketSem983 packet((enumClientPacket)0xc8); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
				packet.Encode4(Doc()->GetStallItem()->stallKey);
				packet.Encode2(1);
				packet.Encode1(item->idx + 1);
				packet.Send(TO_GAME);
				CTask* task = AfxGetTask();
				MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
				task->SendMsgToMainActor(msg);
			}
		}
	}
	return true;
}

bool COfflineShopEditDlg::OnOfflineTradeItemOnDlgResult(int result,
	FrForm* form)
{
	if (result == 1)
	{
		int index;
		int valid;
		m_pFinishEditDlg->GetSelectSlotInfo(index, valid);
		sTradeItem item = *m_pBuyItem;
		int num = atoi(m_pMyItemNumber->GetLine(1, false));
		char buf[10] = { 0 };
		const char* text = m_pMyItemPrice->GetLine(1, false);
		char* dst = buf;
		for (int i = 0; i < 9; i++)
		{
			char ch = text[i];
			if (ch == '\0')
			{
				break;
			}
			if (ch >= '0' && ch <= '9')
			{
				*dst++ = ch;
			}
			else if (ch != ',')
			{
				buf[0] = '\0';
				break;
			}
		}
		item.i64Price = atoi(buf);
		item.iNum = num;
		if (!IsLocalContent(S4_OFFLINE_SHOP))
		{
			m_pFinishEditDlg = NULL;
			return true;
		}
		WSendPacketSem983 packet((enumClientPacket)0xc7); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(index);
		packet.Encode2(1);
		packet.Encode1(valid);
		packet.EncodeBuffer(&item, sizeof(item));
		packet.Send(TO_GAME);
		CTask* task = AfxGetTask();
		MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
		task->SendMsgToMainActor(msg);
	}
	m_pFinishEditDlg = NULL;
	return true;
}

void COfflineShopEditDlg::OnToolTipInit(int param)
{
	m_pToolTip = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void COfflineShopEditDlg::OnToolTipOnwerDraw(int param)
{
	CIconToolTip::GetInstance()->Render();
}

void COfflineShopEditDlg::DelListMyItem(sTradeItem* item, unsigned long guid,
	unsigned char flag)
{
	if (item->dwTid == 0)
	{
		SetListMyItem();
	}
	else
	{
		FrTradeEditDlg::DelListMyItem(item);
	}
	SetTradeMyItem(NULL);
	SetControlsTradeMyItem(NULL, false);
	SetListTradeItem();
	if (m_pMyItemList)
	{
		m_pMyItemList->UnselectAllItem();
	}
}

bool COfflineShopEditDlg::OnOfflineTradeBuyDlgResult(int result, FrForm* form)
{
	if (result == 1)
	{
		FrOfflineTradeFinishEditDlg* dlg =
			CreateForm<FrOfflineTradeFinishEditDlg>(g_pFresh->GetManager(),
				this, "offlinetradefinisheditdlg");
		if (dlg)
		{
			dlg->Open(NULL, 3);
			dlg->SetFadeout(false);
			dlg->SetListTradeItem(&m_listTradeItem);
			dlg->SetTradeUserId();
			dlg->SetTradeTitle(m_title);
		}
	}
	return true;
}

IMPLEMENT_OBJECT(COfflineShopDlg, FrForm)

BEGIN_FRESH_MSGMAP(COfflineShopDlg, FrForm)

ON_FRESH_VI("tradedlg", FRCMD_INIT, COfflineShopDlg::OnTradeDlgInit)
ON_FRESH_VI("visitor_back", FRCMD_INIT, COfflineShopDlg::OnVisitorBackInit)
ON_FRESH_VV("visitor_back", FRCMD_OWNERDRAW,
	COfflineShopDlg::OnVisitorBackOwnerDraw)
ON_FRESH_VI("mypang", FRCMD_INIT, COfflineShopDlg::OnMyPangInit)
ON_FRESH_VV("mypang", FRCMD_OWNERDRAW, COfflineShopDlg::OnMyPangOwnerDraw)
ON_FRESH_VI("trade_item_list", FRCMD_INIT, COfflineShopDlg::OnTradeItemListInit)
ON_FRESH_VV("trade_item_list", FRCMD_LBUTTONDOWN,
	COfflineShopDlg::OnTradeItemListLClick)
ON_FRESH_VV("trade_item_list", FRCMD_RBUTTONDOWN,
	COfflineShopDlg::OnTradeItemListRClick)
ON_FRESH_VI("trade_item_list", FRCMD_OWNERDRAW,
	COfflineShopDlg::OnTradeItemListOwnerDraw)
ON_FRESH_VI("noitem01", FRCMD_INIT, COfflineShopDlg::OnNoItem01Init)
ON_FRESH_VI("noitem02", FRCMD_INIT, COfflineShopDlg::OnNoItem02Init)
ON_FRESH_VI("visitor", FRCMD_INIT, COfflineShopDlg::OnVisitorInit)
ON_FRESH_VI("userid", FRCMD_INIT, COfflineShopDlg::OnUserIdInit)
ON_FRESH_VI("trade_title", FRCMD_INIT, COfflineShopDlg::OnTitleInit)
ON_FRESH_VI("edit", FRCMD_INIT, COfflineShopDlg::OnEditBnInit)
ON_FRESH_VV("edit", FRCMD_LBUTTONUP, COfflineShopDlg::OnEditBnUp)
ON_FRESH_VI("close", FRCMD_INIT, COfflineShopDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, COfflineShopDlg::OnCloseBnUp)
ON_FRESH_VI("hide", FRCMD_INIT, COfflineShopDlg::OnHideBnInit)
ON_FRESH_VV("hide", FRCMD_LBUTTONUP, COfflineShopDlg::OnHideBnUp)
ON_FRESH_VI("exit", FRCMD_INIT, COfflineShopDlg::OnExitBnInit)
ON_FRESH_VV("exit", FRCMD_LBUTTONUP, COfflineShopDlg::OnExitBnUp)
ON_FRESH_VI("tooltip", FRCMD_INIT, COfflineShopDlg::OnToolTipInit)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW, COfflineShopDlg::OnToolTipOnwerDraw)

END_FRESH_MSGMAP()

COfflineShopDlg::COfflineShopDlg()
{
	m_pVisitor = NULL;
	m_closing = 0;
	m_pSelectedSlot = NULL;
	m_extendBtnN.pBitmap = g_pFresh->GetBitmap("rmr_extend_btn_n");
	m_extendBtnN.rect = WRect(96.0f, 66.0f, m_extendBtnN.pBitmap->Width(),
		m_extendBtnN.pBitmap->Height());
	m_extendBtnN.color = 0xffffffff;
	m_extendBtnO.pBitmap = g_pFresh->GetBitmap("rmr_extend_btn_o");
	m_extendBtnO.rect = WRect(96.0f, 66.0f, m_extendBtnO.pBitmap->Width(),
		m_extendBtnO.pBitmap->Height());
	m_extendBtnO.color = 0xffffffff;
	m_returnN.pBitmap = g_pFresh->GetBitmap("rmr_return_n");
	m_returnN.rect = WRect(144.0f, 66.0f, m_returnN.pBitmap->Width(),
		m_returnN.pBitmap->Height());
	m_returnN.color = 0xffffffff;
	m_returnO.pBitmap = g_pFresh->GetBitmap("rmr_return_o");
	m_returnO.rect = WRect(144.0f, 66.0f, m_returnO.pBitmap->Width(),
		m_returnO.pBitmap->Height());
	m_returnO.color = 0xffffffff;
	m_deleteBtnN.pBitmap = g_pFresh->GetBitmap("delete_btn_n");
	m_deleteBtnN.rect = WRect(144.0f, 66.0f, m_deleteBtnN.pBitmap->Width(),
		m_deleteBtnN.pBitmap->Height());
	m_deleteBtnN.color = 0xffffffff;
	m_deleteBtnO.pBitmap = g_pFresh->GetBitmap("delete_btn_o");
	m_deleteBtnO.rect = WRect(144.0f, 66.0f, m_deleteBtnO.pBitmap->Width(),
		m_deleteBtnO.pBitmap->Height());
	m_deleteBtnO.color = 0xffffffff;
	m_buyBtnN.pBitmap = g_pFresh->GetBitmap("buy_btn_n");
	m_buyBtnN.rect = WRect(144.0f, 66.0f, m_buyBtnN.pBitmap->Width(),
		m_buyBtnN.pBitmap->Height());
	m_buyBtnN.color = 0xffffffff;
	m_buyBtnO.pBitmap = g_pFresh->GetBitmap("buy_btn_o");
	m_buyBtnO.rect = WRect(144.0f, 66.0f, m_buyBtnO.pBitmap->Width(),
		m_buyBtnO.pBitmap->Height());
	m_buyBtnO.color = 0xffffffff;
	m_delete.pBitmap = g_pFresh->GetBitmap("delete");
	m_delete.rect = WRect(144.0f, 35.0f, m_delete.pBitmap->Width(),
		m_delete.pBitmap->Height());
	m_tradeState1.pBitmap = g_pFresh->GetBitmap("rmr_trade_state1");
	m_tradeState1.rect = WRect(70.0f, 35.0f, m_tradeState1.pBitmap->Width(),
		m_tradeState1.pBitmap->Height());
	m_tradeState2.pBitmap = g_pFresh->GetBitmap("rmr_trade_state2");
	m_tradeState2.rect = WRect(70.0f, 35.0f, m_tradeState2.pBitmap->Width(),
		m_tradeState2.pBitmap->Height());
	m_tradeState3.pBitmap = g_pFresh->GetBitmap("rmr_trade_state3");
	m_tradeState3.rect = WRect(70.0f, 35.0f, m_tradeState3.pBitmap->Width(),
		m_tradeState3.pBitmap->Height());
	m_itemUploadN.pBitmap = g_pFresh->GetBitmap("rmr_item_upload_n");
	m_itemUploadN.rect = WRect(144.0f, 66.0f, m_itemUploadN.pBitmap->Width(),
		m_itemUploadN.pBitmap->Height());
	m_itemUploadN.color = 0xffffffff;
	m_itemUploadO.pBitmap = g_pFresh->GetBitmap("rmr_item_upload_o");
	m_itemUploadO.rect = WRect(144.0f, 66.0f, m_itemUploadO.pBitmap->Width(),
		m_itemUploadO.pBitmap->Height());
	m_itemUploadO.color = 0xffffffff;
}

COfflineShopDlg::~COfflineShopDlg()
{
	if (m_closing == 0)
	{
		if (IsLocalContent(S4_OFFLINE_SHOP))
		{
		}
	}
}

void COfflineShopDlg::OnVisitorBackOwnerDraw()
{
}

void COfflineShopDlg::OnVisitorInit(int param)
{
	m_pVisitor = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (IsLocalContent(S4_OFFLINE_SHOP) && Doc()->GetTradeMode() == 1)
	{
		m_pVisitor->SetVisible(false);
	}
}

void COfflineShopDlg::OnCloseBnUp()
{
	m_closing = 1;
	if (IsLocalContent(S4_OFFLINE_SHOP))
	{
		CTask* task = AfxGetTask();
		MsgObject msg(NULL, 0x81, 0, 0, 0, 0, 0); // 983 006567e0 (645: 0x22f)
		task->SendMsgToMainActor(msg);
	}
	FrForm::OnOK();
}

void COfflineShopDlg::OnVisitorBackInit(int param)
{
	m_pVisitorBack = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pVisitorBack)
	{
		m_pVisitorBack->SetVisible(false);
	}
}

void COfflineShopDlg::OnTradeItemListOwnerDraw(int param)
{
	FrTradeDlg::OnTradeItemListOwnerDraw(param);
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	FrListItem* item = (FrListItem*)param;
	if (gdi && item && item->pData)
	{
		const Bitmap* slotBmp = g_pFresh->GetBitmap("rmr_item_slot");
		sStallSlot* slot = (sStallSlot*)item->pData;
		gdi->SetTextStyle(0);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		if (slot->IsValid == 0)
		{
			if (slotBmp)
			{
				gdi->DrawTexture(slotBmp,
					WRect(item->pos.x, item->pos.y, slotBmp->Width(),
						slotBmp->Height()),
					0xffffffff, 0);
			}
			gdi->Print(WPoint(item->pos.x + 182.0f, item->pos.y + 7.0f), 2,
				"\xbb\xf3\xc1\xa1 \xbe\xc6\xc0\xcc\xc5\xdb \xbd\xbd\xb7\xd4 \xb1\xb8\xb8\xc5");
			if (item->underCursor &&
				(m_buyBtnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_buyBtnO.pBitmap,
					m_buyBtnO.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
					0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x1b);
			}
			else
			{
				WRect rect = m_buyBtnN.rect + item->pos;
				gdi->DrawTexture(m_buyBtnN.pBitmap, rect, 0xffffffff, 0);
			}
			return;
		}
		__int64 remain = 0;
		if (!slot->IsPermanence)
		{
			FILETIME slotTime;
			SystemTimeToFileTime(&slot->time, &slotTime);
			FILETIME serverTime;
			SystemTimeToFileTime(&Doc()->GetServerTime(), &serverTime);
			__int64 diff = ((unsigned __int64)slotTime.dwHighDateTime -
							   (unsigned __int64)serverTime.dwHighDateTime) *
					0x100000000 -
				serverTime.dwLowDateTime + slotTime.dwLowDateTime;
			remain = diff / 60 / 60 / 10000000;
			gdi->DrawTexture(m_tradeState3.pBitmap,
				m_tradeState3.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
				0);
			if (remain >= 0 && diff >= 0)
			{
				gdi->SetTextColor(0xffffffff, 0xffffffff);
				if (remain / 24 >= 1)
				{
					if (remain % 24 != 0)
					{
						WPoint pos(m_tradeState3.rect.x + item->pos.x,
							m_tradeState3.rect.y + item->pos.y);
						pos.x += 55.0f;
						pos.y += 6.0f;
						gdi->Print(pos, 2, "%d\xc0\xcf", remain / 24 + 1);
					}
					else
					{
						WPoint pos(m_tradeState3.rect.x + item->pos.x,
							m_tradeState3.rect.y + item->pos.y);
						pos.x += 55.0f;
						pos.y += 6.0f;
						gdi->Print(pos, 2, "%d\xc0\xcf", remain / 24);
					}
				}
				else
				{
					WPoint pos(m_tradeState3.rect.x + item->pos.x,
						m_tradeState3.rect.y + item->pos.y);
					pos.x += 55.0f;
					pos.y += 6.0f;
					gdi->Print(pos, 2, "%d\xbd\xc3", remain);
				}
			}
			else
			{
				gdi->SetTextColor(0xffffffff, 0xffffffff);
				WPoint pos(m_tradeState3.rect.x + item->pos.x,
					m_tradeState3.rect.y + item->pos.y);
				pos.x += 55.0f;
				pos.y += 6.0f;
				gdi->Print(pos, 2, "\xb1\xe2\xb0\xa3\xb8\xb8\xb7\xe1");
			}
			gdi->SetTextColor(0xff000000, 0xffffffff);
			if (item->underCursor &&
				(m_extendBtnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_extendBtnO.pBitmap,
					m_extendBtnO.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x53);
			}
			else
			{
				gdi->DrawTexture(m_extendBtnN.pBitmap,
					m_extendBtnN.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
			}
		}
		if (slot->itemData.dwTid == 0)
		{
			if (slotBmp)
			{
				gdi->DrawTexture(slotBmp,
					WRect(item->pos.x, item->pos.y, slotBmp->Width(),
						slotBmp->Height()),
					0xffffffff, 0);
			}
			gdi->Print(WPoint(item->pos.x + 182.0f, item->pos.y + 7.0f), 2,
				"\xbb\xf3\xc1\xa1 \xbe\xc6\xc0\xcc\xc5\xdb \xbd\xbd\xb7\xd4");
			if (item->underCursor &&
				(m_itemUploadN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_itemUploadO.pBitmap,
					m_itemUploadO.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x54);
			}
			else
			{
				gdi->DrawTexture(m_itemUploadN.pBitmap,
					m_itemUploadN.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
			}
			return;
		}
		if (!slot->IsPermanence && remain < 0)
		{
			if (item->underCursor &&
				(m_returnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_returnO.pBitmap,
					m_returnO.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
					0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x55);
			}
			else
			{
				gdi->DrawTexture(m_returnN.pBitmap,
					m_returnN.rect + item->pos + WPoint(0.0f, 0.0f), 0xffffffff,
					0);
			}
		}
		else
		{
			if (item->underCursor &&
				(m_deleteBtnN.rect + item->pos).IsInRect(mouse))
			{
				gdi->DrawTexture(m_deleteBtnO.pBitmap,
					m_deleteBtnO.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)0x52);
			}
			else
			{
				gdi->DrawTexture(m_deleteBtnN.pBitmap,
					m_deleteBtnN.rect + item->pos + WPoint(0.0f, 0.0f),
					0xffffffff, 0);
			}
		}
	}
}

void COfflineShopDlg::OnToolTipInit(int param)
{
	m_pToolTip = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void COfflineShopDlg::OnToolTipOnwerDraw(int param)
{
	CIconToolTip::GetInstance()->Render();
}

void COfflineShopDlg::OnTradeItemListLClick()
{
	FrListItem* item = m_pTradeItemList->GetItemUnderCursor();
	if (item == NULL || item->pData == NULL)
	{
		return;
	}
	FrWndManager* manager = g_pFresh->GetManager();
	unsigned char valid = ((sStallSlot*)item->pData)->IsValid;
	const WPoint& mouse = manager->GetMousePos();
	if (valid == 0)
	{
		WRect rect(item->pos.x + m_buyBtnN.rect.x,
			item->pos.y + m_buyBtnN.rect.y, m_buyBtnN.rect.w, m_buyBtnN.rect.h);
		if (rect.IsInRect(mouse) ||
			(m_buyBtnO.rect + item->pos).IsInRect(mouse))
		{
			CBuyingSlotDlg* dlg =
				CreateForm<CBuyingSlotDlg>(manager, this, "buyslotdlg");
			dlg->SetSlotNum(item->idx + 1);
			dlg->Open(NULL, 1);
			return;
		}
	}
	if (valid == 1)
	{
		if (((sStallSlot*)item->pData)->itemData.dwTid == 0)
		{
			if ((m_itemUploadN.rect + item->pos).IsInRect(mouse) ||
				(m_itemUploadO.rect + item->pos).IsInRect(mouse))
			{
				m_pSelectedSlot = item;
				FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify_yesno");
				if (form == NULL)
				{
					return;
				}
				form->Open((FRESH_PFN_RESULT)&COfflineShopDlg::
							   OnOfflineShopEditDlgCreateResult,
					0xc0);
				form->SetFadeout(false);
				form->SetMessage(
					"\xbe\xc6\xc0\xcc\xc5\xdb \xb5\xee\xb7\xcf\xc0\xbb \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?",
					false);
				return;
			}
		}
	}
	if (valid == 1)
	{
		if ((m_extendBtnN.rect + item->pos).IsInRect(mouse) ||
			(m_extendBtnO.rect + item->pos).IsInRect(mouse))
		{
			CBuyingSlotDlg* dlg =
				CreateForm<CBuyingSlotDlg>(manager, this, "buyslotdlg");
			dlg->SetSlotNum(item->idx + 1);
			dlg->Open(NULL, 1);
			return;
		}
	}
	if (valid == 1)
	{
		if ((m_returnN.rect + item->pos).IsInRect(mouse) ||
			(m_returnO.rect + item->pos).IsInRect(mouse) ||
			(m_deleteBtnN.rect + item->pos).IsInRect(mouse) ||
			(m_deleteBtnO.rect + item->pos).IsInRect(mouse))
		{
			if (!IsLocalContent(S4_OFFLINE_SHOP))
			{
				return;
			}
			WSendPacketSem983 packet((enumClientPacket)0xc8); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(Doc()->GetStallItem()->stallKey);
			packet.Encode2(1);
			packet.Encode1(item->idx + 1);
			packet.Send(TO_GAME);
			MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
			AfxGetTask()->SendMsgToMainActor(msg);
			return;
		}
	}
	item->selected = !item->selected;
}

bool COfflineShopDlg::OnOfflineShopEditDlgCreateResult(int result, FrForm* form)
{
	if (result == 1)
	{
		if (m_pSelectedSlot && m_pSelectedSlot->pData)
		{
			sStallSlot* slot = (sStallSlot*)m_pSelectedSlot->pData;
			m_closing = 1;
			COfflineShopEditDlg* dlg = CreateForm<COfflineShopEditDlg>(
				g_pFresh->GetManager(), this, "tradeeditdlg");
			if (dlg)
			{
				dlg->Open(NULL, 0xc0);
				dlg->SetSelectData(slot->stallKey, slot->Idx);
				dlg->SetFadeout(false);
				m_pSelectedSlot = NULL;
				OnFreshOkay();
			}
		}
		else
		{
			FrForm* notify =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
			notify->SetMessage(
				"\xbc\xb1\xc5\xc3\xc7\xd1 \xbd\xbd\xb7\xd4\xc0\xc7 \xc1\xa4\xba\xb8\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xc6\xed\xc1\xfd \xbd\xc7\xc6\xd0\xc0\xd4\xb4\xcf\xb4\xd9.",
				false);
			notify->Open(NULL, 3);
		}
	}
	else
	{
		m_pSelectedSlot = NULL;
	}
	return true;
}

void COfflineShopDlg::OnProc(const float dt)
{
	FrForm::OnProc(dt);
	CIconToolTip::GetInstance()->ClearToolTip();
}

void COfflineShopDlg::SetListTradeItem(bool clear)
{
	FrTradeDlg::SetListTradeItem(false);
	m_pTradeItemList->UseRightButton(false);
	sStall* stall = Doc()->GetStallItem();
	m_pTradeItemList->ClearItem();
	for (int i = 0; i < stall->maxSlotNum; i++)
	{
		m_pTradeItemList->AddItem(&stall->slotList[i]);
	}
}

IMPLEMENT_OBJECT(COfflineShopVisitorDlg, FrForm)

BEGIN_FRESH_MSGMAP(COfflineShopVisitorDlg, FrForm)

ON_FRESH_VI("tradedlg", FRCMD_INIT, COfflineShopVisitorDlg::OnTradeDlgInit)
ON_FRESH_VI("visitor_back", FRCMD_INIT,
	COfflineShopVisitorDlg::OnVisitorBackInit)
ON_FRESH_VV("visitor_back", FRCMD_OWNERDRAW,
	COfflineShopVisitorDlg::OnVisitorBackOwnerDraw)
ON_FRESH_VI("mypang", FRCMD_INIT, COfflineShopVisitorDlg::OnMyPangInit)
ON_FRESH_VV("mypang", FRCMD_OWNERDRAW,
	COfflineShopVisitorDlg::OnMyPangOwnerDraw)
ON_FRESH_VI("trade_item_list", FRCMD_INIT,
	COfflineShopVisitorDlg::OnTradeItemListInit)
ON_FRESH_VV("trade_item_list", FRCMD_LBUTTONDOWN,
	COfflineShopVisitorDlg::OnTradeItemListLClick)
ON_FRESH_VV("trade_item_list", FRCMD_RBUTTONDOWN,
	COfflineShopVisitorDlg::OnTradeItemListRClick)
ON_FRESH_VI("trade_item_list", FRCMD_OWNERDRAW,
	COfflineShopVisitorDlg::OnTradeItemListOwnerDraw)
ON_FRESH_VI("noitem01", FRCMD_INIT, COfflineShopVisitorDlg::OnNoItem01Init)
ON_FRESH_VI("noitem02", FRCMD_INIT, COfflineShopVisitorDlg::OnNoItem02Init)
ON_FRESH_VI("visitor", FRCMD_INIT, COfflineShopVisitorDlg::OnVisitorInit)
ON_FRESH_VI("userid", FRCMD_INIT, COfflineShopVisitorDlg::OnUserIdInit)
ON_FRESH_VI("trade_title", FRCMD_INIT, COfflineShopVisitorDlg::OnTitleInit)
ON_FRESH_VI("edit", FRCMD_INIT, COfflineShopVisitorDlg::OnEditBnInit)
ON_FRESH_VV("edit", FRCMD_LBUTTONUP, COfflineShopVisitorDlg::OnEditBnUp)
ON_FRESH_VI("close", FRCMD_INIT, COfflineShopVisitorDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, COfflineShopVisitorDlg::OnCloseBnUp)
ON_FRESH_VI("hide", FRCMD_INIT, COfflineShopVisitorDlg::OnHideBnInit)
ON_FRESH_VV("hide", FRCMD_LBUTTONUP, COfflineShopVisitorDlg::OnHideBnUp)
ON_FRESH_VI("exit", FRCMD_INIT, COfflineShopVisitorDlg::OnExitBnInit)
ON_FRESH_VV("exit", FRCMD_LBUTTONUP, COfflineShopVisitorDlg::OnExitBnUp)
ON_FRESH_VI("tooltip", FRCMD_INIT, COfflineShopVisitorDlg::OnToolTipInit)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW,
	COfflineShopVisitorDlg::OnToolTipOnwerDraw)

END_FRESH_MSGMAP()

COfflineShopVisitorDlg::COfflineShopVisitorDlg()
{
	m_pVisitor = NULL;
	m_buyBtnN.pBitmap = g_pFresh->GetBitmap("buy_btn_n");
	m_buyBtnN.rect = WRect(144.0f, 66.0f, m_buyBtnN.pBitmap->Width(),
		m_buyBtnN.pBitmap->Height());
	m_buyBtnN.color = 0xffffffff;
	m_buyBtnO.pBitmap = g_pFresh->GetBitmap("buy_btn_o");
	m_buyBtnO.rect = WRect(144.0f, 66.0f, m_buyBtnO.pBitmap->Width(),
		m_buyBtnO.pBitmap->Height());
	m_buyBtnO.color = 0xffffffff;
	m_tradeState1.pBitmap = g_pFresh->GetBitmap("rmr_trade_state1");
	m_tradeState1.rect = WRect(70.0f, 35.0f, m_tradeState1.pBitmap->Width(),
		m_tradeState1.pBitmap->Height());
	m_tradeState2.pBitmap = g_pFresh->GetBitmap("rmr_trade_state2");
	m_tradeState2.rect = WRect(70.0f, 35.0f, m_tradeState2.pBitmap->Width(),
		m_tradeState2.pBitmap->Height());
	m_tradeState3.pBitmap = g_pFresh->GetBitmap("rmr_trade_state3");
	m_tradeState3.rect = WRect(70.0f, 35.0f, m_tradeState3.pBitmap->Width(),
		m_tradeState3.pBitmap->Height());
}

COfflineShopVisitorDlg::~COfflineShopVisitorDlg()
{
}

void COfflineShopVisitorDlg::OnEditBnUp()
{
}

void COfflineShopVisitorDlg::OnVisitorBackOwnerDraw()
{
}

void COfflineShopVisitorDlg::OnCloseBnUp()
{
	FrTradeDlg::OnOK();
}

void COfflineShopVisitorDlg::OnVisitorInit(int param)
{
	m_pVisitor = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (IsLocalContent(S4_OFFLINE_SHOP) && Doc()->GetTradeMode() == 1)
	{
		m_pVisitor->SetVisible(false);
	}
}

void COfflineShopVisitorDlg::OnVisitorBackInit(int param)
{
	m_pVisitorBack = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pVisitorBack)
	{
		m_pVisitorBack->SetVisible(false);
	}
}

void COfflineShopVisitorDlg::OnTradeItemListOwnerDraw(int param)
{
	FrTradeDlg::OnTradeItemListOwnerDraw(param);
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	FrListItem* item = (FrListItem*)param;
	if (gdi && item)
	{
		sStallSlot* slot = (sStallSlot*)item->pData;
		if (slot)
		{
			g_pFresh->GetBitmap("rmr_item_slot");
			if (slot->IsValid == 1 && slot->itemData.dwTid != 0)
			{
				FILETIME slotTime;
				SystemTimeToFileTime(&slot->time, &slotTime);
				FILETIME serverTime;
				SystemTimeToFileTime(&Doc()->GetServerTime(), &serverTime);
				__int64 remain =
					(((unsigned __int64)slotTime.dwHighDateTime -
						 (unsigned __int64)serverTime.dwHighDateTime) *
							0x100000000 -
						serverTime.dwLowDateTime + slotTime.dwLowDateTime) /
					60 / 60 / 10000000;
				if (remain >= 0)
				{
					if (!slot->IsPermanence)
					{
						gdi->DrawTexture(m_tradeState3.pBitmap,
							m_tradeState3.rect + item->pos + WPoint(0.0f, 0.0f),
							0xffffffff, 0);
						gdi->SetTextColor(0xffffffff, 0xffffffff);
						if (remain / 24 > 1)
						{
							if (remain % 24 != 0)
							{
								WPoint pos(m_tradeState3.rect.x + item->pos.x,
									m_tradeState3.rect.y + item->pos.y);
								pos.x += 55.0f;
								pos.y += 6.0f;
								gdi->Print(pos, 2, "%d\xc0\xcf",
									remain / 24 + 1);
							}
							else
							{
								WPoint pos(m_tradeState3.rect.x + item->pos.x,
									m_tradeState3.rect.y + item->pos.y);
								pos.x += 55.0f;
								pos.y += 6.0f;
								gdi->Print(pos, 2, "%d\xc0\xcf", remain / 24);
							}
						}
						else
						{
							WPoint pos(m_tradeState3.rect.x + item->pos.x,
								m_tradeState3.rect.y + item->pos.y);
							pos.x += 55.0f;
							pos.y += 6.0f;
							gdi->Print(pos, 2, "%d\xbd\xc3", remain);
						}
					}
					if (item->underCursor &&
						(m_buyBtnN.rect + item->pos).IsInRect(mouse))
					{
						gdi->DrawTexture(m_buyBtnO.pBitmap,
							m_buyBtnO.rect + item->pos + WPoint(0.0f, 0.0f),
							0xffffffff, 0);
						CIconToolTip::GetInstance()->SetToolTip(
							(eBarButton)0x1b);
					}
					else
					{
						gdi->DrawTexture(m_buyBtnN.pBitmap,
							m_buyBtnN.rect + item->pos + WPoint(0.0f, 0.0f),
							0xffffffff, 0);
					}
				}
				else
				{
					gdi->DrawTexture(m_tradeState3.pBitmap,
						m_tradeState3.rect + item->pos + WPoint(0.0f, 0.0f),
						0xffffffff, 0);
					gdi->SetTextColor(0xffffffff, 0xffffffff);
					WPoint pos(m_tradeState3.rect.x + item->pos.x,
						m_tradeState3.rect.y + item->pos.y);
					pos.x += 55.0f;
					pos.y += 6.0f;
					gdi->Print(pos, 2, "\xb1\xb8\xb8\xc5 \xba\xd2\xb0\xa1");
				}
			}
		}
	}
}

void COfflineShopVisitorDlg::OnTradeItemListLClick()
{
	FrListItem* item = m_pTradeItemList->GetItemUnderCursor();
	if (item == NULL)
	{
		return;
	}
	sStallSlot* slot = (sStallSlot*)item->pData;
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	FILETIME slotTime;
	SystemTimeToFileTime(&slot->time, &slotTime);
	FILETIME serverTime;
	SystemTimeToFileTime(&Doc()->GetServerTime(), &serverTime);
	if (slot->IsValid == 1)
	{
		__int64 remain =
			(((unsigned __int64)slotTime.dwHighDateTime -
				 (unsigned __int64)serverTime.dwHighDateTime) *
					0x100000000 -
				serverTime.dwLowDateTime + slotTime.dwLowDateTime) /
			60 / 60 / 10000000;
		if (remain >= 0 || slot->IsPermanence)
		{
			WRect rect(item->pos.x + m_buyBtnN.rect.x,
				item->pos.y + m_buyBtnN.rect.y, m_buyBtnN.rect.w,
				m_buyBtnN.rect.h);
			bool hit = rect.IsInRect(mouse);
			if (!hit)
			{
				WRect rectOver(item->pos.x + m_buyBtnO.rect.x,
					item->pos.y + m_buyBtnO.rect.y, m_buyBtnO.rect.w,
					m_buyBtnO.rect.h);
				hit = rectOver.IsInRect(mouse);
			}
			if (hit)
			{
				COfflineBuyDlg* dlg = CreateForm<COfflineBuyDlg>(
					g_pFresh->GetManager(), this, "tradebuydlg");
				if (dlg == NULL)
				{
					return;
				}
				dlg->Open((FRESH_PFN_RESULT)&COfflineShopVisitorDlg::
							  OnTradeBuyDlgResult,
					0x40);
				dlg->SetFadeout(false);
				dlg->SetBuyItem(&slot->itemData);
				dlg->SetBuyitemData(*slot);
				dlg->SetControlsBuyItem(&slot->itemData);
				return;
			}
		}
	}
	item->selected = !item->selected;
}

void COfflineShopVisitorDlg::OnToolTipInit(int param)
{
	m_pToolTip = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void COfflineShopVisitorDlg::OnToolTipOnwerDraw(int param)
{
	CIconToolTip::GetInstance()->Render();
}

bool COfflineShopVisitorDlg::OnOfflineShopEditDlgCreateResult(int result,
	FrForm* form)
{
	return true;
}

void COfflineShopVisitorDlg::OnProc(const float dt)
{
	FrForm::OnProc(dt);
	CIconToolTip::GetInstance()->ClearToolTip();
}

void COfflineShopVisitorDlg::SetListTradeItem(bool clear)
{
	FrTradeDlg::SetListTradeItem(false);
	sStall* stall = Doc()->GetStallItem();
	m_pTradeItemList->UseRightButton(false);
	m_pTradeItemList->ClearItem();
	for (int i = 0; i < stall->maxSlotNum; i++)
	{
		if (stall->slotList[i].IsValid == 1 &&
			stall->slotList[i].itemData.dwTid != 0)
		{
			m_pTradeItemList->AddItem(&stall->slotList[i]);
		}
	}
}

IMPLEMENT_OBJECT(COfflineBuyDlg, FrForm)

BEGIN_FRESH_MSGMAP(COfflineBuyDlg, FrForm)

ON_FRESH_VI("tradebuydlg", FRCMD_INIT, COfflineBuyDlg::OnTradeBuyDlgInit)
ON_FRESH_VI("ok", FRCMD_INIT, COfflineBuyDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, COfflineBuyDlg::OnOkBnUp)
ON_FRESH_VI("buyitem_icon", FRCMD_INIT, COfflineBuyDlg::OnBuyItemIconInit)
ON_FRESH_VV("buyitem_icon", FRCMD_OWNERDRAW,
	COfflineBuyDlg::OnBuyItemIconOwnerDraw)
ON_FRESH_VI("buyitem_name", FRCMD_INIT, COfflineBuyDlg::OnBuyItemNameInit)
ON_FRESH_VI("buyitem_ability", FRCMD_INIT, COfflineBuyDlg::OnBuyItemAbilityInit)
ON_FRESH_VI("my_pang", FRCMD_INIT, COfflineBuyDlg::OnMyPangInit)
ON_FRESH_VI("price", FRCMD_INIT, COfflineBuyDlg::OnPriceInit)
ON_FRESH_VI("pang_left", FRCMD_INIT, COfflineBuyDlg::OnPangLeftInit)
ON_FRESH_VI("num", FRCMD_INIT, COfflineBuyDlg::OnNumberInit)
ON_FRESH_VI("num_next", FRCMD_INIT, COfflineBuyDlg::OnNumberNextBnInit)
ON_FRESH_VV("num_next", FRCMD_LBUTTONUP, COfflineBuyDlg::OnNumberNextBnUp)
ON_FRESH_VI("num_prev", FRCMD_INIT, COfflineBuyDlg::OnNumberPrevBnInit)
ON_FRESH_VV("num_prev", FRCMD_LBUTTONUP, COfflineBuyDlg::OnNumberPrevBnUp)

END_FRESH_MSGMAP()

COfflineBuyDlg::COfflineBuyDlg()
{
	// HACK: the original ctor is compiled one wave later (a PCH inline it
	// references is emitted later there); a dead reference keeps the order.
	if (0)
		Doc();
}

COfflineBuyDlg::~COfflineBuyDlg()
{
}

void COfflineBuyDlg::OnOkBnUp()
{
	bool enough = true;
	m_count = 0;
	if (!IsLocalContent(S4_OFFLINE_SHOP) || Doc()->GetTradeMode() != 1)
	{
		FrTradeBuyDlg::OnOkBnUp();
		return;
	}
	std::list<sStall>& stallList = Doc()->m_listStallItem;
	std::list<sStall>::iterator it;
	for (it = stallList.begin(); it != stallList.end(); it++)
	{
		if ((*it).stallKey == m_slot.stallKey)
		{
			break;
		}
	}
	if (it != stallList.end())
	{
		m_count = atoi(m_pNumber->GetLine(1, false));
		if (m_count >= m_pBuyItem->iNum)
		{
			m_count = m_pBuyItem->iNum;
		}
		if (m_count <= 1)
		{
			m_count = 1;
		}
		int num = m_buyItem.iNum;
		if (m_buyItem.i64Price * num > Doc()->m_myInfo.stat.i64Pang)
		{
			enough = false;
		}
		if (!Doc()->ConfirmInsertItem(m_buyItem.dwTid, num))
		{
			FrForm* notify =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify");
			WRect rect = notify->GetRect();
			notify->SetRect(WRect(rect.x, rect.y, rect.w, rect.h));
			notify->SetTimeLimit(12.0f);
			notify->SetMessage(
				K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xba\xb8\xc0\xaf\xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb1\xb8\xc0\xd4 \xb6\xc7\xb4\xc2 \xbf\xc5\xb1\xe6 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			notify->Open(NULL, 0);
		}
		else if (enough)
		{
			FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify_okcancel");
			form->SetMessage(
				K2L_Compatibility("\\c0xffff0000\\c\xc1\xa4\xb8\xbb \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
				false);
			form->Open((FRESH_PFN_RESULT)&COfflineBuyDlg::OnConfirmResult, 3);
			return;
		}
	}
	FrForm::OnFreshCancel();
}

bool COfflineBuyDlg::OnConfirmResult(int result, FrForm* form)
{
	// HACK: the original emits the (unreferenced) inline sBuyItem ctor of the
	// PCH; this dead local keeps its COMDAT.
	if (0)
	{
		sBuyItem item;
	}
	if (result == 1)
	{
		if (form)
		{
			if (IsLocalContent(S4_OFFLINE_SHOP))
			{
				WSendPacketSem983 packet((enumClientPacket)0xc6); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
				packet.Encode4(Doc()->m_replayTargetUID);
				packet.Encode4(m_slot.stallKey);
				packet.Encode1(m_slot.Idx);
				packet.Encode2(m_count);
				packet.Encode8(m_slot.itemData.i64Price);
				packet.Send(TO_GAME);
				CTask* task = AfxGetTask();
				MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
				task->SendMsgToMainActor(msg);
				m_count = 0;
			}
			OnFreshOkay();
		}
	}
	return true;
}
