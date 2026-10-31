#include "minatl.h"
#include "vc10_layout.h"
#include <algorithm>
#include "makeroomdlg.h"
#include "s5/utilities.h" // E6: NotifyServiceStopped983
#include "jp/shareddocjp.h" // E35: IsControlServerService983
#include "frbutton.h"
#include "fredit.h"
#include "frarea.h"
#include "frviewer.h"
#define REBANG_DEFER_SETSEPWIDTH
#include "frtabbutton.h"
#include "frgaugebar.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "packet.h"
#include "treasure.h"
#include "treasuredlg.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "commonutil.h"
#include "s5/utilities.h"
#include "artifactcheck983.h"

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
unsigned char PickRandomCourse983(); // lobbymain.cpp (983 00641650)

// unused, but the original emitted g_HUGE/g_EPSILON into .rdata
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

extern Fresh* g_pFresh;

int IsMassGame(unsigned char gameType);
unsigned char FirstMap();
unsigned char LastMap(bool bChat);
unsigned char PrevMap(unsigned char map);
unsigned char NextMap(unsigned char map, bool bChat);

IMPLEMENT_OBJECT(FrMakeRoomDlg, FrForm)

// 983 00cef2e0: rotulo do botao de cada tipo de jogo (00f03280, 23 entradas; 20..22 nulas)
static const char* s_gameTypeTitle[23] = {
	K2L_Compatibility("\xbd\xba\xc6\xae\xb7\xce\xc5\xa9"), K2L_Compatibility("\xb8\xc5\xc4\xa1"), K2L_Compatibility("\xc3\xa4\xc6\xc3\xb9\xe6"), K2L_Compatibility("\xb7\xa1\xb4\xf5"),
	K2L_Compatibility("\xb4\xeb\xc8\xb8"), K2L_Compatibility("\xbc\xf4\xb0\xd4\xc0\xd3"), K2L_Compatibility("\xb1\xe6\xb5\xe5\xb4\xeb\xc0\xfc"), K2L_Compatibility("\xc6\xce\xb9\xe8\xc6\xb2"), "", "",
	K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"), K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xc0\xcf\xb9\xdd"), K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xc0\xcf\xb9\xdd\xbd\xcc\xb1\xdb"), K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xbb\xf3\xb1\xde"), K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xbb\xf3\xb1\xde\xbd\xcc\xb1\xdb"),
	"", "", "", K2L_Compatibility("\xbc\xc5\xc7\xc3\xc4\xda\xbd\xba"), K2L_Compatibility("\xbd\xcc\xb1\xdb\xc7\xc3\xb7\xb9\xc0\xcc"), NULL, NULL, NULL,
};

BEGIN_FRESH_MSGMAP(FrMakeRoomDlg, FrForm)

ON_FRESH_VI("make_vs", FRCMD_INIT, FrMakeRoomDlg::OnMakeVsInit)
ON_FRESH_VV("make_vs", FRCMD_LBUTTONDOWN, FrMakeRoomDlg::OnMakeVsDown)
ON_FRESH_VI("make_mass", FRCMD_INIT, FrMakeRoomDlg::OnMakeMassInit)
ON_FRESH_VV("make_mass", FRCMD_LBUTTONDOWN, FrMakeRoomDlg::OnMakeMassDown)
ON_FRESH_VI("make_battle", FRCMD_INIT, FrMakeRoomDlg::OnMakeBattleInit)
ON_FRESH_VV("make_battle", FRCMD_LBUTTONDOWN, FrMakeRoomDlg::OnMakeBattleDown)
ON_FRESH_VI("make_chat", FRCMD_INIT, FrMakeRoomDlg::OnMakeChatInit)
ON_FRESH_VV("make_chat", FRCMD_LBUTTONDOWN, FrMakeRoomDlg::OnMakeChatDown)
ON_FRESH_VI("make_chaos", FRCMD_INIT, FrMakeRoomDlg::OnMakeChaosInit)
ON_FRESH_VV("make_chaos", FRCMD_LBUTTONDOWN, FrMakeRoomDlg::OnMakeChaosDown)
ON_FRESH_VI("make_event", FRCMD_INIT, FrMakeRoomDlg::OnMakeEventInit)
ON_FRESH_VV("make_event", FRCMD_LBUTTONDOWN, FrMakeRoomDlg::OnMakeEventDown)

ON_FRESH_VI("picture", FRCMD_INIT, FrMakeRoomDlg::OnPictureInit)
ON_FRESH_VI("text", FRCMD_INIT, FrMakeRoomDlg::OnTextInit)
ON_FRESH_VI("title", FRCMD_INIT, FrMakeRoomDlg::OnTitleInit)
ON_FRESH_BI("title", FRCMD_ENTERKEY, FrMakeRoomDlg::OnTitleEnterKey)
ON_FRESH_VI("password", FRCMD_INIT, FrMakeRoomDlg::OnPasswordInit)

ON_FRESH_VI("map_ctrl.mapprev", FRCMD_INIT, FrMakeRoomDlg::OnMapPrevInit)
ON_FRESH_VV("map_ctrl.mapprev", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnMapPrevUp)
ON_FRESH_VI("map_ctrl.mapnext", FRCMD_INIT, FrMakeRoomDlg::OnMapNextInit)
ON_FRESH_VV("map_ctrl.mapnext", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnMapNextUp)
ON_FRESH_VI("mapprev_chat", FRCMD_INIT, FrMakeRoomDlg::OnMapPrevChatInit)
ON_FRESH_VV("mapprev_chat", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnMapPrevUp)
ON_FRESH_VI("mapnext_chat", FRCMD_INIT, FrMakeRoomDlg::OnMapNextChatInit)
ON_FRESH_VV("mapnext_chat", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnMapNextUp)

ON_FRESH_VI("gametype", FRCMD_INIT, FrMakeRoomDlg::OnGameTypeInit)
ON_FRESH_VV("gametype", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnGameTypeButtonUp)
ON_FRESH_VI("gametype", FRCMD_OWNERDRAW, FrMakeRoomDlg::OnGameTypeOwnerDraw)
ON_FRESH_VI("userlimit", FRCMD_INIT, FrMakeRoomDlg::OnUserLimitInit)
ON_FRESH_VV("userlimit", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnUserLimitButtonUp)
ON_FRESH_VI("userlimit", FRCMD_OWNERDRAW, FrMakeRoomDlg::OnUserLimitOwnerDraw)
ON_FRESH_VI("hole", FRCMD_INIT, FrMakeRoomDlg::OnHoleInit)
ON_FRESH_VV("hole", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnHoleButtonUp)
ON_FRESH_VI("hole", FRCMD_OWNERDRAW, FrMakeRoomDlg::OnHoleOwnerDraw)
ON_FRESH_VI("timelimit", FRCMD_INIT, FrMakeRoomDlg::OnTimeLimitInit)
ON_FRESH_VV("timelimit", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnTimeLimitButtonUp)
ON_FRESH_VI("timelimit", FRCMD_OWNERDRAW, FrMakeRoomDlg::OnTimeLimitOwnerDraw)
ON_FRESH_VI("map_ctrl.course", FRCMD_INIT, FrMakeRoomDlg::OnCourseInit)
ON_FRESH_VV("map_ctrl.course", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnCourseUp)
ON_FRESH_VI("map_ctrl.course", FRCMD_OWNERDRAW, FrMakeRoomDlg::OnCourseOwnerDraw)
ON_FRESH_VI("map_ctrl.holetype", FRCMD_INIT, FrMakeRoomDlg::OnHoleTypeInit)
ON_FRESH_VI("map_ctrl.holetype", FRCMD_OWNERDRAW, FrMakeRoomDlg::OnHoleTypeOwnerDraw)

ON_FRESH_VI("course_chat", FRCMD_INIT, FrMakeRoomDlg::OnCourseChatInit)
ON_FRESH_VV("course_chat", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnCourseChatUp)
ON_FRESH_VI("hole_min", FRCMD_INIT, FrMakeRoomDlg::OnHoleMinInit)
ON_FRESH_VV("hole_min", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnHoleMinUp)
ON_FRESH_VI("hole_prev", FRCMD_INIT, FrMakeRoomDlg::OnHolePrevInit)
ON_FRESH_VV("hole_prev", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnHolePrevUp)
ON_FRESH_VI("hole_chat", FRCMD_INIT, FrMakeRoomDlg::OnHoleChatInit)
ON_FRESH_VI("hole_chat", FRCMD_OWNERDRAW, FrMakeRoomDlg::OnHoleChatOwnerDraw)
ON_FRESH_VI("hole_next", FRCMD_INIT, FrMakeRoomDlg::OnHoleNextInit)
ON_FRESH_VV("hole_next", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnHoleNextUp)
ON_FRESH_VI("hole_max", FRCMD_INIT, FrMakeRoomDlg::OnHoleMaxInit)
ON_FRESH_VV("hole_max", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnHoleMaxUp)

ON_FRESH_VI("map_ctrl", FRCMD_INIT, FrMakeRoomDlg::OnMapCtrlInit)
ON_FRESH_VI("map_ctrl.text_map", FRCMD_INIT, FrMakeRoomDlg::OnMapCtrlTextInit)
ON_FRESH_VI("artifact_ctrl", FRCMD_INIT, FrMakeRoomDlg::OnArtifactCtrlInit)
ON_FRESH_VI("artifact_ctrl.arti_frame", FRCMD_INIT, FrMakeRoomDlg::OnArtiFrameInit)
ON_FRESH_VI("artifact_ctrl.arti_icon", FRCMD_INIT, FrMakeRoomDlg::OnArtiIconInit)
ON_FRESH_VI("artifact_ctrl.arti_ban", FRCMD_INIT, FrMakeRoomDlg::OnArtiBanInit)
ON_FRESH_VI("artifact_ctrl.arti_name", FRCMD_INIT, FrMakeRoomDlg::OnArtiNameInit)
ON_FRESH_VI("artifact_ctrl.arti_desc", FRCMD_INIT, FrMakeRoomDlg::OnArtiDescInit)
ON_FRESH_VI("artifact_ctrl.arti_prev", FRCMD_INIT, FrMakeRoomDlg::OnArtiPrevInit)
ON_FRESH_VV("artifact_ctrl.arti_prev", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnArtiPrevUp)
ON_FRESH_VI("artifact_ctrl.arti_next", FRCMD_INIT, FrMakeRoomDlg::OnArtiNextInit)
ON_FRESH_VV("artifact_ctrl.arti_next", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnArtiNextUp)
ON_FRESH_VI("map_ctrl.overgauge", FRCMD_INIT, FrMakeRoomDlg::OnOverGauge_Init)
ON_FRESH_VI("classictype_btn", FRCMD_INIT, FrMakeRoomDlg::OnClassicTypeBtnInit)
ON_FRESH_VV("classictype_btn", FRCMD_LBUTTONUP, FrMakeRoomDlg::OnClassicTypeBtnUp)

END_FRESH_MSGMAP()

FrMakeRoomDlg::FrMakeRoomDlg()
	: m_artifact983(&Doc()->m_myItemList) // 983 00adead0: o CItemCursor983 parte da lista do CSharedDoc+0x375c
{
	// A216: layout do 983 (00adead0 ctor, 00aded00/00adea20/00ade540 Init dos controles, 00ae2203/00ae2238 pacote 0x08)
	C_ASSERT_VC10(sizeof(FrMakeRoomDlg) == 0x210);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pMakeBtn) == 0x110);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pClassicTypeBtn) == 0x15c);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pMapCtrl) == 0x180);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pMapCtrlText) == 0x184);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pArtifactCtrl) == 0x188);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pArtiNext) == 0x1a4);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_artifact983) == 0x1fc);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_artifact983) + offsetof(CItemCursor983, m_pCur) == 0x1fc + 0xc);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_classicFlags) == 0x20c);
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pad983_138) == 0x138); // A847: 983 0 acessos
	C_ASSERT_VC10(offsetof(FrMakeRoomDlg, m_pad983_1d0) == 0x1d0);
	C_ASSERT(offsetof(sArtifactNode983, value) + offsetof(sItemInfo, tid) == 0xc); // 00479e90 GetCur: [cursor]+0xc
	memset(m_pMakeBtn, 0, sizeof(m_pMakeBtn));
	m_btnDelay = 0.7f;
	m_gameType = (Doc()->m_curChannel.Type & 0x4000) ? 7 : 0; // 983: sem o ramo 0x80 -> 3 do 645
	m_pMapPrev = NULL;
	m_pMapNext = NULL;
	m_pMapPrevChat = NULL;
	m_pMapNextChat = NULL;
	m_pMapSelectDlg = NULL;
	m_pMapCtrl = NULL;
	m_pMapCtrlText = NULL;
	m_pArtifactCtrl = NULL;
	m_pArtiFrame = NULL;
	m_pArtiIcon = NULL;
	m_pArtiBan = NULL;
	m_pArtiName = NULL;
	m_pArtiDesc = NULL;
	m_pArtiPrev = NULL;
	m_pArtiNext = NULL;
	m_pClassicTypeBtn = NULL;
	SetClassicFlags983(0);
}

// 983 00adcfb0: bit0 do flag -> status (0/2) do classictype_btn
__declspec(noinline) void FrMakeRoomDlg::ApplyClassicFlags983()
{
	if (m_pClassicTypeBtn)
		m_pClassicTypeBtn->SetStatus((FrButton::eButMode)((m_classicFlags & 1) * 2));
}

// 983 00add000
__declspec(noinline) void FrMakeRoomDlg::SetClassicFlags983(unsigned long flags)
{
	m_classicFlags = flags;
	ApplyClassicFlags983();
}

// 983 00adea20: guarda o botao em 0x15c e aplica o flag 0x20c direto (00adcfb0, sem regravar)
void FrMakeRoomDlg::OnClassicTypeBtnInit(int param)
{
	m_pClassicTypeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pClassicTypeBtn)
		ApplyClassicFlags983();
}

// 983 00adeaa0: status 2 do botao liga o bit0 do flag, senao desliga
void FrMakeRoomDlg::OnClassicTypeBtnUp()
{
	if (m_pClassicTypeBtn)
	{
		unsigned long fl = m_classicFlags;
		if (m_pClassicTypeBtn->GetStatus() == 2)
			SetClassicFlags983(fl | 1);
		else
			SetClassicFlags983(fl & ~1UL);
	}
}

FrMakeRoomDlg::~FrMakeRoomDlg()
{
}

static float g_mapCtrlOfs983[2]; // 983 00f03268/00f0326c (WPoint)
static float s_artiNoChatOfs983[2] = { 65.0f, -16.0f }; // 983 00e397e0/e4 (WPoint)
// A423: os botoes/controles sao do wangreal.dll original; os campos usados aqui no layout do 983
struct FrButtonLayout983 : FrButton { static void Check() {
	C_ASSERT_VC10(offsetof(FrButtonLayout983, m_rect) == 0x34); C_ASSERT_VC10(offsetof(FrButtonLayout983, m_dwStyle) == 0x44);
	C_ASSERT_VC10(offsetof(FrButtonLayout983, m_pushDelay) == 0x104); C_ASSERT_VC10(offsetof(FrButtonLayout983, m_pushedTime) == 0x108); } };
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.stat.Level) == 0x91d); // 983 00ae1ca6
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.holeType) == 0x1094f); // 983 00adf6d5
C_ASSERT_VC10(offsetof(CSharedDoc, m_myItemList) == 0x375c); // 983 00adead0
C_ASSERT_VC10(offsetof(CSharedDoc, m_serverProperty) == 0x10d3c); // 983 bit 7 = tipo do servidor como máscara
C_ASSERT(sizeof(IFF_ITEM_COMMON) == 0xc0); // sItem::Artifact = bits 1-2 do dword +0xc0


// 983 00b2eae0: mascara de tipos de sala liberados (1 vs, 2 chat, 4 mass, 8 battle, 0xe..) pelo servico de controle do servidor
// (CSharedDocJp::IsControlServerService983, 64 bits): bits 6..13 de cada tipo
static unsigned long OpenTypeMask983()
{
	unsigned char f[8] = { 0 };
	for (int i = 0; i < 8; ++i)
	{
		if (DOCJP()->IsControlServerService983(1 << (i + 6)))
			f[i] = 1;
		else
			f[i] = 0;
	}
	unsigned long m = 0;
	for (int k = 0; k < 8; ++k)
		m |= (f[k] == 1) ? (0x40 << k) : 0;
	if ((unsigned char)(Doc()->m_serverProperty >> 7) & 1)
	{
		unsigned long r = 0;
		if (f[0] == 1) r = 2;
		if (f[1] == 1) r |= 2;
		if (f[2] == 1) r |= 4;
		if (f[3] == 1) r |= 4;
		if (f[4] == 1) r |= 4;
		if (f[5] == 1) r |= 8;
		if (f[6] == 1) r |= 8;
		if (f[7] == 1) r |= 0x10;
		return r;
	}
	if (m == 0)
		return 1;
	if (DOCJP()->IsControlServerService983(0x1fc0) == 0 && DOCJP()->IsControlServerService983(0x2000) == 0x2000)
		return 0xe;
	if (!DOCJP()->IsControlServerService983(0xc0))
		return 2;
	if (!DOCJP()->IsControlServerService983(0x700))
		return 4;
	if (!DOCJP()->IsControlServerService983(0x1800))
		return 8;
	if (!DOCJP()->IsControlServerService983(0x2000))
		return 0x10;
	return 0;
}

// 983 00adcf20: bit do tipo (2 vs, 4 mass, 8 battle, 0x10 chat) -> desabilita o botao m_pMakeBtn[0..3]
void FrMakeRoomDlg::DisableTypeBtn983(unsigned long bit)
{
	switch (bit)
	{
	case 2:
		m_pMakeBtn[0]->Enable(false);
		break;
	case 4:
		m_pMakeBtn[1]->Enable(false);
		break;
	case 8:
		m_pMakeBtn[2]->Enable(false);
		break;
	case 0x10:
		m_pMakeBtn[3]->Enable(false);
		break;
	}
}

// 983 00ae2460: mascara -> habilita os 5 botoes de tipo (m_pMakeBtn[0..4], sem teste de NULL), chama o tratador do botao
// (00ae1860 Vs / 00ae1890 Mass / 00ae18c0 Battle / 00ae18e0 Chat) e deixa o botao apertado
void FrMakeRoomDlg::ApplyTypeMask983(unsigned long mask)
{
	switch (mask)
	{
	case 1:
		m_pMakeBtn[0]->Enable(true);
		m_pMakeBtn[1]->Enable(true);
		m_pMakeBtn[2]->Enable(true);
		m_pMakeBtn[3]->Enable(true);
		m_pMakeBtn[4]->Enable(true);
		OnMakeVsDown();
		m_pMakeBtn[0]->SetStatus(FrButton::PRESSED);
		break;
	case 2:
		m_pMakeBtn[0]->Enable(true);
		m_pMakeBtn[1]->Enable(false);
		m_pMakeBtn[2]->Enable(false);
		m_pMakeBtn[3]->Enable(false);
		m_pMakeBtn[4]->Enable(false);
		OnMakeVsDown();
		m_pMakeBtn[0]->SetStatus(FrButton::PRESSED);
		break;
	case 4:
		m_pMakeBtn[0]->Enable(false);
		m_pMakeBtn[1]->Enable(true);
		m_pMakeBtn[2]->Enable(false);
		m_pMakeBtn[3]->Enable(false);
		m_pMakeBtn[4]->Enable(true);
		OnMakeMassDown();
		m_pMakeBtn[1]->SetStatus(FrButton::PRESSED);
		break;
	case 8:
		m_pMakeBtn[0]->Enable(false);
		m_pMakeBtn[1]->Enable(false);
		m_pMakeBtn[2]->Enable(true);
		m_pMakeBtn[3]->Enable(false);
		m_pMakeBtn[4]->Enable(false);
		OnMakeBattleDown();
		m_pMakeBtn[2]->SetStatus(FrButton::PRESSED);
		break;
	case 0x10:
		m_pMakeBtn[0]->Enable(false);
		m_pMakeBtn[1]->Enable(false);
		m_pMakeBtn[2]->Enable(false);
		m_pMakeBtn[3]->Enable(true);
		m_pMakeBtn[4]->Enable(false);
		OnMakeChatDown();
		m_pMakeBtn[3]->SetStatus(FrButton::PRESSED);
		break;
	case 0xe:
		m_pMakeBtn[0]->Enable(true);
		m_pMakeBtn[1]->Enable(true);
		m_pMakeBtn[2]->Enable(true);
		m_pMakeBtn[3]->Enable(false);
		m_pMakeBtn[4]->Enable(true);
		OnMakeVsDown();
		m_pMakeBtn[0]->SetStatus(FrButton::PRESSED);
		break;
	default:
		break;
	}
}

// 983 00adfa70: mostra/esconde a area do artefato (map_ctrl desloca para a posicao do dialogo + offset)
void FrMakeRoomDlg::SetArtifactArea983(bool bShow, bool bChat)
{
	if (m_pMapCtrl)
	{
		m_pMapCtrl->SetVisible(bShow);
		if (bShow)
		{
			// 983: deslocamento + canto do dialogo; sem chat soma o WPoint 00e397e0 (65, -16)
			WPoint base(GetRect().x, GetRect().y);
			WPoint pt = *(WPoint*)g_mapCtrlOfs983 + base;
			if (!bChat)
			{
				pt.x = s_artiNoChatOfs983[0] + pt.x;
				pt.y = s_artiNoChatOfs983[1] + pt.y;
			}
			m_pMapCtrl->MoveWindow(pt);
		}
	}
	if (m_pArtifactCtrl)
		m_pArtifactCtrl->SetVisible(bChat);
	if (!bChat)
		RefreshArtiButtons983();
}

// 983 00adf9e0: cursor no item 0, ajusta prev/next e redesenha
void FrMakeRoomDlg::RefreshArtiButtons983()
{
	unsigned long zero = 0;
	m_artifact983.Find(&zero);
	if (m_pArtiPrev)
		m_pArtiPrev->Enable(m_artifact983.CanPrev983());
	if (m_pArtiNext)
		m_pArtiNext->Enable(m_artifact983.CanNext983());
	RefreshArtifact983();
}

bool FrMakeRoomDlg::OnInit()
{
	m_holeType = 0;
	m_chatHole = 1;
	m_gameTime = 0;
	m_shotTime = 0;
	SetClassicFlags983(0);
	m_courseX = m_pCourse->GetRect().x;
	m_pMapSelectO =
		g_pFresh->GetManager()->GetBitmap("MAKEROOM", "map_select_o");

	ChangeCourse(PickRandomCourse983()); // 983 00ae26c0: campo sorteado (641650) no lugar de FirstMap; antes do ChangeGameType
	unsigned long mask = OpenTypeMask983();
	if ((unsigned char)(Doc()->m_serverProperty >> 7) & 1)
	{
		// 983 00ae2760: modo 0x80 so desabilita os botoes dos tipos nao liberados (00adcf20)
		signed char bits[5] = { 1, 2, 4, 8, 0x10 };
		for (int i = 0; i < 5; ++i)
		{
			if (mask & bits[i])
				DisableTypeBtn983(bits[i]);
		}
	}
	else
		ApplyTypeMask983(mask);
	ChangeGameType(m_gameType, true);

	if (m_pMakeBtn[m_gameTypeGroup])
		m_pMakeBtn[m_gameTypeGroup]->SetStatus(FrButton::PRESSED);
	RefreshArtiButtons983();

	return true;
}

void FrMakeRoomDlg::OnMakeVsInit(int param)
{
	m_pMakeBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 00ae1860: tipo 0 + area do artefato (1,1) + UpdateArtifact983(1,0)
void FrMakeRoomDlg::OnMakeVsDown()
{
	ChangeGameType(0, true);
	SetArtifactArea983(true, true);
	UpdateArtifact983(true, 0);
}

void FrMakeRoomDlg::OnMakeMassInit(int param)
{
	m_pMakeBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 00ae1890
void FrMakeRoomDlg::OnMakeMassDown()
{
	ChangeGameType(4, true);
	SetArtifactArea983(true, true);
	UpdateArtifact983(true, 0);
}

void FrMakeRoomDlg::OnMakeBattleInit(int param)
{
	m_pMakeBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 00ae18c0
void FrMakeRoomDlg::OnMakeBattleDown()
{
	ChangeGameType(10, true);
	SetArtifactArea983(true, false);
}

void FrMakeRoomDlg::OnMakeChatInit(int param)
{
	m_pMakeBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 00ae18e0
void FrMakeRoomDlg::OnMakeChatDown()
{
	ChangeGameType(2, true);
	SetArtifactArea983(false, false);
}

void FrMakeRoomDlg::OnMakeChaosInit(int param)
{
	m_pMakeBtn[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMakeBtn[4])
	{
		bool bFind = false;
		std::list<sItemInfo>::iterator it;
		for (it = CSharedDoc::Instance()->m_myItemList.begin();
			it != CSharedDoc::Instance()->m_myItemList.end(); ++it)
		{
			if ((*it).tid == 0x1a0000f7)
			{
				bFind = true;
				break;
			}
		}
		if (!bFind)
			m_pMakeBtn[4]->SetVisible(false);
		m_pMakeBtn[4]->EnableBlink(true);
	}
}

void FrMakeRoomDlg::OnMakeChaosDown()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
		(int)MakeStr(
			K2L_Compatibility("\xbd\xba\xc6\xf3\xbc\xc8 \xbc\xc5\xc7\xc3 \xb8\xf0\xb5\xe5 \xb9\xe6\xc0\xbb \xb8\xb8\xb5\xe9 \xb6\xa7, \n\\c0xffff0000\\c%s\\c0xff000000\\c\xc0\xcc \xbc\xd2\xb8\xf0 \xb5\xcb\xb4\xcf\xb4\xd9."),
			ItemManager()->GetItemName(0x1a0000f7)),
		0, 0, 0, 0));

	ChangeGameType(0x12, true); // 983 00ae1900
	SetArtifactArea983(false, false);
}

// 983 00aded00: so aparece com o bit 4 da identidade
void FrMakeRoomDlg::OnMakeEventInit(int param)
{
	m_pMakeBtn[5] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	bool on = (CSharedDoc::Instance()->m_myInfo.info.dwIdentity & 4) != 0;
	m_pMakeBtn[5]->Enable(on);
	m_pMakeBtn[5]->SetVisible(on);
}

// 983 00ae19a0
void FrMakeRoomDlg::OnMakeEventDown()
{
	ChangeGameType(0xb, true);
	SetArtifactArea983(true, false);
}

void FrMakeRoomDlg::OnPictureInit(int param)
{
	m_pPicture = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pPicture)
		m_pPicture->SetMouseEvent(false);
}

void FrMakeRoomDlg::OnTextInit(int param)
{
	m_pText = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}


void FrMakeRoomDlg::OnMapCtrlInit(int param)
{
	// 983 00ade540: FrStatic em +0x180; guarda o deslocamento do controle em relacao ao dialogo (globais 00f03268/6c)
	m_pMapCtrl = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
	if (m_pMapCtrl)
	{
		WPoint a(GetRect().x, GetRect().y); // 983: FrWnd+0x34/+0x38 = m_rect.x/y
		WPoint p(m_pMapCtrl->GetRect().x, m_pMapCtrl->GetRect().y);
		WPoint d(p.x - a.x, p.y - a.y);
		*(unsigned long*)&g_mapCtrlOfs983[0] = *(unsigned long*)&d.x;
		*(unsigned long*)&g_mapCtrlOfs983[1] = *(unsigned long*)&d.y;
	}
}

void FrMakeRoomDlg::OnMapCtrlTextInit(int param)
{
	m_pMapCtrlText = DYNAMIC_CAST(FrArea, (FrWnd*)param); // 983 00ade5f0 (+0x184)
}

void FrMakeRoomDlg::OnArtifactCtrlInit(int param)
{
	m_pArtifactCtrl = DYNAMIC_CAST(FrWnd, (FrWnd*)param); // 983 00ade660 (+0x188)
}

// 983 00ade6d0..00ade890: os controles do artefato guardam o ponteiro (dynamic_cast) em +0x18c..+0x19c
void FrMakeRoomDlg::OnArtiFrameInit(int param)
{
	m_pArtiFrame = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrMakeRoomDlg::OnArtiIconInit(int param)
{
	m_pArtiIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrMakeRoomDlg::OnArtiBanInit(int param)
{
	m_pArtiBan = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrMakeRoomDlg::OnArtiNameInit(int param)
{
	m_pArtiName = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrMakeRoomDlg::OnArtiDescInit(int param)
{
	m_pArtiDesc = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

// 983 00ade900 / 00ade990: botoes anterior/proximo; sem duplo clique, atraso de 0.1 (+0x104/+0x108)
void FrMakeRoomDlg::OnArtiPrevInit(int param)
{
	m_pArtiPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pArtiPrev)
	{
		static_cast<FrButton*>(m_pArtiPrev)->SetPushDelay(0.0f); // 983: +0x104 = 0, +0x108 = 0.1
		m_pArtiPrev->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrMakeRoomDlg::OnArtiNextInit(int param)
{
	m_pArtiNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pArtiNext)
	{
		static_cast<FrButton*>(m_pArtiNext)->SetPushDelay(0.0f); // 983: +0x104 = 0, +0x108 = 0.1
		m_pArtiNext->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

// 983 00adf920 / 00adf980: move o cursor do artefato, ajusta os dois botoes e redesenha (00adf400)
void FrMakeRoomDlg::OnArtiPrevUp()
{
	if (m_pArtiPrev)
	{
		m_pArtiPrev->Enable(m_artifact983.StepPrev983());
		m_pArtiNext->Enable(m_artifact983.CanNext983());
		RefreshArtifact983();
	}
}

void FrMakeRoomDlg::OnArtiNextUp()
{
	if (m_pArtiNext)
	{
		m_pArtiNext->Enable(m_artifact983.StepNext983());
		m_pArtiPrev->Enable(m_artifact983.CanPrev983());
		RefreshArtifact983();
	}
}

// 983 00adf400: icone/nome/descricao do artefato do cursor; "no_artifact" quando nao ha (ou nao e utilizavel)
void FrMakeRoomDlg::RefreshArtifact983()
{
	const char* pName = K2L_Compatibility("\xbb\xe7\xbf\xeb\xbe\xc8\xc7\xd4"); // 983 00d4a700 ("Not In Use")
	const Bitmap* pIcon = g_pFresh->GetBitmap("no_artifact");
	const char* pDesc = "";

	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(*m_artifact983.GetCur());
	if (pItem && pItem->Artifact != 0)
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(pItem->c.Icon);
		if (pBmp)
			pIcon = pBmp;
		pName = pItem->c.Name;
		const char* pTxt = ItemManager()->FindArtifactDesc983(*m_artifact983.GetCur());
		if (pTxt)
			pDesc = pTxt;
	}
	if (m_pArtiIcon)
		((FrArea*)m_pArtiIcon)->SetBgImg(pIcon);
	UpdateArtifact983(true, 0);
	if (m_pArtiName)
		m_pArtiName->SetCaption(pName); // 983 FrStatic::SetCaption 00c02500
	if (m_pArtiDesc)
		m_pArtiDesc->SetCaption(pDesc);
}

void FrMakeRoomDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pTitle)
	{
		m_pTitle->SetCharLimit(20, false);
		SetDefaultTitle();
	}
}

bool FrMakeRoomDlg::OnTitleEnterKey(int param)
{
	return false;
}

void FrMakeRoomDlg::OnPasswordInit(int param)
{
	FrEdit* pPassword = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrMakeRoomDlg::OnMapPrevInit(int param)
{
	m_pMapPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMapPrev)
	{
		m_pMapPrev->SetPushDelay(0.0f);
		m_pMapPrev->UseDblClick(false); // 983 00add490: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrMakeRoomDlg::OnMapPrevUp()
{
	if (m_pMapPrev && m_pMapPrevChat)
	{
		unsigned char map = PrevMap(m_course);
		ChangeCourse(map);
	}
}

void FrMakeRoomDlg::OnMapNextInit(int param)
{
	m_pMapNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMapNext)
	{
		m_pMapNext->SetPushDelay(0.0f);
		m_pMapNext->UseDblClick(false); // 983 00add520: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrMakeRoomDlg::OnMapNextUp()
{
	if (m_pMapNext && m_pMapNextChat)
	{
		unsigned char map = NextMap(m_course, m_gameType == 2);
		ChangeCourse(map);
	}
}

void FrMakeRoomDlg::OnMapPrevChatInit(int param)
{
	m_pMapPrevChat = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMapPrevChat)
	{
		m_pMapPrevChat->SetPushDelay(0.0f);
		m_pMapPrevChat->UseDblClick(false); // 983 00add5b0: +0x44 |= 0x200 (FWS_NODBLCLICK) antes do SetVisible
		m_pMapPrevChat->SetVisible(false);
	}
}

void FrMakeRoomDlg::OnMapNextChatInit(int param)
{
	m_pMapNextChat = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMapNextChat)
	{
		m_pMapNextChat->SetPushDelay(0.0f);
		m_pMapNextChat->UseDblClick(false); // 983 00add650: +0x44 |= 0x200 (FWS_NODBLCLICK) antes do SetVisible
		m_pMapNextChat->SetVisible(false);
	}
}

// E6: MakeRoom pelo 983 (00ae19c0); o corpo em asm copiado do 645 foi removido.
bool FrMakeRoomDlg::MakeRoom(bool bQuick)
{
	const char* pStopMsg = NULL;
	if (DOCJP()->IsControlServerService983(0x40) && m_gameType == 0)
		pStopMsg = "\xbd\xba\xc6\xae\xb7\xce\xc5\xa9\x20\xb0\xd4\xc0\xd3\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x80) && m_gameType == 1)
		pStopMsg = "\xb8\xc5\xc4\xa1\x20\xb0\xd4\xc0\xd3\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x100) && m_gameType == 4)
		pStopMsg = "\x33\x30\xc0\xce\x20\xb4\xeb\xc8\xb8\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x2000000) && m_gameType == GAME_TYPE_USEMAX)
		pStopMsg = "\xbc\xc5\xc7\xc3\x20\xb8\xf0\xb5\xe5\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x200) && m_gameType == 5)
		pStopMsg = "\x33\x30\xc0\xce\x20\xb4\xeb\xc8\xb8\xc6\xc0\xc0\xfc\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x400) && m_gameType == 6)
		pStopMsg = "\xb1\xe6\xb5\xe5\xb4\xeb\xc0\xfc\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x800) && m_gameType == 7)
		pStopMsg = "\xc6\xce\xb9\xe8\xc6\xb2\x20\xb0\xd4\xc0\xd3\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x1000) && m_gameType == 10)
		pStopMsg = "\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1\x20\xb0\xd4\xc0\xd3\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	else if (DOCJP()->IsControlServerService983(0x2000) && m_gameType == 2)
		pStopMsg = "\xb4\xeb\xc8\xad\xb9\xe6\x20\xb0\xfc\xb7\xc3\x20\xba\xce\xba\xd0\x20\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9\x2e";
	if (pStopMsg) // 983 00ae19c0: K2L uma vez, no envio (sem teste do retorno)
	{
		AfxGetTask()->GetActor("Lobby")
			<< MsgObject(NULL, M983(0x23), (int)K2L_Compatibility(pStopMsg), 0, 0, 0, 0);
		return false;
	}

	// 983: servico 0x20000000 (artefato) parado com o flag 2 do cursor +0x20c: so o aviso de servico parado
	if (DOCJP()->IsControlServerService983(0x20000000) && (m_classicFlags & 2))
	{
		S5::UTIL::NotifyServiceStopped983(0x20000000);
		return false;
	}

	std::string title;
	FrWnd* pWnd = FindChildByName("title");
	FrEdit* pTitle = DYNAMIC_CAST(FrEdit, pWnd);
	if (pTitle)
		title = pTitle->GetLine(1, false);

	if (m_gameType == 6 && CSharedDoc::Instance()->m_myInfo.info.dwGuildId == 0)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xb1\xe6\xb5\xe5 \xb0\xa1\xc0\xd4\xc0\xda\xb8\xb8 \xb9\xe6\xc0\xbb \xb8\xb8\xb5\xe5\xbd\xc7 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 3);
	}
	else
	{
		if (m_gameType == 2 && m_course == 0x7f)
			m_course = rand() % 11;

		if (title.empty() || title.find_first_not_of(' ') == std::string::npos)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xb9\xe6 \xc1\xa6\xb8\xf1\xc0\xbb \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4"),
				false);
			pForm->Open(NULL, 3);
		}
		else if (CSharedDoc::Instance()->m_chatManager.Filtering(
					 (char*)title.c_str()))
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xb9\xe6 \xc1\xa6\xb8\xf1\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb9\xae\xc0\xda\xb8\xa6 \xc6\xf7\xc7\xd4\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 3);
		}
		// 983 00ae1ca6: tipos 4/5 exigem o byte +0x91d do CSharedDoc (aviso "notify": rookie F nao joga 30 pessoas)
		else if ((m_gameType == 4 || m_gameType == 5) &&
			CSharedDoc::Instance()->m_myInfo.stat.Level == 0)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pForm->SetMessage(
				K2L_Compatibility("\xb7\xe7\xc5\xb0\x46 \xb7\xb9\xba\xa7\xc0\xba 30\xc0\xce\xb4\xeb\xc0\xfc \xb9\xd7 30\xc0\xce\xc6\xc0\xc0\xfc \xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open(NULL, 3);
		}
		else
		{
			pWnd = FindChildByName("password");
			FrEdit* pPassword = DYNAMIC_CAST(FrEdit, pWnd);
			std::string password;
			if (pPassword)
			{
				const char* text = pPassword->GetLine(1, false);
				password = text;
				if (!bQuick)
				{
					AfxGetTask()->GetActor("Lobby")
						<< MsgObject(NULL, M983(0x27), (int)text, 0, 0, 0, 0);
				}
				else
				{
					AfxGetTask()->GetActor("AvatarChat")
						<< MsgObject(NULL, M983(0x27), (int)text, 0, 0, 0, 0);
				}
			}

			if (IsMassGame(m_gameType))
			{
				m_shotTime = 0;
				if (m_gameType == 10)
					m_gameTime = m_timeLimit * 1000;
				else
					m_gameTime = m_timeLimit * 60000;
			}
			else if ((unsigned char)(m_gameType - 0xb) < 4)
			{
				m_shotTime = 0; // 983 00ae1ebd: tipos 0xb..0xe nao mexem no tempo de partida
			}
			else
			{
				m_shotTime = m_timeLimit * 1000;
				m_gameTime = 0;
			}

			if (m_gameType == 2)
				m_hole = m_chatHole;

			// 983 00ae1f10: titulo cortado em 20 por buffer de 0xff (memset 0xfe; antes da checagem do canal)
			{
				char cut[0xff] = {0};
				strncpy(cut, title.c_str(), 0xff);
				cut[0x14] = 0;
				title = cut;
			}

			if (IsLocalContent(S3_ROOKIE_RESTRICT) &&
				(CSharedDoc::Instance()->m_curChannel.Type & 0x800) &&
				m_gameType == 7)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", NULL);
				pForm->SetMessage(
					K2L_Compatibility("\xb7\xe7\xc5\xb0 \xc3\xa4\xb3\xce\xbf\xa1\xbc\xad\xb4\xc2 \xc6\xce\xb9\xe8\xc6\xb2 \xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				pForm->Open(NULL, 3);
			}
			else
			{
				nsInvalidCheck::_invalidPwd check =
					std::for_each(password.begin(), password.end(),
						nsInvalidCheck::_invalidPwd());
				if (check.bInvalid)
				{
					FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "notify", NULL);
					pForm->SetMessage(
						K2L_Compatibility("\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xbf\xa1 \xb0\xf8\xb9\xe9\xc0\xba \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						false);
					pForm->Open(NULL, 3);
				}
				else
				{
					// 983 00ae1fda: UpdateArtifact983(0, 2) falha -> nada e enviado
					if (UpdateArtifact983(false, 2))
						return false;

					WSendPacket packet((enumClientPacket)8);
					packet.Encode1(bQuick ? 1 : 0);
					packet.Encode4(m_shotTime);
					packet.Encode4(m_gameTime);
					packet.Encode1(m_userLimit);
					if (m_gameType == 5)
						m_gameType = 4;
					packet.Encode1(m_gameType);
					packet.Encode1(m_hole);
					packet.Encode1(m_course);
					packet.Encode1(m_holeType);
					// 983 00ae2150: titulo cortado em 20 e senha em 8 (strncpy em buffer de 0x28 + terminador no limite)
					{
						char titleBuf[0x28] = {0};
						strncpy(titleBuf, title.c_str(), 0x28);
						titleBuf[0x14] = 0;
						title = titleBuf;
						char pwdBuf[0x28];
						memset(pwdBuf, 0, sizeof(pwdBuf));
						strncpy(pwdBuf, password.c_str(), 0x28);
						pwdBuf[8] = 0;
						password = pwdBuf;
					}
					// 983 00ae2203: dword de this+0x20c antes do titulo; 00ae2238: no fim, o dword apontado pelo objeto de
					// this+0x1fc (00479e90: [obj+0xc]+0xc; os controles "artifact_ctrl" -> artefato da sala). A216: m_classicFlags e
					// m_artifact983 (C_ASSERT no construtor), com guarda de nulo no cursor.
					packet.Encode4(m_classicFlags);
					packet.EncodeStr(title);
					packet.EncodeStr(password);
					{
						packet.Encode4(m_artifact983.m_pCur ? *m_artifact983.GetCur() : 0);
					}
					packet.Send(TO_GAME);

					if (!bQuick)
					{
						AfxGetTask()->GetActor("Lobby")
							<< MsgObject(NULL, M983(0x16), 1, 0, 0, 0, 0);
						AfxGetTask()->GetActor("Lobby")
							<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
					}
					else
					{
						AfxGetTask()->GetActor("AvatarChat")
							<< MsgObject(NULL, M983(0x16), 1, 0, 0, 0, 0);
						AfxGetTask()->GetActor("AvatarChat")
							<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
					}

					return true;
				}
			}
		}
	}

	return false;
}

void FrMakeRoomDlg::ChangeGameType(unsigned char type, bool bInit)
{
	if (m_gameType == 2 && type != 2)
	{
		m_pMapPrev->SetVisible(true);
		m_pMapNext->SetVisible(true);
		m_pMapPrevChat->SetVisible(false);
		m_pMapNextChat->SetVisible(false);
		m_pCourseChat->SetVisible(false);
		m_pHoleChat->SetVisible(false);
		m_pHoleMin->SetVisible(false);
		m_pHolePrev->SetVisible(false);
		m_pHoleNext->SetVisible(false);
		m_pHoleMax->SetVisible(false);
		m_pHoleType->SetVisible(true);
		m_pCourse->SetVisible(true);
		m_pOverGauge->SetVisible(true);
		if (m_pClassicTypeBtn)
			m_pClassicTypeBtn->SetVisible(true);
	}

	if (m_gameType != 2 && type == 2)
	{
		m_pMapPrev->SetVisible(false);
		m_pMapNext->SetVisible(false);
		m_pMapPrevChat->SetVisible(true);
		m_pMapNextChat->SetVisible(true);
		m_pCourseChat->SetVisible(true);
		m_pHoleChat->SetVisible(true);
		m_pHoleMin->SetVisible(true);
		m_pHolePrev->SetVisible(true);
		m_pHoleNext->SetVisible(true);
		m_pHoleMax->SetVisible(true);
		m_pHoleType->SetVisible(false);
		m_pCourse->SetVisible(false);
		m_pOverGauge->SetVisible(false);
		if (m_pClassicTypeBtn)
			m_pClassicTypeBtn->SetVisible(false);

		if (m_course == 0x7f)
			ChangeCourse(FirstMap());
	}
	else if (m_gameType == GAME_TYPE_USEMAX && type != GAME_TYPE_USEMAX)
	{
		m_pCourse->SetMouseEvent(true);
		m_pMapPrev->SetVisible(true);
		m_pMapNext->SetVisible(true);
		m_pHoleType->SetVisible(true);
		m_pCourse->SetVisible(true);
		m_pOverGauge->SetVisible(true);

		if (m_course == 0x7f)
			ChangeCourse(FirstMap());
	}

	if (m_gameType == 10 && type != 10)
	{
		m_pCourse->SetMouseEvent(true);
		ChangeCourse(FirstMap());
	}
	else if (m_gameType == 6 && type != 6)
	{
		m_pCourse->SetMouseEvent(true);
		ChangeCourse(FirstMap());
	}
	else if (m_gameType != 6 && type == 6)
	{
		m_pCourse->SetMouseEvent(false);
		ChangeCourse(0x7f);
		m_pMapPrev->Enable(false);
		m_pMapPrevChat->Enable(false);
		m_pMapNext->Enable(false);
		m_pMapNextChat->Enable(false);
	}
	else if (m_gameType != 10 && type == 10)
	{
		m_pCourse->SetMouseEvent(false);
		ChangeCourse(0x7f);
		m_pMapPrev->Enable(false);
		m_pMapPrevChat->Enable(false);
		m_pMapNext->Enable(false);
		m_pMapNextChat->Enable(false);
	}

	if (m_gameType != GAME_TYPE_USEMAX && type == GAME_TYPE_USEMAX)
	{
		m_pCourse->SetMouseEvent(false);
		m_pMapPrev->SetVisible(false);
		m_pMapPrevChat->SetVisible(false);
		m_pMapNext->SetVisible(false);
		m_pMapNextChat->SetVisible(false);
		m_pHoleType->SetVisible(false);
		m_pCourse->SetVisible(false);
		m_pOverGauge->SetVisible(false);
		ChangeCourse(0x7f);
	}

	// 983: tipos 0xb/0xd (hole in one / evento): curso fixo 0x40, sem setas
	switch (m_gameType)
	{
	case GAME_TYPE_HOLEINONE983:
	case GAME_TYPE_UNK983_13:
		if (type != GAME_TYPE_HOLEINONE983 && type != GAME_TYPE_UNK983_13 && type != 2)
		{
			m_pCourse->SetMouseEvent(true);
			m_pMapPrev->SetVisible(true);
			m_pMapNext->SetVisible(true);
			m_pHoleType->SetVisible(true);
			m_pCourse->SetVisible(true);
			m_pOverGauge->SetVisible(true);
			ChangeCourse(FirstMap());
		}
		break;
	default:
		if (type == GAME_TYPE_HOLEINONE983 || type == GAME_TYPE_UNK983_13)
		{
			m_pCourse->SetMouseEvent(false);
			m_pMapPrev->SetVisible(false);
			m_pMapPrevChat->SetVisible(false);
			m_pMapNext->SetVisible(false);
			m_pMapNextChat->SetVisible(false);
			m_pHoleType->SetVisible(true);
			m_pCourse->SetVisible(true);
			m_pOverGauge->SetVisible(false);
			ChangeCourse(0x40);
		}
		break;
	}

	m_gameType = type;

	for (int i = 0; i < 6; ++i)
		m_pMakeBtn[i]->SetStatus(FrButton::NORMAL);

	if (IsLocalContent(S3_ROOKIE_RESTRICT) &&
		(CSharedDoc::Instance()->m_curChannel.Type & 0x800) &&
		CSharedDoc::Instance()->m_myInfo.stat.Level == 0)
		m_pMakeBtn[2]->Enable(false);

	m_pUserLimit->ClearTab();
	m_pHole->ClearTab();
	m_pTimeLimit->ClearTab();

	switch (m_gameType)
	{
	case 0:
		m_pPicture->Open("picture_stroke.tga");
		m_pText->SetBgImg("text_vs");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_vs_map"); // 983 00adfc60: +0x184 sem teste de NULL (todos os cases)
		m_gameTypeGroup = 0;
		m_userLimit = 2;
		m_hole = 3;
		m_shotTime = 40000;
		m_timeLimit = 40;
		m_holeType = 0;
		m_pUserLimit->AddTab("select_button", 2, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 3, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 4, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 3, &WSize(37.5f, 20.0f));
		m_pHole->AddTab("select_button", 6, &WSize(37.5f, 20.0f));
		m_pHole->AddTab("select_button", 9, &WSize(37.5f, 20.0f));
		m_pHole->AddTab("select_button", 18, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 40, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 60, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 120, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 300, &WSize(37.5f, 20.0f));
		if (!bInit)
			m_pMakeBtn[0]->SetStatus(FrButton::PRESSED);
		break;
	case 1:
		m_pPicture->Open("picture_match.tga");
		m_pText->SetBgImg("text_vs");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 0;
		m_userLimit = 4;
		m_hole = 6;
		m_shotTime = 40000;
		m_timeLimit = 40;
		m_holeType = 0;
		m_pUserLimit->AddTab("select_button", 2, &WSize(75.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 4, &WSize(75.0f, 20.0f));
		m_pHole->AddTab("select_button", 6, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 9, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 18, &WSize(50.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 40, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 60, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 120, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 300, &WSize(37.5f, 20.0f));
		if (!bInit)
			m_pMakeBtn[0]->SetStatus(FrButton::PRESSED);
		break;
	case 4:
	case 5:
		m_pPicture->Open(m_gameType == 5 ? "picture_30s_short.tga" : "picture_30s.tga");
		m_pText->SetBgImg("text_mass");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_mass_map");
		m_gameTypeGroup = 1;
		m_userLimit = 30;
		m_hole = 18;
		m_gameTime = 2100000;
		m_timeLimit = 35;
		m_holeType = 0;
		if (IsLocalContent(S3_HUNDRED_MODE) &&
			(CSharedDoc::Instance()->m_myInfo.info.dwIdentity & 4))
		{
			m_pUserLimit->AddTab("select_button", 10, &WSize(25.0f, 20.0f));
			m_pUserLimit->AddTab("select_button", 20, &WSize(25.0f, 20.0f));
			m_pUserLimit->AddTab("select_button", 30, &WSize(25.0f, 20.0f));
			m_pUserLimit->AddTab("select_button", 100, &WSize(25.0f, 20.0f));
			m_pUserLimit->AddTab("select_button", 200, &WSize(25.0f, 20.0f));
			m_pUserLimit->AddTab("select_button", 250, &WSize(25.0f, 20.0f));
		}
		else
		{
			m_pUserLimit->AddTab("select_button", 10, &WSize(50.0f, 20.0f));
			m_pUserLimit->AddTab("select_button", 20, &WSize(50.0f, 20.0f));
			m_pUserLimit->AddTab("select_button", 30, &WSize(50.0f, 20.0f));
		}
		m_pHole->AddTab("select_button", 9, &WSize(75.0f, 20.0f));
		m_pHole->AddTab("select_button", 18, &WSize(75.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 35, &WSize(30.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 40, &WSize(30.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 45, &WSize(30.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 50, &WSize(30.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 55, &WSize(30.0f, 20.0f));
		if (!bInit)
			m_pMakeBtn[1]->SetStatus(FrButton::PRESSED);
		if (m_gameType == 5)
		{
			m_gameTime = 900000;
			m_timeLimit = 15;
			SetClassicFlags983(2);
		}
		break;
	case 6:
		m_pPicture->Open("picture_guild.tga");
		m_pText->SetBgImg("text_mass");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_mass_map");
		m_gameTypeGroup = 1;
		m_userLimit = 30;
		m_hole = 18;
		m_gameTime = 1800000;
		m_timeLimit = 30;
		m_holeType = 0;
		m_pUserLimit->AddTab("select_button", 10, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 20, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 30, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 9, &WSize(75.0f, 20.0f));
		m_pHole->AddTab("select_button", 18, &WSize(75.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 30, &WSize(50.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 35, &WSize(50.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 40, &WSize(50.0f, 20.0f));
		if (!bInit)
			m_pMakeBtn[1]->SetStatus(FrButton::PRESSED);
		break;
	case 7:
		m_pPicture->Open("picture_battle.tga");
		m_pText->SetBgImg("text_vs");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 2;
		m_userLimit = 2;
		m_hole = 6;
		m_shotTime = 40000;
		m_timeLimit = 40;
		m_holeType = 0;
		m_pUserLimit->AddTab("select_button", 2, &WSize(75.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 4, &WSize(75.0f, 20.0f));
		m_pHole->AddTab("select_button", 6, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 9, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 18, &WSize(50.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 40, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 60, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 120, &WSize(37.5f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 300, &WSize(37.5f, 20.0f));
		if (!bInit)
			m_pMakeBtn[2]->SetStatus(FrButton::PRESSED);
		break;
	case 10:
		m_pPicture->Open("picture_approach.tga");
		m_pText->SetBgImg("text_vs");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 2;
		m_userLimit = 6;
		m_hole = 3;
		m_gameTime = 40000;
		m_timeLimit = 40;
		m_holeType = 3;
		m_pUserLimit->AddTab("select_button", 6, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 20, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 30, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 3, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 6, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 9, &WSize(50.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 40, &WSize(150.0f, 20.0f));
		if (!bInit)
			m_pMakeBtn[2]->SetStatus(FrButton::PRESSED);
		break;
	case 9:
		m_pPicture->Open("picture_approach.tga");
		m_pText->SetBgImg("text_vs");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 2;
		m_userLimit = 30;
		m_hole = 1;
		m_gameTime = 40000;
		m_timeLimit = 40;
		m_holeType = 2;
		m_pUserLimit->AddTab("select_button", 10, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 20, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 30, &WSize(50.0f, 20.0f));
		m_pHole->AddTab("select_button", 1, &WSize(150.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 40, &WSize(150.0f, 20.0f));
		if (!bInit)
			m_pMakeBtn[2]->SetStatus(FrButton::PRESSED);
		break;
	case 2:
		m_pPicture->Open("picture_chat.tga");
		m_pText->SetBgImg("text_chat");
		m_gameTypeGroup = 3;
		m_userLimit = 20;
		m_hole = 1;
		m_pUserLimit->AddTab("select_button", 10, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 20, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 30, &WSize(50.0f, 20.0f));
		if (!bInit)
			m_pMakeBtn[3]->SetStatus(FrButton::PRESSED);
		break;
	case GAME_TYPE_USEMAX:
		m_pPicture->Open("picture_chaos.tga");
		m_pText->SetBgImg("text_chaos");
		m_gameTypeGroup = 4;
		m_userLimit = 30;
		m_hole = 18;
		m_gameTime = 2400000;
		m_timeLimit = 40;
		m_holeType = 5;
		if (!bInit)
			m_pMakeBtn[4]->SetStatus(FrButton::PRESSED);
		break;
	case GAME_TYPE_HOLEINONE983:
	case GAME_TYPE_UNK983_13:
		m_pPicture->Open("picture_30s.tga");
		m_pText->SetBgImg("text_mass");
		((FrArea*)m_pMapCtrlText)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 5;
		m_gameTime = 420000;
		m_timeLimit = 7;
		m_pTimeLimit->AddTab("select_button", 7, &WSize(75.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 10, &WSize(75.0f, 20.0f));
		m_userLimit = 100;
		m_pUserLimit->AddTab("select_button", 4, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 50, &WSize(50.0f, 20.0f));
		m_pUserLimit->AddTab("select_button", 100, &WSize(50.0f, 20.0f));
		m_holeType = 0;
		m_hole = 1;
		m_pHole->AddTab("select_button", 1, &WSize(150.0f, 20.0f));
		if (!bInit)
			m_pMakeBtn[5]->SetStatus(FrButton::PRESSED);
		break;
	}

	// 983 00adfc60: SetStatus e o bloco do tipo 5 ficam dentro de cada case
	m_pTimeLimit->SetVisible(m_gameType != 6);
	SetDefaultTitle();
	if (m_hole == 18 && m_gameType != 10 && m_gameType != GAME_TYPE_USEMAX &&
		(m_holeType == 1 || m_holeType == 2))
		m_holeType = 0;
	if (bInit)
	{
		m_pGameType->ClearTab();
		switch (m_gameTypeGroup)
		{
		case 0:
			m_pGameType->AddTab("select_button", 0, &WSize(75.0f, 20.0f));
			m_pGameType->AddTab("select_button", 1, &WSize(75.0f, 20.0f));
			break;
		case 1:
			m_pGameType->AddTab("select_button", 4, &WSize(50.0f, 20.0f));
			m_pGameType->AddTab("select_button", 5, &WSize(50.0f, 20.0f));
			m_pGameType->AddTab("select_button", 6, &WSize(50.0f, 20.0f));
			break;
		case 2:
			if (IsLocalContent(S3_ROOKIE_RESTRICT) &&
				(CSharedDoc::Instance()->m_curChannel.Type & 0x800))
			{
				m_pGameType->AddTab("select_button", 10, &WSize(150.0f, 20.0f));
			}
			else
			{
				m_pGameType->AddTab("select_button", 10, &WSize(75.0f, 20.0f));
				m_pGameType->AddTab("select_button", 7, &WSize(75.0f, 20.0f));
			}
			break;
		case 3:
			m_pGameType->AddTab("select_button", 2, &WSize(150.0f, 20.0f));
			break;
		case 4:
			m_pGameType->ClearTab();
			break;
		case 5:
			m_pGameType->AddTab("select_button", 11, &WSize(75.0f, 20.0f));
			m_pGameType->AddTab("select_button", 13, &WSize(75.0f, 20.0f));
			break;
		}
	}
	m_pGameType->Select(m_gameType);
	m_pUserLimit->Select(m_userLimit);
	m_pHole->Select(m_hole);
	m_pTimeLimit->Select(m_timeLimit);
	ApplyClassicFlags983();
}

void FrMakeRoomDlg::ChangeCourse(unsigned char course)
{
	unsigned int index;
	if (course <= 0x7f || course == 0xfd)
		index = course;
	else
		index = course - 0x80;

	IFF_STRUCT::sCourse* pCourse =
		ItemManager()->FindCourse(index | 0x28000000);
	if (pCourse == NULL)
		return;

	if (course != 0x7f)
	{
		m_pCourse->Open(MakeStr("small_%s.tga", pCourse->c.Icon));
		m_pCourseChat->Open(MakeStr("small_%s.tga", pCourse->c.Icon));
		unsigned long gauge =
			CTHunter::Instance()->GetTHunterGauge((eMapType)course);
		if (gauge != 9999)
		{
			m_pOverGauge->ResetGauge();
			int pos;
			if (gauge < 701)
				pos = 0;
			else
				pos = gauge - 700;
			m_pOverGauge->SetDestPos(pos, false);
		}
	}
	else
	{
		m_pCourse->Open("map_random_middle.tga");
		m_pCourseChat->Open("map_random_middle.tga");
		if (m_pOverGauge)
		{
			m_pOverGauge->ResetGauge();
			m_pOverGauge->SetDestPos(0, false);
		}
	}

	m_course = course;

	if (m_pMapPrev)
		m_pMapPrev->Enable(m_course != FirstMap());
	if (m_pMapPrevChat)
		m_pMapPrevChat->Enable(m_course != FirstMap());
	const bool bAll = Doc()->m_roomInfo.holeType == 4; // 983 00adf6d5: lido antes do teste de m_pMapNext
	if (m_pMapNext)
		m_pMapNext->Enable(m_course != LastMap(bAll));
	if (m_pMapNextChat)
		m_pMapNextChat->Enable(m_course != LastMap(true));
}

void FrMakeRoomDlg::OnGameTypeInit(int param)
{
	m_pGameType = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	if (m_pGameType)
		m_pGameType->SetSepWidth(1.0f);
}

// 983 00ae1700: tipo 5 refaz o tempo (15..35, seleciona 15) e liga o bit 2 do flag 0x20c; outro tipo desliga o bit
void FrMakeRoomDlg::OnGameTypeButtonUp()
{
	TButtonItem* pItem = m_pGameType->GetSelected();
	if (pItem)
	{
		ChangeGameType((unsigned char)pItem->index, false);
		unsigned long flags = m_classicFlags;
		if (pItem->index == 5)
		{
			if (m_pTimeLimit)
			{
				m_pTimeLimit->ClearTab();
				m_pTimeLimit->AddTab("select_button", 15, &WSize(30.0f, 20.0f));
				m_pTimeLimit->AddTab("select_button", 20, &WSize(30.0f, 20.0f));
				m_pTimeLimit->AddTab("select_button", 25, &WSize(30.0f, 20.0f));
				m_pTimeLimit->AddTab("select_button", 30, &WSize(30.0f, 20.0f));
				m_pTimeLimit->AddTab("select_button", 35, &WSize(30.0f, 20.0f));
				m_timeLimit = 15;
				m_pTimeLimit->Select(15);
			}
			flags |= 2;
		}
		else
			flags &= ~2UL;
		SetClassicFlags983(flags);
		UpdateArtifact983(true, 1);
	}
}

void FrMakeRoomDlg::OnGameTypeOwnerDraw(int param)
{
	TButtonItem* pItem = (TButtonItem*)param;
	if (pItem == NULL)
		return;

	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;

	pGI->PrintText11(
		WPoint(pItem->rect.w * 0.5f + pItem->rect.x, pItem->rect.y + 5.0f), 1,
		s_gameTypeTitle[pItem->index], NULL);
}

void FrMakeRoomDlg::OnUserLimitInit(int param)
{
	m_pUserLimit = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	if (m_pUserLimit)
		m_pUserLimit->SetSepWidth(1.0f);
}

void FrMakeRoomDlg::OnUserLimitButtonUp()
{
	if (m_pUserLimit->GetSelected())
		ChangeUserLimit((unsigned char)m_pUserLimit->GetSelected()->index);
}

void FrMakeRoomDlg::OnUserLimitOwnerDraw(int param)
{
	TButtonItem* pItem = (TButtonItem*)param;
	if (pItem == NULL)
		return;

	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;

	pGI->PrintText11(
		WPoint(pItem->rect.w * 0.5f + pItem->rect.x, pItem->rect.y + 5.0f), 1,
		MakeStr("%d", pItem->index), NULL);
}

void FrMakeRoomDlg::OnHoleInit(int param)
{
	m_pHole = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	if (m_pHole)
		m_pHole->SetSepWidth(1.0f);
}

void FrMakeRoomDlg::OnHoleButtonUp()
{
	if (m_pHole->GetSelected())
		ChangeHole((unsigned char)m_pHole->GetSelected()->index);
}

void FrMakeRoomDlg::OnHoleOwnerDraw(int param)
{
	TButtonItem* pItem = (TButtonItem*)param;
	if (pItem == NULL)
		return;

	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;

	pGI->PrintText11(
		WPoint(pItem->rect.w * 0.5f + pItem->rect.x, pItem->rect.y + 5.0f), 1,
		MakeStr("%d", pItem->index), NULL);
}

void FrMakeRoomDlg::OnTimeLimitInit(int param)
{
	m_pTimeLimit = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	if (m_pTimeLimit)
		m_pTimeLimit->SetSepWidth(1.0f);
}

void FrMakeRoomDlg::OnTimeLimitButtonUp()
{
	if (m_pTimeLimit->GetSelected())
		ChangeTimeLimit(m_pTimeLimit->GetSelected()->index);
}

void FrMakeRoomDlg::OnTimeLimitOwnerDraw(int param)
{
	TButtonItem* pItem = (TButtonItem*)param;
	if (pItem == NULL)
		return;

	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;

	pGI->PrintText11(
		WPoint(pItem->rect.w * 0.5f + pItem->rect.x, pItem->rect.y + 5.0f), 1,
		MakeStr("%d", pItem->index), NULL);
}

void FrMakeRoomDlg::OnCourseInit(int param)
{
	m_pCourse = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

bool FrMakeRoomDlg::OnMapSelectDlgResult(int result, FrForm* form)
{
	unsigned char map = 0;
	if (form)
		map = ((FrTreasureCourse*)form)->GetCurMapIdx();
	ChangeCourse(map);

	m_pMapSelectDlg = NULL;
	m_btnDelay = 0.7f;
	return true;
}

void FrMakeRoomDlg::OnCourseUp()
{
	// 983 00adf7e0: switch(tipo): 6, 10, 11, 13 e 18 nao abrem a lista de mapas
	switch (m_gameType)
	{
	case 6:
	case 10:
	case 11:
	case 13:
	case 18:
		return;
	}

	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);

	m_pMapSelectDlg = CreateForm<FrTreasureCourse>(g_pFresh->GetManager(), this,
		"treasuremap", NULL);
	if (m_pMapSelectDlg)
	{
		m_pMapSelectDlg->SetSelectMap(m_course);
		m_pMapSelectDlg->SetMapIndexChanged(false);
		if (DOCJP()->GetState983() == 7) // 983: botao aleatorio (+0x120) desligado no estado 7 (desvio, sem setne)
			m_pMapSelectDlg->EnableRandomBtn(false);
		else
			m_pMapSelectDlg->EnableRandomBtn(true);
		m_pMapSelectDlg->LockRandom(m_gameType == 2);
		m_pMapSelectDlg->EnableDrag(false);
		m_pMapSelectDlg->Open(
			(FRESH_PFN_RESULT)&FrMakeRoomDlg::OnMapSelectDlgResult, 3);
	}
}

void FrMakeRoomDlg::OnCourseOwnerDraw(int param)
{
	if (m_gameType == 10 || m_gameType == 6 || m_pCourse == NULL ||
		m_pMapSelectO == NULL)
		return;

	WRect rect = m_pCourse->GetRect();
	FrWndManager* pManager = g_pFresh->GetManager();
	if (rect.IsInRect(pManager->GetMousePos()))
	{
		FrGraphicInterface* pGI = pManager->GetGDI();
		if (pGI)
		{
			WRect dest(rect.x - 25.0f, rect.y - 11.0f,
				(float)m_pMapSelectO->Width(), (float)m_pMapSelectO->Height());
			pGI->DrawTexture(m_pMapSelectO, dest, 0xffffffff, 0);
		}
	}
}

void FrMakeRoomDlg::OnHoleTypeInit(int param)
{
	m_pHoleType = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pHoleType)
	{
		m_pHoleTypeBtn[0][0] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_front_n");
		m_pHoleTypeBtn[0][1] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_front_d");
		m_pHoleTypeBtn[1][0] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_back_n");
		m_pHoleTypeBtn[1][1] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_back_d");
		m_pHoleTypeBtn[2][0] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_random_n");
		m_pHoleTypeBtn[2][1] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_random_d");
		m_pHoleTypeBtn[3][0] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_shuffle_n");
		m_pHoleTypeBtn[3][1] =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "hole_shuffle_d");
	}
}

void FrMakeRoomDlg::DrawHoleTypeButton(FrGraphicInterface* gi, eHoleType type,
	WRect rect, bool bEnable)
{
	const Bitmap* pBitmap = m_pHoleTypeBtn[type][0];
	unsigned long color = 0xffffffff;

	if (bEnable)
	{
		if (m_holeType == type)
			pBitmap = m_pHoleTypeBtn[type][1];
		else
		{
			const WPoint& mousePos = g_pFresh->GetManager()->GetMousePos();
			if (IsBtnEnable() && rect.IsInRect(mousePos) &&
				g_input->GetButton(LEFT_BUTTON) == 2)
				ChangeHoleType(type);
		}
	}
	else
		color = 0xff909090;

	gi->DrawTexture(pBitmap, rect, color, 0);
}

void FrMakeRoomDlg::OnHoleTypeOwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI)
	{
		// 983 00adde40: retangulos dos botoes 0..2 montados antes, com um x corrente somado pela largura de cada botao
		WRect base = m_pHoleType->GetRect();
		float w = (float)m_pHoleTypeBtn[0][0]->Width();
		float h = (float)m_pHoleTypeBtn[0][0]->Height();
		WRect rc0(base.x, base.y, w, h);
		float x = base.x + w;
		w = (float)m_pHoleTypeBtn[1][0]->Width();
		WRect rc1(x, base.y, w, h);
		x += w;
		w = (float)m_pHoleTypeBtn[2][0]->Width();
		WRect rc2(x, base.y, w, h);
		x += w;
		w = (float)m_pHoleTypeBtn[3][0]->Width();
		// 983: tabela de desvio sobre gameType-10 (0..8): 10, 11 (Hole in One), 13 e 18 ficam cinza
		switch (m_gameType)
		{
		case 10:
		case GAME_TYPE_HOLEINONE983:
		case GAME_TYPE_UNK983_13:
		case GAME_TYPE_USEMAX:
			DrawHoleTypeButton(pGI, (eHoleType)0, rc0, false);
			DrawHoleTypeButton(pGI, (eHoleType)1, rc1, false);
			DrawHoleTypeButton(pGI, (eHoleType)2, rc2, false);
			DrawHoleTypeButton(pGI, (eHoleType)3, WRect(x, base.y, w, h), false);
			break;
		default:
			// com 18 buracos so os botoes 1 e 2 desligam; o 3 fica ligado
			if (m_hole == 18)
			{
				DrawHoleTypeButton(pGI, (eHoleType)0, rc0, true);
				DrawHoleTypeButton(pGI, (eHoleType)1, rc1, false);
				DrawHoleTypeButton(pGI, (eHoleType)2, rc2, false);
			}
			else
			{
				DrawHoleTypeButton(pGI, (eHoleType)0, rc0, true);
				DrawHoleTypeButton(pGI, (eHoleType)1, rc1, true);
				DrawHoleTypeButton(pGI, (eHoleType)2, rc2, true);
			}
			DrawHoleTypeButton(pGI, (eHoleType)3, WRect(x, base.y, w, h), true);
			break;
		}
	}
}

void FrMakeRoomDlg::OnCourseChatInit(int param)
{
	m_pCourseChat = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pCourseChat)
		m_pCourseChat->SetVisible(false);
}

void FrMakeRoomDlg::OnCourseChatUp()
{
	OnCourseUp();
}

void FrMakeRoomDlg::OnHoleChatInit(int param)
{
	m_pHoleChat = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pHoleChat)
		m_pHoleChat->SetVisible(false);
}

void FrMakeRoomDlg::OnHoleChatOwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI == NULL)
		return;

	// 983 00ade1c0: WRect copiado (so x/y usados) e posicao montada dele
	WRect rc = m_pHoleChat->GetRect();
	WPoint pos(rc.x + 17.0f, rc.y + 5.0f);
	pGI->PrintText11(pos, 1, MakeStr("%d", m_chatHole), NULL);
}

void FrMakeRoomDlg::ChangeUserLimit(unsigned char limit)
{
	m_userLimit = limit;
}

// 983 00adec00 (this, 1, 1): o artefato escolhido (CItemCursor983 em +0x1fc) x tipo/buraco da sala. Mostra o nome da imagem
// em m_pArtiBan (0x194) e, se o artefato nao serve para o tipo, o aviso na tela. Retorna bFail.
bool FrMakeRoomDlg::UpdateArtifact983(bool bUpdateBan, int notifyMode)
{
	static unsigned char s_mode983 = 0xff; // 983 00d4c894 (0xff = sala normal; com 0 todo artefato caia em modo nao permitido)
	const unsigned long* pTid = m_artifact983.GetCur();
	unsigned char flag = m_artifact983.GetFlag();
	sArtifactCheckArg983 arg;
	arg.pFlag = &flag;
	arg.pRoomType = &m_gameType;
	arg.pMode = &s_mode983;
	arg.p1094b = &m_hole;
	sArtifactCheck983 check(pTid, &arg);

	if (m_pArtiBan && bUpdateBan)
	{
		((FrArea*)m_pArtiBan)->SetBgImg(check.name);
		m_pArtiBan->SetVisible(check.bFail);
	}

	if (check.bFail && notifyMode > 0)
	{
		// 983: modo 1 avisa so sem b0 (sete), modo 2 sempre
		bool bSend;
		if (notifyMode == 1)
			bSend = check.b0 == 0;
		else if (notifyMode == 2)
			bSend = true;
		else
			bSend = false;
		if (bSend)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)check.text, 0, 0, 0, 0));
	}
	return check.bFail;
}

// 983 00adedf0 (pseudo-C do Ghidra): todo caminho termina em UpdateArtifact983(1, 1).
void FrMakeRoomDlg::ChangeHole(unsigned char hole)
{
	if (m_hole == hole)
		return;

	m_hole = hole;

	switch (m_gameType)
	{
	case 0:
	case 1:
	case 7:
		if (hole == 18 && (m_holeType == 1 || m_holeType == 2))
			m_holeType = 0;
		break;

	case 4:
		if (hole == 9)
		{
			m_pTimeLimit->ClearTab();
			m_pTimeLimit->AddTab("select_button", 15, &WSize(37.5f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 20, &WSize(37.5f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 25, &WSize(37.5f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 30, &WSize(37.5f, 20.0f));
			m_timeLimit = 15;
		}
		else if (hole == 18)
		{
			m_pTimeLimit->ClearTab();
			m_pTimeLimit->AddTab("select_button", 35, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 40, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 45, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 50, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 55, &WSize(30.0f, 20.0f));
			m_timeLimit = 35;
			if (m_holeType == 1 || m_holeType == 2)
				m_holeType = 0;
		}
		break;

	case 5:
		if ((m_classicFlags & 2) && m_pTimeLimit)
		{
			m_pTimeLimit->ClearTab();
			m_pTimeLimit->AddTab("select_button", 15, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 20, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 25, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 30, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 35, &WSize(30.0f, 20.0f));
			m_timeLimit = 15;
			m_pTimeLimit->Select(15);
		}
		break;

	case 6:
		if (hole == 9)
		{
			m_pTimeLimit->ClearTab();
			m_pTimeLimit->AddTab("select_button", 20, &WSize(37.5f, 20.0f));
			m_timeLimit = 20;
			m_pTimeLimit->Select(20);
		}
		else if (hole == 18)
		{
			m_pTimeLimit->ClearTab();
			m_pTimeLimit->AddTab("select_button", 30, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 35, &WSize(30.0f, 20.0f));
			m_pTimeLimit->AddTab("select_button", 40, &WSize(30.0f, 20.0f));
			m_timeLimit = 30;
			m_pTimeLimit->Select(30);
		}
		m_pTimeLimit->SetVisible(false);
		m_holeType = 0;
		break;

	case 18:
		m_pTimeLimit->ClearTab();
		m_pTimeLimit->SetVisible(false);
		m_timeLimit = 30;
		m_holeType = 2;
		break;

	case 11:
	case 13:
		m_pTimeLimit->AddTab("select_button", 7, &WSize(45.0f, 20.0f));
		m_pTimeLimit->AddTab("select_button", 10, &WSize(45.0f, 20.0f));
		break;
	}

	UpdateArtifact983(true, 1);
}

void FrMakeRoomDlg::ChangeTimeLimit(unsigned long time)
{
	m_timeLimit = time;
}

__declspec(noinline) void FrMakeRoomDlg::ChangeHoleType(eHoleType type)
{
	m_holeType = type;
}

void FrMakeRoomDlg::OnHoleMinInit(int param)
{
	m_pHoleMin = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pHoleMin)
	{
		m_pHoleMin->SetVisible(false);
		// 983 (ade230/ade2d0/ade370/ade410): sem duplo clique, atraso 0.1 (+0x104/+0x108), flag 0x200
		m_pHoleMin->SetPushDelay(0.0f); // 983: +0x104 = 0, +0x108 = 0.1
		m_pHoleMin->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrMakeRoomDlg::OnHoleMinUp()
{
	m_chatHole = 1;
}

void FrMakeRoomDlg::OnHolePrevInit(int param)
{
	m_pHolePrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pHolePrev)
	{
		m_pHolePrev->SetVisible(false);
		// 983 (ade230/ade2d0/ade370/ade410): sem duplo clique, atraso 0.1 (+0x104/+0x108), flag 0x200
		m_pHolePrev->SetPushDelay(0.0f); // 983: +0x104 = 0, +0x108 = 0.1
		m_pHolePrev->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrMakeRoomDlg::OnHolePrevUp()
{
	if (m_chatHole > 1)
		--m_chatHole;
}

void FrMakeRoomDlg::OnHoleNextInit(int param)
{
	m_pHoleNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pHoleNext)
	{
		m_pHoleNext->SetVisible(false);
		// 983 (ade230/ade2d0/ade370/ade410): sem duplo clique, atraso 0.1 (+0x104/+0x108), flag 0x200
		m_pHoleNext->SetPushDelay(0.0f); // 983: +0x104 = 0, +0x108 = 0.1
		m_pHoleNext->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrMakeRoomDlg::OnHoleNextUp()
{
	if (m_chatHole < 18)
		++m_chatHole;
}

void FrMakeRoomDlg::OnHoleMaxInit(int param)
{
	m_pHoleMax = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pHoleMax)
	{
		m_pHoleMax->SetVisible(false);
		// 983 (ade230/ade2d0/ade370/ade410): sem duplo clique, atraso 0.1 (+0x104/+0x108), flag 0x200
		m_pHoleMax->SetPushDelay(0.0f); // 983: +0x104 = 0, +0x108 = 0.1
		m_pHoleMax->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrMakeRoomDlg::OnHoleMaxUp()
{
	m_chatHole = 18;
}

void FrMakeRoomDlg::OnOverGauge_Init(int param)
{
	m_pOverGauge = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pOverGauge)
	{
		m_pOverGauge->SetRange(0, 300, 0);
		m_pOverGauge->SetBarSpeed(200.0f);
	}
}

void FrMakeRoomDlg::OnProc(const float dt)
{
	FrForm::OnProc(dt);

	if (m_pMapSelectDlg == NULL && m_btnDelay >= 0.0f)
		m_btnDelay -= dt;
}

bool FrMakeRoomDlg::IsBtnEnable()
{
	if (m_pMapSelectDlg == NULL && m_btnDelay < 0.0f)
		return true;
	return false;
}

void FrMakeRoomDlg::SetDefaultTitle()
{
	m_pTitle->SetLine(1, Doc()->GetDefaultTitle(m_gameType), 0, false, 0);
}
