#pragma once

#include "frform.h"

class FrKbdManualDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrKbdManualDlg)

	FrKbdManualDlg() { }

protected:
	void OnCloseBtnUp();

	DECLARE_FRESH_MSGMAP()
};
