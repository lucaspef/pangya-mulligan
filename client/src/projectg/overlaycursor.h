#pragma once
#include <map>

class FrWnd;
class WOverlay;

struct sOCItem
{
	sOCItem()
	{
		type = 0;
		overlay = NULL;
		x = 0;
		y = 0;
	}

	int type;
	WOverlay* overlay;
	int x;
	int y;
};

class COverlayCursor
{
public:
	enum eCursorType
	{
		CURSOR_NONE = 0
	};

	COverlayCursor();
	~COverlayCursor();

	void Add(eCursorType type, const char* name, int x, int y);
	void Render();
	void SetRect(FrWnd** ppRect) { m_ppRect = ppRect; }
	void SetCursor(eCursorType type) { m_eCursor = type; }

private:
	std::map<int, sOCItem> m_cursorMap;
	FrWnd** m_ppRect;
	eCursorType m_eCursor;
};
