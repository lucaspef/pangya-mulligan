#pragma once
// JP 983: CClubSetInfoFont (vtable 00d0068c, só o destrutor; RTTI .?AVCClubSetInfoFont@@; sizeof 0x10 pelo `operator new` de
// 0049a6b0 / 004a51a2). Fonte do nível e do rank do taco na oficina: as letras do rank ("[font_rank.png", "EDCBASL") pela
// WTitleFontEx e o número do nível por uma CWindFont própria. Corpos na unidade da FrClubSetResetDlgBase
// (src/projectg/jp/clubsetresetwnd.cpp, 00499800…). Usada também pela FrClubSetWorkshopSelector (004a4540 / 004a4ef0).
// Nomes dos métodos e dos campos são meus (sessão B89).
#include "titlefontex.h"

class CClubSetInfoFont : public WTitleFontEx
{
public:
	CClubSetInfoFont(); // 00499800
	virtual ~CClubSetInfoFont() // 004998e0 (com `delete`: 004999e0, que embute este)
	{
		if (m_pNumFont)
		{
			delete m_pNumFont;
			m_pNumFont = NULL;
		}
	}

	// 00499950: "L" pela fonte do rank e o nível (level + 1) pela CWindFont, 2 à direita e 3 abaixo
	void PrintLevel983(int level, float x, float y, eTITLEFONTALINE align, float space, float scale);
	// 0049aec0: a letra do rank (0 = E … 5 = S); fora disso, nada
	void PrintRank983(int rank, float x, float y, eTITLEFONTALINE align, float space, float scale);

	CWindFont* m_pNumFont; // +0xc
};
C_ASSERT(sizeof(CClubSetInfoFont) == 0x10);
