// JP 983: unidade 00852ee0–00853170 (00853170 em diante é a approachdesc): _champ::CMatchActor, o ator do campeonato.
// No 645 o matchactor.cpp existe vazio. Nomes meus (sessão B117); ver include/jp/championship.h.
#include "minatl.h"
#include "championship.h"
#include "contentsdoc.h"
#include "exception.h"

namespace _champ
{
	// 00852ee0
	CMatchActor::CMatchActor()
	{
		m_pRuleHandler = NULL;
		m_pDoc = NULL;
	}

	// 00852f10
	CMatchActor::~CMatchActor()
	{
	}

	// 00852f20
	void CMatchActor::HandleMsg(const MsgObject& msg)
	{
		m_pRuleHandler->Execute(msg);
	}

	IMPLEMENT_ACTOR(CMatchActor, IActor)

	// 00853030
	void CMatchActor::OnInit()
	{
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x95, m_pDoc);
		if (m_pDoc != NULL)
		{
			m_pRuleHandler = m_pDoc->m_pRuleHandler;
			if (m_pRuleHandler == NULL)
				throw WAppException(std::string("champRuleHandler Failed to Create!"));

			if (!m_pRuleHandler->Initialize())
				throw WAppException(std::string("RuleHandler Failed to Create!"));
		}
		else
		{
			throw WAppException(std::string("ChampionShip Data Resource Failed to Create!"));
		}
	}
}
