#pragma once

#include "encrypttypes.h"
#include "wscene.h"

class WPuppet;

namespace IFF_STRUCT
{
	struct sClubSet;
}

struct sClubSet
{
	typedef WMatrix sFrame;

	typedef LightSet sGuide;

	sClubSet();

	IFF_STRUCT::sClubSet* pInfo;
	WPuppet* pHead[4];
	WPuppet* pHeadSub[4];
	sFrame frame[4];
	float dist983[4]; // 983: +0xe4 (AddClubSet 006aeba0), sizeof 0x164
	sGuide guide[4];
};

struct sFindClubSet
{
	sFindClubSet(unsigned long tid);
	bool operator()(sClubSet* pSet);

	unsigned long m_tid;
};

#ifdef REBANG_CLUB_CPP
typedef char rb_club_list_inst[sizeof(std::list<sClubSet*>)];
#endif

class CClub
{
public:
	enum eClub
	{
		CLUB_WOOD_1,
		CLUB_WOOD_2,
		CLUB_WOOD_3,
		CLUB_IRON_2,
		CLUB_IRON_3,
		CLUB_IRON_4,
		CLUB_IRON_5,
		CLUB_IRON_6,
		CLUB_IRON_7,
		CLUB_IRON_8,
		CLUB_IRON_9,
		CLUB_WEDGE_P,
		CLUB_WEDGE_S,
		CLUB_PUTTER_1,
		CLUB_PUTTER_2
	};

	struct sClub
	{
		WCrypticValue<float> range;
		WCrypticValue<float> lie;
		unsigned char id;
		unsigned char index;
		unsigned char type;
		WCrypticValue<float> power;
		WCrypticValue<float> curve;
		WCrypticValue<float> spin;
		char name[4];
		float pointFactor;
		WCrypticValue<float> barSpeed;
	};

	CClub();
	~CClub();

	bool Is(char* name, sClub* pClub = NULL);
	void SetPointFactor(float factor);
	void ClearPointFactor();
	float GetAirNightLuckyRange(char* name);
	float GetAirNightLuckyPower(char* name);
	const char* GetClubString(eClub club);
	void SetApproachMode();
	float GetLie(const char* name = NULL);
	unsigned char GetType(unsigned char index);
	void SetClub(const char* name);
	void SetClub(unsigned char index);
	float GetPowerShotFactor();
	void ClearClubSet();
	void SetAccuracy();
	sClubSet* FindClubSet(unsigned long tid);
	sClubSet* AddClubSet(unsigned long tid);
	void AddCrypticValue();
	void Process(float deltaTime);
	float GetNarrowAccuracy();
	float GetWideAccuracy();
	void SelectClubByDist(float dist);
	void SelectBestClub(const WVector& pos);
	void SelectBestClub();

	float GetRange(const char* name = NULL);
	float GetDriverRange983(unsigned char player); // JP 983: 006add30 (alcance do 1W do jogador, sem tacada forte)
	// JP 983: 006ad580 (alcance do 1W a partir do equipamento; fora do jogo usa os dados do proprio jogador)
	float GetRangeByEquip983(int player, sUserEquip* pEquip);
	float GetPower();
#ifdef REBANG_QUADTREE_CPP // HACK: other includers keep their IL (emission order)
	// 983 ApplyForce 00950d86/00950efb/009511d0: o bloco do SetShotOverride983 (+0x80) vale sobre o taco escolhido
	float GetCurve() { sClub* c = m_pShotOverride983 ? m_pShotOverride983 : m_pClub; return c->curve; }
	float GetSpin() { sClub* c = m_pShotOverride983 ? m_pShotOverride983 : m_pClub; return c->spin; }
#endif
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp emission order (club.h:179-194 in the original line table)
	unsigned char GetType() { return m_pClub->type; }
	char* GetName() { return m_pClub->name; }
	unsigned char GetIndex() { return m_pClub->index; }
	unsigned char GetID() { return m_pClub->id; }
	float GetBarSpeed() { return m_pClub->barSpeed; }
#else
#ifdef REBANG_GUI_CONTROL_CPP // HACK: other includers keep their IL (emission order)
	float GetBarWidth() { return m_barRemain; }
	unsigned char GetBarAlpha() { return (unsigned char)m_barPower; }
#endif
	unsigned char GetID() { return m_pClub->id; }
	unsigned char GetIndex() { return m_pClub->index; }
	char* GetName() { return m_pClub->name; }
	unsigned char GetType() { return m_pClub->type; }
#endif
	int PowerShotType() { return m_powerShot; }
	unsigned char GetApproachMode() { return m_approachMode; }
	bool BarIs(int state) { return m_bar == state; }
#ifndef REBANG_GOLFPLAYER_CPP
	bool Updated() { return m_updated; }
#else
	// only golfplayer.cpp sees these (it reads the flags as a dword), so the IL of the other includers is unchanged
	bool Updated() { return m_flags.GetFlag(1); }
	bool TypeUpdated() { return m_flags.GetFlag(2); }
	void SetPowerShot(int type) { m_powerShot = type; }
	void SetCalcRandomPower(bool calc) { m_bCalcRandomPower = calc; }
	void SetPowerShotExtra983(unsigned long bonus, unsigned long itemType) { m_powerShotBonus983 = bonus; m_powerShotItemType983 = itemType; } // 983 Shot_PowerShot 00742eed/00742efe
#endif
	void SetBar(int bar) { m_bar = bar; }
	void SetItemWindow(bool itemWindow) {} // 983: sem m_bItemWindow (BarProcess 006ad110 usa +0x58 = m_bBarLock)
	bool IsPowerPutter() { return m_bPowerPutter; }
	bool IsCalcRandomPower();
#ifdef REBANG_GOLFRULE_PCH
	void SetPowerShot(int type) { m_powerShot = type; }
	void SetRandomPower(float power) { m_addPower = power; }
	float GetRandomPower() const { return m_addPower; }
	void SetCalcRandomPower(bool calc) { m_bCalcRandomPower = calc; }
#endif
#ifdef REBANG_GOLFTASK_CPP // HACK: other includers keep their IL (emission order)
	void SetPowerShot(int type) { m_powerShot = type; }
#endif

private:
	void BarProcess(float deltaTime);
	sClub* SelectClub(unsigned char index);
	void ClubProcess(float deltaTime);
	void AddClub(char* name, float range, float lie, float power, float curve,
		float spin, unsigned char type);
	void Init();

public:
	int m_reserved;
	sClub* m_pClub;
	WList<sClub*> m_clubList;
	unsigned char m_curIndex;
	unsigned char m_limitIndex;
	WCrypticValue<int> m_powerShot;
	unsigned char m_approachMode;
	unsigned char m_unused3d[3];
#ifdef REBANG_GOLFPLAYER_CPP
	WFlags m_flags;
#else
	union
	{
		unsigned long m_flags;
		struct
		{
			unsigned char m_updated : 1;
		};
	};
#endif
	union
	{
		unsigned char m_bOffGreen;
		bool m_bPowerPutter;
	};
	unsigned char m_unused45[3];
	float m_barTime;
	float m_barPower;
	float m_barRemain;
	int m_bar;
	bool m_bBarLock; // 983: +0x58 (o 645 tinha m_bItemWindow em +0x58 e BarLock em +0x59)
	unsigned char m_prevIndex;
	unsigned char m_unused5a[2];
	float m_accuracy;
	float m_wideAccuracy;
	float m_addPower;
	union
	{
		bool m_bClubSetInit;
		bool m_bCalcRandomPower;
	};
	unsigned char m_unused69[3];
	unsigned long m_powerShotBonus983;    // 983 +0x6c (Shot_PowerShot 00742eed)
	unsigned long m_powerShotItemType983; // 983 +0x70 (00742efe); 8 bytes novos (a lista anda de 0x6c para 0x74: ClearClubSet 006ae12b, AddClubSet 006aefd7)
	std::list<sClubSet*> m_clubSetList;
	sClub* m_pShotOverride983; // 983 +0x80: bloco de 0x6c do SetShotOverride983 (006ade10); sizeof 0x84 (global 00e78fb8, vizinho em +0x84)
};

typedef CClub::eClub eClub;

CClub& GolfClub();
