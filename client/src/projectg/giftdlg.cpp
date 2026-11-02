#include "minatl.h"
#include "giftdlg.h"
#include "../../include/packets/browserpackets.h" // C33: OnChargeResult
#include "frienddlg.h"
#include "emoticondlg.h"
#include "frbutton.h"
#include "frstatic.h"
#include "frcombobox.h"
#include "fredit.h"
#include "frlistbox.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "binstr.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "buddymanager.h"
#include "networksystem.h"
#include "browser.h"
#include "logininfo.h"
#include "inputmanager.h"
#include "wview.h"
#include "mathconsts.h"
#include "../shared/s5/sharedutilities.h"
#include "../shared/s5/sharedutilities.h"
#include "packets/giftpackets.h"	// PKT (frente C)

#include <wininet.h>

// HACK: checkoutdlg.obj comes first in the link and owned this string in the original; its naked
// FrCheckoutDlg::SetPrice keeps it as a local static (aliased to the literal's name), so the C code
// here uses that copy through the alias below (a second copy from this unit would stay in .rdata)
#define STR_FMT_COOKIE "%I64d\xc4\xed\xc5\xb0"


extern Fresh* g_pFresh;

int float2int(float f);

namespace S5
{
	bool ISCutinItem(unsigned long typeId);
}

struct days_t
{
	const char* name;
	unsigned short value;
};

// 983 008eb350: tabela propria da unidade, mesmos dias do checkoutdlg.cpp
struct sGiftDayInfo
{
	const char* name;
	int value;
};
// inicializador dinamico, name = K2L_Compatibility(...)
static sGiftDayInfo days[6] = {
	{ K2L_Compatibility("1\xc0\xcf"), 1 },
	{ K2L_Compatibility("7\xc0\xcf"), 7 },
	{ K2L_Compatibility("15\xc0\xcf"), 15 },
	{ K2L_Compatibility("30\xc0\xcf"), 30 },
	{ K2L_Compatibility("365\xc0\xcf"), 365 },
	{ K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"), 0 }
};

IMPLEMENT_OBJECT(FrGiftDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrGiftDlg, FrForm)
ON_FRESH_VI("checkout", FRCMD_INIT, FrGiftDlg::OnCheckoutInit)
ON_FRESH_VV("checkout", FRCMD_LBUTTONUP, FrGiftDlg::OnCheckoutBtnUp)
ON_FRESH_VI("friend", FRCMD_INIT, FrGiftDlg::OnFriendBtnInit)
ON_FRESH_VV("friend", FRCMD_LBUTTONUP, FrGiftDlg::OnFriendBtnUp)
ON_FRESH_VI("cart", FRCMD_INIT, FrGiftDlg::OnCartInit)
ON_FRESH_VI("cart", FRCMD_OWNERDRAW, FrGiftDlg::OnCartOwnerDraw)
ON_FRESH_VI("sxid", FRCMD_INIT, FrGiftDlg::OnSxIdInit)
ON_FRESH_VI("edid", FRCMD_INIT, FrGiftDlg::OnEdIdInit)
ON_FRESH_BI("edid", FRCMD_ENTERKEY, FrGiftDlg::OnEdIdEnterKey)
ON_FRESH_VV("edid", FRCMD_LBUTTONDOWN, FrGiftDlg::OnEdIdDown)
ON_FRESH_VI("id_check", FRCMD_INIT, FrGiftDlg::OnBtIdCheckInit)
ON_FRESH_VV("id_check", FRCMD_LBUTTONUP, FrGiftDlg::OnBtIdCheckUp)
ON_FRESH_VI("sxnick", FRCMD_INIT, FrGiftDlg::OnSxNickInit)
ON_FRESH_VI("ednick", FRCMD_INIT, FrGiftDlg::OnEdNickInit)
ON_FRESH_BI("ednick", FRCMD_ENTERKEY, FrGiftDlg::OnEdNickEnterKey)
ON_FRESH_VV("ednick", FRCMD_LBUTTONDOWN, FrGiftDlg::OnEdNickDown)
ON_FRESH_VI("emoticon", FRCMD_INIT, FrGiftDlg::OnEmoticonBtnInit)
ON_FRESH_VV("emoticon", FRCMD_LBUTTONUP, FrGiftDlg::OnEmoticonBtnUp)
ON_FRESH_VI("nick_check", FRCMD_INIT, FrGiftDlg::OnBtNickCheckInit)
ON_FRESH_VV("nick_check", FRCMD_LBUTTONUP, FrGiftDlg::OnBtNickCheckUp)
ON_FRESH_VI("message", FRCMD_INIT, FrGiftDlg::OnMsgInit)
ON_FRESH_BI("message", FRCMD_ENTERKEY, FrGiftDlg::OnMsgEnterKey)
ON_FRESH_VI("confirm", FRCMD_INIT, FrGiftDlg::OnEdConfirmInit)
ON_FRESH_VI("price", FRCMD_INIT, FrGiftDlg::OnPriceInit)
ON_FRESH_VI("my_money", FRCMD_INIT, FrGiftDlg::OnMyMoneyInit)
ON_FRESH_VI("money_left", FRCMD_INIT, FrGiftDlg::OnMoneyLeftInit)
ON_FRESH_VI("price_cookie", FRCMD_INIT, FrGiftDlg::OnPrice_CookieInit)
ON_FRESH_VI("my_money_cookie", FRCMD_INIT, FrGiftDlg::OnMyMoney_CookieInit)
ON_FRESH_VI("money_left_cookie", FRCMD_INIT, FrGiftDlg::OnMoneyLeft_CookieInit)
ON_FRESH_VI("num_static", FRCMD_INIT, FrGiftDlg::OnNumStaticInit)
ON_FRESH_VI("num", FRCMD_INIT, FrGiftDlg::OnNumInit)
ON_FRESH_VI("num_prev", FRCMD_INIT, FrGiftDlg::OnNumPrevInit)
ON_FRESH_VV("num_prev", FRCMD_LBUTTONUP, FrGiftDlg::OnNumPrevBtnUp)
ON_FRESH_VI("num_next", FRCMD_INIT, FrGiftDlg::OnNumNextInit)
ON_FRESH_VV("num_next", FRCMD_LBUTTONUP, FrGiftDlg::OnNumNextBtnUp)
ON_FRESH_VI("days", FRCMD_INIT, FrGiftDlg::OnDaysInit)
ON_FRESH_VV("days", FRCMD_LBUTTONDOWN, FrGiftDlg::OnDaysBtnDown)
ON_FRESH_VI("warn", FRCMD_INIT, FrGiftDlg::OnWarnInit)
ON_FRESH_VI("explane", FRCMD_INIT, FrGiftDlg::OnExplaneStaticInit)
END_FRESH_MSGMAP()

FrGiftDlg::FrGiftDlg()
{
	memset(&m_item, 0, sizeof(m_item));
	m_receiverUid = (unsigned long)-1;
	m_receiverId = "";
	m_bEnoughPang = false;
	m_bEnoughCookie = false;
	m_pFriendBtn = NULL;
	m_pCheckout = NULL;
	m_pCart = NULL;
	m_pPrice = NULL;
	m_pMyMoney = NULL;
	m_pMoneyLeft = NULL;
	m_pPriceCookie = NULL;
	m_pMyMoneyCookie = NULL;
	m_pMoneyLeftCookie = NULL;
	m_pNumStatic = NULL;
	m_pNum = NULL;
	m_pNumPrev = NULL;
	m_pNumNext = NULL;
	m_pFriendDlg = NULL;
	m_pBtNickCheck = NULL;
	m_pWarn = NULL;
	m_pExplane = NULL;
	m_pSxNick = NULL;
	m_pEmoticonBtn = NULL;
	m_pEmoticonDlg = NULL;
	m_myPang = 0;
	m_myCookie = 0;
	m_remainCash = 0;
	m_bCanBuy = false;
	m_childCount = 0;
	memset(m_child, 0, sizeof(m_child));
	m_mode = -1;
}

void FrGiftDlg::SendGift(const IFF_ITEM_COMMON& item, __int64 myPang,
	__int64 myCookie)
{
	m_item = item;
	m_pang = 0;
	m_cookie = 0;
	m_bEnoughPang = false;
	m_bEnoughCookie = false;

	FrWnd* pWnd;
	if (m_item.IsCash)
		pWnd = m_pWarn;
	else
	{
		if (m_pSxId)
			m_pSxId->SetVisible(false);
		if (m_pEdId)
			m_pEdId->SetVisible(false);
		if (m_pBtIdCheck)
			m_pBtIdCheck->SetVisible(false);
		if (m_pSxNick)
			m_pSxNick->SetVisible(false);
		if (m_pEdNick)
			m_pEdNick->SetVisible(false);
		if (m_pBtNickCheck)
			m_pBtNickCheck->SetVisible(false);
		if (m_pEdMsg)
			m_pEdMsg->SetKeyFocus(true);
		if (m_pWarn)
			m_pWarn->SetVisible(true);
		pWnd = m_pEmoticonBtn;
	}
	if (pWnd)
		pWnd->SetVisible(false);

	m_dayIndex = 1;
	unsigned long group = m_item.TypeId >> 26;
	CItemManager* pMgr = ItemManager();
	if (group == 0xe)
	{
		IFF_STRUCT::sSkin* pSkin = pMgr->FindSkin(item.TypeId);
		if (pSkin && !S5::ISCutinItem(item.TypeId))
		{
			for (int i = 0; i < 5; i++)
			{
				if (pSkin->COM[i] != 0)
				{
					m_dayIndex = i;
					break;
				}
			}
		}
	}
	else if (group == 8)
	{
		IFF_STRUCT::sCadItem* pCad = pMgr->FindCadItem(item.TypeId);
		for (int i = 0; i < 4; i++)
		{
			if (pCad->COM[i] != 0)
			{
				m_dayIndex = i;
				break;
			}
		}
	}
	else if (group == 0x10)
	{
		// 983 00aae8d0: mascote de cookie sem TimeFlag = sem limite (dias[5])
		IFF_STRUCT::sMascot* pMascot = pMgr->FindMascot(item.TypeId);
		if (pMascot)
		{
			if (pMascot->c.IsCash && pMascot->c.TimeFlag == 0)
				m_dayIndex = 5;
			else
			{
				for (int i = 0; i < 4; i++)
				{
					if (pMascot->COM[i] != 0)
					{
						m_dayIndex = i;
						break;
					}
				}
			}
		}
	}
	else if (group == 6 && (item.TypeId & 0x1f00000) == 0x100000)
	{
		// 983 00aae8d0: item por dias
		IFF_STRUCT::sItem* pDayItem = pMgr->FindItem(item.TypeId);
		if (pDayItem)
		{
			for (int i = 0; i < 5; i++)
			{
				if (pDayItem->COM[i] != 0)
				{
					m_dayIndex = i;
					break;
				}
			}
		}
	}
	else
	{
		m_childCount = pMgr->FindChildItemNumber(item.TypeId);
		if (m_childCount > 0)
		{
			memset(m_child, 0, sizeof(m_child));
			if (m_childCount > 5)
				m_childCount = 5;
			for (int i = 0; i < m_childCount; i++)
			{
				IFF_STRUCT::sChildItem* pChild =
					ItemManager()->FindChildItem(item.TypeId, i);
				if (!pChild || !pChild->IsDisplay)
					continue;
				if (pChild->NumberFlag)
				{
					sprintf(m_child[i].name, "%d\xb0\xb3", pChild->Number);
					m_child[i].bCash = pChild->IsCash;
					m_child[i].numberPrice = pChild->NumberPrice;
					m_child[i].numberFlag = pChild->NumberFlag;
					m_child[i].number = pChild->Number;
				}
				else if (pChild->TimeFlag)
				{
					switch (pChild->TimeFlag)
					{
					case 0:
						break;
					case 3:
						sprintf(m_child[i].name, "%d\xbd\xc3\xb0\xa3",
							pChild->Time);
						break;
					case 4:
						sprintf(m_child[i].name, "%d\xc0\xcf", pChild->Time);
						break;
					case 5:
						sprintf(m_child[i].name, "%d\xb0\xb3\xbf\xf9",
							pChild->Time);
						break;
					default:
						sprintf(m_child[i].name, "%d\xc0\xcf", pChild->Time);
						break;
					}
					m_child[i].bCash = pChild->IsCash;
					m_child[i].timePrice = pChild->TimePrice;
					m_child[i].timeFlag = pChild->TimeFlag;
					m_child[i].time = pChild->Time;
				}
			}
		}
	}

	m_myPang = myPang;
	m_myCookie = myCookie;
	RecalculateTotalCookie(myPang, myCookie, true);

	bool bCount = false;
	if ((m_item.TypeId & 0xfc000000) == 0x18000000 &&
		(m_item.TypeId & 0x2000000) == 0)
	{
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(m_item.TypeId);
		if (pItem->COM[0] > 0)
		{
			bCount = true;
			if (m_pNum)
				m_pNum->SetLine(1, MakeStr("%d", (int)pItem->COM[0]), 0, false,
					0);
		}
	}

	if ((m_item.TypeId & 0x2000000) != 0 ||
		(m_item.TypeId & 0xfc000000) != 0x18000000 || bCount)
	{
		if (m_pNumStatic)
			m_pNumStatic->SetVisible(false);
		if (m_pNum)
			m_pNum->SetVisible(false);
		if (m_pNumPrev)
			m_pNumPrev->SetVisible(false);
		if (m_pNumNext)
			m_pNumNext->SetVisible(false);
	}
	else
	{
		if (m_pNum)
		{
			WPoint pos;
			pos.y = m_pNum->GetRect().y;
			pos.x =
				(float)((m_item.IsCash ? 100 : 0) - 50) + m_pNum->GetRect().x;
			m_pNum->MoveWindow(pos);
		}
		if (m_pNumPrev)
		{
			WPoint pos;
			pos.y = m_pNumPrev->GetRect().y;
			pos.x = (float)((m_item.IsCash ? 100 : 0) - 50) +
				m_pNumPrev->GetRect().x;
			m_pNumPrev->MoveWindow(pos);
		}
		if (m_pNumNext)
		{
			WPoint pos;
			pos.y = m_pNumNext->GetRect().y;
			pos.x = (float)((m_item.IsCash ? 100 : 0) - 50) +
				m_pNumNext->GetRect().x;
			m_pNumNext->MoveWindow(pos);
		}
	}

	m_bDaysChanged = true;
	m_pDays->SetVisible(false);
}

void FrGiftDlg::GiftProgress()
{
	// JP 983 (00aaf840; frente C, lote 14): registro de 0x25 bytes por item (sShopBuyItem983) com o preco do item
	// (CItemManager 00a1f680 = GetItemSalePrice) em pang ou cookie; o KR 645 mandava 0x10 bytes.
	FrListBox* pCart = m_pCart;
	const char* msg = m_pEdMsg->GetLine(1, false);
	Doc()->m_chatManager.FilteringHack(msg, true);
	unsigned long count = atoi(m_pNum->GetLine(1, false));

	sShopBuyItem983 entry;
	memset(&entry, 0, sizeof(entry));
	entry.guid = (unsigned long)-1;

	if (m_childCount > 0 && m_dayIndex < 5)
	{
		unsigned long group = m_item.TypeId >> 26;
		if (group == 6)
		{
			// 983: o item de grupo 6 com subtipo 0x100000 nao vai por aqui (sai sem mandar nem fechar)
			if ((m_item.TypeId & 0x1f00000) == 0x100000)
				return;

			WSendPacket send(0x1f);
			send.EncodeStr(m_myNick);
			send.Encode4(m_receiverUid);
			send.EncodeStr(std::string(msg));
			send.Encode1(1);
			if ((m_item.TypeId & 0x2000000) != 0)
			{
				if (m_child[m_dayIndex].numberFlag)
					entry.count = m_child[m_dayIndex].number;
				else
					entry.count = 1;
			}
			else
				entry.count = count;
			send.Encode2(1);
			entry.tid = m_item.TypeId;
			entry.days = m_child[m_dayIndex].time;
			entry.option = (unsigned short)m_dayIndex;
			if (m_item.IsCash)
			{
				entry.cookie = ItemManager()->GetItemSalePrice(m_item.TypeId, 1, entry.days);
				entry.pang = 0;
			}
			else
			{
				entry.cookie = 0;
				entry.pang = ItemManager()->GetItemSalePrice(m_item.TypeId, 1, entry.days);
			}
			send.EncodeBuffer(&entry, sizeof(entry));
			send.Send(TO_GAME);
		}
		else
		{
			unsigned long numItems = pCart->m_itemList.size();
			WSendPacket send(0x1f);
			send.EncodeStr(m_myNick);
			send.Encode4(m_receiverUid);
			send.EncodeStr(std::string(msg));
			send.Encode1(0);
			send.Encode2((unsigned short)numItems);
			FrListBox::ITEM_LIST::iterator it;
			for (it = pCart->m_itemList.begin(); it != pCart->m_itemList.end();
				it++)
			{
				IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
				entry.tid = pData->TypeId;
				entry.days = m_child[m_dayIndex].time;
				entry.option = (unsigned short)m_dayIndex;
				if (pData->IsCash)
				{
					entry.cookie = ItemManager()->GetItemSalePrice(entry.tid, count, entry.days);
					entry.pang = 0;
				}
				else
				{
					entry.cookie = 0;
					entry.pang = ItemManager()->GetItemSalePrice(entry.tid, count, entry.days);
				}
				send.EncodeBuffer(&entry, sizeof(entry));
			}
			send.Send(TO_GAME);
		}
	}
	else
	{
		unsigned long numItems = pCart->m_itemList.size();
		count = 1;
		WSendPacket send(0x1f);
		send.EncodeStr(m_myNick);
		send.Encode4(m_receiverUid);
		send.EncodeStr(std::string(msg));
		send.Encode1(0);
		send.Encode2((unsigned short)numItems);
		FrListBox::ITEM_LIST::iterator it;
		for (it = pCart->m_itemList.begin(); it != pCart->m_itemList.end();
			it++)
		{
			IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)(*it)->pData;
			entry.tid = pData->TypeId;
			entry.option = 0xffff;
			unsigned long group = m_item.TypeId >> 26;
			if (group == 5)
			{
				IFF_STRUCT::sBall* pBall =
					ItemManager()->FindBall(m_item.TypeId);
				if (pBall)
					count = pBall->COM[0];
			}
			else if (group == 6)
			{
				if ((m_item.TypeId & 0x2000000) == 0)
				{
					if ((m_item.TypeId & 0x2000000) == 0 && m_item.IsCash &&
						(short)numItems == 1)
					{
						IFF_STRUCT::sItem* pItem =
							ItemManager()->FindItem(m_item.TypeId);
						if (pItem)
							count = pItem->COM[0];
					}
				}
				else
				{
					IFF_STRUCT::sItem* pItem =
						ItemManager()->FindItem(m_item.TypeId);
					if (pItem)
						count = pItem->COM[0];
				}
			}
			group = m_item.TypeId >> 26;
			if (group == 6 && (m_item.TypeId & 0x1f00000) == 0x100000)
			{
				// 983: subtipo 0x100000 do grupo 6 leva o dia escolhido e quantidade 1
				entry.days = days[m_dayIndex].value;
				count = 1;
			}
			else if (group == 0xe || group == 8 || group == 0x10)
			{
				if (!S5::ISCutinItem(m_item.TypeId))
					entry.days = days[m_dayIndex].value;
			}
			else
				entry.days = 0;
			entry.count = count;
			if (pData->IsCash)
			{
				entry.cookie = ItemManager()->GetItemSalePrice(entry.tid, count, entry.days);
				entry.pang = 0;
			}
			else
			{
				entry.cookie = 0;
				entry.pang = ItemManager()->GetItemSalePrice(entry.tid, count, entry.days);
			}
			send.EncodeBuffer(&entry, sizeof(entry));
		}
		send.Send(TO_GAME);
	}

	Doc()->m_giftName = std::string(m_pEdNick->GetLine(1, false));
	Doc()->m_giftIndex = m_receiverUid;
	// 983: as duas mensagens vao ao ator principal da tarefa (a loja e ator do CLobbyTask; nao ha GetActor("Shop"))
	IActor* pMain = AfxGetTask()->GetMainActor();
	pMain << MsgObject(NULL, M983(0xc0), 0, 0, 0, 0, 0);
	pMain << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
	Close(FrOK, true);
}

void FrGiftDlg::SetReceiverInfo(unsigned long uid, const sPangYaUserInfo& info)
{
	m_receiverUid = uid;
	if (m_pEdNick)
		m_pEdNick->SetLine(1, info.sNick, 0, false, 0);
	if (m_pEdConfirm)
	{
		switch (m_mode)
		{
		case 0: // 983 00aaf290: os 2 modos mostram o nick, sem a conta mascarada do 645
		case 1:
			m_pEdConfirm->SetLine(1, info.sNick, 0, false, 0);
			m_receiverId = info.sNick;
			break;
		}
	}
}

void FrGiftDlg::SetPrice(int count)
{
	m_bEnoughPang = true;
	m_bEnoughCookie = true;

	if (m_pPrice && m_pMyMoney && m_pang != 0)
	{
		char buf[32] = "";
		__int64 num = count;
		MakePriceStrA(buf, num * m_pang);
		strcat(buf, K2L_Compatibility("\xc6\xce"));
		m_pPrice->SetLine(1, buf, 0, false, 0);
		if (Doc()->m_myInfo.stat.i64Pang - num * m_pang >= 0)
		{
			if (m_pMoneyLeft)
			{
				MakePriceStrA(buf, Doc()->m_myInfo.stat.i64Pang - num * m_pang);
				strcat(buf, K2L_Compatibility("\xc6\xce"));
				m_pMoneyLeft->SetLine(1, buf, 0, false, 0);
			}
		}
		else
		{
			if (m_pMoneyLeft)
			{
				MakePriceStrA(buf, num * m_pang - Doc()->m_myInfo.stat.i64Pang);
				strcat(buf, K2L_Compatibility("\xc6\xce \xba\xce\xc1\xb7"));
				m_pMoneyLeft->SetLine(1, buf, 0xffff0000, false, 0);
			}
			m_bEnoughPang = false;
		}
	}
	else if (m_pMoneyLeft)
	{
		m_pMoneyLeft->SetLine(1, m_pMyMoney->GetLine(1, false), 0, false, 0);
	}

	if (m_pPriceCookie && m_pMyMoneyCookie && m_cookie != 0)
	{
		char buf[32] = "\0";
		sprintf(buf, K2L_Compatibility("%d\xc4\xed\xc5\xb0"), m_cookie);
		m_pPriceCookie->SetLine(1, buf, 0, false, 0);
		__int64 remain = Doc()->m_cookie - m_cookie;
		if (remain >= 0)
		{
			if (m_pMoneyLeftCookie)
			{
				sprintf(buf, K2L_Compatibility("%d\xc4\xed\xc5\xb0"), remain);
				m_pMoneyLeftCookie->SetLine(1, buf, 0, false, 0);
			}
		}
		else
		{
			if (m_pMoneyLeftCookie)
			{
				MakePriceStrA(buf, m_cookie - Doc()->m_cookie);
				strcat(buf, K2L_Compatibility("\xc4\xed\xc5\xb0 \xba\xce\xc1\xb7"));
				m_pMoneyLeftCookie->SetLine(1, buf, 0xffff0000, false, 0);
			}
			m_bEnoughCookie = false;
		}
	}
	else if (m_pMoneyLeftCookie)
	{
		m_pMoneyLeftCookie->SetLine(1, m_pMyMoneyCookie->GetLine(1, false), 0,
			false, 0);
	}

	if (m_pNum)
		m_pNum->SetLine(1, MakeStr("%d", count), 0, false, 0);
}

bool FrGiftDlg::OnFriendListResult(int result, FrForm* form)
{
	if (result == 1)
	{
		m_mode = 0;
		m_receiverUid = m_pFriendDlg->GetUid();
		m_receiverId = m_pEdConfirm->GetLine(1, false);
	}
	m_pFriendDlg = NULL;
	return true;
}

bool FrGiftDlg::OnChargeResult(int result, FrForm* form)
{
	// JP 983 (00aacae0): pagina de pagamento do JP com o id do jogador e o KID que o login mandou (CSharedDoc+0x190), aberta
	// pelo navegador do 983 com o tipo 0xc (pacote 0xa1); a loja e ator do CLobbyTask, a mensagem 0xe4 vai ao ator principal.
	// O 983 grava WS_VSCROLL (0x200000) em CBrowser+0x110 (m_dwStyle) antes de abrir (s103: CBrowser no layout do 983).
	if (result == 1)
	{
		char url[0x400] = { 0 };
		switch (Doc()->m_provType)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 7:
		case 10:
		case 0xb:
		case 0xc:
		case 0xd:
			sprintf(url, "https://www3.pangya.jp/payment/login.aspx?ID=%s&KID=%s", Doc()->m_myInfo.info.sID,
				Doc()->m_paymentKID983.c_str());
			CBrowser::Instance()->SetStyle983(WS_VSCROLL);
			CBrowser::Instance()->OpenWeb983(url, false, 0xc);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xc2), 0, 0, 0, 0, 0));
			g_input->Reset();
			break;
		}
	}
	return true;
}

bool FrGiftDlg::OnConfirmResult(int result, FrForm* form)
{
	if (result == 1)
		GiftProgress();
	return true;
}

void FrGiftDlg::OnCheckoutInit(int param)
{
	m_pCheckout = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrGiftDlg::OnCheckoutBtnUp()
{
	if (m_mode == -1 || m_receiverUid == (unsigned long)-1)
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xba\xb8\xb3\xbb\xb1\xe2 \xc0\xfc\xbf\xa1 \xb9\xde\xb4\xc2 \xc0\xcc\xb0\xa1 \xc8\xae\xbd\xc7\xc8\xf7 \xb8\xc2\xb4\xc2\xc1\xf6\n\xb2\xc0 \\c0xffff0000\\c'\xc8\xae\xc0\xce'\\c0xff000000\\c \xb9\xf6\xc6\xb0\xc0\xbb \xb4\xad\xb7\xaf \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4"),
			0, 0, 0, 0);
	}
	else if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 8) & 1))
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xbc\xb1\xb9\xb0\xc7\xcf\xb1\xe2\xb8\xa6 \xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb0\xe8\xc1\xa4\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
	}
	else if ((m_mode == 0 && m_receiverId == Doc()->m_myInfo.info.sNick) ||
		(m_mode == 1 && m_receiverId == Doc()->m_myInfo.info.sNick))
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xc0\xda\xb1\xe2 \xc0\xda\xbd\xc5\xbf\xa1\xb0\xd4\xb4\xc2 \xbc\xb1\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
	}
	else
	{
		if (!m_bEnoughPang)
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc6\xce\xc0\xcc \xb8\xf0\xc0\xda\xb6\xf8\xb4\xcf\xb4\xd9.\n\xb0\xd4\xc0\xd3\xbf\xa1 \xc2\xfc\xbf\xa9\xc7\xd8\xbc\xad \xc6\xce\xc0\xbb \xb8\xf0\xc0\xb8\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0);
		}
		else
		{
			if (!m_bEnoughCookie)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify_okcancel", NULL);
				pForm->SetMessage(
					K2L_Compatibility("\\c0xffff0000\\c\xc4\xed\xc5\xb0\\c0xff000000\\c\xb0\xa1 \xb8\xf0\xc0\xda\xb6\xf8\xb4\xcf\xb4\xd9.\n\xc4\xed\xc5\xb0\xb8\xa6 \xc3\xe6\xc0\xfc\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
					false);
				pForm->Open((FRESH_PFN_RESULT)&FrGiftDlg::OnChargeResult, 3);
				return;
			}
			if (!ItemManager()->IsInSalePeriod(m_item.TypeId,
					Doc()->GetServerTime()))
			{
				MsgObject msgSale = MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
					0, 0, 0, 0);
				AfxGetTask()->GetMainActor() << msgSale;
			}
			else
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify_yesno", NULL);
				// 983 00ab0153: notify_yesno e so a frase (MakeStr(K2L_Compatibility(fmt), nick)), sem as 2 linhas do 645
				std::string text = MakeStr(
					K2L_Compatibility("'\\c0xffff0000\\c%s\\c0xff000000\\c'\xb4\xd4\xbf\xa1\xb0\xd4 \xbc\xb1\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbb\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
					m_pEdNick->GetLine(1, false));
				pForm->SetMessage(text.c_str(), false);
				pForm->Open((FRESH_PFN_RESULT)&FrGiftDlg::OnConfirmResult, 1);
				return;
			}
		}
	}
}

void FrGiftDlg::OnFriendBtnInit(int param)
{
	m_pFriendBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrGiftDlg::OnFriendBtnUp()
{
	if (BuddyMGR->GetBuddySize() == 0)
	{
		if (!WNetworkSystem::Instance()->IsConnected(
				(WNetworkSystem::eNetUnit)1))
		{
			WSendPacket send((enumClientPacket)0x3c);
			send.Encode2(0x11f);
			send.Send(TO_GAME);
		}
		else
		{
			WSendPacket send(0x13);
			send.Encode2(0x11f);
			send.Send(TO_MSN);
		}
	}
	if (m_pFriendDlg == NULL)
	{
		m_pFriendDlg = CreateForm<FrFriendDlg>(g_pFresh->GetManager(), this,
			"friend_list_form", NULL);
		if (m_pFriendDlg)
		{
			m_pEdId->ClearLine();
			m_pFriendDlg->SetParent(this);
			m_pFriendDlg->EnableDrag(false);
			// 983 00aaf3a0: ao lado do form; sem espaco na tela fica no x do proprio dialogo
			float x = GetRect().x + GetRect().w;
			if (g_view->GetWidth() - m_pFriendDlg->GetRect().w <= x)
				x = m_pFriendDlg->GetRect().x;
			WPoint pos(x, GetRect().y);
			m_pFriendDlg->Open((FRESH_PFN_RESULT)&FrGiftDlg::OnFriendListResult,
				pos, 1);
		}
	}
}

void FrGiftDlg::OnCartInit(int param)
{
	m_pCart = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	C_ASSERT_VC10(offsetof(FrGiftDlg, m_pad983_294) == 0x294 && offsetof(FrGiftDlg, m_clockIcon) == 0x324 &&
		offsetof(FrGiftDlg, m_childCount) == 0x33c); // E431
	const Bitmap* pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "clock");
	m_clockIcon.pBitmap = pBitmap;
	WRect rect(40.0f, 35.0f, pBitmap->Width(), pBitmap->Height());
	m_clockIcon.rect = rect;
}

void FrGiftDlg::OnCartOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;

	IFF_ITEM_COMMON* pData = (IFF_ITEM_COMMON*)pItem->pData;
	FrGraphicInterface* pGDI = m_pCart->WndManager()->GetGDI();

	m_pCart->WndManager()->GetBitmap("ITEMS", pData->Icon);
	const Bitmap* pBitmap;
	unsigned long group = pData->TypeId >> 26;
	if (group == 0xe)
		pBitmap = m_pCart->WndManager()->GetBitmap("SKINS", pData->Icon);
	else if (group == 0xf)
		pBitmap = m_pCart->WndManager()->GetBitmap("HAIRS", pData->Icon);
	else
	{
		pBitmap = m_pCart->WndManager()->GetBitmap("ITEMS", pData->Icon);
		if (pBitmap == NULL)
			pBitmap =
				m_pCart->WndManager()->GetBitmap("ITEMS_FASHION", pData->Icon);
	}

	if (pBitmap)
	{
		WRect rect(pItem->pos.x, pItem->pos.y, pBitmap->Width(),
			pBitmap->Height());
		if (group != 2 && group != 0xf)
		{
			rect.x += 10.0f;
			rect.y += 20.0f;
		}
		pGDI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
		if (m_childCount > 0 && m_child[m_dayIndex].timeFlag)
		{
			WRect clock(pItem->pos.x + m_clockIcon.rect.x,
				pItem->pos.y + 10.0f + m_clockIcon.rect.y, m_clockIcon.rect.w,
				m_clockIcon.rect.h);
			pGDI->DrawTexture(m_clockIcon.pBitmap, clock, 0xffffffff, 0);
		}
	}

	pGDI->SetTextColor(0xff000000, 0xffffffff);
	pGDI->SetTextStyle(1);
	const char* suffix =
		pData->InStock == 2 ? K2L_Compatibility("(\xbc\xb1\xb9\xb0\xc0\xfc\xbf\xeb)") : ""; // 983 00aace01
	float width =
		pGDI->Print(WPoint(pItem->pos.x + 90.0f, pItem->pos.y + 14.0f), 0,
			"%s%s", pData->Name, suffix);
	if (!ItemManager()->IsInSalePeriod(pData->TypeId, Doc()->GetServerTime()) &&
		pData->DateFlag != 2)
	{
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		pGDI->Print(WPoint((float)float2int(width) + 90.0f + pItem->pos.x,
						pItem->pos.y + 14.0f),
			0, K2L_Compatibility("(\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3 \xc1\xbe\xb7\xe1)"));
		pGDI->SetTextColor(0xff000000, 0xffffffff);
	}
	pGDI->SetTextStyle(0);

	float px = pItem->pos.x;
	float py = pItem->pos.y;
	unsigned long tid = pData->TypeId;
	if (m_childCount < 1)
	{
		const char* fmtPrice = pData->IsCash
			? K2L_Compatibility("%d\xc4\xed\xc5\xb0")
			: K2L_Compatibility("%d\xc6\xce + %d\xc6\xce(\xbc\xf6\xbc\xf6\xb7\xe1)");
		float w = pGDI->Print(WPoint(px + 90.0f, py + 34.0f), 0,
			K2L_Compatibility("\xb0\xa1    \xb0\xdd:   "));
		float priceX = (float)float2int(w) + 90.0f;
		WPoint pricePos(priceX + px, py + 34.0f);
		IFF_STRUCT::sItem* pDayItem = NULL;
		if (group == 6 && (tid & 0x1f00000) == 0x100000)
			pDayItem = ItemManager()->FindItem(tid);
		if (group == 0xe && !S5::ISCutinItem(tid))
		{
			IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(tid);
			if (pSkin)
				pGDI->Print(pricePos, 0, fmtPrice, (int)pSkin->COM[m_dayIndex],
					pSkin->COM[m_dayIndex] * 5 / 100);
		}
		else if (group == 8)
		{
			IFF_STRUCT::sCadItem* pCad = ItemManager()->FindCadItem(tid);
			pGDI->Print(pricePos, 0, fmtPrice, (int)pCad->COM[m_dayIndex],
				pCad->COM[m_dayIndex] * 5 / 100);
		}
		else if (group == 0x10)
		{
			IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tid);
			if (pMascot->c.IsCash && pMascot->c.TimeFlag == 0)
				pGDI->Print(pricePos, 0, fmtPrice, pMascot->c.Price); // 983 00aad2xx: sem limite
			else
				pGDI->Print(pricePos, 0, fmtPrice, (int)pMascot->COM[m_dayIndex],
					pMascot->COM[m_dayIndex] * 5 / 100);
		}
		else if (pDayItem)
		{
			pGDI->Print(pricePos, 0, fmtPrice, (int)pDayItem->COM[m_dayIndex],
				pDayItem->COM[m_dayIndex] * 5 / 100);
		}
		else
		{
			float oldW = pGDI->Print(pricePos, 0, fmtPrice, pData->Price,
				pData->Price * 5 / 100);
			// 983 00aad3xx: riscado do tamanho do preco antigo e preco novo logo depois; DateFlag 2 fora do periodo = sem promocao
			if (pData->SalePrice != 0 && pData->SalePrice < pData->Price &&
				(pData->DateFlag != 2 ||
					ItemManager()->IsInSalePeriod(tid, Doc()->GetServerTime())))
			{
				float saleX = (float)float2int(oldW) + priceX;
				float ly =
					(float)float2int((float)pGDI->GetFontHeight() * 0.5f) +
					34.0f + py;
				_WPOINT p1, p2;
				p1.x = priceX - 2.0f + px;
				p1.y = ly;
				p2.x = saleX + 2.0f + px;
				p2.y = ly;
				g_view->DrawLine2D(p1, p2, 0xff000000, 0xff000000, 0);
				pGDI->SetTextColor(0xffff0000, 0xffffffff);
				pGDI->Print(WPoint(saleX + px, py + 34.0f), 0,
					pData->IsCash
						? K2L_Compatibility(" %d\xc4\xed\xc5\xb0")
						: K2L_Compatibility(" %d\xc6\xce + %d\xc6\xce(\xbc\xf6\xbc\xf6\xb7\xe1)"),
					pData->SalePrice, pData->SalePrice * 5 / 100);
				pGDI->SetTextColor(0xff000000, 0xffffffff);
			}
		}
	}
	else
	{
		__int64 price = 0;
		if (m_child[m_dayIndex].numberFlag == 0)
		{
			if (m_child[m_dayIndex].timeFlag)
				price = m_child[m_dayIndex].timePrice;
		}
		else
		{
			price = m_child[m_dayIndex].numberPrice;
		}
		float w = pGDI->Print(WPoint(px + 90.0f, py + 34.0f), 0,
			K2L_Compatibility("\xb0\xa1    \xb0\xdd:   "));
		if (m_child[m_dayIndex].bCash)
			pGDI->Print(WPoint((float)float2int(w) + 90.0f + px, py + 34.0f), 0,
				K2L_Compatibility(STR_FMT_COOKIE), price);
		else
			pGDI->Print(WPoint((float)float2int(w) + 90.0f + px, py + 34.0f), 0,
				K2L_Compatibility("%I64d\xc6\xce"), price);
	}

	// 983 00aad92f: as linhas de baixo descem 20 quando ha periodo de venda, e o combo de dias junto
	float rowY = 54.0f;
	_SYSTEMTIME* pStart;
	_SYSTEMTIME* pEnd;
	if (ItemManager()->HasSalePeriod(tid, &pStart, &pEnd) &&
		pStart->wYear != 0 && pEnd->wYear != 0)
	{
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		pGDI->Print(WPoint(px + 90.0f, py + 54.0f), 0,
			K2L_Compatibility("\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3: %d\xbf\xf9 %d\xc0\xcf ~ %d\xbf\xf9 %d\xc0\xcf"),
			pStart->wMonth, pStart->wDay, pEnd->wMonth, pEnd->wDay);
		pGDI->SetTextColor(0xff000000, 0xffffffff);
		rowY = 74.0f;
		static WRect s_daysRect = m_pDays->GetRect();
		WRect daysRect(GetRect().x + 170.0f, py + 74.0f + 18.0f, s_daysRect.w,
			s_daysRect.h);
		m_pDays->SetRect(daysRect);
	}

	const Bitmap* pLevel = g_pFresh->GetManager()->GetBitmap("LEVELS",
		MakeStr("level_%03d", (pData->Level & 0x7f) + 1));
	if (pLevel)
	{
		float lw = pGDI->GetTextExtendOld983(K2L_Compatibility("\xb7\xb9    \xba\xa7:"));
		WRect lr(px + 102.0f + lw, py + rowY - 3.0f, pLevel->Width(),
			pLevel->Height());
		pGDI->DrawTexture(pLevel, lr, 0xffffffff, 0);
		pGDI->Print(WPoint(px + 90.0f, py + rowY), 0, K2L_Compatibility("\xb7\xb9    \xba\xa7:"));
		pGDI->Print(WPoint(lr.w + lr.x + 5.0f, py + rowY), 0, "%s",
			K2L_Compatibility("\xc0\xcc\xbb\xf3"));
	}
	rowY += 20.0f;

	const char* sLabelDays = K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:");
	const char* sUnlimited = K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1");
	WPoint rowPos(px + 90.0f, py + rowY);
	if (group == 7)
	{
		// 983 00aadbb3: mensalidade de cookie mostra sem limite
		pGDI->SetTextColor(0xff000000, 0xffffffff);
		pGDI->Print(rowPos, 0, sLabelDays);
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		const char* sValue = pData->IsCash ? sUnlimited
			: K2L_Compatibility("\xbf\xf9\xb1\xde\xc1\xa6");
		pGDI->Print(WPoint(pGDI->GetTextExtendOld983(sLabelDays) + 95.0f + px, py + rowY), 0, sValue);
		return;
	}

	const char* sel = NULL;
	if (group == 8)
	{
		pGDI->SetTextColor(0xff000000, 0xffffffff);
		pGDI->Print(rowPos, 0, sLabelDays);
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		if (m_bDaysChanged)
		{
			m_pDays->SetVisible(true);
			IFF_STRUCT::sCadItem* pCad = ItemManager()->FindCadItem(tid);
			for (int i = 0; i < 4; i++)
			{
				if (pCad->COM[i] != 0)
					m_pDays->AddString(days[i].name);
			}
			sel = days[m_dayIndex].name;
			m_pDays->SetLine(1, sel, 0, false, 0);
			m_bDaysChanged = false;
		}
	}
	else if (group == 0xe && !S5::ISCutinItem(tid) && (tid & 0x3c00000) != 0x1800000)
	{
		pGDI->SetTextColor(0xff000000, 0xffffffff);
		pGDI->Print(rowPos, 0, sLabelDays);
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		if (m_bDaysChanged)
		{
			m_pDays->SetVisible(true);
			IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(tid);
			for (int i = 0; i < 5; i++)
			{
				if (pSkin->COM[i] != 0)
					m_pDays->AddString(days[i].name);
			}
			sel = days[m_dayIndex].name;
			m_pDays->SetLine(1, sel, 0, false, 0);
			m_bDaysChanged = false;
		}
	}
	else if (group == 0x10)
	{
		pGDI->SetTextColor(0xff000000, 0xffffffff);
		pGDI->Print(rowPos, 0, sLabelDays);
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		if (m_bDaysChanged)
		{
			m_pDays->SetVisible(true);
			IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tid);
			for (int i = 0; i < 5; i++)
			{
				if (pMascot->COM[i] != 0)
					m_pDays->AddString(days[i].name);
			}
			if (pMascot->c.IsCash && pMascot->c.TimeFlag == 0)
			{
				m_pDays->AddString(sUnlimited);
				sel = sUnlimited;
			}
			else
				sel = days[m_dayIndex].name;
			m_pDays->SetLine(1, sel, 0, false, 0);
			m_bDaysChanged = false;
		}
	}
	else if (group == 6 && (tid & 0x1f00000) == 0x100000)
	{
		pGDI->SetTextColor(0xff000000, 0xffffffff);
		pGDI->Print(rowPos, 0, sLabelDays);
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		if (m_bDaysChanged)
		{
			m_pDays->SetVisible(true);
			IFF_STRUCT::sItem* pDayItem = ItemManager()->FindItem(tid);
			for (int i = 0; i < 5; i++)
			{
				if (pDayItem->COM[i] != 0)
					m_pDays->AddString(days[i].name);
			}
			sel = days[m_dayIndex].name;
			m_pDays->SetLine(1, sel, 0, false, 0);
			m_bDaysChanged = false;
		}
	}
	else if (m_childCount > 0)
	{
		IFF_STRUCT::sChildItem* pChild =
			ItemManager()->FindChildItem(tid, m_dayIndex);
		pGDI->SetTextColor(0xff000000, 0xffffffff);
		pGDI->Print(rowPos, 0, (pChild && pChild->NumberFlag)
			? K2L_Compatibility("\xb0\xb3    \xbc\xf6:") : sLabelDays);
		pGDI->SetTextColor(0xffff0000, 0xffffffff);
		if (m_bDaysChanged)
		{
			m_pDays->SetVisible(true);
			for (int i = 0; i < m_childCount; i++)
			{
				IFF_STRUCT::sChildItem* pC =
					ItemManager()->FindChildItem(tid, i);
				if (pC && pC->IsDisplay)
					m_pDays->AddString(m_child[i].name);
			}
			sel = m_child[m_dayIndex].name;
			m_pDays->SetLine(1, sel, 0, false, 0);
			m_bDaysChanged = false;
		}
	}
}

void FrGiftDlg::OnSxIdInit(int param)
{
	m_pSxId = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pSxId)
		m_pSxId->SetVisible(false);
}

void FrGiftDlg::OnEdIdInit(int param)
{
	m_pEdId = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pEdId)
		m_pEdId->SetKeyFocus(true);
	if (m_pEdId)
		m_pEdId->SetVisible(false);
}

bool FrGiftDlg::OnEdIdEnterKey(int param)
{
	return false;
}

void FrGiftDlg::OnEdIdDown()
{
}

void FrGiftDlg::OnEdNickDown()
{
	m_mode = -1;
}

void FrGiftDlg::OnBtIdCheckInit(int param)
{
	m_pBtIdCheck = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pBtIdCheck)
		m_pBtIdCheck->SetVisible(false);
}

void FrGiftDlg::OnBtIdCheckUp()
{
}

void FrGiftDlg::OnSxNickInit(int param)
{
	m_pSxNick = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrGiftDlg::OnEdNickInit(int param)
{
	m_pEdNick = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	// 983 00aabe70: o campo do nick abre com o foco do teclado
	if (m_pEdNick)
		m_pEdNick->SetKeyFocus(true);
}

bool FrGiftDlg::OnEdNickEnterKey(int param)
{
	OnBtNickCheckUp();
	return false;
}

void FrGiftDlg::OnMsgInit(int param)
{
	m_pEdMsg = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool FrGiftDlg::OnMsgEnterKey(int param)
{
	OnCheckoutBtnUp();
	return false;
}

void FrGiftDlg::OnEdConfirmInit(int param)
{
	m_pEdConfirm = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pEdConfirm)
		m_pEdConfirm->SetLine(1, K2L_Compatibility("<\xc8\xae\xc0\xce \xbe\xc8\xb5\xca>"), 0,
			false, 0);
}

void FrGiftDlg::OnBtNickCheckInit(int param)
{
	m_pBtNickCheck = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrGiftDlg::OnBtNickCheckUp()
{
	if (m_pEdConfirm)
		m_pEdConfirm->ClearLine();
	if (m_pEdNick)
	{
		const char* nick = m_pEdNick->GetLine(1, false);
		int len = strlen(nick);
		if (len <= 0)
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb9\xde\xb4\xc2\xc0\xcc\xc0\xc7 \xb4\xeb\xc8\xad\xb8\xed\xc0\xbb \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4"),
				0, 0, 0, 0);
		}
		else if (len >= 22)
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
				(int)MakeStr(
					K2L_Compatibility("\xb4\xeb\xc8\xad\xb8\xed\xc0\xba %d\xc0\xda \xb1\xee\xc1\xf6 \xc0\xd4\xb7\xc2\xc7\xd2\xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
					22),
				0, 0, 0, 0);
		}
		else if (strcmp(Doc()->m_myInfo.info.sNick, nick) != 0)
		{
			WSendPacket send((enumClientPacket)7);
			send.Encode1(0);
			send.EncodeStr(std::string(nick));
			send.Send(TO_GAME);
			m_mode = 0;
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		}
		else
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc0\xda\xb1\xe2 \xc0\xda\xbd\xc5\xbf\xa1\xb0\xd4\xb4\xc2 \xbc\xb1\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		}
	}
	if (m_pEdMsg)
		m_pEdMsg->SetKeyFocus(true);
}

void FrGiftDlg::OnPriceInit(int param)
{
	m_pPrice = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrGiftDlg::OnMyMoneyInit(int param)
{
	m_pMyMoney = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrGiftDlg::OnMoneyLeftInit(int param)
{
	m_pMoneyLeft = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrGiftDlg::OnPrice_CookieInit(int param)
{
	m_pPriceCookie = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrGiftDlg::OnMyMoney_CookieInit(int param)
{
	m_pMyMoneyCookie = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrGiftDlg::OnMoneyLeft_CookieInit(int param)
{
	m_pMoneyLeftCookie = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrGiftDlg::OnNumStaticInit(int param)
{
	m_pNumStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrGiftDlg::OnNumInit(int param)
{
	m_pNum = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrGiftDlg::OnNumPrevInit(int param)
{
	m_pNumPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pNumPrev)
	{
		m_pNumPrev->SetPushDelay(0.0f);
		m_pNumPrev->UseDblClick(false); // 983 00aac3e0: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrGiftDlg::OnNumPrevBtnUp()
{
	if (m_pNum)
	{
		int n = atoi(m_pNum->GetLine(1, false));
		if (n > 1)
			SetPrice(n - 1);
	}
}

void FrGiftDlg::OnNumNextInit(int param)
{
	m_pNumNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pNumNext)
	{
		m_pNumNext->SetPushDelay(0.0f);
		m_pNumNext->UseDblClick(false); // 983 00aac470: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrGiftDlg::OnNumNextBtnUp()
{
	if (m_pNum)
	{
		int n = atoi(m_pNum->GetLine(1, false));
		if (n < 10)
			SetPrice(n + 1);
	}
}

void FrGiftDlg::RecalculateTotalCookie(__int64 myPang, __int64 myCookie,
	bool bShow)
{
	m_pang = 0;
	m_cookie = 0;
	if (m_pCart)
	{
		m_pCart->ClearItem();
		unsigned long tid = m_item.TypeId;
		unsigned long group = tid >> 26;
		if (group == 0xe)
		{
			if ((tid & 0x3c00000) == 0x1400000)
			{
				__int64 price = ItemManager()->GetItemSalePrice(tid, 1, 0);
				if (m_item.IsCash)
					m_cookie += price;
				else
					m_pang += (price * 5) / 100 + price;
			}
			else
			{
				IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(tid);
				if (pSkin->c.IsCash)
					m_cookie += pSkin->COM[m_dayIndex];
				else
				{
					int p = (pSkin->COM[m_dayIndex] * 5) / 100 +
						pSkin->COM[m_dayIndex];
					m_pang += p;
				}
			}
		}
		else if (group == 8)
		{
			IFF_STRUCT::sCadItem* pCad = ItemManager()->FindCadItem(tid);
			if (pCad->c.IsCash)
				m_cookie += pCad->COM[m_dayIndex];
			else
			{
				int p =
					(pCad->COM[m_dayIndex] * 5) / 100 + pCad->COM[m_dayIndex];
				m_pang += p;
			}
		}
		else if (group == 0x10)
		{
			IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tid);
			if (pMascot->c.IsCash)
				m_cookie += pMascot->COM[m_dayIndex];
			else
			{
				int p = (pMascot->COM[m_dayIndex] * 5) / 100 +
					pMascot->COM[m_dayIndex];
				m_pang += p;
			}
		}
		else if (m_childCount > 0)
		{
			int idx = m_dayIndex;
			__int64 price = 0xffffffff;
			if (m_child[idx].timeFlag)
				price = m_child[idx].timePrice;
			else if (m_child[idx].numberFlag)
				price = m_child[idx].numberPrice;
			if (m_child[idx].bCash)
				m_cookie += price;
			else
				m_pang += (price * 5) / 100 + price;
		}
		else
		{
			// 983 00aae1d0: preco de GetItemSalePrice (00a1f680); grupo 6 pela quantidade do pacote (COM[0]) ou,
			// no subtipo 0x100000, pelo dia escolhido
			int num = 1;
			unsigned long day = 0;
			if (group == 6)
			{
				IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(tid);
				if ((tid & 0x1f00000) == 0x100000)
					day = days[m_dayIndex].value;
				else if (pItem->COM[0] >= 1)
					num = pItem->COM[0];
			}
			__int64 price = ItemManager()->GetItemSalePrice(tid, num, day);
			if (m_item.IsCash)
				m_cookie += price;
			else
				m_pang += (price * 5) / 100 + price;
		}
		m_pCart->AddItem(&m_item);
	}

	if (bShow)
	{
		char bPang = 0;
		bool bCookie = false;
		char buf[32];
		if (m_pPrice)
		{
			MakePriceStrA(buf, m_pang);
			strcat(buf, K2L_Compatibility("\xc6\xce"));
			m_pPrice->SetLine(1, buf, 0, false, 0);
			if (m_pang > 0)
				bPang = 1;
		}
		if (m_pPriceCookie)
		{
			sprintf(buf, K2L_Compatibility(STR_FMT_COOKIE), m_cookie);
			m_pPriceCookie->SetLine(1, buf, 0, false, 0);
			if (m_cookie > 0)
			{
				bCookie = true;
				m_remainCash =
					Doc()->RemainOwnCash(Doc()->m_provType, m_cookie);
			}
		}
		if (bPang != 1 || bCookie)
			m_bCanBuy = true;
		else
		{
			m_bCanBuy = false;
			m_remainCash = 0;
		}
		if (m_pMyMoney)
		{
			MakePriceStrA(buf, myPang);
			strcat(buf, K2L_Compatibility("\xc6\xce"));
			m_pMyMoney->SetLine(1, buf, 0, false, 0);
		}
		if (m_pMyMoneyCookie)
		{
			sprintf(buf, K2L_Compatibility(STR_FMT_COOKIE), myCookie);
			m_pMyMoneyCookie->SetLine(1, buf, 0, false, 0);
		}
	}
	SetPrice(1);
}

void FrGiftDlg::OnDaysInit(int param)
{
	m_pDays = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrGiftDlg::OnDaysBtnDown()
{
	if (m_childCount > 0)
	{
		const char* sel = m_pDays->GetLine(1, false);
		for (int i = 0; i < m_childCount; i++)
		{
			if (stricmp(m_child[i].name, sel) == 0)
				m_dayIndex = i;
		}
	}
	else
	{
		const char* sel = m_pDays->GetLine(1, false);
		for (int i = 0; i < 5; i++)
		{
			if (stricmp(days[i].name, sel) == 0)
				m_dayIndex = i;
		}
	}
	RecalculateTotalCookie(m_myPang, m_myCookie, true);
}

void FrGiftDlg::OnWarnInit(int param)
{
	m_pWarn = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pWarn)
		m_pWarn->SetCaption(
			K2L_Compatibility("\xc6\xce \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xc4\xa3\xb1\xb8\xbf\xa1\xb0\xd4\xb8\xb8 \xbc\xb1\xb9\xb0\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
}

void FrGiftDlg::OnEmoticonBtnInit(int param)
{
	m_pEmoticonBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrGiftDlg::OnEmoticonBtnUp()
{
	if (m_pEmoticonBtn && m_pEmoticonDlg == NULL)
	{
		m_pEmoticonDlg = CreateForm<FrEmoticonDlg>(g_pFresh->GetManager(), this,
			"emoticon", NULL);
		if (m_pEmoticonDlg)
		{
			WPoint pos(0.0f, 0.0f);
			const WRect& rect = m_pEmoticonBtn->GetRect();
			pos.x = rect.x - 200.0f;
			pos.y = rect.y + 24.0f;
			m_pEmoticonDlg->Open((FRESH_PFN_RESULT)&FrGiftDlg::OnEmoticonResult,
				pos, 3);
		}
	}
}

void FrGiftDlg::OnExplaneStaticInit(int param)
{
	m_pExplane = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

bool FrGiftDlg::OnEmoticonResult(int result, FrForm* form)
{
	if (m_pEmoticonDlg != NULL && result == 1)
	{
		FrEmoticonDlg* pDlg = DYNAMIC_CAST(FrEmoticonDlg, (FrWnd*)form);
		if (pDlg != NULL && pDlg->GetSelectedIcon() != NULL)
		{
			DYNAMIC_CAST(FrEdit, FindChildByName("id"));
			if (m_pEdNick)
			{
				const char* front = m_pEdNick->GetEditText_Front();
				const char* comp = m_pEdNick->GetEditText_Comp();
				const char* end = m_pEdNick->GetEditText_End();
				m_pEdNick->SetLine(1,
					MakeStr("%s%s%s%s", front, comp, pDlg->GetSelectedIcon(),
						end),
					0, false, 0);
				m_pEdNick->SetKeyFocus(true);
			}
		}
	}
	m_pEmoticonDlg = NULL;
	return true;
}
