#pragma once
#include "eventthread.h"
#include "work.h"

// WSendPacket(enumClientPacket) is inline in packet.h (PCH) but out of line in the original;
// the destructor is declared so that no implicit one is emitted (it is ~WSendPacket).
struct RbSendPacketExtern : public WSendPacket
{
	RbSendPacketExtern(enumClientPacket type);
	virtual ~RbSendPacketExtern();
};
#pragma comment(linker, \
	"/alternatename:??1RbSendPacketExtern@@UAE@XZ=??1WSendPacket@@UAE@XZ")
#pragma comment(linker, \
	"/alternatename:??0RbSendPacketExtern@@QAE@W4enumClientPacket@@@Z=??0WSendPacket@@QAE@W4enumClientPacket@@@Z")
class CWorkAliveCheck : public CWork
{
public:
	CWorkAliveCheck(unsigned long interval)
		: m_tick(GetTickCount()), m_interval(interval), m_count(0)
	{
		CEventThread::Instance()->RegisterWork(this);
	}

	virtual ~CWorkAliveCheck()
	{
		CEventThread::Instance()->UnregisterWork(this);
	}

	virtual void Act()
	{
		if (m_interval < GetTickCount() - m_tick)
		{
			RbSendPacketExtern packet((enumClientPacket)0xf4); // JP 983 (s63): 0xf6 no KR 645
			packet.Encode4(m_count++);
			packet.Send(TO_GAME);
			m_tick = GetTickCount();
		}
	}

private:
	unsigned long m_tick;
	unsigned long m_interval;
	unsigned long m_count;
};
