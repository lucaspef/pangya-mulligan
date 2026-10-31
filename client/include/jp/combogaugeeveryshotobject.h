#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCComboGaugeEveryshotObject@@, 0x60 bytes (operator new em 004a5aa0), habilidade 0x1a.
class CComboGaugeEveryshotObject : public CControlledObject
{
public:
	CComboGaugeEveryshotObject(eSpecialAbility ability); // 004a5990
	virtual ~CComboGaugeEveryshotObject(); // 004a5a00
	virtual void On01(const sControlledParam* pParam); // 004a5a10

	bool Roll(); // 004a5a60
	bool IsReady() const; // 004a5a70
	int GetPercent() const; // 004a5a90
	bool IsHit() const; // 004a5b40 (falso se o jogador da vez não existe)

private:
	sControlledSource* m_pSource; // +0x50
	int m_54; // +0x54
	int m_percent; // +0x58
	bool m_bHit; // +0x5c
	unsigned char m_bReady; // +0x5d (não é bool: IsReady compara com 0 e usa setne)
};
