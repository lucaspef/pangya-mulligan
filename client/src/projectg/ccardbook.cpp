#include "minatl.h"
#include "ccardbook.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "wfont.h"
#include "wresrcmng.h"
#include "jp/taskform.h" // FrTaskForm (CTask::GetWidgetForm983): origem das coordenadas do 983


extern Fresh* g_pFresh;
extern WView* g_view;
extern WResourceManager* g_resrcmng;

// HACK: the owned card list is declared in the PCH only as padding.
#define CARD_LIST() (Doc()->m_cardList)

CCardSlot::CCardSlot()
	: m_state(0),
	  m_pButton(NULL),
	  m_pCard(NULL),
	  m_pCommon(NULL),
	  m_pItem(NULL),
	  m_pOverlay(NULL),
	  m_pOpenCardPackDlg(NULL),
	  m_pItemAttachCardDlg(NULL),
	  m_pSpCardUseDlg(NULL)
{
	m_rect.x = 0;
	m_rect.y = 0;
	m_rect.w = 0;
	m_rect.h = 0;
}

CCardSlot::~CCardSlot()
{
	if (m_pOverlay && g_resrcmng)
	{
		g_resrcmng->Release(m_pOverlay);
		m_pOverlay = NULL;
	}
}

// A300: origem (canto do card_book) das casas do CCardPage, que no 983 sao relativas (007f62c0);
// fica (0,0) fora de CCardBook::DrawCard (as casas do CCardPackPage seguem absolutas, 007f5a10)
static WPoint s_cardPageOrigin983(0.0f, 0.0f);

// 983 00800d90 (FrItemAttachCardDlg, carddlg.cpp): slot do botao -> indice no pacote
// (tipo por 00800cd0: <=3 -> 0, <=6 -> 1, <=8 -> 5; indice = slot + 0/1/2)
static unsigned long __fastcall ItemAttachSlotIndex983(
	FrItemAttachCardDlg* pDlg)
{
	int slot = pDlg->m_slot;
	int type = -1;
	if (slot <= 3)
		type = 0;
	else if (slot <= 6)
		type = 1;
	else if (slot <= 8)
		type = 5;
	if (type == 0)
		return slot;
	if (type == 1)
		return slot + 1;
	if (type == 5)
		return slot + 2;
	return (unsigned long)-1; // 983 00800d90: -1 fora das 3 faixas
}

void CCardSlot::SetCard(unsigned long tid)
{
	if (tid != 0)
	{
		m_pItem = NULL;
		for (std::list<sCards>::iterator it = CARD_LIST().begin();
			it != CARD_LIST().end(); ++it)
		{
			m_pCard = ItemManager()->FindCard(tid);
			m_pCommon = ItemManager()->FindCommonItem(tid);
			m_pItem = &*it;
			if (m_pItem)
			{
				if (m_pItem->tid == tid)
					break;
				m_pCard = NULL;
				m_pCommon = NULL;
				m_pItem = NULL;
			}
		}
	}
	else
	{
		m_pCard = NULL;
		m_pCommon = NULL;
		m_pItem = NULL;
	}
	if (m_pButton && tid != 0)
	{
		if (m_pCard && m_pCommon)
		{
			m_pButton->SetVisible(true);
			m_pButton->SetButtonImg(m_pCommon->Icon, FrButton::NORMAL);
			m_pButton->SetButtonImg(m_pCommon->Icon, FrButton::OVER);
			m_pButton->SetButtonImg(m_pCommon->Icon, FrButton::PRESSED);
		}
	}
	else if (m_pButton && tid == 0)
		m_pButton->SetVisible(false);
	m_state = 0;
	if (m_pOverlay && g_resrcmng)
	{
		g_resrcmng->Release(m_pOverlay);
		m_pOverlay = NULL;
	}
}

void CCardSlot::SetPosition(WRect& rect)
{
	m_rect = rect;
}

void CCardSlot::Select(unsigned char state)
{
	m_state = state;
}

void CCardSlot::UseCard()
{
	if (m_pCard && m_pCommon && m_pItem)
	{
		switch ((m_pItem->tid >> 22) & 0xf)
		{
		case 2:
			m_pSpCardUseDlg = CreateForm<FrSpCardUseDlg>(g_pFresh->GetManager(),
				this, "usecard", NULL);
			m_pSpCardUseDlg->SetSpCardInfo(m_pItem->tid);
			m_pSpCardUseDlg->Open(
				(FRESH_PFN_RESULT)&CCardSlot::OnSpCardDlgResult, 1);
			break;
		case 3:
			m_pOpenCardPackDlg = CreateForm<FrOpenCardPackDlg>(
				g_pFresh->GetManager(), this, "cardpack_open", NULL);
			m_pOpenCardPackDlg->SetCaption(K2L_Compatibility(
				"\xc4\xab\xb5\xe5\xc6\xd1\xb0\xb3\xba\xc0\xc8\xae\xc0\xce"));
			m_pOpenCardPackDlg->m_guid = m_pItem->uid;
			m_pOpenCardPackDlg->m_tid = m_pItem->tid;
			m_pOpenCardPackDlg->Open(
				(FRESH_PFN_RESULT)&CCardSlot::OnCardPackDlgResult, 1);
			break;
		default:
			m_pCardUseDlg = CreateForm<FrCardUseDlg>(g_pFresh->GetManager(),
				this, "usecard", NULL);
			m_pCardUseDlg->SetCaption(
				K2L_Compatibility("\xc4\xab\xb5\xe5\xbb\xe7\xbf\xeb"));
			m_pCardUseDlg->m_guid = m_pItem->uid;
			m_pCardUseDlg->m_tid = m_pItem->tid;
			m_pCardUseDlg->Open((FRESH_PFN_RESULT)&CCardSlot::OnCardDlgResult,
				1);
			break;
		}
	}
}

bool CCardSlot::OnCardPackDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		FrOpenCardPackDlg* pDlg = (FrOpenCardPackDlg*)pForm;
		if (pDlg && (pDlg->m_tid & 0x3c00000) == 0xc00000)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			{
				WSendPacket packet((enumClientPacket)0xca);
				packet.Encode4(pDlg->m_tid);
				packet.Encode4(pDlg->m_guid);
				packet.Send(TO_GAME);
			}
			m_pOpenCardPackDlg = NULL;
		}
		else
			m_pOpenCardPackDlg = NULL;
	}
	else
		m_pOpenCardPackDlg = NULL;
	return true;
}

bool CCardSlot::OnSpCardDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		FrSpCardUseDlg* pDlg = (FrSpCardUseDlg*)pForm;
		if (pDlg)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			{
				WSendPacket packet((enumClientPacket)0xbd);
				packet.Encode4(pDlg->GetSpCardTid());
				packet.Send(TO_GAME);
			}
			m_pSpCardUseDlg = NULL;
		}
		else
			m_pSpCardUseDlg = NULL;
	}
	else
		m_pSpCardUseDlg = NULL;
	return true;
}


bool CCardSlot::OnCardDlgResult(int result, FrForm* pForm)
{
	if (result == 1 && pForm)
	{
		FrCardUseDlg* pDlg = (FrCardUseDlg*)pForm;
		unsigned int type = (pDlg->m_tid >> 22) & 0xf;
		if (type == 2)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			WSendPacket packet((enumClientPacket)0xbd);
			packet.Encode4(pDlg->m_tid);
			packet.Send(TO_GAME);
		}
		else if (type == 0 || type == 1 || type == 5)
		{
			m_pItemAttachCardDlg = CreateForm<FrItemAttachCardDlg>(
				g_pFresh->GetManager(), this, "itemattachcarddlg", NULL);
			m_pItemAttachCardDlg->SetCaption(K2L_Compatibility("\xc0\xc7\xbb\xf3\xbc\xb1\xc5\xc3"));
			m_pItemAttachCardDlg->m_cardGuid = pDlg->m_guid;
			m_pItemAttachCardDlg->m_cardTid = pDlg->m_tid;
			m_pItemAttachCardDlg->m_cardType = (pDlg->m_tid >> 22) & 0xf;
			m_pItemAttachCardDlg->Open(
				(FRESH_PFN_RESULT)&CCardSlot::OnItemAttachCardDlgResult, 1);
		}
	}
	m_pCardUseDlg = NULL;
	return true;
}

bool CCardSlot::OnItemAttachCardDlgResult(int result, FrForm* pForm)
{
	if (pForm && result == 1)
	{
		FrItemAttachCardDlg* pDlg = (FrItemAttachCardDlg*)pForm;
		if (pDlg->m_slot == 0xff)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xbd\xbd\xb7\xd4\xc0\xcc \xbc\xb1\xc5\xc3\xb5\xc7\xc1\xf6 "
					"\xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
		else
		{
			// 983 007f3cd0: a casa vai convertida (00800d90 = ItemAttachSlotIndex983:
			// caddie +1, npc +2), como no selectedCardSlotNum; m_slot cru recusava caddie/npc
			if (ItemManager()->IsPartAttachCard(pDlg->m_cardTid,
					pDlg->m_partTid, ItemAttachSlotIndex983(pDlg)))
			{
				sCards info;
				info.tid = pDlg->m_cardTid;
				info.uid = pDlg->m_cardGuid;
				info.partTid = pDlg->m_partTid;
				info.partUid = pDlg->m_partGuid;
				info.selectedCardSlotNum = ItemAttachSlotIndex983(pDlg);
				WSendPacket packet((enumClientPacket)0xc8);
				packet.EncodeBuffer(&info, 0x3a);
				packet.Send(TO_GAME);
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
				return true;
			}
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xbd\xbd\xb7\xd4\xb0\xfa \xc4\xab\xb5\xe5\xc0\xc7 "
					 "\xc5\xb8\xc0\xd4\xc0\xcc \xb4\xde\xb6\xf3\xbc\xad "
					 "\xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 "
					 "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
		}
	}
	return true;
}

void CCardSlot::Draw(FrGraphicInterface* pGDI)
{
	if (m_state != 0)
	{
		const char* name;
		if (m_state == 1)
			name = "select_card_d";
		else if (m_state == 2)
			name = "select_card_0";
		else
			name = NULL;
		const Bitmap* pBitmap = name ? g_pFresh->GetBitmap(name) : NULL;
		if (pBitmap)
		{
			WRect rc(s_cardPageOrigin983.x + m_rect.x, s_cardPageOrigin983.y + m_rect.y,
				pBitmap->Width(), pBitmap->Height());
			pGDI->DrawTexture(pBitmap, rc, 0xffffffff, 0);
		}
	}
	if (m_state == 1)
	{
		// 983: todas as coordenadas sao relativas ao FrTaskForm do task atual
		float ox = 0.0f;
		float oy = 0.0f;
		FrTaskForm* pTaskForm = AfxGetTask()->GetWidgetForm983();
		if (pTaskForm)
		{
			ox = pTaskForm->GetRect().x;
			oy = pTaskForm->GetRect().y;
		}
		if (m_pCommon && m_pCommon->TypeId &&
			ItemManager()->FindCard(m_pCommon->TypeId))
		{
			if (m_pOverlay == NULL &&
				ItemManager()->FindCard(m_pCommon->TypeId))
			{
				char name[64];
				sprintf(name, "%s.png",
					ItemManager()->FindCard(m_pCommon->TypeId)->Image);
				m_pOverlay = g_resrcmng->GetOverlay(name, 0);
			}
			if (m_pOverlay && AfxGetTask()->GetWidgetForm983())
			{
				WRect src(0, 0, 1.0f, 1.0f);
				WRect dst(ox + 92.0, oy + 117.0, (float)m_pOverlay->GetWidth(),
					(float)m_pOverlay->GetHeight());
				m_pOverlay->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
			}
		}
		const Bitmap* pHelp1 = g_pFresh->GetBitmap("help_1");
		const Bitmap* pHelp2 = g_pFresh->GetBitmap("help_2");
		if (pHelp1)
		{
			WRect rc(ox + 12.0, oy + 417.0, pHelp1->Width(), pHelp1->Height());
			pGDI->DrawTexture(pHelp1, rc, 0xffffffff, 0);
		}
		if (pHelp2)
		{
			WRect rc(ox + 268.0, oy + 417.0, pHelp2->Width(), pHelp2->Height());
			pGDI->DrawTexture(pHelp2, rc, 0xffffffff, 0);
		}
		if (m_pItem)
		{
			IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_pItem->tid);
			pGDI->SetTextColor(0xff887966, 0xffffffff);
			if (m_pCommon)
			{
				WPoint pt(ox + 93.0, oy + 436.0);
				pGDI->Print(pt, 0, m_pCommon->Name);
			}
			if (pDesc)
			{
				// 983: 5 linhas de 128 bytes (0x280); '%' antes da quebra e mantido
				char lines[5][128];
				memset(lines, 0, sizeof(lines));
				const char* src = pDesc->Desc;
				unsigned int len = strlen(src);
				char* tmp = new char[len + 1];
				memset(tmp, 0, len + 1);
				if (tmp)
				{
					strcpy(tmp, src);
					unsigned int line = 0;
					unsigned int col = 0;
					char* flat = &lines[0][0];
					for (unsigned int i = 0; i < len; i++)
					{
						flat[line + col] = src[i];
						col++;
						if (tmp[i] == '\n' || (col > 40 && tmp[i] == ' '))
						{
							if (tmp[i - 1] == '%')
								flat[line + col - 1] = '%';
							else
								flat[line + col - 1] = 0;
							line += 0x80;
							col = 0;
						}
						if (line >= 0x280)
							break;
					}
					WPoint pt1(ox + 93.0, oy + 452.0);
					pGDI->Print(pt1, 0, lines[0]);
					for (int k = 1; k < 5; k++)
					{
						WPoint ptn(ox + 45.0, oy + 452.0 + k * 12);
						pGDI->Print(ptn, 0, lines[k]);
					}
					delete[] tmp;
				}
			}
			pGDI->SetTextColor(0xff000000, 0xffffffff);
		}
	}
}

bool CCardSlot::BoundControl(FrButton* pButton)
{
	m_pButton = pButton;
	if (pButton)
	{
		if (m_pCommon)
		{
			m_pButton->SetButtonImg(m_pCommon->Icon, FrButton::NORMAL);
			m_pButton->SetButtonImg(m_pCommon->Icon, FrButton::OVER);
			m_pButton->SetButtonImg(m_pCommon->Icon, FrButton::PRESSED);
		}
		m_pButton->SetVisible(false);
		m_pButton->SetPushDelay(0.1f);
		m_pButton->UseDblClick(true);
	}
	return m_pButton ? true : false;
}

bool CCardPackPage::Init()
{
	m_pSlots = new CCardSlot[8];
	struct
	{
		unsigned long tid;
		float x;
		float y;
		float w;
		float h;
	} info[8] = {
		{ 0x7cc00000, 439.0f, 247.0f },
        { 0x7cc00004, 521.0f, 247.0f },
		{ 0,          603.0f, 247.0f },
        { 0,          685.0f, 247.0f },
		{ 0x7cc00001, 439.0f, 377.0f },
        { 0x7cc00002, 521.0f, 377.0f },
		{ 0x7cc00003, 603.0f, 377.0f },
        { 0,          685.0f, 377.0f }
	};
	if (m_pCardPackList == NULL)
	{
		m_pCardPackList = new std::vector<unsigned long>;
		if (m_pCardPackList)
			m_pCardPackList->clear();
	}
	for (int i = 0; i < 8; i++)
	{
		if (m_pCardPackList)
			m_pCardPackList->push_back(info[i].tid);
		m_pSlots[i].SetCard(0);
		WRect rc(info[i].x, info[i].y, 0, 0);
		m_pSlots[i].SetPosition(rc);
		m_pSlots[i].Select(0);
	}
	return false;
}

void CCardPackPage::Draw(FrGraphicInterface* pGDI)
{
	for (int i = 0; i < 8; i++)
	{
		if (m_pSlots)
		{
			m_pSlots[i].Draw(pGDI);
			sCards* pItem = m_pSlots[i].m_pItem;
			if (pItem && m_pNumberFont)
			{
				WRect rc = m_pSlots[i].GetPosition();
				m_pNumberFont->Print(g_view, rc.x + 42.0f, rc.y + 50.0f,
					MakeStr("%d", pItem->cardCount), 0, 0xffffffff, NULL);
			}
		}
	}
}

void CCardPackPage::SetCard(unsigned char slot, unsigned long tid)
{
	if (m_pSlots)
		m_pSlots[slot].SetCard(tid);
}

void CCardPackPage::ShowSlotControl(bool bShow)
{
	for (int i = 0; i < 8; i++)
	{
		if (m_pSlots && m_pSlots[i].GetCardInfo())
			m_pSlots[i].Show(bShow);
	}
}

void CCardPackPage::SelectSlot(unsigned char type, unsigned char slot)
{
	if (m_pSlots)
	{
		unsigned char bUse = 0;
		if (type == 2)
			bUse = type = 1;
		for (int i = 0; i < 8; i++)
		{
			if (i == slot)
			{
				if (type != 1)
				{
					if (m_pSlots[i].IsPushed() != 1)
						m_pSlots[i].Select(type);
				}
				else if (m_pSlots[i].IsPushed() != 1)
					m_pSlots[i].Select(1);
			}
			else if (type != 1)
			{
				if (m_pSlots[i].IsPushed() != 1)
					m_pSlots[i].Select(type);
			}
			else
				m_pSlots[i].Select(0);
		}
		if (bUse == 1 && m_pSlots)
			m_pSlots[slot].UseCard();
	}
}

static inline bool CardTypeAllowed983(unsigned long tid)
{
	// 983: tipo ((tid >> 22) & 0xf) < 3 ou == 5 (no 645 era so < 3)
	unsigned long t = (tid >> 22) & 0xf;
	return t < 3 || t == 5;
}

CCardPage::CCardPage()
	: m_volume(0)
{
	m_pCardList[0] = NULL;
	m_pCardList[1] = NULL;
	m_pCardList[2] = NULL;
	m_pCardList[3] = NULL;
	m_pCardList[4] = NULL;
	m_pCardList[5] = NULL;
	m_pBackground = NULL;
}

CCardPage::~CCardPage()
{
	if (m_pCardList[0])
	{
		delete[] m_pCardList[0];
		m_pCardList[0] = NULL;
	}
	if (m_pCardList[1])
	{
		delete[] m_pCardList[1];
		m_pCardList[1] = NULL;
	}
	if (m_pCardList[2])
	{
		delete[] m_pCardList[2];
		m_pCardList[2] = NULL;
	}
	if (m_pCardList[3])
	{
		delete[] m_pCardList[3];
		m_pCardList[3] = NULL;
	}
	if (m_pCardList[4])
	{
		delete[] m_pCardList[4];
		m_pCardList[4] = NULL;
	}
	if (m_pCardList[5])
	{
		delete[] m_pCardList[5];
		m_pCardList[5] = NULL;
	}
}


bool CCardPage::Init()
{
	BuildCardList();
	m_pSlots = new CCardSlot[12];
	WRect pos[12];
	// 983 007f62c0: casas relativas ao card_book (no 645 eram absolutas 426..698 x 216..380); a origem
	// entra no desenho (s_cardPageOrigin983, gravada por CCardBook::DrawCard)
	pos[0] = WRect(47.0f, 69.0f, 0, 0);
	pos[1] = WRect(138.0f, 69.0f, 0, 0);
	pos[2] = WRect(228.0f, 69.0f, 0, 0);
	pos[3] = WRect(319.0f, 69.0f, 0, 0);
	pos[4] = WRect(47.0f, 151.0f, 0, 0);
	pos[5] = WRect(138.0f, 151.0f, 0, 0);
	pos[6] = WRect(228.0f, 151.0f, 0, 0);
	pos[7] = WRect(319.0f, 151.0f, 0, 0);
	pos[8] = WRect(47.0f, 233.0f, 0, 0);
	pos[9] = WRect(138.0f, 233.0f, 0, 0);
	pos[10] = WRect(228.0f, 233.0f, 0, 0);
	pos[11] = WRect(319.0f, 233.0f, 0, 0);
	for (int i = 0; i < 12; i++)
	{
		m_pSlots[i].SetCard(0);
		m_pSlots[i].SetPosition(pos[i]);
		m_pSlots[i].Select(0);
	}
	m_pBackground = g_pFresh->GetBitmap("collection_bg");
	return false;
}

void CCardPage::Draw(FrGraphicInterface* pGDI)
{
	unsigned int total = 0;
	unsigned long pages = GetPageCount(m_volume);
	for (unsigned int p = 0; p < pages; p++)
	{
		std::vector<unsigned long>* pList = GetCardList(m_volume);
		if (!pList)
			break;
		total += pList[p].size();
	}
	for (int i = 0; i < 12; i++)
	{
		CCardSlot* pSlots = m_pSlots;
		if (pSlots && total > (unsigned int)(i + m_page * 12))
		{
			if (m_pBackground && !pSlots[i].m_pItem) // 983: fundo so no slot vazio
			{
				WRect pos = pSlots[i].GetPosition();
				pGDI->DrawTexture(m_pBackground,
					WRect(s_cardPageOrigin983.x + pos.x, s_cardPageOrigin983.y + pos.y,
						m_pBackground->Width(),
						m_pBackground->Height()),
					0xffffffff, 0);
			}
			m_pSlots[i].Draw(pGDI);
			sCards* pItem = m_pSlots[i].m_pItem;
			if (pItem && m_pNumberFont)
			{
				WRect rc = m_pSlots[i].GetPosition();
				m_pNumberFont->Print(g_view, s_cardPageOrigin983.x + rc.x + 42.0f,
					s_cardPageOrigin983.y + rc.y + 50.0f,
					MakeStr("%d", pItem->cardCount), 0, 0xffffffff, NULL);
			}
		}
	}
}

void CCardPage::PageSeparate(std::vector<unsigned long>*& pList,
	unsigned char volume)
{
	if ((volume >= 1 && volume <= 5) || volume == 100)
	{
		unsigned int count = 0;
		for (std::map<unsigned int, IFF_STRUCT::sCard>::const_iterator it =
				 ItemManager()->m_CardMap.begin();
			it != ItemManager()->m_CardMap.end(); ++it)
		{
			IFF_STRUCT::sCard card = (*it).second;
			if (ItemManager()->FindCard((*it).first) && card.Volume == volume &&
				CardTypeAllowed983(card.c.TypeId))
				count++;
		}
		if (count != 0)
		{
			unsigned int pages = count / 12;
			if (count % 12 != 0)
				pages++;
			pList = new std::vector<unsigned long>[pages];
			unsigned int page = 0;
			unsigned int inPage = 0;
			for (std::map<unsigned int, IFF_STRUCT::sCard>::const_iterator it =
					 ItemManager()->m_CardMap.begin();
				it != ItemManager()->m_CardMap.end(); ++it)
			{
				IFF_STRUCT::sCard card = (*it).second;
				ItemManager()->FindCard((*it).first);
				unsigned long tid = (*it).first;
				if (card.Volume == volume &&
					CardTypeAllowed983(card.c.TypeId))
				{
					pList[page].push_back(tid);
					inPage++;
				}
				if (inPage == 12)
				{
					page++;
					inPage = 0;
					if (pages < page)
						return;
				}
			}
		}
	}
}

void CCardPage::BuildCardList()
{
	PageSeparate(m_pCardList[0], 1);
	PageSeparate(m_pCardList[1], 2);
	PageSeparate(m_pCardList[2], 3);
	PageSeparate(m_pCardList[3], 4);
	PageSeparate(m_pCardList[4], 5);
	PageSeparate(m_pCardList[5], 100);
}

std::vector<unsigned long>* CCardPage::GetCardList(unsigned char volume)
{
	std::vector<unsigned long>* pList = NULL;
	switch (volume)
	{
	case 1:
		return m_pCardList[0];
	case 2:
		return m_pCardList[1];
	case 3:
		return m_pCardList[2];
	case 4:
		return m_pCardList[3];
	case 5:
		return m_pCardList[4];
	case 100:
		pList = m_pCardList[5];
	}
	return pList;
}

unsigned long CCardPage::GetPageCount(unsigned char volume)
{
	// HACK: dead call that delays the emission order of this function.
	if (0)
		((CCardSlot*)this)->UseCard();
	unsigned int count = 0;
	for (std::map<unsigned int, IFF_STRUCT::sCard>::const_iterator it =
			 ItemManager()->m_CardMap.begin();
		it != ItemManager()->m_CardMap.end(); ++it)
	{
		IFF_STRUCT::sCard card = (*it).second;
		if (ItemManager()->FindCard((*it).first) && card.Volume == volume &&
			CardTypeAllowed983(card.c.TypeId))
			count++;
	}
	unsigned long pages = count / 12;
	if (count % 12 != 0)
		pages++;
	return pages;
}

void CCardPage::SetCard(unsigned char slot, unsigned long tid)
{
	if (m_pSlots)
		m_pSlots[slot].SetCard(tid);
}

void CCardPage::ShowSlotControl(bool bShow)
{
	for (int i = 0; i < 12; i++)
	{
		if (m_pSlots)
			m_pSlots[i].Show(bShow);
	}
}

void CCardPage::SelectSlot(unsigned char type, unsigned char slot)
{
	if (m_pSlots)
	{
		unsigned char bUse = 0;
		if (type == 2)
			bUse = type = 1;
		for (int i = 0; i < 12; i++)
		{
			if (i == slot)
			{
				if (type != 1)
				{
					if (m_pSlots[i].IsPushed() != 1)
						m_pSlots[i].Select(type);
				}
				else if (m_pSlots[i].IsPushed() != 1)
					m_pSlots[i].Select(1);
			}
			else if (type != 1)
			{
				if (m_pSlots[i].IsPushed() != 1)
					m_pSlots[i].Select(type);
			}
			else
				m_pSlots[i].Select(0);
		}
		// 983: o botao Use e liberado por 009d9d40 (tid != 0x7c800026 && != 0x7c800041)
		if (bUse == 1 && m_pSlots)
		{
			unsigned long tid = m_pSlots[slot].m_pItem->tid;
			if (tid != 0x7c800026 && tid != 0x7c800041)
				m_pSlots[slot].UseCard();
		}
	}
}

CCardBook::CCardBook()
{
	ClearVariables();
	LoadImages();
	m_pPages[0] = new CCardPackPage;
	m_pPages[1] = new CCardPage;
	m_pPages[0]->Init();
	m_pPages[1]->Init();
}

CCardBook::~CCardBook()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_pPages[i])
		{
			delete m_pPages[i];
			m_pPages[i] = NULL;
		}
	}
}

void CCardBook::ClearVariables()
{
	m_bShow = false;
	m_bShowProperty = false;
	m_tab = 0;
	m_pageCount = 0;
	m_curPage = 0;
	m_hoverPage = 0;
	m_selectedSlot = 0;
	m_unusedList.clear();
	m_pCurCardList = NULL;
	m_pageIndexRects.clear();
	m_pBackground = NULL;
	m_pProperty = NULL;
	m_pIndexList = NULL;
	m_pTabButton[0] = NULL;
	m_pTabButton[1] = NULL;
	m_pTabButton[2] = NULL;
	m_pTabButton[3] = NULL;
	m_pTabButton[4] = NULL;
	m_pTabButton[5] = NULL;
	m_pTabButton[6] = NULL;
	m_pUseButton = NULL;
	m_pBgBitmap[0] = NULL;
	m_pCardPackBitmap[0] = NULL;
	m_pCollectionBitmap[0] = NULL;
	m_pBgBitmap[1] = NULL;
	m_pCardPackBitmap[1] = NULL;
	m_pCollectionBitmap[1] = NULL;
	m_pBgBitmap[2] = NULL;
	m_pCardPackBitmap[2] = NULL;
	m_pCollectionBitmap[2] = NULL;
	m_pBgBitmap[3] = NULL;
	m_pCardPackBitmap[3] = NULL;
	m_pCollectionBitmap[3] = NULL;
	m_pPages[0] = NULL;
	m_pPages[1] = NULL;
}

void CCardBook::LoadImages()
{
	m_pBgBitmap[0] = g_pFresh->GetBitmap("cardbook_bg1");
	m_pBgBitmap[1] = g_pFresh->GetBitmap("cardbook_bg2");
	m_pBgBitmap[2] = g_pFresh->GetBitmap("cardbook_bg3");
	m_pBgBitmap[3] = g_pFresh->GetBitmap("cardbook_bg4");
	m_pCardPackBitmap[0] = g_pFresh->GetBitmap("cardpack1");
	m_pCardPackBitmap[1] = g_pFresh->GetBitmap("cardpack2");
	m_pCardPackBitmap[2] = g_pFresh->GetBitmap("cardpack3");
	m_pCardPackBitmap[3] = g_pFresh->GetBitmap("cardpack4");
	m_pCollectionBitmap[0] = g_pFresh->GetBitmap("collection1");
	m_pCollectionBitmap[1] = g_pFresh->GetBitmap("collection2");
	m_pCollectionBitmap[2] = g_pFresh->GetBitmap("collection3");
	m_pCollectionBitmap[3] = g_pFresh->GetBitmap("collection4");
}

void CCardBook::ShowCardProperty(bool bShow)
{
	m_bShowProperty = bShow;
}

void CCardBook::DrawCard(FrGraphicInterface* pGDI)
{
	if (m_pPages[1])
	{
		// 983: posicoes relativas ao fundo (m_pBackground), nao absolutas
		const WRect& bg = m_pBackground->GetRect();
		const Bitmap* pBitmap = m_pCollectionBitmap[0];
		pGDI->DrawTexture(pBitmap, WRect(bg.x + 33.0, bg.y + 50.0,
			pBitmap->Width(), pBitmap->Height()), 0xffffffff, 0);
		pBitmap = m_pCollectionBitmap[1];
		pGDI->DrawTexture(pBitmap, WRect(bg.x + 289.0, bg.y + 50.0,
			pBitmap->Width(), pBitmap->Height()), 0xffffffff, 0);
		pBitmap = m_pCollectionBitmap[2];
		pGDI->DrawTexture(pBitmap, WRect(bg.x + 33.0, bg.y + 306.0,
			pBitmap->Width(), pBitmap->Height()), 0xffffffff, 0);
		pBitmap = m_pCollectionBitmap[3];
		pGDI->DrawTexture(pBitmap, WRect(bg.x + 289.0, bg.y + 306.0,
			pBitmap->Width(), pBitmap->Height()), 0xffffffff, 0);
		CCardPage* pPage = (CCardPage*)m_pPages[1];
		// A300: as casas do 983 sao relativas ao card_book (007f62c0)
		s_cardPageOrigin983 = WPoint(bg.x, bg.y);
		if (pPage->m_pSlots)
		{
			std::vector<unsigned long>* pList = pPage->GetCardList(m_tab);
			unsigned int total = 0;
			if (pList)
			{
				unsigned long pages = pPage->GetPageCount(m_tab);
				for (unsigned long p = 0; p < pages; p++)
					total += pList[p].size();
			}
			for (int i = 0; i < 12; i++)
			{
				unsigned int idx = i + 1 + m_curPage * 12;
				if (total < idx)
					break;
				WPoint slotPos(pPage->m_pSlots[i].GetPosition().x,
					pPage->m_pSlots[i].GetPosition().y);
				pGDI->SetTextColor(0xff887966, 0xffffffff);
				pGDI->SetTextStyle(1);
				WPoint pt(bg.x + slotPos.x - 2.0f, bg.y + slotPos.y + 1.0f);
				pGDI->Print(pt, 2, "%d", idx);
				pGDI->SetTextColor(0xff000000, 0xffffffff);
				pGDI->SetTextStyle(0);
			}
		}
		m_pPages[1]->Draw(pGDI);
		s_cardPageOrigin983 = WPoint(0.0f, 0.0f);
		DrawPageIndex(pGDI);
	}
}

void CCardBook::DrawPageIndex(FrGraphicInterface* pGDI)
{
	unsigned int i = 0;
	if (m_pageIndexRects.size() != 0)
	{
		for (; i < m_pageIndexRects.size(); i++)
		{
			WRect rc = m_pageIndexRects[i];
			if (i == m_curPage)
			{
				pGDI->SetTextColor(0xffff0000, 0xffffffff);
				pGDI->SetTextStyle(1);
			}
			else if (i == m_hoverPage)
			{
				pGDI->SetTextColor(0xffaa1111, 0xffffffff);
				pGDI->SetTextStyle(1);
			}
			else
			{
				pGDI->SetTextColor(0xff000000, 0xffffffff);
				pGDI->SetTextStyle(0);
			}
			WPoint pt((rc.w - rc.x) * 0.5f + rc.x, rc.y);
			pGDI->Print(pt, 1, "%d", i + 1);
			pGDI->SetTextColor(0xff000000, 0xffffffff);
			pGDI->SetTextStyle(0);
			if (i + 1 < m_pageIndexRects.size())
			{
				WPoint pt2(rc.w, rc.y);
				pGDI->Print(pt2, 1, "|");
			}
		}
	}
}

void CCardBook::Show(bool bShow)
{
	m_bShow = bShow;
	if (m_pBackground)
	{
		m_pBackground->SetVisible(bShow);
		// 983 007f6d20: as 7 abas (Pack, Vol.1..5, SP); o 645 tinha 3 e Vol.3..SP ficavam no My Room (C233.1)
		for (int i = 0; i < 7; i++)
		{
			if (m_pTabButton[i])
				m_pTabButton[i]->SetVisible(m_bShow);
		}
		if (m_bShow == 1)
			ButtonTabControl(0);
	}
	m_bShowProperty = bShow;
	if (m_pProperty)
		m_pProperty->SetVisible(bShow);
	ShowSlotControl(m_bShow);
	if (!bShow)
		m_pUseButton->SetVisible(false);
	if (m_pIndexList)
		m_pIndexList->SetVisible(bShow);
}

void CCardBook::Draw()
{
	if (m_bShow)
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
		if (pGDI)
		{
			if (m_tab)
				DrawCard(pGDI);
		}
	}
}

// 007f33b0: fundo do caderno (cardbook_bg1..4 em 2x2 de 256 px, a partir do card_book); OWNERDRAW do card_book_back_bg (006056e0)
void CCardBook::DrawBackground()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI || !m_pBackground || !m_pBackground->IsVisible())
		return;
	const WRect& bg = m_pBackground->GetRect();
	static const float pos[4][2] = { { 0.0f, 0.0f }, { 256.0f, 0.0f }, { 0.0f, 256.0f }, { 256.0f, 256.0f } };
	for (int i = 0; i < 4; i++)
	{
		const Bitmap* pBitmap = m_pBgBitmap[i];
		pGDI->DrawTexture(pBitmap, WRect(bg.x + pos[i][0], bg.y + pos[i][1],
			pBitmap->Width(), pBitmap->Height()), 0xffffffff, 0);
	}
}

void CCardBook::ReBuildCurrentPage()
{
	ICardBookPage* pPage;
	if (m_tab == 0)
	{
		BuildCardPackTab();
		pPage = m_pPages[0];
	}
	else
	{
		BuildCardPage(m_curPage);
		pPage = m_pPages[1];
	}
	if (pPage && m_pUseButton)
		m_pUseButton->SetVisible(false);
}

void CCardBook::BuildCardPackTab()
{
	CCardPackPage* pPage = (CCardPackPage*)m_pPages[0];
	if (pPage)
	{
		std::vector<unsigned long>* pList = pPage->GetCardPackList();
		if (pList)
		{
			int i = 0;
			for (std::vector<unsigned long>::iterator it = pList->begin();
				it != pList->end(); ++it)
			{
				unsigned long tid = *it;
				for (std::list<sCards>::iterator cit = CARD_LIST().begin();
					cit != CARD_LIST().end(); ++cit)
				{
					if ((*cit).tid == tid)
					{
						pPage->SetCard(i, (*cit).tid);
						break;
					}
					pPage->SetCard(i, 0);
				}
				i++;
				if (i == 8)
					break;
			}
			if (m_pUseButton)
				m_pUseButton->SetVisible(false);
		}
	}
}

void CCardBook::BuildCardTab(unsigned long volume)
{
	if (volume != 0)
	{
		CCardPage* pPage = (CCardPage*)m_pPages[1];
		m_pageIndexRects.clear();
		if (pPage)
		{
			pPage->m_volume = volume;
			m_pCurCardList = pPage->GetCardList(volume);
			if (m_pCurCardList != NULL)
			{
				unsigned long pages = pPage->GetPageCount(volume);
				m_pageCount = pages;
				m_curPage = 0;
				BuildCardPage(0);
				// 983: relativo ao fundo (m_pBackground), e pages == 100 vira 6
				float bgY = m_pBackground->GetRect().y + 345.0;
				float bgX = m_pBackground->GetRect().x + 207.0;
				float width = (float)pages * 20.0f;
				float base = bgX - width * 0.5f;
				if (pages == 100)
					pages = 6;
				WRect rect;
				for (unsigned int i = 0; i < pages; i++)
				{
					rect = WRect(base + (float)i * 20.0f, bgY,
						(float)i * 20.0f + base + 20.0f, bgY + 10.0f);
					m_pageIndexRects.push_back(rect);
				}
			}
			else
				BuildCardPage(0xff);
			if (m_pUseButton)
				m_pUseButton->SetVisible(false);
		}
	}
}

void CCardBook::BuildCardPage(unsigned long page)
{
	if (page != 0xff)
	{
		if (m_pCurCardList == NULL)
			return;
		CCardPage* pPage = (CCardPage*)m_pPages[1];
		unsigned long pages = pPage->GetPageCount(m_tab);
		if (pages - 1 < page)
			return;
		m_curPage = page;
		if (pPage)
		{
			pPage->m_page = page;
			int slot = 0;
			for (int i = 0; i < 12; i++)
				pPage->SetCard(i, 0);
			for (std::vector<unsigned long>::iterator it =
					 m_pCurCardList[page].begin();
				it != m_pCurCardList[page].end(); ++it)
			{
				unsigned long tid = *it;
				for (std::list<sCards>::iterator cit = CARD_LIST().begin();
					cit != CARD_LIST().end(); ++cit)
				{
					if ((*cit).tid == tid)
					{
						pPage->SetCard(slot, (*cit).tid);
						break;
					}
					pPage->SetCard(slot, 0);
				}
				slot++;
			}
		}
	}
	else
	{
		ICardBookPage* pPage = m_pPages[1];
		if (pPage)
		{
			for (int i = 0; i < 12; i++)
				pPage->SetCard(i, 0);
		}
	}
	if (m_pUseButton)
		m_pUseButton->SetVisible(false);
}

void CCardBook::ShowSlotControl(bool bShow)
{
	ICardBookPage* pPack = m_pPages[0];
	ICardBookPage* pCard = m_pPages[1];
	if (bShow == true)
	{
		if (m_tab == 0)
		{
			if (pPack)
				pPack->ShowSlotControl(true);
			if (pCard)
				pCard->ShowSlotControl(false);
			return;
		}
		if (pPack)
			pPack->ShowSlotControl(false);
		if (pCard)
			pCard->ShowSlotControl(true);
		return;
	}
	if (pPack)
		pPack->ShowSlotControl(false);
	if (pCard)
		pCard->ShowSlotControl(false);
}

void CCardBook::UseCard()
{
	ICardBookPage* pPage;
	if (m_tab == 0)
		pPage = m_pPages[0];
	else
		pPage = m_pPages[1];
	if (pPage)
		pPage->SelectSlot(2, m_selectedSlot);
}

void CCardBook::CardSlotControl(unsigned char page, unsigned char slot,
	unsigned char type)
{
	if (m_pPages[page])
		m_pPages[page]->SelectSlot(type, slot);
}

void CCardBook::ButtonTabControl(unsigned char tab)
{
	m_tab = tab;
	if (tab != 0)
		BuildCardTab(tab);
	else
		BuildCardPackTab();
	// 983 007f6c10: 7 botoes (Pack, Vol1..5, SP em +0x88); SP chega como 100 e vira 6 no fim
	for (int i = 0; i < 7; i++)
	{
		if (m_pTabButton[i] == NULL)
			return;
		if (i == 0)
		{
			if (tab == 0)
			{
				m_pTabButton[i]->SetButtonImg("tab_pack_d", FrButton::NORMAL);
				m_pTabButton[i]->SetButtonImg("tab_pack_d", FrButton::OVER);
			}
			else
			{
				m_pTabButton[i]->SetButtonImg("tab_pack_n", FrButton::NORMAL);
				m_pTabButton[i]->SetButtonImg("tab_pack_o", FrButton::OVER);
			}
		}
		else
		{
			if (i == tab)
			{
				m_pTabButton[i]->SetButtonImg(MakeStr("tab_vol%d_d", i),
					FrButton::NORMAL);
				m_pTabButton[i]->SetButtonImg(MakeStr("tab_vol%d_d", i),
					FrButton::OVER);
			}
			else
			{
				m_pTabButton[i]->SetButtonImg(MakeStr("tab_vol%d_n", i),
					FrButton::NORMAL);
				m_pTabButton[i]->SetButtonImg(MakeStr("tab_vol%d_o", i),
					FrButton::OVER);
			}
		}
	}
	if (m_tab == 100)
	{
		m_tab = 6;
		m_pTabButton[6]->SetButtonImg("tab_sp_n", FrButton::NORMAL);
		m_pTabButton[6]->SetButtonImg("tab_sp_o", FrButton::OVER);
	}
}

void CCardBook::PageIndexBehavior(const WPoint& pos, unsigned char type)
{
	unsigned char idx = 0;
	if (m_pageIndexRects.size() != 0)
	{
		std::vector<WRect>::iterator it;
		for (it = m_pageIndexRects.begin(); it != m_pageIndexRects.end(); ++it)
		{
			if ((*it).x <= pos.x && (*it).y <= pos.y && (*it).w >= pos.x &&
				(*it).h >= pos.y)
			{
				switch (type)
				{
				case 1:
					m_curPage = idx;
					BuildCardPage(idx);
					return;
				case 2:
					m_hoverPage = idx;
					return;
				}
				return;
			}
			idx++;
		}
		m_hoverPage = idx;
	}
}

void CCardBook::DrawCardPackHelp(unsigned long tid)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	// 983: coordenadas relativas ao FrTaskForm do task atual
	float ox = 0.0f;
	float oy = 0.0f;
	FrTaskForm* pTaskForm = AfxGetTask()->GetWidgetForm983();
	if (pTaskForm)
	{
		ox = pTaskForm->GetRect().x;
		oy = pTaskForm->GetRect().y;
	}
	const Bitmap* pHelp1 = g_pFresh->GetBitmap("help_1");
	if (pHelp1)
	{
		WRect rc(ox + 12.0, oy + 417.0, pHelp1->Width(), pHelp1->Height());
		pGDI->DrawTexture(pHelp1, rc, 0xffffffff, 0);
	}
	const Bitmap* pHelp2 = g_pFresh->GetBitmap("help_2");
	if (pHelp2)
	{
		WRect rc(ox + 268.0, oy + 417.0, pHelp2->Width(), pHelp2->Height());
		pGDI->DrawTexture(pHelp2, rc, 0xffffffff, 0);
	}
	pGDI->SetTextColor(0xff887966, 0xffffffff);
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(tid);
	IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(tid);
	if (pCard)
	{
		WPoint pt(ox + 93.0, oy + 436.0);
		pGDI->Print(pt, 0, pCard->c.Name);
	}
	if (pDesc)
	{
		char desc[512] = "\0";
		char lines[3][128];
		unsigned int len = strlen(pDesc->Desc);
		strcpy(desc, pDesc->Desc);
		memset(lines, 0, sizeof(lines));
		unsigned int line = 0;
		unsigned int col = 0;
		for (unsigned int i = 0; i < len; i++)
		{
			unsigned char c = desc[i];
			if (c == '\n')
				continue;
			lines[0][line + col] = c;
			col++;
			if (col > 0x2a && len > i + 1)
			{
				// byte de abertura de caractere de 2 bytes (0x81-0x9f, 0xe0-0xfc)
				if ((c >= 0x80 && c < 0xa0) || (unsigned char)(c + 0x20) <= 0x1c)
				{
					i++;
					lines[0][line + col] = desc[i];
					col++;
				}
				lines[0][line + col] = 0;
				line += 0x80;
				col = 0;
			}
			if (line >= 0x100)
				break;
		}
		WPoint pt1(ox + 93.0, oy + 452.0);
		pGDI->Print(pt1, 0, lines[0]);
		WPoint pt2(ox + 45.0, oy + 464.0);
		pGDI->Print(pt2, 0, lines[1]);
	}
	pGDI->SetTextColor(0xff000000, 0xffffffff);
}
