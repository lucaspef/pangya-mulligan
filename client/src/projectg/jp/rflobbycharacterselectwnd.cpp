// JP 983: unidade 005d1a70–005d5d20 — as janelas RF da seleção de personagem do lobby: RFLobbyCharacterSelectWnd (o botão
// "btn_character" da barra, "lobby_char_select_wnd.xml") e RFLobbyCharacterSelectAnimWnd (a gaveta com um botão por
// personagem, "lobby_char_select_anim_wnd.xml"), e no fim o HandleMsg983 do RFLobbyDesktop (slot 35, 005d5520: os avisos do
// jogo à área de trabalho do lobby). 005d1a70–005d2220 são cópias fora de linha das funções embutidas e moldes (functores,
// vector). Sessão B135. Nomes meus (…983), menos as classes e os da RF.
#include "stdafx.h"
#include "refresh/rflobbycharacterselectwnd.h"
#include "refresh/rflobbydesktop.h"
#include "lobbyjp.h"
#include "projectg.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "taskform.h"
#include "actor.h"
#include "packet.h"
#include "commonutil.h"
#include "createcharacteroption.h"
#include "avatar_avatar.h"

extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)
void __fastcall SendLobbyChar983(unsigned long id); // 00566f30 (lobbyplayer.cpp)
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // A805 temporario
struct sAvatarMotionJp983; // ntAvatarManager (visão em cardcomposedlg.h): +0xb0 = bool
extern sAvatarMotionJp983* g_pAvatarMotion983; // 00e73e80

#define DOCCHARID983() (::Doc()->m_myInfo.userEquip.guidChar) // personagem em uso
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidChar) == 0xa2a); // A736

RFLobbyCharacterSelectWnd* RFLobbyCharacterSelectWnd::ms_pInstance;
RFWnd* RFLobbyCharacterSelectWnd::s_pParent983;
RFLobbyCharacterSelectAnimWnd* RFLobbyCharacterSelectAnimWnd::ms_pInstance;
RFWnd* RFLobbyCharacterSelectAnimWnd::s_pParent983;

// ---------------------------------------------------------------------------------------------------------------------
// construtores e destrutores

// 005d2220
RFLobbyCharacterSelectWnd::RFLobbyCharacterSelectWnd()
	: m_curType983(0xf), m_charType983(0xf), m_bChanged983(false)
{
	ms_pInstance = this;
}

// 005d22b0
RFLobbyCharacterSelectWnd::~RFLobbyCharacterSelectWnd()
{
	ms_pInstance = NULL;
}

// 005d2670
RFLobbyCharacterSelectAnimWnd::RFLobbyCharacterSelectAnimWnd()
	: m_bOpen(false), m_bAnimating(false), m_prevType983(0xf), m_curButton983(0xf)
{
	ms_pInstance = this;
}

// 005d2710
RFLobbyCharacterSelectAnimWnd::~RFLobbyCharacterSelectAnimWnd()
{
	ms_pInstance = NULL;
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyCharacterSelectWnd

// 005d2980
RFLobbyCharacterSelectWnd* RFLobbyCharacterSelectWnd::Create983(RFWnd* parent)
{
	RFLobbyCharacterSelectWnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyCharacterSelectWnd>(L"lobby_char_select_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyCharacterSelectWnd>(L"lobby_char_select_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFLobbyCharacterSelectWnd>(L"lobby_char_select_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d2ac0
void RFLobbyCharacterSelectWnd::SetChar983(int type, bool bSend)
{
	if (type < 0xf && RFLobbyDesktop::Instance983())
	{
		unsigned long id = DOCLOBBYJP()->FindCharIdByTid983(type | 0x4000000);
		sCharacterInfo* pInfo = Doc()->FindChar983(id);
		jp983_rastro("A805 barra SetChar type=%d send=%d atual=%d id=%08x info=%p modal=%d", type, bSend, m_charType983, id,
			pInfo, g_lobbyModal983);
		if (pInfo)
		{
			if (m_charType983 != type && !m_names[type].empty())
			{
				RFButton* button = GetChild983<RFButton>(L"btn_character");
				if (button)
				{
					std::wstring normal = MultiByteToWide983(MakeStr("char_selcet_btn_main_%s_n.tga", m_names[type].c_str()));
					std::wstring over = MultiByteToWide983(MakeStr("char_selcet_btn_main_%s_o.tga", m_names[type].c_str()));
					std::wstring down = MultiByteToWide983(MakeStr("char_selcet_btn_main_%s_p.tga", m_names[type].c_str()));
					button->ChangeNormalButtonImage(normal.c_str());
					button->ChangeOverButtonImage(over.c_str());
					button->ChangeDownButtonImage(down.c_str());
					button->ChangeDisableButtonImage(normal.c_str());
				}
			}
			if (bSend)
			{
				if (g_lobbyModal983 == 1)
				{
					WSendPacket packet(0xc);
					packet.Encode1(4);
					packet.Encode4(id);
					packet.Send((eSendTo)0);
				}
				else if (RFLobbyDesktop::Instance983()->IsGroupA983())
				{
					AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x187, (int)pInfo, 0, 0, 0, 0);
					SendLobbyChar983(id);
				}
				else
				{
					AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x192, id, (int)pInfo, 0, 0, 0);
				}
			}
			else if (RFLobbyDesktop::Instance983()->IsGroupA983())
				DOCCHARID983() = id;
			m_charType983 = type;
			m_bChanged983 = m_curType983 != type;
			Doc()->BuildMyPartTidList(*pInfo);
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyCharacterSelectAnimWnd

// 005d2ef0
RFLobbyCharacterSelectAnimWnd* RFLobbyCharacterSelectAnimWnd::Create983(RFWnd* parent)
{
	RFLobbyCharacterSelectAnimWnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyCharacterSelectAnimWnd>(L"lobby_char_select_anim_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyCharacterSelectAnimWnd>(L"lobby_char_select_anim_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFLobbyCharacterSelectAnimWnd>(L"lobby_char_select_anim_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d3030
void RFLobbyCharacterSelectAnimWnd::SetButton983(int type)
{
	if (m_curButton983 != type)
	{
		if (m_curButton983 != 0xf)
		{
			RFButton* button = dynamic_cast<RFButton*>(GetChild(m_btnNames[m_curButton983].c_str()));
			if (button)
				button->ResetState983();
		}
		if (RFLobbyCharacterSelectWnd::ms_pInstance)
			RFLobbyCharacterSelectWnd::ms_pInstance->SetChar983(type, false);
		m_curButton983 = type;
	}
}

// 005d3170
void RFLobbyCharacterSelectAnimWnd::OnNuriClick983(EventArgs& e)
{
	SetButton983(0xb);
}

// 005d3180
void RFLobbyCharacterSelectAnimWnd::OnNuriOldClick983(EventArgs& e)
{
	SetButton983(0);
}

// 005d3190
void RFLobbyCharacterSelectAnimWnd::OnHanaClick983(EventArgs& e)
{
	SetButton983(0xc);
}

// 005d31a0
void RFLobbyCharacterSelectAnimWnd::OnHanaOldClick983(EventArgs& e)
{
	SetButton983(1);
}

// 005d31b0
void RFLobbyCharacterSelectAnimWnd::OnCeciliaOldClick983(EventArgs& e)
{
	SetButton983(3);
}

// 005d31c0
void RFLobbyCharacterSelectAnimWnd::OnCeciliaClick983(EventArgs& e)
{
	SetButton983(0xe);
}

// 005d31d0
void RFLobbyCharacterSelectAnimWnd::OnArinClick983(EventArgs& e)
{
	SetButton983(6);
}

// 005d31e0
void RFLobbyCharacterSelectAnimWnd::OnLuciaClick983(EventArgs& e)
{
	SetButton983(8);
}

// 005d31f0
void RFLobbyCharacterSelectAnimWnd::OnNellClick983(EventArgs& e)
{
	SetButton983(9);
}

// 005d3200
void RFLobbyCharacterSelectAnimWnd::OnKoohClick983(EventArgs& e)
{
	SetButton983(5);
}

// 005d3210
void RFLobbyCharacterSelectAnimWnd::OnSpikaClick983(EventArgs& e)
{
	SetButton983(0xa);
}

// 005d3220
void RFLobbyCharacterSelectAnimWnd::OnKazClick983(EventArgs& e)
{
	SetButton983(7);
}

// 005d3230
void RFLobbyCharacterSelectAnimWnd::OnMaxClick983(EventArgs& e)
{
	SetButton983(4);
}

// 005d3240
void RFLobbyCharacterSelectAnimWnd::OnAzerClick983(EventArgs& e)
{
	SetButton983(2);
}

// 005d3250: ativa/desativa a filha pelo nome
void EnableChild983(RFWnd* wnd, const wchar_t* name, bool enable)
{
	RFWnd* child = wnd->GetChild(name);
	if (child)
		child->EnableWindow(enable);
}

// 005d3320
void RFLobbyCharacterSelectAnimWnd::UpdateButtons983()
{
	for (int i = 0; i < 15; i++)
		EnableChild983(this, m_btnNames[i].c_str(), false);
	for (unsigned int i = 0; i < m_owned983.size(); i++)
		EnableChild983(this, m_btnNames[m_owned983[i]].c_str(), true);
}

// 005d33b0: durante a animação os botões ficam desativados; no fim, o botão marcado fica apertado e a gaveta para no
// "_Normal_" (aberta) ou no "_Init_" (fechada)
void RFLobbyCharacterSelectAnimWnd::OnUpdate983(EventArgs& e)
{
	if (IsAnimationPlaying())
	{
		for (int i = 0; i < 15; i++)
			EnableChild983(this, m_btnNames[i].c_str(), false);
	}
	else
	{
		RFButton* button = dynamic_cast<RFButton*>(GetChild(m_btnNames[m_curButton983].c_str()));
		if (button)
		{
			button->SetState(2);
		}
		if (m_bAnimating)
		{
			if (!m_bOpen)
				s_pParent983->SetHitTestVisible983(false);
			PlayAnimation(m_bOpen ? L"_Normal_" : L"_Init_");
			StopAnimation();
			m_bAnimating = false;
			UpdateButtons983();
		}
	}
}

// 005d35b0
void RFLobbyCharacterSelectAnimWnd::Select983(int what, bool bRestore)
{
	if (!m_bAnimating)
	{
		switch (what)
		{
		case 0:
			m_bOpen = !m_bOpen;
			break;
		case 1:
			m_bOpen = true;
			break;
		case 2:
			m_bOpen = false;
			break;
		}
		PlayAnimation(m_bOpen ? L"_Opening_" : L"_Closing_");
		UpdateButtons983();
		if (bRestore)
			m_curButton983 = m_prevType983;
		if (m_bOpen)
		{
			unsigned long type = 0;
			sCharacterInfo* pInfo = Doc()->FindChar983(0xffffffff);
			if (pInfo)
				type = pInfo->tid & 0x3ffffff;
			m_prevType983 = type;
			s_pParent983->SetHitTestVisible983(true);
		}
		else if (RFLobbyCharacterSelectWnd::ms_pInstance)
			RFLobbyCharacterSelectWnd::ms_pInstance->SetChar983(m_curButton983, m_curButton983 != m_prevType983);
	}
}

// 005d3720: "btn_closer_left"/"btn_closer_right" fecham e voltam ao personagem de antes
void RFLobbyCharacterSelectAnimWnd::OnCloserClick983(EventArgs& e)
{
	if (!m_bAnimating)
		Select983(2, true);
}

// 005d3740
void RFLobbyCharacterSelectAnimWnd::OnNuriDblClick983(EventArgs& e)
{
	OnNuriClick983(e);
	Select983(2, false);
}

// 005d3760
void RFLobbyCharacterSelectAnimWnd::OnNuriOldDblClick983(EventArgs& e)
{
	OnNuriOldClick983(e);
	Select983(2, false);
}

// 005d3780
void RFLobbyCharacterSelectAnimWnd::OnHanaDblClick983(EventArgs& e)
{
	OnHanaClick983(e);
	Select983(2, false);
}

// 005d37a0
void RFLobbyCharacterSelectAnimWnd::OnHanaOldDblClick983(EventArgs& e)
{
	OnHanaOldClick983(e);
	Select983(2, false);
}

// 005d37c0
void RFLobbyCharacterSelectAnimWnd::OnCeciliaOldDblClick983(EventArgs& e)
{
	OnCeciliaOldClick983(e);
	Select983(2, false);
}

// 005d37e0
void RFLobbyCharacterSelectAnimWnd::OnCeciliaDblClick983(EventArgs& e)
{
	OnCeciliaClick983(e);
	Select983(2, false);
}

// 005d3800
void RFLobbyCharacterSelectAnimWnd::OnArinDblClick983(EventArgs& e)
{
	OnArinClick983(e);
	Select983(2, false);
}

// 005d3820
void RFLobbyCharacterSelectAnimWnd::OnLuciaDblClick983(EventArgs& e)
{
	OnLuciaClick983(e);
	Select983(2, false);
}

// 005d3840
void RFLobbyCharacterSelectAnimWnd::OnNellDblClick983(EventArgs& e)
{
	OnNellClick983(e);
	Select983(2, false);
}

// 005d3860
void RFLobbyCharacterSelectAnimWnd::OnKoohDblClick983(EventArgs& e)
{
	OnKoohClick983(e);
	Select983(2, false);
}

// 005d3880
void RFLobbyCharacterSelectAnimWnd::OnSpikaDblClick983(EventArgs& e)
{
	OnSpikaClick983(e);
	Select983(2, false);
}

// 005d38a0
void RFLobbyCharacterSelectAnimWnd::OnKazDblClick983(EventArgs& e)
{
	OnKazClick983(e);
	Select983(2, false);
}

// 005d38c0
void RFLobbyCharacterSelectAnimWnd::OnMaxDblClick983(EventArgs& e)
{
	OnMaxClick983(e);
	Select983(2, false);
}

// 005d38e0
void RFLobbyCharacterSelectAnimWnd::OnAzerDblClick983(EventArgs& e)
{
	OnAzerClick983(e);
	Select983(2, false);
}

// 005d3900
void RFLobbyCharacterSelectWnd::OnCharacterClick983(EventArgs& e)
{
	RFLobbyCharacterSelectAnimWnd* anim = RFLobbyCharacterSelectAnimWnd::ms_pInstance;
	if (anim && !anim->m_bAnimating)
		anim->Select983(0, false);
}

// 005d3920
void RFLobbyCharacterSelectAnimWnd::SetChar983(int type)
{
	m_owned983.clear();
	for (std::map<unsigned int, sCharacterInfo>::iterator it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
	{
		unsigned long owned = it->second.tid & 0x3ffffff;
		m_owned983.push_back(owned);
	}
	PlayAnimation(L"_Init_");
	UpdateButtons983();
	if (type != 0xf)
	{
		m_curButton983 = type;
		m_prevType983 = type;
	}
	if (m_bOpen)
		Select983(2, false);
}

// 005d3b90
void RFLobbyCharacterSelectWnd::Refresh983()
{
	unsigned long type = 0;
	sCharacterInfo* pInfo = Doc()->FindChar983(0xffffffff);
	if (pInfo)
		type = pInfo->tid & 0x3ffffff;
	m_curType983 = type;
	SetChar983(type, false);
	CenterWindow_Horizontal(NULL);
	if (RFLobbyCharacterSelectAnimWnd::ms_pInstance)
		RFLobbyCharacterSelectAnimWnd::ms_pInstance->SetChar983(m_charType983);
}

// 005d3bf0
void RFLobbyCharacterSelectAnimWnd::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventUpdate, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnUpdate983, this));
	RegisterTo(L"", RFWnd::EventAnimationPlay983, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnAnimationPlay983, this));
	RegisterTo(L"btn_closer_left", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnCloserClick983, this));
	RegisterTo(L"btn_closer_right", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnCloserClick983, this));
	RegisterTo(L"btn_char_nuri", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnNuriClick983, this));
	RegisterTo(L"btn_char_nuri_old", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnNuriOldClick983, this));
	RegisterTo(L"btn_char_hana", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnHanaClick983, this));
	RegisterTo(L"btn_char_hana_old", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnHanaOldClick983, this));
	RegisterTo(L"btn_char_cecilia_old", RFButton::EventClicked,
		Subscriber(&RFLobbyCharacterSelectAnimWnd::OnCeciliaOldClick983, this));
	RegisterTo(L"btn_char_cecilia", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnCeciliaClick983, this));
	RegisterTo(L"btn_char_arin", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnArinClick983, this));
	RegisterTo(L"btn_char_lucia", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnLuciaClick983, this));
	RegisterTo(L"btn_char_nell", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnNellClick983, this));
	RegisterTo(L"btn_char_kooh", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnKoohClick983, this));
	RegisterTo(L"btn_char_spika", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnSpikaClick983, this));
	RegisterTo(L"btn_char_kaz", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnKazClick983, this));
	RegisterTo(L"btn_char_max", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnMaxClick983, this));
	RegisterTo(L"btn_char_azer", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnAzerClick983, this));
	RegisterTo(L"btn_char_nuri", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnNuriDblClick983, this));
	RegisterTo(L"btn_char_nuri_old", RFWnd::EventLButtonDblClick,
		Subscriber(&RFLobbyCharacterSelectAnimWnd::OnNuriOldDblClick983, this));
	RegisterTo(L"btn_char_hana", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnHanaDblClick983, this));
	RegisterTo(L"btn_char_hana_old", RFWnd::EventLButtonDblClick,
		Subscriber(&RFLobbyCharacterSelectAnimWnd::OnHanaOldDblClick983, this));
	RegisterTo(L"btn_char_cecilia_old", RFWnd::EventLButtonDblClick,
		Subscriber(&RFLobbyCharacterSelectAnimWnd::OnCeciliaOldDblClick983, this));
	RegisterTo(L"btn_char_cecilia", RFWnd::EventLButtonDblClick,
		Subscriber(&RFLobbyCharacterSelectAnimWnd::OnCeciliaDblClick983, this));
	RegisterTo(L"btn_char_arin", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnArinDblClick983, this));
	RegisterTo(L"btn_char_lucia", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnLuciaDblClick983, this));
	RegisterTo(L"btn_char_nell", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnNellDblClick983, this));
	RegisterTo(L"btn_char_kooh", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnKoohDblClick983, this));
	RegisterTo(L"btn_char_spika", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnSpikaDblClick983, this));
	RegisterTo(L"btn_char_kaz", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnKazDblClick983, this));
	RegisterTo(L"btn_char_max", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnMaxDblClick983, this));
	RegisterTo(L"btn_char_azer", RFWnd::EventLButtonDblClick, Subscriber(&RFLobbyCharacterSelectAnimWnd::OnAzerDblClick983, this));
	if (RFLobbyCharacterSelectWnd::ms_pInstance)
	{
		for (int i = 0; i < 15; i++)
			m_btnNames[i] = MultiByteToWide983(
				std::string(MakeStr("btn_char_%s", RFLobbyCharacterSelectWnd::ms_pInstance->GetCharName983(i))));
	}
	CenterWindow_Horizontal(NULL);
	unsigned long type = 0;
	sCharacterInfo* pInfo = Doc()->FindChar983(0xffffffff);
	if (pInfo)
		type = pInfo->tid & 0x3ffffff;
	SetChar983(type);
	SetPreDraw(true);
}

// 005d5240
void RFLobbyCharacterSelectWnd::OnInitialUpdate()
{
	RegisterTo(L"btn_character", RFButton::EventClicked, Subscriber(&RFLobbyCharacterSelectWnd::OnCharacterClick983, this));
	RegisterTo(L"btn_character", RFWnd::EventMouseEnter, Subscriber(&RFLobbyCharacterSelectWnd::OnCharacterEnter983, this));
	RegisterTo(L"btn_character", RFWnd::EventMouseLeave, Subscriber(&RFLobbyCharacterSelectWnd::OnCharacterLeave983, this));
	CreateCharacterOption::Instance()->GetCharFileNames983(m_names);
	Refresh983();
	SetPreDraw(true);
}

// ---------------------------------------------------------------------------------------------------------------------
// RFLobbyDesktop::HandleMsg983 (slot 35): avisos do jogo à área de trabalho do lobby

// 005d5520
void RFLobbyDesktop::HandleMsg983(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 1:
		m_242 = true;
		break;
	case 2:
		m_242 = false;
		break;
	case 0x53: // um modal da RF abriu
		m_242 = true;
		RFSceneManager::Instance()->SetRFMouseFocus983(true);
		m_189 = false;
		break;
	case 0x54: // fechou
		m_242 = false;
		if (!RFSceneManager::GetManager983()->IsModalExist())
			RFSceneManager::Instance()->SetRFMouseFocus983(false);
		m_189 = true;
		break;
	case 0x56: // carregando
		if (IsWindowVisible(false) && !RFLoadingProgressDlg::ms_pInstance)
		{
			RFLoadingProgressDlg* loading = RFLoadingProgressDlg::Create983(this);
			if (loading)
				loading->Start983();
		}
		break;
	case 0x57: // carregou
		if (RFLoadingProgressDlg::ms_pInstance)
			RFLoadingProgressDlg::ms_pInstance->Close(1);
		AfxGetTask()->GetActor("LobbyBg") << MsgObject(NULL, 0x182, 0, 0, 0, 0, 0);
		break;
	case 0x58:
		// A299: 00e73e80 = ntAvatarManager::m_pInstance (g_pAvatarMotion983 era só declaração = nulo no exe)
		if (ntAvatarManager::m_pInstance && !RFSceneManager::GetManager983()->IsModalExist())
			ntAvatarManager::m_pInstance->m_bActive = true;
		break;
	case 0x55: // abre a tela pelo nome (loja / my room / tutorial)
		if (msg.param1)
		{
			std::string name((const char*)msg.param1);
			EventArgs e;
			switch (FindScreen983(name))
			{
			case 2:
			case 3:
			case 4:
				OnShopClick983(e);
				break;
			case 5:
			case 6:
			case 7:
				OnMyRoomClick983(e);
				break;
			case 8:
				HandleMsg983(MsgObject(NULL, 0x88, (int)"CTutorialWidget", 0, 0, 0, 0));
				break;
			}
		}
		break;
	case 0x88: // a widget `param1` abriu por cima do lobby ("RFLobbyDesktop" = voltou)
	{
		std::string name((const char*)msg.param1);
		if (name == "RFLobbyDesktop")
		{
			if (m_224.empty())
			{
				SetScreen983(1);
				UpdateLobbyActors983();
			}
		}
		else
		{
			m_224 = name;
			FrTaskForm* form = AfxGetTask()->GetWidgetForm983();
			if (form)
				form->Close(true);
			if (name == "CTutorialWidget")
			{
				SetScreen983(8);
				HandleMsg983(MsgObject(NULL, 0x180, 0, 0, 0, 0, 0));
			}
		}
		break;
	}
	case 0x59:
		SetScreen983(msg.param1);
		break;
	case 0xca:
	case 0xcb:
		OnSpcardMsg983(msg.message);
		break;
	case 0xcc:
		if (msg.param2)
		{
			ShowMessengerBubble983(true, std::string((const char*)msg.param1));
			m_246 = true;
		}
		else if (m_246)
			m_246 = false;
		break;
	case 0xcd:
	case 0xce:
	case 0xcf:
	case 0xd1:
		OnMessengerMsg983(msg.message);
		break;
	case 0x106:
		ResetCharSelect983();
		break;
	case 0x180: // a widget fechou: volta à tela de antes e acorda os atores do lobby
		switch (m_state)
		{
		case 2:
		case 5:
			if (!m_224.empty())
				AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x19a, 0, 0, 0, 0, 0);
		case 3:
		case 4:
		case 6:
		case 7:
			if (!m_bLocked)
				break;
		case 8:
			if (!m_224.empty())
			{
				if (CTaskManager::Instance())
					CTaskManager::Instance()->OpenWidget983(m_224.c_str(), "TOPPAGE");
				m_224.clear();
			}
			break;
		}
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x18e, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("LobbyCaddie") << MsgObject(NULL, 0x18e, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("LobbyMascot") << MsgObject(NULL, 0x18e, 0, 0, 0, 0, 0);
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x1a0, 0, 0, 0, 0, 0);
		m_bLocked = false;
		break;
	}
}
