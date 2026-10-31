#pragma once
#include "vc10_layout.h"
// JP 983: _gateway::_questoneday::CQuestHandler (RTTI .?AVCQuestHandler@_questoneday@_gateway@@, vtables 00d12e58 / 00d12e50 /
// 00d12e44, construtor 005efaf0, 0x30 bytes pelo `new` de 005efeb0). Ator do sistema de missões diárias e conquistas: recebe os
// pacotes (param1 = função, tabela montada em 005eff70) e abre as janelas da missão diária (jp/onedayquest.cpp) e das conquistas.
// Base a mais que o CControllerHandler: FrCmdTarget em +0x20 (destino dos retornos das janelas; mapa de mensagens o da base).
// Mesma unidade do CQuestDocument (ver questdocument.h). Sessão B99; nomes dos métodos e campos meus, "id" = chave do FunctionMapper.
#include "controllerhandler.h"
#include "frcmdtarget.h"
#include "frform.h"
#include "questdocument.h"
#include "achievementdlg.h"

namespace _gateway
{
namespace _questoneday
{
class FrOnedayQuest;
class FrOnedayQuestComplete;

class CQuestHandler : public CGatewayHandlerJp, public FrCmdTarget
{
public:
	static const WRTTI m_RTTI; // 00e78c7c
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 005efb60

	CQuestHandler(); // 005efaf0 (fora de linha: o MakeInstance chama)
	virtual ~CQuestHandler() { } // 005efb90 (005eff40 com delete)
	virtual bool Initialize(); // 005eff70
	virtual bool Execute(const MsgObject& msg); // 005eed70

	CQuestDocument* Doc983() const { return (CQuestDocument*)m_pDocument; }
	void UpdateCurrent983(); // 005ef100: m_current do documento = as missões do jogador abertas, fora as diárias, com QuestItem.iff +0xc8 == 1

	void OnQuestList983(const MsgObject& msg); // id 0x02, 005ef270: datas, as de amanhã e as que saíram; abre "oneday_quest_main"
	void OnQuestUpdate983(const MsgObject& msg); // id 0x03, 005ef8a0: missões com contadores novos
	void OnQuestComplete983(const MsgObject& msg); // id 0x04, 005eed90: missões completas; aviso "oneday_quest_complete"
	void OnQuestReward983(const MsgObject& msg); // id 0x05, 005ef020: prêmio pago (resposta ao 0x153)
	void OnQuestCancel983(const MsgObject& msg); // id 0x06, 005ef640: missões desistidas (resposta ao 0x154)
	void OnAchievementList983(const MsgObject& msg); // id 0x08, 005ee6e0: conquistas para m_newAchievements do documento
	void OnAchievementOpen983(); // id 0x09, 005ee910: abre "ach_main" (ou só atualiza a loja "bongdari_shop" se aberta)
	void OnAchievementDone983(const MsgObject& msg); // id 0x0a, 005efc40: conquistas concluídas; aviso "oneday_quest_complete"

	bool OnQuestDlgResult983(int ret, FrForm* pForm); // 005ee790
	bool OnCompleteDlgResult983(int ret, FrForm* pForm); // 005ee160
	bool OnAchievementDlgResult983(int ret, FrForm* pForm); // 005ee800

protected:
	FrOnedayQuest* m_pQuestDlg; // +0x24
	FrOnedayQuestComplete* m_pCompleteDlg; // +0x28
	FrAchievement* m_pAchievementDlg; // +0x2c
};
}
}
C_ASSERT_VC10(sizeof(_gateway::_questoneday::CQuestHandler) == 0x30);
