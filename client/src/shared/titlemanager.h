#pragma once

#include <map>

enum eTitle;
class Title;

class TitleManager
{
	typedef std::map<eTitle, Title*> TITLEMAP;
	typedef TITLEMAP::iterator TITLEITR;

	TITLEMAP TitleMap;

public:
	TitleManager();
	~TitleManager();

	void Register(eTitle TitleNum, Title* ClassPtr);
	Title* GetTitle(eTitle TitleNum);
};

extern TitleManager* g_TitleManager;
