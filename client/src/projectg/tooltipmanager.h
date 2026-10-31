#pragma once

#include <vector>
#include "tooltip.h"

struct sToolTip
{
	FrWnd** ppWnd;
	eBarButton tip;
};

class CToolTipManager
{
public:
	CToolTipManager();
	~CToolTipManager();

	void Add(FrWnd** ppWnd, eBarButton tip);
	void Process();

private:
	std::vector<sToolTip> m_toolTips;
};
