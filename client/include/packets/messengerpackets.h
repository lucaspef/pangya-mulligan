// messengerpackets.h - frente C (pacotes). JP 983: blocos do sub-protocolo do mensageiro (caso 0x30 do CTask::OnPacketCommon,
// 009c70b0). Os nomes são nossos; tamanho e campos medidos no exe 983.
#pragma once

#include <vector>
// usa sUserPosition (src/shared/globalgamedefine.h): incluir depois dele

#pragma pack(push, 1)
// sub-caso 0x115 (009c80f9): estado de um amigo. Bloco de 0x54 lido de uma vez (construtor 009baa30).
struct sFriendStateNotify983
{
	unsigned long uid;		// +0x00
	unsigned long state;	// +0x04 (0 jogando, 1 ausente, 3 ocupado, 4 voltou, 5 saiu da partida)
	unsigned char srvByte983;	// +0x08 vem do servidor; o 983 nao le (009c80f9 so usa uid/estado/pos.iServerGUID; ctor 009baa30 = 0)
	sUserPosition pos;		// +0x09

	sFriendStateNotify983()
	{
		uid = 0xffffffff;
		state = 5;
		srvByte983 = 0;
	}
};
#pragma pack(pop)
C_ASSERT(sizeof(sFriendStateNotify983) == 0x54);
C_ASSERT(offsetof(sFriendStateNotify983, pos.iServerGUID) == 0xf);

// sub-caso 0x127 / caso 0x118: sGuildInvite983 (CSharedDoc+0x10ff8) em guildlist983.h
#include "guildlist983.h"
