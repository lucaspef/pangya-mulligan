#include "minatl.h"
#include "detailedroominfodlg.h"
#include "passworddlg.h"
#include "user_info.h"
#include "intrusion.h"
#include "../shared/localize.h"
#include "netresourcemanager.h"
#include "projectg.h"
#include "fredit.h"
#include "commonutil.h"
#include "jp/gamestringmanager.h"
#include "jp/grandprix983.h"
#include "jp/levelnametable.h"
#include "../shared/itemmanager.h"
#include "vc10_layout.h"
#include <vector>

// 00bfb1d0: o FrEdit::AddText do 983 tem 5 argumentos (mesmo emprestimo do grandprixlimitdlg.cpp)
class FrEditJp983 : public FrEdit
{
public:
	// A312: inline -> FrEdit::AddText de 5 argumentos (fredit.h); antes sem corpo = stub no exe (texto nunca aparecia)
	void AddText983(const char* text, bool a, bool b, unsigned long c, unsigned long d) { AddText(text, a, b, c, d); }
};

static CLevelNameTable983 s_levelNames; // 00f03184

int float2int(float f);

extern Fresh* g_pFresh;

static bool CompareIdDesc(const void* left, const void* right)
{
	std::map<unsigned long, sBriefUserInfo>& users = Doc()->m_briefUserInfoMap;
	std::map<unsigned long, sBriefUserInfo>::iterator ia =
		users.find(((sRoomUserInfo*)left)->uid);
	std::map<unsigned long, sBriefUserInfo>::iterator ib =
		users.find(((sRoomUserInfo*)right)->uid);
	if (ia == Doc()->m_briefUserInfoMap.end())
		return true;
	if (ib == Doc()->m_briefUserInfoMap.end())
		return true;
	bool result = strcmp(ia->second.sNick, ib->second.sNick) < 0;
	return result;
}
static bool CompareStatusDesc(const void* left, const void* right)
{
	sRoomUserInfo* a = (sRoomUserInfo*)left;
	sRoomUserInfo* b = (sRoomUserInfo*)right;
	if (a->hole > b->hole)
		return true;
	if (a->hole < b->hole)
		return false;
	return CompareIdDesc(left, right);
}
static bool CompareLevelAsc(const void* left, const void* right)
{
	const sRoomUserInfo* a = (const sRoomUserInfo*)left;
	const sRoomUserInfo* b = (const sRoomUserInfo*)right;
	if (a->level > b->level)
		return false;
	if (a->level < b->level)
		return true;
	return CompareIdDesc(left, right);
}

static bool CompareIdAsc(const void* left, const void* right)
{
	std::map<unsigned long, sBriefUserInfo>& users = Doc()->m_briefUserInfoMap;
	std::map<unsigned long, sBriefUserInfo>::iterator ia =
		users.find(((sRoomUserInfo*)left)->uid);
	std::map<unsigned long, sBriefUserInfo>::iterator ib =
		users.find(((sRoomUserInfo*)right)->uid);
	if (ia == Doc()->m_briefUserInfoMap.end())
		return true;
	if (ib == Doc()->m_briefUserInfoMap.end())
		return true;
	bool result = strcmp(ia->second.sNick, ib->second.sNick) >= 0;
	return result;
}
static bool CompareStatusAsc(const void* left, const void* right)
{
	sRoomUserInfo* a = (sRoomUserInfo*)left;
	sRoomUserInfo* b = (sRoomUserInfo*)right;
	if (a->hole > b->hole)
		return false;
	if (a->hole < b->hole)
		return true;
	return CompareIdDesc(left, right);
}
static bool CompareLevelDesc(const void* left, const void* right)
{
	// HACK
	if (false)
		new FrDtRoomDlg;
	const sRoomUserInfo* a = (const sRoomUserInfo*)left;
	const sRoomUserInfo* b = (const sRoomUserInfo*)right;
	if (a->level > b->level)
		return true;
	if (a->level < b->level)
		return false;
	return CompareIdDesc(left, right);
}
IMPLEMENT_OBJECT(FrDtRoomDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrDtRoomDlg, FrForm)

ON_FRESH_VI("gamedesc", FRCMD_INIT, FrDtRoomDlg::OnGameDescInit)
ON_FRESH_VI("join", FRCMD_INIT, FrDtRoomDlg::OnJoinBtnInit)
ON_FRESH_VV("join", FRCMD_LBUTTONUP, FrDtRoomDlg::OnJoinBtnUp)
ON_FRESH_VI("userinfo", FRCMD_INIT, FrDtRoomDlg::OnUserInfoBtnInit)
ON_FRESH_VV("userinfo", FRCMD_LBUTTONUP, FrDtRoomDlg::OnUserInfoBtnUp)
ON_FRESH_VI("level", FRCMD_INIT, FrDtRoomDlg::OnLevelHdrInit)
ON_FRESH_VV("level", FRCMD_LBUTTONDOWN, FrDtRoomDlg::OnLevelHdrDown)
ON_FRESH_VI("id", FRCMD_INIT, FrDtRoomDlg::OnIdHdrInit)
ON_FRESH_VV("id", FRCMD_LBUTTONDOWN, FrDtRoomDlg::OnIdHdrDown)
ON_FRESH_VI("status", FRCMD_INIT, FrDtRoomDlg::OnStatusHdrInit)
ON_FRESH_VV("status", FRCMD_LBUTTONDOWN, FrDtRoomDlg::OnStatusHdrDown)
ON_FRESH_VI("userlist", FRCMD_INIT, FrDtRoomDlg::OnUserListInit)
ON_FRESH_VI("userlist", FRCMD_OWNERDRAW, FrDtRoomDlg::OnUserListOwnerDraw)
ON_FRESH_VV("userlist", FRCMD_LBUTTONDOWN, FrDtRoomDlg::OnUserListLBtnDown)
ON_FRESH_VV("userlist", FRCMD_RBUTTONUP, FrDtRoomDlg::OnUserListRBtnUp)

END_FRESH_MSGMAP()

FrDtRoomDlg::FrDtRoomDlg()
	: m_pGameDesc(NULL),
	  m_pJoinBtn(NULL),
	  m_pUserInfoBtn(NULL),
	  m_pGalleryBtn(NULL),
	  m_pUserList(NULL)
{
	m_pSelUser = NULL;
	m_pIdHdr = NULL;
	m_pLevelHdr = NULL;
	m_pStatusHdr = NULL;
	m_sortHeader = eHEADER_ID;
	m_bGrandPrix983 = false;
	m_pGrandPrix983 = NULL;
	C_ASSERT_VC10(sizeof(FrDtRoomDlg) == 0x260 && offsetof(FrDtRoomDlg, m_pGrandPrix983) == 0x250); // 983: push 0x260 em 00ad0383
}

// 00ad0420
bool FrDtRoomDlg::OnInit()
{
	m_sortHeader = (eHeader)3;
	OnHeaderLBtnDown(eHEADER_LEVEL);
	if (m_bGrandPrix983)
	{
		// Grand Prix: sem botoes de entrar/info; a descricao ocupa tambem a area do botao
		m_pJoinBtn->SetVisible(false);
		m_pUserInfoBtn->SetVisible(false);
		WRect rc = m_pGameDesc->GetRect();
		rc.h += m_pJoinBtn->GetRect().h;
		m_pGameDesc->SetRect(rc);
		ShowGrandPrix983();
		return true;
	}
	UpdateGameDesc983();
	return true;
}

// 00ace830
void FrDtRoomDlg::SetGrandPrix983(unsigned long typeId)
{
	m_pGrandPrix983 = (sGPInfo983*)Doc()->m_itemManager.FindGrandPrixData983(typeId);
	if (m_pGrandPrix983)
		m_bGrandPrix983 = true;
}

void FrDtRoomDlg::SetRoomInfo(const sRoomInfo& info)
{
	m_roomInfo = info;
}

void FrDtRoomDlg::OnGameDescInit(int param)
{
	m_pGameDesc = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrDtRoomDlg::UpdateGameDesc983()
{
	char text[256];
	const char* typeName;
	switch (Doc()->m_roomDetail.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case GAME_TYPE_HOLEINONE983: // 983: 4, 5, 6, 9, 10, 11 e 18 (IsMassGame embutido)
	case GAME_TYPE_USEMAX:
	{
		IFF_STRUCT::sMatch* match =
			ItemManager()->FindMatch(Doc()->m_roomDetail.matchTypeId);
		if (match)
			typeName = match->Name;
		else
			typeName = Doc()->m_gameTypeInfo[Doc()->m_roomDetail.gameType].name;
	}
	break;
	default:
		typeName = Doc()->m_gameTypeInfo[Doc()->m_roomDetail.gameType].name;
		break;
	}
	sprintf(text, K2L_Compatibility("\260\324\300\323\305\270\300\324 : %s%s"), typeName,
		Doc()->m_roomDetail.gameType == 5 ? K2L_Compatibility("\306\300\300\374") : "");
	m_pGameDesc->SetLine(1, text, 0, false, 0);
	if (Doc()->m_roomDetail.map == 255)
	{
		const char* source = K2L_Compatibility("\304\332\275\272 : RANDOM");
		int i = 0;
		do
		{
			text[i] = source[i];
		} while (source[i++]);
	}
	else
	{
		unsigned char map = Doc()->m_roomDetail.map;
		IFF_STRUCT::sCourse* course = Doc()->m_itemManager.FindCourse(
			(map <= 127 || map == 253 ? map : map - 128) | 0x28000000);
		if (course)
			sprintf(text, K2L_Compatibility("\304\332\275\272 : %s"), course->c.Name);
		else
		{
			const char* source = K2L_Compatibility("\304\332\275\272 : \270\360\270\247");
			int i = 0;
			do
			{
				text[i] = source[i];
			} while (source[i++]);
		}
	}
	m_pGameDesc->SetLine(2, text, 0, false, 0);
	if (Doc()->m_roomDetail.holeOrder == 2)
		sprintf(text,
			K2L_Compatibility("\275\303\300\333\310\246\040\072\040\267\243\264\375\040\310\246\040\057\040\310\246\040\274\366\040\072\040\045\144\040\310\246"),
			Doc()->m_roomDetail.holes);
	else if (Doc()->m_roomDetail.holeOrder == 3)
		sprintf(text,
			K2L_Compatibility("\275\303\300\333\310\246\040\072\040\274\305\307\303\040\310\246\040\057\040\310\246\040\274\366\040\072\040\045\144\040\310\246"),
			Doc()->m_roomDetail.holes);
	else
		sprintf(text,
			K2L_Compatibility("\275\303\300\333\310\246 : %d \310\246 / \310\246 \274\366 : %d \310\246"),
			Doc()->m_roomDetail.holeOrder ? 10 : 1, Doc()->m_roomDetail.holes);
	m_pGameDesc->SetLine(3, text, 0, false, 0);
	if (Doc()->m_roomDetail.gameType == 2)
		text[0] = 0;
	else
	{
		switch (Doc()->m_roomDetail.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case GAME_TYPE_HOLEINONE983: // 983: 4, 5, 6, 9, 10, 11 e 18 (IsMassGame embutido)
		case GAME_TYPE_USEMAX:
			if (Doc()->m_roomDetail.gameType == 10)
				goto strokeTime;
			sprintf(text,
				K2L_Compatibility("\260\324\300\323\301\246\307\321\275\303\260\243 : %d \272\320"),
				Doc()->m_roomDetail.gameTime / 60000);
			break;
		default:
strokeTime:
			if (!Doc()->m_roomDetail.gameTime)
			{
				const char* source =
					K2L_Compatibility("\305\270\261\270\301\246\307\321\275\303\260\243 : \276\370\300\275");
				int i = 0;
				do
				{
					text[i] = source[i];
				} while (source[i++]);
			}
			else
				sprintf(text,
					K2L_Compatibility("\305\270\261\270\301\246\307\321\275\303\260\243 : %d \303\312"),
					Doc()->m_roomDetail.gameTime / 1000);
			break;
		}
	}
	m_pGameDesc->SetLine(4, text, 0, false, 0);
}

void FrDtRoomDlg::OnJoinBtnInit(int param)
{
	m_pJoinBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pJoinBtn && (Doc()->m_curChannel.Type & 0x80))
		m_pJoinBtn->Enable(false);
}

void FrDtRoomDlg::OnJoinBtnUp()
{
	int developer = (Doc()->m_myInfo.info.dwIdentity & 14) == 14;
	if (developer)
	{
		OnGalleryBtnUp();
		return;
	}
	// 983 ad0535: com Intrusion ligado, encerra o jogo de intrusao corrente antes de entrar
	if (IsLocalContent(S3_INTRUSION))
		CIntrusion::Instance()->FinishGame();
	if (m_roomInfo.bAvailable)
	{
		if (!Doc()->m_myInfo.info.IsIdentity(4))
		{
			// 983 ad0580: um unico envio da mensagem de recusa (ad0585) para os tres avisos
			const char* denyMsg = NULL;
			if (m_roomInfo.nUserNum == m_roomInfo.nUserLimit)
				denyMsg = ("\301\244\277\370\300\314 \303\312\260\372 \265\307\276\372\275\300\264\317\264\331");
			else
			{
				if (m_roomInfo.gameType == 5)
				{
					unsigned long uid = MyGuid(false);
					std::map<unsigned long, sBriefUserInfo>::iterator it =
						Doc()->m_briefUserInfoMap.find(uid);
					if (it != Doc()->m_briefUserInfoMap.end() &&
						it->second.gender > 1)
						denyMsg = ("\260\255\301\246 \301\276\267\341\300\262\300\314 \263\364\300\272 \300\257\300\372\264\302 30\300\316 \306\300\300\374 \260\324\300\323\300\273 \307\322 \274\366 \276\370\275\300\264\317\264\331.");
				}
				// 983 ad061a: tipos 4/5 exigem o byte +0x91d do CSharedDoc (rookie F)
				if (!denyMsg && (m_roomInfo.gameType == 4 || m_roomInfo.gameType == 5) &&
					Doc()->m_myInfo.stat.Level == 0)
					denyMsg = ("\xb7\xe7\xc5\xb0\x46 \xb7\xb9\xba\xa7\xc0\xba 30\xc0\xce\xb4\xeb\xc0\xfc \xb9\xd7 30\xc0\xce\xc6\xc0\xc0\xfc \xb0\xd4\xc0\xd3\xc0\xbb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			}
			if (denyMsg)
			{
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(NULL, M983(0x23), (int)K2L_Compatibility(denyMsg), 0, 0, 0, 0);
				return;
			}
		}
		if (m_roomInfo.gameType == 6 && !Doc()->m_myInfo.info.IsIdentity(0x14))
		{
			std::string text;
			unsigned long guild = Doc()->m_myInfo.info.dwGuildId;
			bool denied = false;
			if (!guild)
			{
				text =
					K2L_Compatibility("\261\346\265\345\277\241 \260\241\300\324\307\317\274\305\276\337 \261\346\265\345\264\353\300\374\300\314 \260\241\264\311\307\325\264\317\264\331.");
				denied = true;
			}
			if (m_roomInfo.GuildInfo.nGuildID[0] != guild &&
				m_roomInfo.GuildInfo.nGuildID[0] &&
				m_roomInfo.GuildInfo.nGuildID[1] != guild &&
				m_roomInfo.GuildInfo.nGuildID[1])
			{
				text =
					K2L_Compatibility("\305\270\261\346\265\345\300\307 \264\353\300\374\300\324\264\317\264\331.");
				denied = true;
			}
			if (denied)
			{
				// 983 00ad04e0: o texto ja convertido passa de novo por K2L no envio
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(NULL, M983(0x23), (int)K2L_Compatibility(text.c_str()), 0, 0, 0, 0);
				return;
			}
		}
		// 983 00ad04e0: sala fechada (sem GM) pede a senha; aberta entra por Intrusion ou pelo pacote 9
		if (!m_roomInfo.bPublic && !Doc()->m_myInfo.info.IsIdentity(0x14))
		{
			FrPasswordDlg* dialog = CreateForm<FrPasswordDlg>(
				g_pFresh->GetManager(), this, "password", NULL);
			dialog->Open((FRESH_PFN_RESULT)&FrDtRoomDlg::OnPasswordDlgResult,
				3);
		}
		else if (IsLocalContent(S3_INTRUSION) && m_roomInfo.bIntrusion == true &&
			!Doc()->m_myInfo.info.IsIdentity(0x14))
		{
			CIntrusion::Instance()->DoJoinRoom(m_roomInfo.roomGuid);
			Close((eFormRet)1, true);
		}
		else
		{
			if (m_pJoinBtn)
				m_pJoinBtn->Enable(false);
			{
				WSendPacket packet((enumClientPacket)9);
				packet.Encode2(m_roomInfo.roomGuid);
				packet.EncodeStr(std::string(""));
				packet.Send(TO_GAME);
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(NULL, M983(0x16), 0, 0, 0, 0, 0);
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
			}
			Close((eFormRet)1, true);
		}
	}
	else
	{
		if (m_roomInfo.bIntrusion)
			CIntrusion::Instance()->DoJoinRoom(m_roomInfo.roomGuid);
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\260\324\300\323 \301\370\307\340\301\337\300\316 \271\346\300\324\264\317\264\331"),
			0, 0, 0, 0);
	}
}

void FrDtRoomDlg::OnGalleryBtnInit(int param)
{
	m_pGalleryBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDtRoomDlg::OnGalleryBtnUp()
{
	if (m_roomInfo.bAvailable)
	{
		if (m_roomInfo.nGalleryNum != m_roomInfo.nGalleryLimit ||
			Doc()->m_myInfo.info.IsIdentity(4))
		{
			if (m_roomInfo.bPublic || Doc()->m_myInfo.info.IsIdentity(4))
			{
				if (m_pGalleryBtn)
					m_pGalleryBtn->Enable(false);
				{
					WSendPacket packet((enumClientPacket)62);
					packet.Encode2(m_roomInfo.roomGuid);
					packet.EncodeStr(std::string(""));
					packet.Send(TO_GAME);
					AfxGetTask()->GetActor("Lobby")
						<< MsgObject(NULL, M983(0x16), 0, 0, 0, 0, 0);
					AfxGetTask()->GetActor("Lobby")
						<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
					Close((eFormRet)1, true);
				}
			}
			else
			{
				FrPasswordDlg* dialog = CreateForm<FrPasswordDlg>(
					g_pFresh->GetManager(), this, "password", NULL);
				dialog->Open(
					(FRESH_PFN_RESULT)&FrDtRoomDlg::OnPasswordDlgResult, 3);
			}
		}
		else
		{
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\301\244\277\370\300\314 \303\312\260\372 \265\307\276\372\275\300\264\317\264\331"),
				0, 0, 0, 0);
		}
	}
	else
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\260\324\300\323 \301\370\307\340\301\337\300\316 \271\346\300\324\264\317\264\331"),
			0, 0, 0, 0);
	}
}

void FrDtRoomDlg::EnableJoinBtn(bool enable)
{
	if (m_pJoinBtn)
		m_pJoinBtn->Enable(enable); // 983 00ace410: sem o teste do canal (ficou no OnJoinBtnInit)
}

void FrDtRoomDlg::OnProc(const float dt)
{
	std::list<void*>::iterator it;
	for (it = m_delList.begin(); it != m_delList.end();)
	{
		if (m_pUserList)
			m_pUserList->DelItem(*it);
		it = m_delList.erase(it);
	}
}

void FrDtRoomDlg::OnUserInfoBtnInit(int param)
{
	m_pUserInfoBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pUserInfoBtn->Enable(false);
}

void FrDtRoomDlg::OnUserInfoBtnUp()
{
	if (CUserInfo::Instance())
	{
		FrListItem* item = m_pUserList->GetSelected();
		if (item)
		{
			sRoomUserInfo* user = (sRoomUserInfo*)item->pData;
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				Doc()->m_briefUserInfoMap.find(user->uid);
			if (it != Doc()->m_briefUserInfoMap.end())
				CUserInfo::Instance()->SetInfo(it->second.dwUid, user->uid,
					true, true, false, false, it->second.sNick);
		}
	}
}

void FrDtRoomDlg::OnLevelHdrInit(int param)
{
	m_pLevelHdr = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDtRoomDlg::OnLevelHdrDown()
{
	OnHeaderLBtnDown(eHEADER_LEVEL);
}

void FrDtRoomDlg::OnIdHdrInit(int param)
{
	m_pIdHdr = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDtRoomDlg::OnIdHdrDown()
{
	OnHeaderLBtnDown(eHEADER_ID);
}

void FrDtRoomDlg::OnStatusHdrInit(int param)
{
	m_pStatusHdr = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrDtRoomDlg::OnStatusHdrDown()
{
	OnHeaderLBtnDown(eHEADER_STATUS);
}

void FrDtRoomDlg::OnHeaderLBtnDown(eHeader header)
{
	if (m_pUserList)
	{
		if (header == m_sortHeader)
		{
			m_sortHeader = (eHeader)(header + 3);
			switch (header)
			{
			case eHEADER_LEVEL:
				m_pUserList->SortItem(CompareLevelAsc);
				break;
			case eHEADER_ID:
				m_pUserList->SortItem(CompareIdAsc);
				break;
			case eHEADER_STATUS:
				m_pUserList->SortItem(CompareStatusAsc);
				break;
			}
		}
		else
		{
			m_sortHeader = header;
			switch (header)
			{
			case eHEADER_LEVEL:
				m_pUserList->SortItem(CompareLevelDesc);
				break;
			case eHEADER_ID:
				m_pUserList->SortItem(CompareIdDesc);
				break;
			case eHEADER_STATUS:
				m_pUserList->SortItem(CompareStatusDesc);
				break;
			}
		}
	}
}

void FrDtRoomDlg::OnUserListInit(int param)
{
	m_pUserList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pUserList->UseRightButton(true);
	m_pIcon[0] = m_pUserList->GetBitmap("i_male");
	m_pIcon[1] = m_pUserList->GetBitmap("i_female");
	m_pIcon[2] = m_pUserList->GetBitmap("i_male_02");
	m_pIcon[3] = m_pUserList->GetBitmap("i_female_02");
	m_pIcon[4] = m_pUserList->GetBitmap("i_male_03");
	m_pIcon[5] = m_pUserList->GetBitmap("i_female_03");
	m_pIcon[6] = m_pUserList->GetBitmap("i_male_manner");
	m_pIcon[7] = m_pUserList->GetBitmap("i_female_manner");
	m_pIcon[8] = m_pUserList->GetBitmap("i_male_angel");
	m_pIcon[9] = m_pUserList->GetBitmap("i_female_angel");
	m_pUserList->ClearItem();
	CSharedDoc* doc;
	std::list<sRoomUserInfo>& users = Doc()->m_roomUserList;
	std::list<sRoomUserInfo>::iterator it = users.begin();
	for (; it != users.end(); it++)
	{
		doc = Doc();
		if (!doc->m_myInfo.info.IsIdentity(4) && ((*it).capability & 0x14))
		{
			std::map<unsigned long, sBriefUserInfo>::iterator found =
				doc->m_briefUserInfoMap.find((*it).uid);
			if (found != doc->m_briefUserInfoMap.end())
			{
				if (!(sBriefUserInfo(found->second).state & 1))
					continue;
			}
		}
		m_pUserList->AddItem(&*it);
	}
}

void FrDtRoomDlg::OnUserListOwnerDraw(int param)
{
	if (!param)
		return;
	FrListItem* item = (FrListItem*)param;
	sRoomUserInfo*& user = (sRoomUserInfo*&)param;
	user = (sRoomUserInfo*)item->pData;
	if (!user)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	std::map<unsigned long, sBriefUserInfo>::iterator it =
		Doc()->m_briefUserInfoMap.find(user->uid);
	if (it == Doc()->m_briefUserInfoMap.end())
	{
		m_delList.push_back(item->pData);
		return;
	}
	sBriefUserInfo& info = it->second;
	unsigned long nickColor =
		((unsigned char)info.dwIdentity & 0x14) ? 0x50ffffff : 0xffffffff;
	if (item->underCursor)
	{
		gdi->SetTextColor(0xffffffff, 0xff808080);
		gdi->SetTextStyle(2);
	}
	else
	{
		if (info.bSleep)
			gdi->SetTextColor(0xff808080, 0xffffffff);
		else
			gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(0);
	}
	const Bitmap* bitmap;
	if (info.angelicWings)
		bitmap = m_pIcon[8 + (info.gender % 2)];
	else if (info.manner)
		bitmap = m_pIcon[6 + (info.gender % 2)];
	else
		bitmap = m_pIcon[info.gender];
	if (!bitmap)
		return;
	if (user == m_pSelUser)
	{
		_WRECT selection = { item->pos.x, item->pos.y,
			(float)m_pUserList->GetItemWidth(),
			(float)m_pUserList->GetItemHeight() + 2.0f };
		gdi->Box(selection, 0x999b9dff, 0, 0.0f);
	}
	gdi->DrawTexture(bitmap,
		WRect(item->pos.x + 58.0f, item->pos.y, (float)bitmap->Width(),
			(float)bitmap->Height()),
		0xffffffff, 0);
	const Bitmap* badge;
	{
		IFF_STRUCT::sSkin* skin;
		if (info.dwTitle && (skin = ItemManager()->FindSkin(info.dwTitle)))
			badge = g_pFresh->GetBitmap(skin->c.Icon);
		else
			badge = g_pFresh->GetManager()->GetBitmap("LEVELS",
				MakeStr("level_%03d", user->level + 1));
	}
	if (info.dwIdentity & 0x8000) // 983 acfa7c: host (gp_host) so se o bitmap existe
	{
		const Bitmap* host = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");
		if (host)
			badge = host;
	}
	if (badge)
	{
		// 983 acf9c0: y antes de x
		float by = item->pos.y + 13.0f - float2int(badge->Height() * 0.5f);
		float bx = item->pos.x + 28.0f - float2int(badge->Width() * 0.5f);
		gdi->DrawTexture(badge,
			WRect(bx, by, (float)badge->Width(), (float)badge->Height()),
			0xffffffff, 0);
	}
	if (info.m_GuildId && !info.IsIdentity(0x14))
	{
		const Bitmap* emblem =
			NetResourceManager::Instance()->GetEmblem983(info.m_GuildId, info.dwEmblemVer983);
		if (emblem)
		{
			gdi->DrawTexture(emblem,
				WRect(item->pos.x + 75.0f, item->pos.y + 1.0f,
					(float)emblem->Width(), (float)emblem->Height()),
				0xffffffff, 0);
		}
	}
	if (!CProjectG::Instance()->HidePrivacy())
		g_pFresh->GetManager()->PrintText(
			WPoint(item->pos.x + 98.0f, item->pos.y + 7.0f), 0, info.sNick,
			-1.0f, nickColor);
	sRoomUserInfo* current = user;
	if (current->hole == 0)
	{
		// 983: um K2L so com o texto escolhido, depois do ponto
		const char* state = info.bSleep ? "\300\341\274\366\301\337" : "\264\353\261\342\301\337";
		gdi->Print(WPoint(item->pos.x + 207.0f, item->pos.y + 7.0f), 1, K2L_Compatibility(state));
	}
	else if (current->hole == 19)
		gdi->Print(WPoint(item->pos.x + 207.0f, item->pos.y + 7.0f), 1, "End");
	else
		gdi->Print(WPoint(item->pos.x + 207.0f, item->pos.y + 7.0f), 1,
			K2L_Compatibility("%d \310\246"), current->hole);
}

void FrDtRoomDlg::OnUserListLBtnDown()
{
	FrListItem* item = m_pUserList->GetItemUnderCursor();
	if (item)
	{
		sRoomUserInfo* user = (sRoomUserInfo*)item->pData;
		m_pUserList->SelectItem(item, true);
		m_pSelUser = user;
		if (m_pUserInfoBtn)
			m_pUserInfoBtn->Enable(true);
	}
}

void FrDtRoomDlg::OnUserListRBtnUp()
{
	OnUserListLBtnDown();
	if (m_pSelUser && CUserInfo::Instance())
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(m_pSelUser->uid);
		if (it != Doc()->m_briefUserInfoMap.end())
			CUserInfo::Instance()->SetInfo(it->second.dwUid, m_pSelUser->uid,
				true, true, false, false, it->second.sNick);
	}
}

bool FrDtRoomDlg::OnPasswordDlgResult(int result, FrForm* form)
{
	if (result == 1)
	{
		FrWnd* child = form->FindChildByName("password");
		FrEdit* password = DYNAMIC_CAST(FrEdit, child);
		std::string text;
		if (password)
		{
			const char* line = password->GetLine(1, false);
			text = line;
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0x27), (int)line, 0, 0, 0, 0);
		}
		FrWnd* ok = form->FindChildByName("ok");
		if (ok)
			ok->Enable(false);
		WSendPacket packet((enumClientPacket)9);
		packet.Encode2(m_roomInfo.roomGuid);
		packet.EncodeStr(text);
		packet.Send(TO_GAME);
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x16), 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		Close((eFormRet)1, true);
	}
	return true;
}

// 00acfcf0: o texto do Grand Prix nas 4 linhas da descricao + regra, nivel, condicao, equipamento e premios no fim
void FrDtRoomDlg::ShowGrandPrix983()
{
	if (!m_bGrandPrix983)
		return;
	sGPInfo983* pInfo = m_pGrandPrix983;
	char text[0x200];
	memset(text, 0, sizeof(text));
	sprintf(text, "%s : %s", GetGameString(0x7543), pInfo->name);
	m_pGameDesc->SetLine(1, text, 0, false, 0);

	if (Doc()->m_itemManager.HasGrandPrixSpecialHole983(pInfo->ticket) == 1)
		strcpy(text, GetGameString(0x7555));
	else if (pInfo->course == 255)
		strcpy(text, K2L_Compatibility("\304\332\275\272 : RANDOM"));
	else
	{
		unsigned long map = pInfo->course;
		if (map > 127 && map != 253)
			map -= 128;
		IFF_STRUCT::sCourse* course = Doc()->m_itemManager.FindCourse(map | 0x28000000);
		if (course)
			sprintf(text, K2L_Compatibility("\304\332\275\272 : %s"), course->c.Name);
		else
			strcpy(text, K2L_Compatibility("\304\332\275\272 : \270\360\270\247"));
	}
	m_pGameDesc->SetLine(2, text, 0, false, 0);

	if (pInfo->startHole == 2)
		sprintf(text,
			K2L_Compatibility("\275\303\300\333\310\246 : \267\243\264\375 \310\246 / \310\246 \274\366 : %d \310\246"),
			pInfo->nHole);
	else if (pInfo->startHole == 3)
		sprintf(text,
			K2L_Compatibility("\275\303\300\333\310\246 : \274\305\307\303 \310\246 / \310\246 \274\366 : %d \310\246"),
			pInfo->nHole);
	else
		sprintf(text,
			K2L_Compatibility("\275\303\300\333\310\246 : %d \310\246 / \310\246 \274\366 : %d \310\246"),
			pInfo->startHole ? 10 : 1, pInfo->nHole);
	m_pGameDesc->SetLine(3, text, 0, false, 0);

	// horario: so no Grand Prix sem tipo ou com tipo (bits 19-25) != 0
	if (pInfo->ticket == 0 || ((pInfo->ticket >> 19) & 0x7f) != 0)
	{
		sprintf(text, GetGameString(0x7545), pInfo->start[4], pInfo->start[5], pInfo->end[4], pInfo->end[5]);
		m_pGameDesc->SetLine(4, text, 0, false, 0);
	}

	std::string rule = GPRuleText983(pInfo);
	if (!rule.empty())
	{
		sprintf(text, rule.c_str()); // 983: sprintf de 2 args
		((FrEditJp983*)m_pGameDesc)->AddText983(text, false, true, 0, 0);
	}

	if (m_pGrandPrix983->maxLevel > 0)
	{
		((FrEditJp983*)m_pGameDesc)->AddText983(MakeStr(GetGameString(0x754a),
			K2L_Compatibility(s_levelNames.Get(m_pGrandPrix983->minLevel).name),
			K2L_Compatibility(s_levelNames.Get(m_pGrandPrix983->maxLevel).name)), false, true, 0, 0);
	}
	if (m_pGrandPrix983->m_9c > 0)
	{
		if (m_pGrandPrix983->m_9c == 0xb4)
			((FrEditJp983*)m_pGameDesc)->AddText983(MakeStr(GetGameString(0x7556), m_pGrandPrix983->m_98), false, true, 0, 0);
		else if (m_pGrandPrix983->m_98 == 0)
			((FrEditJp983*)m_pGameDesc)->AddText983(MakeStr(GetGameString(0x754c), m_pGrandPrix983->m_9c), false, true, 0, 0);
		else
			((FrEditJp983*)m_pGameDesc)->AddText983(MakeStr(GetGameString(0x754b), m_pGrandPrix983->m_98, m_pGrandPrix983->m_9c), false, true, 0, 0);
	}

	std::vector<IFF983::sGrandPrixConditionEquip>* pEquip =
		Doc()->m_itemManager.FindGrandPrixConditionEquip983(m_pGrandPrix983->ticket);
	if (pEquip)
		((FrEditJp983*)m_pGameDesc)->AddText983((const char*)&pEquip->at(0).raw[4], false, true, 0, 0);

	std::string prizes;
	std::vector<IFF983::sGrandPrixRankReward>* pRank =
		Doc()->m_itemManager.FindGrandPrixRankReward983(m_pGrandPrix983->ticket);
	if (pRank && pRank->begin() != pRank->end())
	{
		IFF983::sGrandPrixRankReward first = pRank->at(0);
		const unsigned long* pTid = (const unsigned long*)((const char*)&first + 0xc);
		for (unsigned int i = 0; i < 5; i++)
		{
			const char* name;
			if (pTid[i] && (name = Doc()->m_itemManager.GetItemName(pTid[i])) != NULL)
			{
				prizes.append(name);
				prizes.append(", ");
			}
		}
	}
	for (int i = 0; i < 5; i++)
	{
		const char* name;
		if (m_pGrandPrix983->prize[i] && (name = Doc()->m_itemManager.GetItemName(m_pGrandPrix983->prize[i])) != NULL)
		{
			prizes.append(name);
			prizes.append(", ");
		}
	}
	if (!prizes.empty())
	{
		std::string::size_type pos = prizes.find_last_of(",");
		if (pos != std::string::npos)
			prizes.erase(pos - 1);
		strcpy(text, MakeStr(GetGameString(0x7546), prizes.c_str()));
		((FrEditJp983*)m_pGameDesc)->AddText983(text, false, true, 0, 0);
	}
}
