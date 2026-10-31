// giftpackets.h - frente C (pacotes). JP 983: registro de cada item da compra na loja (pacote 0x1d,
// FrCheckoutDlg::OnConfirmResult 00aa25f0) e do presente (pacote 0x1f, FrGiftDlg::GiftProgress 00aaf840). Os nomes sao
// nossos; tamanho e campos medidos no exe 983 (EncodeBuffer de 0x25, registro zerado com guid = -1 nas duas funcoes).
// O KR 645 mandava 0x10 bytes (guid, tid, dias, opcao, quantidade).
#pragma once

#pragma pack(push, 1)
struct sShopBuyItem983
{
	unsigned long guid;			// +0x00 (-1)
	unsigned long tid;			// +0x04
	unsigned short days;		// +0x08 dias/tempo
	unsigned short option;		// +0x0a indice do item filho/dia (0xffff sem)
	unsigned long count;		// +0x0c
	unsigned long pang;			// +0x10 preco em pang (CItemManager 00a1f680) / aluguel de peca com cookie
	unsigned long cookie;		// +0x14 preco em cookie (item de cash)
	unsigned long partValue;	// +0x18 983: pItem+0x238 nas 35 pecas de 00d46118 = fora do sPart (no do map); zero
	char partText[9];			// +0x1c 983: strcpy de pItem+0x23c (lixo do heap); zero
};
#pragma pack(pop)
C_ASSERT(sizeof(sShopBuyItem983) == 0x25);
