#pragma once
#include <map>
#include <vector>
#include "controlledobject.h"

// JP 983: RTTI .?AVCComboGaugeLimitIncrease@@, vtable 00d020a0, 0x70 bytes (operator new em 004a62d0), habilidade 8.
// Sem controlador: CSharedDoc::GetComboGaugeLimit (0062fd10) lê o valor por 004a5c70.
class CComboGaugeLimitIncrease : public CControlledObject
{
public:
	CComboGaugeLimitIncrease(eSpecialAbility ability); // 004a5bf0
	virtual ~CComboGaugeLimitIncrease(); // 004a5c10
	virtual void On01(const sControlledParam* pParam); // 004a5c20

	float GetValue(unsigned char index); // 004a5c70

private:
	unsigned long m_typeId[4]; // +0x50
	float m_value[4]; // +0x60
};

// JP 983: RTTI .?AVCComboGaugeObject@@, vtable 00d020d4, 0x5c bytes. Junta, por jogador, os bônus de medidor de combo dos
// itens (tipos 2, 16 e 28 do typeid) e soma ao medidor do jogador (ou do time, no modo de jogo 1).
class CComboGaugeObject : public CControlledObject
{
public:
	struct sBonus // 0xc
	{
		float value;
		unsigned long typeId;
		unsigned char player;
	};
	typedef std::map<unsigned char, std::vector<sBonus> > BonusMap;

	CComboGaugeObject(eSpecialAbility ability); // 004a8730
	virtual ~CComboGaugeObject(); // 004a8630
	virtual void On01(const sControlledParam* pParam); // 004a8410

	float Sum(const std::vector<sBonus>& list); // 004a6b80
	void Apply(); // 004a6d90
	float GetBonus(unsigned char player); // 004a7230

protected:
	BonusMap m_bonus; // +0x50
};

// JP 983: RTTI .?AVCComboGaugeGameObject@@, vtable 00d02108, 0x5c bytes: o mesmo objeto, ligado ao CComboGaugeGameController.
class CComboGaugeGameObject : public CComboGaugeObject
{
public:
	CComboGaugeGameObject(eSpecialAbility ability); // 004a8830
	virtual ~CComboGaugeGameObject(); // 004a86b0
};
