// JP 983: unidade 00542240–00542c70 (CGDSetAuxparsAbility, dados de jogo "SetAuxpartsStatus"). Classe nova do 983; segue o molde
// das CGD* do 645 (gdtextbox.cpp). O começo da faixa (00542240–005427b0) são cópias fora de linha: destrutor do IGameData,
// `Instance`, `Find` e o std::list<sRow>.
#include "minatl.h"
#include "gdsetauxparsability.h"

CGDSetAuxparsAbility* CGDSetAuxparsAbility::s_pInstance;

bool CGDSetAuxparsAbility::Find(unsigned long tidA, unsigned long tidB)
{
	std::list<sRow>::iterator it = m_rows.begin();
	while (it != m_rows.end())
	{
		m_cur = *it++;
		if ((m_cur.tid1 == tidA && m_cur.tid2 == tidB) || (m_cur.tid1 == tidB && m_cur.tid2 == tidA))
			return true;
	}
	return false;
}

CGDSetAuxparsAbility::CGDSetAuxparsAbility()
{
}

CGDSetAuxparsAbility::~CGDSetAuxparsAbility()
{
	s_pInstance = NULL;
}

int CGDSetAuxparsAbility::GetStatus(int type, const unsigned long* tids)
{
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			if (i != j && Find(tids[i], tids[j]))
			{
				switch (type)
				{
				case 0: return m_cur.pow;
				case 1: return m_cur.cnt;
				case 2: return m_cur.acc;
				case 3: return m_cur.spn;
				case 4: return m_cur.cur;
				}
			}
		}
	}
	return 0;
}

void CGDSetAuxparsAbility::Initialize(TiXmlNode* pNode)
{
	TiXmlNode* pChild = pNode->FirstChild("row");
	while (pChild)
	{
		sRow row;
		row.tid1 = atol(pChild->ToElement()->Attribute("tid1").c_str());
		row.tid2 = atol(pChild->ToElement()->Attribute("tid2").c_str());
		row.pow = atoi(pChild->ToElement()->Attribute("POW").c_str());
		row.cnt = atoi(pChild->ToElement()->Attribute("CNT").c_str());
		row.acc = atoi(pChild->ToElement()->Attribute("ACC").c_str());
		row.spn = atoi(pChild->ToElement()->Attribute("SPN").c_str());
		row.cur = atoi(pChild->ToElement()->Attribute("CUR").c_str());
		row.drvup = atoi(pChild->ToElement()->Attribute("DRVUP").c_str());
		m_rows.push_back(row);
		pChild = pChild->NextSibling("row");
	}
}
