#include "minatl.h"
#include "projectg.h"
#include "sea.h"
#include "quadtree.h"

extern bool __fastcall WIsClockwise(const std::vector<WVector>& poly, int direct);
extern bool __fastcall WIsFrontFace(const WVector& p0, const WVector& p1,
	const WVector& p2, int direct);
extern bool __fastcall WIsInTriangle(const WVector& p, const WVector& a,
	const WVector& b, const WVector& c, int direct);

CSea::CSea()
	: m_layerNum(0), m_scale(1.0f)
{
}

CSea::~CSea()
{
}

// 983 (00701d30): no mapa 0x14 a textura e [sea20_0N, senao [sea_0N; N = Min(camada, 2)
const char* CSea::TexName(int layer)
{
	if (GetCurMap() == 0x14)
		return MakeStr("[sea20_0%d.jpg", Min(layer, 2));
	return MakeStr("[sea_0%d.jpg", Min(layer, 2));
}

void CSea::LoadTexture()
{
	for (int i = 0; i < m_layerNum; i++)
	{
		sLayer& layer = m_layer[i];
		if (g_resrcmng && layer.texHandle)
		{
			g_resrcmng->Release(layer.texHandle);
			layer.texHandle = 0;
		}
		layer.texHandle = g_resrcmng->LoadTexture(TexName(i + 1), 0, 0, 0);
	}
}

// 983 (00703c60): recorte de orelhas; escreve em out os indices dos triangulos
// (3 por orelha) e devolve false (out vazio) se nao fechar.
static bool __fastcall Triangulate(const std::vector<WVector>& pts, int direct,
	std::vector<unsigned short>* out)
{
	struct sTri { unsigned short v[3]; };
	struct sVertex { unsigned short idx; bool used; };

	const unsigned n = pts.size();
	if (n < 3)
		return false;

	bool clockwise = WIsClockwise(pts, direct);
	std::vector<sVertex> vtx(n);
	for (unsigned i = 0; i < n; i++)
	{
		vtx[i].idx = (unsigned short)i;
		vtx[i].used = false;
	}

	std::vector<sTri> rejected;
	unsigned remain = n;
	unsigned cur = 0;
	while (cur < n)
	{
		if (vtx[cur].used)
		{
			cur++;
			continue;
		}

		// os dois proximos vertices vivos (circular)
		sTri tri;
		tri.v[0] = (unsigned short)cur;
		int k = 1;
		unsigned lim = n + 1;
		for (unsigned j = cur + 1; j < lim && k < 3; j++)
		{
			unsigned idx = j % n;
			if (!vtx[idx].used)
			{
				tri.v[k++] = (unsigned short)idx;
				lim++;
			}
		}
		if (k < 3)
		{
			out->clear();
			return false;
		}
		if (!clockwise)
			std::swap(tri.v[1], tri.v[2]);

		bool seen = false;
		for (unsigned r = 0; r < rejected.size() && !seen; r++)
			seen = !memcmp(&rejected[r], &tri, sizeof(tri));
		if (seen)
		{
			cur++;
			continue;
		}

		const WVector& p0 = pts[tri.v[0]];
		const WVector& p1 = pts[tri.v[1]];
		const WVector& p2 = pts[tri.v[2]];
		if (!WIsFrontFace(p0, p1, p2, direct))
		{
			rejected.push_back(tri);
			cur++;
			continue;
		}

		bool ear = true;
		for (unsigned s = 0; s < n && ear; s++)
		{
			if (!vtx[s].used && s != tri.v[0] && s != tri.v[1] && s != tri.v[2] &&
				WIsInTriangle(pts[s], p0, p1, p2, direct))
				ear = false;
		}
		if (!ear)
		{
			cur++;
			continue;
		}

		out->push_back(tri.v[0]);
		out->push_back(tri.v[1]);
		out->push_back(tri.v[2]);
		vtx[clockwise ? tri.v[1] : tri.v[2]].used = true;
		if (--remain < 3)
			return true;
		cur = 0;
	}

	out->clear();
	return false;
}

void CSea::SetArea(const std::vector<WVector>& area)
{
	m_point = area;
	m_area = Waabb(WVector(FLT_MAX, FLT_MAX, FLT_MAX),
		WVector(-FLT_MAX, -FLT_MAX, -FLT_MAX));

	const unsigned n = m_point.size();
	for (unsigned i = 0; i < n; i++)
	{
		m_point[i].y = area[0].y;
		m_area.AddPoint(m_point[i]);
	}

	m_area.max.y -= 1.0f;
	m_area.min.y = m_area.max.y - 1.0f;
	for (unsigned i = 0; i < n; i++)
		m_point[i].y = m_area.max.y;

	Triangulate(m_point, 1, &m_index);

	for (unsigned j = 0; j < n / 2; j++)
	{
		const WVector& a = m_point[j];
		const WVector& b = m_point[n - 1 - j];
		const WVector& c = m_point[j + 1];
		const WVector& d = m_point[n - 2 - j];

		Waabb box;
		box.max.x = max(max(a.x, b.x), max(c.x, d.x));
		box.max.y = m_area.max.y;
		box.max.z = max(max(a.z, b.z), max(c.z, d.z));
		box.min.x = min(min(a.x, b.x), min(c.x, d.x));
		box.min.y = m_area.min.y;
		box.min.z = min(min(a.z, b.z), min(c.z, d.z));

		GetPVS().AddWater(box, 1);
	}

	Init();
}

void CSea::Init()
{
	float rad = 30.0f * g_DEGTORAD;
	float speedV = sinf(rad) * -0.02f;
	float speedU = cosf(rad) * 0.02f;

	AddLayer(-speedU, speedV, 200.0f, 255);
	AddLayer(speedU, speedV, 100.0f, 255);
	AddLayer(0.0f, 0.02f, 150.0f, 255);
}

void CSea::AddLayer(float speedU, float speedV, float scale,
	unsigned char alpha)
{
	sLayer& layer = m_layer[m_layerNum++];
	const unsigned n = m_point.size();

	layer.speed.x = speedU;
	layer.speed.y = speedV;
	layer.color = g_lightset.ambient2 | (alpha << 24);
	layer.uv.resize(n);
	layer.vertex.resize(n);
	layer.texHandle = g_resrcmng->LoadTexture(TexName(m_layerNum), 0, 0, 0);

	for (unsigned i = 0; i < n; i++)
	{
		layer.vertex[i].SetPosition(m_point[i]);
		layer.uv[i].x = (m_point[i].x - m_area.min.x) / scale;
		layer.uv[i].y = (m_point[i].z - m_area.min.z) / scale;
		layer.vertex[i].diffuse = layer.color;
	}
}

void CSea::Process(float delta)
{
	for (int i = 0; i < m_layerNum; i++)
	{
		sLayer& layer = m_layer[i];
		for (unsigned j = 0; j < m_point.size(); j++)
			layer.uv[j] += WVector2D(delta * layer.speed.x, delta * layer.speed.y);
	}
}

void CSea::Render()
{
	if (!g_view->InFrustum(m_area) || m_index.size() < 3)
		return;

	for (int i = 0; i < m_layerNum; i++)
	{
		sLayer& layer = m_layer[i];
		for (unsigned j = 0; j < m_point.size(); j++)
		{
			layer.vertex[j].tu = layer.uv[j].x;
			layer.vertex[j].tv = layer.uv[j].y;
		}

		g_view->DrawIndexedTriangles(&layer.vertex[0], layer.vertex.size(),
			&m_index[0], m_index.size(),
			(layer.texHandle & 0x7ff) | 0x20800000, 0, 1, 0);
	}
}
