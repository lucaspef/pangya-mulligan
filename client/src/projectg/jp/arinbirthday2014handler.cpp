// JP 983: unidade 00477bf0–00478cc2 (_gateway::CArinBirthDay2014Handler, o ator do aniversário da Arin 2014; ver
// include/jp/arinbirthday2014handler.h). Classe nova do 983 (não existe no 645). Sessão B127, F3.7.12 item (2). O corte da F3.7.1
// (00478a30) só pegava Execute/AddFunctionType/Initialize: os 6 métodos da tabela estão antes, no meio dos moldes da RF
// (RFFormModalBase<RFWndEx>::DoModal 004783e0) e do map<int, sFunctionHandler> (00477d40–00478a00) que o compilador gera sozinho.
#include "minatl.h"
#include "projectg.h"
#include "packet.h"
#include "shareddoc.h"
#include "arinbirthday2014handler.h"
#include "rf2014arinbirthdaydlg.h"

using namespace _gateway;

// 00477bf0
void CArinBirthDay2014Handler::OnStep3_983()
{
	if (RF2014ArinBirthdayDlg::s_pInstance)
	{
		RF2014ArinBirthdayDlg::s_pInstance->SetStep983(3);
		RF2014ArinBirthdayDlg::s_pInstance->UpdateStep983();
	}
}

// 00477c10
void CArinBirthDay2014Handler::OnStep5_983()
{
	if (RF2014ArinBirthdayDlg::s_pInstance)
	{
		RF2014ArinBirthdayDlg::s_pInstance->SetStep983(5);
		RF2014ArinBirthdayDlg::s_pInstance->UpdateStep983();
	}
}

// 00477c30
void CArinBirthDay2014Handler::OnMissionResult983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = (WReceivedPacket*)msg.param2;
	int result = pPacket->Decode4();
	if (RF2014ArinBirthdayDlg::s_pInstance)
	{
		RF2014ArinBirthdayDlg::s_pInstance->OpenMissionInfo983();
		RF2014ArinBirthdayDlg::s_pInstance->ShowStage983(10);
		RF2014ArinBirthdayDlg::s_pInstance->SetMissionResult983(result - 0x15);
	}
}

// 00477c80
void CArinBirthDay2014Handler::OnMapResult983()
{
	if (RF2014ArinBirthdayDlg::s_pInstance)
	{
		RF2014ArinBirthdayDlg::s_pInstance->OpenMapMissionInfo983();
		RF2014ArinBirthdayDlg::s_pInstance->ShowMapResult983();
	}
}

// 00478720
void CArinBirthDay2014Handler::OnOpen983()
{
	int step;
	if (Doc()->FindMyItemByTypeId983(0x1be00013))
		step = 9;
	else
		step = Doc()->FindMyItemByTypeId983(0x1a0002b8) ? 4 : 0;
	if (Doc()->FindMyItemByTypeId983(0x1be00014))
		step = 0xb;
	if (RF2014ArinBirthdayDlg::s_pInstance)
		return;
	RF2014ArinBirthdayDlg* pDlg = RF2014ArinBirthdayDlg::Create983(NULL);
	if (pDlg)
	{
		pDlg->SetDocument983(Doc983());
		pDlg->SetStep983(step);
		pDlg->UpdateStep983();
		pDlg->CenterWindow(NULL);
		pDlg->DoModal();
	}
}

// 004787c0
void CArinBirthDay2014Handler::OnMissionList983(const MsgObject& msg)
{
	CArinBirthDay2014Doc* pDoc = Doc983();
	if (pDoc)
	{
		pDoc->Decode983((WReceivedPacket*)msg.param2);
		if ((int)pDoc->GetMissions983().size() >= 5)
			OnOpen983();
	}
}

// 00478a30
bool CArinBirthDay2014Handler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec<CArinBirthDay2014Handler, const MsgObject&>(this, msg);
}

// 00478ab0
bool CArinBirthDay2014Handler::Initialize()
{
	AddFunctionType(0x00, FUNCTIONTYPE_VOID, (FUNCTION_PTR)&CArinBirthDay2014Handler::OnOpen983);
	AddFunctionType(0x01, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CArinBirthDay2014Handler::*)())&CArinBirthDay2014Handler::OnMissionList983);
	AddFunctionType(0x02, FUNCTIONTYPE_VOID, (FUNCTION_PTR)&CArinBirthDay2014Handler::OnStep3_983);
	AddFunctionType(0x03, FUNCTIONTYPE_VOID, (FUNCTION_PTR)&CArinBirthDay2014Handler::OnStep5_983);
	AddFunctionType(0x04, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CArinBirthDay2014Handler::*)())&CArinBirthDay2014Handler::OnMissionResult983);
	AddFunctionType(0x05, FUNCTIONTYPE_VOID, (FUNCTION_PTR)&CArinBirthDay2014Handler::OnMapResult983);
	AddFunctionType(0x06, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CArinBirthDay2014Handler::*)())&CArinBirthDay2014Handler::OnIgnore983);
	return true;
}
