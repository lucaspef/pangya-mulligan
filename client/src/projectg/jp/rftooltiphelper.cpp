// JP 983: unidade 005dfa30–005e0420 — RFTooltipHelper, o ponto único das dicas da RF que as janelas do lobby e da sala chamam
// (Show983/ShowImage983/Hide983). O corte da F3.7.1 começava em 005dfbf0 (construtor), mas ShowCurrent983/Hide983, os moldes do
// WSingleton<RFTooltipHelper> (005dfa50/005dfa70/005dfa90), o destrutor da sTipInfo983 (005dfb40) e o Reset983 vêm antes dele.
#include "rftooltiphelper.h"
#include "refresh/rftooltip.h"

std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

// 005dfd30
bool RFTooltipHelper::sTipInfo983::operator!=(const sTipInfo983& other) const
{
	if (align != other.align || pos != other.pos || type != other.type)
		return true;
	switch (type)
	{
	case 2:
		if (image != other.image)
			return true;
	case 1:
		if (text.compare(other.text) != 0)
			return true;
	}
	return false;
}

// 005dfa30
void RFTooltipHelper::ShowCurrent983(bool show)
{
	if (m_pCurrent)
		m_pCurrent->Show983(show);
}

// 005dfad0
void RFTooltipHelper::Hide983()
{
	ShowCurrent983(false);
}

// 005dfb90
void RFTooltipHelper::Reset983()
{
	m_pCurrent = 0;
	m_info.target = 0;
	m_info.type = 0;
	m_info.pos = 0;
	m_info.text.clear();
	m_info.image.clear();
	m_info.align = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (m_wnds[i])
			m_wnds[i]->Show983(false);
	}
}

// 005dfbf0
RFTooltipHelper::RFTooltipHelper()
{
	m_pCurrent = 0;
	m_wnds[0] = 0;
	m_wnds[1] = 0;
	m_wnds[2] = 0;
	m_wnds[1] = RFCommonTooltip::Create983(0);
	m_wnds[2] = RFCommonTooltipByImage::Create983(0);
}

// 005dfc80
RFTooltipHelper::~RFTooltipHelper()
{
	Reset983();
}

// 005dfe20
void RFTooltipHelper::Set983(int target, int type, int pos, const std::string& text, const std::string& image, int align)
{
	sTipInfo983 info;
	info.target = target;
	info.type = type;
	info.pos = pos;
	info.text = text;
	info.image = image;
	info.align = align;
	if (!(m_info != info))
		return;
	ShowCurrent983(false);
	m_pCurrent = m_wnds[type];
	m_info = info;
	if (type == 1)
	{
		if (m_pCurrent)
		{
			m_pCurrent->SetTooltip983((RFWnd*)target, pos, align);
			((RFCommonTooltip*)m_pCurrent)->SetText983(MultiByteToWide983(std::string(text.c_str())));
		}
	}
	else if (type == 2)
	{
		if (m_pCurrent)
		{
			m_pCurrent->SetTooltip983((RFWnd*)target, pos, align);
			std::string t(text.c_str());
			std::string i(image.c_str());
			((RFCommonTooltipByImage*)m_pCurrent)->SetImageText983(MultiByteToWide983(i), MultiByteToWide983(t));
		}
	}
}

// 005e01c0
void RFTooltipHelper::Show983(int target, const std::string& text, int pos, int align)
{
	Set983(target, 1, pos, std::string(text.c_str()), std::string(""), align);
	ShowCurrent983(true);
}

// 005e02e0
void RFTooltipHelper::ShowImage983(int target, const std::string& image, const std::string& text, int pos, int align)
{
	Set983(target, 2, pos, std::string(text.c_str()), std::string(image.c_str()), align);
	ShowCurrent983(true);
}
