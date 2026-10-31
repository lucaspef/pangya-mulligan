// JP 983: unidade grandprixopendlg (0055aa50–0055cd30): _gateway::_grandprix::FrGrandPrixOpenDlg, a janela "grandprixopen" do
// Grand Prix. Fonte reconstruída do 983 (classe nova, sem par no 645). Sessão B66: registro, construtor, destrutor, mapa de mensagens
// (10 entradas, 00e0bcc4; ponteiros gravados pelo inicializador 00cce620), OnInit/OnProc/OnMouseMove e os métodos; no fim, as peças
// do relógio que o desenho usa e cujo código ficou nesta unidade (0055cd30–0055ce76, inline do cabeçalho / primeiro uso).
#include "minatl.h"
#include "projectg.h"
#include "grandprixopendlg.h"
#include "grandprixtimebase.h"
#include "shareddocjp.h"
#include "gamestringmanager.h"
#include "taskmanager.h"
#include "packet.h"
#include "commonutil.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frarea.h"
#include "fredit.h"
#include "frstatic.h"
#include "bitmap.h"
#include "titlefontex.h"
#include "rftooltiphelper.h"
#include "../../shared/itemmanager.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"
#include <stdio.h>

extern Fresh* g_pFresh;

using namespace _gateway::_grandprix;

// JP 983: empréstimos (nomes meus; pedido B → A da B65/B66)
// 00bfb1d0: o `FrEdit::AddText` do 983 tem 5 argumentos (fredit.h da A declara 3)
class FrEditJp983 : public FrEdit
{
public:
	// A312: inline -> FrEdit::AddText de 5 argumentos (fredit.h); antes sem corpo = stub no exe (texto nunca aparecia)
	void AddText983(const char* text, bool a, bool b, unsigned long c, unsigned long d) { AddText(text, a, b, c, d); }
};
#define EDIT983(p) ((FrEditJp983*)(p))
// 00a17bd0: lista do GrandPrixRankReward.iff (mapa CItemManager+0x524); 00a17c10: há registro no mapa +0x504
// (GrandPrixSpecialHole.iff) — o Grand Prix de mapa sorteado
class CItemManagerGpOpenJp
{
public:
	std::vector<IFF983::sGrandPrixRankReward>* FindGrandPrixRankReward983(unsigned long tid);
	bool HasGrandPrixSpecialHole983(unsigned long tid);
};
#define ITEMMANAGERGP983() ((CItemManagerGpOpenJp*)&Doc()->m_itemManager)

// leitura por índice do lua_table_helper do 983 (004d5250 tabela filha, 0049a000 float de uma tabela); mesma visão do
// normallobbyroom.cpp
struct LuaIdx983 : lua_system::lua_table_helper
{
	lua_system::lua_table_helper GetTableByIndex(int index); // 004d5250
};
#define LUAIDX983(tbl) (*(LuaIdx983*)&(tbl))
struct LuaTable983 : lua_tinker::table
{
	float GetByIndex(int index); // 0049a000
};
inline LuaTable983& AsTable983(const lua_tinker::table& tbl) { return *(LuaTable983*)&tbl; }

static const char* s_gateway = "Gateway"; // 00e0bcb4

static char s_time2[4]; // 00e751c4: dois dígitos da hora/minuto (GPFormat2_983)

// ------------------------------------------------------------------------------------------------------------------------
// FrGrandPrixOpenDlg

namespace _gateway
{
namespace _grandprix
{
IMPLEMENT_OBJECT(FrGrandPrixOpenDlg, FrForm)

FrGrandPrixOpenDlg::FrGrandPrixOpenDlg()
	: m_pNumFont(NULL), m_pTimeFont(NULL), m_pStartTime(NULL), m_pFinishTime(NULL), m_pWarningMsg(NULL), m_pTitle(NULL),
	  m_pSpecialRule(NULL), m_pInfo(NULL)
{
	m_pClassBmp[0] = NULL;
	m_pClassBmp[1] = NULL;
	m_pClassBmp[2] = NULL;
	m_pClassBmp[3] = NULL;
	m_pMarkBmp[0] = NULL;
	m_pMarkBmp[1] = NULL;
	m_pMarkBmp[2] = NULL;
	m_pMarkBmp[3] = NULL;
	m_tipId = -1;
}

FrGrandPrixOpenDlg::~FrGrandPrixOpenDlg()
{
}

BEGIN_FRESH_MSGMAP(FrGrandPrixOpenDlg, FrForm)

ON_FRESH_VI("starttime", FRCMD_INIT, FrGrandPrixOpenDlg::OnStartTimeInit)
ON_FRESH_VI("finishtime", FRCMD_INIT, FrGrandPrixOpenDlg::OnFinishTimeInit)
ON_FRESH_VI("diff", FRCMD_INIT, FrGrandPrixOpenDlg::OnDiffInit)
ON_FRESH_VV("diff", FRCMD_OWNERDRAW, FrGrandPrixOpenDlg::OnDiffDraw)
ON_FRESH_VV("prize", FRCMD_OWNERDRAW, FrGrandPrixOpenDlg::OnPrizeDraw)
ON_FRESH_VI("warningmsg", FRCMD_INIT, FrGrandPrixOpenDlg::OnWarningMsgInit)
ON_FRESH_VI("title", FRCMD_INIT, FrGrandPrixOpenDlg::OnTitleInit)
ON_FRESH_VI("specialrule", FRCMD_INIT, FrGrandPrixOpenDlg::OnSpecialRuleInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrGrandPrixOpenDlg::OnOk)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrGrandPrixOpenDlg::OnCancelBtnUp983)

END_FRESH_MSGMAP()

void FrGrandPrixOpenDlg::Init983(const sGPEnter983* pEnter)
{
	m_state = pEnter->state;
	m_pInfo = pEnter->pInfo;
	m_pNumFont = (WTitleFontEx*)pEnter->m_08;
	m_pTimeFont = (WTitleFontEx*)pEnter->m_0c;
}

void FrGrandPrixOpenDlg::OnCancelBtnUp983()
{
	Close(FrCANCEL, true);
}

void FrGrandPrixOpenDlg::OnWarningMsgInit(int param)
{
	m_pWarningMsg = DYNAMIC_CAST(FrEdit, param);
	m_pWarningMsg->SetReadOnly(true);
}

void FrGrandPrixOpenDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrStatic, param);
}

void FrGrandPrixOpenDlg::OnSpecialRuleInit(int param)
{
	m_pSpecialRule = DYNAMIC_CAST(FrStatic, param);
}

void FrGrandPrixOpenDlg::OnDiffInit(int param)
{
	m_pDiff = DYNAMIC_CAST(FrArea, param);
}

void FrGrandPrixOpenDlg::OnStartTimeInit(int param)
{
	m_pStartTime = DYNAMIC_CAST(FrArea, param);
}

void FrGrandPrixOpenDlg::OnFinishTimeInit(int param)
{
	m_pFinishTime = DYNAMIC_CAST(FrArea, param);
}

// "ok": no estado 0 o aviso 0x753e / 9 ao "Gateway" (com o typeid do Grand Prix); no estado 2 o pacote 0x179
void FrGrandPrixOpenDlg::OnOk()
{
	switch (m_state)
	{
	case 0:
		{
			MsgObject msg(NULL, 0x753e, 9, m_pInfo->typeId, 0, 0, 0);
			AfxGetTask()->GetActor(s_gateway) << msg;
		}
		break;
	case 2:
		{
			WSendPacket packet((enumClientPacket)0x179);
			packet.Encode4(m_pInfo->typeId);
			packet.Send(TO_GAME);
		}
		break;
	}
	Close(FrOK, true);
}

// nível (selo pelo tipo do ticket), número (+0xac), mapa ("room_<curso>" ou o sorteado "room_map_17_01") e, no canal de evento, os
// horários de começo e fim (hora de 12 + minuto + "gp_time_am/pm")
void FrGrandPrixOpenDlg::OnDiffDraw()
{
	FrGraphicInterface* pGdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* pClass;
	const Bitmap* pMark;
	FrArea* pDiff = m_pDiff;
	switch ((m_pInfo->ticket >> 19) & 0x7f)
	{
	case 1:
		pClass = m_pClassBmp[1];
		pMark = m_pMarkBmp[1];
		break;
	case 2:
		pClass = m_pClassBmp[2];
		pMark = m_pMarkBmp[2];
		break;
	case 3:
		pClass = m_pClassBmp[3];
		pMark = m_pMarkBmp[3];
		break;
	default:
		pClass = m_pClassBmp[0];
		pMark = m_pMarkBmp[0];
		break;
	}
	const WRect& rc = pDiff->GetRect();
	pGdi->DrawTexture(pMark, WRect(rc.x, rc.y, (float)pMark->Width(), (float)pMark->Height()), 0xffffffff, 0);
	pGdi->DrawTexture(pClass, WRect(rc.x, rc.y + 30.0, (float)pClass->Width(), (float)pClass->Height()), 0xffffffff, 0);
	m_pNumFont->Print(rc.x + 66.0, rc.y + 79.0, MakeStr("%d", m_pInfo->m_ac), (eTITLEFONTALINE)0, 0.0f, 1.0f);

	const char* map;
	if (ITEMMANAGERGP983()->HasGrandPrixSpecialHole983(m_pInfo->ticket))
		map = "room_map_17_01";
	else
	{
		unsigned long course = m_pInfo->course;
		if (course >= 0x7f)
			course = 0x7f;
		IFF_STRUCT::sCourse* pCourse = Doc()->m_itemManager.FindCourse(course | 0x28000000);
		if (!pCourse)
			goto time;
		map = MakeStr("room_%s", pCourse->c.Icon);
	}
	{
		const Bitmap* pMap = g_pFresh->GetBitmap(map);
		if (pMap)
		{
			pGdi->DrawTexture(pMap, WRect(m_offset[4].x + rc.x, m_offset[4].y + rc.y, (float)pMap->Width(),
				(float)pMap->Height()), 0xffffffff, 0);
		}
	}

time:
	if (DOCJP()->GetGPChannel983() == 0)
		return;

	sGPInfo983* pInfo = m_pInfo;
	CGrandPrixHour hour;
	hour.Set(pInfo->start[4]);
	{
		float x = m_offset[0].x + GetRect().x;
		float y = m_offset[0].y + GetRect().y;
		GPFormat2_983(hour.Get(), s_time2);
		m_pTimeFont->Print(x, y, s_time2, (eTITLEFONTALINE)0, 0.0f, 1.0f);
		GPFormat2_983(pInfo->start[5], s_time2);
		m_pTimeFont->Print(x + 47.0, y, s_time2, (eTITLEFONTALINE)0, 0.0f, 1.0f);
		const Bitmap* pAmPm = g_pFresh->GetBitmap(hour.m_bPM == 1 ? "gp_time_pm" : "gp_time_am");
		pGdi->DrawTexture(pAmPm, WRect(x + 91.0, y + 6.0, (float)pAmPm->Width(), (float)pAmPm->Height()), 0xffffffff, 0);
	}
	pInfo = m_pInfo;
	hour.Set(pInfo->end[4]);
	{
		float x = m_offset[1].x + GetRect().x;
		float y = m_offset[1].y + GetRect().y;
		GPFormat2_983(hour.Get(), s_time2);
		m_pTimeFont->Print(x, y, s_time2, (eTITLEFONTALINE)0, 0.0f, 1.0f);
		GPFormat2_983(pInfo->end[5], s_time2);
		m_pTimeFont->Print(x + 47.0, y, s_time2, (eTITLEFONTALINE)0, 0.0f, 1.0f);
		const Bitmap* pAmPm = g_pFresh->GetBitmap(hour.m_bPM == 1 ? "gp_time_pm" : "gp_time_am");
		pGdi->DrawTexture(pAmPm, WRect(x + 91.0, y + 6.0, (float)pAmPm->Width(), (float)pAmPm->Height()), 0xffffffff, 0);
	}
}

void FrGrandPrixOpenDlg::OnProc(float dt)
{
	if (m_tip.size() != 0 && m_tipId != -1 && RFTooltipHelper::Instance983())
		RFTooltipHelper::Instance983()->Show983(0, std::string(m_tip.c_str()), 1, 0xb);
}

// regra especial (texto do 0055ecc0; sem ele e com m_86, 0x7547 + 0x18769), aviso pelo estado da linha e, só no canal de evento,
// as áreas dos horários
void FrGrandPrixOpenDlg::SetTexts983()
{
	int minCount = 1;
	if (m_pInfo)
	{
		minCount = m_pInfo->m_58;
		std::string rule = GPRuleText983(m_pInfo);
		if (rule.size() != 0)
			m_pSpecialRule->SetCaption(rule.c_str());
		else if (m_pInfo->m_86)
		{
			rule = GetGameString(0x7547);
			rule += GetGameString(0x18769);
			if (rule.size() != 0)
				m_pSpecialRule->SetCaption(rule.c_str());
		}
	}
	switch (m_state)
	{
	case 0:
		EDIT983(m_pWarningMsg)->AddText983(GetGameString(0x753b), false, true, 0, 0);
		EDIT983(m_pWarningMsg)->AddText983(GetGameString(0x753c), false, true, 0, 0);
		break;
	case 2:
		EDIT983(m_pWarningMsg)->AddText983(MakeStr(GetGameString(0x753a), minCount), false, true, 0, 0);
		break;
	case 3:
		EDIT983(m_pWarningMsg)->AddText983(GetGameString(0x7538), false, true, 0, 0);
		EDIT983(m_pWarningMsg)->AddText983(GetGameString(0x7550), false, true, 0, 0);
		break;
	case 4:
		EDIT983(m_pWarningMsg)->AddText983(GetGameString(0x7539), false, true, 0, 0);
		EDIT983(m_pWarningMsg)->AddText983(GetGameString(0x7550), false, true, 0, 0);
		break;
	}
	bool bEvent = DOCJP()->GetGPChannel983() != 0;
	if (m_pStartTime)
		m_pStartTime->SetVisible(bEvent);
	if (m_pFinishTime)
		m_pFinishTime->SetVisible(bEvent);
}

// dica com o nome do prêmio quando o mouse está sobre o retângulo; ao sair, fecha a dica deste prêmio
void FrGrandPrixOpenDlg::CheckTip983(int id, float x, float y, float w, float h, const std::string& text)
{
	if (m_mouse.x > x && m_mouse.x < x + w && m_mouse.y > y && m_mouse.y < y + h)
	{
		if (m_tipId == -1)
		{
			m_tipId = id;
			m_tip.assign(text, 0, std::string::npos);
		}
	}
	else if (m_tipId == id)
	{
		if (RFTooltipHelper::Instance983())
			RFTooltipHelper::Instance983()->Hide983();
		m_tipId = -1;
	}
}

// fator para caber em 64×64
static __forceinline float FitScale983(float w, float h)
{
	if ((int)(w - h) >= 0)
	{
		if (w > 64.0f)
			return (float)(64.0 / w);
		return (float)(w * 0.015625);
	}
	if (h > 64.0f)
		return (float)(64.0 / h);
	return (float)(h * 0.015625);
}

// as duas filas de prêmios: ícone (reduzido a 64), quantidade " %d" e a dica do nome; a 1ª fila tem o acerto da posição pelo tipo
// do item (o acerto fica de um item para o outro, como no 983)
void FrGrandPrixOpenDlg::OnPrizeDraw()
{
	FrGraphicInterface* pGdi = g_pFresh->GetManager()->GetGDI();
	pGdi->SetTextColor(0xffff0000, 0xffffffff);

	float baseX = m_offset[2].x + GetRect().x;
	float baseY = m_offset[2].y + GetRect().y;
	unsigned int n = m_prizes.size();
	float offX = 0.0f;
	float offY = 0.0f;
	for (unsigned int i = 0; i < n; i++)
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(m_prizes.at(i).pItem->Icon);
		if (!pBmp)
			continue;
		unsigned long tid = m_prizes[i].pItem->TypeId;
		switch (tid >> 26)
		{
		case 2:
		case 7:
		case 9:
			offY = -10.0f;
			break;
		case 4:
			offX = offY = 7.0f;
			break;
		case 5:
			offX = 4.0f;
			offY = -4.0f;
			break;
		case 0xe:
			offY = (tid & 0x3c00000) == 0x800000 ? -14.0f : -3.0f;
			break;
		case 0x10:
			offX = -2.0f;
			offY = -14.0f;
			break;
		case 0x1f:
			offX = 8.5f;
			break;
		}
		float w = (float)pBmp->Width();
		float h = (float)pBmp->Height();
		float scale = 1.0f;
		if (w > 64.0f || h > 64.0f)
			scale = FitScale983(w, h);
		float fi = (float)i;
		WRect rc(offX + baseX + fi * 64.0, offY + baseY, scale * w, scale * h);
		pGdi->DrawTexture(pBmp, rc, 0xffffffff, 0);
		m_pTicketFont->Print(baseX + 50.0 + m_offset[5].x * fi, baseY + 43.0, MakeStr(" %d", m_prizes[i].count),
			(eTITLEFONTALINE)2, 0.0f, 1.0f);
		CheckTip983(i, rc.x, rc.y, rc.w, rc.h, std::string(m_prizes[i].pItem->Name));
	}

	baseX = m_offset[3].x + GetRect().x;
	baseY = m_offset[3].y + GetRect().y;
	unsigned int n2 = m_rankPrizes.size();
	for (unsigned int i = 0; i < n2; i++)
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(m_rankPrizes.at(i).pItem->Icon);
		if (!pBmp)
			continue;
		float w = (float)pBmp->Width();
		float h = (float)pBmp->Height();
		float scale = 1.0f;
		if (w > 64.0f || h > 64.0f)
			scale = FitScale983(w, h);
		float fi = (float)i;
		WRect rc(baseX + fi * 64.0, baseY, scale * w, scale * h);
		pGdi->DrawTexture(pBmp, rc, 0xffffffff, 0);
		m_pTicketFont->Print(baseX + 50.0 + m_offset[5].x * fi, baseY + 43.0, MakeStr(" %d", m_rankPrizes[i].count),
			(eTITLEFONTALINE)2, 0.0f, 1.0f);
		CheckTip983(i + n, rc.x, rc.y, rc.w, rc.h, std::string(m_rankPrizes[i].pItem->Name));
	}
	pGdi->SetTextColor(0xff000000, 0xffffffff);
}

void FrGrandPrixOpenDlg::OnMouseMove(const WPoint& pt)
{
	m_mouse = pt;
}

// prêmios: os 5 do registro (+0xb4/+0xc8) e, do GrandPrixRankReward.iff, os 5 de cada linha com +0x08 == 1 (+0x0c/+0x20)
void FrGrandPrixOpenDlg::BuildPrizes983()
{
	for (int i = 0; i < 5; i++)
	{
		unsigned long tid = m_pInfo->prize[i];
		if (tid)
		{
			IFF_ITEM_COMMON* pItem = Doc()->m_itemManager.FindCommonItem(tid);
			if (pItem)
			{
				sGPPrize983 prize;
				prize.pItem = pItem;
				prize.count = (short)m_pInfo->prizeCount[i];
				m_prizes.push_back(prize);
			}
		}
	}
	std::vector<IFF983::sGrandPrixRankReward>* pRank = ITEMMANAGERGP983()->FindGrandPrixRankReward983(m_pInfo->ticket);
	if (!pRank)
		return;
	unsigned int n = pRank->size();
	for (unsigned int i = 0; i < n; i++)
	{
		const char* raw = (const char*)&pRank->at(i);
		if (*(const int*)(raw + 8) == 1)
		{
			for (int j = 0; j < 5; j++)
			{
				IFF_ITEM_COMMON* pItem = Doc()->m_itemManager.FindCommonItem(*(const unsigned long*)(raw + 0xc + j * 4));
				if (pItem)
				{
					sGPPrize983 prize;
					prize.pItem = pItem;
					prize.count = *(const short*)(raw + 0x20 + j * 4);
					m_rankPrizes.push_back(prize);
				}
			}
		}
	}
}

bool FrGrandPrixOpenDlg::OnInit()
{
	m_pTicketFont = std::tr1::shared_ptr<WTitleFontEx>(new CGrandPrixGameRoomTicketFont);
	m_pClassBmp[0] = g_pFresh->GetBitmap("gp_beginner");
	m_pClassBmp[1] = g_pFresh->GetBitmap("gp_class1");
	m_pClassBmp[2] = g_pFresh->GetBitmap("gp_class2");
	m_pClassBmp[3] = g_pFresh->GetBitmap("gp_class3");
	m_pMarkBmp[0] = g_pFresh->GetBitmap("gp_mark_beginner");
	m_pMarkBmp[1] = g_pFresh->GetBitmap("gp_mark_class1");
	m_pMarkBmp[2] = g_pFresh->GetBitmap("gp_mark_class2");
	m_pMarkBmp[3] = g_pFresh->GetBitmap("gp_mark_class3");

	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (!script)
		return false;
	script->RunFile("grandprix_form.lua");
	{
		lua_system::lua_table_helper tbl = script->GetTable("gpFormOffset");
		int count = lua_tinker::call<int, lua_tinker::table>(tbl.state, "get_array_size", tbl.table);
		sGPFormPoint983* pOffset = m_offset;
		for (int i = 1; i <= count; i++, pOffset++)
		{
			float x = AsTable983(LUAIDX983(tbl).GetTableByIndex(i).table).GetByIndex(1);
			float y = AsTable983(LUAIDX983(tbl).GetTableByIndex(i).table).GetByIndex(2);
			pOffset->x = x;
			pOffset->y = y;
		}
	}
	m_pTitle->SetCaption(m_pInfo->name);
	SetTexts983();
	BuildPrizes983();
	return FrForm::OnInit();
}
}
}

// ------------------------------------------------------------------------------------------------------------------------
// peças do relógio com código nesta unidade

// 0055cd50
void __fastcall GPFormat2_983(unsigned short value, char* out)
{
	char buf[4] = { 0 };
	sprintf_s(buf, 4, "%d", value);
	if (value < 10)
	{
		out[0] = '0';
		out[1] = buf[0];
	}
	else
	{
		out[0] = buf[0];
		out[1] = buf[1];
	}
}

// 0055cdb0: a volta das 24 h zera também o PM (B67: o `ok` vem antes do incremento — o 983 soma com o 1 do retorno)
bool CGrandPrixHour::Inc()
{
	bool ok = true;
	++m_hour;
	if (m_hour < 12)
		m_bPM = 0;
	else if (m_hour == 24)
	{
		m_bPM = 0;
		m_hour = 0;
	}
	else
		m_bPM = 1;
	unsigned short h = m_hour;
	if (h > 0 && h <= 12)
		m_value = h;
	else if (h > 12 && h < 24)
		m_value = h - 12;
	else if (h == 0)
		m_value = 12;
	else
	{
		m_value = 0x63;
		ok = false;
	}
	return ok;
}
