#include "minatl.h"
#include "vc10_layout.h"
#include <atltime.h>
#include <sstream>
#include <algorithm>

#include "itemmanager.h"
#include "tikimagicboxtable.h"
#include "localize.h"
#include "wlocalize.h"
#include "cardsystem.h"
#include "../projectg/md5.h"
#include "../../include/jp/clubsetstat.h"
#include "../../include/jp/statushelper.h"

struct sRecycleItem
{
	unsigned long dwTid;
	unsigned long dwItemid;
	unsigned short iNum;
	unsigned char byLevel;
	unsigned char byFlag983; // 983: +0x0b, byte baixo de sCadieMagicBox+0x14 (00a22d80)
	short iChar;
	short iIndex;
	unsigned char byItemType;
	unsigned short wTime;
	_SYSTEMTIME date;
	unsigned long dwRandSeq;
	char szRandName[40];
	_SYSTEMTIME dateStart983; // 983: +0x50 e +0x60 (sizeof 0x70), periodo da receita no CadieMagicBox.iff
	_SYSTEMTIME dateEnd983;

	sRecycleItem() // 983: o new[] do leitor 00a22d80 zera cada elemento
	{
		memset(this, 0, sizeof(*this));
	}
};
C_ASSERT(sizeof(sRecycleItem) == 0x70);
C_ASSERT(offsetof(sRecycleItem, szRandName) == 0x28);

extern int g_NumIndexRecycleItem;
extern int g_NumLowRecycleItem;
extern int g_NumMidRecycleItem;
extern int g_NumHighRecycleItem;
extern int g_NumSpecialRecycleItem;
extern int g_NumEventRecycleItem;
extern sRecycleItem** g_ArrayTidMixItem;
extern sRecycleItem* g_ArrayRecycleItem;

namespace S5
{
	bool ISCutinItem(unsigned long typeId);
}

struct IFF_FILE_HEADER
{
	unsigned short nRecords;
	unsigned int Version;
};

void DevFinalStockPrice(IFF_ITEM_COMMON& c)
{
	if (c.Final && c.Price >= 1000000)
	{
		if (c.InStock != 0 && c.InStock != 4)
		{
			char szMsg[128] = { 0 };
			sprintf(szMsg,
				"\xbb\xf3\xc1\xa1 \xc1\xf8\xbf\xad\xbb\xf3\xc7\xb0 "
				"\xb0\xa1\xb0\xdd\xc0\xcc\xbb\xf3(%s)",
				c.Name);
			MessageBox(NULL, szMsg, "PangYa", MB_OK);
			LogOut(0, szMsg);

			c.IsSalable = 0;
			c.InStock = 0;
		}
	}
}

void DevRentalPreventSale(IFF_STRUCT::sPart& part)
{
	if (part.c.Final && part.c.IsSalable != 0)
	{
		if (part.RentalPrice > 0)
		{
			char szMsg[128] = { 0 };
			sprintf(szMsg,
				"\xb0\xc5\xb7\xa1\xb0\xa1 \xb5\xc7\xb4\xc2 \xb7\xa3\xc5\xbb "
				"\xc6\xc4\xc3\xf7 \xb9\xdf\xb0\xdf(%s)",
				part.c.Name);
			MessageBox(NULL, szMsg, "PangYa", MB_OK);
			LogOut(0, szMsg);

			part.RentalPrice = 0;
		}
	}
}

// JP 983 (00a3fc00): sem seção crítica; as duas tabelas de +0x2e0/+0x2e4 são alocadas aqui.
CItemManager::CItemManager()
{
	// JP 983: cada membro no deslocamento medido (construtor 00a3fc00, destrutor 00a3edc0, leitores do Load)
	// registros das tabelas (sessao 79): tamanho do 983 e os campos que o exe le
	C_ASSERT(sizeof(IFF_ITEM_COMMON) == 0xc0);
	C_ASSERT(sizeof(IFF_STRUCT::sClubSet) == 0x104);
	C_ASSERT(offsetof(IFF_STRUCT::sClubSet, Slot) == 0xda);
	C_ASSERT(offsetof(IFF_STRUCT::sClubSet, CharacterSlot983) == 0xfa);
	C_ASSERT(offsetof(IFF_STRUCT::sClubSet, CaddieSlot983) == 0xfc);
	C_ASSERT(offsetof(IFF_STRUCT::sClubSet, nKind983) == 0x100);
	C_ASSERT(sizeof(IFF_STRUCT::sBall) == 0x330);
	C_ASSERT(offsetof(IFF_STRUCT::sBall, Flag983) == 0xc0);
	C_ASSERT(offsetof(IFF_STRUCT::sBall, Fx) == 0xf4);
	C_ASSERT(offsetof(IFF_STRUCT::sBall, FxBone) == 0x20c);
	C_ASSERT(offsetof(IFF_STRUCT::sBall, COM) == 0x324);
	C_ASSERT(sizeof(IFF_STRUCT::sCourse) == 0x1d0);
	C_ASSERT(offsetof(IFF_STRUCT::sCourse, TexProp) == 0x111);
	C_ASSERT(offsetof(IFF_STRUCT::sCourse, SkyFx) == 0x140);
	C_ASSERT(offsetof(IFF_STRUCT::sCourse, HolePar983) == 0x198);
	C_ASSERT(sizeof(IFF_STRUCT::sMascot) == 0x130);
	C_ASSERT(offsetof(IFF_STRUCT::sMascot, DriveUp) == 0x11a);
	C_ASSERT(offsetof(IFF_STRUCT::sMascot, ItemSlot) == 0x124);
	C_ASSERT(offsetof(IFF_STRUCT::sMascot, MsgFee) == 0x128);
	C_ASSERT(sizeof(IFF_STRUCT::sCard) == 0x180);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, COM) == 0xea);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, SubIcon) == 0xf8);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, Volume) == 0x172);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, CardIndex) == 0x174);
	C_ASSERT(sizeof(IFF_STRUCT::sCadieMagicBox) == 0x8c);
	C_ASSERT(offsetof(IFF_STRUCT::sCadieMagicBox, uiOutput) == 0x18);
	C_ASSERT(offsetof(IFF_STRUCT::sCadieMagicBox, uiElem) == 0x20);
	C_ASSERT(offsetof(IFF_STRUCT::sCadieMagicBox, uiElemCount) == 0x30);
	C_ASSERT(offsetof(IFF_STRUCT::sCadieMagicBox, uiRandSeq) == 0x40);
	C_ASSERT(offsetof(IFF_STRUCT::sCadieMagicBox, szRandName) == 0x44);
	C_ASSERT(offsetof(IFF_STRUCT::sCadieMagicBox, dateStart983) == 0x6c);
	C_ASSERT(offsetof(sRecycleItem, byFlag983) == 0x0b);
	C_ASSERT(offsetof(sRecycleItem, dwRandSeq) == 0x24);
	C_ASSERT(offsetof(sRecycleItem, dateStart983) == 0x50);
	C_ASSERT(offsetof(CItemManager, m_strLoaded983) == 0x004);
	C_ASSERT_VC10(offsetof(CItemManager, m_CharMap) == 0x020);
	C_ASSERT_VC10(offsetof(CItemManager, m_PartMap) == 0x030);
	C_ASSERT_VC10(offsetof(CItemManager, m_ClubMap) == 0x040);
	C_ASSERT_VC10(offsetof(CItemManager, m_ClubSetMap) == 0x050);
	C_ASSERT_VC10(offsetof(CItemManager, m_BallMap) == 0x060);
	C_ASSERT_VC10(offsetof(CItemManager, m_ItemMap) == 0x070);
	C_ASSERT_VC10(offsetof(CItemManager, m_CaddieMap) == 0x080);
	C_ASSERT_VC10(offsetof(CItemManager, m_CadItemMap) == 0x090);
	C_ASSERT_VC10(offsetof(CItemManager, m_SetItemMap) == 0x0a0);
	C_ASSERT_VC10(offsetof(CItemManager, m_CourseMap) == 0x0b0);
	C_ASSERT_VC10(offsetof(CItemManager, m_MatchMap) == 0x0c0);
	C_ASSERT_VC10(offsetof(CItemManager, m_TitleMap) == 0x0d0);
	C_ASSERT_VC10(offsetof(CItemManager, m_EnchantMap) == 0x0e0);
	C_ASSERT_VC10(offsetof(CItemManager, m_DescMap) == 0x0f0);
	C_ASSERT_VC10(offsetof(CItemManager, m_SkinMap) == 0x100);
	C_ASSERT_VC10(offsetof(CItemManager, m_HairStyleMap) == 0x110);
	C_ASSERT_VC10(offsetof(CItemManager, m_ChildItemMap) == 0x120);
	C_ASSERT_VC10(offsetof(CItemManager, m_MascotMap) == 0x130);
	C_ASSERT_VC10(offsetof(CItemManager, m_AuxPartMap) == 0x140);
	C_ASSERT_VC10(offsetof(CItemManager, m_QuestDropMap) == 0x150);
	C_ASSERT_VC10(offsetof(CItemManager, m_QuestMap) == 0x160);
	C_ASSERT_VC10(offsetof(CItemManager, m_FurnitureMap) == 0x170);
	C_ASSERT_VC10(offsetof(CItemManager, m_AchievementMap983) == 0x180);
	C_ASSERT_VC10(offsetof(CItemManager, m_AbilityMap983) == 0x190);
	C_ASSERT_VC10(offsetof(CItemManager, m_SetEffectTableMap983) == 0x1a0);
	C_ASSERT_VC10(offsetof(CItemManager, m_CardMap) == 0x1b0);
	C_ASSERT_VC10(offsetof(CItemManager, m_RandomRecycleMaps) == 0x1c0);
	C_ASSERT_VC10(offsetof(CItemManager, m_CadieMagicBox) == 0x1d0);
	C_ASSERT_VC10(offsetof(CItemManager, m_FurnitureAbility) == 0x1dc);
	C_ASSERT_VC10(offsetof(CItemManager, m_TikiPointTable) == 0x1e8);
	C_ASSERT_VC10(offsetof(CItemManager, m_TikiSpecialRecipe) == 0x1f4);
	C_ASSERT_VC10(offsetof(CItemManager, m_PointShopMap983) == 0x200);
	C_ASSERT_VC10(offsetof(CItemManager, m_ShopLimitItemMap983) == 0x210);
	C_ASSERT_VC10(offsetof(CItemManager, m_RandomBoxMap) == 0x220);
	C_ASSERT_VC10(offsetof(CItemManager, m_NonVisibleItemMap) == 0x230);
	C_ASSERT_VC10(offsetof(CItemManager, m_SubscriptionItemMap) == 0x240);
	C_ASSERT_VC10(offsetof(CItemManager, m_ScratchItem983) == 0x250);
	C_ASSERT_VC10(offsetof(CItemManager, m_BongdariShopNormalItem983) == 0x260);
	C_ASSERT_VC10(offsetof(CItemManager, m_BongdariShopRareItem983) == 0x270);
	C_ASSERT_VC10(offsetof(CItemManager, m_CutinInformationMap) == 0x280);
	C_ASSERT_VC10(offsetof(CItemManager, m_SPItemMap) == 0x290);
	C_ASSERT_VC10(offsetof(CItemManager, m_pBroadCastNotices983) == 0x2e0);
	C_ASSERT_VC10(offsetof(CItemManager, m_pTwinsItemMap983) == 0x2e4);
	C_ASSERT_VC10(offsetof(CItemManager, m_CupInConditionMap983) == 0x2e8);
	C_ASSERT_VC10(offsetof(CItemManager, m_LevelUpPrizeItemMap983) == 0x2f8);
	C_ASSERT_VC10(offsetof(CItemManager, m_ErrorCodeInfoMap983) == 0x308);
	C_ASSERT_VC10(offsetof(CItemManager, m_CounterItemMap983) == 0x318);
	C_ASSERT_VC10(offsetof(CItemManager, m_QuestStuffMap983) == 0x328);
	C_ASSERT_VC10(offsetof(CItemManager, m_QuestItemMap983) == 0x338);
	C_ASSERT_VC10(offsetof(CItemManager, m_ArtifactManaInfoMap983) == 0x348);
	C_ASSERT_VC10(offsetof(CItemManager, m_ScratchShopConfig983) == 0x358);
	C_ASSERT_VC10(offsetof(CItemManager, m_ScratchShopItem983) == 0x36c);
	C_ASSERT_VC10(offsetof(CItemManager, m_WorkShopLevelUpProb983) == 0x37c);
	C_ASSERT_VC10(offsetof(CItemManager, m_WorkShopLevelUpLimit983) == 0x38c);
	C_ASSERT_VC10(offsetof(CItemManager, m_WorkShopRankUpExp983) == 0x39c);
	C_ASSERT_VC10(offsetof(CItemManager, m_WorkShopChangeProb983) == 0x3ac);
	C_ASSERT_VC10(offsetof(CItemManager, m_WorkShopChangeItem983) == 0x3c8);
	C_ASSERT_VC10(offsetof(CItemManager, m_AttendancePrizeConfig983) == 0x3d8);
	C_ASSERT_VC10(offsetof(CItemManager, m_AttendancePrizeItem983) == 0x464);
	C_ASSERT_VC10(offsetof(CItemManager, m_AddonPartMap983) == 0x474);
	C_ASSERT_VC10(offsetof(CItemManager, m_HoleCupDropItemMap983) == 0x484);
	C_ASSERT_VC10(offsetof(CItemManager, m_MemorialShopCoinItem983) == 0x494);
	C_ASSERT_VC10(offsetof(CItemManager, m_MemorialShopNormalItem983) == 0x4a4);
	C_ASSERT_VC10(offsetof(CItemManager, m_MemorialShopRareItem983) == 0x4b4);
	C_ASSERT_VC10(offsetof(CItemManager, m_GrandPrixAIMap983) == 0x4c4);
	C_ASSERT_VC10(offsetof(CItemManager, m_GrandPrixAIMap983_4d4) == 0x4d4);
	C_ASSERT_VC10(offsetof(CItemManager, m_GrandPrixAIMap983_4e4) == 0x4e4);
	C_ASSERT_VC10(offsetof(CItemManager, m_GrandPrixDataMap983) == 0x4f4);
	C_ASSERT_VC10(offsetof(CItemManager, m_GrandPrixSpecialHoleMap983) == 0x504);
	C_ASSERT_VC10(offsetof(CItemManager, m_GrandPrixConditionEquipMap983) == 0x514);
	C_ASSERT_VC10(offsetof(CItemManager, m_GrandPrixRankRewardMap983) == 0x524);
	C_ASSERT_VC10(offsetof(CItemManager, m_AlchemyConfig983) == 0x534);
	C_ASSERT_VC10(offsetof(CItemManager, m_CharacterMastery983) == 0x538);
	C_ASSERT_VC10(offsetof(CItemManager, m_CourseBonus983) == 0x548);
	C_ASSERT_VC10(offsetof(CItemManager, m_CaddieVoice983) == 0x558);
	C_ASSERT_VC10(offsetof(CItemManager, m_ItemBuff) == 0x568);
	C_ASSERT_VC10(offsetof(CItemManager, m_pSetParts983) == 0x578);
	C_ASSERT_VC10(offsetof(CItemManager, m_pSetAuxParts983) == 0x57c);
	C_ASSERT_VC10(offsetof(CItemManager, m_pad983_580) == 0x580);
	C_ASSERT_VC10(offsetof(CItemManager, m_setClub983) == 0x584);
	C_ASSERT_VC10(offsetof(CItemManager, m_setCaddie983) == 0x588);
	C_ASSERT_VC10(offsetof(CItemManager, m_setMascot983) == 0x58c);
	C_ASSERT_VC10(offsetof(CItemManager, m_pSetEffect983) == 0x590);
	C_ASSERT_VC10(sizeof(CItemManager) == 0x594);
	C_ASSERT_VC10(sizeof(_private::sRandomBox) == 0x114);
	C_ASSERT(sizeof(IFF983::sAchievement) == 0x600);
	C_ASSERT(sizeof(IFF983::sCounterItem) == 0x118);
	C_ASSERT(sizeof(IFF983::sQuestStuff) == 0x10c);
	C_ASSERT(sizeof(IFF983::sQuestItem) == 0x110);
	C_ASSERT(sizeof(IFF983::sErrorCodeInfo) == 0x110);
	C_ASSERT(sizeof(IFF983::sArtifactManaInfo) == 0x98);
	C_ASSERT(sizeof(IFF983::sGrandPrixData) == 0x330);
	C_ASSERT(sizeof(IFF983::sScratchShopConfig) == 0x14);
	C_ASSERT(sizeof(IFF983::sClubSetWorkShopChangeProb) == 0x1c);
	C_ASSERT(sizeof(IFF983::sAttendancePrizeConfig) == 0x8c);
	C_ASSERT(sizeof(IFF983::sPointShop) == 0x14);
	C_ASSERT(offsetof(IFF983::sScratchItem, TypeId) == 0x0c);
	C_ASSERT(offsetof(IFF983::sBroadCastNotice, time220) == 0x220);
	C_ASSERT(offsetof(IFF983::sTwinsItem, tid) == 0x08);
	C_ASSERT(offsetof(IFF983::sGrandPrixAIOptionalData, id) == 0x04);
	C_ASSERT(offsetof(IFF983::sGrandPrixAIOptionalData, key34) == 0x34);
	C_ASSERT(offsetof(IFF983::sGrandPrixAIOptionalData, key54) == 0x54);
	C_ASSERT_VC10(sizeof(std::map<unsigned char, std::vector<unsigned short> >) == 0x10);
	C_ASSERT(sizeof(IFF983::sScratchItem) == 0x30);
	C_ASSERT(sizeof(IFF983::sBongdariShopNormalItem) == 0x10);
	C_ASSERT(sizeof(IFF983::sBongdariShopRareItem) == 0x30);
	C_ASSERT(sizeof(IFF983::sBroadCastNotice) == 0x230);
	C_ASSERT(sizeof(IFF983::sTwinsItem) == 0x1c);
	C_ASSERT(sizeof(IFF983::sCupInCondition) == 0xa0);
	C_ASSERT(offsetof(IFF983::sCupInCondition, Name) == 0x18 && offsetof(IFF983::sCupInCondition, flag9c) == 0x9c);
	C_ASSERT(sizeof(IFF983::sCourseBonus) == 0x38 && offsetof(IFF983::sCourseBonus, course) == 0x30);
	C_ASSERT(offsetof(IFF983::sLevelUpPrizeItem, tidPrize) == 0x24 && offsetof(IFF983::sLevelUpPrizeItem, nPrize) == 0x2c);
	C_ASSERT(offsetof(IFF983::sLevelUpPrizeItem, szPrize) == 0xc0 && offsetof(IFF983::sLevelUpPrizeItem, szNextPrize) == 0x141);
	C_ASSERT(offsetof(IFF983::sBroadCastNotice, periodUnit) == 0x0c && offsetof(IFF983::sBroadCastNotice, time220) == 0x220);
	C_ASSERT(offsetof(IFF_ITEM_COMMON, TimeFlag) == 0x82);
	C_ASSERT(offsetof(IFF_STRUCT::sCourse, CourseBonus983) == 0x168 && offsetof(IFF_STRUCT::sCourse, CourseBonus2_983) == 0x180);
	C_ASSERT(sizeof(IFF983::sScratchShopItem) == 0x18);
	C_ASSERT(sizeof(IFF983::sClubSetWorkShopLevelUpProb) == 0x18);
	C_ASSERT(sizeof(IFF983::sClubSetWorkShopLevelUpLimit) == 0x14);
	C_ASSERT(sizeof(IFF983::sClubSetWorkShopRankUpExp) == 0x1c);
	C_ASSERT(sizeof(IFF983::sClubSetWorkShopChangeItem) == 0x14);
	C_ASSERT(sizeof(IFF983::sAttendancePrizeItem) == 0x14);
	C_ASSERT(sizeof(IFF983::sAddonPart) == 0x120);
	C_ASSERT(sizeof(IFF983::sMemorialShopCoinItem) == 0x40);
	C_ASSERT(sizeof(IFF983::sMemorialShopNormalItem) == 0x48);
	C_ASSERT(sizeof(IFF983::sMemorialShopRareItem) == 0x5c);
	C_ASSERT(sizeof(IFF983::sGrandPrixAIOptionalData) == 0x25c);
	C_ASSERT(sizeof(IFF983::sGrandPrixSpecialHole) == 0x14);
	C_ASSERT(sizeof(IFF983::sGrandPrixConditionEquip) == 0x210);
	C_ASSERT(sizeof(IFF983::sGrandPrixRankReward) == 0x4c);
	C_ASSERT(sizeof(IFF983::sCharacterMastery) == 0x3c);
	C_ASSERT(sizeof(IFF983::sCaddieVoice) == 0x86);
	C_ASSERT(sizeof(IFF983::sAbility) == 0x50);
	C_ASSERT(sizeof(IFF983::sSetEffectTable) == 0x4c);
	C_ASSERT(sizeof(IFF983::sLevelUpPrizeItem) == 0x1c4);
	C_ASSERT(sizeof(IFF983::sShopLimitItem) == 0x3c);
	C_ASSERT(sizeof(IFF983::sHoleCupDropItem) == 0x2c);
	C_ASSERT(offsetof(IFF983::sAbility, bFinal) == 0x4c);
	C_ASSERT(offsetof(IFF983::sSetEffectTable, range) == 0x4a);
	C_ASSERT(offsetof(IFF983::sSetEffectTable, tidPiece) == 0x1c && offsetof(IFF983::sSetEffectTable, flag) == 0x30);
	C_ASSERT(offsetof(IFF983::sSetEffectTable, value36) == 0x36 && offsetof(IFF983::sSetEffectTable, value40) == 0x40);
	C_ASSERT(PIG_PART == 2 && PIG_CLUBSET == 4 && PIG_CADDIE == 7 && PIG_MASCOT == 0x10 && PIG_AUXPART == 0x1c);
	C_ASSERT(offsetof(IFF983::sLevelUpPrizeItem, level) == 0x22);
	C_ASSERT(offsetof(IFF983::sShopLimitItem, TypeId) == 0x08);
	C_ASSERT(offsetof(IFF983::sShopLimitItem, total) == 0x0c && offsetof(IFF983::sShopLimitItem, hours) == 0x14);
	C_ASSERT(offsetof(IFF983::sShopLimitItem, saleStart) == 0x1c && offsetof(IFF983::sShopLimitItem, saleEnd) == 0x2c);

	m_strLoaded983 = "";
	memset(&m_ScratchShopConfig983, 0, sizeof(m_ScratchShopConfig983));
	m_pBroadCastNotices983 = new std::vector<IFF983::sBroadCastNoticeEntry>;
	m_pTwinsItemMap983 = new std::map<unsigned int, std::vector<unsigned long> >;
	m_AlchemyConfig983 = 0;
	m_pSetParts983 = NULL;
	m_pSetAuxParts983 = NULL;
	m_pad983_580 = 0;
	m_setClub983 = 0;
	m_setCaddie983 = 0;
	m_setMascot983 = 0;
}

// JP 983 (00a3edc0)
CItemManager::~CItemManager()
{
	Reset();

	if (m_pBroadCastNotices983)
		delete m_pBroadCastNotices983;
	m_pBroadCastNotices983 = NULL;

	if (m_pTwinsItemMap983)
		delete m_pTwinsItemMap983;
	m_pTwinsItemMap983 = NULL;
}

// sem par achado no 983 (o m_IsReloading e a trava do 645 não existem lá)
void CItemManager::Reload()
{
	Reset();
	Load();
}

// JP 983 (00a3a8d0): começa esvaziando o MD5 do pacote carregado. Lista conferida no desmonte: as 25 tabelas do 645 na
// mesma ordem, depois do m_CutinInformationMap mais 3 (PointShop, ShopLimitItem, AddonPart), o m_QuestMap e os globais.
// As outras tabelas novas são limpas pelo próprio leitor.
void CItemManager::Reset()
{
	m_strLoaded983 = "";

	m_CharMap.clear();
	m_PartMap.clear();
	m_ClubMap.clear();
	m_ClubSetMap.clear();
	m_BallMap.clear();
	m_ItemMap.clear();
	m_CaddieMap.clear();
	m_CadItemMap.clear();
	m_SetItemMap.clear();
	m_CourseMap.clear();
	m_MatchMap.clear();
	m_TitleMap.clear();
	m_EnchantMap.clear();
	m_SkinMap.clear();
	m_HairStyleMap.clear();
	m_ChildItemMap.clear();
	m_MascotMap.clear();
	m_DescMap.clear();
	m_AuxPartMap.clear();
	m_QuestDropMap.clear();
	m_FurnitureMap.clear();
	m_CardMap.clear();
	m_RandomRecycleMaps.clear();
	m_CadieMagicBox.clear();
	m_FurnitureAbility.clear();

	m_CutinInformationMap.clear();
	m_PointShopMap983.clear();
	m_ShopLimitItemMap983.clear();
	m_AddonPartMap983.clear();

	for (std::map<unsigned int, IFF_STRUCT::sQuest>::iterator itr =
			 m_QuestMap.begin();
		itr != m_QuestMap.end(); ++itr)
	{
		IFF_STRUCT::sQuest* pQuest = &(*itr).second;

		if (pQuest && pQuest->aTidItems)
		{
			delete[] pQuest->aTidItems;
			pQuest->aTidItems = NULL;
		}
	}

	m_QuestMap.clear();

	if (g_ArrayTidMixItem)
	{
		for (int i = 0; i < g_NumIndexRecycleItem; i++)
		{
			if (g_ArrayTidMixItem[i])
			{
				delete[] g_ArrayTidMixItem[i];
				g_ArrayTidMixItem[i] = NULL;
			}
		}

		delete[] g_ArrayTidMixItem;
		g_ArrayTidMixItem = NULL;
	}

	if (g_ArrayRecycleItem)
	{
		delete[] g_ArrayRecycleItem;
		g_ArrayRecycleItem = NULL;
	}
}

struct ChangeWordPair
{
	std::string from;
	std::string to;
};

ChangeWordPair ChangeWordList[] = {
	{ "'", "`" },
};

int ChangeWordListCount = sizeof(ChangeWordList) / sizeof(ChangeWordPair);

template <class T>
void CheckCommonItemName(T& data)
{
	std::string str(data.second.c.Name);

	for (int i = 0; i < ChangeWordListCount; i++)
	{
		if (str_Replace(str, ChangeWordList[i].from, ChangeWordList[i].to) ==
			true)
		{
			// 983 00a1cb50: limite 0x27 fixo, embora Name tenha 64 bytes
			if (str.length() >= 0x27)
			{
				strncpy(data.second.c.Name, str.c_str(), 0x27);
				data.second.c.Name[0x27] = 0;
			}
			else
			{
				strcpy(data.second.c.Name, str.c_str());
			}
		}
	}
}

template <class T>
void CheckItemName(T& data)
{
	std::string str(data.second.Name);

	for (int i = 0; i < ChangeWordListCount; i++)
	{
		if (str_Replace(str, ChangeWordList[i].from, ChangeWordList[i].to) ==
			true)
		{
			if (str.length() >= sizeof(data.second.Name) - 1)
			{
				strncpy(data.second.Name, str.c_str(),
					sizeof(data.second.Name) - 1);
				data.second.Name[sizeof(data.second.Name) - 1] = 0;
			}
			else
			{
				strcpy(data.second.Name, str.c_str());
			}
		}
	}
}

void CItemManager::CheckInvalidItemName()
{
	std::for_each(m_CharMap.begin(), m_CharMap.end(),
		CheckCommonItemName<std::pair<const unsigned int, IFF_STRUCT::sChar> >);
	std::for_each(m_PartMap.begin(), m_PartMap.end(),
		CheckCommonItemName<std::pair<const unsigned int, IFF_STRUCT::sPart> >);
	std::for_each(m_ClubMap.begin(), m_ClubMap.end(),
		CheckCommonItemName<std::pair<const unsigned int, IFF_STRUCT::sClub> >);
	std::for_each(m_ClubSetMap.begin(), m_ClubSetMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sClubSet> >);
	std::for_each(m_BallMap.begin(), m_BallMap.end(),
		CheckCommonItemName<std::pair<const unsigned int, IFF_STRUCT::sBall> >);
	std::for_each(m_ItemMap.begin(), m_ItemMap.end(),
		CheckCommonItemName<std::pair<const unsigned int, IFF_STRUCT::sItem> >);
	std::for_each(m_CaddieMap.begin(), m_CaddieMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sCaddie> >);
	std::for_each(m_CadItemMap.begin(), m_CadItemMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sCadItem> >);
	std::for_each(m_SetItemMap.begin(), m_SetItemMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sSetItem> >);
	std::for_each(m_SkinMap.begin(), m_SkinMap.end(),
		CheckCommonItemName<std::pair<const unsigned int, IFF_STRUCT::sSkin> >);
	std::for_each(m_HairStyleMap.begin(), m_HairStyleMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sHairStyle> >);
	std::for_each(m_MascotMap.begin(), m_MascotMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sMascot> >);
	std::for_each(m_AuxPartMap.begin(), m_AuxPartMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sAuxPart> >);
	std::for_each(m_QuestDropMap.begin(), m_QuestDropMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sQuestDrop> >);
	std::for_each(m_FurnitureMap.begin(), m_FurnitureMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sFurniture> >);
	std::for_each(m_CardMap.begin(), m_CardMap.end(),
		CheckCommonItemName<std::pair<const unsigned int, IFF_STRUCT::sCard> >);
	std::for_each(m_CourseMap.begin(), m_CourseMap.end(),
		CheckCommonItemName<
			std::pair<const unsigned int, IFF_STRUCT::sCourse> >);

	std::for_each(m_MatchMap.begin(), m_MatchMap.end(),
		CheckItemName<std::pair<const unsigned int, IFF_STRUCT::sMatch> >);
	std::for_each(m_TitleMap.begin(), m_TitleMap.end(),
		CheckItemName<std::pair<const unsigned int, IFF_STRUCT::sTitle> >);
	std::for_each(m_ChildItemMap.begin(), m_ChildItemMap.end(),
		CheckItemName<std::pair<const unsigned int, IFF_STRUCT::sChildItem> >);
}

bool CItemManager::Load()
{
	// JP 983 (00a3de50): o pacote é PangYa_JP.iff e o MD5 dele (em hexadecimal) fica em m_strLoaded983; o mesmo
	// pacote não é lido de novo.
	std::string path = "";

	path = GetPackName983("iff");

	WFile file(path.c_str());

	unsigned int len = file.Length();
	unsigned char* buff = new unsigned char[len];

	file.Read(buff, len);
	file.Close();

	char md5[0x80];

	memset(md5, 0, sizeof(md5));
	GetMD5String(buff, len, md5);

	if (m_strLoaded983 == md5)
	{
		delete[] buff;
		return true;
	}

	HZIP hz = OpenZipU(buff, len, ZIP_MEMORY);
	if (!hz)
	{
		delete[] buff;
		return false;
	}

	Reset();

	m_strLoaded983 = md5;

	ZIPENTRY ze;
	GetZipItemA(hz, -1, &ze);
	int numItems = ze.index;

	for (int i = 0; i < numItems; i++)
	{
		GetZipItemA(hz, i, &ze);
		char* pBuffer = new char[ze.unc_size];
		UnzipItem(hz, i, pBuffer, ze.unc_size, ZIP_MEMORY);

		if (!stricmp(ze.name, "Character.iff"))
		{
			MakeCharMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Part.iff"))
		{
			MakePartMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Club.iff"))
		{
			MakeClubMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ClubSet.iff"))
		{
			MakeClubSetMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Ball.iff"))
		{
			MakeBallMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Item.iff"))
		{
			MakeItemMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Caddie.iff"))
		{
			MakeCaddieMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "CaddieItem.iff"))
		{
			MakeCadItemMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "SetItem.iff"))
		{
			MakeSetItemMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Course.iff"))
		{
			MakeCourseMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Match.iff"))
		{
			MakeMatchMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Title.iff"))
		{
			MakeTitleMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Enchant.iff"))
		{
			MakeEnchantMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Skin.iff"))
		{
			MakeSkinMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "HairStyle.iff"))
		{
			MakeHairStyleMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ChildItem.iff"))
		{
			MakeChildItemMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Mascot.iff"))
		{
			MakeMascotMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "AuxPart.iff"))
		{
			MakeAuxPartMap(pBuffer, ze.unc_size);
		}
		// JP 983: QuestDrop.iff e Quest.iff não são mais lidos (MakeQuestDropMap e MakeQuestMap ficaram sem chamador)
		else if (!stricmp(ze.name, "Card.iff"))
		{
			if (IsLocalContent(S4_CARD_SYSTEM))
				MakeCardMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Furniture.iff"))
		{
			if (IsLocalContent(S4_REAL_MYROOM))
				MakeFurnitureMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Desc.iff"))
		{
			MakeDescMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "CadieMagicBox.iff"))
		{
			MakeCadieMagicBoxMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "CadieMagicBoxRandom.iff"))
		{
			MakeRandomRecycleMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "FurnitureAbility.iff"))
		{
			MakeFurnitureAbilityList(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "TikiRecipe.iff"))
		{
			MakeTikiRecipeMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "TikiPointTable.iff"))
		{
			MakeTikiPointTableMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "TikiSpecialTable.iff"))
		{
			MakeTikiSpecialTableMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "CutinInfomation.iff"))
		{
			MakeCutinInforMationTableMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "SpecialPrizeItem.iff"))
		{
			MakeSpecialPrizeItemMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "RandomBox.sff"))
		{
			MakeRandomBoxMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "NonVisibleItemTable.iff"))
		{
			MakeNonVisibleItemMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "SubscriptionItemTable.iff"))
		{
			MakeSubscriptionItemMap(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Achievement.iff"))
		{
			MakeAchievementMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "CounterItem.iff"))
		{
			MakeCounterItemMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "QuestStuff.iff"))
		{
			MakeQuestStuffMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "QuestItem.iff"))
		{
			MakeQuestItemMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ErrorCodeInfo.iff"))
		{
			MakeErrorCodeInfoMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ArtifactManaInfo.iff"))
		{
			MakeArtifactManaInfoMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "GrandPrixData.iff"))
		{
			MakeGrandPrixDataMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "Ability.iff"))
		{
			MakeAbilityMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "SetEffectTable.iff"))
		{
			MakeSetEffectTableMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "HoleCupDropItem.iff"))
		{
			MakeHoleCupDropItemMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ShopLimitItem.iff"))
		{
			MakeShopLimitItemMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "LevelUpPrizeItem.iff"))
		{
			MakeLevelUpPrizeItemMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ScratchShopConfig.iff"))
		{
			MakeScratchShopConfig983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ClubSetWorkShopChangeProb.sff"))
		{
			MakeWorkShopChangeProb983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "AttendancePrizeConfig.sff"))
		{
			MakeAttendancePrizeConfig983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "AlchemyConfig.iff"))
		{
			MakeAlchemyConfig983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "PointShop.iff"))
		{
			MakePointShopMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ScratchItem.sff"))
		{
			MakeScratchItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "BongdariShopNormalItem.sff"))
		{
			MakeBongdariShopNormalItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "BongdariShopRareItem.sff"))
		{
			MakeBongdariShopRareItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "BroadCastNotices.sff"))
		{
			MakeBroadCastNotices983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "TwinsItemTable.iff"))
		{
			MakeTwinsItemMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ScratchShopItem.iff"))
		{
			MakeScratchShopItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ClubSetWorkShopLevelUpProb.iff"))
		{
			MakeWorkShopLevelUpProb983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ClubSetWorkShopLevelUpLimit.iff"))
		{
			MakeWorkShopLevelUpLimit983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ClubSetWorkShopRankUpExp.iff"))
		{
			MakeWorkShopRankUpExp983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "ClubSetWorkShopChangeItem.sff"))
		{
			MakeWorkShopChangeItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "AttendancePrizeItem.sff"))
		{
			MakeAttendancePrizeItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "GrandPrixSpecialHole.iff"))
		{
			MakeGrandPrixSpecialHoleMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "GrandPrixConditionEquip.iff"))
		{
			MakeGrandPrixConditionEquipMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "GrandPrixRankReward.iff"))
		{
			MakeGrandPrixRankRewardMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "GrandPrixAIOptionalData.sff"))
		{
			MakeGrandPrixAIMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "AddonPart.iff"))
		{
			MakeAddonPartMap983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "MemorialShopRareItem.iff"))
		{
			MakeMemorialShopRareItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "MemorialShopCoinItem.sff"))
		{
			MakeMemorialShopCoinItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "MemorialShopNormalItem.sff"))
		{
			MakeMemorialShopNormalItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "MemorialShopRareItem.sff"))
		{
			MakeMemorialShopRareItem983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "CharacterMastery.iff"))
		{
			MakeCharacterMastery983(pBuffer, ze.unc_size);
		}
		else if (!stricmp(ze.name, "CaddieVoiceTable.iff"))
		{
			MakeCaddieVoiceTable983(pBuffer, ze.unc_size);
		}

		delete[] pBuffer;
	}

	CloseZipU(hz);
	delete[] buff;

	// JP 983: depois do laço vêm 00a3abd0, 00a2a620, 00a1f2a0 e 00a36130, nesta ordem
	MakeItemBuffMap();

	MakeCupInConditionMap983();

	CheckInvalidItemName();

	MakeDerivedTables983();

	return true;
}

void CItemManager::MakeCharMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sChar sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sChar));
		p += sizeof(IFF_STRUCT::sChar);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_CharMap.insert(
				std::map<unsigned int, IFF_STRUCT::sChar>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakePartMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sPart sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sPart));
		p += sizeof(IFF_STRUCT::sPart);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			DevFinalStockPrice(sItem.c);
			DevRentalPreventSale(sItem);
			m_PartMap.insert(
				std::map<unsigned int, IFF_STRUCT::sPart>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeClubMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sClub sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sClub));
		p += sizeof(IFF_STRUCT::sClub);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			DevFinalStockPrice(sItem.c);
			m_ClubMap.insert(
				std::map<unsigned int, IFF_STRUCT::sClub>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeClubSetMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sClubSet sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sClubSet));
		p += sizeof(IFF_STRUCT::sClubSet);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_ClubSetMap.insert(
				std::map<unsigned int, IFF_STRUCT::sClubSet>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeBallMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sBall sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sBall));
		p += sizeof(IFF_STRUCT::sBall);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_BallMap.insert(
				std::map<unsigned int, IFF_STRUCT::sBall>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeItemMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sItem sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sItem));
		p += sizeof(IFF_STRUCT::sItem);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			DevFinalStockPrice(sItem.c);
			m_ItemMap.insert(
				std::map<unsigned int, IFF_STRUCT::sItem>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeCaddieMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sCaddie sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sCaddie));
		p += sizeof(IFF_STRUCT::sCaddie);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_CaddieMap.insert(
				std::map<unsigned int, IFF_STRUCT::sCaddie>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeCadItemMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sCadItem sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sCadItem));
		p += sizeof(IFF_STRUCT::sCadItem);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_CadItemMap.insert(
				std::map<unsigned int, IFF_STRUCT::sCadItem>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeSetItemMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sSetItem sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sSetItem));
		p += sizeof(IFF_STRUCT::sSetItem);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			DevFinalStockPrice(sItem.c);
			m_SetItemMap.insert(
				std::map<unsigned int, IFF_STRUCT::sSetItem>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeCourseMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sCourse sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sCourse));
		p += sizeof(IFF_STRUCT::sCourse);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_CourseMap.insert(
				std::map<unsigned int, IFF_STRUCT::sCourse>::value_type(
					sItem.c.TypeId, sItem));
		}
	}

	memset(&sItem, 0, sizeof(IFF_STRUCT::sCourse));
	sItem.c.TypeId = 0x280000fd;

	sItem.Difficulty = 1;
	strcpy(sItem.c.Name, "TUTORIAL");
	strcpy(sItem.Data, "tuto");
	sItem.Slope = 1.1f;
	strcpy(sItem.TexProp, "tuto_property.xml");

	m_CourseMap.insert(std::map<unsigned int, IFF_STRUCT::sCourse>::value_type(
		sItem.c.TypeId, sItem));

	memset(&sItem, 0, sizeof(IFF_STRUCT::sCourse));
	sItem.c.TypeId = 0x280000fe;

	sItem.Difficulty = 1;
	strcpy(sItem.c.Name, "TEST");
	strcpy(sItem.Data, "test");
	sItem.Slope = 1.1f;
	strcpy(sItem.TexProp, "test_property.xml");

	m_CourseMap.insert(std::map<unsigned int, IFF_STRUCT::sCourse>::value_type(
		sItem.c.TypeId, sItem));

	memset(&sItem, 0, sizeof(IFF_STRUCT::sCourse));
	sItem.c.TypeId = 0x2800007f;
	sItem.c.Final = true;

	sItem.Difficulty = 1;
	strcpy(sItem.c.Name, "RANDOM");
	strcpy(sItem.Data, "random");
	sItem.Slope = 1.1f;
	strcpy(sItem.TexProp, "test_property.xml");
	strcpy(sItem.c.Icon, "map_random");

	m_CourseMap.insert(std::map<unsigned int, IFF_STRUCT::sCourse>::value_type(
		sItem.c.TypeId, sItem));
}

void CItemManager::MakeMatchMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sMatch sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sMatch));
		p += sizeof(IFF_STRUCT::sMatch);

		if (sItem.Final)
		{
			m_MatchMap.insert(
				std::map<unsigned int, IFF_STRUCT::sMatch>::value_type(
					sItem.TypeId, sItem));
		}
	}
}

void CItemManager::MakeTitleMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sTitle sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sTitle));
		p += sizeof(IFF_STRUCT::sTitle);

		if (sItem.Final)
		{
			m_TitleMap.insert(
				std::map<unsigned int, IFF_STRUCT::sTitle>::value_type(
					sItem.TypeId, sItem));
		}
	}
}

void CItemManager::MakeEnchantMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sEnchant sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sEnchant));
		p += sizeof(IFF_STRUCT::sEnchant);

		if (sItem.Final)
		{
			m_EnchantMap.insert(
				std::map<unsigned int, IFF_STRUCT::sEnchant>::value_type(
					sItem.TypeId, sItem));
		}
	}
}

void CItemManager::MakeSkinMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sSkin sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sSkin));
		p += sizeof(IFF_STRUCT::sSkin);

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_SkinMap.insert(
				std::map<unsigned int, IFF_STRUCT::sSkin>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeCardMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sCard sCard;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sCard, p, sizeof(IFF_STRUCT::sCard));
		p += sizeof(IFF_STRUCT::sCard);

		if (sCard.c.SalePrice == 0)
			sCard.c.SalePrice = sCard.c.Price;

		if (sCard.c.UsedPrice >= sCard.c.SalePrice)
			sCard.c.UsedPrice = 0;

		if (sCard.c.saleStart.wYear != 0 || sCard.c.saleEnd.wYear != 0)
		{
			sCard.c.New = 0;
			sCard.c.Hit = 0;
		}

		if (sCard.c.Final)
		{
			m_CardMap.insert(
				std::map<unsigned int, IFF_STRUCT::sCard>::value_type(
					sCard.c.TypeId, sCard));
		}
	}
}

void CItemManager::MakeFurnitureMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sFurniture sFurni;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&sFurni, p, sizeof(IFF_STRUCT::sFurniture));
		p += sizeof(IFF_STRUCT::sFurniture);

		if (sFurni.c.SalePrice == 0)
			sFurni.c.SalePrice = sFurni.c.Price;

		if (sFurni.c.UsedPrice >= sFurni.c.SalePrice)
			sFurni.c.UsedPrice = 0;

		if (sFurni.c.saleStart.wYear != 0 || sFurni.c.saleEnd.wYear != 0)
		{
			sFurni.c.New = 0;
			sFurni.c.Hit = 0;
		}

		if (sFurni.c.Final)
		{
			DevFinalStockPrice(sFurni.c);
			m_FurnitureMap.insert(
				std::map<unsigned int, IFF_STRUCT::sFurniture>::value_type(
					sFurni.c.TypeId, sFurni));
		}
	}
}

void CItemManager::MakeCadieMagicBoxMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sCadieMagicBox record;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_CadieMagicBox.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&record, p, sizeof(IFF_STRUCT::sCadieMagicBox));
		p += sizeof(IFF_STRUCT::sCadieMagicBox);

		m_CadieMagicBox.push_back(record);

		switch (record.uiCategory)
		{
		case 0:
			g_NumLowRecycleItem++;
			break;
		case 1:
			g_NumMidRecycleItem++;
			break;
		case 2:
			g_NumHighRecycleItem++;
			break;
		case 3:
			g_NumSpecialRecycleItem++;
			break;
		case 4:
			g_NumEventRecycleItem++;
			break;
		}

		g_NumIndexRecycleItem++;
	}

	if (g_ArrayTidMixItem)
	{
		for (int i = 0; i < 4; i++)
		{
			if (g_ArrayTidMixItem[i])
			{
				delete[] g_ArrayTidMixItem[i];
				g_ArrayTidMixItem[i] = NULL;
			}
		}

		delete[] g_ArrayTidMixItem;
		g_ArrayTidMixItem = NULL;
	}

	if (g_ArrayRecycleItem)
	{
		delete[] g_ArrayRecycleItem;
		g_ArrayRecycleItem = NULL;
	}

	g_ArrayTidMixItem = new sRecycleItem*[g_NumIndexRecycleItem];
	if (g_ArrayTidMixItem)
	{
		for (int i = 0; i < g_NumIndexRecycleItem; i++)
			g_ArrayTidMixItem[i] = new sRecycleItem[4];
	}

	g_ArrayRecycleItem = new sRecycleItem[g_NumIndexRecycleItem];

	std::list<IFF_STRUCT::sCadieMagicBox>::iterator it =
		m_CadieMagicBox.begin();
	std::list<IFF_STRUCT::sCadieMagicBox>::iterator itEnd =
		m_CadieMagicBox.end();

	for (; it != itEnd; ++it)
	{
		memset(&g_ArrayRecycleItem[(*it).uiNumber - 1], 0,
			sizeof(sRecycleItem));

		g_ArrayRecycleItem[(*it).uiNumber - 1].dwTid = (*it).uiOutput;
		g_ArrayRecycleItem[(*it).uiNumber - 1].iNum =
			(unsigned short)(*it).uiOutputCount;
		g_ArrayRecycleItem[(*it).uiNumber - 1].byLevel =
			(unsigned char)(*it).iLevel;
		g_ArrayRecycleItem[(*it).uiNumber - 1].iChar = (short)(*it).iCharacter;

		if ((*it).uiRandSeq > 0)
		{
			g_ArrayRecycleItem[(*it).uiNumber - 1].dwRandSeq = (*it).uiRandSeq;
			strncpy(g_ArrayRecycleItem[(*it).uiNumber - 1].szRandName,
				(*it).szRandName, 39);
			g_ArrayRecycleItem[(*it).uiNumber - 1].szRandName[39] = 0;
		}

		for (int j = 0; j < 4; j++)
		{
			memset(&g_ArrayTidMixItem[(*it).uiNumber - 1][j], 0,
				sizeof(sRecycleItem));
			g_ArrayTidMixItem[(*it).uiNumber - 1][j].dwTid = (*it).uiElem[j];
			g_ArrayTidMixItem[(*it).uiNumber - 1][j].iNum =
				(unsigned short)(*it).uiElemCount[j];
			g_ArrayTidMixItem[(*it).uiNumber - 1][j].byLevel =
				(unsigned char)(*it).iLevel;
		}

		// 983 (00a23221): campos novos do registro; a lista nao e esvaziada no fim
		g_ArrayRecycleItem[(*it).uiNumber - 1].byFlag983 =
			(unsigned char)(*it).uiFlag983;
		g_ArrayRecycleItem[(*it).uiNumber - 1].dateStart983 = (*it).dateStart983;
		g_ArrayRecycleItem[(*it).uiNumber - 1].dateEnd983 = (*it).dateEnd983;
	}
}

void CItemManager::MakeRandomRecycleMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sRandomRecycle record;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	std::set<unsigned long> seqNumberSet;
	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&record, p, sizeof(IFF_STRUCT::sRandomRecycle));
		p += sizeof(IFF_STRUCT::sRandomRecycle);

		if (seqNumberSet.find(record.uiRandSeq) == seqNumberSet.end())
			seqNumberSet.insert(record.uiRandSeq);

		m_RandomRecycleMaps.insert(
			std::multimap<unsigned int, IFF_STRUCT::sRandomRecycle>::value_type(
				record.uiRandSeq, record));
	}

	std::set<unsigned long>::const_iterator ci;
	for (ci = seqNumberSet.begin(); ci != seqNumberSet.end(); ++ci)
	{
		std::multimap<unsigned int, IFF_STRUCT::sRandomRecycle>::const_iterator
			lower = m_RandomRecycleMaps.lower_bound(*ci);
		std::multimap<unsigned int, IFF_STRUCT::sRandomRecycle>::const_iterator
			upper = m_RandomRecycleMaps.upper_bound(*ci);
		while (lower != upper)
		{
			++lower;
		}
	}
}

void CItemManager::MakeTikiRecipeMap(char* Buf, int len)
{
	IFF_STRUCT::sTikiOutputTable record;

	IFF_FILE_HEADER _hdr;
	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&record, p, sizeof(IFF_STRUCT::sTikiOutputTable));
		p += sizeof(IFF_STRUCT::sTikiOutputTable);

		if (CTikiMagicBoxDoc::Instance())
			CTikiMagicBoxDoc::Instance()->InsertOutput(record);
	}
}

void CItemManager::MakeTikiPointTableMap(char* Buf, int len)
{
	IFF_STRUCT::sTikiPointTable record;

	IFF_FILE_HEADER _hdr;
	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&record, p, sizeof(IFF_STRUCT::sTikiPointTable));
		p += sizeof(IFF_STRUCT::sTikiPointTable);

		if (CTikiMagicBoxDoc::Instance())
			CTikiMagicBoxDoc::Instance()->InsertPointTable(record);
	}
}

void CItemManager::MakeTikiSpecialTableMap(char* Buf, int len)
{
	IFF_STRUCT::sTikiSpecialRecipe record;

	IFF_FILE_HEADER _hdr;
	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&record, p, sizeof(IFF_STRUCT::sTikiSpecialRecipe));
		p += sizeof(IFF_STRUCT::sTikiSpecialRecipe);

		if (CTikiMagicBoxDoc::Instance())
			CTikiMagicBoxDoc::Instance()->InsertSpecialRecipe(record);
	}
}

struct stFindRandomBox
{
	stFindRandomBox(unsigned long typeId, unsigned long checkTypeId)
		: m_typeId(typeId), m_checkTypeId(checkTypeId)
	{
	}

	bool operator()(
		const std::pair<unsigned long, _private::sRandomBoxItem>& item) const
	{
		return item.second.box.typeId == m_typeId &&
			item.second.box.checkTypeId ==
			(m_checkTypeId ? m_checkTypeId : item.second.box.checkTypeId);
	}

	unsigned long m_typeId;
	unsigned long m_checkTypeId;
};

void CItemManager::MakeRandomBoxMap(char* Buf, int len)
{
	static unsigned long s_itemIndex;

	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sRandomBox box;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_RandomBoxMap.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&box, p, sizeof(IFF_STRUCT::sRandomBox));
		p += sizeof(IFF_STRUCT::sRandomBox);

		if (!box.active)
			continue;

		unsigned char kind = box.kind;
		if (!kind)
			continue;

		std::map<unsigned int, _private::sRandomBox>::iterator it =
			m_RandomBoxMap.find(box.boxTypeId);
		if (it == m_RandomBoxMap.end() && kind == 1)
		{
			std::pair<std::map<unsigned int, _private::sRandomBox>::iterator,
				bool>
				ret = m_RandomBoxMap.insert(
					std::map<unsigned int, _private::sRandomBox>::value_type(
						box.boxTypeId, _private::sRandomBox()));
			if (!ret.second)
				continue;

			it = ret.first;
		}

		if (it == m_RandomBoxMap.end())
			continue;

		_private::sRandomBox& randomBox = (*it).second;

		switch (kind)
		{
		case 1:
			memcpy(&randomBox.box, &box, sizeof(IFF_STRUCT::sRandomBox));
			break;
		case 2:
		{
			_private::sRandomBoxItem item;
			memcpy(&item.box, &box, sizeof(IFF_STRUCT::sRandomBox));
			std::pair<
				std::map<unsigned long, _private::sRandomBoxItem>::iterator,
				bool>
				result = randomBox.items.insert(std::map<unsigned long,
					_private::sRandomBoxItem>::value_type(++s_itemIndex, item));
			if (!result.second)
				break;
		}
		break;
		case 3:
		{
			std::map<unsigned long, _private::sRandomBoxItem>::iterator itItem =
				std::find_if(randomBox.items.begin(), randomBox.items.end(),
					stFindRandomBox(box.typeId, box.checkTypeId));
			if (itItem == randomBox.items.end())
				break;

			(*itItem).second.checkItems.push_back(box);
		}
		break;
		case 4:
		{
			_private::sRandomBoxStuff stuff;
			memcpy(&stuff.box, &box, sizeof(IFF_STRUCT::sRandomBox));
			randomBox.stuffs.push_back(stuff);
		}
		break;
		case 5:
		{
			// 983 (00a3ddc9): quinto tipo de registro vai para o segundo vetor (+0x104 do valor do mapa)
			_private::sRandomBoxStuff stuff;
			memcpy(&stuff.box, &box, sizeof(IFF_STRUCT::sRandomBox));
			randomBox.stuffsType5.push_back(stuff);
		}
		break;
		}
	}
}

void CItemManager::MakeNonVisibleItemMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sNonVisibleItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_NonVisibleItemMap.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF_STRUCT::sNonVisibleItem));
		p += sizeof(IFF_STRUCT::sNonVisibleItem);

		if (item.active)
		{
			std::pair<std::map<unsigned int,
						  IFF_STRUCT::sNonVisibleItem>::const_iterator,
				bool>
				result = m_NonVisibleItemMap.insert(std::map<unsigned int,
					IFF_STRUCT::sNonVisibleItem>::value_type(item.typeId,
					item));
		}
	}
}

void CItemManager::MakeSubscriptionItemMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sSubscriptionItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_SubscriptionItemMap.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF_STRUCT::sSubscriptionItem));
		p += sizeof(IFF_STRUCT::sSubscriptionItem);

		if (item.active)
		{
			std::pair<std::map<unsigned int,
						  IFF_STRUCT::sSubscriptionItem>::const_iterator,
				bool>
				result = m_SubscriptionItemMap.insert(std::map<unsigned int,
					IFF_STRUCT::sSubscriptionItem>::value_type(item.typeId,
					item));
		}
	}
}

void CItemManager::MakeHairStyleMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sHairStyle sHair;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sHair, p, sizeof(IFF_STRUCT::sHairStyle));
		p += sizeof(IFF_STRUCT::sHairStyle);

		if (sHair.c.SalePrice == 0)
			sHair.c.SalePrice = sHair.c.Price;

		if (sHair.c.UsedPrice >= sHair.c.SalePrice)
			sHair.c.UsedPrice = 0;

		if (sHair.c.saleStart.wYear != 0 || sHair.c.saleEnd.wYear != 0)
		{
			sHair.c.New = 0;
			sHair.c.Hit = 0;
		}

		if (sHair.c.Final)
		{
			m_HairStyleMap.insert(
				std::map<unsigned int, IFF_STRUCT::sHairStyle>::value_type(
					sHair.c.TypeId, sHair));
		}
	}
}

void CItemManager::MakeChildItemMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sChildItem sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sChildItem));
		p += sizeof(IFF_STRUCT::sChildItem);

		if (sItem.Final)
		{
			m_ChildItemMap.insert(
				std::map<unsigned int, IFF_STRUCT::sChildItem>::value_type(i,
					sItem));
		}
	}
}

void CItemManager::MakeAuxPartMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sAuxPart sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sAuxPart));
		p += sizeof(IFF_STRUCT::sAuxPart);

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			DevFinalStockPrice(sItem.c);
			m_AuxPartMap.insert(
				std::map<unsigned int, IFF_STRUCT::sAuxPart>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeQuestDropMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sQuestDrop sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sQuestDrop));
		p += sizeof(IFF_STRUCT::sQuestDrop);

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			m_QuestDropMap.insert(
				std::map<unsigned int, IFF_STRUCT::sQuestDrop>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeQuestMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sQuest sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sQuest));
		p += sizeof(IFF_STRUCT::sQuest);

		sItem.ProbTotal = 0;
		sItem.CurItemIdx = 0;
		sItem.aTidItems = NULL;

		if (sItem.Final)
		{
			m_QuestMap.insert(
				std::map<unsigned int, IFF_STRUCT::sQuest>::value_type(
					sItem.TypeId, sItem));
		}
	}
}

void CItemManager::MakeMascotMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sMascot sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::sMascot));
		p += sizeof(IFF_STRUCT::sMascot);

		if (sItem.c.SalePrice == 0)
			sItem.c.SalePrice = sItem.c.Price;

		if (sItem.c.UsedPrice >= sItem.c.SalePrice)
			sItem.c.UsedPrice = 0;

		if (sItem.c.saleStart.wYear != 0 || sItem.c.saleEnd.wYear != 0)
		{
			sItem.c.New = 0;
			sItem.c.Hit = 0;
		}

		if (sItem.c.Final)
		{
			DevFinalStockPrice(sItem.c);
			m_MascotMap.insert(
				std::map<unsigned int, IFF_STRUCT::sMascot>::value_type(
					sItem.c.TypeId, sItem));
		}
	}
}

void CItemManager::MakeDescMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sDesc desc;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&desc, p, sizeof(IFF_STRUCT::sDesc));
		p += sizeof(IFF_STRUCT::sDesc);

		m_DescMap.insert(std::map<unsigned int, IFF_STRUCT::sDesc>::value_type(
			desc.TypeId, desc));
	}
}

void CItemManager::MakeFurnitureAbilityList(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sFurnitureAbility sOs;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sOs, p, sizeof(IFF_STRUCT::sFurnitureAbility));
		p += sizeof(IFF_STRUCT::sFurnitureAbility);

		if (sOs.bFinal)
			m_FurnitureAbility.push_back(sOs);
	}
}

// JP 983: leitores das tabelas que só o 983 carrega (mesma forma do MakeNonVisibleItemMap: limpa o mapa e insere
// os registros com bFinal). Escritos pelo desmonte de cada leitor (out/F3.5/s78/RELATORIO.md).

// Achievement.iff (00a2aed0)
void CItemManager::MakeAchievementMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sAchievement item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_AchievementMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sAchievement));
		p += sizeof(IFF983::sAchievement);

		if (item.bFinal)
		{
			m_AchievementMap983.insert(
				std::map<unsigned int, IFF983::sAchievement>::value_type(item.TypeId, item));
		}
	}
}

// CounterItem.iff (00a2a9e0)
void CItemManager::MakeCounterItemMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sCounterItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_CounterItemMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sCounterItem));
		p += sizeof(IFF983::sCounterItem);

		if (item.bFinal)
		{
			m_CounterItemMap983.insert(
				std::map<unsigned int, IFF983::sCounterItem>::value_type(item.TypeId, item));
		}
	}
}

// QuestStuff.iff (00a2aa90)
void CItemManager::MakeQuestStuffMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sQuestStuff item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_QuestStuffMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sQuestStuff));
		p += sizeof(IFF983::sQuestStuff);

		if (item.bFinal)
		{
			m_QuestStuffMap983.insert(
				std::map<unsigned int, IFF983::sQuestStuff>::value_type(item.TypeId, item));
		}
	}
}

// QuestItem.iff (00a2ab60)
void CItemManager::MakeQuestItemMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sQuestItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_QuestItemMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sQuestItem));
		p += sizeof(IFF983::sQuestItem);

		if (item.bFinal)
		{
			m_QuestItemMap983.insert(
				std::map<unsigned int, IFF983::sQuestItem>::value_type(item.TypeId, item));
		}
	}
}

// ErrorCodeInfo.iff (00a2a930)
void CItemManager::MakeErrorCodeInfoMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sErrorCodeInfo item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_ErrorCodeInfoMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sErrorCodeInfo));
		p += sizeof(IFF983::sErrorCodeInfo);

		if (item.bFinal)
		{
			m_ErrorCodeInfoMap983.insert(
				std::map<unsigned int, IFF983::sErrorCodeInfo>::value_type(item.TypeId, item));
		}
	}
}

// ArtifactManaInfo.iff (00a2b770)
void CItemManager::MakeArtifactManaInfoMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sArtifactManaInfo item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_ArtifactManaInfoMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sArtifactManaInfo));
		p += sizeof(IFF983::sArtifactManaInfo);

		if (item.bFinal)
		{
			m_ArtifactManaInfoMap983.insert(
				std::map<unsigned int, IFF983::sArtifactManaInfo>::value_type(item.TypeId, item));
		}
	}
}

// GrandPrixData.iff (00a2b820)
void CItemManager::MakeGrandPrixDataMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sGrandPrixData item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_GrandPrixDataMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sGrandPrixData));
		p += sizeof(IFF983::sGrandPrixData);

		if (item.bFinal)
		{
			m_GrandPrixDataMap983.insert(
				std::map<unsigned int, IFF983::sGrandPrixData>::value_type(item.TypeId, item));
		}
	}
}

// Ability.iff (00a2afa0)
void CItemManager::MakeAbilityMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sAbility item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_AbilityMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sAbility));
		p += sizeof(IFF983::sAbility);

		if (item.bFinal)
		{
			m_AbilityMap983.insert(
				std::map<unsigned int, IFF983::sAbility>::value_type(item.TypeId, item));
		}
	}
}

// SetEffectTable.iff (00a2b050)
void CItemManager::MakeSetEffectTableMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sSetEffectTable item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_SetEffectTableMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sSetEffectTable));
		p += sizeof(IFF983::sSetEffectTable);

		m_SetEffectTableMap983.insert(
			std::map<unsigned int, IFF983::sSetEffectTable>::value_type(item.id, item));
	}
}

// HoleCupDropItem.iff (00a2b8d0)
void CItemManager::MakeHoleCupDropItemMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sHoleCupDropItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_HoleCupDropItemMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sHoleCupDropItem));
		p += sizeof(IFF983::sHoleCupDropItem);

		m_HoleCupDropItemMap983.insert(
			std::map<unsigned int, IFF983::sHoleCupDropItem>::value_type(item.TypeId, item));
	}
}

// ShopLimitItem.iff (00a2a360)
void CItemManager::MakeShopLimitItemMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sShopLimitItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF983::sShopLimitItem));
		p += sizeof(IFF983::sShopLimitItem);

		if (item.bFinal)
		{
			m_ShopLimitItemMap983.insert(
				std::map<unsigned int, IFF983::sShopLimitItem>::value_type(item.TypeId, item));
		}
	}
}

// LevelUpPrizeItem.iff (00a2a880): o arquivo traz 0xc0 por registro; a chave é o nível (+0x22). O resto do valor
// (0x1c4) não vem do arquivo e não é inicializado pelo leitor do 983.
void CItemManager::MakeLevelUpPrizeItemMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sLevelUpPrizeItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_LevelUpPrizeItemMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, 0xc0);
		p += 0xc0;

		if (item.bFinal)
		{
			m_LevelUpPrizeItemMap983.insert(
				std::map<unsigned int, IFF983::sLevelUpPrizeItem>::value_type(item.level, item));
		}
	}
}

// ScratchShopConfig.iff (009d9e60), ClubSetWorkShopChangeProb.sff (009d9f10), AttendancePrizeConfig.sff (009d9f70) e
// AlchemyConfig.iff (009da020): um registro só, copiado para dentro da classe (se vierem vários fica o último).
void CItemManager::MakeScratchShopConfig983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		m_ScratchShopConfig983 = *(IFF983::sScratchShopConfig*)p;
		p += sizeof(IFF983::sScratchShopConfig);
	}
}

void CItemManager::MakeWorkShopChangeProb983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		m_WorkShopChangeProb983 = *(IFF983::sClubSetWorkShopChangeProb*)p;
		p += sizeof(IFF983::sClubSetWorkShopChangeProb);
	}
}

void CItemManager::MakeAttendancePrizeConfig983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		m_AttendancePrizeConfig983 = *(IFF983::sAttendancePrizeConfig*)p;
		p += sizeof(IFF983::sAttendancePrizeConfig);
	}
}

void CItemManager::MakeAlchemyConfig983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		m_AlchemyConfig983 = *(unsigned long*)p;
		p += sizeof(unsigned long);
	}
}

// JP 983: os 22 leitores que faltavam do Load 00a3de50 (sessão 80; pseudo-C em out/F3.5/s80/pseudoc_983.txt).

// PointShop.iff (00a2f610): sem teste de bFinal; só entra o primeiro registro de cada tid
void CItemManager::MakePointShopMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sPointShop item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_PointShopMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF983::sPointShop));
		p += sizeof(IFF983::sPointShop);

		if (m_PointShopMap983.find(item.TypeId) == m_PointShopMap983.end())
			m_PointShopMap983[item.TypeId] = item;
	}
}

// ScratchItem.sff (00a2a550): guarda só o tid dos registros com bFinal e bit 1 de +0x04. O 983 lê a hora atual
// (_time64) no começo e não a usa.
void CItemManager::MakeScratchItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sScratchItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	__time64_t now = 0;
	_time64(&now);

	m_ScratchItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF983::sScratchItem));
		p += sizeof(IFF983::sScratchItem);

		if (item.bFinal && (item.flag & 2))
		{
			unsigned long tid = item.TypeId;
			m_ScratchItem983.push_back(tid);
		}
	}
}

// BongdariShopNormalItem.sff (00a31fe0)
void CItemManager::MakeBongdariShopNormalItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sBongdariShopNormalItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_BongdariShopNormalItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sBongdariShopNormalItem));
		p += sizeof(IFF983::sBongdariShopNormalItem);

		if (item.bFinal)
			m_BongdariShopNormalItem983.push_back(item);
	}
}

// BongdariShopRareItem.sff (00a32180)
void CItemManager::MakeBongdariShopRareItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sBongdariShopRareItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_BongdariShopRareItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF983::sBongdariShopRareItem));
		p += sizeof(IFF983::sBongdariShopRareItem);

		if (item.bFinal)
			m_BongdariShopRareItem983.push_back(item);
	}
}

// 009df4f0: período de repetição do aviso em segundos (0 = não repete)
unsigned __int64 GetBroadCastNoticePeriod983(const IFF983::sBroadCastNotice* pNotice)
{
	switch (pNotice->periodUnit)
	{
	case 1:
		return pNotice->periodCount * 604800;
	case 2:
		return pNotice->periodCount * 86400;
	case 3:
		return pNotice->periodCount * 3600;
	case 4:
		return pNotice->periodCount * 60;
	}

	return 0;
}

C_ASSERT(sizeof(IFF983::sBroadCastNoticeEntry) == 0x240);

// 009e9dc0: calcula a próxima exibição a partir do instante inicial (+0x220) e do período
static void CalcBroadCastNoticeTime983(__int64 now, IFF983::sBroadCastNoticeEntry* pEntry, char bShown)
{
	__int64 start = pEntry->notice.time220;

	if (now < start)
	{
		pEntry->last = 0;
		pEntry->next = start;
		return;
	}

	if (bShown == 1)
		pEntry->last = now;

	__int64 period = GetBroadCastNoticePeriod983(&pEntry->notice);
	if (period != 0)
	{
		if (pEntry->next == 0)
			pEntry->next = start + ((now - start) - (now - start) % period);

		pEntry->next += period;
		return;
	}

	pEntry->next = 0x7fffffff;
}

// BroadCastNotices.sff (00a36150 + 00a35360/00a34020): registros com bFinal e bit 1 de +0x04
void CItemManager::MakeBroadCastNotices983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sBroadCastNotice item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	__time64_t now = 0;
	_time64(&now);

	std::vector<IFF983::sBroadCastNoticeEntry>* pNotices = m_pBroadCastNotices983;

	if (pNotices)
		pNotices->clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF983::sBroadCastNotice));
		p += sizeof(IFF983::sBroadCastNotice);

		if (item.bFinal && (item.flag & 2) && pNotices)
		{
			IFF983::sBroadCastNoticeEntry entry;

			memset(&entry, 0, sizeof(entry));
			entry.notice = item;
			CalcBroadCastNoticeTime983(now, &entry, 0);
			pNotices->push_back(entry);
		}
	}
}

// TwinsItemTable.iff (00a39aa0 + 00a390a0): para cada tid não nulo do registro, a lista de todos os tids não nulos
// do mesmo registro (ele próprio incluído)
void CItemManager::MakeTwinsItemMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sTwinsItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	if (m_pTwinsItemMap983)
		m_pTwinsItemMap983->clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sTwinsItem));
		p += sizeof(IFF983::sTwinsItem);

		if (item.bFinal && (item.flag & 2) && m_pTwinsItemMap983)
		{
			for (int a = 0; a < 5; a++)
			{
				if (item.tid[a] == 0)
					continue;

				for (unsigned int b = 0; b < 5; b++)
				{
					unsigned long tid = item.tid[b];

					if (tid != 0 && item.tid[a] != 0)
					{
						std::map<unsigned int, std::vector<unsigned long> >::iterator it =
							m_pTwinsItemMap983->find(item.tid[a]);

						if (it == m_pTwinsItemMap983->end())
						{
							it = m_pTwinsItemMap983->insert(
								std::map<unsigned int, std::vector<unsigned long> >::value_type(
									item.tid[a], std::vector<unsigned long>())).first;
						}

						it->second.push_back(tid);
					}
				}
			}
		}
	}
}

// ScratchShopItem.iff (00a32230)
void CItemManager::MakeScratchShopItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sScratchShopItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_ScratchShopItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sScratchShopItem));
		p += sizeof(IFF983::sScratchShopItem);

		if (item.bFinal)
			m_ScratchShopItem983.push_back(item);
	}
}

// ClubSetWorkShopLevelUpProb.iff (00a322d0): sem teste de bFinal
void CItemManager::MakeWorkShopLevelUpProb983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sClubSetWorkShopLevelUpProb item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_WorkShopLevelUpProb983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sClubSetWorkShopLevelUpProb));
		p += sizeof(IFF983::sClubSetWorkShopLevelUpProb);

		m_WorkShopLevelUpProb983.push_back(item);
	}
}

// ClubSetWorkShopLevelUpLimit.iff (00a323e0): sem teste de bFinal
void CItemManager::MakeWorkShopLevelUpLimit983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sClubSetWorkShopLevelUpLimit item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_WorkShopLevelUpLimit983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sClubSetWorkShopLevelUpLimit));
		p += sizeof(IFF983::sClubSetWorkShopLevelUpLimit);

		m_WorkShopLevelUpLimit983.push_back(item);
	}
}

// ClubSetWorkShopRankUpExp.iff (00a324e0): sem teste de bFinal
void CItemManager::MakeWorkShopRankUpExp983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sClubSetWorkShopRankUpExp item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_WorkShopRankUpExp983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sClubSetWorkShopRankUpExp));
		p += sizeof(IFF983::sClubSetWorkShopRankUpExp);

		m_WorkShopRankUpExp983.push_back(item);
	}
}

// ClubSetWorkShopChangeItem.sff (00a32590): sem teste de bFinal
void CItemManager::MakeWorkShopChangeItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sClubSetWorkShopChangeItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_WorkShopChangeItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sClubSetWorkShopChangeItem));
		p += sizeof(IFF983::sClubSetWorkShopChangeItem);

		m_WorkShopChangeItem983.push_back(item);
	}
}

// AttendancePrizeItem.sff (00a32690)
void CItemManager::MakeAttendancePrizeItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sAttendancePrizeItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_AttendancePrizeItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sAttendancePrizeItem));
		p += sizeof(IFF983::sAttendancePrizeItem);

		if (item.bFinal)
			m_AttendancePrizeItem983.push_back(item);
	}
}

// GrandPrixSpecialHole.iff (00a3c200): os registros com bFinal ficam agrupados pela chave de +0x04
void CItemManager::MakeGrandPrixSpecialHoleMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sGrandPrixSpecialHole item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_GrandPrixSpecialHoleMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sGrandPrixSpecialHole));
		p += sizeof(IFF983::sGrandPrixSpecialHole);

		if (item.bFinal)
		{
			std::map<unsigned int, std::vector<IFF983::sGrandPrixSpecialHole> >::iterator it = m_GrandPrixSpecialHoleMap983.find(item.TypeId);

			if (it == m_GrandPrixSpecialHoleMap983.end())
			{
				std::vector<IFF983::sGrandPrixSpecialHole> holes;

				holes.push_back(item);
				m_GrandPrixSpecialHoleMap983.insert(
					std::map<unsigned int, std::vector<IFF983::sGrandPrixSpecialHole> >::value_type(item.TypeId, holes));
			}
			else
			{
				it->second.push_back(item);
			}
		}
	}
}

// GrandPrixConditionEquip.iff (00a3b470): os registros com bFinal ficam agrupados pela chave de +0x04
void CItemManager::MakeGrandPrixConditionEquipMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sGrandPrixConditionEquip item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_GrandPrixConditionEquipMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sGrandPrixConditionEquip));
		p += sizeof(IFF983::sGrandPrixConditionEquip);

		if (item.bFinal)
		{
			std::map<unsigned int, std::vector<IFF983::sGrandPrixConditionEquip> >::iterator it = m_GrandPrixConditionEquipMap983.find(item.TypeId);

			if (it == m_GrandPrixConditionEquipMap983.end())
			{
				it = m_GrandPrixConditionEquipMap983.insert(std::map<unsigned int, std::vector<IFF983::sGrandPrixConditionEquip> >::value_type(
					item.TypeId, std::vector<IFF983::sGrandPrixConditionEquip>())).first;
			}

			it->second.push_back(item);
		}
	}
}

// GrandPrixRankReward.iff (00a3b6d0): os registros com bFinal ficam agrupados pela chave de +0x04
void CItemManager::MakeGrandPrixRankRewardMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sGrandPrixRankReward item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_GrandPrixRankRewardMap983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sGrandPrixRankReward));
		p += sizeof(IFF983::sGrandPrixRankReward);

		if (item.bFinal)
		{
			std::map<unsigned int, std::vector<IFF983::sGrandPrixRankReward> >::iterator it = m_GrandPrixRankRewardMap983.find(item.TypeId);

			if (it == m_GrandPrixRankRewardMap983.end())
			{
				it = m_GrandPrixRankRewardMap983.insert(std::map<unsigned int, std::vector<IFF983::sGrandPrixRankReward> >::value_type(
					item.TypeId, std::vector<IFF983::sGrandPrixRankReward>())).first;
			}

			it->second.push_back(item);
		}
	}
}

// GrandPrixAIOptionalData.sff (00a380b0): o registro entra no mapa por id e o id entra em dois índices (pelo byte
// de +0x34 e, se não for zero, pelo valor de +0x54)
void CItemManager::MakeGrandPrixAIMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sGrandPrixAIOptionalData item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_GrandPrixAIMap983_4d4.clear();
	m_GrandPrixAIMap983.clear();
	m_GrandPrixAIMap983_4e4.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF983::sGrandPrixAIOptionalData));
		p += sizeof(IFF983::sGrandPrixAIOptionalData);

		if (!item.bFinal)
			continue;

		unsigned short id = item.id;

		std::map<unsigned char, std::vector<unsigned short> >::iterator it34 = m_GrandPrixAIMap983_4d4.find(item.key34);
		if (it34 == m_GrandPrixAIMap983_4d4.end())
		{
			std::vector<unsigned short> ids;

			ids.push_back(id);
			m_GrandPrixAIMap983_4d4.insert(
				std::map<unsigned char, std::vector<unsigned short> >::value_type(item.key34, ids));
		}
		else
		{
			it34->second.push_back(id);
		}

		if (item.key54 != 0)
		{
			std::map<unsigned int, std::vector<unsigned short> >::iterator it54 =
				m_GrandPrixAIMap983_4e4.find(item.key54);

			if (it54 == m_GrandPrixAIMap983_4e4.end())
			{
				std::vector<unsigned short> ids;

				ids.push_back(id);
				m_GrandPrixAIMap983_4e4.insert(
					std::map<unsigned int, std::vector<unsigned short> >::value_type(item.key54, ids));
			}
			else
			{
				it54->second.push_back(id);
			}
		}

		if (m_GrandPrixAIMap983.find(id) == m_GrandPrixAIMap983.end())
			m_GrandPrixAIMap983[id] = item;
	}
}

// AddonPart.iff (00a3bef0): grupo = número do personagem quando o tid é de personagem (tipo 1) ou o campo de
// 8 bits em 18 quando é de peça (tipo 2) com os bits 0x600 zerados; os outros tids ficam de fora. O 983 não limpa
// o mapa antes.
void CItemManager::MakeAddonPartMap983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sAddonPart item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&item, p, sizeof(IFF983::sAddonPart));
		p += sizeof(IFF983::sAddonPart);

		if (!item.bFinal)
			continue;

		unsigned int group;

		if ((item.TypeId >> 26) == 1)
			group = item.TypeId & 0x3ffffff;
		else if ((item.TypeId >> 26) == 2 && (item.TypeId & 0x600) == 0)
			group = (item.TypeId >> 18) & 0xff;
		else
			continue;

		if (m_AddonPartMap983.find(group) == m_AddonPartMap983.end())
		{
			m_AddonPartMap983.insert(
				std::map<unsigned int, std::multimap<unsigned int, IFF983::sAddonPart> >::value_type(
					group, std::multimap<unsigned int, IFF983::sAddonPart>()));
		}

		m_AddonPartMap983[group].insert(
			std::multimap<unsigned int, IFF983::sAddonPart>::value_type(item.TypeId, item));
	}
}

// MemorialShopRareItem.iff e MemorialShopRareItem.sff (00a32b50): sem teste de bFinal
void CItemManager::MakeMemorialShopRareItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sMemorialShopRareItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_MemorialShopRareItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sMemorialShopRareItem));
		p += sizeof(IFF983::sMemorialShopRareItem);

		m_MemorialShopRareItem983.push_back(item);
	}
}

// MemorialShopCoinItem.sff (00a32850): sem teste de bFinal
void CItemManager::MakeMemorialShopCoinItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sMemorialShopCoinItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_MemorialShopCoinItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sMemorialShopCoinItem));
		p += sizeof(IFF983::sMemorialShopCoinItem);

		m_MemorialShopCoinItem983.push_back(item);
	}
}

// MemorialShopNormalItem.sff (00a32a10): sem teste de bFinal
void CItemManager::MakeMemorialShopNormalItem983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sMemorialShopNormalItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_MemorialShopNormalItem983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sMemorialShopNormalItem));
		p += sizeof(IFF983::sMemorialShopNormalItem);

		m_MemorialShopNormalItem983.push_back(item);
	}
}

// CharacterMastery.iff (00a32d70): sem teste de bFinal
void CItemManager::MakeCharacterMastery983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sCharacterMastery item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_CharacterMastery983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sCharacterMastery));
		p += sizeof(IFF983::sCharacterMastery);

		m_CharacterMastery983.push_back(item);
	}
}

// CaddieVoiceTable.iff (00a32e20): sem teste de bFinal; registro de 0x86 sem alinhamento
void CItemManager::MakeCaddieVoiceTable983(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF983::sCaddieVoice item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	m_CaddieVoice983.clear();

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF983::sCaddieVoice));
		p += sizeof(IFF983::sCaddieVoice);

		m_CaddieVoice983.push_back(item);
	}
}

// 00a2f1a0: nome do pacote de tabelas ("PangYa_JP" + "." + extensão)
std::string CItemManager::GetPackName983(const char* ext)
{
	std::stringstream path;

	path << "" << "PangYa_JP" << "." << ext;

	return path.str();
}

IFF_STRUCT::sChar* CItemManager::FindChar(unsigned long tid)
{
	IFF_STRUCT::sChar* pChar = NULL;
	std::map<unsigned int, IFF_STRUCT::sChar>::iterator it = m_CharMap.find(tid);

	if (it != m_CharMap.end())
		pChar = &(*it).second;

	return pChar;
}

IFF_STRUCT::sPart* CItemManager::FindPart(unsigned long tid)
{
	IFF_STRUCT::sPart* pPart = NULL;
	std::map<unsigned int, IFF_STRUCT::sPart>::iterator it = m_PartMap.find(tid);

	if (it != m_PartMap.end())
		pPart = &(*it).second;

	return pPart;
}

// 983 00a193b0: o grupo do AddonPart.iff de um tid (a mesma regra do MakeAddonPartMap983, sem o teste de 0x600);
// usado pelo CPetFrame::SetPartPets (sub-pecas do personagem)
std::multimap<unsigned int, IFF983::sAddonPart>* CItemManager::FindAddonPartGroup983(unsigned long tid)
{
	unsigned long type = tid >> 26;
	if (type == 1)
		tid &= 0x3ffffff;
	else if (type == 2)
		tid = (tid >> 18) & 0xff;
	else
		return NULL;

	std::multimap<unsigned int, IFF983::sAddonPart>* pGroup = NULL;
	std::map<unsigned int, std::multimap<unsigned int, IFF983::sAddonPart> >::iterator it = m_AddonPartMap983.find(tid);
	if (it != m_AddonPartMap983.end())
		pGroup = &(*it).second;
	return pGroup;
}

IFF_STRUCT::sClub* CItemManager::FindClub(unsigned long tid)
{
	IFF_STRUCT::sClub* pClub = NULL;
	std::map<unsigned int, IFF_STRUCT::sClub>::iterator it = m_ClubMap.find(tid);

	if (it != m_ClubMap.end())
		pClub = &(*it).second;

	return pClub;
}

IFF_STRUCT::sClubSet* CItemManager::FindClubSet(unsigned long tid)
{
	IFF_STRUCT::sClubSet* pClubSet = NULL;
	std::map<unsigned int, IFF_STRUCT::sClubSet>::iterator it = m_ClubSetMap.find(tid);

	if (it != m_ClubSetMap.end())
		pClubSet = &(*it).second;

	return pClubSet;
}

IFF_STRUCT::sBall* CItemManager::FindBall(unsigned long tid)
{
	IFF_STRUCT::sBall* pBall = NULL;
	std::map<unsigned int, IFF_STRUCT::sBall>::iterator it = m_BallMap.find(tid);

	if (it != m_BallMap.end())
		pBall = &(*it).second;

	return pBall;
}

IFF_STRUCT::sItem* CItemManager::FindItem(unsigned long tid)
{
	IFF_STRUCT::sItem* pItem = NULL;
	std::map<unsigned int, IFF_STRUCT::sItem>::iterator it = m_ItemMap.find(tid);

	if (it != m_ItemMap.end())
		pItem = &(*it).second;

	return pItem;
}

IFF_STRUCT::sCaddie* CItemManager::FindCaddie(unsigned long tid)
{
	IFF_STRUCT::sCaddie* pCaddie = NULL;
	std::map<unsigned int, IFF_STRUCT::sCaddie>::iterator it = m_CaddieMap.find(tid);

	if (it != m_CaddieMap.end())
		pCaddie = &(*it).second;

	return pCaddie;
}

IFF_STRUCT::sCadItem* CItemManager::FindCadItem(unsigned long tid)
{
	IFF_STRUCT::sCadItem* pCadItem = NULL;
	std::map<unsigned int, IFF_STRUCT::sCadItem>::iterator it = m_CadItemMap.find(tid);

	if (it != m_CadItemMap.end())
		pCadItem = &(*it).second;

	return pCadItem;
}

IFF_STRUCT::sSetItem* CItemManager::FindSetItem(unsigned long tid)
{
	IFF_STRUCT::sSetItem* pSetItem = NULL;
	std::map<unsigned int, IFF_STRUCT::sSetItem>::iterator it = m_SetItemMap.find(tid);

	if (it != m_SetItemMap.end())
		pSetItem = &(*it).second;

	return pSetItem;
}

IFF_STRUCT::sSetItem* CItemManager::FindSetItemAtConstruct(unsigned long tid)
{
	for (std::map<unsigned int, IFF_STRUCT::sSetItem>::iterator it =
			 m_SetItemMap.begin();
		it != m_SetItemMap.end(); ++it)
	{
		IFF_STRUCT::sSetItem* pSetItem = &(*it).second;
		if (pSetItem)
		{
			for (int i = 0; i < pSetItem->nElems; i++)
			{
				if (pSetItem->ElemIds[i] == tid)
					return pSetItem;
			}
		}
	}

	return NULL;
}

IFF_STRUCT::sCourse* CItemManager::FindCourse(unsigned long tid)
{
	IFF_STRUCT::sCourse* pCourse = NULL;
	std::map<unsigned int, IFF_STRUCT::sCourse>::iterator it = m_CourseMap.find(tid);

	if (it != m_CourseMap.end())
		pCourse = &(*it).second;

	return pCourse;
}

IFF_STRUCT::sMatch* CItemManager::FindMatch(unsigned long tid)
{
	IFF_STRUCT::sMatch* pMatch = NULL;
	std::map<unsigned int, IFF_STRUCT::sMatch>::iterator it = m_MatchMap.find(tid);

	if (it != m_MatchMap.end())
		pMatch = &(*it).second;

	return pMatch;
}

IFF_STRUCT::sTitle* CItemManager::FindTitle(unsigned long tid)
{
	IFF_STRUCT::sTitle* pTitle = NULL;
	std::map<unsigned int, IFF_STRUCT::sTitle>::iterator it = m_TitleMap.find(tid);

	if (it != m_TitleMap.end())
		pTitle = &(*it).second;

	return pTitle;
}

IFF_STRUCT::sEnchant* CItemManager::FindEnchant(unsigned long tid)
{
	IFF_STRUCT::sEnchant* pEnchant = NULL;
	std::map<unsigned int, IFF_STRUCT::sEnchant>::iterator it = m_EnchantMap.find(tid);

	if (it != m_EnchantMap.end())
		pEnchant = &(*it).second;

	return pEnchant;
}

IFF_STRUCT::sSkin* CItemManager::FindSkin(unsigned long tid)
{
	IFF_STRUCT::sSkin* pSkin = NULL;
	std::map<unsigned int, IFF_STRUCT::sSkin>::iterator it = m_SkinMap.find(tid);

	if (it != m_SkinMap.end())
		pSkin = &(*it).second;

	return pSkin;
}

IFF_STRUCT::sRandomBox* CItemManager::FindRandomBox(unsigned long typeId)
{
	IFF_STRUCT::sRandomBox* pBox = NULL;

	_private::sRandomBox* pRandomBox = findRandomBox(typeId);
	if (pRandomBox)
	{
		pBox = &pRandomBox->box;
	}

	return pBox;
}

void CItemManager::MakeCutinInforMationTableMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::S5::sCutinInformation sItem;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memmove(&sItem, p, sizeof(IFF_STRUCT::S5::sCutinInformation));
		p += sizeof(IFF_STRUCT::S5::sCutinInformation);

		if (sItem.bFinal)
		{
			m_CutinInformationMap.insert(
				std::map<unsigned int, IFF_STRUCT::S5::sCutinInformation>::
					value_type(sItem.NormalTypeId, sItem));
		}
	}
}

IFF_STRUCT::S5::sCutinInformation* CItemManager::FindCutinInformation(
	unsigned long tid)
{
	IFF_STRUCT::S5::sCutinInformation* pCutin = NULL;
	std::map<unsigned int, IFF_STRUCT::S5::sCutinInformation>::iterator it = m_CutinInformationMap.find(tid);

	if (it != m_CutinInformationMap.end())
		pCutin = &(*it).second;

	return pCutin;
}

IFF_STRUCT::sHairStyle* CItemManager::FindHairStyle(unsigned long tid)
{
	IFF_STRUCT::sHairStyle* pHairStyle = NULL;
	std::map<unsigned int, IFF_STRUCT::sHairStyle>::iterator it = m_HairStyleMap.find(tid);

	if (it != m_HairStyleMap.end())
		pHairStyle = &(*it).second;

	return pHairStyle;
}

IFF_STRUCT::sHairStyle* CItemManager::FindHairStyle(unsigned char charID,
	unsigned char hairID)
{
	IFF_STRUCT::sHairStyle* pHairStyle = NULL;
	std::map<unsigned int, IFF_STRUCT::sHairStyle>::iterator it;

	for (it = m_HairStyleMap.begin(); it != m_HairStyleMap.end(); ++it)
	{
		if ((*it).second.cCharID == charID &&
			(*it).second.cHairID == hairID)
		{
			pHairStyle = &(*it).second;
			break;
		}
	}

	return pHairStyle;
}

IFF_STRUCT::sChildItem* CItemManager::FindChildItem(unsigned long tid,
	int iIndex)
{
	IFF_STRUCT::sChildItem* pChildItem = NULL;
	int iNum = 0;
	std::map<unsigned int, IFF_STRUCT::sChildItem>::iterator it;

	for (it = m_ChildItemMap.begin(); it != m_ChildItemMap.end(); ++it)
	{
		pChildItem = &(*it).second;

		if (pChildItem)
		{
			if (pChildItem->TypeId == tid)
			{
				if (iIndex == iNum)
					break;

				iNum++;
			}
		}
	}

	return pChildItem;
}

int CItemManager::FindChildItemNumber(unsigned long tid)
{
	int count = 0;

	for (std::map<unsigned int, IFF_STRUCT::sChildItem>::iterator it =
			 m_ChildItemMap.begin();
		it != m_ChildItemMap.end(); ++it)
	{
		IFF_STRUCT::sChildItem* pChildItem = &(*it).second;

		if (pChildItem)
		{
			if (pChildItem->TypeId == tid)
				count++;
		}
	}

	return count;
}

IFF_STRUCT::sAuxPart* CItemManager::FindAuxPart(unsigned long tid)
{
	IFF_STRUCT::sAuxPart* pAuxPart = NULL;
	std::map<unsigned int, IFF_STRUCT::sAuxPart>::iterator it = m_AuxPartMap.find(tid);

	if (it != m_AuxPartMap.end())
		pAuxPart = &(*it).second;

	return pAuxPart;
}

IFF_STRUCT::sQuestDrop* CItemManager::FindQuestDrop(unsigned long tid)
{
	IFF_STRUCT::sQuestDrop* pQuestDrop = NULL;
	std::map<unsigned int, IFF_STRUCT::sQuestDrop>::iterator it = m_QuestDropMap.find(tid);

	if (it != m_QuestDropMap.end())
		pQuestDrop = &(*it).second;

	return pQuestDrop;
}

IFF_STRUCT::sQuest* CItemManager::FindQuest(unsigned long tid)
{
	IFF_STRUCT::sQuest* pQuest = NULL;
	std::map<unsigned int, IFF_STRUCT::sQuest>::iterator it = m_QuestMap.find(tid);

	if (it != m_QuestMap.end())
		pQuest = &(*it).second;

	return pQuest;
}

IFF_STRUCT::sMascot* CItemManager::FindMascot(unsigned long tid)
{
	std::map<unsigned int, IFF_STRUCT::sMascot>::iterator it = m_MascotMap.find(tid);

	if (it != m_MascotMap.end())
		return &(*it).second;

	return NULL;
}

IFF_STRUCT::sCard* CItemManager::FindCard(unsigned long tid)
{
	IFF_STRUCT::sCard* pCard = NULL;
	std::map<unsigned int, IFF_STRUCT::sCard>::iterator it = m_CardMap.find(tid);

	if (it != m_CardMap.end())
		pCard = &(*it).second;

	return pCard;
}

IFF_STRUCT::sFurniture* CItemManager::FindFurniture(unsigned long tid)
{
	if (!IsLocalContent(S4_REAL_MYROOM))
		return NULL;

	IFF_STRUCT::sFurniture* pFurniture = NULL;
	std::map<unsigned int, IFF_STRUCT::sFurniture>::iterator it = m_FurnitureMap.find(tid);

	if (it != m_FurnitureMap.end())
		pFurniture = &(*it).second;

	return pFurniture;
}

// JP 983: não há OfflineShop.iff nem mapa para ele (o Load 00a3de50 não o lê); fica só para os chamadores herdados.
IFF_STRUCT::sOfflineShop* CItemManager::FindOfflineShop(unsigned long tid)
{
	return NULL;
}

IFF_STRUCT::sDesc* CItemManager::FindDesc(unsigned long typeId)
{
	std::map<unsigned int, IFF_STRUCT::sDesc>::iterator it;
	it = m_DescMap.find(typeId);

	if (it == m_DescMap.end())
		return NULL;

	return &(*it).second;
}

IFF_ITEM_COMMON* CItemManager::FindCommonItem(unsigned long tid)
{
	IFF_ITEM_COMMON* pCommon = NULL;

	switch (tid >> 26)
	{
	case 1:
	{
		IFF_STRUCT::sChar* pChar = FindChar(tid);
		if (pChar)
			return &pChar->c;
	}
	break;
	case 2:
	{
		IFF_STRUCT::sPart* pPart = FindPart(tid);
		if (pPart)
			return &pPart->c;
	}
	break;
	case 3:
	{
		IFF_STRUCT::sClub* pClub = FindClub(tid);
		if (pClub)
			return &pClub->c;
	}
	break;
	case 4:
	{
		IFF_STRUCT::sClubSet* pClubSet = FindClubSet(tid);
		if (pClubSet)
			return &pClubSet->c;
	}
	break;
	case 5:
	{
		IFF_STRUCT::sBall* pBall = FindBall(tid);
		if (pBall)
			return &pBall->c;
	}
	break;
	case 6:
	{
		IFF_STRUCT::sItem* pItem = FindItem(tid);
		if (pItem)
			return &pItem->c;
	}
	break;
	case 7:
	{
		IFF_STRUCT::sCaddie* pCaddie = FindCaddie(tid);
		if (pCaddie)
			return &pCaddie->c;
	}
	break;
	case 8:
	{
		IFF_STRUCT::sCadItem* pCadItem = FindCadItem(tid);
		if (pCadItem)
			return &pCadItem->c;
	}
	break;
	case 9:
	{
		IFF_STRUCT::sSetItem* pSetItem = FindSetItem(tid);
		if (pSetItem)
			return &pSetItem->c;
	}
	break;
	case 10:
	{
		IFF_STRUCT::sCourse* pCourse = FindCourse(tid);
		if (pCourse)
			return &pCourse->c;
	}
	break;
	case 14:
	{
		IFF_STRUCT::sSkin* pSkin = FindSkin(tid);
		if (pSkin)
			return &pSkin->c;
	}
	break;
	case 15:
	{
		IFF_STRUCT::sHairStyle* pHairStyle = FindHairStyle(tid);
		if (pHairStyle)
			return &pHairStyle->c;
	}
	break;
	case 16:
	{
		pCommon = (IFF_ITEM_COMMON*)FindMascot(tid);
	}
	break;
	case 17:
		return NULL;
	case 28:
	{
		IFF_STRUCT::sAuxPart* pAuxPart = FindAuxPart(tid);
		if (pAuxPart)
			return &pAuxPart->c;
	}
	break;
	// JP 983 (00a17cf0): 0x1b = CounterItem, 0x1d = QuestStuff, 0x13/0x1e = QuestItem; o registro tem TypeId em +4 e Name
	// em +8 como o IFF_ITEM_COMMON (o premio da missao diaria 0x6c000001 sai daqui; NULL derrubava o SetDone983)
	case 27:
		pCommon = (IFF_ITEM_COMMON*)FindCounterItem983(tid);
	break;
	case 29:
		pCommon = (IFF_ITEM_COMMON*)FindQuestStuff983(tid);
	break;
	case 30:
		pCommon = (IFF_ITEM_COMMON*)FindQuestItem983(tid);
	break;
	case 31:
	{
		IFF_STRUCT::sCard* pCard = FindCard(tid);
		if (pCard)
			return &pCard->c;
	}
	break;
	case 18:
	{
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			IFF_STRUCT::sFurniture* pFurniture = FindFurniture(tid);
			if (pFurniture)
				return &pFurniture->c;
		}
	}
	break;
	case 19:
	{
		pCommon = (IFF_ITEM_COMMON*)FindQuestItem983(tid);
	}
	break;
	}

	if (pCommon)
		return pCommon;

	return NULL;
}

// JP 983: buscas nas tabelas novas (mesma forma do FindChar do 983: find + comparação com end).

// 00a17750
IFF983::sAchievement* CItemManager::FindAchievement983(unsigned long tid)
{
	IFF983::sAchievement* pRet = NULL;
	std::map<unsigned int, IFF983::sAchievement>::iterator it = m_AchievementMap983.find(tid);

	if (m_AchievementMap983.end() != it)
		pRet = &(*it).second;

	return pRet;
}

// 00a17660
IFF983::sCounterItem* CItemManager::FindCounterItem983(unsigned long tid)
{
	IFF983::sCounterItem* pRet = NULL;
	std::map<unsigned int, IFF983::sCounterItem>::iterator it = m_CounterItemMap983.find(tid);

	if (m_CounterItemMap983.end() != it)
		pRet = &(*it).second;

	return pRet;
}

// 00a176b0
IFF983::sQuestStuff* CItemManager::FindQuestStuff983(unsigned long tid)
{
	IFF983::sQuestStuff* pRet = NULL;
	std::map<unsigned int, IFF983::sQuestStuff>::iterator it = m_QuestStuffMap983.find(tid);

	if (m_QuestStuffMap983.end() != it)
		pRet = &(*it).second;

	return pRet;
}

// 00a17700
IFF983::sQuestItem* CItemManager::FindQuestItem983(unsigned long tid)
{
	IFF983::sQuestItem* pRet = NULL;
	std::map<unsigned int, IFF983::sQuestItem>::iterator it = m_QuestItemMap983.find(tid);

	if (m_QuestItemMap983.end() != it)
		pRet = &(*it).second;

	return pRet;
}

// 00a17610
IFF983::sErrorCodeInfo* CItemManager::FindErrorCodeInfo983(unsigned long tid)
{
	IFF983::sErrorCodeInfo* pRet = NULL;
	std::map<unsigned int, IFF983::sErrorCodeInfo>::iterator it = m_ErrorCodeInfoMap983.find(tid);

	if (m_ErrorCodeInfoMap983.end() != it)
		pRet = &(*it).second;

	return pRet;
}

// 00a17b00
IFF983::sGrandPrixData* CItemManager::FindGrandPrixData983(unsigned long tid)
{
	IFF983::sGrandPrixData* pRet = NULL;
	std::map<unsigned int, IFF983::sGrandPrixData>::iterator it = m_GrandPrixDataMap983.find(tid);

	if (it != m_GrandPrixDataMap983.end())
		pRet = &(*it).second;

	return pRet;
}

// 00a17b50
std::vector<IFF983::sGrandPrixSpecialHole>* CItemManager::FindGrandPrixSpecialHole983(unsigned long tid)
{
	std::map<unsigned int, std::vector<IFF983::sGrandPrixSpecialHole> >::iterator it = m_GrandPrixSpecialHoleMap983.find(tid);

	if (it == m_GrandPrixSpecialHoleMap983.end())
		return NULL;

	return &(*it).second;
}

// 00a17b90
std::vector<IFF983::sGrandPrixConditionEquip>* CItemManager::FindGrandPrixConditionEquip983(unsigned long tid)
{
	std::map<unsigned int, std::vector<IFF983::sGrandPrixConditionEquip> >::iterator it = m_GrandPrixConditionEquipMap983.find(tid);

	if (it == m_GrandPrixConditionEquipMap983.end())
		return NULL;

	return &(*it).second;
}

// 00a17bd0
std::vector<IFF983::sGrandPrixRankReward>* CItemManager::FindGrandPrixRankReward983(unsigned long tid)
{
	std::map<unsigned int, std::vector<IFF983::sGrandPrixRankReward> >::iterator it = m_GrandPrixRankRewardMap983.find(tid);

	if (it == m_GrandPrixRankRewardMap983.end())
		return NULL;

	return &(*it).second;
}

// 00a17c10
bool CItemManager::HasGrandPrixSpecialHole983(unsigned long tid)
{
	return FindGrandPrixSpecialHole983(tid) != NULL;
}

// 00a177a0
IFF983::sAbility* CItemManager::FindAbility983(unsigned long tid)
{
	IFF983::sAbility* pRet = NULL;
	std::map<unsigned int, IFF983::sAbility>::iterator it = m_AbilityMap983.find(tid);

	if (it != m_AbilityMap983.end())
		pRet = &(*it).second;

	return pRet;
}

// 00a177f0
IFF983::sSetEffectTable* CItemManager::FindSetEffectTable983(unsigned long tid)
{
	IFF983::sSetEffectTable* pRet = NULL;
	std::map<unsigned int, IFF983::sSetEffectTable>::iterator it = m_SetEffectTableMap983.find(tid);

	if (it != m_SetEffectTableMap983.end())
		pRet = &(*it).second;

	return pRet;
}

// 00a17520
IFF983::sShopLimitItem* CItemManager::FindShopLimitItem983(unsigned long tid)
{
	IFF983::sShopLimitItem* pRet = NULL;
	std::map<unsigned int, IFF983::sShopLimitItem>::iterator it = m_ShopLimitItemMap983.find(tid);

	if (it != m_ShopLimitItemMap983.end())
		pRet = &(*it).second;

	return pRet;
}

// 00a175c0: a chave é o nível (um byte)
IFF983::sLevelUpPrizeItem* CItemManager::FindLevelUpPrizeItem983(unsigned char level)
{
	IFF983::sLevelUpPrizeItem* pRet = NULL;
	std::map<unsigned int, IFF983::sLevelUpPrizeItem>::iterator it = m_LevelUpPrizeItemMap983.find(level);

	if (it != m_LevelUpPrizeItemMap983.end())
		pRet = &(*it).second;

	return pRet;
}

// 00a17570
IFF983::sPointShop* CItemManager::FindPointShop983(unsigned long tid)
{
	IFF983::sPointShop* pRet = NULL;
	std::map<unsigned int, IFF983::sPointShop>::iterator it = m_PointShopMap983.find(tid);

	if (it != m_PointShopMap983.end())
		pRet = &(*it).second;

	return pRet;
}

// 00a17cb0
IFF983::sGrandPrixAIOptionalData* CItemManager::FindGrandPrixAI983(unsigned long tid)
{
	std::map<unsigned int, IFF983::sGrandPrixAIOptionalData>::iterator it = m_GrandPrixAIMap983.find((unsigned short)tid);

	if (it == m_GrandPrixAIMap983.end())
		return NULL;

	return &(*it).second;
}

// 00a19320
const char* CItemManager::FindArtifactDesc983(unsigned long tid)
{
	std::map<unsigned int, IFF983::sArtifactManaInfo>::iterator it = m_ArtifactManaInfoMap983.find(tid);

	if (it == m_ArtifactManaInfoMap983.end())
		return NULL;

	return (const char*)&(*it).second + 0x0c;
}

// 00a19360
int CItemManager::GetArtifactValue983(unsigned long tid)
{
	std::map<unsigned int, IFF983::sArtifactManaInfo>::iterator it = m_ArtifactManaInfoMap983.find(tid);

	if (it == m_ArtifactManaInfoMap983.end())
		return -1;

	return *(int*)((char*)&(*it).second + 0x90);
}

// JP 983 (009d9af0): guarda o pedido do bônus de conjunto; +0x580 não é gravado.
void CItemManager::SetRangeQuery983(const unsigned long* pParts, const unsigned long* pAuxParts, unsigned long club,
	unsigned long caddie, unsigned long mascot)
{
	m_pSetParts983 = pParts;
	m_pSetAuxParts983 = pAuxParts;
	m_setClub983 = club;
	m_setCaddie983 = caddie;
	m_setMascot983 = mascot;
}

// JP 983 (009d9b30): o tid está entre as 5 peças auxiliares do pedido?
bool CItemManager::HasSetAuxPart983(unsigned long tid)
{
	const unsigned long* p = m_pSetAuxParts983;
	if (p == NULL)
		return false;

	for (int i = 0; i < 5; i++, p++)
	{
		if (*p != 0 && *p == tid)
			return true;
	}

	return false;
}

// JP 983 (009d9b80): o tid está entre as 24 peças do pedido?
bool CItemManager::HasSetPart983(unsigned long tid)
{
	const unsigned long* p = m_pSetParts983;
	if (p == NULL)
		return false;

	for (int i = 0; i < 24; i++, p++)
	{
		if (*p != 0 && *p == tid)
			return true;
	}

	return false;
}

// JP 983 (009d9bd0, 009d9bf0, 009d9c10)
bool CItemManager::IsSetClub983(unsigned long tid)
{
	if (m_setClub983 == 0)
		return false;

	return m_setClub983 == tid;
}

bool CItemManager::IsSetCaddie983(unsigned long tid)
{
	if (m_setCaddie983 == 0)
		return false;

	return m_setCaddie983 == tid;
}

bool CItemManager::IsSetMascot983(unsigned long tid)
{
	if (m_setMascot983 == 0)
		return false;

	return m_setMascot983 == tid;
}

// JP 983 (009e9bf0): a peça `tid` do conjunto está no pedido? O grupo do item (tid >> 26) escolhe onde procurar.
bool CItemManager::HasSetPiece983(unsigned long tid)
{
	switch (tid >> 26)
	{
	case PIG_PART:
		return HasSetPart983(tid);
	case PIG_CLUBSET:
		return IsSetClub983(tid);
	case PIG_CADDIE:
		return IsSetCaddie983(tid);
	case PIG_MASCOT:
		return IsSetMascot983(tid);
	case PIG_AUXPART:
		return HasSetAuxPart983(tid);
	}

	return false;
}

// JP 983 (00a18a80): acha o 1º registro do SetEffectTable.iff cujas peças marcadas estão todas no pedido e o deixa em
// m_pSetEffect983 (NULL se nenhum). Registro sem nenhuma marca é pulado.
void CItemManager::CalcSetEffect983()
{
	for (std::map<unsigned int, IFF983::sSetEffectTable>::iterator it = m_SetEffectTableMap983.begin();
		it != m_SetEffectTableMap983.end(); ++it)
	{
		m_pSetEffect983 = &(*it).second;

		if (m_pSetEffect983->flag[0] || m_pSetEffect983->flag[1] || m_pSetEffect983->flag[2] ||
			m_pSetEffect983->flag[3] || m_pSetEffect983->flag[4])
		{
			unsigned long tids[5];
			tids[0] = (*it).second.tidPiece[0];
			tids[1] = (*it).second.tidPiece[1];
			tids[2] = (*it).second.tidPiece[2];
			tids[3] = (*it).second.tidPiece[3];
			tids[4] = (*it).second.tidPiece[4];

			for (unsigned int i = 0; i < 5; i++)
			{
				if (tids[i] != 0 && HasSetPiece983(tids[i]) == true)
					tids[i] = 0;
			}

			if (tids[0] + tids[1] + tids[2] + tids[3] + tids[4] == 0)
				return;
		}
	}

	m_pSetEffect983 = NULL;
}

// JP 983 (009d9a70): alcance do registro do SetEffectTable.iff escolhido pelo CalcSetEffect983
short CItemManager::GetSetRange983()
{
	if (m_pSetEffect983)
		return m_pSetEffect983->range;

	return 0;
}

bool CItemManager::GetSetItemElem(IFF_STRUCT::sSetItem* pSetItem, int index,
	unsigned long& typeId, unsigned long& count, unsigned long& type)
{
	if (pSetItem == NULL)
		return false;

	if (index >= pSetItem->nElems)
		return false;

	typeId = pSetItem->ElemIds[index];
	switch (typeId >> 26)
	{
	case 8:
	{
		IFF_STRUCT::sCadItem* pCadItem = ItemManager()->FindCadItem(typeId);
		if (pCadItem == NULL)
			return false;

		type = pCadItem->c.Time;
		count = 1;
	}
	break;
	case 16:
	{
		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(typeId);
		if (pMascot == NULL)
			return false;

		type = pMascot->c.Time;
		count = 1;
	}
	break;
	case 14:
	{
		IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(typeId);
		if (pSkin == NULL)
			return false;

		type = pSkin->c.Time;
		count = 1;
	}
	break;
	case 6:
	{
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(typeId);
		if (pItem == NULL)
			return false;

		type = pItem->c.Time;
		count = pItem->COM[0];
	}
	break;
	case 5:
	{
		IFF_STRUCT::sBall* pBall = ItemManager()->FindBall(typeId);
		if (pBall == NULL)
			return false;

		type = pBall->c.Time;
		count = pBall->COM[0];
	}
	break;
	case 4:
	{
		IFF_STRUCT::sClubSet* pClubSet = ItemManager()->FindClubSet(typeId);
		if (pClubSet == NULL)
			return false;

		type = pClubSet->c.Time;
		count = 1;
	}
	break;
	case 2:
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(typeId);
		if (pPart == NULL)
			return false;

		type = pPart->c.Time;
		count = 1;
	}
	break;
	case 1:
	{
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(typeId);
		if (pChar == NULL)
			return false;

		type = 0;
		count = 1;
	}
	break;
	case 31:
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(typeId);
		if (pCard == NULL)
			return false;

		type = 0;
		count = pCard->COM[0];
	}
	break;
	case 9:
		return false;
	default:
		type = 0;
		count = 1;
		break;
	}

	return true;
}

bool CItemManager::GetDefCombo(unsigned long tidChar,
	unsigned long* atidDefCombo)
{
	if (atidDefCombo)
	{
		IFF_STRUCT::sChar* pChar = FindChar(tidChar);
		if (pChar)
		{
			for (int i = 0; i < 24; i++)
			{
				unsigned long partTypeId =
					0x8000400 | (((tidChar << 5) | i) << 13);
				atidDefCombo[i] = FindPart(partTypeId) ? partTypeId : 0;
			}

			return true;
		}
	}

	return false;
}

bool CItemManager::GetRecycleRandomMixOutItems(unsigned long randSeqNum,
	std::list<const IFF_STRUCT::sRandomRecycle*>& ltMixOutList)
{
	ltMixOutList.clear();

	std::multimap<unsigned int, IFF_STRUCT::sRandomRecycle>::const_iterator
		lower = m_RandomRecycleMaps.lower_bound(randSeqNum);
	std::multimap<unsigned int, IFF_STRUCT::sRandomRecycle>::const_iterator
		upper = m_RandomRecycleMaps.upper_bound(randSeqNum);
	for (; lower != upper; ++lower)
		ltMixOutList.push_back(&lower->second);

	return !ltMixOutList.empty();
}

unsigned int CItemManager::GetItemPrice(unsigned long tid)
{
	IFF_ITEM_COMMON* pItem = FindCommonItem(tid);
	if (pItem)
		return pItem->Price;

	return (unsigned int)-1;
}

int CItemManager::CheckStockType(unsigned long typeId, unsigned int stock)
{
	int result = 0;

	IFF_ITEM_COMMON* pItem = FindCommonItem(typeId);
	if (pItem)
	{
		if (pItem->InStock == stock)
			result = 1;
	}

	return result;
}

bool CItemManager::IsValidBuyItemCount(unsigned long in_dwItemType,
	int in_iItemCount)
{
	if ((in_dwItemType & 0xfc000000) == 0x18000000)
	{
		IFF_STRUCT::sItem* pItem = FindItem(in_dwItemType);
		if (pItem == NULL)
			return false;

		if (pItem->c.IsCash && pItem->COM[0] > 0 &&
			in_iItemCount != pItem->COM[0])
			return false;
	}

	return true;
}

// 009dedd0: tids que o 983 nao deixa comprar (tabela de 35 em 00d41188)
static bool __fastcall IsBlockedBuyItem983(unsigned long tid)
{
	static const unsigned long tbl[35] = {
		0x08006055, 0x0800a035, 0x08044085, 0x08048025, 0x080440b3,
		0x08084044, 0x0808a025, 0x080c408b, 0x080ca023, 0x080c40b2,
		0x0810404f, 0x08108030, 0x08144084, 0x0814a02c, 0x081440bf,
		0x08184087, 0x08188020, 0x081840c7, 0x081c404b, 0x081ca01f,
		0x0820404a, 0x0820a00e, 0x08204091, 0x08244064, 0x0824a004,
		0x0824408f, 0x08284026, 0x08284027, 0x0828a012, 0x0830401b,
		0x0830401e, 0x0830a00c, 0x0838400e, 0x08384011, 0x0838a006,
	};
	for (int i = 0; i < 35; i++)
		if (tbl[i] == tid)
			return true;
	return false;
}

bool CItemManager::IsSellableItem(IFF_ITEM_COMMON* pItem, _SYSTEMTIME& sysTime)
{
	if (pItem->InStock != 0 && pItem->InStock != 5 && pItem->InStock != 6 &&
		ItemManager()->IsInSalePeriod(pItem->TypeId, sysTime))
		return true;
	return pItem->DateFlag == 2;
}

unsigned int CItemManager::IsValidBuyItem(unsigned long in_dwItemType)
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);

	// JP 983: tids da tabela 009dedd0 devolvem 20 antes de olhar o periodo de venda
	if (IsBlockedBuyItem983(in_dwItemType))
		return 20;

	if (!IsInSalePeriod(in_dwItemType, sysTime))
	{
		return 11;
	}

	IFF_ITEM_COMMON* pItem = FindCommonItem(in_dwItemType);
	if (pItem)
	{
		return pItem->InStock ? 20 : 19;
	}

	return 19;
}

unsigned int CItemManager::GetItemSalePrice(unsigned long in_dwTid,
	int in_iCount, unsigned long in_dwDayCount)
{
	unsigned long dwSalesPrice = (unsigned long)-1;

	IFF_ITEM_COMMON* pItem = FindCommonItem(in_dwTid);
	if (!pItem)
		return dwSalesPrice;

	// JP 983 (00a1f680): so os grupos 6, 8, 14 e 16 (este com o byte +0x82 ligado) usam a tabela de dias;
	// o resto (inclusive 18) devolve o preco comum (009d8ac0). O 983 ainda le a hora do servidor e testa
	// IsInSalePeriod quando DateFlag == 2, mas descarta o resultado.
	unsigned long itemType = in_dwTid >> 26;
	bool special = false;
	switch (itemType)
	{
	case 6:
	case 8:
	case 14:
		special = true;
		break;
	case 16:
		special = ((unsigned char*)pItem)[0x82] != 0;
		break;
	}

	unsigned int unitPrice = (pItem->SalePrice > 0 && pItem->SalePrice < pItem->Price) ? pItem->SalePrice : pItem->Price;
	if (!special)
		return unitPrice;

	int sPrice[5] = { 0, 0, 0, 0, 0 };

	if (itemType == 6 && (in_dwTid & 0x1f00000) == 0x100000)
	{
		IFF_STRUCT::sItem* pItem2 = FindItem(in_dwTid);
		if (pItem2 == NULL)
			return (unsigned int)-1;

		switch (in_dwDayCount)
		{
		case 1:
			return pItem2->COM[0];
		case 7:
			return pItem2->COM[1];
		case 15:
			return pItem2->COM[2];
		case 30:
			return pItem2->COM[3];
		case 365:
			return pItem2->COM[4];
		}
		return (unsigned int)-1;
	}

	switch (itemType)
	{
	case 6:
	{
		IFF_STRUCT::sItem* pItem2 = FindItem(in_dwTid);

		int num;
		if (pItem2->COM[0] > 0)
			num = in_iCount / pItem2->COM[0];
		else
			num = in_iCount;

		return num * unitPrice;
	}
	case 14:
	{
		if (S5::ISCutinItem(in_dwTid))
			return unitPrice;

		if ((in_dwTid & 0x3c00000) == 0x1800000)
			return unitPrice;

		IFF_STRUCT::sSkin* pSkin = FindSkin(in_dwTid);
		if (pSkin == NULL)
			return (unsigned int)-1;

		for (int i = 0; i < 5; i++)
			sPrice[i] = pSkin->COM[i];
	}
	break;
	case 8:
	{
		IFF_STRUCT::sCadItem* pCadItem = FindCadItem(in_dwTid);
		if (pCadItem == NULL)
			return (unsigned int)-1;

		for (int i = 0; i < 5; i++)
			sPrice[i] = pCadItem->COM[i];

		sPrice[1] = -1;
	}
	break;
	case 16:
	{
		IFF_STRUCT::sMascot* pMascot = FindMascot(in_dwTid);
		if (pMascot == NULL)
			return (unsigned int)-1;

		for (int i = 0; i < 5; i++)
			sPrice[i] = pMascot->COM[i];

		sPrice[2] = -1;
	}
	break;
	}

	switch (in_dwDayCount)
	{
	case 1:
		dwSalesPrice = sPrice[0];
		break;
	case 7:
		dwSalesPrice = sPrice[1];
		break;
	case 15:
		dwSalesPrice = sPrice[2];
		break;
	case 30:
		dwSalesPrice = sPrice[3];
		break;
	case 365:
		dwSalesPrice = sPrice[4];
		break;
	default:
		dwSalesPrice = (unsigned int)-1;
		break;
	}

	return dwSalesPrice;
}

unsigned int CItemManager::GetChildItemPrice(unsigned long tid, int iIndex)
{
	IFF_STRUCT::sChildItem* pChildItem = FindChildItem(tid, iIndex);
	if (pChildItem)
	{
		if (pChildItem->NumberFlag)
			return pChildItem->NumberPrice;

		if (pChildItem->TimeFlag)
			return pChildItem->TimePrice;
	}

	return (unsigned int)-1;
}

// JP 983 (00a194f0): bit 0 do campo novo de Ball.iff (+0xc0); so para tid de bola
bool CItemManager::IsBallFlag983(unsigned long tid)
{
	if ((tid & 0xfc000000) == 0x14000000)
	{
		IFF_STRUCT::sBall* pBall = FindBall(tid);
		if (pBall)
			return (pBall->Flag983 & 1) != 0;
	}
	return false;
}

bool CItemManager::IsCashItem(unsigned long tid)
{
	switch (tid >> 26)
	{
	case 1:
	{
		IFF_STRUCT::sChar* pChar = FindChar(tid);
		if (pChar)
			return pChar->c.IsCash;
	}
	break;
	case 2:
	{
		IFF_STRUCT::sPart* pPart = FindPart(tid);
		if (pPart)
			return pPart->c.IsCash;
	}
	break;
	case 3:
	{
		IFF_STRUCT::sClub* pClub = FindClub(tid);
		if (pClub)
			return pClub->c.IsCash;
	}
	break;
	case 4:
	{
		IFF_STRUCT::sClubSet* pClubSet = FindClubSet(tid);
		if (pClubSet)
			return pClubSet->c.IsCash;
	}
	break;
	case 5:
	{
		IFF_STRUCT::sBall* pBall = FindBall(tid);
		if (pBall)
			return pBall->c.IsCash;
	}
	break;
	case 6:
	{
		IFF_STRUCT::sItem* pItem = FindItem(tid);
		if (pItem)
			return pItem->c.IsCash;
	}
	break;
	case 7:
	{
		IFF_STRUCT::sCaddie* pCaddie = FindCaddie(tid);
		if (pCaddie)
			return pCaddie->c.IsCash;
	}
	break;
	case 8:
	{
		IFF_STRUCT::sCadItem* pCadItem = FindCadItem(tid);
		if (pCadItem)
			return pCadItem->c.IsCash;
	}
	break;
	case 9:
	{
		IFF_STRUCT::sSetItem* pSetItem = FindSetItem(tid);
		if (pSetItem)
			return pSetItem->c.IsCash;
	}
	break;
	case 14:
	{
		IFF_STRUCT::sSkin* pSkin = FindSkin(tid);
		if (pSkin)
			return pSkin->c.IsCash;
	}
	break;
	case 15:
	{
		IFF_STRUCT::sHairStyle* pHairStyle = FindHairStyle(tid);
		if (pHairStyle)
			return pHairStyle->c.IsCash;
	}
	break;
	case 16:
	{
		IFF_STRUCT::sMascot* pMascot = FindMascot(tid);
		if (pMascot)
			return pMascot->c.IsCash;
	}
	break;
	case 28:
	{
		IFF_STRUCT::sAuxPart* pAuxPart = FindAuxPart(tid);
		if (pAuxPart)
			return pAuxPart->c.IsCash;
	}
	break;
	// JP 983 (00a18250): grupos 29 (QuestDrop) e 19 (OfflineShop) caem no default (true)
	case 31:
	{
		IFF_STRUCT::sCard* pCard = FindCard(tid);
		if (pCard)
			return pCard->c.IsCash;
	}
	break;
	case 18:
	{
		if (IsLocalContent(S4_REAL_MYROOM))
		{
			IFF_STRUCT::sFurniture* pFurniture = FindFurniture(tid);
			if (pFurniture)
				return pFurniture->c.IsCash;
		}
	}
	break;
	}

	return true;
}

bool CItemManager::IsCashChildItem(unsigned long tid, int iIndex)
{
	IFF_STRUCT::sChildItem* pChildItem = FindChildItem(tid, iIndex);
	if (pChildItem)
		return pChildItem->IsCash;

	return true;
}

// 983 (pedido B81, s102): ClubSetWorkShopRankUpExp.iff, registro de 0x1c = +0 typeId, +4 exp por rank (dwords)
bool CItemManager::GetWorkShopRankUpExp983(unsigned long typeId, int rank, int* pExp) // 00a07030
{
	std::vector<IFF983::sClubSetWorkShopRankUpExp>::iterator it = std::find_if(m_WorkShopRankUpExp983.begin(),
		m_WorkShopRankUpExp983.end(), [typeId](const IFF983::sClubSetWorkShopRankUpExp& r) {
			return *(const unsigned long*)r.raw == typeId;
		});
	if (it == m_WorkShopRankUpExp983.end())
		return false;

	*pExp = ((int*)(it->raw + 4))[rank];
	return true;
}

// 983 (s102): soma (sem zerar *pSum) a exp dos ranks first..last do registro de typeId
bool CItemManager::SumWorkShopRankUpExp983(unsigned long typeId, int first, int last, int* pSum) // 00a07070
{
	std::vector<IFF983::sClubSetWorkShopRankUpExp>::iterator it = std::find_if(m_WorkShopRankUpExp983.begin(),
		m_WorkShopRankUpExp983.end(), [typeId](const IFF983::sClubSetWorkShopRankUpExp& r) {
			return *(const unsigned long*)r.raw == typeId;
		});
	if (it == m_WorkShopRankUpExp983.end())
		return false;

	for (int i = first; i <= last; i++)
		*pSum += ((int*)(it->raw + 4))[i];
	return true;
}

bool CItemManager::IsTimeLimit(unsigned long tid)
{
	unsigned long group = tid >> 26;

	switch (group)
	{
	case 6:
	{
		IFF_STRUCT::sItem* pItem = FindItem(tid);
		if (pItem == NULL)
			break;

		if (pItem->c.TimeFlag)
			return true;
	}
	break;
	case 14:
	{
		IFF_STRUCT::sSkin* pSkin = FindSkin(tid);
		if (pSkin == NULL)
			break;

		if (pSkin->c.TimeFlag == 1 || pSkin->c.TimeFlag == 2)
			return true;
	}
	break;
	}

	return false;
}

// 009de910: tids de missao antigos (0x78000000..0x78000003 e 0x78000082)
static bool __fastcall IsQuestTid983(unsigned long tid)
{
	return tid >= 0x78000000 && (tid <= 0x78000003 || tid == 0x78000082);
}

const char* CItemManager::GetItemName(unsigned long tid)
{
	if (tid != 0)
	{
		switch (tid >> 26)
		{
		case 1:
		{
			IFF_STRUCT::sChar* pChar = FindChar(tid);
			if (pChar)
				return pChar->c.Name;
		}
		break;
		case 2:
		{
			IFF_STRUCT::sPart* pPart = FindPart(tid);
			if (pPart)
				return pPart->c.Name;
		}
		break;
		case 3:
		{
			IFF_STRUCT::sClub* pClub = FindClub(tid);
			if (pClub)
				return pClub->c.Name;
		}
		break;
		case 4:
		{
			IFF_STRUCT::sClubSet* pClubSet = FindClubSet(tid);
			if (pClubSet)
				return pClubSet->c.Name;
		}
		break;
		case 5:
		{
			IFF_STRUCT::sBall* pBall = FindBall(tid);
			if (pBall)
				return pBall->c.Name;
		}
		break;
		case 6:
		{
			IFF_STRUCT::sItem* pItem = FindItem(tid);
			if (pItem)
				return pItem->c.Name;
		}
		break;
		case 7:
		{
			IFF_STRUCT::sCaddie* pCaddie = FindCaddie(tid);
			if (pCaddie)
				return pCaddie->c.Name;
		}
		break;
		case 8:
		{
			IFF_STRUCT::sCadItem* pCadItem = FindCadItem(tid);
			if (pCadItem)
				return pCadItem->c.Name;
		}
		break;
		case 9:
		{
			IFF_STRUCT::sSetItem* pSetItem = FindSetItem(tid);
			if (pSetItem)
				return pSetItem->c.Name;
		}
		break;
		case 10:
		{
			IFF_STRUCT::sCourse* pCourse = FindCourse(tid);
			if (pCourse)
				return pCourse->c.Name;
		}
		break;
		case 11:
		{
			IFF_STRUCT::sMatch* pMatch = FindMatch(tid);
			if (pMatch)
				return pMatch->Name;
		}
		break;
		case 12:
		{
			IFF_STRUCT::sTitle* pTitle = FindTitle(tid);
			if (pTitle)
				return pTitle->Name;
		}
		break;
		case 14:
		{
			IFF_STRUCT::sSkin* pSkin = FindSkin(tid);
			if (pSkin)
				return pSkin->c.Name;
		}
		break;
		case 15:
		{
			IFF_STRUCT::sHairStyle* pHairStyle = FindHairStyle(tid);
			if (pHairStyle)
				return pHairStyle->c.Name;
		}
		break;
		case 16:
		{
			IFF_STRUCT::sMascot* pMascot = FindMascot(tid);
			if (pMascot)
				return pMascot->c.Name;
		}
		break;
		case 28:
		{
			IFF_STRUCT::sAuxPart* pAuxPart = FindAuxPart(tid);
			if (pAuxPart)
				return pAuxPart->c.Name;
		}
		break;
		// JP 983 (00a18400): 27 = CounterItem, 29 = QuestStuff, 19/30 = Quest (tids 009de910) ou QuestItem; o nome
		// fica em +8 de cada registro
		case 27:
		{
			IFF983::sCounterItem* pCounter = FindCounterItem983(tid);
			if (pCounter)
				return (const char*)pCounter + 8;
		}
		break;
		case 29:
		{
			IFF983::sQuestStuff* pStuff = FindQuestStuff983(tid);
			if (pStuff)
				return (const char*)pStuff + 8;
		}
		break;
		case 19:
		case 30:
		{
			if (IsQuestTid983(tid))
			{
				IFF_STRUCT::sQuest* pQuest = FindQuest(tid);
				if (pQuest)
					return pQuest->Name;
			}
			else
			{
				IFF983::sQuestItem* pQuestItem = FindQuestItem983(tid);
				if (pQuestItem)
					return (const char*)pQuestItem + 8;
			}
		}
		break;
		case 31:
		{
			IFF_STRUCT::sCard* pCard = FindCard(tid);
			if (pCard)
				return pCard->c.Name;
		}
		break;
		case 18:
		{
			if (IsLocalContent(S4_REAL_MYROOM))
			{
				IFF_STRUCT::sFurniture* pFurniture = FindFurniture(tid);
				if (pFurniture)
					return pFurniture->c.Name;
			}
		}
		break;
		}
	}

	return "unknown";
}

bool CItemManager::HasSalePeriod(unsigned long tid, _SYSTEMTIME** pSaleS,
	_SYSTEMTIME** pSaleE)
{
	*pSaleS = NULL;
	*pSaleE = NULL;

	// JP 983 (00a18700): o item de ShopLimitItem.iff (+0x1c/+0x2c) tem prioridade sobre o periodo do item comum
	IFF_ITEM_COMMON* pItem = FindCommonItem(tid);
	if (pItem)
	{
		IFF983::sShopLimitItem* pLimit = FindShopLimitItem983(tid);
		if (pLimit)
		{
			*pSaleS = &pLimit->saleStart;
			*pSaleE = &pLimit->saleEnd;
		}
		else
		{
			*pSaleS = &pItem->saleStart;
			*pSaleE = &pItem->saleEnd;
		}
	}

	return *pSaleS != NULL ? true : false;
}

bool CItemManager::IsInSalePeriod(unsigned long tid, _SYSTEMTIME& sysTime)
{
	_SYSTEMTIME* pStart = NULL;
	_SYSTEMTIME* pEnd = NULL;

	if (!HasSalePeriod(tid, &pStart, &pEnd))
		return true;

	if (!pStart && !pEnd)
		return true;

	return IsBetween(pStart, pEnd, sysTime);
}

bool CItemManager::IsLevelLocked(IFF_ITEM_COMMON* pItem, unsigned long* pLevel)
{
	if (pItem->InStock == 3)
	{
		unsigned int lv = pItem->Level;
		if (pItem->IsUnderLvl)
		{
			if (lv < *pLevel)
				return true;
		}
		else if (lv > *pLevel + 1)
			return true;
	}
	return false;
}

int CItemManager::CompareSystemTime(_SYSTEMTIME& tm0, _SYSTEMTIME& tm1)
{
	if (tm0.wYear < tm1.wYear)
		return -1;
	if (tm0.wYear > tm1.wYear)
		return 1;

	if (tm0.wMonth < tm1.wMonth)
		return -1;
	if (tm0.wMonth > tm1.wMonth)
		return 1;

	if (tm0.wDay < tm1.wDay)
		return -1;
	if (tm0.wDay > tm1.wDay)
		return 1;

	if (tm0.wHour < tm1.wHour)
		return -1;
	if (tm0.wHour > tm1.wHour)
		return 1;

	if (tm0.wMinute < tm1.wMinute)
		return -1;
	if (tm0.wMinute > tm1.wMinute)
		return 1;

	if (tm0.wSecond < tm1.wSecond)
		return -1;
	if (tm0.wSecond > tm1.wSecond)
		return 1;

	if (tm0.wMilliseconds < tm1.wMilliseconds)
		return -1;
	if (tm0.wMilliseconds > tm1.wMilliseconds)
		return 1;

	return 0;
}

int CItemManager::GetCouponKind(unsigned long dwTid)
{
	int kind = 0;

	// 983 009d8b70: so tipo 6 com subtipo 0 (o 645 testava apenas o bit 0x2000000 em qualquer item)
	if ((dwTid & 0xfc000000) != 0x18000000 || (dwTid & 0x1f00000) != 0)
		return 0;

	if (dwTid & 0x2000000)
	{
		switch (dwTid & 0xfffff)
		{
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x1d:
		case 0x1e:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x3a:
		case 0x3c:
		case 0x4e:
			kind = 1;
			break;
		case 0x3d:
		case 0x3f:
		case 0x53:
			kind = 5;
			break;
		case 0x28:
		case 0x29:
		case 0x2a:
			kind = 2;
			break;
		case 0x30:
		case 0x33:
		case 0x34:
		case 0xa3:
		case 0x2ac: // 983 009d8bd2
			kind = 3;
			break;
		case 0x36:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0xd2:
			kind = 4;
			break;
		default:
			kind = 0;
			break;
		}
	}

	return kind;
}

// 00a18830
bool CItemManager::IsDisableDuplicateItem(unsigned long dwTid)
{
	if (dwTid & 0x2000000)
		return (dwTid & 0xfffff) == 0xf;

	if ((dwTid & 0xfc000000) == 0x8000000)
	{
		IFF_STRUCT::sPart* pPart = FindPart(dwTid);

		if (pPart && !pPart->c.IsCash)
		{
			if (pPart->c.IsSalable == 1 || pPart->c.IsSalable == 3)
				return false;
		}

		return true;
	}

	return false;
}

bool CItemManager::IsDisplayItemNumber(unsigned long dwTid)
{
	bool bDisplay = false;

	switch (dwTid >> 26)
	{
	case 6:
		if ((dwTid & 0x2000000) && (dwTid & 0xfffff) == 6) // 983 00a1fa5b
			break;

		bDisplay = true;
		break;
	case 5:
		if (IsUnlimitAztec(dwTid))
			break;

		bDisplay = true;
		break;
	case 28:
	{
		IFF_STRUCT::sAuxPart* pAuxPart = FindAuxPart(dwTid);
		if (pAuxPart == NULL)
			break;

		if (pAuxPart->COM[0] == 0)
			break;

		bDisplay = true;
	}
	break;
	case 31:
		bDisplay = true;
		break;
	}

	return bDisplay;
}

// 009d8cd0: tabela tid -> indice (1/8/10/13); 0 = nao tem. Valores medidos por emulacao do 983 (266 tids).
unsigned long CItemManager::GetIndexAbilityItem(unsigned long dwTid)
{
	// 983 009d8cd0: tabela conferida tid a tid (B1592, out/B1592/abil.py)
	switch (dwTid)
	{
	case 0x800082d:
	case 0x800082e:
	case 0x8000884:
	case 0x8000886:
	case 0x8000887:
	case 0x801a80e:
	case 0x801a817:
	case 0x801a818:
	case 0x801a828:
	case 0x801a829:
	case 0x801a82a:
	case 0x804086e:
	case 0x8050810:
	case 0x8050816:
	case 0x8050817:
	case 0x8050827:
	case 0x8050828:
	case 0x8050829:
	case 0x805a80d:
	case 0x805a80e:
	case 0x805a80f:
	case 0x805a810:
	case 0x805a823:
	case 0x805a824:
	case 0x805a834:
	case 0x8090817:
	case 0x8090824:
	case 0x8090825:
	case 0x8090834:
	case 0x8090835:
	case 0x8090836:
	case 0x809a810:
	case 0x809a811:
	case 0x809a81f:
	case 0x809a822:
	case 0x809a823:
	case 0x809a837:
	case 0x80c0033:
	case 0x80d0817:
	case 0x80d0818:
	case 0x80d0819:
	case 0x80d081a:
	case 0x80d0839:
	case 0x80d083a:
	case 0x80d084e:
	case 0x80d4819:
	case 0x80d4823:
	case 0x80d4824:
	case 0x80d4833:
	case 0x80d4834:
	case 0x80d4835:
	case 0x8100037:
	case 0x8112017:
	case 0x8112020:
	case 0x8112021:
	case 0x8112030:
	case 0x8112031:
	case 0x8112032:
	case 0x811a010:
	case 0x811a011:
	case 0x811a81e:
	case 0x811a81f:
	case 0x811a833:
	case 0x8140046:
	case 0x815a01d:
	case 0x815a01e:
	case 0x815a01f:
	case 0x815a020:
	case 0x815a064:
	case 0x815a84f:
	case 0x815a850:
	case 0x816280e:
	case 0x8162815:
	case 0x8162816:
	case 0x8162824:
	case 0x8162825:
	case 0x8162826:
	case 0x8180064:
	case 0x818e024:
	case 0x818e025:
	case 0x818e026:
	case 0x818e027:
	case 0x818e04a:
	case 0x818e04b:
	case 0x818e05e:
	case 0x8196012:
	case 0x819601c:
	case 0x819601d:
	case 0x819602c:
	case 0x819602d:
	case 0x819602e:
	case 0x81d880a:
	case 0x81d880b:
	case 0x81d881b:
	case 0x81d881f:
	case 0x81d8820:
	case 0x81d8839:
	case 0x81da814:
	case 0x81da81c:
	case 0x81da81d:
	case 0x81da82c:
	case 0x81da82d:
	case 0x81da82e:
	case 0x821203f:
	case 0x8212043:
	case 0x8212044:
	case 0x821205b:
	case 0x821a807:
	case 0x821a810:
	case 0x821a811:
	case 0x821a824:
	case 0x821a825:
	case 0x821a826:
	case 0x824001c:
	case 0x825200c:
	case 0x825200d:
	case 0x8252028:
	case 0x825a801:
	case 0x825a805:
	case 0x825a806:
	case 0x825a81c:
	case 0x825a81d:
	case 0x825a81e:
	case 0x829a817:
		return 1;
	case 0x801a813:
	case 0x801a814:
	case 0x8050812:
	case 0x8050813:
	case 0x809081f:
	case 0x8090820:
	case 0x80d481e:
	case 0x80d481f:
	case 0x811201c:
	case 0x811201d:
	case 0x8162811:
	case 0x8162812:
	case 0x8196014:
	case 0x8196015:
	case 0x81da818:
	case 0x81da819:
	case 0x821a809:
	case 0x821a80a:
	case 0x825a80f:
	case 0x825a810:
	case 0x70000005:
		return 8;
	case 0x8000828:
	case 0x8000829:
	case 0x800082a:
	case 0x800082b:
	case 0x801a806:
	case 0x801a807:
	case 0x801a80a:
	case 0x801a80c:
	case 0x801a80d:
	case 0x8050808:
	case 0x8050809:
	case 0x805080c:
	case 0x805080e:
	case 0x805080f:
	case 0x8058800:
	case 0x805880f:
	case 0x805a809:
	case 0x805a80a:
	case 0x805a80b:
	case 0x805a80c:
	case 0x809080c:
	case 0x809080d:
	case 0x8090810:
	case 0x8090815:
	case 0x8090816:
	case 0x8098800:
	case 0x809880f:
	case 0x809a80c:
	case 0x809a80d:
	case 0x809a80e:
	case 0x809a80f:
	case 0x80d0813:
	case 0x80d0814:
	case 0x80d0815:
	case 0x80d0816:
	case 0x80d480a:
	case 0x80d480b:
	case 0x80d480e:
	case 0x80d4817:
	case 0x80d4818:
	case 0x80dc800:
	case 0x80dc80f:
	case 0x811200e:
	case 0x811200f:
	case 0x8112012:
	case 0x8112015:
	case 0x8112016:
	case 0x8118800:
	case 0x811880f:
	case 0x811a00c:
	case 0x811a00d:
	case 0x811a00e:
	case 0x811a00f:
	case 0x815a018:
	case 0x815a019:
	case 0x815a01a:
	case 0x815a01b:
	case 0x8160800:
	case 0x816080f:
	case 0x8162806:
	case 0x8162807:
	case 0x816280a:
	case 0x816280c:
	case 0x816280d:
	case 0x818e01c:
	case 0x818e01d:
	case 0x818e01e:
	case 0x818e01f:
	case 0x8190800:
	case 0x819080f:
	case 0x8196008:
	case 0x8196009:
	case 0x819600d:
	case 0x8196010:
	case 0x8196011:
	case 0x81d8806:
	case 0x81d8807:
	case 0x81d8808:
	case 0x81d8809:
	case 0x81da807:
	case 0x81da808:
	case 0x81da80b:
	case 0x81da812:
	case 0x81da813:
	case 0x821a801:
	case 0x821a802:
	case 0x821a805:
	case 0x821a806:
	case 0x821a832:
	case 0x825a80c:
	case 0x825a80d:
	case 0x825a82c:
	case 0x8016800:
	case 0x801680f:
	case 0x81e2800:
	case 0x81e280f:
	case 0x8214800:
	case 0x821480f:
	case 0x8254800:
	case 0x825480f:
	case 0x8294811:
	case 0x829a805:
	case 0x829a814:
	case 0x829a815:
	case 0x829a81d:
	case 0x82d4804:
	case 0x831480c:
	case 0x839c800:
	case 0x839c80c:
		return 10;
	case 0x8026800:
	case 0x8026801:
	case 0x8026802:
	case 0x8064800:
	case 0x8064801:
	case 0x8064802:
	case 0x8064803:
	case 0x80a2800:
	case 0x80a2801:
	case 0x80a2802:
	case 0x80e4800:
	case 0x80e4801:
	case 0x80e4802:
	case 0x8122800:
	case 0x8122801:
	case 0x8122802:
	case 0x816e801:
	case 0x816e802:
	case 0x816e803:
	case 0x816e805:
	case 0x816e806:
	case 0x81a4800:
	case 0x81a4801:
	case 0x81ea800:
	case 0x8228800:
	case 0x8228801:
	case 0x8228802:
	case 0x8228803:
	case 0x8228804:
	case 0x8268800:
	case 0x82a6800:
	case 0x82e4800:
	case 0x82e4801:
	case 0x8320800:
	case 0x8320801:
	case 0x8320802:
	case 0x83a4800:
	case 0x83a4801:
	case 0x83a4802:
		return 13;
	}

	return 0;
}

// GetMascotBonusPang: inline em itemmanager.h (o 983 nao tem funcao propria: esta inlined em fontinfo)
bool CItemManager::IsBetween(_SYSTEMTIME* pStart, _SYSTEMTIME* pEnd,
	_SYSTEMTIME& sysTime)
{
	if (pStart != NULL && pEnd != NULL)
	{
		if (pStart->wYear > 0 && pEnd->wYear > 0)
		{
			if (CompareSystemTime(*pStart, sysTime) <= 0 &&
				CompareSystemTime(sysTime, *pEnd) <= 0)
				return true;
			else
				return false;
		}
		else if (pStart->wYear > 0)
		{
			return CompareSystemTime(*pStart, sysTime) <= 0 ? true : false;
		}
		else if (pEnd->wYear > 0)
		{
			return CompareSystemTime(sysTime, *pEnd) <= 0 ? true : false;
		}
	}
	else if (pStart != NULL)
	{
		return CompareSystemTime(*pStart, sysTime) <= 0 ? true : false;
	}
	else if (pEnd != NULL)
	{
		return CompareSystemTime(sysTime, *pEnd) <= 0 ? true : false;
	}

	return true;
}

int CItemManager::GetNumItems(PANGYA_ITEM_GROUP pig)
{
	int num = 0;

	switch (pig)
	{
	case PIG_CHAR:
		num = m_CharMap.size();
		break;
	case PIG_PART:
		num = m_PartMap.size();
		break;
	case PIG_CLUB:
		num = m_ClubMap.size();
		break;
	case PIG_CLUBSET:
		num = m_ClubSetMap.size();
		break;
	case PIG_BALL:
		num = m_BallMap.size();
		break;
	case PIG_ITEM:
		num = m_ItemMap.size();
		break;
	case PIG_CADDIE:
		num = m_CaddieMap.size();
		break;
	case PIG_CADITEM:
		num = m_CadItemMap.size();
		break;
	case PIG_SETITEM:
		num = m_SetItemMap.size();
		break;
	case PIG_COURSE:
		num = m_CourseMap.size();
		break;
	case PIG_MATCH:
		num = m_MatchMap.size();
		break;
	case PIG_TITLE:
		num = m_TitleMap.size();
		break;
	case PIG_SKIN:
		num = m_SkinMap.size();
		break;
	case PIG_ENCHANT:
		num = m_EnchantMap.size();
		break;
	case PIG_HAIR:
		num = m_HairStyleMap.size();
		break;
	case PIG_CHILDITEM:
		num = m_ChildItemMap.size();
		break;
	case PIG_AUXPART:
		num = m_AuxPartMap.size();
		break;
	// JP 983 (009e9a00): sem PIG_QUESTDROP (o mapa nao e mais lido)
	case PIG_QUEST:
		num = m_QuestMap.size();
		break;
	case PIG_CARD:
		num = m_CardMap.size();
		break;
	case PIG_FURNITURE:
		if (IsLocalContent(S4_REAL_MYROOM))
			num = m_FurnitureMap.size();
		break;
	case PIG_OFFLINESHOP:
		break;
	}

	return num;
}

int CItemManager::GetCourseNum()
{
	int num;

	num = m_CourseMap.size() - 3;

	return num;
}

class CItemManagerStatusJp
{
public:
	unsigned char GetSetEffectSlot983(unsigned long type, const unsigned long* tidParts,
		const unsigned long* tidAuxParts); // 00a1fad0 (jp/statushelper.cpp)
	unsigned char GetCaddieStat983(unsigned long type, IFF_STRUCT::sCaddie* pCaddie, unsigned char level); // 009d9ce0
	unsigned char GetMascotStat983(unsigned long type, IFF_STRUCT::sMascot* pMascot, unsigned char level); // 009d9d10
};

unsigned char CItemManager::GetItemLevel(eLvlType eType,
	const unsigned long* patidParts, const unsigned long* patidAuxParts)
{
	if (patidParts == NULL)
		return 0;

	unsigned char level = 0;

	IFF_STRUCT::sPart* apParts[24];
	for (int i = 0; i < 24; i++)
	{
		if (patidParts[i] == 0)
			apParts[i] = NULL;
		else
			apParts[i] = FindPart(patidParts[i]);

		if (apParts[i])
			level += (unsigned char)apParts[i]->Attr[eType];
	}

	if (patidAuxParts && eType < 5)
	{
		IFF_STRUCT::sAuxPart* apAuxParts[5];
		for (int i = 0; i < 5; i++)
		{
			if (patidAuxParts[i] == 0)
				apAuxParts[i] = NULL;
			else
				apAuxParts[i] = FindAuxPart(patidAuxParts[i]);

			if (apAuxParts[i] && apAuxParts[i]->Attr[eType] != 99)
				level += apAuxParts[i]->Attr[eType];
		}
	}

	return level;
}

unsigned char CItemManager::GetCharCapacity(eLvlType eType,
	unsigned long tidChar, const unsigned long* patidParts,
	const unsigned long* patidAuxParts, unsigned char level)
{
	unsigned char capacity;

	if (tidChar)
	{
		IFF_STRUCT::sChar* pChar = FindChar(tidChar);
		if (pChar)
			capacity = (unsigned char)pChar->Attr[eType];
		else
			capacity = 0;
	}
	else
	{
		capacity = 0;
	}

	if (eType == 0 && level >= 6)
		capacity += (level - 1) / 5;

	if (patidParts)
	{
		IFF_STRUCT::sPart* apParts[24];
		for (int i = 0; i < 24; i++)
		{
			if (patidParts[i] == 0)
				apParts[i] = NULL;
			else
				apParts[i] = FindPart(patidParts[i]);
			if (apParts[i])
				capacity += (unsigned char)apParts[i]->Slot[eType];
		}
	}

	if (patidAuxParts && eType < 5)
	{
		IFF_STRUCT::sAuxPart* apAuxParts[5];
		for (int i = 0; i < 5; i++)
		{
			if (patidAuxParts[i] == 0)
				apAuxParts[i] = NULL;
			else
				apAuxParts[i] = FindAuxPart(patidAuxParts[i]);
			if (apAuxParts[i] && apAuxParts[i]->Slot[eType] != 99)
				capacity += apAuxParts[i]->Slot[eType];
		}
	}

	// JP 983 (00a23500): bonus do conjunto (00a1fad0 = GetSetEffectSlot983) e cartas sem teste de conteudo local
	capacity += ((CItemManagerStatusJp*)this)->GetSetEffectSlot983(eType, patidParts, patidAuxParts);
	return capacity + (unsigned char)CCardManager::Instance()->GetCardStatusSlot(eType);
}

unsigned char CItemManager::GetCharLevel(eLvlType eType, unsigned long tidChar,
	const char* pCharPCL)
{
	if (tidChar == 0)
		return 0;

	IFF_STRUCT::sChar* pChar = FindChar(tidChar);
	if (pChar == NULL)
		return 0;

	unsigned char level = pChar->PCL[eType];
	if (pCharPCL)
		level += pCharPCL[eType];

	return level;
}

// 983: tid do caddie -> registro (o tipo 0x20 vira 0x1c + soma dos dois campos)
static __forceinline unsigned long CaddieKey983(unsigned long tid)
{
	if ((tid & 0xfc000000) == 0x20000000)
		return (((tid >> 21) & 0x1f) + ((tid >> 16) & 0x1f)) | 0x1c000000;
	return tid;
}


// 00a1fb40: capacidade (slots) = base do personagem + pecas + auxiliares + taco + caddie + mascote + cartas + conjunto
unsigned char CItemManager::GetCapacity(const sCapacityParam983* p)
{
	const unsigned long* pParts = p->tidParts;
	const unsigned long* pAuxParts = p->tidAuxParts;
	int type = p->type;
	unsigned char level = p->level;

	IFF_STRUCT::sChar* pChar = FindChar(p->tidChar);
	unsigned char capacity = 0;
	if (pChar)
	{
		if (type == 0)
		{
			capacity = (unsigned char)pChar->Attr[0];
			if (level >= 6)
				capacity += (level - 1) / 5;
		}
		else if (type > 0 && type <= 4)
			capacity = (unsigned char)pChar->Attr[type];
	}

	if (pParts)
	{
		IFF_STRUCT::sPart* parts[24];
		for (int i = 0; i < 24; i++)
		{
			if (pParts[i] == 0)
				parts[i] = NULL;
			else
				parts[i] = FindPart(pParts[i]);
			if (parts[i])
				capacity += (unsigned char)parts[i]->Slot[type] + (unsigned char)parts[i]->Attr[type];
		}
	}

	if (pAuxParts && type < 5)
	{
		IFF_STRUCT::sAuxPart* auxParts[5];
		for (int i = 0; i < 5; i++)
		{
			if (pAuxParts[i] == 0)
				auxParts[i] = NULL;
			else
				auxParts[i] = FindAuxPart(pAuxParts[i]);
			if (auxParts[i])
				capacity += (unsigned char)auxParts[i]->Slot[type] + (unsigned char)auxParts[i]->Attr[type];
		}
	}

	GetCapacityStat983(p, false);
	capacity += (unsigned char)GetCapacitySlot983(p, false);

	IFF_STRUCT::sCaddie* pCaddie = FindCaddie(CaddieKey983(p->tidCaddie));
	capacity += ((CItemManagerStatusJp*)this)->GetCaddieStat983(type, pCaddie, level);

	IFF_STRUCT::sMascot* pMascot = FindMascot(p->m_28);
	capacity += ((CItemManagerStatusJp*)this)->GetMascotStat983(type, pMascot, level);

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		capacity += CCardManager::Instance()->GetCardPeriodStatus(type);
		capacity += CCardManager::Instance()->GetCardStatusSlot(type);
		capacity += CCardManager::Instance()->GetClubCardStatusSlot983(type);
	}

	return capacity + ((CItemManagerStatusJp*)this)->GetSetEffectSlot983(type, pParts, pAuxParts);
}

// 00a1fd60: nivel limitado pela capacidade (so personagem + pecas + auxiliares + cartas + maestria + conjunto)
unsigned char CItemManager::GetCharLevelCore983(unsigned long type, IFF_STRUCT::sChar* pChar, const char* pStat,
	unsigned char upgrade, const unsigned long* pParts, const unsigned long* pAuxParts, int extra)
{
	unsigned char level = 0;
	unsigned char capacity = 0;

	if (pChar)
	{
		level = (unsigned char)pChar->PCL[type];
		if (type == 0)
		{
			capacity = (unsigned char)pChar->Attr[0];
			if (upgrade >= 6)
				capacity += (upgrade - 1) / 5;
		}
		else if ((int)type > 0 && (int)type <= 4)
			capacity = (unsigned char)pChar->Attr[type];
	}

	if (pParts)
	{
		for (int i = 0; i < 24; i++)
		{
			IFF_STRUCT::sPart* pPart = pParts[i] ? FindPart(pParts[i]) : NULL;
			if (pPart)
			{
				unsigned char attr = (unsigned char)pPart->Attr[type];
				level += attr;
				capacity += (unsigned char)pPart->Slot[type] + attr;
			}
		}
	}

	if (pAuxParts && (int)type < 5)
	{
		for (int i = 0; i < 5; i++)
		{
			IFF_STRUCT::sAuxPart* pAux = pAuxParts[i] ? FindAuxPart(pAuxParts[i]) : NULL;
			if (pAux)
			{
				if (pAux->Attr[type] != 99)
				{
					level += pAux->Attr[type];
					capacity += pAux->Attr[type];
				}
				if (pAux->Slot[type] != 99)
					capacity += pAux->Slot[type];
			}
		}
	}

	unsigned char card = (unsigned char)CCardManager::Instance()->GetCardStatusSlot(type);
	card += (unsigned char)CCardManager::Instance()->GetCardPeriodStatus(type);
	capacity += card;

	if (extra)
	{
		CStatusHelper helper;
		capacity += (unsigned char)helper.GetMasterySlotOf983(type, pChar ? pChar->c.TypeId : 0, extra);
	}

	capacity += ((CItemManagerStatusJp*)this)->GetSetEffectSlot983(type, pParts, pAuxParts);

	if (pStat)
	{
		unsigned char value = level + pStat[type];
		if (value <= capacity)
			return value;
	}
	return capacity;
}

// 00a23620
unsigned char CItemManager::GetLevel(const sCapacityParam983* p)
{
	IFF_STRUCT::sChar* pChar = FindChar(p->tidChar);
	IFF_STRUCT::sMascot* pMascot = FindMascot(p->m_28);
	IFF_STRUCT::sCaddie* pCaddie = FindCaddie(CaddieKey983(p->tidCaddie));
	int extra = p->m_30;

	int type = p->type;
	unsigned char level = p->level;
	int total = GetCharLevelCore983(type, pChar, (const char*)p->m_8, level, p->tidParts, p->tidAuxParts, extra);
	total += GetCapacityStat983(p, true);
	total += ((CItemManagerStatusJp*)this)->GetCaddieStat983(type, pCaddie, level);

	unsigned char card = (unsigned char)CCardManager::Instance()->GetCardPeriodStatus(type);
	card += (unsigned char)CCardManager::Instance()->GetNpcCardStatus983(type);

	unsigned char mascot = ((CItemManagerStatusJp*)this)->GetMascotStat983(type, pMascot, level);
	return (unsigned char)(mascot + total + card);
}

int CalcPowerPenalty(int upgrade)
{
	if (upgrade >= 66)
		return 13;
	if (upgrade >= 61)
		return 12;
	if (upgrade >= 56)
		return 11;
	if (upgrade >= 51)
		return 10;
	if (upgrade >= 46)
		return 9;
	if (upgrade >= 41)
		return 8;
	if (upgrade >= 36)
		return 7;
	if (upgrade >= 31)
		return 6;
	if (upgrade >= 26)
		return 5;
	if (upgrade >= 21)
		return 4;
	if (upgrade >= 16)
		return 3;
	if (upgrade >= 11)
		return 2;
	if (upgrade >= 6)
		return 1;

	return 0;
}

// 00a2b570
status_t CItemManager::GetLevelWithPenalty(const sCapacityParam983* p)
{
	unsigned char penalty = 0;
	unsigned char level = GetLevel(p);

	if (p->type == 1 || p->type == 2)
	{
		char buf[0x34];
		memcpy(buf, p, 0x34);
		((sCapacityParam983*)buf)->type = 0;
		unsigned char power = GetLevel((const sCapacityParam983*)buf);

		penalty = Max(0, power - CalcPowerPenalty(p->level) - 20);

		if (level <= penalty)
		{
			level = 0;
			status_t status;
			status.level = level;
			status.penalty = penalty;
			return status;
		}
		level -= penalty;
	}

	status_t status;
	status.level = level;
	status.penalty = penalty;

	return status;
}

bool CItemManager::isTrade(unsigned long typeId)
{
	IFF_ITEM_COMMON* pItem = FindCommonItem(typeId);
	if (pItem == NULL)
		return false;

	return pItem->IsSalable == 1 ? true : false;
}

bool CItemManager::IsCanOverlapped(unsigned long dwTID,
	unsigned char btCheckType, unsigned char exceptGroup)
{
	bool bResult = false;

	IFF_ITEM_COMMON* pItem = FindCommonItem(dwTID);
	if (pItem == NULL)
		return false;

	switch (dwTID >> 26)
	{
	case 1:
		if (exceptGroup == 1)
			bResult = true;
		break;
	case 7:
		if (btCheckType == 1)
		{
			if (dwTID == 0x1c000001 || dwTID == 0x1c000002 ||
				dwTID == 0x1c000003 || dwTID == 0x1c000007)
				bResult = true;
		}
		break;
	case 2:
	case 4: // JP 983 (tabela 00a18d00): o grupo 4 segue o mesmo teste de IsSalable do 2
		if (pItem->IsSalable != 1 && pItem->IsSalable != 3)
			bResult = false;
		else
			bResult = true;
		break;
	case 8:
	case 14:
	case 15:
		bResult = false;
		break;
	case 16: // JP 983: so o byte +0x82 do registro
		bResult = ((unsigned char*)pItem)[0x82] != 0;
		break;
	case 9:
		if (((dwTID >> 21) & 0x1f) == 6 || ((dwTID >> 21) & 0x1f) == 5)
			bResult = true;
		else
			bResult = false;
		break;
	case 28:
		if ((dwTID & 0x1f0000) == 0)
			bResult = true;
		break;
	default:
		bResult = true;
		break;
	}

	return bResult;
}

// 00a1ff60: o 983 le os slots da peca/ClubSet pelo GetCardSlotNum (tres saidas)
bool CItemManager::IsPartAttachCard(unsigned long dwCardTid,
	unsigned long dwPartTid, int iSlotNum)
{
	unsigned short slotCharacter = 0;
	unsigned short slotCaddie = 0;
	unsigned short slotNpc = 0;
	unsigned long kind = (dwCardTid >> 22) & 0xf;
	bool bAttach = false;

	if (!GetCardSlotNum(dwPartTid, slotCharacter, slotCaddie, slotNpc))
		return false;

	if (kind == 0)
	{
		if ((short)slotCharacter >= iSlotNum && (unsigned int)(iSlotNum - 1) <= 3)
			bAttach = true;
	}
	else if (kind == 1)
	{
		iSlotNum -= 4;
		if ((short)slotCaddie >= iSlotNum && (unsigned int)(iSlotNum - 1) <= 3)
			bAttach = true;
	}
	else if (kind == 5)
	{
		iSlotNum -= 8;
		if ((short)slotNpc >= iSlotNum && (unsigned int)(iSlotNum - 1) <= 1)
			bAttach = true;
	}

	return bAttach;
}

// 00a19200 (peca: +0x228/+0x22a/+0x22c; ClubSet: +0xfa/+0xfc)
bool CItemManager::GetCardSlotNum(unsigned long in_TID,
	unsigned short& out_LimitCharacterSlot, unsigned short& out_LimitCaddieSlot,
	unsigned short& out_LimitNpcSlot)
{
	switch (in_TID >> 26)
	{
	case 2:
	{
		IFF_STRUCT::sPart* pPart = FindPart(in_TID);
		if (!pPart)
			return false;

		out_LimitCharacterSlot = pPart->CharacterSlot;
		out_LimitCaddieSlot = pPart->CaddieSlot;
		out_LimitNpcSlot = pPart->NpcSlot983;
		return true;
	}
	case 4:
	{
		IFF_STRUCT::sClubSet* pClubSet = FindClubSet(in_TID);
		if (pClubSet)
		{
			out_LimitCharacterSlot = pClubSet->CharacterSlot983;
			out_LimitCaddieSlot = pClubSet->CaddieSlot983;
			return true;
		}
	}
	break;
	}

	return false;
}

bool CItemManager::IsEquipCard(unsigned long in_CardTID)
{
	if ((in_CardTID & 0xfc000000) != 0x7c000000)
		return false;

	return (in_CardTID & 0x3c00000) <= 0x400000 ? true : false;
}

COUNTING_TYPE CItemManager::GetItemCountingType(unsigned long in_Tid,
	unsigned char in_itemType)
{
	int type = 0;

	switch (in_Tid >> 26)
	{
	case 5:
	case 6:
	case 28:
		type = in_itemType ? 2 : 1;
		break;
	case 2:
	case 4:
		type = 3;
		break;
	case 7:
		type = 4;
		break;
	case 31:
		if (IsLocalContent(S4_CARD_SYSTEM))
			type = 5;
		break;
	}

	return (COUNTING_TYPE)type;
}

void CItemManager::MakeSpecialPrizeItemMap(char* Buf, int len)
{
	IFF_FILE_HEADER _hdr;
	IFF_STRUCT::sSpecialPrizeItem item;

	len = sizeof(IFF_FILE_HEADER);
	memcpy(&_hdr, Buf, len);
	char* p = Buf + len;

	for (int i = 0; i < _hdr.nRecords; i++)
	{
		memcpy(&item, p, sizeof(IFF_STRUCT::sSpecialPrizeItem));
		p += sizeof(IFF_STRUCT::sSpecialPrizeItem);

		m_SPItemMap[item.ability].insert(
			std::map<unsigned int, IFF_STRUCT::sSpecialPrizeItem>::value_type(
				item.typeId, item));
	}
}

IFF_STRUCT::sSpecialPrizeItem* CItemManager::FindSPItem(unsigned long typeId,
	eSPAVILITYTYPE type)
{
	IFF_STRUCT::sSpecialPrizeItem* pItem = NULL;
	std::map<unsigned int, IFF_STRUCT::sSpecialPrizeItem>::iterator it;

	it = m_SPItemMap[type].find(typeId);

	if (it != m_SPItemMap[type].end())
		pItem = &(*it).second;

	return pItem;
}

// JP 983 (00a3abd0): 6 itens (o 645 tinha 2); cada um exclui os 6
void CItemManager::MakeItemBuffMap()
{
	sItemBuffHelper helper;
	BuffMap_Itr it;

	m_ItemBuff.clear();

	m_ItemBuff.insert(std::make_pair(0x1a0000b3, helper));
	it = m_ItemBuff.find(0x1a0000b3);
	if (it != m_ItemBuff.end())
	{
		(*it).second.Accum = 1;
		(*it).second.BuffType = SPECIAL_EXP_REAE_UP_TIME;
		(*it).second.Duration = 120;
		(*it).second.Value = 5;
		std::set<unsigned long>& exclusiveSet = (*it).second.ExclusiveSet;
		exclusiveSet.insert(0x1a0000b4);
		exclusiveSet.insert(0x1a0000b3);
		exclusiveSet.insert(0x1a0000e1);
		exclusiveSet.insert(0x1a0000e2);
		exclusiveSet.insert(0x1a0000e3);
		exclusiveSet.insert(0x1a0000e4);
	}

	m_ItemBuff.insert(std::make_pair(0x1a0000b4, helper));
	it = m_ItemBuff.find(0x1a0000b4);
	if (it != m_ItemBuff.end())
	{
		(*it).second.Accum = 1;
		(*it).second.BuffType = SPECIAL_EXP_REAE_UP_TIME;
		(*it).second.Duration = 120;
		(*it).second.Value = 50;
		std::set<unsigned long>& exclusiveSet = (*it).second.ExclusiveSet;
		exclusiveSet.insert(0x1a0000b3);
		exclusiveSet.insert(0x1a0000b4);
		exclusiveSet.insert(0x1a0000e1);
		exclusiveSet.insert(0x1a0000e2);
		exclusiveSet.insert(0x1a0000e3);
		exclusiveSet.insert(0x1a0000e4);
	}

	// tipo 0x20 não existe no enum do 645
	helper.Accum = 1;
	helper.BuffType = (specialcardavility)0x20;
	helper.Duration = 100;
	helper.Value = 100;
	helper.ExclusiveSet.clear();
	helper.ExclusiveSet.insert(0x1a0000b3);
	helper.ExclusiveSet.insert(0x1a0000b4);
	helper.ExclusiveSet.insert(0x1a0000e1);
	helper.ExclusiveSet.insert(0x1a0000e2);
	helper.ExclusiveSet.insert(0x1a0000e3);
	helper.ExclusiveSet.insert(0x1a0000e4);
	m_ItemBuff.insert(std::make_pair(0x1a0000e1, helper));

	helper.Accum = 1;
	helper.BuffType = (specialcardavility)0x20;
	helper.Duration = 50;
	helper.Value = 100;
	helper.ExclusiveSet.clear();
	helper.ExclusiveSet.insert(0x1a0000b3);
	helper.ExclusiveSet.insert(0x1a0000b4);
	helper.ExclusiveSet.insert(0x1a0000e1);
	helper.ExclusiveSet.insert(0x1a0000e2);
	helper.ExclusiveSet.insert(0x1a0000e3);
	helper.ExclusiveSet.insert(0x1a0000e4);
	m_ItemBuff.insert(std::make_pair(0x1a0000e2, helper));

	helper.Accum = 1;
	helper.BuffType = SPECIAL_EXP_REAE_UP_TIME;
	helper.Duration = 50;
	helper.Value = 100;
	helper.ExclusiveSet.clear();
	helper.ExclusiveSet.insert(0x1a0000b3);
	helper.ExclusiveSet.insert(0x1a0000b4);
	helper.ExclusiveSet.insert(0x1a0000e1);
	helper.ExclusiveSet.insert(0x1a0000e2);
	helper.ExclusiveSet.insert(0x1a0000e3);
	helper.ExclusiveSet.insert(0x1a0000e4);
	m_ItemBuff.insert(std::make_pair(0x1a0000e3, helper));

	helper.Accum = 1;
	helper.BuffType = SPECIAL_PANG_RATE_UP_TIME;
	helper.Duration = 50;
	helper.Value = 100;
	helper.ExclusiveSet.clear();
	helper.ExclusiveSet.insert(0x1a0000b3);
	helper.ExclusiveSet.insert(0x1a0000b4);
	helper.ExclusiveSet.insert(0x1a0000e1);
	helper.ExclusiveSet.insert(0x1a0000e2);
	helper.ExclusiveSet.insert(0x1a0000e3);
	helper.ExclusiveSet.insert(0x1a0000e4);
	m_ItemBuff.insert(std::make_pair(0x1a0000e4, helper));
}

// 009d8890: monta um registro do mapa de +0x2e8 (+0x98 fica sem valor, como no 983)
static IFF983::sCupInCondition* SetCupInCondition983(IFF983::sCupInCondition* p, unsigned char bFinal, unsigned long kind,
	unsigned long id, unsigned long param10, unsigned long param14, char b1, char b2, char b4, char b8, char b10, char b20,
	unsigned char mask, char b400, char b800, char b1000, unsigned long flag9c, const char* name)
{
	p->bFinal = bFinal;
	p->id = id;
	p->kind = kind;
	p->param10 = param10;
	p->flags = 0;
	p->flag9c = 0;
	p->param14 = param14;

	if (b1 == 1)
		p->flags = 1;
	if (b2 == 1)
		p->flags |= 2;
	if (b4 == 1)
		p->flags |= 4;
	if (b8 == 1)
		p->flags |= 8;
	if (b10 == 1)
		p->flags |= 0x10;
	if (b20 == 1)
		p->flags |= 0x20;

	if ((mask & 0xe) == 0xe)
	{
		p->flags |= 0x40;
	}
	else
	{
		if (mask & 2)
			p->flags |= 0x80;
		if (mask & 4)
			p->flags |= 0x200;
		if (mask & 8)
			p->flags |= 0x100;
	}

	if (b400 == 1)
		p->flags |= 0x400;
	if (b800 == 1)
		p->flags |= 0x800;
	if (b1000 == 1)
		p->flags |= 0x1000;

	p->flag9c = flag9c & 6;
	strncpy(p->Name, name, sizeof(p->Name));

	return p;
}

// 00a2a620: os 5 registros fixos do mapa de +0x2e8
void CItemManager::MakeCupInConditionMap983()
{
	IFF983::sCupInCondition rec;

	m_CupInConditionMap983.clear();

	SetCupInCondition983(&rec, 1, 10, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, "");
	m_CupInConditionMap983.insert(std::map<unsigned int, IFF983::sCupInCondition>::value_type(1, rec));

	SetCupInCondition983(&rec, 1, 10, 2, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4,
		K2L_Compatibility("\xb4\xdc \xc7\xd1 \xb9\xf8\xc0\xc7 \xbd\xc3\xb5\xb5\xb7\xce \xc8\xa6\xc0\xce"));
	m_CupInConditionMap983.insert(std::map<unsigned int, IFF983::sCupInCondition>::value_type(2, rec));

	SetCupInCondition983(&rec, 1, 10, 3, 0, 1, 1, 0, 0, 0, 0, 0, 0xe, 0, 0, 0, 4,
		K2L_Compatibility("\xc6\xaf\xbc\xf6\xbc\xa6\xc0\xb8\xb7\xce \xc8\xa6\xc0\xce"));
	m_CupInConditionMap983.insert(std::map<unsigned int, IFF983::sCupInCondition>::value_type(3, rec));

	SetCupInCondition983(&rec, 1, 10, 4, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4,
		K2L_Compatibility("4\xb9\xe6\xc7\xe2 \xc4\xbf\xb8\xc7\xb5\xe5 \xbe\xf8\xc0\xcc \xc8\xa6\xc0\xce"));
	m_CupInConditionMap983.insert(std::map<unsigned int, IFF983::sCupInCondition>::value_type(4, rec));

	SetCupInCondition983(&rec, 1, 10, 5, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4,
		K2L_Compatibility("\xc6\xce\xbe\xdf\xbc\xa6\xc0\xbb \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xa9 \xc8\xa6\xc0\xce"));
	m_CupInConditionMap983.insert(std::map<unsigned int, IFF983::sCupInCondition>::value_type(5, rec));
}

// 00a36130: tabelas derivadas das que o laço do Load leu
void CItemManager::MakeDerivedTables983()
{
	ShuffleScratchItem983();
	MakeLevelUpPrizeText983();
	MakeAchievementQuestItem983();
	MakeCourseBonus983();
}

// 00a23760 (o embaralhamento é 00a17060, o random_shuffle da biblioteca)
void CItemManager::ShuffleScratchItem983()
{
	std::random_shuffle(m_ScratchItem983.begin(), m_ScratchItem983.end());
}

// 009da0b0
static bool HasLevelUpPrize983(const IFF983::sLevelUpPrizeItem* pItem)
{
	char n = pItem->tidPrize[0] != 0;

	if (pItem->tidPrize[1] != 0)
		n++;

	return n != 0;
}

// 00a2f970: "nome quantidade, nome quantidade" dos 2 prêmios do nível. No 983 é função solta e chega ao
// CItemManager pelo CSharedDoc global (00e47e2c + 0x20c); aqui é método (só existe um CItemManager).
std::string CItemManager::GetPrizeText983(const IFF983::sLevelUpPrizeItem* pItem)
{
	std::stringstream ss;

	for (int i = 0; i < 2; i++)
	{
		unsigned long tid = pItem->tidPrize[i];
		int count = pItem->nPrize[i];

		if (tid == 0)
			continue;

		if (tid == 0x1a000010)
		{
			ss << count << K2L_Compatibility("\xc6\xce");
		}
		else
		{
			ss << GetItemName(tid);

			IFF_ITEM_COMMON* pCommon = FindCommonItem(tid);
			if (pCommon)
			{
				switch (pCommon->TimeFlag)
				{
				case 0:
					if (count > 1)
						ss << K2L_Compatibility(" ") << count << K2L_Compatibility("\xb0\xb3");
					break;
				case 1:
				case 4:
					ss << K2L_Compatibility(" ") << count << K2L_Compatibility("\xc0\xcf");
					break;
				default:
					ss << K2L_Compatibility(" ") << K2L_Compatibility("(") << count << K2L_Compatibility(")");
					break;
				}
			}
		}

		ss << K2L_Compatibility(", ");
	}

	size_t pos = ss.str().rfind(',');

	return ss.str().substr(0, pos);
}

// 00a331a0
std::string CItemManager::GetLevelPrizeText983(const IFF983::sLevelUpPrizeItem* pItem)
{
	if (!HasLevelUpPrize983(pItem))
		return "";

	std::stringstream ss;

	ss << K2L_Compatibility("\xbc\xb1\xb9\xb0\xb7\xce") << K2L_Compatibility(" ") << GetPrizeText983(pItem)
	   << K2L_Compatibility("(\xc0\xbb)\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.");

	return ss.str();
}

// 00a33350
std::string CItemManager::GetNextLevelPrizeText983(const IFF983::sLevelUpPrizeItem* pItem)
{
	if (!HasLevelUpPrize983(pItem))
		return "";

	std::stringstream ss;

	ss << K2L_Compatibility("\xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9") << K2L_Compatibility(" ") << GetPrizeText983(pItem)
	   << K2L_Compatibility("(\xc0\xbb)\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.");

	return ss.str();
}

// 00a351f0: textos do prêmio de cada nível e do nível seguinte
void CItemManager::MakeLevelUpPrizeText983()
{
	std::map<unsigned int, IFF983::sLevelUpPrizeItem>::iterator it;

	for (it = m_LevelUpPrizeItemMap983.begin(); it != m_LevelUpPrizeItemMap983.end(); ++it)
	{
		IFF983::sLevelUpPrizeItem& item = (*it).second;

		strncpy(item.szPrize, GetLevelPrizeText983(&item).c_str(), 0x80);

		std::map<unsigned int, IFF983::sLevelUpPrizeItem>::iterator next =
			m_LevelUpPrizeItemMap983.find((unsigned char)(item.level + 1));

		if (next != m_LevelUpPrizeItemMap983.end())
			strncpy(item.szNextPrize, GetNextLevelPrizeText983(&(*next).second).c_str(), 0x80);
	}
}

// 009e9d00 (+ 009de840): conquista vista como item de missão do tipo 2
static void AchievementToQuestItem983(IFF983::sQuestItem* pQuest, const IFF983::sAchievement* pAchievement)
{
	unsigned long* q = (unsigned long*)pQuest;
	const unsigned long* a = (const unsigned long*)pAchievement;
	int i;

	for (i = 0xc0 / 4; i <= 0x10c / 4; i++)
		q[i] = 0;

	pQuest->bFinal = pAchievement->bFinal;
	pQuest->TypeId = pAchievement->TypeId;
	strncpy((char*)pQuest + 8, (const char*)pAchievement + 8, 0x80);

	q[0xc8 / 4] = 2;
	for (i = 0; i < 10; i++)
		q[0xcc / 4 + i] = a[0x5d4 / 4 + i];
	q[0x10c / 4] = a[0x5fc / 4];
}

// 00a2b970: cada conquista entra também no mapa do QuestItem.iff
void CItemManager::MakeAchievementQuestItem983()
{
	std::map<unsigned int, IFF983::sAchievement>::iterator it;

	for (it = m_AchievementMap983.begin(); it != m_AchievementMap983.end(); ++it)
	{
		IFF983::sQuestItem item;

		AchievementToQuestItem983(&item, &(*it).second);
		m_QuestItemMap983.insert(std::map<unsigned int, IFF983::sQuestItem>::value_type(item.TypeId, item));
	}
}

// 00a07150: primeiro elemento com (course, bSecond); NULL se nao houver
IFF983::sCourseBonus* CItemManager::FindCourseBonus983(unsigned char course, unsigned char bSecond)
{
	std::vector<IFF983::sCourseBonus>::iterator it;

	for (it = m_CourseBonus983.begin(); it != m_CourseBonus983.end(); ++it)
	{
		if ((*it).course == course && (*it).bSecond == bSecond)
			return &*it;
	}
	return NULL;
}

// 00a33cb0: 2 elementos por campo
void CItemManager::MakeCourseBonus983()
{
	std::map<unsigned int, IFF_STRUCT::sCourse>::iterator it;

	m_CourseBonus983.clear();

	for (it = m_CourseMap.begin(); it != m_CourseMap.end(); ++it)
	{
		const unsigned int* p = (*it).second.CourseBonus983; // as 2 faixas em sequencia (+0x168, +0x180)

		for (unsigned int i = 0; i < 2; i++)
		{
			IFF983::sCourseBonus bonus;

			bonus.course = (unsigned char)(*it).second.c.TypeId;
			bonus.value0 = (int)p[0];
			bonus.bSecond = i == 1;
			bonus.value1 = (int)p[1];
			bonus.value2 = (int)p[2];
			bonus.value3 = (int)p[3];
			bonus.value4 = (int)p[4];
			bonus.value5 = (int)p[5];
			m_CourseBonus983.push_back(bonus);

			p += 6;
		}
	}
}

bool CItemManager::IsItemBuff(unsigned long dwTID)
{
	BuffMap_Itr it = m_ItemBuff.find(dwTID);
	if (it != m_ItemBuff.end())
		return true;
	return false;
}

bool CItemManager::IsCompatibleBuff(unsigned long dwBuffTID,
	unsigned long dwWithTID)
{
	BuffMap_Itr it = m_ItemBuff.find(dwBuffTID);
	if (it == m_ItemBuff.end())
	{
		return false;
	}

	const std::set<unsigned long>& exclusiveSet = (*it).second.ExclusiveSet;
	std::set<unsigned long>::const_iterator check =
		exclusiveSet.find(dwWithTID);
	if (check == exclusiveSet.end())
		return true;

	return false;
}

bool CItemManager::GetBuffProperty(unsigned long dwTID,
	specialcardavility& eType, int& Value, int& Duration, int& Accum)
{
	BuffMap_Itr it = m_ItemBuff.find(dwTID);
	if (it == m_ItemBuff.end())
		return false;

	eType = (*it).second.BuffType;
	Value = (*it).second.Value;
	Duration = (*it).second.Duration;
	Accum = (*it).second.Accum;
	return true;
}

namespace S5
{
	bool ISCutinItem(unsigned long _typeId)
	{
		if ((_typeId & 0xfc000000) == 0x38000000 &&
			(_typeId & 0x3c00000) == 0x1400000)
			return true;

		return false;
	}
}

const char* CItemManager::GetIconName(unsigned long typeId)
{
	if ((typeId & 0xfc000000) != 0x2c000000)
	{
		IFF_ITEM_COMMON* pItem = FindCommonItem(typeId);
		if (pItem)
			return pItem->Icon;
	}
	else
	{
		IFF_STRUCT::sMatch* pMatch = FindMatch(typeId);
		if (pMatch)
			return pMatch->Icon[0];
	}

	return "";
}

bool CItemManager::IsUnlimitAztec(unsigned long typeId)
{
	if ((typeId & 0xfc000000) == 0x14000000)
	{
		IFF_STRUCT::sBall* pBall = FindBall(typeId);
		if (pBall && pBall->COM[0] == 0)
			return true;
	}

	return false;
}

bool CItemManager::IsBasicAztecFamily(unsigned long tid)
{
	if ((tid & 0xfc000000) == 0x14000000)
	{
		if (tid == 0x14000000)
			return true;

		IFF_STRUCT::sBall* pBall = FindBall(tid);
		if (pBall && pBall->c.Price == 0 && pBall->COM[0] == 0)
			return true;
	}

	return false;
}

const char* CItemManager::ChangePartsPosmask(unsigned long dwTypeID,
	unsigned long dwPosMask)
{
	IFF_STRUCT::sPart* pPart = FindPart(dwTypeID);
	if (pPart)
	{
		pPart->PosMask = dwPosMask;
		return "The posmask is changed";
	}

	return "No exist";
}

const char* CItemManager::ChangePartsPetName(unsigned long dwTypeID,
	const char* pPetName)
{
	IFF_STRUCT::sPart* pPart = FindPart(dwTypeID);
	if (pPart)
	{
		char OldPetName[256];
		strcpy(OldPetName, pPart->Data);
		strcpy(pPart->Data, pPetName);
		return "The petname is changed";
	}

	return "No exist";
}

const char* CItemManager::ChangePartsTexName(unsigned long dwTypeID,
	unsigned long dwTexType, unsigned long dwTexNum, const char* pTexName)
{
	if (dwTexType != 0 && dwTexType != 1)
		return "Invalid texture type.";

	if (dwTexNum <= 2)
	{
		IFF_STRUCT::sPart* pPart = FindPart(dwTypeID);
		if (pPart)
		{
			if (dwTexType == 0)
				strcpy(pPart->Tex[dwTexNum], pTexName);
			else if (dwTexType == 1)
				strcpy(pPart->OrgTex[dwTexNum], pTexName);

			return "The texname is changed";
		}

		return "No exist";
	}

	return "Invalid texture.";
}

_private::sRandomBox* CItemManager::findRandomBox(unsigned long boxTypeId)
{
	_private::sRandomBox* pBox = NULL;
	std::map<unsigned int, _private::sRandomBox>::iterator it = m_RandomBoxMap.find(boxTypeId);

	if (it != m_RandomBoxMap.end())
		pBox = &it->second;

	return pBox;
}

IFF_STRUCT::sRandomBox* CItemManager::GetRandomBoxInfo(unsigned long boxTypeId,
	unsigned long typeId, unsigned long itemIndex)
{
	_private::sRandomBox* pBox = findRandomBox(boxTypeId);
	if (pBox == NULL)
		return NULL;

	std::map<unsigned long, _private::sRandomBoxItem>::iterator it =
		pBox->items.find(itemIndex);
	if (it == pBox->items.end())
		return NULL;

	return &it->second.box;
}

bool CItemManager::GetEnumRandomBoxStuff(unsigned long boxTypeId,
	std::vector<IFF_STRUCT::sRandomBox>& out)
{
	_private::sRandomBox* pBox = findRandomBox(boxTypeId);
	if (pBox == NULL)
		return false;

	std::vector<_private::sRandomBoxStuff>::iterator it = pBox->stuffs.begin();
	std::vector<_private::sRandomBoxStuff>::iterator itEnd = pBox->stuffs.end();

	for (; it != itEnd; ++it)
		out.push_back((*it).box);

	return true;
}

bool CItemManager::GetEnumRandomBoxItem(unsigned long boxTypeId,
	std::vector<std::pair<unsigned long, IFF_STRUCT::sRandomBox> >& out)
{
	_private::sRandomBox* pBox = findRandomBox(boxTypeId);
	if (pBox == NULL)
		return false;

	std::map<unsigned long, _private::sRandomBoxItem>::iterator it =
		pBox->items.begin();
	std::map<unsigned long, _private::sRandomBoxItem>::iterator itEnd =
		pBox->items.end();

	for (; it != itEnd; ++it)
		out.push_back(std::make_pair(it->first, it->second.box));

	return true;
}

bool CItemManager::GetEnumRandomBoxCheckItem(unsigned long boxTypeId,
	unsigned long typeId, unsigned long itemIndex,
	std::vector<IFF_STRUCT::sRandomBox>& out)
{
	_private::sRandomBox* pBox = findRandomBox(boxTypeId);
	if (pBox == NULL)
		return false;

	std::map<unsigned long, _private::sRandomBoxItem>::iterator it =
		pBox->items.find(itemIndex);
	if (it == pBox->items.end())
		return false;

	if (typeId != it->second.box.typeId)
		return false;

	std::vector<IFF_STRUCT::sRandomBox>::const_iterator itCheck =
		it->second.checkItems.begin();
	std::vector<IFF_STRUCT::sRandomBox>::const_iterator itCheckEnd =
		it->second.checkItems.end();

	for (; itCheck != itCheckEnd; ++itCheck)
		out.push_back(*itCheck);

	return true;
}

// JP 983 (00a20080): assinatura (00a19170) ou nao-visivel; nos dois o teste e o bit 1 de "type" (no 645 era o bit 0)
bool CItemManager::IsNonVisibleItem(unsigned long typeId)
{
	if (IsSubScriptionCoupon(typeId))
		return true;

	std::map<unsigned int, IFF_STRUCT::sNonVisibleItem>::const_iterator it =
		m_NonVisibleItemMap.find(typeId);
	if (it == m_NonVisibleItemMap.end())
		return false;

	return ((it->second.type >> 1) & 1) != 0;
}

// 00a19170
bool CItemManager::IsSubScriptionCoupon(unsigned long typeId)
{
	std::map<unsigned int, IFF_STRUCT::sSubscriptionItem>::const_iterator it =
		m_SubscriptionItemMap.find(typeId);
	if (it == m_SubscriptionItemMap.end())
		return false;

	return ((it->second.type >> 1) & 1) != 0;
}

// 00a200e0: o tid esta num grupo do TwinsItemTable.iff?
bool CItemManager::IsTwinsItem983(unsigned long typeId)
{
	if (!m_pTwinsItemMap983)
		return false;
	return m_pTwinsItemMap983->find(typeId) != m_pTwinsItemMap983->end();
}

// 00a20120: tid2 esta no grupo do tid?
bool CItemManager::IsTwinsGroupMember983(unsigned long typeId, unsigned long typeId2)
{
	if (!m_pTwinsItemMap983)
		return false;
	std::map<unsigned int, std::vector<unsigned long> >::iterator it = m_pTwinsItemMap983->find(typeId);
	if (it == m_pTwinsItemMap983->end())
		return false;
	return std::find(it->second.begin(), it->second.end(), typeId2) != it->second.end();
}
