#pragma once

#include <vector>

class CMonitor
{
public:
	CMonitor();
	virtual ~CMonitor();

	virtual void Init(int sampleNum, int maxValue, WRect& rect);
	virtual void Trace(int value);
	virtual void Process(float fElapsed);
	virtual bool Render(WView* view) const;

protected:
	unsigned long m_boxColor;
	unsigned long m_lineColor;
	bool m_bVisible;
	int m_current;
	int m_sampleNum;
	int m_maxValue;
	WRect m_rect;
	std::vector<int> m_samples;

public:
	void SetColor(unsigned long boxColor, unsigned long lineColor)
	{
		m_boxColor = boxColor;
		m_lineColor = lineColor;
	}
};
