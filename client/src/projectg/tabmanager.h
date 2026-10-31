#pragma once

#include <vector>

class FrWnd;

struct sTabElement
{
	int id;
	std::vector<FrWnd**> windows;
};

class CTabManager
{
public:
	CTabManager();
	~CTabManager();
	void Add(int id, ...);
	void Open(int id);

private:
	sTabElement* Find(int id);
	std::vector<sTabElement> m_tabs;
	sTabElement m_defaultTab;
};
