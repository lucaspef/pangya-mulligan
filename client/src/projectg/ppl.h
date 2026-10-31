#pragma once

class CPPL : public WSingleton<CPPL>
{
public:
	CPPL();
	virtual ~CPPL();

	bool IsPPLEnable();
};
