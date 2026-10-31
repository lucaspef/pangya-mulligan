#include "minatl.h"
#include "schooldlg.h"
#include "frcombobox.h"
#include "wresrcmng.h"
#include "../shared/token.h"

extern Fresh* g_pFresh;

std::string szJpSchoolType[] = { K2L_Compatibility("\301\337\307\320"), K2L_Compatibility("\260\355\261\263"),
	K2L_Compatibility("\264\334\264\353"), K2L_Compatibility("\264\353\307\320") }; // 983 00ce48a0

IMPLEMENT_OBJECT(FrSchoolDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrSchoolDlg, FrForm)

ON_FRESH_VI("like", FRCMD_INIT, FrSchoolDlg::OnLikeInit)
ON_FRESH_BI("like", FRCMD_ENTERKEY, FrSchoolDlg::OnLikeEnterKey)
ON_FRESH_VI("school", FRCMD_INIT, FrSchoolDlg::OnSchoolInit)
ON_FRESH_VI("school", FRCMD_LBUTTONDOWN, FrSchoolDlg::OnSchoolBtnDown)
ON_FRESH_VV("search", FRCMD_LBUTTONUP, FrSchoolDlg::OnSearchBtnUp)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrSchoolDlg::OnOkBtnUp)

END_FRESH_MSGMAP()

FrSchoolDlg::FrSchoolDlg()
{
	m_schoolCode = 0;
	m_pType = NULL;
	m_pDist0 = NULL;
	m_pDist1 = NULL;
	m_pStaticDist1 = NULL;
	m_type = 0;
	m_dist0 = 0;
	m_dist1 = 0;
}

FrSchoolDlg::~FrSchoolDlg()
{
}

void FrSchoolDlg::OnLikeInit(int param)
{
	m_pLike = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pLike)
		m_pLike->SetVisible(false); // 983 008e37f0: slot +0x1c
}

bool FrSchoolDlg::OnLikeEnterKey(int param)
{
	OnSearchBtnUp();
	return false;
}

void FrSchoolDlg::OnSchoolInit(int param)
{
	m_pSchool = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrSchoolDlg::OnSchoolBtnDown(int param)
{
	FrListItem* item = (FrListItem*)param;
	std::string* name = (std::string*)item->pData;
	m_schoolCode = m_searchCode[item->no];
	m_schoolName = *name;
}

void FrSchoolDlg::OnSearchBtnUp()
{
	// HACK
	if (0)
		OnSchoolInit(0);
	if (!m_pLike || !m_pSchool)
		return;
	const char* like = m_pLike->GetLine(1, false);
	if (strlen(like) < 4)
		return;
	m_pSchool->ClearList();
	m_pSchool->ClearLine();
	cFile* file = g_resrcmng->GetCFile("school.txt", 0xffff);
	if (!file)
	{
		m_pSchool->AddString(like);
		m_pSchool->SetLine(1, like, 0, false, 0);
		return;
	}
	{
		char line[8192];
		char code[64];
		char name[128];
		int count = 0;
		while (file->Scan("%n", line))
		{
			int length = strlen(line);
			cTokenV token;
			token.Init(line, length);
			token.GetToken(code, " ", 1);
			token.GetToken(name, "\r\n", 2);
			if (strstr(name, like))
			{
				if (count < 10)
				{
					m_searchCode[count++] = atoi(code);
					m_pSchool->AddString(name);
					if (!m_pSchool->GetLineNum())
					{
						m_schoolCode = atoi(code);
						m_schoolName = name;
						m_pSchool->SetLine(1, name, 0, false, 0);
					}
				}
				else
				{
					m_schoolCode = 0;
					m_schoolName = "";
					m_pSchool->ClearList();
					m_pSchool->SetLine(1,
						K2L_Compatibility("\301\273\264\365 \300\332\274\274\307\317\260\324 \263\326\276\356\301\326\274\274\277\344"),
						0, false, 0);
					break;
				}
			}
		}
		CloseCFile(file);
		if (!m_pSchool->GetLineNum())
			m_pSchool->SetLine(1,
				K2L_Compatibility("\270\361\267\317\277\241\274\255 \303\243\300\273 \274\366 \276\370\275\300\264\317\264\331"),
				0, false, 0);
	}
}

void FrSchoolDlg::OnOkBtnUp()
{
	if (!m_schoolCode)
	{
		m_schoolName = "";
		m_pSchool->ClearList();
	}
	OnFreshOkay();
}

void FrSchoolDlg::OnExampleInit(int param)
{
	FrStatic* control = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (control)
		control->SetVisible(false);
}

void FrSchoolDlg::OnTypeInit(int param)
{
	m_pType = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrSchoolDlg::OnTypeListLBtnDown(int param)
{
	const char* name = m_pType->GetLine(1, false);
	if (!strcmp(name, m_typeName.c_str()))
		return;
	m_type = 5;
	for (unsigned int i = 0; i < 4; ++i)
	{
		if (!strcmp(name, szJpSchoolType[i].c_str()))
		{
			m_type = i + 1;
			m_typeName = name;
			break;
		}
	}
	if (m_type > 4)
		return;
	m_dist0Name = "";
	m_dist1Name = "";
	m_schoolName = "";
	m_dist0 = 0;
	m_dist1 = 0;
	m_schoolCode = 0;
	if (m_pDist0)
	{
		m_pDist0->SetLine(1, "", 0, false, 0);
		m_pDist0->ClearList();
		for (std::list<sJpSchool>::iterator it = m_dist0List.begin();
			it != m_dist0List.end(); ++it)
			if ((((*it).code >> 28) & 7) == m_type)
				m_pDist0->AddString((*it).name.c_str());
	}
	if (m_pDist1)
	{
		m_pDist1->SetLine(1, "", 0, false, 0);
		m_pDist1->ClearList();
	}
	if (m_pSchool)
	{
		m_pSchool->SetLine(1, "", 0, false, 0);
		m_pSchool->ClearList();
	}
	m_dist1Name = "";
	m_schoolName = "";
	m_dist1 = 0;
	m_schoolCode = 0;
	if (m_type - 1 == 3 || m_type - 1 == 2)
	{
		if (m_pDist1)
			m_pDist1->SetVisible(false);
		if (m_pStaticDist1)
			m_pStaticDist1->SetVisible(false);
	}
	else
	{
		if (m_pDist1)
			m_pDist1->SetVisible(true);
		if (m_pStaticDist1)
			m_pStaticDist1->SetVisible(true);
	}
}

void FrSchoolDlg::OnDist0Init(int param)
{
	m_pDist0 = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrSchoolDlg::OnDist0ListLBtnDown(int param)
{
	const char* name = m_pDist0->GetLine(1, false);
	if (!strcmp(name, m_dist0Name.c_str()))
		return;
	std::list<sJpSchool>::iterator it;
	for (it = m_dist0List.begin(); it != m_dist0List.end(); ++it)
	{
		if ((((*it).code >> 28) & 7) == m_type &&
			!strcmp(name, (*it).name.c_str()))
		{
			m_dist0 = (*it).code;
			m_dist0Name = name;
			break;
		}
	}
	if (it == m_dist0List.end())
		return;
	if (m_type - 1 == 3 || m_type - 1 == 2)
	{
		m_schoolName = "";
		m_schoolCode = 0;
		if (m_pSchool)
		{
			m_pSchool->SetLine(1, "", 0, false, 0);
			m_pSchool->ClearList();
			for (it = m_schoolList.begin(); it != m_schoolList.end(); ++it)
				if ((((*it).code >> 28) & 7) == m_type &&
					((*it).code & 0xfe00000) == (m_dist0 & 0xfe00000))
					m_pSchool->AddString((*it).name.c_str());
		}
	}
	else
	{
		m_dist1Name = "";
		m_schoolName = "";
		m_dist1 = 0;
		m_schoolCode = 0;
		if (m_pDist1)
		{
			m_pDist1->SetLine(1, "", 0, false, 0);
			m_pDist1->ClearList();
			for (it = m_dist1List.begin(); it != m_dist1List.end(); ++it)
				if ((((*it).code >> 28) & 7) == m_type &&
					((*it).code & 0xfe00000) == (m_dist0 & 0xfe00000))
					m_pDist1->AddString((*it).name.c_str());
		}
		if (m_pSchool)
		{
			m_pSchool->SetLine(1, "", 0, false, 0);
			m_pSchool->ClearList();
		}
	}
}

void FrSchoolDlg::OnDist1Init(int param)
{
	m_pDist1 = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrSchoolDlg::OnDist1ListLBtnDown(int param)
{
	const char* name = m_pDist1->GetLine(1, false);
	if (!strcmp(name, m_dist1Name.c_str()))
		return;
	std::list<sJpSchool>::iterator it;
	for (it = m_dist1List.begin(); it != m_dist1List.end(); ++it)
	{
		if ((((*it).code >> 28) & 7) == m_type &&
			((*it).code & 0xfe00000) == (m_dist0 & 0xfe00000) &&
			!strcmp(name, (*it).name.c_str()))
		{
			m_dist1 = (*it).code;
			m_dist1Name = name;
			break;
		}
	}
	if (it == m_dist0List.end())
		return;
	m_schoolName = "";
	m_schoolCode = 0;
	if (m_pSchool)
	{
		m_pSchool->SetLine(1, "", 0, false, 0);
		m_pSchool->ClearList();
		for (it = m_schoolList.begin(); it != m_schoolList.end(); ++it)
			if ((((*it).code >> 28) & 7) == m_type &&
				((*it).code & 0xfe00000) == (m_dist0 & 0xfe00000) &&
				((*it).code & 0x1fe000) == (m_dist1 & 0x1fe000))
				m_pSchool->AddString((*it).name.c_str());
	}
}

void FrSchoolDlg::OnStaticDist1Init(int param)
{
	m_pStaticDist1 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrSchoolDlg::OnSearchInit(int param)
{
	FrButton* control = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (control)
		control->SetVisible(false);
}

bool FrSchoolDlg::OnInit()
{
	if (m_pType)
		for (int i = 0; i < 4; ++i)
			m_pType->AddString(szJpSchoolType[i].c_str());
	cFile* file = g_resrcmng->GetCFile("school.txt", 0xffff);
	if (file)
	{
		int code;
		char name[1024];
		sJpSchool school;
		m_dist0List.clear();
		while (file->Scan("%d %s", &code, name))
		{
			school.code = code;
			school.name = name;
			if (code & 0x1fff)
				m_schoolList.push_back(school);
			else if (code & 0x1fe000)
				m_dist1List.push_back(school);
			else if (code & 0xfe00000)
				m_dist0List.push_back(school);
		}
		CloseCFile(file);
	}
	return true;
}
