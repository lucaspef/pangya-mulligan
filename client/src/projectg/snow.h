#pragma once

#include "weather.h"

class CSnow : public CWeatherType
{
public:
	CSnow(int num, const char* sprName, int sprIndex, int sprWidth,
		int sprHeight);
	virtual ~CSnow();

	virtual void Process(float dt);
	virtual void Display();
	virtual void DisplayFullScreenOverlay();
	virtual void Reset(int num, float size);

	void TurnOffAdditiveBlend();

protected:
	struct sFrag
	{
		WVector pos;
		WVector basePos;
		int sprIndex;
		WVector sway;
		float swayAngle;
		float swayPhase;
		bool bDraw;
	};

	float m_size;
	Waabb m_area;
	sFrag* m_frag;
	W3dAniSpr* m_spr;
	int m_num;
	int m_sprIndex;
	bool m_bNoAdditiveBlend;
};
