#pragma once

#include <list>
#include "frform.h"
#include "../shared/globalgamedefine.h"

class FrListBox;
class FrStatic;
class FrButton;
class FrArea;

class FrFortuneOpenDlg : public FrForm
{
	DECLARE_OBJECT(FrFortuneOpenDlg)

	FrFortuneOpenDlg();
	virtual ~FrFortuneOpenDlg();

	void SetItemList(unsigned long* items, int num);
	void SetItemNewList();
	unsigned long GetTypeId() { return m_typeId; }

protected:
	void OnItemListInit(int param);
	void OnOkInit(int param);
	void OnCaption1Init(int param);
	void OnCaption11Init(int param);
	void OnCaption2Init(int param);
	void OnItemListOwnerDraw(int param);
	void OnTotalPriceInit(int param);
	void OnCaptionImgInit(int param);

	void UpdateCaptions();
	void UpdateLayout();

	std::list<sPouchPrize> m_list;	// 983: +0x110 (12 bytes: cabeca, tamanho em +0x114, alocador)
	unsigned long m_typeId;			// 983: +0x11c (zerado no construtor)
	FrListBox* m_pItemList;			// +0x120
	FrStatic* m_pTotalPrice;		// +0x124 (escondido no Init)
	FrButton* m_pOk;				// +0x128
	FrStatic* m_pCaption1;			// +0x12c
	FrStatic* m_pCaption11;			// +0x130
	FrStatic* m_pCaption2;			// +0x134
	FrArea* m_pCaptionImg;			// +0x138
	unsigned long m_pad983_13c;	// +0x13c: sem leitura/escrita no 983 (00aa8000..00aaa400; new 0x140)

	DECLARE_FRESH_MSGMAP()
};
