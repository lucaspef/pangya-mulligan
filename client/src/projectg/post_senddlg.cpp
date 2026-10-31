#include "minatl.h"
#include "vc10_layout.h"
#include "frienddlg.h"
#include "post_senddlg.h"
#include "binstr.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "fredit.h"
#include "frarea.h"
#include "commonutil.h"
#include "emoticondlg.h"
#include "transferableitem.h"
#include "sea.h"
#include "wfont.h"
#include "../shared/localize.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frwndinl.h"
#include "golftask.h"
#include "actor.h"
#include "projectg.h"
#include "networksystem.h"
#include "buddymanager.h"
#include "gatewayactor.h"
#include "ucclibrary.h"
#include "shareddoc.h"
#include "../shared/globalnetworkdefine.h"
#include "jp/createcharacteroption.h"
#include <stdlib.h>
#include <algorithm>
#include <vector>


extern Fresh* g_pFresh;
int float2int(float f);

void __fastcall CallErrorMsg(const char* msg)
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), NULL, "notify", NULL);
	pForm->SetMessage(msg, false);
	pForm->Open(NULL, 3);
}

IMPLEMENT_OBJECT(CPost_SendDlg, FrForm)

BEGIN_FRESH_MSGMAP(CPost_SendDlg, FrForm)
ON_FRESH_VI("recive_user", FRCMD_INIT, CPost_SendDlg::OnPostReciveUserInit)
ON_FRESH_BI("recive_user", FRCMD_ENTERKEY,
	CPost_SendDlg::OnPostReceiveUserEnterKey)
ON_FRESH_VI("user_Nick", FRCMD_INIT, CPost_SendDlg::OnPostUserNickInit)
ON_FRESH_VI("recive_msg", FRCMD_INIT, CPost_SendDlg::OnPostReciveMsgInit)
ON_FRESH_BI("recive_msg", FRCMD_ENTERKEY,
	CPost_SendDlg::OnPostReciveMsgEnterKey)
ON_FRESH_VI("search_user", FRCMD_INIT, CPost_SendDlg::OnPostSearchUserInit)
ON_FRESH_VV("search_user", FRCMD_LBUTTONUP,
	CPost_SendDlg::OnPostSearchUserBtnUp)
ON_FRESH_VI("friend_list", FRCMD_INIT, CPost_SendDlg::OnPostFriendListInit)
ON_FRESH_VV("friend_list", FRCMD_LBUTTONUP,
	CPost_SendDlg::OnPostFriendListBtnUp)
ON_FRESH_VI("item_appending", FRCMD_INIT, CPost_SendDlg::OnPostItemAddInit)
ON_FRESH_VV("item_appending", FRCMD_LBUTTONUP,
	CPost_SendDlg::OnPostItemAddBtnUp)
ON_FRESH_VI("post_slot", FRCMD_INIT, CPost_SendDlg::OnPostListInit)
ON_FRESH_VI("post_slot", FRCMD_OWNERDRAW, CPost_SendDlg::OnPostListOwnerDraw)
ON_FRESH_VI("Btnemoticon", FRCMD_INIT, CPost_SendDlg::OnEmoticonBtnInit)
ON_FRESH_VV("Btnemoticon", FRCMD_LBUTTONUP, CPost_SendDlg::OnEmoticonBtnUp)
ON_FRESH_VI("edit_servicecharge", FRCMD_INIT,
	CPost_SendDlg::OnPostServiceChargeInit)
END_FRESH_MSGMAP()

CPost_SendDlg::CPost_SendDlg()
{
	m_pFriendDlg = NULL;
	m_pReciveUser = NULL;
	m_pReciveMsg = NULL;
	m_pSearchUser = NULL;
	m_pFriendList = NULL;
	m_pEmoticonBtn = NULL;
	m_pEmoticonDlg = NULL;
	m_pItemAdd = NULL;
	m_pMyItemList = NULL;
	m_receiverUid = 0xffffffff;
	m_receiverNick = "";
	m_receiverId = "";
	m_bSearched = false;
	CTransferableItem::Instance()->Reset();
	CTransferableItem::Instance()->SetTransferableItemList();
	CTransferableItem::Instance()->Set_RegistrationListSize(4);
	m_pUccOriginal = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pUccCopied = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pUccTempSaved = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
	static char* s_fontWind[] = { "[font_wind.jpg" };
	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_fontWind;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);
}

CPost_SendDlg::~CPost_SendDlg()
{
	Reset();
	if (m_pTitleFont)
		m_pTitleFont->Erase();
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

void CPost_SendDlg::Reset()
{
	// HACK: dead reference, only to emit the FrEdit::GetSelectLine COMDAT where the original has it
	if (0)
		m_pReciveMsg->GetSelectLine();
	m_pFriendDlg = NULL;
	m_pReciveUser = NULL;
	m_pReciveMsg = NULL;
	m_pSearchUser = NULL;
	m_pFriendList = NULL;
	m_pEmoticonBtn = NULL;
	m_pEmoticonDlg = NULL;
	m_pItemAdd = NULL;
	m_pMyItemList = NULL;
	m_receiverUid = 0xffffffff;
	m_receiverNick = "";
	m_receiverId = "";
	m_bSearched = false;
	CTransferableItem::Instance()->Reset();
	CTransferableItem::Instance()->SetTransferableItemList();
	CTransferableItem::Instance()->Set_RegistrationListSize(4);
}

void CPost_SendDlg::OnPostReciveUserInit(int param)
{
	m_pReciveUser = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_SendDlg::OnPostReciveMsgInit(int param)
{
	m_pReciveMsg = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pReciveMsg)
	{
		m_pReciveMsg->SetCharLimit(800, true);
	}
}

// ---- CPost_SendDlg ----

bool CPost_SendDlg::OnPostReciveMsgEnterKey(int param)
{
	return true;
}

void CPost_SendDlg::OnPostUserNickInit(int param)
{
	m_pUserNick = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void CPost_SendDlg::OnPostSearchUserInit(int param)
{
	m_pSearchUser = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

bool CPost_SendDlg::OnPostReceiveUserEnterKey(int param)
{
	OnPostSearchUserBtnUp();
	return false;
}

void CPost_SendDlg::OnPostSearchUserBtnUp()
{
	if (m_pReciveUser)
	{
		const char* nick = m_pReciveUser->GetLine(1, false);
		if (_stricmp(nick, Doc()->m_myInfo.info.sNick) == 0)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xba\xbb\xc0\xce\xbf\xa1\xb0\xd4\xb4\xc2 \xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. "),
				false);
			pForm->Open(NULL, 3);
			return;
		}
		int len = strlen(nick);
		if (nick && len > 0 && len < 22)
		{
			WSendPacket packet((enumClientPacket)7);
			packet.Encode1(0);
			packet.EncodeStr(std::string(nick));
			packet.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
		else
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb9\xde\xb4\xc2 \xc0\xcc\xc0\xc7 \xb4\xd0\xb3\xd7\xc0\xd3\xc0\xbb \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0));
		}
	}
}

void CPost_SendDlg::OnPostFriendListInit(int param)
{
	m_pFriendList = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CPost_SendDlg::OnPostFriendListBtnUp()
{
	if (BuddyMGR->GetBuddySize() == 0)
	{
		if (!WSingleton<WNetworkSystem>::Instance()->IsConnected(
				WNetworkSystem::NET_MSN))
		{
			WSendPacket packet((enumClientPacket)0x3c);
			packet.Encode2(0x11f);
			packet.Send(TO_GAME);
		}
		else
		{
			WSendPacket packet(0x13);
			packet.Encode2(0x11f);
			packet.Send(TO_MSN);
		}
	}
	if (m_pFriendDlg == NULL)
	{
		m_pFriendDlg = CreateForm<FrFriendDlg>(g_pFresh->GetManager(), this,
			"post_friend_list_form", NULL);
		if (m_pFriendDlg)
		{
			m_pFriendDlg->SetParent2(this);
			m_pFriendDlg->EnableDrag(false);
			WPoint pos(0, 0);
			pos.x = 602.0f;
			pos.y = GetRect().y;
			m_pFriendDlg->Open(
				(FRESH_PFN_RESULT)&CPost_SendDlg::OnFriendListResult, pos, 1);
		}
	}
}

void CPost_SendDlg::OnPostItemAddInit(int param)
{
	m_pItemAdd = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CPost_SendDlg::OnPostItemAddBtnUp()
{
	// 983 (0080e330): sem a consulta ao gateway (M983 0x26d) do 645
	if (m_pMyItemList == NULL)
	{
		m_pMyItemList = CreateForm<CPost_MyItemList>(g_pFresh->GetManager(),
			this, "myitem_choice", NULL);
		FrForm::ms_bHasTail = false;
		m_pMyItemList->Open(
			(FRESH_PFN_RESULT)&CPost_SendDlg::OnPostItemListResutl, 0x81);
		m_pMyItemList->SetFadeout(false);
	}
}

void CPost_SendDlg::OnPostListInit(int param)
{
	m_pPostList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void CPost_SendDlg::OnPostListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (item)
	{
		sTradeItem* info = (sTradeItem*)item->pData;
		FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
		if (info)
		{
			unsigned long tid = info->dwTid;
			int num = info->iNum;
			if (gdi && tid)
			{
				IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
				if (pCommon)
				{
					float offsetX = 0.0f;
					float offsetY = 0.0f;
					const Bitmap* icon = g_pFresh->GetBitmap(pCommon->Icon);
					if (IsLocalContent(S4_UCC) && (info->status & 1) &&
						IsUccClothes(info->dwTid))
					{
						const Bitmap* ucc = GetUccItemIcon(info->dwGuid);
						if (ucc)
							icon = ucc;
					}
					if ((info->dwTid & 0xfc000000) == 0x8000000)
					{
						IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(
							((info->dwTid >> 18) & 0xff) | 0x4000000);
						if (pChar)
						{
							const Bitmap* face = g_pFresh->GetBitmap(
								MakeStr("s_%s", pChar->c.Icon));
							if (face)
							{
								WRect rect(item->pos.x + 3.0f,
									item->pos.y + 3.0f, (float)face->Width(),
									(float)face->Height());
								gdi->DrawTexture(face, rect, 0xffffffff, 0);
							}
						}
					}
					if ((tid & 0xfc000000) == 0x8000000)
					{
						offsetX = -12.0f;
						offsetY = -20.0f;
					}
					if (icon)
					{
						WRect rect(offsetX + item->pos.x + 14.0f,
							offsetY + item->pos.y + 11.0f, (float)icon->Width(),
							(float)icon->Height());
						gdi->DrawTexture(icon, rect, 0xffffffff, 0);
						gdi->SetTextColor(0xff000000, 0xffffffff);
						gdi->SetTextStyle(4);
					}
					if (IsLocalContent(S4_UCC) && IsUccClothes(info->dwTid))
					{
						unsigned char ucc = info->status & 1;
						if (ucc && info->Seq == 1 && m_pUccOriginal)
						{
							WRect rect(item->pos.x + 3.0f, item->pos.y + 15.0f,
								(float)m_pUccOriginal->Width(),
								(float)m_pUccOriginal->Height());
							gdi->DrawTexture(m_pUccOriginal, rect, 0xffffffff,
								0);
						}
						else if (ucc && info->Seq >= 2 && m_pUccCopied)
						{
							WRect rect(item->pos.x + 3.0f, item->pos.y + 15.0f,
								(float)m_pUccCopied->Width(),
								(float)m_pUccCopied->Height());
							gdi->DrawTexture(m_pUccCopied, rect, 0xffffffff, 0);
						}
						else if ((info->status & 2) && m_pUccTempSaved)
						{
							WRect rect(item->pos.x + 3.0f, item->pos.y + 15.0f,
								(float)m_pUccTempSaved->Width(),
								(float)m_pUccTempSaved->Height());
							gdi->DrawTexture(m_pUccTempSaved, rect, 0xffffffff,
								0);
						}
					}
					// 983 (0080c320): sem o desenho dos encaixes de carta (DrawPartsSlot)
					if (num > 1)
					{
						const char* count = MakeStr("%d", num);
						float x = item->pos.x + 54.0f;
						for (unsigned int i = 0; i < strlen(count); ++i)
							x -= m_pTitleFont->GetCharWidth(g_view,
								count[i] - '0');
						m_pTitleFont->Print(g_view, x, item->pos.y + 42.0f,
							count, 0, 0xffffffff, NULL);
					}
				}
			}
		}
	}
}

inline void CPost_SendDlg::DrawPartsSlot(FrGraphicInterface* gdi, FrListItem* item)
{
	if (item->pData && (gdi || (gdi = g_pFresh->GetManager()->GetGDI())))
	{
		float charX = 5.0f;
		float caddieX = 5.0f;
		sTradeItem* info = (sTradeItem*)item->pData;
		int slot;

		if (info->CharacterSlotNum <= -1 || info->CharacterSlotNum >= 5)
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(info->dwTid);
			if (!part)
			{
				info->CharacterSlotNum = 0;
			}
			else
			{
				info->CharacterSlotNum = part->CharacterSlot;
				if (info->CharacterSlotNum <= -1 || info->CharacterSlotNum >= 5)
					info->CharacterSlotNum = 0;
			}
		}
		for (slot = 1; slot < info->CharacterSlotNum + 1; ++slot)
		{
			bool found = false;
			for (std::list<sSCardAvilityPeriodInfo>::iterator it =
					 Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
			{
				if (info->dwTid == (*it).partsTid &&
					info->dwGuid == (*it).partsUid)
				{
					IFF_STRUCT::sCard* card =
						ItemManager()->FindCard((*it).tid);
					if (card && slot == (*it).slotNum && (*it).cardType == 0)
					{
						found = true;
						const Bitmap* bitmap =
							g_pFresh->GetBitmap(card->SlotImg);
						if (bitmap)
						{
							WRect rect(charX + item->pos.x, item->pos.y + 54.0f,
								(float)bitmap->Width(),
								(float)bitmap->Height());
							gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
							charX += 16.0f;
						}
					}
				}
			}
			if (!found)
			{
				const Bitmap* bitmap =
					g_pFresh->GetBitmap("card_character_empty");
				if (bitmap)
				{
					WRect rect(charX + item->pos.x, item->pos.y + 54.0f,
						(float)bitmap->Width(), (float)bitmap->Height());
					gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
					charX += 16.0f;
				}
			}
		}

		if (info->CaddieSlotNum <= -1 || info->CaddieSlotNum >= 5)
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(info->dwTid);
			if (!part)
			{
				info->CaddieSlotNum = 0;
			}
			else
			{
				info->CaddieSlotNum = part->CaddieSlot;
				if (info->CaddieSlotNum <= -1 || info->CaddieSlotNum >= 5)
					info->CaddieSlotNum = 0;
			}
		}
		for (slot = 1; slot < info->CaddieSlotNum + 1; ++slot)
		{
			bool found = false;
			for (std::list<sSCardAvilityPeriodInfo>::iterator it =
					 Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
			{
				if (info->dwTid == (*it).partsTid &&
					info->dwGuid == (*it).partsUid)
				{
					IFF_STRUCT::sCard* card =
						ItemManager()->FindCard((*it).tid);
					if (card && slot == (*it).slotNum && (*it).cardType == 1)
					{
						found = true;
						const Bitmap* bitmap =
							g_pFresh->GetBitmap(MakeStr(card->SlotImg));
						if (bitmap)
						{
							WRect rect(caddieX + item->pos.x,
								item->pos.y + 70.0f, (float)bitmap->Width(),
								(float)bitmap->Height());
							gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
							caddieX += 16.0f;
						}
					}
				}
			}
			if (!found)
			{
				const Bitmap* bitmap = g_pFresh->GetBitmap("card_caddie_empty");
				if (bitmap)
				{
					WRect rect(caddieX + item->pos.x, item->pos.y + 70.0f,
						(float)bitmap->Width(), (float)bitmap->Height());
					gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
					caddieX += 16.0f;
				}
			}
		}
	}
}

void CPost_SendDlg::OnPostServiceChargeInit(int param)
{
	m_pServiceCharge = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pServiceCharge)
	{
		m_pServiceCharge->AddText(MakeStr(K2L_Compatibility("%6d"), 100), false, true);
	}
}

void CPost_SendDlg::OnEmoticonBtnInit(int param)
{
	m_pEmoticonBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pEmoticonBtn)
	{
		m_pEmoticonBtn->SetVisible(true);
	}
}

void CPost_SendDlg::OnEmoticonBtnUp()
{
	if (m_pEmoticonBtn && m_pEmoticonDlg == NULL)
	{
		m_pEmoticonDlg = CreateForm<FrEmoticonDlg>(g_pFresh->GetManager(), this,
			"emoticon", NULL);
		if (m_pEmoticonDlg)
		{
			WPoint pos(0, 0);
			const WRect& rect = m_pEmoticonBtn->GetRect();
			pos.x = rect.x - 200.0f;
			pos.y = rect.y + 24.0f;
			m_pEmoticonDlg->Open(
				(FRESH_PFN_RESULT)&CPost_SendDlg::OnEmoticonResult, pos, 3);
		}
	}
}


void CPost_SendDlg::OnOK()
{
	int serviceCharge =
		CTransferableItem::Instance()->Get_RegistrationListSize();
	std::string letter = GetLetterMsg();
	int letterLen = letter.size();
	const char* msg;

	if (!m_bSearched)
	{
		CallErrorMsg(
			K2L_Compatibility("\xbc\xb1\xb9\xb0\xc7\xd2 \xb4\xeb\xbb\xf3\xc0\xc7 \xc0\xcc\xb8\xa7\xc0\xcc \xc8\xae\xc0\xce\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9. \xb0\xcb\xbb\xf6\xc0\xb8\xb7\xce \xc8\xae\xc0\xce\xc7\xcf\xbd\xc3\xb0\xc5\xb3\xaa \xc4\xa3\xb1\xb8\xb8\xae\xbd\xba\xc6\xae\xbf\xa1\xbc\xad \xc3\xdf\xb0\xa1\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4. "));
		return;
	}
	if (strlen(m_pReciveUser->GetLine(1, false)) == 0)
	{
		if (strlen(m_pUserNick->GetLine(1, false)) == 0)
		{
			CallErrorMsg(
				K2L_Compatibility("\xbc\xb1\xb9\xb0\xc7\xd2 \xb4\xeb\xbb\xf3\xc0\xc7 \xc0\xcc\xb8\xa7\xc0\xcc \xc8\xae\xc0\xce\xb5\xc7\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9. \xb0\xcb\xbb\xf6\xc0\xb8\xb7\xce \xc8\xae\xc0\xce\xc7\xcf\xbd\xc3\xb0\xc5\xb3\xaa \xc4\xa3\xb1\xb8\xb8\xae\xbd\xba\xc6\xae\xbf\xa1\xbc\xad \xc3\xdf\xb0\xa1\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4. "));
			return;
		}
	}
	if (strlen(m_pReciveMsg->GetLine(1, false)) == 0)
	{
		if (CTransferableItem::Instance()->Get_RegistrationListSize() == 0)
		{
			CallErrorMsg(
				K2L_Compatibility("\xb8\xde\xbd\xc3\xc1\xf6\xb3\xaa \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc3\xb7\xba\xce\xc7\xcf\xbc\xc5\xbe\xdf \xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
			return;
		}
	}
	if (serviceCharge == 0 && Doc()->m_myInfo.stat.i64Pang < 100)
	{
		CallErrorMsg(
			K2L_Compatibility("\xbc\xf6\xbc\xf6\xb7\xe1\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."));
	}
	else
	{
		if ((__int64)(serviceCharge * 500) <= Doc()->m_myInfo.stat.i64Pang)
		{
			if (letterLen > 800)
			{
				CallErrorMsg(
					K2L_Compatibility("\xc6\xed\xc1\xf6 \xb3\xbb\xbf\xeb\xc0\xba 800\xc0\xda\xb8\xa6 \xc3\xca\xb0\xfa\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
				return;
			}
			if (VerifySendItemList())
			{
				if (VerifyIncludeSameItem())
				{
					FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "rmr_notify", NULL);
					pForm->SetMessage(
						MakeStr(
							K2L_Compatibility("\xbc\xf6\xbc\xf6\xb7\xe1\xb0\xa1 %d \xc0\xd4\xb4\xcf\xb4\xd9. \n\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbb\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
							GetSendServiceCharge()),
						false);
					pForm->Open(
						(FRESH_PFN_RESULT)&CPost_SendDlg::OnSendQuestionResult,
						2);
					return;
				}
				msg = K2L_Compatibility(
					"\xb0\xb0\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb5\xce \xb0\xa1\xc1\xf6 \xc0\xcc\xbb\xf3 \xc3\xb7\xba\xce\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			}
			else
			{
				msg =
					K2L_Compatibility("\xc3\xb7\xba\xce\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xb5\xe9 \xc1\xdf\xbf\xa1 \xb5\xa5\xc0\xcc\xc5\xcd\xb0\xa1 \xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			}
		}
		else
		{
			msg =
				K2L_Compatibility("\xbc\xf6\xbc\xf6\xb7\xe1\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
		}
		CallErrorMsg(msg);
		CTransferableItem::Instance()->ResetRegistrationItemList();
		m_pPostList->ClearItem();
	}
}

bool CPost_SendDlg::OnSendQuestionResult(int result, FrForm* form)
{
	if (result == 1)
	{
		SetFadeout(true);
		FrForm::OnOK();
	}
	return true;
}

void CPost_SendDlg::OnCancel()
{
	CTransferableItem::Instance()->ResetRegistrationItemList();
	FrForm::OnCancel();
}

bool CPost_SendDlg::OnFriendListResult(int result, FrForm* form)
{
	if (result == 1)
	{
		m_bSearched = true;
		if (!m_pFriendDlg->IsSelectPerson() && m_pFriendDlg->HaveFriends())
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc4\xa3\xb1\xb8\xb8\xa6 \xbc\xb1\xc5\xc3\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0));
			m_pFriendDlg = NULL;
			return true;
		}
		m_receiverUid = m_pFriendDlg->GetUid();
		OnPostSearchUserBtnUp();
		m_pFriendDlg = NULL;
		return true;
	}
	m_pReciveUser->ClearLine();
	m_pUserNick->ClearLine();
	m_receiverUid = 0xffffffff;
	m_receiverId.clear();
	m_receiverNick.clear();
	m_pFriendDlg = NULL;
	return true;
}

bool CPost_SendDlg::OnPostItemListResutl(int result, FrForm* form)
{
	if (form)
		FrForm::ms_bHasTail = true;
	if (result == 1)
	{
		m_pPostList->ClearItem();
		std::list<sTradeItem>* pList =
			CTransferableItem::Instance()->GetList_RegistrationItem();
		std::list<sTradeItem>::iterator it;
		for (it = pList->begin(); it != pList->end(); ++it)
		{
			if (IsLocalContent(S4_CARD_SYSTEM))
			{
				IFF_STRUCT::sPart* pPart = ItemManager()->FindPart((*it).dwTid);
				if (pPart)
				{
					(*it).CharacterSlotNum = pPart->CharacterSlot;
					(*it).CaddieSlotNum = pPart->CaddieSlot;
					(*it).NpcSlotNum983 = pPart->NpcSlot983; // 983 0080d9c0: +0x6b <- sPart+0x22c
				}
				else
				{
					(*it).CharacterSlotNum = 0;
					(*it).CaddieSlotNum = 0;
					(*it).NpcSlotNum983 = 0;
				}
			}
			m_pPostList->AddItem(&(*it));
		}
	}
	else
	{
		std::list<sTradeItem>* pList =
			CTransferableItem::Instance()->GetList_RegistrationItem();
		if (m_pPostList->GetCurrentItemSize(true) != 0)
		{
			std::list<sTradeItem>::iterator it;
			for (it = pList->begin(); it != pList->end();)
			{
				if (m_pPostList->FindItem(&(*it)) == NULL)
					CTransferableItem::Instance()
						->DelRegistrationItem_itor_return((*it).dwGuid, &it);
				else
					++it;
			}
		}
		else
		{
			CTransferableItem::Instance()->ResetRegistrationItemList();
		}
	}
	form = (FrForm*)CTransferableItem::Instance()->Get_RegistrationListSize();
	int charge = (int)form * 500;
	if ((__int64)charge > Doc()->m_myInfo.stat.i64Pang)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), NULL, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xbc\xf6\xbc\xf6\xb7\xe1\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 3);
		CTransferableItem::Instance()->ResetRegistrationItemList();
		m_pPostList->ClearItem();
	}
	m_pServiceCharge->ClearLine();
	if (form == NULL)
		charge = 100;
	m_pServiceCharge->AddText(MakeStr(K2L_Compatibility("%6d"), charge), false, true);
	m_pMyItemList = NULL;
	return true;
}

void CPost_SendDlg::SetReceiverInfo(unsigned long uid,
	const sPangYaUserInfo& info)
{
	const char* nick = info.sNick;
	m_receiverUid = uid;
	m_receiverNick = nick;
	m_receiverId = info.sID;
	m_receiverId =
		(IsLocalContent((localContentType_t)0x10) && info.nChannelingFlag == 1)
		? m_pReciveUser->GetLine(1, false)
		: nick;
	if (m_pReciveUser)
	{
		m_bSearched = true;
		// 983 (0080e4f0): o campo do destinatario mostra o nick (+0x130) tambem; o id (+0x14c) so vai ao servidor
		m_pReciveUser->SetLine(1, m_receiverNick.c_str(), 0, false, 0);
		m_pUserNick->SetVisible(true);
	}
	if (m_pUserNick)
	{
		m_pUserNick->SetLine(1, m_receiverNick.c_str(), 0, false, 0);
	}
}

bool CPost_SendDlg::OnEmoticonResult(int result, FrForm* form)
{
	if (m_pEmoticonDlg && result == 1)
	{
		FrEmoticonDlg* pDlg = DYNAMIC_CAST(FrEmoticonDlg, form);
		if (pDlg)
		{
			const char* icon = pDlg->GetSelectedIcon();
			if (icon)
			{
				FrEdit* pEdit = DYNAMIC_CAST(FrEdit, FindChildByName("id"));
				if (m_pReciveUser)
				{
					const char* front = m_pReciveUser->GetEditText_Front();
					const char* comp = m_pReciveUser->GetEditText_Comp();
					const char* end = m_pReciveUser->GetEditText_End();
					m_pReciveUser->SetLine(1,
						MakeStr("%s%s%s%s", front, comp, icon, end), 0, false,
						0);
					m_pReciveUser->SetKeyFocus(true);
				}
			}
		}
	}
	m_pEmoticonDlg = NULL;
	return true;
}

std::string CPost_SendDlg::GetLetterMsg()
{
	static std::string s_msg;
	s_msg.clear();
	int lines = m_pReciveMsg->GetLineNum();
	for (int i = 1; i <= lines; i++)
	{
		s_msg += m_pReciveMsg->GetLine(i, false);
		if (i != lines)
			s_msg += "\n";
	}
	return s_msg;
}

std::list<sMailIncludeItem>* CPost_SendDlg::GetSendList()
{
	m_listSend.clear();
	sMailIncludeItem mail;
	memset(&mail, 0, sizeof(mail));
	std::list<sTradeItem>* pList =
		CTransferableItem::Instance()->GetList_RegistrationItem();
	std::list<sTradeItem>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{
		mail.dwIDX = it->dwGuid;
		mail.dwTID = it->dwTid;
		mail.iCount = it->iNum;
		mail.btItemType = (unsigned char)it->byItemType;
		mail.iTimeCount = it->wTime;
		m_listSend.push_back(mail);
	}
	return &m_listSend;
}

int CPost_SendDlg::GetSendListSize()
{
	return CTransferableItem::Instance()->Get_RegistrationListSize();
}

bool CPost_SendDlg::VerifySendItemList()
{
	std::list<sTradeItem>* pList =
		CTransferableItem::Instance()->GetList_RegistrationItem();
	std::list<sTradeItem>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{
		unsigned long tid = it->dwTid;
		sItemInfo* pInfo = Doc()->FindMyItemByGuid(it->dwGuid);
		if ((pInfo && pInfo->tid != tid) || it->iNum > 99)
			return false;
	}
	return true;
}

bool CPost_SendDlg::VerifyIncludeSameItem()
{
	std::vector<unsigned long> tids;
	std::list<sTradeItem>* pList =
		CTransferableItem::Instance()->GetList_RegistrationItem();
	std::list<sTradeItem>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
		tids.push_back(it->dwTid);
	unsigned int count = (unsigned int)tids.size() - 1;
	for (unsigned int i = 0; i < count; i++)
	{
		if (std::count(tids.begin(), tids.end(), tids[i]) > 1)
			return false;
	}
	return true;
}

int CPost_SendDlg::GetSendServiceCharge()
{
	if (CTransferableItem::Instance()->Get_RegistrationListSize() == 0)
		return 100;
	return CTransferableItem::Instance()->Get_RegistrationListSize() * 500;
}

IMPLEMENT_OBJECT(CPost_MyItemList, FrForm)

BEGIN_FRESH_MSGMAP(CPost_MyItemList, FrForm)
ON_FRESH_VI("myitem_list", FRCMD_INIT, CPost_MyItemList::OnPost_ItemListInit)
ON_FRESH_VI("myitem_list", FRCMD_OWNERDRAW,
	CPost_MyItemList::OnPost_ItemListOwnerDraw)
ON_FRESH_VV("myitem_list", FRCMD_LBUTTONDOWN,
	CPost_MyItemList::OnPost_ItemListBtnDown)
ON_FRESH_VV("myitem_list", FRCMD_RBUTTONDOWN,
	CPost_MyItemList::OnPost_ItemListRBtnDown)
ON_FRESH_VV("myitem_list", FRCMD_MOUSEMOVE,
	CPost_MyItemList::OnPost_ItemListMouseMove)
ON_FRESH_VV("myitem_list", FRCMD_DBLCLICK,
	CPost_MyItemList::OnPost_ItemListDClick)
ON_FRESH_VI("select_itemlist", FRCMD_INIT,
	CPost_MyItemList::OnPost_SelectListInit)
ON_FRESH_VI("select_itemlist", FRCMD_OWNERDRAW,
	CPost_MyItemList::OnPost_SelectListOwnerDraw)
ON_FRESH_VV("select_itemlist", FRCMD_LBUTTONDOWN,
	CPost_MyItemList::OnPost_SelectListBtnDown)
ON_FRESH_VV("select_itemlist", FRCMD_MOUSEMOVE,
	CPost_MyItemList::OnPost_SelectListMouseMove)
ON_FRESH_VV("select_itemlist", FRCMD_DBLCLICK,
	CPost_MyItemList::OnPost_SelectListDClick)
ON_FRESH_VI("select_item_img", FRCMD_INIT, CPost_MyItemList::OnPost_ImgSelect)
ON_FRESH_VI("Edit_array", FRCMD_INIT, CPost_MyItemList::OnPost_EditArrayInit)
ON_FRESH_VI("Btn_Array", FRCMD_INIT, CPost_MyItemList::OnPostBtnArrayInit)
ON_FRESH_VV("Btn_Array", FRCMD_LBUTTONUP,
	CPost_MyItemList::OnPostBtnArrayButtonUp)
END_FRESH_MSGMAP()

CPost_MyItemList::CPost_MyItemList()
{
	m_pParent = NULL;
	m_pCountDlg = 0;
	m_pSelected = 0;
	m_pBtnArray = NULL;
	m_pEditArray = NULL;
	m_pArrayDlg = 0;
	m_charFilter = -1;
	m_pSelectN = g_pFresh->GetBitmap("rmr_select_n");
	m_pSelectD = g_pFresh->GetBitmap("rmr_select_d");
	m_pLine = g_pFresh->GetBitmap("rmr_line");
	static char* s_fontWind[] = { "[font_wind.jpg" };
	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_fontWind;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
	static char* s_fontBong[] = { "[font_bong.jpg" };
	tagWTITLEFONT info2;
	memset(&info2, 0, sizeof(info2));
	info2.filename = s_fontBong;
	info2.fontw = 9;
	info2.fonth = 16;
	info2.numPages = 1;
	info2.texw = 128;
	info2.texh = 32;
	info2.pCharSet = "1234567890,-";
	m_pFontPang = g_resrcmng->GetTitleFont();
	m_pFontPang->Create(&info2);
	m_pUccOriginal = g_pFresh->RegisterBitmap("ucc_state_original");
	m_pUccCopied = g_pFresh->RegisterBitmap("ucc_state_copied");
	m_pUccTempSaved = g_pFresh->RegisterBitmap("ucc_state_temp_saved");
	m_cancelBtn.pNormal = g_pFresh->RegisterBitmap("rmr_visitor_book_cancel_n");
	m_cancelBtn.pOver = g_pFresh->RegisterBitmap("rmr_visitor_book_cancel_o");
	char* tex = *(char**)((char*)m_cancelBtn.pNormal + 4);
	unsigned long w = *(unsigned long*)(tex + 4);
	unsigned long h = *(unsigned long*)(tex + 8);
	m_cancelBtn.rect = WRect(52.0f, 60.0f, (float)w, (float)h);
}

CPost_MyItemList::~CPost_MyItemList()
{
	if (m_pFont)
		m_pFont->Erase();
	if (m_pFontPang)
		m_pFontPang->Erase();
	if (g_resrcmng)
	{
		if (m_pFont)
		{
			g_resrcmng->Release(m_pFont);
			m_pFont = NULL;
		}
		if (g_resrcmng && m_pFontPang)
		{
			g_resrcmng->Release(m_pFontPang);
			m_pFontPang = NULL;
		}
	}
}

void CPost_MyItemList::OnPost_ItemListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pItemList)
	{
		m_pItemList->SetMultiSelect(true);
	}
}

void CPost_MyItemList::OnPost_ItemListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	FrGraphicInterface* gdi;
	if (item && (gdi = g_pFresh->GetManager()->GetGDI(), item->pData))
	{
		const Bitmap* select;
		unsigned long color;
		if (item->selected)
		{
			select = m_pSelectD;
			color = 0xffff0000;
		}
		else
		{
			color = 0xff000000;
			if (item->underCursor)
				select = m_pSelectN;
			else
				select = NULL;
		}
		gdi->SetTextColor(color, 0xffffffff);
		sTradeItem* info = (sTradeItem*)item->pData;
		WRect client;
		GetClientRect(client);
		for (int row = 0; row < 2; ++row)
		{
			double y = row * 110.0 + 87.0;
			for (int col = 0; col < 3; ++col)
			{
				WRect rect;
				rect = WRect(col * 92.0 + 126.0 + client.x, client.y + y,
					(float)m_pLine->Width(), (float)m_pLine->Height());
				gdi->DrawTexture(m_pLine, rect, 0xffffffff, 0);
			}
		}
		if (select)
		{
			WRect rect(item->pos.x - 2.0f, item->pos.y, (float)select->Width(),
				(float)select->Height());
			gdi->DrawTexture(select, rect, 0xffffffff, 0);
		}
		if ((info->dwTid & 0xfc000000) == 0x8000000)
		{
			IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(
				((info->dwTid >> 18) & 0xff) | 0x4000000);
			if (pChar)
			{
				const Bitmap* face =
					g_pFresh->GetBitmap(MakeStr("s_%s", pChar->c.Icon));
				if (face)
				{
					WRect rect(item->pos.x + 7.0f, item->pos.y + 5.0f,
						(float)face->Width(), (float)face->Height());
					gdi->DrawTexture(face, rect, 0xffffffff, 0);
				}
			}
		}
		DrawItem(WPoint(item->pos.x + 13.0f, item->pos.y + 20.0f), info, 0);
		// 983 (0080d2d0): sem o desenho dos encaixes de carta (DrawPartsSlot)
	}
}

inline void CPost_MyItemList::DrawPartsSlot(FrGraphicInterface* gdi, FrListItem* item)
{
	if (0) // HACK: keeps the inline naked DrawPang emitted (nakedify)
		((CPost_MyItemList*)0)->DrawPang();
	if (item->pData && (gdi || (gdi = g_pFresh->GetManager()->GetGDI())))
	{
		float charX = 5.0f;
		float caddieX = 5.0f;
		sTradeItem* info = (sTradeItem*)item->pData;
		int slot;

		if (info->CharacterSlotNum <= -1 || info->CharacterSlotNum >= 5)
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(info->dwTid);
			if (!part)
			{
				info->CharacterSlotNum = 0;
			}
			else
			{
				info->CharacterSlotNum = part->CharacterSlot;
				if (info->CharacterSlotNum <= -1 || info->CharacterSlotNum >= 5)
					info->CharacterSlotNum = 0;
			}
		}
		for (slot = 1; slot < info->CharacterSlotNum + 1; ++slot)
		{
			bool found = false;
			for (std::list<sSCardAvilityPeriodInfo>::iterator it =
					 Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
			{
				if (info->dwTid == (*it).partsTid &&
					info->dwGuid == (*it).partsUid)
				{
					IFF_STRUCT::sCard* card =
						ItemManager()->FindCard((*it).tid);
					if (card && slot == (*it).slotNum && (*it).cardType == 0)
					{
						found = true;
						const Bitmap* bitmap =
							g_pFresh->GetBitmap(card->SlotImg);
						if (bitmap)
						{
							WRect rect(charX + item->pos.x, item->pos.y + 54.0f,
								(float)bitmap->Width(),
								(float)bitmap->Height());
							gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
							charX += 16.0f;
						}
					}
				}
			}
			if (!found)
			{
				const Bitmap* bitmap =
					g_pFresh->GetBitmap("card_character_empty");
				if (bitmap)
				{
					WRect rect(charX + item->pos.x, item->pos.y + 54.0f,
						(float)bitmap->Width(), (float)bitmap->Height());
					gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
					charX += 16.0f;
				}
			}
		}

		if (info->CaddieSlotNum <= -1 || info->CaddieSlotNum >= 5)
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(info->dwTid);
			if (!part)
			{
				info->CaddieSlotNum = 0;
			}
			else
			{
				info->CaddieSlotNum = part->CaddieSlot;
				if (info->CaddieSlotNum <= -1 || info->CaddieSlotNum >= 5)
					info->CaddieSlotNum = 0;
			}
		}
		for (slot = 1; slot < info->CaddieSlotNum + 1; ++slot)
		{
			bool found = false;
			for (std::list<sSCardAvilityPeriodInfo>::iterator it =
					 Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
			{
				if (info->dwTid == (*it).partsTid &&
					info->dwGuid == (*it).partsUid)
				{
					IFF_STRUCT::sCard* card =
						ItemManager()->FindCard((*it).tid);
					if (card && slot == (*it).slotNum && (*it).cardType == 1)
					{
						found = true;
						const Bitmap* bitmap =
							g_pFresh->GetBitmap(MakeStr(card->SlotImg));
						if (bitmap)
						{
							WRect rect(caddieX + item->pos.x,
								item->pos.y + 70.0f, (float)bitmap->Width(),
								(float)bitmap->Height());
							gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
							caddieX += 16.0f;
						}
					}
				}
			}
			if (!found)
			{
				const Bitmap* bitmap = g_pFresh->GetBitmap("card_caddie_empty");
				if (bitmap)
				{
					WRect rect(caddieX + item->pos.x, item->pos.y + 70.0f,
						(float)bitmap->Width(), (float)bitmap->Height());
					gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
					caddieX += 16.0f;
				}
			}
		}
	}
}

void CPost_MyItemList::OnPost_ItemListBtnDown()
{
	FrListItem* pListItem = m_pItemList->GetItemUnderCursor();
	if (pListItem)
	{
		sTradeItem* pItem = (sTradeItem*)pListItem->pData;
		if (pItem == NULL)
		{
			CallErrorMsg(K2L_Compatibility(
				"\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9."));
			return;
		}
		if (pListItem->selected == false)
		{
			pListItem->selected = true;
			CallErrorMsg(K2L_Compatibility(
				"\xc0\xcc\xb9\xcc \xc3\xdf\xb0\xa1\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9."));
			return;
		}
		if (m_pSelectList->FindItem(pListItem))
		{
			pListItem->selected = false;
			CallErrorMsg(K2L_Compatibility(
				"\xb0\xb0\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc3\xdf\xb0\xa1\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
			return;
		}
		if (CTransferableItem::Instance()->Get_RegistrationListSize() > 3)
		{
			pListItem->selected = false;
			CallErrorMsg(K2L_Compatibility(
				"\xc0\xcc\xb9\xcc 4\xb0\xb3\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb8\xf0\xb5\xce \xbc\xb1\xc5\xc3\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."));
			return;
		}
		if (!CTransferableItem::Instance()->IsCheckCardParts(pItem->dwGuid,
				pItem->dwTid))
		{
			pListItem->selected = false;
			CallErrorMsg(K2L_Compatibility(
				"\xc4\xab\xb5\xe5\xb0\xa1 \xc0\xce\xc3\xa6\xc6\xae \xb5\xc8 \xc0\xc7\xbb\xf3\xc0\xba \xc7\xf6\xc0\xe7 \xbc\xb1\xb9\xb0 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
			return;
		}
		if ((pItem->dwTid & 0xfc000000) == 0x7c000000)
		{
			pListItem->selected = false;
			CallErrorMsg(K2L_Compatibility(
				"\xc4\xab\xb5\xe5\xb4\xc2 \xc7\xf6\xc0\xe7 \xbc\xb1\xb9\xb0 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
			return;
		}
		if (pItem->iNum < 2)
		{
			if (CTransferableItem::Instance()->SetRegistrationItem(
					pItem->dwGuid, 1))
			{
				if (!m_pImgSelect->IsVisible())
					m_pImgSelect->SetVisible(true);
				if (!m_pSelectList->IsVisible())
					m_pSelectList->SetVisible(true);
				SetList();
				return;
			}
			pListItem->selected = false;
			CallErrorMsg(K2L_Compatibility(
				"\xbf\xec\xc6\xed\xbf\xa1 \xc6\xf7\xc7\xd4\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xc5\xb1\xe2\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."));
		}
		else if (m_pCountDlg == NULL)
		{
			m_pSelected = pListItem;
			m_pCountDlg = CreateForm<CPost_ItemCountDlg>(g_pFresh->GetManager(),
				this, "post_itemcount", NULL);
			WRect rect;
			rect.w = m_pCountDlg->GetRect().w;
			rect.h = m_pCountDlg->GetRect().h;
			rect.x = pListItem->pos.x - 20.0f;
			rect.y = pListItem->pos.y + 80.0f;
			m_pCountDlg->SetRect(rect);
			m_pCountDlg->SetSelectItem(pItem->dwGuid, pItem->iNum);
			rect.h = 0;
			m_pCountDlg->Open(
				(FRESH_PFN_RESULT)&CPost_MyItemList::OnPostItemCountResult,
				0x81);
		}
	}
}

// ---- CPost_MyItemList ----

void CPost_MyItemList::OnPost_ItemListMouseMove()
{
}

void CPost_MyItemList::OnPost_ItemListRBtnDown()
{
	if (0)
		((CPost_MyItemList*)0)->CPost_MyItemList::OnPost_ItemListBtnDown();
}

void CPost_MyItemList::OnPost_ItemListDClick()
{
}

void CPost_MyItemList::OnPost_ImgSelect(int param)
{
	m_pImgSelect = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pImgSelect)
	{
		m_pImgSelect->SetVisible(false);
	}
}

void CPost_MyItemList::OnPost_SelectListInit(int param)
{
	m_pSelectList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pSelectList)
	{
		m_pSelectList->SetVisible(false);
	}
}

void CPost_MyItemList::OnPost_EditArrayInit(int param)
{
	m_pEditArray = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pEditArray)
	{
		m_pEditArray->ClearLine();
		m_pEditArray->AddText(K2L_Compatibility("\xc0\xfc \xc3\xbc"), false, true);
	}
}

void CPost_MyItemList::OnPost_SelectListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (item == NULL)
		return;
	sTradeItem* info = (sTradeItem*)item->pData;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (info == NULL)
		return;
	if ((info->dwTid & 0xfc000000) == 0x8000000)
	{
		IFF_STRUCT::sChar* pChar =
			ItemManager()->FindChar(((info->dwTid >> 18) & 0xff) | 0x4000000);
		if (pChar)
		{
			const Bitmap* face =
				g_pFresh->GetBitmap(MakeStr("s_%s", pChar->c.Icon));
			if (face)
			{
				WRect rect(item->pos.x + 3.0f, item->pos.y + 3.0f,
					(float)face->Width(), (float)face->Height());
				gdi->DrawTexture(face, rect, 0xffffffff, 0);
			}
		}
	}
	WPoint pos;
	if (!item->underCursor)
	{
		pos.y = item->pos.y + 7.0f;
		pos.x = item->pos.x + 5.0f;
	}
	else
	{
		pos.y = item->pos.y + 5.0f;
		pos.x = item->pos.x + 3.0f;
	}
	DrawItem(pos, info, 0);
	const Bitmap* cancel = m_cancelBtn.pNormal;
	if (item->underCursor || item->selected)
	{
		WRect rect(item->pos.x + m_cancelBtn.rect.x,
			item->pos.y + m_cancelBtn.rect.y, m_cancelBtn.rect.w,
			m_cancelBtn.rect.h);
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			cancel = m_cancelBtn.pOver;
	}
	WRect rect(item->pos.x + m_cancelBtn.rect.x,
		item->pos.y + m_cancelBtn.rect.y, m_cancelBtn.rect.w,
		m_cancelBtn.rect.h);
	gdi->DrawTexture(cancel, rect, 0xffffffff, 0);
}

void CPost_MyItemList::OnPost_SelectListBtnDown()
{
	FrListItem* pListItem = m_pSelectList->GetItemUnderCursor();
	if (pListItem)
	{
		WRect rect(m_cancelBtn.rect.x + pListItem->pos.x,
			pListItem->pos.y + m_cancelBtn.rect.y, m_cancelBtn.rect.w,
			m_cancelBtn.rect.h);
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		{
			sTradeItem* pItem = (sTradeItem*)pListItem->pData;
			if (pItem == NULL)
			{
				CallErrorMsg(K2L_Compatibility(
					"\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9."));
				return;
			}
			if (!CTransferableItem::Instance()->DelRegistrationItem(
					pItem->dwGuid))
			{
				pListItem->selected = true;
				CallErrorMsg(K2L_Compatibility(
					"\xbe\xc6\xc0\xcc\xc5\xdb \xbc\xb1\xc5\xc3 \xc3\xeb\xbc\xd2 \xbd\xc7\xc6\xd0"));
				return;
			}
			if (CTransferableItem::Instance()->Get_RegistrationListSize() == 0)
			{
				m_pSelectList->ClearItem();
				m_pSelectList->SetVisible(false);
				m_pImgSelect->SetVisible(false);
			}
			SetList();
		}
	}
}

void CPost_MyItemList::OnPost_SelectListMouseMove()
{
}

void CPost_MyItemList::OnPost_SelectListDClick()
{
}

void CPost_MyItemList::OnPostBtnArrayInit(int param)
{
	m_pBtnArray = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void CPost_MyItemList::OnPostBtnArrayButtonUp()
{
	const WRect& rect = m_pBtnArray->GetRect();
	float x = rect.x;
	float y = rect.y;
	float h = rect.h;
	if (m_pArrayDlg == NULL)
	{
		m_pArrayDlg = CreateForm<CStandingInLine>(g_pFresh->GetManager(), this,
			"arraydlg", NULL);
		if (m_pArrayDlg)
		{
			WPoint pos;
			pos.x = x;
			pos.y = h + y;
			m_pArrayDlg->Open(
				(FRESH_PFN_RESULT)&CPost_MyItemList::OnPostItemArrayResult, pos,
				1);
		}
	}
}

void CPost_MyItemList::DrawItem(WPoint pos, sTradeItem* item,
	unsigned char selected)
{
	if (item == NULL)
		return;
	unsigned long tid = item->dwTid;
	int num = item->iNum;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi == NULL)
		return;
	if (tid == 0)
		return;
	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
	if (pCommon == NULL)
		return;
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	const Bitmap* icon = g_pFresh->GetBitmap(pCommon->Icon);
	if (IsLocalContent(S4_UCC) && (item->status & 1) &&
		IsUccClothes(item->dwTid))
	{
		const Bitmap* ucc = GetUccItemIcon(item->dwGuid);
		if (ucc)
			icon = ucc;
	}
	if ((tid >> 26) == 2)
	{
		offsetX = -12.0f;
		offsetY = -20.0f;
	}
	if (icon)
	{
		WRect rect(pos.x + offsetX + 14.0f, pos.y + offsetY + 11.0f,
			(float)icon->Width(), (float)icon->Height());
		gdi->DrawTexture(icon, rect, 0xffffffff, 0);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(4);
		if (IsLocalContent(S4_UCC) && IsUccClothes(item->dwTid))
		{
			unsigned char st = item->status;
			if ((st & 1) && item->Seq == 1 && m_pUccOriginal)
			{
				gdi->DrawTexture(m_pUccOriginal,
					WRect(pos.x + 3.0f, pos.y + 15.0f,
						(float)m_pUccOriginal->Width(),
						(float)m_pUccOriginal->Height()),
					0xffffffff, 0);
			}
			else if ((st & 1) && item->Seq > 1 && m_pUccCopied)
			{
				gdi->DrawTexture(m_pUccCopied,
					WRect(pos.x + 3.0f, pos.y + 15.0f,
						(float)m_pUccCopied->Width(),
						(float)m_pUccCopied->Height()),
					0xffffffff, 0);
			}
			else if ((st & 2) && m_pUccTempSaved)
			{
				gdi->DrawTexture(m_pUccTempSaved,
					WRect(pos.x + 3.0f, pos.y + 15.0f,
						(float)m_pUccTempSaved->Width(),
						(float)m_pUccTempSaved->Height()),
					0xffffffff, 0);
			}
		}
		if (num > 1)
		{
			const char* count = MakeStr("%d", num);
			float x = pos.x + 54.0f;
			for (num = 0; num < strlen(count); ++num)
				x -= m_pFont->GetCharWidth(g_view, count[num] - '0');
			m_pFont->Print(g_view, x, pos.y + 42.0f, count, 0, 0xffffffff,
				NULL);
		}
	}
	switch (tid >> 26)
	{
	case 2:
	case 4:
	case 7:
	case 0x10:
	case 0x1c:
		const Bitmap* level = g_pFresh->GetBitmap(
			MakeStr("level_%03d", (pCommon->Level & 0x7f) + 1));
		if (level)
		{
			WRect rect(pos.x + 38.0f - float2int(level->Width() * 0.5f),
				pos.y + 35.0f - float2int(level->Height() * 0.5f),
				(float)level->Width(), (float)level->Height());
			gdi->DrawTexture(level, rect, 0xffffffff, 0);
		}
	}
}

void CPost_MyItemList::DrawPang()
{
	if (g_pFresh->GetManager()->GetGDI() && m_pFontPang)
	{
		float x = m_rect.x;
		char text[32] = { 0 };
		float y = m_rect.y;
		static int fontW = m_pFontPang->GetFontInfo()->fontw;
		static float width;
		MakePriceStrA(text, Doc()->m_myInfo.stat.i64Pang);
		width = ((float)fontW - 1.0f) * (float)(strlen(text) - 1);
		m_pFontPang->Print(g_view, (x + 130.0f) - width, y + 483.0f, text, 0,
			0xffffffff, NULL);
	}
}

void CPost_MyItemList::OnOK()
{
	FrForm::OnOK();
}

bool CPost_MyItemList::OnInit()
{
	SetList();
	return true;
}

// visao local do CSharedDoc: 983 00636850 (metodo fora de linha, nome meu): guid presente na lista apontada por
// doc+0x111d8 (no do std::list: guid em +0x10)
class CSharedDocPost983 : public CSharedDoc
{
public:
	bool IsInPostBlockList983(unsigned long guid);
};

bool CSharedDocPost983::IsInPostBlockList983(unsigned long guid)
{
	struct Node983
	{
		Node983* next;
		Node983* prev;
		unsigned long d0;
		unsigned long d1;
		unsigned long guid;
	};
	// D871: era this+0x111d8 cru (VC10); no cl 14.44 o CSharedDoc anda (STL menor, A728) -> ancorado no membro
	C_ASSERT_VC10(offsetof(CSharedDoc, m_listTradeItem) == 0x111d8);
	Node983* head = *(Node983**)((char*)this + offsetof(CSharedDoc, m_listTradeItem));
	for (Node983* n = head->next; n != head; n = n->next)
	{
		if (n->guid == guid)
			return true;
	}
	return false;
}

void CPost_MyItemList::SetList()
{
	m_pItemList->ClearItem();
	m_pSelectList->ClearItem();
	std::list<sTradeItem>* pTransferable =
		CTransferableItem::Instance()->GetList_TransferableItem();
	std::list<sTradeItem>* pRegistration =
		CTransferableItem::Instance()->GetList_RegistrationItem();
	std::list<sTradeItem>::iterator it;
	for (it = pTransferable->begin(); it != pTransferable->end(); ++it)
	{
		if (((*it).dwTid & 0xfc000000) == 0x7c000000)
			continue;
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).dwTid);
		if (pCommon && !pCommon->IsSalable)
			continue;
		if (Doc()->IsEquipParts((*it).dwGuid, (*it).dwTid))
			continue;
		if (((CSharedDocPost983*)Doc())->IsInPostBlockList983((*it).dwGuid))
			continue;

		FrListItem* pListItem = NULL;
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart((*it).dwTid);
			if (pPart)
			{
				(*it).CharacterSlotNum = pPart->CharacterSlot;
				(*it).CaddieSlotNum = pPart->CaddieSlot;
				(*it).NpcSlotNum983 = pPart->NpcSlot983; // 983 0080dbf0: +0x6b <- sPart+0x22c
			}
			else
			{
				(*it).CharacterSlotNum = 0;
				(*it).CaddieSlotNum = 0;
				(*it).NpcSlotNum983 = 0;
			}
		}

		bool bAdd = false;
		int filter = m_charFilter;
		unsigned long tid = (*it).dwTid;
		if (filter == -1)
		{
			// 983: fora da lista de habilidades de carta (partsUid == guid)
			if ((tid & 0xfc000000) != 0x7c000000)
			{
				std::list<sSCardAvilityPeriodInfo>& cards =
					Doc()->m_cardAbilityList[0];
				std::list<sSCardAvilityPeriodInfo>::iterator itCard;
				bAdd = true;
				for (itCard = cards.begin(); itCard != cards.end(); ++itCard)
				{
					if (itCard->partsUid == (*it).dwGuid)
					{
						bAdd = false;
						break;
					}
				}
			}
		}
		else if (filter >= 0 && filter + 1 < 15)
		{
			unsigned int type = tid >> 26;
			if (type == 2 || type == 1 || type == 9)
			{
				int want = (filter == 13) ? 14 : filter;
				bAdd = (int)((tid >> 18) & 0xff) == want;
			}
		}
		else if (filter == 14)
		{
			bAdd = (tid & 0xfc000000) == 0x18000000;
		}
		else if (filter == 15)
		{
			unsigned int type = tid >> 26;
			bAdd = type == 7 || type == 8 || type == 0x1c || type == 0x10 ||
				type == 0xe;
		}

		if (bAdd)
		{
			pListItem = m_pItemList->AddItem(&*it);
			if (pListItem && m_charFilter == -1)
			{
				std::list<sTradeItem>::iterator itReg;
				for (itReg = pRegistration->begin();
					itReg != pRegistration->end(); ++itReg)
				{
					if (((sTradeItem*)pListItem->pData)->dwGuid ==
						itReg->dwGuid)
					{
						if (!m_pSelectList->IsVisible())
						{
							m_pSelectList->SetVisible(true);
							m_pImgSelect->SetVisible(true);
						}
						pListItem->selected = true;
						break;
					}
				}
				continue;
			}
		}

		std::list<sTradeItem>::iterator itReg;
		for (itReg = pRegistration->begin(); itReg != pRegistration->end();
			++itReg)
		{
			if ((*it).dwGuid == itReg->dwGuid)
			{
				if (!m_pSelectList->IsVisible())
				{
					m_pSelectList->SetVisible(true);
					m_pImgSelect->SetVisible(true);
				}
				if (pListItem)
					pListItem->selected = true;
				break;
			}
		}
	}
	std::list<sTradeItem>::iterator itReg;
	for (itReg = pRegistration->begin(); itReg != pRegistration->end(); ++itReg)
		m_pSelectList->AddItem(&*itReg);
}

void CPost_MyItemList::SetParent(CPost_SendDlg* pParent)
{
	m_pParent = pParent;
}

bool CPost_MyItemList::OnPostItemCountResult(int result, FrForm* form)
{
	if (result == 1)
	{
		m_registerCount = 0;
		m_registerCount = m_pCountDlg->GetCount();
		if (CTransferableItem::Instance()->SetRegistrationItem(
				m_pCountDlg->GetSelectItem(), m_registerCount))
		{
			if (!m_pImgSelect->IsVisible())
				m_pImgSelect->SetVisible(true);
			if (!m_pSelectList->IsVisible())
				m_pSelectList->SetVisible(true);
			SetList();
			m_pCountDlg = NULL;
			return true;
		}
		m_pSelected->selected = false;
		CallErrorMsg(K2L_Compatibility( // 983 0080e95a
			"\xbe\xc6\xc0\xcc\xc5\xdb \xbc\xb1\xc5\xc3 \xc3\xeb\xbc\xd2 \xbd\xc7\xc6\xd0"));
		m_pCountDlg = NULL;
		return true;
	}
	m_pSelected->selected = false;
	CallErrorMsg(K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb \xbc\xb1\xc5\xc3 \xc3\xeb\xbc\xd2"));
	m_pCountDlg = NULL;
	return true;
}

bool CPost_MyItemList::OnPostItemArrayResult(int result, FrForm* form)
{
	if (result == 1)
	{
		m_charFilter = m_pArrayDlg->GetSelectNum() - 1;
		m_pEditArray->ClearLine();
		if (m_charFilter == -1)
			m_pEditArray->AddText(K2L_Compatibility("\xc0\xfc \xc3\xbc"), false, true);
		else
			m_pEditArray->AddText(MakeStr("%s", K2L_Compatibility(
				CreateCharacterOption::Instance()->GetName983(m_charFilter))),
				false, true);
		SetList();
	}
	m_pArrayDlg = NULL;
	return true;
}

// ---- CPost_ItemCountDlg ----

CPost_ItemCountDlg::CPost_ItemCountDlg()
{
	m_pMinus = 0;
	m_pPluse = 0;
	m_pEditCount = 0;
	m_count = 1;
}

CPost_ItemCountDlg::~CPost_ItemCountDlg()
{
}

IMPLEMENT_OBJECT(CPost_ItemCountDlg, FrForm)

BEGIN_FRESH_MSGMAP(CPost_ItemCountDlg, FrForm)
ON_FRESH_VI("btn_minus", FRCMD_INIT, CPost_ItemCountDlg::OnPostMinusInit)
ON_FRESH_VV("btn_minus", FRCMD_LBUTTONUP, CPost_ItemCountDlg::OnPostMinusBtnUp)
ON_FRESH_VI("btn_pluse", FRCMD_INIT, CPost_ItemCountDlg::OnPostPluseInit)
ON_FRESH_VV("btn_pluse", FRCMD_LBUTTONUP, CPost_ItemCountDlg::OnPostPluseBtnUp)
ON_FRESH_VI("input_count", FRCMD_INIT,
	CPost_ItemCountDlg::OnPostEditItemCountInit)
ON_FRESH_BI("input_count", FRCMD_ENTERKEY,
	CPost_ItemCountDlg::OnPostEditItemCountEnterKey)
END_FRESH_MSGMAP()

void CPost_ItemCountDlg::OnPostMinusInit(int param)
{
	m_pMinus = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMinus)
	{
		m_pMinus->SetPushDelay(0.0f);
	}
}

void CPost_ItemCountDlg::OnPostMinusBtnUp()
{
	m_count = atoi(m_pEditCount->GetLine(1, false)) - 1;
	if (m_count < 1)
	{
		m_count = 1;
	}
	SetText();
}

void CPost_ItemCountDlg::OnPostPluseInit(int param)
{
	m_pPluse = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pPluse)
	{
		m_pPluse->SetPushDelay(0.0f);
	}
}

void CPost_ItemCountDlg::OnPostPluseBtnUp()
{
	m_count = atoi(m_pEditCount->GetLine(1, false)) + 1;
	if (m_count > m_maxCount)
	{
		m_count = m_maxCount;
	}
	SetText();
}

void CPost_ItemCountDlg::OnPostEditItemCountInit(int param)
{
	m_pEditCount = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pEditCount)
	{
		m_pEditCount->SetCharLimit(2, true);
	}
	SetText();
}

int CPost_ItemCountDlg::GetCount()
{
	if (0)
		((CPost_ItemCountDlg*)0)->CPost_ItemCountDlg::OnPostPluseInit(*(int*)0);
	int count = atoi(m_pEditCount->GetLine(1, false));
	int maxCount = m_maxCount;
	m_count = count;
	if (count < 1)
	{
		count = 1;
	}
	else if (count > maxCount)
	{
		m_count = maxCount;
		return maxCount;
	}
	m_count = count;
	return count;
}

void CPost_ItemCountDlg::SetText()
{
	if (0)
		((CPost_ItemCountDlg*)0)->CPost_ItemCountDlg::GetCount();
	if (m_pEditCount)
	{
		m_pEditCount->ClearLine();
		m_pEditCount->AddText(MakeStr(K2L_Compatibility("%d"), m_count), false, true);
	}
}

void CPost_ItemCountDlg::SetSelectItem(unsigned long item, int maxCount)
{
	m_selectItem = item;
	m_maxCount = maxCount;
}

unsigned long CPost_ItemCountDlg::GetSelectItem()
{
	return m_selectItem;
}

bool CPost_ItemCountDlg::OnPostEditItemCountEnterKey(int param)
{
	OnOK();
	return true;
}

