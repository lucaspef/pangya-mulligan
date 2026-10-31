#pragma once
#include <string>
#include <map>
#include "rfbase.h"
#include "../lobbyjp.h"
#include "fresh.h"
#include "frwndmanager.h"

class CTaskMain;
class CLobbyMain;
class ntAvatarChatMain;
class WTitleFont;
class Bitmap;
class MsgObject;

// JP 983: base das áreas de trabalho da RF (RTTI .?AV?$RFFormDesktopBase@VRFWndEx@@@@, vtable 00d130b4; 0x18c bytes). Ocupa a
// tela inteira (Initialize 005f7840) e, ao fechar, tira a área de trabalho do RFManager. Construtor 005f6000 e destrutor 005f6030
// fora de linha (os das derivadas embutem). Nomes meus (…983), menos a classe e os da RF.
template<class T> class RFFormDesktopBase : public T
{
public:
	RFFormDesktopBase() : m_bSubscribed(false), m_layer(0), m_188(false), m_189(true) { }
	virtual ~RFFormDesktopBase() { }

	virtual void Initialize983(); // 005f7840: tela inteira, pré-desenho
	// 005f6a50 (embutida no Initialize983; a cópia fora de linha vem da instanciação explícita, sem chamador)
	void SetFullScreen983()
	{
		m_layer = 0;
		RFSize size = RFSceneManager::GetManager983()->GetRoot983()->GetSize();
		SetRect(RFRect(0, size.width, 0, size.height)); // 005f6a50: esquerda, direita, cima, baixo
		SetPreDraw(true);
	}
	// 005f69e0 (embutida no OnDestroy das derivadas): tira a área de trabalho do RFManager
	virtual void OnDestroy()
	{
		RFSceneManager::GetManager983()->DeleteDesktop();
		RFSceneManager::Instance()->SetFocusAll983(false);
		RFSceneManager::Instance()->m_41 = false;
		T::OnDestroy();
		m_bSubscribed = false;
	}

	void SetLayer983(int layer) { m_layer = layer; } // 005f6040
	// 005f6af0 (embutida no UpdateFocus983): a tela do jogo (+0xac) não é a desta área
	bool IsOtherLayer983()
	{
		FrWndManager* pScreen;
		if (g_pF02340_983 && (pScreen = FRESHMGR983()) != NULL)
		{
			if (m_layer > 0)
				return m_layer != FRMGR_FORMCOUNT983(pScreen);
			return FRMGR_FORMCOUNT983(pScreen) != 0;
		}
		return false;
	}
	// 005fb820: na 1ª vez assina o EventUpdate; mostrando, vira a área de trabalho do RFManager e libera a entrada
	void Show983(bool show);
	void OnUpdateEvent983(EventArgs& e); // 005fa1f0: visível e sem modal → UpdateFocus983
	void UpdateFocus983(); // 005f8ba0: foco/mouse da RF conforme a tela do jogo e o que está sob o mouse
	bool IsMouseOverOther983(); // 005f8a00: a janela sob o mouse não é esta

	bool m_bSubscribed; // +0x180
	int m_layer; // +0x184 (0 = qualquer tela)
	bool m_188; // +0x188
	bool m_189; // +0x189
};

// JP 983: a área de trabalho do lobby da RF ("lobby_desktop.xml"; RTTI .?AVRFLobbyDesktop@@, vtable 00d1329c; 0x2cc bytes; a
// instância fica em 00e78ca8 — a frente A a vê como CLobbyState983(a)). Barra de baixo do lobby (botões jogo/conversa/prática/
// loja/my room/correio/canal/sair/cartas/mensageiro/minhas informações/opções/anúncio/premium), dinheiro (pang/cookie/won),
// avisos e o menu de botões ("panel_popup"). Unidade rflobbydesktop.cpp 005f5df0–00600070 (o tratador 005d5520, slot 35, mora
// na unidade rflobbycharacterselectwnd). Nomes meus (…983), menos a classe e os da RF.
class RFLobbyDesktop;
#define RFLOBBYDESKTOP983 (*(RFLobbyDesktop**)&CLobbyState983::ms_pInstance)

class RFLobbyDesktop : public RFFormDesktopBase<RFWndEx>
{
public:
	RFLobbyDesktop(); // 005ffc60
	virtual ~RFLobbyDesktop(); // 005f7b70 (??_G 005f7ee0)

	virtual void OnInitialUpdate(); // 005fd000 (slot 10)
	virtual void OnDestroy(); // 005f7080
	virtual void OnLayoutChanged(EventArgs& e); // 005f8d20
	virtual void OnShowDesktop983(bool show); // 005ff4f0 (slot 34, +0x88): mostra (refaz o lobby) / esconde
	virtual void HandleMsg983(const MsgObject& msg); // 005d5520 (slot 35, +0x8c; no rflobbycharacterselectwnd.cpp)

	// 005fff30: cria pelo "lobby_desktop.xml" (modo s_createMode983) e guarda o pai
	static RFLobbyDesktop* Create983(RFWnd* parent);
	static RFLobbyDesktop* Instance983() { return RFLOBBYDESKTOP983; } // (cópias fora de linha 005f5df0…)
	static void SetInstance983(RFLobbyDesktop* p) { RFLOBBYDESKTOP983 = p; } // 005f5df0
	static void ClearInstance983() { RFLOBBYDESKTOP983 = NULL; } // 005f5e00
	static bool IsLobbyMain983() { return g_lobbyModal983 == 0; } // 005f5e10

	// mapa estático nome da tela → número ("" 0, lobbymain 1, shop|char 2 … tutorial 8)
	static void InitScreenMap983(); // 005ff8f0
	static void __stdcall AddScreen983(std::map<std::string, int>& screens, const char* name, int id); // 005ff790 (sem chamador)

	// janelas filhas pelo nome (__fastcall, sem chamador de fora)
	static void ShowChild983(RFWnd* wnd, const wchar_t* name, bool show); // 005fa270
	static void EnableChild983(RFWnd* wnd, const wchar_t* name, bool enable); // 005fa340
	static void FocusChild983(RFWnd* wnd, const wchar_t* name); // 005fa410
	void EnableButton983(std::wstring name, bool enable); // 005fa910 (tradedlg 008f80f7, 009ccc17)

	void Refresh983(); // 005fbe40: dinheiro, canal, anúncio, premium (16 chamadores de fora)
	void UpdateMoney983(); // 005f8380: textos de pang/cookie/won
	void UpdateChannelName983(); // 005fa980: "servidor > canal" (ou "OFFLINE PLAY")
	void UpdateAdLabel983(); // 005fa900 (= 005f8f20)
	void UpdateAdRect983(); // 005f8f20: o COneLineBoard ocupa o "label_ad"
	void UpdatePremium983(bool fade); // 005fae60: botão e dica do premium
	void SetButtonGroup983(int mode); // 005fafe0: ícones dos lados e o botão jogo/prática
	void ShowSpecialCardEmpty983(); // 005fb250 (CTaskMain 00656be0)
	bool IsPopupVisible983(); // 005f9090
	void ShowPopup983(bool show); // 005fafb0: menu de botões ("panel_popup")
	void OnPopupKeyDown983(EventArgs& e); // 005fb260: Esc fecha o menu
	void OnPopupClick983(EventArgs& e); // 005fb280: "btn_popup" abre/fecha o menu
	void OnPopupKillFocus983(EventArgs& e); // 005fb2d0: fecha o menu se o mouse não está nele nem no botão

	void OnGameClick983(EventArgs& e); // 005f74e0 ("btn_game", "btn_talk")
	void OnUpdate983(EventArgs& e); // 005fcb10
	void OnShopClick983(EventArgs& e); // 005fcb70
	void OnMyRoomClick983(EventArgs& e); // 005fcbf0
	void UpdateSpcardButton983(float delta); // 005f9610: piscar do "btn_spcard"
	void UpdateMessengerButton983(float delta); // 005f9780: piscar do "btn_messenger"
	void UpdateShopCamera983(); // 005f7310
	void UpdateLayer983(); // 005fc490: camada da área de trabalho conforme o estado do lobby
	void UpdateInputState983(); // 005f9150: entrada da RF conforme modal/menu/seleção de personagem
	void SetScreen983(int screen); // 005fbe70: troca a tela do lobby (2 loja, 5 my room…)
	void UpdateLobbyActors983(); // 005f8020: câmera e atores do lobby conforme a tela
	bool IsGroupA983() const; // 0055fe80 (cópia fora de linha noutra unidade; = CLobbyState983::IsGroupA983): tela 2, 3 ou 4
	void ResetPremiumFade983(); // 005f5f40
	void UpdatePremiumFade983(float delta); // 005f5ef0
	void LoadPremiumImages983(); // 005f6470
	void DrawPremium983(); // 005f64e0
	void OpenLobbyActors983(); // 005f7210
	void OnCookieClick983(EventArgs& e); // 005f5f60
	void OnNothing983(EventArgs& e); // 005f5f80 ("btn_pang_cookie" entra)
	void OnPangCookieLeave983(EventArgs& e); // 005f5f90 (vazia)
	void OnAdEnter983(EventArgs& e); // 005f6910 (vazia)
	void OnMyInfoClick983(EventArgs& e); // 005f5fa0
	void OnOptionClick983(EventArgs& e); // 005f5fb0
	void OnAdClick983(EventArgs& e); // 005f5fc0
	void OnPremiumClick983(EventArgs& e); // 005f5fd0
	void OnFinalize983(EventArgs& e); // 005f67b0
	void OnHideTooltip983(EventArgs& e); // 005f6870 ("pic_server_name" sai); cópias iguais, uma por botão:
	void OnSpcardLeave983(EventArgs& e); // 005f6890
	void OnMessengerLeave983(EventArgs& e); // 005f68b0
	void OnMyInfoLeave983(EventArgs& e); // 005f68d0
	void OnOptionLeave983(EventArgs& e); // 005f68f0
	void OnPremiumLeave983(EventArgs& e); // 005f6920
	void OnAdLeave983(EventArgs& e); // 005f6940
	void OnPremiumDraw983(EventArgs& e); // 005f6960
	void OnWonDraw983(EventArgs& e); // 005f6970
	void OnAdDraw983(EventArgs& e); // 005f6980
	void OnKeyDown983(EventArgs& e); // 005f67d0
	void OnPracticeClick983(EventArgs& e); // 005f7540: modo solo (C56.1)
	void OnChannelClick983(EventArgs& e); // 005f75a0
	void OnSpcardEnter983(EventArgs& e); // 005f8580
	void OnMessengerEnter983(EventArgs& e); // 005f8640
	void OnMyInfoEnter983(EventArgs& e); // 005f8700
	void OnOptionEnter983(EventArgs& e); // 005f87c0
	void OnPremiumEnter983(EventArgs& e); // 005f8880
	void CallLobbyMain983(); // 005f5ee0
	void UpdateButtons983(); // 005fa7c0: botões conforme o servidor e o tipo do lobby
	void OnLButtonDown983(EventArgs& e); // 005f98f0
	void OnSpcardEmptyUpdate983(EventArgs& e); // 005f9a00 ("wnd_special_card_empty")
	void OnServerNameEnter983(EventArgs& e); // 005fb440 ("pic_server_name")
	void OnPostClick983(EventArgs& e); // 005f9b00
	void OnExitClick983(EventArgs& e); // 005f8470
	void OnSpcardClick983(EventArgs& e); // 005f9c40
	void OnMessengerClick983(EventArgs& e); // 005fcc70
	void OnPangDraw983(EventArgs& e); // 005f7610
	void OnCookieDraw983(EventArgs& e); // 005f7710
	// avisos do HandleMsg983 (005d5520) e dos cliques: ícones e piscar dos botões
	void OnSpcardMsg983(int msg); // 005f92f0: 0xca cartão especial (se houver alarme), 0xcb premium, outro = normal
	void OnMessengerMsg983(int msg); // 005fc520: 0xcd/0xd1 mensagem, 0xce presente, 0xcf presente aceso, 0xcc nada, outro = normal
	void ShowMessengerBubble983(bool show, const std::string& text = ""); // 005fb190
	int GetCharType983(); // 005f6410: personagem escolhido na seleção (00e78220 +0x188; 0xf sem ela; não usa o this)
	unsigned long GetCharTypeId983(); // 005f6430: 0x4000000 | tipo (0 sem personagem)
	void ResetCharSelect983(); // 005f6450 (HandleMsg983): seleção animada com 0xf
	static int FindScreen983(const std::string& name); // 005f8af0 (HandleMsg983): número da tela pelo nome, 0 se não há
	void FocusPopup983(); // 005f8e50 (RFLobbyPopupMenuWnd 005d5d60)
	// itens do menu de botões ("panel_popup"): chamados pelos tratadores da RFLobbyPopupMenuWnd (005d5dc0…005d5ee0); fecham o menu
	void OnMenuGacha983(EventArgs& e); // 005fb620 (conversa de avatar: guilda)
	void OnMenuBongdari983(EventArgs& e); // 005fb670
	void OnMenuPointShop983(EventArgs& e); // 005fb690
	void OnMenuOneDayQuest983(EventArgs& e); // 005fb6b0
	void OnMenuNotice983(EventArgs& e); // 005fb700
	void OnMenuRanking983(EventArgs& e); // 005fb730
	void OnMenuAchievement983(EventArgs& e); // 005fb760
	void OnMenuClub983(EventArgs& e); // 005fb780
	void OnMenuCombine983(EventArgs& e); // 005fb7e0
	void OnMenuGacha2_983(EventArgs& e); // 005fb800

	// a instância (00e78ca8) é o `CLobbyState983::ms_pInstance` e o tipo do lobby (00e78cac: 0 = lobby/CLobbyMain, 1 = conversa de
	// avatar/ntAvatarChatMain) é o `g_lobbyModal983` (include/jp/lobbyjp.h): os outros .obj já citam esses nomes (C66.2); definidos
	// no rflobbydesktop.cpp
	static int s_createMode983; // 00e03268 (= 2: área de trabalho)
	static RFWnd* s_pParent983; // 00e78ca4
	static std::map<std::string, int> s_screens983; // 00e78cb8

	CTaskMain* m_pTaskMain; // +0x18c
	CLobbyMain* m_pLobbyMain; // +0x190
	ntAvatarChatMain* m_pAvatarChat; // +0x194
	WTitleFont* m_pPangFont; // +0x198
	WTitleFont* m_pCookieFont; // +0x19c
	WTitleFont* m_pWonFont; // +0x1a0
	std::string m_pangText; // +0x1a4
	std::string m_cookieText; // +0x1c0
	std::string m_wonText; // +0x1dc
	int m_1f8; // +0x1f8 (1)
	int m_state; // +0x1fc (1; ver CLobbyState983 em lobbyjp.h)
	bool m_200; // +0x200
	float m_204; // +0x204
	std::wstring m_208; // +0x208
	std::string m_224; // +0x224
	bool m_bLocked; // +0x240: os botões não respondem
	bool m_241;
	bool m_242;
	bool m_243;
	bool m_bCursorBusy; // +0x244
	bool m_bOverPet; // +0x245
	bool m_246;
	bool m_247;
	int m_spcardBlink983; // +0x248: "btn_spcard" pisca
	bool m_bMessengerBlink983; // +0x24c: "btn_messenger" pisca
	float m_spcardBlinkTime983; // +0x250
	float m_messengerBlinkTime983; // +0x254
	bool m_bChanged; // +0x258
	bool m_259;
	bool m_25a;
	const Bitmap* m_pPremiumOn0; // +0x25c ("Premium service_on_00")
	const Bitmap* m_pPremiumOn1; // +0x260
	const Bitmap* m_pPremiumOn2; // +0x264
	const Bitmap* m_pPremiumBase; // +0x268 ("Premium service_base")
	bool m_bPremiumFade; // +0x26c
	float m_premiumAlpha; // +0x270 (255 → 0)
	float m_premiumFadeSpeed; // +0x274 (30 por segundo)
	std::string m_278; // +0x278
	std::string m_294; // +0x294
	std::string m_premiumTip; // +0x2b0 (dica do botão premium)
};

// JP 983: balão do mensageiro do lobby ("lobby_messenger_bubble_wnd.xml"; RTTI .?AVRFLobbyMessengerBubble@@, vtable 00d1318c;
// base RFFormBase<RFWndEx>; 0x188). Instância 00e78cb4.
class RFLobbyMessengerBubble : public RFFormBase<RFWndEx>
{
public:
	RFLobbyMessengerBubble() : m_time983(0) { ms_pInstance = this; } // 005f69a0
	virtual ~RFLobbyMessengerBubble(); // 005f69c0 (??_G 005f7810)

	virtual void OnInitialUpdate(); // 005f9df0

	static RFLobbyMessengerBubble* Instance983() { return ms_pInstance; } // 005f5e20 / 005f5e30
	static void SetInstance983(RFLobbyMessengerBubble* p) { ms_pInstance = p; } // 005f5e40
	static void ClearInstance983() { ms_pInstance = NULL; } // 005f5e50

	// 005f9cb0: cria pelo "lobby_messenger_bubble_wnd.xml" (modo g_rfCreateMode983) e guarda o pai
	static RFLobbyMessengerBubble* Create983(RFWnd* parent);
	void Hide983(); // 005f7f10
	void Show983(const std::string& text); // 005f9f10: texto em "notice_box_text", caixa do tamanho do texto, "_Opening_", 5 s
	void OnUpdate983(EventArgs& e); // 005f7f30: conta o tempo e esconde

	static RFLobbyMessengerBubble* ms_pInstance; // 00e78cb4
	static RFWnd* s_pParent983; // 00e78cb0

	float m_time983; // +0x184: segundos até esconder
};
