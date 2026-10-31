#pragma once

#include <string.h>

// Common header for every shop item record in pangya.iff
struct IFF_ITEM_COMMON
{
	bool Final;
	unsigned int TypeId;
	char Name[64]; // 983: 40 -> 64
	unsigned char Level : 7;
	unsigned char IsUnderLvl : 1;
	char Icon[40];
	unsigned int Price;
	unsigned int SalePrice;
	unsigned int UsedPrice;
	unsigned char IsCash : 1;
	unsigned char IsSalable : 4;
	unsigned char InStock : 3;
	unsigned char New : 1;
	unsigned char Hit : 1;
	unsigned char Reserve : 1;
	unsigned char TimeFlag;
	unsigned char Time;
	// Tiki Shop (troca de itens por Tiki Points / quebra de mileage); nomes da comunidade em docs/IFF_ITEM_COMMON.md
	unsigned int TikiQtyPerPoint; // unidades que valem TikiPoints (era Point; quantidade entregue na loja de pontos)
	unsigned int TikiPoints; // 983: preço na loja de pontos; 0 = fora dela
	unsigned short Mileage; // 983: mileage ("MP") da quebra na alquimia; 0 = não quebra
	unsigned short BonusProb; // 983: chance do bônus de mileage em % (só o servidor lê)
	unsigned short BonusMin; // 983: bônus mínimo de mileage (só o servidor lê)
	unsigned short BonusMax; // 983: bônus máximo de mileage (só o servidor lê)
	unsigned int TikiType; // 983: aba da quebra (1 Pang, 2 cash, 3 especial)
	unsigned int TikiPang; // 983: custo em Pang por item na quebra
	unsigned int DateFlag; // 983: 2 = saleStart/saleEnd valem só para a promoção, não para a venda
	_SYSTEMTIME saleStart;
	_SYSTEMTIME saleEnd;
};
C_ASSERT(offsetof(IFF_ITEM_COMMON, Icon) == 0x49);
C_ASSERT(offsetof(IFF_ITEM_COMMON, Price) == 0x74);
C_ASSERT(offsetof(IFF_ITEM_COMMON, TikiQtyPerPoint) == 0x84);
C_ASSERT(offsetof(IFF_ITEM_COMMON, Mileage) == 0x8c);
C_ASSERT(offsetof(IFF_ITEM_COMMON, DateFlag) == 0x9c);
C_ASSERT(sizeof(IFF_ITEM_COMMON) == 0xc0);

namespace IFF_STRUCT
{
	// Character.iff
	struct sChar
	{
		IFF_ITEM_COMMON c;
		char Data[40];
		char HairTex[40];
		char ShirtsTex[40];
		char FaceTex[40];
		short Attr[5];
		unsigned char nParts;
		unsigned char nAcsries;
		unsigned char ClubType;
		float ClubScale;
		char PCL[5];
		char Mtn30sWinner[40];
	};

	// Part.iff
	struct sPart
	{
		IFF_ITEM_COMMON c;
		char Data[40];
		unsigned char Category;
		unsigned int PosMask;
		unsigned int HideMask;
		char Tex[3][40];
		char OrgTex[3][40];
		short Attr[5];
		short Slot[5];
		char EquippableWith[40];
		unsigned int SubPart[2];
		short CharacterSlot;
		short CaddieSlot;
		short NpcSlot983; // 983: 3ª contagem de slots (0x22c; cartas com kind == 5); nome provisório
		unsigned int RentalPrice;
		unsigned char Beginner;
		short Point;
	};

	// Club.iff
	struct sClub
	{
		IFF_ITEM_COMMON c;
		char Data[40];
		unsigned char Type;
		short COM[5];
	};

	// ClubSet.iff
	struct sClubSet
	{
		IFF_ITEM_COMMON c;
		unsigned int ClubIds[4];
		short Attr[5];
		short Slot[5];
		// 983 (0x104): campos novos no fim (chamadores do FindClubSet 00a171e0; valores do ClubSet.iff do 839/JP 966)
		int nWorkshopType983; // +0xe4: -1..4; indice do bonus de rank (GetClubRankBonus983); -1 = sem Club Workshop
		int nWorkshopParam983; // +0xe8: 0, 1, 3, 4; vai para sSlot983::m_58 do ClubSetWorkshopFx983
		int nRecoveryMax983; // +0xec: 5 (ou 0); limite de recuperacao ("%d/%d" do Club Workshop)
		float fRate983; // +0xf0: 1.0 (ou 1.5)
		int nWorkshopTypeId983; // +0xf4: 0..3; vai para sSlot983::typeId do ClubSetWorkshopFx983
		short flag983; // +0xf8: 1 em 13 sets do JP 966 (tid 0x10000001..07, 0d..10, 1b, 61), 0 nos outros; sem leitura achada
		unsigned short CharacterSlot983; // +0xfa: cartas (00807010, 00807960, 00807ca0, 007ff6c0)
		unsigned short CaddieSlot983; // +0xfc
		short pad983_fe; // +0xfe: alinhamento do nKind983 (0 nos 126 registros do JP 966)
		unsigned int nKind983; // +0x100: 0 = 0x12 (CSharedDoc 00632fd0)
	};

	// Ball.iff
	struct sBall
	{
		enum
		{
			FX_ALWAYS,
			FX_B4_SHOT,
			FX_SHOT,
			FX_FLY,
			FX_BOUNCE,
			FX_ROLL,
			FX_HOLE_IN,
			FX_MAX_NUM
		};

		IFF_ITEM_COMMON c;
		unsigned int Flag983; // 983: +0xc0, novo antes de Data; bit 0 lido por CItemManager 00a194f0
		char Data[40];
		unsigned int Bound;
		unsigned int Roll;
		char Fx[FX_MAX_NUM][40];
		char FxBone[FX_MAX_NUM][40];
		short COM[5];
	};

	// Caddie.iff
	struct sCaddie
	{
		IFF_ITEM_COMMON c;
		unsigned int MonthlyFee;
		char Data[40];
		short Attr[5];
	};

	// CaddieItem.iff
	struct sCadItem
	{
		IFF_ITEM_COMMON c;
		char FaceTex[40];
		char BodyTex[40];
		short COM[5];
	};

	// SetItem.iff
	struct sSetItem
	{
		IFF_ITEM_COMMON c;
		unsigned char nElems;
		unsigned int ElemIds[10];
		short ElemNum[10];
		short COM[5];
	};

	// Course.iff
	struct sCourse
	{
		IFF_ITEM_COMMON c;
		char Data[40];
		char AmbientSnd[40];
		unsigned char Difficulty : 4;
		unsigned char DiffFlag : 4;
		char TexProp[40];
		float Slope;
		char SkyFx[40];
		// 983 (0x1d0): campos novos no fim (Course.iff do 839; FindCourse 00a17390 nao tem chamador que os leia)
		unsigned int CourseBonus983[6]; // +0x168: 1a faixa do IFF983::sCourseBonus (00a33cb0); 50, 50, 40, 30, 20, 10 no Blue Lagoon
		unsigned int CourseBonus2_983[6]; // +0x180: 2a faixa (bSecond); 150, 150, 120, 90, 60, 30
		char HolePar983[18]; // +0x198: par de cada buraco (3..5)
		char HoleMin983[18]; // +0x1aa: -2/-3/-4 por buraco
		char HoleMax983[18]; // +0x1bc: 5 por buraco
	};

	// Match.iff
	struct sMatch
	{
		bool Final;
		unsigned int TypeId;
		char Name[80];
		unsigned char Level;
		char Icon[6][40];
	};

	// Title.iff
	struct sTitle
	{
		bool Final;
		unsigned int TypeId;
		char Name[40];
		char Icon[40];
	};

	// Enchant.iff
	struct sEnchant
	{
		bool Final;
		unsigned int TypeId;
		unsigned int ReqdPang;
		unsigned char ReqdLevel;
	};

	// Skin.iff
	struct sSkin
	{
		IFF_ITEM_COMMON c;
		char Tex[40];
		unsigned char HScroll;
		unsigned char VScroll;
		short COM[5];
	};

	// HairStyle.iff
	struct sHairStyle
	{
		IFF_ITEM_COMMON c;
		unsigned char cHairID;
		unsigned char cCharID;
	};

	// ChildItem.iff
	struct sChildItem
	{
		bool Final;
		unsigned int TypeId;
		char Name[40];
		unsigned char Level : 7;
		unsigned char IsUnderLvl : 1;
		unsigned char IsDisplay : 1;
		unsigned char IsCash : 1;
		unsigned char IsSalable : 1;
		unsigned int NumberPrice;
		unsigned char NumberFlag;
		unsigned short Number;
		unsigned int TimePrice;
		unsigned char TimeFlag;
		unsigned short Time;
		short FailPercent;
	};

	// Item.iff
	struct sItem
	{
		IFF_ITEM_COMMON c;
		unsigned int RandomBox : 1;
		unsigned int Artifact : 2; // 983: +0xc0 bits 1-2 (`& 6`): artefato (fora das listas, vai no slot da sala)
		char Data[40];
		short COM[5];
		short Point;

		bool IsRandomBox() const { return RandomBox ? true : false; }
	};

	// Mascot.iff
	struct sMascot
	{
		IFF_ITEM_COMMON c;
		char Data[40];
		char MsgTex[40];
		char COM[5];
		char Attr[5];
		char DriveUp;
		unsigned short ItemDropUp;
		unsigned short ComboUp;
		unsigned short PangUp;
		unsigned short ExpUp;
		unsigned char ItemSlot;
		unsigned char ShowMsg;
		unsigned int MsgFee;
		unsigned int PangBonus983; // 983: +0x12c, novo no fim (0x130); bonus de Pang: short baixo = melhor tacada, alto = normal (00a18890); 10, 15, 0x5000a, 0x5000f no Mascot.iff do 839
	};

	// AuxPart.iff
	struct sAuxPart
	{
		enum
		{
			AUX_DRIVEUP,
			AUX_ITEMDROPUP,
			AUX_COMBOUP,
			AUX_PANGUP,
			AUX_COMBO_BONUS,
			AUX_EXPUP,
			AUX_MAX_NUM
		};

		IFF_ITEM_COMMON c;
		short COM[5];
		char Attr[5];
		char Slot[5];
		char DriveUp;
		unsigned short ItemDropUp;
		unsigned short ComboUp;
		unsigned short PangUp;
		unsigned short ExpUp;
		short SetDriveUp983; // 983: +0xde, alcance a mais quando a peca de tidSet983 tambem esta equipada (006364d0)
		unsigned long tidSet983; // 983: +0xe0 (sizeof 0xe4)
	};

	// QuestDrop.iff
	struct sQuestDrop
	{
		IFF_ITEM_COMMON c;
		_SYSTEMTIME dropStart;
		_SYSTEMTIME dropEnd;
		char dropFx[40];
		unsigned char dropCycle;
		unsigned short holesToPlay[10];
		unsigned char mapDiff;
		unsigned char holeDrop;
		unsigned char timeDrop;
	};

	// Quest.iff
	struct sQuest
	{
		bool Final;
		unsigned int TypeId;
		char Name[40];
		unsigned char Level;
		char Icon[40];
		unsigned long CompTid[10];
		unsigned char CompProb[10];
		unsigned long DropTid[5];
		unsigned char DropNum[5];
		unsigned short ProbTotal;
		unsigned short CurItemIdx;
		unsigned int* aTidItems;
		_SYSTEMTIME questStart;
		_SYSTEMTIME questEnd;
		unsigned char BtnType;
	};

	// Card.iff
	struct sCard
	{
		IFF_ITEM_COMMON c;
		unsigned char RareType;
		char Image[40];
		short COM[5];
		unsigned short Avility;
		unsigned short AvilityValue;
		char SubIcon[40];
		char SlotImg[40];
		char BuffImg[40];
		unsigned short UseTime;
		unsigned short Volume;
		unsigned short CardIndex;
		// 983: dois campos novos no fim (0x180), so vem do Card.iff (0..15 e 0/1 no 839); sem leitor no 983
		// (D941: chamadas de FindCard 00a17a10 +0x800, unidades de cartas e itemmanager varridas)
		unsigned int pad983_178;
		unsigned int pad983_17c;
	};

	// Furniture.iff
	struct sFurniture
	{
		IFF_ITEM_COMMON c;
		char Data[40];
		short Num;
		short IsOwn;
		short IsMove;
		short IsFunction;
		short Etc;
		float Pos[4];
		char Tex[3][40];
		char OrgTex[3][40];
		short COM[5];
		unsigned short UseTime;
	};

	// OfflineShop.iff
	struct sOfflineShop
	{
		IFF_ITEM_COMMON c;
		unsigned char nElems;
		unsigned int ElemIds[10];
		short COM[5];
	};

	// Desc.iff
	struct sDesc
	{
		unsigned int TypeId;
		char Desc[512];
	};

	// RandomBox.sff
	// pad983_*: so copiados do arquivo; o 983 MakeRandomBoxMap 00a3db10 le so active/boxTypeId/kind/
	// typeId/checkTypeId (+0/+0x24/+0x28/+0x2c/+0x38) e as consultas (00a1f430/00a20040/009d8860) nao tem chamador (D941)
	struct sRandomBox
	{
		bool active;
		unsigned char pad983_01[35];
		unsigned int boxTypeId;
		unsigned char kind;
		unsigned long typeId;
		unsigned long pad983_30[2];
		unsigned long checkTypeId;
		unsigned long pad983_3c[42];
	};

	// CadieMagicBoxRandom.iff
	struct sRandomRecycle
	{
		unsigned int uiRandSeq;
		unsigned int uiTypeId;
		unsigned int uiCount;
		unsigned int uiProbs;
	};

	// NonVisibleItemTable.iff
	struct sNonVisibleItem
	{
		bool active;
		unsigned long type;
		unsigned long typeId;
		_SYSTEMTIME start;
		_SYSTEMTIME end;
	};

	// SubscriptionItemTable.iff
	struct sSubscriptionItem
	{
		bool active;
		unsigned long type;
		unsigned long typeId;
		_SYSTEMTIME start;
		_SYSTEMTIME end;
	};

	// SpecialPrizeItem.iff
	struct sSpecialPrizeItem
	{
		unsigned long typeId;
		unsigned long ability;
		float rate;
	};

	// CadieMagicBox.iff
	struct sCadieMagicBox
	{
		enum eCadieMagicBoxTab
		{
			TAB_LOW,
			TAB_MIDDLE,
			TAB_HIGH,
			TAB_SPECIAL,
			TAB_EVENT
		};

		sCadieMagicBox()
		{
			uiNumber = 0;
			bFinal = false;
			uiCategory = 0;
			iCharacter = 0;
			iLevel = 0;
			uiOutput = 0;
			uiOutputCount = 0;
			memset(uiElem, 0, sizeof(uiElem));
			memset(uiElemCount, 0, sizeof(uiElemCount));
			uiRandSeq = 0;
			memset(szRandName, 0, sizeof(szRandName));
			uiFlag983 = 0;
			memset(&dateStart983, 0, sizeof(dateStart983));
			memset(&dateEnd983, 0, sizeof(dateEnd983));
		}

		unsigned int uiNumber;
		bool bFinal;
		unsigned int uiCategory;
		int iCharacter;
		int iLevel;
		unsigned int uiFlag983; // 983: +0x14, novo; o leitor 00a22d80 copia o byte baixo para sRecycleItem+0x0b
		unsigned int uiOutput;
		unsigned int uiOutputCount;
		unsigned int uiElem[4];
		unsigned int uiElemCount[4];
		unsigned int uiRandSeq;
		char szRandName[40];
		_SYSTEMTIME dateStart983; // 983: +0x6c e +0x7c, novos no fim (0x8c); vao para sRecycleItem+0x50/+0x60
		_SYSTEMTIME dateEnd983;
	};

	namespace S5
	{
		// CutinInfomation.iff
		struct sCutinInformation
		{
			sCutinInformation()
			{
				bFinal = false;
				NormalTypeId = 0;
				RareTypeId = 0;
				Rarity = 0;
				CharIndex = 0;
				Condition = 0;
				memset(Char_Tex, 0, sizeof(Char_Tex));
				Char_Ani = 0;
				memset(Bg_Tex, 0, sizeof(Bg_Tex));
				Bg_Ani = 0;
				memset(Pattern_Tex, 0, sizeof(Pattern_Tex));
				Pattern_Ani = 0;
				memset(Text_Tex, 0, sizeof(Text_Tex));
				Text_Ani = 0;
				Out_Ani = 0;
			}

			bool bFinal;
			unsigned int NormalTypeId;
			unsigned int RareTypeId;
			unsigned int Rarity;
			int Condition;
			int Img_Pos;
			int CharIndex;
			char Char_Tex[40];
			unsigned int Char_Ani;
			char Bg_Tex[40];
			unsigned int Bg_Ani;
			char Pattern_Tex[40];
			unsigned int Pattern_Ani;
			char Text_Tex[40];
			unsigned int Text_Ani;
			unsigned int Out_Ani;
		};
	}

	// FurnitureAbility.iff
	struct sFurnitureAbility
	{
		enum sFurnitureAbilityType
		{
			FAT_ITEM,
			FAT_BUFF
		};

		enum sFurnitureAbilityEffectType
		{
			FAET_ME,
			FAET_FRIEND,
			FAET_GUILD,
			FAET_ALL
		};

		enum sFuniturreAbilitySuccessType
		{
			FAST_STAY,
			FAST_SETIN,
			FAST_PUTOUT
		};

		sFurnitureAbility()
		{
			bFinal = false;
			btAbilityType = 0xff;
			dwFurnitureTID = 0xffffffff;
			btSuccessType = 0xff;
			iStayTime = 0;
			btEffectType = 0;
			dwSetInTID = 0xffffffff;
			iMaxCountAtFurniture = 0;
			memset(&stStart, 0, sizeof(stStart));
			dwDuringTime = 0;
			dwPutOutTID = 0xffffffff;
			iProbability = 0;
			iMaxCountAtUser = 0;
		}

		bool bFinal;
		unsigned char btAbilityType;
		unsigned long dwFurnitureTID;
		unsigned char btSuccessType;
		int iStayTime;
		unsigned char btEffectType;
		unsigned long dwSetInTID;
		int iMaxCountAtFurniture;
		_SYSTEMTIME stStart;
		unsigned long dwDuringTime;
		unsigned long dwPutOutTID;
		int iProbability;
		int iMaxCountAtUser;
	};

	// TikiRecipe.iff
	struct sTikiOutputTable
	{
		sTikiOutputTable() { memset(this, 0, sizeof(sTikiOutputTable)); }

		unsigned int uiIndex;
		bool bFinal;
		char strCategory[32];
		unsigned int uiTypeID;
		unsigned int uiCount;
		unsigned int uiRate;
	};

	// TikiPointTable.iff
	struct sTikiPointTable
	{
		sTikiPointTable() { memset(this, 0, sizeof(sTikiPointTable)); }

		unsigned int uiIndex;
		bool bFinal;
		char strCategory[32];
		unsigned int uiMin;
		unsigned int uiMax;
	};

	// TikiSpecialTable.iff
	struct sTikiSpecialRecipe
	{
		sTikiSpecialRecipe() { memset(this, 0, sizeof(sTikiSpecialRecipe)); }

		unsigned int uiIndex;
		bool bFinal;
		char strCategory[32];
		unsigned int uiElemCount;
		unsigned int uiElem[4];
	};
}
