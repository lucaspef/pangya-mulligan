#pragma once
// JP 983: FrSingleTypeDlg (unidade 00521f70–00523250; vtables 00d08304 / 00d082f8 / 00d082f0; sizeof 0x440 pelo `new` de
// 005228a0). Sala do modo solo: percurso/repetição ("course_ctrl": buraco fixo, posição do copo) e chip-in ("chipin_ctrl":
// inclinação, tamanho do copo, bilhetes). Layout pelo 00521fd0 e pelos métodos (sessão B41). Nomes dos campos e dos métodos
// são meus.

#include "gameroomdlg.h"
#include "titlefontex.h"

class FrHoleSelector;

// 006f4ac0 e 006f4b80: CSingleNumCourseFont e CRepeatHoleFont estao no titlefontex.h (frente A, pedido B58).

class FrSingleTypeDlg : public FrGameRoomDlg
{
	DECLARE_OBJECT(FrSingleTypeDlg)

public:
	FrSingleTypeDlg(); // 00522120
	virtual ~FrSingleTypeDlg(); // 005221c0

protected:
	virtual void OnProc(const float deltaTime); // 00521f80
	virtual bool OnInit(); // 00523160

	void Reset983(); // 00521fd0
	void Release983(); // 005220a0
	void FadeIn983(); // 00522230 (sem chamador no 983)
	void Setup983(); // 00522f00
	bool OnHoleSelected(int ret, FrForm* pForm); // 00522960: resultado da janela "hole_selector"

	void OnCloseSingle(); // 005220d0
	void OnModeTitleInit(int param); // 005222d0
	void OnPet0Init(int param); // 00522340 (sem entrada no mapa de mensagens do 983)
	void OnCourseCtrlInit(int param); // 005223b0
	void OnChipinCtrlInit(int param); // 00522420
	void OnBgCourseInit(int param); // 00522490
	void OnBgCourseDraw(); // 00522a40
	void OnFixHoleInit(int param); // 00522500
	void OnFixHole(); // 00522bf0
	void OnFixHoleDraw(); // 00522cd0
	void OnFixHolecupPosInit(int param); // 00522580
	void OnFixHolecupPos(); // 00522d90
	void OnSlopeInit(int param); // 00522e60
	void OnSlope(); // 00522600
	void OnSizeInit(int param); // 00522640
	void OnSize(); // 005226b0
	void OnTicketInit(int param); // 005226f0
	void OnTicketDraw(); // 00522760

	CSingleNumCourseFont* m_pNumFont; // +0x430: números de "bg_course" e dos bilhetes
	char m_ticket[8]; // +0x434: "x<n>" (quantidade do item 0x1a00017e)

	DECLARE_FRESH_MSGMAP()
};
