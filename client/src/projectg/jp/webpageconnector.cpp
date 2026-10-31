// JP 983: unidade 008a9fa0–008aa260 (CWebPageConnector; classe em include/jp/webpageconnector.h). Frente C, sessão C33 (C-PKT
// lote 26): pede ao servidor de jogo (0xfb) o que falta para abrir uma página do PangYa e, na resposta (0x1ad do
// CTask::OnPacketCommon), abre o navegador. No 983 a página abre por CBrowser::OpenWeb983 (00870d90); o nosso CBrowser ainda é o
// do 645 (pedido C → A), então aqui vão a parte de rede dele (SendWebPageOpen983, browser.cpp) e o Open do 645.
#include "minatl.h"
#include "projectg.h"
#include <string.h>
#include <stdio.h>
#include "packet.h"
#include "shareddoc.h"
#include "netresourcemanager.h"
#include "webpageconnector.h"
#include "../../../include/packets/browserpackets.h"

// 008aa0d0
CWebPageConnector::CWebPageConnector()
{
	memset(m_url, 0, sizeof(m_url));
}

// 008aa100
CWebPageConnector::~CWebPageConnector()
{
}

// 008a9fa0
void CWebPageConnector::RequestOpen983(const char* url, bool bResetScreen, int kind)
{
	memset(m_url, 0, sizeof(m_url));
	strcpy(m_url, url);
	m_kind = kind;
	m_bResetScreen = bResetScreen;

	WSendPacket packet((enumClientPacket)0xfb);
	packet.Send(TO_GAME);
}

// 008aa040: retorno vazio do pedido de RSS da guilda (o 983 passa um `ret`)
static void __fastcall OnGuildRSS983(int)
{
}

// 008aa150
void CWebPageConnector::OnServerReply983(const char* userId, const char* serverText)
{
	if (strlen(m_url) == 0)
		return;

	char url[0x200] = { 0 };
	sprintf(url, m_url, userId, serverText);
#ifdef JP983_SERVER_CONFIG
	// C647: sem rugburn nada desvia o www5 (reg-20261029-0140: Gacha e Guild em "Nao e possivel acessar esta pagina"). O rugburn
	// do robo desvia so gacha/login.aspx e club/login.aspx -> localhost:30080, mesmo caminho (gglog reg-20261023-1520; entrance.aspx
	// "no rewrite rules matched"): JP983_WWW=http://host:porta, senao http://localhost:30080 (como o emblema, netresourcemanager.cpp)
	{
		static const char* const kWww5 = "https://www5.pangya.jp";
		const char* path = url + strlen(kWww5);
		if (strncmp(url, kWww5, strlen(kWww5)) == 0 && (strncmp(path, "/on_client/gacha/login.aspx", 27) == 0
			|| strncmp(path, "/on_client/club/login.aspx", 26) == 0))
		{
			char www[96];
			if (!GetEnvironmentVariableA("JP983_WWW", www, sizeof(www)))
				strcpy(www, "http://localhost:30080");
			char local[0x200];
			_snprintf(local, sizeof(local) - 1, "%s%s", www, path);
			local[sizeof(local) - 1] = '\0';
			strcpy(url, local);
		}
	}
#endif

	if (strcmp(m_url, "https://www5.pangya.jp/on_client/club") == 0)
	{
		NetResourceManager::Instance()->RequestGuildRSS_JP(Doc()->m_myInfo.info.dwGuildId, OnGuildRSS983, userId,
			serverText);
		return;
	}

	// 983: CBrowser::Instance()->OpenWeb983(url, m_bResetScreen, m_kind) (00870d90)
	OpenWebPage983(url, m_bResetScreen, m_kind);
}
