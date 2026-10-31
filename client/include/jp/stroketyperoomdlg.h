#pragma once
// JP 983: FrStrokeTypeRoomDlg (unidade 00523250–005235f0; vtables 00d0863c / 00d08630 / 00d08628; sizeof 0x430 pelo `new` de
// 00523370 — nenhum campo além dos da FrGameRoomDlg). Sala de jogo por tacadas. Sessão B40. Nomes dos métodos são meus.

#include "gameroomdlg.h"

class FrStrokeTypeRoomDlg : public FrGameRoomDlg
{
	DECLARE_OBJECT(FrStrokeTypeRoomDlg)

public:
	FrStrokeTypeRoomDlg(); // 00523260

protected:
	virtual void OnProc(const float deltaTime); // 00523330
	virtual bool OnInit(); // 00523300

	void OnPet0Init(int param); // 00523430
	void OnPet1Init(int param); // 005234a0
	void OnPet2Init(int param); // 00523510
	void OnPet3Init(int param); // 00523580

	DECLARE_FRESH_MSGMAP()
};
