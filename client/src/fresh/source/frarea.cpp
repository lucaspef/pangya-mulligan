#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include "frarea.h"
#include "frwndmanager.h"
#include "frelement.h"
#include "frgraphicinterface.h"
#include "binstr.h"
#include "wmath.h"

#ifndef REBANG_SEMANTIC_ONLY
// HACK: ts_parser owns this shared literal; reuse it to keep its original COMDAT slot.
extern "C" const char rb_ts_parser_format4[];
#pragma comment(linker, \
	"/alternatename:_rb_ts_parser_format4=?rb_str_FMGEODLN@?3??PushMacro@ts_Parser@@IAE_NPBDAAUsTS_Macro@@@Z@4QBDB")
#else
#define rb_ts_parser_format4 "%d %d %d %d"
#endif

IObject* FrAreaMakeInstance()
{
	return new FrArea;
}

struct __sFrArea
{
	__sFrArea()
	{
		ObjectFactory().AddObjectFunctor(FrAreaMakeInstance, "FrArea");
	}
};

const WRTTI FrArea::m_RTTI("FrArea", &FrWnd::m_RTTI);
static __sFrArea __implFrArea;

FrArea::FrArea()
	: m_pBitmap(NULL)
{
	m_center = false;
	m_stretch = false;
	m_mouseEvent = false;
	m_color = 0xffffffff;
	m_imgName = "";

	m_isClipping = false;
	m_rcClipArea = WRect(0, 0, 0, 0);
	m_pEntrance983 = NULL;
}

FrArea::~FrArea()
{
	if (m_pEntrance983)
	{
		delete m_pEntrance983;
		m_pEntrance983 = NULL;
	}
}

// 983: 00bea560
float __stdcall FrAreaEntrance983::Step(float target, float cur, float remain, float elapsed, bool* done)
{
	*done = false;
	float next = (float)(int)((target - cur) / remain * elapsed) + cur;
	if (cur < target)
	{
		if (next < target)
			return next;
	}
	else if (!(next <= target))
		return next;
	*done = true;
	return target;
}

// 983: 00beaa90
void FrAreaEntrance983::SetFrom(const char* from)
{
	if (_stricmp(from, "top") == 0)
		m_from = FROM_TOP;
	else if (_stricmp(from, "bottom") == 0)
		m_from = FROM_BOTTOM;
	else if (_stricmp(from, "left") == 0)
		m_from = FROM_LEFT;
	else if (_stricmp(from, "right") == 0)
		m_from = FROM_RIGHT;

	WRect rect = m_pArea->m_rect;
	switch (m_from)
	{
	case FROM_TOP:
		rect.y -= rect.h;
		break;
	case FROM_BOTTOM:
		rect.y += rect.h;
		break;
	case FROM_LEFT:
		rect.x -= rect.w;
		break;
	case FROM_RIGHT:
		rect.x += rect.w;
		break;
	}
	m_pArea->SetRect(rect);
}

// 983: 00beab90
void FrAreaEntrance983::Opening(float elapsed)
{
	FrArea* pArea = m_pArea;
	WRect rect = pArea->m_rect;
	bool done = false;
	switch (m_from)
	{
	case FROM_TOP:
	case FROM_BOTTOM:
		rect.y = Step(pArea->m_rcClipArea.y, rect.y, m_remain, elapsed, &done);
		break;
	case FROM_LEFT:
	case FROM_RIGHT:
		rect.x = Step(pArea->m_rcClipArea.x, rect.x, m_remain, elapsed, &done);
		break;
	default:
		goto skip;
	}
	if (done)
	{
		if (0.0f <= m_duration)
			m_state = ST_HOLD;
		else
		{
			pArea->m_isClipping = false;
			m_state = ST_DONE;
		}
	}
skip:
	float remain = m_remain - elapsed;
	if (remain < 0.001f)
		remain = 0.001f;
	m_remain = remain;
	m_pArea->SetRect(rect);
}

// 983: 00beacc0
void FrAreaEntrance983::Closing(float elapsed)
{
	FrArea* pArea = m_pArea;
	WRect rect = pArea->m_rect;
	bool done = false;
	switch (m_from)
	{
	case FROM_TOP:
		rect.y = Step(pArea->m_rcClipArea.y - pArea->m_rcClipArea.h, rect.y, m_remain, elapsed, &done);
		break;
	case FROM_BOTTOM:
		rect.y = Step(pArea->m_rcClipArea.h + pArea->m_rcClipArea.y, rect.y, m_remain, elapsed, &done);
		break;
	case FROM_LEFT:
		rect.x = Step(pArea->m_rcClipArea.x - pArea->m_rcClipArea.w, rect.x, m_remain, elapsed, &done);
		break;
	case FROM_RIGHT:
		rect.x = Step(pArea->m_rcClipArea.w + pArea->m_rcClipArea.x, rect.x, m_remain, elapsed, &done);
		break;
	default:
		goto skip;
	}
	if (done)
	{
		pArea->SetVisible(false);
		m_pArea->m_isClipping = false;
		m_state = ST_DONE;
	}
skip:
	float remain = m_remain - elapsed;
	if (remain < 0.001f)
		remain = 0.001f;
	m_remain = remain;
	m_pArea->SetRect(rect);
}

// 983: 00beae60
void FrAreaEntrance983::OnProc(float elapsed)
{
	switch (m_state)
	{
	case ST_WAIT_FADE:
	{
		float one = 1.0f;
		float alpha = m_pArea->GDI()->GetAlpha();
		if (WisEqual(alpha, one, 0.00001f))
			m_state = ST_SOUND;
		break;
	}
	case ST_SOUND:
		m_pArea->SetPushSound(m_sound.c_str());
		m_pArea->PlayPushSound();
		m_state = ST_OPENING;
		break;
	case ST_OPENING:
		Opening(elapsed);
		break;
	case ST_HOLD:
		m_duration -= elapsed;
		if (m_duration < 0.0f)
		{
			m_state = ST_CLOSING;
			m_remain = 0.2f;
		}
		break;
	case ST_CLOSING:
		Closing(elapsed);
		break;
	}
}

// 983: 00beaf80
void FrArea::OnProc(const float deltaTime)
{
	if (m_pEntrance983)
		m_pEntrance983->OnProc(deltaTime);
}

// 983: 00beb2e0
FrAreaEntrance983::FrAreaEntrance983(FrArea* pArea)
{
	m_remain = 1.0f;
	m_duration = -1.0f;
	m_pArea = pArea;
	m_from = FROM_TOP;
	m_sound = "";
	m_state = ST_WAIT_FADE;
	m_pArea->m_isClipping = true;

	WRect rect = m_pArea->m_rect;
	if (m_pArea->m_pBitmap)
	{
		rect.w = (float)m_pArea->m_pBitmap->Width();
		rect.h = (float)m_pArea->m_pBitmap->Height();
	}
	m_pArea->SetRect(rect);
	m_pArea->m_rcClipArea = rect;
}

// 983: 00beb400
void FrAreaEntrance983::Parse(const char* text)
{
	char item[260] = { 0 };
	char key[260] = { 0 };
	char value[260] = { 0 };
	cTokenV items;
	cTokenV pair;
	items.Init(text, strlen(text));
	for (;;)
	{
		if (!items.GetToken(item, " ", 1) && strlen(item) == 0)
			break;
		pair.Init(item, strlen(item));
		pair.GetToken(key, ":", 1);
		pair.GetToken(value, "", 1);
		if (_stricmp(key, "from") == 0)
			SetFrom(value);
		else if (_stricmp(key, "openingtime") == 0)
			m_remain = (float)atof(value);
		else if (_stricmp(key, "duration") == 0)
			m_duration = (float)atof(value);
		else if (_stricmp(key, "openingsound") == 0)
			m_sound = value;
	}
}

void FrArea::Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent)
{
	FrElementDoc* pDoc = pManager->GetDocument();

	bool bVisible = true;

	std::map<std::string, std::string>& param = item.m_param;

	if (param.find("bgimg") != param.end())
	{
		m_imgName = param["bgimg"];
		m_pBitmap = pDoc->GetBitmap(m_imgName);
	}

	if (param.find("center") != param.end())
		m_center = atoi(param["center"].c_str()) != 0;

	if (param.find("stretch") != param.end())
		m_stretch = atoi(param["stretch"].c_str()) != 0;

	if (param.find("mouse_event") != param.end())
		m_mouseEvent = atoi(param["mouse_event"].c_str()) != 0;

	if (param.find("color") != param.end())
		sscanf(param["color"].c_str(), "%x", &m_color);

	if (param.find("visible") != param.end())
		bVisible = atoi(param["visible"].c_str()) == 1;

	m_pItem = &item;

	WRect rect((float)item.m_rect.left, (float)item.m_rect.top,
		(short)(item.m_rect.right - item.m_rect.left),
		(short)(item.m_rect.bottom - item.m_rect.top));
	unsigned long style = (m_mouseEvent ? 0 : 0x40) | (bVisible ? 1 : 0);
	Create(item.m_caption.c_str(), item.m_name.c_str(), pManager, style, rect,
		pParent);

	if (param.find("enable_clip") != param.end())
		m_isClipping = atoi(param["enable_clip"].c_str()) == 1;

	m_rcClipArea = m_rect;
	if (param.find("clip_area") != param.end())
	{
		int v[4];
		sscanf(param["clip_area"].c_str(), rb_ts_parser_format4, &v[0], &v[1],
			&v[2], &v[3]);

		m_rcClipArea.x = (float)v[0];
		m_rcClipArea.y = (float)v[1];
		m_rcClipArea.w = (float)(v[2] - v[0]);
		m_rcClipArea.h = (float)(v[3] - v[1]);
	}

	if (param.find("entrance_effect") != param.end() && !m_pEntrance983)
	{
		m_pEntrance983 = new FrAreaEntrance983(this);
		m_pEntrance983->Parse(param["entrance_effect"].c_str());
	}
}

bool FrArea::SetBgImg(const char* name)
{
	if (!name)
	{
		m_pBitmap = NULL;
		return true;
	}

	if (!strcmpi(m_imgName.c_str(), name))
		return true;

	m_imgName = name;

	FrElementDoc* pDoc = WndManager()->GetDocument();
	if (pDoc)
	{
		m_pBitmap = pDoc->GetBitmap(name);
		if (m_pBitmap)
			return true;
	}

	return false;
}

bool FrArea::SetBgImg(const Bitmap* pBitmap)
{
	if (pBitmap)
	{
		m_pBitmap = pBitmap;
		return true;
	}

	return false;
}

void FrArea::OnDraw()
{
	FrWnd::OnDraw();

	WRect src, dst;
	if (ClippingArea(src, dst))
	{
		GDI()->DrawTexture(m_pBitmap, src, dst,
			FrALPHA(m_color, m_wndAlpha2 * m_wndAlpha), 0);
	}

	SendCmdToOwnerTarget(FRCMD_OWNERDRAW, 0, 0);
}

void FrArea::OnSetCursor(bool bInClient, const WPoint& mousePos)
{
	if (bInClient && m_mouseEvent)
		SetCursor(1);
}

void FrArea::SetMouseEvent(bool active)
{
	m_mouseEvent = active;

	if (active)
		m_dwStyle.Disable(0x40);
	else
		m_dwStyle.Enable(0x40);
}

bool FrArea::OnLButtonDown(const WPoint& mousePos)
{
	SendCmdToOwnerTarget(FRCMD_LBUTTONDOWN, 0, 0);
	return false;
}

bool FrArea::OnLButtonUp(const WPoint& mousePos)
{
	SendCmdToOwnerTarget(FRCMD_LBUTTONUP, 0, 0);
	return false;
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B1510 (provisorio)

bool FrArea::ClippingArea(WRect& src, WRect& dst)
{
	if (!m_pBitmap)
		return false;

	// B1510 (provisorio): queda d898-1 na entrada do Lounge = m_pBitmap (+0xd4) = 0x2d1 numa FrArea filha de form de topo;
	// registra qual janela/imagem antes de desenhar e nao desenha o lixo
	// D920: d898-1/reg-1700 tem m_pBitmap valido e o lixo em [m_pBitmap+4] (0x2d1/0x4cf; Width() le [[+4]+4]) = Bitmap liberado
	if ((unsigned long)m_pBitmap < 0x10000 || *(unsigned long*)((const char*)m_pBitmap + 4) < 0x10000)
	{
		FrWnd* pParent = GetParent();
		jp983_rastro("B1510 FrArea bmp=%p [+4]=%08x this=%p nome=%s img=%s pai=%p %s", m_pBitmap,
			(unsigned long)m_pBitmap < 0x10000 ? 0 : *(unsigned long*)((const char*)m_pBitmap + 4), this,
			m_wndName.c_str(), m_imgName.c_str(), pParent, pParent ? pParent->GetWindowName983().c_str() : "-");
		return false;
	}

	WRect re_src(0, 0, m_pBitmap->Width(), m_pBitmap->Height());
	WRect re_dst(m_rect.x, m_rect.y, m_rect.w, m_rect.h);

	if (m_center)
	{
		re_dst.w = (float)m_pBitmap->Width();
		re_dst.h = (float)m_pBitmap->Height();
		re_dst.x += (int)(((m_rect.w - re_dst.w) + 1) * 0.5f);
		re_dst.y += (int)(((m_rect.h - re_dst.h) + 1) * 0.5f);
	}
	else if (!m_stretch)
	{
		re_dst.w = (float)m_pBitmap->Width();
		re_dst.h = (float)m_pBitmap->Height();
	}

	dst = re_dst;
	src = re_src;

	if (!m_isClipping)
		return true;

	if (m_rcClipArea.x <= dst.x &&
		(m_rcClipArea.x + m_rcClipArea.w) >= (dst.x + dst.w) &&
		m_rcClipArea.y <= dst.y &&
		(m_rcClipArea.y + m_rcClipArea.h) >= (dst.y + dst.h))
		return true;

	re_dst.x = max(dst.x, m_rcClipArea.x);
	re_dst.y = max(dst.y, m_rcClipArea.y);
	re_dst.w =
		max(0, min(dst.x + dst.w, m_rcClipArea.x + m_rcClipArea.w) - re_dst.x);
	re_dst.h =
		max(0, min(dst.y + dst.h, m_rcClipArea.y + m_rcClipArea.h) - re_dst.y);

	re_src.x = max(0, (m_rcClipArea.x - dst.x) / dst.w) * re_src.w;
	re_src.y = max(0, (m_rcClipArea.y - dst.y) / dst.h) * re_src.h;
	re_src.w = min(1, re_dst.w / dst.w) * re_src.w;
	re_src.h = min(1, re_dst.h / dst.h) * re_src.h;

	dst = re_dst;
	src = re_src;

	return true;
}
