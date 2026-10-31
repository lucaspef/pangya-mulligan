// JP 983: unidade 0051c9e0–00521710 (FrRoomListDlg). Janela da lista de salas: é o código `OnRoomList_*`/`RoomList_*` do
// `CLobbyMain` do 645 (src/projectg/lobbymain.cpp) mudado para uma classe. Sessão B48: corpos portados do 645 e conferidos
// com o 983 (gerador build/B/s48/gen.py + correções à mão); o que mudou no 983 vai comentado em cada função.
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "gamestringmanager.h"
#include "icontentsdoc.h"
#include "taskmanager.h"
#include "netresourcemanager.h"
#include "packet.h"
#include "wfont.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frviewer.h"
#include "frbutton.h"
#include "frgroupbox.h"
#include "frcombobox.h"
#include "frlistbox.h"
#include "frgaugebar.h"
#include "frstatic.h"
#include "fredit.h"
#include "frgraphicinterface.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "bitmap.h"
#include "changeroominfodlg.h"
#include "messengerdlg.h"
#include "userlistdlg.h"
#include "lobbyavataractor.h"
#include "user_info.h"
#include "lobbyjp.h"
#include "rftooltiphelper.h"
#include "skinnedtheme.h"
#include "makeroomdlg.h"
#include "passworddlg.h"
#include "contentsdoc.h"
#include "guilddefine.h"
#include "frdesktop.h"
#include "intrusion.h"
#include "detailedroominfodlg.h"
#include "roomlistdlg.h"

extern Fresh* g_pFresh;

#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))

int float2int(float f);
// comparadores das listas: ficam na unidade do lobby (0059f280–0059f390 no 983; lobbymain.cpp do 645)
bool RoomNoCompare(const void* a, const void* b);
bool RoomNoRevCompare(const void* a, const void* b);
bool RoomHoleCompare(const void* a, const void* b);
bool RoomHoleRevCompare(const void* a, const void* b);
bool RoomCourseCompare(const void* a, const void* b);
bool RoomCourseRevCompare(const void* a, const void* b);
bool RoomTitleCompare(const void* a, const void* b);
bool RoomTitleRevCompare(const void* a, const void* b);
bool RoomGameTypeCompare(const void* a, const void* b);
bool RoomGameTypeRevCompare(const void* a, const void* b);
bool RoomStateCompare(const void* a, const void* b);
bool RoomStateRevCompare(const void* a, const void* b);
bool UserGenderCompare(const void* a, const void* b);
bool UserGenderRevCompare(const void* a, const void* b);
bool UserLevelCompare(const void* a, const void* b);
bool UserLevelRevCompare(const void* a, const void* b);
bool UserGuildCompare(const void* a, const void* b);
bool UserGuildRevCompare(const void* a, const void* b);
bool UserNicknameCompare(const void* a, const void* b);
bool UserNicknameRevCompare(const void* a, const void* b);

// 00e74ee8 (8 textos de 0x1c): tipos de jogo das três listas de categoria
// 983 00ccd050: inicializador dinamico, cada texto = K2L_Compatibility(...)
std::string g_strGameType983[8] = {
	K2L_Compatibility("\xbd\xba\xc6\xae\xb7\xce\xc5\xa9"),
	K2L_Compatibility("\xb8\xc5\xc4\xa1"),
	K2L_Compatibility("30\xc0\xce\xb4\xeb\xc8\xb8"),
	K2L_Compatibility("30\xc0\xce\xc6\xc0\xc0\xfc"),
	K2L_Compatibility("\xb1\xe6\xb5\xe5\xb4\xeb\xc0\xfc"),
	K2L_Compatibility("\xc6\xce\xb9\xe8\xc6\xb2"),
	K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"),
	K2L_Compatibility("\xb4\xeb\xc8\xad") };

int GetCateByGameType(int gameType); // 0059f130 (lobbymain.cpp)

// modos 0x14 e 0x15 (Grand Prix); o 983 testa como `(modo + -0x14) <= 1` sem sinal (forma de `switch`, como no gameroomdlg.cpp)
inline bool IsGPMode983(int mode)
{
	switch (mode)
	{
	case 0x14:
	case 0x15:
		return true;
	}
	return false;
}
// sala que aparece na lista: sem o bit 0 de +0xd9 e fora do Grand Prix (byte +0xc4); cópia fora de linha em 0051d4e0
#define IS_LISTED_ROOM983(p) (!(*((unsigned char*)(p) + 0xd9) & 1) && !IsGPMode983(*((unsigned char*)(p) + 0xc4)))

#define DOC983_U32(off) (*(unsigned long*)DOCPTR983(off))
#define DOCJP() ((CSharedDocJp*)Doc())
// empréstimos da frente A sem nome no mapa (nomes meus, pelo endereço)
class CSharedDoc983b
{
public:
	void SetRoomListKind983(int a); // 0062bf40
	void SetRoomListRequest983(int a); // 0062bf60: 1 = aba do Grand Prix, 0 = aba das salas
};
// CSharedDoc+0x10d3c: bits lidos como campo (`shr 0xb; test 1`); bit 11 = Grand Prix liberado
struct sDocFlags983
{
	unsigned long b0 : 11;
	unsigned long b11 : 1;
};
#define DOCFLAGS983() ((sDocFlags983*)DOCPTR983(0x10d3c))
#define DOCJP983B() ((CSharedDoc983b*)Doc())
bool LatestVersion983(); // 0059fa80 (`LatestVersion` do CLobbyMain do 645; no 983 não usa o `this`)
int GetServerType983(); // 00b2eae0 (classicserver.cpp)

// 008a9ae0: o `CUserInfo::SetInfo` do 983 tem dois argumentos a mais (zero aqui). PROVISÓRIO: pedido B → A (B44).
class CUserInfoJp
{
public:
	void SetInfo983(unsigned long uid, unsigned long guid, bool bCheckTime, bool bOption, bool bMsnControl,
		bool bControlOff, std::string nickname, int a8, int a9);
};

IMPLEMENT_OBJECT(FrRoomListDlg, FrChatDlg)

BEGIN_FRESH_MSGMAP(FrRoomListDlg, FrChatDlg)

ON_FRESH_VI("room_no", FRCMD_INIT, FrRoomListDlg::OnRoomSortNoInit)
ON_FRESH_VV("room_no", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnRoomSortNoDown)
ON_FRESH_VI("room_no", FRCMD_OWNERDRAW, FrRoomListDlg::OnRoomSortNoOwnerDraw)
ON_FRESH_VI("btn_cate_total", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateAll_BtnInit)
ON_FRESH_VV("btn_cate_total", FRCMD_LBUTTONUP, FrRoomListDlg::OnRoomSortCateAll_BtnDown)
ON_FRESH_VI("btn_cate_vs", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateVS_BtnInit)
ON_FRESH_VV("btn_cate_vs", FRCMD_LBUTTONUP, FrRoomListDlg::OnRoomSortCateVS_BtnDown)
ON_FRESH_VI("btn_cate_mass", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateMass_BtnInit)
ON_FRESH_VV("btn_cate_mass", FRCMD_LBUTTONUP, FrRoomListDlg::OnRoomSortCateMass_BtnDown)
ON_FRESH_VI("btn_cate_battle", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateBattle_BtnInit)
ON_FRESH_VV("btn_cate_battle", FRCMD_LBUTTONUP, FrRoomListDlg::OnRoomSortCateBattle_BtnDown)
ON_FRESH_VI("btn_cate_chat", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateChat_BtnInit)
ON_FRESH_VV("btn_cate_chat", FRCMD_LBUTTONUP, FrRoomListDlg::OnRoomSortCateChat_BtnDown)
ON_FRESH_VI("room_hole", FRCMD_INIT, FrRoomListDlg::OnRoomSortHoleInit)
ON_FRESH_VV("room_hole", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnRoomSortHoleDown)
ON_FRESH_VI("room_hole", FRCMD_OWNERDRAW, FrRoomListDlg::OnRoomSortHoleOwnerDraw)
ON_FRESH_VI("room_course", FRCMD_INIT, FrRoomListDlg::OnRoomSortCourseInit)
ON_FRESH_VV("room_course", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnRoomSortCourseDown)
ON_FRESH_VI("room_course", FRCMD_OWNERDRAW, FrRoomListDlg::OnRoomSortCourseOwnerDraw)
ON_FRESH_VI("room_state", FRCMD_INIT, FrRoomListDlg::OnRoomSortStateInit)
ON_FRESH_VV("room_state", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnRoomSortStateDown)
ON_FRESH_VI("room_state", FRCMD_OWNERDRAW, FrRoomListDlg::OnRoomSortStateOwnerDraw)
ON_FRESH_VI("cate_vs_list", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateVS_ListInit)
ON_FRESH_VV("cate_vs_list", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnRoomSortCateVS_ListLUp)
ON_FRESH_VI("cate_vs_list", FRCMD_OWNERDRAW, FrRoomListDlg::OnRoomSortCateVS_ListOwnerDraw)
ON_FRESH_VI("cate_mass_list", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateMass_ListInit)
ON_FRESH_VV("cate_mass_list", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnRoomSortCateMass_ListLUp)
ON_FRESH_VI("cate_mass_list", FRCMD_OWNERDRAW, FrRoomListDlg::OnRoomSortCateMass_ListOwnerDraw)
ON_FRESH_VI("cate_battle_list", FRCMD_INIT, FrRoomListDlg::OnRoomSortCateBattle_ListInit)
ON_FRESH_VV("cate_battle_list", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnRoomSortCateBattle_ListLUp)
ON_FRESH_VI("cate_battle_list", FRCMD_OWNERDRAW, FrRoomListDlg::OnRoomSortCateBattle_ListOwnerDraw)
ON_FRESH_VI("makeroom", FRCMD_INIT, FrRoomListDlg::OnMakeRoomInit)
ON_FRESH_VV("makeroom", FRCMD_LBUTTONUP, FrRoomListDlg::OnMakeRoomUp)
ON_FRESH_VV("close_wnd", FRCMD_LBUTTONUP, FrRoomListDlg::OnCloseWndUp)
ON_FRESH_VI("cur_equip_item", FRCMD_INIT, FrRoomListDlg::OnCurEquipItemInit)
ON_FRESH_VV("cur_equip_item", FRCMD_LBUTTONUP, FrRoomListDlg::OnCurEquipItemUp)
ON_FRESH_VI("quest_btn", FRCMD_INIT, FrRoomListDlg::OnQuestBtnInit)
ON_FRESH_VV("quest_btn", FRCMD_LBUTTONUP, FrRoomListDlg::OnQuestBtnUp)
ON_FRESH_VI("userlist", FRCMD_INIT, FrRoomListDlg::OnUserListInit)
ON_FRESH_VV("userlist", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnUserListLBtnUp)
ON_FRESH_VV("userlist", FRCMD_RBUTTONUP, FrRoomListDlg::OnUserListRBtnUp)
ON_FRESH_VI("userlist", FRCMD_OWNERDRAW, FrRoomListDlg::OnUserListOwnerDraw)
ON_FRESH_VI("user_gender", FRCMD_INIT, FrRoomListDlg::OnUserSortGenderInit)
ON_FRESH_VV("user_gender", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnUserSortGenderUp)
ON_FRESH_VI("user_gender", FRCMD_OWNERDRAW, FrRoomListDlg::OnUserSortGenderOwnerDraw)
ON_FRESH_VI("user_level", FRCMD_INIT, FrRoomListDlg::OnUserSortLevelInit)
ON_FRESH_VV("user_level", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnUserSortLevelUp)
ON_FRESH_VI("user_level", FRCMD_OWNERDRAW, FrRoomListDlg::OnUserSortLevelOwnerDraw)
ON_FRESH_VI("user_guild", FRCMD_INIT, FrRoomListDlg::OnUserSortGuildInit)
ON_FRESH_VV("user_guild", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnUserSortGuildUp)
ON_FRESH_VI("user_guild", FRCMD_OWNERDRAW, FrRoomListDlg::OnUserSortGuildOwnerDraw)
ON_FRESH_VI("user_nick", FRCMD_INIT, FrRoomListDlg::OnUserSortNickInit)
ON_FRESH_VV("user_nick", FRCMD_LBUTTONDOWN, FrRoomListDlg::OnUserSortNickUp)
ON_FRESH_VI("user_nick", FRCMD_OWNERDRAW, FrRoomListDlg::OnUserSortNickOwnerDraw)
ON_FRESH_VI("def_bg.tab_gproom", FRCMD_INIT, FrRoomListDlg::OnTabGproomInit)
ON_FRESH_VV("def_bg.tab_gproom", FRCMD_LBUTTONUP, FrRoomListDlg::OnTabGproomUp)
ON_FRESH_VI("def_bg.tab_mtroom", FRCMD_INIT, FrRoomListDlg::OnTabMtroomInit)
ON_FRESH_VV("def_bg.tab_mtroom", FRCMD_LBUTTONUP, FrRoomListDlg::OnTabMtroomUp)
ON_FRESH_VI("def_bg.tab_avatarchat", FRCMD_INIT, FrRoomListDlg::OnTabAvatarchatInit)
ON_FRESH_VV("def_bg.tab_avatarchat", FRCMD_LBUTTONUP, FrRoomListDlg::OnTabAvatarchatUp)

END_FRESH_MSGMAP()

// 0051d950 (645: CLobbyMain::OnRoomList_RoomSortNoInit)
void FrRoomListDlg::OnRoomSortNoInit(int param)
{
	m_pRoomSortNo = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bRoomSortDown[0] = false;
}

// 00521510 (645: CLobbyMain::OnRoomList_RoomSortNoDown)
void FrRoomListDlg::OnRoomSortNoDown()
{
	if (!(m_roomSortDelay > 0.0f))
	{
		if (m_roomSort == ROOM_SORT_NO)
			m_bRoomSortRev ^= 1;

		SetControlsRoomListSort(ROOM_SORT_NO);
		m_bRoomSortDown[0] = true;
		m_roomSortDelay = 0.3f;
		SortRoomList(ROOM_SORT_NO, 0);
		g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}
}

// 0051d9d0 (645: CLobbyMain::OnRoomList_RoomSortNoOwnerDraw)
void FrRoomListDlg::OnRoomSortNoOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		if (m_pRoomSortNo)
		{
			int state = 0;

			if (m_bRoomSortDown[0])
			{
				state = 2;
			}
			else
			{
				WRect rect = m_pRoomSortNo->GetRect();

				if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
					state = 1;
			}

			pGDI->DrawTexture(m_pRoomSortNoBmp[state], m_pRoomSortNo->GetRect(),
				0xffffffff, 0);
		}
	}
}

// 0051da60 (645: CLobbyMain::OnRoomList_RoomSortCateAll_BtnInit)
void FrRoomListDlg::OnRoomSortCateAll_BtnInit(int param)
{
	m_pRoomSortCateAll = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRoomSortCateAll->SetStatus(
		m_pDoc->m_cate == 4 ? FrButton::PRESSED : FrButton::NORMAL);
}

// 0051ffa0 (645: CLobbyMain::OnRoomList_RoomSortCateAll_BtnDown)
void FrRoomListDlg::OnRoomSortCateAll_BtnDown()
{
	FilterRoomList(ROOM_SORT_CATE, 4);
	SetCateTabBtn();

	m_pDoc->m_gameType = -1;
}

// 0051dae0 (645: CLobbyMain::OnRoomList_RoomSortCateVS_BtnInit)
void FrRoomListDlg::OnRoomSortCateVS_BtnInit(int param)
{
	m_pRoomSortCateVS = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRoomSortCateVS->SetStatus(
		m_pDoc->m_cate == 0 ? FrButton::PRESSED : FrButton::NORMAL);
}

// 0051ffd0 (645: CLobbyMain::OnRoomList_RoomSortCateVS_BtnDown)
void FrRoomListDlg::OnRoomSortCateVS_BtnDown()
{
	FilterRoomList(ROOM_SORT_CATE, 0);
	SetCateTabBtn();

	m_cateListTime[0] = 0.0f;
	m_pDoc->m_gameType = -1;
}

// 0051db60 (645: CLobbyMain::OnRoomList_RoomSortCateMass_BtnInit)
void FrRoomListDlg::OnRoomSortCateMass_BtnInit(int param)
{
	m_pRoomSortCateMass = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRoomSortCateMass->SetStatus(
		m_pDoc->m_cate == 1 ? FrButton::PRESSED : FrButton::NORMAL);
}

// 00520000 (645: CLobbyMain::OnRoomList_RoomSortCateMass_BtnDown)
void FrRoomListDlg::OnRoomSortCateMass_BtnDown()
{
	FilterRoomList(ROOM_SORT_CATE, 1);
	SetCateTabBtn();

	m_cateListTime[1] = 0.0f;
	m_pDoc->m_gameType = -1;
}

// 0051dbe0 (645: CLobbyMain::OnRoomList_RoomSortCateBattle_BtnInit)
void FrRoomListDlg::OnRoomSortCateBattle_BtnInit(int param)
{
	m_pRoomSortCateBattle = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRoomSortCateBattle->SetStatus(
		m_pDoc->m_cate == 2 ? FrButton::PRESSED : FrButton::NORMAL);
}

// 00520030 (645: CLobbyMain::OnRoomList_RoomSortCateBattle_BtnDown)
void FrRoomListDlg::OnRoomSortCateBattle_BtnDown()
{
	FilterRoomList(ROOM_SORT_CATE, 2);
	SetCateTabBtn();

	m_cateListTime[2] = 0.0f;
	m_pDoc->m_gameType = -1;
}

// 0051dc60 (645: CLobbyMain::OnRoomList_RoomSortCateChat_BtnInit)
void FrRoomListDlg::OnRoomSortCateChat_BtnInit(int param)
{
	m_pRoomSortCateChat = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRoomSortCateChat->SetStatus(
		m_pDoc->m_cate == 3 ? FrButton::PRESSED : FrButton::NORMAL);
}

// 00520060 (645: CLobbyMain::OnRoomList_RoomSortCateChat_BtnDown)
void FrRoomListDlg::OnRoomSortCateChat_BtnDown()
{
	FilterRoomList(ROOM_SORT_CATE, 3);
	SetCateTabBtn();

	m_pDoc->m_gameType = -1;
}

// 0051dce0 (645: CLobbyMain::OnRoomList_RoomSortHoleInit)
void FrRoomListDlg::OnRoomSortHoleInit(int param)
{
	m_pRoomSortHole = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bRoomSortDown[1] = false;
}

// 00521590 (645: CLobbyMain::OnRoomList_RoomSortHoleDown)
void FrRoomListDlg::OnRoomSortHoleDown()
{
	if (!(m_roomSortDelay > 0.0f))
	{
		if (m_roomSort == ROOM_SORT_HOLE)
			m_bRoomSortRev ^= 1;

		SetControlsRoomListSort(ROOM_SORT_HOLE);
		m_bRoomSortDown[1] = true;
		m_roomSortDelay = 0.3f;
		SortRoomList(ROOM_SORT_HOLE, 0);
		g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}
}

// 0051dd60 (645: CLobbyMain::OnRoomList_RoomSortHoleOwnerDraw)
void FrRoomListDlg::OnRoomSortHoleOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		if (m_pRoomSortHole)
		{
			int state = 0;

			if (m_bRoomSortDown[1])
			{
				state = 2;
			}
			else
			{
				WRect rect = m_pRoomSortHole->GetRect();

				if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
					state = 1;
			}

			pGDI->DrawTexture(m_pRoomSortHoleBmp[state],
				m_pRoomSortHole->GetRect(), 0xffffffff, 0);
		}
	}
}

// 0051ddf0 (645: CLobbyMain::OnRoomList_RoomSortCourseInit)
void FrRoomListDlg::OnRoomSortCourseInit(int param)
{
	m_pRoomSortCourse = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bRoomSortDown[2] = false;
}

// 00521610 (645: CLobbyMain::OnRoomList_RoomSortCourseDown)
void FrRoomListDlg::OnRoomSortCourseDown()
{
	if (!(m_roomSortDelay > 0.0f))
	{
		if (m_roomSort == ROOM_SORT_COURSE)
			m_bRoomSortRev ^= 1;

		SetControlsRoomListSort(ROOM_SORT_COURSE);
		m_bRoomSortDown[2] = true;
		m_roomSortDelay = 0.3f;
		SortRoomList(ROOM_SORT_COURSE, 0);
		g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}
}

// 0051de70 (645: CLobbyMain::OnRoomList_RoomSortCourseOwnerDraw)
void FrRoomListDlg::OnRoomSortCourseOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		if (m_pRoomSortCourse)
		{
			int state = 0;

			if (m_bRoomSortDown[2])
			{
				state = 2;
			}
			else
			{
				WRect rect = m_pRoomSortCourse->GetRect();

				if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
					state = 1;
			}

			pGDI->DrawTexture(m_pRoomSortCourseBmp[state],
				m_pRoomSortCourse->GetRect(), 0xffffffff, 0);
		}
	}
}

// 0051df00 (645: CLobbyMain::OnRoomList_RoomSortStateInit)
void FrRoomListDlg::OnRoomSortStateInit(int param)
{
	m_pRoomSortState = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bRoomSortDown[3] = false;
}

// 00521690 (645: CLobbyMain::OnRoomList_RoomSortStateDown)
void FrRoomListDlg::OnRoomSortStateDown()
{
	if (m_roomSortDelay > 0.0f)
		return;

	if (m_roomSort == 6)
	{
		m_bRoomSortRev ^= 1;
	}

	SetControlsRoomListSort((roomSort_t)6);
	m_bRoomSortDown[3] = true;
	m_roomSortDelay = 0.3f;
	SortRoomList((roomSort_t)6, 0);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 0051df80 (645: CLobbyMain::OnRoomList_RoomSortStateOwnerDraw)
void FrRoomListDlg::OnRoomSortStateOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();

	if (!pDevice)
		return;

	if (!m_pRoomSortState)
		return;

	int state = 0;

	if (m_bRoomSortDown[3])
		state = 2;
	else
	{
		WRect rect = m_pRoomSortState->GetRect();
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			state = 1;
	}

	pDevice->DrawTexture(m_pRoomSortStateUpBmp[m_bRoomSortRev * 3 + state],
		m_pRoomSortState->GetRect(), 0xffffffff, 0);
}

// 0051e020 (645: CLobbyMain::OnRoomList_RoomSortCateVS_ListInit)
void FrRoomListDlg::OnRoomSortCateVS_ListInit(int param)
{
	m_pRoomSortCateVSList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pRoomSortCateVSList)
	{
		for (int i = 0; i < 2; i++)
		{
			m_pRoomSortCateVSList->AddItem(&g_strGameType983[i]);
		}

		m_pRoomSortCateVSList->SetVisible(false);
		m_pRoomSortCateVSList->EnableScrollBar(false);
	}
}

// 00520090 (645: CLobbyMain::OnRoomList_RoomSortCateVS_ListLUp)
void FrRoomListDlg::OnRoomSortCateVS_ListLUp()
{
	int count = m_pRoomSortCateVSList->GetCurrentItemSize(true);

	for (int i = 0; i < count; i++)
	{
		FrListItem* pItem = m_pRoomSortCateVSList->GetItem(i);
		if (!pItem)
			continue;

		WRect rect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 54.0f, 21.0f);
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		{
			m_bCateSelected = true;

			FilterRoomList((roomSort_t)3, pItem->idx);
			break;
		}
	}

	if (m_pRoomSortCateVSList)
		m_pRoomSortCateVSList->SetVisible(false);
}

// 0051eec0 (645: CLobbyMain::OnRoomList_RoomSortCateVS_ListOwnerDraw)
void FrRoomListDlg::OnRoomSortCateVS_ListOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;

	if (!pDevice)
		return;
	if (!pItem)
		return;

	std::string* pStr = (std::string*)pItem->pData;
	WRect rect;
	rect.x = pItem->pos.x;
	rect.y = pItem->pos.y;
	rect.w = 56.0f;
	rect.h = 22.0f;

	pDevice->Box(rect, 0x880e4f75);

	rect.x = pItem->pos.x + 1.0f;
	rect.y = pItem->pos.y + 1.0f;
	rect.w = 54.0f;
	rect.h = 20.0f;

	if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		pDevice->Box(rect, 0x88082b46);
		pDevice->SetTextStyle(1);
	}
	else
		pDevice->SetTextStyle(0);

	pDevice->SetTextColor(0xffffffff, 0xffffffff);
	pDevice->Print(WPoint(pItem->pos.x + 2.0f, pItem->pos.y + 6.0f), 0,
		pStr->c_str());
}

// 0051e0d0 (645: CLobbyMain::OnRoomList_RoomSortCateMass_ListInit)
void FrRoomListDlg::OnRoomSortCateMass_ListInit(int param)
{
	m_pRoomSortCateMassList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pRoomSortCateMassList)
	{
		for (int i = 2; i < 5; i++)
		{
			m_pRoomSortCateMassList->AddItem(&g_strGameType983[i]);
		}

		m_pRoomSortCateMassList->SetVisible(false);
		m_pRoomSortCateMassList->EnableScrollBar(false);
	}
}

// 00520170 (645: CLobbyMain::OnRoomList_RoomSortCateMass_ListLUp)
void FrRoomListDlg::OnRoomSortCateMass_ListLUp()
{
	int count = m_pRoomSortCateMassList->GetCurrentItemSize(true);

	for (int i = 0; i < count; i++)
	{
		FrListItem* pItem = m_pRoomSortCateMassList->GetItem(i);
		if (!pItem)
			continue;

		WRect rect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 54.0f, 21.0f);
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		{
			m_bCateSelected = true;

			FilterRoomList((roomSort_t)3, pItem->idx + 4);
			break;
		}
	}

	if (m_pRoomSortCateMassList)
		m_pRoomSortCateMassList->SetVisible(false);
}

// 0051f020 (645: CLobbyMain::OnRoomList_RoomSortCateMass_ListOwnerDraw)
void FrRoomListDlg::OnRoomSortCateMass_ListOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;

	if (!pDevice)
		return;
	if (!pItem)
		return;

	std::string* pStr = (std::string*)pItem->pData;
	WRect rect;
	rect.x = pItem->pos.x;
	rect.y = pItem->pos.y;
	rect.w = 56.0f;
	rect.h = 22.0f;

	pDevice->Box(rect, 0x880e4f75);

	rect.x = pItem->pos.x + 1.0f;
	rect.y = pItem->pos.y + 1.0f;
	rect.w = 54.0f;
	rect.h = 20.0f;

	if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		pDevice->Box(rect, 0x88082b46);
		pDevice->SetTextStyle(1);
	}
	else
		pDevice->SetTextStyle(0);

	pDevice->SetTextColor(0xffffffff, 0xffffffff);
	pDevice->Print(WPoint(pItem->pos.x + 2.0f, pItem->pos.y + 6.0f), 0,
		pStr->c_str());
}

// 0051e180 (645: CLobbyMain::OnRoomList_RoomSortCateBattle_ListInit)
void FrRoomListDlg::OnRoomSortCateBattle_ListInit(int param)
{
	m_pRoomSortCateBattleList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pRoomSortCateBattleList)
	{
		for (int i = 5; i < 7; i++)
		{
			m_pRoomSortCateBattleList->AddItem(&g_strGameType983[i]);
		}

		m_pRoomSortCateBattleList->SetVisible(false);
		m_pRoomSortCateBattleList->EnableScrollBar(false);
	}
}

// 00520250 (645: CLobbyMain::OnRoomList_RoomSortCateBattle_ListLUp)
void FrRoomListDlg::OnRoomSortCateBattle_ListLUp()
{
	int count = m_pRoomSortCateBattleList->GetCurrentItemSize(true);

	for (int i = 0; i < count; i++)
	{
		FrListItem* pItem = m_pRoomSortCateBattleList->GetItem(i);
		if (!pItem)
			continue;

		WRect rect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 54.0f, 21.0f);
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		{
			m_bCateSelected = true;
			FilterRoomList((roomSort_t)3, pItem->idx ? 10 : 7);
			break;
		}
	}

	if (m_pRoomSortCateBattleList)
		m_pRoomSortCateBattleList->SetVisible(false);
}

// 0051f180 (645: CLobbyMain::OnRoomList_RoomSortCateBattle_ListOwnerDraw)
void FrRoomListDlg::OnRoomSortCateBattle_ListOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;

	if (!pDevice)
		return;
	if (!pItem)
		return;

	std::string* pStr = (std::string*)pItem->pData;
	WRect rect;
	rect.x = pItem->pos.x;
	rect.y = pItem->pos.y;
	rect.w = 56.0f;
	rect.h = 22.0f;

	pDevice->Box(rect, 0x880e4f75);

	rect.x = pItem->pos.x + 1.0f;
	rect.y = pItem->pos.y + 1.0f;
	rect.w = 54.0f;
	rect.h = 20.0f;

	if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		pDevice->Box(rect, 0x88082b46);
		pDevice->SetTextStyle(1);
	}
	else
		pDevice->SetTextStyle(0);

	pDevice->SetTextColor(0xffffffff, 0xffffffff);
	pDevice->Print(WPoint(pItem->pos.x + 2.0f, pItem->pos.y + 6.0f), 0,
		pStr->c_str());
}

// 0051e230 (645: CLobbyMain::OnRoomList_MakeRoomInit)
// 983: sem as imagens do tema
void FrRoomListDlg::OnMakeRoomInit(int param)
{
	m_pMakeRoom = DYNAMIC_CAST(FrButton, param);
}

// 0051f2e0 (645: CLobbyMain::OnRoomList_MakeRoomUp)
// 983: sem `CloseAllDialogs`; o servidor em manutenção avisa com o texto pela mensagem 0x23 e a janela de criar sala é aberta
// pelo ator principal (mensagem 0x8b; 0 = pede antes a lista de servidores com o pacote 0x43, 1 = abre)
void FrRoomListDlg::OnMakeRoomUp()
{
	if (!LatestVersion983())
		return;

	if (DOCJP()->IsControlServerService983(2))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)K2L_Compatibility("\xb0\xd4\xc0\xd3 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
		return;
	}

	if (Doc()->m_curChannel.Uid == 0xff)
	{
		WSendPacket packet((enumClientPacket)0x43);
		packet.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x8b, 0, 0, 0, 0, 0));
	}
	else
	{
		EnableRoomEnterControls(false);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x8b, 1, 0, 0, 0, 0));
	}
}

// 0051d0d0
void FrRoomListDlg::OnCloseWndUp()
{
	Close(FrOK, true);
}

// 0051e2a0
void FrRoomListDlg::OnCurEquipItemInit(int param)
{
	m_pCurEquipItem = DYNAMIC_CAST(FrButton, param);
}

// 0051f490
void FrRoomListDlg::OnCurEquipItemUp()
{
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x8b, 5, 0, 0, 0, 0);
}

// 0051e310
void FrRoomListDlg::OnQuestBtnInit(int param)
{
	m_pQuestBtn = DYNAMIC_CAST(FrButton, param);
}

// 0051f4f0
void FrRoomListDlg::OnQuestBtnUp()
{
	{
		WSendPacket packet((enumClientPacket)0x151);
		packet.Send(TO_GAME);
	}
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 1, 0, 0, 0, 0, 0);
}

// 00520340 (645: CLobbyMain::OnRoomList_UserListInit)
void FrRoomListDlg::OnUserListInit(int param)
{
	m_pUserList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (!m_pUserList)
		return;

	m_pUserList->UseRightButton(true);

	for (std::map<unsigned long, sBriefUserInfo>::iterator it =
			 Doc()->m_briefUserInfoMap.begin();
		it != Doc()->m_briefUserInfoMap.end(); ++it)
	{
		if (Doc()->m_myInfo.info.dwIdentity & 4)
			m_pUserList->AddItem((void*)&(*it).second);
		else if (!(sBriefUserInfo((*it).second).dwIdentity & 4))
			m_pUserList->AddItem((void*)&(*it).second);
		else if (sBriefUserInfo((*it).second).state & 1)
			m_pUserList->AddItem((void*)&(*it).second);
	}
}

// 0051e380 (645: CLobbyMain::OnRoomList_UserListLBtnUp)
void FrRoomListDlg::OnUserListLBtnUp()
{
	FrListItem* pItem = m_pUserList->GetItemUnderCursor();
	if (pItem)
	{
		sBriefUserInfo* pInfo = (sBriefUserInfo*)pItem->pData;

		if (pInfo)
		{
			m_pDoc->m_selOID = pInfo->dwGuid;
			m_pDoc->m_selUID = pInfo->dwUid;

			if (m_pUserInfo)
				m_pUserInfo->Enable(true);
		}

		pItem->selected = true;
	}
}

// 00520c40 (645: CLobbyMain::OnRoomList_UserListRBtnUp)
void FrRoomListDlg::OnUserListRBtnUp()
{
	FrListItem* pItem = m_pUserList->GetItemUnderCursor();
	if (pItem)
	{
		sBriefUserInfo* pInfo = (sBriefUserInfo*)pItem->pData;

		if (pInfo)
		{
			m_pDoc->m_selOID = pInfo->dwGuid;
			m_pDoc->m_selUID = pInfo->dwUid;

			if (m_pUserInfo)
				m_pUserInfo->Enable(true);

			if (CUserInfo::IsInstantiated())
			{
				((CUserInfoJp*)CUserInfo::Instance())->SetInfo983(pInfo->dwUid, pInfo->dwGuid,
					true, true, false, false, pInfo->sNick, 0, 0);
			}
		}
		else
		{
			CChatMsg::Instance()->AddChatMsg(
				K2L_Compatibility("\xc0\xdf\xb8\xf8\xb5\xc8 \xb4\xeb\xbb\xf3\xc0\xd4\xb4\xcf\xb4\xd9."),
				7, true, false);
		}

		pItem->selected = true;
	}
}

// A215: entrada do pacote 0x0046 (200 B) e deslocamentos lidos pelo desenho da lista no 983 (00af98f0)
C_ASSERT(sizeof(sBriefUserInfo) == 200);
C_ASSERT(offsetof(sBriefUserInfo, dwGuid) == 0x04);
C_ASSERT(offsetof(sBriefUserInfo, roomIndex) == 0x08);
C_ASSERT(offsetof(sBriefUserInfo, sNick) == 0x0a);
C_ASSERT(offsetof(sBriefUserInfo, dwIdentity) == 0x21);
C_ASSERT(offsetof(sBriefUserInfo, dwTitle) == 0x25);
C_ASSERT(offsetof(sBriefUserInfo, m_GuildId) == 0x2e);
C_ASSERT(offsetof(sBriefUserInfo, dwEmblemVer983) == 0x32);
C_ASSERT(offsetof(sBriefUserInfo, state) == 0x42);

// 0051f5a0 (645: CLobbyMain::OnRoomList_UserListOwnerDraw)
void FrRoomListDlg::OnUserListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	sBriefUserInfo* pInfo = (sBriefUserInfo*)pItem->pData;
	if (!pInfo)
		return;

	unsigned long color;

	if (pInfo->dwIdentity & 0x14)
	{
		if (Doc()->m_myInfo.info.dwIdentity & 0x14)
		{
			color = 0x50ffffff;
		}
		else
		{
			if (!(pInfo->state & 1))
				return;

			color = 0xffffffff;
		}
	}
	else
	{
		if (IsLocalContent((localContentType_t)0x10))
			color = pInfo->nChannelingFlag == 1 ? 0xff33dbff : 0xffffffff;
		else
			color = 0xffffffff;
	}

	if (pItem->selected)
	{
		if (m_pDoc->m_selOID == ((sBriefUserInfo*)pItem->pData)->dwGuid)
			pDevice->Box(WRect(pItem->pos.x, pItem->pos.y,
							 (float)m_pUserList->GetItemWidth(),
							 (float)m_pUserList->GetItemHeight()),
				0xff265785);
		else
			pItem->selected = false;
	}

	const Bitmap* pIcon;

	if (pInfo->angelicWings)
		pIcon = m_pGenderBmp[pInfo->gender % 2 + 8];
	else if (pInfo->manner)
		pIcon = m_pGenderBmp[pInfo->gender % 2 + 6];
	else
		pIcon = m_pGenderBmp[pInfo->gender];

	WRect rect(pItem->pos.x + 8.0f, pItem->pos.y, (float)pIcon->Width(),
		(float)pIcon->Height());
	pDevice->DrawTexture(pIcon, rect, 0xffffffff, 0);

	if (pInfo->roomIndex != 0xffff)
	{
		pDevice->DrawTexture(m_pInGameBmp,
			WRect(rect.x, rect.y + 3.0f, (float)m_pInGameBmp->Width(),
				(float)m_pInGameBmp->Height()),
			0xffffffff, 0);
	}

	// 983: sem o ícone de ladder do canal 0x80; o anfitrião do Grand Prix (bit 15 da identidade) troca o ícone do nível
	if (pInfo->dwTitle)
	{
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(pInfo->dwTitle);
		if (pSkin)
			pIcon = g_pFresh->GetBitmap(pSkin->c.Icon);
		else
			pIcon = g_pFresh->GetBitmap(MakeStr("level_%03d", pInfo->level + 1));
	}
	else
		pIcon = g_pFresh->GetBitmap(MakeStr("level_%03d", pInfo->level + 1));

	if (pInfo->dwIdentity & 0x8000)
		pIcon = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");

	if (pIcon)
	{
		pDevice->DrawTexture(pIcon,
			WRect(pItem->pos.x + 66.0f - (int)(pIcon->Width() * 0.5f),
				pItem->pos.y + 11.0f - (int)(pIcon->Height() * 0.5f),
				(float)pIcon->Width(), (float)pIcon->Height()),
			0xffffffff, 0);
	}

	if (pInfo->m_GuildId && !(pInfo->dwIdentity & 0x14))
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			pInfo->m_GuildId, pInfo->dwEmblemVer983);

		if (pEmblem)
		{
			pDevice->DrawTexture(pEmblem,
				WRect(pItem->pos.x + 106.0f, pItem->pos.y + 7.0f - 6.0f,
					(float)pEmblem->Width(), (float)pEmblem->Height()),
				0xffffffff, 0);
		}
	}

	if (CProjectG::Instance()->HidePrivacy())
		return;

	if (pItem->underCursor)
	{
		pDevice->SetTextColor(0xffffffff, 0xff124371);
		pDevice->SetTextStyle(2);
	}
	else
	{
		pDevice->SetTextColor(color, 0xffffffff);
		pDevice->SetTextStyle(0);
	}

	g_pFresh->GetManager()->PrintText(
		WPoint(pItem->pos.x + 143.0f, pItem->pos.y + 7.0f), 0, pInfo->sNick,
		(float)m_pUserList->GetItemWidth() - 143.0f, color);
}

// 0051e3d0 (645: CLobbyMain::OnRoomList_UserSortGenderInit)
void FrRoomListDlg::OnUserSortGenderInit(int param)
{
	m_pUserSortGender = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bUserSortDown[0] = false;
}

// 0051d0e0 (645: CLobbyMain::OnRoomList_UserSortGenderUp)
void FrRoomListDlg::OnUserSortGenderUp()
{
	if (m_userSortDelay > 0.0f)
		return;

	if (m_userSort == 0)
		m_bUserSortRev ^= 1;
	else
		m_bUserSortRev = false;

	m_bUserSortDown[0] = true;
	m_bUserSortDown[1] = false;
	m_bUserSortDown[2] = false;
	m_bUserSortDown[3] = false;
	m_userSortDelay = 0.3f;
	SortUserList((userSort_t)0);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 0051e450 (645: CLobbyMain::OnRoomList_UserSortGenderOwnerDraw)
void FrRoomListDlg::OnUserSortGenderOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();

	if (!pDevice)
		return;

	if (!m_pUserSortGender)
		return;

	int state = 0;

	if (m_bUserSortDown[0])
		state = 2;
	else
	{
		WRect rect = m_pUserSortGender->GetRect();
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			state = 1;
	}

	WRect dest = m_pUserSortGender->GetRect();

	pDevice->DrawTexture(m_pUserSortGenderBmp[state], dest, 0xffffffff, 0);

	if (m_userSort == 0)
		pDevice->DrawTexture(m_pUserSortArrowBmp[!m_bUserSortRev],
			WRect(dest.Right(), dest.y + 5.0f, 12.0f, 12.0f), 0xffffffff, 0);
}

// 0051e570 (645: CLobbyMain::OnRoomList_UserSortLevelInit)
void FrRoomListDlg::OnUserSortLevelInit(int param)
{
	m_pUserSortLevel = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bUserSortDown[1] = false;
}

// 0051d160 (645: CLobbyMain::OnRoomList_UserSortLevelUp)
void FrRoomListDlg::OnUserSortLevelUp()
{
	if (m_userSortDelay > 0.0f)
		return;

	if (m_userSort == 1)
		m_bUserSortRev ^= 1;
	else
		m_bUserSortRev = false;

	m_bUserSortDown[0] = false;
	m_bUserSortDown[1] = true;
	m_bUserSortDown[2] = false;
	m_bUserSortDown[3] = false;
	m_userSortDelay = 0.3f;
	SortUserList((userSort_t)1);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 0051e5f0 (645: CLobbyMain::OnRoomList_UserSortLevelOwnerDraw)
void FrRoomListDlg::OnUserSortLevelOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();

	if (!pDevice)
		return;

	if (!m_pUserSortLevel)
		return;

	int state = 0;

	if (m_bUserSortDown[1])
		state = 2;
	else
	{
		WRect rect = m_pUserSortLevel->GetRect();
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			state = 1;
	}

	WRect dest = m_pUserSortLevel->GetRect();

	pDevice->DrawTexture(m_pUserSortLevelBmp[state], dest, 0xffffffff, 0);

	if (m_userSort == 1)
		pDevice->DrawTexture(m_pUserSortArrowBmp[!m_bUserSortRev],
			WRect(dest.Right(), dest.y + 5.0f, 12.0f, 12.0f), 0xffffffff, 0);
}

// 0051e710 (645: CLobbyMain::OnRoomList_UserSortGuildInit)
void FrRoomListDlg::OnUserSortGuildInit(int param)
{
	m_pUserSortGuild = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bUserSortDown[2] = false;
}

// 0051d1e0 (645: CLobbyMain::OnRoomList_UserSortGuildUp)
void FrRoomListDlg::OnUserSortGuildUp()
{
	if (m_userSortDelay > 0.0f)
		return;

	if (m_userSort == 2)
		m_bUserSortRev ^= 1;
	else
		m_bUserSortRev = false;

	m_bUserSortDown[0] = false;
	m_bUserSortDown[1] = false;
	m_bUserSortDown[2] = true;
	m_bUserSortDown[3] = false;
	m_userSortDelay = 0.3f;
	SortUserList((userSort_t)2);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 0051e790 (645: CLobbyMain::OnRoomList_UserSortGuildOwnerDraw)
void FrRoomListDlg::OnUserSortGuildOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();

	if (!pDevice)
		return;

	if (!m_pUserSortGuild)
		return;

	int state = 0;

	if (m_bUserSortDown[2])
		state = 2;
	else
	{
		WRect rect = m_pUserSortGuild->GetRect();
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			state = 1;
	}

	WRect dest = m_pUserSortGuild->GetRect();

	pDevice->DrawTexture(m_pUserSortGuildBmp[state], dest, 0xffffffff, 0);

	if (m_userSort == 2)
		pDevice->DrawTexture(m_pUserSortArrowBmp[!m_bUserSortRev],
			WRect(dest.Right(), dest.y + 5.0f, 12.0f, 12.0f), 0xffffffff, 0);
}

// 0051e8b0 (645: CLobbyMain::OnRoomList_UserSortNickInit)
void FrRoomListDlg::OnUserSortNickInit(int param)
{
	m_pUserSortNick = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_bUserSortDown[3] = false;
}

// 0051d260 (645: CLobbyMain::OnRoomList_UserSortNickUp)
void FrRoomListDlg::OnUserSortNickUp()
{
	if (m_userSortDelay > 0.0f)
		return;

	if (m_userSort == 3)
		m_bUserSortRev ^= 1;
	else
		m_bUserSortRev = false;

	m_bUserSortDown[0] = false;
	m_bUserSortDown[1] = false;
	m_bUserSortDown[2] = false;
	m_bUserSortDown[3] = true;
	m_userSortDelay = 0.3f;
	SortUserList((userSort_t)3);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 0051e930 (645: CLobbyMain::OnRoomList_UserSortNickOwnerDraw)
void FrRoomListDlg::OnUserSortNickOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();

	if (!pDevice)
		return;

	if (!m_pUserSortNick)
		return;

	int state = 0;

	if (m_bUserSortDown[3])
		state = 2;
	else
	{
		WRect rect = m_pUserSortNick->GetRect();
		if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			state = 1;
	}

	WRect dest = m_pUserSortNick->GetRect();

	pDevice->DrawTexture(m_pUserSortNickBmp[state], dest, 0xffffffff, 0);

	if (m_userSort == 3)
		pDevice->DrawTexture(m_pUserSortArrowBmp[!m_bUserSortRev],
			WRect(dest.Right(), dest.y + 5.0f, 12.0f, 12.0f), 0xffffffff, 0);
}

// 0051fa10
void FrRoomListDlg::OnTabGproomInit(int param)
{
	m_pTabGpRoom = DYNAMIC_CAST(FrButton, param);
	ShowTabGpRoom983();
}

// 0051fa90
void FrRoomListDlg::OnTabGproomUp()
{
	if (!(bool)((DOC983_U32(0x10d3c) >> 11) & 1))
	{
		m_pTabGpRoom->Enable(false);
		return;
	}
	Close(FrOK, true);
	DOCJP983B()->SetRoomListRequest983(1);
}

// 0051ea50
void FrRoomListDlg::OnTabMtroomInit(int param)
{
	m_pTabMtRoom = DYNAMIC_CAST(FrButton, param);
}

// 0051fad0
void FrRoomListDlg::OnTabMtroomUp()
{
	Close(FrOK, true);
	DOCJP983B()->SetRoomListRequest983(0);
}

// 0051eac0
void FrRoomListDlg::OnTabAvatarchatInit(int param)
{
	m_pTabAvatarChat = DYNAMIC_CAST(FrButton, param);
	SetTabs983();
}

// 0051d2e0
void FrRoomListDlg::OnTabAvatarchatUp()
{
}

//  (645: CLobbyMain::RoomList_FilterRoomList)
// 983: pula as salas com o bit 0 de sRoomInfo+0xd9 e as dos modos 0x14/0x15 (Grand Prix, byte +0xc4), e a categoria 4 de uma
// sala também entra em qualquer filtro
void FrRoomListDlg::FilterRoomList(roomSort_t sort, int value)
{
	if (m_pRoomList)
	{
		m_pRoomList->ClearItem();

		if (sort == ROOM_SORT_CATE)
		{
			m_pDoc->m_cate = value;
			std::list<sRoomInfo>::iterator it = Doc()->m_roomList.begin();
			for (; it != Doc()->m_roomList.end(); ++it)
			{
				sRoomInfo* pRoom = &(*it);
				if (IS_LISTED_ROOM983(pRoom))
				{
					if (GetCateByGameType(pRoom->gameType) == value || value == 4 ||
						GetCateByGameType(pRoom->gameType) == 4)
						m_pRoomList->AddItem(pRoom);
				}
			}
		}
		else if (sort == ROOM_SORT_GAMETYPE)
		{
			m_pDoc->m_gameType = value;
			std::list<sRoomInfo>::iterator it = Doc()->m_roomList.begin();
			for (; it != Doc()->m_roomList.end(); ++it)
			{
				sRoomInfo* pRoom = &(*it);
				if (IS_LISTED_ROOM983(pRoom))
				{
					if (pRoom->gameType == value)
						m_pRoomList->AddItem(pRoom);
				}
			}
		}

		for (int i = m_pRoomList->GetCurrentItemSize(true); i < 6; i++)
			m_pRoomList->AddItem(NULL);
	}
}

// 0051d3e0 (embutida; cópia isolada no 983): comparador da lista de salas — o item de enchimento (pData nulo) vai sempre para o
// fim, qualquer que seja a ordem; os outros passam pelo comparador do 645 (lobbymain.cpp, 0059f220–0059f8b0)
struct sRoomItemCompare983
{
	sRoomItemCompare983(bool (*compare)(const void*, const void*)) : m_compare(compare) {} // 0051eba0

	bool operator()(const FrListItem* a, const FrListItem* b) const
	{
		if (!a->pData)
			return false;
		if (!b->pData)
			return true;
		return m_compare(a->pData, b->pData);
	}

	bool (*m_compare)(const void*, const void*);
};

// 00521260: no 983 é a `FrListBox::SortItem` com functor (std::list<FrListItem*>::sort 00520f70 + RecalcTopBottom); fica
// aqui como função livre até a FrListBox da A ganhar o molde (pedido B49)
template <class F>
inline void SortItem983(FrListBox* pList, F compare)
{
	pList->m_itemList.sort(compare);
	pList->RecalcTopBottom();
}

// 00521290 (645: CLobbyMain::RoomList_SortRoomList; os mesmos 6 casos, com a queda de GAMETYPE em COURSE)
void FrRoomListDlg::SortRoomList(roomSort_t sort, int unused)
{
	FrListBox* pList = m_pRoomList;
	if (pList)
	{
		m_roomSort = sort;

		switch (sort)
		{
		case ROOM_SORT_NO:
			if (m_bRoomSortRev)
				SortItem983(pList, sRoomItemCompare983(RoomNoRevCompare));
			else
				SortItem983(pList, sRoomItemCompare983(RoomNoCompare));
			break;
		case ROOM_SORT_TITLE:
			if (m_bRoomSortRev)
				SortItem983(pList, sRoomItemCompare983(RoomTitleRevCompare));
			else
				SortItem983(pList, sRoomItemCompare983(RoomTitleCompare));
			break;
		case ROOM_SORT_HOLE:
			if (m_bRoomSortRev)
				SortItem983(pList, sRoomItemCompare983(RoomHoleRevCompare));
			else
				SortItem983(pList, sRoomItemCompare983(RoomHoleCompare));
			break;
		case ROOM_SORT_GAMETYPE:
			if (m_bRoomSortRev)
				SortItem983(pList, sRoomItemCompare983(RoomGameTypeRevCompare));
			else
				SortItem983(pList, sRoomItemCompare983(RoomGameTypeCompare));
		case ROOM_SORT_COURSE:
			if (m_bRoomSortRev)
				SortItem983(m_pRoomList, sRoomItemCompare983(RoomCourseRevCompare));
			else
				SortItem983(m_pRoomList, sRoomItemCompare983(RoomCourseCompare));
			break;
		case ROOM_SORT_STATE:
			if (m_bRoomSortRev)
				SortItem983(pList, sRoomItemCompare983(RoomStateRevCompare));
			else
				SortItem983(pList, sRoomItemCompare983(RoomStateCompare));
			break;
		}
	}
}

//  (645: CLobbyMain::RoomList_SetCateTabBtn)
void FrRoomListDlg::SetCateTabBtn()
{
	int cate = m_pDoc->m_cate;
	m_pRoomSortCateVS->SetStatus(cate == 0 ? FrButton::PRESSED : FrButton::NORMAL);
	m_pRoomSortCateMass->SetStatus(cate == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	m_pRoomSortCateBattle->SetStatus(cate == 2 ? FrButton::PRESSED : FrButton::NORMAL);
	m_pRoomSortCateChat->SetStatus(cate == 3 ? FrButton::PRESSED : FrButton::NORMAL);
	m_pRoomSortCateAll->SetStatus(cate == 4 ? FrButton::PRESSED : FrButton::NORMAL);
}

//  (645: CLobbyMain::RoomList_SortUserList)
void FrRoomListDlg::SortUserList(userSort_t sort)
{
	if (m_pUserList)
	{
		m_userSort = sort;

		switch (sort)
		{
		case USER_SORT_GENDER:
			if (m_bUserSortRev)
				m_pUserList->SortItem(UserGenderRevCompare);
			else
				m_pUserList->SortItem(UserGenderCompare);
			break;
		case USER_SORT_LEVEL:
			if (m_bUserSortRev)
				m_pUserList->SortItem(UserLevelRevCompare);
			else
				m_pUserList->SortItem(UserLevelCompare);
			break;
		case USER_SORT_GUILD:
			if (m_bUserSortRev)
				m_pUserList->SortItem(UserGuildRevCompare);
			else
				m_pUserList->SortItem(UserGuildCompare);
			break;
		case USER_SORT_NICK:
			if (m_bUserSortRev)
				m_pUserList->SortItem(UserNicknameRevCompare);
			else
				m_pUserList->SortItem(UserNicknameCompare);
			break;
		}
	}
}

//  (645: CLobbyMain::SetControlsRoomListSort)
void FrRoomListDlg::SetControlsRoomListSort(int type)
{
	m_bRoomSortDown[0] = false;
	m_bRoomSortDown[1] = false;
	m_bRoomSortDown[2] = false;
	m_bRoomSortDown[3] = false;

	switch (type)
	{
	case 1:
		if (m_pRoomSortCateVSList)
			m_pRoomSortCateVSList->SetVisible(false);
		if (m_pRoomSortCateMassList)
			m_pRoomSortCateMassList->SetVisible(false);
		if (m_pRoomSortCateBattleList)
			m_pRoomSortCateBattleList->SetVisible(false);
		break;
	default:
		if (m_pRoomSortCateVSList)
			m_pRoomSortCateVSList->SetVisible(false);
		if (m_pRoomSortCateMassList)
			m_pRoomSortCateMassList->SetVisible(false);
		if (m_pRoomSortCateBattleList)
			m_pRoomSortCateBattleList->SetVisible(false);
		break;
	}
}

// 0051d510
FrRoomListDlg::FrRoomListDlg()
{
	m_roomSortDelay = 0.0f;
	m_userSortDelay = 0.0f;
	m_cateListTime[0] = 0.0f;
	m_cateListTime[1] = 0.0f;
	m_cateListTime[2] = 0.0f;
	m_pRoom = NULL;
	m_pDtRoomDlg = NULL;
	m_p14c = NULL;
	m_pMakeRoom = NULL;
	m_p290 = NULL;
	m_pUserList = NULL;
	m_pRoomList = NULL;
	m_pSelRoom = NULL;
	m_userSort = USER_SORT_NICK;
	m_bRoomSortRev = false;
	m_bUserSortRev = false;
	m_pRoomList2 = NULL;
	m_154 = 0;
	m_158 = 0;
	m_15c = 0;
	m_pRoomSortCateAll = NULL;
	m_pRoomSortCateVS = NULL;
	m_pRoomSortCateMass = NULL;
	m_pRoomSortCateBattle = NULL;
	m_pRoomSortCateChat = NULL;
	m_pRoomSortCateVSList = NULL;
	m_pRoomSortCateMassList = NULL;
	m_pRoomSortCateBattleList = NULL;
	m_pRoomSortNo = NULL;
	m_pRoomSortHole = NULL;
	m_pRoomSortCourse = NULL;
	m_pRoomSortState = NULL;
	m_pCurEquipItem = NULL;
	m_pQuestBtn = NULL;
	m_pUserInfo = NULL;
	m_pInGameBmp = NULL;
	m_2f4 = 0;
	m_roomSort = ROOM_SORT_NO;
	m_pTabGpRoom = NULL;
	m_pTabMtRoom = NULL;
	m_pTabAvatarChat = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xa5, m_pDoc);
}

// 0051c9f0
FrRoomListDlg::~FrRoomListDlg()
{
}

// 0051ca20
void FrRoomListDlg::OnProc(const float deltaTime)
{
	FrChatDlg::OnProc(deltaTime);
}

class FrWnd983
{
public:
	static void SetF1ab38_983(); // 00be7950 (o mesmo empréstimo do gameroomdlg.cpp)
};

// 0051ed50
bool FrRoomListDlg::OnInit()
{
	FrWnd983::SetF1ab38_983();
	if (CLobbyState983::Instance983())
		AfxPostMsg(NULL, "Lobby", 0x59, 9, 0, 0, 0);
	return FrForm::OnInit();
}

// 0051ed90 (645: CLobbyMain::EnableRoomEnterControls; no 983 entram "cur_equip_item" e "quest_btn", e o controle de +0x290
// fica sempre desligado)
void FrRoomListDlg::EnableRoomEnterControls(bool enable)
{
	if (m_pRoomList)
		m_pRoomList->Enable(enable);
	if (m_pMakeRoom)
		m_pMakeRoom->Enable(enable);
	if (m_pCurEquipItem)
		m_pCurEquipItem->Enable(enable);
	if (m_pQuestBtn)
		m_pQuestBtn->Enable(enable);
	if (m_p290)
		m_p290->Enable(false);
	if (enable)
	{
		if (m_pDtRoomDlg)
			m_pDtRoomDlg->EnableJoinBtn(true);
	}
	if ((Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe)
	{
		if (m_p290)
			m_p290->Enable(false);
	}
}

// 0051ee60: retorno da janela de detalhes da sala
bool FrRoomListDlg::OnDtRoomDlgResult983(int result, FrForm* form)
{
	if (result != 1)
		EnableRoomEnterControls(true);
	m_pDtRoomDlg = NULL;
	return true;
}

// 0051ee90: a aba do Grand Prix aparece com o bit 11 de CSharedDoc+0x10d3c
void FrRoomListDlg::ShowTabGpRoom983()
{
	if (m_pTabGpRoom)
		m_pTabGpRoom->SetVisible(DOCFLAGS983()->b11);
}

// 0051ce80: a aba da conversa só aparece no servidor do tipo 0x10 (00b2eae0); com ela, a das salas some
void FrRoomListDlg::SetTabs983()
{
	if (m_pTabAvatarChat)
	{
		bool bChat = GetServerType983() == 0x10;
		m_pTabAvatarChat->SetVisible(bChat);
		if (m_pTabMtRoom)
		{
			if (bChat)
				m_pTabMtRoom->SetVisible(false);
			else
				m_pTabMtRoom->SetVisible(true);
		}
	}
}

// 0051ce40
void FrRoomListDlg::SetRoom983(CLobbyRoomSelector983* pRoom)
{
	m_pRoom = pRoom;
}

// 0051ce50
void FrRoomListDlg::Set154_983(int a, int b, int c)
{
	m_154 = a;
	m_158 = b;
	m_15c = c;
}

// 0051cee0: desliga o botão da categoria do tipo de jogo
void FrRoomListDlg::DisableCateBtn983(int gameType)
{
	switch (gameType)
	{
	case 2:
		m_pRoomSortCateVS->Enable(false);
		break;
	case 4:
		m_pRoomSortCateMass->Enable(false);
		break;
	case 8:
		m_pRoomSortCateBattle->Enable(false);
		break;
	case 16:
		m_pRoomSortCateChat->Enable(false);
		break;
	}
}

// 0051cf70
void FrRoomListDlg::Room310_983(int a)
{
	m_pRoom->Fn5a0310_983(a, this);
}

// 0051cf90
void FrRoomListDlg::AddUser983(void* pUser)
{
	m_pUserList->AddItem(pUser);
	SortUserList(m_userSort);
}

// 0051cfc0: tira o jogador da lista; se era o escolhido, limpa a escolha e desliga o botão de +0x294
void FrRoomListDlg::DelUser983(sBriefUserInfo* pUser, int index)
{
	if (m_pUserList)
		m_pUserList->DelItem(pUser, index);
	if (pUser->dwGuid == m_pDoc->m_selOID)
	{
		m_pDoc->m_selOID = 0xffffffff;
		m_pDoc->m_selUID = 0xffffffff;
		if (m_pUserInfo)
			m_pUserInfo->Enable(false);
	}
}

// 0051d020
void FrRoomListDlg::ClearUserList983()
{
	if (m_pUserList)
		m_pUserList->ClearItem();
	if (m_pDoc)
	{
		m_pDoc->m_selOID = 0xffffffff;
		m_pDoc->m_selUID = 0xffffffff;
	}
}

// 0051d060
void FrRoomListDlg::Room1f0_983(int a)
{
	if (m_pRoom)
		m_pRoom->Fn5a01f0_983(m_pRoomList, a);
}

// 0051d090
void FrRoomListDlg::Room220_983(int a)
{
	if (m_pRoom)
		m_pRoom->Fn5a0220_983(m_pRoomList, a);
}

// 0051d0c0
void FrRoomListDlg::ClearRoomList983()
{
	if (m_pRoomList)
		m_pRoomList->ClearItem();
}

// 0051d6b0 (645: CLobbyMain::RoomList_LoadRoomImages, 1ª metade; a 2ª é o 0051ca40)
void FrRoomListDlg::LoadRoomImages()
{
	m_pRoomListBoxBmp[0][0] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_n_01");
	m_pRoomListBoxBmp[1][0] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_n_02");
	m_pRoomListBoxBmp[2][0] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_n_03");
	m_pRoomListBoxBmp[0][1] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_o_01");
	m_pRoomListBoxBmp[1][1] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_o_02");
	m_pRoomListBoxBmp[2][1] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_o_03");
	m_pRoomListBoxBmp[0][2] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_n_01"); // 983 0051d6b0: estado 2 = _n; _d nao e carregado
	m_pRoomListBoxBmp[1][2] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_n_02"); // 983 0051d6b0: estado 2 = _n; _d nao e carregado
	m_pRoomListBoxBmp[2][2] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_n_03"); // 983 0051d6b0: estado 2 = _n; _d nao e carregado
	m_pRoomListBoxBmp[0][3] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_e_01");
	m_pRoomListBoxBmp[1][3] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_e_02");
	m_pRoomListBoxBmp[2][3] =
		(Bitmap*)g_pFresh->RegisterBitmap("roomlistbox_e_03");

	m_pRoomSortNoBmp[0] = (Bitmap*)g_pFresh->RegisterBitmap("room_sort_no_n");
	m_pRoomSortNoBmp[1] = (Bitmap*)g_pFresh->RegisterBitmap("room_sort_no_o");
	m_pRoomSortNoBmp[2] = (Bitmap*)g_pFresh->RegisterBitmap("room_sort_no_d");
	m_pRoomSortCateBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_cate_n_down");
	m_pRoomSortCateBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_cate_o");
	m_pRoomSortCateBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_cate_d");
	m_pRoomSortHoleBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_hole_n");
	m_pRoomSortHoleBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_hole_o");
	m_pRoomSortHoleBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_hole_d");
	m_pRoomSortCourseBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_course_n");
	m_pRoomSortCourseBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_course_o");
	m_pRoomSortCourseBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_course_d");
	m_pRoomSortStateUpBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_state_up_n");
	m_pRoomSortStateUpBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_state_up_o");
	m_pRoomSortStateUpBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_state_up_d");
	m_pRoomSortStateDnBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_state_dn_n");
	m_pRoomSortStateDnBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_state_dn_o");
	m_pRoomSortStateDnBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("room_sort_state_dn_d");
	LoadUserImages983();
}

// 0051ca40
void FrRoomListDlg::LoadUserImages983()
{
	m_pGenderBmp[0] = (Bitmap*)g_pFresh->RegisterBitmap("i_male");
	m_pGenderBmp[1] = (Bitmap*)g_pFresh->RegisterBitmap("i_female");
	m_pGenderBmp[2] = (Bitmap*)g_pFresh->RegisterBitmap("i_male_02");
	m_pGenderBmp[3] = (Bitmap*)g_pFresh->RegisterBitmap("i_female_02");
	m_pGenderBmp[4] = (Bitmap*)g_pFresh->RegisterBitmap("i_male_03");
	m_pGenderBmp[5] = (Bitmap*)g_pFresh->RegisterBitmap("i_female_03");

	m_pGenderBmp[6] = (Bitmap*)g_pFresh->RegisterBitmap("i_male_manner");
	m_pGenderBmp[7] = (Bitmap*)g_pFresh->RegisterBitmap("i_female_manner");

	m_pGenderBmp[8] = (Bitmap*)g_pFresh->RegisterBitmap("i_male_angel");
	m_pGenderBmp[9] = (Bitmap*)g_pFresh->RegisterBitmap("i_female_angel");

	m_pUserSortGenderBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_gender_normal");
	m_pUserSortGenderBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_gender_over");
	m_pUserSortGenderBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_gender_click");
	m_pUserSortLevelBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_level_normal");
	m_pUserSortLevelBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_level_over");
	m_pUserSortLevelBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_level_click");

	m_pUserSortGuildBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_guild_normal");
	m_pUserSortGuildBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_guild_over");
	m_pUserSortGuildBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_guild_click");
	m_pUserSortNickBmp[0] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_nick_normal");
	m_pUserSortNickBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_nick_over");
	m_pUserSortNickBmp[2] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_nick_click");

	m_pUserSortArrowBmp[0] = (Bitmap*)g_pFresh->RegisterBitmap("user_sort_up");
	m_pUserSortArrowBmp[1] =
		(Bitmap*)g_pFresh->RegisterBitmap("user_sort_down");

	m_pInGameBmp = (Bitmap*)g_pFresh->RegisterBitmap("ingame");
	m_pAIPlayerBmp = (Bitmap*)g_pFresh->RegisterBitmap("i_aiplayer");
}

// 00521470: completa a lista (0 = "roomlist", 1 = a de +0x2bc) até 6 linhas com linhas vazias, ou tira as vazias que
// sobram acima de 6, e reordena
void FrRoomListDlg::PadRoomList983(int which)
{
	FrListBox* pList = NULL;
	if (which == 0)
		pList = m_pRoomList;
	else if (which == 1)
		pList = m_pRoomList2;
	if (!pList)
		return;

	int count = pList->GetCurrentItemSize(true);
	if (count > 6)
	{
		int extra = count - 6;
		std::list<FrListItem*>::iterator it = pList->m_itemList.begin();
		while (it != pList->m_itemList.end())
		{
			if (extra <= 0)
				break;
			if ((*it)->pData == NULL)
			{
				extra--;
				it = pList->_DelItem(it);
				if (extra < 0)
					break;
			}
			else
				++it;
		}
	}
	else if (count < 6)
	{
		int n = 6 - count;
		do
		{
			pList->AddItem(NULL);
		} while (--n);
	}
	SortRoomList(m_roomSort, 0);
}


// item da segunda lista (+0x2bc, a do Grand Prix): o primeiro campo aponta o registro com o typeid em +4
struct sGpRoomItem983
{
	struct sData
	{
		unsigned long active983; // +0 (Final do IFF_ITEM_COMMON; nao lido aqui)
		unsigned long typeId; // +4
	}* pData;
};

// 0051fe80 (645: caso 0x33 do CLobbyMain::HandleMsg). No 983 o `param1` deixou de ser o erro "sala não existe": 0 = detalhes
// da sala escolhida na lista (como no 645); outro valor = detalhes do Grand Prix (`param2` = typeid, trocado pelo do item
// escolhido na segunda lista quando há um).
void FrRoomListDlg::OpenDtRoomDlg983(const MsgObject& msg)
{
	EnableRoomEnterControls(false);

	if (msg.param1 == 0)
	{
		if (m_pDtRoomDlg || !m_pRoomList || !m_pRoomList->GetSelected())
			return;

		m_pDtRoomDlg = CreateForm<FrDtRoomDlg>(g_pFresh->GetManager(), this, "detailroominfo", NULL);
		m_pDtRoomDlg->SetRoomInfo(*(sRoomInfo*)m_pRoomList->GetSelected()->pData);
	}
	else
	{
		if (m_pDtRoomDlg)
			return;

		unsigned long typeId = msg.param2;
		FrListItem* pSel = m_pRoomList2->GetSelected();
		if (pSel && pSel->pData)
			typeId = ((sGpRoomItem983*)pSel->pData)->pData->typeId;
		// lista de CSharedDoc+0x10a40 (elemento não levantado; o 983 só solta os nós): esvaziada quando o typeid não tem
		// os bits 19–25
		if (typeId != 0 && (typeId & 0x3f80000) == 0)
			((std::list<int>*)DOCPTR983(0x10a40))->clear();

		m_pDtRoomDlg = CreateForm<FrDtRoomDlg>(g_pFresh->GetManager(), this, "detailroominfo", NULL);
		m_pDtRoomDlg->SetGrandPrix983(typeId);
	}

	m_pDtRoomDlg->Open((FRESH_PFN_RESULT)&FrRoomListDlg::OnDtRoomDlgResult983, 1);
}

// 00520900: botões de categoria conforme o tipo do servidor (00b2eae0). Com o bit 7 de CSharedDoc+0x10d3c o tipo é uma
// máscara: cada bit desliga a sua categoria e, com os cinco, some também o "todas"; sem ele o tipo é um valor só.
void FrRoomListDlg::SetCateBtnByServerType983()
{
	int type = GetServerType983();

	if ((bool)((DOC983_U32(0x10d3c) >> 7) & 1))
	{
		m_pRoomSortCateAll->Enable(true);
		m_pRoomSortCateVS->Enable(true);
		m_pRoomSortCateMass->Enable(true);
		m_pRoomSortCateBattle->Enable(true);
		m_pRoomSortCateChat->Enable(true);

		int count = 0;
		char mask[5] = { 1, 2, 4, 8, 16 };
		for (int i = 0; i < 5; i++)
		{
			if (type & mask[i])
			{
				count++;
				DisableCateBtn983(mask[i]);
			}
		}
		if (count == 5)
			m_pRoomSortCateAll->Enable(false);
		return;
	}

	switch (type)
	{
	case 1:
		m_pRoomSortCateAll->Enable(true);
		m_pRoomSortCateVS->Enable(true);
		m_pRoomSortCateMass->Enable(true);
		m_pRoomSortCateBattle->Enable(true);
		m_pRoomSortCateChat->Enable(true);
		OnRoomSortCateAll_BtnDown();
		break;
	case 2:
		m_pRoomSortCateAll->Enable(false);
		m_pRoomSortCateVS->Enable(true);
		m_pRoomSortCateMass->Enable(false);
		m_pRoomSortCateBattle->Enable(false);
		m_pRoomSortCateChat->Enable(false);
		OnRoomSortCateVS_BtnDown();
		break;
	case 4:
		m_pRoomSortCateAll->Enable(false);
		m_pRoomSortCateVS->Enable(false);
		m_pRoomSortCateMass->Enable(true);
		m_pRoomSortCateBattle->Enable(false);
		m_pRoomSortCateChat->Enable(false);
		OnRoomSortCateMass_BtnDown();
		break;
	case 8:
		m_pRoomSortCateAll->Enable(false);
		m_pRoomSortCateVS->Enable(false);
		m_pRoomSortCateMass->Enable(false);
		m_pRoomSortCateBattle->Enable(true);
		m_pRoomSortCateChat->Enable(false);
		OnRoomSortCateBattle_BtnDown();
		break;
	case 16:
		m_pRoomSortCateAll->Enable(false);
		m_pRoomSortCateVS->Enable(false);
		m_pRoomSortCateMass->Enable(false);
		m_pRoomSortCateBattle->Enable(false);
		m_pRoomSortCateChat->Enable(true);
		OnRoomSortCateChat_BtnDown();
		break;
	case 14:
		m_pRoomSortCateAll->Enable(true);
		m_pRoomSortCateVS->Enable(true);
		m_pRoomSortCateMass->Enable(true);
		m_pRoomSortCateBattle->Enable(true);
		m_pRoomSortCateChat->Enable(false);
		OnRoomSortCateAll_BtnDown();
		break;
	}
}

// 00520520 (645: CLobbyMain::ResetControlsRoomListSort). Só muda a condição do bloco das listas de categoria: o 645 olhava
// o layout "ROOMLIST"; o 983 pede CSharedDoc+0x11630 (0062bf50) igual a zero.
void FrRoomListDlg::ResetControlsRoomListSort(float elapsed)
{
	if (m_roomSortDelay > 0.0f)
	{
		m_roomSortDelay -= elapsed;

		if (m_roomSortDelay <= 0.0f)
		{
			m_bRoomSortDown[0] = false;
			m_bRoomSortDown[1] = false;
			m_bRoomSortDown[2] = false;
			m_bRoomSortDown[3] = false;
		}
	}

	if (m_userSortDelay > 0.0f)
	{
		m_userSortDelay -= elapsed;

		if (m_userSortDelay <= 0.0f)
		{
			m_bUserSortDown[0] = false;
			m_bUserSortDown[1] = false;
			m_bUserSortDown[2] = false;
			m_bUserSortDown[3] = false;
		}
	}

	if (DOCJP()->GetRoomListKind983() == 0)
	{
		if (g_input->GetButton(LEFT_BUTTON) == 3)
		{
			WVector mousePos = g_input->GetMousePoint();
			WVector downPos = g_input->GetButtonDownPos(LEFT_BUTTON);

			WRect vsRect = m_pRoomSortCateVS->GetRect(), vsListRect = m_pRoomSortCateVSList->GetRect();
			WRect massRect = m_pRoomSortCateMass->GetRect(), massListRect = m_pRoomSortCateMassList->GetRect();
			WRect battleRect = m_pRoomSortCateBattle->GetRect(), battleListRect = m_pRoomSortCateBattleList->GetRect();

			if (m_pRoomSortCateVS->GetStatus() == FrButton::PRESSED && vsRect.IsInRect(WPoint(downPos.x, downPos.y)) &&
				(vsRect.IsInRect(WPoint(mousePos.x, mousePos.y)) || vsListRect.IsInRect(WPoint(mousePos.x, mousePos.y))))
			{
				m_cateListTime[0] += elapsed;

				if (m_cateListTime[0] > 0.8f)
					m_pRoomSortCateVSList->SetVisible(true);
			}
			else
			{
				m_cateListTime[0] = 0.0f;
				m_pRoomSortCateVSList->SetVisible(false);
			}

			if (m_pRoomSortCateMass->GetStatus() == FrButton::PRESSED && massRect.IsInRect(WPoint(downPos.x, downPos.y)) &&
				(massRect.IsInRect(WPoint(mousePos.x, mousePos.y)) || massListRect.IsInRect(WPoint(mousePos.x, mousePos.y))))
			{
				m_cateListTime[1] += elapsed;

				if (m_cateListTime[1] > 0.8f)
					m_pRoomSortCateMassList->SetVisible(true);
			}
			else
			{
				m_cateListTime[1] = 0.0f;
				m_pRoomSortCateMassList->SetVisible(false);
			}

			if (m_pRoomSortCateBattle->GetStatus() == FrButton::PRESSED && battleRect.IsInRect(WPoint(downPos.x, downPos.y)) &&
				(battleRect.IsInRect(WPoint(mousePos.x, mousePos.y)) || battleListRect.IsInRect(WPoint(mousePos.x, mousePos.y))))
			{
				m_cateListTime[2] += elapsed;

				if (m_cateListTime[2] > 0.8f)
					m_pRoomSortCateBattleList->SetVisible(true);
			}
			else
			{
				m_cateListTime[2] = 0.0f;
				m_pRoomSortCateBattleList->SetVisible(false);
			}
		}
		else
		{
			if (m_pRoomSortCateVSList->IsVisible())
				OnRoomSortCateVS_ListLUp();

			if (m_pRoomSortCateMassList->IsVisible())
				OnRoomSortCateMass_ListLUp();

			if (m_pRoomSortCateBattleList->IsVisible())
				OnRoomSortCateBattle_ListLUp();

			m_cateListTime[0] = 0.0f;
			m_cateListTime[1] = 0.0f;
			m_cateListTime[2] = 0.0f;
		}
	}
}
