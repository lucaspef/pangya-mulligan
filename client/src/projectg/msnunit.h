#pragma once

#include <string>
#include "networkunit.h"

class MSNUnit : public NetworkUnit
{
public:
	MSNUnit();
	virtual ~MSNUnit();

	virtual bool Init();
	virtual void ShutDown();
	virtual bool OnLine();
	virtual bool OffLine();
	virtual bool Connect();
	virtual bool Connecting();
	virtual bool Disconnecting();
	virtual bool ResetConnect();

protected:
	void ConnectToMSNServer();
	bool GetConnectServerInfo(std::string* ip, int* port);
	void RequestMSNServerList();

private:
	int OnConnect(WReceivedPacket& packet);

protected:
	unsigned long m_serverListRequestTime;
	unsigned long m_connectTryTime;
	bool m_bConnectTried;
};
