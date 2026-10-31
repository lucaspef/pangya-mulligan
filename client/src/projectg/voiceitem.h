#pragma once

#include <string>
#include <map>
#include <vector>

struct sGD_VOICEITEM;

struct sVOICECLUBINSERT
{
	void operator()(const sGD_VOICEITEM& item) const;
};

struct sVOICECLUBVOICEINSERT
{
	void operator()(const sGD_VOICEITEM& item) const;
};

class CVoiceItem : public WSingleton<CVoiceItem>
{
public:
	enum eWaveState
	{
		WAVESTATE_02 = 0x02,
		WAVESTATE_08 = 0x08,
		WAVESTATE_10 = 0x10,
		WAVESTATE_20 = 0x20,
		WAVESTATE_80 = 0x80,
		WAVESTATE_100 = 0x100,
		WAVE_NONE = 0,
		WAVE_READY = 1,
		WAVE_SHOT = 2
	};

	class cVoice
	{
	public:
		unsigned long state;
		std::string name;
		int num;
		int kind;
	};

	CVoiceItem();
	virtual ~CVoiceItem();

	void Initialize();
	void Destroy();
	void SetSound(unsigned long typeID, std::string name, unsigned long state,
		unsigned char numOfSounds);
	bool PlaySoundA(unsigned long typeID, eWaveState state, int index);
	// 983 00859f30: som de tacada do clubset (SoundEffectClub); sem entrada = som padrao do estado (00859830)
	void PlayClubSfx983(unsigned long typeID, unsigned long state);

	void SetClubVoice(unsigned long typeID, std::string name,
		unsigned long state, int numOfSounds, int kind);

private:
	bool ClubPlaySound(unsigned long typeID, eWaveState state, int index);
	bool PlayVoiceNoChar(std::string name, int charType, cVoice* pVoice);
	bool PlayVoiceChar(std::string name, int charType, cVoice* pVoice);
	bool PlayVoiceKind(std::string name, int charType, cVoice* pVoice);
	bool PlayVoiceByType(std::string name, int charType, unsigned long typeID,
		cVoice* pVoice, eWaveState state);
	bool AztecPlaySound(unsigned long typeID, eWaveState state);
	bool IsExistVoiceFile(char* filename);

	std::multimap<unsigned long, cVoice*> m_voiceMap;
	std::multimap<unsigned long, cVoice*> m_effectMap;
};

// 983: tabela de sons de itens (singleton estatico 00f03b10, Instance 00b3b1e0;
// ctor 00b3a330 = carga do GameData "VoiceAztec" 00b38f90 + tabela fixa 00b3a140;
// PlaySound 00b3acb0 procura o tid no mapa e dispara o som 00b3aa30).
class CItemSoundTable
{
public:
	static CItemSoundTable& Instance();
	bool PlaySound(unsigned long typeID, int state, int charType);
	std::string GetSoundName(unsigned long typeID, int state); // 00b3ac70 (vazio se nao ha)
	int GetFixed983(unsigned long typeID, int kind); // 00b396c0 (kind 1/2/3 = a/b/c; sem o tid: 30 no kind 1, 0 nos outros)

private:
	struct sSound // 0x20 bytes
	{
		unsigned long state;
		std::string name;
	};
	struct sEntry // 0x14 bytes (00b3a640)
	{
		std::vector<sSound> sounds;
	};
	struct sFixed // 0xc bytes (00b3a090)
	{
		int a;
		int b;
		int c;
	};

	CItemSoundTable();
	void LoadGameData();   // 983 00b38f90
	void AddFixed(unsigned long tid, int a, int b, int c); // 983 00b3a090
	void Add(unsigned long tid, const std::string& name, unsigned long state,
		int count); // 983 00b3acf0
	bool PlayEntry(sEntry* pEntry, unsigned long state, int charType); // 00b3aa30

	std::map<unsigned long, sEntry*> m_entries;
	std::map<unsigned long, sFixed*> m_fixed;
};

class CSoundItemManager : public WSingleton<CSoundItemManager>
{
public:
	CSoundItemManager() { }
	virtual ~CSoundItemManager() { }

	void PlayCurrentSoundItem();
};

bool WearWig(unsigned char player);
bool WearHeadgear983(int unused, unsigned char player); // 983 00858100
