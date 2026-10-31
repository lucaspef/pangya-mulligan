#pragma once
#include <list>
#include "gamedatadb.h"

// JP 983: dados de jogo "SetAuxpartsStatus" (classe nova; RTTI CGDSetAuxparsAbility, vtable 00d096cc, 0x30 bytes — `push 0x30` do
// criador 00a409e0). Tipo 5 do CGameDataDB. Cada linha "row" do XML dá um par de anéis (tid1, tid2) e o bônus de cada atributo.
// Nomes de método e de campo meus (os de campo seguem o nome do atributo no XML).
#define GAMEDATA983_SETAUXPARTSSTATUS ((eGameDataDB)5)

class CGDSetAuxparsAbility : public IGameData
{
public:
	struct sRow
	{
		sRow() : tid1(0), tid2(0), pow(0), cnt(0), acc(0), spn(0), cur(0), drvup(0) { }
		unsigned long tid1; // +0x00
		unsigned long tid2; // +0x04
		int pow; // +0x08
		int cnt; // +0x0c
		int acc; // +0x10
		int spn; // +0x14
		int cur; // +0x18
		int drvup; // +0x1c
	};

	// 00542280 (cópia fora de linha; ninguém chama por `call`)
	static CGDSetAuxparsAbility* Instance()
	{
		if (!s_pInstance)
			s_pInstance = (CGDSetAuxparsAbility*)CGameDataDB::Instance()->GetGameData(GAMEDATA983_SETAUXPARTSSTATUS,
				"SetAuxpartsStatus");
		return s_pInstance;
	}

	CGDSetAuxparsAbility(); // 005427b0
	virtual ~CGDSetAuxparsAbility(); // 00542860 (slot 0 = 005429a0)

	virtual void Initialize(TiXmlNode* pNode); // 005429d0

	// 005428b0: bônus do atributo `type` (0 POW … 4 CUR) do primeiro par de anéis de `tids[5]` que tem linha
	int GetStatus(int type, const unsigned long* tids);

	// 005426c0: deixa a linha do par em m_cur
	bool Find(unsigned long tidA, unsigned long tidB);

private:
	static CGDSetAuxparsAbility* s_pInstance; // 00e75188

	std::list<sRow> m_rows; // +0x04
	sRow m_cur; // +0x10
};
