#include "minatl.h"
#include "socket.h"

// mstcpip.h (o SDK do build nao tem): keep-alive por socket
#ifndef SIO_KEEPALIVE_VALS
struct tcp_keepalive
{
	u_long onoff;
	u_long keepalivetime;
	u_long keepaliveinterval;
};
#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR, 4) // 0x98000004
#endif
#include "exception.h"

extern HWND g_hwnd;

sockaddr_in WSocket::ms_addrEmpty;
int WSocket::ms_bInit = FALSE;

WSocket::WSocket(SOCKET sock, sockaddr_in addr)
	: m_socket(INVALID_SOCKET), m_bConnected(FALSE)
{
	if (!ms_bInit)
		Startup();

	if (sock == INVALID_SOCKET)
	{
		CreateSocket();
	}
	else
	{
		m_socket = sock;
		m_addr = addr;
		m_bConnected = TRUE;
	}
}

WSocket::~WSocket()
{
	Close();
}

void WSocket::Startup()
{
	ms_bInit = TRUE;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 983 00a4f180: Winsock 2.2 (645: 1.1)
		throw WSysException("! Socket initialization failed");

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		throw WSysException("! Cannot find usable winsock version");
	}

	memset(&ms_addrEmpty, 0, sizeof(ms_addrEmpty));
}

void WSocket::Cleanup()
{
	WSACleanup();
}

void WSocket::CreateSocket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	m_bConnected = FALSE;
}

// 983 00a4ecf0: tira o aviso da janela, volta a bloqueante e fecha sem linger
void WSocket::Close()
{
	if (m_socket != INVALID_SOCKET)
	{
		WSAAsyncSelect(m_socket, g_hwnd, 0, 0);
		u_long nonBlocking = 0;
		ioctlsocket(m_socket, FIONBIO, &nonBlocking);
		BOOL dontLinger = TRUE;
		setsockopt(m_socket, SOL_SOCKET, SO_DONTLINGER, (const char*)&dontLinger,
			sizeof(dontLinger));
		closesocket(m_socket);
	}
	m_socket = INVALID_SOCKET;
	m_bConnected = FALSE;
}

int WSocket::Poll(int timeout)
{
	timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };

	fd_set writefds;
	fd_set exceptfds;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	FD_SET(m_socket, &readfds);

	return select(m_socket + 1, &readfds, &writefds, &exceptfds, &tv) != 0;
}

std::string& WSocket::GetAddressString(std::string& str)
{
	return GetAddressString(str, GetPeerAddress());
}

void WSocket::Listen()
{
	if (listen(m_socket, 5) != 0)
		throw WSysException("! Socket listen error");
	m_bConnected = TRUE;
}

WSocket* WSocket::Accept()
{
	sockaddr addr;
	int addrlen = sizeof(addr);
	if (accept(m_socket, &addr, &addrlen) == INVALID_SOCKET)
	{
		std::string msg;
	}
	return NULL;
}

int WSocket::Accept(WSocket& sock, sockaddr* addr, int* addrlen)
{
	sockaddr_in sa;
	int len = sizeof(sa);
	SOCKET s = accept(m_socket, (sockaddr*)&sa, &len);
	if (s != INVALID_SOCKET)
	{
		sock.m_socket = s;
		sock.m_addr = sa;
		sock.m_bConnected = TRUE;
		m_bConnected = TRUE;
		if (addr)
			*(sockaddr_in*)addr = sa;
		if (addrlen)
			*addrlen = len;
		return TRUE;
	}

	std::string msg;
	return FALSE;
}

void WSocket::Bind(unsigned long addr, unsigned short port)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = addr;
	m_addr.sin_port = port;
	if (bind(m_socket, (sockaddr*)&m_addr, sizeof(m_addr)) != 0)
		throw WSysException("! Socket bind error");
}

std::string& WSocket::GetInfo(std::string& str)
{
	std::string tmp;
	return str;
}

std::string& WSocket::GetAddressString(std::string& str, unsigned long addr)
{
	return str;
}

int WSocket::Connect(unsigned long addr, unsigned short port)
{
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = addr;
	m_addr.sin_port = port;
	// 983 00a4ef70: TCP_NODELAY antes do connect e keep-alive (60 s / 2 s) depois; o 645 nao tinha nenhum dos dois
	int noDelay = 1;
	SetSocketOpt(IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay));
	if (connect(m_socket, (sockaddr*)&m_addr, sizeof(m_addr)) == 0)
		m_bConnected = TRUE;
	tcp_keepalive keepAlive;
	keepAlive.onoff = 1;
	keepAlive.keepalivetime = 60000;
	keepAlive.keepaliveinterval = 2000;
	DWORD bytes;
	WSAIoctl(m_socket, SIO_KEEPALIVE_VALS, &keepAlive, sizeof(keepAlive), NULL, 0, &bytes, NULL, NULL);
	return m_bConnected;
}

int WSocket::Read(unsigned char* buf, int len)
{
	return recv(m_socket, (char*)buf, len, 0);
}

int WSocket::Write(const unsigned char* buf, int len)
{
	return send(m_socket, (const char*)buf, len, 0);
}

int WSocket::SetSocketOpt(int level, int optname, const char* optval,
	int optlen)
{
	return setsockopt(m_socket, level, optname, optval, optlen);
}

int WSocket::GetSocketOpt(int level, int optname, char* optval, int* optlen)
{
	return getsockopt(m_socket, level, optname, optval, optlen);
}
