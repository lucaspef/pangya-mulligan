#pragma once
// JP 983: barra de medidor desenhada com duas figuras (fundo + barra) e valor que anda até um alvo. 0x4c, sem vtable.
// Código em 00923080–00923800 (vão entre specialboxdlg e miles do 645; sem par no 645). Usada pela FrAlchemyMPBreakDlg
// (3 barras de MP), pela conquista (004ea0xx), missão diária (0051c7xx), oficina de evento (0082f0xx) e 00abb1xx.
// Cabeçalho da frente B (sessão B95); corpos da unidade 00923290–00923b06 na jp/pktviews983.cpp da frente C (C70).
// Nomes enganosos (aviso C70): m_cur = andamento 0..1, m_target = valor MOSTRADO, m_value = alvo; m_pRes = figura dos tracinhos.
#include "wtypes.h"

class WOverlay;
class WResource;

class CGaugeBar983
{
public:
	CGaugeBar983(); // 00923410
	~CGaugeBar983(); // 00923290: solta as duas figuras (+0x14/+0x18) e o recurso +0x1c

	// 00923450: figuras de fundo e da barra, posição/limites ({x, y, mínimo, máximo}), margem, …; false sem figura
	bool Init(const char* bg, const char* bar, WRect rc, int margin, bool b20, float f24, bool b48, bool b4a);
	void Reset(); // 009232e0: zera +0/+4/+8 (não embutida no 983)
	void SetImages(const char* bg, const char* bar); // 009232f0: troca as figuras (NULL mantém a atual); missão diária (B98)
	void SetValue(float value); // 00923520: valor já mostrado, preso em [m_min, m_max]
	void SetTarget(float value); // 009235e0: valor para onde a barra anda
	void SetNow983(float value); // 00923580: valor-alvo e mostrado juntos (preso em [m_min, m_max])
	void SetOverlays983(WOverlay* bg, WOverlay* bar); // 00923370 (sem chamador direto no 983)
	void SetTicks983(const char* name, int count); // 009233e0: tracinhos (m_pRes) e nº de divisões (m_34)
	void DrawHorz983(); // 00923700 (m_b48)
	void DrawVert983(); // 00923860
	WPoint GetTickPos983(int index); // 00923ac0
	void Process(const float& deltaTime); // 00923650
	void Draw(); // 009239f0
	bool IsDone() const { return m_target == m_value; } // 004f30d0 (COMDAT)
	bool IsFull() const { return m_target == m_max; } // 004f30f0 (COMDAT)

	float m_cur; // +0x0
	float m_target; // +0x4
	float m_value; // +0x8
	float m_min; // +0xc
	float m_max; // +0x10
	WOverlay* m_pOverlay[2]; // +0x14: fundo, barra
	WResource* m_pRes; // +0x1c
	bool m_b20; // +0x20
	float m_f24; // +0x24
	float m_x; // +0x28
	float m_y; // +0x2c
	int m_margin; // +0x30
	int m_34; // +0x34
	int m_w; // +0x38
	int m_h; // +0x3c
	float m_f40; // +0x40
	float m_f44; // +0x44
	bool m_b48; // +0x48
	bool m_b49; // +0x49
	bool m_b4a; // +0x4a
};
C_ASSERT(sizeof(CGaugeBar983) == 0x4c);
