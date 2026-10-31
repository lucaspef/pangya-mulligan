#pragma once
class WProc;

class WDevice
{
public:
	virtual ~WDevice() { }
	virtual char* GetDeviceName() { return 0; }
	virtual char* EnumModeName() { return "\0"; }
	virtual WProc* ExternProc() { return 0; }
};
