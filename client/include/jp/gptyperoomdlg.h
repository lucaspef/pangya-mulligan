#pragma once
#include "vc10_layout.h"
// JP 983: unidade gptyperoomdlg (00510a10–00517140). Duas classes novas derivadas da FrGameRoomDlg:
// - FrMassTypeRoomDlg (vtables 00d06d3c / 00d06d2c / 00d06d24; sizeof 0x4b0 pelo `new` de 00515000; construtor 00512ee0;
//   mapa 00e09464, 32 entradas): a sala de 30 jogadores — é o código `OnGameRoomExt_*`/`OnGameRoomExtRes_*` do CLobbyMain do
//   645 (src/projectg/lobbymain.cpp) mudado para uma classe; os nomes dos handlers e dos campos vêm de lá.
// - FrGPTypeRoomDlg : FrMassTypeRoomDlg (vtables 00d06dec / 00d06de0 / 00d06dd8; sizeof 0x4b0 pelo `new` de 005153e0 — nenhum
//   campo a mais; construtor 00514340; mapa 00e0946c, 10 entradas): a sala do Grand Prix (quadro de horário sGPTimeBoard983 em
//   FrGameRoomDlg+0x150, interface do grandprix_room_ui.lua, espera de IA do grandprix_ai_player.lua).
// Sessão B69 (1º lote: layout, mapas, construtores, destrutores e os handlers simples). Nomes dos métodos novos são meus.

#include "gameroomdlg.h"
#include "normallobbyroom.h"

class FrMassTypeRoomDlg : public FrGameRoomDlg
{
	DECLARE_OBJECT(FrMassTypeRoomDlg)

public:
	FrMassTypeRoomDlg(); // 00512ee0
	virtual ~FrMassTypeRoomDlg(); // 00510fd0

protected:
	virtual void OnProc(const float deltaTime); // 00511c90: no estado 8 o título mostra o tempo que falta
	virtual bool OnInit(); // 005162c0

	void ResetRoomMass983(); // 00510a20 (só chama o ResetRoom983 da base)
	void HideControls983(); // 00510a40: OpenNewRecordForm e esconde mapa, botões de dono e quadros de equipamento
	void ResetIdle983(); // 00511d20: o começo do ResetRoom983 (estado 8 = partida em andamento)
	void SetGPRoom983(); // 00511d90
	void InitMySlot983(); // 005160a0

	void OnGuildInviteInit(int param); // 00511290 (645: OnGameRoomExt_GuildInviteInit)
	void OnGuildInviteLBtnUp(); // 00516390 (645: OnGameRoomExt_GuildInviteLBtnUp)
	void OnEventOwnerDraw(int param); // 00511f10 (645: OnGameRoom_EventOwnerDraw)
	void OnRoomUserInit(int param); // 00515090 (645: OnGameRoomExt_RoomUserInit)
	void OnRoomUserOwnerDraw(int param); // 00513270 (645: OnGameRoomExt_RoomUserOwnerDraw)
	void OnTeamTab1Init(int param); // 00511060 (645: OnGameRoomExt_TeamTab1Init)
	void OnTeamTab2Init(int param); // 005110d0
	void OnGuildTab1Init(int param); // 00511140
	void OnGuildTab2Init(int param); // 005111b0
	void OnTeamNumInit(int param); // 00511220
	void OnTeamNumOwnerDraw(int param); // 00511fc0
	void OnGuildNumInit(int param); // 005121d0
	void OnGuildNumOwnerDraw(int param); // 00512250
	void OnCountdownInit(int param); // 00511300
	void OnUserInfoInit(int param); // 00511380
	void OnUserInfoLBtnUp(); // 00515810
	void OnFrameInit(int param); // 00511400
	void OnGuildScoreGaugeInit(int param); // 005125a0
	void OnRoomUserRankTabInit(int param); // 00512610
	void OnRoomUserRankInit(int param); // 00513910
	void OnRoomUserRankOwnerDraw(int param); // 00513ac0
	void OnRoomUserRankLBtnUp(); // 00511470
	void OnRoomUserRankRBtnUp(); // 005158c0
	void OnRoomGuildRankInit(int param); // 005140c0
	void OnRoomGuildRankOwnerDraw(int param); // 005169b0
	void OnRoomGuildRankLBtnUp(); // 005114f0
	void OnRoomGuildRankRBtnUp(); // 005153d0
	void OnScoreInit(int param); // 00511540 (645: OnGameRoom_ScoreInit)
	void OnScoreBtnUp(); // 00514200 (645: OnGameRoom_ScoreBtnUp)

	// interface do Grand Prix (lidos pela FrGPTypeRoomDlg, 00512f80; o construtor não grava)
	const Bitmap* m_pGPUiBmp983[5]; // +0x430: "grandprixroomuiname"
	sGPPoint983 m_gpUiOffset983[11]; // +0x444: "grandprixroomuioffset"
	void* m_p49c; // +0x49c
	unsigned long m_rankViewKind; // +0x4a0 (nome do 645): 0 = lista de jogadores (OnUserInfoLBtnUp)
	char m_pad4a4[0x4b0 - 0x4a4];

	DECLARE_FRESH_MSGMAP()
};
C_ASSERT_VC10(sizeof(FrMassTypeRoomDlg) == 0x4b0);

class FrGPTypeRoomDlg : public FrMassTypeRoomDlg
{
	DECLARE_OBJECT(FrGPTypeRoomDlg)

public:
	FrGPTypeRoomDlg(); // 00514340
	virtual ~FrGPTypeRoomDlg(); // 005115c0

protected:
	virtual bool OnInit(); // 00516fc0

	void Init983(); // 00512f80: quadro de horário e grandprix_room_ui.lua
	void Leave983(); // 00511020: para o som do lobby e apaga o quadro de horário

	void OnGPUserOwnerDraw(int param); // 005143b0
	void OnGPDetailInit(int param); // 00515ac0
	void OnGPDetailHoverOn(int param); // 005116b0
	void OnGPDetailHoverOff(int param); // 005116c0
	void OnGPInfoInit(int param); // 005116d0
	void OnGPInfoOwnerDraw(); // 005126d0
	void OnWaitingInit(); // 00514a80: grandprix_ai_player.lua
	void OnWaitingOwnerDraw(); // 00515470

	DECLARE_FRESH_MSGMAP()
};
C_ASSERT_VC10(sizeof(FrGPTypeRoomDlg) == 0x4b0);
