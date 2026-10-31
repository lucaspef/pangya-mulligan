#pragma once

#include "frform.h"

class FrViewer;

class NtDetailDlg : public FrForm
{
public:
	DECLARE_OBJECT(NtDetailDlg)

	NtDetailDlg();
	virtual ~NtDetailDlg();

protected:
	void OnCloseBtnInit(int param);
	void OnCloseBtnUp();
	void OnCaptionInit(int param);
	void OnViewInit(int param);

	FrViewer* m_pView;

	DECLARE_FRESH_MSGMAP()
};
