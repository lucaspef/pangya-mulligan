// JP 983: unidade 005cfe00–005d0f00 — RFCommonEventAboutDlg, o "sobre o evento" da RF ("common_event_about_dialog.xml"; imagem
// com rolagem, ESC fecha). 005d0f00+ é a unidade seguinte (acessores da instância 00e7820c). Sessão B138.
// Nomes meus (…983), menos a classe e os da RF.
#include "stdafx.h"
#include "refresh/rfcommoneventaboutdlg.h"
#include "wlocalize.h"

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

RFCommonEventAboutDlg* RFCommonEventAboutDlg::ms_pInstance;
RFWnd* RFCommonEventAboutDlg::s_pParent983;

// 005cfe00
void RFCommonEventAboutDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005cfe10
void RFCommonEventAboutDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005cfe20
void RFCommonEventAboutDlg::OnUpdate983(EventArgs& e)
{
}

// 005cfe30
void RFCommonEventAboutDlg::OnKeyDown983(EventArgs& e)
{
	if (e.m_0c == VK_ESCAPE)
	{
		Close(2);
		e.handled = true;
	}
}

// 005cfe80
RFCommonEventAboutDlg::RFCommonEventAboutDlg()
{
	ms_pInstance = this;
	m_imageSize983 = RFSize();
	m_bScroll983 = false;
}

// 005cff50
RFCommonEventAboutDlg::~RFCommonEventAboutDlg()
{
	ms_pInstance = NULL;
}

// 005d0230
RFCommonEventAboutDlg* RFCommonEventAboutDlg::Create983(RFWnd* parent)
{
	RFCommonEventAboutDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCommonEventAboutDlg>(L"common_event_about_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCommonEventAboutDlg>(L"common_event_about_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCommonEventAboutDlg>(L"common_event_about_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d0370
void RFCommonEventAboutDlg::OnMouseWheel983(EventArgs& e)
{
	if (m_bScroll983)
	{
		RFScrollBar* bar = GetChild983<RFScrollBar>(L"vscrollBar_event");
		if (bar)
		{
			bar->GetScrollPos();
			if (((MouseWheelEventArgs983&)e).m_delta983 < 0.0f)
				bar->Next();
			else
				bar->Prev();
		}
	}
}

// 005d0470
void RFCommonEventAboutDlg::OnScroll983(EventArgs& e)
{
	if (m_bScroll983)
	{
		RFScrollBar* bar = GetChild983<RFScrollBar>(L"vscrollBar_event");
		RFPicture* image = GetChild983<RFPicture>(L"pic_event_bg");
		if (bar)
		{
			float pos = (float)e.m_0c;
			if (image)
			{
				RFVector2 at = image->GetPos();
				float ratio = pos / 100.0 * -1.0;
				float y = ratio * m_imageSize983.height;
				image->SetPos(at.x, y);
			}
		}
	}
}

// 005d05f0
void RFCommonEventAboutDlg::SetLabel983(RFWnd* wnd, const wchar_t* name, const std::wstring& text)
{
	RFLabel* label = wnd->GetChild983<RFLabel>(name);
	if (label)
		label->SetLabel(text.c_str());
}

// 005d0730
void RFCommonEventAboutDlg::ShowChild983(RFWnd* wnd, const wchar_t* name, bool show)
{
	RFWnd* child = wnd->GetChild(name);
	if (child)
		child->ShowWindow(show);
}

// 005d0800
void RFCommonEventAboutDlg::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventUpdate, Subscriber(&RFCommonEventAboutDlg::OnUpdate983, this));
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFCommonEventAboutDlg::OnKeyDown983, this));
	RegisterTo(L"", RFWnd::EventMouseWheel, Subscriber(&RFCommonEventAboutDlg::OnMouseWheel983, this));
	RegisterTo(L"vscrollBar_event", RFScrollBar::EventPosChanged, Subscriber(&RFCommonEventAboutDlg::OnScroll983, this));
	RFScrollBar* bar = GetChild983<RFScrollBar>(L"vscrollBar_event");
	if (bar)
	{
		bar->SetScrollRange(0, 100);
		bar->SetScrollSize(10);
		bar->EnableSmoothScroll983(true);
		bar->ShowWindow(false);
	}
	// "ESC키를 사용하여 창을 닫을 수 있습니다." (ESC fecha a janela)
	SetLabel983(this, L"label_event_help", MultiByteToWide983(K2L_Compatibility(
		"ESC\xc5\xb0\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbf\xa9 \xc3\xa2\xc0\xbb \xb4\xdd\xc0\xbb \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.")));
}

// 005d0c60
void RFCommonEventAboutDlg::SetImage983(const char* name)
{
	RFPicture* image = GetChild983<RFPicture>(L"pic_event_bg");
	if (image)
	{
		image->SetPicture(MultiByteToWide983(name).c_str());
		image->FitToContent();
		m_imageSize983 = image->GetSize();
	}
	RFWnd* frame = GetChild(L"wnd_event_bg");
	if (frame)
	{
		RFSize size = frame->GetSize();
		m_bScroll983 = m_imageSize983.height > size.height;
		if (m_bScroll983)
			m_imageSize983.height -= size.height;
	}
	ShowChild983(this, L"vscrollBar_event", m_bScroll983);
}
