// JP 983: unidade grandprixdocument (0054f480–00559cc0): _gateway::_grandprix::CGrandPrixDocument, o documento 0xaf do CContentsDoc
// que guarda as linhas da lista do Grand Prix (uma por registro do GrandPrixData.iff), a lista e os pares que o servidor manda, os
// premiados e o aviso de abertura (CGPRoomTimerSteward). Classe nova do 983, sem par no 645. Sessão B68. O resto da faixa (≈ 640
// funções) são instâncias da STL que vêm sozinhas com o uso. Ver include/jp/grandprixdocument.h.
#include "minatl.h"
#include "projectg.h"
#include "grandprixdocument.h"
#include "gproomtimersteward.h"
#include "shareddoc.h"
#include "actor.h"
#include "taskmanager.h"
#include "commonutil.h"
#include "packet.h"
#include "wtimer.h"
#include "../../shared/itemmanager.h"
#include <algorithm>

using namespace _gateway;
using namespace _gateway::_grandprix;

// S5 (sharedutilities.cpp da frente A; sem nome no fonte do 645) — empréstimo com nome meu (pedido B67)
namespace S5
{
void SysTimeToSec983(const SYSTEMTIME* pTime, __int64* pOut); // 00b30830
}

// CTimerManager do 983 (frente A): AddTimer devolve o índice da vaga (-1 = cheio) e há uma remoção pelo índice
class CTimerManagerGpJp
{
public:
	int AddTimer983(float time, const char* name, CTimerEvent* event); // 0099cae0
	void RemoveTimer983(int id); // 0099cc60
	void ClearTimer(const char* name); // 0099cba0
	// D876/D957: lista e vagas pelos membros do CTimerManager (VC10 +0x28/+0x34, vaga 0x30; cl 14.44 +0x24/+0x2c, vaga 0x2c)
	typedef CTimerManager::sTimer sTimer983;
	static std::list<sTimer983*>& TimerList983(CTimerManagerGpJp* self) { return ((CTimerManager*)self)->m_timerList; }
	static sTimer983* TimerSlots983(CTimerManagerGpJp* self) { return ((CTimerManager*)self)->m_timers; }
};
#define TIMERMANAGERGP() ((CTimerManagerGpJp*)CTimerManager::Instance())

// layout do CTimerManager no 983 (desmonte 0099cae0/0099cc60): lista de ponteiros em +0x28 (tamanho em +0x2c), 10 vagas de 0x30 em +0x34
typedef CTimerManagerGpJp::sTimer983 sTimerGp983;
static std::list<sTimerGp983*>& TimerListGp(CTimerManagerGpJp* self) { return CTimerManagerGpJp::TimerList983(self); }
static sTimerGp983* TimerSlotsGp(CTimerManagerGpJp* self) { return CTimerManagerGpJp::TimerSlots983(self); }
C_ASSERT_VC10(sizeof(sTimerGp983) == 0x30);

// 0099cae0
int CTimerManagerGpJp::AddTimer983(float time, const char* name, CTimerEvent* event)
{
	sTimerGp983* slots = TimerSlotsGp(this);
	for (int i = 0; i < 10; ++i)
	{
		if (slots[i].active == true)
			continue;

		slots[i].active = true;
		slots[i].name.assign(name, strlen(name));
		slots[i].time = time;
		slots[i].event = event;
		slots[i].id = i;
		TimerListGp(this).push_back(&slots[i]);
		return i;
	}
	return -1;
}

// 0099cc60
void CTimerManagerGpJp::RemoveTimer983(int id)
{
	std::list<sTimerGp983*>& l = TimerListGp(this);
	for (std::list<sTimerGp983*>::iterator it = l.begin(); it != l.end(); ++it)
	{
		if ((*it)->id == id)
		{
			l.erase(it);
			return;
		}
	}
}

// 0099cba0 (B679): acha o primeiro timer da lista com esse nome, zera a vaga (tempo, decorrido, evento, nome, ativo) e o tira da lista
void CTimerManagerGpJp::ClearTimer(const char* name)
{
	std::list<sTimerGp983*>& l = TimerListGp(this);
	for (std::list<sTimerGp983*>::iterator it = l.begin(); it != l.end(); ++it)
	{
		sTimerGp983* t = *it;
		if (strcmp(t->name.c_str(), name) != 0)
			continue;

		t->time = 0.0f;
		t->elapsed = 0.0f;
		t->event = NULL;
		t->name.clear();
		t->active = false;
		l.erase(it);
		return;
	}
}

// CItemManager do 983 (frente A): taxa do Grand Prix pelo typeid (1, ou as constantes 00cfea84/00cfe8d4 pelo tipo de 00a19480)
class CItemManagerGpDocJp
{
public:
	float GetGrandPrixRate983(unsigned long typeId); // 00a194a0
};

// 00a194a0 (B661): byte +0x87 do registro de GrandPrixData (00a19480): 1 -> 2.0 (00cfea84), 2 -> 3.0 (00cfe8d4), senao 1.0
float CItemManagerGpDocJp::GetGrandPrixRate983(unsigned long typeId)
{
	char* pData = (char*)((CItemManager*)this)->FindGrandPrixData983(typeId);
	int kind = pData ? (unsigned char)pData[0x87] : 0;

	if (kind == 1)
		return 2.0f;
	if (kind == 2)
		return 3.0f;
	return 1.0f;
}

// campos do CSharedDoc (pedido B67/B68; antes crus 0x109c1/0x8cb/0x680a.., A741 -> membros p/ o cl novo)
#define DOC983_GPTYPEID() (Doc()->m_roomInfo.gpTypeId983) // typeid do Grand Prix da sala
#define DOC983_MYUID() (Doc()->m_myInfo.info.dwUID)
#define DOC983_MYA() (Doc()->m_userInfo[0].charInfo.hairClr) // dois bytes do personagem (vão para sGPAward983::a/b)
#define DOC983_MYB() (Doc()->m_userInfo[0].charInfo.shirtsClr)
#define DOC983_MYTIDS() (Doc()->m_userInfo[0].charInfo.tidParts) // 24 typeids das peças
#define DOC983_MYUIDS() (Doc()->m_userInfo[0].charInfo.ItemIdList) // 24 uids das peças
#define DOC983_MYBUILD() (Doc()->m_userInfo[0].charInfo.tidAuxParts) // 5 dwords
#define ROOM983_GPTYPEID(p) ((p)->gpTypeId983)
#define ROOM983_ISGP(p) (*(unsigned char*)&(p)->gpRoomFlags983 & 1)
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo) + offsetof(sRoomInfo, gpTypeId983) == 0x109c1 && offsetof(CSharedDoc, m_myInfo.info.dwUID) == 0x8cb);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, charInfo) == 0x6802 && offsetof(sCharacterInfo, tidParts) == 0xc);
C_ASSERT(offsetof(sCharacterInfo, ItemIdList) == 0x6c && offsetof(sCharacterInfo, tidAuxParts) == 0x1a4 && offsetof(sRoomInfo, gpRoomFlags983) == 0xd9);

static const char* s_lobby = "Lobby";

// 0054f480
float CGrandPrixDocument::GetScore983()
{
	return m_score;
}

// 0054f490
void CGrandPrixDocument::SetKind983(int kind)
{
	m_kind = kind;
}

// 0054f4a0
bool CGrandPrixDocument::Release()
{
	return true;
}

// 0054f4b0
bool CGrandPrixDocument::Reset()
{
	return true;
}

// 005525b0
void CGrandPrixDocument::RefreshTicket983()
{
	sItemInfo* pItem = Doc()->FindMyItemByTypeId983(0x1a000144);
	m_ticket = 0;
	if (pItem)
		m_ticket = pItem->Common[0];
}

// 005525e0
void CGrandPrixDocument::SetTypeId983(unsigned long typeId)
{
	if (m_timerId != -1)
		TIMERMANAGERGP()->RemoveTimer983(m_timerId);

	sGPInfo983* pInfo = (sGPInfo983*)Doc()->m_itemManager.FindGrandPrixData983(typeId);
	if (pInfo)
	{
		SYSTEMTIME now;
		CGrandPrixClock::Instance()->GetTime983(&now);
		SYSTEMTIME open = *(SYSTEMTIME*)pInfo->open;
		open.wYear = now.wYear;
		open.wMonth = now.wMonth;
		open.wDayOfWeek = now.wDayOfWeek;
		open.wDay = now.wDay;
		__int64 openSec;
		__int64 nowSec;
		S5::SysTimeToSec983(&open, &openSec);
		S5::SysTimeToSec983(&now, &nowSec);
		if (openSec > nowSec)
		{
			m_pSteward->m_typeId = typeId;
			m_timerId = TIMERMANAGERGP()->AddTimer983((float)(openSec - nowSec), MakeStr("%d", typeId), m_pSteward.get());
		}
	}
}

// 00552700
void CGrandPrixDocument::ClearTimer983(int)
{
	if (m_pSteward->m_typeId)
	{
		TIMERMANAGERGP()->ClearTimer(MakeStr("%d", m_pSteward->m_typeId));
		m_pSteward->m_typeId = 0;
	}
}

// 00552740
unsigned long CGrandPrixDocument::GetCurrentTypeId983()
{
	return m_pSteward->m_typeId;
}

// 00552750
void CGrandPrixDocument::Fn552750_983()
{
	float rate = ((CItemManagerGpDocJp*)&Doc()->m_itemManager)->GetGrandPrixRate983(DOC983_GPTYPEID());
	Doc()->m_courseScale983[2] = rate; // taxa do Grand Prix
}

// 00553dd0
sGPRoomItem983* CGrandPrixDocument::FindRoomItemByPrev983(unsigned long typeId)
{
	for (RoomMap::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		if (typeId == it->second.pInfo->prevTypeId)
			return &it->second;
	}
	return NULL;
}

// 00555070
bool CGrandPrixDocument::Initialize()
{
	const char* name = "CGrandPrixHandler";
	m_pHandler = (CGatewayHandler*)ActorRegistry983().Get(name);
	m_pSteward = std::unique_ptr<CGPRoomTimerSteward>(new CGPRoomTimerSteward());
	m_timerId = -1;
	m_score = 0.0f;
	m_ticket = 0;
	return true;
}

// 00555140
void CGrandPrixDocument::Fn555140_983()
{
	__int64 now = CGrandPrixClock::Instance()->GetNow983();
	for (RoomMap::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		if (it->second.pInfo->ticket & 0x3f80000)
			GPItemUpdateState983(&it->second, &now, GetCurrentTypeId983());
	}
}

// 005551f0
sGPRoomItem983* CGrandPrixDocument::FindRoomItem983(unsigned long typeId)
{
	RoomMap::iterator it = m_rooms.find(typeId);
	if (it != m_rooms.end())
		return &it->second;
	return NULL;
}

// 00555260
void CGrandPrixDocument::UpdateEnabled983()
{
	for (RoomMap::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		if (it->second.pInfo->listId)
			it->second.bEnabled = std::find(m_list.begin(), m_list.end(), it->second.pInfo->listId) != m_list.end();
	}
}

// 005552f0
void CGrandPrixDocument::UpdateKinds983()
{
	for (std::vector<KindPair>::iterator p = m_pairs.begin(); p != m_pairs.end(); ++p)
	{
		unsigned long key = p->first;
		for (RoomMap::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
		{
			if (it->second.pInfo->ticket == key)
				it->second.kind = p->second;
		}
	}
}

// 005564a0
void CGrandPrixDocument::Read5564a0_983(WReceivedPacket* pPacket)
{
	int n = pPacket->Decode4();
	m_list.clear();
	m_list.reserve(n);
	for (int i = 0; i < n; ++i)
	{
		int v = pPacket->Decode4();
		m_list.push_back(v);
	}
	AfxPostMsg(NULL, s_lobby, 0x352, 99, 0, 0, 0);
}

// 00556620
void CGrandPrixDocument::LinkRooms983(std::vector<sRoomInfo*>& rooms)
{
	std::vector<sRoomInfo*>::iterator end = rooms.end();
	for (std::vector<sRoomInfo*>::iterator p = rooms.begin(); p != end; ++p)
	{
		unsigned long typeId = ROOM983_GPTYPEID(*p);
		if (typeId)
		{
			RoomMap::iterator it = m_rooms.find(typeId);
			if (it != m_rooms.end())
				it->second.pRoom = *p;
		}
	}
}

// 005566b0
void CGrandPrixDocument::Fn5566b0_983(sRoomInfo* pRoom)
{
	RoomMap::iterator it = m_rooms.find(ROOM983_GPTYPEID(pRoom));
	if (it != m_rooms.end())
		it->second.pRoom = pRoom;
}

// 005566e0
void CGrandPrixDocument::Fn5566e0_983(sRoomInfo* pRoom)
{
	RoomMap::iterator it = m_rooms.find(ROOM983_GPTYPEID(pRoom));
	if (it != m_rooms.end())
		it->second.pRoom = NULL;
}

// 005580d0: pares (ticket, kind)
static void ReadPairs983(std::vector<CGrandPrixDocument::KindPair>& pairs, WReceivedPacket* pPacket)
{
	int n = pPacket->Decode4();
	pairs.clear();
	pairs.reserve(n);
	for (int i = 0; i < n; ++i)
	{
		CGrandPrixDocument::KindPair p;
		p.first = pPacket->Decode4();
		p.second = pPacket->Decode4();
		pairs.push_back(p);
	}
}

// 00558250
void CGrandPrixDocument::UpdatePair983(WReceivedPacket* pPacket)
{
	KindPair p;
	p.first = pPacket->Decode4();
	p.second = pPacket->Decode4();
	std::vector<KindPair>::iterator it = m_pairs.begin();
	for (; it != m_pairs.end(); ++it)
	{
		if (p.first == it->first)
			break;
	}
	if (it != m_pairs.end())
	{
		it->second = p.second;
		return;
	}
	m_pairs.push_back(KindPair(p));
}

// 005582b0
void CGrandPrixDocument::Read5582b0_983(WReceivedPacket* pPacket)
{
	m_awards.clear();
	unsigned char count = pPacket->Decode1();
	for (unsigned char i = count; i != 0; --i)
	{
		sGPAward983 award;
		award.uid = pPacket->Decode4();
		award.place = pPacket->Decode4();
		award.a = pPacket->Decode1();
		award.b = pPacket->Decode1();
		pPacket->DecodeBuffer(award.tids, sizeof(award.tids));
		pPacket->DecodeBuffer(award.buildUids, sizeof(award.buildUids));
		pPacket->DecodeBuffer(award.uids, sizeof(award.uids));
		award.bUser = 1;
		m_awards.push_back(award);
	}

	std::vector<int> picks;
	if (count < 3)
		AfxGetTask()->GetActor("AIPlayer") << MsgObject(NULL, 0x34b, (int)&picks, 0, 0, 0, 0);

	if ((unsigned char)picks.size() + count >= 3)
	{
		unsigned int next = 0;
		for (int place = 1; place <= 3; ++place)
		{
			std::vector<sGPAward983>::iterator it = m_awards.begin();
			for (; it != m_awards.end(); ++it)
			{
				if (place == it->place)
					break;
			}
			if (it == m_awards.end())
			{
				sGPAward983 award;
				award.uid = picks.at(next++);
				award.place = place;
				award.bUser = 0;
				award.a = 0;
				award.b = 0;
				memset(award.tids, 0, sizeof(award.tids));
				memset(award.buildUids, 0, sizeof(award.buildUids));
				memset(award.uids, 0, sizeof(award.uids));
				m_awards.push_back(award);
			}
		}
	}

	std::sort(m_awards.begin(), m_awards.end(),
		[](const sGPAward983& a, const sGPAward983& b) { return a.place < b.place; });

	std::vector<sGPAward983>::iterator it = m_awards.begin();
	for (; it != m_awards.end(); ++it)
	{
		if (DOC983_MYUID() == it->uid)
			break;
	}
	if (it == m_awards.end())
	{
		sGPAward983 award;
		award.uid = DOC983_MYUID();
		award.bUser = 1;
		award.place = 4;
		award.a = DOC983_MYA();
		award.b = DOC983_MYB();
		memcpy_s(award.tids, sizeof(award.tids), DOC983_MYTIDS(), sizeof(award.tids));
		memcpy_s(award.buildUids, sizeof(award.buildUids), DOC983_MYBUILD(), sizeof(award.buildUids));
		memcpy_s(award.uids, sizeof(award.uids), DOC983_MYUIDS(), sizeof(award.uids));
		m_awards.push_back(award);
	}
}

// 005587b0
float CGrandPrixDocument::GetClearRate983(int kind, unsigned long extra)
{
	int count = 0;
	std::map<int, bool> seen;
	for (RoomMap::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		if (kind == ((it->second.pInfo->ticket >> 19) & 0x7f))
		{
			int group = (it->second.pInfo->ticket >> 8) & 0x7ff;
			if (seen.find(group) == seen.end())
			{
				if (it->second.kind > 0)
				{
					++count;
					seen[group] = true;
				}
				else
					seen[group] = false;
			}
		}
	}
	if (extra)
	{
		int group = (extra >> 8) & 0x7ff;
		std::map<int, bool>::iterator f = seen.find(group);
		if (f == seen.end() || !f->second)
			++count;
	}
	if (seen.empty())
		return 0.0f;
	if (count > (int)seen.size())
		count = seen.size();
	return (float)count / (int)seen.size();
}

// 00558ef0
void CGrandPrixDocument::GetRoomItems983(std::vector<sGPRoomItem983*>* pOut, __int64* pNow)
{
	for (RoomMap::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		sGPRoomItem983* pItem = &it->second;
		if (pItem->bEnabled)
		{
			pItem->flags = 0;
			GPItemCheckLevel983(pItem);
			GPItemCheckScore983(pItem, m_score);
			GPItemCheckEquip983(pItem);
			GPItemCheckSpecialHole983(pItem);
			GPItemCheckRankReward983(pItem);
			GPItemBuildTitle983(pItem);
			if ((pItem->pInfo->ticket & 0x3f80000) == 0) // tipo (bits 19–25) 0 = sempre aberto
				pItem->state = 2;
			else
				GPItemUpdateState983(pItem, pNow, GetCurrentTypeId983());
			pOut->push_back(pItem);
		}
	}

	switch (m_kind)
	{
	case 0:
	{
		std::sort(pOut->begin(), pOut->end(),
			[](const sGPRoomItem983* a, const sGPRoomItem983* b) { return a->openTime < b->openTime; });
		unsigned int n = pOut->size();
		for (unsigned int i = 0; i < n; ++i)
		{
			sGPRoomItem983* pItem = (*pOut)[i];
			if (pItem->pInfo->prevTypeId == 0)
				pItem->opts &= ~4;
			else
			{
				std::vector<sGPRoomItem983*>::iterator p = pOut->begin();
				for (; p != pOut->end(); ++p)
				{
					if (pItem->pInfo->prevTypeId == (*p)->pInfo->ticket)
						break;
				}
				if (p != pOut->end() && (*p)->kind == 0)
					pItem->opts |= 4;
			}
		}
		break;
	}
	case 1:
	case 2:
	case 3:
		std::sort(pOut->begin(), pOut->end(),
			[](const sGPRoomItem983* a, const sGPRoomItem983* b) { return a->openTime < b->openTime; });
		break;
	case 99:
		std::sort(pOut->begin(), pOut->end(),
			[](const sGPRoomItem983* a, const sGPRoomItem983* b) { return a->openTime < b->openTime; });
		std::stable_sort(pOut->begin(), pOut->end(),
			[](const sGPRoomItem983* a, const sGPRoomItem983* b)
			{ return a->openTime == b->openTime && (a->pInfo->ticket & 0x7ff00) < (b->pInfo->ticket & 0x7ff00); });
		break;
	}
}

// 005592b0
void CGrandPrixDocument::ReadList983(WReceivedPacket* pPacket)
{
	m_list.clear();
	int n = pPacket->Decode4();
	for (int i = 0; i < n; ++i)
	{
		int v = pPacket->Decode4();
		m_list.push_back(v);
	}
	ReadPairs983(m_pairs, pPacket);
	pPacket->DecodeBuffer(&m_score, 4);
}

// 00559540
void CGrandPrixDocument::BuildRooms983()
{
	m_rooms.clear();
	std::map<unsigned int, IFF983::sGrandPrixData>& table = Doc()->m_itemManager.m_GrandPrixDataMap983;
	for (std::map<unsigned int, IFF983::sGrandPrixData>::iterator it = table.begin(); it != table.end(); ++it)
	{
		sGPInfo983* pInfo = (sGPInfo983*)&it->second;
		if (((pInfo->ticket >> 19) & 0x7f) == (unsigned long)m_kind)
		{
			sGPRoomItem983 item(pInfo);
			m_rooms.insert(std::make_pair(pInfo->typeId, item));
		}
	}
}

// 00559700
void CGrandPrixDocument::Open983(int kind)
{
	m_kind = kind;
	BuildRooms983();
	UpdateEnabled983();
	UpdateKinds983();
	std::vector<sRoomInfo*> rooms;
	std::list<sRoomInfo>::iterator end = Doc()->m_roomList.end();
	for (std::list<sRoomInfo>::iterator it = Doc()->m_roomList.begin(); it != end; ++it)
	{
		sRoomInfo* pRoom = &*it;
		if (ROOM983_ISGP(pRoom))
			rooms.push_back(pRoom);
	}
	if (!rooms.empty())
		LinkRooms983(rooms);
}

// 00559980
CGrandPrixDocument::~CGrandPrixDocument()
{
}

// 00559b00
CGrandPrixDocument::CGrandPrixDocument(int type) : CGatewayControlDocJp(type)
{
}

// 00559c00
CGatewayControlDocJp* __fastcall CGrandPrixDocument::Create()
{
	return new CGrandPrixDocument(0xaf);
}

static bool RegisterGrandPrixDocument() // 00559c70
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CGrandPrixDocument::Create);
	return true;
}

static bool s_bRegistered = RegisterGrandPrixDocument();
