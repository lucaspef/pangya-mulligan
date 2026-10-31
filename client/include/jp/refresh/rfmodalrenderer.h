#pragma once
#include <list>

// JP 983: RFModalRenderer (RTTI .?AVRFModalRenderer@@, vtable 00d12c14; 4 bytes, só o vptr) — a 2ª base das janelas modais da
// RF (RFFormModalBase<T> +0x180). Pilha estática das modais abertas (std::list em 00e78c64: cabeça 00e78c68, tamanho
// 00e78c6c); a de cima escurece a tela. Unidade rfmodalrenderer.cpp 005e51f0–005e5c20. Nomes meus, menos a classe.
class RFModalRenderer
{
public:
	RFModalRenderer(); // 005e51f0
	virtual ~RFModalRenderer(); // 005e5200 (??_G 005e5400)

	// a modal de cima escurece a tela: alfa (10·n + 100)·a, n = nº de modais (até 6), a = 1 com mais de uma
	void Draw983(float alpha); // 005e58f0 (chamada por 004777c0, o EventDraw do RFFormModalBase)
	void Push983(); // 005e5b70 (RFFormModalBase::DoModal 004783e0)
	void Remove983(); // 005e5ab0

	static std::list<RFModalRenderer*> s_modals983; // 00e78c68 (cabeça; tamanho em 00e78c6c)
};
