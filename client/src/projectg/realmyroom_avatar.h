#pragma once

// A421: deslocamentos do CRealMyRoom_Avatar completo (visao do #else, medidos no build) que a visao parcial do
// realmyroommainui.cpp usa; a visao parcial tinha os do 645 (charInfo 0x3d9, pet 0x660, burden 0x290, oculto 0x690)
#define RMRAVATAR_OFS_CHARINFO 0x421
#define RMRAVATAR_OFS_PETFRAME 0x6ec
#define RMRAVATAR_OFS_HIDE     0x71c
#define RMRAVATAR_BURDEN_SIZE  0x2d4

// HACK: realmyroommainui.cpp keeps the partial view it was matched with: its code layout follows the
// amount of IL in front of it (own PCH, see stdafx_realmyroommainui.h), so the full classes and the
// headers they need are only visible to the other units.
#ifdef REBANG_REALMYROOMMAINUI_CPP

#include "singleton.h"

class CPartTidList;

class CPetFrame
{
public:
	bool Refresh(CPartTidList* pTids);
	// 983 008b28b0 (= partsinfo.h): 4o argumento (sem ele o nome nao existia e virava stub no exe; os chamadores do 983 passam 1)
	unsigned long SetPart(CPartTidList* pTids, unsigned long tid,
		unsigned long uid, bool bHairTid = true);
	bool SetAuxPart(unsigned char index, unsigned long tid);
	bool SetHairClr(CPartTidList* pTids, unsigned char color);
};

struct sCharacterInfo;
struct sSlotInfo;
class CRealMyRoomMain;

class CRealMyRoom_AvatarBurden
{
public:
	CRealMyRoom_AvatarBurden(unsigned long uid, const sCharacterInfo& info,
		CPetFrame* pPet);
	~CRealMyRoom_AvatarBurden();

	unsigned char m_pad[RMRAVATAR_BURDEN_SIZE];
};

class CRealMyRoom_Avatar
{
public:
	void ChangeCharacterInfo();
	void SetState(unsigned long state);
	void SetAngle(float angle);
	void Stop();
	void SetAction(int action);
	void SetTargetPosition(const WVector& pos);
	void SetPendingTargetAngle(float angle);
	void ChangeMotion(int type, const char* name, float blend);
	void SetHide(bool hide);
	void SetCharactorInfo(const sCharacterInfo& info);

	void SetPosition(const WVector& pos);

	CPetFrame* GetPetFrame() { return m_pPetFrame; }
	const char* GetIDName() const { return m_szIDName; }
	const char* GetNickName() const { return m_szNickName; }
	const WVector& GetTargetPosition() const { return m_vTarget; }
	void SubState(unsigned long state) { SetState(m_state & ~state); }

	char m_szIDName[0x16];
	char m_szNickName[0xe8 - 0x16];
	unsigned long m_state;
	unsigned char m_padEC[0xf4 - 0xec];
	WVector m_vPos;
	unsigned char m_pad100[0x128 - 0xf4 - sizeof(WVector)];
	WVector m_vTarget;
	unsigned char m_pad2[RMRAVATAR_OFS_CHARINFO - 0x128 - sizeof(WVector)];
	unsigned char m_charInfo[RMRAVATAR_OFS_PETFRAME - RMRAVATAR_OFS_CHARINFO]; // sCharacterInfo + CPartTidList
	CPetFrame* m_pPetFrame;
	unsigned char m_pad3[RMRAVATAR_OFS_HIDE - RMRAVATAR_OFS_PETFRAME - 4];
	bool m_bHide; // m_bVisible da visao completa (SetHide grava nele)

	const sCharacterInfo& GetCharacterInfo() const { return *(const sCharacterInfo*)m_charInfo; }
};
C_ASSERT(offsetof(CRealMyRoom_Avatar, m_charInfo) == RMRAVATAR_OFS_CHARINFO);
C_ASSERT(offsetof(CRealMyRoom_Avatar, m_pPetFrame) == RMRAVATAR_OFS_PETFRAME);
C_ASSERT(offsetof(CRealMyRoom_Avatar, m_bHide) == RMRAVATAR_OFS_HIDE);

class CRealMyRoom_AvatarManager : public WSingleton<CRealMyRoom_AvatarManager>
{
public:
	unsigned char m_pad[4 + 0x78]; // A727: m_avatarCount + m_pAvatar[30] apos a base (0xa4 no VC10, 0xa0 no cl novo)
	CRealMyRoom_Avatar* m_pAvatar; // 0xa4 (VC10)

	CRealMyRoom_Avatar* FindAvatarByGuid(int guid) const;
	CRealMyRoom_Avatar* CreateAvatar(const sSlotInfo& slot,
		const sCharacterInfo& charInfo, CRealMyRoomMain* pMain);
	bool DeleteAvatar(const char* name);
};

class CRealMyRoomTask
{
public:
	void AddAvatarBurden(const CRealMyRoom_AvatarBurden& burden);
	void RemoveAvatarBurden(const CRealMyRoom_AvatarBurden& burden);
};

#else

#include <list>

#include "singleton.h"
#include "wmath.h"
#include "wboneset.h"
#include "wscene.h"
#include "fx.h"
#include "parttidlist.h"
#include "partsinfo.h"
#include "../shared/globalgamedefine.h"

class Bitmap;
class CRealMyRoomMain;
struct sTexturePart;
struct w_motion_data;
class ntBalloonText;

class CRealMyRoom_AvatarBurden
{
public:
	CRealMyRoom_AvatarBurden();
	CRealMyRoom_AvatarBurden(unsigned long guid, const sCharacterInfo& charInfo,
		CPetFrame* pPetFrame);
	~CRealMyRoom_AvatarBurden();

	bool LoadAvatarBurden();

	unsigned long m_guid;
	sCharacterInfo m_charInfo;
	CPartTidList m_partTidList;
	CPetFrame* m_pPetFrame;
};

class CRealMyRoom_Avatar
{
public:
	CRealMyRoom_Avatar(const sSlotInfo& slotInfo,
		const sCharacterInfo& charInfo, CRealMyRoomMain* pMain);
	~CRealMyRoom_Avatar();

	const WVector& GetPosition() const { return m_position; }
	void SetHeadIcon(int headIcon);

	bool IsTargetPositionChanged(bool bChanged);
	void SetHide(bool bHide);
	bool IsClickable(float x, float y) const;
	CRealMyRoom_Avatar* GetFollowingAvatar() const;
	bool GetBoneScreenCoord(const char* boneName, int* pX, int* pY) const;
	void ChangeCharacterInfo();
	void SetCharactorInfo(const sCharacterInfo& charInfo);
	void SetPosition(const WVector& position);
	void SetStartPosition(const WVector& position);
	void SetTargetPosition(const WVector& position);
	void SetAngle(float angle);
	void SetTargetAngle(float angle);
	void SetPendingTargetAngle(float angle);
	void SetState(unsigned long state);
	void SetAction(int action);
	void Stop();
	void Follow(const char* nickName);
	void ChangeMotion(int motion, const char* name, float blendTime);
	void Process(float elapsed);
	void Render();
	void RenderOverlay(bool bDetail);
	void SetChatFacial(int facial);
	void SetBalloonText(const char* text);
	void SetTradeTitle(const char* title);
	void RefreshUccClothes(unsigned long tid, unsigned long uid);

private:
	void SendMove(const WVector& delta, bool bForce);
	void ProcessMotion(float elapsed);
	void* AttachFx(const Attacher& attacher, const char* name, bool bSequence,
		defines_t* defines);
	void ProcessChangeState();
	void ScriptProcess(const w_motion_data* motion, float from, float to,
		bool bInit);
	void ParsePuppetScript(const char* script);
	void ParseBoundingBoxScript();
	void ParseScript_ShowVis(bool bShow, const char* groups);
	sTexturePart* FindTexturePart(const char* part, const char* texture);
	void SetPendingTexPart(const char* part, const char* texture);

public:
	char m_idName[22];
	char m_nickName[22];
	bool m_bMove;
	unsigned char m_pad983_2d[0x15];
	char m_followNickName[22];
	bool m_bChanneling;
	char m_channelingName[0x80];
	unsigned char m_pad983_d9[3];
	int m_guid;
	bool m_bAdministrator;
	unsigned char m_gender;
	bool m_bManner;
	bool m_bAngelicWings;
	unsigned long m_tidChar;
	unsigned long m_state;
	unsigned long m_prevState;
	unsigned long m_action;
	WVector m_position;
	WVector m_startPosition;
	float m_angle;
	float m_targetAngle;
	bool m_bPendingTargetAngle;
	float m_pendingTargetAngle;
	WVector m_up;
	WVector m_targetPosition;
	bool m_bTargetPositionChanged;
	int m_headIcon;
	WVector m_moveDelta;
	CRealMyRoom_AvatarBurden m_burden;
	unsigned long m_pad983_3d4;
#pragma pack(push, 1)
	bool m_bCharInfoChanged;
	sCharacterInfo m_charInfo;
#pragma pack(pop)
	CPartTidList m_partTidList;
	CPetFrame* m_pPetFrame;
	std::list<sTexturePart*> m_texParts;
	float m_time;
	LightSet m_light;
	bool m_bVisible;
	w_motion_data* m_pMotion;
	int m_motion;
	WBoneSet m_boneSet;
	w_motion_data* m_pPrevMotion;
	int m_prevMotion;
	WBoneSet m_prevBoneSet;
	float m_blendDuration;
	float m_motionTime;
	float m_lifeTime983; // 983 +0x798: zerado no ctor (007a750a) e somado no Process (007a866d); sem leitor
	float m_facialTime;
	unsigned long m_lastMotionSendTime;
	ntBalloonText* m_pBalloon; // 983 +0x7a4 (RenderOverlay 007a5670): nunca gravado nesta classe
	float m_random[3];
	float m_balloonTime;
	unsigned long m_balloonElapsed;
	char m_tradeTitle[0x40];
	Bitmap* m_pEmblem;
	CRealMyRoomMain* m_pMain;
	CPvsFxBox m_fxBox;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbParseboundingboxFunclets(); // HACK: EH funclets of the naked ParseBoundingBoxScript (nakedify)
#endif
};

class CRealMyRoom_AvatarManager : public WSingleton<CRealMyRoom_AvatarManager>
{
public:
	CRealMyRoom_AvatarManager();
	virtual ~CRealMyRoom_AvatarManager();

	void FreeAllAvatars();
	CRealMyRoom_Avatar* CreateAvatar(const sSlotInfo& slotInfo,
		const sCharacterInfo& charInfo, CRealMyRoomMain* pMain);
	bool DeleteAvatar(const char* idName);
	CRealMyRoom_Avatar* FindAvatarByIDName(const char* idName) const;
	CRealMyRoom_Avatar* FindAvatarByNickName(const char* nickName) const;
	CRealMyRoom_Avatar* FindAvatarByGuid(int guid) const;
	CRealMyRoom_Avatar* AvatarByRayIntersect(const WVector& pos,
		const WVector& dir) const;
	CRealMyRoom_Avatar* CheckClick(float x, float y) const;
	void Process(float elapsed);
	void Render();
	void RenderOverlay();
	void ResetAvatarPositionAngle();

	int m_avatarCount;
	CRealMyRoom_Avatar* m_pAvatar[30];
	CRealMyRoom_Avatar* m_pMyAvatar;
	CRealMyRoom_Avatar* m_pFocusAvatar;
	CRealMyRoom_Avatar* m_pClickAvatar; // 983 +0xac = CheckClick (007aa56e); lido no RenderOverlay (007a5c46)
	bool m_bRender;
	bool m_bOverlayDetail;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCreateavatarFunclets(); // HACK: EH funclets of the naked CreateAvatar (nakedify)
#endif
};
C_ASSERT(offsetof(CRealMyRoom_AvatarManager, m_pMyAvatar) == sizeof(WSingleton<CRealMyRoom_AvatarManager>) + 4 + 0x78); // A727: = visao parcial

#endif
