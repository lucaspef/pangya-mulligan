#pragma once

#include <vector>
#include "../shared/globalgamedefine.h"
#include "anotherhand.h"
#include "taskmain.h"

class CExhibition;
class CPartTidList;
class CStatBar;
class FrArea;
class FrButton;
class FrListBox;
class FrSubEquipBar;
class FrTreasureCourse;
struct FrListItem;

class CFamilyMain : public CTaskMain
{
	DECLARE_OBJECT(CFamilyMain)

public:
	class cFamily
	{
	public:
		cFamily();
		~cFamily();

		void Init(const WRect& rect, std::vector<int>& petList,
			unsigned char petKind);
		void Render();
		void Process(float delta);
		void ConvertUserInfo(sUserInfo& info);
		void SetUserEquip(int category, unsigned long id);
		void SetItemSlot(unsigned long* pItemSlot);
		const WPoint& GetFramePos() const;
		const sUserEquip& GetUserEquip() const;

	private:
		friend class CFamilyMain;
		void SetStatus();
		void PetRebuild();
		void MakePet(std::vector<int>& petList);

		WRect m_rect;
		WPoint m_framePos;
		WPoint m_modelPos;
		CExhibition* m_pExhibition;
		CAnotherHand m_anotherHand;
		CPartTidList* m_pPartTidList;
		sCharacterInfo m_charInfo;
		sCaddieInfo m_caddieInfo;
		sItemInfo m_clubInfo;
		sUserEquip m_userEquip;
		sMascotInfo m_mascotInfo;
		CStatBar* m_pStatBar;
		unsigned char m_petKind;
#ifndef REBANG_SEMANTIC_ONLY
	public:
		static void __cdecl
		RbMakepetFunclets(); // HACK: EH funclets of the naked MakePet (nakedify)
#endif
	};

	static unsigned char m_mapIndex;
	static unsigned long ms_itemslotBackup[4][10];

	CFamilyMain();
	virtual ~CFamilyMain();

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnDisplay();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

protected:
	virtual bool OnResultSubEquipBar(int result, FrForm* pForm);

	void TurnOffControls();
	void OnInitFamilyMain(int param);
	void OnInitFamilyFinish();
	void OnInitAreaPet1(int param);
	void OnInitAreaPet2(int param);
	void OnInitAreaPet3(int param);
	void OnInitAreaPet4(int param);
	void OnInitAreaGlobal(int param);
	void OnDrawAreaPet1();
	void OnDrawAreaPet2();
	void OnDrawAreaPet3();
	void OnDrawAreaPet4();
	void OnDrawAreaGlobal();
	void OnInitBtnStart(int param);
	void OnInitBtnSelMap(int param);
	void OnInitBtnHoleType(int param);
	void OnInitBtnHoleNum(int param);
	void OnInitBtnTimeLimit(int param);
	void OnInitCreatePet3(int param);
	void OnInitCreatePet4(int param);
	void OnInitDeletePet3(int param);
	void OnInitDeletePet4(int param);
	void OnDownBtnStart();
	void OnDownBtnSelMap();
	bool OnResultSelMap(int result, FrForm* pForm);
	void OnDownBtnHoleType();
	void OnDownBtnHoleNum();
	void OnDownBtnTimeLimit();
	void OnDownCreatePet3();
	void OnDownCreatePet4();
	void OnDownDeletePet3();
	void OnDownDeletePet4();
	void OnDrawBtnStart();
	void OnDrawBtnSelMap();
	void OnDrawBtnHoleType();
	void OnDrawBtnHoleNum();
	void OnDrawBtnTimeLimit();
	void OnInitListEquip1(int param);
	void OnInitListEquip2(int param);
	void OnInitListEquip3(int param);
	void OnInitListEquip4(int param);
	void OnDownListEquip1(int param);
	void OnDownListEquip2(int param);
	void OnDownListEquip3(int param);
	void OnDownListEquip4(int param);
	void OnDownEquipListBox(int index);
	bool OnItemEquipResult(int result, FrForm* pForm);
	void OnDrawListEquip1(int param);
	void OnDrawListEquip2(int param);
	void OnDrawListEquip3(int param);
	void OnDrawListEquip4(int param);
	void OnDrawEquipListBox(FrListItem* pItem);
	void ReFresh(bool);

	DECLARE_FRESH_MSGMAP()

private:
	std::vector<cFamily*> m_families;
	std::vector<FrListBox*> m_equipLists;
	std::vector<FrArea*> m_areas;
	std::vector<FrButton*> m_buttons;
	unsigned long m_timeLimit;
	unsigned char m_holeNum;
	unsigned char m_holeType;
	std::vector<int> m_petList;
	FrTreasureCourse* m_pSelMapDlg;
	FrSubEquipBar* m_pEquipBar;
	int m_curPet;
	FrListItem* m_pCurItem;
	bool m_bCreatingPet;
};
