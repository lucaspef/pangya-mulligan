#include "minatl.h"
#include "vc10_layout.h"
#include "frform.h"
#include "frwndmanager.h"
#include "userinfoform.h"
#include "exhibition.h"
#include "matchhistory.h"
#include "addfrienddlg.h"
#include "commonutil.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "fredit.h"
#include "frarea.h"
#include "frstatic.h"
#include "frlistbox.h"
#include "frgaugebar.h"
#include "shareddoc.h"
#include "projectg.h"
#include "golftask.h"
#include "lobbytask.h"
#include "avatar_task.h"
#include "chatmsg.h"
#include "actor.h"
#include <algorithm>
#include "uccmanager.h"
#include "partsinfo.h"
#include "netresourcemanager.h"
#include "buddymanager.h"
#include "s5/utilities.h"
#include "jp/gamestringmanager.h" // GetGameString 00540a10


extern Fresh* g_pFresh;

// HACK: 228.0f and "???" are defined as function-local statics aliased to the literal names in
// realmyroommain.cpp / headicon.cpp (naked asm there); using the literals here would emit competing
// COMDATs that the linker would keep in this object.
#define RB_228F 228.0f

IMPLEMENT_OBJECT(FrUserInfoForm, FrForm)

int float2int(float f);
BEGIN_FRESH_MSGMAP(FrUserInfoForm, FrForm)

ON_FRESH_VI("friend", FRCMD_INIT, FrUserInfoForm::OnFriendInit)
ON_FRESH_VV("friend", FRCMD_LBUTTONUP, FrUserInfoForm::OnFriendUp)
ON_FRESH_VI("whisper", FRCMD_INIT, FrUserInfoForm::OnWhisperInit)
ON_FRESH_VV("whisper", FRCMD_LBUTTONUP, FrUserInfoForm::OnWhisperUp)
ON_FRESH_VI("ignore", FRCMD_INIT, FrUserInfoForm::OnIgnoreInit)
ON_FRESH_VV("ignore", FRCMD_LBUTTONUP, FrUserInfoForm::OnIgnoreUp)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrUserInfoForm::OnCloseUp)
ON_FRESH_VI("ghost", FRCMD_INIT, FrUserInfoForm::OnGhostInit)
ON_FRESH_VV("ghost", FRCMD_LBUTTONUP, FrUserInfoForm::OnGhostdUp)
ON_FRESH_VI("pet", FRCMD_INIT, FrUserInfoForm::OnPetInit)
ON_FRESH_VV("pet", FRCMD_OWNERDRAW, FrUserInfoForm::OnPetOwnerDraw)
ON_FRESH_VI("pet_load", FRCMD_INIT, FrUserInfoForm::OnPetLoadInit)
ON_FRESH_VI("nick", FRCMD_INIT, FrUserInfoForm::OnNickInit)
ON_FRESH_VI("nick", FRCMD_OWNERDRAW, FrUserInfoForm::OnNickOwnerDraw)
ON_FRESH_VI("general", FRCMD_INIT, FrUserInfoForm::OnGeneralInfoInit)
ON_FRESH_VV("general", FRCMD_LBUTTONDOWN, FrUserInfoForm::OnGeneralInfoDown)
ON_FRESH_VI("special", FRCMD_INIT, FrUserInfoForm::OnSpecialInfoInit)
ON_FRESH_VV("special", FRCMD_LBUTTONDOWN, FrUserInfoForm::OnSpecialInfoDown)
ON_FRESH_VI("record", FRCMD_INIT, FrUserInfoForm::OnRecordInfoInit)
ON_FRESH_VV("record", FRCMD_LBUTTONDOWN, FrUserInfoForm::OnRecordInfoDown)
ON_FRESH_VI("trophy", FRCMD_INIT, FrUserInfoForm::OnTrophyInfoInit)
ON_FRESH_VV("trophy", FRCMD_LBUTTONDOWN, FrUserInfoForm::OnTrophyInfoDown)
ON_FRESH_VI("guild", FRCMD_INIT, FrUserInfoForm::OnGuildInfoInit)
ON_FRESH_VV("guild", FRCMD_LBUTTONDOWN, FrUserInfoForm::OnGuildInfoDown)
ON_FRESH_VI("general_back", FRCMD_INIT, FrUserInfoForm::OnGeneralBackInit)
ON_FRESH_VI("rec_course_back", FRCMD_INIT, FrUserInfoForm::OnRecCourseBackInit)
ON_FRESH_VI("tro_normal_back", FRCMD_INIT, FrUserInfoForm::OnTroNormalBackInit)
ON_FRESH_VI("tro_special_back", FRCMD_INIT,
	FrUserInfoForm::OnTroSpecialBackInit)
ON_FRESH_VI("guild_back", FRCMD_INIT, FrUserInfoForm::OnGuildBackInit)
// PKT (frente C, lote 37): botao das conquistas do 983 (mapa 00cdf4d0, entre "guild_back" e "btn_season")
ON_FRESH_VI("btn_achievement", FRCMD_INIT, FrUserInfoForm::OnAchievementBtnInit983)
ON_FRESH_VV("btn_achievement", FRCMD_LBUTTONUP, FrUserInfoForm::OnAchievementBtnUp983)
ON_FRESH_VI("btn_season", FRCMD_INIT, FrUserInfoForm::OnTitleSeasonBtnInit)
ON_FRESH_VV("btn_season", FRCMD_LBUTTONUP, FrUserInfoForm::OnTitleSeasonBtnUp)
ON_FRESH_VI("btn_total", FRCMD_INIT, FrUserInfoForm::OnTitleTotalBtnInit)
ON_FRESH_VV("btn_total", FRCMD_LBUTTONUP, FrUserInfoForm::OnTitleTotalBtnUp)
ON_FRESH_VI("eg0", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_ID)
ON_FRESH_VI("eg1", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_SCHOOL)
ON_FRESH_VI("eg2", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_CLUB)
ON_FRESH_VI("eg3", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_EXP)
ON_FRESH_VI("eg4", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_CONNSTATUS)
ON_FRESH_VI("eg5", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_DISCONN)
ON_FRESH_VI("eg6", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_COMBO)
ON_FRESH_VI("eg7", FRCMD_INIT, FrUserInfoForm::OnEditInit_GI_QUIZLEVEL)
ON_FRESH_VI("general_level", FRCMD_INIT, FrUserInfoForm::OnAreaInit_GI_Level)
ON_FRESH_VI("general_guild", FRCMD_INIT, FrUserInfoForm::OnAreaInit_GI_ClUB)
ON_FRESH_VI("general_guild", FRCMD_OWNERDRAW,
	FrUserInfoForm::OnAreaDraw_GI_ClUB)
ON_FRESH_VI("general_question", FRCMD_INIT,
	FrUserInfoForm::OnAreaInit_Question_mark)
ON_FRESH_VI("Btn_rmrvisitor", FRCMD_INIT, FrUserInfoForm::OnBtn_RmrVisitorInit)
ON_FRESH_VV("Btn_rmrvisitor", FRCMD_LBUTTONUP,
	FrUserInfoForm::OnBtn_RmrVisitorBtnUp)
ON_FRESH_VI("speclist", FRCMD_INIT, FrUserInfoForm::OnSpecListboxInit)
ON_FRESH_VI("speclist", FRCMD_OWNERDRAW, FrUserInfoForm::OnSpecListboxOwnerDraw)
ON_FRESH_VI("es0_1", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_TROPHY_GOLD)
ON_FRESH_VI("es0_2", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_TROPHY_SILVER)
ON_FRESH_VI("es0_3", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_TROPHY_BRONZE)
ON_FRESH_VI("es0_4", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_AWARD_4)
ON_FRESH_VI("es0_5", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_AWARD_5)
ON_FRESH_VI("es0_6", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_AWARD_6)
ON_FRESH_VI("es1", FRCMD_INIT,
	FrUserInfoForm::OnEditInit_SI_MATCH_WIN_PERCENTAGE)
ON_FRESH_VI("es2", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_AVG_SCORE)
ON_FRESH_VI("es3", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_MEAN_SHOTTIME)
ON_FRESH_VI("es4", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_PANGYA_PERCENTAGE)
ON_FRESH_VI("es5", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_FAIRWAY_PERCENTAGE)
ON_FRESH_VI("es6", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_PUTT_PERCENTAGE)
ON_FRESH_VI("es7", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_OB_PERCENTAGE)
ON_FRESH_VI("es8", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_TOTAL_SKINS_PANG)
ON_FRESH_VI("mh1", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY1)
ON_FRESH_VI("mh2", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY2)
ON_FRESH_VI("mh3", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY3)
ON_FRESH_VI("mh4", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY4)
ON_FRESH_VI("mh5", FRCMD_INIT, FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY5)
ON_FRESH_VI("btn_si1", FRCMD_INIT, FrUserInfoForm::OnMatchHistoryBtn1Init)
ON_FRESH_VV("btn_si1", FRCMD_LBUTTONUP, FrUserInfoForm::OnMatchHistoryBtn1Up)
ON_FRESH_VI("btn_si2", FRCMD_INIT, FrUserInfoForm::OnMatchHistoryBtn2Init)
ON_FRESH_VV("btn_si2", FRCMD_LBUTTONUP, FrUserInfoForm::OnMatchHistoryBtn2Up)
ON_FRESH_VI("btn_si3", FRCMD_INIT, FrUserInfoForm::OnMatchHistoryBtn3Init)
ON_FRESH_VV("btn_si3", FRCMD_LBUTTONUP, FrUserInfoForm::OnMatchHistoryBtn3Up)
ON_FRESH_VI("btn_si4", FRCMD_INIT, FrUserInfoForm::OnMatchHistoryBtn4Init)
ON_FRESH_VV("btn_si4", FRCMD_LBUTTONUP, FrUserInfoForm::OnMatchHistoryBtn4Up)
ON_FRESH_VI("btn_si5", FRCMD_INIT, FrUserInfoForm::OnMatchHistoryBtn5Init)
ON_FRESH_VV("btn_si5", FRCMD_LBUTTONUP, FrUserInfoForm::OnMatchHistoryBtn5Up)
ON_FRESH_VI("recordlist", FRCMD_INIT, FrUserInfoForm::OnRecTotalListboxInit)
ON_FRESH_VI("recordlist", FRCMD_OWNERDRAW,
	FrUserInfoForm::OnRecTotalListboxOwnerDraw)
ON_FRESH_VI("btn_rec0", FRCMD_INIT, FrUserInfoForm::OnRecordNormalBtnInit)
ON_FRESH_VV("btn_rec0", FRCMD_LBUTTONUP, FrUserInfoForm::OnRecordNormalBtnUp)
ON_FRESH_VI("btn_rec1", FRCMD_INIT, FrUserInfoForm::OnRecordCourseBtnInit)
ON_FRESH_VV("btn_rec1", FRCMD_LBUTTONUP, FrUserInfoForm::OnRecordCourseBtnUp)
ON_FRESH_VI("btn_rec2", FRCMD_INIT, FrUserInfoForm::OnRecordClassicBtnInit)
ON_FRESH_VV("btn_rec2", FRCMD_LBUTTONUP, FrUserInfoForm::OnRecordClassicBtnUp)
ON_FRESH_VI("er0", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_TOT_PLAYTIME)
ON_FRESH_VI("er0", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er0", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er1", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_TOT_SHOTS)
ON_FRESH_VI("er1", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er1", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er2", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_TOT_HOLES)
ON_FRESH_VI("er2", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er2", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er3_0", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_EARTH_ROUND)
ON_FRESH_VI("er3_0", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er3_0", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er3_1", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_TOT_DRIVE)
ON_FRESH_VI("er3_1", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er3_1", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er4", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_BEST_SCORE_3)
ON_FRESH_VI("er4", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er4", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er5", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_BEST_SCORE_2)
ON_FRESH_VI("er6", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_BEST_SCORE_1)
ON_FRESH_VI("er7", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_TOT_HOLEINONES)
ON_FRESH_VI("er7", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er7", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er8", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_TOT_ALBATROSS)
ON_FRESH_VI("er8", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er8", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er9", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_LONGEST_DRIVE)
ON_FRESH_VI("er9", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er9", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er10", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_LONGEST_PUTT)
ON_FRESH_VI("er10", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er10", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er11", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_LONGEST_CHIPIN)
ON_FRESH_VI("er11", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("er11", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("er12", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_BEST_SCORE_5)
ON_FRESH_VI("er13", FRCMD_INIT, FrUserInfoForm::OnEditInit_RI_BEST_SCORE_4)
ON_FRESH_VI("era0", FRCMD_INIT, FrUserInfoForm::OnEditInit_RA0)
ON_FRESH_VI("era0", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("era0", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("era1", FRCMD_INIT, FrUserInfoForm::OnEditInit_RA1)
ON_FRESH_VI("era1", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("era1", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("era2", FRCMD_INIT, FrUserInfoForm::OnEditInit_RA2)
ON_FRESH_VI("era2", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("era2", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("era3", FRCMD_INIT, FrUserInfoForm::OnEditInit_RA3)
ON_FRESH_VI("era3", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("era3", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("era4", FRCMD_INIT, FrUserInfoForm::OnEditInit_RA4)
ON_FRESH_VI("era4", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("era4", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("era5", FRCMD_INIT, FrUserInfoForm::OnEditInit_RA5)
ON_FRESH_VI("era5", FRCMD_HOVERON, FrUserInfoForm::OnEditInit_HoverOn)
ON_FRESH_VI("era5", FRCMD_HOVEROFF, FrUserInfoForm::OnEditInit_HoverOff)
ON_FRESH_VI("rec_award_back", FRCMD_INIT, FrUserInfoForm::OnRecAwardBackInit)
ON_FRESH_VI("btn_rec_award", FRCMD_INIT, FrUserInfoForm::OnRecordAwardBtnInit)
ON_FRESH_VV("btn_rec_award", FRCMD_LBUTTONUP, FrUserInfoForm::OnRecordAwardBtnUp)
ON_FRESH_VI("edit_gp_normal", FRCMD_INIT, FrUserInfoForm::OnGpNumInit0)
ON_FRESH_VI("edit_gp_special", FRCMD_INIT, FrUserInfoForm::OnGpNumInit1)
ON_FRESH_VI("btn_gp_normal", FRCMD_INIT, FrUserInfoForm::OnGpBtnInit0)
ON_FRESH_VI("btn_gp_special", FRCMD_INIT, FrUserInfoForm::OnGpBtnInit1)
ON_FRESH_VV("btn_gp_normal", FRCMD_LBUTTONUP, FrUserInfoForm::OnGpBtnUp0)
ON_FRESH_VV("btn_gp_special", FRCMD_LBUTTONUP, FrUserInfoForm::OnGpBtnUp1)
ON_FRESH_VI("listbox_gp_normal", FRCMD_INIT, FrUserInfoForm::OnGpBoxInit0)
ON_FRESH_VI("listbox_gp_normal", FRCMD_OWNERDRAW, FrUserInfoForm::OnGpNormalBoxOwnerDraw983)
ON_FRESH_VI("listbox_gp_special", FRCMD_INIT, FrUserInfoForm::OnGpBoxInit1)
ON_FRESH_VI("listbox_gp_special", FRCMD_OWNERDRAW, FrUserInfoForm::OnGpSpecialBoxOwnerDraw983)
ON_FRESH_VI("scorelist", FRCMD_INIT, FrUserInfoForm::OnScoreListInit)
ON_FRESH_VI("scorelist", FRCMD_OWNERDRAW, FrUserInfoForm::OnScoreListOwnerDraw)
ON_FRESH_VI("tri_normalnum", FRCMD_INIT, FrUserInfoForm::OnTrophyNormalNumInit)
ON_FRESH_VI("tri_specialnum", FRCMD_INIT,
	FrUserInfoForm::OnTrophySpecialNumInit)
ON_FRESH_VI("btn_normal", FRCMD_INIT, FrUserInfoForm::OnTrophyNormalBtnInit)
ON_FRESH_VV("btn_normal", FRCMD_LBUTTONUP, FrUserInfoForm::OnTrophyNormalBtnUp)
ON_FRESH_VI("btn_special", FRCMD_INIT, FrUserInfoForm::OnTrophySpecialBtnInit)
ON_FRESH_VV("btn_special", FRCMD_LBUTTONUP,
	FrUserInfoForm::OnTrophySpecialBtnUp)
ON_FRESH_VI("trispecialbox", FRCMD_INIT,
	FrUserInfoForm::OnListBoxInit_TRI_SPECIALBOX)
ON_FRESH_VI("trispecialbox", FRCMD_OWNERDRAW,
	FrUserInfoForm::OnListBoxOwnerDraw_TRI_SPECIALBOX)
ON_FRESH_VI("trinormalbox", FRCMD_INIT,
	FrUserInfoForm::OnListBoxInit_TRI_NORMALBOX)
ON_FRESH_VI("trinormalbox", FRCMD_OWNERDRAW,
	FrUserInfoForm::OnListBoxOwnerDraw_TRI_NORMALBOX)
ON_FRESH_VI("trinotrophy", FRCMD_INIT,
	FrUserInfoForm::OnStaticInit_TRI_NOTROPHY)
ON_FRESH_VI("eguild0", FRCMD_INIT, FrUserInfoForm::OnEditInit_GUILD_NAME)
ON_FRESH_VI("eguild1", FRCMD_INIT, FrUserInfoForm::OnEditInit_GUILD_PANG)
ON_FRESH_VI("eguild2", FRCMD_INIT, FrUserInfoForm::OnEditInit_GUILD_MYPERCENT)
ON_FRESH_VI("guildnotrophy", FRCMD_INIT,
	FrUserInfoForm::OnStaticInit_GUILD_NOTROPHY)
ON_FRESH_VI("myguild", FRCMD_INIT, FrUserInfoForm::OnMyGuildInit)
ON_FRESH_VV("myguild", FRCMD_OWNERDRAW, FrUserInfoForm::OnMyGuildOwnerDraw)
ON_FRESH_VI("guildlist", FRCMD_INIT, FrUserInfoForm::OnGuildTrophyListInit)
ON_FRESH_VI("guildlist", FRCMD_OWNERDRAW,
	FrUserInfoForm::OnGuildTrophyListOwnerDraw)
ON_FRESH_VI("comment", FRCMD_INIT, FrUserInfoForm::OnCommentInit)

END_FRESH_MSGMAP()

FrUserInfoForm::FrUserInfoForm()
{
	m_seasonType = SEASON_CURRENT;
	memset(m_pTitleBtn, 0, sizeof(m_pTitleBtn));
	memset(m_pTabBtn, 0, sizeof(m_pTabBtn));
	memset(m_pBackArea, 0, sizeof(m_pBackArea));
	memset(m_pGeneralEdit, 0, sizeof(m_pGeneralEdit));
	memset(m_pSpecialEdit, 0, sizeof(m_pSpecialEdit));
	memset(m_pRecordBtn, 0, sizeof(m_pRecordBtn));
	memset(m_pSpecialEdit, 0, sizeof(m_pSpecialEdit));
	memset(m_pRecordEdit, 0, sizeof(m_pRecordEdit));
	memset(m_pTrophyBtn, 0, sizeof(m_pTrophyBtn));
	memset(m_pGpTrophyBox983, 0, sizeof(m_pGpTrophyBox983));
	memset(m_pGpTrophyNum983, 0, sizeof(m_pGpTrophyNum983));
	memset(m_pGuildEdit, 0, sizeof(m_pGuildEdit));
	memset(m_pRecTotalBack, 0, sizeof(m_pRecTotalBack));
	memset(m_pRecAwardWnd, 0, sizeof(m_pRecAwardWnd));
	m_pRecTotalListBox = NULL;
	m_pLevelArea = NULL;
	m_pClubArea = NULL;
	m_pQuestionMark = NULL;
	m_pSpecialTrophyBox = NULL;
	m_pNormalTrophyBox = NULL;
	m_pNoTrophy = NULL;
	m_pNormalTrophyNum = NULL;
	m_pSpecialTrophyNum = NULL;
	m_pGuildNoTrophy = NULL;
	m_recordType = 0;
	m_trophyType = 0;
	m_pGuildTrophyList = NULL;
	m_oldSeason.Clear();
	m_totalSeason.Clear();
	memset(&m_userInfo, 0, sizeof(m_userInfo));
	m_userInfo.info.dwGuid = 0xffffffff;
	m_userInfo.info.dwUID = 0xffffffff;
	m_pFriendBtn = NULL;
	m_pWhisperBtn = NULL;
	m_pIgnoreBtn = NULL;
	m_pPetArea = NULL;
	m_pExhibition = NULL;
	m_pPetLoadGauge = NULL;
	m_pPartTidList = NULL;
	m_pComment = NULL;
	m_pNickArea = NULL;
	for (int i = 0; i < 2; ++i)
	{
		m_skinScrollType[i] = 0;
		m_skinScroll[i] = 0.0f;
	}
	m_pScoreList = NULL;
	m_pScoreFont = NULL;
	m_pMyGuild = NULL;
	m_reserved2a8 = 0;
	m_pGuildIcon = NULL;
	m_bRefresh = false;
	m_bMsnControl = false;
	m_bControlOff = false;
	m_guildPang = 0;
	m_guildPoint = 0;
	memset(m_position.csChannelName, 0, sizeof(m_position.csChannelName));
	m_position.iChannelUid = 0xff;
	m_position.iRoomIdx = 0xffff;
	m_position.iRoomType = -1;
	m_position.iServerGUID = -1;
	m_bChatMsgOpen = false;
	memset(m_pMatchHistoryBtn, 0, 0x58c);
	m_pSpecListBox = NULL;
	m_bMatchHistoryRequest = false;
	m_bUnknownLocation = false;
	m_bReserved244c = false;
	m_delta = 0.0f;
	m_pGhostBtn = NULL;
	m_pRmrVisitorBtn = NULL;
	m_recvSeason = 0;
	m_requestSeason = 0;
	memset(m_classicMapStatistics, 0, sizeof(m_classicMapStatistics));
	for (int i = 0; i < 20; ++i)
		m_classicMapStatistics[i].cBestScore = 0x7f;
	memset(m_oldClassicMapStatistics, 0, sizeof(m_oldClassicMapStatistics));
	for (int i = 0; i < 20; ++i)
		m_oldClassicMapStatistics[i].cBestScore = 0x7f;
	memset(m_totalClassicMapStatistics, 0, sizeof(m_totalClassicMapStatistics));
	for (int i = 0; i < 20; ++i)
		m_totalClassicMapStatistics[i].cBestScore = 0x7f;
	memset(m_pBestScoreMap, 0, sizeof(m_pBestScoreMap));
	memset(m_bestMaxPang983, 0, sizeof(m_bestMaxPang983)); // 983 008a7bc6..008a7bfc
	m_pAddFriendDlg = NULL;
	m_bReserved983_5d30 = false; // 983 008a7c74 (lido no RefreshAllUccClothes antes do Refresh gravar)
}

FrUserInfoForm::~FrUserInfoForm()
{
	Clear();
}

void FrUserInfoForm::Clear()
{
	if (CUserInfo::Instance())
		CUserInfo::Instance()->m_pDlg = NULL;
	g_resrcmng->UnlockAllByThread(m_anotherHand.GetThreadID());
	m_anotherHand.BreakWorkingThread();
	HidePet(true);
	if (g_resrcmng && m_pScoreFont)
	{
		g_resrcmng->Release(m_pScoreFont);
		m_pScoreFont = NULL;
	}
}

void FrUserInfoForm::OnTitleSeasonBtnInit(int param)
{
	m_pTitleBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_seasonType == SEASON_CURRENT)
		m_pTitleBtn[0]->SetStatus(FrButton::PRESSED);
	else
		m_pTitleBtn[0]->SetStatus(FrButton::NORMAL);
}

void FrUserInfoForm::OnTitleSeasonBtnUp()
{
	if (!IsRecvSeason(4))
		RequestSeasonData(5);
	OnTitleSeasonVisible(); // 983 008a9226: jmp 008a9020 (mesmo corpo)
}

void FrUserInfoForm::OnTitleTotalBtnInit(int param)
{
	m_pTitleBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_seasonType == SEASON_TOTAL)
		m_pTitleBtn[1]->SetStatus(FrButton::PRESSED);
	else
		m_pTitleBtn[1]->SetStatus(FrButton::NORMAL);
}

void FrUserInfoForm::OnTitleTotalBtnUp()
{
	if (!IsRecvSeason(2))
		RequestSeasonData(0);
	m_seasonType = SEASON_TOTAL;
	m_pTitleBtn[0]->SetStatus(FrButton::NORMAL);
	m_pTitleBtn[1]->SetStatus(FrButton::PRESSED);
	SetContent(true);
	SetTrophyStatic();
	SetSpecListBoxItem(SEASON_TOTAL);
}

// 983 008a3b40: sem testar nulo (chamado com a janela carregada); caixa da aba pelo tipo no sub-objeto 0x27c
// (008a3b10/008a0520: contagem <= 0)
void FrUserInfoForm::SetTrophyStatic()
{
	if (m_tabType == TAB_TROPHY)
	{
		FrListBox* boxes[4] = { m_pNormalTrophyBox, m_pSpecialTrophyBox, m_pGpTrophyBox983[0], m_pGpTrophyBox983[1] };
		m_pNoTrophy->SetVisible(boxes[m_trophyType]->m_itemList.empty());
	}
	else
		m_pNoTrophy->SetVisible(false);
	m_pGuildNoTrophy->SetVisible(m_tabType == TAB_GUILD && m_pGuildTrophyList->m_itemList.empty());
}

void FrUserInfoForm::OnGeneralInfoInit(int param)
{
	m_pTabBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnGeneralInfoDown()
{
	SetFormA(TAB_GENERAL);
}

void FrUserInfoForm::OnSpecialInfoInit(int param)
{
	m_pTabBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnSpecialInfoDown()
{
	SetFormA(TAB_SPECIAL);
}

void FrUserInfoForm::OnRecordInfoInit(int param)
{
	m_pTabBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnRecordInfoDown()
{
	SetFormA(TAB_RECORD);
}

void FrUserInfoForm::OnTrophyInfoInit(int param)
{
	m_pTabBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnTrophyInfoDown()
{
	SetFormA(TAB_TROPHY);
}

void FrUserInfoForm::OnGuildInfoInit(int param)
{
	m_pTabBtn[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnGuildInfoDown()
{
	SetFormA(TAB_GUILD);
}

void FrUserInfoForm::OnGeneralBackInit(int param)
{
	m_pBackArea[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUserInfoForm::OnRecCourseBackInit(int param)
{
	m_pBackArea[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUserInfoForm::OnTroNormalBackInit(int param)
{
	m_pBackArea[4] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUserInfoForm::OnTroSpecialBackInit(int param)
{
	m_pBackArea[5] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUserInfoForm::OnGuildBackInit(int param)
{
	m_pBackArea[6] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// PKT (frente C, lote 37): 983 00899b60 - o Init do "btn_achievement" nao guarda o botao
void FrUserInfoForm::OnAchievementBtnInit983(int)
{
}

// PKT (frente C, lote 37): 983 008a0540 - pede as conquistas do dono da janela (pacote 0x157 com o UID, +0x42b) e manda a
// mensagem 1 ao ator principal
void FrUserInfoForm::OnAchievementBtnUp983()
{
	WSendPacket packet((enumClientPacket)0x157);
	packet.Encode4(m_userInfo.info.dwUID);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

void FrUserInfoForm::OnEditInit_GI_ID(int param)
{
	m_pGeneralEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[0]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GI_SCHOOL(int param)
{
	m_pGeneralEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[1]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GI_CLUB(int param)
{
	m_pGeneralEdit[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[2]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GI_EXP(int param)
{
	m_pGeneralEdit[3] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[3]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GI_CONNSTATUS(int param)
{
	m_pGeneralEdit[4] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[4]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GI_DISCONN(int param)
{
	m_pGeneralEdit[5] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[5]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GI_COMBO(int param)
{
	m_pGeneralEdit[6] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[6]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GI_QUIZLEVEL(int param)
{
	m_pGeneralEdit[7] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGeneralEdit[7]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_TROPHY_GOLD(int param)
{
	m_pSpecialEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[0]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_TROPHY_SILVER(int param)
{
	m_pSpecialEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[1]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_TROPHY_BRONZE(int param)
{
	m_pSpecialEdit[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[2]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_MATCH_WIN_PERCENTAGE(int param)
{
	m_pSpecialEdit[6] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[6]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_AVG_SCORE(int param)
{
	m_pSpecialEdit[7] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[7]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_MEAN_SHOTTIME(int param)
{
	m_pSpecialEdit[8] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[8]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_PANGYA_PERCENTAGE(int param)
{
	m_pSpecialEdit[9] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[9]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_FAIRWAY_PERCENTAGE(int param)
{
	m_pSpecialEdit[10] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[10]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_PUTT_PERCENTAGE(int param)
{
	m_pSpecialEdit[11] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[11]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_OB_PERCENTAGE(int param)
{
	m_pSpecialEdit[12] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[12]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_TOTAL_SKINS_PANG(int param)
{
	m_pSpecialEdit[13] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[13]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_TOT_PLAYTIME(int param)
{
	m_pRecordEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[0]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_TOT_SHOTS(int param)
{
	m_pRecordEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[1]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_TOT_HOLES(int param)
{
	m_pRecordEdit[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[2]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_TOT_DRIVE(int param)
{
	m_pRecordEdit[4] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[4]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_EARTH_ROUND(int param)
{
	m_pRecordEdit[3] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[3]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_BEST_SCORE_5(int param)
{
	m_pRecordEdit[5] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[5]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_BEST_SCORE_4(int param)
{
	m_pRecordEdit[6] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[6]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_BEST_SCORE_3(int param)
{
	m_pRecordEdit[7] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[7]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_BEST_SCORE_2(int param)
{
	m_pRecordEdit[8] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[8]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_BEST_SCORE_1(int param)
{
	m_pRecordEdit[9] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[9]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_TOT_HOLEINONES(int param)
{
	m_pRecordEdit[10] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[10]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_TOT_ALBATROSS(int param)
{
	m_pRecordEdit[11] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[11]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_LONGEST_DRIVE(int param)
{
	m_pRecordEdit[12] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[12]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_LONGEST_PUTT(int param)
{
	m_pRecordEdit[13] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[13]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_RI_LONGEST_CHIPIN(int param)
{
	m_pRecordEdit[14] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pRecordEdit[14]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GUILD_NAME(int param)
{
	m_pGuildEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGuildEdit[0]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GUILD_PANG(int param)
{
	m_pGuildEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGuildEdit[1]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_GUILD_MYPERCENT(int param)
{
	m_pGuildEdit[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pGuildEdit[2]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnAreaInit_GI_Level(int param)
{
	m_pLevelArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pLevelArea)
		m_pLevelArea->SetVisible(false);
}

void FrUserInfoForm::OnAreaInit_GI_ClUB(int param)
{
	m_pClubArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pClubArea)
		m_pClubArea->SetVisible(false);
}

void FrUserInfoForm::OnAreaInit_Question_mark(int param)
{
	m_pQuestionMark = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pQuestionMark)
	{
		m_pQuestionMark->SetToolTipText(
			K2L_Compatibility("\260\255\301\246\301\276\267\341\300\262\300\272 \272\361\261\342\263\312E \300\314\273\363, \260\324\300\323\310\275\274\366 50\310\270 \300\314\273\363 \265\316 \301\266\260\307\300\273 \270\360\265\316 \270\270\301\267\307\317\277\251\276\337 \301\244\273\363 \307\245\261\342\265\313\264\317\264\331."));
	}
	const Bitmap* bitmap = g_pFresh->GetManager()->GetBitmap("USER_INFO",
		"user_info_question_mark");
	if (bitmap && m_pQuestionMark)
	{
		m_pQuestionMark->SetBgImg(bitmap);
		m_pQuestionMark->SetVisible(false);
	}
}

void FrUserInfoForm::OnAreaDraw_GI_ClUB(int param)
{
	if (m_userInfo.info.dwGuildId && !(m_userInfo.info.dwIdentity & 0x14))
	{
		const Bitmap* pEmblem =
			NetResManager()->GetEmblem983(m_userInfo.info.dwGuildId, m_userInfo.info.dwEmblemVer);
		if (pEmblem)
		{
			FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
			if (pDevice)
			{
				WRect rect;
				float right = m_pGeneralEdit[2]->GetRect().x +
					m_pGeneralEdit[2]->GetRect().w;
				unsigned width = pEmblem->Width();
				float textW = pDevice->GetTextExtendOld983(m_userInfo.info.sGuild);
				unsigned height = pEmblem->Height();
				rect.x = right - textW - (float)width - 15.0f;
				rect.y = m_pGeneralEdit[2]->GetRect().y + 2.0f;
				rect.w = (float)pEmblem->Width();
				rect.h = (float)height;
				pDevice->DrawTexture(pEmblem, rect, 0xffffffff, 0);
			}
		}
	}
}

// 983: es0_4..6 (0089b4d0/b550/b5d0) e era0..5 (0089ba50..bcd0): so guardam o campo e ligam o hover; sem eles os rotulos
// da aba Adv./Record ficavam soltos sobre a aba General
#define INIT_EDIT983(fn, slot) \
	void FrUserInfoForm::fn(int param) \
	{ \
		FrEdit* pEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param); \
		slot = pEdit; \
		if (pEdit) \
			pEdit->EnableHover(true, 0.0f); \
	}
INIT_EDIT983(OnEditInit_SI_AWARD_4, m_pSpecialEdit[3])
INIT_EDIT983(OnEditInit_SI_AWARD_5, m_pSpecialEdit[4])
INIT_EDIT983(OnEditInit_SI_AWARD_6, m_pSpecialEdit[5])
INIT_EDIT983(OnEditInit_RA0, m_pRecAwardWnd[0])
INIT_EDIT983(OnEditInit_RA1, m_pRecAwardWnd[1])
INIT_EDIT983(OnEditInit_RA2, m_pRecAwardWnd[2])
INIT_EDIT983(OnEditInit_RA3, m_pRecAwardWnd[3])
INIT_EDIT983(OnEditInit_RA4, m_pRecAwardWnd[4])
INIT_EDIT983(OnEditInit_RA5, m_pRecAwardWnd[5])
#undef INIT_EDIT983

void FrUserInfoForm::OnRecordAwardBtnInit(int param)
{
	m_pRecordBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRecordBtn[1]->SetStatus(m_recordType == 1 ? FrButton::PRESSED : FrButton::NORMAL);
}

// 983 008a0650: aba "Award" da Record (tipo 1)
void FrUserInfoForm::OnRecordAwardBtnUp()
{
	m_recordType = 1;
	for (int i = 0; i < 4; ++i)
		if (m_pRecordBtn[i])
			m_pRecordBtn[i]->SetStatus(i == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	ChangeRecordType();
}

// 983: edit_gp_*, btn_gp_*, listbox_gp_* (0089e5e0..0089e860) entram nos slots 2 e 3 do sub-objeto de trofeus
void FrUserInfoForm::OnGpNumInit0(int param) { m_pGpTrophyNum983[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param); }
void FrUserInfoForm::OnGpNumInit1(int param) { m_pGpTrophyNum983[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param); }
void FrUserInfoForm::OnGpBtnInit0(int param) { m_pTrophyBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param); }
void FrUserInfoForm::OnGpBtnInit1(int param) { m_pTrophyBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param); }
void FrUserInfoForm::OnGpBoxInit0(int param) { m_pGpTrophyBox983[0] = DYNAMIC_CAST(FrListBox, (FrWnd*)param); }
void FrUserInfoForm::OnGpBoxInit1(int param) { m_pGpTrophyBox983[1] = DYNAMIC_CAST(FrListBox, (FrWnd*)param); }

void FrUserInfoForm::OnRecordNormalBtnInit(int param)
{
	m_pRecordBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_recordType == 0)
		m_pRecordBtn[0]->SetStatus(FrButton::PRESSED);
	else
		m_pRecordBtn[0]->SetStatus(FrButton::NORMAL);
}

void FrUserInfoForm::OnRecordNormalBtnUp()
{
	m_recordType = 0;
	m_pRecordBtn[0]->SetStatus(FrButton::PRESSED);
	m_pRecordBtn[1]->SetStatus(FrButton::NORMAL);
	m_pRecordBtn[2]->SetStatus(FrButton::NORMAL);
	m_pRecordBtn[3]->SetStatus(FrButton::NORMAL);
	ChangeRecordType();
}

void FrUserInfoForm::OnRecordCourseBtnInit(int param)
{
	m_pRecordBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_recordType == 2)
		m_pRecordBtn[2]->SetStatus(FrButton::PRESSED);
	else
		m_pRecordBtn[2]->SetStatus(FrButton::NORMAL);
}

void FrUserInfoForm::OnRecordCourseBtnUp()
{
	m_recordType = 2;
	m_pRecordBtn[2]->SetStatus(FrButton::PRESSED);
	m_pRecordBtn[0]->SetStatus(FrButton::NORMAL);
	m_pRecordBtn[1]->SetStatus(FrButton::NORMAL);
	m_pRecordBtn[3]->SetStatus(FrButton::NORMAL);
	ChangeRecordType();
	UpdateEditMapStatistics(m_seasonType, false);
}

void FrUserInfoForm::OnRecordClassicBtnInit(int param)
{
	m_pRecordBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_recordType == 3)
		m_pRecordBtn[3]->SetStatus(FrButton::PRESSED);
	else
		m_pRecordBtn[3]->SetStatus(FrButton::NORMAL);
}

void FrUserInfoForm::OnRecordClassicBtnUp()
{
	m_recordType = 3;
	m_pRecordBtn[3]->SetStatus(FrButton::PRESSED);
	m_pRecordBtn[2]->SetStatus(FrButton::NORMAL);
	m_pRecordBtn[1]->SetStatus(FrButton::NORMAL);
	m_pRecordBtn[0]->SetStatus(FrButton::NORMAL);
	ChangeRecordType();
	UpdateEditMapStatistics(m_seasonType, true);
}

void FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY1(int param)
{
	m_pSpecialEdit[14] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[14]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY2(int param)
{
	m_pSpecialEdit[15] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[15]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY3(int param)
{
	m_pSpecialEdit[16] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[16]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY4(int param)
{
	m_pSpecialEdit[17] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[17]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnEditInit_SI_MATCH_HISTORY5(int param)
{
	m_pSpecialEdit[18] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pSpecialEdit[18]->EnableHover(true, 0.0f);
}

void FrUserInfoForm::OnSpecListboxInit(int param)
{
	m_pSpecListBox = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pSpecListBox->SetVisible(false);
	m_pSpecialBack = g_pFresh->RegisterBitmap("special_back");
	m_pSpecialBack00 = g_pFresh->RegisterBitmap("special_back00");
	m_pSpecListBox->AddItem((void*)m_pSpecialBack);
	m_pSpecListBox->AddItem((void*)m_pSpecialBack00);
}

void FrUserInfoForm::OnMatchHistoryBtn1Init(int param)
{
	m_pMatchHistoryBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnMatchHistoryBtn2Init(int param)
{
	m_pMatchHistoryBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnMatchHistoryBtn3Init(int param)
{
	m_pMatchHistoryBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnMatchHistoryBtn4Init(int param)
{
	m_pMatchHistoryBtn[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnMatchHistoryBtn5Init(int param)
{
	m_pMatchHistoryBtn[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnMatchHistoryBtn1Up()
{
	MatchHistoryUserBtnUp(0);
}

void FrUserInfoForm::OnMatchHistoryBtn2Up()
{
	MatchHistoryUserBtnUp(1);
}

void FrUserInfoForm::OnMatchHistoryBtn3Up()
{
	MatchHistoryUserBtnUp(2);
}

void FrUserInfoForm::OnMatchHistoryBtn4Up()
{
	MatchHistoryUserBtnUp(3);
}

void FrUserInfoForm::OnMatchHistoryBtn5Up()
{
	MatchHistoryUserBtnUp(4);
}

void FrUserInfoForm::MatchHistoryUserBtnUp(int index)
{
	WSendPacket packet((enumClientPacket)0x2f);
	packet.Encode4(m_matchHistory[index].dwUID);
	packet.Encode1(5);
	packet.Send(TO_GAME);
	m_bMatchHistoryRequest = true;
	m_pMatchHistoryBtn[index]->SetStatus(FrButton::NORMAL);
	CUserInfo::Instance()->m_nickname = m_matchHistory[index].szNickName;
}

void FrUserInfoForm::MsnDisconExeception(float delta, FrListItem* item)
{
	static int s_count;
	static float s_time;
	if (m_bMatchHistoryRequest)
	{
		s_time += delta;
		if (s_time < 0.7f)
		{
			FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
			if (!gdi)
				return;
			if (!item)
				return;
			gdi->SetTextColor(0xffc0125c, 0xffffffff);
			gdi->SetTextStyle(2);
			g_pFresh->GetManager()->PrintText(
				WPoint(item->pos.x + 37.0f, item->pos.y + 212.0f), 0,
				K2L_Compatibility("\274\255\271\366\300\307 \300\333\276\367\267\256\300\314 \270\271\275\300\264\317\264\331."),
				-1.0f, 0xffffffff);
			g_pFresh->GetManager()->PrintText(
				WPoint(item->pos.x + 37.0f, item->pos.y + RB_228F), 0,
				K2L_Compatibility("\300\341\275\303 \310\304 \264\331\275\303 \275\303\265\265\307\330 \301\326\275\312\275\303\277\300."),
				-1.0f, 0xffffffff);
		}
		if (s_time > 1.0f)
		{
			s_time = 0.0f;
			s_count++;
			if (s_count > 3)
			{
				s_count = 0;
				m_bMatchHistoryRequest = false;
			}
		}
	}
}

void FrUserInfoForm::SetMatchHistoryContents()
{
	char text[128] = "";
	if (CMatchHistory::Instance()->IsUpdated() == true)
	{
		for (int i = 0; i < 5; ++i)
		{
			if (m_matchHistory[i].dwUID)
				m_pSpecialEdit[14 + i]->SetLine(1, m_matchHistory[i].szNickName,
					0xff000000, false, 0);
			else
				m_pSpecialEdit[14 + i]->SetLine(1,
					K2L_Compatibility("\261\342\267\317 \276\370\300\275"), 0xff000000, false, 0);
		}
	}
	else
	{
		const char* src = K2L_Compatibility("\310\256\300\316 \272\322\260\241.");
		strcpy(text, src);
		for (int i = 0; i < 5; ++i)
		{
			m_pSpecialEdit[14 + i]->SetLine(1, text, 0xff000000, false, 0);
		}
	}
}

void FrUserInfoForm::SetSpecListBoxItem(eSeasonType type)
{
	if (strcmp(m_userInfo.info.sNick, Doc()->m_myInfo.info.sNick) == 0 &&
		m_pTabBtn[2]->GetStatus() == FrButton::PRESSED && m_pRecordBtn[3])
	{
		m_pRecordBtn[3]->SetVisible(true);
	}
}

void FrUserInfoForm::OnSpecListboxOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice || !param)
		return;

	FrListItem* pItem = (FrListItem*)param;
	float x = pItem->pos.x;
	float y = pItem->pos.y;
	if (pItem->pData == (void*)m_pSpecialBack)
	{
		const Bitmap* pBack = m_pSpecialBack;
		WRect rect(x, y, (float)pBack->Width(), (float)pBack->Height());
		pDevice->DrawTexture(pBack, rect, 0xffffffff, 0);
		for (int i = 0; i < 3; ++i)
		{
			WRect editRect(x + (float)(i * 60) + 82.0f, y + 17.0f, 40.0f,
				20.0f);
			m_pSpecialEdit[i]->SetRect(editRect);
			m_pSpecialEdit[i]->SetVisible(true);
		}
		// 983 0089d3c6: linha do Grand Prix, y+57
		for (int i = 0; i < 3; ++i)
		{
			WRect editRect(x + (float)(i * 60) + 82.0f, y + 57.0f, 40.0f,
				20.0f);
			m_pSpecialEdit[3 + i]->SetRect(editRect);
			m_pSpecialEdit[3 + i]->SetVisible(true);
		}
		WRect rect3(x + 130.0f, y + 38.0f, 100.0f, 20.0f);
		m_pSpecialEdit[6]->SetRect(rect3);
		m_pSpecialEdit[6]->SetVisible(true);
		for (int i = 0; i < 6; ++i)
		{
			WRect editRect(x + 130.0f, y + (float)(i * 20) + 98.0f, 100.0f,
				20.0f);
			m_pSpecialEdit[7 + i]->SetRect(editRect);
			m_pSpecialEdit[7 + i]->SetVisible(true);
		}
		WRect rect10(x + 130.0f, y + 232.0f, 100.0f, 20.0f);
		m_pSpecialEdit[13]->SetRect(rect10);
		m_pSpecialEdit[13]->SetVisible(true);
		for (int i = 0; i < 5; ++i)
			m_pSpecialEdit[14 + i]->SetVisible(false);
		for (int i = 0; i < 5; ++i)
			m_pMatchHistoryBtn[i]->SetVisible(false);
		return;
	}

	const Bitmap* pBack = m_pSpecialBack00;
	WRect rect(x, y, (float)pBack->Width(), (float)pBack->Height());
	pDevice->DrawTexture(pBack, rect, 0xffffffff, 0);
	for (int i = 0; i < 14; ++i)
		m_pSpecialEdit[i]->SetVisible(false);
	int offsetY = 0;
	for (int i = 0; i < 5; ++i)
	{
		if (m_matchHistory[i].dwUID == 0)
		{
			m_pMatchHistoryBtn[i]->SetVisible(false);
			WRect editRect(x + 42.0f, (float)offsetY + y + 46.0f, 124.0f,
				20.0f);
			m_pSpecialEdit[14 + i]->SetRect(editRect);
			m_pSpecialEdit[14 + i]->SetVisible(true);
		}
		else
		{
			const Bitmap* pIcon = (&m_pMaleIcon)[m_matchHistory[i].nGender];
			WRect iconRect(x + 14.0f, (float)offsetY + y + 45.0f,
				(float)pIcon->Width(), (float)pIcon->Height());
			pDevice->DrawTexture(pIcon, iconRect, 0xffffffff, 0);
			m_pMatchHistoryBtn[i]->SetVisible(true);
			pDevice->SetTextColor(0xff000000, 0xffffffff);
			pDevice->SetTextStyle(1);
			WPoint pos(x + 51.0f, (float)offsetY + y + 51.0f);
			g_pFresh->GetManager()->PrintText(pos, 0,
				m_matchHistory[i].szNickName, -1.0f, 0xffffffff);
			m_pSpecialEdit[14 + i]->SetVisible(false);
		}
		offsetY += 26;
	}
	MsnDisconExeception(m_delta, pItem);
}

static __forceinline bool GetServerName(char* buf, unsigned long serverGuid)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::list<sGameServerInfo>::iterator it;
	for (it = pDoc->m_gameServerList.begin();
		it != pDoc->m_gameServerList.end(); ++it)
	{
		if ((*it).id == serverGuid)
		{
			wsprintfA(buf, "%s", (*it).name);
			return true;
		}
	}
	return false;
}

// NOTE: UpdateEditUserLocation still differs in one register (original keeps the room index in si,
// ours in ax). The separate helper is what keeps the stack cleanup of the first wsprintfA apart.
__declspec(dllimport) __forceinline bool FormatServerName(char* buf,
	std::list<sGameServerInfo>& servers, sFriend* pFriend)
{
	for (std::list<sGameServerInfo>::iterator it = servers.begin();
		it != servers.end(); ++it)
	{
		if ((*it).id == pFriend->userPosition.iServerGUID)
		{
			wsprintfA(buf, "%s", (*it).name);
			return true;
		}
	}
	return false;
}

void FrUserInfoForm::UpdateEditUserLocation()
{
	if (m_userInfo.roomIndex != 0xffff)
	{
		std::list<sRoomInfo>& rooms = CSharedDoc::Instance()->m_roomList;
		std::list<sRoomInfo>::iterator it =
			std::find(rooms.begin(), rooms.end(), m_userInfo.roomIndex);
		if (it == rooms.end())
		{
			m_pGeneralEdit[4]->SetLine(1, K2L_Compatibility("\276\313 \274\366 \276\370\300\275"),
				0xff000000, false, 0);
		}
		else
		{
			int mode = 0;
			if (!(*it).bPublic &&
				!((unsigned char)(CSharedDoc::Instance()
									  ->m_myInfo.info.dwIdentity >>
					  2) &
					1))
				mode = 2;
			else if (!(*it).bAvailable ||
				(IsLocalContent((localContentType_t)0x2f /*983: era 0x2e; interpolado*/) && (*it).bIntrusion))
				mode = 1;
			switch (mode)
			{
			case 0:
				m_pGeneralEdit[4]->SetLine(1,
					MakeStr(K2L_Compatibility("[%d]\271\370 \271\346 %s"), (*it).roomGuid,
						(*it).bSleep ? K2L_Compatibility("\300\341\274\366\301\337")
									 : K2L_Compatibility("\264\353\261\342\301\337")),
					0xff000000, false, 0);
				break;
			case 1:
				m_pGeneralEdit[4]->SetLine(1,
					MakeStr(K2L_Compatibility("[%d]\271\370 \271\346 %s"), (*it).roomGuid,
						((unsigned char)(m_userInfo.info.dwIdentity >> 1) & 1)
							? K2L_Compatibility("\260\374\300\374\301\337")
							: K2L_Compatibility("\260\324\300\323\301\337")),
					0xff000000, false, 0);
				break;
			case 2:
				m_pGeneralEdit[4]->SetLine(1,
					K2L_Compatibility("\272\361\260\370\260\263\271\346\277\241 \301\242\274\323\301\337"),
					0xff000000, false, 0);
				break;
			}
		}
	}
	else if (m_bMsnControl)
	{
		sFriend* pFriend = BuddyMGR->GetBuddy(m_userInfo.info.dwUID);
		if (pFriend)
		{
			if (pFriend->IsLogOn)
			{
				CSharedDoc* pDoc = CSharedDoc::Instance();
				char buf[128] = "\0";
				if (!FormatServerName(buf, pDoc->m_gameServerList, pFriend))
				{
					m_pGeneralEdit[4]->SetLine(1, K2L_Compatibility("\301\242\274\323\301\337"),
						0xff000000, false, 0);
					return;
				}
				wsprintfA(buf, "%s %s", buf,
					pFriend->userPosition.csChannelName);
				if (pFriend->userPosition.iRoomIdx != 0xffff)
					wsprintfA(buf, "%s [%d]", buf,
						pFriend->userPosition.iRoomIdx);
				m_pGeneralEdit[4]->SetLine(1, buf, 0xff000000, false, 0);
			}
			else
			{
				m_pGeneralEdit[4]->SetLine(1,
					K2L_Compatibility("\301\242\274\323\307\317\301\366 \276\312\300\275"),
					0xff000000, false, 0);
			}
		}
		else
		{
			if (m_bUnknownLocation)
				m_pGeneralEdit[4]->SetLine(1,
					K2L_Compatibility("\276\313 \274\366 \276\370\300\275"), 0xff000000, false, 0);
			else
				m_pGeneralEdit[4]->SetLine(1,
					K2L_Compatibility("\304\243\261\270\276\306\264\324"), 0xff000000, false, 0);
		}
	}
	else
	{
		m_pGeneralEdit[4]->SetLine(1,
			K2L_Compatibility("\264\353\261\342\275\307\277\241 \301\242\274\323\301\337"),
			0xff000000, false, 0);
	}
}

void FrUserInfoForm::OnRecTotalListboxInit(int param)
{
	m_pRecTotalListBox = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pRecTotalListBox->SetVisible(false);
	m_pRecTotalBack[0] = g_pFresh->RegisterBitmap("special_back01");
	m_pRecTotalBack[1] = g_pFresh->RegisterBitmap("special_back02");
	m_pRecTotalBack[2] = g_pFresh->RegisterBitmap("special_back03");
	m_pRecTotalBackNd = g_pFresh->RegisterBitmap("special_back02_nd");
	m_pRecTotalListBox->AddItem((void*)m_pRecTotalBack[0]);
	m_pRecTotalListBox->AddItem((void*)m_pRecTotalBack[1]);
	m_pRecTotalListBox->AddItem((void*)m_pRecTotalBack[2]);
}

void FrUserInfoForm::OnRecTotalListboxOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	WRect rect;
	WRect editRect;
	rect.x = pItem->pos.x;
	rect.y = pItem->pos.y;
	if (pItem->pData == (void*)m_pRecTotalBack[0])
	{
		rect.w = (float)m_pRecTotalBack[0]->Width();
		rect.h = (float)m_pRecTotalBack[0]->Height();
		pDevice->DrawTexture(m_pRecTotalBack[0], rect, 0xffffffff, 0);
		editRect = WRect(rect.x + 68.0f, rect.y + 22.0f, 160.0f, 20.0f);
		m_pRecordEdit[0]->SetRect(editRect);
		m_pRecordEdit[0]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 42.0f, 160.0f, 20.0f);
		m_pRecordEdit[1]->SetRect(editRect);
		m_pRecordEdit[1]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 62.0f, 160.0f, 20.0f);
		m_pRecordEdit[2]->SetRect(editRect);
		m_pRecordEdit[2]->SetVisible(true);
		editRect = WRect(rect.x + 121.0f, rect.y + 81.0f, 44.0f, 20.0f);
		m_pRecordEdit[3]->SetRect(editRect);
		m_pRecordEdit[3]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 81.0f, 160.0f, 20.0f);
		m_pRecordEdit[4]->SetRect(editRect);
		m_pRecordEdit[4]->SetVisible(true);
		m_pRecordEdit[12]->SetVisible(false);
		m_pRecordEdit[13]->SetVisible(false);
		m_pRecordEdit[14]->SetVisible(false);
		return;
	}
	if (pItem->pData == (void*)m_pRecTotalBack[1])
	{
		rect.w = (float)m_pRecTotalBack[1]->Width();
		rect.h = (float)m_pRecTotalBack[1]->Height();
		pDevice->DrawTexture(m_pRecTotalBack[1], rect, 0xffffffff, 0);
		editRect = WRect(rect.x + 68.0f, rect.y + 14.0f, 160.0f, 20.0f);
		m_pRecordEdit[5]->SetRect(editRect);
		m_pRecordEdit[5]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 32.0f, 160.0f, 20.0f);
		m_pRecordEdit[6]->SetRect(editRect);
		m_pRecordEdit[6]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 50.0f, 160.0f, 20.0f);
		m_pRecordEdit[7]->SetRect(editRect);
		m_pRecordEdit[7]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 68.0f, 160.0f, 20.0f);
		m_pRecordEdit[8]->SetRect(editRect);
		m_pRecordEdit[8]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 86.0f, 160.0f, 20.0f);
		m_pRecordEdit[9]->SetRect(editRect);
		m_pRecordEdit[9]->SetVisible(true);
		if (rect.y - GetRect().y < 250.0f)
		{
			editRect = WRect(rect.x + 38.0f, rect.y + 126.0f, 69.0f, 20.0f);
			m_pRecordEdit[10]->SetRect(editRect);
			m_pRecordEdit[10]->SetVisible(true);
			editRect = WRect(rect.x + 145.0f, rect.y + 126.0f, 69.0f, 20.0f);
			m_pRecordEdit[11]->SetRect(editRect);
			m_pRecordEdit[11]->SetVisible(true);
			return;
		}
		m_pRecordEdit[10]->SetVisible(false);
		m_pRecordEdit[11]->SetVisible(false);
		return;
	}
	if (pItem->pData == (void*)m_pRecTotalBack[2])
	{
		rect.w = (float)m_pRecTotalBack[2]->Width();
		rect.h = (float)m_pRecTotalBack[2]->Height();
		pDevice->DrawTexture(m_pRecTotalBack[2], rect, 0xffffffff, 0);
		editRect = WRect(rect.x + 68.0f, rect.y + 52.0f, 160.0f, 20.0f);
		m_pRecordEdit[12]->SetRect(editRect);
		m_pRecordEdit[12]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 74.0f, 160.0f, 20.0f);
		m_pRecordEdit[13]->SetRect(editRect);
		m_pRecordEdit[13]->SetVisible(true);
		editRect = WRect(rect.x + 68.0f, rect.y + 96.0f, 160.0f, 20.0f);
		m_pRecordEdit[14]->SetRect(editRect);
		m_pRecordEdit[14]->SetVisible(true);
		m_pRecordEdit[0]->SetVisible(false);
		m_pRecordEdit[1]->SetVisible(false);
		m_pRecordEdit[2]->SetVisible(false);
		m_pRecordEdit[3]->SetVisible(false);
		m_pRecordEdit[4]->SetVisible(false);
	}
}

// 983 0089e0f0 / 0089e170 (tri_normalnum / tri_specialnum): so guardam o ponteiro no sub-objeto 0x27c (008999a0),
// sem o SetVisible(false) do 645; a visibilidade fica com ChangeTrophyType / SetVisible(aba)
void FrUserInfoForm::OnTrophyNormalNumInit(int param)
{
	m_pNormalTrophyNum = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrUserInfoForm::OnTrophySpecialNumInit(int param)
{
	m_pSpecialTrophyNum = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

// 983 0089e1f0 / 0089e270 (btn_normal / btn_special): so guardam o ponteiro no sub-objeto 0x27c (00899970),
// sem o SetStatus do 645; o estado do botao fica com ChangeTrophyType
void FrUserInfoForm::OnTrophyNormalBtnInit(int param)
{
	m_pTrophyBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnTrophyNormalBtnUp()
{
	m_trophyType = 0;
	m_pTrophyBtn[0]->SetStatus(FrButton::PRESSED);
	m_pTrophyBtn[1]->SetStatus(FrButton::NORMAL);
	ChangeTrophyType();
}

void FrUserInfoForm::OnTrophySpecialBtnInit(int param)
{
	m_pTrophyBtn[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnTrophySpecialBtnUp()
{
	m_trophyType = 1;
	m_pTrophyBtn[1]->SetStatus(FrButton::PRESSED);
	m_pTrophyBtn[0]->SetStatus(FrButton::NORMAL);
	ChangeTrophyType();
}

// 983 0089e2f0: cada tipo (0 total, 1 award, 2/3 course/classic) mostra so os seus widgets
void FrUserInfoForm::ChangeRecordType()
{
	int type = m_recordType;
	if (type == 0)
	{
		if (m_pBackArea[3])
			m_pBackArea[3]->SetVisible(false);
		if (m_pBackArea[2])
			m_pBackArea[2]->SetVisible(false);
		SetVisible((FrWnd**)m_pRecAwardWnd, 6, false);
		m_pRecTotalListBox->SetVisible(true);
		m_pScoreList->SetVisible(false);
		return;
	}
	if (type == 2 || type == 3)
	{
		if (m_pBackArea[3])
			m_pBackArea[3]->SetVisible(true);
		SetVisible((FrWnd**)m_pRecordEdit, 15, false);
		if (m_pBackArea[2])
			m_pBackArea[2]->SetVisible(false);
		SetVisible((FrWnd**)m_pRecAwardWnd, 6, false);
		m_pRecTotalListBox->SetVisible(false);
		m_pScoreList->SetVisible(true);
	}
	else if (type == 1)
	{
		if (m_pBackArea[3])
			m_pBackArea[3]->SetVisible(false);
		if (m_pBackArea[2])
			m_pBackArea[2]->SetVisible(true);
		SetVisible((FrWnd**)m_pRecAwardWnd, 6, true);
		SetVisible((FrWnd**)m_pRecordEdit, 15, false);
		m_pRecTotalListBox->SetVisible(false);
		m_pScoreList->SetVisible(false);
	}
}

// 983 008a3c70 (tipos 0 normal, 1 special, 2 GP normal, 3 GP special): botao pressionado, so o numero/lista do tipo,
// area de fundo [5] so no tipo 1 e [4] nos demais (008a3ca0/008a3cc0 gravam +0x278 = 2/3), "sem trofeu" se a lista do tipo esta vazia
void FrUserInfoForm::ChangeTrophyType()
{
	int type = m_trophyType;
	if (type < 0 || type > 3)
		return;
	FrEdit* nums[4] = { m_pNormalTrophyNum, m_pSpecialTrophyNum, m_pGpTrophyNum983[0], m_pGpTrophyNum983[1] };
	FrListBox* boxes[4] = { m_pNormalTrophyBox, m_pSpecialTrophyBox, m_pGpTrophyBox983[0], m_pGpTrophyBox983[1] };
	for (int i = 0; i < 4; ++i)
	{
		if (m_pTrophyBtn[i])
			m_pTrophyBtn[i]->SetStatus(i == type ? FrButton::PRESSED : FrButton::NORMAL);
		if (nums[i])
			nums[i]->SetVisible(i == type);
		if (boxes[i])
			boxes[i]->SetVisible(i == type);
	}
	if (m_pBackArea[4])
		m_pBackArea[4]->SetVisible(type != 1);
	if (m_pBackArea[5])
		m_pBackArea[5]->SetVisible(type == 1);
	if (m_pNoTrophy && boxes[type])
		m_pNoTrophy->SetVisible(boxes[type]->m_itemList.size() == 0);
}

void FrUserInfoForm::OnGpBtnUp0()
{
	m_trophyType = 2;
	ChangeTrophyType();
}

void FrUserInfoForm::OnGpBtnUp1()
{
	m_trophyType = 3;
	ChangeTrophyType();
}

void FrUserInfoForm::OnListBoxInit_TRI_SPECIALBOX(int param)
{
	m_pSpecialTrophyBox = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

// 983 008a0710: contas em double como o 983 (5/15/30, 3, 60+125, 0.5)
void FrUserInfoForm::OnListBoxOwnerDraw_TRI_SPECIALBOX(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	sSpecialTrophy* pTrophy = (sSpecialTrophy*)pItem->pData;
	float itemHeight = (float)m_pNormalTrophyBox->GetItemHeight();
	IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(pTrophy->tid);
	if (!pMatch)
		return;

	char buf[100];
	strcpy(buf, pMatch->Name);
	int line = 0;
	char* pText = buf;
	char* pNewLine = strstr(buf, "\n");
	while (pNewLine)
	{
		*pNewLine = '\0';
		pDevice->SetTextStyle(1);
		pDevice->Print(pItem->pos + WPoint(5.0, line * 15.0 + 30.0), 0, "%s", pText);
		++line;
		pText = pNewLine + 1;
		pNewLine = strstr(pText, "\n");
	}
	pDevice->SetTextStyle(0);
	pDevice->Print(pItem->pos + WPoint(5.0, line * 15.0 + 30.0), 0, "%s", pText);

	WRect rect;
	rect.y = pItem->pos.y + 3.0;
	pDevice->SetTextStyle(0);
	const Bitmap* pIcon =
		g_pFresh->GetManager()->GetBitmap("TROPHY", pMatch->Icon[0]);
	rect.x = pItem->pos.x + 60.0 + 125.0;
	rect.w = (float)pIcon->Width();
	rect.h = (float)pIcon->Height();
	pDevice->DrawTexture(pIcon, rect, pTrophy->count ? 0xffffffff : 0x60000000,
		0);
	unsigned long count = pTrophy->count;
	if (count > 1)
	{
		float hw = rect.w * 0.5;
		int half = (int)hw;
		pDevice->Print(WPoint(half + rect.x, (itemHeight + pItem->pos.y) - 15.0), 1, "%d",
			count);
	}
}

// 983 008a6290: item da caixa GP normal = um grupo de m_gpTrophy983 (FillGpTrophyBox983); nome do grupo
// (texto 0x7542 com o byte 2 do tid, 008a6040) e os 3 trofeus do grupo (byte 0 do tid = 1..3) a 0.8 do tamanho;
// trofeu que falta = icone do 1o do grupo apagado e numero 0
void FrUserInfoForm::OnGpNormalBoxOwnerDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	sGpTrophy983* pGroup = (sGpTrophy983*)pItem->pData;
	std::string name;
	GetGpTrophyGroupName983(2, (*pGroup)[0].tid, name);
	WPoint textPos;
	GetTrophyTextPos983(2, pItem->pos, textPos);
	PrintTrophyGroupName983(pDevice, textPos, name.c_str());
	WPoint start(textPos.x + pDevice->GetTextExtendOld983(name.c_str()), pItem->pos.y);

	for (unsigned int k = 0; k < 3; ++k)
	{
		IFF_STRUCT::sMatch* pMatch;
		unsigned long count = 0;
		unsigned long color;
		unsigned int i;
		for (i = 0; i < pGroup->size(); ++i)
			if (((*pGroup)[i].tid & 0xff) == k + 1)
				break;
		if (i < pGroup->size())
		{
			pMatch = ItemManager()->FindMatch((*pGroup)[i].tid);
			count = (*pGroup)[i].count;
			color = 0xffffffff;
		}
		else
		{
			pMatch = ItemManager()->FindMatch((*pGroup)[0].tid);
			color = 0x60000000;
		}
		const Bitmap* pIcon = g_pFresh->GetManager()->GetBitmap("TROPHY", pMatch->Icon[0]);
		if (!pIcon)
			break;
		float half = 0.0f;
		if (m_pGpTrophyBox983[0])
			half = m_pGpTrophyBox983[0]->GetItemHeight() * 0.5f;
		WRect rect;
		rect.x = (float)(pIcon->Width() * k) + start.x;
		rect.y = (half - pIcon->Height() * 0.5f) + start.y;
		rect.w = pIcon->Width() * 0.8f;
		rect.h = pIcon->Height() * 0.8f;
		pDevice->DrawTexture(pIcon, rect, color, 0);
		pDevice->Print(WPoint(rect.w * 0.5f + rect.x, rect.h + rect.y), 1, "%d", count);
	}
}

// 983 008a0bc0: item da caixa GP special = sSpecialTrophy* (FillTrophyListBox); nome em linhas e o icone
// a 0.8 do tamanho, centrado na altura do item da caixa GP normal (o 983 testa a special e mede a normal)
void FrUserInfoForm::OnGpSpecialBoxOwnerDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	sSpecialTrophy* pTrophy = (sSpecialTrophy*)pItem->pData;
	float itemHeight = (float)m_pNormalTrophyBox->GetItemHeight();
	IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(pTrophy->tid);
	if (!pMatch)
		return;

	char buf[100];
	strcpy(buf, pMatch->Name);
	int line = 0;
	char* pText = buf;
	char* pNewLine = strstr(buf, "\n");
	while (pNewLine)
	{
		*pNewLine = '\0';
		pDevice->SetTextStyle(1);
		float y = line * 15.0f + 30.0f;
		pDevice->Print(WPoint(pItem->pos.x + 5.0f, pItem->pos.y + y), 0, "%s", pText);
		++line;
		pText = pNewLine + 1;
		pNewLine = strstr(pText, "\n");
	}
	pDevice->SetTextStyle(0);
	float y = line * 15.0f + 30.0f;
	pDevice->Print(WPoint(pItem->pos.x + 5.0f, pItem->pos.y + y), 0, "%s", pText);

	const Bitmap* pIcon = g_pFresh->GetManager()->GetBitmap("TROPHY", pMatch->Icon[0]);
	float half = 0.0f;
	if (m_pGpTrophyBox983[1])
		half = m_pGpTrophyBox983[0]->GetItemHeight() * 0.5f;
	WRect rect;
	rect.y = half - pIcon->Height() * 0.5f + pItem->pos.y;
	pDevice->SetTextStyle(0);
	rect.x = pItem->pos.x + 60.0f + 125.0f;
	rect.w = pIcon->Width() * 0.8f;
	rect.h = pIcon->Height() * 0.8f;
	pDevice->DrawTexture(pIcon, rect, pTrophy->count ? 0xffffffff : 0x60000000, 0);
	unsigned long count = pTrophy->count;
	if (count > 1)
	{
		int w = float2int(rect.w * 0.5f);
		pDevice->Print(WPoint((float)w + rect.x, (pItem->pos.y + itemHeight) - 15.0f), 1, "%d", count);
	}
}

void FrUserInfoForm::OnListBoxInit_TRI_NORMALBOX(int param)
{
	m_pNormalTrophyBox = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrUserInfoForm::OnListBoxOwnerDraw_TRI_NORMALBOX(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	int idx = (int)pItem->pData;
	if (idx < 0 || idx >= 13)
		return;

	unsigned short* pCount;
	switch (m_seasonType)
	{
	case SEASON_CURRENT:
		pCount = m_userInfo.trophy.Trophy[idx];
		break;
	case SEASON_TOTAL:
		pCount = m_totalSeason.trophy.Trophy[idx];
		break;
	default:
		pCount = m_userInfo.trophy.Trophy[idx];
		break;
	}

	float itemHeight = (float)m_pNormalTrophyBox->GetItemHeight();
	int type = idx | 0x2c00;
	ItemManager()->FindMatch(type << 16);
	IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(type << 16);
	if (!pMatch)
		return;

	pDevice->SetTextStyle(1);
	pDevice->Print(WPoint(pItem->pos.x + 20.0f, pItem->pos.y + 38.0f), 0, "%s",
		pMatch->Name);

	WRect rect;
	rect.y = pItem->pos.y + 3.0f;
	pDevice->SetTextStyle(0);
	const Bitmap* pIcon[3];
	for (int i = 0; i < 3; ++i)
	{
		pIcon[i] = g_pFresh->GetManager()->GetBitmap("TROPHY", pMatch->Icon[i]);
		rect.x = i * 60.0f + pItem->pos.x + 75.0f;
		if (pIcon[i])
		{
			rect.w = (float)pIcon[i]->Width();
			rect.h = (float)pIcon[i]->Height();
			pDevice->DrawTexture(pIcon[i], rect,
				pCount[i] ? 0xffffffff : 0x60000000, 0);
			pDevice->Print(WPoint((float)(int)(rect.w * 0.5f) + rect.x,
							   (itemHeight + pItem->pos.y) - 10.0f),
				1, "%d", pCount[i]);
		}
	}
}

void FrUserInfoForm::OnStaticInit_TRI_NOTROPHY(int param)
{
	m_pNoTrophy = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pNoTrophy)
		m_pNoTrophy->SetVisible(false);
}

void FrUserInfoForm::OnMyGuildInit(int param)
{
	m_pMyGuild = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pGuildIcon = g_pFresh->GetManager()->GetBitmap("TROPHY", "guild");
}

void FrUserInfoForm::OnMyGuildOwnerDraw()
{
	if (m_userInfo.info.dwGuildId && !(m_userInfo.info.dwIdentity & 0x14))
	{
		FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
		if (pDevice)
		{
			WRect rect;
			float x = m_pMyGuild->GetRect().x;
			float y = m_pMyGuild->GetRect().y;
			if (m_pGuildIcon)
			{
				rect.w = (float)m_pGuildIcon->Width();
				rect.h = (float)m_pGuildIcon->Height();
				rect.x = x;
				rect.y = y;
				pDevice->DrawTexture(m_pGuildIcon, rect, 0xffffffff, 0);
			}
			const Bitmap* pEmblem =
				NetResManager()->GetEmblem983(m_userInfo.info.dwGuildId, m_userInfo.info.dwEmblemVer);
			if (pEmblem)
			{
				rect.x = x + 21.0f;
				rect.y = y + 12.0f;
				rect.w = (float)pEmblem->Width();
				rect.h = (float)pEmblem->Height();
				pDevice->DrawTexture(pEmblem, rect, 0xffffffff, 0);
			}
		}
	}
}

void FrUserInfoForm::OnGuildTrophyListInit(int param)
{
	m_pGuildTrophyList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pGuildTrophyList)
		m_pGuildTrophyList->SetVisible(false);
}

// 983 008a0ec0: nome do trofeu da guilda impresso com "%s" (o 645 passava o nome como formato)
void FrUserInfoForm::OnGuildTrophyListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice || !pItem->pData)
		return;

	IFF_STRUCT::sMatch* pMatch =
		ItemManager()->FindMatch(*(unsigned long*)pItem->pData);
	if (!pMatch)
		return;

	pDevice->SetTextColor(0xff000000, 0xffffffff);
	char buf[100];
	strcpy(buf, pMatch->Name);
	int line = 0;
	char* pText = buf;
	for (char* pNewLine = strstr(buf, "\n"); pNewLine;
		pNewLine = strstr(pText, "\n"))
	{
		*pNewLine = '\0';
		pDevice->SetTextStyle(1);
		float dy = line * 15.0 + 30.0;
		pDevice->Print(WPoint(pItem->pos.x + 18.0, dy + pItem->pos.y), 0,
			"%s", pText);
		++line;
		pText = pNewLine + 1;
	}
	pDevice->SetTextStyle(0);
	float dy = line * 15.0 + 30.0;
	pDevice->Print(WPoint(pItem->pos.x + 18.0, pItem->pos.y + dy), 0, "%s",
		pText);
	float y = pItem->pos.y + 3.0;
	pDevice->SetTextStyle(0);
	const Bitmap* pIcon =
		g_pFresh->GetManager()->GetBitmap("TROPHY", pMatch->Icon[0]);
	WRect rect(pItem->pos.x + 60.0 + 70.0, y, (float)pIcon->Width(),
		(float)pIcon->Height());
	pDevice->DrawTexture(pIcon, rect, 0xffffffff, 0);
}

void FrUserInfoForm::OnStaticInit_GUILD_NOTROPHY(int param)
{
	m_pGuildNoTrophy = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pGuildNoTrophy)
		m_pGuildNoTrophy->SetVisible(false);
}

void FrUserInfoForm::OnWhisperInit(int param)
{
	m_pWhisperBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnNickInit(int param)
{
	m_pNickArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pMaleIcon = g_pFresh->RegisterBitmap("i_male");
	m_pFemaleIcon = g_pFresh->RegisterBitmap("i_female");
}

void FrUserInfoForm::OnNickOwnerDraw(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (pDevice && m_userInfo.info.dwGuid != 0xffffffff)
	{
		const Bitmap* pIcon = (&m_pMaleIcon)[m_userInfo.info.gender];
		WRect rect(m_pNickArea->GetRect().x, m_pNickArea->GetRect().y,
			(float)pIcon->Width(), (float)pIcon->Height());
		pDevice->DrawTexture(pIcon, rect, 0xffffffff, 0);
		if (!CProjectG::Instance()->HidePrivacy())
		{
			pDevice->SetTextColor(0xff000000, 0xffffffff);
			pDevice->SetTextStyle(1);
			WPoint pos(m_pNickArea->GetRect().x + 27.0f,
				m_pNickArea->GetRect().y + 8.0f);
			g_pFresh->GetManager()->PrintText(pos, 0, m_userInfo.info.sNick,
				-1.0f, 0xffffffff);
		}
	}
}

void FrUserInfoForm::OnWhisperUp()
{
	if (m_userInfo.info.dwUID != 0xffffffff &&
		m_userInfo.info.dwUID != CSharedDoc::Instance()->m_myInfo.info.dwUID)
	{
		if (IS_KINDOF(CGolfTask, AfxGetTask()))
		{
			AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, M983(0x37),
				(int)m_userInfo.info.sNick, 0, 0, 0, 0);
		}
		else if (IS_KINDOF(CLobbyTask, AfxGetTask()))
		{
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x37),
				(int)m_userInfo.info.sNick, 0, 0, 0, 0);
		}
		else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
		{
			AfxGetTask()->GetActor("AvatarChat") << MsgObject(NULL, M983(0x37),
				(int)m_userInfo.info.sNick, 0, 0, 0, 0);
		}
	}
}

void FrUserInfoForm::OnFriendInit(int param)
{
	m_pFriendBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnFriendUp()
{
	if (IS_KINDOF(CGolfTask, AfxGetTask()) ||
		IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		m_bChatMsgOpen = CChatMsg::Instance()->IsActive();
	}
	if (!m_pAddFriendDlg)
	{
		m_pAddFriendDlg = CreateForm<FrAddFriendDlg>(g_pFresh->GetManager(),
			this, "addfriend", NULL);
		m_pAddFriendDlg->CheckRequestNickname(m_userInfo.info.sNick);
		m_pAddFriendDlg->Open(
			(FRESH_PFN_RESULT)&FrUserInfoForm::OnAddFriendFormResult, 1);
	}
}

void FrUserInfoForm::OnIgnoreInit(int param)
{
	m_pIgnoreBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnIgnoreUp()
{
	if (m_userInfo.info.dwUID != 0xffffffff &&
		m_userInfo.info.dwUID != CSharedDoc::Instance()->m_myInfo.info.dwUID)
	{
		bool bIgnore = false;
		std::list<std::string>::iterator it = std::find(
			CSharedDoc::Instance()->m_ignoreList.begin(),
			CSharedDoc::Instance()->m_ignoreList.end(), m_userInfo.info.sNick);
		if (it == CSharedDoc::Instance()->m_ignoreList.end())
		{
			CSharedDoc::Instance()->m_ignoreList.push_back(
				m_userInfo.info.sNick);
			bIgnore = true;
		}
		else
		{
			CSharedDoc::Instance()->m_ignoreList.erase(it);
		}
		SetIgnoreButtonImg(bIgnore);
	}
}

void FrUserInfoForm::SetIgnoreButtonImg(bool bIgnore)
{
	if (bIgnore)
	{
		m_pIgnoreBtn->SetButtonImg("btn_i02_02_n", FrButton::NORMAL);
		m_pIgnoreBtn->SetButtonImg("btn_i02_02_o", FrButton::OVER);
		m_pIgnoreBtn->SetButtonImg("btn_i02_02_d", FrButton::PRESSED);
	}
	else
	{
		m_pIgnoreBtn->SetButtonImg("btn_i02_n", FrButton::NORMAL);
		m_pIgnoreBtn->SetButtonImg("btn_i02_o", FrButton::OVER);
		m_pIgnoreBtn->SetButtonImg("btn_i02_d", FrButton::PRESSED);
	}
}

void FrUserInfoForm::OnCloseUp()
{
	Close(FrNONE, true);
}

bool FrUserInfoForm::Close(eFormRet result, bool bFade)
{
	if (m_anotherHand.IsWorking(0))
		return false;
	return FrForm::Close(result, bFade);
}

void FrUserInfoForm::OnProc(const float delta)
{
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
		RefreshAllUccClothes();
	m_delta = delta;
	if (m_petLoadTime < 3.0f)
	{
		float sum = delta + m_petLoadTime;
		m_petLoadTime = sum;
		if (!(sum >= 3.0f) && m_anotherHand.IsWorking(0))
			return;
		m_pPetLoadGauge->SetVisible(false);
		m_petLoadTime = 3.0f;
	}
}

void FrUserInfoForm::OnAnotherHand(int key, void* param)
{
	if (key == 0)
		ShowPet();
}

void FrUserInfoForm::RefreshAllUccClothes()
{
	if (0)
		CSharedDoc::
			Instance(); // HACK: emission order (CExhibition::GetPetFrame is a late PCH inline in the original)
	// 983 (0089ef40): tambem exige o byte +0x5d30 (o 983 so grava 0 nele: 008a7c74/008a9aab)
	if (m_pPartTidList && m_pExhibition && m_pExhibition->GetPetFrame() &&
		m_bReserved983_5d30)
	{
		sUccRefreshItem item;
		for (int i = 0; i < 10; i++)
		{
			if (!UccManager()->GetRefreshItem(item))
				return;
			unsigned char index = (unsigned char)((item.typeId >> 13) & 0x1f);
			if (m_pPartTidList->m_tid[index] == item.typeId)
			{
				unsigned long uid = m_pPartTidList->m_uid[index];
				sUccClothes* pClothes = UccManager()->FindClothes(uid);
				if (pClothes && !strcmpi(pClothes->uccIndex, item.uccIndex) &&
					m_pExhibition && m_pExhibition->GetPetFrame())
				{
					m_pExhibition->GetPetFrame()->SetPart983(pClothes);
				}
			}
		}
	}
}

bool FrUserInfoForm::OnInit()
{
	m_disconnRect = m_pGeneralEdit[5]->GetRect();
	m_tabType = TAB_NONE;
	// 983 (008a55b0): esconde as 5 abas pelo SetVisible(tab, false) (008a4620)
	SetVisible(TAB_GENERAL, false);
	SetVisible(TAB_SPECIAL, false);
	SetVisible(TAB_RECORD, false);
	SetVisible(TAB_TROPHY, false);
	SetVisible(TAB_GUILD, false);
	m_petLoadTime = 0.0f;
	m_pGeneralEdit[0]->HidePrivacy(true);
	m_pGeneralEdit[0]->DisableEditStyle(FrEdit::ES_EMOTICON);
	return true;
}

void FrUserInfoForm::SetFormA(eTabType tab)
{
	if (m_tabType != tab)
	{
		SetVisible(m_tabType, false);
		m_tabType = tab;
		SetVisible(tab, true);
		FrButton** ppBtn = m_pTabBtn;
		for (int i = 0; i < 5; ++i)
		{
			FrButton* pBtn = *ppBtn;
			if (i == m_tabType)
			{
				if (pBtn)
					pBtn->SetStatus(FrButton::PRESSED);
			}
			else if (pBtn)
				pBtn->SetStatus(FrButton::NORMAL);
			++ppBtn;
		}
	}
}

void FrUserInfoForm::HidePet(bool bWait)
{
	if (bWait)
		m_anotherHand.WaitUntilWorkDone(9999);
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
	if (m_pPartTidList)
	{
		delete m_pPartTidList;
		m_pPartTidList = NULL;
	}
}

void FrUserInfoForm::SetInfo(sUserInfoTime* pInfo)
{
	m_position.iServerGUID = pInfo->userPosition.iServerGUID;
	wsprintf(m_position.csChannelName, "%s", pInfo->userPosition.csChannelName);
	m_position.iRoomIdx = pInfo->userPosition.iRoomIdx;
	m_userInfo = pInfo->info;
	// 983 (008a9290): peça que o pangya.iff não conhece sai do boneco
	for (int i = 0; i < 24; ++i)
	{
		if (m_userInfo.charInfo.tidParts[i] &&
			!ItemManager()->FindPart(m_userInfo.charInfo.tidParts[i]))
			m_userInfo.charInfo.tidParts[i] = 0;
	}
	if (IsLocalContent((localContentType_t)0x29 /*983: era 0x28; interpolado*/))
	{
		if (_strcmpi(m_userInfo.info.sNick,
				CSharedDoc::Instance()->m_myInfo.info.sNick) == 0)
		{
			// 983: histórico copiado e pacote 0x9c (RequestHistory) em linha
			CMatchHistory::Instance()->GetMatchHistory(m_matchHistory);
			if (!CMatchHistory::Instance()->IsUpdated())
				CMatchHistory::Instance()->RequestHistory();
			if (!m_pSpecListBox->FindItem((void*)m_pSpecialBack00))
				m_pSpecListBox->AddItem((void*)m_pSpecialBack00);
		}
		else if (m_pSpecListBox->FindItem((void*)m_pSpecialBack00))
		{
			m_pSpecListBox->DelItem((void*)m_pSpecialBack00);
		}
	}
	else
	{
		m_pSpecListBox->DelItem((void*)m_pSpecialBack00);
	}
	m_specialTrophy.clear();
	m_guildTrophy.clear();
	m_gpSpecialTrophy983.clear();
	// 983: as três listas são std::vector no sUserInfoTime (+0x3b0c, +0x3b1c, +0x3b2c; a 3ª vem do 0x257)
	for (int i = 0; i < (int)pInfo->trophies.size(); ++i)
		m_specialTrophy.push_back(pInfo->trophies[i]);
	for (int i = 0; i < (int)pInfo->guildTrophies.size(); ++i)
		m_guildTrophy.push_back(pInfo->guildTrophies[i]);
	for (int i = 0; i < (int)pInfo->trophies983.size(); ++i)
		m_gpSpecialTrophy983.push_back(pInfo->trophies983[i]);
	m_guildPang = pInfo->dwGuildPang;
	m_guildPoint = pInfo->dwGuildPoint;
	// 983: clássico = 3º vetor de 22 (sUserInfoTime+0xa62) e o antigo = 4º (+0xe14), não o classicMapStat
	memcpy(m_classicMapStatistics, pInfo->info.mapStat + 2 * 22,
		sizeof(m_classicMapStatistics));
	memcpy(m_oldClassicMapStatistics, pInfo->info.mapStat + 3 * 22,
		sizeof(m_oldClassicMapStatistics));
	m_oldSeason.statistics = pInfo->oldSeason.userStat;
	for (int i = 0; i < 22; ++i)
		m_oldSeason.mapStatistics[i] = pInfo->oldSeason.mapStat[i];
	m_oldSeason.trophy = pInfo->oldSeason.trophyStat;
	m_oldSeason.specialTrophy.clear();
	m_oldSeason.guildTrophy.clear();
	m_oldSeason.gpSpecialTrophy983.clear();
	for (int i = 0; i < (int)pInfo->oldSeason.trophies.size(); ++i)
		m_oldSeason.specialTrophy.push_back(pInfo->oldSeason.trophies[i]);
	for (int i = 0; i < (int)pInfo->oldSeason.guildTrophies.size(); ++i)
		m_oldSeason.guildTrophy.push_back(pInfo->oldSeason.guildTrophies[i]);
	for (int i = 0; i < (int)pInfo->oldSeason.trophies983.size(); ++i)
		m_oldSeason.gpSpecialTrophy983.push_back(pInfo->oldSeason.trophies983[i]);
	if (m_userInfo.info.dwUID != 0xffffffff)
	{
		if (m_pFriendBtn)
		{
			if (m_userInfo.info.dwUID ==
				CSharedDoc::Instance()->m_myInfo.info.dwUID)
				m_pFriendBtn->Enable(false);
			else
				m_pFriendBtn->Enable(!m_bControlOff);
		}
		if (m_pWhisperBtn)
		{
			bool bEnable = true;
			if (m_userInfo.info.dwUID ==
					CSharedDoc::Instance()->m_myInfo.info.dwUID ||
				m_bControlOff)
				bEnable = false;
			m_pWhisperBtn->Enable(bEnable);
		}
		if (m_pIgnoreBtn)
		{
			bool bEnable = true;
			if (m_userInfo.info.dwUID ==
					CSharedDoc::Instance()->m_myInfo.info.dwUID ||
				m_bControlOff)
				bEnable = false;
			m_pIgnoreBtn->Enable(bEnable);
			std::list<std::string>::iterator it =
				std::find(CSharedDoc::Instance()->m_ignoreList.begin(),
					CSharedDoc::Instance()->m_ignoreList.end(),
					m_userInfo.info.sNick);
			SetIgnoreButtonImg(
				it != CSharedDoc::Instance()->m_ignoreList.end());
		}
		if (m_userInfo.userEquip.tidSkin[0])
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(m_userInfo.userEquip.tidSkin[0]);
			if (pSkin)
			{
				m_skinScrollType[0] = pSkin->HScroll;
				m_skinScrollType[1] = pSkin->VScroll;
			}
			m_skinScroll[0] = 0.0f;
		}
	}
	SetFormA(TAB_GENERAL);
	BuildTotalStat();
	SetContent(false);
	m_bReserved983_5d30 = false;
	m_anotherHand.Work(this, 0, NULL, true, -1);
	m_bRefresh = false;
	// 983: sem o bloqueio das abas no canal 0x80 do 645
}

// 983 00899d90: item = &m_mapFlag983[i] (ponteiro + flag); melhor score primeiro, empate = maior Pang
bool __fastcall ScoreListCompare(const void* a, const void* b)
{
	const sMapStatistics* pA = *(const sMapStatistics* const*)a;
	const sMapStatistics* pB = *(const sMapStatistics* const*)b;
	if (pA->cBestScore != pB->cBestScore)
		return pA->cBestScore < pB->cBestScore;
	return pA->i64MaxPang > pB->i64MaxPang;
}

void FrUserInfoForm::SelectContents(eSeasonType type,
	sPangYaUserStatistics** ppStatistics, sTrophyStatistics** ppTrophy,
	std::list<sSpecialTrophy>** ppSpecialTrophy,
	std::list<sSpecialTrophy>** ppGpTrophy,
	std::list<sGuildTrophy>** ppGuildTrophy)
{
	// 983 (00899dc0): tambem devolve a lista de trofeus GP (0x34e0 / total 0x3f4c)
	switch (type)
	{
	case SEASON_TOTAL:
		*ppStatistics = &m_totalSeason.statistics;
		*ppTrophy = &m_totalSeason.trophy;
		*ppSpecialTrophy = &m_totalSeason.specialTrophy;
		*ppGuildTrophy = &m_totalSeason.guildTrophy;
		*ppGpTrophy = &m_totalSeason.gpSpecialTrophy983;
		break;
	default:
		*ppStatistics = &m_userInfo.stat;
		*ppTrophy = &m_userInfo.trophy;
		*ppSpecialTrophy = &m_specialTrophy;
		*ppGuildTrophy = &m_guildTrophy;
		*ppGpTrophy = &m_gpSpecialTrophy983;
		break;
	}
}

unsigned long FrUserInfoForm::GetSeasonTextColor(eSeasonType type)
{
	return 0xff000000;
}

void FrUserInfoForm::UpdateEditSchoolAndGuild(eSeasonType type,
	const sPangYaUserInfo* info)
{
	char text[128] = "";
	Doc()->LoadSchoolName(text,
		m_userInfo.info.dwUID == Doc()->m_myInfo.info.dwUID
			? Doc()->m_myInfo.info.school
			: info->school);
	if (text[0])
		m_pGeneralEdit[1]->SetLine(1, text, 0xff000000, false, 0);
	else
		m_pGeneralEdit[1]->SetLine(1,
			K2L_Compatibility("\274\322\274\323 \307\320\261\263 \276\370\300\275"), 0xff000000,
			false, 0);
	if (m_userInfo.info.IsIdentity(4))
	{
		m_pGeneralEdit[2]->SetLine(1, K2L_Compatibility("\276\313 \274\366 \276\370\300\275"),
			0xff000000, false, 0);
	}
	else
	{
		int len = strlen(info->sGuild);
		sprintf(text, "%s",
			len ? info->sGuild
				: K2L_Compatibility("\274\322\274\323 \261\346\265\345 \276\370\300\275"),
			0xff000000);
		m_pGeneralEdit[2]->SetLine(1, text, 0xff000000, false, 0);
	}
}

void FrUserInfoForm::UpdateLevelTextImg(unsigned char level)
{
	int index = level + 1;
	if (index >= 0x47)
		index = 0x47;
	const Bitmap* pBitmap = g_pFresh->GetBitmap(MakeStr("level_%03d", index));
	if (pBitmap)
	{
		// 983 (00899e50): GM (dwIdentity & 0x8000, form +0x372) mostra "gp_host" no lugar do nivel
		C_ASSERT_VC10(offsetof(FrUserInfoForm, m_userInfo) + offsetof(sUserInfo, info.dwIdentity) == 0x372);
		C_ASSERT_VC10(offsetof(FrUserInfoForm, m_bestMaxPang983) == 0x4ad0);
		if (m_userInfo.info.dwIdentity & 0x8000)
			m_pLevelArea->SetBgImg("gp_host");
		else
			m_pLevelArea->SetBgImg(pBitmap);
	}
}

bool __fastcall IsNothingRecord(const sPangYaUserStatistics* pStat)
{
	if (pStat == NULL)
		return false;
	return !(pStat->Level != 0 && pStat->dwGameCount != 0);
}

bool __fastcall IsMyRecord(const sUserInfo* pInfo)
{
	if (pInfo == NULL)
		return false;
	return pInfo->info.dwUID == Doc()->m_myInfo.info.dwUID ||
		Doc()->m_myInfo.info.IsIdentity(4);
}

bool __fastcall IsHideDisconnectRate(const sPangYaUserStatistics* pStat)
{
	return !(pStat->dwGameCount > 50 && pStat->Level >= 6);
}

// 983 008a1a70: cor pela temporada; sem o "???" do canal 0x80 (ramo do 645); sem registro, a interrogacao so na aba 0
void FrUserInfoForm::UpdateEditExpAndMannerInfo(eSeasonType type,
	const sPangYaUserStatistics* stat)
{
	char text[128] = "";
	unsigned long color = GetSeasonTextColor(type);
	if (stat->Level < 70)
	{
		sprintf(text, "%d / %d", stat->dwExp,
			CSharedDoc::Instance()->m_levelTable[stat->Level]);
	}
	else
	{
		sprintf(text, "MAX");
	}
	m_pGeneralEdit[3]->SetLine(1, text, color, false, 0);
	if (IsNothingRecord(stat))
	{
		if (m_tabType == 0)
			SetDisconnQuestionMark(true);
		sprintf(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	}
	else
	{
		SetDisconnQuestionMark(false);
		if (IsMyRecord(&m_userInfo))
		{
			if (IsHideDisconnectRate(stat))
			{
				SetDisconnQuestionMark(true);
				sprintf(text, "- - -  [%d / %d]", stat->dwNoMannerGameCount,
					stat->dwGameCount);
			}
			else
			{
				sprintf(text, "%.1f%% [%d / %d]",
					(float)stat->dwNoMannerGameCount * 100.0 /
						(float)stat->dwGameCount,
					stat->dwNoMannerGameCount, stat->dwGameCount);
			}
		}
		else if (IsHideDisconnectRate(stat))
		{
			SetDisconnQuestionMark(true);
			sprintf(text, "- - -");
		}
		else
		{
			sprintf(text, "%.1f %%",
				(float)stat->dwNoMannerGameCount * 100.0 /
					(float)stat->dwGameCount);
		}
	}
	m_pGeneralEdit[5]->SetLine(1, text, color, false, 0);
	sprintf(text, "%d / %d", stat->nMannerCombo, stat->nMaxMannerCombo);
	m_pGeneralEdit[6]->SetLine(1, text, color, false, 0);
}

__declspec(dllimport) __forceinline unsigned long SumShots14(unsigned long a,
	unsigned long b)
{
	return b + a;
}

__declspec(dllimport) __forceinline unsigned long SumShots15(unsigned long a,
	unsigned long b)
{
	return a + b;
}

__declspec(dllimport) __forceinline unsigned long SumShots16(unsigned long a,
	unsigned long b)
{
	return b + a;
}

// HACK: by-value lower-of helper (the original clamps through an inlined template whose compare is
// `a < b`, unlike Min of wminmax.h); dllimport + __forceinline keeps it from being emitted out of line
template <class T>
__declspec(dllimport) __forceinline T LowerOf(T a, T b)
{
	return (a < b) ? a : b;
}

// 983 008a7f10: trofeus GP normais (filtro 008a1830 tipo 2)
// agrupados pelo byte 2 do tid, na ordem em que cada grupo aparece; cada grupo vira um vector em m_gpTrophy983
void FrUserInfoForm::BuildGpTrophy983(const std::list<sSpecialTrophy>* pGpTrophy)
{
	m_gpTrophy983.clear();
	std::vector<unsigned long> keys;
	std::multimap<unsigned long, sSpecialTrophy> byKey;
	std::list<sSpecialTrophy>::const_iterator it;
	for (it = pGpTrophy->begin(); it != pGpTrophy->end(); ++it)
	{
		if (!IsGpTrophyOfType983(2, (*it).tid))
			continue;
		unsigned long key = (unsigned char)((*it).tid >> 16);
		if (std::find(keys.begin(), keys.end(), key) == keys.end())
			keys.push_back(key);
		byKey.insert(std::make_pair(key, *it));
	}
	for (unsigned int i = 0; i < keys.size(); ++i)
	{
		std::pair<std::multimap<unsigned long, sSpecialTrophy>::iterator,
			std::multimap<unsigned long, sSpecialTrophy>::iterator> range = byKey.equal_range(keys[i]);
		sGpTrophy983 group;
		for (std::multimap<unsigned long, sSpecialTrophy>::iterator m = range.first; m != range.second; ++m)
			group.push_back((*m).second);
		m_gpTrophy983.push_back(group);
	}
}

// 983 008a5500: um item por grupo na caixa GP normal; devolve a soma dos count
int FrUserInfoForm::FillGpTrophyBox983()
{
	int total = 0;
	FrListBox* pBox = m_pGpTrophyBox983[0];
	pBox->ClearItem();
	std::list<sGpTrophy983>::iterator it;
	for (it = m_gpTrophy983.begin(); it != m_gpTrophy983.end(); ++it)
	{
		pBox->AddItem(&*it);
		for (unsigned int i = 0; i < (*it).size(); ++i)
			total += (*it)[i].count;
	}
	return total;
}

// 983 00899c00: trofeus normais (13 mapas, do ultimo ao primeiro); devolve a soma ouro+prata+bronze
int FrUserInfoForm::FillNormalTrophyBox983(const sTrophyStatistics* pTrophy)
{
	int total = 0;
	FrListBox* pBox = (&m_pNormalTrophyBox)[0];
	pBox->ClearItem();
	for (int i = 12; i >= 0; --i)
	{
		if (pTrophy->Trophy[i][0] != 0 || pTrophy->Trophy[i][1] != 0 || pTrophy->Trophy[i][2] != 0)
		{
			pBox->AddItem((void*)i);
			total += pTrophy->Trophy[i][0] + pTrophy->Trophy[i][2] + pTrophy->Trophy[i][1];
		}
	}
	return total;
}

// 983 008a3df0: trofeus especiais; sem a caixa devolve 0
int FrUserInfoForm::FillSpecialTrophyBox983(const std::list<sSpecialTrophy>* pList)
{
	int total = 0;
	FrListBox* pBox = (&m_pNormalTrophyBox)[1];
	if (!pBox)
		return 0;
	pBox->ClearItem();
	std::list<sSpecialTrophy>::const_iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{
		pBox->AddItem((void*)&*it);
		total += (*it).count;
	}
	return total;
}

// 983 008a3e50: trofeus GP do tipo (3 = especiais, tid 0x64xxxx) filtrados por 008a1830
int FrUserInfoForm::FillGpSpecialTrophyBox983(int type, const std::list<sSpecialTrophy>* pList)
{
	int total = 0;
	FrListBox* pBox = (&m_pNormalTrophyBox)[type];
	pBox->ClearItem();
	std::list<sSpecialTrophy>::const_iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{
		if (IsGpTrophyOfType983(type, (*it).tid))
		{
			pBox->AddItem((void*)&*it);
			total += (*it).count;
		}
	}
	return total;
}

// 983 008a6040 (ret 0xc): nome do grupo GP = texto 0x7542 com o byte 2 do tid; so o tipo 2
void __stdcall FrUserInfoForm::GetGpTrophyGroupName983(int type, unsigned long tid, std::string& name)
{
	if (type == 2)
		name = MakeStr(GetGameString(0x7542), (tid >> 16) & 0xff);
}

// 983 0089f040 (sub-objeto 0x27c, ret 0xc): posicao do texto do item = pos + (20, meia altura do item da caixa do tipo)
void FrUserInfoForm::GetTrophyTextPos983(int type, const WPoint& pos, WPoint& out)
{
	float half = (&m_pNormalTrophyBox)[type]->GetItemHeight() * 0.5;
	out.x = pos.x + 20.0;
	out.y = pos.y + half;
}

// 983 008a1890 (ret 0xc): nome do grupo no estilo 1
void __stdcall FrUserInfoForm::PrintTrophyGroupName983(FrGraphicInterface* pDevice, const WPoint& pos, const std::string& name)
{
	pDevice->SetTextStyle(1);
	pDevice->Print(pos, 0, "%s", name.c_str());
	pDevice->SetTextStyle(0);
}

// 983 008a1830: item da partida existe; tipo 2 = tid fora de 0x64xxxx, tipo 3 = tid 0x64xxxx, outros = sempre
bool __stdcall FrUserInfoForm::IsGpTrophyOfType983(int type, unsigned long tid)
{
	if (ItemManager()->FindMatch(tid))
	{
		if (type == 2)
		{
			if ((tid & 0xff0000) != 0x640000)
				return true;
		}
		else
		{
			if (type != 3)
				return true;
			if ((tid & 0xff0000) == 0x640000)
				return true;
		}
	}
	return false;
}

// 983 008a5ab0: numero do tipo (edits a partir de 0x29c) em "%d", cor 0
void FrUserInfoForm::SetTrophyNum983(int type, int count)
{
	std::string text(MakeStr("%d", count));
	(&m_pNormalTrophyNum)[type]->SetLine(1, text.c_str(), 0, false, 0);
}

void FrUserInfoForm::UpdateEditSpecialInfo(eSeasonType type,
	const sPangYaUserStatistics* pStat, const sTrophyStatistics* pTrophy,
	const std::list<sSpecialTrophy>* pSpecialTrophy)
{
	// 983 008a3ec0: cor da temporada (008a3ee9) em todas as linhas; antes preto fixo
	const char* szNone = "\261\342\267\317 \276\370\300\275";
	char text[128] = { 0 };
	unsigned long color = GetSeasonTextColor(type);
	int sum[3] = { 0, 0, 0 };
	for (int j = 0; j < 3; ++j)
	{
		for (int i = 0; i < 13; ++i)
			sum[j] += pTrophy->Trophy[i][j];
	}
	sprintf(text, K2L_Compatibility("%d "), sum[0]);
	m_pSpecialEdit[0]->SetLine(1, text, color, false, 0);
	sprintf(text, K2L_Compatibility("%d "), sum[1]);
	m_pSpecialEdit[1]->SetLine(1, text, color, false, 0);
	sprintf(text, K2L_Compatibility("%d "), sum[2]);
	m_pSpecialEdit[2]->SetLine(1, text, color, false, 0);

	// Grand Prix (es0_4..6) = soma de count por (tid & 0xff) - 1, menos os de byte 2 == 'd'
	int gp[3] = { 0, 0, 0 };
	std::list<sSpecialTrophy>::const_iterator it;
	for (it = pSpecialTrophy->begin(); it != pSpecialTrophy->end(); ++it)
	{
		if (((it->tid >> 16) & 0xff) != 'd')
			gp[(unsigned short)((it->tid & 0xff) - 1)] += it->count;
	}
	sprintf(text, K2L_Compatibility("%d "), gp[0]);
	m_pSpecialEdit[3]->SetLine(1, text, color, false, 0);
	sprintf(text, K2L_Compatibility("%d "), gp[1]);
	m_pSpecialEdit[4]->SetLine(1, text, color, false, 0);
	sprintf(text, K2L_Compatibility("%d "), gp[2]);
	m_pSpecialEdit[5]->SetLine(1, text, color, false, 0);

	if (pStat->dwTeamGames == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
		sprintf(text, "%.1f %%",
			(float)pStat->dwTeamWin / (float)pStat->dwTeamGames * 100.0f);
	m_pSpecialEdit[6]->SetLine(1, text, color, false, 0);

	if (pStat->dwHole - pStat->dwMatchHole == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
	{
		// 983: m�dia em double; no pa�s 0x1e limitada a 100
		double score = pStat->iTotalScore * 18.0;
		if (IsLocalCountry(0x1e))
		{
			double average = score / (pStat->dwHole - pStat->dwMatchHole) + 72.0;
			sprintf(text, K2L_Compatibility("%.2f \305\270"), average > 100.0 ? 100.0 : average);
		}
		else
			sprintf(text, K2L_Compatibility("%.2f \305\270"),
				score / (pStat->dwHole - pStat->dwMatchHole) + 72.0);
	}
	m_pSpecialEdit[7]->SetLine(1, text, color, false, 0);

	if (pStat->dwDrive + pStat->dwPutt == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
		sprintf(text, K2L_Compatibility("%.1f \303\312"),
			(float)pStat->dwShotTime / (float)(pStat->dwDrive + pStat->dwPutt));
	m_pSpecialEdit[8]->SetLine(1, text, color, false, 0);

	if (pStat->dwDrive == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
		sprintf(text, "%.1f %%",
			(float)pStat->dwPangya / (float)pStat->dwDrive * 100.0f);
	m_pSpecialEdit[9]->SetLine(1, text, color, false, 0);

	if (pStat->dwHole - pStat->dwMatchHole == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
		sprintf(text, "%.1f %%",
			(float)pStat->dwFairway /
				(float)(pStat->dwHole - pStat->dwMatchHole) * 100.0f);
	m_pSpecialEdit[10]->SetLine(1, text, color, false, 0);

	if (pStat->dwPutt == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
		sprintf(text, "%.1f %%",
			(float)pStat->dwPuttIn / (float)pStat->dwPutt * 100.0f);
	m_pSpecialEdit[11]->SetLine(1, text, color, false, 0);

	if (pStat->dwDrive + pStat->dwPutt == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
		sprintf(text, "%.1f %%",
			(float)pStat->dwOB / (float)(pStat->dwDrive + pStat->dwPutt) *
				100.0f);
	m_pSpecialEdit[12]->SetLine(1, text, color, false, 0);

	if (pStat->i64TotalSkinsPang == 0)
		strcpy(text, K2L_Compatibility(szNone));
	else
		sprintf(text, "%I64d", pStat->i64TotalSkinsPang);
	m_pSpecialEdit[13]->SetLine(1, text, color, false, 0);
}

// 983 008a1290/008a15b0/008a1420: junta o mapa das duas listas do pacote 0x15c (a 2a = recorde com o Play Assist):
// copia o de melhor score (empate: o de maior Pang) e fica com o maior Pang dos dois; devolve 1 se veio da 2a
static __forceinline unsigned char MergeMapStat983(const sMapStatistics* pA, const sMapStatistics* pB, sMapStatistics* pOut)
{
	const sMapStatistics* pSrc;
	unsigned char fromB;
	__int64 maxPang;
	if (pB->cBestScore < pA->cBestScore)
	{
		pSrc = pB;
		fromB = 1;
		maxPang = pB->i64MaxPang;
		if (pA->cBestScore != 0x7f && pA->i64MaxPang > maxPang)
			maxPang = pA->i64MaxPang;
	}
	else if (pA->cBestScore < pB->cBestScore)
	{
		pSrc = pA;
		fromB = 0;
		maxPang = pA->i64MaxPang;
		if (pB->cBestScore != 0x7f && pB->i64MaxPang > maxPang)
			maxPang = pB->i64MaxPang;
	}
	else
	{
		fromB = pB->i64MaxPang > pA->i64MaxPang ? 1 : 0;
		pSrc = fromB ? pB : pA;
		maxPang = pSrc->i64MaxPang;
	}
	*pOut = *pSrc;
	pOut->i64MaxPang = maxPang;
	return fromB;
}

static __forceinline bool IsFinalCourse983(unsigned char bMap)
{
	unsigned long course = (bMap < 0x80 || bMap == 0xfd) ? bMap : bMap - 0x80;
	IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(course | 0x28000000);
	return pCourse && pCourse->c.Final;
}

// 983 008a1290: pA[i] x pB[i] -> m_mapStatistics983[0] + m_mapFlag983
void FrUserInfoForm::MergeMapStatistics983(const sMapStatistics* pA, const sMapStatistics* pB)
{
	sMapStatistics* pOut = m_mapStatistics983[0];
	memset(pOut, 0, sizeof(m_mapStatistics983[0]));
	for (int i = 0; i < 22; ++i)
	{
		pOut[i].cBestScore = 0x7f;
		if (i == 0x11 || !IsFinalCourse983(pA[i].bMap))
			continue;
		m_mapFlag983[i].b = MergeMapStat983(&pA[i], &pB[i], &pOut[i]);
		m_mapFlag983[i].v = (unsigned long)&pOut[i];
	}
}

// 983 008a15b0: total de uma lista (pA = temporada anterior, pB = atual) -> m_mapStatistics983[1] ou [2]
void FrUserInfoForm::MergeSeasonMapStatistics983(const sMapStatistics* pA, const sMapStatistics* pB,
	sMapStatistics** ppOut, bool bSecond)
{
	sMapStatistics* pOut = m_mapStatistics983[bSecond ? 2 : 1];
	memset(pOut, 0, sizeof(m_mapStatistics983[0]));
	for (int i = 0; i < 22; ++i)
	{
		pOut[i].cBestScore = 0x7f;
		if (i == 0x11 || !IsFinalCourse983(pA[i].bMap))
			continue;
		MergeMapStat983(&pA[i], &pB[i], &pOut[i]);
		ppOut[i] = &pOut[i];
	}
}

// 983 008a1420: junta os dois totais (sem e com Play Assist) -> m_mapStatistics983[3] + m_mapFlag983
void FrUserInfoForm::MergeTotalMapStatistics983(sMapStatistics* const* ppA, sMapStatistics* const* ppB)
{
	sMapStatistics* pOut = m_mapStatistics983[3];
	memset(pOut, 0, sizeof(m_mapStatistics983[0]));
	for (int i = 0; i < 22; ++i)
	{
		pOut[i].cBestScore = 0x7f;
		if (i == 0x11 || !ppA[i] || !ppB[i] || !IsFinalCourse983(ppA[i]->bMap))
			continue;
		m_mapFlag983[i].b = MergeMapStat983(ppA[i], ppB[i], &pOut[i]);
		m_mapFlag983[i].v = (unsigned long)&pOut[i];
	}
}

// 983 008a1d00: 22 mapas; cada linha junta a lista normal e a do Play Assist (vetores de 22 do pacote 0x15c a partir de
// mapStat: 0/4 atual, 1/5 anterior; 2/6 e 3/7 os mesmos no bClassic); item da lista = &m_mapFlag983[i]
void FrUserInfoForm::UpdateEditMapStatistics(eSeasonType type, bool bClassic)
{
	const sMapStatistics* pArr = m_userInfo.mapStat; // os 12 vetores sao seguidos (mapStat, classicMapStat, mapStat983)
	sMapStatistics* first[22] = { 0 };
	sMapStatistics* second[22] = { 0 };
	const int k = bClassic ? 2 : 0;
	memset(m_mapFlag983, 0, sizeof(m_mapFlag983));
	if (type == SEASON_TOTAL)
	{
		MergeSeasonMapStatistics983(pArr + (k + 1) * 22, pArr + k * 22, first, false);
		MergeSeasonMapStatistics983(pArr + (k + 5) * 22, pArr + (k + 4) * 22, second, true);
		MergeTotalMapStatistics983(first, second);
	}
	else
		MergeMapStatistics983(pArr + k * 22, pArr + (k + 4) * 22);

	if (m_pScoreList)
	{
		m_pScoreList->ClearItem();
		for (int i = 0; i < 22; ++i)
		{
			const sMapStatistics* pMap = (const sMapStatistics*)m_mapFlag983[i].v;
			if (pMap && pMap->bMap != 0x11 && IsFinalCourse983(pMap->bMap))
				m_pScoreList->AddItem(&m_mapFlag983[i]);
		}
		m_pScoreList->SortItem(ScoreListCompare);
	}
}

void FrUserInfoForm::SetDisconntextAlign()
{
	if (m_pQuestionMark)
	{
		WRect rect;
		rect = m_pGeneralEdit[5]->GetRect();
		if (m_pQuestionMark->IsVisible())
		{
			if (rect.x == m_disconnRect.x)
			{
				rect.x -= m_pQuestionMark->GetRect().w;
				m_pGeneralEdit[5]->SetRect(rect);
			}
		}
		else if (rect.x != m_disconnRect.x)
		{
			rect.x += m_pQuestionMark->GetRect().w;
			m_pGeneralEdit[5]->SetRect(rect);
		}
	}
}

void FrUserInfoForm::SetDisconnQuestionMark(bool bVisible)
{
	if (m_pQuestionMark)
	{
		m_pQuestionMark->SetVisible(bVisible);
		SetDisconntextAlign();
	}
}

extern void __fastcall RemakeNumber(int number, char* buf);

// 983 008a1ed0: 22 mapas; alem do melhor mapa por dificuldade guarda o maior Pang (sem 0x7f) da dificuldade em pMaxPang
void __fastcall SortMapStat(sMapStatistics** const ppMap, sMapStatistics* pStat, __int64* const pMaxPang)
{
	for (int i = 0; i < 22; ++i, ++pStat)
	{
		if (i == 0x11)
			continue;
		unsigned long course = (pStat->bMap > 0x7f && pStat->bMap != 0xfd)
			? pStat->bMap - 0x80
			: pStat->bMap;
		IFF_STRUCT::sCourse* pCourse =
			ItemManager()->FindCourse(course | 0x28000000);
		if (pCourse && pCourse->c.Final)
		{
			sMapStatistics*& pBest = ppMap[pCourse->Difficulty - 1];
			__int64& maxPang = pMaxPang[pCourse->Difficulty - 1];
			if (pBest == NULL)
			{
				pBest = pStat;
				if (pStat->cBestScore != 0x7f)
					maxPang = pStat->i64MaxPang;
			}
			else if (pStat->cBestScore != 0x7f)
			{
				if (pStat->i64MaxPang > maxPang)
					maxPang = pStat->i64MaxPang;
				if (pStat->cBestScore < pBest->cBestScore)
					pBest = pStat;
				else if (pStat->cBestScore == pBest->cBestScore &&
					pStat->i64MaxPang != 0 &&
					pStat->i64MaxPang > pBest->i64MaxPang)
					pBest = pStat;
			}
		}
	}
}


void FrUserInfoForm::SetContent(bool bRefresh)
{
	if (m_userInfo.info.dwUID == 0xffffffff)
		return;

	// HACK: emits g_HUGE next to g_EPSILON, as the original's .rdata has both.
	(void)&g_HUGE;

	m_bRefresh = bRefresh;
	sPangYaUserStatistics* pStat = NULL;
	sTrophyStatistics* pTrophy = NULL;
	std::list<sSpecialTrophy>* pSpecialTrophy = NULL;
	std::list<sSpecialTrophy>* pGpTrophy = NULL;
	std::list<sGuildTrophy>* pGuildTrophy = NULL;
	const sPangYaUserInfo* pInfo = &m_userInfo.info;
	memset(m_pBestScoreMap, 0, sizeof(m_pBestScoreMap));
	if (m_seasonType == SEASON_CURRENT)
		SortMapStat(m_pBestScoreMap, m_userInfo.mapStat, m_bestMaxPang983);
	else if (m_seasonType == SEASON_TOTAL)
		SortMapStat(m_pBestScoreMap, m_totalSeason.mapStatistics, m_bestMaxPang983);
	SelectContents(m_seasonType, &pStat, &pTrophy, &pSpecialTrophy,
		&pGpTrophy, &pGuildTrophy);
	unsigned long color = GetSeasonTextColor(m_seasonType); // 983 008a8459

	m_pGeneralEdit[0]->SetLine(1, m_userInfo.info.sNick, color, false, 0);
	UpdateEditSchoolAndGuild(m_seasonType, pInfo);
	sPangYaUserStatistics* pUserStat = pStat;
	UpdateEditExpAndMannerInfo(m_seasonType, pUserStat);
	UpdateEditUserLocation();
	UpdateLevelTextImg(pUserStat->Level);
	UpdateEditSpecialInfo(m_seasonType, pUserStat, pTrophy, pGpTrophy); // 983 008a84f4: lista GP
	SetMatchHistoryContents();

	char text[128] = "";
	int hour = pUserStat->dwVisitTime / 3600;
	int minute = (pUserStat->dwVisitTime - hour * 3600) / 60;
	if (hour == 0)
		sprintf(text, K2L_Compatibility("%d \272\320"), minute);
	else if (hour < 24)
		sprintf(text, K2L_Compatibility("%d \275\303\260\243 %d \272\320"), hour, minute);
	else
		sprintf(text, K2L_Compatibility("%d \300\317 %d \275\303\260\243 %d \272\320"), hour / 24,
			hour % 24, minute);
	m_pRecordEdit[0]->SetLine(1, text, color, false, 0);

	sprintf(text, K2L_Compatibility("%d "), (int)(pUserStat->dwDistance / 43745.48828125)); // 983 008a85f7
	m_pRecordEdit[3]->SetLine(1, text, color, false, 0);
	if (pUserStat->dwDistance == 0)
		strcpy(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	else
		sprintf(text, K2L_Compatibility("%.1f\270\270 y "),
			(double)((float)pUserStat->dwDistance * 0.0001f));
	m_pRecordEdit[4]->SetLine(1, text, color, false, 0);

	if (pUserStat->dwPutt + pUserStat->dwDrive == 0)
	{
		strcpy(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	}
	else
	{
		RemakeNumber(pUserStat->dwPutt + pUserStat->dwDrive, text);
		strcat(text, K2L_Compatibility(" \305\270"));
	}
	m_pRecordEdit[1]->SetLine(1, text, color, false, 0);

	if (pUserStat->dwHole == 0)
	{
		strcpy(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	}
	else
	{
		RemakeNumber(pUserStat->dwHole, text);
		strcat(text, K2L_Compatibility(" \310\246"));
	}
	m_pRecordEdit[2]->SetLine(1, text, color, false, 0);

	if (Wabs(pUserStat->fLongest) < g_EPSILON)
		strcpy(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	else
		sprintf(text, "%.2f y", pUserStat->fLongest);
	m_pRecordEdit[12]->SetLine(1, text, color, false, 0);

	if (Wabs(pUserStat->fLongestPuttIn) < g_EPSILON)
		strcpy(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	else
		sprintf(text, "%.2f y", pUserStat->fLongestPuttIn);
	m_pRecordEdit[13]->SetLine(1, text, color, false, 0);

	if (Wabs(pUserStat->fLongestChipIn) < g_EPSILON)
		strcpy(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	else
		sprintf(text, "%.2f y", pUserStat->fLongestChipIn);
	m_pRecordEdit[14]->SetLine(1, text, color, false, 0);

	char bestText[5][128];
	if (m_seasonType == SEASON_CURRENT || m_seasonType == SEASON_TOTAL)
	{
		for (int i = 0; i < 5; ++i)
		{
			if (m_pBestScoreMap[4 - i] == NULL)
				strcpy(bestText[4 - i], K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
			else if (m_pBestScoreMap[4 - i]->cBestScore == 0)
				sprintf(bestText[4 - i], "%+d [%I64dp]",
					m_pBestScoreMap[4 - i]->cBestScore,
					m_bestMaxPang983[4 - i]);
			else if (m_pBestScoreMap[4 - i]->cBestScore == 0x7f)
				strcpy(bestText[4 - i], K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
			else
				sprintf(bestText[4 - i], "%+d [%I64dp]",
					m_pBestScoreMap[4 - i]->cBestScore,
					m_bestMaxPang983[4 - i]);
		}
	}
	if (m_seasonType == SEASON_CURRENT || m_seasonType == SEASON_TOTAL)
	{
		m_pRecordEdit[5]->SetLine(1, bestText[4], color, false, 0);
		m_pRecordEdit[6]->SetLine(1, bestText[3], color, false, 0);
	}
	m_pRecordEdit[7]->SetLine(1, bestText[2], color, false, 0);
	m_pRecordEdit[8]->SetLine(1, bestText[1], color, false, 0);
	m_pRecordEdit[9]->SetLine(1, bestText[0], color, false, 0);

	sPangYaUserStatistics* pStat2 = pStat;
	sprintf(text, "%d", pStat2->wHoleInOne);
	m_pRecordEdit[10]->SetLine(1, text, color, false, 0);
	sprintf(text, "%d", pStat2->wAlbatross);
	m_pRecordEdit[11]->SetLine(1, text, color, false, 0);
	// 983 008a8d..: contadores dos 6 premios da aba Record>Award (+0xeb,+0xef,+0xf3,+0xf7,+0xfb,+0xe7 da estatistica; "-" se 0) e
	// o nivel de quiz do General (ushort +0xff; 0 = "Sem registro", senao "%d grau" = 6 - valor)
	static const unsigned char kAwardOff[6] = { 0xeb, 0xef, 0xf3, 0xf7, 0xfb, 0xe7 };
	for (int i = 0; i < 6; ++i)
	{
		int count = *(const int*)((const char*)pStat2 + kAwardOff[i]);
		if (count == 0)
			strcpy(text, "-");
		else
			sprintf(text, "%d", count);
		if (m_pRecAwardWnd[i])
			((FrEdit*)m_pRecAwardWnd[i])->SetLine(1, text, color, false, 0);
	}
	unsigned short quiz = *(const unsigned short*)((const char*)pStat2 + 0xff);
	if (quiz == 0)
		strcpy(text, K2L_Compatibility("\261\342\267\317 \276\370\300\275"));
	else
		sprintf(text, K2L_Compatibility("%d \261\336"), 6 - quiz);
	if (m_pGeneralEdit[7])
		m_pGeneralEdit[7]->SetLine(1, text, color, false, 0);
	const bool& bTotal = (m_pRecordBtn[3]->GetStatus() == FrButton::PRESSED);
	UpdateEditMapStatistics(m_seasonType, bTotal);

	// 983 008a8e..: cada tipo enche a sua lista e o numero do tipo recebe a soma (008a5ab0: MakeStr("%d"), cor 0)
	// 0 normal (00899c00), 1 special (008a3df0), 3 GP special (008a3e50: so trofeus GP de tid 0x64xxxx, filtro 008a1830)
	// 2 GP normal: 008a7f10 agrupa a lista GP em m_gpTrophy983 e 008a5500 enche a caixa e soma
	SetTrophyNum983(0, FillNormalTrophyBox983(pTrophy));
	SetTrophyNum983(1, FillSpecialTrophyBox983(pSpecialTrophy));
	BuildGpTrophy983(pGpTrophy);
	SetTrophyNum983(2, FillGpTrophyBox983());
	SetTrophyNum983(3, FillGpSpecialTrophyBox983(3, pGpTrophy));

	if (m_pGuildTrophyList)
	{
		m_pGuildTrophyList->ClearItem();
		std::list<sGuildTrophy>::reverse_iterator it;
		for (it = pGuildTrophy->rbegin(); it != pGuildTrophy->rend(); ++it)
			m_pGuildTrophyList->AddItem(&*it);
	}

	if (m_userInfo.info.dwGuildId != 0)
	{
		sprintf(text, "%s", m_userInfo.info.sGuild);
		m_pGuildEdit[0]->SetLine(1, text, color, false, 0);
		sprintf(text, "%d", m_guildPang);
		m_pGuildEdit[1]->SetLine(1, text, color, false, 0);
		sprintf(text, "%d", m_guildPoint);
	}
	else
	{
		strcpy(text, K2L_Compatibility("\276\370\300\275"));
		m_pGuildEdit[0]->SetLine(1, text, color, false, 0);
		strcpy(text, K2L_Compatibility("\276\370\300\275"));
		m_pGuildEdit[1]->SetLine(1, text, color, false, 0);
		strcpy(text, K2L_Compatibility("\276\370\300\275"));
	}
	m_pGuildEdit[2]->SetLine(1, text, color, false, 0);
}

void FrUserInfoForm::BuildTotalStat()
{
	sPangYaUserStatistics& total = m_totalSeason.statistics;
	const sPangYaUserStatistics& cur = m_userInfo.stat;
	const sPangYaUserStatistics& old = m_oldSeason.statistics;

	m_totalSeason.Clear();
	total.Level = cur.Level;
	total.dwExp = cur.dwExp;
	total.dwGameCount = cur.dwGameCount;
	total.dwNoMannerGameCount = cur.dwNoMannerGameCount;
	total.nMannerCombo = cur.nMannerCombo;
	total.nMaxMannerCombo = cur.nMaxMannerCombo;
	total.dwTeamGames = cur.dwTeamGames + old.dwTeamGames;
	total.dwTeamWin = cur.dwTeamWin + old.dwTeamWin;
	total.dwHole = cur.dwHole + old.dwHole;
	total.dwMatchHole = cur.dwMatchHole + old.dwMatchHole;
	total.iTotalScore = cur.iTotalScore + old.iTotalScore;
	total.dwDrive = cur.dwDrive + old.dwDrive;
	total.dwPutt = cur.dwPutt + old.dwPutt;
	total.dwShotTime = cur.dwShotTime + old.dwShotTime;
	total.dwPangya = cur.dwPangya + old.dwPangya;
	total.dwFairway = cur.dwFairway + old.dwFairway;
	total.dwPuttIn = cur.dwPuttIn + old.dwPuttIn;
	total.dwOB = cur.dwOB + old.dwOB;
	total.i64TotalSkinsPang = cur.i64TotalSkinsPang + old.i64TotalSkinsPang;
	total.dwVisitTime = cur.dwVisitTime + old.dwVisitTime;
	total.dwDistance = cur.dwDistance + old.dwDistance;
	if (cur.fLongest <= old.fLongest)
		total.fLongest = old.fLongest;
	else
		total.fLongest = cur.fLongest;
	if (cur.fLongestPuttIn <= old.fLongestPuttIn)
		total.fLongestPuttIn = old.fLongestPuttIn;
	else
		total.fLongestPuttIn = cur.fLongestPuttIn;
	if (cur.fLongestChipIn <= old.fLongestChipIn)
		total.fLongestChipIn = old.fLongestChipIn;
	else
		total.fLongestChipIn = cur.fLongestChipIn;
	total.wHoleInOne = cur.wHoleInOne + old.wHoleInOne;
	total.wAlbatross = cur.wAlbatross + old.wAlbatross;

	for (int i = 0; i < 5; ++i)
	{
		total.cBestScore[i] = 0x7f;
		if (old.cBestScore[i] != 0x7f)
		{
			total.cBestScore[i] = old.cBestScore[i];
			total.i64MaxPang[i] = old.i64MaxPang[i];
		}
		if (cur.cBestScore[i] != 0x7f)
		{
			if (cur.cBestScore[i] < total.cBestScore[i])
			{
				total.cBestScore[i] = cur.cBestScore[i];
				total.i64MaxPang[i] = cur.i64MaxPang[i];
			}
			else if (total.cBestScore[i] == cur.cBestScore[i] &&
				total.i64MaxPang[i] < cur.i64MaxPang[i])
			{
				total.i64MaxPang[i] = cur.i64MaxPang[i];
			}
		}
	}

	for (int i = 0; i < 20; ++i)
	{
		m_totalSeason.mapStatistics[i].bMap = i;
		m_totalSeason.mapStatistics[i].cBestScore = 0x7f;
		if (m_oldSeason.mapStatistics[i].cBestScore != 0x7f)
			m_totalSeason.mapStatistics[i] = m_oldSeason.mapStatistics[i];
		if (m_userInfo.mapStat[i].cBestScore != 0x7f &&
			m_userInfo.mapStat[i].cBestScore <
				m_totalSeason.mapStatistics[i].cBestScore)
			m_totalSeason.mapStatistics[i] = m_userInfo.mapStat[i];
	}

	for (int i = 0; i < 20; ++i)
	{
		m_totalClassicMapStatistics[i].bMap = i;
		m_totalClassicMapStatistics[i].cBestScore = 0x7f;
		if (m_oldClassicMapStatistics[i].cBestScore != 0x7f)
			m_totalClassicMapStatistics[i] = m_oldClassicMapStatistics[i];
		if (m_classicMapStatistics[i].cBestScore != 0x7f &&
			m_classicMapStatistics[i].cBestScore <
				m_totalClassicMapStatistics[i].cBestScore)
			m_totalClassicMapStatistics[i] = m_classicMapStatistics[i];
	}

	for (int j = 0; j < 3; ++j)
	{
		for (int i = 0; i < 13; ++i)
		{
			m_totalSeason.trophy.Trophy[i][j] =
				m_totalSeason.trophy.Trophy[i][j] +
				m_userInfo.trophy.Trophy[i][j] +
				m_oldSeason.trophy.Trophy[i][j];
		}
	}

	m_totalSeason.specialTrophy.clear();
	std::list<sSpecialTrophy>::iterator itSpecial;
	for (itSpecial = m_oldSeason.specialTrophy.begin();
		itSpecial != m_oldSeason.specialTrophy.end(); ++itSpecial)
		m_totalSeason.specialTrophy.push_back(*itSpecial);
	for (itSpecial = m_specialTrophy.begin();
		itSpecial != m_specialTrophy.end(); ++itSpecial)
		m_totalSeason.specialTrophy.push_back(*itSpecial);

	m_totalSeason.guildTrophy.clear();
	std::list<sGuildTrophy>::iterator itGuild;
	for (itGuild = m_oldSeason.guildTrophy.begin();
		itGuild != m_oldSeason.guildTrophy.end(); ++itGuild)
		m_totalSeason.guildTrophy.push_back(*itGuild);
	for (itGuild = m_guildTrophy.begin(); itGuild != m_guildTrophy.end();
		++itGuild)
		m_totalSeason.guildTrophy.push_back(*itGuild);

	// 983 (008a6bfb..008a6ce3): total GP = temporadas antigas + atual
	m_totalSeason.gpSpecialTrophy983.clear();
	for (itSpecial = m_oldSeason.gpSpecialTrophy983.begin();
		itSpecial != m_oldSeason.gpSpecialTrophy983.end(); ++itSpecial)
		m_totalSeason.gpSpecialTrophy983.push_back(*itSpecial);
	for (itSpecial = m_gpSpecialTrophy983.begin();
		itSpecial != m_gpSpecialTrophy983.end(); ++itSpecial)
		m_totalSeason.gpSpecialTrophy983.push_back(*itSpecial);
}

unsigned long FrUserInfoForm::GetBestScoreColor(unsigned char index)
{
	if (m_userInfo.info.dwUID == Doc()->m_myInfo.info.dwUID)
		return 0xff000000;
	bool bBetter;
	if (m_userInfo.stat.cBestScore[index] == 0x7f)
		bBetter = false;
	else if (Doc()->m_myInfo.stat.cBestScore[index] == 0x7f)
		bBetter = true;
	else
		bBetter = m_userInfo.stat.cBestScore[index] <
			Doc()->m_myInfo.stat.cBestScore[index];
	return bBetter ? 0xffffa045 : 0xff000000;
}

unsigned long FrUserInfoForm::GetMaxPangColor(unsigned char index)
{
	if (m_userInfo.info.dwUID == Doc()->m_myInfo.info.dwUID)
		return 0xffffffff;
	bool bBetter = m_userInfo.stat.i64MaxPang[index] >
		Doc()->m_myInfo.stat.i64MaxPang[index];
	return bBetter ? 0xffffa045 : 0xff000000;
}

unsigned long FrUserInfoForm::Get30sRecordColor(int index)
{
	if (m_userInfo.info.dwUID == Doc()->m_myInfo.info.dwUID)
		return 0xff000000;
	int mySum = 0;
	int sum = 0;
	for (int i = 0; i < 13; ++i)
	{
		mySum += Doc()->m_myInfo.trophy.Trophy[i][index];
		sum += m_userInfo.trophy.Trophy[i][index];
	}
	return sum > mySum ? 0xffffa045 : 0;
}

unsigned long FrUserInfoForm::GetColor(eTabType tab, unsigned long index)
{
	if (m_userInfo.info.dwUID == Doc()->m_myInfo.info.dwUID &&
		(tab != 0 || index != 5))
		return 0xff000000;

	const sPangYaUserStatistics& st = m_userInfo.stat;
	const sPangYaUserStatistics& my = Doc()->m_myInfo.stat;

	bool bHigher = false;
	switch (tab)
	{
	case 0:
		if (index != 5)
			break;
		if (st.dwGameCount == 0)
			return 0xff000000;
		{
			unsigned char c = (unsigned char)(0xff -
				(unsigned char)(int)((float)st.dwNoMannerGameCount /
					(float)st.dwGameCount * 255.0f));
			return ((c | 0xffffff00) << 8) | c;
		}
	case 1:
		switch (index)
		{
		case 0:
			bHigher = st.dwVisitTime > my.dwVisitTime;
			break;
		case 4:
			bHigher = st.dwDistance > my.dwDistance;
			break;
		case 1:
			bHigher = st.dwPutt + st.dwDrive > my.dwPutt + my.dwDrive;
			break;
		case 2:
			bHigher = st.dwHole > my.dwHole;
			break;
		case 12:
			bHigher = st.fLongest > my.fLongest;
			break;
		case 13:
			bHigher = st.fLongestPuttIn > my.fLongestPuttIn;
			break;
		case 14:
			bHigher = st.fLongestChipIn > my.fLongestChipIn;
			break;
		case 10:
			bHigher = st.wHoleInOne > my.wHoleInOne;
			break;
		case 11:
			bHigher = st.wAlbatross > my.wAlbatross;
			break;
		}
		break;
	case 2:
		switch (index)
		{
		case 3:
			if (st.dwTeamGames == 0)
				bHigher = false;
			else if (my.dwTeamGames == 0)
				bHigher = true;
			else
				bHigher = (float)st.dwTeamWin / (float)st.dwTeamGames >
					(float)my.dwTeamWin / (float)my.dwTeamGames;
			break;
		case 4:
			if (st.dwHole - st.dwMatchHole == 0)
				bHigher = false;
			else if (my.dwHole - my.dwMatchHole == 0)
				bHigher = true;
			else
				bHigher = (float)st.iTotalScore /
						(float)(st.dwHole - st.dwMatchHole) <
					(float)my.iTotalScore / (float)(my.dwHole - my.dwMatchHole);
			break;
		case 5:
			if (st.dwDrive + st.dwPutt == 0)
				bHigher = false;
			else if (my.dwPutt + my.dwDrive == 0)
				bHigher = true;
			else
				bHigher =
					(float)st.dwShotTime / (float)(st.dwDrive + st.dwPutt) <
					(float)my.dwShotTime / (float)(my.dwPutt + my.dwDrive);
			break;
		case 6:
			if (st.dwDrive == 0)
				bHigher = false;
			else if (my.dwDrive == 0)
				bHigher = true;
			else
				bHigher = (float)st.dwPangya / (float)st.dwDrive >
					(float)my.dwPangya / (float)my.dwDrive;
			break;
		case 7:
			if (st.dwHole - st.dwMatchHole == 0)
				bHigher = false;
			else if (my.dwHole - my.dwMatchHole == 0)
				bHigher = true;
			else
				bHigher =
					(float)st.dwFairway / (float)(st.dwHole - st.dwMatchHole) >
					(float)my.dwFairway / (float)(my.dwHole - my.dwMatchHole);
			break;
		case 8:
			if (st.dwPutt == 0)
				bHigher = false;
			else if (my.dwPutt == 0)
				bHigher = true;
			else
				bHigher = (float)st.dwPuttIn / (float)st.dwPutt >
					(float)my.dwPuttIn / (float)my.dwPutt;
			break;
		case 9:
			if (st.dwDrive + st.dwPutt == 0)
				bHigher = false;
			else if (my.dwPutt + my.dwDrive == 0)
				bHigher = true;
			else
				bHigher = (float)st.dwOB / (float)(st.dwDrive + st.dwPutt) <
					(float)my.dwOB / (float)(my.dwPutt + my.dwDrive);
			break;
		}
		break;
	}
	return bHigher ? 0xffffa045 : 0;
}

void FrUserInfoForm::OnPetInit(int param)
{
	m_pPetArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUserInfoForm::OnPetOwnerDraw()
{
	g_view->Clear(0, 8); // 983 008a2739: limpa o stencil, nao o Z (0, 2 era do 645)
	if (m_pPetLoadGauge && m_pPetLoadGauge->IsVisible())
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	WRect area = m_pPetArea->GetRect();
	IFF_STRUCT::sSkin* pSkin;
	const Bitmap* pBmp;
	if (m_userInfo.userEquip.tidSkin[0] &&
		(pSkin = ItemManager()->FindSkin(m_userInfo.userEquip.tidSkin[0])) &&
		(pBmp = g_pFresh->GetBitmap(pSkin->Tex)))
	{
		float bw = (float)pBmp->Width();
		float bh = (float)pBmp->Height();
		if (m_skinScroll[0] > bw)
			m_skinScroll[0] -= bw;
		else if (m_skinScroll[0] < 0.0f)
			m_skinScroll[0] += bw;
		if (m_skinScroll[1] > bh)
			m_skinScroll[1] -= bh;
		else if (m_skinScroll[1] < 0.0f)
			m_skinScroll[1] += bh;

		WRect src(m_skinScroll[0], m_skinScroll[1], bw - m_skinScroll[0],
			bh - m_skinScroll[1]);
		WRect dst(area.x, area.y, src.w, src.h);
		float right = area.x + area.w;
		float bottom = area.y + area.h;
		while (dst.y < bottom)
		{
			src.h = dst.h;
			dst.x = area.x;
			while (dst.x < right)
			{
				src.w = dst.w;
				pDevice->DrawTexture(pBmp, src, dst, 0xffffffff, 0);
				dst.x += dst.w;
				dst.w = right - dst.x;
				if (bw <= dst.w)
					dst.w = bw;
				src.x = 0.0f;
			}
			dst.y += dst.h;
			dst.x = area.x;
			src.w = bw - m_skinScroll[0];
			dst.w = src.w;
			dst.h = bottom - dst.y;
			if (bh <= dst.h)
				dst.h = bh;
			src.x = m_skinScroll[0];
			src.h = dst.h;
			src.y = 0.0f;
		}
	}

	if (!m_anotherHand.IsWorking(0))
	{
		m_anotherHand.Enter();
		if (m_pExhibition)
			m_pExhibition->Display(0.0f, 0.0f);
		m_anotherHand.Leave();
	}

	if (m_userInfo.userEquip.tidSkin[1] &&
		(pSkin = ItemManager()->FindSkin(m_userInfo.userEquip.tidSkin[1])) &&
		(pBmp = g_pFresh->GetBitmap(pSkin->Tex)))
		pDevice->DrawTexture(pBmp, area, 0xffffffff, 0);

	if (m_userInfo.userEquip.tidSkin[2] &&
		(pSkin = ItemManager()->FindSkin(m_userInfo.userEquip.tidSkin[2])) &&
		(pBmp = g_pFresh->GetBitmap(pSkin->Tex)))
		pDevice->DrawTexture(pBmp, area, 0xffffffff, 0);
}

void FrUserInfoForm::OnPetLoadInit(int param)
{
	m_pPetLoadGauge = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	if (m_pPetLoadGauge)
	{
		m_pPetLoadGauge->SetRange(0, 120, 0);
		m_pPetLoadGauge->SetPos(0);
		m_pPetLoadGauge->SetDestPos(120, false);
		m_pPetLoadGauge->SetVisible(true);
	}
}

void FrUserInfoForm::ShowPet()
{
	m_anotherHand.Enter();
	HidePet(false);
	if (m_pPetArea)
	{
		WRect rect = m_pPetArea->GetRect();
		m_pExhibition = new CExhibitionByHardwareTnL(rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f, 0.35f, true);
		m_pExhibition->SetDistance(18.75f /* 983 0089f290: fixo, nao escala com a largura */, true,
			false);
		m_pExhibition->SetArea(rect.w, rect.h);
		m_pPartTidList = new CPartTidList;
		sCharacterInfo* charInfo = &m_userInfo.charInfo;
		if (!m_pPartTidList->SetTids(charInfo, 0xff) ||
			!m_pPartTidList->IsComboValid())
			m_pPartTidList->SetDefaultTids();
		m_pExhibition->SetModel(*m_pPartTidList, charInfo->tidAuxParts, NULL,
			3.0f, 0);
		if (!m_userInfo.info.angelicWings && m_userInfo.info.angelicWingsEffect)
			m_pExhibition->AttachAngelWingFx();
	}
	m_anotherHand.Leave();
}

void FrUserInfoForm::OnCustomProc(float delta)
{
	if (m_anotherHand.IsWorking(0))
	{
		Sleep(20);
	}
	else
	{
		m_anotherHand.Enter();
		if (m_pExhibition)
			m_pExhibition->Process(delta, true);
		m_anotherHand.Leave();
	}
	// 983 008a5787: TAB so no proprio perfil (o teste de canal 0x80 e do 645)
	if (m_userInfo.info.dwUID == CSharedDoc::Instance()->m_myInfo.info.dwUID &&
		g_input->GetDown("TAB", true) && !g_input->Get("LALT", true) &&
		!g_input->Get("RALT", true))
	{
		g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		switch (m_tabType)
		{
		case TAB_GENERAL:
			SetFormA(TAB_SPECIAL);
			break;
		case TAB_SPECIAL:
			SetFormA(TAB_RECORD);
			break;
		case TAB_RECORD:
			SetFormA(TAB_TROPHY);
			break;
		case TAB_TROPHY:
			SetFormA(TAB_GUILD);
			break;
		case TAB_GUILD:
			SetFormA(TAB_GENERAL);
			break;
		}
	}
	if (m_userInfo.userEquip.tidSkin[0])
	{
		for (int i = 0; i < 2; i++)
		{
			float speed = i == 0 ? 138.0f : RB_228F;
			switch (m_skinScrollType[i])
			{
			case 0:
				m_skinScroll[i] = 0.0f;
				break;
			case 1:
				m_skinScroll[i] -= speed * delta * 0.25f;
				break;
			case 2:
				m_skinScroll[i] += speed * delta * 0.25f;
				break;
			}
		}
	}
}

void FrUserInfoForm::SetVisible(FrWnd** const ppWnd, int num, bool bVisible)
{
	for (int i = 0; i < num; ++i)
	{
		if (ppWnd[i])
			ppWnd[i]->SetVisible(bVisible);
	}
}

// 983 008a4620: a aba Geral nao mexe no botao de visita do My Room (o 983 nao tem "Btn_rmrvisitor"); a Special
// esconde sempre os 5 "btn_si"; a de trofeus mostra/esconde o sub-objeto 0x27c inteiro (008999e0, sem testar nulo)
void FrUserInfoForm::SetVisible(eTabType tab, bool bVisible)
{
	switch (tab)
	{
	case TAB_GENERAL:
		SetVisible((FrWnd**)m_pGeneralEdit, 8, bVisible);
		if (m_pLevelArea)
			m_pLevelArea->SetVisible(bVisible);
		if (m_pClubArea)
			m_pClubArea->SetVisible(bVisible);
		if (m_pBackArea[0])
			m_pBackArea[0]->SetVisible(bVisible);
		if (m_pQuestionMark)
			SetDisconnQuestionMark(
				bVisible && IsHideDisconnectRate(&m_userInfo.stat));
		break;
	case TAB_SPECIAL:
		SetVisible((FrWnd**)m_pSpecialEdit, 19, bVisible);
		for (int i = 0; i < 5; ++i)
			m_pMatchHistoryBtn[i]->SetVisible(false);
		m_pSpecListBox->SetVisible(bVisible);
		break;
	case TAB_RECORD:
		for (int i = 0; i < 4; ++i)
			if (m_pRecordBtn[i])
				m_pRecordBtn[i]->SetVisible(bVisible);
		if (bVisible)
		{
			ChangeRecordType();
			return;
		}
		SetVisible((FrWnd**)m_pRecAwardWnd, 6, false);
		if (m_pBackArea[2])
			m_pBackArea[2]->SetVisible(false);
		SetVisible((FrWnd**)m_pRecordEdit, 15, false);
		m_pScoreList->SetVisible(false);
		m_pRecTotalListBox->SetVisible(false);
		if (m_pBackArea[3])
			m_pBackArea[3]->SetVisible(false);
		break;
	case TAB_TROPHY:
	{
		FrWnd* boxes[4] = { m_pNormalTrophyBox, m_pSpecialTrophyBox, m_pGpTrophyBox983[0], m_pGpTrophyBox983[1] };
		FrWnd* nums[4] = { m_pNormalTrophyNum, m_pSpecialTrophyNum, m_pGpTrophyNum983[0], m_pGpTrophyNum983[1] };
		for (int i = 0; i < 4; ++i)
		{
			m_pTrophyBtn[i]->SetVisible(bVisible);
			boxes[i]->SetVisible(bVisible);
			nums[i]->SetVisible(bVisible);
		}
		m_pNoTrophy->SetVisible(bVisible);
		if (m_pBackArea[4])
			m_pBackArea[4]->SetVisible(bVisible);
		if (m_pBackArea[5])
			m_pBackArea[5]->SetVisible(bVisible);
		if (bVisible)
			ChangeTrophyType();
		break;
	}
	case TAB_GUILD:
		SetVisible((FrWnd**)m_pGuildEdit, 3, bVisible);
		m_pMyGuild->SetVisible(bVisible);
		if (m_pBackArea[6])
			m_pBackArea[6]->SetVisible(bVisible);
		if (m_pGuildTrophyList)
			m_pGuildTrophyList->SetVisible(bVisible);
		m_pGuildNoTrophy->SetVisible(
			bVisible && m_pGuildTrophyList->m_itemList.size() == 0);
		break;
	}
}

void FrUserInfoForm::OnCommentInit(int param)
{
	m_pComment = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pComment)
		m_pComment->SetVisible(false);
}

void FrUserInfoForm::OnEditInit_HoverOn(int param)
{
	if (m_pComment)
	{
		FrEdit* pEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
		if (pEdit)
		{
			const char* text = "";
			int len = strlen(text);
			if (len)
			{
				m_pComment->ClearLine();
				m_pComment->AddText(text, false, false);
				WPoint pos(pEdit->GetRect().x, pEdit->GetRect().y);
				m_pComment->MoveWindow(pos);
				m_pComment->SetVisible(true);
			}
		}
	}
}

void FrUserInfoForm::OnEditInit_HoverOff(int param)
{
	if (m_pComment)
		m_pComment->SetVisible(false);
}

void FrUserInfoForm::OnScoreListInit(int param)
{
	static char* s_font[] = { "[font_wind.jpg" };
	m_pScoreList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pScoreList)
	{
		m_pScoreList->SetVisible(false);
		WTITLEFONT font;
		memset(&font, 0, sizeof(font));
		font.filename = s_font;
		font.fontw = 16;
		font.fonth = 16;
		font.numPages = 1;
		font.texw = 128;
		font.texh = 64;
		font.pCharSet = "1234567890ym%-.?/:";
		m_pScoreFont = g_resrcmng->GetTitleFont();
		m_pScoreFont->Create(&font);
	}
}

void FrUserInfoForm::OnScoreListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;

	// 983 008a2b10: pData = &m_mapFlag983[i]; sem a fonte da lista nao desenha
	if (!m_pScoreFont)
		return;
	const sMapFlag983* pFlag = (const sMapFlag983*)pItem->pData;
	sMapStatistics* pMap = (sMapStatistics*)pFlag->v;
	if (!pMap)
		return;
	unsigned char mapId = pMap->bMap;
	unsigned int courseId;
	if (mapId <= 0x7f || mapId == 0xfd)
		courseId = mapId;
	else
		courseId = mapId - 0x80;

	IFF_STRUCT::sCourse* pCourse =
		ItemManager()->FindCourse(courseId | 0x28000000);
	if (pCourse)
	{
		const Bitmap* pCaption = g_pFresh->GetManager()->GetBitmap("CAPTION",
			MakeStr("room_%s", pCourse->c.Icon));
		WRect rect(pItem->pos.x, pItem->pos.y + 7.0f, (float)pCaption->Width(),
			(float)pCaption->Height());
		pDevice->DrawTexture(pCaption, rect,
			pMap->cBestScore != 0x7f ? 0xffffffff : 0x60000000, 0);
	}

	if (pMap->cBestScore != 0x7f)
	{
		if (pFlag->b)
		{
			// 983: recorde feito com o Play Assist
			const Bitmap* pAssist = g_pFresh->GetBitmap("assist_title");
			WRect rect(pItem->pos.x + 125.0f, pItem->pos.y + 38.0f,
				(float)pAssist->Width(), (float)pAssist->Height());
			pDevice->DrawTexture(pAssist, rect, 0xffffffff, 0);
		}
		const char* szScore = MakeStr("%d", (int)pMap->cBestScore);
		// HACK: 130.0f; carddlg.cpp owns __real@43020000 as a function-local static (naked asm), the
		// literal here would emit a competing COMDAT (the volatile read keeps the x87 operand order)
		m_pScoreFont->Print(g_view,
			pItem->pos.x + 130.0f,	// 983/VS2010: literal direto
			pItem->pos.y + 20.0f, szScore, 0, 0xffffffff, NULL);
		if (pMap->tidChar != 0)
		{
			IFF_ITEM_COMMON* pItemCommon =
				ItemManager()->FindCommonItem(pMap->tidChar);
			if (pItemCommon)
			{
				const Bitmap* pBar = g_pFresh->GetManager()->GetBitmap("ITEMS",
					MakeStr("bar_%s", pItemCommon->Icon));
				if (pBar)
				{
					WRect rect(pItem->pos.x + 79.0f, pItem->pos.y + 13.0f,
						(float)pBar->Width(), (float)pBar->Height());
					pDevice->DrawTexture(pBar, rect, 0xffffffff, 0);
				}
			}
		}
		else
		{
			m_pScoreFont->Print(g_view, pItem->pos.x + 88.0f,
				pItem->pos.y + 20.0f, "-", 0, 0xffffffff, NULL);
		}
		m_pScoreFont->Print(g_view, pItem->pos.x + 188.0f, pItem->pos.y + 20.0f,
			MakeStr("%I64d", pMap->i64MaxPang), 0, 0xffffffff, NULL);
	}
	else
	{
		m_pScoreFont->Print(g_view, pItem->pos.x + 88.0f, pItem->pos.y + 20.0f,
			"-", 0, 0xffffffff, NULL);
		m_pScoreFont->Print(g_view, pItem->pos.x + 140.0f, pItem->pos.y + 20.0f,
			"-", 0, 0xffffffff, NULL);
		m_pScoreFont->Print(g_view, pItem->pos.x + 200.0f, pItem->pos.y + 20.0f,
			"-", 0, 0xffffffff, NULL);
	}
}

bool FrUserInfoForm::OnAddFriendFormResult(int result, FrForm* pForm)
{
	m_pAddFriendDlg = NULL;
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		if (AfxGetTask()->GetActor("GolfRule") && !m_bChatMsgOpen)
		{
			AfxGetTask()->GetActor("GolfRule")
				<< MsgObject(NULL, M983(0x62), 0, 0, 0, 0, 0);
		}
	}
	else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		if (AfxGetTask()->GetActor("AvatarChat") && !m_bChatMsgOpen)
		{
			AfxGetTask()->GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x62), 0, 0, 0, 0, 0);
		}
	}
	return true;
}

void FrUserInfoForm::OnTitleSeasonVisible()
{
	m_seasonType = SEASON_CURRENT;
	m_pTitleBtn[0]->SetStatus(FrButton::PRESSED);
	m_pTitleBtn[1]->SetStatus(FrButton::NORMAL);
	SetContent(true);
	SetTrophyStatic();
	SetSpecListBoxItem(SEASON_CURRENT);
}

void FrUserInfoForm::SetRequestFriendData(const char* nick, unsigned long uid)
{
	if (nick && m_pAddFriendDlg)
	{
		m_pAddFriendDlg->SetNick(nick);
		m_pAddFriendDlg->SetFindUID(uid);
	}
}


// 983 RTTI FrAIInfoForm: ficha do rival de IA do Grand Prix (layout "aiinfo"; 0x3480 B no 983). So boneco, barra de carga,
// icone e nome: o registro vem de FindGrandPrixAI983, nao do servidor.
class FrAIInfoForm : public FrForm, public CHandOwner
{
	DECLARE_OBJECT(FrAIInfoForm)

public:
	FrAIInfoForm();
	virtual ~FrAIInfoForm();
	virtual bool Close(eFormRet result, bool bFade);
	void Clear();
	void SetInfo(unsigned short aiId);
	void OnCustomProc(float delta);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float delta);
	virtual void OnAnotherHand(int key, void* param);
	void OnPetInit(int param);
	void OnPetOwnerDraw();
	void OnPetLoadInit(int param);
	void OnNickInit(int param);
	void OnNickOwnerDraw();
	void OnCloseUp();
	void ResetPetLoadGauge();
	void HidePet(bool bWait);
	void ShowPet();

	FrArea* m_pPetArea;
	FrArea* m_pNickArea;
	CExhibition* m_pExhibition;
	CPartTidList* m_pPartTidList;
	FrGaugeBar* m_pPetLoadGauge;
	CAnotherHand m_anotherHand;
	sUserInfo m_userInfo;
	float m_petLoadTime;
	const Bitmap* m_pAiIcon;

	DECLARE_FRESH_MSGMAP()
};

// registro de 00a17cb0: nome em +8, personagem em +0x30, 7 pecas em +0x38 (mesma leitura de dummyplayeractor.cpp)
struct sAiInfoNpc983
{
	char m_00[8];
	char name[0x28];
	unsigned long charIndex;
	int m_34;
	unsigned long parts[7];
};

IMPLEMENT_OBJECT(FrAIInfoForm, FrForm)

BEGIN_FRESH_MSGMAP(FrAIInfoForm, FrForm) // 00ce0850
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrAIInfoForm::OnCloseUp)
ON_FRESH_VI("pet", FRCMD_INIT, FrAIInfoForm::OnPetInit)
ON_FRESH_VV("pet", FRCMD_OWNERDRAW, FrAIInfoForm::OnPetOwnerDraw)
ON_FRESH_VI("pet_load", FRCMD_INIT, FrAIInfoForm::OnPetLoadInit)
ON_FRESH_VI("nick", FRCMD_INIT, FrAIInfoForm::OnNickInit)
ON_FRESH_VV("nick", FRCMD_OWNERDRAW, FrAIInfoForm::OnNickOwnerDraw)
END_FRESH_MSGMAP()

FrAIInfoForm::FrAIInfoForm() // 008a3130
	: m_pPetArea(NULL),
	  m_pNickArea(NULL),
	  m_pExhibition(NULL),
	  m_pPartTidList(NULL),
	  m_pPetLoadGauge(NULL),
	  m_petLoadTime(0.0f),
	  m_pAiIcon(NULL)
{
	memset(&m_userInfo, 0, sizeof(m_userInfo));
	m_userInfo.info.dwGuid = 0xffffffff;
	m_userInfo.info.dwUID = 0xffffffff;
}

FrAIInfoForm::~FrAIInfoForm() // 008a3220
{
	Clear();
}

void FrAIInfoForm::Clear() // 0089f770
{
	if (CUserInfo::Instance())
		CUserInfo::Instance()->m_pAiInfoDlg = NULL;
	g_resrcmng->UnlockAllByThread(m_anotherHand.GetThreadID());
	m_anotherHand.BreakWorkingThread();
	HidePet(true);
}

bool FrAIInfoForm::Close(eFormRet result, bool bFade) // 0089f700
{
	if (m_anotherHand.IsWorking(0))
		return false;
	return FrForm::Close(result, bFade);
}

bool FrAIInfoForm::OnInit() // 0089a0e0
{
	m_petLoadTime = 0.0f;
	return true;
}

void FrAIInfoForm::OnProc(const float delta) // 0089fb30
{
	if (m_petLoadTime < 3.0f)
	{
		m_petLoadTime += delta;
		if (m_petLoadTime >= 3.0f || !m_anotherHand.IsWorking(0))
		{
			m_pPetLoadGauge->SetVisible(false);
			m_petLoadTime = 3.0f;
		}
	}
}

void FrAIInfoForm::OnAnotherHand(int key, void* param) // 008a32a0
{
	if (key == 0)
		ShowPet();
}

void FrAIInfoForm::OnCustomProc(float delta) // 0089fba0
{
	if (m_anotherHand.IsWorking(0))
	{
		Sleep(20);
		return;
	}
	m_anotherHand.Enter();
	if (m_pExhibition)
		m_pExhibition->Process(delta, true);
	m_anotherHand.Leave();
}

void FrAIInfoForm::OnCloseUp() // 0089a0d0
{
	Close(FrNONE, true);
}

void FrAIInfoForm::OnPetInit(int param) // 0089f7b0
{
	m_pPetArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAIInfoForm::OnPetOwnerDraw() // 0089f820
{
	g_view->Clear(0, 8);
	if ((m_pPetLoadGauge && m_pPetLoadGauge->IsVisible()) || m_anotherHand.IsWorking(0))
		return;
	m_anotherHand.Enter();
	if (m_pExhibition)
		m_pExhibition->Display(0.0f, 0.0f);
	m_anotherHand.Leave();
}

void FrAIInfoForm::ResetPetLoadGauge() // 0089a120
{
	if (m_pPetLoadGauge)
	{
		m_pPetLoadGauge->SetRange(0, 120, 0);
		m_pPetLoadGauge->SetPos(0);
		m_pPetLoadGauge->SetDestPos(120, false);
		m_pPetLoadGauge->SetVisible(true);
	}
}

void FrAIInfoForm::OnPetLoadInit(int param) // 0089f8e0
{
	m_pPetLoadGauge = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	ResetPetLoadGauge();
}

void FrAIInfoForm::OnNickInit(int param) // 0089f960
{
	m_pNickArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pAiIcon = g_pFresh->RegisterBitmap("i_aiplayer");
}

void FrAIInfoForm::OnNickOwnerDraw() // 0089f9e0
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	WRect area = m_pNickArea->GetRect();
	pDevice->DrawTexture(m_pAiIcon, WRect(area.x, area.y, (float)m_pAiIcon->Width(), (float)m_pAiIcon->Height()),
		0xffffffff, 0);
	if (!CProjectG::Instance()->HidePrivacy())
	{
		pDevice->SetTextColor(0xff000000, 0xffffffff);
		pDevice->SetTextStyle(1);
		float width = pDevice->GetTextExtendOld983(m_userInfo.info.sNick);
		WPoint pos(area.x + area.w * 0.5f - width * 0.5f, area.y + area.h * 0.5f - pDevice->GetFontHeight() * 0.5f);
		g_pFresh->GetManager()->PrintText(pos, 0, m_userInfo.info.sNick, -1.0f, 0xffffffff);
	}
}

void FrAIInfoForm::HidePet(bool bWait) // 0089a170
{
	if (bWait)
		m_anotherHand.WaitUntilWorkDone(9999);
	delete m_pExhibition;
	m_pExhibition = NULL;
	delete m_pPartTidList;
	m_pPartTidList = NULL;
}

void FrAIInfoForm::ShowPet() // 0089fc50
{
	m_anotherHand.Enter();
	HidePet(false);
	if (m_pPetArea)
	{
		WRect rect = m_pPetArea->GetRect();
		m_pExhibition = new CExhibitionByHardwareTnL(rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f, 0.35f, true);
		m_pExhibition->SetDistance(18.75f, true, false);
		m_pExhibition->SetArea(rect.w, rect.h);
		m_pPartTidList = new CPartTidList;
		sCharacterInfo* charInfo = &m_userInfo.charInfo;
		if (!m_pPartTidList->SetTids(charInfo, 0xff) || !m_pPartTidList->IsComboValid())
			m_pPartTidList->SetDefaultTids();
		m_pExhibition->SetModel(*m_pPartTidList, charInfo->tidAuxParts, NULL, 3.0f, 0);
		if (!m_userInfo.info.angelicWings && m_userInfo.info.angelicWingsEffect)
			m_pExhibition->AttachAngelWingFx();
	}
	m_anotherHand.Leave();
}

C_ASSERT_VC10(offsetof(IFF_STRUCT::sPart, PosMask) == 0xec && offsetof(IFF_STRUCT::sPart, SubPart) == 0x220);
C_ASSERT_VC10(sizeof(sUserInfo) == 0x31c6 && offsetof(sUserInfo, info.sNick) == 0x18 && offsetof(sUserInfo, charInfo) == 0x2f52);
void FrAIInfoForm::SetInfo(unsigned short aiId) // 008a32c0
{
	memset(&m_userInfo, 0, sizeof(m_userInfo)); // 0089a1d0
	m_userInfo.info.dwGuid = 0xffffffff;
	m_userInfo.info.dwUID = 0xffffffff;
	ResetPetLoadGauge();
	m_petLoadTime = 0.0f;
	sAiInfoNpc983* pNpc = (sAiInfoNpc983*)ItemManager()->FindGrandPrixAI983(aiId);
	if (!pNpc)
		return;
	sCharacterInfo& info = m_userInfo.charInfo;
	info.tid = pNpc->charIndex | 0x4000000;
	strncpy(m_userInfo.info.sNick, pNpc->name, 0x16);
	ItemManager()->GetDefCombo(info.tid, info.tidParts);
	if (!ItemManager()->FindChar(info.tid))
		return;
	for (int i = 0; i < 7; ++i)
	{
		unsigned long tid = pNpc->parts[i];
		unsigned long pos = (tid >> 13) & 0x1f;
		if (!tid || pos >= 0x19)
			continue;
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(tid);
		// tira do combo o que ocupa a mesma posicao da peca nova e entram as sub-pecas dela
		for (int slot = 0; slot < 0x18; ++slot)
		{
			if (!info.tidParts[slot])
				continue;
			IFF_STRUCT::sPart* pCur = ItemManager()->FindPart(info.tidParts[slot]);
			if (!(pPart->PosMask & pCur->PosMask))
				continue;
			unsigned long curPos = (pCur->c.TypeId >> 13) & 0x1f;
			if (curPos == ((pPart->c.TypeId >> 13) & 0x1f))
				info.tidParts[slot] = tid;
			else
				info.tidParts[curPos] = 0;
			if (pPart->SubPart[0])
				info.tidParts[(pPart->SubPart[0] >> 13) & 0x1f] = pPart->SubPart[0];
			if (pPart->SubPart[1])
				info.tidParts[(pPart->SubPart[1] >> 13) & 0x1f] = pPart->SubPart[1];
		}
		info.tidParts[pos] = tid;
	}
	m_anotherHand.Work(this, 0, NULL, true, -1);
}

CUserInfo::CUserInfo()
	: m_pDlg(NULL),
	  m_pAiInfoDlg(NULL),
	  m_guid(0xffffffff),
	  m_uid(0xffffffff),
	  m_bCheckTime(false),
	  m_bRequest(false)
{
}

CUserInfo::~CUserInfo()
{
}

void CUserInfo::Clear()
{
	if (m_pDlg)
		m_pDlg->Clear();
	if (m_pAiInfoDlg)
		((FrAIInfoForm*)m_pAiInfoDlg)->Clear();
}

bool CUserInfo::IsVisible()
{
	// 983 008a3100: a ficha de IA so conta no Grand Prix
	if ((unsigned)(CSharedDoc::Instance()->m_roomInfo.realGameType - 0x14) <= 1)
		return m_pDlg || m_pAiInfoDlg;
	return m_pDlg ? true : false;
}

void CUserInfo::SetInfo(unsigned long uid, unsigned long guid, bool bCheckTime,
	bool bOption, bool bMsnControl, bool bControlOff, std::string nickname,
	unsigned char aiType, unsigned short aiId)
{
	// 983 008a9ae0: rival de IA do Grand Prix abre a ficha "aiinfo"
	if ((unsigned)(CSharedDoc::Instance()->m_roomInfo.realGameType - 0x14) <= 1 && aiType)
	{
		if (!m_pAiInfoDlg)
		{
			m_pAiInfoDlg = CreateForm<FrAIInfoForm>(g_pFresh->GetManager(), this, "aiinfo", NULL);
			if (!m_pAiInfoDlg)
				return;
			if (m_pDlg)
				m_pDlg->Close(FrCANCEL, true);
			m_pAiInfoDlg->Open((FRESH_PFN_RESULT)&CUserInfo::OnAiInfoFormResult, 1);
			m_pAiInfoDlg->EnableDrag(false);
			m_pAiInfoDlg->SetTopmost(bControlOff);
			m_pAiInfoDlg->FindNextTopFocus(false);
		}
		((FrAIInfoForm*)m_pAiInfoDlg)->SetInfo(aiId);
		return;
	}
	if (OnlinePlay())
	{
		if (uid == 0xffffffff)
			return;
		if (!m_pDlg)
		{
			m_pDlg = CreateForm<FrUserInfoForm>(g_pFresh->GetManager(), this,
				"userinfo", NULL);
			if (!m_pDlg)
				return;
			if (m_pAiInfoDlg)
				m_pAiInfoDlg->Close(FrCANCEL, true);
			m_pDlg->Open((FRESH_PFN_RESULT)&CUserInfo::OnUserInfoFormResult, 1);
			m_pDlg->EnableDrag(false);
			m_pDlg->SetTopmost(bControlOff);
			m_pDlg->SetMsnControl(bMsnControl);
			m_pDlg->SetControlOff(bControlOff);
			m_pDlg->FindNextTopFocus(false);
			m_nickname = nickname;
		}

		m_guid = guid;
		m_uid = uid;
		m_bRequest = true;
		m_bCheckTime = bCheckTime;

		std::map<unsigned long, sUserInfoTime>::iterator it;
		if (!CSharedDoc::Instance()->FindUserInfoTimeUID(uid, it) ||
			(bCheckTime && g_CurrentTime - it->second.dwUpdate >= 60000))
		{
			if (bOption && nickname.size())
			{
				WSendPacket packet((enumClientPacket)0x2f);
				packet.Encode4(uid);
				packet.Encode1(5);
				packet.Send(TO_GAME);
			}
			return;
		}

		unsigned long myUID = CSharedDoc::Instance()->m_myInfo.info.dwUID;
		if (myUID == m_uid)
		{
			std::map<unsigned int, sCharacterInfo>::iterator ci =
				CSharedDoc::Instance()->m_charMap.find(
					CSharedDoc::Instance()->m_myInfo.userEquip.guidChar);
			if (ci != CSharedDoc::Instance()->m_charMap.end())
				it->second.info.charInfo = ci->second;
			if (!IS_KINDOF(CGolfTask, AfxGetTask()))
				it->second.info.stat.i64Pang =
					CSharedDoc::Instance()->m_myInfo.stat.i64Pang;
			if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
			{
				it->second.info.stat.dwExp =
					CSharedDoc::Instance()->m_myInfo.stat.dwExp;
				it->second.info.stat.Level =
					CSharedDoc::Instance()->m_myInfo.stat.Level;
			}
			it->second.info.userEquip =
				CSharedDoc::Instance()->m_myInfo.userEquip;
		}
		if (bMsnControl)
			it->second.info.roomIndex = 0xffff;
		m_pDlg->SetInfo(&it->second);
		m_bRequest = false;
	}
}

void CUserInfo::Refresh()
{
	if (m_pDlg)
		m_pDlg->SetContent(true);
}

void CUserInfo::Process(float delta)
{
	if (m_bRequest)
	{
		SetInfo(m_uid, m_guid, m_bCheckTime, false, false, false, "");
		if (m_bRequest &&
			WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN))
			SetInfo(m_uid, m_guid, m_bCheckTime, false, true, false, "");
	}
	if (m_pDlg)
		m_pDlg->OnCustomProc(delta);
	if (m_pAiInfoDlg)
		((FrAIInfoForm*)m_pAiInfoDlg)->OnCustomProc(delta);
}

bool CUserInfo::OnUserInfoFormResult(int result, FrForm* pForm)
{
	m_guid = 0xffffffff;
	m_uid = 0xffffffff;
	m_bRequest = false;
	m_pDlg = NULL;
	return true;
}

bool CUserInfo::OnAiInfoFormResult(int result, FrForm* pForm) // 00899fb0
{
	m_pAiInfoDlg = NULL;
	return true;
}

void CUserInfo::Close()
{
	if (m_pDlg)
		m_pDlg->Close(FrNONE, true);
	if (m_pAiInfoDlg)
		m_pAiInfoDlg->Close(FrNONE, true);
}

bool CUserInfo::IsRecvedData(unsigned char season)
{
	if (!m_pDlg)
		return false;
	unsigned long bit = 0;
	if (season == 0)
		bit = 2;
	else if (season == 5)
		bit = 4;
	m_pDlg->RecvedSeasonData(bit);
	if (m_pDlg->IsTotalTab())
	{
		if (!m_pDlg->IsRecvSeason(2))
		{
			m_pDlg->RequestSeasonData(0);
			return false;
		}
		if (!m_pDlg->IsRecvSeason(4))
		{
			m_pDlg->RequestSeasonData(5);
			return false;
		}
	}
	return true;
}

void FrUserInfoForm::OnRecAwardBackInit(int param)
{
	m_pBackArea[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUserInfoForm::OnGhostInit(int param)
{
	m_pGhostBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pGhostBtn->Enable(false);
	if (!(CSharedDoc::Instance()->m_curChannel.Type & 0x800))
	{
		if (IS_KINDOF(CLobbyTask, AfxGetTask()) &&
			_stricmp("ROOMLIST", g_pFresh->GetManager()->GetLayoutID()) == 0 &&
			CSharedDoc::Instance()->m_myInfo.roomIndex == 0xffff)
		{
			m_pGhostBtn->Enable(true);
		}
	}
}

void FrUserInfoForm::OnGhostdUp()
{
	FrForm* form =
		CreateForm<FrForm>(g_pFresh->GetManager(), NULL, "notify", NULL);
	form->SetTimeLimit(5.0f);
	form->SetMessage(
		K2L_Compatibility("\274\255\272\361\275\272 \301\330\272\361 \301\337\300\324\264\317\264\331."),
		false);
	form->Open(NULL, 3);
}

void FrUserInfoForm::OnBtn_RmrVisitorInit(int param)
{
	m_pRmrVisitorBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUserInfoForm::OnBtn_RmrVisitorBtnUp()
{
	m_anotherHand.WaitUntilWorkDone(9999);
	CSharedDoc::Instance()->m_replayTargetUID = m_userInfo.info.dwUID;
	CSharedDoc::Instance()->m_rmrMode =
		CSharedDoc::Instance()->m_myInfo.info.dwUID !=
			CSharedDoc::Instance()->m_replayTargetUID
		? 5
		: 1;
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x222), 0, 0, 0, 0, 0));
}

void FrUserInfoForm::RequestSeasonData(unsigned char season)
{
	if (!m_requestSeason)
	{
		WSendPacket packet((enumClientPacket)0x2f);
		packet.Encode4(m_userInfo.info.dwUID);
		packet.Encode1(season);
		packet.Send(TO_GAME);
		m_requestSeason = 1;
	}
}

inline void FrUserInfoForm::RecvedSeasonData(unsigned long season)
{
	m_recvSeason |= season;
	m_requestSeason = 0;
}

int FrUserInfoForm::IsTotalTab()
{
	return m_seasonType == SEASON_TOTAL;
}
