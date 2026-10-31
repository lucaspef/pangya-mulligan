#pragma once

#include "frform.h"

class FrButton;
class FrViewer;

class FrEventPrizeDlg : public FrForm
{
	DECLARE_OBJECT(FrEventPrizeDlg)

	FrEventPrizeDlg()
		: m_pCancel(NULL), m_pView(NULL)
	{
	}

	void SetEvent(unsigned char event) { m_event = event; }
	unsigned char GetEvent() const { return m_event; }

protected:
	virtual bool OnInit();

	void OnCancelInit(int param);
	void OnViewInit(int param);
	void OnViewBtnUp();

	FrButton* m_pCancel;
	FrViewer* m_pView;
	unsigned char m_event;

	DECLARE_FRESH_MSGMAP()
};
