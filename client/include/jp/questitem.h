#pragma once
// JP 983: _quest::CQuestItem (0x108), uma missão (diária / conquista) do jogador: typeid do QuestItem.iff, uid, estado
// (1 START, 2 ACCEPT, 3 CONDITION, 4 COMPLETE, 5 REWARD) e 10 contadores (QuestStuff.iff). É o elemento da lista
// `CSharedDoc+0x384c` (CSharedDoc::m_quests983; pedido B97 → A). Unidade questitem.cpp, 005e5c00–005e71a0,
// compilada com LTCG (005e6050 recebe o typeid em eax; 005e6350 guarda o `this` em edx através de uma chamada): não dá IGUAL.
// vtable 00d0b40c (1 posição: o destrutor com `delete`, 005851a0, dobrado com outro idêntico). Existe no 839 (005ea630…), não no 645.
// Nomes dos métodos e dos campos são meus (sessão B97).

#include <sstream>
#include <string>

class WReceivedPacket;

namespace _quest
{
	// contador de uma missão (0x18): peça do QuestStuff.iff, item contado e data de conclusão. 005e5c00 é o construtor (COMDAT).
	struct sQuestCounter983
	{
		sQuestCounter983() : stuffId(0), typeId(0), uid(0), doneTime(0) {} // 005e5c00

		unsigned long stuffId; // +0x0: QuestStuff.iff (FindQuestStuff983; +0xd4 = quantidade pedida)
		unsigned long typeId; // +0x4: item contado (0 = contador sem uso)
		unsigned long uid; // +0x8: chave na lista de contagens do CSharedDoc (+0x3840)
		__int64 doneTime; // +0x10: quando completou (0 = não completou); vira data por 00b307f0
	};

	// contagem do servidor (elemento 0x10 da lista CSharedDoc+0x3840): pacote {u8, u32, u32, u32}
	struct sQuestCount983
	{
		void Decode(WReceivedPacket* p); // 005e6010

		bool bValid; // +0x0
		unsigned long typeId; // +0x4
		unsigned long uid; // +0x8
		unsigned long count; // +0xc
	};

	class CQuestItem
	{
	public:
		enum { MAX_COUNTER = 10 };

		CQuestItem(); // 005e6080
		CQuestItem(unsigned long typeId, unsigned long uid); // 005e6190: estado START, ativa
		virtual ~CQuestItem(); // 005e5c20

		void Update983(const CQuestItem& other); // 005e5c30: copia cabeçalho e os contadores usados (typeId != 0)
		void Deactivate983(); // 005e5ee0
		bool IsActive983() const; // 005e5ef0
		unsigned long GetTypeId983() const; // 005e5f00
		unsigned long GetUid983() const; // 005e5f10
		bool IsState983(int state) const; // 005e5f20
		int GetState983() const; // 005e5f30
		unsigned long GetCounterUid983(int index) const; // 005e5f40
		const char* GetStateName983() const; // 005e5f60
		sQuestCounter983* GetCounter983(unsigned int index); // 005e5fb0: NULL fora de 0..9
		sQuestCounter983* FindCounter983(unsigned long stuffId); // 005e5fd0
		void Decode983(WReceivedPacket* p); // 005e62b0: {u8 ativa, u32 typeid, u32 uid, u32 estado, u32 n, n × {u32 × 4}}
		void MergeDone983(const CQuestItem* other); // 005e6350: estado e datas de conclusão de `other`
		void Accept983(); // 005e63b0: missão diária → CONDITION e contagens zeradas no CSharedDoc; senão REWARD e inativa
		float GetProgress983(unsigned int index); // 005e6780: % (0..100) do contador
		float GetProgress10_983(unsigned int index); // 005e6800: % do resto por 10 (medidor da missão "10 vezes")
		std::string ToString983(); // 005e6f60: texto de depuração (nome, TID, UID, estado, contadores)
		void AppendCounters983(std::stringstream* pStream); // 005e6870: contadores da conquista (cumpridos, faltando, contagens)
		void AppendCounts983(std::stringstream* pStream); // 005e66d0: contagens da missão diária

	protected:
		long GetCounterGoal983(const sQuestCounter983* c); // 005e6420: QuestStuff.iff +0xd4 (`this` sem uso)
		long GetCounterCount983(const sQuestCounter983* c); // 005e6680: contagem na lista do CSharedDoc (+0x3840) pelo uid (`this` sem uso)

		bool m_bActive; // +0x8 (+0x4 é enchimento: a classe alinha em 8 pelo __int64 dos contadores)
		unsigned long m_typeId; // +0xc
		unsigned long m_uid; // +0x10
		int m_state; // +0x14
		sQuestCounter983 m_counters[MAX_COUNTER]; // +0x18
	};

	bool IsDailyQuestType983(unsigned long typeId); // 005e6050: (typeId & 0xfc000000) == 0x78000000 e subtipo (bits 22..25) == 2
}
C_ASSERT(sizeof(_quest::sQuestCounter983) == 0x18);
C_ASSERT(sizeof(_quest::sQuestCount983) == 0x10);
C_ASSERT(sizeof(_quest::CQuestItem) == 0x108);
