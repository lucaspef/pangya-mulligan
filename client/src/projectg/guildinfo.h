#pragma once

#include <string>
#include <list>
#include <map>
#include "contentsdoc.h"
#include "../shared/globalgamedefine.h"

namespace _guild
{
	void __fastcall ExchangeQuotation(char* text);

	struct sExchangerQuota
	{
		void operator()(GUILD_USER_LIST& user);
	};

	class CGuildInfo : public IContentsDataContainer
	{
	public:
		virtual bool Initialize();
		virtual bool Release();

		void SetGuildIntroduce(const std::string& introduce);
		void SetGuildNotice(const std::string& notice);
		const GUILD_INFO& GetMyGuildInfo() const;
		bool IsGuildMember() const;
		bool IsGuildMember(GUILD_CLASS_IDX classIdx);
		bool IsManager() const;
		bool IsManager(GUILD_CLASS_IDX classIdx);
		const char* GuildMsg(int id);
		const std::list<GUILD_USER_LIST>* GetMemberList(int page);
		const std::list<GUILD_LIST>* GetSearchList(int page);
		int GetAllMemberCount() { return m_allMemberCount; }
		int GetAllGuildCount() { return m_allGuildCount; }
		int GetMaxMemberPage();
		int GetMaxSearchPage();

		void SetMyGuildInfo(GUILD_INFO& info);
		void SetMyGuildUserInfo(GUILD_USER_INFO& info);
		void SetGuildEmblem(GUILD_USER_INFO* pInfo);
		void SetGuildName(GUILD_INFO* pInfo);
		void ClearMemberPage();
		void ClearMemberList(int page);
		void ClearSearchPage();
		void ClearSearchList(int page);
		void ClearHistoryList();
		void ClearAllData();
		void SetHistoryList(std::list<GUILD_HISTORY>* pList);
		void SetMemberPage(int page, std::list<GUILD_USER_LIST>* pList);
		void SetSearchList(int page, std::list<GUILD_LIST>* pList);

		const GUILD_CLASS_IDX GetClassIndex() const
		{
			return m_myGuildInfo.classIdx;
		}
		int GetGuildNumber() const { return m_myGuildInfo.guildUID; }
// HACK: inline in the original class; only seen by ingameguildlobbydlg.cpp, the
// extra names move COMDATs / scheduling in projectg.cpp and golfrule.cpp
#ifdef REBANG_GUILDINFO_INLINES
		bool IsWaitingJoin() const
		{
			if (m_myGuildInfo.classIdx == 9)
				return true;
			return false;
		}
		const std::list<GUILD_HISTORY>& GetHistoryList() const
		{
			return m_historyList;
		}
#endif
		void SetAllMemberCount(int count) { m_allMemberCount = count; }
		void SetAllGuildCount(int count) { m_allGuildCount = count; }

	protected:
		GUILD_INFO m_myGuildInfo;
		unsigned char m_unused12d[3];
		std::list<GUILD_HISTORY> m_historyList;
		std::map<int, const char*> m_msgMap;
		std::map<int, std::list<GUILD_USER_LIST> > m_memberPageMap;
		std::map<int, std::list<GUILD_LIST> > m_searchPageMap;
		int m_allMemberCount;
		int m_allGuildCount;
	};
}
