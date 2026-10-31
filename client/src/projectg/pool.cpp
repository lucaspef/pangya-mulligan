#include "minatl.h"
#include "projectg.h"
#include "wtextureview.h"
#include "quadtree.h"
#include "slicestr.h"
#include "clientsetting.h"
#include "pool.h"
#include "mathconsts.h"

extern WMatrix g_camera;

bool IsCurMapSnow();

float causticSpeed = 1.0f;
int causticAlpha = 255;

inline const char* GetCurMapName()
{
	if (Doc()->m_golfGame.pCourse)
		return Doc()->m_golfGame.pCourse->Data;
	return NULL;
}

CPool* CPool::CreatePoolByMapType(eMapType type)
{
	switch (type)
	{
	case MAP_ICE_CANNON:
	case MAP_PINK_WIND:
	case MAP_DEEP_INFERNO:
	case MAP_ICE_SPA:
	case MAP_LOST_SEAWAY:
	case MAP_EASTERN_VALLEY:
	case MAP_MAP_17:
	case MAP_MAP_18:	// 983
	case MAP_WIZ_CITY:
		return new CPoolCaustic;
	default:
		return new CPoolReflective;
	}
}

CPool::CPool()
	: m_type(0)
{
	m_area.Reset();
}

CPool::~CPool()
{
}

void CPool::SetArea(const Waabb& area, const char* option)
{
	Waabb& box = m_area;
	box.min = area.min;
	box.max = area.max;

	m_type = 0;
	WSliceStr(option).Get("*type", "%d", &m_type);
	if (strstr(option, "*noripple"))
		m_type = 1;

	switch (m_type)
	{
	case 1:
		m_flags.Enable(3);
		break;
	case 2:
		m_flags.Enable(0xd);
		break;
	}

	if (!m_flags.GetFlag(4))
		GetPVS().AddWater(m_area, 3);

	BuildMesh();
}

CPoolCaustic::CPoolCaustic()
{
	m_layerNum = 0;
}

CPoolCaustic::~CPoolCaustic()
{
}

void CPoolCaustic::Process(float delta)
{
	WView* view = g_view;
	if (view->InFrustum(m_area) && view->InFrustum(m_box))
	{
		float speed = 1.0f;
		if (GetCurMap() == 13)
			speed = 0.7f;
		else if (GetCurMap() == 14 && m_type == 2)
			speed = 0.5f;

		for (int i = 0; i < m_layerNum; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_layer[i].vertex[j].tu += speed * m_layer[i].speed.x * delta;
				m_layer[i].vertex[j].tv += delta * (speed * m_layer[i].speed.y);
			}
		}
	}
}

void CPoolCaustic::Render(WView* view)
{
	if (view->InFrustum(m_area) && view->InFrustum(m_box))
	{
		for (int i = 0; i < m_layerNum; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_layer[i].vertex[j].SetPosition(m_corner[j]);
				m_layer[i].vertex[j].diffuse =
					(m_layer[i].vertex[j].diffuse & 0xffffff) |
					(causticAlpha << 24);
			}

			unsigned long flag = 0x800000;
			if (GetCurMap() == 14)
			{
				if (i < 2)
					flag = 0;
			}

			view->DrawPolygonFan(m_layer[i].ppVertex,
				(m_layer[i].texHandle & 0x7ff) | flag | 0x400000, 0x200,
				false);
		}
	}
}

void CPoolCaustic::BuildMesh()
{
	m_box = m_area;
	m_box.max.y -= 1.0f;
	m_box.min.y = m_box.max.y - 1.0f;

	m_corner[0] = WVector(m_box.max.x, m_box.max.y, m_box.max.z);
	m_corner[1] = WVector(m_box.max.x, m_box.max.y, m_box.min.z);
	m_corner[2] = WVector(m_box.min.x, m_box.max.y, m_box.min.z);
	m_corner[3] = WVector(m_box.min.x, m_box.max.y, m_box.max.z);

	float scale = 1.3f;
	if (GetCurMap() == 14 && m_type == 2)
		scale = 0.325f;

	float rad = 30.0f * g_DEGTORAD;
	float speedV = sinf(rad) * -0.02f;
	float speedU = cosf(rad) * 0.02f;

	AddLayer(-speedU, speedV, scale * 200.0f, 255);
	AddLayer(speedU, speedV, scale * 100.0f, 255);
	AddLayer(0.0f, 0.02f, scale * 150.0f, 255);
}

void CPoolCaustic::AddLayer(float speedU, float speedV, float scale,
	unsigned char alpha)
{
	if (m_layerNum >= 3)
		return;

	sLayer& layer = m_layer[m_layerNum];
	layer.speed.x = speedU;
	layer.speed.y = speedV;

	int index = Min(m_layerNum + 1, 2);
	const char* name = GetCurMapName();
	if (stricmp(name, "pola") == 0 && m_type == 0)
		name = "icee";

	layer.texHandle = g_resrcmng->LoadTexture(
		MakeStr("[caustic_%s%02d.jpg", name, index), 0, 0, 0);
	if (!layer.texHandle)
		layer.texHandle = g_resrcmng->LoadTexture(
			MakeStr("[caustic_%02d.jpg", index), 0, 0, 0);

	float inv = 1.0f / scale;
	int j;
	for (j = 0; j < 4; j++)
	{
		layer.vertex[j].tu = (m_corner[j].x - m_box.min.x) * inv;
		layer.vertex[j].tv = (m_corner[j].z - m_box.min.z) * inv;
		layer.vertex[j].diffuse = (alpha << 24) | g_lightset.ambient2;
		layer.ppVertex[j] = &layer.vertex[j];
	}
	layer.ppVertex[j] = NULL;

	m_layerNum++;
}

CPoolReflective::CPoolReflective()
	: m_ppVertex(NULL), m_pTexView(NULL)
{
	m_ppVtx[0] = &m_vtx[0];
	m_ppVtx[1] = &m_vtx[1];
	m_ppVtx[2] = &m_vtx[3];
	m_ppVtx[3] = &m_vtx[2];
	m_ppVtx[4] = NULL;
}

CPoolReflective::~CPoolReflective()
{
	if (m_ppVertex)
	{
		for (int i = 0; i < m_height; i++)
		{
			if (m_ppVertex[i])
			{
				delete[] m_ppVertex[i];
				m_ppVertex[i] = NULL;
			}
		}
		if (m_ppVertex)
		{
			delete[] m_ppVertex;
			m_ppVertex = NULL;
		}
	}

	if (m_pTexView)
	{
		delete m_pTexView;
		m_pTexView = NULL;
	}
}

void CPoolReflective::SetReflection(bool bEnable)
{
	if (bEnable)
	{
		if (m_pTexView == NULL)
		{
			m_pTexView = new WTextureView;
			m_pTexView->SetResourceManager(g_resrcmng);
			m_pTexView->SetViewport(g_view->SCREEN_XS * 0.5f,
				g_view->SCREEN_YS * 0.5f);
			m_pTexView->SetReflective(true);

			WTextureView::TextureParam param;
			param.m_numRts = 1;
			param.m_rtTexInfo[0].Set(0x14000,
				WRenderToTextureSizeInfo::SIZE_HALF, false, 0);
			param.m_depthSurfInfo.Set(
				WRenderToTextureParam::DepthSurfInfo::SurfaceUsage(2),
				WRenderToTextureSizeInfo::SIZE_HALF, true, 1.0f);
			m_pTexView->SetTextureParam(param);
		}
	}
	else
	{
		if (m_pTexView)
		{
			delete m_pTexView;
			m_pTexView = NULL;
		}
	}
}

void CPoolReflective::Process(float delta)
{
	if (!g_view->InFrustum(m_area))
		return;

	if (!m_flags.GetFlag(1))
	{
		if (g_CurrentTime - m_dropTime > 1000)
		{
			m_dropTime = g_CurrentTime;
			Droplet();
		}
	}

	if (!m_flags.GetFlag(2))
	{
		MakeNorm();
		Wave(delta);
	}
}

void CPoolReflective::Prerender(WView* view)
{
	if (view->GetCamera().pivot.y < m_area.max.y || !view->InFrustum(m_area))
		return;

	if (!m_pTexView || !CSceneManager::m_pInstance)
		return;

	WVideoDev* dev = m_pTexView->GetVideoDevice();
	m_pTexView->SetFOV(view->GetFOV()); // 983 006fb43c: GetFOV de verdade

	WMatrix cam = view->GetCamera() *
		~WMatrix(WPlane(WVector::UNIT_POS_Y, -m_area.max.y));
	m_pTexView->SetClip(0.1f, view->clip_far, false);
	m_pTexView->SetCamera(cam);
	m_pTexView->UpdateCamera();

	WView* oldView = g_view;
	WMatrix oldCamera = g_camera;
	g_view = m_pTexView;
	g_camera = m_pTexView->GetCamera();

	if (m_pTexView->Begin()) // 983 006fb51b: so desenha e fecha se abriu
	{
		CSceneManager::m_pInstance->Render(m_pTexView, 0x80);

		// D904: 983 006fb3e0 Render(0x10d) com o plano de corte; flag 8 -> so as nuvens (0x100)
		if (!m_flags.GetFlag(8))
		{
			WPlane plane(WVector::UNIT_POS_Y, 1.0f - m_area.max.y);
			dev->BeginUsingCustomRenderState();
			dev->SetCustomRenderState(WVDRENDERSTATETYPE(0x98), 1);
			dev->SetCustomClipPlane(0, plane);
			CSceneManager::m_pInstance->Render(m_pTexView, 0x10d);
			dev->EndUsingCustomRenderState();
		}
		else
			CSceneManager::m_pInstance->Render(m_pTexView, 0x100);

		m_pTexView->End();
	}

	g_view = oldView;
	g_camera = oldCamera;
}

// HACK: normal * half, as the second operand of the max() that clamps the
// specular term in CPoolReflective::Render. The original loads only the x
// product of that copy in the other order (fld half.x; fmul normal.x); with
// operator*(WVector, WVector) on both sides of the max() all six products
// come out the same way. Passing the normal twice reproduces it.
// dllimport + __forceinline: always inlined and never emitted out of line
// (a plain inline would add a COMDAT the original does not have).
__declspec(dllimport) __forceinline float DotHalf(const WVector& normal,
	const WVector& normalX, const WVector& half)
{
	return normalX.x * half.x + normal.y * half.y + normal.z * half.z;
}

void CPoolReflective::Render(WView* view)
{
	if (view->GetCamera().pivot.y < m_area.max.y || !view->InFrustum(m_area))
		return;

	int tex = m_pTexView ? m_pTexView->GetTexture(0) : 0;
	WVideoDev* dev = view->GetVideoDevice();
	if (tex)
	{
		dev->BeginUsingCustomRenderState();
		WMatrix4 m = view->xGetViewState().xmView *
			view->xGetViewState().xmProj * WView::ms_uvConvMat;
		dev->SetCustomFxMacro(0x80000); // 983 006fb6ae: PROJ_TEXCOORD; 0x10000 do 645 aqui e OVERLAY
		dev->SetCustomFxParamMatrix(WFxParameterType(0x12), m);
	}

	unsigned char alpha;
	if (IsCurMapSnow() || GetCurMap() == 10)
		alpha = 40;
	else if (m_type == 2)
		alpha = 80;
	else
		alpha = 160;

	float d = max(-view->GetCamera().za * WVector::UNIT_POS_Y, 0.0f);
	float fresnel = 1.0f - d;
	float sun;

	if (m_flags.GetFlag(2))
	{
		WVector pos;
		pos.y = m_area.max.y;
		sun = max(-g_lightset.nearOne * WVector::UNIT_POS_Y, 0.0f);
		float spec = max(WVector::UNIT_POS_Y *
				((view->GetCamera().pivot - m_area.GetCenter()).Normalize() -
					g_lightset.nearOne)
					.Normalize(),
			0.0f);
		spec *= spec;

		unsigned long color =
			AddDiffuse(AddDiffuse(AddDiffuse(0, g_lightset.ambient, 0x66),
						   g_lightset.diffuse, sun * 102.0f),
				0x808080, spec * 255.0f);
		color |= (unsigned char)(((1.0f - fresnel) * 0.5f + fresnel) * alpha)
			<< 24;

		for (int k = 0; k < 4; k++)
		{
			pos.x = ((k >> 1) & 1) ? m_area.max.x : m_area.min.x;
			pos.z = (k & 1) ? m_area.max.z : m_area.min.z;
			m_vtx[k].SetPosition(pos);
			m_vtx[k].diffuse = color;
		}

		view->DrawPolygonFan(m_ppVtx, (tex & 0x7ff) | 0x16400000, 0, false);
	}
	else
	{
		for (int i = 0; i < m_height - 1; i++)
		{
			for (int j = 0; j < m_width - 1; j++)
			{
				int count = 0;
				for (int k = 0; k < 4; k++)
				{
					sWaveVertex* v =
						&m_ppVertex[i + (k & 1)][j + ((unsigned)k >> 1 & 1)];
					if (!v->active)
						count++;

					m_vtx[k].SetPosition(v->pos);

					sun = max(-g_lightset.nearOne * v->normal, 0.0f);
					// max(half * v->normal, 0.0f), see DotHalf
					float spec =
						(((view->GetCamera().pivot - v->pos).Normalize() -
							 g_lightset.nearOne)
									.Normalize() *
								v->normal >
							0.0f)
						? DotHalf(v->normal, v->normal,
							  ((view->GetCamera().pivot - v->pos).Normalize() -
								  g_lightset.nearOne)
								  .Normalize())
						: 0.0f;
					spec *= spec;

					unsigned long color = AddDiffuse(
						AddDiffuse(AddDiffuse(0, g_lightset.ambient, 0x66),
							g_lightset.diffuse, sun * 102.0f),
						0x808080, spec * 255.0f);
					m_vtx[k].diffuse = color |
						((unsigned char)(((1.0f - fresnel) * 0.5f + fresnel) *
							 alpha)
							<< 24);
				}

				if (count != 4)
					view->DrawPolygonFan(m_ppVtx, (tex & 0x7ff) | 0x16400000, 0,
						false);
			}
		}
	}

	if (tex)
		dev->EndUsingCustomRenderState();
}

void CPoolReflective::BuildMesh()
{
	if (!m_flags.GetFlag(2))
	{
		m_width = (int)ceilf((m_area.max.x - m_area.min.x) / 20.0f) + 1;
		m_height = (int)ceilf((m_area.max.z - m_area.min.z) / 20.0f) + 1;

		float dx = (m_area.max.x - m_area.min.x) / (m_width - 1);
		float dz = (m_area.max.z - m_area.min.z) / (m_height - 1);

		m_ppVertex = new sWaveVertex*[m_height];
		for (int i = 0; i < m_height; i++)
			m_ppVertex[i] = new sWaveVertex[m_width];

		for (int i = 0; i < m_height; i++)
		{
			for (int j = 0; j < m_width; j++)
			{
				m_ppVertex[i][j].pos.x = j * dx + m_area.min.x;
				m_ppVertex[i][j].pos.y = m_area.max.y;
				m_ppVertex[i][j].pos.z = i * dz + m_area.min.z;

				m_ppVertex[i][j].force = 0.0f;
				m_ppVertex[i][j].velocity = 0.0f;

				m_ppVertex[i][j].u = 0.5f;
				m_ppVertex[i][j].v = 0.5f;

				m_ppVertex[i][j].normal = WVector::UNIT_POS_Y;
				m_ppVertex[i][j].active = false;
			}
		}

		m_dropTime = g_CurrentTime;
		ExcludePoint();
	}

	SetReflection(COption::m_pInstance->vIsReflectionEnabled());
}

void CPoolReflective::Droplet()
{
	int x = rand() % (m_width - 1);
	int y = rand() % (m_height - 1);

	m_ppVertex[y][x].velocity += 2.0f;
	m_ppVertex[y][x + 1].velocity += 2.0f;
	m_ppVertex[y + 1][x].velocity += 2.0f;
	m_ppVertex[y + 1][x + 1].velocity += 2.0f;
}

void CPoolReflective::MakeNorm()
{
	for (int i = 0; i < m_height; i++)
	{
		for (int j = 0; j < m_width; j++)
		{
			sWaveVertex* v = &m_ppVertex[i][j];
			if (!v->active)
				continue;

			int lastX = m_width - 1, lastY = m_height - 1;
			WVector a, b;
			if (i == lastY)
			{
				int nx = (j == lastX) ? j - 1 : j + 1;
				a = m_ppVertex[i][nx].pos - m_ppVertex[i][j].pos;
				b = m_ppVertex[i - 1][j].pos - m_ppVertex[i][j].pos;
			}
			else if (j == lastX)
			{
				a = m_ppVertex[i][j - 1].pos - m_ppVertex[i][j].pos;
				int ny = (i + 1 == lastY) ? i - 1 : i + 1;
				b = m_ppVertex[ny][j].pos - m_ppVertex[i][j].pos;
			}
			else
			{
				a = m_ppVertex[i][j + 1].pos - m_ppVertex[i][j].pos;
				b = m_ppVertex[i + 1][j].pos - m_ppVertex[i][j].pos;
			}

			WVector n;
			n = WCrossProduct(b, a);
			n.Normalize();
			v->normal = n;
			m_ppVertex[i][j].u = (n.x + 1.0f) * 0.5f;
			m_ppVertex[i][j].v = (n.z + 1.0f) * 0.5f;
		}
	}
}

void CPoolReflective::Wave(float delta)
{
	int i, j;
	for (i = 0; i < m_height; i++)
		for (j = 0; j < m_width; j++)
			m_ppVertex[i][j].force = 0.0f;

	for (i = 1; i < m_height - 1; i++)
	{
		for (j = 1; j < m_width - 1; j++)
		{
			if (m_ppVertex[i][j].active)
			{
				float d;

				d = m_ppVertex[i][j].pos.y - m_ppVertex[i][j - 1].pos.y;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i][j - 1].force += d;

				d = 0.0f;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i - 1][j].force += d;

				d = m_ppVertex[i][j].pos.y - m_ppVertex[i][j + 1].pos.y;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i][j + 1].force += d;

				d = m_ppVertex[i][j].pos.y - m_ppVertex[i + 1][j].pos.y;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i + 1][j].force += d;

				d = (m_ppVertex[i][j].pos.y - m_ppVertex[i + 1][j + 1].pos.y) *
					0.70710677f;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i + 1][j + 1].force += d;

				d = (m_ppVertex[i][j].pos.y - m_ppVertex[i - 1][j - 1].pos.y) *
					0.70710677f;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i - 1][j - 1].force += d;

				d = (m_ppVertex[i][j].pos.y - m_ppVertex[i + 1][j - 1].pos.y) *
					0.70710677f;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i + 1][j - 1].force += d;

				d = (m_ppVertex[i][j].pos.y - m_ppVertex[i + 1][j - 1].pos.y) *
					0.70710677f;
				m_ppVertex[i][j].force -= d;
				m_ppVertex[i + 1][j - 1].force += d;
			}
		}
	}

	for (i = 0; i < m_height; i++)
	{
		for (j = 0; j < m_width; j++)
		{
			if (m_ppVertex[i][j].active)
			{
				m_ppVertex[i][j].velocity += delta * m_ppVertex[i][j].force;
				m_ppVertex[i][j].pos.y = Max(m_area.max.y - 2.0f,
					m_ppVertex[i][j].pos.y + delta * m_ppVertex[i][j].velocity);
			}
		}
	}
}

void CPoolReflective::ExcludePoint()
{
	for (int i = 0; i < m_height; i++)
	{
		for (int j = 0; j < m_width; j++)
		{
			int index;
			WVector p;
			p = GetPVS().GetGroundPoint(m_ppVertex[i][j].pos, true, &index,
				false);
			if (index >= 0 && p.y < m_area.max.y &&
				GetPVS().GetMeshNormal(index).y > 0.0f)
				m_ppVertex[i][j].active = true;
		}
	}
}

ulong __fastcall AddDiffuse(ulong left, ulong right, unsigned char value)
{
	ulong blue =
		Min<ulong>(0xff, (left & 0xff) + (((right & 0xff) * value) >> 8));
	ulong green = Min<ulong>(0xff,
		(left >> 8 & 0xff) + (((right >> 8 & 0xff) * value) >> 8));
	ulong red = Min<ulong>(0xff,
		(left >> 16 & 0xff) + (((right >> 16 & 0xff) * value) >> 8));
	return (red << 16) | (green << 8) | blue;
}
