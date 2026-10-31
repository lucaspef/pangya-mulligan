#include "minatl.h"
#define REBANG_RANKINGDLG_CPP
#include "rankingdlg.h"
#include "networksystem.h"
#include "packet.h"
#include "actor.h"
#include "frstatic.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "shareddoc.h"
#include "soundmanager.h"
#include "frcombobox.h"
#include "fredit.h"
#include "frviewer.h"
#include "frarea.h"
#include "frtextbutton.h"
#include "frgaugebar.h"
extern int g_lobbyModal983; // 00e78cac
#include "inputmanager.h"
#include "binstr.h"
#include "exhibition.h"
#include "emoticondlg.h"
#include "taskmanager.h"
#include "avatar_task.h"
#include "avatar_avatar.h"
#include "commonutil.h"
#include "frgraphicinterface.h"
#include "wview.h"
#include "woverlay.h"
#include "wresrcmng.h"
#include "mousecursor.h"
#include "../shared/localize.h"
#include "../shared/itemmanager.h"


const int FrRankingDlg::s_iListCountPerPage = 12;

// unused, but the original emitted g_HUGE/g_EPSILON into .rdata
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

extern Fresh* g_pFresh;

// HACK: VC7.1 emits WRect::WRect<float, float>(const WPoint&, float, float) out of line as soon as it is used,
// even when every call is inlined. In the original that copy is a discarded COMDAT duplicate of scoreboard's,
// but scoreboard.cpp provides it through /alternatename, which loses against a real definition. This
// non-template twin inlines to the same code and, being dllimport + __forceinline, is never emitted.
typedef WRect RbRect;

// NOTE: inline in shareddoc.h in the original; an inline body in the PCH
// changes the COMDAT order of other units (itemmanager)
inline const sPangYaUserInfo& CSharedDoc::GetPangyaUserInfo()
{
	return m_myInfo.info;
}

// HACK: the original tables are initialized by code (dynamic initializers that store each
// non-zero member); RB_DEFER_INIT makes the front end treat the initializer as dynamic
#define RB_DEFER_INIT(s) (1 ? (s) : 0)

static const sTypeName s_aRankType[] = {
	{ 0, RB_DEFER_INIT(K2L_Compatibility("\xc1\xbe\xc7\xd5 \xb7\xa9\xc5\xb7")) },
	{ 1, K2L_Compatibility("\xc4\xda\xbd\xba\xba\xb0 \xb7\xa9\xc5\xb7")        },
	{ 2, K2L_Compatibility("\xb1\xe2\xb7\xcf \xb7\xa9\xc5\xb7")                }
};

static const sTypeName s_aClassType[] = {
	{ 0, RB_DEFER_INIT("\xc0\xfc\xc3\xbc") },
    { 1, K2L_Compatibility("\xc3\xca\xb1\xde")                },
	{ 2, K2L_Compatibility("\xc1\xdf\xb1\xde")                },
    { 3, K2L_Compatibility("\xbb\xf3\xb1\xde")                }
};

static const sTypeName s_aGenSubType[] = {
	{ 0, RB_DEFER_INIT(K2L_Compatibility("\xc1\xbe\xc7\xd5 \xc1\xa1\xbc\xf6"))  },
	{ 1, K2L_Compatibility("\xbd\xba\xc4\xda\xbe\xee \xc1\xa1\xbc\xf6")         },
	{ 2, K2L_Compatibility("\xc6\xae\xb7\xce\xc7\xc7 \xc1\xa1\xbc\xf6")         },
	{ 3, K2L_Compatibility("\xc8\xb9\xb5\xe6\xc6\xce \xc1\xa1\xbc\xf6")         },
	{ 4, K2L_Compatibility("\xc7\xc3\xb7\xb9\xc0\xcc\xc8\xa6 \xc1\xa1\xbc\xf6") },
	{ 5, K2L_Compatibility("\xbe\xf7\xc0\xfb \xc6\xf7\xc0\xce\xc6\xae")          } // 983 00cf5cde
};

static const sTypeName s_aCourseType[] = {
	{ 0,  RB_DEFER_INIT(K2L_Compatibility("\xba\xed\xb7\xe7\xb6\xf3\xb1\xba"))   },
	{ 1,  K2L_Compatibility("\xba\xed\xb7\xe7\xbf\xf6\xc5\xcd")                  },
	{ 2,  K2L_Compatibility("\xbc\xbc\xc7\xc7\xbe\xc6\xc0\xa9\xb5\xe5")          },
	{ 3,  K2L_Compatibility("\xc0\xa9\xb5\xe5\xc8\xfa")                          },
	{ 4,  K2L_Compatibility("\xc0\xa7\xc1\xee\xc0\xa7\xc1\xee")                  },
	{ 5,  K2L_Compatibility("\xbf\xfe\xbd\xba\xc6\xae\xc0\xa7\xc1\xee")          },
	{ 6,  K2L_Compatibility("\xba\xed\xb7\xe7\xb9\xae")                          },
	{ 7,  "\xbd\xc7\xba\xf1\xbe\xc6\xc4\xb3\xb3\xed"          },
	{ 8,  K2L_Compatibility("\xbe\xc6\xc0\xcc\xbd\xba\xc4\xb3\xb3\xed")          },
	{ 9,  K2L_Compatibility("\xc8\xad\xc0\xcc\xc6\xae\xc0\xa7\xc1\xee")          },
	{ 10, K2L_Compatibility("\xbb\xfe\xc0\xcc\xb4\xd7\xbb\xf7\xb5\xe5")          },
	{ 11, K2L_Compatibility("\xc7\xce\xc5\xa9\xc0\xa9\xb5\xe5")                  },
	{ 12, K2L_Compatibility("\xb5\xf6\xc0\xce\xc6\xe4\xb8\xa3\xb3\xeb")          },
	{ 13, K2L_Compatibility("\xbe\xc6\xc0\xcc\xbd\xba \xbd\xba\xc6\xc4")         },
	{ 14, K2L_Compatibility("\xb7\xce\xbd\xba\xc6\xae \xbd\xc3\xbf\xfe\xc0\xcc") },
	{ 15, K2L_Compatibility("\xc0\xcc\xbd\xba\xc5\xcf \xba\xa7\xb8\xae")         },
	{ 16, K2L_Compatibility("\xbe\xc6\xc0\xcc\xbd\xba\xc0\xce\xc6\xe4\xb8\xa3\xb3\xeb") },
	{ 17, "\xc0\xa7\xc1\xee\xbd\xc3\xc6\xbc"                     }, // 983 00cf5e9a: sem espaco (chave do K2L)
	{ 18, K2L_Compatibility("\xbf\xa1\xba\xb8\xc6\xae \xb8\xb6\xc0\xce")         },
	{ 19, "Mystic Ruins"                                               }
};

static const sTypeName s_aRecordType[] = {
	{ 0, RB_DEFER_INIT("Albatross") },
	{ 1, "HoleInOne"                },
	{ 2, K2L_Compatibility("\xb0\xad\xc1\xbe\xc0\xb2")   },
	{ 3, "Level"                    },
	{ 4, "\xc3\xd1\xc5\xb8\xb1\xb8\xba\xf1\xb0\xc5\xb8\xae" }
};

static const sTypeName s_aTermType[] = {
	{ 0,
     RB_DEFER_INIT(K2L_Compatibility("\xbd\xc3\xc1\xf0"
					  "5")) }
};

static const sTypeName s_aSearchType[] = {
	{ 0, RB_DEFER_INIT(K2L_Compatibility("\xb4\xd0\xb3\xd7\xc0\xd3")) },

	{ 1, "\xbc\xf8\xc0\xa7"                        }
};

long GetRealCourseType(long course)
{
	if (course == 12)
	{
		return 13;
	}
	else if (course == 13)
	{
		return 14;
	}
	else if (course == 14)
	{
		return 15;
	}
	else if (course == 15)
	{
		return 16;
	}

	if (course == 16)
	{
		return 18;
	}
	else if (course == 17)
	{
		return 19;
	}
	else if (course == 18)
	{
		return 20;
	}
	else if (course == 19)
	{
		return 21;
	}

	return course;
}

IMPLEMENT_ACTOR(FrRankingDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrRankingDlg, FrForm)

ON_FRESH_VI("class_type", FRCMD_INIT, FrRankingDlg::OnClassType_ComboInit)
ON_FRESH_VI("class_type", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnClassType_ComboDown)
ON_FRESH_VI("btn_current_season", FRCMD_INIT,
	FrRankingDlg::OnTermType_S5_BtnInit)
ON_FRESH_VV("btn_current_season", FRCMD_LBUTTONUP,
	FrRankingDlg::OnTermType_S5_BtnUp)
ON_FRESH_VI("course", FRCMD_INIT, FrRankingDlg::OnCourseAreaInit)
ON_FRESH_VI("course", FRCMD_OWNERDRAW, FrRankingDlg::OnCourseAreaOwnerDraw)
ON_FRESH_VI("user_rank", FRCMD_INIT, FrRankingDlg::OnUserRankEditInit)
ON_FRESH_VI("user", FRCMD_INIT, FrRankingDlg::OnRankList_Init)
ON_FRESH_VI("user", FRCMD_OWNERDRAW, FrRankingDlg::OnRankList_OwnerDraw)
ON_FRESH_VV("user", FRCMD_LBUTTONDOWN, FrRankingDlg::OnRankList_LBtnDown)
ON_FRESH_VI("prev_page", FRCMD_INIT, FrRankingDlg::OnRankList_PrevPageInit)
ON_FRESH_VV("prev_page", FRCMD_LBUTTONUP,
	FrRankingDlg::OnRankList_PrevPageBtnUp)
ON_FRESH_VI("next_page", FRCMD_INIT, FrRankingDlg::OnRankList_NextPageInit)
ON_FRESH_VV("next_page", FRCMD_LBUTTONUP,
	FrRankingDlg::OnRankList_NextPageBtnUp)
ON_FRESH_VI("cur_page", FRCMD_INIT, FrRankingDlg::OnRankList_CurrentPageInit)
ON_FRESH_VI("tot_page", FRCMD_INIT, FrRankingDlg::OnRankList_TotalPageInit)
ON_FRESH_VI("rlist_myinfo", FRCMD_INIT, FrRankingDlg::OnRankList_MyInfo_Init)
ON_FRESH_VV("rlist_myinfo", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnRankList_MyInfo_LBtnDown)
ON_FRESH_VI("rlist_myinfo", FRCMD_OWNERDRAW,
	FrRankingDlg::OnRankList_MyInfo_OwnerDraw)
ON_FRESH_VI("rlist_s6", FRCMD_INIT, FrRankingDlg::OnRankList_NoInRank_Init)
ON_FRESH_VI("search_user", FRCMD_INIT, FrRankingDlg::OnRankList_SearchUserInit)
ON_FRESH_VV("search_user", FRCMD_LBUTTONUP,
	FrRankingDlg::OnRankList_SearchUserBtnUp)
ON_FRESH_VI("edit_id", FRCMD_INIT, FrRankingDlg::OnRankList_UserIdEditInit)
ON_FRESH_VV("edit_id", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnRankList_UserIdEditDown)
ON_FRESH_VI("emoticon", FRCMD_INIT, FrRankingDlg::OnEmoticonInit)
ON_FRESH_VV("emoticon", FRCMD_LBUTTONUP, FrRankingDlg::OnEmoticonBtnUp)
ON_FRESH_VI("language", FRCMD_INIT, FrRankingDlg::OnLanguageInit)
ON_FRESH_VI("pet_char", FRCMD_INIT, FrRankingDlg::OnPetCharacterInit)
ON_FRESH_VV("pet_char", FRCMD_OWNERDRAW, FrRankingDlg::OnPetCharacterOwnerDraw)
ON_FRESH_VI("pet_load", FRCMD_INIT, FrRankingDlg::OnPetLoadInit)
ON_FRESH_VI("info_up", FRCMD_INIT, FrRankingDlg::OnUpperMiniInfoInit)
ON_FRESH_VV("info_up", FRCMD_OWNERDRAW, FrRankingDlg::OnUpperMiniInfoOwnerDraw)
ON_FRESH_VI("about", FRCMD_INIT, FrRankingDlg::OnAboutBtnInit)
ON_FRESH_VV("about", FRCMD_LBUTTONUP, FrRankingDlg::OnAboutBtnUp)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrRankingDlg::OnCloseBtnUp)
ON_FRESH_VI("message", FRCMD_INIT, FrRankingDlg::OnBottomMessageInit)
ON_FRESH_VI("rank_type", FRCMD_INIT, FrRankingDlg::OnRankTypeListInit)
ON_FRESH_VV("rank_type", FRCMD_LBUTTONDOWN, FrRankingDlg::OnRankTypeListLDown)
ON_FRESH_VI("rank_type", FRCMD_OWNERDRAW, FrRankingDlg::OnRankTypeListOwnerDraw)
ON_FRESH_VI("rank_type_v", FRCMD_INIT, FrRankingDlg::OnRankTypeViewerInit)
ON_FRESH_VV("rank_type_v", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnRankTypeViewerDown)
ON_FRESH_VV("rank_type_v", FRCMD_OWNERDRAW,
	FrRankingDlg::OnRankTypeViewerOwnerDraw)
ON_FRESH_VI("gensub_type", FRCMD_INIT, FrRankingDlg::OnGeneralSubTypeListInit)
ON_FRESH_VV("gensub_type", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnGeneralSubTypeListLDown)
ON_FRESH_VI("gensub_type", FRCMD_OWNERDRAW,
	FrRankingDlg::OnGeneralSubTypeListOwnerDraw)
ON_FRESH_VI("gensub_type_v", FRCMD_INIT,
	FrRankingDlg::OnGeneralSubTypeViewerInit)
ON_FRESH_VV("gensub_type_v", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnGeneralSubTypeViewerDown)
ON_FRESH_VV("gensub_type_v", FRCMD_OWNERDRAW,
	FrRankingDlg::OnGeneralSubTypeViewerOwnerDraw)
ON_FRESH_VI("course_type", FRCMD_INIT, FrRankingDlg::OnCourseTypeListInit)
ON_FRESH_VV("course_type", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnCourseTypeListLDown)
ON_FRESH_VI("course_type", FRCMD_OWNERDRAW,
	FrRankingDlg::OnCourseTypeListOwnerDraw)
ON_FRESH_VI("course_type_v", FRCMD_INIT, FrRankingDlg::OnCourseTypeViewerInit)
ON_FRESH_VV("course_type_v", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnCourseTypeViewerDown)
ON_FRESH_VV("course_type_v", FRCMD_OWNERDRAW,
	FrRankingDlg::OnCourseTypeViewerOwnerDraw)
ON_FRESH_VI("record_type", FRCMD_INIT, FrRankingDlg::OnRecordTypeListInit)
ON_FRESH_VV("record_type", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnRecordTypeListLDown)
ON_FRESH_VI("record_type", FRCMD_OWNERDRAW,
	FrRankingDlg::OnRecordTypeListOwnerDraw)
ON_FRESH_VI("record_type_v", FRCMD_INIT, FrRankingDlg::OnRecordTypeViewerInit)
ON_FRESH_VV("record_type_v", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnRecordTypeViewerDown)
ON_FRESH_VV("record_type_v", FRCMD_OWNERDRAW,
	FrRankingDlg::OnRecordTypeViewerOwnerDraw)
ON_FRESH_VI("search_type", FRCMD_INIT, FrRankingDlg::OnSearchTypeListInit)
ON_FRESH_VV("search_type", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnSearchTypeListLDown)
ON_FRESH_VI("search_type", FRCMD_OWNERDRAW,
	FrRankingDlg::OnSearchTypeListOwnerDraw)
ON_FRESH_VI("search_type_v", FRCMD_INIT, FrRankingDlg::OnSearchTypeViewerInit)
ON_FRESH_VV("search_type_v", FRCMD_LBUTTONDOWN,
	FrRankingDlg::OnSearchTypeViewerDown)
ON_FRESH_VV("search_type_v", FRCMD_OWNERDRAW,
	FrRankingDlg::OnSearchTypeViewerOwnerDraw)
ON_FRESH_VI("ghost_btn", FRCMD_INIT, FrRankingDlg::OnGhostBtnInit)
ON_FRESH_VV("ghost_btn", FRCMD_LBUTTONDOWN, FrRankingDlg::OnGhostBtnUp)

END_FRESH_MSGMAP()

int FrRankingDlg::s_iRankType;
int FrRankingDlg::s_iSubType;
int FrRankingDlg::s_iTermType;
int FrRankingDlg::s_iClassType;
int FrRankingDlg::s_iPageNum;
int FrRankingDlg::s_iPageTotal;
int FrRankingDlg::s_iSearchType;

FrRankingDlg::FrRankingDlg()
{
	s_iRankType = 0;
	s_iSubType = 0;
	s_iTermType = 0;
	s_iClassType = 0;
	s_iPageNum = 0;
	s_iPageTotal = 0;
	s_iSearchType = 0;

	m_pExhibition = NULL;
	m_pPartTidList = NULL;
	m_pUpperMiniInfo = NULL;
	m_userInfoDelay = -1.0f;

	m_rankList.clear();
	memset(&m_userInfo, 0, sizeof(m_userInfo));
	m_userInfo.uid = 0xffffffff;
	m_myInfo.uid = 0xffffffff;

	ResetControl();

	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
	{
		WSendPacket packet((enumClientPacket)0xc1);
		packet.Encode1(2);
		packet.Send((eSendTo)0);
	}
}

FrRankingDlg::~FrRankingDlg()
{
	HidePet(true);

	g_resrcmng->UnlockAllByThread(m_anotherHand.GetThreadID());
	m_anotherHand.BreakWorkingThread();

	if (CMouseCursor::m_pInstance)
		CMouseCursor::m_pInstance->SetActive(true);

	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/) && WNetworkSystem::Instance())
	{
		WSendPacket packet((enumClientPacket)0xc1);
		packet.Encode1(1);
		packet.Send((eSendTo)0);
	}

	CTaskManager* pTaskManager = CTaskManager::Instance();
	if (pTaskManager)
		pTaskManager->PostMsg(NULL, "AvatarChat", 0x58, 0, 0, 0, 0);

	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pMyAvatar)
			pMyAvatar->SetState(pMyAvatar->m_state983 & ~0x40);
	}
}

bool FrRankingDlg::OnInit()
{
	FrForm::OnInit();

	SendRequestRank(true);
	SendRequestUserInfo(CSharedDoc::Instance()->GetPangyaUserInfo().dwUID,
		CSharedDoc::Instance()->GetPangyaUserInfo().sID);

	// 983: in the avatar chat lobby the own avatar gets state 0x40 while the ranking is open
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pMyAvatar)
			pMyAvatar->SetState(pMyAvatar->m_state983 | 0x40);
	}

	return true;
}

void FrRankingDlg::OnCancel()
{
	Close(true);
}

void FrRankingDlg::OnProc(const float dt)
{
	if (m_userInfoDelay > 0.0f)
	{
		m_userInfoDelay -= dt;

		if (m_userInfoDelay < 0.0f)
		{
			SetVisibleUserInfo(true);
			SetAllControlsEnable(true);
		}
	}

	if (g_input->GetButton(LEFT_BUTTON) || g_input->GetButton(RIGHT_BUTTON))
	{
		CloseAllComboBtns();
	}

	if (g_input->GetDown("ENTER", true) || g_input->GetDown("PADENTER", true))
	{
		if (m_pUserIdEdit->IsViewFocused())
		{
			OnRankList_SearchUserBtnUp();
			g_audio->PlaySfx("ui_icon_click", 0, 0, 0, 0, 0.5f, 200.0f);
		}
	}

	if (m_pLanguageArea)
	{
		// IME mode the indicator shows (data 0xABE7A0, no symbol)
		static bool s_bAlphaNumericMode = true;

		if (g_ime->IsAlphaNumericMode() != s_bAlphaNumericMode)
		{
			s_bAlphaNumericMode = g_ime->IsAlphaNumericMode();

			if (s_bAlphaNumericMode)
			{
				m_pLanguageArea->SetBgImg("chat_eng");
			}
			else
			{
				m_pLanguageArea->SetBgImg("chat_kor");
			}
		}
	}

	if ((g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) &&
		g_input->GetDown("EMOTICON", false))
	{
		OnEmoticonBtnUp();
	}
}

void FrRankingDlg::ResetControl()
{
	m_pTermS5Btn = 0;

	m_pRankList = 0;
	m_pMyInfoList = 0;
	m_pClassTypeCombo = 0;
	m_pCourseArea = 0;
	m_pNoMyRankWnd = 0;
	m_pPetCharacterArea = 0;
	m_pSearchUserBtn = 0;
	m_pEmoticonBtn = 0;
	m_pUserIdEdit = 0;
	m_pCurPageEdit = 0;
	m_pTotalPageEdit = 0;
	m_pUserRankEdit = 0;
	m_pRankTypeList = 0;
	m_pRankTypeViewer = 0;
	m_pGenSubTypeList = 0;
	m_pGenSubTypeViewer = 0;
	m_pCourseTypeList = 0;
	m_pCourseTypeViewer = 0;
	m_pRecordTypeList = 0;
	m_pRecordTypeViewer = 0;
	m_pSearchTypeList = 0;
	m_pSearchTypeViewer = 0;
	m_pPetLoadGauge = 0;
	m_pBottomMsg = 0;
	m_pEmoticonDlg = 0;
	m_pAboutDlg = 0;
	m_pGhostBtn = 0;

	memset(m_pRankTypeViewerImg, 0, sizeof(m_pRankTypeViewerImg));
	memset(m_pGenSubTypeViewerImg, 0, sizeof(m_pGenSubTypeViewerImg));
	memset(m_pCourseTypeViewerImg, 0, sizeof(m_pCourseTypeViewerImg));
	memset(m_pRecordTypeViewerImg, 0, sizeof(m_pRecordTypeViewerImg));
	memset(m_pSearchTypeViewerImg, 0, sizeof(m_pSearchTypeViewerImg));
}

void FrRankingDlg::SetVisibleByRankType()
{
	if (m_pGenSubTypeViewer)
	{
		m_pGenSubTypeViewer->SetVisible(s_iRankType == 0);
	}

	if (m_pCourseTypeViewer)
	{
		m_pCourseTypeViewer->SetVisible(s_iRankType == 1);
	}

	if (m_pRecordTypeViewer)
	{
		m_pRecordTypeViewer->SetVisible(s_iRankType == 2);
	}
}

void FrRankingDlg::SetVisibleUserInfo(bool bVisible)
{
	if (m_pPetCharacterArea)
	{
		m_pPetCharacterArea->SetVisible(bVisible);
	}
	if (m_pUpperMiniInfo)
	{
		m_pUpperMiniInfo->SetVisible(bVisible);
	}
	if (m_pPetLoadGauge)
	{
		// the gauge shows while the pet loads
		m_pPetLoadGauge->SetVisible(!bVisible);
		m_pPetLoadGauge->SetPos(0);
		m_pPetLoadGauge->SetDestPos(150, bVisible);
	}

	if (m_pUserRankEdit)
	{
		m_pUserRankEdit->SetVisible(bVisible);
	}
}

void FrRankingDlg::OnAnotherHand(int nWork, void* pData)
{
	if (nWork == 0)
	{
		SetPetCharacter();
	}
}

void FrRankingDlg::UpdateRankList()
{
	if (m_pRankList)
	{
		m_pRankList->ClearItem();

		int count = 0;
		for (std::list<ranktype::sListInfo>::iterator it = m_rankList.begin();
			it != m_rankList.end(); ++it)
		{
			if (count >= 12)
			{
				break;
			}

			m_pRankList->AddItem(&*it);
			count++;
		}
	}

	SynchronizeButtons();

	char text[32];
	itoa(s_iPageNum + 1, text, 10);
	m_pCurPageEdit->SetLine(1, text, 0, false, 0);
	m_pCurPageEdit->SetVisible(true);

	itoa(s_iPageTotal, text, 10);
	m_pTotalPageEdit->SetLine(1, text, 0, false, 0);
	m_pTotalPageEdit->SetVisible(true);
}

void FrRankingDlg::ClearRankList()
{
	if (m_pRankList)
	{
		m_pRankList->ClearItem();
	}

	char text[32];
	s_iPageNum = 0;
	itoa(s_iPageNum, text, 10);
	m_pCurPageEdit->SetLine(1, text, 0, false, 0);

	s_iPageTotal = 0;
	itoa(s_iPageTotal, text, 10);
	m_pTotalPageEdit->SetLine(1, text, 0, false, 0);

	SynchronizeButtons();
}

void FrRankingDlg::UpdateMyInfo()
{
	if (m_pMyInfoList)
	{
		m_pMyInfoList->ClearItem();

		if (m_myInfo.uid != -1)
		{
			m_pMyInfoList->AddItem(&m_myInfo);
			m_pMyInfoList->SetVisible(true);
			m_pNoMyRankWnd->SetVisible(false);
		}
		else
		{
			m_pMyInfoList->SetVisible(false);
			m_pNoMyRankWnd->SetVisible(true);
		}
	}
}

void FrRankingDlg::UpdateUserInfo()
{
	m_anotherHand.Work(this, 0, 0, true, -1);

	if (m_pPetCharacterArea)
	{
		m_pPetCharacterArea->SetVisible(true);
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	pGDI->SetTextStyle(1);

	char text[32];
	if (m_userInfo.rank[0].rank <= 0)
	{
		sprintf(text, K2L_Compatibility("- \xc0\xa7"));
	}
	else
	{
		sprintf(text, K2L_Compatibility("%d \xc0\xa7"), m_userInfo.rank[0].rank);
	}
	m_pUserRankEdit->SetLine(1, text, 0xffffffff, false, 0);

	pGDI->SetTextStyle(0);
}

void FrRankingDlg::SetPetCharacter()
{
	m_anotherHand.Enter();

	if (!m_pPetCharacterArea)
	{
		m_anotherHand.Leave();
		return;
	}

	m_pPetLoadGauge->SetVisible(true);

	HidePet(false);

	WRect rect = m_pPetCharacterArea->GetRect();
	m_pExhibition = new CExhibitionByHardwareTnL(rect.w * 0.5f + rect.x,
		rect.h * 0.5f + rect.y, 0.15f, true);
	m_pExhibition->SetDistance(16.25f, true, false);
	m_pExhibition->SetArea(rect.w, rect.h);

	m_pPartTidList = new CPartTidList;
	sCharacterInfo* pCharInfo = &m_userInfo.charInfo;
	for (int i = 0; i < 24; i++)
	{
		if (pCharInfo->tidParts[i] &&
			!ItemManager()->FindPart(pCharInfo->tidParts[i]))
		{
			pCharInfo->tidParts[i] = 0;
		}
	}
	if (!m_pPartTidList->SetTids(pCharInfo, 255) ||
		!m_pPartTidList->IsComboValid())
	{
		m_pPartTidList->SetDefaultTids();
	}

	m_pExhibition->SetModel(*m_pPartTidList, pCharInfo->tidAuxParts, NULL, 3.0f,
		0);

	m_anotherHand.Leave();
}

// HACK: the original emits the "no record" literal between the two literals owned by SetListItemInfo. Our
// SetListItemInfo is naked asm (no live C reference), so the literal is a local static of that (inline, for
// external linkage) function and DrawUserInfoLine reaches the same bytes through an alias.
#define RB_NO_RECORD_TEXT K2L_Compatibility("\xb1\xe2\xb7\xcf\xbe\xf8\xc0\xbd")

void FrRankingDlg::SetListItemInfo(FrListItem* pItem)
{
	ranktype::sListInfo* pInfo = (ranktype::sListInfo*)pItem->pData;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	unsigned long color = 0xffffffff;

	if (pInfo->uid == m_userInfo.uid)
	{
		WPoint pt(pItem->pos.x + 1.0f, pItem->pos.y - 5.0f);
		WSize size(m_pRankList->GetItemWidth() + 17.0f,
			(float)m_pRankList->GetItemHeight());
		WRect rect(pt, size);

		WOverlay::DrawBox(g_view, rect, 0, 0xA0FFFF00, 0.0f);
		color = 0xff000000;
	}

	pGDI->SetTextStyle(0);

	if (pInfo->prevRank != 0 &&
		!(fabs((float)pInfo->rank - (float)pInfo->prevRank) > 10000.0f))
	{
		if (pInfo->rank > pInfo->prevRank)
		{
			const Bitmap* pBmp =
				g_pFresh->GetManager()->GetBitmap("RANKING", "dn");
			if (pBmp)
			{
				WRect rect(pItem->pos.x + 45.0f, pItem->pos.y + 2.0f,
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			pGDI->SetTextColor(0xff0000ff, 0xffffffff);
			pGDI->PrintText(
				WPoint(pBmp->Width() + pItem->pos.x + 45.0f, pItem->pos.y), 0,
				MakeStr("%d", pInfo->rank - pInfo->prevRank), NULL);
		}
		else if (pInfo->rank < pInfo->prevRank)
		{
			const Bitmap* pBmp =
				g_pFresh->GetManager()->GetBitmap("RANKING", "up");
			if (pBmp)
			{
				WRect rect(pItem->pos.x + 45.0f, pItem->pos.y + 2.0f,
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			pGDI->SetTextColor(0xffff0000, 0xffffffff);
			pGDI->PrintText(
				WPoint(pBmp->Width() + pItem->pos.x + 45.0f, pItem->pos.y), 0,
				MakeStr("%d", pInfo->prevRank - pInfo->rank), NULL);
		}
		else
		{
			pGDI->SetTextColor(color, 0xffffffff);
			pGDI->PrintText(WPoint(pItem->pos.x + 53.0f, pItem->pos.y), 0, "-",
				NULL);
		}
	}
	else
	{
		const Bitmap* pBmp = g_pFresh->GetManager()->GetBitmap("ICONS", "new");
		if (pBmp)
		{
			WRect rect(pItem->pos.x + 45.0f, pItem->pos.y + 4.0f,
				(float)pBmp->Width(), (float)pBmp->Height());
			pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
		}
	}

	const char* rankText;
	if (pInfo->rank != 0 && pInfo->rank <= 10000 && pInfo->value != 0)
	{
		rankText = MakeStr("%d", pInfo->rank);
	}
	else
	{
		rankText = "-";
	}

	pGDI->SetTextColor(color, 0xffffffff);
	pGDI->SetTextStyle(1);
	pGDI->PrintText(WPoint(pItem->pos.x + 7.0f, pItem->pos.y), 0, rankText,
		NULL);
	pGDI->SetTextStyle(0);

	if (!CProjectG::Instance()->HidePrivacy())
	{
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 85.0f, pItem->pos.y), 0,
			pInfo->nickname.c_str(), -1.0f, 0xffffffff);
	}

	pGDI->SetTextColor(color, 0xffffffff);

	if (s_iRankType == 2 && s_iSubType == 3)
	{
		unsigned short level = min(pInfo->level + 1, 71);
		const Bitmap* pBmp = g_pFresh->GetManager()->GetBitmap("LEVELS",
			MakeStr("level_%03d", level));
		if (pBmp)
		{
			WRect rect(pItem->pos.x + 240.0f, pItem->pos.y - 4.0f,
				(float)pBmp->Width(), (float)pBmp->Height());
			pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
		}
	}
	else
	{
		long value = pInfo->value;
		if (value > 9999999999)
		{
			pGDI->PrintText(WPoint(pItem->pos.x + 310.0f, pItem->pos.y), 2,
				K2L_Compatibility("\xb1\xe2\xb7\xcf\xbe\xf8\xc0\xbd"), NULL);
		}
		else if (s_iRankType == 2 && s_iSubType == 2)
		{
			pGDI->PrintText(WPoint(pItem->pos.x + 310.0f, pItem->pos.y), 2,
				MakeStr("%.1f", value * 0.1f), NULL);
		}
		else if (s_iRankType == 2 && s_iSubType == 4)
		{
			pGDI->PrintText(WPoint(pItem->pos.x + 310.0f, pItem->pos.y), 2,
				MakeStr(K2L_Compatibility("%d \xb8\xb8y"), value / 10000), NULL);
		}
		else
		{
			pGDI->PrintText(WPoint(pItem->pos.x + 310.0f, pItem->pos.y), 2,
				MakeStr("%d", value), NULL);
		}
	}

	pGDI->SetTextColor(0xffffffff, 0xffffffff);
}

void FrRankingDlg::DrawUserInfoLine(int type, int line, WPoint& pos)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	pGDI->SetTextColor(0xffffffff, 0xffffffff);
	pGDI->SetTextStyle(0);

	unsigned long rank = m_userInfo.rank[type].rank;
	const char* rankText;
	if (rank != 0 && rank < 10000 && m_userInfo.rank[type].value != 0)
	{
		rankText = MakeStr(K2L_Compatibility("%d\xc0\xa7"), rank);
	}
	else
	{
		rankText = K2L_Compatibility("- ");
	}

	pGDI->SetTextColor(0xffffffff, 0xffffffff);

	pGDI->PrintText(WPoint(pos.x + 100.0f, line * 15 + pos.y), 2, rankText,
		NULL);

	if (m_userInfo.rank[type].value > 99999999)
	{
		pGDI->PrintText(WPoint(pos.x + 195.0f, line * 15 + pos.y), 2,
			RB_NO_RECORD_TEXT, NULL);
	}
	else
	{
		pGDI->PrintText(WPoint(pos.x + 195.0f, line * 15 + pos.y), 2,
			MakeStr("%d", m_userInfo.rank[type].value), NULL);
	}

	unsigned long prevRank = m_userInfo.rank[type].prevRank;
	if (prevRank != 0 && !(fabs((float)rank - (float)prevRank) > 10000.0f))
	{
		if (rank > prevRank)
		{
			const Bitmap* pBmp =
				g_pFresh->GetManager()->GetBitmap("RANKING", "dn");
			if (pBmp)
			{
				WRect rect(pos.x + 105.0f, line * 15 + pos.y + 2.0f,
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			pGDI->SetTextColor(0xff0000ff, 0xffffffff);
			pGDI->PrintText(
				WPoint(pBmp->Width() + pos.x + 105.0f, line * 15 + pos.y), 0,
				MakeStr("%d", rank - prevRank), NULL);
			pGDI->SetTextColor(0xffffffff, 0xffffffff);
		}
		else if (rank < prevRank)
		{
			const Bitmap* pBmp =
				g_pFresh->GetManager()->GetBitmap("RANKING", "up");
			if (pBmp)
			{
				WRect rect(pos.x + 105.0f, line * 15 + pos.y + 2.0f,
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			pGDI->SetTextColor(0xffff0000, 0xffffffff);
			pGDI->PrintText(
				WPoint(pBmp->Width() + pos.x + 105.0f, line * 15 + pos.y), 0,
				MakeStr("%d", prevRank - rank), NULL);
			pGDI->SetTextColor(0xffffffff, 0xffffffff);
		}
		else
		{
			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->PrintText(WPoint(pos.x + 113.0f, line * 15 + pos.y), 0, "-",
				NULL);
		}
	}
	else
	{
		const Bitmap* pBmp = g_pFresh->GetManager()->GetBitmap("ICONS", "new");
		if (pBmp)
		{
			WRect rect(pos.x + 105.0f, line * 15 + pos.y + 3.0f,
				(float)pBmp->Width(), (float)pBmp->Height());
			pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
		}
	}
}

void FrRankingDlg::ShowUserInfo()
{
	// HACK: dead reference; the original compiles this function one wave later (emission order)
	if (0)
		((FrRankingDlg*)0)->FrRankingDlg::SetPetCharacter();
	if (m_userInfo.uid == 0xffffffff)
	{
		return;
	}

	WPoint pt = m_pUpperMiniInfo->GetRect().TopLeft();

	WPoint pos(pt.x, pt.y + 33.0f);
	DrawUserInfoLine(1, 0, pos);
	DrawUserInfoLine(2, 1, pos);
	DrawUserInfoLine(3, 2, pos);
	DrawUserInfoLine(4, 3, pos);
	DrawUserInfoLine(5, 4, pos);
	pos.y += 4.0f;
	DrawUserInfoLine(0, 5, pos);
}

void FrRankingDlg::DisconnectRankServer()
{
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_RANK))
	{
		WNetworkSystem::Instance()->ForceShutDown(WNetworkSystem::NET_RANK);
	}
}

void FrRankingDlg::SetRankTypes(unsigned char rankType, unsigned char subType,
	unsigned char termType, unsigned char classType, int pageNum, int pageTotal)
{
	s_iRankType = rankType;
	s_iSubType = subType;
	s_iTermType = termType;
	s_iClassType = classType;
	s_iPageNum = pageNum;
	s_iPageTotal = pageTotal;
}

void FrRankingDlg::SelectRankList(int index)
{
	if (m_pRankList)
	{
		FrListItem* pItem = m_pRankList->GetItem(index);
		if (!pItem)
		{
			return;
		}

		m_pRankList->SelectItem(pItem, true);
	}

	FrListItem* pSelected = m_pRankList->GetSelected();
	ranktype::sListInfo* pInfo = (ranktype::sListInfo*)pSelected->pData;
	if (pInfo)
	{
		SendRequestUserInfo(pInfo->uid, pInfo->id.c_str());
	}
}

void FrRankingDlg::SynchronizeButtons()
{
	if (m_pRankTypeList)
	{
		const char* name =
			((FrLine*)m_pRankTypeList->GetSelected()->pData)->text.c_str();
		if (strcmpi(K2L_Compatibility(s_aRankType[s_iRankType].m_name), name) == 0)
		{
			m_pRankTypeList->SelectItem(m_pRankTypeList->GetItem(s_iRankType),
				true);
			m_pRankTypeList->SetVisible(false);
			SetVisibleByRankType();
		}
	}

	switch (s_iRankType)
	{
	case 0:
		if (m_pGenSubTypeList)
		{
			m_pGenSubTypeList->SelectItem(
				m_pGenSubTypeList->GetItem(s_iSubType), true);
			m_pGenSubTypeList->SetVisible(false);
		}
		break;

	case 1:
		if (m_pCourseTypeList)
		{
			m_pCourseTypeList->SelectItem(
				m_pCourseTypeList->GetItem(s_iSubType), true);
			m_pCourseTypeList->SetVisible(false);
		}
		break;

	case 2:
		if (m_pRecordTypeList)
		{
			// the record list has no entry for sub type 1 (983: index - 1 from 2 up)
			int recIdx = s_iSubType;
			if (recIdx >= 2)
				recIdx--;
			m_pRecordTypeList->SelectItem(
				m_pRecordTypeList->GetItem(recIdx), true);
			m_pRecordTypeList->SetVisible(false);
		}
		break;
	}
}

void FrRankingDlg::SetAllControlsEnable(bool bEnable)
{
	m_pTermS5Btn->Enable(bEnable);

	m_pRankList->Enable(bEnable);
	m_pMyInfoList->Enable(bEnable);
	m_pSearchUserBtn->Enable(bEnable);
	m_pRankTypeList->Enable(bEnable);
	m_pRankTypeViewer->Enable(bEnable);
	m_pGenSubTypeList->Enable(bEnable);
	m_pGenSubTypeViewer->Enable(bEnable);
	m_pCourseTypeList->Enable(bEnable);
	m_pCourseTypeViewer->Enable(bEnable);
	m_pRecordTypeList->Enable(bEnable);
	m_pRecordTypeViewer->Enable(bEnable);
	m_pSearchTypeList->Enable(bEnable);
	m_pSearchTypeViewer->Enable(bEnable);
}

void FrRankingDlg::CloseAllComboBtns()
{
	const WPoint& mousePos = g_pFresh->GetManager()->GetMousePos();

	if (m_pRankTypeList && m_pRankTypeViewer && m_pRankTypeList->IsVisible())
	{
		WRect rect;
		rect = m_pRankTypeViewer->GetRect();
		if (!rect.IsInRect(mousePos))
		{
			rect = m_pRankTypeList->GetRect();
			if (!rect.IsInRect(mousePos))
			{
				m_pRankTypeList->SetVisible(false);
			}
		}
	}

	if (m_pGenSubTypeList && m_pGenSubTypeList->IsVisible())
	{
		WRect rect;
		rect = m_pGenSubTypeViewer->GetRect();
		if (!rect.IsInRect(mousePos))
		{
			rect = m_pGenSubTypeList->GetRect();
			if (!rect.IsInRect(mousePos))
			{
				m_pGenSubTypeList->SetVisible(false);
			}
		}
	}

	if (m_pCourseTypeList && m_pCourseTypeList->IsVisible())
	{
		WRect rect;
		rect = m_pCourseTypeViewer->GetRect();
		if (!rect.IsInRect(mousePos))
		{
			rect = m_pCourseTypeList->GetRect();
			if (!rect.IsInRect(mousePos))
			{
				m_pCourseTypeList->SetVisible(false);
			}
		}
	}

	if (m_pRecordTypeList && m_pRecordTypeList->IsVisible())
	{
		WRect rect;
		rect = m_pRecordTypeViewer->GetRect();
		if (!rect.IsInRect(mousePos))
		{
			rect = m_pRecordTypeList->GetRect();
			if (!rect.IsInRect(mousePos))
			{
				m_pRecordTypeList->SetVisible(false);
			}
		}
	}

	if (m_pSearchTypeList && m_pSearchTypeList->IsVisible())
	{
		WRect rect;
		rect = m_pSearchTypeViewer->GetRect();
		if (!rect.IsInRect(mousePos))
		{
			rect = m_pSearchTypeList->GetRect();
			if (!rect.IsInRect(mousePos))
			{
				m_pSearchTypeList->SetVisible(false);
			}
		}
	}
}

__declspec(noinline) void FrRankingDlg::HidePet(bool bHide)
{
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = 0;
	}
	if (m_pPartTidList)
	{
		delete m_pPartTidList;
		m_pPartTidList = 0;
	}
}

void FrRankingDlg::SendRequestRank(bool bFirst)
{
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_RANK))
	{
		return;
	}

	WSendPacket packet(0);

	unsigned long uid = CSharedDoc::Instance()->GetPangyaUserInfo().dwUID;
	packet.Encode4(uid);
	packet.EncodeStr(
		std::string(CSharedDoc::Instance()->GetPangyaUserInfo().sID));
	packet.Encode1(s_iRankType);
	packet.Encode1(s_iSubType);
	packet.Encode1(s_iTermType);
	packet.Encode1(s_iClassType);
	packet.Encode4(s_iPageNum);
	packet.Encode1(bFirst);
	packet.Send(TO_RANK);

	m_userInfoDelay = 1.0f;
	SetAllControlsEnable(false);

	CRankingInfo::Instance()->GetDlg()->SetBottomMsg("");
}

void FrRankingDlg::SendRequestUserInfo(unsigned long uid, const char* nickname)
{
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_RANK))
	{
		return;
	}

	if (uid == 0xffffffff)
	{
		return;
	}

	WSendPacket packet(1);
	packet.Encode4(uid);
	packet.EncodeStr(nickname);
	packet.Encode1(s_iTermType);
	packet.Send(TO_RANK);

	m_userInfoDelay = 1.0f;
	SetVisibleUserInfo(false);
	SetAllControlsEnable(false);

	CRankingInfo::Instance()->GetDlg()->SetBottomMsg("");
}

void FrRankingDlg::SendRequestSearchUser(const char* nickname)
{
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_RANK))
	{
		return;
	}

	// 983 (00b293e0): empty text and the special characters share the one "special symbols" notice
	if (0 == strcmp(nickname, "") || strstr(nickname, "'") ||
		strstr(nickname, ";") || strstr(nickname, "\""))
	{
		OnNotify(
			K2L_Compatibility("\xc6\xaf\xbc\xf6\xb1\xe2\xc8\xa3 ' ; \" \xb4\xc2 \xc0\xd4\xb7\xc2\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			2);
		return;
	}

	WSendPacket packet(2);
	packet.Encode1(s_iSearchType);

	if (s_iSearchType == 1)
	{
		int rank = atoi(nickname);
		if (rank <= 0)
		{
			OnNotify(
				K2L_Compatibility("\xbc\xfd\xc0\xda\xb0\xa1 \xbe\xc6\xb4\xcf\xb0\xc5\xb3\xaa \xb0\xaa\xc0\xcc \xb3\xca\xb9\xab \xc0\xdb\xbd\xc0\xb4\xcf\xb4\xd9."),
				2);
			return;
		}
		if (rank > 100000)
		{
			OnNotify(
				K2L_Compatibility("\xc3\xa3\xb4\xc2 \xbc\xf8\xc0\xa7\xc0\xc7 \xb0\xaa\xc0\xcc \xb3\xca\xb9\xab \xc5\xae\xb4\xcf\xb4\xd9."),
				2);
			return;
		}
		packet.Encode4(rank);
	}
	else
	{
		packet.EncodeStr(std::string(nickname));
	}

	packet.Encode1(s_iRankType);
	packet.Encode1(s_iSubType);
	packet.Encode1(s_iTermType);
	packet.Encode1(s_iClassType);
	packet.Encode4(s_iPageNum);
	packet.Send(TO_RANK);

	m_userInfoDelay = 1.0f;
	SetAllControlsEnable(false);

	CRankingInfo::Instance()->GetDlg()->SetBottomMsg("");
}

void FrRankingDlg::OnTermType_S5_BtnInit(int param)
{
	m_pTermS5Btn = DYNAMIC_CAST(FrButton, param);
	m_pTermS5Btn->SetPushDelay(1.0f);

	m_pTermS5Btn->SetStatus(FrButton::PRESSED);
}

void FrRankingDlg::OnTermType_S5_BtnUp()
{
	s_iTermType = 0;

	m_pTermS5Btn->SetStatus(FrButton::PRESSED);

	s_iPageNum = 0;
	SendRequestRank(true);
	SendRequestUserInfo(m_userInfo.uid, m_userInfo.id);
}

void FrRankingDlg::OnCourseAreaInit(int param)
{
	m_pCourseArea = DYNAMIC_CAST(FrArea, param);
}

void FrRankingDlg::OnCourseAreaOwnerDraw(int param)
{
	// HACK: dead reference; the original compiles this empty handler in the last wave, right after
	// FrRankingDlgMakeInstance and ??_G (emission order)
	if (0)
		FrRankingDlgMakeInstance();
}

void FrRankingDlg::OnRankList_Init(int param)
{
	if (m_pRankList)
	{
		m_pRankList->ClearItem();
	}
	else
	{
		m_pRankList = DYNAMIC_CAST(FrListBox, param);
	}
}

void FrRankingDlg::OnRankList_OwnerDraw(int param)
{
	if (m_rankList.empty())
	{
		return;
	}

	if (!param)
	{
		return;
	}

	SetListItemInfo((FrListItem*)param);
}

void FrRankingDlg::OnRankList_LBtnDown()
{
	if (!(m_userInfoDelay > 0.0f) && !m_anotherHand.IsWorking(0))
	{
		FrListItem* pItem = m_pRankList->GetItemUnderCursor();
		if (!pItem)
		{
			return;
		}

		ranktype::sListInfo* pInfo = (ranktype::sListInfo*)pItem->pData;
		if (!pInfo)
		{
			return;
		}

		if (strcmpi(pInfo->id.c_str(), m_userInfo.id) == 0)
		{
			return;
		}

		SendRequestUserInfo(pInfo->uid, pInfo->id.c_str());
	}
}

void FrRankingDlg::OnRankList_PrevPageInit(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, param);
	pBtn->SetPushDelay(1.0f);
}

void FrRankingDlg::OnRankList_PrevPageBtnUp()
{
	if (s_iPageNum > 0)
	{
		s_iPageNum--;
		SendRequestRank(false);
	}
	else
	{
		// "There are no more ranking records."
		SetBottomMsg(K2L_Compatibility(
			"\xB4\xF5 \xC0\xCC\xBB\xF3 \xB7\xA9\xC5\xB7 \xB1\xE2\xB7\xCF\xC0\xCC \xBE\xF8\xBD\xC0\xB4\xCF\xB4\xD9."));
	}
}

void FrRankingDlg::OnRankList_NextPageInit(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, param);
	pBtn->SetPushDelay(1.0f);
}

void FrRankingDlg::OnRankList_NextPageBtnUp()
{
	s_iPageNum++;
	SendRequestRank(false);
}

void FrRankingDlg::OnRankList_CurrentPageInit(int param)
{
	m_pCurPageEdit = DYNAMIC_CAST(FrEdit, param);
}

void FrRankingDlg::OnRankList_TotalPageInit(int param)
{
	m_pTotalPageEdit = DYNAMIC_CAST(FrEdit, param);
}

void FrRankingDlg::OnRankList_MyInfo_Init(int param)
{
	if (m_pMyInfoList)
	{
		m_pMyInfoList->ClearItem();
	}
	else
	{
		m_pMyInfoList = DYNAMIC_CAST(FrListBox, param);
	}

	m_pMyInfoList->UseRightButton(true);
}

void FrRankingDlg::OnRankList_MyInfo_LBtnDown()
{
	if (!(m_userInfoDelay > 0.0f) && !m_anotherHand.IsWorking(0))
	{
		FrListItem* pItem = m_pMyInfoList->GetItemUnderCursor();
		if (!pItem)
		{
			return;
		}

		ranktype::sListInfo* pInfo = (ranktype::sListInfo*)pItem->pData;
		if (!pInfo)
		{
			return;
		}

		if (strcmpi(pInfo->id.c_str(), m_userInfo.id) == 0)
		{
			return;
		}

		SendRequestUserInfo(pInfo->uid, pInfo->id.c_str());
	}
}

void FrRankingDlg::OnRankList_MyInfo_OwnerDraw(int param)
{
	if (param)
	{
		SetListItemInfo((FrListItem*)param);
	}
}

void FrRankingDlg::OnRankList_NoInRank_Init(int param)
{
	m_pNoMyRankWnd = DYNAMIC_CAST(FrStatic, param);
	m_pNoMyRankWnd->SetVisible(false);
}

void FrRankingDlg::OnRankList_SearchUserInit(int param)
{
	m_pSearchUserBtn = DYNAMIC_CAST(FrButton, param);
	m_pSearchUserBtn->SetPushDelay(0.1f);
}

void FrRankingDlg::OnRankList_SearchUserBtnUp()
{
	char nickname[260];
	nickname[0] = 0;

	if (m_pUserIdEdit)
	{
		strcpy(nickname, m_pUserIdEdit->GetLine(1, false));
		TrimA(nickname, " \t\n\r\0");
	}

	SendRequestSearchUser(nickname);
}

void FrRankingDlg::OnRankList_UserIdEditInit(int param)
{
	m_pUserIdEdit = DYNAMIC_CAST(FrEdit, param);
	m_pUserIdEdit->SetCharLimit(0x20, true);
}

void FrRankingDlg::OnRankList_UserIdEditDown()
{
}

void FrRankingDlg::OnEmoticonInit(int param)
{
	m_pEmoticonBtn = DYNAMIC_CAST(FrButton, param);
}

void FrRankingDlg::OnEmoticonBtnUp()
{
	if (m_pEmoticonBtn && !m_pEmoticonDlg)
	{
		m_pEmoticonDlg = CreateForm<FrEmoticonDlg>(g_pFresh->GetManager(), this,
			"emoticon", 0);
		if (!m_pEmoticonDlg)
		{
			return;
		}

		FrButton* pBtn = m_pEmoticonBtn;
		WPoint pos(0.0f, 0.0f);
		if (pBtn)
		{
			pos.x = pBtn->GetRect().x - 200.0f;
			pos.y = pBtn->GetRect().y - 260.0f;
		}

		m_pEmoticonDlg->Open((FRESH_PFN_RESULT)&FrRankingDlg::OnEmoticonResult,
			pos, 3);
	}
}

void FrRankingDlg::OnLanguageInit(int param)
{
	m_pLanguageArea = DYNAMIC_CAST(FrArea, param);
	if (m_pLanguageArea)
	{
		if (g_ime->IsAlphaNumericMode())
			m_pLanguageArea->SetBgImg("chat_eng");
		else
			m_pLanguageArea->SetBgImg("chat_kor");
	}
}

void FrRankingDlg::OnAboutBtnInit(int param)
{
	if ((__rtti_obj = (void*)param) != 0 &&
		((IObject*)__rtti_obj)->IsKindOf(&FrButton::m_RTTI))
	{
	}
}

void FrRankingDlg::OnAboutBtnUp()
{
	m_pAboutDlg = CreateForm<FrRankingAboutDlg>(g_pFresh->GetManager(), this,
		"ranking_about", 0);

	if (m_pAboutDlg)
	{
		m_pAboutDlg->Open((FRESH_PFN_RESULT)&FrRankingDlg::OnAboutDlgResult, 3);
	}
}

void FrRankingDlg::OnCloseBtnUp()
{
	Close(true);
}

void FrRankingDlg::OnBottomMessageInit(int param)
{
	m_pBottomMsg = DYNAMIC_CAST(FrStatic, param);
	m_pBottomMsg->SetVisible(false);
}

void FrRankingDlg::OnPetCharacterInit(int param)
{
	m_pPetCharacterArea = DYNAMIC_CAST(FrArea, param);
}

void FrRankingDlg::OnPetCharacterOwnerDraw()
{
	if (m_pPetLoadGauge && m_pPetLoadGauge->IsVisible())
	{
		return;
	}

	if (!m_anotherHand.IsWorking(0))
	{
		m_anotherHand.Enter();
		if (m_pExhibition)
		{
			m_pExhibition->Display(0.0f, 0.0f);
		}
		m_anotherHand.Leave();
	}
}

void FrRankingDlg::OnPetLoadInit(int param)
{
	m_pPetLoadGauge = DYNAMIC_CAST(FrGaugeBar, param);
	if (m_pPetLoadGauge)
	{
		m_pPetLoadGauge->SetRange(0, 150, 0);
		m_pPetLoadGauge->SetVisible(false);
	}
}

void FrRankingDlg::OnUpperMiniInfoInit(int param)
{
	m_pUpperMiniInfo = DYNAMIC_CAST(FrArea, param);
}

void FrRankingDlg::OnUpperMiniInfoOwnerDraw()
{
	if (m_pUpperMiniInfo)
	{
		WRect rc = m_pUpperMiniInfo->GetRect();
		WPoint pos(rc.x, rc.y);
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		if (m_userInfo.uid != 0xffffffff)
		{
			unsigned char level = min(m_userInfo.level + 1, 71);
			const Bitmap* pBmp = g_pFresh->GetManager()->GetBitmap("LEVELS",
				MakeStr("level_%03d", level));
			if (pBmp)
			{
				WRect rect(pos.x + 10.0f, pos.y + 6.0f, (float)pBmp->Width(),
					(float)pBmp->Height());
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}

			char text[256];
			sprintf(text, K2L_Compatibility("%s"), m_userInfo.nickname);

			pGDI->SetTextStyle(0);

			if (!CProjectG::Instance()->HidePrivacy())
			{
				g_pFresh->GetManager()->PrintText(
					WPoint(pos.x + 80.0f, pos.y + 9.0f), 0, text, -1.0f,
					0xffffffff);
			}
		}

		ShowUserInfo();
	}
}

void FrRankingDlg::OnClassType_ComboInit(int param)
{
	m_pClassTypeCombo = DYNAMIC_CAST(FrComboBox, param);

	for (int i = 0; i < 4; i++)
	{
		m_pClassTypeCombo->AddString(K2L_Compatibility(s_aClassType[i].m_name));
	}

	m_pClassTypeCombo->SetLine(1, K2L_Compatibility(s_aClassType[s_iClassType].m_name), 0, false,
		0);
	m_pClassTypeCombo->SetVisible(false);
}

void FrRankingDlg::OnClassType_ComboDown(int param)
{
	const char* text = m_pClassTypeCombo->GetLine(1, false);

	int prevClassType = s_iClassType;
	s_iClassType = m_pClassTypeCombo->FindString(text) - 1;
	if (s_iClassType < 0)
	{
		s_iClassType = 0;
	}
	else if (s_iClassType > 3)
	{
		s_iClassType = 3;
	}

	if (s_iClassType != prevClassType)
	{
		SendRequestRank(true);
	}
}

void FrRankingDlg::OnRankTypeListInit(int param)
{
	m_pRankTypeList = DYNAMIC_CAST(FrListBox, param);

	if (m_pRankTypeList)
	{
		for (int i = 0; i < 3; i++)
		{
			m_rankTypeLine[i].text = std::string(K2L_Compatibility(s_aRankType[i].m_name));
			m_pRankTypeList->AddItem(&m_rankTypeLine[i]);
		}

		m_pRankTypeList->SelectItem(
			m_pRankTypeList->FindItem(&m_rankTypeLine[0]), true);
		m_pRankTypeList->SetVisible(false);
		m_pRankTypeList->EnableScrollBar(false);
	}
}

void FrRankingDlg::OnRankTypeListLDown()
{
	FrListItem* pItem = m_pRankTypeList->GetItemUnderCursor();

	if (pItem)
	{
		m_pRankTypeList->SelectItem(pItem, true);
		m_pRankTypeList->SetVisible(false);

		s_iRankType = pItem->idx;
		if (s_iRankType < 0)
		{
			s_iRankType = 0;
		}
		else if (s_iRankType > 2)
		{
			s_iRankType = 2;
		}

		s_iSubType = 0;
		s_iPageNum = 0;
		SendRequestRank(true);
		SetVisibleByRankType();
	}
}

void FrRankingDlg::OnRankTypeListOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (pItem)
		{
			FrLine* pLine = (FrLine*)pItem->pData;

			WOverlay::DrawBox(g_view,
				RbRect(WPoint(pItem->pos.x, pItem->pos.y), 82.0f, 22.0f), 0,
				0xAA0E4F75, 0.0f);

			if (pItem->underCursor)
			{
				pGDI->Box(WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 80.0f,
							  20.0f),
					0x80082B46, 0, 0.0f);
				pGDI->SetTextStyle(1);
			}
			else
			{
				pGDI->SetTextStyle(0);
			}

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pItem->pos.x + 10.0f, pItem->pos.y + 6.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnRankTypeViewerInit(int param)
{
	m_pRankTypeViewer = DYNAMIC_CAST(FrViewer, param);

	m_pRankTypeViewerImg[0] = g_pFresh->RegisterBitmap("btn_drop01_n");
	m_pRankTypeViewerImg[1] = g_pFresh->RegisterBitmap("btn_drop01_o");
	m_pRankTypeViewerImg[2] = g_pFresh->RegisterBitmap("btn_drop01_d");
}

void FrRankingDlg::OnRankTypeViewerDown()
{
	if (m_pRankTypeList)
	{
		m_pRankTypeList->SetVisible(!m_pRankTypeList->IsVisible());
		g_audio->PlaySfx("ui_icon_click", 0, 0, 0, 0, 0.5f, 200.0f);
	}
}

void FrRankingDlg::OnRankTypeViewerOwnerDraw()
{
	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pGDI = pManager->GetGDI();

	if (pGDI)
	{
		FrViewer* pViewer = m_pRankTypeViewer;
		if (pViewer)
		{
			int state = 0;
			if (m_pRankTypeList->IsVisible())
			{
				state = 2;
			}
			else
			{
				WRect rect = pViewer->GetRect();

				if (rect.IsInRect(pManager->GetMousePos()))
				{
					state = 1;
				}
			}

			pGDI->DrawTexture(m_pRankTypeViewerImg[state], pViewer->GetRect(),
				0xffffffff, 0);

			FrLine* pLine = (FrLine*)m_pRankTypeList->GetSelected()->pData;
			WRect viewerRect = m_pRankTypeViewer->GetRect();
			WPoint pt(viewerRect.x, viewerRect.y);
			RbRect viewRect(pt, viewerRect.w, viewerRect.h);

			WOverlay::DrawBox(g_view, viewRect, 0, 0x800E4F75, 0.0f);
			pGDI->SetTextStyle(1);

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pt.x + 4.0f, pt.y + 5.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnGeneralSubTypeListInit(int param)
{
	m_pGenSubTypeList = DYNAMIC_CAST(FrListBox, param);

	if (m_pGenSubTypeList)
	{
		for (int i = 0; i < 6; i++) // 983 00b2a103: 6 linhas (0xf03984..0xf039b4)
		{
			m_genSubTypeLine[i].text = std::string(K2L_Compatibility(s_aGenSubType[i].m_name));
			m_pGenSubTypeList->AddItem(&m_genSubTypeLine[i]);
		}

		m_pGenSubTypeList->SelectItem(
			m_pGenSubTypeList->FindItem(&m_genSubTypeLine[0]), true);
		m_pGenSubTypeList->SetVisible(false);
		m_pGenSubTypeList->EnableScrollBar(false);
	}
}

void FrRankingDlg::OnGeneralSubTypeListLDown()
{
	FrListItem* pItem = m_pGenSubTypeList->GetItemUnderCursor();

	if (pItem)
	{
		m_pGenSubTypeList->SelectItem(pItem, true);
		m_pGenSubTypeList->SetVisible(false);

		s_iSubType = pItem->idx;
		if (s_iSubType < 0)
		{
			s_iSubType = 0;
		}
		else if (s_iSubType > 5)
		{
			s_iSubType = 5;
		}

		s_iPageNum = 0;
		SendRequestRank(true);
	}
}

void FrRankingDlg::OnGeneralSubTypeListOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (pItem)
		{
			FrLine* pLine = (FrLine*)pItem->pData;

			WOverlay::DrawBox(g_view,
				RbRect(WPoint(pItem->pos.x, pItem->pos.y), 91.0f, 22.0f), 0,
				0xAA0E4F75, 0.0f);

			if (pItem->underCursor)
			{
				pGDI->Box(WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 90.0f,
							  20.0f),
					0x80082B46, 0, 0.0f);
				pGDI->SetTextStyle(1);
			}
			else
			{
				pGDI->SetTextStyle(0);
			}

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pItem->pos.x + 10.0f, pItem->pos.y + 6.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnGeneralSubTypeViewerInit(int param)
{
	m_pGenSubTypeViewer = DYNAMIC_CAST(FrViewer, param);

	m_pGenSubTypeViewerImg[0] = g_pFresh->RegisterBitmap("btn_drop02_n");
	m_pGenSubTypeViewerImg[1] = g_pFresh->RegisterBitmap("btn_drop02_o");
	m_pGenSubTypeViewerImg[2] = g_pFresh->RegisterBitmap("btn_drop02_d");
}

void FrRankingDlg::OnGeneralSubTypeViewerDown()
{
	if (m_pGenSubTypeList)
	{
		m_pGenSubTypeList->SetVisible(!m_pGenSubTypeList->IsVisible());
		g_audio->PlaySfx("ui_icon_click", 0, 0, 0, 0, 0.5f, 200.0f);
	}
}

void FrRankingDlg::OnGeneralSubTypeViewerOwnerDraw()
{
	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pGDI = pManager->GetGDI();

	if (pGDI)
	{
		FrViewer* pViewer = m_pGenSubTypeViewer;
		if (pViewer)
		{
			int state = 0;
			if (m_pGenSubTypeList->IsVisible())
			{
				state = 2;
			}
			else
			{
				WRect rect = pViewer->GetRect();

				if (rect.IsInRect(pManager->GetMousePos()))
				{
					state = 1;
				}
			}

			pGDI->DrawTexture(m_pGenSubTypeViewerImg[state], pViewer->GetRect(),
				0xffffffff, 0);

			FrLine* pLine = (FrLine*)m_pGenSubTypeList->GetSelected()->pData;
			WRect viewerRect = m_pGenSubTypeViewer->GetRect();
			WPoint pt(viewerRect.x, viewerRect.y);
			RbRect viewRect(pt, viewerRect.w, viewerRect.h);

			WOverlay::DrawBox(g_view, viewRect, 0, 0x800E4F75, 0.0f);
			pGDI->SetTextStyle(1);

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pt.x + 4.0f, pt.y + 5.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnCourseTypeListInit(int param)
{
	m_pCourseTypeList = DYNAMIC_CAST(FrListBox, param);

	if (m_pCourseTypeList)
	{
		for (int i = 0; i < 20; i++)
		{
			long course = i;
			course = GetRealCourseType(course);
			IFF_STRUCT::sCourse* pCourse =
				ItemManager()->FindCourse(0x28000000 | course);
			if (pCourse)
			{
				m_courseTypeLine[i].text = std::string(K2L_Compatibility(s_aCourseType[i].m_name));
				m_pCourseTypeList->AddItem(&m_courseTypeLine[i]);
			}
		}

		const WRect& rect = m_pCourseTypeList->GetRect();

		m_pCourseTypeList->SetRect(
			RbRect(WPoint(rect.x, rect.y), 107.0f, 440.0f));

		m_pCourseTypeList->SelectItem(
			m_pCourseTypeList->FindItem(&m_courseTypeLine[0]), true);
		m_pCourseTypeList->SetVisible(false);
		m_pCourseTypeList->EnableScrollBar(false);
	}
}

void FrRankingDlg::OnCourseTypeListLDown()
{
	FrListItem* pItem = m_pCourseTypeList->GetItemUnderCursor();

	if (pItem)
	{
		m_pCourseTypeList->SelectItem(pItem, true);
		m_pCourseTypeList->SetVisible(false);

		s_iSubType = pItem->idx;
		if (s_iSubType < 0)
		{
			s_iSubType = 0;
		}
		else if (s_iSubType > 19)
		{
			s_iSubType = 19;
		}

		s_iPageNum = 0;
		SendRequestRank(true);
	}
}

void FrRankingDlg::OnCourseTypeListOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (pItem)
		{
			FrLine* pLine = (FrLine*)pItem->pData;

			WOverlay::DrawBox(g_view,
				RbRect(WPoint(pItem->pos.x, pItem->pos.y), 91.0f, 22.0f), 0,
				0xAA0E4F75, 0.0f);

			if (pItem->underCursor)
			{
				pGDI->Box(WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 90.0f,
							  20.0f),
					0x80082B46, 0, 0.0f);
				pGDI->SetTextStyle(1);
			}
			else
			{
				pGDI->SetTextStyle(0);
			}

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pItem->pos.x + 10.0f, pItem->pos.y + 6.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnCourseTypeViewerInit(int param)
{
	m_pCourseTypeViewer = DYNAMIC_CAST(FrViewer, param);

	m_pCourseTypeViewerImg[0] = g_pFresh->RegisterBitmap("btn_drop02_n");
	m_pCourseTypeViewerImg[1] = g_pFresh->RegisterBitmap("btn_drop02_o");
	m_pCourseTypeViewerImg[2] = g_pFresh->RegisterBitmap("btn_drop02_d");
}

void FrRankingDlg::OnCourseTypeViewerDown()
{
	if (m_pCourseTypeList)
	{
		m_pCourseTypeList->SetVisible(!m_pCourseTypeList->IsVisible());
		g_audio->PlaySfx("ui_icon_click", 0, 0, 0, 0, 0.5f, 200.0f);
	}
}

void FrRankingDlg::OnCourseTypeViewerOwnerDraw()
{
	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pGDI = pManager->GetGDI();

	if (pGDI)
	{
		FrViewer* pViewer = m_pCourseTypeViewer;
		if (pViewer)
		{
			int state = 0;
			if (m_pCourseTypeList->IsVisible())
			{
				state = 2;
			}
			else
			{
				WRect rect = pViewer->GetRect();

				if (rect.IsInRect(pManager->GetMousePos()))
				{
					state = 1;
				}
			}

			pGDI->DrawTexture(m_pCourseTypeViewerImg[state], pViewer->GetRect(),
				0xffffffff, 0);

			FrLine* pLine = (FrLine*)m_pCourseTypeList->GetSelected()->pData;
			WRect viewerRect = m_pCourseTypeViewer->GetRect();
			WPoint pt(viewerRect.x, viewerRect.y);
			RbRect viewRect(pt, viewerRect.w, viewerRect.h);

			WOverlay::DrawBox(g_view, viewRect, 0, 0x800E4F75, 0.0f);
			pGDI->SetTextStyle(1);

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pt.x + 4.0f, pt.y + 5.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnRecordTypeListInit(int param)
{
	m_pRecordTypeList = DYNAMIC_CAST(FrListBox, param);

	if (m_pRecordTypeList)
	{
		for (int i = 0; i < 5; i++)
		{
			if (i == 2)
			{
				continue;
			}
			m_recordTypeLine[i].text = std::string(K2L_Compatibility(s_aRecordType[i].m_name));
			m_pRecordTypeList->AddItem(&m_recordTypeLine[i]);
		}

		m_pRecordTypeList->SelectItem(
			m_pRecordTypeList->FindItem(&m_recordTypeLine[0]), true);
		m_pRecordTypeList->SetVisible(false);
		m_pRecordTypeList->EnableScrollBar(false);
	}
}

void FrRankingDlg::OnRecordTypeListLDown()
{
	FrListItem* pItem = m_pRecordTypeList->GetItemUnderCursor();

	if (pItem)
	{
		m_pRecordTypeList->SelectItem(pItem, true);
		m_pRecordTypeList->SetVisible(false);

		s_iSubType = pItem->idx;
		if (s_iSubType > 1)
		{
			s_iSubType++;
		}
		if (s_iSubType < 0)
		{
			s_iSubType = 0;
		}
		else if (s_iSubType > 4)
		{
			s_iSubType = 4;
		}

		s_iPageNum = 0;
		SendRequestRank(true);
	}
}

void FrRankingDlg::OnRecordTypeListOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (pItem)
		{
			FrLine* pLine = (FrLine*)pItem->pData;

			WOverlay::DrawBox(g_view,
				RbRect(WPoint(pItem->pos.x, pItem->pos.y), 91.0f, 22.0f), 0,
				0xAA0E4F75, 0.0f);

			if (pItem->underCursor)
			{
				pGDI->Box(WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 90.0f,
							  20.0f),
					0x80082B46, 0, 0.0f);
				pGDI->SetTextStyle(1);
			}
			else
			{
				pGDI->SetTextStyle(0);
			}

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pItem->pos.x + 10.0f, pItem->pos.y + 6.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnRecordTypeViewerInit(int param)
{
	m_pRecordTypeViewer = DYNAMIC_CAST(FrViewer, param);

	m_pRecordTypeViewerImg[0] = g_pFresh->RegisterBitmap("btn_drop02_n");
	m_pRecordTypeViewerImg[1] = g_pFresh->RegisterBitmap("btn_drop02_o");
	m_pRecordTypeViewerImg[2] = g_pFresh->RegisterBitmap("btn_drop02_d");
}

void FrRankingDlg::OnRecordTypeViewerDown()
{
	if (m_pRecordTypeList)
	{
		m_pRecordTypeList->SetVisible(!m_pRecordTypeList->IsVisible());
		g_audio->PlaySfx("ui_icon_click", 0, 0, 0, 0, 0.5f, 200.0f);
	}
}

void FrRankingDlg::OnRecordTypeViewerOwnerDraw()
{
	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pGDI = pManager->GetGDI();

	if (pGDI)
	{
		FrViewer* pViewer = m_pRecordTypeViewer;
		if (pViewer)
		{
			int state = 0;
			if (m_pRecordTypeList->IsVisible())
			{
				state = 2;
			}
			else
			{
				WRect rect = pViewer->GetRect();

				if (rect.IsInRect(pManager->GetMousePos()))
				{
					state = 1;
				}
			}

			pGDI->DrawTexture(m_pRecordTypeViewerImg[state], pViewer->GetRect(),
				0xffffffff, 0);

			FrLine* pLine = (FrLine*)m_pRecordTypeList->GetSelected()->pData;
			WRect viewerRect = m_pRecordTypeViewer->GetRect();
			WPoint pt(viewerRect.x, viewerRect.y);
			RbRect viewRect(pt, viewerRect.w, viewerRect.h);

			WOverlay::DrawBox(g_view, viewRect, 0, 0x800E4F75, 0.0f);
			pGDI->SetTextStyle(1);

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pt.x + 6.0f, pt.y + 5.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnSearchTypeListInit(int param)
{
	m_pSearchTypeList = DYNAMIC_CAST(FrListBox, param);

	if (m_pSearchTypeList)
	{
		for (int i = 0; i < 2; i++)
		{
			m_searchTypeLine[i].text = std::string(K2L_Compatibility(s_aSearchType[i].m_name));
			m_pSearchTypeList->AddItem(&m_searchTypeLine[i]);
		}

		m_pSearchTypeList->SelectItem(
			m_pSearchTypeList->FindItem(&m_searchTypeLine[0]), true);
		m_pSearchTypeList->SetVisible(false);
		m_pSearchTypeList->EnableScrollBar(false);
	}
}

void FrRankingDlg::OnSearchTypeListLDown()
{
	FrListItem* pItem = m_pSearchTypeList->GetItemUnderCursor();

	if (pItem)
	{
		m_pSearchTypeList->SelectItem(pItem, true);
		m_pSearchTypeList->SetVisible(false);

		s_iSearchType = pItem->idx;
	}
}

void FrRankingDlg::OnSearchTypeListOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

	if (pGDI)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (pItem)
		{
			FrLine* pLine = (FrLine*)pItem->pData;

			WOverlay::DrawBox(g_view,
				RbRect(WPoint(pItem->pos.x, pItem->pos.y), 62.0f, 22.0f), 0,
				0xAA0E4F75, 0.0f);

			if (pItem->underCursor)
			{
				pGDI->Box(WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 62.0f,
							  20.0f),
					0x80082B46, 0, 0.0f);
				pGDI->SetTextStyle(1);
			}
			else
			{
				pGDI->SetTextStyle(0);
			}

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pItem->pos.x + 10.0f, pItem->pos.y + 6.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnSearchTypeViewerInit(int param)
{
	m_pSearchTypeViewer = DYNAMIC_CAST(FrViewer, param);

	m_pSearchTypeViewerImg[0] = g_pFresh->RegisterBitmap("btn_drop03_n");
	m_pSearchTypeViewerImg[1] = g_pFresh->RegisterBitmap("btn_drop03_o");
	m_pSearchTypeViewerImg[2] = g_pFresh->RegisterBitmap("btn_drop03_d");
}

void FrRankingDlg::OnSearchTypeViewerDown()
{
	if (m_pSearchTypeList)
	{
		m_pSearchTypeList->SetVisible(!m_pSearchTypeList->IsVisible());
		g_audio->PlaySfx("ui_icon_click", 0, 0, 0, 0, 0.5f, 200.0f);
	}
}

void FrRankingDlg::OnSearchTypeViewerOwnerDraw()
{
	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pGDI = pManager->GetGDI();

	if (pGDI)
	{
		FrViewer* pViewer = m_pSearchTypeViewer;
		if (pViewer)
		{
			int state = 0;
			if (m_pSearchTypeList->IsVisible())
			{
				state = 2;
			}
			else
			{
				WRect rect = pViewer->GetRect();

				if (rect.IsInRect(pManager->GetMousePos()))
				{
					state = 1;
				}
			}

			pGDI->DrawTexture(m_pSearchTypeViewerImg[state], pViewer->GetRect(),
				0xffffffff, 0);

			FrLine* pLine = (FrLine*)m_pSearchTypeList->GetSelected()->pData;
			WRect viewerRect = m_pSearchTypeViewer->GetRect();
			WPoint pt(viewerRect.x, viewerRect.y);
			RbRect viewRect(pt, viewerRect.w, viewerRect.h);

			WOverlay::DrawBox(g_view, viewRect, 0, 0x000E4F75, 0.0f);
			pGDI->SetTextStyle(1);

			pGDI->SetTextColor(0xffffffff, 0xffffffff);
			pGDI->Print(WPoint(pt.x + 4.0f, pt.y + 5.0f), 0,
				pLine->text.c_str());
		}
	}
}

void FrRankingDlg::OnUserRankEditInit(int param)
{
	m_pUserRankEdit = DYNAMIC_CAST(FrEdit, param);
	m_pUserRankEdit->SetVisible(false);
}

void FrRankingDlg::OnNotify(const char* msg, unsigned long param)
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
	pForm->SetTimeLimit(1.0f);
	pForm->SetMessage(msg, false);
	pForm->Open((FRESH_PFN_RESULT)0, param);

	m_pBottomMsg->SetVisible(false);
}

void FrRankingDlg::SetBottomMsg(const char* msg)
{
	m_pBottomMsg->SetCaption(msg);
	m_pBottomMsg->SetVisible(true);
}

void FrRankingDlg::OnCustomProc(float dt)
{
	if (m_anotherHand.IsWorking(0))
	{
		if (m_pGhostBtn && m_pGhostBtn->IsVisible())
		{
			m_pGhostBtn->SetVisible(false);
		}

		Sleep(10);
		return;
	}

	m_anotherHand.Enter();

	if (m_pExhibition && m_pPetLoadGauge && !m_pPetLoadGauge->IsVisible())
	{
		WRect rect = m_pPetCharacterArea->GetRect();
		m_pExhibition->SetCenter(rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f);
		m_pExhibition->Process(dt, true);
	}

	m_anotherHand.Leave();
}

bool FrRankingDlg::OnEmoticonResult(int ret, FrForm* pForm)
{
	if (m_pEmoticonDlg && ret == 1)
	{
		FrEmoticonDlg* pDlg = DYNAMIC_CAST(FrEmoticonDlg, pForm);
		if (pDlg)
		{
			const char* icon = pDlg->GetSelectedIcon();
			if (icon)
			{
				FrEdit* pEdit = m_pUserIdEdit;

				if (pEdit)
				{
					const char* text = pEdit->GetEditText_Front();
					pEdit->SetLine(1, MakeStr("%s%s", text, icon), 0, false, 0);
					pEdit->SetKeyFocus(true);
				}
			}
		}
	}

	m_pEmoticonDlg = 0;
	return true;
}

bool FrRankingDlg::OnAboutDlgResult(int ret, FrForm* pForm)
{
	m_pAboutDlg = 0;
	return true;
}

void FrRankingDlg::OnGhostBtnInit(int param)
{
	m_pGhostBtn = DYNAMIC_CAST(FrButton, param);
	m_pGhostBtn->SetVisible(false);
	m_pGhostBtn->Enable(false);
}

void FrRankingDlg::OnGhostBtnUp()
{
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), 0, "notify", 0);
	pForm->SetTimeLimit(5.0f);
	// "The service is being prepared."
	pForm->SetMessage(K2L_Compatibility(
		"\xBC\xAD\xBA\xF1\xBD\xBA \xC1\xD8\xBA\xF1 \xC1\xDF\xC0\xD4\xB4\xCF\xB4\xD9."),
		false);
	pForm->Open((FRESH_PFN_RESULT)0, 3);
}

CRankingInfo::CRankingInfo()
{
	m_pDlg = 0;
}

bool CRankingInfo::Open()
{
	if (!m_pDlg)
	{
		m_pDlg = CreateForm<FrRankingDlg>(g_pFresh->GetManager(), this,
			"rankingdlg", 0);
		m_pDlg->Open((FRESH_PFN_RESULT)&CRankingInfo::OnRankingDlgResult, 1);
		m_pDlg->EnableDrag(false);

		if ((__rtti_obj = AfxGetTask()) != 0
				? ((IObject*)__rtti_obj)
					  ->IsExactKindOf(&ntAvatarChatTask::m_RTTI)
				: false)
		{
			ntAvatarManager::Instance()->m_bActive = false;
		}
	}

	return true;
}

void CRankingInfo::Close()
{
	if (m_pDlg)
	{
		m_pDlg->Close(true);
	}
}

bool CRankingInfo::OnRankingDlgResult(int ret, FrForm* pForm)
{
	m_pDlg->DisconnectRankServer();
	m_pDlg = 0;

	if ((__rtti_obj = AfxGetTask()) != 0
			? ((IObject*)__rtti_obj)->IsExactKindOf(&ntAvatarChatTask::m_RTTI)
			: false)
	{
		ntAvatarManager::Instance()->m_bActive = true;
	}
	return true;
}

void CRankingInfo::Process(float dt)
{
	if (m_pDlg)
	{
		m_pDlg->OnCustomProc(dt);
	}
}

void CRankingInfo::OnNotify(const char* msg, unsigned long param)
{
	if (m_pDlg)
	{
		m_pDlg->OnNotify(msg, param);
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetTimeLimit(3.0f);
		pForm->SetMessage(msg, false);
		pForm->Open((FRESH_PFN_RESULT)0, param);
	}
}

IMPLEMENT_OBJECT(FrRankingAboutDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrRankingAboutDlg, FrForm)

ON_FRESH_VI("aboutviewer", FRCMD_INIT, FrRankingAboutDlg::OnAboutViewerInit)
ON_FRESH_VI("index01", FRCMD_INIT, FrRankingAboutDlg::OnIndex01Init)
ON_FRESH_VV("index01", FRCMD_LBUTTONUP, FrRankingAboutDlg::OnIndex01BnUp)
ON_FRESH_VI("index02", FRCMD_INIT, FrRankingAboutDlg::OnIndex02Init)
ON_FRESH_VV("index02", FRCMD_LBUTTONUP, FrRankingAboutDlg::OnIndex02BnUp)
ON_FRESH_VI("index03", FRCMD_INIT, FrRankingAboutDlg::OnIndex03Init)
ON_FRESH_VV("index03", FRCMD_LBUTTONUP, FrRankingAboutDlg::OnIndex03BnUp)
ON_FRESH_VI("index04", FRCMD_INIT, FrRankingAboutDlg::OnIndex04Init)
ON_FRESH_VV("index04", FRCMD_LBUTTONUP, FrRankingAboutDlg::OnIndex04BnUp)
ON_FRESH_VI("backward", FRCMD_INIT, FrRankingAboutDlg::OnBackwardBnInit)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP, FrRankingAboutDlg::OnBackwardBnUp)
ON_FRESH_VI("forward", FRCMD_INIT, FrRankingAboutDlg::OnForwardBnInit)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrRankingAboutDlg::OnForwardBnUp)
ON_FRESH_VI("ok", FRCMD_INIT, FrRankingAboutDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrRankingAboutDlg::OnOkBnUp)

END_FRESH_MSGMAP()

FrRankingAboutDlg::FrRankingAboutDlg()
{
	for (int i = 0; i < 4; i++)
	{
		m_pIndexBtn[i] = 0;
	}
	m_aboutIndex = 0;
	m_pBackwardBtn = 0;
	m_pForwardBtn = 0;
}

FrRankingAboutDlg::~FrRankingAboutDlg()
{
}

void FrRankingAboutDlg::OnAboutViewerInit(int param)
{
	m_pAboutViewer = DYNAMIC_CAST(FrViewer, param);
}

void FrRankingAboutDlg::OnIndex01Init(int param)
{
	m_pIndexBtn[0] = DYNAMIC_CAST(FrTextButton, param);
}

void FrRankingAboutDlg::OnIndex01BnUp()
{
	SetImgAboutIndex(0);
}

void FrRankingAboutDlg::OnIndex02Init(int param)
{
	m_pIndexBtn[1] = DYNAMIC_CAST(FrTextButton, param);
}

void FrRankingAboutDlg::OnIndex02BnUp()
{
	SetImgAboutIndex(1);
}

void FrRankingAboutDlg::OnIndex03Init(int param)
{
	m_pIndexBtn[2] = DYNAMIC_CAST(FrTextButton, param);
}

void FrRankingAboutDlg::OnIndex03BnUp()
{
	SetImgAboutIndex(2);
}

void FrRankingAboutDlg::OnIndex04Init(int param)
{
	m_pIndexBtn[3] = DYNAMIC_CAST(FrTextButton, param);
}

void FrRankingAboutDlg::OnIndex04BnUp()
{
	SetImgAboutIndex(3);
}

void FrRankingAboutDlg::OnBackwardBnInit(int param)
{
	m_pBackwardBtn = DYNAMIC_CAST(FrButton, param);
	m_pBackwardBtn->SetPushDelay(0.0f);
	m_pBackwardBtn->UseDblClick(false); // 983 00b26e60: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrRankingAboutDlg::OnBackwardBnUp()
{
	SetImgAboutIndex(--m_aboutIndex);
}

void FrRankingAboutDlg::OnForwardBnInit(int param)
{
	m_pForwardBtn = DYNAMIC_CAST(FrButton, param);
}

void FrRankingAboutDlg::OnForwardBnUp()
{
	SetImgAboutIndex(++m_aboutIndex);
}

void FrRankingAboutDlg::OnOkBnInit(int param)
{
	if ((__rtti_obj = (void*)param) != 0 &&
		((IObject*)__rtti_obj)->IsKindOf(&FrButton::m_RTTI))
	{
	}
}

void FrRankingAboutDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrRankingAboutDlg::OnInit()
{
	SetImgAboutIndex(0);
	return true;
}

void FrRankingAboutDlg::SetImgAboutIndex(int index)
{
	for (int i = 0; i < 4; i++)
	{
		m_pIndexBtn[i]->Unselect(0);
	}

	if (m_pBackwardBtn)
	{
		m_pBackwardBtn->Enable(true);
	}

	if (m_pForwardBtn)
	{
		m_pForwardBtn->Enable(true);
	}

	if (index <= 0)
	{
		index = 0;
		m_pIndexBtn[0]->Select(0);

		if (m_pBackwardBtn)
		{
			m_pBackwardBtn->Enable(false);
		}
	}
	else if (index >= 3)
	{
		index = 3;
		m_pIndexBtn[3]->Select(0);

		if (m_pForwardBtn)
		{
			m_pForwardBtn->Enable(false);
		}
	}

	m_pIndexBtn[index]->Select(0);

	m_aboutIndex = index;
	if (m_pAboutViewer)
	{
		m_pAboutViewer->Open(MakeStr("help_ranking_%02d.jpg", index + 1));
	}
}

// D867: taskmanager.cpp (pacotes 0x1389/0x138a/0x138c) le a lista/minha linha/usuario do dialogo pelos offsets do VC10
// (+0x114/+0x120/+0x16c, a classe so e visivel aqui). No cl novo std::list/std::string mudam de tamanho: acessores.
C_ASSERT_VC10(offsetof(FrRankingDlg, m_rankList) == 0x114);
C_ASSERT_VC10(offsetof(FrRankingDlg, m_myInfo) == 0x120);
C_ASSERT_VC10(offsetof(FrRankingDlg, m_userInfo) == 0x16c);
#if _MSC_VER > 1600
void* __fastcall RankList983(FrRankingDlg* p) { return &p->m_rankList; }
void* __fastcall RankMyInfo983(FrRankingDlg* p) { return &p->m_myInfo; }
void* __fastcall RankUserInfo983(FrRankingDlg* p) { return &p->m_userInfo; }
#endif
