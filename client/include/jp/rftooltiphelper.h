#pragma once
#include <string>
#include "engine/singleton.h"

class RFTooltip;

// JP 983: RFTooltipHelper (WSingleton<RFTooltipHelper>, vtable 00d122ec, instância 00e74e18; 0x80 bytes, criada pelo
// RFSceneManager::Init983). Unidade rftooltiphelper.cpp 005dfa30–005e0420 (o corte da F3.7.1 dizia 005dfbf0). Guarda a dica
// pedida e mostra uma de três janelas da RF: [1] RFCommonTooltip (texto), [2] RFCommonTooltipByImage (imagem + texto); [0] fica
// nula. Nomes meus, menos a classe.
class RFTooltipHelper : public WSingleton<RFTooltipHelper>
{
public:
	// a dica pedida (0x48; destrutor fora de linha 005dfb40, cópia 005dfdd0, comparação 005dfd30)
	struct sTipInfo983
	{
		bool operator!=(const sTipInfo983& other) const; // 005dfd30

		int target; // +0x0 (1º argumento do Show983; repassado ao slot 34 da janela)
		int type; // +0x4: 1 = texto, 2 = imagem + texto (índice em m_wnds)
		int pos; // +0x8
		std::string text; // +0xc
		std::string image; // +0x28
		int align; // +0x44
	};

	RFTooltipHelper(); // 005dfbf0
	virtual ~RFTooltipHelper(); // 005dfc80 (??_G 005dfda0)
	static RFTooltipHelper* Instance983() { return Instance(); }

	void ShowCurrent983(bool show); // 005dfa30
	void Hide983(); // 005dfad0
	// zera a dica e esconde as três janelas
	void Reset983(); // 005dfb90
	void Set983(int target, int type, int pos, const std::string& text, const std::string& image, int align); // 005dfe20
	void Show983(int target, const std::string& text, int pos, int align); // 005e01c0 (a sala chama com 0, texto, 1, 0x0b)
	void ShowImage983(int target, const std::string& image, const std::string& text, int pos, int align); // 005e02e0

	RFTooltip* m_pCurrent; // +0x28
	sTipInfo983 m_info; // +0x2c
	RFTooltip* m_wnds[3]; // +0x74
};
