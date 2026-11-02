// JP 983: unidade 006277b0–0062b2e0 (CSetAbility + oito derivadas, o estado de cada conjunto e o CSetEffectObject). Classes novas do
// 983: efeito de conjunto de peças (habilidade 0x0d). Cada peça vestida chega pelo slot 1 com o id do conjunto; quando as cinco
// peças do registro (00a177f0) estão presentes, o efeito do momento pedido pelo CSetEffectController vai para a fila e é aplicado.
// As faixas setability / setabilitywindminus / seteffectobject do mapa da F3.7.1 são esta unidade só.
#include "seteffectobject.h"
#include "shareddocjp.h"
#include "docofs983.h"
#include "motionadvisor.h"
#include "taskmanager.h"
#include "actor.h"
#include "gatewayactor.h"

// 00e14da8: item do corte escondido por parte (índice = CSetEffectState::m_part)
static unsigned long s_hiddenCutIn[] =
{
	0x394001b7, 0x394001b8, 0x394001b9, 0x394001ba, 0x394001bb, 0x394001bc, 0x394001bd, 0x394001be, 0x394001bf, 0x394001c0,
	0x394001c1,
};

// argumento do aviso 0x753b / 9 ao "Gateway" (nomes meus)
struct sSetAbilityNotify
{
	unsigned char m_player; // +0x00
	int m_kind; // +0x04
	int m_value; // +0x08
	int m_bPercent; // +0x0c
	float m_rate; // +0x10
};

CSetAbility::~CSetAbility()
{
}

bool CSetAbility::Cancel()
{
	return true;
}

bool CSetAbility::Apply(CSetEffectState*)
{
	return true;
}

bool CSetAbility::IsDeferred()
{
	return false;
}

CSetEffectState::CSetEffectState()
	: m_pData(NULL)
	, m_part(0xf)
	, m_bComplete(false)
{
	memset(m_have, 0, sizeof(m_have));
	memset(m_count, 0, sizeof(m_count));
}

void CSetEffectState::ResetComplete()
{
	m_bComplete = false;
}

bool CSetEffectState::IsComplete()
{
	return m_bComplete;
}

bool CSetEffectState::Check()
{
	if (memcmp(m_pData->m_typeId, m_have, sizeof(m_have)) != 0)
		return false;
	if (memcmp(m_pData->m_count, m_count, sizeof(m_count)) != 0)
		return false;
	return true;
}

void CSetEffectState::SetPart(int part)
{
	m_part = part;
}

int CSetEffectState::GetPart()
{
	return m_part;
}

void CSetEffectState::Add(unsigned long typeId, unsigned char count)
{
	for (unsigned int i = 0; i < 5; i++)
	{
		if (m_pData->m_typeId[i] == typeId)
		{
			m_have[i] = typeId;
			m_count[i] += count;
			break;
		}
	}
	m_bComplete = Check();
}

int CSetEffectState::ToTrigger(int kind)
{
	switch (kind)
	{
	case 1:
		return 4;
	case 8:
		return 8;
	case 4:
		return 0x10;
	case 0x10:
		return 2;
	case 2:
		return 0x20;
	case 0:
		return 0;
	}
	return 0;
}

int CSetEffectState::GetAbility(int kind)
{
	int trigger = ToTrigger(kind);
	for (unsigned char i = 0; i < 3; i++)
	{
		if (m_pData->m_trigger[i] == trigger)
			return m_pData->m_ability[i];
	}
	return 0;
}

CSetAbility* CSetEffectObject::CreateAbility(int ability)
{
	CSetAbility* pAbility = NULL;
	switch (ability)
	{
	case 1:
		pAbility = new CSetAbilityMotionChange;
		break;
	case 2:
		pAbility = new CSetAbilityAddMotion;
		break;
	case 3:
		pAbility = new CSetAbilityHiddenCutIn;
		break;
	case 4:
		pAbility = new CSetAbilityAuxpartImpactZone;
		break;
	case 5:
		pAbility = new CSetAbilityPlusYard;
		break;
	case 6:
		pAbility = new CSetAbilitySlotPlus;
		break;
	case 7:
		pAbility = new CSetAbilityWindMinus;
		break;
	case 8:
		pAbility = new CSetAbilityAnimalPangya;
		break;
	}
	return pAbility;
}

CSetAbility* CSetEffectObject::GetAbility(int ability)
{
	CSetAbility* pAbility = m_abilities.at(ability);
	if (pAbility == NULL)
	{
		CSetAbility* pNew = CreateAbility(ability);
		if (pNew != NULL)
		{
			m_abilities[ability] = pNew;
			pAbility = pNew;
		}
	}
	return pAbility;
}

bool CSetAbilityHiddenCutIn::Apply(CSetEffectState* pState)
{
	if (DOCJP()->Is63f6c0_983(1, pState->m_player) != true)
		DOCJP()->SetSetEffect983((unsigned char)pState->m_player, s_hiddenCutIn[pState->GetPart()]);
	return true;
}

bool CSetAbilityAuxpartImpactZone::Apply(CSetEffectState* pState)
{
	sSetAbilityNotify notify;
	notify.m_rate = 1.0f;
	notify.m_player = (unsigned char)pState->m_player;
	notify.m_kind = 1;
	notify.m_value = pState->m_have[0];
	notify.m_bPercent = 0;
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 9, (int)&notify, 0, 0, 0);
	return true;
}

bool CSetAbilityPlusYard::Apply(CSetEffectState* pState)
{
	sSetAbilityNotify notify;
	notify.m_rate = 100.0f;
	notify.m_player = (unsigned char)pState->m_player;
	notify.m_kind = 0x19;
	notify.m_value = pState->m_have[0];
	notify.m_bPercent = 1;
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 9, (int)&notify, 0, 0, 0);
	return true;
}

bool CSetAbilitySlotPlus::Apply(CSetEffectState* pState)
{
	sSetAbilityNotify notify;
	notify.m_rate = 1.0f;
	notify.m_player = (unsigned char)pState->m_player;
	notify.m_kind = 0x21;
	notify.m_value = pState->m_have[0];
	notify.m_bPercent = 1;
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 9, (int)&notify, 0, 0, 0);
	return true;
}

bool CSetAbilityWindMinus::Apply(CSetEffectState* pState)
{
	sSetAbilityNotify notify;
	notify.m_rate = 1.0f;
	notify.m_player = (unsigned char)pState->m_player;
	notify.m_kind = 0x22;
	notify.m_value = pState->m_have[0];
	notify.m_bPercent = 1;
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 9, (int)&notify, 0, 0, 0);
	return true;
}

bool CSetAbilityAnimalPangya::Apply(CSetEffectState* pState)
{
	sSetAbilityNotify notify;
	notify.m_rate = 80.0f;
	notify.m_player = (unsigned char)pState->m_player;
	notify.m_kind = 0x23;
	notify.m_value = pState->m_have[0];
	notify.m_bPercent = 1;
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 9, (int)&notify, 0, 0, 0);
	return true;
}

// corpo aqui porque o 983 guarda a cópia fora de linha nesta unidade (embutido do CSharedDoc); o mapa fica em +0x38a0
int CSharedDocJp::FindKeyByPlayer983(unsigned char player)
{
	if (player != 0xff)
	{
		// B1462: era (char*)this + 0x38a0 cru (VC10 m_indexMap); no cl 14.44 o CSharedDoc anda (D876) -> membro
		// (CSharedDocJp sem campo = Doc(); map<int> como o 983: mesmo layout do map<unsigned long>)
		std::map<int, unsigned char>* pMap = (std::map<int, unsigned char>*)&((CSharedDoc*)this)->m_indexMap;
		for (std::map<int, unsigned char>::iterator it = pMap->begin(); it != pMap->end(); ++it)
		{
			if (it->second == player)
				return it->first;
		}
	}
	return -1;
}

bool CSetAbilityMotionChange::Apply(CSetEffectState* pState)
{
	int key = DOCJP()->FindKeyByPlayer983((unsigned char)pState->m_player);
	CMotionAdvisor::Instance()->AddPart(key, pState->GetPart());
	// "기본자세" → "연패기본자세" (CP949)
	CMotionAdvisor::Instance()->SetMotionName(key, "\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc", "\xbf\xad\xc7\xf7\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc");
	return true;
}

void CSetEffectObject::Flush()
{
	while (!m_pending.empty())
	{
		std::pair<unsigned long, int>& entry = m_pending.front();
		CSetAbility* pAbility = GetAbility(entry.second);
		if (pAbility != NULL)
		{
			std::map<unsigned long, CSetEffectState>::iterator it = m_states.find(entry.first);
			if (it != m_states.end())
				pAbility->Apply(&it->second);
		}
		m_pending.pop();
	}
}

void CSetEffectObject::Apply(int kind)
{
	for (std::map<unsigned long, CSetEffectState>::iterator it = m_states.begin(); it != m_states.end(); ++it)
	{
		if (it->second.IsComplete() == true)
		{
			int ability = it->second.GetAbility(kind);
			if (ability != 0)
				m_pending.push(std::pair<unsigned long, int>(it->first, ability));
		}
		else
		{
			CSetAbility* pAbility = GetAbility(it->second.GetAbility(kind));
			if (pAbility != NULL)
				pAbility->Cancel();
		}
	}
	Flush();
}

void CSetEffectObject::ApplyDeferred()
{
	for (std::map<unsigned long, CSetEffectState>::iterator it = m_states.begin(); it != m_states.end(); ++it)
	{
		if (it->second.IsComplete() == true)
		{
			int ability = it->second.GetAbility(0x10);
			if (ability != 0)
			{
				CSetAbility* pAbility = GetAbility(ability);
				if (pAbility != NULL && pAbility->IsDeferred())
					m_pending.push(std::pair<unsigned long, int>(it->first, ability));
			}
		}
		else
		{
			CSetAbility* pAbility = GetAbility(it->second.GetAbility(0x10));
			if (pAbility != NULL)
				pAbility->Cancel();
		}
	}
	Flush();
}

// conjuntos que valem por vários: a peça do grupo 0x10 (typeId >> 26) conta para todos os da lista
struct sSetEffectGroup
{
	int m_id;
	int m_ids[15];
};

void CSetEffectObject::Add(unsigned long typeId, unsigned long setId)
{
	sSetEffectData983* pData = ((CItemManagerJp*)&Doc()->m_itemManager)->FindSetEffect983(setId);
	if (pData == NULL)
		return;
	int part = 0xf;
	if ((typeId >> 26) == 2)
		part = (typeId >> 18) & 0xff;
	if ((typeId >> 26) != 0x10)
	{
		std::map<unsigned long, CSetEffectState>::iterator it = m_states.find(setId);
		if (it != m_states.end())
		{
			it->second.m_pData = pData;
			if (it->second.IsComplete())
				return;
			it->second.Add(typeId, 1);
			return;
		}
		CSetEffectState state;
		state.m_pData = pData;
		state.m_player = m_playerIndex;
		state.Add(typeId, 1);
		state.SetPart(part);
		m_states.insert(std::pair<unsigned long, CSetEffectState>(setId, state));
		return;
	}
	sSetEffectGroup groups[2] =
	{
		{ 0x2c, { 0x2b, 0x24, 0x1f, 0x20, 0x22, 0x23, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2c, 0x25, -1, 0x21 } },
		{ -1, { 0 } },
	};
	for (int i = 0; groups[i].m_id != -1; i++)
	{
		if (groups[i].m_id != setId)
			continue;
		for (int j = 0; j < 15; j++)
		{
			int id = groups[i].m_ids[j];
			if (id == -1)
				continue;
			pData = ((CItemManagerJp*)&Doc()->m_itemManager)->FindSetEffect983(id);
			if (pData == NULL)
				continue;
			std::map<unsigned long, CSetEffectState>::iterator it = m_states.find(id);
			if (it != m_states.end())
			{
				it->second.m_pData = pData;
				if (it->second.IsComplete())
					return;
				it->second.Add(typeId, 1);
				continue;
			}
			CSetEffectState state;
			state.m_pData = pData;
			state.m_player = m_playerIndex;
			state.Add(typeId, 1);
			state.SetPart(part);
			m_states.insert(std::pair<unsigned long, CSetEffectState>(id, state));
		}
	}
}

CSetEffectObject::~CSetEffectObject()
{
	for (std::vector<CSetAbility*>::iterator it = m_abilities.begin(), end = m_abilities.end(); it != end; ++it)
	{
		if (*it != NULL)
			delete *it;
	}
}

void CSetEffectObject::On01(const sControlledParam* pParam)
{
	m_playerIndex = pParam->m_playerIndex;
	Add(pParam->m_pSource->m_typeId, (unsigned long)pParam->m_pSource->m_values[pParam->m_index]);
}

void CSetEffectObject::On09(int)
{
	if (IsActive983())
		ApplyDeferred();
}

CSetEffectObject::CSetEffectObject(eSpecialAbility ability)
	: CControlledObject(ability)
{
	AddController("CSetEffectController");
	m_abilities.resize(9);
}

static CControlledObject* CreateSetEffectObject(eSpecialAbility ability) // 0062b220
{
	return new CSetEffectObject(ability);
}

static bool RegisterSetEffectObject() // 0062b290
{
	return RegisterControlledObject(eSpecialAbility_SetEffect, CreateSetEffectObject);
}

static bool s_bRegistered = RegisterSetEffectObject();
