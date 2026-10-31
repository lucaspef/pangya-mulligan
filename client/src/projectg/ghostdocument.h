#pragma once
#include "msg983.h"
#include <list>
#include <string>
#include <stdio.h>
#include <vector>

class MsgObject;
struct sShotProtocol;

struct GHOST_DATA_HEADER
{
	int m_version;
	int m_size4;
	int m_size8;
	int m_shotUnitSize;
	int m_shotCount;
	int m_cardUnitSize;
	int m_cardCount;

	unsigned int GetDataBodySize()
	{
		return m_cardUnitSize * m_cardCount + m_shotCount * m_shotUnitSize +
			m_size8 + m_size4;
	}
};

struct GHOST_GAME_DATA;

struct GHOST_GAME_BRIEF
{
	unsigned char m_holePar[18];
	unsigned char m_pad12[2];
	unsigned long m_holeScore[18];
	unsigned char m_map;
	unsigned char m_holeType;
	unsigned char m_gameType;
	unsigned char m_pad5F;
	unsigned long m_serverProperty;
};

struct GHOST_USER_DATA
{
	unsigned long m_charTypeId;
	unsigned char m_hairColor;
	unsigned char m_shirtsColor;
	unsigned char m_pad6[2];
	unsigned long m_parts[24];
	unsigned long m_auxParts[5];
	unsigned char m_charPcl[5];
	char m_uccIndexList[24][9];
	unsigned char m_pad159[3];
	unsigned long m_clubTypeId;
	short m_clubPcl[5];
	unsigned char m_pad16a[2];
	unsigned long m_ballTypeId;
	unsigned long m_caddieTypeId;
	unsigned long m_caddieParts;
	unsigned char m_caddieLevel;
	unsigned char m_pad179[3];
	unsigned long m_mascotTypeId;
	char m_mascotMsg[0x20];
	unsigned long m_itemSlot[10];
	char m_nick[0x20];
	char m_id[0x20];
	char m_guild[0x20];
	int m_identity;
	unsigned long m_emblemVer; // 983: +4 (00828130 le +0x22c -> info.dwEmblemVer)
	char m_emblem[0x20];
	unsigned char m_gender;
	unsigned char m_angelWing;
	unsigned char m_angelEffect;
	unsigned char m_devilWing;
	unsigned char m_level;
	unsigned char m_pad251[3];
	unsigned long m_uid;
};

struct GHOST_CARD_DATA
{
	unsigned long m_typeId;
	unsigned long m_itemId;
	unsigned long m_attachedId;
	unsigned long m_partsItemId;
	int m_ability;
	int m_abilityValue;
	int m_slotNum;
	int m_cardType;
};

struct GHOST_ACT_DATA
{
	unsigned char m_used;
	unsigned char m_type;
	unsigned char m_pad[2];
	int m_args[5];
};

struct GHOST_SHOT_PROTOCOL
{
	float m_maxClick;
	float m_minClick;
	float m_curve;
	float m_spin;
	unsigned char m_accuracy;
	unsigned char m_pad11[3];
	unsigned long m_special;
	unsigned long m_frame;
	float m_shotAngle;
	unsigned long m_shotTime;
	float m_center;
	unsigned char m_club;
	unsigned char m_pad29[3];
	float m_randomPower;
	float m_randomAngle;
	float m_pangyaArea; // 983 +0x2e do bloco do 0x12 (sHitShotInfo983::pangyaArea; conv 00827dc0/00827e20)
};

#pragma pack(push, 1)
struct sShotProtocol
{
	float m_maxClick;
	float m_minClick;
	float m_curve;
	float m_spin;
	unsigned char m_accuracy;
	unsigned long m_special;
	unsigned long m_frame;
	float m_shotAngle;
	unsigned long m_shotTime;
	float m_center;
	unsigned char m_club;
	float m_randomPower;
	float m_randomAngle;
	float m_pangyaArea; // 983 +0x2e do bloco do 0x12 (sHitShotInfo983::pangyaArea; conv 00827dc0/00827e20)
};
#pragma pack(pop)

struct GHOST_FILE_INFO
{
	int m_date;
	int m_time;
	int m_course;
	int m_score;
	int m_uid;
	int m_index;
	int m_pad983_18; // 983 new 0x1c (00828d44): sscanf so preenche +0..+0x14; +0x18 nunca escrito
};

struct GHOST_SHOT_DATA
{
	unsigned long m_valid;
	unsigned long m_holeIndex;
	unsigned long m_frame;
	GHOST_ACT_DATA m_acts[9];
	GHOST_SHOT_PROTOCOL m_protocol;
	float m_windIntensity;
	float m_windDirection;
	unsigned long m_windIntensityInt;
	float m_ballPos[3];
	float m_windVec[3];
	float m_cameraAngle;
};

namespace GHOST
{
	enum eGHOSTACTMSG
	{
		CHANGE_WIND = 0,
		CHANGE_WEATHER = 1,
		CHANGE_DIRECTION = 2,
		CHANGE_CLUB = 4,
		USE_ITEM = 5,
		BALL_DROP = 7,
		TIME_OUT = 8,
		GAME_TIME_OVER = 11,
		SAVE_SHOT_DATA = 12
	};
}

class memory_buffer
{
public:
	memory_buffer(unsigned int size);
	memory_buffer(char* data, unsigned int size);
	~memory_buffer();
	unsigned int reserve(unsigned int size);
	void encode(const void* data, unsigned int size);
	void decode(void* data, unsigned int size);
	const void* get_buffer() const;
	unsigned int length() const;

private:
	friend class CGhostDocument;
	void* m_pBuffer;
	unsigned int m_length;
	unsigned int m_readPos;
	unsigned int m_capacity;
};

class CGhostHoleData
{
public:
	CGhostHoleData();
	~CGhostHoleData();
	void SetHoleNum(unsigned char holeNum) { m_holeNum = holeNum; }
	const std::list<GHOST_SHOT_DATA*>& GetShotDataList() const
	{
		return m_shotDataList;
	}
	void RecordAct(int a, int b, const int* c, int d);
	unsigned int RollBackShotData(unsigned int count);
	void RecordDummyShot(unsigned int frame);
	void AddShotData(const GHOST_SHOT_DATA* shot);
	const GHOST_SHOT_DATA* RecordShotData(const sShotProtocol* protocol,
		unsigned int frame);

private:
	friend class CGhostDocument;
	unsigned char m_holeNum;
	std::list<GHOST_SHOT_DATA*> m_shotDataList;
	GHOST_SHOT_DATA* m_pCurrentShot;
};

template <int N>
void py_strcpy(char (&dst)[N], const char* src)
{
	memset(dst, 0, N);
	memcpy(dst, src, N - 1);
}

class CGhostDocument
{
	friend CGhostDocument* CreateDocument();
	friend class CGhostPlayer;

public:
	~CGhostDocument();
	void RecordGameData();
	void RecordUserData();
	bool Import(const std::string& path);
	void Export(const std::string& path);
	bool Import(unsigned char* data, unsigned int size);
	void RecordHoleData(const MsgObject& msg);
	const CGhostHoleData* GetHoleData(unsigned char holeNum) const;
	void RecordException(const char* text);
	bool WriteTextFile(const char* path) const;
	void SetInvalidate() { m_invalidate = 1; }

protected:
	CGhostDocument();

private:
	bool WriteExceptionLogFile() const;
	bool WriteTextFile(FILE* fp, const GHOST_SHOT_DATA* shot) const;
	bool WriteTextFile(FILE* fp, const CGhostHoleData* hole) const;
	std::string MakeTextFileName() const;

	GHOST_GAME_BRIEF* m_pHeader;
	GHOST_USER_DATA* m_pUserData;
	CGhostHoleData* m_pHoleData;
	std::vector<GHOST_CARD_DATA> m_cards;
	std::list<std::string> m_exceptionLog;
	int m_shotCount;
	int m_invalidate;
#ifdef REBANG_GHOSTMAIN_CPP
public:
	int IsInvalidate() const { return m_invalidate; }
	const GHOST_USER_DATA* GetUserData() const { return m_pUserData; }
	const GHOST_GAME_DATA* GetGameData() const
	{
		return (const GHOST_GAME_DATA*)m_pHeader;
	}
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbImportFunclets(); // HACK: EH funclets of the naked Import (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbRecorduserdataFunclets(); // HACK: EH funclets of the naked RecordUserData (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbExportFunclets(); // HACK: EH funclets of the naked Export (nakedify)
#endif
};

// 983: classe sem membros (ctor vazio); LoadFromMemory 00820c20 e LoadFromFile 00820c50 completas em ghosthandler.cpp.
class CGhostLoader
{
public:
	CGhostLoader();

	bool LoadFromMemory(unsigned char* data, unsigned int size);
	bool LoadFromFile(const std::string& path);
};
