#pragma once

#include <list>
#include <string>
#include "gamedatadb.h"

struct sGD_VOICEITEM
{
	sGD_VOICEITEM()
	{
		name = "";
		numOfSounds = 0;
		state = 0;
		typeID = 0;
		charIndex = -1;
	}
	unsigned long typeID;
	std::string name;
	unsigned long state;
	int numOfSounds;
	int charIndex;
};

class CGDVoiceItem : public IGameData
{
public:
	DECLARE_GAMEDATA(CGDVoiceItem, GAMEDATA_VOICEITEM, "voiceitem")

	CGDVoiceItem();
	virtual ~CGDVoiceItem();

	virtual void Initialize(TiXmlNode* pNode);

	std::list<sGD_VOICEITEM>& GetVoiceList() { return m_voiceList; }

private:
	std::list<sGD_VOICEITEM> m_voiceList;
};
