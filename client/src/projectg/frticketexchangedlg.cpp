#include "minatl.h"
#include "frticketexchangedlg.h"
#include "frbutton.h"
#include "fredit.h"
#include "frviewer.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "packet.h"
#include "taskmanager.h"
#include "actor.h"
#include "shareddoc.h"
#include "wlocalize.h"


extern Fresh* g_pFresh;

namespace nsTicketHelper
{
	__declspec(noinline) unsigned long __fastcall Combine(unsigned long type, unsigned long id)
	{
		return type | id;
	}

	void __fastcall Analyze(unsigned long& code, unsigned long& type,
		unsigned long& id)
	{
		type = code & 0xffff0000;
		id = code & 0xffff;
	}
} // namespace nsTicketHelper

IMPLEMENT_OBJECT(FrTicketExchangeDescDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTicketExchangeDescDlg, FrForm)

ON_FRESH_VI("desc_view", FRCMD_INIT, FrTicketExchangeDescDlg::OnInitDescViewer)

END_FRESH_MSGMAP()

FrTicketExchangeDescDlg::FrTicketExchangeDescDlg()
{
	m_pDescViewer = NULL;
	memset(m_descImage, 0, sizeof(m_descImage));
}

FrTicketExchangeDescDlg::~FrTicketExchangeDescDlg()
{
}

void FrTicketExchangeDescDlg::SetDescImage(const char* image)
{
	if (m_pDescViewer)
	{
		memset(m_descImage, 0, sizeof(m_descImage));
		strcpy(m_descImage, image);
		m_pDescViewer->ShowScrollBar(true);
		m_pDescViewer->Open(m_descImage);
	}
}

void FrTicketExchangeDescDlg::OnInitDescViewer(int obj)
{
	m_pDescViewer = FR_DYNAMIC_CAST(FrViewer, obj);
}

IMPLEMENT_OBJECT(FrTicketExchangeDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTicketExchangeDlg, FrForm)

ON_FRESH_VI("num_0", FRCMD_INIT, FrTicketExchangeDlg::onInit_Number_0)
ON_FRESH_VI("num_1", FRCMD_INIT, FrTicketExchangeDlg::onInit_Number_1)
ON_FRESH_VI("num_2", FRCMD_INIT, FrTicketExchangeDlg::onInit_Number_2)
ON_FRESH_VI("num_3", FRCMD_INIT, FrTicketExchangeDlg::onInit_Number_3)
ON_FRESH_VV("help", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_Help)

ON_FRESH_VI("icon_0", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_0)
ON_FRESH_VI("icon_1", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_1)
ON_FRESH_VI("icon_2", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_2)
ON_FRESH_VI("icon_3", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_3)
ON_FRESH_VI("icon_4", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_4)
ON_FRESH_VI("icon_5", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_5)
ON_FRESH_VI("icon_6", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_6)
ON_FRESH_VI("icon_7", FRCMD_INIT, FrTicketExchangeDlg::onInit_Icon_7)

ON_FRESH_VI("ent_b0", FRCMD_INIT, FrTicketExchangeDlg::onInit_ItemE0)
ON_FRESH_VV("ent_b0", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemE0)
ON_FRESH_VI("ent_0e0", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit0E0)
ON_FRESH_VI("ent_1e0", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit1E0)
ON_FRESH_VI("ent_b1", FRCMD_INIT, FrTicketExchangeDlg::onInit_ItemE1)
ON_FRESH_VV("ent_b1", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemE1)
ON_FRESH_VI("ent_0e1", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit0E1)
ON_FRESH_VI("ent_1e1", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit1E1)

ON_FRESH_VI("grp_b0", FRCMD_INIT, FrTicketExchangeDlg::onInit_ItemG0)
ON_FRESH_VV("grp_b0", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemG0)
ON_FRESH_VI("grp_0e0", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit0G0)
ON_FRESH_VI("grp_b1", FRCMD_INIT, FrTicketExchangeDlg::onInit_ItemG1)
ON_FRESH_VV("grp_b1", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemG1)
ON_FRESH_VI("grp_0e1", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit0G1)
ON_FRESH_VI("grp_b2", FRCMD_INIT, FrTicketExchangeDlg::onInit_ItemG2)
ON_FRESH_VV("grp_b2", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemG2)
ON_FRESH_VI("grp_0e2", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit0G2)
ON_FRESH_VI("grp_b3", FRCMD_INIT, FrTicketExchangeDlg::onInit_ItemG3)
ON_FRESH_VV("grp_b3", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemG3)
ON_FRESH_VI("grp_0e3", FRCMD_INIT, FrTicketExchangeDlg::onInit_Edit0G3)

ON_FRESH_VI("imd_b0", FRCMD_INIT, FrTicketExchangeDlg::onInit_ItemI0)
ON_FRESH_VV("imd_b0", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemI0)
ON_FRESH_VI("imd_b1", FRCMD_INIT, FrTicketExchangeDlg::onInit_Item_I1)
ON_FRESH_VV("imd_b1", FRCMD_LBUTTONUP, FrTicketExchangeDlg::onLBtnUp_ItemI1)

ON_FRESH_VI("grp_view0", FRCMD_INIT, FrTicketExchangeDlg::onInit_View0)
ON_FRESH_VI("grp_view1", FRCMD_INIT, FrTicketExchangeDlg::onInit_View1)
ON_FRESH_VI("grp_view2", FRCMD_INIT, FrTicketExchangeDlg::onInit_View2)
ON_FRESH_VI("grp_view3", FRCMD_INIT, FrTicketExchangeDlg::onInit_View3)

END_FRESH_MSGMAP()

FrTicketExchangeDlg::FrTicketExchangeDlg()
{
	m_pDescDlg = NULL;
	memset(m_pNumber, 0, sizeof(m_pNumber));
	new TicketInfo;
}

FrTicketExchangeDlg::~FrTicketExchangeDlg()
{
	std::map<unsigned long, TicketItemObject*>::iterator it = m_itemMap.begin();
	for (; it != m_itemMap.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
			it->second = NULL;
		}
	}
	m_itemMap.clear();
	memset(m_pNumber, 0, sizeof(m_pNumber));
	if (TicketInfo::Instance())
		delete TicketInfo::Instance();
}

bool FrTicketExchangeDlg::OnTicketExchangeDescResult(int result, FrForm* form)
{
	m_pDescDlg = NULL;
	return true;
}

TicketItemObject* FrTicketExchangeDlg::FindItem(unsigned long id)
{
	std::map<unsigned long, TicketItemObject*>::iterator it =
		m_itemMap.find(id);
	if (it != m_itemMap.end() && it->second)
		return it->second;

	TicketItemObject* item = new TicketItemObject;
	m_itemMap[id] = item;
	return item;
}

bool FrTicketExchangeDlg::Request(unsigned int type, unsigned long tid,
	unsigned int quantity)
{
	WSendPacket packet(0xdf);
	packet.Encode4(type);
	if (type == 0x10000 || type == 0x100000 || type == 0x1000000)
	{
		packet.Encode4(tid);
		packet.Encode4(quantity);
	}
	packet.Send(TO_GAME);

	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
	return true;
}

void FrTicketExchangeDlg::Reponse(unsigned long type, unsigned char result)
{
	char msg[256] = "";

	switch (result)
	{
	case 0:
		switch (type)
		{
		case 0x10000:
			strcpy(msg, K2L_Compatibility("\xc0\xc0\xb8\xf0 \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."
				"\n\xc7\xe0\xbf\xee\xc0\xbb \xb1\xe2\xbf\xf8\xc7\xd5\xb4\xcf\xb4\xd9."));
			break;
		case 0x100000:
			TicketInfo::Instance()->m_state = 0;
		case 0x1000000:
			strcpy(msg, K2L_Compatibility("\xc1\xf6\xb1\xde \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."
				"\n\xbf\xec\xc6\xed\xc7\xd4\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xba\xb8\xbd\xca\xbd\xc3\xbf\xc0."));
			break;
		case 0x11110000:
			ApplyInformation();
			return;
		}
		break;
	case 2:
		strcpy(msg, K2L_Compatibility("\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xc0\xe7\xb0\xed\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."
			"\n\xb4\xd9\xbd\xc3 \xb5\xe9\xbe\xee\xbf\xc3\xb6\xa7\xb1\xee\xc1\xf6 \xb1\xe2\xb4\xd9\xb7\xc1 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0."));
		break;
	case 4:
		strcpy(msg, K2L_Compatibility("\xb1\xb3\xc8\xaf \xb0\xa1\xb4\xc9\xc7\xd1 \xc6\xbc\xc4\xcf\xc0\xcc \xc0\xfb\xbd\xc0\xb4\xcf\xb4\xd9."
			"\n\xc1\xb6\xb1\xdd \xb4\xf5 \xb8\xf0\xbe\xc6\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."));
		break;
	}

	FrForm* form =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
	form->SetTimeLimit(3.0f);
	form->SetMessage(msg, true);
	form->Open(NULL, 0);

	if (result == 0)
		ApplyInformation();
}

void FrTicketExchangeDlg::ApplyInformation()
{
	if (TicketInfo::Instance())
	{
		char text[9] = "";

		SetCountImage(TicketInfo::Instance()->m_ticketCount);

		std::list<sItemInfo>::iterator lit;
		for (lit = CSharedDoc::Instance()->m_myItemList.begin();
			lit != CSharedDoc::Instance()->m_myItemList.end(); ++lit)
		{
			if (lit->tid == 0x1a0000bd)
			{
				lit->Common[0] = (short)TicketInfo::Instance()->m_ticketCount;
				if (lit->Common[0] < 1)
					CSharedDoc::Instance()->m_myItemList.erase(lit);
				break;
			}
		}

		std::map<unsigned long, TicketItemObject*>::iterator it =
			m_itemMap.begin();
		for (; it != m_itemMap.end(); it++)
		{
			unsigned long code = it->first;
			unsigned long type, id;
			nsTicketHelper::Analyze(code, type, id);
			switch (type)
			{
			case 0x100000:
	{
					TicketItemObject* item = it->second;
					TicketInfo::sTicketItem info =
						TicketInfo::Instance()->FindCount(item->get_TID());
					FrEdit* edit = item->get_Edit(0, 0);
					if (edit == NULL)
						continue;

					edit->ClearLine();
					sprintf(text, "%d", info.remain);
					edit->AddText(text, false, true);

					FrButton* button = item->get_Button(0);
					if (button == NULL)
						continue;

					if (info.remain == 0 || TicketInfo::Instance()->m_state == 0)
					{
						button->Enable(false);
						ChangeItemState(item, info.remain != 0 ? 1 : 2);
					}
					else
					{
						button->Enable(true);
						ChangeItemState(item, -1);
					}
				}
				break;
			case 0x10000:
	{
					TicketItemObject* item = it->second;
					TicketInfo::sTicketItem info =
						TicketInfo::Instance()->FindCount(item->get_TID());
					FrEdit* edit0 = item->get_Edit(0, 0);
					FrEdit* edit1 = item->get_Edit(1, 0);
					if (edit0 == NULL || edit1 == NULL)
						continue;

					edit0->ClearLine();
					edit1->ClearLine();
					sprintf(text, "%d", info.count);
					edit0->AddText(text, false, true);
					sprintf(text, "%d", info.remain);
					edit1->AddText(text, false, true);
				}
				break;
			}

			if (TicketInfo::Instance()->m_ticketCount == 0)
			{
				FrButton* button = it->second->get_Button(0);
				FrViewer* frame = it->second->get_Viewer(0, true);
				if (button && frame)
				{
					button->Enable(false);
					frame->Enable(true);
					frame->SetVisible(true);
				}
			}
		}
	}
}

void FrTicketExchangeDlg::ChangeItemState(TicketItemObject* item, int state)
{
	if (item)
	{
		FrViewer* viewer = item->get_Viewer(0, false);
		FrViewer* frame = item->get_Viewer(0, true);
		char image[64] = "";

		if (state == -1)
		{
			viewer->Enable(false);
			viewer->SetVisible(false);
			if (frame)
			{
				frame->Enable(false);
				frame->SetVisible(false);
			}
		}
		else
		{
			if (state != 1)
			{
				if (state != 2)
					return;
				if (strlen(image) == 0)
					strcpy(image, "soldout.tga");
			}
			if (strlen(image) == 0)
				strcpy(image, "payment.tga");
			viewer->Enable(true);
			viewer->SetVisible(true);
			viewer->Open(image);
			if (frame)
			{
				frame->Enable(true);
				frame->SetVisible(true);
			}
		}
	}
}

void FrTicketExchangeDlg::SetCountImage(unsigned int count)
{
	if (count > 9999)
		count = 9999;

	char number[5] = "";
	sprintf(number, "%d", count);
	unsigned int len = strlen(number);

	WRect rect((4.0f - len) * 0.5f * 14.0f + 457.0f + GetRect().x,
		GetRect().y + 69.0f, 14.0f, 19.0f);

	for (unsigned int i = 0; i < 4; ++i)
	{
		char image[16] = "";
		bool show = i < len;
		if (show)
			sprintf(image, "ticket_%c.tga", number[i]);
		m_pNumber[i]->Enable(show);
		m_pNumber[i]->SetVisible(show);
		m_pNumber[i]->Open(image);
		m_pNumber[i]->SetRect(rect);
		rect.x += rect.w;
	}
}

void FrTicketExchangeDlg::Auto_Initialize(unsigned int kind, unsigned int index,
	int obj, unsigned long id)
{
	TicketItemObject* item = FindItem(id);
	if (item)
	{
		switch (kind)
		{
		case 0:
			item->get_Button(obj);
			break;
		case 1:
			item->get_Edit(index, obj)->AddText("12345678", false, true);
			break;
		case 2:
			item->get_Viewer(obj, false);
			ChangeItemState(item, -1);
			break;
		case 3:
			char image[32] = "";
			sprintf(image, "ticket_frame%d.tga", index);
			FrViewer* viewer = item->get_Viewer(obj, true);
			viewer->Open(image);
			viewer->Enable(false);
			viewer->SetVisible(false);
			break;
		}
	}
}

bool FrTicketExchangeDlg::OnInit()
{
	unsigned long quantities[8] = { 1, 1, 5, 5, 5, 5, 3, 3 };
	unsigned long ids[8] = {
		nsTicketHelper::Combine(0x10000, 1), nsTicketHelper::Combine(0x10000, 2),
		nsTicketHelper::Combine(0x100000, 1), nsTicketHelper::Combine(0x100000, 2),
		nsTicketHelper::Combine(0x100000, 3), nsTicketHelper::Combine(0x100000, 4),
		nsTicketHelper::Combine(0x1000000, 1), nsTicketHelper::Combine(0x1000000, 2) };
	unsigned long tids[8] = { 0x3b9ac9f7, 0x3b9ac9f8, 0x1a00000e, 0x18000006,
		0x1a000011, 0x1a000040, 0x1a0000bc, 0x1a000030 };

	for (unsigned int i = 0; i < 8; ++i)
	{
		TicketItemObject* item = FindItem(ids[i]);
		if (item)
		{
			item->set_TID(tids[i]);
			item->set_Quantity(quantities[i]);
		}
	}

	for (int i = 0; i < 6; ++i)
	{
		if (TicketInfo::Instance()->m_itemMap.find(tids[i]) ==
			TicketInfo::Instance()->m_itemMap.end())
		{
			TicketInfo::sTicketItem& entry =
				TicketInfo::Instance()->m_itemMap[tids[i]];
			entry.count = 0;
			entry.remain = 0;
		}
	}

	Request(0x11110000, 0, 0);
	return true;
}

void FrTicketExchangeDlg::OnProc(const float dt)
{
}

void FrTicketExchangeDlg::onLBtnUp_Help()
{
	if (m_pDescDlg == NULL)
		m_pDescDlg = CreateForm<FrTicketExchangeDescDlg>(g_pFresh->GetManager(),
			this, "ticketexchange_desc", NULL);

	if (m_pDescDlg)
	{
		m_pDescDlg->SetDescImage("tiketexchange_event_explanation.jpg");
		m_pDescDlg->SetIconRect(m_pDescDlg->GetRect());
		m_pDescDlg->Open(
			(FRESH_PFN_RESULT)&FrTicketExchangeDlg::OnTicketExchangeDescResult,
			17);
	}
}

void FrTicketExchangeDlg::onInit_Icon_0(int obj)
{
	Auto_Initialize(3, 1, obj, nsTicketHelper::Combine(0x10000, 1));
}

void FrTicketExchangeDlg::onInit_Icon_1(int obj)
{
	Auto_Initialize(3, 2, obj, nsTicketHelper::Combine(0x10000, 2));
}

void FrTicketExchangeDlg::onInit_Icon_2(int obj)
{
	Auto_Initialize(3, 3, obj, nsTicketHelper::Combine(0x100000, 1));
}

void FrTicketExchangeDlg::onInit_Icon_3(int obj)
{
	Auto_Initialize(3, 4, obj, nsTicketHelper::Combine(0x100000, 2));
}

void FrTicketExchangeDlg::onInit_Icon_4(int obj)
{
	Auto_Initialize(3, 5, obj, nsTicketHelper::Combine(0x100000, 3));
}

void FrTicketExchangeDlg::onInit_Icon_5(int obj)
{
	Auto_Initialize(3, 6, obj, nsTicketHelper::Combine(0x100000, 4));
}

void FrTicketExchangeDlg::onInit_Icon_6(int obj)
{
	Auto_Initialize(3, 7, obj, nsTicketHelper::Combine(0x1000000, 1));
}

void FrTicketExchangeDlg::onInit_Icon_7(int obj)
{
	Auto_Initialize(3, 8, obj, nsTicketHelper::Combine(0x1000000, 2));
}

void FrTicketExchangeDlg::onInit_ItemE0(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x10000, 1));
}

void FrTicketExchangeDlg::onLBtnUp_ItemE0()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x10000, 1));
	if (item)
		Request(0x10000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Edit0E0(int obj)
{
	Auto_Initialize(1, 0, obj, nsTicketHelper::Combine(0x10000, 1));
}

void FrTicketExchangeDlg::onInit_Edit1E0(int obj)
{
	Auto_Initialize(1, 1, obj, nsTicketHelper::Combine(0x10000, 1));
}

void FrTicketExchangeDlg::onInit_ItemE1(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x10000, 2));
}

void FrTicketExchangeDlg::onLBtnUp_ItemE1()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x10000, 2));
	if (item)
		Request(0x10000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Edit0E1(int obj)
{
	Auto_Initialize(1, 0, obj, nsTicketHelper::Combine(0x10000, 2));
}

void FrTicketExchangeDlg::onInit_Edit1E1(int obj)
{
	Auto_Initialize(1, 1, obj, nsTicketHelper::Combine(0x10000, 2));
}

void FrTicketExchangeDlg::onInit_ItemG0(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x100000, 1));
}

void FrTicketExchangeDlg::onLBtnUp_ItemG0()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x100000, 1));
	if (item)
		Request(0x100000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Edit0G0(int obj)
{
	Auto_Initialize(1, 0, obj, nsTicketHelper::Combine(0x100000, 1));
}

void FrTicketExchangeDlg::onInit_ItemG1(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x100000, 2));
}

void FrTicketExchangeDlg::onLBtnUp_ItemG1()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x100000, 2));
	if (item)
		Request(0x100000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Edit0G1(int obj)
{
	Auto_Initialize(1, 0, obj, nsTicketHelper::Combine(0x100000, 2));
}

void FrTicketExchangeDlg::onInit_ItemG2(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x100000, 3));
}

void FrTicketExchangeDlg::onLBtnUp_ItemG2()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x100000, 3));
	if (item)
		Request(0x100000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Edit0G2(int obj)
{
	Auto_Initialize(1, 0, obj, nsTicketHelper::Combine(0x100000, 3));
}

void FrTicketExchangeDlg::onInit_ItemG3(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x100000, 4));
}

void FrTicketExchangeDlg::onLBtnUp_ItemG3()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x100000, 4));
	if (item)
		Request(0x100000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Edit0G3(int obj)
{
	Auto_Initialize(1, 0, obj, nsTicketHelper::Combine(0x100000, 4));
}

void FrTicketExchangeDlg::onInit_ItemI0(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x1000000, 1));
}

void FrTicketExchangeDlg::onLBtnUp_ItemI0()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x1000000, 1));
	if (item)
		Request(0x1000000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Item_I1(int obj)
{
	Auto_Initialize(0, 0, obj, nsTicketHelper::Combine(0x1000000, 2));
}

void FrTicketExchangeDlg::onLBtnUp_ItemI1()
{
	TicketItemObject* item = FindItem(nsTicketHelper::Combine(0x1000000, 2));
	if (item)
		Request(0x1000000, item->get_TID(), item->get_Quantity());
}

void FrTicketExchangeDlg::onInit_Number_0(int obj)
{
	if (m_pNumber[0] == NULL)
		m_pNumber[0] = FR_DYNAMIC_CAST(FrViewer, obj);
}

void FrTicketExchangeDlg::onInit_Number_1(int obj)
{
	if (m_pNumber[1] == NULL)
		m_pNumber[1] = FR_DYNAMIC_CAST(FrViewer, obj);
}

void FrTicketExchangeDlg::onInit_Number_2(int obj)
{
	if (m_pNumber[2] == NULL)
		m_pNumber[2] = FR_DYNAMIC_CAST(FrViewer, obj);
}

void FrTicketExchangeDlg::onInit_Number_3(int obj)
{
	if (m_pNumber[3] == NULL)
		m_pNumber[3] = FR_DYNAMIC_CAST(FrViewer, obj);
}

void FrTicketExchangeDlg::onInit_View0(int obj)
{
	Auto_Initialize(2, 0, obj, nsTicketHelper::Combine(0x100000, 1));
}

void FrTicketExchangeDlg::onInit_View1(int obj)
{
	Auto_Initialize(2, 0, obj, nsTicketHelper::Combine(0x100000, 2));
}

void FrTicketExchangeDlg::onInit_View2(int obj)
{
	Auto_Initialize(2, 0, obj, nsTicketHelper::Combine(0x100000, 3));
}

void FrTicketExchangeDlg::onInit_View3(int obj)
{
	Auto_Initialize(2, 0, obj, nsTicketHelper::Combine(0x100000, 4));
}
