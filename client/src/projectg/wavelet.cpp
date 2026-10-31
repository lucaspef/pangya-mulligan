#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "scenemanager.h"

WTVertex CWavelet::vtr[4];
WTVertex* CWavelet::vtx[5] = { &vtr[0], &vtr[1], &vtr[3], &vtr[2], NULL };

CWavelet::CWavelet()
	: m_texHandle(0), m_width(16.0f)
{
	vtr[0].tv = 0.0f;
	vtr[2].tv = 0.0f;
	vtr[1].tv = 1.0f;
	vtr[3].tv = 1.0f;
}

CWavelet::~CWavelet()
{
	m_wavelet.clear();
	m_layer.clear();

	if (g_resrcmng && m_texHandle)
	{
		g_resrcmng->Release(m_texHandle);
		m_texHandle = 0;
	}
}

void CWavelet::SetLayer(int num)
{
	m_layer.reserve(num);

	for (int i = 0; i < num; i++)
	{
		sLayer layer;
		layer.alpha = 0.0f;
		layer.speed = 38.4f;
		layer.pos = 0.0f;
		layer.state = 2;
		layer.delay = 0.0f;
		layer.time = 0.0f;

		m_layer.push_back(layer);
	}

	m_layer[0].state = 0;
}

void CWavelet::ReadAttribute(const char* name)
{
	strchr(name, '_'); // result unused in the original as well

	int index;
	if (strchr(name, '_') && sscanf(strchr(name, '_') + 1, "%d", &index))
	{
		m_texHandle = g_resrcmng->LoadTexture(
			GOLFDOC()->m_waveInfo.GetTexture(index), 0, 0, 0);
		m_width = GOLFDOC()->m_waveInfo.GetWidth(index);
		m_frequency = GOLFDOC()->m_waveInfo.GetFreq(index);

		m_fadeInTime = m_frequency * (1.2f / 15.0f);
		m_fadeOutTime = m_frequency * 0.72f;
		m_delay = m_frequency * 0.2f;

		for (int i = 0; i < 4; i++)
			vtr[i].diffuse = 0xffffff;

		m_deceleration = m_width /
			(4.0f * m_fadeInTime - m_fadeInTime * m_fadeInTime * 0.5f);
	}
}

void CWavelet::SetWavelet(const std::vector<WVector>& points, const char* name,
	int layerNum)
{
	ReadAttribute(name);

	m_wavelet.clear();
	m_wavelet.reserve(points.size());

	float y = GetPVS().GetGroundPoint(points[1], true, NULL, true).y;

	for (unsigned int i = 0; i < points.size(); i++)
	{
		sWavelet wavelet;
		wavelet.pos[0] = points[i];
		wavelet.pos[0].y = y;

		if (i == points.size() - 1)
		{
			wavelet.dir = m_wavelet[i - 1].dir;
		}
		else
		{
			wavelet.dir = points[i + 1] - points[i];
			wavelet.dir.y = 0.0f;
			wavelet.dir =
				WCrossProduct(wavelet.dir, WVector::UNIT_POS_Y).Normalize();
		}

		wavelet.pos[1] = wavelet.pos[0] + m_width * wavelet.dir;
		m_wavelet.push_back(wavelet);
	}

	for (unsigned int i = 0; i < 2; i++)
	{
		m_wavelet[0].u[i] = 0.0f;
		for (unsigned int j = 1; j < m_wavelet.size(); j++)
		{
			m_wavelet[j].u[i] = m_wavelet[j - 1].u[i] +
				(m_wavelet[j].pos[i] - m_wavelet[j - 1].pos[i]).Magnitude() /
					m_width;
		}
	}

	SetLayer(layerNum);
}

void CWavelet::Process(float dt)
{
	for (unsigned char i = 0; i < m_layer.size(); i++)
	{
		m_layer[i].time += dt;

		switch (m_layer[i].state)
		{
		case 0:
			m_layer[i].alpha += dt * (255.0f / m_fadeInTime);
			m_layer[i].speed -= dt * m_deceleration;
			m_layer[i].pos += dt * m_layer[i].speed;
			if (m_layer[i].alpha > 255.0f)
			{
				m_layer[i].alpha = 255.0f;
				m_layer[i].state = 1;
				m_layer[(i + 1) % m_layer.size()].delay = m_delay;
			}
			break;

		case 1:
			m_layer[i].alpha -= dt * (255.0f / m_fadeOutTime);
			m_layer[i].speed -= dt * m_deceleration;
			m_layer[i].pos += dt * m_layer[i].speed;
			if (m_layer[i].alpha < 0.0f)
				m_layer[i].state = 2;
			break;

		case 2:
			if (m_layer[i].delay > 0.0f)
			{
				m_layer[i].delay -= dt;
				if (m_layer[i].delay < 0.0f)
				{
					m_layer[i].state = 0;
					m_layer[i].alpha = 0.0f;
					m_layer[i].time = 0.0f;
					m_layer[i].pos = 0.0f;
					m_layer[i].speed = m_deceleration * 4.0f;
				}
			}
			break;
		}
	}
}

void CWavelet::Render()
{
	for (unsigned char j = 0; j < m_layer.size(); j++)
	{
		for (unsigned char k = 0; k < m_wavelet.size() - 1; k++)
		{
			unsigned char n;
			int m;
			for (n = 0, m = 0; n < 4; n++, m++)
			{
				unsigned char w = k + ((n & 2) ? 1 : 0);
				bool bEdge = (w == 0 || w == m_wavelet.size() - 1);
				bool bFar = (n & 1) != 0;
				float scale = bFar ? 0.1f : (bEdge ? 0.5f : 1.0f);

				WVector pos = m_wavelet[w].pos[m & 1] -
					scale * m_layer[j].pos * m_wavelet[w].dir;
				WVector ground;
				ground = GetPVS().GetGroundPoint(pos, true, NULL, true);

				vtr[n].SetPosition(ground);
				vtr[n].tu = m_wavelet[w].u[m & 1];
				if (bEdge)
					vtr[n].diffuse = 0xffffff;
				else
					vtr[n].diffuse = 0xffffff | ((int)m_layer[j].alpha << 24);
			}

			g_view->DrawPolygonFan(vtx, (m_texHandle & 0x7ff) | 0x10c00000,
				0x200, false);
		}
	}
}
