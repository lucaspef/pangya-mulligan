#define CONTENTSDOC_DEFER_GIMMICK_CONTAINER
#include "minatl.h"
#include "contentsdoc.h"
#include "../shared/globalgamedefine.h"
typedef char rb_ob_m0[sizeof(std::map<int, const char*>)];
typedef char rb_ob_m1[sizeof(std::map<int, std::list<GUILD_USER_LIST> >)];
typedef char rb_ob_m2[sizeof(std::map<int, std::list<GUILD_LIST> >)];
#include "guildinfo.h"
#include "gimmickcontainer.h"
#include "guildactor.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "packet.h"
#include "packets/tutorialcontainer.h"	// PKT (frente C)
#include "../shared/LuaSystem/luascriptmanager.h"
#include "../shared/LuaSystem/luatablehelper.h"
#include "../shared/s5/sharedutilities.h"
#include <algorithm>
#include <string.h>
// HACK: template order only. The sizeof typedefs (rb_ob_*) instantiate classes
// where the original did; rb_ob_lp<int>::f is never emitted, but its body makes
// lua_tinker::pop<int> referenced one round earlier, as in the original.
template <class T>
inline T rb_ob_h(lua_State* L)
{
	return lua_tinker::pop<T>(L);
}
template <class T>
struct rb_ob_lp
{
	virtual T f(lua_State* L) { return rb_ob_h<T>(L); }
};
typedef char rb_ob_lps[sizeof(rb_ob_lp<int>)];
typedef char rb_ob_inst0[sizeof(std::list<GUILD_USER_LIST>)];
typedef char rb_ob_inst1[sizeof(std::list<GUILD_LIST>)];

namespace _guild
{
	// guild UI message ids (enumerator names not in the PDB)
	enum eGuildMsgCode
	{
		GUILD_MSG_FIRST = 1000,
	};
}

// guild server error codes (enumerator names not in the PDB)
enum IngameGuildErrorCode
{
	INGAME_GUILD_ERROR_FIRST = 54001,
};

void CGiftBoxEvent::SetOpenedBoxTID(unsigned long tid)
{
	// HACK: keeps the unreferenced wmath.h constant emitted, as in the original
	(void)&g_EPSILON;
	m_openedBoxTID = tid;
}

unsigned long CGiftBoxEvent::GetOpenedBoxTID()
{
	return m_openedBoxTID;
}

bool CGiftBoxEvent::Initialize()
{
	m_openedBoxTID = 0;
	return true;
}

bool CGiftBoxEvent::Release()
{
	return true;
}

// PKT (frente C, lote 51): CTutorialContainer do 983 (documento 0xa4; include/packets/tutorialcontainer.h)
// 009079a0
bool CTutorialContainer983::Initialize()
{
	Reset();
	return true;
}

// 009079b0
bool CTutorialContainer983::Release()
{
	return true;
}

// 009079c0
bool CTutorialContainer983::Reset()
{
	m_category = -1;
	m_mode = -1;
	m_index = -1;
	m_10 = -1;
	m_pendingQuest = 0xff;
	m_bSendResult = false;
	return true;
}

// 009079e0: o caso M983(0x150) do CTutorialMain::HandleMsg do 645 sem o m_Tutorials[index].enable
void CTutorialContainer983::SendTutorialResult983(int mode, unsigned long flags, int result, bool last)
{
	if (result == 1 && m_bSendResult)
	{
		m_bSendResult = false;
		WSendPacket packet((enumClientPacket)0xae);
		packet.Encode1((unsigned char)last);
		packet.Encode1((unsigned char)m_category);
		packet.Encode4(flags);
		packet.Send((eSendTo)0);
	}
}

// 00907a80: categoria, modo e indice escolhidos (o CTutorialMain::StartSelectedTutorial do 983 testa antes de ir ao jogo)
bool CTutorialContainer983::IsValid983() const
{
	if (m_category < 0 || m_mode < 0 || m_index < 0)
		return false;
	return true;
}

void CWorldTourEvent::SetMapFlag(unsigned long flag)
{
	m_mapFlag = flag;
}

void CWorldTourEvent::SetGotGift(bool bGot)
{
	m_bGotGift = bGot;
}

void CWorldTourEvent::SetCanGift(bool bCan)
{
	m_bCanGift = bCan;
}

unsigned long CWorldTourEvent::GetMapFlag() const
{
	return m_mapFlag;
}

bool CWorldTourEvent::GetGotGift() const
{
	return m_bGotGift;
}

bool CWorldTourEvent::GetCanGift() const
{
	return m_bCanGift;
}

bool CWorldTourEvent::Initialize()
{
	m_mapFlag = 0;
	m_bGotGift = false;
	m_bCanGift = false;
	return true;
}

bool CWorldTourEvent::Release()
{
	return Initialize();
}

void CHalloweenEvent::SetMapFlag(unsigned long flag)
{
	m_mapFlag = flag;
}

void CHalloweenEvent::SetGiftCnt(unsigned long cnt)
{
	m_giftCnt = cnt;
}

void CHalloweenEvent::SetShowGiftDlg(bool bShow)
{
	m_bShowGiftDlg = bShow;
}

void CHalloweenEvent::SetEventState(bool bState)
{
	m_bEventState = bState;
}

unsigned long CHalloweenEvent::GetMapFlag() const
{
	return m_mapFlag;
}

unsigned long CHalloweenEvent::GetGiftCnt() const
{
	return m_giftCnt;
}

bool CHalloweenEvent::GetShowGiftDlg() const
{
	return m_bShowGiftDlg;
}

bool CHalloweenEvent::GetEventState() const
{
	return m_bEventState;
}

bool CHalloweenEvent::Initialize()
{
	m_mapFlag = 0;
	m_giftCnt = 0;
	m_bShowGiftDlg = false;
	m_bEventState = false;
	return true;
}

bool CHalloweenEvent::Release()
{
	return Initialize();
}

void CEscapeBeginnerEvent::SetFlag(bool bFlag)
{
	m_bFlag = bFlag;
}

bool CEscapeBeginnerEvent::GetFlag() const
{
	return m_bFlag;
}

bool CEscapeBeginnerEvent::Initialize()
{
	m_bFlag = false;
	return true;
}

bool CEscapeBeginnerEvent::Release()
{
	return Initialize();
}

void CPassingTitleChk::SetFlag(bool bFlag)
{
	m_bFlag = bFlag;
}

bool CPassingTitleChk::GetFlag() const
{
	return m_bFlag;
}

bool CPassingTitleChk::Initialize()
{
	m_bFlag = false;
	return true;
}

bool CPassingTitleChk::Release()
{
	return Initialize();
}

void CChristmasEvent::SetFlag(bool bFlag)
{
	m_bFlag = bFlag;
}

bool CChristmasEvent::GetFlag() const
{
	return m_bFlag;
}

bool CChristmasEvent::Initialize()
{
	m_bFlag = true;
	return true;
}

bool CChristmasEvent::Release()
{
	return Initialize();
}

void CTicketExchange::SetFlag(bool bFlag)
{
	m_bFlag = bFlag;
}

bool CTicketExchange::GetFlag() const
{
	return m_bFlag;
}

bool CTicketExchange::Initialize()
{
	m_bFlag = true;
	return true;
}

bool CTicketExchange::Release()
{
	return Initialize();
}

bool CMissionEvent::Initialize()
{
	m_eventFlag = 0;
	m_bDayGiftFlag = false;
	m_bTermGiftFlag = false;
	return true;
}

bool CMissionEvent::Release()
{
	return true;
}

void CMissionEvent::SetEventFlag(unsigned int flag)
{
	m_eventFlag = flag;
}

unsigned int CMissionEvent::GetEventFlag() const
{
	return m_eventFlag;
}

int CMissionEvent::GetMissionField(int index) const
{
	static int s_missionFieldBit[16] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
		1024, 2048, 4096, 8192, 16384, 32768 };

	if (index < 12)
		return s_missionFieldBit[index];
	return s_missionFieldBit[index + 1];
}

bool CPeriodContents::Initialize()
{
	return true;
}

bool CPeriodContents::Release()
{
	return true;
}

void CPeriodContents::IsUsable(sPeriodUnit& unit)
{
	SYSTEMTIME& now = Doc()->GetServerTime();
	if (unit.startTime.wYear <= now.wYear &&
		unit.startTime.wMonth <= now.wMonth &&
		unit.startTime.wDay <= now.wDay && unit.startTime.wHour <= now.wHour)
		unit.bUsable = true;
	if (unit.bNoEndTime != true)
		unit.bUsable =
			ItemManager()->IsBetween(&unit.startTime, &unit.endTime, now) != 0;
}

bool CMissionEvent::IsCompleteMission(int index)
{
	return m_missionInfo[index].complete;
}

bool CMissionEvent::IsCompleteAllDayMission()
{
	for (int i = 0; i < 2; i++)
	{
		if (!m_missionInfo[i].complete)
			return false;
	}
	return true;
}

bool CMissionEvent::IsCompleteAllTermMission()
{
	for (int i = 2; i < 7; i++)
	{
		if (!m_missionInfo[i].complete)
			return false;
	}
	return true;
}

bool CMissionEvent::IsCompleteCourse(int course)
{
	return (GetCondition(6) & GetMissionField(course)) ? true : false;
}

typedef char rb_ob_p0[sizeof(
	std::pair<std::map<int, CPeriodContents::sPeriodUnit>::iterator, bool>)];
bool CPeriodContents::IsUsableContents(int id)
{
	std::map<int, sPeriodUnit>::const_iterator it = m_contentsMap.find(id);
	if (it == m_contentsMap.end())
		return false;
	return (*it).second.bUsable;
}

void CPeriodContents::CheckAllContents()
{
	for (std::map<int, sPeriodUnit>::iterator it = m_contentsMap.begin();
		it != m_contentsMap.end(); ++it)
		IsUsable((*it).second);
}

bool CPeriodContents::CheckContent(int id)
{
	std::map<int, sPeriodUnit>::iterator it = m_contentsMap.find(id);
	if (it == m_contentsMap.end())
		return false;
	sPeriodUnit& unit = (*it).second;
	IsUsable(unit);
	return unit.bUsable;
}

void CPeriodContents::AddUsableContents(int id, const SYSTEMTIME* pStart,
	const SYSTEMTIME* pEnd, bool bNoEnd)
{
	if (id < 4)
	{
		sPeriodUnit unit;
		unit.startTime = *pStart;
		if (!bNoEnd)
			unit.endTime = *pEnd;
		unit.bNoEndTime = bNoEnd;
		unit.bUsable = false;
		m_contentsMap.insert(std::make_pair(id, unit));
	}
}

void CPeriodContents::Set_PreSettingContentsDate()
{
	sPeriodUnit unit;
	SYSTEMTIME start = S5::MakeSysTime(2011, 4, 14, 8);
	SYSTEMTIME end = S5::MakeSysTime(2011, 5, 3, 8);
	unit.startTime = start;
	unit.endTime = end;
	unit.bNoEndTime = false;
	unit.bUsable = false;
	m_contentsMap.insert(std::make_pair(3, unit));
	CheckAllContents();
}

void _guild::ExchangeQuotation(char* text)
{
	std::string str(text);
	str_Replace(str, "`", "'");
	strcpy(text, str.c_str());
}

void _guild::sExchangerQuota::operator()(GUILD_USER_LIST& user)
{
	ExchangeQuotation(user.message);
}

void _guild::CGuildInfo::SetMyGuildInfo(GUILD_INFO& info)
{
	m_myGuildInfo = info;
	ExchangeQuotation(m_myGuildInfo.introduce);
	ExchangeQuotation(m_myGuildInfo.notice);
}

void _guild::CGuildInfo::SetMyGuildUserInfo(GUILD_USER_INFO& info)
{
	static_cast<GUILD_USER_INFO&>(m_myGuildInfo) = info;
	ExchangeQuotation(m_myGuildInfo.introduce);
	ExchangeQuotation(m_myGuildInfo.notice);
}

void _guild::CGuildInfo::SetGuildEmblem(GUILD_USER_INFO* pInfo)
{
	strncpy(m_myGuildInfo.guildMark, pInfo->guildMark, 11);
}

void _guild::CGuildInfo::SetGuildName(GUILD_INFO* pInfo)
{
	strncpy(m_myGuildInfo.guildName, pInfo->guildName, 20);
}

void _guild::CGuildInfo::SetGuildIntroduce(const std::string& introduce)
{
	strcpy(m_myGuildInfo.introduce, introduce.c_str());
}

void _guild::CGuildInfo::SetGuildNotice(const std::string& notice)
{
	strcpy(m_myGuildInfo.notice, notice.c_str());
}

const GUILD_INFO& _guild::CGuildInfo::GetMyGuildInfo() const
{
	return m_myGuildInfo;
}

bool _guild::CGuildInfo::IsGuildMember() const
{
	if ((int)m_myGuildInfo.classIdx > 0 && (int)m_myGuildInfo.classIdx <= 3)
		return true;
	return false;
}

bool _guild::CGuildInfo::IsGuildMember(GUILD_CLASS_IDX classIdx)
{
	if ((int)classIdx > 0 && (int)classIdx <= 3)
		return true;
	return false;
}

bool _guild::CGuildInfo::IsManager() const
{
	if ((int)m_myGuildInfo.classIdx > 0 && (int)m_myGuildInfo.classIdx <= 2)
		return true;
	return false;
}

bool _guild::CGuildInfo::IsManager(GUILD_CLASS_IDX classIdx)
{
	if ((int)classIdx > 0 && (int)classIdx <= 2)
		return true;
	return false;
}

int _guild::CGuildInfo::GetMaxMemberPage()
{
	if (GetAllMemberCount() == 0)
		return 1;
	int pages = GetAllMemberCount() / 15;
	if (GetAllMemberCount() % 15 != 0)
		pages++;
	return pages;
}

int _guild::CGuildInfo::GetMaxSearchPage()
{
	if (GetAllGuildCount() == 0)
		return 1;
	int pages = GetAllGuildCount() / 15;
	if (GetAllGuildCount() % 15 != 0)
		pages++;
	return pages;
}

typedef char
	rb_ob_p1[sizeof(std::pair<std::map<int, const char*>::iterator, bool>)];
typedef char rb_ob_inst2[sizeof(std::map<int, const char*>::const_iterator)];

void _guild::CGuildInfo::ClearMemberPage()
{
	for (std::map<int, std::list<GUILD_USER_LIST> >::iterator it =
			 m_memberPageMap.begin();
		it != m_memberPageMap.end(); ++it)
		(*it).second.clear();
}

void _guild::CGuildInfo::ClearMemberList(int page)
{
	std::map<int, std::list<GUILD_USER_LIST> >::iterator it =
		m_memberPageMap.find(page);
	if (it != m_memberPageMap.end())
		(*it).second.clear();
}

inline void rb_ob_d0()
{
	((std::pair<int, std::list<GUILD_USER_LIST> >*)0)->~pair();
	((std::pair<const int, std::list<GUILD_USER_LIST> >*)0)->~pair();
}

typedef char rb_ob_p2[sizeof(
	std::pair<std::map<int, std::list<GUILD_USER_LIST> >::iterator, bool>)];
typedef char rb_ob_c0[sizeof(std::list<GUILD_USER_LIST>::iterator)];
void _guild::CGuildInfo::ClearSearchPage()
{
	for (std::map<int, std::list<GUILD_LIST> >::iterator it =
			 m_searchPageMap.begin();
		it != m_searchPageMap.end(); ++it)
		(*it).second.clear();
}

void _guild::CGuildInfo::ClearSearchList(int page)
{
	std::map<int, std::list<GUILD_LIST> >::iterator it =
		m_searchPageMap.find(page);
	if (it != m_searchPageMap.end())
		(*it).second.clear();
}

inline void rb_ob_d1()
{
	((std::pair<int, std::list<GUILD_LIST> >*)0)->~pair();
	((std::pair<const int, std::list<GUILD_LIST> >*)0)->~pair();
}

void _guild::CGuildInfo::ClearHistoryList()
{
	if (!m_historyList.empty())
		m_historyList.clear();
}

const char* _guild::CGuildInfo::GuildMsg(int id)
{
	std::map<int, const char*>::iterator it = m_msgMap.find(id);
	if (it == m_msgMap.end())
		return NULL;
	return (*it).second;
}

const std::list<GUILD_USER_LIST>* _guild::CGuildInfo::GetMemberList(int page)
{
	std::map<int, std::list<GUILD_USER_LIST> >::iterator it =
		m_memberPageMap.find(page);
	if (it == m_memberPageMap.end())
		return NULL;
	return &(*it).second;
}

const std::list<GUILD_LIST>* _guild::CGuildInfo::GetSearchList(int page)
{
	std::map<int, std::list<GUILD_LIST> >::iterator it =
		m_searchPageMap.find(page);
	if (it == m_searchPageMap.end())
		return NULL;
	return &(*it).second;
}

bool _guild::CGuildInfo::Initialize()
{
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1000, "길드명을 입력해 주세요."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1001, "길드를 소개해 주세요."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1005, "길드를 생성합니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1006, "길드마크를 등록 합니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1007, "길드명을 변경합니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1008,
		"승인(승급)|강등|길드원 추방|길드장 위임|정보보기"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1009,
		"승인(승급)|길드원 추방|정보보기"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1010, "정보보기"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1011,
		"더 이상 승급시킬 수 없습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1012,
		"더 이상 강등시킬 수 없습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1036,
		"길드장을 위임할 수 없는 등급입니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1037, "검색어를 입력해 주세요."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1038,
		"가입할 길드를 선택해 주세요."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1013,
		"이미 길드에 가입되어 있습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1014,
		"가입 신청 글을 작성해 주세요."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1015,
		"가입 신청을 취소합니다. 진행하시겠습니까?"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1016,
		"가입 신청이 취소되었습니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1017, "강제 탈퇴시켰습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1018,
		"가입 신청을 거부하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1019,
		"가입 신청을 승인하였습니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1020, "가입 신청하였습니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1021, "길드를 탈퇴하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1022,
		"길드를 탈퇴할 수 없는 등급입니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1047,
		"길드를 탈퇴하시겠습니까?"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1024,
		"길드를 폐쇄합니다. (길드를 폐쇄하면 24시간 이내에 새로 길드를 만들 수 없습니다.)"));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1025, "길드를 폐쇄하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1026,
		"길드 소개를 변경하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1027,
		"길드 공지를 변경하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1028,
		"길드 이름을 다시 확인해 주세요."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1029,
		"사용 가능한 길드명입니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1030,
		"개인 메시지를 변경하였습니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1031, "길드가 생성되었습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1032,
		"선택한 부길드장에게 길드장을 위임하시겠습니까?"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1033,
		"%s 권한을 위임하였습니다.\n%s이 되었습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1034,
		"%s 권한으로 변경하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1061, "가입신청"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1062, "가입취소"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1063, "가입승인"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1064, "가입거절"));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1065, "길드원등급변경"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1066, "길드원 추방"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1067, "길드탈퇴"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1068, "길드개설"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1069, "길드폐쇄"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1070, "길드장 위임"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1071, "길드장 임명"));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1072, "부길드장 임명"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1073, "길드원 승인"));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1074, "길드마크블럭"));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1075, "길드마크블럭해제"));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1076, "GM에의한추방"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1077, "길드장"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1078, "부길드장"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1079, "길드원"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1080, "가입대기자"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1081, "승인(승급)"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1082, "강등"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1083, "길드원 추방"));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1084, "정보보기"));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1035, "검색된 길드가 없습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1039,
		"길드명이 변경되었습니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1040, "가입한 길드가 없습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1048,
		"%s 님이 길드에 가입하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1049,
		"%s 님이 길드를 탈퇴하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1050,
		"등록된 길드마크가 없습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1051,
		"가로 22, 세로 20 크기가 아닙니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1052, "png 파일이 아닙니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1053,
		"길드 마크 업로드에 성공하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1054,
		"길드 마크 업로드에 실패하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1056,
		"파일 읽기에 실패하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1057,
		"파일 찾기에 실패하였습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1058,
		"파일 크기가 너무 큽니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1059,
		"32비트 png 파일만 등록이 가능합니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1041,
		"\\c0xffff0000\\c영문 %d - %d 글자\\c0xff000000\\c 또는 \\c0xffff0000\\c한글 %d - %d글자\\c0xff000000\\c의 길이를 가져야 합니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1042,
		"\\c0xffff0000\\c영문 %d글자\\c0xff000000\\c 또는 \\c0xffff0000\\c한글 %d글자\\c0xff000000\\c이상 입력하여야 합니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1043,
		"\\c0xffff0000\\c공백문자\\c0xff000000\\c를 포함할 수 없습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1044,
		"사용할 수 없는 문자를 포함하고 있습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1045,
		"사용할 수 없는 단어를 포함하고 있습니다."));
	m_msgMap.insert(
		std::make_pair((_guild::eGuildMsgCode)1046, "입력된 글이 없습니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1055,
		"길드 마크 등록이 금지된 상태입니다. 고객 지원 센터로 문의 바랍니다."));
	m_msgMap.insert(std::make_pair((_guild::eGuildMsgCode)1060,
		"내용이 바뀌지 않았습니다."));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)0, "정의되지 않은 오류입니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)1, "성공하였습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54001,
		"길드 정보 처리 실패입니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54002,
		"존재하지 않는 계정입니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54003,
		"이미 동일한 이름의 길드가 있습니다."));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54004, "레벨이 부족합니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54005,
		"이미 길드에 가입되어 있거나 가입 신청 중입니다.."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54006,
		"길드 개설/폐쇄/탈퇴 후 24시간이 지나지 않았습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54019,
		"오류가 발생하였습니다.(길드 생성 번호)"));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54008,
		"명령을 실행할 권한이 없습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54009,
		"신청한 길드의 인원이 모두 차서 더는 가입할 수 없습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54010,
		"요청한 유저가 길드원이 아닙니다."));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54011, "부길드장이 되었습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54012,
		"부길드장 제한 인원 초과입니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54013, "길드장 추방"));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54014,
		"남아있는 길드원이 있기 때문에 길드를 폐쇄할 수 없습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54015,
		"길드가 존재하지 않습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54016,
		"길드원이 남아 있는 상태에서 길드를 탈퇴할 수 없습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54017,
		"이미 길드에 가입되어 있습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54018,
		"가입 대기 중인 길드원이 아닙니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54019,
		"가입 신청 상태가 아닙니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54020,
		"가입 대기자가 아니므로 길드 가입 철회를 요청할 수 없습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54021,
		"가입 대기자는 길드를 탈퇴할 수 없습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54030,
		"길드 대전 결과 저장 오류"));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54500, "데이터 베이스 오류"));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54501, "시스템 시간 변환 실패"));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54502,
		"아이템을 소유하고 있지 않습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54503,
		"길드 생성은 성공하였으나, 아이템 삭제에 실패하였습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54504,
		"현재 등급에서는 사용할 수 없는 기능입니다."));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54505, "길드장이 아닙니다."));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54506, "부길드장이 아닙니다."));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54507, "관리권한이 없습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54508,
		"길드에 가입되어 있지 않습니다."));
	m_msgMap.insert(
		std::make_pair((IngameGuildErrorCode)54509, "가입 대기자가 아닙니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54510,
		"이미 길드에 가입되어 있습니다."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54511,
		"유효하지 않은 길드상태입니다. 재접속 후 다시 시도해 주세요."));
	m_msgMap.insert(std::make_pair((IngameGuildErrorCode)54022,
		"길드 마크 등록이 금지된 상태입니다. 고객 지원 센터로 문의 바랍니다."));
	ClearAllData();
	return true;
}

bool _guild::CGuildInfo::Release()
{
	ClearAllData();
	return true;
}

void _guild::CGuildInfo::ClearAllData()
{
	if (0)
		((CPeriodContents*)0)->CPeriodContents::Set_PreSettingContentsDate();
	m_myGuildInfo.classIdx = (GUILD_CLASS_IDX)0;
	m_myGuildInfo.masterUID = (unsigned long)-1;
	m_myGuildInfo.guildUID = 0;
	m_myGuildInfo.guildPang = 0;
	m_myGuildInfo.guildPoint = 0;
	m_myGuildInfo.memberCount = 0;
	memset(&m_myGuildInfo.createTime, 0, sizeof(m_myGuildInfo.createTime));
	memset(m_myGuildInfo.introduce, 0, sizeof(m_myGuildInfo.introduce));
	memset(m_myGuildInfo.guildMark, 0, sizeof(m_myGuildInfo.guildMark));
	memset(m_myGuildInfo.masterNickname, 0,
		sizeof(m_myGuildInfo.masterNickname));
	memset(m_myGuildInfo.guildName, 0, sizeof(m_myGuildInfo.guildName));
	memset(m_myGuildInfo.notice, 0, sizeof(m_myGuildInfo.notice));
	m_allMemberCount = 0;
	m_allGuildCount = 0;
	ClearMemberPage();
	ClearHistoryList();
}

void _guild::CGuildInfo::SetHistoryList(std::list<GUILD_HISTORY>* pList)
{
	if (!m_historyList.empty())
		m_historyList.clear();
	for (std::list<GUILD_HISTORY>::iterator it = pList->begin();
		it != pList->end(); ++it)
		m_historyList.push_back(*it);
}

void _guild::CGuildInfo::SetMemberPage(int page,
	std::list<GUILD_USER_LIST>* pList)
{
	std::for_each(pList->begin(), pList->end(), sExchangerQuota());
	std::map<int, std::list<GUILD_USER_LIST> >::iterator found =
		m_memberPageMap.find(page);
	if (found != m_memberPageMap.end())
	{
		(*found).second.swap(*pList);
		return;
	}
	std::list<GUILD_USER_LIST> empty;
	m_memberPageMap.insert(std::make_pair(page, empty));
	m_memberPageMap[page].swap(*pList);
}

void _guild::CGuildInfo::SetSearchList(int page, std::list<GUILD_LIST>* pList)
{
	std::map<int, std::list<GUILD_LIST> >::iterator found =
		m_searchPageMap.find(page);
	if (found != m_searchPageMap.end())
	{
		(*found).second.swap(*pList);
		return;
	}
	std::list<GUILD_LIST> empty;
	m_searchPageMap.insert(std::make_pair(page, empty));
	m_searchPageMap[page].swap(*pList);
}

void CChristmasSockEvent::SetFlag(bool bFlag)
{
	m_bFlag = bFlag;
}

bool CChristmasSockEvent::GetFlag() const
{
	return m_bFlag;
}

bool CChristmasSockEvent::Initialize()
{
	m_bFlag = true;
	return true;
}

bool CChristmasSockEvent::Release()
{
	return Initialize();
}

bool CVectorSlideTestContainer::Initialize()
{
	m_bNewTypeVectorSlide = true;
	m_limitSpeed = 80;
	m_boundHeight = 25;
	m_limitVectorSlideCount = 35;
	return true;
}

bool CVectorSlideTestContainer::Release()
{
	return Initialize();
}

bool CGimmickContainer::Initialize()
{
	m_randomSeed = rand();
	m_minDistBtwItems = 40.0f;
	// 983 (00910120): chaves 1..18 (as do LoadFromPacket/LoadFromScript), limpa os itens pegos e o +0x9c
	for (int i = 1; i <= 18; i++)
	{
		std::vector<GimmickDispositionInformation> empty;
		m_gimmickPositionMap[(unsigned char)i] = empty;
	}
	m_acquiredItemInfoArray.clear();
	m_carpetSeqName = "wizcity_magic_carpet_bounding.seq";
	m_crashSeqName[0] = "wizcity_field_item_pangcoin_crash.seq";
	m_crashSeqName[1] = "wizcity_field_item_randombox_crash.seq";
	m_bSpecialBoxAnyMap983 = false;
	return true;
}

bool CGimmickContainer::Release()
{
	return Initialize();
}

void CGimmickContainer::AddGimmickPosition(unsigned char hole,
	const std::vector<GimmickDispositionInformation>& positions)
{
	m_gimmickPositionMap[hole] = positions;
}

std::vector<GimmickDispositionInformation>& CGimmickContainer::
	GetGimmickPosition(unsigned char hole)
{
	return m_gimmickPositionMap[hole];
}

void CGimmickContainer::ResetGimmikFlag(unsigned char hole,
	GimmickDispositionInformation::eFlag flag)
{
	std::vector<GimmickDispositionInformation>& positions =
		GetGimmickPosition(hole);
	for (unsigned int i = 0; i < positions.size(); i++)
		positions[i].flag = flag;
}

GimmickDispositionInformation* CGimmickContainer::FindDisposeInfo(
	unsigned char hole, GIMMICK_ITEM_TYPE type,
	GimmickDispositionInformation::eFlag flag)
{
	std::vector<GimmickDispositionInformation>& positions =
		GetGimmickPosition(hole);
	for (unsigned int i = 0; i < positions.size(); i++)
	{
		bool bType = false;
		bool bFlag = false;
		if (type == GIMMICK_ITEM_ANY || positions[i].type == type)
			bType = true;
		if (flag == GimmickDispositionInformation::GIMMICK_ANY ||
			positions[i].flag == flag)
			bFlag = true;
		if (bType && bFlag)
			return &positions[i];
	}
	return NULL;
}

int CGimmickContainer::GetNumDisposeInfo(unsigned char hole,
	GIMMICK_ITEM_TYPE type, GimmickDispositionInformation::eFlag flag)
{
	std::vector<GimmickDispositionInformation>& positions =
		GetGimmickPosition(hole);
	int count = 0;
	for (unsigned int i = 0; i < positions.size(); i++)
	{
		bool bType = false;
		bool bFlag = false;
		if (type == GIMMICK_ITEM_ANY || positions[i].type == type)
			bType = true;
		if (flag == GimmickDispositionInformation::GIMMICK_ANY ||
			positions[i].flag == flag)
			bFlag = true;
		if (bType && bFlag)
			count++;
	}
	return count;
}

C_ASSERT(sizeof(GimmickDispositionInformation) == 0x24);
void CGimmickContainer::LoadFromPacket(WReceivedPacket& packet)
{
	Release();
	m_randomSeed = packet.Decode4();
	// 983 (009103e0): buracos 1..18; registro com +0x20 == 4 ignorado; chave = ordem dos buracos do
	// CSharedDoc; caixa (type 1) liga m_bSpecialBoxAnyMap983
	for (int hole = 1; hole <= 18; hole++)
	{
		int count = packet.Decode1();
		for (int i = 0; i < count; i++)
		{
			GimmickDispositionInformation info;
			packet.DecodeBuffer(&info, sizeof(info));
			if (info.texType983 == 4)
				continue;
			unsigned char key = CSharedDoc::Instance()->m_holeOrder[info.hole];
			m_gimmickPositionMap[key].push_back(info);
			if (info.type == GIMMICK_ITEM_BOX)
				m_bSpecialBoxAnyMap983 = true;
		}
	}
}

bool CGimmickContainer::LoadFromScript(const char* filename)
{
	Release();
	lua_system::lua_table_helper helper =
		lua_system::lua_script_manger::GetInstance()->GetTable(filename);
	// 983 (00910600): posicoes so no mapa 0x13 (Wiz City), contagens lidas uma vez;
	// seed/gap/seq valem em qualquer mapa
	if (GetCurMap() == 0x13)
	{
		int boxCount = helper.GetValueByName<int>("gimmick_randombox_count");
		int pangCount = helper.GetValueByName<int>("gimmick_pang_count");
		for (int hole = 1; hole <= 18; hole++)
		{
			std::vector<GimmickDispositionInformation> positions;
			GimmickDispositionInformation info;
			for (int i = 0; i < boxCount; i++)
			{
				info.index = i;
				info.type = GIMMICK_ITEM_BOX;
				positions.push_back(info);
			}
			for (int i = 0; i < pangCount; i++)
			{
				info.index = boxCount + i;
				info.type = GIMMICK_ITEM_PANG;
				positions.push_back(info);
			}
			AddGimmickPosition((unsigned char)hole, positions);
		}
	}
	m_randomSeed = helper.GetValueByName<int>("gimmick_randomseed");
	m_minDistBtwItems =
		(float)helper.GetValueByName<int>("coin_object_placement_gap");
	m_carpetSeqName =
		helper.GetValueByName<const char*>("FxForBoundOnMagicCarpet");
	m_crashSeqName[0] =
		helper.GetValueByName<const char*>("FxForCrashFieldPang");
	m_crashSeqName[1] =
		helper.GetValueByName<const char*>("FxForCrashFieldRandomBox");
	return true;
}

bool CGimmickContainer::NeedToMakeTriPtArray(const char* texName,
	FieldItem::eDisposeTextureType* pType) const
{
	if (strstr(texName, "ct_green_b") != NULL)
	{
		if (pType != NULL)
			*pType = FieldItem::DISPOSE_TEXTURE_COIN;
		return true;
	}
	if (strcmp(texName, "ct_booster.dds") == 0)
	{
		if (pType != NULL)
			*pType = FieldItem::DISPOSE_TEXTURE_BOOSTER;
		return true;
	}
	return false;
}

void CMissionEvent::DecodeMissionInfo(WReceivedPacket& packet)
{
	if (!m_missionInfo.empty())
		ClearMissionInfo();

	long count = packet.Decode4();
	while (count > 0)
	{
		GlobalEnum::sMissionInfo info;
		packet.DecodeBuffer(&info, sizeof(info));
		m_missionInfo.push_back(info);
		count--;
	}
	SetDayGiftFlag(packet.Decode1() != 0);
	SetTermGiftFlag(packet.Decode1() != 0);
}

void CGimmickContainer::AddItemAcquired(const FieldItem::AcquiredItemInfo& info)
{
	m_acquiredItemInfoArray.push_back(info);
}
