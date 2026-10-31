#pragma once

#include "frform.h"
struct sNoteInfo;
class FrNoteDlg;
class FrListBox;
class FrMailBoxDlg : public FrForm
{
	DECLARE_OBJECT(FrMailBoxDlg)

	FrMailBoxDlg()
		: m_pNoteList(NULL), m_pSelNote(NULL), m_pNoteDlg(NULL)
	{
	}

protected:
	void OnNoteInit(int param);
	void OnNoteOwnerDraw(int param);
	void OnNoteLBtnUp();
	void OnNoteRBtnUp();
	bool OnNoteDlgResult(int result, FrForm* pForm);
	void OnCancelBtnUp();
	bool OnCloseDlgResult(int result, FrForm* pForm);

	FrListBox* m_pNoteList;
	sNoteInfo* m_pSelNote;
	FrNoteDlg* m_pNoteDlg;

	DECLARE_FRESH_MSGMAP()
};
