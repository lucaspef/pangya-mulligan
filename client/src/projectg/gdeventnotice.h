#pragma once

#include <map>
#include <string>
#include "gamedatadb.h"

class CGDEventNotice : public IGameData
{
public:
	DECLARE_GAMEDATA(CGDEventNotice, GAMEDATA_EVENTNOTICE, "eventnotice")

	CGDEventNotice();
	virtual ~CGDEventNotice();

	virtual void Initialize(TiXmlNode* pNode);
	const char* GetText(int key);

private:
	std::map<int, std::string> m_textMap;
};
