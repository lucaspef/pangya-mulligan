#pragma once
// JP 983: as duas salas do lobby (derivadas da `CLobbyRoom`, include/jp/lobbyroom.h). São os `OnRoomList_RoomList*` do
// `CLobbyMain` do 645 mudados de classe: a `CNormalLobbyRoom` (vtable 00d0c5d0, 0x28 bytes) cuida da lista de salas comuns e
// a `CGrandPrixLobbyRoom` (vtable 00d0c588, 0x194 bytes) da lista do Grand Prix. As duas estão em
// src/projectg/jp/normallobbyroom.cpp (a Normal na sessão B50, a Grand Prix na B51 e o desenho da linha dela na B52).
#include <string>
#include "lobbyroom.h"

class WTitleFont;
struct sRoomInfo;

class CNormalLobbyRoom : public CLobbyRoom
{
public:
	CNormalLobbyRoom(); // 00599700
	virtual ~CNormalLobbyRoom(); // 005997d0 (com `delete`: 00599870)
	virtual void Init(FrRoomListDlg* pDlg); // 0059a250 (645: OnRoomList_RoomListInit)
	virtual void AddRoom983(FrListBox* pList, int room); // 005988a0
	virtual void DelRoom983(FrListBox* pList, int room); // 00598200
	virtual void OnRoomListOwnerDraw(int param, FrRoomListDlg* pDlg); // 0059a600 (645: OnRoomList_RoomListOwnerDraw)
	virtual void OnRoomListLDown(FrRoomListDlg* pDlg); // 00598910
	virtual void OnRoomListRUp(FrRoomListDlg* pDlg); // 00598e70
	virtual void OnRoomListDClick(FrRoomListDlg* pDlg); // 0059b980

protected:
	WTitleFont* m_pFont; // +0x04: números da sala ("1234567890ym%-.?/:,", 16×16)
	std::string m_tip; // +0x08: texto da dica do mapa personalizado (artefato da sala)
	int m_tipItem; // +0x24: índice da linha que está com a dica aberta (-1 = nenhuma)
};

// Sessões B51 e B52 (o desenho da linha, 0059c710). Os itens da lista são `sGPRoomItem983*` (normallobbyroom.cpp), feitos
// pelo documento 0xaf do Grand Prix; a sala só escolhe, avisa o ator "Gateway" (mensagem 0x753e) e desenha.
class Bitmap;
struct sGPPoint983
{
	float x, y;
};

class CGrandPrixLobbyRoom : public CLobbyRoom
{
public:
	CGrandPrixLobbyRoom(); // 00598990
	virtual ~CGrandPrixLobbyRoom(); // 00598a00 (corpo 005989f0)
	virtual void Init(FrRoomListDlg* pDlg); // 0059c0c0
	virtual void AddRoom983(FrListBox* pList, int room); // 00598f90: aviso 0x753e / 0x0b ao "Gateway"
	virtual void DelRoom983(FrListBox* pList, int room); // 00599000: aviso 0x753e / 0x0c
	virtual void OnRoomListOwnerDraw(int param, FrRoomListDlg* pDlg); // 0059c710
	virtual void OnRoomListLDown(FrRoomListDlg* pDlg); // 00598a30
	virtual void OnRoomListRUp(FrRoomListDlg* pDlg); // 0059a390
	virtual void OnRoomListDClick(FrRoomListDlg* pDlg); // 005990f0
	virtual void LoadScript983(); // 00599cd0: grandprix_lobby_ui.lua + os bitmaps da linha

	void LoadOffset983(FrListBox* pList); // 005998a0: tabela "grandprixuioffset" (ou "gpeventlist") e tamanho da linha

protected:
	sGPPoint983 m_offset[18]; // +0x04: posições dentro da linha (zeradas com memset pelo construtor)
	sGPPoint983 m_normalItem; // +0x94: largura e altura da linha comum ("gp_normal_item")
	sGPPoint983 m_eventItem; // +0x9c: idem da linha de evento ("gp_event_item")
	const Bitmap* m_pGpBmp[3][4]; // +0xa4: gp_roomlistbox_{n,d,e,o}_{01,02,03}, [pedaço][estado]
	const Bitmap* m_pEventBmp[3][4]; // +0xd4: event_roomlistbox_{n,d,e,o}_{01,02,03}
	const Bitmap* m_pNameBmp[33]; // +0x104: bitmaps de "grandprixRoomListUINames" (o desenho usa os índices 0–16 e 23–32)
	int m_188; // +0x188: cópia de FrRoomListDlg+0x158, feita no Init — é um `WTitleFontEx*` (horário da linha)
	int m_18c; // +0x18c: cópia de FrRoomListDlg+0x15c — `WTitleFontEx*` (número da linha)
	int m_190; // +0x190: -1 no Init (o construtor não inicia)
};
