#pragma once

#include <string>
#include <vector>
#include "taskmain.h"
#include "anotherhand.h"

class CStatBar;
class CExhibition;
class CPartTidList;
class FrArea;
class FrListBox;
class FrForm;
struct FrListItem;

class CGhostMain : public CTaskMain
{
	DECLARE_OBJECT(CGhostMain)

public:
	class cImgBinder;

	CGhostMain();
	virtual ~CGhostMain();

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnDestroy();
	virtual void OnProcess(float dt);
	virtual void OnDisplay();
	virtual void HandleMsg(const MsgObject& msg);

protected:
	void OnInitGhostMain(int param);
	void OnInitGhostFinish();
	void OnInitStart(int param);
	void OnDownStart();
	CExhibition* MakePet(FrArea* pArea, CPartTidList* pParts, int index);
	void OnExit();
	void HidePet(bool bWait);
	void OnInitPlayerZone(int param);
	void OnDrawPlayerZone();
	void OnInitSelectCourse(int param);
	void OnDownSelectCourse();
	void OnInitUserEquip(int param);
	void OnDrawUserEquip(int param);
	void OnDownUserEquip();
	void OnInitGhostEquip(int param);
	void OnDrawGhostEquip(int param);
	void OnInitGhostZone(int param);
	void OnDrawGhostZone();
	void OnInitTipZone(int param);
	void OnDrawTipZone();
	void OnInitAreaUserStat(int param);
	void OnDrawAreaUserStat();
	void OnInitAreaGhostStat(int param);
	void OnDrawAreaGhostStat();

private:
	virtual bool OnResultSubEquipBar(int result, FrForm* pForm);
	void Initialize();
	CStatBar* BuildStatusBar(int index);
	void StatProcess(float dt);
	void OnProcTip(float dt);
	void SetUserEquip(PANGYA_ITEM_GROUP group, unsigned long id);
	void UserPetRebuild();
	bool CallBackEquipItemResult(int result, FrForm* pForm);
	bool IsFocusToMain();
	void SendPacketEquipResult(PANGYA_ITEM_GROUP group, unsigned long id);
	void RefreshAllUccClothes();

	CAnotherHand m_hand;
	CExhibition* m_pUserPet;
	CExhibition* m_pGhostPet;
	CPartTidList* m_pUserParts;
	CPartTidList* m_pGhostParts;
	float m_ghostAlpha;
	bool m_bGhostVisible;
	std::vector<FrArea*> m_areas;
	cImgBinder* m_pUserStatImage;
	cImgBinder* m_pGhostStatImage;
	float m_userStatAlpha;
	float m_userStatStopX;
	float m_ghostStatStopX;
	float m_userStatSpeed;
	float m_ghostStatSpeed;
	bool m_bShowStat;
	float m_statTime;
	float m_statSpeedTime;
	FrListBox* m_pUserEquip;
	FrListBox* m_pGhostEquip;
	FrListItem* m_pSelectedIcon;
	std::vector<std::string> m_userIcons;
	std::vector<std::string> m_ghostIcons;
	float m_equipAlpha;
	FrForm* m_pEquipDlg;
	CStatBar* m_pUserStat;
	CStatBar* m_pGhostStat;
	WRect m_courseRect;
	const Bitmap* m_pCourseBitmap;
	unsigned long m_unused408;
	unsigned char m_ghostSlot;
	unsigned long m_unused410;
	int m_ghostScore;
	int m_ghostPang;
	unsigned long m_unused41c[2];
	cImgBinder* m_pTipImage;
	float m_tipAlpha;
	bool m_bTipHide;
	float m_tipTime;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOndownstartFunclets(); // HACK: EH funclets of the naked OnDownStart (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbBuildstatusbarFunclets(); // HACK: EH funclets of the naked BuildStatusBar (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnresultsubequipFunclets(); // HACK: EH funclets of the naked OnResultSubEquipBar (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSetuserequipFunclets(); // HACK: EH funclets of the naked SetUserEquip (nakedify)
#endif
};
