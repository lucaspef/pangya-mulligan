#pragma once

#include "uccbase.h"

class CUccColorPicker : public CUccBase
{
public:
	CUccColorPicker();
	virtual ~CUccColorPicker();

	virtual void Initialize(FrArea* area);
	virtual void DestroyVariables();

	void GetColor(const WPoint& pos, unsigned char& r, unsigned char& g,
		unsigned char& b);

	virtual bool IsInitialized() { return m_pArea && m_pTexture32; }
	virtual void Process() { }

	virtual void MouseEventHandler(eButton button, int event, const WPoint& pos)
	{
	}

protected:
	virtual void Refresh2D() { }
	virtual void Refresh3D() { }

	virtual void GenerateColorMap();
	void GetColor(int x, int y, unsigned char& r, unsigned char& g,
		unsigned char& b);
	sRGB m_color;
};

class CUccColorBrightness : public CUccColorPicker
{
public:
	CUccColorBrightness();
	virtual ~CUccColorBrightness();
	virtual void Initialize(FrArea* area);
	virtual bool IsInitialized() { return m_pArea && m_pTexture32; }

	void GenerateBrightnessMap(unsigned char r, unsigned char g,
		unsigned char b);

private:
	virtual void GenerateColorMap() { }
};
