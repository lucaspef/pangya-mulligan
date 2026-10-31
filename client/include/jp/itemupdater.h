#pragma once
#include "vc10_layout.h"
// JP 983: CItemUpdater, unidade 005846c0–0058c3f0 (o corte da F3.7.1 dizia 005846c0–005850e0; o "semclasse_00589b4f" é dela).
// Aplica no CSharedDoc as mudanças de item do pacote 0x216 (só 983; o OnPacketCommon decodifica cada registro com 009bfc90 e
// chama o embrulho 00633680 do CSharedDoc, que repassa à entrada Update983 0058c160). Classe de 4 B (só vtable 00d0b404),
// num std::tr1::shared_ptr em CSharedDoc+0x11628 (ctor do shared_ptr 00589b50, chamado pelo CSharedDoc::CSharedDoc).
// É o sucessor do CTaskManager::AddNewItemToDoc do 645: mesmas listas por categoria (typeId >> 26), agora com remoção
// (count < 0), cartas, oficina de tacos, PCL/mestria do personagem e um código de erro por caso (0x1e8501…, 2000129…).
// Todos os métodos recebem (agora, registro): `now` é a hora do servidor (__time64_t; o OnPacketCommon passa o 1º Decode4 do
// pacote estendido a 64 bits) — vai para o _difftime64 de 005855a0 nos itens com prazo (B122; na B121 eram dois int a/b).
// Sessão B121; nomes dos métodos e dos campos do registro são meus.

#include <string>
#include <memory>

class WReceivedPacket;
struct sItemInfo;
struct sCharacterInfo;
struct sCards;
namespace _quest { struct sQuestCount983; class CQuestItem; }

// bloco da oficina de tacos do registro (+0x54, 0x1c; copiado como um só pelo construtor de cópia 00589730)
struct sItemUpdateWorkshop983
{
	short slots[5]; // +0x0 (registro +0x54) → sItemInfo +0xaa (5 short, ver clubsetstat.h)
	int exp; // +0xc (registro +0x60) → sItemInfo +0xb4
	int srvByte983; // +0x10 (registro +0x64) → sItemInfo +0xbc (o construtor põe -1)
	int srvInt983; // +0x14 (registro +0x68) → sItemInfo +0xc0
	int recovery983; // +0x18 (registro +0x6c) → sItemInfo +0xb8 (cancelamentos já usados, sItemInfo::recoveryUsed983)
};

// registro do pacote 0x216 (0x7c; construtor 009be6c0 e leitor 009bfc90 no taskmanager.cpp da frente A)
struct sItemUpdate983
{
	int kind; // +0: 2 = item comum (Add/Remove pela categoria), 0xc9..0xce = mudanças de atributo
	unsigned long typeId; // +4
	unsigned long uid; // +8
	int periodType; // +0xc: ItemType do sItemInfo (1..6; ver PeriodSeconds983)
	int prevTotal983; // +0x10: total anterior (UpdateItem983: novo <= 20000 e anterior > 20000 desliga o aviso de itens demais)
	int total; // +0x14: total novo (0 apaga o item); nos itens com prazo é o fim do prazo (time_t, B122)
	int count; // +0x18: < 0 = remover
	short counters[5]; // +0x1c: Common[5] do sItemInfo / PCL do personagem
	std::string text; // +0x28
	int uccStatus983; // +0x44 → sItemInfo::status da UCC (NewUccItem983 00589e50)
	int uccSeq983; // +0x48 → sItemInfo::Seq da UCC (00589e50)
	unsigned long cardTypeId; // +0x4c
	unsigned char cardSlot; // +0x50: 1.. (0 = nenhum)
	sItemUpdateWorkshop983 workshop; // +0x54
	unsigned long mastery; // +0x70
	unsigned char caddieLevel; // +0x74
	unsigned long caddieExp; // +0x78

	sItemUpdate983(); // 009be6c0 (frente A)
	void Decode983(WReceivedPacket& packet); // 009bfc90 (leitor; corpo da frente C no taskmanager.cpp, pedido C59.1)
};

// sItemCount983 (elemento de CSharedDoc::m_itemCounts983, +0x3828, 12 B): contagem de item da categoria 0xb
// (typeId com os bits 24..25 = 3); nome meu (B122). Definido no shareddoc.h desde a A683.

class CItemUpdater
{
public:
	CItemUpdater(); // 005846c0
	virtual ~CItemUpdater(); // 005846d0 (??_G 00585620)

	int Update983(__time64_t now, const sItemUpdate983& rec); // 0058c160

private:
	// mudanças de atributo (kind 0xc9..0xce)
	int UpdateCounters983(__time64_t now, const sItemUpdate983& rec); // 00589ba0 (0xc9)
	int UpdateCard983(__time64_t now, const sItemUpdate983& rec); // 00589bf0 (0xcb)
	int UpdateWorkshop983(__time64_t now, const sItemUpdate983& rec); // 00589c60 (0xcc)
	int UpdateMastery983(__time64_t now, const sItemUpdate983& rec); // 00589ca0 (0xcd)
	int UpdateCaddieLevel983(__time64_t now, const sItemUpdate983& rec); // 00589cd0 (0xce)
	// item comum (kind 2)
	int UpdateItem983(__time64_t now, const sItemUpdate983& rec); // 0058c080
	int Add983(__time64_t now, const sItemUpdate983& rec); // 0058be40
	int Remove983(__time64_t now, const sItemUpdate983& rec); // 0058bb50

	// buscas no CSharedDoc pelo uid do registro
	sItemInfo* FindPart983(const sItemUpdate983& rec); // 00588230 (m_partsList)
	sItemInfo* FindUccItem983(const sItemUpdate983& rec); // 00588270 (m_uccItemList)
	sItemInfo* FindBall983(const sItemUpdate983& rec); // 005882b0 (m_ballList)
	sItemInfo* FindMyItem983(const sItemUpdate983& rec); // 005882f0 (m_myItemList)
	sItemInfo* FindMyItem2_983(const sItemUpdate983& rec); // 00588330 (só repassa)
	sItemInfo* FindCutin983(const sItemUpdate983& rec); // 00588340 (m_cutinList)
	_quest::sQuestCount983* FindQuestCount983(const sItemUpdate983& rec); // 00588470 (CSharedDoc::m_questCounts983, 0x3840)
	sItemInfo* FindAuxPart983(const sItemUpdate983& rec); // 00588510 (m_auxPartList)
	_quest::CQuestItem* FindQuest983(const sItemUpdate983& rec); // 00588550 (CSharedDoc::m_quests983, 0x384c)
	sCards* FindCard983(const sItemUpdate983& rec); // 005885c0 (m_cardList)
	sCharacterInfo* FindCharacter983(const sItemUpdate983& rec); // 00588b50 (m_charMap)

	// personagem
	int SetCharacterPCL983(__time64_t now, const sItemUpdate983& rec); // 00588ba0
	int SetCharacterCard983(__time64_t now, const sItemUpdate983& rec); // 00588c00
	int SetCharacterMastery983(__time64_t now, const sItemUpdate983& rec); // 00588d30
	int SetHairStyle983(__time64_t now, const sItemUpdate983& rec); // 00588380 (categoria 0xf)
	// partes / bolas / itens / ucc / aux / cartas
	int SetPartCard983(__time64_t now, const sItemUpdate983& rec); // 00588d60
	int RemovePart983(__time64_t now, const sItemUpdate983& rec); // 00588e80
	int RemoveUccItem983(__time64_t now, const sItemUpdate983& rec); // 00588f00
	int RemoveBall983(__time64_t now, const sItemUpdate983& rec); // 00589270
	int RemoveMyItem983(__time64_t now, const sItemUpdate983& rec); // 00589380
	int RemoveMyItem2_983(__time64_t now, const sItemUpdate983& rec); // 005893e0 (só repassa)
	int RemoveQuestCount983(__time64_t now, const sItemUpdate983& rec); // 005895b0
	int RemoveAuxPart983(__time64_t now, const sItemUpdate983& rec); // 00589610
	int RemoveCard983(__time64_t now, const sItemUpdate983& rec); // 005896d0
	int AddQuestCount983(__time64_t now, const sItemUpdate983& rec); // 005884b0 (categoria 0x1b)
	int NewQuestCount983(__time64_t now, const sItemUpdate983& rec); // 005861a0
	// tacos (m_clubSetMap)
	int RemoveClubSet983(__time64_t now, const sItemUpdate983& rec); // 00588f80
	int SetClubSetCounters983(__time64_t now, const sItemUpdate983& rec); // 00588ff0
	int SetClubSetCard983(__time64_t now, const sItemUpdate983& rec); // 00589070
	int SetClubSetWorkshop983(__time64_t now, const sItemUpdate983& rec); // 005891c0
	// caddie / mascote
	int RemoveCaddie983(__time64_t now, const sItemUpdate983& rec); // 00589400
	int SetCaddieLevel983(__time64_t now, const sItemUpdate983& rec); // 00589460
	int SetCaddiePart983(__time64_t now, const sItemUpdate983& rec); // 005894c0 (categoria 8)
	int RemoveMascot983(__time64_t now, const sItemUpdate983& rec); // 00589550

	// Remove983 por categoria (B122): count < 0 baixa a contagem; o total novo (+0x14) zerado apaga o item
	int RemovePartItem983(__time64_t now, const sItemUpdate983& rec); // 0058ab80 (categoria 2; UCC → 00589e10)
	int RemoveUccChecked983(__time64_t now, const sItemUpdate983& rec); // 00589e10
	int UpdateBallCount983(__time64_t now, const sItemUpdate983& rec); // 00589f40 (categoria 5)
	int UpdateMyItemCount983(__time64_t now, const sItemUpdate983& rec); // 0058aea0 (categoria 6)
	int UpdateMyItemTime983(__time64_t now, const sItemUpdate983& rec); // 0058a180 (categoria 6, subtipo 1: item com prazo)
	int UpdateQuestCount983(__time64_t now, const sItemUpdate983& rec); // 0058a510 (categoria 0x1b)
	int UpdateAuxPartCount983(__time64_t now, const sItemUpdate983& rec); // 0058a570 (categoria 0x1c)
	int UpdateCardCount983(__time64_t now, const sItemUpdate983& rec); // 0058a760 (categoria 0x1f)
	int RemoveQuestItem983(__time64_t now, const sItemUpdate983& rec); // 0058ba40 (categoria 0x13; só repassa)
	int RemoveQuest983(__time64_t now, const sItemUpdate983& rec); // 0058b420 (categoria 0x1e)
	int EraseQuest983(__time64_t now, const sItemUpdate983& rec); // 0058a6c0
	// Add983 por categoria (B122)
	int AddCharacter983(__time64_t now, const sItemUpdate983& rec); // 0058b690 (categoria 1; só repassa)
	int NewCharacter983(__time64_t now, const sItemUpdate983& rec); // 0058aac0
	int AddPart983(__time64_t now, const sItemUpdate983& rec); // 0058b6b0 (categoria 2)
	int AddUccItem983(__time64_t now, const sItemUpdate983& rec); // 0058acb0
	int NewUccItem983(__time64_t now, const sItemUpdate983& rec); // 00589e50
	int NewPart983(__time64_t now, const sItemUpdate983& rec); // 0058abe0
	int NewPartTimed983(__time64_t now, const sItemUpdate983& rec); // 00589d10
	int AddClubSet983(__time64_t now, const sItemUpdate983& rec); // 0058b770 (categoria 4)
	int NewClubSet983(__time64_t now, const sItemUpdate983& rec); // 0058acf0
	int AddBall983(__time64_t now, const sItemUpdate983& rec); // 0058add0 (categoria 5)
	int NewBall983(__time64_t now, const sItemUpdate983& rec); // 00589fb0
	int AddMyItem983(__time64_t now, const sItemUpdate983& rec); // 0058b7d0 (categoria 6)
	int AddMyItemTime983(__time64_t now, const sItemUpdate983& rec); // 0058b030 (categoria 6, subtipo 1)
	int NewMyItemTime983(__time64_t now, const sItemUpdate983& rec); // 0058a230
	int NewMyItem983(__time64_t now, const sItemUpdate983& rec); // 0058af40
	int NewMyItemTimed983(__time64_t now, const sItemUpdate983& rec); // 0058a080
	int AddCaddie983(__time64_t now, const sItemUpdate983& rec); // 0058bce0 (categoria 7)
	int NewCaddie983(__time64_t now, const sItemUpdate983& rec); // 0058b860
	int NewCaddieTimed983(__time64_t now, const sItemUpdate983& rec); // 0058b0b0
	int AddItemCount983(__time64_t now, const sItemUpdate983& rec); // 0058bb20 (categoria 0xb)
	int UpdateItemCount983(__time64_t now, const sItemUpdate983& rec); // 0058b580
	int NewItemCount983(__time64_t now, const sItemUpdate983& rec); // 0058a880
	int AddCutin983(__time64_t now, const sItemUpdate983& rec); // 0058b910 (categoria 0xe)
	int NewCutin983(__time64_t now, const sItemUpdate983& rec); // 0058b160
	int NewCutinTimed983(__time64_t now, const sItemUpdate983& rec); // 0058a370
	int AddMascot983(__time64_t now, const sItemUpdate983& rec); // 0058bd90 (categoria 0x10)
	int NewMascot983(__time64_t now, const sItemUpdate983& rec); // 0058b980
	int NewMascotTimed983(__time64_t now, const sItemUpdate983& rec); // 0058b250
	int AddFurniture983(__time64_t now, const sItemUpdate983& rec); // 0058b310 (categoria 0x12; só repassa)
	int NewFurniture983(__time64_t now, const sItemUpdate983& rec); // 0058a450
	int AddQuestItem983(__time64_t now, const sItemUpdate983& rec); // 0058be20 (categoria 0x13; só repassa)
	int AddQuest983(__time64_t now, const sItemUpdate983& rec); // 0058bb00 (categoria 0x1e; só repassa)
	int NewQuest983(__time64_t now, const sItemUpdate983& rec); // 0058b470
	int AddAuxPart983(__time64_t now, const sItemUpdate983& rec); // 0058ba60 (categoria 0x1c)
	int NewAuxPart983(__time64_t now, const sItemUpdate983& rec); // 0058b330
	int NewAuxPartTimed983(__time64_t now, const sItemUpdate983& rec); // 0058a5c0
	int AddCard983(__time64_t now, const sItemUpdate983& rec); // 0058b540 (categoria 0x1f)
	int NewCard983(__time64_t now, const sItemUpdate983& rec); // 0058a7b0
};
// 00589b50: cria a instância do CSharedDoc+0x11628 (chamada pelo construtor do CSharedDoc, 0064a7ad); função livre (/Gr), nome meu
std::tr1::shared_ptr<CItemUpdater> CreateItemUpdater983();

C_ASSERT_VC10(sizeof(sItemUpdate983) == 0x7c);
C_ASSERT(sizeof(CItemUpdater) == 4);
