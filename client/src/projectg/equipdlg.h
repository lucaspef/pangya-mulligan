#pragma once

#include <map>
#include "frform.h"

class FrButton;
class FrGaugeBar;
class FrComboBox;
class FrArea;
class FrEdit;
class FrListBox;
class FrStatic;
class FrViewer;
class WTitleFont;

class FrEquipDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrEquipDlg)

	FrEquipDlg();
	virtual ~FrEquipDlg();

	void Init(unsigned long* slot, bool bSubtract);
	unsigned long* GetMyItemSlot();

	struct MiniBtn
	{
		MiniBtn() { }
		WRect rect;
		const Bitmap* pBitmap;
		int index;
	};

protected:
	void OnCloseInit(int param);
	void OnCloseBtnUp();
	void OnWarehouseInit(int param);
	void OnWarehouseOwnerDraw(int param);
	void OnWarehouseBtnDown();
	void OnWarehouseRBtnDown();
	void OnWarehouseDblClick();
	void OnItemSlotInit(int param);
	void OnItemSlotOwnerDraw(int param);
	void OnItemSlotDblClick();
	void OnEquipSlotInit(int param);
	void OnEquipSlotBtnUp();
	void OnUnequipSlotInit(int param);
	void OnUnequipSlotBtnUp();
	void OnResetSlotInit(int param);
	void OnResetSlotBtnUp();
	void EquipItem(FrListItem* item);
	void UnequipItem(FrListItem* item);
	void OpenInformation(unsigned long typeId);

	FrListBox* m_pWarehouse;
	FrListBox* m_pItemSlot;
	std::map<unsigned long, int> m_itemCount;
	unsigned long m_mySlot[11];
	const Bitmap* m_pWarehouseBtn[3];
	const Bitmap* m_pItemSelectBtn[2];
	WTitleFont* m_pFont;
	// E432: 983 (equipdlg.cpp 00ad3420..00ad5020) so acessa delete +0x1b4 e clock +0x1cc (OnWarehouseInit 00ad3480, 00ad4540)
	MiniBtn m_pad983_16c[3];
	MiniBtn m_deleteIcon;
	MiniBtn m_clockIcon;
	MiniBtn m_pad983_1e4[3];

private:
	DECLARE_FRESH_MSGMAP()
};
