// JP 983: unidade 004a8a50–004acd40 (CConditionalAuxpartObject). Classe nova do 983: peças auxiliares 0x7021000x cujo efeito
// (distância, itens, combo, pang) só vale quando o controlador vê a condição da peça; guarda o efeito por jogador no CSharedDoc.
#include "conditionalauxpartobject.h"
#include "shareddocjp.h"
#include "golfdoc.h"
#include "golfball.h"
#include "club.h"
#include "taskmanager.h"
#include "actor.h"
#include "chatmsg.h"
#include "packet.h"
#include "soundmanager.h"
#include "commonutil.h"
#include "miraclesignskillcontroller.h"

#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60

// 00e05140: dois registros de 0x20 bytes; só o typeid é lido. O resto vem do dump (0x0f em +0x18), sentido não levantado.
struct sAuxpartTable983
{
	unsigned long m_typeId; // +0x00
	unsigned long m_04[7];  // +0x04
};
sAuxpartTable983 g_auxpartTable983[2] = { { 0x70210003, { 0, 0, 0, 0, 0, 0x0f, 0 } } };

// 004a8a70: o typeid está na tabela? Ninguém chama no 983 (nem por `call`, nem por endereço).
bool IsInAuxpartTable983(unsigned long typeId)
{
	for (int i = 0; i < 2; ++i)
	{
		if (g_auxpartTable983[i].m_typeId == typeId)
			return true;
	}
	return false;
}

void CConditionalAuxpartObject::On01(const sControlledParam* pParam)
{
	m_playerIndex = pParam->m_00;
	On03(pParam);
}

void CConditionalAuxpartObject::On08(int arg)
{
	m_apply = *(const sAuxpartApply*)arg;
}

int CConditionalAuxpartObject::GetPlayerIndex() const
{
	return m_playerIndex;
}

void CConditionalAuxpartObject::ResetApply()
{
	m_apply.m_playerIndex = 0;
	m_apply.m_typeId = 0;
	m_apply.m_uid = -1;
	m_apply.m_kind = -1;
}

bool CConditionalAuxpartObject::HasApply() const
{
	if (m_apply.m_typeId == 0 || m_apply.m_uid == -1)
		return false;
	return true;
}

const sAuxpartApply* CConditionalAuxpartObject::GetApply() const
{
	return &m_apply;
}

int CConditionalAuxpartObject::GetCount() const
{
	return (int)m_values.size();
}

sAuxpartValue* CConditionalAuxpartObject::GetAt(int index)
{
	if (m_values.empty())
		return NULL;
	return &m_values[index];
}

// 004a9f50 (peça 0x70210003)
static void OnAuxpartDouble(const sAuxpartValue&)
{
	AfxGetTask()->GetActor("Spot") << MsgObject(NULL, 0x11f, 0, 0, 0, 0, 0);
	// 00d02160: nome de efeito em CP949 (texto coreano do 983)
	AfxGetTask()->GetActor("Player") << MsgObject(NULL, 0x336, (unsigned long)"\xbd\xba\xc5\xe8\xba\xea\xb8\xb5\xb0\xc5\xb8\xb5\xb9\xdf\xb5\xbf", 0, 0, 0, 0);
	g_audio->PlaySfx("2x");
}

void CConditionalAuxpartObject::SyncDoc()
{
	DOCJP()->SetAuxpartValues983(&m_applied, m_playerIndex);
}

// 004aa670: bônus do par — o `short` de +0xde do item quando o jogador da vez também veste a peça de typeid +0xe0
float GetPairBonus(const IFF_STRUCT::sAuxPart* pItem)
{
	const unsigned long* pParts = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tidAuxParts;
	for (int i = 0; i < 5; ++i)
	{
		if (AuxPartPairTypeId983(pItem) == pParts[i])
			return (float)AuxPartPairBonus983(pItem);
	}
	return 0.0f;
}

// 004aa6e0 (peças 0x70210005 e 0x70210008)
static void OnAuxpartPair(const sAuxpartValue& value)
{
	if (GolfBall().m_groundType == 2)
		return;
	if (GolfClub().GetType() == 3)
		return;
	if (GetPairBonus(value.m_pItem) == 0.0f)
		return;
	AfxGetTask()->GetActor("Player") << MsgObject(NULL, 0x335, value.m_pItem->c.TypeId, 0, 0, 0, 0);
}

// 004aa7d0: pacote 0x15d com { typeid, valor, kind }
void CConditionalAuxpartObject::Send(int index, int kind)
{
	sAuxpartValue value = m_values[index];
	const IFF_STRUCT::sAuxPart* pItem = value.m_pItem;
	unsigned long amount = 0;
	WSendPacket packet((enumClientPacket)0x15d);
	packet.Encode4(pItem->c.TypeId);
	switch (kind)
	{
	case 0:
		amount = pItem->DriveUp;
		break;
	case 1:
		if (pItem->ItemDropUp != 0)
			amount = pItem->ItemDropUp;
		break;
	case 2:
		if (pItem->ComboUp != 0)
			amount = pItem->ComboUp;
		break;
	case 3:
		if (pItem->PangUp != 0)
			amount = pItem->PangUp;
		break;
	case 4:
		{
			unsigned long type = pItem->c.TypeId & 0xffff;
			if (type == 9)
				amount = type;
			else if (type == 0xb || type == 0x83)
				amount = 0xb;
		}
		break;
	}
	packet.Encode4((unsigned long)(GetPairBonus(pItem) + (float)amount));
	packet.Encode1((unsigned char)kind);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

sAuxpartValue* CConditionalAuxpartObject::Find(unsigned long typeId)
{
	if (m_values.empty())
		return NULL;
	std::vector<sAuxpartValue>::iterator it = m_values.begin();
	for (; it != m_values.end(); ++it)
	{
		if (it->m_pItem->c.TypeId == typeId)
			break;
	}
	return &*it;
}

// 004aadb0 (peças 0x70210000, 0x70210001 e 0x70210006): "%s님의 %s효과로 비거리가 증가합니다." no chat
static void OnAuxpartDistance(const sAuxpartValue& value)
{
	if (GolfBall().m_groundType == 2)
		return;
	if (GolfClub().GetType() == 3)
		return;
	AfxGetTask()->GetActor("Player") << MsgObject(NULL, 0x335, value.m_pItem->c.TypeId, 0, 0, 0, 0);
	// 00d0219c: texto em CP949 (coreano do 983)
	const char* text = MakeStr(K2L_Compatibility("%s\xb4\xd4\xc0\xc7 %s\xc8\xbf\xb0\xfa\xb7\xce \xba\xf1\xb0\xc5\xb8\xae\xb0\xa1 \xc1\xf5\xb0\xa1\xc7\xd5\xb4\xcf\xb4\xd9."),
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].info.sNick, value.m_pItem->c.Name);
	CChatMsg::Instance()->AddChatMsg(text, 0xffffff00, false, false);
	g_audio->PlaySfx("caddie_XXX");
}

// 004aaf70: chama o tratador da peça e avisa conforme o `kind` (o índice do jogador não é usado)
void CConditionalAuxpartObject::Notify(int kind, const sAuxpartValue* pValue, int)
{
	std::map<unsigned long, Handler>::iterator it = m_handlers.find(pValue->m_pItem->c.TypeId);
	if (it != m_handlers.end())
		it->second(*pValue);
	switch (kind)
	{
	case 0:
		AfxGetTask()->GetActor("Spot") << MsgObject(NULL, 0x11f, 0, 0, 0, 0, 0);
		break;
	case 2:
		CChatMsg::Instance()->AddChatMsg(GetGameString(0x9c41), 0xffffff00, false, false);
		break;
	}
}

// 004ab200: calcula o valor do efeito da 1ª peça registrada, guarda-o na posição da peça em tidAuxParts e avisa
void CConditionalAuxpartObject::Apply(const sAuxpartApply* pApply)
{
	if (pApply->m_typeId == 0)
		return;
	int uid = pApply->m_uid;
	if (uid == -1)
		return;
	int playerIndex = pApply->m_playerIndex;
	int kind = pApply->m_kind;
	int count = GetCount();
	for (int i = 0; i < count; ++i)
	{
		sAuxpartValue* pValue = &m_values[i];
		if (pValue->m_pItem == NULL)
			continue;
		const IFF_STRUCT::sAuxPart* pItem = pValue->m_pItem;
		float value = 0.0f;
		switch (kind)
		{
		case 0:
			value = (float)pItem->DriveUp;
			break;
		case 1:
			if (pItem->ItemDropUp != 0)
				value = (float)(pItem->ItemDropUp / 100.0);
			break;
		case 2:
			if (pItem->ComboUp != 0)
				value = (float)(pItem->ComboUp / 100.0);
			break;
		case 3:
			if (pItem->PangUp != 0)
				value = (float)(pItem->PangUp / 100.0);
			break;
		case 4:
			{
				unsigned long type = pItem->c.TypeId & 0xffff;
				if (type == 9)
					value = 9.0f;
				else if (type == 0xb || type == 0x83)
					value = 11.0f;
			}
			break;
		}
		value = GetPairBonus(pItem) + value;
		pValue->m_value = value;
		pValue->m_uid = uid;
		pValue->m_kind = kind;
		const unsigned long* pParts = Doc()->m_userInfo[playerIndex].charInfo.tidAuxParts;
		for (int slot = 0; slot < 5; ++slot)
		{
			if (pParts[slot] == pItem->c.TypeId)
			{
				m_applied[slot] = *pValue;
				break;
			}
		}
		if (pValue->m_bSilent == 0)
			Notify(kind, pValue, playerIndex);
		return;
	}
}

void CConditionalAuxpartObject::On07(int arg)
{
	if (arg == 0)
		return;
	Apply((const sAuxpartApply*)arg);
	m_28 = false;
	ResetApply();
}

void CConditionalAuxpartObject::On03(const sControlledParam* pParam)
{
	IFF_STRUCT::sAuxPart* pItem = ItemManager()->FindAuxPart(pParam->m_pSource->m_typeId);
	if (pItem == NULL)
		return;
	sAuxpartValue value;
	value.m_pItem = pItem;
	value.m_uid = Doc()->m_userInfo[m_playerIndex].info.dwUID;
	if (pParam->m_pSource->m_values[pParam->m_index] != 0.0f)
		value.m_bSilent = 1;
	m_values.push_back(value);
	for (std::vector<sAuxpartValue>::iterator it = m_values.begin(); it != m_values.end(); ++it)
	{
		m_record.Add(&it->m_kind);
		m_record.Add(&it->m_value);
	}
	m_record.Commit();
}

// 004ac4c0: desfaz o efeito de todas as peças e refaz os 5 elementos vazios de m_applied
void CConditionalAuxpartObject::ResetAll()
{
	for (std::vector<sAuxpartValue>::iterator it = m_values.begin(); it != m_values.end(); ++it)
		it->Reset();
	m_applied.clear();
	m_applied.assign(5, sAuxpartValue());
}

void CConditionalAuxpartObject::On10(int index)
{
	ResetAll();
	SyncDoc();
	char* pRecord = (char*)m_record.Get(index);
	for (std::vector<sAuxpartValue>::iterator it = m_values.begin(); it != m_values.end(); ++it)
	{
		memcpy_s(&it->m_kind, 4, pRecord, 4);
		pRecord += 4;
		memcpy_s(&it->m_value, 4, pRecord, 4);
		pRecord += 4;
	}
	for (int i = 0; i < GetCount(); ++i)
	{
		sAuxpartValue* pValue = GetAt(i);
		if (pValue == NULL || pValue->m_kind == -1)
			continue;
		sAuxpartApply apply;
		apply.m_playerIndex = 0;
		apply.m_typeId = pValue->m_pItem->c.TypeId;
		apply.m_uid = pValue->m_uid;
		apply.m_kind = pValue->m_kind;
		Apply(&apply);
	}
}

CConditionalAuxpartObject::~CConditionalAuxpartObject()
{
}

CConditionalAuxpartObject::CConditionalAuxpartObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0)
{
	AddController("CConditionalAuxpartController");
	m_values.reserve(5); // 004acae5: 004ab3d0 = reserve em +0x50 (resize deixava 5 elementos sem item -> queda no Check)
	ResetApply();
	m_handlers.insert(std::make_pair(0x70210000UL, &OnAuxpartDistance));
	m_handlers.insert(std::make_pair(0x70210001UL, &OnAuxpartDistance));
	m_handlers.insert(std::make_pair(0x70210003UL, &OnAuxpartDouble));
	m_handlers.insert(std::make_pair(0x70210005UL, &OnAuxpartPair));
	m_handlers.insert(std::make_pair(0x70210006UL, &OnAuxpartDistance));
	m_handlers.insert(std::make_pair(0x70210008UL, &OnAuxpartPair));
}

static CControlledObject* CreateConditionalAuxpartObject(eSpecialAbility ability) // 004acc80
{
	return new CConditionalAuxpartObject(ability);
}

static bool RegisterConditionalAuxpartObject() // 004accf0
{
	return RegisterControlledObject(eSpecialAbility_ConditionalAuxpart, CreateConditionalAuxpartObject);
}

static bool s_registered = RegisterConditionalAuxpartObject();
