// V3: layout do 983 das classes vistas por src/projectg/messengerdlg.cpp (gerado por tools/v3/gen.py; não editar à mão).
// unidade: src/projectg/messengerdlg.cpp
V3_OFF(CMessengerInfo, m_inviteGuildName983, 0x2c) // 983: anotação do cabeçalho src/projectg/messengerdlg.h:273
V3_OFF(CMessengerInfo, m_inviteKey983, 0x48) // 983: anotação do cabeçalho src/projectg/messengerdlg.h:274
V3_OFF(CMessengerInfo, m_chatSlotList, 0x4c) // 983: anotação do cabeçalho src/projectg/messengerdlg.h:275
V3_OFF(CMessengerInfo, m_serverList, 0x88) // 983: CTask::OnPacketCommon caso 0xfc 009ca33d (add ecx,0x88 + push_back de MSNServerInfo); o 645 lia +0x64 (C-PKT lote 18)
V3_OFF(CMessengerInfo, m_complete, 0xa4) // 983: ChatManager::SendChatMessage 009a56c7 e OnPacketCommon 009c3e80 (cmp [+0xa4],4); o 645 lia +0x80 (C-PKT lote 18)
V3_OFF(CMessengerInfo, m_bWaitServerList, 0xa8) // 983: OnPacketCommon caso 0xfc (mov byte [+0xa8],0 depois da lista) (C-PKT lote 18)
V3_OFF(CMessengerInfo, m_myStatus, 0xb0) // 983: OnPacketCommon 009c2b53 e 009c4bb5 (cmp [+0xb0],3); o 645 lia +0x8c (C-PKT lote 18)
V3_SIZE(FrMessengerDlg, 0x3c0) // 983: push 0x3c0; call operator new em 009036aa, construtor 00902780 (mapa=2; 2 new); unidade por construtor
V3_OFF(FrMessengerDlg, m_pNoticeBtn, 0x188) // 983: anotação do cabeçalho src/projectg/messengerdlg.h:174
V3_OFF(FrMessengerDlg, m_pGuildManageDlg983, 0x18c) // 983: anotação do cabeçalho src/projectg/messengerdlg.h:175
V3_OFF(FrMessengerDlg, m_pMemoDlg, 0x1c4) // 983: anotação do cabeçalho src/projectg/messengerdlg.h:181
