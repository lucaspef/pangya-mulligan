#pragma once
// 983: as funcoes da tabela sao embutidas nos chamadores (sem corpo proprio no 983)

struct sRecycleItem
{
	unsigned long dwTid;
	unsigned long dwItemid;
	union
	{
		struct
		{
			unsigned short iNum;
			unsigned char byLevel;
			unsigned char byFlag983; // 983: +0x0b, byte baixo de sCadieMagicBox+0x14 (00a22d80)
		};
		unsigned long dwNumLevel983; // 983: o construtor zera +0x8 com um dword so
	};
	short iChar;
	short iIndex;
	unsigned char byItemType;
	unsigned short wTime;
	_SYSTEMTIME date;
	unsigned long dwRandSeq;
	char szRandName[40];
	_SYSTEMTIME dateStart983; // 983: +0x50 e +0x60 (sizeof 0x70), periodo da receita no CadieMagicBox.iff
	_SYSTEMTIME dateEnd983;

	sRecycleItem() // 983 008db4f0: zera campo a campo (o new[] do leitor 00a22d80 e os membros do FrRecycleDlg)
		: dwTid(0), dwItemid(0), dwNumLevel983(0), iChar(0), iIndex(0), byItemType(0), wTime(0), dwRandSeq(0)
	{
		memset(&date, 0, sizeof(date));
		memset(szRandName, 0, sizeof(szRandName));
		memset(&dateStart983, 0, sizeof(dateStart983));
		memset(&dateEnd983, 0, sizeof(dateEnd983));
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

inline void InitIndexRecycleItem()
{
	for (short i = 0; i < g_NumIndexRecycleItem; i++)
	{
		g_ArrayRecycleItem[i].iIndex = i;
	}
}

inline unsigned long GetTidMixItem(short index, int mix)
{
	if (index < 0)
		return 0;
	if (index >= g_NumIndexRecycleItem)
		return 0;
	if (mix < 0)
		return 0;
	if (mix >= 4)
		return 0;

	return g_ArrayTidMixItem[index][mix].dwTid;
}

inline unsigned short GetNumMixItem(short index, int mix)
{
	if (index < 0)
		return 0;
	if (index >= g_NumIndexRecycleItem)
		return 0;
	if (mix < 0)
		return 0;
	if (mix >= 4)
		return 0;

	return g_ArrayTidMixItem[index][mix].iNum;
}

inline unsigned long GetTidRecycleItem(short index)
{
	if (index < 0)
		return 0;
	if (index >= g_NumIndexRecycleItem)
		return 0;

	return g_ArrayRecycleItem[index].dwTid;
}

inline unsigned short GetNumRecycleItem(short index)
{
	if (index < 0)
		return 0;
	if (index >= g_NumIndexRecycleItem)
		return 0;

	return g_ArrayRecycleItem[index].iNum;
}

inline const sRecycleItem* GetRecycleItem(short index)
{
	if (index < 0)
		return 0;
	if (index >= g_NumIndexRecycleItem)
		return 0;

	return &g_ArrayRecycleItem[index];
}
