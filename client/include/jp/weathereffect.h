#pragma once
#include <vector>
#include <memory>
#include "weather.h"

class W3dAniSpr;

// JP 983: clima "efeito" (classe nova; RTTI CWeatherEffect, vtable 00d0aa3c, 0x30 bytes — `push 0x30` em 009b39af). É o CSnow do
// 645 (snow.cpp) com vários conjuntos de flocos, um por `AddEffect`, guardados por std::tr1::shared_ptr (RTTI
// std::tr1::_Ref_count<CWeatherEffect::sEffect>). Quem cria é o CSceneManager (009b39c7: dois "~gp_twinkle*_ball.jpg" e o
// "[2#zodiac_cloud.jpg"). Nomes de método e de campo meus, pelos do CSnow.
class CWeatherEffect : public CWeatherType
{
public:
	CWeatherEffect(); // 0056d890
	virtual ~CWeatherEffect(); // 0056d960 (slot 0 = 0056da40)

	virtual void Process(float dt); // 0056ccc0
	virtual void Display(); // 0056d1d0
	virtual void DisplayFullScreenOverlay(); // 0056c560

	// 0056da70 (13 argumentos, `ret 0x34`)
	void AddEffect(int num, const char* sprName, int sprIndex, float sprWidth, float sprHeight, float frameTime, float width,
		float height, bool bWind, float fall, float sway, int color, unsigned char blend);

protected:
	struct sFrag // 0x40
	{
		float pos[3]; // +0x00
		float basePos[3]; // +0x0c
		int sprIndex; // +0x18
		float sway[3]; // +0x1c
		float swayAngle; // +0x28
		float swayPhase; // +0x2c
		bool bDraw; // +0x30
		int color; // +0x34
		float frameTime; // +0x38
		float time; // +0x3c
	};

	struct sEffect // 0x28
	{
		~sEffect(); // 0056c100 (cópia fora de linha)

		sFrag* frag; // +0x00
		W3dAniSpr* spr; // +0x04
		int num; // +0x08
		int sprIndex; // +0x0c (< 0: sorteado por floco e animado)
		unsigned char blend; // +0x10 (1 = sem mistura aditiva)
		float size; // +0x14
		bool bAnimate; // +0x18
		bool bWind; // +0x19
		float fall; // +0x1c
		int color; // +0x20
		float sway; // +0x24
	};

	Waabb m_area; // +0x08
	std::vector<std::tr1::shared_ptr<sEffect> > m_effects; // +0x20
};
