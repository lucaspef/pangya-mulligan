#pragma once

#include <string.h>
#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

#include "classdefine.h"

enum eLvlType
{
	LVL_POWER,
	LVL_CONTROL,
	LVL_ACCURACY,
	LVL_SPIN,
	LVL_CURVE,
	LVL_NUM_MAX
};

enum PANGYA_ITEM_GROUP
{
	PIG_CHAR = 1,
	PIG_PART,
	PIG_CLUB,
	PIG_CLUBSET,
	PIG_BALL,
	PIG_ITEM,
	PIG_CADDIE,
	PIG_CADITEM,
	PIG_SETITEM,
	PIG_COURSE,
	PIG_MATCH,
	PIG_TITLE,
	PIG_ENCHANT,
	PIG_SKIN,
	PIG_HAIR,
	PIG_MASCOT,
	PIG_CHILDITEM,
	PIG_FURNITURE,
	PIG_OFFLINESHOP,
	PIG_AUXPART = 28,
	PIG_QUESTDROP,
	PIG_QUEST,
	PIG_CARD,
	PIG_MAX
};

enum specialcardavility
{
	SPECIAL_EXP_ADD,
	SPECIAL_PANG_RATE_UP_TIME,
	SPECIAL_EXP_REAE_UP_TIME,
	SPECIAL_PANG_ADD,
	SPECIAL_POWER_UP_TIME,
	SPECIAL_CONTROL_UP_TIME,
	SPECIAL_ACCCURACY_UP_TIME,
	SPECIAL_SPIN_UP_TIME,
	SPECIAL_CURVE_UP_TIME,
	SPECIAL_START_COMBOGAUGE_UP_TIME,
	SPECIAL_SLOT_UP_TIME,
	SPECIAL_PANGYAZONE_UP,
	SPECIAL_CLEARBONUS_UP_SEPIAWIND,
	SPECIAL_CLEARBONUS_UP_WINDHILL,
	SPECIAL_CLEARBONUS_UP_PINKWIND,
	SPECIAL_CLEARBONUS_UP_BLUEMOON,
	SPECIAL_RANDOMPANG_ADD,
	SPECIAL_TREASUREPOINT_UP,
	SPECIAL_RAINFALL_PROBABILITY_UP,
	SPECIAL_AVILITY_MAX
};

enum COUNTING_TYPE;
enum eSPAVILITYTYPE;

struct status_t
{
	unsigned char level;
	unsigned char penalty;
};

typedef std::map<unsigned int, IFF_STRUCT::sChar>::iterator CharIterator;
typedef std::map<unsigned int, IFF_STRUCT::sPart>::iterator PartIterator;
typedef std::map<unsigned int, IFF_STRUCT::sClub>::iterator ClubIterator;
typedef std::map<unsigned int, IFF_STRUCT::sClubSet>::iterator ClubSetIterator;
typedef std::map<unsigned int, IFF_STRUCT::sBall>::iterator BallIterator;
typedef std::map<unsigned int, IFF_STRUCT::sItem>::iterator ItemIterator;
typedef std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator CaddieIterator;
typedef std::map<unsigned int, IFF_STRUCT::sCadItem>::iterator CadItemIterator;
typedef std::map<unsigned int, IFF_STRUCT::sSetItem>::iterator SetItemIterator;
typedef std::map<unsigned int, IFF_STRUCT::sCourse>::iterator CourseIterator;
typedef std::map<unsigned int, IFF_STRUCT::sMatch>::iterator MatchIterator;
typedef std::map<unsigned int, IFF_STRUCT::sTitle>::iterator TitleIterator;
typedef std::map<unsigned int, IFF_STRUCT::sEnchant>::iterator EnchantIterator;
typedef std::map<unsigned int, IFF_STRUCT::sDesc>::iterator DescIterator;
typedef std::map<unsigned int, IFF_STRUCT::sSkin>::iterator SkinIterator;
typedef std::map<unsigned int, IFF_STRUCT::sHairStyle>::iterator
	HairStyleIterator;
typedef std::map<unsigned int, IFF_STRUCT::sChildItem>::iterator
	ChildItemIterator;
typedef std::map<unsigned int, IFF_STRUCT::sMascot>::iterator MascotIterator;
typedef std::map<unsigned int, IFF_STRUCT::sAuxPart>::iterator AuxPartIterator;
typedef std::map<unsigned int, IFF_STRUCT::sQuestDrop>::iterator
	QuestDropIterator;
typedef std::map<unsigned int, IFF_STRUCT::sQuest>::iterator QuestIterator;
typedef std::map<unsigned int, IFF_STRUCT::sFurniture>::iterator
	FurnitureIterator;
typedef std::map<unsigned int, IFF_STRUCT::sCard>::iterator CardIterator;
typedef std::map<unsigned int, IFF_STRUCT::sOfflineShop>::iterator
	OfflineShopIterator;
typedef std::list<IFF_STRUCT::sCadieMagicBox>::iterator CadieMagicBoxIterator;
typedef std::list<IFF_STRUCT::sTikiPointTable>::iterator TikiPointTableIterator;
typedef std::list<IFF_STRUCT::sTikiSpecialRecipe>::iterator
	TikiSpecialRecipeIterator;
typedef std::map<unsigned int, IFF_STRUCT::S5::sCutinInformation>::iterator
	CutinInformationIterator;
typedef std::map<unsigned int, IFF_STRUCT::sSpecialPrizeItem>::iterator
	SpecialPrizeItemIterator;

namespace _private
{
	struct sRandomBoxItem
	{
		IFF_STRUCT::sRandomBox box;
		std::vector<IFF_STRUCT::sRandomBox> checkItems;

		sRandomBoxItem()
		{
			memset(this, 0, sizeof(IFF_STRUCT::sRandomBox));
			checkItems.clear();
		}

		~sRandomBoxItem() { checkItems.clear(); }
	};

	struct sRandomBoxStuff
	{
		IFF_STRUCT::sRandomBox box;

		sRandomBoxStuff() { memset(this, 0, sizeof(sRandomBoxStuff)); }
	};

	struct sRandomBox
	{
		IFF_STRUCT::sRandomBox box;
		std::map<unsigned long, sRandomBoxItem> items;
		std::vector<sRandomBoxStuff> stuffs;
		std::vector<sRandomBoxStuff> stuffsType5; // +0x104: registros do tipo 5 (00a3ddc9). JP 983: o valor do mapa tem 0x114 (nó 0x128 em 00a3fb80); tipo não conferido

		sRandomBox()
		{
			memset(this, 0, sizeof(IFF_STRUCT::sRandomBox));
			items.clear();
			stuffs.clear();
		}

		~sRandomBox()
		{
			items.clear();
			stuffs.clear();
		}
	};
}

typedef std::map<unsigned int, _private::sRandomBox>::iterator
	RandomBoxIterator;
typedef std::map<unsigned int, IFF_STRUCT::sNonVisibleItem>::iterator
	NonVisibleItemIterator;
typedef std::map<unsigned int, IFF_STRUCT::sSubscriptionItem>::iterator
	SubscriptionItemIterator;

// JP 983: registros das tabelas que só o 983 carrega (tamanho = passo do leitor; chave e bFinal lidos no leitor de
// cada uma; o resto dos campos ainda não tem nome). Leitores e deslocamentos: out/F3.5/s78/RELATORIO.md.
namespace IFF983
{
#define IFF983_REC(nome, tam) \
	struct nome \
	{ \
		unsigned char bFinal; \
		unsigned long TypeId; \
		unsigned char raw[(tam) - 8]; \
	}
#define IFF983_RAW(nome, tam) \
	struct nome \
	{ \
		unsigned char raw[tam]; \
	}
	IFF983_REC(sAchievement, 0x600); // Achievement.iff, leitor 00a2aed0
	IFF983_REC(sCounterItem, 0x118); // CounterItem.iff, 00a2a9e0
	IFF983_REC(sQuestStuff, 0x10c); // QuestStuff.iff, 00a2aa90
	IFF983_REC(sQuestItem, 0x110); // QuestItem.iff, 00a2ab60
	IFF983_REC(sErrorCodeInfo, 0x110); // ErrorCodeInfo.iff, 00a2a930
	IFF983_REC(sArtifactManaInfo, 0x98); // ArtifactManaInfo.iff, 00a2b770
	IFF983_REC(sGrandPrixData, 0x330); // GrandPrixData.iff, 00a2b820

	// Ability.iff (0x50), leitor 00a2afa0: a chave é o 1º campo e o bFinal fica em +0x4c
	struct sAbility
	{
		unsigned long TypeId;
		unsigned char raw[0x48];
		unsigned char bFinal; // +0x4c
	};

	// SetEffectTable.iff (0x4c), leitor 00a2b050: a chave do mapa é o id de 16 bits do começo. Campos lidos por
	// CalcSetEffect983 00a18a80 e pelos leitores 009d9a70/009d9a90/009d9ad0; os nomes são provisórios.
	struct sSetEffectTable
	{
		unsigned short id;
		unsigned long effect[3]; // +0x04: 009d9a90 conta quantos valem 6
		unsigned char raw10[0xc];
		unsigned long tidPiece[5]; // +0x1c: peças do conjunto
		unsigned char flag[5]; // +0x30
		short value36[5]; // +0x36: 009d9ad0
		short value40[5]; // +0x40: 009d9a90
		short range; // +0x4a: 009d9a70
	};

	// LevelUpPrizeItem.iff: o arquivo traz 0xc0 por registro, o mapa guarda 0x1c4 (leitor 00a2a880; chave = byte +0x22)
	struct sLevelUpPrizeItem
	{
		unsigned char bFinal;
		unsigned char raw01[0x21];
		unsigned char level; // +0x22
		unsigned long tidPrize[2]; // +0x24: 0x1a000010 = pang
		int nPrize[2]; // +0x2c
		unsigned char raw34[0xc0 - 0x34];
		// não vêm do arquivo: textos montados depois do laço do Load (00a351f0)
		char szPrize[0x81]; // +0xc0: prêmio deste nível
		char szNextPrize[0x81]; // +0x141: prêmio do nível seguinte
	};

	// ShopLimitItem.iff (0x3c), leitor 00a2a360: chave em +0x08
	struct sShopLimitItem
	{
		unsigned char bFinal;
		unsigned long flag; // +0x04: 1 nos 3 registros do JP 966
		unsigned long TypeId; // +0x08
		unsigned long total; // +0x0c: quantidade total da venda (00a897c0 "\xc3\xd1 %d")
		unsigned long perDay; // +0x10: quantidade por periodo (00a897c0)
		unsigned long hours; // +0x14: periodo em horas; 0 = so o total (00a897c0)
		unsigned long raw18; // +0x18: sem leitor em 00a897c0/00a18700
		_SYSTEMTIME saleStart; // +0x1c: inicio da venda (00a18700)
		_SYSTEMTIME saleEnd; // +0x2c: fim da venda; wYear 0 = sem prazo (00a897c0)
	};

	// HoleCupDropItem.iff (0x2c), leitor 00a2b8d0: sem teste de bFinal; chave no 1º campo
	struct sHoleCupDropItem
	{
		unsigned long TypeId;
		unsigned char raw[0x28];
	};

	IFF983_RAW(sScratchShopConfig, 0x14); // ScratchShopConfig.iff, 009d9e60 (um registro só)
	IFF983_RAW(sClubSetWorkShopChangeProb, 0x1c); // ClubSetWorkShopChangeProb.sff, 009d9f10 (um registro só)
	IFF983_RAW(sAttendancePrizeConfig, 0x8c); // AttendancePrizeConfig.sff, 009d9f70 (um registro só)

	// PointShop.iff (0x14), leitor 00a2f610: sem teste de bFinal; chave em +0x04
	struct sPointShop
	{
		unsigned long active; // byte baixo = ativo (pointshopdlg.cpp FUN de lista); 0xffffffff nos 12 registros do JP 966
		unsigned long TypeId; // +0x04
		unsigned char raw[0xc];
	};

	// ScratchItem.sff (0x30), leitor 00a2a550: só o tid (+0x0c) dos registros com bFinal e bit 1 de +0x04 é guardado
	struct sScratchItem
	{
		unsigned char bFinal;
		unsigned long flag; // +0x04
		unsigned long pad08; // o leitor não lê
		unsigned long TypeId; // +0x0c
		unsigned char raw[0x20];
	};

	// BroadCastNotices.sff (0x230), leitor 00a36150 (bFinal e bit 1 de +0x04). 009e9dc0 lê o instante de +0x220 e
	// grava em +0x230/+0x238 de uma cópia maior; o que há no meio não tem nome.
	struct sBroadCastNotice
	{
		unsigned char bFinal;
		unsigned long flag; // +0x04
		unsigned long pad08; // nenhum leitor conhecido lê
		unsigned long periodUnit; // +0x0c: 1 = semana, 2 = dia, 3 = hora, 4 = minuto (009df4f0)
		unsigned long periodCount; // +0x10
		unsigned char raw[0x20c];
		__int64 time220; // +0x220
		unsigned char raw228[8];
	};

	// Aviso como fica guardado (0x240): o registro do arquivo + próxima e última exibição (00a34020 monta, 009e9dc0 calcula)
	struct sBroadCastNoticeEntry
	{
		sBroadCastNotice notice;
		__int64 next; // +0x230
		__int64 last; // +0x238
	};

	// TwinsItemTable.iff (0x1c), leitor 00a39aa0 + 00a390a0: cada tid do grupo aponta para todos os tids do grupo
	struct sTwinsItem
	{
		unsigned char bFinal;
		unsigned long flag; // +0x04
		unsigned long tid[5]; // +0x08
	};

	// GrandPrixAIOptionalData.sff (0x25c), leitor 00a380b0: 3 índices (por id, pelo byte +0x34 e pelo valor +0x54)
	struct sGrandPrixAIOptionalData
	{
		unsigned char bFinal;
		unsigned char pad01[3];
		unsigned short id; // +0x04
		unsigned char raw06[0x2e];
		unsigned char key34; // +0x34
		unsigned char raw35[0x1f];
		unsigned long key54; // +0x54 (0 = fora do índice de +0x4e4)
		unsigned char raw58[0x204];
	};

#define IFF983_FIN(nome, tam) \
	struct nome \
	{ \
		unsigned char bFinal; \
		unsigned char raw[(tam) - 1]; \
	}
	IFF983_FIN(sBongdariShopNormalItem, 0x10); // BongdariShopNormalItem.sff, 00a31fe0
	IFF983_FIN(sBongdariShopRareItem, 0x30); // BongdariShopRareItem.sff, 00a32180
	IFF983_FIN(sScratchShopItem, 0x18); // ScratchShopItem.iff, 00a32230
	IFF983_FIN(sAttendancePrizeItem, 0x14); // AttendancePrizeItem.sff, 00a32690
#undef IFF983_FIN
	IFF983_REC(sGrandPrixSpecialHole, 0x14); // GrandPrixSpecialHole.iff, 00a3c200 (chave em +0x04)
	IFF983_REC(sGrandPrixConditionEquip, 0x210); // GrandPrixConditionEquip.iff, 00a3b470 (chave em +0x04)
	IFF983_REC(sGrandPrixRankReward, 0x4c); // GrandPrixRankReward.iff, 00a3b6d0 (chave em +0x04)
	IFF983_REC(sAddonPart, 0x120); // AddonPart.iff, 00a3bef0 (tid em +0x04)

	// lidos sem teste de bFinal e guardados inteiros (campos sem nome)
	// mapa de +0x2e8: 5 registros fixos montados por 009d8890 e postos por 00a2a620 depois do laço do Load (não vêm
	// de arquivo). Os nomes são condições de "hole in" (1 tentativa, tacada especial, sem comando, PangYa falhado).
	struct sCupInCondition
	{
		unsigned char bFinal;
		unsigned long id; // +0x04: 1..5 (chave do mapa)
		unsigned long kind; // +0x08: sempre 10
		unsigned long flags; // +0x0c
		unsigned long param10; // +0x10: 0 nos 5 registros
		unsigned long param14; // +0x14: 0, 1, 1, 1, 3 (ids 1..5); nosso código não lê
		char Name[0x80]; // +0x18
		unsigned long pad98; // +0x98: o 983 não grava
		unsigned long flag9c; // +0x9c: só os bits 2 e 4
	};

	// elemento do vetor de +0x548 (0x38): 2 por campo, tirados dos 12 números novos do Course.iff (00a33cb0)
	struct sCourseBonus
	{
		__int64 value0; // sem vetor: a função do 983 não tem cookie de pilha
		__int64 value1;
		__int64 value2;
		__int64 value3;
		__int64 value4;
		__int64 value5;
		unsigned char course; // +0x30: byte baixo do tid do campo
		unsigned char bSecond; // +0x31: 0 = 1ª faixa (+0x168), 1 = 2ª faixa (+0x180)
	};

	IFF983_RAW(sClubSetWorkShopLevelUpProb, 0x18); // ClubSetWorkShopLevelUpProb.iff, 00a322d0
	IFF983_RAW(sClubSetWorkShopLevelUpLimit, 0x14); // ClubSetWorkShopLevelUpLimit.iff, 00a323e0
	IFF983_RAW(sClubSetWorkShopRankUpExp, 0x1c); // ClubSetWorkShopRankUpExp.iff, 00a324e0
	IFF983_RAW(sClubSetWorkShopChangeItem, 0x14); // ClubSetWorkShopChangeItem.sff, 00a32590
	IFF983_RAW(sMemorialShopCoinItem, 0x40); // MemorialShopCoinItem.sff, 00a32850
	IFF983_RAW(sMemorialShopNormalItem, 0x48); // MemorialShopNormalItem.sff, 00a32a10
	IFF983_RAW(sMemorialShopRareItem, 0x5c); // MemorialShopRareItem.iff e .sff, 00a32b50 (no KR 839 o registro tem 0x44)
	IFF983_RAW(sCharacterMastery, 0x3c); // CharacterMastery.iff, 00a32d70
	IFF983_RAW(sCaddieVoice, 0x86); // CaddieVoiceTable.iff, 00a32e20
#undef IFF983_REC
#undef IFF983_RAW
}

struct sCapacityParam983; // include/jp/clubsetstat.h
class CItemManager
{
public:
	CItemManager();
	virtual ~CItemManager();

	void Reset();
	bool Load();
	void Reload();

	// JP 983: layout medido no construtor 00a3fc00, no destrutor 00a3edc0 e nos leitores do Load 00a3de50. O 983 não
	// tem a seção crítica nem o m_IsReloading do 645: em +0x04 fica o nome do pacote já carregado (o Load compara e
	// não recarrega o mesmo) e os mapas começam em +0x20.
	std::string m_strLoaded983; // +0x004
	std::map<unsigned int, IFF_STRUCT::sChar> m_CharMap; // +0x020
	std::map<unsigned int, IFF_STRUCT::sPart> m_PartMap; // +0x030
	std::map<unsigned int, IFF_STRUCT::sClub> m_ClubMap; // +0x040
	std::map<unsigned int, IFF_STRUCT::sClubSet> m_ClubSetMap; // +0x050
	std::map<unsigned int, IFF_STRUCT::sBall> m_BallMap; // +0x060
	std::map<unsigned int, IFF_STRUCT::sItem> m_ItemMap; // +0x070
	std::map<unsigned int, IFF_STRUCT::sCaddie> m_CaddieMap; // +0x080
	std::map<unsigned int, IFF_STRUCT::sCadItem> m_CadItemMap; // +0x090
	std::map<unsigned int, IFF_STRUCT::sSetItem> m_SetItemMap; // +0x0a0
	std::map<unsigned int, IFF_STRUCT::sCourse> m_CourseMap; // +0x0b0
	std::map<unsigned int, IFF_STRUCT::sMatch> m_MatchMap; // +0x0c0
	std::map<unsigned int, IFF_STRUCT::sTitle> m_TitleMap; // +0x0d0
	std::map<unsigned int, IFF_STRUCT::sEnchant> m_EnchantMap; // +0x0e0
	std::map<unsigned int, IFF_STRUCT::sDesc> m_DescMap; // +0x0f0
	std::map<unsigned int, IFF_STRUCT::sSkin> m_SkinMap; // +0x100
	std::map<unsigned int, IFF_STRUCT::sHairStyle> m_HairStyleMap; // +0x110
	std::map<unsigned int, IFF_STRUCT::sChildItem> m_ChildItemMap; // +0x120
	std::map<unsigned int, IFF_STRUCT::sMascot> m_MascotMap; // +0x130
	std::map<unsigned int, IFF_STRUCT::sAuxPart> m_AuxPartMap; // +0x140
	std::map<unsigned int, IFF_STRUCT::sQuestDrop> m_QuestDropMap; // +0x150 (nenhum leitor do 983 grava nele)
	std::map<unsigned int, IFF_STRUCT::sQuest> m_QuestMap; // +0x160 (idem; o Reset ainda libera aTidItems)
	std::map<unsigned int, IFF_STRUCT::sFurniture> m_FurnitureMap; // +0x170
	std::map<unsigned int, IFF983::sAchievement> m_AchievementMap983; // +0x180
	std::map<unsigned int, IFF983::sAbility> m_AbilityMap983; // +0x190
	std::map<unsigned int, IFF983::sSetEffectTable> m_SetEffectTableMap983; // +0x1a0
	std::map<unsigned int, IFF_STRUCT::sCard> m_CardMap; // +0x1b0
	std::multimap<unsigned int, IFF_STRUCT::sRandomRecycle> m_RandomRecycleMaps; // +0x1c0
	std::list<IFF_STRUCT::sCadieMagicBox> m_CadieMagicBox; // +0x1d0
	std::list<IFF_STRUCT::sFurnitureAbility> m_FurnitureAbility; // +0x1dc
	std::list<IFF_STRUCT::sTikiPointTable> m_TikiPointTable; // +0x1e8
	std::list<IFF_STRUCT::sTikiSpecialRecipe> m_TikiSpecialRecipe; // +0x1f4
	std::map<unsigned int, IFF983::sPointShop> m_PointShopMap983; // +0x200
	std::map<unsigned int, IFF983::sShopLimitItem> m_ShopLimitItemMap983; // +0x210
	std::map<unsigned int, _private::sRandomBox> m_RandomBoxMap; // +0x220
	std::map<unsigned int, IFF_STRUCT::sNonVisibleItem> m_NonVisibleItemMap; // +0x230
	std::map<unsigned int, IFF_STRUCT::sSubscriptionItem> m_SubscriptionItemMap; // +0x240
	std::vector<unsigned long> m_ScratchItem983; // +0x250: tids (00a2a550)
	std::vector<IFF983::sBongdariShopNormalItem> m_BongdariShopNormalItem983; // +0x260
	std::vector<IFF983::sBongdariShopRareItem> m_BongdariShopRareItem983; // +0x270
	std::map<unsigned int, IFF_STRUCT::S5::sCutinInformation>
		m_CutinInformationMap; // +0x280
	std::map<unsigned int, IFF_STRUCT::sSpecialPrizeItem> m_SPItemMap[5]; // +0x290
	std::vector<IFF983::sBroadCastNoticeEntry>* m_pBroadCastNotices983; // +0x2e0 (new 0x10 no construtor; elemento de 0x240)
	std::map<unsigned int, std::vector<unsigned long> >* m_pTwinsItemMap983; // +0x2e4: tid -> tids do grupo (00a390a0)
	std::map<unsigned int, IFF983::sCupInCondition> m_CupInConditionMap983; // +0x2e8 (00a2a620)
	std::map<unsigned int, IFF983::sLevelUpPrizeItem> m_LevelUpPrizeItemMap983; // +0x2f8
	std::map<unsigned int, IFF983::sErrorCodeInfo> m_ErrorCodeInfoMap983; // +0x308
	std::map<unsigned int, IFF983::sCounterItem> m_CounterItemMap983; // +0x318
	std::map<unsigned int, IFF983::sQuestStuff> m_QuestStuffMap983; // +0x328
	std::map<unsigned int, IFF983::sQuestItem> m_QuestItemMap983; // +0x338
	std::map<unsigned int, IFF983::sArtifactManaInfo> m_ArtifactManaInfoMap983; // +0x348
	IFF983::sScratchShopConfig m_ScratchShopConfig983; // +0x358
	std::vector<IFF983::sScratchShopItem> m_ScratchShopItem983; // +0x36c
	std::vector<IFF983::sClubSetWorkShopLevelUpProb> m_WorkShopLevelUpProb983; // +0x37c
	std::vector<IFF983::sClubSetWorkShopLevelUpLimit> m_WorkShopLevelUpLimit983; // +0x38c
	std::vector<IFF983::sClubSetWorkShopRankUpExp> m_WorkShopRankUpExp983; // +0x39c
	IFF983::sClubSetWorkShopChangeProb m_WorkShopChangeProb983; // +0x3ac
	std::vector<IFF983::sClubSetWorkShopChangeItem> m_WorkShopChangeItem983; // +0x3c8
	IFF983::sAttendancePrizeConfig m_AttendancePrizeConfig983; // +0x3d8
	std::vector<IFF983::sAttendancePrizeItem> m_AttendancePrizeItem983; // +0x464
	std::map<unsigned int, std::multimap<unsigned int, IFF983::sAddonPart> > m_AddonPartMap983; // +0x474: grupo -> tid -> registros (00a3bef0; multimap: o insert 00a2e620 nao testa chave igual, ex. brinco 081a4801 = Wing_Feather + g_wing)
	std::map<unsigned int, IFF983::sHoleCupDropItem> m_HoleCupDropItemMap983; // +0x484
	std::vector<IFF983::sMemorialShopCoinItem> m_MemorialShopCoinItem983; // +0x494
	std::vector<IFF983::sMemorialShopNormalItem> m_MemorialShopNormalItem983; // +0x4a4
	std::vector<IFF983::sMemorialShopRareItem> m_MemorialShopRareItem983; // +0x4b4
	std::map<unsigned int, IFF983::sGrandPrixAIOptionalData> m_GrandPrixAIMap983; // +0x4c4
	std::map<unsigned char, std::vector<unsigned short> > m_GrandPrixAIMap983_4d4; // +0x4d4: byte +0x34 -> ids (00a380b0)
	std::map<unsigned int, std::vector<unsigned short> > m_GrandPrixAIMap983_4e4; // +0x4e4: valor +0x54 -> ids (00a380b0)
	std::map<unsigned int, IFF983::sGrandPrixData> m_GrandPrixDataMap983; // +0x4f4
	std::map<unsigned int, std::vector<IFF983::sGrandPrixSpecialHole> > m_GrandPrixSpecialHoleMap983; // +0x504 (conferido: 00a3c200)
	std::map<unsigned int, std::vector<IFF983::sGrandPrixConditionEquip> > m_GrandPrixConditionEquipMap983; // +0x514 (conferido: 00a3b470)
	std::map<unsigned int, std::vector<IFF983::sGrandPrixRankReward> > m_GrandPrixRankRewardMap983; // +0x524 (conferido: 00a3b6d0)
	unsigned long m_AlchemyConfig983; // +0x534
	std::vector<IFF983::sCharacterMastery> m_CharacterMastery983; // +0x538
	std::vector<IFF983::sCourseBonus> m_CourseBonus983; // +0x548 (00a33cb0)
	std::vector<IFF983::sCaddieVoice> m_CaddieVoice983; // +0x558

private:
	void MakeItemBuffMap(); // 983: 00a3abd0

	// JP 983: fim do Load (depois do laço dos arquivos)
	void MakeCupInConditionMap983(); // 00a2a620
	void MakeDerivedTables983(); // 00a36130
	void ShuffleScratchItem983(); // 00a23760
	void MakeLevelUpPrizeText983(); // 00a351f0
	void MakeAchievementQuestItem983(); // 00a2b970
	void MakeCourseBonus983(); // 00a33cb0
	std::string GetPrizeText983(const IFF983::sLevelUpPrizeItem* pItem); // 00a2f970
	std::string GetLevelPrizeText983(const IFF983::sLevelUpPrizeItem* pItem); // 00a331a0
	std::string GetNextLevelPrizeText983(const IFF983::sLevelUpPrizeItem* pItem); // 00a33350

	struct sItemBuffHelper
	{
		specialcardavility BuffType;
		int Value;
		int Duration;
		int Accum;
		std::set<unsigned long> ExclusiveSet;
	};
	typedef std::map<unsigned long, sItemBuffHelper> BuffMap;
	typedef BuffMap::iterator BuffMap_Itr;

	BuffMap m_ItemBuff; // +0x568

public:
	// JP 983: pedido e resultado do bônus de conjunto (SetEffectTable.iff). SetRangeQuery983 009d9af0 grava o pedido,
	// CalcSetEffect983 00a18a80 percorre m_SetEffectTableMap983 e deixa o registro escolhido em m_pSetEffect983.
	const unsigned long* m_pSetParts983; // +0x578: 24 tids de peça (009d9b80)
	const unsigned long* m_pSetAuxParts983; // +0x57c: 5 tids de peça auxiliar (009d9b30)
	unsigned long m_pad983_580; // +0x580 (zerado no construtor; o pedido não grava)
	unsigned long m_setClub983; // +0x584 (009d9bd0)
	unsigned long m_setCaddie983; // +0x588 (009d9bf0)
	unsigned long m_setMascot983; // +0x58c (009d9c10)
	IFF983::sSetEffectTable* m_pSetEffect983; // +0x590 (não é zerado no construtor)

	void SetRangeQuery983(const unsigned long* pParts, const unsigned long* pAuxParts, unsigned long club,
		unsigned long caddie, unsigned long mascot); // 009d9af0
	short GetSetRange983(); // 009d9a70
	void CalcSetEffect983(); // 00a18a80
	bool HasSetPiece983(unsigned long tid); // 009e9bf0
	bool HasSetPart983(unsigned long tid); // 009d9b80
	bool HasSetAuxPart983(unsigned long tid); // 009d9b30
	bool IsSetClub983(unsigned long tid); // 009d9bd0
	bool IsSetCaddie983(unsigned long tid); // 009d9bf0
	bool IsSetMascot983(unsigned long tid); // 009d9c10

private:

	void MakeCharMap(char* Buf, int len);
	void MakePartMap(char* Buf, int len);
	void MakeClubMap(char* Buf, int len);
	void MakeClubSetMap(char* Buf, int len);
	void MakeBallMap(char* Buf, int len);
	void MakeItemMap(char* Buf, int len);
	void MakeFurnitureMap(char* Buf, int len);
	void MakeCaddieMap(char* Buf, int len);
	void MakeCadItemMap(char* Buf, int len);
	void MakeSetItemMap(char* Buf, int len);
	void MakeCourseMap(char* Buf, int len);
	void MakeMatchMap(char* Buf, int len);
	void MakeTitleMap(char* Buf, int len);
	void MakeEnchantMap(char* Buf, int len);
	void MakeSkinMap(char* Buf, int len);
	void MakeHairStyleMap(char* Buf, int len);
	void MakeChildItemMap(char* Buf, int len);
	void MakeMascotMap(char* Buf, int len);
	void MakeDescMap(char* Buf, int len);
	void MakeAuxPartMap(char* Buf, int len);
	void MakeQuestDropMap(char* Buf, int len);
	void MakeQuestMap(char* Buf, int len);
	void MakeCardMap(char* Buf, int len);
	void MakeCadieMagicBoxMap(char* Buf, int len);
	void MakeRandomRecycleMap(char* Buf, int len);
	void MakeFurnitureAbilityList(char* Buf, int len);
	void MakeTikiRecipeMap(char* Buf, int len);
	void MakeTikiPointTableMap(char* Buf, int len);
	void MakeTikiSpecialTableMap(char* Buf, int len);
	void MakeRandomBoxMap(char* Buf, int len);
	void MakeNonVisibleItemMap(char* Buf, int len);
	void MakeSubscriptionItemMap(char* Buf, int len);
	void MakeCutinInforMationTableMap(char* Buf, int len);
	void MakeSpecialPrizeItemMap(char* Buf, int len);
	// JP 983: leitores das tabelas novas
	void MakeAchievementMap983(char* Buf, int len); // 00a2aed0
	void MakeCounterItemMap983(char* Buf, int len); // 00a2a9e0
	void MakeQuestStuffMap983(char* Buf, int len); // 00a2aa90
	void MakeQuestItemMap983(char* Buf, int len); // 00a2ab60
	void MakeErrorCodeInfoMap983(char* Buf, int len); // 00a2a930
	void MakeArtifactManaInfoMap983(char* Buf, int len); // 00a2b770
	void MakeGrandPrixDataMap983(char* Buf, int len); // 00a2b820
	void MakeAbilityMap983(char* Buf, int len); // 00a2afa0
	void MakeSetEffectTableMap983(char* Buf, int len); // 00a2b050
	void MakeHoleCupDropItemMap983(char* Buf, int len); // 00a2b8d0
	void MakeShopLimitItemMap983(char* Buf, int len); // 00a2a360
	void MakeLevelUpPrizeItemMap983(char* Buf, int len); // 00a2a880
	void MakeScratchShopConfig983(char* Buf, int len); // 009d9e60
	void MakeWorkShopChangeProb983(char* Buf, int len); // 009d9f10
	void MakeAttendancePrizeConfig983(char* Buf, int len); // 009d9f70
	void MakeAlchemyConfig983(char* Buf, int len); // 009da020
	void MakePointShopMap983(char* Buf, int len); // 00a2f610
	void MakeScratchItem983(char* Buf, int len); // 00a2a550
	void MakeBongdariShopNormalItem983(char* Buf, int len); // 00a31fe0
	void MakeBongdariShopRareItem983(char* Buf, int len); // 00a32180
	void MakeBroadCastNotices983(char* Buf, int len); // 00a36150
	void MakeTwinsItemMap983(char* Buf, int len); // 00a39aa0
	void MakeScratchShopItem983(char* Buf, int len); // 00a32230
	void MakeWorkShopLevelUpProb983(char* Buf, int len); // 00a322d0
	void MakeWorkShopLevelUpLimit983(char* Buf, int len); // 00a323e0
	void MakeWorkShopRankUpExp983(char* Buf, int len); // 00a324e0
	void MakeWorkShopChangeItem983(char* Buf, int len); // 00a32590
	void MakeAttendancePrizeItem983(char* Buf, int len); // 00a32690
	void MakeGrandPrixSpecialHoleMap983(char* Buf, int len); // 00a3c200
	void MakeGrandPrixConditionEquipMap983(char* Buf, int len); // 00a3b470
	void MakeGrandPrixRankRewardMap983(char* Buf, int len); // 00a3b6d0
	void MakeGrandPrixAIMap983(char* Buf, int len); // 00a380b0
	void MakeAddonPartMap983(char* Buf, int len); // 00a3bef0
	void MakeMemorialShopRareItem983(char* Buf, int len); // 00a32b50
	void MakeMemorialShopCoinItem983(char* Buf, int len); // 00a32850
	void MakeMemorialShopNormalItem983(char* Buf, int len); // 00a32a10
	void MakeCharacterMastery983(char* Buf, int len); // 00a32d70
	void MakeCaddieVoiceTable983(char* Buf, int len); // 00a32e20
	std::string GetPackName983(const char* ext); // 00a2f1a0
	_private::sRandomBox* findRandomBox(unsigned long boxTypeId);
	void CheckInvalidItemName();

public:
	IFF_STRUCT::sChar* FindChar(unsigned long tid);
	IFF_STRUCT::sPart* FindPart(unsigned long tid);
	IFF983::sCourseBonus* FindCourseBonus983(unsigned char course, unsigned char bSecond); // 00a07150
	std::multimap<unsigned int, IFF983::sAddonPart>* FindAddonPartGroup983(unsigned long tid); // 00a193b0 (NULL se o tid nao tem grupo)
	IFF_STRUCT::sClub* FindClub(unsigned long tid);
	IFF_STRUCT::sClubSet* FindClubSet(unsigned long tid);
	IFF_STRUCT::sBall* FindBall(unsigned long tid);
	bool IsBallFlag983(unsigned long tid); // JP 983: 00a194f0 (bit 0 de sBall::Flag983)
	IFF_STRUCT::sItem* FindItem(unsigned long tid);
	IFF_STRUCT::sCaddie* FindCaddie(unsigned long tid);
	IFF_STRUCT::sCadItem* FindCadItem(unsigned long tid);
	IFF_STRUCT::sSetItem* FindSetItem(unsigned long tid);
	IFF_STRUCT::sSetItem* FindSetItemAtConstruct(unsigned long tid);
	IFF_STRUCT::sCourse* FindCourse(unsigned long tid);
	IFF_STRUCT::sMatch* FindMatch(unsigned long tid);
	IFF_STRUCT::sTitle* FindTitle(unsigned long tid);
	IFF_STRUCT::sEnchant* FindEnchant(unsigned long tid);
	IFF_STRUCT::sSkin* FindSkin(unsigned long tid);
	IFF_STRUCT::sRandomBox* FindRandomBox(unsigned long tid);
	IFF_STRUCT::sHairStyle* FindHairStyle(unsigned long tid);
	IFF_STRUCT::sHairStyle* FindHairStyle(unsigned char charID,
		unsigned char hairID);
	IFF_STRUCT::sChildItem* FindChildItem(unsigned long tid, int iIndex);
	int FindChildItemNumber(unsigned long tid);
	IFF_STRUCT::sMascot* FindMascot(unsigned long tid);
	IFF_STRUCT::sAuxPart* FindAuxPart(unsigned long tid);
	IFF_STRUCT::sQuestDrop* FindQuestDrop(unsigned long tid);
	IFF_STRUCT::sQuest* FindQuest(unsigned long tid);
	IFF_ITEM_COMMON* FindCommonItem(unsigned long tid);
	IFF_STRUCT::sCard* FindCard(unsigned long tid);
	IFF_STRUCT::sFurniture* FindFurniture(unsigned long tid);
	IFF_STRUCT::sOfflineShop* FindOfflineShop(unsigned long tid);
	IFF_STRUCT::sDesc* FindDesc(unsigned long tid);
	// JP 983: buscas nas tabelas novas
	IFF983::sAchievement* FindAchievement983(unsigned long tid); // 00a17750
	IFF983::sCounterItem* FindCounterItem983(unsigned long tid); // 00a17660
	IFF983::sQuestStuff* FindQuestStuff983(unsigned long tid); // 00a176b0
	IFF983::sQuestItem* FindQuestItem983(unsigned long tid); // 00a17700
	IFF983::sErrorCodeInfo* FindErrorCodeInfo983(unsigned long tid); // 00a17610
	IFF983::sGrandPrixData* FindGrandPrixData983(unsigned long tid); // 00a17b00
	std::vector<IFF983::sGrandPrixSpecialHole>* FindGrandPrixSpecialHole983(unsigned long tid); // 00a17b50 (s93)
	std::vector<IFF983::sGrandPrixConditionEquip>* FindGrandPrixConditionEquip983(unsigned long tid); // 00a17b90 (pedido B65)
	std::vector<IFF983::sGrandPrixRankReward>* FindGrandPrixRankReward983(unsigned long tid); // 00a17bd0 (pedido B66)
	bool HasGrandPrixSpecialHole983(unsigned long tid); // 00a17c10 (pedido B66): Grand Prix de mapa sorteado
	IFF983::sAbility* FindAbility983(unsigned long tid); // 00a177a0
	IFF983::sSetEffectTable* FindSetEffectTable983(unsigned long tid); // 00a177f0
	IFF983::sShopLimitItem* FindShopLimitItem983(unsigned long tid); // 00a17520
	IFF983::sLevelUpPrizeItem* FindLevelUpPrizeItem983(unsigned char level); // 00a175c0
	IFF983::sPointShop* FindPointShop983(unsigned long tid); // 00a17570
	IFF983::sGrandPrixAIOptionalData* FindGrandPrixAI983(unsigned long tid); // 00a17cb0
	const char* FindArtifactDesc983(unsigned long tid); // 00a19320: texto em +0x0c do registro do ArtifactManaInfo, ou NULL (nome da frente B)
	int GetArtifactValue983(unsigned long tid); // 00a19360: dword em +0x90 do mesmo registro, ou -1 (nome meu)
	IFF_STRUCT::S5::sCutinInformation* FindCutinInformation(unsigned long tid);
	bool GetSetItemElem(IFF_STRUCT::sSetItem* pSetItem, int index,
		unsigned long& typeId, unsigned long& count, unsigned long& type);

	bool GetRecycleRandomMixOutItems(unsigned long randSeqNum,
		std::list<const IFF_STRUCT::sRandomRecycle*>& ltMixOutList);
	unsigned int GetItemPrice(unsigned long tid);
	int CheckStockType(unsigned long typeId, unsigned int stock);
	unsigned int GetItemSalePrice(unsigned long in_dwTid, int in_iCount,
		unsigned long in_dwDayCount);
	unsigned int GetChildItemPrice(unsigned long tid, int iIndex);
	bool IsCashItem(unsigned long tid);
	bool IsCashChildItem(unsigned long tid, int iIndex);
	bool GetWorkShopRankUpExp983(unsigned long typeId, int rank, int* pExp); // 00a07030 (pedido B81)
	bool SumWorkShopRankUpExp983(unsigned long typeId, int first, int last, int* pSum); // 00a07070
	bool IsTimeLimit(unsigned long tid);
	const char* GetItemName(unsigned long tid);
	bool HasSalePeriod(unsigned long tid, _SYSTEMTIME** pSaleS,
		_SYSTEMTIME** pSaleE);
	bool IsInSalePeriod(unsigned long tid, _SYSTEMTIME& sysTime);
	// 983 009d8a80: item restrito por nivel (InStock==3): Level > nivel+1 (ou, com IsUnderLvl, Level < nivel)
	bool IsLevelLocked(IFF_ITEM_COMMON* pItem, unsigned long* pLevel);
	// 983 00a1f570: a venda (InStock != 0/5/6 e no periodo) ou item com DateFlag == 2
	bool IsSellableItem(IFF_ITEM_COMMON* pItem, _SYSTEMTIME& sysTime);
	bool IsBetween(_SYSTEMTIME* pStart, _SYSTEMTIME* pEnd,
		_SYSTEMTIME& sysTime);
	int CompareSystemTime(_SYSTEMTIME& tm0, _SYSTEMTIME& tm1);
	// 983 009e99b0: receita de reciclagem dentro do periodo (corpo em recycledlg.cpp)
	bool IsInRecyclePeriod983(short index, _SYSTEMTIME& now);
	// 983 00a23710: parte que nao conta para o set (gemeo nao; set 0x24 por elemento; resto por tipo; corpo em recycledlg.cpp)
	bool IsPartExcluded983(unsigned long tid, bool gift, bool b3);
	// 983 00a18c20: item restrito por tipo (parte/taco IsSalable 1 ou 3, caddie alugado, set 5/6...; corpo em recycledlg.cpp)
	bool IsRestrictedItem983(unsigned long tid, bool gift, bool b3);
	// 983 00a18d20: set 0x24 com todos os elementos restritos (corpo em recycledlg.cpp)
	bool IsRestrictedSet983(unsigned long tid);
	int GetCouponKind(unsigned long dwTid);
	bool IsDisableDuplicateItem(unsigned long dwTid);
	unsigned int IsValidBuyItem(unsigned long in_dwItemType);
	bool IsValidBuyItemCount(unsigned long in_dwItemType, int in_iItemCount);
	bool IsDisplayItemNumber(unsigned long dwTid);
	unsigned long GetIndexAbilityItem(unsigned long dwTid);
	unsigned long GetMascotBonusPang(unsigned long dwTid, bool bBestShot);
	bool GetDefCombo(unsigned long tidChar, unsigned long* atidDefCombo);
	int GetNumItems(PANGYA_ITEM_GROUP pig);
	int GetCourseNum();
	bool isTrade(unsigned long typeId);
	bool IsCanOverlapped(unsigned long dwTID, unsigned char btCheckType,
		unsigned char exceptGroup);
	bool IsPartAttachCard(unsigned long dwCardTid, unsigned long dwPartTid,
		int iSlotNum);
	COUNTING_TYPE GetItemCountingType(unsigned long in_Tid,
		unsigned char in_itemType);
	IFF_STRUCT::sSpecialPrizeItem* FindSPItem(unsigned long typeId,
		eSPAVILITYTYPE type);
	bool GetCardSlotNum(unsigned long in_TID,
		unsigned short& out_LimitCharacterSlot,
		unsigned short& out_LimitCaddieSlot,
		unsigned short& out_LimitNpcSlot); // 00a19200
	bool IsEquipCard(unsigned long in_CardTID);
	bool IsItemBuff(unsigned long dwTID);
	bool IsCompatibleBuff(unsigned long dwBuffTID, unsigned long dwWithTID);
	bool GetBuffProperty(unsigned long dwTID, specialcardavility& eType,
		int& Value, int& Duration, int& Accum);
	unsigned char GetItemLevel(eLvlType eType, const unsigned long* patidParts,
		const unsigned long* patidAuxParts);
	unsigned char GetCharCapacity(eLvlType eType, unsigned long tidChar,
		const unsigned long* patidParts, const unsigned long* patidAuxParts,
		unsigned char level);
	unsigned char GetCharLevel(eLvlType eType, unsigned long tidChar,
		const char* pCharPCL);
	// JP 983: as tres contas recebem UMA struct (sCapacityParam983, +0x30 = ptr extra da maestria); as de cima ficam como
	// adaptadores para os chamadores antigos.
	unsigned char GetCapacity(const sCapacityParam983* p); // 00a1fb40
	unsigned char GetLevel(const sCapacityParam983* p); // 00a23620
	status_t GetLevelWithPenalty(const sCapacityParam983* p); // 00a2b570
	unsigned char GetCharLevelCore983(unsigned long type, IFF_STRUCT::sChar* pChar, const char* pStat,
		unsigned char upgrade, const unsigned long* pParts, const unsigned long* pAuxParts, int extra); // 00a1fd60
	unsigned char GetCapacity(eLvlType type, unsigned long charTypeId,
		const char* pStat, const unsigned long* pParts,
		unsigned long clubTypeId, const short* pClubStat,
		unsigned long caddieTypeId, unsigned char upgrade);
	unsigned char GetLevel(eLvlType type, unsigned long charTypeId,
		const char* pStat, const unsigned long* pParts,
		const unsigned long* pAuxParts, unsigned long clubTypeId,
		const short* pClubStat, unsigned long caddieTypeId,
		unsigned char upgrade);
	status_t GetLevelWithPenalty(eLvlType type, unsigned long charTypeId,
		const char* pStat, const unsigned long* pParts,
		const unsigned long* pAuxParts, unsigned long clubTypeId,
		const short* pClubStat, unsigned long caddieTypeId,
		unsigned char upgrade);
	const char* GetIconName(unsigned long typeId);
	bool IsUnlimitAztec(unsigned long typeId);
	bool IsBasicAztecFamily(unsigned long tid);
	const char* ChangePartsPosmask(unsigned long dwTypeID,
		unsigned long dwPosMask);
	const char* ChangePartsTexName(unsigned long dwTypeID,
		unsigned long dwTexType, unsigned long dwTexNum, const char* pTexName);
	const char* ChangePartsPetName(unsigned long dwTypeID,
		const char* pPetName);
	IFF_STRUCT::sRandomBox* GetRandomBoxInfo(unsigned long boxTypeId,
		unsigned long typeId, unsigned long itemIndex);
	bool GetEnumRandomBoxStuff(unsigned long boxTypeId,
		std::vector<IFF_STRUCT::sRandomBox>& out);
	bool GetEnumRandomBoxItem(unsigned long boxTypeId,
		std::vector<std::pair<unsigned long, IFF_STRUCT::sRandomBox> >& out);
	bool GetEnumRandomBoxCheckItem(unsigned long boxTypeId,
		unsigned long typeId, unsigned long itemIndex,
		std::vector<IFF_STRUCT::sRandomBox>& out);
	bool IsNonVisibleItem(unsigned long typeId);
	bool IsSubScriptionCoupon(unsigned long typeId); // 00a19170
	bool IsTwinsItem983(unsigned long typeId); // 00a200e0
	bool IsTwinsGroupMember983(unsigned long typeId, unsigned long typeId2); // 00a20120

};

// Adaptadores (nossos, sem par no 983) para os chamadores antigos (familymain, lobbymain, realmyroommainui, rb_ de
// ghostmain/playerdlg/golfdoc): campos soltos -> struct do 983; sem mascote nem maestria, slots ganhos do taco = 0.
// Inline no header: ficam nos chamadores, nao no itemmanager.obj.
#include "../../include/jp/clubsetstat.h"

inline void FillCapacityParam983(sCapacityParam983* p, eLvlType type, unsigned long charTypeId, const char* pStat,
	const unsigned long* pParts, const unsigned long* pAuxParts, unsigned long clubTypeId, const short* pClubStat,
	unsigned long caddieTypeId, unsigned char upgrade)
{
	static const short zeroSlotUp[5] = { 0, 0, 0, 0, 0 };
	memset(p, 0, 0x34);
	p->type = type;
	p->tidChar = charTypeId;
	p->m_8 = (unsigned long)pStat;
	p->tidParts = pParts;
	p->tidAuxParts = pAuxParts;
	p->tidClub = clubTypeId;
	p->clubAttrUp = pClubStat;
	p->clubSlotUp = zeroSlotUp;
	p->tidCaddie = caddieTypeId;
	p->level = upgrade;
}

inline unsigned char CItemManager::GetCapacity(eLvlType type, unsigned long charTypeId, const char* pStat,
	const unsigned long* pParts, unsigned long clubTypeId, const short* pClubStat, unsigned long caddieTypeId,
	unsigned char upgrade)
{
	char buf[0x34];
	sCapacityParam983* p = (sCapacityParam983*)buf;
	FillCapacityParam983(p, type, charTypeId, pStat, pParts, NULL, clubTypeId, pClubStat, caddieTypeId, upgrade);
	return GetCapacity(p);
}

inline unsigned char CItemManager::GetLevel(eLvlType type, unsigned long charTypeId, const char* pStat,
	const unsigned long* pParts, const unsigned long* pAuxParts, unsigned long clubTypeId, const short* pClubStat,
	unsigned long caddieTypeId, unsigned char upgrade)
{
	char buf[0x34];
	sCapacityParam983* p = (sCapacityParam983*)buf;
	FillCapacityParam983(p, type, charTypeId, pStat, pParts, pAuxParts, clubTypeId, pClubStat, caddieTypeId, upgrade);
	return GetLevel(p);
}

inline status_t CItemManager::GetLevelWithPenalty(eLvlType type, unsigned long charTypeId, const char* pStat,
	const unsigned long* pParts, const unsigned long* pAuxParts, unsigned long clubTypeId, const short* pClubStat,
	unsigned long caddieTypeId, unsigned char upgrade)
{
	char buf[0x34];
	sCapacityParam983* p = (sCapacityParam983*)buf;
	FillCapacityParam983(p, type, charTypeId, pStat, pParts, pAuxParts, clubTypeId, pClubStat, caddieTypeId, upgrade);
	return GetLevelWithPenalty(p);
}

// 983: sem funcao propria (inlined em fontinfo); noinline so para o simbolo existir no alternatename do rb_ de fontinfo
__declspec(noinline) inline unsigned long CItemManager::GetMascotBonusPang(unsigned long dwTid,
	bool bBestShot)
{
	unsigned long bonus = 0;

	if ((dwTid & 0xfc000000) == 0x40000000)
	{
		switch (dwTid & 0x3ffffff)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			bonus = bBestShot ? 10 : 0;
			break;
		case 6:
		case 0xe:
		case 0x12:
			bonus = bBestShot ? 10 : 5;
			break;
		case 4:
		case 0xd:
		case 0x10:
		case 0x13:
			bonus = bBestShot ? 15 : 5;
			break;
		case 9:
		case 0xa:
		case 0xb:
		case 0xc:
			bonus = bBestShot ? 15 : 0;
			break;
		}
	}

	return bonus;
}

