#pragma once
char* __cdecl MakeStr(const char* format, ...);

class C3dFont
{
public:
	struct ProjectionStyle
	{
		enum Enum
		{
			Perspective,
			Screen,
		};
	};

	C3dFont();
	~C3dFont();

	void LoadTexture(const char* filename, int width, int height,
		const char* charTable, float offsetX, float offsetY);
	void SetPos(const WVector& pos);
	void SetSize(int width, int height);
	void SetColor(int color);
	void Printf(WView* view, int space, int flags, const char* str);
	void SetProjectionStyle(ProjectionStyle::Enum style)
	{
		m_projectionStyle = style;
	}

protected:
	void ClipChar(int index);

	int m_texHandle;
	const char* m_charTable;
	int m_cols;
	float m_uStep;
	float m_vStep;
	int m_width;
	int m_height;
	float m_offsetX;
	float m_offsetY;
	WVector m_pos;
	WTVertex m_vtx[4];
	WTVertex* m_pVtx[4];
	int m_reserved;
	ProjectionStyle::Enum m_projectionStyle;
};
