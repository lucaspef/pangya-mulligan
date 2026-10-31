#pragma once

#include "exception.h"

class WSocket
{
public:
	unsigned long GetPeerAddress() const { return m_addr.sin_addr.S_un.S_addr; }

	WSocket(SOCKET sock, sockaddr_in addr);
	virtual ~WSocket();

	static void Startup();
	static void Cleanup();

	virtual void CreateSocket();
	virtual void Close();
	virtual int Read(unsigned char* buf, int len);
	virtual int Write(const unsigned char* buf, int len);

	int Poll(int timeout);
	void Listen();
	WSocket* Accept();
	int Accept(WSocket& sock, sockaddr* addr, int* addrlen);
	void Bind(unsigned long addr, unsigned short port);
	int Connect(unsigned long addr, unsigned short port);
	int SetSocketOpt(int level, int optname, const char* optval, int optlen);
	int GetSocketOpt(int level, int optname, char* optval, int* optlen);
	std::string& GetInfo(std::string& str);
	std::string& GetAddressString(std::string& str);
	static std::string& GetAddressString(std::string& str, unsigned long addr);

	SOCKET m_socket;
	sockaddr_in m_addr;
	int m_bConnected;

protected:
	static sockaddr_in ms_addrEmpty;
	static int ms_bInit;
};
