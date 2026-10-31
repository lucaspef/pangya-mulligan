#pragma once

// A224/C696: deslocamentos do ntAvatar = 983. Os primeiros 0x68 bytes do 983 sao vptr (+0, ctor 007a7300), m_pPetFrame (+4,
// D641) e os 2 WBoneSet (+8/+0x38: ChangeMotion 007a77d0 anima this+8 e copia para this+0x38); antes ficavam no meio da classe
// (petFrame em +0x6ec, WBoneSet depois do m_motionType). As visoes parciais abaixo pulam essa base com NTAVATAR_OFS_BASE.
#if _MSC_VER > 1600
// D859/C696: fora do VC10 o std::vector do WBoneSet (0x10 -> 0xc) e o std::list m_texParts (+0x754, 0xc -> 8) encolhem
#include <list>
#include <vector>
#define NTAVATAR_OFS_BASE       (8 + 2 * (0x20 + sizeof(std::vector<int>)))
#else
#define NTAVATAR_OFS_BASE       0x68
#endif
#define NTAVATAR_OFS_CHARINFO   (NTAVATAR_OFS_BASE + 0x421) // 983 +0x489 (007a7575)
#define NTAVATAR_OFS_PARTTIDS   (NTAVATAR_OFS_BASE + 0x624) // 983 +0x68c (007a7364)
#define NTAVATAR_OFS_PETFRAME   4
#if _MSC_VER > 1600
#define NTAVATAR_OFS_MOTIONTYPE (NTAVATAR_OFS_PARTTIDS + 0xc8 + sizeof(std::list<void*>) + 0x24)
#else
#define NTAVATAR_OFS_MOTIONTYPE 0x784 // 983 007a77de
#endif
#include <list>
#include "wscene.h"
#include "parttidlist.h"
struct sTexturePart;
struct w_motion_data;
// D859: BaseObject (base do WSingleton) tem std::string: 0x28 no VC10, 0x24 no cl novo; as visoes parciais do ntAvatarManager
// contam a partir do fim da base (no VC10 = 0xa4 / 0xb0 do 983)
#define NTAVMGR_OFS_MYAVATAR (sizeof(WSingleton<ntAvatarManager>) + 0x7c)

#ifdef REBANG_AVATAR_AVATAR_CPP
// the real classes, as avatar_avatar.cpp defines them

#include <list>
#include "singleton.h"
#include "wboneset.h"
#include "../shared/globalgamedefine.h"
#include "../shared/s5/specialprize/specialprizeitem.h"
#include "parttidlist.h"
#include "fx.h"
#include "wscene.h"
#include <memory>
class FxInterpreter;

class CQuadTree;
class ntBalloonText;
struct sUccItem;
class CPetFrame;
class ntAvatarChatMain;
class Bitmap;
class Waabb;
struct sTexturePart;
struct trace_s;
struct w_motion_data;
enum eSPAVILITYTYPE;

class ntAvatarBurden
{
public:
	ntAvatarBurden()
		: m_pPetFrame(NULL)
	{
	}
	ntAvatarBurden(unsigned long guid, unsigned long face,
		const sCharacterInfo& info, CPetFrame* pPetFrame);
	~ntAvatarBurden();

	bool Load();

	// 983: ctor 007a4710 (+0/+4/+8/+0x20c/+0x2d4); copia 0xb7 dwords = 0x2dc (007b3ec0), nó da lista 0x2e4 (007b4069)
	unsigned long m_guid;
	unsigned long m_face;
	sCharacterInfo m_info;
	CPartTidList m_tids;
	CPetFrame* m_pPetFrame;
	unsigned long m_pad983_2d8; // A839: sem acesso no 983 (scan 0079f000..007c0000 disp 0x2d8: só ntAvatarChatMain), só a cópia 007b3ec0; o ctor do 983 não grava
};
C_ASSERT(sizeof(ntAvatarBurden) == 0x2dc && offsetof(ntAvatarBurden, m_tids) == 0x20c && offsetof(ntAvatarBurden, m_pPetFrame) == 0x2d4);

class ntAvatar
{
	friend class ntAvatarManager;

public:
	virtual ~ntAvatar(); // C696: 983 grava ntAvatar::vftable em +0 (007a7300)

	const sCharacterInfo& GetCharacterInfo() const { return m_charInfo; }
	unsigned long GetGuid() const;
	const WVector& GetPosition() const;
	void SetHeadIcon(int icon);
	bool IsTargetPositionChanged(bool bReset);
	void Follow(const char* name);
	void RefreshUccClothes(const sUccItem* pUcc);
	void SetTradeTitle(const char* title);
	SpecialPrizeItem& GetMySpItem(eSPAVILITYTYPE type);
	bool IsClickable(float x, float y) const;
	ntAvatar* GetFollowingAvatar() const;
	bool GetBoneScreenCoord(const char* name, int* x, int* y) const;
	void SetTargetPosition(const WVector& pos);
	void SetAngle(float angle);
	void SetTargetAngle(float angle);
	void SetPendingTargetAngle(float angle);
	void SetAction(int action);
	void Stop();
	void SetBalloonText(const char* text);
	void UseSPItem(eSPAVILITYTYPE type);
	void SetCharactorInfo(const sCharacterInfo& info);
	void SetPosition(const WVector& pos);
	void SetState(unsigned long state);
	void SetChatFacial(int facial);
	void ChangeMotion(int motion, const char* name, float blendTime);
	bool SendMotion983(const char* name, int type); // 983 007a5000 (PKT, frente C): pacote 0x63 do ChangeMotion
	CPetFrame* GetPetFrame983() const; // D641: 983 le avatar+4 direto (msg 0x198 do Lounge)
	void Render();
	void RenderOverlay(bool bDetail);
	void Process(float elapsed);

protected:
	ntAvatar(CQuadTree& pvs, const sSlotInfo& slot, const sCharacterInfo& info,
		ntAvatarChatMain* pChatMain);
	CQuadTree& GetPVS();
	void SetLocalPVS(CQuadTree& pvs);

private:
	void SendMove(const WVector& delta, bool bForce);
	void ProcessChangeState();
	void* AttachFx(const Attacher& attacher, const char* name, bool bFlag,
		defines_t* defines);
	void ScriptProcess(const w_motion_data* data, float from, float to,
		bool bAll);
	void ParsePuppetScript(const char* script);
	void ProcessMotion(float elapsed);
	void ParseScript_ShowVis(bool bShow, const char* script);
	void SetPendingTexPart(const char* part, const char* texture);
	sTexturePart* FindTexturePart(const char* part, const char* texture);

	// offsets verified against Ghidra: total size 0xb04
	CPetFrame* m_pPetFrame;    // 0x4 (983 le avatar+4: D641, msg 0x198 do Lounge)
	WBoneSet m_boneSet1;       // 0x8 (983 007a77d0: this+8)
	WBoneSet m_boneSet2;       // 0x38 (983 007a77d0: this+0x38)
	CQuadTree* m_pPVS;         // 0x68
	char m_idName[0x16];       // 0x6c
	char m_nickName[0x16];     // 0x82
	char m_moveTarget[0x16];   // 0x98
	char m_followName[0x16];   // 0xae
	bool m_bHasGuild;          // 0xc4
	char m_guildName[0x80];    // 0xc5
	unsigned long m_e0;        // 0x148
	unsigned long m_guid;      // 0x14c
	unsigned char m_e8;        // 0x150
	unsigned char m_e9;        // 0x151
	unsigned char m_ea;        // 0x152
	unsigned char m_eb;        // 0x153
	unsigned long m_ec;        // 0x154
	unsigned long m_state;     // 0x158
	unsigned long m_prevState; // 0x15c
	int m_action;              // 0x160
	WVector m_pos;             // 0x164
	float m_angle;             // 0x170
	float m_targetAngle;       // 0x174
	bool m_bPendingAngle;      // 0x178
	float m_pendingAngle;      // 0x17c
	WVector m_up;              // 0x180
	WVector m_targetPos;       // 0x18c
	bool m_bTargetChanged;     // 0x198
	int m_headIcon;            // 0x19c
	WVector m_moveAccum;       // 0x1a0
	ntAvatarBurden m_burden;   // 0x1ac (0x2dc no 983)
	bool m_b3d8;                         // 0x488 (983: +0x488, 007a7569; o pad do 645 saiu com o burden de 0x2dc, A224)
	sCharacterInfo m_charInfo;           // 0x489 (983: +0x489, 007a7575)
	CPartTidList m_partTids;             // 0x600
	std::list<sTexturePart*> m_texParts; // 0x754
	float m_alpha;                       // 0x760
	LightSet m_light;                    // 0x764
	w_motion_data* m_pMotion;            // 0x780
	int m_motionType;                    // 0x784
	w_motion_data* m_pPrevMotion;        // 0x788
	int m_prevMotionType;                // 0x78c
	float m_blendTime;                   // 0x790
	float m_blendElapsed;                // 0x794
	float m_6f8;                         // 0x798
	float m_facialTime;                  // 0x79c
	unsigned long m_motionSendTime;      // 0x7a0
	ntBalloonText* m_pBalloon;           // 0x7a4
	float m_balloonRand[3];              // 0x7a8
	float m_714;                         // 0x7b4
	float m_718;                         // 0x7b8
	char m_tradeTitle[0x40];             // 0x7bc
	Bitmap* m_pEmblem;                   // 0x7fc
	ntAvatarChatMain* m_pChatMain;       // 0x800
	CPvsFxBox m_fxBox;                   // 0x804
	SpecialPrizeItem m_spItems[5];       // 0xb68
	// 983 +0xc80/+0xc84: FxInterpreter(&fxbox, 0) criado no construtor (007a76f9); fx das pecas vestidas (007a4d90)
	std::tr1::shared_ptr<FxInterpreter> m_spFx983;

	void AttachItemFx983(); // 007a4d90
	friend struct RbNtAvatarLayout; // D859
};
struct RbNtAvatarLayout // D859: os offsets que as visoes parciais usam, nos dois compiladores
{
	C_ASSERT(offsetof(ntAvatar, m_charInfo) == NTAVATAR_OFS_CHARINFO && offsetof(ntAvatar, m_pPetFrame) == NTAVATAR_OFS_PETFRAME);
	C_ASSERT(offsetof(ntAvatar, m_partTids) == NTAVATAR_OFS_PARTTIDS && offsetof(ntAvatar, m_motionType) == NTAVATAR_OFS_MOTIONTYPE);
};

class ntAvatarManager : public WSingleton<ntAvatarManager>
{
public:
	using WSingleton<ntAvatarManager>::m_pInstance;

	ntAvatarManager();
	virtual ~ntAvatarManager();

	ntAvatar* FindAvatarByIDName(const char* name) const;
	ntAvatar* FindAvatarByNickName(const char* name) const;
	ntAvatar* FindAvatarByGuid(int guid) const;
	ntAvatar* CheckClick(float x, float y) const;
	void ResetAvatarPositionAngle();
	void Render();
	void RenderOverlay();
	void Process(float elapsed);
	ntAvatar* CreateAvatar(const sSlotInfo& slot, const sCharacterInfo& info,
		ntAvatarChatMain* pChatMain);
	bool DeleteAvatar(const char* name);
	ntAvatar* AvatarByRayIntersect(const WVector& pos,
		const WVector& dir) const;
	void FreeAllAvatars();
	void SetFxMode983(bool bTopView); // 983 007a4e70

	ntAvatar* GetMyAvatar() const { return m_pMyAvatar; }

protected:
	CQuadTree& GetPVS();
	void SetLocalPVS(CQuadTree& pvs);

public:
	int m_count;             // 0x28
	ntAvatar* m_avatars[30]; // 0x2c
	ntAvatar* m_pMyAvatar;   // 0xa4
	unsigned long m_a8;      // 0xa8
	unsigned long m_ac;      // 0xac
	bool m_bFlag0;           // 0xb0
	bool m_bFlag1;           // 0xb1
	CQuadTree* m_pPVS;       // 0xb4
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCreateavatarFunclets(); // HACK: EH funclets of the naked CreateAvatar (nakedify)
#endif
};
C_ASSERT(offsetof(ntAvatarManager, m_pMyAvatar) == NTAVMGR_OFS_MYAVATAR && offsetof(ntAvatarManager, m_bFlag0) == NTAVMGR_OFS_MYAVATAR + 0xc); // D859

#ifdef REBANG_AVATAR_AVATAR_CPP
// emitted out of line by avatar_avatar.obj only, before the inlines of the
// headers that avatar_avatar.cpp includes next (balloontext.h)
unsigned long ntAvatar::GetGuid() const
{
	return m_guid;
}

inline const WVector& ntAvatar::GetPosition() const
{
	return m_pos;
}

void ntAvatar::SetHeadIcon(int icon)
{
	m_headIcon = icon;
}

#endif

// 983: sizeof(ntAvatar) do 645 (0xb04) não vale aqui (bases no layout do 983; tamanho do 983 não medido)
#else
// the partial views the other units were written against

#include "singleton.h"
#include "../shared/globalgamedefine.h"
#include <list>

class CPetFrame;
class CQuadTree;
struct sCharacterInfo;

// Known burden fields and ABI padding used by avatar_task.
class ntAvatarBurden
{
public:
	ntAvatarBurden(unsigned long guid, unsigned long key,
		const sCharacterInfo& info, CPetFrame* pPet);
	~ntAvatarBurden();

	bool Load();
	bool operator<(const ntAvatarBurden& rhs);
	bool operator==(const ntAvatarBurden& rhs);

	// mesma memória do ntAvatarBurden real (983 0x2dc): m_guid aqui é o 2o argumento do ctor (+4), que o operator< do 983 compara (007b36f0)
	unsigned long m_unused0;
	unsigned long m_guid;
	unsigned long m_unused8[(0x2d4 - 8) / 4];
	CPetFrame* m_pPet;
	unsigned long m_unused2d8;
};
C_ASSERT(sizeof(ntAvatarBurden) == 0x2dc && offsetof(ntAvatarBurden, m_pPet) == 0x2d4);

inline bool ntAvatarBurden::operator<(const ntAvatarBurden& rhs)
{
	return m_guid < rhs.m_guid;
}

inline bool ntAvatarBurden::operator==(const ntAvatarBurden& rhs)
{
	return m_guid == rhs.m_guid && m_pPet != (CPetFrame*)-1 &&
		rhs.m_pPet != (CPetFrame*)-1;
}

#ifdef REBANG_AVATAR_MAINUI_CPP
// avatar_mainui.cpp sees the UI-facing fields of the avatar classes.
// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class ntAvatar
{
public:
	const sCharacterInfo& GetCharacterInfo() const { return m_charInfo; }
	const char* GetNickName() const { return m_nickName; }
	const WVector& GetPosition() const { return m_position; }
	bool IsMoving() const
	{
		if ((m_position - m_targetPosition).Magnitude() > 0.001f)
			return true;
		return false;
	}

	void Stop();
	void Follow(const char* nickName);
	void SetState(unsigned long state);
	void SetBalloonText(const char* text);
	void SetChatFacial(int face);

	unsigned char m_base983[NTAVATAR_OFS_BASE]; // C696: vptr, m_pPetFrame, 2 WBoneSet
	CQuadTree* m_pPVS;
	char m_idName[0x16];
	char m_nickName[0x16];
	char m_moveTarget[0x16];
	char m_followName[0x16];
	bool m_bHasGuild;
	char m_guildName[0x80];
	unsigned long m_uid;
	int m_guid;
	bool m_canKick;
	unsigned long m_charTypeId;
	unsigned long m_state;
	unsigned long m_prevState;
	int m_action;
	WVector m_position;
	float m_angle;
	float m_targetAngle;
	bool m_bPendingAngle;
	float m_pendingAngle;
	WVector m_up;
	WVector m_targetPosition;
	bool m_bTargetChanged;
	int m_headIcon;
	WVector m_moveAccum;
	ntAvatarBurden m_burden;
	bool m_b3d8;
	sCharacterInfo m_charInfo;
};
C_ASSERT(offsetof(ntAvatar, m_state) == NTAVATAR_OFS_BASE + 0xf0);
C_ASSERT(offsetof(ntAvatar, m_charInfo) == NTAVATAR_OFS_CHARINFO);

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class ntAvatarManager : public WSingleton<ntAvatarManager>
{
public:
	using WSingleton<ntAvatarManager>::m_pInstance;

	ntAvatar* FindAvatarByGuid(int guid) const;
	ntAvatar* FindAvatarByNickName(const char* nickName) const;

	int m_avatarCount;
	ntAvatar* m_avatars[30];
	ntAvatar* m_pMyAvatar;
	ntAvatar* m_pLitAvatar;
	ntAvatar* m_pHighlightAvatar;
	bool m_active;
	bool m_overlayDetail;
};
#elif !defined(REBANG_AVATAR_MAIN_CPP)
enum eSPAVILITYTYPE
{
};

class SpecialPrizeItem
{
public:
	unsigned char m_unused0[8];
	unsigned long m_value;
};

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class ntAvatar
{
public:
	const sCharacterInfo& GetCharacterInfo() const { return m_charInfo; }
	bool GetBoneScreenCoord(const char* bone, int* x, int* y) const;

	SpecialPrizeItem& GetMySpItem(eSPAVILITYTYPE type);
	void SetBalloonText(const char* text);
	void SetState(unsigned long state); // 983 007a4820 (s111: resultados da AlchemyDlg/card_compose do CTaskMain)

#ifdef REBANG_AVATAR_TASK_CPP
	unsigned char m_unused0[NTAVATAR_OFS_PETFRAME];
	CPetFrame* m_pPetFrame;
	unsigned char m_unused8[NTAVATAR_OFS_BASE + 0xe0 - NTAVATAR_OFS_PETFRAME - 4];
	unsigned long m_guid;
	unsigned long m_key;
	unsigned char m_unusede8[0xfc - 0xe8];
	WVector m_pos;
	unsigned char m_unused108[0x144 - 0x108];
	ntAvatarBurden m_burden;
	bool m_bBurdenLoaded;
	sCharacterInfo m_charInfo;
#else
	unsigned char m_base983[NTAVATAR_OFS_BASE]; // C696: vptr, m_pPetFrame, 2 WBoneSet
	CQuadTree* m_pPVS;
	char m_idName[0x16];
	char m_nickName[0x16];
	char m_moveTarget[0x16];
	char m_followName[0x16];
	bool m_bHasGuild;
	char m_guildName[0x80];
	unsigned long m_e0;
	int m_guid;
	unsigned char m_e8;
	unsigned char m_e9;
	unsigned char m_ea;
	unsigned char m_eb;
	unsigned long m_ec;
	// A224: o estado do 983 em +0x158 (base de 0x68 + 0xf0, C696) e o m_state do avatar_avatar.cpp,
	// que o SetState grava; em +0x158 o bongdari/ranking/taskmain liam lixo do burden e o mandavam no pacote 0x63
	unsigned long m_state983;
	unsigned long m_prevState;
	int m_action;
	WVector m_pos;
	float m_angle;
	float m_targetAngle;
	bool m_bPendingAngle;
	float m_pendingAngle;
	WVector m_up;
	WVector m_targetPos;
	bool m_bTargetChanged;
	int m_headIcon;
	WVector m_moveAccum;
	ntAvatarBurden m_burden;
	bool m_b3d8;
	sCharacterInfo m_charInfo;
	CPartTidList m_partTids;
	std::list<sTexturePart*> m_texParts;
	float m_alpha;
	LightSet m_light;
	w_motion_data* m_pMotion;
	int m_motionType983; // 983 +0x784 (007a77d0 grava o tipo de motion) = m_motionType do avatar_avatar.cpp
#endif
};
#ifdef REBANG_AVATAR_TASK_CPP
C_ASSERT(offsetof(ntAvatar, m_burden) == NTAVATAR_OFS_BASE + 0x144);
C_ASSERT(offsetof(ntAvatar, m_bBurdenLoaded) == NTAVATAR_OFS_BASE + 0x420);
C_ASSERT(offsetof(ntAvatar, m_charInfo) == NTAVATAR_OFS_CHARINFO);
C_ASSERT(offsetof(ntAvatar, m_pPetFrame) == NTAVATAR_OFS_PETFRAME);
#else
C_ASSERT(offsetof(ntAvatar, m_state983) == NTAVATAR_OFS_BASE + 0xf0);
C_ASSERT(offsetof(ntAvatar, m_charInfo) == NTAVATAR_OFS_CHARINFO);
C_ASSERT(offsetof(ntAvatar, m_partTids) == NTAVATAR_OFS_PARTTIDS);
C_ASSERT(offsetof(ntAvatar, m_motionType983) == NTAVATAR_OFS_MOTIONTYPE);
#endif

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
#ifdef REBANG_AVATAR_TASK_CPP
// HACK: instantiate the burden list before the manager singleton, as in the original.
typedef char RbAvatarBurdenListSize[sizeof(std::list<ntAvatarBurden>)];
#endif

class ntAvatarManager : public WSingleton<ntAvatarManager>
{
public:
	using WSingleton<ntAvatarManager>::m_pInstance;

	ntAvatar* FindAvatarByGuid(int guid) const;
	ntAvatarManager();
	virtual ~ntAvatarManager();

	ntAvatar* FindAvatarByNickName(const char* name) const;
#ifdef REBANG_TASKMAIN_PCH // other includers keep their IL
	void SetActive(bool bActive) { m_bActive = bActive; }
#endif

	unsigned char m_unused4[NTAVMGR_OFS_MYAVATAR - sizeof(WSingleton<ntAvatarManager>)];
	ntAvatar* m_pMyAvatar; // 0xa4 (VC10)
	unsigned char m_unuseda8[0xb0 - 0xa8];
	bool m_bActive; // 0xb0 (VC10)
	unsigned char m_unusedb1[0xb8 - 0xb1];
};
C_ASSERT(offsetof(ntAvatarManager, m_bActive) == NTAVMGR_OFS_MYAVATAR + 0xc); // D859
#else
// avatar_main.cpp sees the real SpecialPrizeItem (specialprizeitem.h) and a wider view of the avatar classes.
class CPartTidList;
class ntAvatarChatMain;
class SpecialPrizeItem;
enum eSPAVILITYTYPE;

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class ntAvatar
{
public:
	const sCharacterInfo& GetCharacterInfo() const { return m_charInfo; }

	bool IsFollowing() const { return m_followName[0] ? true : false; }
	bool IsHoldState() const { return (m_state & 0x7e) ? true : false; }
	unsigned long GetUID() const { return m_uid; }
	const char* GetIDName() const { return m_idName; }
	const char* GetNickName() const { return m_nickName; }
	const WVector& GetTargetPosition() const { return m_targetPos; }
	int GetMotion() const { return m_motion; }
	unsigned long GetState() const { return m_state; }
	unsigned long GetAction() const { return m_action; }
	void AddState(unsigned long state) { SetState(m_state | state); }
	void SubState(unsigned long state) { SetState(m_state & ~state); }
	CPartTidList& GetPartTidList()
	{
		return m_partTids; // A421: era 0x598 (645)
	}

	void SetState(unsigned long state);
	void SetPosition(const WVector& pos);
	void SetAngle(float angle);
	void SetPendingTargetAngle(float angle);
	void ChangeMotion(int type, const char* name, float blend);
	CPetFrame* GetPetFrame983() const; // D641 (avatar_avatar.cpp)
	void SetCharactorInfo(const sCharacterInfo* info);
	void SetTradeTitle(const char* title);
	void SetChatFacial(int facial);
	void Stop();
	void SetTargetPosition(const WVector& pos);
	void SetAction(unsigned long action);
	void Follow(const char* name);
	bool IsTargetPositionChanged(bool bReset);
	ntAvatar* GetFollowingAvatar();
	void UseSPItem(eSPAVILITYTYPE type);
	SpecialPrizeItem& GetMySpItem(eSPAVILITYTYPE type);
	void RefreshUccClothes(const sUccItem* pUcc);

	unsigned char m_base983[NTAVATAR_OFS_BASE]; // C696: vptr, m_pPetFrame, 2 WBoneSet
	CQuadTree* m_pPVS;
	char m_idName[0x16];
	char m_nickName[0x2c];
	char m_followName[0x9a];
	unsigned long m_uid;
	int m_guid;
	unsigned char m_e8;
	unsigned char m_e9;
	unsigned char m_ea;
	unsigned char m_eb;
	unsigned long m_ec;
	unsigned long m_state;
	unsigned long m_prevState;
	unsigned long m_action;
	WVector m_pos;
	float m_angle;
	float m_targetAngle;
	bool m_bPendingAngle;
	float m_pendingAngle;
	WVector m_up;
	WVector m_targetPos;
	bool m_bTargetChanged;
	int m_headIcon;
	WVector m_moveAccum;
	ntAvatarBurden m_burden;
	bool m_b3d8;
	sCharacterInfo m_charInfo;
	CPartTidList m_partTids;
	std::list<sTexturePart*> m_texParts;
	float m_alpha;
	LightSet m_light;
	w_motion_data* m_pMotion;
	int m_motion;
};
C_ASSERT(offsetof(ntAvatar, m_state) == NTAVATAR_OFS_BASE + 0xf0);
C_ASSERT(offsetof(ntAvatar, m_charInfo) == NTAVATAR_OFS_CHARINFO);
C_ASSERT(offsetof(ntAvatar, m_partTids) == NTAVATAR_OFS_PARTTIDS);
C_ASSERT(offsetof(ntAvatar, m_motion) == NTAVATAR_OFS_MOTIONTYPE);

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class ntAvatarManager : public WSingleton<ntAvatarManager>
{
public:
	using WSingleton<ntAvatarManager>::m_pInstance;

	ntAvatar* FindAvatarByGuid(int guid) const;
	ntAvatar* FindAvatarByNickName(const char* nick) const;
	ntAvatar* FindAvatarByIDName(const char* name) const;
	void DeleteAvatar(const char* nick);
	void Render();
	void Process(float dt);
	ntAvatar* AvatarByRayIntersect(WVector& origin, WVector& dir);
	ntAvatar* CheckClick(float x, float y);
	void RenderOverlay();
	void ResetAvatarPositionAngle();
	void FreeAllAvatars();
	void SetFxMode983(bool bTopView); // 983 007a4e70
	ntAvatar* CreateAvatar(sSlotInfo* slot, sCharacterInfo* charInfo,
		ntAvatarChatMain* main);

	int m_count;
	ntAvatar* m_avatars[30];
	ntAvatar* m_pMyAvatar;
	ntAvatar* m_pHoverAvatar;
	ntAvatar* m_pClickAvatar;
	bool m_bRestored;
	bool m_overlayDetail; // 0xb1
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	void SetActive(bool bActive);
#endif
};
#endif
#endif
