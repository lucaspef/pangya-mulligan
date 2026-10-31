// JP 983: unidade 004b4f50–004b51e0 (IController). Fonte reconstruída do 983 (classe nova, sem par no 645).
#include "controller.h"

IController::IController()
	: m_pObject(0)
	, m_0c(0)
{
}

IController::~IController()
{
}

bool IController::On02()
{
	m_0c = 0;
	return true;
}

bool IController::On03()
{
	return true;
}

bool IController::On04()
{
	return m_bEnable;
}

bool IController::On05()
{
	return m_bEnable;
}

bool IController::On06(float)
{
	return m_bEnable;
}

bool IController::On07(int)
{
	return m_bEnable;
}

bool IController::On08()
{
	return m_bEnable;
}

bool IController::On09()
{
	return m_bEnable;
}

bool IController::On10(int)
{
	return m_bEnable;
}

bool IController::On11()
{
	return m_bEnable;
}

bool IController::On12()
{
	return m_bEnable;
}

bool IController::On13()
{
	return m_bEnable;
}

IMPLEMENT_OBJECT(IController, IObject)

CControlledObject* IController::GetControlled() const
{
	return m_pObject;
}

void IController::SetEnable(bool bEnable)
{
	m_bEnable = bEnable;
}

bool IController::TestMask(unsigned int kind, unsigned int flags)
{
	unsigned int mask = 0;
	switch (kind)
	{
	case 1:
		mask = 0x1f;
		break;
	case 16:
		mask = 4;
		break;
	case 32:
		mask = 2;
		break;
	case 14:
		if (flags == 0x10)
			mask = 0x10;
		else if (flags == 1)
			mask = 1;
		else if (flags == 8)
			mask = 8;
		break;
	case 2:
		mask = 0x10;
		break;
	case 4:
		mask = 1;
		break;
	case 8:
		mask = 8;
		break;
	}
	return (flags & mask) ? true : false;
}
