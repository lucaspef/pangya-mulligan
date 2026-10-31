#pragma once
#include <vector>
#include "monitor.h"

class CFpsMonitor : public CMonitor
{
public:
	CFpsMonitor() { }
	virtual ~CFpsMonitor() { }

	virtual bool Render(WView* view) const
	{
		if (!CMonitor::Render(view))
			return false;

		const WRect& rect = m_rect;
		int prev = -1;
		for (int i = 0; i < m_sampleNum; i++)
		{
			int index = (m_current + i - 1) % m_sampleNum;
			int value = m_samples[index];
			int last = (prev == -1) ? value : prev;
			prev = value;
			float y = rect.y + rect.h -
				(float)(value < m_maxValue ? value : m_maxValue) / m_maxValue *
					rect.h;
			float ly = rect.y + rect.h -
				(float)(last < m_maxValue ? last : m_maxValue) / m_maxValue *
					rect.h;
			float x = rect.x + i;
			_WPOINT to = { x, y };
			_WPOINT from = { x - 1.0f, ly };
			view->DrawLine2D(from, to, m_lineColor, 0);
		}

		return true;
	}
};
