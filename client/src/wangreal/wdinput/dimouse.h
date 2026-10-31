#pragma once
#include <winput.h>
#include <wproc.h>
#include <dinput.h>

class DirectInputMouse : public WProc, public WInputDev
{
public:
	DirectInputMouse(IDirectInput8A* di, HWND hWnd);
	virtual ~DirectInputMouse();
	virtual bool InitDevice(HWND hWnd, bool exclusive);
	virtual WProc* ExternProc() { return this; }
	virtual int WinProc(unsigned int message, unsigned long wParam,
		unsigned long lParam);
	virtual int GetState(int sort, int n);
	virtual void Update(unsigned long timeStamp);
	virtual unsigned long GetEventTime(int n);

private:
	HRESULT Acquire();
	void ClearBuffer();
	void FlushBuffer(DWORD timestamp);
	void GetDeviceData();

	IDirectInput8A* diBackup;
	IDirectInputDevice8A* diMouse;
	DIDEVICEOBJECTDATA mousebuf[256];
	unsigned int timebuf[4];
	int x;
	int y;
	int z;
	int b[4];
	int bufCur;
	int passCur;
	void* hEvent;
	HWND m_myHwnd;
	bool m_moveMode;
	int m_clientX;
	int m_clientY;
	int m_clientWidth;
	int m_clientHeight;
};
