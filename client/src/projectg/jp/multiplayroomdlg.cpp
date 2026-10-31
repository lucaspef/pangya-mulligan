// JP 983: unidade 00517140–00517740 (FrMultiplayRoomDlg). Fonte reconstruída do 983 (classe nova): a lista de salas do modo de
// vários jogadores. Sessão B48: tudo da classe — mapa de mensagens (5 entradas), construtor, registro, OnInit, OnProc, a
// preparação da lista (o `OnRoomList_Init` do CLobbyMain do 645) e os cinco handlers da lista "roomlist".
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "gamestringmanager.h"
#include "icontentsdoc.h"
#include "taskmanager.h"
#include "netresourcemanager.h"
#include "packet.h"
#include "wfont.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frviewer.h"
#include "frbutton.h"
#include "frgroupbox.h"
#include "frcombobox.h"
#include "frlistbox.h"
#include "frgaugebar.h"
#include "frstatic.h"
#include "fredit.h"
#include "frgraphicinterface.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "bitmap.h"
#include "changeroominfodlg.h"
#include "messengerdlg.h"
#include "userlistdlg.h"
#include "lobbyavataractor.h"
#include "user_info.h"
#include "lobbyjp.h"
#include "rftooltiphelper.h"
#include "multiplayroomdlg.h"
#include "createcharacteroption.h"

#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))
// empréstimos da frente A sem nome no mapa (nomes meus, pelo endereço)
class CSharedDoc983b
{
public:
	void SetRoomListKind983(int a); // 0062bf40
};
#define DOCJP983B() ((CSharedDoc983b*)Doc())

IMPLEMENT_OBJECT(FrMultiplayRoomDlg, FrRoomListDlg)

BEGIN_FRESH_MSGMAP(FrMultiplayRoomDlg, FrRoomListDlg)

ON_FRESH_VI("roomlist", FRCMD_INIT, FrMultiplayRoomDlg::OnRoomListInit)
ON_FRESH_VI("roomlist", FRCMD_OWNERDRAW, FrMultiplayRoomDlg::OnRoomListOwnerDraw)
ON_FRESH_VV("roomlist", FRCMD_LBUTTONDOWN, FrMultiplayRoomDlg::OnRoomListLDown)
ON_FRESH_VV("roomlist", FRCMD_RBUTTONUP, FrMultiplayRoomDlg::OnRoomListRUp)
ON_FRESH_VV("roomlist", FRCMD_DBLCLICK, FrMultiplayRoomDlg::OnRoomListDClick)

END_FRESH_MSGMAP()

// 005175f0
FrMultiplayRoomDlg::FrMultiplayRoomDlg()
{
	InitRoomList983();
}

// 00517150
FrMultiplayRoomDlg::~FrMultiplayRoomDlg()
{
}

// 00517180
void FrMultiplayRoomDlg::Refresh983()
{
	SetCateBtnByServerType983();
}

// 00517190
void FrMultiplayRoomDlg::OnRoomListLDown()
{
	m_pRoom->OnRoomListLDown983(this);
}

// 005171a0
void FrMultiplayRoomDlg::OnRoomListRUp()
{
	m_pRoom->OnRoomListRUp983(this);
}

// 005171b0
void FrMultiplayRoomDlg::OnRoomListDClick()
{
	m_pRoom->OnRoomListDClick983(this);
}

// 00517210: as teclas de subir e descer rolam a lista quando não há janela por cima
void FrMultiplayRoomDlg::OnProc(const float deltaTime)
{
	FrRoomListDlg::OnProc(deltaTime);
	if (m_pRoomList && !m_p14c)
	{
		if (g_input->GetDown("\xbb\xf3", false))
			m_pRoomList->GetScrollBar()->ScrollUp(1);
		if (g_input->GetDown("\xc7\xcf", false))
			m_pRoomList->GetScrollBar()->ScrollDown(1);
	}
	ResetControlsRoomListSort(deltaTime);
}

// 005172a0 (645: CLobbyMain::OnRoomList_RoomListInit, sem o enchimento da lista)
void FrMultiplayRoomDlg::OnRoomListInit(int param)
{
	m_pRoomList = DYNAMIC_CAST(FrListBox, param);
	if (m_pRoomList)
	{
		m_pRoomList->UseDummy(true);
		m_pRoomList->UseRightButton(true);
	}
}

// 00517330: linha vazia desliga a dica da lista (FWF_TOOLTIPS); o desenho é do objeto da sala
void FrMultiplayRoomDlg::OnRoomListOwnerDraw(int param)
{
	if (!param)
	{
		m_pRoomList->m_nFlags.Disable(FWF_TOOLTIPS);
		return;
	}
	m_pRoom->OnRoomListOwnerDraw983(param, this);
}

// 00517360
bool FrMultiplayRoomDlg::OnInit()
{
	if (!FrRoomListDlg::OnInit())
		return false;
	*(int*)m_pRoom = 0;
	DOCJP983B()->SetRoomListKind983(0);
	m_pRoom->Init983(this);
	Refresh983();
	PadRoomList983(0);
	if (m_pTabMtRoom)
		m_pTabMtRoom->SetStatus(FrButton::PRESSED);
	return true;
}

// 005173c0 (645: CLobbyMain::OnRoomList_Init)
void FrMultiplayRoomDlg::InitRoomList983()
{
	DOC983_U8(0x1094c) = 0x17;
	DOC983_U8(0x109b8) = 0x17;
	LoadRoomImages();

	for (std::map<unsigned long, sRoomSlot>::iterator it =
			 Doc()->m_roomSlotMap.begin();
		it != Doc()->m_roomSlotMap.end(); ++it)
	{
		if (it->second.pExhibition)
		{
			delete (CExhibition983*)it->second.pExhibition; // 983: destrutor virtual (classe do lobbyavataractor.h)
			it->second.pExhibition = NULL;
		}
	}
	Doc()->m_roomSlotMap.clear();

	if (IsLocalContent(S4_UCC))
	{
		NetResourceManager::Instance()->SetClearGarbage();
	}

	float rate = (float)Doc()->m_myInfo.stat.dwNoMannerGameCount /
				 (float)(Doc()->m_myInfo.stat.dwGameCount < 1 ? 1 : Doc()->m_myInfo.stat.dwGameCount) * 100.0f;
	if (!(rate < 3.0f))
	{
		CPartTidList& partTidList = Doc()->GetMyPartTidList();
		sCharacterInfo& charInfo =
			Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];

		for (int i = 0; i < partTidList.m_partNum; i++)
		{
			if (CreateCharacterOption::Instance()->IsPartGroupA983(charInfo.tidParts[i]))
			{
				charInfo.tidParts[i] = 0;
				charInfo.ItemIdList[i] = 0;
			}

			if (CreateCharacterOption::Instance()->IsPartGroupA983(partTidList.m_tid[i]))
				partTidList.m_tid[i] = 0;
		}
	}

	m_roomSort = ROOM_SORT_STATE;
	m_bRoomSortRev = false;
}
