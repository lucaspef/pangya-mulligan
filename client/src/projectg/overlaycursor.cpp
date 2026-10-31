#include "minatl.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frwnd.h"
#include "mousecursor.h"
#include "wresrcmng.h"
#include "overlaycursor.h"

extern Fresh* g_pFresh;
extern WView* g_view;

COverlayCursor::COverlayCursor()
{
	m_eCursor = CURSOR_NONE;
	m_ppRect = NULL;
}

COverlayCursor::~COverlayCursor()
{
	std::map<int, sOCItem>::iterator it;

	for (it = m_cursorMap.begin(); it != m_cursorMap.end(); it++)
	{
		WOverlay* overlay = it->second.overlay;
		if (g_resrcmng && overlay)
			g_resrcmng->Release(overlay);
	}
}

void COverlayCursor::Add(eCursorType type, const char* name, int x, int y)
{
	if (name == NULL)
		return;

	if (strlen(name) == 0)
		return;

	WOverlay* overlay = g_resrcmng->GetOverlay(name, 0);

	if (overlay == NULL)
		return;

	sOCItem cursor;
	cursor.type = type;
	cursor.overlay = overlay;
	cursor.x = x;
	cursor.y = y;

	m_cursorMap.insert(std::map<int, sOCItem>::value_type(type, cursor));
}

void COverlayCursor::Render()
{
	const WPoint& pos = g_pFresh->GetManager()->GetMousePos();

	if (m_eCursor == CURSOR_NONE)
	{
		CMouseCursor::Instance()->ForceMove(pos.x, pos.y, -1.0f);
		CMouseCursor::Instance()->Toggle(true);
		return;
	}

	std::map<int, sOCItem>::iterator it = m_cursorMap.find(m_eCursor);

	if (it == m_cursorMap.end())
	{
		CMouseCursor::Instance()->Toggle(true);
		return;
	}

	if (m_ppRect && *m_ppRect)
	{
		WRect rect = (*m_ppRect)->GetRect();

		if (!(rect.x <= pos.x && pos.x <= rect.x + rect.w && rect.y <= pos.y &&
				pos.y <= rect.y + rect.h))
		{
			CMouseCursor::Instance()->ForceMove(pos.x, pos.y, -1.0f);
			CMouseCursor::Instance()->Toggle(true);
			return;
		}
	}

	if (!(*m_ppRect)->IsViewFocused())
	{
		CMouseCursor::Instance()->Toggle(true);
		return;
	}

	CMouseCursor::Instance()->Toggle(false);

	WOverlay* overlay = it->second.overlay;
	WRect dest;
	dest.x = pos.x - it->second.x;
	dest.y = pos.y - it->second.y;
	dest.w = overlay->GetWidth();
	dest.h = overlay->GetHeight();

	overlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f), dest, 0, 0xffffffff,
		0.0f, 0);
}
