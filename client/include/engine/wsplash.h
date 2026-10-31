#pragma once

class WPoint;
class WRect;

class WSplash
{
public:
#ifdef WANGREAL_DEVICE
	WSplash() { }
#else
	WSplash();
#endif
#ifdef WANGREAL_DEVICE
	virtual ~WSplash() { }
#else
	virtual ~WSplash();
#endif
	virtual int Init(BITMAPINFO& bi, void* vram, bool screenSized) = 0;
	virtual int InitFromScreen(bool screenSized) = 0;
	virtual void Reset() = 0;
	virtual void Draw(unsigned long color) = 0;
	virtual void Draw(const WPoint* pos, const WRect* rect, unsigned long color)
#ifdef WANGREAL_DEVICE
		= 0
#endif
		;
	virtual void ResetScreenSize() = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual void SetTexCoordOffset(float u, float v) = 0;
};
