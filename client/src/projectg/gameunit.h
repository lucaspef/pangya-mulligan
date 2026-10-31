#pragma once

#include "networkunit.h"

class GameUnit : public NetworkUnit
{
public:
	GameUnit();
	virtual ~GameUnit();

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
