#pragma once

#include "packet.h"
#include "socket.h"
#include "criticalsection.h"

class WClientSocket : public WSocket
{
public:
	WClientSocket(unsigned int sock = INVALID_SOCKET,
		sockaddr_in addr = ms_addrEmpty);
	virtual ~WClientSocket();

	virtual void Close();

	// JP 983 (00a4d020): bPoll = testar com Poll(0) antes (as unidades sim, a WinProc no FD_READ nao); 4 = WSAEWOULDBLOCK
	int TryRead(bool bPoll);
	int GetPacket(WReceivedPacket& packet);
	// JP 983 (00a4d160, 00a4d270): o destino vem junto (vai para o MakePacketComplete)
	int PutPacket(WSendPacket& packet, eSendTo to);
	int SendPacket(WSendPacket& packet, eSendTo to);
	void Flush();
	void AsyncSelect(long events);
	// JP 983 (00a4cca0): posta na janela o mesmo aviso do WSAAsyncSelect (ex.: FD_CONNECT com erro)
	void PostSelectEvent983(LPARAM lParam);
	void SetEventMsg(unsigned int msg) { m_eventMsg = msg; }
	void SetPacketHeaderType(int type);
	void SetParseKey(int key) { m_parseKey = key; }
	int GetRcvCnt() const { return m_rcvCnt; }

protected:
	WMemBlock m_rcvBuf;
	WMemBlock m_sndBuf;
	int m_rcvCnt;
	int m_sndCnt;
	int m_packetHeaderType;
	int m_sendSeq;
	int m_parseKey;
	unsigned int m_eventMsg;
	CCriticalSection m_cs;
};
