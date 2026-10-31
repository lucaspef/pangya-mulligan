// JP 983: unidade 0050f1e0–005100c0 (FrGrandPrixRoomDlg). Fonte reconstruída do 983 (classe nova): a lista de salas do Grand Prix.
// Sessão B69: tudo da classe — mapa de mensagens (19 entradas), construtor, destrutor, registro, OnInit, OnProc, a preparação da
// lista (cópia da FrMultiplayRoomDlg sem o S4_UCC), os botões de canal, o relógio e as duas funções livres de "má conduta".
// Ver include/jp/grandprixroomdlg.h.
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "taskmanager.h"
#include "netresourcemanager.h"
#include "packet.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "frgraphicinterface.h"
#include "soundmanager.h"
#include "bitmap.h"
#include "titlefontex.h"
#include "commonutil.h"
#include "lobbyavataractor.h"
#include "gproomtimersteward.h"
#include "grandprixroomdlg.h"
#include "createcharacteroption.h"

extern Fresh* g_pFresh; // 00f02340

static const char* s_gateway = "Gateway"; // 00e09064

#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))
// empréstimos da frente A sem nome no mapa (nomes meus, pelo endereço)
class CSharedDoc983b
{
public:
	void SetGPChannel983(int channel); // 0062bf20: grava o canal do Grand Prix (o 0062bf30 lê)
	void SetRoomListKind983(int a); // 0062bf40
};
#define DOCJP983B() ((CSharedDoc983b*)Doc())

IMPLEMENT_OBJECT(FrGrandPrixRoomDlg, FrRoomListDlg)

BEGIN_FRESH_MSGMAP(FrGrandPrixRoomDlg, FrRoomListDlg)

ON_FRESH_VI("roomlist", FRCMD_INIT, FrGrandPrixRoomDlg::OnRoomListInit)
ON_FRESH_VI("roomlist", FRCMD_OWNERDRAW, FrGrandPrixRoomDlg::OnRoomListOwnerDraw)
ON_FRESH_VV("roomlist", FRCMD_LBUTTONDOWN, FrGrandPrixRoomDlg::OnRoomListLDown)
ON_FRESH_VV("roomlist", FRCMD_RBUTTONUP, FrGrandPrixRoomDlg::OnRoomListRUp)
ON_FRESH_VV("roomlist", FRCMD_DBLCLICK, FrGrandPrixRoomDlg::OnRoomListDClick)
ON_FRESH_VI("btn_sort_event", FRCMD_INIT, FrGrandPrixRoomDlg::OnSortEventInit)
ON_FRESH_VV("btn_sort_event", FRCMD_LBUTTONUP, FrGrandPrixRoomDlg::OnSortEventUp)
ON_FRESH_VI("btn_sort_rookie", FRCMD_INIT, FrGrandPrixRoomDlg::OnSortRookieInit)
ON_FRESH_VV("btn_sort_rookie", FRCMD_LBUTTONUP, FrGrandPrixRoomDlg::OnSortRookieUp)
ON_FRESH_VI("btn_sort_c1", FRCMD_INIT, FrGrandPrixRoomDlg::OnSortC1Init)
ON_FRESH_VV("btn_sort_c1", FRCMD_LBUTTONUP, FrGrandPrixRoomDlg::OnSortC1Up)
ON_FRESH_VI("btn_sort_c2", FRCMD_INIT, FrGrandPrixRoomDlg::OnSortC2Init)
ON_FRESH_VV("btn_sort_c2", FRCMD_LBUTTONUP, FrGrandPrixRoomDlg::OnSortC2Up)
ON_FRESH_VI("btn_sort_c3", FRCMD_INIT, FrGrandPrixRoomDlg::OnSortC3Init)
ON_FRESH_VV("btn_sort_c3", FRCMD_LBUTTONUP, FrGrandPrixRoomDlg::OnSortC3Up)
ON_FRESH_VI("gp_limited_tooltip", FRCMD_INIT, FrGrandPrixRoomDlg::OnLimitedTooltipInit)
ON_FRESH_VI("gpstate_ticket", FRCMD_INIT, FrGrandPrixRoomDlg::OnStateTicketInit)
ON_FRESH_VI("gpstate", FRCMD_INIT, FrGrandPrixRoomDlg::OnStateInit)
ON_FRESH_VI("gpstate", FRCMD_OWNERDRAW, FrGrandPrixRoomDlg::OnStateOwnerDraw)

END_FRESH_MSGMAP()

// 0050f1f0
void FrGrandPrixRoomDlg::OnProc(const float deltaTime)
{
	FrRoomListDlg::OnProc(deltaTime);
	ResetControlsRoomListSort(deltaTime);
}

// 0050f220
void FrGrandPrixRoomDlg::OnRoomListLDown()
{
	m_pRoom->OnRoomListLDown983(this);
}

// 0050f230
void FrGrandPrixRoomDlg::OnRoomListRUp()
{
	m_pRoom->OnRoomListRUp983(this);
}

// 0050f240
void FrGrandPrixRoomDlg::OnRoomListDClick()
{
	m_pRoom->OnRoomListDClick983(this);
}

// 0050f270
void FrGrandPrixRoomDlg::Leave983()
{
	g_audio->StopSfx("gp_lobby_amb");
	if (CGrandPrixClock::Instance())
		CGrandPrixClock::Instance()->m_bShow = 0;
}

// 0050f2a0
void FrGrandPrixRoomDlg::OnRoomListInit(int param)
{
	m_pRoomList2 = DYNAMIC_CAST(FrListBox, param);
	if (m_pRoomList2)
	{
		m_pRoomList2->UseDummy(true);
		m_pRoomList2->UseRightButton(true);
	}
}

// 0050f330: linha vazia desliga a dica da lista (FWF_TOOLTIPS); o desenho é do objeto da sala
void FrGrandPrixRoomDlg::OnRoomListOwnerDraw(int param)
{
	if (!param)
	{
		m_pRoomList2->m_nFlags.Disable(FWF_TOOLTIPS);
		return;
	}
	m_pRoom->OnRoomListOwnerDraw983(param, this);
}

// 0050f360
void FrGrandPrixRoomDlg::OnSortEventInit(int param)
{
	m_pGPSortBtn[0] = DYNAMIC_CAST(FrButton, param);
}

// 0050f3d0
void FrGrandPrixRoomDlg::OnSortRookieInit(int param)
{
	m_pGPSortBtn[1] = DYNAMIC_CAST(FrButton, param);
}

// 0050f440
void FrGrandPrixRoomDlg::OnSortC1Init(int param)
{
	m_pGPSortBtn[2] = DYNAMIC_CAST(FrButton, param);
}

// 0050f4b0
void FrGrandPrixRoomDlg::OnSortC2Init(int param)
{
	m_pGPSortBtn[3] = DYNAMIC_CAST(FrButton, param);
}

// 0050f520
void FrGrandPrixRoomDlg::OnSortC3Init(int param)
{
	m_pGPSortBtn[4] = DYNAMIC_CAST(FrButton, param);
}

// 0050f590
void FrGrandPrixRoomDlg::OnLimitedTooltipInit(int param)
{
	m_pLimitedTooltip = DYNAMIC_CAST(FrArea, param);
}

// 0050f600
void FrGrandPrixRoomDlg::OnStateTicketInit(int param)
{
	m_pStateTicket = DYNAMIC_CAST(FrArea, param);
}

// 0050f670
void FrGrandPrixRoomDlg::OnStateInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, param);
	float y = pArea->GetRect().y + 9.0;
	CGrandPrixClock* pClock = CGrandPrixClock::Instance();
	pClock->m_x = pArea->GetRect().x + 10.0;
	pClock->m_y = y;
}

// 0050f700
void FrGrandPrixRoomDlg::OnStateOwnerDraw(int param)
{
	CGrandPrixClock* pClock = CGrandPrixClock::Instance();
	FrGraphicInterface* pGdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* pBmp = g_pFresh->GetBitmap("gp_txt_clock");
	pGdi->DrawTexture(pBmp, WRect(pClock->m_x + 45.0, pClock->m_y - 24.0, (float)pBmp->Width(), (float)pBmp->Height()),
		0xffffffff, 0);
	CGrandPrixClock::Instance()->Draw983(pGdi);
	((WTitleFontEx*)m_154)->Print(pClock->m_x + 185.0, pClock->m_y + 5.0, MakeStr(" %d", m_2f4));
}

// 0050f800
bool IsNoMannerPart983(unsigned long tid)
{
	return CreateCharacterOption::Instance()->IsPartGroupA983(tid);
}

// 0050f810
bool IsGoodMannerRate983()
{
	float rate = (float)Doc()->m_myInfo.stat.dwNoMannerGameCount /
				 (float)(Doc()->m_myInfo.stat.dwGameCount < 1 ? 1 : Doc()->m_myInfo.stat.dwGameCount) * 100.0f;
	if (rate < 3.0f)
		return true;
	return false;
}

// 0050f8a0
FrGrandPrixRoomDlg::~FrGrandPrixRoomDlg()
{
	Leave983();
}

// 0050f940
void FrGrandPrixRoomDlg::SelectChannel983(int channel)
{
	for (int i = 0; i < 5; i++)
		m_pGPSortBtn[i]->SetStatus(FrButton::NORMAL);

	switch (channel)
	{
	case 99:
		m_pGPSortBtn[0]->SetStatus(FrButton::PRESSED);
		break;
	case 0:
		m_pGPSortBtn[1]->SetStatus(FrButton::PRESSED);
		break;
	case 1:
		m_pGPSortBtn[2]->SetStatus(FrButton::PRESSED);
		break;
	case 2:
		m_pGPSortBtn[3]->SetStatus(FrButton::PRESSED);
		break;
	case 3:
		m_pGPSortBtn[4]->SetStatus(FrButton::PRESSED);
		break;
	}
	DOCJP983B()->SetGPChannel983(channel);
}

// 0050fa40
void FrGrandPrixRoomDlg::OnSortEventUp()
{
	SelectChannel983(99);
	m_pRoom->Fn5a0310_983(99, this);
}

// 0050fa60
void FrGrandPrixRoomDlg::OnSortRookieUp()
{
	SelectChannel983(0);
	m_pRoom->Fn5a0310_983(0, this);
}

// 0050fa80
void FrGrandPrixRoomDlg::OnSortC1Up()
{
	SelectChannel983(1);
	m_pRoom->Fn5a0310_983(1, this);
}

// 0050faa0
void FrGrandPrixRoomDlg::OnSortC2Up()
{
	SelectChannel983(2);
	m_pRoom->Fn5a0310_983(2, this);
}

// 0050fac0
void FrGrandPrixRoomDlg::OnSortC3Up()
{
	SelectChannel983(3);
	m_pRoom->Fn5a0310_983(3, this);
}

// 0050fb20: o "Gateway" escreve o número de salas em m_2f4 (aviso 0x753e / 4, texto 0x1a000264)
void FrGrandPrixRoomDlg::Enter983()
{
	WSendPacket packet((enumClientPacket)0x5c);
	packet.Send((eSendTo)0);
	int channel = DOCJP()->GetGPChannel983();
	m_pRoom->Fn5a0310_983(channel, this);
	SelectChannel983(channel);
	m_2f4 = 0;
	MsgObject msg(NULL, 0x753e, 4, 0x1a000264, (int)&m_2f4, 0, 0);
	AfxGetTask()->GetActor(s_gateway) << msg;
	CGrandPrixClock::Instance()->m_bShow = 1;
	g_audio->PlaySfx("gp_lobby_amb", NULL, 1, "gp_lobby_amb");
}

// 0050fc80: aba do Grand Prix apertada; a lista de jogadores fica 80 mais baixa
bool FrGrandPrixRoomDlg::OnInit()
{
	if (!FrRoomListDlg::OnInit())
		return false;
	*(int*)m_pRoom = 1;
	DOCJP983B()->SetRoomListKind983(1);
	m_pRoom->Init983(this);
	Enter983();
	if (m_pTabGpRoom)
		m_pTabGpRoom->SetStatus(FrButton::PRESSED);
	if (m_pUserList)
	{
		WRect rc;
		rc.x = m_pUserList->GetRect().x;
		rc.y = m_pUserList->GetRect().y;
		rc.w = m_pUserList->GetRect().w;
		rc.h = m_pUserList->GetRect().h - 80.0;
		m_pUserList->SetRect(rc);
	}
	return true;
}

// 0050fd90 (645: CLobbyMain::OnRoomList_Init; = FrMultiplayRoomDlg::InitRoomList983 com as imagens dos jogadores e sem o S4_UCC)
void FrGrandPrixRoomDlg::InitRoomList983()
{
	DOC983_U8(0x1094c) = 0x17;
	DOC983_U8(0x109b8) = 0x17;
	LoadUserImages983();

	for (std::map<unsigned long, sRoomSlot>::iterator it =
			 Doc()->m_roomSlotMap.begin();
		it != Doc()->m_roomSlotMap.end(); ++it)
	{
		if (it->second.pExhibition)
		{
			delete (CExhibition983*)it->second.pExhibition;
			it->second.pExhibition = NULL;
		}
	}
	Doc()->m_roomSlotMap.clear();

	NetResourceManager::Instance()->SetClearGarbage();

	if (!IsGoodMannerRate983())
	{
		CPartTidList& partTidList = Doc()->GetMyPartTidList();
		sCharacterInfo& charInfo =
			Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];

		for (int i = 0; i < partTidList.m_partNum; i++)
		{
			if (IsNoMannerPart983(charInfo.tidParts[i]))
			{
				charInfo.tidParts[i] = 0;
				charInfo.ItemIdList[i] = 0;
			}

			if (IsNoMannerPart983(partTidList.m_tid[i]))
				partTidList.m_tid[i] = 0;
		}
	}
}

// 0050ff90
FrGrandPrixRoomDlg::FrGrandPrixRoomDlg()
{
	m_pLimitedTooltip = NULL;
	m_pStateTicket = NULL;
	InitRoomList983();
}
