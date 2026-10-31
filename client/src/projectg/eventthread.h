#pragma once
#include <list>
#include <algorithm>
#include "criticalsection.h"
#include "thread.h"
#include "work.h"
class CEventThread : public CThread, public WSingleton<CEventThread>
{
public:
	CEventThread()
		: m_hEvent(CreateEventA(NULL, 0, 0, NULL))
	{
		Start();
	}

	virtual ~CEventThread() { Stop(); }

	virtual void Stop()
	{
		SetEvent(m_hEvent);
		CThread::Stop();
	}

	void RegisterWork(CWork* pWork)
	{
		m_cs.Enter();
		m_workList.push_back(pWork);
		m_cs.Leave();
	}

	void UnregisterWork(CWork* pWork)
	{
		m_cs.Enter();
		std::list<CWork*>::iterator it =
			std::find(m_workList.begin(), m_workList.end(), pWork);
		if (it == m_workList.end())
		{
			m_cs.Leave();
			return;
		}
		m_workList.erase(it);
		m_cs.Leave();
	}

protected:
	virtual void Act()
	{
		while (WaitForSingleObject(m_hEvent, 0))
		{
			Sleep(33);
			m_cs.Enter();
			for (std::list<CWork*>::iterator it = m_workList.begin();
				it != m_workList.end(); ++it)
			{
				CWork* pWork = *it;
				pWork->Act();
			}
			m_cs.Leave();
		}
	}

	void* m_hEvent;
	std::list<CWork*> m_workList;
	CCriticalSection m_cs;
};
