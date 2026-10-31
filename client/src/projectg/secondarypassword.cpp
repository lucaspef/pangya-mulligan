#include "minatl.h"
#include "browser.h"
#include "secondarypassword.h"
#include "secondarypassworddlg.h"
#include "gatewayactor.h"
#include "clientsetting.h"
#include "logininfo.h"
#include "messagemanager.h"
#include "../shared/s5/sharedutilities.h"

extern Fresh* g_pFresh;

using namespace _gateway;

CSecondaryPassword::CSecondaryPassword()
	: m_pExplanePasswordDlg(NULL),
	  m_pPasswordInputDlg(NULL),
	  m_pSelectCertifyDlg(NULL),
	  m_pPasswordNotifyDlg(NULL)
{
}

CSecondaryPassword::~CSecondaryPassword()
{
}

bool CSecondaryPassword::Initialize()
{
	AddFunctionType(1, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnResultRegistPassword);
	AddFunctionType(2, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnRequestRegistPassword);
	AddFunctionType(3, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnRequestStoreCertifyFlag);
	AddFunctionType(4, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CSecondaryPassword::
			OnRequestCheckCertifyFlag);
	AddFunctionType(5, FUNCTIONTYPE_VOID,
		(void (
			FunctionMapper::*)())&CSecondaryPassword::OnOpenPasswordExplaneDlg);
	AddFunctionType(6, FUNCTIONTYPE_VOID,
		(void (
			FunctionMapper::*)())&CSecondaryPassword::OnOpenPasswordCreateDlg);
	AddFunctionType(7, FUNCTIONTYPE_VOID,
		(void (
			FunctionMapper::*)())&CSecondaryPassword::OnOpenPasswordLoginDlg);
	AddFunctionType(8, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CSecondaryPassword::OnClosePasswordDlg);
	AddFunctionType(9, FUNCTIONTYPE_VOID,
		(void (
			FunctionMapper::*)())&CSecondaryPassword::OnOpenSelectCertifyDlg);
	AddFunctionType(10, FUNCTIONTYPE_VOID,
		(void (
			FunctionMapper::*)())&CSecondaryPassword::OnCloseSelectCertifyDlg);
	AddFunctionType(13, FUNCTIONTYPE_VOID,
		(void (
			FunctionMapper::*)())&CSecondaryPassword::OnOpenSelfCertifyWebPage);
	AddFunctionType(12, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnRequestLoginPassword);
	AddFunctionType(11, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnResultLoginPassword);
	AddFunctionType(14, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CSecondaryPassword::OnWaitGameServerList);
	AddFunctionType(15, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(void (
			CSecondaryPassword::*)())&CSecondaryPassword::OnNotifyPasswordInfo);
	AddFunctionType(16, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnRequestAvailableContents);
	AddFunctionType(17, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CSecondaryPassword::
			OnNotifyCannotUseContents);
	AddFunctionType(19, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CSecondaryPassword::
			OnNotifySecondPasswordConfirm);
	AddFunctionType(20, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnRequestQualifyLogin);
	AddFunctionType(21, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(
			void (CSecondaryPassword::*)())&CSecondaryPassword::
			OnRequestCheckPasswordValid);
	AddFunctionType(22, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(void (
			CSecondaryPassword::*)())&CSecondaryPassword::OnNotifySystemMsg);
	return true;
}

bool CSecondaryPassword::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec(this, (int)&msg);
}

void CSecondaryPassword::OnRequestRegistPassword(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	std::string password = (const char*)pMsg->param2;
	WSendPacket packet(9);
	packet.EncodeStr(password);
	packet.Send(TO_LOGIN);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
}

void CSecondaryPassword::OnResultRegistPassword(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	int error = pMsg->param2;
	char buffer[256] = { 0 };
	int message = 0;
	switch (error)
	{
	case 140101:
	case 140102:
	case 140301:
	case 145001:
	case 145002:
	case 145003:
	case 145004:
	case 145005:
	case 730002:
	case 730003:
		message = 105;
		break;
	}
	sprintf(buffer, "%s : %d",
		_systemmsg::CMessageManager::Instance()->GetMsg(
			(_systemmsg::eMsgIdentifier)message),
		error);
	OnOpenSystemMsg(message, buffer);
}

void CSecondaryPassword::OnRequestStoreCertifyFlag(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
		return;
	if (*(bool*)pMsg->param2 == true)
		pDoc->SetCertifyFlag();
}

void CSecondaryPassword::OnRequestCheckCertifyFlag()
{
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
		return;
	if (pDoc->GetCertifyFlag() == true)
		OnOpenPasswordLoginDlg();
	else
		OnOpenSelectCertifyDlg();
}

void CSecondaryPassword::OnOpenPasswordExplaneDlg()
{
	if (m_pExplanePasswordDlg)
		return;
	m_pExplanePasswordDlg = CreateForm<FrExplanePassword>(
		g_pFresh->GetManager(), this, "explanepassword", NULL);
	m_pExplanePasswordDlg->EnableDrag(false);
	m_pExplanePasswordDlg->Open(
		(FRESH_PFN_RESULT)&CSecondaryPassword::OnPasswordExplaneFormResult, 0);
}

void CSecondaryPassword::OnOpenPasswordCreateDlg()
{
	if (m_pPasswordInputDlg)
		return;
	m_pPasswordInputDlg = CreateForm<FrPasswordInputDlg>(g_pFresh->GetManager(),
		this, "passwordinputdlg", NULL);
	m_pPasswordInputDlg->EnableDrag(false);
	m_pPasswordInputDlg->SetDlgType(SECONDPWD_INPUT_CREATE);
	m_pPasswordInputDlg->Open(
		(FRESH_PFN_RESULT)&CSecondaryPassword::OnInputPasswordFormResult,
		FrENTER);
}

void CSecondaryPassword::OnOpenPasswordLoginDlg()
{
	if (m_pPasswordInputDlg || m_pSelectCertifyDlg)
		return;
	m_pPasswordInputDlg = CreateForm<FrPasswordInputDlg>(g_pFresh->GetManager(),
		this, "passwordinputdlg", NULL);
	m_pPasswordInputDlg->EnableDrag(false);
	m_pPasswordInputDlg->SetDlgType(SECONDPWD_INPUT_LOGIN);
	m_pPasswordInputDlg->Open(
		(FRESH_PFN_RESULT)&CSecondaryPassword::OnInputPasswordFormResult,
		FrENTER);
}

void CSecondaryPassword::OnClosePasswordDlg()
{
	if (m_pPasswordInputDlg)
		m_pPasswordInputDlg->Close(FrOK, true);
}

void CSecondaryPassword::OnOpenSelectCertifyDlg()
{
	if (m_pSelectCertifyDlg)
		return;
	m_pSelectCertifyDlg = CreateForm<FrSelectCertification>(
		g_pFresh->GetManager(), this, "selectcertifydlg", NULL);
	m_pSelectCertifyDlg->EnableDrag(false);
	m_pSelectCertifyDlg->Open(
		(FRESH_PFN_RESULT)&CSecondaryPassword::OnSelectCertifyFormResult, 0);
}

void CSecondaryPassword::OnCloseSelectCertifyDlg()
{
	if (m_pSelectCertifyDlg)
		m_pSelectCertifyDlg->Close(FrOK, true);
}

void CSecondaryPassword::OnRequestLoginPassword(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	std::string password = (const char*)pMsg->param2;
	WSendPacket packet(10);
	packet.EncodeStr(password);
	packet.Send(TO_LOGIN);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
}

void CSecondaryPassword::OnResultLoginPassword(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	int error = pMsg->param2;
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
		return;
	switch (error)
	{
	case 140101:
	case 140102:
	case 140401:
	case 145005:
	case 145006:
	case 730002:
	case 730003:
	{
		char buffer[256] = { 0 };
		sprintf(buffer, "%s : %d",
			_systemmsg::CMessageManager::Instance()->GetMsg(
				(_systemmsg::eMsgIdentifier)119),
			error);
		OnOpenSystemMsg(119, buffer);
	}
	break;
	case 140402:
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x25),
			(int)_systemmsg::CMessageManager::Instance()->GetMsg(
				(_systemmsg::eMsgIdentifier)111),
			0, 0, 0, 0));
		break;
	case 145008:
		OnOpenSystemMsg(110);
		pDoc->IncreseInvalidInputPassword();
		break;
	}
}

void CSecondaryPassword::OnOpenSelfCertifyWebPage()
{
	if (CBrowser::Instance()->IsOpen())
		return;
	COption::Instance()->vApplyLobbyScreenSize();
	CBrowser::Instance()->SetPagetype(CBrowser::WEBPAGE_SELFCERTIFY);
	CBrowser::Instance()->InitializePostArgument();
	CBrowser::Instance()->AddPostArgument("sn", "9");
	CLoginInfo* loginInfo = CLoginInfo::Instance();
	CBrowser::Instance()->AddPostArgument("mn",
		MakeStr("%d", loginInfo->AuthUid()));
	CBrowser::Instance()->OpenPost(
		"https://qa.pangya.gametree.co.kr/InGame/Certify/Default.aspx", false,
		true);
}

void CSecondaryPassword::OnWaitGameServerList()
{
}

void CSecondaryPassword::OnNotifyPasswordInfo(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	eSecondPasswordState state = (eSecondPasswordState)pMsg->param2;
	int day = pMsg->param3;
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
		return;
	pDoc->SetPasswordState(state);
	pDoc->SetRemainDay(day);
}

void CSecondaryPassword::OnRequestAvailableContents(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	bool* pAvailable = (bool*)pMsg->param2;
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
	{
		*pAvailable = false;
		return;
	}
	switch (pDoc->GetPasswordState())
	{
	case 1:
	case 3:
	case 4:
		*pAvailable = true;
		return;
	case 2:
		*pAvailable = false;
		break;
	}
	if (pDoc->GetRemainDay() <= 0)
		*pAvailable = true;
}

void CSecondaryPassword::OnNotifyCannotUseContents()
{
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
		return;
	const char* text = MakeStr(_systemmsg::CMessageManager::Instance()->GetMsg(
								   (_systemmsg::eMsgIdentifier)109),
		pDoc->GetRemainDay());
	_systemmsg::CMessageManager::Instance()->NotifyNormalMessage(text);
}

void CSecondaryPassword::OnNotifySecondPasswordConfirm()
{
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
		return;
	pDoc->SetSecondPasswordConfirm();
}

void CSecondaryPassword::OnRequestQualifyLogin(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	bool* pQualify = (bool*)pMsg->param2;
	CSecondPwdDoc* pDoc = GetDocument<CSecondPwdDoc*>();
	if (pDoc == NULL)
	{
		*pQualify = false;
		return;
	}
	unsigned long provider = CLoginInfo::Instance()->ProvType();
	if (provider == 2)
		*pQualify = pDoc->GetSecondPasswordConfirm() == true;
	else if (provider == 4)
		*pQualify = true;
}

void CSecondaryPassword::OnRequestCheckPasswordValid(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	const std::string* password = (const std::string*)pMsg->param2;
	volatile bool* pValid = (volatile bool*)pMsg->param3;
	*pValid = false;
	switch (_util::VerifyInputPassword(*password))
	{
	case _util::PWD_EMPTY:
		OnOpenSystemMsg(106);
		break;
	case _util::PWD_NOT_NUMBER:
		OnOpenSystemMsg(108);
		break;
	case _util::PWD_TOO_SHORT:
		OnOpenSystemMsg(102);
		break;
	case _util::PWD_TOO_LONG:
		OnOpenSystemMsg(101);
		break;
	default:
		*pValid = true;
		break;
	}
	if (*pValid)
	{
		*pValid = false;
		switch (_util::VerifyStringContents_Limit(*password))
		{
		case _util::PWD_SAME_CHARS:
			OnOpenSystemMsg(112);
			break;
		case _util::PWD_ASCENDING:
			OnOpenSystemMsg(113);
			break;
		case _util::PWD_DESCENDING:
			OnOpenSystemMsg(113);
			break;
		default:
			*pValid = true;
			break;
		}
	}
}

void CSecondaryPassword::OnNotifySystemMsg(int param)
{
	OnOpenSystemMsg(((const MsgObject*)param)->param2);
}

void CSecondaryPassword::OnOpenSystemMsg(int msg)
{
	if (m_pPasswordNotifyDlg)
		return;
	m_pPasswordNotifyDlg = CreateForm<FrPasswordNotifyDlg>(
		g_pFresh->GetManager(), this, "passwordnotify", NULL);
	m_pPasswordNotifyDlg->SetNotifyType(msg);
	m_pPasswordNotifyDlg->Open(
		(FRESH_PFN_RESULT)&CSecondaryPassword::OnSystemMsgFormResult, FrESCAPE);
}

void CSecondaryPassword::OnOpenSystemMsg(int msg, const char* text)
{
	if (m_pPasswordNotifyDlg)
		return;
	m_pPasswordNotifyDlg = CreateForm<FrPasswordNotifyDlg>(
		g_pFresh->GetManager(), this, "passwordnotify", NULL);
	m_pPasswordNotifyDlg->SetNotifyType(msg);
	m_pPasswordNotifyDlg->SetMessage(text);
	m_pPasswordNotifyDlg->Open(
		(FRESH_PFN_RESULT)&CSecondaryPassword::OnSystemMsgFormResult, FrESCAPE);
}

bool CSecondaryPassword::OnPasswordExplaneFormResult(int result, FrForm* pForm)
{
	m_pExplanePasswordDlg = NULL;
	return true;
}

bool CSecondaryPassword::OnInputPasswordFormResult(int result, FrForm* pForm)
{
	m_pPasswordInputDlg = NULL;
	return true;
}

bool CSecondaryPassword::OnSelectCertifyFormResult(int result, FrForm* pForm)
{
	m_pSelectCertifyDlg = NULL;
	return true;
}

bool CSecondaryPassword::OnSystemMsgFormResult(int result, FrForm* pForm)
{
	m_pPasswordNotifyDlg = NULL;
	return true;
}

// 983: _gateway::CSecondPwdDoc nao existe no exe (sem RTTI); movida do gatewaydoc.cpp (D173) -- so este arquivo a usa.

CSecondPwdDoc::CSecondPwdDoc()
	: m_pHandler(NULL)
{
}

bool CSecondPwdDoc::Initialize()
{
	m_pHandler = _util::NewCreator<CSecondaryPassword>::CreateObject();
	if (m_pHandler)
	{
		m_pHandler->Initialize();
	}

	InitializeForLogout();

	_systemmsg::
		sMsgInfo
			msgInfo
				[] = {

					{ (_systemmsg::eMsgIdentifier)100,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb8\xa6 \xc8\xae\xc0\xce\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."																																																																																																																																																																																																												 },
					{ (_systemmsg::eMsgIdentifier)101,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb8\xa6 8\xc0\xda\xb8\xae \xc0\xcc\xc7\xcf\xb7\xce \xc0\xd4\xb7\xc2\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."																																																																																																																																																																																																	  },
					{ (_systemmsg::eMsgIdentifier)102,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb8\xa6 5\xc0\xda\xb8\xae \xc0\xcc\xbb\xf3\xc0\xb8\xb7\xce \xc0\xd4\xb7\xc2\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."																																																																																																																																																																																															  },
					{ (_systemmsg::eMsgIdentifier)103,
                     "\xc8\xad\xb8\xe9\xc0\xc7 \xbc\xfd\xc0\xda\xc6\xd0\xb5\xe5\xb8\xa6 \xc0\xcc\xbf\xeb\xc7\xd8 \xc0\xd4\xb7\xc2\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."																																																																																																																																																																																															   },
					{ (_systemmsg::eMsgIdentifier)104,
                     "2\xc2\xf7 \xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xbb\xfd\xbc\xba\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."																																																																																																																																																																																																																},
					{ (_systemmsg::eMsgIdentifier)105,
                     "2\xc2\xf7 \xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xbb\xfd\xbc\xba\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. : "																																																																																																																																																																																																											},
					{ (_systemmsg::eMsgIdentifier)106,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb8\xa6 \xc0\xd4\xb7\xc2\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."																																																																																																																																																																																																												 },
					{ (_systemmsg::eMsgIdentifier)107,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xc0\xd4\xb7\xc2 \xc1\xa4\xba\xb8\xbf\xcd \xc8\xae\xc0\xce \xc1\xa4\xba\xb8\xb0\xa1 \xc0\xcf\xc4\xa1\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."																																																																																																																																																																																													 },
					{ (_systemmsg::eMsgIdentifier)108,
                     "\xbc\xfd\xc0\xda\xb0\xa1 \xbe\xc6\xb4\xd1 \xb9\xae\xc0\xda\xb0\xa1 \xc0\xd4\xb7\xc2\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe7\xc0\xd4\xb7\xc2\xc7\xcf\xbc\xc5\xbe\xdf \xc7\xd5\xb4\xcf\xb4\xd9."																																																																																																																																																																																									 },
					{ (_systemmsg::eMsgIdentifier)109,
                     "\xbe\xc6\xc1\xf7 \xba\xbb\xc0\xce \xc0\xce\xc1\xf5 \xc0\xfd\xc2\xf7\xb8\xa6 \xb0\xc5\xc4\xa1\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9.\x0a\xc7\xd8\xb4\xe7 \xb1\xe2\xb4\xc9\xc0\xc7 \xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3\xb1\xee\xc1\xf6 \xbe\xd5\xc0\xb8\xb7\xce %d\xc0\xcf \xb3\xb2\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9.\x0a\xba\xbb\xc0\xce \xc0\xce\xc1\xf5\xc0\xbb \xc7\xcf\xbd\xc3\xb8\xe9 \xc1\xef\xbd\xc3 \xc7\xd8\xb4\xe7 \xb1\xe2\xb4\xc9\xc0\xc7 \xbb\xe7\xbf\xeb\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."                                                                                                                                                                                                                                                                                                                                                                                                                                              },
					{ (_systemmsg::eMsgIdentifier)110,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xc0\xcf\xc4\xa1\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xbf\xac\xbc\xd3 5\xc8\xb8 \xbf\xc0\xb7\xf9 \xbd\xc3 \xc5\xac\xb6\xf3\xc0\xcc\xbe\xf0\xc6\xae\xb0\xa1 \xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1\xb5\xcb\xb4\xcf\xb4\xd9."																																																																																																																																																																								 },
					{ (_systemmsg::eMsgIdentifier)111,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 5\xc8\xb8 \xbf\xc0\xb7\xf9\xc0\xd4\xb4\xcf\xb4\xd9.\x0a\xc6\xce\xbe\xdf\xb0\xa1 \xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1\xb5\xcb\xb4\xcf\xb4\xd9."																																																																																																																																																																																																  },
					{ (_systemmsg::eMsgIdentifier)112,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xc0\xfc\xc3\xbc\xb8\xa6 \xb5\xbf\xc0\xcf\xc7\xd1 \xbc\xfd\xc0\xda\xb7\xce \xc0\xd4\xb7\xc2\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."																																																																																																																																																																																													 },
					{ (_systemmsg::eMsgIdentifier)113,
                     "\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xc0\xfc\xc3\xbc\xb8\xa6 \xbf\xac\xbc\xd3\xb5\xc8 \xbc\xfd\xc0\xda\xb7\xce \xc0\xd4\xb7\xc2\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."																																																																																																																																																																																													 },
					{ (_systemmsg::eMsgIdentifier)114,
                     "2\xc2\xf7 \xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb4\xc2 \xc8\xb8\xbf\xf8\xb4\xd4\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb \xb9\xd7 \xb0\xb3\xc0\xce\xc1\xa4\xba\xb8 \xba\xb8\xc8\xa3\xb8\xa6 \xc0\xa7\xc7\xd8 \xb5\xb5\xc0\xd4\xb5\xc8 \xbd\xc3\xbd\xba\xc5\xdb\xc0\xd4\xb4\xcf\xb4\xd9.\x0a"
						"2\xc2\xf7 \xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb8\xa6 \xbc\xb3\xc1\xa4\xc7\xcf\xbc\xc5\xbe\xdf\xb8\xb8 \xb0\xd4\xc0\xd3 \xb7\xce\xb1\xd7\xc0\xce\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9.\x0a\x0a\x0a\x0a\x0a\x0a\xb4\xd9\xc0\xbd\xb0\xfa \xb0\xb0\xc0\xba \xb1\xe2\xb4\xc9\xc0\xbb \xc1\xef\xbd\xc3 \xc0\xcc\xbf\xeb\xc7\xcf\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad\xb4\xc2 \xba\xbb\xc0\xce\xc0\xce\xc1\xf5\xc0\xbb \xbf\xcf\xb7\xe1\xc7\xcf\xbc\xc5\xbe\xdf \xc7\xd5\xb4\xcf\xb4\xd9.\x0a\\c0xffff0000\\c\xb1\xb8\xb8\xc5, \xbc\xb1\xb9\xb0, \xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1\xb0\xc5\xb7\xa1, \xbf\xec\xc6\xed\xc7\xd4(\xc3\xb7\xba\xce), \xc0\xfc\xb1\xa4\xc6\xc7, \xb4\xd0\xb3\xd7\xc0\xd3 \xba\xaf\xb0\xe6\x0a\\c0xff000000\\c\xba\xbb\xc0\xce\xc0\xce\xc1\xf5\xc0\xbb \xc7\xcf\xc1\xf6 \xbe\xca\xc0\xb8\xbd\xc5 \xb0\xe6\xbf\xec\xbf\xa1\xb4\xc2 10\xc0\xcf \xb0\xe6\xb0\xfa \xc8\xc4\xbf\xa1 \xc0\xcc\xbf\xeb\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9." },
					{ (_systemmsg::eMsgIdentifier)115,
                     "\xba\xbb\xc0\xce \xc0\xce\xc1\xf5\xc0\xbb \xc7\xcf\xb8\xe9 \xc1\xef\xbd\xc3\x0a\xb8\xf0\xb5\xe7 \xb1\xe2\xb4\xc9\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6\x0a\xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."																																																																																																																																																																																									 },
					{ (_systemmsg::eMsgIdentifier)116,
                     "\xc1\xf6\xb1\xdd\xc0\xb8\xb7\xce\xba\xce\xc5\xcd 10\xc0\xcf\xb0\xa3\x0a\xb4\xd9\xc0\xbd\xb0\xfa \xb0\xb0\xc0\xba \xb1\xe2\xb4\xc9\xc0\xc7 \xc0\xcc\xbf\xeb\xc0\xcc\x0a\xc1\xa6\xc7\xd1\xb5\xcb\xb4\xcf\xb4\xd9.\x0a\\c0xffff0000\\c\xb1\xb8\xb8\xc5, \xbc\xb1\xb9\xb0, \xb0\xb3\xc0\xce\xbb\xf3\xc1\xa1\xb0\xc5\xb7\xa1,\x0a\xbf\xec\xc6\xed\xc7\xd4(\xc3\xb7\xba\xce), \xc0\xfc\xb1\xa4\xc6\xc7,\x0a\xb4\xd0\xb3\xd7\xc0\xd3 \xba\xaf\xb0\xe6"																																																																																																																															  },
					{ (_systemmsg::eMsgIdentifier)117,
                     "\xc7\xf6\xc0\xe7 \xc8\xb8\xbf\xf8\xb4\xd4\xc0\xc7 \xb0\xe8\xc1\xa4\xc0\xba 2\xc2\xf7 \xba\xf1\xb9\xd0 \xb9\xf8\xc8\xa3\xb0\xa1 \xbb\xfd\xbc\xba\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\x0a \\c0xffff0000\\c\xc6\xce\xbe\xdf \xc8\xa8\xc6\xe4\xc0\xcc\xc1\xf6> \xb0\xed\xb0\xb4\xbc\xbe\xc5\xcd> 1:1\xb9\xae\xc0\xc7\\c0xffffffff\\c\xb8\xa6 \xc5\xeb\xc7\xd8\x0a\xbd\xc5\xb0\xed\xc7\xd8 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."																																																																																																																											  },
					{ (_systemmsg::eMsgIdentifier)118,
                     "2\xc2\xf7 \xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xc1\xa4\xba\xb8 \xc8\xa3\xc3\xe2\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. : "																																																																																																																																																																																																						   },
					{ (_systemmsg::eMsgIdentifier)119,
                     "2\xc2\xf7 \xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3 \xb7\xce\xb1\xd7\xc0\xce\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. : "																																																																																																																																																																																																									},
					{ (_systemmsg::eMsgIdentifier)20, NULL }, // B1408: contagem lida em info[num].id (ver InsertMessageGroup)
    };

	_systemmsg::CMessageManager::Instance()->InsertMessageGroup(msgInfo,
		sizeof(msgInfo) / sizeof(msgInfo[0]) - 1);

	return true;
}

bool CSecondPwdDoc::Release()
{
	if (m_pHandler)
	{
		delete m_pHandler;
		m_pHandler = NULL;
	}

	return true;
}

void CSecondPwdDoc::InitializeForLogout()
{
	m_passwordState = (eSecondPasswordState)0;
	m_remainDay = 0;
	m_invalidInputCount = 0;
	m_bSecondPasswordConfirm = false;
	m_bCertifyFlag = false;
}
