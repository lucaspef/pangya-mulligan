// JP 983: unidade 0057062e–~005795d0 (_gateway::_golfgame::CHoleInOneGameDoc). Fonte reconstruída do 983 (classe nova, sem par no
// 645). Sessão B25: construtor, destrutor, virtuais, registro e os métodos que o CHoleInOneGameHandler chama. O resto da faixa são
// instâncias da STL (map, vector, list, sort).
#include "minatl.h"
#include "projectg.h"
#include "holeinonegamedoc.h"
#include "shareddoc.h"
#include "shareddocjp.h"
#include "golfdoc.h"
#include "docofs983.h"
#include "vc10_layout.h"
#include "actor.h"
#include <algorithm>
#include <list>

using namespace _gateway;
using namespace _gateway::_golfgame;

// JP 983: campos lidos por deslocamento (o layout do 983 nestas classes ainda não está no fonte da frente A).
// A739: ancorados nos membros do CSharedDoc/CGolfDoc (no VS 2022 o Doc anda; no VC10 = 983, C_ASSERT_VC10 abaixo).
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwGuid) == 0x81a); // A743: ex-DOC983_U32(0x81a)
inline CItemTableJpHio& ItemTable983() { return *(CItemTableJpHio*)(char*)&::Doc()->m_itemManager; }
#define ITEMTABLE983() (&ItemTable983())
// 009da0a0 (fatal em falta_exe): devolve o mapa typeId -> registro em this + 0x2e8
CItemTableJpHio::RecordMap& CItemTableJpHio::GetMap983()
{
	// D878: era (char*)this + 0x2e8 cru (VC10); no cl 14.44 string/map do CItemManager encolhem -> membro (C_ASSERT_VC10 itemmanager.cpp)
	return ((CItemManager*)this)->m_CupInConditionMap983;
}
// 00a191c0: map::find em this + 0x2e8; devolve no + 0x10 = &->second (chave em +0xc). B1574: &*it era o par (no + 0xc),
// 4 bytes antes: flag9c caia em pad98, m_14 em param10 (sempre 0) e o modo ficava sem missoes
IFF983::sCupInCondition* CItemTableJpHio::Find983(unsigned long typeId)
{
	RecordMap& records = GetMap983();
	RecordMap::iterator it = records.find(typeId);
	if (it == records.end())
		return NULL;
	return &it->second;
}
struct sRoomPlayer983 // ficha do jogador na sala (0x1b8 em CSharedDoc+0x1042c); só o uid
{
	char m_pad00[4];
	unsigned long m_uid; // +0x04
	char m_pad08[0x1b8 - 0x08];
};
#define ROOMPLAYERS983() (*(std::vector<sRoomPlayer983>*)&::Doc()->m_rivalList)
struct sHoleDoc983 // buraco do CGolfDoc (0x38 por buraco em +0xd8); só a segunda posição
{
	char m_pad00[0x0c];
	float m_x; // +0x0c
	float m_y; // +0x10
	float m_z; // +0x14
	char m_pad18[0x38 - 0x18];
};
#define GOLFDOC983_CURHOLE() (&((sHoleDoc983*)::Doc()->m_pGolfDoc->m_pHoleData)[::Doc()->m_pGolfDoc->m_currentHole - 1])
C_ASSERT_VC10(offsetof(CSharedDoc, m_itemManager) == 0x20c && offsetof(CSharedDoc, m_rivalList) == 0x1042c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024 && sizeof(sRivalData) == sizeof(sRoomPlayer983));
C_ASSERT_VC10(offsetof(CGolfDoc, m_pHoleData) == 0xd8 && offsetof(CGolfDoc, m_currentHole) == 0x17c);

const char* g_szHoleInOneGameHandler = "CHoleInOneGameHandler"; // 00e0c748 (ponteiro lido em Initialize)

// 005706f0/00570700: predicado "registro deste typeId com flag9c == 2" (quem o usa não foi levantado)
struct sHioItemIs983
{
	sHioItemIs983(unsigned long typeId) : m_typeId(typeId) { }
	bool operator()(const IFF983::sCupInCondition* p) const
	{
		if (m_typeId == p->id && p->flag9c == 2)
			return true;
		return false;
	}
	unsigned long m_typeId;
};

struct sHioItemLess983 // 00570730
{
	bool operator()(const IFF983::sCupInCondition* a, const IFF983::sCupInCondition* b) const { return a->flag9c < b->flag9c; }
};

inline float LengthSq983(const sHioSpotDist983& d)
{
	return d.m_dx * d.m_dx + d.m_dz * d.m_dz;
}
struct sHioDistLess983
{
	bool operator()(const sHioSpotDist983& a, const sHioSpotDist983& b) const { return LengthSq983(a) < LengthSq983(b); }
};

bool CHoleInOneGameDoc::Initialize()
{
	Reset();
	const char* name = g_szHoleInOneGameHandler;
	m_pHandler = NULL;
	m_pHandler = (CGatewayHandler*)ActorRegistry983().Get(name);
	m_7c = 1;
	m_80 = 1;
	m_84 = 0;
	m_pMine = NULL;
	return true;
}

int CHoleInOneGameDoc::GetMine0c983()
{
	return m_pMine->m_0c;
}

void CHoleInOneGameDoc::Fn5706d0_983()
{
	++m_28;
	m_24 = 0;
}

void CHoleInOneGameDoc::Fn5706e0_983()
{
	++m_24;
	++m_28;
}

void CHoleInOneGameDoc::Fn570750_983()
{
	memset(&m_3c, 0, sizeof(float) * 3);
}

void CHoleInOneGameDoc::SetItemTotal983(int total)
{
	m_58 = total;
}

void CHoleInOneGameDoc::Fn570770_983()
{
	m_58 = 0;
}

bool CHoleInOneGameDoc::SetRecord983(const sHioRecord983* p)
{
	RecordMap::iterator it = m_records.find(p->m_uid);
	if (it == m_records.end())
		return false;
	it->second = *p;
	return true;
}

void CHoleInOneGameDoc::Fn576690_983()
{
	CItemTableJpHio::RecordMap& records = ITEMTABLE983()->GetMap983();
	for (CItemTableJpHio::RecordMap::iterator it = records.begin(); it != records.end(); ++it)
	{
		IFF983::sCupInCondition* pRecord = &it->second; // 00576690: no + 0x10, como o Find983
		if (pRecord->flag9c & 4)
			m_items.push_back(pRecord);
	}
}

bool CHoleInOneGameDoc::Fn576820_983(unsigned long typeId)
{
	IFF983::sCupInCondition* pRecord = ITEMTABLE983()->Find983(typeId);
	if (pRecord == NULL)
		return false;
	m_items.push_back(ItemList::value_type(pRecord));
	return true;
}

void CHoleInOneGameDoc::Fn577220_983()
{
	int n = (int)ROOMPLAYERS983().size();
	for (int i = 0; i < n; ++i)
	{
		unsigned long uid = ROOMPLAYERS983()[i].m_uid;
		m_records.insert(RecordMap::value_type(uid, sHioRecord983(uid, 0, 0, 0)));
	}
	m_pMine = NULL;
	unsigned long mine = ::Doc()->m_myInfo.info.dwGuid;
	m_pMine = &m_records.find(mine)->second;
}

void CHoleInOneGameDoc::SetPlayerPairs983(unsigned long uid, PairList* pPairs)
{
	if (m_playerPairs.find(uid) == m_playerPairs.end())
		m_playerPairs.insert(PlayerPairMap::value_type(uid, pPairs));
}

CHoleInOneGameDoc::PairList* CHoleInOneGameDoc::GetPlayerPairs983(unsigned long uid)
{
	PlayerPairMap::iterator it = m_playerPairs.find(uid);
	if (it == m_playerPairs.end())
		return NULL;
	return it->second;
}

void CHoleInOneGameDoc::Fn577ed0_983()
{
	sHioItemLess983 less;
	std::sort(m_items.begin(), m_items.end(), less);
}

void CHoleInOneGameDoc::Fn578580_983()
{
	std::list<IFF983::sCupInCondition*> list;
	list.assign(m_items.begin(), m_items.end());
	for (std::list<IFF983::sCupInCondition*>::iterator it = list.begin(); it != list.end(); )
	{
		if ((*it)->flag9c & 2)
			it = list.erase(it);
		else
			++it;
	}
	m_items.clear();
	m_items.assign(list.begin(), list.end());
}

void CHoleInOneGameDoc::SortSpots983()
{
	std::vector<sHioSpotDist983> dists;
	sHioSpot983 pos(GOLFDOC983_CURHOLE()->m_x, GOLFDOC983_CURHOLE()->m_z);
	sHioSpot983 hole(pos);
	unsigned int n = m_spots.size();
	for (unsigned int i = 0; i < n; ++i)
	{
		sHioSpot983 d = hole - m_spots[i];
		sHioSpotDist983 dist;
		dist.m_index = i;
		dist.m_dx = d.m_x;
		dist.m_dz = d.m_z;
		dists.push_back(dist);
	}
	sHioDistLess983 less;
	std::sort(dists.begin(), dists.end(), less);
	std::vector<sHioSpot983> old;
	std::swap(old, m_spots);
	m_spots.clear();
	m_spots.assign(dists.size(), pos);
	int count = (int)dists.size();
	for (int i = 0; i < count; ++i)
		m_spots[i] = old[dists[i].m_index];
}

CHoleInOneGameDoc::CHoleInOneGameDoc(int type) : CGatewayControlDocJp(type)
{
}

CHoleInOneGameDoc::~CHoleInOneGameDoc()
{
}

CGatewayControlDocJp* __fastcall CHoleInOneGameDoc::Create()
{
	return new CHoleInOneGameDoc(0xaa);
}

bool CHoleInOneGameDoc::Reset()
{
	m_records.clear();
	m_items.clear();
	m_spots.clear();
	m_24 = 0;
	m_28 = 0;
	m_44 = 0.0f;
	m_3c = 0.0f;
	m_40 = 0.0f;
	m_players.clear();
	m_playerPairs.clear();
	m_58 = 0;
	m_spot = 0;
	return true;
}

static bool RegisterHoleInOneGameDoc() // 00579540 (chamado pelo inicializador estático 00ccec30)
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CHoleInOneGameDoc::Create);
	return true;
}

static bool s_bRegistered = RegisterHoleInOneGameDoc();

void CHoleInOneGameDoc::ResizePlayers983(int n)
{
	m_players.clear();
	m_players.resize(n);
	m_playerPairs.clear();
}
