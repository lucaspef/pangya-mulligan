// JP 983: unidade 005e07a0–005e0ca0 (CSafetyItemObject). Classe nova do 983; como o CLosslessHitObject, com o evento 2 do jogador e, ao ligar,
// a frase 0x9c47 com o apelido do dono no chat, o aviso 0x753b ao "Gateway" e o 0x27f à "Screen"; slot 8 vazio próprio (005e0830).
#include "safetyitemobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"
#include "taskmanager.h"
#include "actor.h"
#include "chatmsg.h"
#include "miraclesignskillcontroller.h"
#include "gatewayactor.h"
#include "commonutil.h"

CSafetyItemObject::CSafetyItemObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0xff)
	, m_typeId(0)
	, m_value(0)
	, m_bOn(false)
{
	AddController("CSafetyItemController");
}

CSafetyItemObject::~CSafetyItemObject()
{
}

void CSafetyItemObject::On01(const sControlledParam* pParam)
{
	On03(pParam);
}

void CSafetyItemObject::On08(int)
{
}

static CControlledObject* CreateSafetyItemObject(eSpecialAbility ability) // 005e0840
{
	return new CSafetyItemObject(ability);
}

void CSafetyItemObject::Reset()
{
	if (m_value == 0 || m_value == 100)
		return;
	m_bOn = false;
	GOLFDOCJP()->DisablePlayerEvent(2, m_playerIndex);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x215, 0, 0, 0, 0, 0);
}

void CSafetyItemObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	memcpy_s(&m_bOn, 1, pRecord, 1);
	if (m_bOn)
		GOLFDOCJP()->EnablePlayerEvent(2, m_playerIndex);
	else
		GOLFDOCJP()->DisablePlayerEvent(2, m_playerIndex);
}

void CSafetyItemObject::On07(int typeId)
{
	if (typeId == 0 || typeId != m_typeId)
		return;
	GOLFDOCJP()->EnablePlayerEvent(2, m_playerIndex);
	m_bOn = true;
	std::string nick = Doc()->m_userInfo[m_playerIndex].info.sNick;
	{
		std::string text = MakeStr(GetGameString(0x9c47), nick.c_str());
		CChatMsg::Instance()->AddChatMsg(text, 0xffffff00, false, false);
	}
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 6, 0x18000028, 0, 0, 0);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x27f, 0, 0, 0, 0, 0);
	m_28 = false;
}

static bool RegisterSafetyItemObject() // 005e0bd0
{
	return RegisterControlledObject(eSpecialAbility_SafetyItem, CreateSafetyItemObject);
}

static bool s_registered = RegisterSafetyItemObject();

void CSafetyItemObject::On03(const sControlledParam* pParam)
{
	if (ItemManager()->FindCommonItem(pParam->m_pSource->m_typeId) == NULL)
		return;
	m_playerIndex = pParam->m_playerIndex;
	m_typeId = pParam->m_pSource->m_typeId;
	m_value = (unsigned char)(int)pParam->m_pSource->m_values[pParam->m_index];
	m_bOn = false;
	m_record.Add(&m_bOn);
	m_record.Commit();
}
