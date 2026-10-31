// JP 983: unidade 005819b0–005832d0 (CImpactZoneObject + nove derivadas). Classes novas do 983. Entre as funções próprias o 983
// guarda as instâncias de std::vector<std::pair<float, unsigned long>> e a cópia fora de linha de CPowerGauge::Instance (00581a00).
#include "impactzoneobject.h"
#include "powergauge.h"
#include <algorithm>

struct sFindImpactZoneItem
{
	sFindImpactZoneItem(unsigned long typeId) : m_typeId(typeId) { }
	bool operator()(const std::pair<float, unsigned long>& item) const { return item.second == m_typeId; }
	unsigned long m_typeId;
};

void CImpactZoneObject::ResetSaved()
{
	m_savedZone = 0.0f;
	m_savedFlag[0] = 0;
	m_savedFlag[1] = 0;
}

void CImpactZoneObject::SetSavedFlag(int index, int value)
{
	m_savedFlag[index] = value;
}

bool CImpactZoneObject::IsSaved()
{
	if ((m_savedFlag[1] ^ m_savedFlag[0]) == 1)
		return true;
	return false;
}

bool CImpactZoneOwnerItem::On11()
{
	return false;
}

void CImpactZoneObject::On01(const sControlledParam* pParam)
{
	On02((int)pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneObject::Save()
{
	m_savedZone = m_zone;
	m_zone = 0.0f;
	SetSavedFlag(0, 1);
	SetSavedFlag(1, 0);
}

void CImpactZoneObject::Restore()
{
	m_zone = m_savedZone;
	SetSavedFlag(0, 0);
	SetSavedFlag(1, 0);
}

void CImpactZoneElfEar::On01(const sControlledParam* pParam)
{
	On03(pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneElfEarImitation::On01(const sControlledParam* pParam)
{
	On03(pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneHighElfEar::On01(const sControlledParam* pParam)
{
	On03(pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneOwnerItem::On01(const sControlledParam* pParam)
{
	On02((int)pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneAuxPartWeakWind::On01(const sControlledParam* pParam)
{
	On02((int)pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneProbability5::On01(const sControlledParam* pParam)
{
	On02((int)pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneUndine::On01(const sControlledParam* pParam)
{
	On03(pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
}

void CImpactZoneMascot::On01(const sControlledParam* pParam)
{
	On03(pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
	m_level = (int)pParam->m_pSource->m_values[pParam->m_index];
}

void CWeatherImpactZone::On01(const sControlledParam* pParam)
{
	On03(pParam);
	CPowerGauge::Instance()->AddImpactZone983(pParam->m_00, this);
	m_level = (int)pParam->m_pSource->m_values[pParam->m_index];
}

unsigned long CImpactZoneObject::GetItemTypeId(unsigned int index)
{
	if (m_items.size() <= index)
		return 0;
	return m_items.at(index).second;
}

float CImpactZoneObject::GetItemValue(unsigned int index)
{
	if (m_items.size() <= index)
		return 0.0f;
	return m_items.at(index).first;
}

CImpactZoneObject::CImpactZoneObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_zone(0.0f)
	, m_savedZone(0.0f)
{
	m_savedFlag[0] = 0;
	m_savedFlag[1] = 0;
}

CImpactZoneObject::~CImpactZoneObject()
{
}

CImpactZoneElfEar::CImpactZoneElfEar(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CWindImpactZoneElfEarController");
}

CImpactZoneElfEar::~CImpactZoneElfEar()
{
}

CImpactZoneElfEarImitation::CImpactZoneElfEarImitation(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CWindImpactZoneElfEarImitationController");
}

CImpactZoneElfEarImitation::~CImpactZoneElfEarImitation()
{
}

CImpactZoneHighElfEar::CImpactZoneHighElfEar(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CWindImpactZoneHighElfEarController");
}

CImpactZoneHighElfEar::~CImpactZoneHighElfEar()
{
}

CImpactZoneOwnerItem::CImpactZoneOwnerItem(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("COwnerItemImpactZoneController");
}

CImpactZoneOwnerItem::~CImpactZoneOwnerItem()
{
}

CImpactZoneAuxPartWeakWind::CImpactZoneAuxPartWeakWind(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CWindImpactZoneAuxPartController");
}

CImpactZoneAuxPartWeakWind::~CImpactZoneAuxPartWeakWind()
{
}

CImpactZoneProbability5::CImpactZoneProbability5(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CProbability5ImpactZoneController");
}

CImpactZoneProbability5::~CImpactZoneProbability5()
{
}

CImpactZoneUndine::CImpactZoneUndine(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CWindImpactZoneUndineController");
}

CImpactZoneUndine::~CImpactZoneUndine()
{
}

CImpactZoneMascot::CImpactZoneMascot(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CWindImpactZoneMascotController");
}

CImpactZoneMascot::~CImpactZoneMascot()
{
}

CWeatherImpactZone::CWeatherImpactZone(eSpecialAbility ability)
	: CImpactZoneObject(ability)
{
	AddController("CWeatherImpactZoneController");
}

CWeatherImpactZone::~CWeatherImpactZone()
{
}

static CControlledObject* CreateImpactZoneObject(eSpecialAbility ability)
{
	return new CImpactZoneObject(ability);
}

static bool RegisterImpactZoneObject()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZone, CreateImpactZoneObject);
}

static bool s_registeredImpactZoneObject = RegisterImpactZoneObject();

void CImpactZoneObject::On02(int param)
{
	const sControlledParam* pParam = (const sControlledParam*)param;
	sControlledSource* pSource = pParam->m_pSource;
	unsigned long typeId = pSource->m_typeId;
	if (std::find_if(m_items.begin(), m_items.end(), sFindImpactZoneItem(typeId)) != m_items.end())
		return;
	float value = pSource->m_values[pParam->m_index] * 0.5;
	m_items.push_back(std::pair<float, unsigned long>(value, typeId));
	m_zone += value;
}

void CImpactZoneObject::On03(const sControlledParam* pParam)
{
	sControlledSource* pSource = pParam->m_pSource;
	m_items.push_back(std::pair<float, unsigned long>(pSource->m_values[pParam->m_index] * 0.5, pSource->m_typeId));
}

static CControlledObject* CreateImpactZoneElfEar(eSpecialAbility ability)
{
	return new CImpactZoneElfEar(ability);
}

static bool RegisterImpactZoneElfEar()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneElfEar, CreateImpactZoneElfEar);
}

static bool s_registeredImpactZoneElfEar = RegisterImpactZoneElfEar();

static CControlledObject* CreateImpactZoneElfEarImitation(eSpecialAbility ability)
{
	return new CImpactZoneElfEarImitation(ability);
}

static bool RegisterImpactZoneElfEarImitation()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneElfEarImitation, CreateImpactZoneElfEarImitation);
}

static bool s_registeredImpactZoneElfEarImitation = RegisterImpactZoneElfEarImitation();

static CControlledObject* CreateImpactZoneHighElfEar(eSpecialAbility ability)
{
	return new CImpactZoneHighElfEar(ability);
}

static bool RegisterImpactZoneHighElfEar()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneHighElfEar, CreateImpactZoneHighElfEar);
}

static bool s_registeredImpactZoneHighElfEar = RegisterImpactZoneHighElfEar();

static CControlledObject* CreateImpactZoneOwnerItem(eSpecialAbility ability)
{
	return new CImpactZoneOwnerItem(ability);
}

static bool RegisterImpactZoneOwnerItem()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneOwnerItem, CreateImpactZoneOwnerItem);
}

static bool s_registeredImpactZoneOwnerItem = RegisterImpactZoneOwnerItem();

static CControlledObject* CreateImpactZoneAuxPartWeakWind(eSpecialAbility ability)
{
	return new CImpactZoneAuxPartWeakWind(ability);
}

static bool RegisterImpactZoneAuxPartWeakWind()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneAuxPartWeakWind, CreateImpactZoneAuxPartWeakWind);
}

static bool s_registeredImpactZoneAuxPartWeakWind = RegisterImpactZoneAuxPartWeakWind();

static CControlledObject* CreateImpactZoneProbability5(eSpecialAbility ability)
{
	return new CImpactZoneProbability5(ability);
}

static bool RegisterImpactZoneProbability5()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneProbability5, CreateImpactZoneProbability5);
}

static bool s_registeredImpactZoneProbability5 = RegisterImpactZoneProbability5();

static CControlledObject* CreateImpactZoneUndine(eSpecialAbility ability)
{
	return new CImpactZoneUndine(ability);
}

static bool RegisterImpactZoneUndine()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneUndine, CreateImpactZoneUndine);
}

static bool s_registeredImpactZoneUndine = RegisterImpactZoneUndine();

static CControlledObject* CreateImpactZoneMascot(eSpecialAbility ability)
{
	return new CImpactZoneMascot(ability);
}

static bool RegisterImpactZoneMascot()
{
	return RegisterControlledObject(eSpecialAbility_ImpactZoneMascot, CreateImpactZoneMascot);
}

static bool s_registeredImpactZoneMascot = RegisterImpactZoneMascot();

static CControlledObject* CreateWeatherImpactZone(eSpecialAbility ability)
{
	return new CWeatherImpactZone(ability);
}

static bool RegisterWeatherImpactZone()
{
	return RegisterControlledObject(eSpecialAbility_WeatherImpactZone, CreateWeatherImpactZone);
}

static bool s_registeredWeatherImpactZone = RegisterWeatherImpactZone();
