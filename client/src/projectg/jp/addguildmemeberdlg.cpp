// JP 983: unidade 008f98f0–008fa3d0 (FrAddGuildMemeberDlg, o convite para a guilda; ver include/jp/addguildmemeberdlg.h).
// Classe nova do 983 (o addguildmemberdlg.cpp do 645 é vazio); molde: FrAddFriendDlg do 645 (addfrienddlg.cpp). Sessão B112.
#include "minatl.h"
#include "projectg.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "packet.h"
#include "wlocalize.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "fredit.h"
#include "golftask.h"
#include "actor.h"
#include "frwndinl.h"
#include "addguildmemeberdlg.h"
#include <string.h>
#include <algorithm>
#include <ctype.h>

extern Fresh* g_pFresh;

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

IMPLEMENT_OBJECT(FrAddGuildMemeberDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrAddGuildMemeberDlg, FrForm)

ON_FRESH_VI("id", FRCMD_INIT, FrAddGuildMemeberDlg::OnEditIdInit)
ON_FRESH_BI("id", FRCMD_ENTERKEY, FrAddGuildMemeberDlg::OnEditIdEnterKey)
ON_FRESH_VV("id", FRCMD_LBUTTONDOWN, FrAddGuildMemeberDlg::OnEditIdDown)
ON_FRESH_VI("nick", FRCMD_INIT, FrAddGuildMemeberDlg::OnDispNickInit)
ON_FRESH_VV("check", FRCMD_LBUTTONUP, FrAddGuildMemeberDlg::OnCheckBtnUp)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrAddGuildMemeberDlg::OnOKBtnUp)

END_FRESH_MSGMAP()

// 008f9900
void FrAddGuildMemeberDlg::SetNick983(const char* text)
{
	if (m_pDispNick)
		m_pDispNick->SetLine(1, text, 0, false, 0);
}

// 008f9930
void FrAddGuildMemeberDlg::SetId983(const char* text)
{
	if (m_pEditId)
		m_pEditId->SetLine(1, text, 0, false, 0);
}

// 008f9970
void FrAddGuildMemeberDlg::OnDispNickInit(int param)
{
	m_pDispNick = KINDOF_CAST983(FrEdit, param);
	if (m_pDispNick)
	{
		m_pDispNick->m_nFlags.Enable(0x1000);
		m_pDispNick->SetLine(1, K2L_Compatibility("<\xc8\xae\xc0\xce \xbe\xc8\xb5\xca>"), 0, false, 0);
	}
}

// 008f9a10
void FrAddGuildMemeberDlg::OnProc(const float delta)
{
	if (m_bFocused == false)
	{
		if (m_elapsed > 0.3f)
		{
			WndManager()->GetDesktop()->ResetKeyFocus();
			m_pEditId->SetKeyFocus(true);
			m_bFocused = true;
		}

		m_elapsed += delta;
	}
}

// 008f9a70
void FrAddGuildMemeberDlg::OnEditIdInit(int param)
{
	m_pEditId = KINDOF_CAST983(FrEdit, param);
	m_pEditId->m_nFlags.Enable(0x1000);

	if (IS_KINDOF(CGolfTask, AfxGetTask()))
		g_pFresh->GetManager()->SetExclusiveKey(true);
}

// 008f9b30
void FrAddGuildMemeberDlg::OnEditIdDown()
{
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
		g_pFresh->GetManager()->SetExclusiveKey(true);
}

// 008f9b90
FrAddGuildMemeberDlg::FrAddGuildMemeberDlg()
	: m_pEditId(NULL), m_pDispNick(NULL), m_elapsed(0), m_bFocused(false)
{
}

// 008f9d60
void FrAddGuildMemeberDlg::SetCheckResult983(const char* id, const char* target)
{
	if (m_pDispNick)
		m_pDispNick->SetLine(1, id, 0, false, 0);
	m_target = target;
	m_checkedId = id;
}

// 008f9dd0: confere o ID (pacote 7 com 1); vazio ou com mais de 21 letras, avisa
void FrAddGuildMemeberDlg::OnCheckBtnUp()
{
	if (m_pDispNick)
		m_pDispNick->ClearLine();

	if (m_pEditId)
	{
		const char* id = m_pEditId->GetLine(1, false);
		int len = strlen(id);
		if (id && len > 0 && len < 22)
		{
			WSendPacket packet((enumClientPacket)7);
			packet.Encode1(1);
			packet.EncodeStr(std::string(id));
			packet.Send(TO_GAME);
		}
		else if (IS_KINDOF(CGolfTask, AfxGetTask()))
		{
			AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0x23,
				(int)K2L_Compatibility("\xc4\xa3\xb1\xb8\xc0\xc7 \xbe\xc6\xc0\xcc\xb5\xf0\xb8\xa6 \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4"),
					0, 0, 0, 0);
		}
		else
		{
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x23,
				(int)K2L_Compatibility("\xc4\xa3\xb1\xb8\xc0\xc7 \xbe\xc6\xc0\xcc\xb5\xf0\xb8\xa6 \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4"),
					0, 0, 0, 0);
		}
	}
}

// 008f9fa0: o ID digitado tem de ser o conferido (sem caixa); manda o convite (0x26) ao messenger
void FrAddGuildMemeberDlg::OnOKBtnUp()
{
	if (m_pEditId)
	{
		if (strlen(m_pEditId->GetLine(1, false)) == 0)
		{
			if (IS_KINDOF(CGolfTask, AfxGetTask()))
			{
				AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0x23,
					(int)K2L_Compatibility("\xc4\xa3\xb1\xb8\xc0\xc7 \xbe\xc6\xc0\xcc\xb5\xf0\xb8\xa6 \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4"),
					0, 0, 0, 0);
				return;
			}
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x23,
				(int)K2L_Compatibility("\xc4\xa3\xb1\xb8\xc0\xc7 \xbe\xc6\xc0\xcc\xb5\xf0\xb8\xa6 \xb3\xd6\xbe\xee\xc1\xd6\xbc\xbc\xbf\xe4"),
					0, 0, 0, 0);
			return;
		}

		std::string id = m_pEditId->GetLine(1, false);
		std::transform(m_checkedId.begin(), m_checkedId.end(), m_checkedId.begin(), tolower);
		std::transform(id.begin(), id.end(), id.begin(), tolower);
		if (m_checkedId.compare(0, m_checkedId.size(), id.c_str(), id.size()) != 0)
		{
			if (IS_KINDOF(CGolfTask, AfxGetTask()))
				AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0x23,
					(int)K2L_Compatibility("\xba\xb8\xb3\xbb\xb1\xe2 \xc0\xfc\xbf\xa1 \xb9\xde\xb4\xc2 \xc0\xcc\xb0\xa1 \xc8\xae\xbd\xc7\xc8\xf7 \xb8\xc2\xb4\xc2\xc1\xf6\n\xb2\xc0 \\c0xffff0000\\c'\xbe\xc6\xc0\xcc\xb5\xf0\xc8\xae\xc0\xce'\\c0xff000000\\c \xb9\xf6\xc6\xb0\xc0\xbb \xb4\xad\xb7\xaf \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4"),
					0, 0, 0, 0);
			else
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x23,
					(int)K2L_Compatibility("\xba\xb8\xb3\xbb\xb1\xe2 \xc0\xfc\xbf\xa1 \xb9\xde\xb4\xc2 \xc0\xcc\xb0\xa1 \xc8\xae\xbd\xc7\xc8\xf7 \xb8\xc2\xb4\xc2\xc1\xf6\n\xb2\xc0 \\c0xffff0000\\c'\xbe\xc6\xc0\xcc\xb5\xf0\xc8\xae\xc0\xce'\\c0xff000000\\c \xb9\xf6\xc6\xb0\xc0\xbb \xb4\xad\xb7\xaf \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4"),
					0, 0, 0, 0);
			return;
		}

		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN))
		{
			WSendPacket packet(0x26);
			packet.EncodeStr(std::string(Doc()->m_myInfo.info.sID));
			packet.EncodeStr(m_target);
			packet.Encode4(Doc()->m_myInfo.info.dwGuildId);
			packet.Encode1(1);
			packet.Send(TO_MSN);
		}
	}

	OnFreshOkay();
}

// 008fa3c0
bool FrAddGuildMemeberDlg::OnEditIdEnterKey(int param)
{
	OnCheckBtnUp();
	return false;
}
