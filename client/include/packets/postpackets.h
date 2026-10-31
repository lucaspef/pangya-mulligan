// postpackets.h - frente C (pacotes). JP 983: item anexado ao correio no pacote 0x145 (CTaskMain::OnPostSendDlgResult983,
// 0065c2d0) e nos recebidos 0x210-0x212/0x215 (caixa de correio). Os nomes sao nossos; tamanho e campos medidos no exe 983.
#pragma once

#pragma pack(push, 1)
// 0x37 bytes (o sMailIncludeItem do 645 tem 0x35). O CPost_SendDlg::GetSendList do 983 (0080f0e0) refaz a lista a cada
// chamada: memset(0, 0x37) e so os 5 primeiros campos preenchidos (idx +0x10, tid +0x0c, tipo +0x18, quantidade +0x14 e
// o word +0x19 do item da lista de origem, este estendido para dword) - o resto vai zerado ao servidor.
struct sMailIncludeItem983
{
	unsigned long dwIDX;		// +0x00
	unsigned long dwTID;		// +0x04
	unsigned char btItemType;	// +0x08
	int iCount;					// +0x09
	int iTimeCount;				// +0x0d
	// daqui em diante pelo construtor do 983 (005b4860, item dentro do sMailInfoBrief em +0x8d; 005b33c0 sozinho):
	// a mesma ordem do 645 com um word a mais depois de CaddieSlotNum
	short soCom[5];				// +0x11
	unsigned short CharacterSlotNum; // +0x1b
	unsigned short CaddieSlotNum;	// +0x1d
	unsigned short NpcSlotNum983;	// +0x1f (novo no 983; = sMailIncludeItem::NpcSlotNum983)
	unsigned long dwSetTID;		// +0x21
	int iSetItemCount;			// +0x25
	char csUCCIndex[9];			// +0x29
	unsigned long reserved32;	// +0x32
	unsigned char reserved36;	// +0x36
};

// 0xc4 bytes (o sMailInfoBrief do 645 tem 0xc2): resumo de carta da caixa de correio nos casos 0x210, 0x211 e 0x215 do
// CTask::OnPacketCommon do 983 (009d38b2, 009d3c19, 009d4152). Os primeiros 0x8d bytes sao os do 645.
struct sMailInfoBrief983
{
	unsigned long id;			// +0x00
	char sender[22];			// +0x04
	char reserved[0x6e];		// +0x1a
	bool bRead;					// +0x88
	int itemCount;				// +0x89
	sMailIncludeItem983 item;	// +0x8d
};
#pragma pack(pop)
C_ASSERT(sizeof(sMailIncludeItem983) == 0x37);
C_ASSERT(sizeof(sMailInfoBrief983) == 0xc4);
C_ASSERT(sizeof(sMailIncludeItem) == 0x37);
C_ASSERT(offsetof(sMailIncludeItem, NpcSlotNum983) == 0x1f);
C_ASSERT(offsetof(sMailIncludeItem, dwSetTID) == 0x21);
C_ASSERT(sizeof(sMailInfoBrief) == 0xc4);

// Item e resumo do 983 -> structs do fonte (as listas do CSharedDoc, do CTransferableItem e do sMailInfo continuam com os
// tipos do fonte; A765: sMailIncludeItem ja tem o layout do 983, 0x37, com NpcSlotNum983 em +0x1f).
inline sMailIncludeItem MailIncludeItemFrom983(const sMailIncludeItem983& r)
{
	sMailIncludeItem m;
	m.dwIDX = r.dwIDX;
	m.dwTID = r.dwTID;
	m.btItemType = r.btItemType;
	m.iCount = r.iCount;
	m.iTimeCount = r.iTimeCount;
	for (int i = 0; i < 5; ++i)
		m.soCom[i] = r.soCom[i];
	m.CharacterSlotNum = r.CharacterSlotNum;
	m.CaddieSlotNum = r.CaddieSlotNum;
	m.NpcSlotNum983 = r.NpcSlotNum983;
	m.dwSetTID = r.dwSetTID;
	m.iSetItemCount = r.iSetItemCount;
	memcpy(m.csUCCIndex, r.csUCCIndex, sizeof(m.csUCCIndex));
	m.reserved30 = r.reserved32;
	m.reserved34 = r.reserved36;
	return m;
}

inline sMailInfoBrief MailInfoBriefFrom983(const sMailInfoBrief983& r)
{
	sMailInfoBrief m;
	memcpy(&m, &r, 0x8d);
	m.item = MailIncludeItemFrom983(r.item);
	return m;
}
