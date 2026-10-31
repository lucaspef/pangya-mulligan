#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "scenemanager.h"
#include "slicestr.h"
#include "river.h"


WTVertex CRiver::vtr[4];
WTVertex* CRiver::vtx[5] = { &vtr[0], &vtr[1], &vtr[3], &vtr[2], NULL };
WTVertex ntRiver_Lava::vtr[4];
WTVertex* ntRiver_Lava::vtx[5] = { &vtr[0], &vtr[1], &vtr[3], &vtr[2], NULL };
WTVertex CRiver_ex::vtr[4];
WTVertex* CRiver_ex::vtx[5] = { &vtr[0], &vtr[1], &vtr[3], &vtr[2], NULL };

// ramo semântico: os leitores do CRiverInfo estão em linha no golfdoc.h
#define RIVERINFO() (GOLFDOC()->m_riverInfo)
#define PVS() (GetPVS()) // CQuadTree::GetLowestGround já está no quadtree.h

CRiver::CRiver()
{
	for (int i = 0; i < 2; i++)
	{
		m_side[i].scroll = 0.0f;
		m_side[i].scrollVel = 0.0f;
		m_side[i].texHandle = 0;
		m_side[i].tileHead = 0.0f;
		m_side[i].tileSide = 0.0f;
		m_side[i].depth = 0.0f;
		m_side[i].bActive = false;
	}
}

CRiver::~CRiver()
{
	for (int i = 0; i < 2; i++)
	{
		if (g_resrcmng && m_side[i].texHandle)
		{
			g_resrcmng->Release(m_side[i].texHandle);
			m_side[i].texHandle = 0;
		}
	}
}

const char* __fastcall GetCurMapName();

void CRiver::ReadAttribute(const char* name)
{
	int index[2] = { -1, -1 };

	if (strstr(name, "_"))
	{
		m_side[0].bActive = sscanf(strchr(name, '_') + 1, "%d", &index[0]) != 0;
		int n = sscanf(strrchr(name, '_') + 1, "%d", &index[1]);
		if (n == 0 || index[1] == index[0])
			m_side[1].bActive = false;
		else
			m_side[1].bActive = true;

		for (int i = 0; i < 2; i++)
		{
			if (m_side[i].bActive)
			{
				m_side[i].scroll = Random(0.0f, 1.0f);
				m_side[i].scrollVel = RIVERINFO().GetScrollVelocity(index[i]);
				m_side[i].tileHead = RIVERINFO().GetHeadTile(index[i]);
				m_side[i].tileSide = RIVERINFO().GetSideTile(index[i]);
				m_side[i].depth = RIVERINFO().GetDepth(index[i]) * 3.2f;
				m_side[i].texHandle = g_resrcmng->LoadTexture(
					MakeStr("[%s_river_%s.jpg",
						GetCurMapName(),
						i == 0 ? "up" : "down"),
					0, 0, 0);
			}
		}
		return;
	}

	if (strstr(name, "*d"))
	{
		float depth = 0.0f;
		if (sscanf(strchr(name, 'd') + 1, "%f", &depth) == -1)
			depth = 1.0f;

		for (int i = 0; i < 2; i++)
		{
			m_side[i].scroll = Random(0.0f, 1.0f);
			m_side[i].scrollVel = Random(1.0f, 3.0f);
			m_side[i].tileHead = 1.0f;
			m_side[i].tileSide = 1.0f;
			m_side[i].depth = depth;
			m_side[i].texHandle = g_resrcmng->LoadTexture(
				MakeStr("[%s_river_%s.jpg",
					GetCurMapName(),
					i == 0 ? "up" : "down"),
				0, 0, 0);
			m_side[i].bActive = true;
		}
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			m_side[i].scroll = Random(0.0f, 1.0f);
			m_side[i].scrollVel = Random(1.0f, 3.0f);
			m_side[i].tileHead = 1.0f;
			m_side[i].tileSide = 1.0f;

			const char* format;
			if (GetCurMap() == 11)
			{
				m_side[i].depth = 10.56f;
				format = "[pink_river_%s.jpg";
			}
			else
			{
				m_side[i].depth = 3.2f;
				if (GetCurMap() == 15)
					format = "[gs_river_%s.jpg";
				else
					format = "[wind_river_%s.jpg";
			}

			m_side[i].texHandle = g_resrcmng->LoadTexture(
				MakeStr(format, i == 0 ? "up" : "down"), 0, 0, 0);
			m_side[i].bActive = true;
		}
	}
}

void CRiver::Setup(const std::vector<WVector>& points, const char* name)
{
	ReadAttribute(name);

	WVector dir = points[1] - points[0];
	dir.y = 0.0f;
	dir = WCrossProduct(dir, WVector::UNIT_POS_Y).Normalize();

	m_step = 0.0f;
	float depth = m_side[0].depth;
	if (depth < m_side[1].depth)
		depth = m_side[1].depth;

	float limit = PVS().GetLowestGround(points[1]) + depth;
	float ground;
	do
	{
		m_step += 3.2f;
		ground = PVS().GetLowestGround(points[1] + m_step * dir);
	} while (ground <= limit);
	m_step += 3.2f;

	m_river.reserve(50);

	unsigned int count = 0;
	for (unsigned int i = 0; i < points.size() - 1; i++)
	{
		WVector seg = points[i + 1] - points[i];
		seg.y = 0.0f;
		WVector n = WCrossProduct(seg, WVector::UNIT_POS_Y).Normalize();
		WVector off = n * m_step;

		float g1 = PVS().GetLowestGround(points[i + 1]);
		float g0 = PVS().GetLowestGround(points[i]);
		float dh = abs(g1 - g0);
		float steps = ceil(dh / (double)1.6f);
		if (steps <= 1.0f)
			steps = 1.0f;
		float dt = 1.0f / steps;

		float prevGround = 0.0f;
		for (float t = 0.0f; t < 1.0f; t += dt)
		{
			WVector p = (1.0f - t) * points[i] + t * points[i + 1];
			float prev = prevGround;
			prevGround = PVS().GetLowestGround(p);
			if (prevGround <= prev + 1.6f)
			{
				sRiver r;
				float d = m_side[0].depth;
				for (unsigned int j = 0; j < 2; j++)
				{
					float sign = (j & 1) ? 1.0f : -1.0f;
					r.pos[j] = p + sign * off;
					r.pos[j].y = prevGround + d;
					if (count == 0)
					{
						r.u[j] = 0.0f;
					}
					else
					{
						const sRiver& last = m_river[count - 1];
						r.u[j] = (r.pos[j] - last.pos[j]).Magnitude() *
								m_side[j].tileHead / m_step +
							last.u[j];
					}
				}

				if (count != 0)
				{
					const sRiver& last = m_river[count - 1];
					Waabb box;
					box.max.x = max(max(r.pos[0].x, r.pos[1].x),
						max(last.pos[0].x, last.pos[1].x));
					box.max.y = min(last.pos[0].y, r.pos[0].y);
					box.max.z = max(max(r.pos[0].z, r.pos[1].z),
						max(last.pos[0].z, last.pos[1].z));
					box.min.x = min(min(r.pos[0].x, r.pos[1].x),
						min(last.pos[0].x, last.pos[1].x));
					box.min.y =
						box.max.y - max(m_side[1].depth, m_side[0].depth);
					box.min.z = min(min(r.pos[0].z, r.pos[1].z),
						min(last.pos[0].z, last.pos[1].z));
					GetPVS().AddWater(box, 2);
				}

				m_river.push_back(r);
				count++;
			}
		}
	}
}

void CRiver::Process(float dt)
{
	if (m_side[0].bActive)
		m_side[0].scroll -= dt * m_side[0].scrollVel;
	if (m_side[1].bActive)
		m_side[1].scroll -= dt * m_side[1].scrollVel;
}

void CRiver::Render()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_side[i].bActive)
		{
			for (unsigned char j = 0; j < 4; j++)
				vtr[j].tu = (j & 1) ? 0.0f : m_side[i].tileSide;

			for (unsigned char k = 0; k < m_river.size() - 1; k++)
			{
				unsigned char n;
				int m;
				for (n = 0, m = 0; n < 4; n++, m++)
				{
					unsigned char w = k + ((n & 2) ? 1 : 0);
					vtr[n].SetPosition(m_river[w].pos[m & 1]);
					vtr[n].tv = m_river[w].u[i] + m_side[i].scroll;
					if (w == 0 || w == m_river.size() - 1)
						vtr[n].diffuse = 0xffffff;
					else
						vtr[n].diffuse = 0xffffffff;
				}

				g_view->DrawPolygonFan(vtx,
					(m_side[i].texHandle & 0x7ff) | 0x10c00000, 0x200, false);
			}
		}
	}
}

ntRiver_Lava::ntRiver_Lava()
{
	for (int i = 0; i < 2; i++)
	{
		m_side[i].scroll = 0.0f;
		m_side[i].scrollVel = 0.0f;
		m_side[i].texHandle = 0;
		m_side[i].tileHead = 0.0f;
		m_side[i].tileSide = 0.0f;
		m_side[i].depth = 0.0f;
		m_side[i].bActive = false;
	}
}

ntRiver_Lava::~ntRiver_Lava()
{
	for (int i = 0; i < 2; i++)
	{
		if (g_resrcmng && m_side[i].texHandle)
		{
			g_resrcmng->Release(m_side[i].texHandle);
			m_side[i].texHandle = 0;
		}
	}
}

void ntRiver_Lava::ReadAttribute(const char* name)
{
	int index[2] = { 0, 1 };

	m_side[0].bActive = true;
	m_side[1].bActive = true;

	for (int i = 0; i < 2; i++)
	{
		m_side[i].scroll = Random(0.0f, 1.0f);
		m_side[i].scrollVel = RIVERINFO().GetScrollVelocity(index[i]);
		m_side[i].tileHead = RIVERINFO().GetHeadTile(index[i]);
		m_side[i].tileSide = RIVERINFO().GetSideTile(index[i]);
		m_side[i].depth = RIVERINFO().GetDepth(index[i]) * 3.2f;
		m_side[i].texHandle = g_resrcmng->LoadTexture(
			MakeStr("[hell_river_%s.jpg", i == 0 ? "up" : "down"), 0, 0, 0);
	}

	if (strstr(name, "*d"))
	{
		float depth = 0.0f;
		if (sscanf(strchr(name, 'd') + 1, "%f", &depth) == -1)
			depth = 1.0f;

		m_side[0].depth = depth;
		m_side[1].depth = depth;
	}
}

void ntRiver_Lava::Process(float dt)
{
	if (m_side[0].bActive)
		m_side[0].scroll -= dt * m_side[0].scrollVel;
	if (m_side[1].bActive)
		m_side[1].scroll -= dt * m_side[1].scrollVel;
}

void ntRiver_Lava::Render()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_side[i].bActive)
		{
			for (unsigned char j = 0; j < 4; j++)
				vtr[j].tu = (j & 1) ? 0.0f : 1.0f;

			for (unsigned char k = 0; k < m_river.size() - 1; k++)
			{
				unsigned char n;
				int m;
				for (n = 0, m = 0; n < 4; n++, m++)
				{
					unsigned char w = k + ((n & 2) ? 1 : 0);
					vtr[n].SetPosition(m_river[w].pos[m & 1]);
					vtr[n].tv = m_river[w].u[i] + m_side[i].scroll;
					if (w == 0 || w == m_river.size() - 1)
						vtr[n].diffuse = 0xffffff;
					else
						vtr[n].diffuse = 0xffffffff;
				}

				g_view->DrawPolygonFan(vtx,
					(m_side[i].texHandle & 0x7ff) | 0x10c00000, 0x200, false);
			}
		}
	}
}

void ntRiver_Lava::Setup(const std::vector<WVector>& points, const char* name)
{
	ReadAttribute(name);

	float step[256] = { 0.0f };

	unsigned int i;
	for (i = 0; i < points.size() - 1; i++)
	{
		WVector seg = points[i + 1] - points[i];
		seg.y = 0.0f;
		WVector n = WCrossProduct(seg, WVector::UNIT_POS_Y).Normalize();

		float depth = m_side[0].depth;
		if (depth < m_side[1].depth)
			depth = m_side[1].depth;
		float limit = PVS().GetLowestGround(points[i]) + depth;

		step[i] += 6.4f;
		float ground = PVS().GetLowestGround(points[i + 1] + step[i] * n);
		bool bCapped = false;
		while (ground <= limit)
		{
			if (step[i] > 160.0f)
			{
				bCapped = true;
				break;
			}
			step[i] += 6.4f;
			ground = PVS().GetLowestGround(points[i + 1] + step[i] * n);
		}

		if (!bCapped)
			step[i] += 3.2f;
	}

	m_river.reserve(50);

	unsigned int count = 0;
	for (i = 0; i < points.size() - 1; i++)
	{
		WVector seg = points[i + 1] - points[i];
		seg.y = 0.0f;
		WVector n = WCrossProduct(seg, WVector::UNIT_POS_Y).Normalize();
		WVector off = step[i] * n;

		float g = PVS().GetLowestGround(points[i]);
		float d = m_side[0].depth;

		sRiver r;
		for (unsigned int j = 0; j < 2; j++)
		{
			float sign = (j & 1) ? 1.0f : -1.0f;
			r.pos[j] = points[i] + sign * off;
			r.pos[j].y = g + d;
			if (count == 0)
			{
				r.u[j] = 0.0f;
			}
			else
			{
				const sRiver& last = m_river[count - 1];
				r.u[j] = (r.pos[j] - last.pos[j]).Magnitude() / step[i] *
						m_side[j].tileHead +
					last.u[j];
			}
		}

		if (count != 0)
		{
			const sRiver& last = m_river[count - 1];
			Waabb box;
			box.max.x = max(max(r.pos[0].x, r.pos[1].x),
				max(last.pos[0].x, last.pos[1].x));
			box.max.y = min(last.pos[0].y, r.pos[0].y);
			box.max.z = max(max(r.pos[0].z, r.pos[1].z),
				max(last.pos[0].z, last.pos[1].z));
			box.min.x = min(min(r.pos[0].x, r.pos[1].x),
				min(last.pos[0].x, last.pos[1].x));
			box.min.y = box.max.y - max(m_side[1].depth, m_side[0].depth);
			box.min.z = min(min(r.pos[0].z, r.pos[1].z),
				min(last.pos[0].z, last.pos[1].z));
			GetPVS().AddWater(box, 2);
		}

		m_river.push_back(r);
		count++;
	}
}

CRiver_ex::CRiver_ex()
{
	for (int i = 0; i < 2; i++)
	{
		m_side[i].scroll = 0.0f;
		m_side[i].scrollVel = 0.0f;
		m_side[i].texHandle = 0;
		m_side[i].tileHead = 0.0f;
		m_side[i].tileSide = 0.0f;
		m_side[i].depth = 0.0f;
		m_side[i].bActive = false;
	}
}

CRiver_ex::~CRiver_ex()
{
	for (int i = 0; i < 2; i++)
	{
		if (g_resrcmng && m_side[i].texHandle)
		{
			g_resrcmng->Release(m_side[i].texHandle);
			m_side[i].texHandle = 0;
		}
	}
}

void CRiver_ex::ReadAttribute(const char* name)
{
	int index[2] = { -1, -1 };

	if (strstr(name, "_"))
	{
		m_side[0].bActive = sscanf(strchr(name, '_') + 1, "%d", &index[0]) != 0;
		int n = sscanf(strrchr(name, '_') + 1, "%d", &index[1]);
		if (n == 0 || index[1] == index[0])
			m_side[1].bActive = false;
		else
			m_side[1].bActive = true;

		for (int i = 0; i < 2; i++)
		{
			if (m_side[i].bActive)
			{
				m_side[i].scroll = Random(0.0f, 1.0f);
				m_side[i].scrollVel = RIVERINFO().GetScrollVelocity(index[i]);
				m_side[i].tileHead = RIVERINFO().GetHeadTile(index[i]);
				m_side[i].tileSide = RIVERINFO().GetSideTile(index[i]);
				m_side[i].depth = RIVERINFO().GetDepth(index[i]) * 3.2f;
				m_side[i].texHandle = g_resrcmng->LoadTexture(
					MakeStr("[%s_river_%s.jpg",
						GetCurMapName(),
						i == 0 ? "up" : "down"),
					0, 0, 0);
			}
		}
		return;
	}

	if (strstr(name, "*d"))
	{
		float depth = 0.0f;
		if (sscanf(strchr(name, 'd') + 1, "%f", &depth) == -1)
			depth = 1.0f;

		float speed = 3.0f;
		if (strstr(name, "*v"))
		{
			WSliceStr slice(name);
			if (!slice.Get("*v", "%f", &speed))
				speed = 1.0f;
		}

		for (int i = 0; i < 2; i++)
		{
			m_side[i].scroll = Random(0.0f, 1.0f);
			m_side[i].scrollVel = speed;
			m_side[i].tileHead = 1.0f;
			m_side[i].tileSide = 1.0f;
			m_side[i].depth = depth;
			m_side[i].texHandle = g_resrcmng->LoadTexture(
				MakeStr("[%s_river_%s.jpg",
					GetCurMapName(),
					i == 0 ? "up" : "down"),
				0, 0, 0);
			m_side[i].bActive = true;
		}
	}
	else
	{
		float speed = 3.0f;
		if (strstr(name, "*v"))
		{
			WSliceStr slice(name);
			if (!slice.Get("*v", "%f", &speed))
				speed = 1.0f;
		}

		for (int i = 0; i < 2; i++)
		{
			m_side[i].scroll = Random(0.0f, 1.0f);
			m_side[i].scrollVel = speed;
			m_side[i].tileHead = 1.0f;
			m_side[i].tileSide = 1.0f;

			const char* format;
			if (GetCurMap() == 11)
			{
				m_side[i].depth = 10.56f;
				format = "[pink_river_%s.jpg";
			}
			else if (GetCurMap() == 15)
			{
				m_side[i].depth = 3.2f;
				format = "[gs_river_%s.jpg";
			}
			else if (GetCurMap() == 16)
			{
				m_side[i].depth = 3.2f;
				format = "[panda_river_%s.jpg";
			}
			else
			{
				m_side[i].depth = 3.2f;
				if (GetCurMap() == 20)
					format = "[abbot_river_%s.jpg";
				else
					format = "[wind_river_%s.jpg";
			}

			m_side[i].texHandle = g_resrcmng->LoadTexture(
				MakeStr(format, i == 0 ? "up" : "down"), 0, 0, 0);
			m_side[i].bActive = true;
		}
	}
}

void CRiver_ex::Setup(const std::vector<WVector>& points, const char* name)
{
	if (points.size() > 2)
	{
		ReadAttribute(name);
		m_river.reserve(50);

		unsigned int count = 0;
		for (unsigned int i = 0; i < points.size() - 1; i += 2)
		{
			sRiver r;
			for (unsigned int j = 0; j < 2; j++)
			{
				r.pos[j] = points[i + j];
				r.u[j] = 0.0f;
			}

			if (count != 0)
			{
				const sRiver& last = m_river[count - 1];
				float len = (r.pos[1] - r.pos[0]).Magnitude();
				if (len != 0.0f)
				{
					float inv = 1.0f / len;
					r.u[0] = (r.pos[0] - last.pos[0]).Magnitude() * inv *
							m_side[0].tileHead +
						last.u[0];
					r.u[1] = (r.pos[1] - last.pos[1]).Magnitude() * inv *
							m_side[1].tileHead +
						last.u[1];
				}

				Waabb box;
				box.max.x = max(max(r.pos[0].x, r.pos[1].x),
					max(last.pos[0].x, last.pos[1].x));
				box.max.y = max(max(r.pos[0].y, r.pos[1].y),
					max(last.pos[0].y, last.pos[1].y));
				box.max.z = max(max(r.pos[0].z, r.pos[1].z),
					max(last.pos[0].z, last.pos[1].z));
				box.min.x = min(min(r.pos[0].x, r.pos[1].x),
					min(last.pos[0].x, last.pos[1].x));
				box.min.y = min(min(r.pos[0].y, r.pos[1].y),
					min(last.pos[0].y, last.pos[1].y));
				box.min.z = min(min(r.pos[0].z, r.pos[1].z),
					min(last.pos[0].z, last.pos[1].z));

				if (abs(box.max.y - box.min.y) <= 8.0f)
				{
					GetPVS().AddWater(box, 2);
				}
				else
				{
					WVector cur = (r.pos[1] - r.pos[0]) * 0.25f;
					WVector prv = (last.pos[1] - last.pos[0]) * 0.25f;
					WVector a = r.pos[0];
					WVector b = last.pos[0];
					for (int s = 4; s != 0; s--)
					{
						WVector a2 = a + cur;
						WVector b2 = b + prv;
						Waabb sub;
						sub.max.x = max(max(a.x, a2.x), max(b.x, b2.x));
						sub.max.y = max(max(a.y, a2.y), max(b.y, b2.y));
						sub.max.z = max(max(a.z, a2.z), max(b.z, b2.z));
						sub.min.x = min(min(a.x, a2.x), min(b.x, b2.x));
						sub.min.y = min(min(a.y, a2.y), min(b.y, b2.y));
						sub.min.z = min(min(a.z, a2.z), min(b.z, b2.z));
						GetPVS().AddWater(sub, 2);
						a = a2;
						b = b2;
					}
				}
			}

			m_river.push_back(r);
			count++;
		}
	}
}

void CRiver_ex::Process(float dt)
{
	if (m_side[0].bActive)
		m_side[0].scroll -= dt * m_side[0].scrollVel;
	if (m_side[1].bActive)
		m_side[1].scroll -= dt * m_side[1].scrollVel;
}

void CRiver_ex::Render()
{
	unsigned int count = m_river.size();
	if (count > 2)
	{
		for (int i = 0; i < 2; i++)
		{
			if (m_side[i].bActive)
			{
				for (unsigned char j = 0; j < 4; j++)
					vtr[j].tu = (j & 2) ? 1.0f : 0.0f;

				for (unsigned char k = 0; k < count - 1; k++)
				{
					for (unsigned char n = 0; n < 4; n++)
					{
						unsigned char w = (n & 1) + k;
						vtr[n].SetPosition(m_river[w].pos[(n >> 1) & 1]);
						vtr[n].tv = m_river[w].u[i] + m_side[i].scroll;
						if ((k == 0 && !(n & 1)) ||
							(k > count - 3 && (n & 1)))
							vtr[n].diffuse = 0xffffff;
						else
							vtr[n].diffuse = 0xffffffff;
					}

					g_view->DrawPolygonFan(vtx,
						(m_side[i].texHandle & 0x7ff) | 0x10c00000, 0x200,
						false);
				}
			}
		}
	}
}

