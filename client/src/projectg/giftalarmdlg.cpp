#include "minatl.h"
#include "giftalarmdlg.h"
#include "shareddoc.h"
#include "treasure.h"
#include "../shared/localize.h"

extern Fresh* g_pFresh;

// HACK: 96.0f is defined in autotarget.cpp as a function-local static aliased
// to __real@42c00000; using the literal here would emit a competing COMDAT that
// all the other objects would bind to. A global (unlike a constant) is always
// loaded first by x87 codegen, so the y coordinate is read through a volatile
// lvalue (same instruction) to get "fld y; fadd 96" in the original order.
extern "C" __declspec(selectany) const float rb_real_42c00000 = 96.0f; // C64.4: o estático de função do rebang não existe mais

IMPLEMENT_OBJECT(FrGiftAlarmDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrGiftAlarmDlg, FrForm)

ON_FRESH_VI("giftlist", FRCMD_INIT, FrGiftAlarmDlg::OnGiftListInit)
ON_FRESH_VI("giftlist", FRCMD_OWNERDRAW, FrGiftAlarmDlg::OnGiftListOwnerDraw)
ON_FRESH_VI("infomsg", FRCMD_INIT, FrGiftAlarmDlg::OnInfoMsgInit)

END_FRESH_MSGMAP()

struct GiftMessage
{
	void* gift;
	char* lines[2];
};
struct TreasureMessage
{
	unsigned long typeId;
	unsigned short count;
	char sender[64];
	char message[128];
};

FrGiftAlarmDlg::FrGiftAlarmDlg()
{
	m_pGiftList = NULL;
	m_drawIndex = 0;
	m_giftNum = 0;
	m_blinkTime = 0;
	CSharedDoc::Instance()->m_bRefreshCamera = false;
	m_pArrivalMail = g_pFresh->RegisterBitmap("rmr_Arrival_mail");
	m_pOpenMail = g_pFresh->RegisterBitmap("rmr_Open_mail");
}

FrGiftAlarmDlg::~FrGiftAlarmDlg()
{
	for (FrListBox::ITEM_LIST::iterator it = m_pGiftList->m_itemList.begin();
		it != m_pGiftList->m_itemList.end(); ++it)
	{
		FrListItem* item = *it;
		GiftMessage* gift = (GiftMessage*)item->pData;
		if (gift)
		{
			delete[] gift->lines[0];
			delete[] gift->lines[1];
		}
		delete gift;
	}
}

void FrGiftAlarmDlg::OnInfoMsgInit(int param)
{
	m_pInfoMsg = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrGiftAlarmDlg::MakeSenderMsg(const char* msg, char** const lines)
{
	int length = strlen(msg);
	if (length > 20)
	{
		int lead = 0, other = 0;
		for (int i = length - 1; i >= 20; --i)
		{
			if (IsDBCSLeadByte(msg[i]))
				++lead;
			else
				++other;
		}
		if (lead % 2)
			++lead;
		int split = length - other - lead;
		memcpy(lines[0], msg, split);
		lines[0][split] = 0;
		for (int i = 0; i < 20; ++i)
		{
			if (msg[split] != ' ')
				break;
			++split;
		}
		strcpy(lines[1], msg + split);
	}
	else
	{
		strcpy(lines[0], msg);
		lines[1][0] = 0;
	}
}

void FrGiftAlarmDlg::OnGiftListInit(int param)
{
	char message[128] = "\0";
	m_pGiftList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pGiftList)
	{
		// 983: sem o ramo m_sendGiftList do 645 (so a lista de cartas)
		if (IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
		{
			std::list<sMailInfoBrief>& mails = CSharedDoc::Instance()->m_mailList;
			for (std::list<sMailInfoBrief>::iterator it = mails.begin(); it != mails.end(); ++it)
			{
				GiftMessage* gift = new GiftMessage;
				gift->gift = &*it;
				gift->lines[0] = new char[80];
				gift->lines[1] = new char[80];
				MakeSenderMsg(message, gift->lines);
				m_pGiftList->AddItem(gift);
			}
			m_giftNum = mails.size();
		}
	}
}

void FrGiftAlarmDlg::OnGiftListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	char sender[128] = "\0";
	char line1[80] = "\0";
	char line2[80] = "\0";
	unsigned long typeId;
	int type, count;
	if (IsLocalContent(S4_MAILBOX_FOR_REAL_MYROOM))
	{
		GiftMessage* gift = (GiftMessage*)item->pData;
		if (!gift)
			return;
		sMailInfoBrief* mail = (sMailInfoBrief*)gift->gift;
		wsprintf(sender, "%s", mail->sender);
		wsprintf(line1, "%s", gift->lines[0]);
		wsprintf(line2, "%s", gift->lines[1]);
		typeId = mail->item.dwTID;
		if (mail->item.iTimeCount > 0)
		{
			type = 2;
			count = mail->item.iTimeCount;
		}
		else
		{
			type = 0;
			count = mail->item.iCount;
		}
	}
	else
	{
		GiftMessage* gift = (GiftMessage*)item->pData;
		if (!gift)
			return;
		sGiftInfo* info = (sGiftInfo*)gift->gift;
		wsprintf(sender, "%s", info->sFromID);
		wsprintf(line1, "%s", gift->lines[0]);
		wsprintf(line2, "%s", gift->lines[1]);
		typeId = info->tid;
		type = info->ItemType;
		count = info->Arg0;
	}
	m_drawIndex = item->idx;
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->SetTextStyle(0);
	g_pFresh->GetManager()->PrintText(WPoint(item->pos.x + 10, item->pos.y + 5),
		0, sender, -1, 0xffffffff);
	const Bitmap* message =
		g_pFresh->GetManager()->GetBitmap("FRAMES8", "text_message");
	if (message)
	{
		gdi->DrawTexture(message,
			WRect(item->pos.x + 24, item->pos.y + 27, (float)message->Width(),
				(float)message->Height()),
			0xffffffff, 0);
	}
	gdi->SetTextColor(0xffffffff, 0xffffffff);
	// 983 008c643c: remetente "@PointEvent" = uma linha so, o texto do proprio nome por K2L
	if (_stricmp(sender, "@PointEvent") == 0)
		g_pFresh->GetManager()->PrintText(WPoint(item->pos.x + 28, item->pos.y + 56), 0,
			K2L_Compatibility("@PointEvent"), -1, 0xffffffff);
	else
	{
		g_pFresh->GetManager()->PrintText(
			WPoint(item->pos.x + 28, item->pos.y + 56), 0, line1, -1, 0xffffffff);
		g_pFresh->GetManager()->PrintText(
			WPoint(item->pos.x + 28, item->pos.y + 74), 0, line2, -1, 0xffffffff);
	}
	char name[80] = "\0";
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
	const Bitmap* icon;
	if (!common)
	{
		icon = m_pArrivalMail;
		wsprintf(name, K2L_Compatibility("\306\355\301\366"));
	}
	else
	{
		icon = g_pFresh->GetBitmap(common->Icon);
		wsprintf(name, "%s", common->Name);
	}
	if (icon)
	{
		float x, y;
		switch (typeId >> 26)
		{
		case 2:
		case 28:
			x = -10;
			y = -27;
			break;
		case 6:
			x = 0;
			y = 0;
			break;
		case 11:
			x = -18;
			y = -20;
			break;
		case 5:
			x = -10;
			y = -8;
			break;
		default:
			x = -10;
			y = -27;
			break;
		}
		{
			gdi->DrawTexture(icon,
				WRect(x + item->pos.x + 36, y + item->pos.y + 121,
					(float)icon->Width(), (float)icon->Height()),
				0xffffffff, 0);
		}
	}
	gdi->SetTextColor(0xff000000, 0xffffffff);
	if ((typeId >> 26) == 11)
	{
		// 983: nome com '\n' (flags 0x3000000) vira duas linhas, a segunda em estilo normal
		std::string line(name);
		const float x = item->pos.x + 181;
		float y = item->pos.y + 96;
		if (typeId & 0x3000000)
		{
			size_t nl = line.find('\n');
			if (nl != std::string::npos)
			{
				std::string first(line);
				first.erase(nl);
				gdi->SetTextStyle(1);
				gdi->Print(WPoint(x, y), 2, "%s", first.c_str());
				line.erase(0, nl + 1);
				gdi->SetTextStyle(0);
				y = item->pos.y + 111;
			}
			else
				gdi->SetTextStyle(1);
		}
		else
			gdi->SetTextStyle(0);
		gdi->Print(WPoint(x, y), 2, "%s", line.c_str());
		gdi->Print(WPoint(x, item->pos.y + 126), 2, "%s",
			MakeStr(K2L_Compatibility("%d\260\263"), count));
		return;
	}
	gdi->SetTextStyle(1);
	{
		// HACK: WPoint(x + 181, y + 96); see rb_real_42c00000 above
		WPoint pt;
		pt.x = item->pos.x + 181;
		pt.y = *(volatile float*)&item->pos.y + rb_real_42c00000;
		gdi->Print(pt, 2, "%s", name);
	}
	gdi->SetTextStyle(0);
	const char* text = NULL;
	if (type >= 3 && type <= 5)
	{
		if (type == 3)
			text = MakeStr("%d\275\303\260\243", count);
		else if (type == 4)
			text = MakeStr(K2L_Compatibility("%d\300\317"), count);
		else
			text = MakeStr("%d\260\263\277\371", count);
	}
	else
	{
		if ((typeId >> 26) == 28)
		{
			if (count > 0)
				text = MakeStr(K2L_Compatibility("%d\260\263"), count);
			else
			{
				IFF_STRUCT::sAuxPart* part = ItemManager()->FindAuxPart(typeId);
				if (part && part->COM[0] > 0)
					text = MakeStr(K2L_Compatibility("%d\260\263"), part->COM[0]);
			}
		}
		else if ((typeId >> 26) == 5)
		{
			if (count > 0)
				text = MakeStr(K2L_Compatibility("%d\260\263"), count);
			else
			{
				IFF_STRUCT::sBall* ball = ItemManager()->FindBall(typeId);
				if (ball && ball->COM[0] > 0)
					text = MakeStr(K2L_Compatibility("%d\260\263"), ball->COM[0]);
			}
		}
		else if ((typeId >> 26) == 6)
		{
			if (typeId == 436207632)
				text = MakeStr(K2L_Compatibility("%d\306\316"), count);
			else
				text = MakeStr(K2L_Compatibility("%d\260\263"), count);
		}
		else if ((typeId >> 26) == 14)
		{
			text = MakeStr(K2L_Compatibility("%d\300\317"), count);
		}
		else if ((typeId >> 26) == 2)
		{
			text = MakeStr(K2L_Compatibility("%d\260\263"), 1);
		}
	}
	if (text)
		gdi->Print(WPoint(item->pos.x + 181, item->pos.y + 116), 2, text);
}

void FrGiftAlarmDlg::OnProc(const float dt)
{
	static bool changed = false;
	static bool visible = false;
	m_blinkTime += dt;
	if (m_blinkTime > 1.0f)
	{
		visible ^= true;
		m_blinkTime = 0;
		changed = true;
	}
	if (visible)
	{
		if (!changed)
			return;
		m_pInfoMsg->SetCaption(MakeStr(
			K2L_Compatibility("\277\354\306\355\307\324\300\273 \310\256\300\316\307\317\274\274\277\344   (%d/%d)"),
			m_drawIndex + 1, m_giftNum));
	}
	else
	{
		if (!changed)
			return;
		m_pInfoMsg->SetCaption("");
	}
	changed = false;
}

IMPLEMENT_OBJECT(CTreasureAlarmDlg, FrForm)

BEGIN_FRESH_MSGMAP(CTreasureAlarmDlg, FrForm)

ON_FRESH_VI("giftlist", FRCMD_INIT, CTreasureAlarmDlg::OnListBox_Gift_Init)
ON_FRESH_VI("giftlist", FRCMD_OWNERDRAW,
	CTreasureAlarmDlg::OnListBox_Gift_OwnerDraw)
ON_FRESH_VI("infomsg", FRCMD_INIT, CTreasureAlarmDlg::OnStatic_Msg_Init)

END_FRESH_MSGMAP()

CTreasureAlarmDlg::CTreasureAlarmDlg()
{
	m_pGiftList = NULL;
	m_pMsg = NULL;
}

CTreasureAlarmDlg::~CTreasureAlarmDlg()
{
	for (FrListBox::ITEM_LIST::iterator it = m_pGiftList->m_itemList.begin();
		it != m_pGiftList->m_itemList.end(); ++it)
		delete (TreasureMessage*)(*it)->pData;
}

void CTreasureAlarmDlg::OnListBox_Gift_Init(int param)
{
	m_pGiftList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pGiftList)
	{
		CTHunter::Instance()->ReAdjustRepository();
		unsigned char count = CTHunter::Instance()->GetRepositorySize();
		for (unsigned char i = 0; i < count; ++i)
		{
			sTreasureHunt info;
			info = CTHunter::Instance()->GetGiftInfo(i);
			unsigned long typeId = info.dwTid;
			unsigned short quantity = info.wCount;
			TreasureMessage* gift = new TreasureMessage;
			gift->typeId = typeId;
			gift->count = quantity;
			strncpy(gift->sender, K2L_Compatibility("\306\256\267\271\301\256 \274\261\271\260"),
				64);
			strncpy(gift->message, K2L_Compatibility(""), 128);
			m_pGiftList->AddItem(gift);
		}
		m_giftNum = count;
	}
}

void CTreasureAlarmDlg::OnListBox_Gift_OwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	TreasureMessage* gift = (TreasureMessage*)item->pData;
	if (!gift)
		return;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(gift->typeId);
	if (!common)
		return;
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->SetTextStyle(0);
	g_pFresh->GetManager()->PrintText(WPoint(item->pos.x + 10, item->pos.y + 5),
		0, gift->sender, -1, 0xffffffff);
	const Bitmap* message = g_pFresh->RegisterBitmap("text_message_treasure");
	if (message)
	{
		gdi->DrawTexture(message,
			WRect(item->pos.x + 24, item->pos.y + 27, (float)message->Width(),
				(float)message->Height()),
			0xffffffff, 0);
	}
	gdi->SetTextColor(0xffffffff, 0xffffffff);
	g_pFresh->GetManager()->PrintText(
		WPoint(item->pos.x + 28, item->pos.y + 56), 0, gift->message, -1,
		0xffffffff);
	const Bitmap* icon = g_pFresh->GetBitmap(common->Icon);
	if (icon)
	{
		float x, y;
		switch (common->TypeId >> 26)
		{
		case 6:
			x = 0;
			y = 0;
			break;
		case 5:
			x = -10;
			y = -8;
			break;
		default:
			x = -10;
			y = -27;
			break;
		}
		{
			gdi->DrawTexture(icon,
				WRect(item->pos.x + 20 + 16 + x, item->pos.y + 35 + 86 + y,
					(float)icon->Width(), (float)icon->Height()),
				0xffffffff, 0);
		}
	}
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->SetTextStyle(1);
	{
		// HACK: WPoint(x + 181, y + 96); see rb_real_42c00000 above
		WPoint pt;
		pt.x = item->pos.x + 181;
		pt.y = *(volatile float*)&item->pos.y + rb_real_42c00000;
		gdi->Print(pt, 2, "%s", common->Name);
	}
	gdi->SetTextStyle(0);
	// 983: um unico MakeStr(K2L(fmt), n) no fim (cauda compartilhada)
	const char* text = NULL;
	unsigned long typeId = common->TypeId;
	if ((typeId >> 26) == 28)
	{
		if (gift->count > 0)
			text = MakeStr(K2L_Compatibility("%d\260\263"), gift->count);
		else
		{
			IFF_STRUCT::sAuxPart* part = ItemManager()->FindAuxPart(typeId);
			if (part && part->COM[0] > 0)
				text = MakeStr(K2L_Compatibility("%d\260\263"), part->COM[0]);
		}
	}
	else if ((typeId >> 26) == 5)
	{
		if (gift->count > 0)
			text = MakeStr(K2L_Compatibility("%d\260\263"), gift->count);
		else
		{
			IFF_STRUCT::sBall* ball = ItemManager()->FindBall(typeId);
			if (ball && ball->COM[0] > 0)
				text = MakeStr(K2L_Compatibility("%d\260\263"), ball->COM[0]);
		}
	}
	else if ((typeId >> 26) == 6)
	{
		if (typeId == 436207632)
			text = MakeStr(K2L_Compatibility("%d\306\316"), gift->count);
		else
			text = MakeStr(K2L_Compatibility("%d\260\263"), gift->count);
	}
	else if ((typeId >> 26) == 14)
	{
		text = MakeStr(K2L_Compatibility("%d\300\317"), gift->count);
	}
	else
	{
		text = MakeStr(K2L_Compatibility("%d\260\263"), 1);
	}

	if (text)
		gdi->Print(WPoint(item->pos.x + 181, item->pos.y + 116), 2, text);
}

void CTreasureAlarmDlg::OnStatic_Msg_Init(int param)
{
	m_pMsg = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void CTreasureAlarmDlg::OnProc(const float dt)
{
	static bool changed = false;
	static bool visible = false;
	m_blinkTime += dt;
	if (m_blinkTime > 1.0f)
	{
		visible ^= true;
		m_blinkTime = 0;
		changed = true;
	}
	if (visible)
	{
		if (!changed)
			return;
		m_pMsg->SetCaption(
			K2L_Compatibility("\270\266\300\314\267\353\277\241\274\255 \310\256\300\316\307\317\274\274\277\344"));
	}
	else
	{
		if (!changed)
			return;
		m_pMsg->SetCaption("");
	}
	changed = false;
}
