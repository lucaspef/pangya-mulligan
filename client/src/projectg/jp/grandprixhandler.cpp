// JP 983: unidade 00559cc0–0055aa10 (_gateway::_grandprix::CGrandPrixHandler). Fonte reconstruída do 983 (classe nova, sem par no 645).
// Sessão B65: registro, construtor, destrutor, tabela de mensagens, Execute e as 14 funções. O CGrandPrixTimeBase (0055a9d0–0055aa10)
// fica no fim, como no 983 (a unidade grandprixtimebase da F3.7.1 é este pedaço).
#include "minatl.h"
#include "projectg.h"
#include "grandprixhandler.h"
#include "grandprixopendlg.h"
#include "grandprixlimitdlg.h"
#include "grandprixtimebase.h"
#include "gatewaydocjp.h"
#include "shareddocjp.h"
#include "gamestringmanager.h"
#include "initdelegate.h"
#include "taskmanager.h"
#include "packet.h"
#include "fresh.h"
#include "frwndmanager.h"

using namespace _gateway;
using namespace _gateway::_grandprix;

extern Fresh* g_pFresh;

IObject* CGrandPrixHandlerMakeInstance() // 0055a910
{
	return new CGrandPrixHandler;
}

struct __sCGrandPrixHandler
{
	__sCGrandPrixHandler() // 0055a980
	{
		ObjectFactory().AddObjectFunctor(CGrandPrixHandlerMakeInstance, "CGrandPrixHandler");
	}
};

const WRTTI CGrandPrixHandler::m_RTTI("CGrandPrixHandler", &IObject::m_RTTI);
static __sCGrandPrixHandler __implCGrandPrixHandler;

static void ReleaseGrandPrixHandler() // 00559cc0 (vazia)
{
}

void CGrandPrixHandler::OnMsg06(const MsgObject& msg)
{
}

static bool AddReleaseGrandPrixHandler() // 00559d40
{
	static bool s_bAdded; // 00e751b5
	if (!s_bAdded)
	{
		CInitDelegate::AddReleaseFunction(ReleaseGrandPrixHandler);
		s_bAdded = true;
	}
	return s_bAdded;
}

void CGrandPrixHandler::On03()
{
	Doc983()->Fn555140_983();
}

void CGrandPrixHandler::On0d()
{
	Doc983()->Fn552750_983();
}

void CGrandPrixHandler::OnReadList(const MsgObject& msg)
{
	CGrandPrixDocument* pDoc = Doc983();
	pDoc->ReadList983(MSGJP(msg).Param2<WReceivedPacket*>());
	pDoc->RefreshTicket983();
}

void CGrandPrixHandler::OnUpdatePair(const MsgObject& msg)
{
	CGrandPrixDocument* pDoc = Doc983();
	pDoc->UpdatePair983(MSGJP(msg).Param2<WReceivedPacket*>());
	pDoc->RefreshTicket983();
}

void CGrandPrixHandler::OnRead02(const MsgObject& msg)
{
	Doc983()->Read5564a0_983(MSGJP(msg).Param2<WReceivedPacket*>());
}

void CGrandPrixHandler::OnOpen(const MsgObject& msg)
{
	const sGPEnter983* pEnter = MSGJP(msg).Param2<const sGPEnter983*>();
	m_pOpenDlg = CreateForm<FrGrandPrixOpenDlg>(g_pFresh->GetManager(), this, "grandprixopen", NULL);
	m_pOpenDlg->Init983(pEnter);
	m_pOpenDlg->Open(NULL, 1);
}

void CGrandPrixHandler::OnLimit(const MsgObject& msg)
{
	sGPRoomItem983* pItem = MSGJP(msg).Param2<sGPRoomItem983*>();
	m_pLimitDlg = CreateForm<FrGrandPrixLimitDlg>(g_pFresh->GetManager(), this, "gplimitdlg", NULL);
	m_pLimitDlg->Init983(pItem->pInfo, pItem->flags);
	m_pLimitDlg->Open(NULL, 3);
}

void CGrandPrixHandler::OnGetItemCount(const MsgObject& msg)
{
	int typeId = MSGJP(msg).Param2<int>();
	int* pCount = MSGJP(msg).Param3<int*>();
	sItemInfo* pItem = Doc()->FindMyItemByTypeId983(typeId);
	if (pItem)
		*pCount = pItem->Common[0];
}

void CGrandPrixHandler::OnSetTypeId(const MsgObject& msg)
{
	Doc983()->SetTypeId983(MSGJP(msg).Param2<unsigned long>());
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)GetGameString(0x7540), 0, 0, 0, 0));
	On03();
}

void CGrandPrixHandler::OnClearTimer(const MsgObject& msg)
{
	Doc983()->ClearTimer983(MSGJP(msg).Param2<int>());
}

void CGrandPrixHandler::On0b(const MsgObject& msg)
{
	Doc983()->Fn5566b0_983(MSGJP(msg).Param2<sRoomInfo*>());
}

void CGrandPrixHandler::On0c(const MsgObject& msg)
{
	Doc983()->Fn5566e0_983(MSGJP(msg).Param2<sRoomInfo*>());
}

void CGrandPrixHandler::OnRead05(const MsgObject& msg)
{
	Doc983()->Read5582b0_983(MSGJP(msg).Param2<WReceivedPacket*>());
}

bool CGrandPrixHandler::Execute(const MsgObject& msg)
{
	SetFunctionType(msg.param1);
	return Exec<CGrandPrixHandler, const MsgObject&>(this, msg);
}

static void RegisterGrandPrixHandlerActor() // 0055a3b0
{
	ActorRegistry983().Add(CGrandPrixHandler::m_RTTI.GetName(),
		ObjectFactory().CreateObject(CGrandPrixHandler::m_RTTI.GetName()));
}

bool CGrandPrixHandler::Initialize()
{
#define GP_ARG(id, f) AddFunctionType(id, FUNCTIONTYPE_ARG, (FUNCTION_PTR)(void (CGrandPrixHandler::*)())&CGrandPrixHandler::f)
#define GP_VOID(id, f) AddFunctionType(id, FUNCTIONTYPE_VOID, (FUNCTION_PTR)(void (CGrandPrixHandler::*)())&CGrandPrixHandler::f)
	GP_ARG(0x00, OnReadList);
	GP_ARG(0x01, OnUpdatePair);
	GP_ARG(0x02, OnRead02);
	GP_VOID(0x03, On03);
	GP_ARG(0x06, OnMsg06);
	GP_ARG(0x07, OnOpen);
	GP_ARG(0x08, OnLimit);
	GP_ARG(0x04, OnGetItemCount);
	GP_ARG(0x09, OnSetTypeId);
	GP_ARG(0x05, OnRead05);
	GP_ARG(0x0b, On0b);
	GP_ARG(0x0c, On0c);
	GP_VOID(0x0d, On0d);
#undef GP_ARG
#undef GP_VOID
	return true;
}

static bool AddInitGrandPrixHandler() // 0055a7a0
{
	static bool s_bAdded; // 00e751b4
	if (!s_bAdded)
	{
		CInitDelegate::AddInitFunction(RegisterGrandPrixHandlerActor);
		s_bAdded = true;
	}
	return s_bAdded;
}

CGrandPrixHandler::CGrandPrixHandler()
	: m_pLimitDlg(NULL), m_2c(false)
{
}

static bool s_bInitAdded = AddInitGrandPrixHandler();
static bool s_bReleaseAdded = AddReleaseGrandPrixHandler();

// ------------------------------------------------------------------------------------------------------------------------
// CGrandPrixTimeBase (0055a9d0–0055aa10)

// construtor, Set e Get: inline em grandprixtimebase.h (B67)

void CGrandPrixTimeBase::OnChange()
{
}
