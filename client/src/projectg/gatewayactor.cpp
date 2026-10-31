#include "minatl.h"
#include "gatewayactor.h"

#include <algorithm>
#include "gatewaydoc.h"
#include "contentsdoc.h"
#include "exception.h"

static const char* s_taskName[5] = { "CLobbyTask", "CShopTask",
	"CRealMyRoomTask", "ntAvatarChatTask", "CGolfTask" };

namespace _gateway
{

	IMPLEMENT_ACTOR(CGatewayActor, IActor)

	const WRTTI CGatewayHandler::m_RTTI("CGatewayHandler", &IObject::m_RTTI); // 00f007ac

	CGatewayActor::CGatewayActor()
	{
		m_dispPriority = DISP_PRIORITY_4;
		AddSkipList(0x20);

		m_tasks.insert(std::make_pair(s_taskName[0], 1));
		m_tasks.insert(std::make_pair(s_taskName[1], 2));
		m_tasks.insert(std::make_pair(s_taskName[2], 4));
		m_tasks.insert(std::make_pair(s_taskName[3], 8));
		m_tasks.insert(std::make_pair(s_taskName[4], 0x10));

		RegisterAllContents();
	}

	CGatewayActor::~CGatewayActor()
	{
	}

	// 0091f970: slot 10 (OnLeave) de cada manipulador, depois esvazia a lista, a tabela e as tarefas. B1368: o nosso só fazia
	// m_tasks.clear(); sem o OnLeave o CControllerDoc nunca era limpo (004bb130) e o SetObjects da partida seguinte (que só insere
	// jogador novo) mantinha os objetos da primeira — Elf Ears e os outros controladores só valiam na 1ª partida da sessão.
	void CGatewayActor::OnDestroy()
	{
		for (std::vector<CGatewayHandler*>::iterator it = m_handlerList.begin(), end = m_handlerList.end(); it != end; ++it)
			(*it)->OnEvent10_983();
		m_handlerList.clear();
		m_handlers.clear();
		m_tasks.clear();
	}

	struct sHandlerProcess
	{
		float m_delta;
		sHandlerProcess(float delta) : m_delta(delta) { }
		void operator()(CGatewayHandler* pHandler) { pHandler->OnEvent07_983(m_delta); }
	};

	// 0091d510..0091d5e0: os slots do ator repassam a cada manipulador (slots 4..8 do CGatewayHandler; vazios em todos no 983)
	void CGatewayActor::OnPreLoadInit()
	{
		for (std::vector<CGatewayHandler*>::iterator it = m_handlerList.begin(), end = m_handlerList.end(); it != end; ++it)
			(*it)->OnEvent04_983();
	}

	void CGatewayActor::OnLoad()
	{
		for (std::vector<CGatewayHandler*>::iterator it = m_handlerList.begin(), end = m_handlerList.end(); it != end; ++it)
			(*it)->OnEvent05_983();
	}

	void CGatewayActor::OnInit()
	{
		for (std::vector<CGatewayHandler*>::iterator it = m_handlerList.begin(), end = m_handlerList.end(); it != end; ++it)
			(*it)->OnEvent06_983();
	}

	void CGatewayActor::OnProcess(float delta)
	{
		std::for_each(m_handlerList.begin(), m_handlerList.end(), sHandlerProcess(delta));
	}

	void CGatewayActor::OnDisplay()
	{
		for (std::vector<CGatewayHandler*>::iterator it = m_handlerList.begin(), end = m_handlerList.end(); it != end; ++it)
			(*it)->OnEvent08_983();
	}

	void CGatewayActor::HandleMsg(const MsgObject& msg)
	{
		taskMsg_t message = (taskMsg_t)msg.message;
		if (message != 0x2712)
		{
			std::map<taskMsg_t, CGatewayHandler*>::const_iterator it =
				m_handlers.find(message);
			if (it == m_handlers.end())
				return;
			(*it).second->Execute(msg);
			return;
		}

		CreateHandler((const char*)msg.param1);
	}

	bool CGatewayActor::CreateHandler(const char* name)
	{
		std::map<std::string, int>::const_iterator it =
			m_tasks.find(std::string(name));
		if (it == m_tasks.end())
			return false;

		int taskMask = (*it).second;
		for (std::map<localContentType_t, sContents>::const_iterator c =
				 m_contents.begin();
			c != m_contents.end(); ++c)
		{
			if ((*c).second.taskMask & taskMask)
				QueryHandler((*c).second.type, (*c).second.msg, taskMask);
		}
		return true;
	}

	bool CGatewayActor::InsertHandler(taskMsg_t msg, CGatewayHandler* pHandler)
	{
		m_handlers.insert(std::make_pair(msg, pHandler));
		m_handlerList.push_back(pHandler);
		return true;
	}

	void CGatewayActor::QueryHandler(localContentType_t type, taskMsg_t msg,
		int taskMask)
	{
		// sic: no exe 983 (0091e881) o teste é `type > 0xb1 && type < 0xa7`, que nunca é verdadeiro
		if (type > 0xb1 && type < 0xa7)
			throw WAppException("Passed Unknown doctype!!\nCode[0]");

		IContentsDataContainer* pDoc = CContentsDoc::Instance()->GetContainer(type);
		if (pDoc == NULL)
			throw WAppException("Need Doc Object!!\nCode[1]");

		CGatewayHandler* pHandler =
			((CGatewayControlDoc*)pDoc)->GetGatewayHandler();
		if (pHandler == NULL)
			throw WAppException("Need Gateway Handler!!\nCode[2]");

		((CGatewayControlDoc*)pDoc)->SetTaskMask983(taskMask);
		pHandler->SetDocument(pDoc);
		if (pHandler->EnsureInitialized983())
			InsertHandler(msg, pHandler);
	}

	void CGatewayActor::RegisterContents(localContentType_t type, taskMsg_t msg,
		int taskMask)
	{
		sContents contents;
		contents.type = type;
		contents.msg = msg;
		contents.taskMask = taskMask;

		if (m_contents.find(type) == m_contents.end())
			m_contents.insert(std::make_pair(type, contents));
	}

	// JP 983 (0091ecf0): tipo de documento, número da mensagem (lido do exe; não passa por M983) e tarefas que o usam
	// (1 lobby, 2 loja, 4 meu quarto, 8 chat de avatar, 0x10 jogo). Classe de cada tipo (tools/f35/doccriadores.py):
	// 0xac CQuestDocument (_questoneday), 0xa9 CGolfItemDoc, 0xab CSpecialBoxDoc, 0xaa CHoleInOneGameDoc, 0xad CControllerDoc,
	// 0xae CWorkShop2013Doc, 0xaf CGrandPrixDocument, 0xb0 CArinBirthDay2014Doc. A senha secundária e a troca do 645
	// (CSecondPwdDoc/CTradeDoc, mensagens 0x26d/0x273) não existem no 983.
	void CGatewayActor::RegisterAllContents()
	{
		RegisterContents((localContentType_t)0xac, (taskMsg_t)0x753a, 0x1f);
		RegisterContents((localContentType_t)0xa9, (taskMsg_t)0x7532, 0x11);
		RegisterContents((localContentType_t)0xab, (taskMsg_t)0x7531, 9);
		RegisterContents((localContentType_t)0xaa, (taskMsg_t)0x7535, 0x10);
		RegisterContents((localContentType_t)0xad, (taskMsg_t)0x753b, 0x19);
		RegisterContents((localContentType_t)0xae, (taskMsg_t)0x753c, 1);
		RegisterContents((localContentType_t)0xaf, (taskMsg_t)0x753e, 0x11);
		RegisterContents((localContentType_t)0xb0, (taskMsg_t)0x753f, 1);
	}

}
