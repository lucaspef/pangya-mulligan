#include "minatl.h"
#include "paper.h"
#include <atltime.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "packet.h"

static const char* s_szThemes[] = {
	"PT_CUPINHOLDING",
	"PT_RECORD",
	"PT_AWARD",
	"PT_MAX",
};

static ATL::CTimeSpan s_tsServerDiff;

CPaperMemo::CPaperMemo(const char* funcSig)
	: m_funcSig(funcSig)
{
}

CPaperMemo::~CPaperMemo()
{
}

void CPaperMemo::Write(ePaperTheme theme, const char* fmt, ...)
{
	char buf[512] = { 0 };
	va_list args;
	va_start(args, fmt);
	_vsnprintf(buf, 511, fmt, args);
	va_end(args);
	WSingleton<CPaper>::Instance()->AddMemo(theme, m_funcSig.c_str(), buf);
}

CPaper::CPaper()
	: m_bUpdatable(0)
{
	s_tsServerDiff = ATL::CTimeSpan(0);
}

CPaper::~CPaper()
{
}

void CPaper::SetServerTime(const SYSTEMTIME& st)
{
	SYSTEMTIME local;
	GetLocalTime(&local);
	ATL::CTime localTime(local);
	ATL::CTime serverTime(st);
	s_tsServerDiff = serverTime - localTime;
}

void CPaper::PrepareNewPaper(const char* nick)
{
	Destroy();
	m_nick = nick;
	GetServerTime(m_createTime);
}

void CPaper::Destroy()
{
	memset(&m_createTime, 0, sizeof(m_createTime));
	memset(&m_sendTime, 0, sizeof(m_sendTime));
	m_nick = "";
	m_memos.clear();
	m_funcSigs.clear();
	m_funcSigIds.clear();
}

unsigned int CPaper::GetMemoSize(ePaperTheme theme) const
{
	std::multimap<ePaperTheme, cMemo>::const_iterator first =
		m_memos.lower_bound(theme);
	std::multimap<ePaperTheme, cMemo>::const_iterator last =
		m_memos.upper_bound(theme);
	return std::distance(first, last);
}

bool CPaper::AddMemo(ePaperTheme theme, const char* funcSig, const char* msg)
{
	if (m_bUpdatable)
		return false;
	std::multimap<ePaperTheme, cMemo>::const_iterator first =
		m_memos.lower_bound(theme);
	std::multimap<ePaperTheme, cMemo>::const_iterator last =
		m_memos.upper_bound(theme);
	if (std::distance(first, last) < 999)
	{
		cMemo memo(GetServerTime(), GetFuncSigId(funcSig), msg);
		m_memos.insert(
			std::multimap<ePaperTheme, cMemo>::value_type(theme, memo));
		return true;
	}
	return false;
}

const char* CPaper::GetEnumStr(ePaperTheme theme) const
{
	if (theme < PT_MAX)
		return s_szThemes[theme];
	return "";
}

const char* CPaper::GetFuncSigStr(unsigned int id) const
{
	if (id > 0 && id < m_funcSigs.size())
		return m_funcSigs[id].c_str();
	return "";
}

const char* CPaper::GetServerTime() const
{
	static char buf[512];
	SYSTEMTIME st;
	GetLocalTime(&st);
	ATL::CTime t(st);
	t += s_tsServerDiff;
	struct tm local;
	t.GetLocalTm(&local);
	memset(buf, 0, sizeof(buf));
	strftime(buf, 511, "%y-%m-%d %H:%M:%S", &local);
	return buf;
}

void CPaper::GetServerTime(SYSTEMTIME& st) const
{
	SYSTEMTIME local;
	GetLocalTime(&local);
	ATL::CTime t(local);
	t += s_tsServerDiff;
	t.GetAsSystemTime(st);
}

bool CPaper::PrintOut(ePaperTheme theme)
{
	std::multimap<ePaperTheme, cMemo>::const_iterator it =
		m_memos.lower_bound(theme);
	if (it == m_memos.end())
		return false;
	FILE* fp = fopen("./saved/paper.xml", "wt");
	if (fp == NULL)
		return false;
	fprintf(fp,
		"<?xml version=\"1.0\" encoding=\"euc-kr\" standalone=\"yes\" ?>\n");
	fprintf(fp,
		"<?xml:stylesheet type=\"text/xsl\" href=\"paperview.xsl\" ?>\n");
	fprintf(fp, "<!DOCTYPE PAPERS[\n");
	fprintf(fp, "\t<!ELEMENT PAPERS (INDEX,PAPER)>\n");
	fprintf(fp, "\t<!ELEMENT INDEX  (USERNICK,CREATETIME,SENDTIME)>\n");
	fprintf(fp, "\t<!ELEMENT PAPER  (NUMBER,TIME,FUNCSIG,MESSAGE)>\n");
	fprintf(fp, "\t<!ELEMENT USERNICK \t(#PCDATA)>\n");
	fprintf(fp, "\t<!ELEMENT CREATETIME (#PCDATA)>\n");
	fprintf(fp, "\t<!ELEMENT SENDTIME \t(#PCDATA)>\n");
	fprintf(fp, "\t<!ELEMENT NUMBER \t(#PCDATA)>\n");
	fprintf(fp, "\t<!ELEMENT TIME\t\t(#PCDATA)>\n");
	fprintf(fp, "\t<!ELEMENT FUNCSIG\t(#PCDATA)>\n");
	fprintf(fp, "\t<!ELEMENT MESSAGE\t(#PCDATA)>\n");
	fprintf(fp, "]>\n");
	fprintf(fp, "<PAPERS>\n");
	fprintf(fp, "<INDEX>\n");
	fprintf(fp, "\t<THEME>%s</THEME>\n", GetEnumStr(theme));
	fprintf(fp, "\t<USERNICK>%s</USERNICK>\n", m_nick.c_str());
	fprintf(fp, "\t<CREATETIME>%s</CREATETIME>\n",
		SystemTimeToString(m_createTime));
	fprintf(fp, "\t<SENDTIME>%s</SENDTIME>\n", GetServerTime());
	fprintf(fp, "</INDEX>\n");
	int number = 0;
	std::multimap<ePaperTheme, cMemo>::const_iterator end =
		m_memos.upper_bound(theme);
	for (; it != end; ++it)
	{
		const cMemo& memo = it->second;
		fprintf(fp, "<PAPER>\n");
		fprintf(fp, "\t<NUMBER>%d</NUMBER>\n", ++number);
		fprintf(fp, "\t<TIME>%s</TIME>\n", memo.m_time.c_str());
		fprintf(fp, "\t<FUNCSIG>%s</FUNCSIG>\n",
			GetFuncSigStr(memo.m_funcSigId));
		fprintf(fp, "\t<MESSAGE>%s</MESSAGE>\n", memo.m_msg.c_str());
		fprintf(fp, "</PAPER>\n");
	}
	fprintf(fp, "</PAPERS>\n");
	fclose(fp);
	return true;
}

void CPaper::ReceivedPacket(WReceivedPacket* pPacket)
{
	pPacket->Decode1();
	m_bUpdatable = 0;
}

void CPaper::SendUpdatableQuery()
{
	m_bUpdatable = 1;
}

void CPaper::Send()
{
}

const char* CPaper::SystemTimeToString(const SYSTEMTIME& st) const
{
	static char buf[512];
	ATL::CTime t(st);
	struct tm local;
	t.GetLocalTm(&local);
	memset(buf, 0, sizeof(buf));
	strftime(buf, 511, "%y-%m-%d %H:%M:%S", &local);
	return buf;
}

unsigned int CPaper::GetFuncSigId(const char* funcSig)
{
	std::map<std::string, unsigned int>::iterator it =
		m_funcSigIds.find(funcSig);
	if (it != m_funcSigIds.end())
		return it->second;
	unsigned int id = (unsigned int)m_funcSigIds.size();
	m_funcSigIds.insert(
		std::map<std::string, unsigned int>::value_type(funcSig, id));
	m_funcSigs.push_back(funcSig);
	return id;
}
