#pragma once

#include "singleton.h"

class CGolfDoc;

class CRival : public WSingleton<CRival>
{
public:
	CRival();
	virtual ~CRival();

	void Shot();
	void SetVariable(CGolfDoc* pDoc);
	void SetResult();

	WVector GetPos() { return m_target; }

private:
	WVector GetGreenSlope();

	WVector m_target;
	WVector m_expected;
	int m_reserved;
	float m_angle;
	float m_power;
	int m_club;
	int m_mode;
	float m_curve;
	float m_spin;
};
