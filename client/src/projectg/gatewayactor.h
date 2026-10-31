#pragma once

#include <map>
#include <string>
#include <vector>
#include "actor.h"
#include "gatewayinterface.h"
#include "wtemplate.h"

static const char* s_gatewayActor[2] = { "CGatewayActor", "Gateway" };

namespace _gateway
{

	class CGatewayActor : public IActor
	{
	protected:
		CGatewayActor();

	public:
		DECLARE_OBJECT(CGatewayActor)

		virtual ~CGatewayActor();

		virtual void OnPreLoadInit(); // 0091d510 (slots 2-7: repassam a cada manipulador de m_handlerList)
		virtual void OnLoad(); // 0091d540
		virtual void OnInit(); // 0091d570
		virtual void OnDestroy();
		virtual void OnProcess(float delta); // 0091d5a0
		virtual void OnDisplay(); // 0091d5e0
		virtual void HandleMsg(const MsgObject& msg);

	private:
		// JP 983: os manipuladores não são mais criados por uma função por tarefa (Create*Handler do 645). O construtor
		// guarda a máscara de cada tarefa (m_tasks) e a tabela tipo de documento -> (mensagem, máscara das tarefas) (m_contents);
		// CreateHandler(nome da tarefa) percorre a tabela (0091eb80).
		struct sContents
		{
			localContentType_t type;
			taskMsg_t msg;
			int taskMask;
		};

		bool CreateHandler(const char* name);
		bool InsertHandler(taskMsg_t msg, CGatewayHandler* pHandler);
		void QueryHandler(localContentType_t type, taskMsg_t msg, int taskMask);
		void RegisterContents(localContentType_t type, taskMsg_t msg, int taskMask); // 0091e9b0
		void RegisterAllContents(); // 0091ecf0

		std::map<std::string, int> m_tasks; // +0x18
		std::map<taskMsg_t, CGatewayHandler*> m_handlers; // +0x28
		std::vector<CGatewayHandler*> m_handlerList; // +0x38: InsertHandler (0091e800) também guarda aqui, na ordem de inserção
		std::map<localContentType_t, sContents> m_contents; // +0x48
	};

}
