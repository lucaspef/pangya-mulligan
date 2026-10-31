#pragma once
// JP 983: unidade 0049c430–0049cb90 = funções livres (__fastcall pelo /Gr) dos atributos do taco: soma do atributo-base do
// ClubSet.iff com o que o jogador subiu (sItemInfo), teto pelos slots (+ cartas do taco, + bônus de rank), grau pela soma
// dos slots. Sem classe nem RTTI; par 839 0049fd40…; chamadas pela oficina (B) e pela A (FrDowngradeDlg::SetLevelBar
// 00aa60f0, 00614c60, CRealMyRoomMain::OnItemListOwnerDraw 006166b0, CItemManager 00a1fb40/00a23620). Nomes meus (sessão B88).
namespace IFF_STRUCT { struct sClubSet; }

class CCardManager;
struct sItemInfo;

// Argumento das contas de capacidade do CItemManager do 983 (00a1fb40 = GetCharCapacity do 645 com os argumentos numa
// struct). Só os campos lidos; nomes meus.
struct sCapacityParam983
{
	int type; // +0: atributo (eLvlType)
	unsigned long tidChar; // +4
	unsigned long m_8; // +8
	const unsigned long* tidParts; // +0xc
	const unsigned long* tidAuxParts; // +0x10
	unsigned long tidClub; // +0x14
	const short* clubAttrUp; // +0x18: subidas do atributo (sItemInfo::Common)
	const short* clubSlotUp; // +0x1c: slots ganhos (sItemInfo +0xaa)
	unsigned long guidClub; // +0x20
	unsigned long tidCaddie; // +0x24
	unsigned long m_28; // +0x28
	unsigned char level; // +0x2c
	int m_30; // +0x30: extra da maestria (0 no FrPlayerDlg::SetLevelBar 00b22230)
};

// Método do CItemManager que o fonte do 645 não tem (classe de empréstimo, como CItemManagerJp da seteffectobject.h).
class CItemManagerStatJp
{
public:
	// 00a06fb0: tabela de bônus por (tipo do taco, grau) em +0x38c (registros de 0x14); false se não achou
	bool GetClubRankBonus983(int type, int grade, short* p0, short* p1, short* p2, short* p3, short* p4);
};

// Método do CItemManager (tabela ClubSetWorkShopRankUpExp em +0x39c)
class CItemManagerRankUpJp
{
public:
	// 00a07030: registro com +0 == typeId -> *pExp = dword [+4 + 4*rank]; false se não achou
	bool GetRankUpExp983(unsigned long typeId, int rank, int* pExp);
};

unsigned short AddStat983(short a, short b, short c); // 0049c430
CCardManager* GetCardManager983(); // 0049c440
short MinStat983(short a, short b); // 0049c450
unsigned short AddStatMax983(short a, short b, short max); // 0049c460
short GetCardSlot983(int type); // 0049c480: CCardManager::GetClubCardStatusSlot983
unsigned short GetClubSlotMax983(int type, IFF_STRUCT::sClubSet* pClub, sItemInfo* pItem, bool bCard); // 0049c490
IFF_STRUCT::sClubSet* FindClubSetIff983(unsigned long tid); // 0049c4e0
unsigned short GetClubStat983(int type, IFF_STRUCT::sClubSet* pClub, sItemInfo* pItem, bool bCard); // 0049c500
int GetClubSetSlotSum983(unsigned long tid); // 0049c550
const char* GetClubSetName983(unsigned long tid); // 0049c590
int GetClubSetStep983(unsigned long tid); // 0049c5a0: soma 30…59 → 0…4 dentro do grau
int GetClubSetGrade983(unsigned long tid); // 0049c610: soma 30…59 → grau 0…5
sItemInfo* FindMyClubSet983(unsigned long guid); // 0049c690: busca linear no CSharedDoc::m_clubSetMap
short GetMyClubStat983(int type, unsigned long guid, bool bCard); // 0049c6e0 (bCard não lido: sempre com cartas)
short GetMyClubSlotMax983(int type, unsigned long guid, bool bCard); // 0049c720
int GetMyClubSlotSum983(unsigned long guid); // 0049c770
bool HasClubCard983(unsigned long guid); // 0049c7f0
IFF_STRUCT::sClubSet* FindMyClubSetIff983(unsigned long guid); // 0049c820
int GetMyClubStep983(unsigned long guid); // 0049c840
int GetMyClubGrade983(unsigned long guid); // 0049c8b0
bool IsClubStepChanged983(unsigned long guid); // 0049c930
bool IsClubGradeChanged983(unsigned long guid); // 0049c960
short GetClubRankBonus983(int type, IFF_STRUCT::sClubSet* pClub, sItemInfo* pItem); // 0049c990
short GetCapacitySlot983(const sCapacityParam983* p, bool bCard); // 0049ca10
short GetMyClubRankBonus983(int type, unsigned long guid); // 0049cb00
short GetCapacityStat983(const sCapacityParam983* p, bool bCard); // 0049cb30
