// JP 983: unidade 005235f0–005238f0 (FrTaskForm). Classe nova do 983, sem par no 645; ver include/jp/taskform.h.
// Sessão B53. 00523630 (`mov eax, [ecx + 0x30]`) é a cópia fora de linha de `CTask::GetMainActor`.
#include "minatl.h"
#include "projectg.h"
#include "actor.h"
#include "taskmanager.h"
#include "taskwidget.h"
#include "taskform.h"
#include "jp/gdskinitem.h"
#include "gdskininstance983.h"

const WRTTI FrTaskForm::m_RTTI("FrTaskForm", &FrForm::m_RTTI);

// 00523680
const WRTTI* FrTaskForm::GetRTTI() const
{
	return &m_RTTI;
}

// 005235f0
bool FrTaskForm::OnOpenWidget983()
{
	return true;
}

// 00523600
bool FrTaskForm::OnInit()
{
	return FrForm::OnInit();
}

// 00523610
bool FrTaskForm::OnCloseWidget983()
{
	m_pTask = NULL;
	return true;
}

// 00523620
bool FrTaskForm::OnMsg983(const MsgObject& msg)
{
	return false;
}

// 00523690
FrTaskForm::FrTaskForm()
{
	m_pTask = AfxGetTask();
}

// 00523640
FrTaskForm::~FrTaskForm()
{
	if (CTask* pTask = AfxGetTask())
		pTask->ForgetWidgetForm983(this);
	FrTaskForm::OnCloseWidget983();
}

// embutida em 00523730, 005237d0 e 00523860: solta a widget da tarefa (se ainda for a corrente), avisa o ator principal e
// desliga o byte +0x7a do CGDSkinItem
__forceinline void FrTaskForm::LeaveWidget983()
{
	if (m_pTask && m_pTask == AfxGetTask())
		m_pTask->CloseWidget983(); // A301: a visão CTaskJp983 caía no stub e a widget nunca era apagada
	else if (AfxGetTask()->GetWidgetForm983() == this) // dono NULL (B345): sem isto a tarefa guardava o form liberado (C115.4)
		AfxGetTask()->CloseWidget983();
	AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x88, (int)"RFLobbyDesktop", 0, 0, 0, 0);
	CGDSkinItemPetJp::Instance983()->SetB7a983(false); // 00542f00
}

// 00523730
bool FrTaskForm::Close(bool bFade)
{
	LeaveWidget983();
	return FrForm::Close(bFade);
}

// 005237d0
void FrTaskForm::OnOK()
{
	LeaveWidget983();
	FrForm::OnOK();
}

// 00523860
void FrTaskForm::OnCancel()
{
	LeaveWidget983();
	FrForm::OnCancel();
}
