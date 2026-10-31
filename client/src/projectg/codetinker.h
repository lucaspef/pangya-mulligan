#pragma once

class CCodeTinker
{
public:
	CCodeTinker();
	virtual ~CCodeTinker();

	const char* GetHoleDropEffectName(unsigned long typeId);
	bool IsNeedOpenMiniButton(unsigned long typeId);
};
