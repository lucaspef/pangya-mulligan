#pragma once
// JP 983: FrHoleSelector (unidade 005100c0–00510a10; vtables 00d06bfc / 00d06bf0 / 00d06be8; sizeof 0x150 pelo `new` de
// 005107a0). Janela de escolha do buraco do modo solo: lista "holeIds" com os 18 números e a miniatura "hole_preview" (duas
// folhas de 10 e 9 quadros de 140 × 93, "<percurso>_1.png" e "_2.png"). Layout pelo construtor 005102a0 e pelos métodos
// (sessão B40). Nomes dos campos e dos métodos são meus.

#include "frform.h"

class FrListBox;
class FrArea;
class Bitmap;
class WOverlay;

class FrHoleSelector : public FrForm
{
	DECLARE_OBJECT(FrHoleSelector)

public:
	FrHoleSelector(); // 005102a0
	virtual ~FrHoleSelector(); // 005100d0

	virtual bool Close(eFormRet result, bool bFade); // 00510190: fecha sem desvanecer

	void SetCourse983(unsigned char course, unsigned char hole); // 00510830 (chamada de 00522c85 e de 00acb5de)
	unsigned char GetHole983() const { return m_hole; } // embutida no 983 (00522960 lê +0x148)

protected:
	virtual void OnMouseMove(const WPoint& mousePos); // 00510930

	float Alpha983() const { return m_wndAlpha; } // embutida no 983 (FrWnd::GetAlpha da frente A é fora de linha)
	void SetPreview983(unsigned char hole); // 005106f0
	void OnPreviewInit(int param); // 00510390
	void OnPreviewDraw(int param); // 00510400
	void OnHoleInit(int param); // 005104d0
	void OnHoleDraw(int param); // 005105a0
	void OnHoleClick(); // 00510980
	void OnHoleHoverOff(int param); // 005109f0

	FrListBox* m_pHoleList; // +0x110: "holeIds"
	FrArea* m_pPreview; // +0x114: "hole_preview"
	const Bitmap* m_pBtn[3]; // +0x118: single_btn_hole_Normal, _Hover, _Select
	WOverlay* m_pCur; // +0x124: folha do buraco mostrado
	WOverlay* m_pSheet[2]; // +0x128: buracos 1–10 e 11–19
	_WRECT m_uv; // +0x130: quadro do buraco dentro da folha
	float m_textX; // +0x140: centro do item da lista
	float m_textY; // +0x144
	unsigned char m_hole; // +0x148: 1–18, 0x13 = nenhum

	DECLARE_FRESH_MSGMAP()
};
