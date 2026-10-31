// guildpackets.h - frente C (pacotes). JP 983: bloco da guilda de um jogador (caso 0x15d do CTask::OnPacketCommon,
// 009d13e6). Os nomes sao nossos; tamanho (0x4d) e deslocamentos lidos no exe 983. O 645 lia um GUILD_INFO inteiro (0x129).
#pragma once

#pragma pack(push, 1)
struct sGuildBrief983
{
	unsigned long guildUID;		// +0x00 (nao lido pelo caso; suposto pelo GUILD_USER_INFO do 645)
	unsigned char srvByte04_983;	// +0x04 vem do servidor; o caso 0x15d do 983 nao le (D952)
	char guildName[20];			// +0x05 -> sPangYaUserInfo::sGuild
	char guildMark[12];			// +0x19 -> sPangYaUserInfo::szEmblemName
	unsigned long emblemVer;	// +0x25 -> sPangYaUserInfo::dwEmblemVer (no do mapa +0xa2)
	unsigned char srvData29_983[8];	// +0x29 idem, sem leitor
	unsigned long guildPang;	// +0x31 -> sUserInfoTime::dwGuildPang
	unsigned char srvData35_983[20];	// +0x35 idem, sem leitor
	unsigned long guildPoint;	// +0x49 -> sUserInfoTime::dwGuildPoint (so para o proprio jogador)
};
#pragma pack(pop)
C_ASSERT(sizeof(sGuildBrief983) == 0x4d);
C_ASSERT(offsetof(sGuildBrief983, guildMark) == 0x19 && offsetof(sGuildBrief983, emblemVer) == 0x25);
C_ASSERT(offsetof(sGuildBrief983, guildPang) == 0x31 && offsetof(sGuildBrief983, guildPoint) == 0x49);

// casos 0x116-0x119: sGuildJoinEntry983 (CSharedDoc+0x10fe8) em guildlist983.h
#include "guildlist983.h"

// Visao (sem corpo: a classe e da frente A) do metodo que os casos 0x117/0x119 chamam depois de mexer nos vetores.
// 00900ba0: metodo da FrMessengerDlg que refaz m_pGuildList a partir de CSharedDoc+0x10ff8. (O 008fafc0 do caso 0x116 e o
// FrGuildManageDlg::RefreshList983 da frente B, include/jp/guildmanagedlg.h, desde a C64.)
struct RbMessengerDlgGuild983
{
	void RefreshGuildInvite983();	// 00900ba0
};

// 009b9470 (unidade taskmanager, argumento em ecx): aviso do mensageiro para o codigo de erro dos casos 0x117 e 0x119.
// Tabela de saltos 009b94d4 (codigo - 3, 9 entradas; o 4 cai no texto geral 00d1508c).
inline void GuildInviteNotify983(int code)
{
	const char* text;
	switch (code)
	{
	case 3:
		text = "\xc7\xd8\xb4\xe7 \xc0\xaf\xc0\xfa\xb0\xa1 \xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.";
		break;
	case 5:
		text = "\xc7\xd8\xb4\xe7 \xc0\xaf\xc0\xfa\xb4\xc2 \xc0\xcc\xb9\xcc \xb4\xd9\xb8\xa5 \xb1\xe6\xb5\xe5\xbf\xa1 \xb0\xa1\xc0\xd4\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.";
		break;
	case 6:
		text = "\xc0\xcc\xb9\xcc \xc3\xca\xb4\xeb\xb5\xc8 \xc0\xaf\xc0\xfa\xc0\xd4\xb4\xcf\xb4\xd9.";
		break;
	case 7:
		text = "\xb1\xe6\xb5\xe5 \xb0\xfc\xb8\xae\xc0\xda\xb8\xb8 \xb1\xe6\xb5\xe5\xc3\xca\xb4\xeb\xb8\xa6 \xc7\xd2\xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.";
		break;
	case 8:
		text = "\xc3\xd6\xb4\xeb \xb1\xe6\xb5\xe5\xc3\xca\xb4\xeb \xc0\xce\xbf\xf8\xbc\xf6\xb4\xc2 10\xb8\xed \xc0\xd4\xb4\xcf\xb4\xd9.";
		break;
	case 9:
		text = "\xc3\xd6\xb4\xeb 5\xb0\xb3\xc0\xc7 \xb1\xe6\xb5\xe5\xb8\xb8 \xc3\xca\xb4\xeb\xc7\xd2\xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.";
		break;
	case 10:
		text = "\xc7\xd8\xb4\xe7 \xb1\xe6\xb5\xe5\xb4\xc2 \xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.";
		break;
	case 11:
		text = "\xb1\xe6\xb5\xe5 \xb0\xa1\xc0\xd4 \xbf\xe4\xc3\xbb\xc0\xbb \xc7\xcf\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9.";
		break;
	default:
		text = "\xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
		break;
	}
	CMessengerInfo::Instance()->Notify1(K2L_Compatibility(text));
}
