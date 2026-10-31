#include "minatl.h"
#include "bonusticket.h"

CBonusTicket& CBonusTicket::Instance()
{
	static CBonusTicket s_instance;

	return s_instance;
}

CBonusTicket::CBonusTicket()
{
}

CBonusTicket::~CBonusTicket()
{
	Release();
}

void CBonusTicket::Release()
{
	std::map<std::string, sTicketTable*>::iterator it = m_ticketTable.begin();
	while (it != m_ticketTable.end())
	{
		if (it->second)
		{
			delete it->second;
			it->second = NULL;
			m_ticketTable.erase(it++);
		}
		else
			++it;
	}

	m_ticketTable.clear();
}

void CBonusTicket::DeleteTable(std::string name)
{
	std::map<std::string, sTicketTable*>::iterator it =
		m_ticketTable.find(name);
	if (it != m_ticketTable.end())
	{
		if (it->second)
		{
			delete it->second;
			it->second = NULL;
			m_ticketTable.erase(it);
		}
	}
}

void CBonusTicket::SetTicketFromServer(std::string name, unsigned char* ticket,
	unsigned int size)
{
	if (size == sTicketTable::MAX_TICKET)
	{
		DeleteTable(name);
		sTicketTable* table = new sTicketTable;
		memcpy(table->ticket, ticket, sTicketTable::MAX_TICKET);
		m_ticketTable[name] = table;
	}
}

void __cdecl CBonusTicket::SetTicketFromClient(std::string name, int num, ...)
{
	DeleteTable(name);

	std::vector<int> rate;
	rate.reserve(num);

	va_list args;
	va_start(args, num);

	int sum = 0;
	for (int i = 0; i < num; ++i)
	{
		int r = va_arg(args, int);
		sum += r;
		rate.push_back(r);
	}

	va_end(args);

	if (sum > sTicketTable::MAX_TICKET)
		rate.clear();

	sTicketTable* table = new sTicketTable;
	SetRandomTicket(*table, rate);
	m_ticketTable[name] = table;
}

void CBonusTicket::SetRandomTicket(sTicketTable& table, std::vector<int>& rate)
{
	int pos = 0;
	int idx = 0;

	for (std::vector<int>::iterator it = rate.begin(); it != rate.end(); ++it)
	{
		int count = *it;
		for (int j = 0; j < count; ++j)
			table.ticket[pos++] = idx;

		++idx;
	}

	for (int i = 0; i < sTicketTable::MAX_TICKET; ++i)
	{
		int r = rand() % sTicketTable::MAX_TICKET;
		std::swap(table.ticket[i], table.ticket[r]);
	}
}

int CBonusTicket::GetRandomTicket(std::string name)
{
	std::map<std::string, sTicketTable*>::iterator it =
		m_ticketTable.find(name);
	if (it != m_ticketTable.end())
	{
		sTicketTable* table = it->second;
		return table->GetTicket();
	}

	return 0;
}
