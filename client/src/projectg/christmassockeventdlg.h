#pragma once
#include "frform.h"
class FrViewer;
class FrChristmasSockEventDescDlg : public FrForm
{
	DECLARE_OBJECT(FrChristmasSockEventDescDlg)

	FrChristmasSockEventDescDlg();
	virtual ~FrChristmasSockEventDescDlg();

protected:
	void OnInitDescView(int param);

	FrViewer* m_pViewer;

	DECLARE_FRESH_MSGMAP()
};
