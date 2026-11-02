// JP 983: unidade 008fa3d0–008fb8e0 (FrGuildManageDlg, a gerência dos pedidos de entrada / convites da guilda; ver
// include/jp/guildmanagedlg.h). Classe nova do 983; moldes: OnMessenger_GuildList* / DrawGuildGroup / OnNoteToFriend /
// OnNoteDlgResult / OnMessenger_FindLBtnDown do FrMessengerDlg do 645 (messengerdlg.cpp da frente A). Sessão B113.
#include "minatl.h"
#include "projectg.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "packet.h"
#include "networksystem.h"
#include "inputmanager.h"
#include "wlocalize.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frlistbox.h"
#include "frbutton.h"
#include "frcontextmenuctrl.h"
#include "frwndinl.h"
#include "notedlg.h"
#include "addguildmemeberdlg.h"
#include "guildmanagedlg.h"
#include <string.h>
#include <vector>

extern Fresh* g_pFresh;

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

// CSharedDoc+0x10fe8 = m_guildJoinList983 (std::vector<sGuildJoinEntry983>; a C le igual no caso 0x116)
#define JOINLIST983() Doc()->m_guildJoinList983

// textos do menu "rmenu_reqJoin" (CP949)
#define TXT_INVITE_ACCEPT983 "\xB1\xE6\xB5\xE5\xC3\xCA\xB4\xEB\xBD\xC2\xB3\xAB" // "guild invite accept"
#define TXT_INVITE_REFUSE983 "\xB1\xE6\xB5\xE5\xC3\xCA\xB4\xEB\xB0\xC5\xC0\xFD" // "guild invite refuse"
#define TXT_JOIN_ACCEPT983 "\xB1\xE6\xB5\xE5\xB0\xA1\xC0\xD4\xBD\xC2\xB3\xAB" // "guild join accept"
#define TXT_JOIN_REFUSE983 "\xB1\xE6\xB5\xE5\xB0\xA1\xC0\xD4\xB0\xC5\xC0\xFD" // "guild join refuse"

IMPLEMENT_OBJECT(FrGuildManageDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrGuildManageDlg, FrForm)

ON_FRESH_VI("rmenu_reqJoin", FRCMD_INIT, FrGuildManageDlg::OnRightMenuInit)
ON_FRESH_VV("rmenu_reqJoin", FRCMD_LBUTTONDOWN, FrGuildManageDlg::OnRightMenuLBtnDown)
ON_FRESH_VI("addmember", FRCMD_INIT, FrGuildManageDlg::OnAddMemberInit)
ON_FRESH_VV("addmember", FRCMD_LBUTTONDOWN, FrGuildManageDlg::OnAddMemberLBtnDown)
ON_FRESH_VI("memberlist", FRCMD_INIT, FrGuildManageDlg::OnMemberListInit)
ON_FRESH_VI("memberlist", FRCMD_OWNERDRAW, FrGuildManageDlg::OnMemberListOwnerDraw)
ON_FRESH_VV("memberlist", FRCMD_LBUTTONDOWN, FrGuildManageDlg::OnMemberListLBtnDown)
ON_FRESH_VV("memberlist", FRCMD_RBUTTONUP, FrGuildManageDlg::OnMemberListRBtnUp)

END_FRESH_MSGMAP()

// 008fa3f0
void FrGuildManageDlg::RequestList983()
{
	WSendPacket packet((enumClientPacket)0xa5);
	packet.Send(TO_GAME);
}

// 008fa3e0
bool FrGuildManageDlg::OnAddMemberDlgResult983(int ret, FrForm* pForm)
{
	m_pAddMemberDlg = NULL;
	return true;
}

// 008fa4b0
FrGuildManageDlg::FrGuildManageDlg()
{
	m_pRightMenu = NULL;
	m_pNoteDlg = NULL;
	m_groupRow[0].state = 0;
	m_groupRow[0].dw0 = 0xffffffff;
	m_groupRow[1].state = 1;
	m_groupRow[1].dw0 = 0xffffffff;
	m_groupRow[2].state = 2;
	m_groupRow[2].dw0 = 0xffffffff;
	m_collapsed[0] = 0;
	m_collapsed[1] = 0;
	m_collapsed[2] = 1;
	m_pGroupIcon[1] = g_pFresh->GetManager()->GetBitmap("MESSENGER", "down_btn_n");
	m_pGroupIcon[0] = g_pFresh->GetManager()->GetBitmap("MESSENGER", "up_btn_n");
	m_pSexIcon[0] = g_pFresh->GetManager()->GetBitmap("ICONS", "i_male");
	m_pSexIcon[1] = g_pFresh->GetManager()->GetBitmap("ICONS", "i_female");
	m_pSexIcon[2] = g_pFresh->GetManager()->GetBitmap("ICONS", "i_male_02");
	m_pSexIcon[3] = g_pFresh->GetManager()->GetBitmap("ICONS", "i_female_02");
	m_pSexIcon[4] = g_pFresh->GetManager()->GetBitmap("ICONS", "i_male_03");
	m_pSexIcon[5] = g_pFresh->GetManager()->GetBitmap("ICONS", "i_female_03");
	m_selectUID = 0xffffffff;
	m_pAddMemberDlg = NULL;
	m_inviteCount = 0;
	m_joinCount = 0;
}

// 008fa6f0
void FrGuildManageDlg::OnRightMenuInit(int param)
{
	m_pRightMenu = KINDOF_CAST983(FrContextMenuCtrl, param);
	if (m_pRightMenu)
	{
		m_pRightMenu->SetVisible(false);
		m_pRightMenu->SetFixed(true);
	}
}

// 008fa770
void FrGuildManageDlg::OnAddMemberInit(int param)
{
	m_pAddMemberBtn = KINDOF_CAST983(FrButton, param);
}

// 008fa7e0
void FrGuildManageDlg::OnMemberListInit(int param)
{
	m_pMemberList = KINDOF_CAST983(FrListBox, param);
	if (m_pMemberList)
	{
		m_pMemberList->UseRightButton(true);
		RequestList983();
	}
}

// 008fa860: linha de grupo (uid -1): "길드초대 대기자 (%d)" / "길드가입 승낙 (%d)" / separador vazio, seta de abrir/fechar
void FrGuildManageDlg::DrawGroup983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;

	sGuildJoinEntry983* pEntry = (sGuildJoinEntry983*)pItem->pData;
	if (pEntry == NULL)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	pGDI->SetAlpha(m_wndAlpha * m_wndAlpha2);

	char text[64];
	switch (pEntry->state)
	{
	case 0:
		strcpy(text, MakeStr(K2L_Compatibility("\xB1\xE6\xB5\xE5\xC3\xCA\xB4\xEB \xB4\xEB\xB1\xE2\xC0\xDA (%d)"), m_inviteCount));
		break;
	case 1:
		strcpy(text, MakeStr(K2L_Compatibility("\xB1\xE6\xB5\xE5\xB0\xA1\xC0\xD4 \xBD\xC2\xB3\xAB (%d)"), m_joinCount));
		break;
	case 2:
		text[0] = '\0';
		break;
	}

	pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGDI->SetTextStyle(1);
	g_pFresh->GetManager()->PrintText(WPoint(pItem->pos.x + 25.0f, pItem->pos.y + 7.0f), 0, text, -1.0f, 0xFFFFFFFF);
	pGDI->SetTextStyle(0);

	if (pEntry->state != 2)
	{
		unsigned char collapsed = (unsigned char)m_collapsed[pEntry->state];
		pGDI->DrawTexture(m_pGroupIcon[collapsed],
			WRect(pItem->pos.x + 5.0f, pItem->pos.y + 5.0f, m_pGroupIcon[collapsed]->Width(),
				m_pGroupIcon[collapsed]->Height()),
			FrALPHA(0xFFFFFFFF, m_wndAlpha * m_wndAlpha2), 0);
	}

	pGDI->SetAlpha(1.0f);
}

// 008fab80
void FrGuildManageDlg::OnMemberListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;

	sGuildJoinEntry983* pEntry = (sGuildJoinEntry983*)pItem->pData;
	if (pEntry == NULL)
		return;

	if (pEntry->dw0 == 0xFFFFFFFF)
	{
		DrawGroup983(param);
		return;
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	pGDI->SetAlpha(m_wndAlpha * m_wndAlpha2);

	const Bitmap* pSex = m_pSexIcon[pEntry->sexIcon & 7];
	WRect rect(pItem->pos.x + 10.0f, pItem->pos.y, pSex->Width(), pSex->Height());

	if (pEntry->dw0 == m_selectUID)
	{
		pGDI->Box(WRect(rect.x, rect.y + 3.0f, m_pMemberList->GetItemWidth() - 25, m_pMemberList->GetItemHeight()),
			((int)(m_wndAlpha * 0.59765625 * m_wndAlpha2 * 255.0) << 24) | 0x9B9DFF, 0, 0.0f);
	}

	pGDI->DrawTexture(pSex, rect, ((int)(m_wndAlpha * m_wndAlpha2 * 80.0) << 24) | 0xFFFFFF, 0);

	g_pFresh->GetManager()->PrintText(WPoint(pItem->pos.x + 30.0f, pItem->pos.y + 7.0f), 0,
		(const char*)pEntry + 0x1e, -1.0f, 0xFFFFFFFF); // o 983 não volta o alfa a 1 aqui
}

// 008fadd0 = CreateForm<FrAddGuildMemeberDlg> (molde, fora de linha)
// 008faea0
void FrGuildManageDlg::OnAddMemberLBtnDown()
{
	m_pRightMenu->SetVisible(false);

	if (m_pAddMemberDlg != NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	m_pAddMemberDlg = CreateForm<FrAddGuildMemeberDlg>(g_pFresh->GetManager(), this, "addguildmember", this);
	// "길드원으로 초대하고 싶은 유저의 아이디를\n입력하면 접속하지 않은 유저에게도 길드원 초대\n요청이 가능합니다."
	m_pAddMemberDlg->SetMessage(K2L_Compatibility(
		"\xB1\xE6\xB5\xE5\xBF\xF8\xC0\xB8\xB7\xCE \xC3\xCA\xB4\xEB\xC7\xCF\xB0\xED \xBD\xCD\xC0\xBA \xC0\xAF\xC0\xFA\xC0\xC7 "
		"\xBE\xC6\xC0\xCC\xB5\xF0\xB8\xA6\n\xC0\xD4\xB7\xC2\xC7\xCF\xB8\xE9 \xC1\xA2\xBC\xD3\xC7\xCF\xC1\xF6 \xBE\xCA\xC0\xBA "
		"\xC0\xAF\xC0\xFA\xBF\xA1\xB0\xD4\xB5\xB5 \xB1\xE6\xB5\xE5\xBF\xF8 \xC3\xCA\xB4\xEB\n\xBF\xE4\xC3\xBB\xC0\xCC \xB0\xA1"
		"\xB4\xC9\xC7\xD5\xB4\xCF\xB4\xD9."), false);
	m_pAddMemberDlg->SetFixed(true);
	m_pAddMemberDlg->Open((FRESH_PFN_RESULT)&FrGuildManageDlg::OnAddMemberDlgResult983, pt, 1);

	WRect rect = m_pAddMemberDlg->GetRect();
	m_pAddMemberDlg->Adjust(rect);
	m_pAddMemberDlg->MoveWindow(WPoint(rect.x, rect.y));
}

// 008fafc0: grupo 0 + pedidos (state 2), separador, grupo 1 + aceitos (state 3); conta os dois
void FrGuildManageDlg::RefreshList983()
{
	m_pMemberList->ClearItem();

	std::vector<sGuildJoinEntry983>& list = JOINLIST983();

	m_pMemberList->AddItem(&m_groupRow[0]);
	if (m_collapsed[0] == 0)
	{
		for (std::vector<sGuildJoinEntry983>::iterator it = list.begin(); it != list.end(); ++it)
		{
			if (it->state == 2)
				m_pMemberList->AddItem(&*it);
		}
	}

	m_pMemberList->AddItem(&m_groupRow[2]);
	m_pMemberList->AddItem(&m_groupRow[1]);
	if (m_collapsed[1] == 0)
	{
		for (std::vector<sGuildJoinEntry983>::iterator it = list.begin(); it != list.end(); ++it)
		{
			if (it->state == 3)
				m_pMemberList->AddItem(&*it);
		}
	}

	m_inviteCount = 0;
	m_joinCount = 0;
	for (std::vector<sGuildJoinEntry983>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (it->state == 2)
			m_inviteCount++;
		else if (it->state == 3)
			m_joinCount++;
	}
}

// 008fb0d0 (não usa o this)
sGuildJoinEntry983* FrGuildManageDlg::FindJoinEntry983(unsigned long uid)
{
	std::vector<sGuildJoinEntry983>& list = JOINLIST983();
	for (std::vector<sGuildJoinEntry983>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (it->dw0 == uid)
			return &*it;
	}
	return NULL;
}

// 008fb120: o 983 procura pelo m_selectUID, não pelo argumento
void FrGuildManageDlg::SendJoinAnswer983(unsigned long uid, unsigned char accept)
{
	sGuildJoinEntry983* pEntry = FindJoinEntry983(m_selectUID);
	if (pEntry == NULL)
		return;

	WSendPacket packet((enumClientPacket)0xa7);
	packet.Encode4(pEntry->dw0);
	packet.Encode4(pEntry->dw4);
	packet.Encode1(accept);
	packet.Send(TO_GAME);
}

// 008fb1c0: clique na linha de grupo abre/fecha o grupo
void FrGuildManageDlg::OnMemberListLBtnDown()
{
	FindNextTopFocus(false);
	m_pRightMenu->SetVisible(false);

	FrListItem* pItem = m_pMemberList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	sGuildJoinEntry983* pEntry = (sGuildJoinEntry983*)pItem->pData;
	if (pEntry == NULL)
		return;

	m_selectUID = pEntry->dw0;
	if (pEntry->dw0 != 0xFFFFFFFF)
		return;

	m_collapsed[pEntry->state] = m_collapsed[pEntry->state] == 0;
	RefreshList983();
}

// 008fb220: o 983 junta o fim do default com o caso 3
void FrGuildManageDlg::OnMemberListRBtnUp()
{
	OnMemberListLBtnDown();

	FrListItem* pItem = m_pMemberList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	sGuildJoinEntry983* pEntry = (sGuildJoinEntry983*)pItem->pData;
	if (pEntry == NULL || pEntry->dw0 == 0xFFFFFFFF)
		return;

	m_pRightMenu->SetVisible(true);
	WPoint pt(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
	m_pRightMenu->MoveWindow(pt);

	switch (pEntry->state)
	{
	case 2:
		m_pRightMenu->UnableMenuItem(K2L_Compatibility(TXT_INVITE_ACCEPT983));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility(TXT_INVITE_REFUSE983));
		break;
	default:
		m_pRightMenu->UnableMenuItem(K2L_Compatibility(TXT_INVITE_ACCEPT983));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility(TXT_INVITE_REFUSE983));
		// segue no 3
	case 3:
		m_pRightMenu->UnableMenuItem(K2L_Compatibility(TXT_JOIN_ACCEPT983));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility(TXT_JOIN_REFUSE983));
		break;
	}
}

// 008fb340: recusa o convite (0x26 ao mensageiro, com o meu ID, o ID do outro e a minha guilda)
void FrGuildManageDlg::SendInviteAnswer983(unsigned long uid, unsigned char accept)
{
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN))
		return;

	sGuildJoinEntry983* pEntry = FindJoinEntry983(m_selectUID);
	if (pEntry == NULL)
		return;

	WSendPacket packet(0x26);
	packet.EncodeStr(std::string(Doc()->m_myInfo.info.sID));
	packet.EncodeStr(std::string((const char*)pEntry + 8));
	packet.Encode4(Doc()->m_myInfo.info.dwGuildId);
	packet.Encode1(accept);
	packet.Send(TO_MSN);
}

// 008fb4b0: sem a linha selecionada devolve false e mantém m_pNoteDlg
bool FrGuildManageDlg::OnNoteDlgResult983(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		sGuildJoinEntry983* pEntry = FindJoinEntry983(m_selectUID);
		if (pEntry == NULL)
			return false;

		WSendPacket packet((enumClientPacket)0x3C);
		packet.Encode2(0x111);
		packet.Encode4(pEntry->dw0);
		packet.EncodeStr(std::string(m_pNoteDlg->GetNote()));
		packet.Encode1(0);
		packet.Send(TO_GAME);
	}

	m_pNoteDlg = NULL;

	return true;
}

// 008fb5d0
void FrGuildManageDlg::OnNoteToMember983()
{
	sGuildJoinEntry983* pEntry = FindJoinEntry983(m_selectUID);
	if (pEntry == NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	if (Doc()->m_myInfo.stat.i64Pang < 10)
	{
		// "쪽지를 보내기 위해서는 %d팡이 필요합니다."
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", this);
		pForm->SetMessage(
			MakeStr(K2L_Compatibility("\xC2\xCA\xC1\xF6\xB8\xA6 \xBA\xB8\xB3\xBB\xB1\xE2 \xC0\xA7\xC7\xD8\xBC\xAD\xB4\xC2 %d\xC6\xCE\xC0\xCC \xC7\xCA\xBF\xE4\xC7\xD5\xB4\xCF\xB4\xD9."),
				10),
			false);
		pForm->SetTimeLimit(5.0f);
		pForm->SetFixed(true);
		pForm->Open((FRESH_PFN_RESULT)0, pt, 3);
		return;
	}

	if (m_pNoteDlg != NULL)
		return;

	m_pNoteDlg = CreateForm<FrNoteDlg>(g_pFresh->GetManager(), this, "note", this);
	m_pNoteDlg->SetNick((const char*)pEntry + 0x1e, 0);
	m_pNoteDlg->SetFixed(true);
	m_pNoteDlg->Open((FRESH_PFN_RESULT)&FrGuildManageDlg::OnNoteDlgResult983, pt, 1);

	WRect rect = m_pNoteDlg->GetRect();
	m_pNoteDlg->Adjust(rect);
	m_pNoteDlg->MoveWindow(WPoint(rect.x, rect.y));
}

// 008fb790
void FrGuildManageDlg::OnRightMenuLBtnDown()
{
	m_pRightMenu->SetVisible(false);
	FindNextTopFocus(false);

	const char* text = m_pRightMenu->GetSelectMenuText();

	if (strcmp(text, K2L_Compatibility(TXT_JOIN_ACCEPT983)) == 0)
	{
		SendJoinAnswer983(m_selectUID, 1);
		return;
	}
	if (strcmp(text, K2L_Compatibility(TXT_JOIN_REFUSE983)) == 0)
	{
		SendJoinAnswer983(m_selectUID, 0);
		return;
	}
	if (strcmp(text, K2L_Compatibility(TXT_INVITE_REFUSE983)) == 0)
	{
		SendInviteAnswer983(m_selectUID, 0);
		return;
	}
	if (strcmp(text, K2L_Compatibility("\xC2\xCA\xC1\xF6\xBA\xB8\xB3\xBB\xB1\xE2")) == 0) // "send note"
	{
		OnNoteToMember983();
		return;
	}
	if (strcmp(text, K2L_Compatibility("\xC1\xA4\xBA\xB8\xBA\xB8\xB1\xE2")) == 0) // "view info": sem ação no 983
	{
	}
}
