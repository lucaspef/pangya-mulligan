#include "minatl.h"
#include "loginunit.h"
#include "networksystem.h"
#include "projectg.h"
#include "logininfo.h"
#include "uwin.h"

const char* LSIP_ADDRESS[] = { "157.7.173.48" };	// 983: 00e30834 (645: 211.44.251.73)
int LOGIN_PORT_LIST[] = { 10803, 10803 };	// 983: 00e30838 (645: 10101)

LoginUnit::LoginUnit()
{
}

LoginUnit::~LoginUnit()
{
}

bool LoginUnit::Init()
{
	NetworkUnit::Init();
	return true;
}

void LoginUnit::ShutDown()
{
	Init();
}

bool LoginUnit::OnLine()
{
	WReceivedPacket packet;

	if (m_socket.GetPacket(packet) == 3)
	{
		CProjectG::Instance()->OnPacket(packet);
	}

	return true;
}

bool LoginUnit::OffLine()
{
	return true;
}

bool LoginUnit::Connect()
{
	if (m_socket.m_bConnected)
	{
		setForceTransition(0);
	}
	else
	{
		std::string ip;

		ip = LSIP_ADDRESS[rand() % 1];

		int port = LOGIN_PORT_LIST[rand() & 1];
#ifdef JP983_SERVER_CONFIG
		// sem rugburn (client/COMPILAR.md): JP983_LOGIN=host:porta no ambiente troca o endereço fixo do 983
		// sem JP983_LOGIN vale o servidor C# local (o robo nao passa ambiente ao jogo, C116.1)
		char env[64];
		if (!GetEnvironmentVariable("JP983_LOGIN", env, sizeof(env)))
			strcpy(env, "127.0.0.1:30101");
		if (char* colon = strchr(env, ':'))
		{
			*colon = 0;
			port = atoi(colon + 1);
		}
		ip = env;
#endif

		m_socket.SetEventMsg(WM_USER + 13);
		char szIP[21] = "\0";

		hostent* host = gethostbyname(ip.c_str());

		if (host)
		{
			wsprintf(szIP, "%s", inet_ntoa(*(in_addr*)host->h_addr_list[0]));
		}

		if (host == NULL || m_socket.Connect(inet_addr(szIP), htons(port)) == 0)
		{
			setForceTransition(1);

			m_bConnectionComplete = TRUE;
			return false;
		}

		m_connectTime = GetTickCount();
		setTransition(0);
	}

	return true;
}

bool LoginUnit::Connecting()
{
	if (GetTickCount() - m_connectTime > 3000 || !m_socket.TryRead(true))
	{
		setTransition(1);
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

bool LoginUnit::Disconnecting()
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

int LoginUnit::OnConnect(WReceivedPacket& packet)
{
	// 983: 00a520b0
	packet.Decode1();

	if (packet.Decode2() != 0)
		return FALSE;

	int parseKey = packet.Decode4();
	Doc()->m_loginServerUID = packet.Decode4();

	m_socket.SetParseKey(parseKey);

	if (*(bool*)&Doc()->m_bReturnToLogin983)	// 983: byte em CSharedDoc+0x11250 (volta ao servidor de login com a chave)
	{
		WSendPacket send(0xb);
		send.EncodeStr(LOGINID());
		send.Encode4(MyUID());
		send.EncodeStr(Doc()->GetLoginAuthKey());
		send.Send(TO_LOGIN);
	}
	else
	{
		WSendPacket send(1);
		send.EncodeStr(LOGINID());

		unsigned long provType = LOGININFO()->ProvType();

		send.EncodeStr(LOGINPW());

		send.Encode4(provType);
		send.Encode1(IsWebLogin());
		send.Encode4(AUTHUID());
		switch (provType)
		{
		case 2:
			send.Encode8(0x7fffffffffffffff);
			break;

		default:
			return FALSE;
		}

		// 983: o pacote termina com o endereço MAC ("xx:xx:xx:xx:xx:xx") no lugar do byte IsPcBang do 645
		char mac[0x400] = "";
		std::string strMac;
		if (UWIN::GetLocalMacAddress(mac, sizeof(mac), ':') > 0)
		{
			mac[17] = '\0';
			strMac = mac;
		}
		send.EncodeStr(strMac);

		send.Send(TO_LOGIN);
	}

	return TRUE;
}
