// JP 983: unidade 00546b60–0054f2fe (FrGrandPrixAwardDlg, tela de premiação do Grand Prix). Classe do 839/983, sem par no
// 645; ver include/jp/grandprixawarddlg.h. Sessão B72 (1º lote): construtor, destrutor, mapa de mensagens, OnProc e os
// contadores, temporizador, mostrar/esconder os grupos de controles, grandprix_award_ui_rewardbox e o desenho dos números.
// Sessão B73 (2º lote): OnInit, molduras de 9 peças, posições do pódio/troféu e dois métodos da GrandPrixAwardAnimation.
// Sessão B74 (3º lote): GrandPrixAwardAnimation inteira (construtor, destrutor, LoadItemLayout983, SetRewards983, Draw983) e
// PreOpen983. Sessão B75: ShowNotify983 (o retorno 00671310 é da A, no golfrule.cpp). Faltam os moldes da STL.
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "icontentsdoc.h"
#include "taskmanager.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "soundmanager.h"
#include "commonutil.h"
#include "frarea.h"
#include "frstatic.h"
#include "packet.h"
#include "grandprixdocument.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"
#include "simpleoverlay.h"
#include "titlefontex.h"
#include "gamestringmanager.h"
#include "grandprixawarddlg.h"
#include "docofs983.h" // D864: crus do CSharedDoc ancorados no membro (VS 2022)

// CSharedDoc+0x8cb: uid da conta (ver gptyperoomdlg.cpp)
#define DOC983_MYUID() (*(unsigned long*)DOCPTR983(0x8cb))

// leituras do Lua como no 983 (ver holeinonemissiondlg.cpp e dummyplayeractor.cpp): por nome fora da classe; a tabela filha
// por nome (0049a4e0) volta como lua_table_helper (8 bytes: a pilha do 983 tem o lugar dela); por índice fora da classe
// (0049a000) ou embutido (GetInl983: o empurrar embutido + lua_tinker::pop fora, como no PreOpen983)
struct LuaChild983 : lua_system::lua_table_helper
{
	float GetByIndex(int index); // 0049a000
	void Push983(int index)
	{
		if (table.m_obj)
		{
			lua_pushnumber(state, (double)index);
			lua_gettable(state, table.m_obj->m_index);
			if (lua_type(state, -1) != LUA_TNIL)
			{
				lua_remove(state, -1);
				lua_rawgeti(state, table.m_obj->m_index, index);
				return;
			}
			lua_remove(state, -1);
		}
		lua_pushnil(state);
	}
	template <typename T>
	T GetInl983(int index)
	{
		Push983(index);
		return lua_tinker::pop<T>(state);
	}
};
struct LuaHelper983 : lua_system::lua_table_helper
{
	template <typename T>
	T Get(const char* name);
	LuaChild983 GetChild983(const char* name); // 0049a4e0
};
template <typename T>
T LuaHelper983::Get(const char* name)
{
	return table.get<T>(name);
}
#define LUA983_GET(tbl, type, name) ((LuaHelper983&)(tbl)).Get<type>(name)
#define LUA983_POS(tbl, name, index) ((LuaHelper983&)(tbl)).GetChild983(name).GetByIndex(index)
#define LUA983_POSI(tbl, name, index) ((LuaHelper983&)(tbl)).GetChild983(name).GetInl983<float>(index)

// registro do GrandPrixRankReward.iff (0x4c; opaco no itemmanager.h da frente A): colocação e cinco prêmios
struct sGPRankReward983
{
	int m_00;
	int m_04;
	unsigned long place; // +0x08
	unsigned long tid[5]; // +0x0c
	int count[5]; // +0x20
	int extra[5]; // +0x34
	int m_48;
};

// visão da FrWnd983 (SetF1ab38_983, 00be7950; o mesmo empréstimo da gptyperoomdlg.cpp)
class FrWnd983
{
public:
	static void SetF1ab38_983();
};

extern Fresh* g_pFresh; // 00f02340

// tela (00f02458; ver formbase.cpp): 800 × 600 sem a visão
class WView;
extern WView* g_pMainView983;
#define VIEWW983() (*(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x19c)))
#define VIEWH983() (*(float*)((char*)g_pMainView983 + VC10_OFS_BO(0x1a0)))
// (macros: as funções inline do formbase.cpp não são embutidas aqui)
#define VIEWWIDTH983() (g_pMainView983 ? VIEWW983() : 800.0f)
#define VIEWHEIGHT983() (g_pMainView983 ? VIEWH983() : 600.0f)

// 00e7519c: gravado pelo pacote 0x25c do CGolfTask (golftask.cpp, frente A), zerado no destrutor daqui
unsigned long g_gpAwardMatchTid983;

FrGrandPrixAwardDlg* FrGrandPrixAwardDlg::s_pInstance = NULL;
std::string FrGrandPrixAwardDlg::s_timerName = "grandprix_award_ui";

IMPLEMENT_OBJECT(FrGrandPrixAwardDlg, FrFormBase)

// handler comum das derivadas da FrFormBase (004fedd0), gravado como VI
#define ON_FRESH_CTRL983(name) \
	{ (FRESH_PFN)(FRESH_PFN_VI)(void (FrCmdTarget::*)(IObject*))&FrFormBase::OnCtrlCreated983, name, FRCMD_INIT, FrFuncVI },

BEGIN_FRESH_MSGMAP(FrGrandPrixAwardDlg, FrFormBase)

ON_FRESH_CTRL983("common_area")
ON_FRESH_CTRL983("area_bg1")
ON_FRESH_CTRL983("area_bg2")
ON_FRESH_CTRL983("area_bg3")
ON_FRESH_CTRL983("area_bg4")
ON_FRESH_CTRL983("outback0")
ON_FRESH_CTRL983("outback1")
ON_FRESH_CTRL983("outback2")
ON_FRESH_CTRL983("outback3")
ON_FRESH_CTRL983("outback4")
ON_FRESH_CTRL983("outback5")
ON_FRESH_CTRL983("outback6")
ON_FRESH_CTRL983("outback7")
ON_FRESH_CTRL983("outback8")
ON_FRESH_CTRL983("inback0")
ON_FRESH_CTRL983("inback1")
ON_FRESH_CTRL983("inback2")
ON_FRESH_CTRL983("inback3")
ON_FRESH_CTRL983("inback4")
ON_FRESH_CTRL983("inback5")
ON_FRESH_CTRL983("inback6")
ON_FRESH_CTRL983("inback7")
ON_FRESH_CTRL983("inback8")
ON_FRESH_CTRL983("fbar0")
ON_FRESH_CTRL983("fbar1")
ON_FRESH_CTRL983("rewardlist")
ON_FRESH_CTRL983("char1")
ON_FRESH_CTRL983("char2")
ON_FRESH_CTRL983("char3")
ON_FRESH_CTRL983("myrank123")
ON_FRESH_CTRL983("myrank4")
ON_FRESH_CTRL983("char1_name")
ON_FRESH_CTRL983("char2_name")
ON_FRESH_CTRL983("char3_name")
ON_FRESH_VI("rewardlist", FRCMD_OWNERDRAW, FrGrandPrixAwardDlg::OnRewardListDraw)
ON_FRESH_VI("common_area", FRCMD_OWNERDRAW, FrGrandPrixAwardDlg::OnCommonAreaDraw)
ON_FRESH_CTRL983("trophy")
ON_FRESH_CTRL983("trophy_attr1")
ON_FRESH_CTRL983("trophy_attr2")
ON_FRESH_CTRL983("trophy_other1")
ON_FRESH_CTRL983("trophy_other2")

END_FRESH_MSGMAP()

// 0054dc90
FrGrandPrixAwardDlg::FrGrandPrixAwardDlg()
{
	SetSubForm983(this);
	m_bEnd = false;
	m_pAnim = std::tr1::shared_ptr<GrandPrixAwardAnimation>(new GrandPrixAwardAnimation);
	m_dt = 0.0f;
	m_place = 0;
	m_bShowMain = false;
	m_bShowTrophy = false;
	m_retryTime = 2.0f;
	m_trophyTime = 0.0f;
	m_mainTime = 0.0f;
	m_rewards.clear();
	m_2b0 = 0.0f;
	m_2b4 = 1.0f;
	m_notifyCount = 0;
	m_endTime = 45.0f;
	m_bNotify = false;
	m_step = 0;

	m_mainCtrls.push_back(sCtrlShow983("area_bg1", 0));
	m_mainCtrls.push_back(sCtrlShow983("area_bg2", 0));
	m_mainCtrls.push_back(sCtrlShow983("area_bg3", 0));
	m_mainCtrls.push_back(sCtrlShow983("area_bg4", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback0", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback1", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback2", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback3", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback4", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback5", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback6", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback7", 0));
	m_mainCtrls.push_back(sCtrlShow983("outback8", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback0", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback1", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback2", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback3", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback4", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback5", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback6", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback7", 0));
	m_mainCtrls.push_back(sCtrlShow983("inback8", 0));
	m_mainCtrls.push_back(sCtrlShow983("fbar0", 0));
	m_mainCtrls.push_back(sCtrlShow983("fbar1", 0));
	m_mainCtrls.push_back(sCtrlShow983("rewardlist", 0));
	m_mainCtrls.push_back(sCtrlShow983("char1", 0));
	m_mainCtrls.push_back(sCtrlShow983("char2", 0));
	m_mainCtrls.push_back(sCtrlShow983("char3", 0));
	m_mainCtrls.push_back(sCtrlShow983("char1_name", 0));
	m_mainCtrls.push_back(sCtrlShow983("char2_name", 0));
	m_mainCtrls.push_back(sCtrlShow983("char3_name", 0));
	m_mainCtrls.push_back(sCtrlShow983("myrank123", 1));
	m_mainCtrls.push_back(sCtrlShow983("myrank4", 2));

	m_trophyCtrls.push_back(sCtrlShow983("trophy", 1));
	m_trophyCtrls.push_back(sCtrlShow983("trophy_attr1", 1));
	m_trophyCtrls.push_back(sCtrlShow983("trophy_attr2", 1));
	m_trophyCtrls.push_back(sCtrlShow983("trophy_other1", 2));
	m_trophyCtrls.push_back(sCtrlShow983("trophy_other2", 2));
}

// 0054da40
FrGrandPrixAwardDlg::~FrGrandPrixAwardDlg()
{
	if (CTimerManager::IsInstantiated())
		ClearTimer(s_timerName.c_str());
	g_gpAwardMatchTid983 = 0;
	ClearSubForm983();
}

// 00546b60
void FrGrandPrixAwardDlg::SetSubForm983(FrFormBase* pForm)
{
	s_pInstance = (FrGrandPrixAwardDlg*)pForm;
	FrFormBase::SetSubForm983(pForm);
}

// 00546b80
void FrGrandPrixAwardDlg::ClearSubForm983()
{
	s_pInstance = NULL;
	m_pSubForm = NULL;
}

// 00546ba0
bool FrGrandPrixAwardDlg::TickEnd983(float dt)
{
	if (m_endTime > 0.0f)
	{
		m_endTime -= dt;
		if (m_endTime < 0.0f)
		{
			m_endTime = 0.0f;
			return true;
		}
	}
	return false;
}

// 00546bf0
bool FrGrandPrixAwardDlg::TickTrophy983(float dt)
{
	if (m_bShowTrophy)
	{
		m_trophyTime -= dt;
		if (m_trophyTime < 0.0f)
		{
			m_trophyTime = 0.0f;
			return true;
		}
	}
	return false;
}

// 00546c40
bool FrGrandPrixAwardDlg::TickMain983(float dt)
{
	if (!m_bShowMain)
	{
		m_mainTime -= dt;
		if (m_mainTime < 0.0f)
		{
			m_mainTime = 0.0f;
			return true;
		}
	}
	return false;
}

// 00546c90: devolve true enquanto o tempo não acabou (e o zera) — assim no 983
bool FrGrandPrixAwardDlg::TickRetry983(float dt)
{
	m_retryTime -= dt;
	if (m_retryTime > 0.0f)
	{
		m_retryTime = 0.0f;
		return true;
	}
	return false;
}

// 00546cd0 (também pelo retorno do aviso, 00671310 no golfrule.cpp)
void FrGrandPrixAwardDlg::OnNotifyOk983()
{
	m_notifyCount += 1;
	m_bNotify = false;
	float time = m_notifyCount < 3 ? 0.2f : m_endDelay;
	m_retryTime = time;
}

// 00546d20
void GrandPrixAwardAnimation::Skip983(bool bSkip)
{
	if (bSkip && !m_bSkip)
	{
		if (m_phase == 0)
		{
			m_bSkip = true;
			m_time = m_skipTime0;
			return;
		}
		if (m_phase == 1)
			m_time = m_skipTime1;
		m_bSkip = true;
	}
}

// 00547390: posição de cada prêmio (5 por linha) a partir de "th_item_box_n"
void GrandPrixAwardAnimation::SetPadding983(int x, int y)
{
	m_padX = (float)x;
	m_padY = (float)y;
	const Bitmap* pBmp = g_pFresh->GetBitmap("th_item_box_n");
	if (pBmp)
	{
		for (int i = 0; i < m_itemCount; i++)
		{
			m_itemPos[i].x = (i % 5) * (pBmp->Width() + m_gapX) + (m_gapX + m_padX);
			m_itemPos[i].y = (pBmp->Height() + m_gapY) * (int)(i > 4) + (m_gapY + m_padY);
		}
	}
}

// 0054be50
GrandPrixAwardAnimation::GrandPrixAwardAnimation()
{
	m_bShow = false;
	m_bFinished = false;
	for (int i = 0; i < 10; i++)
	{
		m_smokeFrame[i] = 0.0f;
		// o 983 zera 10 pares a partir de +0x104 (só 3 cabem antes de m_phase)
		((sPos983*)m_pad983_104)[i].x = 0.0f;
		((sPos983*)m_pad983_104)[i].y = 0.0f;
		m_pCountFont[i] = std::tr1::shared_ptr<WTitleFontEx>((WTitleFontEx*)NULL);
		m_pPeriodBmp[i] = NULL;
		m_color[i] = 0xffffffff;
		m_iconW[i] = 0;
		m_iconH[i] = 0;
		m_slotBmp[i][0] = NULL;
		m_slotBmp[i][1] = NULL;
		m_slotBmp[i][2] = NULL;
	}
	m_phase = 0;
	m_elapsed = 0.0f;
	m_bSkip = false;
	m_time = 0.0f;
	m_pBoxN = NULL;
	m_gapX = 0.0f;
	m_pBoxO = NULL;
	m_gapY = 0.0f;
	m_itemCount = 0;
	m_itemX = 0.0f;
	m_itemW = 0;
	m_itemY = 0.0f;
	m_itemH = 0;
	m_offX = 0.0f;
	m_offY = 0.0f;
	m_readyX = 0.0f;
	m_readyY = 0.0f;
	m_emptyX = 0.0f;
	m_emptyY = 0.0f;
	m_smokeX = 0.0f;
	m_smokeY = 0.0f;
	m_digitX = 0.0f;
	m_digitY = 0.0f;
	m_periodX = 0.0f;
	m_periodY = 0.0f;
}

// 0054c030
GrandPrixAwardAnimation::~GrandPrixAwardAnimation()
{
}

// 00549410: grandprix_award_ui_rewardbox_item (sem teste do script antes do GetTable; apaga o script no fim)
void GrandPrixAwardAnimation::LoadItemLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("grandprix_award_ui.lua");
	lua_system::lua_table_helper tbl = script->GetTable("grandprix_award_ui_rewardbox_item");
	m_gapX = (float)LUA983_GET(tbl, int, "RewardItemPaddingX");
	m_gapY = (float)LUA983_GET(tbl, int, "RewardItemPaddingY");
	m_itemX = (float)LUA983_GET(tbl, int, "ItemPosX");
	m_itemY = (float)LUA983_GET(tbl, int, "ItemPosY");
	m_offX = (float)LUA983_GET(tbl, int, "ItemOffsetX");
	m_offY = (float)LUA983_GET(tbl, int, "ItemOffsetY");
	m_readyX = (float)LUA983_GET(tbl, int, "ReadyPosX");
	m_readyY = (float)LUA983_GET(tbl, int, "ReadyPosY");
	m_emptyX = (float)LUA983_GET(tbl, int, "EmptyPosX");
	m_emptyY = (float)LUA983_GET(tbl, int, "EmptyPosY");
	m_smokeX = (float)LUA983_GET(tbl, int, "SmokePosX");
	m_smokeY = (float)LUA983_GET(tbl, int, "SmokePosY");
	m_skipTime0 = LUA983_GET(tbl, float, "ReadyPlayTime");
	m_skipTime1 = LUA983_GET(tbl, float, "MaskPlayTime");
	m_digitX = LUA983_POS(tbl, "ItemDigitPos", 1);
	m_digitY = LUA983_POS(tbl, "ItemDigitPos", 2);
	m_periodX = LUA983_POS(tbl, "ItemPeriodImgPos", 1);
	m_periodY = LUA983_POS(tbl, "ItemPeriodImgPos", 2);
	m_itemW = LUA983_GET(tbl, int, "ItemWidth");
	m_itemH = LUA983_GET(tbl, int, "ItemHeight");
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// 0054cab0
void GrandPrixAwardAnimation::SetRewards983(const std::vector<sGPAwardReward983>& rewards)
{
	m_rewards = rewards;
	m_itemCount = m_rewards.size() > 5 ? 10 : 5;
	LoadItemLayout983();
	m_pSmoke = std::tr1::shared_ptr<CSimpleUI>(new CSimpleUI("[emoti_object.png"));
	m_pSmoke->Add("0", WRect(0.0f, 128.0f, 64.0f, 64.0f), false);
	m_pSmoke->Add("1", WRect(64.0f, 128.0f, 64.0f, 64.0f), false);
	m_pSmoke->Add("2", WRect(128.0f, 128.0f, 64.0f, 64.0f), false);
	m_pSmoke->Add("3", WRect(192.0f, 128.0f, 64.0f, 64.0f), false);
	m_pSmoke->Add("4", WRect(0.0f, 192.0f, 64.0f, 64.0f), false);
	m_pSmoke->Add("5", WRect(64.0f, 192.0f, 64.0f, 64.0f), false);
	m_pSmoke->Add("6", WRect(128.0f, 192.0f, 64.0f, 64.0f), false);
	m_pSmoke->Add("7", WRect(192.0f, 192.0f, 64.0f, 64.0f), false);
	m_pBoxN = g_pFresh->RegisterBitmap("th_item_box_n");
	m_pBoxO = g_pFresh->RegisterBitmap("th_item_box_o");
	for (int i = 0; i < m_itemCount; i++)
	{
		m_slotBmp[i][0] = g_pFresh->RegisterBitmap("th_item_mark1");
		std::string name;
		if (i < m_rewards.size())
		{
			if (IFF_ITEM_COMMON* pItem = Doc()->m_itemManager.FindCommonItem(m_rewards[i].typeId))
				name = pItem->Icon;
		}
		else
			name = "th_item_mark2";
		if (const Bitmap* pIcon = g_pFresh->GetManager()->GetBitmap("ITEMS", name.c_str()))
		{
			m_slotBmp[i][1] = pIcon;
			m_slotBmp[i][2] = pIcon;
			if (i < m_rewards.size())
			{
				// ícone maior que ItemWidth × ItemHeight: reduzido pelo lado maior
				if (pIcon->Width() > m_itemW || pIcon->Height() > m_itemH)
				{
					float scale;
					if ((int)(pIcon->Width() - pIcon->Height()) >= 0)
						scale = pIcon->Width() > m_itemW ? (float)m_itemW / pIcon->Width() : pIcon->Width() / (float)m_itemW;
					else
						scale = pIcon->Height() > m_itemH ? (float)m_itemH / pIcon->Height() : pIcon->Height() / (float)m_itemH;
					m_iconW[i] = (int)(pIcon->Width() * scale);
					m_iconH[i] = (int)(pIcon->Height() * scale);
				}
				else
				{
					m_iconW[i] = pIcon->Width();
					m_iconH[i] = pIcon->Height();
				}
			}
			else
			{
				m_iconW[i] = pIcon->Width();
				m_iconH[i] = pIcon->Height();
			}
		}
		if (i < m_rewards.size())
		{
			if (m_rewards[i].kind == 0)
				m_pCountFont[i] = std::tr1::shared_ptr<WTitleFontEx>(new CWindFont);
			else if (m_rewards[i].kind == 1)
				m_pPeriodBmp[i] = g_pFresh->RegisterBitmap("recycle_in_period");
		}
	}
}

// 00547ca0
void GrandPrixAwardAnimation::Draw983(FrGraphicInterface* pGDI, float dt)
{
	if (!m_bShow)
		return;
	for (int i = 0; i < m_itemCount; i++)
	{
		if (i < m_rewards.size() && m_phase == 2)
		{
			if (m_pBoxO)
				pGDI->DrawTexture(m_pBoxO, WRect(m_itemPos[i].x, m_itemPos[i].y, m_pBoxO->Width(), m_pBoxO->Height()), 0xffffffff, 0);
		}
		else
		{
			if (m_pBoxN)
				pGDI->DrawTexture(m_pBoxN, WRect(m_itemPos[i].x, m_itemPos[i].y, m_pBoxN->Width(), m_pBoxN->Height()), 0xffffffff, 0);
		}
		int x = 0;
		int y = 0;
		switch (m_phase)
		{
		case 0:
			x = (int)(m_readyX + m_itemPos[i].x);
			y = (int)(m_readyY + m_itemPos[i].y);
			m_color[i] = 0xffffffff;
		case 1:
			if (m_elapsed > i * 0.1)
			{
				if (const Bitmap* pBmp = m_slotBmp[i][m_phase])
				{
					m_color[i] = 0xffffffff;
					if (m_phase == 1)
					{
						if (i < m_rewards.size())
						{
							m_color[i] = 0xa0202020;
							x = (int)(m_itemX + m_itemPos[i].x + (m_itemW - m_iconW[i]) / 2);
							y = (int)(m_itemY + m_itemPos[i].y + (m_itemH - m_iconH[i]) / 2);
							sGPAwardReward983& reward = m_rewards[i];
							if ((reward.typeId & 0xfc000000) == 0x18000000)
							{
								x += m_offX;
								y += m_offY;
							}
							if (Doc()->m_itemManager.GetCouponKind(reward.typeId))
							{
								x -= 10.0f;
								y -= 9.0f;
							}
						}
						else
						{
							x = (int)(m_emptyX + m_itemPos[i].x);
							y = (int)(m_emptyY + m_itemPos[i].y);
						}
						pGDI->DrawTexture(pBmp, WRect((float)x, (float)y, (float)m_iconW[i], (float)m_iconH[i]), m_color[i], 0);
					}
					else
						pGDI->DrawTexture(pBmp, WRect((float)x, (float)y, pBmp->Width(), pBmp->Height()), 0xffffffff, 0);
					Skip983(i == m_itemCount - 1);
				}
			}
			else if (const Bitmap* pMark = m_slotBmp[i][0])
			{
				pGDI->DrawTexture(pMark, WRect((float)(int)(m_readyX + m_itemPos[i].x), (float)(int)(m_readyY + m_itemPos[i].y),
					pMark->Width(), pMark->Height()), 0xffffffff, 0);
			}
			break;
		case 2:
			if (const Bitmap* pBmp = m_slotBmp[i][2])
			{
				float fx, fy;
				unsigned long color = 0xffffffff;
				if (i < m_rewards.size())
				{
					fx = m_itemX + m_itemPos[i].x + (float)((m_itemW - m_iconW[i]) / 2);
					fy = m_itemY + m_itemPos[i].y + (float)((m_itemH - m_iconH[i]) / 2);
					sGPAwardReward983& reward = m_rewards[i];
					if ((reward.typeId & 0xfc000000) == 0x18000000)
					{
						fx = m_offX + fx;
						fy = m_offY + fy;
					}
					if (Doc()->m_itemManager.GetCouponKind(reward.typeId))
					{
						fx = fx - 10.0;
						fy = fy - 9.0;
					}
					color = m_color[i];
				}
				else
				{
					fx = m_emptyX + m_itemPos[i].x;
					fy = m_emptyY + m_itemPos[i].y;
				}
				pGDI->DrawTexture(pBmp, WRect(fx, fy, (float)m_iconW[i], (float)m_iconH[i]), color, 0);
			}
			if (m_elapsed > i * (double)0.2f)
			{
				if (i < m_rewards.size())
				{
					if (m_smokeFrame[i] == 0.0f)
						g_audio->PlaySfx("play_fail_item");
					m_color[i] = 0xffffffff;
					if (CSimpleUI* pSmoke = m_pSmoke.get())
					{
						if (m_smokeFrame[i] >= 0.0f)
						{
							int frame = (int)m_smokeFrame[i];
							if (frame < 8)
							{
								pSmoke->Render(MakeStr("%d", frame), m_smokeX + m_itemPos[i].x, m_smokeY + m_itemPos[i].y, 1.5f, 0);
								m_smokeFrame[i] += dt * 15.0;
							}
						}
					}
					sGPAwardReward983& reward = m_rewards[i];
					if (reward.kind == 0)
					{
						if (WTitleFontEx* pFont = m_pCountFont[i].get())
						{
							x = (int)(m_digitX + m_itemPos[i].x);
							y = (int)(m_digitY + m_itemPos[i].y);
							pFont->Print((float)x, (float)y, MakeStr("%d", reward.count), TFA_CENTER, 0.0f, 1.0f);
						}
					}
					else if (reward.kind == 1)
					{
						if (m_pPeriodBmp[i])
							pGDI->DrawTexture(m_pPeriodBmp[i], WRect((float)(int)(m_periodX + m_itemPos[i].x),
								(float)(int)(m_periodY + m_itemPos[i].y), m_pPeriodBmp[i]->Width(), m_pPeriodBmp[i]->Height()),
								0xffffffff, 0);
					}
					if ((int)m_smokeFrame[m_rewards.size() - 1] >= 8)
						m_bFinished = true;
				}
				if (m_rewards.size() == 0 && i == m_itemCount - 1)
					m_bFinished = true;
			}
			else
				m_color[i] = i < m_rewards.size() ? 0xa0202020 : 0xffffffff;
			break;
		}
	}
	if (m_bSkip && m_time > 0.0f)
	{
		m_time -= dt;
		if (m_time <= 0.0f)
		{
			if (m_phase < 2)
			{
				m_elapsed = 0.0f;
				m_phase++;
			}
			m_time = 0.0f;
			m_bSkip = false;
		}
	}
	m_elapsed += dt;
}

// 00547ac0
bool FrGrandPrixAwardDlg::IsMeOnPodium983()
{
	_gateway::_grandprix::CGrandPrixDocument* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);
	if (Doc()->m_roomInfo.gpTypeId983)
	{
		std::vector<sGPAward983>& awards = pDoc->GetAwards983();
		for (unsigned int i = 0; i < awards.size(); i++)
		{
			sGPAward983& award = awards[i];
			if (award.uid == DOC983_MYUID())
				return award.place < 4;
		}
	}
	return false;
}

// número da colocação (as três chamadas da 00547b40: inteiros antes, escala por uma cópia)
#define PRINTRANK983(pos, align) \
	{ \
		int x = (int)(pos).x; \
		int y = (int)(pos).y; \
		float scale = (pos).scale; \
		m_pRankFont->Print((float)x, (float)y, MakeStr("%d", m_place), align, 0.0f, scale); \
	}

// 00547b40: números da colocação (fora do pódio na tela principal; no troféu, o do pódio centrado e o de fora à esquerda)
void FrGrandPrixAwardDlg::OnCommonAreaDraw(int param)
{
	if (m_bShowMain && m_place > 3 && m_pRankFont)
		PRINTRANK983(m_myRankPos, TFA_CENTER)
	if (m_bShowTrophy && m_pRankFont)
	{
		if (m_place > 3)
			PRINTRANK983(m_trophyPos4, TFA_LEFT)
		else if (m_place)
			PRINTRANK983(m_trophyPos123, TFA_CENTER)
	}
}

// 00548d10
void FrGrandPrixAwardDlg::StartTimer983(float time, std::string name)
{
	AddTimer(time, name.empty() ? s_timerName.c_str() : name.c_str());
}

// 00548da0
void FrGrandPrixAwardDlg::OnRewardListDraw(int param)
{
	if (m_pAnim)
		m_pAnim->Draw983(GDI(), m_dt);
}

// 00549210
void FrGrandPrixAwardDlg::OnTimer()
{
	// D861: era o cru +0x154 (VC10); no cl novo m_timerName fica em 0x134 -> o membro protegido da CTimerEvent
	std::string& name = m_timerName;
	if (name == s_timerName)
	{
		m_step = 2;
		m_mainTime = m_rewardDelay;
		ClearTimer(s_timerName.c_str());
		name.clear();
	}
}

// 00549290
void FrGrandPrixAwardDlg::LoadRewardBox983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
	{
		script->RunFile("grandprix_award_ui.lua");
		lua_system::lua_table_helper tbl = script->GetTable("grandprix_award_ui_rewardbox");
		m_boxW = LUA983_GET(tbl, int, "box_width");
		m_boxH = LUA983_GET(tbl, int, m_rewards.size() > 5 ? "box_height2" : "box_height1");
		m_boxX = LUA983_POS(tbl, "box_pos", 1) + m_originX;
		m_boxY = LUA983_POS(tbl, "box_pos", 2) + m_originY;
		m_padW = LUA983_GET(tbl, int, "padding_w");
		m_padH = LUA983_GET(tbl, int, "padding_h");
		m_rewardDelay = LUA983_GET(tbl, float, "rewardbox_show_delaytime");
	}
}

// 00549a90
void FrGrandPrixAwardDlg::ShowMain983(bool bShow, bool bFade)
{
	GrandPrixAwardAnimation* pAnim = m_pAnim.get();
	m_bShowMain = bShow;
	pAnim->SetShow983(bShow);
	if (bShow)
		g_audio->PlayBGM("Grandprix_Result_2.mp3", false, true);
	for (unsigned int i = 0; i < m_mainCtrls.size(); i++)
	{
		sCtrlShow983& ctrl = m_mainCtrls[i];
		bool bOk = true;
		switch (ctrl.cond)
		{
		case 0:
			bOk = true;
			break;
		case 1:
			bOk = m_place <= 3;
			break;
		case 2:
			bOk = m_place > 3;
			break;
		}
		if (bFade && bOk)
			FadeOutCtrl983(this, ctrl.name.c_str(), !bShow);
		else
			ShowCtrl983(this, ctrl.name.c_str(), bOk ? bShow : false);
	}
}

// 00549c20
void FrGrandPrixAwardDlg::ShowTrophy983(bool bShow, bool bFade)
{
	m_bShowTrophy = bShow;
	for (unsigned int i = 0; i < m_trophyCtrls.size(); i++)
	{
		sCtrlShow983& ctrl = m_trophyCtrls[i];
		bool bOk = true;
		switch (ctrl.cond)
		{
		case 0:
			bOk = true;
			break;
		case 1:
			bOk = m_place <= 3;
			break;
		case 2:
			bOk = m_place > 3;
			break;
		}
		if (m_place == 0)
			bOk = false;
		if (bFade && bOk)
			FadeOutCtrl983(this, ctrl.name.c_str(), !bShow);
		else
			ShowCtrl983(this, ctrl.name.c_str(), bOk ? bShow : false);
	}
}

// JP 983: o retorno do aviso é o CGolfRule::OnGPAwardNotifyResult983 (00671310, golfrule.cpp da frente A), mas a classe inteira
// do CGolfRule só existe no golfrule.cpp (fora dele o golfrule.h é a visão IActor) e o método é protegido. Visão de empréstimo
// com a mesma base (IActor + FrCmdTarget em +0x18, como o CTaskMain): o ponteiro de membro sai igual ({00671310, -0x18, 0, 0}).
class CGolfRuleNotifyJp : public IActor, public FrCmdTarget
{
public:
	bool OnGPAwardNotifyResult983(int result, FrForm* pForm); // 00671310 = CGolfRule::OnGPAwardNotifyResult983
};

// 00549d80: aviso "notify_ok2" das 3 tentativas (0 = o torneio com o ícone, 1 = a taxa, 2 = o próximo torneio); sem texto,
// passa direto à próxima
void FrGrandPrixAwardDlg::ShowNotify983()
{
	Bitmap* pBmp = NULL;
	std::string text;
	switch (m_notifyCount)
	{
	case 0:
		if (m_pMatch && m_place <= 3)
		{
			pBmp = (Bitmap*)g_pFresh->GetBitmap(m_pMatch->Icon[0]);
			text = MakeStr(GetGameString(0x7551), m_pMatch->Name);
		}
		break;
	case 1:
		if (m_ratio > 0.0f)
			text = MakeStr(GetGameString(0x754d), m_class, (int)(m_ratio * 100.0));
		break;
	case 2:
		if (m_pNextItem)
			text = MakeStr(GetGameString(0x754e), m_pNextItem->pInfo->name);
		break;
	}
	if (text.size() != 0)
	{
		FrFormBase* pDlg = CreateForm<FrFormBase>(g_pFresh->GetManager(), NULL, "notify_ok2", NULL);
		if (pDlg)
		{
			if (pBmp)
				pDlg->AddBitmap983(pBmp);
			pDlg->SetMessage(text.c_str(), false);
			pDlg->SetCaption(GetGameString(0x7553));
			pDlg->Open((FRESH_PFN_RESULT)&CGolfRuleNotifyJp::OnGPAwardNotifyResult983, 0x803);
		}
	}
	else
		OnNotifyOk983();
}

// 0054a2e0
void FrGrandPrixAwardDlg::OnProc(const float deltaTime)
{
	m_dt = deltaTime;
	if (TickEnd983(deltaTime))
		m_bEnd = true;
	if (m_step == 1)
	{
		if (TickTrophy983(deltaTime))
		{
			ShowTrophy983(false, true);
			return;
		}
	}
	else if (m_step == 2)
	{
		if (TickMain983(deltaTime))
			ShowMain983(true, true);
		if (!m_bEnd && !m_bNotify && m_pAnim->m_bFinished)
		{
			if (TickRetry983(deltaTime))
			{
				if (m_notifyCount < 3)
				{
					m_bNotify = true;
					m_retryTime = 1.0f;
					ShowNotify983();
					return;
				}
				m_bEnd = true;
			}
		}
	}
}

// 0054a9f0: fundos ("area_bg%d" com bg_file_name) e posições do grandprix_award_ui_main (somadas à origem, o canto do
// area_bg1) e do grandprix_award_ui_cutscene (somadas ao "trophy_pos")
bool FrGrandPrixAwardDlg::PreOpen983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("grandprix_award_ui.lua");
	lua_system::lua_table_helper tbl = script->GetTable("grandprix_award_ui_main");
	lua_system::lua_table_helper cut = script->GetTable("grandprix_award_ui_cutscene");
	int bgCount = LUA983_GET(tbl, int, "bg_file_count");
	const char* bgName = LUA983_GET(tbl, const char*, "bg_file_name");
	int bgX = (int)LUA983_POS(tbl, "bg_pos", 1);
	int bgY = (int)LUA983_POS(tbl, "bg_pos", 2);
	for (int i = 0; i < bgCount; i++)
	{
		FrArea* pArea = GetCtrl983<FrArea>(MakeStr("area_bg%d", i + 1));
		if (pArea)
		{
			if (const Bitmap* pBmp = g_pFresh->RegisterBitmap(MakeStr(bgName, i + 1)))
			{
				pArea->SetBgImg(pBmp);
				WRect rect(pArea->GetRect());
				rect.x += bgX;
				rect.y += bgY;
				pArea->SetRect(rect);
				if (i == 0)
				{
					m_originX = rect.x;
					m_originY = rect.y;
				}
			}
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if (i < 3)
		{
			m_charPos[i].x = LUA983_POSI(tbl, MakeStr("char%d_pic_pos", i + 1), 1) + m_originX;
			m_charPos[i].y = m_originY + LUA983_POSI(tbl, MakeStr("char%d_pic_pos", i + 1), 2);
			m_namePos[i].x = LUA983_POSI(tbl, MakeStr("char%d_name_pos", i + 1), 1) + m_originX;
			m_namePos[i].y = m_originY + LUA983_POSI(tbl, MakeStr("char%d_name_pos", i + 1), 2);
		}
		// i = 3 cai em m_myRankOutPos (+0x1cc), logo depois das três
		m_myRankInPos[i].x = LUA983_POSI(tbl, MakeStr("myrank%d_pic_pos", i + 1), 1) + m_originX;
		m_myRankInPos[i].y = m_originY + LUA983_POSI(tbl, MakeStr("myrank%d_pic_pos", i + 1), 2);
	}
	m_myRankPos.x = LUA983_POSI(tbl, "rank_digit_pos", 1) + m_originX;
	m_myRankPos.y = m_originY + LUA983_POSI(tbl, "rank_digit_pos", 2);
	m_myRankPos.scale = LUA983_GET(tbl, float, "rank_digit_scale");
	m_endDelay = LUA983_GET(tbl, float, "close_delaytime");
	int x = (int)(LUA983_POSI(cut, "trophy_pos", 1) + m_originX);
	int y = (int)(m_originY + LUA983_POSI(cut, "trophy_pos", 2));
	m_trophyPicPos.x = (float)x + LUA983_POSI(cut, "trophy_pic_pos", 1);
	m_trophyPicPos.y = (float)y + LUA983_POSI(cut, "trophy_pic_pos", 2);
	for (int i = 0; i < 2; i++)
	{
		m_attrPos[i].x = LUA983_POSI(cut, MakeStr("trophy_attr%d_pos", i + 1), 1);
		m_attrPos[i].y = LUA983_POSI(cut, MakeStr("trophy_attr%d_pos", i + 1), 2);
		m_otherPos[i].x = LUA983_POSI(cut, MakeStr("trophy_other%d_pos", i + 1), 1);
		m_otherPos[i].y = LUA983_POSI(cut, MakeStr("trophy_other%d_pos", i + 1), 2);
	}
	m_trophyPos123.x = (float)x + LUA983_POSI(cut, "rank_digit_pos", 1);
	m_trophyPos123.y = (float)y + LUA983_POSI(cut, "rank_digit_pos", 2);
	m_trophyPos4.x = (float)x + LUA983_POSI(cut, "rank_other_digit_pos", 1);
	m_trophyPos4.y = (float)y + LUA983_POSI(cut, "rank_other_digit_pos", 2);
	m_trophyPos123.scale = LUA983_GET(cut, float, "rank_digit_scale");
	m_trophyPos4.scale = LUA983_GET(cut, float, "rank_other_digit_scale");
	m_trophyTime = LUA983_GET(cut, float, "cutscene_playtime");
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
	return true;
}

// 0054b900: moldura de 9 peças (fmt%d, 0 = canto de cima à esquerda, 8 = de baixo à direita) que cobre (x, y, w, h)
void FrGrandPrixAwardDlg::SetRects983(const char* fmt, float x, float y, int w, int h)
{
	int left = 0;
	int top = 0;
	int right = 0;
	int bottom = 0;
	FrWnd* pWnd = GetCtrl983<FrArea>(MakeStr(fmt, 0));
	if (pWnd)
	{
		left = (int)pWnd->GetRect().w;
		top = (int)pWnd->GetRect().h;
	}
	pWnd = GetCtrl983<FrArea>(MakeStr(fmt, 8));
	if (pWnd)
	{
		right = (int)pWnd->GetRect().w;
		bottom = (int)pWnd->GetRect().h;
	}
	int midW = w - right - left;
	int midH = h - bottom - top;
	for (int i = 0; i < 9; i++)
	{
		pWnd = GetCtrl983<FrArea>(MakeStr(fmt, i));
		if (pWnd)
		{
			WRect rect;
			rect.w = pWnd->GetRect().w;
			rect.h = pWnd->GetRect().h;
			rect.x = x;
			rect.y = y;
			int col = i % 3;
			if (col != 0)
			{
				rect.x += left;
				if (col == 1)
					rect.w = (float)midW;
				else if (col == 2)
					rect.x += midW;
			}
			if (i > 2)
			{
				rect.y += top;
				if (i < 6)
					rect.h = (float)midH;
				else
					rect.y += midH;
			}
			pWnd->SetRect(rect);
		}
	}
}

// 0054baf0
void FrGrandPrixAwardDlg::LayoutTrophy983()
{
	FrArea* pArea = GetCtrl983<FrArea>("trophy");
	if (pArea)
	{
		if (m_place <= 3 && m_place != 0)
		{
			WRect rect = pArea->GetRect();
			rect.x = VIEWWIDTH983() * 0.5 - rect.w * 0.5;
			rect.y = VIEWHEIGHT983() * 0.5 - rect.h * 0.5;
			pArea->SetRect(rect);
			pArea->SetBgImg(MakeStr("gp_awards_winner_%d", m_place));
			pArea->SetVisible(true);
		}
		else
			pArea->SetVisible(false);
	}
	for (int i = 0; i < 2; i++)
	{
		FrArea* pWnd = GetCtrl983<FrArea>(MakeStr("trophy_attr%d", i + 1));
		if (pWnd)
		{
			if (m_place <= 3 && m_place != 0)
			{
				WRect rect = pWnd->GetRect();
				rect.x = VIEWWIDTH983() * 0.5 - rect.w * 0.5;
				rect.y = VIEWHEIGHT983() * 0.5 - rect.h * 0.5;
				rect.x += m_attrPos[i].x;
				rect.y = rect.y + m_attrPos[i].y;
				pWnd->SetRect(rect);
				pWnd->SetVisible(true);
			}
			else
				pWnd->SetVisible(false);
		}
		pWnd = GetCtrl983<FrArea>(MakeStr("trophy_other%d", i + 1));
		if (pWnd)
		{
			if (m_place > 3)
			{
				WRect rect = pWnd->GetRect();
				rect.x = VIEWWIDTH983() * 0.5 - rect.w * 0.5;
				rect.y = VIEWHEIGHT983() * 0.5 - rect.h * 0.5;
				rect.x += m_otherPos[i].x;
				rect.y = rect.y + m_otherPos[i].y;
				pWnd->SetRect(rect);
				pWnd->SetVisible(true);
			}
			else
				pWnd->SetVisible(false);
		}
	}
}

// 0054c360
void FrGrandPrixAwardDlg::LayoutMain983()
{
	WRect rect;
	WRect rect2;
	for (int i = 0; i < 3; i++)
	{
		FrArea* pChar = GetCtrl983<FrArea>(MakeStr("char%d", i + 1));
		if (pChar)
		{
			rect.w = pChar->GetRect().w;
			rect.h = pChar->GetRect().h;
			rect.x = m_charPos[i].x;
			rect.y = m_charPos[i].y;
			pChar->SetRect(rect);
		}
		FrStatic* pName = GetCtrl983<FrStatic>(MakeStr("char%d_name", i + 1));
		if (pName)
		{
			rect2.w = pName->GetRect().w;
			rect2.h = pName->GetRect().h;
			rect2.x = m_namePos[i].x;
			rect2.y = m_namePos[i].y;
			pName->SetRect(rect2);
		}
	}
	FrArea* pArea = GetCtrl983<FrArea>(MakeStr("myrank123"));
	if (pArea)
	{
		if (m_place < 4 && m_place > 0)
		{
			rect.x = pArea->GetRect().x;
			rect.y = pArea->GetRect().y;
			rect.w = pArea->GetRect().w;
			rect.h = pArea->GetRect().h;
			rect.x = m_myRankInPos[m_place - 1].x;
			rect.y = m_myRankInPos[m_place - 1].y;
			pArea->SetRect(rect);
			pArea->SetBgImg("gp_awards_me_in");
			pArea->SetVisible(true);
		}
		else
			pArea->SetVisible(false);
	}
	pArea = GetCtrl983<FrArea>(MakeStr("myrank4"));
	if (pArea)
	{
		if (m_place > 3)
		{
			rect2.w = pArea->GetRect().w;
			rect2.h = pArea->GetRect().h;
			rect2.x = m_myRankOutPos.x;
			rect2.y = m_myRankOutPos.y;
			pArea->SetRect(rect2);
			pArea->SetBgImg("gp_awards_me_out");
			pArea->SetVisible(true);
		}
		else
			pArea->SetVisible(false);
	}
	SetRects983("outback%d", m_boxX, m_boxY, m_boxW, m_boxH);
	FrArea* pBar = GetCtrl983<FrArea>("fbar0");
	if (pBar)
	{
		rect = pBar->GetRect();
		rect.x = m_padW + m_boxX;
		rect.y = m_padH + m_boxY;
		pBar->SetRect(rect);
		pBar = GetCtrl983<FrArea>("fbar1");
		if (pBar)
		{
			rect2.w = pBar->GetRect().w;
			rect2.h = pBar->GetRect().h;
			rect2.x = rect.w + rect.x;
			rect2.y = rect.y;
			pBar->SetRect(rect2);
		}
	}
	int x = (int)(m_padW + m_boxX);
	int y = (int)(m_padH + m_boxY + rect.h);
	int w = m_boxW - (m_padW * 2 + m_padW / 2);
	int h = (int)(m_boxH - (rect.h + (m_padH * 2 + m_padH / 2)));
	float fy = (float)y;
	float fx = (float)x;
	SetRects983("inback%d", fx, fy, w, h);
	FrWnd* pList = GetCtrl983<FrWnd>("rewardlist");
	if (pList)
	{
		if (m_pAnim)
			m_pAnim->SetPadding983(x, y);
		rect2.x = fx;
		rect2.y = fy;
		rect2.w = (float)w;
		rect2.h = (float)h;
		pList->SetRect(rect2);
	}
}

// 0054c7d0
void FrGrandPrixAwardDlg::UpdateTrophy983()
{
	LayoutTrophy983();
}

// 0054d110
bool FrGrandPrixAwardDlg::OnInit()
{
	FrWnd983::SetF1ab38_983();
	if (!Doc()->m_bSentHoleStat)
	{
		Doc()->m_bSentHoleStat = true;
		WSendPacket packet(6);
		packet.EncodeBuffer(&Doc()->m_holeStatistics[0], sizeof(sPangYaUserStatistics));
		packet.Send(TO_GAME);
	}
	EnableDrag(false);
	_gateway::_grandprix::CGrandPrixDocument* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);
	sRoomInfo& room = Doc()->m_roomInfo;
	sGPRoomItem983* pItem = pDoc->FindRoomItem983(room.gpTypeId983);
	if (room.gpTypeId983)
	{
		for (unsigned int i = 0; i < pDoc->GetAwards983().size(); i++)
		{
			sGPAward983& award = pDoc->GetAwards983()[i];
			int place = award.place;
			if (place > 3)
			{
				if (award.uid == DOC983_MYUID())
				{
					for (unsigned int j = 0; j < Doc()->m_rivalList.size(); j++)
					{
						sRivalData& rival = Doc()->m_rivalList[j];
						if (rival.uid == award.uid)
						{
							m_place = rival.rank;
							break;
						}
					}
				}
			}
			else
			{
				IFF_STRUCT::sChar* pChar = NULL;
				std::string name;
				if (award.bUser)
				{
					for (unsigned int j = 0; j < Doc()->m_rivalList.size(); j++)
					{
						sRivalData& rival = Doc()->m_rivalList[j];
						if (rival.uid == award.uid)
						{
							if (IFF_STRUCT::sChar* p = Doc()->m_itemManager.FindChar(rival.tidChar983))
								pChar = p;
							if (award.uid == DOC983_MYUID())
								m_place = rival.rank;
							name = rival.nickname;
							break;
						}
					}
				}
				else
				{
					IFF983::sGrandPrixAIOptionalData* pAI = Doc()->m_itemManager.FindGrandPrixAI983(award.uid);
					if (pAI)
					{
						if (IFF_STRUCT::sChar* p = Doc()->m_itemManager.FindChar(*(unsigned long*)((char*)pAI + 0x30) | 0x4000000))
							pChar = p;
						name = (char*)pAI + 8;
					}
				}
				FrArea* pArea = GetCtrl983<FrArea>(MakeStr("char%d", place));
				if (pArea && pChar)
					pArea->SetBgImg(MakeStr("w_%s", pChar->c.Icon));
				FrStatic* pName = GetCtrl983<FrStatic>(MakeStr("char%d_name", place));
				if (pName)
					pName->SetCaption(name.c_str());
			}
		}
		if (pItem)
		{
			if (sGPInfo983* pInfo = pItem->pInfo)
			{
				for (int i = 0; i < 5; i++)
				{
					if (pInfo->prize[i])
					{
						sReward983 reward = { pInfo->prize[i], pInfo->prizeExtra[i], pInfo->prizeCount[i] };
						m_rewards.push_back(reward);
					}
				}
			}
			if (m_place)
			{
				std::vector<IFF983::sGrandPrixRankReward>* pRank = Doc()->m_itemManager.FindGrandPrixRankReward983(room.gpTicket983);
				if (pRank)
				{
					for (unsigned int j = 0; j < pRank->size(); j++)
					{
						sGPRankReward983& rank = (sGPRankReward983&)(*pRank)[j];
						if (m_place == rank.place)
						{
							for (int k = 0; k < 5; k++)
							{
								if (rank.tid[k])
								{
									sReward983 reward = { rank.tid[k], rank.extra[k], rank.count[k] };
									m_rewards.push_back(reward);
								}
							}
							break;
						}
					}
				}
			}
		}
	}
	m_pMatch = Doc()->m_itemManager.FindMatch(g_gpAwardMatchTid983);
	unsigned long ticket = room.gpTicket983;
	int kind = (ticket >> 19) & 0x7f;
	if (kind >= 1 && kind <= 3 && pItem)
	{
		m_ratio = pDoc->GetClearRate983(kind, ticket);
		m_class = kind;
		m_gpName = pItem->pInfo->name;
	}
	else
	{
		m_class = 0;
		m_ratio = -1.0f;
	}
	if (pItem)
	{
		if (pItem->kind > 0)
			m_pNextItem = NULL;
		else
			m_pNextItem = pDoc->FindRoomItemByPrev983(room.gpTicket983);
	}
	else
		m_pNextItem = NULL;
	LoadRewardBox983();
	LayoutMain983();
	UpdateTrophy983();
	m_pAnim->SetRewards983(m_rewards);
	m_pRankFont = std::tr1::shared_ptr<CGrandPrixAwardRankFont>(new CGrandPrixAwardRankFont);
	m_step = 1;
	ShowMain983(false, false);
	ShowTrophy983(true, true);
	g_audio->PlayBGM("Grandprix_Result_1.mp3", true, true);
	return true;
}

// 0054f470 (depois da MakeInstance 0054f3e0 e do CreateForm<FrGrandPrixAwardDlg> 0054f310: fim da unidade)
FrGrandPrixAwardDlg* __fastcall FrGrandPrixAwardDlg::Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest)
{
	return CreateForm<FrGrandPrixAwardDlg>(pManager, pCmdDest, "GrandPrix_Award", NULL);
}
