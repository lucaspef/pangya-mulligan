#pragma once
// JP 983: FrSingleModeStarter (unidade 00521710–00521f70; vtables 00d0815c / 00d08150 / 00d08148; sizeof 0x170 pelo `new` de
// 00521e40). Janela de entrada do modo solo: quatro botões (percurso, repetição, chip-in, tutorial) e o botão "expl", que troca
// as duas imagens pelos textos de explicação e esconde os botões. Layout pelo construtor 00521790 e pelos métodos (sessão B40).
// Nomes dos campos e dos métodos são meus.

#include "frform.h"

class FrButton;
class FrArea;
class Bitmap;

class FrSingleModeStarter : public FrForm
{
	DECLARE_OBJECT(FrSingleModeStarter)

public:
	FrSingleModeStarter(); // 00521790

	unsigned char GetMode983() const { return m_mode; } // lido por quem abre a janela (+0x160)

protected:
	void ToggleExpl983(); // 00521920
	void OnExplInit(int param); // 00521b00
	void OnExpl(); // 00521b90
	void OnClose(); // 00521720
	void OnImgCourseInit(int param); // 00521ba0
	void OnImgChipinInit(int param); // 00521c10
	void OnBtnCourseInit(int param); // 00521c80
	void OnBtnRepeatInit(int param); // 00521cf0
	void OnBtnChipinInit(int param); // 00521d60
	void OnBtnTutorialInit(int param); // 00521dd0
	void OnBtnCourse(); // 00521730
	void OnBtnRepeat(); // 00521750
	void OnBtnChipin(); // 00521770
	void OnBtnTutorial(); // 00521ed0

	FrButton* m_pExplBtn; // +0x110: "expl"
	FrArea* m_pImg[2]; // +0x114: "img_course", "img_chipin"
	FrButton* m_pBtn[4]; // +0x11c: "btn_course", "btn_repeat", "btn_chipin", "btn_tutorial"
	int m_expl; // +0x12c: 0 imagens, 1 explicação
	const Bitmap* m_pBmp[2][2]; // +0x130: [m_expl][imagem]
	_WPOINT m_offset[2][2]; // +0x140: deslocamento somado à imagem em cada troca
	unsigned char m_mode; // +0x160: 0 percurso, 4 repetição, 6 chip-in, 0xff nada escolhido
	char m_pad161[0x170 - 0x161]; // sem leitor nem gravador nesta unidade

	DECLARE_FRESH_MSGMAP()
};
