#pragma once
// JP 983: FrRoomListDlg (unidade 0051c9e0–00521710; vtables 00d07afc / 00d07af0 / 00d07ae8; sizeof 0x300 pelo `new` da derivada
// FrMultiplayRoomDlg em 00517660, que não acrescenta campo). Janela da lista de salas; a base é a FrChatDlg (mapa-base
// 00e04210). É o código `OnRoomList_*`/`RoomList_*` do `CLobbyMain` do 645 mudado para uma classe: os nomes dos handlers, dos
// controles e dos campos vêm de lá (sem o prefixo `RoomList_`). Os handlers da lista "roomlist" saíram daqui: estão no mapa
// da FrMultiplayRoomDlg e repassam para o objeto de +0x140. Layout (sessão B48): pelo construtor 0051d510 (grava de +0x140 a
// +0x2f4), pelos 23 handlers INIT e pelos dois carregadores de imagem (0051d6b0, 0051ca40) — de +0x160 a +0x24c a ordem é a
// do 645. Campos `m_<deslocamento>`: gravados pelo construtor, uso não levantado.

#include <list>
#include <string>
#include "chatdlg.h"
#include "lobbyroom.h"
#include "lobbycontainer983.h"

class FrArea;
class FrViewer;
class FrButton;
class FrListBox;
class FrForm;
class Bitmap;
struct sRoomInfo;
struct sBriefUserInfo;
class MsgObject;
class FrDtRoomDlg;
class FrWnd;
class FrRoomListDlg;

// documento 0xa5 do CContentsDoc (o mesmo da FrGameRoomDlg) = CLobbyContainer983 (include/jp/lobbycontainer983.h; A859: era
// visão POD com unk0 no lugar da vtable). O nome fica para os símbolos de jp.tsv (SetRoomListDoc983, CLobbyRoomSelector983).
// m_gameType/m_cate (+4/+8) são os `s_roomGameType`/`s_roomCate` do lobbymain.cpp do 645; m_selOID/m_selUID (+0xc/+0x10) os
// `m_selOID`/`m_selUID` do CLobbyMain do 645 (-1 = ninguém).
struct sRoomListDoc983 : public CLobbyContainer983
{
};

class FrRoomListDlg : public FrChatDlg
{
	DECLARE_OBJECT(FrRoomListDlg)

public:
	enum roomSort_t
	{
		ROOM_SORT_NO,
		ROOM_SORT_CATE,
		ROOM_SORT_TITLE,
		ROOM_SORT_GAMETYPE,
		ROOM_SORT_HOLE,
		ROOM_SORT_COURSE,
		ROOM_SORT_STATE
	};
	enum userSort_t
	{
		USER_SORT_GENDER,
		USER_SORT_LEVEL,
		USER_SORT_GUILD,
		USER_SORT_NICK
	};

	FrRoomListDlg(); // 0051d510
	virtual ~FrRoomListDlg(); // 0051c9f0

	friend class FrMultiplayRoomDlg;
	friend class CNormalLobbyRoom;
	friend class CGrandPrixLobbyRoom;
	friend class FrGrandPrixRoomDlg;
	friend class CLobbyMain; // 983 CLobbyMain::HandleMsg 0x4/0x5 chama Room1f0_983/Room220_983 (C457)

protected:
	virtual void OnProc(const float deltaTime); // 0051ca20
	virtual bool OnInit(); // 0051ed50

	void OnRoomSortNoInit(int param); // 0051d950
	void OnRoomSortNoDown(); // 00521510
	void OnRoomSortNoOwnerDraw(int param); // 0051d9d0
	void OnRoomSortCateAll_BtnInit(int param); // 0051da60
	void OnRoomSortCateAll_BtnDown(); // 0051ffa0
	void OnRoomSortCateVS_BtnInit(int param); // 0051dae0
	void OnRoomSortCateVS_BtnDown(); // 0051ffd0
	void OnRoomSortCateMass_BtnInit(int param); // 0051db60
	void OnRoomSortCateMass_BtnDown(); // 00520000
	void OnRoomSortCateBattle_BtnInit(int param); // 0051dbe0
	void OnRoomSortCateBattle_BtnDown(); // 00520030
	void OnRoomSortCateChat_BtnInit(int param); // 0051dc60
	void OnRoomSortCateChat_BtnDown(); // 00520060
	void OnRoomSortHoleInit(int param); // 0051dce0
	void OnRoomSortHoleDown(); // 00521590
	void OnRoomSortHoleOwnerDraw(int param); // 0051dd60
	void OnRoomSortCourseInit(int param); // 0051ddf0
	void OnRoomSortCourseDown(); // 00521610
	void OnRoomSortCourseOwnerDraw(int param); // 0051de70
	void OnRoomSortStateInit(int param); // 0051df00
	void OnRoomSortStateDown(); // 00521690
	void OnRoomSortStateOwnerDraw(int param); // 0051df80
	void OnRoomSortCateVS_ListInit(int param); // 0051e020
	void OnRoomSortCateVS_ListLUp(); // 00520090
	void OnRoomSortCateVS_ListOwnerDraw(int param); // 0051eec0
	void OnRoomSortCateMass_ListInit(int param); // 0051e0d0
	void OnRoomSortCateMass_ListLUp(); // 00520170
	void OnRoomSortCateMass_ListOwnerDraw(int param); // 0051f020
	void OnRoomSortCateBattle_ListInit(int param); // 0051e180
	void OnRoomSortCateBattle_ListLUp(); // 00520250
	void OnRoomSortCateBattle_ListOwnerDraw(int param); // 0051f180
	void OnMakeRoomInit(int param); // 0051e230
	void OnMakeRoomUp(); // 0051f2e0
	void OnCloseWndUp(); // 0051d0d0
	void OnCurEquipItemInit(int param); // 0051e2a0
	void OnCurEquipItemUp(); // 0051f490
	void OnQuestBtnInit(int param); // 0051e310
	void OnQuestBtnUp(); // 0051f4f0
	void OnUserListInit(int param); // 00520340
	void OnUserListLBtnUp(); // 0051e380
	void OnUserListRBtnUp(); // 00520c40
	void OnUserListOwnerDraw(int param); // 0051f5a0
	void OnUserSortGenderInit(int param); // 0051e3d0
	void OnUserSortGenderUp(); // 0051d0e0
	void OnUserSortGenderOwnerDraw(int param); // 0051e450
	void OnUserSortLevelInit(int param); // 0051e570
	void OnUserSortLevelUp(); // 0051d160
	void OnUserSortLevelOwnerDraw(int param); // 0051e5f0
	void OnUserSortGuildInit(int param); // 0051e710
	void OnUserSortGuildUp(); // 0051d1e0
	void OnUserSortGuildOwnerDraw(int param); // 0051e790
	void OnUserSortNickInit(int param); // 0051e8b0
	void OnUserSortNickUp(); // 0051d260
	void OnUserSortNickOwnerDraw(int param); // 0051e930
	void OnTabGproomInit(int param); // 0051fa10
	void OnTabGproomUp(); // 0051fa90
	void OnTabMtroomInit(int param); // 0051ea50
	void OnTabMtroomUp(); // 0051fad0
	void OnTabAvatarchatInit(int param); // 0051eac0
	void OnTabAvatarchatUp(); // 0051d2e0
	void FilterRoomList(roomSort_t sort, int value);
	void SortRoomList(roomSort_t sort, int unused);
	void SetCateTabBtn();
	void SortUserList(userSort_t sort);
	void SetControlsRoomListSort(int type);

	bool OnDtRoomDlgResult983(int result, FrForm* form); // 0051ee60
	void ShowTabGpRoom983(); // 0051ee90
	void SetTabs983(); // 0051ce80
	void DisableCateBtn983(int gameType); // 0051cee0
	void Room310_983(int a); // 0051cf70
	void Room1f0_983(int a); // 0051d060
	void Room220_983(int a); // 0051d090

public:
	void SetRoom983(CLobbyRoomSelector983* pRoom); // 0051ce40
	void EnableRoomEnterControls(bool enable); // 0051ed90 (público: o CLobbyMain do 983 chama; C57.1)
	void Set154_983(int a, int b, int c); // 0051ce50 (público: C57.1)
	void AddUser983(void* pUser); // 0051cf90
	void DelUser983(sBriefUserInfo* pUser, int index); // 0051cfc0
	void ClearUserList983(); // 0051d020
	void ClearRoomList983(); // 0051d0c0
	void PadRoomList983(int which); // 00521470
	void SetCateBtnByServerType983(); // 00520900
	void ResetControlsRoomListSort(float elapsed); // 00520520 (645: CLobbyMain::ResetControlsRoomListSort)
	void OpenDtRoomDlg983(const MsgObject& msg); // 0051fe80 (645: caso 0x33 do CLobbyMain::HandleMsg)

protected:
	void LoadRoomImages(); // 0051d6b0
	void LoadUserImages983(); // 0051ca40 (2ª metade do RoomList_LoadRoomImages do 645)

	CLobbyRoomSelector983* m_pRoom; // +0x140: seletor da sala do lobby (include/jp/lobbyroom.h); a FrMultiplayRoomDlg repassa a lista "roomlist" para ele
	sRoomListDoc983* m_pDoc; // +0x144: documento 0xa5 do CContentsDoc
	FrDtRoomDlg* m_pDtRoomDlg; // +0x148: janela de detalhes da sala (`EnableJoinBtn` 00ace410)
	FrForm* m_p14c; // +0x14c: janela aberta por cima (com ela a FrMultiplayRoomDlg não rola a lista)
	sRoomInfo* m_pSelRoom; // +0x150: sala escolhida na lista (o `m_pSelRoom` do CLobbyMain do 645; gravada pelas CLobbyRoom)
	int m_154; // na FrGrandPrixRoomDlg é a fonte (WTitleFontEx*) do número de salas (0050f700)
	int m_158;
	int m_15c;
	Bitmap* m_pRoomListBoxBmp[3][4]; // +0x160
	Bitmap* m_pRoomSortNoBmp[3]; // +0x190
	Bitmap* m_pRoomSortCateBmp[3]; // +0x19c
	Bitmap* m_pRoomSortHoleBmp[3]; // +0x1a8
	Bitmap* m_pRoomSortCourseBmp[3]; // +0x1b4
	Bitmap* m_pRoomSortStateUpBmp[3]; // +0x1c0
	Bitmap* m_pRoomSortStateDnBmp[3]; // +0x1cc
	Bitmap* m_pGenderBmp[10]; // +0x1d8
	Bitmap* m_pUserSortGenderBmp[3]; // +0x200
	Bitmap* m_pUserSortLevelBmp[3]; // +0x20c
	Bitmap* m_pUserSortGuildBmp[3]; // +0x218
	Bitmap* m_pUserSortNickBmp[3]; // +0x224
	unsigned long m_reserved230[3];
	Bitmap* m_pInGameBmp; // +0x23c
	Bitmap* m_pUserSortArrowBmp[2]; // +0x240
	Bitmap* m_pAIPlayerBmp; // +0x248: "i_aiplayer"
	FrListBox* m_pUserList; // +0x24c: "userlist"
	FrListBox* m_pRoomList; // +0x250: "roomlist" (gravado pela FrMultiplayRoomDlg, 005172a0)
	FrButton* m_pRoomSortCateAll; // +0x254: "btn_cate_total"
	FrButton* m_pRoomSortCateVS; // +0x258
	FrButton* m_pRoomSortCateMass; // +0x25c
	FrButton* m_pRoomSortCateBattle; // +0x260
	FrButton* m_pRoomSortCateChat; // +0x264
	FrListBox* m_pRoomSortCateVSList; // +0x268
	FrListBox* m_pRoomSortCateMassList; // +0x26c
	FrListBox* m_pRoomSortCateBattleList; // +0x270
	FrViewer* m_pRoomSortNo; // +0x274: "room_no"
	FrViewer* m_pRoomSortHole; // +0x278
	FrViewer* m_pRoomSortCourse; // +0x27c
	FrViewer* m_pRoomSortState; // +0x280
	FrButton* m_pCurEquipItem; // +0x284: "cur_equip_item"
	FrButton* m_pQuestBtn; // +0x288: "quest_btn"
	FrButton* m_pMakeRoom; // +0x28c: "makeroom"
	FrWnd* m_p290; // +0x290: controle sempre desligado pelo 0051ed90 (o "guildinfo" do 645, pela posição)
	FrButton* m_pUserInfo; // +0x294: desligado quando o jogador escolhido sai da lista (0051cfc0)
	FrViewer* m_pUserSortGender; // +0x298
	FrViewer* m_pUserSortLevel; // +0x29c
	FrViewer* m_pUserSortGuild; // +0x2a0
	FrViewer* m_pUserSortNick; // +0x2a4
	FrButton* m_pGPSortBtn[5]; // +0x2a8: botões de filtro da FrGrandPrixRoomDlg — "btn_sort_event", "btn_sort_rookie", "btn_sort_c1..c3" (B69)
	FrListBox* m_pRoomList2; // +0x2bc: lista do Grand Prix — é a que a CGrandPrixLobbyRoom usa (sessão B51); 0 e 1 no 00521470; quem grava não foi achado
	FrButton* m_pTabGpRoom; // +0x2c0: "def_bg.tab_gproom"
	FrButton* m_pTabMtRoom; // +0x2c4: "def_bg.tab_mtroom"
	FrButton* m_pTabAvatarChat; // +0x2c8: "def_bg.tab_avatarchat"
	float m_roomSortDelay; // +0x2cc
	bool m_bRoomSortDown[4]; // +0x2d0
	float m_userSortDelay; // +0x2d4
	float m_cateListTime[3]; // +0x2d8
	bool m_bUserSortDown[4]; // +0x2e4
	bool m_bCateSelected; // +0x2e8 (o construtor não grava)
	bool m_bRoomSortRev; // +0x2e9
	bool m_bUserSortRev; // +0x2ea
	roomSort_t m_roomSort; // +0x2ec
	userSort_t m_userSort; // +0x2f0: começa em 3
	int m_2f4; // na FrGrandPrixRoomDlg: número de salas do Grand Prix, escrito pelo "Gateway" (aviso 0x753e / 4 leva &m_2f4)
	char m_pad2f8[0x300 - 0x2f8];

	DECLARE_FRESH_MSGMAP()
};
