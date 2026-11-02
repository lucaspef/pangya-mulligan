// JP 983: _gateway::_questoneday::CQuestDocument + CQuestHandler (classes novas, sem par no 645; existem no 839). Sessões B99 (handler)
// e B100 (documento), F3.7.10. A unidade do 983 (~005e71a0–005f0200, LTCG) é uma só: o documento vem primeiro, na ordem dos
// inicializadores estáticos (00cd2100 registra o documento; 00cd2110…00cd2160 são os do handler). Handler = 005ee100–005f0200
// (registro, tabela de mensagens, Execute, as 8 mensagens e os 3 retornos de janela); documento = o resto. O resto da faixa são
// instâncias da STL, os operadores das lambdas (005ee130, 005ee880, 005eeb20, 005e7310/7330/7350, 005ec0b0) e os
// CreateForm<FrOnedayQuest / FrOnedayQuestComplete / FrAchievement> (005ee440 / 005ee510 / 005ee5e0), que o compilador gera sozinho.
// Ver out/F3.7/s99/RELATORIO.md e out/F3.7/s100/RELATORIO.md.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "../../shared/itemmanager.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "messagemanager.h"
#include "initdelegate.h"
#include "questitem.h"
#include "questhandler.h"
#include "onedayquest.h"
#include "cardcomposedlg.h"
#include "bongdarishopdlg.h"
#include "avatar_avatar.h"
#include <algorithm>
#include <numeric>
#include <list>

extern Fresh* g_pFresh;

using namespace _quest;
using namespace _gateway;
using namespace _gateway::_questoneday;

// S5 (sharedutilities.cpp da frente A) — empréstimo, nome meu (ver gproomtimersteward.cpp)
namespace S5
{
void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); // 00b307f0
}

// a loja "bongdari_shop" do 983 (FrBongdariShopDlg) ganhou um campo +0x584 que o fonte da A não tem: o setter 008b4f20.
// Empréstimo; nome meu.
class FrBongdariShopJp983 : public FrWnd
{
public:
	void SetAchievementPoint983(int point) // 008b4f20: FrBongdariShopDlg::m_achievementPoint983 (pontos da Memorial Shop)
	{
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_achievementPoint983) == 0x584); // D866: era o cru +0x584 (anda no cl novo)
		((FrBongdariShopDlg*)(FrWnd*)this)->m_achievementPoint983 = point;
	}
};

// 005ee310: missão do QuestItem.iff (0x78…) dos subtipos 1 e 2 (bits 22..25); cópia da 005185f0 da onedayquest.cpp (LTCG: typeid em eax)
// o 983 monta a mensagem só depois de testar o ator (o `<<` do actor.h monta antes)
#define LOBBYAVATARMSG983(p1) \
	{ \
		IActor* pActor = AfxGetTask()->GetActor("LobbyAvatar"); \
		if (pActor) \
			pActor << MsgObject(NULL, 0x1a3, p1, 0, 0, 0, 0); \
	}

static bool IsOnedayQuestType983(unsigned long typeId)
{
	if ((typeId & 0xfc000000) != 0x78000000)
		return false;
	return ((typeId >> 22) & 0xf) - 1 <= 1;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// CQuestDocument (B100). As funções da STL da faixa (find_if/equal_range/stable_sort/accumulate, operadores das lambdas 005e7310,
// 005e7330, 005e7350, 005ec0b0, nós dos mapas) o compilador gera sozinho.

// visão local do CItemManager: 00a20180 põe no vetor o endereço de cada registro do Achievement.iff (mapa +0x180). Empréstimo.
class CItemManagerQuestJp983 : public CItemManager
{
public:
	void GetAchievements983(std::vector<IFF983::sAchievement*>& out); // 00a20180
};

// B153 (PRIORIDADE item 4: stub alcançado): o mapa +0x180 é std::map<DWORD, registro de 0x600> (nó com _Isnil em +0x611);
// reserva o tamanho (+0x188) e guarda o endereço de cada registro. O 983 devolve 1 (o chamador 005eddc5 ignora).
void CItemManagerQuestJp983::GetAchievements983(std::vector<IFF983::sAchievement*>& out)
{
	C_ASSERT_VC10(offsetof(CItemManagerQuestJp983, m_AchievementMap983) == 0x180); // D866: era o cru +0x180 (anda no cl novo)
	typedef std::map<unsigned int, IFF983::sAchievement> Map983;
	Map983& iffs = m_AchievementMap983;
	out.reserve(iffs.size());
	for (Map983::iterator it = iffs.begin(); it != iffs.end(); ++it)
		out.push_back((IFF983::sAchievement*)&it->second);
}


namespace
{
inline int StuffInt983(const IFF983::sQuestStuff* p, int offset) { return *(const int*)((const char*)p + offset); }

const unsigned long ACHIEVEMENT_POINT_ITEM = 0x6c000001; // prêmio do QuestStuff.iff que conta como ponto de conquista
}

const char* g_szQuestHandler = "CQuestHandler"; // 00e12f44 (ponteiro lido em Initialize)

// 005ebf80 (LTCG: lista em ecx, estado em edx): linha de "amanhã" para a missão do jogador com esse typeid no estado pedido
static void AddFutureSlot983(std::vector<sQuestSlot983>& slots, int state, unsigned long typeId, bool bNoCount)
{
	std::list<CQuestItem>& quests = ::Doc()->m_quests983;
	std::list<CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(), [typeId, state](CQuestItem& q)
	{
		return q.GetState983() == state && q.GetTypeId983() == typeId;
	});
	if (it == ::Doc()->m_quests983.end())
		return;
	CQuestItem* pItem = &*it;
	if (pItem->GetState983() != state)
		return;
	sQuestSlot983 slot;
	slot.pIff = ::Doc()->m_itemManager.FindQuestItem983(pItem->GetTypeId983());
	slot.pItem = pItem;
	if (bNoCount == true)
		slot.pCount = NULL;
	else
	{
		std::list<sQuestCount983>& counts = ::Doc()->m_questCounts983;
		std::list<sQuestCount983>::iterator c = std::find_if(counts.begin(), counts.end(), [pItem](sQuestCount983& count)
		{
			return pItem->GetUid983() == count.uid;
		});
		slot.pCount = c != ::Doc()->m_questCounts983.end() ? &*c : NULL;
	}
	slots.push_back(slot);
}

// 005ebee0 (LTCG: lista em ecx, missão em edx): linha de "hoje" (estado pedido ou COMPLETE); a contagem é a do 1º contador
static void AddCurrentSlot983(std::vector<sQuestSlot983>& slots, CQuestItem* pItem, int state)
{
	if (pItem->GetState983() != state && pItem->GetState983() != 4)
		return;
	sQuestSlot983 slot;
	slot.pIff = ::Doc()->m_itemManager.FindQuestItem983(pItem->GetTypeId983());
	slot.pItem = pItem;
	std::list<sQuestCount983>& counts = ::Doc()->m_questCounts983;
	std::list<sQuestCount983>::iterator c = std::find_if(counts.begin(), counts.end(), [pItem](sQuestCount983& count)
	{
		return pItem->GetCounterUid983(0) == count.uid;
	});
	slot.pCount = c != ::Doc()->m_questCounts983.end() ? &*c : NULL;
	slots.push_back(slot);
}

// 005ede60
CQuestDocument::CQuestDocument(int type) : CGatewayControlDocJp(type)
{
}

// 005edaf0
CQuestDocument::~CQuestDocument()
{
}

// 005ee040
CGatewayControlDocJp* __fastcall CQuestDocument::Create()
{
	return new CQuestDocument(0xac);
}

// 005ee0b0 (chamada pelo inicializador estático 00cd2100)
static bool RegisterQuestDocument()
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CQuestDocument::Create);
	return true;
}

static bool s_bDocRegistered = RegisterQuestDocument();

// 005edc30
CGatewayHandler* CQuestDocument::GetGatewayHandler()
{
	return (CGatewayHandler*)(CGatewayHandlerJp*)m_pHandler; // a visão CGatewayHandlerJp (controllerhandler.h) não deriva do CGatewayHandler
}

// 005e7250
bool CQuestDocument::Release()
{
	return true;
}

// 005e7260
bool CQuestDocument::Reset()
{
	memset(&m_todayDate, 0, sizeof(m_todayDate));
	memset(&m_procDate, 0, sizeof(m_procDate));
	return true;
}

// 005edc40
bool CQuestDocument::Initialize()
{
	const char* name = g_szQuestHandler;
	m_pHandler = (CQuestHandler*)ActorRegistry983().Get(name);

	// o 983 passa num=10: a contagem e info[10].id (0x2c6) -> registra os 11 e segue lendo a pilha; aqui a entrada final da a contagem 11
	_systemmsg::sMsgInfo msgInfo[] = {
		{ (_systemmsg::eMsgIdentifier)0x2bc, K2L_Compatibility("\277\300\264\303\300\307 \304\371\275\272\306\256\270\246 \271\336\300\270\275\303\260\332\275\300\264\317\261\356?") },
		{ (_systemmsg::eMsgIdentifier)0x2bd, K2L_Compatibility("\304\371\275\272\306\256\260\241 \260\241\265\346\302\367 \271\336\300\273 \274\366 \276\370\275\300\264\317\264\331.") },
		{ (_systemmsg::eMsgIdentifier)0x2be, K2L_Compatibility("\304\371\275\272\306\256\270\246 \306\367\261\342\307\317\270\351, \271\336\301\366 \276\312\300\272 \304\371\275\272\306\256 "
			"\272\270\273\363\300\314 \270\360\265\316 \273\347\266\363\301\375\264\317\264\331.\012\304\371\275\272\306\256\270\246 \300\374\272\316 \306\367\261\342\307\317\275\303\260\332\275\300\264\317\261\356?") },
		{ (_systemmsg::eMsgIdentifier)0x2bf, K2L_Compatibility("%d\260\263 \304\371\275\272\306\256\270\246 \277\317\267\341\307\317\277\264\275\300\264\317\264\331.") },
		{ (_systemmsg::eMsgIdentifier)0x2c0, K2L_Compatibility("\274\272\260\370") },
		{ (_systemmsg::eMsgIdentifier)0x2c1, K2L_Compatibility("\304\371\275\272\306\256 \307\330\260\341") },
		{ (_systemmsg::eMsgIdentifier)0x2c2, K2L_Compatibility("\304\371\275\272\306\256 \276\370\300\275") },
		{ (_systemmsg::eMsgIdentifier)0x2c3, K2L_Compatibility("\304\371\275\272\306\256 \272\270\273\363\300\273 \271\336\300\270\275\303\260\332\275\300\264\317\261\356?") },
		{ (_systemmsg::eMsgIdentifier)0x2c4, K2L_Compatibility("\306\257\272\260 \304\371\275\272\306\256 \272\270\273\363\300\273 \271\336\300\270\275\303\260\332\275\300\264\317\261\356?") },
		{ (_systemmsg::eMsgIdentifier)0x2c5, K2L_Compatibility("%d\260\263 \276\367\300\373\300\273 \277\317\267\341\307\317\277\264\275\300\264\317\264\331.") },
		{ (_systemmsg::eMsgIdentifier)0x2c6, K2L_Compatibility("\276\367\300\373 \301\370\307\340 \261\342\267\317\300\314 \276\370\275\300\264\317\264\331") },
		{ (_systemmsg::eMsgIdentifier)11, NULL }, // B1408: contagem lida em info[num].id (ver InsertMessageGroup)
	};
	_systemmsg::CMessageManager::Instance()->InsertMessageGroup(msgInfo,
		sizeof(msgInfo) / sizeof(msgInfo[0]) - 1);

	m_achievementIffs.clear();
	for (int i = 0; i < MAX_CATEGORY; ++i)
		m_categoryPoints[i] = 0;
	((CItemManagerQuestJp983&)::Doc()->m_itemManager).GetAchievements983(m_achievementIffs);
	SortAchievementIffs983();
	BuildAchievementInfos983();
	return true;
}

// 005e7290
void CQuestDocument::SetDate983(const SYSTEMTIME& st, int index)
{
	if (index < 2)
		(&m_todayDate)[index] = st;
}

// 005e7fd0
bool CQuestDocument::HasCurrent983() const
{
	return m_current.empty() ? false : true;
}

// 005e9620
sQuestSlot983* CQuestDocument::FindCurrentByUid983(unsigned long uid)
{
	std::vector<sQuestSlot983>::iterator it = std::find_if(m_current.begin(), m_current.end(), [uid](sQuestSlot983& slot)
	{
		return slot.pItem && slot.pItem->GetUid983() == uid;
	});
	return it != m_current.end() ? &*it : NULL;
}

// 005ec050
void CQuestDocument::SetFuture983(const std::vector<unsigned long>& typeIds)
{
	m_future.clear();
	for (std::vector<unsigned long>::const_iterator it = typeIds.begin(); it != typeIds.end(); ++it)
		AddFutureSlot983(m_future, 1, *it, false);
}

// 005ecd80
void CQuestDocument::SetCurrent983(const std::vector<CQuestItem*>& items)
{
	m_current.clear();
	for (std::vector<CQuestItem*>::const_iterator it = items.begin(), end = items.end(); it != end; ++it)
		AddCurrentSlot983(m_current, *it, 3);
}

// 005eb9f0
void CQuestDocument::AddAchievement983(const _achievement::CAchievement& a)
{
	if (m_newAchievements.find(a.GetTypeId983()) == m_newAchievements.end())
		m_newAchievements.insert(std::make_pair(a.GetTypeId983(), a));
}

// 005ecde0
void CQuestDocument::FlipAchievements983()
{
	m_achievements.swap(m_newAchievements);
	m_newAchievements.clear();
}

// 005ed6c0
int CQuestDocument::GetPoint983(const _achievement::CAchievement* p)
{
	if (p == NULL)
		return 0;
	int last = p->GetLastCounter983();
	if (last == -1)
		return 0;
	std::vector<IFF983::sQuestStuff*>& stuffs = m_achievementInfos[p->GetTypeId983()].stuffs;
	int point = 0;
	for (int i = 0; i <= last; ++i)
		point += StuffInt983(stuffs[i], 0xf4);
	return point;
}

// 005ed3d0
int CQuestDocument::GetAchievementPoint983()
{
	if (m_achievements.empty())
		return 0;
	int point = 0;
	for (AchievementMap::iterator it = m_achievements.begin(); it != m_achievements.end(); ++it)
		point += GetPoint983(&it->second);
	return point;
}

// 005ed500
int CQuestDocument::GetCategoryPoint983(int category)
{
	if (category >= MAX_CATEGORY)
		return 0;
	sAchievementView983 key;
	key.category = category;
	std::pair<std::vector<IFF983::sAchievement*>::iterator, std::vector<IFF983::sAchievement*>::iterator> range =
		std::equal_range(m_achievementIffs.begin(), m_achievementIffs.end(), (IFF983::sAchievement*)&key,
			[](const IFF983::sAchievement* a, const IFF983::sAchievement* b) { return AchievementView983(a)->category < AchievementView983(b)->category; });
	if (range.first == range.second)
		return 0;
	int point = 0;
	for (std::vector<IFF983::sAchievement*>::iterator it = range.first; it != range.second; ++it)
	{
		AchievementMap::iterator found = m_achievements.find((*it)->TypeId);
		if (found != m_achievements.end())
			point += GetPoint983(&found->second);
	}
	return point;
}

// 005ea470
std::vector<IFF983::sQuestStuff*>* CQuestDocument::FindStuffs983(unsigned long typeId)
{
	AchievementInfoMap::iterator it = m_achievementInfos.find(typeId);
	if (it == m_achievementInfos.end())
		return NULL;
	return &it->second.stuffs;
}

// 005ec0d0
void CQuestDocument::SortAchievementIffs983()
{
	std::stable_sort(m_achievementIffs.begin(), m_achievementIffs.end(),
		[](const IFF983::sAchievement* a, const IFF983::sAchievement* b) { return AchievementView983(a)->category < AchievementView983(b)->category; });
}

// 005eac20
void CQuestDocument::MakeStuffs983(std::vector<IFF983::sQuestStuff*>& stuffs, IFF983::sAchievement* pIff)
{
	const unsigned long* pId = AchievementView983(pIff)->stuffIds;
	for (int i = 0; i < 10; ++i, ++pId)
	{
		if (*pId == 0)
			continue;
		IFF983::sQuestStuff* pStuff = ::Doc()->m_itemManager.FindQuestStuff983(*pId);
		if (pStuff)
			stuffs.push_back(pStuff);
	}
}

// 005e9670
int CQuestDocument::CountStuffPoints983(const std::vector<IFF983::sQuestStuff*>& stuffs)
{
	return std::accumulate(stuffs.begin(), stuffs.end(), 0, [](int point, IFF983::sQuestStuff* p) -> int
	{
		for (int i = 0; i < 3; ++i)
		{
			if ((unsigned long)StuffInt983(p, 0xe8 + i * 4) == ACHIEVEMENT_POINT_ITEM)
				point += StuffInt983(p, 0xf4 + i * 4);
		}
		return point;
	});
}

// 005ed780
void CQuestDocument::BuildAchievementInfos983()
{
	for (int i = 0; i < MAX_CATEGORY; ++i)
	{
		sAchievementView983 key;
		key.category = i;
		std::pair<std::vector<IFF983::sAchievement*>::iterator, std::vector<IFF983::sAchievement*>::iterator> range =
			std::equal_range(m_achievementIffs.begin(), m_achievementIffs.end(), (IFF983::sAchievement*)&key,
				[](const IFF983::sAchievement* a, const IFF983::sAchievement* b) { return AchievementView983(a)->category < AchievementView983(b)->category; });
		if (range.first == range.second)
			continue;
		int point = 0;
		for (std::vector<IFF983::sAchievement*>::iterator it = range.first; it != range.second; ++it)
		{
			IFF983::sAchievement* pIff = *it;
			std::vector<IFF983::sQuestStuff*> stuffs;
			MakeStuffs983(stuffs, pIff);
			sAchievementInfo983 info;
			info.typeId = pIff->TypeId;
			info.points = CountStuffPoints983(stuffs);
			point += info.points;
			info.pIff = pIff;
			m_achievementInfos.insert(std::make_pair(info.typeId, info));
			m_achievementInfos[info.typeId].stuffs.swap(stuffs);
		}
		m_categoryPoints[i] = point;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------
// CQuestHandler (B99)

CQuestHandler::CQuestHandler()
	: m_pQuestDlg(NULL)
{
}

IObject* CQuestHandlerMakeInstance() // 005efeb0
{
	return new CQuestHandler;
}

struct __sCQuestHandler
{
	__sCQuestHandler() // 005eff20
	{
		ObjectFactory().AddObjectFunctor(CQuestHandlerMakeInstance, "CQuestHandler");
	}
};

const WRTTI CQuestHandler::m_RTTI("CQuestHandler", &IObject::m_RTTI);
static __sCQuestHandler __implCQuestHandler;

static void ReleaseQuestHandler() // 005ee150 (vazia)
{
}

static void RegisterQuestHandlerActor() // 005ef240: o ator já nasce criado e fica guardado na fábrica pelo nome
{
	ActorRegistry983().Add(CQuestHandler::m_RTTI.GetName(),
		ObjectFactory().CreateObject(CQuestHandler::m_RTTI.GetName()));
}

static bool AddInitQuestHandler() // 005ef880 (chamada pelo inicializador estático 00cd2150)
{
	static bool s_bAdded; // 00e78c78
	if (!s_bAdded)
	{
		CInitDelegate::AddInitFunction(RegisterQuestHandlerActor);
		s_bAdded = true;
	}
	return s_bAdded;
}

static bool AddReleaseQuestHandler() // 005ee360 (chamada pelo inicializador estático 00cd2160)
{
	static bool s_bAdded; // 00e78c79
	if (!s_bAdded)
	{
		CInitDelegate::AddReleaseFunction(ReleaseQuestHandler);
		s_bAdded = true;
	}
	return s_bAdded;
}

static bool s_bInitAdded = AddInitQuestHandler(); // 00e78c7b
static bool s_bReleaseAdded = AddReleaseQuestHandler(); // 00e78c84

bool CQuestHandler::OnCompleteDlgResult983(int ret, FrForm* pForm)
{
	m_pCompleteDlg = NULL;
	return true;
}

bool CQuestHandler::OnQuestDlgResult983(int ret, FrForm* pForm)
{
	m_pQuestDlg = NULL;
	LOBBYAVATARMSG983(1);
	return true;
}

bool CQuestHandler::OnAchievementDlgResult983(int ret, FrForm* pForm)
{
	m_pAchievementDlg = NULL;
	LOBBYAVATARMSG983(1);
	return true;
}

void CQuestHandler::OnAchievementList983(const MsgObject& msg)
{
	CQuestDocument* pDoc = Doc983();
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	pPacket->Decode4();
	for (unsigned long n = pPacket->Decode4(); n != 0; --n)
	{
		_achievement::CAchievement achievement;
		achievement.Decode983(pPacket);
		pDoc->AddAchievement983(achievement);
	}
}

void CQuestHandler::OnAchievementOpen983()
{
	CQuestDocument* pDoc = Doc983();
	pDoc->FlipAchievements983();
	if (pDoc->m_achievements.empty())
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x2c6), 0, 0, 0, 0));
		::Doc()->m_goStopMode = false;
		return;
	}
	if (::Doc()->m_goStopMode)
	{
		::Doc()->m_goStopMode = false;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x356, 0, 0, 0, 0, 0));
		return;
	}
	LOBBYAVATARMSG983(0);
	FrWnd* pShop = g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop");
	if (pShop == NULL)
	{
		m_pAchievementDlg = CreateForm<FrAchievement>(g_pFresh->GetManager(), this, "ach_main", NULL);
		m_pAchievementDlg->m_pDoc = pDoc;
		m_pAchievementDlg->EnableDrag(false);
		m_pAchievementDlg->SetTopmost(true);
		m_pAchievementDlg->FindNextTopFocus(true);
		m_pAchievementDlg->Open((FRESH_PFN_RESULT)&CQuestHandler::OnAchievementDlgResult983, 1);
		if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
			ntAvatarManager::m_pInstance->m_bActive = false;
	}
	else
	{
		((FrBongdariShopJp983*)pShop)->SetAchievementPoint983(pDoc->GetAchievementPoint983());
	}
}

bool CQuestHandler::Execute(const MsgObject& msg)
{
	if (m_functionMap.size() == 0)
		Initialize(); // E141: a tabela nao era montada (OnEnter nao chamado pelo Gateway)
	SetFunctionType(msg.param1);
	return Exec<CQuestHandler, const MsgObject&>(this, msg);
}

void CQuestHandler::OnQuestComplete983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	unsigned long count = pPacket->Decode4();
	std::vector<CQuestItem*> done;
	for (unsigned int i = 0; i < count; ++i)
	{
		CQuestItem item;
		item.Decode983(pPacket);
		unsigned long uid = item.GetUid983();
		std::list<CQuestItem>& quests = ::Doc()->m_quests983;
		std::list<CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
			[uid](CQuestItem& q) { return q.GetUid983() == uid; });
		if (it != ::Doc()->m_quests983.end())
		{
			CQuestItem* pQuest = &*it;
			pQuest->MergeDone983(&item);
			if ((pQuest->GetTypeId983() & 0x3c00000) != 0x400000 && item.IsState983(4)
				&& (item.GetTypeId983() & 0xfc000000) != 0x4c000000)
				done.push_back(pQuest);
		}
	}
	if (done.empty())
		return;
	for (std::vector<CQuestItem*>::iterator it = done.begin(); it != done.end(); ++it)
	{
		if (((*it)->GetTypeId983() & 0xfc000000) == 0x4c000000)
			return;
	}
	m_pCompleteDlg = CreateForm<FrOnedayQuestComplete>(g_pFresh->GetManager(), this, "oneday_quest_complete", NULL);
	m_pCompleteDlg->SetQuests983(done);
	if (m_pCompleteDlg->HasTypes983())
	{
		m_pCompleteDlg->EnableDrag(false);
		m_pCompleteDlg->MoveUp983();
		m_pCompleteDlg->SetTopmost(true);
		m_pCompleteDlg->FindNextTopFocus(true);
		m_pCompleteDlg->Open((FRESH_PFN_RESULT)&CQuestHandler::OnCompleteDlgResult983, 1);
	}
}

void CQuestHandler::OnQuestReward983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	pPacket->Decode4();
	unsigned long uid = pPacket->Decode4();
	std::list<CQuestItem>& quests = ::Doc()->m_quests983;
	std::list<CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
		[uid](CQuestItem& q) { return q.GetUid983() == uid; });
	if (it == ::Doc()->m_quests983.end())
		return;
	it->Accept983();
	if (m_pQuestDlg)
	{
		if ((it->GetTypeId983() & 0x3c00000) == 0)
			m_pQuestDlg->OnQuestRewarded983(&*it);
		else
			m_pQuestDlg->OnDailyRewarded983(&*it);
	}
	if (!it->IsActive983())
		::Doc()->m_quests983.erase(it);
}

void CQuestHandler::UpdateCurrent983()
{
	std::vector<CQuestItem*> current;
	std::list<CQuestItem>& quests = ::Doc()->m_quests983;
	for (std::list<CQuestItem>::iterator it = quests.begin(), end = quests.end(); it != end; ++it)
	{
		CQuestItem* pQuest = &*it;
		if (pQuest->IsState983(1) != true && IsOnedayQuestType983(pQuest->GetTypeId983()) != true)
		{
			IFF983::sQuestItem* pIff = ItemManager()->FindQuestItem983(pQuest->GetTypeId983());
			if (pIff && *(int*)((char*)pIff + 0xc8) == 1)
				current.push_back(pQuest);
		}
	}
	Doc983()->SetCurrent983(current);
}

void CQuestHandler::OnQuestList983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	CQuestDocument* pDoc = Doc983();
	pDoc->Reset();
	unsigned long todaySec = pPacket->Decode4();
	unsigned long procSec = pPacket->Decode4();
	SYSTEMTIME st;
	if (todaySec)
	{
		S5::SecToSysTime983(&st, todaySec);
		pDoc->SetDate983(st, 0);
	}
	if (procSec)
	{
		S5::SecToSysTime983(&st, procSec);
		pDoc->SetDate983(st, 1);
	}
	unsigned long uid;
	std::vector<unsigned long> uids;
	unsigned long count = pPacket->Decode4();
	for (unsigned int i = 0; i < count; ++i)
	{
		uid = pPacket->Decode4();
		uids.push_back(uid);
	}
	count = pPacket->Decode4();
	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned long uid = pPacket->Decode4();
		std::list<CQuestItem>& quests = ::Doc()->m_quests983;
		std::list<CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
			[uid](CQuestItem& q) { return q.GetUid983() == uid; });
		if (it != ::Doc()->m_quests983.end())
		{
			it->Deactivate983();
			if (!it->IsActive983())
				::Doc()->m_quests983.erase(it);
		}
	}
	pDoc->SetFuture983(uids);
	if (procSec)
		S5::SecToSysTime983(&st, procSec);
	UpdateCurrent983();
	LOBBYAVATARMSG983(0);
	m_pQuestDlg = CreateForm<FrOnedayQuest>(g_pFresh->GetManager(), this, "oneday_quest_main", NULL);
	WRect rc = m_pQuestDlg->GetRect();
	rc.y += 60.0;
	rc.x += 30.0;
	m_pQuestDlg->SetRect(rc);
	m_pQuestDlg->EnableDrag(false);
	m_pQuestDlg->m_pDoc = pDoc;
	m_pQuestDlg->Open((FRESH_PFN_RESULT)&CQuestHandler::OnQuestDlgResult983, 1);
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
		ntAvatarManager::m_pInstance->m_bActive = false;
}

void CQuestHandler::OnQuestCancel983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	unsigned long count = pPacket->Decode4();
	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned long uid = pPacket->Decode4();
		std::list<CQuestItem>& quests = ::Doc()->m_quests983;
		std::list<CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
			[uid](CQuestItem& q) { return q.GetUid983() == uid; });
		if (it == ::Doc()->m_quests983.end())
			break;
		it->Deactivate983();
		if (!it->IsActive983())
			::Doc()->m_quests983.erase(it);
	}
	UpdateCurrent983();
	if (m_pQuestDlg)
		m_pQuestDlg->RefreshLists983();
}

void CQuestHandler::OnQuestUpdate983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	CQuestDocument* pDoc = Doc983();
	unsigned long count = pPacket->Decode4();
	for (unsigned int i = 0; i < count; ++i)
	{
		CQuestItem item;
		item.Decode983(pPacket);
		unsigned long uid = item.GetUid983();
		std::list<CQuestItem>& quests = ::Doc()->m_quests983;
		std::list<CQuestItem>::iterator it = std::find_if(quests.begin(), quests.end(),
			[uid](CQuestItem& q) { return q.GetUid983() == uid; });
		if (it == ::Doc()->m_quests983.end())
			break;
		it->Update983(item);
	}
	pDoc->m_future.clear();
	UpdateCurrent983();
	if (m_pQuestDlg)
		m_pQuestDlg->RefreshLists983();
}

void CQuestHandler::OnAchievementDone983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = MSGJP(msg).Param2<WReceivedPacket*>();
	std::vector<sQuestDone983> done;
	unsigned long count = pPacket->Decode4();
	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned long typeId = pPacket->Decode4();
		unsigned long stuffId = pPacket->Decode4();
		if ((typeId & 0xfc000000) == 0x78000000)
			return;
		sQuestDone983 d;
		d.typeId = typeId;
		d.stuffId = stuffId;
		done.push_back(d);
	}
	if (done.empty())
		return;
	m_pCompleteDlg = CreateForm<FrOnedayQuestComplete>(g_pFresh->GetManager(), this, "oneday_quest_complete", NULL);
	m_pCompleteDlg->SetDone983(done);
	if (m_pCompleteDlg->HasTypes983())
	{
		m_pCompleteDlg->EnableDrag(false);
		m_pCompleteDlg->MoveDown983();
		m_pCompleteDlg->SetTopmost(true);
		m_pCompleteDlg->FindNextTopFocus(true);
		m_pCompleteDlg->Open((FRESH_PFN_RESULT)&CQuestHandler::OnCompleteDlgResult983, 0x801);
	}
}

bool CQuestHandler::Initialize()
{
	AddFunctionType(0x02, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnQuestList983);
	AddFunctionType(0x03, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnQuestUpdate983);
	AddFunctionType(0x04, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnQuestComplete983);
	AddFunctionType(0x05, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnQuestReward983);
	AddFunctionType(0x06, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnQuestCancel983);
	AddFunctionType(0x08, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnAchievementList983);
	AddFunctionType(0x09, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnAchievementOpen983);
	AddFunctionType(0x0a, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CQuestHandler::*)())&CQuestHandler::OnAchievementDone983);
	return true;
}
