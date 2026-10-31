#pragma once
#include "rfbase.h"

// JP 983: barra de medidor das janelas da maestria RF (sessão B138; desde a B141 tudo na unidade rfcharmasterystatdetailwnd:
// a faixa 005c6360–005c6400 da B138 são as cópias fora de linha do construtor/??_G no meio dela). RTTI .?AVMasteryGaugeBar@@, vtable 00d0fc78 (só o
// destrutor); 0x18 bytes. Três imagens da RF já criadas pelo dono (RFCharMasteryStatColumn: status_bar_stat, status_bar_slot,
// status_bar_bg); a largura cheia vem da primeira. Criada pela RFCharMasteryStatColumn (005cb0fb, construtor embutido).
// Nomes dos campos meus (…983). Sessão B141: os três ponteiros são um array (SetValue983/SetPicture983 indexam +4 + i*4).
class MasteryGaugeBar
{
public:
	enum { BAR_STAT983 = 0, BAR_SLOT983 = 1, BAR_BG983 = 2 };

	// 005c6360 (cópia fora de linha sem chamador; embutido na RFCharMasteryStatColumn::OnInitialUpdate 005cacd0 — por isso no
	// cabeçalho desde a B141; a masterygaugebar.cpp da B138 saiu)
	MasteryGaugeBar(RFPicture* stat, RFPicture* slot, RFPicture* bg)
	{
		m_pWnd983[BAR_STAT983] = stat;
		m_pWnd983[BAR_SLOT983] = slot;
		m_pWnd983[BAR_BG983] = bg;
		m_max983 = 0;
		m_width983 = stat->GetSize().width;
		m_pWnd983[BAR_STAT983]->ShowWindow(true);
		m_pWnd983[BAR_SLOT983]->ShowWindow(true);
		m_pWnd983[BAR_BG983]->ShowWindow(true);
	}
	virtual ~MasteryGaugeBar() { } // 005c63c0 (cópia fora de linha; embutido no ??_G 005c63d0)

	void SetMax983(int max) { m_max983 = max; } // 005c5aa0 (cópia fora de linha sem chamador)
	// 005c5ab0 (cópia fora de linha sem chamador; embutida na RFCharMasteryStatColumn::UpdateGauge983): largura = value /
	// máximo (até 1) da largura cheia
	void SetValue983(int value, unsigned int index)
	{
		if (index < 2 && m_width983 != 0.0f)
		{
			float rate = (float)value / (float)m_max983;
			if (rate > 1.0f)
				rate = 1.0f;
			RFSize size = m_pWnd983[index]->GetSize();
			size.width = m_width983 * rate;
			m_pWnd983[index]->SetSize(size);
		}
	}
	void Show983(bool show); // 005c5b20
	void SetPicture983(std::wstring name, unsigned int index); // 005c7610

	RFPicture* m_pWnd983[3]; // +0x4 (BAR_*)
	int m_max983; // +0x10
	float m_width983; // +0x14: largura cheia (da 1ª imagem)
};
C_ASSERT(sizeof(MasteryGaugeBar) == 0x18);
