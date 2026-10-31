#pragma once

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

class FrHelpDlg;

class FrDowngradeDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrDowngradeDlg)

	FrDowngradeDlg();

	void SetInfo(unsigned long charTypeId, char* upgrade, unsigned long* parts,
		unsigned long* auxParts, int mode);
	void SetInfo(unsigned long clubId, int mode);
	void SetLevelBar();

protected:
	void OnTitleInit(int param);
	void OnHelpBtnInit(int param);
	void OnHelpBtnUp();
	bool OnHelpDlgResult(int result, FrForm* form);
	void OnHelpEdit1Init(int param);
	void OnHelpEdit2Init(int param);
	void OnPowerBarInit(int param);
	void OnControlBarInit(int param);
	void OnImpactBarInit(int param);
	void OnSpinBarInit(int param);
	void OnCurveBarInit(int param);
	void OnPowerInit(int param);
	void OnControlInit(int param);
	void OnImpactInit(int param);
	void OnSpinInit(int param);
	void OnCurveInit(int param);
	void OnPowerEdit1Init(int param);
	void OnControlEdit1Init(int param);
	void OnImpactEdit1Init(int param);
	void OnSpinEdit1Init(int param);
	void OnCurveEdit1Init(int param);
	void OnPowerEdit2Init(int param);
	void OnControlEdit2Init(int param);
	void OnImpactEdit2Init(int param);
	void OnSpinEdit2Init(int param);
	void OnCurveEdit2Init(int param);
	void OnPowerBtnUp();
	void OnControlBtnUp();
	void OnImpactBtnUp();
	void OnSpinBtnUp();
	void OnCurveBtnUp();
	void OpenDowngrade(unsigned char stat);
	void SetLevelNumber(int index);
	void SetEnchantNumber(int index);

	FrGaugeBar* m_pBar[5];
	FrEdit* m_pLevelEdit[5];
	FrEdit* m_pEnchantEdit[5];
	FrButton* m_pStatBtn[5];
	FrEdit* m_pHelpEdit[2];
	FrArea* m_pTitle;
	FrButton* m_pHelpBtn;
	FrHelpDlg* m_pHelpDlg;
	unsigned long m_clubId;
	int m_type;
	int m_mode;
	char* m_pUpgrade;
	unsigned long* m_pParts;
	unsigned long* m_pAuxParts;
	unsigned long m_charTypeId;

private:
	DECLARE_FRESH_MSGMAP()
};
