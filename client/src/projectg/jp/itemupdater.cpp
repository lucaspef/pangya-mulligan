// JP 983: unidade 005846c0–0058c3f0 (CItemUpdater). Classe nova do 983 (existe no 839); ver include/jp/itemupdater.h. Sessão B121.
// 005846c0–00589b50 são sobretudo moldes da STL (map<unsigned, sItemInfo/sCaddieInfo/sMascotInfo>, list<sItemInfo/sCards/
// sFurniture_List>…) que outras unidades reaproveitam por COMDAT; o código próprio começa por volta de 00588230 e é uma árvore
// com entrada única Update983 0058c160. 0058c310–0058c3d0 (5 setters de byte de outra classe, sem chamador) ficam fora.
#include "minatl.h"
#include "vc10_layout.h"
#include "wlocalize.h"
#include "projectg.h"
#include "actor.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "../shared/localize.h"
#include "questitem.h"
#include "itemupdater.h"
#include <algorithm>
#include <math.h>
#include <time.h>
#include "ucclibrary.h"

// S5 (sharedutilities.cpp da frente A) — empréstimo (ver achievementdlg.cpp)
namespace S5
{
void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); // 00b307f0
}

// visão local do CSharedDoc: dois métodos fora de linha da lista de habilidades de carta (+0x37ec). Empréstimo, nomes meus.
class CSharedDocItemJp983 : public CSharedDoc
{
public:
	void RemoveCardAbilityByPart983(unsigned long partUid); // 0063db90: apaga as habilidades cujo partsUid é este
	void AddCardAbility983(const sSCardAvilityPeriodInfo& info); // 00641350: m_cardAbilityList[0].push_back fora de linha
};

static inline CSharedDocItemJp983* Doc983()
{
	return (CSharedDocItemJp983*)CSharedDoc::Instance();
}

// 00585530: período válido (ItemType 1..6)
static bool IsValidPeriod983(int type)
{
	if (0 < type && type < 7)
		return true;
	return false;
}

// 00585540: período em segundos (2/3 = horas, 1/4/6 = dias, 5 = 30 dias)
static int PeriodSeconds983(int type, int value)
{
	switch (type)
	{
	case 2:
	case 3:
		return value * 3600;
	case 1:
	case 4:
	case 6:
		return value * 86400;
	case 5:
		return value * 2592000;
	}
	return 0;
}

// 00585580: período em horas
static int PeriodHours983(int type, int value)
{
	return PeriodSeconds983(type, value) / 3600;
}

// 005855a0: horas que faltam até `end` (arredonda com 5 minutos de folga)
static int RemainHours983(__time64_t end, __time64_t now)
{
	return (int)floor((_difftime64(end, now) + 300.0) / 3600.0);
}

// sMascotInfo do 983: +0x2b = byte "tem prazo" e +0x2d = data de fim (SYSTEMTIME); o globalgamedefine.h ainda tem o layout do
// 645 (Remain_Date +0x2b, Purchase +0x2d, endDate +0x2e) — pedido B122.1 → A
#define MASCOT983_TIMED(info) (((unsigned char*)&(info))[0x2b])
#define MASCOT983_END(info) (*(SYSTEMTIME*)((unsigned char*)&(info) + 0x2d))

// lista CSharedDoc+0x3828
static inline std::list<sItemCount983>& ItemCountList983()
{
	return CSharedDoc::Instance()->m_itemCounts983;
}

// predicados das missões pelo uid: dois tipos iguais, um para FindQuest983 (005847a0 construtor, 005847b0 operator()) e outro
// para EraseQuest983 (005847d0/005847e0). No 983 os dois membros estão fora de linha com convenção de registro (LTCG).
struct sQuestUid983
{
	unsigned long uid;
	sQuestUid983(unsigned long u);
	bool operator()(const _quest::CQuestItem& q) const;
};

// 005847a0
sQuestUid983::sQuestUid983(unsigned long u) : uid(u)
{
}

// 005847b0
bool sQuestUid983::operator()(const _quest::CQuestItem& q) const
{
	return q.GetUid983() == uid;
}

struct sQuestUid983b
{
	unsigned long uid;
	sQuestUid983b(unsigned long u);
	bool operator()(const _quest::CQuestItem& q) const;
};

// 005847d0
sQuestUid983b::sQuestUid983b(unsigned long u) : uid(u)
{
}

// 005847e0
bool sQuestUid983b::operator()(const _quest::CQuestItem& q) const
{
	return q.GetUid983() == uid;
}

// 005846c0
CItemUpdater::CItemUpdater()
{
}

// 005846d0
CItemUpdater::~CItemUpdater()
{
}

// 005861a0: contagem de missão nova (CSharedDoc::AddQuestCount983 fora de linha)
int CItemUpdater::NewQuestCount983(__time64_t now, const sItemUpdate983& rec)
{
	_quest::sQuestCount983 c;
	c.typeId = rec.typeId;
	c.uid = rec.uid;
	c.count = rec.count;
	c.bValid = true;
	CSharedDoc::Instance()->AddQuestCount983(c);
	return 0;
}

// 00588230
sItemInfo* CItemUpdater::FindPart983(const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_partsList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 00588270
sItemInfo* CItemUpdater::FindUccItem983(const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_uccItemList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 005882b0
sItemInfo* CItemUpdater::FindBall983(const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_ballList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 005882f0
sItemInfo* CItemUpdater::FindMyItem983(const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_myItemList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 00588330
sItemInfo* CItemUpdater::FindMyItem2_983(const sItemUpdate983& rec)
{
	return FindMyItem983(rec);
}

// 00588340
sItemInfo* CItemUpdater::FindCutin983(const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_cutinList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 00588380: categoria 0xf (estilo de cabelo): a cor do cabelo do personagem dono; avisa o ator principal (800)
int CItemUpdater::SetHairStyle983(__time64_t now, const sItemUpdate983& rec)
{
	IFF_STRUCT::sHairStyle* pHair = CSharedDoc::Instance()->m_itemManager.FindHairStyle(rec.typeId);
	if (!pHair)
		return 0x1e851b;
	std::map<unsigned int, sCharacterInfo>& charMap = CSharedDoc::Instance()->m_charMap;
	for (std::map<unsigned int, sCharacterInfo>::iterator it = charMap.begin(); it != charMap.end(); ++it)
	{
		if (it->second.tid == (pHair->cCharID | 0x4000000))
		{
			it->second.hairClr = pHair->cHairID;
			if (IsLocalContent((localContentType_t)0x61))
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 800, (int)pHair, 0, 0, 0, 0));
			return 0;
		}
	}
	return 0x1e851c;
}

// 00588470
_quest::sQuestCount983* CItemUpdater::FindQuestCount983(const sItemUpdate983& rec)
{
	std::list<_quest::sQuestCount983>& l = CSharedDoc::Instance()->m_questCounts983;
	unsigned long uid = rec.uid;
	std::list<_quest::sQuestCount983>::iterator it = std::find_if(l.begin(), l.end(), [uid](const _quest::sQuestCount983& e) { return uid == e.uid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 005884b0: categoria 0x1b
int CItemUpdater::AddQuestCount983(__time64_t now, const sItemUpdate983& rec)
{
	_quest::sQuestCount983* p = FindQuestCount983(rec);
	if (!p)
		return NewQuestCount983(now, rec);
	if (IsValidPeriod983(rec.periodType) == true)
		return 0x1e8535;
	p->count += rec.count;
	return 0;
}

// 00588510
sItemInfo* CItemUpdater::FindAuxPart983(const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_auxPartList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 00588550
_quest::CQuestItem* CItemUpdater::FindQuest983(const sItemUpdate983& rec)
{
	std::list<_quest::CQuestItem>& l = CSharedDoc::Instance()->m_quests983;
	std::list<_quest::CQuestItem>::iterator it = std::find_if(l.begin(), l.end(), sQuestUid983(rec.uid));
	if (CSharedDoc::Instance()->m_quests983.end() == it)
		return NULL;
	return &*it;
}

// 005885c0
sCards* CItemUpdater::FindCard983(const sItemUpdate983& rec)
{
	std::list<sCards>& l = CSharedDoc::Instance()->m_cardList;
	unsigned long uid = rec.uid;
	std::list<sCards>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sCards& e) { return uid == e.uid; });
	if (l.end() == it)
		return NULL;
	return &*it;
}

// 00588b50
sCharacterInfo* CItemUpdater::FindCharacter983(const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sCharacterInfo>::iterator it = pDoc->m_charMap.find(rec.uid);
	if (it == pDoc->m_charMap.end())
		return NULL;
	return &it->second;
}

// 00588ba0: 0xc9, categoria 1 — PCL do personagem
int CItemUpdater::SetCharacterPCL983(__time64_t now, const sItemUpdate983& rec)
{
	sCharacterInfo* p = FindCharacter983(rec);
	if (!p)
		return 0x1e8568;
	p->PCL[0] = (char)rec.counters[0];
	p->PCL[1] = (char)rec.counters[1];
	p->PCL[2] = (char)rec.counters[2];
	p->PCL[3] = (char)rec.counters[3];
	p->PCL[4] = (char)rec.counters[4];
	return 0;
}

// monta a habilidade da carta posta no slot (os três SetXxxCard983 fazem igual)
#define ITEMUPDATER_CARD_INFO(info, rec) \
	(info).tid = (rec).cardTypeId; \
	(info).partsTid = (rec).typeId; \
	(info).partsUid = (rec).uid; \
	(info).cardType = ((rec).cardTypeId >> 22) & 0xf; \
	(info).slotNum = (rec).cardSlot; \
	if (IFF_STRUCT::sCard* pCard = CSharedDoc::Instance()->m_itemManager.FindCard((info).tid)) \
	{ \
		(info).Avility = pCard->Avility; \
		(info).AvilityValue = pCard->AvilityValue; \
	} \
	else \
	{ \
		(info).Avility = 0; \
		(info).AvilityValue = 0; \
	}

// 00588c00: 0xcb, categoria 1 — carta no personagem
int CItemUpdater::SetCharacterCard983(__time64_t now, const sItemUpdate983& rec)
{
	sCharacterInfo* p = FindCharacter983(rec);
	if (!p)
		return 0x1e8569;
	if (!rec.cardSlot)
		return 0x1e856a;
	p->card[0][rec.cardSlot - 1] = rec.cardTypeId;
	sSCardAvilityPeriodInfo info;
	ITEMUPDATER_CARD_INFO(info, rec)
	std::list<sSCardAvilityPeriodInfo>& l = Doc983()->m_cardAbilityList[0];
	std::list<sSCardAvilityPeriodInfo>::iterator it = std::find_if(l.begin(), l.end(),
		[&info](const sSCardAvilityPeriodInfo& c) {
			return c.partsUid == info.partsUid && c.cardType == info.cardType && c.slotNum == info.slotNum;
		});
	if (it != l.end())
		l.erase(it);
	Doc983()->AddCardAbility983(info);
	return 0;
}

// 00588d30: 0xcd — mestria do personagem
int CItemUpdater::SetCharacterMastery983(__time64_t now, const sItemUpdate983& rec)
{
	sCharacterInfo* p = FindCharacter983(rec);
	if (!p)
		return 0x1e856b;
	p->mastery = rec.mastery;
	return 0;
}

// 00588d60: 0xcb, categoria 2 — carta numa parte
int CItemUpdater::SetPartCard983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindPart983(rec);
	if (!p)
		return 0x1e8551;
	if (!rec.cardSlot)
		return 0x1e8552;
	p->attachCard[rec.cardSlot - 1] = rec.cardTypeId;
	sSCardAvilityPeriodInfo info;
	ITEMUPDATER_CARD_INFO(info, rec)
	std::list<sSCardAvilityPeriodInfo>& l = Doc983()->m_cardAbilityList[0];
	std::list<sSCardAvilityPeriodInfo>::iterator it = std::find_if(l.begin(), l.end(),
		[&info](const sSCardAvilityPeriodInfo& c) {
			return c.partsUid == info.partsUid && c.cardType == info.cardType && c.slotNum == info.slotNum;
		});
	if (it != l.end())
		l.erase(it);
	Doc983()->AddCardAbility983(info);
	return 0;
}

// 00588e80: Remove, categoria 2
int CItemUpdater::RemovePart983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_partsList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return 0x1e8506;
	Doc983()->RemoveCardAbilityByPart983(rec.uid);
	CSharedDoc::Instance()->m_partsList.erase(it);
	return 0;
}

// 00588f00: Remove, item UCC
int CItemUpdater::RemoveUccItem983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_uccItemList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return 0x1e850a;
	Doc983()->RemoveCardAbilityByPart983(rec.uid);
	CSharedDoc::Instance()->m_uccItemList.erase(it);
	return 0;
}

// 00588f80: Remove, categoria 4
int CItemUpdater::RemoveClubSet983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	unsigned long uid = rec.uid;
	std::map<unsigned int, sItemInfo>::iterator it = pDoc->m_clubSetMap.find(uid);
	if (it == pDoc->m_clubSetMap.end())
		return 0x1e850b;
	Doc983()->RemoveCardAbilityByPart983(uid);
	CSharedDoc::Instance()->m_clubSetMap.erase(it);
	return 0;
}

// 00588ff0: 0xc9, categoria 4
int CItemUpdater::SetClubSetCounters983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sItemInfo>::iterator it = pDoc->m_clubSetMap.find(rec.uid);
	if (it == pDoc->m_clubSetMap.end())
		return 0x1e8553;
	it->second.Common[0] = rec.counters[0];
	it->second.Common[1] = rec.counters[1];
	it->second.Common[2] = rec.counters[2];
	it->second.Common[3] = rec.counters[3];
	it->second.Common[4] = rec.counters[4];
	return 0;
}

// 00589070: 0xcb, categoria 4 — carta no taco
int CItemUpdater::SetClubSetCard983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sItemInfo>::iterator itClub = pDoc->m_clubSetMap.find(rec.uid);
	if (itClub == pDoc->m_clubSetMap.end())
		return 0x1e8554;
	if (!rec.cardSlot)
		return 0x1e8555;
	itClub->second.attachCard[rec.cardSlot - 1] = rec.cardTypeId;
	sSCardAvilityPeriodInfo info;
	ITEMUPDATER_CARD_INFO(info, rec)
	std::list<sSCardAvilityPeriodInfo>& l = Doc983()->m_cardAbilityList[0];
	std::list<sSCardAvilityPeriodInfo>::iterator it = std::find_if(l.begin(), l.end(),
		[&info](const sSCardAvilityPeriodInfo& c) {
			return c.partsUid == info.partsUid && c.cardType == info.cardType && c.slotNum == info.slotNum;
		});
	if (it != l.end())
		l.erase(it);
	Doc983()->AddCardAbility983(info);
	return 0;
}

// 005891c0: 0xcc — oficina de tacos (rabo do sItemInfo do 983: slotUp983/clubExp983/recoveryUsed983)
int CItemUpdater::SetClubSetWorkshop983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sItemInfo>::iterator it = pDoc->m_clubSetMap.find(rec.uid);
	if (it == pDoc->m_clubSetMap.end())
		return 0x1e8556;
	sItemInfo& item = it->second;
	item.slotUp983[0] = rec.workshop.slots[0];
	item.slotUp983[1] = rec.workshop.slots[1];
	item.slotUp983[2] = rec.workshop.slots[2];
	item.slotUp983[3] = rec.workshop.slots[3];
	item.slotUp983[4] = rec.workshop.slots[4];
	item.clubExp983 = rec.workshop.exp;
	item.workshopSrvByte983 = rec.workshop.srvByte983;
	item.workshopSrvInt983 = rec.workshop.srvInt983;
	item.recoveryUsed983 = rec.workshop.recovery983;
	return 0;
}
// A418: offsets que o leitor 009bfc90 e o 005891c0 usam (maestria nova do taco depois do Transfer da Maintenance; string +0x28 = move-assign)
C_ASSERT(offsetof(sItemUpdate983, uid) == 8 && offsetof(sItemUpdate983, counters) == 0x1c && offsetof(sItemUpdate983, text) == 0x28);
C_ASSERT_VC10(offsetof(sItemUpdate983, cardSlot) == 0x50 && offsetof(sItemUpdate983, workshop) == 0x54);
C_ASSERT_VC10(offsetof(sItemUpdate983, workshop.exp) == 0x60 && offsetof(sItemUpdate983, workshop.recovery983) == 0x6c);
C_ASSERT_VC10(offsetof(sItemUpdate983, mastery) == 0x70 && offsetof(sItemUpdate983, caddieExp) == 0x78);
C_ASSERT(offsetof(sItemInfo, NpcSlotNum983) == 0xa8 && offsetof(sItemInfo, slotUp983) == 0xaa && offsetof(sItemInfo, clubExp983) == 0xb4);
C_ASSERT(offsetof(sItemInfo, recoveryUsed983) == 0xb8 && offsetof(sItemInfo, workshopSrvByte983) == 0xbc && sizeof(sItemInfo) == 0xc4);

// 00589270: Remove, bola
int CItemUpdater::RemoveBall983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_ballList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return 0x1e8510;
	l.erase(it);
	// a bola equipada sumiu: equipa a bola padrão
	CSharedDoc* pDoc = CSharedDoc::Instance();
	if (pDoc->m_myInfo.userEquip.tidBall == rec.typeId)
	{
		unsigned long tid = 0x14000000;
		if ((pDoc->m_myInfo.info.dwIdentity >> 14) & 1)
			tid = 0x140000d8;
		WSendPacket packet((enumClientPacket)0xb);
		packet.Encode1(2);
		packet.Encode4(tid);
		packet.Send((eSendTo)0);
	}
	return 0;
}

// 00589380: Remove, item
int CItemUpdater::RemoveMyItem983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_myItemList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return 0x1e8514;
	l.erase(it);
	return 0;
}

// 005893e0
int CItemUpdater::RemoveMyItem2_983(__time64_t now, const sItemUpdate983& rec)
{
	return RemoveMyItem983(now, rec);
}

// 00589400: Remove, categoria 7
int CItemUpdater::RemoveCaddie983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sCaddieInfo>::iterator it = pDoc->m_caddieMap.find(rec.uid);
	if (it == pDoc->m_caddieMap.end())
		return 0x1e8515;
	pDoc->m_caddieMap.erase(it);
	return 0;
}

// 00589460: 0xce — nível e experiência do caddie
int CItemUpdater::SetCaddieLevel983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sCaddieInfo>::iterator it = pDoc->m_caddieMap.find(rec.uid);
	if (it == pDoc->m_caddieMap.end())
		return 0x1e856f;
	it->second.Level = rec.caddieLevel;
	it->second.Exp = rec.caddieExp;
	return 0;
}

// 005894c0: Add, categoria 8 (roupa do caddie; = caso 8 do AddNewItemToDoc do 645)
int CItemUpdater::SetCaddiePart983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sCaddieInfo>::iterator it = pDoc->m_caddieMap.find(rec.uid);
	if (it == pDoc->m_caddieMap.end())
		return 0x1e8519;
	int hours = PeriodHours983(rec.periodType, rec.count);
	it->second.tidPart = rec.typeId;
	it->second.Remain_Partdate = (unsigned short)hours;
	if (it->second.Remain_Date < hours / 24)
		it->second.Remain_Date = (unsigned short)(hours / 24);
	it->second.Rent_flag = 1;
	return 0;
}

// 00589550: Remove, categoria 0x10
int CItemUpdater::RemoveMascot983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sMascotInfo>::iterator it = pDoc->m_mascotMap.find(rec.uid);
	if (it == pDoc->m_mascotMap.end())
		return 0x1e851d;
	pDoc->m_mascotMap.erase(it);
	return 0;
}

// 005895b0: Remove, contagem de missão
int CItemUpdater::RemoveQuestCount983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<_quest::sQuestCount983>& l = CSharedDoc::Instance()->m_questCounts983;
	unsigned long uid = rec.uid;
	std::list<_quest::sQuestCount983>::iterator it = std::find_if(l.begin(), l.end(), [uid](const _quest::sQuestCount983& e) { return uid == e.uid; });
	if (l.end() == it)
		return 0x1e8536;
	l.erase(it);
	return 0;
}

// 00589610: Remove, parte auxiliar
int CItemUpdater::RemoveAuxPart983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<sItemInfo>& l = CSharedDoc::Instance()->m_auxPartList;
	unsigned long uid = rec.uid;
	std::list<sItemInfo>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sItemInfo& e) { return uid == e.guid; });
	if (l.end() == it)
		return 0x1e8523;
	l.erase(it);
	// equipada: tira de todos os personagens
	if (CSharedDoc::Instance()->IsEquipParts(rec.uid, rec.typeId))
	{
		std::map<unsigned int, sCharacterInfo>& charMap = CSharedDoc::Instance()->m_charMap;
		for (std::map<unsigned int, sCharacterInfo>::iterator itChar = charMap.begin(); itChar != charMap.end(); itChar++)
		{
			unsigned int slot = (rec.typeId >> 21) & 0x1f;
			if (slot < 5)
				itChar->second.tidAuxParts[slot] = 0;
		}
	}
	return 0;
}

// 005896d0: Remove, carta
int CItemUpdater::RemoveCard983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<sCards>& l = CSharedDoc::Instance()->m_cardList;
	unsigned long uid = rec.uid;
	std::list<sCards>::iterator it = std::find_if(l.begin(), l.end(), [uid](const sCards& e) { return uid == e.uid; });
	if (l.end() == it)
		return 0x1e8525;
	l.erase(it);
	return 0;
}

// 00589b50
std::tr1::shared_ptr<CItemUpdater> CreateItemUpdater983()
{
	return std::tr1::shared_ptr<CItemUpdater>(new CItemUpdater);
}

// 00589ba0: 0xc9 — contadores (PCL do personagem / Common do taco)
int CItemUpdater::UpdateCounters983(__time64_t now, const sItemUpdate983& rec)
{
	int r;
	switch (rec.typeId >> 26)
	{
	case 1:
		r = SetCharacterPCL983(now, rec);
		if (r)
			return r;
		break;
	case 4:
		r = SetClubSetCounters983(now, rec);
		if (r)
			return r;
		break;
	default:
		return 0x1e854e;
	}
	return 0;
}

// 00589bf0: 0xcb — carta posta num slot
int CItemUpdater::UpdateCard983(__time64_t now, const sItemUpdate983& rec)
{
	int r;
	switch (rec.typeId >> 26)
	{
	case 1:
		r = SetCharacterCard983(now, rec);
		if (r)
			return r;
		break;
	case 2:
		r = SetPartCard983(now, rec);
		if (r)
			return r;
		break;
	case 4:
		r = SetClubSetCard983(now, rec);
		if (r)
			return r;
		break;
	default:
		return 0x1e854f;
	}
	return 0;
}

// 00589c60: 0xcc — oficina de tacos
int CItemUpdater::UpdateWorkshop983(__time64_t now, const sItemUpdate983& rec)
{
	if ((rec.typeId & 0xfc000000) != 0x10000000)
		return 0x1e8550;
	return SetClubSetWorkshop983(now, rec);
}

// 00589ca0: 0xcd — mestria
int CItemUpdater::UpdateMastery983(__time64_t now, const sItemUpdate983& rec)
{
	switch (rec.typeId >> 26)
	{
	case 1:
		return SetCharacterMastery983(now, rec);
	}
	return 0x1e8567;
}

// 00589cd0: 0xce — nível do caddie
int CItemUpdater::UpdateCaddieLevel983(__time64_t now, const sItemUpdate983& rec)
{
	if ((rec.typeId & 0xfc000000) != 0x1c000000)
		return 0x1e856e;
	return SetCaddieLevel983(now, rec);
}

// 00589d10: parte nova com prazo
int CItemUpdater::NewPartTimed983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.ItemType = rec.periodType;
	info.HourRemain = PeriodHours983(rec.periodType, rec.count);
	info.Common[0] = rec.counters[0];
	info.Common[1] = rec.counters[1];
	info.Common[2] = rec.counters[2];
	info.Common[3] = rec.counters[3];
	info.Common[4] = rec.counters[4];
	info.IsValid = 1;
	CSharedDoc::Instance()->m_partsList.push_back(info);
	return 0;
}

// 00589e10
int CItemUpdater::RemoveUccChecked983(__time64_t now, const sItemUpdate983& rec)
{
	if (!FindUccItem983(rec))
		return 0x1e8507;
	return RemoveUccItem983(now, rec);
}

// 00589e50: roupa UCC nova (índice UCC e número de série vêm do registro)
int CItemUpdater::NewUccItem983(__time64_t now, const sItemUpdate983& rec)
{
	if (FindUccItem983(rec))
		return 0;
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	strncpy(info.UccIndex, rec.text.c_str(), 9);
	info.status = (unsigned char)rec.uccStatus983;
	info.Seq = (unsigned short)rec.uccSeq983;
	info.IsValid = 1;
	CSharedDoc::Instance()->m_uccItemList.push_back(info);
	return 0;
}

// 00589f40: Remove, categoria 5 — contagem de bolas
int CItemUpdater::UpdateBallCount983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindBall983(rec);
	if (!p)
		return 0x1e850e;
	if (IsValidPeriod983(rec.periodType) == true)
		return 0x1e850f;
	p->Common[0] = (short)rec.total;
	if (rec.total == 0)
	{
		int r = RemoveBall983(now, rec);
		if (r)
			return r;
	}
	return 0;
}

// 00589fb0
int CItemUpdater::NewBall983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.IsValid = 1;
	info.ItemType = rec.periodType;
	info.Common[0] = (short)rec.count;
	CSharedDoc::Instance()->m_ballList.push_back(info);
	return 0;
}

// 0058a080: item novo com prazo em horas/dias
int CItemUpdater::NewMyItemTimed983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.ItemType = rec.periodType;
	info.HourRemain = PeriodHours983(rec.periodType, rec.count);
	info.Common[0] = rec.counters[0];
	info.Common[1] = rec.counters[1];
	info.Common[2] = rec.counters[2];
	info.Common[3] = rec.counters[3];
	info.Common[4] = rec.counters[4];
	info.IsValid = 1;
	CSharedDoc::Instance()->m_myItemList.push_back(info);
	return 0;
}

// 0058a180: Remove, categoria 6 subtipo 1 — item com data de fim (total = fim; 0 = já venceu)
int CItemUpdater::UpdateMyItemTime983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindMyItem983(rec);
	if (!p)
		return 0x1e855e;
	if (!IsValidPeriod983(rec.periodType))
		return 0x1e855f;
	__time64_t end;
	if (rec.total == 0)
		end = now;
	else
		end = rec.total;
	p->ItemType = rec.periodType;
	p->HourRemain = RemainHours983(end, now);
	*(__time64_t*)((char*)p + 0x20) = end;
	if (p->HourRemain <= 0)
	{
		int r = RemoveMyItem2_983(now, rec);
		if (r)
			return r;
	}
	return 0;
}

// 0058a230: item novo com data de fim
int CItemUpdater::NewMyItemTime983(__time64_t now, const sItemUpdate983& rec)
{
	if (!IsValidPeriod983(rec.periodType))
		return 0;
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.ItemType = rec.periodType;
	info.HourRemain = RemainHours983(rec.total, now);
	info.Common[0] = rec.counters[0];
	info.Common[1] = rec.counters[1];
	info.Common[2] = rec.counters[2];
	info.Common[3] = rec.counters[3];
	info.Common[4] = rec.counters[4];
	info.IsValid = 1;
	*(__time64_t*)((char*)&info + 0x20) = rec.total;
	CSharedDoc::Instance()->m_myItemList.push_back(info);
	return 0;
}

// 0058a370: cut-in novo com prazo
int CItemUpdater::NewCutinTimed983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.ItemType = rec.periodType;
	info.HourRemain = PeriodSeconds983(rec.periodType, rec.count) / 3600;
	info.IsValid = 1;
	CSharedDoc::Instance()->m_cutinList.push_back(info);
	return 0;
}

// 0058a450: móvel (posição padrão do Furniture.iff)
int CItemUpdater::NewFurniture983(__time64_t now, const sItemUpdate983& rec)
{
	IFF_STRUCT::sFurniture* pFurniture = CSharedDoc::Instance()->m_itemManager.FindFurniture(rec.typeId);
	if (!pFurniture)
		return 0x1e8521;
	sFurniture_List f;
	f.id = rec.uid;
	f.typeId = rec.typeId;
	f.x = pFurniture->Pos[0];
	f.y = pFurniture->Pos[1];
	f.z = pFurniture->Pos[2];
	f.r = pFurniture->Pos[3];
	f.bArrange = 0;
	CSharedDoc::Instance()->m_furnitureList.push_back(f);
	return 0;
}

// 0058a510: Remove, categoria 0x1b — contagem de missão
int CItemUpdater::UpdateQuestCount983(__time64_t now, const sItemUpdate983& rec)
{
	_quest::sQuestCount983* p = FindQuestCount983(rec);
	if (!p)
		return 0x1e8533;
	if (IsValidPeriod983(rec.periodType) == true)
		return 0x1e8534;
	p->count += rec.count;
	if (rec.total == 0)
	{
		int r = RemoveQuestCount983(now, rec);
		if (r)
			return r;
	}
	return 0;
}

// 0058a570: Remove, categoria 0x1c
int CItemUpdater::UpdateAuxPartCount983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindAuxPart983(rec);
	if (!p)
		return 0x1e8522;
	p->Common[0] = (short)rec.total;
	if (rec.total == 0)
	{
		int r = RemoveAuxPart983(now, rec);
		if (r)
			return r;
	}
	return 0;
}

// 0058a5c0: parte auxiliar nova com prazo
int CItemUpdater::NewAuxPartTimed983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.ItemType = rec.periodType;
	info.HourRemain = PeriodHours983(rec.periodType, rec.count);
	info.Common[0] = rec.counters[0];
	info.Common[1] = rec.counters[1];
	info.Common[2] = rec.counters[2];
	info.Common[3] = rec.counters[3];
	info.Common[4] = rec.counters[4];
	info.IsValid = 1;
	CSharedDoc::Instance()->m_auxPartList.push_back(info);
	return 0;
}

// 0058a6c0
int CItemUpdater::EraseQuest983(__time64_t now, const sItemUpdate983& rec)
{
	std::list<_quest::CQuestItem>& l = CSharedDoc::Instance()->m_quests983;
	std::list<_quest::CQuestItem>::iterator it = std::find_if(l.begin(), l.end(), sQuestUid983b(rec.uid));
	CSharedDoc* pDoc = CSharedDoc::Instance();
	if (it == pDoc->m_quests983.end())
		return 0x1e8539;
	pDoc->m_quests983.erase(it);
	return 0;
}

// 0058a760: Remove, categoria 0x1f
int CItemUpdater::UpdateCardCount983(__time64_t now, const sItemUpdate983& rec)
{
	sCards* p = FindCard983(rec);
	if (!p)
		return 0x1e8524;
	p->cardCount += rec.count;
	if (rec.total == 0)
	{
		int r = RemoveCard983(now, rec);
		if (r)
			return r;
	}
	return 0;
}

// 0058a7b0
int CItemUpdater::NewCard983(__time64_t now, const sItemUpdate983& rec)
{
	sCards card = { rec.uid };
	card.tid = rec.typeId;
	card.cardCount = rec.count;
	CSharedDoc::Instance()->m_cardList.push_back(card);
	return 0;
}

// 0058a880
int CItemUpdater::NewItemCount983(__time64_t now, const sItemUpdate983& rec)
{
	sItemCount983 e;
	e.uid = rec.uid;
	e.typeId = rec.typeId;
	e.count = rec.count;
	ItemCountList983().push_back(e);
	return 0;
}

// 0058aac0: personagem novo (peças padrão do Character.iff)
int CItemUpdater::NewCharacter983(__time64_t now, const sItemUpdate983& rec)
{
	sCharacterInfo info;
	info.guid = rec.uid;
	info.tid = rec.typeId;
	CSharedDoc::Instance()->m_itemManager.GetDefCombo(info.tid, info.tidParts);
	if (!CSharedDoc::Instance()->m_charMap.insert(std::map<unsigned int, sCharacterInfo>::value_type(info.guid, info)).second)
		return 0x1e8503;
	return 0;
}

// 0058ab80: Remove, categoria 2
int CItemUpdater::RemovePartItem983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsUccClothes(rec.typeId) == true)
		return RemoveUccChecked983(now, rec);
	if (!FindPart983(rec))
		return 0x1e8504;
	return RemovePart983(now, rec);
}

// 0058abe0
int CItemUpdater::NewPart983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsValidPeriod983(rec.periodType) == true)
		return NewPartTimed983(now, rec);
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.IsValid = 1;
	CSharedDoc::Instance()->m_partsList.push_back(info);
	return 0;
}

// 0058acb0
int CItemUpdater::AddUccItem983(__time64_t now, const sItemUpdate983& rec)
{
	if (FindUccItem983(rec))
		return 0x1e8508;
	return NewUccItem983(now, rec);
}

// 0058acf0
int CItemUpdater::NewClubSet983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo info;
	info.IsValid = 1;
	info.Common[0] = rec.counters[0];
	info.Common[2] = rec.counters[2];
	info.Common[1] = rec.counters[1];
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.Common[4] = rec.counters[4];
	info.Common[3] = rec.counters[3];
	if (!CSharedDoc::Instance()->m_clubSetMap.insert(std::map<unsigned int, sItemInfo>::value_type(info.guid, info)).second)
		return 0x1e850d;
	return 0;
}

// 0058add0: Add, categoria 5 — bola nova; quem tem o bit 14 do dwIdentity e está com a bola padrão troca pela 0x140000d8
int CItemUpdater::AddBall983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindBall983(rec);
	if (p)
	{
		p->Common[0] = (short)rec.total;
		return 0;
	}
	int r = NewBall983(now, rec);
	if (r)
		return r;
	CSharedDoc* pDoc = CSharedDoc::Instance();
	if ((pDoc->m_myInfo.info.dwIdentity >> 14) & 1)
	{
		if (pDoc->m_myInfo.userEquip.tidBall == 0x14000000)
		{
			WSendPacket packet((enumClientPacket)0xb);
			packet.Encode1(2);
			packet.Encode4(0x140000d8);
			packet.Send((eSendTo)0);
		}
	}
	return 0;
}

// 0058aea0: Remove, categoria 6
int CItemUpdater::UpdateMyItemCount983(__time64_t now, const sItemUpdate983& rec)
{
	if ((rec.typeId & 0xfc000000) == 0x18000000 && (rec.typeId & 0x1f00000) == 0x100000)
		return UpdateMyItemTime983(now, rec);
	sItemInfo* p = FindMyItem983(rec);
	if (!p)
		return 0x1e8511;
	if (IsValidPeriod983(rec.periodType) == true)
		return 0x1e8512;
	p->Common[0] = (short)rec.total;
	if (rec.total == 0)
	{
		int r = RemoveMyItem983(now, rec);
		if (r)
			return r;
	}
	return 0;
}

// 0058af40
int CItemUpdater::NewMyItem983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsValidPeriod983(rec.periodType) == true)
		return NewMyItemTimed983(now, rec);
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.IsValid = 1;
	info.ItemType = rec.periodType;
	info.Common[0] = (short)rec.count;
	CSharedDoc::Instance()->m_myItemList.push_back(info);
	return 0;
}

// 0058b030: Add, categoria 6 subtipo 1
int CItemUpdater::AddMyItemTime983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindMyItem983(rec);
	if (!p)
		return NewMyItemTime983(now, rec);
	if (!IsValidPeriod983(rec.periodType))
		return 0x1e8560;
	p->ItemType = rec.periodType;
	p->HourRemain = RemainHours983(rec.total, now);
	*(__time64_t*)((char*)p + 0x20) = rec.total;
	return 0;
}

// 0058b0b0: caddie novo com prazo (dias)
int CItemUpdater::NewCaddieTimed983(__time64_t now, const sItemUpdate983& rec)
{
	sCaddieInfo info = { rec.uid };
	info.tid = rec.typeId;
	int hours = PeriodHours983(rec.periodType, rec.count);
	info.Rent_flag = 1;
	info.Remain_Date = (unsigned short)(hours / 24);
	info.Level = (unsigned char)rec.counters[1];
	info.Exp = rec.counters[2];
	if (!CSharedDoc::Instance()->m_caddieMap.insert(std::map<unsigned int, sCaddieInfo>::value_type(info.guid, info)).second)
		return 0x1e8518;
	return 0;
}

// 0058b160
int CItemUpdater::NewCutin983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsValidPeriod983(rec.periodType) == true)
		return NewCutinTimed983(now, rec);
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.IsValid = 1;
	info.ItemType = rec.periodType;
	CSharedDoc::Instance()->m_cutinList.push_back(info);
	return 0;
}

// 0058b250: mascote nova com data de fim
int CItemUpdater::NewMascotTimed983(__time64_t now, const sItemUpdate983& rec)
{
	sMascotInfo info = { rec.uid };
	info.tid = rec.typeId;
	MASCOT983_TIMED(info) = 1;
	S5::SecToSysTime983(&MASCOT983_END(info), rec.total);
	strcpy(info.szMsg, "PANGYA!");
	if (!CSharedDoc::Instance()->m_mascotMap.insert(std::map<unsigned int, sMascotInfo>::value_type(info.guid, info)).second)
		return 0x1e8520;
	return 0;
}

// 0058b310: Add, categoria 0x12
int CItemUpdater::AddFurniture983(__time64_t now, const sItemUpdate983& rec)
{
	return NewFurniture983(now, rec);
}

// 0058b330
int CItemUpdater::NewAuxPart983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsValidPeriod983(rec.periodType) == true)
		return NewAuxPartTimed983(now, rec);
	sItemInfo info;
	info.tid = rec.typeId;
	info.guid = rec.uid;
	info.IsValid = 1;
	info.ItemType = rec.periodType;
	info.Common[0] = (short)rec.count;
	CSharedDoc::Instance()->m_auxPartList.push_back(info);
	return 0;
}

// 0058b420: Remove, categoria 0x1e
int CItemUpdater::RemoveQuest983(__time64_t now, const sItemUpdate983& rec)
{
	if (!FindQuest983(rec))
		return 0x1e8537;
	if (IsValidPeriod983(rec.periodType) == true)
		return 0x1e8538;
	return EraseQuest983(now, rec);
}

// 0058b470: missão nova (estado START)
int CItemUpdater::NewQuest983(__time64_t now, const sItemUpdate983& rec)
{
	_quest::CQuestItem quest(rec.typeId, rec.uid);
	CSharedDoc::Instance()->m_quests983.push_back(quest);
	return 0;
}

// 0058b540: Add, categoria 0x1f
int CItemUpdater::AddCard983(__time64_t now, const sItemUpdate983& rec)
{
	sCards* p = FindCard983(rec);
	if (!p)
		return NewCard983(now, rec);
	p->cardCount += rec.count;
	return 0;
}

// 0058b580: contagem por typeId na lista +0x3828 (a lambda leva o registro inteiro por valor)
int CItemUpdater::UpdateItemCount983(__time64_t now, const sItemUpdate983& rec)
{
	if (!rec.uid)
		return 0x1e8557;
	if (!rec.typeId)
		return 0x1e8558;
	auto pred = [rec](const sItemCount983& e) { return rec.typeId == e.typeId; };
	std::list<sItemCount983>::iterator it = std::find_if(ItemCountList983().begin(), ItemCountList983().end(), pred);
	if (it == ItemCountList983().end())
	{
		int r = NewItemCount983(now, rec);
		if (r)
			return r;
	}
	else
		it->count += rec.count;
	return 0;
}

// 0058b690: Add, categoria 1
int CItemUpdater::AddCharacter983(__time64_t now, const sItemUpdate983& rec)
{
	return NewCharacter983(now, rec);
}

// 0058b6b0: Add, categoria 2
int CItemUpdater::AddPart983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsUccClothes(rec.typeId) == true)
		return AddUccItem983(now, rec);
	sItemInfo* p = FindPart983(rec);
	if (!p)
		return NewPart983(now, rec);
	if (!IsValidPeriod983(rec.periodType))
		return 0x1e8505;
	p->ItemType = rec.periodType;
	p->HourRemain = PeriodHours983(rec.periodType, rec.count);
	p->Common[0] = rec.counters[0];
	p->Common[1] = rec.counters[1];
	p->Common[2] = rec.counters[2];
	p->Common[3] = rec.counters[3];
	p->Common[4] = rec.counters[4];
	return 0;
}

// 0058b770: Add, categoria 4
int CItemUpdater::AddClubSet983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sItemInfo>::iterator it = pDoc->m_clubSetMap.find(rec.uid);
	if (pDoc->m_clubSetMap.end() != it)
		return 0x1e850c;
	return NewClubSet983(now, rec);
}

// 0058b7d0: Add, categoria 6
int CItemUpdater::AddMyItem983(__time64_t now, const sItemUpdate983& rec)
{
	if ((rec.typeId & 0xfc000000) == 0x18000000 && (rec.typeId & 0x1f00000) == 0x100000)
		return AddMyItemTime983(now, rec);
	sItemInfo* p = FindMyItem983(rec);
	if (!p)
		return NewMyItem983(now, rec);
	if (IsValidPeriod983(rec.periodType) == true)
		return 0x1e8513;
	p->Common[0] = (short)rec.total;
	return 0;
}

// 0058b860
int CItemUpdater::NewCaddie983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsValidPeriod983(rec.periodType) == true)
		return NewCaddieTimed983(now, rec);
	sCaddieInfo info = { rec.uid };
	info.tid = rec.typeId;
	info.Rent_flag = 0;
	info.Remain_Date = 0;
	info.Level = (unsigned char)rec.counters[1];
	info.Exp = rec.counters[2];
	if (!CSharedDoc::Instance()->m_caddieMap.insert(std::map<unsigned int, sCaddieInfo>::value_type(info.guid, info)).second)
		return 0x1e8517;
	return 0;
}

// 0058b910: Add, categoria 0xe
int CItemUpdater::AddCutin983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindCutin983(rec);
	if (!p)
		return NewCutin983(now, rec);
	if (!IsValidPeriod983(rec.periodType))
		return 0x1e851a;
	p->ItemType = rec.periodType;
	p->HourRemain = PeriodHours983(rec.periodType, rec.count);
	return 0;
}

// 0058b980
int CItemUpdater::NewMascot983(__time64_t now, const sItemUpdate983& rec)
{
	if (IsValidPeriod983(rec.periodType) == true)
		return NewMascotTimed983(now, rec);
	sMascotInfo info = { rec.uid };
	info.tid = rec.typeId;
	strcpy(info.szMsg, "PANGYA!");
	MASCOT983_TIMED(info) = 0;
	if (!CSharedDoc::Instance()->m_mascotMap.insert(std::map<unsigned int, sMascotInfo>::value_type(info.guid, info)).second)
		return 0x1e851f;
	return 0;
}

// 0058ba40: Remove, categoria 0x13
int CItemUpdater::RemoveQuestItem983(__time64_t now, const sItemUpdate983& rec)
{
	return RemoveQuest983(now, rec);
}

// 0058ba60: Add, categoria 0x1c
int CItemUpdater::AddAuxPart983(__time64_t now, const sItemUpdate983& rec)
{
	sItemInfo* p = FindAuxPart983(rec);
	if (!p)
		return NewAuxPart983(now, rec);
	if (IsValidPeriod983(rec.periodType) == true)
	{
		p->ItemType = rec.periodType;
		p->HourRemain = PeriodHours983(rec.periodType, rec.count);
		p->Common[0] = rec.counters[0];
		p->Common[1] = rec.counters[1];
		p->Common[2] = rec.counters[2];
		p->Common[3] = rec.counters[3];
		p->Common[4] = rec.counters[4];
		return 0;
	}
	p->Common[0] = (short)rec.total;
	return 0;
}

// 0058bb00: Add, categoria 0x1e
int CItemUpdater::AddQuest983(__time64_t now, const sItemUpdate983& rec)
{
	return NewQuest983(now, rec);
}

// 0058bb20: Add, categoria 0xb
int CItemUpdater::AddItemCount983(__time64_t now, const sItemUpdate983& rec)
{
	if (((rec.typeId >> 24) & 3) == 3)
		return UpdateItemCount983(now, rec);
	return 0x1e8559;
}

// 0058bb50: count < 0, pela categoria
int CItemUpdater::Remove983(__time64_t now, const sItemUpdate983& rec)
{
	int r;
	switch (rec.typeId >> 26)
	{
	case 2:
		r = RemovePartItem983(now, rec);
		if (r)
			return r;
		break;
	case 4:
		r = RemoveClubSet983(now, rec);
		if (r)
			return r;
		break;
	case 5:
		r = UpdateBallCount983(now, rec);
		if (r)
			return r;
		break;
	case 6:
		r = UpdateMyItemCount983(now, rec);
		if (r)
			return r;
		break;
	case 7:
		r = RemoveCaddie983(now, rec);
		if (r)
			return r;
		break;
	case 0x10:
		r = RemoveMascot983(now, rec);
		if (r)
			return r;
		break;
	case 0x13:
		r = RemoveQuestItem983(now, rec);
		if (r)
			return r;
		break;
	case 0x1b:
		r = UpdateQuestCount983(now, rec);
		if (r)
			return r;
		break;
	case 0x1c:
		r = UpdateAuxPartCount983(now, rec);
		if (r)
			return r;
		break;
	case 0x1e:
		r = RemoveQuest983(now, rec);
		if (r)
			return r;
		break;
	case 0x1f:
		r = UpdateCardCount983(now, rec);
		if (r)
			return r;
		break;
	default:
		return 0x1e8501;
	}
	return 0;
}

// 0058bce0: Add, categoria 7
int CItemUpdater::AddCaddie983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sCaddieInfo>::iterator it = pDoc->m_caddieMap.find(rec.uid);
	if (it == pDoc->m_caddieMap.end())
		return NewCaddie983(now, rec);
	if (!IsValidPeriod983(rec.periodType))
		return 0x1e8516;
	it->second.Remain_Date = (unsigned short)(PeriodHours983(rec.periodType, rec.count) / 24);
	it->second.Level = (unsigned char)rec.counters[1];
	it->second.Exp = rec.counters[2];
	return 0;
}

// 0058bd90: Add, categoria 0x10
int CItemUpdater::AddMascot983(__time64_t now, const sItemUpdate983& rec)
{
	CSharedDoc* pDoc = CSharedDoc::Instance();
	std::map<unsigned int, sMascotInfo>::iterator it = pDoc->m_mascotMap.find(rec.uid);
	if (it == pDoc->m_mascotMap.end())
		return NewMascot983(now, rec);
	if (!IsValidPeriod983(rec.periodType))
		return 0x1e851e;
	MASCOT983_TIMED(it->second) = 1;
	S5::SecToSysTime983(&MASCOT983_END(it->second), rec.total);
	return 0;
}

// 0058be20: Add, categoria 0x13
int CItemUpdater::AddQuestItem983(__time64_t now, const sItemUpdate983& rec)
{
	return AddQuest983(now, rec);
}

// 0058be40: count >= 0, pela categoria
int CItemUpdater::Add983(__time64_t now, const sItemUpdate983& rec)
{
	int r;
	switch (rec.typeId >> 26)
	{
	case 1:
		r = AddCharacter983(now, rec);
		if (r)
			return r;
		break;
	case 2:
		r = AddPart983(now, rec);
		if (r)
			return r;
		break;
	case 4:
		r = AddClubSet983(now, rec);
		if (r)
			return r;
		break;
	case 5:
		r = AddBall983(now, rec);
		if (r)
			return r;
		break;
	case 6:
		r = AddMyItem983(now, rec);
		if (r)
			return r;
		break;
	case 7:
		r = AddCaddie983(now, rec);
		if (r)
			return r;
		break;
	case 8:
		r = SetCaddiePart983(now, rec);
		if (r)
			return r;
		break;
	case 0xb:
		r = AddItemCount983(now, rec);
		if (r)
			return r;
		break;
	case 0xe:
		r = AddCutin983(now, rec);
		if (r)
			return r;
		break;
	case 0xf:
		r = SetHairStyle983(now, rec);
		if (r)
			return r;
		break;
	case 0x10:
		r = AddMascot983(now, rec);
		if (r)
			return r;
		break;
	case 0x12:
		r = AddFurniture983(now, rec);
		if (r)
			return r;
		break;
	case 0x13:
		r = AddQuestItem983(now, rec);
		if (r)
			return r;
		break;
	case 0x1b:
		r = AddQuestCount983(now, rec);
		if (r)
			return r;
		break;
	case 0x1c:
		r = AddAuxPart983(now, rec);
		if (r)
			return r;
		break;
	case 0x1e:
		r = AddQuest983(now, rec);
		if (r)
			return r;
		break;
	case 0x1f:
		r = AddCard983(now, rec);
		if (r)
			return r;
		break;
	default:
		return 0x1e8502;
	}
	return 0;
}

// 0058c080: kind 2 — item comum; depois liga o aviso de itens demais (CSharedDoc+0x11030 = 2) acima de 20000
int CItemUpdater::UpdateItem983(__time64_t now, const sItemUpdate983& rec)
{
	int r;
	if (rec.count < 0)
	{
		r = Remove983(now, rec);
		if (r)
			return r;
	}
	else
	{
		r = Add983(now, rec);
		if (r)
			return r;
	}
	if ((rec.typeId & 0xfc000000) == 0x6c000000)
		return 0;
	if (rec.typeId == 0x1a0002a6)
		return 0;
	CSharedDoc* pDoc;
	if (rec.total > 20000)
	{
		if (IsValidPeriod983(rec.periodType) != true)
		{
			CSharedDoc::Instance()->m_specialShotMark = 2;
			return 0;
		}
		if (rec.count <= 20000)
			return 0;
	}
	else
	{
		if (rec.prevTotal983 <= 20000)
			return 0;
		CSharedDoc::Instance()->m_specialShotMark = 0;
		pDoc = CSharedDoc::Instance();
		std::list<sItemInfo>::iterator it = pDoc->m_myItemList.begin();
		for (;;)
		{
			if (it == pDoc->m_myItemList.end())
				return 0;
			if (it->Common[0] > 20000)
				break;
			++it;
		}
	}
	CSharedDoc::Instance()->m_specialShotMark = 2;
	return 0;
}

// 0058c160: entrada (embrulho 00633680 do CSharedDoc, pacote 0x216)
int CItemUpdater::Update983(__time64_t now, const sItemUpdate983& rec)
{
	int r;
	switch (rec.kind)
	{
	case 2:
		r = UpdateItem983(now, rec);
		if (r)
			return r;
		break;
	case 0xc9:
		r = UpdateCounters983(now, rec);
		if (r)
			return r;
		break;
	case 0xca:
		return 0x1e854d;
	case 0xcb:
		r = UpdateCard983(now, rec);
		if (r)
			return r;
		break;
	case 0xcc:
		r = UpdateWorkshop983(now, rec);
		if (r)
			return r;
		break;
	case 0xcd:
		r = UpdateMastery983(now, rec);
		if (r)
			return r;
		break;
	case 0xce:
		r = UpdateCaddieLevel983(now, rec);
		if (r)
			return r;
		break;
	}
	return 0;
}
