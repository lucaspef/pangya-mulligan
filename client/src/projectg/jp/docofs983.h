#pragma once
// D862: offsets crus do CSharedDoc copiados do 983 (= VC10) -> ancorados no membro que os contém.
// No cl novo (VS 2022) o CSharedDoc anda (STL menor: m_pGolfDoc 0x11024 -> 0x10cc4, A728); dentro de um membro POD
// (sRoomInfo, sMyInfo, sGolfGame, sGameTypeInfo...) o layout não muda. No VC10 a conta dobra para o mesmo offset
// (C_ASSERT_VC10 abaixo) e o obj não muda. `off` precisa ser constante; offset fora das âncoras fica cru (só VC10).
#include "shareddoc.h"
#include <stddef.h>

#define DOCOFS983_IN(off, m, v) \
	((unsigned)((off) - (v)) < sizeof(((CSharedDoc*)0)->m)) ? (offsetof(CSharedDoc, m) + ((off) - (v))) :
// B1490: no cl novo offset fora das ancoras = DOCOFS983_FORA (0xffffffff) -> DocOfs983<> sem definicao = erro de compilacao
// (o cru leria campo vizinho; out/B1490/hit.py: os 46 literais de hoje caem todos em ancora). No VC10 fica o cru (obj igual).
#if _MSC_VER > 1600
template<unsigned N> struct DocOfs983 { enum { v = N }; };
template<> struct DocOfs983<0xffffffffu>;
#define DOCOFS983_FORA(off) 0xffffffffu
#define DOCOFS983(off) ((unsigned)DocOfs983<DOCOFS983_CHAIN(off)>::v)
#else
#define DOCOFS983_FORA(off) (off)
#define DOCOFS983(off) DOCOFS983_CHAIN(off)
#endif
#define DOCOFS983_CHAIN(off) ( \
	DOCOFS983_IN(off, m_gameTypeInfo, 0xc8) \
	DOCOFS983_IN(off, m_myInfo, 0x7a0) \
	DOCOFS983_IN(off, m_myItemList, 0x375c) \
	DOCOFS983_IN(off, m_userInfo, 0x38b0) \
	DOCOFS983_IN(off, m_rivalList, 0x1042c) \
	DOCOFS983_IN(off, m_bGameOver, 0x10488) \
	DOCOFS983_IN(off, m_gameMode, 0x1048c) \
	DOCOFS983_IN(off, m_holeOrder, 0x1051e) \
	DOCOFS983_IN(off, m_approachStartTime, 0x1057c) \
	DOCOFS983_IN(off, m_bFastForward, 0x10588) \
	DOCOFS983_IN(off, m_golfGame, 0x1058c) \
	DOCOFS983_IN(off, m_unused47cc, 0x10700) \
	DOCOFS983_IN(off, m_unused47d0, 0x10704) \
	DOCOFS983_IN(off, m_bTreasureResult, 0x10777) \
	DOCOFS983_IN(off, m_bTreasureAward, 0x10778) \
	DOCOFS983_IN(off, m_roomInfo, 0x108f4) \
	DOCOFS983_IN(off, m_slotList, 0x109fc) \
	DOCOFS983_IN(off, m_roomUserList, 0x10a40) \
	DOCOFS983_IN(off, m_curChannel, 0x10a54) \
	DOCOFS983_IN(off, m_curGameServer, 0x10aa9) \
	DOCOFS983_IN(off, m_bRefreshCamera, 0x10d80) \
	DOCOFS983_IN(off, m_bMessengerAlarm, 0x10d81) \
	DOCOFS983_IN(off, m_serverProperty, 0x10d3c) \
	DOCOFS983_IN(off, m_inviteMode, 0x11380) \
	DOCOFS983_IN(off, m_bBongdariShopOpen, 0x11410) \
	DOCOFS983_IN(off, m_holePrizeItem983, 0x1145c) \
	DOCOFS983_IN(off, m_bFixHoleRepeat983, 0x11609) \
	DOCOFS983_FORA(off))
#define DOCPTR983(off) ((char*)Doc() + DOCOFS983(off))

C_ASSERT_VC10(offsetof(CSharedDoc, m_gameTypeInfo) == 0xc8 && offsetof(CSharedDoc, m_myInfo) == 0x7a0);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myItemList) == 0x375c && offsetof(CSharedDoc, m_bGameOver) == 0x10488);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame) == 0x1058c && offsetof(CSharedDoc, m_roomInfo) == 0x108f4);
C_ASSERT_VC10(offsetof(CSharedDoc, m_slotList) == 0x109fc && offsetof(CSharedDoc, m_roomUserList) == 0x10a40);
C_ASSERT_VC10(offsetof(CSharedDoc, m_curGameServer) == 0x10aa9 && offsetof(CSharedDoc, m_serverProperty) == 0x10d3c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bBongdariShopOpen) == 0x11410 && offsetof(CSharedDoc, m_bFixHoleRepeat983) == 0x11609);
C_ASSERT_VC10(offsetof(CSharedDoc, m_approachStartTime) == 0x1057c && offsetof(CSharedDoc, m_inviteMode) == 0x11380);
// B1461: crus restantes do golfrule.cpp (SaveShot/LoadReplay m_userInfo/m_curChannel, +0x10700/+0x10704, premio +0x11460).
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) == 0x38b0 && offsetof(CSharedDoc, m_curChannel) == 0x10a54);
C_ASSERT_VC10(offsetof(CSharedDoc, m_unused47cc) == 0x10700 && offsetof(CSharedDoc, m_unused47d0) == 0x10704);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holePrizeItem983) == 0x1145c && offsetof(CSharedDoc, m_cardAbilityList) == 0x37ec);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bFastForward) == 0x10588 && offsetof(CSharedDoc, m_bTreasureResult) == 0x10777);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bRefreshCamera) == 0x10d80 && offsetof(CSharedDoc, m_bMessengerAlarm) == 0x10d81);
// B1448: vector m_rivalList = _Myfirst (+0) / _Mylast (+4) nos dois compiladores (VC10 e cl 14.44 sem iterator debug).
C_ASSERT_VC10(offsetof(CSharedDoc, m_rivalList) == 0x1042c && offsetof(CSharedDoc, m_gameMode) == 0x1048c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeOrder) == 0x1051e && offsetof(CSharedDoc, m_bTreasureAward) == 0x10778);
// B1462: FindKeyByPlayer983 (seteffectobject.cpp) usa o membro; 983 +0x38a0.
C_ASSERT_VC10(offsetof(CSharedDoc, m_indexMap) == 0x38a0);
