// JP 983: unidade 005c0260–005c5a20 — janelas RF dos encaixes de maestria: RFMasteryslotNoticeDlg, RFMasteryslotConfirmDlg,
// RFMasteryTileColumn, RFCharMasteryTileDlg e RFSoltAniWnd (os cortes rfsoltaniwnd/rfcharmasterytiledlg/_p2/semclasse_005c29ae
// da F3.7.1 são esta unidade; 005c5a20+ é a rfcharmasterystatdetailwnd). Sessão B139. Nomes meus (…983), menos as classes e os
// da RF; ver include/jp/refresh/rfcharmasterytiledlg.h.
#include "stdafx.h"
#include "vc10_layout.h"
#include "refresh/rfcharmasterytiledlg.h"
#include "refresh/rfmasterytooltipwnd.h"
#include "projectg.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "soundmanager.h"
#include "gamestringmanager.h"
#include "../../shared/itemmanager.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "icontentsdoc.h"
#include "wview.h"
#include "wresrcmng.h"
#include "achievement.h"
#include "questdocument.h"
#include <algorithm>
// A770: o 983 le a maestria do personagem em +0x1cd (sCharacterInfo pack 1, mesmo offset nos dois compiladores)
C_ASSERT(offsetof(sCharacterInfo, mastery) == 0x1cd);

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern WView* g_view; // 00f02458
extern const WVector g_vZero983_f041c4; // 00f041c4
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

#define DOC983_U8(off) (*(unsigned char*)((char*)Doc() + (off)))
#define DOCCHARID983() (Doc()->m_myInfo.userEquip.guidChar) // personagem em uso
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidChar) == 0xa2a); // A736
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.stat.Level) == 0x91d); // A742

// CFxBox::Finished (0096d7f0) só é visível na fxbox.cpp: visão com o nome no jp.tsv (a mesma da alchemympbreakdlg.cpp)
class CFxBoxJp983 : public CFxBox
{
public:
	bool Finished();
};

#include "levelnametable.h"

static CLevelNameTable983 s_levelNames; // 00e78194

RFMasteryslotNoticeDlg* RFMasteryslotNoticeDlg::ms_pInstance;
RFWnd* RFMasteryslotNoticeDlg::s_pParent983;
RFWnd* RFMasteryslotConfirmDlg::s_pParent983;
RFWnd* RFMasteryTileColumn::s_pParent983;
RFCharMasteryTileDlg* RFCharMasteryTileDlg::ms_pInstance;
RFWnd* RFCharMasteryTileDlg::s_pParent983;
RFWnd* RFSoltAniWnd::s_pParent983;

// 005c0260
RFMasteryslotNoticeDlg* RFMasteryslotNoticeDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005c0270
void RFMasteryslotNoticeDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005c0280
void RFMasteryslotNoticeDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005c0290
const sCharacterMastery983* RFMasteryTileColumn::GetMastery983()
{
	return m_pMastery983;
}

// 005c02a0
RFCharMasteryTileDlg* RFCharMasteryTileDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005c02b0
void RFCharMasteryTileDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005c02c0
void RFCharMasteryTileDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005c02e0: a animação acaba = fecha
void RFSoltAniWnd::OnUpdate(EventArgs& e)
{
	RFWnd::OnUpdate(e);
	if (((CFxBoxJp983*)&m_fxBox983)->Finished())
	{
		Close(1);
		return;
	}
	m_fxBox983.ProcessAttacher(e.m_time983 * 1000.0);
	m_fxBox983.Process(e.m_time983 * 1000.0);
}

// 005c0360
void RFMasteryslotNoticeDlg::OnOk983(EventArgs& e)
{
	g_audio->PlaySfx("ui_button_ok_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005c03a0
void RFMasteryslotConfirmDlg::OnOk983(EventArgs& e)
{
	g_audio->PlaySfx("ui_button_ok_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005c03e0
void RFMasteryslotConfirmDlg::OnCancel983(EventArgs& e)
{
	g_audio->PlaySfx("ui_button_cancel_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005c0420
void RFMasteryTileColumn::SetMastery983(const sCharacterMastery983* p)
{
	m_pMastery983 = p;
}

// 005c0430
void RFMasteryTileColumn::ShowSelected983(bool show)
{
	if (m_pSelectedImg983)
		m_pSelectedImg983->ShowWindow(show);
}

// 005c0450: encaixes de força do nível do jogador e o limite de força
void RFCharMasteryTileDlg::UpdateRank983()
{
	CStatusHelper helper;
	if (m_pRankSlot983)
		m_pRankSlot983->SetLabel(helper.GetLevelSlot983(0, -1), false);
	if (m_pRankPenalty983)
		m_pRankPenalty983->SetLabel(helper.GetPowerLimit983() + 1, false);
}

// 005c0610
void RFCharMasteryTileDlg::OnTipLeave983(EventArgs& e)
{
	if (m_pTip983)
		m_pTip983->ShowTip983(false);
}

// 005c0830
bool RFMasteryslotNoticeDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005c0840: nome do atributo (tipo − 1)
const char* CStatusHelper::GetStatName983(unsigned int index)
{
	if (index > 4)
		return "";
	const char* names[5];
	names[0] = K2L_Compatibility("\xc6\xc4\xbf\xf6"); // 파워
	names[1] = K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1"); // 컨트롤
	names[2] = K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5"); // 정확도
	names[3] = K2L_Compatibility("\xbd\xba\xc7\xc9"); // 스핀
	names[4] = K2L_Compatibility("\xc4\xbf\xba\xea"); // 커브
	return names[index];
}

// 005c08b0: efeito do atributo no centro da janela
void RFSoltAniWnd::Play983(int type)
{
	m_fxBox983.SetScreenCenter(GetRect().GetWidth() * 0.5 + GetScreenPos().x, GetRect().GetHeight() * 0.5 + GetScreenPos().y);
	CFxSequence* pSeq;
	switch (type)
	{
	case 0:
		pSeq = m_fxBox983.LoadSequence("mastery_eff_power.seq", true, NULL);
		break;
	case 1:
		pSeq = m_fxBox983.LoadSequence("mastery_eff_con.seq", true, NULL);
		break;
	case 3:
		pSeq = m_fxBox983.LoadSequence("mastery_eff_spin.seq", true, NULL);
		break;
	case 4:
		pSeq = m_fxBox983.LoadSequence("mastery_eff_curve.seq", true, NULL);
		break;
	default:
		return;
	}
	if (pSeq)
	{
		float x = GetRect().GetWidth() * 0.5 + GetScreenPos().x;
		pSeq->m_attachX = (float)(int)x;
		float y = GetRect().GetHeight() * 0.5 + GetScreenPos().y;
		pSeq->m_attachY = (float)(int)y;
		pSeq->m_pos = g_vZero983_f041c4;
		m_fxBox983.AddSequence(pSeq);
		g_audio->PlaySfx("mastery_stat_extend", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}
}

// 005c0a80: o efeito com câmera e campo de visão próprios; devolve os da tela
void RFSoltAniWnd::OnDraw()
{
	RFWndEx::OnDraw();
	if (!((CFxBoxJp983*)&m_fxBox983)->Finished())
	{
		bool bFog = g_view->SetFogEnable(false);
		g_view->SetCamera(m_camera983);
		float oldNear = g_view->clip_near;
		WView::PROJECTION_MODE mode = g_view->GetProjectionMode();
		float oldFar = g_view->clip_far;
		float oldFov = g_view->GetFOV();
		g_view->SetProjectionMode((WView::PROJECTION_MODE)0);
		g_view->SetFOV(1.5707964f);
		g_view->SetClip(0.5f, 50.0f, false);
		g_view->UpdateCamera();
		m_fxBox983.Display(g_view);
		g_view->Flush(1, 1);
		g_view->SetProjectionMode(mode);
		g_view->SetClip(oldNear, oldFar, false);
		g_view->SetFOV(oldFov);
		WRect rc;
		rc.x = 0.0f;
		rc.y = 0.0f;
		rc.w = g_view->SCREEN_XS;
		rc.h = g_view->SCREEN_YS;
		g_view->SetClippingArea(rc);
		g_view->SetScreenCenter(g_view->SCREEN_XS * 0.5, g_view->SCREEN_YS * 0.5);
		g_view->UpdateCamera();
		g_view->SetFogEnable(bFog);
	}
}

// 005c0c30
void RFSoltAniWnd::OnInitialUpdate()
{
	m_fxBox983.Init(g_resrcmng);
	m_fxBox983.SetCamera(&g_camera);
	m_fxBox983.m_frameTime = 33.333332f;
	m_camera983 = WMatrix::IDENTITY;
	m_camera983.pivot = WVector(0.0f, 0.0f, -30.0f);
}

// 005c0d20
RFMasteryslotNoticeDlg::RFMasteryslotNoticeDlg()
{
	ms_pInstance = this;
	m_pSlotImg983 = NULL;
	m_pMessage983 = NULL;
	m_pMastery983 = NULL;
}

// 005c0dc0
RFMasteryslotNoticeDlg::~RFMasteryslotNoticeDlg()
{
	ms_pInstance = NULL;
}

// 005c0e30: Enter/ESC = ok
void RFMasteryslotNoticeDlg::OnKey983(EventArgs& e)
{
	if (e.m_0c == 0xd || e.m_0c == 0x1b)
	{
		OnOk983(e);
		e.handled = true;
	}
}

// 005c0e60
RFMasteryslotConfirmDlg::RFMasteryslotConfirmDlg()
{
	m_pMessage983 = NULL;
	m_pSlotImg983 = NULL;
	m_pMastery983 = NULL;
}

// 005c0ef0
RFMasteryslotConfirmDlg::~RFMasteryslotConfirmDlg()
{
}

// 005c0f60
RFMasteryTileColumn::RFMasteryTileColumn()
	: m_pSlotImg983(NULL), m_pLockedImg983(NULL), m_pSelectedImg983(NULL), m_pMastery983(NULL)
{
	m_state983 = 3;
}

// 005c0fa0
RFMasteryTileColumn::~RFMasteryTileColumn()
{
}

// 005c16c0: abre o próximo encaixe (pacote 0x187)
void RFCharMasteryTileDlg::OnExpandOk983(EventArgs& e)
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	WSendPacket packet((enumClientPacket)0x187);
	packet.Encode4(m_pChar983->tid);
	packet.Encode4(m_pChar983->guid);
	packet.Send(TO_GAME);
}

// 005c17a0
void RFCharMasteryTileDlg::HideSelected983()
{
	if (m_pTable983[0])
	{
		for (int i = 0; i < m_pTable983[0]->GetItemCount983(); ++i)
		{
			RFMasteryTileColumn* col = (RFMasteryTileColumn*)m_pTable983[0]->GetItemData(i);
			if (col)
				col->ShowSelected983(false);
		}
	}
	if (m_pTable983[1])
	{
		for (int i = 0; i < m_pTable983[1]->GetItemCount983(); ++i)
		{
			RFMasteryTileColumn* col = (RFMasteryTileColumn*)m_pTable983[1]->GetItemData(i);
			if (col)
				col->ShowSelected983(false);
		}
	}
}

// 005c1840: registro do nível (preso entre o 1º e o último)
const sCharacterMastery983* RFCharMasteryTileDlg::GetMastery983(int level)
{
	if (m_list983.empty())
		return NULL;
	int i = level - 1;
	int last = (int)m_list983.size() - 1;
	if (i < 0)
		i = 0;
	else if (i > last)
		i = last;
	return m_list983.at(i);
}

// 005c1b70: Enter = abre o encaixe, ESC = fecha
void RFMasteryslotConfirmDlg::OnKey983(EventArgs& e)
{
	if (e.m_0c != 0xd)
	{
		if (e.m_0c == 0x1b)
		{
			Close(2);
			e.handled = true;
		}
	}
	else
	{
		if (RFCharMasteryTileDlg::ms_pInstance)
			RFCharMasteryTileDlg::ms_pInstance->OnExpandOk983(e);
		e.handled = true;
		Close(1);
	}
}

// 005c1c70: alguma conquista do jogador já cumpriu o contador dessa peça?
bool RFCharMasteryTileDlg::IsStuffDone983(unsigned long id)
{
	if (!m_pQuestDoc983)
		return false;
	for (_gateway::_questoneday::CQuestDocument::AchievementMap::iterator it = m_pQuestDoc983->m_achievements.begin();
		it != m_pQuestDoc983->m_achievements.end(); ++it)
	{
		for (unsigned i = 0; i < _achievement::CAchievement::MAX_COUNTER; ++i)
		{
			if (it->second.GetCounterId983(i) == id && it->second.GetCounterTime983(i) != 0)
				return true;
		}
	}
	return false;
}

// 005c1dc0
void RFMasteryslotNoticeDlg::SetSlot983(const sCharacterMastery983* p)
{
	m_pMastery983 = p;
	if (p && m_pSlotImg983)
		m_pSlotImg983->SetPicture(MultiByteToWide983(MakeStr("slot_image_%d_1.tga", p->statType - 1)));
}

// 005c1ec0
void RFMasteryslotConfirmDlg::SetSlot983(const sCharacterMastery983* p)
{
	m_pMastery983 = p;
	if (p && m_pSlotImg983)
		m_pSlotImg983->SetPicture(MultiByteToWide983(MakeStr("slot_image_%d_0.tga", p->statType - 1)));
}

// 005c1fc0: aberto (o personagem já tem o nível), o próximo (com o nível do jogador e a peça) ou fechado
void RFMasteryTileColumn::UpdateState983(int level)
{
	if (m_pMastery983)
	{
		m_state983 = STATE_LOCKED983;
		if (level >= m_pMastery983->level)
		{
			m_state983 = STATE_OPEN983;
			return;
		}
		if (m_pMastery983->level == level + 1)
		{
			bool stuffOk = true;
			bool levelOk = true;
			if (m_pMastery983->stuffId983)
				stuffOk = RFCharMasteryTileDlg::ms_pInstance->IsStuffDone983(m_pMastery983->stuffId983);
			if (m_pMastery983->reqLevel983)
				levelOk = Doc()->m_myInfo.stat.Level >= m_pMastery983->reqLevel983;
			if (stuffOk && levelOk)
				m_state983 = STATE_NEXT983;
		}
	}
}

// 005c2050
void RFMasteryTileColumn::UpdateImage983()
{
	if (m_pMastery983)
	{
		if (m_pSlotImg983)
			m_pSlotImg983->SetPicture(MultiByteToWide983(MakeStr("slot_image_%d_%d.tga", m_pMastery983->statType - 1,
				m_state983 == STATE_OPEN983)));
		if (m_pLockedImg983)
			m_pLockedImg983->ShowWindow(m_state983 == STATE_LOCKED983);
	}
}

// 005c2190
RFCharMasteryTileDlg::RFCharMasteryTileDlg()
{
	ms_pInstance = this;
	m_pSelected983 = NULL;
	m_pRankImg983 = NULL;
	m_pRankSlot983 = NULL;
	m_pRankPenalty983 = NULL;
	memset(m_pTable983, 0, sizeof(m_pTable983));
	memset(m_pTotal983, 0, sizeof(m_pTotal983));
	memset(m_pReqText983, 0, sizeof(m_pReqText983));
	memset(m_pReqCheck983, 0, sizeof(m_pReqCheck983));
	m_pExpandedImg983 = NULL;
	m_pExpandBtn983 = NULL;
	m_list983.clear();
	m_pChar983 = NULL;
	m_pTip983 = NULL;
	m_pQuestDoc983 = NULL;
}

// 005c2280
RFCharMasteryTileDlg::~RFCharMasteryTileDlg()
{
	ms_pInstance = NULL;
}

// 005c2320: quantos encaixes de cada atributo o personagem já abriu
void RFCharMasteryTileDlg::UpdateTotals983()
{
	// 005c0510 (cópia fora de linha): tipo → índice dos 4 rótulos
	auto toIndex = [](int type) -> int
	{
		int i = type - 1;
		switch (type)
		{
		case 1:
			i = 0;
			break;
		case 2:
			i = 1;
			break;
		case 4:
			i = 2;
			break;
		case 5:
			i = 3;
			break;
		}
		return i;
	};
	int counts[4] = { 0, 0, 0, 0 };
	// 005c0580 (cópia fora de linha do corpo); laço 005c0730
	std::for_each(m_list983.begin(), m_list983.end(), [&](const sCharacterMastery983* p)
	{
		int i = toIndex(p->statType);
		if (i == -1)
			return;
		if (p->level <= (int)m_pChar983->mastery)
			++counts[i];
	});
	for (int i = 0; i < 4; ++i)
	{
		if (m_pTotal983[i])
			m_pTotal983[i]->SetLabel(MultiByteToWide983(MakeStr("x%d", counts[i])));
	}
}

// 005c2460
void RFCharMasteryTileDlg::OnRankEnter983(EventArgs& e)
{
	if (m_pTip983)
	{
		std::string text = GetGameString(0x18843);
		m_pTip983->SetText983(text.c_str());
		if (m_pRankImg983)
		{
			RFVector2 pos = m_pRankImg983->GetScreenPos();
			RFSize size = m_pRankImg983->GetSize();
			m_pTip983->SetPos983(pos.x + size.width, pos.y);
		}
		m_pTip983->ShowTip983(true);
	}
}

// 005c2580: o próximo nível do jogador que dá encaixe de força
void RFCharMasteryTileDlg::OnPowerSlotEnter983(EventArgs& e)
{
	if (m_pTip983)
	{
		CStatusHelper helper;
		int cur = helper.GetLevelSlot983(0, -1);
		int next = -1;
		for (int level = Doc()->m_myInfo.stat.Level + 1; level < 0x47; ++level)
		{
			if (cur < helper.GetLevelSlot983(0, level))
			{
				next = level;
				break;
			}
		}
		const char* name;
		if (next == -1)
			name = "-";
		else
			name = K2L_Compatibility(s_levelNames.Get(next).name);
		std::string text = MakeStr(GetGameString(0x18844), name);
		m_pTip983->SetText983(text.c_str());
		if (m_pRankSlot983)
		{
			RFVector2 pos = m_pRankSlot983->GetScreenPos();
			RFSize size = m_pRankSlot983->GetSize();
			m_pTip983->SetPos983(pos.x + size.width, pos.y);
		}
		m_pTip983->ShowTip983(true);
	}
}

// 005c2730
void RFCharMasteryTileDlg::OnPenaltyEnter983(EventArgs& e)
{
	if (m_pTip983)
	{
		std::string text = GetGameString(0x18845);
		m_pTip983->SetText983(text.c_str());
		if (m_pRankPenalty983)
		{
			RFVector2 pos = m_pRankPenalty983->GetScreenPos();
			RFSize size = m_pRankPenalty983->GetSize();
			m_pTip983->SetPos983(pos.x + size.width, pos.y);
		}
		m_pTip983->ShowTip983(true);
	}
}

// 005c2970
void RFMasteryTileColumn::Update983(int level)
{
	UpdateState983(level);
	UpdateImage983();
}

// 005c2a40: o encaixe escolhido e o que falta para abri-lo (TP, nível do jogador, peça de conquista)
void RFCharMasteryTileDlg::Select983(const sCharacterMastery983* p)
{
	if (!p)
		return;
	if (m_pSelected983)
	{
		m_pSelected983->SetMastery983(p);
		m_pSelected983->Update983((int)m_pChar983->mastery);
	}
	std::string text;
	int cost = 0;
	if (m_pReqText983[REQ_NEEDTP983])
	{
		m_pReqText983[REQ_NEEDTP983]->ClearText();
		cost = p->tpCost983;
		text = MakeStr(GetGameString(0x1883e), cost);
		m_pReqText983[REQ_NEEDTP983]->SetText(MultiByteToWide983(text.c_str()));
	}
	int have = 0;
	if (m_pReqText983[REQ_CURTP983])
	{
		m_pReqText983[REQ_CURTP983]->ClearText();
		sItemInfo* pItem = Doc()->FindMyItemByTypeId983(0x1a0002a6);
		if (pItem)
			have = pItem->Common[0];
		text = MakeStr(GetGameString(0x1883f), have);
		m_pReqText983[REQ_CURTP983]->SetFontColor983(cost > have ? 0xffff0000 : 0xff000000); // vermelho se não dá
		m_pReqText983[REQ_CURTP983]->SetText(MultiByteToWide983(text.c_str()));
	}
	if (m_pReqText983[REQ_LEVEL983])
	{
		m_pReqText983[REQ_LEVEL983]->ClearText();
		if (p->reqLevel983)
		{
			text.append(MakeStr(GetGameString(0x18840), K2L_Compatibility(s_levelNames.Get((unsigned short)p->reqLevel983).name)));
			m_pReqText983[REQ_LEVEL983]->SetText(MultiByteToWide983(text.c_str()));
		}
	}
	if (m_pReqCheck983[REQCHK_LEVEL983])
	{
		m_pReqCheck983[REQCHK_LEVEL983]->ShowWindow(p->reqLevel983 != 0);
		m_pReqCheck983[REQCHK_LEVEL983]->SetCheck(p->reqLevel983 <= Doc()->m_myInfo.stat.Level);
	}
	unsigned long stuff = p->stuffId983;
	if (m_pReqText983[REQ_ACHIEVE983])
	{
		m_pReqText983[REQ_ACHIEVE983]->ClearText();
		IFF983::sQuestStuff* pStuff = ItemManager()->FindQuestStuff983(stuff);
		if (pStuff)
		{
			text = MakeStr(GetGameString(0x18841), (const char*)pStuff + 8); // nome (+8)
			m_pReqText983[REQ_ACHIEVE983]->SetText(MultiByteToWide983(text.c_str()));
		}
	}
	if (m_pReqCheck983[REQCHK_ACHIEVE983])
	{
		m_pReqCheck983[REQCHK_ACHIEVE983]->ShowWindow(stuff != 0);
		m_pReqCheck983[REQCHK_ACHIEVE983]->SetCheck(IsStuffDone983(stuff));
	}
	if (m_pExpandedImg983)
		m_pExpandedImg983->ShowWindow(p->level <= (int)m_pChar983->mastery);
	if (m_pExpandBtn983)
	{
		bool enable = (int)m_pChar983->mastery + 1 == p->level && p->tpCost983 <= have;
		if (m_pReqCheck983[REQCHK_LEVEL983]->IsWindowVisible(true) && !m_pReqCheck983[REQCHK_LEVEL983]->GetCheck())
			enable = false;
		if (m_pReqCheck983[REQCHK_ACHIEVE983]->IsWindowVisible(true) && !m_pReqCheck983[REQCHK_ACHIEVE983]->GetCheck())
			enable = false;
		m_pExpandBtn983->EnableWindow(enable);
	}
}

// 005c30c0
RFMasteryslotNoticeDlg* RFMasteryslotNoticeDlg::Create983(RFWnd* parent)
{
	RFMasteryslotNoticeDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryslotNoticeDlg>(L"mastery_slot_notice_dlg.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryslotNoticeDlg>(L"mastery_slot_notice_dlg.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFMasteryslotNoticeDlg>(L"mastery_slot_notice_dlg.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c3200
void RFMasteryslotNoticeDlg::SetMessage983(const char* text)
{
	if (m_pMessage983)
		m_pMessage983->SetText(MultiByteToWide983(text).c_str());
}

// 005c3340
void RFMasteryslotNoticeDlg::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFMasteryslotNoticeDlg::OnKey983, this));
	RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFMasteryslotNoticeDlg::OnOk983, this));
	m_pSlotImg983 = GetChild983<RFPicture>(L"slot_img");
	m_pMessage983 = GetChild983<RFTextArea>(L"message");
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005c3600
RFMasteryslotConfirmDlg* RFMasteryslotConfirmDlg::Create983(RFWnd* parent)
{
	RFMasteryslotConfirmDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryslotConfirmDlg>(L"mastery_slot_confirm_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryslotConfirmDlg>(L"mastery_slot_confirm_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFMasteryslotConfirmDlg>(L"mastery_slot_confirm_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c3740
void RFMasteryslotConfirmDlg::SetMessage983(const char* text)
{
	if (m_pMessage983)
		m_pMessage983->SetText(MultiByteToWide983(text).c_str());
}

// 005c3880
void RFMasteryslotConfirmDlg::OnInitialUpdate()
{
	m_pMessage983 = GetChild983<RFTextArea>(L"message");
	m_pSlotImg983 = GetChild983<RFPicture>(L"slot_img");
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFMasteryslotConfirmDlg::OnKey983, this));
	RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFMasteryslotConfirmDlg::OnOk983, this));
	RegisterTo(L"cancel_btn", RFButton::EventClicked, Subscriber(&RFMasteryslotConfirmDlg::OnCancel983, this));
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005c3c00
RFMasteryTileColumn* RFMasteryTileColumn::Create983(RFWnd* parent)
{
	RFMasteryTileColumn* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryTileColumn>(L"slot_mastery_column.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryTileColumn>(L"slot_mastery_column.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFMasteryTileColumn>(L"slot_mastery_column.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c3d40
void RFMasteryTileColumn::OnClick983(EventArgs& e)
{
	if (RFCharMasteryTileDlg::ms_pInstance)
	{
		RFCharMasteryTileDlg::ms_pInstance->Select983(m_pMastery983);
		RFCharMasteryTileDlg::ms_pInstance->HideSelected983();
	}
	if (m_pSelectedImg983)
		m_pSelectedImg983->ShowWindow(true);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005c3db0
RFCharMasteryTileDlg* RFCharMasteryTileDlg::Create983(RFWnd* parent)
{
	RFCharMasteryTileDlg* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryTileDlg>(L"slot_mastery_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryTileDlg>(L"slot_mastery_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryTileDlg>(L"slot_mastery_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c3ef0
void RFCharMasteryTileDlg::InitRank983()
{
	m_pRankImg983 = GetChild983<RFPicture>(L"rank_img");
	if (m_pRankImg983)
	{
		m_pRankImg983->SetPicture(MultiByteToWide983(MakeStr("level_%03d.tga", Doc()->m_myInfo.stat.Level + 1)));
		m_pRankImg983->SetSize(m_pRankImg983->GetTextureSize());
	}
	m_pRankSlot983 = GetChild983<RFLabel>(L"rank_power_slot_num_label");
	m_pRankPenalty983 = GetChild983<RFLabel>(L"rank_penalty_power_num_label");
}

// 005c4190
void RFCharMasteryTileDlg::InitTotals983()
{
	m_pTotal983[0] = GetChild983<RFLabel>(L"total_power_slot_num_label");
	m_pTotal983[1] = GetChild983<RFLabel>(L"total_control_slot_num_label");
	m_pTotal983[2] = GetChild983<RFLabel>(L"total_spin_slot_num_label");
	m_pTotal983[3] = GetChild983<RFLabel>(L"total_curve_slot_num_label");
}

// 005c4370: "abrir o encaixe?" com o custo em TP
void RFCharMasteryTileDlg::OnExpand983(EventArgs& e)
{
	RFMasteryslotConfirmDlg* dlg = RFMasteryslotConfirmDlg::Create983(this);
	if (dlg)
	{
		const sCharacterMastery983* p = m_pSelected983->m_pMastery983;
		std::string text = MakeStr(GetGameString(0x18842), p->tpCost983);
		dlg->SetSlot983(p);
		dlg->SetMessage983(text.c_str());
		dlg->RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFCharMasteryTileDlg::OnExpandOk983, this));
		dlg->DoModal();
	}
}

// 005c45c0
void RFMasteryTileColumn::OnInitialUpdate()
{
	RegisterTo(L"slot_btn", RFButton::EventClicked, Subscriber(&RFMasteryTileColumn::OnClick983, this));
	m_pSlotImg983 = GetChild983<RFPicture>(L"slot_img");
	m_pLockedImg983 = GetChild983<RFPicture>(L"locked_img");
	m_pSelectedImg983 = GetChild983<RFPicture>(L"selected_img");
	if (m_pLockedImg983)
		m_pLockedImg983->ShowWindow(false);
	if (m_pSelectedImg983)
		m_pSelectedImg983->ShowWindow(false);
}

// 005c4810: estado dos encaixes; o próximo fica marcado
void RFCharMasteryTileDlg::UpdateColumns983()
{
	if (!m_list983.empty())
	{
		HideSelected983();
		int i = 0;
		// 005c29c0 (cópia fora de linha do corpo); laço 005c3020
		std::for_each(m_list983.begin(), m_list983.end(), [this, &i](const sCharacterMastery983* p)
		{
			RFList* list = m_pTable983[0];
			if (i > 4)
				list = m_pTable983[1];
			RFMasteryTileColumn* col = (RFMasteryTileColumn*)list->GetItemData(i % TILE_PER_LIST983);
			if (col)
			{
				col->Update983((int)m_pChar983->mastery);
				if ((int)m_pChar983->mastery + 1 == p->level)
					col->ShowSelected983(true);
			}
			++i;
		});
	}
}

// 005c4860: os registros de maestria do personagem em uso
void RFCharMasteryTileDlg::LoadMastery983()
{
	CStatusHelper helper;
	helper.GetMasteryList983(m_list983, 0, -1);
	m_pChar983 = &Doc()->m_charMap[DOCCHARID983()];
}

// 005c48f0
void RFCharMasteryTileDlg::InitExpand983()
{
	RegisterTo(L"button_slot_expand", RFButton::EventClicked, Subscriber(&RFCharMasteryTileDlg::OnExpand983, this));
	RFWndEx* holder = GetChild983<RFWndEx>(L"selected_slot_wnd");
	RFWndEx* pop = GetChild983<RFWndEx>(L"slot_expand_pop");
	if (holder && pop)
	{
		m_pSelected983 = RFMasteryTileColumn::Create983(pop);
		if (m_pSelected983)
		{
			m_pSelected983->SetPos(holder->GetPos().x, holder->GetPos().y);
			m_pSelected983->ShowSelected983(false);
			m_pSelected983->ShowWindow983(true);
		}
		DeleteChild(holder);
	}
	m_pReqText983[REQ_NEEDTP983] = GetChild983<RFTextArea>(L"need_tp_label");
	m_pReqText983[REQ_CURTP983] = GetChild983<RFTextArea>(L"current_tp_label");
	m_pReqText983[REQ_LEVEL983] = GetChild983<RFTextArea>(L"level_limit_label");
	m_pReqText983[REQ_ACHIEVE983] = GetChild983<RFTextArea>(L"achivement_clear_label");
	m_pReqCheck983[REQCHK_LEVEL983] = GetChild983<RFCheck>(L"level_limit_check");
	m_pReqCheck983[REQCHK_ACHIEVE983] = GetChild983<RFCheck>(L"achivement_clear_check");
	m_pExpandedImg983 = GetChild983<RFPicture>(L"slot_expanded_img");
	m_pExpandBtn983 = GetChild983<RFButton>(L"button_slot_expand");
}

// 005c4e70
void RFCharMasteryTileDlg::Refresh983()
{
	UpdateRank983();
	UpdateColumns983();
	UpdateTotals983();
	Select983(GetMastery983((int)m_pChar983->mastery + 1));
}

// 005c4eb0: as duas listas de 5 encaixes
void RFCharMasteryTileDlg::InitTables983()
{
	if (!m_list983.empty())
	{
		m_pTable983[0] = GetChild983<RFList>(L"mastery_table1");
		m_pTable983[1] = GetChild983<RFList>(L"mastery_table2");
		if (m_pTable983[0] && m_pTable983[1])
		{
			int i = 0;
			// 005c4130 (cópia fora de linha do corpo); laço 005c4540
			std::for_each(m_list983.begin(), m_list983.end(), [this, &i](const sCharacterMastery983* p)
			{
				RFList* list = m_pTable983[0];
				if (i > 4)
					list = m_pTable983[1];
				// C295: pai = a lista (983 005c414e/005c4571: ecx = tabela 0x194/0x198), não o diálogo (apagava duas vezes)
				RFMasteryTileColumn* col = RFMasteryTileColumn::Create983(list);
				if (col)
				{
					col->SetMastery983(p);
					col->ShowWindow983(true);
					list->InsertWindowItem(col);
				}
				++i;
			});
		}
	}
}

// 005c5000: a animação do encaixe acabou: aviso com o atributo e redesenho
void RFCharMasteryTileDlg::OnSlotAniEnd983()
{
	if (!RFMasteryslotNoticeDlg::ms_pInstance)
	{
		RFMasteryslotNoticeDlg* dlg = RFMasteryslotNoticeDlg::Create983(this);
		if (dlg)
		{
			CStatusHelper helper;
			const sCharacterMastery983* p = m_pSelected983->m_pMastery983;
			std::string text = MakeStr(GetGameString(0x18846), helper.GetStatName983(p->statType - 1));
			dlg->SetSlot983(p);
			dlg->SetMessage983(text.c_str());
			dlg->DoModal();
		}
	}
	Refresh983();
}

// 005c5130
RFSoltAniWnd::RFSoltAniWnd()
{
}

// 005c51c0
RFSoltAniWnd::~RFSoltAniWnd()
{
	m_fxBox983.ClearAll(false);
	if (RFCharMasteryTileDlg::ms_pInstance)
		RFCharMasteryTileDlg::ms_pInstance->OnSlotAniEnd983();
}

// 005c5270
void RFCharMasteryTileDlg::OnInitialUpdate()
{
	RegisterTo(L"bg_rank", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryTileDlg::OnRankEnter983, this));
	RegisterTo(L"bg_rank", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryTileDlg::OnTipLeave983, this));
	RegisterTo(L"bg_power_slot", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryTileDlg::OnPowerSlotEnter983, this));
	RegisterTo(L"bg_power_slot", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryTileDlg::OnTipLeave983, this));
	RegisterTo(L"bg_penalty_power", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryTileDlg::OnPenaltyEnter983, this));
	RegisterTo(L"bg_penalty_power", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryTileDlg::OnTipLeave983, this));
	LoadMastery983();
	InitRank983();
	InitTables983();
	InitTotals983();
	InitExpand983();
	m_pTip983 = RFMasteryToolTipWnd::Create983(GetSuper(false));
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xac, m_pQuestDoc983);
}

// 005c5870
RFSoltAniWnd* RFSoltAniWnd::Create983(RFWnd* parent)
{
	RFSoltAniWnd* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFSoltAniWnd>(L"kam_mastery_slotani.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFSoltAniWnd>(L"kam_mastery_slotani.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFSoltAniWnd>(L"kam_mastery_slotani.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c59e0: abre a animação do encaixe escolhido (casos 0x26f/0x270 do CTask::OnPacketCommon)
void RFCharMasteryTileDlg::PlaySlotAni983()
{
	RFSoltAniWnd* wnd = RFSoltAniWnd::Create983(NULL);
	if (wnd)
	{
		int type = m_pSelected983->m_pMastery983->statType;
		wnd->DoModal();
		wnd->Play983(type - 1);
	}
}
