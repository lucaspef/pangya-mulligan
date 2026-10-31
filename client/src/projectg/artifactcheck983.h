#pragma once

// 983 00479bc0 (unidade 00478cc2-0047a060, a do CItemCursor983): pode a sala comecar com o artefato escolhido? `bFail` com o
// texto do aviso e o nome da imagem ("mana_limited" / "mana_no"). Declaracao IGUAL a da copia local da B em
// jp/gameroomdlg.cpp (sessao B44); corpo no changeroominfodlg.cpp (A s159).
struct sArtifactCheckArg983
{
	unsigned char* pFlag; // +0x00: CItemCursor983::GetFlag (o jogador pode usar o artefato)
	unsigned char* pRoomType; // +0x04: CSharedDoc+0x1094c (sRoomInfo::gameType)
	unsigned char* pMode; // +0x08: CSharedDoc+0x109b8 (sRoomInfo::realGameType)
	unsigned char* p1094b; // +0x0c: sRoomInfo::nHole
};
struct sArtifactCheck983
{
	sArtifactCheck983(const unsigned long* pTid, const sArtifactCheckArg983* pArg); // 00479bc0

	bool b0; // +0x00
	bool bFail; // +0x01
	const char* text; // +0x04
	const char* name; // +0x08
};
