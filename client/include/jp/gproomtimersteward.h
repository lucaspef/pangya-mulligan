#pragma once
#include "vc10_layout.h"
// JP 983: unidade gproomtimersteward (0055ce30–0055ee70). Sessão B67. Classes novas do 983 (sem par no 645): o relógio do Grand Prix
// (CGrandPrixClock, WSingleton em 00e74e8c), o aviso de abertura de cada Grand Prix (CGPRoomTimerSteward, um CTimerEvent por
// typeid), o quadro de horário/limite da FrGPTypeRoomDlg (struct de 0x7c sem RTTI) e os auxiliares das linhas da lista do Grand
// Prix (sGPRoomItem983). Nomes de classe pela RTTI; nomes de método e de campo meus.
#include "singleton.h"
#include "wtimer.h"
#include "grandprixtimebase.h"
#include "grandprix983.h"

class Bitmap;
class WTitleFontEx;
class FrGraphicInterface;

// relógio do servidor: acertado pelo pacote do servidor (Sync983, 009c20a0), anda pelo CProjectG::Process (Update983, 00a68f20) e
// manda 0x753e / 3 ao "Gateway" a cada minuto. O desenho (Draw983) é da FrGrandPrixRoomDlg (0050f700); as imagens vêm do
// CLobbyMain::OnPreLoadInit (LoadBitmaps983, 00ab9800). Criado em 00a65710 com `new` de 0x80.
class CGrandPrixClock : public WSingleton<CGrandPrixClock>
{
public:
	CGrandPrixClock(); // 0055d250
	virtual ~CGrandPrixClock(); // 0055d2c0 (apagador 0055db50)

	bool LoadBitmaps983(); // 0055cef0
	void GetTime983(SYSTEMTIME* pOut); // 0055cf50: data do servidor com a hora do relógio (hora de 24)
	void Draw983(FrGraphicInterface* pGdi); // 0055d2e0
	void Update983(float dt); // 0055db80
	void Sync983(); // 0055dc50: acerta pela hora do servidor (CSharedDoc::m_serverTime) e liga o relógio
	__int64 GetNow983(); // 0055dcd0: a hora do relógio na data do servidor (segundos; S5 00b30870)

	SYSTEMTIME m_time; // +0x28
	CGrandPrixHour m_hour; // +0x38
	CGrandPrixMinute m_minute; // +0x4c
	CGrandPrixSecond m_second; // +0x54
	float m_elapsed; // +0x5c
	int m_bRunning; // +0x60
	int m_bShow; // +0x64
	float m_x; // +0x68
	float m_y; // +0x6c
	WTitleFontEx* m_pFont; // +0x70
	const Bitmap* m_pAm; // +0x74: "gp_time_am"
	const Bitmap* m_pPm; // +0x78: "gp_time_pm"
	const Bitmap* m_pColon; // +0x7c: "gp_timecount_w" (pisca nos segundos pares)
};
C_ASSERT_VC10(sizeof(CGrandPrixClock) == 0x80);

// aviso de abertura de um Grand Prix: o CGrandPrixDocument (00555070) cria um por typeid e o põe no CTimerManager; no disparo,
// texto 0x753e no CNoticeBoard (hora, minuto e nome) e o aviso 0x753e / 10 ao "Gateway" com o typeid
class CGPRoomTimerSteward : public CTimerEvent
{
public:
	CGPRoomTimerSteward(); // 0055d180 (fora de linha: o CGrandPrixDocument 00555070 o chama)
	virtual ~CGPRoomTimerSteward() { } // 0055d1a0 (apagador 0055db20)

	virtual void OnTimer(); // 0055df30
	void SetTypeId983(unsigned long typeId) { m_typeId = typeId; } // 0055d150 (o 983 guarda a cópia fora de linha)

	unsigned long m_typeId; // +0x20
};
C_ASSERT_VC10(sizeof(CGPRoomTimerSteward) == 0x24);

// quadro de horário e de limite da FrGPTypeRoomDlg (sem RTTI; `new` de 0x7c em 00512f80, desenho em 005126d0): horário de começo e
// de fim do Grand Prix atual, caixa "item limitado" e fundo de "encerrado" (9 pedaços)
struct sGPTimeBoard983
{
	sGPTimeBoard983(); // 0055cfa0
	~sGPTimeBoard983(); // 0055d020 (`ret`, fora de linha: o FrGPTypeRoomDlg::Leave983 00511020 e o apagador 00510bc0 o chamam)
	void DrawTime983(float x, float y); // 0055d030: as quatro casas (hora/minuto de começo e de fim)
	void SetFont983(WTitleFontEx* pFont); // 0055d150 (fora de linha: FrGPTypeRoomDlg::OnInit 00516fc0)
	int IsStartPM983(); // 0055d160 (fora de linha: o desenho 005128e0 chama)
	int IsEndPM983(); // 0055d170
	void DrawLimited983(float x, float y); // 0055d480
	void DrawClosed983(float x, float y); // 0055d660
	bool Init983(); // 0055dd40

	WTitleFontEx* m_pFont; // +0x00
	const Bitmap* m_pLimited[3]; // +0x04: "gp_limitedbox1..3"
	const Bitmap* m_pLimitedText; // +0x10: "gp_txt_item_limited"
	const Bitmap* m_pClosed[9]; // +0x14: "gp_closed_background1..9"
	const Bitmap* m_pClosedText; // +0x38: "txt_gp_closed"
	CGrandPrixHour m_start; // +0x3c
	CGrandPrixMinute m_startMin; // +0x50
	CGrandPrixHour m_end; // +0x58
	CGrandPrixMinute m_endMin; // +0x6c
	bool m_bLimited; // +0x74: o Grand Prix pede equipamento e o jogador o veste (GPHasTicket983)
	bool m_bClosed; // +0x75: byte +0x10488 do CSharedDoc
	sGPInfo983* m_pInfo; // +0x78
};
C_ASSERT(sizeof(sGPTimeBoard983) == 0x7c);

// auxiliares das linhas da lista (o 983 passa a linha em ecx)
void GPItemCheckScore983(sGPRoomItem983* pItem, float value); // 0055dff0: bits 4/8 de `flags` pelos limites +0x98/+0x9c
unsigned short GPHour12_983(unsigned short hour); // 0055e060: hora em 12 h (0 e 24 viram 12; fora de 0–24, 0x63)
void GPItemCheckLevel983(sGPRoomItem983* pItem); // 0055e160: bits 1/2 de `flags` (nível mínimo/máximo)
void GPItemCheckSpecialHole983(sGPRoomItem983* pItem); // 0055e190: bit 1 de `opts`
void GPItemCheckRankReward983(sGPRoomItem983* pItem); // 0055e1c0: bit 2 de `opts`
void GPItemUpdateState983(sGPRoomItem983* pItem, __int64* pNow, int typeId); // 0055e1f0
bool GPItemCheckCondition983(sGPRoomItem983* pItem); // 0055e310: sem o item de condição liga 0x80 em `flags`
bool GPHasConditionEquip983(unsigned long key); // 0055e350: há lista no GrandPrixConditionEquip.iff
void GPItemBuildTitle983(sGPRoomItem983* pItem); // 0055e890: `title` = texto da regra especial
bool GPHasTicket983(unsigned long key); // 0055ea60: o jogador veste o equipamento pedido (personagem, taco, caddie, mascote)
void GPItemCheckEquip983(sGPRoomItem983* pItem); // 0055ec80: sem o equipamento pedido liga 0x10 em `flags`
