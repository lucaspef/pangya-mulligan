#include "minatl.h"
#define REBANG_LOGININFO_CPP
#include "logininfo.h"
#include "s5/utilities.h"

int NtreevLogin::Init()
{
	// 983 00b2c580: abre o mapeamento direto (sem ChannelingChecker)
	_sNtreevUserIdentity identity;
	memset(&identity, 0, sizeof(identity));

	HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE,
		"927628CA6D76A6E9162C56D4E3E6D6E3");
	if (hMapFile == NULL)
		return FALSE;

	void* pMapData = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (pMapData == NULL)
		return FALSE;

	SetNtreevUserIdentity((const char*)pMapData, identity);
	Convert(&identity);

	return TRUE;
}

void NtreevLogin::SetProvType()
{
	m_provType = 2;
}

void NtreevLogin::Convert(void* pIdentity)
{
	_sNtreevUserIdentity* identity = (_sNtreevUserIdentity*)pIdentity;

	m_id = identity->loginId;
	m_fullId = identity->loginId;
	m_passwd = identity->authKey;
	m_authUid = identity->memberNo;
	m_pcBang = identity->pcBangNo != 0;	// 983 00b2b2e0: sem birthDay/zipCode/gender
}

void NtreevLogin::SetNtreevUserIdentity(const char* data,
	_sNtreevUserIdentity& identity)
{
	std::string strData(data);
	std::string key;
	std::string value;

	size_t pos = 0;
	do
	{
		size_t start = strData.find('|', pos) + 1;
		pos = strData.find('|', start);

		std::string token;
		if (start != std::string::npos)
		{
			if (pos != std::string::npos)
			{
				token.assign(strData.begin() + start, strData.begin() + pos);
			}
			else
			{
				token.assign(strData.begin() + start, strData.end());
			}

			size_t eq = token.find('=');
			key.assign(token.begin(), token.begin() + eq);
			value.assign(token.begin() + eq + 1, token.end());

			if (stricmp(key.c_str(), "LoginID") == 0)
			{
				strncpy(identity.loginId, value.c_str(), 50);
				continue;
			}
			if (stricmp(key.c_str(), "AuthKey") == 0)
			{
				strncpy(identity.authKey, value.c_str(), 64);
				continue;
			}
			if (stricmp(key.c_str(), "Sex") == 0)
			{
				identity.sex = !(value == "M");
				continue;
			}
			if (stricmp(key.c_str(), "MemberNo") == 0)
			{
				identity.memberNo = atoi(value.c_str());
				continue;
			}
			if (stricmp(key.c_str(), "Age") == 0)
			{
				identity.age = atoi(value.c_str());
				continue;
			}
			if (stricmp(key.c_str(), "WardNo") == 0)
			{
				identity.wardNo = atoi(value.c_str());
				continue;
			}
			if (stricmp(key.c_str(), "CityCode") == 0)
			{
				identity.cityCode = atoi(value.c_str());
				continue;
			}
			if (stricmp(key.c_str(), "PCBangNo") == 0)
			{
				identity.pcBangNo = atoi(value.c_str());
				continue;
			}
		}
	} while (pos != std::string::npos);
}

CLoginInfo* CLoginInfo::m_pInst = NULL;

CLoginInfo::CLoginInfo()
	: m_pLoginInfo(NULL), m_bWebLogin(0)
{
}

int CLoginInfo::Destroy()
{
	if (m_pLoginInfo != NULL)
	{
		delete m_pLoginInfo;
		m_pLoginInfo = NULL;
	}

	if (m_pInst != NULL)
	{
		delete m_pInst;
		m_pInst = NULL;
	}

	return TRUE;
}

CLoginInfo::~CLoginInfo()
{
}

int CLoginInfo::InitWebLogin(const char* cmdLine)
{
	// 983 00b2b360: a linha de comando vira string mas nao escolhe mais Ntreev/NHN (sem -vwccnp)
	std::string cmd(cmdLine);

	if (m_pLoginInfo != NULL)
	{
		if (m_pLoginInfo->Init())
		{
			m_pLoginInfo->SetProvType();
			m_bWebLogin = TRUE;
			return TRUE;
		}

		delete m_pLoginInfo;
	}

	m_pLoginInfo = new PangYaLogin;
	return FALSE;
}

unsigned long CLoginInfo::ProvType() const
{
	return m_pLoginInfo->ProvType();
}

const char* CLoginInfo::FullId() const
{
	return m_pLoginInfo->FullId();
}

const char* CLoginInfo::Id() const
{
	return m_pLoginInfo->Id();
}

unsigned long CLoginInfo::Uid() const
{
	return m_pLoginInfo->Uid();
}

const char* CLoginInfo::Passwd() const
{
	return m_pLoginInfo->Passwd();
}

int CLoginInfo::IsPcBang() const
{
	return m_pLoginInfo->IsPcBang();
}

unsigned long CLoginInfo::AuthUid() const
{
	return m_pLoginInfo->AuthUid();
}

void CLoginInfo::EnablePcBang(int enable)
{
	m_pLoginInfo->EnablePcBang(enable);
}

void CLoginInfo::SetId(const std::string& id)
{
	m_pLoginInfo->Id(id);
	m_pLoginInfo->FullId(id);
}

void CLoginInfo::SetUid(unsigned long uid)
{
	m_pLoginInfo->Uid(uid);
}

void CLoginInfo::SetAuthUid(unsigned long uid)
{
	m_pLoginInfo->AuthUid(uid);
}

void CLoginInfo::SetPw(const std::string& pw)
{
	m_pLoginInfo->Passwd(pw);
}

ILoginInfo* CLoginInfo::QueryInterface()
{
	return m_pLoginInfo;
}

void CLoginInfo::SetProvType(unsigned long type)
{
	m_pLoginInfo->ProvType(type);
}

bool cHttpNtreevAuthResult::GetResult()
{
	std::string result = GetElement("result");
	if (result == "true")
	{
		return true;
	}

	return false;
}

std::string cHttpNtreevAuthResult::GetMessageA()
{
	std::string message = GetElement("messages");

	char multiByte[260] = "";
	wchar_t wide[260] = L"";
	MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, wide, 260);
	WideCharToMultiByte(CP_OEMCP, 0, wide, -1, multiByte, 260, NULL, NULL);

	return multiByte;
}

std::string cHttpNtreevAuthResult::GetElement(const char* tag)
{
	std::string startTag = "<";
	startTag += tag;
	startTag += ">";

	std::string endTag = "</";
	endTag += tag;
	endTag += ">";

	size_t start = m_xml.find(startTag);
	size_t end = m_xml.find(endTag);
	if (start != std::string::npos && end != std::string::npos)
	{
		start += startTag.length();
		return std::string(m_xml.begin() + start, m_xml.begin() + end);
	}

	return "";
}

struct FnTrim
{
	FnTrim(const char* trimChars)
		: m_trimChars(trimChars), m_result("")
	{
	}
	void operator()(char c)
	{
		if (m_trimChars.find(c) == std::string::npos)
			m_result += c;
	}

	std::string m_trimChars;
	std::string m_result;
};

std::string cHttpNtreevAuthResult::GetFieldValue(const char* name)
{
	std::string arg = GetElement("arg");
	FnTrim trim = std::for_each(arg.begin(), arg.end(), FnTrim(" \t\n\r\0"));
	arg = trim.m_result;

	size_t pos = arg.find(name);
	if (pos == std::string::npos)
	{
		return "";
	}

	size_t eq = arg.find("=", pos + 1);
	size_t bar = arg.find("|", eq + 1);
	if (eq != std::string::npos)
	{
		if (bar != std::string::npos)
		{
			return std::string(arg.begin() + eq + 1, arg.begin() + bar);
		}

		return std::string(arg.begin() + eq + 1, arg.end());
	}

	return "";
}

std::string cHttpNtreevAuthResult::GetExternalLink()
{
	return "";
}
