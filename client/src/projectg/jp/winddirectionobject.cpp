// JP 983: unidade 0082dd80–0082e330 (CWindDirectionObject). Classe nova do 983. Começa pelas cópias fora de linha de oito
// acessos aos campos +0x58…+0x68 (0082dd80…0082ddf0; no fonte são embutidos) e guarda a cópia de CWind::GetDirection (0082dfd0).
// O corte da F3.7.1 parava em 0082e330 certo; o que vem depois é o WorkShopEvent2013.
#include "winddirectionobject.h"
#include "classicwind.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "chatmsg.h"
#include "commonutil.h"

#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60

CWindDirectionObject::~CWindDirectionObject()
{
}

void CWindDirectionObject::On05()
{
	Reset();
}

void CWindDirectionObject::On10(int index)
{
	if (index == 0)
		On05();
	else
		On06(0);
	char* pRecord = (char*)m_record.Get(index);
	if (pRecord != NULL)
	{
		// o 983 grava m_newDirection (On01) e lê em m_sumDirection: assim no original
		memcpy_s(&m_kind, 4, pRecord, 4);
		pRecord += 4;
		memcpy_s(&m_sumDirection, 4, pRecord, 4);
		pRecord += 4;
		memcpy_s(&m_bActive, 1, pRecord, 1);
		if (m_bActive)
			Wind().SetDirection(GetSumDirection());
	}
}

void CWindDirectionObject::ClearChange()
{
	m_change.kind = 0;
	m_change.direction = 0.0f;
}

void CWindDirectionObject::SetChange(const sWindDirectionChange* pChange)
{
	m_change.direction = pChange->direction;
	m_change.kind = pChange->kind;
}

sWindDirectionChange* CWindDirectionObject::GetChange()
{
	return &m_change;
}

CWindDirectionObject::CWindDirectionObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_typeId(0)
	, m_playerIndex(0)
	, m_kind(0)
	, m_oldDirection(0.0f)
	, m_sumDirection(0.0f)
	, m_newDirection(0.0f)
	, m_bActive(false)
{
	AddController("CWindDirectionController");
	ClearChange();
}

void CWindDirectionObject::On08(int pChange)
{
	if (pChange != 0)
		SetChange((const sWindDirectionChange*)pChange);
}

static CControlledObject* CreateWindDirectionObject(eSpecialAbility ability) // 0082dff0
{
	return new CWindDirectionObject(ability);
}

static inline bool IsWindRestoreGame(unsigned char gameType)
{
	switch (gameType)
	{
	case 4: case 5: case 6: case 9: case 10: case 11:
		return true;
	}
	return false;
}

// 0082e060: fim da tacada — nos modos 4…7 e 9…11 devolve a direção anterior ao vento
void CWindDirectionObject::On06(int)
{
	if (m_bActive == true)
	{
		unsigned char gameType = Doc()->m_golfGame.gameType;
		if (IsWindRestoreGame(gameType) || gameType == 7)
			Wind().ChangeDirection983(GetOldDirection(), true);
	}
	Reset();
}

void CWindDirectionObject::On07(int arg)
{
	const sWindDirectionChange* pChange = (const sWindDirectionChange*)arg;
	SetActive(true);
	switch (pChange->kind)
	{
	case 0:
	case 1:
		SetKind(1);
		break;
	case 2:
	case 3:
		SetKind(2);
		break;
	}
	SetSumDirection(Wind().GetDirectionSum983());
	SetOldDirection(WINDJP()->GetDirection983());
	unsigned char index = m_playerIndex;
	if (GOLFDOC()->GetPlayer(index) == NULL)
		return;
	SetNewDirection(pChange->direction);
	Wind().ChangeDirection983(GetNewDirection(), false);
	{
		// 00d300c0: "%s의 힘으로 바람의 방향이 바뀝니다." em CP949 (texto coreano do 983); o 983 não confere o item nulo
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(m_typeId);
		const char* str = MakeStr(K2L_Compatibility("%s\xc0\xc7 \xc8\xfb\xc0\xb8\xb7\xce \xb9\xd9\xb6\xf7\xc0\xc7 \xb9\xe6\xc7\xe2\xc0\xcc "
			"\xb9\xd9\xb2\xf2\xb4\xcf\xb4\xd9."), pItem->Name);
		std::string text = str;
		CChatMsg::Instance()->AddChatMsg(text, 0xffffff00, false, false);
	}
	ClearChange();
	m_28 = false;
}

static bool RegisterWindDirectionObject() // 0082e260
{
	return RegisterControlledObject(eSpecialAbility_WindDirection, CreateWindDirectionObject);
}

static bool s_registered = RegisterWindDirectionObject();

void CWindDirectionObject::On01(const sControlledParam* pParam)
{
	m_typeId = pParam->m_pSource->m_typeId;
	m_playerIndex = pParam->m_playerIndex;
	m_record.Add(&m_kind);
	m_record.Add(&m_newDirection);
	m_record.Add(&m_bActive);
	m_record.Commit();
}
