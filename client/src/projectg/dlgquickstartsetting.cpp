#include "minatl.h"
#include <algorithm>
#include "dlgquickstartsetting.h"
#include "fredit.h"
#include "frbutton.h"
#include "frarea.h"
#include "frcombobox.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "shareddoc.h"
#include "vc10_layout.h"
#include "projectg.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrQuickResultDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrQuickResultDlg, FrForm)

ON_FRESH_VI("ok1", FRCMD_INIT, FrQuickResultDlg::OnOkBtnInit)
ON_FRESH_VV("ok1", FRCMD_LBUTTONUP, FrQuickResultDlg::OnOkBtnUp)

END_FRESH_MSGMAP()

FrQuickResultDlg::FrQuickResultDlg()
{
}

FrQuickResultDlg::~FrQuickResultDlg()
{
}

void FrQuickResultDlg::OnOkBtnUp()
{
	OnOK();
}

void FrQuickResultDlg::SetContents(const char* text)
{
	if (text)
		SetMessage(text, false);
}

void FrQuickResultDlg::SetOkbtnEnable(bool bEnable)
{
	if (m_pOkBtn)
		m_pOkBtn->Enable(bEnable);
}

void FrQuickResultDlg::OnOkBtnInit(int param)
{
	m_pOkBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pOkBtn)
		m_pOkBtn->Enable(false);
}

IMPLEMENT_OBJECT(FrQuickStartDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrQuickStartDlg, FrForm)

ON_FRESH_VI("stroke", FRCMD_INIT, FrQuickStartDlg::OnStrokeBtnInit)
ON_FRESH_VV("stroke", FRCMD_LBUTTONUP, FrQuickStartDlg::OnStrokeBtnUp)
ON_FRESH_VI("strokestop", FRCMD_INIT, FrQuickStartDlg::OnStrokeStopInit)
ON_FRESH_VI("tournament", FRCMD_INIT, FrQuickStartDlg::OnTournamentBtnInit)
ON_FRESH_VV("tournament", FRCMD_LBUTTONUP, FrQuickStartDlg::OnTournamentBtnUp)
ON_FRESH_VI("tournamentstop", FRCMD_INIT, FrQuickStartDlg::OnTournamenStopInit)
ON_FRESH_VI("battle", FRCMD_INIT, FrQuickStartDlg::OnBattleBtnInit)
ON_FRESH_VV("battle", FRCMD_LBUTTONUP, FrQuickStartDlg::OnBattleBtnUp)
ON_FRESH_VI("battlestop", FRCMD_INIT, FrQuickStartDlg::OnBattleStopInit)
ON_FRESH_VI("chat", FRCMD_INIT, FrQuickStartDlg::OnChatBtnInit)
ON_FRESH_VV("chat", FRCMD_LBUTTONUP, FrQuickStartDlg::OnChatBtnUp)
ON_FRESH_VI("chatstop", FRCMD_INIT, FrQuickStartDlg::OnChatStopInit)
ON_FRESH_VI("option", FRCMD_INIT, FrQuickStartDlg::OnOptionBtnInit)
ON_FRESH_VV("option", FRCMD_LBUTTONUP, FrQuickStartDlg::OnOptionBtnUp)

END_FRESH_MSGMAP()

FrQuickStartDlg::FrQuickStartDlg()
{
	m_pSettingDlg = NULL;
}

FrQuickStartDlg::~FrQuickStartDlg()
{
}

bool FrQuickStartDlg::OnInit()
{
	bool bEnable = CMatchingSystem::Instance()->CheckIntrestCode(0) != 0;
	if (m_pStrokeBtn)
	{
		m_pStrokeBtn->Enable(bEnable);
		m_pStrokeBtn->SetVisible(bEnable);
	}
	if (m_pStrokeStop)
	{
		if (bEnable)
			m_pStrokeStop->SetVisible(false);
		else
			m_pStrokeStop->SetVisible(true);
	}

	bEnable = CMatchingSystem::Instance()->CheckIntrestCode(4) != 0;
	if (m_pTournamentBtn)
	{
		m_pTournamentBtn->Enable(bEnable);
		m_pTournamentBtn->SetVisible(bEnable);
	}
	if (m_pTournamentStop)
	{
		if (bEnable)
			m_pTournamentStop->SetVisible(false);
		else
			m_pTournamentStop->SetVisible(true);
	}

	bEnable = CMatchingSystem::Instance()->CheckIntrestCode(10) != 0;
	if (m_pBattleBtn)
	{
		m_pBattleBtn->Enable(bEnable);
		m_pBattleBtn->SetVisible(bEnable);
	}
	if (m_pBattleStop)
	{
		if (bEnable)
			m_pBattleStop->SetVisible(false);
		else
			m_pBattleStop->SetVisible(true);
	}

	bEnable = CMatchingSystem::Instance()->CheckIntrestCode(2) != 0;
	if (m_pChatBtn)
	{
		m_pChatBtn->Enable(bEnable);
		m_pChatBtn->SetVisible(bEnable);
	}
	if (m_pChatStop)
	{
		if (bEnable)
			m_pChatStop->SetVisible(false);
		else
			m_pChatStop->SetVisible(true);
	}

	return true;
}

void FrQuickStartDlg::OnStrokeStopInit(int param)
{
	m_pStrokeStop = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuickStartDlg::OnTournamenStopInit(int param)
{
	m_pTournamentStop = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuickStartDlg::OnBattleStopInit(int param)
{
	m_pBattleStop = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuickStartDlg::OnChatStopInit(int param)
{
	m_pChatStop = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrQuickStartDlg::OnStrokeBtnInit(int param)
{
	m_pStrokeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrQuickStartDlg::OnStrokeBtnUp()
{
	if (m_pStrokeBtn->IsEnabled())
	{
		SendRequestMatching(0);
		OnOK();
	}
}

void FrQuickStartDlg::OnTournamentBtnInit(int param)
{
	m_pTournamentBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrQuickStartDlg::OnTournamentBtnUp()
{
	if (m_pTournamentBtn->IsEnabled())
	{
		SendRequestMatching(4);
		OnOK();
	}
}

void FrQuickStartDlg::OnBattleBtnInit(int param)
{
	m_pBattleBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrQuickStartDlg::OnBattleBtnUp()
{
	if (m_pBattleBtn->IsEnabled())
	{
		SendRequestMatching(10);
		OnOK();
	}
}

void FrQuickStartDlg::OnChatBtnInit(int param)
{
	m_pChatBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrQuickStartDlg::OnChatBtnUp()
{
	if (m_pChatBtn->IsEnabled())
	{
		SendRequestMatching(2);
		OnOK();
	}
}

void FrQuickStartDlg::OnOptionBtnInit(int param)
{
	m_pOptionBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrQuickStartDlg::OnOptionBtnUp()
{
	m_pSettingDlg = CreateForm<FrQuickStartSettingDlg>(g_pFresh->GetManager(),
		this, "quick_start_setting", NULL);

	if (m_pSettingDlg)
	{
		WPoint pos(468.0f, 158.0f);
		m_pSettingDlg->Open(
			(FRESH_PFN_RESULT)&FrQuickStartDlg::OnQuickStartSettingDlgResult,
			pos, 0xc0);
	}
}

bool FrQuickStartDlg::OnQuickStartSettingDlgResult(int result, FrForm* form)
{
	switch (result)
	{
	case 1:
		m_pSettingDlg->UpdateData();
		OnInit();
		break;
	}

	m_pSettingDlg = NULL;
	return true;
}

void FrQuickStartDlg::SendRequestMatching(unsigned char type)
{
	// HACK: dead calls (emitted after _Construct<sIntrest> in the original)
	if (0)
	{
		std::vector<sIntrest>::iterator* q = 0;
		std::vector<unsigned char>::iterator* p = 0;
		if (*q != *q)
			std::find(*p, *p, (unsigned char)0);
		std::copy_backward((sIntrest*)0, (sIntrest*)0, (sIntrest*)0);
	}
	if (type != 0x17)
	{
		CSharedDoc::Instance()->m_userInfoMod = 3;

		WSendPacket packet((enumClientPacket)0xbf);
		packet.Encode1(type);
		packet.Send(TO_GAME);
	}
}

IMPLEMENT_OBJECT(FrQuickStartSettingDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrQuickStartSettingDlg, FrForm)

ON_FRESH_VI("chk_stroke", FRCMD_INIT,
	FrQuickStartSettingDlg::OnChkStrokeBtnInit)
ON_FRESH_VV("chk_stroke", FRCMD_LBUTTONUP,
	FrQuickStartSettingDlg::OnChkStrokeBtnUp)
ON_FRESH_VI("chk_tournament", FRCMD_INIT,
	FrQuickStartSettingDlg::OnChkTournamentBtnInit)
ON_FRESH_VV("chk_tournament", FRCMD_LBUTTONUP,
	FrQuickStartSettingDlg::OnChkTournamentBtnUp)
ON_FRESH_VI("chk_battle", FRCMD_INIT,
	FrQuickStartSettingDlg::OnChkBattleBtnInit)
ON_FRESH_VV("chk_battle", FRCMD_LBUTTONUP,
	FrQuickStartSettingDlg::OnChkBattleBtnUp)
ON_FRESH_VI("chk_chat", FRCMD_INIT, FrQuickStartSettingDlg::OnChkChatBtnInit)
ON_FRESH_VV("chk_chat", FRCMD_LBUTTONUP, FrQuickStartSettingDlg::OnChkChatBtnUp)
ON_FRESH_VI("combo_condition_1", FRCMD_INIT,
	FrQuickStartSettingDlg::OnCmbCondition1BoxInit)
ON_FRESH_VI("combo_condition_2", FRCMD_INIT,
	FrQuickStartSettingDlg::OnCmbCondition2BoxInit)
ON_FRESH_VI("combo_condition_3", FRCMD_INIT,
	FrQuickStartSettingDlg::OnCmbCondition3BoxInit)

END_FRESH_MSGMAP()

FrQuickStartSettingDlg::FrQuickStartSettingDlg()
{
	C_ASSERT_VC10(sizeof(FrQuickStartSettingDlg) == 0x190 && offsetof(FrQuickStartSettingDlg, m_bCheck) == 0x12c); // 983: push 0x190 em 00ad1552
	for (int i = 0; i < 0x17; i++)
		m_bCheck[i] = 0;
}

FrQuickStartSettingDlg::~FrQuickStartSettingDlg()
{
}

void FrQuickStartSettingDlg::OnChkStrokeBtnInit(int param)
{
	m_pChkStroke = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pChkStroke && CMatchingSystem::IsInstantiated())
	{
		if (CMatchingSystem::Instance()->CheckIntrestCode(0))
		{
			m_pChkStroke->SetStatus(FrButton::PRESSED);
			m_bCheck[0] = 1;
		}
		else
		{
			m_pChkStroke->SetStatus(FrButton::NORMAL);
		}
	}
}

void FrQuickStartSettingDlg::OnChkStrokeBtnUp()
{
	if (CMatchingSystem::IsInstantiated() && m_pChkStroke)
		m_bCheck[0] = m_pChkStroke->GetStatus() == FrButton::PRESSED;
}

void FrQuickStartSettingDlg::OnChkTournamentBtnInit(int param)
{
	m_pChkTournament = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pChkTournament && CMatchingSystem::IsInstantiated())
	{
		if (CMatchingSystem::Instance()->CheckIntrestCode(4))
		{
			m_pChkTournament->SetStatus(FrButton::PRESSED);
			m_bCheck[4] = 1;
		}
		else
		{
			m_pChkTournament->SetStatus(FrButton::NORMAL);
		}
	}
}

void FrQuickStartSettingDlg::OnChkTournamentBtnUp()
{
	if (CMatchingSystem::IsInstantiated() && m_pChkTournament)
		m_bCheck[4] = m_pChkTournament->GetStatus() == FrButton::PRESSED;
}

void FrQuickStartSettingDlg::OnChkBattleBtnInit(int param)
{
	m_pChkBattle = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pChkBattle && CMatchingSystem::IsInstantiated())
	{
		if (CMatchingSystem::Instance()->CheckIntrestCode(10))
		{
			m_pChkBattle->SetStatus(FrButton::PRESSED);
			m_bCheck[10] = 1;
		}
		else
		{
			m_pChkBattle->SetStatus(FrButton::NORMAL);
		}
	}
}

void FrQuickStartSettingDlg::OnChkBattleBtnUp()
{
	if (CMatchingSystem::IsInstantiated() && m_pChkBattle)
		m_bCheck[10] = m_pChkBattle->GetStatus() == FrButton::PRESSED;
}

void FrQuickStartSettingDlg::OnChkChatBtnInit(int param)
{
	m_pChkChat = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pChkChat && CMatchingSystem::IsInstantiated())
	{
		if (CMatchingSystem::Instance()->CheckIntrestCode(2))
		{
			m_pChkChat->SetStatus(FrButton::PRESSED);
			m_bCheck[2] = 1;
		}
		else
		{
			m_pChkChat->SetStatus(FrButton::NORMAL);
		}
	}
}

void FrQuickStartSettingDlg::OnChkChatBtnUp()
{
	if (CMatchingSystem::IsInstantiated() && m_pChkChat)
		m_bCheck[2] = m_pChkChat->GetStatus() == FrButton::PRESSED;
}

void FrQuickStartSettingDlg::OnCmbCondition1BoxInit(int param)
{
	m_pCondition[0] = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);

	if (m_pCondition[0] && CMatchingSystem::Instance())
	{
		CMatchingSystem* pSystem = CMatchingSystem::Instance();
		m_pCondition[0]->AddString(
			K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4"));

		std::vector<sIntrest> intrests;
		std::vector<sIntrest>::iterator it;
		int result = pSystem->GetDisplayIntrests(K2L_Compatibility("\xb0\xd4\xc0\xd3"), &intrests);
		if (result && !intrests.empty())
		{
			for (it = intrests.begin(); it != intrests.end(); ++it)
			{
				m_pCondition[0]->AddString(it->name);
			}

			m_pCondition[0]->SetLine(1,
				pSystem->GetNameChecked(K2L_Compatibility("\xb0\xd4\xc0\xd3")).c_str(), 0, false,
				0);
		}
		else
		{
			m_pCondition[0]->SetLine(1,
				K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4"), 0, false,
				0);
		}
	}
}

void FrQuickStartSettingDlg::OnCmbCondition2BoxInit(int param)
{
	m_pCondition[1] = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);

	if (m_pCondition[1] && CMatchingSystem::Instance())
	{
		CMatchingSystem* pSystem = CMatchingSystem::Instance();
		m_pCondition[1]->AddString(
			K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4"));

		std::vector<sIntrest> intrests;
		std::vector<sIntrest>::iterator it;
		int result = pSystem->GetDisplayIntrests(K2L_Compatibility("\xc3\xeb\xb9\xcc"), &intrests);
		if (result && !intrests.empty())
		{
			for (it = intrests.begin(); it != intrests.end(); ++it)
			{
				m_pCondition[1]->AddString(it->name);
			}

			m_pCondition[1]->SetLine(1,
				pSystem->GetNameChecked(K2L_Compatibility("\xc3\xeb\xb9\xcc")).c_str(), 0, false,
				0);
		}
		else
		{
			m_pCondition[1]->SetLine(1,
				K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4"), 0, false,
				0);
		}
	}
}

void FrQuickStartSettingDlg::OnCmbCondition3BoxInit(int param)
{
	m_pCondition[2] = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);

	if (m_pCondition[2] && CMatchingSystem::Instance())
	{
		CMatchingSystem* pSystem = CMatchingSystem::Instance();
		m_pCondition[2]->AddString(
			K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4"));

		std::vector<sIntrest> intrests;
		std::vector<sIntrest>::iterator it;
		int result = pSystem->GetDisplayIntrests(K2L_Compatibility("\xbf\xac\xbe\xd6"), &intrests);
		if (result && !intrests.empty())
		{
			for (it = intrests.begin(); it != intrests.end(); ++it)
			{
				m_pCondition[2]->AddString(it->name);
			}

			m_pCondition[2]->SetLine(1,
				pSystem->GetNameChecked(K2L_Compatibility("\xbf\xac\xbe\xd6")).c_str(), 0, false,
				0);
		}
		else
		{
			m_pCondition[2]->SetLine(1,
				K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4"), 0, false,
				0);
		}
	}
}

void FrQuickStartSettingDlg::UpdateData()
{
	CMatchingSystem* pSystem = CMatchingSystem::Instance();
	if (pSystem)
	{
		std::vector<unsigned char> codes;

		for (int i = 0; i < 0x17; i++)
		{
			if (m_bCheck[i])
				codes.push_back(i);
		}

		if (strcmp(m_pCondition[0]->GetLine(1, false),
				K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4")) != 0)
		{
			unsigned char code = pSystem->GetDisplayIntrestCode(
				K2L_Compatibility("\xb0\xd4\xc0\xd3"), m_pCondition[0]->GetLine(1, false));
			if (code != 0xff)
				codes.push_back(code);
		}

		if (strcmp(m_pCondition[1]->GetLine(1, false),
				K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4")) != 0)
		{
			unsigned char code = pSystem->GetDisplayIntrestCode(
				K2L_Compatibility("\xc3\xeb\xb9\xcc"), m_pCondition[1]->GetLine(1, false));
			if (code != 0xff)
				codes.push_back(code);
		}

		if (strcmp(m_pCondition[2]->GetLine(1, false),
				K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4")) != 0)
		{
			unsigned char code = pSystem->GetDisplayIntrestCode(
				K2L_Compatibility("\xbf\xac\xbe\xd6"), m_pCondition[2]->GetLine(1, false));
			if (code != 0xff)
				codes.push_back(code);
		}

		pSystem->ClearMyIntrests();

		unsigned char count = (unsigned char)codes.size();
		WSendPacket packet((enumClientPacket)0xbe);
		packet.Encode1(count);
		for (int i = 0; i < count; i++)
		{
			packet.Encode1(codes[i]);
			pSystem->SetMyIntrest(codes[i]);
		}
		packet.Encode1(1);
		packet.Send(TO_GAME);
	}
}

CMatchingSystem::CMatchingSystem()
{
}

CMatchingSystem::~CMatchingSystem()
{
}

void CMatchingSystem::Clear()
{
	m_myIntrests.clear();
}

int CMatchingSystem::SetDisplayIntrest(const sIntrest& intrest)
{
	m_intrests.push_back(intrest);
	return 1;
}

const char* CMatchingSystem::GetGroup(unsigned char code)
{
	for (std::vector<sIntrest>::iterator it = m_intrests.begin();
		it != m_intrests.end(); ++it)
	{
		if (it->code == code)
			return it->group;
	}
	return NULL;
}

const sIntrest* CMatchingSystem::GetIntrestInfo(unsigned char code)
{
	const sIntrest* pInfo = NULL;
	for (std::vector<sIntrest>::iterator it = m_intrests.begin();
		it != m_intrests.end(); ++it)
	{
		if (it->code == code)
		{
			pInfo = &*it;
			break;
		}
	}
	return pInfo;
}

unsigned char CMatchingSystem::GetDisplayIntrestCode(const std::string& group,
	const std::string& name)
{
	std::vector<sIntrest>::iterator it;
	unsigned char code = 0xff;
	for (it = m_intrests.begin(); it != m_intrests.end(); ++it)
	{
		if (strcmp(group.c_str(), it->group) == 0 &&
			strcmp(name.c_str(), it->name) == 0)
		{
			code = it->code;
			break;
		}
	}
	return code;
}

int CMatchingSystem::SetMyIntrest(unsigned char code)
{
	m_myIntrests.push_back(code);
	return 1;
}

int CMatchingSystem::CheckIntrestCode(unsigned char code)
{
	return std::find(m_myIntrests.begin(), m_myIntrests.end(), code) !=
			m_myIntrests.end()
		? 1
		: 0;
}

std::string CMatchingSystem::GetNameChecked(const std::string& group)
{
	unsigned char i;
	unsigned char count = (unsigned char)m_myIntrests.size();
	if (count)
	{
		for (i = 0; i < count; i++)
		{
			const sIntrest* pInfo = GetIntrestInfo(m_myIntrests[i]);
			if (pInfo && strcmp(group.c_str(), pInfo->group) == 0)
				return pInfo->name;
		}
	}
	// 983 ad2245: default text also goes through K2L
	return K2L_Compatibility("\xc1\xb6\xb0\xc7\xbc\xb3\xc1\xa4\xbe\xc8\xc7\xd4");
}

int CMatchingSystem::GetDisplayIntrests(const std::string& group,
	std::vector<sIntrest>* out)
{
	int result = 0;
	// HACK: dead /GS buffer; the original has the cookie and shadow copies
	// of both parameters
	char buf[32];
	buf[0] = 0;
	for (std::vector<sIntrest>::iterator it = m_intrests.begin();
		it != m_intrests.end(); ++it)
	{
		if (strcmp(group.c_str(), it->group) == 0)
		{
			out->push_back(*it);
			result = 1;
		}
	}
	return result;
}
