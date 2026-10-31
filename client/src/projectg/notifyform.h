#pragma once

#include "frform.h"

class FrNotifyForm : public FrForm
{
public:
	DECLARE_OBJECT(FrNotifyForm)

	FrNotifyForm();
	virtual ~FrNotifyForm();

protected:
	virtual void OnFreshOkay();
	virtual void OnFreshCancel();
	virtual void OnFreshYes();
	virtual void OnFreshNo();

	DECLARE_FRESH_MSGMAP()
};
