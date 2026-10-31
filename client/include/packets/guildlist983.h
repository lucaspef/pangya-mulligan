// guildlist983.h - elementos dos dois vetores de guilda do CSharedDoc (983 +0x10fe8 / +0x10ff8). Vieram do
// guildpackets.h / messengerpackets.h (frente C) para o shareddoc.h poder tipar os vetores sem os casts.
#pragma once

// casos 0x116-0x119 do CTask::OnPacketCommon (009cfb3d-009cfd50): lista de pedidos de entrada na guilda (quem pediu) e de
// convites recebidos, mostradas pelo mensageiro.
#pragma pack(push, 1)
// caso 0x116: elemento (0x37) do vetor de CSharedDoc+0x10fe8 (construtor 008fa4a0, copia 009ba520, push_back 009c0820,
// esvaziar 009bf8a0). So o comeco e o fim foram lidos: FrGuildManageDlg 008fafc0 separa as linhas por +0x35 (2 e outro valor).
struct sGuildJoinEntry983
{
	unsigned long dw0;			// +0x00 (-1 no construtor)
	unsigned long dw4;			// +0x04 (-1 no construtor)
	unsigned char m_pad983_08[0x2c];	// +0x08 (nao lido)
	unsigned char sexIcon;		// +0x34: indice dos icones de sexo (FrGuildManageDlg 008fafc0, & 7)
	unsigned short state;		// +0x35

	sGuildJoinEntry983() { dw0 = 0xffffffff; dw4 = 0xffffffff; }
};
#pragma pack(pop)
C_ASSERT(sizeof(sGuildJoinEntry983) == 0x37);
C_ASSERT(offsetof(sGuildJoinEntry983, state) == 0x35);

// sub-caso 0x127 (009c82c4) e caso 0x118 (009cfc0a): convite de guilda recebido; elemento (0x2c) do vetor de CSharedDoc+0x10ff8
// (construtor 008fe430, cópia 009ba540 = rep movsd de 0xb dwords, push_back 009c08a0, esvaziar 009bf8f0).
// O 983 monta um por volta do laço no caso 0x118 e não chama destrutor: os quatro dwords do fim não são um std::vector
// (embora o desenho - três zerados + um solto - seja o do std::vector deste build, que tem 0x10).
struct sGuildInvite983
{
	unsigned long dw0;			// +0x00: 2o Decode4 do pacote (-1 no construtor)
	unsigned long dw4;			// +0x04: 1o Decode4 do pacote
	char szName[20];			// +0x08: nome da guilda
	unsigned long dw1c[3];		// +0x1c: zerados no construtor (uso não lido)
	unsigned long dw28;			// +0x28: o construtor não toca

	sGuildInvite983() { dw0 = 0xffffffff; dw1c[0] = dw1c[1] = dw1c[2] = 0; }
};
C_ASSERT(sizeof(sGuildInvite983) == 0x2c);
C_ASSERT(offsetof(sGuildInvite983, dw1c) == 0x1c);
