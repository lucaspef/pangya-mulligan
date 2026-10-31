// JP 983: base das salas do lobby (`CLobbyRoom`, 00598150–00598890) e o seletor guardado na FrRoomListDlg
// (`CLobbyRoomSelector983`, 005a01a0–005a0310 + construtor 005a0780). Ver include/jp/lobbyroom.h. A derivada CNormalLobbyRoom
// está em normallobbyroom.cpp (sessão B50); a CGrandPrixLobbyRoom ainda não foi escrita.
#include "stdafx.h"
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "icontentsdoc.h"
#include "contentsdoc.h"
#include "grandprixdocument.h"
// `DeleteScript` é privada no cabeçalho da frente A (nome do 645); o 983 chama de fora da classe em 005980d0 e 0059dbe0
// (pedido B → A da B50: tornar pública)
#define private public
#include "../../shared/LuaSystem/luascriptmanager.h"
#undef private
#include "../../shared/LuaSystem/luascript.h"
#include "lobbyroom.h"
#include "normallobbyroom.h"

#define DOCJP() ((CSharedDocJp*)Doc())

// 00e75ea4: script Lua das salas (carregado pelo 005980d0, a pedido do 9º virtual da CGrandPrixLobbyRoom e do 005a06ea), solto
// pelo destrutor da base. Sessão B50: o "00c20060 + 00c1f510" da B49 é `lua_script_manger::GetInstance()->ReleaseScript()`.
lua_system::lua_script* g_pLobbyRoomScript983; // 00e75ea4


// 005980d0: troca o script das salas (`name` em ecx)
void LoadLobbyRoomScript983(const char* name)
{
	if (g_pLobbyRoomScript983)
	{
		lua_system::lua_script_manger::GetInstance()->DeleteScript(g_pLobbyRoomScript983);
		g_pLobbyRoomScript983 = NULL;
	}
	g_pLobbyRoomScript983 = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (g_pLobbyRoomScript983)
		g_pLobbyRoomScript983->RunFile(name);
}

// 00598130
lua_system::lua_script* GetLobbyRoomScript983()
{
	return g_pLobbyRoomScript983;
}

// 00598140
CLobbyRoom::CLobbyRoom()
{
}

// 005981d0 (o 00598870 é o destrutor com `delete`)
CLobbyRoom::~CLobbyRoom()
{
	if (g_pLobbyRoomScript983)
	{
		lua_system::lua_script_manger::GetInstance()->ReleaseScript(g_pLobbyRoomScript983);
		g_pLobbyRoomScript983 = NULL;
	}
}

// 00598150
void CLobbyRoom::Init(FrRoomListDlg* pDlg)
{
}

// 00598160
void CLobbyRoom::AddRoom983(FrListBox* pList, int a)
{
}

// 00598170
void CLobbyRoom::DelRoom983(FrListBox* pList, int a)
{
}

// 00598180
void CLobbyRoom::OnRoomListOwnerDraw(int param, FrRoomListDlg* pDlg)
{
}

// 00598190
void CLobbyRoom::OnRoomListLDown(FrRoomListDlg* pDlg)
{
}

// 005981a0
void CLobbyRoom::OnRoomListRUp(FrRoomListDlg* pDlg)
{
}

// 005981b0
void CLobbyRoom::OnRoomListDClick(FrRoomListDlg* pDlg)
{
}

// 005981c0
void CLobbyRoom::LoadScript983()
{
}

// 005a0780: guarda o documento 0xa5 para as salas e cria uma sala de cada tipo, na ordem de `CSharedDoc+0x11630`
CLobbyRoomSelector983::CLobbyRoomSelector983(sRoomListDoc983* pDoc)
{
	m_type = 2;
	LoadRoomListLayoutThunk983();
	SetRoomListDoc983(pDoc);
	m_rooms.push_back(new CNormalLobbyRoom);
	m_rooms.push_back(new CGrandPrixLobbyRoom);
}

// 005a01c0
void CLobbyRoomSelector983::Init983(FrRoomListDlg* pDlg)
{
	m_rooms[GetType983()]->Init(pDlg);
}

// 005a01f0
void CLobbyRoomSelector983::Fn5a01f0_983(FrListBox* pList, int a)
{
	m_rooms[GetType983()]->AddRoom983(pList, a);
}

// 005a0220
void CLobbyRoomSelector983::Fn5a0220_983(FrListBox* pList, int a)
{
	m_rooms[GetType983()]->DelRoom983(pList, a);
}

// 005a0250
void CLobbyRoomSelector983::OnRoomListOwnerDraw983(int param, FrRoomListDlg* pDlg)
{
	m_rooms[GetType983()]->OnRoomListOwnerDraw(param, pDlg);
}

// 005a0280
void CLobbyRoomSelector983::OnRoomListLDown983(FrRoomListDlg* pDlg)
{
	m_rooms[GetType983()]->OnRoomListLDown(pDlg);
}

// 005a02b0
void CLobbyRoomSelector983::OnRoomListRUp983(FrRoomListDlg* pDlg)
{
	m_rooms[GetType983()]->OnRoomListRUp(pDlg);
}

// 005a02e0
void CLobbyRoomSelector983::OnRoomListDClick983(FrRoomListDlg* pDlg)
{
	m_rooms[GetType983()]->OnRoomListDClick(pDlg);
}

// 005a0310: passa o valor ao documento 0xaf (00559700) e inicia a sala do tipo atual
void CLobbyRoomSelector983::Fn5a0310_983(int a, FrRoomListDlg* pDlg)
{
	_gateway::_grandprix::CGrandPrixDocument* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);
	pDoc->Open983(a);
	Init983(pDlg);
}

// 005a06b0: os bitmaps e posições da lista do Grand Prix (CGrandPrixLobbyRoom 00599cd0) só existem depois disto
void CLobbyRoomSelector983::LoadScript983()
{
	for (std::vector<CLobbyRoom*>::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
		(*it)->LoadScript983();
}

// 005a06e0
void CLobbyRoomSelector983::ReloadScript983(const char* name)
{
	LoadLobbyRoomScript983(name);
	LoadScript983();
}
