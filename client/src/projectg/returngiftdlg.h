#pragma once

#include "frform.h"

class FrReturnGiftDlg : public FrForm
{
	DECLARE_OBJECT(FrReturnGiftDlg)

	FrReturnGiftDlg();

	unsigned long m_giftIdx;
	unsigned long m_typeId;
};
