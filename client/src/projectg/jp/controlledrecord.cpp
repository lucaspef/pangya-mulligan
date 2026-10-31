// JP 983: unidade 004b4330–004b4f50 (gravador de estado do CControlledObject; classe sem RTTI, nome CControlledRecord meu).
// Unidade separada do controlledobject.cpp: o construtor do CControlledObject guarda `this` em esi em volta da chamada a 004b4c40,
// o que o cl só faz quando não vê o corpo da função chamada.
#include "controlledobject.h"

void* CControlledRecord::Get(int index)
{
	if (index < 0)
		return NULL;
	if (!m_records.empty() && (int)m_records.size() > index)
		return m_records[index].get();
	return NULL;
}

void CControlledRecord::Commit()
{
	for (std::vector<std::pair<size_t, void*> >::iterator it = m_fields.begin(); it != m_fields.end(); ++it)
		m_recordSize += it->first;
}

void CControlledRecord::Write(FILE* fp)
{
	int count = (int)m_records.size();
	fwrite(&count, sizeof(count), 1, fp);
	fwrite(&m_recordSize, sizeof(m_recordSize), 1, fp);
	for (std::vector<std::unique_ptr<char[]> >::iterator it = m_records.begin(); it != m_records.end(); ++it)
		fwrite(it->get(), m_recordSize, 1, fp);
}

CControlledRecord::CControlledRecord()
	: m_recordSize(0)
{
}

void CControlledRecord::Clear()
{
	m_records.clear();
}

void CControlledRecord::Snapshot()
{
	if (m_fields.empty())
		return;
	int index = (int)m_records.size();
	m_records.push_back(std::unique_ptr<char[]>(new char[m_recordSize]));
	char* pRecord = m_records[index].get();
	size_t offset = 0;
	for (std::vector<std::pair<size_t, void*> >::iterator it = m_fields.begin(); it != m_fields.end(); ++it)
	{
		memcpy_s(pRecord + offset, m_recordSize, it->second, it->first);
		offset += it->first;
	}
}

void CControlledRecord::Read(FILE* fp)
{
	int count = 0;
	int size = 0;
	fread(&count, sizeof(count), 1, fp);
	fread(&size, sizeof(size), 1, fp);
	for (int i = 0; i < count; ++i)
	{
		std::unique_ptr<char[]> p(new char[size]);
		m_records.emplace_back(std::move(p));
	}
	for (int i = 0; i < count; ++i)
		fread(m_records[i].get(), size, 1, fp);
}
