#pragma once

#include <map>
#include "actor.h"
#include "contentsdoc.h"
#include "wtemplate.h"

enum taskMsg_t
{
	TASKMSG_NONE
};

namespace _gateway
{
	// JP 983 (pedido B14): vtables 00d026fc/00d026f4, construtor 004bd460, 0x20 bytes. Deriva de IObject (+0x00) e
	// FunctionMapper (+0x04); o FrCmdTarget do 645 saiu da base (cada manipulador que recebe retorno de janela o herda).
	// Virtuais: [0] GetRTTI, [1] destrutor, [2] Initialize, [3] Execute, [4]-[8] e [10] vazios na base, [9] EnsureInitialized.
	class CGatewayHandler : public IObject, public FunctionMapper
	{
	public:
		static const WRTTI m_RTTI; // 00f007ac
		virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 004bd590

		CGatewayHandler()
			: m_pDocument(NULL)
		{
		}
		virtual ~CGatewayHandler() { }

		virtual bool Initialize() = 0;
		virtual bool Execute(const MsgObject& msg) = 0;
		virtual void OnEvent04_983() { } // 004bd520 (nome provisorio; uso nao levantado)
		virtual void OnEvent05_983() { } // 004bd530
		virtual void OnEvent06_983() { } // 004bd540
		virtual void OnEvent07_983(float) { } // 004bd550
		virtual void OnEvent08_983() { } // 004bd560
		// 004bb3d0: sem nenhuma funcao registrada no FunctionMapper, chama Initialize; senao devolve true
		virtual bool EnsureInitialized983()
		{
			if (m_functionMap.size() == 0)
				return Initialize();
			return true;
		}
		virtual void OnEvent10_983() { } // 004bd570

		template <class T>
		T GetDocument()
		{
			return (T)m_pDocument;
		}
		void SetDocument(IContentsDataContainer* pDocument)
		{
			m_pDocument = pDocument;
		}

	protected:
		IContentsDataContainer* m_pDocument; // +0x1c
	};
}
