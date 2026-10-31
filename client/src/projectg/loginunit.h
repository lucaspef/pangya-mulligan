#pragma once

#include "networkunit.h"

class WReceivedPacket;

class LoginUnit : public NetworkUnit
{
public:
	LoginUnit();
	virtual ~LoginUnit();

	virtual bool Init();
	virtual void ShutDown();
	virtual bool OnLine();
	virtual bool OffLine();
	virtual bool Connect();
	virtual bool Connecting();
	virtual bool Disconnecting();

private:
	int OnConnect(WReceivedPacket& packet);
};
