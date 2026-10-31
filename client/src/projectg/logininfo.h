#pragma once

#include <string>

struct _sNtreevUserIdentity
{
	unsigned long memberNo;
	char sex;
	unsigned char age;
	int wardNo;
	unsigned short cityCode;
	unsigned char pcBangNo;
	char reserved983;	// 983 00b2bfd0: loginId +0x10, authKey +0x43, sizeof 0x84 (sem o reserved[65] do 645)
	char loginId[51];
	char authKey[65];
};

class ILoginInfo
{
public:
	ILoginInfo() { }
	virtual ~ILoginInfo() { }

	virtual int Init() = 0;
	virtual void SetProvType() = 0;

	virtual unsigned long ProvType() = 0;
	virtual void ProvType(unsigned long type) = 0;
	virtual const char* FullId() = 0;
	virtual void FullId(std::string id) = 0;
	virtual const char* Id() = 0;
	virtual void Id(std::string id) = 0;
	virtual unsigned long Uid() = 0;
	virtual void Uid(unsigned long uid) = 0;
	virtual const char* Passwd() = 0;
	virtual void Passwd(std::string pw) = 0;
	virtual int IsPcBang() = 0;
	virtual unsigned long AuthUid() = 0;
	virtual void AuthUid(unsigned long uid) = 0;
	virtual void EnablePcBang(int enable) = 0;
	virtual void PcBang(int pcBang) = 0;
};

class CLoginInfo
{
public:
	// A775: inline em todo o 983 (38 chamadas diretas ao ctor 00b2aee0; Login 00adae90, OnPasswordInit 00ad9c10)
	static __forceinline CLoginInfo* Instance()
	{
		if (m_pInst == NULL)
			m_pInst = new CLoginInfo;
		return m_pInst;
	}

	int Destroy();
	int InitWebLogin(const char* cmdLine);

	int IsWebLogin() const { return m_bWebLogin; }

	unsigned long ProvType() const;
	const char* FullId() const;
	const char* Id() const;
	unsigned long Uid() const;
	const char* Passwd() const;
	int IsPcBang() const;
	unsigned long AuthUid() const;

	void EnablePcBang(int enable);
	void SetId(const std::string& id);
	void SetUid(unsigned long uid);
	void SetAuthUid(unsigned long uid);
	void SetPw(const std::string& pw);
	void SetProvType(unsigned long type);

	ILoginInfo* QueryInterface();

private:
	CLoginInfo();
	virtual ~CLoginInfo();

	static CLoginInfo* m_pInst;

	ILoginInfo* m_pLoginInfo;
	int m_bWebLogin;
};

// 983 PangYaLogin 0x68 (vtable 00d5265c, 19 entradas): sem Gender/ZipCode/BirthDay, sem ChannelingChecker e sem NhnLogin
class BaseLoginInfo : public ILoginInfo
{
public:
	BaseLoginInfo()
		: m_authUid(0),
		  m_uid(0),
		  m_provType(2),
		  m_pcBang(0)
	{
	}
	virtual ~BaseLoginInfo() { }

	virtual const char* FullId() { return m_fullId.c_str(); }
	virtual const char* Id() { return m_id.c_str(); }
	virtual unsigned long Uid() { return m_uid; }
	virtual unsigned long AuthUid() { return m_authUid; }
	virtual const char* Passwd() { return m_passwd.c_str(); }
	virtual unsigned long ProvType() { return m_provType; }
	virtual int IsPcBang() { return m_pcBang; }

	virtual void Id(std::string id) { m_id = id; }
	virtual void FullId(std::string id) { m_fullId = id; }
	virtual void Uid(unsigned long uid) { m_uid = uid; }
	virtual void AuthUid(unsigned long uid) { m_authUid = uid; }
	virtual void Passwd(std::string pw) { m_passwd = pw; }
	virtual void ProvType(unsigned long type) { m_provType = type; }
	virtual void PcBang(int pcBang) { m_pcBang = pcBang; }

	virtual void EnablePcBang(int enable) { m_pcBang = enable; }

protected:
	virtual void Convert(void* pIdentity) = 0;

	std::string m_id;
	std::string m_fullId;
	std::string m_passwd;
	unsigned long m_authUid;
	unsigned long m_uid;
	unsigned long m_provType;
	int m_pcBang;
};

class NtreevLogin : public BaseLoginInfo
{
public:
	NtreevLogin() { }
	virtual ~NtreevLogin() { }

	virtual int Init();
	virtual void SetProvType();

protected:
	virtual void Convert(void* pIdentity);

private:
	void SetNtreevUserIdentity(const char* data,
		_sNtreevUserIdentity& identity);
};

class PangYaLogin : public BaseLoginInfo
{
public:
	PangYaLogin() { }
	virtual ~PangYaLogin() { }

	virtual int Init() { return FALSE; }

	virtual void Convert(void* pIdentity) { }
	virtual void SetProvType() { m_provType = 2; }
};

class cHttpNtreevAuthResult
{
public:
	cHttpNtreevAuthResult(const char* xml)
		: m_xml(xml)
	{
	}

	bool GetResult();
	std::string GetMessageA();
	std::string GetFieldValue(const char* name);
	std::string GetExternalLink();

private:
	std::string GetElement(const char* tag);

	std::string m_xml;
};

inline CLoginInfo* LOGININFO()
{
	return CLoginInfo::Instance();
}

inline const char* LOGINID()
{
	return LOGININFO()->Id();
}
inline unsigned long AUTHUID()
{
	return LOGININFO()->AuthUid();
}

inline const char* LOGINPW()
{
	return LOGININFO()->Passwd();
}
inline bool IsWebLogin()
{
	return LOGININFO()->IsWebLogin() ? true : false;
}
inline int IsPcBang()
{
	return LOGININFO()->IsPcBang();
}

// HACK: logininfo.cpp defines FnTrim itself (its position there decides the
// COMDAT order); the other includers keep seeing it here.
#ifndef REBANG_LOGININFO_CPP
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
#endif
