#pragma once

#include <string>
#include <list>
#include "frform.h"

class FrComboBox;
class FrEdit;
class FrStatic;

class FrSchoolDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrSchoolDlg)
	FrSchoolDlg();
	virtual ~FrSchoolDlg();

	struct sJpSchool
	{
		unsigned long code;
		std::string name;
	};

protected:
	virtual bool OnInit();

	void OnLikeInit(int param);
	bool OnLikeEnterKey(int param);
	void OnSchoolInit(int param);
	void OnSchoolBtnDown(int param);
	void OnSearchBtnUp();
	void OnOkBtnUp();
	void OnExampleInit(int param);
	void OnTypeInit(int param);
	void OnTypeListLBtnDown(int param);
	void OnDist0Init(int param);
	void OnDist0ListLBtnDown(int param);
	void OnDist1Init(int param);
	void OnDist1ListLBtnDown(int param);
	void OnStaticDist1Init(int param);
	void OnSearchInit(int param);

public:
	unsigned long m_schoolCode;
	std::string m_schoolName;

protected:
	FrEdit* m_pLike;
	FrComboBox* m_pSchool;
	FrComboBox* m_pType;
	FrComboBox* m_pDist0;
	FrComboBox* m_pDist1;
	FrStatic* m_pStaticDist1;
	std::string m_typeName;
	std::string m_dist0Name;
	std::string m_dist1Name;
	unsigned long m_type;
	unsigned long m_dist0;
	unsigned long m_dist1;
	std::list<sJpSchool> m_dist0List;
	std::list<sJpSchool> m_dist1List;
	std::list<sJpSchool> m_schoolList;
	int m_searchCode[32];

private:
	DECLARE_FRESH_MSGMAP()
};
