// JP 983: unidade 005e0ca0–005e30a0 (CShopWidget). Classe nova do 983; ver include/jp/shopwidget.h. Sessão B109.
// Ficam fora (cópias fora de linha de moldes usados pelo `OnPacket`): 005e0d00 / 005e0d20 (== / != de iterador), 005e0f60,
// 005e0f70, 005e0f80, 005e0fa0, 005e0fb0.
#include "minatl.h"
#include "vc10_layout.h"
#include "wlocalize.h"
#include "projectg.h"
#include "actor.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "wview.h"
#include "frform.h"
#include "taskform.h"
#include "../shared/localize.h"
#include "lobbyjp.h"
#include "shopwidget.h"
#include "refresh/rflobbydesktop.h"

extern int g_iRecvPacketType; // 00f02494: tipo do último pacote lido (C67.1)

// 983 (ex.: 005e11a4): todo texto passa por K2L_Compatibility e vai ao ator principal (no 645, ao ator "Shop" da CShopTask)
// PKT (frente C, lote 11): a lista de itens do 983 é a de CSharedDoc+0x375c
#define DOC_ITEMS (Doc()->m_myItemList)

#define SHOPMSG(id, str) \
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, id, (int)K2L_Compatibility(str), 0, 0, 0, 0))

// 00e12e58 / 00e12e74: dois textos montados pelo mesmo inicializador estático (00cd1f90); o 1º não tem leitor no 983
static const std::string s_widgetName("CShopWidget");
static const std::string s_formName("shopmain");

IMPLEMENT_OBJECT(CShopWidget, CTaskWidget)

// 005e0ca0
CShopWidget::CShopWidget()
{
}

// 005e0cd0
void CShopWidget::Close()
{
}

// 005e0ce0
void CShopWidget::Process(float delta)
{
	CTaskWidget::Process(delta);
}

// 005e0e00
CShopWidget::~CShopWidget()
{
}

// 005e0e80: igual à CMyRoomWidget::Activate (005a7790)
void CShopWidget::Activate()
{
	if (m_pForm)
	{
		if (AfxGetTask()->GetActor("Lobby"))
		{
			WRect rect = m_pForm->GetRect();
			rect.x = (g_view->GetWidth() - 800.0) * 0.3f + rect.x;
			m_pForm->SetRect(rect);
		}
		m_pForm->Open(NULL, 1);
	}
	CTaskWidget::Activate();
}

// 005e0fd0
void CShopWidget::Open()
{
	m_formName = s_formName;
	CTaskWidget::Open();
}

// 005e0ff0: o `CShopTask::OnPacket` do 645 (corpo da frente C em shoptask.cpp, com os números do 983); os avisos vão ao ator
// principal da tarefa corrente; o que o 645 repassava ao `OnPacketCommon` aqui devolve 0
int CShopWidget::OnPacket(WReceivedPacket& packet)
{
	packet.Reset();
	g_iRecvPacketType = packet.Decode2();
	bool handled = true;
	switch (g_iRecvPacketType)
	{
	case 0x6a:
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 2, 0, 0, 0, 0, 0)
			<< MsgObject(NULL, 0xe2 /*M983(0xc0)*/, 1, 0, 0, 0, 0);
		// PKT (frente C, lote 11): sub-codigos na numeracao do 983 (tabela 005e2fb8/005e2f2c): o 0x29 do 645 saiu e os de
		// 0x2b a 0x35 andaram -1; 0x35 e novo
		switch (packet.Decode4())
		{
		case 0x0:
			// PKT (frente C, lote 11): 983 005e1158. Pang em +0x91e, o segundo qword em +0x3870 (era DOC_AT 0x1298 do 645);
			// depois do aviso chama 005fbe40 do objeto de 00e78ca8; o aviso ao mensageiro é o pacote 0x29 (era 0x27) com
			// CSharedDoc+0x10d7c (= m_giftIndex; era DOC_AT 0x4db8) e o ramo sem o conteúdo 0x61 (pacote 0x21) saiu.
			Doc()->m_myInfo.stat.i64Pang = packet.Decode8();
			Doc()->m_bonusCash = packet.Decode8();
			Doc()->RefreshAllCookie();
			SHOPMSG(0x23,
				"\xbc\xb1\xb9\xb0\xc7\xcf\xb1\xe2\xb0\xa1 \xbf\xcf\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9");
			if (RFLobbyDesktop::Instance983())
				RFLobbyDesktop::Instance983()->Refresh983();
			if (IsLocalContent((localContentType_t)0x61 /*983: era 0x60; medido*/))
			{
				if (WNetworkSystem::Instance()->IsConnected(
						WNetworkSystem::NET_MSN))
				{
					C_ASSERT_VC10(offsetof(CSharedDoc, m_giftIndex) == 0x10d7c);
					WSendPacket packet2(0x29);
					packet2.Encode4(MyUID());
					packet2.Encode4(Doc()->m_giftIndex);
					packet2.Send(TO_MSN);
				}
			}
			break;
		case 0x2:
			SHOPMSG(0x23,
				"\xb1\xdd\xbe\xd7\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9");
			break;
		case 0x3:
			SHOPMSG(0x23,
				"\xc0\xdf\xb8\xf8\xb5\xc8 \xbb\xf3\xc7\xb0 \xc4\xda\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9");
			break;
		case 0x1:
			SHOPMSG(0x23,
				"\xbc\xb1\xb9\xb0\xc7\xcf\xb1\xe2\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9");
			break;
		case 0x4:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc0\xcc\xb9\xcc \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9");
			break;
		case 0x28:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc0\xcc\xb9\xcc \xbf\xec\xc6\xed\xc7\xd4\xbf\xa1 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x2a: // PKT: era 0x2b (645); texto do 983 (00d12ac8)
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xc0\xcc\xb9\xcc \xb5\xb9\xc7\xc7\xb4\xcf\xc0\xc7 \xb1\xdd\xb0\xed\xbf\xa1 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x8:
			SHOPMSG(0x23,
				"\xbc\xb1\xb9\xb0\xc7\xcf\xb1\xe2\xb8\xa6 \xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb0\xe8\xc1\xa4\xc0\xd4\xb4\xcf\xb4\xd9.");
			break;
		case 0x7:
			SHOPMSG(0x23,
				"\xc1\xd6\xb9\xce\xb9\xf8\xc8\xa3\xb0\xa1 \xc0\xdf\xb8\xf8 \xc0\xd4\xb7\xc2\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9");
			break;
		case 0xb:
		{
			_SYSTEMTIME time;
			packet.DecodeBuffer(&time, 16);
			Doc()->SetServerTime(time);
			SHOPMSG(0x23,
				"\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.");
			break;
		}
		case 0x9:
		{
			_SYSTEMTIME time;
			packet.DecodeBuffer(&time, 16);
			Doc()->SetServerTime(time);
			SHOPMSG(0x23,
				"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xb1\xe2\xb0\xa3\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.");
			break;
		}
		case 0xc:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xbf\xa1\xb0\xd4 \xc7\xd8\xb4\xe7 \xc4\xb3\xb5\xf0\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9");
			break;
		case 0xe:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xbf\xa1\xb0\xd4 \xc7\xd8\xb4\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9");
			break;
		case 0x10:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xb1\xe2\xb0\xa3\xc1\xa6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9");
			break;
		case 0x11:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xb0\xb9\xbc\xf6\xc1\xa6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9");
			break;
		case 0x12:
			SHOPMSG(0x23,
				"\xbe\xc6\xc0\xcc\xc5\xdb \xba\xb8\xc0\xaf \xc7\xd1\xb5\xb5\xb8\xa6 \xc3\xca\xb0\xfa\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 0x15:
			SHOPMSG(0x23,
				"\xc7\xd1\xb9\xf8\xbf\xa1 \xb3\xca\xb9\xab \xb8\xb9\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb1\xb8\xc0\xd4\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x17:
			SHOPMSG(0x23,
				"\xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc5\xb0\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 0x18:
			SHOPMSG(0x23,
				"\xc4\xed\xc5\xb0\xc1\xa4\xba\xb8\xb8\xa6 \xbe\xf7\xb5\xa5\xc0\xcc\xc6\xae\xc7\xcf\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x1c:
			SHOPMSG(0x23,
				"\xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc5 \xbc\xb1\xb9\xb0\xc0\xbb \xc0\xfa\xc0\xe5\xc7\xcf\xc1\xf6 \xb8\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x1e:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xc7 \xc1\xa4\xba\xb8\xb8\xa6 \xc1\xb6\xc8\xb8\xc7\xcf\xb4\xc2\xb5\xa5 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x1f:
			SHOPMSG(0x23,
				"\xb1\xb8\xc0\xd4\xc7\xcf\xb7\xc1\xb0\xed \xc7\xcf\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x20:
			SHOPMSG(0x23,
				"\xbc\xb1\xc5\xc3\xc7\xcf\xbd\xc5 \xc4\xed\xc6\xf9\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9..");
			break;
		case 0x21:
			SHOPMSG(0x23,
				"\xbb\xf3\xb4\xeb\xb9\xe6\xc0\xc7 \xbc\xb1\xb9\xb0\xc7\xd4\xc0\xcc \xb0\xa1\xb5\xe6 \xc2\xf7 \xb4\xf5 \xc0\xcc\xbb\xf3 \xbc\xb1\xb9\xb0\xc0\xbb \xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x13:
			SHOPMSG(0x23,
				"\xc6\xc7\xb8\xc5\xc7\xcf\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 0x19:
			SHOPMSG(0x23,
				"\xc4\xed\xc5\xb0\xb8\xa6 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xd9\xb4\xc2 \xc1\xa4\xba\xb8\xb8\xa6 \xc3\xa3\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x2d: // PKT: era 0x2e (645)
			SHOPMSG(0x23,
				"\xc3\xa4\xb3\xce\xb8\xb5\xbc\xad\xba\xf1\xbd\xba\xb0\xa1 \xc1\xbe\xb7\xe1\xb5\xc7\xbe\xee \xb4\xf5\xc0\xcc\xbb\xf3 \xb1\xb8\xb8\xc5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9..");
			break;
		case 0x22:
			SHOPMSG(0x23,
				"\xbc\xb1\xb9\xb0\xc0\xbb \xc7\xcf\xb7\xc1\xb4\xc2 \xc0\xaf\xc0\xfa\xb4\xc2 \xc0\xcc \xbc\xb1\xb9\xb0\xc0\xbb \xb9\xde\xc0\xb8\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x2b: // PKT: era 0x2c (645)
			SHOPMSG(0x23,
				"\xbc\xb1\xb9\xb0\xc0\xbb \xb9\xde\xb4\xc2 \xc0\xaf\xc0\xfa\xc0\xc7 \xb7\xb9\xba\xa7\xc0\xcc \xb3\xb7\xbe\xc6 \xbc\xb1\xb9\xb0\xc7\xd2\xbc\xf6\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x2c: // PKT: era 0x2d (645)
			SHOPMSG(0x23,
				"\xbc\xb1\xb9\xb0\xc0\xbb \xb9\xde\xb4\xc2 \xc0\xaf\xc0\xfa\xc0\xc7 \xb7\xb9\xba\xa7\xc0\xcc \xb3\xf4\xbe\xc6 \xbc\xb1\xb9\xb0 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x33: // PKT: era 0x34 (645)
			SHOPMSG(0x23,
				"\xc0\xda\xb1\xe2 \xc0\xda\xbd\xc5\xbf\xa1\xb0\xd4\xb4\xc2 \xbc\xb1\xb9\xb0\xc0\xbb \xba\xb8\xb3\xbe \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x32: // PKT: era 0x33 (645)
			SHOPMSG(0x23,
				"\xbc\xb1\xb9\xb0\xc7\xd2 \xc0\xaf\xc0\xfa\xc0\xc7 \xc1\xa4\xba\xb8\xb8\xa6 \xc3\xa3\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4");
			break;
		case 0x34: // PKT: era 0x35 (645)
			SHOPMSG(0x23,
				"\xc7\xf6\xc0\xe7 \xb8\xf0\xb5\xe7 \xc4\xb3\xbd\xac\xbf\xa1 \xb4\xeb\xc7\xd1 \xc1\xb6\xc8\xb8 \xb9\xd7 \xbb\xe7\xbf\xeb\xc0\xcc \xc1\xa4\xc1\xf6\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x35:
			// PKT (frente C, lote 11): 983 005e1f3b (novo) - um dword repassado na mensagem 0xe9 (so do 983)
		{
			unsigned long value = packet.Decode4();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xe9, value, 0, 0, 0, 0));
			break;
		}
		}
		// PKT: o 983 manda a 0xea (so do 983) depois de qualquer sub-codigo, inclusive os sem caso (005e1f8b)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xea, 0, 0, 0, 0, 0));
		break;

	case 0x68:
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 2, 0, 0, 0, 0, 0)
			<< MsgObject(NULL, 0xe2 /*M983(0xc0)*/, 1, 0, 0, 0, 0);
		// PKT (frente C, lote 11): sub-codigos na numeracao do 983 (tabela 005e3050/005e2ff0): o 0x29 do 645 saiu e os de
		// 0x2b a 0x35 andaram -1; 0x35 e novo
		switch (packet.Decode4())
		{
		case 0x2e: // PKT: era 0x2f (645)
		{
			unsigned long tid = packet.Decode4();
			unsigned long guid = packet.Decode4();
			unsigned short qty = (unsigned short)packet.Decode4();
			unsigned long bonus = packet.Decode4();
			std::list<sItemInfo>::iterator it;
			for (it = DOC_ITEMS.begin(); it != DOC_ITEMS.end(); ++it)
			{
				if ((*it).guid == guid && (*it).tid == tid)
				{
					(*it).Common[0] = (short)qty;
					break;
				}
			}
			if (it == DOC_ITEMS.end())
			{
				sItemInfo item;
				memset(&item, 0, sizeof(item));
				item.tid = tid;
				item.guid = guid;
				item.IsValid = 1;
				item.Common[0] = (short)qty;
				DOC_ITEMS.push_back(item);
			}
			AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x2bc /*M983(0x241)*/, (int)K2L_Compatibility(
				MakeStr(
					"\xc1\xa4\xbb\xf3\xc0\xfb\xc0\xb8\xb7\xce \xb1\xb8\xb8\xc5\xb0\xa1 \xbf\xcf\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.\n\xba\xb8\xb3\xca\xbd\xba %s %d\xb0\xb3\xb8\xa6 \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.",
					Doc()->m_itemManager.GetItemName(tid), bonus)), 0, 0, 0, 0);
			break;
		}
		case 0x0:
			Doc()->m_myInfo.stat.i64Pang = packet.Decode8();
			// PKT: era DOC_AT(__int64, 0x1298) (645); o 983 grava em CSharedDoc+0x3870
			C_ASSERT_VC10(offsetof(CSharedDoc, m_bonusCash) == 0x3870);
			Doc()->m_bonusCash = packet.Decode8();
			Doc()->RefreshAllCookie();
			SHOPMSG(0x23,
				"\xc1\xa4\xbb\xf3\xc0\xfb\xc0\xb8\xb7\xce \xb1\xb8\xb8\xc5\xb0\xa1 \xbf\xcf\xb7\xe1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.");
			// 983 (005e2280): no lugar do aviso 0x6a ao CTaskMain da CShopTask, o estado do lobby e o byte +0x106fc
			if (RFLobbyDesktop::Instance983())
				RFLobbyDesktop::Instance983()->Refresh983();
			Doc()->m_bShopBuyDone983 = true;
			break;
		case 0x2:
			SHOPMSG(0x23,
				"\xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc6\xce\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9");
			break;
		case 0x3:
			SHOPMSG(0x23,
				"\xc0\xdf\xb8\xf8\xb5\xc8 \xbb\xf3\xc7\xb0 \xc4\xda\xb5\xe5\xc0\xd4\xb4\xcf\xb4\xd9");
			break;
		case 0x1:
			SHOPMSG(0x23,
				"\xb1\xb8\xb8\xc5\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9");
			break;
		case 0x4:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			SHOPMSG(0x23,
				"\xc0\xcc\xb9\xcc \xb1\xb8\xb8\xc5\xc7\xd1 \xb9\xb0\xb0\xc7\xc0\xd4\xb4\xcf\xb4\xd9.");
			break;
		case 0x28:
			SHOPMSG(0x23,
				"\xc0\xcc\xb9\xcc \xbf\xec\xc6\xed\xc7\xd4\xbf\xa1 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x2a: // PKT: era 0x2b (645)
			SHOPMSG(0x23,
				"\xc0\xcc\xb9\xcc \xb5\xb9\xc7\xc7\xb4\xcf\xc0\xc7 \xba\xf1\xb9\xd0\xb1\xdd\xb0\xed\xbf\xa1 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0xb:
		{
			_SYSTEMTIME time;
			packet.DecodeBuffer(&time, 16);
			Doc()->SetServerTime(time);
			SHOPMSG(0x23,
				"\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.");
			break;
		}
		case 0x9:
		{
			_SYSTEMTIME time;
			packet.DecodeBuffer(&time, 16);
			Doc()->SetServerTime(time);
			SHOPMSG(0x23,
				"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xb1\xe2\xb0\xa3\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.");
			break;
		}
		case 0x10:
			SHOPMSG(0x23,
				"\xb1\xe2\xb0\xa3\xc1\xa6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9");
			break;
		case 0x11:
			SHOPMSG(0x23,
				"\xb0\xb9\xbc\xf6\xc1\xa6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9");
			break;
		case 0x15:
			SHOPMSG(0x23,
				"\xc7\xd1\xb9\xf8\xbf\xa1 \xb3\xca\xb9\xab \xb8\xb9\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb1\xb8\xc0\xd4\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x17:
			SHOPMSG(0x23,
				"\xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc5\xb0\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 0x18:
			SHOPMSG(0x23,
				"\xc4\xed\xc5\xb0\xb8\xa6 \xbe\xf7\xb5\xa5\xc0\xcc\xc6\xae\xc7\xcf\xb4\xc2\xb5\xa5 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x12:
			SHOPMSG(0x23,
				"\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb4\xf5 \xc0\xcc\xbb\xf3 \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x23:
			SHOPMSG(0x23,
				"\xbb\xf3\xc1\xa1\xc0\xcc \xc8\xa5\xc0\xe2\xc7\xcf\xbf\xa9 \xc1\xf6\xb1\xdd\xc0\xba \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3 \xc8\xc4 \xc0\xcc\xbf\xeb\xc7\xd8\xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.");
			break;
		case 0x13:
			SHOPMSG(0x23,
				"\xc6\xc7\xb8\xc5\xc7\xcf\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xc6\xb4\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 0x2d: // PKT: era 0x2e (645)
			SHOPMSG(0x23,
				"\xc3\xa4\xb3\xce\xb8\xb5\xbc\xad\xba\xf1\xbd\xba\xb0\xa1 \xc1\xbe\xb7\xe1\xb5\xc7\xbe\xee \xb4\xf5\xc0\xcc\xbb\xf3 \xb1\xb8\xb8\xc5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x22:
			SHOPMSG(0x23,
				"\xc0\xcc \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb4\xf5 \xc0\xcc\xbb\xf3 \xba\xb8\xc0\xaf\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x2b: // PKT: era 0x2c (645)
		case 0x2c: // PKT: era 0x2d (645)
			SHOPMSG(0x23,
				"\xc7\xf6\xc0\xe7 \xb7\xb9\xba\xa7\xb7\xce\xb4\xc2 \xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9");
			break;
		case 0x34: // PKT: era 0x35 (645)
			SHOPMSG(0x23,
				"\xc7\xf6\xc0\xe7 \xb8\xf0\xb5\xe7 \xc4\xb3\xbd\xac\xbf\xa1 \xb4\xeb\xc7\xd1 \xc1\xb6\xc8\xb8 \xb9\xd7 \xbb\xe7\xbf\xeb\xc0\xcc \xc1\xa4\xc1\xf6\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 0x35:
			// PKT (frente C, lote 11): 983 005e1f3b (novo; mesmo bloco do caso 0x6a) - dword repassado na mensagem 0xe9 (so do 983)
		{
			unsigned long value = packet.Decode4();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xe9, value, 0, 0, 0, 0));
			break;
		}
		}
		// PKT: como no 0x6a, o 983 manda a 0xea (so do 983) depois de qualquer sub-codigo (005e1f8b)
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xea, 0, 0, 0, 0, 0));
		break;

	case 0xb0:
		break;

	case 0xb6:
	{
		unsigned long value = packet.Decode4();
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0xe3 /*M983(0xc1)*/, value, 0, 0, 0, 0));
		break;
	}

	case 0x194:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 2, 0, 0, 0, 0, 0));
		switch (packet.Decode4())
		{
		case 0:
		{
			unsigned long key = packet.Decode4();
			unsigned long value = packet.Decode4();
			sCaddieInfo caddie;
			packet.DecodeBuffer(&caddie, 25);
			std::map<unsigned int, sCaddieInfo>::iterator it =
				Doc()->m_caddieMap.find(key);
			if (it != Doc()->m_caddieMap.end())
				Doc()->m_caddieMap.erase(it);
			Doc()->AddCaddie(&caddie);
			// PKT: era DOC_AT(unsigned long, 0x628) (645); o 983 grava em CSharedDoc+0xa26 (005e2bfc)
			C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidCaddie) == 0xa26);
			Doc()->m_myInfo.userEquip.guidCaddie = value;
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2cb /*M983(0x24f)*/,
				(int)K2L_Compatibility("\xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xb8\xa6 \xbc\xba\xb0\xf8\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				caddie.tid, 0, 0, 0));
			break;
		}
		case 1:
			SHOPMSG(0x23,
				"\xc7\xd8\xb4\xe7 \xc4\xb3\xb5\xf0\xb8\xa6 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 2:
			SHOPMSG(0x23,
				"\xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xb8\xa6 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 3:
			SHOPMSG(0x23,
				"\xbe\xcb\xbc\xf6\xbe\xf8\xb4\xc2 \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		}
	}

	default:
		handled = false;
		break;

	case 0x12d:
		if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		{
			int result = packet.Decode4();
			if (result == 1)
			{
				int count = packet.Decode2();
				for (; count > 0; count--)
				{
					sFurniture_List item;
					memset(&item, 0, 27);
					packet.DecodeBuffer(&item, 27);
					std::list<sFurniture_List>& furn =
						Doc()->m_furnitureList; // PKT: era DOC_AT 0x1230 (645); 983 +0x37d4 (005e2e17)
					std::list<sFurniture_List>::iterator it;
					for (it = furn.begin(); it != furn.end(); ++it)
					{
						if ((*it).id == item.id)
							break;
					}
					if (it == furn.end())
						Doc()->AddFurniture(item);
				}
			}
			else
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x2a6 /*M983(0x228)*/, 0x12d, result, 0, 0, 0));
			}
		}
		break;

	case 0x107:
		if (!packet.Decode1())
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27c /*M983(0x1ff)*/, 0, 0, 0, 0, 0));
		break;
	}

	return handled;
}
