#include "minatl.h"
#include "gatewayactor.h"
#include "eventactor.h"

static const char* s_taskName[5] = { "CLobbyTask", "CShopTask",
	"CRealMyRoomTask", "ntAvatarChatTask", "CGolfTask" };

namespace _event
{

	IMPLEMENT_ACTOR(CEventActor, IActor)

	CEventActor::CEventActor()
	{
		m_creators.insert(std::make_pair(s_taskName[0],
			_util::MakeMemFun(*this, &CEventActor::CreateLobbyHandler)));
		m_creators.insert(std::make_pair(s_taskName[1],
			_util::MakeMemFun(*this, &CEventActor::CreateShopHandler)));
		m_creators.insert(std::make_pair(s_taskName[2],
			_util::MakeMemFun(*this, &CEventActor::CreateMyRoomHandler)));
		m_creators.insert(std::make_pair(s_taskName[3],
			_util::MakeMemFun(*this, &CEventActor::CreateAvatarChatHandler)));
		m_creators.insert(std::make_pair(s_taskName[4],
			_util::MakeMemFun(*this, &CEventActor::CreateGolfHandler)));
	}

	CEventActor::~CEventActor()
	{
	}

	void CEventActor::OnPreLoadInit()
	{
		m_events.push_back(EVENT_NONE);
	}

	void CEventActor::HandleMsg(const MsgObject& msg)
	{
		taskMsg_t message = (taskMsg_t)msg.message;
		switch (message)
		{
		case M983(0x274):
		case 0x2712:
			return;
		}

		std::map<taskMsg_t, _gateway::CGatewayHandler*>::const_iterator it =
			m_handlers.find(message);
		if (it != m_handlers.end())
			(*it).second->Execute(msg);
	}

	void CEventActor::CreateLobbyHandler()
	{
	}

	void CEventActor::CreateAvatarChatHandler()
	{
	}

	void CEventActor::CreateMyRoomHandler()
	{
	}

	void CEventActor::CreateShopHandler()
	{
	}

	void CEventActor::CreateGolfHandler()
	{
	}

}
