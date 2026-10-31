#pragma once
#include <map>
#include <string.h>

// JP 983: tabela nível → nome do jogador (unidade jp/levelnametable.cpp, 00832050–008345e0 — vão dentro da faixa do titlemanager.cpp do
// mapa). Objetos estáticos nas unidades que a usam (grandprixlimitdlg 00e75f0c, rfcharmasterytiledlg 00e78194). Nomes meus.
struct sLevelName983
{
	// o strcpy embutido deste construtor sai em laço de bytes no 983 (construtor 008331a0 e Get 008329d0)
	sLevelName983() {}
	sLevelName983(const char* n, int v) { strcpy(name, n); value = v; }

	char name[0x10];
	int value; // 0x10 (Get 008329d0: mov [eax+0x10],0) — pontos do nível: 30, 40, 50… (-1 no último)
};

class CLevelNameTable983
{
public:
	CLevelNameTable983(); // 008331a0
	~CLevelNameTable983(); // 00832de0
	sLevelName983 Get(int level); // 008329d0: "dummy" / 0 quando não acha

private:
	std::map<int, sLevelName983> m_names; // 0x0 (dtor 00832de0 lê a cabeça em [this+4]; o par fica a +0x10 do nó)
};
