#pragma once
#include <windows.h>
#include "wdevice.h"
#include "wproc.h"
class WInputDev : public WDevice
{
public:
	WInputDev();
	virtual ~WInputDev();
	virtual bool InitDevice(HWND window, bool active);
	virtual void SetActive(bool active);
	virtual void Reset();
	virtual void Update(unsigned long time);
	virtual int GetState(int type, int index);
	virtual unsigned long GetEventTime(int index);
	virtual bool IsAlphaNumericMode();
	virtual void SetAlphaNumericMode(bool enabled);
	virtual bool IsUpdated();
	virtual unsigned long GetLastInputTime();
	virtual void ResetInputTime();
	virtual void SetState(int type, int value);
	virtual void SetAlphaNumericMode();
	virtual void SetOpenStatus(bool open);
	virtual WInputDev* MakeClone(char* mode, HWND window);

protected:
	bool active;
	bool bUpdated;
	unsigned long dwLastInputTime;
};
