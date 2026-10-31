// JP 983: unidade 00486700–00488d60, logo antes da cardcomposedlg.cpp: regras da alquimia de cartas (CCardComposeRule983) e o
// cache de figuras das casas (CCardOverlayCache983), mais os moldes de std::map<std::string, WOverlay*> que eles usam. É outro
// .cpp porque o construtor e o destrutor vazios da regra (00486700/00486710) são CHAMADOS pela FrCardComposeDlg (0048e400/
// 0048dd20): na mesma unidade o cl elimina chamadas a funções vazias (sonda build/B/s93/pr/p.cpp, sessão B93). As cópias soltas
// 00488cf0 (construtor do sCardComposeItem983), 00488d20 e 00488d40 (inline da visão do ntAvatarManager) também caem aqui,
// sem chamador. Nomes meus (sessão B93); ver include/jp/cardcomposedlg.h.
#include "minatl.h"
#include "projectg.h"
#include <time.h>
#include <algorithm>
#include "wresrcmng.h"
#include "woverlay.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "../../shared/itemmanager.h"
#include "cardcomposedlg.h"

// 00486700
CCardComposeRule983::CCardComposeRule983()
{
}

// 00486710
CCardComposeRule983::~CCardComposeRule983()
{
}

// 00486720
bool CCardComposeRule983::IsFull(const int& count)
{
	if (count == 3)
		return true;
	return false;
}

// 00486730
bool CCardComposeRule983::HasCards(const unsigned long* tids)
{
	for (int i = 0; i < 3; ++i)
	{
		if (!tids)
			return false;
	}
	return true;
}

// 00486750
bool CCardComposeRule983::NoSpecialType3(const unsigned long* tids)
{
	for (int i = 0; i < 3; ++i)
	{
		if ((tids[i] & 0x3c00000) == 0xc00000)
			return false;
	}
	return true;
}

// 00486790
bool CCardComposeRule983::NoSpecialType4(const unsigned long* tids)
{
	for (int i = 0; i < 3; ++i)
	{
		if ((tids[i] & 0x3c00000) == 0x1000000)
			return false;
	}
	return true;
}

// 004867d0
void CCardComposeRule983::Release983(sCardComposeSel983* pSel)
{
	if (g_resrcmng && pSel->m_pRes)
	{
		g_resrcmng->Release(pSel->m_pRes);
		pSel->m_pRes = NULL;
	}
}

// 00486f70
void CCardComposeRule983::Send983(__int64 price, const unsigned long* tids)
{
	WSendPacket packet((enumClientPacket)0x155);
	packet.Encode8(price);
	for (int i = 0; i < 3; ++i)
		packet.Encode4(tids[i]);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// 00487050
bool CCardComposeRule983::HasPang(const __int64& price)
{
	if (Doc()->m_myInfo.stat.i64Pang < price)
		return false;
	return true;
}

// 00487080
bool CCardComposeRule983::NoRare3(const unsigned long* tids)
{
	for (int i = 0; i < 3; ++i)
	{
		if (ItemManager()->FindCard(tids[i])->RareType == 3)
			return false;
	}
	return true;
}

// 004870d0
__int64 CCardComposeRule983::CalcPrice(const unsigned long* tids)
{
	__int64 price = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (tids[i])
		{
			switch (ItemManager()->FindCard(tids[i])->RareType)
			{
			case 0:
				price += 1000;
				break;
			case 1:
				price += 2000;
				break;
			case 2:
				price += 5000;
				break;
			}
		}
	}
	return price;
}

// 004877f0: cada typeId conta as repetidas da escolha; o inventário precisa ter pelo menos essas
bool CCardComposeRule983::HasStock(const unsigned long* tids)
{
	int count[3] = { 0, 0, 0 };
	for (int i = 0; i < 3; ++i)
	{
		if (count[i] == -1)
			continue;
		for (int j = i + 1; j < 3; ++j)
		{
			if (tids[i] == tids[j])
			{
				count[j] = -1;
				++count[i];
			}
		}
		std::list<sCards>& cards = Doc()->m_cardList;
		for (std::list<sCards>::iterator it = cards.begin(); it != cards.end(); ++it)
		{
			if (it->tid == tids[i])
			{
				++count[i];
				if (it->cardCount < count[i])
					return false;
			}
		}
	}
	return true;
}

// 00487960
bool CCardComposeRule983::CanCompose(const int& count, const __int64& price, const unsigned long* tids)
{
	if (IsFull(count) && HasPang(price) && HasCards(tids) && NoRare3(tids) && NoSpecialType3(tids) && NoSpecialType4(tids)
		&& HasStock(tids))
		return true;
	return false;
}

// 00488920
CCardOverlayCache983::CCardOverlayCache983()
{
}

// Solta a figura de uma entrada do mapa (00487220 = o std::for_each com ele embutido)
struct sReleaseOverlay983
{
	void operator()(std::pair<const std::string, WOverlay*>& p) const
	{
		if (g_resrcmng && p.second)
		{
			g_resrcmng->Release(p.second);
			p.second = NULL;
		}
	}
};

// 004885d0
void CCardOverlayCache983::Clear983()
{
	std::for_each(m_map.begin(), m_map.end(), sReleaseOverlay983());
	m_map.clear();
}

// 004889f0: entrada vazia ou a de hora mais antiga
void CCardOverlayCache983::Add983(const char* name, WOverlay* pOverlay)
{
	int oldest = 0;
	__time64_t t = _time64(NULL);
	for (int i = 0; i < 20; ++i)
	{
		if (m_entry[i].time == 0)
		{
			oldest = i;
			break;
		}
		if (t > m_entry[i].time)
		{
			oldest = i;
			t = m_entry[i].time;
		}
	}
	sEntry& e = m_entry[oldest];
	if (e.time != 0)
	{
		WOverlay*& pOld = m_map[e.name];
		if (pOld)
			delete pOld;
		m_map.erase(e.name);
	}
	e.name = name;
	e.time = _time64(NULL);
	m_map[std::string(name)] = pOverlay;
}

// 00488b60
WOverlay* CCardOverlayCache983::Get983(const char* name)
{
	WOverlay* p = m_map[std::string(name)];
	if (p)
	{
		for (int i = 0; i < 20; ++i)
		{
			if (m_entry[i].name == name)
			{
				m_entry[i].time = _time64(NULL);
				break;
			}
		}
	}
	else
	{
		p = g_resrcmng->GetOverlay(name, 0);
		if (p)
			Add983(name, p);
	}
	return p;
}
