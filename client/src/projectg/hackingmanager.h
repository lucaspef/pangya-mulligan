#pragma once

#include <string>
#include <list>

#include "encrypttypes.h"

struct HINSTANCE__;

class HackingManager : public WSingleton<HackingManager>
{
public:
	HackingManager();
	virtual ~HackingManager();

	bool Init();
	void Process();
	bool AllProcessScan();
	bool CheckExcuteProgam();

	void ClearCrypticList();
	void ResetCrypticValue(int range);
	void CheckCrypticValue();

	void AddCrypticByteValue(WCrypticValue<unsigned char>* value)
	{
		m_byteList.push_back(value);
	}
	void AddCrypticIntValue(WCrypticValue<int>* value)
	{
		m_intList.push_back(value);
	}
#ifdef REBANG_GOLFRULE_PCH
	__forceinline void AddCrypticFloatValue(WCrypticValue<float>* value)
#else
	void AddCrypticFloatValue(WCrypticValue<float>* value)
#endif
	{
		m_floatList.push_back(value);
	}

private:
	void Close();
	bool CompareHackToolByName(const char* filename);

	struct sHackTool
	{
		long size;
		char name[256];
		unsigned char code[32];
	};

	typedef void(__stdcall* InitHooksDllFn)(HWND__* hwnd);
	typedef void(__stdcall* InstallHookFn)(int);
	typedef void(__stdcall* UnInstallHookFn)(int);

	std::string m_lastExe;
	int m_hackToolNum;
	sHackTool* m_hackTool;
	HINSTANCE__* m_hHookDll;
	void* m_hMapFile;
	char* m_pMapView;
	InitHooksDllFn m_pInitHooksDll;
	InstallHookFn m_pInstallHook;
	UnInstallHookFn m_pUnInstallHook;
	int m_checkCount;
	int m_checkInterval;
	std::list<WCrypticValue<unsigned char>*> m_byteList;
	std::list<WCrypticValue<int>*> m_intList;
	std::list<WCrypticValue<float>*> m_floatList;
};
