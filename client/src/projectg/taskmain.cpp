#include "minatl.h"
#include "taskmain.h"
#include "commonutil.h"
#include "soundmanager.h"
#include "taskmanager.h"
#include "jp/itemsearchdlg.h"
#include "packet.h"
#include "tooltip.h"
#include "inputmanager.h"
#include "wresrcmng.h"
#include "wview.h"
#include "frwnd.h"
#include "frgroupbox.h"
#include "frform.h"
#include "frbutton.h"
#include "frarea.h"
#include "fresh.h"
#include "rtti.h"
#include "skinnedtheme.h"
#include "../shared/localize.h"
#include "tikimagicboxdlg.h"
#include "logoutdlg.h"
#include "serverdlg.h"
#include "loginrsdlg.h"
#include <wininet.h>
#include "projectg.h"
#include "tutorialtask.h"
#include "browser.h"
#include "frdesktop.h"
#include "uccmanager.h"
#include "jp/grandprixlimitdlg.h"
#include "levelupitemdlg.h"
#include "standinginline.h"
#include "binstr.h"
#include "itembuffactor.h"
#include "frviewer.h"
#include "frtabbutton.h"
#include "actor.h"
#include "s5/utilities.h"
#include "s5/classicserver/classicserver.h"
int GetServerType983(); // 00b2eae0 (classicserver.cpp)
#include "../shared/s5/sharedutilities.h"
#include "mousecursor.h"
#include "avatar_task.h"
#include "ghosttask.h"
#include "avatar_avatar.h"
#include "messengerdlg.h"
#include "userinfoform.h"
#include "user_info.h"
inline FrUserInfoForm* USERINFODLG()
{
	CUserInfo* pInfo = CUserInfo::Instance();
	if (!pInfo)
		return NULL;
	return pInfo->GetDlg();
}
#include "clientsetting.h"
#include "onelinereqdlg.h"
#include "onelineboard.h"
#include "quitwindow.h"
#include "noticedlg.h"
#include "kbdmanualdlg.h"
#include "frienddlg.h"
#include "giftdlg.h"
#include "buddymanager.h"
#include "cardsystem.h"
#include "carddlg.h"
#include "lobbymain.h"
#include "playerdlg.h"
#include "golftask.h"
#include "treasure.h"
#include "lobbytask.h"
#include "jp/taskform.h" // FrTaskForm (CTask::GetWidgetForm983; MakeRealMyRoom do 983)
#include "jp/shareddocjp.h" // PKT (frente C): IsControlServerService983 (OnUnderBar_GiftUp do 983)
#include "jp/webpageconnector.h" // 983 (s106): CWebPageConnector::RequestOpen983 (botoes combine/gacha/clube da barra)
#include "equipdlg.h" // PKT (frente C): FrEquipDlg::GetMyItemSlot (OnItemEquipResult983)
#include "post_senddlg.h" // PKT (frente C): CPost_SendDlg "write_post" (OnPostSendDlgResult983, pacote 0x145)
#include "post_recivedlg.h" // PKT (frente C): CPost_ReciveDlg "postbox"
#include "packets/postpackets.h" // PKT (frente C): sMailIncludeItem983 (0x37)
#include "giftalarmdlg.h"
#include "scenemanager.h"
#include "shareddoc.h"
#include "optiondlg.h"
#include "realmyroom_authoritydlg.h"
#include "networksystem.h"
#include "initdelegate.h"
#include "caddieinfodlg.h"



#include "shoptask.h"

// only its RTTI is needed here (realmyroomtask.h has its own view of CItemStorage)
class CRealMyRoomTask
{
public:
	static const WRTTI m_RTTI;
};

// the view of CItemStorage this unit needs: IsVisible is inline in the original header (taskmain.obj has
// its own copy) and itemstorage.h keeps it out of line
class CItemStorage : public WSingleton<CItemStorage>
{
public:
	bool IsVisible() const { return m_bVisible; }

private:
	char m_pad[4]; // A727: FrCmdTarget apos a base (m_visible 0x2c no VC10, 0x28 no cl novo)
	bool m_bVisible;
};

#include "logininfo.h"
#include "birthdayeventdlg.h"

// FrItemStorageAccessPanel: only its in-class constants, which the original emitted into this
// unit's .rdata (itemstorage.h does not declare them)
class FrItemStorageAccessPanel
{
public:
	static const int MAX_PASSWORD_LENGTH = 4;
	static const int MIN_PASSWORD_LENGTH = 4;
	static const int COUNT_NUMBER_SLOT = 10;
};

#include "bongdarishopdlg.h"
#include "offlinetradedlg.h"
#include "jp/extraresource.h"
#include "jp/lobbyjp.h" // 983 0065d913: CLobbyState983 (RFLobbyDesktop, 00e78ca8) recebe as mensagens do HandleMsgCommon
#include "jp/refresh/rfscenemanager.h" // 983 0065b7d0: SetInputBlocked983
#include "jp/refresh/rfcharmasteryaboutdlg.h" // 983 0x356: RFCharMasteryMainDlg
#include "jp/gamestringmanager.h" // GetGameString 00540a10 (msg 0x27)
#include "jp/grandprixdocument.h" // FindRoomItem983 (msg 0x4f)
#include "jp/grandprix983.h"
#include "jp/attendancedlg.h" // B96: FrAttendanceDlg (m_pAttendanceDlg983, resultado 00651270)
#include "jp/cardcomposedlg.h" // B93: FrCardComposeDlg (m_pCardComposeDlg983, msgs 0x324-0x326; s111)
#include "jp/docofs983.h" // DOCPTR983
#include "jp/pointshopdlg.h" // B106: FrPointShopDlg / FrPointShopMessageDlg (msgs 0x2e0..0x2e2; s114)
#include "jp/alchemympbreakdlg.h" // B95: FrAlchemyMPBreakDlg (m_pAlchemyDlg983, msgs 0x2e3/0x2e5/0x2e6; s111)

extern bool g_bQuit;

// 983: 004dbb40 devolve os avisos baixados pelo CExtraResource (+0x98); o include/jp/extraresource.h (frente B) ainda nao tem o
// metodo publico (pedido A -> B, sessao 87). Visao sem campo, so empresta o `this`.
class CExtraResourceNoticesA983 : public CExtraResource
{
public:
	const std::vector<std::string>& GetNotices983() const { return m_notices; }
};

// The views of FrRecycleDlg / FrScratchDlg this unit needs (CreateForm<T>: size, RTTI, constructor).
// recycledlg.h needs types that only recycledlg.cpp has and scratchdlg.h redefines sBonusBall of bongdarishopdlg.h.
class FrRecycleDlg : public FrForm
{
	DECLARE_OBJECT(FrRecycleDlg)

public:
	FrRecycleDlg();
	void SelectLevel983(int level); // 983 008e3600 (recycledlg.h; msg 0x339)

private:
	char m_pad[0x810 - sizeof(FrForm)]; // JP 983: sizeof 0x810 (construtor 008e15f0; recycledlg.cpp, s102)
};

class FrScratchDlg : public FrForm
{
	DECLARE_OBJECT(FrScratchDlg)

public:
	FrScratchDlg();

private:
	char m_pad[0x2d0 - sizeof(FrForm)]; // JP 983: sizeof 0x2d0 (construtor 008ec050)
};

namespace _util
{
	bool IsAvatarChatServer(int);
	const char* GetAvatarServerServiceMessage(int);
	const char* GetNormalServerServiceMessage(int);
}

// unused, but the original emitted g_HUGE/g_EPSILON into .rdata
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

extern WView* g_view;

// original .bss+0: set while the BGM is playing
static int s_bgmPlaying;

// original .data: [0x440] actor name ptrs, then _MsgMap, then the BGM list and 2 empty strings
static const char* s_actorClassName = "CGatewayActor";
static const char* s_gatewayActorName = "Gateway";

IMPLEMENT_ACTOR(CTaskMain, IActor)

BEGIN_FRESH_MSGMAP(CTaskMain, FrCmdTarget)
// CTaskMain has no entries of its own: its handlers are registered in the message map of lobbymain.cpp
END_FRESH_MSGMAP()

static const char* s_randomBGMList[2] = { "frog.mp3", "daydream.mp3" };

// 983: os dois botoes de alarme do messenger sairam (static provisorio do taskmain.h); sem definicao viravam dado de stub no exe
FrButton* CTaskMain::m_pMessengerAlarmBtn = NULL;
FrButton* CTaskMain::m_pMessengerGiftAlarmBtn = NULL;

bool CTaskMain::IsActiveRecycleDlg()
{
	if (m_pRecycleDlg)
		return true;
	return false;
}

bool CTaskMain::IsActiveScratchDlg()
{
	if (m_pScratchDlg)
		return true;
	return false;
}

void CTaskMain::OnPreLoadInit()
{
	m_pUnderBarBack = NULL;
	m_pUnderBarFront = NULL;
	m_pUnderBarShop = NULL;
	m_pUnderBarMyRoom = NULL;
	m_pUnderBarItemStorage = NULL;
	m_pUnderBarGift = NULL;
	m_pUnderBarCookie = NULL;
	m_pUnderBarGame = NULL;
	m_pUnderBarServer = NULL;
	m_pUnderBarExit = NULL;
	m_pOverBarSpecialCard = NULL;
	m_pOverBarMessenger = NULL;
	m_pOverBarMyInfo = NULL;
	m_pOverBarOption = NULL;
	m_pUnderBarMagicBox = NULL;
	m_pUnderBarBongdariShop = NULL;
	m_pUnderBarRanking = NULL;
	m_pUnderBarGuild = NULL;
	m_pUnderBarNotice = NULL;
	m_pUnderBarMaket = NULL;
	m_pUnderBarScratch = NULL;
	m_pUnderBarTabBtn = NULL;
	m_pOverBarMascot = NULL;
	m_pUnderBarTabToolTip = NULL;
	m_bUnderTabOn = false;
	m_pMaketDlg = NULL;
	InitOverBar();
	InitBongdariShop();
	InitMessenger();
	InitControls();
}

void CTaskMain::OnInit()
{
	m_pUnderBarBack = NULL;
	m_pUnderBarFront = NULL;
	m_pUnderBarShop = NULL;
	m_pUnderBarMyRoom = NULL;
	m_pUnderBarItemStorage = NULL;
	m_pUnderBarGift = NULL;
	m_pUnderBarCookie = NULL;
	m_pUnderBarGame = NULL;
	m_pUnderBarServer = NULL;
	m_pUnderBarExit = NULL;
	m_pOverBarSpecialCard = NULL;
	m_pOverBarMessenger = NULL;
	m_pOverBarMyInfo = NULL;
	m_pOverBarOption = NULL;
	m_pUnderBarMagicBox = NULL;
	m_pUnderBarBongdariShop = NULL;
	m_pUnderBarRanking = NULL;
	m_pUnderBarGuild = NULL;
	m_pUnderBarNotice = NULL;
	m_pUnderBarMaket = NULL;
	m_pUnderBarScratch = NULL;
	// 983 (00658260): zera 0x20..0x78 (com os dois botões novos); a aba e o mascote passaram para o InitUnderBar, e o
	// m_pMaketDlg e os diálogos novos para o InitControls; sem o m_pMissionDlg do 645
	m_pUnderBarCombine = NULL;
	m_pUnderBarPointShop = NULL;
	InitUnderBar();
	InitOverBar();
	InitBongdariShop();
	InitMessenger();
	InitControls();
	m_pRMREnterSetting = NULL;
}

void CTaskMain::OnLoad()
{
	static char* s_fontFile[] = { "[font_wind.jpg" };
	if (0) // HACK: emission order: the original compiles OnLoad one wave after the bar handlers
		OnUnderBarInit(0);
	CalcUIOffset(false); // 983 0065593b; 800x600 fixo do 645 punha a barra em x-140 com tela maior
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_fontFile;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);
}

void CTaskMain::OnProcess(float delta)
{
	if (0) // HACK: keeps the literal keeper of MakeRealMyRoom emitted
		((std::list<sRoomInfo>*)0)->clear();
	if (0) // HACK: keeps the inline naked OnOverBar_PangCookieOwnerDraw emitted (nakedify)
		((CTaskMain*)0)->OnOverBar_PangCookieOwnerDraw(*(int*)0);
	if (0) // HACK: keeps the literal keeper of OnOverBar_PangCookieOwnerDraw emitted
		((std::list<sItemAttachCard>*)0)->clear();
	if (0) // HACK: keeps the literal keeper of OnMessengerBubbleOwnerDraw emitted
		((std::list<sNoteInfo>*)0)->clear();
	ProcessOverBar();
	// C130.1 / 983 (0065d440): F12/ESC so aqui FORA do lobby RF (00e78ca8 == 0); no lobby tratam-se no RFLobbyDesktop
	if (!CLobbyState983::ms_pInstance)
	{
	if (g_input->GetDown("FUNC_12", true))
	{
		if (m_pKbdManualDlg)
		{
			m_pKbdManualDlg->Close(true);
		}
		else
		{
			m_pKbdManualDlg = CreateForm<FrKbdManualDlg>(g_pFresh->GetManager(),
				this, "kbd_manual", NULL);
			if (m_pKbdManualDlg)
			{
				m_pKbdManualDlg->Open(
					(FRESH_PFN_RESULT)&CTaskMain::OnKbdManualDlgResult, 1);
				m_pKbdManualDlg->EnableDrag(false);
			}
		}
	}
	if (g_input->GetDown("ESCAPE", true) == 1 && // C759: 983 0065c61d; 3 = ESC ja gasto por form
		!g_pFresh->GetManager()->HasEscKeyWindow())
	{
		// C116.2 / 983 (0065c580+0x164): ESC com o painel do Pop-Up (utab) aberto e o botao visivel chama o
		// OnUnderBar_TabBtnUp (65a100, fecha o painel) e sai SEM abrir o Esc Menu
		if (CMessengerInfo::Instance()->IsVisible() && !IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
			CMessengerInfo::Instance()->Close(false); // 983 0065c68a
		else if (m_bUnderTabOn && m_pUnderBarTabBtn && m_pUnderBarTabBtn->IsVisible())
			OnUnderBar_TabBtnUp();
		// D243: a faixa do Arin (FrSubEquipBar aberta por obar.char) fecha com ESC sem abrir o Esc Menu (orig-c116-1/029)
		else if (m_pSubEquipBar)
		{
			m_pSubEquipBar->Close(false);
		}
		else if (!IS_EXACTKINDOF(CGolfTask, AfxGetTask()) &&
			!IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
			OnUnderBar_BackUp();
	}
	}
	if (m_fKbdManualTime > -0.1f)
	{
		m_fKbdManualTime += delta;
		float limit = IS_KINDOF(CLobbyTask, AfxGetTask()) ? 1.0f : 0.5f;
		if (m_fKbdManualTime > limit)
		{
			if (!m_pServerDlg)
			{
				if (!m_pWaitDlg)
				{
					m_pWaitDlg = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "wait", NULL);
					m_pWaitDlg->SetMessage(
						K2L_Compatibility("\xbc\xad\xb9\xf6\xc0\xc7 \xc0\xc0\xb4\xe4\xc0\xbb "
						"\xb1\xe2\xb4\xd9\xb8\xae\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
						false);
					m_pWaitDlg->Open(
						(FRESH_PFN_RESULT)&CTaskMain::OnWaitDlgResult, 0x800); // 983 0065d53f
					FrButton* pCancel = DYNAMIC_CAST(FrButton,
						m_pWaitDlg->FindChildByName("cancel"));
					if (pCancel)
						pCancel->Enable(false);
					return;
				}
				if (!(m_fKbdManualTime > 15.0f))
					return;
				FrButton* pCancel = DYNAMIC_CAST(FrButton,
					m_pWaitDlg->FindChildByName("cancel"));
				if (pCancel)
					pCancel->Enable(true);
			}
			m_fKbdManualTime = -10.0f;
		}
	}
}


void CTaskMain::OnDestroy()
{
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
	if (g_audio)
		g_audio->StopAllSound(); // 983 00650e90
}

void CTaskMain::OnPreserveBack(const char*)
{
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

void CTaskMain::OnRestorePreserved()
{
	OnLoad();
	OnInit();
}

void CTaskMain::OnUnderBar_ShopInit(int param)
{
	m_pUnderBarShop = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarShop)
	{
		// 983 006559d0: a loja e o ator "shopmain" do lobby, nao a CShopTask do 645; sem o REALMYROOMVISITOR (nao ha no 983)
		if (AfxGetTask()->IsCurrentActor983("shopmain"))
			m_pUnderBarShop->Enable(false);
		m_pUnderBarShop->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarShop->EnableHover(true, 0.0f);
	}
}

// 983 (00be87c0 e vizinhas usam [g_view]+0x19c/+0x1a0): largura/altura reais da tela, 800x600 sem view.
static inline float ViewWidth983() { return g_view ? g_view->GetWidth() : 800.0f; }
static inline float ViewHeight983() { return g_view ? g_view->GetHeight() : 600.0f; }

// 983 (006519a0): a UI fica na tela real mesmo com bFixed (o 645 fixava 800x600), entao m_offsetX/m_offsetY
// saem 0 e os Init da barra (SetCenterOffset/SetLeftOffset) deixam as janelas na posicao do layout.
void CTaskMain::CalcUIOffset(bool bFixed)
{
	if (!bFixed)
	{
		m_screenWidth = g_view->GetWidth();
		m_screenHeight = g_view->GetHeight();
	}
	else
	{
		m_screenWidth = ViewWidth983();
		m_screenHeight = ViewHeight983();
	}
	m_offsetX = (m_screenWidth - ViewWidth983()) * 0.5;
	m_offsetY = m_screenHeight - ViewHeight983();
}

// 983 (00651a70): copia o retangulo e soma dx/dy (s111: IGUAL; a forma com o construtor de 4 floats dava 82%)
void CTaskMain::SetCenterOffset(FrWnd* pWnd, float dx, float dy)
{
	WRect r = pWnd->GetRect();
	r.x += dx;
	r.y += dy;
	pWnd->SetRect(r);
}

// 983 (00651ad0): ancora pela direita da tela real (o asm do 645 usava 800 e o retangulo em +0x20)
void CTaskMain::SetLeftOffset(FrWnd* pWnd, float dx)
{
	const WRect& rc = pWnd->GetRect();
	WRect r(dx - (ViewWidth983() - rc.x), rc.y, rc.w, rc.h);
	pWnd->SetRect(r);
}

void CTaskMain::InitMessenger()
{
	m_pMessengerBubbleEdit = NULL;
	m_pMessengerBubbleBmp[0] = NULL;
	m_pMessengerBubbleBmp[1] = NULL;
	m_pMessengerBubbleBmp[2] = NULL;
	m_pMessengerAlarmBtn = NULL;
	m_pMessengerGiftAlarmBtn = NULL;
	m_pTreasureAlarm = NULL;
	m_pMessengerBtn = NULL;
	m_pMessengerBubble = NULL;
}

void CTaskMain::InitBongdariShop()
{
	m_bBongdariShop0 = false;
	m_bBongdariShop1 = false;
	Doc()->m_bBongdariShopOpen = false; // 983 006559b0
}

void CTaskMain::InitControls()
{
	m_pOverBar = NULL;
	m_pUnderTab = NULL;
	m_pUnderBar = NULL;
	m_pGiftDlg = NULL;
	m_pOptionDlg = NULL;
	m_pKbdManualDlg = NULL;
	m_pServerDlg = NULL;
	m_pLoginRsDlg = NULL;
	m_pRecycleDlg = NULL;
	m_pTikiMagicBoxDlg = NULL;
	m_pScratchDlg = NULL;
	m_pOnelineReqDlg = NULL;
	m_pWaitDlg = NULL;
	m_pNoticeDlg = NULL;
	m_pDlg150 = NULL;
	m_pQuitDlg = NULL;
	m_pSpecialCardDlg = NULL;
	m_pSubEquipBar = NULL;
	// 983 (00650f10): diálogos novos de 0x18c..0x1ac e o bloco de 0xd0
	m_pCardComposeDlg983 = NULL;
	m_pAttendanceDlg983 = NULL;
	m_pBongdariShop983 = NULL;
	m_pEquipDlg983 = NULL;
	for (int i = 0; i < 10; i++)
		m_myItemSlot983[i] = 0;
	m_fKbdManualTime = -10.0f;
	m_pPointShopDlg983 = NULL;
	m_pMaketDlg = NULL;
	m_pPostReciveDlg983 = NULL;
	m_pPostSendDlg983 = NULL;
	m_pAlchemyDlg983 = NULL;
}

void CTaskMain::InitUnderBar()
{
	m_pUnderBarTabBtn = NULL;
	m_pOverBarMascot = NULL;
	m_pUnderBarTabToolTip = NULL;
	m_bUnderTabOn = false;
	// 983 (00651000): sem o m_pMaketDlg (InitControls)
}

// 983 (00651b40): so esconde; sem o deslocamento do 645
void CTaskMain::OnUnderBarInit(int param)
{
	m_pUnderBar = DYNAMIC_CAST(FrGroupBox, (FrWnd*)param);
	if (m_pUnderBar)
		m_pUnderBar->SetVisible(false);
}

// 983 (00651bc0): so esconde; sem o deslocamento do 645
void CTaskMain::OnUnderTab_Init(int param)
{
	m_pUnderTab = DYNAMIC_CAST(FrGroupBox, (FrWnd*)param);
	if (m_pUnderTab)
		m_pUnderTab->SetVisible(false);
}

// 983 (00651c40): so esconde (sem o tema do S5 do 645)
void CTaskMain::OnOverBar_Init(int param)
{
	m_pOverBar = DYNAMIC_CAST(FrGroupBox, (FrWnd*)param);
	if (m_pOverBar)
		m_pOverBar->SetVisible(false);
}

void CTaskMain::OnUnderBar_BackInit(int param)
{
	m_pUnderBarBack = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarBack)
	{
		m_pUnderBarBack->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarBack->EnableHover(true, 0.0f);
	}
}

// JP 983 (0065c0d0; PKT lote 31): o botao "voltar" da barra so trata FAMILY, TOPPAGE, SERVERLIST e AVATARCHAT_MAIN e nunca
// manda o 0xc1 (visitante) do 645; ROOMLIST/GAMEROOM*/SHOPMAIN/RealMyRoom/TUTORIAL_MAIN/UCCROOM/REALMYROOMVISITOR sairam
// (no 983 a loja, o my room e o tutorial sao widgets da tarefa: em TOPPAGE o "voltar" fecha o widget).
// 0065bf70: o widget da tarefa corrente e "shopmain", "realmyroom" ou "tutorialmain"
static bool IsBackClosesWidget983()
{
	FrTaskForm* pWidget = AfxGetTask()->GetWidgetForm983();
	if (pWidget == NULL)
		return false;
	std::string name = pWidget->GetWindowName983();
	return name == "shopmain" || name == "realmyroom" || name == "tutorialmain";
}

void CTaskMain::OnUnderBar_BackUp()
{
	CSharedDoc::Instance()->m_rmrMode = 2;
	if (g_pFresh->IsCurrentLayout("FAMILY"))
	{
		CSharedDoc::Instance()->m_gameMode = 0;
		goto lobbyTop;
	}
	if (g_pFresh->IsCurrentLayout("TOPPAGE"))
	{
		int state = DOCJP()->GetState983();
		if (state == 0)
			goto quit;
		if (state != 8 && IsBackClosesWidget983())
			AfxGetTask()->CloseWidget983(); // 009b77d0 (CTask, pedido C53.1 aplicado pela A)
		goto done;
	}
	if (g_pFresh->IsCurrentLayout("SERVERLIST"))
	{
quit:
		// 983: resultado 00657200 (cria o "attendance_dlg" e manda 0x16e)
		m_pQuitDlg =
			CreateForm<FrLogoutDlg>(g_pFresh->GetManager(), this, "quit", NULL);
		m_pQuitDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnQuitDlgResult, 1);
		goto done;
	}
	if (!g_pFresh->IsCurrentLayout("AVATARCHAT_MAIN"))
		goto done;
	// 983 le o byte +0x10b28 (= m_bReturnLobbyNotice do 983; o nosso CSharedDoc ainda o tem em +0x10b27, pedido C -> A)
	if (CSharedDoc::Instance()->m_bReturnLobbyNotice)
	{
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		SendRoomExitPacket();
		m_waitedLayout = "TOPPAGE";
		goto done;
	}
lobbyTop:
	CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
	CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
done:
	OnBarHoverOff(0);
}



void CTaskMain::OnUnderBar_FrontInit(int param)
{
	m_pUnderBarFront = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarFront)
	{
		m_pUnderBarFront->EnableHover(true, 0.0f);
		m_pUnderBarFront->Enable(false);
	}
}

void CTaskMain::OnUnderBar_FrontUp()
{
	OnBarHoverOff(0);
}

// PKT (frente C, lote 43): 983 0065f9b0. Manda 0x140 (atualizar a loja) e o aviso 1 ao ator principal; a loja e a widget
// "CShopWidget" do lobby (nao ha CShopTask no 983): da praca volta ao CLobbyTask (CGolfDoc) e pede a widget com a mensagem
// 0x88 (so do 983); no lobby pergunta 0xe5 ao "Lobby" e manda a 0x88; no tutorial passa a 0x88 pelo HandleMsgCommon. O 645
// trocava para a CShopTask a partir do lobby, praca, tutorial, GHOST e my room.
void CTaskMain::OnUnderBar_ShopUp()
{
	WSendPacket packet((enumClientPacket)0x140);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	if (!AfxGetTask()->IsCurrentActor983("shopmain"))
	{
		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		{
			CProjectG::Instance()->FromGolfToLobbySettings();
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88 /*so do 983*/,
				(int)"CShopWidget", (int)"TOPPAGE", 0, 0);
			ntAvatarManager::m_pInstance->SetActive(false);
		}
		else if (IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
		{
			int tab, item;
			SendMsg(NULL, "Lobby", M983(0xc3), (int)&tab, (int)&item, 0, 0);
			SendMsg(NULL, "Lobby", 0x88 /*so do 983*/, (int)"CShopWidget",
				(int)"TOPPAGE", 0, 0);
		}
		else if (AfxGetTask()->IsCurrentActor983("tutorialmain"))
		{
			HandleMsgCommon(MsgObject(NULL, 0x88 /*so do 983*/, (int)"CShopWidget",
				(int)"TOPPAGE", 0, 0, 0));
		}
	}
	OnBarHoverOff(0);
}

void CTaskMain::OnUnderBar_MyRoomInit(int param)
{
	m_pUnderBarMyRoom = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarMyRoom)
	{
		// 983 00651da0: so som e hover
		m_pUnderBarMyRoom->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarMyRoom->EnableHover(true, 0.0f);
	}
}

void CTaskMain::OnUnderBar_MyRoomUp()
{
	// JP 983 (0065c290; PKT lote 21): sem as travas de conteudo, sem a troca para o CRealMyRoomTask (que o 983 nao tem)
	// e sem o 0xc1; o MakeRealMyRoom (0065b8a0) leva ao my room do CLobbyTask
	CSharedDoc::Instance()->m_rmrMode = 1;
	CSharedDoc::Instance()->m_replayTargetUID =
		CSharedDoc::Instance()->m_myInfo.info.dwUID;
	MakeRealMyRoom();
	OnBarHoverOff(0);
}


void CTaskMain::OnUnderBar_ItemStorageInit(int param)
{
	m_pUnderBarItemStorage = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarItemStorage)
	{
		m_pUnderBarItemStorage->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarItemStorage->EnableHover(true, 0.0f);
		m_bGiftUp = false;
		if (IsLocalContent(S4_REAL_MYROOM) &&
			g_pFresh->IsCurrentLayout("REALMYROOMVISITOR"))
			m_pUnderBarItemStorage->Enable(false);
	}
}

void CTaskMain::OnUnderBar_ItemStorageUp()
{
	if (IsLocalContent(S4_ITEM_STORAGE))
	{
		CSharedDoc::Instance()->m_rmrMode = 4;
		CSharedDoc::Instance()->m_replayTargetUID =
			CSharedDoc::Instance()->m_myInfo.info.dwUID;
		MakeRealMyRoom();
		OnBarHoverOff(0);
	}
}

// 983 (00651e30): "ubar.achievement"; o campo e o do botao do armazem (+0x30)
void CTaskMain::OnUnderBar_AchievementInit983(int param)
{
	m_pUnderBarItemStorage = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarItemStorage)
	{
		m_pUnderBarItemStorage->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarItemStorage->EnableHover(true, 0.0f);
	}
}

// 983 (006582f0, pedido C40.3): janela de conquistas; online pede ao servidor (0x157 com o UID), fora dele avisa o "Gateway"
void CTaskMain::OnUnderBar_AchievementUp983()
{
	if (OnlinePlay())
	{
		WSendPacket pkt((enumClientPacket)0x157);
		pkt.Encode4(CSharedDoc::Instance()->m_myInfo.info.dwUID);
		pkt.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	}
	else
	{
		MsgObject msg(NULL, 0x753a, 7, 0, 0, 0, 0);
		AfxGetTask()->GetActor(s_gatewayActorName) << msg;
	}
	OnBarHoverOff(0);
}

void CTaskMain::OnUnderBar_GiftInit(int param)
{
	m_pUnderBarGift = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarGift)
	{
		m_pUnderBarGift->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarGift->EnableHover(true, 0.0f);
		m_bGiftUp = false; // 983 00651eb0: sem o Enable(false) do 645 em REALMYROOMVISITOR
	}
}

// 983 (00658420, C-PKT lote 25): sem my room e sem o 0x95 do 645 - o botao de presente abre o correio ("postbox"). Com o
// servico 0x40000 em manutencao so avisa (e nao chama o OnBarHoverOff).
void CTaskMain::OnUnderBar_GiftUp()
{
	if (DOCJP()->IsControlServerService983(0x40000))
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xbf\xec\xc6\xed\xc7\xd4 \xb1\xe2\xb4\xc9\xc0\xc7 \xba\xce\xba\xd0 "
				"\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9." /*983 00d1518c*/),
			0, 0, 0, 0);
		return;
	}
	if (m_pPostReciveDlg983 == NULL)
	{
		m_pPostReciveDlg983 = CreateForm<CPost_ReciveDlg>(g_pFresh->GetManager(),
			this, "postbox", NULL);
		m_pPostReciveDlg983->Open(
			(FRESH_PFN_RESULT)&CTaskMain::OnPostReciveDlgResult983, 0x81);
	}
	OnBarHoverOff(0);
}


void CTaskMain::OnUnderBar_CookieInit(int param)
{
	m_pUnderBarCookie = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarCookie)
	{
		m_pUnderBarCookie->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarCookie->EnableHover(true, 0.0f);
	}
}

// 983 (006584e0): pagina de pagamento do JP (id do jogador e o KID do login, CSharedDoc+0x190) no navegador do 983, tipo 0xc
// (pacote 0xa1), com WS_VSCROLL e a tela em 800x600; o aviso do consentimento dos pais passa pelo K2L. Sem o caso do provType
// sem pagina: o 983 abre o navegador assim mesmo (endereco vazio).
void CTaskMain::OnUnderBar_CookieUp()
{
	if (CSharedDoc::Instance()->m_needParentAgree)
	{
		this << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("14\xbc\xbc \xb9\xcc\xb8\xb8\xc0\xce \xba\xd0\xc0\xba\n"
				 "\xc8\xb8\xbf\xf8\xb0\xa1\xc0\xd4(1) \xb9\xd7 \xb0\xe1\xc1\xa6(2)"
				 "\xbf\xa1 \xb4\xeb\xc7\xd1\n\xba\xce\xb8\xf0\xb4\xd4\xc0\xc7 "
				 "\xbd\xc2\xc0\xce\xc0\xcc \xc0\xd6\xbe\xee\xbe\xdf "
				 "\xc7\xd5\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
	}
	else
	{
		COption::Instance()->vApplyLobbyScreenSize();
		char url[0x400] = { 0 };
		switch (CSharedDoc::Instance()->m_provType)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 7:
		case 10:
		case 0xb:
		case 0xc:
		case 0xd:
			sprintf(url, "https://www3.pangya.jp/payment/login.aspx?ID=%s&KID=%s",
				CSharedDoc::Instance()->m_myInfo.info.sID, CSharedDoc::Instance()->m_paymentKID983.c_str());
			break;
		}
		CBrowser::Instance()->SetStyle983(WS_VSCROLL);
		if (COption::Instance())
			COption::Instance()->SetScreenSize983(800, 600);
		CBrowser::Instance()->OpenWeb983(url, true, 0xc);
	}
	if (IsLocalContent(S4_MATCHING_SYSTEM))
	{
		WSendPacket pkt((enumClientPacket)0xc1);
		pkt.Encode1(2);
		pkt.Send(TO_GAME);
	}
	OnBarHoverOff(0);
}

void CTaskMain::OnUnderBar_GameInit(int param)
{
	m_pUnderBarGame = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarGame)
	{
		m_pUnderBarGame->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarGame->EnableHover(true, 0.0f);
		m_pUnderBarGame->Enable(true);
		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()) ||
			IS_EXACTKINDOF(ntAvatarChatTask,
				CTaskManager::Instance()->GetPreservedTask()) ||
			IS_EXACTKINDOF(CGhostTask, AfxGetTask()) ||
			IS_EXACTKINDOF(CGhostTask,
				CTaskManager::Instance()->GetPreservedTask()))
			m_pUnderBarGame->Enable(false);
	}
}

// 006586c0 (Start Game do lobby RF): com o bit 11 de m_serverProperty abre o seletor Multiplay/Grand Prix
void CTaskMain::OnUnderBar_GameUp()
{
	if (!IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		if (CSharedDoc::Instance()->m_serverProperty & 0x800)
			CreateForm<FrRoomListSelector>(g_pFresh->GetManager(), NULL, "roomlistselector")->Open(NULL, 1);
		else if (!CSharedDoc::Instance()->m_bReturnLobbyNotice)
		{
			WSendPacket pkt((enumClientPacket)0x81);
			pkt.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
		else
		{
			if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
				CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
					false);
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE",
				0, 0, 0);
		}
	}
	OnBarHoverOff(0);
}

void CTaskMain::OnUnderBar_ServerInit(int param)
{
	m_pUnderBarServer = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarServer)
	{
		m_pUnderBarServer->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarServer->EnableHover(true, 0.0f);
		if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()) ||
			IS_EXACTKINDOF(ntAvatarChatTask,
				CTaskManager::Instance()->GetPreservedTask()) ||
			AfxGetTask()->IsCurrentActor983("shopmain")) // 983 00655cd9: na loja nao troca de servidor
			m_pUnderBarServer->Enable(false);
		else
			m_pUnderBarServer->Enable(true);
	}
}

void CTaskMain::OnUnderBar_ServerUp()
{
	if (!IS_KINDOF(ntAvatarChatTask, AfxGetTask()) &&
		!IS_EXACTKINDOF(ntAvatarChatTask,
			CTaskManager::Instance()->GetPreservedTask()))
	{
		// 983 00658950: ao ator principal da tarefa (form do My Room e loja tratam 0x75); o 645 mandava ao "RealMyRoom"
		AfxGetTask()->SendMsgToMainActor(MsgObject(this, M983(0x68), 0, 0, 0, 0, 0));
		CloseAllDialogs();
		WSendPacket pkt((enumClientPacket)0x43);
		pkt.Send(TO_GAME);
		// 983 00658890: só cria se fechado; Open sem posição e sem SetIconRect (no lobby RF a barra é NULL)
		if (!m_pServerDlg)
		{
			m_pServerDlg = CreateForm<FrServerDlg>(g_pFresh->GetManager(),
				this, "server", NULL);
			m_pServerDlg->Open(
				(FRESH_PFN_RESULT)&CTaskMain::OnServerDlgResult, 0x11);
		}
	}
	OnBarHoverOff(0);
}

void CTaskMain::OnUnderBar_ExitInit(int param)
{
	m_pUnderBarExit = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarExit)
	{
		m_pUnderBarExit->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarExit->EnableHover(true, 0.0f);
	}
}

void CTaskMain::OnUnderBar_ExitUp()
{
	CloseAllDialogs();
	if (g_pFresh->IsCurrentLayout("AVATARCHAT_MAIN"))
	{
		if (CQuitWindow::Instance())
			CQuitWindow::Instance()->Process();
	}
	else
	{
		m_pQuitDlg =
			CreateForm<FrLogoutDlg>(g_pFresh->GetManager(), this, "quit", NULL);
		m_pQuitDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnQuitDlgResult, 0x11);
		if (m_pUnderBarExit)
			m_pQuitDlg->SetIconRect(m_pUnderBarExit->GetRect());
		// 983 (00658a20): layouts GAMEROOM* nao existem mais; dentro da sala vale o estado de CSharedDoc+0x11638
		unsigned long roomState = Doc()->m_lobbyState983;
		if ((roomState >= 3 && roomState <= 7) ||
			(roomState == 8 && Doc()->m_roomListKind983 <= 1))
			m_pQuitDlg->DisableServerSelect();
		if (!g_pFresh->IsCurrentLayout("TOPPAGE"))
			m_pQuitDlg->MoveCursor("server");
		else
			m_pQuitDlg->MoveCursor("logout");
	}
	OnBarHoverOff(0);
}

void CTaskMain::OnUnderBar_TabToolTipInit(int param)
{
	m_pUnderBarTabToolTip = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pUnderBarTabToolTip)
		m_pUnderBarTabToolTip->SetVisible(
			COption::Instance()->gGetUnderExtBtn() == 0);
}

void CTaskMain::OnUnderBar_TabBtnInit(int param)
{
	m_pUnderBarTabBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarTabBtn)
	{
		m_pUnderBarTabBtn->SetPushDelay(0.0f);
		if (COption::Instance()->gGetUnderExtBtn() == 1)
			m_pUnderBarTabBtn->EnableBlink(false);
	}
}

void CTaskMain::OnUnderBar_TabBtnUp()
{
	if (!m_bUnderTabOn)
		TurnOnUnderTabBar();
	else
		TurnOffUnderTabBar();
	if (m_pUnderBarTabToolTip && m_pUnderBarTabToolTip->IsVisible())
	{
		COption::Instance()->gSetUnderExtBtn(1);
		m_pUnderBarTabToolTip->SetVisible(false);
		m_pUnderBarTabBtn->EnableBlink(false);
	}
	OnBarHoverOff(0);
}

void CTaskMain::TurnOnUnderTabBar()
{
	if (!m_bUnderTabOn)
	{
		m_bUnderTabOn = true;
		m_pUnderTab->SetVisible(true);
		m_pUnderBarMagicBox->Enable(IsLocalContent(S3_RECYCLE));
		m_pUnderBarScratch->Enable(IsLocalContent(S3_SCRATCH));
		m_pUnderBarRanking->Enable(!IsLocalCountry(0x10));
		WRect r(m_pUnderBarTabBtn->GetRect().x,
			m_pUnderBarTabBtn->GetRect().y - 53.0f,
			m_pUnderBarTabBtn->GetRect().w, m_pUnderBarTabBtn->GetRect().h);
		m_pUnderBarTabBtn->SetRect(r);
		std::string name;
		m_pUnderBarTabBtn->GetWindowName(name);
		if (stricmp(name.c_str(), "tabbtn_b") == 0)
		{
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_bd_n",
				FrButton::NORMAL);
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_bd_o",
				FrButton::OVER);
		}
		else
		{
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_d_n",
				FrButton::NORMAL);
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_d_o", FrButton::OVER);
		}
	}
}



void CTaskMain::TurnOffUnderTabBar()
{
	if (0) // HACK: keeps the literal keeper of TurnOnUnderTabBar emitted
		((std::list<COption::sDisplayMode>*)0)->clear();
	if (m_bUnderTabOn)
	{
		m_bUnderTabOn = false;
		m_pUnderTab->SetVisible(false);
		WRect r = m_pUnderBarTabBtn->GetRect();
		r.y += 53.0f;
		m_pUnderBarTabBtn->SetRect(r);
		std::string name;
		m_pUnderBarTabBtn->GetWindowName(name);
		if (stricmp(name.c_str(), "tabbtn_b") == 0)
		{
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_bu_n",
				FrButton::NORMAL);
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_bu_o",
				FrButton::OVER);
		}
		else
		{
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_u_n",
				FrButton::NORMAL);
			m_pUnderBarTabBtn->SetButtonImg("underbar_tab_u_o", FrButton::OVER);
		}
	}
}

void CTaskMain::OnUnderBar_MagicBoxInit(int param)
{
	m_pUnderBarMagicBox = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarMagicBox)
	{
		m_pUnderBarMagicBox->EnableHover(true, 0.0f);
		// 983 (00652190): sem o r.x += 74 do 645 (underbar.xml do 983: magicbox = 285, passo 45)
	}
}

// 983 (00652210): "utab.combine"
void CTaskMain::OnUnderBar_CombineInit983(int param)
{
	m_pUnderBarCombine = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarCombine)
		m_pUnderBarCombine->EnableHover(true, 0.0f);
}

// 983 (00652290): "utab.gacha" grava no lugar do botao da raspadinha (+0x70; o 983 nao tem "utab.scratch")
void CTaskMain::OnUnderBar_GachaInit983(int param)
{
	m_pUnderBarScratch = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarScratch)
		m_pUnderBarScratch->EnableHover(true, 0.0f);
}

// 983 (00652520): "utab.point_shop" (+0x78)
void CTaskMain::OnUnderBar_PointShopInit983(int param)
{
	m_pUnderBarPointShop = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarPointShop)
		m_pUnderBarPointShop->EnableHover(true, 0.0f);
}

// 983 (00652600): "utab.card_compose" grava no lugar do botao do mercado (+0x6c; o 983 nao tem "utab.maket")
void CTaskMain::OnUnderBar_CardComposeInit983(int param)
{
	m_pUnderBarMaket = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarMaket)
		m_pUnderBarMaket->EnableHover(true, 0.0f);
}

// 983 (00659540): a abertura da "recycledlg" saiu do botao para uma funcao propria (tambem chamada pela msg 0x338)
void CTaskMain::OpenRecycleDlg983()
{
	if (IsLocalContent((localContentType_t)5) && !m_pRecycleDlg)
	{
		m_pRecycleDlg = CreateForm<FrRecycleDlg>(g_pFresh->GetManager(),
			this, "recycledlg", NULL);
		m_pRecycleDlg->Open(
			(FRESH_PFN_RESULT)&CTaskMain::OnRecycleDlgResult, 0xc1);
		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
			ntAvatarManager::m_pInstance->SetActive(false);
	}
}

// 983 (0065a160): servico 0x8000 com mascara de 64 bits, textos pelo K2L_Compatibility e a abertura no OpenRecycleDlg983
void CTaskMain::OnUnderBar_MagicBoxUp()
{
	if (DOCJP()->IsControlServerService983(0x8000))
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xb8\xb6\xb9\xfd\xbb\xf3\xc0\xda \xb0\xfc\xb7\xc3 "
								   "\xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9." /*983 00d17068*/),
			0, 0, 0, 0);
		return;
	}
	if (CSharedDoc::Instance()->m_myInfo.info.dwFlagBlock & 0x40)
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\\c0xffff0000\\c \xc4\xab\xb5\xe5\xc1\xa4\xba\xb8 "
								   "\xc0\xcc\xbb\xf3\xc0\xb8\xb7\xce "
								   "\xb8\xb6\xb9\xfd\xbb\xf3\xc0\xda\\c0xff000000\\c\xb8\xa6 "
								   "\xc0\xcc\xbf\xeb\xc7\xd2 \xbc\xf6 "
								   "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9." /*983 00d17018*/),
			0, 0, 0, 0);
		return;
	}
	OpenRecycleDlg983();
	OnBarHoverOff(0);
}


void CTaskMain::OnUnderBar_TikiMagicBoxInit(int)
{
}

void CTaskMain::OnUnderBar_TikiMagicBoxUp()
{
	if (IsLocalContent((localContentType_t)0x79) && !m_pTikiMagicBoxDlg)
	{
		m_pTikiMagicBoxDlg = CreateForm<FrTikiMagicBoxDlg>(
			g_pFresh->GetManager(), this, "tikimagicbox", NULL);
		m_pTikiMagicBoxDlg->Open(
			(FRESH_PFN_RESULT)&CTaskMain::OnTikiMagicBoxDlgResult, 0xc1);
		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
			ntAvatarManager::m_pInstance->SetActive(false);
	}
	OnBarHoverOff(0);
}
void CTaskMain::OnUnderBar_ScratchInit(int param)
{
	m_pUnderBarScratch = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarScratch)
	{
		m_pUnderBarScratch->EnableHover(true, 0.0f);
		WRect r = m_pUnderBarScratch->GetRect();
		r.x += 74.0f;
		m_pUnderBarScratch->SetRect(r);
	}
}

void CTaskMain::OnUnderBar_ScratchUp()
{
	if (CSharedDoc::Instance()->IsControlServerService(0x4000))
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xbd\xba\xc5\xa9\xb7\xa1\xc4\xa1 \xb0\xfc\xb7\xc3 "
				 "\xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		return;
	}
	if (IsLocalContent((localContentType_t)2))
	{
		if (CSharedDoc::Instance()->m_specialShotMark == 2)
		{
			FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			WRect rc = form->GetRect();
			form->SetRect(rc);
			form->SetTimeLimit(12.0f);
			form->SetMessage(
				K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xd1 \xc1\xbe\xb7\xf9\xc0\xc7 "
				"\xbe\xc6\xc0\xcc\xc5\xdb \xba\xb8\xc0\xaf "
				"\xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce "
				"\xc7\xd8\xb4\xe7 \xc4\xc1\xc5\xd9\xc3\xf7\xb8\xa6 "
				"\xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			form->Open(NULL, 0);
		}
		else if (!m_pScratchDlg)
		{
			m_pScratchDlg = CreateForm<FrScratchDlg>(g_pFresh->GetManager(),
				this, "scratchdlg", NULL);
			m_pScratchDlg->Open(
				(FRESH_PFN_RESULT)&CTaskMain::OnScratchDlgResult, 0xc1);
			if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
				ntAvatarManager::m_pInstance->SetActive(false);
		}
	}
	OnBarHoverOff(0);
}


void CTaskMain::OnUnderBar_BongdariShopInit(int param)
{
	m_pUnderBarBongdariShop = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarBongdariShop)
	{
		m_pUnderBarBongdariShop->SetPushSound("ui_desktop_icon_click");
		m_pUnderBarBongdariShop->EnableHover(true, 0.0f);
		// 983 (00652310, INIT de "TOPPAGE.utab.bongdarishop"): sem o r.x += 74 do 645 (la o "maket" ficava antes;
		// no underbar.xml do 983 o bongdarishop fica em 240 e o magicbox em 285)
	}
}

void CTaskMain::OnUnderBar_BongdariShopUp()
{
	if (CSharedDoc::Instance()->IsControlServerService(0x10))
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xba\xc0\xb4\xd9\xb8\xae\xbc\xa5 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
				 "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		return;
	}
	if (CSharedDoc::Instance()->m_specialShotMark != 2)
	{
		if (!m_bBongdariShop1)
		{
			FrWnd* pShop =
				g_pFresh->GetManager()->GetDesktop()->FindChildByName(
					"bongdari_shop");
			if (pShop)
			{
				pShop->Close(true);
			}
			else
			{
				__rtti_obj = NULL;
				WSendPacket pkt((enumClientPacket)0x98);
				pkt.Send(TO_GAME);
				CSharedDoc::Instance()->m_pendingMenu = 1;
			}
		}
	}
	else
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		WRect rc = pForm->GetRect();
		pForm->SetRect(rc);
		pForm->SetTimeLimit(12.0f);
		pForm->SetMessage(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xd1 \xc1\xbe\xb7\xf9\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb "
			"\xba\xb8\xc0\xaf \xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce "
			"\xc7\xd8\xb4\xe7 \xc4\xc1\xc5\xd9\xc3\xf7\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 "
			"\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 0);
	}
	m_bBongdariShop0 = false;
	m_bBongdariShop1 = false;
	OnBarHoverOff(0);
}



void CTaskMain::OnUnderBar_MaketInit(int param)
{
	m_pUnderBarMaket = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarMaket)
	{
		m_pUnderBarMaket->EnableHover(true, 0.0f);
		m_pUnderBarMaket->SetVisible(false);
	}
}

void CTaskMain::OnUnderBar_MaketUp()
{
	if (0) // HACK: keeps the literal keeper of OnUnderBar_BongdariShopUp emitted
		((std::list<sChannelInfo>*)0)->clear();
	if (!IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
	{
		this << MsgObject(this, M983(0x23),
			(int)"\xc6\xce\xbe\xdf\xb8\xb6\xc4\xcf \xb9\xd7 \xc1\xf8\xbf\xad\xb4\xeb "
				 "\xb1\xe2\xb4\xc9\xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
				 "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.",
			0, 0, 0, 0);
		return;
	}
	if (CSharedDoc::Instance()->IsControlServerService(0x80000))
	{
		this << MsgObject(this, M983(0x23),
			(int)"\xc6\xce\xbe\xdf\xb8\xb6\xc4\xcf \xb9\xd7 \xc1\xf8\xbf\xad\xb4\xeb "
				 "\xb1\xe2\xb4\xc9\xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
				 "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.",
			0, 0, 0, 0);
		return;
	}
	if (!m_pUnderBarMaket)
		return;
	CloseAllDialogs();
	if (IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
	{
		WSendPacketSem983 pkt((enumClientPacket)0xca); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		pkt.Encode2(1);
		pkt.Send(TO_GAME);
	}
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	OnBarHoverOff(0);
}

// 983 (00655d10): a janela de busca de itens (FrItemSearchDlg "ItemSearchDlg", jp/itemsearchdlg.cpp), aberta pelo 0x2d8
// (botao de busca da Loja/My Room) e pela tecla SEARCH; resultado 00651060 limpa o ponteiro. O COfflineTradeDlg
// "offlinetraderanking" do 645 nao existe no 983 (D638).
void CTaskMain::CreateMaketDlg()
{
	if (!m_pMaketDlg)
	{
		m_pMaketDlg = CreateForm<FrItemSearchDlg>(g_pFresh->GetManager(), this,
			"ItemSearchDlg", NULL);
		m_pMaketDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnMaketListDlgResult, 1);
	}
}

void CTaskMain::OnUnderBar_RankingInit(int param)
{
	m_pUnderBarRanking = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarRanking)
	{
		m_pUnderBarRanking->EnableHover(true, 0.0f);
		// 983 (006523a0): sem o r.x += 74 do 645 (underbar.xml do 983: ranking = 330, passo 45)
	}
}

void CTaskMain::OnUnderBar_RankingUp()
{
	if (CSharedDoc::Instance()->IsControlServerService(0x10000))
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xb7\xa9\xc5\xb7 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
				 "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		return;
	}
	CloseAllDialogs();
	if (WSingleton<WNetworkSystem>::Instance()->IsConnected(
			WNetworkSystem::NET_GAME))
	{
		WSendPacket pkt((enumClientPacket)0x47);
		pkt.Send(TO_GAME);
	}
	else
	{
		WSingleton<WNetworkSystem>::Instance()->Init(WNetworkSystem::NET_RANK);
		WSingleton<WNetworkSystem>::Instance()->SendMessage(
			WNetworkSystem::NET_RANK, (NetworkUnit::eEvent)0);
	}
	if (!m_pLoginRsDlg)
	{
		m_pLoginRsDlg = CreateForm<FrLoginRsDlg>(g_pFresh->GetManager(), this,
			"login_gs", NULL);
		FrLoginRsDlg::SetState(LOGINRS_CONNECTING);
		m_pLoginRsDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnLoginRsDlgResult,
			0x41);
	}
	OnBarHoverOff(0);
}

// 983 (00658e20): botao "utab.combine" -> pagina "entrance" do PangYa pelo CWebPageConnector (pacote 0xfb; o 0x1ad abre), tipo 10
void CTaskMain::OnUnderBar_CombineUp983()
{
	CWebPageConnector* pConnector = CWebPageConnector::Instance();
	if (pConnector)
	{
		CloseAllDialogs();
		if (COption::Instance())
			COption::Instance()->SetScreenSize983(800, 600);
		pConnector->RequestOpen983(
			"https://www5.pangya.jp/on_client/entrance.aspx?ID=%s&KID=%s", true, 10);
		OnBarHoverOff(0);
	}
}

// 983 (00658e70): botao "utab.gacha" -> pagina do gacha, tipo 2
void CTaskMain::OnUnderBar_GachaUp983()
{
	CWebPageConnector* pConnector = CWebPageConnector::Instance();
	if (pConnector)
	{
		CloseAllDialogs();
		if (COption::Instance())
			COption::Instance()->SetScreenSize983(800, 600);
		pConnector->RequestOpen983(
			"https://www5.pangya.jp/on_client/gacha/login.aspx?ID=%s&KID=%s", true, 2);
		OnBarHoverOff(0);
	}
}

// 983 (00656700): "obar.eqitem" (sem checar o ponteiro, como no 983); com a janela mais larga que a tela recua 140, senao
// centraliza como os outros da barra; desligado no estado 8 do doc
void CTaskMain::OnOverBar_EqItemInit983(int param)
{
	m_pOverBarEqItem = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	float viewWidth = g_view ? g_view->GetWidth() : 800.0f;
	SetCenterOffset(m_pOverBarEqItem, (m_screenWidth < viewWidth) ? -140.0f : m_offsetX, 0.0f);
	m_pOverBarEqItem->Enable(DOCJP()->GetState983() != 8);
}

// 983 (006567e0): msg 0x81 (caso do HandleMsgCommon -> OpenEquipDlg983)
void CTaskMain::OnOverBar_EqItemDown983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x81 /*so do 983*/, 0, 0, 0, 0, 0));
}

// 983 (00659bd0): figura "overbar_equip_item" (apagada quando desligado) e a selecao do botao 0x33
void CTaskMain::OnOverBar_EqItemOwnerDraw983(int)
{
	FrWndManager* pManager = g_pFresh->GetManager();
	FrGraphicInterface* pDevice = pManager->GetGDI();
	if (!pDevice)
		return;
	unsigned long color = m_pOverBarEqItem->IsEnabled() ? 0xffffffff : 0x50b0b0b0;
	const Bitmap* pBmp = pManager->GetBitmap("", "overbar_equip_item");
	WRect& rc = const_cast<WRect&>(m_pOverBarEqItem->GetRect());
	if (pBmp)
	{
		WRect r = rc;
		pDevice->DrawTexture(pBmp, r, color, 0);
	}
	_DrawSelection(m_pOverBarEqItem, rc, (eBarButton)0x33 /*983*/, pDevice);
}

// 983 (00659340): botao "utab.point_shop" -> fecha os dialogos e abre a "AlchemyDlg" (quebra de MP)
void CTaskMain::OnUnderBar_PointShopUp983()
{
	CloseAllDialogs();
	OpenAlchemyDlg983();
}

// 983 (00659600): botao "utab.card_compose" (o OpenCardComposeDlg983 ja fecha os dialogos)
void CTaskMain::OnUnderBar_CardComposeUp983()
{
	OpenCardComposeDlg983();
}

// 983 (00655d70): "AlchemyDlg" (FrAlchemyMPBreakDlg da B). Servico 0x2_00000000 suspenso -> so o aviso (msg 0x2ee).
// Com a janela ja aberta nao faz nada; no lobby modal (00e78cac == 1) desliga o ntAvatarManager.
void CTaskMain::OpenAlchemyDlg983()
{
	if (DOCJP()->IsControlServerService983(0x200000000i64))
	{
		S5::UTIL::NotifyServiceStopped983(0x200000000i64);
		return;
	}
	if (m_pAlchemyDlg983 == NULL)
	{
		m_pAlchemyDlg983 = CreateForm<FrAlchemyMPBreakDlg>(g_pFresh->GetManager(), this,
			"AlchemyDlg", NULL);
		m_pAlchemyDlg983->Open((FRESH_PFN_RESULT)&CTaskMain::OnAlchemyDlgResult983, 0x11);
		if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
			ntAvatarManager::m_pInstance->SetActive(false);
		CSharedDoc::Instance()->m_bShowNotice = false; // +0x10b25
	}
}

// 983 (00659490): "card_compose" (FrCardComposeDlg da B), presa (sem arrastar) e com o icone no botao da barra (+0x6c).
// Servico 0x1_00000000 suspenso -> so o aviso (msg 0x2ee).
void CTaskMain::OpenCardComposeDlg983()
{
	if (DOCJP()->IsControlServerService983(0x100000000i64))
	{
		S5::UTIL::NotifyServiceStopped983(0x100000000i64);
		return;
	}
	CloseAllDialogs();
	m_pCardComposeDlg983 = CreateForm<FrCardComposeDlg>(g_pFresh->GetManager(), this,
		"card_compose", NULL);
	m_pCardComposeDlg983->EnableDrag(false);
	m_pCardComposeDlg983->Open((FRESH_PFN_RESULT)&CTaskMain::OnCardComposeDlgResult983, 0x11);
	if (m_pUnderBarMaket && m_pCardComposeDlg983)
		m_pCardComposeDlg983->SetIconRect(m_pUnderBarMaket->GetRect());
	OnBarHoverOff(0);
}

// 983 (00659360): msg 0x2e0 -> pede a lista (0x127) e abre a "PointShopDlg" (loja de pontos do Tiki) com o icone no
// botao "utab.point_shop"
void CTaskMain::OpenPointShopDlg983()
{
	WSendPacket packet(0x127);
	packet.Send(TO_GAME);

	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

	m_pPointShopDlg983 = CreateForm<FrPointShopDlg>(g_pFresh->GetManager(), this, "PointShopDlg", NULL);
	if (m_pPointShopDlg983)
	{
		m_pPointShopDlg983->Open((FRESH_PFN_RESULT)&CTaskMain::OnPointShopDlgResult983, 0x11);
		if (m_pUnderBarPointShop && m_pPointShopDlg983)
			m_pPointShopDlg983->SetIconRect(m_pUnderBarPointShop->GetRect());
		OnBarHoverOff(0);
		CSharedDoc::Instance()->m_bShowNotice = false; // +0x10b25
	}
}

// 983 (00651030): msg 0x2e2
void CTaskMain::RefreshPointShopDlg983()
{
	if (m_pPointShopDlg983)
		m_pPointShopDlg983->Refresh983();
}

// 983 (00651260)
bool CTaskMain::OnPointShopDlgResult983(int result, FrForm* pForm)
{
	m_pPointShopDlg983 = NULL;
	return true;
}

// 983 (00651080): msg 0x2e6 (sem checar o ponteiro, como no 983)
void CTaskMain::ResetAlchemyDlg983()
{
	m_pAlchemyDlg983->Reset983();
}

void CTaskMain::OnUnderBar_GuildInit(int param)
{
	m_pUnderBarGuild = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarGuild)
	{
		m_pUnderBarGuild->EnableHover(true, 0.0f); // 983 00652420: sem o SetVisible(false) do 645 (underbar.xml: guild = 375)
	}
}

// 983 (00658ec0): so fecha as janelas (vtable+0x3c) e solta o hover; a pagina web do guild do 645 nao existe no 983
void CTaskMain::OnUnderBar_GuildUp()
{
	CloseAllDialogs();
	OnBarHoverOff(0);
}

// 983 (00659240, virtual: slot 23 da vtable): pagina do clube, tipo 5
void CTaskMain::OnUnderBar_ClubUp983()
{
	CWebPageConnector* pConnector = CWebPageConnector::Instance();
	if (pConnector)
	{
		CloseAllDialogs();
		if (COption::Instance())
			COption::Instance()->SetScreenSize983(800, 600);
		pConnector->RequestOpen983(
			"https://www5.pangya.jp/on_client/club/login.aspx?ID=%s&KID=%s", true, 5);
		OnBarHoverOff(0);
	}
}

void CTaskMain::OnUnderBar_NoticeInit(int param)
{
	m_pUnderBarNotice = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnderBarNotice)
	{
		m_pUnderBarNotice->EnableHover(true, 0.0f); // 983 006524a0: sem o r.x += 24.5 do 645 (underbar.xml: notice = 420)
	}
}

void CTaskMain::OnUnderBar_NoticeUp()
{
	CloseAllDialogs();
	m_pNoticeDlg = CreateForm<FrNoticeDlg>(g_pFresh->GetManager(), this,
		"notice_popup", NULL);
	// 983 (00659290): as imagens do aviso vem do CExtraResource (baixadas), nao do notice_popup.ini
	m_pNoticeDlg->SetImages983(
		static_cast<CExtraResourceNoticesA983*>(CExtraResource::Instance())->GetNotices983());
	m_pNoticeDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnNoticeDlgResult, 0x11);
	if (m_pUnderBarNotice && m_pNoticeDlg)
		m_pNoticeDlg->SetIconRect(m_pUnderBarNotice->GetRect());
	OnBarHoverOff(0);
	CSharedDoc::Instance()->m_bShowNotice = false;
}

void CTaskMain::InitOverBar()
{
	m_pOverBarTab = NULL;
	m_pOverBarEqItem = NULL; // 983 00651090
	m_pOverBarChar = NULL;
	m_pOverBarCaddie = NULL;
	m_pOverBarAztec = NULL;
	m_pOverBarClub = NULL;
	m_pOverBarCharSel = NULL;
	m_pOverBarCaddieSel = NULL;
	m_pOverBarClubSel = NULL;
	m_pOverBarAztecSel = NULL;
	m_pOverBarMascotSel = NULL;
	m_pOverBarSite = NULL;
	m_pOverBarPangCookie = NULL;
	m_pOverBarTitle = NULL;
	m_pOverBarOneLineView = NULL;
	m_pOverBarOneLineReqBtn = NULL;
	m_bShowSubSelection = false;
	m_subSelTypeId = 0;
	m_subSelItemId = 0;
	m_bOverBarFlag0 = false;
	m_bOverBarFlag1 = false;
}

void CTaskMain::ProcessOverBar()
{
	if (g_input->GetButton(LEFT_BUTTON) == 2)
		m_bOverBarFlag0 = true;
	if (m_bOverBarFlag1 && m_bOverBarFlag0)
	{
		if (g_input->GetButton(LEFT_BUTTON) == 1)
		{
			m_bOverBarFlag0 = false;
			m_bOverBarFlag1 = false;
			SendSubSelectionPacket((PANGYA_ITEM_GROUP)0x20);
		}
	}
	// 983 (0065966a): imagens do botao spcard refeitas a cada quadro pelo bit de premium
	if (m_pOverBarSpecialCard)
	{
		if ((Doc()->m_myInfo.info.dwIdentity >> 14) & 1)
		{
			m_pOverBarSpecialCard->SetButtonImg("spcard_btn_on", FrButton::NORMAL);
			m_pOverBarSpecialCard->SetButtonImg("premium_btn_on", FrButton::OVER);
			return;
		}
		m_pOverBarSpecialCard->SetButtonImg("spcard_btn_n", FrButton::NORMAL);
		m_pOverBarSpecialCard->SetButtonImg("spcard_btn_o", FrButton::OVER);
	}
}

void CTaskMain::SendSubSelectionPacket(PANGYA_ITEM_GROUP group)
{
	CloseAllSubSelection();
	int id;
	if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		id = 0xc;
	else
		id = 0xb;
	if (group == 0x20)
		group = (PANGYA_ITEM_GROUP)(m_subSelTypeId >> 26);
	switch (group)
	{
	case PIG_CHAR:
	{
		if (IS_EXACTKINDOF(CLobbyTask, AfxGetTask()) &&
			CSharedDoc::Instance()->m_roomInfo.roomGuid != 0xffff &&
			!IsMassGame(CSharedDoc::Instance()->m_roomInfo.holeType))
			id = 0xc;
		WSendPacket pkt(id);
		pkt.Encode1(4);
		pkt.Encode4(m_subSelItemId);
		pkt.Send(TO_GAME);
		break;
	}
	case PIG_CADDIE:
	{
		WSendPacket pkt(id);
		pkt.Encode1(1);
		pkt.Encode4(m_subSelItemId);
		pkt.Send(TO_GAME);
		break;
	}
	case PIG_CLUBSET:
	{
		WSendPacket pkt(id);
		pkt.Encode1(3);
		pkt.Encode4(m_subSelItemId);
		pkt.Send(TO_GAME);
		break;
	}
	case PIG_BALL:
	{
		WSendPacket pkt(id);
		pkt.Encode1(2);
		pkt.Encode4(m_subSelTypeId);
		pkt.Send(TO_GAME);
		break;
	}
	case PIG_MASCOT:
	{
		WSendPacket pkt(id);
		pkt.Encode1(5);
		pkt.Encode4(m_subSelItemId);
		pkt.Send(TO_GAME);
		break;
	}
	}
}

void CTaskMain::ShowOverAndUnderBar(bool bShow)
{
	static bool s_bBarShown = true;
	static bool s_barVisible[3];
	if (s_bBarShown == bShow)
		return;
	s_bBarShown = bShow;
	FrGroupBox** bars = &m_pOverBar;
	for (int i = 0; i < 3; i++)
	{
		if (bars[i])
		{
			if (bShow)
			{
				bars[i]->SetVisible(s_barVisible[i]);
			}
			else
			{
				s_barVisible[i] = bars[i]->IsVisible();
				bars[i]->SetVisible(false);
			}
		}
	}
	if (m_pUnderBarMaket)
		m_pUnderBarMaket->SetVisible(false);
}

void CTaskMain::EnableOverAndUnderBar(bool bEnable)
{
	static bool s_bBarEnabled = true;
	static bool s_barEnabled[35];
	FrWnd* list[35] = { m_pOverBarChar, m_pOverBarCaddie, m_pOverBarAztec,
		m_pOverBarClub, m_pOverBarCharSel, m_pOverBarCaddieSel,
		m_pOverBarAztecSel, m_pOverBarClubSel, m_pOverBarMascotSel,
		m_pOverBarPangCookie, m_pOverBarTitle, m_pOverBarOneLineView,
		m_pOverBarOneLineReqBtn, m_pUnderBarBack, m_pUnderBarFront,
		m_pUnderBarShop, m_pUnderBarMyRoom, m_pUnderBarItemStorage,
		m_pUnderBarGift, m_pUnderBarCookie, m_pUnderBarBongdariShop,
		m_pUnderBarGame, m_pUnderBarServer, m_pUnderBarExit, m_pUnderBarTabBtn,
		m_pOverBar, m_pUnderBar, m_pUnderTab, m_pUnderBarMagicBox,
		m_pUnderBarScratch, m_pUnderBarRanking, m_pUnderBarGuild,
		m_pUnderBarNotice, m_pOverBarMascot, m_pUnderBarMaket };
	if (s_bBarEnabled == bEnable)
		return;
	s_bBarEnabled = bEnable;
	for (unsigned int i = 0; i < 35; i++)
	{
		if (list[i])
		{
			if (bEnable)
			{
				list[i]->Enable(s_barEnabled[i]);
			}
			else
			{
				s_barEnabled[i] = list[i]->IsEnabled();
				list[i]->Enable(false);
			}
		}
	}
	if (m_pUnderBarMaket)
		m_pUnderBarMaket->SetVisible(false);
}

void CTaskMain::EnableUnderBar(bool enable)
{
	m_pUnderBarShop->Enable(enable);
	m_pUnderBarMyRoom->Enable(enable);
	m_pUnderBarItemStorage->Enable(enable);
	m_pUnderBarGift->Enable(enable);
	m_pUnderBarCookie->Enable(enable);
	m_pUnderBarBongdariShop->Enable(enable);
	m_pUnderBarGame->Enable(enable);
	m_pUnderBarServer->Enable(enable);
	m_pUnderBarTabBtn->Enable(enable);
}

void CTaskMain::ShowAllSubSelection()
{
	m_bShowSubSelection = true;
	m_subSelTypeId = 0;
	m_subSelItemId = 0;
}

void CTaskMain::CloseAllSubSelection()
{
	if (m_pOverBarCharSel)
		m_pOverBarCharSel->SetVisible(false);
	if (m_pOverBarCaddieSel)
		m_pOverBarCaddieSel->SetVisible(false);
	if (m_pOverBarCaddieSel)
		m_pOverBarCaddieSel->SetVisible(false);
	if (m_pOverBarAztecSel)
		m_pOverBarAztecSel->SetVisible(false);
	if (m_pOverBarClubSel)
		m_pOverBarClubSel->SetVisible(false);
	if (m_pOverBarMascotSel)
		m_pOverBarMascotSel->SetVisible(false);
	m_bShowSubSelection = false;
}

void CTaskMain::OnOverBar_Bar1Init(int param)
{
	m_pOverBarBar1 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarBar1)
	{
		if (S5::CLASSICSRV::IsClassicGame983())
			m_pOverBarBar1->SetBgImg("classic_over_bar_01");
	}
}


void CTaskMain::OnOverBar_Bar2Init(int param)
{
	m_pOverBarBar2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarBar2)
	{
		WRect r = m_pOverBarBar2->GetRect();
		r.w = (m_pOverBarBar3->GetRect().x - m_pOverBarBar1->GetRect().x) -
			m_pOverBarBar1->GetRect().w;
		m_pOverBarBar2->SetRect(r);
	}
}

void CTaskMain::OnOverBar_Bar3Init(int param)
{
	m_pOverBarBar3 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarBar3)
	{
		SetLeftOffset(m_pOverBarBar3, m_screenWidth); // 983: 00651ad0
	}
}

void CTaskMain::OnOverBar_TitleInit(int param)
{
	m_pOverBarTitle = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void CTaskMain::OnOverBar_Frame1Init(int param)
{
	m_pOverBarFrame1 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarFrame1)
	{
		const WRect& rc = m_pOverBarFrame1->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarFrame1->SetRect(r);
	}
}

void CTaskMain::OnOverBar_Frame2Init(int param)
{
	m_pOverBarFrame2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarFrame2)
	{
		const WRect& rc = m_pOverBarFrame2->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarFrame2->SetRect(r);
	}
}

void CTaskMain::OnOverBar_Frame3Init(int param)
{
	m_pOverBarFrame3 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarFrame3)
	{
		const WRect& rc = m_pOverBarFrame3->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarFrame3->SetRect(r);
	}
}

void CTaskMain::OnOverBar_TabInit(int param)
{
	m_pOverBarTab = DYNAMIC_CAST(FrTabButton, (FrWnd*)param);
	if (m_pOverBarTab)
	{
		const WRect& rc = m_pOverBarTab->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarTab->SetRect(r);
	}
}

void CTaskMain::OnOverBar_TabDown()
{
	if (0)
		((CTaskMain*)0)->CTaskMain::OnOverBar_TabInit(*(int*)0);
}

void CTaskMain::OnOverBar_CharInit(int param)
{
	m_pOverBarChar = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarChar)
	{
		const WRect& rc = m_pOverBarChar->GetRect();
		float x = (ViewWidth983() <= m_screenWidth) ? rc.x + m_offsetX : rc.x - 140.0f;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarChar->SetRect(r);
		m_pOverBarChar->Enable(DOCJP()->GetState983() != 8); // 983 00656050: layout GAMEROOM_EXTRES nao existe; 8 = FrGameRoomDlg
	}
}

void CTaskMain::OnOverBar_CharDown()
{
	WRect rc = m_pOverBarCharSel->GetRect();
	WPoint pos(rc.x, rc.y + rc.h);
	OpenSubEquipBar(pos, PIG_CHAR, 3);
	if (m_pSubEquipBar)
	{
		WRect r = m_pSubEquipBar->GetRect();
		r.x = rc.x;
		m_pSubEquipBar->SetRect(r);
	}
}

void CTaskMain::OnOverBar_CharOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	unsigned long color = m_pOverBarChar->IsEnabled() ? 0xffffffff : 0x50b0b0b0;
	WRect rc = m_pOverBarChar->GetRect();
	const char* name;
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidChar == 0)
	{
		name = "bar_no";
	}
	else
	{
		std::map<unsigned int, sCharacterInfo>::iterator it =
			CSharedDoc::Instance()->m_charMap.find(
				CSharedDoc::Instance()->m_myInfo.userEquip.guidChar);
		if (it == CSharedDoc::Instance()->m_charMap.end()) // 983 0065c90a
			goto drawSel;
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (!pItem)
			goto drawSel;
		name = MakeStr("s_%s", pItem->Icon);
	}
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(name);
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, color, 0);
	}
drawSel:
	_DrawSelection(m_pOverBarChar, rc, TOOLTIP_CHARACTER, pDevice);
}

void CTaskMain::_DrawSelection(FrViewer* pViewer, WRect& rc, eBarButton btn,
	FrGraphicInterface* pGI)
{
	if (!pGI)
	{
		pGI = g_pFresh->GetManager()->GetGDI();
		if (!pGI)
			return;
	}
	WVector mouse = g_input->GetMousePoint();
	WPoint pt(mouse.x, mouse.y);
	if (pViewer->IsEnabled() && rc.IsInRect(pt))
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap("select_frame");
		if (pBmp)
		{
			WRect r(rc.x - 3.0f, rc.y - 3.0f, rc.w + 6.0f, rc.h + 6.0f);
			pGI->DrawTexture(pBmp, r, 0xffffffff, 0);
		}
		CIconToolTip::GetInstance()->SetToolTip(btn);
		return;
	}
	if (CIconToolTip::GetInstance()->IsCurToolTip(btn))
		OnBarHoverOff(0);
}


void CTaskMain::OnOverBar_CaddieDown()
{
	WRect rc = m_pOverBarCaddieSel->GetRect();
	WPoint pos(rc.x, rc.y + rc.h);
	OpenSubEquipBar(pos, PIG_CADDIE, 3);
	if (m_pSubEquipBar)
	{
		WRect r = m_pSubEquipBar->GetRect();
		r.x = rc.x;
		m_pSubEquipBar->SetRect(r);
	}
}

void CTaskMain::NotifyServerControlMsg(int code)
{
	const char* text;
	if (_util::IsAvatarChatServer(0x40))
		text = _util::GetAvatarServerServiceMessage(code);
	else
		text = _util::GetNormalServerServiceMessage(code);
	if (text)
	{
		MsgObject msg(NULL, 0x23, (int)text, 0, 0, 0, 0);
		this << msg;
	}
}

void CTaskMain::OnOverBar_CaddieInit(int param)
{
	m_pOverBarCaddie = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarCaddie)
	{
		const WRect& rc = m_pOverBarCaddie->GetRect();
		float x = (ViewWidth983() <= m_screenWidth) ? rc.x + m_offsetX : rc.x - 140.0f;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarCaddie->SetRect(r);
		m_pOverBarCaddie->Enable(DOCJP()->GetState983() != 8); // 983 00656050: layout GAMEROOM_EXTRES nao existe; 8 = FrGameRoomDlg
	}
}

void CTaskMain::OnOverBar_CaddieOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	unsigned long color =
		m_pOverBarCaddie->IsEnabled() ? 0xffffffff : 0x50b0b0b0;
	WRect rc = m_pOverBarCaddie->GetRect();
	const Bitmap* pBmp = NULL;
	bool bRentExpired = false;
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie == 0)
	{
		pBmp = g_pFresh->GetBitmap("bar_no");
	}
	else
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			CSharedDoc::Instance()->m_caddieMap.find(
				CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie);
		if (it == CSharedDoc::Instance()->m_caddieMap.end()) // 983 0065ca40
			goto drawSel;
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (pItem)
			pBmp = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->Icon));
		IFF_STRUCT::sCaddie* pCaddie =
			CSharedDoc::Instance()->m_itemManager.FindCaddie((*it).second.tid);
		if (pCaddie && (*it).second.Rent_flag &&
			(*it).second.Remain_Date == 0 && (*it).second.tidPart == 0 &&
			pCaddie->MonthlyFee)
			bRentExpired = true;
	}
	if (pBmp)
		pDevice->DrawTexture(pBmp, rc, color, 0);
	if (bRentExpired)
	{
		g_view->DrawLine2D(WPoint(rc.x, rc.h * 0.5f + rc.y - 1.0f),
			WPoint(rc.x + rc.w - 1.0f, rc.h * 0.5f + rc.y - 1.0f), 0xffff0000,
			0xffff0000, 0);
		g_view->DrawLine2D(WPoint(rc.x, rc.h * 0.5f + rc.y),
			WPoint(rc.x + rc.w - 1.0f, rc.h * 0.5f + rc.y), 0xffff0000,
			0xffff0000, 0);
		g_view->DrawLine2D(WPoint(rc.x, rc.h * 0.5f + rc.y + 1.0f),
			WPoint(rc.x + rc.w - 1.0f, rc.h * 0.5f + rc.y + 1.0f), 0xffff0000,
			0xffff0000, 0);
	}
drawSel:
	_DrawSelection(m_pOverBarCaddie, rc, TOOLTIP_CADDIE, pDevice);
}

void CTaskMain::OnOverBar_ClubInit(int param)
{
	m_pOverBarClub = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarClub)
	{
		const WRect& rc = m_pOverBarClub->GetRect();
		float x = (ViewWidth983() <= m_screenWidth) ? rc.x + m_offsetX : rc.x - 140.0f;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarClub->SetRect(r);
		m_pOverBarClub->Enable(DOCJP()->GetState983() != 8); // 983 00656050: layout GAMEROOM_EXTRES nao existe; 8 = FrGameRoomDlg
	}
}

void CTaskMain::OnOverBar_ClubDown()
{
	WRect rc = m_pOverBarClubSel->GetRect();
	WPoint pos(rc.x, rc.y + rc.h);
	OpenSubEquipBar(pos, PIG_CLUBSET, 3);
	if (m_pSubEquipBar)
	{
		WRect r = m_pSubEquipBar->GetRect();
		r.x = rc.x;
		m_pSubEquipBar->SetRect(r);
	}
}

void CTaskMain::OnOverBar_ClubOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	unsigned long color = m_pOverBarClub->IsEnabled() ? 0xffffffff : 0x50b0b0b0;
	WRect rc = m_pOverBarClub->GetRect();
	const char* name;
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet == 0)
	{
		name = "bar_no";
	}
	else
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			CSharedDoc::Instance()->m_clubSetMap.find(
				CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet);
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (!pItem)
			goto drawSel;
		name = MakeStr("s_%s", pItem->Icon);
	}
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(name);
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, color, 0);
	}
drawSel:
	_DrawSelection(m_pOverBarClub, rc, TOOLTIP_CLUB, pDevice);
}

void CTaskMain::OnOverBar_AztecInit(int param)
{
	m_pOverBarAztec = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarAztec)
	{
		const WRect& rc = m_pOverBarAztec->GetRect();
		float x = (ViewWidth983() <= m_screenWidth) ? rc.x + m_offsetX : rc.x - 140.0f;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarAztec->SetRect(r);
		m_pOverBarAztec->Enable(DOCJP()->GetState983() != 8); // 983 00656050: layout GAMEROOM_EXTRES nao existe; 8 = FrGameRoomDlg
	}
}

void CTaskMain::OnOverBar_AztecDown()
{
	WRect rc = m_pOverBarAztecSel->GetRect();
	WPoint pos(rc.x, rc.y + rc.h);
	OpenSubEquipBar(pos, PIG_BALL, 3);
	if (m_pSubEquipBar)
	{
		WRect r = m_pSubEquipBar->GetRect();
		r.x = rc.x;
		m_pSubEquipBar->SetRect(r);
	}
}

void CTaskMain::OpenSubEquipBar(const WPoint& pos, PANGYA_ITEM_GROUP group,
	int subType)
{
	if (!m_pSubEquipBar)
	{
		m_pSubEquipBar = CreateForm<FrSubEquipBar>(g_pFresh->GetManager(), this,
			"EQUIPBAR_SUB", NULL);
		if (m_pSubEquipBar)
			m_pSubEquipBar->Open(
				(FRESH_PFN_RESULT)&CTaskMain::OnResultSubEquipBar,
				(FrSubEquipBar::eCategory)group, pos,
				(FrSubEquipBar::eSubType)subType);
	}
}


bool CTaskMain::OnResultSubEquipBar(int result, FrForm*)
{
	if (result == 1)
	{
		PANGYA_ITEM_GROUP group = (PANGYA_ITEM_GROUP)0x20;
		if (m_pSubEquipBar->GetResult())
		{
			unsigned long tid = m_pSubEquipBar->GetResult()->TypeId;
			if ((tid >> 26) == PIG_BALL)
			{
				CSharedDoc::Instance()->m_myInfo.userEquip.tidBall = tid;
				m_subSelTypeId = tid;
				m_subSelItemId = 0;
			}
			else
			{
				m_subSelTypeId = tid;
				m_subSelItemId = m_pSubEquipBar->GetLastSelectedGuid();
				if (!m_pSubEquipBar->SetClientEquipInfo(
						(PANGYA_ITEM_GROUP)(tid >> 26), m_subSelItemId))
				{
					m_pSubEquipBar = NULL;
					return true;
				}
			}
		}
		else
		{
			if (m_pSubEquipBar->GetCategory() == FrSubEquipBar::CATEGORY_CADDIE)
			{
				CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie = 0;
				group = PIG_CADDIE;
			}
			else if (m_pSubEquipBar->GetCategory() ==
				FrSubEquipBar::CATEGORY_MASCOT)
			{
				CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot = 0;
				group = PIG_MASCOT;
			}
			m_subSelTypeId = 0;
			m_subSelItemId = 0;
		}
		SendSubSelectionPacket(group);
	}
	m_pSubEquipBar = NULL;
	return true;
}

void CTaskMain::OnOverBar_AztecOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	WRect rc = m_pOverBarAztec->GetRect();
	unsigned long color =
		m_pOverBarAztec->IsEnabled() ? 0xffffffff : 0x50b0b0b0;
	const char* name;
	if (CSharedDoc::Instance()->m_myInfo.userEquip.tidBall == 0)
	{
		name = "bar_no";
	}
	else
	{
		// 983 00659b36: com premium (bit 14 de +0x812) a bola padrao aparece como 0x140000d8
		unsigned long tidBall = CSharedDoc::Instance()->m_myInfo.userEquip.tidBall;
		if (((*(unsigned long*)DOCPTR983(0x812) >> 0xe) & 1) &&
			tidBall == 0x14000000)
			tidBall = 0x140000d8;
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(tidBall);
		if (!pItem)
			goto drawSel;
		name = MakeStr("s_%s", pItem->Icon);
	}
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(name);
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, color, 0);
	}
drawSel:
	_DrawSelection(m_pOverBarAztec, rc, TOOLTIP_AZTEC, pDevice);
}

void CTaskMain::OnOverBar_MascotInit(int param)
{
	m_pOverBarMascot = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarMascot)
	{
		const WRect& rc = m_pOverBarMascot->GetRect();
		float x = (ViewWidth983() <= m_screenWidth) ? rc.x + m_offsetX : rc.x - 140.0f;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarMascot->SetRect(r);
		m_pOverBarMascot->Enable(DOCJP()->GetState983() != 8); // 983 00656050: layout GAMEROOM_EXTRES nao existe; 8 = FrGameRoomDlg
	}
}

void CTaskMain::OnOverBar_MascotDown()
{
	WRect rc = m_pOverBarMascotSel->GetRect();
	WPoint pos(rc.x, rc.y + rc.h);
	OpenSubEquipBar(pos, PIG_MASCOT, 3);
	if (m_pSubEquipBar)
	{
		WRect r = m_pSubEquipBar->GetRect();
		r.x = rc.x;
		m_pSubEquipBar->SetRect(r);
	}
}

void CTaskMain::OnOverBar_MascotOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	unsigned long color =
		m_pOverBarMascot->IsEnabled() ? 0xffffffff : 0x50b0b0b0;
	WRect rc = m_pOverBarMascot->GetRect();
	const char* name;
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot == 0)
	{
		name = "bar_no";
	}
	else
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			CSharedDoc::Instance()->m_mascotMap.find(
				CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot);
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (!pItem)
			goto drawSel;
		name = MakeStr("s_%s", pItem->Icon);
	}
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(name);
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, color, 0);
	}
drawSel:
	_DrawSelection(m_pOverBarMascot, rc, TOOLTIP_MASCOT, pDevice);
}

void CTaskMain::OnOverBar_CharSelInit(int param)
{
	m_pOverBarCharSel = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarCharSel)
	{
		m_pOverBarCharSel->SetVisible(false);
		const WRect& rc = m_pOverBarCharSel->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarCharSel->SetRect(r);
	}
}

void CTaskMain::OnOverBar_CharSelOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	int count = (int)CSharedDoc::Instance()->m_charMap.size();
	if (count <= 1)
		return;

	float x = m_pOverBarCharSel->GetRect().x;
	float y = m_pOverBarCharSel->GetRect().y;
	float w;
	const Bitmap* pBmp = g_pFresh->GetBitmap("select_bar_01");
	if (pBmp)
	{
		w = 8.0f;
		pDevice->DrawTexture(pBmp, WRect(x, y, w, (float)pBmp->Height()),
			0xffffffff, 0);
	}
	float ix = w + x;
	x = ix;
	pBmp = g_pFresh->GetBitmap("select_bar_02");
	if (pBmp)
	{
		w = (float)count * 24.0f + (float)(count - 1) * 3.0f;
		pDevice->DrawTexture(pBmp, WRect(x, y, w, (float)pBmp->Height()),
			0xffffffff, 0);
	}
	x = w + x;
	pBmp = g_pFresh->GetBitmap("select_bar_03");
	if (pBmp)
	{
		pDevice->DrawTexture(pBmp, WRect(x, y, 8.0f, (float)pBmp->Height()),
			0xffffffff, 0);
	}

	m_subSelTypeId = 0;
	y += 7.0f;
	m_subSelItemId = 0;
	for (std::map<unsigned int, sCharacterInfo>::iterator it =
			 CSharedDoc::Instance()->m_charMap.begin();
		it != CSharedDoc::Instance()->m_charMap.end(); ++it)
	{
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (!pItem)
			continue;
		WRect rc(ix, y, 24.0f, 26.0f);
		pBmp = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->Icon));
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
		WVector mouse = g_input->GetMousePoint();
		WPoint pt(mouse.x, mouse.y);
		if (rc.IsInRect(pt))
		{
			pBmp = g_pFresh->GetBitmap("select_frame");
			if (pBmp)
				pDevice->DrawTexture(pBmp,
					WRect(rc.x - 3.0f, rc.y - 3.0f, rc.w + 6.0f, rc.h + 6.0f),
					0xffffffff, 0);
			if (CSharedDoc::Instance()->m_myInfo.userEquip.guidChar !=
				(*it).second.guid)
			{
				m_subSelTypeId = (*it).second.tid;
				m_subSelItemId = (*it).second.guid;
			}
		}
		ix += 27.0f;
	}
}

void CTaskMain::OnOverBar_CaddieSelInit(int param)
{
	m_pOverBarCaddieSel = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarCaddieSel)
	{
		m_pOverBarCaddieSel->SetVisible(false);
		const WRect& rc = m_pOverBarCaddieSel->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarCaddieSel->SetRect(r);
	}
}

void CTaskMain::OnOverBar_CaddieSelOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	int count = (int)CSharedDoc::Instance()->m_caddieMap.size();
	if (count <= 0)
		return;
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie != 0)
		count++;

	float x = m_pOverBarCaddieSel->GetRect().x;
	float y = m_pOverBarCaddieSel->GetRect().y;
	float w;
	const Bitmap* pBmp = g_pFresh->GetBitmap("select_bar_01");
	if (pBmp)
	{
		w = 8.0f;
		pDevice->DrawTexture(pBmp, WRect(x, y, w, (float)pBmp->Height()),
			0xffffffff, 0);
	}
	float ix = w + x;
	x = ix;
	pBmp = g_pFresh->GetBitmap("select_bar_02");
	if (pBmp)
	{
		w = (float)count * 24.0f + (float)(count - 1) * 3.0f;
		pDevice->DrawTexture(pBmp, WRect(x, y, w, (float)pBmp->Height()),
			0xffffffff, 0);
	}
	x = w + x;
	pBmp = g_pFresh->GetBitmap("select_bar_03");
	if (pBmp)
	{
		pDevice->DrawTexture(pBmp, WRect(x, y, 8.0f, (float)pBmp->Height()),
			0xffffffff, 0);
	}

	m_subSelTypeId = 0;
	y += 7.0f;
	m_subSelItemId = 0;
	for (std::map<unsigned int, sCaddieInfo>::iterator it =
			 CSharedDoc::Instance()->m_caddieMap.begin();
		it != CSharedDoc::Instance()->m_caddieMap.end(); ++it)
	{
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (!pItem)
			continue;
		WRect rc(ix, y, 24.0f, 26.0f);
		pBmp = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->Icon));
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
		IFF_STRUCT::sCaddie* pCaddie =
			CSharedDoc::Instance()->m_itemManager.FindCaddie((*it).second.tid);
		if (pCaddie && (*it).second.Rent_flag &&
			(*it).second.Remain_Date == 0 && (*it).second.tidPart == 0 &&
			pCaddie->MonthlyFee)
		{
			g_view->DrawLine2D(WPoint(rc.x, rc.h * 0.5f + rc.y - 1.0f),
				WPoint(rc.x + rc.w - 1.0f, rc.h * 0.5f + rc.y - 1.0f),
				0xffff0000, 0xffff0000, 0);
			g_view->DrawLine2D(WPoint(rc.x, rc.h * 0.5f + rc.y),
				WPoint(rc.x + rc.w - 1.0f, rc.h * 0.5f + rc.y), 0xffff0000,
				0xffff0000, 0);
			g_view->DrawLine2D(WPoint(rc.x, rc.h * 0.5f + rc.y + 1.0f),
				WPoint(rc.x + rc.w - 1.0f, rc.h * 0.5f + rc.y + 1.0f),
				0xffff0000, 0xffff0000, 0);
		}
		WVector mouse = g_input->GetMousePoint();
		WPoint pt(mouse.x, mouse.y);
		if (rc.IsInRect(pt))
		{
			pBmp = g_pFresh->GetBitmap("select_frame");
			if (pBmp)
				pDevice->DrawTexture(pBmp,
					WRect(rc.x - 3.0f, rc.y - 3.0f, rc.w + 6.0f, rc.h + 6.0f),
					0xffffffff, 0);
			if (CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie !=
				(*it).second.guid)
			{
				m_subSelTypeId = (*it).second.tid;
				m_subSelItemId = (*it).second.guid;
			}
		}
		ix += 27.0f;
	}
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie != 0)
	{
		WRect rc(ix, y, 24.0f, 26.0f);
		pBmp = g_pFresh->GetBitmap("s_no");
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
		WVector mouse = g_input->GetMousePoint();
		WPoint pt(mouse.x, mouse.y);
		if (rc.IsInRect(pt))
		{
			pBmp = g_pFresh->GetBitmap("select_frame");
			if (pBmp)
				pDevice->DrawTexture(pBmp,
					WRect(rc.x - 3.0f, rc.y - 3.0f, rc.w + 6.0f, rc.h + 6.0f),
					0xffffffff, 0);
			m_subSelTypeId =
				(*CSharedDoc::Instance()->m_caddieMap.begin()).second.tid;
			m_subSelItemId = 0;
		}
	}
}

void CTaskMain::OnOverBar_ClubSelInit(int param)
{
	m_pOverBarClubSel = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarClubSel)
	{
		m_pOverBarClubSel->SetVisible(false);
		const WRect& rc = m_pOverBarClubSel->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarClubSel->SetRect(r);
	}
}

void CTaskMain::OnOverBar_ClubSelOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	int count = (int)CSharedDoc::Instance()->m_clubSetMap.size();
	if (count <= 1)
		return;

	float x, y;
	float* p = &x;
	*p = m_pOverBarClubSel->GetRect().x;
	p = &y;
	*p = m_pOverBarClubSel->GetRect().y;
	float w;
	const Bitmap* pBmp = g_pFresh->GetBitmap("select_bar_01");
	if (pBmp)
	{
		w = 8.0f;
		WRect rc;
		rc.x = x;
		rc.y = y;
		rc.w = w;
		rc.h = (float)pBmp->Height();
		pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}
	x = w + x;
	float ix = x;
	pBmp = g_pFresh->GetBitmap("select_bar_02");
	if (pBmp)
	{
		w = (float)(count - 1) * 3.0f + (float)count * 24.0f;
		WRect rc;
		rc.x = x;
		rc.y = y;
		rc.w = w;
		rc.h = (float)pBmp->Height();
		pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}
	x = w + x;
	pBmp = g_pFresh->GetBitmap("select_bar_03");
	if (pBmp)
	{
		WRect rc;
		rc.x = x;
		rc.y = y;
		rc.w = 8.0f;
		rc.h = (float)pBmp->Height();
		pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}

	m_subSelTypeId = 0;
	y += 7.0f;
	m_subSelItemId = 0;
	for (std::map<unsigned int, sItemInfo>::const_iterator it =
			 CSharedDoc::Instance()->m_clubSetMap.begin();
		it != CSharedDoc::Instance()->m_clubSetMap.end(); ++it)
	{
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (!pItem)
			continue;
		pBmp = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->Icon));
		if (pBmp)
			pDevice->DrawTexture(pBmp, WRect(ix, y, 24.0f, 26.0f), 0xffffffff,
				0);
		bool bLevelOk =
			CSharedDoc::Instance()->CheckUnderLevelClubSet((*it).second.tid);
		if (!bLevelOk)
		{
			float right = ix + 24.0f - 1.0f;
			float ly = y + 13.0f;
			g_view->DrawLine2D(WPoint(ix, ly - 1.0f), WPoint(right, ly - 1.0f),
				0xffff0000, 0xffff0000, 0);
			g_view->DrawLine2D(WPoint(ix, ly), WPoint(right, ly), 0xffff0000,
				0xffff0000, 0);
			g_view->DrawLine2D(WPoint(ix, ly + 1.0f), WPoint(right, ly + 1.0f),
				0xffff0000, 0xffff0000, 0);
		}
		WPoint pt(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
		if (WRect(ix, y, 24.0f, 26.0f).IsInRect(pt))
		{
			pBmp = g_pFresh->GetBitmap("select_frame");
			if (pBmp)
				pDevice->DrawTexture(pBmp,
					WRect(ix - 3.0f, y - 3.0f, 30.0f, 32.0f), 0xffffffff, 0);
			if (CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet !=
					(*it).second.guid &&
				bLevelOk == true)
			{
				m_subSelTypeId = (*it).second.tid;
				m_subSelItemId = (*it).second.guid;
			}
		}
		ix += 27.0f;
	}
}

void CTaskMain::OnOverBar_AztecSelInit(int param)
{
	m_pOverBarAztecSel = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarAztecSel)
	{
		m_pOverBarAztecSel->SetVisible(false);
		const WRect& rc = m_pOverBarAztecSel->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarAztecSel->SetRect(r);
	}
}

void CTaskMain::OnOverBar_AztecSelOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	if (!CSharedDoc::Instance()->m_itemManager.FindCommonItem(
			CSharedDoc::Instance()->m_myInfo.userEquip.tidBall))
		return;
	int count = (int)CSharedDoc::Instance()->m_ballList.size();
	if (count <= 1)
		return;

	float x = m_pOverBarAztecSel->GetRect().x;
	float y = m_pOverBarAztecSel->GetRect().y;
	float w;
	const Bitmap* pBmp = g_pFresh->GetBitmap("select_bar_01");
	if (pBmp)
	{
		w = 8.0f;
		pDevice->DrawTexture(pBmp, WRect(x, y, w, (float)pBmp->Height()),
			0xffffffff, 0);
	}
	float ix = w + x;
	x = ix;
	pBmp = g_pFresh->GetBitmap("select_bar_02");
	if (pBmp)
	{
		w = (float)count * 24.0f + (float)(count - 1) * 3.0f;
		pDevice->DrawTexture(pBmp, WRect(x, y, w, (float)pBmp->Height()),
			0xffffffff, 0);
	}
	x = w + x;
	pBmp = g_pFresh->GetBitmap("select_bar_03");
	if (pBmp)
	{
		pDevice->DrawTexture(pBmp, WRect(x, y, 8.0f, (float)pBmp->Height()),
			0xffffffff, 0);
	}

	m_subSelTypeId = 0;
	y += 7.0f;
	m_subSelItemId = 0;
	for (std::list<sItemInfo>::iterator it =
			 CSharedDoc::Instance()->m_ballList.begin();
		it != CSharedDoc::Instance()->m_ballList.end(); ++it)
	{
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem((*it).tid);
		if (!pItem)
			continue;
		WRect rc(ix, y, 24.0f, 26.0f);
		pBmp = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->Icon));
		if (pBmp)
			pDevice->DrawTexture(pBmp, WRect(ix, y, 24.0f, 26.0f), 0xffffffff,
				0);
		WVector mouse = g_input->GetMousePoint();
		WPoint pt(mouse.x, mouse.y);
		if (rc.IsInRect(pt))
		{
			pBmp = g_pFresh->GetBitmap("select_frame");
			if (pBmp)
				pDevice->DrawTexture(pBmp,
					WRect(ix - 3.0f, y - 3.0f, 30.0f, 32.0f), 0xffffffff, 0);
			if (CSharedDoc::Instance()->m_myInfo.userEquip.tidBall != (*it).tid)
			{
				m_subSelTypeId = (*it).tid;
				m_subSelItemId = (*it).guid;
			}
		}
		ix += 27.0f;
	}
}

void CTaskMain::OnOverBar_MascotSelInit(int param)
{
	m_pOverBarMascotSel = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pOverBarMascotSel)
	{
		m_pOverBarMascotSel->SetVisible(false);
		const WRect& rc = m_pOverBarMascotSel->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarMascotSel->SetRect(r);
	}
}

void CTaskMain::OnOverBar_MascotSelOwnerDraw(int)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	int count = (int)CSharedDoc::Instance()->m_mascotMap.size();
	if (count < 1)
		return;
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot != 0)
		count++;

	float x, y;
	float* p = &x;
	*p = m_pOverBarMascotSel->GetRect().x;
	p = &y;
	*p = m_pOverBarMascotSel->GetRect().y;
	float w;
	const Bitmap* pBmp = g_pFresh->GetBitmap("select_bar_01");
	if (pBmp)
	{
		w = 8.0f;
		WRect rc;
		rc.x = x;
		rc.y = y;
		rc.w = w;
		rc.h = (float)pBmp->Height();
		pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}
	x = w + x;
	float ix = x;
	pBmp = g_pFresh->GetBitmap("select_bar_02");
	if (pBmp)
	{
		w = (float)(count - 1) * 3.0f + (float)count * 24.0f;
		WRect rc;
		rc.x = x;
		rc.y = y;
		rc.w = w;
		rc.h = (float)pBmp->Height();
		pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}
	x = w + x;
	pBmp = g_pFresh->GetBitmap("select_bar_03");
	if (pBmp)
	{
		WRect rc;
		rc.x = x;
		rc.y = y;
		rc.w = 8.0f;
		rc.h = (float)pBmp->Height();
		pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
	}

	m_subSelTypeId = 0;
	y += 7.0f;
	m_subSelItemId = 0;
	for (std::map<unsigned int, sMascotInfo>::const_iterator it =
			 CSharedDoc::Instance()->m_mascotMap.begin();
		it != CSharedDoc::Instance()->m_mascotMap.end(); ++it)
	{
		IFF_ITEM_COMMON* pItem =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(
				(*it).second.tid);
		if (!pItem)
			continue;
		pBmp = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->Icon));
		if (pBmp)
			pDevice->DrawTexture(pBmp, WRect(ix, y, 24.0f, 26.0f), 0xffffffff,
				0);
		WPoint pt(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
		if (WRect(ix, y, 24.0f, 26.0f).IsInRect(pt))
		{
			pBmp = g_pFresh->GetBitmap("select_frame");
			if (pBmp)
				pDevice->DrawTexture(pBmp,
					WRect(ix - 3.0f, y - 3.0f, 30.0f, 32.0f), 0xffffffff, 0);
			if (CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot !=
				(*it).second.guid)
			{
				m_subSelTypeId = (*it).second.tid;
				m_subSelItemId = (*it).second.guid;
			}
		}
		ix += 27.0f;
	}
	if (CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot != 0)
	{
		WRect rc(ix, y, 24.0f, 26.0f);
		pBmp = g_pFresh->GetBitmap("s_no");
		if (pBmp)
			pDevice->DrawTexture(pBmp, rc, 0xffffffff, 0);
		WPoint pt(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
		if (rc.IsInRect(pt))
		{
			pBmp = g_pFresh->GetBitmap("select_frame");
			if (pBmp)
				pDevice->DrawTexture(pBmp,
					WRect(rc.x - 3.0f, rc.y - 3.0f, rc.w + 6.0f, rc.h + 6.0f),
					0xffffffff, 0);
			m_subSelTypeId =
				(*CSharedDoc::Instance()->m_mascotMap.begin()).second.tid;
			m_subSelItemId = 0;
		}
	}
}

void CTaskMain::OnOverBar_SiteInit(int param)
{
	m_pOverBarSite = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pOverBarSite)
	{
		UpdateSiteInfo();
		WRect r = m_pOverBarSite->GetRect();
		r.x += m_offsetX;
		m_pOverBarSite->SetRect(r);
	}
}

void CTaskMain::OnOverBar_OneLineReqBtnInit(int param)
{
	m_pOverBarOneLineReqBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOverBarOneLineReqBtn)
	{
		const WRect& rc = m_pOverBarOneLineReqBtn->GetRect();
		float x = (m_screenWidth < ViewWidth983()) ? rc.x - 140.0f : rc.x + m_offsetX;
		WRect r(x, rc.y, rc.w, rc.h);
		m_pOverBarOneLineReqBtn->SetRect(r);
	}
}

void CTaskMain::OnOverBar_OneLineReqBtnUp()
{
	if (!CSharedDoc::Instance()->IsControlServerService(0x20000))
	{
		char bEnable = 0;
		IActor* pActor = AfxGetTask()->GetActor(s_gatewayActorName);
		pActor << MsgObject(NULL, M983(0x26d), 0x10, (int)&bEnable, 0, 0, 0);
		if (!bEnable)
		{
			pActor << MsgObject(NULL, M983(0x26d), 0x11, 0, 0, 0, 0);
		}
		else
		{
			CloseAllDialogs();
			if (CSharedDoc::Instance()->m_myInfo.info.dwFlagBlock & 0x10)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", NULL);
				pForm->SetTimeLimit(12.0f);
				pForm->SetMessage(
					K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc0\xfc\xb1\xa4\xc6\xc7 \xc0\xcc\xbf\xeb "
					"\xc1\xa6\xc0\xe7\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. "
					"\xc0\xfc\xb1\xa4\xc6\xc7\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 "
					"\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				pForm->Open(NULL, 0);
				return;
			}
			m_pOnelineReqDlg = CreateForm<FrOnelineReqDlg>(
				g_pFresh->GetManager(), this, "oneline_req", NULL);
			m_pOnelineReqDlg->Open(
				(FRESH_PFN_RESULT)&CTaskMain::OnOnelineReqDlgResult, 0x11);
			if (!g_pFresh->GetManager()->IsKeyExclusive())
				g_pFresh->GetManager()->SetExclusiveKey(true);
			if (m_pOverBarOneLineReqBtn)
				m_pOnelineReqDlg->SetIconRect(m_pOverBarOneLineReqBtn->GetRect());
			OnBarHoverOff(0);
		}
	}
	else
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xc0\xfc\xb1\xa4\xc6\xc7 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 "
				 "\xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
	}
}


void CTaskMain::OnOverBar_OneLineViewInit(int param)
{
	m_pOverBarOneLineView = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarOneLineView)
	{
		const WRect& rc = m_pOverBarOneLineView->GetRect();
		float x = rc.x;
		if (m_screenWidth < ViewWidth983())
			x -= 140.0f;
		else
			x += m_offsetX;
		m_pOverBarOneLineView->SetRect(WRect(x, rc.y, rc.w, rc.h));
		COneLineBoard::Instance()->SetRect(m_pOverBarOneLineView->GetRect());
	}
}

void CTaskMain::OnOverBar_OneLineViewOwnerDraw(int)
{
	COneLineBoard::Instance()->Display();
}

void CTaskMain::OnOverBar_OptionInit(int param)
{
	m_pOverBarOption = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOverBarOption)
	{
		m_pOverBarOption->EnableHover(true, 0.0f);
		SetLeftOffset(m_pOverBarOption, m_screenWidth); // 983: 00651ad0
	}
}

void CTaskMain::OnOverBar_OptionUp()
{
	CloseAllDialogs();
	m_pOptionDlg =
		CreateForm<FrOptionDlg>(g_pFresh->GetManager(), this, "option", NULL);
	// 983 00659d30 não chama SetIconRect (no lobby RF m_pOverBarOption é NULL: C114.1)
	m_pOptionDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnOptionDlgResult, 0x11);
	m_pOptionDlg->SetApplyScrSize(
		IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()) ? true : false);
	OnBarHoverOff(0);
}

void CTaskMain::OnOverBar_MyInfoInit(int param)
{
	m_pOverBarMyInfo = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOverBarMyInfo)
	{
		m_pOverBarMyInfo->EnableHover(true, 0.0f);
		SetLeftOffset(m_pOverBarMyInfo, m_screenWidth); // 983: 00651ad0
	}
}

void CTaskMain::OnOverBar_MyInfoUp()
{
	CloseAllDialogs();
	CUserInfo::Instance()->SetInfo(CSharedDoc::Instance()->m_myInfo.info.dwUID,
		CSharedDoc::Instance()->m_myInfo.info.dwGuid, true, true, false, false,
		CSharedDoc::Instance()->m_myInfo.info.sNick);
	OnBarHoverOff(0);
}

bool CTaskMain::OnNotifyDlgResult(int result, FrForm*)
{
	if (result == 1)
	{
		if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/) &&
			!CSharedDoc::Instance()->m_bHaveHalloweenItem)
		{
			CMouseCursor::Instance()->SetActive(false);
			// 983 006572f0: ChangeTask("CShopTask") + 0x88 "CShopWidget"/"TOPPAGE" ao "Lobby"; o 983 nao registra CShopTask na
			// fabrica (CreateObject NULL, AV em 009bf600), entao vai direto a loja pelo caminho da barra. Sem m_sendGiftList.clear do 645
			OnUnderBar_ShopUp();
		}
	}
	return true;
}

void CTaskMain::OnOverBar_PangCookieInit(int param)
{
	m_pOverBarPangCookie = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pOverBarPangCookie)
	{
		// 983 00653540: no lobby o Pang e do RFLobbyDesktop (wnd_pang), sem OWNERDRAW no mapa
		if (g_pFresh->IsCurrentLayout("TOPPAGE"))
			m_pOverBarPangCookie->SetVisible(false);
		SetLeftOffset(m_pOverBarPangCookie, m_screenWidth); // 983: 00651ad0
		m_pPangCookieBmp[0] = NULL;
		m_pPangCookieBmp[1] = NULL;
		m_pPangCookieBmp[0] = g_pFresh->GetBitmap("pang_bar_treecash");
		m_pPangCookieBmp[1] = g_pFresh->GetBitmap("pang_bar_frame_tree");
		m_pPangCookieBmp[2] = g_pFresh->GetBitmap("pang_bar_hancoin");
		m_pPangCookieBmp[3] = g_pFresh->GetBitmap("pang_bar_frame_han");
	}
}

void CTaskMain::OnOverBar_PangCookieOwnerDraw(int)
{
	if (CProjectG::Instance()->HidePrivacy())
		return;
	char buf[32] = { 0 };
	FrArea* pArea = m_pOverBarPangCookie;
	const WRect& rc = pArea->GetRect();
	MakePriceStrA(buf, CSharedDoc::Instance()->m_myInfo.stat.i64Pang);
	Print(rc.x + 113.0f, rc.y + 6.0f, buf, 2.0f, 2, 1.0f, 0xffffd18e);
	__int64 cookie =
		S5::ExchangeOwnCashToCookie(CSharedDoc::Instance()->m_cash, 0, 100);
	if (cookie < 0)
		cookie = 0;
	MakePriceStrA(buf, cookie);
	if (CSharedDoc::Instance()->m_bonusCash != 0)
	{
		char bonus[32] = { 0 };
		strcat(buf, "/");
		MakePriceStrA(bonus, CSharedDoc::Instance()->m_bonusCash);
		strcat(buf, bonus);
	}
	Print(rc.x + 218.0f, rc.y + 6.0f, buf, 2.0f, 2, 1.0f, 0xffffd18e);
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	const Bitmap* pBmp;
	const Bitmap* pHover;
	switch (CLoginInfo::Instance()->ProvType())
	{
	case 2:
		pBmp = m_pPangCookieBmp[0];
		pHover = m_pPangCookieBmp[1];
		break;
	case 4:
		pBmp = m_pPangCookieBmp[2];
		pHover = m_pPangCookieBmp[3];
		break;
	default:
		return;
	}
	if (pBmp && pHover)
	{
		float w = (float)pBmp->Width();
		unsigned h = pBmp->Height();
		WRect r;
		r.y = rc.y + 3.0f;
		r.x = rc.Right() - (w + 5.0f);
		r.w = w;
		r.h = (float)h;
		pDevice->DrawTexture(pBmp, r, 0xffffffff, 0);
		r.x = r.x - 100.0f;
		r.w = r.w + 150.0f;
		if (CMouseCursor::Instance()->InArea(r))
		{
			WRect r2(r.x - 75.0f, r.y + 10.0f, (float)pHover->Width(),
				(float)pHover->Height());
			pDevice->DrawTexture(pHover, r2, 0xffffffff, 0);
			MakePriceStrA(buf, CSharedDoc::Instance()->m_cash);
			Print(r.x + 75.0f, r.y + 14.0f, buf, 2.0f, 2, 1.0f, 0xffffd18e);
		}
	}
}

void CTaskMain::OnBarHoverOn(int param)
{
	m_hoverBtn = (int)DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_hoverBtn)
	{
		for (int i = 0; i < 0x17; i++) // 983: + Combine e Point Shop
		{
			if (m_hoverBtn == (int)(&m_pUnderBarBack)[i])
			{
				CIconToolTip::GetInstance()->SetToolTip((eBarButton)i);
				return;
			}
		}
	}
}

void CTaskMain::OnBarHoverOff(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pBtn && (int)pBtn != m_hoverBtn)
		return;
	m_hoverBtn = 0;
	CIconToolTip::GetInstance()->ClearToolTip();
}

void CTaskMain::OnBarTooltipOwnerDraw(int)
{
	CIconToolTip::GetInstance()->Render();
}

void CTaskMain::OnOverBar_SpecialCardInit(int param)
{
	m_pOverBarSpecialCard = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOverBarSpecialCard)
	{
		m_pOverBarSpecialCard->EnableHover(true, 0.0f);
		m_pOverBarSpecialCard->EnableBlink(
			CCardManager::Instance()->ShowSPCardAlarm());
		SetLeftOffset(m_pOverBarSpecialCard, m_screenWidth); // 983: 00651ad0
	}
}

// 983 00656d40 (A315, out/F5/cartas/a315/dz_00656d40.txt): como o 00656be0 do lobby (rflobbydesktop.cpp) -- sem o
// SetVisible(false)/SetFixed do 645; abre centrado em x sobre o botao (preso na largura da tela), logo abaixo dele, Open modo 5;
// sem cartao o aviso 0xcc passa pelo K2L_Compatibility; 00806700 = UpdatePremium983 (linha do premium 0x1a100002, A319).
void CTaskMain::OnOverBar_SpecialCardUp()
{
	if (!m_pSpecialCardDlg)
	{
		m_pOverBarSpecialCard->EnableBlink(false);
		CCardManager::Instance()->SetSPCardAlarm(false);
		m_pSpecialCardDlg = CreateForm<FrSPCardBuffDlg>(g_pFresh->GetManager(),
			this, "spcardbuff", NULL);
		CCardManager::Instance()->SetCardBuffDlg(m_pSpecialCardDlg);
		CItemBuff::UpdateBuffDlg(m_pSpecialCardDlg);
		m_pSpecialCardDlg->UpdatePremium983();
		const WRect& button = m_pOverBarSpecialCard->GetRect();
		float w = m_pSpecialCardDlg->GetRect().w;
		WPoint pos(button.x - w * 0.5f, button.y + button.h);
		float screenW = (float)g_view->GetWidth();
		if (screenW < pos.x + w)
			pos.x -= (w + pos.x) - screenW;
		m_pSpecialCardDlg->EnableDrag(false);
		m_pSpecialCardDlg->Open(
			(FRESH_PFN_RESULT)&CTaskMain::OnSpecialCardFormResult, pos, 5);
		if (m_pSpecialCardDlg->GetSpecialCardNum() < 1)
		{
			m_pSpecialCardDlg->Close(FrCANCEL, true);
			WPoint msgPos(m_pOverBarSpecialCard->GetRect().x - 86.0f,
				m_pMessengerBubbleEdit->GetRect().y);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xa8),
				(int)K2L_Compatibility("\xbb\xe7\xbf\xeb\xc1\xdf\xc0\xce \xc8\xbf\xb0\xfa\xb0\xa1 "
					 "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				1, (int)&msgPos, 0, 0));
		}
		else
		{
			m_pSpecialCardDlg->SetVisible(true);
		}
	}
	else
	{
		m_pSpecialCardDlg->Close(FrCANCEL, true);
	}
	OnBarHoverOff(0);
}


void CTaskMain::OnOverBar_MessengerInit(int param)
{
	m_pOverBarMessenger = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOverBarMessenger)
	{
		m_pOverBarMessenger->EnableHover(true, 0.0f);
		CMessengerInfo::Instance()->m_alarmRect =
			m_pOverBarMessenger->GetRect();
		if (!CSharedDoc::Instance()->m_bMessengerInit)
			m_pOverBarMessenger->EnableBlink(false);
		SetLeftOffset(m_pOverBarMessenger, m_screenWidth); // 983: 00651ad0
	}
}

void CTaskMain::OnOverBar_MessengerUp()
{
	if (0) // HACK: keeps the literal keeper of OnOverBar_SpecialCardUp emitted
		((std::list<sMailInfoBrief>*)0)->clear();
	// 983 0065b7d0: o mesmo botao trata os avisos: mensageiro (doc+0x10d81), tesouro (doc+0x11038 == 1, com doc+0x10d82 = 0),
	// presente (doc+0x10d80, 00659df0) e, sem aviso, abre o mensageiro (00653ab0)
	CSharedDoc* doc = CSharedDoc::Instance();
	if (doc->m_bMessengerAlarm)
	{
		OnMessengerAlarmBtnUp();
		OnBarHoverOff(0);
		return;
	}
	if (!doc->m_bMessengerInit)
	{
		if ((int)doc->m_toppageFeature == 1)
		{
			OnTreasureAlarmUp();
			OnBarHoverOff(0);
			return;
		}
		if (doc->m_bRefreshCamera)
		{
			RFSceneManager::Instance()->SetInputBlocked983(true);
			FrGiftAlarmDlg* pDlg = CreateForm<FrGiftAlarmDlg>(g_pFresh->GetManager(), this, "gift_alarm", NULL);
			pDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnNotifyDlgResult, 0x11);
			pDlg->SetTopmost(true);
			pDlg->FindNextTopFocus(false);
			OnBarHoverOff(0);
			return;
		}
	}
	CloseAllDialogs();
	CMessengerInfo::Instance()->Open(true);
	OnBarHoverOff(0);
}

void CTaskMain::OnTreasureAlarmUp()
{
	// 983 00656fe0: botao opcional; com o dialogo aberto o aviso passa a 3 (doc+0x11038), o que devolve o
	// btn_messenger ao normal (005fc520) e faz o proximo clique abrir o mensageiro, nao o tesouro de novo
	if (m_pTreasureAlarm)
	{
		m_pTreasureAlarm->SetVisible(false);
		m_pTreasureAlarm->Enable(false);
	}
	CTHunter::Instance()->SetUsedTikiReport(false);
	CTreasureAlarmDlg* pDlg = CreateForm<CTreasureAlarmDlg>(
		g_pFresh->GetManager(), this, "treasure_alarm", NULL);
	if (pDlg)
	{
		pDlg->Open(NULL, 0x11);
		pDlg->SetTopmost(true);
		pDlg->FindNextTopFocus(false);
		pDlg->SetIconRect(pDlg->GetRect());
		CSharedDoc::Instance()->m_toppageFeature = 3;
	}
	__rtti_obj = NULL;
	OnBarHoverOff(0);
}

void CTaskMain::OnTreasureAlarmInit(int param)
{
	m_pTreasureAlarm = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pTreasureAlarm && !CTHunter::Instance()->IsUsedTikiReport())
	{
		m_pTreasureAlarm->SetVisible(false);
		m_pTreasureAlarm->Enable(false);
		SetLeftOffset(m_pTreasureAlarm, m_screenWidth);
	}
}

void CTaskMain::OnMessengerBubbleInit(int param)
{
	m_pMessengerBubble = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	SetLeftOffset(m_pMessengerBubble, m_screenWidth); // 983: 00651ad0
	for (int i = 0; i < 3; i++)
		m_pMessengerBubbleBmp[i] =
			g_pFresh->GetBitmap(MakeStr("login_%02d", i));
}


void CTaskMain::OnMessengerBubbleOwnerDraw(int)
{
	if (!m_pMessengerBubbleEdit || !m_pMessengerBubbleEdit->IsVisible())
		return;
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (!pDevice)
		return;
	if (m_pMessengerBubbleEdit->GetLineNum() == 0)
		return;

	WRect r = m_pMessengerBubbleEdit->GetRect();
	_WRECT bubble;
	bubble.x = r.x - 2.0f;
	bubble.y = r.y - 8.0f;
	bubble.h = (float)m_pMessengerBubbleEdit->GetLineNum() * 14.0f;
	const Bitmap** ppBmp = m_pMessengerBubbleBmp;
	if (m_pMessengerBubbleBmp[0])
	{
		unsigned bh = ppBmp[0]->Height();
		unsigned bw = ppBmp[0]->Width();
		WRect rc(bubble.x, bubble.y, (float)bw, (float)bh);
		pDevice->DrawTexture(ppBmp[0], rc, 0xffffffff, 0);
		bubble.y += (float)ppBmp[0]->Height();
	}
	if (m_pMessengerBubbleBmp[1])
	{
		unsigned bw = ppBmp[1]->Width();
		WRect rc(bubble.x, bubble.y, (float)bw, bubble.h);
		pDevice->DrawTexture(ppBmp[1], rc, 0xffffffff, 0);
		bubble.y += bubble.h;
	}
	if (m_pMessengerBubbleBmp[2])
	{
		unsigned bh = ppBmp[2]->Height();
		unsigned bw = ppBmp[2]->Width();
		WRect rc(bubble.x, bubble.y, (float)bw, (float)bh);
		pDevice->DrawTexture(ppBmp[2], rc, 0xffffffff, 0);
	}
}

void CTaskMain::OnMessengerBubbleEditInit(int param)
{
	m_pMessengerBubbleEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pMessengerBubbleEdit->SetVisible(false);
	SetLeftOffset(m_pMessengerBubbleEdit, m_screenWidth); // 983: 00651ad0
}

void CTaskMain::OnMessengerAlarmBtnInit(int param)
{
	m_pMessengerAlarmBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	SetLeftOffset(m_pMessengerAlarmBtn, m_screenWidth); // 983: 00651ad0
	if (m_pMessengerAlarmBtn && !CSharedDoc::Instance()->m_bMessengerAlarm)
		m_pMessengerAlarmBtn->SetVisible(false);
}

void CTaskMain::OnMessengerAlarmBtnUp()
{
	CMessengerInfo::Instance()->AlarmBtnUp(0xffffffff);
}

void CTaskMain::OnMessengerGiftAlarmBtnInit(int param)
{
	m_pMessengerGiftAlarmBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pMessengerGiftAlarmBtn->SetVisible(false);
	SetLeftOffset(m_pMessengerGiftAlarmBtn, m_screenWidth); // 983: 00651ad0
}

void CTaskMain::OnMessengerGiftAlarmBtnUp()
{
	m_pMessengerGiftAlarmBtn->SetVisible(false);
	FrGiftAlarmDlg* pDlg = CreateForm<FrGiftAlarmDlg>(g_pFresh->GetManager(),
		this, "gift_alarm", NULL);
	pDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnNotifyDlgResult, 0x11);
	pDlg->SetTopmost(true);
	pDlg->FindNextTopFocus(false);
	pDlg->MoveWindow(*(const WPoint*)&m_pMessengerGiftAlarmBtn->GetRect());
	OnBarHoverOff(0);
}


void CTaskMain::CloseAllDialogs()
{
	if (m_pNoticeDlg)
		m_pNoticeDlg->Close(true);
	if (m_pDlg150)
		m_pDlg150->Close(true);
	// 983 (0065ced0): também os diálogos novos
	if (m_pCardComposeDlg983)
		m_pCardComposeDlg983->Close(true);
	if (m_pAttendanceDlg983)
		m_pAttendanceDlg983->Close(true);
	if (m_pBongdariShop983)
		m_pBongdariShop983->Close(true);
	if (m_pAlchemyDlg983)
		m_pAlchemyDlg983->Close(true);
	if (!m_levelupForms.empty())
	{
		for (unsigned int i = 0; i < m_levelupForms.size(); i++)
		{
			if (m_levelupForms[i])
			{
				m_levelupForms[i]->Close(FrNONE, true);
				m_levelupForms[i] = NULL;
			}
		}
		m_levelupForms.clear();
	}
}

bool CTaskMain::OnWaitDlgResult(int result, FrForm*)
{
	if (result == 2)
	{
		CMouseCursor::Instance()->SetActive(false);
		AfxGetTask()->CloseWidget983(); // 983 006570c2
		if (!IS_KINDOF(CLobbyTask, AfxGetTask()))
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
				false);
		PostMsg(this, "Lobby", 0, (int)"LOGIN", 0, 0, 0);
	}
	m_pWaitDlg = NULL;
	return true;
}

bool CTaskMain::OnServerDlgResult(int, FrForm*)
{
	m_pServerDlg = NULL;
	if (IS_EXACTKINDOF(CLobbyTask, AfxGetTask()) &&
		CSharedDoc::Instance()->m_bTutorialResume)
		GetActor("Lobby") << MsgObject(NULL, M983(0x56), 0, 0, 0, 0, 0);
	return true;
}

// 983 (00657200): "exit"/"logout" fora das telas de login marcam o FrLogoutDlg (+0x120/+0x121, A109) e fecham; aqui cria o
// "attendance_dlg" (check-out: +0x130 = 1; +0x12c 0 fecha o jogo, 1 volta ao login) e pede a presença ao servidor (0x16e).
// Quem abre a janela é a resposta (msg 0x327 -> SetReward983); o fim vem pelo OnAttendanceDlgResult983 (00651270).
bool CTaskMain::OnQuitDlgResult(int, FrForm*)
{
	if (m_pQuitDlg)
	{
		bool bExit = m_pQuitDlg->m_bExitRequested983;
		bool bLogout = m_pQuitDlg->m_bLogoutRequested983;
		if (bExit || bLogout)
		{
			m_pAttendanceDlg983 = CreateForm<FrAttendanceDlg>(g_pFresh->GetManager(), this,
				"attendance_dlg", NULL);
			if (m_pAttendanceDlg983)
			{
				if (bExit)
					m_pAttendanceDlg983->m_exitAction983 = 0;
				else if (bLogout)
					m_pAttendanceDlg983->m_exitAction983 = 1;
				m_pAttendanceDlg983->m_checkMode983 = 1;
				WSendPacket pkt((enumClientPacket)0x16e);
				pkt.Send(TO_GAME);
			}
		}
	}
	m_pQuitDlg = NULL;
	return true;
}

bool CTaskMain::OnOptionDlgResult(int, FrForm*)
{
	m_pOptionDlg = NULL;
	return true;
}

// 983 (00651270): "attendance_dlg" fechada pelo botão de saída (FrCANCEL): 1 volta ao login, 0 fecha o jogo
bool CTaskMain::OnAttendanceDlgResult983(int ret, FrForm*)
{
	if (ret == FrCANCEL)
	{
		if (m_pAttendanceDlg983->m_exitAction983 == 1)
			m_pAttendanceDlg983->DoLogout983();
		else if (m_pAttendanceDlg983->m_exitAction983 == 0)
			g_bQuit = true;
	}

	m_pAttendanceDlg983 = NULL;
	return true;
}

bool CTaskMain::OnQuitNotifyResult983(int, FrForm*) // 00a62d90
{
	g_bQuit = true;
	return true;
}

bool CTaskMain::OnNotifyOneDlgResult(int, FrForm*)
{
	m_pNotifyOneDlg = NULL;
	return true;
}

// 983: no lobby modal (00e78cac == 1), quando nao sobra nenhuma das tres janelas (troca, alquimia de cartas, reciclagem),
// devolve o avatar ao estado normal (tira o bit 0x20) e religa o ntAvatarManager
static __forceinline void RestoreLobbyAvatar983()
{
	ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
	if (pMyAvatar)
		pMyAvatar->SetState(pMyAvatar->m_state983 & ~0x20);
	ntAvatarManager::m_pInstance->SetActive(true);
}

// 983 (006525a0): fim da "AlchemyDlg"
bool CTaskMain::OnAlchemyDlgResult983(int, FrForm*)
{
	m_pAlchemyDlg983 = NULL;
	if (g_lobbyModal983 == 1 && m_pCardComposeDlg983 == NULL && m_pRecycleDlg == NULL
		&& ntAvatarManager::m_pInstance)
		RestoreLobbyAvatar983();
	return true;
}

// 983 (00653ae0): fim da "card_compose"
bool CTaskMain::OnCardComposeDlgResult983(int, FrForm*)
{
	m_pCardComposeDlg983 = NULL;
	if (g_lobbyModal983 == 1 && m_pAlchemyDlg983 == NULL && m_pRecycleDlg == NULL
		&& ntAvatarManager::m_pInstance)
		RestoreLobbyAvatar983();
	return true;
}

// 983 (006573c0): igual a Alchemy/card_compose, a ultima das 3 janelas do lobby modal restaura o avatar
bool CTaskMain::OnRecycleDlgResult(int, FrForm*)
{
	m_pRecycleDlg = NULL;
	if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		ntAvatarManager::m_pInstance->SetActive(true);
	if (g_lobbyModal983 == 1 && m_pAlchemyDlg983 == NULL && m_pCardComposeDlg983 == NULL
		&& ntAvatarManager::m_pInstance)
		RestoreLobbyAvatar983();
	return true;
}

bool CTaskMain::OnTikiMagicBoxDlgResult(int, FrForm*)
{
	m_pTikiMagicBoxDlg = NULL;
	if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		ntAvatarManager::m_pInstance->SetActive(true);
	return true;
}

bool CTaskMain::OnBongdariDlgResult(int, FrForm*)
{
	// 983 006574e0 (D819): zera +0x194 ao fechar; sem isso o CloseAllDialogs seguinte chama Close no diálogo liberado (queda dono10 14:25)
	if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		ntAvatarManager::m_pInstance->SetActive(true);
	// 983 0065753a: 0xe1 sempre ao ator principal (o CRealMyRoomTask do 645 nunca existe; a atualizacao se perdia)
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xe1), 0, 0, 0, 0, 0));
	m_pBongdariShop983 = NULL;
	return true;
}

bool CTaskMain::OnScratchDlgResult(int, FrForm*)
{
	m_pScratchDlg = NULL;
	if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		ntAvatarManager::m_pInstance->SetActive(true);
	return true;
}

bool CTaskMain::OnLoginRsDlgResult(int, FrForm*)
{
	m_pLoginRsDlg = NULL;
	return true;
}

bool CTaskMain::OnMaketListDlgResult(int, FrForm*)
{
	m_pMaketDlg = NULL;
	return true;
}

// 983 (0065c2d0, C-PKT lote 25): resultado do "write_post" - e o CRealMyRoomMain::OnPostSendDlgResult do 645 (0xc3) mudado
// para o CTaskMain, com o pacote 0x145 na ordem do 983 (id do destinatario como texto, taxa em Encode8, itens de 0x37).
// Destinatario pelos acessores do post_senddlg.h (C32, A s102; lote 28: eram deslocamentos crus).
bool CTaskMain::OnPostSendDlgResult983(int result, FrForm*)
{
	if (result == 1)
	{
		if (m_pPostSendDlg983)
		{
			WSendPacket packet((enumClientPacket)0x145);
			packet.Encode4(CSharedDoc::Instance()->m_myInfo.info.dwUID);
			packet.Encode4(m_pPostSendDlg983->GetSendUID()); // +0x168
			packet.EncodeStr(m_pPostSendDlg983->GetReceiverId983()); // 0065bf30 (+0x14c)
			packet.EncodeStr(std::string(""));
			packet.EncodeStr(m_pPostSendDlg983->GetLetterMsg());
			packet.Encode8(m_pPostSendDlg983->GetSendServiceCharge());
			packet.Encode1((unsigned char)m_pPostSendDlg983->GetSendListSize());
			std::list<sMailIncludeItem>* pList = m_pPostSendDlg983->GetSendList();
			for (std::list<sMailIncludeItem>::iterator it = pList->begin();
				 it != pList->end(); ++it)
			{
				// 983: registro de 0x37 com so os 5 primeiros campos (GetSendList 0080f0e0)
				sMailIncludeItem983 item;
				memset(&item, 0, sizeof(item));
				item.dwIDX = it->dwIDX;
				item.dwTID = it->dwTID;
				item.btItemType = it->btItemType;
				item.iCount = it->iCount;
				item.iTimeCount = it->iTimeCount;
				packet.EncodeBuffer(&item, sizeof(item));
			}
			packet.Send(TO_GAME);
			m_pPostSendDlg983 = NULL;
		}
	}
	else
	{
		m_pPostReciveDlg983 = NULL;
		m_pPostSendDlg983 = NULL;
		m_pPostReciveDlg983 = CreateForm<CPost_ReciveDlg>(g_pFresh->GetManager(),
			this, "postbox", NULL);
		m_pPostReciveDlg983->Open(
			(FRESH_PFN_RESULT)&CTaskMain::OnPostReciveDlgResult983, 0x81);
	}
	return true;
}

// 983 (00651070, C-PKT lote 25): resultado do "postbox".
bool CTaskMain::OnPostReciveDlgResult983(int, FrForm*)
{
	m_pPostReciveDlg983 = NULL;
	return true;
}

// 983 (00653be0, C-PKT lote 25): resultado do "equip" - e o CLobbyMain::OnItemEquipResult do 645 mudado para o CTaskMain;
// no 983 o pacote 0x20 (tipo 2) sai ANTES da mensagem 1.
bool CTaskMain::OnItemEquipResult983(int result, FrForm*)
{
	if (result == 1 || result == 2)
	{
		if (m_pEquipDlg983)
			memcpy(m_myItemSlot983, m_pEquipDlg983->GetMyItemSlot(),
				sizeof(m_myItemSlot983));
		m_pEquipDlg983 = NULL;

		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(2);
		packet.EncodeBuffer(m_myItemSlot983, sizeof(m_myItemSlot983));
		packet.Send(TO_GAME);

		HandleMsg(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}
	return true;
}

// 983 (00657600, C-PKT lote 28): abre o "equip" - e o CLobbyMain::OnGameRoom_EquipItemDown do 645 mudado para o CTaskMain
// (sem o InitControlsSelectedUserInfo); chamado pela mensagem 0x81 do HandleMsgCommon e pelo CLobbyMain::HandleMsg do 983
// (mensagem 0x8b, param1 5: 00abcc38). O resultado 00653be0 manda o 0x20 tipo 2.
void CTaskMain::OpenEquipDlg983()
{
	if (m_pEquipDlg983 == NULL)
	{
		m_pEquipDlg983 =
			CreateForm<FrEquipDlg>(g_pFresh->GetManager(), this, "equip", NULL);
		CSharedDoc* pDoc = CSharedDoc::Instance();
		if ((bool)((pDoc->m_myInfo.info.dwIdentity >> 1) & 1))
			m_pEquipDlg983->Init(pDoc->m_userInfo[0].userEquip.tidItemSlot, true);
		else
			m_pEquipDlg983->Init(pDoc->m_myInfo.userEquip.tidItemSlot, true);
		m_pEquipDlg983->Open((FRESH_PFN_RESULT)&CTaskMain::OnItemEquipResult983, 3);
	}
}

// 983 (0065c510, C-PKT lote 28): resultado do "rmr_notify" do caso 0x2a3 (carta enviada; "escrever outra?") - sim reabre o
// "write_post" (resultado 0065c2d0 = pacote 0x145).
bool CTaskMain::OnPostSendDlg2Result983(int result, FrForm*)
{
	m_pPostSendDlg983 = NULL;
	if (result == 1)
	{
		m_pPostSendDlg983 = CreateForm<CPost_SendDlg>(g_pFresh->GetManager(),
			this, "write_post", NULL);
		m_pPostSendDlg983->Open(
			(FRESH_PFN_RESULT)&CTaskMain::OnPostSendDlgResult983, 0x81);
	}
	return true;
}

bool CTaskMain::OnOnelineReqDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		const char* line = ((FrOnelineReqDlg*)pForm)->GetInputString();
		if (CSharedDoc::Instance()->m_chatManager.Filtering((char*)line))
		{
			FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			form->SetMessage(K2L_Compatibility("\xc0\xfb\xb4\xe7\xc7\xcf\xc1\xf6\xbe\xca\xc0\xba "
							 "\xb4\xdc\xbe\xee\xb0\xa1 \xc6\xf7\xc7\xd4\xb5\xc7"
							 "\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."), // JP 983 (0065cfe0): K2L
				false);
			form->Open(NULL, 3);
			m_pOnelineReqDlg = NULL;
			return true;
		}
		if (*line == '\0')
		{
			FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			form->SetMessage(
				K2L_Compatibility("\xba\xb8\xb3\xbe \xb3\xbb\xbf\xeb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			form->Open(NULL, 3);
			m_pOnelineReqDlg = NULL;
			return true;
		}
		COneLineBoard::Instance()->SetMessage(line);
		WSendPacket pkt((enumClientPacket)0x67);
		pkt.Send(TO_GAME);
	}
	m_pOnelineReqDlg = NULL;
	return true;
}

bool CTaskMain::OnNoticeDlgResult(int result, FrForm*)
{
	// JP 983 (006512c0; PKT lote 31): sem a escolha de loja, o 0x3d e o ramo do UCC do 645; so a pagina do JP no aviso 0x65
	if (result == 1)
	{
		if (IsLocalCountry(1) && m_pNoticeDlg->GetTypeId() == 0x65)
			ShellExecuteA(NULL, NULL, "http://www.pangya.jp", NULL, NULL, 3);
	}
	m_pNoticeDlg = NULL;
	return true;
}



bool CTaskMain::OnKbdManualDlgResult(int, FrForm*)
{
	m_pKbdManualDlg = NULL;
	return true;
}

bool CTaskMain::OnLevelupFormResult(int, FrForm*)
{
	if (0) // HACK: keeps the inline naked OnOverBar_MascotSelOwnerDraw emitted (nakedify)
		((CTaskMain*)0)->OnOverBar_MascotSelOwnerDraw(*(int*)0);
	if (0) // HACK: keeps the literal keeper of OnOverBar_MascotSelOwnerDraw emitted
		((std::list<sMapEventInfo>*)0)->clear();
	if (0) // HACK: keeps the inline naked OnOverBar_ClubSelOwnerDraw emitted (nakedify)
		((CTaskMain*)0)->OnOverBar_ClubSelOwnerDraw(*(int*)0);
	if (0) // HACK: keeps the literal keeper of OnOverBar_ClubSelOwnerDraw emitted
		((std::list<sGuildMatchFlagInfo>*)0)->clear();
	if (0) // HACK: keeps the inline naked OnOverBar_MascotOwnerDraw emitted (nakedify)
		((CTaskMain*)0)->OnOverBar_MascotOwnerDraw(*(int*)0);
	if (0) // HACK: keeps the inline naked OnMessengerGiftAlarmBtnUp emitted (nakedify)
		((CTaskMain*)0)->OnMessengerGiftAlarmBtnUp();
	if (0) // HACK: keeps the literal keeper of OnMessengerGiftAlarmBtnUp emitted
		((std::list<sSlotInfo>*)0)->clear();
	if (!m_levelupForms.empty())
	{
		m_levelupForms[m_levelupForms.size() - 1] = NULL;
		m_levelupForms.erase(m_levelupForms.end() - 1);
	}
	return true;
}

bool CTaskMain::OnOnelineReqChkOutDlgResult(int result, FrForm*)
{
	if (result == 1)
	{
		WSendPacket pkt(0x66);
		pkt.EncodeStr(COneLineBoard::Instance()->GetMessageA());
		pkt.Send(TO_GAME);
	}
	return true;
}


bool CTaskMain::OnSpecialCardFormResult(int, FrForm*)
{
	CCardManager::Instance()->SetCardBuffDlg(NULL);
	m_pSpecialCardDlg = NULL;
	return true;
}

float CTaskMain::Print(float x, float y, const char* text, float spacing,
	int align, float scale, unsigned long color)
{
	if (!text || strlen(text) == 0)
		return 0.0f;

	float oldScale = m_pTitleFont->GetScale();
	const char* charSet = m_pTitleFont->GetFontInfo()->pCharSet;
	m_pTitleFont->SetScale(scale);

	float offset = 0.0f;
	if (align == 1)
	{
		for (const char* p = text; *p; p++)
		{
			offset += m_pTitleFont->GetCharWidth(g_view,
						  strchr(charSet, *p) - charSet) +
				spacing * scale;
		}
		offset = (offset - spacing) * 0.5f;
	}
	else if (align == 2)
	{
		for (const char* p = text; *p; p++)
		{
			offset += m_pTitleFont->GetCharWidth(g_view,
						  strchr(charSet, *p) - charSet) +
				spacing * scale;
		}
		offset = offset - spacing;
	}

	float cx = x - offset;
	for (const char* p = text; *p; p++)
	{
		cx += m_pTitleFont->PutChar(g_view, cx, y,
				  strchr(charSet, *p) - charSet, 0, color) +
			spacing;
	}
	m_pTitleFont->SetScale(oldScale);
	m_pTitleFont->Flush(g_view);

	if (align == 0)
		return cx - x;
	if (align > 0 && align < 3)
		return offset;
	return x;
}

char const* CTaskMain::MakeStatusString(int a, int b, int c)
{
	if (0) // HACK: emission order: the original compiles it one wave later
		SetBGMNone();
	const char* names[5] = { "\xc6\xc4\xbf\xf6", "\xc4\xc1\xc6\xae\xb7\xd1",
		"\xc1\xa4\xc8\xae\xb5\xb5", "\xbd\xba\xc7\xc9", "\xc4\xbf\xba\xea" };
	if (b != 0)
	{
		if (c != 0)
			return MakeStr("%s+%d/%s+%d", names[a], b, "\xbd\xbd\xb7\xd4", c);
		return MakeStr("%s+%d", names[a], b);
	}
	if (c != 0)
		return MakeStr("%s%s+%d", names[a], "\xbd\xbd\xb7\xd4", c);
	return "";
}

char const* CTaskMain::MakeStatusString2(int a, int b, int c)
{
	if (a == 0)
	{
		if (b != 0)
			return MakeStr("%d", b);
		return "";
	}
	if (b != 0)
	{
		if (c != 0)
			return MakeStr("%d(-%d)/%d", a, c);
		return MakeStr("%d/%d", a, b);
	}
	return MakeStr("%d", a);
}

char const* CTaskMain::MakeStatusString3(int a, int b, int c)
{
	if (c != 0)
		return MakeStr("%d(-%d)/%d", a, c, b);
	return MakeStr("%d/%d", a, b);
}

char const* CTaskMain::MakeCardStatusString(int a, int b)
{
	if (0) // HACK: emission order: the original compiles it one wave later
		SetBGMNone();
	const char* names[5] = { K2L_Compatibility("\xc6\xc4\xbf\xf6\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xbd\xba\xc7\xc9\xbd\xbd\xb7\xd4"),
		K2L_Compatibility("\xc4\xbf\xba\xea\xbd\xbd\xb7\xd4") };
	if (b != 0)
		return MakeStr("%s+%d", names[a], b);
	return "";
}

void CTaskMain::SetBGMNone()
{
	if (s_bgmPlaying)
	{
		g_audio->StopBGM(true);
		s_bgmPlaying = 0;
	}
}

void CTaskMain::SetRandomBGM()
{
	if (s_bgmPlaying != 1)
	{
		g_audio->SetFadeOutTime(1.0f);
		g_audio->PlayBGM(s_randomBGMList[rand() % 2], false, true);
		s_bgmPlaying = 1;
	}
}

void CTaskMain::SetTimeVariableBGM()
{
	if (s_bgmPlaying != 2)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		g_audio->SetFadeOutTime(1.0f);
		if (st.wHour == 22 || st.wHour == 23)
			g_audio->PlayBGM("tea_time_vibra.mp3", false, true);
		else
			g_audio->PlayBGM("tea_time_piano.mp3", false, true);
		s_bgmPlaying = 2;
	}
}

void CTaskMain::SetConstantBGM(char const* name, bool bLoop, bool bForce)
{
	if (s_bgmPlaying != 3 || bForce)
	{
		g_audio->SetFadeOutTime(1.0f);
		g_audio->PlayBGM(name, bLoop, true);
		s_bgmPlaying = 3;
	}
}

void CTaskMain::TurnOn3DBackground()
{
	if (0) // HACK: keeps the literal keeper of OnNoticeDlgResult emitted
		((std::list<sQuest>*)0)->clear();
	if (0) // HACK: emission order: the original compiles it one wave later
		GetBtnState(0, (eBarButton)0);
	CSharedDoc::Instance()->m_golfGame.weather = 0;
	if (CSceneManager::Instance())
		CSceneManager::Instance()->SetLobbyWeather(false);
	CSharedDoc::Instance()->m_bBackgroundVideo = false;
}

void CTaskMain::TurnOff3DBackground()
{
	if (0) // HACK: emission order: the original compiles it one wave later
		GetBtnState(0, (eBarButton)0);
	CSharedDoc::Instance()->m_bBackgroundVideo = true;
}

void CTaskMain::CheckNaviButtons()
{
	if (m_pUnderBarBack)
		m_pUnderBarBack->Enable(
			CTaskManager::Instance()->IsNaviBackwardEnable());
	if (m_pUnderBarFront)
		m_pUnderBarFront->Enable(
			CTaskManager::Instance()->IsNaviForwardEnable());
}

// 983 (00657eb0): funcao livre __fastcall (ecx = botao, edx = mensagem, pilha = bool). O 983 nao tem mais os botoes
// "messenger_alarm"/"gift_alarm": as msgs 0xcd/0xce/0xd1 (645 0xa9/0xaa/0xac) trocam as imagens do proprio botao do
// mensageiro (CTaskMain +0x4c, CGolfRule +0x13c). Prioridade dos avisos: +0x10d81, +0x10d82, (0), +0x10d80; com on, a
// mensagem so olha os avisos acima dela. Tabela 00e14ff8 (linha 4 = sem imagem de baixo).
static const char* s_messengerBtnImg983[5][4] = {
	{ "pin_btn_n", "pin_btn_o", "pin_btn_n", "pin_btn_on_2" },
	{ "pin_btn_on_1", "pin_btn_on_1", "pin_btn_on_1", "pin_btn_on_2" },
	{ "gift_btn_on_1", "gift_btn_on_1", "gift_btn_on_1", "gift_btn_on_2" },
	{ "pin_btn_on_1", "pin_btn_on_1", "pin_btn_on_1", "pin_btn_on_2" },
	{ NULL, NULL, NULL, NULL },
};

void __fastcall UpdateMessengerBtn983(FrButton* btn, int msgId, bool on)
{
	if (!btn)
		return;
	CSharedDoc* doc = CSharedDoc::Instance();
	unsigned char alarm[5] = { doc->m_bMessengerAlarm, doc->m_bMessengerInit, 0, doc->m_bRefreshCamera, 1 };
	int n = 5;
	if (on)
	{
		if (msgId == 0xcd)
			n = 0;
		else if (msgId == 0xd1)
			n = 1;
		else if (msgId == 0xce)
			n = 3;
	}
	int i = 0;
	for (; i < n; i++)
		if (alarm[i])
			break;
	int img, below;
	bool blink;
	switch (i)
	{
	case 0: img = 3; below = 1; blink = true; break;
	case 1: img = 1; below = 4; blink = true; break;
	case 2: img = 3; below = 2; blink = true; break;
	case 3: img = 2; below = 4; blink = true; break;
	default: img = 0; below = 4; blink = false; break;
	}
	for (int m = 0; m < 4; m++)
	{
		btn->SetButtonImg(s_messengerBtnImg983[img][m], (FrButton::eButMode)m);
		btn->SetButtonBelowImg(s_messengerBtnImg983[below][m], (FrButton::eButMode)m);
	}
	btn->EnableBlink(blink);
}

// 983 0065df0e (msg 0x27) + 006577c0: texto do ErrorCodeInfo.iff, tipo 1 = sem aviso, tipo 3 com texto = aviso que encerra
// o jogo (00a62d90) antes do aviso comum; sem registro, GameString 0x66
void CTaskMain::NotifyServerError983(unsigned long code)
{
	IFF983::sErrorCodeInfo* pInfo = Doc()->m_itemManager.FindErrorCodeInfo983(code);
	const char* text;
	if (pInfo == NULL)
		text = GetGameString(0x66);
	else
	{
		text = (const char*)&pInfo->raw[4];
		int kind = *(int*)&pInfo->raw[0]; // +8
		if (kind == 1)
			return;
		if (kind == 3)
		{
			if (strlen(text) > 1)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
				pForm->SetMessage(text, false);
				pForm->Open((FRESH_PFN_RESULT)&CTaskMain::OnQuitNotifyResult983, 3);
			}
			else
				g_bQuit = true;
		}
	}
	std::string str(text);
	str.append("\n\\c0xff888888\\c", 0xf);
	str += MakeStr("\n\nCode : %d", code);
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
	pForm->SetMessage(str.c_str(), false);
	pForm->Open(NULL, 0x803);
}

void CTaskMain::HandleMsgCommon(const MsgObject& msg)
{
	if (0) // HACK: keeps the inline USERINFODLG emitted (nakedify)
		USERINFODLG();
	if (0) // HACK: keeps the CreateForm<FrOptionDlg> used by a naked body instantiated (nakedify)
		CreateForm<FrOptionDlg>(0, 0, 0, 0);
	if (0) // HACK: keeps the CreateForm<FrGiftAlarmDlg> used by a naked body instantiated (nakedify)
		CreateForm<FrGiftAlarmDlg>(0, 0, 0, 0);
	if (0) // HACK: keeps the CreateForm<FrOnelineReqDlg> used by a naked body instantiated (nakedify)
		CreateForm<FrOnelineReqDlg>(0, 0, 0, 0);
	if (0) // HACK: keeps the CreateForm<FrTikiMagicBoxDlg> used by a naked body instantiated (nakedify)
		CreateForm<FrTikiMagicBoxDlg>(0, 0, 0, 0);
	if (0) // HACK: keeps the CreateForm<FrScratchDlg> used by a naked body instantiated (nakedify)
		CreateForm<FrScratchDlg>(0, 0, 0, 0);
	if (0) // HACK: keeps the CreateForm<FrRecycleDlg> used by a naked body instantiated (nakedify)
		CreateForm<FrRecycleDlg>(0, 0, 0, 0);
	if (0) // HACK: keeps the inline naked OnUnderBar_MagicBoxUp emitted (nakedify)
		((CTaskMain*)0)->OnUnderBar_MagicBoxUp();
	if (0) // HACK: keeps the literal keeper of OnUnderBar_MagicBoxUp emitted
		((std::list<sBurnningSpCard>*)0)->clear();
	if (0) // HACK: keeps the inline naked OnUnderBar_ScratchUp emitted (nakedify)
		((CTaskMain*)0)->OnUnderBar_ScratchUp();
	if (0) // HACK: keeps the literal keeper of OnUnderBar_ScratchUp emitted
		((std::list<sRecordedItemInfo>*)0)->clear();
	if (0) // HACK: keeps the inline naked OnOverBar_OneLineReqBtnUp emitted (nakedify)
		((CTaskMain*)0)->OnOverBar_OneLineReqBtnUp();
	if (0) // HACK: keeps the literal keeper of OnOverBar_OneLineReqBtnUp emitted
		((std::list<sSCardAvilityPeriodInfo>*)0)->clear();
	if (0) // HACK: keeps the inline COMDATs of the original emitted (naked bodies carry them inlined)
	{
		WPoint p0;
		WPoint p1(p0);
		Buddy();
		LOGININFO();
		LOGINPW();
		MESSENGER();
		TOOLTIP();
		AfxPostMsg(0, 0, 0, 0, 0, 0, 0);
		AfxGetPreservedTask();
		OfflinePlay();
		MyOutsideCash();
		GalleryMode();
		MyGuid(false);
		WSingleton<ntAvatarManager>::Instance();
		WSingleton<CUserInfo>::IsInstantiated();
		WSingleton<CQuitWindow>::IsInstantiated();
		((FrUserInfoForm*)0)->IsOpenAddFriendDlg();
		((COption*)0)->gSetUnderExtBtn(0);
		((FrGiftDlg*)0)->UpdateFriendList();
		((FrTabButton*)0)->GetItemUnderCursor();
		((FrGraphicInterface*)0)->Line(p0, p1, 0, 0, 0);
		((sPangYaUserInfo*)0)->IsIdentity(0);
		((CTask*)0)->GetMainActor();
		((CSharedDoc*)0)->SetRmrMode(0);
		((CSharedDoc*)0)->GetRmrMode();
		((CSharedDoc*)0)->GetFlagBlock();
		((CSharedDoc*)0)->IsHaveHalloweenItem();
		((CSharedDoc*)0)->GetPlayingMode();
		((CSharedDoc*)0)->SetPlayingMode((eReplayModeType)0);
		((CSharedDoc*)0)->GetNowGoWithMode();
		((CSharedDoc*)0)->GetGoWithFriendUID();
	}
	// 983 0065d913: toda mensagem vai antes ao RFLobbyDesktop (00e78ca8, HandleMsg983 = slot 35, +0x8c): 0x56/0x57 abrem e
	// fecham o "Loading..." do lobby, 0x53/0x54 os modais da RF etc.
	if (CLobbyState983::ms_pInstance)
	{
		typedef void (__thiscall* HandleMsg983Fn)(CLobbyState983*, const MsgObject&);
		((HandleMsg983Fn)(*(void***)CLobbyState983::ms_pInstance)[35])(CLobbyState983::ms_pInstance, msg);
	}
	switch (msg.message)
	{
	case M983(0x1):
		m_fKbdManualTime = 0.0f;
		return;
	case M983(0x2):
		m_fKbdManualTime = -30.0f;
		if (m_pWaitDlg)
		{
			m_pWaitDlg->Close((eFormRet)1, true);
			m_pWaitDlg = NULL;
		}
		return;
	case M983(0x19):
		if (m_pServerDlg)
			m_pServerDlg->BuildServerList();
		return;
	case M983(0x1b):
		if (m_pServerDlg)
			m_pServerDlg->BuildChannelList();
		return;
	case 0x1d: // 983 0065dc2d (taskmanager 009c59d7..): erro do servidor no login religa a lista de canais da janela do servidor
		if (FrWnd* pWnd = g_pFresh->GetManager()->GetDesktop()->FindChildForm("server"))
			if (FrServerDlg* pDlg = DYNAMIC_CAST(FrServerDlg, pWnd))
				pDlg->EnableChannelList983();
		return;
	case 0x27: // 983 0065df0e -> ErrorCodeInfo.iff: aviso do erro do servidor com o codigo em cinza
		NotifyServerError983((unsigned long)msg.param1);
		return;
	case 0x4f: // 983 0065d968 (FrGameRoomDlg ao mudar o Ready): liga/desliga os visores da barra de cima
	{
		bool bEnable = msg.param1 != 0;
		if ((unsigned)(Doc()->m_roomInfo.realGameType - 20) < 2)
		{
			_gateway::_grandprix::CGrandPrixDocument* pDoc = NULL;
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);
			if (pDoc)
			{
				sGPRoomItem983* pItem = pDoc->FindRoomItem983(Doc()->m_roomInfo.gpTypeId983);
				if (pItem && ((pItem->flags >> 4) & 1))
					bEnable = false;
			}
		}
		if (m_pOverBarChar)
			m_pOverBarChar->Enable(bEnable);
		if (m_pOverBarCaddie)
			m_pOverBarCaddie->Enable(bEnable);
		if (m_pOverBarClub)
			m_pOverBarClub->Enable(bEnable);
		if (m_pOverBarAztec)
			m_pOverBarAztec->Enable(bEnable);
		if (m_pOverBarMascot)
			m_pOverBarMascot->Enable(bEnable);
		if (m_pOverBarEqItem)
			m_pOverBarEqItem->Enable(bEnable);
		CloseAllSubSelection();
		if (!bEnable && m_pEquipDlg983)
			m_pEquipDlg983->Close((eFormRet)1, true);
		return;
	}
	case 0x50: // 983 0065dab4 (FrGameRoomDlg::InitControlsSelectedUserInfo 00503580): no 645 a sala zerava estes direto
		m_subSelTypeId = 0;
		m_subSelItemId = 0;
		return;
	case 0x51: // 983 0065da68: fim do tempo do resultado da sala fecha as janelas de level up
		if (!m_levelupForms.empty())
		{
			for (size_t i = 0; i < m_levelupForms.size(); i++)
				if (m_levelupForms[i])
					m_levelupForms[i]->Close((eFormRet)1, true);
			m_levelupForms.clear();
		}
		return;
	case 0x8a: // 983 0065dba7
		OnOverBar_OptionUp();
		return;
	case 0x17e: // 983 0065e770: ao voltar da partida o tempo do cenario volta ao do param1 (taskmanager 009c4ab9/009d65da)
		if (CSceneManager::IsInstantiated())
			CSceneManager::Instance()->SetWeather((unsigned char)msg.param1, false);
		return;
	case 0x1b2: // 983 0065e793 (A171; so do 983): o login aceitou o servidor escolhido (pacote 0x3, lobbytask.cpp) ->
		// a janela do servidor conecta no OnProc (estado 1 -> ConnectToServer); sem a janela, reconecta o servidor de jogo
		if (m_pServerDlg)
		{
			m_pServerDlg->SetState((gsLoginState_t)1);
		}
		else
		{
			if (NET()->IsConnected(WNetworkSystem::NET_GAME))
				NET()->ForceShutDown(WNetworkSystem::NET_GAME);
			NET()->SendMessage(WNetworkSystem::NET_GAME, (NetworkUnit::eEvent)0);
		}
		return;
	case M983(0x23):
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage((const char*)msg.param1, false);
		// 983 0065dedb: Open(NULL, 0x803) e nada depois (sem SetTopmost/FindNextTopFocus); o 3 deixava o veu escuro (C372.1)
		pForm->Open(NULL, 0x803);
		return;
	}
	case M983(0x24):
		if (m_pNotifyOneDlg)
			return;
		m_pNotifyOneDlg =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		m_pNotifyOneDlg->SetMessage((const char*)msg.param1, false);
		m_pNotifyOneDlg->Open(
			(FRESH_PFN_RESULT)&CTaskMain::OnNotifyOneDlgResult, 0x803);
		return;
	case M983(0x26):
	{
		unsigned long itemId = msg.param2;
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(itemId);
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_item", NULL);
		if (!pForm)
			return;
		FrArea* pIcon =
			DYNAMIC_CAST(FrArea, pForm->FindChildByName("itemicon"));
		FrEdit* pName =
			DYNAMIC_CAST(FrEdit, pForm->FindChildByName("itemname"));
		if (pIcon)
		{
			if (pItem)
				pIcon->SetBgImg(pItem->Icon);
			else if (itemId)
			{
				WRect rc = pIcon->GetRect();
				pIcon->SetRect(WRect(rc.x + 15.0f, rc.y + 7.0f, rc.w, rc.h));
				pIcon->SetBgImg("item1_16");
			}
			else
				pIcon->SetBgImg("hide");
		}
		if (pName)
		{
			if (pItem)
				pName->AddText(pItem->Name, false, true);
			else if (itemId)
				pName->AddText(MakeStr(K2L_Compatibility("%d\xc6\xce"), itemId), false, true);
			else
				pName->AddText("none", false, true);
			pName->AddText("\n", false, true);
		}
		pForm->SetMessage((const char*)msg.param1, false);
		pForm->Open(NULL, 0x802); // 983 0065ded6
		return;
	}
	case M983(0x48):
		// D620: 983 0065dff6 = lista de amigos do CPost_SendDlg (+0x1a4, amigo em +0x110), nao do FrGiftDlg;
		// o RAW 645 m_pGiftDlg+0x244 caia em outro membro (nosso m_pFriendDlg do gift = +0x274) e chamava UpdateFriendList em lixo
		if (IsLocalContent((localContentType_t)0x57) && m_pPostSendDlg983 &&
			m_pPostSendDlg983->GetFriendDlg983())
			m_pPostSendDlg983->GetFriendDlg983()->UpdateFriendList();
		return;
	case 0x4d: // 983 0065e0b5: a janela do presente e do ator principal; +0x150 = m_pGiftDlg, que o caso M983(0x50) preenche
		m_pGiftDlg = CreateForm<FrGiftDlg>(g_pFresh->GetManager(), this, "send_gift", NULL);
		m_pGiftDlg->SendGift(*(const IFF_ITEM_COMMON*)msg.param1,
			Doc()->m_myInfo.stat.i64Pang, Doc()->m_cookie);
		m_pGiftDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnGiftDlgResult983, 0x81);
		return;
	case M983(0x50):
		if (m_pGiftDlg)
			m_pGiftDlg->SetReceiverInfo(msg.param1,
				*(sPangYaUserInfo*)msg.param2);
		// D620: 983 0065e042..0065e064 tambem preenche o destinatario do correio (FUN_0080e4f0 = CPost_SendDlg::SetReceiverInfo)
		if (IsLocalContent((localContentType_t)0x57) && m_pPostSendDlg983)
			m_pPostSendDlg983->SetReceiverInfo(msg.param1,
				*(sPangYaUserInfo*)msg.param2);
		if (CUserInfo::Instance()->IsVisible() == true && CUserInfo::Instance())
		{
			// D620: 983 0065e087 testa form+0x5d20 (= nosso m_pAddFriendDlg) e le dwUID em info+0x129; RAW 645 era +0x2554/+0x109
			FrUserInfoForm* pForm = CUserInfo::Instance()->GetDlg();
			if (pForm && pForm->IsOpenAddFriendDlg())
				pForm->SetRequestFriendData(((sPangYaUserInfo*)msg.param2)->sNick,
					((sPangYaUserInfo*)msg.param2)->dwUID);
		}
		return;
	case 0x52:
	{
		// D848: 983 0065dadc (so do 983): os icones cur_caddie/cur_club/cur_aztec/cur_mascot do FrGameRoomDlg mandam a 0x52
		// (param1 = WRect* do "..._sel", param2 = grupo, param3 = modo); sem este caso o Gears da sala nao abria a barra
		const WRect& sel = *(const WRect*)msg.param1;
		WPoint pos(sel.x, sel.y - 5.0f);
		OpenSubEquipBar(pos, (PANGYA_ITEM_GROUP)msg.param2, msg.param3);
		if (msg.param3 == 1 && m_pSubEquipBar)
		{
			WRect barRect = m_pSubEquipBar->GetRect();
			barRect.x = sel.x - barRect.w * 0.5f + 0.5f;
			if (barRect.x < 5.0f)
				barRect.x = 5.0f;
			if (barRect.x + barRect.w > g_view->GetWidth())
				barRect.x = g_view->GetWidth() - barRect.w;
			m_pSubEquipBar->SetRect(barRect);
		}
		return;
	}
	case M983(0x51):
		if (CUserInfo::Instance())
		{
			FrUserInfoForm* pForm = CUserInfo::Instance()->GetDlg();
			if (pForm && CUserInfo::Instance()->IsVisible() == true &&
				pForm->IsOpenAddFriendDlg()) // D620: 983 +0x5d20 (RAW 645 +0x2554)
				pForm->SetRequestFriendData((const char*)msg.param1,
					msg.param2);
		}
		// no break in the original: falls through into the caddie_info case
	case M983(0x5c):
	{
		FrCaddieInfoDlg* pDlg = (FrCaddieInfoDlg*)g_pFresh->GetManager()
									->GetDesktop()
									->FindChildByName("caddie_info");
		if (!pDlg)
		{
			__rtti_obj = NULL;
			return;
		}
		pDlg->EndContract(msg.param1);
		return;
	}
	case M983(0xa8):
	{
		if (!m_pMessengerBubbleEdit)
			return;
		WPoint pos;
		if (msg.param2 != 0)
		{
			const char* text = (const char*)msg.param1;
			if (msg.param3 != 0)
			{
				CMessengerInfo::Instance()->Notify2(text);
				pos.x = ((WPoint*)msg.param3)->x;
				pos.y = ((WPoint*)msg.param3)->y;
				m_pMessengerBubbleEdit->MoveWindow(pos);
				return;
			}
			pos.x = CMessengerInfo::Instance()->m_alarmRect.x - 86.0f;
			pos.y = m_pMessengerBubbleEdit->GetRect().y;
			m_pMessengerBubbleEdit->MoveWindow(pos);
			m_pMessengerBubbleEdit->ClearLine();
			m_pMessengerBubbleEdit->AddText(text, false, true);
			m_pMessengerBubbleEdit->SetVisible(true);
			return;
		}
		m_pMessengerBubbleEdit->SetVisible(false);
		return;
	}
	case M983(0xa9):
	case M983(0xaa):
	case M983(0xac):
		// 983 (0065e27d): os tres pelo botao do mensageiro (+0x4c); sem os botoes de alarme nem o teste do CTutorialTask
		UpdateMessengerBtn983(m_pOverBarMessenger, msg.message, msg.param1 != 0);
		return;
	case M983(0x6c):
		if (!m_pTreasureAlarm)
			return;
		if (msg.param1 == 1)
			m_pTreasureAlarm->SetVisible(true);
		else
			m_pTreasureAlarm->SetVisible(false);
		if (msg.param1 == 1)
		{
			m_pTreasureAlarm->Enable(true);
			m_pTreasureAlarm->SetVisible(true);
			return;
		}
		m_pTreasureAlarm->SetVisible(false);
		m_pTreasureAlarm->Enable(false);
		return;
	case M983(0x5d):
	{
		COneLineBoard::Instance()->SetWaitMsgNum(msg.param1);
		COneLineBoard::Instance()->SetWaitMsgTime(msg.param2);
		if (msg.param1 >= 0x400)
		{
			this << MsgObject(this, M983(0x23),
				(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbf\xb9\xbe\xe0\xb5\xc8 \xb8\xde\xbd\xc3\xc1\xf6\xb0\xa1 \xb3\xca\xb9\xab \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9. \n\xc0\xe1\xbd\xc3\xc8\xc4\xbf\xa1 \xb4\xd9\xbd\xc3 \xc7\xd8\xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0."),
				0, 0, 0, 0);
			return;
		}
		FrOnelineReqChkOutDlg* pDlg = CreateForm<FrOnelineReqChkOutDlg>(
			g_pFresh->GetManager(), this, "oneline_chkout", NULL);
		if (!pDlg)
			return;
		pDlg->Open((FRESH_PFN_RESULT)&CTaskMain::OnOnelineReqChkOutDlgResult,
			0x11);
		return;
	}
	case M983(0x1c):
		this << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
		CSharedDoc::Instance()->m_curChannel =
			CSharedDoc::Instance()->m_selChannel;
		CSharedDoc::Instance()->m_ignoreList.clear();
		UpdateSiteInfo();
		if (!g_pFresh->IsCurrentLayout("SERVERLIST"))
		{
			// 983 0065e58f: troca de canal ja no lobby: lista Grand Prix do servidor 0x10 reabre e as abas se refazem
			if (DOCJP()->GetState983() == 2 && GetServerType983() == 0x10)
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x8b, 4, 0, 0, 0, 0));
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x8c, 0, 0, 0, 0, 0));
			return;
		}
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0,
			0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", M983(0x35), 0, 0, 0, 0);
		return;
	case M983(0x20):
		if (m_pServerDlg)
		{
			// 983 0065e61e: resultado = param1; 2 com m_provType 4 fecha o jogo (g_bQuit); o 645 fechava sempre com 1
			m_pServerDlg->Close((eFormRet)msg.param1, true);
			if (msg.param1 == 2 && CSharedDoc::Instance()->m_provType == 4)
				g_bQuit = true;
			m_pServerDlg = NULL;
		}
		return;
	case M983(0x22):
		if (m_pServerDlg)
			m_pServerDlg->SetLoginMessageAlert((const char*)msg.param1);
		return;
	case M983(0xad): // 983 0065e2fa (case 0xd2, pacote 0xd5): cupom de Papel Shop (tipo 2) nao sai da lista aqui
	{
		unsigned long guid = (unsigned long)msg.param1;
		std::list<sItemInfo>& items = CSharedDoc::Instance()->m_myItemList;
		std::list<sItemInfo>::iterator it;
		for (it = items.begin(); it != items.end(); ++it)
		{
			if ((*it).guid == guid)
				break;
		}
		if (it == items.end() || !(*it).tid)
			return;
		int kind = ItemManager()->GetCouponKind((*it).tid);
		if (kind == 2)
			return;
		if ((*it).ItemType == 0 || kind == 3)
		{
			(*it).Common[0]--;
			if ((*it).Common[0] > 0)
				return;
		}
		else if (kind == 0 && (*it).HourRemain > 0)
			return;
		items.erase(it);
		return;
	}
	case M983(0xae):
		CMessengerInfo::Instance()->Close(true);
		return;
	case M983(0xea):
		OnUnderBar_ServerUp();
		return;
	case M983(0xeb):
		UpdateSiteInfo();
		return;
	case M983(0xec):
		OpenWaitedLayout();
		return;
	case 0x276: // 983 0065e9b4 (M983(0x1fa) ficava em aberto e o taskmanager manda 0x276 cru: nunca chegava)
	{
		for (int i = 0; i < 6; i++)
		{
			unsigned long tid =
				CSharedDoc::Instance()->m_myInfo.userEquip.tidSkin[i];
			if ((tid & 0xfc000000) == 0x38000000 && (tid & 0x3c00000) != 0x1800000)
			{
				std::list<sItemInfo>::iterator it;
				for (it = CSharedDoc::Instance()->m_cutinList.begin();
					it != CSharedDoc::Instance()->m_cutinList.end(); ++it)
				{
					if ((*it).tid == tid)
					{
						if ((*it).ItemType != 0) // 983 0065e9e5
							(*it).ItemType = 2;
						break;
					}
				}
			}
		}
		return;
	}
	case 0x277: // 983 0065ebd4 -> 006579a0 ("information")
		OpenInformation(msg.param1);
		return;
	case M983(0x200):
	{
		CloseAllDialogs();
		FrBongdariShopDlg* pShop = CreateForm<FrBongdariShopDlg>(
			g_pFresh->GetManager(), this, "bongdari_shop", NULL);
		if (!pShop)
			return;
		pShop->Open((FRESH_PFN_RESULT)&CTaskMain::OnBongdariDlgResult, 0x41);
		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
			ntAvatarManager::m_pInstance->SetActive(false);
		return;
	}
	case M983(0x12d):
	{
		FrLevelupItemForm* pForm = CreateForm<FrLevelupItemForm>(
			g_pFresh->GetManager(), this, "levelupitem", NULL);
		if (!pForm)
			return;
		pForm->SetData((sLevelUpDone*)msg.param1);
		pForm->Open((FRESH_PFN_RESULT)&CTaskMain::OnLevelupFormResult, 3);
		pForm->SetTopmost(true); // 983 0065e74c..0065e759
		pForm->FindNextTopFocus(false);
		m_levelupForms.push_back(pForm);
		return;
	}
	case M983(0x216):
	{
		if (CSharedDoc::Instance()->m_goWithMode == 1)
		{
			if (CSharedDoc::Instance()->m_goWithFriendUID == -1)
				return;
			sFriend* pFriend =
				BuddyMGR->GetBuddy(CSharedDoc::Instance()->m_goWithFriendUID);
			if (!pFriend)
				return;
			CSharedDoc::Instance()->SetDirectMoveRoomInfo(
				pFriend->userPosition.iChannelUid,
				pFriend->userPosition.iRoomIdx,
				pFriend->userPosition.iServerGUID);
		}
		sGameServerInfo server;
		server.name[0] = 0;
		server.addr[0] = 0;
		unsigned long uid = 0xffffffff;
		unsigned char type = 0xff;
		unsigned short room = 0xffff;
		CSharedDoc::Instance()->GetDirectMoveRoomInfo(&type, &room, &uid);
		for (std::list<sGameServerInfo>::iterator it =
				 CSharedDoc::Instance()->m_gameServerList.begin();
			it != CSharedDoc::Instance()->m_gameServerList.end(); ++it)
		{
			server = *it;
			if (server.id == uid)
				break;
		}
		if (server.maxUser == 0)
			return;
		if (CSharedDoc::Instance()->m_goWithMode == 1)
		{
			if (type == 0xff)
				return;
		}
		else if (CSharedDoc::Instance()->m_goWithMode == 2 ||
			CSharedDoc::Instance()->m_goWithMode == 3)
		{
			if (type == 0xff)
				return;
			if (room == 0xffff)
				return;
		}
		CSharedDoc::Instance()->m_gameServerAddr = server.addr;
		CSharedDoc::Instance()->m_gameServerPort = server.port;
		CSharedDoc::Instance()->m_gameServerUID = server.id;
		// JP 983 (0065ed80; PKT lote 21): sem derrubar e religar a rede aqui; pede a troca ao servidor (0x119 com o id,
		// como o serverdlg 00af4ce0) e a resposta faz o resto
		CSharedDoc::Instance()->m_curGameServer = server;
		CSharedDoc::Instance()->SetDirectMoveRoomInfo(type, room, uid);
		{
			WSendPacket pkt((enumClientPacket)0x119);
			pkt.Encode4(server.id);
			pkt.Send(TO_GAME);
		}
		return;
	}
	case M983(0x270):
		CInitDelegate::ExecuteInitFunctions();
		return;
	case M983(0x271):
		CInitDelegate::ExecuteReleaseFunctions();
		return;
	case M983(0x222):
		if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
			MakeRealMyRoom();
		return;
	case M983(0x21f):
		if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		{
			CSharedDoc* pDoc = CSharedDoc::Instance();
			if (pDoc->m_rmrAuthority.ownerUID == pDoc->m_myInfo.info.dwUID ||
				!pDoc->m_rmrAuthority.bPrivate)
				ChangeRealMyRoom();
			else
			{
				int result = 0;
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x231), (int)&result, 2, 3, 0, 0));
				if (result == 1)
				{
					CloseAllDialogs();
					if (!m_pRMREnterSetting)
					{
						m_pRMREnterSetting =
							CreateForm<CRealMyRoom_EnterSetting>(
								g_pFresh->GetManager(), this, "enter_setting",
								NULL);
						FrForm::EnableTail(false);
						m_pRMREnterSetting->Open(
							(FRESH_PFN_RESULT)&CTaskMain::OnRMR_ResultSetting,
							0x81);
						m_pRMREnterSetting->SetFadeout(false);
					}
				}
				else
					this << MsgObject(this, M983(0x23),
						(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc0\xaf\xc0\xfa\xb0\xa1 \xb4\xeb\xb1\xe2\xb9\xe6\xbf\xa1 \xc0\xd6\xbe\xee \xb4\xd9\xb8\xa5 \xc0\xaf\xc0\xfa\xc0\xc7 \xb9\xe6\xbf\xa1 \xb9\xe6\xb9\xae\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0);
			}
		}
		return;
	// 983 (0065e7f5..0065e8f0, C-PKT lote 25): o correio do 983 e do CTaskMain (no 645 estes casos eram do
	// CRealMyRoomMain::HandleMsg): escrever carta, lista e leitura do "postbox".
	case M983(0x221):
		if (IsLocalContent((localContentType_t)0x57 /*983*/))
		{
			m_pPostSendDlg983 = NULL;
			m_pPostSendDlg983 = CreateForm<CPost_SendDlg>(g_pFresh->GetManager(),
				this, "write_post", NULL);
			FrForm::ms_bHasTail = false;
			m_pPostSendDlg983->Open(
				(FRESH_PFN_RESULT)&CTaskMain::OnPostSendDlgResult983, 0x81);
			m_pPostSendDlg983->SetFadeout(false);
		}
		return;
	case M983(0x223):
		if (IsLocalContent((localContentType_t)0x57 /*983*/) && m_pPostReciveDlg983 != NULL)
			m_pPostReciveDlg983->SetLoadPostList();
		return;
	// 983 0065f144 (C167.1): religa os botoes de receber da caixa de entrada
	case 0x2bf:
		if (IsLocalContent((localContentType_t)0x57 /*983*/) && m_pPostReciveDlg983 != NULL)
			m_pPostReciveDlg983->EnableMiniBtns(msg.param1 != 0);
		return;
	// 983 0065f1cf/0065f1f3/0065f211 (A236): lista de cartas, "recebendo a pagina?" (param1 = bool*) e resposta de cada 0x146
	// da pagina (mandadas pelo caso 0x214 do OnPacketCommon)
	case 0x2c0:
		if (m_pPostReciveDlg983 != NULL)
			m_pPostReciveDlg983->EnablePostList983(msg.param1 != 0);
		return;
	case 0x321:
		if (m_pPostReciveDlg983 != NULL)
			*(bool*)msg.param1 = m_pPostReciveDlg983->IsReceivingPage983();
		return;
	// 983 0065f188/0065f199 (D638): botao de busca da Loja/My Room abre a busca de itens; 0x2d9 fecha (00651040)
	case 0x2d8:
		CreateMaketDlg();
		return;
	case 0x2d9:
		if (m_pMaketDlg)
			m_pMaketDlg->Close(true);
		return;
	case 0x322:
		if (m_pPostReciveDlg983 != NULL)
			m_pPostReciveDlg983->OnReceivePageItemResult983(msg.param1 != 0);
		return;
	case M983(0x224):
		if (IsLocalContent((localContentType_t)0x57 /*983*/) && m_pPostReciveDlg983 != NULL)
			m_pPostReciveDlg983->OnPostDescDlgCreate();
		return;
	case M983(0x226):
		if (IsLocalContent((localContentType_t)0x57 /*983*/) && m_pPostReciveDlg983 != NULL)
			m_pPostReciveDlg983->OnLoadPostList(); // 983 005b4790 (0x143)
		return;
	// 983 (0065e8f5, C-PKT lote 28): resposta do envio de carta (o caso 0x146 do OnPacketCommon manda 0x2a3 com 0x146 ao ator
	// principal): avisa e pergunta se escreve outra.
	case M983(0x225):
		if (msg.param1 == 0x146)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"rmr_notify", NULL);
			pForm->SetMessage(K2L_Compatibility("\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xc2\xbd\xc0\xb4\xcf\xb4\xd9. \n"
							 "\xc6\xed\xc1\xf6 \xbe\xb2\xb1\xe2\xb8\xa6 \xb0\xe8\xbc\xd3 "
							 "\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"), // 983 00d17378
				false);
			pForm->Open((FRESH_PFN_RESULT)&CTaskMain::OnPostSendDlg2Result983, 2);
		}
		return;
	// 983 (0065e67d, C-PKT lote 28): mensagem 0x81 so do 983 (sem numero no 645): abre o "equip".
	case 0x81:
		OpenEquipDlg983();
		return;
	// 983 (0065f24d..0065f3bb, s111): mensagens so do 983 da "AlchemyDlg" (quebra de MP) e da "card_compose" (alquimia de
	// cartas); a FrRecycleDlg manda 0x2e3/0x324. 0x2e0..0x2e2 = loja de pontos do Tiki (s114, B106).
	case 0x2e0:
		OpenPointShopDlg983();
		return;
	case 0x2e2:
		RefreshPointShopDlg983();
		return;
	case 0x2e3:
		OpenAlchemyDlg983();
		return;
	// 983 (0065f30d, A202): 0x2f0 = FrServerDlg::FailedToConnectServer (00af4600): volta ao servidor de login com a chave
	// (+0x11250 = 1 -> CLoginUnit manda a 0x0B em vez da 0x01 e o OnPacket do lobby limpa a lista de servidores)
	case 0x2f0:
		if (NET()->IsConnected(WNetworkSystem::NET_LOGIN))
			NET()->ForceShutDown(WNetworkSystem::NET_LOGIN);
		*(bool*)&Doc()->m_bReturnToLogin983 = true;
		NET()->SendMessage(WNetworkSystem::NET_LOGIN, (NetworkUnit::eEvent)0);
		return;
	case 0x2e4: // 0065f259
		if (m_pAlchemyDlg983)
			m_pAlchemyDlg983->SetMPValues983(msg.param1, msg.param2);
		return;
	// 0065f2a3: "faltam pontos" / resultado da troca ou da compra (param1 = texto, param2 = 1 sucesso, 0 falha)
	case 0x2e1:
	{
		FrPointShopMessageDlg* pDlg = CreateForm<FrPointShopMessageDlg>(g_pFresh->GetManager(), this,
			"PointShopMessageDlg", NULL);
		pDlg->SetMessage983((const char*)msg.param1);
		pDlg->SetSuccess983(msg.param2);
		pDlg->Open(NULL, 3);
		pDlg->SetTopmost(true); // 983 0065f2ff..0065f303
		return;
	}
	case 0x2e5:
		if (m_pAlchemyDlg983)
			m_pAlchemyDlg983->SetState983(1);
		return;
	case 0x2e6:
		ResetAlchemyDlg983();
		return;
	case 0x324:
		OpenCardComposeDlg983();
		return;
	// 983 (0065f60e): 0x356 (resposta de 0x157 com m_goStopMode, questhandler) abre a janela de maestria se ainda nao aberta
	case 0x356:
		if (RFCharMasteryMainDlg::ms_pInstance == NULL)
		{
			RFCharMasteryMainDlg* dlg = RFCharMasteryMainDlg::Create983(NULL);
			if (dlg)
				dlg->DoModal();
		}
		return;
	// 983 (0065f19e / 0065f1aa): 0x338 abre a "recycledlg"; 0x339 = o botao da barra e o nivel param1 (0..4) na janela
	case 0x338:
		OpenRecycleDlg983();
		return;
	case 0x339:
		OnUnderBar_MagicBoxUp();
		if (m_pRecycleDlg)
			m_pRecycleDlg->SelectLevel983(msg.param1);
		return;
	case 0x325: // param1 = int* raridade da carta nova
	{
		int rarity = *(int*)msg.param1;
		if (m_pCardComposeDlg983)
			m_pCardComposeDlg983->SetSeq983(rarity);
		return;
	}
	case 0x326: // param1 = unsigned long* typeId da carta nova
	{
		unsigned long tid = *(unsigned long*)msg.param1;
		if (m_pCardComposeDlg983 && m_pCardComposeDlg983->SetNewCard983(tid))
			m_pCardComposeDlg983->OpenResult983();
		return;
	}
	// 983 (0065f3c0, s110): mensagem 0x327 so do 983: a resposta da presenca diaria (param1 = bool* "mostrar sempre",
	// param2 = sAttendanceReward983*). Sem janela aberta, cria o "attendance_dlg" (fora do ntAvatarChatTask so com *param1);
	// no ntAvatarChatTask e o check-out da saida da sala (+0x130 = 1, +0x12c = 0, +0x1a6 = 1), nos outros o check-in.
	case 0x327:
	{
		bool bForce = *(bool*)msg.param1;
		sAttendanceReward983 reward = *(const sAttendanceReward983*)msg.param2;
		if (!m_pAttendanceDlg983)
		{
			if (!bForce && !IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
				return;
			m_pAttendanceDlg983 = CreateForm<FrAttendanceDlg>(g_pFresh->GetManager(), this,
				"attendance_dlg", NULL);
			if (m_pAttendanceDlg983)
			{
				if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
				{
					m_pAttendanceDlg983->m_checkMode983 = 1;
					m_pAttendanceDlg983->m_exitAction983 = 0;
					m_pAttendanceDlg983->m_bExitRoom983 = true;
				}
				else
					m_pAttendanceDlg983->m_checkMode983 = 0;
			}
		}
		if (m_pAttendanceDlg983)
		{
			m_pAttendanceDlg983->EnableDrag(false);
			m_pAttendanceDlg983->SetReward983(&reward);
			m_pAttendanceDlg983->Open((FRESH_PFN_RESULT)&CTaskMain::OnAttendanceDlgResult983, 0x11);
		}
		return;
	}
	case 0x328: // 0065f538 (so do 983; pedido C60.2): resultado da bongdari (pacote 0x264, taskmanager.cpp)
	{
		FrWnd* pWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName("bongdari_shop");
		if (!pWnd)
		{
			__rtti_obj = NULL;
			return;
		}
		m_pBongdariShop983 = pWnd;
		std::vector<unsigned int> results(*(const std::vector<unsigned int>*)msg.param1);
		std::vector<unsigned long> typeIds(*(const std::vector<unsigned long>*)msg.param2);
		std::vector<int> quantities(*(const std::vector<int>*)msg.param3);
		((FrBongdariShopDlg*)m_pBongdariShop983)->SetBongdariResult983(results, typeIds, quantities, msg.param4);
		return;
	}
	// 983 0065efe9 (0x2a6, moveis: subcodigos 0x12a..0x12d) e 0065e980 (0x29d, correio: 0x147..0x165). O 645 juntava os dois
	// no 0x228 com os ids de pacote do 645 (0x122..0x15d); os remetentes ja mandam os do 983 -> o aviso abria vazio.
	case M983(0x228):
	case 0x29d:
	{
		static const char* const kPostSendError[] = { // 0x147: resultado 0..0xa
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc6\xed\xc1\xf6 \xbc\xf6\xbd\xc5\xc0\xbb \xb0\xc5\xba\xce\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xc0\xaf\xc0\xfa\xb8\xa6 \xc3\xa3\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc0\xcc\xb9\xcc \xbc\xd2\xc0\xaf\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbc\xd2\xc0\xaf\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb \xbc\xf6\xb7\xae\xba\xb8\xb4\xd9 \xb4\xf5 \xb8\xb9\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xba\xb8\xb3\xbb\xb7\xc1\xb0\xed \xbd\xc3\xb5\xb5\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9..",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbc\xd2\xc0\xaf\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xba\xb8\xb3\xbb\xb7\xc1\xb0\xed \xbd\xc3\xb5\xb5\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc0\xcc\xb9\xcc \xc6\xed\xc1\xf6\xb7\xce \xb9\xde\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc3\xb7\xba\xce\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xc6\xed\xc1\xf6 \xb9\xdf\xbc\xdb\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xc7 \xbf\xec\xc6\xed\xc7\xd4\xc0\xcc \xb0\xa1\xb5\xe6 \xc2\xf7\xbc\xad \xc7\xf6\xc0\xe7\xb4\xc2 \xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xb9\xde\xc0\xbb \xbc\xf6 \xbe\xf8\xb4\xc2 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xc7\xd4 \xb1\xe2\xb4\xc9\xc0\xc7 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.",
			"\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xc7 \xc4\xb3\xb5\xf0\xb0\xa1 \xc8\xde\xb0\xa1\xc1\xdf\xc0\xcc\xb0\xc5\xb3\xaa, \xba\xb8\xc0\xaf\xc7\xd1 \xc4\xb3\xb5\xf0\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
		};
		const char* text = NULL;
		switch (msg.param1)
		{
		case 0x12a: text = msg.param2 ? "\xb1\xc7\xc7\xd1 \xbc\xb3\xc1\xa4 \xbc\xba\xb0\xf8" : "\xb1\xc7\xc7\xd1 \xbc\xb3\xc1\xa4 \xbd\xc7\xc6\xd0"; break;
		case 0x12b: text = "\xb8\xb6\xc0\xcc\xb7\xeb\xbf\xa1 \xb5\xe9\xbe\xee\xb0\xa5 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."; break;
		case 0x12c:
			text = msg.param2 ? "\xc7\xf6\xc0\xe7 \xb9\xe8\xc4\xa1\xb8\xa6 \xc0\xfa\xc0\xe5\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."
				: "\xb8\xb6\xc0\xcc\xb7\xeb\xbf\xa1 \xb0\xa1\xb1\xb8\xb8\xa6 \xb3\xf5\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.. ";
			break;
		case 0x12d: text = "\xb8\xb6\xc0\xcc\xb7\xeb\xbf\xa1 \xc0\xd6\xb4\xc2 \xb0\xa1\xb1\xb8 \xc1\xa4\xba\xb8\xb8\xa6 \xb0\xa1\xc1\xae\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."; break;
		case 0x147:
			if ((unsigned)msg.param2 < 11)
				text = kPostSendError[msg.param2];
			else if (msg.param2 == 0x14)
				text = "\xc3\xb3\xb8\xae\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3\xb8\xb8 \xb1\xe2\xb4\xd9\xb8\xae\xbc\xbc\xbf\xe4.";
			else if (msg.param2 == 0x16)
				text = "\xb8\xde\xc0\xcf \xc3\xb7\xba\xce \xb0\xa1\xb4\xc9\xc7\xd1 \xc3\xd6\xb4\xeb \xb0\xb3\xbc\xf6\xb8\xa6 \xc3\xca\xb0\xfa\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
			else
				text = "\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbb\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
			break;
		case 0x149: text = "\xbf\xec\xc6\xed\xc7\xd4\xc0\xc7 \xc6\xed\xc1\xf6\xb5\xe9\xc0\xbb \xc0\xd0\xbe\xee\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."; break;
		case 0x14b: text = "\xbf\xec\xc6\xed\xb9\xb0\xc0\xc7 \xb3\xbb\xbf\xeb\xc0\xbb \xc0\xd0\xbe\xee\xbf\xc0\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."; break;
		case 0x14d: text = "\xbf\xec\xc6\xed\xb9\xb0\xbf\xa1 \xc3\xb7\xba\xce\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbf\xc5\xb1\xe2\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."; break;
		case 0x165:
			text = msg.param2 == 9 ? "\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc6\xf7\xc7\xd4\xb5\xc7\xbe\xee \xc0\xd6\xbe\xee \xc1\xf6\xbf\xec\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."
				: "\xbf\xec\xc6\xed\xb9\xb0\xc0\xbb \xc1\xf6\xbf\xec\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.";
			break;
		}
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		if (text)
			pForm->SetMessage(K2L_Compatibility(text), false);
		pForm->Open(NULL, msg.message == 0x29d ? 0x800 : 0);
		// 983 0065f0b5: msg 2 a tarefa atual (vtable +0x30 = SendMsgToMainActor); o 645 mandava ao "RealMyRoom"
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		return;
	}
	case M983(0x231):
		if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		{
			unsigned char btn = (unsigned char)msg.param2;
			eBarButton bar = (eBarButton)msg.param3;
			*(int*)msg.param1 = GetBtnState(btn, bar);
		}
		else
			*(char*)msg.param3 = 0;
		return;
	// 983 (s100): sem o caso 0x252 do 645 (missões do aniversário): o CTaskMain do 983 não tem m_pMissionDlg e o exe não
	// tem as strings "BirthdayEventMissionSkin1/2" (o FrBirthdayEventMissionDlg continua, sem chamador direto do taskmain)
	case M983(0x272):
		NotifyServerControlMsg(msg.param1);
		return;
	}
}

// 00651320
bool CTaskMain::OnGiftDlgResult983(int, FrForm*)
{
	m_pGiftDlg = NULL;
	return true;
}

bool CTaskMain::CompletedMission_Result(int, FrForm* pForm)
{
	return true; // 983 (s100): sem o m_pMissionDlg (ver o caso 0x252 do HandleMsgCommon)
}

void CTaskMain::SendRoomExitPacket()
{
	if (0) // HACK: emission order: the original compiles it one wave later
		GetBtnState(0, (eBarButton)0);
	WSendPacket pkt(0xf);
	pkt.Encode1(0);
	pkt.Encode2(0xffff);
	pkt.Encode8(0);
	pkt.Encode8(0);
	pkt.Send(TO_GAME);
	CSharedDoc::Instance()->m_myInfo.roomIndex = 0xffff;
	CSharedDoc::Instance()->m_roomInfo.roomGuid = 0xffff;
	this << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
}

void CTaskMain::OpenWaitedLayout()
{
	if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
	const char* layout = m_waitedLayout.c_str();
	CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)layout, 0, 0, 0);
}

void CTaskMain::SendRoomListExitPacket()
{
	WSendPacket pkt(0x82);
	pkt.Send(TO_GAME);
	MsgObject msg(NULL, 1, 0, 0, 0, 0, 0);
	HandleMsg(msg);
}


void CTaskMain::UpdateSiteInfo()
{
	if (0) // HACK: emission order: the original compiles it one wave later
		GetBtnState(0, (eBarButton)0);
	if (m_pOverBarSite)
	{
		if (CSharedDoc::Instance()->m_offlinePlay)
		{
			m_pOverBarSite->SetCaption("OFFLINE PLAY");
		}
		else if (strlen(CSharedDoc::Instance()->m_curChannel.Name))
		{
			m_pOverBarSite->SetCaption(
				MakeStr("%s > %s", CSharedDoc::Instance()->m_curGameServer.name,
					CSharedDoc::Instance()->m_curChannel.Name));
		}
		else
		{
			m_pOverBarSite->SetCaption(
				CSharedDoc::Instance()->m_curGameServer.name);
		}
	}
}

void CTaskMain::OpenInformation(unsigned long typeId)
{
	FrWnd* pWnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("information");
	if (pWnd)
		return;

	__rtti_obj = NULL;
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "information");

	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeId);

	FrArea* pPortrait =
		DYNAMIC_CAST(FrArea, pForm->FindChildByName("portrait"));
	if (pPortrait)
	{
		if (pItem)
			pPortrait->SetBgImg(pItem->Icon);
		else
			pPortrait->SetBgImg("hide");
	}

	if (pItem)
	{
		FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
		FrArea* pLevel = DYNAMIC_CAST(FrArea, pForm->FindChildByName("level"));
		if (pName && pLevel)
		{
			if (pItem->IsUnderLvl)
			{
				pLevel->SetBgImg(MakeStr("level_%03d", pItem->Level + 1));

				pName->AddText(
					MakeStr(
						K2L_Compatibility("\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:              \xc0\xcc\xc7\xcf"),
						pItem->Name),
					false, true);
			}
			else
			{
				pLevel->SetBgImg(MakeStr("level_%03d", pItem->Level + 1));

				pName->AddText(
					MakeStr(
						K2L_Compatibility("\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:              \xc0\xcc\xbb\xf3"),
						pItem->Name),
					false, true);
			}
		}

		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pItem->TypeId);
		if (pDesc)
			pForm->SetMessage(pDesc->Desc, false);
	}
	else
	{
		FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
		if (pName)
			pName->AddText(
				K2L_Compatibility("\xc0\xcc\xb8\xa7: \xba\xd2\xb8\xed\n\n\xb7\xb9\xba\xa7: \xba\xd2\xb8\xed"),
				false, true);

		pForm->SetMessage(
			K2L_Compatibility("\xba\xa3\xc0\xcf\xbf\xa1 \xbd\xce\xc0\xce \xc1\xa4\xc3\xbc\xba\xd2\xb8\xed\xc0\xc7 \xc1\xb8\xc0\xe7"),
			false);
	}

	pForm->Open(NULL, FrESCAPE | FrENTER);
}

void CTaskMain::MakeRealMyRoom()
{
	// JP 983 (0065b8a0; PKT lote 24): o 0xb5 sai UMA vez e sem as travas de conteudo; o my room e um widget do CLobbyTask
	// ("CMyRoomWidget"), nao o CRealMyRoomTask do 645. Mensagens 0x88 (abre o widget) e 0x55 (layout) so do 983.
	FrTaskForm* pWidget = AfxGetTask()->GetWidgetForm983();
	if (pWidget != NULL && pWidget->GetWindowName983() == "realmyroom")
	{
		FrForm* form =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		form->SetMessage(K2L_Compatibility(
			"\xc7\xf6\xc0\xe7 \xc0\xaf\xc0\xfa\xb0\xa1 \xb4\xd9\xb8\xa5 \xbb\xe7\xb6\xf7\xc0\xc7 \xb9\xe6\xc0\xbb \xb9\xe6\xb9\xae\xc1\xdf\xbf\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xb8\xa5 \xb9\xe6\xc0\xb8\xb7\xce \xc0\xcc\xb5\xbf\xc7\xcf\xb1\xe6 \xbf\xf8\xc7\xcf\xbd\xc3\xb8\xe9 \xb7\xce\xba\xf1\xb7\xce \xc0\xcc\xb5\xbf\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
			false);
		form->Open(NULL, 3);
		return;
	}

	if (CSharedDoc::Instance()->m_playingMode == 3)
		CSharedDoc::Instance()->m_playingMode = (eReplayModeType)1;
	{
		WSendPacket pkt((enumClientPacket)0xb5);
		pkt.Encode4(CSharedDoc::Instance()->m_myInfo.info.dwUID);
		pkt.Encode4(CSharedDoc::Instance()->m_replayTargetUID);
		pkt.Send(TO_GAME);
	}

	switch (CSharedDoc::Instance()->m_rmrMode)
	{
	case 1:
		if (IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
		{
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88 /*so do 983*/,
				(int)"CMyRoomWidget", (int)"TOPPAGE", 0, 0);
			break;
		}
		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		{
			ntAvatarManager::m_pInstance->SetActive(false);
			CTaskManager::Instance()->PostMsg(NULL, "AvatarChat",
				0x88 /*so do 983*/, (int)"CMyRoomWidget", (int)"AVATARCHAT_MAIN", 0,
				0);
			break;
		}
		{
			// o 983 compara o nome com "shopmain" e nao usa o resultado
			FrTaskForm* pForm = AfxGetTask()->GetWidgetForm983();
			if (pForm != NULL)
				(void)(pForm->GetWindowName983() != "shopmain");
		}
		goto toLobby;
	case 4:
		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		{
			CTaskManager::Instance()->PostMsgToMainActor983(NULL,
				0x88 /*so do 983*/, (int)"CMyRoomWidget", (int)"AVATARCHAT_MAIN", 0);
			ntAvatarManager::m_pInstance->SetActive(false);
			break;
		}
	toLobby:
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
			CTaskManager::Instance()->GetPreservedTask() != NULL);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0,
			0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88 /*so do 983*/,
			(int)"CMyRoomWidget", (int)"TOPPAGE", 0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x55 /*so do 983*/,
			(int)"myroom|char", 0, 0, 0);
		break;
	case 5:
		pWidget = AfxGetTask()->GetWidgetForm983();
		if (pWidget != NULL && pWidget->GetWindowName983() == "realmyroom")
		{
			FrForm* form =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
			form->SetMessage(K2L_Compatibility(
				"\xc7\xf6\xc0\xe7 \xc0\xaf\xc0\xfa\xb0\xa1 \xb4\xd9\xb8\xa5 \xbb\xe7\xb6\xf7\xc0\xc7 \xb9\xe6\xc0\xbb \xb9\xe6\xb9\xae\xc1\xdf\xbf\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xb8\xa5 \xb9\xe6\xc0\xb8\xb7\xce \xc0\xcc\xb5\xbf\xc7\xcf\xb1\xe6 \xbf\xf8\xc7\xcf\xbd\xc3\xb8\xe9 \xb7\xce\xba\xf1\xb7\xce \xc0\xcc\xb5\xbf\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
				false);
			form->Open(NULL, 3);
		}
		else
		{
			ChangeRealMyRoom();
		}
		break;
	}

	if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
}


void CTaskMain::ChangeRealMyRoom()
{
	// JP 983 (00657cd0; PKT lote 21): so os modos 1, 3 e 4 (o 0xb7 sai pelo S5::UTIL::SendRealMyRoomEnter983, 00b2df70);
	// o 983 nao tem o 0xc4 do modo 1 nem o modo 5 (visita com senha pelo CRealMyRoomTask, que o 983 nao tem)
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		return;

	switch (CSharedDoc::Instance()->m_rmrMode)
	{
	case 1:
	{
		S5::UTIL::SendRealMyRoomEnter983(CSharedDoc::Instance()->m_replayTargetUID, true);
		if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
		{
			WSendPacket pkt((enumClientPacket)0xc1);
			pkt.Encode1(2);
			pkt.Send(TO_GAME);
		}
		CSharedDoc::Instance()->m_rmrMode = 0;
		break;
	}
	case 3:
	{
		S5::UTIL::SendRealMyRoomEnter983(CSharedDoc::Instance()->m_replayTargetUID, true);
		if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
		{
			WSendPacket pkt((enumClientPacket)0xc1);
			pkt.Encode1(2);
			pkt.Send(TO_GAME);
		}
		CSharedDoc::Instance()->m_rmrMode = 0;
		break;
	}
	case 4:
	{
		S5::UTIL::SendRealMyRoomEnter983(CSharedDoc::Instance()->m_replayTargetUID, true);
		if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
		{
			WSendPacket pkt((enumClientPacket)0xc1);
			pkt.Encode1(2);
			pkt.Send(TO_GAME);
		}
		CSharedDoc::Instance()->m_rmrMode = 0;
		break;
	}
	}
}



bool CTaskMain::OnRMR_ResultSetting(int result, FrForm* pForm)
{
	if (0) // HACK: keeps the inline naked OnOverBar_OneLineViewInit emitted (nakedify)
		((CTaskMain*)0)->OnOverBar_OneLineViewInit(*(int*)0);
	if (0) // HACK: keeps the literal keeper of OnOverBar_OneLineViewInit emitted
		((std::list<sFurniture_List>*)0)->clear();
	if (0) // HACK: keeps the literal keeper of MakeRealMyRoom/ChangeRealMyRoom emitted
		((std::list<sRoomUserInfo>*)0)->clear();
	if (pForm)
		pForm->EnableTail(true);
	if (result == 1)
	{
		CSharedDoc::Instance()->m_rmrMode = 5;
		ChangeRealMyRoom();
		return true;
	}
	m_pRMREnterSetting = NULL;
	return true;
}

unsigned long CTaskMain::GetCategoryFlag(int cat)
{
	switch (cat)
	{
	case 0:
	case 0xd:
	case 0x16:
		return 0;
	case 1:
	case 0xe:
		return 1;
	case 2:
	case 0xf:
		return 2;
	case 3:
		return 0x10;
	case 4:
		return 0x2c;
	case 5:
		return 0x40;
	default:
		return cat;
	case 0x10:
		return 3;
	case 0x15:
		return 0x3bf;
	case 0x1a:
		return 0x380;
	case 0x1c:
		return 7;
	case 0x1d:
		return 9;
	case 0x1e:
		return 8;
	case 0x22:
		return 6;
	}
}

bool CTaskMain::GetBtnState(unsigned char overBtn, eBarButton underBtn)
{
	if (!IsLocalContent(S4_REAL_MYROOM))
		return false;
	FrWnd* pUnder = (FrWnd*)(&m_pUnderBarBack)[underBtn];
	FrWnd* pOver = (FrWnd*)(&m_pOverBar)[overBtn];
	if (pUnder && pUnder->IsVisible() && pUnder->IsEnabled() && pOver &&
		pOver->IsVisible())
		return pOver->IsEnabled();
	return false;
}
