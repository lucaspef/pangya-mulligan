// JP 983: unidade 0082e330–0082e940 (_gateway::CWorkShopEvent2013Handler, o ator do evento da oficina de tacos de 2013; ver
// include/jp/workshopevent2013handler.h). Classe nova do 983 (existe no 839, não no 645). Sessão B126. O CreateForm<FrWorkShopEventDlg>
// 0082e350 e o Exec<> 0082e5a0 o compilador gera sozinho.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "packet.h"
#include "workshopevent2013handler.h"
#include "workshopeventdlg.h"

extern Fresh* g_pFresh; // 00f02340

using namespace _gateway;

// 0082e330
bool CWorkShopEvent2013Handler::OnDlgResult983(int ret, FrForm* pForm)
{
	m_pDlg = NULL;
	return true;
}

// 0082e420
void CWorkShopEvent2013Handler::OnOpen983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = (WReceivedPacket*)msg.param2;
	int myHoles = pPacket->Decode4();
	int openedBoxes = pPacket->Decode4();
	int totalHoles = pPacket->Decode4();
	int totalGoal = pPacket->Decode4();
	if (m_pDlg)
		return;
	m_pDlg = CreateForm<FrWorkShopEventDlg>(g_pFresh->GetManager(), this, "WorkShopEvent2013", NULL);
	if (m_pDlg)
	{
		m_pDlg->EnableDrag(false);
		m_pDlg->SetData983(myHoles, openedBoxes, totalHoles, totalGoal);
		m_pDlg->Open((FRESH_PFN_RESULT)&CWorkShopEvent2013Handler::OnDlgResult983, 1);
	}
}

// 0082e4d0
void CWorkShopEvent2013Handler::OnUpdate983(const MsgObject& msg)
{
	WReceivedPacket* pPacket = (WReceivedPacket*)msg.param2;
	int myHoles = pPacket->Decode4();
	int openedBoxes = pPacket->Decode4();
	int totalHoles = pPacket->Decode4();
	int totalGoal = pPacket->Decode4();
	if (m_pDlg)
	{
		m_pDlg->Update983(true, myHoles, openedBoxes, totalHoles, totalGoal);
		return;
	}
	m_pDlg = CreateForm<FrWorkShopEventDlg>(g_pFresh->GetManager(), this, "WorkShopEvent2013", NULL);
	if (m_pDlg)
	{
		m_pDlg->EnableDrag(false);
		m_pDlg->SetData983(myHoles, openedBoxes, totalHoles, totalGoal);
		m_pDlg->Open((FRESH_PFN_RESULT)&CWorkShopEvent2013Handler::OnDlgResult983, 1);
	}
}

// 0082e6c0
bool CWorkShopEvent2013Handler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec<CWorkShopEvent2013Handler, const MsgObject&>(this, msg);
}

// 0082e6e0
bool CWorkShopEvent2013Handler::Initialize()
{
	AddFunctionType(0x00, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CWorkShopEvent2013Handler::*)())&CWorkShopEvent2013Handler::OnOpen983);
	AddFunctionType(0x01, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CWorkShopEvent2013Handler::*)())&CWorkShopEvent2013Handler::OnUpdate983);
	AddFunctionType(0x02, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CWorkShopEvent2013Handler::*)())&CWorkShopEvent2013Handler::OnClose983);
	return true;
}

// 0082e7d0 (chamado pelo Initialize do CWorkShop2013Doc, 009202e0, e por 009200f0)
CWorkShopEvent2013Handler::CWorkShopEvent2013Handler()
	: m_pDlg(NULL)
{
}
