// JP 983: unidade 005b7070–005b7890 (CPowder + CAbbotPowder, o pó da Abbot Mine; ver include/jp/abbotpowder.h).
// Molde: o icesnow.cpp do 645 (ntIceSnow). Sessão B123.
#include "minatl.h"
#include "projectg.h"
#include "w3danispr.h"
#include "wind.h"
#include "abbotpowder.h"

extern WMatrix g_camera;

// 005b7070
CPowder::~CPowder()
{
	if (m_frag)
	{
		delete[] m_frag;
		m_frag = NULL;
	}
	if (g_resrcmng && m_spr)
	{
		g_resrcmng->Release(m_spr);
		m_spr = NULL;
	}
}

// 005b7130
void CPowder::Init983(const char* sprName, float sprSize, int sprIndex)
{
	m_frag = NULL;
	m_spr = g_resrcmng->Get3DAniSpr(sprName, 0, sprSize, sprSize);

	m_swayAmp = 4.0f;
	m_range = 160.0f;
	m_sprIndex = sprIndex;

	m_area.min = WVector(-160.0f, -112.0f, -160.0f);
	m_area.max = WVector(1.0f, 0.3f, 1.0f) * m_range;
}

// 005b71e0
void CPowder::Display()
{
	for (int i = 0; i < m_num; i++)
	{
		if (m_frag[i].bVisible)
		{
			m_spr->SetPos(m_frag[i].pos);
			m_spr->Render(g_view, 0.0f, 0x20800000, m_frag[i].sprIndex);
		}
	}
}

// 005b7590
void CPowder::DisplayFullScreenOverlay()
{
	if (Doc()->m_golfGame.weather)
		WOverlay::DrawBox(g_view,
			WRect(0, 0, g_view->GetWidth(), g_view->GetHeight()), 0, 0x4c3c3c3c,
			0.001f);
}

// 005b7610
void CPowder::Reset(int num, float size)
{
	m_num = num;

	if (m_frag)
	{
		delete[] m_frag;
		m_frag = NULL;
	}
	m_frag = new sFrag[m_num];
	m_size = size;

	m_spr->SetRect(size, size, size * 0.5f, size * 0.5f);

	for (int i = 0; i < m_num; i++)
	{
		m_frag[i].sprIndex =
			(m_sprIndex < 0) ? rand() % m_spr->GetSpriteNum() : m_sprIndex;
		m_frag[i].angle = Random(0.0f, g_PI * 2.0f);
		m_frag[i].phase = Random(0.0f, g_PI);

		for (int j = 0; j < 3; j++)
			m_frag[i].origin.p[j] =
				Random(-1.0f, 1.0f) * m_range + g_camera.pivot.p[j];

		m_frag[i].pos = m_frag[i].origin + m_frag[i].sway;
	}
}

// 005b77c0
CAbbotPowder::CAbbotPowder(int num, const char* sprName, int sprIndex)
{
	Init983(sprName, 8.0f, sprIndex);
	Reset(num, 1.0f);
}

// 005b7250: como o ntIceSnow::Process, mas o pó sobe (mesma gravidade, para cima) e balança nos três eixos
void CAbbotPowder::Process(float dt)
{
	Waabb box = m_area;
	box.min += g_camera.pivot;
	box.max += g_camera.pivot;

	WVector size = m_area.max - m_area.min;

	for (int i = 0; i < m_num; i++)
	{
		m_frag[i].origin += Wind().GetWind(m_frag[i].origin) * dt;
		m_frag[i].origin.y += m_gravity * 34.295296f * dt;

		m_frag[i].phase += dt * (g_PI * 0.2f);
		m_frag[i].sway =
			WVector(cosf(m_frag[i].angle), sinf(m_frag[i].angle), sinf(m_frag[i].angle)) *
			m_swayAmp * sinf(m_frag[i].phase);

		for (int j = 0; j < 3; j++)
		{
			if (m_frag[i].origin.p[j] > box.max.p[j])
				m_frag[i].origin.p[j] -=
					ceilf((m_frag[i].origin.p[j] - box.max.p[j]) / size.p[j]) *
					size.p[j];
			else if (m_frag[i].origin.p[j] < box.min.p[j])
				m_frag[i].origin.p[j] +=
					ceilf((box.min.p[j] - m_frag[i].origin.p[j]) / size.p[j]) *
					size.p[j];
		}

		m_frag[i].pos = m_frag[i].origin + m_frag[i].sway;
		m_frag[i].bVisible =
			(m_frag[i].pos - g_camera.pivot) * g_camera.za > 0.0f;
	}
}
