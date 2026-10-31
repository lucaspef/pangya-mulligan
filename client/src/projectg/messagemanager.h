#pragma once

#include <map>

namespace _systemmsg
{
	enum eMsgIdentifier
	{
	};

	struct sMsgInfo
	{
		eMsgIdentifier id;
		const char* msg;
	};

	class CMessageManager : public WSingleton<CMessageManager>
	{
	public:
		CMessageManager();
		virtual ~CMessageManager();

		bool InsertMessage(eMsgIdentifier id, const char* msg);
		bool InsertMessageGroup(sMsgInfo* info, int num);
		const char* GetMsg(eMsgIdentifier id);
		void NotifyNormalMessage(eMsgIdentifier id);
		void NotifyNormalMessage(const char* msg);

	private:
		std::map<eMsgIdentifier, const char*> m_msgMap;
	};
}
