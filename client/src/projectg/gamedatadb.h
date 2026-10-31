#pragma once

#include <map>
#include <string>
#include "igamedata.h"

enum eGameDataDB
{
	GAMEDATA_TEXTBOX,
	GAMEDATA_BITMAPBOX,
	GAMEDATA_EVENTNOTICE,
	GAMEDATA_VOICEITEM,
	GAMEDATA_SKINITEM983, // 983 00a41826: "skinitem" (CGDSkinItem, frente B)
	GAMEDATA_SETAUXPARTS983, // 983 00a418a6: "setauxparts" (CGDSetAuxparsAbility, frente B)
	GAMEDATA_MAX, // 983: 6 (CGameDataDB::Destory 00a415c6)
};

#define DECLARE_GAMEDATA(className, type, id) \
	static IGameData* CreateInstance() \
	{ \
		return new className; \
	} \
	static void DestoryInstance(IGameData* pData) \
	{ \
		delete pData; \
	} \
	static void RegisterToGameDB() \
	{ \
		CGameDataDB::Instance()->RegisterGameID(type, id); \
		CGameDataDB::Instance()->RegisterCreateFunction(type, CreateInstance); \
		CGameDataDB::Instance()->RegisterDestoryFunction(type, \
			DestoryInstance); \
	}

struct sGAMEDATA
{
	std::map<std::string, IGameData*> dataMap;
	IGameData*(__fastcall* pfnCreate)();
	void(__fastcall* pfnDestory)(IGameData*);
	std::string gameID;
};

class CGameDataDB : public WSingleton<CGameDataDB>
{
public:
	CGameDataDB();
	virtual ~CGameDataDB();

	void RegisterGameDataToDB(eGameDataDB type, std::string& name,
		IGameData* pData);
	void RegisterCreateFunction(eGameDataDB type,
		IGameData*(__fastcall* pfnCreate)());
	void RegisterDestoryFunction(eGameDataDB type,
		void(__fastcall* pfnDestory)(IGameData*));
	void RegisterGameID(eGameDataDB type, const char* id);
	IGameData* GetGameData(eGameDataDB type, const char* name);

private:
	void LoadGameData(const char* filename);
	void RegisterGameData();
	void ParseGameData(TiXmlDocument& doc);
	void Destory();

	sGAMEDATA m_gameData[GAMEDATA_MAX];
};
