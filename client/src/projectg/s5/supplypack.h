#pragma once

#include "frform.h"

class FrListBox;
class FrButton;
class CExhibition;
class WTitleFontEx;
class CFontAnimation;
struct FrListItem;
struct sItemInfo;

namespace S5
{

	class FrSupplyPackDlg : public FrForm
	{
		DECLARE_OBJECT(FrSupplyPackDlg)
		FrSupplyPackDlg();
		virtual ~FrSupplyPackDlg();

		void Initialize(unsigned long tid);
		void UpdateData(unsigned int count);

	protected:
		virtual void OnProc(const float deltaTime);
		virtual void OnCancel() { }
		virtual bool OnResultConfirm(int result, FrForm* pForm);

		void OnItemListInit(int param);
		void OnOkayBtnInit(int param);
		void OnCancelBtnInit(int param);
		void OnOkayBtnUp();
		void OnCancelBtnUp();
		void OnBgAreaDraw();
		void OnItemListDraw(int param);
		void OnItemListUp();

		unsigned long m_tid;
		FrListBox* m_pItemList;
		FrButton* m_pCancelBtn;
		FrButton* m_pOkayBtn;
		CExhibition* m_pExhibition;
		float m_alpha;
		FrListItem* m_pSelected;
		WTitleFontEx* m_pFontNum;
		WTitleFontEx* m_pFontBig;
		float m_resultTimer;
		int m_bResult;
		unsigned int m_resultCount;
		CFontAnimation* m_pFontAnim;
		float m_elapsed;
		float m_arrowOffset;
		int m_resultNum[10];
		std::list<sItemInfo> m_itemList;
		int m_lastCount;
		int m_total;
		int m_step;
		int m_unused18c;

		DECLARE_FRESH_MSGMAP()
	};
}
