// JP 983: unidade 005a2940–005a6fd0 (CMotionAdvisor). O mapa da F3.7.1 juntava esta faixa com a CLSLoginAckPostProcessor
// (005a11f0–005a2940, escrita pela frente C no lobbytask.cpp): nenhuma função do login chama a faixa. O grosso dela são
// instâncias do std::map<unsigned int, std::map<std::string, sMotion> > e do std::vector<std::string>.
#include <algorithm>
#include "motionadvisor.h"

// 005a6370
void CMotionAdvisor::Clear()
{
	std::for_each(m_motions.begin(), m_motions.end(), [](MotionMap::value_type& motion) { motion.second.clear(); });
	m_motions.clear();
}

// 005a6530
bool CMotionAdvisor::Reset()
{
	Clear();
	return true;
}

// 005a6540
const char* CMotionAdvisor::GetMotionName(unsigned int key, int part, const char* name)
{
	NameMap& motions = m_motions[key];
	NameMap::iterator it = motions.find(name);
	if (it == motions.end())
		return NULL;

	std::vector<int>& parts = it->second.parts;
	unsigned int count = parts.size();
	for (unsigned int i = 0; i < count; ++i)
	{
		if (parts[i] == part)
		{
			if (it->second.names[1].size() == 0)
				break;
			return it->second.names[1].c_str();
		}
	}
	return name;
}

// 005a6670
void CMotionAdvisor::AddPart(unsigned int key, unsigned long part)
{
	MotionMap::iterator found = m_motions.find(key);
	if (found != m_motions.end() && found->second.size() != 0)
	{
		for (NameMap::iterator it = found->second.begin(); it != found->second.end(); ++it)
			it->second.parts.push_back(part);
		return;
	}

	NameMap empty;
	m_motions.insert(MotionMap::value_type(key, empty));
	NameMap& motions = m_motions[key];
	for (unsigned int i = 0; i < m_names.size(); ++i)
	{
		sMotion motion;
		motion.parts.push_back(part);
		motion.names[0] = m_names[i].c_str();
		motions.insert(NameMap::value_type(m_names[i], motion));
	}
}

// 005a6ba0
void CMotionAdvisor::SetMotionName(unsigned int key, const char* name, const char* newName)
{
	NameMap& motions = m_motions[key];
	NameMap::iterator it = motions.find(name);
	if (it != motions.end())
		it->second.names[1] = newName;
}

// 005a6d80
CMotionAdvisor::~CMotionAdvisor()
{
}

// 005a6e90
CMotionAdvisor::CMotionAdvisor()
{
	m_names.push_back("\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc"); // "기본자세" (CP949)
}
