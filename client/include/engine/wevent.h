#pragma once

class WSyncEvent
{
public:
	WSyncEvent();
	~WSyncEvent();

protected:
	HANDLE m_hEvent;
	int m_waiting;
};

WSyncEvent::WSyncEvent()
{
	m_hEvent = NULL;
	m_waiting = 0;
}

WSyncEvent::~WSyncEvent()
{
	if (m_hEvent)
		CloseHandle(m_hEvent);
}
