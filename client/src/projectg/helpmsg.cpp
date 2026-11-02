#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "helpmsg.h"
#include "s5/classicserver/classicserver.h"
#include "golfdoc.h"
#include "golfball.h"
#include "club.h"
#include "mousecursor.h"
#include "clientsetting.h"
#include "jp/uiframesetmanager.h"
#include "chatmsg.h"
#include "wresrcmng.h"
#include "wfont.h"
#include "woverlay.h"
#include "../shared/localize.h"


extern WResourceManager* g_resrcmng;

bool IsMyTurn(bool check);
unsigned char GetCurMap();
sPlayerData* PLAYER(unsigned char index);
sTeamData* TEAM(unsigned char index);
int float2int(float value);
const WVector& GetHoleCupPos(); // 983: devolve referencia

// 983 (ctor 006e78a0 new 0x180, dtor 006e62d0 ate 0x180): 32 linhas; o Process passa ao SetHelpMsg ids < 0x20
#define HELPMSG_ROWS 0x20
C_ASSERT(HELPMSG_ROWS * 3 * sizeof(char*) == 0x180);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.realGameType) == 0x109b8 && offsetof(CSharedDoc, m_roomInfo.holeType) == 0x1094f); // A738
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594); // A738

static int s_column;
static int s_lastId = 0x20;	// 983 00e163f0 = 0x20

#define RB_STR_V "/v"

// 983 (006e7790): a base vertical vem da medida "d_helpmsg_base_y" do CUiFrameSetManager (escala da tela
// sobre 640x480); sem ela, a barra do COption.
float CHelpMsg::CalcBaseY()
{
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager::NamedDim* dim = CUiFrameSetManager::Instance()->FindNamedDim("d_helpmsg_base_y");
	if (dim)
		return (float)floor(dim->Calc(scl));
	return COption::Instance()->gGetBar_Y() - 110.0f; // 983 006e7872: o -110 e so do fallback
}

CHelpMsg::CHelpMsg()
{
	m_pOverlay = NULL;
	m_curId = -1;
	m_itemId = 0;
	m_idleTime = 0;
	m_bItem = false;
	m_pReplayItem = NULL;

	float barY = CalcBaseY();
	m_pOverlay = g_resrcmng->GetOverlay("[help.jpg", 0);
	m_pFont = CChatMsg::Instance()->GetMaskedFont();
	m_src[0] = WRect(0.0f, 0.0f, 1.0f, 0.25f);
	m_src[1] = WRect(0.0f, 0.25f, 1.0f, 0.5f);
	m_src[2] = WRect(0.0f, 0.75f, 1.0f, 0.25f);
	m_dst[0] = WRect(g_view->GetWidth() - 168.0f, 0, 256.0f, 8.0f);
	m_dst[1] = WRect(g_view->GetWidth() - 168.0f, 0, 256.0f, 0.0f);
	m_dst[2] = WRect(g_view->GetWidth() - 168.0f, barY, 256.0f, 8.0f);

	// 983 (006e78a0): "(PgUp, PgDn)", "(I)", "(R)", "([ or ])" e "/v" entram crus (sem K2L)
	const char*
		table[HELPMSG_ROWS]
			 [3] = {
				 { K2L_Compatibility("\xc6\xce\xbe\xdf \xc4\xde\xba\xb8\xb0\xd4\xc0\xcc\xc1\xf6"),
                  K2L_Compatibility("\xc6\xce\xbe\xdf \xc4\xde\xba\xb8 \xb0\xd4\xc0\xcc\xc1\xf6\xb8\xa6 \xb8\xf0\xb5\xce \xc3\xa4\xbf\xec\xb8\xe9 \xc6\xc4\xbf\xf6\xbc\xa6\xc0\xbb \xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n/v"),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc6\xc4\xbf\xf6\xbc\xa6"),
                  K2L_Compatibility("\xc6\xce\xbe\xdf \xc4\xde\xba\xb8 \xb0\xd4\xc0\xcc\xc1\xf6\xb0\xa1 \xb8\xf0\xc0\xda\xb6\xf3\xbc\xad \xc6\xc4\xbf\xf6\xbc\xa6\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee \xc1\xa4\xba\xb8"),
                  K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc2\xfc\xb0\xa1\xc0\xda\xc0\xc7 \xc5\xb8\xbc\xf6\xb3\xaa \xb0\xb3\xc0\xce \xc1\xa4\xba\xb8\xb8\xa6 \xba\xbc \xbc\xf6 \xc0\xd6\xb4\xc2 \xb0\xf7 \xc0\xd4\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc8\xa6\xc4\xc5\xc1\xf6\xc1\xa1 \xc1\xa4\xba\xb8"),
                  K2L_Compatibility("\xc8\xa6\xc4\xc5 \xc1\xf6\xc1\xa1\xc0\xc7 \xc0\xa7\xc4\xa1\xb8\xa6 \xb0\xc5\xb8\xae\xbf\xcd \xb3\xf4\xc0\xcc \xc1\xc2\xc7\xa5\xb7\xce \xbe\xcb\xb7\xc1 \xc1\xdd\xb4\xcf\xb4\xd9.\n/v"),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc5\xac\xb7\xb4 \xbc\xb1\xc5\xc3 /k"),
                  K2L_Compatibility("\xb8\xb6\xbf\xec\xbd\xba \xc8\xd9\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xd8 \xc0\xfb\xc0\xfd\xc7\xd1 \xba\xf1\xb0\xc5\xb8\xae\xc0\xc7 \xc5\xac\xb7\xb4\xc0\xbb \xbc\xb1\xc5\xc3 \xc7\xcf\xbd\xca\xbd\xc3\xbf\xe4. /v"),
                  "(PgUp, PgDn)"},
				 { K2L_Compatibility("\xc3\xa4\xc6\xc3 \xc3\xa2 /k"),
                  K2L_Compatibility("\xc3\xa4\xc6\xc3\xc3\xa2\xc0\xbb \xbf\xad\xb0\xed \xb4\xdd\xbd\xc0\xb4\xcf\xb4\xd9."),
                  K2L_Compatibility("(ENTER)")},
				 { K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb \xc3\xa2 /k"),
                  K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc3\xa2\xc0\xbb \xbf\xad\xb0\xed \xb4\xdd\xbd\xc0\xb4\xcf\xb4\xd9."),
                  "(I)"},
				 { K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb \xc3\xa2 /k"),
                  K2L_Compatibility("\xb0\xd4\xc0\xd3 \xc1\xdf\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbe\xb5 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xc7\xd1\xc5\xb8\xbc\xf6\xbf\xa1 1\xc8\xb8 \xb9\xdb\xbf\xa1 \xb8\xf8\xbe\xb2\xb4\xcf \xbd\xc5\xc1\xdf\xc8\xf7 \xbb\xe7\xbf\xeb\xc7\xcf\xbc\xbc\xbf\xe4."),
                  "(I)"},
				 { K2L_Compatibility("\xc5\xb8\xb1\xb8 \xc1\xa6\xc7\xd1 \xbd\xc3\xb0\xe8 /v"),
                  K2L_Compatibility("\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3\xbe\xc8\xbf\xa1 \xbc\xa6\xc0\xbb \xc7\xcf\xc1\xf6 \xb8\xf8\xc7\xcf\xb8\xe9 \xbd\xc3\xb0\xe8\xb0\xa1 \xbf\xef\xb8\xae\xb8\xe9\xbc\xad \xb9\xfa\xc1\xa1\xc0\xbb \xb9\xde\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc7\xb3\xc7\xe2 \xc7\xb3\xbc\xd3 \xc7\xa5\xbd\xc3\xb1\xe2"),
                  K2L_Compatibility("\xb9\xd9\xb6\xf7\xc0\xc7 \xbc\xbc\xb1\xe2\xbf\xcd \xb9\xe6\xc7\xe2\xc0\xbb \xc7\xa5\xbd\xc3\xc7\xd8 \xc1\xdd\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { NULL,
                  K2L_Compatibility("Bunker\xbf\xa1 \xba\xfc\xc1\xf6\xb8\xe9 \xc8\xc4\xc5\xf0 \xc7\xcf\xb4\xf5\xb6\xf3\xb5\xb5 \xc0\xcf\xb4\xdc\xc0\xba \xb0\xa1\xb1\xee\xbf\xee Fairway\xb7\xce \xb0\xf8\xc0\xbb \xc4\xa1\xbc\xbc\xbf\xe4."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc5\xb8\xb1\xb8 \xb9\xd9 /k"),
                  K2L_Compatibility("\xbc\xa6\xc0\xbb \xc7\xcf\xb7\xc1\xb8\xe9 \xc0\xcc\xb0\xf7\xc0\xbb \xc5\xac\xb8\xaf \xc7\xcf\xbc\xbc\xbf\xe4."),
                  K2L_Compatibility("(SPACE)")},
				 { K2L_Compatibility("\xc5\xb8\xc1\xa1 \xb0\xe1\xc1\xa4 /k"),
                  K2L_Compatibility("\xb8\xb6\xbf\xec\xbd\xba \xb5\xe5\xb7\xa1\xb1\xd7\xb7\xce \xb0\xf8\xc0\xc7 \xc5\xb8\xc1\xa1\xc0\xbb \xb0\xe1\xc1\xa4\xc7\xcf\xbf\xa9 \xbd\xba\xc7\xc9\xc0\xbb \xb0\xa1\xb4\xc9\xc7\xcf\xb0\xd4 \xc7\xd5\xb4\xcf\xb4\xd9."),
                  K2L_Compatibility("(CTRL+\xb9\xe6\xc7\xe2\xc5\xb0)")},
				 { K2L_Compatibility("\xba\xbc\xc4\xc1\xb5\xf0\xbc\xc7\xb0\xfa \xc1\xf6\xb8\xe9 \xbb\xf3\xc8\xb2"),
                  K2L_Compatibility("\xc1\xf6\xb8\xe9 \xc6\xaf\xbc\xba\xb0\xfa \xb1\xe2\xbf\xef\xb1\xe2\xbf\xa1 \xb5\xfb\xb6\xf3 \xb0\xf8\xc0\xc7 \xc8\xfb \xc0\xfc\xb4\xde \xbb\xf3\xc5\xc2\xb0\xa1 \xb4\xde\xb6\xf3 \xc1\xfd\xb4\xcf\xb4\xd9. /v\n/c0xFF007A39/cFairway : 90~100%\n/c0xFF063100/cRough : 85~95%\n/c0xFFD38600/cBunker : 55~85%"),
                  NULL																																																																																											 },
				 { NULL,
                  K2L_Compatibility("/c0xFFFF0000/cAlt /c0xFF000000/c\xc5\xb0\xb7\xce \xc4\xb3\xb8\xaf\xc5\xcd\xb8\xa6 \xc5\xf5\xb8\xed\xc7\xcf\xb0\xd4 \xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc0\xcc\xb8\xf0\xc6\xbc\xc4\xdc \xb1\xd7\xb7\xec"),
                  RB_STR_V,																																																																																								   NULL },
				 { K2L_Compatibility("\xc5\xac\xb7\xb4\xbb\xe7\xbf\xeb\xc1\xa6\xc7\xd1"),
                  K2L_Compatibility("\xb4\xf5 \xc0\xcc\xbb\xf3\xc0\xc7 \xc5\xac\xb7\xb4\xc0\xba \xc7\xf6\xc0\xa7\xc4\xa1\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xbb\xe7\xbf\xeb\xc1\xa6\xc7\xd1"),                   K2L_Compatibility("\xbc\xb1\xc5\xc3\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xc7\xf6\xc0\xe7 \xbb\xf3\xc8\xb2\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc5\xac\xb7\xb4\xbb\xe7\xbf\xeb\xc1\xa6\xc7\xd1"),
                  K2L_Compatibility("\xc6\xc4\xbf\xf6\xbc\xa6 \xbb\xf3\xc5\xc2\xbf\xa1\xbc\xad\xb4\xc2 \xc6\xdb\xc5\xcd\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xb0\xf8 \xbf\xc5\xb1\xe2\xb1\xe2 \xb8\xf0\xb5\xe5"),
                  K2L_Compatibility("\xb0\xcb\xc0\xba \xbf\xf8\xc0\xbb \xc1\xa6\xbf\xdc\xc7\xd1 \xc6\xe4\xbe\xee\xbf\xfe\xc0\xcc \xba\xce\xba\xd0\xbf\xa1 Shift+\xb8\xb6\xbf\xec\xbd\xba \xbf\xde\xc2\xca \xc5\xac\xb8\xaf\xc0\xbb \xc7\xcf\xbf\xa9 \xb0\xf8\xc0\xbb \xbf\xc5\xb1\xe6 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xb0\xf8 \xc0\xcc\xb5\xbf \xba\xd2\xb0\xa1 \xc1\xf6\xbf\xaa"),
                  K2L_Compatibility("\xc0\xcc\xb0\xf7\xbf\xa3 \xb0\xf8\xc0\xbb \xbf\xc5\xb1\xe6 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xcc\xb0\xf7\xc0\xbb \xc1\xa6\xbf\xdc\xc7\xd1 \xc6\xe4\xbe\xee\xbf\xfe\xc0\xcc\xbf\xa1 \xb0\xf8\xc0\xbb \xbf\xc5\xb0\xdc \xc1\xd6\xbd\xca\xbd\xc3\xbf\xe4."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc /k"),
                  K2L_Compatibility("\xbc\xa6\xc0\xbb \xc7\xcf\xbf\xa9 \xb3\xaf\xbe\xc6\xb0\xa3 \xb0\xf8\xc0\xcc \xbf\xcf\xc0\xfc\xc8\xf7 \xb8\xd8\xc3\xe1 \xc8\xc4 \xb6\xc7\xb4\xc2 \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xb5\xb5\xc1\xdf, R\xc5\xb0\xb8\xa6 \xb4\xad\xb7\xaf \xbc\xa6\xc0\xbb \xb4\xd9\xbd\xc3 \xba\xbc \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n/v"),
                  "(R)"},
				 { K2L_Compatibility("\xb1\xe2\xb4\xc9\xbb\xe7\xbf\xeb\xba\xd2\xb0\xa1"),
                  K2L_Compatibility("\xb7\xa1\xb4\xf5\xc0\xfc\xbf\xa1\xbc\xad\xb4\xc2 \xbb\xf3\xb4\xeb \xc5\xcf\xc0\xcf\xb6\xa7 \xc5\xbe\xba\xe4\xb8\xa6 \xba\xbc \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xb1\xe2\xb4\xc9\xbb\xe7\xbf\xeb\xba\xd2\xb0\xa1"),
                  K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xb0\xf8 \xbf\xc5\xb1\xe2\xb1\xe2\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xb1\xe2\xb4\xc9\xbb\xe7\xbf\xeb\xba\xd2\xb0\xa1"),
                  K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xc0\xcf\xbd\xc3\xc1\xa4\xc1\xf6\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xb1\xe2\xb4\xc9\xbb\xe7\xbf\xeb\xba\xd2\xb0\xa1"),
                  K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xc6\xc4\xbf\xf6\xbc\xa6"),
                  K2L_Compatibility("[\xc0\xd3\xbd\xc3\xb8\xde\xbc\xbc\xc1\xf6] \xc3\xbc\xb7\xc2\xba\xb8\xc1\xb6\xc1\xa6 \xbb\xe7\xbf\xeb\xc8\xc4 \xc6\xc4\xbf\xf6\xbc\xa6"
					 "2 \xba\xd2\xb0\xa1~"),																																																																																				   NULL },
				 { K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xbb\xe7\xbf\xeb\xc1\xa6\xc7\xd1"),
                  K2L_Compatibility("\xbf\xc0\xc5\xe4\xc4\xb6\xb8\xae\xc6\xdb\xbd\xba\xb8\xa6 \xbb\xe7\xbf\xeb\xc1\xdf\xc0\xcc\xb9\xc7\xb7\xce \xc1\xdf\xba\xb9 \xbb\xe7\xbf\xeb \xba\xd2\xb0\xa1\xb4\xc9 \xc7\xd5\xb4\xcf\xb4\xd9."),
                  NULL																																																																																											 },
				 { K2L_Compatibility("\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc (\xc8\xa6) /k"),
                  K2L_Compatibility("\xc5\xb0\xba\xb8\xb5\xe5 'R'\xc0\xbb \xb4\xa9\xb8\xa3\xb8\xe9 \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc5\xd7\xc0\xd9 1\xb0\xb3\xb8\xa6 \xbc\xd2\xba\xf1\xc7\xcf\xbf\xa9, \xc7\xf6\xc0\xe7 \xc8\xa6\xc0\xc7 \xc7\xc3\xb7\xb9\xc0\xcc \xb3\xbb\xbf\xeb\xc0\xbb \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc\xb7\xce \xc0\xfa\xc0\xe5\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n/v"),
                  "(R)"},
				 // 983 (006e78a0): linhas 0x1d..0x1f (camera do green, reserva da posicao do tee, reserva de troca de ambiente)
				 { K2L_Compatibility("\xbf\xa1\xbe\xee \xb3\xeb\xc6\xae \xc4\xab\xb8\xde\xb6\xf3/k"),
                  K2L_Compatibility("\xc0\xcc\xc0\xfc \xbc\xa6 \xc0\xa7\xc4\xa1\xb7\xce \xc4\xab\xb8\xde\xb6\xf3\xb8\xa6 \xc0\xcc\xb5\xbf\xc7\xd5\xb4\xcf\xb4\xd9."),
                  "([ or ])"},
				 { K2L_Compatibility("\xc6\xbc\xbc\xa6 \xc0\xa7\xc4\xa1 \xc0\xcc\xb5\xbf \xbf\xb9\xbe\xe0"),
                  K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xbf\xa1 \xbc\xba\xb0\xf8\xc7\xcf\xb8\xe9 \xc6\xbc\xbc\xa6 \xc0\xa7\xc4\xa1\xb0\xa1 \xba\xaf\xb0\xe6\xb5\xcb\xb4\xcf\xb4\xd9."),
                  NULL },
				 { K2L_Compatibility("\xc8\xaf\xb0\xe6 \xb9\xd9\xb2\xd9\xb1\xe2 \xbf\xb9\xbe\xe0"),
                  K2L_Compatibility("\xb4\xd9\xc0\xbd \xc8\xa6\xbf\xa1\xbc\xad \xb9\xd9\xb6\xf7, \xb3\xaf\xbe\xbe\xb8\xa6 \xba\xaf\xb0\xe6\xc7\xd5\xb4\xcf\xb4\xd9."),
                  NULL },
    };

	m_table = new char*[HELPMSG_ROWS][3]; // 983: new 0x180
	for (int i = 0; i < HELPMSG_ROWS; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (table[i][j])
			{
				m_table[i][j] = new char[strlen(table[i][j]) + 1];
				strcpy(m_table[i][j], table[i][j]);
			}
			else
			{
				m_table[i][j] = NULL;
			}
		}
	}

	memset(m_title, 0, sizeof(m_title));
	memset(m_body, 0, sizeof(m_body));
	memset(m_sub, 0, sizeof(m_sub));
	memset(m_extra, 0, sizeof(m_extra));
	CheckReplayAvailable();
}

CHelpMsg::~CHelpMsg()
{
	for (int i = 0; i < HELPMSG_ROWS; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (m_table[i][j])
			{
				delete[] m_table[i][j];
				m_table[i][j] = NULL;
			}
		}
	}

	if (m_table)
	{
		delete[] m_table;
		m_table = NULL;
	}

	if (g_resrcmng && m_pOverlay)
	{
		g_resrcmng->Release(m_pOverlay);
		m_pOverlay = NULL;
	}
}

void CHelpMsg::ResetUiPosition()
{
	if (0) // HACK: keeps the inline naked Process emitted (nakedify)
		((CHelpMsg*)0)->Process(*(float*)0);
	m_dst[0].x = g_view->GetWidth() - 168.0f;
	m_dst[1].x = g_view->GetWidth() - 168.0f;
	m_dst[2].x = g_view->GetWidth() - 168.0f;
	m_dst[2].y = CalcBaseY();
	if (m_curId != -1 || m_itemId)
		CalcWindowSize(m_title, m_body);
}

void CHelpMsg::Process(float dt)
{
	char replay[32];
	char text[32];
	char dist[32];
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->GetPlayingMode() == 1)
	{
		if (Doc()->m_replayPlayType == 4 && IsMyTurn(false) &&
			Doc()->m_gameMode != 5 && Doc()->m_gameMode == 0 &&
			GOLFDOC()->m_tutorialMode == 15 &&
			Doc()->m_golfGame.gameType != 10 &&
			Doc()->m_roomInfo.realGameType != 0x12 &&
			(unsigned)(Doc()->m_roomInfo.realGameType - 0x14) > 1)
		{
			if (GetCurMap() != 7 && !S5::CLASSICSRV::IsShortGame983())
				SetHelpMsg(0x1c, NULL);
			return;
		}
	}
	else
	{
		if (GOLFDOC()->m_gameMode & 0xc0)
		{
			if (GOLFDOC()->m_tutorialMode == 15 && Doc()->m_gameMode == 0 &&
				Doc()->m_golfGame.gameType != 10 && IsMyTurn(false) &&
				m_pReplayItem && m_curId == -1 && m_itemId == 0 &&
				(GOLFDOC()->m_gameMode & 0x80) &&
				(!GolfBall().m_state || GOLFDOC()->m_bReplaying) &&
				Doc()->m_golfGame.gameType != 0xb)
			{
				if (IsLocalContent(S4_REPLAY_SYSTEM))
					return;
				sprintf(replay,
					K2L_Compatibility("(%d\xb9\xf8 \xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9)"),
					m_pReplayItem->Common[0]);
				SetHelpMsg(0x15, replay);
				return;
			}
			if (m_curId != 0x15)
				m_curId = -1;
		}
		else
		{
			if (m_curId == 0x15)
				m_curId = -1;
		}
	}

	if (!CMouseCursor::Instance()->IsActive())
		return;

	if (m_curId == 0x11 || m_curId == 0x10 || m_curId == 1 || m_curId == 0x12 ||
		m_curId == 0x14 || m_curId == 0x16 || m_curId == 0x17 ||
		m_curId == 0x18 || m_curId == 0x1b)
	{
		if (m_idleTime > 2.0f)
		{
			m_curId = -1;
		}
		else
		{
			m_idleTime += dt;
			return;
		}
	}

	m_extra[0] = 0;
	text[0] = 0;
	int id = 0x20;
	int area = CMouseCursor::Instance()->GetArea();
	switch (area)
	{
	case 6:
		id = 6;
		break;
	case 18:
		id = 7;
		break;
	case 1:
	case 2:
		id = 4;
		sprintf(m_extra, K2L_Compatibility("(%.0fy\xbf\xeb %s)"), GolfClub().GetRange(NULL),
			GolfClub().GetName());
		break;
	case 7:
		id = 5;
		break;
	case 5:
		id = 11;
		break;
	case 14:
	case 15:
	case 16:
	case 17:
		id = 0;
		{
			float t;
			if (Doc()->m_golfGame.gameType == 1)
				t = TEAM(area - 14)->gauge / 33.0f;
			else
				t = PLAYER(area - 14)->gauge / 33.0f;
			sprintf(m_extra,
				K2L_Compatibility("(\xc6\xc4\xbf\xf6\xbc\xa6\xc0\xbb %d\xb9\xf8 \xbb\xe7\xbf\xeb "
				"\xb0\xa1\xb4\xc9)"),
				float2int(t));
		}
		break;
	case 10:
	case 11:
	case 12:
	case 13:
		id = 2;
		break;
	case 19:
		id = 9;
		break;
	case 20:
	{
		unsigned long ms = GOLFDOC()->m_remainTime;
		unsigned long sec = ms / 1000;
		id = 8;
		if (ms > sec * 1000)
			sec++;
		// 983 006e7256: K2L_Compatibility (00b4ec60) sobre a string 00d1aa28; cru saia coreano no JP
		sprintf(m_extra, K2L_Compatibility("(\xc7\xf6\xc0\xe7 %d\xc3\xca)"), sec);
		break;
	}
	case 21:
	{
		WVector d = GetHoleCupPos() - GolfBall().m_pos;
		float height = d.y * 0.28575f;	// 983 00d08df0: 0.3125 * 0.9144 num fator so
		d.y = 0;
		float range = d.Magnitude();
		range *= 0.3125f;
		if (Abs(height) < 0.1f)
			sprintf(text, K2L_Compatibility("(\xb3\xf4\xc0\xcc:0.0m, "));
		else if (Abs(height) > 10.0f)
			sprintf(text, K2L_Compatibility("(\xb3\xf4\xc0\xcc:%+.0fm, "), height);
		else
			sprintf(text, K2L_Compatibility("(\xb3\xf4\xc0\xcc:%+.1fm, "), height);
		if (range < 1.0f)
			sprintf(dist, K2L_Compatibility("\xb0\xc5\xb8\xae:%.1fy)"), range);
		else
			sprintf(dist, K2L_Compatibility("\xb0\xc5\xb8\xae:%.0fy)"), range);
		strcat(text, dist);
		id = 3;
		break;
	}
	case 3:
		if (!g_input->GetButton(LEFT_BUTTON))
		{
			float x, y;
			AfxGetTask()->GetActor("Spin")
				<< MsgObject(NULL, M983(0x166), (int)&x, (int)&y, 0, 0, 0);
			x -= g_input->GetMousePoint().x;
			y -= g_input->GetMousePoint().y;
			if (GolfClub().GetType() != 3 && x * x + y * y < 25.0f)	// 983 006e6e90: limite 25.0 (d/cfebf8), nao 100.0
			{
				id = 12;
			}
			else
			{
				float spin;
				id = 13;
				AfxGetTask()->GetActor("Screen")
					<< MsgObject(NULL, M983(0x1c2), (int)&spin, 0, 0, 0, 0);
				sprintf(text, K2L_Compatibility("(\xc7\xf6\xc0\xe7 %.0f%%)"), spin * 100.0f);
			}
		}
		break;
	}
	if (area >= 0x33 && area <= 0x3e)
		id = 0xf;

	{
		bool overReview = false;
		AfxGetTask()->GetActor("ShotReview")
			<< MsgObject(NULL, 0x2fd /* 983 */, (int)"IS_MOUSE_OVER_REVIEW_BUTTON",
				(int)&overReview, 0, 0, 0);
		if (overReview)
			id = 0x1d;
	}
	{
		bool ruleHelp = false;
		AfxGetTask()->GetActor("GolfRule")
			<< MsgObject(NULL, 0x316 /* 983 */, (int)&ruleHelp, 0, 0, 0, 0);
		if (ruleHelp)
			id = (Doc()->m_roomInfo.holeType == 4) ? 0x1f : 0x1e;
	}

	if (m_itemId)
	{
		if (id == 0x20)
		{
			s_lastId = id;
			id = 7;
		}
		else
		{
			if (id != s_lastId)
			{
				s_lastId = id;
				m_bItem = false;
			}
			if (m_bItem)
				id = 7;
		}
	}

	C_ASSERT_VC10(offsetof(CGolfDoc, m_bDropRequest) == 0x190);
	if (GOLFDOC()->m_bDropRequest) // 983 +0x190
		id = 0x13;

	if (m_curId == 0x15)
	{
		id = 0x15;
	}
	else if (id < 0x20)
	{
		if (id == 0xf)
			return;
	}
	else
	{
		m_curId = -1;
		return;
	}

	SetHelpMsg(id, text[0] ? text : NULL);
}
void CHelpMsg::CalcWindowSize(char* title, char* body)
{
	if (title)
		m_dst[1].h = 20.0f;
	else
		m_dst[1].h = 0;

	int start;
	int end = 0;
	int pos = 0;
	int ret = -1;
	for (;;)
	{
		ret = GetLine(&start, &end, NULL, body + pos, ret == 5 || ret == 6);
		pos += end;
		if (ret == 0)
			break;
		if (ret == 1 || ret == 4 || ret == 2 || ret == 3)
			continue;
		if (ret == 6)
			pos++;
		m_dst[1].h += 15.0f;
	}

	m_dst[1].h += 11.0f;
	m_dst[1].y = m_dst[2].y - m_dst[1].h;
	m_dst[0].y = m_dst[1].y - 8.0f;
}

int CHelpMsg::GetLine(int* start, int* end, unsigned long* color, char* text,
	bool trim)
{
	int i = 0;
	if (trim)
	{
		while (text[i] == ' ' || text[i] == '\t')
			i++;
	}

	if (text[i] == '/')
	{
		switch (text[i + 1])
		{
		case 'v':
			i += 2;
			*end = i;
			*start = i;
			return 2;
		case 'k':
			i += 2;
			*end = i;
			*start = i;
			return 3;
		case 'c':
			if (color)
			{
				i += 4;
				*color = 0;
				for (int shift = 20; shift >= -8; shift -= 16)
				{
					{
						char c = text[i + 0];
						int v;
						if (c < '0' || c > '9')
							v = c - 55;
						else
							v = c - '0';
						*color |= v << (shift + 8);
					}
					{
						char c = text[i + 1];
						int v;
						if (c < '0' || c > '9')
							v = c - 55;
						else
							v = c - '0';
						*color |= v << (shift + 4);
					}
					{
						char c = text[i + 2];
						int v;
						if (c < '0' || c > '9')
							v = c - 55;
						else
							v = c - '0';
						*color |= v << shift;
					}
					{
						char c = text[i + 3];
						int v;
						if (c < '0' || c > '9')
							v = c - 55;
						else
							v = c - '0';
						*color |= v << (shift - 4);
					}
					i += 4;
				}
				i += 2;
			}
			else
			{
				i += 14;
			}
			*end = i;
			*start = i;
			return 1;
		}
	}

	bool bSpace = false;
	int j = i;
	int space;
	while (text[j])
	{
		if (text[j] == '\n')
			break;
		if (text[j] == '/' &&
			(text[j + 1] == 'c' || text[j + 1] == 'v' || text[j + 1] == 'k'))
			break;
		if (text[j] == ' ')
		{
			space = j;
			bSpace = true;
		}
		j += Custom_IsLeadByte(text[j]) + 1;
		if (s_column - i + j > 25)
			break;
	}

	*start = i;
	*end = j;
	s_column += j - i;

	if (text[j] == 0)
	{
		s_column = 0;
		return 0;
	}
	if (text[j] == '/' &&
		(text[j + 1] == 'c' || text[j + 1] == 'v' || text[j + 1] == 'k'))
		return 4;
	if (text[j] == '\n')
	{
		s_column = 0;
		return 6;
	}
	if (bSpace && s_column - i + j > 25)
		*end = space;
	s_column = 0;
	return 5;
}

void CHelpMsg::Display()
{
	if (m_curId < 0)
		return;

	float h = 0;
	m_pOverlay->Render(g_view, m_src[0], m_dst[0], 0, -1, 0, 0);
	m_pOverlay->Render(g_view, m_src[1], m_dst[1], 0, -1, 0, 0);
	m_pOverlay->Render(g_view, m_src[2], m_dst[2], 0, -1, 0, 0);

	float y = m_dst[0].y + 8.0f;
	if (m_title[0])
	{
		char* tag = strstr(m_title, "/v");
		if (tag)
		{
			char c = *tag;
			*tag = 0;
			float x = g_view->GetWidth() - 159.0f;
			x += m_pFont->Print(g_view, x, y, m_title, 0, 0xffff0000, NULL);
			*tag = c;
			m_pFont->Print(g_view, x, y, m_extra, 0, 0xffff0000, NULL);
		}
		else
		{
			tag = strstr(m_title, "/k");
			if (tag)
			{
				char c = *tag;
				*tag = 0;
				float x = g_view->GetWidth() - 159.0f;
				x += m_pFont->Print(g_view, x, y, m_title, 0, 0xffff0000, NULL);
				*tag = c;
				m_pFont->Print(g_view, x, y, m_sub, 0, 0xffff0000, NULL);
			}
			else
			{
				m_pFont->Print(g_view, g_view->GetWidth() - 159.0f, y, m_title,
					0, 0xffff0000, NULL);
			}
		}
		y += 20.0f;
	}

	float x = g_view->GetWidth() - 156.0f;
	int end = 0;
	int start = 0;
	int pos = 0;
	int ret = -1;
	unsigned long color = 0xff000000;
	for (;;)
	{
		ret = GetLine(&start, &end, &color, &m_body[pos], ret == 5 || ret == 6);
		start += pos;
		pos += end;
		if (ret == 1)
			continue;
		if (ret == 2)
		{
			h = m_pFont->Print(g_view, x, y, m_extra, 0, 0xffff0000, NULL);
			x += h;
		}
		else if (ret == 3)
		{
			h = m_pFont->Print(g_view, x, y, m_sub, 0, 0xffff0000, NULL);
			x += h;
		}
		else
		{
			if (pos > start)
			{
				char c = m_body[pos];
				m_body[pos] = 0;
				h = m_pFont->Print(g_view, x, y, &m_body[start], 0, color,
					NULL);
				m_body[pos] = c;
			}
			if (ret == 4)
			{
				x += h;
			}
			else
			{
				x = g_view->GetWidth() - 156.0f;
				y += 15.0f;
				if (ret == 6)
					pos++;
				else if (ret == 0)
					break;
			}
		}
	}
	m_pFont->Flush(g_view);
}

void CHelpMsg::SetItemHelpMsg(unsigned long itemId)
{
	if (itemId != m_itemId && itemId)
		m_bItem = true;
	m_itemId = itemId;
}

void CHelpMsg::SetHelpMsg(int id, char* extra)
{
	if (GOLFDOC()->m_tutorialMode < 15)
		return;

	if (id < 0)
	{
		m_curId = -1;
		return;
	}

	if (id == 0x11 || id == 1 || id == 0x10 || id == 0x12 || id == 0x14 ||
		id == 0x16 || id == 0x17 || id == 0x18 || id == 0x1b)
		m_idleTime = 0;

	if (extra)
		strcpy(m_extra, extra);

	if (id == 7 && m_itemId != 0 && m_itemId != 0x18000031)
	{
		m_curId = id;
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(m_itemId);
		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(m_itemId);
		if (pItem)
		{
			strncpy(m_title, pItem->c.Name, 31);
			strcat(m_title, " /k");
		}
		if (pDesc)
			strncpy(m_body, pDesc->Desc, 255);
		strncpy(m_sub, K2L_Compatibility("(\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9)"), 16);
		CalcWindowSize(m_title, m_body);
	}
	else if (id > -1 && m_curId != id)
	{
		m_curId = id;
		if (m_table[id][0])
			strncpy(m_title, m_table[id][0], 31);
		else
			m_title[0] = 0;
		strcpy(m_body, m_table[m_curId][1]);
		if (m_table[m_curId][2])
			strncpy(m_sub, m_table[m_curId][2], 15);
		else
			m_sub[0] = 0;
		CalcWindowSize(m_table[m_curId][0], m_table[m_curId][1]);
	}
}

void CHelpMsg::SetEmoticonHelpMsg(char** lines)
{
	m_curId = 0xf;
	const char* title =
		K2L_Compatibility("\xc0\xcc\xb8\xf0\xc6\xbc\xc4\xdc \xb8\xed\xb7\xc9\xbe\xee");
	strcpy(m_title, title);
	m_body[0] = 0;
	strcat(m_body, "/c0xFFFF0000/c/");
	strcat(m_body, lines[0]);
	for (int i = 1; lines[i] && i <= 2; i++)
	{
		strcat(m_body, "/c0xFF000000/c, ");
		strcat(m_body, "/c0xFFFF0000/c/");
		strcat(m_body, lines[i]);
	}
	CalcWindowSize(m_title, m_body);
}

void CHelpMsg::OnChangeTurn()
{
	m_curId = -1;
	m_itemId = 0;
}

void CHelpMsg::CheckReplayAvailable()
{
	const std::list<sItemInfo>& items = Doc()->m_myItemList;
	std::list<sItemInfo>::const_iterator it =
		std::find(items.begin(), items.end(), 0x1a000000);
	if (it == items.end())
		m_pReplayItem = NULL;
	else
		m_pReplayItem = (sItemInfo*)&*it;
}
