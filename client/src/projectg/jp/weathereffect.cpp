// JP 983: unidade 0056c0e0–0056ddc0 (CWeatherEffect). Classe nova do 983, escrita sobre o CSnow do 645 (snow.cpp da frente A):
// mesma conta por floco, com a lista de conjuntos (`sEffect`) no lugar dos campos soltos. O grosso da faixa são instâncias do
// std::vector / std::tr1::shared_ptr<sEffect>.
#include "minatl.h"
#include "projectg.h"
#include "weathereffect.h"
#include "w3danispr.h"
#include "wind.h"

extern WMatrix g_camera;

// valor absoluto pelo bit de sinal (`test [mem], 0x80000000` + `fchs`)
inline float Abs983(const float& f)
{
	return (*(unsigned long*)&f & 0x80000000) ? -f : f;
}

inline bool IsZero983(const float& f, float epsilon = 0.00001f)
{
	return epsilon > Abs983(f);
}

inline CWeatherEffect::sEffect::~sEffect()
{
	if (frag)
	{
		delete[] frag;
		frag = NULL;
	}
	if (g_resrcmng && spr)
	{
		g_resrcmng->Release(spr);
		spr = NULL;
	}
}

void CWeatherEffect::DisplayFullScreenOverlay()
{
	if (Doc()->m_golfGame.weather)
		WOverlay::DrawBox(g_view,
			WRect(0, 0, g_view->GetWidth(), g_view->GetHeight()), 0, 0x4c3c3c3c,
			0.001f);
}

void CWeatherEffect::Process(float dt)
{
	Waabb box = m_area;
	box.min += g_camera.pivot;
	box.max += g_camera.pivot;

	WVector size = m_area.max - m_area.min;

	for (std::vector<std::tr1::shared_ptr<sEffect> >::iterator it = m_effects.begin(); it != m_effects.end(); ++it)
	{
		std::tr1::shared_ptr<sEffect> effect = *it;
		for (int i = 0; i < effect->num; i++)
		{
			if (effect->bWind)
				*(WVector*)effect->frag[i].basePos += Wind().GetWind(*(WVector*)effect->frag[i].basePos) * dt;
			if (0.00001f <= Abs983(effect->fall))
				effect->frag[i].basePos[1] -= dt * (effect->fall * 34.29529571533203);

			effect->frag[i].swayPhase += dt * (g_PI / 2.0f);
			*(WVector*)effect->frag[i].sway = WVector(cosf(effect->frag[i].swayAngle), 0.0f,
										  sinf(effect->frag[i].swayAngle)) *
				effect->sway * sinf(effect->frag[i].swayPhase);

			for (int j = 0; j < 3; j++)
			{
				if (effect->frag[i].basePos[j] > box.max.p[j])
					effect->frag[i].basePos[j] -=
						size.p[j] * ceilf((effect->frag[i].basePos[j] - box.max.p[j]) / size.p[j]);
				else if (effect->frag[i].basePos[j] < box.min.p[j])
					effect->frag[i].basePos[j] +=
						size.p[j] * ceilf((box.min.p[j] - effect->frag[i].basePos[j]) / size.p[j]);
			}

			*(WVector*)effect->frag[i].pos = *(WVector*)effect->frag[i].basePos + *(WVector*)effect->frag[i].sway;
			effect->frag[i].bDraw = (*(WVector*)effect->frag[i].pos - g_camera.pivot) * g_camera.za > 0.0f;

			if (effect->bAnimate)
			{
				if (effect->frag[i].time >= effect->frag[i].frameTime)
				{
					effect->frag[i].time = 0.0f;
					effect->frag[i].sprIndex++;
					effect->frag[i].sprIndex = effect->frag[i].sprIndex % effect->spr->GetSpriteNum();
				}
				effect->frag[i].time = effect->frag[i].time + dt;
			}
		}
	}
}

void CWeatherEffect::Display()
{
	for (std::vector<std::tr1::shared_ptr<sEffect> >::iterator it = m_effects.begin(); it != m_effects.end(); ++it)
	{
		std::tr1::shared_ptr<sEffect> effect = *it;
		int type = (effect->blend == 1) ? 0x20000000 : 0x20800000;

		for (int i = 0; i < effect->num; i++)
		{
			if (effect->frag[i].bDraw)
			{
				effect->spr->SetPos(*(WVector*)effect->frag[i].pos);
				effect->spr->SetColor(effect->frag[i].color);
				effect->spr->Render(g_view, 0.0f, type, effect->frag[i].sprIndex);
			}
		}
	}
}

CWeatherEffect::CWeatherEffect()
{
	m_area.min = WVector(-160.0f, -112.0f, -160.0f);
	m_area.max = WVector(160.0f, 48.0f, 160.0f);

	m_effects.clear();
}

CWeatherEffect::~CWeatherEffect()
{
	m_effects.clear();
}

void CWeatherEffect::AddEffect(int num, const char* sprName, int sprIndex, float sprWidth, float sprHeight, float frameTime,
	float width, float height, bool bWind, float fall, float sway, int color, unsigned char blend)
{
	std::tr1::shared_ptr<sEffect> effect(new sEffect);

	effect->num = num;
	effect->sprIndex = sprIndex;
	effect->blend = blend;
	effect->size = (width > height) ? width : height;
	effect->bAnimate = sprIndex < 0;
	effect->bWind = bWind;
	effect->fall = fall;
	effect->color = color;
	effect->sway = sway;

	effect->frag = new sFrag[effect->num];
	effect->spr = g_resrcmng->Get3DAniSpr(sprName, 0, sprWidth, sprHeight);
	effect->spr->SetRect(width, height, width * 0.5, height * 0.5);
	effect->spr->SetColor(color);

	for (int i = 0; i < effect->num; i++)
	{
		effect->frag[i].sprIndex =
			(effect->sprIndex < 0) ? rand() % effect->spr->GetSpriteNum() : effect->sprIndex;
		effect->frag[i].swayAngle = Random(0.0f, g_PI * 2.0f);
		effect->frag[i].swayPhase = Random(0.0f, g_PI);
		effect->frag[i].frameTime = (effect->sprIndex >= 0) ? 0.0f : frameTime;
		effect->frag[i].time = 0.0f;

		for (int j = 0; j < 3; j++)
			effect->frag[i].basePos[j] = Random(-1.0f, 1.0f) * 160.0 + g_camera.pivot.p[j];

		*(WVector*)effect->frag[i].pos = *(WVector*)effect->frag[i].basePos;
		effect->frag[i].color = color;
	}

	m_effects.push_back(effect);
}
