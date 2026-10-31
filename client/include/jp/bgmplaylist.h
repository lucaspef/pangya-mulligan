#pragma once
#include "vc10_layout.h"
// JP 983: lista de BGM tocada em sequência (fim da unidade da avatarmotiondlg, 004835d0–00484000; corpo em
// src/projectg/jp/avatarmotiondlg.cpp). Sem RTTI nem vtable: singleton estático local (00483df0, objeto 00e73e94, guarda
// 00e73eb4, destrutor 00483d90 pelo atexit 00cf96b0). O construtor lê "default_bgm".defaultBGM do BGMlist.lua.
// Quem usa: CProjectG::Process 00a68f20 (Process983 a cada quadro: passa à próxima quando a atual acaba), o HandleMsg do
// CLobbyMain 00abc140 (PlayDefault983 / Play983) e o jukebox do 983 (FrJukeBoxDlg: ctor 007ba760 grava m_pJukeBox, dtor
// 007ba400 zera; 007bb400 monta a lista com Clear983/Add983/Play983; 007bb1b0/007bb6a0 SetBGM983 e m_bJukeBoxMode).
// Existe no 839 (par 004858b0…), não no 645. Sessão B112. sizeof 0x20; nomes meus.

#include <vector>
#include <string>

class FrJukeBoxDlg;

class CBGMPlayList983
{
public:
	CBGMPlayList983(); // 00483d00
	~CBGMPlayList983(); // 00483d90 (do atexit 00cf96b0)

	static CBGMPlayList983* Instance(); // 00483df0

	void Play983(); // 004835d0: toca m_list[m_index] (lista vazia: para o BGM que estiver tocando)
	bool IsCurrentBGM983(); // 00483680
	void Process983(const float& dt); // 00483780 (ret 4; dt não é lido)
	void Clear983(); // 00483d60
	void Add983(const char* name); // 00483ef0
	void SetBGM983(const char* name); // 00483fc0: lista de um só e toca
	void PlayDefault983(bool bUnused); // 00483ff0 (ret 4; o argumento não é lido)

protected:
	void Load983(); // 00483940
	__forceinline void PlayIndex983()
	{
		if (m_index >= m_list.size())
			m_index = 0;
		if (m_pJukeBox)
			SelectJukeBox983(m_list[m_index].c_str());
		PlayCurrent983();
	}
	inline void SelectJukeBox983(const char* name);
	__forceinline void PlayCurrent983();

public:
	std::vector<std::string> m_list; // +0x00
	unsigned int m_index; // +0x10
	FrJukeBoxDlg* m_pJukeBox; // +0x14: o jukebox aberto (ele mesmo se grava)
	bool m_bJukeBoxMode; // +0x18: o jukebox mandou tocar; cai quando outro BGM entra
	const char* m_defaultBGM; // +0x1c
};
C_ASSERT_VC10(sizeof(CBGMPlayList983) == 0x20);
