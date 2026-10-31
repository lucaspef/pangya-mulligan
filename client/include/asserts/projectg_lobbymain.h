// V3: layout do 983 das classes vistas por src/projectg/lobbymain.cpp (gerado por tools/v3/gen.py; não editar à mão).
// unidade: src/projectg/lobbymain.cpp
V3_SIZE(CLobbyMain, 0x7a4) // 983: push 0x7a4; call operator new em 00592fd3, construtor 00aba400 (mapa=1; 1 new); unidade por construtor
V3_OFF(CLobbyMain, m_pRoomSelector983, 0x778) // 983: HandleMsg 0x8b caso 4 00abcbec le [this+0x778] (CLobbyRoomSelector983*) para o SetRoom983 0051ce40 (C-PKT lote 50)
V3_OFF(CLobbyMain, m_pRoomListForm983, 0x77c) // 983: HandleMsg 0x8b caso 4 00abcb7d grava a lista de salas em [this+0x77c]; OpenGameRoomDlg983 00ab6cd4 fecha (C-PKT lote 50)
V3_OFF(CLobbyMain, m_pGameRoomDlg983, 0x780) // 983: OpenGameRoomDlg983 00ab6e02 grava a janela da sala em [this+0x780]; OnGameRoomDlgResult983 00ab670c zera (C-PKT lote 50)
V3_OFF(CLobbyMain, m_pGPTimerFont983, 0x784) // 983: OpenGameRoomDlg983 00ab6e46 le [this+0x784] e [this+0x794] para o 004ff300 (C-PKT lote 50)
V3_OFF(CSharedDoc, m_bGameOver, 0x10488) // 983: OnGameRoomDlgResult983 00ab67ad testa o byte [doc+0x10488] antes do SendExitGPRoom983 (C-PKT lote 50)
V3_OFF(CSharedDoc, m_bFixHoleRepeat983, 0x11609) // 983: OnSingleModeStartResult 00abad60 grava byte [doc+0x11609]=1 na repeticao; 00abae5b le para o Encode4 7/6 (C-PKT lote 49)
V3_OFF(CSharedDoc, m_roomListKind983, 0x11630) // 983: 0062bf40/0062bf50 gravam/leem [ecx+0x11630]; HandleMsg 0x8b caso 4 00abcb01 grava o tipo da lista (C-PKT lote 50)
V3_OFF(CSharedDoc, m_lobbyState983, 0x11638) // 983: 0062bf80 grava [ecx+0x11638]; OnRoomListDlgResult 00ab66da zera (C-PKT lote 49)
V3_OFF(sMyInfo, stat, 0x12f) // 983: CSharedDoc 00641650 le byte [doc+0x91d] = m_myInfo (0x7a0) + stat (0x12f) + Level (0x4e) (C-PKT lote 49)
V3_OFF(sPangYaUserStatistics, Level, 0x4e) // 983: idem, 00641678 (nivel 0-10 -> dificuldade 2, 11-25 -> 3, 26-45 -> 4, resto 5) (C-PKT lote 49)
V3_OFF(sRoomInfo, nHole, 0x57) // 983: OnSingleModeStartResult 00abaab8 grava byte [doc+0x1094b]=0x13 (m_roomInfo 0x108f4 + 0x57) (C-PKT lote 49)
V3_OFF(sRoomInfo, holeType, 0x5b) // 983: OnSingleModeStartResult 00abaaa9/00abad46 grava byte [doc+0x1094f] = modo (0/4) e manda no 0x8 (C-PKT lote 49)
V3_OFF(sRoomInfo, realGameType, 0xc4) // 983: OnSingleModeStartResult 00abaaf1 grava byte [doc+0x109b8]=0x17 (C-PKT lote 49)
