// JP 983: unidade 008527c0–00852ee0 (logo depois da CPointShop; 00852ee0+ é o CMatchActor): _champ::CChampionShip e
// _champ::CChampionShipImpl, o tratador de regras do campeonato (FunctionMapper de 7 mensagens sobre as rodadas do
// documento 0x95). Classes novas do 983; nomes de método meus (sessão B117); ver include/jp/championship.h.
#include "minatl.h"
#include "championship.h"

namespace _champ
{
	// 00907810: this+8
	std::vector<unsigned char>* CChampionShipDoc983::GetRounds983()
	{
		return &m_rounds;
	}

	// 0090bd80: push_back de cada rodada na lista de saida
	void CChampionShipDoc983::CopyRounds983(std::list<unsigned char>* pOut)
	{
		int n = (int)m_rounds.size();
		for (int i = 0; i < n; i++)
			pOut->push_back(m_rounds[i]);
	}

	// 008527c0: copia a lista de rodadas do documento para a std::list<unsigned char> de param2
	void CChampionShipImpl::OnCopyRounds(int param)
	{
		const MsgObject* pMsg = (const MsgObject*)param;
		m_pDoc->CopyRounds983((std::list<unsigned char>*)pMsg->param2);
	}

	// 008527e0
	void CChampionShipImpl::OnFirst(int param)
	{
		const MsgObject* pMsg = (const MsgObject*)param;
		unsigned char* pOut = (unsigned char*)pMsg->param2;
		m_index = 0;
		*pOut = (*m_pRounds)[0];
	}

	// 00852800: avança até a última (fica nela)
	void CChampionShipImpl::OnNext(int param)
	{
		const MsgObject* pMsg = (const MsgObject*)param;
		unsigned char* pOut = (unsigned char*)pMsg->param2;
		int size = m_pRounds->size();
		if (m_index < size - 1)
		{
			++m_index;
			*pOut = (*m_pRounds)[m_index];
			return;
		}
		*pOut = (*m_pRounds)[size - 1];
	}

	// 00852860: volta até a primeira (fica nela)
	void CChampionShipImpl::OnPrev(int param)
	{
		const MsgObject* pMsg = (const MsgObject*)param;
		unsigned char* pOut = (unsigned char*)pMsg->param2;
		if (m_index != 0)
		{
			--m_index;
			*pOut = (*m_pRounds)[m_index];
			return;
		}
		*pOut = (*m_pRounds)[0];
	}

	// 008528a0
	void CChampionShipImpl::OnLast(int param)
	{
		const MsgObject* pMsg = (const MsgObject*)param;
		*(unsigned char*)pMsg->param2 = (*m_pRounds)[m_pRounds->size() - 1];
	}

	// 008528c0
	void CChampionShipImpl::OnGetFirst(int param)
	{
		const MsgObject* pMsg = (const MsgObject*)param;
		*(unsigned char*)pMsg->param2 = (*m_pRounds)[0];
	}

	// 008528e0
	void CChampionShipImpl::OnGetLast(int param)
	{
		const MsgObject* pMsg = (const MsgObject*)param;
		*(unsigned char*)pMsg->param2 = (*m_pRounds)[m_pRounds->size() - 1];
	}

	// 008529c0
	bool CChampionShipImpl::Execute(const MsgObject& msg)
	{
		SetFunctionType(msg.param1);
		return Exec(this, (int)&msg);
	}

	// 008529e0
	bool CChampionShipImpl::Initialize()
	{
		AddFunctionType(1, FUNCTIONTYPE_ARG,
			(void (FunctionMapper::*)())(void (CChampionShipImpl::*)())&CChampionShipImpl::OnFirst);
		AddFunctionType(3, FUNCTIONTYPE_ARG,
			(void (FunctionMapper::*)())(void (CChampionShipImpl::*)())&CChampionShipImpl::OnNext);
		AddFunctionType(4, FUNCTIONTYPE_ARG,
			(void (FunctionMapper::*)())(void (CChampionShipImpl::*)())&CChampionShipImpl::OnPrev);
		AddFunctionType(2, FUNCTIONTYPE_ARG,
			(void (FunctionMapper::*)())(void (CChampionShipImpl::*)())&CChampionShipImpl::OnLast);
		AddFunctionType(7, FUNCTIONTYPE_ARG,
			(void (FunctionMapper::*)())(void (CChampionShipImpl::*)())&CChampionShipImpl::OnCopyRounds);
		AddFunctionType(5, FUNCTIONTYPE_ARG,
			(void (FunctionMapper::*)())(void (CChampionShipImpl::*)())&CChampionShipImpl::OnGetFirst);
		AddFunctionType(6, FUNCTIONTYPE_ARG,
			(void (FunctionMapper::*)())(void (CChampionShipImpl::*)())&CChampionShipImpl::OnGetLast);

		m_pRounds = m_pDoc->GetRounds983();
		return true;
	}

	// 00852da0
	CChampionShipImpl::CChampionShipImpl(void* pOwner)
		: CChampionShip(pOwner)
	{
		m_pRounds = NULL;
		m_index = 0;
	}

	// 00852e30
	CChampionShipImpl::~CChampionShipImpl()
	{
	}
}
