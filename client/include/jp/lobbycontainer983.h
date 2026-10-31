// lobbycontainer983.h - frente B. JP 983: documento 0xa5 do CContentsDoc (RTTI .?AVCLobbyContainer@@, vtable 00d3c600,
// 0x1c bytes; criado no CContentsDoc::Initialize 00915fc8 com o construtor em linha que só grava a vtable).
// Layout = sRoomListDoc983 (include/jp/roomlistdlg.h) + o resto: lido pela FrRoomListDlg, FrGameRoomDlg (ROOMDOC_*) e
// CLobbyMain (+0x4b8). Corpos dos slots: 00907aa0 (Initialize chama Reset), 00907ab0 (Release), 00907ac0 (Reset).
// Sem arquivo .cpp: tudo em linha (nomes dos campos nossos).
#pragma once

#include "icontentsdatacontainer.h"

class CLobbyContainer983 : public IContentsDataContainer
{
public:
	CLobbyContainer983() { }	// 983: só a vtable; o Initialize do InsertContainer chama Reset()

	virtual bool Initialize() { Reset(); return true; }	// 00907aa0
	virtual bool Release() { return true; }	// 00907ab0
	virtual bool Reset()	// 00907ac0
	{
		m_gameType = -1;
		m_selOID = (unsigned long)-1;
		m_selUID = (unsigned long)-1;
		m_master = (unsigned long)-1;
		m_cate = 4;
		m_b18 = false;
		return true;
	}

	int m_gameType;	// +0x04 (sRoomListDoc983::gameType: -1 = sem filtro por tipo)
	int m_cate;	// +0x08: 4 = todas
	unsigned long m_selOID;	// +0x0c
	unsigned long m_selUID;	// +0x10
	unsigned long m_master;	// +0x14 (ROOMDOC_MASTER)
	bool m_b18;	// +0x18 (ROOMDOC_B18)
};
