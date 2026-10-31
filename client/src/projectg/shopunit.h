#pragma once

#include "networkunit.h"

class ShopUnit : public NetworkUnit
{
public:
	ShopUnit();
	virtual ~ShopUnit();

	virtual bool Init();
	virtual void ShutDown();
	virtual bool OnLine();
	virtual bool OffLine();
	virtual bool Connect();
	virtual bool Connecting();
	virtual bool Disconnecting();

private:
	int OnConnect();
};
