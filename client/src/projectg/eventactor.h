#pragma once

#include <list>
#include <map>
#include <string>
#include "actor.h"
#include "gatewayinterface.h"

namespace _event
{

	enum eEventIdentifier
	{
		EVENT_NONE
	};

	class CEventActor : public IActor
	{
	protected:
		CEventActor();

	public:
		DECLARE_OBJECT(CEventActor)

		virtual ~CEventActor();

		virtual void OnPreLoadInit();
		virtual void HandleMsg(const MsgObject& msg);

	private:
		void CreateLobbyHandler();
		void CreateAvatarChatHandler();
		void CreateMyRoomHandler();
		void CreateShopHandler();
		void CreateGolfHandler();

		std::map<taskMsg_t, _gateway::CGatewayHandler*> m_handlers;
		std::list<eEventIdentifier> m_events;
		std::map<std::string, _util::WMemFun<CEventActor, void> > m_creators;
	};

}
