#pragma once

#include <string.h>

#pragma pack(push, 1)

struct sBuyItemResult
{
	unsigned long Typeid;
	unsigned long guid;
	unsigned short Time;
	unsigned char ItemType;
	unsigned short Count;
	_SYSTEMTIME endDate;
	char UccIndex[9];

	sBuyItemResult()
	{
		Typeid = 0;
		guid = 0;
		Time = 0;
		ItemType = 0;
		Count = 0;
		memset(&endDate, 0, sizeof(endDate));
		memset(UccIndex, 0, sizeof(UccIndex));
	}
};

struct sBuyItem
{
	int Idx;
	unsigned long TypeCode;
	unsigned short DayCount;
	short sChildItemIndex;
	unsigned long ItemCount;

	sBuyItem()
	{
		memset(this, 0, sizeof(sBuyItem));
		Idx = -1;
	}
};

// HACK: itemstorage emission order. In the original sTradeItem's ctor and
// sStoredItemInfo's accessors were PCH inlines, compiled after the TU's main
// pass; itemstorage.cpp defines this as a dead reference to a late PCH inline
// so they wait the same way. Empty elsewhere.
#ifndef REBANG_DEFER_STOREDITEMINFO
#define REBANG_DEFER_STOREDITEMINFO
#endif

struct sTradeItem
{
	int iIndex;
	unsigned long dwTid;
	unsigned long dwGuid;
	int iNum;
	unsigned char byItemType;
	unsigned short wTime;
	__int64 i64Price;
	unsigned long dwUpgradeCost;
	short arrEnchant[5];
	unsigned short wFlagTradeLimit;
	char UccIndex[9];
	unsigned short Seq;
	unsigned char status;
	unsigned long attachCardTid[3][4];
	unsigned short CharacterSlotNum;
	unsigned short CaddieSlotNum;
	unsigned short NpcSlotNum983; // 983: +0x6b, de sPart::NpcSlot983 (008f8a32); sizeof 0xac
	char ItemName[41];
	char CopierNick[22];

	sTradeItem()
	{
		REBANG_DEFER_STOREDITEMINFO
		iIndex = 0;
		dwTid = 0;
		dwGuid = 0;
		iNum = 0;
		byItemType = 0;
		wTime = 0;
		i64Price = 0;
		dwUpgradeCost = 0;
		memset(arrEnchant, 0, sizeof(arrEnchant));
		wFlagTradeLimit = 0;

		memset(UccIndex, 0, sizeof(UccIndex));
		Seq = 0;
		status = 0;

		memset(attachCardTid, 0, sizeof(attachCardTid));
		memset(ItemName, 0, sizeof(ItemName));
		memset(CopierNick, 0, sizeof(CopierNick));
	}
};

struct sStallSlot
{
	unsigned long stallKey;
	unsigned char Idx;
	sTradeItem itemData;
	_SYSTEMTIME time;
	unsigned char IsPermanence;
	unsigned char IsValid;

	sStallSlot() { clear(); }

	void clear()
	{
		stallKey = 0;
		Idx = 0;
		memset(&itemData, 0, sizeof(itemData));
		memset(&time, 0, sizeof(time));
		IsPermanence = 0;
		IsValid = 0;
	}
};

struct sStall
{
	unsigned long typeID;
	unsigned long stallKey;
	unsigned char state;
	unsigned char maxSlotNum;
	unsigned char useSlotNum;
	sStallSlot slotList[12];
	_SYSTEMTIME time;

	sStall() { AllClear(); }

	void SlotClear() { memset(slotList, 0, sizeof(slotList)); }

	void AllClear()
	{
		stallKey = 0;
		maxSlotNum = 0;
		useSlotNum = 0;
		memset(&time, 0, sizeof(time));
		state = 0;
		SlotClear();
	}
};

struct sStoredItemInfo
{
	unsigned long dwStorageID;
	sTradeItem itemInfo;

	unsigned long GetTypeID() const
	{
		REBANG_DEFER_STOREDITEMINFO return itemInfo.dwTid;
	}
	void SetTypeID(unsigned long typeId)
	{
		REBANG_DEFER_STOREDITEMINFO itemInfo.dwTid = typeId;
	}

	unsigned long GetGuid() const
	{
		REBANG_DEFER_STOREDITEMINFO return itemInfo.dwGuid;
	}
	void SetGuid(unsigned long guid)
	{
		REBANG_DEFER_STOREDITEMINFO itemInfo.dwGuid = guid;
	}

	unsigned long GetCount() const
	{
		REBANG_DEFER_STOREDITEMINFO return itemInfo.iNum;
	}
	void SetCount(unsigned long count)
	{
		REBANG_DEFER_STOREDITEMINFO itemInfo.iNum = count;
	}

	sStoredItemInfo& operator=(const sStoredItemInfo& rhs)
	{
		dwStorageID = rhs.dwStorageID;
		itemInfo = rhs.itemInfo;
		return *this;
	}
};

struct sSaleItem
{
	unsigned long ownerUID;
	char nickname[22];
	unsigned long typeId;
	__int64 price;
	unsigned short count;
	unsigned char bOpen;
	unsigned char reserved[0x35];
};

struct sCardStack
{
	unsigned long uid;
	unsigned long typeId;
	int count;
	unsigned char bValid;
	unsigned char srvByte0d_983; // pacote 0x178; o 983 nao le (lista Doc+0x1142c: 0063b150/0063b190/0063b260/006404d0 so ate +0xc)
};

#pragma pack(pop)
