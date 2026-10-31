// JP 983: unidade 005d7910–005d8160 — RFServerNoticeDlg, o aviso do servidor no lobby RF ("server_notice_dialog.xml"; fecha
// sozinho depois de 1 s). 005d8160–005d8700 são as cópias fora de linha da unidade seguinte (rf2014arin*). Sessão B136.
// Nomes meus (…983), menos a classe e os da RF.
#include "stdafx.h"
#include "refresh/rfservernoticedlg.h"

extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

RFServerNoticeDlg* RFServerNoticeDlg::ms_pInstance;
RFWnd* RFServerNoticeDlg::s_pParent983;

// 005d7910
RFServerNoticeDlg::RFServerNoticeDlg()
{
	ms_pInstance = this;
	m_time983 = 1.0f;
}

// 005d7930
RFServerNoticeDlg::~RFServerNoticeDlg()
{
	ms_pInstance = NULL;
}

// 005d7bd0
RFServerNoticeDlg* RFServerNoticeDlg::Create983(RFWnd* parent)
{
	RFServerNoticeDlg* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFServerNoticeDlg>(L"server_notice_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFServerNoticeDlg>(L"server_notice_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFServerNoticeDlg>(L"server_notice_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d7d10
void RFServerNoticeDlg::OnUpdate983(EventArgs& e)
{
	if (m_time983 > 0.0f)
	{
		m_time983 -= e.m_time983;
		if (m_time983 <= 0.0f)
		{
			m_time983 = 0.0f;
			PlayAnimation(L"_Closing_");
		}
	}
	if (m_time983 == 0.0f && IsWindowVisible(false) && !IsAnimationPlaying())
		Close(1);
}

// 005d7e30
void RFServerNoticeDlg::SetLabel983(RFWnd* wnd, const wchar_t* name, const std::wstring& text)
{
	RFLabel* label = wnd->GetChild983<RFLabel>(name);
	if (label)
		label->SetLabel(text.c_str());
}

// 005d7f70
void RFServerNoticeDlg::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventUpdate, Subscriber(&RFServerNoticeDlg::OnUpdate983, this));
}

// 005d8070
void RFServerNoticeDlg::SetNotice983(const std::string& text)
{
	SetLabel983(this, L"label_server_name", MultiByteToWide983(text.c_str()));
	CenterWindow(NULL);
}
