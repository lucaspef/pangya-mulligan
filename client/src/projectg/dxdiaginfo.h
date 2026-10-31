#pragma once

class CDxDiagInfo
{
public:
	CDxDiagInfo()
	{
		Init();
		StartGathering();
	}
	~CDxDiagInfo() { }

	void Init();
	void StartGathering();
	void GatherInfo();

	const char* GetOSName() { return m_osName; }
	const char* GetCPUInfo() { return m_cpuInfo; }
	const char* GetGraphicCardInfo() { return m_graphicCardInfo; }

	bool IsGathered() { return m_bGathered; }

protected:
	char m_osName[256];
	char m_cpuInfo[256];
	char m_graphicCardInfo[256];
	bool m_bGathered;
};
