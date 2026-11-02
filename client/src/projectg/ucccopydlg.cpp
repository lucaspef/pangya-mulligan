#include "minatl.h"
#include "ucccopydlg.h"
#include "uccmanager.h"
#include "projectg.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrUccCopyDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrUccCopyDlg, FrForm)

ON_FRESH_VI("char_icon", FRCMD_INIT, FrUccCopyDlg::OnCharIconInit)
ON_FRESH_VI("item_list", FRCMD_INIT, FrUccCopyDlg::OnItemListInit)
ON_FRESH_VI("item_list", FRCMD_OWNERDRAW, FrUccCopyDlg::OnItemListOwnerDraw)
ON_FRESH_VV("item_list", FRCMD_LBUTTONDOWN, FrUccCopyDlg::OnItemListBtnDown)
ON_FRESH_VI("ok_btn", FRCMD_INIT, FrUccCopyDlg::OnOkBtnInit)
ON_FRESH_VV("ok_btn", FRCMD_LBUTTONUP, FrUccCopyDlg::OnOkBtnUp)
ON_FRESH_VI("cancel_btn", FRCMD_INIT, FrUccCopyDlg::OnCancelBtnInit)
ON_FRESH_VV("cancel_btn", FRCMD_LBUTTONUP, FrUccCopyDlg::OnCancelBtnUp)
ON_FRESH_VI("etc", FRCMD_OWNERDRAW, FrUccCopyDlg::OnEtcOwnerDraw)

END_FRESH_MSGMAP()

FrUccCopyDlg::FrUccCopyDlg()
{
	Init();
}

FrUccCopyDlg::~FrUccCopyDlg()
{
}

void FrUccCopyDlg::Init()
{
	m_pItemInfo = NULL;
	m_pConfirmDlg = NULL;
	m_pItemBaseN = g_pFresh->RegisterBitmap("item_base_n");
	m_pItemBaseO = g_pFresh->RegisterBitmap("item_base_o");
	m_pItemBaseD = g_pFresh->RegisterBitmap("item_base_d");
}

void FrUccCopyDlg::SetItem(sItemInfo* pItemInfo)
{
	m_pItemInfo = pItemInfo;
	BuildItemList();
}

void FrUccCopyDlg::OnCharIconInit(int param)
{
	m_pCharIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pCharIcon && m_pItemInfo)
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(m_pItemInfo->tid);
		if (part)
		{
			const Bitmap* icon = g_pFresh->GetBitmap(part->c.Icon);
			if (icon)
				m_pCharIcon->SetBgImg(icon);
		}
	}
}

void FrUccCopyDlg::OnItemListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrUccCopyDlg::OnItemListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	FrGraphicInterface* gdi = GetGDI();
	if (!gdi)
		return;
	const Bitmap* base = item->selected ? m_pItemBaseD
		: item->underCursor             ? m_pItemBaseO
										: m_pItemBaseN;
	WRect dest(item->pos.x, item->pos.y, (float)base->Width(),
		(float)base->Height());
	gdi->DrawTexture(base, dest, 0xffffffff, 0);
	sItemInfo* info = (sItemInfo*)item->pData;
	if (info)
	{
		const Bitmap* icon = GetUccItemIcon(info->guid);
		const char* name = GetUccItemName(info->guid);
		WPoint origin = item->pos;
		if (icon)
		{
			WRect rect(origin.x + 15, origin.y + 5, (float)icon->Width(),
				(float)icon->Height());
			gdi->DrawTexture(icon, rect, 0xffffffff, 0);
		}
		gdi->PrintText(WPoint(origin.x + 184, origin.y + 6), 2, name, 0);
	}
}

void FrUccCopyDlg::OnItemListBtnDown()
{
}

void FrUccCopyDlg::OnOkBtnInit(int param)
{
	m_pOkBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccCopyDlg::OnOkBtnUp()
{
	if (m_pConfirmDlg)
		return;
	FrListItem* item = m_pItemList->GetSelected();
	sItemInfo* info;
	if (item && (info = (sItemInfo*)item->pData) != NULL)
	{
		float x = item->pos.x - 33;
		float y = item->pos.y - 124;
		m_pConfirmDlg = CreateForm<FrUccCopyConfirmDlg>(g_pFresh->GetManager(),
			this, "ucc_copy_confirm", NULL);
		m_pConfirmDlg->EnableTail(false);
		m_pConfirmDlg->Init(m_pItemInfo, info);
		m_pConfirmDlg->Open(
			(FRESH_PFN_RESULT)&FrUccCopyDlg::OnUccCopyConfirmDlgResult,
			WPoint(x, y), 1);
		m_pConfirmDlg->SetFadeout(false);
	}
	else
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), // 983 007e1951
			(int)K2L_Compatibility("\272\271\301\246\307\322 \300\307\273\363\300\273 \274\261\305\303\307\330\301\326\274\274\277\344."),
			0, 0, 0, 0));
}

void FrUccCopyDlg::OnCancelBtnInit(int param)
{
	m_pCancelBtn = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccCopyDlg::OnCancelBtnUp()
{
	OnFreshCancel();
}

void FrUccCopyDlg::OnEtcOwnerDraw(int param)
{
	FrGraphicInterface* gdi = GetGDI();
	if (gdi && m_pItemInfo)
	{
		WRect origin = m_rect;
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(m_pItemInfo->tid);
		if (part)
		{
			const Bitmap* icon = g_pFresh->GetBitmap(part->c.Icon);
			if (icon)
			{
				WRect dest(origin.x + 226, origin.y + 13, (float)icon->Width(),
					(float)icon->Height());
				gdi->DrawTexture(icon, dest, 0xffffffff, 0);
			}
			gdi->PrintText(WPoint(origin.x + 401, origin.y + 19), 2,
				part->c.Name, 0);
		}
		gdi->PrintText(WPoint(origin.x + 401, origin.y + 32), 2,
			Doc()->m_myInfo.info.sNick, 0);

		// 983 (007e19b0): aviso em vermelho embaixo
		gdi->SetTextColor(0xffff0000, 0xffffffff);
		gdi->Print(WPoint(origin.x + 10, origin.y + 375), 0,
			K2L_Compatibility("\301\2463\300\332\300\307 \261\307\270\256\270\246 \304\247\307\330/\271\314\307\263\276\347\275\304\300\273"));
		gdi->Print(WPoint(origin.x + 10, origin.y + 390), 0,
			K2L_Compatibility("\307\330\304\245 \277\354\267\301\260\241 \276\370\264\302\301\366 \303\346\272\320\310\367 \306\307\264\334\300\273 \307\321 \310\304, \272\271\273\347 \307\330 \301\326\275\312\275\303\277\300"));
	}
}

void FrUccCopyDlg::BuildItemList()
{
	if (!m_pItemList)
		return;
	IFF_STRUCT::sPart* target = ItemManager()->FindPart(m_pItemInfo->tid);
	if (!target)
		return;
	for (std::list<sItemInfo>::iterator it = Doc()->m_uccItemList.begin();
		it != Doc()->m_uccItemList.end(); ++it)
	{
		sItemInfo* info = &*it;
		if (info && (info->status & 1) && info->Seq == 1)
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(info->tid);
			if (part && IsSameClothes(target, part))
				m_pItemList->AddItem(info);
		}
	}
}

bool FrUccCopyDlg::OnUccCopyConfirmDlgResult(int result, FrForm* form)
{
	// HACK
	if (0)
		m_pConfirmDlg->Init(NULL, NULL);
	if (form)
		form->EnableTail(true);
	m_pConfirmDlg = NULL;
	if (result == 1)
		OnFreshOkay();
	return true;
}

IMPLEMENT_OBJECT(FrUccCopyConfirmDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrUccCopyConfirmDlg, FrForm)

ON_FRESH_VI("icon", FRCMD_INIT, FrUccCopyConfirmDlg::OnIconInit)
ON_FRESH_VI("ok_btn", FRCMD_INIT, FrUccCopyConfirmDlg::OnOkBtnInit)
ON_FRESH_VV("ok_btn", FRCMD_LBUTTONUP, FrUccCopyConfirmDlg::OnOkBtnUp)
ON_FRESH_VI("cancel_btn", FRCMD_INIT, FrUccCopyConfirmDlg::OnCancelBtnInit)
ON_FRESH_VV("cancel_btn", FRCMD_LBUTTONUP, FrUccCopyConfirmDlg::OnCancelBtnUp)
ON_FRESH_VI("etc", FRCMD_OWNERDRAW, FrUccCopyConfirmDlg::OnEtcOwnerDraw)

END_FRESH_MSGMAP()

FrUccCopyConfirmDlg::FrUccCopyConfirmDlg()
	: m_pTargetItem(NULL), m_pSourceItem(NULL)
{
}

FrUccCopyConfirmDlg::~FrUccCopyConfirmDlg()
{
}

void FrUccCopyConfirmDlg::OnIconInit(int param)
{
	m_pIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccCopyConfirmDlg::OnOkBtnInit(int param)
{
	m_pOkBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccCopyConfirmDlg::OnOkBtnUp()
{
	WSendPacket packet(0xb9);
	packet.Encode1(2);
	packet.Encode4(m_pSourceItem->tid);
	packet.EncodeStr(m_pSourceItem->UccIndex);
	packet.Encode2(m_pSourceItem->Seq);
	packet.Encode4(m_pTargetItem->guid);
	packet.Send(TO_GAME);
	OnFreshOkay();
}

void FrUccCopyConfirmDlg::OnCancelBtnInit(int param)
{
	m_pCancelBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccCopyConfirmDlg::OnCancelBtnUp()
{
	SetFadeout(true);
	OnFreshCancel();
}

void FrUccCopyConfirmDlg::OnEtcOwnerDraw(int param)
{
	// HACK
	if (0)
		Init(NULL, NULL);
	FrGraphicInterface* gdi = GetGDI();
	if (gdi && m_pSourceItem)
	{
		sUccClothes* clothes = UccManager()->FindClothes(m_pSourceItem->tid,
			m_pSourceItem->UccIndex);
		if (clothes)
		{
			WPoint pos = WPoint(m_rect.x, m_rect.y) + WPoint(100, 28);
			gdi->PrintText(pos, 0,
				K2L_Compatibility("\272\271\301\246\307\317\275\303\260\332\275\300\264\317\261\356?"),
				0);
			gdi->SetTextStyle(1);
			gdi->PrintText(WPoint(pos.x, pos.y + 16), 0, clothes->uccName, 0);
			gdi->SetTextStyle(0);
		}
	}
}

void FrUccCopyConfirmDlg::Init(sItemInfo* pTargetItem, sItemInfo* pSourceItem)
{
	m_pTargetItem = pTargetItem;
	m_pSourceItem = pSourceItem;
	if (m_pIcon && m_pSourceItem)
	{
		const Bitmap* icon = GetUccItemIcon(m_pSourceItem);
		if (icon)
			m_pIcon->SetBgImg(icon);
	}
}

IMPLEMENT_OBJECT(FrUccCopyCompletedDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrUccCopyCompletedDlg, FrForm)

ON_FRESH_VI("ok_btn", FRCMD_INIT, FrUccCopyCompletedDlg::OnOkBtnInit)
ON_FRESH_VV("ok_btn", FRCMD_LBUTTONUP, FrUccCopyCompletedDlg::OnOkBtnUp)
ON_FRESH_VI("etc", FRCMD_OWNERDRAW, FrUccCopyCompletedDlg::OnEtcOwnerDraw)

END_FRESH_MSGMAP()

FrUccCopyCompletedDlg::FrUccCopyCompletedDlg()
	: m_typeID(0)
{
	m_uccIndex[0] = 0;
}

FrUccCopyCompletedDlg::~FrUccCopyCompletedDlg()
{
}

void FrUccCopyCompletedDlg::Init(unsigned long typeID, const char* uccIndex)
{
	m_typeID = typeID;
	strncpy(m_uccIndex, uccIndex, 9);
}

void FrUccCopyCompletedDlg::OnOkBtnInit(int param)
{
	m_pOkBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccCopyCompletedDlg::OnOkBtnUp()
{
	OnFreshOkay();
}

void FrUccCopyCompletedDlg::OnEtcOwnerDraw(int param)
{
	if (!m_typeID || !strlen(m_uccIndex))
		return;
	FrGraphicInterface* gdi = GetGDI();
	if (!gdi)
		return;
	sUccClothes* clothes = UccManager()->FindClothes(m_typeID, m_uccIndex);
	if (!clothes)
		return;
	WPoint pos(m_rect.x + 65, m_rect.y + 113);
	const Bitmap* icon = GetUccItemIcon(GetMyItemInfo(m_typeID, m_uccIndex, 0));
	const char* name = GetUccItemName(GetMyItemInfo(m_typeID, m_uccIndex, 0));
	if (icon)
	{
		WRect dest(pos.x - 5, pos.y - 15, (float)icon->Width(),
			(float)icon->Height());
		gdi->DrawTexture(icon, dest, 0xffffffff, 0);
	}
	pos.x = m_rect.x + 226;
	pos.y = m_rect.y + 133;
	gdi->SetTextStyle(1);
	gdi->PrintText(pos, 2, name, 0);
	gdi->SetTextStyle(0);
	pos.y += 16;
	gdi->PrintText(pos, 2, Doc()->m_myInfo.info.sNick, 0);
	pos.x = m_rect.x + 161;
	pos.y = m_rect.y + 50;
	gdi->PrintText(pos, 1,
		K2L_Compatibility("\274\272\260\370\300\373\300\270\267\316 \272\271\301\246\265\307\276\372\275\300\264\317\264\331."),
		0);
}
