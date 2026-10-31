#pragma once
#include "wproc.h"
#ifndef WANGREAL_DEVICE
inline WProc::~WProc()
{
	if (this->include)
	{
		this->include->DelProc(this);
	}
}

#endif
