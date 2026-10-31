// JP 983: unidade 0064f1a0–0064f900 (CTaskWidget). Classe nova do 983, sem par no 645; ver include/jp/taskwidget.h.
// Sessão B53. Cópias fora de linha que caíram na faixa e não são desta classe: 0064f200 (leitor de +0x1a0 com um argumento),
// 0064f210 / 0064f220 (teste e construtor de um campo de bits de 4 bytes), 0064f2c0 (método da `CAnotherHand` com `this` em
// ecx: trava +0x2c, lê a fila de +0x24 / +0x28 / +0x60).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "actor.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "fresh.h"
#include "frform.h"
#include "gamestringmanager.h"
#include "taskform.h"
#include "taskwidget.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(CTaskWidget, IObject)

// 00c0efc0: singleton local (guarda 00f1acd4)
FrFormFactory983& FrFormFactory983::Instance983()
{
	static FrFormFactory983 s_instance;
	return s_instance;
}

// 00c0ecb0
void FrFormFactory983::Register983(const char* name, Creator creator)
{
	m_creators[name] = creator;
}

// 00c0dff0: nome ausente = NULL
FrForm* FrFormFactory983::Create983(const char* name, FrWndManager* pManager)
{
	std::map<std::string, Creator>::const_iterator it = m_creators.find(name);
	return it == m_creators.end() ? NULL : it->second(pManager, name);
}

// 0064f1a0
void CTaskWidget::Close()
{
	if (m_pForm)
		m_pForm->OnCloseWidget983();
}

// 0064f1c0
void CTaskWidget::OnProcessWhileLoading(float delta)
{
}

// 0064f1d0
void CTaskWidget::OnDisplayWhileLoading()
{
}

// 0064f1e0
int CTaskWidget::OnPacket(WReceivedPacket& packet)
{
	return 0;
}

// 0064f230
void CTaskWidget::Activate()
{
	if (m_pForm)
	{
		m_pForm->Enable(true);
		m_pForm->SetVisible(true);
	}
	m_flag |= 2;
	m_bLoading = false;
	g_input->FreeKey("ESCAPE");
	g_input->FreeMouse();
	((CMouseCursorJp983*)CMouseCursor::Instance())->SetCursor983(1, 0);
}

// 0064f290
void CTaskWidget::Process(float delta)
{
	if (m_bLoading && !(m_flag & 3))
	{
		OnProcessWhileLoading(delta);
		Sleep(30);
	}
}

// 0064f310
void CTaskWidget::Begin983()
{
	g_input->BlockMouse();
	g_input->BlockKey("ESCAPE");
	m_19c = 0;
	((CMouseCursorJp983*)CMouseCursor::Instance())->SetCursor983(7, 0);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0, (int)m_loadingMsg.c_str(), 0, 0, 0, 0));
}

// 0064f3b0
void CTaskWidget::Open()
{
	const char* name = m_formName.c_str();
	FrForm* pForm;
	{
		_client::_private::CLock<_client::CCriticalSection> lock(m_cs);
		pForm = FrFormFactory983::Instance983().Create983(name, g_pFresh->GetManager());
		if (!pForm)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)GetGameString(0x1389), 0, 0, 0, 0));
			return;
		}
		pForm->Enable(false);
	}
	FrTaskForm* pTaskForm = DYNAMIC_CAST(FrTaskForm, pForm);
	if (!pTaskForm)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)GetGameString(0x1389), 0, 0, 0, 0));
		return;
	}
	if (!pTaskForm->OnOpenWidget983())
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)GetGameString(0x138a), 0, 0, 0, 0));
		return;
	}
	m_pForm = pTaskForm;
	m_flag |= 1;
}

// 0064f5d0
void CTaskWidget::OnAnotherHand(int key, void* pParam)
{
	if (key == 1)
	{
		Begin983();
		Open();
		Activate();
	}
}

// 0064f600
void CTaskWidget::Start983()
{
	Begin983();
	Open();
	Activate();
}

// 0064f620
CTaskWidget::CTaskWidget() : m_flag(0), m_19c(0), m_pForm(NULL)
{
	// mapa de layout do 983 (docs/LAYOUTS.md, A193): ctor 0064f620, Open/Activate/Begin983 conferem
	C_ASSERT_VC10(sizeof(CTaskWidget) == 0x1f8 && offsetof(CTaskWidget, m_anotherHand) == 0x14 && offsetof(CTaskWidget, m_pForm) == 0x1a0 && offsetof(CTaskWidget, m_formName) == 0x1a4 && offsetof(CTaskWidget, m_cs) == 0x1dc);
	C_ASSERT_VC10(offsetof(CTaskWidget, m_flag) == 0xc && offsetof(CTaskWidget, m_bLoading) == 0x10 && offsetof(CTaskWidget, m_19c) == 0x19c && offsetof(CTaskWidget, m_loadingMsg) == 0x1c0); // A206: os 8 campos
	m_bLoading = false;
}

// 0064f710
CTaskWidget::~CTaskWidget()
{
	CTaskWidget::Close();
}

// 0064f8d0
void CTaskWidget::SetLoadingMsg983(const char* msg)
{
	if (msg)
		m_loadingMsg = msg;
}
