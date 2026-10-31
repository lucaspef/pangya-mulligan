#pragma once
#include "vc10_layout.h"
// JP 983: missão diária ("_questoneday"), unidade 005176f0–0051c9e0 (a F3.7.1 tinha cortado em onedaymentionselect e onedayquest;
// os três mapas de mensagens são de um inicializador só, 00cccc40/00ccceb0/00cccfb0, sessão B97). Três janelas:
// - FrOnedayQuest ("onedayquest", tabela "quest_ui_config" de onedayquest.lua): listas de amanhã/hoje, 3 botões de prêmio, pedir as
//   missões de hoje, desistir, prêmio final ("last_gift") e 4 medidores (CGaugeBar983). Criada pelo CQuestHandler (005ee440).
// - FrOnedayQuestComplete ("subject"/"gift"/"bg1"/"bg2"): aviso de missão (ou conquista, typeid 0x4c…) concluída, com os prêmios.
// - FrOnedayMentionSelect ("arin_yes"/"arin_no"): pergunta sim/não da Arin ("oneday_quest_mention").
// Existem no 839, não no 645. Nomes dos métodos e dos campos são meus (sessão B98).
#include "frform.h"
#include "gaugebar983.h"
#include <string>
#include <vector>

class FrArea;
class FrButton;
class FrListBox;
class FrStatic;
class Bitmap;
class WOverlay;

namespace _quest
{
class CQuestItem;
}

namespace _gateway
{
namespace _questoneday
{
class CQuestDocument;

// prêmio de missão concluída (8 bytes; lista que o CQuestHandler 005efc40 passa à FrOnedayQuestComplete)
struct sQuestDone983
{
	unsigned long typeId; // QuestItem.iff (0x78… = missão diária, que não entra no aviso) ou conquista
	unsigned long stuffId; // QuestStuff.iff: nome em +8, prêmios {typeid +0xe8, quantidade +0xf4} × 3
};

// botão de prêmio pela missão (vetor +0x2d0 da FrOnedayQuest)
struct sQuestGiftBtn983
{
	unsigned long typeId;
	FrButton* pBtn;
};

class FrOnedayQuest : public FrForm
{
	DECLARE_OBJECT(FrOnedayQuest) // GetRTTI 0051c1e0; MakeInstance 0051c480 (`new` 0x2e0); registro 00cccc20

public:
	FrOnedayQuest(); // 0051c0c0
	virtual ~FrOnedayQuest(); // 0051c210 (com `delete`: 0051c510)

	virtual bool OnInit(); // 0051c610

	void RefreshLists983(); // 0051a1c0: listas de amanhã/hoje pelo documento; botões e medidores sem missão
	void RefreshGauges983(); // 0051a320: botões das missões resolvidas, medidores das de hoje e o da diária
	void OnDailyRewarded983(_quest::CQuestItem* pItem); // 0051a4e0: botão de prêmio da missão desliga (já pago)
	void SendQuestList983(int packetId, bool bFuture); // 0051a550: pacote 0x152 (pedir) / 0x154 (desistir) com os uids
	void RefreshGiftButtons983(); // 0051ad00: liga o botão de prêmio das diárias completas
	void OnQuestRewarded983(_quest::CQuestItem* pItem); // 00519810: medidor cheio, botão desligado e janela do prêmio

protected:
	virtual void OnProc(const float deltaTime); // 00517770: os 4 medidores

	void AddEmptyItems983(int list); // 00517950: 3 linhas vazias na lista (0 amanhã, 1 hoje)
	void SendReward983(int index); // 005199e0: pacote 0x153 com a missão da linha `index` de hoje
	void SendDailyReward983(unsigned long typeId); // 0051a690: pacote 0x153 com a diária desse typeid
	void ShowQuestMention983(); // 0051a790: "pegar o prêmio?" (0x2c3) → OnQuestBtnResult983
	bool HasActiveDaily983(unsigned long typeId); // 0051a810
	void ShowGiftMention983(); // 0051ae10: "pegar o prêmio?" (0x2c3) → OnGiftResult983
	void OnGiftBtn983(unsigned long typeId); // 0051ae90

	bool OnScratchResult983(int ret, FrForm* pForm); // 005179e0: nada
	bool OnQuestBtnResult983(int ret, FrForm* pForm); // 00519ad0
	bool OnGiveUpResult983(int ret, FrForm* pForm); // 0051a840
	bool OnGiftResult983(int ret, FrForm* pForm); // 0051a860
	bool OnReqTodayResult983(int ret, FrForm* pForm); // 0051b3f0

	void OnFutureWorkInit983(int param); // 00518630
	void OnFutureWorkDraw983(int param); // 005186a0
	void OnCurrentWorkInit983(int param); // 00518750
	void OnCurrentWorkDraw983(int param); // 005187c0
	void OnReqTodayInit983(int param); // 00518890
	void OnReqTodayUp983(); // 0051b430
	void OnGiveUpInit983(int param); // 00518900
	void OnGiveUpUp983(); // 0051a9d0
	void OnQuestBtn1Init983(int param); // 00518970
	void OnQuestBtn2Init983(int param); // 005189e0
	void OnQuestBtn3Init983(int param); // 00518a50
	void OnQuestBtn1Up983(); // 0051aa50
	void OnQuestBtn2Up983(); // 0051aa60
	void OnQuestBtn3Up983(); // 0051aa70
	void OnLastGiftInit983(int param); // 00518ac0
	void OnLastGiftUp983(); // 0051b050
	void OnCloseUp983(); // 005179f0
	void OnDrawGauge983(int param); // 00519bc0: medidores e as duas datas

	FrListBox* m_pList[2]; // +0x110: "futurework", "currentwork"
	FrButton* m_pQuestBtn[3]; // +0x118: "quest_btn1..3" (prêmio da linha de hoje)
	FrButton* m_pReqToday; // +0x124: "req_today_quest"
	FrButton* m_pGiveUp; // +0x128: "giveup_quest"
	FrButton* m_pLastGift; // +0x12c: "last_gift" (prêmio da diária 0x78800001)
	int m_pad983_130; // +0x130: 983 só zera (ctor 0051c1a1; 005179d0 sem chamador)
	CGaugeBar983 m_gauge[4]; // +0x134: 3 das linhas de hoje ("procgauge") e o da diária ("totalgauge")
	WOverlay* m_pRewardOverlay; // +0x264: "quest_daily_reward.tga"
	int m_pad983_268; // +0x268: 983 só zera (ctor 0051c1a7)
	float m_todayX; // +0x26c: "datetodayx"
	float m_todayY; // +0x270: "datetodayy"
	float m_procX; // +0x274: "dateprocx"
	float m_procY; // +0x278: "dateprocy"
	std::string m_todayDate; // +0x27c: "%d-%d-%d"
	std::string m_procDate; // +0x298
	int m_selBtn; // +0x2b4: botão de prêmio apertado (0..2; -1 nenhum)
	unsigned long m_giftTypeId; // +0x2b8: diária do "last_gift" esperando a resposta
public:
	CQuestDocument* m_pDoc; // +0x2bc: gravado pelo CQuestHandler antes do OnInit
protected:
	std::vector<_quest::CQuestItem*> m_dailies; // +0x2c0: missões diárias do jogador (filtro 0051bec0)
	std::vector<sQuestGiftBtn983> m_giftBtns; // +0x2d0

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00517740; mapa 00e09b3c (18 entradas)
};

class FrOnedayQuestComplete : public FrForm
{
	DECLARE_OBJECT(FrOnedayQuestComplete) // GetRTTI 0051b0f0; MakeInstance 0051b520 (`new` 0x170); registro 00ccce90

public:
	FrOnedayQuestComplete(); // 0051b070
	virtual ~FrOnedayQuestComplete(); // 0051b120 (com `delete`: 0051b5b0)

	virtual bool OnInit(); // 0051b5e0

	bool HasTypes983() const; // 00518b30 (fora de linha: o CQuestHandler chama)
	void MoveUp983(); // 00518b40: 75 acima
	void MoveDown983(); // 00518b90: 75 abaixo
	void SetQuests983(const std::vector<_quest::CQuestItem*>& quests); // 0051b760: texto "subject" com as missões
	void SetDone983(const std::vector<sQuestDone983>& done); // 0051b950: "subject" com as concluídas e "gift" com os prêmios

protected:
	void OnSubjectInit983(int param); // 00518be0
	void OnGiftInit983(int param); // 00518c50
	void OnBg1Init983(int param); // 00518cc0
	void OnBg1Draw983(); // 00518d30: ícones das conquistas, centrados
	void OnBg2Init983(int param); // 00518e30

	int m_count; // +0x110: missões no texto
	FrStatic* m_pSubject; // +0x114
	FrStatic* m_pGift; // +0x118
	FrArea* m_pBg1; // +0x11c
	FrArea* m_pBg2; // +0x120
	int m_mode; // +0x124: 1 = conquista (fundo "challenge_complete_bg1/2" e ícones)
	std::string m_text; // +0x128
	std::vector<unsigned long> m_typeIds; // +0x144
	std::vector<const Bitmap*> m_icons; // +0x154
	char m_pad983_164[0xc]; // +0x164: sem acesso no 983 (00516000..0051d800)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00517a00; mapa 00e09b44 (5 entradas)
};

class FrOnedayMentionSelect : public FrForm
{
	DECLARE_OBJECT(FrOnedayMentionSelect) // GetRTTI 00517a80; MakeInstance 00518ea0 (`new` 0x120); registro 00cccf90

public:
	FrOnedayMentionSelect(); // 00517a50
	virtual ~FrOnedayMentionSelect(); // 00517a90 (com `delete`: 00518f30)

protected:
	virtual bool OnInit() { return FrForm::OnInit(); } // 00517ab0

	void OnYesInit983(int param); // 00518f60
	void OnNoInit983(int param); // 00518fd0
	void OnYesUp983() { Close(FrOK, true); } // 00517ac0
	void OnNoUp983() { Close(FrNO, true); } // 00517ad0

	FrButton* m_pYes; // +0x110: "arin_yes"
	FrButton* m_pNo; // +0x114: "arin_no"
	char m_pad983_118[8]; // +0x118: sem acesso no 983 (ctor 00517a50, Init 00518f60/00518fd0)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00517a40; mapa 00e09b4c (4 entradas)
};
}
}
C_ASSERT(sizeof(_gateway::_questoneday::sQuestDone983) == 8);
C_ASSERT_VC10(sizeof(_gateway::_questoneday::FrOnedayQuest) == 0x2e0);
C_ASSERT_VC10(sizeof(_gateway::_questoneday::FrOnedayQuestComplete) == 0x170);
C_ASSERT_VC10(sizeof(_gateway::_questoneday::FrOnedayMentionSelect) == 0x120);
