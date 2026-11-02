#include "minatl.h"
#include "realmyroomdata.h"
#include "vc10_layout.h"
#include "actor.h"
#include "commonutil.h"
#include "fresh.h"
#include "frform.h"
#include "frwndmanager.h"
#include "golfdoc.h"
#include "../shared/localize.h"
#include "networksystem.h"
#include "packet.h"
#include "projectg.h"
#include "quadtree.h"
#include "scenemanager.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "wbone.h"
#include "wpuppet.h"
#include "wresrcmng.h"

// D892: realmyroomtask.cpp (0x125) le m_ArrangeList por Instance()+VC10_OFS_BO(0x2c) (classe opaca la); cl 14.44 = 0x28
C_ASSERT(offsetof(CRealMyRoomData, m_ArrangeList) == VC10_OFS_BO(0x2c));

extern Fresh* g_pFresh;

int WisZero(const float& f, float eps);

// wpuppet.h only declares it; the original has the inline body.
inline void WPuppet::SetLightMode(unsigned long mode)
{
	if (0)
		((CRealMyRoomData*)0)->CRealMyRoomData::Process(*(float*)0);
	m_lightmode = mode;
}

// HACK: unused, but they make the compiler emit g_HUGE and g_CM_TO_WU, which
// the original's constant table (wmath.h) has between the ones used here.
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

static float CmToWu(float cm)
{
	return cm * g_CM_TO_WU;
}

// realmyroomdata.cpp:39
CRealMyRoomData::CRealMyRoomData()
	: m_ArrangeList(16, 16)
{
	m_ArrangeList.Reset();

	m_pSelectModel = NULL;
	m_pPickedModel = NULL;

	m_state = 0;

	m_targetPos = WVector(0.0f, -2.0f, 0.0f);

	m_dragPos = m_pickPos = m_targetPos;

	m_bDropped = false;
}

// realmyroomdata.cpp:68
CRealMyRoomData::~CRealMyRoomData()
{
	std::list<sFurniture_List>::iterator it;

	for (it = m_furnitureList.begin(); it != m_furnitureList.end(); ++it)
	{
		sFurniture_List furniture = *it;

		// Wall furniture whose placement changed is reported to the server.
		if ((furniture.typeId & 0x3FF800) == 0x2800)
		{
			for (cModel* pModel = m_ArrangeList.Start(); pModel;
				pModel = m_ArrangeList.Next())
			{
				if (pModel->m_pFurniture->id == furniture.id)
				{
					if (pModel->m_pFurniture->bArrange != furniture.bArrange)
					{
						if (IsLocalContent((localContentType_t)0x67 /*983: era 0x66; interpolado*/))
						{
							WSendPacketSem983 packet(0xCC); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
							packet.Encode4(
								CSharedDoc::Instance()->m_stallItem.stallKey);
							packet.Encode1(furniture.bArrange);
							packet.Encode1(0);
							packet.Send(TO_GAME);

							CTaskManager* pTaskManager =
								CTaskManager::Instance();
							(!pTaskManager ? 0 : pTaskManager->GetCurrentTask())
								->SendMsgToMainActor(
									MsgObject(0, M983(0x1), 0, 0, 0, 0, 0));
						}
						break;
					}
				}
			}
		}
	}

	ResetList(true);
}

// realmyroomdata.cpp:134
void CRealMyRoomData::OnDestory()
{
	for (cModel* pModel = m_ArrangeList.Start(); pModel;
		pModel = m_ArrangeList.Next())
		delete pModel;
	m_ArrangeList.Reset();
}

// realmyroomdata.cpp:139
void CRealMyRoomData::Process(float elapsed)
{
	// The original body (lines 140-189) compiles to nothing.
}

// realmyroomdata.cpp:192
void CRealMyRoomData::Render()
{
	if (0)
		((CRealMyRoomData*)0)->CRealMyRoomData::OnDestory();
	// The selection blinks: its alpha swings between 128 and 253.
	static bool s_bFadeIn = true;
	static int s_alpha = 128;

	if (m_pSelectModel != NULL)
	{
		if (m_state == 1 || m_state == 2)
		{
			if ((m_pSelectModel->m_pFurniture->typeId & 0x3FF800) == 0x7000)
			{
				CSceneManager::Instance()->SetAlpha(
					m_pSelectModel->m_pModelData->name, (unsigned char)s_alpha);

				if (s_bFadeIn == true)
					s_alpha += 3;
				else
					s_alpha -= 3;

				if (s_alpha > 253)
					s_bFadeIn = false;

				if (s_alpha < 128)
					s_bFadeIn = true;
			}
		}
	}
}

// realmyroomdata.cpp:~200
void CRealMyRoomData::OnLoad()
{
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (ItemManager()->m_FurnitureMap.size() != 0)
		{
			for (std::map<unsigned int, IFF_STRUCT::sFurniture>::iterator it =
					 ItemManager()->m_FurnitureMap.begin();
				it != ItemManager()->m_FurnitureMap.end(); ++it)
			{
				sFurniture_List furniture = { 0 };

				furniture.typeId = (*it).second.c.TypeId;
				furniture.id = rand() % 100;
				furniture.x = (*it).second.Pos[0];
				furniture.y = (*it).second.Pos[1];
				furniture.z = (*it).second.Pos[2];
				furniture.r = (*it).second.Pos[3];
				furniture.bArrange = 0;
				furniture.reserved = (*it).second.COM[0];

				if (furniture.typeId)
					AddModel(furniture);
			}
		}
	}
	else
	{
		if (CSharedDoc::Instance()->m_furnitureList.size() != 0)
		{
			std::list<sFurniture_List>::iterator it;
			for (it = CSharedDoc::Instance()->m_furnitureList.begin();
				it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
			{
				IFF_STRUCT::sFurniture* pFurniture =
					ItemManager()->FindFurniture((*it).typeId);
				if (pFurniture)
				{
					char name[256];
					sprintf(name, "%s.pet", pFurniture->Data);

					WVector pos(pFurniture->Pos[0], pFurniture->Pos[1],
						pFurniture->Pos[2]);

					if (GOLFDOC()->m_pPolySoup->FindModel(name) == NULL)
					{
						cModel* pModel = CreateModel(name, pos,
							pFurniture->Pos[3], false, pFurniture->IsMove);
						if (pModel == NULL)
							return;

						m_ArrangeList += pModel;
						m_editModel = *pModel;

						GOLFDOC()->m_pPolySoup->CreateDataModel(
							pModel->m_pModelData);

						pModel->m_pFurniture = &(*it);
						m_state = 1;
						return;
					}
				}
			}
		}
	}
}

// realmyroomdata.cpp:~330
// Checks every placed model against the walls and the other furniture and,
// with bSave, copies the document's furniture list into the room's.
bool CRealMyRoomData::SaveModel(bool bSave)
{
	WList<cModel*> posterList(16, 16);
	bool bCollide = false;
	cModel* pModel;

	for (pModel = m_ArrangeList.Start(); pModel; pModel = m_ArrangeList.Next())
	{
		IFF_STRUCT::sFurniture* pFurniture =
			ItemManager()->FindFurniture(pModel->m_pFurniture->typeId);

		if ((pModel->m_pFurniture->typeId & 0x3FF800) == 0x4000 ||
			(pModel->m_pFurniture->typeId & 0x3FF800) == 0x4800)
			continue;

		if (pModel->m_pModelData == NULL)
			continue;

		m_targetPos = pModel->m_pModelData->pos;

		switch (pFurniture->IsMove)
		{
		case 1:
		case 2:
		case 3:
		{
			trace_s trace;
			GetPVS().TraceOBB_Avatar_genPosition(pModel->m_pModelData->pos,
				m_targetPos, &trace, pModel->m_pModelData);

			if (trace.m_endPos.x == pModel->m_pModelData->pos.x &&
				trace.m_endPos.z == pModel->m_pModelData->pos.z)
				m_targetPos = pModel->m_pModelData->pos;
			else
				m_targetPos = trace.m_endPos;

			GetPVS().TraceOBB_WallSliding(pModel->m_pModelData->pos,
				m_targetPos, &trace, pModel->m_pModelData, 0, false);

			WVector delta = trace.m_endPos - pModel->m_pModelData->pos;
			WVector newPos = delta + pModel->m_pModelData->pos;

			bCollide = GetPVS().TraceOBB_Object(pModel->m_pModelData->pos,
				newPos, pModel->m_pModelData->aabb, &trace,
				pModel->m_pModelData, false);
			break;
		}
		case 5:
			if ((pModel->m_pFurniture->typeId & 0x3FF800) == 0x7000)
			{
				posterList += pModel;
			}
			else
			{
				trace_s trace;
				GetPVS().TraceOBB_WallSliding(pModel->m_pModelData->pos,
					m_targetPos, &trace, pModel->m_pModelData, 1, false);

				WVector delta = trace.m_endPos - pModel->m_pModelData->pos;
				m_targetPos = delta + pModel->m_pModelData->pos;

				bCollide = GetPVS().TraceOBB_Object(pModel->m_pModelData->pos,
					m_targetPos, pModel->m_pModelData->aabb, &trace,
					pModel->m_pModelData, false);
			}
			break;
		}

		if (bCollide)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"rmr_notify", 0);
			pForm->SetMessage(
				MakeStr(
					"[\xC0\xFA\xC0\xE5\xBD\xC7\xC6\xD0]\n\xC3\xE6\xB5\xB9\xC7\xCF\xB4\xC2 \xB0\xA1\xB1\xB8\xB0\xA1 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
				false);
			pForm->Open(NULL, 3);
			return false;
		}
	}

	// The posters may not overlap each other on the wall they hang on.
	if (posterList.GetCount() > 0)
	{
		for (pModel = posterList.Start(); pModel; pModel = posterList.Next())
		{
			Waabb box;
			box.min = WVector(0.0f, 0.0f, 0.0f);
			box.max = WVector(0.0f, 0.0f, 0.0f);

			if (pModel->m_handle == 0)
			{
				WVector vMin(pModel->m_pModelData->aabb.min.x,
					pModel->m_pModelData->aabb.min.y, 0.0f);
				WVector vMax(pModel->m_pModelData->aabb.max.x,
					pModel->m_pModelData->aabb.max.y, 0.0f);

				for (cModel* pOther = m_ArrangeList.Start(); pOther;
					pOther = m_ArrangeList.Next())
				{
					if (pOther &&
						(pOther->m_pFurniture->typeId & 0x3FF800) == 0x7000 &&
						pOther->m_pFurniture->id != pModel->m_pFurniture->id &&
						pOther->m_handle == pModel->m_handle)
					{
						box.min.x = pOther->m_pModelData->aabb.min.x;
						box.min.y = pOther->m_pModelData->aabb.min.y;
						box.max.x = pOther->m_pModelData->aabb.max.x;
						box.max.y = pOther->m_pModelData->aabb.max.y;
						box.max.z = 0.0f;
						box.min.z = 0.0f;

						if (box.IsInclude(WVector(vMin.x, vMin.y, vMin.z)) ||
							box.IsInclude(WVector(vMax.x, vMin.y, vMin.z)) ||
							box.IsInclude(WVector(vMax.x, vMax.y, vMax.z)) ||
							box.IsInclude(WVector(vMin.x, vMax.y, vMax.z)))
						{
							bCollide = true;
							break;
						}
					}
				}
			}
			else if (pModel->m_handle > 0 && pModel->m_handle < 3)
			{
				WVector vMin(0.0f, pModel->m_pModelData->aabb.min.y,
					pModel->m_pModelData->aabb.min.z);
				WVector vMax(0.0f, pModel->m_pModelData->aabb.max.y,
					pModel->m_pModelData->aabb.max.z);

				for (cModel* pOther = m_ArrangeList.Start(); pOther;
					pOther = m_ArrangeList.Next())
				{
					if (pOther &&
						(pOther->m_pFurniture->typeId & 0x3FF800) == 0x7000 &&
						pOther->m_pFurniture->id != pModel->m_pFurniture->id &&
						pOther->m_handle == pModel->m_handle)
					{
						box.min.y = pOther->m_pModelData->aabb.min.y;
						box.min.z = pOther->m_pModelData->aabb.min.z;
						box.max.y = pOther->m_pModelData->aabb.max.y;
						box.max.z = pOther->m_pModelData->aabb.max.z;
						box.max.x = 0.0f;
						box.min.x = 0.0f;

						if (box.IsInclude(WVector(vMin.x, vMin.y, vMin.z)) ||
							box.IsInclude(WVector(vMin.x, vMin.y, vMax.z)) ||
							box.IsInclude(WVector(vMax.x, vMax.y, vMax.z)) ||
							box.IsInclude(WVector(vMax.x, vMax.y, vMin.z)))
						{
							bCollide = true;
							break;
						}
					}
				}
			}
		}

		if (bCollide)
		{
			FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"rmr_notify", 0);
			pForm->SetMessage(
				MakeStr(
					"[\xC0\xFA\xC0\xE5\xBD\xC7\xC6\xD0]\n\xC3\xE6\xB5\xB9\xC7\xCF\xB4\xC2 \xB0\xA1\xB1\xB8\xB0\xA1 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9."),
				false);
			pForm->Open(NULL, 3);
			return false;
		}
	}

	if (bSave)
		m_furnitureList.clear();

	std::list<sFurniture_List>::iterator it;
	if (IsLocalContent((localContentType_t)0x7d /*983: era 0x80; interpolado*/))
	{
		if (bSave)
		{
			for (it = CSharedDoc::Instance()->m_furnitureList.begin();
				it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
			{
				cModel* pSaved = GetModelData((*it).id, (*it).typeId);
				if (pSaved)
				{
					if (pSaved->m_pModelData == NULL)
					{
						(*it).x = pSaved->m_pFurniture->x;
						(*it).y = pSaved->m_pFurniture->y;
						(*it).z = pSaved->m_pFurniture->z;
					}
					else
					{
						(*it).x = pSaved->m_pModelData->pos.x;
						(*it).y = pSaved->m_pModelData->pos.y;
						(*it).z = pSaved->m_pModelData->pos.z;
					}
					(*it).r = pSaved->m_angle;
				}

				m_furnitureList.push_back(*it);
			}
		}
	}
	else if (bSave)
	{
		for (it = CSharedDoc::Instance()->m_furnitureList.begin();
			it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
			m_furnitureList.push_back(*it);
	}

	if (!IsLocalContent((localContentType_t)0x7d /*983: era 0x80; interpolado*/) || bSave)
	{
		m_pSelectModel = NULL;
		m_state = 0;
	}

	return true;
}

// realmyroomdata.cpp:1065
cModel* CRealMyRoomData::CreateModel(const char* name)
{
	m_pSelectModel = NULL;
	m_pPickedModel = NULL;

	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		return NULL;

	if (name == NULL)
		return NULL;

	WPuppet* pCenterPuppet = g_resrcmng->GetPuppet(name, false, true, false);

	pCenterPuppet->SetLightMode(0x4000000);
	w_bound_box* pBox = pCenterPuppet->m_bbList.Find("center");

	cModel* pModel = new cModel;
	pModel->m_pModelData = new WPolySoup::sModel;

	pModel->m_pModelData->type = 1;
	pModel->m_pModelData->flag = 0;

	WMatrix mat;
	mat.Reset();
	pModel->m_pModelData->mat = mat;

	pModel->m_pModelData->scale = 1.0f;

	strncpy(pModel->m_pModelData->name, name, 0x1F);

	bool bSubOrder = pModel->m_pModelData->type != 0;

	pModel->m_pModelData->pet = g_resrcmng->GetPuppet(
		pModel->m_pModelData->name, true, bSubOrder, false);

	if (pBox != NULL)
	{
		pModel->m_pModelData->aabb.min.Reset();
		pModel->m_pModelData->aabb.max.Reset();
		pModel->m_pModelData->aabb = pBox->info->aabb;

		pModel->m_pModelData->obb = pBox->bone->GetMatrix();
		pModel->m_pModelData->obb *=
			(pBox->info->aabb.max - pBox->info->aabb.min) * 0.5f;

		pModel->m_pModelData->obb.center =
			pBox->info->spherePivot * pBox->bone->GetMatrix();

		pModel->m_pModelData->pos =
			(pModel->m_pModelData->aabb.max - pModel->m_pModelData->aabb.min) *
			0.5f;

		pModel->m_pModelData->mat.pivot.x = pModel->m_pModelData->pos.x;
		pModel->m_pModelData->mat.pivot.y = pModel->m_pModelData->pos.y;
		pModel->m_pModelData->mat.pivot.z = pModel->m_pModelData->pos.z;

		pModel->m_pModelData->obb.center = pModel->m_pModelData->pos;

		pModel->m_pModelData->aabb.min += pModel->m_pModelData->pos;
		pModel->m_pModelData->aabb.max += pModel->m_pModelData->pos;
	}

	pModel->m_pModelData->pet->SetMatrix(pModel->m_pModelData->mat);
	pModel->m_pModelData->pet->UpdateBound(true, false);

	pModel->m_pModelData->pet->UpdateLightSource(&g_lightset, false, 0);

	unsigned long flags = 0;
	if (pModel->m_pModelData->type)
		flags = 4;

	if ((pModel->m_pModelData->flag & 0x10) ||
		pModel->m_pModelData->pet->GetBoundSphere().radius > 128.0f)
		flags |= 2;

	if (pModel->m_pModelData->flag & 8)
		flags |= 1;

	CSceneManager::Instance()->RegisterElement((eObjType)3,
		pModel->m_pModelData->pet, NULL, pModel->m_pModelData->name, flags,
		&pModel->m_pModelData->bbox);

	return pModel;
}

// realmyroomdata.cpp:1165
// Creates a piece of furniture at a position and an angle; type 2/3 stand
// on the floor, 5/6 hang on a wall. With bRename the scene name gets the
// first free "%02d" prefix among the room's models.
cModel* CRealMyRoomData::CreateModel(const char* name, WVector pos, float angle,
	bool bRename, short type)
{
	cModel* pModel = NULL;

	m_pSelectModel = NULL;
	m_pPickedModel = NULL;

	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
		return NULL;

	if (name == NULL)
		return NULL;

	WPuppet* pCenterPuppet = g_resrcmng->GetPuppet(name, false, true, false);

	if (pCenterPuppet == NULL)
		return NULL;

	pCenterPuppet->SetLightMode(0x4000000);
	w_bound_box* pBox = pCenterPuppet->m_bbList.Find("center");

	pModel = new cModel;
	pModel->m_pModelData = new WPolySoup::sModel;

	pModel->m_pModelData->type = 1;
	pModel->m_pModelData->flag = 0;

	WMatrix mat;
	mat.Reset();
	pModel->m_pModelData->mat = mat;

	pModel->m_pModelData->scale = 1.0f;

	strncpy(pModel->m_pModelData->name, name, 0x1F);

	bool bSubOrder = pModel->m_pModelData->type != 0;

	pModel->m_pModelData->pet = g_resrcmng->GetPuppet(
		pModel->m_pModelData->name, true, bSubOrder, false);

	if (pBox != NULL)
	{
		pModel->m_pModelData->aabb.min.Reset();
		pModel->m_pModelData->aabb.max.Reset();
		pModel->m_pModelData->aabb = pBox->info->aabb;

		pModel->m_pModelData->obb = pBox->bone->GetMatrix();
		pModel->m_pModelData->obb *=
			(pBox->info->aabb.max - pBox->info->aabb.min) * 0.5f;
		pModel->m_pModelData->obb.center =
			pBox->info->spherePivot * pBox->bone->GetMatrix();

		pModel->m_pModelData->pos = pos;

		if (type == 2 || type == 3)
			pModel->m_pModelData->pos.y = 0.0f;

		pModel->m_pModelData->obb.center = pModel->m_pModelData->pos;

		pModel->m_pModelData->mat.pivot.x = pModel->m_pModelData->pos.x;
		pModel->m_pModelData->mat.pivot.y = pModel->m_pModelData->pos.y;
		pModel->m_pModelData->mat.pivot.z = pModel->m_pModelData->pos.z;

		pModel->m_angle = 0.0f;
		pModel->SetRotate(angle);

		if (type == 2 || type == 3)
		{
			pModel->m_pModelData->obb.center.y = pBox->info->aabb.max.y * 0.5f;
		}
		else if (type == 5 || type == 6)
		{
			pModel->m_pModelData->obb.center.y = pos.y;
		}
		else
		{
			pModel->m_pModelData->obb.center.y = pBox->info->aabb.max.y * 0.5f;
		}

		pModel->m_pModelData->type = 1;

		pModel->m_pModelData->aabb.min += pModel->m_pModelData->pos;
		pModel->m_pModelData->aabb.max += pModel->m_pModelData->pos;
		pModel->OBBToAABB();
	}

	pModel->m_pModelData->pet->SetMatrix(pModel->m_pModelData->mat);
	pModel->m_pModelData->pet->UpdateBound(true, false);

	pModel->m_pModelData->pet->UpdateLightSource(&g_lightset, false, 0);

	if (bRename)
	{
		int index = 1;
		char sceneName[0x20];
		for (;;)
		{
			sprintf(sceneName, "%02d%s", index, pModel->m_pModelData->name);

			bool bUsed = false;
			for (cModel* pOther = m_ArrangeList.Start(); pOther;
				pOther = m_ArrangeList.Next())
			{
				if (pOther && pOther->m_pModelData)
				{
					if (stricmp(sceneName, pOther->m_pModelData->name) == 0)
					{
						bUsed = true;
						index++;
						break;
					}
				}
			}
			if (!bUsed)
			{
				strncpy(pModel->m_pModelData->name, sceneName, 0x20);
				break;
			}
		}
	}

	WVector wallPos;
	pModel->m_handle = GetPVS().Get_WallSlidingAngle(pModel->m_pModelData->pos,
		pModel->m_pModelData, wallPos);

	unsigned long flags = 0;
	if (pModel->m_pModelData->type)
		flags = 4;

	if ((pModel->m_pModelData->flag & 0x10) ||
		pModel->m_pModelData->pet->GetBoundSphere().radius > 128.0f)
		flags |= 2;

	if (pModel->m_pModelData->flag & 8)
		flags |= 1;

	CSceneManager::Instance()->RegisterElement((eObjType)3,
		pModel->m_pModelData->pet, NULL, pModel->m_pModelData->name, flags,
		&pModel->m_pModelData->bbox);

	return pModel;
}

// realmyroomdata.cpp:~1040
// Places a piece of furniture in the room: a model for ordinary furniture,
// or the floor (type 8) / wall (type 9) texture, which keeps a model-less
// entry in the list. Returns false when it cannot be placed.
bool CRealMyRoomData::AddModel(sFurniture_List furniture)
{
	std::list<sFurniture_List>::iterator it;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		for (it = CSharedDoc::Instance()->m_furnitureList.begin();
			it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
		{
			if ((*it).id == furniture.id)
				break;
		}
		if (it == CSharedDoc::Instance()->m_furnitureList.end())
			CSharedDoc::Instance()->AddFurniture(furniture);
	}

	IFF_STRUCT::sFurniture* pFurniture =
		ItemManager()->FindFurniture(furniture.typeId);

	if (furniture.typeId == 0 || pFurniture == NULL)
		return true;

	char name[0x100];
	sprintf(name, "%s.pet", pFurniture->Data);

	WVector pos(furniture.x, furniture.y, furniture.z);

	if (!furniture.bArrange)
	{
		for (it = CSharedDoc::Instance()->m_furnitureList.begin();
			it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
		{
			if ((*it).id == furniture.id)
				break;
		}
		if (it == CSharedDoc::Instance()->m_furnitureList.end())
			CSharedDoc::Instance()->AddFurniture(furniture);
		return true;
	}

	cModel* pModel;

	// Two pieces with the same function cannot be placed together.
	for (pModel = m_ArrangeList.Start(); pModel; pModel = m_ArrangeList.Next())
	{
		if (pModel && pModel->m_pFurniture->id)
		{
			IFF_STRUCT::sFurniture* pOther =
				ItemManager()->FindFurniture(pModel->m_pFurniture->typeId);

			if (pFurniture->IsFunction != 0 &&
				pOther->IsFunction == pFurniture->IsFunction &&
				pOther->IsOwn == 0)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				pForm->SetMessage(
					"\xC1\xDF\xBA\xB9\xB5\xC8 \xB1\xE2\xB4\xC9\xC0\xC7 \xBE\xC6\xC0\xCC\xC5\xDB\xC0\xBB \xC3\xDF\xB0\xA1\xB7\xCE \xB9\xE8\xC4\xA1\xC7\xD2 \xBC\xF6 \xBE\xF8\xBD\xC0\xB4\xCF\xB4\xD9. \xB5\xBF\xC0\xCF \xB1\xE2\xB4\xC9\xC0\xC7 \xB0\xA1\xB1\xB8\xB8\xA6 \xC1\xA6\xB0\xC5\xC7\xCF\xB0\xED \xB9\xE8\xC4\xA1\xC7\xD8 \xC1\xD6\xBC\xBC\xBF\xE4",
					false);
				pForm->Open(NULL, 3);
				return false;
			}
		}
	}

	unsigned long type = (furniture.typeId >> 11) & 0x7FF;

	if (GOLFDOC()->m_pPolySoup->FindModel(name) == NULL && type != 8 &&
		type != 9)
	{
		pModel = CreateModel(name, pos, furniture.r, false, pFurniture->IsMove);

		if (pModel == NULL)
		{
			FrForm* pForm =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
			pForm->SetMessage(
				MakeStr(
					"\xB0\xA1\xB1\xB8 \xBB\xFD\xBC\xBA\xBF\xA1 \xBD\xC7\xC6\xD0\xC7\xCF\xBF\xB4\xBD\xC0\xB4\xCF\xB4\xD9. error %d ",
					furniture.id),
				false);
			pForm->Open(NULL, 3);
			return false;
		}

		GOLFDOC()->m_pPolySoup->CreateDataModel(pModel->m_pModelData);

		for (it = CSharedDoc::Instance()->m_furnitureList.begin();
			it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
		{
			if ((*it).id == furniture.id)
				break;
		}

		if (it != CSharedDoc::Instance()->m_furnitureList.end())
		{
			pModel->m_pFurniture = &*it;
		}
		else
		{
			CSharedDoc::Instance()->AddFurniture(furniture);

			pModel->m_pFurniture->id = furniture.id;
			pModel->m_pFurniture->typeId = furniture.typeId;
			pModel->m_pFurniture->reserved = furniture.reserved;
			pModel->m_pFurniture->x = furniture.x;
			pModel->m_pFurniture->y = furniture.y;
			pModel->m_pFurniture->z = furniture.z;
			pModel->m_pFurniture->r = furniture.r;
			pModel->m_pFurniture->bArrange = furniture.bArrange;
		}

		m_ArrangeList += pModel;
		m_state = 1;

		if ((pModel->m_pFurniture->typeId & 0x3FF800) == 0x7000)
		{
			ChangePosterTexture(pModel->m_pFurniture->typeId,
				*pModel->m_pModelData);
			m_pSelectModel = pModel;
			m_state = 2;
		}

		return true;
	}

	if (GOLFDOC()->m_pPolySoup->FindModel(name) != NULL &&
		pFurniture->IsOwn == 1)
	{
		// A second copy of a model already in the scene gets a numbered name.
		pModel = CreateModel(name, pos, furniture.r, true, pFurniture->IsMove);

		if (pModel == NULL)
		{
			FrForm* pForm =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
			pForm->SetMessage(
				MakeStr(
					"\xB0\xA1\xB1\xB8 \xBB\xFD\xBC\xBA\xBF\xA1 \xBD\xC7\xC6\xD0\xC7\xCF\xBF\xB4\xBD\xC0\xB4\xCF\xB4\xD9. error %d ",
					furniture.id),
				false);
			pForm->Open(NULL, 3);
			return false;
		}

		GOLFDOC()->m_pPolySoup->CreateDataModel(pModel->m_pModelData);

		for (it = CSharedDoc::Instance()->m_furnitureList.begin();
			it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
		{
			if ((*it).id == furniture.id)
				break;
		}

		if (it != CSharedDoc::Instance()->m_furnitureList.end())
		{
			pModel->m_pFurniture = &*it;
		}
		else
		{
			CSharedDoc::Instance()->AddFurniture(furniture);

			pModel->m_pFurniture->id = furniture.id;
			pModel->m_pFurniture->typeId = furniture.typeId;
			pModel->m_pFurniture->reserved = furniture.reserved;
			pModel->m_pFurniture->x = furniture.x;
			pModel->m_pFurniture->y = furniture.y;
			pModel->m_pFurniture->z = furniture.z;
			pModel->m_pFurniture->r = furniture.r;
			pModel->m_pFurniture->bArrange = furniture.bArrange;
		}

		m_ArrangeList += pModel;

		if ((pModel->m_pFurniture->typeId & 0x3FF800) == 0x7000)
		{
			ChangePosterTexture(pModel->m_pFurniture->typeId,
				*pModel->m_pModelData);
			m_pSelectModel = pModel;
			m_state = 2;
		}

		return true;
	}

	if (type == 8)
	{
		// The floor: take the current one out of the list (clicking the same
		// one again just removes it), then apply and list the new one.
		for (pModel = m_ArrangeList.Start(); pModel;
			pModel = m_ArrangeList.Next())
		{
			if ((pModel->m_pFurniture->typeId & 0x3FF800) == 0x4000)
			{
				if (furniture.id == pModel->m_pFurniture->id)
				{
					pModel->m_pFurniture->bArrange = 0;
					pModel->m_pFurniture = NULL;
					m_ArrangeList -= pModel;
					delete pModel;
					ChangeFloorTexture(&furniture, true);
					return true;
				}

				pModel->m_pFurniture->bArrange = 1;
				m_ArrangeList -= pModel;
				break;
			}
		}

		ChangeFloorTexture(&furniture, false);

		bool bFound = false;
		cModel* pNew = new cModel;
		furniture.bArrange = 1;

		for (it = CSharedDoc::Instance()->m_furnitureList.begin();
			it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
		{
			if ((*it).typeId == furniture.typeId && (*it).id == furniture.id)
			{
				pNew->m_pFurniture = &*it;
				bFound = true;
			}
		}

		if (bFound)
		{
			m_ArrangeList += pNew;
		}
		else
		{
			ChangeFloorTexture(&furniture, true);

			FrForm* pForm =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
			pForm->SetMessage(
				MakeStr(
					"\xBA\xAE\xC1\xF6 \xB9\xD7 \xB9\xD9\xB4\xDA\xC1\xA6 \xB9\xD9\xB2\xD9\xB1\xE2 \xBD\xC7\xC6\xD0- \xB0\xA1\xC1\xF6\xB0\xED\xC0\xD6\xB4\xC2 \xB0\xA1\xB1\xB8\xBF\xA1\xBC\xAD \xC7\xF6\xC0\xE7 \xBE\xC6\xC0\xCC\xC5\xDB\xC0\xBB \xC3\xA3\xC1\xF6 \xB8\xF8\xC7\xCF\xBF\xB4\xBD\xC0\xB4\xCF\xB4\xD9.",
					furniture.id),
				false);
			pForm->Open(NULL, 3);
		}
	}
	else if (type == 9)
	{
		// The wall, the same way.
		for (pModel = m_ArrangeList.Start(); pModel;
			pModel = m_ArrangeList.Next())
		{
			if ((pModel->m_pFurniture->typeId & 0x3FF800) == 0x4800)
			{
				if (furniture.id == pModel->m_pFurniture->id)
				{
					pModel->m_pFurniture->bArrange = 0;
					pModel->m_pFurniture = NULL;
					m_ArrangeList -= pModel;
					delete pModel;
					ChangeWallTexture(&furniture, true);
					return true;
				}

				pModel->m_pFurniture->bArrange = 0;
				m_ArrangeList -= pModel;
				break;
			}
		}

		ChangeWallTexture(&furniture, false);

		bool bFound = false;
		cModel* pNew = new cModel;
		furniture.bArrange = 1;

		for (it = CSharedDoc::Instance()->m_furnitureList.begin();
			it != CSharedDoc::Instance()->m_furnitureList.end(); it++)
		{
			if ((*it).typeId == furniture.typeId && (*it).id == furniture.id)
			{
				pNew->m_pFurniture = &*it;
				bFound = true;
			}
		}

		if (bFound)
		{
			m_ArrangeList += pNew;
		}
		else
		{
			ChangeWallTexture(&furniture, true);

			FrForm* pForm =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
			pForm->SetMessage(
				MakeStr(
					"\xBA\xAE\xC1\xF6 \xB9\xD7 \xB9\xD9\xB4\xDA\xC1\xA6 \xB9\xD9\xB2\xD9\xB1\xE2 \xBD\xC7\xC6\xD0- \xB0\xA1\xC1\xF6\xB0\xED\xC0\xD6\xB4\xC2 \xB0\xA1\xB1\xB8\xBF\xA1\xBC\xAD \xC7\xF6\xC0\xE7 \xBE\xC6\xC0\xCC\xC5\xDB\xC0\xBB \xC3\xA3\xC1\xF6 \xB8\xF8\xC7\xCF\xBF\xB4\xBD\xC0\xB4\xCF\xB4\xD9.",
					furniture.id),
				false);
			pForm->Open(NULL, 3);
		}
	}

	return true;
}

// realmyroomdata.cpp:1324
void CRealMyRoomData::ResotreList()
{
	ResetList(false);

	for (std::list<sFurniture_List>::iterator it = m_furnitureList.begin();
		it != m_furnitureList.end(); ++it)
	{
		sFurniture_List furniture = *it;

		if (furniture.bArrange)
		{
			std::list<sFurniture_List>& docList =
				CSharedDoc::Instance()->m_furnitureList;
			for (std::list<sFurniture_List>::iterator docIt = docList.begin();
				docIt != docList.end(); ++docIt)
			{
				if ((*docIt).typeId == (*it).typeId && (*docIt).id == (*it).id)
				{
					(*docIt).bArrange = 1;
					break;
				}
			}

			AddModel(furniture);
		}
	}
}

// realmyroomdata.cpp:1360
void CRealMyRoomData::DeleteList()
{
	if (m_pSelectModel != NULL)
	{
		if (m_pPickedModel)
			m_pPickedModel = NULL;

		if (!GOLFDOC()->m_pPolySoup->DeleteDataStructure(
				m_pSelectModel->m_pModelData))
		{
			FrForm* pForm =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
			pForm->SetMessage(
				"\xBB\xE8\xC1\xA6\xC7\xD2 \xB0\xA1\xB1\xB8\xB8\xA6 \xC3\xA3\xC1\xF6 \xB8\xF8\xC7\xDF\xBD\xC0\xB4\xCF\xB4\xD9.",
				false);
			pForm->Open(NULL, 3);
			return;
		}

		CSceneManager::Instance()->DeleteElement(
			m_pSelectModel->m_pModelData->name);

		m_pSelectModel->m_pModelData->pet = NULL;
		m_ArrangeList -= m_pSelectModel;
		delete m_pSelectModel->m_pModelData;
		if (m_pSelectModel)
		{
			delete m_pSelectModel;
			m_pSelectModel = NULL;
		}
	}
}

// realmyroomdata.cpp:1426
void CRealMyRoomData::ResetList(bool all)
{
	cModel* pModel;

	for (pModel = m_ArrangeList.Start(); pModel; pModel = m_ArrangeList.Next())
	{
		if (pModel && pModel->m_pModelData)
		{
			CSceneManager::Instance()->DeleteElement(
				pModel->m_pModelData->name);

			if (!GOLFDOC()->m_pPolySoup->DeleteDataStructure(
					pModel->m_pModelData))
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				pForm->SetMessage(
					"\xBB\xE8\xC1\xA6\xC7\xD2 \xB0\xA1\xB1\xB8\xB8\xA6 \xC3\xA3\xC1\xF6 \xB8\xF8\xC7\xDF\xBD\xC0\xB4\xCF\xB4\xD9.",
					false);
				pForm->Open(NULL, 3);
				return;
			}
		}
	}

	ChangeWallTexture(NULL, true);
	ChangeFloorTexture(NULL, true);

	if (all)
	{
		for (pModel = m_ArrangeList.Start(); pModel;
			pModel = m_ArrangeList.Next())
		{
			m_ArrangeList -= pModel;

			if (pModel->m_pModelData)
			{
				pModel->m_pModelData->pet = NULL;
				if (pModel->m_pModelData)
				{
					delete pModel->m_pModelData;
					pModel->m_pModelData = NULL;
				}
			}
			else if (pModel->m_pFurniture)
			{
				pModel->m_pFurniture = NULL;
			}

			delete pModel;
		}
	}
	else
	{
		for (pModel = m_ArrangeList.Start(); pModel;
			pModel = m_ArrangeList.Next())
		{
			m_ArrangeList -= pModel;

			if (pModel->m_pModelData)
			{
				pModel->m_pModelData->pet = NULL;
				if (pModel->m_pModelData)
				{
					delete pModel->m_pModelData;
					pModel->m_pModelData = NULL;
				}
			}

			delete pModel;
		}
	}

	for (pModel = m_ArrangeList.Start(); pModel; pModel = m_ArrangeList.Next())
	{
		if (pModel)
			delete pModel;
	}
	m_ArrangeList.Reset();

	m_ArrangeList.Reset();
	m_pSelectModel = NULL;
	m_pPickedModel = NULL;
}

// realmyroomdata.cpp:1511
cModel* CRealMyRoomData::ModelByRayIntersect(WVector const& origin,
	WVector const& dir)
{
	if (0)
		((CRealMyRoomData*)0)->CRealMyRoomData::CreateModel(*(const char**)0);
	for (cModel* pModel = m_ArrangeList.Start(); pModel;
		pModel = m_ArrangeList.Next())
	{
		if (pModel && pModel->m_pModelData)
		{
			if (RayIntersectAABB(pModel->m_pModelData->aabb, origin, dir, NULL,
					0) < 1.0f)
				return pModel;
		}
	}

	return NULL;
}

// realmyroomdata.cpp:1528
cModel* CRealMyRoomData::ModelByRayIntersectOBB(WVector const& pos)
{
	if (m_state != 1)
		return NULL;

	for (cModel* pModel = m_ArrangeList.Start(); pModel;
		pModel = m_ArrangeList.Next())
	{
		if (pModel && pModel->m_pModelData)
		{
			if (pModel->m_pModelData->obb.IsInclude(pos))
				return pModel;
		}
	}

	return NULL;
}

// realmyroomdata.cpp:1547
void CRealMyRoomData::ChangeFloorTexture(sFurniture_List* pList, bool bReset)
{
	static int s_floorTextureIndex = 2;
	char newTex[256];
	char oldTex[256];

	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (s_floorTextureIndex > 6)
			s_floorTextureIndex = 2;

		sprintf(newTex, "my_room_01_%02d.dds", s_floorTextureIndex);
		sprintf(oldTex, "floor_a.dds");

		GOLFDOC()->m_pPolySoup->ChangeFloorTexture(oldTex, newTex);
	}
	else
	{
		IFF_STRUCT::sFurniture* pFurniture;
		if (bReset)
			pFurniture = ItemManager()->FindFurniture(0x48004008);
		else
			pFurniture = ItemManager()->FindFurniture(pList->typeId);

		if (pFurniture)
		{
			for (int i = 0; i < 3; i++)
			{
				if (stricmp(pFurniture->Tex[i], "") == 0 ||
					stricmp(pFurniture->Tex[i], "") == 0)
					break;

				sprintf(newTex, "%s.dds", pFurniture->Tex[i]);
				sprintf(oldTex, "%s.dds", pFurniture->OrgTex[i]);

				GOLFDOC()->m_pPolySoup->ChangeFloorTexture(oldTex, newTex);
			}
		}
	}

	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		s_floorTextureIndex++;
}

// realmyroomdata.cpp:1609
void CRealMyRoomData::ChangeWallTexture(sFurniture_List* pList, bool bReset)
{
	static int s_wallTextureIndex = 2;
	char newTex[256];
	char oldTex[256];

	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (s_wallTextureIndex > 6)
			s_wallTextureIndex = 2;

		sprintf(newTex, "my_room_01_%02d.dds", s_wallTextureIndex);
		sprintf(oldTex, "+wall_a.dds");
	}
	else
	{
		IFF_STRUCT::sFurniture* pFurniture;
		if (bReset)
			pFurniture = ItemManager()->FindFurniture(0x4800480E);
		else
			pFurniture = ItemManager()->FindFurniture(pList->typeId);

		if (pFurniture)
		{
			for (int i = 0; i < 3; i++)
			{
				if (stricmp(pFurniture->Tex[i], "") == 0 ||
					stricmp(pFurniture->Tex[i], "") == 0)
					break;

				sprintf(newTex, "%s.dds", pFurniture->Tex[i]);
				sprintf(oldTex, "%s.dds", pFurniture->OrgTex[i]);

				GOLFDOC()->m_pPolySoup->ChangeObjectTexture(oldTex, newTex, -1);
			}
		}
	}

	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		s_wallTextureIndex++;
}

// realmyroomdata.cpp:1659
bool CRealMyRoomData::ChangePosterTexture(unsigned long typeId,
	WPolySoup::sModel& model)
{
	char newTex[256];
	char oldTex[256];

	IFF_STRUCT::sFurniture* pFurniture = ItemManager()->FindFurniture(typeId);
	if (pFurniture)
	{
		for (int i = 0; i < 3; i++)
		{
			if (stricmp(pFurniture->Tex[i], "") == 0 ||
				stricmp(pFurniture->Tex[i], "") == 0)
				break;

			if ((typeId & 0x3FF800) == 0x7000)
			{
				sprintf(newTex, "%s.jpg", pFurniture->Tex[i]);
				sprintf(oldTex, "%s.jpg", pFurniture->OrgTex[i]);
			}
			else
			{
				sprintf(newTex, "%s.dds", pFurniture->Tex[i]);
				sprintf(oldTex, "%s.dds", pFurniture->OrgTex[i]);
			}

			GOLFDOC()->m_pPolySoup->ChangeObjectTexture(oldTex, newTex,
				model.id);
		}
	}

	return true;
}

// realmyroomdata.cpp:1695
void CRealMyRoomData::SetRotate(float angle)
{
	if (m_state != 3 && m_state != 7)
		return;

	if (angle == 0.0f)
		return;

	if (m_pSelectModel == NULL)
		return;

	for (cModel* pModel = m_ArrangeList.Start(); pModel;
		pModel = m_ArrangeList.Next())
	{
		if (pModel->m_pModelData == NULL ||
			stricmp(pModel->m_pModelData->name,
				m_pSelectModel->m_pModelData->name) != 0)
			continue;

		Wobb obb;
		obb = pModel->m_pModelData->obb;
		WMatrix mat;
		mat = pModel->m_pModelData->mat;

		pModel->SetRotate(angle * 0.03f);
		pModel->OBBToAABB();

		WVector pos;
		pos = ChangeModelPosition(7, pModel, pModel->m_pModelData);
		WPolySoup::sModel* pData = pModel->m_pModelData;
		if (!WisEqual(pos, pData->pos, g_EPSILON))
		{
			pData->obb = obb;
			pModel->m_pModelData->mat = mat;
			pModel->OBBToAABB();
			return;
		}

		pModel->Apply_ChangeModelData();
		m_pSelectModel = pModel;
		return;
	}
}

// realmyroomdata.cpp:1741
void CRealMyRoomData::SetTargetPosition(WVector const& pos, unsigned char mode)
{
	if (m_state == 2)
	{
		switch (mode)
		{
		case 1:
			m_pickPos = m_pSelectModel->m_pModelData->pos;
			m_targetPos = pos;
			break;
		case 2:
			m_targetPos = pos;
			m_dragPos = pos;
			MoveModelPosition(mode);
			break;
		case 3:
			m_bDropped = false;
			m_targetPos = pos;
			MoveModelPosition(mode);
			break;
		}
	}
}

// realmyroomdata.cpp:1777
// Removes from a velocity its part along a plane's normal (scaled by
// overbounce), the Quake way of sliding along a wall.
static void ClipVelocity(const WVector& in, const WVector& normal, WVector& out,
	float overbounce)
{
	float backoff = (in * normal) * overbounce;
	out = in - normal * backoff;
}

// realmyroomdata.cpp:~1800
// Moves the selected model toward the target position the way its kind
// allows (IsMove 1-3: on the floor, 5: along the walls), keeping it out of
// the walls and the other furniture. When a drag (mode 2) ends on a spot
// where it collides, the model goes back to its saved place.
void CRealMyRoomData::MoveModelPosition(unsigned char mode)
{
	WVector delta(0.0f, 0.0f, 0.0f);
	IFF_STRUCT::sFurniture* pFurniture;
	trace_s trace;
	bool bApply = false;

	if (m_pSelectModel)
		pFurniture =
			ItemManager()->FindFurniture(m_pSelectModel->m_pFurniture->typeId);

	switch (pFurniture->IsMove)
	{
	case 1:
	case 2:
	case 3:
	{
		GetPVS().TraceOBB_Avatar_genPosition(m_pSelectModel->m_pModelData->pos,
			m_targetPos, &trace, m_pSelectModel->m_pModelData);

		if (trace.m_endPos.x == m_pSelectModel->m_pModelData->pos.x &&
			trace.m_endPos.z == m_pSelectModel->m_pModelData->pos.z)
			m_targetPos = m_pSelectModel->m_pModelData->pos;
		else
			m_targetPos = trace.m_endPos;

		GetPVS().TraceOBB_WallSliding(m_pSelectModel->m_pModelData->pos,
			m_targetPos, &trace, m_pSelectModel->m_pModelData, 0, false);

		delta = trace.m_endPos - m_pSelectModel->m_pModelData->pos;
		WVector move = delta;
		WVector newPos = delta + m_pSelectModel->m_pModelData->pos;

		m_bDropped = GetPVS().TraceOBB_Object(m_pSelectModel->m_pModelData->pos,
			newPos, m_pSelectModel->m_pModelData->aabb, &trace,
			m_pSelectModel->m_pModelData, false);

		WVector objDelta = trace.m_endPos - m_pSelectModel->m_pModelData->pos;
		m_targetPos = move + m_pSelectModel->m_pModelData->pos;

		if (mode == 2 && !(move.x == 0.0f && move.z == 0.0f))
			move = objDelta;
		else
			move = delta;

		if (pFurniture->IsMove == 2 || pFurniture->IsMove == 3)
			move.y = 0.0f;

		m_pSelectModel->m_pModelData->pos += move;
		m_pSelectModel->m_pModelData->obb.center += move;

		m_pSelectModel->m_pModelData->mat.pivot.x =
			m_pSelectModel->m_pModelData->pos.x;
		m_pSelectModel->m_pModelData->mat.pivot.y =
			m_pSelectModel->m_pModelData->pos.y;
		m_pSelectModel->m_pModelData->mat.pivot.z =
			m_pSelectModel->m_pModelData->pos.z;

		m_pSelectModel->m_pModelData->aabb.min += move;
		m_pSelectModel->m_pModelData->aabb.max += move;

		bApply = true;
		break;
	}
	case 5:
	{
		WVector move(0.0f, 0.0f, 0.0f);

		if (!GetPVS().CheckPickRoom(m_targetPos))
			break;

		WVector wallPos;
		int wall = GetPVS().Get_WallSlidingAngle(m_targetPos,
			m_pSelectModel->m_pModelData, wallPos);

		cModel* pSelect = m_pSelectModel;
		move = m_targetPos - pSelect->m_pModelData->pos;

		if (wall < 0)
			break;

		// Moved over to another wall: turn the model to face out of it.
		if (wall != pSelect->m_handle && pSelect->m_handle >= 0)
		{
			pSelect->m_handle = wall;
			delta = WVector(0.0f, 0.0f, 0.0f);

			switch (wall)
			{
			case 0:
				if (m_pSelectModel->m_angle != 0.0f)
				{
					if (m_pSelectModel->m_angle == 360.0f)
						m_pSelectModel->SetRotate(-360.0f);
					else
						m_pSelectModel->SetRotate(
							360.0f - m_pSelectModel->m_angle);
				}
				break;
			case 1:
				if (m_pSelectModel->m_angle == 0.0f ||
					m_pSelectModel->m_angle == 360.0f)
					m_pSelectModel->SetRotate(270.0f);
				break;
			case 2:
				if (m_pSelectModel->m_angle == 0.0f ||
					m_pSelectModel->m_angle == 360.0f)
					m_pSelectModel->SetRotate(90.0f);
				break;
			}

			delta = m_targetPos - m_pSelectModel->m_pModelData->pos;

			m_pSelectModel->m_pModelData->pos += delta;
			m_pSelectModel->m_pModelData->obb.center += delta;
			m_pSelectModel->SetPosition(m_pSelectModel->m_pModelData->pos);
			m_pSelectModel->OBBToAABB();

			m_pSelectModel->m_pModelData->pet->SetMatrix(m_pSelectModel->m_pModelData->mat);
			m_pSelectModel->m_pModelData->pet->UpdateBound(true, false);
			m_pSelectModel->Apply_ChangeModelData();
		}

		GetPVS().TraceOBB_WallSliding(m_pSelectModel->m_pModelData->pos,
			m_targetPos, &trace, m_pSelectModel->m_pModelData,
			m_pSelectModel->m_handle + 1, false);

		move = trace.m_endPos - m_pSelectModel->m_pModelData->pos;

		m_pSelectModel->m_pModelData->pos += move;
		m_pSelectModel->m_pModelData->obb.center += move;
		m_pSelectModel->SetPosition(m_pSelectModel->m_pModelData->pos);
		m_pSelectModel->OBBToAABB();

		m_pSelectModel->m_pModelData->pet->SetMatrix(m_pSelectModel->m_pModelData->mat);
		m_pSelectModel->m_pModelData->pet->UpdateBound(true, false);
		m_pSelectModel->Apply_ChangeModelData();

		if (((m_pSelectModel->m_pFurniture->typeId >> 11) & 0x7FF) == 0xE)
		{
			// A poster only collides with the other posters, flat on the wall
			// it hangs on: test the corners of its box against theirs.
			if (mode == 2)
			{
				int handle = m_pSelectModel->m_handle;
				Waabb otherBox;
				Waabb box;

				otherBox.min.Reset();
				otherBox.max.Reset();
				box.min.Reset();
				box.max.Reset();

				if (handle == 0)
				{
					box = m_pSelectModel->m_pModelData->aabb;
					box.max.z = 0.0f;
					box.min.z = 0.0f;

					for (cModel* pModel = m_ArrangeList.Start(); pModel;
						pModel = m_ArrangeList.Next())
					{
						if ((pModel->m_pFurniture->typeId & 0x3FF800) != 0x7000)
							continue;
						if (stricmp(pModel->m_pModelData->name,
								m_pSelectModel->m_pModelData->name) == 0)
							continue;

						otherBox = pModel->m_pModelData->aabb;
						otherBox.max.z = 0.0f;
						otherBox.min.z = 0.0f;

						if (otherBox.IsInclude(
								WVector(box.min.x, box.min.y, box.min.z)) ||
							otherBox.IsInclude(
								WVector(box.max.x, box.min.y, box.min.z)) ||
							otherBox.IsInclude(
								WVector(box.max.x, box.max.y, box.max.z)) ||
							otherBox.IsInclude(
								WVector(box.min.x, box.max.y, box.max.z)))
						{
							m_bDropped = true;
							break;
						}
					}
				}
				else if (handle > 0 && handle < 3)
				{
					box = m_pSelectModel->m_pModelData->aabb;
					box.max.x = 0.0f;
					box.min.x = 0.0f;

					for (cModel* pModel = m_ArrangeList.Start(); pModel;
						pModel = m_ArrangeList.Next())
					{
						if ((pModel->m_pFurniture->typeId & 0x3FF800) != 0x7000)
							continue;
						if (stricmp(pModel->m_pModelData->name,
								m_pSelectModel->m_pModelData->name) == 0)
							continue;

						otherBox = pModel->m_pModelData->aabb;
						otherBox.max.x = 0.0f;
						otherBox.min.x = 0.0f;

						if (otherBox.IsInclude(
								WVector(box.min.x, box.min.y, box.min.z)) ||
							otherBox.IsInclude(
								WVector(box.min.x, box.min.y, box.max.z)) ||
							otherBox.IsInclude(
								WVector(box.max.x, box.max.y, box.max.z)) ||
							otherBox.IsInclude(
								WVector(box.max.x, box.max.y, box.min.z)))
						{
							m_bDropped = true;
							break;
						}
					}
				}
			}
			break;
		}

		GetPVS().TraceOBB_Object(m_pSelectModel->m_pModelData->pos, m_targetPos,
			m_pSelectModel->m_pModelData->aabb, &trace,
			m_pSelectModel->m_pModelData, false);

		WVector objDelta = trace.m_endPos - m_pSelectModel->m_pModelData->pos;

		if (objDelta.x == 0.0f && objDelta.y == 0.0f &&
			(m_pSelectModel->m_pModelData->pos.x != m_targetPos.x ||
				m_pSelectModel->m_pModelData->pos.y != m_targetPos.y))
			m_bDropped = true;
		else
			m_bDropped = false;

		if (mode == 2 && !(move.x == 0.0f && move.y == 0.0f))
		{
			move.x = objDelta.x;
			move.y = objDelta.y;
		}
		move.z = 0.0f;
		delta = objDelta;

		m_pSelectModel->m_pModelData->pos += move;
		m_pSelectModel->m_pModelData->obb.center += move;
		m_pSelectModel->SetPosition(m_pSelectModel->m_pModelData->pos);
		m_pSelectModel->m_pModelData->aabb.min += move;
		m_pSelectModel->m_pModelData->aabb.max += move;

		bApply = true;
		break;
	}
	}

	if (bApply)
	{
		m_pSelectModel->m_pModelData->pet->SetMatrix(m_pSelectModel->m_pModelData->mat);
		m_pSelectModel->m_pModelData->pet->UpdateBound(true, false);
		m_pSelectModel->Apply_ChangeModelData();
	}

	if (mode != 2)
		return;

	if (m_bDropped)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
		pForm->SetMessage(
			"\xB0\xA1\xB1\xB8\xB0\xA1 \xC3\xE6\xB5\xB9\xB5\xC7\xB4\xC2 \xC0\xA7\xC4\xA1\xBF\xA1 \xB3\xF5\xBF\xA9\xBC\xAD \xB4\xD9\xBD\xC3 \xB9\xE8\xC4\xA1\xC7\xCF\xBC\xC5\xBE\xDF \xC7\xD5\xB4\xCF\xB4\xD9.",
			false);
		pForm->Open(NULL, 3);

		// Back to where the saved list has it.
		WVector pos(0.0f, 0.0f, 0.0f);

		for (std::list<sFurniture_List>::iterator it = m_furnitureList.begin();
			it != m_furnitureList.end(); ++it)
		{
			sFurniture_List furniture = *it;

			if (m_pSelectModel->m_pFurniture->id != furniture.id)
				continue;

			pos = WVector(furniture.x, furniture.y, furniture.z);
			if (pFurniture->IsMove == 2)
				pos.y = 0.0f;

			m_pSelectModel->SetRotate(furniture.r - m_pSelectModel->m_angle);

			pos = pos - m_pSelectModel->m_pModelData->pos;

			m_pSelectModel->m_pModelData->pos += pos;
			m_pSelectModel->m_pModelData->obb.center += pos;

			m_pSelectModel->m_pModelData->mat.pivot.x =
				m_pSelectModel->m_pModelData->pos.x;
			m_pSelectModel->m_pModelData->mat.pivot.y =
				m_pSelectModel->m_pModelData->pos.y;
			m_pSelectModel->m_pModelData->mat.pivot.z =
				m_pSelectModel->m_pModelData->pos.z;

			m_pSelectModel->OBBToAABB();

			m_pSelectModel->m_pModelData->pet->SetMatrix(m_pSelectModel->m_pModelData->mat);
			m_pSelectModel->m_pModelData->pet->UpdateBound(true, false);
			m_pSelectModel->Apply_ChangeModelData();

			m_pSelectModel->m_handle =
				GetPVS().Get_WallSlidingAngle(m_pSelectModel->m_pModelData->pos,
					m_pSelectModel->m_pModelData, pos);
			break;
		}
	}

	if (m_pSelectModel &&
		(m_pSelectModel->m_pFurniture->typeId & 0x3FF800) == 0x7000)
		CSceneManager::Instance()->SetAlpha(m_pSelectModel->m_pModelData->name,
			0xFF);
}

// realmyroomdata.cpp:2235
// The step the edited model takes toward the target position for an edit
// mode: along the floor (2), along the wall it hangs on (3), sliding along
// the walls (5), or pushed out of the other furniture (7).
WVector CRealMyRoomData::ChangeModelPosition(int mode, cModel* pModel,
	WPolySoup::sModel* pData)
{
	WVector result = WVector::ZERO;

	WVector roomMax = GetPVS().m_pHolder->aabb.max;

	switch (mode)
	{
	case 2:
	{
		WVector move = m_targetPos - pModel->m_pModelData->pos;

		WVector dir(move.x, 0.0f, move.y);

		move = dir;

		move.Normalize();

		move = move * 17.0f * 0.04f;

		if (fabs(dir.x) < fabs(move.x))
			move.x = dir.x;
		if (fabs(dir.z) < fabs(move.z))
			move.z = dir.z;

		WVector newPos = move + pModel->m_pModelData->pos;

		trace_s trace;
		if (!GetPVS().TraceAABB_BackGround(pModel->m_pModelData->pos, newPos,
				pModel->m_pModelData->aabb, &trace, pData))
			break;

		GetPVS().TraceOBB_Object(pModel->m_pModelData->pos, newPos,
			pModel->m_pModelData->aabb, &trace, pData, false);

		result = trace.m_endPos - pModel->m_pModelData->pos;
		result.y = 0.0f;
		break;
	}
	case 3:
	{
		WVector move = m_targetPos - pModel->m_pModelData->pos;

		move.y = 0.0f;
		move.z = (roomMax.z - 0.5f) -
			fabs(pModel->m_pModelData->aabb.max.z -
				pModel->m_pModelData->aabb.min.z);

		WVector dir = move;

		move.Normalize();

		move = move * 17.0f * 0.04f;

		if (fabs(dir.x) < fabs(move.x))
			move.x = dir.x;
		if (fabs(dir.y) < fabs(move.y))
			move.y = dir.y;

		WVector newPos = move + pModel->m_pModelData->pos;

		trace_s trace;
		if (!GetPVS().TraceAABB_BackGround(pModel->m_pModelData->pos, newPos,
				pModel->m_pModelData->aabb, &trace, pData))
			break;

		GetPVS().TraceOBB_Object(pModel->m_pModelData->pos, newPos,
			pModel->m_pModelData->aabb, &trace, pData, false);

		result = trace.m_endPos - pModel->m_pModelData->pos;
		result.y = 0.0f;
		result.z = 0.0f;
		break;
	}
	case 5:
		result = SlideWallPosition(pModel, pData);
		break;
	case 7:
	{
		trace_s trace;
		GetPVS().TraceOBB_Object(pModel->m_pModelData->pos,
			pModel->m_pModelData->pos, pModel->m_pModelData->aabb, &trace,
			pData, true);

		WVector push = trace.m_endPos - pModel->m_pModelData->pos;

		if (Wabs(push.x) < g_EPSILON && WisZero(push.z, g_EPSILON))
		{
			result = WVector::ZERO;
			break;
		}

		result = pModel->m_pModelData->pos;
		break;
	}
	}

	return result;
}

// realmyroomdata.cpp:2411
// Moves the model a step toward the target position along the wall it
// faces, stopping at the walls and the other furniture.
WVector CRealMyRoomData::SlideWallPosition(cModel* pModel,
	WPolySoup::sModel* pData)
{
	WVector zero = WVector::ZERO;

	WVector roomMax = GetPVS().m_pHolder->aabb.max;

	WMatrix rot;
	pModel->m_pModelData->mat.GetRotMatrix(&rot);

	float angle = acos(rot.xx) * g_RADTODEG;

	WVector dir = m_targetPos - pModel->m_pModelData->pos;
	WVector move = m_targetPos.Normalize() - pModel->m_pModelData->pos;

	if (angle == 90.0f)
	{
		move.x = (roomMax.x - 0.5f) -
			fabs(pModel->m_pModelData->aabb.max.x -
				pModel->m_pModelData->aabb.min.x);
	}
	else if (angle == 0.0f)
	{
		move.z = (roomMax.z - 0.5f) -
			fabs(pModel->m_pModelData->aabb.max.z -
				pModel->m_pModelData->aabb.min.z);
	}
	else if (angle == 270.0f || angle == -90.0f)
	{
		move.x = (roomMax.x - 0.5f) -
			fabs(pModel->m_pModelData->aabb.max.x -
				pModel->m_pModelData->aabb.min.x);
	}

	move.Normalize();

	move = move * 17.0f * 0.04f;

	if (fabs(dir.x) < fabs(move.x))
		move.x = dir.x;
	if (fabs(dir.y) < fabs(move.y))
		move.y = dir.y;

	WVector newPos = move + pModel->m_pModelData->pos;

	trace_s trace;
	if (!GetPVS().TraceAABB_BackGround(pModel->m_pModelData->pos, newPos,
			pModel->m_pModelData->aabb, &trace, pData))
		return zero;

	GetPVS().TraceOBB_Object(pModel->m_pModelData->pos, newPos,
		pModel->m_pModelData->aabb, &trace, pData, false);

	dir = trace.m_endPos - pModel->m_pModelData->pos;
	if (angle == 90.0f)
	{
		dir.x = 0.0f;
	}
	else if (angle == 0.0f)
	{
		dir.z = 0.0f;
	}
	else if (angle == 270.0f || angle == -90.0f)
	{
		dir.x = 0.0f;
	}

	return dir;
}

// realmyroomdata.cpp:2511
// Moves the selected model a step toward the target position against the
// room's back wall, sliding along what it hits.
// Pending header members (quadtree.h, classdefine.h are not this unit's):
// trace_s's hit plane normal at 0x10 and sFurniture's placement kind (a
// short) at 0xBC are read through offsets until they are transcribed.
void CRealMyRoomData::SetModelPosition()
{
	WVector delta;
	delta.Reset();

	WVector roomMax = GetPVS().m_pHolder->aabb.max;

	for (cModel* pModel = m_ArrangeList.Start(); pModel;
		pModel = m_ArrangeList.Next())
	{
		if (m_pSelectModel &&
			strcmp(pModel->m_pModelData->name,
				m_pSelectModel->m_pModelData->name) == 0)
		{
			IFF_STRUCT::sFurniture* pFurniture =
				ItemManager()->FindFurniture(pModel->m_pFurniture->typeId);

			WVector dir;
			dir = m_targetPos - pModel->m_pModelData->pos;
			dir.z = (roomMax.z - 0.5f) -
				fabs(pModel->m_pModelData->aabb.max.z -
					pModel->m_pModelData->aabb.min.z);
			WVector to = dir;

			dir.Normalize();

			WVector move;
			move = dir * 17.0f * 0.04f;
			if (fabs(to.x) < fabs(move.x))
				move.x = to.x;
			if (fabs(to.y) < fabs(move.y))
				move.y = to.y;

			WVector newPos;
			newPos = move + pModel->m_pModelData->pos;
			WVector slide;
			slide.Reset();

			trace_s trace;
			if (GetPVS().TraceAABB_BackGround(pModel->m_pModelData->pos, newPos,
					pModel->m_pModelData->aabb, &trace, pModel->m_pModelData))
			{
				GetPVS().TraceOBB_Object(pModel->m_pModelData->pos, newPos,
					pModel->m_pModelData->aabb, &trace, pModel->m_pModelData, false);

				const WVector& normal = trace.m_normal;
				delta = trace.m_endPos - pModel->m_pModelData->pos;

				if (pFurniture->IsMove == 5)
				{
					delta.z = 0.0f;
					if (delta.x == 0.0f && delta.y == 0.0f)
					{
						if ((normal * move) < 0.99f)
						{
							ClipVelocity(move, normal, slide, 1.001f);

							newPos = pModel->m_pModelData->pos;

							if (slide.x != 0.0f)
								newPos.x += slide.x;
							else
								newPos.y += slide.y;
						}
					}

					pModel->m_pModelData->pos.x += delta.x;
					pModel->m_pModelData->pos.y += delta.y;
				}
				else
				{
					delta.y = 0.0f;
					if (delta.x == 0.0f && delta.z == 0.0f)
					{
						if ((normal * move) < 0.99f)
						{
							ClipVelocity(move, normal, slide, 1.001f);

							newPos = pModel->m_pModelData->pos;

							if (slide.x != 0.0f)
								newPos.x += slide.x;
							else
								newPos.z += slide.z;
						}
					}

					pModel->m_pModelData->pos.x += delta.x;
					pModel->m_pModelData->pos.z += delta.z;
				}
			}

			pModel->m_pModelData->pos += delta;

			pModel->m_pModelData->aabb.min += delta;
			pModel->m_pModelData->aabb.max += delta;
			pModel->m_pModelData->obb.center += delta;

			pModel->m_pModelData->mat.pivot.x = pModel->m_pModelData->pos.x;
			pModel->m_pModelData->mat.pivot.y = pModel->m_pModelData->pos.y;
			pModel->m_pModelData->mat.pivot.z = pModel->m_pModelData->pos.z;

			pModel->Apply_ChangeModelData();
			return;
		}
	}
}

// realmyroomdata.cpp:2715
unsigned long CRealMyRoomData::GetState()
{
	return m_state;
}

// realmyroomdata.cpp:2719
void CRealMyRoomData::SetState(unsigned long state)
{
	switch (state)
	{
	case 6:
		DeleteList();
		break;
	}

	m_state = state;
}

// realmyroomdata.cpp:2748
void CRealMyRoomData::ListFurnitureSelect(sFurniture_List* pList)
{
	if (0)
		((CRealMyRoomData*)0)->CRealMyRoomData::SetModelPosition();
	for (cModel* pModel = m_ArrangeList.Start(); pModel;
		pModel = m_ArrangeList.Next())
	{
		if (pModel && pModel->m_pModelData)
		{
			if (pModel->m_pFurniture->id == pList->id)
				m_pSelectModel = pModel;
		}
	}
}

// realmyroomdata.cpp:2771
bool CRealMyRoomData::IsArragementFurniture(unsigned long id)
{
	bool bArranged = false;
	std::list<sFurniture_List>::iterator it;

	for (it = m_furnitureList.begin(); it != m_furnitureList.end(); ++it)
	{
		if (id == (*it).id)
		{
			if ((*it).bArrange)
				bArranged = true;
			break;
		}
	}

	return bArranged;
}

// realmyroomdata.cpp:2795
cModel* CRealMyRoomData::GetModelData(unsigned long id, unsigned long typeId)
{
	for (cModel* pModel = m_ArrangeList.Start(); pModel;
		pModel = m_ArrangeList.Next())
	{
		if (pModel)
		{
			if (pModel->m_pFurniture->typeId == typeId &&
				pModel->m_pFurniture->id == id)
				return pModel;
		}
	}

	return NULL;
}

// realmyroomdata.cpp:2821
bool CRealMyRoomData::IsModelData_SaveList(unsigned long id,
	unsigned long typeId)
{
	bool bFound = false;
	std::list<sFurniture_List>::iterator it;

	for (it = m_furnitureList.begin(); it != m_furnitureList.end(); ++it)
	{
		if (id == (*it).id && typeId == (*it).typeId && (*it).bArrange)
		{
			bFound = true;
			break;
		}
	}

	return bFound;
}

// realmyroomdata.cpp:2846
int CRealMyRoomData::IsFunritureValueChange(cModel& model)
{
	std::list<sFurniture_List>::iterator it;

	for (it = m_furnitureList.begin(); it != m_furnitureList.end(); ++it)
	{
		if (model.m_pFurniture->id == (*it).id)
		{
			int result = 0;

			if (model.m_pModelData)
			{
				if ((*it).x != model.m_pModelData->pos.x)
					result = 1;

				if ((*it).y != model.m_pModelData->pos.y)
					result = 1;

				if ((*it).z != model.m_pModelData->pos.z)
					result = 1;

				if ((*it).r != model.m_angle)
					result = 1;

				if (model.m_pFurniture->bArrange != (*it).bArrange)
					return 1;

				return result;
			}

			if (model.m_pFurniture->bArrange != (*it).bArrange)
				return 2;

			return result;
		}
	}

	return -1;
}

// realmyroomdata.cpp:2921
cModel::cModel()
{
	m_state = 1;
	m_pModelData = NULL;
	m_pFurniture = NULL;

	m_angle = 0.0f;
	m_handle = -1;
}

// realmyroomdata.cpp:2931
cModel::~cModel()
{
}

// realmyroomdata.cpp:2935
void cModel::SetPosition(WVector const& pos)
{
	m_pModelData->mat.pivot.x = pos.x;
	m_pModelData->mat.pivot.y = pos.y;
	m_pModelData->mat.pivot.z = pos.z;
}

// realmyroomdata.cpp:2943
void cModel::SetRotate(float angle)
{
	if (angle + m_angle < 0.0f)
		m_angle = 360.0f;

	if (angle + m_angle > 360.0f)
		m_angle = 0.0f;

	m_angle += angle;

	WMatrix mat;
	mat.Reset();

	float rad = angle * g_DEGTORAD;
	m_pModelData->mat.Rotate(rad, 1);

	SetPosition(m_pModelData->pos);

	mat.xa = m_pModelData->obb.extend[0];
	mat.ya = m_pModelData->obb.extend[1];
	mat.za = m_pModelData->obb.extend[2];

	mat.Rotate(rad, 1);

	m_pModelData->obb.center += mat.pivot;
	m_pModelData->obb.extend[0] = mat.xa;
	m_pModelData->obb.extend[1] = mat.ya;
	m_pModelData->obb.extend[2] = mat.za;

	if (m_pFurniture)
		m_pFurniture->r = angle;
}

// realmyroomdata.cpp:2980
void cModel::Apply_ChangeModelData()
{
	m_pModelData->pet->SetMatrix(m_pModelData->mat);
	m_pModelData->pet->Transform(NULL, 1.0f, 0);
	m_pModelData->pet->UpdateBSphere();

	CSceneManager::Instance()->SetPuppet(m_pModelData->name, m_pModelData->pet);

	GOLFDOC()->m_pPolySoup->ChangeDataStructure(m_pModelData);
}

// realmyroomdata.cpp:3003
// Rebuilds the model's world box from the eight corners of its oriented box,
// starting from the room's box turned inside out (the height spans 0 to 30).
void cModel::OBBToAABB()
{
	WPolySoup::sModel* pRoom = GOLFDOC()->m_pPolySoup->FindModel(NULL);

	WVector vMax = pRoom->aabb.min;
	WVector vMin = pRoom->aabb.max;
	vMin.y = 30.0f;

	WVector center = m_pModelData->obb.center;
	WVector ext0 = m_pModelData->obb.extend[0];
	WVector ext1 = m_pModelData->obb.extend[1];
	WVector ext2 = m_pModelData->obb.extend[2];

	vMax.y = 0.0f;
	WVector corner[8] = {
		center + ext0 * -1.0f + ext1 + ext2 * -1.0f,
		center + ext0 * -1.0f + ext1 * -1.0f + ext2 * -1.0f,
		center + ext0 + ext1 * -1.0f + ext2 * -1.0f,
		center + ext0 + ext1 + ext2 * -1.0f,
		center + ext0 * -1.0f + ext1 + ext2,
		center + ext0 * -1.0f + ext1 * -1.0f + ext2,
		center + ext0 + ext1 * -1.0f + ext2,
		center + ext0 + ext1 + ext2,
	};

	WVector* pCorner = corner;
	for (int count = 8; count != 0; count--, pCorner++)
	{
		if (!(vMax.x > pCorner->x))
			vMax.x = pCorner->x;
		if (!(vMax.y > pCorner->y))
			vMax.y = pCorner->y;
		if (!(vMax.z > pCorner->z))
			vMax.z = pCorner->z;
		if (!(vMin.x < pCorner->x))
			vMin.x = pCorner->x;
		if (!(vMin.y < pCorner->y))
			vMin.y = pCorner->y;
		if (!(vMin.z < pCorner->z))
			vMin.z = pCorner->z;
	}
	m_pModelData->aabb.max = vMax;
	m_pModelData->aabb.min = vMin;
}
