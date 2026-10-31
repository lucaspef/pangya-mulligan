#include "minatl.h"
#include "itembuffactor.h"
#include "projectg.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "cardsystem.h"
#include "carddlg.h"
#include "tooltip.h"
#include "packet.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frlistbox.h"
#include "frbutton.h"
#include "frarea.h"
#include "fredit.h"
#include <algorithm>

extern Fresh* g_pFresh;

// HACK: realmyroommain owns this shared literal in the original image.
extern "C" const char rb_myroomItemBuffName[];

struct sAbilityFinder
{
	unsigned long m_typeId;

	sAbilityFinder(unsigned long typeId) { m_typeId = typeId; }

	bool operator()(sSCardAvilityPeriodInfo info)
	{
		return m_typeId == info.tid;
	}
};

namespace
{

	// 983 e1e770 (35 entradas, inicializador 00cdb2c0): [2] e [32] por K2L
	const char* g_buffName[35] = {
		"",
		"",
		K2L_Compatibility("\xc8\xb9\xb5\xe6 \xb0\xe6\xc7\xe8\xc4\xa1"),
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		K2L_Compatibility("\xc8\xb9\xb5\xe6 \xb0\xe6\xc7\xe8\xc4\xa1\xbf\xcd \xc6\xce"),
		"",
		"",
	};

	const char* MakeBuffMessage(const char* title, int duration,
		specialcardavility type, int value)
	{
		char* period = MakeStr(K2L_Compatibility("\xb1\xe2\xb0\xa3 : %d\xba\xd0"), duration);
		char* effect =
			MakeStr(K2L_Compatibility("\xc8\xbf\xb0\xfa : %s +%d%%"), g_buffName[type], value);

		return MakeStr("%s\n\\c0xff0000ff\\c%s\n%s\\c0xff000000\\c", title,
			period, effect);
	}

	FrForm* OpenResultDlg(unsigned long typeId, FrCmdTarget* pTarget)
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), pTarget,
			"notify_item", NULL);

		WRect rect = pForm->GetRect();
		rect.w += 55.0f;
		rect.h += 40.0f;
		pForm->SetRect(rect);

		FrButton* pOk = DYNAMIC_CAST(FrButton, pForm->FindChildByName("ok"));
		if (pOk)
		{
			WRect rc = pOk->GetRect();
			rc.x = (rect.w - rc.w) * 0.5f;
			rc.y = rect.h - 80.0f;
			pOk->SetClientRect(rc);
		}

		FrEdit* pMessage =
			DYNAMIC_CAST(FrEdit, pForm->FindChildByName("message"));
		if (pMessage)
		{
			WRect rc = pMessage->GetRect();
			rc.w += 55.0f;
			rc.h += 40.0f;
			pMessage->SetRect(rc);
		}

		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeId);

		specialcardavility type;
		int value;
		int duration;
		int accum;
		const char* text = "";

		if (ItemManager()->GetBuffProperty(typeId, type, value, duration,
				accum))
		{
			text = MakeBuffMessage(
				K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xcf\xbf\xa9 "
				"\xb4\xd9\xc0\xbd \xc8\xbf\xb0\xfa\xb8\xa6 "
				"\xbe\xf2\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9!"),
				duration, type, value);
		}

		FrArea* pIcon =
			DYNAMIC_CAST(FrArea, pForm->FindChildByName("itemicon"));
		FrEdit* pName =
			DYNAMIC_CAST(FrEdit, pForm->FindChildByName("itemname"));

		if (pIcon)
		{
			if (pItem)
				pIcon->SetBgImg(pItem->Icon);
			else
				pIcon->SetBgImg("hide");
		}

		if (pName)
		{
			if (pItem)
				pName->AddText(pItem->Name, false, true);
			else
				pName->AddText("none", false, true);

			pName->AddText("\n", false, true);
		}

		pForm->SetMessage(text, false);
		pForm->Open(NULL, 1);

		return pForm;
	}

	FrForm* OpenConfirmDlg(unsigned long typeId, FrCmdTarget* pTarget,
		FRESH_PFN_RESULT pFnResult)
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), pTarget,
			"notify_item_yesno", NULL);

		WRect rect = pForm->GetRect();
		rect.w += 55.0f;
		rect.h += 40.0f;
		pForm->SetRect(rect);

		FrButton* pYes = DYNAMIC_CAST(FrButton, pForm->FindChildByName("yes"));
		if (pYes)
		{
			WRect rc = pYes->GetRect();
			rc.x = 55.0f;
			rc.y = rect.h - 90.0f;
			pYes->SetClientRect(rc);
		}

		FrButton* pNo = DYNAMIC_CAST(FrButton, pForm->FindChildByName("no"));
		if (pNo)
		{
			WRect rc = pNo->GetRect();
			rc.x = rect.w - rc.w - 55.0f;
			rc.y = rect.h - 90.0f;
			pNo->SetClientRect(rc);
		}

		FrEdit* pMessage =
			DYNAMIC_CAST(FrEdit, pForm->FindChildByName("message"));
		if (pMessage)
		{
			WRect rc = pMessage->GetRect();
			rc.w += 55.0f;
			rc.h += 40.0f;
			pMessage->SetRect(rc);
		}

		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeId);

		specialcardavility type;
		int value;
		int duration;
		int accum;
		const char* text = "";

		if (ItemManager()->GetBuffProperty(typeId, type, value, duration,
				accum))
		{
			text = MakeBuffMessage( // 983 00812fed: titulo por K2L
				K2L_Compatibility("\xc0\xcc \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb "
				"\xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
				duration, type, value);
		}

		FrArea* pIcon =
			DYNAMIC_CAST(FrArea, pForm->FindChildByName("itemicon"));
		FrEdit* pName =
			DYNAMIC_CAST(FrEdit, pForm->FindChildByName("itemname"));

		if (pIcon)
		{
			if (pItem)
				pIcon->SetBgImg(pItem->Icon);
			else
				pIcon->SetBgImg("hide");
		}

		if (pName)
		{
			if (pItem)
				pName->AddText(pItem->Name, false, true);
			else
				pName->AddText("none", false, true);

			pName->AddText("\n", false, true);
		}

		pForm->SetMessage(text, false);
		pForm->Open(pFnResult, 1);

		return pForm;
	}

} // namespace

IMPLEMENT_ACTOR(CItemBuff, IActor)

std::list<sSCardAvilityPeriodInfo> CItemBuff::ms_buffList;

CItemBuff::CItemBuff()
{
	m_pOverBmp = NULL;
	m_pNormalBmp = NULL;
	m_typeId = 0;
}

void CItemBuff::OnLoad()
{
	m_pNormalBmp = g_pFresh->RegisterBitmap("trade_item_edit_bn_normal");
	m_pOverBmp = g_pFresh->RegisterBitmap("trade_item_edit_bn_over");
	m_rect =
		WRect(103.0f, 66.0f, m_pNormalBmp->Width(), m_pNormalBmp->Height());
}

void CItemBuff::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}

	switch (msg.message)
	{
	case M983(0x246):
		HandleBuffItemSubMsg(msg);
		break;

	case M983(0x26):
		OpenResultDlg(msg.param1, this);
		break;
	}
}

void CItemBuff::HandleBuffItemSubMsg(const MsgObject& msg)
{
	switch (msg.param1)
	{
	case 0:
	{
		unsigned long typeId = msg.param5;
		bool* pHandled = (bool*)msg.param2;
		if (ItemManager()->IsItemBuff(typeId))
			*pHandled = true;

		if (*pHandled)
			DrawMiniButton((FrListItem*)msg.param3, *(const WPoint*)msg.param4);
	}
	break;

	case 1:
	{
		unsigned long typeId = msg.param5;
		bool* pHandled = (bool*)msg.param2;
		if (ItemManager()->IsItemBuff(typeId))
			*pHandled = true;

		if (*pHandled)
			ButtonDown((FrListItem*)msg.param3, *(const WPoint*)msg.param4);
	}
	break;

	case 2:
	{
		bool* pHandled = (bool*)msg.param2;
		if (TOOLTIP()->GetCurTipIndex() == 0x4c) // 983 0081398d (0x46 do 645; DrawMiniButton grava 0x4c)
			*pHandled = true;
	}
	break;
	}
}

inline WRect WRect::operator+(WPoint& p) const
{
	return WRect(x + p.x, y + p.y, w, h);
}

void CItemBuff::DrawMiniButton(FrListItem* pItem, const WPoint& point)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	CItemManager* pItemManager = ItemManager();

	if (pItem->underCursor && (m_rect + pItem->pos).IsInRect(point))
	{
		pGDI->DrawTexture(m_pOverBmp, m_rect + pItem->pos, 0xffffffff, 0);
		TOOLTIP()->SetToolTip((eBarButton)0x4c);
	}
	else
	{
		pGDI->DrawTexture(m_pNormalBmp, m_rect + pItem->pos, 0xffffffff, 0);
	}
}

void CItemBuff::ButtonDown(FrListItem* pItem, const WPoint& point)
{
	if (pItem &&
		WRect(pItem->pos.x + m_rect.x, pItem->pos.y + m_rect.y, m_rect.w,
			m_rect.h)
			.IsInRect(point))
	{
		UseBuffItem((sItemInfo*)pItem->pData);
	}
}

void CItemBuff::UseBuffItem(sItemInfo* pItem)
{
	if (!pItem)
		return;

	if (!ItemManager()->FindCommonItem(pItem->tid))
		return;

	m_typeId = pItem->tid;

	bool bCompatible = true;
	unsigned long conflictId = 0;

	IsCompatibleBuff(m_typeId, ms_buffList, bCompatible, conflictId);
	IsCompatibleBuff(m_typeId, Doc()->m_cardAbilityList[0], bCompatible,
		conflictId);

	if (!bCompatible)
	{
		IFF_ITEM_COMMON* pConflict = ItemManager()->FindCommonItem(conflictId);
		char* text;
		if (pConflict)
		{
			text = MakeStr("%s\n%s",
				K2L_Compatibility("\xb4\xd9\xc0\xbd \xbe\xc6\xc0\xcc\xc5\xdb\xb0\xfa "
				"\xc7\xd4\xb2\xb2 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 "
				"\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				pConflict->Name);
		}
		else
		{
			text = MakeStr("%s",
				K2L_Compatibility("\xc7\xd4\xb2\xb2 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 "
				"\xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc "
				"\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
		}

		// 983 008135e0: aviso ao ator principal da tarefa (o 983 nao tem o ator "RealMyRoom")
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)text, 0, 0, 0, 0));

		m_typeId = 0;
		return;
	}

	OpenConfirmDlg(pItem->tid, this,
		(FRESH_PFN_RESULT)&CItemBuff::OnUseBuffItemDlgResult);
}

void CItemBuff::IsCompatibleBuff(unsigned long typeId,
	const std::list<sSCardAvilityPeriodInfo>& buffList, bool& bCompatible,
	unsigned long& conflictId)
{
	std::list<sSCardAvilityPeriodInfo>::const_iterator it;
	for (it = buffList.begin(); bCompatible && it != buffList.end(); ++it)
	{
		if (!ItemManager()->IsCompatibleBuff(typeId, (*it).tid))
		{
			conflictId = (*it).tid;
			bCompatible = false;
		}
	}
}

bool CItemBuff::OnUseBuffItemDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (m_typeId)
		{
			// 983 008133a0: espera no ator principal da tarefa (sem "RealMyRoom")
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));

			WSendPacket packet((enumClientPacket)0xd8);
			packet.Encode4(m_typeId);
			packet.Send(TO_GAME);
		}
	}

	m_typeId = 0;

	return true;
}

void CItemBuff::ResetBuffList()
{
	ms_buffList.clear();
}

// PKT (frente C, lote 55): 983 00813a70 - o pacote do buff e o 0x181 (0x186 no 645); no caso 2 os avisos vao ao ator principal
// da tarefa (o 983 nao tem o ator "RealMyRoom"): 2, 0xca (so do 983) e M983(0x219) com o typeId.
void CItemBuff::ProcessPacket(WReceivedPacket& packet)
{
	sSCardAvilityPeriodInfo info;

	unsigned short id = packet.Decode2();
	if (id != 0x181)
		return;

	unsigned long type = packet.Decode4();
	switch (type)
	{
	case 0:
	{
		unsigned char count = packet.Decode1();
		for (int i = 0; i < count; i++)
		{
			packet.DecodeBuffer(&info, sizeof(info));
			AddBuffItem(info);
		}
	}
	break;

	case 1:
	{
		unsigned long typeId = packet.Decode4();
		RemoveBuffItem(typeId);
	}
	break;

	case 2:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long typeId = 0;
		int result = packet.Decode4();
		if (result == 1)
		{
			packet.Decode4();

			packet.DecodeBuffer(&info, sizeof(info));
			AddBuffItem(info);
			typeId = info.tid;

			CCardManager::Instance()->SetSPCardAlarm(true);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xca, 0, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x219), typeId, 1, 0, 0, 0));
		}

		AfxGetTask()->GetActor(rb_myroomItemBuffName)
			<< MsgObject(NULL, M983(0x26), typeId, 0, 0, 0, 0);
	}
	break;
	}
}

void CItemBuff::AddBuffItem(sSCardAvilityPeriodInfo& info)
{
	if (!ItemManager()->IsItemBuff(info.tid))
		return;

	std::list<sSCardAvilityPeriodInfo>::iterator it = std::find_if(
		ms_buffList.begin(), ms_buffList.end(), sAbilityFinder(info.tid));

	if (it != ms_buffList.end())
	{
		(*it).useEndTime = info.useEndTime;
		(*it).useStartTime.wSecond = HIWORD(g_CurrentTime);
		(*it).useStartTime.wMilliseconds = LOWORD(g_CurrentTime);
	}
	else
	{
		info.useStartTime.wSecond = HIWORD(g_CurrentTime);
		info.useStartTime.wMilliseconds = LOWORD(g_CurrentTime);
		ms_buffList.push_back(info);
	}

	CCardManager::Instance()->UpdateCardBuffDlg();
}

void CItemBuff::RemoveBuffItem(unsigned long typeId)
{
	for (std::list<sSCardAvilityPeriodInfo>::iterator it = ms_buffList.begin();
		it != ms_buffList.end();)
	{
		if ((*it).tid == typeId)
		{
			it = ms_buffList.erase(it);
		}
		else
		{
			++it;
		}
	}

	CCardManager::Instance()->UpdateCardBuffDlg();
}

void CItemBuff::UpdateBuffDlg(FrSPCardBuffDlg* dlg)
{
	if (dlg)
	{
		dlg->SetBuffItem(Doc()->m_cardAbilityList[0], true);
		dlg->SetBuffItem(ms_buffList, false);
	}
}
