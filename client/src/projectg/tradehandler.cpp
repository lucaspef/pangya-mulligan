#include "minatl.h"
#include <iterator> // VC10: std::back_inserter
#include "../shared/globalnetworkdefine.h"
#include "../shared/localize.h"
#include "tradedlg.h"
#include "gatewaydoc.h"
#include "messagemanager.h"
#include "chatmsg.h"
#include "fresh.h"
#include "frdesktop.h"
#include "gatewayactor.h"
#include "uccmanager.h"

// D807: o 645 lia o manager cru em g_pFresh+0x3c; no 983 e no nosso Fresh ele fica em +0x40 (983 SetTradeResultCode
// 008f6638 mov edx,[ecx+0x40]; offsetof(Fresh, manager) = 0x40 medido) -> +0x3c entregava lixo ao CreateForm.
#define FRESH_MANAGER (g_pFresh->GetManager())

extern Fresh* g_pFresh;

// The original never calls DynamicCast here: __rtti_obj is only written when
// the window is missing, so the cast is spelled out instead of DYNAMIC_CAST.
#define TRADE_CAST(type, wnd) \
	(!(wnd) ? (type*)(__rtti_obj = NULL) : (type*)(wnd))

bool IsUccClothes(unsigned long tid);

namespace _gateway
{
#pragma pack(push, 1)
	struct sTradeBuyBase
	{
		sTradeItem tradeItem;
		sItemInfo itemInfo;
		unsigned char countingType;
	};
#pragma pack(pop)

	struct sTradeBuyResult
	{
		std::vector<sTradeBuyBase> list;
		int result;
		__int64 pang;
	};

	struct sTradeSellResult
	{
		std::vector<sTradeItem> list;
		int result;
		__int64 pang;
	};

	struct sTradeMinusResult
	{
		std::vector<sTradeItem> list;
		std::string nick;
		int state;
		unsigned long uid;
	};

	struct sTransformHelper
	{
		const sTradeItem operator()(const sTradeBuyBase& base);
	};

	void UpdateBuyItem(const sTradeBuyBase& base);
	void UpdateSellItem(const sTradeItem& item);
	void SetTradeResult(int type, int code, std::vector<sTradeItem>* pList);
}

using namespace _gateway;

void _gateway::UpdateBuyItem(const sTradeBuyBase& base)
{
	if (base.countingType >= 2 && base.countingType <= 3)
	{
		switch (base.itemInfo.tid >> 26)
		{
		case 2:
			if (IsLocalContent(S4_UCC) && IsUccClothes(base.itemInfo.tid))
			{
				Doc()->m_uccItemList.push_back(base.itemInfo);
				UccManager()->AddClothes(1, base.itemInfo.guid,
					base.itemInfo.tid, base.itemInfo.UccIndex, false, false);
			}
			else
			{
				Doc()->AddMyPartsList(base.itemInfo);
			}
			break;
		case 4:
			Doc()->m_clubSetMap.insert(
				std::map<unsigned int, sItemInfo>::value_type(
					base.itemInfo.guid, base.itemInfo));
			break;
		}
	}
	else
	{
		_systemmsg::CMessageManager::Instance()->NotifyNormalMessage(
			"Invalid Counting Type!");
	}
}

void _gateway::UpdateSellItem(const sTradeItem& item)
{
	switch (item.dwTid >> 26)
	{
	case 2:
		if (IsLocalContent(S4_UCC) && IsUccClothes(item.dwTid))
		{
			std::list<sItemInfo>::iterator it;
			for (it = Doc()->m_uccItemList.begin();
				it != Doc()->m_uccItemList.end(); ++it)
			{
				if ((*it).guid == item.dwGuid && (*it).tid == item.dwTid &&
					strcmpi((*it).UccIndex, item.UccIndex) == 0 &&
					(*it).Seq == item.Seq)
				{
					Doc()->m_uccItemList.erase(it);
					if (IsLocalContent(S4_CARD_SYSTEM_CHANGE))
						Doc()->DeleteCardToPartsAttach(item.dwGuid);
					break;
				}
			}
		}
		else
		{
			std::list<sItemInfo>::iterator it;
			for (it = Doc()->m_partsList.begin();
				it != Doc()->m_partsList.end(); ++it)
			{
				if ((*it).guid == item.dwGuid)
				{
					if ((*it).tid == item.dwTid)
					{
						Doc()->m_partsList.erase(it);
						if (IsLocalContent(S4_CARD_SYSTEM_CHANGE))
							Doc()->DeleteCardToPartsAttach(item.dwGuid);
					}
					break;
				}
			}
		}
		break;
	case 4:
	{
		std::map<unsigned int, sItemInfo>::iterator it;
		for (it = Doc()->m_clubSetMap.begin(); it != Doc()->m_clubSetMap.end();
			++it)
		{
			if ((*it).second.guid == item.dwGuid)
			{
				if ((*it).second.tid == item.dwTid)
					Doc()->m_clubSetMap.erase(it);
				break;
			}
		}
	}
	break;
	}
}

void _gateway::SetTradeResult(int type, int code,
	std::vector<sTradeItem>* pList)
{
	if (type == 1)
	{
		if (code >= 470 && code <= 471)
		{
			FrTradeResultDlg* pDlg = CreateForm<FrTradeResultDlg>(FRESH_MANAGER,
				NULL, "traderesultdlg");
			if (pDlg && pList)
			{
				pDlg->Open(NULL, 1);
				pDlg->SetFadeout(false);
				// 983 (s84): FrTradeResultDlg sem lista (0x1e0); SetResultItemList do 645 nao existe
			}
		}
		return;
	}

	switch (type)
	{
	case 23:
	{
		FrTradeLevelLimitDlg* pDlg = CreateForm<FrTradeLevelLimitDlg>(
			FRESH_MANAGER, NULL, "tradelevellimitdlg");
		if (pDlg)
			pDlg->Open(NULL, 0x40);
	}
		return;
	case 15:
	{
		FrWnd* pWnd =
			FRESH_MANAGER->GetDesktop()->FindChildByName("tradewarningdlg");
		if (TRADE_CAST(FrTradeWarningDlg, pWnd) == NULL)
		{
			FrTradeWarningDlg* pDlg = CreateForm<FrTradeWarningDlg>(
				FRESH_MANAGER, NULL, "tradewarningdlg");
			if (pDlg)
			{
				pDlg->Open(NULL, 1);
				pDlg->SetWarningMessageType(0x10, 0); // 983: o aviso de loja sob sancao e o 0x10 (0xf no 645); chamada nao conferida no 983
				pDlg->SetFadeout(false);
			}
		}
	}
	break;
	}

	FrWnd* pWnd =
		FRESH_MANAGER->GetDesktop()->FindChildByName("traderesultdlg");
	FrTradeResultDlg* pResultDlg = TRADE_CAST(FrTradeResultDlg, pWnd);
	if (pResultDlg)
	{
		switch (type)
		{
		case 3:
			CChatMsg::Instance()->AddChatMsg(
				K2L_Compatibility("\xbb\xf3\xc1\xa1\xc0\xcc \xb4\xdd\xc7\xfb\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xc0\xbd \xb1\xe2\xc8\xb8\xbf\xa1 \xc0\xcc\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."),
				0xffff, false, false);
			break;
		case 4:
			CChatMsg::Instance()->AddChatMsg(
				K2L_Compatibility("\xbb\xf3\xc1\xa1\xc0\xcc \xc6\xed\xc1\xfd\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. \xb0\xb3\xbc\xb3 \xc8\xc4 \xc0\xcc\xbf\xeb\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4."),
				0xffff, false, false);
			break;
		}
	}
	else
	{
		pWnd = FRESH_MANAGER->GetDesktop()->FindChildByName("tradewarningdlg");
		if (TRADE_CAST(FrTradeWarningDlg, pWnd) == NULL)
		{
			FrTradeWarningDlg* pDlg = CreateForm<FrTradeWarningDlg>(
				FRESH_MANAGER, NULL, "tradewarningdlg");
			if (pDlg)
			{
				pDlg->Open(NULL, 1);
				pDlg->SetWarningMessageForPackage(type);
				pDlg->SetFadeout(false);
			}
		}
	}
}

CTradeHandler::CTradeHandler()
{
}

CTradeHandler::~CTradeHandler()
{
}

bool CTradeHandler::Initialize()
{
	AddFunctionType(0, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CTradeHandler::*)())&CTradeHandler::OnNotifyTradeErrorCode);
	AddFunctionType(1, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CTradeHandler::*)())&CTradeHandler::OnUpdateTradeBuyItem);
	AddFunctionType(2, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CTradeHandler::*)())&CTradeHandler::OnUpdateTradeSellItem);
	AddFunctionType(3, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CTradeHandler::*)())&CTradeHandler::OnUpdateTradeMinusItem);
	AddFunctionType(4, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CTradeHandler::OnSetEnablePackageSale);
	AddFunctionType(5, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CTradeHandler::OnSetDisablePackageSale);
	AddFunctionType(6, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CTradeHandler::*)())&CTradeHandler::OnGetPackageSale);

	return true;
}

bool CTradeHandler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec(this, (int)&msg);
}

void CTradeHandler::OnNotifyTradeErrorCode(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;

	SetTradeResult(pMsg->param2, 0, NULL);
}

const sTradeItem sTransformHelper::operator()(const sTradeBuyBase& base)
{
	return base.tradeItem;
}

void CTradeHandler::OnUpdateTradeBuyItem(int param)
{
	sTradeBuyResult* pResult =
		(sTradeBuyResult*)((const MsgObject*)param)->param2;

	std::for_each(pResult->list.begin(), pResult->list.end(), UpdateBuyItem);

	Doc()->m_myInfo.stat.i64Pang = pResult->pang;

	FrWnd* pWnd = FRESH_MANAGER->GetDesktop()->FindChildByName("tradedlg");
	FrTradeDlg* pDlg = TRADE_CAST(FrTradeDlg, pWnd);
	if (pDlg)
		pDlg->EnableSendingServer(false);

	std::vector<sTradeItem> itemList;
	std::transform(pResult->list.begin(), pResult->list.end(),
		std::back_inserter(itemList), sTransformHelper());

	SetTradeResult(pResult->result, 470, &itemList);
}

void CTradeHandler::OnUpdateTradeSellItem(int param)
{
	sTradeSellResult* pResult =
		(sTradeSellResult*)((const MsgObject*)param)->param2;

	std::for_each(pResult->list.begin(), pResult->list.end(), UpdateSellItem);

	Doc()->m_myInfo.stat.i64Pang += pResult->pang;
	Doc()->AddTradeIncome(pResult->pang);

	FrWnd* pWnd = FRESH_MANAGER->GetDesktop()->FindChildByName("tradedlg");
	FrTradeDlg* pDlg = TRADE_CAST(FrTradeDlg, pWnd);
	if (pDlg)
		pDlg->EnableSendingServer(false);

	std::string msg = MakeStr(
		"%d\xb0\xb3\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xb8\xf0\xb5\xce \xc6\xc7\xb8\xc5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.",
		pResult->list.size());
	CChatMsg::Instance()->AddChatMsg(msg, 0xffff, true, false);
}

void CTradeHandler::OnUpdateTradeMinusItem(int param)
{
	sTradeMinusResult* pResult =
		(sTradeMinusResult*)((const MsgObject*)param)->param2;

	if (Doc()->m_tradeUID == pResult->uid)
	{
		if (pResult->state == STATE_TRADE_OUTOFSTOCK)
			Doc()->SetStateTrade(STATE_TRADE_OUTOFSTOCK);

		std::for_each(pResult->list.begin(), pResult->list.end(),
			_util::MakeMemFun1(*Doc(), &CSharedDoc::MinusListTradeItem));

		FrWnd* pWnd =
			FRESH_MANAGER->GetDesktop()->FindChildByName("tradebuydlg");
		FrTradeBuyDlg* pBuyDlg = TRADE_CAST(FrTradeBuyDlg, pWnd);
		if (pBuyDlg)
		{
			if (!pBuyDlg->IsClosing())
			{
				// 983 (s84): FrTradeBuyDlg sem m_pPurchaseList (0x200); ClearTradeList do 645 nao existe
				CChatMsg::Instance()->AddChatMsg(
					K2L_Compatibility("\xb8\xf0\xb5\xe7 \xbb\xf3\xc7\xb0\xc0\xcc \xc6\xc7\xb8\xc5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
					0xffff, true, false);

				pBuyDlg->Close(true);

				pWnd = FRESH_MANAGER->GetDesktop()->FindChildByName(
					"notify_okcancel");
				FrWnd* pNotify = TRADE_CAST(FrWnd, pWnd);
				if (pNotify)
					pNotify->Close(true);
			}
		}

		pWnd = FRESH_MANAGER->GetDesktop()->FindChildByName("tradedlg");
		FrTradeDlg* pTradeDlg = NULL;
		if (pWnd)
			pTradeDlg = (FrTradeDlg*)pWnd;
		else
			__rtti_obj = NULL;
		if (pTradeDlg)
		{
			pTradeDlg->SetListTradeItem(true);
			pTradeDlg->SetListTradeItem(false);
		}

		if (Doc()->m_stateTrade == STATE_TRADE_OUTOFSTOCK)
		{
			if (strcmp(pResult->nick.c_str(), Doc()->m_myInfo.info.sNick) != 0)
			{
				if (pTradeDlg)
					pTradeDlg->Close(true);
			}
			else
			{
				if (pTradeDlg)
				{
					pTradeDlg->ShowStaticOutOfStock(true);
					pTradeDlg->EnableHideBn(false);
				}
				else
				{
					FrTradeDlg* pDlg =
						CreateForm<FrTradeDlg>(FRESH_MANAGER, this, "tradedlg");
					if (pDlg)
					{
						pDlg->Open(NULL, 0xc0);
						pDlg->OpenTradeShop(Doc()->m_tradeUID);
						pDlg->SetFadeout(false);
					}
				}
			}

			CChatMsg::Instance()->AddChatMsg(
				K2L_Compatibility("\xb8\xf0\xb5\xe7 \xbb\xf3\xc7\xb0\xc0\xcc \xc6\xc7\xb8\xc5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xffff, true, false);
		}
	}

	if (Doc()->m_stateTrade == STATE_TRADE_OUTOFSTOCK)
	{
		AfxGetTask()->GetActor("AvatarChat")
			<< MsgObject(NULL, M983(0xe8), (int)pResult->nick.c_str(), 3, 0, 0, 0);
	}
}

void CTradeHandler::OnSetEnablePackageSale()
{
	CTradeDoc* pDoc = GetDocument<CTradeDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->SetPackageSale(true);
}

void CTradeHandler::OnSetDisablePackageSale()
{
	CTradeDoc* pDoc = GetDocument<CTradeDoc*>();
	if (pDoc == NULL)
		return;

	pDoc->SetPackageSale(false);
}

void CTradeHandler::OnGetPackageSale(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;

	CTradeDoc* pDoc = GetDocument<CTradeDoc*>();
	if (pDoc == NULL)
		return;

	*(bool*)pMsg->param2 = pDoc->GetPackageSale();
}

// 983: _gateway::CTradeDoc nao existe no exe (sem RTTI); movida do gatewaydoc.cpp (D173) -- so este arquivo a usa.

CTradeDoc::CTradeDoc()
	: m_pHandler(NULL), m_bPackageSale(false)
{
}

CTradeDoc::~CTradeDoc()
{
}

bool CTradeDoc::Initialize()
{
	m_pHandler = _util::NewCreator<CTradeHandler>::CreateObject();
	if (m_pHandler)
	{
		m_pHandler->Initialize();
	}

	return true;
}

bool CTradeDoc::Release()
{
	if (m_pHandler)
	{
		delete m_pHandler;
		m_pHandler = NULL;
	}

	return true;
}
