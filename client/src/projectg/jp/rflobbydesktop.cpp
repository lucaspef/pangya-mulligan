// JP 983: unidade 005f5df0–00600070 — a área de trabalho do lobby da RF (RFLobbyDesktop, "lobby_desktop.xml"), a base
// RFFormDesktopBase<RFWndEx> e o balão do mensageiro (RFLobbyMessengerBubble). Depois de 00600070 vêm os moldes da
// rftexturemanager. 1º lote (B131): construtor/destrutor, criação, mapa de telas, fechamento, tratadores pequenos dos botões e
// das dicas. Falta: OnInitialUpdate 005fd000 (9 kB, tabela das 45 assinaturas em out/F3.7/s131/RELATORIO.md), desenho do
// dinheiro/premium, menu de botões, avisos, mensageiro e atualização (005f8020, 005f8f20, 005f9150…005fcc70, 005ff4f0).
#include "stdafx.h"
#include "vc10_layout.h"
#include "refresh/rflobbydesktop.h"
#include "rftooltiphelper.h"
#include "lobbyjp.h"
#include "taskmanager.h"
#include "taskmain.h"
#include "kbdmanualdlg.h"
#include "avatar_task.h"
#include "golftask.h"
#include "frdesktop.h"
#include "frwndmanager.h"
#include "lobbymain.h"
#include "actor.h"
#include "engine/wresrcmng.h"
#include "projectg.h"
#include "onelineboard.h"
#include "shareddoc.h"
#include "messengerdlg.h"
#include "giftalarmdlg.h"
#include "fresh.h"
#include "taskform.h"
#include "binstr.h"
#include "logininfo.h"
#include "engine/wfont.h"
#include "shareddocjp.h"
#include "chatmsg.h"
#include "quitwindow.h"
#include "cardsystem.h"
#include "carddlg.h"
#include "itembuffactor.h"
#include "commonutil.h"
#include "gamestringmanager.h"
#include "frgraphicinterface.h"

extern COneLineBoard* g_pOneLineBoard983; // 00e78cc8 (cópias de leitura 005f6050/005f6060)
void __fastcall SendLobbyChar983(unsigned long id); // 00566f30 (lobbyplayer.cpp)

#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60
#include "refresh/rflobbycharacterselectwnd.h"

#include "refresh/rflobbypopupmenuwnd.h"
#include "refresh/rfservernoticedlg.h"
#include "onelinereqdlg.h"

// visões parciais das janelas RF de outras unidades (ainda não escritas)
extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern bool g_bQuit; // 00f02489
extern WView* g_pMainView983; // 00f02458
extern const float g_fRFScreenH983; // 00d0562c (= 800: a largura; nomes trocados, ver B129)
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)
namespace S5 { void SecToSysTime983(SYSTEMTIME* pOut, __int64 sec); } // 00b307f0
int GetServerType983(); // 00b2eae0

// visões da frente A: métodos do CTaskMain/CLobbyMain que a barra chama (nomes meus, sem corpo aqui)
class CTaskMainLobbyJp983 : public CTaskMain
{
public:
	using CTaskMain::OnUnderBar_CookieUp; // 006584e0 (slot +0x4c)
	using CTaskMain::OnOverBar_OptionUp; // 00659d30
	using CTaskMain::OnUnderBar_ShopUp; // (slot +0x44)
	using CTaskMain::OnUnderBar_MyRoomUp; // (slot +0x48)
	using CTaskMain::OnUnderBar_ExitUp; // 00658a20
	using CTaskMain::OnUnderBar_GiftUp; // 00658420
	using CTaskMain::OnUnderBar_GameUp; // 006586c0: Start Game (abre o "roomlistselector")
	void OnUnderBar_SpecialCardUp983(const WRect& button); // 00656be0: lista de cartões especiais junto do botão
	void OnUnderBar_MessengerUp983(); // 0065b7d0
	using CTaskMain::OnUnderBar_GuildUp; // slot +0x50
	using CTaskMain::OnUnderBar_BongdariShopUp; // slot +0x54
	using CTaskMain::OnUnderBar_RankingUp; // slot +0x58
	using CTaskMain::OnUnderBar_ClubUp983; // slot +0x5c
	using CTaskMain::OnUnderBar_NoticeUp; // slot +0x60
	using CTaskMain::OnUnderBar_GachaUp983; // 00658e70
	using CTaskMain::OnUnderBar_PointShopUp983; // 00659340
	using CTaskMain::OnUnderBar_AchievementUp983; // 006582f0
	using CTaskMain::OnUnderBar_CombineUp983; // 00658e20
	void OnOverBar_MyInfoUp983(); // 0065ce40
	void OnOverBar_AdUp983(); // 0065d610
	void CheckLobbyHotKeys983(); // 0065c580: teclas do lobby ("FUNC_12"…)
	using CTaskMain::m_pKbdManualDlg; // +0x158
	using CTaskMain::m_bUnderTabOn; // +0x1d4 (D866: RFLobbyDesktop o zera pelo membro)
	using CTaskMain::OnKbdManualDlgResult;
};
// visão do ntAvatarChatMain (avatar_main.h não entra junto do lobby): as duas virtuais novas depois das do CTaskMain (vtable 00d25e64)
class ntAvatarChatMainJp983 : public CTaskMainLobbyJp983
{
public:
	virtual void Slot64_983(); // 007a9990 (+0x64)
	virtual void OnToppage_OneDayQuestDown983(); // 007b0d20 (+0x68)
};
class CLobbyMainDesktopJp983 : public CLobbyMain
{
public:
	void OpenPremiumService983(); // 00596370 (o "TOPPAGE.PremiumService" do 645)
	void OnDesktop983(); // 00ab8430
	using CLobbyMain::OnToppage_OneDayQuestDown983; // 00593990
};

CLobbyState983* CLobbyState983::ms_pInstance; // 00e78ca8 (= RFLOBBYDESKTOP983)
int g_lobbyModal983; // 00e78cac
// 00ab8430 = CLobbyMain::OpenMotionDlg983; 00596370 = CLobbyMain::OpenPremiumService983 (eram stub no exe)
void CLobbyMainDesktopJp983::OnDesktop983()
{
	OpenMotionDlg983();
}
void CLobbyMainDesktopJp983::OpenPremiumService983()
{
	CLobbyMain::OpenPremiumService983();
}

int RFLobbyDesktop::s_createMode983 = 2;
RFWnd* RFLobbyDesktop::s_pParent983;
std::map<std::string, int> RFLobbyDesktop::s_screens983;
RFLobbyMessengerBubble* RFLobbyMessengerBubble::ms_pInstance;

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: pequenas

// 0065ce40: fecha os dialogos, CUserInfo::SetInfo(meu uid/guid/nick, 1, 1, 0, 0) e tira o hover da barra = CTaskMain::OnOverBar_MyInfoUp
void CTaskMainLobbyJp983::OnOverBar_MyInfoUp983()
{
	OnOverBar_MyInfoUp();
}

// 0065d610: botao do Ticker (btn_ad, 005f5fc0) = o OnOverBar_OneLineReqBtnUp do 645 sem a consulta ao gateway (0x26d 0x10/0x11)
// e sem o SetIconRect: servico 0x20000 fechado -> aviso 0x23; senao fecha os dialogos (vtable+0x3c), bloqueio 0x10 (doc+0x83f)
// -> "notify" de 12 s (00d06668); senao abre o "oneline_req" (resultado 0065cfe0, 0x11) e tira o hover da barra (00656a80).
void CTaskMainLobbyJp983::OnOverBar_AdUp983()
{
	if (CSharedDoc::Instance()->IsControlServerService(0x20000))
	{
		this << MsgObject(this, 0x23,
			(int)K2L_Compatibility("\xc0\xfc\xb1\xa4\xc6\xc7 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
								   "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		return;
	}
	CloseAllDialogs();
	if (CSharedDoc::Instance()->m_myInfo.info.dwFlagBlock & 0x10)
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetTimeLimit(12.0f);
		pForm->SetMessage(K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc0\xfc\xb1\xa4\xc6\xc7 \xc0\xcc\xbf\xeb "
											"\xc1\xa6\xc0\xe7\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. "
											"\xc0\xfc\xb1\xa4\xc6\xc7\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 "
											"\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 0);
		return;
	}
	m_pOnelineReqDlg = CreateForm<FrOnelineReqDlg>(g_pFresh->GetManager(), this, "oneline_req", NULL);
	m_pOnelineReqDlg->Open((FRESH_PFN_RESULT)&CTaskMainLobbyJp983::OnOnelineReqDlgResult, 0x11);
	if (!g_pFresh->GetManager()->IsKeyExclusive())
		g_pFresh->GetManager()->SetExclusiveKey(true);
	OnBarHoverOff(0);
}

// 0065b7d0: botao Messenger da barra. O 983 usa o mesmo botao para os alarmes: aviso do mensageiro (doc+0x10d81 ->
// OnMessengerAlarmBtnUp), tesouro (doc+0x11038 == 1, com doc+0x10d82 = 0), presente (doc+0x10d80 -> 00659df0: "gift_alarm",
// o dialogo "Gift") e, sem alarme, abre o mensageiro (00653ab0: vtable+0x3c e 00905170 = CMessengerInfo; aqui o Open(true)
// do 645). 00659df0 (D662): bloqueia a entrada da cena 3D (006038f0(1)), abre o "gift_alarm" e o deixa por cima
// (00be85e0 SetTopmost(1)) com o foco (00be8860 FindNextTopFocus(0)).
void CTaskMainLobbyJp983::OnUnderBar_MessengerUp983()
{
	OnOverBar_MessengerUp();
}

// 00656be0: botao do cartao especial (btn_spcard, 005f9c40) = o OnOverBar_SpecialCardUp do 645 refeito: o "spcardbuff" abre
// centrado em x sobre o botao (preso na largura da tela) e logo abaixo dele (Open modo 5); sem o SetVisible(false)/SetFixed/
// EnableBlink e sem o OnBarHoverOff. Sem cartao: fecha e o RFLobbyDesktop mostra "wnd_special_card_empty" (005fb250), em vez do
// aviso 0xa8 do 645. 00806700 = FrSPCardBuffDlg::UpdatePremium983 (linha extra do servico premium 0x1a100002, bit 14; A319).
void CTaskMainLobbyJp983::OnUnderBar_SpecialCardUp983(const WRect& button)
{
	CCardManager::Instance()->SetSPCardAlarm(false);
	if (m_pSpecialCardDlg)
	{
		m_pSpecialCardDlg->Close(FrCANCEL, true);
		return;
	}
	m_pSpecialCardDlg = CreateForm<FrSPCardBuffDlg>(g_pFresh->GetManager(), this, "spcardbuff", NULL);
	CCardManager::Instance()->SetCardBuffDlg(m_pSpecialCardDlg);
	CItemBuff::UpdateBuffDlg(m_pSpecialCardDlg);
	m_pSpecialCardDlg->UpdatePremium983();
	float w = m_pSpecialCardDlg->GetRect().w;
	WPoint pos(button.x - w * 0.5f, button.y + button.h);
	float screenW = (float)g_pMainView983->GetWidth();
	if (screenW < pos.x + w)
		pos.x -= (w + pos.x) - screenW;
	m_pSpecialCardDlg->EnableDrag(false);
	m_pSpecialCardDlg->Open((FRESH_PFN_RESULT)&CTaskMainLobbyJp983::OnSpecialCardFormResult, pos, 5);
	if (m_pSpecialCardDlg->GetSpecialCardNum() < 1)
	{
		if (m_pSpecialCardDlg)
			m_pSpecialCardDlg->Close(FrCANCEL, true);
		if (CLobbyState983::ms_pInstance)
			((RFLobbyDesktop*)CLobbyState983::ms_pInstance)->ShowSpecialCardEmpty983();
		return;
	}
	m_pSpecialCardDlg->SetVisible(true);
}

// 0065c580 (CTaskMain, trecho "teclas" do OnProcess 0065d440): F12 abre/fecha o manual de teclado; ESC = voltar (ou fecha a aba
// aberta); SEARCH abre o "ItemSearchDlg" da loja/my room. DIFERENTE: o 983 decide o ESC pelo tipo da tarefa corrente (00902650/
// 009b79c0 + RTTI e7ad08/e78fac); aqui as mesmas tarefas pelas classes (golfe/avatar chat fora, como no 645).
void CTaskMainLobbyJp983::CheckLobbyHotKeys983()
{
	if (g_input->GetDown("FUNC_12", true))
	{
		if (!m_pKbdManualDlg)
		{
			m_pKbdManualDlg = CreateForm<FrKbdManualDlg>(g_pFresh->GetManager(), this, "kbd_manual", NULL);
			if (m_pKbdManualDlg)
			{
				m_pKbdManualDlg->Open((FRESH_PFN_RESULT)&CTaskMainLobbyJp983::OnKbdManualDlgResult, 1);
				m_pKbdManualDlg->EnableDrag(false);
			}
		}
		else
			m_pKbdManualDlg->Close(true);
	}
	// C759: 983 0065c61d so aceita 1; 3 = ESC ja gasto por form (ExclusiveGetDownUseDone), ex. gameroom_single sem FrESCAPE
	if (g_input->GetDown("ESCAPE", true) == 1 && !g_pFresh->GetManager()->HasEscKeyWindow())
	{
		// C116.2: com o Pop-Up (panel_popup) aberto o ESC fecha o painel (o original fecha e ainda abre o Esc Menu)
		if (RFLobbyDesktop::Instance983() && RFLobbyDesktop::Instance983()->IsPopupVisible983())
		{
			RFLobbyDesktop::Instance983()->ShowPopup983(false);
			C_ASSERT_VC10(offsetof(CTaskMainLobbyJp983, m_bUnderTabOn) == 0x1d4); // D861: era o cru +0x1d4 (anda no cl novo)
			m_bUnderTabOn = false; // E154: aba do Pop-Up ligada (+0x1d4) faria o ESC virar "fechar aba" sem Esc Menu
		}
		FrWndManager* mgr = g_pFresh->GetManager();
		if (mgr->GetDesktop()->FindChildByName("holeinoneresult") || mgr->GetDesktop()->FindChildByName("uniteresult"))
			return;
		if (CMessengerInfo::Instance()->IsVisible() && !IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		{
			CMessengerInfo::Instance()->Close(false); // 983 0065c68a: mensageiro fecha antes da aba e do Esc Menu
			return;
		}
		if (m_bUnderTabOn && mgr->GetDesktop()->IsViewFocused())
		{
			OnUnderBar_TabBtnUp();
			return;
		}
		if (!IS_EXACTKINDOF(CGolfTask, AfxGetTask()) && !IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()) &&
			!(CLobbyState983::ms_pInstance && RFLOBBYDESKTOP983->m_bLocked))
			OnUnderBar_BackUp();
	}
	if (g_input->GetDown("SEARCH", true))
	{
		FrTaskForm* form = AfxGetTask()->GetWidgetForm983(); // NULL apos ForgetWidgetForm983 (B370)
		const std::string name = form ? form->GetWindowName983() : std::string();
		if (name == "realmyroom" || name == "shopmain")
			CreateMaketDlg();
	}
}

// 004db4b0 (par 839 004dfbc0): interpolacao quartica do inicio ao fim
float __cdecl Fade983(float from, float to, float time, float duration)
{
	time = time / duration;
	return time * time * time * time * (to - from) + from;
}

// 005f5ee0
void RFLobbyDesktop::CallLobbyMain983()
{
	if (m_pLobbyMain)
		((CLobbyMainDesktopJp983*)m_pLobbyMain)->OnDesktop983();
}

// 005f5ef0
void RFLobbyDesktop::UpdatePremiumFade983(float delta)
{
	if (m_bPremiumFade)
	{
		m_premiumAlpha = m_premiumAlpha - m_premiumFadeSpeed * delta;
		if (m_premiumAlpha < 0.0f)
			m_premiumAlpha = 0.0f;
	}
}

// 005f5f40
void RFLobbyDesktop::ResetPremiumFade983()
{
	m_bPremiumFade = false;
	m_premiumAlpha = 255.0f;
	m_premiumFadeSpeed = 30.0f;
}

// 005f5f60
void RFLobbyDesktop::OnCookieClick983(EventArgs& e)
{
	if (!m_bLocked)
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_CookieUp();
}

// 005f5f80
void RFLobbyDesktop::OnNothing983(EventArgs& e)
{
}

// 005f5f90
void RFLobbyDesktop::OnPangCookieLeave983(EventArgs& e)
{
}

// 005f5fa0
void RFLobbyDesktop::OnMyInfoClick983(EventArgs& e)
{
	((CTaskMainLobbyJp983*)m_pTaskMain)->OnOverBar_MyInfoUp983();
}

// 005f5fb0
void RFLobbyDesktop::OnOptionClick983(EventArgs& e)
{
	// O6 (A245): original nao deixa a dica "Options" com a Settings aberta
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
	((CTaskMainLobbyJp983*)m_pTaskMain)->OnOverBar_OptionUp();
}

// 005f5fc0
void RFLobbyDesktop::OnAdClick983(EventArgs& e)
{
	((CTaskMainLobbyJp983*)m_pTaskMain)->OnOverBar_AdUp983();
}

// 005f5fd0
void RFLobbyDesktop::OnPremiumClick983(EventArgs& e)
{
	if (g_lobbyModal983 == 0 && m_pLobbyMain)
		((CLobbyMainDesktopJp983*)m_pLobbyMain)->OpenPremiumService983();
}

// ---------------------------------------------------------------------------------------------------------------------
// RFFormDesktopBase<RFWndEx>

// 005f7840
template<class T> void RFFormDesktopBase<T>::Initialize983()
{
	SetFullScreen983();
	RFWnd::Initialize983();
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: premium

// 005f6470
void RFLobbyDesktop::LoadPremiumImages983()
{
	m_pPremiumOn0 = g_pFresh->GetBitmap("Premium service_on_00");
	m_pPremiumOn1 = g_pFresh->GetBitmap("Premium service_on_01");
	m_pPremiumOn2 = g_pFresh->GetBitmap("Premium service_on_02");
	m_pPremiumBase = g_pFresh->GetBitmap("Premium service_base");
	ResetPremiumFade983();
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: eventos pequenos

// 005f67b0
void RFLobbyDesktop::OnFinalize983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Reset983();
}

// 005f67d0: Esc fecha / Enter abre a seleção de personagem (sem modal e sem tela por cima)
void RFLobbyDesktop::OnKeyDown983(EventArgs& e)
{
	RFManager* manager = RFSceneManager::GetManager983();
	if (manager && manager->IsModalExist())
		return;
	FrWndManager* pScreen;
	if (g_pF02340_983 && (pScreen = FRESHMGR983()) != NULL && FRMGR_FORMCOUNT983(pScreen))
		return;
	if (e.m_0c == VK_ESCAPE)
	{
		RFLobbyCharacterSelectAnimWnd* sel = RFLobbyCharacterSelectAnimWnd::ms_pInstance;
		if (sel && !sel->m_bAnimating && sel->m_bOpen)
			sel->Select983(2, true);
	}
	else if (e.m_0c == VK_RETURN)
	{
		RFLobbyCharacterSelectAnimWnd* sel = RFLobbyCharacterSelectAnimWnd::ms_pInstance;
		if (sel && !sel->m_bAnimating && sel->m_bOpen)
			sel->Select983(2, false);
	}
}

// 005f6870 (cópias iguais, uma por botão, logo abaixo)
void RFLobbyDesktop::OnHideTooltip983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 005f6890
void RFLobbyDesktop::OnSpcardLeave983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 005f68b0
void RFLobbyDesktop::OnMessengerLeave983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 005f68d0
void RFLobbyDesktop::OnMyInfoLeave983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 005f68f0
void RFLobbyDesktop::OnOptionLeave983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 005f6920
void RFLobbyDesktop::OnPremiumLeave983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 005f6940
void RFLobbyDesktop::OnAdLeave983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Hide983();
}

// 005f6910
void RFLobbyDesktop::OnAdEnter983(EventArgs& e)
{
}

// 005f64e0: "Premium service" no meio da tela (fundo + 3 figuras lado a lado), apagando; no fim volta ao começo
void RFLobbyDesktop::DrawPremium983()
{
	const Bitmap* on0 = m_pPremiumOn0;
	const Bitmap* on1 = m_pPremiumOn1;
	const Bitmap* on2 = m_pPremiumOn2;
	const Bitmap* base = m_pPremiumBase;
	if (m_bPremiumFade && on0 && on1 && on2 && base)
	{
		FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
		unsigned long color = ((unsigned char)m_premiumAlpha << 24) | 0xffffff;
		float x = *(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x19c)) * 0.5
			- ((float)on0->Width() + (float)on1->Width() + (float)on2->Width()) * 0.5;
		float y = *(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x1a0)) * 0.5 - (float)on0->Height() * 0.5;
		pGI->DrawTexture(base, WRect(0.0f, y - 20.0, *(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x19c)), (float)on0->Height() + 40.0),
			color, 0);
		pGI->DrawTexture(on0, WRect(x, y, (float)on0->Width(), (float)on0->Height()), color, 0);
		pGI->DrawTexture(on1, WRect((float)on0->Width() + x, y, (float)on1->Width(), (float)on1->Height()), color, 0);
		pGI->DrawTexture(on2, WRect((float)on0->Width() + x + (float)on1->Width(), y, (float)on2->Width(),
			(float)on2->Height()), color, 0);
		if (m_premiumAlpha <= 0.0f)
			ResetPremiumFade983();
	}
}

// 005f6960
void RFLobbyDesktop::OnPremiumDraw983(EventArgs& e)
{
	DrawPremium983();
}

// 005f6970
void RFLobbyDesktop::OnWonDraw983(EventArgs& e)
{
	CProjectG::Instance()->HidePrivacy();
}

// 005f6980
void RFLobbyDesktop::OnAdDraw983(EventArgs& e)
{
	if (g_pOneLineBoard983)
		g_pOneLineBoard983->Display();
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: abrir/fechar

// 005f7080: avisa os atores do lobby e tira a área de trabalho
void RFLobbyDesktop::OnDestroy()
{
	AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x183, 0, 0, 0, 0, 0);
	AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x184, 0, 0, 0, 0, 0);
	AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x184, 0, 0, 0, 0, 0);
	AfxGetTask()->GetActor("LobbyMascot") << MsgObject(NULL, 0x184, 0, 0, 0, 0, 0);
	::Doc()->m_bLobbyBgCameraSway983 = true;
	SendLobbyChar983(0);
	RFFormDesktopBase<RFWndEx>::OnDestroy();
}

// 005f7210
void RFLobbyDesktop::OpenLobbyActors983()
{
	if (g_lobbyModal983 == 0)
	{
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x185, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x1a5, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("LobbyMascot") << MsgObject(NULL, 0x1aa, 0, 0, 0, 0, 0);
	}
}

// 005f7540: modo solo (C56.1)
void RFLobbyDesktop::OnPracticeClick983(EventArgs& e)
{
	if (!m_bLocked)
	{
		FrTaskForm* form = AfxGetTask()->GetWidgetForm983();
		if (form)
			form->Close(true);
		RFSceneManager::Instance()->SetInputBlocked983(true);
		m_pLobbyMain->OpenSingleModeStarter983();
	}
}

// 005f75a0: canal
void RFLobbyDesktop::OnChannelClick983(EventArgs& e)
{
	if (!m_bLocked)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x112, 0, 0, 0, 0, 0));
}

// 005f7b70
RFLobbyDesktop::~RFLobbyDesktop()
{
	m_pTaskMain = NULL;
	m_pLobbyMain = NULL;
	if (g_resrcmng && m_pPangFont)
	{
		g_resrcmng->Release(m_pPangFont);
		m_pPangFont = NULL;
	}
	if (g_resrcmng && m_pCookieFont)
	{
		g_resrcmng->Release(m_pCookieFont);
		m_pCookieFont = NULL;
	}
	if (g_resrcmng && m_pWonFont)
	{
		g_resrcmng->Release(m_pWonFont);
		m_pWonFont = NULL;
	}
	RFLOBBYDESKTOP983 = NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: dicas dos botões

// 005f8580
void RFLobbyDesktop::OnSpcardEnter983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Show983((int)e.m_pWnd, K2L_Compatibility("\xc8\xbf\xb0\xfa\xc1\xa4\xba\xb8"), 1, 0xb);
}

// 005f8640
void RFLobbyDesktop::OnMessengerEnter983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Show983((int)e.m_pWnd, K2L_Compatibility("\xb8\xde\xbd\xc5\xc0\xfa"), 1, 0xb);
}

// 005f8700
void RFLobbyDesktop::OnMyInfoEnter983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Show983((int)e.m_pWnd, K2L_Compatibility("\xb3\xbb\xc1\xa4\xba\xb8"), 1, 0xb);
}

// 005f87c0
void RFLobbyDesktop::OnOptionEnter983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Show983((int)e.m_pWnd, K2L_Compatibility("\xbf\xc9\xbc\xc7"), 1, 0xb);
}

// 005f8880
void RFLobbyDesktop::OnPremiumEnter983(EventArgs& e)
{
	if (!m_premiumTip.empty() && RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Show983((int)e.m_pWnd, m_premiumTip.c_str(), 1, 0xb);
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: mapa de telas, construtor, criação

// 005ff790
void __stdcall RFLobbyDesktop::AddScreen983(std::map<std::string, int>& screens, const char* name, int id)
{
	screens[name] = id;
}

// 005ff8f0
void RFLobbyDesktop::InitScreenMap983()
{
	if (s_screens983.size() == 0)
	{
		s_screens983[""] = 0;
		s_screens983["lobbymain"] = 1;
		s_screens983["shop|char"] = 2;
		s_screens983["shop|caddie"] = 3;
		s_screens983["shop|item"] = 4;
		s_screens983["myroom|char"] = 5;
		s_screens983["myroom|caddie"] = 6;
		s_screens983["myroom|item"] = 7;
		s_screens983["tutorial"] = 8;
	}
}

// 005ffc60
RFLobbyDesktop::RFLobbyDesktop()
	: m_pTaskMain(NULL), m_pLobbyMain(NULL), m_pPangFont(NULL), m_pCookieFont(NULL), m_pWonFont(NULL), m_1f8(1), m_state(1),
	m_200(false), m_204(0), m_bLocked(false), m_241(false), m_242(false), m_243(false), m_bCursorBusy(false), m_bOverPet(false),
	m_246(false), m_247(false), m_spcardBlink983(0), m_bMessengerBlink983(false), m_spcardBlinkTime983(0), m_messengerBlinkTime983(0),
	m_bChanged(false), m_259(false), m_25a(false), m_pPremiumOn0(NULL), m_pPremiumOn1(NULL), m_pPremiumOn2(NULL), m_pPremiumBase(NULL), m_bPremiumFade(false), m_premiumAlpha(255.0f),
	m_premiumFadeSpeed(0)
{
	RFLOBBYDESKTOP983 = this;
	InitScreenMap983();
}

// 005fff30
RFLobbyDesktop* RFLobbyDesktop::Create983(RFWnd* parent)
{
	RFLobbyDesktop* p = NULL;
	switch (s_createMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyDesktop>(L"lobby_desktop.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyDesktop>(L"lobby_desktop.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFLobbyDesktop>(L"lobby_desktop.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// ---------------------------------------------------------------------------------------------------------------------
// RFFormDesktopBase<RFWndEx>: mostrar e foco (2º lote, B132)

// 005f8a00
template<class T> bool RFFormDesktopBase<T>::IsMouseOverOther983()
{
	bool over = false;
	WVector* pos = RFSceneManager::Instance()->GetPos983();
	RFManager* manager = RFSceneManager::GetManager983();
	RFWnd* hit = manager->HitTest2(RFVector2(pos->x, pos->y), true, true);
	if (hit)
		over = hit->GetName983() != GetName983(); // cl 14.44: _name da lib em +0xb8, m_name da visao = +0xd8 fixo
	return over;
}

// 005f8ba0
template<class T> void RFFormDesktopBase<T>::UpdateFocus983()
{
	if (!m_189)
		return;
	if (IsOtherLayer983())
	{
		if (RFSceneManager::Instance()->HasRFFocus983())
			RFSceneManager::Instance()->SetRFFocus983(false);
		if (!RFSceneManager::Instance()->IsInputBlocked983())
			RFSceneManager::Instance()->SetInputBlocked983(true);
		return;
	}
	if (RFSceneManager::Instance()->IsInputBlocked983())
		RFSceneManager::Instance()->SetInputBlocked983(false);
	bool over = IsMouseOverOther983();
	if (m_layer == FRMGR_FORMCOUNT983(FRESHMGR983()))
	{
		if (over != RFSceneManager::Instance()->HasRFMouseFocus983())
			RFSceneManager::Instance()->SetRFMouseFocus983(over);
	}
	else
	{
		if (over != RFSceneManager::Instance()->HasRFFocus983())
			RFSceneManager::Instance()->SetRFFocus983(over);
	}
}

// 005fa1f0
template<class T> void RFFormDesktopBase<T>::OnUpdateEvent983(EventArgs& e)
{
	if (IsWindowVisible(false))
	{
		if (!RFSceneManager::GetManager983()->IsModalExist())
			UpdateFocus983();
		else if (RFSceneManager::Instance()->IsInputBlocked983())
			RFSceneManager::Instance()->SetInputBlocked983(false);
	}
	e.handled = false;
}

// 005fb820
template<class T> void RFFormDesktopBase<T>::Show983(bool show)
{
	if (!m_bSubscribed)
	{
		RegisterTo(GetName983(), RFWnd::EventUpdate, Subscriber(&RFFormDesktopBase<T>::OnUpdateEvent983, this));
		m_bSubscribed = true;
	}
	if (show)
	{
		m_layer = 0;
		if (RFSceneManager::GetManager983()->GetDesktop() != this)
			RFSceneManager::GetManager983()->DeleteDesktop();
		RFSceneManager::GetManager983()->RegisterDesktop(this);
		RFSceneManager::Instance()->m_41 = true;
		RFSceneManager::Instance()->SetInputBlocked983(false);
	}
	ShowWindow(show);
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: janelas filhas, menu de botões, atualização (2º lote, B132)

// 005f8380
void RFLobbyDesktop::UpdateMoney983()
{
	char buf[64];
	MakePriceStrA(buf, Doc()->m_myInfo.stat.i64Pang);
	m_pangText = buf;
	MakePriceStrA(buf, Doc()->m_cookie);
	m_cookieText = buf;
	char won[64];
	MakePriceStrA(won, Doc()->m_cookie * 100);
	m_wonText = won;
}

// 005f8f20
void RFLobbyDesktop::UpdateAdRect983()
{
	RFLabel* label = dynamic_cast<RFLabel*>(GetChild(L"label_ad"));
	if (label)
	{
		if (g_pOneLineBoard983)
		{
			RFRect rc = label->GetScreenRect();
			g_pOneLineBoard983->SetRect(WRect(rc.left, rc.top + 1.5, rc.right - rc.left, rc.bottom - rc.top));
		}
		label->SetLabel(L"");
	}
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B537 (provisorio)

// 005f9090
bool RFLobbyDesktop::IsPopupVisible983()
{
	RFWnd* popup = GetChild(L"panel_popup");
	if (popup)
		return popup->IsWindowVisible(false);
	return false;
}

// 005fa270
void RFLobbyDesktop::ShowChild983(RFWnd* wnd, const wchar_t* name, bool show)
{
	RFWnd* child = wnd->GetChild(name);
	if (child)
		child->ShowWindow(show);
}

// 005fa340
void RFLobbyDesktop::EnableChild983(RFWnd* wnd, const wchar_t* name, bool enable)
{
	RFWnd* child = wnd->GetChild(name);
	if (child)
		child->EnableWindow(enable);
}

// 005fa410
void RFLobbyDesktop::FocusChild983(RFWnd* wnd, const wchar_t* name)
{
	RFWnd* child = wnd->GetChild(name);
	if (child)
		RFSceneManager::GetManager983()->FocusWindow(child);
}

// 005fa900
void RFLobbyDesktop::UpdateAdLabel983()
{
	UpdateAdRect983();
}

// 005fa910
void RFLobbyDesktop::EnableButton983(std::wstring name, bool enable)
{
	EnableChild983(this, name.c_str(), enable);
}

// 005fafb0
void RFLobbyDesktop::ShowPopup983(bool show)
{
	if (show)
	{
		// B566: o SetScreenPos de OnInitialUpdate nao vale (b565-1: y=-760 no init, painel seguiu 1 px abaixo do original);
		// aqui a posicao ja e a final: sobe 1 px uma vez por posicao (original, orig-c116-1/022)
		RFWnd* popup = GetChild(L"panel_popup");
		if (popup)
		{
			static float s_fixedY = -1.0e9f;
			RFVector2 p = popup->GetScreenPos();
			if (p.y != s_fixedY)
			{
				popup->SetScreenPos(p.x, p.y - 1.0f);
				s_fixedY = popup->GetScreenPos().y;
			}
		}
	}
	ShowChild983(this, L"panel_popup", show);
	m_241 = show;
}

// 005fafe0: 1 = os dois grupos de ícones; 9 = só os da direita + "btn_game"; 10/11 = nenhum + "btn_practice" (volta ao normal)
void RFLobbyDesktop::SetButtonGroup983(int mode)
{
	RFButton* button;
	switch (mode)
	{
	case 1:
		EnableChild983(this, L"group_lefticon", true);
		EnableChild983(this, L"group_righticon", true);
	default:
		return;
	case 9:
		EnableChild983(this, L"group_lefticon", false);
		EnableChild983(this, L"group_righticon", true);
		button = dynamic_cast<RFButton*>(GetChild(L"btn_game"));
		break;
	case 10:
	case 11:
		EnableChild983(this, L"group_lefticon", false);
		EnableChild983(this, L"group_righticon", false);
		button = dynamic_cast<RFButton*>(GetChild(L"btn_practice"));
		break;
	}
	if (button)
		button->ResetState983();
}

// 005fb250
void RFLobbyDesktop::ShowSpecialCardEmpty983()
{
	ShowChild983(this, L"wnd_special_card_empty", true);
}

// 005fb260
void RFLobbyDesktop::OnPopupKeyDown983(EventArgs& e)
{
	if (e.m_0c == VK_ESCAPE)
		ShowPopup983(false);
}

// 005fb280
void RFLobbyDesktop::OnPopupClick983(EventArgs& e)
{
	// E292 (L2, orig-c157-2): com o Rare Shop (Fresh "bongdari_shop") aberto o original ignora o clique do Pop-Up
	bool shopOpen = g_pFresh && g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop") != NULL;
	if (!m_bLocked && !shopOpen)
	{
		bool show = !IsPopupVisible983();
		ShowPopup983(show);
		if (show)
			FocusChild983(this, L"panel_popup");
	}
}

// 005fb2d0
void RFLobbyDesktop::OnPopupKillFocus983(EventArgs& e)
{
	RFWnd* popup = GetChild(L"panel_popup");
	RFWnd* button = GetChild(L"btn_popup");
	if (popup && button)
	{
		RFRect rcPopup = popup->GetScreenRect();
		RFRect rcButton = button->GetScreenRect();
		WVector* pos = RFSceneManager::Instance()->GetPos983();
		if (!rcPopup.PtInRect(pos->x, pos->y) && !rcButton.PtInRect(pos->x, pos->y))
			ShowPopup983(false);
	}
}

// 005fbe40
void RFLobbyDesktop::Refresh983()
{
	UpdateMoney983();
	UpdateChannelName983();
	UpdateAdLabel983();
	UpdatePremium983(true);
}

// 005ff4f0
void RFLobbyDesktop::OnShowDesktop983(bool show)
{
	if (show)
	{
		Refresh983();
		if (RFLobbyCharacterSelectWnd::ms_pInstance)
			RFLobbyCharacterSelectWnd::ms_pInstance->Refresh983();
		if (g_lobbyModal983 == 0)
		{
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x77, 0, 0, 0, 0, 0);
		}
		if (RFSceneManager::Instance())
			RFSceneManager::Instance()->SetInputBlocked983(false);
	}
	else
	{
		m_1f8 = 1;
		m_state = 1;
	}
	Show983(show);
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: botões da barra (2º lote, B132)

// 005f74e0: "btn_game" e "btn_talk" — lista de salas
void RFLobbyDesktop::OnGameClick983(EventArgs& e)
{
	if (!m_bLocked)
	{
		RFSceneManager::Instance()->SetInputBlocked983(true);
		FrTaskForm* form = AfxGetTask()->GetWidgetForm983();
		if (form)
			form->Close(true);
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_GameUp();
	}
}

// 005fcb10: EventUpdate desta janela (e.m_pWnd = o tempo do quadro)
void RFLobbyDesktop::OnUpdate983(EventArgs& e)
{
	UpdateSpcardButton983(e.m_time983);
	UpdateMessengerButton983(e.m_time983);
	if (g_lobbyModal983 == 0)
	{
		UpdateShopCamera983();
		UpdateLayer983();
	}
	UpdateInputState983();
	UpdatePremiumFade983(e.m_time983);
}

// 005fcb70: loja
void RFLobbyDesktop::OnShopClick983(EventArgs& e)
{
	if (AfxGetTask()->IsCurrentActor983("shopmain"))
		return;
	switch (g_lobbyModal983)
	{
	case 0:
		if (!m_bLocked)
		{
			((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_ShopUp();
			SetScreen983(2);
			UpdateLobbyActors983();
		}
		break;
	case 1:
		if (m_pAvatarChat)
		{
			((CTaskMainLobbyJp983*)m_pAvatarChat)->OnUnderBar_ShopUp();
			m_layer = 1;
		}
		break;
	}
}

// 005fcbf0: my room
void RFLobbyDesktop::OnMyRoomClick983(EventArgs& e)
{
	if (AfxGetTask()->IsCurrentActor983("realmyroom"))
		return;
	switch (g_lobbyModal983)
	{
	case 0:
		if (!m_bLocked)
		{
			((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_MyRoomUp();
			SetScreen983(5);
			UpdateLobbyActors983();
		}
		break;
	case 1:
		if (m_pAvatarChat)
		{
			((CTaskMainLobbyJp983*)m_pAvatarChat)->OnUnderBar_MyRoomUp();
			m_layer = 1;
		}
		break;
	}
}

// 005f9610
void RFLobbyDesktop::UpdateSpcardButton983(float delta)
{
	if (m_spcardBlink983)
	{
		if (dynamic_cast<RFButton*>(GetChild(L"btn_spcard")))
		{
			if (m_spcardBlinkTime983 >= 1.0f)
			{
				RFButton* button = dynamic_cast<RFButton*>(GetChild(L"btn_spcard"));
				if (button)
					button->SetState(button->GetState983() != 2 ? 2 : 0);
				m_spcardBlinkTime983 = 0.0f;
			}
			else
				m_spcardBlinkTime983 += delta;
		}
	}
}

// 005f9780
void RFLobbyDesktop::UpdateMessengerButton983(float delta)
{
	if (m_bMessengerBlink983)
	{
		if (dynamic_cast<RFButton*>(GetChild(L"btn_messenger")))
		{
			if (m_messengerBlinkTime983 >= 1.0f)
			{
				RFButton* button = dynamic_cast<RFButton*>(GetChild(L"btn_messenger"));
				if (button)
					button->SetState(button->GetState983() != 2 ? 2 : 0);
				m_messengerBlinkTime983 = 0.0f;
			}
			else
				m_messengerBlinkTime983 += delta;
		}
	}
}

// 005fc490
void RFLobbyDesktop::UpdateLayer983()
{
	switch (m_state)
	{
	case 1:
	case 8:
		m_layer = 0;
		return;
	case 9:
		if (g_pF02340_983 && FRESHMGR983() && FRMGR_FORMCOUNT983(FRESHMGR983()) == 0)
		{
			m_layer = 0;
			SetScreen983(1);
			return;
		}
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 10:
	case 11:
	case 12:
		m_layer = 1;
		return;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: canal, premium, entrada, câmera (3º lote, B133)

// 005fa980: "servidor > canal" no "label_channel_name"; mudou o par = aviso do servidor (RFServerNoticeDlg) e botão
// "btn_talk" (servidor tipo 0x10) no lugar do "btn_game"
void RFLobbyDesktop::UpdateChannelName983()
{
	RFLabel* label = dynamic_cast<RFLabel*>(GetChild(L"label_channel_name"));
	if (!label)
		return;
	if (Doc()->m_offlinePlay)
	{
		label->SetLabel(L"OFFLINE PLAY");
		return;
	}
	if (Doc()->m_curChannel.Name[0] == 0)
	{
		label->SetLabel(MultiByteToWide983(Doc()->m_curGameServer.name));
		return;
	}
	std::string server(Doc()->m_curGameServer.name);
	if (m_278 != server || m_294 != std::string(Doc()->m_curChannel.Name))
	{
		m_278 = Doc()->m_curGameServer.name;
		m_294 = Doc()->m_curChannel.Name;
		RFServerNoticeDlg* notice;
		if (!RFLoadingProgressDlg::ms_pInstance && !RFServerNoticeDlg::ms_pInstance && g_lobbyModal983 == 0 &&
			(notice = RFServerNoticeDlg::Create983(this)) != NULL)
		{
			notice->SetNotice983(MakeStr("%s > %s", m_278.c_str(), m_294.c_str()));
			notice->Show983(true);
		}
		bool talk = GetServerType983() == 0x10;
		ShowChild983(this, L"btn_talk", talk);
		ShowChild983(this, L"btn_game", !talk);
	}
	std::string text = MakeStr("%s > %s", m_278.c_str(), m_294.c_str());
	label->SetLabel(MultiByteToWide983(text.c_str()));
}

// 005fae60: botão premium (bit 14 do dwIdentity) e a dica com o tempo que resta do item 0x1a100002
void RFLobbyDesktop::UpdatePremium983(bool fade)
{
	bool premium = (Doc()->m_myInfo.info.dwIdentity >> 14) & 1;
	if (m_247 != premium)
	{
		m_247 = premium;
		if (premium && fade)
			m_bPremiumFade = true;
		ShowChild983(this, L"btn_premium", premium);
	}
	const std::list<sItemInfo>& items = Doc()->GetConstMyItemList();
	std::list<sItemInfo>::const_iterator it;
	for (it = items.begin(); it != items.end(); ++it)
		if (it->typeId == 0x1a100002)
			break;
	if (it == items.end())
		return;
	SYSTEMTIME end = { 0 };
	S5::SecToSysTime983(&end, *(__int64*)((char*)&*it + 0x20)); // 983: +0x20 = fim do prazo (layout do sItemInfo provisório)
	__int64 hours = DOCJP()->GetRemainHours983(&end);
	if (hours != 0)
	{
		__int64 days = hours / 24;
		WORD d = (WORD)days, h = (WORD)(hours - days * 24);
		if (d != 0 || h != 0)
		{
			m_premiumTip = MakeStr(K2L_Compatibility("%d\xc0\xcf %d\xbd\xc3\xb0\xa3"), d, h);
			return;
		}
	}
	m_premiumTip = K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: 1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n");
}

// 005f9150: entrada da RF conforme o travamento (+0x240/+0x242) e o "ocupado" (+0x200: menu aberto, seleção de personagem
// animando ou modal); livre = teclas do lobby
void RFLobbyDesktop::UpdateInputState983()
{
	if (RFSceneManager::GetManager983())
		RFSceneManager::GetManager983()->FreezeInputs(m_242);
	bool locked = m_bLocked;
	if (IsWindowEnabled(false) == locked)
	{
		EnableWindow(!locked);
		if (RFLobbyCharacterSelectAnimWnd::ms_pInstance && RFLobbyCharacterSelectAnimWnd::ms_pInstance->m_bOpen)
			RFLobbyCharacterSelectAnimWnd::ms_pInstance->Select983(2, false);
	}
	m_200 = false;
	if (g_pF02340_983 && FRESHMGR983() && FRMGR_FORMCOUNT983(FRESHMGR983()) != 0)
		m_200 = false;
	else
	{
		RFWnd* popup = GetChild(L"panel_popup");
		if (popup && popup->IsWindowVisible(false))
		{
			// C116.2: o painel nao tem o foco do teclado (OnPopupKeyDown983 nao recebe o ESC): fecha aqui; o ESC segue
			// para o CTaskMain::OnProcess, que abre o Esc Menu (original: fecha o painel e abre o Esc Menu)
			if (g_input->GetDown("ESCAPE", true) == 1)
			{
				ShowPopup983(false);
				C_ASSERT_VC10(offsetof(CTaskMainLobbyJp983, m_bUnderTabOn) == 0x1d4); // D866: era o cru +0x1d4 (anda no cl novo)
				((CTaskMainLobbyJp983*)m_pTaskMain)->m_bUnderTabOn = false; // E151: senao CheckLobbyHotKeys983 trata o ESC como "fechar aba" e nao abre o Esc Menu
				m_200 = false; // o mesmo ESC abre o Esc Menu (orig-c116-1/024)
			}
			else
				m_200 = true;
		}
		else
		{
			if (RFLobbyCharacterSelectAnimWnd::ms_pInstance)
				m_200 = RFLobbyCharacterSelectAnimWnd::ms_pInstance->m_bOpen;
			if (m_200)
				return;
			if (RFSceneManager::GetManager983())
				m_200 = RFSceneManager::GetManager983()->IsModalExist();
		}
	}
	if (!m_200)
		((CTaskMainLobbyJp983*)m_pTaskMain)->CheckLobbyHotKeys983();
}

// 005f7310: roda do mouse na loja (2/5) ou no caddie (3/6, com caddie equipado) = câmera de perto/de longe do "LobbyBg"
void RFLobbyDesktop::UpdateShopCamera983()
{
	if (m_bLocked)
		return;
	bool zoom;
	switch (m_state)
	{
	case 2:
	case 5:
	{
		if (!m_bCursorBusy)
			return;
		float wheel = RFSceneManager::Instance()->GetWheel983();
		if (wheel == 0.0f)
			return;
		zoom = wheel > 0.0f;
		if (m_243 == zoom)
			return;
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x17f, (int)(zoom ? "ShopCamera_z" : "ShopCamera"), 0, 1, 0, 0);
		break;
	}
	case 3:
	case 6:
	{
		if (!Doc()->m_myInfo.userEquip.guidCaddie)
			return;
		if (!m_bOverPet)
			return;
		float wheel = RFSceneManager::Instance()->GetWheel983();
		if (wheel == 0.0f)
			return;
		zoom = wheel > 0.0f;
		if (m_243 == zoom)
			return;
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x17f, (int)(zoom ? "CaddieCamera_z" : "CaddieCamera"), 0, 1, 0, 0);
		break;
	}
	default:
		return;
	}
	m_243 = zoom;
	m_bLocked = true;
}

// 0055fe80 (B163): telas 2, 3 ou 4
bool RFLobbyDesktop::IsGroupA983() const
{
	return m_state == 2 || m_state == 3 || m_state == 4;
}

// 005fbe70: troca a tela do lobby (1 principal, 2–4 loja/caddie/itens, 5–7 my room…): avisa os bonecos que voltam ou saem da
// pose de vitrine, guarda a anterior em +0x1f8 e avisa o "Lobby" (0x1a0)
void RFLobbyDesktop::SetScreen983(int screen)
{
	if (m_state == screen)
		return;
	switch (screen)
	{
	case 1:
		if (!IsGroupA983())
		{
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x1ce, 0, 0, 0, 0, 0);
			AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x1ce, 0, 0, 0, 0, 0);
		}
		else
		{
			if (m_bChanged)
			{
				AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x189, 0, 0, 0, 0, 0);
				AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x85, 0, 0, 0, 0, 0);
			}
			if (m_259)
				AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x1a9, 0, 0, 0, 0, 0);
			AfxGetTask()->GetActor("LobbyMascot") << MsgObject(NULL, 0x1aa, 0, 0, 0, 0, 0);
		}
		break;
	case 2:
	case 3:
	case 4:
		if (m_bChanged)
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x187, 0, 1, 0, 0, 0);
		if (m_259)
			AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x1a7, 0, 0, 0, 0, 0);
		break;
	case 5:
	case 6:
	case 7:
		if (IsGroupA983())
		{
			if (m_bChanged)
			{
				AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x189, 0, 0, 0, 0, 0);
				AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x85, 0, 0, 0, 0, 0);
			}
			if (m_259)
			{
				AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x1a8, 0, 0, 0, 0, 0);
				AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x1a9, 0, 0, 0, 0, 0);
			}
			AfxGetTask()->GetActor("LobbyMascot") << MsgObject(NULL, 0x1aa, 0, 0, 0, 0, 0);
		}
		break;
	}
	m_1f8 = m_state;
	m_state = screen;
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x1a0, 0, 0, 0, 0, 0);
	m_243 = false;
	m_bChanged = false;
	m_259 = false;
	SetButtonGroup983(screen);
}

// 005f8020: a tela mudou = mascote e câmera do "LobbyBg" da tela nova; trava os botões até a câmera chegar
void RFLobbyDesktop::UpdateLobbyActors983()
{
	if (g_lobbyModal983 != 0 || m_1f8 == m_state)
		return;
	std::string next = "";
	switch (m_state)
	{
	case 1:
		AfxGetTask()->GetActor("LobbyMascot") << MsgObject(NULL, 0x1aa, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x17f, (int)"DefaultCamera", (int)next.c_str(), 1, 0, 0);
		break;
	case 2:
	case 5:
		AfxGetTask()->GetActor("LobbyMascot") << MsgObject(NULL, 0x1ab, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x17f, (int)(m_243 ? "ShopCamera_z" : "ShopCamera"), (int)next.c_str(), 1, 0, 0);
		break;
	case 3:
	case 6:
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x17f, (int)"CaddieCamera", (int)next.c_str(), 1, 0, 0);
		break;
	case 4:
	case 7:
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x17f, (int)"ItemCamera", (int)next.c_str(), 1, 0, 0);
		break;
	default:
		return;
	}
	if (!m_bLocked)
		m_bLocked = true;
}

// 005f8d20
void RFLobbyDesktop::OnLayoutChanged(EventArgs& e)
{
	RFWnd::OnLayoutChanged(e);
	RFWnd* area = GetChild(L"wnd_char_select_area");
	if (area)
		area->CenterWindow_Horizontal(NULL);
	area = GetChild(L"wnd_char_select_anim_area");
	if (area)
		area->CenterWindow_Horizontal(NULL);
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: OnInitialUpdate (3º lote, B133)

// 005fd000: assina os 45 eventos (tabela no out/F3.7/s131/RELATORIO.md), acha o CTaskMain/CLobbyMain/ntAvatarChatMain, cria as
// fontes do dinheiro, as janelas filhas da RF (seleção de personagem, menu de botões, balão, carregamento) e esconde o resto
void RFLobbyDesktop::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventFinalize, Subscriber(&RFLobbyDesktop::OnFinalize983, this));
	RegisterTo(L"", RFWnd::EventUpdate, Subscriber(&RFLobbyDesktop::OnUpdate983, this));
	RegisterTo(L"", RFWnd::EventLButtonDown, Subscriber(&RFLobbyDesktop::OnLButtonDown983, this));
	RegisterTo(L"wnd_special_card_empty", RFWnd::EventUpdate, Subscriber(&RFLobbyDesktop::OnSpcardEmptyUpdate983, this));
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFLobbyDesktop::OnKeyDown983, this));
	RegisterTo(L"panel_popup", RFWnd::EventKeyDown, Subscriber(&RFLobbyDesktop::OnPopupKeyDown983, this));
	RegisterTo(L"panel_popup", RFWnd::EventKillFocus, Subscriber(&RFLobbyDesktop::OnPopupKillFocus983, this));
	RegisterTo(L"btn_pang_cookie", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnNothing983, this));
	RegisterTo(L"btn_pang_cookie", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnPangCookieLeave983, this));
	RegisterTo(L"pic_server_name", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnServerNameEnter983, this));
	RegisterTo(L"pic_server_name", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnHideTooltip983, this));
	RegisterTo(L"btn_spcard", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnSpcardEnter983, this));
	RegisterTo(L"btn_spcard", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnSpcardLeave983, this));
	RegisterTo(L"btn_messenger", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnMessengerEnter983, this));
	RegisterTo(L"btn_messenger", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnMessengerLeave983, this));
	RegisterTo(L"btn_myinfo", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnMyInfoEnter983, this));
	RegisterTo(L"btn_myinfo", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnMyInfoLeave983, this));
	RegisterTo(L"btn_option", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnOptionEnter983, this));
	RegisterTo(L"btn_option", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnOptionLeave983, this));
	RegisterTo(L"btn_ad", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnAdEnter983, this));
	RegisterTo(L"btn_ad", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnAdLeave983, this));
	RegisterTo(L"btn_premium", RFWnd::EventMouseEnter, Subscriber(&RFLobbyDesktop::OnPremiumEnter983, this));
	RegisterTo(L"btn_premium", RFWnd::EventMouseLeave, Subscriber(&RFLobbyDesktop::OnPremiumLeave983, this));
	RegisterTo(L"btn_game", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnGameClick983, this));
	RegisterTo(L"btn_talk", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnGameClick983, this));
	RegisterTo(L"btn_practice", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnPracticeClick983, this));
	RegisterTo(L"btn_shop", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnShopClick983, this));
	RegisterTo(L"btn_myroom", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnMyRoomClick983, this));
	RegisterTo(L"btn_cookie", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnCookieClick983, this));
	RegisterTo(L"btn_post", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnPostClick983, this));
	RegisterTo(L"btn_channel", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnChannelClick983, this));
	RegisterTo(L"btn_popup", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnPopupClick983, this));
	RegisterTo(L"btn_exit", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnExitClick983, this));
	RegisterTo(L"btn_spcard", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnSpcardClick983, this));
	RegisterTo(L"btn_messenger", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnMessengerClick983, this));
	RegisterTo(L"btn_myinfo", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnMyInfoClick983, this));
	RegisterTo(L"btn_option", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnOptionClick983, this));
	RegisterTo(L"btn_ad", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnAdClick983, this));
	RegisterTo(L"btn_pang_cookie", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnCookieClick983, this));
	RegisterTo(L"btn_premium", RFButton::EventClicked, Subscriber(&RFLobbyDesktop::OnPremiumClick983, this));
	RegisterTo(L"", RFWnd::EventDraw, Subscriber(&RFLobbyDesktop::OnPremiumDraw983, this));
	RegisterTo(L"wnd_pang", RFWnd::EventDraw, Subscriber(&RFLobbyDesktop::OnPangDraw983, this));
	RegisterTo(L"wnd_cookie", RFWnd::EventDraw, Subscriber(&RFLobbyDesktop::OnCookieDraw983, this));
	RegisterTo(L"wnd_won", RFWnd::EventDraw, Subscriber(&RFLobbyDesktop::OnWonDraw983, this));
	RegisterTo(L"label_ad", RFWnd::EventDraw, Subscriber(&RFLobbyDesktop::OnAdDraw983, this));

	if (CLoginInfo::Instance()->ProvType() == 4)
	{
		RFPicture* pic = dynamic_cast<RFPicture*>(GetChild(L"pic_pang_bar_frame_tree"));
		if (pic)
			pic->SetPicture(L"pang_bar_frame_han.tga");
	}

	m_pTaskMain = (CTaskMain*)AfxGetTask()->GetMainActor();
	m_pLobbyMain = (CLobbyMain*)AfxGetTask()->GetActor("Lobby");
	m_pAvatarChat = (ntAvatarChatMain*)AfxGetTask()->GetActor("AvatarChat");
	std::string error;
	bool ok;
	switch (g_lobbyModal983)
	{
	case 0:
		ok = m_pTaskMain && m_pLobbyMain;
		error = "<CTaskMain> or <CLobbyMain> load error.";
		if (!ok)
			g_bQuit = true;
		break;
	case 1:
		ok = m_pTaskMain && m_pAvatarChat;
		error = "<CTaskMain> or <ntAvatarChatMain> load error.";
		if (!ok)
			g_bQuit = true;
		break;
	}

	OpenLobbyActors983();
	ShowChild983(this, L"panel_popup", false);
	ShowChild983(this, L"wnd_special_card_empty", false);
	ShowChild983(this, L"pic_pang_bar_treecash", false);
	ShowChild983(this, L"pic_pang_bar_frame_tree", false);
	ShowChild983(this, L"btn_premium", false);
	RFTextArea* bubble = reinterpret_cast<RFTextArea*>(GetChild(L"textarea_bubble"));
	if (bubble)
	{
		bubble->SetText(MultiByteToWide983(K2L_Compatibility("\xbb\xe7\xbf\xeb\xc1\xdf\xc0\xce \xc8\xbf\xb0\xfa\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")));
		RFWnd* empty = GetChild(L"wnd_special_card_empty");
		if (empty)
			bubble->CenterWindow(empty);
	}

	static char* s_font[] = { "[font_wind.jpg" };
	WTITLEFONT font;
	memset(&font, 0, sizeof(font));
	font.filename = s_font;
	font.numPages = 1;
	font.fontw = 16;
	font.fonth = 16;
	font.texw = 128;
	font.texh = 64;
	font.pCharSet = "1234567890ym%-.?/:,";
	m_pPangFont = g_resrcmng->GetTitleFont();
	m_pPangFont->Create(&font);
	m_pCookieFont = g_resrcmng->GetTitleFont();
	m_pCookieFont->Create(&font);
	m_pWonFont = g_resrcmng->GetTitleFont();
	m_pWonFont->Create(&font);

	if (!RFLobbyCharacterSelectWnd::ms_pInstance)
	{
		RFWnd* area = GetChild(L"wnd_char_select_area");
		if (area)
		{
			RFLobbyCharacterSelectWnd* wnd = RFLobbyCharacterSelectWnd::Create983(area);
			if (wnd)
				wnd->ShowWindow983(true);
		}
	}
	if (!RFLobbyCharacterSelectAnimWnd::ms_pInstance)
	{
		RFWnd* area = GetChild(L"wnd_char_select_anim_area");
		if (area)
		{
			RFLobbyCharacterSelectAnimWnd* wnd = RFLobbyCharacterSelectAnimWnd::Create983(area);
			if (wnd)
				wnd->ShowWindow983(true);
			area->SetHitTestVisible983(false);
		}
	}
	if (!RFLobbyPopupMenuWnd::ms_pInstance)
	{
		RFWnd* popup = GetChild(L"panel_popup");
		if (popup)
		{
			float y = 0.0f;
			RFWnd* underbar = GetChild(L"wnd_underbar");
			if (underbar)
				y = underbar->GetScreenPos().y;
			RFLobbyPopupMenuWnd* menu = RFLobbyPopupMenuWnd::Create983(popup);
			if (menu)
			{
				popup->SetSize(menu->GetSize());
				menu->ShowWindow983(true);
				menu->SetPos(0.0f, 0.0f);
			}
			RFSize size = popup->GetSize();
			float w = g_pMainView983 ? *(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x19c)) : g_fRFScreenH983;
			popup->SetScreenPos(w - size.width, y - size.height - 1.0f); // C278/C289: original fica 1 px acima (medido por foto)
			jp983_rastro("B544 underbar=%p upos=%.2f,%.2f usz=%.2fx%.2f menuSz=%.2fx%.2f", underbar, underbar ? underbar->GetPos().x : 0.f, underbar ? underbar->GetPos().y : 0.f, underbar ? underbar->GetSize().width : 0.f, underbar ? underbar->GetSize().height : 0.f, menu ? menu->GetSize().width : 0.f, menu ? menu->GetSize().height : 0.f);
			jp983_rastro("B537 popup w=%.2f y=%.2f size=%.2fx%.2f pos=%.2f,%.2f", w, y, size.width, size.height, popup->GetScreenPos().x, popup->GetScreenPos().y);
			popup->ShowWindow(false);
		}
	}
	if (!RFLobbyMessengerBubble::ms_pInstance)
	{
		RFWnd* box = GetChild(L"panel_notice_box");
		if (box)
		{
			box->SetPreDraw(false);
			RFLobbyMessengerBubble* b = RFLobbyMessengerBubble::Create983(box);
			if (b)
				b->Hide983();
			box->ShowWindow(true);
			box->SetHitTestVisible983(false);
		}
	}
	UpdateButtons983();
	LoadPremiumImages983();
	UpdatePremium983(false);
	RFWnd* wnd = GetChild(L"panel_popup");
	if (wnd)
		wnd->SetPreDraw(false);
	wnd = GetChild(L"pic_pang_bar_frame_tree");
	if (wnd)
		wnd->SetPreDraw(false);
	if (g_lobbyModal983 == 0 && !RFLoadingProgressDlg::ms_pInstance)
	{
		RFLoadingProgressDlg* loading = RFLoadingProgressDlg::Create983(this);
		if (loading)
			loading->Start983();
	}
	ShowChild983(this, L"wnd_char_flip_anim", false);
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop: tratadores restantes (4º lote, B134)

// D865: os crus Doc()+0x10d80/0x10d81/0x10d82/0x11038/0x111d4 (VC10) viraram membros do CSharedDoc

// 005f98f0: clique fora da área da seleção de personagem aberta fecha a seleção
void RFLobbyDesktop::OnLButtonDown983(EventArgs& e)
{
	if (RFLobbyCharacterSelectAnimWnd::ms_pInstance && RFLobbyCharacterSelectAnimWnd::ms_pInstance->m_bOpen)
	{
		RFWnd* area = GetChild(L"wnd_char_select_anim_area");
		if (area)
		{
			MouseEventArgs983& me = static_cast<MouseEventArgs983&>(e);
			RFRect rc = area->GetScreenRect();
			if (!rc.PtInRect(me.m_x983, me.m_y983))
				RFLobbyCharacterSelectAnimWnd::ms_pInstance->Select983(2, true);
		}
	}
}

// 005f9a00: o aviso "sem cartão especial" some depois de 3 s
void RFLobbyDesktop::OnSpcardEmptyUpdate983(EventArgs& e)
{
	RFWnd* wnd = GetChild(L"wnd_special_card_empty");
	if (wnd && wnd->IsWindowVisible(false))
	{
		if ((m_204 += e.m_time983) > 3.0f)
		{
			m_204 = 0;
			wnd->ShowWindow(false);
		}
	}
}

// 005fb440: dica do nome do servidor ("접속서버:" servidor, "접속채널:" canal)
void RFLobbyDesktop::OnServerNameEnter983(EventArgs& e)
{
	if (RFTooltipHelper::Instance983())
	{
		std::string server = MakeStr("<font color=\"#ffaa00\">%s</font> %s",
			K2L_Compatibility("\xc1\xa2\xbc\xd3\xbc\xad\xb9\xf6:"), m_278.c_str());
		std::string channel = m_294.empty() ? "" : MakeStr("\n<font color=\"#ffaa00\">%s</font> %s",
			K2L_Compatibility("\xc1\xa2\xbc\xd3\xc3\xa4\xb3\xce:"), m_294.c_str());
		std::string text = server + channel;
		RFTooltipHelper::Instance983()->Show983((int)e.m_pWnd, text.c_str(), 1, 0xb);
	}
}

// 005f9b00: correio (volta o ícone normal do "btn_post" e abre os presentes)
void RFLobbyDesktop::OnPostClick983(EventArgs& e)
{
	if (!m_bLocked)
	{
		if (m_25a)
		{
			RFButton* button = dynamic_cast<RFButton*>(GetChild(L"btn_post"));
			if (button)
			{
				button->ChangeNormalButtonImage(L"post_n.tga");
				button->ChangeOverButtonImage(L"post_o.tga");
				button->ChangeDownButtonImage(L"post_d.tga");
				m_25a = false;
			}
		}
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_GiftUp();
	}
}

// 005f8470: sair (lobby: CTaskMain; conversa de avatar: não sai com a loja aberta, senão a janela de saída)
void RFLobbyDesktop::OnExitClick983(EventArgs& e)
{
	if (m_bLocked)
		return;
	if (m_pAvatarChat)
	{
		FrWndManager* pScreen;
		if (g_pF02340_983 && (pScreen = FRESHMGR983()) != NULL && FRMGR_FORMCOUNT983(pScreen) != 0)
			return;
		if (RFSceneManager::GetManager983() && RFSceneManager::GetManager983()->IsModalExist())
			return;
	}
	switch (g_lobbyModal983)
	{
	case 0:
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_ExitUp();
		break;
	case 1:
		if (::Doc()->m_stateTrade)
			CChatMsg::Instance()->AddChatMsg(K2L_Compatibility("\xbb\xf3\xc1\xa1 \xc0\xcc\xbf\xeb\xc1\xdf\xbf\xa1\xb4\xc2 "
				"\xb4\xeb\xc8\xad\xb9\xe6\xbf\xa1\xbc\xad \xb3\xaa\xb0\xa5\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0xffff00, true, false);
		else if (CQuitWindow::Instance())
			CQuitWindow::Instance()->Process();
		break;
	}
}

// 005f9c40: cartão especial (o CTaskMain abre a lista junto do botão)
void RFLobbyDesktop::OnSpcardClick983(EventArgs& e)
{
	if (e.m_pWnd)
	{
		RFRect rc = e.m_pWnd->GetScreenRect();
		WRect rect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_SpecialCardUp983(rect);
	}
	OnSpcardMsg983(0xca);
}

// 005fcc70: mensageiro
void RFLobbyDesktop::OnMessengerClick983(EventArgs& e)
{
	ShowMessengerBubble983(false);
	((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_MessengerUp983();
	OnMessengerMsg983(0xcd);
}

// 005f7610: pang, alinhado à direita da janela
void RFLobbyDesktop::OnPangDraw983(EventArgs& e)
{
	if (!CProjectG::Instance()->HidePrivacy() && e.m_pWnd)
	{
		RFRect rc = e.m_pWnd->GetScreenRect();
		float w = m_pPangFont->GetTextWidthOld983(g_pMainView983, m_pangText.c_str());
		float h = (float)m_pPangFont->GetFontHeight();
		float x = rc.GetWidth() - w + rc.left;
		float y = rc.GetHeight() - h + rc.top;
		m_pPangFont->Print(g_pMainView983, x - 8.0, y - 1.0, m_pangText.c_str(), 0, 0xfff0bc10, NULL);
	}
}

// 005f7710: cookie
void RFLobbyDesktop::OnCookieDraw983(EventArgs& e)
{
	if (!CProjectG::Instance()->HidePrivacy() && e.m_pWnd)
	{
		RFRect rc = e.m_pWnd->GetScreenRect();
		float w = m_pCookieFont->GetTextWidthOld983(g_pMainView983, m_cookieText.c_str());
		float h = (float)m_pCookieFont->GetFontHeight();
		float x = rc.GetWidth() - w + rc.left;
		float y = rc.GetHeight() - h + rc.top;
		m_pCookieFont->Print(g_pMainView983, x - 3.0, y - 1.0, m_cookieText.c_str(), 0, 0xfff0bc10, NULL);
	}
}

// 005fa7c0: botões conforme o servidor (0x10 = conversa) e o tipo do lobby
void RFLobbyDesktop::UpdateButtons983()
{
	bool talk = GetServerType983() == 0x10;
	ShowChild983(this, L"btn_talk", talk);
	ShowChild983(this, L"btn_game", !talk);
	switch (g_lobbyModal983)
	{
	case 0:
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x77, 1, 0, 0, 0, 0);
		break;
	case 1:
		EnableChild983(this, L"btn_game", false);
		EnableChild983(this, L"btn_talk", false);
		EnableChild983(this, L"btn_practice", false);
		EnableChild983(this, L"btn_shop", false);
		EnableChild983(this, L"btn_myroom", true);
		EnableChild983(this, L"btn_cookie", true);
		EnableChild983(this, L"btn_post", true);
		EnableChild983(this, L"btn_channel", false);
		break;
	}
	if (RFLobbyCharacterSelectWnd::ms_pInstance)
		RFLobbyCharacterSelectWnd::ms_pInstance->Refresh983();
	if (RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Reset983();
}

// 005f92f0: ícone do "btn_spcard" (0xcb premium aceso, 0xca cartão aceso se o CCardManager tiver alarme, senão normal)
void RFLobbyDesktop::OnSpcardMsg983(int msg)
{
	switch (msg)
	{
	case 0xcb:
		{
			RFButton* button = dynamic_cast<RFButton*>(GetChild(L"btn_spcard"));
			if (button)
			{
				button->ChangeNormalButtonImage(L"premium_btn_on_1.tga");
				button->ChangeOverButtonImage(L"premium_btn_on_2.tga");
				button->ChangeDownButtonImage(L"premium_btn_on_p.tga");
			}
			m_spcardBlink983 = 1;
		}
		break;
	case 0xca:
		if (CCardManager::Instance() && CCardManager::Instance()->ShowSPCardAlarm())
		{
			RFButton* button = dynamic_cast<RFButton*>(GetChild(L"btn_spcard"));
			if (button)
			{
				button->ChangeNormalButtonImage(L"spcard_btn_on_1.tga");
				button->ChangeOverButtonImage(L"spcard_btn_on_2.tga");
				button->ChangeDownButtonImage(L"spcard_btn_on_p.tga");
			}
			m_spcardBlink983 = 2;
			break;
		}
	default:
		{
			RFButton* button = dynamic_cast<RFButton*>(GetChild(L"btn_spcard"));
			if (button)
			{
				button->ChangeNormalButtonImage(L"spcard_btn_n.tga");
				button->ChangeOverButtonImage(L"spcard_btn_o.tga");
				button->ChangeDownButtonImage(L"spcard_btn_p.tga");
			}
			m_spcardBlink983 = 0;
		}
		break;
	}
}

// 005fc520: ícone do "btn_messenger" (e do "btn_post") conforme o aviso; mensagem nova abre o balão (0xea63/0xea64)
void RFLobbyDesktop::OnMessengerMsg983(int msg)
{
	RFButton* button;
	switch (msg)
	{
	case 0xcc:
		return;
	case 0xcf:
		button = dynamic_cast<RFButton*>(GetChild(L"btn_messenger"));
		if (button)
		{
			button->ChangeNormalButtonImage(L"gift_btn_on_1.tga");
			button->ChangeOverButtonImage(L"gift_btn_on_2.tga");
			button->ChangeDownButtonImage(L"gift_btn_on_p.tga");
			m_messengerBlinkTime983 = 0;
			m_bMessengerBlink983 = true;
		}
		return;
	case 0xcd:
	case 0xd1:
		if (::Doc()->m_bMessengerAlarm || ::Doc()->m_bMessengerInit)
		{
			button = dynamic_cast<RFButton*>(GetChild(L"btn_messenger"));
			if (button)
			{
				button->ChangeNormalButtonImage(L"pin_btn_on_1.tga");
				button->ChangeOverButtonImage(L"pin_btn_on_2.tga");
				button->ChangeDownButtonImage(L"pin_btn_on_p.tga");
			}
			m_messengerBlinkTime983 = 0;
			m_bMessengerBlink983 = true;
			ShowMessengerBubble983(true, GetGameString(0xea63));
			return;
		}
		if (::Doc()->m_toppageFeature == 3)
		{
			button = GetChild983<RFButton>(L"btn_messenger");
			if (button)
			{
				button->ChangeNormalButtonImage(L"pin_btn_n.tga");
				button->ChangeOverButtonImage(L"pin_btn_o.tga");
				button->ChangeDownButtonImage(L"pin_btn_p.tga");
				button->ResetState983();
			}
			m_bMessengerBlink983 = false;
			return;
		}
	case 0xce:
		if (::Doc()->m_bRefreshCamera)
		{
			button = GetChild983<RFButton>(L"btn_messenger");
			if (button)
			{
				button->ChangeNormalButtonImage(L"gift_btn_on_1.tga");
				button->ChangeOverButtonImage(L"gift_btn_on_2.tga");
				button->ChangeDownButtonImage(L"gift_btn_on_p.tga");
			}
			button = GetChild983<RFButton>(L"btn_post");
			if (button)
			{
				button->ChangeNormalButtonImage(L"n_post_btn_n.png");
				button->ChangeOverButtonImage(L"n_post_btn_o.png");
				button->ChangeDownButtonImage(L"n_post_btn_d.png");
				m_25a = true;
			}
			m_messengerBlinkTime983 = 0;
			m_bMessengerBlink983 = true;
			ShowMessengerBubble983(true, GetGameString(0xea64));
			return;
		}
	default:
		button = dynamic_cast<RFButton*>(GetChild(L"btn_messenger"));
		if (button)
		{
			button->ChangeNormalButtonImage(L"pin_btn_n.tga");
			button->ChangeOverButtonImage(L"pin_btn_o.tga");
			button->ChangeDownButtonImage(L"pin_btn_p.tga");
			button->ResetState983();
		}
		m_bMessengerBlink983 = false;
		return;
	}
}

// 005f6410
int RFLobbyDesktop::GetCharType983()
{
	if (RFLobbyCharacterSelectWnd::ms_pInstance)
		return RFLobbyCharacterSelectWnd::ms_pInstance->m_charType983;
	return 0xf;
}

// 005f6430
unsigned long RFLobbyDesktop::GetCharTypeId983()
{
	int type = GetCharType983();
	if (type != 0xf)
		return type | 0x4000000;
	return 0;
}

// 005f6450
void RFLobbyDesktop::ResetCharSelect983()
{
	if (RFLobbyCharacterSelectAnimWnd::ms_pInstance)
		RFLobbyCharacterSelectAnimWnd::ms_pInstance->SetChar983(0xf);
}

// 005f8af0
int RFLobbyDesktop::FindScreen983(const std::string& name)
{
	std::map<std::string, int>::iterator it = s_screens983.find(name);
	if (it != s_screens983.end())
		return it->second;
	return 0;
}

// 005f8e50
void RFLobbyDesktop::FocusPopup983()
{
	RFWnd* popup = GetChild(L"panel_popup");
	if (popup && RFSceneManager::GetManager983())
		RFSceneManager::GetManager983()->FocusWindow(popup);
}

// 005fb620
void RFLobbyDesktop::OnMenuGacha983(EventArgs& e)
{
	switch (g_lobbyModal983)
	{
	case 0:
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_GachaUp983();
		break;
	case 1:
		((CTaskMainLobbyJp983*)m_pAvatarChat)->OnUnderBar_GuildUp();
		break;
	}
	ShowPopup983(false);
}

// 005fb670
void RFLobbyDesktop::OnMenuBongdari983(EventArgs& e)
{
	((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_BongdariShopUp();
	ShowPopup983(false);
}

// 005fb690
void RFLobbyDesktop::OnMenuPointShop983(EventArgs& e)
{
	((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_PointShopUp983();
	ShowPopup983(false);
}

// 005fb6b0: missão do dia (lobby: CLobbyMain; conversa de avatar: slot +0x68 do ntAvatarChatMain)
void RFLobbyDesktop::OnMenuOneDayQuest983(EventArgs& e)
{
	switch (g_lobbyModal983)
	{
	case 0:
		if (m_pLobbyMain)
			((CLobbyMainDesktopJp983*)m_pLobbyMain)->OnToppage_OneDayQuestDown983();
		break;
	case 1:
		if (m_pAvatarChat)
			((ntAvatarChatMainJp983*)m_pAvatarChat)->OnToppage_OneDayQuestDown983();
		break;
	}
	ShowPopup983(false);
}

// 005fb700
void RFLobbyDesktop::OnMenuNotice983(EventArgs& e)
{
	if (m_pTaskMain)
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_NoticeUp();
	ShowPopup983(false);
}

// 005fb730
void RFLobbyDesktop::OnMenuRanking983(EventArgs& e)
{
	if (m_pTaskMain)
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_RankingUp();
	ShowPopup983(false);
}

// 005fb760
void RFLobbyDesktop::OnMenuAchievement983(EventArgs& e)
{
	if (m_pTaskMain)
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_AchievementUp983();
	ShowPopup983(false);
}

// 005fb780
void RFLobbyDesktop::OnMenuClub983(EventArgs& e)
{
	switch (g_lobbyModal983)
	{
	case 0:
		if (m_pTaskMain)
			((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_ClubUp983();
		break;
	case 1:
		if (m_pAvatarChat)
			((CTaskMainLobbyJp983*)m_pAvatarChat)->OnUnderBar_ClubUp983();
		break;
	}
	ShowPopup983(false);
}

// 005fb7e0
void RFLobbyDesktop::OnMenuCombine983(EventArgs& e)
{
	if (m_pTaskMain)
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_CombineUp983();
	ShowPopup983(false);
}

// 005fb800
void RFLobbyDesktop::OnMenuGacha2_983(EventArgs& e)
{
	if (m_pTaskMain)
		((CTaskMainLobbyJp983*)m_pTaskMain)->OnUnderBar_GachaUp983();
	ShowPopup983(false);
}

// 005fb190
void RFLobbyDesktop::ShowMessengerBubble983(bool show, const std::string& text)
{
	if (RFLobbyMessengerBubble::Instance983())
	{
		if (show)
			RFLobbyMessengerBubble::Instance983()->Show983(text.c_str());
		else
			RFLobbyMessengerBubble::Instance983()->Hide983();
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyMessengerBubble (B133)

RFWnd* RFLobbyMessengerBubble::s_pParent983;

// 005f69c0
RFLobbyMessengerBubble::~RFLobbyMessengerBubble()
{
	ms_pInstance = NULL;
}

// 005f9cb0
RFLobbyMessengerBubble* RFLobbyMessengerBubble::Create983(RFWnd* parent)
{
	RFLobbyMessengerBubble* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyMessengerBubble>(L"lobby_messenger_bubble_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyMessengerBubble>(L"lobby_messenger_bubble_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFLobbyMessengerBubble>(L"lobby_messenger_bubble_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005f9df0
void RFLobbyMessengerBubble::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventUpdate, Subscriber(&RFLobbyMessengerBubble::OnUpdate983, this));
	SetPreDraw(false);
}

// 005f7f10
void RFLobbyMessengerBubble::Hide983()
{
	ShowWindow983(false);
	m_time983 = 0;
}

// 005f7f30
void RFLobbyMessengerBubble::OnUpdate983(EventArgs& e)
{
	if (IsWindowVisible(false) && m_time983 > 0.0f)
	{
		m_time983 -= e.m_time983;
		if (m_time983 <= 0.0f)
			Hide983();
	}
}

// 005f9f10
void RFLobbyMessengerBubble::Show983(const std::string& text)
{
	Hide983();
	RFTextArea* area = reinterpret_cast<RFTextArea*>(GetChild(L"notice_box_text"));
	if (area)
	{
		area->SetText(MultiByteToWide983(text.c_str()));
		RFWnd* box = GetChild(L"wnd_notice_box");
		if (box)
		{
			RFRect rc = box->GetRect();
			box->SetRect(RFRect(rc.left, rc.right, rc.top, area->GetSize().height + 14.0));
			SetRect(box->GetRect());
			RFWnd* parent = s_pParent983;
			parent->SetRect(box->GetRect());
		}
	}
	ShowWindow983(true);
	PlayAnimation(L"_Opening_");
	m_time983 = 5.0f;
}

template class RFFormDesktopBase<RFWndEx>;
