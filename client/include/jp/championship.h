#pragma once
#include <list>
#include <vector>
#include "actor.h"
#include "wtemplate.h"
#include "icontentsdatacontainer.h"

// JP 983: campeonato (namespace _champ; RTTI .?AVCChampionShip@_champ@@, .?AVCChampionShipImpl@_champ@@,
// .?AVCMatchActor@_champ@@). Unidades 008527c0–00852ee0 (jp/championshipimpl.cpp) e 00852ee0–00853170 (jp/matchactor.cpp).
// O 645 não tem nada disso (matchactor.cpp vazio). Nomes de método e de campo meus (sessão B117).
//
// Ninguém no 983 chama o construtor da CChampionShipImpl (00852da0) nem cria o documento 0x95: o CMatchActor só lança
// exceção se o documento não existir. Código vivo no binário, mas sem uso visível.

namespace _champ
{
	class CChampionShip;

	// Documento de conteúdo 0x95 (localContentType_t), da unidade contentsdoc (frente A). VISÃO (empréstimo): só o que a
	// unidade usa. +0x08 vector<unsigned char> (as rodadas), +0x18 o tratador de regras.
	struct CChampionShipDoc983 : public IContentsDataContainer
	{
		std::vector<unsigned char>* GetRounds983(); // 00907810: &m_rounds
		void CopyRounds983(std::list<unsigned char>* pOut); // 0090bd80: push_back de cada rodada

		int m_04_983; // +0x04
		std::vector<unsigned char> m_rounds; // +0x08
		CChampionShip* m_pRuleHandler; // +0x18
	};

	// vtable 00d31904 (dtor, 2 puras), 0x1c bytes
	class CChampionShip : public FunctionMapper
	{
	public:
		CChampionShip(void* pOwner) // 00852c20 (cópia fora de linha sem chamador)
			: m_pOwner(pOwner)
		{
		}
		virtual ~CChampionShip() { } // 00852d20 (deleting); 00852cb0 = cópia fora de linha sem chamador

		virtual bool Initialize() = 0;
		virtual bool Execute(const MsgObject& msg) = 0;

	protected:
		void* m_pOwner; // +0x18
	};

	// vtable 00d31914, 0x28 bytes. Mensagens (param1 = id, param2 = saída): 1 primeira (volta ao início), 2 última,
	// 3 próxima, 4 anterior, 5 primeira, 6 última, 7 copia a lista inteira (std::list<unsigned char>*).
	class CChampionShipImpl : public CChampionShip
	{
	public:
		CChampionShipImpl(void* pOwner); // 00852da0
		virtual ~CChampionShipImpl(); // 00852e30

		virtual bool Initialize(); // 008529e0
		virtual bool Execute(const MsgObject& msg); // 008529c0

	private:
		void OnCopyRounds(int param); // 008527c0
		void OnFirst(int param); // 008527e0
		void OnNext(int param); // 00852800
		void OnPrev(int param); // 00852860
		void OnLast(int param); // 008528a0
		void OnGetFirst(int param); // 008528c0
		void OnGetLast(int param); // 008528e0

		CChampionShipDoc983* m_pDoc; // +0x1c (não iniciado pelo construtor)
		std::vector<unsigned char>* m_pRounds; // +0x20
		unsigned char m_index; // +0x24
	};

	// vtable 00d3192c, 0x20 bytes. Pega o documento 0x95 e repassa as mensagens ao tratador de regras dele.
	class CMatchActor : public IActor
	{
	public:
		static const WRTTI m_RTTI; // 00e7c760
		virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00852f00

		CMatchActor(); // 00852ee0
		virtual ~CMatchActor(); // 00852f10

		virtual void OnInit(); // 00853030
		virtual void HandleMsg(const MsgObject& msg); // 00852f20

	private:
		CChampionShip* m_pRuleHandler; // +0x18
		CChampionShipDoc983* m_pDoc; // +0x1c
	};
}
