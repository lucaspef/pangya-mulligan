#pragma once

#include "frform.h"

class FrListBox;
class FrArea;
class FrButton;
class FrEdit;
class WTitleFont;
struct sItemInfo;

class CFurniture_AbilityDlg : public FrForm
{
public:
	DECLARE_OBJECT(CFurniture_AbilityDlg)

	CFurniture_AbilityDlg();
	virtual ~CFurniture_AbilityDlg();

	void GetItemInfo(sItemInfo* info);

	void OnFurnitureAbility_ItemListInit(int param);
	void OnFurnitureAbility_ItemListOwnerDraw(int param);
	void OnFurnitureAbility_ItemListBtnDown();
	void OnFurnitureAbility_ItemListMouseMove();
	void OnFurnitureAbility_ImgSelect(int param);
	void OnFurnitureAbility_SelectListInit(int param);
	void OnFurnitureAbility_EditArrayInit(int param);
	void OnFurnitureAbility_BtnArrayInit(int param);
	void OnFurnitureAbility_TextImg(int param);

protected:
	void SetList();

	FrListBox* m_pItemList;
	FrListBox* m_pSelectList;
	FrArea* m_pSelectImg;
	int m_reserved;
	sItemInfo* m_pSelectedItem;
	FrButton* m_pBtnArray;
	FrEdit* m_pEditArray;
	FrArea* m_pTextImg;
	WTitleFont* m_pWindFont;
	WTitleFont* m_pBongFont;
	const Bitmap* m_pSelectBitmap;
	const Bitmap* m_pSelectDisableBitmap;
	const Bitmap* m_pLineBitmap;
	unsigned long m_unused[3];

	DECLARE_FRESH_MSGMAP()
};
