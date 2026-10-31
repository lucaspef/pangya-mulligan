// JP 983: unidade 005a0c70–005a1170 (CLosslessHitObject). Classe nova do 983; como o CWindMinusObject, com o evento 8 do jogador e, ao ligar,
// bunker zerado, precisão do taco refeita, avisos à "Screen"/"Spot"/"Player" e a frase 0x9c42 no chat.
// A função seguinte (005a1170: cria o singleton CLoginInfo de 00f03a68) é COMDAT de cabeçalho, não desta unidade.
#include "losslesshitobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"
#include "taskmanager.h"
#include "actor.h"
#include "chatmsg.h"
#include "miraclesignskillcontroller.h"

CLosslessHitObject::CLosslessHitObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0xff)
	, m_typeId(0)
	, m_value(0)
	, m_bOn(false)
{
	AddController("CLosslessHitController");
}

CLosslessHitObject::~CLosslessHitObject()
{
}

void CLosslessHitObject::On01(const sControlledParam* pParam)
{
	On03(pParam);
}

static CControlledObject* CreateLosslessHitObject(eSpecialAbility ability) // 005a0d00
{
	return new CLosslessHitObject(ability);
}

void CLosslessHitObject::Reset()
{
	if (m_value == 0 || m_value == 100)
		return;
	m_bOn = false;
	GOLFDOCJP()->DisablePlayerEvent(8, m_playerIndex);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x215, 0, 0, 0, 0, 0);
}

void CLosslessHitObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	memcpy_s(&m_bOn, 1, pRecord, 1);
	if (m_bOn)
		GOLFDOCJP()->EnablePlayerEvent(8, m_playerIndex);
	else
		GOLFDOCJP()->DisablePlayerEvent(8, m_playerIndex);
}

void CLosslessHitObject::On07(int typeId)
{
	if (typeId == 0 || typeId != m_typeId)
		return;
	GOLFDOCJP()->EnablePlayerEvent(8, m_playerIndex);
	m_bOn = true;
	if (!GOLFDOC()->IsCurrentPlayerEvent983(6))
		AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x215, 0, 0, 0, 0, 0);
	unsigned char current = GOLFDOC()->m_currentPlayer;
	GOLFDOC()->GetPlayer(current)->bunker = 0;
	GolfClub().SetAccuracy();
	AfxGetTask()->GetActor("Spot") << MsgObject(NULL, 0x11f, 0, 0, 0, 0, 0);
	// 00d0a804: nome de efeito em CP949 (texto coreano do 983)
	AfxGetTask()->GetActor("Player") << MsgObject(NULL, 0x336, (unsigned long)"\xc7\xc1\xb8\xaf\xbc\xc7\xb8\xb5\xb9\xdf\xb5\xbf", 0, 0, 0, 0);
	CChatMsg::Instance()->AddChatMsg(GetGameString(0x9c42), 0xffffff00, false, false);
	m_28 = false;
}

static bool RegisterLosslessHitObject() // 005a10a0
{
	return RegisterControlledObject(eSpecialAbility_LosslessHit, CreateLosslessHitObject);
}

static bool s_registered = RegisterLosslessHitObject();

void CLosslessHitObject::On03(const sControlledParam* pParam)
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
