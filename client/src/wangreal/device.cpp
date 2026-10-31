#include <windows.h>
#include <wvideo.h>
#include <winput.h>
#include <wlist.h>
#include "wdinput/directinput.h"
#include "wime/wime.h"

static WList<WVideoDev*> vidList(16, 16);
static WList<WInputDev*> inpList(16, 16);

extern void EnumDirect3D8();

extern "C" char* __cdecl WVersion()
{
	return "Nov  9 2017";
}
extern "C" char* __cdecl WDeviceName()
{
	return "DIrectX8";
}

extern "C" WVideoDev* __cdecl WEnumVideoDevices(int iIndex)
{
	if (!vidList.Start())
		EnumDirect3D8();
	int i = 0;
	for (WVideoDev* p = vidList.Start(); p; p = vidList.Next())
	{
		if (i == iIndex)
			return p;
		++i;
	}
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_DETACH)
	{
		for (WVideoDev* p = vidList.Start(); p; p = vidList.Next())
			delete p;
		for (WInputDev* p = inpList.Start(); p; p = inpList.Next())
			delete p;
	}
	return TRUE;
}

void __fastcall AddVideoDevice(WVideoDev* video)
{
	vidList += video;
}
void __fastcall AddInputDevice(WInputDev* input)
{
	inpList += input;
}

extern "C" WInputDev* __cdecl WEnumInputDevices(int iIndex)
{
	if (!inpList.Start())
	{
		AddInputDevice(new WDirectInput);
		AddInputDevice(new WIme);
	}
	int i = 0;
	for (WInputDev* p = inpList.Start(); p; p = inpList.Next())
	{
		if (i == iIndex)
			return p;
		++i;
	}
	return 0;
}
