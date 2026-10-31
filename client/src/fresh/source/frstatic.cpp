#include <stdlib.h>
#include "frstatic.h"
#include "frwndmanager.h"
#include "frelement.h"
#include "frgraphicinterface.h"
#include "commonutil.h"
#include "inputmanager.h"
#include "binstr.h"

IObject* FrStaticMakeInstance()
{
	return new FrStatic;
}

struct __sFrStatic
{
	__sFrStatic()
	{
		ObjectFactory().AddObjectFunctor(FrStaticMakeInstance, "FrStatic");
	}
};

const WRTTI FrStatic::m_RTTI("FrStatic", &FrWnd::m_RTTI);
static __sFrStatic __implFrStatic;

FrStatic::FrStatic()
	: m_scrollRect983(m_rect)
{
	m_style = 0;
	m_align = 0;
	m_clip = NULL;
	m_smallFont = false;
	m_scrolling983 = false;
	m_color = 0xff808080;
	m_outlineColor = 0xffffffff;
}

FrStatic::~FrStatic()
{
	if (m_clip)
	{
		delete m_clip;
		m_clip = NULL;
	}
}

// 983: 00c02500
void FrStatic::SetCaption(const char* caption)
{
	std::string str = caption;
	if (IsLongCaption983(str))
	{
		char buf[200];
		memset(buf, 0, sizeof(buf));
		mbscut(buf, str.c_str(), 100);
		m_caption = buf;
		m_caption += "...";
	}
	else
		m_caption = str;

	if (m_scrolling983)
	{
		m_scrollRect983 = m_rect;
		m_scrollRect983.w = GDI()->GetTextExtendOld983(m_caption.c_str());
		SetClippingArea(&m_rect);
	}
}

void FrStatic::Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent)
{
	FrElementDoc* pDoc = pManager->GetDocument();

	std::map<std::string, std::string>& param = item.m_param;
	if (param.find("style") != param.end())
		m_style = atoi(param["style"].c_str());

	if (param.find("align") != param.end())
		m_align = atoi(param["align"].c_str());

	if (param.find("color") != param.end())
		m_color = htoi(param["color"].c_str());

	if (m_style & 6)
		m_outlineColor = htoi(param["color2"].c_str());

	if (param.find("scrolling") != param.end())
		m_scrolling983 = _stricmp(param["scrolling"].c_str(), "true") == 0;

	_RectangleSHORT r = item.m_rect;
	Create(item.m_caption.c_str(), item.m_name.c_str(), pManager, FWS_VISIBLE,
		WRect(r.left, r.top, r.Width(), r.Height()), pParent);
	SetCaption(item.m_caption.c_str());
}

// 983: 00c01d50
void FrStatic::ScrollBack983(float elapsed, float speed)
{
	m_scrollRect983.x = elapsed * speed + m_scrollRect983.x;
	if (m_scrollRect983.x >= m_rect.x)
		m_scrollRect983.x = m_rect.x;
}

// 983: 00c01e40
void FrStatic::ScrollText983(float elapsed, float speed)
{
	m_scrollRect983.x -= elapsed * speed;
	if (m_scrollRect983.w + m_scrollRect983.x <= m_rect.x)
		m_scrollRect983.x = m_rect.w + m_rect.x - m_scrollRect983.w;
}

// 983: 00c01ec0
void FrStatic::MoveWindow(const WPoint& point)
{
	float oldX = m_rect.x;
	float oldY = m_rect.y;
	float newX = point.x;
	float newY = point.y;
	FrWnd::MoveWindow(point);
	if (m_scrolling983)
	{
		m_scrollRect983.x += newX - oldX;
		m_scrollRect983.y += newY - oldY;
		SetClippingArea(&m_rect);
	}
}

// 983: 00c01f30
bool __stdcall FrStatic::IsLongCaption983(const std::string& caption)
{
	return caption.size() > 100;
}

// 983: 00c02160 — com o cursor em cima o texto mais largo que a janela rola; fora, volta.
void FrStatic::OnProc(const float deltaTime)
{
	if (!m_scrolling983)
	{
		m_scrollRect983 = m_rect;
		return;
	}

	float mouseY = g_input->GetMousePoint().y;
	float mouseX = g_input->GetMousePoint().x;
	if (m_rect.x <= mouseX && mouseX <= m_rect.w + m_rect.x &&
		m_rect.y <= mouseY && mouseY <= m_rect.h + m_rect.y)
	{
		if (0.0f < m_scrollRect983.w && m_rect.w < m_scrollRect983.w &&
			m_rect.w + m_rect.x < m_scrollRect983.w + m_scrollRect983.x)
			ScrollText983(deltaTime, 100.0f);
	}
	else if (m_scrollRect983.x < m_rect.x)
	{
		m_scrollRect983.x = deltaTime * 100.0 + m_scrollRect983.x;
		if (m_scrollRect983.x >= m_rect.x)
			m_scrollRect983.x = m_rect.x;
	}
}

void FrStatic::SetClippingArea(WRect* clip)
{
	if (clip)
	{
		if (!m_clip)
			m_clip = new WRect(*clip);
		else
		{
			m_clip->x = clip->x;
			m_clip->y = clip->y;
			m_clip->w = clip->w;
			m_clip->h = clip->h;
		}
	}
	else if (m_clip)
	{
		delete m_clip;
		m_clip = NULL;
	}
}

void FrStatic::SetTextColor(unsigned long color, unsigned long outline_color)
{
	m_color = color;
	m_outlineColor = outline_color;
}

void FrStatic::OnDraw()
{
	FrWnd::OnDraw();

	if (WndManager()->HidePrivacy() && m_nFlags.GetFlag(FWF_PRIVACY))
		return;

	if (m_clip)
		GDI()->SetClippingArea(m_clip);

	if (IsEnabled())
		GDI()->SetTextColor(FrALPHA(m_color, m_wndAlpha2 * m_wndAlpha),
			FrALPHA(m_outlineColor, m_wndAlpha2 * m_wndAlpha));
	else
		GDI()->SetTextColor(FrALPHA(m_color, 0.4f), 0xffffffff);

	GDI()->SetTextStyle(m_style);
	if (m_smallFont)
		WndManager()->PrintText11(WPoint(m_scrollRect983.x, m_scrollRect983.y), m_align,
			m_caption.c_str(), -1.0f, 0xffffffff);
	else
		WndManager()->PrintText(WPoint(m_scrollRect983.x, m_scrollRect983.y), m_align,
			m_caption.c_str(), -1.0f, 0xffffffff);

	if (m_clip)
		GDI()->SetClippingArea(NULL);
}

void FrStatic::Enable(bool enable)
{
	m_dwStyle.Turn(FWS_DISABLED, !enable);
}
