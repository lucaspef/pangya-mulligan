#define REBANG_FXSEQUENCE_CPP
#include "minatl.h"
#include "projectg.h"
#include "fx.h"
#include "fxbox.h"
#include "wind.h"
#include "lensflare.h"

#include "sky.h"

extern WResourceManager* g_resrcmng;
extern WMatrix g_camera;

unsigned char GetCurMap();
unsigned long ModifyColor(unsigned long color, float factor);

CSky::CSky()
	: m_pLensFlare(NULL), m_pStars(NULL), m_pMoons(NULL)
{
	m_cloudTex = 0;
	m_cloudNum = 0;
	m_visibleCloudNum = 0;
	m_cloudRadius = 1120.0f;
	m_cloudFar = 1600.0f;
	m_cloudHeight = 416.0f;
	m_color = 0xffffffff;
	m_cloudInterval = 0.0f;
	m_cloudTimer = 0.0f;
	memset(m_texFar, 0, sizeof(m_texFar));
	memset(m_texNear, 0, sizeof(m_texNear));
	memset(m_texUp, 0, sizeof(m_texUp));
	memset(m_texDn, 0, sizeof(m_texDn));
	m_bLoaded = 0;
	m_upRot = 0.0f;
	m_nearRot = 0.0f;

	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		CalcClipFarByView(NULL);
}

CSky::~CSky()
{
	Clear();
}

void CSky::Clear()
{
	if (!m_bLoaded)
		return;

	int i;
	for (i = 0; i < 4; i++)
	{
		if (m_batch[i].xiFlag0 & 0x7ff)
		{
			g_resrcmng->VideoReference()->xReleaseVertexBuffer(
				(unsigned int)m_batch[i].xiFlag1 >> 22);
			g_resrcmng->VideoReference()->xReleaseIndexBuffer(
				(m_batch[i].xiFlag1 >> 13) & 0x1ff);
			m_batch[i].xiFlag1 = 0;
			m_batch[i].xiFlag0 = 0;
		}
	}

	for (i = 0; i < sizeof(m_texFar) / sizeof(int); i++)
	{
		if (g_resrcmng && m_texFar[i])
		{
			g_resrcmng->Release(m_texFar[i]);
			m_texFar[i] = 0;
		}
	}
	for (i = 0; i < sizeof(m_texNear) / sizeof(int); i++)
	{
		if (g_resrcmng && m_texNear[i])
		{
			g_resrcmng->Release(m_texNear[i]);
			m_texNear[i] = 0;
		}
	}
	for (i = 0; i < sizeof(m_texUp) / sizeof(int); i++)
	{
		if (g_resrcmng && m_texUp[i])
		{
			g_resrcmng->Release(m_texUp[i]);
			m_texUp[i] = 0;
		}
	}
	for (i = 0; i < sizeof(m_texDn) / sizeof(int); i++)
	{
		if (g_resrcmng && m_texDn[i])
		{
			g_resrcmng->Release(m_texDn[i]);
			m_texDn[i] = 0;
		}
	}

	ClearClouds();

	if (m_pLensFlare)
	{
		delete m_pLensFlare;
		m_pLensFlare = NULL;
	}
	if (m_pStars)
	{
		delete m_pStars;
		m_pStars = NULL;
	}
	if (m_pMoons)
	{
		delete m_pMoons;
		m_pMoons = NULL;
	}

	m_bLoaded = 0;
}

void CSky::LoadTexByWeather(const char* name, const char* type)
{
	char path[128] = "";
	sprintf(path, "%s_%s.jpg", name, type);
	int tex = g_resrcmng->LoadTexture(path, 0x80000000, 0, 0);
	if (!tex)
	{
		char path2[256] = "";
		sprintf(path2, "[%s", path);
		tex = g_resrcmng->LoadTexture(path2, 0x80000000, 0, 0);
	}

	if (tex)
	{
		if (!strcmp("far", type))
			m_texFar[0] = tex;
		else if (!strcmp("near", type))
			m_texNear[0] = tex;
		else if (!strcmp("up", type))
			m_texUp[0] = tex;
		else if (!strcmp("dn", type))
			m_texDn[0] = tex;
	}

	for (int i = tex != 0; i < 5; i++)
	{
		sprintf(path, "%s_%s_%d.jpg", name, type, i);
		tex = g_resrcmng->LoadTexture(path, 0x80000000, 0, 0);
		if (!strcmp("far", type))
			m_texFar[i] = tex;
		else if (!strcmp("near", type))
			m_texNear[i] = tex;
		else if (!strcmp("up", type))
		{
			m_texUp[i] = tex;
			return;
		}
		else if (!strcmp("dn", type))
		{
			m_texDn[i] = tex;
			return;
		}
	}
}

int CSky::IsThereTexHandle(const char* type, int index)
{
	int* p;
	if (!strcmp(type, "far"))
		p = &m_texFar[index];
	else if (!strcmp(type, "near"))
		p = &m_texNear[index];
	else if (!strcmp(type, "up"))
		p = index == 0 ? &m_texUp[0] : &m_texUp[1];
	else if (!strcmp(type, "dn"))
		p = index == 0 ? &m_texDn[0] : &m_texDn[1];
	else
		return -1;
	return *p;
}

// HACK: the naked LoadSky below is the only user of 500.0f and 1/32 in this
// object, but other (later) objects define the same __real@ COMDATs, so the
// constants must be compiler-generated here to keep the original's copy
// (and position). This specialization is referenced only from LoadSky's dead
// C copy: compiling it emits the two constants in order, while its own COMDAT
// loses to the Max<int> instance clientsetting.obj already defines.
template <>
inline int Max<int>(int a, int b)
{
	return (a < b) ? b : (int)(((float)a * 0.03125f + (float)b) * 500.0f);
}

// HACK: the original vertex loop keeps the locked buffer and the vertex
// offset in separate registers and spills x/z/u through memory; no C
// form tried reproduces its register allocation and stack layout. The C
// equivalent is in the #else. LoadSky is the only user of these float
// constants, so they are emitted as its function-local statics and
// aliased to the __real@ names the other objects reference.
// 983 00704bb0: number of times the first loaded texture of the set tiles around the cylinder
static int SkyTexRepeat(const int* tex)
{
	int n = 0;
	while (tex[n] == 0)
	{
		n++;
		if (n >= 5)
			return 1;
	}
	int w = g_resrcmng->GetTextureWidth(tex[n]);
	int h = g_resrcmng->GetTextureHeight(tex[n]);
	if (w <= 0 || h <= 0)
		return 1;
	float ratio = (float)w / h;
	int r = (int)(4.0 / ratio + (double)0.00001f);
	if (r < 1)
		return 1;
	return r;
}

void CSky::LoadSky(const char* name, float turn)
{
	int num[4];
	unsigned long color[3];
	float y[3];
	float tv[3];

	if (m_bLoaded)
		Clear();

	m_bLoaded = 1;

	LoadTexByWeather(name, "far");
	int tex = IsThereTexHandle("far", 0);
	if (tex)
		m_batch[0].xiFlag0 = (tex & 0x7ff) | 0x22300000;
	else
		m_batch[0].xiFlag0 = 0;

	LoadTexByWeather(name, "near");
	tex = IsThereTexHandle("near", 0);
	if (tex)
	{
		m_batch[1].xiFlag0 = (tex & 0x7ff) | 0x22300000;
	}
	else
	{
		m_batch[1].xiFlag0 = 0;
	}

	LoadTexByWeather(name, "up");
	tex = IsThereTexHandle("up", 0);
	if (tex)
		m_batch[2].xiFlag0 = (tex & 0x7ff) | 0x26300000;
	else
		m_batch[2].xiFlag0 = 0;

	LoadTexByWeather(name, "dn");
	tex = IsThereTexHandle("dn", 0);
	if (tex)
		m_batch[3].xiFlag0 = (tex & 0x7ff) | 0x26300000;
	else
		m_batch[3].xiFlag0 = 0;

	num[0] = SkyTexRepeat(m_texFar);
	num[1] = SkyTexRepeat(m_texNear);
	num[2] = 1;
	num[3] = 1;

	WVideoDev* video = g_resrcmng->VideoReference();

	for (int i = 0; i < 4; i++)
	{
		if (m_batch[i].xiFlag0 & 0x7ff)
		{
			m_batch[i].xiFlag1 = 5;
			unsigned long fvf = video->xDetermineFVF(m_batch[i].xiFlag0, 5, 0);
			unsigned long usage = video->xDetermineBufferUsage(fvf) | 8;
			int stride = video->VertexSize(fvf);

			// 983 0070557e: fixed 33 columns x 3 rings (far/near), or a single quad (up/dn)
			int vtxNum, idxNum;
			float radius = 0.0f, capY = 0.0f;
			if (i < 2)
			{
				vtxNum = 99;
				idxNum = 132;
				radius = (i == 0) ? 500.0f : 400.0f;
				y[0] = radius * (g_PI / 4);
				y[1] = radius * (g_PI / 5);
				y[2] = radius * -(g_PI / 5);
				tv[0] = 0.0f;
				tv[1] = 0.1f;
				tv[2] = 1.0f;
			}
			else
			{
				m_batch[i].xiFlag1 |= 0x200; // 983 007055ee (dupla face)
				vtxNum = 4;
				idxNum = 4;
				capY = (i == 3) ? -(250.0f * g_PI / 4) : (250.0f * g_PI / 4);
			}

			int vb = video->xCreateVertexBuffer(vtxNum, fvf, usage);
			int ib = video->xCreateIndexBuffer(idxNum, usage);

			unsigned char* pVtx = video->xLockVertexBuffer(vb, 0, 0);
			if (i < 2)
			{
				if (i == 0)
					color[0] = ModifyColor(m_color, GOLFDOC()->m_bright);
				else
					color[0] = ModifyColor(m_color, GOLFDOC()->m_bright) | 0xff000000;
				color[1] = color[0] | 0xff000000;
				color[2] = color[1];

				float numf = (float)num[i];
				for (int c = 0; c < 33; c++)
				{
					float angle = (float)c * (1.0f / 32.0f) * (float)(g_PI * 2.0) - turn;
					float x = (float)cos((double)angle) * radius;
					float z = (float)sin((double)angle) * radius;
					float u = (1.0f - (float)c * (1.0f / 32.0f)) * numf;
					for (int k = 0; k < 3; k++)
					{
						unsigned char* v = pVtx + (k * 33 + c) * stride;
						*(float*)(v + 0) = x;
						*(float*)(v + 4) = y[k];
						*(float*)(v + 8) = z;
						*(unsigned long*)(v + 12) = color[k];
						*(float*)(v + 16) = u;
						*(float*)(v + 20) = tv[k];
					}
				}
			}
			else
			{
				unsigned long col = ModifyColor(m_color, GOLFDOC()->m_bright) | 0xff000000;
				for (int j = 0; j < 4; j++)
				{
					unsigned char* v = pVtx + j * stride;
					*(float*)(v + 0) = (j & 1) ? 500.0f : -500.0f;
					*(float*)(v + 4) = capY;
					*(float*)(v + 8) = (j < 2) ? -500.0f : 500.0f;
					*(unsigned long*)(v + 12) = col;
					*(float*)(v + 16) = (float)(j & 1);
					*(float*)(v + 20) = (j < 2) ? 1.0f : 0.0f;
				}
			}
			video->xUnlockVertexBuffer(vb);

			unsigned short* pIdx =
				(unsigned short*)video->xLockIndexBuffer(ib, 0, 0);
			if (i < 2)
			{
				int k = 0;
				for (int s = 0; s < 2; s++)
				{
					for (int c = 0; c < 33; c++)
					{
						pIdx[k] = (unsigned short)(s * 33 + c);
						pIdx[k + 1] = (unsigned short)(s * 33 + c + 33);
						k += 2;
					}
				}
			}
			else
			{
				for (int k = 0; k < idxNum; k++)
					pIdx[k] = (unsigned short)k;
			}
			video->xUnlockIndexBuffer(ib);

			m_batch[i].xnmTransfs = 0;
			m_batch[i].xiBaseVtxIdx = 0;
			m_batch[i].xnVtxs = vtxNum;
			m_batch[i].xiBaseIdxIdx = 0;
			m_batch[i].xnIdxs = idxNum;
			m_batch[i].xiFlag1 |= ((ib & 0x1ff) | (vb << 9)) << 13; // 983 0070597d
		}
	}
}

void CSky::SetSkyTex(int texHandle, unsigned char weather, unsigned char index)
{
	if (texHandle)
		m_batch[index].xiFlag0 = (texHandle & 0x7ff) | 0x22300000;
	else
		m_batch[index].xiFlag0 = 0;
}

void CSky::ChangeWeatherTex(unsigned char weather)
{
	int texFar = IsThereTexHandle("far", weather);
	int texNear = IsThereTexHandle("near", weather);
	if (!texFar && !texNear)
		return;

	SetSkyTex(texFar, weather, 0);
	SetSkyTex(texNear, weather, 1);
	SetSkyTex(IsThereTexHandle("up", weather), weather, 2);
	SetSkyTex(IsThereTexHandle("dn", weather), weather, 3);
}

void CSky::ClearClouds()
{
	if (g_resrcmng && m_cloudTex)
	{
		g_resrcmng->Release(m_cloudTex);
		m_cloudTex = 0;
	}
	m_cloudNum = 0;
	m_visibleCloudNum = 0;
}

void CSky::RegisterCloud(char* name)
{
	ClearClouds();
	m_cloudTex = g_resrcmng->LoadTexture(name, 0, 0, 0);
}

void CSky::ShowClouds(bool bShow)
{
	int i;
	if (bShow == true)
	{
		for (i = 0; i < m_cloudNum; i++)
			m_clouds[i].bActive = rand() % 2 == 0;
	}
	else
	{
		for (i = 0; i < m_cloudNum; i++)
			m_clouds[i].bActive = true;
	}
}

void CSky::GenerateClouds(int num, float rate)
{
	if (num > 50)
		num = 50;
	m_cloudNum = num;

	if (rate < 1.0f)
		m_cloudInterval = 10.0f;
	else
		m_cloudInterval = 9.0f / rate + 1.0f;

	for (int i = 0; i < num; i++)
	{
		m_clouds[i].scale = Random(1.0f, 2.0f) * 256.0f;
		m_clouds[i].type = rand() % 4;
		m_clouds[i].speed = Random(-0.1f, 0.1f);
		m_clouds[i].bFlip = rand() % 2 != 0;

		if (rand() % 2 == 0)
		{
			m_clouds[i].pos = m_cloudRadius *
				WVector(Random(-1.0f, 1.0f), 0.0f, Random(-1.0f, 1.0f));
			m_clouds[i].pos.y = m_cloudHeight;
			m_clouds[i].bActive = true;
			m_visibleCloudNum++;
		}
		else
		{
			m_clouds[i].pos.y = m_cloudHeight;
			m_clouds[i].bActive = false;
		}
	}
}

void CSky::GenerateStars(int map)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		GenerateMoons();

	if (m_pStars)
	{
		delete m_pStars;
		m_pStars = NULL;
	}

	if (map == -1)
		map = GetCurMap();

	IFF_STRUCT::sCourse* course = ItemManager()->FindCourse(
		0x28000000 | (map > 127 && map != 253 ? map - 128 : map));
	if (!course || !course->SkyFx[0])
		return;

	char* fx = course->SkyFx;
	int len = strlen(fx);
	if (len < 5)
		return;

	char* ext = strrchr(fx, '.');
	if (!ext || strlen(ext) < 4)
		return;

	if (!m_pStars)
	{
		m_pStars = new CPvsFxBox;
		m_pStars->Init(g_resrcmng);
		m_pStars->SetCamera(&g_camera);
		m_pStars->m_frameTime = 1000.0f / 30.0f;
	}

	ext++;
	if (!stricmp(ext, "spr"))
	{
		CFxSpray* spray = m_pStars->LoadSpray(fx, NULL);
		if (spray)
		{
			Attacher attacher;
			attacher.mat[0] = NULL;
			attacher.pos[0] = &g_view->camera.pivot;

			spray->m_pAttacher = &attacher;
			spray->m_attachX = attacher.x;
			spray->m_attachY = attacher.y;
			memcpy(spray->m_attachMat, attacher.mat, sizeof(attacher.mat));
			memcpy(spray->m_attachPos, attacher.pos, sizeof(attacher.pos));

			m_pStars->AddSpray(spray);
		}
	}
	else if (!stricmp(ext, "seq"))
	{
		CFxSequence* seq = m_pStars->LoadSequence(fx, true, NULL);
		if (seq)
		{
			Attacher attacher;
			attacher.mat[0] = NULL;
			attacher.pos[0] = &g_view->camera.pivot;

			seq->m_pAttacher = &attacher;
			seq->m_attachX = attacher.x;
			seq->m_attachY = attacher.y;

			m_pStars->AddSequence(seq);
			m_pStars->Process(m_pStars->m_frameTime);

			for (std::map<CFxSpray*, CFxSpray*>::iterator it =
					 seq->GetSprayList().begin();
				it != seq->GetSprayList().end(); ++it)
			{
				CFxSpray* spray = (*it).second;
				spray->m_pAttacher = &attacher;
				spray->m_attachX = attacher.x;
				spray->m_attachY = attacher.y;
				memcpy(spray->m_attachMat, attacher.mat, sizeof(attacher.mat));
				memcpy(spray->m_attachPos, attacher.pos, sizeof(attacher.pos));

				m_pStars->AddSpray(spray);
			}

			m_pStars->DeleteSequence(seq, false);
		}
	}
}

void CSky::GenerateMoons()
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		Render(NULL);

	if (m_pMoons)
	{
		delete m_pMoons;
		m_pMoons = NULL;
	}

	int num = GOLFDOC()->m_pPolySoup->m_header.pointNum;
	WPolySoup::sPoint* objs = GOLFDOC()->m_pPolySoup->m_point;
	for (int i = 0; i < num; i++)
	{
		WPolySoup::sPoint* obj = &objs[i];
		if (obj->type != 3)
			continue;

		const char* ext = strrchr(obj->option.c_str(), '.');
		if (!ext || strlen(ext) < 4)
			continue;

		if (!m_pMoons)
		{
			m_pMoons = new CPvsFxBox;
			m_pMoons->Init(g_resrcmng);
			m_pMoons->SetCamera(&g_camera);
			m_pMoons->m_frameTime = 1000.0f / 30.0f;
		}

		ext++;
		if (!stricmp(ext, "spr"))
		{
			CFxSpray* spray = m_pMoons->LoadSpray(obj->option.c_str(), NULL);
			if (spray)
			{
				Attacher attacher;
				attacher.mat[0] = NULL;
				attacher.pos[0] = &g_view->camera.pivot;

				spray->m_pAttacher = &attacher;
				spray->m_attachX = attacher.x;
				spray->m_attachY = attacher.y;
				spray->m_offset = obj->pos.Normalize() * 50.0f;
				memcpy(spray->m_attachMat, attacher.mat, sizeof(attacher.mat));
				memcpy(spray->m_attachPos, attacher.pos, sizeof(attacher.pos));

				m_pMoons->AddSpray(spray);
			}
		}
		else if (!stricmp(ext, "seq"))
		{
			CFxSequence* seq =
				m_pMoons->LoadSequence(obj->option.c_str(), true, NULL);
			if (seq)
			{
				Attacher attacher;
				attacher.mat[0] = NULL;
				attacher.pos[0] = &g_view->camera.pivot;

				seq->m_pAttacher = &attacher;
				seq->m_attachX = attacher.x;
				seq->m_attachY = attacher.y;

				m_pMoons->AddSequence(seq);
				m_pMoons->Process(m_pMoons->m_frameTime);

				for (std::map<CFxSpray*, CFxSpray*>::iterator it =
						 seq->GetSprayList().begin();
					it != seq->GetSprayList().end(); ++it)
				{
					CFxSpray* spray = (*it).second;
					spray->m_pAttacher = &attacher;
					spray->m_attachX = attacher.x;
					spray->m_attachY = attacher.y;
					spray->m_offset =
						obj->pos.Normalize() * 50.0f + spray->Pos();
					memcpy(spray->m_attachMat, attacher.mat,
						sizeof(attacher.mat));
					memcpy(spray->m_attachPos, attacher.pos,
						sizeof(attacher.pos));

					m_pMoons->AddSpray(spray);
				}

				m_pMoons->DeleteSequence(seq, false);
			}
		}
	}
}

void CSky::GenerateLensFlare()
{
	if (m_pLensFlare)
	{
		delete m_pLensFlare;
		m_pLensFlare = NULL;
	}

	int num = GOLFDOC()->m_pPolySoup->m_header.pointNum;
	WPolySoup::sPoint* obj = GOLFDOC()->m_pPolySoup->m_point;
	for (int i = 0; i < num; i++, obj++)
	{
		if (obj && obj->type == 2)
		{
			if (!m_pLensFlare)
				m_pLensFlare = new CLensFlare;
			m_pLensFlare->Init(obj->pos);
			return;
		}
	}
}

void CSky::ClearSky()
{
	// 983 (00704e30): maps 5/6 clear both boxes (null-checked) then delete both; 7 only moons; 19 only stars
	switch (GetCurMap())
	{
	case 5:
	case 6:
		if (m_pStars)
			m_pStars->Clear();
		if (m_pMoons)
			m_pMoons->Clear();
		if (m_pStars)
		{
			delete m_pStars;
			m_pStars = NULL;
		}
		if (m_pMoons)
		{
			delete m_pMoons;
			m_pMoons = NULL;
		}
		break;
	case 7:
		m_pMoons->Clear();
		if (m_pMoons)
		{
			delete m_pMoons;
			m_pMoons = NULL;
		}
		break;
	case 19:
		m_pStars->Clear();
		if (m_pStars)
		{
			delete m_pStars;
			m_pStars = NULL;
		}
		break;
	}
}

void CSky::Process(float dt)
{
	m_cloudTimer += dt;
	if (m_cloudTimer > m_cloudInterval)
	{
		m_cloudTimer -= m_cloudInterval;
		if (m_visibleCloudNum < m_cloudNum)
		{
			for (int i = 0; i < m_cloudNum; i++)
			{
				if (!m_clouds[i].bActive)
				{
					WVector pos = WVector(Random(-1.0f, 1.0f), 0.0f, -1.0f) *
						m_cloudRadius;
					m_clouds[i].pos =
						pos * RotMat(Wind().GetGlobalDirection(), 1);
					m_clouds[i].pos.y = m_cloudHeight;
					m_clouds[i].bActive = true;
					m_clouds[i].bFlip = rand() % 2 != 0;
					m_visibleCloudNum++;
					break;
				}
			}
		}
	}

	for (int i = 0; i < m_cloudNum; i++)
	{
		if (!m_clouds[i].bActive)
			continue;

		m_clouds[i].pos +=
			(m_clouds[i].speed + 1.0f) * 3.0f * Wind().GetGlobalWind() * dt;

		float dx = m_clouds[i].pos.x - g_camera.pivot.x;
		float dz = m_clouds[i].pos.z - g_camera.pivot.z;
		float dist = (float)sqrt((double)(dz * dz + dx * dx));
		float a = Max(0.0f, 1.0f - dist / m_cloudFar);
		if (a > 0.5f)
			m_clouds[i].alpha = 255.0f;
		else
			m_clouds[i].alpha = (a + a) * 255.0f;

		WVector d;
		d = m_clouds[i].pos - GOLFDOC()->m_pPolySoup->GetBaseModel()->pos;
		dist = (float)sqrt((double)(d.x * d.x + d.z * d.z));
		a = Max(0.0f, 1.0f - dist / m_cloudRadius);
		if (a < 0.1f)
			m_clouds[i].alpha = a * m_clouds[i].alpha * 10.0f;

		if (dist > (float)(sqrt(2.0) * m_cloudRadius))
		{
			m_clouds[i].bActive = false;
			m_visibleCloudNum--;
		}
	}

	if (m_pStars)
		m_pStars->Process(dt * 1000.0f);
	if (m_pMoons)
		m_pMoons->Process(dt * 1000.0f);
	if (m_pLensFlare)
		m_pLensFlare->Process(dt);

	// 983 00705e9c: no mapa 6 com vento classico (+0x109bd bit 0) o ceu nao gira
	if (GetCurMap() == 6 && (Doc()->m_roomInfo.roomFlags983 & 1))
		return;

	WVector wind = Wind().GetGlobalWind();
	WMatrix rot;
	g_camera.GetRotMatrix(&rot);
	WVector dir;
	dir = WVector(-1.0f, 0.0f, 0.0f) * rot;
	dir.Normalize();
	wind.Normalize();
	float dot = dir * wind;
	m_nearRot += Wind().GetGlobalWind().Magnitude() * dot * dt * 0.005f;
	if ((float)fabs((double)m_nearRot) >= 360.0f)
		m_nearRot = 0.0f;

	m_upRot += dt * 0.01f;
	if (m_upRot >= 360.0f)
		m_upRot = 0.0f;
}

void CSky::Render(WView* view)
{
	float clipNear = view->GetClipNearValue();
	float clipFar = view->GetClipFarValue();

	view->SetClip(1.0f, CalcClipFarByView(view), false);
	view->UpdateCamera();

	RenderUpAndBottom(view);

	if (GOLFDOC()->m_gameMode != 0x200 || view->GetReflective())
	{
		RenderFarCylinder(view);

		if (m_pStars)
		{
			for (std::map<CFxSpray*, CFxSpray*>::iterator it =
					 m_pStars->GetSprayList().begin();
				it != m_pStars->GetSprayList().end(); ++it)
			{
				(*it).second->Pos() = g_view->GetCamera().pivot;
			}
			m_pStars->Display(g_view);
			g_view->Flush(0, true);
		}

		if (m_pMoons)
		{
			for (std::map<CFxSpray*, CFxSpray*>::const_iterator it =
					 m_pMoons->GetSprayList().begin();
				it != m_pMoons->GetSprayList().end(); ++it)
			{
				CFxSpray* spray = (*it).second;
				spray->Pos() = spray->m_offset + g_view->GetCamera().pivot;
			}
			m_pMoons->Display(g_view);
			g_view->Flush(0, true);
		}

		if (m_pLensFlare && !Doc()->m_golfGame.weather)
			m_pLensFlare->Render(view);

		RenderNearCylinder(view);
		// 983: as nuvens não são desenhadas daqui
	}

	view->SetClip(clipNear, clipFar, false);
	view->UpdateCamera();
}

// 983 007060f0 (via 009af5f0, CSceneManager::Render com 0x200): manchas 2D do lens flare, fora do Render do ceu
void CSky::RenderLensFlareOverlay983(WView* view)
{
	if (m_pLensFlare && !Doc()->m_golfGame.weather)
		m_pLensFlare->RenderOverlay983(view);
}

// D904: 983 00706a50 (via 009af470, CSceneManager::Render com 0x100 logo depois do Render_BaseAndShadow): as nuvens
// geradas por GenerateClouds/RegisterCloud nao eram desenhadas por ninguem no nosso
void CSky::RenderClouds983(WView* view)
{
	unsigned long mode = GOLFDOC()->m_gameMode;
	if (mode == 0x200000 || (mode == 0x200 && !view->GetReflective()))
		return;
	if (mode & 0x200)
		RenderFlatCloud(view);
	else
		RenderBillboardCloud(view);
}

void CSky::RenderUpAndBottom(WView* view)
{
	WMatrix m;
	if (m_batch[2].xiFlag0 & 0x7ff)
	{
		m.Reset();
		m.Rotate(m_upRot, 1);
		m *= CalcScaleByView(view);
		m.pivot = view->GetCamera().pivot;
		m_batch[2].xmW = m;
		view->xDrawIndexedTriangles(m_batch[2]);
	}
	if (m_batch[3].xiFlag0 & 0x7ff)
	{
		m.Reset();
		m *= CalcScaleByView(view);
		m.pivot = view->GetCamera().pivot;
		m_batch[3].xmW = m;
		view->xDrawIndexedTriangles(m_batch[3]);
	}
}

void CSky::RenderFarCylinder(WView* view)
{
	if (m_batch[0].xiFlag0 & 0x7ff)
	{
		WMatrix m;
		m.Reset();
		m *= CalcScaleByView(view);
		m.pivot = view->GetCamera().pivot;
		m_batch[0].xmW = m;
		view->xDrawIndexedTriangles(m_batch[0]);
	}
}

void CSky::RenderBillboardCloud(WView* view)
{
	WTVertex vtx[4];
	WTVertex* pVtx[5] = { &vtx[0], &vtx[1], &vtx[3], &vtx[2], NULL };

	for (int i = 0; i < m_cloudNum; i++)
	{
		if (!m_clouds[i].bActive)
			continue;

		for (int j = 0; j < 4; j++)
		{
			WVector p;
			p = RotVec(WVector((j & 2) ? 0.5f : -0.5f, (j & 1) ? 0.25f : -0.25f,
						   0.0f),
					view->GetCamera()) *
					m_clouds[i].scale +
				m_clouds[i].pos;
			vtx[j].SetPosition(p);
			vtx[j].tu = m_clouds[i].bFlip ? ((j & 2) ? 0.0f : 1.0f)
										  : ((j & 2) ? 1.0f : 0.0f);
			vtx[j].tv = (float)(((j & 1) ? 0 : 1) + m_clouds[i].type) * 0.25f;
			vtx[j].diffuse = ModifyColor(m_color, GOLFDOC()->m_bright) |
				((unsigned char)m_clouds[i].alpha << 24);
		}

		view->DrawPolygonFan(pVtx, (m_cloudTex & 0x7ff) | 0x400000, 0x200,
			false);
	}
}

void CSky::RenderFlatCloud(WView* view)
{
	WTVertex vtx[4];
	WTVertex* pVtx[5] = { &vtx[0], &vtx[1], &vtx[3], &vtx[2], NULL };

	for (int i = 0; i < m_cloudNum; i++)
	{
		if (!m_clouds[i].bActive)
			continue;

		for (int j = 0; j < 4; j++)
		{
			WVector p;
			p = WVector((j & 2) ? 0.5f : -0.5f, 0.0f,
					(j & 1) ? 0.25f : -0.25f) *
					m_clouds[i].scale +
				m_clouds[i].pos;
			vtx[j].SetPosition(p);
			vtx[j].tu = m_clouds[i].bFlip ? ((j & 2) ? 0.0f : 1.0f)
										  : ((j & 2) ? 1.0f : 0.0f);
			vtx[j].tv = (float)(((j & 1) ? 0 : 1) + m_clouds[i].type) * 0.25f;
			vtx[j].diffuse =
				ModifyColor(m_color, GOLFDOC()->m_bright) | 0xff000000;
		}

		view->DrawPolygonFan(pVtx, (m_cloudTex & 0x7ff) | 0x400000, 0x200,
			false);
	}
}

void CSky::RenderNearCylinder(WView* view)
{
	if (m_batch[1].xiFlag0 & 0x7ff)
	{
		WMatrix m;
		m.Reset();
		m.Rotate(m_nearRot, 1);
		m *= CalcScaleByView(view);
		m.pivot = view->GetCamera().pivot;
		m_batch[1].xmW = m;
		view->xDrawIndexedTriangles(m_batch[1]);
	}
}

void CSky::UpdateBright()
{
}

float CSky::CalcScaleByView(WView* view) const
{
	if (view->GetProjectionMode() == WView::PARALLEL)
		return (view->GetScale() + view->GetScale()) / 500.0;
	return 1.0f;
}

float CSky::CalcClipFarByView(WView* view) const
{
	if (view->GetProjectionMode() == WView::PARALLEL)
		return CalcScaleByView(view) * 1000.0f;
	return view->GetClipFarValue();
}
