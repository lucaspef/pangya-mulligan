// JP 983: unidade 005d8160–005dfa30 — janelas RF do evento de aniversário da Arin 2014: RF2014ArinBirthdayDlg (a janela do
// evento), RF2014ArinStage1Wnd (bilhetes), RF2014ArinStage2Wnd (missões/magia), RF2014ArinNotifyDlg, RF2014ArinMissionInfoDlg e
// RF2014ArinMapMissionInfoDlg. Os cortes rf2014arinnotifydlg/stage1wnd/stage2wnd da F3.7.1 são esta unidade; começa nos acessores
// de instância (005d8160+). Sessão B143 (F3.7.12 item 7). Nomes meus (…983), menos as classes e os da RF; ver
// include/jp/refresh/rf2014arinbirthdaydlg.h.
#include "stdafx.h"
#include "refresh/rf2014arinbirthdaydlg.h"
#include "arinbirthday2014doc.h"
#include "wlocalize.h"
#include "projectg.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "gamestringmanager.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"

using namespace _gateway;

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

RF2014ArinBirthdayDlg* RF2014ArinBirthdayDlg::s_pInstance;
RFWnd* RF2014ArinBirthdayDlg::s_pParent983;
RFWnd* RF2014ArinStage1Wnd::s_pParent983;
RFWnd* RF2014ArinStage2Wnd::s_pParent983;
RF2014ArinNotifyDlg* RF2014ArinNotifyDlg::ms_pInstance;
RFWnd* RF2014ArinNotifyDlg::s_pParent983;
RF2014ArinMissionInfoDlg* RF2014ArinMissionInfoDlg::ms_pInstance;
RFWnd* RF2014ArinMissionInfoDlg::s_pParent983;
RF2014ArinMapMissionInfoDlg* RF2014ArinMapMissionInfoDlg::ms_pInstance;
RFWnd* RF2014ArinMapMissionInfoDlg::s_pParent983;

// 00d11768: os 5 bilhetes do evento (itens)
const unsigned long RF2014ArinStage1Wnd::s_ticketItems983[TICKET_COUNT983] = { 0x1a0002bb, 0x1a0002bc, 0x1a0002bd, 0x1a0002be, 0x1a0002bf };

// 005d8170
void RF2014ArinBirthdayDlg::SetInstance983()
{
	s_pInstance = this;
}

// 005d8180
void RF2014ArinBirthdayDlg::ClearInstance983()
{
	s_pInstance = NULL;
}

// 005d8190
void RF2014ArinNotifyDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005d81a0
void RF2014ArinNotifyDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005d81b0
void RF2014ArinMissionInfoDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005d81c0
void RF2014ArinMissionInfoDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005d81d0
void RF2014ArinMapMissionInfoDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005d81e0
void RF2014ArinMapMissionInfoDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005d81f0
void RF2014ArinBirthdayDlg::SetDocument983(CArinBirthDay2014Doc* pDoc)
{
	m_pDoc = pDoc;
}

// 005d8210
void RF2014ArinBirthdayDlg::OnClose983(EventArgs& e)
{
	Close(2);
}

// 005d8270
bool RF2014ArinStage1Wnd::HasTicket983(int index)
{
	return m_count983[index] > 0 ? true : false;
}

// 005d8290
void RF2014ArinStage1Wnd::UseTicket983(int index)
{
	if (m_count983[index])
	{
		m_count983[index]--;
		m_bUsed983[index] = true;
	}
}

// 005d82c0
void RF2014ArinStage2Wnd::SetStep983(int step)
{
	m_step983 = step;
}

// 005d82d0
int RF2014ArinStage2Wnd::CountRewarded983()
{
	int n = 0;
	if (m_bRewarded983[0])
		n = 1;
	if (m_bRewarded983[1])
		n++;
	if (m_bRewarded983[2])
		n++;
	if (m_bRewarded983[3])
		n++;
	if (m_bRewarded983[4])
		n++;
	return n;
}

// 005d8310
void RF2014ArinNotifyDlg::OnClose983(EventArgs& e)
{
	Close(2);
}

// 005d8320
void RF2014ArinMissionInfoDlg::OnClose983(EventArgs& e)
{
	Close(2);
}

// 005d8330
void RF2014ArinMapMissionInfoDlg::OnClose983(EventArgs& e)
{
	Close(2);
}

// 005d86e0
void RF2014ArinBirthdayDlg::OnKeyDown983(EventArgs& e)
{
	if (e.m_0c == VK_ESCAPE)
	{
		OnClose983(e);
		e.handled = true;
	}
}

// 005d8700
RF2014ArinStage1Wnd::RF2014ArinStage1Wnd()
{
	for (int i = 0; i < TICKET_COUNT983; i++)
	{
		m_count983[i] = 0;
		m_bUsed983[i] = false;
	}
}

// 005d8740
RF2014ArinStage1Wnd::~RF2014ArinStage1Wnd()
{
}

// 005d8750
RF2014ArinNotifyDlg::RF2014ArinNotifyDlg()
{
	ms_pInstance = this;
}

// 005d87e0
RF2014ArinNotifyDlg::~RF2014ArinNotifyDlg()
{
	ms_pInstance = NULL;
}

// 005d9020
void RF2014ArinBirthdayDlg::SendStart983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	WSendPacket packet((enumClientPacket)0x190);
	packet.Send(TO_GAME);
}

// 005d9100
void RF2014ArinStage1Wnd::SendTicketsDone983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	WSendPacket packet((enumClientPacket)0x191);
	packet.Send(TO_GAME);
}

// 005d91b0
void RF2014ArinStage2Wnd::SendReward983(int index)
{
	if (m_missions983[index].bDone && m_missions983[index].bRewarded != true)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)0x193);
		packet.Encode4(index + 0x15);
		packet.Send(TO_GAME);
	}
}

// 005d9290
void RF2014ArinStage2Wnd::SendFinal983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	WSendPacket packet((enumClientPacket)0x194);
	packet.Send(TO_GAME);
}

// 005d97f0
void RF2014ArinStage2Wnd::OnReward1_983(EventArgs& e)
{
	SendReward983(0);
}

// 005d9800
void RF2014ArinStage2Wnd::OnReward2_983(EventArgs& e)
{
	SendReward983(1);
}

// 005d9810
void RF2014ArinStage2Wnd::OnReward3_983(EventArgs& e)
{
	SendReward983(2);
}

// 005d9820
void RF2014ArinStage2Wnd::OnReward4_983(EventArgs& e)
{
	SendReward983(3);
}

// 005d9830
void RF2014ArinStage2Wnd::OnReward5_983(EventArgs& e)
{
	SendReward983(4);
}

// 005d9840
void RF2014ArinStage2Wnd::OnFinal983(EventArgs& e)
{
	SendFinal983();
}

// 005d99b0
void RF2014ArinStage1Wnd::SetTicketCount983(int index, int count)
{
	RFLabel* pLabel = GetChild983<RFLabel>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "label_ticket", index + 1))));
	m_count983[index] = count;
	pLabel->SetLabel(count, false);
}

// 005d9ae0
void RF2014ArinStage1Wnd::ShowTicketMark983(int index, bool show)
{
	RFPicture* pPic = GetChild983<RFPicture>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s%d", "pic_event_ticket", index + 1))));
	pPic->ShowWindow(show);
}

// 005d9c00
void RF2014ArinStage2Wnd::ShowMissionBtn983(int index, bool show)
{
	RFButton* pBtn = GetChild983<RFButton>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "btn_mission", index + 1))));
	pBtn->ShowWindow(show);
}

// 005d9d20
void RF2014ArinStage2Wnd::ShowRewardBtn983(int index, bool show)
{
	RFButton* pBtn = GetChild983<RFButton>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "btn_reward", index + 1))));
	pBtn->ShowWindow(show);
}

// 005d9e40
void RF2014ArinStage2Wnd::SetMissionBtnState983(int index, int state)
{
	RFButton* pBtn = GetChild983<RFButton>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "btn_mission", index + 1))));
	pBtn->SetState(state);
}

// 005d9f80
void RF2014ArinStage2Wnd::SetRewardBtnState983(int index, int state)
{
	RFButton* pBtn = GetChild983<RFButton>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "btn_reward", index + 1))));
	pBtn->SetState(state);
}

// 005da3f0
void RF2014ArinBirthdayDlg::ShowStage983(int index)
{
	RFTextArea* pText = GetChild983<RFTextArea>(L"text_area");
	pText->SetText(MultiByteToWide983(K2L_Compatibility(m_190[index].c_str())));
}

// 005da550
void RF2014ArinBirthdayDlg::ShowOk983(bool show)
{
	RFButton* pBtn = GetChild983<RFButton>(L"btn_ok");
	pBtn->ShowWindow(show);
}

// 005da620
void RF2014ArinBirthdayDlg::SetBg2_983()
{
	RFPicture* pPic = GetChild983<RFPicture>(L"pic_arinbirth_bg");
	pPic->SetPicture(L"pic_arinbirth_bg_02.png");
}

// 005da720
void RF2014ArinBirthdayDlg::SetBg1_983()
{
	RFPicture* pPic = GetChild983<RFPicture>(L"pic_arinbirth_bg");
	pPic->SetPicture(L"pic_arinbirth_bg_01.png");
}

// 005da820
void RF2014ArinBirthdayDlg::SetTiki983(int index)
{
	RFPicture* pPic = GetChild983<RFPicture>(L"pic_char");
	pPic->SetPicture(MultiByteToWide983(MakeStr("pic_tiki_%d.png", index + 1)));
}

// 005da970
void RF2014ArinBirthdayDlg::SetArin983(int index)
{
	RFPicture* pPic = GetChild983<RFPicture>(L"pic_char");
	pPic->SetPicture(MultiByteToWide983(MakeStr("pic_arin_%d.png", index + 1)));
}

// 005daac0
RF2014ArinStage1Wnd* RF2014ArinStage1Wnd::Create983(RFWnd* parent)
{
	RF2014ArinStage1Wnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinStage1Wnd>(L"event_arinbirth_ticket_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinStage1Wnd>(L"event_arinbirth_ticket_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RF2014ArinStage1Wnd>(L"event_arinbirth_ticket_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005dac00
void RF2014ArinStage1Wnd::ShowEventTicket983(bool show)
{
	RFPicture* pPic = GetChild983<RFPicture>(L"pic_event_ticket");
	pPic->ShowWindow(show);
}

// 005dacd0
RF2014ArinStage2Wnd::RF2014ArinStage2Wnd()
{
	m_step983 = 0;
	m_19d_983 = false;
	memset(m_bRewarded983, 0, sizeof(m_bRewarded983));
}

// 005dad10
RF2014ArinStage2Wnd::~RF2014ArinStage2Wnd()
{
}

// 005dadb0
void RF2014ArinStage2Wnd::SetMagicBg983(int count)
{
	RFPicture* pPic = GetChild983<RFPicture>(L"pic_event_magic_bg");
	pPic->SetPicture(MultiByteToWide983(MakeStr("pic_event_magic_bg_%02d.png", count)));
}

// 005daf00
void RF2014ArinStage2Wnd::ShowFinalBtn983(bool show)
{
	RFButton* pBtn = GetChild983<RFButton>(L"btn_final");
	pBtn->ShowWindow(show);
}

// 005dafd0
void RF2014ArinStage2Wnd::ShowFinalPic983(bool show)
{
	RFPicture* pPic = GetChild983<RFPicture>(L"pic_final_1");
	pPic->ShowWindow(show);
	pPic = GetChild983<RFPicture>(L"pic_final_2");
	pPic->ShowWindow(show);
}

// 005db110
bool RF2014ArinStage2Wnd::IsAllRewarded983()
{
	if (m_step983 == 0xb)
		return false;
	for (int i = 0; i < MISSION_COUNT983; i++)
	{
		if (!m_bRewarded983[i])
			return false;
	}
	PlayAnimation(L"_Final_");
	return true;
}

// 005db1d0
RF2014ArinNotifyDlg* RF2014ArinNotifyDlg::Create983(RFWnd* parent)
{
	RF2014ArinNotifyDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinNotifyDlg>(L"event_arinbirth_popup_1_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinNotifyDlg>(L"event_arinbirth_popup_1_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RF2014ArinNotifyDlg>(L"event_arinbirth_popup_1_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005db310
void RF2014ArinNotifyDlg::SetText983(std::string text)
{
	RFTextArea* pText = GetChild983<RFTextArea>(L"TextArea");
	if (pText)
		pText->SetText(MultiByteToWide983(K2L_Compatibility(text.c_str())));
}

// 005db470
void RF2014ArinNotifyDlg::OnInitialUpdate()
{
	RegisterTo(L"btn_evert_arinbirth_ckeck", RFButton::EventClicked, Subscriber(&RF2014ArinNotifyDlg::OnClose983, this));
}

// 005db570
void RF2014ArinMissionInfoDlg::SetMission983(int index, const sArinMission983& mission)
{
	RFLabel* pLabel = GetChild983<RFLabel>(L"Label");
	if (pLabel)
		pLabel->SetLabel(MultiByteToWide983(K2L_Compatibility(MakeStr(m_texts983[index].title.c_str(), mission.count))));
	RFTextArea* pText = GetChild983<RFTextArea>(L"TextArea");
	if (pText)
		pText->SetText(MultiByteToWide983(K2L_Compatibility(MakeStr(m_texts983[index].text.c_str(), mission.value, mission.count))));
}

// 005db850
void RF2014ArinMissionInfoDlg::OnInitialUpdate()
{
	RegisterTo(L"btn_evert_arinbirth_ckeck", RFButton::EventClicked, Subscriber(&RF2014ArinMissionInfoDlg::OnClose983, this));
}

// 005db950: 5ª missão — os mapas feitos (bits do valor) ganham a imagem "_o"
void RF2014ArinMapMissionInfoDlg::SetMission983(const sArinMission983& mission)
{
	RFLabel* pLabel = GetChild983<RFLabel>(L"Label");
	if (pLabel)
		pLabel->SetLabel(MultiByteToWide983(K2L_Compatibility(m_texts983[0].title.c_str())));
	if (mission.value & 0x20)
	{
		RFPicture* pPic = GetChild983<RFPicture>(L"pic_westwiz");
		if (pPic)
			pPic->SetPicture(L"pic_westwiz_o.png");
	}
	if (mission.value & 0x40000)
	{
		RFPicture* pPic = GetChild983<RFPicture>(L"pic_wizcity");
		if (pPic)
			pPic->SetPicture(L"pic_wizcity_o.png");
	}
	if (mission.value & 0x200)
	{
		RFPicture* pPic = GetChild983<RFPicture>(L"pic_whitewiz");
		if (pPic)
			pPic->SetPicture(L"pic_whitewiz_o.png");
	}
	if (mission.value & 0x10)
	{
		RFPicture* pPic = GetChild983<RFPicture>(L"pic_wizwiz");
		if (pPic)
			pPic->SetPicture(L"pic_wizwiz_o.png");
	}
}

// 005dbde0
void RF2014ArinMapMissionInfoDlg::OnInitialUpdate()
{
	RegisterTo(L"btn_evert_arinbirth_ckeck", RFButton::EventClicked, Subscriber(&RF2014ArinMapMissionInfoDlg::OnClose983, this));
}

// 005dc280
void RF2014ArinBirthdayDlg::OpenMissionInfo983()
{
	RF2014ArinNotifyDlg* p = RF2014ArinNotifyDlg::Create983(NULL);
	if (p)
	{
		p->SetText983(GetGameString(0x30e1e));
		p->CenterWindow(NULL);
		p->DoModal();
	}
}

// 005dc2f0
void RF2014ArinBirthdayDlg::OpenMapMissionInfo983()
{
	RF2014ArinNotifyDlg* p = RF2014ArinNotifyDlg::Create983(NULL);
	if (p)
	{
		p->SetText983(GetGameString(0x30e1f));
		p->CenterWindow(NULL);
		p->DoModal();
	}
}

// 005dc360
void RF2014ArinStage1Wnd::CheckAll983()
{
	for (int i = 0; i < TICKET_COUNT983; i++)
	{
		if (!m_bUsed983[i])
			return;
	}
	ShowEventTicket983(true);
	SendTicketsDone983();
}

// 005dc390
RF2014ArinStage2Wnd* RF2014ArinStage2Wnd::Create983(RFWnd* parent)
{
	RF2014ArinStage2Wnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinStage2Wnd>(L"event_arinbirth_magic_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinStage2Wnd>(L"event_arinbirth_magic_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RF2014ArinStage2Wnd>(L"event_arinbirth_magic_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005dc500
void RF2014ArinStage2Wnd::ShowAll983(bool show)
{
	for (int i = 0; i < MISSION_COUNT983; i++)
	{
		ShowMissionBtn983(i, show);
		ShowRewardBtn983(i, show);
	}
	ShowFinalBtn983(show);
	ShowFinalPic983(show);
}

// 005dc540
void RF2014ArinStage2Wnd::Refresh983()
{
	ShowAll983(false);
	for (unsigned int i = 0; i < m_missions983.size(); i++)
	{
		if (m_missions983[i].bDone)
		{
			ShowMissionBtn983(i, false);
			if (m_missions983[i].bRewarded)
			{
				m_bRewarded983[i] = true;
				ShowRewardBtn983(i, true);
				SetRewardBtnState983(i, 3);
			}
			else
			{
				SetRewardBtnState983(i, 0);
				ShowRewardBtn983(i, true);
			}
		}
		else
			ShowMissionBtn983(i, true);
	}
	ShowFinalBtn983(IsAllRewarded983());
	ShowFinalPic983(IsAllRewarded983());
}

// 005dc600
void RF2014ArinStage2Wnd::OnRewarded983(int index)
{
	m_bRewarded983[index] = true;
	SetRewardBtnState983(index, 3);
	ShowFinalBtn983(IsAllRewarded983());
	ShowFinalPic983(IsAllRewarded983());
	SetMagicBg983(CountRewarded983());
}

// 005dc650
void RF2014ArinStage2Wnd::CheckFinal983()
{
	if (m_step983 == 0xb)
	{
		for (int i = 0; i < MISSION_COUNT983; i++)
			ShowMissionBtn983(i, false);
		ShowFinalBtn983(false);
		ShowFinalPic983(false);
		RFPicture* pPic = GetChild983<RFPicture>(L"pic_event_magic_bg");
		pPic->SetPicture(L"pic_event_magic_bg_05.png");
		StopAnimation();
	}
}

// 005dc850
void RF2014ArinBirthdayDlg::SetMissionResult983(int index)
{
	RFWnd* pCtrl = GetChild(L"wnd_event_ctrl");
	RF2014ArinStage2Wnd* p = pCtrl->GetChild983<RF2014ArinStage2Wnd>(L"wnd_event_magic_ctrl");
	if (p)
		p->OnRewarded983(index);
}

// 005dc970
void RF2014ArinBirthdayDlg::ShowMapResult983()
{
	RFWnd* pCtrl = GetChild(L"wnd_event_ctrl");
	RF2014ArinStage2Wnd* p = pCtrl->GetChild983<RF2014ArinStage2Wnd>(L"wnd_event_magic_ctrl");
	if (p)
	{
		p->SetStep983(0xb);
		p->CheckFinal983();
	}
}

// 005dca90
void RF2014ArinStage1Wnd::OnTicket983(int index)
{
	if (HasTicket983(index))
	{
		UseTicket983(index);
		SetTicketCount983(index, m_count983[index]);
		ShowTicketMark983(index, true);
		CheckAll983();
	}
}

// 005dcad0
void RF2014ArinStage1Wnd::UseOwnedTickets983()
{
	for (int i = 0; i < TICKET_COUNT983; i++)
	{
		unsigned long typeId = s_ticketItems983[i];
		RFButton* pBtn = GetChild983<RFButton>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "btn_ticket", i + 1))));
		RFLabel* pLabel = GetChild983<RFLabel>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "label_ticket", i + 1))));
		if (Doc()->FindMyItemByTypeId983(typeId))
		{
			OnTicket983(i);
			if (m_count983[i] < 1)
				pBtn->EnableWindow(false);
		}
	}
}

// 005dcd40
void RF2014ArinStage2Wnd::SetMissions983(const std::vector<sArinMission983>& missions)
{
	m_missions983 = missions;
	Refresh983();
	SetMagicBg983(CountRewarded983());
	CheckFinal983();
}

// 005dd210: ok — passo seguinte
void RF2014ArinBirthdayDlg::OnOk983(EventArgs& e)
{
	switch (m_step)
	{
	case 0:
		SetStep983(1);
		break;
	case 1:
		SetStep983(2);
		break;
	case 2:
		SetStep983(3);
		break;
	case 3:
	case 6:
		SetStep983(7);
		break;
	case 4:
		SendStart983();
		break;
	case 5:
		SetStep983(6);
		break;
	case 7:
		SetStep983(8);
		break;
	case 8:
		SetStep983(9);
		break;
	}
	UpdateStep983();
}

// 005dcdd0
void RF2014ArinBirthdayDlg::UpdateStep983()
{
	RFPicture* pBg = GetChild983<RFPicture>(L"pic_arinbirth_bg");
	RFWnd* pCtrl = GetChild(L"wnd_event_ctrl");
	RFPicture* pChar = GetChild983<RFPicture>(L"pic_char");
	switch (m_step)
	{
	case 0:
		SetBg1_983();
		ShowStage983(3);
		SetTiki983(2);
		break;
	case 1:
		ShowStage983(4);
		SetTiki983(3);
		break;
	case 2:
	{
		ShowStage983(5);
		SetTiki983(2);
		RF2014ArinStage1Wnd* p = RF2014ArinStage1Wnd::Create983(pCtrl);
		p->CenterWindow(pCtrl);
		p->ShowWindow983(true);
		ShowOk983(false);
		break;
	}
	case 3:
		ShowStage983(6);
		SetTiki983(1);
		ShowOk983(true);
		break;
	case 4:
		SetBg1_983();
		ShowStage983(0);
		SetTiki983(0);
		break;
	case 5:
		ShowStage983(1);
		SetTiki983(1);
		break;
	case 6:
		ShowStage983(2);
		SetTiki983(0);
		break;
	case 7:
	{
		RFWnd* pTicket = pCtrl->GetChild(L"wnd_event_ticket_ctrl");
		if (pTicket)
			pTicket->ShowWindow(false);
		ShowStage983(7);
		SetArin983(0);
		ShowOk983(true);
		break;
	}
	case 8:
		ShowStage983(8);
		SetArin983(1);
		break;
	case 9:
	{
		SetBg2_983();
		RFWnd* pTicket = pCtrl->GetChild(L"wnd_event_ticket_ctrl");
		if (pTicket)
			pTicket->ShowWindow(false);
		ShowStage983(9);
		SetArin983(2);
		RF2014ArinStage2Wnd* p = RF2014ArinStage2Wnd::Create983(pCtrl);
		p->SetStep983(9);
		p->SetMissions983(m_pDoc->GetMissions983());
		p->CenterWindow(pCtrl);
		p->ShowWindow983(true);
		pChar->SetPicture(L"pic_arin_1.png");
		ShowOk983(false);
		break;
	}
	case 10:
		SetBg2_983();
		ShowStage983(0xb);
		SetArin983(3);
		break;
	case 0xb:
	{
		SetBg2_983();
		ShowStage983(0xc);
		SetArin983(0);
		pCtrl->ShowWindow(true);
		RF2014ArinStage2Wnd* p = RF2014ArinStage2Wnd::Create983(pCtrl);
		p->SetStep983(0xb);
		p->SetMissions983(m_pDoc->GetMissions983());
		p->CenterWindow(pCtrl);
		p->ShowWindow983(true);
		ShowOk983(false);
		break;
	}
	}
}

// 005dd2e0
void RF2014ArinStage1Wnd::OnInitialUpdate()
{
	RegisterTo(L"btn_ticket_01", RFWnd::EventLButtonDblClick, Subscriber(&RF2014ArinStage1Wnd::OnTicket1_983, this));
	RegisterTo(L"btn_ticket_02", RFWnd::EventLButtonDblClick, Subscriber(&RF2014ArinStage1Wnd::OnTicket2_983, this));
	RegisterTo(L"btn_ticket_03", RFWnd::EventLButtonDblClick, Subscriber(&RF2014ArinStage1Wnd::OnTicket3_983, this));
	RegisterTo(L"btn_ticket_04", RFWnd::EventLButtonDblClick, Subscriber(&RF2014ArinStage1Wnd::OnTicket4_983, this));
	RegisterTo(L"btn_ticket_05", RFWnd::EventLButtonDblClick, Subscriber(&RF2014ArinStage1Wnd::OnTicket5_983, this));
	ShowEventTicket983(false);
	for (int i = 0; i < TICKET_COUNT983; i++)
	{
		ShowTicketMark983(i, false);
		unsigned long typeId = s_ticketItems983[i];
		RFButton* pBtn = GetChild983<RFButton>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "btn_ticket", i + 1))));
		RFLabel* pLabel = GetChild983<RFLabel>(MultiByteToWide983(K2L_Compatibility(MakeStr("%s_%02d", "label_ticket", i + 1))));
		sItemInfo* pItem = Doc()->FindMyItemByTypeId983(typeId);
		if (pItem)
		{
			pBtn->EnableWindow(true);
			m_count983[i] = pItem->Common[0];
			SetTicketCount983(i, pItem->Common[0]);
		}
		else
		{
			pBtn->EnableWindow(false);
			SetTicketCount983(i, 0);
		}
	}
	UseOwnedTickets983();
}

// 005ddb40
RF2014ArinBirthdayDlg::RF2014ArinBirthdayDlg()
{
	m_step = 0;
	m_pDoc = NULL;
	s_pInstance = this;
}

// 005ddbe0
RF2014ArinBirthdayDlg::~RF2014ArinBirthdayDlg()
{
	s_pInstance = NULL;
}

// 005ddc70
RF2014ArinMissionInfoDlg::~RF2014ArinMissionInfoDlg()
{
	ms_pInstance = NULL;
}

// 005ddd10
RF2014ArinMapMissionInfoDlg::~RF2014ArinMapMissionInfoDlg()
{
	ms_pInstance = NULL;
}

// 005ddf50
RF2014ArinBirthdayDlg* RF2014ArinBirthdayDlg::Create983(RFWnd* parent)
{
	RF2014ArinBirthdayDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinBirthdayDlg>(L"event_arinbirth_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinBirthdayDlg>(L"event_arinbirth_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RF2014ArinBirthdayDlg>(L"event_arinbirth_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005de0c0: explicação das missões 1–4 (título e texto pelo GetGameString)
RF2014ArinMissionInfoDlg::RF2014ArinMissionInfoDlg()
{
	ms_pInstance = this;
	sArinMissionText983 info;
	info.title = GetGameString(0x30e15);
	info.text = GetGameString(0x30e16);
	m_texts983.push_back(info);
	info.title = GetGameString(0x30e17);
	info.text = GetGameString(0x30e18);
	m_texts983.push_back(info);
	info.title = GetGameString(0x30e19);
	info.text = GetGameString(0x30e1a);
	m_texts983.push_back(info);
	info.title = GetGameString(0x30e1b);
	info.text = GetGameString(0x30e1c);
	m_texts983.push_back(info);
}

// 005de330: explicação da 5ª missão (só o título)
RF2014ArinMapMissionInfoDlg::RF2014ArinMapMissionInfoDlg()
{
	ms_pInstance = this;
	sArinMissionText983 info;
	info.title = GetGameString(0x30e1d);
	info.text = "";
	m_texts983.push_back(info);
}

// 005de6b0
void RF2014ArinBirthdayDlg::InitTexts983()
{
	m_190.clear();
	m_190.push_back(GetGameString(0x30e08));
	m_190.push_back(GetGameString(0x30e09));
	m_190.push_back(GetGameString(0x30e0a));
	m_190.push_back(GetGameString(0x30e0b));
	m_190.push_back(GetGameString(0x30e0c));
	m_190.push_back(GetGameString(0x30e0d));
	m_190.push_back(GetGameString(0x30e0e));
	m_190.push_back(GetGameString(0x30e0f));
	m_190.push_back(GetGameString(0x30e10));
	m_190.push_back(GetGameString(0x30e11));
	m_190.push_back(GetGameString(0x30e12));
	m_190.push_back(GetGameString(0x30e13));
	m_190.push_back(GetGameString(0x30e14));
}

// 005debf0
RF2014ArinMissionInfoDlg* RF2014ArinMissionInfoDlg::Create983(RFWnd* parent)
{
	RF2014ArinMissionInfoDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinMissionInfoDlg>(L"event_arinbirth_popup_2_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinMissionInfoDlg>(L"event_arinbirth_popup_2_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RF2014ArinMissionInfoDlg>(L"event_arinbirth_popup_2_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005ded30
RF2014ArinMapMissionInfoDlg* RF2014ArinMapMissionInfoDlg::Create983(RFWnd* parent)
{
	RF2014ArinMapMissionInfoDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinMapMissionInfoDlg>(L"event_arinbirth_popup_3_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RF2014ArinMapMissionInfoDlg>(L"event_arinbirth_popup_3_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RF2014ArinMapMissionInfoDlg>(L"event_arinbirth_popup_3_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005dee70
void RF2014ArinBirthdayDlg::OnInitialUpdate()
{
	RegisterTo(L"btn_event_page_out_n", RFButton::EventClicked, Subscriber(&RF2014ArinBirthdayDlg::OnClose983, this));
	RegisterTo(L"btn_ok", RFButton::EventClicked, Subscriber(&RF2014ArinBirthdayDlg::OnOk983, this));
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RF2014ArinBirthdayDlg::OnKeyDown983, this));
	InitTexts983();
}

// 005df110
void RF2014ArinStage2Wnd::OpenMission983(int index)
{
	if (m_missions983.size() >= 5)
	{
		if (m_missions983[index].bDone != true && m_missions983[index].bRewarded != true)
		{
			RF2014ArinMissionInfoDlg* p = RF2014ArinMissionInfoDlg::Create983(NULL);
			if (p)
			{
				p->SetMission983(index, m_missions983[index]);
				p->CenterWindow(NULL);
				p->DoModal();
			}
		}
	}
}

// 005df180
void RF2014ArinStage2Wnd::OpenMapMission983(int index)
{
	if (m_missions983.size() >= 5)
	{
		RF2014ArinMapMissionInfoDlg* p = RF2014ArinMapMissionInfoDlg::Create983(NULL);
		if (p)
		{
			p->SetMission983(m_missions983[index]);
			p->CenterWindow(NULL);
			p->DoModal();
		}
	}
}

// 005df1e0
void RF2014ArinStage2Wnd::OnMission1_983(EventArgs& e)
{
	OpenMission983(0);
}

// 005df1f0
void RF2014ArinStage2Wnd::OnMission2_983(EventArgs& e)
{
	OpenMission983(1);
}

// 005df200
void RF2014ArinStage2Wnd::OnMission3_983(EventArgs& e)
{
	OpenMission983(2);
}

// 005df210
void RF2014ArinStage2Wnd::OnMission4_983(EventArgs& e)
{
	OpenMission983(3);
}

// 005df220
void RF2014ArinStage2Wnd::OnMission5_983(EventArgs& e)
{
	OpenMapMission983(4);
}

// 005df230
void RF2014ArinStage2Wnd::OnInitialUpdate()
{
	RegisterTo(L"btn_mission_01", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnMission1_983, this));
	RegisterTo(L"btn_mission_02", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnMission2_983, this));
	RegisterTo(L"btn_mission_03", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnMission3_983, this));
	RegisterTo(L"btn_mission_04", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnMission4_983, this));
	RegisterTo(L"btn_mission_05", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnMission5_983, this));
	RegisterTo(L"btn_reward_01", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnReward1_983, this));
	RegisterTo(L"btn_reward_02", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnReward2_983, this));
	RegisterTo(L"btn_reward_03", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnReward3_983, this));
	RegisterTo(L"btn_reward_04", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnReward4_983, this));
	RegisterTo(L"btn_reward_05", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnReward5_983, this));
	RegisterTo(L"btn_final", RFButton::EventClicked, Subscriber(&RF2014ArinStage2Wnd::OnFinal983, this));
}
