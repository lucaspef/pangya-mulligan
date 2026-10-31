#pragma once
// JP 983: _gateway::_grandprix::FrGrandPrixOpenDlg (unidade grandprixopendlg, 0055aa50–0055cd30; vtables 00d09fe4 / 00d09fd8 /
// 00d09fd0 pelo construtor 0055c4b0, sizeof 0x1e0 pelo `new` de 00559da0). Janela "grandprixopen" (entrar num Grand Prix), aberta
// pelo CGrandPrixHandler (id 7): nível, mapa, horário de começo/fim, prêmios (os do registro e os do GrandPrixRankReward.iff, com
// dica do nome sob o mouse), regra especial e o aviso pelo estado da linha. Posições em grandprix_form.lua ("gpFormOffset").
// Sessão B65: declaração; B66: corpo. Nomes dos campos e métodos são meus.

#include "frform.h"
#include "grandprix983.h"
#include <vector>
#include <string>
#include <memory>

class FrArea;
class FrEdit;
class FrStatic;
class Bitmap;
class WTitleFontEx;
struct IFF_ITEM_COMMON;

namespace _gateway
{
namespace _grandprix
{
struct sGPPrize983
{
	IFF_ITEM_COMMON* pItem;
	short count;
};

struct sGPFormPoint983
{
	float x, y;
};

class FrGrandPrixOpenDlg : public FrForm
{
	DECLARE_OBJECT(FrGrandPrixOpenDlg)

public:
	FrGrandPrixOpenDlg(); // 0055c4b0
	virtual ~FrGrandPrixOpenDlg(); // 0055c5e0 (0055c820 com delete)

	void Init983(const sGPEnter983* pEnter); // 0055aa80

protected:
	virtual bool OnInit(); // 0055ca90
	virtual void OnProc(float dt); // 0055b8a0: dica do prêmio sob o mouse
	virtual void OnMouseMove(const WPoint& pt); // 0055c5a0

	void OnStartTimeInit(int param); // 0055ae60
	void OnFinishTimeInit(int param); // 0055aed0
	void OnDiffInit(int param); // 0055adf0
	void OnDiffDraw(); // 0055b1f0: nível, número, mapa e os dois horários
	void OnPrizeDraw(); // 0055bc90
	void OnWarningMsgInit(int param); // 0055ac90
	void OnTitleInit(int param); // 0055ad10
	void OnSpecialRuleInit(int param); // 0055ad80
	void OnOk(); // 0055b0e0 (pela entrada 0055b1e0)
	void OnCancelBtnUp983(); // 0055aab0: so pelo msgmap; o 983 nao sobrepoe FrForm::OnCancel (slot 34, ESC)

	void SetTexts983(); // 0055b9d0: regra especial, aviso pelo estado, horários só no canal de evento
	void CheckTip983(int id, float x, float y, float w, float h, const std::string& text); // 0055bbe0
	void BuildPrizes983(); // 0055c850

	const Bitmap* m_pClassBmp[4]; // +0x110: "gp_beginner", "gp_class1..3" (bits 19–25 do ticket)
	const Bitmap* m_pMarkBmp[4]; // +0x120: "gp_mark_*"
	WTitleFontEx* m_pNumFont; // +0x130 (sGPEnter983::m_08)
	WTitleFontEx* m_pTimeFont; // +0x134 (sGPEnter983::m_0c)
	FrArea* m_pStartTime; // +0x138
	FrArea* m_pFinishTime; // +0x13c
	FrArea* m_pDiff; // +0x140
	FrEdit* m_pWarningMsg; // +0x144
	FrStatic* m_pTitle; // +0x148
	FrStatic* m_pSpecialRule; // +0x14c
	sGPFormPoint983 m_offset[6]; // +0x150: começo, fim, prêmios, prêmios do ranking, mapa, passo do número (x)
	sGPInfo983* m_pInfo; // +0x180
	int m_state; // +0x184: estado da linha (sGPRoomItem983::state)
	std::tr1::shared_ptr<WTitleFontEx> m_pTicketFont; // +0x188
	std::vector<sGPPrize983> m_prizes; // +0x190
	std::vector<sGPPrize983> m_rankPrizes; // +0x1a0
	WPoint m_mouse; // +0x1b0
	int m_tipId; // +0x1b8: prêmio com a dica aberta (-1 = nenhum)
	std::string m_tip; // +0x1bc
	char m_1d8[0x1e0 - 0x1d8]; // não iniciado; uso não visto

	DECLARE_FRESH_MSGMAP()
};
}
}
