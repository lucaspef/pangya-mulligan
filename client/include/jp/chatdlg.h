#pragma once
// JP 983: FrChatDlg (unidade 0048f0a0–00490ef0; 00490ef0 já é da classicwind). Janela com a conversa do lobby ("chat_group.*":
// fundo, botão de subir/descer, emoticon, idioma, entrada, alvo, vista e denúncia); base de FrGameRoomDlg e FrRoomListDlg.
// É o código de conversa do `CLobbyMain` do 645 (OnChatBgInit…OnEmoticonResult, ToggleChatBtn, o pedaço de conversa do
// ProcessDialogKeyboard e o caso 3 do HandleMsg) mudado para uma classe própria. Existe no 839 (00490370…), não no 645.
// Sessão B42: tamanho/ctor/dtor/mapa só declarados; sessão B108: classe inteira (src/projectg/jp/chatdlg.cpp).
// vtables 00cff63c / 00cff630 / 00cff628; sizeof 0x140 (operator new de 0048f520). Nomes dos membros pelos do CLobbyMain do 645.

#include "frform.h"

class FrArea;
class FrButton;
class FrEdit;
class FrComboBox;
class FrEmoticonDlg;
class FrReportDlg;

class FrChatDlg : public FrForm
{
	DECLARE_OBJECT(FrChatDlg) // GetRTTI 0048f110; MakeInstance 0048f520; registro 00cc86f0

public:
	FrChatDlg(); // 0048f0b0
	virtual ~FrChatDlg(); // 0048f120 (com `delete`: 0048f5b0; ajustes 0048f3f0 / 0048f400)

	// chamadas pelo CLobbyMain do 983 (HandleMsg 00abc140, 00ab7a70); nomes meus
	void SetLanguage983(bool bKor); // 0048f150: "chat_kor" / "chat_eng" no m_pLanguage
	const char* GetChatText983(); // 0048f180: linha 1 da entrada (NULL sem entrada)
	void SetChatTarget983(const char* name); // 0048fcb0: sussurro para `name` (AddWhisperPartner + SetLine) e foco na entrada
	void AddWhisperPartner983(const char* name); // 0048fd00
	void ResetChatTarget983(); // 0048fd20: InitRecentWhisperList + InitReservedChatList(…, 0)
	void ToggleChatBtn(bool bMove); // 0048f6c0
	void AddChatLine983(const char* text, unsigned long color); // 00490d10: o caso 3 do HandleMsg do CLobbyMain do 645

protected:
	virtual void OnProc(const float deltaTime); // 00490430: atalho do emoticon, histórico (PAGE_UP/PAGE_DOWN), cor da entrada
	virtual bool OnInit(); // 0048f140 (só repassa à FrForm)

	void OnChatBgInit(int param); // 0048f5e0
	void OnChatBgOwnerDraw(int param); // 0048f650
	void OnChatWndBtnInit(int param); // 0048fd50
	void OnChatWndBtnUp(); // 0048fdf0
	void OnEmoticonInit(int param); // 0048f960
	void OnEmoticonBtnUp(); // 00490170
	void OnLanguageInit(int param); // 0048f7f0
	void OnChatInputInit(int param); // 0048f890
	bool OnChatInputEnterKey(int param); // 00490e50
	void OnChatTargetInit(int param); // 0048fe10
	void OnChatTargetBtnDown(int param); // 0048f1b0
	bool OnChatTargetEnterKey(int param); // 0048f1c0
	void OnChatViewInit(int param); // 00490b30
	void OnReportBtnUp(); // 004907f0
	bool OnEmoticonResult(int result, FrForm* form); // 0048ffe0
	bool OnReportDlgResult(int result, FrForm* form); // 0048f1a0

	FrEmoticonDlg* m_pEmoticonDlg; // +0x110
	FrReportDlg* m_pReportDlg; // +0x114
	FrArea* m_pChatBg; // +0x118: "chatbg"
	FrButton* m_pChatWndBtn; // +0x11c: "chatwndbtn"
	FrArea* m_pLanguage; // +0x120: "language"
	FrEdit* m_pChatInput; // +0x124: "chatinput"
	FrEdit* m_pChatView; // +0x128: "chatview"
	FrComboBox* m_pChatTarget; // +0x12c: "chattarget"
	FrButton* m_pEmoticon; // +0x130: "emoticon"
	char m_unused134[0x140 - 0x134]; // +0x134: sem acesso no 983 (nada em 0048f0b0..00491000, nem o construtor)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0048f0a0; mapa 00e04210 (14 entradas, inicializador 00cc8710)
};
