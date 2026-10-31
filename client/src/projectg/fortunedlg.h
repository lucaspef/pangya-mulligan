#pragma once

#include "frform.h"
struct IFF_ITEM_COMMON;
class FrArea;
class FrStatic;
class FrFortuneDlg : public FrForm
{
	DECLARE_OBJECT(FrFortuneDlg)

	FrFortuneDlg();
	virtual ~FrFortuneDlg();

	void SetTypeId(unsigned long typeId, IFF_ITEM_COMMON* item);

	void SetIcon983(const char* icon);
	void SetBox983(unsigned long typeId, const char* icon, const char* itemName); // 00aa78a0

	unsigned long GetTypeId() { return m_typeId; }

protected:
	void OnPortraitInit(int param);
	void OnDescNameInit(int param);
	void OnDescLevelInit(int param);
	void OnDescDescInit(int param);
	void OnTextNameInit983(int param); // 00aa7700
	void OnTextOpenInit983(int param); // 00aa7770
	void OnCaptionImgInit983(int param); // 00aa7690

	unsigned long m_reserved;
	unsigned long m_typeId;
	FrArea* m_pPortrait;
	FrStatic* m_pDescName;
	FrStatic* m_pDescLevel;
	FrStatic* m_pDescDesc;
	FrStatic* m_pTextName983; // +0x118: "text_name"
	FrStatic* m_pTextOpen983; // +0x11c: "text_open"
	FrArea* m_pCaptionImg983; // +0x120: "caption_img"

	DECLARE_FRESH_MSGMAP()
};
