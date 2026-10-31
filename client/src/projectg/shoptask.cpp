#include "minatl.h"
#include "vc10_layout.h"
#include "wlocalize.h"
#include "shoptask.h"
#include "actor.h"
#include "fresh.h"
#include "gatewayactor.h"
#include "rtti.h"
#include "itemwindow.h"
#include "taskmain.h"
#include "../shared/localize.h"

extern Fresh* g_pFresh;
extern int g_iRecvPacketType;

#define DOC_AT(type, offset) (*(type*)((char*)Doc() + (offset)))

// PKT (frente C, lote 11): era DOC_AT(std::list<sItemInfo>, 0x11a0), deslocamento do KR 645; no 983 o caso 0x68 (005e1fc2)
// percorre a lista de CSharedDoc+0x375c
C_ASSERT_VC10(offsetof(CSharedDoc, m_myItemList) == 0x375c);
#define DOC_ITEMS (Doc()->m_myItemList)

// 983 (ex.: 005e11a4): todo texto passa por K2L_Compatibility (pedido C18)
#define SHOPMSG(id, str) \
	GetActor("Shop") << MsgObject(NULL, id, (int)K2L_Compatibility(str), 0, 0, 0, 0)

extern const float g_shopFloatPair[2] = { 3.402823466e+38f, 1.0e-5f };

IMPLEMENT_OBJECT(CShopTask, CTask)

CShopTask::CShopTask()
{
	IActor::m_defaultSkipLayer = 0;
	IActor::m_pLayer = NULL;
}

CShopTask::~CShopTask()
{
}

void CShopTask::Init(const char* wallPaper)
{
	CTask::Init("shop_background.jpg");

	SetWhisper(false);
}

void CShopTask::Register()
{
	SetMainActor(AddActor("CShopMain", "Shop", 0, ""));
	AddActor(s_gatewayActor[0], s_gatewayActor[1], 0, GetRTTI()->GetName());

	CTask* pTask = AfxGetPreservedTask();
	if (pTask)
	{
		IActor* pActor = pTask->GetActor("TutorialMain");
		if (pActor)
		{
			int a = 0, b = 0;
			pActor << MsgObject(NULL, M983(0x14e), (int)&a, (int)&b, 0, 0, 0);
			AddActor((const char*)a, (const char*)b, 0, "");
		}
	}
}

void CShopTask::Load()
{
	SetProgressBar(false);
	SetTip(false);

	// PKT (frente C, lote 42): o 983 nao tem CShopTask (sem RTTI; a fabrica so registra a CLobbyTask, 00abf540): a loja e a
	// CShopWidget (vtable 00d12380) aberta no lobby pelo resultado 006572f0 do CTaskMain, e nada nesse caminho manda pacote.
	// O 0xb7 do KR 645 (entrada no my room) no 983 so sai do S5::UTIL::SendRealMyRoomEnter983 (00b2df70, chamado pelo
	// CTaskMain::ChangeRealMyRoom) e o 0xc4 nao existe no 983: os dois envios sairam daqui.

	CTask::Load();
}

void CShopTask::Process(float delta)
{
	g_pFresh->OnProcess(delta);
	CTask::Process(delta);
}

void CShopTask::Display()
{
	if (!Doc()->m_bBackgroundVideo)
		CTask::Display();
	g_pFresh->OnDisplay(Doc()->m_bBackgroundVideo != false);
}

// PKT (frente C, lote 43): o 983 nao tem CShopTask (sem RTTI; a fabrica so registra a CLobbyTask, 00abf540). A loja e a
// CShopWidget (include/jp/shopwidget.h, frente B, B109) aberta por cima do lobby, e o despachante 005e0ff0 e o
// CShopWidget::OnPacket (src/projectg/jp/shopwidget.cpp), alcancado pelo repasse do CTask::OnPacketCommon (009c20d7). O corpo
// do 645 que ficava aqui (lotes 11/23 da C) passou inteiro para la; como o CRealMyRoomTask::OnPacket da C46, sobra o repasse.
int CShopTask::OnPacket(WReceivedPacket& packet)
{
	return OnPacketCommon(packet);
}
