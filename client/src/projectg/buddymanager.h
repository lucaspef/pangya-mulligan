#pragma once

#include <map>

struct sockaddr_in;

class BuddyManager : public WSingleton<BuddyManager>
{
public:
	BuddyManager();
	virtual ~BuddyManager();

	bool Init();
	void Close();
	void UnRegisterDNS();
	void CheckBuddyList();
	void QueryBuddy(const char* nick);
	void ClearBuddy();
	void SendDNS(const char* addr, int port);
	void AddBuddy(sFriend* pFriend);
	void DeleteBuddy(unsigned long uid);
	void SendChat(const char* nick, const char* msg);
	int GetBuddySize();
	std::map<unsigned long, sFriend>* GetBuddyList();
	void UpdateBuddyInfo(sFriend* pFriend);
	void BuddyLogOn(unsigned long uid);
	void BuddyLogOut(unsigned long uid);
	sFriend* GetBuddyByNick(const char* nick);
	sFriend* GetBuddy(unsigned long uid);
	void SetRecentPlayer();
	void Process();
	void DisplayString(const char* str);
	void ProcCallBackMsg(sockaddr_in& addr, const char* packet, int size);
	void ProcCallBackErr(const char* err);

	int IsInit() { return m_bInit; }

private:
	unsigned long m_reserved;
	std::map<unsigned long, sFriend> m_buddyList;
	bool m_bInit;
};

extern BuddyManager* BuddyMGR;
inline BuddyManager* Buddy()
{
	return BuddyMGR;
}
