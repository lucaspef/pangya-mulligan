#pragma once

inline bool CAnotherHand::IsWorkDone()
{
	EnterCriticalSection(&m_csQueue);
	m_bQueseLock = true;
	if (m_curWork == m_vacation)
	{
		m_bQueseLock = false;
		LeaveCriticalSection(&m_csQueue);
		return true;
	}
	m_bQueseLock = false;
	LeaveCriticalSection(&m_csQueue);
	return false;
}

inline bool CAnotherHand::IsWorking(int key)
{
	EnterCriticalSection(&m_csQueue);
	m_bQueseLock = true;
	if (m_curWork != m_vacation && m_aWork[m_curWork].key == key)
	{
		m_bQueseLock = false;
		LeaveCriticalSection(&m_csQueue);
		return true;
	}
	m_bQueseLock = false;
	LeaveCriticalSection(&m_csQueue);
	return false;
}

inline void CAnotherHand::Enter()
{
	EnterCriticalSection(&m_csUser);
	m_bUserLock = true;
}

inline void CAnotherHand::Leave()
{
	m_bUserLock = false;
	LeaveCriticalSection(&m_csUser);
}

inline unsigned long CAnotherHand::GetStartTime()
{
	return m_aWork[m_curWork].startTime;
}
