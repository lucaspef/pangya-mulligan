// JP 983: unidade 005e5c00–005e71a0 (_quest::CQuestItem, a missão do jogador: diária e conquista). Classe do 839/983, sem par no
// 645; ver include/jp/questitem.h. Sessão B97 (F3.7.10, `quests_presenca`). Compilada com LTCG no 983 (convenções próprias em
// 005e6050 / 005e6350): o critério é funcional. Os textos com stringstream (005e6870, 005e6f60, 005e66d0) entraram na B104.
// 005e6450 / 005e6605 são o `operator<<(ostream&, const string&)` (molde).
#include "minatl.h"
#include "projectg.h"
#include "packet.h"
#include "../../shared/itemmanager.h"
#include "shareddoc.h"
#include "questitem.h"
#include "questdocument.h"
#include <math.h>
#include <stdio.h>
#include <sstream>

// S5 (sharedutilities.cpp da frente A) — empréstimo, nome meu (ver gproomtimersteward.cpp)
namespace S5
{
void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); // 00b307f0
}

using namespace _quest;

#define QUESTSTUFF_GOAL983(p) (*(const long*)((const char*)(p) + 0xd4))

// 005e5c20
CQuestItem::~CQuestItem()
{
}

// 005e5c30
void CQuestItem::Update983(const CQuestItem& other)
{
	m_bActive = other.m_bActive;
	m_typeId = other.m_typeId;
	m_uid = other.m_uid;
	m_state = other.m_state;
	for (int i = 0; i < MAX_COUNTER; i++)
	{
		if (other.m_counters[i].typeId != 0)
			m_counters[i] = other.m_counters[i];
	}
}

// 005e5ee0
void CQuestItem::Deactivate983()
{
	m_bActive = false;
}

// 005e5ef0
bool CQuestItem::IsActive983() const
{
	return m_bActive == true;
}

// 005e5f00
unsigned long CQuestItem::GetTypeId983() const
{
	return m_typeId;
}

// 005e5f10
unsigned long CQuestItem::GetUid983() const
{
	return m_uid;
}

// 005e5f20
bool CQuestItem::IsState983(int state) const
{
	return m_state == state;
}

// 005e5f30
int CQuestItem::GetState983() const
{
	return m_state;
}

// 005e5f40
unsigned long CQuestItem::GetCounterUid983(int index) const
{
	return m_counters[index].uid;
}

// 005e5f60
const char* CQuestItem::GetStateName983() const
{
	switch (m_state)
	{
	case 1: return "START";
	case 2: return "ACCEPT";
	case 3: return "CONDITION";
	case 4: return "COMPLETE";
	case 5: return "REWARD";
	}
	return "NONE";
}

// 005e5fb0
sQuestCounter983* CQuestItem::GetCounter983(unsigned int index)
{
	if (index >= MAX_COUNTER)
		return NULL;
	return &m_counters[index];
}

// 005e5fd0
sQuestCounter983* CQuestItem::FindCounter983(unsigned long stuffId)
{
	for (unsigned int i = 0; i < MAX_COUNTER; i++)
	{
		if (m_counters[i].stuffId == stuffId)
			return &m_counters[i];
	}
	return NULL;
}

// 005e6010
void sQuestCount983::Decode(WReceivedPacket* p)
{
	bValid = p->Decode1() != 0;
	typeId = p->Decode4();
	uid = p->Decode4();
	count = p->Decode4();
}

// 005e6050
bool _quest::IsDailyQuestType983(unsigned long typeId)
{
	if ((typeId & 0xfc000000) != 0x78000000)
		return false;
	return ((typeId >> 22) & 0xf) == 2;
}

// 005e6080
CQuestItem::CQuestItem()
	: m_bActive(false), m_typeId(0), m_uid(0), m_state(0)
{
}

// 005e6190
CQuestItem::CQuestItem(unsigned long typeId, unsigned long uid)
{
	m_typeId = typeId;
	m_uid = uid;
	m_state = 1;
	m_bActive = true;
}

// 005e62b0
void CQuestItem::Decode983(WReceivedPacket* p)
{
	m_bActive = p->Decode1() != 0;
	m_typeId = p->Decode4();
	m_uid = p->Decode4();
	m_state = p->Decode4();
	unsigned long n = p->Decode4();
	for (unsigned int i = 0; i < n; i++)
	{
		sQuestCounter983* c = GetCounter983(i);
		if (c)
		{
			c->stuffId = p->Decode4();
			c->typeId = p->Decode4();
			c->uid = p->Decode4();
			c->doneTime = p->Decode4();
		}
	}
}

// 005e6350
void CQuestItem::MergeDone983(const CQuestItem* other)
{
	m_state = other->GetState983();
	for (int i = 0; i < MAX_COUNTER; i++)
	{
		const sQuestCounter983& src = other->m_counters[i];
		if (src.doneTime != 0)
		{
			sQuestCounter983* c = FindCounter983(src.stuffId);
			if (c)
				c->doneTime = src.doneTime;
		}
	}
}

// 005e63b0
void CQuestItem::Accept983()
{
	if (IsDailyQuestType983(GetTypeId983()) == true)
	{
		m_state = 3;
		for (int i = 0; i < MAX_COUNTER; i++)
		{
			if (m_counters[i].stuffId != 0)
			{
				sQuestCount983 count;
				count.bValid = true;
				count.typeId = m_counters[i].typeId;
				count.uid = m_counters[i].uid;
				count.count = 0;
				Doc()->AddQuestCount983(count);
			}
		}
		return;
	}
	m_state = 5;
	m_bActive = false;
}

// 005e6420
long CQuestItem::GetCounterGoal983(const sQuestCounter983* c)
{
	const IFF983::sQuestStuff* pStuff = ItemManager()->FindQuestStuff983(c->stuffId);
	if (!pStuff)
		return 0;
	return QUESTSTUFF_GOAL983(pStuff);
}

// 005e6680
long CQuestItem::GetCounterCount983(const sQuestCounter983* c)
{
	if (c->uid != 0)
	{
		std::list<sQuestCount983>& counts = Doc()->m_questCounts983;
		std::list<sQuestCount983>::iterator it = counts.begin();
		for (; it != counts.end(); ++it)
		{
			if (c->uid == (*it).uid)
				break;
		}
		if (it != counts.end() && (*it).bValid == true)
			return (*it).count;
	}
	return 0;
}

// 005e6780
float CQuestItem::GetProgress983(unsigned int index)
{
	sQuestCounter983* c = GetCounter983(index);
	if (c && c->typeId != 0)
	{
		long count = GetCounterCount983(c);
		long goal = GetCounterGoal983(c);
		float value = (float)((double)count / goal * 100.0);
		return (float)floor(value);
	}
	return 0.0f;
}

// 005e6800
float CQuestItem::GetProgress10_983(unsigned int index)
{
	sQuestCounter983* c = GetCounter983(index);
	if (c && c->typeId != 0)
	{
		long count = GetCounterCount983(c);
		float value = (float)((double)(count % 10) / 10.0 * 100.0);
		return (float)floor(value);
	}
	return 0.0f;
}

// 005e66d0: missão diária aceita — "[contagem/meta]" de cada contador usado
void CQuestItem::AppendCounts983(std::stringstream* pStream)
{
	if (m_state > 1)
	{
		*(std::ostream*)pStream << "\n\t";
		for (unsigned i = 0; i < MAX_COUNTER; ++i)
		{
			sQuestCounter983* c = GetCounter983(i);
			if (c && c->typeId)
			{
				int count = GetCounterCount983(c);
				int goal = GetCounterGoal983(c);
				*(std::ostream*)pStream << "[" << count << "/" << goal << "]";
			}
		}
	}
}

// 005e6870: conquista — "[contagem/meta]" de cada contador; cumpridos com "- (완료)", nome, contagem e data; os que faltam com
// "- (진행)" (só o primeiro quando a conquista é em ordem, Achievement.iff +0xc0). Estado CONDITION: as contagens primeiro.
void CQuestItem::AppendCounters983(std::stringstream* pStream)
{
	const IFF983::sAchievement* pIff = Doc()->m_itemManager.FindAchievement983(GetTypeId983());
	if (!pIff)
		return;
	bool bPending = false;
	bool bDone = false;
	std::stringstream pending;
	std::stringstream counts;
	std::stringstream done;
	if (m_state > 1)
	{
		for (unsigned i = 0; i < MAX_COUNTER; ++i)
		{
			sQuestCounter983* c = GetCounter983(i);
			if (!c || !c->typeId)
				continue;
			int count = GetCounterCount983(c);
			int goal = GetCounterGoal983(c);
			counts << "[" << count << "/" << goal << "]";
			if (c->doneTime != 0)
			{
				bDone = true;
				SYSTEMTIME st;
				S5::SecToSysTime983(&st, c->doneTime);
				char date[32];
				sprintf(date, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
				const char* name = Doc()->m_itemManager.GetItemName(c->stuffId);
				done << "\n\t" << "- (\xbf\xcf\xb7\xe1)" << "'" << name << "'" << " " << "[" << count << "/" << goal << "]" << " "
					<< "DATE(" << date << ")";
			}
			else if (!AchievementView983(pIff)->lastStuffId983 || !bPending)
			{
				bPending = true;
				const char* name = Doc()->m_itemManager.GetItemName(c->stuffId);
				pending << "\n\t" << "- (\xc1\xf8\xc7\xe0)" << "'" << name << "'" << " " << "[" << count << "/" << goal << "]" << " ";
			}
		}
		if (m_state == 3)
			*(std::ostream*)pStream << "\n\t" << counts.str();
		if (bPending)
			*(std::ostream*)pStream << pending.str();
		if (bDone)
			*(std::ostream*)pStream << done.str();
	}
}

// 005e6f60: texto de depuração da missão — nome, TID, UID, estado e os contadores (conquista 0x13 / missão diária 0x1e)
std::string CQuestItem::ToString983()
{
	std::stringstream ss;
	const char* state = GetStateName983();
	ss << "'" << Doc()->m_itemManager.GetItemName(GetTypeId983()) << "'" << " " << "(" << "TID:" << GetTypeId983() << ", "
		<< "UID:" << GetUid983() << ", " << "STATUS:" << state << ")";
	unsigned long kind = GetTypeId983() >> 26;
	if (kind == 0x13)
		AppendCounters983(&ss);
	else if (kind == 0x1e)
		AppendCounts983(&ss);
	return ss.str();
}
