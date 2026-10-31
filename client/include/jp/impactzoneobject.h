#pragma once
#include "controlledobject.h"
#include <utility>

// JP 983: unidade 005819b0–005832d0 (CImpactZoneObject + nove derivadas). RTTI .?AVCImpactZoneObject@@, vtable 00d0b0a4,
// 0x74 bytes (operator new em 005828c0), habilidade 1. Soma de quanto os itens do jogador alargam a zona de impacto do PangYa.
// As derivadas só trocam o On01 (qual dos dois "acrescenta item" chamam) e o nome do controlador. Nomes dos métodos são meus.
class CImpactZoneObject : public CControlledObject
{
public:
	typedef std::vector<std::pair<float, unsigned long> > ItemList; // { metade do valor do item, typeid }

	CImpactZoneObject(eSpecialAbility ability); // 00582390
	virtual ~CImpactZoneObject(); // 005823d0
	virtual void On01(const sControlledParam* pParam); // 00581c20
	virtual void On02(int param); // 005829b0: item novo (typeid ainda fora da lista) entra e soma na zona
	virtual void On03(const sControlledParam* pParam); // 00582a20: item entra na lista sem conferir nem somar

	void ResetSaved(); // 005819b0
	void SetSavedFlag(int index, int value); // 005819c0
	bool IsSaved(); // 005819e0
	void Save(); // 00581c50: guarda a zona, zera e marca
	void Restore(); // 00581c70: repõe a zona guardada e desmarca
	unsigned long GetItemTypeId(unsigned int index); // 00582000 (0 fora da lista)
	float GetItemValue(unsigned int index); // 00582030 (0 fora da lista)

	void SetZone(float zone) { m_zone = zone; } // cópia fora de linha em 00580720
	void AddZone(float zone) { m_zone += zone; } // 00580730
	bool HasItems() { return m_items.empty() ? false : true; } // 00580ea0
	unsigned int GetItemCount() { return m_items.size(); } // 00580eb0
	int GetLevel() { return m_level; }
	float GetZone() const { return m_zone; } // lido pelo CPowerGauge (0085c190)

protected:
	int m_level; // +0x50 (só o mascote e o clima gravam: valor do item como inteiro)
	ItemList m_items; // +0x54
	float m_zone; // +0x64
	float m_savedZone; // +0x68
	int m_savedFlag[2]; // +0x6c
};

// JP 983: RTTI .?AVCImpactZoneElfEar@@, vtable 00d0b0d8, 0x74 bytes, habilidade 0x02.
class CImpactZoneElfEar : public CImpactZoneObject
{
public:
	CImpactZoneElfEar(eSpecialAbility ability);
	virtual ~CImpactZoneElfEar();
	virtual void On01(const sControlledParam* pParam);
};

// JP 983: RTTI .?AVCImpactZoneElfEarImitation@@, vtable 00d0b10c, 0x74 bytes, habilidade 0x03.
class CImpactZoneElfEarImitation : public CImpactZoneObject
{
public:
	CImpactZoneElfEarImitation(eSpecialAbility ability);
	virtual ~CImpactZoneElfEarImitation();
	virtual void On01(const sControlledParam* pParam);
};

// JP 983: RTTI .?AVCImpactZoneHighElfEar@@, vtable 00d0b140, 0x74 bytes, habilidade 0x04.
class CImpactZoneHighElfEar : public CImpactZoneObject
{
public:
	CImpactZoneHighElfEar(eSpecialAbility ability);
	virtual ~CImpactZoneHighElfEar();
	virtual void On01(const sControlledParam* pParam);
};

// JP 983: RTTI .?AVCImpactZoneOwnerItem@@, vtable 00d0b174, 0x74 bytes, habilidade 0x05.
class CImpactZoneOwnerItem : public CImpactZoneObject
{
public:
	CImpactZoneOwnerItem(eSpecialAbility ability);
	virtual ~CImpactZoneOwnerItem();
	virtual void On01(const sControlledParam* pParam);
	virtual bool On11(); // 005819f0: falso
};

// JP 983: RTTI .?AVCImpactZoneAuxPartWeakWind@@, vtable 00d0b1a8, 0x74 bytes, habilidade 0x06.
class CImpactZoneAuxPartWeakWind : public CImpactZoneObject
{
public:
	CImpactZoneAuxPartWeakWind(eSpecialAbility ability);
	virtual ~CImpactZoneAuxPartWeakWind();
	virtual void On01(const sControlledParam* pParam);
};

// JP 983: RTTI .?AVCImpactZoneProbability5@@, vtable 00d0b1dc, 0x74 bytes, habilidade 0x0f.
class CImpactZoneProbability5 : public CImpactZoneObject
{
public:
	CImpactZoneProbability5(eSpecialAbility ability);
	virtual ~CImpactZoneProbability5();
	virtual void On01(const sControlledParam* pParam);
};

// JP 983: RTTI .?AVCImpactZoneUndine@@, vtable 00d0b210, 0x74 bytes, habilidade 0x11.
class CImpactZoneUndine : public CImpactZoneObject
{
public:
	CImpactZoneUndine(eSpecialAbility ability);
	virtual ~CImpactZoneUndine();
	virtual void On01(const sControlledParam* pParam);
};

// JP 983: RTTI .?AVCImpactZoneMascot@@, vtable 00d0b244, 0x74 bytes, habilidade 0x1b.
class CImpactZoneMascot : public CImpactZoneObject
{
public:
	CImpactZoneMascot(eSpecialAbility ability);
	virtual ~CImpactZoneMascot();
	virtual void On01(const sControlledParam* pParam);
};

// JP 983: RTTI .?AVCWeatherImpactZone@@, vtable 00d0b278, 0x74 bytes, habilidade 0x1c.
class CWeatherImpactZone : public CImpactZoneObject
{
public:
	CWeatherImpactZone(eSpecialAbility ability);
	virtual ~CWeatherImpactZone();
	virtual void On01(const sControlledParam* pParam);
};
