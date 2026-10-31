#pragma once
#include "controller.h"
#include "impactzoneobject.h"

// JP 983: unidade 00580330–005819b0, nove controladores da zona de impacto do PangYa (todos 0x10 bytes, sem campo próprio).
// On05 = recalcular na vez; On07 = recalcular para o jogador do argumento; On06 = alternar guardar/repor a zona (tacada
// especial); On10 = recalcular quando o item do argumento é um dos cinco de IsWindZoneItem983; On13 = recalcular.
// Nomes dos métodos são meus (o 983 só traz o nome das classes).
bool IsWindZoneItem983(unsigned long typeId); // 00580740: (typeId & 0xfffff) é 6, 32, 44, 45 ou 47
int GetWindLevel983(float wind); // 00580ec0: 0 = vento 1 a 4, 1 = 4 a 7, 2 = 7 a 9, 3 = fora

// JP 983: RTTI .?AVCWindImpactZoneElfEarController@@, vtable 00d0ae5c, 0x10 bytes.
class CWindImpactZoneElfEarController : public IController
{
	DECLARE_OBJECT(CWindImpactZoneElfEarController)
public:
	CWindImpactZoneElfEarController() { }
	virtual bool On05();
	virtual bool On06(float arg);
	virtual bool On07(int arg);
	virtual bool On10(int typeId);
	virtual bool On13();
};

// JP 983: RTTI .?AVCWindImpactZoneElfEarImitationController@@, vtable 00d0ae98, 0x10 bytes.
class CWindImpactZoneElfEarImitationController : public IController
{
	DECLARE_OBJECT(CWindImpactZoneElfEarImitationController)
public:
	CWindImpactZoneElfEarImitationController() { }
	virtual bool On05();
	virtual bool On06(float arg);
	virtual bool On07(int arg);
	virtual bool On10(int typeId);
	virtual bool On13();
};

// JP 983: RTTI .?AVCWindImpactZoneHighElfEarController@@, vtable 00d0aed4, 0x10 bytes.
class CWindImpactZoneHighElfEarController : public IController
{
	DECLARE_OBJECT(CWindImpactZoneHighElfEarController)
public:
	CWindImpactZoneHighElfEarController() { }
	virtual bool On05();
	virtual bool On06(float arg);
	virtual bool On07(int arg);
	virtual bool On10(int typeId);
	virtual bool On13();
};

// JP 983: RTTI .?AVCOwnerItemImpactZoneController@@, vtable 00d0af10, 0x10 bytes.
class COwnerItemImpactZoneController : public IController
{
	DECLARE_OBJECT(COwnerItemImpactZoneController)
public:
	COwnerItemImpactZoneController() { }
	virtual bool On05();
	virtual bool On07(int arg);
};

// JP 983: RTTI .?AVCWindImpactZoneAuxPartController@@, vtable 00d0af4c, 0x10 bytes.
class CWindImpactZoneAuxPartController : public IController
{
	DECLARE_OBJECT(CWindImpactZoneAuxPartController)
public:
	CWindImpactZoneAuxPartController() { }
	virtual bool On05();
	virtual bool On07(int arg);
	virtual bool On10(int typeId);
	virtual bool On13();
};

// JP 983: RTTI .?AVCProbability5ImpactZoneController@@, vtable 00d0af88, 0x10 bytes.
class CProbability5ImpactZoneController : public IController
{
	DECLARE_OBJECT(CProbability5ImpactZoneController)
public:
	CProbability5ImpactZoneController() { }
	virtual bool On05();
	virtual bool On07(int arg);
};

// JP 983: RTTI .?AVCWindImpactZoneUndineController@@, vtable 00d0afc4, 0x10 bytes.
class CWindImpactZoneUndineController : public IController
{
	DECLARE_OBJECT(CWindImpactZoneUndineController)
public:
	CWindImpactZoneUndineController() { }
	virtual bool On05();
	virtual bool On06(float arg);
	virtual bool On07(int arg);
	virtual bool On10(int typeId);
	virtual bool On13();
};

// JP 983: RTTI .?AVCWindImpactZoneMascotController@@, vtable 00d0b000, 0x10 bytes.
class CWindImpactZoneMascotController : public IController
{
	DECLARE_OBJECT(CWindImpactZoneMascotController)
public:
	CWindImpactZoneMascotController() { }
	virtual bool On05();
	virtual bool On06(float arg);
	virtual bool On07(int arg);
	virtual bool On10(int typeId);
	virtual bool On13();
};

// JP 983: RTTI .?AVCWeatherImpactZoneController@@, vtable 00d0b03c, 0x10 bytes.
class CWeatherImpactZoneController : public IController
{
	DECLARE_OBJECT(CWeatherImpactZoneController)
public:
	CWeatherImpactZoneController() { }
	virtual bool On05();
	virtual bool On06(float arg);
	virtual bool On07(int arg);
	virtual bool On10(int typeId);
	bool Refresh983(); // 00580e50: mesmo corpo do On13 das outras, mas fora da vtable (o slot 13 é o da base)
};
