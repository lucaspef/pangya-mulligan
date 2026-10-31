#include "minatl.h"
#include "changeroominfodlg.h"
#include "frbutton.h"
#include "frarea.h"
#include "fredit.h"
#include "frviewer.h"
#include "frstatic.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "treasuredlg.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "hatmanager.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "../shared/localize.h"
#include "s5/utilities.h" // PKT (frente C, lote 45): NotifyServiceStopped983
#include "artifactcheck983.h" // A s159: sArtifactCheck983 (00479bc0)
#include "jp/shareddocjp.h" // PKT (frente C, lote 45): IsControlServerService983, GetFixHole983
#include "msg983.h"
#include "jp/holeselector.h"
#include "titlefontex.h"
#include "wlocalize.h"
#include "vc10_layout.h"
#include "jp/docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#ifndef DOC983_U8
#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))
#endif


extern Fresh* g_pFresh;
extern CInputManager* g_input;
extern CSoundManager* g_audio;

IMPLEMENT_OBJECT(FrChangeRoomInfoDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrChangeRoomInfoDlg, FrForm)
ON_FRESH_VI("text", FRCMD_INIT, FrChangeRoomInfoDlg::OnTextInit)
ON_FRESH_VI("title", FRCMD_INIT, FrChangeRoomInfoDlg::OnTitleInit)
ON_FRESH_VV("title", FRCMD_ENTERKEY, FrChangeRoomInfoDlg::OnOkBtnUp)
ON_FRESH_VI("password", FRCMD_INIT, FrChangeRoomInfoDlg::OnPasswordInit)
ON_FRESH_VV("password", FRCMD_ENTERKEY, FrChangeRoomInfoDlg::OnOkBtnUp)
ON_FRESH_VI("map_ctrl.mapprev", FRCMD_INIT, FrChangeRoomInfoDlg::OnMapPrevInit)
ON_FRESH_VV("map_ctrl.mapprev", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnMapPrevUp)
ON_FRESH_VI("map_ctrl.mapnext", FRCMD_INIT, FrChangeRoomInfoDlg::OnMapNextInit)
ON_FRESH_VV("map_ctrl.mapnext", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnMapNextUp)
ON_FRESH_VI("gametype", FRCMD_INIT, FrChangeRoomInfoDlg::OnGameTypeInit)
ON_FRESH_VI("gametype", FRCMD_OWNERDRAW,
	FrChangeRoomInfoDlg::OnGameTypeOwnerDraw)
ON_FRESH_VI("userlimit", FRCMD_INIT, FrChangeRoomInfoDlg::OnUserLimitInit)
ON_FRESH_VI("userlimit", FRCMD_OWNERDRAW,
	FrChangeRoomInfoDlg::OnUserLimitOwnerDraw)
ON_FRESH_VI("hole", FRCMD_INIT, FrChangeRoomInfoDlg::OnHoleInit)
ON_FRESH_VI("hole", FRCMD_OWNERDRAW, FrChangeRoomInfoDlg::OnHoleOwnerDraw)
ON_FRESH_VI("timelimit", FRCMD_INIT, FrChangeRoomInfoDlg::OnTimeLimitInit)
ON_FRESH_VI("timelimit", FRCMD_OWNERDRAW,
	FrChangeRoomInfoDlg::OnTimeLimitOwnerDraw)
ON_FRESH_VI("map_ctrl.course", FRCMD_INIT, FrChangeRoomInfoDlg::OnCourseInit)
ON_FRESH_VV("map_ctrl.course", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnCourseUp)
ON_FRESH_VI("map_ctrl.course", FRCMD_OWNERDRAW, FrChangeRoomInfoDlg::OnCourseOwnerDraw)
ON_FRESH_VI("map_ctrl.holetype", FRCMD_INIT, FrChangeRoomInfoDlg::OnHoleTypeInit)
ON_FRESH_VI("map_ctrl.holetype", FRCMD_OWNERDRAW,
	FrChangeRoomInfoDlg::OnHoleTypeOwnerDraw)
ON_FRESH_VI("cover", FRCMD_INIT, FrChangeRoomInfoDlg::OnSingleCtrlInit983)
ON_FRESH_VI("single_ctrl", FRCMD_INIT, FrChangeRoomInfoDlg::OnSingleCtrlInit983)
ON_FRESH_VI("single_ctrl.repeat", FRCMD_INIT, FrChangeRoomInfoDlg::OnRepeatInit983)
ON_FRESH_VI("single_ctrl.repeat.fix_hole", FRCMD_INIT, FrChangeRoomInfoDlg::OnFixHoleInit983)
ON_FRESH_VV("single_ctrl.repeat.fix_hole", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnFixHoleUp983)
ON_FRESH_VV("single_ctrl.repeat.fix_hole", FRCMD_OWNERDRAW, FrChangeRoomInfoDlg::OnFixHoleOwnerDraw983)
ON_FRESH_VI("single_ctrl.repeat.fix_holecup_pos", FRCMD_INIT, FrChangeRoomInfoDlg::OnFixHolecupPosInit983)
ON_FRESH_VV("single_ctrl.repeat.fix_holecup_pos", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnFixHolecupPosUp983)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnOkBtnUp)
ON_FRESH_VI("map_ctrl", FRCMD_INIT, FrChangeRoomInfoDlg::OnMapCtrlInit983)
ON_FRESH_VI("map_ctrl.text_map", FRCMD_INIT, FrChangeRoomInfoDlg::OnTextMapInit983)
ON_FRESH_VI("artifact_ctrl", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtifactCtrlInit983)
ON_FRESH_VI("artifact_ctrl.arti_frame", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtiFrameInit983)
ON_FRESH_VI("artifact_ctrl.arti_icon", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtiIconInit983)
ON_FRESH_VI("artifact_ctrl.arti_ban", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtiBanInit983)
ON_FRESH_VI("artifact_ctrl.arti_name", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtiNameInit983)
ON_FRESH_VI("artifact_ctrl.arti_desc", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtiDescInit983)
ON_FRESH_VI("artifact_ctrl.arti_prev", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtiPrevInit983)
ON_FRESH_VV("artifact_ctrl.arti_prev", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnArtiPrevUp983)
ON_FRESH_VI("artifact_ctrl.arti_next", FRCMD_INIT, FrChangeRoomInfoDlg::OnArtiNextInit983)
ON_FRESH_VV("artifact_ctrl.arti_next", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnArtiNextUp983)
ON_FRESH_VI("classictype_btn", FRCMD_INIT, FrChangeRoomInfoDlg::OnClassicTypeInit983)
ON_FRESH_VV("classictype_btn", FRCMD_LBUTTONUP, FrChangeRoomInfoDlg::OnClassicTypeUp983)
END_FRESH_MSGMAP()

// 983 00cecfe0 (inicializador dinamico): 23 nomes por K2L em 00f030f8; DrawGameTypeButton 00acd130 indexa pelo tipo.
// No 983 o indice 5 e "short game" (d06804, nao o texto de 30 pessoas em equipe) e o shuffle fica no 18.
static const char* s_gameTypeName[23] = {
	K2L_Compatibility("\xbd\xba\xc6\xae\xb7\xce\xc5\xa9"),
	K2L_Compatibility("\xb8\xc5\xc4\xa1"),
	K2L_Compatibility("\xc3\xa4\xc6\xc3\xb9\xe6"),
	K2L_Compatibility("\xb7\xa1\xb4\xf5"),
	K2L_Compatibility("\xb4\xeb\xc8\xb8"),
	K2L_Compatibility("\xbc\xf4\xb0\xd4\xc0\xd3"),
	K2L_Compatibility("\xb1\xe6\xb5\xe5\xb4\xeb\xc0\xfc"),
	K2L_Compatibility("\xc6\xce\xb9\xe8\xc6\xb2"),
	"",
	"",
	K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"),
	K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xc0\xcf\xb9\xdd"),
	K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xc0\xcf\xb9\xdd\xbd\xcc\xb1\xdb"),
	K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xbb\xf3\xb1\xde"),
	K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xbb\xf3\xb1\xde\xbd\xcc\xb1\xdb"),
	"",
	"",
	"",
	K2L_Compatibility("\xbc\xc5\xc7\xc3\xc4\xda\xbd\xba"),
	K2L_Compatibility("\xbd\xcc\xb1\xdb\xc7\xc3\xb7\xb9\xc0\xcc"),
	NULL,
	NULL,
	NULL
};

extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)

// 983 00acae20: a CItemCursor983 do artefato (+0x1e0) parte da lista do CSharedDoc+0x375c
FrChangeRoomInfoDlg::FrChangeRoomInfoDlg()
	: m_artifact983(DOCPTR983(0x375c))
{
	C_ASSERT_VC10(sizeof(FrChangeRoomInfoDlg) == 0x2e0 && offsetof(FrChangeRoomInfoDlg, m_pRepeatHoleFont983) == 0x2d0); // 983: push 0x2e0 em 00acb783
	m_pTitle = NULL;
	m_pPassword = NULL;
	m_pText = NULL;
	m_pMapCtrl = NULL;
	m_pMapPrev = NULL;
	m_pMapNext = NULL;
	m_pGameType = NULL;
	m_pUserLimit = NULL;
	m_pHole = NULL;
	m_pTimeLimit = NULL;
	m_pCourse = NULL;
	m_pHoleType = NULL;
	m_pSingleCtrl983 = NULL; // 983 00acae20: +0x140..+0x178 zerados (SetRepeat983/ChangeGameType testam NULL)
	m_pRepeat983 = NULL;
	m_pFixHole983 = NULL;
	m_pFixHolecupPos983 = NULL;
	m_pMapCtrl983 = NULL;
	m_pTextMap983 = NULL;
	m_pArtifactCtrl983 = NULL;
	m_pArtiFrame983 = NULL;
	m_pArtiIcon983 = NULL;
	m_pArtiBan983 = NULL;
	m_pArtiName983 = NULL;
	m_pArtiDesc983 = NULL;
	m_pArtiPrev983 = NULL;
	m_pArtiNext983 = NULL;
	m_pClassicTypeBtn983 = NULL;
	m_pSelectBtnN =
		g_pFresh->GetManager()->GetBitmap("ITEMS", "select_button_n");
	m_pSelectBtnO =
		g_pFresh->GetManager()->GetBitmap("ITEMS", "select_button_o");
	m_pSelectBtnD =
		g_pFresh->GetManager()->GetBitmap("ITEMS", "select_button_d");
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
	const Bitmap* selectO =
		g_pFresh->GetManager()->GetBitmap("MAKEROOM", "map_select_o");
	m_pMapSelectDlg = NULL;
	m_pMapSelectO = selectO;
	// D720: o 983 (00acae8b/00acb0a3/00acb13c/00acb148) inicia estes 4; sem isso o OK mandava os campos 0xb/0xc com lixo
	m_bRepeat983 = false;
	m_pRepeatHoleFont983 = NULL;
	m_roomInfo = Doc()->m_roomInfo;
	m_fixHole983 = DOCJP()->GetFixHole983();
	m_bFixHolecupPos983 = false;
	int gt = m_roomInfo.gameType;
	m_hole = m_roomInfo.nHole;
	m_userLimit = m_roomInfo.nUserLimit;
	m_gameTypeGroup = 0;
	m_gameType = gt;
	m_gameTime = m_roomInfo.gameTimeLimit;
	m_shotTime = m_roomInfo.shotTimeLimit;
	m_course = m_roomInfo.mapType;
	m_holeType = m_roomInfo.holeType;
	m_artifact983.Find(&m_roomInfo.artifactTid983); // 983 00acb12c
	switch (gt)
	{
	case 0:
	case 1:
	case 7: // 983 00acb15b: o tipo 3 nao tem caso
		m_timeLimit = m_roomInfo.shotTimeLimit / 1000;
		break;
	default:
		goto done;
	case 4:
	case 5:
	case 6:
		m_timeLimit = m_roomInfo.gameTimeLimit / 60000;
		break;
	case 9:
	case 10:
		m_timeLimit = m_roomInfo.gameTimeLimit / 1000;
	}
done:
	m_btnDelay = 0.7f;
	SetRoomFlags983(m_roomInfo.roomFlags983); // 983 00acb1a8
}

void FrChangeRoomInfoDlg::ChangeUserLimit(int limit)
{
	m_userLimit = limit;
}

void FrChangeRoomInfoDlg::ChangeHole(int hole)
{
	// 983 (00acb7f0): Stroke/Match/7 so mexem no tipo de buraco com 18;
	// modo 4: 9 -> 15, 18 -> 35; modo 5: sempre 15; e todo caminho termina
	// em CheckArtifact983(true, 1).
	m_hole = hole;
	switch (m_gameType)
	{
	case 0:
	case 1:
	case 7:
		if (hole != 18)
			break;
	holeType:
		if (m_holeType == 1)
			m_holeType = 0;
		else if (m_holeType == 2)
			m_holeType = 0;
		break;
	case 4:
		if (hole == 9)
		{
			m_timeLimit = 15;
			break;
		}
		if (hole != 18)
			break;
		m_timeLimit = 35;
		goto holeType;
	case 5:
		m_timeLimit = 15;
		goto holeType;
	case 6:
		if (hole == 9)
		{
			m_timeLimit = 20;
			break;
		}
		if (hole != 18)
			break;
		m_timeLimit = 30;
		m_holeType = 0;
		break;
	default:
		break;
	}
	CheckArtifact983(true, 1);
}

void FrChangeRoomInfoDlg::ChangeTimeLimit(int time)
{
	m_timeLimit = time;
}

void FrChangeRoomInfoDlg::ChangeHoleType(eHoleType type)
{
	m_holeType = type;
}

void FrChangeRoomInfoDlg::OnProc(const float dt)
{
	FrForm::OnProc(dt);
	if (m_pMapSelectDlg == NULL && m_btnDelay >= 0.0f)
		m_btnDelay -= dt;
}

// 00acac50 (983): sala de buraco fixo/repeticao. Mostra single_ctrl e esconde titulo, senha, tipo de jogo e limite de
// jogadores; com o buraco fixo ligado, mostra repeat/fix_hole/fix_holecup_pos, guarda a posicao do copo e a fonte dos
// numeros e esconde o tipo de buraco. Imagem do botao de posicao do copo = 00ac9300 (single_font_on/off nos 3 modos).
void FrChangeRoomInfoDlg::SetRepeat983(bool bFixHole, unsigned char holecupPos, CRepeatHoleFont* pFont)
{
	// 983 (00acac50): um bloco por controle, sem tabela
	if (m_pSingleCtrl983)
	{
		m_pSingleCtrl983->Enable(true);
		m_pSingleCtrl983->SetVisible(true);
	}
	if (m_pTitle)
	{
		m_pTitle->Enable(false);
		m_pTitle->SetVisible(false);
	}
	if (m_pPassword)
	{
		m_pPassword->Enable(false);
		m_pPassword->SetVisible(false);
	}
	if (m_pGameType)
	{
		m_pGameType->Enable(false);
		m_pGameType->SetVisible(false);
	}
	if (m_pUserLimit)
	{
		m_pUserLimit->Enable(false);
		m_pUserLimit->SetVisible(false);
	}
	m_bRepeat983 = true;
	if (!bFixHole)
		return;
	m_pRepeatHoleFont983 = pFont;
	if (m_pRepeat983)
	{
		m_pRepeat983->Enable(true);
		m_pRepeat983->SetVisible(true);
	}
	if (m_pFixHole983)
	{
		m_pFixHole983->Enable(true);
		m_pFixHole983->SetVisible(true);
	}
	if (m_pFixHolecupPos983)
	{
		m_pFixHolecupPos983->Enable(true);
		m_pFixHolecupPos983->SetVisible(true);
		m_bFixHolecupPos983 = holecupPos != 0;
		RefreshFixHolecupPos983();
	}
	if (m_pHoleType)
	{
		m_pHoleType->Enable(false);
		m_pHoleType->SetVisible(false);
	}
}

bool FrChangeRoomInfoDlg::IsBtnEnable()
{
	if (m_pMapSelectDlg == NULL && m_btnDelay < 0.0f)
		return true;
	return false;
}

FrChangeRoomInfoDlg::~FrChangeRoomInfoDlg()
{
}

void FrChangeRoomInfoDlg::OnTextInit(int param)
{
	m_pText = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrChangeRoomInfoDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pTitle)
	{
		m_pTitle->SetCharLimit(20, false);
		m_pTitle->SetLine(1, m_roomInfo.title, 0, false, 0);
	}
}

void FrChangeRoomInfoDlg::OnPasswordInit(int param)
{
	m_pPassword = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pPassword)
	{
		// 983 (00acb210): a senha atual vem do "Lobby" (0x29 = M983(0x28), m_roomPassword) e aparece no campo;
		// o sRoomInfo do 983 nao guarda a senha
		const char* roomPassword = "";
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x28),
			(int)&roomPassword, 0, 0, 0, 0);
		m_pPassword->SetCharLimit(7, false);
		m_pPassword->SetLine(1, roomPassword, 0, false, 0);
	}
}

void FrChangeRoomInfoDlg::OnMapPrevInit(int param)
{
	m_pMapPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMapPrev)
	{
		m_pMapPrev->SetPushDelay(0.0f);
		m_pMapPrev->SetVisible(true);
		m_pMapPrev->UseDblClick(false); // 983 00ac95a0: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrChangeRoomInfoDlg::OnMapNextInit(int param)
{
	m_pMapNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMapNext)
	{
		m_pMapNext->SetPushDelay(0.0f);
		m_pMapNext->SetVisible(true);
		m_pMapNext->UseDblClick(false); // 983 00ac9640: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrChangeRoomInfoDlg::OnGameTypeInit(int param)
{
	m_pGameType = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrChangeRoomInfoDlg::OnUserLimitInit(int param)
{
	m_pUserLimit = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrChangeRoomInfoDlg::DrawUserLimitButton(FrGraphicInterface* gi, int limit,
	WRect rect)
{
	const Bitmap* bitmap = m_pSelectBtnN;
	if (m_userLimit == limit)
		bitmap = m_pSelectBtnD;
	else
	{
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (IsBtnEnable() && rect.IsInRect(mouse))
		{
			if (g_input->GetButton(LEFT_BUTTON) == 2)
				m_userLimit = limit;
			bitmap = m_pSelectBtnO;
		}
	}
	gi->DrawTexture(bitmap, rect, 0xffffffff, 0);
	WPoint pos(rect.w * 0.5f + rect.x, rect.y + 5.0f);
	gi->PrintText11(pos, 1, MakeStr("%d", limit), NULL);
}

void FrChangeRoomInfoDlg::OnHoleInit(int param)
{
	m_hole = m_roomInfo.nHole;
	m_pHole = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrChangeRoomInfoDlg::DrawHoleButton(FrGraphicInterface* gi, int hole,
	WRect rect)
{
	const Bitmap* bitmap = m_pSelectBtnN;
	if (m_hole == hole)
		bitmap = m_pSelectBtnD;
	else
	{
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (IsBtnEnable() && rect.IsInRect(mouse))
		{
			if (g_input->GetButton(LEFT_BUTTON) == 2)
				ChangeHole(hole);
			bitmap = m_pSelectBtnO;
		}
	}
	gi->DrawTexture(bitmap, rect, 0xffffffff, 0);
	WPoint pos(rect.w * 0.5f + rect.x, rect.y + 5.0f);
	gi->PrintText11(pos, 1, MakeStr("%d", hole), NULL);
}

void FrChangeRoomInfoDlg::OnHoleOwnerDraw(int param)
{
	// 983 (00acbbf0): parte = w * 0.25 / w / 3.0 / w * 0.5 e o ultimo botao em x + (parte + parte) ou
	// x + parte * 3.0 (o 645 fazia w * 2/3 + x); todos os WRect de cada case montados antes das chamadas,
	// o compilador junta os rabos como no 983
	FrGraphicInterface* gi = g_pFresh->GetManager()->GetGDI();
	if (gi == NULL)
		return;
	WRect rc = m_pHole->GetRect(); // copia
	float w;
	switch (m_gameType)
	{
	case 0:
	{
		w = (float)(rc.w * 0.25);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h),
			r3((float)(w * 3.0 + rc.x), rc.y, w, rc.h);
		DrawHoleButton(gi, 3, r0);
		DrawHoleButton(gi, 6, r1);
		DrawHoleButton(gi, 9, r2);
		DrawHoleButton(gi, 18, r3);
		break;
	}
	case 1:
	case 7:
	{
		w = (float)(rc.w / 3.0);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h);
		DrawHoleButton(gi, 6, r0);
		DrawHoleButton(gi, 9, r1);
		DrawHoleButton(gi, 18, r2);
		break;
	}
	case 4:
	case 5:
	case 6:
	{
		w = (float)(rc.w * 0.5);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h);
		DrawHoleButton(gi, 9, r0);
		DrawHoleButton(gi, 18, r1);
		break;
	}
	case 9:
	case 10:
	{
		w = (float)(rc.w / 3.0);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h);
		DrawHoleButton(gi, 3, r0);
		DrawHoleButton(gi, 6, r1);
		DrawHoleButton(gi, 9, r2);
		break;
	}
	}
}

void FrChangeRoomInfoDlg::OnTimeLimitInit(int param)
{
	m_pTimeLimit = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrChangeRoomInfoDlg::DrawTimeLimitButton(FrGraphicInterface* gi, int time,
	WRect rect)
{
	const Bitmap* bitmap = m_pSelectBtnN;
	if (m_timeLimit == time)
		bitmap = m_pSelectBtnD;
	else
	{
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (IsBtnEnable() && rect.IsInRect(mouse))
		{
			if (g_input->GetButton(LEFT_BUTTON) == 2)
				m_timeLimit = time;
			bitmap = m_pSelectBtnO;
		}
	}
	gi->DrawTexture(bitmap, rect, 0xffffffff, 0);
	WPoint pos(rect.w * 0.5f + rect.x, rect.y + 5.0f);
	gi->PrintText11(pos, 1, MakeStr("%d", time), NULL);
}

void FrChangeRoomInfoDlg::OnTimeLimitOwnerDraw(int param)
{
	// 983 (00ac9a50): 0/1/7 = 40..300; 4 = 9 buracos 15..30 / 18 buracos 35..55; 5 = 15..35; 9/10 = 40 na largura
	// inteira do controle (o nosso usava w sem valor). parte = w * 0.25 ou w / 5.0 (double); todos os WRect de cada
	// case montados antes das chamadas, o compilador junta os rabos como no 983
	FrGraphicInterface* gi = g_pFresh->GetManager()->GetGDI();
	if (gi == NULL)
		return;
	WRect rc = m_pTimeLimit->GetRect();
	float w;
	switch (m_gameType)
	{
	case 0:
	case 1:
	case 7:
	{
		w = (float)(rc.w * 0.25);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h),
			r3((float)(w * 3.0 + rc.x), rc.y, w, rc.h);
		DrawTimeLimitButton(gi, 40, r0);
		DrawTimeLimitButton(gi, 60, r1);
		DrawTimeLimitButton(gi, 120, r2);
		DrawTimeLimitButton(gi, 300, r3);
		break;
	}
	case 4:
		if (m_hole == 9)
		{
			w = (float)(rc.w * 0.25);
			float lastX = (float)(w * 3.0);
			WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h),
				r3(lastX + rc.x, rc.y, w, rc.h);
			DrawTimeLimitButton(gi, 15, r0);
			DrawTimeLimitButton(gi, 20, r1);
			DrawTimeLimitButton(gi, 25, r2);
			DrawTimeLimitButton(gi, 30, r3);
		}
		else if (m_hole == 18)
		{
			w = (float)(rc.w / 5.0);
			WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h),
				r3((float)(w * 3.0 + rc.x), rc.y, w, rc.h), r4((float)(w * 4.0 + rc.x), rc.y, w, rc.h);
			DrawTimeLimitButton(gi, 35, r0);
			DrawTimeLimitButton(gi, 40, r1);
			DrawTimeLimitButton(gi, 45, r2);
			DrawTimeLimitButton(gi, 50, r3);
			DrawTimeLimitButton(gi, 55, r4);
		}
		break;
	case 5:
	{
		w = (float)(rc.w / 5.0);
		float lastX = (float)(w * 4.0);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h),
			r3((float)(w * 3.0 + rc.x), rc.y, w, rc.h), r4(lastX + rc.x, rc.y, w, rc.h);
		DrawTimeLimitButton(gi, 15, r0);
		DrawTimeLimitButton(gi, 20, r1);
		DrawTimeLimitButton(gi, 25, r2);
		DrawTimeLimitButton(gi, 30, r3);
		DrawTimeLimitButton(gi, 35, r4);
		break;
	}
	case 9:
	case 10:
		DrawTimeLimitButton(gi, 40, rc);
		break;
	}
}

void FrChangeRoomInfoDlg::OnCourseInit(int param)
{
	m_pCourse = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void FrChangeRoomInfoDlg::OnCourseOwnerDraw(int param)
{
	if (m_gameType != 10 && m_gameType != 6 && m_pCourse && m_pMapSelectO)
	{
		WRect rect = m_pCourse->GetRect();
		FrWndManager* manager = g_pFresh->GetManager();
		if (rect.IsInRect(manager->GetMousePos()))
		{
			FrGraphicInterface* gi = manager->GetGDI();
			if (gi)
			{
				WRect dst(rect.x - 25.0f, rect.y - 15.0f,
					(float)m_pMapSelectO->Width(),
					(float)m_pMapSelectO->Height());
				gi->DrawTexture(m_pMapSelectO, dst, 0xffffffff, 0);
			}
		}
	}
}

void FrChangeRoomInfoDlg::OnHoleTypeInit(int param)
{
	m_pHoleType = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrChangeRoomInfoDlg::DrawHoleTypeButton(FrGraphicInterface* gi,
	eHoleType type, WRect rect, bool bEnable)
{
	const Bitmap* bitmap = m_pHoleTypeBtn[type][0];
	unsigned long color = 0xffffffff;
	if (bEnable)
	{
		if (m_holeType == type)
			bitmap = m_pHoleTypeBtn[type][1];
		else if (IsBtnEnable())
		{
			if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				if (g_input->GetButton(LEFT_BUTTON) == 2)
					m_holeType = type;
			}
		}
	}
	else
		color = 0xff909090;
	gi->DrawTexture(bitmap, rect, color, 0);
}

void FrChangeRoomInfoDlg::OnHoleTypeOwnerDraw(int param)
{
	FrGraphicInterface* gi = g_pFresh->GetManager()->GetGDI();
	if (gi)
	{
		// 983 00aca110: retangulos dos botoes 0..2 montados antes, com um x corrente somado pela largura de cada botao
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
		if (m_gameType == 10)
		{
			DrawHoleTypeButton(gi, (eHoleType)0, rc0, false);
			DrawHoleTypeButton(gi, (eHoleType)1, rc1, false);
			DrawHoleTypeButton(gi, (eHoleType)2, rc2, false);
			DrawHoleTypeButton(gi, (eHoleType)3, WRect(x, base.y, w, h), false);
		}
		else
		{
			// 983 00aca110: com 18 buracos so os botoes 1 e 2 desligam; o 3 fica ligado
			if (m_hole == 18)
			{
				DrawHoleTypeButton(gi, (eHoleType)0, rc0, true);
				DrawHoleTypeButton(gi, (eHoleType)1, rc1, false);
				DrawHoleTypeButton(gi, (eHoleType)2, rc2, false);
			}
			else
			{
				DrawHoleTypeButton(gi, (eHoleType)0, rc0, true);
				DrawHoleTypeButton(gi, (eHoleType)1, rc1, true);
				DrawHoleTypeButton(gi, (eHoleType)2, rc2, true);
			}
			DrawHoleTypeButton(gi, (eHoleType)3, WRect(x, base.y, w, h), true);
		}
	}
}

// 983 00500340 (ecx = sRoomInfo): sala de GM (bAdminMaster, fora do tipo 11) com 100+ vagas
static __declspec(noinline) bool __fastcall IsGMHundredRoom983(const sRoomInfo* room)
{
	return room->gameType != 11 && room->bAdminMaster && room->nUserLimit > 99;
}

void FrChangeRoomInfoDlg::OnUserLimitOwnerDraw(int param)
{
	// 983 (00acb300): copia do retangulo do botao, terco = w / 3.0; os WRect de cada case sao montados antes das chamadas como no 983;
	// 2/5/6 e 10 so trocam o 1o valor (10/6)
	FrGraphicInterface* gi = g_pFresh->GetManager()->GetGDI();
	if (gi == NULL)
		return;
	WRect rc = m_pUserLimit->GetRect();
	float w;
	int first;
	switch (m_gameType)
	{
	case 0:
	{
		w = (float)(rc.w / 3.0);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h);
		DrawUserLimitButton(gi, 2, r0);
		DrawUserLimitButton(gi, 3, r1);
		DrawUserLimitButton(gi, 4, r2);
		break;
	}
	case 1:
	case 7:
	{
		w = (float)(rc.w * 0.5);
		WRect r0(rc.x, rc.y, w, rc.h), r1(w + rc.x, rc.y, w, rc.h);
		DrawUserLimitButton(gi, 2, r0);
		DrawUserLimitButton(gi, 4, r1);
		break;
	}
	case 4:
		if (IsGMHundredRoom983(&Doc()->m_roomInfo)) // um botao so
		{
			DrawUserLimitButton(gi, Doc()->m_roomInfo.nUserLimit, rc);
			return;
		}
	case 2:
	case 5:
	case 6:
		first = 10;
		goto thirds;
	case 10:
		first = 6;
	thirds:
	{
		w = (float)(rc.w / 3.0);
		WRect r0(rc.x, rc.y, w, rc.h), r1(rc.x + w, rc.y, w, rc.h), r2(w + w + rc.x, rc.y, w, rc.h);
		DrawUserLimitButton(gi, first, r0);
		DrawUserLimitButton(gi, 20, r1);
		DrawUserLimitButton(gi, 30, r2);
		break;
	}
	}
}

// PKT (frente C, lote 45): 983 00acc1d0 - pacote 0xa com os campos novos do 983 (0xe sinalizadores, 0xd artefato, 0xb buraco fixo,
// 0xc copo fixo), senha pedida ao "Lobby" (a sRoomInfo do 983 nao tem senha), titulo cortado em 20 e senha em 8, avisos com 0x803.
void FrChangeRoomInfoDlg::OnOkBtnUp()
{
	if (m_roomFlags983 & 2)
		m_gameType = 4;
	if (DOCJP()->IsControlServerService983(0x20000000) && (m_roomFlags983 & 1))
	{
		S5::UTIL::NotifyServiceStopped983(0x20000000);
		return;
	}
	if (m_roomInfo.nUserNum > 1 &&
		!(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
	{
		std::list<sSlotInfo>::iterator it;
		for (it = Doc()->m_slotList.begin(); it != Doc()->m_slotList.end();
			it++)
		{
			if ((*it).bMaster == 0 && (*it).bReady == 0)
				break;
		}
		if (it == Doc()->m_slotList.end())
		{
			OnFreshOkay();
			return;
		}
	}
	std::string title(m_pTitle->GetLine(1, false));
	FrForm* pForm;
	if (title.empty() || title.find_first_not_of(' ', 0) == std::string::npos)
	{
		pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(K2L_Compatibility(
			"\xb9\xe6 \xc1\xa6\xb8\xf1\xc0\xbb \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4"),
			false);
		pForm->Open(NULL, 0x803);
	}
	else if (Doc()->m_chatManager.Filtering((char*)title.c_str()))
	{
		pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(K2L_Compatibility(
			"\xb9\xe6 \xc1\xa6\xb8\xf1\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb9\xae\xc0\xda\xb8\xa6 \xc6\xf7\xc7\xd4\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 0x803);
	}
	else
	{
		if (CheckArtifact983(false, 2))
			return;
		if (IsMassGame(m_gameType))
		{
			m_shotTime = 0;
			if (m_gameType != 10)
				m_gameTime = m_timeLimit * 60000;
			else
				m_gameTime = m_timeLimit * 1000;
		}
		else
		{
			m_shotTime = m_timeLimit * 1000;
			m_gameTime = 0;
		}
		const char* roomPassword = "";
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x28),
			(int)&roomPassword, 0, 0, 0, 0);
		unsigned char changed = 0;
		if (m_gameType != m_roomInfo.gameType)
			changed = 1;
		if (m_hole != m_roomInfo.nHole)
			changed++;
		if (m_shotTime != m_roomInfo.shotTimeLimit)
			changed++;
		if (m_gameTime != m_roomInfo.gameTimeLimit)
			changed++;
		if (m_userLimit != m_roomInfo.nUserLimit)
			changed++;
		if (strcmp(m_roomInfo.title, m_pTitle->GetLine(1, false)) != 0)
			changed++;
		if (strcmp(roomPassword, m_pPassword->GetLine(1, false)) != 0)
			changed++;
		if (m_holeType != m_roomInfo.holeType)
			changed++;
		if (m_roomFlags983 != m_roomInfo.roomFlags983)
			changed++;
		if (m_course != m_roomInfo.mapType)
			changed++;
		if (*m_artifact983.GetCur() != m_roomInfo.artifactTid983)
			changed++;
		if (m_fixHole983 != DOCJP()->GetFixHole983())
			changed++;
		if (m_pFixHolecupPos983 && DOC983_U8(0x11609) != m_bFixHolecupPos983)
			changed++;
		if (changed != 0)
		{
			if (Doc()->m_curChannel.Type & 2)
				changed++;
			char text[40] = {0};
			WSendPacket packet(10);
			packet.Encode2(0xffff);
			packet.Encode1(changed);
			if (strcmp(m_roomInfo.title, m_pTitle->GetLine(1, false)) != 0)
			{
				packet.Encode1(0);
				memset(text, 0, sizeof(text));
				strncpy(text, m_pTitle->GetLine(1, false), sizeof(text));
				text[20] = 0;
				packet.EncodeStr(std::string(text));
			}
			if (strcmp(roomPassword, m_pPassword->GetLine(1, false)) != 0)
			{
				std::string password("");
				packet.Encode1(1);
				memset(text, 0, sizeof(text));
				strncpy(text, m_pPassword->GetLine(1, false), sizeof(text));
				text[8] = 0;
				packet.EncodeStr(std::string(text));
				password = m_pPassword->GetLine(1, false);
				FrChangeRoomInfoInvalidPwd result =
					std::for_each(password.begin(), password.end(),
						FrChangeRoomInfoInvalidPwd());
				if (result.bInvalid)
				{
					pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
						"notify", NULL);
					pForm->SetMessage(K2L_Compatibility(
						"\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xbf\xa1 \xb0\xf8\xb9\xe9\xc0\xba \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						false);
					pForm->Open(NULL, 0x803);
					return;
				}
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x27),
					(int)m_pPassword->GetLine(1, false), 0, 0, 0, 0);
			}
			if (m_hole != m_roomInfo.nHole)
			{
				packet.Encode1(4);
				packet.Encode1(m_hole);
			}
			if (m_roomFlags983 != m_roomInfo.roomFlags983)
			{
				packet.Encode1(0xe);
				packet.Encode4(m_roomFlags983);
			}
			if (m_shotTime != m_roomInfo.shotTimeLimit)
			{
				packet.Encode1(6);
				packet.Encode2(m_shotTime / 1000);
			}
			if (m_gameTime != m_roomInfo.gameTimeLimit)
			{
				packet.Encode1(8);
				if (m_gameType == 10)
					packet.Encode1(m_gameTime / 1000);
				else
					packet.Encode1(m_gameTime / 60000);
			}
			if (m_userLimit != m_roomInfo.nUserLimit)
			{
				packet.Encode1(7);
				packet.Encode1(m_userLimit);
			}
			if (m_gameType != m_roomInfo.gameType)
			{
				packet.Encode1(2);
				packet.Encode1(m_gameType);
			}
			if (Doc()->m_curChannel.Type & 2)
			{
				packet.Encode1(5);
				packet.Encode1(m_roomInfo.holeType);
			}
			if (m_holeType != m_roomInfo.holeType)
			{
				packet.Encode1(5);
				packet.Encode1(m_holeType);
			}
			if (m_course != m_roomInfo.mapType)
			{
				packet.Encode1(3);
				packet.Encode1(m_course);
			}
			if (*m_artifact983.GetCur() != m_roomInfo.artifactTid983)
			{
				packet.Encode1(0xd);
				packet.Encode4(*m_artifact983.GetCur());
			}
			if (m_fixHole983 != DOCJP()->GetFixHole983())
			{
				packet.Encode1(0xb);
				packet.Encode1(m_fixHole983);
			}
			if (DOC983_U8(0x11609) != m_bFixHolecupPos983)
			{
				packet.Encode1(0xc);
				if (m_bFixHolecupPos983)
					packet.Encode4(7);
				else
					packet.Encode4(6);
			}
			packet.Send(TO_GAME);
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		}
		OnFreshOkay();
	}
}

void FrChangeRoomInfoDlg::ChangeCourse(int course)
{
	unsigned int mapId;
	if (course > 0x7f && course != 0xfd)
		mapId = course - 0x80;
	else
		mapId = course;
	IFF_STRUCT::sCourse* pCourse =
		ItemManager()->FindCourse(mapId | 0x28000000);
	if (pCourse)
	{
		if (course != 0x7f)
			m_pCourse->Open(MakeStr("small_%s.tga", pCourse->c.Icon));
		else
			m_pCourse->Open("map_random_middle.tga");
		m_course = course;
		if (m_pMapPrev)
			m_pMapPrev->Enable(m_course != FirstMap());
		bool bAll = Doc()->m_roomInfo.holeType == 4; // 983 00acb8e0: sete antes do teste do botao
		if (m_pMapNext)
			m_pMapNext->Enable(m_course != LastMap(bAll));
	}
}

void FrChangeRoomInfoDlg::OnMapPrevUp()
{
	if (m_pMapPrev)
		ChangeCourse(PrevMap(m_course));
}

void FrChangeRoomInfoDlg::OnMapNextUp()
{
	if (m_pMapNext)
		ChangeCourse(NextMap(m_course, m_gameType == 2));
}

bool FrChangeRoomInfoDlg::OnMapSelectDlgResult(int result, FrForm* form)
{
	unsigned char map = 0;
	if (form)
		map = ((FrTreasureCourse*)form)->GetCurMapIdx();
	ChangeCourse(map);
	m_pMapSelectDlg = NULL;
	m_btnDelay = 0.7f;
	return true;
}

void FrChangeRoomInfoDlg::OnCourseUp()
{
	if (m_gameType != 10 && m_gameType != 6)
	{
		g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		m_pMapSelectDlg = CreateForm<FrTreasureCourse>(g_pFresh->GetManager(),
			this, "treasuremap", NULL);
		if (m_pMapSelectDlg)
		{
			m_pMapSelectDlg->SetSelectMap(m_course);
			m_pMapSelectDlg->SetMapIndexChanged(false);
			// 983 00acbf6c: sem aleatorio (+0x120) na sala de repeticao (holeType 4)
			m_pMapSelectDlg->EnableRandomBtn(Doc()->m_roomInfo.holeType != 4);
			m_pMapSelectDlg->EnableDrag(false);
			m_pMapSelectDlg->Open(
				(FRESH_PFN_RESULT)&FrChangeRoomInfoDlg::OnMapSelectDlgResult,
				3);
		}
	}
}

// 983 00aca660: "map_ctrl" (+0x150); guarda o deslocamento do controle em relacao ao dialogo (00f030e0/e4)
static float g_mapCtrlOfsX983, g_mapCtrlOfsY983;

// 983 00aca360/00aca3f0/00aca480/00aca5b0: guardam o controle (+0x140..+0x14c) e o deixam
// desligado e escondido; SetRepeat983 os mostra nas salas de repeticao.
#define HIDE_CTRL983(m) \
	if (m) \
	{ \
		(m)->Enable(false); \
		(m)->SetVisible(false); \
	}

void FrChangeRoomInfoDlg::OnSingleCtrlInit983(int param)
{
	m_pSingleCtrl983 = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
	HIDE_CTRL983(m_pSingleCtrl983)
}

void FrChangeRoomInfoDlg::OnRepeatInit983(int param)
{
	m_pRepeat983 = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
	HIDE_CTRL983(m_pRepeat983)
}

void FrChangeRoomInfoDlg::OnFixHoleInit983(int param)
{
	m_pFixHole983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	HIDE_CTRL983(m_pFixHole983)
}

void FrChangeRoomInfoDlg::OnFixHolecupPosInit983(int param)
{
	m_pFixHolecupPos983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	HIDE_CTRL983(m_pFixHolecupPos983)
}

// 00acb550: abre "hole_selector" logo abaixo do botao (course = +0x1c4, buraco = +0x1d8)
void FrChangeRoomInfoDlg::OnFixHoleUp983()
{
	if (!m_pFixHole983)
		return;
	FrHoleSelector* pSel = CreateForm<FrHoleSelector>(g_pFresh->GetManager(), this, "hole_selector", NULL);
	if (!pSel)
		return;
	float h = (float)(m_pFixHole983->GetRect().h + 3.0);
	WPoint btn;
	btn.x = m_pFixHole983->GetRect().x;
	btn.y = m_pFixHole983->GetRect().y;
	WPoint pos;
	pos.x = (float)(btn.x - 126.0);
	pos.y = h + btn.y;
	unsigned char course = *(unsigned char*)&m_course;
	pSel->EnableDrag(false);
	pSel->SetCourse983(course, m_fixHole983);
	pSel->Open((FRESH_PFN_RESULT)&FrChangeRoomInfoDlg::OnHoleSelected983, pos, FrESCAPE | FrNOTAIL | FrRET_CANCEL);
}

// 00acadc0: guarda o buraco escolhido (o envio ao servidor sai no OnOkBtnUp)
bool FrChangeRoomInfoDlg::OnHoleSelected983(int ret, FrForm* pForm)
{
	if (ret == FrOK)
	{
		FrHoleSelector* pSel = DYNAMIC_CAST(FrHoleSelector, pForm);
		if (pSel && pSel->GetHole983() != m_fixHole983)
			m_fixHole983 = pSel->GetHole983();
	}
	return true;
}

// 00aca510: numero do buraco fixo no botao ("!" = fonte das dezenas; formatos 00e37974)
void FrChangeRoomInfoDlg::OnFixHoleOwnerDraw983()
{
	if (!m_pFixHole983 || m_fixHole983 > 0x12 || !m_pRepeatHoleFont983)
		return;
	const char* text = MakeStr(m_fixHole983 < 10 ? "%d" : "!%d", (unsigned char)(m_fixHole983 % 10));
	m_pRepeatHoleFont983->Print((float)(m_pFixHole983->GetRect().w * 0.5 + m_pFixHole983->GetRect().x),
		(float)(m_pFixHole983->GetRect().y + 4.0), text, TFA_CENTER, 0.0f, 1.0f);
}

void FrChangeRoomInfoDlg::OnFixHolecupPosUp983()
{
	if (!m_pFixHolecupPos983)
		return;
	m_bFixHolecupPos983 = !m_bFixHolecupPos983;
	RefreshFixHolecupPos983();
}

void FrChangeRoomInfoDlg::RefreshFixHolecupPos983()
{
	if (!m_pFixHolecupPos983)
		return;
	FrButton* pBtn = (FrButton*)m_pFixHolecupPos983;
	if (m_bFixHolecupPos983)
	{
		pBtn->SetButtonImg("single_font_on", (FrButton::eButMode)0);
		((FrButton*)m_pFixHolecupPos983)->SetButtonImg("single_font_on", (FrButton::eButMode)1);
		((FrButton*)m_pFixHolecupPos983)->SetButtonImg("single_font_on", (FrButton::eButMode)2);
	}
	else
	{
		pBtn->SetButtonImg("single_font_off", (FrButton::eButMode)0);
		((FrButton*)m_pFixHolecupPos983)->SetButtonImg("single_font_off", (FrButton::eButMode)1);
		((FrButton*)m_pFixHolecupPos983)->SetButtonImg("single_font_off", (FrButton::eButMode)2);
	}
}

void FrChangeRoomInfoDlg::OnMapCtrlInit983(int param)
{
	m_pMapCtrl983 = DYNAMIC_CAST(FrWnd, (FrWnd*)param);
	if (m_pMapCtrl983)
	{
		g_mapCtrlOfsX983 = m_pMapCtrl983->GetRect().x - GetRect().x;
		g_mapCtrlOfsY983 = m_pMapCtrl983->GetRect().y - GetRect().y;
	}
}

void FrChangeRoomInfoDlg::OnTextMapInit983(int param)
{
	m_pTextMap983 = DYNAMIC_CAST(FrArea, (FrWnd*)param); // 983 00aca710 (+0x154)
}

void FrChangeRoomInfoDlg::OnArtifactCtrlInit983(int param)
{
	m_pArtifactCtrl983 = DYNAMIC_CAST(FrWnd, (FrWnd*)param); // 983 00aca780 (+0x158)
}

void FrChangeRoomInfoDlg::OnArtiFrameInit983(int param)
{
	m_pArtiFrame983 = DYNAMIC_CAST(FrArea, (FrWnd*)param); // 983 00aca7f0 (+0x15c)
}

void FrChangeRoomInfoDlg::OnArtiIconInit983(int param)
{
	m_pArtiIcon983 = DYNAMIC_CAST(FrArea, (FrWnd*)param); // 983 00aca860 (+0x160)
}

void FrChangeRoomInfoDlg::OnArtiBanInit983(int param)
{
	m_pArtiBan983 = DYNAMIC_CAST(FrArea, (FrWnd*)param); // 983 00aca8d0 (+0x164)
}

void FrChangeRoomInfoDlg::OnArtiNameInit983(int param)
{
	m_pArtiName983 = DYNAMIC_CAST(FrStatic, (FrWnd*)param); // 983 00aca940 (+0x168)
}

void FrChangeRoomInfoDlg::OnArtiDescInit983(int param)
{
	m_pArtiDesc983 = DYNAMIC_CAST(FrStatic, (FrWnd*)param); // 983 00aca9b0 (+0x16c)
}

// 983 00acaa20/00acaab0: botoes sem atraso e sem duplo clique
void FrChangeRoomInfoDlg::OnArtiPrevInit983(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pArtiPrev983 = pBtn;
	if (pBtn)
	{
		pBtn->SetPushDelay(0.0f);
		pBtn->UseDblClick(false);
	}
}

void FrChangeRoomInfoDlg::OnArtiNextInit983(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pArtiNext983 = pBtn;
	if (pBtn)
	{
		pBtn->SetPushDelay(0.0f);
		pBtn->UseDblClick(false);
	}
}

// 983 00acbfe0
void FrChangeRoomInfoDlg::OnArtiPrevUp983()
{
	if (m_pArtiPrev983)
	{
		m_pArtiPrev983->Enable(m_artifact983.StepPrev983());
		m_pArtiNext983->Enable(m_artifact983.CanNext983());
		RefreshArtifact983();
	}
}

// 983 00acc040
void FrChangeRoomInfoDlg::OnArtiNextUp983()
{
	if (m_pArtiNext983)
	{
		m_pArtiNext983->Enable(m_artifact983.StepNext983());
		m_pArtiPrev983->Enable(m_artifact983.CanPrev983());
		RefreshArtifact983();
	}
}

// 983 00ac9270
void FrChangeRoomInfoDlg::SetRoomFlags983(unsigned long flags)
{
	m_roomFlags983 = flags;
	if (m_pClassicTypeBtn983)
		((FrButton*)m_pClassicTypeBtn983)->SetStatus((FrButton::eButMode)((flags & 1) * 2));
}

// 983 00acab40: nas salas 12/14/19 o botao vai para a altura do "hole"
void FrChangeRoomInfoDlg::OnClassicTypeInit983(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pClassicTypeBtn983 = pBtn;
	if (pBtn)
	{
		switch (m_roomInfo.realGameType)
		{
		case 12:
		case 14:
		case 19:
			{
				WRect rc = pBtn->GetRect();
				rc.y = m_pHole->GetRect().y;
				pBtn->SetRect(rc);
			}
			break;
		}
		((FrButton*)m_pClassicTypeBtn983)->SetStatus((FrButton::eButMode)((m_roomFlags983 & 1) * 2));
	}
}

// 983 00acac20: o botao alterna o bit 0 (classico)
void FrChangeRoomInfoDlg::OnClassicTypeUp983()
{
	FrButton* pBtn = (FrButton*)m_pClassicTypeBtn983;
	if (pBtn)
	{
		unsigned long flags = m_roomFlags983;
		if (pBtn->GetStatus() == FrButton::PRESSED)
			SetRoomFlags983(flags | 1);
		else
			SetRoomFlags983(flags & ~1);
	}
}

// 983 00ac9380: na sala de repeticao o artefato nao aparece
bool FrChangeRoomInfoDlg::ArtifactShow983(bool bShow)
{
	return m_bRepeat983 ? false : bShow;
}

// 983 00acb9c0: icone/nome/descricao do artefato do cursor; "no_artifact" quando nao ha
void FrChangeRoomInfoDlg::RefreshArtifact983()
{
	const char* pName = K2L_Compatibility("\xbb\xe7\xbf\xeb\xbe\xc8\xc7\xd4"); // "Not In Use"
	const char* pDesc = "";
	const Bitmap* pIcon = g_pFresh->GetBitmap("no_artifact");

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
	if (m_pArtiIcon983)
		((FrArea*)m_pArtiIcon983)->SetBgImg(pIcon);
	CheckArtifact983(true, 0);
	if (m_pArtiName983)
		((FrStatic*)m_pArtiName983)->SetCaption(pName);
	if (m_pArtiDesc983)
		((FrStatic*)m_pArtiDesc983)->SetCaption(pDesc);
}

// 983 00acc0a0: cursor no item tid, ajusta prev/next e redesenha
void FrChangeRoomInfoDlg::RefreshArtiButtons983(unsigned long tid)
{
	m_artifact983.Find(&tid);
	if (m_pArtiPrev983)
		m_pArtiPrev983->Enable(m_artifact983.CanPrev983());
	if (m_pArtiNext983)
		m_pArtiNext983->Enable(m_artifact983.CanNext983());
	RefreshArtifact983();
}

// 983 00acc120: mostra/esconde o map_ctrl (na posicao do dialogo + deslocamento) e a area do artefato
void FrChangeRoomInfoDlg::SetArtifactArea983(bool bShow, bool bArtifact)
{
	if (m_pMapCtrl983)
	{
		m_pMapCtrl983->SetVisible(bShow);
		if (bShow)
		{
			WPoint pt(g_mapCtrlOfsX983 + GetRect().x, g_mapCtrlOfsY983 + GetRect().y);
			if (!bArtifact)
			{
				pt.x += 65.0f; // 983 00e37388/8c
				pt.y += -16.0f;
			}
			m_pMapCtrl983->MoveWindow(pt);
		}
	}
	if (m_pArtifactCtrl983)
		m_pArtifactCtrl983->SetVisible(bArtifact);
	if (!bArtifact)
		RefreshArtiButtons983(0);
}

// 983 00accca0. Ao sair do tipo 10 o 983 trava o curso (0x7f) e volta sem gravar o tipo novo.
void FrChangeRoomInfoDlg::ChangeGameType(int type, bool bUnused,
	bool bKeepValues)
{
	if (m_gameType == 2 && type != 2)
	{
		m_pMapCtrl->SetVisible(true);
		m_pMapPrev->SetVisible(true);
		m_pMapNext->SetVisible(true);
		m_pHoleType->SetVisible(true);
		m_pCourse->SetVisible(true);
	}
	else if (m_gameType != 2 && type == 2)
	{
		m_pMapCtrl->SetVisible(false);
		m_pMapPrev->SetVisible(false);
		m_pMapNext->SetVisible(false);
		m_pHoleType->SetVisible(false);
		m_pCourse->SetVisible(false);
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
		m_pMapNext->Enable(false);
	}
	else if (m_gameType == 10 && type != 10)
	{
		m_pCourse->SetMouseEvent(false);
		ChangeCourse(0x7f);
		m_pMapPrev->Enable(false);
		m_pMapNext->Enable(false);
		return;
	}
	else if (m_gameType != 10 && type == 10)
	{
		m_pCourse->SetMouseEvent(false);
		ChangeCourse(0x7f);
		m_pMapPrev->Enable(false);
		m_pMapNext->Enable(false);
	}
	if (type == 5)
		type = 4;
	m_gameType = type;
	switch (type)
	{
	case 0:
		m_pText->SetBgImg("text_vs");
		if (m_pTextMap983) // 983 sem teste
			((FrArea*)m_pTextMap983)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 0;
		if (!bKeepValues)
		{
			m_userLimit = 2;
			m_hole = 3;
			m_shotTime = 40000;
			m_timeLimit = 40;
		}
		break;
	case 1:
		m_pText->SetBgImg("text_vs");
		if (m_pTextMap983)
			((FrArea*)m_pTextMap983)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 0;
		if (!bKeepValues)
		{
			m_userLimit = 4;
			m_hole = 6;
			m_shotTime = 40000;
			m_timeLimit = 40;
		}
		break;
	case 2:
		m_pText->SetBgImg("text_chat");
		m_gameTypeGroup = 3;
		break;
	case 4:
		m_pText->SetBgImg("text_mass");
		if (m_pTextMap983)
			((FrArea*)m_pTextMap983)->SetBgImg("text_mass_map");
		m_gameTypeGroup = 1;
		if (!bKeepValues)
			m_timeLimit = m_hole != 9 ? 35 : 15;
		break;
	case 5:
	case 6:
		m_pText->SetBgImg("text_mass");
		if (m_pTextMap983)
			((FrArea*)m_pTextMap983)->SetBgImg("text_mass_map");
		m_gameTypeGroup = 1;
		break;
	case 7:
		m_pText->SetBgImg("text_vs");
		if (m_pTextMap983)
			((FrArea*)m_pTextMap983)->SetBgImg("text_vs_map");
		m_gameTypeGroup = 2;
		break;
	case 9:
	case 10:
		m_pText->SetBgImg("text_vs");
		m_gameTypeGroup = 2;
		if (m_pTextMap983)
			((FrArea*)m_pTextMap983)->SetBgImg("text_vs_map");
		break;
	}
	switch (m_gameType)
	{
	case 0:
	case 1:
	case 4:
	case 5:
	case 6:
		SetArtifactArea983(true, ArtifactShow983(true));
		return;
	default:
		SetArtifactArea983(false, ArtifactShow983(false));
		return;
	case 7:
	case 9:
	case 10:
		SetArtifactArea983(true, ArtifactShow983(false));
		return;
	}
}

bool FrChangeRoomInfoDlg::OnInit()
{
	// 983 00acd0a0
	ChangeGameType(m_roomInfo.gameType, false, true);
	ChangeCourse(m_roomInfo.mapType);
	SetRoomFlags983(m_roomInfo.roomFlags983);
	if (m_gameType == 10 || m_gameType == 6)
	{
		if (m_pMapPrev)
			m_pMapPrev->Enable(false);
		if (m_pMapNext)
			m_pMapNext->Enable(false);
	}
	RefreshArtiButtons983(*m_artifact983.GetCur());
	return true;
}

void FrChangeRoomInfoDlg::DrawGameTypeButton(FrGraphicInterface* gi, int type,
	WRect rect, bool bEnable)
{
	const Bitmap* bitmap = m_pSelectBtnN;
	unsigned long color = 0xffffffff;
	if (bEnable)
	{
		if (m_gameType == type)
			bitmap = m_pSelectBtnD;
		else if (IsBtnEnable())
		{
			if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
			{
				if (g_input->GetButton(LEFT_BUTTON) == 2)
					ChangeGameType(type, false, false);
				bitmap = m_pSelectBtnO;
			}
		}
	}
	else
		color = 0xff909090; // 983 00acd1af
	gi->DrawTexture(bitmap, rect, color, 0);
	WPoint pos(rect.w * 0.5f + rect.x, rect.y + 5.0f);
	gi->PrintText11(pos, 1, s_gameTypeName[type], NULL);
}

void FrChangeRoomInfoDlg::OnGameTypeOwnerDraw(int param)
{
	FrGraphicInterface* gi = g_pFresh->GetManager()->GetGDI();
	if (gi)
	{
		// 983 00acd210: a flag de shortgame forca o tipo 5 a cada desenho (o OnOkBtnUp forca 4)
		if (m_roomFlags983 & 2)
			m_gameType = 5;
		const WRect& base = m_pGameType->GetRect();
		float x = base.x;
		float y = base.y;
		float bw = base.w;
		float h = base.h;
		float w;
		// 983: parte = w * 0.5 / w / 3.0 em double num float proprio (a largura lida nao e regravada); o 2o botao em x + parte, o 3o em parte + parte + x
		switch (m_gameTypeGroup)
		{
		case 0:
		{
			w = (float)(bw * 0.5);
			WRect first(x, y, w, h);
			WRect second(x + w, y, w, h);
			DrawGameTypeButton(gi, 0, first, true);
			DrawGameTypeButton(gi, 1, second, true);
			return;
		}
		case 1:
		{
			// 983: sala de GM (bAdminMaster, fora do tipo 11) com 100+ vagas = so o botao 4, desligado
			if (Doc()->m_roomInfo.gameType != 11 &&
				Doc()->m_roomInfo.bAdminMaster &&
				Doc()->m_roomInfo.nUserLimit > 99)
			{
				DrawGameTypeButton(gi, 4, WRect(x, y, bw, h), false);
				return;
			}
			bool bMass = m_gameType == 6;
			w = (float)(bw / 3.0);
			bool bEnable = !bMass;
			WRect first(x, y, w, h);
			WRect second(x + w, y, w, h);
			WRect third(w + w + x, y, w, h);
			DrawGameTypeButton(gi, 4, first, bEnable);
			DrawGameTypeButton(gi, 5, second, bEnable);
			DrawGameTypeButton(gi, 6, third, bMass);
			return;
		}
		case 2:
		{
			bool bEnable;
			w = (float)(bw * 0.5);
			WRect first(x, y, w, h);
			WRect second(x + w, y, w, h);
			if (m_gameType == 7)
			{
				DrawGameTypeButton(gi, 7, first, true);
				bEnable = false;
			}
			else
			{
				if (m_gameType != 10)
					return;
				DrawGameTypeButton(gi, 7, first, false);
				bEnable = true;
			}
			DrawGameTypeButton(gi, 10, second, bEnable);
			return;
		}
		case 3:
			DrawGameTypeButton(gi, 2, WRect(x, y, bw, h), true);
			return;
		}
	}
}

// 983 00acb620: confere o artefato do cursor com o tipo de sala escolhido; com bUpdateUI troca a imagem do "arti_ban" e o
// mostra se nao serve; notify 2 (ou 1 sem b0) avisa o ator principal (0x23) com o texto. Devolve bFail.
bool FrChangeRoomInfoDlg::CheckArtifact983(bool bUpdateUI, int notify)
{
	unsigned long* pTid = m_artifact983.GetCur();
	unsigned char gameType = (unsigned char)m_gameType;
	unsigned char hole = (unsigned char)m_hole;
	unsigned char flag = m_artifact983.GetFlag();
	sArtifactCheckArg983 arg;
	arg.pFlag = &flag;
	arg.pRoomType = &gameType;
	arg.pMode = (unsigned char*)&m_roomInfo.realGameType;
	arg.p1094b = &hole;
	sArtifactCheck983 check(pTid, &arg);
	if (m_pArtiBan983 && bUpdateUI)
	{
		((FrArea*)m_pArtiBan983)->SetBgImg(check.name);
		m_pArtiBan983->SetVisible(check.bFail);
	}
	if (check.bFail && notify > 0)
	{
		// 983: modo 1 avisa so sem b0 (sete), modo 2 sempre
		bool bSend;
		if (notify == 1)
			bSend = check.b0 == 0;
		else if (notify == 2)
			bSend = true;
		else
			bSend = false;
		if (bSend)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)check.text, 0, 0, 0, 0));
	}
	return check.bFail;
}
