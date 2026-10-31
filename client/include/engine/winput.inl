#pragma once
#include "winput.h"
#include "wdevice.h"
#include <mmsystem.h>
inline WInputDev::WInputDev()
{
	active = true;
	bUpdated = false;
}
inline WInputDev::~WInputDev()
{
}
inline bool WInputDev::InitDevice(HWND, bool)
{
	return false;
}
inline void WInputDev::SetActive(bool stat)
{
	active = stat;
}
inline void WInputDev::Reset()
{
}
inline void WInputDev::Update(unsigned long)
{
}
inline int WInputDev::GetState(int, int)
{
	return 0;
}
inline unsigned long WInputDev::GetEventTime(int)
{
	return 0;
}
inline bool WInputDev::IsAlphaNumericMode()
{
	return true;
}
inline void WInputDev::SetAlphaNumericMode(bool)
{
}
inline bool WInputDev::IsUpdated()
{
	return bUpdated;
}
inline unsigned long WInputDev::GetLastInputTime()
{
	return dwLastInputTime;
}
inline void WInputDev::ResetInputTime()
{
	dwLastInputTime = timeGetTime();
}
inline void WInputDev::SetState(int, int)
{
}
inline void WInputDev::SetAlphaNumericMode()
{
}
inline void WInputDev::SetOpenStatus(bool)
{
}
inline WInputDev* WInputDev::MakeClone(char*, HWND)
{
	return 0;
}
