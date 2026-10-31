#pragma once

// JP 983: CStatusHelper (vtable 00d12c0c; unidade nova 005e3430-005e49b0, sem fonte). Aqui so o que o club.cpp usa;
// os corpos sao da unidade nova (pedido A -> B, sessao 77).
class CStatusHelper
{
public:
	CStatusHelper(); // 005e3430
	virtual ~CStatusHelper(); // 005e3440
	// 005e48e0: soma das parcelas do atributo (0 = power ... 4) do jogador; 0 se type > 4
	int GetStatus983(unsigned long type);
};
