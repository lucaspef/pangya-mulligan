#pragma once

class TiXmlNode;

class IGameData
{
public:
	IGameData() { }
	virtual ~IGameData() { }

	virtual void Initialize(TiXmlNode* pNode) = 0;
};
