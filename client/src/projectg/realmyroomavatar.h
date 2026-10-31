#pragma once

#include "singleton.h"

class CPartTidList;
class CRealMyRoomMain;
class CPetFrame;
struct sSlotInfo;
struct sCharacterInfo;

// Partial reconstruction (only what realmyroommain uses so far).
class CRealMyRoom_Avatar
{
public:
	void RefreshUccClothes(unsigned long typeId, unsigned long id);
	void SetTargetPosition(const WVector& pos);
	void SetAction(int action);
	void Follow(const char* name);
	void Stop();
	bool IsHoldState() const { return (m_state & 0x7e) ? true : false; }
	CPartTidList& GetPartTidList()
	{
		return *(CPartTidList*)((char*)this + 0x598);
	}
	int GetMotion() const { return m_motion; }
	unsigned long GetAction() const { return m_action; }
	bool GetHide() { return m_bOnStair; }

	unsigned char m_pad0[0x42];
	char m_nick[0xdc - 0x42];
	unsigned long m_id;
	unsigned char m_pade0[0xe8 - 0xe0];
	unsigned long m_state;
	unsigned char m_padec[0xf0 - 0xec];
	unsigned long m_action;
	WVector m_vPos;
	unsigned char m_pad100[0x128 - 0xf4 - sizeof(WVector)];
	WVector m_vPosSelf;
	unsigned char m_pad134[0x5a0 - 0x128 - sizeof(WVector)];
	unsigned long m_tid[24];
	unsigned long m_uid[24];
	unsigned char m_pad660[0x690 - 0x660];
	bool m_bOnStair;
	unsigned char m_pad691[0x698 - 0x691];
	int m_motion;
};

class CRealMyRoom_AvatarManager : public WSingleton<CRealMyRoom_AvatarManager>
{
public:
	void Render();
	void RenderOverlay();
	CRealMyRoom_Avatar* FindAvatarByNickName(const char* nick) const;
	CRealMyRoom_Avatar* CreateAvatar(sSlotInfo* slot, sCharacterInfo* info,
		CRealMyRoomMain* main);
	void FreeAllAvatars();
	CRealMyRoom_Avatar* AvatarByRayIntersect(const WVector& start,
		const WVector& ray) const;
	CRealMyRoom_Avatar* CheckClick(float x, float y) const;
	void Process(float dt);

	int m_avatarCount;
	CRealMyRoom_Avatar* m_avatars[30];
	CRealMyRoom_Avatar* m_pCurAvatar;
	CRealMyRoom_Avatar* m_pRayAvatar;
	CRealMyRoom_Avatar* m_pClickAvatar;
};

// Partial reconstruction.
class CTransferableItem : public WSingleton<CTransferableItem>
{
public:
	void Render();
	void OnLoad();
	void Process(float dt);
};

// Partial reconstruction. 645: 0x290. JP 983 (A s159): o real (realmyroom_avatar.h: guid + sCharacterInfo 0x201 +
// CPartTidList 0xc8 + CPetFrame*) tem 0x2d4 (medido com o compilador do exe); o objeto de pilha precisa caber no construtor
// real, para onde o realmyroommain.cpp aponta este nome (/alternatename: int x unsigned long, ponteiro x referencia).
class CRealMyRoom_AvatarBurden
{
public:
	CRealMyRoom_AvatarBurden(int index, sCharacterInfo* info, CPetFrame* pet);
	~CRealMyRoom_AvatarBurden();

private:
#ifdef REBANG_SEMANTIC_ONLY
	unsigned long m_pad0[0xb5];
#else
	unsigned long m_pad0[0xa4];
#endif
};
