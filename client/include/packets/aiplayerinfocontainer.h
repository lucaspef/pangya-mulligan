// aiplayerinfocontainer.h - frente C (pacotes). JP 983: documento 0xa3 do CContentsDoc (RTTI .?AVCAIPlayerInfoContainer@@,
// vtable 00d3c658, 0x34 bytes; construtor 00915b90, destrutor 00915ca0). Guarda, para os jogadores de IA do modo solo:
// as linhas de cada buraco lidas dos arquivos "map%02d_%s_hole%02d.aibin" (+0x04), o resultado de cada IA por buraco que
// o servidor manda no pacote 0x256 (+0x14) e o número do adversário pronto de cada IA (+0x24).
// A B chama este documento pela visão `CAIDocJp` (include/jp/aiplayer.h); os corpos dos métodos de lá estão em
// src/projectg/jp/pktviews983.cpp lendo por esta classe (pedido C75.1). Nomes nossos. Unidade do 983: 00907960-00915d8e.
#pragma once

#include <map>
#include <vector>
#include "icontentsdatacontainer.h"
#include "nodecontainer.h"
#include "jp/aiplayer.h"

class WReceivedPacket;
class cFile;

// cabeçalho do .aibin (0x2c; o mesmo começo do sHeader do WPolySoup; construtor fora de linha 00907970)
struct sAiBinHeader983
{
	__declspec(noinline) sAiBinHeader983() // 00907970
	{
		id[0] = 0;
		version = 0;
		modelNum = 0;
		modelNum1 = 0;
		modelNum2 = 0;
		cameraNum = 0;
		pointNum = 0;
		areaNum = 0;
		textureNum = 0;
		nodeNum = 0;
		attachNum = 0;
	}
	char id[4];
	int version; // +0x04: o 983 só aceita >= 0x72
	int modelNum;
	int modelNum1;
	int modelNum2;
	int cameraNum;
	int pointNum;
	int areaNum;
	int textureNum;
	int nodeNum; // +0x24: número de linhas
	int attachNum; // +0x28
};

// uma linha do .aibin (0x28) seguida de `count` pontos de 0x0c
struct sAiBinLine983
{
	char name[0x20]; // "…ai…" marca a linha como tipo 0xf
	int count; // +0x20
	int reserved; // +0x24
};

class CAIPlayerInfoContainer983 : public IContentsDataContainer
{
public:
	CAIPlayerInfoContainer983(); // 00915b90
	virtual bool Initialize(); // 00910aa0: esvazia os três
	virtual bool Release(); // 00907960: Initialize()

	// 00910b80 (C-PKT lote 68): pacote 0x256 - por IA (Decode2 = quantas): número do adversário (Decode4) e N
	// registros de 0x28 (Decode1 = N), um por buraco; chamado pelo CTask::OnPacketCommon.
	void ReadPacket983(WReceivedPacket& packet);
	// 0090f720: linhas de um .aibin (versão >= 0x72); não lê o this
	bool ReadAiBin983(cFile* pFile, std::vector<CNodeContainer>* pOut);

	std::map<int, std::vector<CNodeContainer> > m_lineMap; // +0x04: buraco -> linhas (00911180)
	std::map<int, std::vector<sAIDocInfo983> > m_infoMap; // +0x14: IA -> resultado por buraco (pacote 0x256)
	std::vector<unsigned long> m_npcIds; // +0x24: IA -> número do adversário pronto (0 = sem)
};
