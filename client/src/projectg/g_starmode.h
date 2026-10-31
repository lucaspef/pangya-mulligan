#pragma once

#include <map>

struct sCharacterInfo;

class CGStarMode
{
public:
	CGStarMode();
	~CGStarMode();
	void GetAllItemFromItemManager();
	bool IsInternetConnect();
	bool LoadFile(const char* filename);
	bool SavePartsInfoCharacter(const sCharacterInfo& character,
		const char* filename, bool truncate, void (*onError)(void*));
	void Skip30vs();
	bool SavePartsInfoCharacterMap(
		const std::map<unsigned int, sCharacterInfo>& characters,
		const char* filename);
	void SkipServer();
	void GStarModeInit();

private:
	void Parse(int character, int part, char* text);
	void SetParts(int character, unsigned long* parts);
	void SetPlayerData();

	unsigned long m_parts[8][24];
	sCharacterInfo* m_characters;
	unsigned long* m_items;
};
