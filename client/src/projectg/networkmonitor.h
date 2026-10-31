#pragma once

#define NETMON_SECONDS 30

class CNetworkMonitor : public WSingleton<CNetworkMonitor>
{
public:
	CNetworkMonitor();
	virtual ~CNetworkMonitor();

	void Init();
	void TraceInBytes(unsigned int bytes);
	void TraceOutBytes(unsigned int bytes);
	void Process(float delta);
	void Render(WView* pView) const;

private:
	bool m_bShow;
	int m_index;
	float m_elapsed;
	unsigned int m_inBytes[NETMON_SECONDS];
	unsigned int m_outBytes[NETMON_SECONDS];
};
