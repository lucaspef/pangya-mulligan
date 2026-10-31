#pragma once
unsigned long __stdcall ThreadFunc(void* pParam);
class CThread
{
public:
	CThread()
		: m_hThread((HANDLE)-1)
	{
	}

	virtual ~CThread() { }

	virtual void Stop()
	{
		if (m_hThread != (HANDLE)-1)
		{
			WaitForSingleObject(m_hThread, 0xffffffff);
			CloseHandle(m_hThread);
			m_hThread = (HANDLE)-1;
		}
	}

	void Start() { m_hThread = CreateThread(0, 0, ThreadFunc, this, 0, 0); }

protected:
	friend unsigned long __stdcall ThreadFunc(void* pParam);
	virtual void Act() = 0;

	HANDLE m_hThread;
};
inline unsigned long __stdcall ThreadFunc(void* pParam)
{
	((CThread*)pParam)->Act();
	return 0;
}
