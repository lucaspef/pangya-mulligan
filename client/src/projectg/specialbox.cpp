#include "minatl.h"
#include "specialbox.h"
#include "specialboxdlg.h"
#include "gatewayactor.h"
#include "messagemanager.h"
#include "fresh.h"
#include "frdesktop.h"
extern Fresh* g_pFresh;
using namespace _gateway;

bool CSpecialBoxHandler::Initialize()
{
	AddFunctionType(0, FUNCTIONTYPE_VOID,
		(void (FunctionMapper::*)())&CSpecialBoxHandler::OnReqOpenBox);
	AddFunctionType(1, FUNCTIONTYPE_ARG,
		(void (FunctionMapper::*)())(void (
			CSpecialBoxHandler::*)())&CSpecialBoxHandler::OnReqWarningMsg);

	return true;
}

bool CSpecialBoxHandler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec(this, (int)&msg);
}

bool CSpecialBoxHandler::OnSpecialBoxCloseCallback(int result, FrForm* pForm)
{
	m_pSpecialBoxDlg = NULL;

	return true;
}

bool CSpecialBoxHandler::VerifyHaveKeyItem()
{
	std::list<sItemInfo>& itemList = Doc()->m_myItemList;
	std::list<sItemInfo>::iterator it;
	for (it = itemList.begin(); it != itemList.end(); ++it)
	{
		if ((*it).tid == 0x1A00015C)
			break;
	}
	return it != Doc()->m_myItemList.end() ? true : false;
}

// 983 00921660: so abre o "notify_yesno" (callback 00921520); a checagem da chave e o specialBoxOpen
// ficam no resultado do dialogo (OnReqOpenBoxConfirm983).
void CSpecialBoxHandler::OnReqOpenBox()
{
	FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
		"notify_yesno", NULL);
	pForm->SetMessage(K2L_Compatibility("\xbd\xba\xc7\xc9\xc5\xa5\xba\xea\xb8\xa6 \xbf\xad\xbe\xee \xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"), false);
	pForm->Open((FRESH_PFN_RESULT)&CSpecialBoxHandler::OnReqOpenBoxConfirm983, 3);
}

// 983 00921520: com "sim" (1): sem a chave (tid 0x1A00015C) manda a msg 0x200B3C e fecha o notify_yesno; com a chave abre o specialBoxOpen.
bool CSpecialBoxHandler::OnReqOpenBoxConfirm983(int result, FrForm* pForm)
{
	if (result != 1)
		return true;

	if (!VerifyHaveKeyItem())
	{
		CTaskManager::Instance()->PostMsg(NULL, s_gatewayActor[1], M983(0x276), 1,
			0x200B3C, 0, 0);
		FrForm* pYesNo = DYNAMIC_CAST(FrForm, g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"notify_yesno"));
		if (pYesNo)
			pYesNo->Close((eFormRet)2, true);
		return false;
	}

	m_pSpecialBoxDlg = CreateForm<FrSpecialBoxDlg>(g_pFresh->GetManager(), this,
		"specialBoxOpen", NULL);

	const char* msg = _systemmsg::CMessageManager::Instance()->GetMsg(
		(_systemmsg::eMsgIdentifier)203);

	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(0x1A00015C);

	m_pSpecialBoxDlg->SetMessage(MakeStr(msg, pItem->Name));
	m_pSpecialBoxDlg->Open(
		(FRESH_PFN_RESULT)&CSpecialBoxHandler::OnSpecialBoxCloseCallback, 1);
	return true;
}

void CSpecialBoxHandler::OnReqWarningMsg(int param)
{
	const MsgObject* pMsg = (const MsgObject*)param;
	int type = 0;

	switch (pMsg->param2)
	{
	case 0x200B3B:
		type = 200;
		break;
	case 0x200B3C:
		type = 201;
		break;
	case 0x200B3D:
		type = 202;
		break;
	}

	m_pSpecialBoxDlg = CreateForm<FrSpecialBoxDlg>(g_pFresh->GetManager(), this,
		"specialBoxOpen", NULL);
	m_pSpecialBoxDlg->SetMessageType(type);
	m_pSpecialBoxDlg->DisbleProgressBar();
	m_pSpecialBoxDlg->Open(
		(FRESH_PFN_RESULT)&CSpecialBoxHandler::OnSpecialBoxCloseCallback, 1);
}
