#pragma once
// JP 983: FrMultiplayRoomDlg (unidade 00517140–00517740; vtables 00d07524 / 00d07514 / 00d0750c; sizeof 0x300 pelo `new` de
// 00517660 — nenhum campo além dos da FrRoomListDlg). Lista de salas do modo de vários jogadores: os cinco handlers da lista
// "roomlist" estão aqui e repassam para o objeto da sala do lobby (`m_pRoom`, +0x140). Sessão B48. Nomes dos métodos são meus.

#include "roomlistdlg.h"

class FrMultiplayRoomDlg : public FrRoomListDlg
{
	DECLARE_OBJECT(FrMultiplayRoomDlg)

public:
	FrMultiplayRoomDlg(); // 005175f0
	virtual ~FrMultiplayRoomDlg(); // 00517150

protected:
	virtual void OnProc(const float deltaTime); // 00517210
	virtual bool OnInit(); // 00517360

	void InitRoomList983(); // 005173c0 (o `OnRoomList_Init` do CLobbyMain do 645)
	void Refresh983(); // 00517180 (salto para o 00520900 da base)
	void OnRoomListInit(int param); // 005172a0
	void OnRoomListOwnerDraw(int param); // 00517330
	void OnRoomListLDown(); // 00517190
	void OnRoomListRUp(); // 005171a0
	void OnRoomListDClick(); // 005171b0

	DECLARE_FRESH_MSGMAP()
};
