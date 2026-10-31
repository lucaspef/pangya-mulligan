#pragma once

// 983 (C-PKT lote 45, out/PKT/s45/RELATORIO.md; sessão A118): cópia da lista de itens do CSharedDoc+0x375c com um cursor
// (artefato da sala). Unidade 00478cc2-0047a060 (sem dono). Nomes provisórios. Header próprio para a A (FrChangeRoomInfoDlg,
// membro +0x1e0) e a B (jp/gameroomdlg.cpp, que hoje tem uma cópia local) usarem a mesma declaração.
// A s159: corpos no changeroominfodlg.cpp. No 983 é um std::list<sArtifactItem983> do VC10 (cabeça +0, tamanho +4, alocadores
// +8) seguido do iterador; aqui os nós são escritos à mão com o mesmo layout (nó 0xd0, como o operator new(0xd0) do 983).
#include "../shared/globalgamedefine.h" // sItemInfo (0xc4)

struct sArtifactItem983
{
	sItemInfo item; // +0x00 (cópia do item do CSharedDoc+0x375c; só o tid quando não é do jogador)
	bool bUsable; // +0xc4 (GetFlag)

	sArtifactItem983() {}
	sArtifactItem983(const sItemInfo& i, bool b) : bUsable(b) { item = i; } // em linha no Fill983 (004798c0)
};

struct sArtifactNode983
{
	sArtifactNode983* pNext; // +0x00
	sArtifactNode983* pPrev; // +0x04
	sArtifactItem983 value; // +0x08
};

// std::list<sArtifactItem983> do VC10 (cabeca, tamanho, alocador): o ctor e fora de linha no 983 (004797a0, partilhado com
// 00604b60/0061f1f0); o ~list fica em linha no ~CItemCursor983 (00479800)
struct sArtifactList983
{
	sArtifactNode983* m_pHead; // +0x00
	int m_size; // +0x04
	int m_8; // +0x08

	sArtifactList983(); // 004797a0
	__forceinline ~sArtifactList983();
};

class CItemCursor983 : public sArtifactList983
{
public:
	CItemCursor983(void* pList); // 00479f40
	~CItemCursor983(); // 00479800
	unsigned long* GetCur(); // 00479e90: &nó_do_cursor->tid (+0xc)
	bool Find(const unsigned long* pTid); // 00479fa0: põe o cursor no item com este tid
	unsigned char GetFlag(); // 00479ea0: byte +0xc4 do item do cursor
	void Reload983(void* pList); // 00478f40: esvazia e refaz sem a entrada "nenhum"

	// E35: iterador (lista circular com sentinela m_pHead); no 983 sao funcoes proprias, chamadas por FrMakeRoomDlg
	bool CanPrev983(); // 00479ee0
	bool CanNext983(); // 00479ef0
	bool StepPrev983(); // 00479fd0
	bool StepNext983(); // 00479ff0

protected:
	void Clear983();
	sArtifactNode983* Buynode983(sArtifactNode983* pNext, sArtifactNode983* pPrev, const sArtifactItem983& v); // 004795d0
	void PushBack983(const sArtifactItem983& v); // 00479870
	void PushBackInline983(const sArtifactItem983& v);
	void Fill983(void* pList, bool bNone); // 004798c0: lista de artefatos pelo gerente de itens

public:
	sArtifactNode983* m_pCur; // +0x0c
};
