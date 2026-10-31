#pragma once
#include "vc10_layout.h"
// JP 983: FrAttendanceDlg ("attendance_dlg"), a presença diária: carimbo de entrada (check-in, ao entrar) ou de saída (check-out,
// ao sair do jogo / voltar ao login) com o prêmio de hoje ou de amanhã e a contagem 3-2-1. Unidade 0047c930–0047dae0 (vtables
// 00cfe794 / 00cfe784 / 00cfe77c; RTTI 00e73e64; mapa de mensagens 00e034b0 com 2 entradas, ambas do "image_area"; registro
// 00cc7c60). sizeof 0x1f0 pelo `operator new` de 0047d250. Quem cria é o CTaskMain (frente A): CreateForm 006550d0 no
// OnQuitDlgResult 00657200 (saída: +0x12c, +0x130 = 1, pacote 0x16e), e a mensagem 0x327 (HandleMsgCommon 0065f4fb: +0x12c = 0,
// +0x1a6 = 1, EnableDrag(false), SetReward983); o resultado 00651270 chama DoLogout983. Posições em "attendance.lua"
// (tabela "attendance_ui_config"). Existe no 839, não no 645. Nomes dos métodos e dos campos são meus (sessão B96).
#include "frform.h"
#include "packet.h"
#include "packets/attendancepackets.h"

class FrArea;
class Bitmap;
class WTitleFontEx;

class FrAttendanceDlg : public FrForm
{
	DECLARE_OBJECT(FrAttendanceDlg) // GetRTTI 0047c9f0; MakeInstance 0047d250; registro 0047d2c0

public:
	FrAttendanceDlg(); // 0047ce10
	virtual ~FrAttendanceDlg(); // 0047c950 (com `delete`: 0047cee0; ajustes 0047cb10 / 0047cb20)

	virtual bool OnInit(); // 0047d590

	void SetCountdown983(int countdown); // 0047c930 (sem chamador no 983)
	// 0047da80 (chamada pelo CTaskMain, mensagem 0x327): copia o prêmio e prepara figura, quantidade e dica
	void SetReward983(const sAttendanceReward983* pReward);
	// 0047d770: o logout do FrLogoutDlg (00adc440) repetido aqui; chamado no fim da contagem e pelo CTaskMain 00651270
	void DoLogout983();

protected:
	virtual void OnProc(const float deltaTime); // 0047d960: 1 s acende a entrada; 3-2-1; aos 5 s fecha ou sai

	void LoadBitmaps983(); // 0047ca00: figuras pelo modo
	void SetCount983(); // 0047cad0: quantidade mostrada = a de hoje (entrada) ou a de amanhã (saída)
	void LoadIcon983(); // 0047d2e0: ícone do item do prêmio (Icon do .iff)
	void LoadConfig983(); // 0047d330: "attendance.lua"
	void SetToolTip983(); // 0047d640: nome do item como dica do "image_area"

	void OnImageAreaInit983(int param); // 0047cf10
	void OnImageAreaDraw983(int param); // 0047cfa0

	FrArea* m_pImageArea; // +0x110: "image_area"
	sAttendanceReward983 m_reward; // +0x114
public:
	int m_exitAction983; // +0x12c: depois da saída: 0 fecha o jogo, 1 volta ao login (-1 no construtor); gravado pelo CTaskMain
	int m_checkMode983; // +0x130: 0 entrada (check-in), 1 saída (check-out) (-1 no construtor); gravado pelo CTaskMain
protected:
	const Bitmap* m_pBmp[5]; // +0x134: [0] item de hoje/amanhã, [1..3] contagem 1..3, [4] carimbo normal
	const Bitmap* m_pHoverBmp; // +0x148: carimbo aceso (só na entrada)
	const Bitmap* m_pIconBmp; // +0x14c: ícone do item
	WRect m_itemRect; // +0x150: "todayitem_or_tomorrowitem_*"
	WRect m_pad983_160[3]; // +0x160: sem leitura/escrita no 983 (0047c000..0047e000)
	WRect m_checkRect; // +0x190: "checkin_or_checkout_*" (carimbo e contagem)
	int m_countdown; // +0x1a0: 1..3 (-1 nada)
	bool m_bHover; // +0x1a4: carimbo aceso (entrada, depois de 1 s)
	bool m_bHasReward; // +0x1a5: SetReward983 já veio
public:
	bool m_bExitRoom983; // +0x1a6: ao fechar o jogo, sai da sala antes (online); gravado pelo CTaskMain
protected:
	WTitleFontEx* m_pRoundFont; // +0x1a8: CAttendanceDate, "%04d" da rodada
	WTitleFontEx* m_pCountFont; // +0x1ac: CWindFont, "x%d" da quantidade
	int m_count; // +0x1b0: quantidade mostrada (-1 nada)
	float m_fontX; // +0x1b4: "font_x"
	float m_fontY; // +0x1b8: "font_y"
	float m_itemX; // +0x1bc: "item_x" (ícone)
	float m_itemY; // +0x1c0: "item_y"
	WRect m_toolTipRect; // +0x1c4: "itemtooltip_*" (fora dele a dica apaga)
	float m_time; // +0x1d4: tempo desde a abertura
	float m_baseX; // +0x1d8: canto do "image_area"
	float m_baseY; // +0x1dc
	float m_pad983_1e0; // +0x1e0: zerado no OnImageAreaInit983
	float m_pad983_1e4; // +0x1e4
	char m_pad983_1e8[8]; // +0x1e8: sem leitura/escrita no 983

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0047c940; mapa 00e034b0
};
C_ASSERT_VC10(sizeof(FrAttendanceDlg) == 0x1f0);
