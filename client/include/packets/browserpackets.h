// browserpackets.h - frente C (pacotes). JP 983: envios do navegador por tipo de pagina (sessao C33, C-PKT lote 26).
// No 983 sao metodos do CBrowser (layout do 983: 0x170, tipo da pagina em +0x120); o nosso CBrowser ainda e o do 645 (0x124,
// so WEBPAGE_NORMAL/SELFCERTIFY, pedido C -> A), entao aqui ficam como funcoes livres com o tipo por argumento. Corpos no fim de
// src/projectg/browser.cpp. Tipos vistos no 983: 0xb = icone de evento (CWebIcon::OpenUrl983), 0xc = "nenhum" (gravado ao
// fechar). Os nomes sao nossos.
#pragma once

// 00870d90 (parte de rede de CBrowser::OpenWeb983(url, bResetScreen, kind)): mouse no meio (400, 325), g_ime->SetState(0xd, 0),
// pacote 0xa1 com o tipo (E1) ao servidor de jogo. Chamado por CWebPageConnector::OnServerReply983 (008aa150) e pelo lobby,
// taskmain, shopmain (00abde18, 006584e0, 00aacb8e, 00a9d234).
void SendWebPageOpen983(int kind);
// 00870d90 inteiro, com o navegador do 645: SendWebPageOpen983(kind) e CBrowser::Open(url, bResetScreenSize, false) (o 983 nao
// troca o modo de video ao abrir; o 645 ja abria o bingo assim). Chamadores do 983: CWebPageConnector
// (008aa150), FrCheckoutDlg/FrGiftDlg::OnChargeResult (00a9d180/00aacae0, tipo 0xc), CLobbyMain::HandleMsg (00abde18) e taskmain
// (006584e0).
void OpenWebPage983(const char* url, bool bResetScreenSize, int kind);
// 00868320 (chamado no comeco do CBrowser::Close 00868de0): o envio de cada tipo ao fechar (2: 0x9e; 4: 0xa0 E1(1); 5: 0xa2;
// 6: 0xa4 + 0xa2; 0xb: 0x3d) e, no lobby (tarefa CLobbyTask), as mensagens 1 (atualiza) e 0x23 (aviso dos tipos 3/4) ao ator
// "Lobby". Tipo 0xd: nada.
void SendWebPageClose983(int kind);
