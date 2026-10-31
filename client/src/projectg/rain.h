#pragma once

#include "weather.h"

class CRain : public CWeatherType
{
public:
	CRain();
	virtual ~CRain();

	virtual void Process(float dt);
	virtual void Display();
	virtual void DisplayFullScreenOverlay();
	virtual void UpdateWind();

protected:
	struct sFrag
	{
		WVector pos;
		bool bDraw;
	};

	WVector m_area;
	float m_radius;
	WVector m_dir;
	sFrag* m_frag;
};
