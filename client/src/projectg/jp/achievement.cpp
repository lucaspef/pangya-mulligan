// JP 983: unidade 0047a020–0047c930 (_achievement::CAchievement, uma conquista do jogador). Classe do 839/983, sem par no
// 645; ver include/jp/achievement.h. Sessão B101 (F3.7.10). O corte da F3.7.1 começava em 0047a060: 0047a020 é o Decode do
// contador (inline: embutido no Decode983 e emitido). O resto da faixa são moldes que o compilador gera daqui: std::sort do vetor de
// ponteiros de contador (0047a450…0047c2b0), std::transform com back_inserter (0047c180/0047c530), vector::reserve
// (0047ba70) e o std::stringstream do ToString983 (0047a510…0047c580).
#include "minatl.h"
#include "projectg.h"
#include "packet.h"
#include "../../shared/itemmanager.h"
#include "shareddoc.h"
#include "achievement.h"
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>

namespace _achievement
{
// 0047a020: embutida no Decode983 e, como no 983, emitida fora de linha também (COMDAT sem chamador; IGUAL)
inline void sAchievementCounter983::Decode983(WReceivedPacket* p)
{
	id = p->Decode4();
	value = p->Decode4();
	time = p->Decode4();
}

// 0047a060
CAchievement::~CAchievement()
{
}

// 0047a070
unsigned long CAchievement::GetTypeId983() const
{
	return m_typeId;
}

// 0047a080
unsigned long CAchievement::GetCounterId983(unsigned i) const
{
	return m_counters[i].id;
}

// 0047a090
int CAchievement::FindCounterValue983(unsigned long id) const
{
	for (unsigned i = 0; i < MAX_COUNTER; ++i)
	{
		if (m_counters[i].id == id)
			return m_counters[i].value;
	}
	return 0;
}

// 0047a0c0
int CAchievement::GetCounterValue983(unsigned i) const
{
	return m_counters[i].value;
}

// 0047a0d0
const __int64& CAchievement::GetCounterTime983(unsigned i) const
{
	return m_counters[i].time;
}

// ordem do std::sort do GetLatestCounter983: o mais recente primeiro
struct sCounterTimeGreater983
{
	bool operator()(const sAchievementCounter983* a, const sAchievementCounter983* b) const; // 0047a0f0
};

// 0047a0f0
bool sCounterTimeGreater983::operator()(const sAchievementCounter983* a, const sAchievementCounter983* b) const
{
	return a->time > b->time;
}

// 0047a120
unsigned long CAchievement::GetOpenCounterId983() const
{
	for (unsigned i = 0; i < MAX_COUNTER; ++i)
	{
		if (m_counters[i].id != 0 && m_counters[i].time == 0)
			return m_counters[i].id;
	}
	return 0;
}

// 0047a150
int CAchievement::GetOpenCounter983() const
{
	for (unsigned i = 0; i < MAX_COUNTER; ++i)
	{
		if (m_counters[i].id != 0 && m_counters[i].time == 0)
			return i;
	}
	return -1;
}

// 0047a170
int CAchievement::GetLastCounter983() const
{
	for (int i = MAX_COUNTER - 1; i > -1; --i)
	{
		const sAchievementCounter983& c = m_counters[i];
		if ((c.time != 0 || c.id != 0) && c.time != 0 && c.id != 0)
			return i;
	}
	return -1;
}

// 0047a1b0
unsigned long CAchievement::GetCounterId2_983(unsigned i) const
{
	return m_counters[i].id;
}

// 0047a1f0
int CAchievement::GetCounterValue2_983(unsigned i) const
{
	return m_counters[i].value;
}

// 0047a200
sAchievementCounter983* CAchievement::GetCounter983(unsigned i)
{
	if (i >= MAX_COUNTER)
		return NULL;
	return &m_counters[i];
}

// 0047a640
CAchievement::CAchievement()
{
	m_typeId = 0;
	m_uid = 0;
	for (int i = 0; i < MAX_COUNTER; ++i)
	{
		m_counters[i].id = 0;
		m_counters[i].value = 0;
		m_counters[i].time = 0;
	}
}

// 0047a720
void CAchievement::Decode983(WReceivedPacket* p)
{
	m_typeId = p->Decode4();
	m_uid = p->Decode4();
	unsigned long n = p->Decode4();
	for (unsigned i = 0; i < n; ++i)
	{
		sAchievementCounter983* c = GetCounter983(i);
		if (c)
			c->Decode983(p);
	}
}

// endereço de cada contador (std::transform do GetLatestCounter983)
struct sCounterAddress983
{
	sAchievementCounter983* operator()(sAchievementCounter983& c) const { return &c; }
};

// 0047c5b0
int CAchievement::GetLatestCounter983()
{
	std::vector<sAchievementCounter983*> counters;
	counters.reserve(MAX_COUNTER);
	std::transform(m_counters, m_counters + MAX_COUNTER, std::back_inserter(counters), sCounterAddress983());
	std::sort(counters.begin(), counters.end(), sCounterTimeGreater983());
	if (counters[0]->time == 0)
		return -1;
	int index = 0;
	for (unsigned i = 0; i < MAX_COUNTER; ++i)
	{
		if (counters[0] == &m_counters[i])
		{
			index = i;
			break;
		}
	}
	return index;
}

// 0047c6c0
std::string CAchievement::ToString983()
{
	std::stringstream ss;
	ss << "'" << Doc()->m_itemManager.GetItemName(GetTypeId983()) << "'" << " " << "(" << "TID:" << GetTypeId983() << ")";
	for (unsigned i = 0; i < MAX_COUNTER; ++i)
	{
		sAchievementCounter983* c = GetCounter983(i);
		if (c && c->id)
			ss << "[" << c->value << "]";
	}
	return ss.str();
}
}
