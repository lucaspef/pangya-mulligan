#pragma once
#include <list>
#include <string>
#include "gamedatadb.h"

// JP 983: dados de jogo de pele/bronzeado ("TanLines"; classe nova; RTTI CGDSkinItem, vtable 00d0971c, 0x7c bytes — `push 0x7c`
// do criador 00a40f13). Cada linha "row" do XML dá (typeid, name, value): `typeid` = peça de roupa, `name` = tipo de textura
// que a linha NÃO troca, `value` = sufixo posto no nome do arquivo. Nomes de método e de campo meus.
struct sSkinParts983 // visao de CPartTidList (quem chama faz o cast); o 005446a0 so le as 24 pecas em +8
{
	unsigned long charTid; // +0x00: CPartTidList::m_charTid
	unsigned long partNumColors; // +0x04: m_defPartNum, m_partNum, m_hairColor, m_shirtsColor
	unsigned long tidParts[0x18]; // +0x08
};

class CGDSkinItem : public IGameData
{
public:
	struct sRow // 0x3c
	{
		sRow() { name = ""; value = ""; typeId = 0; }
		unsigned long typeId; // +0x00
		std::string name; // +0x04
		std::string value; // +0x20
	};

	CGDSkinItem(); // 00544880
	virtual ~CGDSkinItem(); // 00544910 (slot 0 = 005449a0)

	virtual void Initialize(TiXmlNode* pNode); // 005449d0

	// 005446a0 (único chamador de fora: 008afa64)
	void Apply(const char* kind, const char* name, std::string* pOut, const sSkinParts983* pParts, int cur);

	void SetParts(const unsigned long* pParts); // 00542d70 (chamado de 005ad07c)

protected:
	bool FileExists(const char* filename); // 00542c80 (não usa o `this`)
	void ResetCur(); // 00542cb0
	void SetCur(int cur); // 00542cc0
	int GetCur(); // 00542ce0 (ninguém chama por `call`)
	void ResetParts(); // 00542cf0
	void SetParts(const sSkinParts983* pParts); // 00542d40

	bool Overlaps(const unsigned long* pParts, unsigned long typeId); // 00543010
	bool OverlapsKept(const unsigned long* pParts, unsigned long typeId); // 00543690
	bool Check(const unsigned long* pParts, unsigned long typeId); // 00543970

	bool HasRow(const unsigned long* pParts); // 00543490: alguma das 24 peças tem linha
	bool IsRow(unsigned long typeId); // 005435d0
	bool ReplaceByCur(const char* kind, const char* name, std::string* pOut, unsigned long typeId); // 00543ac0

public:
	bool HasPartRow(); // 00543740 (chamado de 005acf6b): o mesmo que HasRow(m_parts)
	// 00543e10 (chamado de 007a61c9): pParts nulo = as peças guardadas
	bool Replace(const char* kind, const char* name, std::string* pOut, const unsigned long* pParts);
	// 00544200 (9 chamadores): guarda as peças e troca o nome do arquivo
	bool Replace(const char* kind, const char* name, std::string* pOut, const sSkinParts983* pParts);

	void SetB7a983(bool b) { m_b7a = b; } // FrTaskForm::Close (00523730) desliga +0x7a

protected:

	std::list<sRow> m_rows; // +0x04
	int m_cur; // +0x10
	bool m_bCur; // +0x14
	unsigned long m_parts[0x18]; // +0x18
	bool m_bParts; // +0x78
	bool m_bKept; // +0x79
	bool m_b7a; // +0x7a (só o construtor grava nesta parte)

public:
	static CGDSkinItem* s_pInstance; // 00e75190
};
