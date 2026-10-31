#pragma once

#include "actor.h"

class CPrivateTradeImpl;
class WReceivedPacket;

class CPrivateTrade : public IActor
{
public:
	DECLARE_OBJECT(CPrivateTrade)

	CPrivateTrade();
	virtual ~CPrivateTrade();

	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnReload();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void OnDestroy();

	void OnPacket(WReceivedPacket* pPacket);

protected:
	virtual void HandleMsg(const MsgObject& msg);

private:
	CPrivateTradeImpl* m_pImpl;
};
