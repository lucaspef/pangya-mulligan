// JP 983: unidade 005ce010–005cfe30 — as dicas da RF: RFTooltip (base: posição pelo alvo ou pelo mouse, medida do texto com o
// FrGraphicInterface da tela, some quando o mouse sai do alvo) e as duas janelas do RFTooltipHelper, RFCommonTooltip (texto,
// "tooltip_common_wnd.xml") e RFCommonTooltipByImage (imagem + texto, "tooltip_common_wnd2.xml"). Os moldes da RF que caíram aqui
// (RFFormBase<RFWndEx>::SetVisible983/Show983 005ce780/005ce880, MemberFunctionFunctor<RFTooltip>, RFManager::CreateWnd<T>/
// CreateDesktop<T>, wstring::replace) vêm dos cabeçalhos. 005cfe30 em diante é da unidade seguinte (chamados por 005d0800).
#include <algorithm>
#include "refresh/rftooltip.h"
#include "lobbyjp.h"
#include "frgraphicinterface.h"

std::string WideToMultiByte983(const std::wstring& text); // 005e4d60 (textconv.cpp)

extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho

extern const RFRect g_rcRFZero983; // 00f1de34
extern const RFVector2 g_vRFZero983; // 00f1de44


RFWnd* RFCommonTooltip::s_pParent983;
RFWnd* RFCommonTooltipByImage::s_pParent983;

// ---------------------------------------------------------------------------------------------------------------------
// RFTooltip

// 005ce010
void RFTooltip::Show983(bool show)
{
	m_bShow = show;
}

// 005ce020
float RFTooltip::GetLineHeight983(int fontSize)
{
	if (m_pGraphic)
	{
		if (fontSize <= 11)
			return (float)m_pGraphic->GetFontHeight11();
		return (float)m_pGraphic->GetFontHeight();
	}
	return 0;
}

// 005ce0c0
RFTooltip::RFTooltip()
	: m_bShow(false), m_pGraphic(NULL), m_pos(g_vRFZero983), m_targetRect(g_rcRFZero983), m_bFollowMouse(true), m_pTarget(NULL),
	m_style(0), m_pFrame(NULL), m_pTextArea(NULL), m_pPicture(NULL)
{
}

// 005ce160
void RFTooltip::SetTooltip983(RFWnd* target, int style, int fontSize)
{
	// a tooltip e criada cedo (RFTooltipHelper): se OnInitialUpdate nao achou a tela, pega o grafico aqui
	if (!m_pGraphic && g_pF02340_983 && FRESHMGR983())
		m_pGraphic = FRMGR_GDI983(FRESHMGR983());
	m_pTarget = NULL;
	if (target)
	{
	if (!m_bFollowMouse)
	{
		RFVector2 pos = target->GetScreenPos();
		RFSize size = target->GetSize();
		m_pos.x = pos.x + size.width - size.width * 0.3f;
		m_pos.y = pos.y + size.height + 2.0;
	}
	else
	{
		RFSize screen = RFSceneManager::GetManager983()->GetRoot983()->GetSize();
		float w = GetRect().GetWidth();
		m_pos.x = w + screen.width;
		float h = GetRect().GetHeight();
		m_pos.y = h + screen.height;
	}
	SetScreenPos(m_pos.x, m_pos.y);
	m_targetRect = target->GetScreenRect();
	m_pTarget = target;
	m_style = style;
	}
	else
	{
		m_bFollowMouse = true;
		m_style = style;
	}
}

// 005ce8c0: segue o mouse e mostra/esconde conforme m_bShow
void RFTooltip::Update983(EventArgs& e)
{
	if (m_bShow)
	{
		if (!g_bFocus)
			return;
		WVector* mouse = RFSceneManager::Instance()->GetPos983();
		RFWnd* hit = RFSceneManager::GetManager983()->HitTest2(RFVector2(mouse->x, mouse->y), true, true);
		if (hit && m_pTarget && hit != m_pTarget)
			Show983(false);
		if (m_bFollowMouse)
		{
			RFSize screen = RFSceneManager::GetManager983()->GetRoot983()->GetSize();
			RFSize size = GetSize();
			m_pos.x = mouse->x;
			m_pos.y = mouse->y;
			m_pos.x += m_style == 1 ? 15.0f : 20.0f;
			m_pos.y += m_style == 1 ? 15.0f : 18.0f;
			float right = size.width + m_pos.x;
			if (screen.width < right)
			{
				m_pos.x = screen.width - size.width;
				m_pos.y = m_pos.y + 14.0;
			}
			float bottom = size.height + m_pos.y;
			if (screen.height < bottom)
				m_pos.y = mouse->y - size.height - 10.0;
			SetScreenPos(m_pos.x, m_pos.y);
		}
	}
	bool show = m_bShow;
	if (show != IsWindowVisible(false))
		RFFormBase<RFWndEx>::Show983(show);
}

// 005cedb0
void RFTooltip::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventUpdate, Subscriber(&RFTooltip::Update983, this));
	FrWndManager* pScreen;
	if (g_pF02340_983 && (pScreen = FRESHMGR983()) != NULL)
		m_pGraphic = FRMGR_GDI983(pScreen);
}

// 005ceec0
float RFTooltip::GetTextWidth983(const std::wstring& text, int fontSize)
{
	if (m_pGraphic)
	{
		if (fontSize <= 11)
			return m_pGraphic->GetTextExtendOld11_983(WideToMultiByte983(text.c_str()).c_str());
		return m_pGraphic->GetTextExtendOld983(WideToMultiByte983(text.c_str()).c_str());
	}
	return 0;
}

// 005cf390: largura da linha mais larga e altura somada (as linhas separadas por '\n')
RFSize RFTooltip::MeasureText983(const std::wstring& text, int fontSize)
{
	float lineHeight = GetLineHeight983(fontSize);
	size_t start = 0;
	float width = 0;
	float height = 0;
	size_t pos = text.find(L'\n', 0);
	for (;;)
	{
		std::wstring line;
		if (start != std::wstring::npos)
		{
			if (pos != std::wstring::npos)
				line.assign(text.begin() + start, text.begin() + pos);
			else
				line.assign(text.begin() + start, text.end());
			float w = GetTextWidth983(line.c_str(), fontSize);
			if (w > width)
				width = w;
		}
		height += lineHeight;
		if (pos == std::wstring::npos)
			return RFSize(width, height);
		height = (float)m_pTextArea->GetTextHeight() - lineHeight + height;
		start = text.find(L'\n', pos) + 1;
		pos = text.find(L'\n', start);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// RFCommonTooltip

// 005ce060
void RFCommonTooltip::SetText983(const std::wstring& text)
{
	if (m_pTextArea)
	{
		m_pTextArea->SetText(text);
		m_pTextArea->FitToContent(false);
	}
	UpdateLayout983();
}

// 005ceb30
RFCommonTooltip* RFCommonTooltip::Create983(RFWnd* parent)
{
	RFCommonTooltip* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCommonTooltip>(L"tooltip_common_wnd.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCommonTooltip>(L"tooltip_common_wnd.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCommonTooltip>(L"tooltip_common_wnd.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005cf1f0
void RFCommonTooltip::ShowChild983(RFWnd* wnd, const wchar_t* name, bool show)
{
	RFWnd* child = wnd->GetChild(name);
	if (child)
		child->ShowWindow(show);
}

// 005cf700: escolhe a moldura e a área de texto do estilo e da fonte
void RFCommonTooltip::SetTooltip983(RFWnd* target, int style, int fontSize)
{
	ShowChild983(this, L"wnd_tooltip_simple", false);
	ShowChild983(this, L"wnd_frame_dark", false);
	ShowChild983(this, L"textarea_text_dark", false);
	ShowChild983(this, L"textarea_text_dark11", false);
	ShowChild983(this, L"textarea_text_simple", false);
	ShowChild983(this, L"textarea_text_simple11", false);
	switch (style)
	{
	case 0:
		m_pFrame = GetChild(L"wnd_tooltip_simple");
		m_pTextArea = reinterpret_cast<RFTextArea*>(GetChild(fontSize <= 11 ? L"textarea_text_simple11" : L"textarea_text_simple")); // E140: o filho real e RFTextAreaSC (outra hierarquia no fonte); dynamic_cast dava NULL e a dica ficava vazia
		ShowChild983(this, L"wnd_tooltip_simple", true);
		break;
	case 1:
		m_pFrame = GetChild(L"wnd_frame_dark");
		m_pTextArea = reinterpret_cast<RFTextArea*>(GetChild(fontSize <= 11 ? L"textarea_text_dark11" : L"textarea_text_dark")); // E140: o filho real e RFTextAreaSC (outra hierarquia no fonte); dynamic_cast dava NULL e a dica ficava vazia
		// 983 005cf883: mostra a moldura escura (com a simples, a caixa bege do .xml ficava vazia e sem medir: C133/L3)
		ShowChild983(this, L"wnd_frame_dark", true);
		break;
	}
	if (m_pTextArea)
		m_pTextArea->ShowWindow(true);
	RFTooltip::SetTooltip983(target, style, fontSize);
}

// 005cf9d0: a área de texto do tamanho do texto; a janela e a moldura com a margem dela
void RFCommonTooltip::UpdateLayout983()
{
	if (m_pGraphic && m_pFrame && m_pTextArea)
	{
		RFVector2 pos = m_pTextArea->GetPos();
		RFSize text = MeasureText983(m_pTextArea->GetText(), m_pTextArea->GetFontSize983());
		RFSize area = m_pTextArea->GetSize();
		m_pTextArea->SetSize(text);
		RFSize size(0, 0);
		area = m_pTextArea->GetSize();
		size.width = pos.x * 2.0 + area.width;
		size.height = pos.y * 2.0 + area.height - 1.0;
		SetSize(size);
		m_pFrame->SetSize(size);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// RFCommonTooltipByImage

// 005cec70
RFCommonTooltipByImage* RFCommonTooltipByImage::Create983(RFWnd* parent)
{
	RFCommonTooltipByImage* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCommonTooltipByImage>(L"tooltip_common_wnd2.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCommonTooltipByImage>(L"tooltip_common_wnd2.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCommonTooltipByImage>(L"tooltip_common_wnd2.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005cf080
void RFCommonTooltipByImage::SetImageText983(const std::wstring& image, const std::wstring& text)
{
	if (m_pPicture)
	{
		m_pPicture->SetPicture(image.c_str());
		m_pPicture->FitToContent();
	}
	if (m_pTextArea)
	{
		m_pTextArea->SetText(text);
		m_pTextArea->FitToContent(false);
	}
	UpdateLayout983();
}

// 005cf2c0
void RFCommonTooltipByImage::ShowChild983(RFWnd* wnd, const wchar_t* name, bool show)
{
	RFWnd* child = wnd->GetChild(name);
	if (child)
		child->ShowWindow(show);
}

// 005cfab0
void RFCommonTooltipByImage::SetTooltip983(RFWnd* target, int style, int fontSize)
{
	ShowChild983(this, L"wnd_frame_dark", true);
	ShowChild983(this, L"textarea_text_dark", false);
	ShowChild983(this, L"textarea_text_dark11", false);
	m_pFrame = GetChild(L"wnd_frame_dark");
	m_pPicture = dynamic_cast<RFPicture*>(GetChild(L"pic_dark"));
	m_pTextArea = reinterpret_cast<RFTextArea*>(GetChild(fontSize <= 11 ? L"textarea_text_dark11" : L"textarea_text_dark")); // E140: o filho real e RFTextAreaSC (outra hierarquia no fonte); dynamic_cast dava NULL e a dica ficava vazia
	if (m_pPicture)
		m_pPicture->ShowWindow(true);
	if (m_pTextArea)
		m_pTextArea->ShowWindow(true);
	RFTooltip::SetTooltip983(target, style, fontSize);
}

// 005cfcb0: imagem à esquerda, texto 10 à direita dela; altura = a maior das duas
void RFCommonTooltipByImage::UpdateLayout983()
{
	if (m_pGraphic && m_pFrame && m_pTextArea)
	{
		RFVector2 picPos = m_pPicture->GetPos();
		RFSize picSize = m_pPicture->GetSize();
		RFVector2 textPos = m_pTextArea->GetPos();
		textPos.x = picPos.x + picSize.width + 10.0;
		m_pTextArea->SetPos(textPos.x, textPos.y);
		RFSize text = MeasureText983(m_pTextArea->GetText(), m_pTextArea->GetFontSize983());
		RFSize textSize = text;
		m_pTextArea->SetSize(textSize);
		RFSize area = m_pTextArea->GetSize();
		RFSize size(0, 0);
		size.height = area.height > picSize.height ? area.height : picSize.height;
		area = m_pTextArea->GetSize();
		size.width = area.width + textPos.x + picPos.x;
		size.height = picPos.y + picPos.y + size.height - 1.0;
		SetSize(size);
		m_pFrame->SetSize(size);
	}
}
