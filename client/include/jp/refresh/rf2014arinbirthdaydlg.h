#pragma once
#include "vc10_layout.h"
#include <string>
#include <vector>
#include "rfformmodalbase.h"

// JP 983: janelas RF do evento de aniversário da Arin 2014 (unidade rf2014arinbirthdaydlg.cpp, 005d8160–005dfa30; sessão B143,
// F3.7.12 item 7). Os cortes rf2014arinnotifydlg/rf2014arinstage1wnd/rf2014arinstage2wnd da F3.7.1 são UMA unidade (começa nos
// acessores de instância 005d8160+, que a B136 tirou da rfservernoticedlg). Classes pela RTTI (namespace _gateway); métodos e
// campos com nome meu (…983), menos os da RF. A visão parcial da B127 (include/jp/rf2014arinbirthdaydlg.h) agora inclui este.
// - RF2014ArinBirthdayDlg ("event_arinbirth_dialog.xml"; vtables 00d11f0c / 00d11f00; 0x1a0; modal): a janela do evento em 12
//   passos (+0x18c, 0..0xb): texto do passo (13 textos 0x30e08..0x30e14), figura (Tiki/Arin), fundo; abre a janela dos bilhetes
//   (passo 2) e a da magia (passos 9 e 0xb). Instância 00e78248, pai 00e78244. Aberta pelo CArinBirthDay2014Handler.
// - RF2014ArinStage1Wnd ("event_arinbirth_ticket_wnd.xml"; vtable 00d11784; 0x1a0; não modal): 5 bilhetes (itens da tabela
//   00d11768), cada um usado uma vez; os 5 usados mandam 0x191. Pai 00e7824c.
// - RF2014ArinStage2Wnd ("event_arinbirth_magic_wnd.xml"; vtable 00d11ac4; 0x1a0; não modal): 5 missões (cópia da lista do
//   documento) com botão de missão (abre a explicação) e de prêmio (0x193 com missão + 0x15); as 5 premiadas mostram o final
//   (0x194). Pai 00e78254.
// - RF2014ArinNotifyDlg ("event_arinbirth_popup_1_dialog.xml"; vtables 00d1181c / 00d11810; 0x188; modal): aviso com texto.
//   Instância 00e78260, pai 00e7825c.
// - RF2014ArinMissionInfoDlg ("event_arinbirth_popup_2_dialog.xml"; vtables 00d11fa4 / 00d11f98; 0x198; modal): explicação das
//   missões 1–4 (textos 0x30e15..0x30e1c). Instância 00e78268, pai 00e78264.
// - RF2014ArinMapMissionInfoDlg ("event_arinbirth_popup_3_dialog.xml"; vtables 00d1203c / 00d12030; 0x198; modal): a 5ª missão
//   (mapas: bits do valor da missão). Instância 00e78270, pai 00e7826c.

namespace _gateway
{
class CArinBirthDay2014Doc;
struct sArinMission983;

// texto de uma missão (0x54; construtor 005d9850, cópia 005da170, destrutor 005d9880): título (formato com a contagem) e
// explicação (formato com valor e contagem); o 3º nunca é preenchido
struct sArinMissionText983
{
	std::string title; // +0x00
	std::string text; // +0x1c
	std::string extra; // +0x38
};

class RF2014ArinBirthdayDlg : public RFFormModalBase<RFWndEx>
{
public:
	enum { TEXT_COUNT983 = 13 };

	RF2014ArinBirthdayDlg(); // 005ddb40
	virtual ~RF2014ArinBirthdayDlg(); // 005ddbe0 (??_G 005de090; 2ª base 005ddbd0)

	virtual void OnInitialUpdate(); // 005dee70 (slot 10)

	// 005ddf50: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai em 00e78244
	static RF2014ArinBirthdayDlg* __fastcall Create983(RFWnd* parent);
	void SetInstance983(); // 005d8170 (ninguém chama)
	static void ClearInstance983(); // 005d8180 (ninguém chama)

	void SetDocument983(CArinBirthDay2014Doc* pDoc); // 005d81f0
	__declspec(noinline) void SetStep983(int step) { m_step = step; } // 005d8200
	void OnClose983(EventArgs& e); // 005d8210: Close(2)
	void OnKeyDown983(EventArgs& e); // 005d86e0: Esc fecha
	void OnOk983(EventArgs& e); // 005dd210: passo seguinte (passo 4 manda 0x190)
	void SendStart983(); // 005d9020: 0x190
	void InitTexts983(); // 005de6b0: os 13 textos dos passos
	void UpdateStep983(); // 005dcdd0: monta a janela pelo passo (+0x18c, 0..0xb)
	void ShowStage983(int index); // 005da3f0: texto `index` em text_area
	void ShowOk983(bool show); // 005da550
	void SetBg2_983(); // 005da620: pic_arinbirth_bg_02.png
	void SetBg1_983(); // 005da720: pic_arinbirth_bg_01.png
	void SetTiki983(int index); // 005da820: pic_tiki_%d.png
	void SetArin983(int index); // 005da970: pic_arin_%d.png
	void OpenMissionInfo983(); // 005dc280: aviso 0x30e1e (modal)
	void OpenMapMissionInfo983(); // 005dc2f0: aviso 0x30e1f (modal)
	void SetMissionResult983(int index); // 005dc850 (o handler passa valor − 0x15): prêmio da missão na janela da magia
	void ShowMapResult983(); // 005dc970: janela da magia no passo 0xb

	static RF2014ArinBirthdayDlg* s_pInstance; // 00e78248
	static RFWnd* s_pParent983; // 00e78244

	CArinBirthDay2014Doc* m_pDoc; // +0x188
	int m_step; // +0x18c
	std::vector<std::string> m_190; // +0x190 (os 13 textos)
};
C_ASSERT_VC10(sizeof(RF2014ArinBirthdayDlg) == 0x1a0);

class RF2014ArinStage1Wnd : public RFFormBase<RFWndEx>
{
public:
	enum { TICKET_COUNT983 = 5 };

	RF2014ArinStage1Wnd(); // 005d8700
	virtual ~RF2014ArinStage1Wnd(); // 005d8740 (??_G 005d90d0)

	virtual void OnInitialUpdate(); // 005dd2e0 (slot 10)

	// 005daac0: cria pelo xml (modo g_rfCreateMode983) e guarda o pai em 00e7824c
	static RF2014ArinStage1Wnd* __fastcall Create983(RFWnd* parent);
	void OnTicket1_983(EventArgs& e) { } // 005d8220 (duplo clique; vazia)
	void OnTicket2_983(EventArgs& e) { } // 005d8230
	void OnTicket3_983(EventArgs& e) { } // 005d8240
	void OnTicket4_983(EventArgs& e) { } // 005d8250
	void OnTicket5_983(EventArgs& e) { } // 005d8260
	bool HasTicket983(int index); // 005d8270
	void UseTicket983(int index); // 005d8290
	void SetTicketCount983(int index, int count); // 005d99b0: label_ticket_%02d
	void ShowTicketMark983(int index, bool show); // 005d9ae0: pic_event_ticket%d
	void ShowEventTicket983(bool show); // 005dac00: pic_event_ticket
	void CheckAll983(); // 005dc360: os 5 usados → mostra e manda 0x191
	void OnTicket983(int index); // 005dca90: usa o bilhete `index`
	void UseOwnedTickets983(); // 005dcad0: usa os bilhetes que o jogador tem
	void SendTicketsDone983(); // 005d9100: 0x191

	static const unsigned long s_ticketItems983[TICKET_COUNT983]; // 00d11768
	static RFWnd* s_pParent983; // 00e7824c

	bool m_bUsed983[TICKET_COUNT983]; // +0x184
	int m_count983[TICKET_COUNT983]; // +0x18c
};
C_ASSERT_VC10(sizeof(RF2014ArinStage1Wnd) == 0x1a0);

class RF2014ArinStage2Wnd : public RFFormBase<RFWndEx>
{
public:
	enum { MISSION_COUNT983 = 5 };

	RF2014ArinStage2Wnd(); // 005dacd0
	virtual ~RF2014ArinStage2Wnd(); // 005dad10 (??_G 005dc4d0)

	virtual void OnInitialUpdate(); // 005df230 (slot 10)

	// 005dc390: cria pelo xml (modo g_rfCreateMode983) e guarda o pai em 00e78254
	static RF2014ArinStage2Wnd* __fastcall Create983(RFWnd* parent);
	void SetStep983(int step); // 005d82c0
	int CountRewarded983(); // 005d82d0
	void OnMission1_983(EventArgs& e); // 005df1e0
	void OnMission2_983(EventArgs& e); // 005df1f0
	void OnMission3_983(EventArgs& e); // 005df200
	void OnMission4_983(EventArgs& e); // 005df210
	void OnMission5_983(EventArgs& e); // 005df220
	void OnReward1_983(EventArgs& e); // 005d97f0
	void OnReward2_983(EventArgs& e); // 005d9800
	void OnReward3_983(EventArgs& e); // 005d9810
	void OnReward4_983(EventArgs& e); // 005d9820
	void OnReward5_983(EventArgs& e); // 005d9830
	void OnFinal983(EventArgs& e); // 005d9840
	void SendReward983(int index); // 005d91b0: 0x193 (missão feita e sem prêmio)
	void SendFinal983(); // 005d9290: 0x194
	void ShowMissionBtn983(int index, bool show); // 005d9c00: btn_mission_%02d
	void ShowRewardBtn983(int index, bool show); // 005d9d20: btn_reward_%02d
	void SetMissionBtnState983(int index, int state); // 005d9e40 (sem chamador; 3 = desligado)
	void SetRewardBtnState983(int index, int state); // 005d9f80 (3 = desligado)
	void SetMagicBg983(int count); // 005dadb0: pic_event_magic_bg_%02d.png
	void ShowFinalBtn983(bool show); // 005daf00
	void ShowFinalPic983(bool show); // 005dafd0
	bool IsAllRewarded983(); // 005db110: fora do passo 0xb, os 5 premiados tocam "_Final_"
	void ShowAll983(bool show); // 005dc500
	void Refresh983(); // 005dc540
	void OnRewarded983(int index); // 005dc600
	void CheckFinal983(); // 005dc650: passo 0xb = tudo escondido, fundo 05
	void SetMissions983(const std::vector<sArinMission983>& missions); // 005dcd40
	void OpenMission983(int index); // 005df110: explicação das missões 1–4
	void OpenMapMission983(int index); // 005df180: explicação da 5ª

	static RFWnd* s_pParent983; // 00e78254

	std::vector<sArinMission983> m_missions983; // +0x184
	int m_step983; // +0x194
	bool m_bRewarded983[MISSION_COUNT983]; // +0x198
	bool m_19d_983; // +0x19d (só o construtor zera)
};
C_ASSERT_VC10(sizeof(RF2014ArinStage2Wnd) == 0x1a0);

class RF2014ArinNotifyDlg : public RFFormModalBase<RFWndEx>
{
public:
	RF2014ArinNotifyDlg(); // 005d8750
	virtual ~RF2014ArinNotifyDlg(); // 005d87e0 (??_G 005d9340; 2ª base 005d87d0)

	virtual void OnInitialUpdate(); // 005db470 (slot 10)

	// 005db1d0: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai em 00e7825c
	static RF2014ArinNotifyDlg* __fastcall Create983(RFWnd* parent);
	void SetInstance983(); // 005d8190 (ninguém chama)
	static void ClearInstance983(); // 005d81a0 (ninguém chama)
	void OnClose983(EventArgs& e); // 005d8310: Close(2)
	void SetText983(std::string text); // 005db310: TextArea

	static RF2014ArinNotifyDlg* ms_pInstance; // 00e78260
	static RFWnd* s_pParent983; // 00e7825c
};
C_ASSERT_VC10(sizeof(RF2014ArinNotifyDlg) == 0x188);

class RF2014ArinMissionInfoDlg : public RFFormModalBase<RFWndEx>
{
public:
	RF2014ArinMissionInfoDlg(); // 005de0c0
	virtual ~RF2014ArinMissionInfoDlg(); // 005ddc70 (??_G 005de300; 2ª base 005ddd00)

	virtual void OnInitialUpdate(); // 005db850 (slot 10)

	// 005debf0: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai em 00e78264
	static RF2014ArinMissionInfoDlg* __fastcall Create983(RFWnd* parent);
	void SetInstance983(); // 005d81b0 (ninguém chama)
	static void ClearInstance983(); // 005d81c0 (ninguém chama)
	void OnClose983(EventArgs& e); // 005d8320: Close(2)
	void SetMission983(int index, const sArinMission983& mission); // 005db570

	static RF2014ArinMissionInfoDlg* ms_pInstance; // 00e78268
	static RFWnd* s_pParent983; // 00e78264

	std::vector<sArinMissionText983> m_texts983; // +0x188
};
C_ASSERT_VC10(sizeof(RF2014ArinMissionInfoDlg) == 0x198);

class RF2014ArinMapMissionInfoDlg : public RFFormModalBase<RFWndEx>
{
public:
	RF2014ArinMapMissionInfoDlg(); // 005de330
	virtual ~RF2014ArinMapMissionInfoDlg(); // 005ddd10 (??_G 005de440; 2ª base 005ddda0)

	virtual void OnInitialUpdate(); // 005dbde0 (slot 10)

	// 005ded30: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai em 00e7826c
	static RF2014ArinMapMissionInfoDlg* __fastcall Create983(RFWnd* parent);
	void SetInstance983(); // 005d81d0 (ninguém chama)
	static void ClearInstance983(); // 005d81e0 (ninguém chama)
	void OnClose983(EventArgs& e); // 005d8330: Close(2)
	void SetMission983(const sArinMission983& mission); // 005db950

	static RF2014ArinMapMissionInfoDlg* ms_pInstance; // 00e78270
	static RFWnd* s_pParent983; // 00e7826c

	std::vector<sArinMissionText983> m_texts983; // +0x188
};
C_ASSERT_VC10(sizeof(RF2014ArinMapMissionInfoDlg) == 0x198);
}
