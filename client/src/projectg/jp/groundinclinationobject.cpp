// JP 983: unidade 0055f290–0055fe40 (CGroundInclinationObject). Classe nova do 983. Entre as funções próprias o 983 guarda as
// instâncias de std::vector<std::pair<unsigned long, unsigned long>> (0055f2b0…0055f4d0, 0055f5a0…0055f660, 0055f6f0…0055f770,
// 0055f950…0055fac0, 0055fc30). O corte da F3.7.1 (fim em 0055fee0) estava longo: 0055fe40–0055fee0 são acessos a campos
// +0xb0/+0x1fc/+0x244 de outra classe (sem dono).
#include "groundinclinationobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"
#include "taskmanager.h"
#include "actor.h"
#include "chatmsg.h"
#include "messagemanager.h"
#include "commonutil.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B1399 temporario

void CGroundInclinationObject::On08(int typeId)
{
	m_activeTypeId = typeId;
}

void CGroundInclinationObject::Reset()
{
	unsigned char count = GOLFDOC()->m_playerNum;
	for (unsigned char i = 0; i < count; ++i)
		GOLFDOCJP()->DisablePlayerEvent(6, i);
	AfxGetTask()->GetActor("Player") << MsgObject(NULL, 0x280, 0, 0, 0, 0, 0);
	m_activeTypeId = 0;
}

void CGroundInclinationObject::On05()
{
	Reset();
}

void CGroundInclinationObject::On06(int)
{
	if (!m_28)
		Reset();
}

void CGroundInclinationObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	if (pRecord == NULL)
		return;
	memcpy_s(&m_activeTypeId, 4, pRecord, 4);
	if (m_activeTypeId != 0)
		On07(m_activeTypeId);
}

void CGroundInclinationObject::On07(int typeId)
{
	m_activeTypeId = typeId;
	jp983_rastro("B1399 GI On07 tid=%08x player=%d", typeId, m_playerIndex); // temporario
	GOLFDOCJP()->EnablePlayerEvent(4, m_playerIndex);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x27f, 0, 0, 0, 0, 0);
	jp983_rastro("B1399 GI On07 screen ok online=%d msg=%p", (int)OnlinePlay(),
		_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x12f));
	if (OnlinePlay())
	{
		std::string name = ItemManager()->GetItemName(typeId);
		std::string text = MakeStr(_systemmsg::CMessageManager::Instance()->GetMsg((_systemmsg::eMsgIdentifier)0x12f), name.c_str());
		CChatMsg::Instance()->AddChatMsg(text, 0xffffff00, false, false);
	}
	m_28 = false;
}

CGroundInclinationObject::CGroundInclinationObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_typeId(0)
	, m_playerIndex(0)
	, m_activeTypeId(0)
{
	AddController("CGroundInclinationController");
}

CGroundInclinationObject::~CGroundInclinationObject()
{
}

static CControlledObject* CreateGroundInclinationObject(eSpecialAbility ability) // 0055fca0
{
	return new CGroundInclinationObject(ability);
}

static bool RegisterGroundInclinationObject() // 0055fd10
{
	return RegisterControlledObject(eSpecialAbility_GroundInclination, CreateGroundInclinationObject);
}

static bool s_registered = RegisterGroundInclinationObject();

void CGroundInclinationObject::On01(const sControlledParam* pParam)
{
	m_typeId = pParam->m_pSource->m_typeId;
	m_playerIndex = pParam->m_playerIndex;
	m_record.Add(&m_activeTypeId);
	m_record.Commit();
	m_items.push_back(std::pair<unsigned long, unsigned long>(pParam->m_pSource->m_typeId,
		(unsigned long)pParam->m_pSource->m_values[pParam->m_index]));
}
