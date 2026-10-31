// JP 983: unidade 005c5a20–005ccb80 — janelas RF dos atributos do personagem na maestria: RFMasteryStatNoticeDlg,
// RFMasteryStatUpDnDlg, RFCharMasteryStatColumn, RFCharMasteryStatDetailWnd e RFCharMasteryStatusDlg, mais os métodos da
// MasteryGaugeBar (o construtor dela, 005c6360–005c6400, é a masterygaugebar.cpp, no meio desta). Começa nos acessores de
// instância 005c5a20 (logo depois da rfcharmasterytiledlg); 005ccbc0+ é a rfuseitemdlg. Sessão B141. Nomes meus (…983),
// menos as classes e os da RF; ver include/jp/refresh/rfcharmasterystatdetailwnd.h.
#include "stdafx.h"
#include "refresh/rfcharmasterystatdetailwnd.h"
#include "refresh/rfmasterytooltipwnd.h"
#include "refresh/rfuseitemdlg.h"
#include "wlocalize.h"
#include "projectg.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "soundmanager.h"
#include "gamestringmanager.h"
#include "../../shared/itemmanager.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "cardsystem.h"
#include "statushelper.h"
#include <algorithm>

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

// Métodos do CItemManager do 983 sem declaração pública na A (o mesmo empréstimo da statushelper.cpp)
class CItemManagerStatusJp
{
public:
	// 009e9cc0: min(Attr do taco + subida, teto 009e9c80 com os slots ganhos)
	unsigned char GetClubSetStat983(unsigned long type, IFF_STRUCT::sClubSet* pClub, const short* attrUp,
		const short* slotUp);
};

static inline CItemManagerStatusJp* ItemMgr983()
{
	return (CItemManagerStatusJp*)ItemManager();
}

// sItemInfo +0xaa: slots ganhos na oficina (5 short; o sItemInfo da A ainda não tem o campo, pedido B122.1)
static inline const short* ClubSlotUp983(const sItemInfo& info)
{
	return (const short*)((const char*)&info + 0xaa);
}


RFMasteryStatNoticeDlg* RFMasteryStatNoticeDlg::ms_pInstance;
RFWnd* RFMasteryStatNoticeDlg::s_pParent983;
RFMasteryStatUpDnDlg* RFMasteryStatUpDnDlg::ms_pInstance;
RFWnd* RFMasteryStatUpDnDlg::s_pParent983;
RFWnd* RFCharMasteryStatDetailWnd::s_pParent983;
RFWnd* RFCharMasteryStatColumn::s_pParent983;
RFCharMasteryStatusDlg* RFCharMasteryStatusDlg::ms_pInstance;
RFWnd* RFCharMasteryStatusDlg::s_pParent983;

// 005c5a20
void RFMasteryStatNoticeDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005c5a30
void RFMasteryStatNoticeDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005c5a40
RFMasteryStatUpDnDlg* RFMasteryStatUpDnDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005c5a50
RFMasteryStatUpDnDlg* RFMasteryStatUpDnDlg::GetInstance2_983()
{
	return ms_pInstance;
}

// 005c5a60
void RFMasteryStatUpDnDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005c5a70
void RFMasteryStatUpDnDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005c5a80
bool RFMasteryStatUpDnDlg::IsSent983()
{
	return m_sent983;
}

// 005c5a90
void RFMasteryStatUpDnDlg::SetSent983(bool sent)
{
	m_sent983 = sent;
}

// 005c5ab0: MasteryGaugeBar::SetValue983 (embutida; corpo no masterygaugebar.h)

// 005c5b20
void MasteryGaugeBar::Show983(bool show)
{
	m_pWnd983[BAR_STAT983]->ShowWindow(show);
	m_pWnd983[BAR_SLOT983]->ShowWindow(show);
	m_pWnd983[BAR_BG983]->ShowWindow(show);
}

// 005c5b50
void RFMasteryStatUpDnDlg::SetMode983(int mode)
{
	m_mode983 = mode;
}

// 005c5b90
void RFMasteryStatNoticeDlg::OnOk983(EventArgs& e)
{
	g_audio->PlaySfx("ui_button_ok_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(1);
}

// 005c5bd0
void RFMasteryStatUpDnDlg::OnCancel983(EventArgs& e)
{
	g_audio->PlaySfx("ui_button_cancel_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005c5c10: atributo e encaixes livres antes/depois
void RFMasteryStatUpDnDlg::UpdateSlots983()
{
	if (m_mode983 != MODE_NONE983)
	{
		CStatusHelper helper;
		int total = helper.GetStatus983(m_type983) + (m_mode983 != MODE_DOWN983) * 2 - 1;
		int before = helper.GetMyFreeSlot983(m_type983);
		int after;
		if (m_mode983 == MODE_UP983)
			after = before - 1;
		else
			after = helper.GetMyOverSlot983(m_type983) ? 0 : before + 1;
		if (m_pNum983[NUM_TOTAL983])
			m_pNum983[NUM_TOTAL983]->SetLabel(total, false);
		if (m_pNum983[NUM_BEFORE983])
			m_pNum983[NUM_BEFORE983]->SetLabel(before, false);
		if (m_pNum983[NUM_AFTER983])
			m_pNum983[NUM_AFTER983]->SetLabel(after, false);
	}
}

// 005c5d60: medidor do atributo (o máximo é 60 para força e 30 para os outros)
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B539 (provisorio)
void RFCharMasteryStatColumn::UpdateGauge983()
{
	if (m_type983 != 5)
	{
		CStatusHelper helper;
		if (m_pGauge983)
		{
			int status = helper.GetStatus983(m_type983);
			int slot = helper.GetMyFreeSlot983(m_type983) + status;
			m_pGauge983->SetMax983(m_type983 != 0 ? 30 : 60);
			m_pGauge983->SetValue983(status, MasteryGaugeBar::BAR_STAT983);
			m_pGauge983->SetValue983(slot, MasteryGaugeBar::BAR_SLOT983);
			jp983_rastro("B539 col t=%d char=%d equip=%d pclraw=%d pcl=%d slotparts=%d slotcard=%d slotmast=%d slotlv=%d pen=%d status=%d free=%d maxpic=%p", (int)m_type983,
				helper.GetMyCharStat983(m_type983), helper.GetMyEquipStat983(m_type983), helper.GetMyPCL983(m_type983, true, NULL), helper.GetMyPCL983(m_type983, false, NULL),
				helper.GetPartsSlot983(m_type983, NULL), helper.GetMyCardSlot983(m_type983), helper.GetMasterySlotOf983(m_type983, 0, -1), helper.GetLevelSlot983(m_type983, -1),
				helper.GetMyPenalty983(m_type983), status, slot - status, (void*)m_pMax983);
		}
	}
}

// 005c6400
RFMasteryStatNoticeDlg::RFMasteryStatNoticeDlg()
{
	ms_pInstance = this;
	memset(m_pStatImg983, 0, sizeof(m_pStatImg983));
	m_pArrow983 = NULL;
	m_pStat983 = NULL;
	m_pMessage983 = NULL;
}

// 005c64a0
RFMasteryStatNoticeDlg::~RFMasteryStatNoticeDlg()
{
	ms_pInstance = NULL;
}

// 005c6510: Enter/ESC = ok
void RFMasteryStatNoticeDlg::OnKey983(EventArgs& e)
{
	if (e.m_0c == 0xd || e.m_0c == 0x1b)
	{
		OnOk983(e);
		e.handled = true;
	}
}

// 005c6540
RFMasteryStatUpDnDlg::RFMasteryStatUpDnDlg()
{
	memset(m_pImg983, 0, sizeof(m_pImg983));
	memset(m_pNum983, 0, sizeof(m_pNum983));
	m_pMessage983 = NULL;
	m_pWarning983 = NULL;
	m_mode983 = MODE_NONE983;
	m_type983 = 5;
	m_sent983 = false;
	ms_pInstance = this;
}

// 005c6610
RFMasteryStatUpDnDlg::~RFMasteryStatUpDnDlg()
{
	ms_pInstance = NULL;
}

// 005c6680
RFCharMasteryStatColumn::RFCharMasteryStatColumn()
	: m_pButton983(NULL), m_pMax983(NULL), m_pGauge983(NULL), m_pDetail983(NULL)
{
	m_type983 = 5;
}

// 005c66c0
RFCharMasteryStatColumn::~RFCharMasteryStatColumn()
{
	if (m_pGauge983)
	{
		delete m_pGauge983;
		m_pGauge983 = NULL;
	}
}

// 005c6740
RFCharMasteryStatusDlg::RFCharMasteryStatusDlg()
{
	ms_pInstance = this;
	m_pList983 = NULL;
}

// 005c6770
RFCharMasteryStatusDlg::~RFCharMasteryStatusDlg()
{
	ms_pInstance = NULL;
}

// 005c7610
void MasteryGaugeBar::SetPicture983(std::wstring name, unsigned int index)
{
	if (index <= BAR_BG983)
	{
		if (m_pWnd983[index])
			m_pWnd983[index]->SetPicture(name);
	}
}

// 005c7a70
RFCharMasteryStatDetailWnd::RFCharMasteryStatDetailWnd()
{
	m_pUp983 = NULL;
	m_pDown983 = NULL;
	m_type983 = 5;
	m_pLabel983[LABEL_CHAR983] = NULL;
	m_pLabel983[LABEL_PARTS983] = NULL;
	m_pLabel983[LABEL_SLOT983] = NULL;
	m_pLabel983[LABEL_PENALTY983] = NULL;
	m_pLabel983[LABEL_TOTAL983] = NULL;
	m_pLabel983[LABEL_EXTRA983] = NULL;
	m_pTip983 = NULL;
	m_open983 = 0;
	m_pCaddie983 = NULL;
	m_pMascot983 = NULL;
	m_pClub983 = NULL;
	m_pClubInfo983 = NULL;
}

// 005c7b10
RFCharMasteryStatDetailWnd::~RFCharMasteryStatDetailWnd()
{
}

// 005c7be0
void RFCharMasteryStatColumn::UpdateButton983()
{
	if (m_type983 != 5 && m_pButton983)
	{
		std::string name("");
		name = MakeStr("button_mastery_stat_%d_n.tga", m_type983);
		m_pButton983->ChangeNormalButtonImage(MultiByteToWide983(name));
		m_pButton983->ChangeDisableButtonImage(MultiByteToWide983(name));
		m_pButton983->ChangeOverButtonImage(MultiByteToWide983(name));
		m_pButton983->ChangeDownButtonImage(MultiByteToWide983(name));
	}
}

// 005c7d20
void RFCharMasteryStatColumn::UpdateBar983()
{
	if (m_type983 != 5)
	{
		if (m_pGauge983)
			m_pGauge983->SetPicture983(MultiByteToWide983(MakeStr("mastery_stat_bar_%d.tga", m_type983)),
				MasteryGaugeBar::BAR_STAT983);
		if (m_pMax983)
			m_pMax983->SetPicture(MultiByteToWide983(MakeStr("max_%d.tga", m_type983 != 0 ? 30 : 60))); // B542: o original carrega max_60/max_30.tga (uitrace orig-b538-1), nao max_<tipo>
	}
}

// 005c8400
RFMasteryStatNoticeDlg* RFMasteryStatNoticeDlg::Create983(RFWnd* parent)
{
	RFMasteryStatNoticeDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryStatNoticeDlg>(L"mastery_stat_notice_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryStatNoticeDlg>(L"mastery_stat_notice_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFMasteryStatNoticeDlg>(L"mastery_stat_notice_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c8540
void RFMasteryStatNoticeDlg::SetStat983(unsigned long type, bool up)
{
	CStatusHelper helper;
	int status = helper.GetStatus983(type);
	if (m_pStatImg983[IMG_STAT983])
		m_pStatImg983[IMG_STAT983]->SetPicture(MultiByteToWide983(MakeStr("button_mastery_stat_%d_n.tga", type)));
	if (m_pStatImg983[IMG_CAP983])
		m_pStatImg983[IMG_CAP983]->SetPicture(MultiByteToWide983(MakeStr("button_mastery_stat_%d_cap.tga", type)));
	if (m_pArrow983)
		m_pArrow983->SetPicture(up ? L"mastery_stat_arrow_up.tga" : L"mastery_stat_arrow_down.tga");
	if (m_pStat983)
		m_pStat983->SetLabel(status, false);
	if (m_pMessage983)
	{
		std::string text("");
		if (up)
			text = MakeStr(GetGameString(0x18831), helper.GetStatName983(type));
		else
			text = MakeStr(GetGameString(0x18832), helper.GetStatName983(type));
		m_pMessage983->SetText(MultiByteToWide983(text));
	}
}

// 005c8870
void RFMasteryStatNoticeDlg::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFMasteryStatNoticeDlg::OnKey983, this));
	RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFMasteryStatNoticeDlg::OnOk983, this));
	m_pStatImg983[IMG_STAT983] = GetChild983<RFPicture>(L"stat_img_01");
	m_pStatImg983[IMG_CAP983] = GetChild983<RFPicture>(L"stat_img_02");
	m_pArrow983 = GetChild983<RFPicture>(L"arrow_img");
	m_pStat983 = GetChild983<RFLabel>(L"stat_label");
	m_pMessage983 = GetChild983<RFTextArea>(L"message");
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005c8c50
RFMasteryStatUpDnDlg* RFMasteryStatUpDnDlg::Create983(RFWnd* parent)
{
	RFMasteryStatUpDnDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryStatUpDnDlg>(L"kam_status_updown_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryStatUpDnDlg>(L"kam_status_updown_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFMasteryStatUpDnDlg>(L"kam_status_updown_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c8d90
void RFMasteryStatUpDnDlg::UpdateImages983()
{
	if (m_mode983 != MODE_NONE983)
	{
		if (m_pImg983[IMG_ARROW983])
			m_pImg983[IMG_ARROW983]->SetPicture(m_mode983 == MODE_DOWN983 ? L"mastery_stat_arrow_down.tga" : L"mastery_stat_arrow_up.tga");
		if (m_pImg983[IMG_TITLE983])
			m_pImg983[IMG_TITLE983]->SetPicture(m_mode983 == MODE_DOWN983 ? L"title_status_down.tga" : L"title_status_up.tga");
		if (m_pImg983[IMG_STATBG1_983])
			m_pImg983[IMG_STATBG1_983]->SetPicture(MultiByteToWide983(MakeStr("button_mastery_stat_%d_n.tga", m_type983)));
		if (m_pImg983[IMG_STATBG2_983])
			m_pImg983[IMG_STATBG2_983]->SetPicture(MultiByteToWide983(MakeStr("button_mastery_stat_%d_cap.tga", m_type983)));
	}
}

// 005c9020
RFCharMasteryStatDetailWnd* RFCharMasteryStatDetailWnd::Create983(RFWnd* parent)
{
	RFCharMasteryStatDetailWnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryStatDetailWnd>(L"kam_status_detail_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryStatDetailWnd>(L"kam_status_detail_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryStatDetailWnd>(L"kam_status_detail_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c9160
void RFCharMasteryStatDetailWnd::SetCap983(unsigned long type)
{
	RFPicture* cap = GetChild983<RFPicture>(L"cap_img");
	if (cap)
		cap->SetPicture(MultiByteToWide983(MakeStr("button_mastery_stat_%d_cap.tga", type)));
}

// 005c92a0: abre/fecha
void RFCharMasteryStatDetailWnd::OnCap983(EventArgs& e)
{
	if (m_open983 == 0)
	{
		m_open983 = 1;
		PlayAnimation(L"_push_status_detail_");
		g_audio->PlaySfx("mastery_stat_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}
	else
	{
		m_open983 = 0;
		PlayAnimation(L"_close_status_detail_");
		g_audio->PlaySfx("mastery_stat_closed", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}
}

// 005c9400: dica ao lado do número (LABEL_*)
void RFCharMasteryStatDetailWnd::ShowTip983(bool show, const char* text, int label, unsigned long color)
{
	if (m_pTip983)
	{
		if (!show)
		{
			m_pTip983->ShowTip983(false);
		}
		else
		{
			std::wstring wide = MultiByteToWide983(text);
			m_pTip983->SetTextStyle983(color);
			m_pTip983->SetTextW983(wide);
			float width = m_pLabel983[label]->GetSize().width;
			m_pTip983->SetPos983(m_pLabel983[label]->GetScreenPos().x + width, m_pLabel983[label]->GetScreenPos().y);
			m_pTip983->ShowTip983(show);
		}
	}
}

// 005c9580
RFCharMasteryStatColumn* RFCharMasteryStatColumn::Create983(RFWnd* parent)
{
	RFCharMasteryStatColumn* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryStatColumn>(L"kam_status_column.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryStatColumn>(L"kam_status_column.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryStatColumn>(L"kam_status_column.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c96c0
void RFCharMasteryStatColumn::UpdateCap983()
{
	if (m_pDetail983)
		m_pDetail983->SetCap983(m_type983);
}

// 005c96e0
void RFCharMasteryStatColumn::OnEnter983(EventArgs& e)
{
	if (m_pDetail983 && m_pDetail983->m_open983 == 0)
		m_pDetail983->PlayAnimation(L"_over_status_detail_");
}

// 005c9790
void RFCharMasteryStatColumn::OnLeave983(EventArgs& e)
{
	if (m_pDetail983 && m_pDetail983->m_open983 == 0)
		m_pDetail983->PlayAnimation(L"_out_status_detail_");
}

// 005c9840: abre/fecha o detalhe
void RFCharMasteryStatColumn::OnClick983(EventArgs& e)
{
	RFCharMasteryStatDetailWnd* detail = m_pDetail983;
	if (detail)
	{
		if (detail->m_open983 == 1)
		{
			detail->m_open983 = 0;
			m_pDetail983->PlayAnimation(L"_close_status_detail_");
			g_audio->PlaySfx("mastery_stat_closed", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		}
		else
		{
			detail->m_open983 = 1;
			m_pDetail983->PlayAnimation(L"_push_status_detail_");
			g_audio->PlaySfx("mastery_stat_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		}
	}
}

// 005c99c0
RFCharMasteryStatusDlg* RFCharMasteryStatusDlg::Create983(RFWnd* parent)
{
	RFCharMasteryStatusDlg* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryStatusDlg>(L"kam_mastery_status.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryStatusDlg>(L"kam_mastery_status.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryStatusDlg>(L"kam_mastery_status.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005c9b00: resposta do servidor (CTask::OnPacketCommon 0x26f/0x270)
void RFCharMasteryStatusDlg::OnResult983(unsigned long type, bool up)
{
	if (RFMasteryStatUpDnDlg::ms_pInstance)
		RFMasteryStatUpDnDlg::ms_pInstance->Close(2);
	RFMasteryStatNoticeDlg* dlg = RFMasteryStatNoticeDlg::Create983(this);
	if (dlg)
	{
		dlg->SetStat983(type, up);
		dlg->DoModal();
	}
}

// 005c9bc0: subir custa pang (Enchant.iff); manda 0x188 (subir) ou 0x189 (descer)
void RFMasteryStatUpDnDlg::OnOk983(EventArgs& e)
{
	if (m_sent983)
		return;
	g_audio->PlaySfx("ui_button_ok_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	if (m_mode983 == MODE_NONE983)
		return;

	CStatusHelper helper;
	IFF_STRUCT::sEnchant* pEnchant = ItemManager()->FindEnchant(
		helper.GetMyPCL983(m_type983, true, NULL) | ((m_type983 | 0x340) << 20));
	unsigned long cost = m_mode983 == MODE_DOWN983 ? 0 : pEnchant->ReqdPang;
	if (m_mode983 == MODE_UP983 && Doc()->m_myInfo.stat.i64Pang < cost)
	{
		RFNoticeDlg* dlg = RFNoticeDlg::Create983(this);
		if (dlg)
		{
			// 업그레이드를 위한 팡이 부족합니다. (pang insuficiente para subir)
			std::string text(K2L_Compatibility("\xbe\xf7\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5\xb8\xa6 \xc0\xa7\xc7\xd1 \xc6\xce\xc0\xcc "
				"\xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."));
			dlg->SetMessage983(text.c_str());
			dlg->DoModal();
		}
	}
	else
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		m_sent983 = true;
		sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		WSendPacket packet((enumClientPacket)((m_mode983 == MODE_DOWN983) + 0x188));
		packet.Encode4(m_type983);
		packet.EncodeBuffer(pInfo, 0x201);
		packet.Send(TO_GAME);
	}
}

// 005c9df0
void RFMasteryStatUpDnDlg::UpdateText983()
{
	if (m_mode983 != MODE_NONE983)
	{
		CStatusHelper helper;
		if (m_pMessage983)
		{
			m_pMessage983->ClearText();
			std::string text;
			if (m_mode983 == MODE_DOWN983)
			{
				text = MakeStr(GetGameString(0x18834), helper.GetStatName983(m_type983));
			}
			else
			{
				sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
				if (pInfo)
				{
					IFF_STRUCT::sEnchant* pEnchant = ItemManager()->FindEnchant(
						helper.GetMyPCL983(m_type983, true, NULL) | ((m_type983 | 0x340) << 20));
					if (pEnchant)
						text = MakeStr(GetGameString(0x18833), helper.GetStatName983(m_type983), pEnchant->ReqdPang);
				}
			}
			m_pMessage983->SetText(MultiByteToWide983(text));
		}
		if (m_pWarning983)
		{
			m_pWarning983->ClearText();
			if (m_type983 == 0 && m_mode983 == MODE_UP983)
				m_pWarning983->SetText(MultiByteToWide983(MakeStr(GetGameString(0x18835), helper.GetPowerLimit983() + 1)));
		}
	}
}

// 005ca030 (egg1)
void RFCharMasteryStatDetailWnd::OnBaseEnter983(EventArgs& e)
{
	std::string text(GetGameString(0x18836));
	ShowTip983(true, text.c_str(), LABEL_CHAR983, 0xff000000);
}

// 005ca0f0 (egg2): peças, auxiliares, caddie, mascote, taco e cartas que dão ponto nesse atributo
void RFCharMasteryStatDetailWnd::OnEquipEnter983(EventArgs& e)
{
	std::string text;
	int count = 0;
	// 005c81d0 (for_each fora de linha; corpo 005c80d0)
	std::for_each(m_parts983.begin(), m_parts983.end(), [this, &count, &text](IFF_STRUCT::sPart* p)
	{
		if (0 < p->Attr[m_type983])
		{
			if (count)
				text += '\n';
			text += MakeStr("%s : %d", p->c.Name, (int)p->Attr[m_type983]);
			count++;
		}
	});
	// 005c8280 (for_each fora de linha; corpo 005c8150)
	std::for_each(m_auxParts983.begin(), m_auxParts983.end(), [this, &count, &text](IFF_STRUCT::sAuxPart* p)
	{
		if (p->Attr[m_type983] > 0)
		{
			if (count)
				text += '\n';
			text += MakeStr("%s : %d", p->c.Name, (int)p->Attr[m_type983]);
			count++;
		}
	});
	if (m_pCaddie983 && 0 < m_pCaddie983->Attr[m_type983])
	{
		if (count)
			text += '\n';
		text += MakeStr("%s : %d", m_pCaddie983->c.Name, (int)m_pCaddie983->Attr[m_type983]);
		count++;
	}
	if (m_pMascot983 && m_pMascot983->Attr[m_type983] > 0)
	{
		if (count)
			text += '\n';
		text += MakeStr("%s : %d", m_pMascot983->c.Name, (int)m_pMascot983->Attr[m_type983]);
		count++;
	}
	if (m_pClub983 && 0 < m_pClub983->Attr[m_type983])
	{
		if (count)
			text += '\n';
		text += MakeStr("%s : %d", m_pClub983->c.Name,
			ItemMgr983()->GetClubSetStat983(m_type983, m_pClub983, m_pClubInfo983->Common, ClubSlotUp983(*m_pClubInfo983)));
		count++;
	}
	int npc = CCardManager::Instance()->GetNpcCardStatus983(m_type983);
	if (npc > 0)
	{
		if (count)
			text += '\n';
		text += MakeStr(GetGameString(0x18837), npc);
		count++;
	}
	int card = CCardManager::Instance()->GetCardPeriodStatus(m_type983);
	if (card > 0)
	{
		if (count)
			text += '\n';
		text += MakeStr(GetGameString(0x18838), card);
		count++;
	}
	if (text.empty())
		return;
	ShowTip983(true, text.c_str(), LABEL_PARTS983, 0xff000000);
}

// 005ca410 (egg3): encaixes do atributo (vermelho se o PCL passa dos encaixes)
void RFCharMasteryStatDetailWnd::OnSlotEnter983(EventArgs& e)
{
	std::string text;
	CStatusHelper helper;
	int pcl = helper.GetMyPCL983(m_type983, true, NULL);
	int slot = helper.GetMySlot983(m_type983);
	unsigned long color = pcl > slot ? 0xffff0000 : 0xff000000;
	text += MakeStr("[%s : %d/%d]", helper.GetStatName983(m_type983), pcl, slot);
	int level = helper.GetMasterySlot983(m_type983, NULL) + helper.GetLevelSlot983(m_type983, -1);
	text += level > 0 ? MakeStr(GetGameString(0x18839), level) : "";
	int parts = helper.GetMyPartsSlot983(m_type983);
	text += parts > 0 ? MakeStr(GetGameString(0x1883a), parts) : "";
	int card = helper.GetMyCardSlot983(m_type983);
	text += card > 0 ? MakeStr(GetGameString(0x1883b), card) : "";
	if (text.empty())
		return;
	ShowTip983(true, text.c_str(), LABEL_SLOT983, color);
}

// 005ca640 (egg4): penalidade de força
void RFCharMasteryStatDetailWnd::OnPenaltyEnter983(EventArgs& e)
{
	std::string text;
	CStatusHelper helper;
	int penalty = helper.GetMyPenalty983(m_type983);
	if (penalty < 1)
		return;
	text += MakeStr(GetGameString(0x1883c), helper.GetStatus983(0), penalty);
	if (text.empty())
		return;
	ShowTip983(true, text.c_str(), LABEL_PENALTY983, 0xff000000);
}

// 005ca770
void RFCharMasteryStatDetailWnd::OnTipLeave983(EventArgs& e)
{
	ShowTip983(false, NULL, LABEL_CHAR983, 0xff000000);
}

// 005ca790
void RFCharMasteryStatDetailWnd::OnCapEnter983(EventArgs& e)
{
	if (m_open983 == 0)
		PlayAnimation(L"_over_status_detail_");
	std::string text;
	CStatusHelper helper;
	int over = helper.GetMyOverSlot983(m_type983);
	if (over < 1)
		return;
	text += MakeStr(GetGameString(0x1883d), over);
	ShowTip983(true, text.c_str(), LABEL_TOTAL983, 0xff000000);
}

// 005ca8e0
void RFCharMasteryStatDetailWnd::OnCapLeave983(EventArgs& e)
{
	if (m_open983 == 0)
		PlayAnimation(L"_out_status_detail_");
	ShowTip983(false, NULL, LABEL_CHAR983, 0xff000000);
}

// 005ca9b0: o que o personagem em uso tem que dá ponto nesse atributo
void RFCharMasteryStatDetailWnd::LoadEquip983()
{
	sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
	if (pInfo)
	{
		m_parts983.clear();
		for (int i = 0; i < 24; i++)
		{
			if (pInfo->tidParts[i])
			{
				IFF_STRUCT::sPart* p = ItemManager()->FindPart(pInfo->tidParts[i]);
				if (p && 0 < p->Attr[m_type983])
					m_parts983.push_back(p);
			}
		}
		m_auxParts983.clear();
		for (int i = 0; i < 5; i++)
		{
			if (pInfo->tidAuxParts[i])
			{
				IFF_STRUCT::sAuxPart* p = ItemManager()->FindAuxPart(pInfo->tidAuxParts[i]);
				if (p && p->Attr[m_type983] > 0)
					m_auxParts983.push_back(p);
			}
		}

		std::map<unsigned int, sCaddieInfo>::iterator itCaddie = Doc()->m_caddieMap.find(Doc()->m_myInfo.userEquip.guidCaddie);
		m_pCaddie983 = itCaddie == Doc()->m_caddieMap.end() ? NULL : ItemManager()->FindCaddie(itCaddie->second.tid);
		std::map<unsigned int, sMascotInfo>::iterator itMascot = Doc()->m_mascotMap.find(Doc()->m_myInfo.userEquip.guidMascot);
		m_pMascot983 = itMascot == Doc()->m_mascotMap.end() ? NULL : ItemManager()->FindMascot(itMascot->second.tid);
		std::map<unsigned int, sItemInfo>::iterator itClub = Doc()->m_clubSetMap.find(Doc()->m_myInfo.userEquip.guidClubSet);
		m_pClub983 = itClub == Doc()->m_clubSetMap.end() ? NULL : ItemManager()->FindClubSet(itClub->second.tid);
		m_pClubInfo983 = itClub == Doc()->m_clubSetMap.end() ? NULL : &itClub->second;
	}
}

// 005cacd0
void RFCharMasteryStatColumn::OnInitialUpdate()
{
	RegisterTo(L"status_button", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryStatColumn::OnEnter983, this));
	RegisterTo(L"status_button", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryStatColumn::OnLeave983, this));
	RegisterTo(L"status_button", RFButton::EventClicked, Subscriber(&RFCharMasteryStatColumn::OnClick983, this));
	m_pButton983 = GetChild983<RFButton>(L"status_button");
	m_pMax983 = GetChild983<RFPicture>(L"max_img");
	RFPicture* stat = GetChild983<RFPicture>(L"status_bar_stat");
	RFPicture* slot = GetChild983<RFPicture>(L"status_bar_slot");
	RFPicture* bg = GetChild983<RFPicture>(L"status_bar_bg");
	m_pGauge983 = new MasteryGaugeBar(stat, slot, bg);
	RFWndEx* mark = GetChild983<RFWndEx>(L"status_detail_wnd");
	if (mark)
	{
		m_pDetail983 = RFCharMasteryStatDetailWnd::Create983(this);
		if (m_pDetail983)
		{
			m_pDetail983->SetPos(mark->GetPos().x, mark->GetPos().y);
			m_pDetail983->ShowWindow983(true);
		}
		DeleteChild(mark);
	}
}

// 005cb260
void RFMasteryStatUpDnDlg::Set983(unsigned int type, int mode)
{
	CStatusHelper helper;
	if (type < 5)
	{
		m_type983 = type;
		m_mode983 = mode;
		UpdateImages983();
		UpdateSlots983();
		UpdateText983();
	}
}

// 005cb2f0
void RFMasteryStatUpDnDlg::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFMasteryStatUpDnDlg::OnKey983, this));
	RegisterTo(L"button_confirm", RFButton::EventClicked, Subscriber(&RFMasteryStatUpDnDlg::OnOk983, this));
	RegisterTo(L"button_cancel", RFButton::EventClicked, Subscriber(&RFMasteryStatUpDnDlg::OnCancel983, this));
	m_pImg983[IMG_ARROW983] = GetChild983<RFPicture>(L"arrow_img");
	m_pImg983[IMG_TITLE983] = GetChild983<RFPicture>(L"title_img");
	m_pImg983[IMG_STATBG1_983] = GetChild983<RFPicture>(L"stat_bg1_img");
	m_pImg983[IMG_STATBG2_983] = GetChild983<RFPicture>(L"stat_bg2_img");
	m_pNum983[NUM_TOTAL983] = GetChild983<RFLabel>(L"total_stat_num");
	m_pNum983[NUM_BEFORE983] = GetChild983<RFLabel>(L"before_slot_num");
	m_pNum983[NUM_AFTER983] = GetChild983<RFLabel>(L"after_slot_num");
	m_pMessage983 = GetChild983<RFTextArea>(L"message");
	m_pWarning983 = GetChild983<RFTextArea>(L"warnning_text");
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005cb910: Enter = ok, ESC = fecha
void RFMasteryStatUpDnDlg::OnKey983(EventArgs& e)
{
	switch (e.m_0c)
	{
	case 0xd:
		OnOk983(e);
		e.handled = true;
		break;
	case 0x1b:
		Close(2);
		e.handled = true;
		break;
	}
}

// 005cb950: números do atributo
void RFCharMasteryStatDetailWnd::Update983(unsigned long type)
{
	m_type983 = type;
	CStatusHelper helper;
	int i = 0;
	if (m_type983 < 5)
	{
		int value[5] = { 0 };
		value[LABEL_CHAR983] = helper.GetMyCharStat983(type);
		value[LABEL_PARTS983] = helper.GetMyEquipStat983(type);
		value[LABEL_SLOT983] = helper.GetMyPCL983(type, false, NULL);
		value[LABEL_PENALTY983] = helper.GetMyPenalty983(type);
		value[LABEL_TOTAL983] = helper.GetStatus983(type);
		int extra = helper.GetMyFreeSlot983(m_type983);
		do
		{
			if (m_pLabel983[i])
			{
				int v = value[i];
				std::string text(MakeStr("%d", v));
				if (i > 0)
				{
					if (i < 3)
						text = MakeStr("+%d", v);
					else if (i == 3)
						text = MakeStr("-%d", value[LABEL_PENALTY983]);
				}
				m_pLabel983[i]->SetLabel(MultiByteToWide983(text.c_str()));
			}
			i++;
		} while (i < 5);
		m_pLabel983[LABEL_TOTAL983]->SetColor983(helper.GetMyOverSlot983(m_type983) != 0 ? 0xff000000 : 0xffffffff); // D877: RFLabel +0x1c8 (_fontInfo.color)
		if (m_pLabel983[LABEL_EXTRA983])
			m_pLabel983[LABEL_EXTRA983]->SetLabel(extra, false);
		if (m_pUp983)
			m_pUp983->EnableWindow(extra > 0);
		if (m_pDown983)
			m_pDown983->EnableWindow(value[LABEL_SLOT983] > 0);
		LoadEquip983();
	}
}

// 005cbc30
void RFCharMasteryStatDetailWnd::OnStatUp983(EventArgs& e)
{
	if (!RFMasteryStatUpDnDlg::ms_pInstance)
	{
		RFMasteryStatUpDnDlg* dlg = RFMasteryStatUpDnDlg::Create983(NULL);
		if (dlg)
		{
			dlg->Set983(m_type983, RFMasteryStatUpDnDlg::MODE_UP983);
			dlg->DoModal();
		}
	}
}

// 005cbc70
void RFCharMasteryStatDetailWnd::OnStatDown983(EventArgs& e)
{
	if (!RFMasteryStatUpDnDlg::ms_pInstance)
	{
		RFMasteryStatUpDnDlg* dlg = RFMasteryStatUpDnDlg::Create983(this);
		if (dlg)
		{
			dlg->Set983(m_type983, RFMasteryStatUpDnDlg::MODE_DOWN983);
			dlg->DoModal();
		}
	}
}

// 005cbcb0
void RFCharMasteryStatColumn::UpdateDetail983()
{
	if (m_pDetail983)
		m_pDetail983->Update983(m_type983);
}

// 005cbcd0
void RFCharMasteryStatDetailWnd::OnInitialUpdate()
{
	RegisterTo(L"cap_btn", RFButton::EventClicked, Subscriber(&RFCharMasteryStatDetailWnd::OnCap983, this));
	RegisterTo(L"button_stat_up", RFButton::EventClicked, Subscriber(&RFCharMasteryStatDetailWnd::OnStatUp983, this));
	RegisterTo(L"button_stat_dn", RFButton::EventClicked, Subscriber(&RFCharMasteryStatDetailWnd::OnStatDown983, this));
	RegisterTo(L"egg1", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryStatDetailWnd::OnBaseEnter983, this));
	RegisterTo(L"egg1", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryStatDetailWnd::OnTipLeave983, this));
	RegisterTo(L"egg2", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryStatDetailWnd::OnEquipEnter983, this));
	RegisterTo(L"egg2", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryStatDetailWnd::OnTipLeave983, this));
	RegisterTo(L"egg3", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryStatDetailWnd::OnSlotEnter983, this));
	RegisterTo(L"egg3", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryStatDetailWnd::OnTipLeave983, this));
	RegisterTo(L"egg4", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryStatDetailWnd::OnPenaltyEnter983, this));
	RegisterTo(L"egg4", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryStatDetailWnd::OnTipLeave983, this));
	RegisterTo(L"cap_btn", RFWnd::EventMouseEnter, Subscriber(&RFCharMasteryStatDetailWnd::OnCapEnter983, this));
	RegisterTo(L"cap_btn", RFWnd::EventMouseLeave, Subscriber(&RFCharMasteryStatDetailWnd::OnCapLeave983, this));
	m_pLabel983[LABEL_CHAR983] = GetChild983<RFLabel>(L"char_stat_num_label");
	m_pLabel983[LABEL_PARTS983] = GetChild983<RFLabel>(L"parts_stat_num_label");
	m_pLabel983[LABEL_SLOT983] = GetChild983<RFLabel>(L"slot_stat_num_label");
	m_pLabel983[LABEL_PENALTY983] = GetChild983<RFLabel>(L"penalty_stat_num_label");
	m_pLabel983[LABEL_TOTAL983] = GetChild983<RFLabel>(L"total_stat_num_label");
	m_pLabel983[LABEL_EXTRA983] = GetChild983<RFLabel>(L"extra_slot_num_label");
	m_pUp983 = GetChild983<RFButton>(L"button_stat_up");
	m_pDown983 = GetChild983<RFButton>(L"button_stat_dn");
	m_pTip983 = RFMasteryToolTipWnd::Create983(GetSuper(false)->GetSuper(false)->GetSuper(false));
}

// 005cc970
void RFCharMasteryStatColumn::Update983()
{
	UpdateGauge983();
	UpdateDetail983();
}

// 005cc980: cartas recalculadas, aviso 0x109 ao ator principal e redesenho das linhas
void RFCharMasteryStatusDlg::Refresh983()
{
	CCardManager::Instance()->CalcCardPeriodAndStatus();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x109 /*983*/, 0, 0, 0, 0, 0));
	if (m_pList983)
	{
		int count = m_pList983->GetItemCount983();
		for (int i = 0; i < count; i++)
		{
			RFCharMasteryStatColumn* column = (RFCharMasteryStatColumn*)m_pList983->GetItemData(i);
			if (column)
				column->Update983();
		}
	}
}

// 005cca30
void RFCharMasteryStatColumn::SetType983(unsigned long type)
{
	m_type983 = type;
	UpdateButton983();
	UpdateBar983();
	UpdateCap983();
	Update983();
}

// 005cca60
void RFCharMasteryStatusDlg::InitList983()
{
	m_pList983 = GetChild983<RFList>(L"status_list");
	if (m_pList983)
	{
		for (int i = 0; i < 5; i++)
		{
			// C295: pai = a lista (983 005ccb05: ecx = [edi+0x184]); com this a coluna ficava filha de dois e era apagada duas vezes ao fechar
			RFCharMasteryStatColumn* column = RFCharMasteryStatColumn::Create983(m_pList983);
			if (column)
			{
				column->SetType983(i);
				column->ShowWindow983(true);
				m_pList983->InsertWindowItem(column);
			}
		}
	}
}

// 005ccb60
void RFCharMasteryStatusDlg::OnInitialUpdate()
{
	CCardManager::Instance()->CalcCardPeriodAndStatus();
	InitList983();
}
