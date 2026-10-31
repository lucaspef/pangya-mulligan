#pragma once

class FrWnd;

enum eFadeType
{
	FADE_OUT = -1,
	FADE_IN = 1,
	FADE_INOUT = 2,
};

class CFadingText
{
public:
	CFadingText();
	~CFadingText();

	void Print(const char* text, float time, FrWnd* pWnd, float x, float y,
		eFadeType type, unsigned long color);
	void Process();

protected:
	char m_text[512];
	float m_time;
	FrWnd* m_pWnd;
	float m_x;
	float m_y;
	unsigned long m_color;
	eFadeType m_type;
	float m_elapsed;
	unsigned char m_alpha;
	bool m_bActive;
	int m_dir;
};
