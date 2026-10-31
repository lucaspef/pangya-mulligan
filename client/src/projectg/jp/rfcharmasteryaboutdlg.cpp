// JP 983: unidade 005bd040–005c0260 — janela principal da maestria RF (RFCharMasteryMainDlg), a figura do personagem
// (RFCharImgWnd) e a ajuda em 9 páginas (RFCharMasteryAboutDlg). Começa nos acessores 005bd040…005bd080 logo depois da
// rfcharmasterycardselectdlg (B142; a B140 punha o começo em 005bd240 e o corte da F3.7.1 em 005bd290); 005bd090–005bd240 são
// cópias das funções de encaixe da rfcharmasterycardselectdlg (FALTA aqui: os nomes já são de lá). 005c0260+ é a
// rfcharmasterytiledlg. Sessão B140.
// Nomes meus (…983), menos as classes e os da RF; ver include/jp/refresh/rfcharmasteryaboutdlg.h.
#include "stdafx.h"
#include "refresh/rfcharmasteryaboutdlg.h"
#include "refresh/rfcharmasterytiledlg.h"
#include "refresh/rfcharmasterystatdetailwnd.h"
#include "refresh/rfcharmasterycardselectdlg.h"
#include "wlocalize.h"
#include "projectg.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "soundmanager.h"

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

#define DOCCHARID983() (Doc()->m_myInfo.userEquip.guidChar) // personagem em uso
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidChar) == 0xa2a); // A736

RFCharMasteryAboutDlg* RFCharMasteryAboutDlg::ms_pInstance;
RFWnd* RFCharMasteryAboutDlg::s_pParent983;
RFWnd* RFCharImgWnd::s_pParent983;
RFCharMasteryMainDlg* RFCharMasteryMainDlg::ms_pInstance;
RFWnd* RFCharMasteryMainDlg::s_pParent983;

// 005bd040
void RFCharMasteryAboutDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005bd050
void RFCharMasteryAboutDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005bd060
void RFCharMasteryMainDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005bd070
void RFCharMasteryMainDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005bd080
RFCharMasteryStatusDlg* RFCharMasteryMainDlg::GetStatusDlg983()
{
	return RFCharMasteryStatusDlg::ms_pInstance;
}

// 005bd240
RFCharMasteryTileDlg* RFCharMasteryMainDlg::GetTileDlg983()
{
	return RFCharMasteryTileDlg::ms_pInstance;
}

// 005bd250
void RFCharMasteryAboutDlg::OnConfirm983(EventArgs& e)
{
	g_audio->PlaySfx("ui_dialog_close", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005bd290: ESC fecha
void RFCharMasteryMainDlg::OnKey983(EventArgs& e)
{
	if (e.m_0c == VK_ESCAPE)
	{
		g_audio->PlaySfx("ui_dialog_close", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		Close(2);
		e.handled = true;
	}
}

// 005bd2f0
void RFCharMasteryMainDlg::OnExit983(EventArgs& e)
{
	g_audio->PlaySfx("ui_dialog_close", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005bd3d0
bool RFCharMasteryStatusDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005bd3e0
bool RFCharMasteryTileDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005bd3f0
RFCharMasteryAboutDlg::RFCharMasteryAboutDlg()
{
	ms_pInstance = this;
	memset(m_pPageBtn983, 0, sizeof(m_pPageBtn983));
	m_page983 = 1;
}

// 005bd4c0
RFCharMasteryAboutDlg::~RFCharMasteryAboutDlg()
{
	ms_pInstance = NULL;
}

// 005bd530
RFCharImgWnd::RFCharImgWnd()
{
	m_pImg983 = NULL;
}

// 005bd550
RFCharImgWnd::~RFCharImgWnd()
{
}

// 005bd560
RFCharMasteryMainDlg::RFCharMasteryMainDlg()
{
	ms_pInstance = this;
	memset(m_pTab983, 0, sizeof(m_pTab983));
	m_pCharImg983 = NULL;
	m_pStatus983 = NULL;
	m_pCard983 = NULL;
	m_pTile983 = NULL;
	m_pChar983 = NULL;
	m_tab983 = -1;
}

// 005bd620
RFCharMasteryMainDlg::~RFCharMasteryMainDlg()
{
	ms_pInstance = NULL;
}

// 005bdcc0: status (0) = janela de status + cartas; maestria (1) = encaixes
void RFCharMasteryMainDlg::SetTab983(int tab)
{
	if (m_tab983 != tab)
	{
		if (m_pTab983[TAB_STATUS983])
			m_pTab983[TAB_STATUS983]->SetCheck(tab == TAB_STATUS983);
		if (m_pTab983[TAB_MASTERY983])
			m_pTab983[TAB_MASTERY983]->SetCheck(tab != TAB_STATUS983);
		if (m_pStatus983)
			m_pStatus983->ShowWindow983(tab == TAB_STATUS983);
		if (m_pCard983)
			m_pCard983->ShowWindow983(tab == TAB_STATUS983);
		if (m_pTile983)
			m_pTile983->ShowWindow983(tab != TAB_STATUS983);
	}
}

// 005bdd50
void RFCharImgWnd::SetChar983(unsigned long typeId)
{
	if (m_pImg983 && typeId)
		m_pImg983->SetPicture(MultiByteToWide983(MakeStr("mastery_char_img_%d.png", typeId & 0x3ffffff)));
}

// 005bde40
void RFCharImgWnd::OnUpdate983(EventArgs& e)
{
}

// 005bde50
void RFCharMasteryMainDlg::OnStatusTab983(EventArgs& e)
{
	SetTab983(TAB_STATUS983);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005bde90
void RFCharMasteryMainDlg::OnMasteryTab983(EventArgs& e)
{
	SetTab983(TAB_MASTERY983);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005bded0
RFCharMasteryAboutDlg* RFCharMasteryAboutDlg::Create983(RFWnd* parent)
{
	RFCharMasteryAboutDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryAboutDlg>(L"kam_mastery_about_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryAboutDlg>(L"kam_mastery_about_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryAboutDlg>(L"kam_mastery_about_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005be010
void RFCharMasteryAboutDlg::ShowPage983(int page)
{
	for (int i = 0; i < PAGE_COUNT983; ++i)
	{
		if (m_pPageBtn983[i])
			m_pPageBtn983[i]->EnableWindow(true);
	}
	m_pPageBtn983[page - 1]->EnableWindow(false);
	m_page983 = page;
	RFPicture* pic = GetChild983<RFPicture>(L"pic_help_mastery_01");
	pic->SetPicture(MultiByteToWide983(MakeStr("char_master_help_%d.jpg", page)));
}

// 005be1a0
void RFCharMasteryAboutDlg::Prev983()
{
	--m_page983;
	if (m_page983 < 1)
		m_page983 = 1;
	ShowPage983(m_page983);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005be200
void RFCharMasteryAboutDlg::Next983()
{
	++m_page983;
	if (m_page983 > PAGE_COUNT983)
		m_page983 = PAGE_COUNT983;
	ShowPage983(m_page983);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005be260
RFCharImgWnd* RFCharImgWnd::Create983(RFWnd* parent)
{
	RFCharImgWnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharImgWnd>(L"kam_char_img_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharImgWnd>(L"kam_char_img_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharImgWnd>(L"kam_char_img_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005be3a0
void RFCharImgWnd::OnInitialUpdate()
{
	m_pImg983 = GetChild983<RFPicture>(L"char_img");
}

// 005be450
RFCharMasteryMainDlg* RFCharMasteryMainDlg::Create983(RFWnd* parent)
{
	RFCharMasteryMainDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryMainDlg>(L"kam_mastery_main_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryMainDlg>(L"kam_mastery_main_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryMainDlg>(L"kam_mastery_main_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005be590: a figura no lugar do marcador char_img_wnd
void RFCharMasteryMainDlg::InitCharImg983()
{
	RFWndEx* mark = GetChild983<RFWndEx>(L"char_img_wnd");
	if (mark)
	{
		m_pCharImg983 = RFCharImgWnd::Create983(this);
		if (m_pCharImg983)
		{
			m_pCharImg983->SetPos(mark->GetPos().x, mark->GetPos().y);
			m_pCharImg983->SetChar983(m_pChar983->tid);
			m_pCharImg983->ShowWindow983(true);
		}
		DeleteChild(mark);
	}
}

// 005be6c0: a janela de status no lugar do marcador status_wnd
void RFCharMasteryMainDlg::InitStatus983()
{
	if (RFCharMasteryStatusDlg::ms_pInstance == NULL)
	{
		RFWndEx* mark = GetChild983<RFWndEx>(L"status_wnd");
		if (mark)
		{
			m_pStatus983 = RFCharMasteryStatusDlg::Create983(this);
			if (m_pStatus983)
			{
				m_pStatus983->SetPos(mark->GetPos().x, mark->GetPos().y);
				m_pStatus983->ShowWindow983(true);
			}
			DeleteChild(mark);
		}
	}
}

// 005be7e0: a janela de cartas no lugar do marcador card_wnd
void RFCharMasteryMainDlg::InitCard983()
{
	if (RFCharMasteryCardDlg::ms_pInstance == NULL)
	{
		RFWndEx* mark = GetChild983<RFWndEx>(L"card_wnd");
		if (mark)
		{
			m_pCard983 = RFCharMasteryCardDlg::Create983(this);
			if (m_pCard983)
			{
				m_pCard983->SetPos(mark->GetPos().x, mark->GetPos().y);
				m_pCard983->SetChar983(m_pChar983->guid);
				m_pCard983->ShowWindow983(true);
			}
			DeleteChild(mark);
		}
	}
}

// 005be920: a janela de encaixes no lugar do marcador slot_mastery_wnd
void RFCharMasteryMainDlg::InitTile983()
{
	if (RFCharMasteryTileDlg::ms_pInstance == NULL)
	{
		RFWndEx* mark = GetChild983<RFWndEx>(L"slot_mastery_wnd");
		if (mark)
		{
			m_pTile983 = RFCharMasteryTileDlg::Create983(this);
			if (m_pTile983)
			{
				m_pTile983->Refresh983();
				m_pTile983->SetPos(mark->GetPos().x, mark->GetPos().y);
				m_pTile983->ShowWindow983(true);
			}
			DeleteChild(mark);
		}
	}
}

// 005bea50
void RFCharMasteryMainDlg::OnHelp983(EventArgs& e)
{
	RFCharMasteryAboutDlg* dlg = RFCharMasteryAboutDlg::Create983(this);
	if (dlg)
		dlg->DoModal();
}

// 005bea70
void RFCharMasteryAboutDlg::SetLabel983(RFWnd* wnd, const wchar_t* name, const std::wstring& text)
{
	RFLabelButton* label = wnd->GetChild983<RFLabelButton>(name);
	if (label)
		label->SetLabel(text.c_str());
}

// 005bebb0…005bec30
void RFCharMasteryAboutDlg::OnPage1_983(EventArgs& e) { ShowPage983(1); }
void RFCharMasteryAboutDlg::OnPage2_983(EventArgs& e) { ShowPage983(2); }
void RFCharMasteryAboutDlg::OnPage3_983(EventArgs& e) { ShowPage983(3); }
void RFCharMasteryAboutDlg::OnPage4_983(EventArgs& e) { ShowPage983(4); }
void RFCharMasteryAboutDlg::OnPage5_983(EventArgs& e) { ShowPage983(5); }
void RFCharMasteryAboutDlg::OnPage6_983(EventArgs& e) { ShowPage983(6); }
void RFCharMasteryAboutDlg::OnPage7_983(EventArgs& e) { ShowPage983(7); }
void RFCharMasteryAboutDlg::OnPage8_983(EventArgs& e) { ShowPage983(8); }
void RFCharMasteryAboutDlg::OnPage9_983(EventArgs& e) { ShowPage983(9); }

// 005bec40
void RFCharMasteryAboutDlg::OnForward983(EventArgs& e)
{
	Next983();
}

// 005bec50
void RFCharMasteryAboutDlg::OnBackward983(EventArgs& e)
{
	Prev983();
}

// 005bec60
void RFCharMasteryMainDlg::InitControls983()
{
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFCharMasteryMainDlg::OnKey983, this));
	RegisterTo(L"exit_button", RFButton::EventClicked, Subscriber(&RFCharMasteryMainDlg::OnExit983, this));
	RegisterTo(L"help_button", RFButton::EventClicked, Subscriber(&RFCharMasteryMainDlg::OnHelp983, this));
	RegisterTo(L"status_mgr_tap_check", RFCheck::EventChanged, Subscriber(&RFCharMasteryMainDlg::OnStatusTab983, this));
	RegisterTo(L"mastery_mgr_tap_check", RFCheck::EventChanged, Subscriber(&RFCharMasteryMainDlg::OnMasteryTab983, this));
	m_pTab983[TAB_STATUS983] = GetChild983<RFCheck>(L"status_mgr_tap_check");
	m_pTab983[TAB_MASTERY983] = GetChild983<RFCheck>(L"mastery_mgr_tap_check");
	m_pChar983 = &Doc()->m_charMap[DOCCHARID983()];
}

// 005bf150
void RFCharMasteryAboutDlg::OnInitialUpdate()
{
	RegisterTo(L"label_btn_about_01", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage1_983, this));
	RegisterTo(L"label_btn_about_02", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage2_983, this));
	RegisterTo(L"label_btn_about_03", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage3_983, this));
	RegisterTo(L"label_btn_about_04", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage4_983, this));
	RegisterTo(L"label_btn_about_05", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage5_983, this));
	RegisterTo(L"label_btn_about_06", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage6_983, this));
	RegisterTo(L"label_btn_about_07", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage7_983, this));
	RegisterTo(L"label_btn_about_08", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage8_983, this));
	RegisterTo(L"label_btn_about_09", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnPage9_983, this));
	RegisterTo(L"btn_forward", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnForward983, this));
	RegisterTo(L"btn_backward", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnBackward983, this));
	RegisterTo(L"btn_confirm", RFButton::EventClicked, Subscriber(&RFCharMasteryAboutDlg::OnConfirm983, this));
	for (int i = 0; i < PAGE_COUNT983; ++i)
	{
		m_pPageBtn983[i] = GetChild983<RFLabelButton>(MultiByteToWide983(MakeStr("label_btn_about_%02d", i + 1)));
	}
	// 1. 캐릭터 마스터리 (maestria do personagem)
	SetLabel983(this, L"label_btn_about_01", MultiByteToWide983(K2L_Compatibility(
		"1. \xc4\xb3\xb8\xaf\xc5\xcd \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae")));
	// 2. 스텟의 종류 (tipos de atributo)
	SetLabel983(this, L"label_btn_about_02", MultiByteToWide983(K2L_Compatibility(
		"2. \xbd\xba\xc5\xdd\xc0\xc7 \xc1\xbe\xb7\xf9")));
	// 3. 스텟 관리 (gerir atributos)
	SetLabel983(this, L"label_btn_about_03", MultiByteToWide983(K2L_Compatibility(
		"3. \xbd\xba\xc5\xdd \xb0\xfc\xb8\xae")));
	// 4. 스텟바 세부설명 (a barra de atributos em detalhe)
	SetLabel983(this, L"label_btn_about_04", MultiByteToWide983(K2L_Compatibility(
		"4. \xbd\xba\xc5\xdd\xb9\xd9 \xbc\xbc\xba\xce\xbc\xb3\xb8\xed")));
	// 5. 업/다운그레이드 (subir/descer)
	SetLabel983(this, L"label_btn_about_05", MultiByteToWide983(K2L_Compatibility(
		"5. \xbe\xf7/\xb4\xd9\xbf\xee\xb1\xd7\xb7\xb9\xc0\xcc\xb5\xe5")));
	// 6. 카드 장비/해제 (equipar/tirar carta)
	SetLabel983(this, L"label_btn_about_06", MultiByteToWide983(K2L_Compatibility(
		"6. \xc4\xab\xb5\xe5 \xc0\xe5\xba\xf1/\xc7\xd8\xc1\xa6")));
	// 7. 슬롯 관리 (gerir encaixes)
	SetLabel983(this, L"label_btn_about_07", MultiByteToWide983(K2L_Compatibility(
		"7. \xbd\xbd\xb7\xd4 \xb0\xfc\xb8\xae")));
	// 8. 보너스와 페널티 (bônus e penalidade)
	SetLabel983(this, L"label_btn_about_08", MultiByteToWide983(K2L_Compatibility(
		"8. \xba\xb8\xb3\xca\xbd\xba\xbf\xcd \xc6\xe4\xb3\xce\xc6\xbc")));
	// 9. 슬롯 확장 (abrir encaixes)
	SetLabel983(this, L"label_btn_about_09", MultiByteToWide983(K2L_Compatibility(
		"9. \xbd\xbd\xb7\xd4 \xc8\xae\xc0\xe5")));
	CenterWindow(NULL);
	ShowPage983(1);
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005c0160
void RFCharMasteryMainDlg::OnInitialUpdate()
{
	InitControls983();
	InitCharImg983();
	InitCard983();
	InitStatus983();
	InitTile983();
	RFPicture* title = GetChild983<RFPicture>(L"title_mastery");
	if (title)
		BringChildWndToTop(title);
	SetTab983(TAB_STATUS983);
}
