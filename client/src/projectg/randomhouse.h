#pragma once

#include "singleton.h"

class CRandomHouse : public WSingleton<CRandomHouse>
{
public:
	CRandomHouse();
	virtual ~CRandomHouse();

	void SetRandomSeed(unsigned long seed);
	unsigned long GetRandom();

private:
	unsigned long* m_mt;
	int m_mti;
	unsigned long m_seed;
};
