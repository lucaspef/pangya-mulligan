#include "minatl.h"
#include "gameunit.h"
#include "networksystem.h"
#include "projectg.h"
#include "taskmanager.h"
#include "logininfo.h"
#include "packetversion.h"
#include "uwin.h"
#include "../shared/encryption.h"
#include "../shared/localize.h"

GameUnit::GameUnit()
{
}

GameUnit::~GameUnit()
{
}

bool GameUnit::Init()
{
	NetworkUnit::Init();

	WNetworkSystem::Instance()->SetParseKey(WNetworkSystem::NET_GAME, false);

	return true;
}

void GameUnit::ShutDown()
{
	CTaskManager::Instance()->ReleaseWork();
	Init();
}

bool GameUnit::OnLine()
{
	WReceivedPacket packet;

	unsigned long time = timeGetTime();

	while (m_socket.GetPacket(packet) == 3)
	{
		CProjectG::Instance()->OnPacket(packet);

		if (timeGetTime() - time > 1000)
			break;
	}

	return true;
}

bool GameUnit::OffLine()
{
	return true;
}

bool GameUnit::Connect()
{
	if (m_socket.m_bConnected)
	{
		setForceTransition(0);
		return true;
	}

	std::string addr = Doc()->m_gameServerAddr;
	unsigned long port = Doc()->m_gameServerPort;

	m_socket.SetEventMsg(WM_USER + 10);
	if (m_socket.Connect(inet_addr(addr.c_str()), htons((u_short)port)) == 0)
	{
		// 983 00a51610: conexão em andamento (socket assíncrono) não é falha; nas outras, o erro vai à janela como
		// FD_CONNECT. O 645 refazia o login do servidor movido com 0x5a (sem chamador do SetMoveLoginServer no 983).
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return true;
		m_socket.PostSelectEvent983(WSAMAKESELECTREPLY(FD_CONNECT, err));
		setForceTransition(1);
		m_bConnectionComplete = TRUE;

		NET()->ForceShutDown(WNetworkSystem::NET_MSN);

		return false;
	}

	m_connectTime = GetTickCount();
	setTransition(0);

	return true;
}

bool GameUnit::Connecting()
{
	if (GetTickCount() - m_connectTime > 3000 /*983 00a534c0; 645: 30000*/ || !m_socket.TryRead(true))
	{
		setForceTransition(1);

		m_socket.Close();
		m_bConnectionComplete = TRUE;
		return false;
	}

	WReceivedPacket packet;
	if (m_socket.GetPacket(packet) == 3)
	{
		if (OnConnect(packet))
		{
			m_socket.AsyncSelect(FD_READ | FD_WRITE | FD_CLOSE);
			setForceTransition(0);
			m_bConnectionComplete = TRUE;
			return true;
		}

		return false;
	}

	return true;
}

bool GameUnit::Disconnecting()
{
	if (!m_socket.m_bConnected)
	{
		setForceTransition(1);
	}
	else
	{
		m_socket.Close();
	}

	return true;
}

int GameUnit::OnConnect(WReceivedPacket& packet)
{
	packet.Decode1();

	if (packet.Decode2() != 0x3f) // 983 00a51791: era 0x3d
	{
		return FALSE;
	}

	CTaskManager::Instance()->ReleaseWork();

	packet.Decode1();
	packet.Decode1();
	int parseKey = packet.Decode1();

	m_socket.SetParseKey(parseKey);

	NET()->SetParseKey(WNetworkSystem::NET_GAME, true);

	std::string id = MyId();
	std::string password = Doc()->m_password;

	unsigned long version = 0x783aa8b8; // 983 00a5185e (PY_PACKET_VERSION do 645: 0x77def61b)
	Decrypt(&version, sizeof(version));

	WSendPacket send((enumClientPacket)2);
	send.EncodeStr(id);
	send.Encode4(MyUID());
	send.Encode4(CLoginInfo::Instance()->AuthUid());
	send.Encode2(0x6696);
	send.EncodeStr(Doc()->GetLoginAuthKey());
	send.EncodeStr(std::string("JP.R7.983.00")); // 983 00a5194a (PY_CLIENT_VERSION do 645: "645.00")
	send.Encode4(version);

	// 983 00a519af: o pacote termina com o endereço MAC e um texto do CSharedDoc (+0x11254) no lugar do IsPcBang do 645
	char mac[0x400] = "";
	std::string strMac;
	if (UWIN::GetLocalMacAddress(mac, sizeof(mac), ':') > 0)
	{
		mac[17] = '\0';
		strMac = mac;
	}
	send.EncodeStr(strMac);
	send.EncodeStr(std::string(Doc()->m_loginToken983.c_str()));

	send.Send(TO_GAME);

	return TRUE;
}
