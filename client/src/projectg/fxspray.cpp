#define REBANG_FXSPRAY_CPP
#include "minatl.h"
#include "vc10_layout.h"
#include "fxspray.h"
#include "fxsequence.h"
#include "fxbox.h"
// D863: depois dos std::map do CFxBox (m_sprayCache 0x128...) o offset do VC10 anda no cl novo (vs2022-4: AV em MakeTail
// lendo a camera em +0x17c). Fora do VC10 a visao do CFxBox e completa (RB_FXBOX_FULL, D858): usar os membros.
#ifdef RB_FXBOX_FULL
#define FXBOX_CAMERA983(b) ((b)->m_pCamera)
#define FXBOX_VERTEXPTR983(b) ((b)->m_pVertex)
#define FXBOX_VERTEX983(b) ((b)->m_vertex)
#else
#define FXBOX_CAMERA983(b) (*(const WMatrix**)((char*)(b) + 0x17c)) // 983 CFxBox::m_pCamera (SetCamera 0096c7c0)
#define FXBOX_VERTEXPTR983(b) ((WTVertex**)((char*)(b) + 0x180)) // 983 CFxBox::m_pVertex
#define FXBOX_VERTEX983(b) ((WTVertex*)((char*)(b) + 0x194)) // 983 CFxBox::m_vertex
#endif
// D863: CPetBody+0x109 = m_bGlobalLight no VC10 (C_ASSERT_VC10 no petbody.cpp); no cl novo 0x105. O setter nao e inline
// no VC10 (viraria call), por isso o cru fica so la.
#if _MSC_VER > 1600
#define PETBODY_GLOBALLIGHT983(p, v) ((p)->ActiveGlobalLight(v))
#else
#define PETBODY_GLOBALLIGHT983(p, v) (*((char*)(p) + 0x109) = (v))
#endif
#include "wscene.h"
#include "wview.h"
#include "wutil.h"
#include "petbody.h"
#include "wresrcmng.h"
#include "cfile.h"

extern WMatrix g_camera; // 0xf024a0
#include "binstr.h"
#include "soundmanager.h"
#include <float.h>
#include <math.h>


// Instantiate the list iterators in the order the original emits their members
typedef char rb_inst_ParticleIter[sizeof(CFxParticleList::iterator)];
typedef char rb_inst_SprayPair[sizeof(std::pair<const std::string, CFxSpray*>)];
typedef char
	rb_inst_SprayMap[sizeof(std::map<std::string, CFxSpray*>::iterator)];
typedef char rb_inst_FadeIter[sizeof(std::list<CFxSpray::sFade>::iterator)];
typedef char rb_inst_QuakeIter[sizeof(std::list<CFxSpray::sQuake>::iterator)];
typedef char rb_inst_FlashIter[sizeof(std::list<CFxSpray::sFlash>::iterator)];
typedef char
	rb_inst_GravityIter[sizeof(std::list<CFxSpray::sGravity>::iterator)];
typedef char rb_inst_SpawnIter[sizeof(std::list<CFxSpray::sSpawn>::iterator)];
typedef char rb_inst_RotIter[sizeof(std::list<CFxSpray::sRot>::iterator)];
typedef char rb_inst_SizeIter[sizeof(std::list<CFxSpray::sSize>::iterator)];
typedef char rb_inst_FrameIter[sizeof(std::list<CFxSpray::sFrame>::iterator)];
typedef char rb_inst_WVectorIter[sizeof(std::list<WVector>::iterator)];
typedef char rb_inst_BubbleIter[sizeof(std::list<CFxSpray::sBubble>::iterator)];
typedef char rb_inst_SoundIter[sizeof(std::list<CFxSpray::sSound>::iterator)];
typedef char
	rb_inst_NozzleRotIter[sizeof(std::list<CFxSpray::sNozzleRot>::iterator)];
typedef char rb_inst_RevolIter[sizeof(std::list<CFxSpray::sRevol>::iterator)];
typedef char rb_inst_VelIter[sizeof(std::list<CFxSpray::sVel>::iterator)];
typedef char rb_inst_GenIter[sizeof(std::list<CFxSpray::sGen>::iterator)];

WMatrix RotMat(const WVector& axis, float angle);
WVector operator/(const WVector& vector, float scalar);

inline WVector CFxBox::Random(const WVector& center, float minimum,
	float maximum, float angle)
{
	float cosHalf = (float)cos(angle * 0.5f);
	float range = 1.0f - cosHalf;
	float x, y, z, len;
	do
	{
		x = (float)(rand() % 20001) * 0.0001f - 1.0f;
		y = (float)(rand() % 20001) * 0.0001f - 1.0f;
		z = (float)(rand() % 20001) * range * 0.00005f + cosHalf;
		len = (float)sqrt(x * x + y * y);
	} while (Wabs(len) < 0.00001f);
	float scale = (float)sqrt(1.0f - z * z) / len;
	WVector dir(x * scale, z, y * scale);
	return dir * Random(minimum, maximum) + center;
}

CFxParticle::CFxParticle()
{
	m_pSprite = 0;
	m_pPet = 0;
	m_windRatio = 0;
	m_alpha = 1.0f;
}

CFxParticle::~CFxParticle()
{
	if (m_pPet)
	{
		delete m_pPet;
		m_pPet = 0;
	}
}

static unsigned long ModulateColor(unsigned long a, unsigned long b)
{
	unsigned long red = (a >> 16 & 0xff) * (b >> 16 & 0xff) >> 8;
	unsigned long green = (a >> 8 & 0xff) * (b >> 8 & 0xff) >> 8;
	unsigned long blue = (a & 0xff) * (b & 0xff) >> 8;
	return (red << 16) + (green << 8) + blue;
}

unsigned long SetAlpha(unsigned long color, float alpha)
{
	return (unsigned char)((color >> 24) * alpha) << 24 | color & 0xffffff;
}

static void ScaleAabb(Waabb* out, const Waabb* in, bool sphere, float scaleXZ,
	float scaleY)
{
	if (sphere)
	{
		out->min = in->min;
		out->max.x = scaleXZ * in->max.y;
		out->max.z = 0.0f;
		out->max.y = scaleY * in->max.y;
		return;
	}
	if (scaleXZ >= 1.0f && scaleY >= 1.0f)
	{
		*out = *in;
		return;
	}
	float cx = (in->max.x + in->min.x) * 0.5f;
	float cy = (in->max.y + in->min.y) * 0.5f;
	float cz = (in->max.z + in->min.z) * 0.5f;
	float hx = (in->max.x - in->min.x) * 0.5f;
	float hy = (in->max.y - in->min.y) * 0.5f;
	float hz = (in->max.z - in->min.z) * 0.5f;
	if (scaleXZ >= 1.0f)
	{
		out->min.x = in->min.x;
		out->max.x = in->max.x;
		out->min.z = in->min.z;
		out->max.z = in->max.z;
	}
	else
	{
		out->min.x = (cx - hx) * scaleXZ;
		out->min.z = (cz - hz) * scaleXZ;
		out->max.x = (hx + cx) * scaleXZ;
		out->max.z = (hz + cz) * scaleXZ;
	}
	if (scaleY < 1.0f)
	{
		out->min.y = (cy - hy) * scaleY;
		out->max.y = (hy + cy) * scaleY;
		return;
	}
	out->min.y = in->min.y;
	out->max.y = in->max.y;
}

// Fade factor of a particle position against the area (custom register convention in the original)
static float CalcFade(const Waabb* area, const WVector* pos, const Waabb* full,
	float scaleXZ, float scaleY, bool sphere)
{
	float fade = 1.0f;
	if (sphere)
	{
		float dist =
			(float)sqrt((pos->y - full->min.y) * (pos->y - full->min.y) +
				(pos->x - full->min.x) * (pos->x - full->min.x) +
				(pos->z - full->min.z) * (pos->z - full->min.z));
		if (dist < area->max.x)
		{
			if (Wabs(area->max.x) < 0.00001f)
				return fade;
			return dist / area->max.x;
		}
		if (area->max.y > dist)
			return fade;
		if (Wabs(full->max.y - area->max.y) < 0.00001f)
			return fade;
		return 1.0f - (dist - area->max.y) / (full->max.y - area->max.y);
	}
	if (scaleXZ < 1.0f)
	{
		if (area->min.x > pos->x)
			fade = 1.0f - (pos->x - area->min.x) / (full->min.x - area->min.x);
		else if (area->max.x < pos->x)
			fade = 1.0f - (pos->x - area->max.x) / (full->max.x - area->max.x);
		if (area->min.z > pos->z)
			fade =
				(1.0f - (pos->z - area->min.z) / (full->min.z - area->min.z)) *
				fade;
		else if (pos->z > area->max.z)
			fade =
				(1.0f - (pos->z - area->max.z) / (full->max.z - area->max.z)) *
				fade;
	}
	if (scaleY < 1.0f)
	{
		if (pos->y < area->min.y)
			return (1.0f -
					   (pos->y - area->min.y) / (full->min.y - area->min.y)) *
				fade;
		if (area->max.y < pos->y)
			return (1.0f -
					   (pos->y - area->max.y) / (full->max.y - area->max.y)) *
				fade;
	}
	return fade;
}

// Angle between two vectors (custom register convention in the original)
static float AngleBetween(const WVector& a, const WVector& b)
{
	float lenA = (float)sqrt(a.z * a.z + a.y * a.y + a.x * a.x);
	if (Wabs(lenA) >= 0.00001f)
	{
		float lenB = (float)sqrt(b.z * b.z + b.y * b.y + b.x * b.x);
		if (Wabs(lenB) >= 0.00001f)
		{
			float c = (b.y * a.y + b.z * a.z + a.x * b.x) / (lenB * lenA);
			if (c < -1.0f)
				return (float)acos(-1.0f);
			if (c > 1.0f)
				return (float)acos(1.0f);
			return (float)acos(c);
		}
	}
	return 0.0f;
}

C_ASSERT_VC10(sizeof(CFxSpray) == 0x15a0);

CFxSpray::CFxSpray(CFxBox& box)
	: m_pFxBox(&box)
{
	Reset();
	m_attachX = FLT_MAX;
	m_attachY = FLT_MAX;
	memset(m_attachMat, 0, sizeof(m_attachMat));
	memset(m_attachPos, 0, sizeof(m_attachPos));
}

CFxSpray::~CFxSpray()
{
	for (std::list<CFxParticle*>::iterator it = m_particles.begin();
		it != m_particles.end(); ++it)
		delete *it;
	m_particles.clear();
}

void CFxSpray::Reset()
{
	m_bMark = true;
	m_bActive = true;
	m_pAttacher = NULL;
	m_pSequence = 0;
	m_time = 0;
	m_pos = WVector::ZERO;
	m_offset = WVector::ZERO;
	m_bgRot.Reset();
	m_rot1190.Reset();
	m_quakeShake.Reset();
	m_quakeCenter.Reset();
	m_flashAlpha = false;
	m_pParent = 0;
	for (std::list<CFxParticle*>::iterator it = m_particles.begin();
		it != m_particles.end(); ++it)
		delete *it;
	m_particles.clear();

	m_param.m_bPetBillboard = false;
	m_param.m_bSphere = false;
	m_param.m_aabb.min = m_param.m_aabb.max = WVector::ZERO;
	m_param.m_bRegenAtSamePos = false;
	m_param.m_zBias = 0;
	m_param.m_bBillboard = true;
	m_param.m_pSprite = 0;
	m_param.m_spriteName[0] = 0;
	m_param.m_bPetLight = false;
	m_param.m_bBackground = false;
	m_param.m_bAttach = false;
	m_param.m_attachAxis = 0;
	m_param.m_alpha = 1.0f;
	m_param.m_coreMax = 1.0f;
	m_param.m_coreMin = 1.0f;
	m_param.m_friction = 0;
	m_param.m_groundFriction = 0;
	m_param.m_lifetime2 = 1000;
	m_param.m_lifetime = 1000;
	m_param.m_gravityVec = WVector::ZERO;
	m_param.m_groundX = -1000.0f;
	m_param.m_groundY = 0;
	m_param.m_groundZ = 0;
	m_param.m_lightType = 0;
	m_param.m_lightColor = 0xffffff;
	m_param.m_blendSrc = 0x800000;
	m_param.m_blendDst = 0x800000;
	m_param.m_initVel.min = m_param.m_initVel.max = WVector::ZERO;
	m_param.m_windEnable[0] = m_param.m_windEnable[1] = 0;
	m_param.m_wind.x = m_param.m_wind.y = m_param.m_wind.z = 0;
	m_param.m_extOrient = 0;
	m_param.m_initAngle.min = m_param.m_initAngle.max = WVector::ZERO;
	m_param.m_initRot.min = m_param.m_initRot.max = WVector::ZERO;
	m_param.m_initFrame0 = m_param.m_initFrame1 = 0;
	m_param.m_initSize0 = m_param.m_initSize1 = 1.0f;
	m_param.m_initSize2 = 1.0f;
	m_param.m_tailColor[0] = 0;
	m_param.m_bTail = 0;
	m_param.m_tailInterval = 0;
	m_param.m_tailTexture[0] = 0;
	m_param.m_bandana = 0;
	m_param.m_bandanaWidth = 50.0f;
	m_param.m_bBandanaMonolith = false;
	m_param.m_bBandanaCameraSide = false;
	m_param.m_tailColor[1] = 0xff;
	m_param.m_tailColor[2] = 0xff;
	m_param.m_tailColor[3] = 0xff;
	m_param.m_rot.clear();
	m_param.m_nozzleRot.clear();
	m_param.m_rotate.clear();
	m_param.m_revol.clear();
	m_param.m_gen.clear();
	m_param.m_gravity.clear();
	m_param.m_fade.clear();
	m_param.m_size.clear();
	m_param.m_frame.clear();
	m_param.m_distFar = 0;
	m_param.m_distNear = 0;
	m_param.m_bRing = false;
	m_ringAngle = 0;
	m_killTime = -1;
	memset(m_defines, 0, sizeof(m_defines));
}

// CFxBox+0x128: cache of parsed sprays by file name
#define SPRAY_CACHE(box) \
	(*(std::map<std::string, CFxSpray*>*)((char*)(box) + 0x128))

bool CFxSpray::ParseFile(const char* filename, bool useCache)
{
	strcpy(m_filename, filename);
	std::map<std::string, CFxSpray*>::iterator found =
		SPRAY_CACHE(m_pFxBox).find(filename);
	if (found == SPRAY_CACHE(m_pFxBox).end())
	{
		cFile* file = m_pFxBox->m_pResMng->GetCFile(filename, 0x2000);
		if (!file)
			return false;
		char line[0x400];
		while (file->Scan("%n", line))
		{
			if (!ParseLine(TrimA(line, " \t\n\r")))
			{
				CloseCFile(file);
				return false;
			}
		}
		CloseCFile(file);
		CFxSpray* spray = new CFxSpray(*m_pFxBox);
		spray->m_param = m_param;
		SPRAY_CACHE(m_pFxBox)[filename] = spray;
	}
	else if (useCache)
		m_param = (*found).second->m_param;
	else
	{
		cFile* file = m_pFxBox->m_pResMng->GetCFile(filename, 0x2000);
		if (!file)
			return false;
		char line[0x400];
		while (file->Scan("%n", line))
		{
			if (!ParseLine(TrimA(line, " \t\n\r")))
			{
				CloseCFile(file);
				return false;
			}
		}
		CloseCFile(file);
	}
	return true;
}

bool CFxSpray::ParseLine(const char* line)
{
	if (!*line || *line == ';' ||
		m_pFxBox->PreprocessDefines(*(defines_t*)m_defines, line))
		return true;
	cTokenV tok;
	tok.Init(line, strlen(line));
	char name[0x104];
	tok.GetToken(name, "=", 1);
	TrimA(name, " \t\n\r");
	char t[16][0x104];
	for (int i = 0; i < 16; i++)
	{
		tok.GetToken(t[i], ",", 1);
		TrimA(t[i], " \t()%");
		m_pFxBox->ProcessDefines(*(defines_t*)m_defines, t[i]);
	}
	if (!_strcmpi(name, "source"))
	{
		cTokenV src;
		src.Init(t[0], strlen(t[0]));
		char* word;
		src.GetTokenWithoutFail(&word, " \t", 2);
		if (!_strcmpi(word, "*pet"))
		{
			src.GetTokenWithoutFail(&word, " \t", 2);
			if (m_pFxBox->m_pResMng &&
				_strcmpi(m_pFxBox->m_pResMng->FindMatchFile(word), word))
			{
				strcpy(m_param.m_spriteName, t[0]);
				m_param.m_bPetLight = !_strcmpi(t[1], "petlight");
				m_param.m_bPetBillboard = !_strcmpi(t[1], "petbillboard");
				return true;
			}
		}
		else
		{
			m_param.m_pSprite =
				m_pFxBox->LoadSprite(t[0], atoi(t[1]), atoi(t[2]));
			if (m_param.m_pSprite)
			{
				m_param.m_bBillboard =
					strlen(t[3]) == 0 || !_strcmpi(t[3], "billboard");
				return true;
			}
		}
		return false;
	}
	if (!_strcmpi(name, "tail"))
	{
		m_param.m_bTail = atoi(t[0]);
		m_param.m_tailInterval = m_pFxBox->ParseNumber(t[1]);
		m_param.m_tailColor[0] = atoi(t[2]);
		m_param.m_tailColor[1] = atoi(t[3]);
		m_param.m_tailColor[2] = atoi(t[4]);
		m_param.m_tailColor[3] = atoi(t[5]);
		strcpy(m_param.m_tailTexture, t[6]);
		if (strlen(m_param.m_tailTexture) != 0)
		{
			if (!m_pFxBox->m_pResMng)
				return false;
			m_param.m_bandana = m_pFxBox->m_pResMng->LoadTexture(
				m_param.m_tailTexture, 0, 0, NULL);
			if (!m_param.m_bandana)
				return false;
			m_param.m_bandanaWidth = (float)atof(t[7]);
			m_param.m_bBandanaMonolith = !_strcmpi(t[8], "monolith");
			m_param.m_bBandanaCameraSide = !_strcmpi(t[9], "cameraside");
		}
	}
	else if (!_strcmpi(name, "issphere"))
	{
		int sphere = atoi(t[0]);
		m_param.m_bSphere = sphere != 0;
		if (sphere == 2)
			m_param.m_bRing = true;
	}
	else if (!_strcmpi(name, "genpos"))
	{
		WVector v((float)atof(t[0]), (float)atof(t[1]), (float)atof(t[2]));
		m_param.m_aabb.min = v;
		if (!m_param.m_bSphere)
			v = WVector((float)atof(t[3]), (float)atof(t[4]),
				(float)atof(t[5]));
		else
			v = WVector((float)atof(t[3]), (float)atof(t[4]),
				m_pFxBox->ParseNumber(t[5]));
		m_param.m_aabb.max = v;
	}
	else if (!_strcmpi(name, "regenatsamepos"))
		m_param.m_bRegenAtSamePos = atoi(t[0]) != 0;
	else if (!_strcmpi(name, "camoffset"))
		m_param.m_zBias = (float)atof(t[0]);
	else if (!_strcmpi(name, "add_generation"))
	{
		CFxSpray::sGen gen;
		gen.m_start = (int)m_pFxBox->ParseNumber(t[0]);
		gen.m_end = (int)m_pFxBox->ParseNumber(t[1]);
		gen.m_count = atoi(t[2]);
		gen.m_rate = m_pFxBox->ParseNumber(t[3]);
		gen.m_emitted = 0;
		gen.m_lastTime = 0;
		m_param.m_gen.push_back(gen);
	}
	else if (!_strcmpi(name, "lifetime"))
	{
		m_param.m_lifetime = (int)m_pFxBox->ParseNumber(t[0]);
		m_param.m_lifetime2 = (int)m_pFxBox->ParseNumber(t[1]);
		if (m_param.m_lifetime == 0)
		{
			for (std::list<sFade>::iterator it = m_param.m_fade.begin();
				it != m_param.m_fade.end(); ++it)
			{
				if ((*it).m_time == 0.0f)
				{
					(*it).m_color[0] = 0xff;
					(*it).m_endColor[0] = 0xff;
					break;
				}
			}
		}
		else
			m_param.m_alpha = m_param.m_coreMin = m_param.m_coreMax = 1.0f;
	}
	else if (!_strcmpi(name, "vol_max_opacity"))
	{
		m_param.m_alpha = Between(0.0f, (float)atof(t[0]), 1.0f);
		if (m_param.m_lifetime != 0)
			m_param.m_alpha = 1.0f;
	}
	else if (!_strcmpi(name, "vol_core"))
	{
		if (m_param.m_lifetime == 0)
		{
			m_param.m_coreMin = Between(0.0f, (float)atof(t[0]), 1.0f);
			m_param.m_coreMax = Between(0.0f, (float)atof(t[1]), 1.0f);
		}
		else
			m_param.m_coreMin = m_param.m_coreMax = 1.0f;
	}
	else if (!_strcmpi(name, "init_velocity"))
	{
		m_param.m_initVel.min =
			WVector((float)atof(t[0]), (float)atof(t[1]), (float)atof(t[2]));
		m_param.m_initVel.max =
			WVector((float)atof(t[3]), (float)atof(t[4]), (float)atof(t[5]));
	}
	else if (!_strcmpi(name, "add_velocity"))
	{
		CFxSpray::sVel vel;
		vel.m_start = (int)m_pFxBox->ParseNumber(t[0]);
		vel.m_startVal = (float)atof(t[1]);
		vel.m_end = (int)m_pFxBox->ParseNumber(t[2]);
		vel.m_endVal = (float)atof(t[3]);
		m_param.m_vel.push_back(vel);
	}
	else if (!_strcmpi(name, "add_wind"))
	{
		m_param.m_windEnable[0] = (float)atof(t[0]);
		m_param.m_windEnable[1] = (float)atof(t[1]);
	}
	else if (!_strcmpi(name, "ext_wind"))
		m_param.m_wind =
			WVector((float)atof(t[0]), (float)atof(t[1]), (float)atof(t[2]));
	else if (!_strcmpi(name, "ext_orientation"))
	{
		int x = 0;
		int y = 0;
		int z = 0;
		sscanf(t[0], "%d", &x);
		sscanf(t[1], "%d", &y);
		sscanf(t[2], "%d", &z);
		m_param.m_extOrient = 0;
		if (x != 0)
			m_param.m_extOrient = 1;
		if (y != 0)
			m_param.m_extOrient |= 2;
		if (z != 0)
			m_param.m_extOrient |= 4;
	}
	else if (!_strcmpi(name, "init_angle"))
	{
		m_param.m_initAngle.min.x = m_pFxBox->ParseNumber(t[0]);
		m_param.m_initAngle.min.y = m_pFxBox->ParseNumber(t[1]);
		m_param.m_initAngle.min.z = m_pFxBox->ParseNumber(t[2]);
		m_param.m_initAngle.max.x = m_pFxBox->ParseNumber(t[3]);
		m_param.m_initAngle.max.y = m_pFxBox->ParseNumber(t[4]);
		m_param.m_initAngle.max.z = m_pFxBox->ParseNumber(t[5]);
	}
	else if (!_strcmpi(name, "init_rotation"))
	{
		m_param.m_initRot.min.x = m_pFxBox->ParseNumber(t[0]);
		m_param.m_initRot.min.y = m_pFxBox->ParseNumber(t[1]);
		m_param.m_initRot.min.z = m_pFxBox->ParseNumber(t[2]);
		m_param.m_initRot.max.x = m_pFxBox->ParseNumber(t[3]);
		m_param.m_initRot.max.y = m_pFxBox->ParseNumber(t[4]);
		m_param.m_initRot.max.z = m_pFxBox->ParseNumber(t[5]);
	}
	else if (!_strcmpi(name, "add_rotation"))
	{
		CFxSpray::sRot rot;
		rot.m_start = (float)atof(t[0]);
		rot.m_startVal = m_pFxBox->ParseNumber(t[1]);
		rot.m_end = (float)atof(t[2]);
		rot.m_endVal = m_pFxBox->ParseNumber(t[3]);
		m_param.m_rot.push_back(rot);
	}
	else if (!_strcmpi(name, "friction"))
		m_param.m_friction = (float)atof(t[0]);
	else if (!_strcmpi(name, "groundfriction"))
		m_param.m_groundFriction = (float)atof(t[0]);
	else if (!_strcmpi(name, "gravity"))
		m_param.m_gravityVec =
			WVector((float)atof(t[0]), (float)atof(t[1]), (float)atof(t[2]));
	else if (!_strcmpi(name, "add_gravitypoint"))
	{
		CFxSpray::sGravity grav;
		grav.m_pos.x = (float)atof(t[0]);
		grav.m_pos.y = (float)atof(t[1]);
		grav.m_pos.z = (float)atof(t[2]);
		grav.m_near = (float)atof(t[3]);
		grav.m_radius = (float)atof(t[4]);
		grav.m_far = (float)atof(t[5]);
		m_param.m_gravity.push_back(grav);
	}
	else if (!_strcmpi(name, "ground"))
	{
		m_param.m_groundX = (float)atof(t[0]);
		m_param.m_groundY = (float)atof(t[1]);
		m_param.m_groundZ = (float)atof(t[2]);
	}
	else if (!_strcmpi(name, "blend"))
	{
		if (!_strcmpi(t[0], "normal"))
			m_param.m_blendSrc = 0;
		else if (!_strcmpi(t[0], "add"))
			m_param.m_blendSrc = 0x800000;
		else if (!_strcmpi(t[0], "multiply"))
			m_param.m_blendSrc = 0x1000000;
		else if (!_strcmpi(t[0], "invmultiply"))
			m_param.m_blendSrc = 0x1800000;
		else
			return false;
		if (!_strcmpi(t[1], "normal"))
			m_param.m_blendDst = 0;
		else if (!_strcmpi(t[1], "add"))
			m_param.m_blendDst = 0x800000;
		else if (!_strcmpi(t[1], "multiply"))
			m_param.m_blendDst = 0x1000000;
		else if (!_strcmpi(t[1], "invmultiply"))
			m_param.m_blendDst = 0x1800000;
		else
			return false;
	}
	else if (!_strcmpi(name, "lighting"))
	{
		if (!_strcmpi(t[0], "off"))
			m_param.m_lightType = 0;
		else if (!_strcmpi(t[0], "diffuse"))
			m_param.m_lightType = 1;
		else if (!_strcmpi(t[0], "ambient"))
			m_param.m_lightType = 2;
		else if (!_strcmpi(t[0], "ambient2"))
			m_param.m_lightType = 3;
		else
			return false;
	}
	else if (!_strcmpi(name, "add_fade"))
	{
		CFxSpray::sFade fade;
		fade.m_time = (float)atof(t[0]);
		fade.m_color[0] = atoi(t[1]);
		fade.m_color[1] = atoi(t[2]);
		fade.m_color[2] = atoi(t[3]);
		fade.m_color[3] = atoi(t[4]);
		fade.m_end = (float)atof(t[5]);
		fade.m_endColor[0] = atoi(t[6]);
		fade.m_endColor[1] = atoi(t[7]);
		fade.m_endColor[2] = atoi(t[8]);
		fade.m_endColor[3] = atoi(t[9]);
		if (m_param.m_lifetime == 0 && fade.m_time == 0.0f)
		{
			fade.m_endColor[0] = 0xff;
			fade.m_color[0] = 0xff;
		}
		m_param.m_fade.push_back(fade);
	}
	else if (!_strcmpi(name, "init_size"))
	{
		sscanf(t[0], "%f", &m_param.m_initSize0);
		sscanf(t[1], "%f", &m_param.m_initSize1);
		sscanf(t[2], "%f", &m_param.m_initSize2);
	}
	else if (!_strcmpi(name, "add_size"))
	{
		CFxSpray::sSize size;
		sscanf(t[0], "%f", &size.m_start);
		sscanf(t[1], "%f", &size.m_startVal);
		sscanf(t[2], "%f", &size.m_end);
		sscanf(t[3], "%f", &size.m_endVal);
		m_param.m_size.push_back(size);
	}
	else if (!_strcmpi(name, "init_frame"))
	{
		sscanf(t[0], "%d", &m_param.m_initFrame0);
		sscanf(t[1], "%d", &m_param.m_initFrame1);
	}
	else if (!_strcmpi(name, "add_frame"))
	{
		CFxSpray::sFrame frame;
		sscanf(t[0], "%f", &frame.m_start);
		sscanf(t[1], "%f", &frame.m_end);
		sscanf(t[2], "%d", &frame.m_first);
		sscanf(t[3], "%d", &frame.m_last);
		frame.m_interval = (float)(int)m_pFxBox->ParseNumber(t[4]);
		m_param.m_frame.push_back(frame);
	}
	else if (!_strcmpi(name, "add_quake"))
	{
		CFxSpray::sQuake quake;
		quake.m_amp.x = (float)atof(t[0]);
		quake.m_amp.y = (float)atof(t[1]);
		quake.m_amp.z = (float)atof(t[2]);
		quake.m_start = (int)m_pFxBox->ParseNumber(t[3]);
		quake.m_startVec.x = (float)atof(t[4]);
		quake.m_startVec.y = (float)atof(t[5]);
		quake.m_startVec.z = (float)atof(t[6]);
		quake.m_end = (int)m_pFxBox->ParseNumber(t[7]);
		quake.m_endVec.x = (float)atof(t[8]);
		quake.m_endVec.y = (float)atof(t[9]);
		quake.m_endVec.z = (float)atof(t[10]);
		quake.m_interval = (float)(int)m_pFxBox->ParseNumber(t[11]);
		quake.m_lastTime = 0;
		m_param.m_quake.push_back(quake);
	}
	else if (!_strcmpi(name, "add_flash"))
	{
		CFxSpray::sFlash flash;
		flash.m_start = (int)m_pFxBox->ParseNumber(t[0]);
		flash.m_mid = (int)m_pFxBox->ParseNumber(t[1]);
		flash.m_end = (int)m_pFxBox->ParseNumber(t[2]);
		m_param.m_flash.push_back(flash);
	}
	else if (!_strcmpi(name, "dist_limit"))
	{
		m_param.m_distNear = Max(0.0f, (float)atof(t[0]));
		m_param.m_distFar = Max(0.0f, (float)atof(t[1]));
		if (m_param.m_distFar < m_param.m_distNear)
			m_param.m_distFar = m_param.m_distNear;
	}
	else if (!_strcmpi(name, "add_bubble"))
	{
		CFxSpray::sBubble bubble;
		bubble.m_amp.x = (float)atof(t[0]);
		bubble.m_freq.x = (float)atof(t[1]);
		bubble.m_amp.y = (float)atof(t[2]);
		bubble.m_freq.y = (float)atof(t[3]);
		bubble.m_amp.z = (float)atof(t[4]);
		bubble.m_freq.z = (float)atof(t[5]);
		m_param.m_bubble.push_back(bubble);
	}
	else if (!_strcmpi(name, "sound"))
	{
		CFxSpray::sSound sound;
		sound.m_volume = -1.0f;
		strcpy(sound.m_name, t[0]);
		char* wav = strstr(sound.m_name, ".wav");
		if (wav)
			*wav = '\0';
		sscanf(t[1], "%f", &sound.m_volume);
		m_param.m_sound.push_back(sound);
	}
	return true;
}

bool CFxSpray::Generate(float time)
{
	float speed = 100.0f;
	for (std::list<sVel>::iterator it = m_param.m_vel.begin();
		it != m_param.m_vel.end(); ++it)
	{
		float start = (float)(*it).m_start;
		if (start <= time && time <= (float)(*it).m_end)
		{
			if ((*it).m_end - (*it).m_start > 0)
				speed = (time - start) * ((*it).m_endVal - (*it).m_startVal) /
						(float)((*it).m_end - (*it).m_start) +
					(*it).m_startVal;
			else
				speed = (*it).m_startVal;
		}
	}

	bool generated = false;
	Waabb area;
	area.min = WVector::ZERO;
	area.max = WVector::ZERO;
	if (m_param.m_lifetime == 0)
		area = m_param.m_aabb;
	else
		ScaleAabb(&area, &m_param.m_aabb, m_param.m_bSphere, m_param.m_coreMin,
			m_param.m_coreMax);

	std::list<sGen>::iterator it;
	for (it = m_param.m_gen.begin(); it != m_param.m_gen.end(); it++)
	{
		if (time <= (float)(*it).m_end || (*it).m_emitted < (*it).m_count)
		{
			generated = true;
			if ((float)(*it).m_start <= time)
			{
				int num;
				if ((float)(*it).m_end <= time ||
					(*it).m_end - (*it).m_start < 1)
					num = (*it).m_count - (*it).m_emitted;
				else
					num = (int)((time - (float)(*it).m_start) *
								  (float)(*it).m_count /
								  (float)((*it).m_end - (*it).m_start) +
							  0.0001f) +
						(1 - (*it).m_emitted);
				if (num > 0)
					(*it).m_lastTime = time;
				if ((*it).m_count < num + (*it).m_emitted)
					num = (*it).m_count - (*it).m_emitted;
				while (num > 0)
				{
					CFxParticle* particle = new CFxParticle();
					InitParticle(false, particle, speed, (*it).m_rate, area,
						false);
					m_particles.push_back(particle);
					(*it).m_emitted++;
					num--;
				}
			}
			else
				(*it).m_lastTime = time;
		}
	}
	return generated;
}


// CFxBox vtable slot 2 (CheckGroundCollision is only declared in CPvsFxBox)
struct RbFxBoxVtbl
{
	virtual void Slot0() = 0;
	virtual void Slot1() = 0;
	virtual bool CheckGroundCollision(const WVector& from, const WVector& to,
		float& result, float radius) = 0;
};

bool CFxSpray::Process(float elapsed)
{
	float dt = elapsed * 0.06f;
	m_pos.x = dt * m_offset.x + m_pos.x;
	m_pos.y = dt * m_offset.y + m_pos.y;
	m_pos.z = dt * m_offset.z + m_pos.z;

	WMatrix mat;
	if (m_param.m_bAttach && m_pSequence)
	{
		m_pSequence->GetBoneMatrix(&mat);
		mat.GetRotMatrix(&m_bgRot);
		if (m_param.m_attachAxis != 0 && m_param.m_attachAxis != 7)
		{
			WVector angle;
			angle.x = angle.y = angle.z = 0.0f;
			WVector axis;
			WVector v;
			if (m_param.m_attachAxis & 1)
			{
				axis = WVector::UNIT_POS_Y;
				v = axis * m_bgRot;
				v.x = 0.0f;
				angle.x = AngleBetween(v, axis);
				if (!WisZero(v.z))
					angle.x = (v.z < 0.0f ? -1.0f : 1.0f) * angle.x;
			}
			if (m_param.m_attachAxis & 2)
			{
				axis = WVector::UNIT_POS_Z;
				v = axis * m_bgRot;
				v.y = 0.0f;
				angle.y = AngleBetween(v, axis);
				if (!WisZero(v.x))
					angle.y = (v.x < 0.0f ? -1.0f : 1.0f) * angle.y;
			}
			if (m_param.m_attachAxis & 4)
			{
				axis = WVector::UNIT_POS_X;
				v = axis * m_bgRot;
				v.z = 0.0f;
				angle.z = AngleBetween(v, axis);
				if (!WisZero(v.y))
					angle.z = (v.y <= 0.0f ? 1.0f : -1.0f) * angle.z;
			}
			m_bgRot = ~RotMat(angle);
		}
	}

	mat = RotateNozzle(m_time, dt);
	m_rot1190 = m_rot1190 * mat;
	mat = Rotate(m_time, dt);
	Revolute(m_time, dt);
	RevoluteGrand(m_time, dt);

	// quake
	m_quakeCenter.x = m_quakeCenter.y = m_quakeCenter.z = 0.0f;
	bool bQuake = false;
	WVector quakeSum;
	quakeSum.x = quakeSum.y = quakeSum.z = 0.0f;
	for (std::list<sQuake>::iterator it = m_param.m_quake.begin();
		it != m_param.m_quake.end(); ++it)
	{
		float start = (float)(*it).m_start;
		if (start <= m_time)
		{
			if ((float)(*it).m_end >= m_time &&
				(bQuake = true, (*it).m_interval < m_time - (*it).m_lastTime))
			{
				(*it).m_lastTime = m_time;
				int len = (*it).m_end - (*it).m_start;
				WVector cur;
				WVector* q;
				if (len < 1)
					q = &(*it).m_startVec;
				else
				{
					float k = 1.0f / (float)len;
					float elapsedQ = m_time - start;
					cur.x =
						((*it).m_endVec.x - (*it).m_startVec.x) * elapsedQ * k +
						(*it).m_startVec.x;
					cur.y =
						k * ((*it).m_endVec.y - (*it).m_startVec.y) * elapsedQ +
						(*it).m_startVec.y;
					cur.z =
						k * elapsedQ * ((*it).m_endVec.z - (*it).m_startVec.z) +
						(*it).m_startVec.z;
					q = &cur;
				}
				float qx = q->x;
				float qy = q->y;
				float qz = q->z;
				m_quakeShake.x = m_quakeShake.y = m_quakeShake.z = 0.0f;
				m_quakeShake.x = (float)(rand() % 20001 - 10000) * qx * 0.00005f +
					m_quakeShake.x;
				m_quakeShake.y = (float)(rand() % 20001 - 10000) * qy * 0.00005f +
					m_quakeShake.y;
				m_quakeShake.z = (float)(rand() % 20001 - 10000) * qz * 0.00005f +
					m_quakeShake.z;
				m_quakeCenter.x = qx * (*it).m_amp.x + m_quakeCenter.x;
				m_quakeCenter.y = qy * (*it).m_amp.y + m_quakeCenter.y;
				m_quakeCenter.z = qz * (*it).m_amp.z + m_quakeCenter.z;
				quakeSum.x = quakeSum.x + qx;
				quakeSum.y = quakeSum.y + qy;
				quakeSum.z = quakeSum.z + qz;
			}
		}
		else
			(*it).m_lastTime = m_time;
	}
	if (bQuake)
	{
		if (Wabs(quakeSum.x) >= 0.00001f)
			m_quakeCenter.x = m_quakeCenter.x / quakeSum.x;
		if (Wabs(quakeSum.y) >= 0.00001f)
			m_quakeCenter.y = m_quakeCenter.y / quakeSum.y;
		if (Wabs(quakeSum.z) >= 0.00001f)
			m_quakeCenter.z = m_quakeCenter.z / quakeSum.z;
	}
	else
		m_quakeShake.x = m_quakeShake.y = m_quakeShake.z = 0.0f;

	// flash
	m_flashAlpha = 0;
	for (std::list<sFlash>::iterator it = m_param.m_flash.begin();
		it != m_param.m_flash.end(); ++it)
	{
		float start = (float)(*it).m_start;
		if (start <= m_time)
		{
			int a;
			if ((float)(*it).m_mid < m_time)
			{
				if (m_time <= (float)(*it).m_end)
				{
					if ((*it).m_end - (*it).m_mid < 1)
						a = 0;
					else
						a = 0xff -
							((int)(m_time - (float)(*it).m_mid) * 0xff) /
								((*it).m_end - (*it).m_mid);
					a = a + m_flashAlpha;
					if (a > 0xff)
						a = 0xff;
					m_flashAlpha = (unsigned char)a;
				}
			}
			else
			{
				if ((*it).m_mid - (*it).m_start < 1)
					a = 0;
				else
					a = ((int)(m_time - start) * 0xff) /
						((*it).m_mid - (*it).m_start);
				a = a + m_flashAlpha;
				if (a > 0xff)
					a = 0xff;
				m_flashAlpha = (unsigned char)a;
			}
		}
	}

	bool bGenerated = Generate(m_time);

	Waabb area;
	area.min.x = area.min.y = area.min.z = 0.0f;
	area.max.x = area.max.y = area.max.z = 0.0f;
	bool bRespawn = false;
	ScaleAabb(&area, &m_param.m_aabb, m_param.m_bSphere, m_param.m_coreMin,
		m_param.m_coreMax);

	std::list<CFxParticle*>::iterator pit = m_particles.begin();
	while (pit != m_particles.end())
	{
		CFxParticle* p = *pit;
		WVector base = GetBgPos(p);
		WMatrix rot = GetBgRot(p);
		WVector pos = base + p->m_origin * rot;

		float pct;
		if (p->m_lifetime <= 0.0f)
			pct = 100.0f;
		else
			pct = (p->m_lifetime - p->m_lifeLeft) / p->m_lifetime * 100.0f;

		WVector tailPos;
		tailPos.x = pos.x + p->m_offset.x;
		tailPos.y = pos.y + p->m_offset.y;
		tailPos.z = pos.z + p->m_offset.z;
		MakeTail(m_time, p, tailPos);

		if (!p->m_bNoWind)
		{
			float wind = p->m_windRatio;
			WVector windVec;
			windVec.x = wind * m_param.m_wind.x;
			windVec.y = wind * m_param.m_wind.y;
			windVec.z = wind * m_param.m_wind.z;
			WVector prevPos = pos;
			WMatrix rot2 = GetBgRot(p);
			WVector move = p->m_spawnDir * rot2;
			move.x += windVec.x;
			move.y += windVec.y;
			move.z += windVec.z;
			pos.x = pos.x + dt * move.x;
			pos.y = move.y * dt + pos.y;
			pos.z = (move.z) * dt + pos.z;

			WVector dir = p->m_spawnDir;
			float drag =
				p->m_bGrounded ? m_param.m_groundFriction : m_param.m_friction;
			dir.Normalize();
			WVector step;
			step.x = drag * dt * dir.x;
			step.y = dt * dir.y * drag;
			step.z = dt * dir.z * drag;
			if ((p->m_spawnDir.x > 0.0f && p->m_spawnDir.x < step.x) ||
				(p->m_spawnDir.x < 0.0f && step.x < p->m_spawnDir.x))
				p->m_spawnDir.x = 0.0f;
			else
				p->m_spawnDir.x = p->m_spawnDir.x - step.x;
			if ((p->m_spawnDir.y > 0.0f && p->m_spawnDir.y < step.y) ||
				(p->m_spawnDir.y < 0.0f && step.y < p->m_spawnDir.y))
				p->m_spawnDir.y = 0.0f;
			else
				p->m_spawnDir.y = p->m_spawnDir.y - step.y;
			if ((p->m_spawnDir.z > 0.0f && p->m_spawnDir.z < step.z) ||
				(p->m_spawnDir.z < 0.0f && step.z < p->m_spawnDir.z))
				p->m_spawnDir.z = 0.0f;
			else
				p->m_spawnDir.z = p->m_spawnDir.z - step.z;

			if (!p->m_bGrounded)
			{
				for (std::list<sGravity>::iterator g =
						 m_param.m_gravity.begin();
					g != m_param.m_gravity.end(); ++g)
				{
					WVector gp = GetBgPos();
					WVector d;
					d.x = ((*g).m_pos.x + gp.x) - pos.x;
					d.y = (gp.y + (*g).m_pos.y) - pos.y;
					d.z = (gp.z + (*g).m_pos.z) - pos.z;
					float dist = (float)sqrt(d.x * d.x + d.y * d.y + d.z * d.z);
					float gx, gy, gz;
					if ((*g).m_radius <= dist)
					{
						WVector& n = d.Normalize();
						float f = (*g).m_far;
						gx = dt * n.x * f;
						gy = dt * n.y * f;
						gz = dt * n.z * f;
					}
					else
					{
						float d2 = dist * dist;
						if (d2 < 0.5f)
							d2 = 0.5f;
						WVector& n = d.Normalize();
						float f = (*g).m_near;
						float k = 1.0f / d2;
						gx = dt * n.x * f * k;
						gy = dt * n.y * f * k;
						gz = dt * n.z * f * k;
					}
					p->m_spawnDir.x = gx + p->m_spawnDir.x;
					p->m_spawnDir.y = gy + p->m_spawnDir.y;
					p->m_spawnDir.z = gz + p->m_spawnDir.z;
				}

				WVector gbase = GetBgPos(p);
				float groundY = gbase.y + m_param.m_groundX;
				if (!((RbFxBoxVtbl*)m_pFxBox)
						->CheckGroundCollision(prevPos, pos, groundY,
							m_param.m_groundZ))
				{
					p->m_spawnDir.x =
						dt * m_param.m_gravityVec.x + p->m_spawnDir.x;
					p->m_spawnDir.y =
						dt * m_param.m_gravityVec.y + p->m_spawnDir.y;
					p->m_spawnDir.z =
						dt * m_param.m_gravityVec.z + p->m_spawnDir.z;
				}
				else
				{
					float diff = groundY - (pos.y - m_param.m_groundZ);
					if (diff <= 0.0f)
						diff = (pos.y - m_param.m_groundZ) - groundY;
					if (pos.y <= prevPos.y)
						pos.y = groundY + m_param.m_groundZ;
					else
						pos.y = groundY - m_param.m_groundZ;
					float bounce = m_param.m_groundY * p->m_spawnDir.y * -0.01f;
					p->m_spawnDir.y = bounce;
					if (bounce <= 0.0f)
						bounce = -bounce;
					if ((diff < 0.03f && bounce < diff) || bounce < 0.01f ||
						(p->m_spawnDir.x + m_param.m_gravityVec.x) *
									p->m_spawnDir.x +
								(p->m_spawnDir.y + m_param.m_gravityVec.y) *
									p->m_spawnDir.y +
								(p->m_spawnDir.z + m_param.m_gravityVec.z) *
									p->m_spawnDir.z <
							0.0f)
					{
						p->m_spawnDir.y = 0.0f;
						p->m_bGrounded = true;
					}
					for (std::list<sSpawn>::iterator s =
							 m_param.m_spawn.begin();
						s != m_param.m_spawn.end(); ++s)
					{
						if ((*s).m_type == 3)
							Spawn(*s, p);
					}
				}

				if (m_param.m_rotate.size() != 0)
				{
					WVector bg = GetBgPos();
					WVector rel;
					rel.x = pos.x - bg.x;
					rel.y = pos.y - bg.y;
					rel.z = pos.z - bg.z;
					WVector turned = rel * mat;
					WVector bg2 = GetBgPos();
					pos.x = turned.x + bg2.x;
					pos.y = turned.y + bg2.y;
					pos.z = turned.z + bg2.z;
				}
			}
		}

		// color over life
		for (std::list<sFade>::iterator f = m_param.m_fade.begin();
			f != m_param.m_fade.end(); f++)
		{
			if ((*f).m_time <= pct && pct <= (*f).m_end)
			{
				for (int i = 0; i < 4; i++)
				{
					unsigned char c;
					if ((*f).m_end - (*f).m_time > 0.0f)
						c = (unsigned char)(int)((pct - (*f).m_time) *
								(float)((*f).m_endColor[i] - (*f).m_color[i]) /
								((*f).m_end - (*f).m_time) +
							(float)(*f).m_color[i]);
					else
						c = (*f).m_color[i];
					p->m_rgba[3 - i] = c;
				}
			}
		}

		bool bAdvance = true;
		if (p->m_pSprite == 0 && p->m_pPet == 0)
		{
		}
		else
		{
			// angle
			for (std::list<sRot>::iterator r = m_param.m_rot.begin();
				r != m_param.m_rot.end(); r++)
			{
				if ((*r).m_start <= pct && pct <= (*r).m_end)
				{
					float val;
					if ((*r).m_end - (*r).m_start > 0.0f)
						val = ((pct - (*r).m_start) *
								  ((*r).m_endVal - (*r).m_startVal)) /
								((*r).m_end - (*r).m_start) +
							(*r).m_startVal;
					else
						val = (*r).m_startVal;
					float vx = dt * p->m_rotSpeed.x * val * 0.01f;
					float vy = dt * p->m_rotSpeed.y * val * 0.01f;
					float vz = dt * p->m_rotSpeed.z * val * 0.01f;
					p->m_angle.x = vx + p->m_angle.x;
					p->m_angle.y = vy + p->m_angle.y;
					p->m_angle.z = vz + p->m_angle.z;
				}
			}
			// size
			for (std::list<sSize>::iterator sz = m_param.m_size.begin();
				sz != m_param.m_size.end(); sz++)
			{
				if ((*sz).m_start <= pct && pct <= (*sz).m_end)
				{
					float val;
					if ((*sz).m_end - (*sz).m_start > 0.0f)
						val = ((pct - (*sz).m_start) *
								  ((*sz).m_endVal - (*sz).m_startVal)) /
								((*sz).m_end - (*sz).m_start) +
							(*sz).m_startVal;
					else
						val = (*sz).m_startVal;
					p->m_size = val * p->m_size0 * 0.01f;
				}
			}
			// frame
			if (!p->m_bGrounded)
			{
				for (std::list<sFrame>::iterator fr = m_param.m_frame.begin();
					fr != m_param.m_frame.end(); fr++)
				{
					if ((*fr).m_start <= pct && pct <= (*fr).m_end &&
						(*fr).m_interval <= m_time - p->m_lastFrameTime)
					{
						p->m_lastFrameTime = m_time;
						if ((*fr).m_first == -1)
							p->m_frame = rand() % (*fr).m_last;
						else
						{
							p->m_frame++;
							if (p->m_frame > (*fr).m_last)
								p->m_frame = (*fr).m_first;
						}
					}
				}
			}
			bool bInside = true;
			if (m_param.m_lifetime == 0 && elapsed != 0.0f)
			{
				WMatrix boneMat;
				GetBoneMatrix(&boneMat);
				WMatrix boneRot;
				boneMat.GetRotMatrix(&boneRot);
				WVector bg = GetBgPos(p);
				WVector rel;
				rel.x = pos.x - bg.x;
				rel.y = pos.y - bg.y;
				rel.z = pos.z - bg.z;
				WVector local = rel * ~boneRot;
				if (!m_param.m_bSphere)
				{
					float minX = m_param.m_aabb.min.x - 0.00001f;
					float minY = m_param.m_aabb.min.y - 0.00001f;
					float minZ = m_param.m_aabb.min.z - 0.00001f;
					float maxX = m_param.m_aabb.max.x + 0.00001f;
					float maxY = m_param.m_aabb.max.y + 0.00001f;
					float maxZ = m_param.m_aabb.max.z + 0.00001f;
					bInside = minX < local.x && local.x < maxX &&
						minY < local.y && local.y < maxY && minZ < local.z &&
						local.z < maxZ;
				}
				else
				{
					float dx = local.x - m_param.m_aabb.min.x;
					bInside = (float)sqrt(dx * dx +
								  (local.y - m_param.m_aabb.min.y) *
									  (local.y - m_param.m_aabb.min.y) +
								  (local.z - m_param.m_aabb.min.z) *
									  (local.z - m_param.m_aabb.min.z)) <
						m_param.m_aabb.max.y + 0.0001f;
				}
				if (bInside)
					p->m_alpha = CalcFade(&area, &local, &m_param.m_aabb,
						m_param.m_coreMin, m_param.m_coreMax,
						m_param.m_bSphere);
			}
			if (!bInside)
			{
				InitParticle(m_param.m_bRegenAtSamePos, p, 100.0f, 0.0f, area, true);
				++pit;
				bRespawn = false;
				continue;
			}
			if (p->m_pPet)
			{
				if (p->m_pPet->GetPet())
					p->m_pPet->SetAlpha(
						(unsigned char)((float)p->m_rgba[3] * p->m_alpha *
						m_param.m_alpha)); // 983 Process 00987420: * Param+0x13c
				p->m_pPet->SetLight();
				if (m_param.m_bPetBillboard)
				{
					p->m_pPet->SetPos(pos + p->m_offset);
					p->m_pPet->SetRotate(p->m_angle);
					// 983 00987420 -> 0070ab50 + SetRotate(float) 0070a2c0: pet de frente
					// para a camera (g_camera+0x24 = 0xf024c4, direcao = za * -1)
					p->m_pPet->SetRotate(CalcDeltaAngle(
						g_camera.pivot - p->m_pPet->GetPos(),
						p->m_pPet->GetDirection()));
				}
				else
				{
					p->m_pPet->SetPos(pos);
					p->m_pPet->SetRotate(p->m_angle);
				}
				p->m_pPet->MultRotate(&m_bgRot);
				p->m_pPet->SetScale(p->m_size);
				p->m_pPet->Process(elapsed * 0.001f);
			}
		}

		// spawn
		for (std::list<sSpawn>::iterator s = m_param.m_spawn.begin();
			s != m_param.m_spawn.end(); ++s)
		{
			if ((*s).m_type == 1 && p->m_nSpawn == 0 &&
				(*s).m_interval < pct + 1.0f)
				Spawn(*s, p);
			if ((*s).m_type == 0 && (*s).m_interval < m_time - p->m_lastSpawnTime &&
				p->m_nSpawn < (*s).m_max)
			{
				p->m_lastSpawnTime = m_time;
				Spawn(*s, p);
			}
		}

		if (p->m_lifeLeft > 0.0f && !bRespawn)
		{
			p->m_lifeLeft = p->m_lifeLeft - elapsed;
			if (p->m_lifeLeft <= 0.0f)
			{
				for (std::list<sSpawn>::iterator s = m_param.m_spawn.begin();
					s != m_param.m_spawn.end(); ++s)
				{
					if ((*s).m_type == 2)
						Spawn(*s, p);
				}
				if (!m_param.m_bRegenAtSamePos)
				{
					delete p;
					pit = m_particles.erase(pit);
					continue;
				}
				bRespawn = true;
				InitParticle(m_param.m_bRegenAtSamePos, p, 100.0f, 0.0f, area, true);
				continue;
			}
		}
		// 983 (Process 009891b0): tempo de vida do "spray_kill" da sequência
		if (p->m_killTime != -1)
		{
			p->m_killTime = (int)((float)p->m_killTime - elapsed);
			if (p->m_killTime <= 0)
			{
				delete p;
				pit = m_particles.erase(pit);
				continue;
			}
		}

		// bubble
		if (m_param.m_bubble.size() != 0)
		{
			std::list<WVector>::iterator ph = p->m_trail.begin();
			WVector sum;
			sum.x = sum.y = sum.z = 0.0f;
			for (std::list<sBubble>::iterator b = m_param.m_bubble.begin();
				b != m_param.m_bubble.end(); ++b)
			{
				if (ph == p->m_trail.end())
				{
					p->m_trail.push_back(
						WVector((float)(rand() % 1001) * 0.0062831853f,
							(float)(rand() % 1001) * 0.0062831853f,
							(float)(rand() % 1001) * 0.0062831853f));
					--ph;
				}
				WVector& phase = *ph++;
				sum.x = (float)(cos((*b).m_freq.x * m_time * 0.0062831853f +
									phase.x) *
						(*b).m_amp.x +
					sum.x);
				sum.y = (float)(cos((*b).m_freq.y * m_time * 0.0062831853f +
									phase.y) *
						(*b).m_amp.y +
					sum.y);
				sum.z = (float)(cos((*b).m_freq.z * m_time * 0.0062831853f +
									phase.z) *
						(*b).m_amp.z +
					sum.z);
			}
			WVector vel = p->m_spawnDir;
			WVector up;
			if (vel.x >= 0.00001f || vel.x <= -0.00001f || vel.z >= 0.00001f ||
				vel.z <= -0.00001f)
			{
				up.x = WVector::UNIT_POS_Y.y * vel.z -
					WVector::UNIT_POS_Y.z * vel.y;
				up.y = WVector::UNIT_POS_Y.z * vel.x -
					WVector::UNIT_POS_Y.x * vel.z;
				up.z = WVector::UNIT_POS_Y.x * vel.y -
					WVector::UNIT_POS_Y.y * vel.x;
			}
			else
				up = WVector::UNIT_POS_Z;
			WVector side = up;
			WVector cross;
			cross.x = side.z * vel.y - vel.z * side.y;
			cross.y = vel.z * side.x - side.z * vel.x;
			cross.z = side.y * vel.x - vel.y * side.x;
			WVector& nSide = side.Normalize();
			WVector a;
			a.x = sum.z * nSide.x;
			a.y = sum.z * nSide.y;
			a.z = sum.z * nSide.z;
			WVector& nCross = cross.Normalize();
			WVector b2;
			b2.x = sum.y * nCross.x;
			b2.y = sum.y * nCross.y;
			b2.z = sum.y * nCross.z;
			WVector& nVel = vel.Normalize();
			float tx = sum.x * nVel.x + b2.x + a.x;
			float ty = sum.x * nVel.y + b2.y + a.y;
			float tz = sum.x * nVel.z + b2.z + a.z;
			p->m_offset.x = tx * dt + p->m_offset.x;
			p->m_offset.y = ty * dt + p->m_offset.y;
			p->m_offset.z = tz * dt + p->m_offset.z;
		}

		// sound
		if (m_param.m_sound.size() != 0)
		{
			for (std::list<sSound>::iterator snd = m_param.m_sound.begin();
				snd != m_param.m_sound.end(); ++snd)
			{
				if ((*snd).m_volume >= 0.0f)
				{
					// CFxBox+0x17c (983 SetCamera 0096c7c0; 645 +0x168): camera; 983 009897c3 usa p+0x08 (m_origin), nao o pos do mundo
					const char* cam = (const char*)FXBOX_CAMERA983(m_pFxBox);
					WVector rel;
					rel.x = p->m_origin.x - *(float*)(cam + 0x24);
					rel.y = p->m_origin.y - *(float*)(cam + 0x28);
					rel.z = p->m_origin.z - *(float*)(cam + 0x2c);
					g_audio->PlaySfx((*snd).m_name, &rel, 5, 0, 0, 0.0f,
						(*snd).m_volume);
				}
				else
					g_audio->PlaySfx((*snd).m_name, 0, 0, 0, 0, 0.5f, 200.0f);
			}
			m_param.m_sound.clear();
		}

		// back to local space
		WMatrix toLocal = GetBgRot(p);
		WVector bg3 = GetBgPos(p);
		WVector rel3;
		rel3.x = pos.x - bg3.x;
		rel3.y = pos.y - bg3.y;
		rel3.z = pos.z - bg3.z;
		p->m_origin = rel3 * ~toLocal;
		++pit;
		bRespawn = false;
	}

	m_time = elapsed + m_time;
	if (m_particles.size() == 0 && !bGenerated)
		return false;
	return true;
}

void CFxSpray::Display(WView* view)
{
	if (!m_bActive)
		return;

	float y = m_attachY == g_HUGE ? view->GetHeight() * 0.5f : m_attachY;
	float x = m_attachX == g_HUGE ? view->GetWidth() * 0.5f : m_attachX;
	view->SetScreenCenter(x, y);
	view->UpdateCamera();
	m_nDrawn = 0;
	m_nTailDrawn = 0;

	// CFxBox+0x124: debug display flag
	if (*((bool*)m_pFxBox + 0x124))
	{
		Waabb& box = m_param.m_aabb;
		if (!m_param.m_bSphere)
		{
			view->DrawAABB(box + GetBgPos(), 0xffff0000, false, 0);
			WVector center = (box.min + box.max) * 0.5f;
			view->DrawSphere(center + GetBgPos(), 0.5f, 0xffff0000, 10, 0);
		}
		else
		{
			view->DrawSphere(WSphere(box.min, box.max.x), 0xffff0000, 0x40, 0);
			view->DrawSphere(WSphere(box.min, box.max.y), 0xffff0000, 0x40, 0);
		}
		for (std::list<sGravity>::iterator it = m_param.m_gravity.begin();
			it != m_param.m_gravity.end(); ++it)
			view->DrawSphere(GetBgPos() + (*it).m_pos, 0.3f, 0xffffff00, 10, 0);
	}

	for (std::list<CFxParticle*>::iterator it = m_particles.begin();
		it != m_particles.end(); ++it)
	{
		m_nDrawn++;
		CFxParticle* particle = *it;
		WVector pos = GetBgPos(particle);
		WMatrix rot = GetBgRot(particle);
		WVector drawPos = pos + (particle->m_offset + particle->m_origin) * rot;
		m_drawPos = GetBgPos(particle);
		DrawSprite(view, particle, drawPos);
		DrawTail(view, particle, drawPos);
		DrawBandana(view, particle, drawPos);
	}
	view->SetScreenCenter(view->GetWidth() * 0.5f, view->GetHeight() * 0.5f);
	view->UpdateCamera();
}

WMatrix CFxSpray::Rotate(float time, float scale)
{
	WMatrix mat;
	mat.xx = mat.yy = mat.zz = 1.0f;
	mat.pivot.x = mat.pivot.y = mat.pivot.z = 0.0f;
	mat.yx = mat.zx = mat.xy = mat.zy = mat.xz = mat.yz = 0.0f;
	std::list<sNozzleRot>::iterator end = m_param.m_rotate.end();
	for (std::list<sNozzleRot>::iterator it = m_param.m_rotate.begin();
		it != end; ++it)
	{
		sNozzleRot* rot = &*it;
		if (time >= (float)rot->m_start && time <= (float)rot->m_end)
		{
			float val;
			if (time - (float)rot->m_start > 0.0f)
				val = (rot->m_endVal - rot->m_startVal) *
						(time - (float)rot->m_start) /
						(float)(rot->m_end - rot->m_start) +
					rot->m_startVal;
			else
				val = rot->m_startVal;
			mat = RotMat(rot->m_axis, val * scale);
		}
	}
	return mat;
}

WMatrix CFxSpray::RotateNozzle(float time, float scale)
{
	WMatrix mat;
	mat.xx = mat.yy = mat.zz = 1.0f;
	mat.pivot.x = mat.pivot.y = mat.pivot.z = 0.0f;
	mat.yx = mat.zx = mat.xy = mat.zy = mat.xz = mat.yz = 0.0f;
	std::list<sNozzleRot>::iterator end = m_param.m_nozzleRot.end();
	for (std::list<sNozzleRot>::iterator it = m_param.m_nozzleRot.begin();
		it != end; ++it)
	{
		sNozzleRot* rot = &*it;
		if (time >= (float)rot->m_start && time <= (float)rot->m_end)
		{
			float val;
			if (time - (float)rot->m_start > 0.0f)
				val = (rot->m_endVal - rot->m_startVal) *
						(time - (float)rot->m_start) /
						(float)(rot->m_end - rot->m_start) +
					rot->m_startVal;
			else
				val = rot->m_startVal;
			mat = RotMat(rot->m_axis, val * scale);
		}
	}
	return mat;
}

void CFxSpray::Revolute(float time, float scale)
{
	// 983 0097fca0/009800a0: m_revol +0x14ec, m_rotate +0x14e0, m_revolGrand +0x14f8, m_nozzleRot +0x14d4, m_pos +0x1138, m_offset +0x1150
	C_ASSERT_VC10(offsetof(CFxSpray, m_param.m_revol) == 0x14ec && offsetof(CFxSpray, m_param.m_rotate) == 0x14e0);
	C_ASSERT_VC10(offsetof(CFxSpray, m_param.m_revolGrand) == 0x14f8 && offsetof(CFxSpray, m_param.m_nozzleRot) == 0x14d4);
	C_ASSERT_VC10(offsetof(CFxSpray, m_pos) == 0x1138 && offsetof(CFxSpray, m_offset) == 0x1150);
	std::list<sRevol>::iterator end = m_param.m_revol.end();
	for (std::list<sRevol>::iterator it = m_param.m_revol.begin(); it != end;
		++it)
	{
		sRevol* rev = &*it;
		if ((float)rev->m_start <= time && time <= (float)rev->m_end)
		{
			float val;
			if (rev->m_end - rev->m_start > 0)
				val = (rev->m_endVal - rev->m_startVal) *
						(time - (float)rev->m_start) /
						(float)(rev->m_end - rev->m_start) +
					rev->m_startVal;
			else
				val = rev->m_startVal;
			WMatrix mat = RotMat(rev->m_axis, val * scale);
			WVector prev = m_pos;
			m_pos = (m_pos - rev->m_center) * mat + rev->m_center;
			m_offset += prev;
			m_offset = (m_offset - rev->m_center) * mat + rev->m_center;
			m_offset -= m_pos;
			for (std::list<sNozzleRot>::iterator rot = m_param.m_rotate.begin();
				rot != m_param.m_rotate.end(); ++rot)
			{
				(*rot).m_axis += prev;
				(*rot).m_axis =
					((*rot).m_axis - rev->m_center) * mat + rev->m_center;
				(*rot).m_axis -= m_pos;
			}
		}
	}
}

void CFxSpray::RevoluteGrand(float time, float scale)
{
	std::list<sRevol>::iterator end = m_param.m_revolGrand.end();
	for (std::list<sRevol>::iterator it = m_param.m_revolGrand.begin();
		it != end; ++it)
	{
		sRevol* rev = &*it;
		if ((float)rev->m_start <= time && time <= (float)rev->m_end)
		{
			float val;
			if (rev->m_end - rev->m_start > 0)
				val = (rev->m_endVal - rev->m_startVal) *
						(time - (float)rev->m_start) /
						(float)(rev->m_end - rev->m_start) +
					rev->m_startVal;
			else
				val = rev->m_startVal;
			WMatrix mat = RotMat(rev->m_axis, val * scale);
			WVector prev = m_pos;
			m_pos = (m_pos - rev->m_center) * mat + rev->m_center;
			m_offset += prev;
			m_offset = (m_offset - rev->m_center) * mat + rev->m_center;
			m_offset -= m_pos;
			for (std::list<sNozzleRot>::iterator rot =
					 m_param.m_nozzleRot.begin();
				rot != m_param.m_nozzleRot.end(); ++rot)
			{
				(*rot).m_axis += prev;
				(*rot).m_axis =
					((*rot).m_axis - rev->m_center) * mat + rev->m_center;
				(*rot).m_axis -= m_pos;
			}
			for (std::list<sRevol>::iterator sub = m_param.m_revol.begin();
				sub != m_param.m_revol.end(); ++sub)
			{
				(*sub).m_axis += (*sub).m_center;
				(*sub).m_axis =
					((*sub).m_axis - rev->m_center) * mat + rev->m_center;
				(*sub).m_center =
					((*sub).m_center - rev->m_center) * mat + rev->m_center;
				(*sub).m_axis -= (*sub).m_center;
			}
		}
	}
}

// moves pos back onto the plane (normal nx/ny/nz) it crossed by dist
#define SLIDE_TO_PLANE(pos, dist, denom, nx, ny, nz) \
	do \
	{ \
		float y_ = (dist) * (ny); \
		float z_ = (dist) * (nz); \
		float k_ = 1.0f / (denom); \
		float pz_ = z_ * k_; \
		float x_ = (nx) * (dist) * k_ * -1.0f; \
		(pos).x = x_ + (pos).x; \
		(pos).y = y_ * k_ * -1.0f + (pos).y; \
		(pos).z = pz_ * -1.0f + (pos).z; \
	} while (0)

bool CFxSpray::InitParticle(bool bChild, CFxParticle* particle, float speed,
	float rate, Waabb& area, bool bInit)
{
	if (m_param.m_spriteName[0] && m_pFxBox->m_pResMng)
	{
		if (particle->m_pPet)
		{
			delete particle->m_pPet;
			particle->m_pPet = 0;
		}
		particle->m_pPet = new CPetBody(m_param.m_spriteName, true);
		if (particle->m_pPet)
		{
			if (particle->m_pPet->GetPet())
			{
				// 983 00980690 -> WPuppet::DisableFog 00b6a320 no pet novo
				particle->m_pPet->GetPet()->DisableFog();
				w_motion_data* motion =
					particle->m_pPet->GetPet()->GetFirstMotionData();
				if (motion)
					particle->m_pPet->SetMotion(motion->name, false, 0.0f,
						false);
			}
			// D863: 983 CPetBody+0x109 = m_bGlobalLight (cl novo 0x105: o cru escrevia no m_light)
			PETBODY_GLOBALLIGHT983(particle->m_pPet, m_param.m_bPetLight);
			WPuppet* pet = particle->m_pPet->GetPet();
			if (pet)
				pet->CalcBound((Waabb*)0, true);
		}
	}
	particle->m_bNoWind = false;
	particle->m_pSprite = m_param.m_pSprite;
	if (!bChild)
	{
		WVector origin;
		if (!m_param.m_bSphere)
		{
			origin = m_pFxBox->Random(m_param.m_aabb.min, m_param.m_aabb.max);
		}
		else if (m_param.m_bRing)
		{
			float radius = m_param.m_aabb.max.x;
			float angle = m_ringAngle;
			int total = 0;
			for (std::list<sGen>::iterator it = m_param.m_gen.begin();
				it != m_param.m_gen.end(); ++it)
				total += (*it).m_count;
			float height = m_param.m_aabb.max.z;
			int r = rand();
			r = r % 20001;
			particle->m_origin.x =
				(float)r * height * 0.00005f - m_param.m_aabb.max.z * 0.5f;
			float rad = angle * g_DEGTORAD;
			particle->m_origin.y = (float)sin(rad) * radius;
			particle->m_origin.z = (float)cos(rad) * radius;
			m_ringAngle = 360.0f / (float)total + m_ringAngle;
			goto Placed;
		}
		else
		{
			origin = m_pFxBox->Random(m_param.m_aabb.min, m_param.m_aabb.max.x,
				m_param.m_aabb.max.y, m_param.m_aabb.max.z);
		}
		particle->m_origin.x = origin.x;
		particle->m_origin.y = origin.y;
		particle->m_origin.z = origin.z;
	}
Placed:
	particle->m_offset.z = particle->m_offset.y = particle->m_offset.x = 0.0f;
	WVector bgPos = GetBgPos();
	particle->m_pos.x = bgPos.x;
	particle->m_pos.y = bgPos.y;
	particle->m_pos.z = bgPos.z;
	particle->m_rot = m_bgRot;
	particle->m_bGrounded = false;

	WVector dir =
		m_pFxBox->Random(m_param.m_initVel.min, m_param.m_initVel.max);
	float scale = speed * 0.01f;
	dir.x = dir.x * scale;
	particle->m_spawnDir.x = dir.x;
	dir.y = dir.y * scale;
	particle->m_spawnDir.y = dir.y;
	dir.z = scale * dir.z;
	particle->m_spawnDir.z = dir.z;

	WVector cross;
	WVector* pAxis;
	if (dir.x == 0.0f && dir.z == 0.0f)
	{
		pAxis = (WVector*)&WVector::UNIT_POS_Z;
	}
	else
	{
		pAxis = &cross;
		cross.x = dir.y * WVector::UNIT_POS_Y.z - WVector::UNIT_POS_Y.y * dir.z;
		cross.y = WVector::UNIT_POS_Y.x * dir.z - WVector::UNIT_POS_Y.z * dir.x;
		cross.z = WVector::UNIT_POS_Y.y * dir.x - dir.y * WVector::UNIT_POS_Y.x;
	}
	WVector cur;
	cur.x = pAxis->x;
	cur.y = pAxis->y;
	cur.z = pAxis->z;
	int r = rand();
	WVector axis;
	axis.x = cur.x - particle->m_origin.x;
	axis.y = cur.y - particle->m_origin.y;
	axis.z = cur.z - particle->m_origin.z;
	WMatrix rot = RotMat(axis, (float)(r % 20001) * rate * 0.00005f);
	cur.x = rot.xx * particle->m_spawnDir.x + rot.xz * particle->m_spawnDir.z +
		rot.xy * particle->m_spawnDir.y + rot.xm;
	cur.y = rot.yx * particle->m_spawnDir.x + rot.yz * particle->m_spawnDir.z +
		rot.yy * particle->m_spawnDir.y + rot.ym;
	cur.z = rot.zx * particle->m_spawnDir.x + rot.zz * particle->m_spawnDir.z +
		rot.zy * particle->m_spawnDir.y + rot.zm;
	particle->m_spawnDir.x = cur.x;
	particle->m_spawnDir.y = cur.y;
	particle->m_spawnDir.z = cur.z;
	r = rand();
	rot = RotMat(dir, (float)(r % 20001) * 0.00031415927f);
	{
		float y = particle->m_spawnDir.y;
		float x = particle->m_spawnDir.x;
		float z = particle->m_spawnDir.z;
		float nx = particle->m_spawnDir.x * rot.xx +
			rot.xz * particle->m_spawnDir.z + rot.xy * particle->m_spawnDir.y +
			rot.xm;
		float ny = rot.yz * particle->m_spawnDir.z +
			rot.yy * particle->m_spawnDir.y + rot.yx * x + rot.ym;
		float nz =
			rot.zz * particle->m_spawnDir.z + rot.zy * y + rot.zx * x + rot.zm;
		particle->m_spawnDir.x = nx;
		particle->m_spawnDir.y = ny;
		particle->m_spawnDir.z = nz;
	}
	cur.x = m_rot1190.xx * particle->m_spawnDir.x +
		m_rot1190.xy * particle->m_spawnDir.y +
		m_rot1190.xz * particle->m_spawnDir.z + m_rot1190.xm;
	cur.y = m_rot1190.yy * particle->m_spawnDir.y +
		m_rot1190.yx * particle->m_spawnDir.x +
		m_rot1190.yz * particle->m_spawnDir.z + m_rot1190.ym;
	cur.z = m_rot1190.zy * particle->m_spawnDir.y +
		m_rot1190.zx * particle->m_spawnDir.x +
		m_rot1190.zz * particle->m_spawnDir.z + m_rot1190.zm;
	particle->m_spawnDir.x = cur.x;
	particle->m_spawnDir.y = cur.y;
	particle->m_spawnDir.z = cur.z;

	cur.x = 0.0f;
	cur.y = 0.0f;
	cur.z = 0.0f;
	particle->m_windRatio =
		m_pFxBox->Random(m_param.m_windEnable[0], m_param.m_windEnable[1]);
	particle->m_unused983_14.Reset();

	WMatrix bone;
	GetBoneMatrix(&bone);
	WMatrix boneRot;
	bone.GetRotMatrix(&boneRot);
	WVector out;
	unsigned int mode = m_param.m_extOrient;
	if (mode == 0)
	{
		float len = particle->m_windRatio;
		cur.y = len * m_param.m_wind.y;
		cur.z = len * m_param.m_wind.z;
		WVector sum;
		sum.x = len * m_param.m_wind.x + particle->m_spawnDir.x;
		sum.y = cur.y + particle->m_spawnDir.y;
		sum.z = cur.z + particle->m_spawnDir.z;
		WMatrix inv = ~boneRot;
		out.x = sum.x * inv.xx + sum.y * inv.xy + sum.z * inv.xz + inv.xm;
		out.y = sum.x * inv.yx + sum.y * inv.yy + sum.z * inv.yz + inv.ym;
		out.z = sum.x * inv.zx + sum.y * inv.zy + sum.z * inv.zz + inv.zm;
		cur.x = out.x;
		cur.y = out.y;
		cur.z = out.z;
	}
	else
	{
		WMatrix* pMat;
		WMatrix rotMat;
		if (mode == 7)
		{
			float len = particle->m_windRatio;
			cur.x = len * m_param.m_wind.x;
			cur.y = len * m_param.m_wind.y;
			cur.z = len * m_param.m_wind.z;
			WMatrix inv = ~boneRot;
			WVector t = cur * inv;
			out.x = t.x + particle->m_spawnDir.x;
			out.y = particle->m_spawnDir.y + t.y;
			out.z = particle->m_spawnDir.z + t.z;
			pMat = &boneRot;
			cur.x = out.x;
			cur.y = out.y;
			cur.z = out.z;
		}
		else
		{
			float angX = 0.0f;
			float angY = 0.0f;
			float angZ = 0.0f;
			WVector axisVec;
			if (mode & 1)
			{
				axisVec.x = WVector::UNIT_POS_Y.x;
				axisVec.y = WVector::UNIT_POS_Y.y;
				axisVec.z = WVector::UNIT_POS_Y.z;
				cur = axisVec * boneRot;
				out.y = cur.y;
				out.z = cur.z;
				out.x = 0.0f;
				angX = AngleBetween(axisVec, out);
				float a = out.z;
				if (Wabs(a) >= 0.00001f)
				{
					a = 1.0f;
					if (out.z < 0.0f)
						a = -1.0f;
					angX = a * angX;
				}
			}
			if (m_param.m_extOrient & 2)
			{
				axisVec.x = WVector::UNIT_POS_Z.x;
				axisVec.y = WVector::UNIT_POS_Z.y;
				axisVec.z = WVector::UNIT_POS_Z.z;
				cur = axisVec * boneRot;
				out.x = cur.x;
				out.z = cur.z;
				out.y = 0.0f;
				angY = AngleBetween(axisVec, out);
				float a = out.x;
				if (Wabs(a) >= 0.00001f)
				{
					a = 1.0f;
					if (out.x < 0.0f)
						a = -1.0f;
					angY = a * angY;
				}
			}
			if (m_param.m_extOrient & 4)
			{
				axisVec.x = WVector::UNIT_POS_X.x;
				axisVec.y = WVector::UNIT_POS_X.y;
				axisVec.z = WVector::UNIT_POS_X.z;
				cur = axisVec * boneRot;
				out.x = cur.x;
				out.y = cur.y;
				out.z = 0.0f;
				angZ = AngleBetween(axisVec, out);
				float a = out.y;
				if (Wabs(a) >= 0.00001f)
				{
					a = 1.0f;
					if (0.0f < out.y)
						a = -1.0f;
					angZ = a * angZ;
				}
			}
			WMatrix tmp = RotMat(WVector(angX, angY, angZ));
			WMatrix inv = ~tmp;
			rotMat = inv;
			float len = particle->m_windRatio;
			cur.x = len * m_param.m_wind.x;
			cur.y = len * m_param.m_wind.y;
			cur.z = len * m_param.m_wind.z;
			WVector t = particle->m_spawnDir * rotMat;
			WVector sum;
			sum.x = cur.x + t.x;
			sum.y = cur.y + t.y;
			sum.z = cur.z + t.z;
			WMatrix invBone = ~boneRot;
			cur = sum * invBone;
			out.x = cur.x;
			out.y = cur.y;
			out.z = cur.z;
			pMat = &rotMat;
		}
		cur = particle->m_spawnDir * *pMat;
		particle->m_spawnDir.x = cur.x;
		particle->m_spawnDir.y = cur.y;
		particle->m_spawnDir.z = cur.z;
	}

	if (bInit)
	{
		if (!(Wabs(out.x) < 0.00001f && Wabs(out.y) < 0.00001f &&
				Wabs(out.z) < 0.00001f &&
				Wabs(m_param.m_gravityVec.x) < 0.00001f &&
				WisZero(m_param.m_gravityVec.y) &&
				WisZero(m_param.m_gravityVec.z)))
		{
			if (!m_param.m_bSphere)
			{
				float nx = out.x;
				float ny = out.y;
				float nz = out.z;
				if (Wabs(out.x) < 0.00001f && Wabs(out.y) < 0.00001f &&
					Wabs(out.z) < 0.00001f)
				{
					ny = m_param.m_gravityVec.y;
					nz = m_param.m_gravityVec.z;
					nx = m_param.m_gravityVec.x;
				}
				float maxZ = m_param.m_aabb.max.z;
				float maxY = m_param.m_aabb.max.y;
				float negMinY = -m_param.m_aabb.min.y;
				float maxX = m_param.m_aabb.max.x;
				float negMinX = -m_param.m_aabb.min.x;
				float minY = m_param.m_aabb.min.y;
				float minZ = m_param.m_aabb.min.z;
				float minX = m_param.m_aabb.min.x;
				bool xNeg = false;
				bool xPos = false;
				if (nx <= 0.0001f)
				{
					if (nx < -0.0001f)
						xNeg = true;
				}
				else
					xPos = true;
				bool zNeg = false;
				bool zPos = false;
				if (nz <= 0.0001f)
				{
					if (nz < -0.0001f)
						zNeg = true;
				}
				else
					zPos = true;
				bool yNeg = false;
				bool yPos = false;
				if (ny <= 0.0001f)
				{
					if (ny < -0.0001f)
						yNeg = true;
				}
				else
					yPos = true;
				bool open = !zPos;
				WVector& o = particle->m_origin;
				if (!open)
				{
					WPlane plane(0.0f, 0.0f, 1.0f, -m_param.m_aabb.min.z);
					float d = (o.y * plane.y + o.x * plane.x) + o.z * plane.z +
						plane.dis;
					SLIDE_TO_PLANE(o, d,
						(ny * plane.y + nx * plane.x) + nz * plane.z, nx, ny,
						nz);
				}
				if (zNeg &&
					(open ||
						((o.y - minY) + (o.x - maxX)) * 0.0f -
								(o.z - maxZ) * 1.0f <=
							0.0f))
				{
					float d = ((o.y + o.x) * 0.0f - o.z * 1.0f) + maxZ;
					SLIDE_TO_PLANE(o, d, (ny + nx) * 0.0f - nz * 1.0f, nx, ny,
						nz);
					open = false;
				}
				if (yPos &&
					(open ||
						((o.z - maxZ) + (o.x - maxX)) * 0.0f + (o.y - minY) <=
							0.0f))
				{
					float d = (o.z + o.x) * 0.0f + o.y + negMinY;
					SLIDE_TO_PLANE(o, d, (nz + nx) * 0.0f + ny, nx, ny, nz);
					open = false;
				}
				if (yNeg &&
					(open ||
						((o.z - minZ) + (o.x - minX)) * 0.0f -
								(o.y - maxY) * 1.0f <=
							0.0f))
				{
					float d = ((o.z + o.x) * 0.0f - o.y * 1.0f) + maxY;
					SLIDE_TO_PLANE(o, d, (nz + nx) * 0.0f - ny * 1.0f, nx, ny,
						nz);
					open = false;
				}
				if (xPos &&
					(open ||
						((o.z - maxZ) + (o.y - minY)) * 0.0f + (o.x - minX) <=
							0.0f))
				{
					float d = (o.z + o.y) * 0.0f + negMinX + o.x;
					SLIDE_TO_PLANE(o, d, (ny + nz) * 0.0f + nx, nx, ny, nz);
					open = false;
				}
				if (xNeg &&
					(open ||
						((o.z - minZ) + (o.y - minY)) * 0.0f -
								(o.x - maxX) * 1.0f <=
							0.0f))
				{
					float d = ((o.z + o.y) * 0.0f - o.x * 1.0f) + maxX;
					SLIDE_TO_PLANE(o, d, (ny + nz) * 0.0f - nx * 1.0f, nx, ny,
						nz);
				}
			}
			else
			{
				cur.x = particle->m_origin.x - m_param.m_aabb.min.x;
				cur.y = particle->m_origin.y - m_param.m_aabb.min.y;
				cur.z = particle->m_origin.z - m_param.m_aabb.min.z;
				cur.Normalize();
				float radius = m_param.m_aabb.max.y;
				float y = cur.y * radius;
				float z = cur.z * radius * -0.998f;
				cur.x = cur.x * radius * -0.998f + m_param.m_aabb.min.x;
				cur.y = y * -0.998f + m_param.m_aabb.min.y;
				cur.z = z + m_param.m_aabb.min.z;
				particle->m_origin.x = cur.x;
				particle->m_origin.y = cur.y;
				particle->m_origin.z = cur.z;
			}
		}
	}

	WVector rotSpeed =
		m_pFxBox->Random(m_param.m_initRot.min, m_param.m_initRot.max);
	cur.x = m_rot1190.xx * rotSpeed.x + m_rot1190.xy * rotSpeed.y +
		m_rot1190.xz * rotSpeed.z + m_rot1190.xm;
	cur.y = m_rot1190.yx * rotSpeed.x + m_rot1190.yy * rotSpeed.y +
		m_rot1190.yz * rotSpeed.z + m_rot1190.ym;
	cur.z = m_rot1190.zx * rotSpeed.x + m_rot1190.zy * rotSpeed.y +
		m_rot1190.zz * rotSpeed.z + m_rot1190.zm;
	particle->m_rotSpeed.x = cur.x;
	particle->m_rotSpeed.y = cur.y;
	particle->m_rotSpeed.z = cur.z;
	WVector angle =
		m_pFxBox->Random(m_param.m_initAngle.min, m_param.m_initAngle.max);
	cur.x = m_rot1190.xx * angle.x + m_rot1190.xz * angle.z +
		m_rot1190.xy * angle.y + m_rot1190.xm;
	cur.y = m_rot1190.yz * angle.z + m_rot1190.yx * angle.x +
		m_rot1190.yy * angle.y + m_rot1190.ym;
	cur.z = m_rot1190.zz * angle.z + m_rot1190.zx * angle.x +
		m_rot1190.zy * angle.y + m_rot1190.zm;
	particle->m_angle.x = cur.x;
	particle->m_angle.y = cur.y;
	particle->m_angle.z = cur.z;

	float size = m_pFxBox->Random(m_param.m_initSize0, m_param.m_initSize1);
	particle->m_size0 = particle->m_size = size;
	particle->m_ratio = m_param.m_initSize2;
	particle->m_color = 0xffffffff;
	for (std::list<sFade>::iterator it = m_param.m_fade.begin();
		it != m_param.m_fade.end(); it++)
	{
		if ((*it).m_time == 0.0f)
		{
			particle->m_rgba[3] = (*it).m_color[0];
			particle->m_rgba[2] = (*it).m_color[1];
			particle->m_rgba[1] = (*it).m_color[2];
			particle->m_rgba[0] = (*it).m_color[3];
			break;
		}
	}
	particle->m_alpha = CalcFade(&area, &particle->m_origin, &m_param.m_aabb,
		m_param.m_coreMin, m_param.m_coreMax, m_param.m_bSphere);
	float f =
		m_pFxBox->Random((float)m_param.m_lifetime, (float)m_param.m_lifetime2);
	particle->m_lifeLeft = particle->m_lifetime = f;
	particle->m_lastFrameTime = 0;
	particle->m_lastTail = 0.0f;
	particle->m_killTime = m_killTime;
	particle->m_frame = (int)m_pFxBox->Random((float)m_param.m_initFrame0,
		(float)m_param.m_initFrame1);
	if (m_param.m_bTail > 0)
	{
		particle->m_tail.Init(m_param.m_bTail + 1);
		particle->m_bandana.Init(m_param.m_bTail + 1);
	}
	particle->m_nSpawn = 0;
	particle->m_lastSpawnTime = m_time;
	return true;
}

WVector CFxSpray::GetBgPos()
{
	if (m_pParent == 0 && m_pSequence)
	{
		if (m_param.m_bAttach)
		{
			return m_pos * m_bgRot + m_pSequence->m_pos;
		}
		return m_pSequence->m_pos + m_pos;
	}
	return m_pos;
}

void CFxSpray::MakeTail(float time, CFxParticle* particle, const WVector& pos)
{
	if (m_param.m_bTail &&
		!(time - particle->m_lastTail < m_param.m_tailInterval))
	{
		particle->m_lastTail = time;
		if (m_param.m_bandana)
		{
			TRingQ<CFxParticle::sBandana>& bandana = particle->m_bandana;
			CFxParticle::sBandana* cur = bandana.GetTail(0);
			CFxParticle::sBandana* prev = bandana.GetTail(1);
			WVector side;
			if (particle->m_spawnDir.Magnitude() < 0.0001f &&
				bandana.GetHead(0) != cur)
			{
				side = (prev->m_left - prev->m_right) / 2.0f;
			}
			else
			{
				// CFxBox+0x17c (983 SetCamera 0096c7c0; 645 +0x168): camera matrix
				const WMatrix* camera =
					FXBOX_CAMERA983(m_pFxBox);
				if (camera && m_param.m_bBandanaCameraSide)
					side = WCrossProduct(camera->za, particle->m_spawnDir);
				else
					side = WCrossProduct(WVector::UNIT_POS_Y,
						particle->m_spawnDir);
				side = particle->m_size * side.Normalize() *
					m_param.m_bandanaWidth * 0.005f;
			}
			cur->m_left = pos + side;
			cur->m_right = pos - side;
			if (bandana.GetHead(0) == cur)
				cur->m_len = 0;
			else
			{
				if ((cur->m_left - cur->m_right) *
						(prev->m_left - prev->m_right) <
					0.0f)
				{
					CFxParticle::sBandana tmp;
					tmp.m_left = cur->m_left;
					cur->m_left = cur->m_right;
					cur->m_right = tmp.m_left;
				}
				cur->m_len =
					(pos - (prev->m_left + prev->m_right) * 0.5f).Magnitude();
			}
			if (bandana.IsFull())
				bandana.StepHead(1);
			bandana.StepTail(1);
			return;
		}
		TRingQ<CFxParticle::sTail>& tail = particle->m_tail;
		CFxParticle::sTail* cur = tail.GetTail(0);
		cur->m_pos = pos;
		cur->m_frame = particle->m_frame;
		if (tail.IsFull())
			tail.StepHead(1);
		tail.StepTail(1);
	}
}

void CFxSpray::DrawSprite(WView* view, CFxParticle* particle,
	const WVector& pos)
{
	if (particle->m_pSprite)
	{
		unsigned long color =
			GetParticleColor(view, pos, particle, particle->m_color);
		if ((color & 0xff000000) == 0)
			return;
		particle->m_pSprite->SetColor(color);
		particle->m_pSprite->SetRect(particle->m_ratio * particle->m_size,
			particle->m_size, particle->m_ratio * particle->m_size * 0.5f,
			particle->m_size * 0.5f);
		if (m_param.m_zBias != 0.0f)
		{
			WVector dir = pos - view->GetCamera().pivot;
			float dist = dir.Magnitude();
			dir.Normalize();
			float cosA = dir * view->GetCamera().za;
			if (cosA <= 0.0f)
				return;
			float clip = view->GetClipNearValue() / cosA;
			if (dist < clip + m_param.m_zBias)
				dir = dir * (clip + 0.01f) + view->GetCamera().pivot;
			else
				dir = pos - dir * m_param.m_zBias;
			particle->m_pSprite->SetPos(dir);
		}
		else
			particle->m_pSprite->SetPos(pos);
		if (m_param.m_bBillboard)
		{
			particle->m_pSprite->Render(view, particle->m_angle.x,
				*(int*)((char*)m_pFxBox + 0x114) | m_param.m_blendSrc |
					0x400000, // 983 0097b710: 0x400000 (645: 0x20400000)
				particle->m_frame);
			return;
		}
		particle->m_pSprite->Render(view,
			RotMat(particle->m_angle) * GetBgRot(particle),
			*(int*)((char*)m_pFxBox + 0x114) | m_param.m_blendSrc | 0x400000, // 983 0097b710: 0x400000 (645: 0x20000000)
			particle->m_frame);
	}
	else if (particle->m_pPet)
	{
		particle->m_pPet->Display();
	}
	else
	{
		unsigned long color =
			GetParticleColor(view, pos, particle, particle->m_color);
		if (color & 0xff000000)
		{
			WVector dir = particle->m_spawnDir;
			dir = dir.Normalize() * 0.03f;
			view->DrawLine(pos, color, pos + dir, color, 0);
		}
	}
}

void CFxSpray::DrawTail(WView* view, CFxParticle* particle, const WVector& pos)
{
	if (particle->m_tail.IsEmpty())
		return;

	m_nTailDrawn++;
	unsigned long prevColor = 0xffffffff;
	int len = particle->m_tail.GetLen();
	CFxParticle::sTail* last = particle->m_tail.GetTail(1);
	unsigned char* src = particle->m_rgba;
	if (particle->m_pSprite)
	{
		unsigned long color;
		unsigned char* c = (unsigned char*)&color;
		c[0] = src[0] * (len - 1) / len + m_param.m_tailColor[3] / len;
		c[1] = src[1] * (len - 1) / len + m_param.m_tailColor[2] / len;
		c[2] = src[2] * (len - 1) / len + m_param.m_tailColor[1] / len;
		c[3] = src[3] * (len - 1) / len + m_param.m_tailColor[0] / len;
		color = GetParticleColor(view, last->m_pos, particle, color);
		if (color & 0xff000000)
		{
			particle->m_pSprite->SetColor(color);
			particle->m_pSprite->SetPos(last->m_pos);
			particle->m_pSprite->Render(view, particle->m_angle.x,
				*(int*)((char*)m_pFxBox + 0x114) | m_param.m_blendDst |
					0x400000,
				last->m_frame);
		}
	}
	else
	{
		unsigned char* c = (unsigned char*)&prevColor;
		c[0] = src[0] * (len - 1) / len + m_param.m_tailColor[3] / len;
		c[1] = src[1] * (len - 1) / len + m_param.m_tailColor[2] / len;
		c[2] = src[2] * (len - 1) / len + m_param.m_tailColor[1] / len;
		c[3] = src[3] * (len - 1) / len + m_param.m_tailColor[0] / len;
		view->DrawLine(last->m_pos,
			GetParticleColor(view, last->m_pos, particle, prevColor), pos,
			GetParticleColor(view, last->m_pos, particle, particle->m_color),
			0x400000); // 983 0097bcac: 0x400000 (era 0)
	}

	for (int i = len - 2, k = len - i; i >= 0; i--, k++)
	{
		m_nTailDrawn++;
		CFxParticle::sTail* cur = particle->m_tail.GetHead(i);
		unsigned long color;
		unsigned char* c = (unsigned char*)&color;
		c[0] = m_param.m_tailColor[3] * k / len + src[0] * i / len;
		c[1] = m_param.m_tailColor[2] * k / len + src[1] * i / len;
		c[2] = m_param.m_tailColor[1] * k / len + src[2] * i / len;
		c[3] = m_param.m_tailColor[0] * k / len + src[3] * i / len;
		if (particle->m_pSprite)
		{
			unsigned long c2 =
				GetParticleColor(view, cur->m_pos, particle, color);
			if ((c2 & 0xff000000) == 0)
				continue;
			particle->m_pSprite->SetColor(c2);
			particle->m_pSprite->SetPos(cur->m_pos);
			particle->m_pSprite->Render(view, particle->m_angle.x,
				*(int*)((char*)m_pFxBox + 0x114) | m_param.m_blendDst |
					0x400000,
				cur->m_frame);
		}
		else
			view->DrawLine(cur->m_pos, color,
				particle->m_tail.GetHead(i + 1)->m_pos, prevColor, 0x400000); // 983 0097be54
		prevColor = color;
	}
}

// per-channel blend of the particle color towards the tail color (len = segment count)
#define BLEND_BANDANA_COLOR(color, src, tail, a, k, len) \
	do \
	{ \
		unsigned char* c_ = (unsigned char*)&(color); \
		c_[0] = (src)[0] * (a) / (len) + (tail)[3] * (k) / (len); \
		c_[1] = (src)[1] * (a) / (len) + (tail)[2] * (k) / (len); \
		c_[2] = (src)[2] * (a) / (len) + (tail)[1] * (k) / (len); \
		int alpha_ = (src)[3] * (a) / (len) + (tail)[0] * (k) / (len); \
		if ((src)[3] < alpha_) \
			c_[3] = (src)[3]; \
		else \
			c_[3] = (src)[3] * (a) / (len) + (tail)[0] * (k) / (len); \
	} while (0)

void CFxSpray::DrawBandana(WView* view, CFxParticle* particle,
	const WVector& pos)
{
	TRingQ<CFxParticle::sBandana>& bandana = particle->m_bandana;
	if (bandana.IsEmpty())
		return;
	int len = bandana.GetLen();

	float total = 0.0f;
	for (int i = 1; i < len; i++)
	{
		CFxParticle::sBandana* seg = bandana.GetHead(i);
		if (seg)
			total += seg->m_len;
	}

	// CFxBox+0x16c: vertex pointers, CFxBox+0x180: 4 vertices (stride 0x28), CFxBox+0x114: texture flags
	WTVertex** polygon = FXBOX_VERTEXPTR983(m_pFxBox); // 983 CFxBox::m_pVertex (645 +0x16c)
	WTVertex* vtx = FXBOX_VERTEX983(m_pFxBox); // 983 CFxBox::m_vertex (645 +0x180)
	int texFlags = *(int*)((char*)m_pFxBox + 0x114);
	unsigned long prevColor = particle->m_color;
	m_nTailDrawn++;
	CFxParticle::sBandana* last = bandana.GetTail(1);
	float headLen = (pos - (last->m_left + last->m_right) * 0.5f).Magnitude();
	total += headLen;
	WVector edge[2];
	edge[0] = pos + (last->m_left - last->m_right) * 0.5f;
	edge[1] = pos - (last->m_left - last->m_right) * 0.5f;
	unsigned long color;
	BLEND_BANDANA_COLOR(color, particle->m_rgba, m_param.m_tailColor, len - 1,
		1, len);
	for (int j = 0; j < 4; j++)
	{
		const WVector* pair;
		unsigned long src;
		if ((j & 2) == 0)
		{
			pair = edge;
			vtx[j].tu = 0.0f;
			src = particle->m_color;
		}
		else
		{
			pair = &last->m_left;
			float u = 1.0f;
			if (m_param.m_bBandanaMonolith && total != 0.0f)
				u = headLen / total;
			vtx[j].tu = u;
			src = color;
		}
		const WVector* v = (j & 1) == 0 ? pair + 1 : pair;
		vtx[j].diffuse = GetParticleColor(view, *v, particle, src);
		vtx[j].SetPosition(*v);
	}
	if ((vtx[0].diffuse & 0xff000000) != 0 ||
		(vtx[1].diffuse & 0xff000000) != 0 ||
		(vtx[2].diffuse & 0xff000000) != 0 ||
		(vtx[3].diffuse & 0xff000000) != 0)
		view->DrawPolygonFan(polygon,
			(m_param.m_bandana & 0x7ff) | texFlags | m_param.m_blendDst |
				0x400000,
			0x200, 0);

	int i = len - 2;
	if (i >= 0)
	{
		int k = len - i;
		do
		{
			m_nTailDrawn++;
			CFxParticle::sBandana* a = bandana.GetHead(i);
			CFxParticle::sBandana* b = bandana.GetHead(i + 1);
			unsigned long cur;
			BLEND_BANDANA_COLOR(cur, particle->m_rgba, m_param.m_tailColor, i,
				k, len);
			for (int j = 0; j < 4; j++)
			{
				const WVector* pair;
				unsigned long src;
				if ((j & 2) == 0)
				{
					pair = &b->m_left;
					float u = 0.0f;
					if (m_param.m_bBandanaMonolith && total != 0.0f)
						u = headLen / total;
					vtx[j].tu = u;
					src = prevColor;
				}
				else
				{
					pair = &a->m_left;
					float u = 1.0f;
					if (m_param.m_bBandanaMonolith && total != 0.0f)
						u = (headLen + b->m_len) / total;
					vtx[j].tu = u;
					src = cur;
				}
				const WVector* v = (j & 1) == 0 ? pair + 1 : pair;
				vtx[j].diffuse = GetParticleColor(view, *v, particle, src);
				vtx[j].SetPosition(*v);
			}
			if ((vtx[0].diffuse & 0xff000000) != 0 ||
				(vtx[1].diffuse & 0xff000000) != 0 ||
				(vtx[2].diffuse & 0xff000000) != 0 ||
				(vtx[3].diffuse & 0xff000000) != 0)
				view->DrawPolygonFan(polygon,
					(m_param.m_bandana & 0x7ff) | texFlags |
						m_param.m_blendDst | 0x400000,
					0x200, 0);
			prevColor = cur;
			headLen += b->m_len;
			k++;
			i--;
		} while (i >= 0);
	}
}

unsigned long CFxSpray::GetParticleColor(const WView* view, const WVector& pos,
	const CFxParticle* particle, unsigned long color)
{
	if (particle == 0)
		return 0xffffffff;

	float alpha;
	if (m_param.m_distFar > 0.0f)
	{
		float dist = (view->GetCamera().pivot - pos).Magnitude();
		if (dist < m_param.m_distNear)
			alpha = 1.0f;
		else
		{
			if (dist > m_param.m_distFar)
				return 0;
			alpha = 1.0f -
				Between(0.0f,
					(dist - m_param.m_distNear) /
						(m_param.m_distFar - m_param.m_distNear),
					1.0f);
		}
	}
	else
		alpha = 1.0f;

	unsigned long c = color;
	if (m_param.m_lightType)
		c = ModulateColor(color, m_param.m_lightColor) | color & 0xff000000;
	return (unsigned char)((c >> 24) * m_param.m_alpha * particle->m_alpha *
			   alpha)
		<< 24 |
		c & 0xffffff;
}

void CFxSpray::SetLight(const LightSet& light)
{
	switch (m_param.m_lightType)
	{
	case 0:
		m_param.m_lightColor = 0xffffff;
		break;
	case 1:
		m_param.m_lightColor = light.diffuse;
		break;
	case 2:
		m_param.m_lightColor = light.ambient;
		break;
	case 3:
		m_param.m_lightColor = light.ambient2;
		break;
	}
}

void CFxSpray::SetWind(const WVector& wind)
{
	if (m_param.m_windEnable[0] != 0.0f || m_param.m_windEnable[1] != 0.0f)
		m_param.m_wind = wind;
}

void CFxSpray::SetDistLimit(float nearDist, float farDist)
{
	m_param.m_distNear = nearDist;
	m_param.m_distFar = farDist;
}

void CFxSpray::Spawn(const sSpawn& spawn, CFxParticle* particle)
{
	particle->m_nSpawn++;
	CFxSpray* spray = m_pFxBox->LoadSpray(spawn.m_name, 0);
	if (spray)
	{
		bool bAttach = spawn.m_bAttach;
		spray->m_param.m_bChild = bAttach;
		spray->m_pParent = bAttach ? this : 0;
		spray->m_pSequence = m_pSequence;
		spray->m_attachX = m_attachX;
		spray->m_attachY = m_attachY;
		if (m_pSequence)
		{
			spray->SetWind(*(WVector*)((char*)m_pSequence + 8));
			m_pSequence->m_spray[spray] = spray;
		}
		// 983 00986de4: com sequencia o filho nasce so em m_origin (+8); sem, origin + pos
		if (m_pSequence)
			spray->m_pos = particle->m_origin;
		else
			spray->m_pos = particle->m_origin + particle->m_pos;
		spray->m_offset = particle->m_spawnDir;
		spray->m_bActive = m_bActive;
		m_pFxBox->AddSpray(spray);
	}
}
