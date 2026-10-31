#pragma once
// JP 983: _achievement::CAchievement (0xb0, vtable 00cfe704), uma conquista do jogador: typeid do Achievement.iff, uid e 10
// contadores. Unidade 0047a020–0047c930 (src/projectg/jp/achievement.cpp, sessão B101). Layout pelo construtor (zera
// +0x08…+0xac) e pelo Decode (0047a720). Nomes meus (sessões B99/B101).

#include <string>

class WReceivedPacket;

namespace _achievement
{
// contador de uma conquista (0x10)
struct sAchievementCounter983
{
	inline void Decode983(WReceivedPacket* p); // 0047a020 (inline no achievement.cpp, embutida no CAchievement::Decode983): {u32 id, u32 valor, u32 data}

	unsigned long id; // +0x0: typeid da peça (QuestStuff.iff)
	int value; // +0x4 (o ToString983 escreve com operator<<(int))
	__int64 time; // +0x8: o Decode grava só a parte baixa (a alta fica 0); 0 = ainda não cumprido
};

class CAchievement
{
public:
	enum { MAX_COUNTER = 10 };

	CAchievement(); // 0047a640: tudo zerado
	virtual ~CAchievement(); // 0047a060
	unsigned long GetTypeId983() const; // 0047a070
	unsigned long GetCounterId983(unsigned i) const; // 0047a080
	int FindCounterValue983(unsigned long id) const; // 0047a090: valor do contador da peça id (0 = não tem)
	int GetCounterValue983(unsigned i) const; // 0047a0c0
	const __int64& GetCounterTime983(unsigned i) const; // 0047a0d0
	unsigned long GetOpenCounterId983() const; // 0047a120: peça do 1º contador ainda não cumprido (0 = nenhum)
	int GetOpenCounter983() const; // 0047a150: índice do 1º contador ainda não cumprido (−1 = nenhum)
	int GetLastCounter983() const; // 0047a170: índice do último contador cumprido (−1 = nenhum)
	unsigned long GetCounterId2_983(unsigned i) const; // 0047a1b0: mesmo corpo de 0047a080 (chamada pela FrAchievement 004e7bb0)
	int GetCounterValue2_983(unsigned i) const; // 0047a1f0: mesmo corpo de 0047a0c0 (sem chamador)
	sAchievementCounter983* GetCounter983(unsigned i); // 0047a200: NULL fora de 0..9
	void Decode983(WReceivedPacket* p); // 0047a720: {u32 typeid, u32 uid, u32 n, n × contador}
	int GetLatestCounter983(); // 0047c5b0: índice do contador cumprido mais recente (−1 = nenhum)
	std::string ToString983(); // 0047c6c0: "'nome' (TID:typeid)[valor]…" (depuração)

	unsigned long m_typeId; // +0x08 (+0x4 é enchimento: alinha em 8 pelo __int64 dos contadores)
	unsigned long m_uid; // +0x0c
	sAchievementCounter983 m_counters[MAX_COUNTER]; // +0x10
};
}
C_ASSERT(sizeof(_achievement::sAchievementCounter983) == 0x10);
C_ASSERT(sizeof(_achievement::CAchievement) == 0xb0);
