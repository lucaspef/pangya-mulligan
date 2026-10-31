#pragma once

#include "clientsocket.h"
#include "finitestatemachine.h"

class NetworkUnit
{
public:
	enum eEvent
	{
		eEVENT_NEXT,
		eEVENT_RECONNECT,
		eEVENT_DISCONNECT,
		eEVENT_FAIL
	};

	NetworkUnit();
	virtual ~NetworkUnit();

	virtual bool Init();
	virtual bool SendPacket(WSendPacket& packet, eSendTo to);	// JP 983: slot 2 (00a53000)
	virtual void ShutDown() = 0;
	virtual bool OnLine() = 0;
	virtual bool OffLine() = 0;
	virtual bool Connect() = 0;
	virtual bool Connecting() = 0;
	virtual bool Disconnecting() = 0;
	virtual bool ResetConnect();
	virtual bool Process();
	virtual int IsConnected();	// JP 983: virtual, slot 11 (00a530e0)

	void setTransition(unsigned long state);
	void setForceTransition(unsigned long state);
	unsigned long getState();
	int IsConnectionComplete();
	WClientSocket* GetSocket() { return &m_socket; }

protected:
	WClientSocket m_socket;
	FiniteStateMachine m_fsm;
	int m_bConnectionComplete;
	unsigned long m_connectTime;
};
