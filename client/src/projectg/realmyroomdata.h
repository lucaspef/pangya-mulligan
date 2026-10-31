#pragma once

#include <list>
#include "frcmdtarget.h"
#include "polysoup.h"
#include "singleton.h"
#include "wlist.h"

// Partial views used by other units (same objects as sFurniture_List / WPolySoup::sModel).
struct sMyRoomItemInfo
{
	unsigned long m_pad0;
	unsigned long m_typeId;
};

struct sFurniture_List;

struct sRealMyRoomModelInfo
{
	unsigned long m_pad0;
	unsigned long tid;
};

struct sRealMyRoomModel
{
	unsigned long m_pad0;
	sRealMyRoomModelInfo* pInfo;
};

class cModel
{
public:
	cModel();
	~cModel();

	void SetPosition(const WVector& pos);
	void SetRotate(float angle);
	void Apply_ChangeModelData();
	void OBBToAABB();

	int m_state;
	union
	{
		sFurniture_List* m_pFurniture;
		sFurniture_List* pInfo;
		sMyRoomItemInfo* m_pInfo;
	};
	WPolySoup::sModel* m_pModelData;
	float m_angle;
	int m_handle;
};

// wall-attached furniture (type class 0xe)
#define MYROOM_IS_WALL_ITEM(model) \
	(((model)->m_pInfo->m_typeId & 0x3ff800) == 0x7000)

class CRealMyRoomData : public WSingleton<CRealMyRoomData>, public FrCmdTarget
{
public:
	CRealMyRoomData();
	virtual ~CRealMyRoomData();

	void OnLoad();
	void OnDestory();
	void Process(float elapsed);
	void Render();
	cModel* CreateModel(const char* name);
	cModel* CreateModel(const char* name, WVector pos, float angle,
		bool bRename, short type);
	cModel* ModelByRayIntersectOBB(const WVector& pos);
	cModel* GetModelData(unsigned long id, unsigned long typeId);
	cModel* ModelByRayIntersect(const WVector& origin, const WVector& dir);
	void MoveModelPosition(unsigned char mode);
	void ListFurnitureSelect(sFurniture_List* pList);
	unsigned long GetState();
	void SetState(unsigned long state);
	void DeleteList();
	void SetTargetPosition(const WVector& pos, unsigned char mode);
	bool IsArragementFurniture(unsigned long id);
	bool IsModelData_SaveList(unsigned long id, unsigned long typeId);
	void ResetList(bool all);
	int IsFunritureValueChange(cModel& model);
	void ChangeWallTexture(sFurniture_List* pList, bool bReset);
	bool ChangePosterTexture(unsigned long typeId, WPolySoup::sModel& model);
	void ChangeFloorTexture(sFurniture_List* pList, bool bReset);
	void SetRotate(float angle);
	WVector ChangeModelPosition(int mode, cModel* pModel,
		WPolySoup::sModel* pData);
	WVector SlideWallPosition(cModel* pModel, WPolySoup::sModel* pData);
	void SetModelPosition();
	bool AddModel(sFurniture_List furniture);
	bool SaveModel(bool bSave);
	void ResotreList();

	WList<cModel*> m_ArrangeList;
	std::list<sFurniture_List> m_furnitureList;
	union
	{
		cModel* m_pSelectModel;
		cModel* m_pSelModel;
		sRealMyRoomModel* m_pSelModelInfo;
	};
	union
	{
		cModel* m_pPickedModel;
		cModel* m_pOBBModel;
	};
	cModel m_editModel;
	WVector m_targetPos;
	WVector m_pickPos;
	WVector m_dragPos;
	union
	{
		bool m_bDropped;
		bool m_bModelDragging;
	};
	unsigned long m_state;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSavemodelFunclets(); // HACK: EH funclets of the naked SaveModel (nakedify)
#endif
};
