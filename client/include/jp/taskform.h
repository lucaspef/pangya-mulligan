#pragma once
// JP 983: FrTaskForm (unidade 005235f0–005238f0; vtables 00d0877c / 00d08770 / 00d08768; RTTI 00e74ff8). Form ligado a uma
// `CTaskWidget` (include/jp/taskwidget.h): guarda a tarefa em que nasceu e, ao fechar, solta a widget dela e avisa o ator
// principal (mensagem 0x88, "RFLobbyDesktop"). m_pTask em +0x110 (construtor 00523690); sizeof 0x120 pelo alinhamento 16 do
// FrForm (sessão A124; o V3 mede o mesmo). Três virtuais novos depois dos da FrForm (+0x98, +0x9c, +0xa0); nomes dos métodos e
// dos campos são meus (sessão B53). O +0xa0 recebe um MsgObject (CTutorialMain 007d40a0 lê +4/+8/+0xc): A124.1, sessão A126.

#include "frform.h"

class CTask;
class MsgObject;

class FrTaskForm : public FrForm
{
public:
	static const WRTTI m_RTTI; // 00e74ff8
	virtual const WRTTI* GetRTTI() const; // 00523680

	FrTaskForm(); // 00523690
	virtual ~FrTaskForm(); // 00523640 (com `delete`: 00523700)

	virtual bool Close(bool bFade); // 00523730

protected:
	virtual bool OnInit(); // 00523600: só repassa à FrForm
	virtual void OnOK(); // 005237d0
	virtual void OnCancel(); // 00523860

public:
	virtual bool OnOpenWidget983(); // 005235f0 (+0x98): a CTaskWidget::Open chama depois de criar; falso = erro 0x138a
	virtual bool OnCloseWidget983(); // 00523610 (+0x9c): a widget fechou — esquece a tarefa
	virtual bool OnMsg983(const MsgObject& msg); // 00523620 (+0xa0): falso = não tratou

protected:
	void LeaveWidget983(); // embutida nas três (Close, OnOK, OnCancel)

	CTask* m_pTask; // +0x110: tarefa corrente na hora do construtor
};
