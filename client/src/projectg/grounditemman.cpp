#include "minatl.h"
#include "grounditemman.h"
#include "grounditem.h"
#include "quadtree.h"
#include "contentsdoc.h"
#include "golfdoc.h"
#include "projectg.h"
#include "wresrcmng.h"
#include "mathconsts.h"

IObject* GroundItemManMakeInstance()
{
	return new GroundItemMan;
}

struct __impGroundItemMan
{
	__impGroundItemMan()
	{
		ObjectFactory().AddObjectFunctor(GroundItemManMakeInstance,
			"GroundItemMan");
		ActorFactory().AddActorClass("GroundItemMan");
	}
};

const WRTTI GroundItemMan::m_RTTI("GroundItemMan", &NetSyncActor::m_RTTI);
static __impGroundItemMan __implGroundItemMan;

unsigned int GroundItemMan::ms_initCnt = 0;

GroundItemMan::GroundItemMan()
{
	CContentsDoc::Instance()->GetContainer((localContentType_t)0x9b, // 983: era 0xa3 (push 0x9b no 0085dc70)
		m_pGimmickContainer);
	m_dispPriority = DISP_PRIORITY_5; // 983
	m_pPlayerStats = NULL; // 983
}

GroundItemMan::~GroundItemMan()
{
}

void GroundItemMan::OnLoad()
{
	// 983 0085d650: so aloca os contadores; o test_table vai no OnInit
	m_pPlayerStats = new PlayerStats[Doc()->m_pGolfDoc->m_playerNum];
}

void GroundItemMan::OnReload()
{
}

void GroundItemMan::OnInit()
{
	if (!OnlinePlay())
		m_pGimmickContainer->LoadFromScript("test_table");

	NetSyncActor::OnInit();

	m_pGimmickContainer->ResetGimmikFlag(Doc()->m_pGolfDoc->m_currentHole,
		GimmickDispositionInformation::GIMMICK_UNUSED);

	Ut::MakeObjectName(m_sceneElemName, "SceneElemName", *this, ms_initCnt++);

	CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL, this,
		m_sceneElemName.c_str(), 0, NULL);

	// 983: semente = buraco atual + semente do container
	NSA_SetInitialRandomSeed(Doc()->m_pGolfDoc->m_currentHole + m_pGimmickContainer->GetRandomSeed());

	NSA_GetRandomGenerator().seed(m_initialSeed);

	// 983 (0085e2c0): sem o laco das texturas do 645 (NeedToMakeTriPtArray/GetTriArray); o WPolySoup do 983 nao
	// monta a lista de triangulos por textura.

	if (OnlinePlay())
		PlaceAllGroundItems();

	std::list<GroundItem*>::iterator it = m_groundItemList.begin();

	while (it != m_groundItemList.end())
	{
		GroundItem* pItem = *it;
		pItem->Initialize();
		++it;
	}

	// 983 (mov [esi+0xa30], 0x01010101): item de campo ligado para todos os jogadores/times
	memset(m_enabled983, 1, sizeof(m_enabled983));
}

void GroundItemMan::OnDestroy()
{
	if (m_pPlayerStats) // 983 0085d200
	{
		delete[] m_pPlayerStats;
		m_pPlayerStats = NULL;
	}

	m_pGimmickContainer->Release();

	OnHoleOut();

	NetSyncActor::OnDestroy();
}

void GroundItemMan::HandleMsg(const MsgObject& msg)
{
	NetSyncActor::HandleMsg(msg);

	switch (msg.message)
	{
	case M983(0x277):
		*(std::vector<FieldItem::AcquiredItemInfo>**)msg.param1 =
			&m_pGimmickContainer->GetAcquiredItemInfoArray();
		break;

	case 0x2f7: // 983 (msg983.h 0x27c): quantidade; zera o contador do jogador da vez
		*(int*)msg.param1 = m_groundItemList.size();
		memset(m_pPlayerStats[Doc()->m_pGolfDoc->m_currentPlayer].numAcquired, 0,
			sizeof(int) * 3);
		break;

	case 0x2f6: // 983: flag de habilitado do jogador/time param1 -> *(char*)param2
		*(char*)msg.param2 = m_enabled983[msg.param1];
		break;

	case M983(0x27b): // 983 0x2f5: param1 = ligar, param2 = jogador/time
		m_enabled983[msg.param2] = (msg.param1 == 1);
		EnableGroundItemAll(msg.param1 == 1);
		break;

	case 0x2f8: // 983: ha item de campo?
		*(bool*)msg.param1 = !m_groundItemList.empty();
		break;
	}
}

void GroundItemMan::NSC_OnStep(bool bForce, bool bReplay, float step,
	float time, int ballState)
{
	std::list<GroundItem*>::iterator it = m_groundItemList.begin();
	while (it != m_groundItemList.end())
	{
		GroundItem* pItem = *it;
		pItem->NSC_OnStep(bForce, bReplay, step, time, ballState);
		++it;
	}
}

void GroundItemMan::NSC_OnFirstNewShotFrame(float time, int ballState)
{
	bool bJackpot = false;
	unsigned long r = NSA_GetRandomGenerator().randInt(999999);

	if ((Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer]
				.userEquip.tidBall &
			0x3ffffff) == 0)
	{
		if (r < 20000)
			bJackpot = true;
	}
	else
	{
		if (r < 40000)
			bJackpot = true;
	}

	GroundItem::EnableGreenItemJackpot(bJackpot);

	// 983 0085da80: nao zera numAcquired aqui (so na msg 0x2f7, HandleMsg)

	std::list<GroundItem*>::iterator it = m_groundItemList.begin();
	while (it != m_groundItemList.end())
	{
		GroundItem* pItem = *it;
		pItem->NSC_OnFirstNewShotFrame(time, ballState);
		++it;
	}
}

void GroundItemMan::NSC_OnFirstReplayFrame(int ballState)
{
	std::list<GroundItem*>::iterator it = m_groundItemList.begin();

	while (it != m_groundItemList.end())
	{
		GroundItem* pItem = *it;
		pItem->NSC_OnFirstReplayFrame(ballState);
		++it;
	}
}

void GroundItemMan::OnHoleOut()
{
	// 983 0085e1f0: descarta os itens pegos ainda nao enviados (0x1c) antes de soltar os itens do buraco
	m_pGimmickContainer->GetAcquiredItemInfoArray().clear();

	std::list<GroundItem*>::iterator it = m_groundItemList.begin();

	while (it != m_groundItemList.end())
	{
		GroundItem* pItem = *it;
		pItem->Terminate();
		delete pItem;
		++it;
	}

	m_groundItemList.clear();

	CSceneManager::Instance()->DeleteElement(m_sceneElemName.c_str());
}

void GroundItemMan::Display()
{
}

void GroundItemMan::DebugDisplay()
{
	std::list<GroundItem*>::iterator it = m_groundItemList.begin();

	while (it != m_groundItemList.end())
	{
		GroundItem* pItem = *it;
		pItem->DrawBoundingVolume();
		++it;
	}

	const ThSphere& ballVolume = GroundItem::GetBallBoundingVolume();
	g_view->DrawSphere((const WVector&)ballVolume.pos, ballVolume.r, 0xffff0000,
		10, 0);
}

// 983 0085d6c0: coloca todos os itens da tabela do buraco (info + posicao +0x14 + tipo de textura +0x20)
int GroundItemMan::PlaceAllGroundItems()
{
	std::vector<GimmickDispositionInformation>& list =
		m_pGimmickContainer->GetGimmickPosition(GOLFDOC()->m_currentHole);

	for (unsigned int i = 0; i < list.size(); ++i)
	{
		gaV3 pos;
		pos.x = list[i].posX983;
		pos.y = list[i].posY983;
		pos.z = list[i].posZ983;
		PlaceGroundItem(&list[i], &pos,
			(FieldItem::eDisposeTextureType)list[i].texType983);
	}

	return list.size();
}

// 983 0085e450
void GroundItemMan::PlaceGroundItem(GimmickDispositionInformation* pInfo,
	const gaV3* pPos, FieldItem::eDisposeTextureType texType)
{
	Ut::Spatial spatial;
	spatial.pos = *pPos;

	GroundItem* pItem =
		CreateGroundItem(pInfo->index, pInfo->type, texType, spatial);
	m_groundItemList.push_back(pItem);

	pInfo->flag = GimmickDispositionInformation::GIMMICK_USED;
}

// 983 0085d780: 0/3 = moeda, 1 = qualquer, 2 = caixa
GimmickDispositionInformation* GroundItemMan::FindUnusedDisposeInfo(
	int texType)
{
	GIMMICK_ITEM_TYPE type;

	switch (texType)
	{
	case 0:
	case 3:
		type = GIMMICK_ITEM_PANG;
		break;
	case 1:
		type = GIMMICK_ITEM_ANY;
		break;
	case 2:
		type = GIMMICK_ITEM_BOX;
		break;
	default:
		return NULL;
	}

	return m_pGimmickContainer->FindDisposeInfo(GOLFDOC()->m_currentHole, type,
		GimmickDispositionInformation::GIMMICK_UNUSED);
}

GroundItem* GroundItemMan::CreateGroundItem(unsigned long id,
	GIMMICK_ITEM_TYPE type, FieldItem::eDisposeTextureType texType,
	const Ut::Spatial& spatial)
{
	GroundItem* pItem = NULL;
	std::string modelName;
	Ut::Spatial newSpatial = spatial;

	switch (type)
	{
	case GIMMICK_ITEM_PANG:
		modelName = "coin.pet";
		// 983 (0085df20): escala 20 (10 no mapa 0x13)
		newSpatial.scl = GetCurMap() == 0x13 ? 10.0f : 20.0f;
		newSpatial.pos.y += 0.1f;
		pItem = new GroundItem(this, id, GIMMICK_ITEM_PANG, texType,
			modelName.c_str(), newSpatial);
		break;

	case GIMMICK_ITEM_BOX:
		// 983 (0085df20): ct_event_05.pet; mapa 0x13 = escala 14 / y-3, demais = escala 10 / y-18
		modelName = "ct_event_05.pet";
		if (GetCurMap() == 0x13)
		{
			newSpatial.scl = 14.0f;
			newSpatial.pos.y -= 3.0f;
		}
		else
		{
			newSpatial.scl = 10.0f;
			newSpatial.pos.y -= 18.0f;
		}
		pItem = new GroundItem(this, id, GIMMICK_ITEM_BOX, texType,
			modelName.c_str(), newSpatial);
		break;
	}

	return pItem;
}

void GroundItemMan::EnableGroundItemAll(bool bEnable)
{
	std::list<GroundItem*>::iterator it = m_groundItemList.begin();

	while (it != m_groundItemList.end())
	{
		(*it)->Enable(bEnable);
		++it;
	}
}
