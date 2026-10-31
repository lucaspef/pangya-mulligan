#pragma once
#include <winput.h>
#include <dinput.h>

class WDirectInput : public WInputDev
{
public:
	WDirectInput();
	~WDirectInput();

	WInputDev* MakeClone(char* modeName, HWND hWnd);
	char* GetDeviceName();
	char* EnumModeName();

private:
	IDirectInput8A* di;
};
