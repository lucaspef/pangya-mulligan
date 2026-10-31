#pragma once
#include "weather.h"
#include "wmath.h"

class W3dAniSpr;

// JP 983: pó que sobe do chão da Abbot Mine (mapa 0x14). CSceneManager::SetWeather (009b3954) faz `new CAbbotPowder(num,
// "~glowsapiasmall.jpg", 0)` (0x3c) e guarda em +0xb4. Classes novas do 983 (existem no 839, não no 645). CPowder é o
// ntIceSnow do 645 com o tamanho da figura, a amplitude do balanço e o raio da caixa em campos, e um Init virtual novo
// (vtable +0x18); Process é puro. vtables: CPowder 00d0e2dc, CAbbotPowder 00d0e304. Unidade 005b7070–005b7890 (jp/abbotpowder.cpp;
// o corte do mapa começava em 005b6cfe, mas 005b6d30/005b6eb0 são do CPost_ReciveDlg). Nomes dos campos e do Init são meus.
class CPowder : public CWeatherType
{
public:
	virtual ~CPowder();								// 005b7070

	virtual void Display();							// 005b71e0
	virtual void DisplayFullScreenOverlay();		// 005b7590
	virtual void Reset(int num, float size);		// 005b7610
	virtual void Init983(const char* sprName, float sprSize, int sprIndex); // 005b7130

protected:
	struct sFrag
	{
		WVector pos;		// +0x00
		WVector origin;		// +0x0c
		int sprIndex;		// +0x18
		WVector sway;		// +0x1c
		float angle;		// +0x28
		float phase;		// +0x2c
		bool bVisible;		// +0x30
	};

	float m_size;			// +0x08
	Waabb m_area;			// +0x0c
	sFrag* m_frag;			// +0x24
	W3dAniSpr* m_spr;		// +0x28
	int m_num;				// +0x2c
	int m_sprIndex;			// +0x30
	float m_swayAmp;		// +0x34 (4)
	float m_range;			// +0x38 (160: meia largura da caixa; altura = 0,3×)
};

class CAbbotPowder : public CPowder
{
public:
	CAbbotPowder(int num, const char* sprName, int sprIndex);	// 005b77c0
	virtual ~CAbbotPowder() { }								// embutido no deleting dtor 005b7860

	virtual void Process(float dt);								// 005b7250
};
