#pragma once
// JP 983: `CWebPageConnector` (RTTI do 983; `WSingleton`, instância 00e75d20). Unidade 008a9fa0–008aa260 (jp/webpageconnector.cpp;
// escrita pela frente C na sessão C33, C-PKT lote 26). Fluxo: quem quer abrir uma página do PangYa chama `RequestOpen983` (guarda o
// endereço e pede ao servidor de jogo, pacote 0xfb) → o servidor responde com o 0x1ad (`CTask::OnPacketCommon`, 009d32c0) →
// `OnServerReply983` monta o endereço com o id do jogador e o texto do servidor e abre o navegador (ou, se for a página do clube,
// pede o RSS da guilda). sizeof 0x230: push 0x230 antes do new em 00a67087 (CProjectG, construtor 008aa0d0). Nomes dos métodos meus
// (B57: `RequestOpen983`).
#include "engine/singleton.h"

class CWebPageConnector : public WSingleton<CWebPageConnector>
{
public:
	CWebPageConnector();			// 008aa0d0
	virtual ~CWebPageConnector();	// 008aa100 (+ 008aa120, apagador)

	// 008a9fa0: guarda o endereço e os dois parâmetros e pede ao servidor de jogo (pacote 0xfb) o que falta para abrir a
	// página; chamado também pelo lobby (tutorial de iniciante, 0xa) e pelo FrUserInfoForm
	void RequestOpen983(const char* url, bool bResetScreen, int kind);
	// 008aa150: resposta do servidor (0x1ad com resultado 1): `url` é um formato com dois %s (id do jogador, texto do servidor)
	void OnServerReply983(const char* userId, const char* serverText);

protected:
	char m_url[0x200];		// +0x28
	bool m_bResetScreen;	// +0x228
	int m_kind;				// +0x22c (tipo de página do navegador do 983: 2..0xe; ver S5::UTIL::SendWebPageClose983)
};
