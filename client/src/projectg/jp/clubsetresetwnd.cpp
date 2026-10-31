// JP 983: unidade 00499800–0049c430 (inicialização do taco na oficina: CClubSetInfoFont, FrClubSetResetWnd, FrClubSetResetDlgBase
// e as duas derivadas, FrClubSetResetProgress). Classes do 839/983, sem par no 645; ver include/jp/clubsetresetwnd.h e
// include/jp/clubsetinfofont.h. Sessão B89 (F3.7.9, 14º lote). O compilador gera (e casam) os moldes da unidade: vector<unsigned
// long> (push_back 0049b4a0 e companhia), shared_ptr da figura (0049b1e0 / 0049a860), lua_tinker::table::get<table> (0049a190 /
// 00499dd0), CreateForm<…> (00499f30, 0049b250, 0049b340, 0049c020), GetCtrl983<FrWnd> (0049b9f0) e os MakeInstance.
// Aqui também ficam os corpos das duas leituras de Lua do 983 que as outras unidades só declaram (lua983.h): a tabela filha por
// nome (0049a4e0) e o float por índice (0049a000).
#include "minatl.h"
#include <stdio.h>
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frgaugebar.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "actor.h"
#include "../../../include/packets/clubsetworkpackets.h"
#include "lua983.h"
#include "clubsetstat.h"
#include "clubsetinfofont.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopselector.h"
#include "clubsetresetwnd.h"

extern Fresh* g_pFresh; // 00f02340

// erro do gerenciador de scripts (00c1f480: chama o tratador em +0x14, se houver): LuaScriptManagerJp983 de lua983.h (B112)

// 0049a4e0: tabela filha por nome; sem ela, avisa e devolve a do nil
LuaChild983 LuaHelper983::GetChild983(const char* name)
{
	// D406: indice velho na pilha (d399-1 'index a number value') cai no lua_gettable; revalida como table::get faz
	if (table.m_obj && table.m_obj->validate())
	{
		lua_pushstring(state, name);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) != LUA_TNIL)
		{
			lua_remove(state, -1);
			goto found;
		}
		lua_remove(state, -1);
	}
	char text[512];
	sprintf(text, "not found /'%s/' child table.", name);
	((LuaScriptManagerJp983*)lua_system::lua_script_manger::GetInstance())->Error983(text);
found:
	return LuaChild983(table.get<lua_tinker::table>(name));
}

// 0049a000
float LuaChild983::GetByIndex(int index)
{
	if (table.m_obj && table.m_obj->validate()) // D406
	{
		lua_pushnumber(state, (double)index);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) == LUA_TNIL)
			lua_remove(state, -1);
		else
		{
			lua_remove(state, -1);
			lua_rawgeti(state, table.m_obj->m_index, index);
			return lua_tinker::pop<float>(state);
		}
	}
	lua_pushnil(state);
	return lua_tinker::pop<float>(state);
}

// ---------------------------------------------------------------------------------------------------------------------------
// CClubSetInfoFont

// 00499800
CClubSetInfoFont::CClubSetInfoFont()
{
	static char* fileName[] = { "[font_rank.png" }; // 00e04b38
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 20;
	info.fonth = 18;
	info.numPages = 1;
	info.texw = 140;
	info.texh = 18;
	info.pCharSet = "EDCBASL";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
	m_pNumFont = new CWindFont;
}

// 00499950
void CClubSetInfoFont::PrintLevel983(int level, float x, float y, eTITLEFONTALINE align, float space, float scale)
{
	if (m_pNumFont)
	{
		Print(x, y, "L", align, space, scale);
		m_pNumFont->Print(x + 2.0, y + 3.0, MakeStr("%d", level + 1));
	}
}

// 0049aec0
void CClubSetInfoFont::PrintRank983(int rank, float x, float y, eTITLEFONTALINE align, float space, float scale)
{
	std::string text;
	switch (rank)
	{
	case 0: text = "E"; break;
	case 1: text = "D"; break;
	case 2: text = "C"; break;
	case 3: text = "B"; break;
	case 4: text = "A"; break;
	case 5: text = "S"; break;
	default: return;
	}
	if (!text.empty())
		Print(x, y, text.c_str(), align, space, scale);
}

// ---------------------------------------------------------------------------------------------------------------------------
// FrClubSetResetWnd

FrClubSetResetWnd* FrClubSetResetWnd::s_pInstance = NULL;
FrCmdTarget* FrClubSetResetWnd::s_pOwner = NULL;

// os resultados dos diálogos voltam pelo dono (CRealMyRoomMain, frente A: métodos ainda sem declaração lá) — empréstimo
struct CTaskMainBaseJp983 // a 1ª base do CRealMyRoomMain: só o vptr antes do FrCmdTarget (+4: ajuste −4 no ponteiro de método)
{
	virtual ~CTaskMainBaseJp983();
};
class CRealMyRoomMainJp983 : public CTaskMainBaseJp983, public FrCmdTarget
{
public:
	bool OnClubSetResetSelected983(int result, FrForm* pForm); // 0060ac10: s_pInstance->OnSelectResult983
	bool OnClubSetResetQuestion983(int result, FrForm* pForm); // 0060ac30: s_pInstance->OnQuestionResult983
	bool OnClubSetResetProgress983(int result, FrForm* pForm); // 0060ac50: s_pInstance->OnProgressResult983
	bool OnClubSetResetDone983(int result, FrForm* pForm); // 00605eb0: apaga o s_pInstance
};

// 00499a60
void FrClubSetResetWnd::Clear983()
{
	m_itemTid = 0;
	m_clubGuid = 0;
	m_clubTid = 0;
	m_state = 0;
}

// 00499a80
bool FrClubSetResetWnd::SetClub983(unsigned long clubGuid)
{
	if (clubGuid)
	{
		sItemInfo* pClub = FindMyClubSet983(clubGuid);
		if (pClub)
		{
			m_clubGuid = clubGuid;
			m_clubTid = pClub->typeId;
			return true;
		}
	}
	Clear983();
	return false;
}

// 00499ac0
bool FrClubSetResetWnd::OnProgressResult983(int result, FrForm* pForm)
{
	if (result == 1)
		SendClubSetRandomBox983(m_clubGuid, m_itemTid);
	return true;
}

// 00499ae0
void FrClubSetResetQuestionDlg::OnOK()
{
	ClubSetWorkShopForm::OnOK();
}

// 00499af0
FrWnd* FrClubSetResetQuestionDlg::OnCtrlCreated983(IObject* pObj)
{
	return AddCtrlByType983(pObj);
}

// 00499b00
FrWnd* FrClubSetResetResultDlg::OnCtrlCreated983(IObject* pObj)
{
	return AddCtrlByType983(pObj);
}

// 00499b10
bool FrClubSetResetProgress::OnInit()
{
	FrRandomBoxProgress::OnInit();
	m_totalTime = 3.0f;
	return true;
}

// 00499c50: itens de reset do taco
static bool IsResetItem983(unsigned long tid)
{
	switch (tid)
	{
	case 0x1a000247:
	case 0x1a00024b:
		return true;
	}
	return false;
}

// 00499eb0
FrClubSetResetWnd::FrClubSetResetWnd()
{
	s_pInstance = this;
	m_pOwner = s_pOwner;
	m_itemTid = 0;
	m_clubGuid = 0;
	m_clubTid = 0;
	m_level = -1;
	m_rank = -1;
	m_bDirect = false;
	m_state = 0;
}

// 00499ef0
bool FrClubSetResetWnd::OnCloseQuestion983(int result, FrForm* pForm)
{
	if (FrClubSetResetQuestionDlg::s_pInstance)
		FrClubSetResetQuestionDlg::s_pInstance->Close(eFormRet(2), true);
	return true;
}

// 0049a230
FrClubSetResetWnd* __fastcall FrClubSetResetWnd::Create983(FrCmdTarget* pOwner)
{
	s_pOwner = pOwner;
	return new FrClubSetResetWnd;
}

// 0049a250
FrClubSetResetWnd::~FrClubSetResetWnd()
{
	if (m_clubGuid && m_clubTid)
	{
		unsigned long club[7];
		club[0] = m_clubGuid;
		club[1] = m_clubTid;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x4a, m_clubGuid, (int)club, 0, 0, 0));
	}
	s_pInstance = NULL;
	Clear983();
}

// 0049a2f0
void FrClubSetResetProgress::OnProc(const float deltaTime)
{
	m_elapsedTime += deltaTime;
	if (m_elapsedTime > m_totalTime)
	{
		m_pProgress->SetPos(100);
		if (m_elapsedTime > m_totalTime + 0.5)
		{
			m_elapsedTime = 0;
			m_totalTime = 0;
			FrForm::OnOK();
		}
	}
	else
	{
		int pos = (int)((m_elapsedTime / m_totalTime) * 100.0);
		if (pos < 100)
			m_pProgress->SetPos(pos);
	}
	if (RESETPROGRESS_IMAGE983)
	{
		m_animTime += deltaTime;
		if (m_animTime > 0.5f)
		{
			if (m_animIndex == m_anims.size())
				m_animIndex = 0;
			RESETPROGRESS_IMAGE983->SetBgImg(m_anims[m_animIndex++].c_str());
			m_animTime = 0;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------------
// FrClubSetResetDlgBase

// 0049a6b0
void FrClubSetResetDlgBase::LoadLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_selector");
	if (!m_pFont[0])
		m_pFont[0] = new CClubSetInfoFont;
	if (!m_pFont[1])
		m_pFont[1] = new CClubSetInfoFont;
	m_levelPos[0] = LUA983_POS(tbl, "level_img_pos", 1);
	m_levelPos[1] = LUA983_POS(tbl, "level_img_pos", 2);
	m_rankPos[0] = LUA983_POS(tbl, "rank_img_pos", 1);
	m_rankPos[1] = LUA983_POS(tbl, "rank_img_pos", 2);
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// 0049aa30
FrClubSetResetDlgBase::FrClubSetResetDlgBase()
{
	m_pFont[0] = NULL;
	m_pFont[1] = NULL;
	m_level[0] = -1;
	m_level[1] = -1;
	m_rank[0] = -1;
	m_rank[1] = -1;
	LoadLayout983();
}

// 0049ab30
FrClubSetResetDlgBase::~FrClubSetResetDlgBase()
{
	m_level[0] = -1;
	m_level[1] = -1;
	m_rank[0] = -1;
	m_rank[1] = -1;
	if (m_pFont[0])
	{
		delete m_pFont[0];
		m_pFont[0] = NULL;
	}
	if (m_pFont[1])
	{
		delete m_pFont[1];
		m_pFont[1] = NULL;
	}
}

IMPLEMENT_OBJECT(FrClubSetResetDlgBase, ClubSetWorkShopForm)
IMPLEMENT_OBJECT(FrClubSetResetQuestionDlg, FrClubSetResetDlgBase)
IMPLEMENT_OBJECT(FrClubSetResetResultDlg, FrClubSetResetDlgBase)

FrClubSetResetQuestionDlg* FrClubSetResetQuestionDlg::s_pInstance = NULL;
FrClubSetResetResultDlg* FrClubSetResetResultDlg::s_pInstance = NULL;

// 0049b530
FrClubSetResetQuestionDlg* __fastcall FrClubSetResetQuestionDlg::Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest)
{
	return CreateForm<FrClubSetResetQuestionDlg>(pManager, pCmdDest, "clubsetwork_okcancel", NULL);
}

// 0049b540
FrClubSetResetResultDlg* __fastcall FrClubSetResetResultDlg::Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest)
{
	return CreateForm<FrClubSetResetResultDlg>(pManager, pCmdDest, "clubsetwork_ok", NULL);
}

// 0049b550
void FrClubSetResetWnd::OpenSelector983()
{
	FrClubSetWorkshopSelector* pDlg = CreateForm<FrClubSetWorkshopSelector>(g_pFresh->GetManager(), m_pOwner, "selectclubset", NULL);
	if (pDlg)
	{
		unsigned long none = 0;
		pDlg->m_marked.push_back(none);
		pDlg->m_kind = 6;
		pDlg->Open((FRESH_PFN_RESULT)&CRealMyRoomMainJp983::OnClubSetResetSelected983, 1);
		m_state = 1;
	}
}

// 0049b5e0: "[%s]를 %d개 소모하여 클럽세트의 개조/랭크업 및 업그레이드 상태를 초기화합니다. …"
bool FrClubSetResetQuestionDlg::SetClub983(unsigned long clubGuid, unsigned long clubTid, unsigned long itemTid)
{
	if (!clubGuid || !clubTid)
		return false;
	m_level[0] = GetMyClubStep983(clubGuid);
	m_level[1] = GetClubSetStep983(clubTid);
	m_rank[0] = GetMyClubGrade983(clubGuid);
	m_rank[1] = GetClubSetGrade983(clubTid);
	if (m_level[0] == -1 || m_level[1] == -1 || m_rank[0] == -1 || m_rank[1] == -1)
		return false;
	std::string name = ItemManager()->GetItemName(itemTid);
	IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(clubTid);
	if (pClub)
	{
		SetSize983(350.0f, 340.0f, true);
		if (itemTid == 0x1a000247)
		{
			SetMessage(MakeStr(K2L_Compatibility("[%s]\270\246 \134c0xffff0000\134c%d\260\263 \274\322\270\360\134c0xff000000\134c"
				"\307\317\277\251\012\305\254\267\264\274\274\306\256\300\307 \134c0xffff0000\134c\260\263\301\266/\267\251\305\251\276\367 "
				"\271\327 \276\367\261\327\267\271\300\314\265\345 \273\363\305\302\270\246\012\303\312\261\342\310\255\134c0xff000000\134c"
				"\307\325\264\317\264\331.\012\012\305\254\267\264\274\274\306\256\277\241 \134c0xffff0000\134c\263\262\276\306\300\326\264\302 "
				"\274\367\267\303\265\265\264\302 \300\257\301\366\134c0xff000000\134c\265\307\270\347,\012\300\345\302\370\265\307\276\356 "
				"\300\326\264\302 \304\253\265\345\264\302 \300\257\301\366\265\313\264\317\264\331."), name.c_str(), 1, 50), false);
		}
		else if (itemTid == 0x1a00024b)
		{
			SetMessage(MakeStr(K2L_Compatibility("[%s]\270\246 \134c0xffff0000\134c%d\260\263 \274\322\270\360\134c0xff000000\134c"
				"\307\317\277\251\012\305\254\267\264\274\274\306\256\300\307 \134c0xffff0000\134c\260\263\301\266/\267\251\305\251\276\367 "
				"\271\327 \276\367\261\327\267\271\300\314\265\345 \273\363\305\302\270\246\012\303\312\261\342\310\255\134c0xff000000\134c"
				"\307\325\264\317\264\331.\012\012\267\251\305\251\276\367\277\241 \273\347\277\353\307\321 \134c0xffff0000\134c\274\367\267"
				"\303\265\265\300\307 %d\243\245\134c0xff000000\134c\277\315\012\305\254\267\264 \276\367\261\327\267\271\300\314\265\345\277"
				"\241 \273\347\277\353\307\321 \134c0xffff0000\134c\306\316\300\307 %d\243\245\134c0xff000000\134c\264\302 \265\271\267\301"
				"\271\336\300\270\270\347,\012\300\345\302\370\265\307\276\356 \300\326\264\302 \304\253\265\345\264\302 \300\257\301\366\265"
				"\313\264\317\264\331."), name.c_str(), 1, 50, 50), false);
		}
		m_before = AddBitmap983((Bitmap*)g_pFresh->GetBitmap(pClub->c.Icon));
		AddBitmap983((Bitmap*)g_pFresh->GetBitmap("clubmade_arrow"));
		m_after = AddBitmap983((Bitmap*)g_pFresh->GetBitmap(pClub->c.Icon));
		if (m_before->pBmp && m_after->pBmp)
			return true;
	}
	return false;
}

// 0049b830: "[%s]가 초기화되었습니다."
bool FrClubSetResetResultDlg::SetResult983(int level, int rank, unsigned long clubTid)
{
	if (!clubTid)
		return false;
	m_level[0] = level;
	m_level[1] = GetClubSetStep983(clubTid);
	m_rank[0] = rank;
	m_rank[1] = GetClubSetGrade983(clubTid);
	if (m_level[0] == -1 || m_level[1] == -1 || m_rank[0] == -1 || m_rank[1] == -1)
		return false;
	IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(clubTid);
	if (!pClub)
		return false;
	SetSize983(330.0f, 250.0f, true);
	SetMessage(MakeStr(K2L_Compatibility("[%s]\260\241\012\303\312\261\342\310\255\265\307\276\372\275\300\264\317\264\331."),
		pClub->c.Name), false);
	m_before = AddBitmap983((Bitmap*)g_pFresh->GetBitmap(pClub->c.Icon));
	AddBitmap983((Bitmap*)g_pFresh->GetBitmap("clubmade_arrow"));
	m_after = AddBitmap983((Bitmap*)g_pFresh->GetBitmap(pClub->c.Icon));
	if (m_before->pBmp && m_after->pBmp)
		return true;
	return false;
}

// 0049bb50
void FrClubSetResetWnd::OpenQuestion983()
{
	FrClubSetResetQuestionDlg* pDlg = FrClubSetResetQuestionDlg::Create983(g_pFresh->GetManager(), m_pOwner);
	if (pDlg)
	{
		if (pDlg->SetClub983(m_clubGuid, m_clubTid, m_itemTid))
		{
			m_level = GetMyClubStep983(m_clubGuid);
			m_rank = GetMyClubGrade983(m_clubGuid);
			pDlg->Open((FRESH_PFN_RESULT)&CRealMyRoomMainJp983::OnClubSetResetQuestion983, 1);
			m_state = 2;
			return;
		}
		pDlg->Close(eFormRet(2), true);
		Clear983();
	}
}

// 0049bc00
bool FrClubSetResetWnd::OnSelectResult983(int result, FrForm* pForm)
{
	if (result == 1 && pForm)
	{
		if (SetClub983(((FrClubSetWorkshopSelector*)pForm)->m_selGuid))
			OpenQuestion983();
	}
	return true;
}

// 0049bc40
void FrClubSetResetWnd::OnResetResult983(WReceivedPacket* pPacket)
{
	if (pPacket)
	{
		FrClubSetResetResultDlg* pDlg = FrClubSetResetResultDlg::Create983(g_pFresh->GetManager(), m_pOwner);
		if (pDlg)
		{
			if (pDlg->SetResult983(m_level, m_rank, m_clubTid))
			{
				pDlg->Open((FRESH_PFN_RESULT)&CRealMyRoomMainJp983::OnClubSetResetDone983, 3);
				m_state = 4;
			}
			else
			{
				pDlg->Close(eFormRet(2), true);
				m_state = 0;
			}
		}
		pPacket->Decode4();
		pPacket->Decode4();
	}
}

// 0049bcf0: nível e rank de antes e de depois sobre as duas figuras do taco
void FrClubSetResetDlgBase::DrawBitmaps983()
{
	ClubSetWorkShopForm::DrawBitmaps983();
	if (!m_before->pBmp || !m_after->pBmp)
		return;
	FrWnd* pImage = GetCtrl983<FrWnd>("image");
	if (!pImage)
		return;
	float x0 = m_rect.w * 0.5 - m_bmpTotalW * 0.5;
	WPoint before(m_before->x + pImage->GetRect().x + x0, m_before->y + pImage->GetRect().y);
	WPoint after(m_after->x + pImage->GetRect().x + x0, m_after->y + pImage->GetRect().y);
	if (m_rank[0] != 5)
		m_pFont[0]->PrintLevel983(m_level[0], m_levelPos[0] + before.x, m_levelPos[1] + before.y, TFA_RIGHT, 0.0f, 1.0f);
	if (m_rank[1] != 5)
		m_pFont[1]->PrintLevel983(m_level[1], m_levelPos[0] + after.x, m_levelPos[1] + after.y, TFA_RIGHT, 0.0f, 1.0f);
	m_pFont[0]->PrintRank983(m_rank[0], before.x + m_rankPos[0], m_rankPos[1] + before.y, TFA_RIGHT, 0.0f, 1.0f);
	m_pFont[1]->PrintRank983(m_rank[1], after.x + m_rankPos[0], m_rankPos[1] + after.y, TFA_RIGHT, 0.0f, 1.0f);
}

// 0049bee0
void FrClubSetResetWnd::Start983(unsigned long itemTid, unsigned long clubGuid)
{
	if (itemTid)
	{
		m_itemTid = itemTid;
		if (!clubGuid)
		{
			m_bDirect = false;
			OpenSelector983();
			return;
		}
		m_bDirect = true;
		if (SetClub983(clubGuid))
			OpenQuestion983();
	}
}

// ---------------------------------------------------------------------------------------------------------------------------
// FrClubSetResetProgress

// 0049bf20
FrClubSetResetProgress::FrClubSetResetProgress()
{
	m_animTime = 0;
	m_itemTid = 0;
	m_animIndex = 0;
}

// 0049bfa0
FrClubSetResetProgress::~FrClubSetResetProgress()
{
	m_animTime = 0;
	m_itemTid = 0;
	m_animIndex = 0;
}

// 0049c120
void FrClubSetResetProgress::SetItem983(unsigned long itemTid)
{
	if (!IsResetItem983(itemTid))
		return;
	m_itemTid = itemTid;
	const char* name = Doc()->m_itemManager.GetItemName(itemTid);
	if (name)
		SetExplane(MakeStr(K2L_Compatibility("'%s'\267\316 \305\254\267\264\274\274\306\256\270\246 \303\312\261\342\310\255\307\317"
			"\260\355 \300\326\275\300\264\317\264\331."), name));
	SetFixed(true);
	if (m_itemTid == 0x1a000247)
	{
		m_anims.push_back("club_init_item_a_anim01");
		m_anims.push_back("club_init_item_a_anim02");
	}
	else if (m_itemTid == 0x1a00024b)
	{
		m_anims.push_back("club_init_item_a_anim01");
		m_anims.push_back("club_init_item_a_anim02");
	}
	if (RESETPROGRESS_IMAGE983)
	{
		if (m_anims.size())
			RESETPROGRESS_IMAGE983->SetBgImg(m_anims[m_animIndex++].c_str());
	}
}

// 0049c380
bool FrClubSetResetWnd::OnQuestionResult983(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (m_clubGuid)
		{
			FrClubSetResetProgress* pDlg = CreateForm<FrClubSetResetProgress>(g_pFresh->GetManager(), m_pOwner, "newRandomBox2", NULL);
			if (!pDlg)
				return false;
			pDlg->SetItem983(m_itemTid);
			pDlg->Open((FRESH_PFN_RESULT)&CRealMyRoomMainJp983::OnClubSetResetProgress983, 1);
			m_state = 3;
			return true;
		}
	}
	else if (!m_bDirect)
		OpenSelector983();
	return true;
}
