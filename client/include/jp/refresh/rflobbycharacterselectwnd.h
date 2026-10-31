#pragma once
#include <string>
#include <vector>
#include "rfbase.h"
#include "rfloadingprogressdlg.h"

// JP 983: as duas janelas RF da seleção de personagem do lobby (unidade rflobbycharacterselectwnd.cpp, 005d1a70–005d5d20; o
// mesmo .obj traz o HandleMsg983 do RFLobbyDesktop, 005d5520). Classes pela RTTI; nomes dos métodos e campos meus (…983).

// "lobby_char_select_wnd.xml" (RTTI .?AVRFLobbyCharacterSelectWnd@@, vtable 00d10f9c; base RFFormBase<RFWndEx>; 0x334 bytes).
// O botão "btn_character" mostra o personagem escolhido (imagens "char_selcet_btn_main_<nome>_n/o/p.tga"); o clique abre a
// janela animada. Instância 00e78220.
class RFLobbyCharacterSelectWnd : public RFFormBase<RFWndEx>
{
public:
	RFLobbyCharacterSelectWnd(); // 005d2220
	virtual ~RFLobbyCharacterSelectWnd(); // 005d22b0 (??_G 005d2570)

	virtual void OnInitialUpdate(); // 005d5240 (slot 10)

	static RFLobbyCharacterSelectWnd* Instance983() { return ms_pInstance; } // 005d1a70 / 005d1a80
	static void SetInstance983(RFLobbyCharacterSelectWnd* p) { ms_pInstance = p; } // 005d1a90
	static void ClearInstance983() { ms_pInstance = NULL; } // 005d1aa0

	// 005d2980: cria pelo "lobby_char_select_wnd.xml" (modo g_rfCreateMode983) e guarda o pai
	static RFLobbyCharacterSelectWnd* Create983(RFWnd* parent);
	// 005d2ac0: troca o personagem mostrado (tipo 0…14); bSend = avisa o servidor/os bonecos, senão só grava no documento
	void SetChar983(int type, bool bSend);
	// 005d3b90: volta ao personagem em uso e refaz a janela animada
	void Refresh983();
	// 005d2020 (fora de linha): nome do arquivo do personagem ("nuri_old"…"cecilia"), NULL fora da faixa
	const char* GetCharName983(int type)
	{
		if (type < 15)
			return m_names[type].c_str();
		return NULL;
	}

	void OnCharacterClick983(EventArgs& e); // 005d3900: abre/fecha a janela animada
	void OnCharacterEnter983(EventArgs& e) { } // 005d1b30
	void OnCharacterLeave983(EventArgs& e) { } // 005d1b40

	static RFLobbyCharacterSelectWnd* ms_pInstance; // 00e78220
	static RFWnd* s_pParent983; // 00e7821c

	int m_curType983; // +0x184: personagem em uso (0xf = nenhum)
	int m_charType983; // +0x188: personagem escolhido (0xf = nenhum)
	std::string m_names[15]; // +0x18c: CreateCharacterOption::GetCharFileNames983
	bool m_bChanged983; // +0x330: o escolhido não é o em uso
};

// "lobby_char_select_anim_wnd.xml" (RTTI .?AVRFLobbyCharacterSelectAnimWnd@@, vtable 00d1102c; base RFFormBase<RFWndEx>;
// 0x344 bytes): a gaveta com um botão por personagem ("btn_char_<nome>", só os que o jogador tem ficam ativos), animações
// "_Opening_"/"_Closing_"/"_Init_"/"_Normal_" e os dois "btn_closer_*". Instância 00e78228.
class RFLobbyCharacterSelectAnimWnd : public RFFormBase<RFWndEx>
{
public:
	RFLobbyCharacterSelectAnimWnd(); // 005d2670
	virtual ~RFLobbyCharacterSelectAnimWnd(); // 005d2710 (??_G 005d2950)

	virtual void OnInitialUpdate(); // 005d3bf0 (slot 10)

	static RFLobbyCharacterSelectAnimWnd* Instance983() { return ms_pInstance; } // 005d1ab0 / 005d1ac0
	static void SetInstance983(RFLobbyCharacterSelectAnimWnd* p) { ms_pInstance = p; } // 005d1ad0
	static void ClearInstance983() { ms_pInstance = NULL; } // 005d1ae0
	static RFWnd* GetParent983() { return s_pParent983; } // 005d1af0
	bool IsOpen983() { return m_bOpen; } // 005d1b00
	bool IsAnimating983() { return m_bAnimating; } // 005d1b10

	// 005d2ef0: cria pelo "lobby_char_select_anim_wnd.xml" (modo g_rfCreateMode983) e guarda o pai
	static RFLobbyCharacterSelectAnimWnd* Create983(RFWnd* parent);
	// 005d35b0: abre/fecha a gaveta (0 = alterna, 1 = abre, 2 = fecha); bRestore = volta ao personagem de antes de abrir
	void Select983(int what, bool bRestore);
	// 005d3920: refaz a lista dos personagens do jogador, volta ao "_Init_" e marca `type` (0xf = mantém)
	void SetChar983(int type);
	void SetButton983(int type); // 005d3030: marca o botão do personagem e mostra-o na RFLobbyCharacterSelectWnd
	void UpdateButtons983(); // 005d3320: ativa só os botões dos personagens do jogador

	void OnUpdate983(EventArgs& e); // 005d33b0: fim da animação
	void OnAnimationPlay983(EventArgs& e) { m_bAnimating = true; } // 005d1b50
	void OnCloserClick983(EventArgs& e); // 005d3720
	// cliques (005d3170…005d3240) e cliques duplos (005d3740…005d38e0, escolhem e fecham), um par por personagem
	void OnNuriClick983(EventArgs& e);
	void OnNuriOldClick983(EventArgs& e);
	void OnHanaClick983(EventArgs& e);
	void OnHanaOldClick983(EventArgs& e);
	void OnCeciliaOldClick983(EventArgs& e);
	void OnCeciliaClick983(EventArgs& e);
	void OnArinClick983(EventArgs& e);
	void OnLuciaClick983(EventArgs& e);
	void OnNellClick983(EventArgs& e);
	void OnKoohClick983(EventArgs& e);
	void OnSpikaClick983(EventArgs& e);
	void OnKazClick983(EventArgs& e);
	void OnMaxClick983(EventArgs& e);
	void OnAzerClick983(EventArgs& e);
	void OnNuriDblClick983(EventArgs& e);
	void OnNuriOldDblClick983(EventArgs& e);
	void OnHanaDblClick983(EventArgs& e);
	void OnHanaOldDblClick983(EventArgs& e);
	void OnCeciliaOldDblClick983(EventArgs& e);
	void OnCeciliaDblClick983(EventArgs& e);
	void OnArinDblClick983(EventArgs& e);
	void OnLuciaDblClick983(EventArgs& e);
	void OnNellDblClick983(EventArgs& e);
	void OnKoohDblClick983(EventArgs& e);
	void OnSpikaDblClick983(EventArgs& e);
	void OnKazDblClick983(EventArgs& e);
	void OnMaxDblClick983(EventArgs& e);
	void OnAzerDblClick983(EventArgs& e);

	static RFLobbyCharacterSelectAnimWnd* ms_pInstance; // 00e78228
	static RFWnd* s_pParent983; // 00e78224 ("wnd_char_select_anim_area": recorte desligado com a gaveta aberta)

	bool m_bOpen; // +0x184
	bool m_bAnimating; // +0x185
	int m_prevType983; // +0x188: personagem ao abrir
	int m_curButton983; // +0x18c: botão marcado (0xf = nenhum)
	std::vector<unsigned long> m_owned983; // +0x190: tipos dos personagens do jogador
	std::wstring m_btnNames[15]; // +0x1a0: "btn_char_<nome>" por tipo
};

