#include "minatl.h"
#include "clientsetting.h"
#include "logininfo.h"
#include "logindlg.h"
#include "generichttpclient.h"
#include "projectg.h"
#include "shareddoc.h"
#include "frstatic.h"
#include "frdesktop.h"
#include "binstr.h"
#include <process.h>

extern Fresh* g_pFresh;

extern bool g_bQuit;
eLoginState FrLoginDlg::m_connState = (eLoginState)0;
static int s_errorCode983 = 0; // 983 00e390e0
static std::string s_errorText983; // 983 00e390e4
unsigned __stdcall HttpGetNtreevUserAuthKey(void* param);

IMPLEMENT_OBJECT(FrLoginDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrLoginDlg, FrForm)

ON_FRESH_VI("id", FRCMD_INIT, FrLoginDlg::OnIdInit)
ON_FRESH_BI("id", FRCMD_ENTERKEY, FrLoginDlg::OnEnterKey)
ON_FRESH_VI("password", FRCMD_INIT, FrLoginDlg::OnPasswordInit)
ON_FRESH_BI("password", FRCMD_ENTERKEY, FrLoginDlg::OnEnterKey)
ON_FRESH_VI("auto_id", FRCMD_INIT, FrLoginDlg::OnAutoIdInit)
ON_FRESH_VV("auto_id", FRCMD_LBUTTONUP, FrLoginDlg::OnAutoIdBtnUp)
ON_FRESH_VI("view", FRCMD_INIT, FrLoginDlg::OnViewInit)
ON_FRESH_VI("login", FRCMD_INIT, FrLoginDlg::OnLoginInit)
ON_FRESH_VV("login", FRCMD_LBUTTONUP, FrLoginDlg::OnLoginBtnUp)
ON_FRESH_VI("loginfb", FRCMD_INIT, FrLoginDlg::OnLoginFbInit)
ON_FRESH_VV("loginfb", FRCMD_LBUTTONUP, FrLoginDlg::OnLoginFbBtnUp)
ON_FRESH_VI("idbox", FRCMD_INIT, FrLoginDlg::OnHideAreaInit)
ON_FRESH_VI("passwordbox", FRCMD_INIT, FrLoginDlg::OnHideAreaInit)
// 983 (msgmap 00e39100): sem "find"; o site abre pelo "join_jp" e o "quit" fecha o jogo; "join" so guarda o botao
ON_FRESH_VI("join_jp", FRCMD_INIT, FrLoginDlg::OnJoinJpBtnInit)
ON_FRESH_VV("join_jp", FRCMD_LBUTTONUP, FrLoginDlg::OnJoinBtnUp)
ON_FRESH_VI("quit", FRCMD_INIT, FrLoginDlg::OnQuitBtnInit)
ON_FRESH_VV("quit", FRCMD_LBUTTONUP, FrLoginDlg::OnQuitBtnUp)
ON_FRESH_VI("login_img", FRCMD_INIT, FrLoginDlg::OnLoginImgInit)
ON_FRESH_VI("s_id", FRCMD_INIT, FrLoginDlg::OnHideStaticInit)
ON_FRESH_VI("s_password", FRCMD_INIT, FrLoginDlg::OnHideStaticInit)
ON_FRESH_VI("s_autoid", FRCMD_INIT, FrLoginDlg::OnHideStaticInit)
ON_FRESH_VI("join", FRCMD_INIT, FrLoginDlg::OnJoinBtnInit)
ON_FRESH_VI("logo1", FRCMD_INIT, FrLoginDlg::OnParanAreaInit)
ON_FRESH_VI("logo2", FRCMD_INIT, FrLoginDlg::OnParanAreaInit)
ON_FRESH_VI("bkframe1", FRCMD_INIT, FrLoginDlg::OnHideAreaInit)
ON_FRESH_VV("bkframe1", FRCMD_OWNERDRAW, FrLoginDlg::OnHideAreaDraw)
ON_FRESH_VI("ID_EDIT_NOTICE", FRCMD_INIT, FrLoginDlg::OnLoginNoticeInit)
ON_FRESH_VI("icon1", FRCMD_INIT, FrLoginDlg::OnHideAreaInit)
ON_FRESH_VI("icon2", FRCMD_INIT, FrLoginDlg::OnHideAreaInit)
ON_FRESH_VI("box1", FRCMD_INIT, FrLoginDlg::OnHideAreaInit)
ON_FRESH_VI("box2", FRCMD_INIT, FrLoginDlg::OnHideAreaInit)
ON_FRESH_VI("s_id", FRCMD_INIT, FrLoginDlg::OnHideStaticInit)
ON_FRESH_VI("s_passwd", FRCMD_INIT, FrLoginDlg::OnHideStaticInit)
ON_FRESH_VI("s_save", FRCMD_INIT, FrLoginDlg::OnHideStaticInit)

END_FRESH_MSGMAP()

FrLoginDlg::FrLoginDlg()
{
	// 983 00ada490: m_pId/m_pPassword/m_pAutoId/+0x160 nao sao zerados
	m_pView = NULL;
	m_pLogin = NULL;
	C_ASSERT_VC10(offsetof(FrLoginDlg, m_pLogin) == 0x154 && offsetof(FrLoginDlg, m_pLoginFb) == 0x158 && offsetof(FrLoginDlg, m_pAutoId) == 0x15c); // A772
	C_ASSERT_VC10(offsetof(FrLoginDlg, m_pJoin) == 0x16c && offsetof(FrLoginDlg, m_failCount) == 0x170 && sizeof(FrLoginDlg) == 0x180); // A774
	m_failCount = 0;
	SetState((eLoginState)0);
	m_pJoin = NULL;
	m_reserved174 = 0.0f;
	m_reserved164 = 0;
	m_reserved168 = 0;
	m_pLoginFb = NULL;
	Doc()->SetNeedFirstLoginAlarm(true);
}

FrLoginDlg::~FrLoginDlg()
{
	// 983 00ada550
	if (GetState() == (eLoginState)4)
		FinishLogin();
}

void FrLoginDlg::FinishLogin()
{
	// 983 00ad98a0: OnProc (login ok) e dtor
	if (m_pAutoId->GetStatus() == FrButton::PRESSED)
		COption::Instance()->gSetLastLoginID(m_id.c_str());
	SetState((eLoginState)3);
}

void FrLoginDlg::SetNoticeMsg(const char* text)
{
	if (m_pView)
		m_pView->AddLine(text, 0, 0);
	EnableLoginControls(true);
}

void FrLoginDlg::SetState(eLoginState state)
{
	m_connState = state;
}

eLoginState FrLoginDlg::GetState()
{
	return m_connState;
}

void FrLoginDlg::SetErrorState983(int code)
{
	// 983 00ada7d0: lobbymain 0x1e com param1 0x1c; texto do ErrorCodeInfo.iff (+0xc) e o codigo em cinza
	m_connState = (eLoginState)0x1c;
	s_errorCode983 = code;
	IFF983::sErrorCodeInfo* info = Doc()->m_itemManager.FindErrorCodeInfo983(code);
	if (info)
	{
		const char* name = (const char*)&info->raw[4];
		s_errorText983.assign(name, strlen(name));
		s_errorText983.append("\n\\c0xff888888\\c", 0xf);
		const char* num = MakeStr("(%d)", code);
		s_errorText983.append(num, strlen(num));
	}
	else
	{
		const char* text = K2L_Compatibility("\277\300\267\371\260\241 \271\337\273\375\307\317\277\264\275\300\264\317\264\331. \300\341\275\303 \310\304 \264\331\275\303 \275\303\265\265\307\330 \301\326\274\274\277\344.");
		s_errorText983.assign(text, strlen(text));
		s_errorText983.append("\n\\c0xff888888\\c", 0xf);
		const char* num = MakeStr("(%d)", code);
		s_errorText983.append(num, strlen(num));
	}
}

const char* FrLoginDlg::GetErrorText983()
{
	return s_errorText983.c_str();
}

unsigned __stdcall HttpGetNtreevUserAuthKey(void* param)
{
	cNTAUTHPARAM* auth = (cNTAUTHPARAM*)param;
	GenericHTTPClient client;
	client.InitilizePostArguments();
	client.AddPostArguments("id", auth->id.c_str(), FALSE);
	client.AddPostArguments("pwd", auth->pwd.c_str(), FALSE);
	client.AddPostArguments("gamecode", 9UL);
	FrWnd* wnd = g_pFresh->GetDesktop()->FindChildForm("login");
	FrLoginDlg* dlg = DYNAMIC_CAST(FrLoginDlg, wnd);
	int success = client.Request(
		"http://qa.pangya.gametree.co.kr/Secure/Login/LoginForGame.aspx", 3,
		"MERONG(0.9/;p)");
	const char* response = client.QueryHTTPResponse();
	if (success && strlen(response) <= 520)
	{
		cHttpNtreevAuthResult result(response);
		if (result.GetResult())
		{
			std::string key = result.GetFieldValue("AuthKey");
			std::string member = result.GetFieldValue("MemberNo");
			std::string pcBang = result.GetFieldValue("PCBangNo");

			if (dlg)
			{
				LOGININFO()->SetPw(key);
				LOGININFO()->SetAuthUid(strtoul(member.c_str(), NULL, 10));
				LOGININFO()->EnablePcBang(pcBang == "0" ? FALSE : TRUE);
				dlg->BeginConnect();
			}
			else
			{
				FrForm* notify = CreateForm<FrForm>(g_pFresh->GetManager(),
					NULL, "notify", NULL);
				notify->SetMessage(
					"\267\316\261\327\300\316 \300\316\301\365 \275\303\260\243 \303\312\260\372. \264\331\275\303 \275\303\265\265 \307\317\274\274\277\344.",
					false);
			}
		}
		else
		{
			std::string message = result.GetMessageA();
			if (dlg)
			{
				dlg->SetNoticeMsg(message.c_str());
				dlg->EnableLoginControls(true);
			}
			else
			{
				FrForm* notify = CreateForm<FrForm>(g_pFresh->GetManager(),
					NULL, "notify", NULL);
				notify->SetMessage(message.c_str(), false);
				notify->Open(NULL, FrESCAPE | FrENTER);
			}
		}
		delete auth;
	}
	else
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)"\300\245\274\255\272\361\275\272 \300\316\301\365\277\241 \275\307\306\320\307\317\277\264\275\300\264\317\264\331.",
			0, 0, 0, 0));
		if (dlg)
			dlg->EnableLoginControls(true);
		delete auth;
	}
	return 0;
}

void FrLoginDlg::BeginClientLogin()
{
	if (m_pId && m_pId->IsVisible())
	{
		char id[260] = "";
		strcpy(id, m_pId->GetLine(1, 0));
		m_id = TrimA(id, " \t\n\r\0");
		if (m_id.empty())
		{
			if (m_pView)
				m_pView->AddLine(
					K2L_Compatibility("\276\306\300\314\265\360\270\246 \263\326\276\356\301\326\274\274\277\344."),
					0, 0);
			if (m_pId)
				m_pId->SetKeyFocus(true);
			return;
		}
	}
	if (m_pPassword && m_pPassword->IsVisible())
	{
		char password[260] = "";
		strcpy(password, m_pPassword->GetLine(1, 0));
		m_passwd = TrimA(password, " \t\n\r\0");
		if (m_passwd.empty())
		{
			if (m_pView)
				m_pView->AddLine(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\270\246 \300\324\267\302\307\330\301\326\274\274\277\344."),
					0, 0);
			if (m_pPassword)
				m_pPassword->SetKeyFocus(true);
			return;
		}
	}
	// 983 00adac64: com Doc m_bChannelLogin relê id/senha dos controles mesmo escondidos (sem checar vazio)
	if (Doc()->m_bChannelLogin)
	{
		char buf[260] = "";
		if (m_pId)
		{
			strcpy(buf, m_pId->GetLine(1, 0));
			m_id = TrimA(buf, " \t\n\r\0");
		}
		if (m_pPassword)
		{
			strcpy(buf, m_pPassword->GetLine(1, 0));
			m_passwd = TrimA(buf, " \t\n\r\0");
		}
	}
	_ValidId valid = std::for_each(m_id.begin(), m_id.end(), _ValidId());
	if (valid.bInvalid && m_pView)
	{
		m_pView->AddLine(
			K2L_Compatibility("\276\306\300\314\265\360\264\302 \277\265\271\256\300\332\277\315 \274\375\300\332, '_'\270\270 \273\347\277\353\307\317\275\307 \274\366 "),
			0, 0);
		m_pView->AddLine(K2L_Compatibility("\300\326\275\300\264\317\264\331."), 0, 0);
	}
	else
	{
		LOGININFO()->SetId(m_id);
		LOGININFO()->SetPw(m_passwd);
		// 983 00adae58: conecta direto no servidor de login (BeginConnect 00ada8c0); o 645 pedia antes a chave de
		// autenticacao web coreana (HttpGetNtreevUserAuthKey em qa.pangya.gametree.co.kr: "falha na autenticacao", C96)
		BeginConnect();
		EnableLoginControls(false);
	}
}

bool FrLoginDlg::PrepareWebLogin()
{
	return strlen(LOGINID()) && strlen(LOGINPW());
}

void FrLoginDlg::EnableLoginControls(bool enable)
{
	m_pId->Enable(enable);
	m_pPassword->Enable(enable);
	m_pLogin->Enable(enable);
}

void FrLoginDlg::BeginConnect()
{
	m_defaultRetCode = FrNONE;
	m_id = CLoginInfo::Instance()->Id();
	m_passwd = CLoginInfo::Instance()->Passwd();
	char* id = Doc()->m_myInfo.info.sID;
	memset(id, 0, sizeof(Doc()->m_myInfo.info.sID));
	strncpy(id, m_id.c_str(), 21);
	Doc()->m_password = m_passwd;
	NET()->Init(WNetworkSystem::NET_MAX);
	// 983 00adaa0b: com Doc m_bChannelLogin a linha sai sem o id ("접속 중...|", d4bbf0)
	if (m_pView)
		m_pView->AddLine(Doc()->m_bChannelLogin
			? K2L_Compatibility("\301\242\274\323 \301\337...|")
			: MakeStr(K2L_Compatibility("%s \301\242\274\323 \301\337...|"), LOGINID()),
			0, 0);
	// 983 00adaa4b (A202): login novo (nao a volta da 0x2f0): zera o byte +0x11250 e o conjunto +0x10a20. O 983 ainda grava
	// 1 no estado do login (SetState 00ad9190 = m_connState).
	*(bool*)&Doc()->m_bReturnToLogin983 = false;
	Doc()->m_failedGameServerIds983.clear();
	SetState((eLoginState)1);
	NET()->SendMessage(WNetworkSystem::NET_LOGIN, (NetworkUnit::eEvent)0);
}

void FrLoginDlg::Login()
{
	if (m_pLogin && m_pLogin->IsEnabled())
	{
		if (CLoginInfo::Instance()->IsWebLogin())
		{
			if (PrepareWebLogin())
			{
				EnableLoginControls(false);
				BeginConnect();
			}
			else if (m_pView)
				m_pView->AddLine(
					K2L_Compatibility("\267\316\261\327\300\316 \275\303\275\272\305\333 \301\244\272\361\301\337\300\324\264\317\264\331."),
					0, 0);
		}
		else
			BeginClientLogin();
	}
}

bool FrLoginDlg::OnJoinDlgResult(int result, FrForm*)
{
	if (result == FrOK)
	{
		ShellExecuteA(NULL, NULL,
			"http://www.pangya.jp", NULL, NULL,
			SW_MAXIMIZE);
		if (!COption::Instance()->vIsWindowed())
			g_bQuit = true;
	}
	return true;
}

bool FrLoginDlg::OnDisconPrevSession(int result, FrForm*)
{
	if (result == FrOK)
	{
		CProjectG::Instance()->m_bMoveLoginServer = false;
		AfxGetTask()->GetActor("Lobby")
			<< MsgObject(NULL, 0, (int)"SERVERLIST", 0, 0, 0, 0);
		WSendPacket packet(4);
		packet.Send(TO_LOGIN);
	}
	else
	{
		m_pId->SetLine(1, "", 0, 0, 0);
		m_pId->ClearLine();
		m_pPassword->SetLine(1, "", 0, 0, 0);
		m_pPassword->ClearLine();
		m_pId->SetKeyFocus(true);
		SetState((eLoginState)5); // 983 00ad98f0: call 00ad9190
		SetFadeout(true);
	}
	return true;
}

bool FrLoginDlg::OnFindDlgResult(int result, FrForm*)
{
	if (result == FrOK)
	{
		ShellExecuteA(NULL, NULL, "about:blank", NULL, NULL, SW_MAXIMIZE);
		if (!COption::Instance()->vIsWindowed())
			g_bQuit = true;
	}
	return true;
}

void FrLoginDlg::OnProc(float delta)
{
	bool autoLogin = false;
	if (m_connState == 0)
	{
		if (CLoginInfo::Instance()->IsWebLogin())
			autoLogin = true;
		if ((m_connState == 0 && CProjectG::Instance()->m_bMoveLoginServer) ||
			autoLogin)
			Login();
	}
	if (NET())
	{
		WNetworkSystem* network = NET();
		switch (m_connState)
		{
		case 1:
		{
			int lines = m_pView->GetLineNum();
			if (lines)
			{
				char spin[3] = { '-', '/', '|' };
				const char* line = m_pView->GetLine(lines, 0);
				unsigned int length = strlen(line);
				char text[1024];
				UINT_PTR offset = (UINT_PTR)text - (UINT_PTR)line;
				const char* cursor = line;
				char ch;
				do
				{
					ch = *cursor;
					*(char*)(offset + (UINT_PTR)cursor) = ch;
					++cursor;
				} while (ch);
				if (text[length - 1] == '-' || text[length - 1] == '/' ||
					text[length - 1] == '|')
				{
					static float rotation = 0.0f;
					int step = (int)rotation;
					rotation += delta * 10.0f;
					text[length - 1] = spin[step % 3];
					m_pView->SetLine(lines, text, 0, 0, 0);
				}
			}
			if (network->IsConnectionComplete(WNetworkSystem::NET_LOGIN) &&
				!network->IsConnected(WNetworkSystem::NET_LOGIN))
			{
				SetState((eLoginState)5);
				if (m_pView)
				{
					m_pView->AddLine(
						K2L_Compatibility("\267\316\261\327\300\316 \274\255\271\366\277\241 \301\242\274\323 \307\322 \274\366 \276\370\275\300\264\317\264\331."),
						0, 0);
					EnableLoginControls(true);
				}
			}
			break;
		}
		case 4:
			if (m_pView)
			{
				m_pView->AddLine(K2L_Compatibility("\267\316\261\327\300\316 \274\272\260\370."),
					0, 0);
				m_pView->AddLine(
					K2L_Compatibility("\274\255\271\366\270\256\275\272\306\256\270\246 \271\336\260\355 \300\326\275\300\264\317\264\331."),
					0, 0);
			}
			FinishLogin();
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
			break;
		case 6:
			EnableLoginControls(true);
			ResetInput(
				K2L_Compatibility("\276\306\300\314\265\360\260\241 \300\337\270\370\265\307\276\372\275\300\264\317\264\331."));
			break;
		case 27:
			// 983 00adb256: so o aviso de servidor cheio (sem EnableLoginControls/estado 5)
			ResetInput(
				K2L_Compatibility("\274\255\271\366\300\307 \277\344\303\273\300\314 \270\271\275\300\264\317\264\331. \300\341\275\303 \310\304 \264\331\275\303 \275\303\265\265 \307\330 \301\326\275\312\275\303\277\300."));
			break;
		case 8:
		{
			SetState((eLoginState)3);
			// 983 00adb26d: sessao anterior aberta -> derruba sem perguntar (o que o OK do OnDisconPrevSession faz); o 645
			// perguntava pelo notify_okcancel (texto coreano, sem par no 983) e, com 0x5a, refazia o login do servidor movido.
			CProjectG::Instance()->m_bMoveLoginServer = false;
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, 0, (int)"SERVERLIST", 0, 0, 0, 0);
			WSendPacket packet(4);
			packet.Send(TO_LOGIN);
			break;
		}
		case 7:
			EnableLoginControls(true);
			++m_failCount;
			if (m_pView)
			{
				m_pView->AddLine(
					K2L_Compatibility("\276\306\300\314\265\360\263\252 \272\361\271\320\271\370\310\243\260\241 \300\337\270\370 \300\324\267\302\265\307\276\372\275\300\264\317\264\331."),
					0, 0);
				m_pView->AddLine(
					K2L_Compatibility("\\c0xffff0000,0xffffffff\\c'Caps Lock'\\c0xff000000,0xffffffff\\c \305\260\260\241 \264\255\267\257\301\256 \300\326\264\302\301\366 \310\256\300\316\307\330\272\270\274\274\277\344."),
					0, 0);
				SetState((eLoginState)5);
				if (m_failCount > 4)
					g_bQuit = true;
				m_pPassword->SetLine(1, "", 0, 0, 0);
				m_pPassword->ClearLine();
				m_pPassword->SetKeyFocus(true);
			}
			break;
		case 12:
			EnableLoginControls(true);
			ResetInput(
				K2L_Compatibility("\274\255\271\366\271\256\301\246\267\316 \267\316\261\327\300\316 \307\322\274\366 \276\370\275\300\264\317\264\331."));
			SetState((eLoginState)5);
			break;
		case 16:
			EnableLoginControls(true);
			m_pView->AddLine(
				K2L_Compatibility("\260\241\300\324\265\310 \301\326\271\316 \271\370\310\243\260\241 \277\303\271\331\270\243\301\366 \276\312\275\300\264\317\264\331."),
				0, 0);
			m_pView->AddLine(
				K2L_Compatibility("\\c0xffff0000,0xffffffff\\c'\260\355\260\264 \301\366\277\370'\\c0xff000000,0xffffffff\\c \300\307 \270\336\300\317 \271\256\300\307\270\246 \271\331\266\370\264\317\264\331."),
				0, 0);
			SetState((eLoginState)5);
			break;
		case 23:
			EnableLoginControls(true);
			ResetInput(
				K2L_Compatibility("\260\350\301\244 \300\314\300\374\300\314 \265\307\276\356 \301\242\274\323\307\322 \274\366 \276\370\275\300\264\317\264\331."));
			break;
		case 9:
			// 983 00adb42f: conta bloqueada para sempre
			EnableLoginControls(true);
			if (m_pView)
			{
				m_pView->AddText(
					K2L_Compatibility("\\c0xffff0000,0xffffffff\\c\277\265\261\270\300\373\300\270\267\316 \273\347\277\353\261\335\301\366\\c0xff000000,0xffffffff\\c \265\310 \276\306\300\314\265\360\300\324\264\317\264\331."),
					0, true);
				m_pView->AddText(
					K2L_Compatibility("\260\355\260\264 \301\366\277\370 \274\276\305\315\267\316 \271\256\300\307 \271\331\266\370\264\317\264\331."),
					0, true);
				SetState((eLoginState)5);
			}
			break;
		case 10:
			// 983 00adb47f: bloqueio com prazo; Doc+0x10cf8 (m_chatMode no nosso cabecalho) = horas do pacote 7/5100022
			EnableLoginControls(true);
			if (m_pView)
			{
				std::string period;
				if (Doc()->m_chatMode > 0)
				{
					unsigned long days = Doc()->m_chatMode / 24;
					unsigned long hours = Doc()->m_chatMode % 24;
					if (days > 0)
						period += MakeStr(K2L_Compatibility("%d\300\317 "), days);
					if (hours > 0)
						period += MakeStr(K2L_Compatibility("%d\275\303\260\243 "), hours);
				}
				m_pView->AddText(
					MakeStr(K2L_Compatibility("\\c0xffff0000,0xffffffff\\c%s\265\277\276\310 \273\347\277\353\261\335\301\366\\c0xff000000,0xffffffff\\c \265\310 \276\306\300\314\265\360\300\324\264\317\264\331."),
						period.c_str()),
					0, true);
				m_pView->AddText(
					K2L_Compatibility("\260\355\260\264 \301\366\277\370 \274\276\305\315\267\316 \271\256\300\307 \271\331\266\370\264\317\264\331."),
					0, true);
				SetState((eLoginState)5);
			}
			break;
		case 17:
			EnableLoginControls(true);
			ResetInput(
				K2L_Compatibility("\310\270\277\370\300\273 \305\273\305\360\307\321 \276\306\300\314\265\360\300\324\264\317\264\331."));
			break;
		case 15:
			EnableLoginControls(true);
			if (m_pView)
			{
				m_pView->AddText(
					K2L_Compatibility("(\276\313\270\262)-----------------------------------"), 0,
					true);
				m_pView->AddText(
					K2L_Compatibility("\\c0xffff0000,0xffffffff\\c\270\270 14\274\274 \271\314\270\270\\c0xffffffff,0xff000000\\c\300\307 \310\270\277\370\300\272 \\c0xffff0000,0xffffffff\\c\272\316\270\360\264\324\300\307 \265\277\300\307\\c0xffffffff,0xff000000\\c\260\241 \307\312\277\344\307\325\264\317\264\331. \307\366\300\347 \272\316\270\360\264\324 \265\277\300\307 \260\241\300\324\300\314 \265\307\276\356 \300\326\301\366 \276\312\275\300\264\317\264\331. \\c0xffff0000,0xffffffff\\c\306\316\276\337 \310\250\306\344\300\314\301\366\\c0xffffffff,0xff000000\\c\277\241 \267\316\261\327\300\316\307\321 \310\304 \\c0xffff0000,0xffffffff\\c\272\316\270\360\265\277\300\307\274\255 \300\347\271\337\274\333\\c0xffffffff,0xff000000\\c\300\273 \264\251\270\243\270\351 \272\316\270\360\264\324\265\277\300\307 \275\305\303\273\300\314 \300\314\267\347\276\356\301\366\260\355 \271\331\267\316 \260\324\300\323\300\314 \260\241\264\311\307\325\264\317\264\331."),
					0, true);
				m_pView->AddText("----------------------------------------", 0,
					true);
				SetState((eLoginState)5);
			}
			break;
		case 14:
			EnableLoginControls(true);
			if (m_pView)
			{
				m_pView->AddText(
					K2L_Compatibility("(\276\313\270\262)----------------------------------"), 0,
					true);
				m_pView->AddText(
					K2L_Compatibility("\\c0xffff0000,0xffffffff\\c\270\270 14\274\274 \271\314\270\270\\c0xffffffff,0xff000000\\c\300\307 \310\270\277\370\300\272 \\c0xffff0000,0xffffffff\\c\272\316\270\360\264\324\300\307 \265\277\300\307\\c0xffffffff,0xff000000\\c\260\241 \307\312\277\344\307\325\264\317\264\331. \272\316\270\360\264\324 \265\277\300\307\274\255\260\241 \276\306\301\367 \275\302\300\316\265\307\301\366 \276\312\300\272 \273\363\305\302\300\324\264\317\264\331."),
					0, true);
				m_pView->AddText(
					K2L_Compatibility("\012\\c0xff0000ff,0xffffffff\\c\306\316\276\337 \310\250\306\344\300\314\301\366\277\241 \271\346\271\256\307\317\277\251 \271\256\300\307\307\317\275\303\261\342 \271\331\266\370\264\317\264\331.\\c0xffffffff,0xff000000\\c"),
					0, true);
				m_pView->AddText("---------------------------------------", 0,
					true);
				SetState((eLoginState)5);
			}
			break;
		case 11:
			EnableLoginControls(true);
			if (m_pView)
			{
				m_pView->AddLine(
					K2L_Compatibility("\272\361\271\320\271\370\310\243\260\241 5\310\270 \300\337\270\370 \300\324\267\302\265\307\276\356"),
					0, 0);
				m_pView->AddLine(
					K2L_Compatibility("1\275\303\260\243\265\277\276\310 \274\255\271\366\277\241 \301\242\274\323\307\322 \274\366 \276\370\275\300\264\317\264\331!"),
					0, 0);
				m_pView->AddLine("------------------------------------", 0, 0);
				m_pView->AddLine(
					K2L_Compatibility("====== \306\316\276\337\270\246 \301\276\267\341\307\317\260\332\275\300\264\317\264\331. ======"),
					0, 0);
				m_pView->AddLine("------------------------------------", 0, 0);
			}
			ResetInput(NULL);
			g_bQuit = true;
			break;
		case 18:
			EnableLoginControls(true);
			ResetInput(
				K2L_Compatibility("\300\324\300\345 \300\332\260\335\300\273 \271\336\300\272 \300\257\300\372\270\270 \301\242\274\323\307\322 \274\366 \300\326\275\300\264\317\264\331."));
			break;
		case 13:
			EnableLoginControls(true);
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, 0, (int)"CREATE", 0, 0, 0, 0);
			break;
		case 19:
			EnableLoginControls(true);
			if (m_pView)
				m_pView->AddLine(
					K2L_Compatibility("\301\241\260\313\300\270\267\316 \300\316\307\330 \267\316\261\327\300\316 \301\337\301\366 \265\307\276\372\275\300\264\317\264\331."),
					0, 0);
			ResetInput(NULL);
			break;
		case 24:
			EnableLoginControls(true);
			if (m_pView)
				m_pView->AddLine(
					K2L_Compatibility("\260\355\260\264\274\276\305\315 1:1 \271\256\300\307\267\316 \301\242\274\366 \272\316\305\271\265\345\270\263\264\317\264\331."),
					0, 0);
			ResetInput(NULL);
			break;
		case 20:
			EnableLoginControls(true);
			m_pView->AddText("PangYa is not available from your area.", 0,
				true);
			m_pView->AddText("We are truly sorry for any inconvenience.", 0,
				true);
			ResetInput(NULL);
			SetState((eLoginState)5);
			break;
		case 22:
			EnableLoginControls(true);
			ResetInput(
				K2L_Compatibility("\300\316\301\365\300\314 \277\317\267\341\265\307\301\366 \276\312\276\322\275\300\264\317\264\331.\265\356\267\317 \270\336\300\317\300\273 \310\256\300\316\307\330 \301\326\275\312\275\303\277\300."));
			break;
		case 21:
			EnableLoginControls(true);
			m_pView->AddText(
				K2L_Compatibility("\300\324\300\345\261\307\300\314 \276\370\276\356\274\255 \267\316\261\327\300\316 \307\322 \274\366 \276\370\275\300\264\317\264\331."),
				0, true);
			m_pView->AddText(
				K2L_Compatibility("\306\316\276\337 \310\250\306\344\300\314\301\366\277\241\274\255 \271\253\267\341 \300\324\300\345\261\307\300\273 \271\337\261\336 \271\336\300\270\275\307 \274\366 \300\326\275\300\264\317\264\331."),
				0, true);
			SetState((eLoginState)5); // 983 00adb87b: as 2 linhas por K2L
			break;
		case 25:
			if (IsLocalContent((localContentType_t)126 /*983: era 129; medido*/))
			{
				EnableLoginControls(true);
				if (m_pView)
				{
					m_pView->AddText(
						K2L_Compatibility("\300\337\270\370\265\310 \300\257\300\372\301\244\272\270\270\246 \300\320\276\372\275\300\264\317\264\331."),
						0, true);
					m_pView->AddText(
						K2L_Compatibility("\300\347 \267\316\261\327\300\316\307\317\275\303\260\305\263\252, \260\355\260\264 \301\366\277\370 \274\276\305\315\267\316 \271\256\300\307 \271\331\266\370\264\317\264\331."),
						0, true);
				}
				ResetInput(NULL);
				SetState((eLoginState)5);
			}
			break;
		case 26:
			EnableLoginControls(true);
			if (m_pView)
				m_pView->AddText(
					K2L_Compatibility("CBT\261\342\260\243\300\272 \275\305\261\324\265\356\267\317\300\273 \307\322 \274\366 \276\370\275\300\264\317\264\331."),
					0, true);
			ResetInput(NULL);
			SetState((eLoginState)5);
			break;
		case 28:
			// 983 00adb926: erro do ErrorCodeInfo montado no SetErrorState983
			EnableLoginControls(true);
			if (m_pView)
				m_pView->AddText(GetErrorText983(), 0, true);
			ResetInput(NULL);
			SetState((eLoginState)5);
			break;
		}
	}
}

void FrLoginDlg::OnIdInit(int param)
{
	m_pId = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	// 983 00ad9a20: com Doc m_bChannelLogin o id vem da linha de comando e o campo fica escondido
	if (Doc()->m_bChannelLogin)
	{
		m_pId->SetVisible(false);
		m_pId->SetLine(1, Doc()->m_commandLineID.c_str(), 0, 0, 0);
		return;
	}
	m_pId->m_nFlags.Enable(FWF_PRIVACY);
	m_pId->SetCharLimit(22, true);
	m_pId->SetLine(1, COption::Instance()->gGetLastLoginID(), 0, 0, 0);
	if (CProjectG::Instance()->m_bMoveLoginServer)
		m_pId->SetLine(1, CProjectG::Instance()->m_moveLoginId.c_str(), 0, 0,
			0);
	const char* line = m_pId->GetLine(1, 0);
	if (!line || !strlen(line))
		m_pId->SetKeyFocus(true);
	if (CLoginInfo::Instance()->IsWebLogin())
	{
		m_pId->SetLine(1, LOGINID(), 0, 0, 0);
		m_pId->SetVisible(false);
	}
}

void FrLoginDlg::OnPasswordInit(int param)
{
	m_pPassword = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pPassword)
	{
		// 983 00ad9c10: com Doc m_bChannelLogin a senha vem do Doc (m_password) e o campo fica escondido
		if (Doc()->m_bChannelLogin)
		{
			m_pPassword->SetVisible(false);
			m_pPassword->SetLine(1, Doc()->m_password.c_str(), 0, 0, 0);
			return;
		}
		if (CLoginInfo::Instance()->IsWebLogin())
			m_pPassword->SetLine(1, LOGINPW(), 0, 0, 0);
		else if (CProjectG::Instance()->m_bMoveLoginServer)
			m_pPassword->SetLine(1,
				CProjectG::Instance()->m_moveLoginPassword.c_str(), 0, 0, 0);
		const char* line = m_pId->GetLine(1, 0);
		if (line && strlen(line))
			m_pPassword->SetKeyFocus(true);
		if (CLoginInfo::Instance()->IsWebLogin())
			m_pPassword->SetVisible(false);
	}
}

bool FrLoginDlg::OnEnterKey(int)
{
	return false;
}

void FrLoginDlg::OnAutoIdInit(int param)
{
	m_pAutoId = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	// 983 00ad9e10: esconde por Doc m_bChannelLogin; sem teste de CLoginInfo web login
	if (Doc()->m_bChannelLogin)
	{
		m_pAutoId->SetStatus(FrButton::NORMAL);
		m_pAutoId->SetVisible(false);
		return;
	}
	const char* id = COption::Instance()->gGetLastLoginID();
	if (id && strlen(id))
		m_pAutoId->SetStatus(FrButton::PRESSED);
}

void FrLoginDlg::OnAutoIdBtnUp()
{
	if (m_pAutoId->GetStatus() != FrButton::PRESSED)
		COption::Instance()->gSetLastLoginID("");
}

void FrLoginDlg::OnViewInit(int param)
{
	m_pView = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pView)
		m_pView->AddText(
			K2L_Compatibility("\306\316\276\337\300\307 \274\274\260\350\277\241 \277\300\275\305 \260\315\300\273 \310\257\277\265\307\325\264\317\264\331"),
			0, true);
}

const char* FrLoginDlg::QueryWelcomeMessage()
{
	return NULL;
}

void FrLoginDlg::OnLoginInit(int param)
{
	m_pLogin = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (IsLocalContent((localContentType_t)16))
		if (CLoginInfo::Instance()->IsWebLogin())
			m_pLogin->SetVisible(false);
}

void FrLoginDlg::OnLoginBtnUp()
{
	Login();
}

void FrLoginDlg::OnJoinBtnInit(int param)
{
	// 983 (00ad9680): so guarda o botao
	m_pJoin = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrLoginDlg::OnJoinJpBtnInit(int param)
{
	// 983 (00ada070)
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (Doc()->m_bChannelLogin && pBtn)
	{
		pBtn->SetVisible(false);
		pBtn->Enable(false);
	}
}

void FrLoginDlg::OnQuitBtnInit(int param)
{
	// 983 (00ada180)
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (Doc()->m_bChannelLogin && pBtn)
	{
		pBtn->SetVisible(false);
		pBtn->Enable(false);
	}
}

void FrLoginDlg::OnQuitBtnUp()
{
	// 983 (00ad9320)
	PostQuitMessage(0);
}

void FrLoginDlg::OnJoinBtnUp()
{
	// 983 (00ada100): botao "join_jp" -> pangya.jp
	FrForm* dlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
		"notify_okcancel", NULL);
	if (dlg)
	{
		dlg->SetMessage(K2L_Compatibility(
			"\306\316\276\337 \260\370\275\304 \310\250\306\344\300\314\301\366\267\316 \277\254\260\341 \307\317\275\303\260\332\275\300\264\317\261\356?"),
			false);
		dlg->Open((FRESH_PFN_RESULT)&FrLoginDlg::OnJoinDlgResult, 0x800); // 983: 0x800
	}
}

void FrLoginDlg::ResetInput(const char* text)
{
	if (m_pView && text)
		m_pView->AddLine(text, 0, 0);
	if (m_pPassword)
	{
		m_pPassword->SetLine(1, "", 0, 0, 0);
		m_pPassword->ClearLine();
	}
	if (m_pId && m_pId->m_nFlags.GetFlag(FWF_KEYFOCUS))
		m_pId->SetKeyFocus(true);
	else if (m_pPassword && m_pPassword->m_nFlags.GetFlag(FWF_KEYFOCUS))
		m_pPassword->SetKeyFocus(true);
	SetState((eLoginState)5); // 983 00ad96f0: call 00ad9190
}

bool FrLoginDlg::OnInit()
{
	CProjectG::Instance()->m_bMsnOffline = false;
	SetMessage("\305\327\275\272\306\256 \261\270\271\256", false);
	ms_bModalLock983 = true; // 983 00ada210: call 00be7950
	return true;
}

void FrLoginDlg::OnHideStaticInit(int param)
{
	// 983 00ada2f0/00ada020: esconde com CSharedDoc m_bChannelLogin, nao CLoginInfo web login
	FrStatic* wnd = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (CSharedDoc::Instance()->m_bChannelLogin)
		wnd->SetVisible(false);
}

// 983 00ad9600 (loginfb): guarda o botao em +0x158 e esconde sempre
void FrLoginDlg::OnLoginFbInit(int param)
{
	m_pLoginFb = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pLoginFb)
		m_pLoginFb->SetVisible(false);
}

// 983 00ad9310: vazio
void FrLoginDlg::OnLoginFbBtnUp()
{
}

// 983 00ad9fd0 (login_img): esconde quando NAO e login por canal
void FrLoginDlg::OnLoginImgInit(int param)
{
	if (!CSharedDoc::Instance()->m_bChannelLogin)
	{
		FrArea* wnd = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		if (wnd)
			wnd->SetVisible(false);
	}
}

// 983 00ad9790 (logo1/logo2): esconde sempre, sem teste de web login
void FrLoginDlg::OnParanAreaInit(int param)
{
	FrArea* wnd = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	wnd->SetVisible(false);
}

// 983 00ada370 (bkframe1/icon/box): esconde com CSharedDoc m_bChannelLogin (+0x11458), nao CLoginInfo web login
void FrLoginDlg::OnHideAreaInit(int param)
{
	FrArea* wnd = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (CSharedDoc::Instance()->m_bChannelLogin)
		wnd->SetVisible(false);
}

void FrLoginDlg::OnHideAreaDraw()
{
}

void FrLoginDlg::OnLoginNoticeInit(int param)
{
	// 983 00ad9800: texto via K2L_Compatibility (d4ba60), AddText(.., false, true)
	FrEdit* wnd = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	wnd->AddText(K2L_Compatibility(
		"\241\330\301\337\277\344 : \307\321\272\373\274\322\307\301\306\256 \306\316\276\337 \310\270\277\370\300\314\275\305\260\241\277\344?\012\306\316\276\337 \310\250\306\344\300\314\301\366\277\241\274\255 \271\335\265\345\275\303 \310\270\277\370 \301\244\272\270 \300\314\300\374 \275\305\303\273\300\273 \307\317\275\305 \310\304\277\241 \267\316\261\327\300\316\307\317\274\305\276\337 \301\244\273\363\300\373\300\316 \260\324\300\323\300\314 \260\241\264\311\307\325\264\317\264\331."),
		false, true);
}

void FrLoginDlg::OnHideBtnInit(int param)
{
	FrButton* wnd = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (CLoginInfo::Instance()->IsWebLogin())
		wnd->SetVisible(false);
}
