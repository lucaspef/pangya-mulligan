#pragma once

#include "frform.h"

class FrStatic;

class ntMsnUserSnapshotDlg : public FrForm
{
	DECLARE_OBJECT(ntMsnUserSnapshotDlg)

	ntMsnUserSnapshotDlg();
	virtual ~ntMsnUserSnapshotDlg();

protected:
	void OnIdInit(int param);
	void OnGenderInit(int param);
	void OnLevelInit(int param);
	void OnGuildInit(int param);
	void OnSchoolInit(int param);
	void OnStatusInit(int param);
	void OnDetailClick();

	FrStatic* m_pId;
	FrStatic* m_pGender;
	FrStatic* m_pLevel;
	FrStatic* m_pGuild;
	FrStatic* m_pSchool;
	FrStatic* m_pStatus;

	DECLARE_FRESH_MSGMAP()
};
