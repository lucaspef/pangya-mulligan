#pragma once

#include "frform.h"

class FrArea;
class FrStatic;

class FrAwardDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrAwardDlg)

	FrAwardDlg() { }

protected:
	int GetMedalIndex(unsigned long uid);

	void OnGoldStaticInit(int param);
	void OnSilverStaticInit(int param);
	void OnBronzeStaticInit(int param);
	void OnWinner1Init(int param);
	void OnWinner2Init(int param);
	void OnWinner3Init(int param);
	void OnWinner4Init(int param);
	void OnWinner5Init(int param);
	void OnWinner6Init(int param);
	void OnLuckInit(int param);
	void OnSpeederInit(int param);
	void OnRunnerInit(int param);
	void OnChipInInit(int param);
	void OnLongPuttInit(int param);
	void OnRecoveryInit(int param);
	void OnWinner1OwnerDraw(int param);
	void OnWinner2OwnerDraw(int param);
	void OnWinner3OwnerDraw(int param);
	void OnWinner4OwnerDraw(int param);
	void OnWinner5OwnerDraw(int param);
	void OnWinner6OwnerDraw(int param);
	void OnLuckOwnerDraw(int param);
	void OnSpeederOwnerDraw(int param);
	void OnRunnerOwnerDraw(int param);
	void OnChipInOwnerDraw(int param);
	void OnLongPuttOwnerDraw(int param);
	void OnRecoveryOwnerDraw(int param);

	FrArea* m_pWinner1;
	FrArea* m_pWinner2;
	FrArea* m_pWinner3;
	FrArea* m_pWinner4;
	FrArea* m_pWinner5;
	FrArea* m_pWinner6;
	FrArea* m_pLuck;
	FrArea* m_pSpeeder;
	FrArea* m_pRunner;
	FrArea* m_pChipIn;
	FrArea* m_pLongPutt;
	FrArea* m_pRecovery;

private:
	DECLARE_FRESH_MSGMAP()
};
