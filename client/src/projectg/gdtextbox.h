#pragma once

#include <map>
#include <string>
#include "gamedatadb.h"

class CGDTextBox : public IGameData
{
public:
	DECLARE_GAMEDATA(CGDTextBox, GAMEDATA_TEXTBOX, "textbox")

	CGDTextBox();
	virtual ~CGDTextBox();

	virtual void Initialize(TiXmlNode* pNode);
	const char* GetText(const char* key);
	const char* GetText(std::string& key);

private:
	std::map<std::string, std::string> m_textMap;
};
