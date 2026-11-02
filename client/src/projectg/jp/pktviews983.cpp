// JP 983 (frente C, sessão C65): corpos das visões que o código de pacote (e as janelas da B) chamam e que ninguém
// definia (out/V6/link/falta.tsv). No 983 moram nas unidades da frente A (CSharedDoc, FrMessengerDlg); quando as classes
// tiverem os campos com nome, os corpos vão para lá e saem daqui. Deslocamentos lidos no desmonte do 983.
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "frlistbox.h"
#include "messengerdlg.h"
#include "packets/messengerpackets.h"
#include "packets/guildpackets.h"
#include "bongdarishopdlg.h"
#include "commonutil.h"
#include "cfile.h"
#include "packets/aiplayerinfocontainer.h"
#include "vc10_layout.h"

#define DOC983(T, off) (*(T*)((char*)this + (off)))
// A732: os crus 0x1xxxx do CSharedDoc viram membros (o cl novo desloca o bloco; VC10 = os do 983, C_ASSERT_VC10 abaixo)
#define SDOC983 ((CSharedDoc*)this)
C_ASSERT_VC10(offsetof(CSharedDoc, m_controlServerService) == 0x10d28 && offsetof(CSharedDoc, m_serverProperty) == 0x10d3c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomListKind983) == 0x11630 && offsetof(CSharedDoc, m_gpChannel983) == 0x1163c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_auxPartValues983) == 0x114b8 && offsetof(CSharedDoc, m_holeOrderCrypt983) == 0x114c8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_fixHole983) == 0x115f8 && sizeof(WCrypticValue<unsigned char>) == 0x10);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.realGameType) == 0x109b8 && offsetof(CSharedDoc, m_roomInfo.holeType) == 0x1094f);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594);
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel) + offsetof(sChannelInfo, Type) == 0x10a99);

// (C67: FindPlayerByUid983 0062bee0 e FindMyItemByTypeId983 0063b570 saíram — são do CSharedDoc da A; a B chama por Doc()->)

// 0062ba40: máscara de 64 bits dos serviços suspensos pelo controle do servidor (+0x10d28; o 645 tinha 32 bits)
__int64 CSharedDocJp::IsControlServerService983(__int64 service)
{
	return SDOC983->m_controlServerService & service;
}

// 0062bf30
int CSharedDocJp::GetGPChannel983()
{
	return SDOC983->m_gpChannel983;
}

// 0062bf50
int CSharedDocJp::GetRoomListKind983()
{
	return SDOC983->m_roomListKind983;
}

// 00633560: byte cifrado por buraco (registros de 16 bytes a partir de +0x114b8)
unsigned char CSharedDocJp::GetHoleValue983(int holeIndex)
{
	unsigned char* rec = (unsigned char*)&SDOC983->m_holeOrderCrypt983[-1] + holeIndex * 16; // nº do buraco 1..19
	unsigned char v = rec[0xe];
	v ^= rec[0];
	return (unsigned char)~v;
}

// 00633580: buraco fixo da sala de repetição, cifrado
unsigned char CSharedDocJp::GetFixHole983()
{
	unsigned char* p = (unsigned char*)&SDOC983->m_fixHole983;
	unsigned char v = p[0xe];
	v ^= p[0];
	return (unsigned char)~v;
}

// 00636c40: grava o buraco fixo cifrado; o segundo byte é o primeiro xor o byte baixo do endereço
void CSharedDocJp::SetFixHole983(unsigned char hole)
{
	unsigned char* p = (unsigned char*)&SDOC983->m_fixHole983;
	unsigned char v = ~hole ^ p[0xe];
	p[0] = v;
	p[1] = v ^ (unsigned char)(unsigned long)p;
}

// 0063bba0: fora dos modos 0x0b–0x0e (byte +0x109b8 do documento), liga se o item 0x1be00016 tem word +0xc == 1.
// O 983 lê o documento pelo CSharedDoc::Instance(), não pelo this.
bool CSharedDocJp::IsTrajectoryPreviewOn()
{
	CSharedDoc* doc = CSharedDoc::Instance();
	unsigned char mode = doc->m_roomInfo.realGameType;
	if ((unsigned)(mode - 0xb) > 3)
	{
		sItemInfo* item = doc->FindMyItemByTypeId983(0x1be00016);
		if (item)
			return 1 == *(unsigned short*)((char*)item + 0xc);
	}
	return false;
}

// 00900ba0 (FrMessengerDlg, visão RbMessengerDlgGuild983 de include/packets/guildpackets.h; casos 0x118/0x119 do
// OnPacketCommon): refaz a lista de guilda (+0x114) com os convites do CSharedDoc+0x10ff8 (elementos de 0x2c), com a
// linha de título (+0x190) na frente quando há convite
void RbMessengerDlgGuild983::RefreshGuildInvite983()
{
#define GUILDLIST983 (((FrMessengerDlg*)this)->m_pGuildList) // B1458: membro (cl 14.44 anda)
#define INVITES983 CSharedDoc::Instance()->m_guildInvites983
	if (!GUILDLIST983)
		return;
	GUILDLIST983->ClearItem();
	if (INVITES983.size() == 0)
		return;
	GUILDLIST983->AddItem(&((FrMessengerDlg*)this)->m_guildTitleRow983);
	int count = INVITES983.size();
	for (int i = 0; i < count; i++)
		GUILDLIST983->AddItem(&INVITES983[0] + i);
#undef INVITES983
#undef GUILDLIST983
}

// Lista dos premios da bolsa (include/packets/pouchprizelist983.h; caso 0x129 tipo 0 do my room). No 983: 005a8160 na
// unidade do my room, o resto na do FrFortuneOpenDlg.
#include "packets/pouchprizelist983.h"

// 005a8160: estatico local (00e76900, guarda 00e76910)
RbPouchPrizeList983* RbPouchPrizeList983::Instance983()
{
	static RbPouchPrizeList983 s_list;
	return &s_list;
}

// 00aa9940
void RbPouchPrizeList983::Clear983()
{
	m_list.clear();
	m_typeId = 0;
}

// 00aa8f90
void RbPouchPrizeList983::SetTypeId983(unsigned long typeId)
{
	m_typeId = typeId;
}

// 00aaa2d0
unsigned int RbPouchPrizeList983::Add983(const sPouchPrize* prize)
{
	m_list.push_back(*prize);
	return m_list.size();
}

// ---- C67 (C-PKT lote 60): visões do CSharedDoc que as janelas da B chamam e ninguém definia ----
#include "lobbyjp.h"

namespace S5 { bool ISCutinItem(unsigned long typeId); }

// Empréstimo da B (declarada em multiplayroomdlg.cpp, roomlistdlg.cpp e grandprixroomdlg.cpp; nomes da B)
class CSharedDoc983b
{
public:
	void SetGPChannel983(int channel);
	void SetRoomListKind983(int a);
	void SetRoomListRequest983(int a);
};

// 0062bf20: canal do Grand Prix (o 0062bf30 lê)
void CSharedDoc983b::SetGPChannel983(int channel)
{
	((CSharedDoc*)this)->m_gpChannel983 = channel;
}

// 0062bf40 (o 0062bf50 lê)
void CSharedDoc983b::SetRoomListKind983(int a)
{
	((CSharedDoc*)this)->m_roomListKind983 = a;
}

// 0062bf60: 1 = aba do Grand Prix, 0 = aba das salas
void CSharedDoc983b::SetRoomListRequest983(int a)
{
	((CSharedDoc*)this)->m_roomListRequest983 = a;
}

// Empréstimo da B (include/jp/subgolfrule.h; o cabeçalho não convive com o lobbyjp.h — os dois definem CSceneManagerJp)
class CSharedDocJp2
{
public:
	void AddCardPeriod983(const void* pInfo);
	void SetHoles983(unsigned char* pHoles, bool b);
};

// 00633590: o buraco fixo (GetFixHole983) está em 1..18? (no 983 é método do CSharedDoc, ecx = documento)
static bool IsFixHole983(CSharedDocJp* doc)
{
	unsigned char hole = doc->GetFixHole983();
	if (hole < 1)
		hole = 1;
	else if (hole > 18)
		hole = 18;
	return doc->GetFixHole983() == hole;
}

// 00636c60: com buraco fixo, os 19 bytes cifrados por buraco (+0x114c8, registros de 16) recebem o buraco fixo; com
// `reset` e fora da sala de repetição (+0x1094f != 4) o buraco fixo é apagado antes. O 983 não lê `pHoles`.
void CSharedDocJp2::SetHoles983(unsigned char* pHoles, bool reset)
{
	CSharedDocJp* doc = (CSharedDocJp*)this;
	if (reset && IsFixHole983(doc) && CSharedDoc::Instance()->m_roomInfo.holeType != 4)
		doc->SetFixHole983(0xff);
	if (IsFixHole983(doc))
	{
		unsigned char* rec = (unsigned char*)((CSharedDoc*)this)->m_holeOrderCrypt983;
		for (int i = 0x13; i != 0; i--)
		{
			unsigned char v = ~doc->GetFixHole983() ^ rec[0xe];
			rec[0] = v;
			rec[1] = v ^ (unsigned char)(unsigned long)rec;
			rec += 0x10;
		}
	}
}

// 00641410: registro de período de carta (0x41 bytes) na m_cardPeriodList (+0x381c). Lê o documento pelo Instance(), não pelo this.
void CSharedDocJp2::AddCardPeriod983(const void* pInfo)
{
	CSharedDoc::Instance()->m_cardPeriodList.push_back(*(const sSCardAvilityPeriodInfo*)pInfo);
}

// 00643a90 (visão de aiplayer.cpp)
class CSharedDocAIJp
{
public:
	void AddRival983(sRivalData* pRival);
};
void CSharedDocAIJp::AddRival983(sRivalData* pRival)
{
	((CSharedDoc*)this)->m_rivalList.push_back(*pRival);
}

// 00643ed0: guid do personagem com esse tid no m_charMap (-1 = não tem ou não é personagem); tid -1 = o do personagem
// em uso (m_myInfo.userEquip.guidChar, VC10 +0xa2a)
unsigned long CSharedDocLobbyJp::FindCharIdByTid983(unsigned long tid)
{
	CSharedDoc* doc = (CSharedDoc*)this;
	if (tid == 0xffffffff)
		tid = doc->m_charMap[(unsigned int&)doc->m_myInfo.userEquip.guidChar].tid;
	for (CHAR_MAP_IT it = doc->m_charMap.begin(); it != doc->m_charMap.end(); ++it)
	{
		if (tid == it->second.tid)
		{
			if ((it->second.tid & 0xfc000000) == 0x4000000)
				return it->second.guid;
			break;
		}
	}
	return 0xffffffff;
}

// 0062be00 / 0063bb30: no 983 métodos do CSharedDoc; cópias dos auxiliares estáticos do golfrule.cpp (HitShot)
static sCharacterInfo* CharOfPlayer983(CSharedDoc* doc, short player)
{
	if (player > 4)
		return NULL;
	sCharacterInfo* ch = &doc->m_userInfo[player].charInfo;
	return (ch->tid & 0xfc000000) == 0x4000000 ? ch : NULL;
}
static unsigned long CutinTidOfGuid983(CSharedDoc* doc, unsigned long guid)
{
	if (guid == 0xffffffff)
		return 0;
	std::list<sItemInfo>& list = doc->m_cutinList;
	for (std::list<sItemInfo>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (S5::ISCutinItem(it->tid) && guid == it->guid)
			return it->tid;
	}
	return 0;
}

// 0063f6c0: o personagem do jogador (0xff = o meu) tem carta de corte para esse tipo (`Condition & kind`)?
// (= HasCharCutin983 do golfrule.cpp)
bool CSharedDocJp::Is63f6c0_983(int kind, unsigned short player)
{
	CSharedDoc* doc = (CSharedDoc*)this;
	sCharacterInfo* ch = player == 0xff ? doc->FindChar983(0xffffffff) : CharOfPlayer983(doc, player);
	if (ch == NULL)
		return false;
	for (unsigned int i = 0; i < 4; i++)
	{
		IFF_STRUCT::S5::sCutinInformation* info =
			CSharedDoc::Instance()->m_itemManager.FindCutinInformation(CutinTidOfGuid983(doc, ch->cutIn[i]));
		if (info != NULL && (ch->tid & 0x3ffffff) == info->CharIndex && (info->Condition & kind) != 0)
			return true;
	}
	return false;
}

// 00b2eae0 (unidade s5/classicserver do 983; aqui ao lado do IsControlServerService983): tipo do servidor pelos 8 bits
// de serviço 0x40..0x2000. Com o bit 7 de +0x10d3c: soma de categorias (2, 4, 8, 0x10); sem ele: 1 = nenhum serviço
// suspenso, 0xe, 2, 4, 8, 0x10 conforme o grupo suspenso, 0 = nenhum.
int GetServerType983()
{
	unsigned char on[8] = { 0 };
	for (int i = 0; i < 8; i++)
	{
		if (DOCJP()->IsControlServerService983(1 << (i + 6)) != 0)
			on[i] = 1;
		else
			on[i] = 0;
	}
	int mask = (on[0] == 1 ? 0x40 : 0) | (on[1] == 1 ? 0x80 : 0) | (on[2] == 1 ? 0x100 : 0) | (on[3] == 1 ? 0x200 : 0) |
		(on[4] == 1 ? 0x400 : 0) | (on[5] == 1 ? 0x800 : 0) | (on[6] == 1 ? 0x1000 : 0) | (on[7] == 1 ? 0x2000 : 0);
	if ((CSharedDoc::Instance()->m_serverProperty >> 7) & 1)
	{
		int type = 0;
		if (on[0] == 1)
			type = 2;
		if (on[1] == 1)
			type |= 2;
		if (on[2] == 1)
			type |= 4;
		if (on[3] == 1)
			type |= 4;
		if (on[4] == 1)
			type |= 4;
		if (on[5] == 1)
			type |= 8;
		if (on[6] == 1)
			type |= 8;
		if (on[7] == 1)
			type |= 0x10;
		return type;
	}
	if (mask == 0)
		return 1;
	if (DOCJP()->IsControlServerService983(0x1fc0) == 0 && DOCJP()->IsControlServerService983(0x2000) == 0x2000)
		return 0xe;
	if (DOCJP()->IsControlServerService983(0xc0) == 0)
		return 2;
	if (DOCJP()->IsControlServerService983(0x700) == 0)
		return 4;
	if (DOCJP()->IsControlServerService983(0x1800) == 0)
		return 8;
	if (DOCJP()->IsControlServerService983(0x2000) == 0)
		return 0x10;
	return 0;
}

// 00470040 (chamada pelo CGolfTask::OnPacket do 983 em 006a0d46, caso do resultado da aproximação no Grand Prix: bola a menos
// de 2 medidas do copo): a medida na escala do campo corrente = Scale983 (0046f540, jp/aiplayer.cpp, frente B) com o mapa
// (+0x10594) e o tipo do canal (+0x10a99) do CSharedDoc. O nome é o que o golftask.cpp já cita.
float __fastcall Scale983(int map, unsigned long channelType, float value); // 0046f540
float __fastcall CourseScale983_00470040(float value)
{
	CSharedDoc* doc = CSharedDoc::Instance();
	int map = doc->m_golfGame.gameType;
	return Scale983(map, doc->m_curChannel.Type, value);
}

// 00661cb0 / 00661cf0 (unidade golfdoc do 983; visões da B em include/jp/golfdocjp.h, 17 citações): liga/desliga bits de
// evento do jogador `index` (CGolfDoc+0xc8 = m_pPlayer, registros de 0xdb0, evento em +0x4e4; +0xd5 = número de jogadores).
// Fora da faixa não faz nada (o 983 devolve lixo em eax nesse caso).
#include "golfdocjp.h"
// B1458: membros (cl 14.44 m_pPlayer/m_playerNum andam -0x2c; sPlayerData igual nos dois: 0xdb0, event +0x4e4)
C_ASSERT_VC10(offsetof(CGolfDoc, m_pPlayer) == 0xc8 && offsetof(CGolfDoc, m_playerNum) == 0xd5);
C_ASSERT(sizeof(sPlayerData) == 0xdb0 && offsetof(sPlayerData, event) == 0x4e4 && sizeof(WFlags) == 4);
unsigned long* CGolfDocJp::EnablePlayerEvent(unsigned long flag, unsigned char index)
{
	CGolfDoc* doc = (CGolfDoc*)this;
	if (index < doc->m_playerNum)
	{
		unsigned long* pEvent = (unsigned long*)&doc->m_pPlayer[index].event;
		*pEvent |= flag;
		return pEvent;
	}
	// sem return fora da faixa (aviso C4715): o 983 devolve o que estiver em eax
}

void CGolfDocJp::DisablePlayerEvent(unsigned long flag, unsigned char index)
{
	CGolfDoc* doc = (CGolfDoc*)this;
	if (index < doc->m_playerNum)
		*(unsigned long*)&doc->m_pPlayer[index].event &= ~flag;
}

// 00b30740 (sharedutilities do 983; sem corpo no nosso fonte): a data em horas (FILETIME / 36.000.000.000, unidades de 100 ns)
__int64 __fastcall SystemTimeToHours983(const SYSTEMTIME* st)
{
	FILETIME ft;
	SystemTimeToFileTime(st, &ft);
	return (((__int64)ft.dwHighDateTime << 32) + ft.dwLowDateTime) / 36000000000i64;
}

// 006333c0 (visão da B em include/jp/shareddocjp.h; o RFLobbyDesktop mostra as horas do premium): horas que faltam até `end`
// pela hora do servidor; 0 se já passou
__int64 CSharedDocJp::GetRemainHours983(SYSTEMTIME* end)
{
	__int64 now = SystemTimeToHours983(&((CSharedDoc*)this)->GetServerTime());
	__int64 remain = SystemTimeToHours983(end) - now;
	if (remain > 0)
		return remain;
	return 0;
}

// 0063f830 (visão da B em jp/normallobbyroom.cpp: evento de pang do mapa): o valor guardado para o curso no mapa de +0x1146c
// do CSharedDoc (m_mapEvent983; o valor é o ponteiro do registro), NULL se não há
struct sMapEvent983;
class CSharedDoc983c
{
public:
	sMapEvent983* Fn63f830_983(unsigned long course); // 0063f830
};

sMapEvent983* CSharedDoc983c::Fn63f830_983(unsigned long course)
{
	// no 983 a chave é unsigned long e o valor é o ponteiro (A declarou std::map<int, int>; mesmo layout)
	typedef std::map<unsigned long, sMapEvent983*> MapEvents983;
	sMapEvent983* pEvent = NULL;
	MapEvents983::iterator it = ((MapEvents983&)((CSharedDoc*)this)->m_mapEvent983).find(course);
	if (it != ((MapEvents983&)((CSharedDoc*)this)->m_mapEvent983).end())
		return it->second;
	return pEvent;
}

// 00641350 / 0063db90 (visão da B em jp/itemupdater.cpp, pacote 0x216): lista de habilidades de carta do documento
// (m_cardAbilityList[0], +0x37ec). Os dois leem o documento pelo Instance(), não pelo this.
#include "../../shared/globalgamedefine.h"
class CSharedDocItemJp983 : public CSharedDoc
{
public:
	void RemoveCardAbilityByPart983(unsigned long partUid); // 0063db90
	void AddCardAbility983(const sSCardAvilityPeriodInfo& info); // 00641350
};

void CSharedDocItemJp983::AddCardAbility983(const sSCardAvilityPeriodInfo& info)
{
	CSharedDoc::Instance()->m_cardAbilityList[0].push_back(info);
}

// apaga as habilidades presas à peça `partUid`
void CSharedDocItemJp983::RemoveCardAbilityByPart983(unsigned long partUid)
{
	CARDABILITY_LIST_IT it = CSharedDoc::Instance()->m_cardAbilityList[0].begin();
	while (it != CSharedDoc::Instance()->m_cardAbilityList[0].end())
	{
		if (it->partsUid == partUid)
			it = CSharedDoc::Instance()->m_cardAbilityList[0].erase(it);
		else
			++it;
	}
}

// 00661fb0 / 006620b0 (golfdoc do 983; visões da B em jp/dummyplayeractor.cpp): monta a lista de peças de um boneco
// como o CGolfDoc::SetPartTidList do 645, mas numa lista de fora (o this não é lido). O 006620b0 antes tira as peças
// que se sobrepõem (008b3330).
#include "parttidlist.h"
#include "../../shared/itemmanager.h"

// 008b3330 (parttidlist do 983; sem corpo no nosso fonte): SetTids que antes apaga, de cada par de peças com a mesma
// posição (PosMask), a que é a peça básica (TypeId & 0x1ff == 0)
class CPartTidListJp983 : public CPartTidList
{
public:
	bool SetTidsNoOverlap983(sCharacterInfo* pInfo, unsigned char level);
};

bool CPartTidListJp983::SetTidsNoOverlap983(sCharacterInfo* pInfo, unsigned char level)
{
	if (!pInfo)
		return false;
	for (int i = 0; i < 24; i++)
	{
		IFF_STRUCT::sPart* pPart = m_pItemManager->FindPart(pInfo->tidParts[i]);
		if (pPart == NULL)
			continue;
		for (int j = i + 1; j < 24; j++)
		{
			IFF_STRUCT::sPart* pOther = m_pItemManager->FindPart(pInfo->tidParts[j]);
			if (pOther == NULL || (pPart->PosMask & pOther->PosMask) == 0)
				continue;
			if ((pPart->c.TypeId & 0x1ff) == 0)
				pInfo->tidParts[i] = 0;
			else if ((pOther->c.TypeId & 0x1ff) == 0)
				pInfo->tidParts[j] = 0;
		}
	}
	bool ret = SetTids(pInfo->tid, pInfo->hairClr, pInfo->shirtsClr, pInfo->tidParts, level, pInfo->ItemIdList);
	if (!ret)
		m_charTid = pInfo->tid;
	return ret;
}

class CGolfDocPlayerJp
{
public:
	void MakePartsFull983(CPartTidList* out, unsigned long charTid, int a, int b, unsigned long* tids, unsigned long* uids); // 00661fb0
	void MakeParts983(CPartTidList* out, unsigned long charTid, int a, int b, unsigned long* tids, unsigned long* uids); // 006620b0
};

void CGolfDocPlayerJp::MakePartsFull983(CPartTidList* out, unsigned long charTid, int hairColor, int shirtColor,
	unsigned long* tids, unsigned long* uids)
{
	sCharacterInfo info;
	info.tid = charTid;
	info.hairClr = (unsigned char)hairColor;
	info.shirtsClr = (unsigned char)shirtColor;
	memcpy(info.tidParts, tids, sizeof(info.tidParts));
	if (uids != NULL)
		memcpy(info.ItemIdList, uids, sizeof(info.ItemIdList));
	if (!out->SetTids(&info, 0xff))
		out->SetDefaultTids();
	if (!out->IsComboValid())
		out->SetDefaultTids();
}

void CGolfDocPlayerJp::MakeParts983(CPartTidList* out, unsigned long charTid, int hairColor, int shirtColor,
	unsigned long* tids, unsigned long* uids)
{
	sCharacterInfo info;
	info.tid = charTid;
	info.hairClr = (unsigned char)hairColor;
	info.shirtsClr = (unsigned char)shirtColor;
	memcpy(info.tidParts, tids, sizeof(info.tidParts));
	if (uids != NULL)
		memcpy(info.ItemIdList, uids, sizeof(info.ItemIdList));
	if (!((CPartTidListJp983*)out)->SetTidsNoOverlap983(&info, 0xff))
		out->SetDefaultTids();
	if (!out->IsComboValid())
		out->SetDefaultTids();
}

// ---- C70: CGaugeBar983 (include/jp/gaugebar983.h da B; unidade 00923290–00923b06, sem par no 645, sem dono) ----
// Nomes do cabeçalho da B: m_cur (+0) = andamento 0..1 da animação, m_target (+4) = valor MOSTRADO, m_value (+8) = valor-alvo
// (o IsDone compara os dois). +0x1c é a figura dos tracinhos (m_pRes) e +0x34 o número de divisões (m_34).
// Os 6 métodos *983 entraram no cabeçalho no C70.1 (B136); a visão derivada da C saiu (C72).
#include "woverlay.h"
#include "wresrcmng.h"
#include "gaugebar983.h"

extern WView* g_view; // 00f02458 (o g_pMainView983 da B)
extern const WRect g_rectUnitW983; // 00e4296c
const WRect g_rectUnitW983(0.0f, 0.0f, 1.0f, 1.0f);

template <class T> inline T GaugeClamp983(T v, T lo, T hi)
{
	if (v < lo)
		return lo;
	if (v > hi)
		return hi;
	return v;
}

#define GAUGE_RELEASE983(p) if (g_resrcmng && (p)) { g_resrcmng->Release(p); (p) = NULL; }

CGaugeBar983::CGaugeBar983()
	: m_cur(0.0f), m_target(0.0f), m_min(0.0f), m_max(0.0f), m_pRes(NULL), m_b20(false), m_f24(1.0f), m_x(0.0f), m_y(0.0f),
	  m_margin(0), m_34(0), m_b49(true), m_b4a(true)
{
	for (int i = 0; i < 2; i++)
		m_pOverlay[i] = NULL;
}

CGaugeBar983::~CGaugeBar983()
{
	for (int i = 0; i < 2; i++)
		GAUGE_RELEASE983(m_pOverlay[i]);
	GAUGE_RELEASE983(m_pRes);
}

void CGaugeBar983::Reset()
{
	m_cur = m_target = m_value = 0.0f;
}

void CGaugeBar983::SetImages(const char* bg, const char* bar)
{
	if (bg)
	{
		GAUGE_RELEASE983(m_pOverlay[0]);
		m_pOverlay[0] = g_resrcmng->GetOverlay(bg, 0);
	}
	if (bar)
	{
		GAUGE_RELEASE983(m_pOverlay[1]);
		m_pOverlay[1] = g_resrcmng->GetOverlay(bar, 0);
	}
}

void CGaugeBar983::SetOverlays983(WOverlay* bg, WOverlay* bar)
{
	if (bg && m_pOverlay[0] != bg)
	{
		GAUGE_RELEASE983(m_pOverlay[0]);
		m_pOverlay[0] = bg;
	}
	if (bar && m_pOverlay[1] != bar)
	{
		GAUGE_RELEASE983(m_pOverlay[1]);
		m_pOverlay[1] = bar;
	}
}

void CGaugeBar983::SetTicks983(const char* name, int count)
{
	m_pRes = g_resrcmng->GetOverlay(name, 0);
	m_34 = count;
}

bool CGaugeBar983::Init(const char* bg, const char* bar, WRect rc, int margin, bool b20, float f24, bool b48, bool b4a)
{
	m_pOverlay[0] = g_resrcmng->GetOverlay(bg, 0);
	m_pOverlay[1] = g_resrcmng->GetOverlay(bar, 0);
	for (int i = 0; i < 2; i++)
	{
		if (!m_pOverlay[i])
			return false;
	}
	m_max = rc.h; // rc.w (o "mínimo") não é lido
	m_x = rc.x;
	m_y = rc.y;
	m_b20 = b20;
	m_f24 = f24;
	m_b48 = b48;
	(WPoint&)m_f40 = WPoint(0.0f, 0.0f);
	m_margin = margin;
	int w = m_pOverlay[1]->GetWidth();
	if (m_pOverlay[0]->GetWidth() > w)
		w = m_pOverlay[0]->GetWidth();
	m_w = w + margin * 2;
	int h = m_pOverlay[1]->GetHeight();
	if (m_pOverlay[0]->GetHeight() > h)
		h = m_pOverlay[0]->GetHeight();
	m_h = h + margin * 2;
	m_b4a = b4a;
	return true;
}

void CGaugeBar983::SetValue(float value)
{
	m_value = value;
	m_value = GaugeClamp983(m_value, m_min, m_max);
}

void CGaugeBar983::SetNow983(float value)
{
	m_value = value;
	m_target = m_value = GaugeClamp983(m_value, m_min, m_max);
}

void CGaugeBar983::SetTarget(float value)
{
	if (m_target != 0.0f)
		m_cur = m_target / value;
	m_value = value;
	m_value = GaugeClamp983(m_value, m_min, m_max);
}

void CGaugeBar983::Process(const float& deltaTime)
{
	if (!m_b20)
	{
		m_target = m_value;
		return;
	}
	m_cur += deltaTime * 0.5 * m_f24;
	if (m_cur > 1.0f)
		m_cur = 1.0f;
	m_target = (m_b49 ? m_cur : 1.0f - m_cur) * m_value;
	m_target = GaugeClamp983(m_target, m_min, m_value);
}

void CGaugeBar983::DrawHorz983()
{
	WOverlay* bg = m_pOverlay[0];
	WRect uv(0.0f, 0.0f, 1.0f, 1.0f);
	WRect rc(m_x, m_y, (float)bg->GetWidth(), (float)bg->GetHeight());
	if (m_b4a)
		bg->Render(g_view, uv, rc, 0, 0xffffffff, 0.0f, 0);
	float off = 0.0f;
	int margin = m_margin;
	if (margin > 0)
		off = (float)margin / m_pOverlay[1]->GetWidth();
	int barW = m_pOverlay[1]->GetWidth();
	float cur = m_target - m_min;
	float range = m_max - m_min;
	float px = cur / range * (barW - margin * 2);
	float u = px / barW;
	uv.x = 0.0f;
	uv.y = 0.0f;
	uv.w = m_target == 0.0f ? 0.0f : u + off;
	uv.h = 1.0f;
	rc.x = m_x;
	rc.y = m_y;
	rc.w = m_target == 0.0f ? 0.0f : u * m_pOverlay[1]->GetWidth() + margin;
	rc.h = (float)m_pOverlay[1]->GetHeight();
	m_f40 = rc.x;
	m_f44 = rc.y;
	m_pOverlay[1]->Render(g_view, uv, rc, 0, 0xffffffff, 0.0f, 0);
}

void CGaugeBar983::DrawVert983()
{
	WOverlay* bg = m_pOverlay[0];
	WRect uv(0.0f, 0.0f, 1.0f, 1.0f);
	WRect rc(m_x, m_y, (float)bg->GetWidth(), (float)bg->GetHeight());
	int barH = m_pOverlay[1]->GetHeight();
	bg->Render(g_view, uv, rc, 0, 0xffffffff, 0.0f, 0);
	float off = 0.0f;
	int margin = m_margin;
	if (margin > 0)
		off = (float)margin / barH;
	float cur = m_target - m_min;
	float range = m_max - m_min;
	float px = cur / range * (barH - margin * 2);
	float u = px / barH;
	uv.x = 0.0f;
	uv.y = m_target == 0.0f ? 0.0f : 1.0f - (u + off);
	uv.w = 1.0f;
	uv.h = m_target == 0.0f ? 0.0f : u + off;
	rc.x = m_x;
	rc.y = m_y;
	float fm = (float)margin;
	rc.y = m_pOverlay[0]->GetHeight() + m_y - (m_pOverlay[1]->GetHeight() * u + fm);
	rc.w = (float)m_pOverlay[1]->GetWidth();
	rc.h = m_target == 0.0f ? 0.0f : u * m_pOverlay[1]->GetHeight() + fm;
	m_f40 = rc.x;
	m_f44 = rc.y;
	m_pOverlay[1]->Render(g_view, uv, rc, 0, 0xffffffff, 0.0f, 0);
}

WPoint CGaugeBar983::GetTickPos983(int index)
{
	int x = (int)((float)(m_pOverlay[1]->GetWidth() - m_margin * 2) / m_34 * index + m_x + m_margin);
	return WPoint((float)x, m_y);
}

void CGaugeBar983::Draw()
{
	if (m_b48)
		DrawHorz983();
	else
		DrawVert983();
	if (!m_pRes)
		return;
	for (int i = 0; i < m_34 - 1; i++)
	{
		int x = (int)((float)(m_pOverlay[1]->GetWidth() - m_margin * 2) / m_34 * (i + 1) + m_x + m_margin);
		WOverlay* tick = (WOverlay*)m_pRes;
		tick->Render(g_view, g_rectUnitW983, WRect((float)x, m_y, (float)tick->GetWidth(), (float)tick->GetHeight()), 0,
			0xffffffff, 0.0f, 0);
	}
}

// ---- C-PKT lote 64 (sessão C71): visões sem dono com VA citadas por A e B (out/V6/link/falta.tsv) ----
#include "../../shared/classdefine.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "gamedatadb.h"
#include "gdskinitem.h"
#include "gdskininstance983.h"
#include "gatewayactor.h"

// mesma visão que console.cpp (A) e alchemympbreakdlg.cpp (B) declaram localmente
class CItemManagerJp983 : public CItemManager
{
public:
	bool IsMPBreakItem983(unsigned long typeId);
};

// 00a19530: item que entra na quebra de MP. Grupos (typeId >> 26) 1, 3, 7–14, 16–19, 27, 29, 30 nunca; o resto pelo
// Mileage (+0x8c) do registro comum do .iff.
bool CItemManagerJp983::IsMPBreakItem983(unsigned long typeId)
{
	switch (typeId >> 26)
	{
	case 1: case 3: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14:
	case 16: case 17: case 18: case 19: case 27: case 29: case 30:
		return false;
	}
	IFF_ITEM_COMMON* pIff = FindCommonItem(typeId);
	if (!pIff)
		return false;
	return 0 != pIff->Mileage;
}

// 00b304b0: grava o índice do jogador (`SetIndex`) e manda ao "Gateway" 0x753b/1 com o índice e a ficha do personagem
// (chamado pelo lobby, pela sala e pelo LobbyAvatarActor antes de montar o boneco).
void __fastcall Fn0b304b0_983(unsigned long uid, sCharacterInfo* pInfo)
{
	CSharedDoc::Instance()->SetIndex(uid);
	unsigned char index = CSharedDoc::Instance()->GetIndex(uid);
	IActor* pGateway = AfxGetTask()->GetActor(s_gatewayActor[1]);
	pGateway << MsgObject(NULL, 0x753b, 1, index, (int)pInfo, 0, 0);
}

// visão do lua983.h (B): no 983 o gerenciador de scripts tem um tratador de erro em +0x14 (fastcall, recebe o texto)
namespace lua_system { class lua_script_manger; }
class LuaScriptManagerJp983
{
public:
	void Error983(const char* text);
};

typedef void (__fastcall* LuaErrorFn983)(const char* text);

// 00c1f480
void LuaScriptManagerJp983::Error983(const char* text)
{
	C_ASSERT_VC10(offsetof(lua_system::lua_script_manger, errorFn) == 0x14);
	LuaErrorFn983 fn = ((lua_system::lua_script_manger*)this)->errorFn; // B1458: membro (VC10 +0x14; cl 14.44 std::list menor)
	if (fn)
		fn(text);
}

// o luautilitylib (lua_system::) liga o simbolo ?Error983@LuaScriptManagerJp983@lua_system@@QAEXPBD@Z: mesma funcao 00c1f480 no namespace certo
namespace lua_system
{
	class LuaScriptManagerJp983
	{
	public:
		void Error983(const char* text);
	};
	void LuaScriptManagerJp983::Error983(const char* text)
	{
		LuaErrorFn983 fn = ((lua_system::lua_script_manger*)this)->errorFn; // B1458: membro (VC10 +0x14; cl 14.44 std::list menor)
		if (fn)
			fn(text);
	}
}

// 00542f00: na 1ª chamada pega "TanLines" (tipo 4) do CGameDataDB
CGDSkinItem* CGDSkinItemPetJp::Instance983()
{
	if (!CGDSkinItem::s_pInstance)
		CGDSkinItem::s_pInstance = (CGDSkinItem*)CGameDataDB::Instance()->GetGameData((eGameDataDB)4, "TanLines");
	return CGDSkinItem::s_pInstance;
}

// ---- IPetScriptObject (unidade 005ac510–005ad82b; src/projectg/petscriptobject.h da A, sem corpo; citada por golfplayer e dummypet) ----
#include "petscriptobject.h"
#include "partsinfo.h"
#include "wpuppet.h"
#include "wbone.h"

const char* GetStrConst(const char* str, char* out); // 0098c310 (npc.cpp)

// 00e11820: palavras na ordem do switch do ParseScript ("*fxstop(" cai no mesmo caso do "*fx(")
static const char* s_petScriptCmds983[16] = {
	"*snd(", "*shot", "*tail", "*showclub", "*hideclub", "*swing", "*lockclub", "*fx(",
	"*ptex(", "*showbone(", "*hidebone(", "*playif(", "*opa(", "*showvis(", "*hidevis(", "*fxstop(" };
static char s_petScriptWord983[0x400];      // 00e76cb0
static char s_petScriptMPets983[0x400];     // 00e770b0 (lista "a;b;c" do *playif)
static char s_petScriptMotion983[0x400];    // 00e774b0 (movimento do *playif)

// 005ac510: o analisador do CGolfPlayer::ParsePuppetScript do 645, agora com um virtual por palavra. O 1º "*fx(" com
// "TF_Feather.seq" ganha três "*fx(plume_twinkle.seq …)" na frente (cabeça de madeira ou de ferro).
void IPetScriptObject::ParseScript(char* script, bool bSkip)
{
	if (!script)
		return;
	const char* cur = script;
	char buf[0x400];
	bool bFeather = false;
	sprintf(buf, "%s", cur);
	cur = buf;
	while (cur)
	{
		cur = strchr(cur, '*');
		if (!cur)
			break;
		int len = 0;
		while (cur[len] != '\0')
		{
			len++;
			if (cur[len] == ' ')
				break;
			if (cur[len] == '(')
			{
				len++;
				break;
			}
		}
		if (len <= 1)
			break;
		strncpy(s_petScriptWord983, cur, len);
		cur += len;
		s_petScriptWord983[len] = '\0';
		for (int i = 0; i < 16; i++)
		{
			if (_stricmp(s_petScriptWord983, s_petScriptCmds983[i]) != 0)
				continue;
			switch (i)
			{
			case 0: OnScriptSnd(cur); break;
			case 1: OnScriptShot(cur); break;
			case 2: OnScriptTail(cur); break;
			case 3: OnScriptShowClub(cur); break;
			case 4: OnScriptHideClub(cur); break;
			case 5: OnScriptSwing(cur); break;
			case 6: OnScriptLockClub(cur); break;
			case 7:
			case 15:
				if (i == 7 && !bFeather)
				{
					bFeather = true;
					if (strstr(cur, "TF_Feather.seq"))
					{
						if (strstr(cur, "head_wood"))
						{
							sprintf(buf, "%s *fx(\"plume_twinkle.seq\" \"&268435587\" \"&201327102\" \"&201327103\" \"&201327104\" \"&201327105\" \"head_wood\") ", cur);
							sprintf(buf, "%s *fx(\"plume_twinkle.seq\" \"&268435588\" \"&201327106\" \"&201327107\" \"&201327108\" \"&201327109\" \"head_wood\") ", cur);
							sprintf(buf, "%s *fx(\"plume_twinkle.seq\" \"&268435589\" \"&201327110\" \"&201327111\" \"&201327112\" \"&201327113\" \"head_wood\") ", cur);
						}
						else
						{
							sprintf(buf, "%s *fx(\"plume_twinkle.seq\" \"&268435587\" \"&201327102\" \"&201327103\" \"&201327104\" \"&201327105\" \"head_iron\") ", cur);
							sprintf(buf, "%s *fx(\"plume_twinkle.seq\" \"&268435588\" \"&201327106\" \"&201327107\" \"&201327108\" \"&201327109\" \"head_iron\") ", cur);
							sprintf(buf, "%s *fx(\"plume_twinkle.seq\" \"&268435589\" \"&201327110\" \"&201327111\" \"&201327112\" \"&201327113\" \"head_iron\") ", cur);
						}
						cur = buf;
					}
				}
				OnScriptFx(cur, bSkip, i);
				break;
			case 8: OnScriptPTex(cur); break;
			case 9:
			case 10: OnScriptShowBone(cur, i); break;
			case 11: OnScriptPlayIf(cur); break;
			case 12: OnScriptOpa(cur); break;
			case 13:
			case 14: OnScriptShowVis(cur, i); break;
			}
		}
	}
}

// 005ac900: "*showbone(" (cmd 9) / "*hidebone(" — até 64 nomes de osso, até o ')'
void IPetScriptObject::ScriptShowBone(CPetFrame* pFrame, int cmd, const char*& p)
{
	char name[0x40] = { 0 };
	for (int i = 0; i < 0x40; i++)
	{
		p = GetStrConst(p, name);
		size_t len = strlen(name);
		if (len == 0 || len >= 0x40)
			return;
		WBone* pBone = pFrame->GetBonePet()->FindBone(name);
		if (pBone)
			pBone->SetInvisible(false, cmd != 9);
		if (!p || *p == ')')
			return;
	}
}

// 005aca50: "*playif(" "a;b;c" "movimento": devolve o movimento se o boneco tem algum dos .mpet da lista
const char* IPetScriptObject::ScriptPlayIf(CPetFrame* pFrame, const char*& p)
{
	s_petScriptMotion983[0] = '\0';
	s_petScriptMPets983[0] = '\0';
	p = GetStrConst(p, s_petScriptMPets983);
	p = GetStrConst(p, s_petScriptMotion983);
	if (!s_petScriptMPets983[0] || !s_petScriptMotion983[0])
		return NULL;
	int pos = 0;
	for (;;)
	{
		const char* item = s_petScriptMPets983 + pos;
		const char* semi = strchr(item, ';');
		int len;
		if (semi)
			len = semi - pos - s_petScriptMPets983;
		else
		{
			len = strlen(item);
			if (len == 0)
				return NULL;
		}
		char name[0x28];
		strncpy(name, item, len);
		name[len] = '\0';
		strcat(name, ".mpet");
		if (pFrame->HasMPet(name))
			return s_petScriptMotion983;
		if (!semi)
			return NULL;
		pos += len + 1;
	}
}

// 005acbf0: "*showvis(" / "*hidevis(": cada grupo de visibilidade do CSharedDoc mostra/esconde os seus ossos
void IPetScriptObject::ScriptShowVis(CPetFrame* pFrame, bool bShow, const char*& p)
{
	if (!pFrame)
		return;
	char name[0x100] = { 0 };
	for (int i = 0; i < 0x40; i++)
	{
		p = GetStrConst(p, name);
		size_t len = strlen(name);
		if (len == 0 || len >= 0x100)
			return;
		std::list<std::string>* pGroup = CSharedDoc::Instance()->GetVisGroup(name);
		if (pGroup)
		{
			for (std::list<std::string>::iterator it = pGroup->begin(); it != pGroup->end(); ++it)
			{
				WBone* pBone = pFrame->GetBonePet()->FindBone(it->c_str());
				if (pBone)
					pBone->SetInvisible(false, !bShow);
			}
		}
		if (!p || *p == ')')
			return;
	}
}

// ---- C-PKT lote 65 (sessão C72): as duas variantes com o WPuppet direto (declaradas pela B no include/jp/dummypet.h) ----
#include "dummypet.h"

// 005ac9b0: igual ao 005ac900, com o boneco no lugar do CPetFrame (IDummyPet::OnScriptShowBone). Declarado pela A no
// petscriptobject.h (C71.1, sessão 137); o nome IDummyPet::ScriptShowBonePet983 ficou só como repasse até a B trocar a chamada (C73.1).
void IPetScriptObject::ScriptShowBone(WPuppet* pPet, int cmd, const char*& p)
{
	char name[0x40] = { 0 };
	for (int i = 0; i < 0x40; i++)
	{
		p = GetStrConst(p, name);
		size_t len = strlen(name);
		if (len == 0 || len >= 0x40)
			return;
		WBone* pBone = pPet->FindBone(name);
		if (pBone)
			pBone->SetInvisible(false, cmd != 9);
		if (!p || *p == ')')
			return;
	}
}

// C73.1 (B): repasse provisório; sai quando o jp/dummypet.cpp chamar ScriptShowBone(m_pPuppet, cmd, p) e o dummypet.h perder a declaração.
void IDummyPet::ScriptShowBonePet983(WPuppet* pPet, int cmd, const char*& p)
{
	ScriptShowBone(pPet, cmd, p);
}

// 005acd10: igual ao 005acbf0, com o boneco no lugar do CPetFrame (IDummyPet::OnScriptShowVis)
void IDummyPet::ScriptShowVisPet983(WPuppet* pPet, bool bShow, const char*& p)
{
	if (!pPet)
		return;
	char name[0x100] = { 0 };
	for (int i = 0; i < 0x40; i++)
	{
		p = GetStrConst(p, name);
		size_t len = strlen(name);
		if (len == 0 || len >= 0x100)
			return;
		std::list<std::string>* pGroup = CSharedDoc::Instance()->GetVisGroup(name);
		if (pGroup)
		{
			for (std::list<std::string>::iterator it = pGroup->begin(); it != pGroup->end(); ++it)
			{
				WBone* pBone = pPet->FindBone(it->c_str());
				if (pBone)
					pBone->SetInvisible(false, !bShow);
			}
		}
		if (!p || *p == ')')
			return;
	}
}

// 005ace20: leitor do "*ptex(" do jogador (IDummyPlayer::OnScriptPTex 004d8f7a; o CGolfPlayer::OnScriptPTex 007423bc). Visão da B
// no jp/dummyplayeractor.cpp (só o método, mesmo símbolo). kind 1: textura direta; 2: troca pela tabela se o CGDSkinItem tem a linha
// das peças; 0: troca só com bFlag; outro: troca pelas 24 peças do quadro do roteiro (+4). Como no 983, sem '@' a 1ª leitura é
// descartada (strcpy do resto do roteiro, sobrescrito pela leitura seguinte) e a troca recebe a palavra lida (cond), não o nome.
#include "gdskinitem.h"
class IPetScriptObjectJp : public IPetScriptObject
{
public:
	bool ParsePTex983(w_motion_data* motion, std::string* pTexture, const char*& p, int kind, bool bFlag); // 005ace20
};

bool IPetScriptObjectJp::ParsePTex983(w_motion_data* motion, std::string* pTexture, const char*& p, int kind, bool bFlag)
{
	char cond[64];
	char texName[0x400];
	cond[0] = 0;
	texName[0] = 0;
	p = GetStrConst(p, cond);
	if (strlen(cond) == 0 || strlen(cond) >= 64)
		return false;
	if (cond[0] == '@')
	{
		if (!motion || _stricmp(cond + 1, motion->name) != 0)
			return false;
		p = GetStrConst(p, texName);
		if (strlen(texName) == 0 || strlen(texName) >= 0x400)
			return false;
	}
	else
	{
		strcpy(texName, p);
	}
	p = GetStrConst(p, texName);
	if (strlen(texName) == 0 || strlen(texName) >= 0x400)
		return false;
	if (kind != 1)
	{
		CGDSkinItem* pSkin = CGDSkinItemPetJp::Instance983();
		std::string tmp;
		if (kind == 2)
		{
			if (pSkin->HasPartRow())
			{
				pSkin->Replace(cond, texName, &tmp, (const sSkinParts983*)NULL);
				*pTexture = tmp;
			}
			else
				*pTexture = texName;
			return true;
		}
		if (kind == 0)
		{
			if (bFlag == true)
			{
				pSkin->Replace(cond, texName, &tmp, (const sSkinParts983*)NULL);
				*pTexture = tmp;
				return true;
			}
		}
		else
		{
			unsigned long tids[24];
			CPetFrame* pFrame = m_pScriptFrame;
			for (int i = 0; i < 24; i++)
			{
				if (i < pFrame->GetPartList().GetNumTotalParts())
					tids[i] = pFrame->GetPartList().GetPart((unsigned char)i)->tid;
				else
					tids[i] = 0;
			}
			pSkin->SetParts(tids);
			pSkin->Replace(cond, texName, &tmp, (const sSkinParts983*)NULL);
			*pTexture = tmp;
			return true;
		}
	}
	*pTexture = texName;
	return true;
}

// ---- C74 (lote 67): FrBongdariShopDlg (frente A) — métodos só do 983 chamados pelos casos 0x21b/0x26c do CTask::OnPacketCommon.
// A janela ainda está no layout do 645 (0x3f0; o 983 tem 0xb30): o que depende de campo que o 645 não tem fica de fora
// (pedido C74.1): botão "back_select_gacha_stage" (+0x130), auto-jogo (botão +0x424, flag +0x428, 008b4670) e a lista +0x3e4.
class FrBongdariShopDlgPkt983 : public FrBongdariShopDlg
{
public:
	void StopAutoPlay983();										 // 008b4740
	void EnableAfterResult983();								 // 008b4830
	void SetResultList983(int count, std::vector<sBonusBall>& balls); // 008bee10
	void SetStateReady983();									 // 008b35e0
};

// 008b4740: erro na compra do lobby (0x21b). No 983: flag de auto-jogo +0x428 = 0 e botão de auto-jogo no estado 1 (008b4670)
// antes do mesmo corpo do 008b4830.
void FrBongdariShopDlgPkt983::StopAutoPlay983()
{
	// falta (C74.1): m_bAutoPlay983 (+0x428) = false; SetAutoPlayButton983(1) (008b4670)
	EnableExitButton(true);
	// falta (C74.1): m_pBackSelectGachaStageBn983 (+0x130)->Enable(true)
	m_bUsingBonusBall = false; // 983 +0x2aa
}

// 008b4830: erro na compra do my room (0x26c): botão de sair e "back_select_gacha_stage" ligados, bola em uso desligada
void FrBongdariShopDlgPkt983::EnableAfterResult983()
{
	EnableExitButton(true);
	// falta (C74.1): m_pBackSelectGachaStageBn983 (+0x130)->Enable(true)
	m_bUsingBonusBall = false; // 983 +0x2aa
}

// 008bee10: resultado do my room (0x26c). No 983 o 008bea70 antes copia o vetor inteiro para a lista +0x3e4 (falta, C74.1);
// depois soma as `count` bolas na lista total (AddListTotalItem 008beb40, +0x3d8 no 983).
void FrBongdariShopDlgPkt983::SetResultList983(int count, std::vector<sBonusBall>& balls)
{
	for (int i = 0; i < count; i++)
		AddListTotalItem(&balls[0] + i);
}

// 008b35e0
void FrBongdariShopDlgPkt983::SetStateReady983()
{
	m_state = 1; // 983 +0x134
}

// ---- C-PKT lote 68 (sessão C75): documento 0xa3 dos jogadores de IA (CAIPlayerInfoContainer do 983) ----
// Classe em include/packets/aiplayerinfocontainer.h; os métodos da visão CAIDocJp da B (include/jp/aiplayer.h) leem por ela
// até a visão ser trocada pelo nome real (pedido C75.1).

// 00915b90
CAIPlayerInfoContainer983::CAIPlayerInfoContainer983()
{
}

// 00910aa0
bool CAIPlayerInfoContainer983::Initialize()
{
	m_infoMap.clear();
	m_npcIds.clear();
	m_lineMap.clear();
	return true;
}

// 00907960
bool CAIPlayerInfoContainer983::Release()
{
	return Initialize();
}

// 00910b80: pacote 0x256 (CTask::OnPacketCommon 009d66f8). Um registro por IA: número do adversário e o resultado de cada
// buraco (0x28 cada; o vetor é reservado com o número de buracos da partida).
void CAIPlayerInfoContainer983::ReadPacket983(WReceivedPacket& packet)
{
	std::vector<sAIDocInfo983> infos;
	int count = packet.Decode2();
	sAIDocInfo983 info;
	for (int i = 0; i < count; i++)
	{
		infos.clear();
		infos.reserve(Doc()->m_golfGame.holes);
		unsigned long npcId = packet.Decode4();
		m_npcIds.push_back(npcId);
		int holes = packet.Decode1();
		for (int j = 0; j < holes; j++)
		{
			packet.DecodeBuffer(&info, 0x28);
			infos.push_back(info);
		}
		m_infoMap.insert(std::pair<int, std::vector<sAIDocInfo983> >(i, infos));
	}
}

// 0090f720: linhas de um .aibin; a linha com "ai" no nome é do tipo 0xf
bool CAIPlayerInfoContainer983::ReadAiBin983(cFile* pFile, std::vector<CNodeContainer>* pOut)
{
	sAiBinHeader983 header;
	pFile->Read(&header, sizeof(header));
	if (header.version < 0x72)
		return false;
	if (header.nodeNum > 0)
	{
		pOut->reserve(header.nodeNum);
		for (int i = 0; i < header.nodeNum; i++)
		{
			sAiBinLine983 line;
			pFile->Read(&line, sizeof(line));
			CNodeContainer node(line.name);
			if (strstr(line.name, "ai"))
				node.SetType((CNodeContainer::eType)0xf);
			for (int j = 0; j < line.count; j++)
			{
				float pos[3];
				pFile->Read(pos, sizeof(pos));
				node.Add(WVector(pos[0], pos[1], pos[2]));
			}
			pOut->push_back(node);
		}
	}
	return true;
}

#define AIDOC983 ((CAIPlayerInfoContainer983*)this)

// 009091b0
int CAIDocJp::GetCount983()
{
	return AIDOC983->m_infoMap.size();
}

// 0090a520: ainda sem as linhas, os resultados ou os adversários
bool CAIDocJp::IsBusy983()
{
	if (AIDOC983->m_lineMap.empty() || AIDOC983->m_infoMap.empty() || AIDOC983->m_npcIds.empty())
		return true;
	return false;
}

// 0090d410: resultado da IA `index` no buraco `hole` (não mexe em *pOut se não houver). O 983 não devolve valor (C75.1).
int CAIDocJp::GetInfo983(int index, int hole, sAIDocInfo983* pOut)
{
	if (AIDOC983->m_infoMap.size())
	{
		std::map<int, std::vector<sAIDocInfo983> >::iterator it = AIDOC983->m_infoMap.find(index);
		if (it != AIDOC983->m_infoMap.end())
		{
			std::vector<sAIDocInfo983> infos = it->second;
			if ((unsigned int)hole < infos.size())
				*pOut = infos[hole];
		}
	}
	return 0;
}

// 0090fe00: linhas do buraco `hole` (b = std::vector<CNodeContainer>*)
bool CAIDocJp::Fn90fe00_983(int hole, int b)
{
	std::map<int, std::vector<CNodeContainer> >::iterator it = AIDOC983->m_lineMap.find(hole);
	if (it == AIDOC983->m_lineMap.end())
		return false;
	*(std::vector<CNodeContainer>*)b = it->second;
	return true;
}

// 0090d900: os `count` primeiros adversários (todos, se não houver mais que `count`; param = std::vector<unsigned long>*)
void CAIDocJp::Fn90d900_983(int count, int param)
{
	std::vector<unsigned long>* pOut = (std::vector<unsigned long>*)param;
	pOut->clear();
	if (AIDOC983->m_npcIds.size() > (unsigned int)count)
		pOut->assign(AIDOC983->m_npcIds.begin(), AIDOC983->m_npcIds.begin() + count);
	else
		pOut->assign(AIDOC983->m_npcIds.begin(), AIDOC983->m_npcIds.end());
}

// 00911180: lê o .aibin de cada buraco da partida ("map%02d_%s_hole%02d.aibin") e volta ao mapa do 1º buraco. Arquivo que
// falta ou com versão velha encerra sem voltar o mapa, como no 983.
void CAIDocJp::Fn911180_983()
{
	for (int i = 0; i < Doc()->m_golfGame.holes; i++)
	{
		SetCurMap(Doc()->m_pinIndex[i]);
		std::string name = MakeStr("map%02d_%s_hole%02d.aibin", GetCurMap(), GetCurMapName(), Doc()->m_holeOrder[i]);
		cFile* pFile = g_resrcmng->GetCFile(name.c_str(), -1);
		if (!pFile)
		{
			CloseCFile(pFile);
			return;
		}
		std::vector<CNodeContainer> lines;
		if (!AIDOC983->ReadAiBin983(pFile, &lines))
		{
			CloseCFile(pFile);
			return;
		}
		AIDOC983->m_lineMap.insert(std::pair<int, std::vector<CNodeContainer> >(i, lines));
		CloseCFile(pFile);
	}
	SetCurMap(Doc()->m_pinIndex[0]);
}

// 00909180
int CAIDocJp::GetNpcId983(int index)
{
	if ((unsigned int)index >= AIDOC983->m_npcIds.size())
		return 0;
	return AIDOC983->m_npcIds[index];
}

// B298: taxa de bonus do jogador (983 00636040); usada pelas telas de resultado (OnBonusInit). kind 0 soma (base 0), 1-3 multiplica (base 1), 4 fixa
float CSharedDocJp::GetBonusRate983(unsigned char idx, unsigned char kind)
{
	if (((CSharedDoc*)this)->m_pGolfDoc) // A729: membros (cl novo m_pGolfDoc 0x10cc4, VC10 0x11024; m_playerNum +0xd5)
	{
		CGolfDoc* p = CSharedDoc::Instance()->m_pGolfDoc;
		if (p->m_playerNum <= idx)
			return kind ? 1.0f : 0.0f;
	}
	float result = kind ? 1.0f : 0.0f;
	// lista de 5 itens do jogador (+0x69a6 + idx*0x31c6); so as posicoes 0 (item auxiliar) e 1 (vetor de 0x14 bytes) contam
	unsigned long* ids = (unsigned long*)((CSharedDoc*)this)->m_userInfo[idx].charInfo.tidAuxParts; // B1458: membro (VC10 +0x69a6)
	if (ids[0])
	{
		IFF_STRUCT::sAuxPart* aux = ItemManager()->FindAuxPart(ids[0]);
		if (aux && kind <= 4)
		{
			switch (kind)
			{
			case 0:
				result = (float)(signed char)aux->DriveUp + result;
				break;
			case 1:
			case 2:
			case 3:
			{
				unsigned short v = (&aux->ItemDropUp)[kind - 1];
				if (v)
					result = (float)v / 100.0 * result;
				break;
			}
			case 4:
			{
				unsigned short type = *(unsigned short*)&aux->c.TypeId;
				if (type == 9)
					result = 9.0f;
				else if ((IsLocalContent((localContentType_t)0x4c) && type == 0xb) ||
					((aux->c.TypeId & 0xffff) == 0x83 &&
						(aux->c.TypeId & 0x1f0000) == 0x10000))
					result = 11.0f;
				break;
			}
			}
		}
	}
	if (ids[1])
	{
		char** vec = (char**)&((CSharedDoc*)this)->m_auxPartValues983[idx];
		char* v = *vec;
		if (v)
		{
			char* b = *(char**)v;
			char* e = *(char**)(v + 4);
			if (b != e && (e - b) / 0x14 > 1)
			{
				char* el = b + 0x14;
				int t = *(int*)(el + 4);
				float f = *(float*)(el + 8);
				if (t != -1 && t == kind)
				{
					switch (t)
					{
					case 0:
						result = f + result;
						break;
					case 1:
					case 2:
					case 3:
						if (0.0f < f)
							result = f * result;
						break;
					case 4:
						result = f;
						break;
					}
				}
			}
		}
	}
	return result;
}
