#pragma once

class CWangrealApplication
{
public:
	virtual ~CWangrealApplication() { }
	virtual int MainLoop(int flags) = 0;
	virtual bool Ready() { return true; }
	virtual long __stdcall WinProc(HWND hWnd, unsigned int msg,
		unsigned int wParam, long lParam) = 0;
};
