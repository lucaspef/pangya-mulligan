// JP 983: unidade 00b45a20–00b46570 (CWindVolumeObject). Classe nova do 983. Entre as funções próprias o 983 guarda as
// instâncias de std::vector<std::pair<unsigned long, unsigned long>> (00b45a40…00b45d60, 00b45ea0…00b45f00, 00b460e0…00b46250,
// 00b463b0). O corte da F3.7.1 (fim em 00b46a12) estava longo: 00b46570–00b46a12 é a saída de erro da biblioteca kmlocal.
#include "windvolumeobject.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "taskmanager.h"
#include "actor.h"
#include "gatewayactor.h"
#include "chatmsg.h"
#include "commonutil.h"

#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60

void CWindVolumeObject::On08(int typeId)
{
	m_activeTypeId = typeId;
}

void CWindVolumeObject::On05()
{
	unsigned char current = GOLFDOC()->m_currentPlayer;
	GOLFDOC()->GetPlayer(current)->windAbility983 = 0; // 983 00b45d80/00b45e10: +0x4c0 (B1395)
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0x17, 0, 0, 0, 0);
}

void CWindVolumeObject::On06(int)
{
	unsigned char current = GOLFDOC()->m_currentPlayer;
	GOLFDOC()->GetPlayer(current)->windAbility983 = 0; // 983 00b45d80/00b45e10: +0x4c0 (B1395)
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0x17, 0, 0, 0, 0);
}

void CWindVolumeObject::On07(int)
{
	if (m_activeTypeId == 0)
		return;
	unsigned char current = GOLFDOC()->m_currentPlayer;
	// 983 00b45f20: +0x4c0 = 1 (B1395; antes gravava em item): CWind::ApplyPlayerItem983 caso 1 normaliza o vento = 1m
	GOLFDOC()->GetPlayer(current)->windAbility983 = 1;
	AfxGetTask()->GetActor("Player") << MsgObject(NULL, 0x31b, m_playerIndex, m_activeTypeId, 0, 0, 0);
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(m_activeTypeId);
	if (pItem != NULL)
	{
		// 00d548ec: "%s의 신비한 능력이 발동되어 바람의 세기가 1이 됩니다." em CP949 (texto coreano do 983)
		const char* str = MakeStr(K2L_Compatibility("%s\xc0\xc7 \xbd\xc5\xba\xf1\xc7\xd1 \xb4\xc9\xb7\xc2\xc0\xcc \xb9\xdf\xb5\xbf\xb5\xc7\xbe\xee "
			"\xb9\xd9\xb6\xf7\xc0\xc7 \xbc\xbc\xb1\xe2\xb0\xa1 1\xc0\xcc \xb5\xcb\xb4\xcf\xb4\xd9."), pItem->Name);
		std::string text = str;
		CChatMsg::Instance()->AddChatMsg(text, 0xffffff00, false, false);
	}
	m_28 = false;
	m_activeTypeId = 0;
	AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0x17, 0, 0, 0, 0);
}

CWindVolumeObject::CWindVolumeObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_activeTypeId(0)
{
	AddController("CWindVolumeController");
}

CWindVolumeObject::~CWindVolumeObject()
{
}

static CControlledObject* CreateWindVolumeObject(eSpecialAbility ability) // 00b46420
{
	return new CWindVolumeObject(ability);
}

static bool RegisterWindVolumeObject() // 00b46490
{
	return RegisterControlledObject(eSpecialAbility_WindVolume, CreateWindVolumeObject);
}

static bool s_registered = RegisterWindVolumeObject();

void CWindVolumeObject::On01(const sControlledParam* pParam)
{
	m_items.push_back(std::pair<unsigned long, unsigned long>(pParam->m_pSource->m_typeId,
		(unsigned long)pParam->m_pSource->m_values[pParam->m_index]));
	m_playerIndex = pParam->m_00;
}
