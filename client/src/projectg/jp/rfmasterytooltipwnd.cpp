// JP 983: unidade 005d71e0–005d7910 — RFMasteryToolTipWnd, a dica de texto das janelas de maestria RF
// ("kam_mastery_tooltip.xml"). Sessão B136. Nomes meus (…983), menos a classe e os da RF.
#include "stdafx.h"
#include "refresh/rfmasterytooltipwnd.h"
#include "wview.h"

extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern WView* g_view; // 00f02458

RFWnd* RFMasteryToolTipWnd::s_pParent983;

// 005d71e0
RFMasteryToolTipWnd::RFMasteryToolTipWnd()
	: m_pText(NULL)
{
}

// 005d7200
RFMasteryToolTipWnd::~RFMasteryToolTipWnd()
{
}

// 005d7210
void RFMasteryToolTipWnd::SetPos983(float x, float y)
{
	if (x < 0.0f)
		x = 0.0f;
	if (y < 0.0f)
		y = 0.0f;
	if (GetSize().width + x > g_view->GetWidth())
		x = g_view->GetWidth() - GetSize().width;
	if (GetSize().height + y > g_view->GetHeight())
		y = g_view->GetHeight() - GetSize().height;
	SetScreenPos(x, y);
}

// 005d7300
void RFMasteryToolTipWnd::SetTextStyle983(int value)
{
	if (m_pText)
		m_pText->SetFontColor983(value);
}

// 005d7490
void RFMasteryToolTipWnd::FitSize983()
{
	if (m_pText)
	{
		std::wstring text = m_pText->GetText();
		size_t pos = text.find(L'\n');
		int maxLen = pos == std::wstring::npos ? text.size() : pos;
		int lines = 1;
		while (pos != std::wstring::npos)
		{
			lines++;
			size_t next = text.find(L'\n', pos + 1);
			int len = (next == std::wstring::npos ? text.size() : next) - pos;
			if (len > maxLen)
				maxLen = len;
			pos = next;
		}
		float padW = m_pText->GetPaddingWidth983();
		float w = padW + maxLen * 12.0;
		float padH = m_pText->GetPaddingHeight983();
		float h = padH + lines * 19.0;
		m_pText->SetSize(RFSize(w > 300.0f ? 300.0f : w, h));
	}
}

// 005d76c0
RFMasteryToolTipWnd* RFMasteryToolTipWnd::Create983(RFWnd* parent)
{
	RFMasteryToolTipWnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryToolTipWnd>(L"kam_mastery_tooltip.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFMasteryToolTipWnd>(L"kam_mastery_tooltip.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFMasteryToolTipWnd>(L"kam_mastery_tooltip.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d7800
void RFMasteryToolTipWnd::ShowTip983(bool show)
{
	if (m_pText)
	{
		FitSize983();
		SetSize(m_pText->GetSize());
		ShowWindow(show);
	}
}

// 005d7840
void RFMasteryToolTipWnd::OnInitialUpdate()
{
	m_pText = GetChild983<RFTextArea>(L"message");
}
