#pragma once

#include "frform.h"

class FrButton;
class FrComboBox;
class FrArea;
class FrEdit;
class FrStatic;

class FrTrainingOptionDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTrainingOptionDlg)

	FrTrainingOptionDlg();

protected:
	void OnOkBtnUp();
	void OnMapInit(int param);
	void OnMapPrevBtnInit(int param);
	void OnMapPrevBtnUp();
	void OnMapNextBtnInit(int param);
	void OnMapNextBtnUp();
	void OnWeatherInit(int param);
	void OnWeatherPrevBtnInit(int param);
	void OnWeatherPrevBtnUp();
	void OnWeatherNextBtnInit(int param);
	void OnWeatherNextBtnUp();
	void OnDirectionInit(int param);
	void OnDirectionPrevBtnInit(int param);
	void OnDirectionPrevBtnUp();
	void OnDirectionNextBtnInit(int param);
	void OnDirectionNextBtnUp();
	void OnIntensityInit(int param);
	void OnIntensityPrevBtnInit(int param);
	void OnIntensityPrevBtnUp();
	void OnIntensityNextBtnInit(int param);
	void OnIntensityNextBtnUp();

	FrEdit* m_pMap;
	FrButton* m_pMapPrev;
	FrButton* m_pMapNext;
	FrEdit* m_pWeather;
	FrButton* m_pWeatherPrev;
	FrButton* m_pWeatherNext;
	FrEdit* m_pDirection;
	FrButton* m_pDirectionPrev;
	FrButton* m_pDirectionNext;
	FrEdit* m_pIntensity;
	FrButton* m_pIntensityPrev;
	FrButton* m_pIntensityNext;
	unsigned char m_map;
	unsigned char m_weather;
	int m_direction;
	int m_intensity;
	bool m_bEnableChange;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrNewTrainingOptionDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrNewTrainingOptionDlg)

	FrNewTrainingOptionDlg();
	virtual ~FrNewTrainingOptionDlg();

protected:
	void ClearVariables();

	void OnInitMapButton(int param);
	void OnLBDownMapButton();
	void OnInitMapPrevButton(int param);
	void OnLBDownMapPrevButton();
	void OnInitMapNextButton(int param);
	void OnLBDownMapNextButton();
	void OnInitFrontHoleButton(int param);
	void OnLBDownFrontHoleButton();
	void OnInitBackHoleButton(int param);
	void OnLBDownBackHoleButton();
	void OnInitRandomHoleButton(int param);
	void OnLBDownRandomHoleButton();
	void OnInitShuffleHoleButton(int param);
	void OnLBDownShuffleHoleButton();
	void OnInitHareButton(int param);
	void OnLBDownHareButton();
	void OnInitKumoriButton(int param);
	void OnLBDownKumoriButton();
	void OnInitAmeButton(int param);
	void OnLBDownAmeButton();
	void OnInitWindDirectionEdit(int param);
	void OnInitPlayButton(int param);
	void OnLBDownPlayButton();
	void OnInitCancelButton(int param);
	void OnLBDownCancelButton();

	void ShowExpansionMenu(bool bShow);
	void SetEnvironmentToGame();
	void GetEnvironmentFromGame();

	FrButton* m_pMapButton;
	FrButton* m_pMapPrevButton;
	FrButton* m_pMapNextButton;
	unsigned long m_reserved11c;
	FrButton* m_pFrontHoleButton;
	FrButton* m_pBackHoleButton;
	FrButton* m_pRandomHoleButton;
	FrButton* m_pShuffleHoleButton;
	FrButton* m_pHareButton;
	FrButton* m_pKumoriButton;
	FrButton* m_pAmeButton;
	FrEdit* m_pWindDirectionEdit;
	unsigned long m_reserved140;
	FrButton* m_pPlayButton;
	FrButton* m_pCancelButton;
	bool m_bExpansion;
	unsigned short m_map;
	unsigned short m_hole;
	unsigned short m_weather;
	unsigned short m_windDirection;
	unsigned short m_windPower;

private:
	DECLARE_FRESH_MSGMAP()
};
