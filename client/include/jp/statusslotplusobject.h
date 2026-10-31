#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCStatusSlotPlusObject@@, vtable 00d0a950, 0x5c bytes (operator new em 0056b990).
class CStatusSlotPlusObject : public CControlledObject
{
public:
	CStatusSlotPlusObject(eSpecialAbility ability); // 0056b8c0
	virtual ~CStatusSlotPlusObject(); // 0056b930
	virtual void On01(const sControlledParam*) { } // 0056b940
	virtual void On03(const sControlledParam*) { } // 0056b950
	virtual void On07(int) { } // 0056b960
	virtual void On10(int) { } // 0056b970

private:
	char m_50; // +0x50
	int m_54; // +0x54
	bool m_58; // +0x58
	bool m_59; // +0x59
};
