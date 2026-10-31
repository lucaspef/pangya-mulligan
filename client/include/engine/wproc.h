#pragma once
#include <windows.h>

class WProc;

class WProcManager
{
public:
	WProcManager() { procNum = 0; }
	struct w_handle
	{
		HWND hWnd;
		WProc* proc;
	};

#ifdef WANGREAL_DEVICE
	void DelProc(WProc* proc);
#else
	void DelProc(WProc* proc)
	{
		int n = procNum;
		int i;
		for (i = 0; i < procNum; ++i)
		{
			if (procList[i].proc == proc)
				break;
		}
		procList[i].hWnd = procList[n - 1].hWnd;
		procList[i].proc = procList[n - 1].proc;
		--procNum;
	}
#endif

	void AddProc(WProc* proc, HWND hWnd);

	long __stdcall WinProc(HWND hwnd, UINT message, UINT wParam, long lParam);

private:
	w_handle procList[16];
	int procNum;
};

class WProc
{
public:
	WProc() { this->include = NULL; }

#ifdef WANGREAL_DEVICE
	virtual ~WProc()
	{
		if (this->include)
		{
			this->include->DelProc(this);
		}
	}

#else
	virtual ~WProc();
#endif

	virtual int WinProc(UINT msg, unsigned long wParam,
		unsigned long lParam) = 0;

	void SetProc(WProcManager* procMan, HWND hWnd)
	{
		this->include = procMan;
		procMan->AddProc(this, hWnd);
	}

private:
	WProcManager* include;
};

#ifdef WANGREAL_DEVICE
__declspec(noinline) inline void WProcManager::DelProc(WProc* proc)
{
	int n = procNum;
	int i;
	for (i = 0; i < procNum; ++i)
	{
		if (procList[i].proc == proc)
			break;
	}
	procList[i].hWnd = procList[n - 1].hWnd;
	procList[i].proc = procList[n - 1].proc;
	--procNum;
}

#endif

inline void WProcManager::AddProc(WProc* proc, HWND hWnd)
{
	this->procList[this->procNum].hWnd = hWnd;
	this->procList[this->procNum].proc = proc;
	++this->procNum;
}

inline long __stdcall WProcManager::WinProc(HWND hwnd, UINT message,
	UINT wParam, long lParam)
{
	int result = 0;
	for (int i = 0; i < procNum; ++i)
		result |= procList[i].proc->WinProc(message, wParam, lParam);
	if (result == 0 || result == -1)
		return DefWindowProc(hwnd, message, wParam, lParam);
	return 0;
}
