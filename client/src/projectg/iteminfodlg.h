#pragma once

#include "frform.h"

class FrGaugeBar;
class FrArea;
class FrEdit;
class FrStatic;
struct sTradeItem;
class FrItemInfoDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrItemInfoDlg)

	FrItemInfoDlg();
	virtual ~FrItemInfoDlg();

	void SetItemInfo(sTradeItem* item);
	void SetLevelBar();
	void SetLevelNumber(int index);
	void SetEnchantNumber(int index);
	void SetControlsAbility(bool bEnable);
	void DrawEnchantArrow();

protected:
	virtual bool OnInit();

	void OnPortraitInit(int param);
	void OnPortraitOwnerDraw();
	void OnNameInit(int param);
	void OnLevelInit(int param);
	void OnPowerStaticInit(int param);
	void OnControlStaticInit(int param);
	void OnAccuracyStaticInit(int param);
	void OnSpinStaticInit(int param);
	void OnCurveStaticInit(int param);
	void OnPowerEdit1Init(int param);
	void OnControlEdit1Init(int param);
	void OnAccuracyEdit1Init(int param);
	void OnSpinEdit1Init(int param);
	void OnCurveEdit1Init(int param);
	void OnPowerEdit2Init(int param);
	void OnControlEdit2Init(int param);
	void OnAccuracyEdit2Init(int param);
	void OnSpinEdit2Init(int param);
	void OnCurveEdit2Init(int param);
	void OnPowerBarInit(int param);
	void OnControlBarInit(int param);
	void OnAccuracyBarInit(int param);
	void OnSpinBarInit(int param);
	void OnCurveBarInit(int param);

	sTradeItem* m_pItem;
	FrArea* m_pPortrait;
	FrEdit* m_pName;
	FrArea* m_pLevel;
	FrGaugeBar* m_pBar[5];
	FrEdit* m_pEdit1[5];
	FrEdit* m_pEdit2[5];
	FrStatic* m_pStatic[5];
	const Bitmap* m_pEnchantArrow[2];

private:
	DECLARE_FRESH_MSGMAP()
};
