#pragma once

class CSimpleUI
{
public:
	CSimpleUI(const char* name);
	~CSimpleUI();

	void Add(const char* name, const WRect& rect, bool bAlloc);
	void Render(const char* name, float x, float y, float scale, int align);
	void DeleteAllItem();
	void SetColor(int color) { m_color = color; }
	void SetAngle(float angle) { m_angle = angle; }

private:
	WOverlay* m_pOverlay;
	float m_width;
	float m_height;
	int m_color;
	float m_angle;
	WList<WRect*> m_rectList;
};
