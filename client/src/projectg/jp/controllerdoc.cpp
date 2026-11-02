// JP 983: unidade 004b51e0–004bb130 (_gateway::_controller::CControllerDoc). Fonte reconstruída do 983 (classe nova, sem par no
// 645). Sessão B13: construtor, destrutor, virtuais, registro e consultas; sessão B14: criação dos objetos por jogador (004b6a00,
// 004b7e90, 004b9e90, 004ba060). O resto da faixa são instâncias da STL (map, vector, function, lambdas).
#include "controllerdoc.h"
#include "controller.h"
#include "lobbymain.h"
#include "seteffectobject.h"
#include "shareddoc.h"
#include <algorithm>

using namespace _gateway;
using namespace _gateway::_controller;

const char* g_szControllerHandler = "CControllerHandler"; // 00e053a8 (ponteiro lido em Initialize)

static int CheckTopPage() // 004b5250
{
	return g_pFresh->IsCurrentLayout("TOPPAGE") ? 0 : -1;
}

bool CGatewayControlDocJp::IsUsable983()
{
	bool bResult = true;
	if (m_04 == 1)
	{
		if (CheckTopPage() == -1)
			bResult = false;
	}
	return bResult;
}

CControlledObject* CControllerDoc::CreateObject(eSpecialAbility ability)
{
	return CControlledObject::Create(ability);
}

CControllerDoc::ObjectMap& CControllerDoc::GetObjects()
{
	return m_objects;
}

CControllerDoc::ObjectMap& CControllerDoc::GetSubObjects()
{
	return m_subObjects;
}

void CControllerDoc::AddItem(ObjectList& list, int player, unsigned long typeId, unsigned int flags)
{
	sControlledSource* pSource = ((CItemManagerJp*)&Doc()->m_itemManager)->FindAbility983(typeId);
	if (pSource == NULL)
		return;
	if (!IController::TestMask(pSource->m_mask, flags))
		return;
	if (pSource->m_bPeriod == 1)
	{
		CItemManager& itemManager = Doc()->m_itemManager;
		if (!itemManager.IsBetween(&pSource->m_from, &pSource->m_to, Doc()->GetServerTime()))
			return;
	}
	for (unsigned int i = 0; i < 3; ++i)
	{
		unsigned long ability = pSource->m_abilities[i];
		if (ability == 0 || ability == eSpecialAbility_Max)
			continue;
		CControlledObject* pObject = list.at(ability);
		if (pObject == NULL)
		{
			CControlledObject* pNew = CreateObject((eSpecialAbility)ability);
			list[ability] = pNew;
			pObject = list[ability];
		}
		sControlledParam param;
		param.m_00 = player;
		param.m_pSource = pSource;
		param.m_index = i;
		pObject->On01(&param);
		pObject->On04(flags);
	}
}

int CControllerDoc::CountKept(ObjectList* pList)
{
	int count = 0;
	for (ObjectList::iterator it = pList->begin(); it != pList->end(); ++it)
	{
		if (*it && (*it)->On11())
			count++;
	}
	return count;
}

void CControllerDoc::DeleteObjects(ObjectList& list)
{
	for (ObjectList::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (*it)
		{
			delete *it;
			*it = NULL;
		}
	}
}

void CControllerDoc::DeleteControllers()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [](ControllerMap::value_type& v)
	{
		delete v.second;
		v.second = NULL;
	});
}

void CControllerDoc::RefreshEnable()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [](ControllerMap::value_type& v)
	{
		IController* p = v.second;
		p->SetEnableInline(p->On02() ? true : false);
	});
}

void CControllerDoc::FireOn03()
{
	std::for_each(m_controllers.begin(), m_controllers.end(), [](ControllerMap::value_type& v)
	{
		v.second->On03();
	});
}

CControlledObject* CControllerDoc::GetObjectConst(int player, unsigned long index) const
{
	ObjectMap::const_iterator it = m_objects.find((unsigned char)player);
	if (it == m_objects.end())
		return NULL;
	return it->second.at(index);
}

void CControllerDoc::AddAbility(int player, unsigned long ability, int arg3, unsigned long typeId, float value)
{
	sControlledSource* pFound = ((CItemManagerJp*)&Doc()->m_itemManager)->FindAbility983(typeId);
	if (pFound)
	{
		for (unsigned int i = 0; i < 3; ++i)
		{
			if (pFound->m_abilities[i] == ability)
				return;
		}
	}
	ObjectMap::iterator it = m_objects.find((unsigned char)player);
	if (it == m_objects.end())
		return;
	CControlledObject* pObject = it->second.at(ability);
	if (pObject == NULL)
	{
		CControlledObject* pNew = CreateObject((eSpecialAbility)ability);
		it->second[ability] = pNew;
		pObject = it->second[ability];
	}
	sControlledSource source;
	sControlledParam param;
	source.m_values[1] = 0.0f;
	source.m_values[2] = 0.0f;
	param.m_00 = player;
	source.m_values[0] = value;
	param.m_pSource = &source;
	source.m_04 = arg3;
	source.m_typeId = typeId;
	source.m_08 = 0;
	source.m_abilities[1] = 0;
	source.m_0c = 0;
	source.m_abilities[2] = 0;
	source.m_4c = 1;
	source.m_bPeriod = 0;
	source.m_abilities[0] = ability;
	source.m_mask = 2;
	param.m_index = 0;
	pObject->On01(&param);
	pObject->On04(0x10);
}

void CControllerDoc::FireOn09(int player)
{
	ObjectMap::iterator it = m_objects.find((unsigned char)player);
	if (it != m_objects.end())
	{
		CControlledObject* p = it->second.at(13);
		if (p)
			p->On09(0);
	}
}

void CControllerDoc::Purge(int player)
{
	ObjectMap::iterator it = m_objects.find((unsigned char)player);
	if (it != m_objects.end())
	{
		for (ObjectList::iterator i = it->second.begin(); i != it->second.end(); ++i)
		{
			if (*i && !(*i)->On11())
			{
				if (*i)
				{
					delete *i;
					*i = NULL;
				}
			}
		}
	}
}

CControlledObject* CControllerDoc::GetObjectAt(int player, unsigned long index)
{
	ObjectMap::iterator it = m_objects.find((unsigned char)player);
	if (it == m_objects.end())
		return NULL;
	return it->second.at(index);
}

CControllerDoc::ObjectList* CControllerDoc::GetObjectList(unsigned char player)
{
	ObjectMap::iterator it = m_objects.find(player);
	if (it == m_objects.end())
		return NULL;
	return &it->second;
}

IController* CControllerDoc::Find(const std::string& name)
{
	ControllerMap::iterator it = m_controllers.find(name);
	if (it == m_controllers.end())
		return NULL;
	return it->second;
}

void CControllerDoc::RemoveSubObjects(int player)
{
	ObjectMap::iterator it = m_subObjects.find((unsigned char)player);
	if (it != m_subObjects.end())
	{
		DeleteObjects(it->second);
		m_subObjects.erase(it);
	}
}

void CControllerDoc::Clear()
{
	if (m_04 == 0x10)
	{
		for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
			DeleteObjects(it->second);
		m_objects.clear();
	}
	for (ObjectMap::iterator it = m_subObjects.begin(); it != m_subObjects.end(); ++it)
		DeleteObjects(it->second);
	m_subObjects.clear();
}

void CControllerDoc::SetSubObjects(int player, const sCharacterInfo* pInfo)
{
	ObjectList list;
	list.resize(eSpecialAbility_Max, NULL);
	for (int i = 0; i < 24; ++i)
	{
		if (pInfo->tidParts[i] && pInfo->ItemIdList[i])
			AddItem(list, player, pInfo->tidParts[i], m_04);
	}
	for (int i = 0; i < 5; ++i)
	{
		if (pInfo->tidAuxParts[i])
			AddItem(list, player, pInfo->tidAuxParts[i], m_04);
	}
	if (std::find_if(list.begin(), list.end(), [](CControlledObject* p) { return p != NULL; }) == list.end())
		return;
	ObjectMap::iterator it = m_subObjects.find(*(unsigned char*)&player);
	if (it == m_subObjects.end())
		m_subObjects.insert(std::make_pair(*(unsigned char*)&player, list));
}

void CControllerDoc::SetObjects(int player)
{
	CControlledObject::SetActive983(false);
	sUserInfo* pUser = &Doc()->m_userInfo[player];
	sCharacterInfo* pInfo = &pUser->charInfo;
	ObjectList list;
	list.resize(eSpecialAbility_Max, NULL);
	for (int i = 0; i < 24; ++i)
	{
		if (pInfo->tidParts[i] && pInfo->ItemIdList[i])
			AddItem(list, player, pInfo->tidParts[i], 0x10);
	}
	for (int i = 0; i < 5; ++i)
	{
		if (pInfo->tidAuxParts[i])
			AddItem(list, player, pInfo->tidAuxParts[i], 0x10);
	}
	unsigned long tidMascot = Doc()->m_userInfo[player].mascotInfo.tid;
	unsigned long guidMascot = Doc()->m_userInfo[player].mascotInfo.guid;
	if (tidMascot && guidMascot)
		AddItem(list, player, tidMascot, 0x10);
	if (Doc()->m_userInfo[player].info.dwUID == Doc()->m_myInfo.info.dwUID)
	{
		// o 983 testa `(tid | 0x1be00000) == 0`, que nunca vale: todo item da lista entra
		std::list<sItemInfo>& items = Doc()->m_myItemList;
		for (std::list<sItemInfo>::iterator it = items.begin(), end = items.end(); it != end; ++it)
		{
			if ((it->tid & 0x2000000) || (it->tid | 0x1be00000))
				AddItem(list, player, it->tid, 0x10);
		}
	}
	if (std::find_if(list.begin(), list.end(), [](CControlledObject* p) { return p != NULL; }) == list.end())
		return;
	ObjectMap::iterator it = m_objects.find(*(unsigned char*)&player);
	if (it == m_objects.end())
		m_objects.insert(std::make_pair(*(unsigned char*)&player, list));
}

bool CControllerDoc::Release()
{
	DeleteControllers();
	Clear();
	return true;
}

bool CControllerDoc::RegisterControllers()
{
	ControllerMap& controllers = m_controllers;
	auto add = [&controllers](const char* name)
	{
		IObject* p = ObjectFactory().CreateObject(name);
		if (p)
			controllers.insert(std::make_pair(std::string(name), (IController*)p));
	};
	add("CWindImpactZoneElfEarController");
	add("CWindImpactZoneElfEarImitationController");
	add("CWindImpactZoneHighElfEarController");
	add("COwnerItemImpactZoneController");
	add("CWindImpactZoneAuxPartController");
	add("CComboGaugeHoleController");
	add("CComboGaugeGameController");
	add("CPangyaCorrectionController");
	add("CPangyaAddCorrectionCtrl");
	add("CConditionalAuxpartController");
	add("CPowerShotItemController");
	add("CSafetyItemController");
	add("CPlusYardItemController");
	add("CComboGaugeEveryshotController");
	add("CWindImpactZoneMascotController");
	add("CWeatherImpactZoneController");
	add("CWindDirectionController");
	add("CGroundInclinationController");
	add("CSetEffectController");
	add("CProbability5ImpactZoneController");
	add("CWindVolumeController");
	add("CWindImpactZoneUndineController");
	add("CLosslessHitController");
	add("CAutoMiracleSignController");
	add("CBallDirNotifyController");
	add("CTrajectoryPreviewController");
	add("CLosslessHitWeatherController");
	add("CClubsetExpMultiplyController");
	add("CPutter10YardController");
	add("CMiracleSignSkillController");
	add("CWindMinusController");
	return true;
}

CControllerDoc::CControllerDoc(int type)
	: CGatewayControlDocJp(type)
{
}

CControllerDoc::~CControllerDoc()
{
}

CGatewayControlDocJp* __fastcall CControllerDoc::Create()
{
	return new CControllerDoc(0xad);
}

bool CControllerDoc::Initialize()
{
	const char* name = g_szControllerHandler;
	m_pHandler = NULL;
	m_pHandler = (CGatewayHandler*)ActorRegistry983().Get(name);
	RegisterControllers();
	CControlledObject::SetDoc(this);
	return true;
}

static bool RegisterControllerDoc() // 004bab00 (chamado pelo inicializador estático 00cc9070)
{
	CContentsDoc::CreateCreatorList983();
	CContentsDoc::RegisterCreator983(&CControllerDoc::Create);
	return true;
}

static bool s_bRegistered = RegisterControllerDoc();
