#include "stdafx.h"
#include "itemcursor983.h"
#include "artifactcheck983.h"
#include "shareddoc.h"
#include "../shared/localize.h"

// A594: unidade propria como no 983 (00478cc2-0047a060, longe do FrChangeRoomInfoDlg 00ac9xxx); no mesmo .cpp o cl via o
// corpo do ~CItemCursor983 e tirava o frame de excecao do ~FrChangeRoomInfoDlg (00ac93f0).

// ---- A s159: unidade 00478cc2-0047a060 do 983 (lista de artefatos com cursor; sem dono no mapa) ----
// Os dois leitores do mapa ArtifactManaInfo do CItemManager sao metodos fora de linha no 983 (itemmanager.cpp); aqui ficam
// locais para nao mexer no itemmanager.h (pch).
// 983 00a23780 (CItemManager): tid (+0x04 do registro) de cada artefato da tabela, na ordem do mapa
struct sArtifactMgr983 // thiscall com o CItemManager em ecx, como os metodos do 983
{
	CItemManager* Mgr() { return (CItemManager*)this; }
	__declspec(noinline) void GetArtifactTids983(std::vector<unsigned long>& tids)
	{
		std::map<unsigned int, IFF983::sArtifactManaInfo>::iterator it;
		for (it = Mgr()->m_ArtifactManaInfoMap983.begin(); it != Mgr()->m_ArtifactManaInfoMap983.end(); ++it)
			tids.push_back(*(unsigned long*)((char*)&(*it).second + 0x04));
	}

	// 983 00a192e0 (CItemManager): dword +0x08 do registro do artefato (o item que ele pede; 0 = nao achou)
	__declspec(noinline) unsigned long GetArtifactPairTid983(unsigned long tid)
	{
		std::map<unsigned int, IFF983::sArtifactManaInfo>::iterator it = Mgr()->m_ArtifactManaInfoMap983.find(tid);
		if (it == Mgr()->m_ArtifactManaInfoMap983.end())
			return 0;
		return *(unsigned long*)((char*)&(*it).second + 0x08);
	}
};

// 983 006336a0 (CSharedDoc, sem nome): item escondido do jogador (tipo 6 com subtipo 0x1e sempre; 0x1f fora dos bits
// 0x2804 da identidade; senao CItemManager::IsNonVisibleItem)
struct sHiddenDoc983 // thiscall com o CSharedDoc em ecx
{
	__declspec(noinline) bool IsHiddenItem983(unsigned long tid)
	{
		if ((tid >> 26) == 6)
		{
			if ((tid & 0x1f00000) == 0x1e00000)
				return true;
			if ((tid & 0x1f00000) == 0x1f00000)
				return (((CSharedDoc*)this)->m_myInfo.info.dwIdentity & 0x2804) == 0;
		}
		return ItemManager()->IsNonVisibleItem(tid) == true;
	}
};

__forceinline bool HasItem983(std::list<sItemInfo>* pList, unsigned long tid)
{
	for (std::list<sItemInfo>::iterator it = pList->begin(); it != pList->end(); ++it)
		if ((*it).tid == tid)
			return true;
	return false;
}

// 983 004797a0: list() do VC10 (_Buynode da cabeca sem valor construido)
__declspec(noinline) sArtifactList983::sArtifactList983()
{
	m_size = 0;
	m_pHead = std::_Allocate((size_t)1, (sArtifactNode983*)0);
	m_pHead->pNext = m_pHead;
	m_pHead->pPrev = m_pHead;
}

// 983 00479f40
CItemCursor983::CItemCursor983(void* pList)
{
	m_pCur = NULL;
	Fill983(pList, true);
	m_pCur = m_pHead->pNext;
}

// clear() do std::list do VC10 (inline no Clear983 e duas vezes no destrutor)
static __forceinline void ClearNodes983(sArtifactList983* c)
{
	sArtifactNode983* p = c->m_pHead->pNext;
	c->m_pHead->pNext = c->m_pHead;
	c->m_pHead->pPrev = c->m_pHead;
	c->m_size = 0;
	while (p != c->m_pHead)
	{
		sArtifactNode983* next = p->pNext;
		operator delete(p);
		p = next;
	}
}

void CItemCursor983::Clear983()
{
	ClearNodes983(this);
}

// 983 00479800: ~list do VC10 (clear() do corpo + _Tidy: clear() de novo e a cabeca)
__forceinline sArtifactList983::~sArtifactList983()
{
	ClearNodes983(this);
	ClearNodes983(this);
	operator delete(m_pHead);
}

CItemCursor983::~CItemCursor983()
{
}

// 983 00478f40 (chamado por 0061af80)
void CItemCursor983::Reload983(void* pList)
{
	ClearNodes983(this); // clear() em linha
	Fill983(pList, false);
}

// 983 004795d0 (_Buynode do VC10; o _Cons_val 00479470 com o alocador do valor em +9)
__declspec(noinline) sArtifactNode983* CItemCursor983::Buynode983(sArtifactNode983* pNext, sArtifactNode983* pPrev,
	const sArtifactItem983& v)
{
	sArtifactNode983* node = std::_Allocate((size_t)1, (sArtifactNode983*)0);
	node->pNext = pNext;
	node->pPrev = pPrev;
	std::_Cons_val(*(std::allocator<sArtifactItem983>*)((char*)this + 9), &node->value, v);
	return node;
}

// push_back do VC10 (_Buynode + _Incsize); fora de linha em 00479870, em linha no Fill983 (004798c0)
inline void CItemCursor983::PushBackInline983(const sArtifactItem983& v)
{
	sArtifactNode983* where = m_pHead; // _Insert(end(), v)
	sArtifactNode983* node = Buynode983(where, where->pPrev, v);
	if (0x14cab88 - (size_t)m_size - 1 < 1)
		std::_Xlength_error("list<T> too long");
	m_size++;
	where->pPrev = node;
	node->pPrev->pNext = node;
}

// 983 00479870
void CItemCursor983::PushBack983(const sArtifactItem983& v)
{
	PushBackInline983(v);
}

// 983 004798c0: com bNone, primeiro a entrada "nenhum" (tid 0, usavel). Depois, para cada artefato da tabela que o
// item marca como artefato (bit 1 de +0xc0): os do jogador (estao na lista do CSharedDoc e nao escondidos) com a copia do
// item e "usavel" = o jogador tem o item pedido (00a192e0) e ele tem o bit 2; os livres (bit 3) so para identidade 0x2000.
#pragma strict_gs_check(push, on) // o 983 tem o cookie de buffer neste quadro
void CItemCursor983::Fill983(void* pList, bool bNone)
{
	std::list<sItemInfo>* pItems = (std::list<sItemInfo>*)pList;
	if (bNone)
	{
		sItemInfo none;
		none.tid = 0;
		PushBackInline983(sArtifactItem983(none, true));
	}

	std::vector<unsigned long> tids;
	((sArtifactMgr983*)ItemManager())->GetArtifactTids983(tids);
	for (std::vector<unsigned long>::iterator it = tids.begin(); it != tids.end(); ++it)
	{
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(*it);
		if (pItem == NULL)
			continue;
		unsigned long flags = *(unsigned long*)((char*)pItem + 0xc0);
		if (((flags >> 1) & 1) == 0)
			continue;
		if ((flags >> 3) & 1)
		{
			if ((Doc()->m_myInfo.info.dwIdentity & 0x2000) == 0)
				continue;
			sItemInfo free;
			free.tid = *(unsigned long*)((char*)pItem + 0x04);
			PushBackInline983(sArtifactItem983(free, true));
		}
		else
		{
			if (((sHiddenDoc983*)Doc())->IsHiddenItem983(*it))
				continue;
			std::list<sItemInfo>::iterator own;
			for (own = pItems->begin(); own != pItems->end(); ++own)
				if ((*own).tid == *it)
					break;
			if (own == pItems->end())
				continue;
			unsigned long pair = ((sArtifactMgr983*)ItemManager())->GetArtifactPairTid983(*it);
			bool bUsable = HasItem983(pItems, pair);
			IFF_STRUCT::sItem* pPair = ItemManager()->FindItem(pair);
			if (pPair == NULL || ((*(unsigned long*)((char*)pPair + 0xc0) >> 2) & 1) == 0)
				bUsable = false;
			PushBack983(sArtifactItem983(*own, bUsable));
		}
	}
}

#pragma strict_gs_check(pop)

// 983 00479e90
unsigned long* CItemCursor983::GetCur()
{
	return &m_pCur->value.item.tid;
}

// 983 00479fa0
bool CItemCursor983::Find(const unsigned long* pTid)
{
	// std::find_if em linha: procura e depois confere contra end()
	sArtifactNode983* pHead = m_pHead;
	sArtifactNode983* p = pHead->pNext;
	for (; p != pHead; p = p->pNext)
		if (p->value.item.tid == *pTid)
			break;
	if (p == pHead)
		return false;
	m_pCur = p;
	return true;
}

// 983 00479ee0/00479ef0/00479fd0/00479ff0: iterador (lista circular com sentinela m_pHead),
// funcoes proprias no 983 chamadas por FrMakeRoomDlg/FrChangeRoomInfoDlg
bool CItemCursor983::CanPrev983()
{
	return m_pCur != m_pHead->pNext;
}

bool CItemCursor983::CanNext983()
{
	return m_pCur->pNext != m_pHead;
}

bool CItemCursor983::StepPrev983()
{
	if (!CanPrev983())
		return false;
	m_pCur = m_pCur->pPrev;
	return m_pCur != m_pHead->pNext;
}

bool CItemCursor983::StepNext983()
{
	if (!CanNext983())
		return false;
	m_pCur = m_pCur->pNext;
	return m_pCur->pNext != m_pHead;
}

// 983 00479ea0
unsigned char CItemCursor983::GetFlag()
{
	return m_pCur->value.bUsable;
}

// 983 00479bc0
sArtifactCheck983::sArtifactCheck983(const unsigned long* pTid, const sArtifactCheckArg983* pArg)
{
	b0 = true;
	bFail = true;
	// "As condicoes nao batem; nao da para usar o artefato escolhido."
	text = K2L_Compatibility("\xc1\xb6\xb0\xc7\xc0\xcc \xb8\xc2\xc1\xf6\xbe\xca\xbe\xc6 \xbc\xb3\xc1\xa4\xb5\xc8 \xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
	name = "mana_limited";
	int value = ItemManager()->GetArtifactValue983(*pTid);
	if (*pTid != 0)
	{
		switch (*pArg->pRoomType)
		{
		case 0:
		case 1:
		case 5:
		case 6:
			if (*pArg->pMode == 0xff)
				goto mode_ok;
			break;
		case 4:
			{
				unsigned char mode = *pArg->pMode;
				if ((mode >= 0x14 && mode <= 0x15) || mode == 0xff)
					goto mode_ok;
			}
			break;
		}
		// "Modo de jogo em que nao se pode usar artefato."
		text = K2L_Compatibility("\xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb0\xd4\xc0\xd3\xb8\xf0\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9.");
		return;
mode_ok:
		if (value == -1)
			return;
	}
	if (*pArg->pFlag == 0)
	{
		// "Falta mana ao artefato."
		text = K2L_Compatibility("\xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae\xc0\xc7 \xb8\xb6\xb7\xc2\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
		name = "mana_no";
		return;
	}
	b0 = false;
	bFail = false;
	if (value == 3)
	{
		if (*pArg->pRoomType != 4)
			bFail = true;
	}
	else if (value == 1)
		bFail = *pArg->p1094b != 0x12;
}
