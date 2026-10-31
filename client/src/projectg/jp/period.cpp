// JP 983: unidade 00b3d360–00b3f030 (CPeriod, os períodos de evento; ver include/jp/period.h). Sessão B123.
#include "minatl.h"
#include "projectg.h"
#include "shareddoc.h"
#include "period.h"
#include <string.h>

// 00b3d360
SYSTEMTIME CPeriod::MakeTime983(WORD year, WORD month, WORD day, WORD hour, WORD minute)
{
	SYSTEMTIME t;
	memset(&t, 0, sizeof(t));
	t.wYear = year;
	t.wMonth = month;
	t.wDay = day;
	t.wHour = hour;
	t.wMinute = minute;
	return t;
}

// 00b3e730
CPeriod::~CPeriod()
{
	for (std::map<int, sPeriod983*>::iterator it = m_periods.begin(); it != m_periods.end(); ++it)
	{
		if (it->second)
			delete it->second;
	}
	m_periods.clear();
}

// 00b3e970
bool CPeriod::Add983(int id, SYSTEMTIME start, SYSTEMTIME end, bool bDaily)
{
	sPeriod983* p = new sPeriod983;
	p->start = start;
	p->end = end;
	p->bDaily = bDaily;
	m_periods[id] = p;
	return true;
}

// 00b3e9d0
CPeriod::CPeriod()
{
	Add983(0, MakeTime983(2010, 1, 1, 0, 0), MakeTime983(2010, 1, 2, 0, 0), false);
	Add983(1, MakeTime983(2010, 2, 14, 0, 0), MakeTime983(2010, 2, 15, 0, 0), false);
	Add983(2, MakeTime983(2010, 3, 14, 0, 0), MakeTime983(2010, 3, 18, 0, 0), false);
	Add983(3, MakeTime983(2010, 3, 17, 0, 0), MakeTime983(2010, 3, 18, 0, 0), false);
	Add983(4, MakeTime983(2010, 4, 1, 11, 0), MakeTime983(2010, 4, 2, 0, 0), false);
	Add983(0x15, MakeTime983(2010, 4, 1, 11, 0), MakeTime983(2010, 4, 8, 0, 0), false);
	Add983(0x16, MakeTime983(2011, 8, 12, 0, 0), MakeTime983(2011, 8, 14, 0, 0), false);
	Add983(0x17, MakeTime983(2011, 8, 4, 20, 0), MakeTime983(2011, 8, 25, 23, 59), true);
	Add983(0xd, MakeTime983(2010, 3, 18, 21, 20), MakeTime983(2010, 3, 18, 21, 52), false);
	Add983(0x18, MakeTime983(2012, 5, 2, 11, 0), MakeTime983(2012, 5, 31, 11, 0), false);
}

// 00b3e5a0
CPeriod::sPeriod983* CPeriod::Find983(int id)
{
	sPeriod983* p = NULL;
	std::map<int, sPeriod983*>::iterator it = m_periods.find(id);
	if (it != m_periods.end())
		p = it->second;
	return p;
}

// 00b3e5e0
bool CPeriod::Remove983(int id)
{
	std::map<int, sPeriod983*>::iterator it = m_periods.find(id);
	if (it == m_periods.end())
		return false;
	if (it->second)
	{
		delete it->second;
		it->second = NULL;
	}
	m_periods.erase(it);
	return true;
}

// 00b3eef0: com bDaily, depois das datas confere só as horas (o dia de start e de end vira o de hoje)
bool CPeriod::IsInPeriod983(int id)
{
	sPeriod983* p = Instance().Find983(id);
	SYSTEMTIME start = p->start;
	SYSTEMTIME end = p->end;
	SYSTEMTIME now = Doc()->GetServerTime();
	if (p->bDaily)
	{
		if (!ItemManager()->IsBetween(&start, &end, now))
			return false;
		start.wDay = now.wDay;
		end.wDay = now.wDay;
	}
	return ItemManager()->IsBetween(&start, &end, now) ? true : false;
}
