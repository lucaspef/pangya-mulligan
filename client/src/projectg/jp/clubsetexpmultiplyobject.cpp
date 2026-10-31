// JP 983: unidade do CClubsetExpMultiplyObject (faixa 0056a440–0056ac90 com o par; gêmea do CLosslessHitObject, gerada por tools/f37/gemeo2.py e conferida no 983).
#include "clubsetexpmultiplyobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"
#include "taskmanager.h"
#include "actor.h"
#include "chatmsg.h"
#include "miraclesignskillcontroller.h"

CClubsetExpMultiplyObject::CClubsetExpMultiplyObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0xff)
	, m_typeId(0)
	, m_value(0)
	, m_bOn(false)
{
	AddController("CClubsetExpMultiplyController");
}

CClubsetExpMultiplyObject::~CClubsetExpMultiplyObject()
{
}

void CClubsetExpMultiplyObject::On01(const sControlledParam* pParam)
{
	On03(pParam);
}

static CControlledObject* CreateClubsetExpMultiplyObject(eSpecialAbility ability) // 0056a820
{
	return new CClubsetExpMultiplyObject(ability);
}

void CClubsetExpMultiplyObject::Reset()
{
	if (m_value == 0 || m_value == 100)
		return;
	m_bOn = false;
	GOLFDOCJP()->DisablePlayerEvent(8, m_playerIndex);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x215, 0, 0, 0, 0, 0);
}

void CClubsetExpMultiplyObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	memcpy_s(&m_bOn, 1, pRecord, 1);
	if (m_bOn)
		GOLFDOCJP()->EnablePlayerEvent(8, m_playerIndex);
	else
		GOLFDOCJP()->DisablePlayerEvent(8, m_playerIndex);
}

void CClubsetExpMultiplyObject::On07(int typeId)
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

static bool RegisterClubsetExpMultiplyObject() // 0056abc0
{
	return RegisterControlledObject(eSpecialAbility_ClubsetExpMultiply, CreateClubsetExpMultiplyObject);
}

static bool s_registered = RegisterClubsetExpMultiplyObject();

void CClubsetExpMultiplyObject::On03(const sControlledParam* pParam)
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
