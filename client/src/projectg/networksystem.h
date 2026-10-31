#pragma once

#include "networkunit.h"

class WNetworkSystem : public WSingleton<WNetworkSystem>
{
public:
	enum eNetUnit
	{
		NET_GAME,
		NET_MSN,
		NET_LOGIN,
		NET_RANK,
		NET_MAX
	};

	WNetworkSystem();
	virtual ~WNetworkSystem();

	int Init(eNetUnit unit);
	int Process();
	int SendMessage(eNetUnit unit, NetworkUnit::eEvent event);
	void ForceShutDown(eNetUnit unit);
	void WriteSendTime();
	void SendTTL();
	int IsConnected(eNetUnit unit);
	int IsConnectionComplete(eNetUnit unit);

	// JP 983 (00a4da90): cópia fora de linha sem chamador; embutida em WSendPacket::Send
	bool SendPacket(eNetUnit unit, WSendPacket& packet, eSendTo to)
	{
		return m_pUnit[unit]->SendPacket(packet, to);
	}

	WClientSocket* GetSocket(eNetUnit unit)
	{
		return m_pUnit[unit]->GetSocket();
	}

	bool IsGetParseKey(eNetUnit unit)
	{
		if (unit < NET_MAX)
		{
			return m_bParseKey[unit];
		}

		return false;
	}
	void SetParseKey(eNetUnit unit, bool bSet)
	{
		if (unit < NET_MAX)
		{
			m_bParseKey[unit] = bSet;
		}
	}

	int GetConnectionType() const { return m_connectionType; }

protected:
	unsigned long m_sendTime;
	bool m_bParseKey[NET_MAX];
	NetworkUnit* m_pUnit[NET_MAX];
	int m_connectionType;
};

inline WNetworkSystem* NET()
{
	return WSingleton<WNetworkSystem>::Instance();
}
