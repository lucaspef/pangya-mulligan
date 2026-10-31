#pragma once

#include <vector>
#include "ucclibrary.h"

enum eBrushType
{
	BRUSH_BRUSH,
	BRUSH_PEN,
	BRUSH_ERASER,
	BRUSH_PATTERN
};

class CUccBrush
{
public:
	CUccBrush();
	~CUccBrush();

	void Initialize(eBrushType type);
	void Clear();

	Bitmap* GetBrush(int index);
	void SetBrush(int index, Bitmap* brush);
	void AddBrush(Bitmap* brush);
	void RemoveBrush(int index);
	int GetBrushNum() { return m_brush.size(); }

private:
	Bitmap* LoadBrushFromFile(const char* filename);

	std::vector<Bitmap*> m_brush;
	eBrushType m_type;
};

struct sUccCurrentBrush
{
	sUccCurrentBrush();

	void SetBrush(eBrushType type, int index, unsigned char r, unsigned char g,
		unsigned char b);
	void SetBrush(eBrushType type, int index);
	void SetBrush(unsigned char r, unsigned char g, unsigned char b);
	void SetBrush(eBrushType type, Bitmap* brush);
	void GetBrushColor(unsigned char& r, unsigned char& g, unsigned char& b);

	Bitmap brush;
	eBrushType type;
	int index;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

CUccBrush* UccBrushList(eBrushType type);
sUccCurrentBrush* UccCurrentBrush();
