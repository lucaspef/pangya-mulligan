// JP 983: unidade 0047f2a0–00484010 (FrAvatarMotionDlg, a lista de animações do boneco no lobby e na conversa de avatares;
// ver include/jp/avatarmotiondlg.h). Classe nova do 983 (existe no 839, não no 645). Sessão B111: a janela inteira; B112: o
// pedaço de BGM do fim da unidade (004835d0–00484000, CBGMPlayList983 de include/jp/bgmplaylist.h: "BGMlist.lua",
// "default_bgm", chamado de 00a68f20, do jukebox e do HandleMsg do CLobbyMain 00abc140).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frlistbox.h"
#include "frgraphicinterface.h"
#include "commonutil.h"
#include "wlocalize.h"
#include "shareddoc.h"
#include "actor.h"
#include "taskmanager.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "partsinfo.h"
#include "realmyroom_avatar.h"
#include "avatar_avatar.h"
#include "lua983.h"
#include "cardcomposedlg.h"
#include "avatarmotiondlg.h"
#include "jukeboxdlg.h" // C404: FrJukeBoxDlg::SelectBGMByName (B654); sem ele o build limpo do HEAD falha
#include "soundmanager.h"
#include "bgmplaylist.h"
#include "createcharacteroption.h"
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "refresh/rflobbydesktop.h"

extern Fresh* g_pFresh; // 00f02340
extern WView* g_pMainView983; // 00f02458
#define VIEWW983() (*(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x19c)))

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

// CMouseCursor (frente A): 00891480 sem declaração no cabeçalho; singleton 00e73e7c
class CMouseCursorJp983
{
public:
	bool InArea(const WRect& rect); // 00891480
};
#define MOUSE983() ((CMouseCursorJp983*)CMouseCursor::m_pInstance)

// tabela de animações por personagem (CSharedDoc+0x10c24): visão da lobbyplayer.cpp
class CLobbyMotionTable983
{
public:
	int Count983(const unsigned long* pTid); // 009a3c80
	const char* Get983(const unsigned long* pTid, const int* pIndex); // 009a3c00
	const char* GetName983(const unsigned long* pTid, const int* pIndex); // 009a3c40
};
#define LOBBYMOTION983() ((CLobbyMotionTable983*)&::Doc()->m_chatManager)
C_ASSERT_VC10(offsetof(CSharedDoc, m_chatManager) == 0x10c24); // A748: era cru Doc()+0x10c24

// CLobbyState983 (00e78ca8) = RFLobbyDesktop::Instance983(); RFLobbyDesktop::GetCharType983 005f6410 lê o personagem da seleção

// boneco do jogador (ntAvatarManager 00e73e80 +0xa4): +0x82 nome, +0x154 tid do personagem, +0x495 peças
// A299/B510: g_pAvatarMotion983 era so declaracao (nulo no exe): 00e73e80 = ntAvatarManager::m_pInstance; campos pelo layout nosso
#define MYAVATAR983() (ntAvatarManager::m_pInstance->m_pMyAvatar)
#define MYCHARTYPE983() (MYAVATAR983()->GetCharacterInfo().tid & 0x3ffffff)

// por índice fora de linha (00480010; o pop<const char*> 0047f6b0 também fora de linha)
struct LuaMotionChild983 : LuaChild983
{
	const char* GetStrByIndex983(int index);
};
// 00480010
const char* LuaMotionChild983::GetStrByIndex983(int index)
{
	// C150: o 983 testa antes por lua_gettable, que aqui volta nil com chave numerica (E146/e145-1) -> NULL no
	// _stricmp da FindMotion983 (c147-1, c0000417); le direto pelo rawgeti como o Push983 do lua983.h
	if (table.m_obj)
	{
		lua_rawgeti(state, table.m_obj->m_index, index);
		return lua_tinker::pop<const char*>(state);
	}
	lua_pushnil(state);
	return lua_tinker::pop<const char*>(state);
}

// 0047f2a0 (GetMessageMap); mapa 00e037c8 (3 entradas; ajuste -4 em todas)
BEGIN_FRESH_MSGMAP(FrAvatarMotionDlg, FrForm)
ON_FRESH_VI("motionlist", FRCMD_INIT, FrAvatarMotionDlg::OnMotionListInit)
ON_FRESH_VI("motionlist", FRCMD_OWNERDRAW, FrAvatarMotionDlg::OnMotionListOwnerDraw)
ON_FRESH_VV("motionlist", FRCMD_LBUTTONDOWN, FrAvatarMotionDlg::OnMotionListLBDown)
END_FRESH_MSGMAP()

// 0047f2b0
void FrAvatarMotionDlg::CalcTextX983()
{
	m_textX = (m_hoverRect.w + m_hoverRect.x + m_hoverRect.x) * 0.5;
}

// 0047f2d0
void FrAvatarMotionDlg::LoadBitmaps983()
{
	m_pHoverBmp = g_pFresh->GetBitmap("motionchat_list_Hover");
	m_pPlayBmp = g_pFresh->GetBitmap("motionchat_list_play");
	m_pStopBmp = g_pFresh->GetBitmap("motionchat_list_stop");
}

// 0047f930
void FrAvatarMotionDlg::OnMotionListInit(int param)
{
	m_pMotionList = KINDOF_CAST983(FrListBox, param);
	m_pMotionList->UseDblClick(false);
}

// 0047f9b0
void FrAvatarMotionDlg::InitHoverRect983()
{
	m_hoverRect.x = VIEWW983() - 76.0;
	m_hoverRect.y = 0.0f;
	m_hoverRect.w = m_rect.w - 11.0;
	m_hoverRect.h = 24.0f;
}

// 0047f9f0
void FrAvatarMotionDlg::InitIconRect983()
{
	m_iconRect.x = m_hoverRect.x - 13.0;
	m_iconRect.y = 0.0f;
	m_iconRect.w = (float)m_pPlayBmp->Width();
	m_iconRect.h = (float)m_pPlayBmp->Height();
}

// 0047fa50
void FrAvatarMotionDlg::SlideOpen983(const float& deltaTime)
{
	SetRect(WRect(m_rect.x, m_rect.y, m_rect.w, deltaTime * 3000.0 + m_rect.h));
	float h = (int)m_list.size() * 24.0 + 25.0;
	if (m_rect.h >= h)
	{
		SetRect(WRect(m_rect.x, m_rect.y, m_rect.w, h));
		m_slide = 1;
	}
}

// 0047fb30
void FrAvatarMotionDlg::SlideClose983(const float& deltaTime)
{
	SetRect(WRect(m_rect.x, m_rect.y, m_rect.w, m_rect.h - deltaTime * 3000.0));
	if (m_rect.h <= 40.0f)
	{
		SetRect(WRect(m_rect.x, m_rect.y, m_rect.w, 40.0f));
		m_textAlpha = 0.0f;
		m_slide = 2;
		Close(true);
	}
}

// 0047fd40
const char* FrAvatarMotionDlg::GetMotion983(int index)
{
	if (index >= 0 && (unsigned int)index < m_allMotions.size())
		return m_allMotions[index].motion;
	return NULL;
}

// 0047fd70
void FrAvatarMotionDlg::ProcHover983(const float& deltaTime)
{
	if (MOUSE983()->InArea(m_rect))
	{
		m_hoverAlpha += deltaTime + deltaTime;
		if (m_hoverAlpha > 1.0f)
			m_hoverAlpha = 1.0f;
		SetCursor(1);
	}
	else
	{
		m_hoverAlpha -= deltaTime * 2.0f;
		if (m_hoverAlpha < 0.7f)
			m_hoverAlpha = 0.7f;
	}
	SetAlpha2ToChild(m_hoverAlpha);
}

// 0047fbf0: no AvatarChat, a marca da linha que o meu boneco está fazendo (estado 4/5)
void FrAvatarMotionDlg::DrawChatMark983(FrListItem* pItem)
{
	ntAvatar* pAvatar = MYAVATAR983();
	if (m_selected == pItem->idx)
	{
		int state = pAvatar->m_motionType983; // 983 +0x784 (tipo de motion), nao o m_state +0xf0 (983 +0x158)
		if (state == 4 || state == 5)
		{
			m_iconRect.y = m_selected * 24.0 + m_pMotionList->GetRect().y;
			g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pHoverBmp, m_hoverRect, 0xffffffff, 0);
			g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pPlayBmp, m_iconRect, 0xffffffff, 0);
		}
	}
}

// 0047fca0: no lobby, a marca da linha escolhida (play × stop)
void FrAvatarMotionDlg::DrawLobbyMark983(FrListItem* pItem)
{
	if (m_selected == pItem->idx)
	{
		m_iconRect.y = m_selected * 24.0 + m_pMotionList->GetRect().y;
		g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pHoverBmp, m_hoverRect, 0xffffffff, 0);
		g_pFresh->GetManager()->GetGDI()->DrawTexture(m_bPlaying ? m_pPlayBmp : m_pStopBmp, m_iconRect, 0xffffffff, 0);
	}
}

// 00480270: peças do personagem (no lobby, as do personagem em uso no CSharedDoc)
const char* FrAvatarMotionDlg::GetSetPartsInfo983()
{
	if (m_flags & FLAG_LOBBY)
	{
		sCharacterInfo* pInfo = Doc()->FindChar983(0xffffffff);
		if (pInfo)
			return (const char*)pInfo + 0xc;
		return NULL;
	}
	return (const char*)&MYAVATAR983()->GetCharacterInfo() + 0xc;
}

// 004802b0
void FrAvatarMotionDlg::DrawChatName983(const int* pIndex, const WPoint& pos)
{
	g_pFresh->GetManager()->PrintText(pos, 1, GetItem983(*pIndex).name.c_str(), 70.0f,
		(unsigned long)(unsigned char)(int)m_textAlpha << 24);
}

// 00480360
void FrAvatarMotionDlg::DrawLobbyName983(const int* pIndex, const WPoint& pos)
{
	if (m_bRandom && (int)m_list.size() - 1 == *pIndex)
		g_pFresh->GetManager()->GetGDI()->SetTextColor(0xff0000, 0xffffffff);
	g_pFresh->GetManager()->PrintText(pos, 1, GetItem983(*pIndex).name.c_str(), 70.0f,
		(unsigned long)(unsigned char)(int)m_textAlpha << 24);
}

// 00480450
void FrAvatarMotionDlg::OnProc(const float deltaTime)
{
	if (m_slide & 8)
		SlideOpen983(deltaTime);
	else if (m_slide & 4)
		SlideClose983(deltaTime);
	ProcHover983(deltaTime);
	if (m_textAlpha != 255.0)
	{
		if (255.0 > m_textAlpha)
			m_textAlpha = m_textAlpha + 5.0;
		else
			m_textAlpha = 255.0f;
	}
	if (g_input->GetDown("ESCAPE", true) == 1)
		Close(true);
}

// 00480590
void FrAvatarMotionDlg::PlayLobby983(const int* pIndex)
{
	int index = *pIndex;
	if (index == (int)m_list.size() - 1)
	{
		if (m_bRandom)
			index = (int)Random((float)(int)(m_setList.size() + m_basicList.size()), (float)(index - 1));
		else
		{
			AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x196, 0, 0, 0, 0, 0);
			return;
		}
	}
	AfxGetTask()->GetActor("LobbyAvatar")
		<< MsgObject(NULL, 0x199, (int)GetItem983(index).motion.c_str(), *pIndex, m_bRandom, 0, 0);
}

// 00480730
void FrAvatarMotionDlg::PlayChat983(const int* pIndex)
{
	AfxGetTask()->GetActor("AvatarChat")
		<< MsgObject(NULL, 0x199, (int)GetItem983(*pIndex).motion.c_str(), 0, 0, 0, 0);
}

// 00480a60
void FrAvatarMotionDlg::DrawName983(const int* pIndex, const WPoint& pos)
{
	if (m_flags & FLAG_CHAT)
		DrawChatName983(pIndex, pos);
	else if (m_flags & FLAG_LOBBY)
		DrawLobbyName983(pIndex, pos);
}

// 00480a90
void FrAvatarMotionDlg::Play983(const int* pIndex)
{
	if (m_flags & FLAG_LOBBY)
		PlayLobby983(pIndex);
	else if (m_flags & FLAG_CHAT)
		PlayChat983(pIndex);
}

// 00480c30
void FrAvatarMotionDlg::DrawItem983(FrListItem* pItem)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	unsigned long color = pGI->GetTextColor();
	pGI->SetTextStyle(1);
	m_hoverRect.y = pItem->idx * 24.0 + m_pMotionList->GetRect().y;
	if (m_pMotionList->GetItemUnderCursor() == pItem)
	{
		g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pHoverBmp, m_hoverRect, 0xffffffff, 0);
		if (m_flags & FLAG_LOBBY)
		{
			m_iconRect.y = m_hoverRect.y;
			g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pPlayBmp, m_iconRect, 0xffffffff, 0);
		}
	}
	if ((m_flags & FLAG_CHAT) && m_bPlayed)
		DrawChatMark983(pItem);
	else if ((m_flags & FLAG_LOBBY) && m_bPlayed)
		DrawLobbyMark983(pItem);
	if (pItem->pData)
	{
		DrawName983(&pItem->idx, WPoint(m_textX, m_hoverRect.y + 7.0));
		g_pFresh->GetManager()->GetGDI()->SetTextColor(color, 0xffffffff);
		g_pFresh->GetManager()->GetGDI()->SetTextStyle(0);
	}
}

// 00480d70: só com item e com a lista aberta (m_slide bit 0); c142-2 caiu sem esse guarda (param 0)
void FrAvatarMotionDlg::OnMotionListOwnerDraw(int param)
{
	if (param && (m_slide & 1))
		DrawItem983((FrListItem*)param);
}

// 00480d90
void FrAvatarMotionDlg::SelectMotion983(const char* motion, const int* pUnused)
{
	if (!_stricmp("\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc", motion)) // "기본자세"
	{
		m_selected = -1;
		return;
	}
	int count = 0;
	// 00480500: a linha cuja animação começa pelo nome (sem o "끝" do fim)
	std::vector<sAvatarMotion983>::iterator it = std::find_if(m_list.begin(), m_list.end(),
		[&](const sAvatarMotion983& m) -> bool {
			++count;
			size_t len;
			if (!_stricmp("\xb3\xa1", motion + strlen(motion) - 2)) // "끝"
				len = strlen(motion) - 2;
			else
				len = strlen(motion);
			return strncmp(m.motion.c_str(), motion, len) == 0;
		});
	if (it != m_list.end())
		m_selected = count - 1;
}

// 00480e00
void FrAvatarMotionDlg::OnMotionListLBDown()
{
	FrListItem* pItem = m_pMotionList->GetItemUnderCursor();
	if (pItem && pItem->pData)
	{
		m_selected = pItem->idx;
		if (!m_bRandom && pItem->idx == (int)m_list.size() - 1)
		{
			m_bRandom = true;
			m_bPlayed = true;
			Play983(&pItem->idx);
		}
		else
		{
			m_bRandom = false;
			m_bPlayed = true;
			Play983(&pItem->idx);
		}
	}
}

// 00480fb0
sAvatarMotion983::sAvatarMotion983(int _type, std::string _name, std::string _motion)
	: type(_type), name(_name), motion(_motion)
{
}

// 00481070
void FrAvatarMotionDlg::FindMotion983(const char* motion, sAvatarMotion983* pOut)
{
	for (unsigned int i = 0; i < m_allMotions.size(); i++)
	{
		if (!_stricmp(m_allMotions[i].motion, motion))
		{
			pOut->motion = m_allMotions[i].motion;
			pOut->name = m_allMotions[i].name;
			return;
		}
	}
}

// 00481960: pede o esqueleto do boneco ao dono (msg 0x198) e monta a tabela do personagem sem as animações de conjunto
void FrAvatarMotionDlg::SetupMotions983(const unsigned long* pTid)
{
	MsgObject msg(NULL, 0x198, 0, 0, 0, 0, 0);
	if (m_flags & FLAG_CHAT)
		AfxGetTask()->GetActor("AvatarChat") << msg;
	else if (m_flags & FLAG_LOBBY)
		AfxGetTask()->GetActor("LobbyAvatar") << msg;
	m_pFrame = (CPetFrame*)msg.param1;
	if (m_pFrame)
	{
		m_motionCount = LOBBYMOTION983()->Count983(pTid);
		int count = 0;
		for (int i = 0; i < m_motionCount; i++)
		{
			const char* motion = LOBBYMOTION983()->Get983(pTid, &i);
			if (m_pFrame->FindMotionData(motion, true, NULL))
			{
				const char* name = LOBBYMOTION983()->GetName983(pTid, &i);
				int type = 5;
				if (!CreateCharacterOption::Instance()->IsSetMotion983(motion, &type))
				{
					sAvatarMotionName983 item;
					item.motion = motion;
					item.name = name;
					m_allMotions.push_back(item);
					count++;
				}
			}
		}
		m_motionCount = count;
	}
}

// 00481dc0
FrAvatarMotionDlg::FrAvatarMotionDlg()
	: m_pMotionList(NULL), m_motionCount(0), m_selected(-1), m_textAlpha(0.0f), m_textX(0.0f), m_hoverAlpha(0.0f),
	  m_pPlayBmp(NULL), m_pHoverBmp(NULL), m_bPlayed(false), m_bPlaying(true), m_b16a(false), m_bRandom(false),
	  m_flags(0), m_slide(8)
{
}

IMPLEMENT_OBJECT(FrAvatarMotionDlg, FrForm)

// 00481f00
FrAvatarMotionDlg::~FrAvatarMotionDlg()
{
}

// 00482010
void FrAvatarMotionDlg::ClearLists983()
{
	m_basicList.clear();
	m_setList.clear();
	m_bestList.clear();
	m_randomList.clear();
	m_shuffleList.clear();
	m_list.clear();
}

// 00482280: "best_motion_name_char<personagem>" do arquivo
bool FrAvatarMotionDlg::LoadBestMotion983(std::string file)
{
	m_bestSrc.clear();
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (!script)
		return false;
	if (!script->RunFile(file.c_str()))
		return false;
	int charType = 0;
	if (m_flags & FLAG_LOBBY)
	{
		if (RFLobbyDesktop::Instance983())
			charType = RFLobbyDesktop::Instance983()->GetCharType983();
	}
	else
		charType = MYCHARTYPE983();
	lua_system::lua_table_helper tbl = script->GetTable(MakeStr("best_motion_name_char%d", charType));
	lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
	for (int i = 0; i < lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);)
	{
		sAvatarMotion983 m;
		i++;
		FindMotion983(((LuaMotionChild983&)tbl).GetStrByIndex983(i), &m);
		m.type = 2;
		m_bestSrc.push_back(m);
	}
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
	return true;
}

// 004824c0
void FrAvatarMotionDlg::LoadBasic983()
{
	m_basicList.push_back(sAvatarMotion983(0, K2L_Compatibility("\xb1\xe2\xba\xbb"), // "기본"
		K2L_Compatibility("\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc"))); // "기본자세"
}

// 004825e0
void FrAvatarMotionDlg::LoadSet983()
{
	int charType = 0;
	if (m_flags & FLAG_LOBBY)
	{
		if (RFLobbyDesktop::Instance983())
			charType = RFLobbyDesktop::Instance983()->GetCharType983();
	}
	else
		charType = MYCHARTYPE983();
	for (unsigned int i = 0; i < CreateCharacterOption::Instance()->GetSetCount983(); i++)
	{
		sAvatarMotion983 m;
		std::string& motion = m.motion;
		std::string& name = m.name;
		CreateCharacterOption::Instance()->GetSetMotion983(charType, (const unsigned long*)GetSetPartsInfo983(), i, motion, name);
		if (motion.size() && name.size())
		{
			m.type = 1;
			m.motion = motion;
			m.name = name;
			m_setList.push_back(m);
		}
	}
}

// 004827b0
void FrAvatarMotionDlg::LoadBest983()
{
	for (unsigned int i = 0; i < m_bestSrc.size(); i++)
		m_bestList.push_back(sAvatarMotion983(m_bestSrc[i].type, m_bestSrc[i].name, m_bestSrc[i].motion));
}

// 00482a80: até 14 (12 no lobby) animações sorteadas da tabela, fora as best e as de conjunto
void FrAvatarMotionDlg::LoadRandom983()
{
	std::vector<sAvatarMotionName983> pool;
	int max;
	if (m_flags & FLAG_LOBBY)
		max = 12 - (int)m_setList.size() - (int)m_bestList.size();
	else
		max = 14 - (int)m_setList.size() - (int)m_bestList.size();
	for (unsigned int i = 0; i < m_allMotions.size(); i++)
	{
		unsigned int j;
		for (j = 0; j < m_bestList.size(); j++)
		{
			if (!_stricmp(m_allMotions[i].motion, m_bestList[j].motion.c_str()))
				break;
		}
		if (j < m_bestList.size())
			continue;
		int type = 5;
		if (!CreateCharacterOption::Instance()->IsSetMotion983(m_allMotions[i].motion, &type))
			pool.push_back(m_allMotions[i]);
	}
	if (pool.empty())
		return;
	std::random_shuffle(pool.begin(), pool.end());
	for (int i = 0; i < max; i++)
	{
		if ((unsigned int)i > pool.size() - 1)
			break;
		sAvatarMotion983 m;
		m.type = 3;
		m.motion = pool[i].motion; // 983 00482db0: +0 -> motion, +4 -> name
		m.name = pool[i].name;
		m_randomList.push_back(m);
	}
}

// 00482fd0
void FrAvatarMotionDlg::LoadShuffle983()
{
	m_shuffleList.push_back(sAvatarMotion983(4, K2L_Compatibility("(\xb7\xa3\xb4\xfd)"), K2L_Compatibility(""))); // "(랜덤)"
}

// 004830f0
void FrAvatarMotionDlg::AppendList983(const std::vector<sAvatarMotion983>& src)
{
	for (unsigned int i = 0; i < src.size(); i++)
		m_list.push_back(src[i]);
}

// 00483170
void FrAvatarMotionDlg::LoadLists983()
{
	ClearLists983();
	LoadBasic983();
	LoadSet983();
	LoadBest983();
	LoadRandom983();
	LoadShuffle983();
}

// 004831a0
void FrAvatarMotionDlg::BuildList983()
{
	m_list.clear();
	if (m_flags & FLAG_LOBBY)
		AppendList983(m_basicList);
	AppendList983(m_setList);
	AppendList983(m_bestList);
	AppendList983(m_randomList);
	if (m_flags & FLAG_LOBBY)
		AppendList983(m_shuffleList);
	for (unsigned int i = 0; i < m_list.size(); i++)
		m_pMotionList->AddItem(&m_list[i]);
}

// 004832a0
bool FrAvatarMotionDlg::OnInit()
{
	LoadBestMotion983("motiondlg.lua");
	LoadLists983();
	BuildList983();
	InitHoverRect983();
	CalcTextX983();
	LoadBitmaps983();
	InitIconRect983();
	unsigned long flags = m_flags;
	AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x197, (int)&m_bRandom, 0, 0, 0, 0);
	m_flags = flags;
	return FrForm::OnInit();
}

// 00483370
void FrAvatarMotionDlg::Refresh983(const unsigned long* pTid)
{
	m_allMotions.clear();
	SetupMotions983(pTid);
	m_pMotionList->ClearItem();
	if (!m_allMotions.empty())
	{
		unsigned long flags = m_flags;
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x197, (int)&m_bRandom, 0, 0, 0, 0);
		m_flags = flags;
		LoadBestMotion983("motiondlg.lua");
		LoadLists983();
		BuildList983();
		SetRect(WRect(m_rect.x, m_rect.y, m_rect.w, m_list.size() * 24.0 + 25.0));
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// B112: CBGMPlayList983 (include/jp/bgmplaylist.h), o pedaço de BGM do fim da unidade (004835d0–00484000)

// CSoundManager: nome do BGM tocando (+0x24, protegido)
struct CSoundManagerJp983 : CSoundManager
{
	const char* GetBGMName983() { return m_bgmName; }
};
// FrJukeBoxDlg do 983 (frente A): escolhe na lista do jukebox a faixa com esse nome (007ba810; não usa o this)
class FrJukeBoxDlgJp983
{
public:
	void SelectBGM983(const char* name);
};

// 007ba810 (thiscall ret 4; a faixa vem de FrJukeBoxDlg::SelectBGMByName, que ja e igual ao 983)
void FrJukeBoxDlgJp983::SelectBGM983(const char* name)
{
	FrJukeBoxDlg::SelectBGMByName(name);
}

inline void CBGMPlayList983::SelectJukeBox983(const char* name)
{
	((FrJukeBoxDlgJp983*)m_pJukeBox)->SelectBGM983(name);
}

__forceinline void CBGMPlayList983::PlayCurrent983()
{
	g_audio->PlayBGM(m_list[m_index].c_str(), true, true);
}

// 004835d0
void CBGMPlayList983::Play983()
{
	if (m_list.empty())
	{
		if (g_audio->IsPlayingBGM())
			g_audio->StopBGM(true);
		return;
	}
	PlayIndex983();
}

// 00483680: o BGM tocando é o da lista (ou um dos dois efeitos que tocam por cima)
bool CBGMPlayList983::IsCurrentBGM983()
{
	bool bResult = true;
	if (_stricmp(((CSoundManagerJp983*)g_audio)->GetBGMName983(), m_list[m_index].c_str()) != 0)
	{
		bResult = false;
		m_bJukeBoxMode = false;
	}
	if (_stricmp(((CSoundManagerJp983*)g_audio)->GetBGMName983(), "enchant_down.mp3") == 0)
		bResult = true;
	if (_stricmp(((CSoundManagerJp983*)g_audio)->GetBGMName983(), "caddie_return.mp3") == 0)
		bResult = true;
	return bResult;
}

// 00483780: a cada quadro (CProjectG::Process passa &dt, não lido); acabou a faixa, passa à próxima
void CBGMPlayList983::Process983(const float&)
{
	if (m_list.empty())
		return;
	if (!IsCurrentBGM983())
		return;
	if (g_audio->IsPlaying(m_list[m_index].c_str()))
		return;
	m_index++;
	if (m_list.size() <= m_index)
		m_index = 0;
	if (m_pJukeBox)
		SelectJukeBox983(m_list[m_index].c_str());
	PlayCurrent983();
}

// 00483940
void CBGMPlayList983::Load983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("BGMlist.lua");
	lua_system::lua_table_helper tbl = script->GetTable("default_bgm");
	m_defaultBGM = ((LuaHelper983&)tbl).GetChecked983<const char*>("defaultBGM");
	lua_system::lua_script_manger::GetInstance()->DeleteScript(script);
}

// 00483d00
CBGMPlayList983::CBGMPlayList983()
	: m_index(0), m_pJukeBox(NULL), m_bJukeBoxMode(false)
{
	Load983();
}

// 00483d90 (do atexit 00cf96b0)
CBGMPlayList983::~CBGMPlayList983()
{
	Clear983();
}

// 00483d60
void CBGMPlayList983::Clear983()
{
	m_list.clear();
	m_index = 0;
}

// 00483df0 (destrutor no atexit: 00483d90)
CBGMPlayList983* CBGMPlayList983::Instance()
{
	static CBGMPlayList983 s_instance;
	return &s_instance;
}

// 00483ef0
void CBGMPlayList983::Add983(const char* name)
{
	if (name)
	{
		m_list.push_back(name);
		m_index = m_list.size() - 1;
	}
}

// 00483fc0
void CBGMPlayList983::SetBGM983(const char* name)
{
	if (name)
	{
		Clear983();
		Add983(name);
		Play983();
	}
}

// 00483ff0 (o argumento não é lido: o HandleMsg do CLobbyMain passa 0 ou 1)
void CBGMPlayList983::PlayDefault983(bool)
{
	SetBGM983(m_defaultBGM);
}
