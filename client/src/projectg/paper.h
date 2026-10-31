#pragma once
#include <map>
#include <string>
#include <vector>
#include "singleton.h"

class WReceivedPacket;

enum ePaperTheme
{
	PT_CUPINHOLDING,
	PT_RECORD,
	PT_AWARD,
	PT_MAX
};

class CPaper : public WSingleton<CPaper>
{
public:
	class cMemo
	{
	public:
		cMemo(const char* time, unsigned int funcSigId, const char* msg)
			: m_time(time), m_funcSigId(funcSigId), m_msg(msg)
		{
		}

		std::string m_time;
		unsigned int m_funcSigId;
		std::string m_msg;
	};

	CPaper();
	virtual ~CPaper();

	void Destroy();
	void PrepareNewPaper(const char* nick);
	bool AddMemo(ePaperTheme theme, const char* funcSig, const char* msg);
	bool PrintOut(ePaperTheme theme);
	const char* GetEnumStr(ePaperTheme theme) const;
	const char* GetFuncSigStr(unsigned int id) const;
	unsigned int GetFuncSigId(const char* funcSig);
	unsigned int GetMemoSize(ePaperTheme theme) const;
	void SetServerTime(const SYSTEMTIME& st);
	const char* GetServerTime() const;
	void GetServerTime(SYSTEMTIME& st) const;
	const char* SystemTimeToString(const SYSTEMTIME& st) const;
	void ReceivedPacket(WReceivedPacket* pPacket);
	void SendUpdatableQuery();
	void Send();

protected:
	SYSTEMTIME m_createTime;
	SYSTEMTIME m_sendTime;
	std::string m_nick;
	std::multimap<ePaperTheme, cMemo> m_memos;
	std::vector<std::string> m_funcSigs;
	std::map<std::string, unsigned int> m_funcSigIds;
	int m_bUpdatable;
};

class CPaperMemo
{
public:
	CPaperMemo(const char* funcSig);
	~CPaperMemo();

	void Write(ePaperTheme theme, const char* fmt, ...);

protected:
	std::string m_funcSig;
};
