// JP 983: unidade 006b2bf0–006b30e0 (CTutorialWidget). Classe nova do 983; ver include/jp/tutorialwidget.h. Sessão B55.
// Ficam fora: 006b3000 (par de dois valores, `edx` + pilha = LTCG, sem chamador na faixa) e as quatro pequenas de 006b30a0
// (`return this`, `this + 0x1c`, `this + 4`, byte de +0xa4: cópias fora de linha de outras classes, donas não achadas).
#include "minatl.h"
#include "projectg.h"
#include "actor.h"
#include "shareddoc.h"
#include "icontentsdoc.h"
#include "taskmanager.h"
#include "frform.h"
#include "taskform.h"
#include "tutorialwidget.h"

extern int g_iRecvPacketType; // 00f02494: tipo do último pacote lido (C67.1)
// std::string estáticos da unidade (inicializador 00cd4680, atexit 00cfa3e0; pedido A125.1: não é texto vazio)
static const std::string s_className983("CTutorialWidget"); // 00e15eb8
static const std::string s_formName983("tutorialmain"); // 00e15ed4: o form CTutorialMain (FrTaskForm 0x460)

// documento 0xa4 do CContentsDoc (tutorial; classe não levantada): só o byte de +0x14 é gravado aqui
struct CTutorialDoc983 : public IContentsDataContainer
{
	unsigned char m_04[0x10];
	unsigned char m_14; // +0x14
};

// texto do 983 (o do 645, em lobbymain.cpp, diz "\xc1\xd6\xb4\xcf\xbe\xee" onde este diz "\xc7\xc1\xb7\xce")
static const char* const s_pippinMail983 =
	"[\xc7\xc7\xc7\xc9 \xbc\xb1\xb9\xb0 \xb8\xde\xc0\xcf]\n\n\xbe\xc8\xb3\xe7~ \xc0\xfc \xb4\xe7\xbd\xc5\xc0\xbb \xbf\xa9\xb1\xe2 \xb5\xa5\xb8\xae\xb0\xed \xbf\xc2 \xbd\xc3\xb0\xa3\xc0\xc7 \xbf\xe4\xc1\xa4 \xc7\xc7\xc7\xc9\xc0\xcc\xb6\xf3\xb0\xed \xc7\xd5\xb4\xcf\xb4\xd9. \xb9\xe6\xb1\xdd \xbf\xb9\xbb\xdb \xc4\xb3\xb8\xaf\xc5\xcd\xb8\xa6 \xbc\xb1\xc5\xc3\xc7\xdf\xb3\xd7\xbf\xe4. \xb0\xe8\xbc\xd3 \xc0\xdf \xba\xb8\xbb\xec\xc6\xec\xc1\xd6\xbc\xbc\xbf\xe4.\n\n\xbe\xc6~ \xb1\xd7\xb8\xae\xb0\xed \xc0\xcc\xb0\xc5 \xb9\xde\xc0\xb8\xbc\xbc\xbf\xe4. \xb4\xe7\xbd\xc5\xc0\xcc \xc6\xce\xbe\xdf\xbf\xa1 \xba\xb8\xb4\xd9 \xbd\xb1\xb0\xd4 \xc0\xfb\xc0\xc0\xc7\xcf\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad \xb5\xe5\xb8\xae\xb4\xc2 \xc7\xe0\xbf\xee\xc0\xc7 \xb8\xf1\xb0\xc9\xc0\xcc\xc0\xd4\xb4\xcf\xb4\xd9.\n\xc0\xcc \xc7\xe0\xbf\xee\xc0\xc7 \xb8\xf1\xb0\xc9\xc0\xcc\xb4\xc2 \xb4\xe7\xbd\xc5\xc0\xcc \xb8\xb8\xb5\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xc0\xc7 \\c0xffff0000\\c\xc1\xa4\xc8\xae\xb5\xb5\\c0xff000000\\c\xb8\xa6 \xb3\xf4\xc7\xf4\xc1\xd6\xb0\xed \\c0xffff0000\\c\xc6\xce\xbe\xdf\xc0\xd3\xc6\xd1\xc6\xae\xc1\xb8\\c0xff000000\\c\xc0\xbb \xb3\xd0\xc7\xf4\xc1\xd6\xb4\xc2 \xbf\xaa\xc7\xd2\xc0\xbb \xc7\xd1\xb4\xe4\xb4\xcf\xb4\xd9.\n\xc7\xcf\xb3\xaa \xc1\xd6\xc0\xc7\xc7\xd2 \xb0\xcd\xc0\xba\xbf\xe4~ \xb4\xe7\xbd\xc5\xc0\xc7 \xbd\xc7\xb7\xc2\xc0\xcc \xbe\xee\xb4\xc0 \xc1\xa4\xb5\xb5 \xbe\xc8\xc1\xa4\xb5\xc7\xb4\xc2 \xb7\xb9\xba\xa7, \xc1\xef \\c0xffff0000\\c\xc7\xc1\xb7\xce" "E\\c0xff000000\\c\xb0\xa1 \xb5\xc7\xb8\xe9 \xc8\xbf\xb0\xfa\xb0\xa1 \xbe\xf8\xbe\xee\xc1\xf6\xb4\xcf\xb1\xee\xbf\xe4~ \xbe\xcb\xbe\xc6\xb5\xce\xbd\xc3\xb0\xed\xbf\xe4.\n\n\xb1\xd7\xb8\xae\xb0\xed \xb4\xeb\xc0\xfc\xb8\xf0\xb5\xe5\xbf\xa1 \xb0\xa1\xb1\xe2 \xc0\xfc\xbf\xa1 \xc7\xd1\xb9\xf8 \xc8\xa5\xc0\xda \xbf\xac\xbd\xc0\xc7\xcf\xb0\xed \xbd\xcd\xb4\xd9\xb8\xe9 \xc6\xa9\xc5\xe4\xb8\xae\xbe\xf3 \xb8\xf0\xb5\xe5\xbf\xa1 \xb5\xe9\xbe\xee\xb0\xa1\xba\xb8\xbc\xbc\xbf\xe4.\n\xc2\xf7\xb1\xd9\xc2\xf7\xb1\xd9 \xb8\xf0\xb5\xce \xb9\xe8\xbf\xec\xbd\xc3\xb8\xe9 \xbc\xb1\xb9\xb0\xb5\xb5 \xc1\xd8\xb4\xd9\xb0\xed \xc7\xcf\xb3\xd7\xbf\xe4. ^^\n\n\xb1\xd7\xb7\xb3 \xb4\xe3\xbf\xa1 \xb6\xc7 \xba\xc1\xbf\xe4. \xbe\xc8\xb3\xe7";

IMPLEMENT_OBJECT(CTutorialWidget, CTaskWidget)

// 006b2bf0
CTutorialWidget::CTutorialWidget()
{
}

// 006b2c20
void CTutorialWidget::Close()
{
}

// 006b2c30
void CTutorialWidget::Process(float delta)
{
	CTaskWidget::Process(delta);
}

// 006b2cf0
CTutorialWidget::~CTutorialWidget()
{
}

// 006b2d40: o `CLobbyMain::OnToppage_Finish` do 645 (prêmio 5 ou 2 e a carta do Pippin)
void CTutorialWidget::Activate()
{
	if (m_pForm)
	{
		m_pForm->Open(NULL, 3);
		if (Doc()->m_bAutoRefresh)
		{
			if (IsLocalContent(S3_PARAN_CHANNELING))
			{
				Doc()->m_bAutoRefresh = false;
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x43, 5, 0, 0, 0, 0);
				if (Doc()->m_bTutorialRedirect == true)
				{
					AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x3e, 0x1a000006,
						(int)K2L_Compatibility(s_pippinMail983), 0, 0, 0);
					Doc()->m_bTutorialRedirect = false;
				}
			}
			else
			{
				Doc()->m_bAutoRefresh = false;
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x43, 2, 0, 0, 0, 0);
			}
		}
	}
	CTaskWidget::Activate();
}

// 006b2ee0
int CTutorialWidget::OnPacket(WReceivedPacket& packet)
{
	packet.Reset();
	g_iRecvPacketType = packet.Decode2();
	switch (g_iRecvPacketType)
	{
	case 0x11f:
	{
		unsigned char idx = packet.Decode1();
		unsigned char sub = packet.Decode1();
		if (idx < 3)
			Doc()->m_tutorialComplete[idx] = packet.Decode4();
		else
			packet.DecodeBuffer(Doc()->m_tutorialComplete, 0xc);
		CTutorialDoc983* pDoc;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0xa4, pDoc);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1bc, idx, sub, 0, 0, 0));
		return 1;
	}
	case 0x120:
	{
		unsigned char value = packet.Decode1();
		CTutorialDoc983* pDoc = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0xa4, pDoc);
		pDoc->m_14 = value;
		return 1;
	}
	}
	return 0;
}

// 006b3020
void CTutorialWidget::Open()
{
	m_formName = s_formName983;
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0, (int)"TOPPAGE", 0, 0, 0, 0));
	CTaskWidget::Open();
}
