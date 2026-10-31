#pragma once

#include "networkunit.h"

class WReceivedPacket;

class RankUnit : public NetworkUnit
{
public:
	RankUnit();
	virtual ~RankUnit();

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
