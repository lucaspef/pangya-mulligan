#pragma once

#include "weather.h"

class ntIceSnow : public CWeatherType
{
public:
	ntIceSnow(int num, const char* sprName, int sprIndex);
	virtual ~ntIceSnow();

	virtual void Process(float dt);
	virtual void Display();
	virtual void DisplayFullScreenOverlay();
	virtual void Reset(int num, float size);

protected:
	struct sFrag
	{
		WVector pos;
		WVector origin;
		int sprIndex;
		WVector sway;
		float angle;
		float phase;
		bool bVisible;
	};

	float m_size;
	Waabb m_area;
	sFrag* m_frag;
	W3dAniSpr* m_spr;
	int m_num;
	int m_sprIndex;
};
