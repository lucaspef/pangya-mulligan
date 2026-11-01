// SPDX-License-Identifier: MIT
// fxsim.cpp - port of ProjectG's fxbox.cpp, fxsequence.cpp and fxspray.cpp.
//
// Deliberately close to the original, including its odd corners: the numbers in
// the shipped .spr and .seq files were tuned against this exact behaviour.
#include "fxsim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "wcommon.h"

const float FxBox::FRAME_TIME = 1000.0f / 30.0f;

static const float PI = 3.14159265f;
static const float DEGTORAD = PI / 180.0f;
static const float EPS = 0.00001f;

// -- math helpers the shared header does not need ---------------------------------

static inline void Add(WVector& a, const WVector& b) { a = a + b; }
static inline void Sub(WVector& a, const WVector& b) { a = a - b; }
static inline bool IsZero(float f) { return fabs(f) < EPS; }

static WVector Norm(WVector v)
{
	v.Normalize();
	return v;
}

// wmath.cpp: WMatrix::GetRotMatrix
static WMatrix RotOnly(const WMatrix& m)
{
	WMatrix r = m;
	r.xm = r.ym = r.zm = 0.0f;
	float len = (float)sqrt(r.xx * r.xx + r.yx * r.yx + r.zx * r.zx);
	if (len > 0.0f)
	{
		float s = 1.0f / len;
		r.xx *= s; r.yx *= s; r.zx *= s;
		r.xy *= s; r.yy *= s; r.zy *= s;
		r.xz *= s; r.yz *= s; r.zz *= s;
	}
	return r;
}

// wmath.inl: RotMat(axis, angle)
static WMatrix RotAxis(WVector v, float angle)
{
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	v.Normalize();
	float t = 1.0f - c;
	WMatrix m;
	m.xx = v.x * v.x * t + c;       m.yx = v.x * v.y * t - v.z * s; m.zx = v.x * v.z * t + v.y * s;
	m.xy = v.x * v.y * t + v.z * s; m.yy = v.y * v.y * t + c;       m.zy = v.y * v.z * t - v.x * s;
	m.xz = v.x * v.z * t - v.y * s; m.yz = v.y * v.z * t + v.x * s; m.zz = v.z * v.z * t + c;
	return m;
}

// wmath.inl: RotMat(angle, 0|1|2), combined X * Y * Z as CPetBody::SetRotate does
static WMatrix RotEuler(const WVector& r)
{
	float c = (float)cos(r.x), s = (float)sin(r.x);
	WMatrix x, y, z;
	x.yy = c; x.zy = -s; x.yz = s; x.zz = c;
	c = (float)cos(r.y); s = (float)sin(r.y);
	y.xx = c; y.zx = s; y.xz = -s; y.zz = c;
	c = (float)cos(r.z); s = (float)sin(r.z);
	z.xx = c; z.yx = s; z.xy = -s; z.yy = c;
	return x * y * z;
}

WMatrix FxRotEuler(const WVector& r)
{
	return RotEuler(r);
}

static float Between(float lo, float v, float hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

static float GetAngle(const WVector& a, const WVector& b)
{
	float la = a.Length();
	if (fabs(la) < EPS)
		return 0.0f;
	float lb = b.Length();
	if (fabs(lb) < EPS)
		return 0.0f;
	return (float)acos(Between(-1.0f, a.Dot(b) / (lb * la), 1.0f));
}

// Trims the same character sets the client passes to TrimA.
static std::string Trim(const std::string& s, const char* set)
{
	size_t a = s.find_first_not_of(set);
	if (a == std::string::npos)
		return std::string();
	return s.substr(a, s.find_last_not_of(set) - a + 1);
}

static std::string TrimAll(const std::string& s)
{
	return Trim(s, " \t\r\n");
}

static float F(const std::string& s) { return (float)atof(s.c_str()); }
static int I(const std::string& s) { return atoi(s.c_str()); }
static bool Is(const std::string& s, const char* t) { return !_stricmp(s.c_str(), t); }

// -- attacher ------------------------------------------------------------------------

bool FxAttacher::Get(const std::vector<WMatrix>& world, WVector* out) const
{
	WVector sum;
	int i;
	for (i = 0; i < 4; ++i)
	{
		const WMatrix* m = bone[i] >= 0 && bone[i] < (int)world.size() ? &world[bone[i]] : 0;
		if (hasPos[i])
			sum = sum + (m ? pos[i] * *m : pos[i]);
		else if (m)
			sum = sum + m->Pivot();
		else
			break;
	}
	if (i == 0)
		return false;
	*out = sum * (1.0f / i);
	return true;
}

WMatrix FxAttacher::BoneRotation(const std::vector<WMatrix>& world) const
{
	WMatrix mat;
	for (int i = 0; i < 4; ++i)
	{
		if (bone[i] < 0 || bone[i] >= (int)world.size())
			break;
		mat = mat * RotOnly(world[bone[i]]);
	}
	return mat;
}

static const std::vector<WMatrix>* g_world;

static WMatrix AttachRotation(const FxAttacher* a)
{
	return a && g_world ? a->BoneRotation(*g_world) : WMatrix();
}

// -- box -----------------------------------------------------------------------------

FxBox::FxBox(FxHost* host)
	: m_host(host)
{
}

FxBox::~FxBox()
{
	Clear();
}

void FxBox::Problem(const std::string& text)
{
	if (std::find(m_problems.begin(), m_problems.end(), text) == m_problems.end())
		m_problems.push_back(text);
}

void FxBox::Clear()
{
	ClearAll(false);
	for (std::map<std::string, FxSprayParam*>::iterator i = m_sprayCache.begin(); i != m_sprayCache.end(); ++i)
		delete i->second;
	m_sprayCache.clear();
	m_fileCache.clear();
	// Sprites hold GL names the viewport may still use; they live as long as the box.
}

void FxBox::ClearAll(bool markedOnly)
{
	StopSequences(markedOnly);
	for (std::list<FxSpray*>::iterator i = m_spray.begin(); i != m_spray.end();)
	{
		FxSpray* s = *i;
		if (!markedOnly || s->m_bMark)
		{
			i = m_spray.erase(i);
			delete s;
		}
		else
			++i;
	}
	if (!markedOnly)
	{
		for (std::list<FxAttacher*>::iterator a = m_attachers.begin(); a != m_attachers.end(); ++a)
			delete *a;
		m_attachers.clear();
	}
}

void FxBox::StopSequences(bool markedOnly)
{
	for (std::list<FxSequence*>::iterator i = m_sequence.begin(); i != m_sequence.end();)
	{
		FxSequence* s = *i;
		++i;
		if (!markedOnly || s->m_bMark)
			DeleteSequence(s, false);
	}
}

void FxBox::StopFile(const std::string& file)
{
	for (std::list<FxSequence*>::iterator i = m_sequence.begin(); i != m_sequence.end();)
	{
		FxSequence* s = *i;
		++i;
		if (!_stricmp(s->m_filename.c_str(), file.c_str()))
			DeleteSequence(s, true);
	}
	for (std::list<FxSpray*>::iterator i = m_spray.begin(); i != m_spray.end();)
	{
		FxSpray* s = *i;
		++i;
		if (!_stricmp(s->m_filename.c_str(), file.c_str()))
			DeleteSpray(s);
	}
}

void FxBox::ResetTrails()
{
	for (std::list<FxSpray*>::iterator s = m_spray.begin(); s != m_spray.end(); ++s)
	{
		for (std::list<FxParticle*>::iterator p = (*s)->m_particle.begin(); p != (*s)->m_particle.end(); ++p)
		{
			(*p)->tail.clear();
			(*p)->bandana.clear();
		}
	}
}

void FxBox::DeleteSpray(FxSpray* spray)
{
	m_spray.remove(spray);
	if (spray->m_parent)
	{
		std::vector<FxSpray*>& list = spray->m_parent->m_spray;
		list.erase(std::remove(list.begin(), list.end(), spray), list.end());
		if (spray->m_parent->m_lastSpray == spray)
			spray->m_parent->m_lastSpray = 0;
	}
	for (std::list<FxSpray*>::iterator i = m_spray.begin(); i != m_spray.end(); ++i)
		if ((*i)->m_spawnParent == spray)
			(*i)->m_spawnParent = 0;
	delete spray;
}

// CFxBox::DeleteSequence: orphaned sprays take over the sequence's position, and
// strong-bonded ones its attachment.
void FxBox::DeleteSequence(FxSequence* seq, bool deleteChild)
{
	m_sequence.remove(seq);
	std::vector<FxSpray*> children = seq->m_spray;
	for (size_t i = 0; i < children.size(); ++i)
	{
		FxSpray* spray = children[i];
		if (deleteChild)
		{
			DeleteSpray(spray);
			continue;
		}
		Add(spray->m_pos, seq->m_pos);
		spray->m_parent = 0;
		if (spray->m_param.bStrongBond && seq->m_attacher)
			spray->m_attacher = seq->m_attacher;
	}
	delete seq;
}

bool FxBox::Attach(const FxAttacher& attacher, const std::string& file, bool mark)
{
	if (file.size() < 5)
		return false;
	FxAttacher* a = new FxAttacher(attacher);
	m_attachers.push_back(a);
	if (!_stricmp(file.c_str() + file.size() - 3, "spr"))
	{
		FxSpray* spray = LoadSpray(file);
		if (!spray)
			return false;
		spray->m_filename = file;
		spray->m_bMark = mark;
		spray->m_attacher = a;
		if (g_world)
			a->Get(*g_world, &spray->m_pos);
		AddSpray(spray);
		return true;
	}
	FxSequence* seq = new FxSequence(*this);
	ClearDefines(seq->m_defines);
	if (!seq->LoadFile(file))
	{
		delete seq;
		return false;
	}
	seq->m_filename = file;
	seq->m_bMark = mark;
	seq->m_attacher = a;
	if (g_world)
		a->Get(*g_world, &seq->m_pos);
	m_sequence.push_back(seq);
	return true;
}

void FxBox::Process(float elapsed, const std::vector<WMatrix>& world)
{
	g_world = &world;
	// CPvsFxBox::ProcessAttacher
	WVector pos;
	for (std::list<FxSequence*>::iterator i = m_sequence.begin(); i != m_sequence.end(); ++i)
		if ((*i)->m_attacher && (*i)->m_attacher->Get(world, &pos))
			(*i)->m_pos = pos;
	for (std::list<FxSpray*>::iterator i = m_spray.begin(); i != m_spray.end(); ++i)
		if ((*i)->m_attacher && (*i)->m_attacher->Get(world, &pos))
			(*i)->m_pos = pos;

	if (elapsed > FRAME_TIME * 1.5f)
		elapsed = FRAME_TIME * 1.5f;

	for (std::list<FxSpray*>::iterator i = m_spray.begin(); i != m_spray.end();)
	{
		FxSpray* s = *i;
		++i;
		if (!s->Process(elapsed))
			DeleteSpray(s);
	}
	for (std::list<FxSequence*>::iterator i = m_sequence.begin(); i != m_sequence.end();)
	{
		FxSequence* s = *i;
		++i;
		bool error = false;
		bool alive = s->Process(elapsed, &error);
		if (error)
		{
			Problem(s->m_filename + ": line \"" + s->m_lastError.text + "\"");
			DeleteSequence(s, false);
		}
		else if (!alive)
			DeleteSequence(s, false);
	}
}

int FxBox::ParticleCount() const
{
	int n = 0;
	for (std::list<FxSpray*>::const_iterator i = m_spray.begin(); i != m_spray.end(); ++i)
		n += (int)(*i)->m_particle.size();
	return n;
}

// CFxBox::GetOutput
void FxBox::GetOutput(const WVector& cameraPos, WVector* quake, int* flash) const
{
	WVector q = cameraPos;
	int f = 0;
	for (std::list<FxSpray*>::const_iterator i = m_spray.begin(); i != m_spray.end(); ++i)
	{
		const FxSpray* s = *i;
		float rate = 1.0f / ((q - s->m_quakePos).Length() + 1.0f);
		q = q + s->m_quake * rate;
		f += s->m_flash;
		if (f > 255)
			f = 255;
	}
	*quake = q - cameraPos;
	*flash = f;
}

float FxBox::ParseNumber(const std::string& text) const
{
	if (text.empty())
		return 0.0f;
	char last = text[text.size() - 1];
	if (last == 'd')
		return F(text) * DEGTORAD;
	if (last == 'f')
		return F(text) * FRAME_TIME;
	return F(text);
}

float FxBox::Random(float lo, float hi) const
{
	// RAND_MAX can be as low as 32767, which is enough for the 20001 steps used here.
	return lo + (hi - lo) * (rand() % 20001) * 0.00005f;
}

WVector FxBox::Random(const WVector& lo, const WVector& hi) const
{
	float x = Random(lo.x, hi.x);
	float y = Random(lo.y, hi.y);
	float z = Random(lo.z, hi.z);
	return WVector(x, y, z);
}

WVector FxBox::RandomSphere(const WVector& center, float minRadius, float maxRadius, float angle) const
{
	float c = (float)cos(angle * 0.5f);
	float hx, hz, y, len;
	do
	{
		hx = Random(-1.0f, 1.0f);
		hz = Random(-1.0f, 1.0f);
		y = Random(c, 1.0f);
		len = (float)sqrt(hx * hx + hz * hz);
	} while (fabs(len) < EPS);
	float k = (float)sqrt(1.0f - y * y) / len;
	float r = Random(minRadius, maxRadius);
	return center + WVector(hx * k * r, y * r, hz * k * r);
}

FxSprite* FxBox::LoadSprite(const std::string& name, int w, int h)
{
	char size[32];
	sprintf_s(size, "_%dx%d", w, h);
	std::string key = name + size;
	std::map<std::string, FxSprite*>::iterator i = m_sprite.find(key);
	if (i != m_sprite.end())
		return i->second;
	FxSprite* s = new FxSprite;
	s->path = m_host->ResolveAsset(name);
	s->cellW = w;
	s->cellH = h;
	s->texture = 0;
	s->texW = s->texH = 0;
	s->tried = false;
	if (s->path.empty())
		Problem("texture " + name);
	m_sprite[key] = s;
	return s;
}

void FxBox::ClearDefines(FxDefines& defines) const
{
	defines.name.clear();
	defines.value.clear();
	defines.name.push_back("particle_level");
	defines.value.push_back(1.0f);
}

bool FxBox::PreprocessDefines(FxDefines& defines, const std::string& text) const
{
	if (_strnicmp(text.c_str(), "#define", 7))
		return false;
	std::vector<std::string> tok;
	size_t p = 7;
	while (p < text.size())
	{
		size_t a = text.find_first_not_of(" \t\r\n", p);
		if (a == std::string::npos)
			break;
		size_t b = text.find_first_of(" \t\r\n", a);
		if (b == std::string::npos)
			b = text.size();
		tok.push_back(text.substr(a, b - a));
		p = b;
	}
	if (tok.empty())
		return true;
	for (size_t i = 0; i < defines.name.size(); ++i)
		if (Is(tok[0], defines.name[i].c_str()))
			return true;
	int op = 0;
	float values[2] = { 0.0f, 0.0f };
	int n = 0;
	for (size_t i = 1; i < tok.size(); ++i)
	{
		if (tok[i] == "*")
			op = 3;
		else if (tok[i] == "/")
			op = 4;
		else if (tok[i] == "+")
			op = 1;
		else if (tok[i] == "-")
			op = 2;
		else if (n < 2)
		{
			float v = F(tok[i]);
			for (size_t k = 0; k < defines.name.size(); ++k)
				if (Is(tok[i], defines.name[k].c_str()))
					v = defines.value[k];
			values[n++] = v;
		}
	}
	float r;
	switch (op)
	{
	case 1: r = values[0] + values[1]; break;
	case 2: r = values[0] - values[1]; break;
	case 3: r = values[0] * values[1]; break;
	case 4: r = values[1] != 0.0f ? values[0] / values[1] : 0.0f; break;
	default: r = values[0]; break;
	}
	defines.name.push_back(tok[0]);
	defines.value.push_back(r);
	return true;
}

void FxBox::ProcessDefines(const FxDefines& defines, std::string* text) const
{
	for (size_t i = 0; i < defines.name.size(); ++i)
	{
		if (Is(*text, defines.name[i].c_str()))
		{
			char buf[64];
			sprintf_s(buf, "%g", defines.value[i]);
			*text = buf;
		}
	}
}

// "command = a, (b, c), d": every comma separates, the brackets are decoration.
void FxBox::SplitArgs(const std::string& text, std::string* command, std::string args[16]) const
{
	size_t eq = text.find('=');
	*command = TrimAll(eq == std::string::npos ? text : text.substr(0, eq));
	std::string rest = eq == std::string::npos ? std::string() : text.substr(eq + 1);
	size_t p = 0;
	for (int i = 0; i < 16; ++i)
	{
		args[i].clear();
		if (p > rest.size())
			continue;
		size_t c = rest.find(',', p);
		std::string a = c == std::string::npos ? rest.substr(p) : rest.substr(p, c - p);
		p = c == std::string::npos ? rest.size() + 1 : c + 1;
		args[i] = Trim(a, " \t\r\n()%");
	}
}

const std::vector<std::string>* FxBox::FileLines(const std::string& name)
{
	std::map<std::string, std::vector<std::string> >::iterator i = m_fileCache.find(name);
	if (i != m_fileCache.end())
		return &i->second;
	std::string path = m_host->ResolveAsset(name);
	if (path.empty())
	{
		Problem("file " + name);
		return 0;
	}
	FILE* f = OpenUtf8(path.c_str(), L"rb");
	if (!f)
	{
		Problem("file " + name);
		return 0;
	}
	std::vector<std::string>& lines = m_fileCache[name];
	std::string cur;
	int c;
	while ((c = fgetc(f)) != EOF)
	{
		if (c == '\n')
		{
			lines.push_back(cur);
			cur.clear();
		}
		else if (c != '\r')
			cur.push_back((char)c);
	}
	if (!cur.empty())
		lines.push_back(cur);
	fclose(f);
	return &lines;
}

// -- spray files ---------------------------------------------------------------------

FxSprayParam::FxSprayParam()
{
	bStrongBond = bSpawnAttached = bAttachedToParent = false;
	bBillboard = true;
	attachedAxis = 0;
	sprite = 0;
	petModel = 0;
	bPetLight = bSphere = bRegenAtSamePos = false;
	camOffset = 0.0f;
	lifeTime[0] = lifeTime[1] = 1000;
	volMaxOpacity = 1.0f;
	volCore[0] = volCore[1] = 1.0f;
	friction = groundFriction = 0.0f;
	groundHeight = -1000.0f;
	groundBounce = groundRadius = 0.0f;
	blend[0] = blend[1] = 0x800000;
	lighting = 0;
	initFrame[0] = initFrame[1] = 0;
	initSize[0] = initSize[1] = 1.0f;
	initRatio = 1.0f;
	tailColor[0] = 0;
	tailColor[1] = tailColor[2] = tailColor[3] = 255;
	tailNum = 0;
	tailInterval = 0.0f;
	tailTexture = 0;
	tailTexScale = 50.0f;
	bMonolith = bCameraSide = false;
	addWind[0] = addWind[1] = 0.0f;
	extOrientation = 0;
	distLimit[0] = distLimit[1] = 0.0f;
	bEvenSphere = false;
}

static unsigned int ParseBlend(const std::string& s, bool* ok)
{
	if (Is(s, "normal"))
		return 0;
	if (Is(s, "add"))
		return 0x800000;
	if (Is(s, "multiply"))
		return 0x1000000;
	if (Is(s, "invmultiply"))
		return 0x1800000;
	*ok = false;
	return 0;
}

bool FxBox::ParseSprayLine(const std::string& rawLine, FxSprayParam* param, FxDefines& defines)
{
	std::string line = TrimAll(rawLine);
	if (line.empty() || line[0] == ';' || PreprocessDefines(defines, line))
		return true;
	std::string command, a[16];
	SplitArgs(line, &command, a);
	for (int i = 0; i < 16; ++i)
		ProcessDefines(defines, &a[i]);
	FxSprayParam& p = *param;
	const char* c = command.c_str();

	if (!_stricmp(c, "source"))
	{
		size_t sp = a[0].find_first_of(" \t");
		if (sp != std::string::npos && Is(a[0].substr(0, sp), "*pet"))
		{
			// Only the next word is the name; data has "*pet x.pet off" and "*pet x.pet (petlight".
			p.petName = TrimAll(a[0].substr(sp + 1));
			size_t end = p.petName.find_first_of(" \t");
			if (end != std::string::npos)
				p.petName.erase(end);
			p.bPetLight = Is(a[1], "petlight");
			p.petModel = m_host->LoadPetModel(p.petName);
			if (!p.petModel)
				Problem("puppet " + p.petName);
		}
		else
		{
			p.sprite = LoadSprite(a[0], I(a[1]), I(a[2]));
			p.bBillboard = a[3].empty() || Is(a[3], "billboard");
		}
	}
	else if (!_stricmp(c, "tail"))
	{
		p.tailNum = I(a[0]);
		p.tailInterval = ParseNumber(a[1]);
		for (int i = 0; i < 4; ++i)
			p.tailColor[i] = (unsigned char)I(a[2 + i]);
		if (!a[6].empty())
		{
			p.tailTexture = LoadSprite(a[6], 0, 0);
			p.tailTexScale = F(a[7]);
			p.bMonolith = Is(a[8], "monolith");
			p.bCameraSide = Is(a[9], "cameraside");
		}
	}
	else if (!_stricmp(c, "issphere"))
	{
		int type = I(a[0]);
		p.bSphere = type != 0;
		if (type == 2)
			p.bEvenSphere = true;
	}
	else if (!_stricmp(c, "genpos"))
	{
		p.genMin = WVector(F(a[0]), F(a[1]), F(a[2]));
		p.genMax = WVector(F(a[3]), F(a[4]), p.bSphere ? ParseNumber(a[5]) : F(a[5]));
	}
	else if (!_stricmp(c, "regenatsamepos"))
		p.bRegenAtSamePos = I(a[0]) != 0;
	else if (!_stricmp(c, "camoffset"))
		p.camOffset = F(a[0]);
	else if (!_stricmp(c, "add_generation"))
	{
		FxSprayParam::Gen g;
		g.start = (int)ParseNumber(a[0]);
		g.end = (int)ParseNumber(a[1]);
		g.num = I(a[2]);
		g.spread = ParseNumber(a[3]);
		g.count = 0;
		g.time = 0.0f;
		p.generation.push_back(g);
	}
	else if (!_stricmp(c, "lifetime"))
	{
		p.lifeTime[0] = (int)ParseNumber(a[0]);
		p.lifeTime[1] = (int)ParseNumber(a[1]);
		if (p.lifeTime[0] == 0)
		{
			for (size_t i = 0; i < p.fade.size(); ++i)
			{
				if (p.fade[i].start == 0.0f)
				{
					p.fade[i].startColor[0] = p.fade[i].endColor[0] = 255;
					break;
				}
			}
		}
		else
			p.volMaxOpacity = p.volCore[0] = p.volCore[1] = 1.0f;
	}
	else if (!_stricmp(c, "vol_max_opacity"))
		p.volMaxOpacity = p.lifeTime[0] ? 1.0f : Between(0.0f, F(a[0]), 1.0f);
	else if (!_stricmp(c, "vol_core"))
	{
		p.volCore[0] = p.lifeTime[0] ? 1.0f : Between(0.0f, F(a[0]), 1.0f);
		p.volCore[1] = p.lifeTime[0] ? 1.0f : Between(0.0f, F(a[1]), 1.0f);
	}
	else if (!_stricmp(c, "init_velocity"))
	{
		p.initVelMin = WVector(F(a[0]), F(a[1]), F(a[2]));
		p.initVelMax = WVector(F(a[3]), F(a[4]), F(a[5]));
	}
	else if (!_stricmp(c, "add_velocity"))
	{
		FxSprayParam::Vel v;
		v.start = (int)ParseNumber(a[0]);
		v.startRate = F(a[1]);
		v.end = (int)ParseNumber(a[2]);
		v.endRate = F(a[3]);
		p.velocity.push_back(v);
	}
	else if (!_stricmp(c, "add_wind"))
	{
		p.addWind[0] = F(a[0]);
		p.addWind[1] = F(a[1]);
	}
	else if (!_stricmp(c, "ext_wind"))
		p.extWind = WVector(F(a[0]), F(a[1]), F(a[2]));
	else if (!_stricmp(c, "ext_orientation"))
		p.extOrientation = (I(a[0]) ? 1 : 0) | (I(a[1]) ? 2 : 0) | (I(a[2]) ? 4 : 0);
	else if (!_stricmp(c, "init_angle"))
	{
		p.initAngleMin = WVector(ParseNumber(a[0]), ParseNumber(a[1]), ParseNumber(a[2]));
		p.initAngleMax = WVector(ParseNumber(a[3]), ParseNumber(a[4]), ParseNumber(a[5]));
	}
	else if (!_stricmp(c, "init_rotation"))
	{
		p.initRotMin = WVector(ParseNumber(a[0]), ParseNumber(a[1]), ParseNumber(a[2]));
		p.initRotMax = WVector(ParseNumber(a[3]), ParseNumber(a[4]), ParseNumber(a[5]));
	}
	else if (!_stricmp(c, "add_rotation"))
	{
		FxSprayParam::Rot r;
		r.start = F(a[0]);
		r.startRate = ParseNumber(a[1]);
		r.end = F(a[2]);
		r.endRate = ParseNumber(a[3]);
		p.rotation.push_back(r);
	}
	else if (!_stricmp(c, "friction"))
		p.friction = F(a[0]);
	else if (!_stricmp(c, "groundfriction"))
		p.groundFriction = F(a[0]);
	else if (!_stricmp(c, "gravity"))
		p.gravity = WVector(F(a[0]), F(a[1]), F(a[2]));
	else if (!_stricmp(c, "add_gravitypoint"))
	{
		FxSprayParam::Gravity g;
		g.pos = WVector(F(a[0]), F(a[1]), F(a[2]));
		g.power = F(a[3]);
		g.radius = F(a[4]);
		g.outerPower = F(a[5]);
		p.gravityPoint.push_back(g);
	}
	else if (!_stricmp(c, "ground"))
	{
		p.groundHeight = F(a[0]);
		p.groundBounce = F(a[1]);
		p.groundRadius = F(a[2]);
	}
	else if (!_stricmp(c, "blend"))
	{
		bool ok = true;
		p.blend[0] = ParseBlend(a[0], &ok);
		p.blend[1] = ParseBlend(a[1], &ok);
		if (!ok)
			return false;
	}
	else if (!_stricmp(c, "lighting"))
	{
		if (Is(a[0], "off"))
			p.lighting = 0;
		else if (Is(a[0], "diffuse"))
			p.lighting = 1;
		else if (Is(a[0], "ambient"))
			p.lighting = 2;
		else if (Is(a[0], "ambient2"))
			p.lighting = 3;
		else
			return false;
	}
	else if (!_stricmp(c, "add_fade"))
	{
		FxSprayParam::Fade f;
		f.start = F(a[0]);
		for (int i = 0; i < 4; ++i)
			f.startColor[i] = (unsigned char)I(a[1 + i]);
		f.end = F(a[5]);
		for (int i = 0; i < 4; ++i)
			f.endColor[i] = (unsigned char)I(a[6 + i]);
		if (p.lifeTime[0] == 0 && f.start == 0.0f)
			f.startColor[0] = f.endColor[0] = 255;
		p.fade.push_back(f);
	}
	else if (!_stricmp(c, "init_size"))
	{
		p.initSize[0] = F(a[0]);
		p.initSize[1] = F(a[1]);
		if (!a[2].empty())
			p.initRatio = F(a[2]);
	}
	else if (!_stricmp(c, "add_size"))
	{
		FxSprayParam::Size s;
		s.start = F(a[0]);
		s.startSize = F(a[1]);
		s.end = F(a[2]);
		s.endSize = F(a[3]);
		p.size.push_back(s);
	}
	else if (!_stricmp(c, "init_frame"))
	{
		p.initFrame[0] = I(a[0]);
		p.initFrame[1] = I(a[1]);
	}
	else if (!_stricmp(c, "add_frame"))
	{
		FxSprayParam::Frame f;
		f.start = F(a[0]);
		f.end = F(a[1]);
		f.startFrame = I(a[2]);
		f.endFrame = I(a[3]);
		f.interval = (int)ParseNumber(a[4]);
		p.frame.push_back(f);
	}
	else if (!_stricmp(c, "add_quake"))
	{
		FxSprayParam::Quake q;
		q.pos = WVector(F(a[0]), F(a[1]), F(a[2]));
		q.start = (int)ParseNumber(a[3]);
		q.startPower = WVector(F(a[4]), F(a[5]), F(a[6]));
		q.end = (int)ParseNumber(a[7]);
		q.endPower = WVector(F(a[8]), F(a[9]), F(a[10]));
		q.interval = (int)ParseNumber(a[11]);
		q.time = 0.0f;
		p.quake.push_back(q);
	}
	else if (!_stricmp(c, "add_flash"))
	{
		FxSprayParam::Flash f;
		f.start = (int)ParseNumber(a[0]);
		f.peak = (int)ParseNumber(a[1]);
		f.end = (int)ParseNumber(a[2]);
		p.flash.push_back(f);
	}
	else if (!_stricmp(c, "dist_limit"))
	{
		p.distLimit[0] = F(a[0]) > 0.0f ? F(a[0]) : 0.0f;
		p.distLimit[1] = F(a[1]) > 0.0f ? F(a[1]) : 0.0f;
		if (p.distLimit[1] < p.distLimit[0])
			p.distLimit[1] = p.distLimit[0];
	}
	else if (!_stricmp(c, "add_bubble"))
	{
		FxSprayParam::Bubble b;
		b.amplitude = WVector(F(a[0]), F(a[2]), F(a[4]));
		b.frequency = WVector(F(a[1]), F(a[3]), F(a[5]));
		p.bubble.push_back(b);
	}
	else if (!_stricmp(c, "sound"))
	{
		FxSprayParam::Sound s;
		s.name = a[0];
		size_t ext = s.name.find(".wav");
		if (ext != std::string::npos)
			s.name.erase(ext);
		s.range = a[1].empty() ? -1.0f : F(a[1]);
		p.sound.push_back(s);
	}
	else
		return false;
	return true;
}

bool FxBox::LoadSprayParam(const std::string& name, FxSprayParam* out)
{
	std::map<std::string, FxSprayParam*>::iterator i = m_sprayCache.find(name);
	if (i != m_sprayCache.end())
	{
		if (!i->second)
			return false;
		*out = *i->second;
		return true;
	}
	const std::vector<std::string>* lines = FileLines(name);
	if (!lines)
	{
		m_sprayCache[name] = 0;
		return false;
	}
	FxSprayParam* param = new FxSprayParam;
	FxDefines defines;
	ClearDefines(defines);
	for (size_t k = 0; k < lines->size(); ++k)
	{
		if (!ParseSprayLine((*lines)[k], param, defines))
		{
			// The client abandons the whole spray at the first line it cannot read.
			Problem("spray " + name + ": line \"" + TrimAll((*lines)[k]) + "\"");
			delete param;
			m_sprayCache[name] = 0;
			return false;
		}
	}
	m_sprayCache[name] = param;
	*out = *param;
	return true;
}

FxSpray* FxBox::LoadSpray(const std::string& name)
{
	FxSpray* spray = new FxSpray(*this);
	if (!LoadSprayParam(name, &spray->m_param))
	{
		delete spray;
		return 0;
	}
	return spray;
}

// -- sequence ------------------------------------------------------------------------

FxSequence::FxSequence(FxBox& box)
	: m_box(box)
{
	m_attacher = 0;
	m_bMark = true;
	m_time = 0.0f;
	m_lastSpray = 0;
	m_current = m_lines.end();
}

FxSequence::~FxSequence()
{
}

bool FxSequence::LoadFile(const std::string& name)
{
	const std::vector<std::string>* lines = m_box.FileLines(name);
	if (!lines)
		return false;
	m_lines.clear();
	for (size_t i = 0; i < lines->size(); ++i)
		AddLine((*lines)[i], (int)i + 1);
	m_current = m_lines.begin();
	SetLineTime();
	m_current = m_lines.begin();
	return true;
}

void FxSequence::AddLine(const std::string& text, int number)
{
	std::string t = TrimAll(text);
	if (t.empty() || t[0] == ';' || m_box.PreprocessDefines(m_defines, t))
		return;
	size_t colon = t.find(':');
	std::string head = colon == std::string::npos ? t : t.substr(0, colon);
	std::string command = colon == std::string::npos ? std::string() : TrimAll(t.substr(colon + 1));
	size_t dash = head.find('-');
	std::string startText = TrimAll(dash == std::string::npos ? head : head.substr(0, dash));
	std::string endText = dash == std::string::npos ? std::string() : TrimAll(head.substr(dash + 1));

	Line line;
	line.number = number;
	line.start = line.end = 0;
	if (!startText.empty() && isalpha((unsigned char)startText[0]))
	{
		// Also where a line without a colon lands: it becomes a label and never runs.
		line.time = -1;
		line.text = startText;
	}
	else if (startText.empty())
	{
		line.time = -2;
		line.text = command;
	}
	else
	{
		line.start = atoi(startText.c_str());
		line.end = endText.empty() ? line.start : atoi(endText.c_str());
		line.time = line.start;
		line.text = command;
	}
	m_lines.push_back(line);
}

// Picks a moment inside "start-end" ranges and keeps each block between labels sorted.
void FxSequence::SetLineTime()
{
	for (std::list<Line>::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
	{
		Line& line = *i;
		if (line.time == -1 || line.time == -2)
			continue;
		if (line.end != line.start)
			line.time = line.start + rand() % (line.end - line.start);
		std::list<Line>::iterator first = i, prev = i;
		for (; first != m_lines.begin(); --first)
		{
			--prev;
			if (prev->time == -1 || prev->time == -2)
				break;
		}
		for (; first != i; ++first)
		{
			if (first->time > line.time)
			{
				bool wasCurrent = m_current == i;
				std::list<Line>::iterator moved = m_lines.insert(first, line);
				std::list<Line>::iterator next = m_lines.erase(i);
				if (wasCurrent)
					m_current = moved;
				i = next;
				--i;
				break;
			}
		}
	}
}

bool FxSequence::ParseLine(const std::string& text, bool* error)
{
	*error = false;
	if (text.empty() || text[0] == ';')
		return true;
	std::string command, a[16];
	m_box.SplitArgs(text, &command, a);
	for (int i = 0; i < 16; ++i)
		m_box.ProcessDefines(m_defines, &a[i]);
	if (command.empty())
		return true;
	const char* c = command.c_str();

	if (!_stricmp(c, "spray"))
	{
		FxSpray* spray = m_box.LoadSpray(a[0]);
		if (!spray)
		{
			*error = true;
			return false;
		}
		spray->m_param.bStrongBond = Is(a[7], "strongbond");
		spray->m_parent = this;
		spray->m_bMark = m_bMark;
		spray->m_pos = WVector(F(a[1]), F(a[2]), F(a[3]));
		spray->m_vel = m_rot.Rotate(WVector(F(a[4]), F(a[5]), F(a[6])));
		m_spray.push_back(spray);
		m_box.AddSpray(spray);
		m_lastSpray = spray;
	}
	else if (!_stricmp(c, "add_spawn"))
	{
		if (m_lastSpray)
		{
			FxSprayParam::Spawn s;
			s.name = a[0];
			s.attached = Is(a[1], "attached");
			s.type = -1;
			s.time = 0.0f;
			s.count = 0;
			if (Is(a[2], "interval"))
			{
				s.type = 0;
				s.time = m_box.ParseNumber(a[3]);
				s.count = (int)m_box.ParseNumber(a[4]);
			}
			else if (Is(a[2], "age"))
			{
				s.type = 1;
				s.time = m_box.ParseNumber(a[3]);
			}
			else if (Is(a[2], "ondeath"))
				s.type = 2;
			else if (Is(a[2], "onground"))
				s.type = 3;
			m_lastSpray->m_param.spawn.push_back(s);
		}
	}
	else if (!_stricmp(c, "add_revolution") || !_stricmp(c, "add_prevolution"))
	{
		if (m_lastSpray)
		{
			FxSprayParam::Revol r;
			r.center = WVector(F(a[0]), F(a[1]), F(a[2]));
			r.axis = WVector(F(a[3]), F(a[4]), F(a[5]));
			r.start = (int)m_box.ParseNumber(a[6]);
			r.startSpeed = m_box.ParseNumber(a[7]);
			r.end = (int)m_box.ParseNumber(a[8]);
			r.endSpeed = m_box.ParseNumber(a[9]);
			if (!_stricmp(c, "add_revolution"))
				m_lastSpray->m_param.revolution.push_back(r);
			else
				m_lastSpray->m_param.preRevolution.push_back(r);
		}
	}
	else if (!_stricmp(c, "add_rotation") || !_stricmp(c, "add_nrotation"))
	{
		if (m_lastSpray)
		{
			FxSprayParam::NozzleRot r;
			r.axis = WVector(F(a[0]), F(a[1]), F(a[2]));
			r.start = (int)m_box.ParseNumber(a[3]);
			r.startSpeed = m_box.ParseNumber(a[4]);
			r.end = (int)m_box.ParseNumber(a[5]);
			r.endSpeed = m_box.ParseNumber(a[6]);
			if (!_stricmp(c, "add_rotation"))
				m_lastSpray->m_param.sprayRotation.push_back(r);
			else
				m_lastSpray->m_param.nozzleRotation.push_back(r);
		}
	}
	else if (!_stricmp(c, "attachedtoparent"))
	{
		if (m_lastSpray)
		{
			m_lastSpray->m_param.bAttachedToParent = Is(a[0], "on");
			m_lastSpray->m_param.attachedAxis = (I(a[1]) ? 1 : 0) | (I(a[2]) ? 2 : 0) | (I(a[3]) ? 4 : 0);
		}
	}
	else if (!_stricmp(c, "sound"))
	{
		std::string name = a[0];
		size_t ext = name.find(".wav");
		if (ext != std::string::npos)
			name.erase(ext);
		float distance = a[1].empty() ? -1.0f : F(a[1]);
		float volume = 1.0f;
		if (distance > 0.0f)
		{
			// CSoundManager::LoadSfx, flags 5
			volume = (distance - (m_pos - m_box.m_camPos).Length()) / distance;
			if (volume < 0.0f)
				volume = 0.0f;
		}
		m_box.Host()->PlaySound(name, volume);
	}
	else if (!_stricmp(c, "goto"))
	{
		for (std::list<Line>::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (!_stricmp(i->text.c_str(), a[0].c_str()))
			{
				SetLineTime();
				m_current = ++i;
				if (m_current == m_lines.end())
					return true;
				float time = (float)m_current->start;
				float diff = m_time - time;
				if (diff < FxBox::FRAME_TIME * 2 && diff > FxBox::FRAME_TIME * -2)
					return false;
				m_time = time;
				break;
			}
		}
	}
	else if (!_stricmp(c, "stop"))
	{
		m_current = m_lines.end();
	}
	else
	{
		*error = true;
		return false;
	}
	return true;
}

bool FxSequence::Process(float elapsed, bool* error)
{
	*error = false;
	bool result;
	for (;;)
	{
		if (m_current == m_lines.end())
		{
			result = false;
			break;
		}
		const Line& line = *m_current;
		if (line.time == -1)
		{
			++m_current;
			continue;
		}
		if (line.time != -2 && line.time > (int)(m_time + 0.5f))
		{
			result = true;
			break;
		}
		std::string text = line.text;
		Line copy = line;
		++m_current;
		if (!text.empty())
		{
			bool bad = false;
			if (!ParseLine(text, &bad))
			{
				if (bad)
				{
					m_lastError = copy;
					*error = true;
				}
				// A goto landing right where it already is waits for the next frame.
				if (m_current != m_lines.begin())
					--m_current;
				result = true;
				break;
			}
		}
	}
	m_time += elapsed;
	return result;
}

// -- spray ---------------------------------------------------------------------------

FxSpray::FxSpray(FxBox& box)
	: m_box(box)
{
	m_parent = 0;
	m_spawnParent = 0;
	m_attacher = 0;
	m_bMark = true;
	m_bActive = true;
	m_time = 0.0f;
	m_flash = 0;
	m_sphereAngle = 0.0f;
}

FxSpray::~FxSpray()
{
	for (std::list<FxParticle*>::iterator i = m_particle.begin(); i != m_particle.end(); ++i)
		delete *i;
}

WVector FxSpray::GetBgPos() const
{
	if (m_spawnParent)
		return m_pos;
	if (m_parent)
	{
		if (m_param.bAttachedToParent)
			return m_pos * m_rot + m_parent->m_pos;
		return m_parent->m_pos + m_pos;
	}
	return m_pos;
}

WVector FxSpray::GetBgPos(const FxParticle* p) const
{
	return m_param.bStrongBond ? GetBgPos() : p->bgPos;
}

WMatrix FxSpray::GetBgRot(const FxParticle* p) const
{
	return m_param.bStrongBond ? m_rot : p->bgRot;
}

static void GetCoreBox(WVector& cmin, WVector& cmax, const FxSprayParam& p)
{
	float coreX = p.volCore[0], coreY = p.volCore[1];
	if (!p.bSphere)
	{
		if (coreX < 1.0f || coreY < 1.0f)
		{
			WVector center = (p.genMax + p.genMin) * 0.5f;
			WVector half = (p.genMax - p.genMin) * 0.5f;
			if (coreX < 1.0f)
			{
				cmin.x = (center.x - half.x) * coreX;
				cmin.z = (center.z - half.z) * coreX;
				cmax.x = (center.x + half.x) * coreX;
				cmax.z = (center.z + half.z) * coreX;
			}
			else
			{
				cmin.x = p.genMin.x;
				cmax.x = p.genMax.x;
				cmin.z = p.genMin.z;
				cmax.z = p.genMax.z;
			}
			if (coreY < 1.0f)
			{
				cmin.y = (center.y - half.y) * coreY;
				cmax.y = (center.y + half.y) * coreY;
			}
			else
			{
				cmin.y = p.genMin.y;
				cmax.y = p.genMax.y;
			}
		}
		else
		{
			cmin = p.genMin;
			cmax = p.genMax;
		}
	}
	else
	{
		cmin = p.genMin;
		cmax = WVector(coreX * p.genMax.y, coreY * p.genMax.y, 0.0f);
	}
}

static float VolumeOpacity(const WVector& pos, const WVector& cmin, const WVector& cmax, const FxSprayParam& p)
{
	float opacity = 1.0f;
	float coreX = p.volCore[0], coreY = p.volCore[1];
	if (!p.bSphere)
	{
		if (coreX < 1.0f)
		{
			if (pos.x < cmin.x)
				opacity = 1.0f - (pos.x - cmin.x) / (p.genMin.x - cmin.x);
			else if (cmax.x < pos.x)
				opacity = 1.0f - (pos.x - cmax.x) / (p.genMax.x - cmax.x);
			if (pos.z < cmin.z)
				opacity *= 1.0f - (pos.z - cmin.z) / (p.genMin.z - cmin.z);
			else if (cmax.z < pos.z)
				opacity *= 1.0f - (pos.z - cmax.z) / (p.genMax.z - cmax.z);
		}
		if (coreY < 1.0f)
		{
			if (pos.y < cmin.y)
				opacity *= 1.0f - (pos.y - cmin.y) / (p.genMin.y - cmin.y);
			else if (cmax.y < pos.y)
				opacity *= 1.0f - (pos.y - cmax.y) / (p.genMax.y - cmax.y);
		}
	}
	else
	{
		float dist = (pos - p.genMin).Length();
		if (dist <= cmax.x)
		{
			if (!IsZero(cmax.x))
				opacity = dist / cmax.x;
		}
		else if (cmax.y <= dist)
		{
			if (!IsZero(p.genMax.y - cmax.y))
				opacity = 1.0f - (dist - cmax.y) / (p.genMax.y - cmax.y);
		}
	}
	return opacity;
}

// Ground is the y = 0 plane here; in the game it is the course under the effect.
// CPvsFxBox::CheckGroundCollision, including its range test on the configured height.
static bool GroundCollision(const WVector& from, const WVector& to, float* height, float radius)
{
	if (*height > 99.0f || *height < -99.0f)
		return false;
	const float toHeight = 0.0f, fromHeight = 0.0f;
	*height = toHeight;
	if (fabs(to.y - toHeight) < EPS)
		return true;
	if (to.y < toHeight)
	{
		if (fabs(from.y - fromHeight) < EPS || from.y < fromHeight)
			return fabs(to.y + radius - toHeight) < EPS || to.y + radius > toHeight;
	}
	else
	{
		if (fabs(from.y - fromHeight) < EPS || from.y > fromHeight)
			return fabs(to.y - radius - toHeight) < EPS || to.y - radius < toHeight;
	}
	return true;
}

WMatrix FxSpray::RotateSpray(float time, float dt)
{
	WMatrix mat;
	for (size_t i = 0; i < m_param.sprayRotation.size(); ++i)
	{
		const FxSprayParam::NozzleRot& r = m_param.sprayRotation[i];
		if (time >= r.start && r.end >= time)
		{
			float speed = time - r.start > 0.0f
				? (time - r.start) * (r.endSpeed - r.startSpeed) / (r.end - r.start) + r.startSpeed
				: r.startSpeed;
			mat = RotAxis(r.axis, speed * dt);
		}
	}
	return mat;
}

WMatrix FxSpray::RotateNozzle(float time, float dt)
{
	WMatrix mat;
	for (size_t i = 0; i < m_param.nozzleRotation.size(); ++i)
	{
		const FxSprayParam::NozzleRot& r = m_param.nozzleRotation[i];
		if (time >= r.start && r.end >= time)
		{
			float speed = time - r.start > 0.0f
				? (time - r.start) * (r.endSpeed - r.startSpeed) / (r.end - r.start) + r.startSpeed
				: r.startSpeed;
			mat = RotAxis(r.axis, speed * dt);
		}
	}
	return mat;
}

void FxSpray::Revolute(float time, float dt)
{
	for (size_t i = 0; i < m_param.revolution.size(); ++i)
	{
		const FxSprayParam::Revol& rv = m_param.revolution[i];
		if (rv.start <= time && rv.end >= time)
		{
			float speed = rv.end - rv.start > 0
				? (time - rv.start) * (rv.endSpeed - rv.startSpeed) / (rv.end - rv.start) + rv.startSpeed
				: rv.startSpeed;
			WMatrix mat = RotAxis(rv.axis, speed * dt);
			WVector pos = m_pos;
			m_pos = (m_pos - rv.center) * mat + rv.center;
			Add(m_vel, pos);
			m_vel = (m_vel - rv.center) * mat + rv.center;
			Sub(m_vel, m_pos);
			for (size_t k = 0; k < m_param.sprayRotation.size(); ++k)
			{
				FxSprayParam::NozzleRot& r = m_param.sprayRotation[k];
				Add(r.axis, pos);
				r.axis = (r.axis - rv.center) * mat + rv.center;
				Sub(r.axis, m_pos);
			}
		}
	}
}

void FxSpray::RevoluteGrand(float time, float dt)
{
	for (size_t i = 0; i < m_param.preRevolution.size(); ++i)
	{
		const FxSprayParam::Revol rv = m_param.preRevolution[i];
		if (rv.start <= time && rv.end >= time)
		{
			float speed = rv.end - rv.start > 0
				? (time - rv.start) * (rv.endSpeed - rv.startSpeed) / (rv.end - rv.start) + rv.startSpeed
				: rv.startSpeed;
			WMatrix mat = RotAxis(rv.axis, speed * dt);
			WVector pos = m_pos;
			m_pos = (m_pos - rv.center) * mat + rv.center;
			Add(m_vel, pos);
			m_vel = (m_vel - rv.center) * mat + rv.center;
			Sub(m_vel, m_pos);
			for (size_t k = 0; k < m_param.nozzleRotation.size(); ++k)
			{
				FxSprayParam::NozzleRot& r = m_param.nozzleRotation[k];
				Add(r.axis, pos);
				r.axis = (r.axis - rv.center) * mat + rv.center;
				Sub(r.axis, m_pos);
			}
			for (size_t k = 0; k < m_param.revolution.size(); ++k)
			{
				FxSprayParam::Revol& r = m_param.revolution[k];
				Add(r.axis, r.center);
				r.axis = (r.axis - rv.center) * mat + rv.center;
				r.center = (r.center - rv.center) * mat + rv.center;
				Sub(r.axis, r.center);
			}
		}
	}
}

bool FxSpray::Generate(float time)
{
	float velRate = 100.0f;
	for (size_t i = 0; i < m_param.velocity.size(); ++i)
	{
		const FxSprayParam::Vel& v = m_param.velocity[i];
		if (time >= v.start && time <= v.end)
			velRate = v.end - v.start > 0
				? (v.endRate - v.startRate) * (time - v.start) / (v.end - v.start) + v.startRate
				: v.startRate;
	}
	bool generated = false;
	WVector cmin, cmax;
	if (m_param.lifeTime[0] == 0)
	{
		cmin = m_param.genMin;
		cmax = m_param.genMax;
	}
	else
		GetCoreBox(cmin, cmax, m_param);

	for (size_t i = 0; i < m_param.generation.size(); ++i)
	{
		FxSprayParam::Gen& g = m_param.generation[i];
		if (time <= g.end || g.count < g.num)
		{
			generated = true;
			if (time < g.start)
				g.time = time;
			else
			{
				int num;
				if (time < g.end && g.end - g.start > 0)
					num = (int)((time - g.start) * g.num / (g.end - g.start) + 0.0001f) - g.count + 1;
				else
					num = g.num - g.count;
				if (num > 0)
					g.time = time;
				if (num + g.count > g.num)
					num = g.num - g.count;
				for (int k = 0; k < num; ++k)
				{
					FxParticle* p = new FxParticle;
					InitParticle(false, p, velRate, g.spread, cmin, cmax, false);
					m_particle.push_back(p);
					g.count++;
				}
			}
		}
	}
	return generated;
}

void FxSpray::InitParticle(bool bRegen, FxParticle* p, float velRate, float spread,
	const WVector& cmin, const WVector& cmax, bool bFirst)
{
	p->bStop = false;
	if (!bRegen)
	{
		if (!m_param.bSphere)
			p->pos = m_box.Random(m_param.genMin, m_param.genMax);
		else if (m_param.bEvenSphere)
		{
			float radius = m_param.genMax.x;
			int total = 0;
			for (size_t i = 0; i < m_param.generation.size(); ++i)
				total += m_param.generation[i].num;
			float x = m_box.Random(0.0f, m_param.genMax.z);
			p->pos.x = x - m_param.genMax.z * 0.5f;
			p->pos.y = (float)sin(m_sphereAngle * DEGTORAD) * radius;
			p->pos.z = (float)cos(m_sphereAngle * DEGTORAD) * radius;
			if (total > 0)
				m_sphereAngle += 360.0f / total;
		}
		else
			p->pos = m_box.RandomSphere(m_param.genMin, m_param.genMax.x, m_param.genMax.y, m_param.genMax.z);
	}
	p->bubble = WVector();
	p->bubblePhase.clear();
	p->bgPos = GetBgPos();
	p->bgRot = m_rot;
	p->bGround = false;

	{
		WVector vel = m_box.Random(m_param.initVelMin, m_param.initVelMax) * (velRate * 0.01f);
		p->vel = vel;
		WVector up(0.0f, 1.0f, 0.0f);
		WVector axis = vel.x == 0.0f && vel.z == 0.0f ? WVector(0.0f, 0.0f, 1.0f) : vel.Cross(up);
		float angle = m_box.Random(0.0f, spread);
		p->vel = p->vel * RotAxis(axis - p->pos, angle);
		p->vel = p->vel * RotAxis(vel, m_box.Random(0.0f, PI * 2.0f));
		p->vel = p->vel * m_nozzleRot;
	}

	p->wind = m_box.Random(m_param.addWind[0], m_param.addWind[1]);

	WMatrix rm = AttachRotation(m_attacher);
	WMatrix rmInv = rm.Inverse();
	WVector local;
	if (m_param.extOrientation)
	{
		if (m_param.extOrientation == 7)
		{
			WVector wind = m_param.extWind * p->wind;
			local = p->vel + wind * rmInv;
			p->vel = p->vel * rm;
		}
		else
		{
			WVector up, angle;
			if (m_param.extOrientation & 1)
			{
				up = WVector(0, 1, 0);
				local = up * rm;
				local.x = 0.0f;
				angle.x = GetAngle(up, local);
				if (!IsZero(local.z))
					angle.x *= local.z < 0.0f ? -1.0f : 1.0f;
			}
			if (m_param.extOrientation & 2)
			{
				up = WVector(0, 0, 1);
				local = up * rm;
				local.y = 0.0f;
				angle.y = GetAngle(up, local);
				if (!IsZero(local.x))
					angle.y *= local.x < 0.0f ? -1.0f : 1.0f;
			}
			if (m_param.extOrientation & 4)
			{
				up = WVector(1, 0, 0);
				local = up * rm;
				local.z = 0.0f;
				angle.z = GetAngle(up, local);
				if (!IsZero(local.y))
					angle.z *= local.y > 0.0f ? -1.0f : 1.0f;
			}
			WMatrix orient = RotEuler(angle).Inverse();
			local = (p->vel * orient + m_param.extWind * p->wind) * rmInv;
			p->vel = p->vel * orient;
		}
	}
	else
		local = (m_param.extWind * p->wind + p->vel) * rmInv;

	if (bFirst && (!IsZero(local.x) || !IsZero(local.y) || !IsZero(local.z) ||
		!IsZero(m_param.gravity.x) || !IsZero(m_param.gravity.y) || !IsZero(m_param.gravity.z)))
	{
		if (m_param.bSphere)
		{
			WVector dir = Norm(p->pos - m_param.genMin);
			p->pos = dir * m_param.genMax.y * -0.999f + m_param.genMin;
		}
		else
		{
			WVector dir = !IsZero(local.x) || !IsZero(local.y) || !IsZero(local.z) ? local : m_param.gravity;
			const WVector& lo = m_param.genMin;
			const WVector& hi = m_param.genMax;
			static const WVector normals[6] = {
				WVector(0, 0, 1), WVector(0, 0, -1), WVector(0, 1, 0),
				WVector(0, -1, 0), WVector(1, 0, 0), WVector(-1, 0, 0),
			};
			float d[6] = { -lo.z, hi.z, -lo.y, hi.y, -lo.x, hi.x };
			WVector point[6] = {
				lo, WVector(hi.x, lo.y, hi.z), WVector(hi.x, lo.y, hi.z),
				WVector(lo.x, hi.y, lo.z), WVector(lo.x, lo.y, hi.z), WVector(hi.x, lo.y, lo.z),
			};
			bool firstPlane = true;
			int side = 0;
			if (dir.x > 0.0001f)
				side = 16;
			else if (dir.x < -0.0001f)
				side = 32;
			if (dir.z > 0.0001f)
				side |= 1;
			else if (dir.z < -0.0001f)
				side |= 2;
			if (dir.y > 0.0001f)
				side |= 4;
			else if (dir.y < -0.0001f)
				side |= 8;
			for (int i = 0; i < 6; ++i)
			{
				if (!(side & (1 << i)))
					continue;
				if (!firstPlane && (p->pos - point[i]).Dot(normals[i]) > 0.0f)
					continue;
				float dist = normals[i].Dot(p->pos) + d[i];
				p->pos = p->pos + dir * (dist / normals[i].Dot(dir)) * -1.0f;
				firstPlane = false;
			}
		}
	}

	p->rotVel = m_box.Random(m_param.initRotMin, m_param.initRotMax) * m_nozzleRot;
	p->rot = m_box.Random(m_param.initAngleMin, m_param.initAngleMax) * m_nozzleRot;
	float size = m_box.Random(m_param.initSize[0], m_param.initSize[1]);
	p->initSize = p->size = size;
	p->ratio = m_param.initRatio;

	p->color = 0xffffffff;
	for (size_t i = 0; i < m_param.fade.size(); ++i)
	{
		const FxSprayParam::Fade& f = m_param.fade[i];
		if (f.start == 0.0f)
		{
			p->color = ((unsigned int)f.startColor[0] << 24) | (f.startColor[1] << 16) | (f.startColor[2] << 8) | f.startColor[3];
			break;
		}
	}
	p->volOpacity = VolumeOpacity(p->pos, cmin, cmax, m_param);
	float life = m_box.Random((float)m_param.lifeTime[0], (float)m_param.lifeTime[1]);
	p->lifeTime = p->life = life;
	p->frame = (int)m_box.Random((float)m_param.initFrame[0], (float)m_param.initFrame[1]);
	p->frameTime = 0.0f;
	p->tailTime = 0.0f;
	p->tail.clear();
	p->bandana.clear();
	p->spawnNum = 0;
	p->spawnTime = m_time;
}

void FxSpray::MakeTail(float time, FxParticle* p, const WVector& pos)
{
	if (m_param.tailNum == 0 || time - p->tailTime < m_param.tailInterval)
		return;
	p->tailTime = time;
	if (m_param.tailTexture)
	{
		FxBand band;
		WVector side;
		if (p->vel.Length() < 0.0001f && !p->bandana.empty())
			side = (p->bandana.back().pos[0] - p->bandana.back().pos[1]) * 0.5f;
		else
		{
			if (m_param.bCameraSide)
				side = m_box.m_camForward.Cross(p->vel);
			else
				side = WVector(0, 1, 0).Cross(p->vel);
			side = Norm(side) * (p->size * m_param.tailTexScale * 0.005f);
		}
		band.pos[0] = pos + side;
		band.pos[1] = pos - side;
		if (p->bandana.empty())
			band.length = 0.0f;
		else
		{
			const FxBand& prev = p->bandana.back();
			if ((band.pos[0] - band.pos[1]).Dot(prev.pos[0] - prev.pos[1]) < 0.0f)
				std::swap(band.pos[0], band.pos[1]);
			band.length = (pos - (prev.pos[0] + prev.pos[1]) * 0.5f).Length();
		}
		p->bandana.push_back(band);
		if ((int)p->bandana.size() > m_param.tailNum)
			p->bandana.pop_front();
	}
	else
	{
		FxTail t;
		t.pos = pos;
		t.frame = p->frame;
		p->tail.push_back(t);
		if ((int)p->tail.size() > m_param.tailNum)
			p->tail.pop_front();
	}
}

void FxSpray::SpawnFrom(const FxSprayParam::Spawn& spawn, FxParticle* p)
{
	p->spawnNum++;
	FxSpray* spray = m_box.LoadSpray(spawn.name);
	if (!spray)
		return;
	spray->m_param.bSpawnAttached = spawn.attached;
	spray->m_spawnParent = spawn.attached ? this : 0;
	spray->m_parent = m_parent;
	if (m_parent)
		m_parent->m_spray.push_back(spray);
	spray->m_bMark = m_bMark;
	spray->m_pos = p->pos + p->bgPos;
	spray->m_vel = p->vel;
	spray->m_bActive = m_bActive;
	m_box.AddSpray(spray);
}

unsigned int FxSpray::ParticleColor(const WVector& pos, const FxParticle* p, unsigned int color) const
{
	float rate = 1.0f;
	if (m_param.distLimit[1] > 0.0f)
	{
		float dist = (m_box.m_camPos - pos).Length();
		if (dist > m_param.distLimit[1])
			return 0;
		if (dist >= m_param.distLimit[0])
			rate = 1.0f - Between(0.0f, (dist - m_param.distLimit[0]) / (m_param.distLimit[1] - m_param.distLimit[0]), 1.0f);
	}
	unsigned int a = (unsigned int)((color >> 24) * (m_param.volMaxOpacity * p->volOpacity * rate));
	return (color & 0xffffff) | ((a & 0xff) << 24);
}

bool FxSpray::Process(float elapsed)
{
	float dt = elapsed * 0.06f;
	m_pos = m_pos + m_vel * dt;

	if (m_param.bAttachedToParent && m_parent)
	{
		m_rot = RotOnly(AttachRotation(m_parent->m_attacher));
		if (m_param.attachedAxis && m_param.attachedAxis != 7)
		{
			WVector axis, dir, angle;
			if (m_param.attachedAxis & 1)
			{
				axis = WVector(0, 1, 0);
				dir = axis * m_rot;
				dir.x = 0.0f;
				angle.x = GetAngle(axis, dir);
				if (!IsZero(dir.z))
					angle.x *= dir.z < 0.0f ? -1.0f : 1.0f;
			}
			if (m_param.attachedAxis & 2)
			{
				axis = WVector(0, 0, 1);
				dir = axis * m_rot;
				dir.y = 0.0f;
				angle.y = GetAngle(axis, dir);
				if (!IsZero(dir.x))
					angle.y *= dir.x < 0.0f ? -1.0f : 1.0f;
			}
			if (m_param.attachedAxis & 4)
			{
				axis = WVector(1, 0, 0);
				dir = axis * m_rot;
				dir.z = 0.0f;
				angle.z = GetAngle(axis, dir);
				if (!IsZero(dir.y))
					angle.z *= dir.y > 0.0f ? -1.0f : 1.0f;
			}
			m_rot = RotEuler(angle).Inverse();
		}
	}

	m_nozzleRot = m_nozzleRot * RotateNozzle(m_time, dt);
	WMatrix rot = RotateSpray(m_time, dt);
	Revolute(m_time, dt);
	RevoluteGrand(m_time, dt);

	m_quakePos = WVector();
	bool quaking = false;
	WVector powerSum;
	for (size_t i = 0; i < m_param.quake.size(); ++i)
	{
		FxSprayParam::Quake& q = m_param.quake[i];
		if (m_time < q.start)
			q.time = m_time;
		else if (m_time <= q.end)
		{
			quaking = true;
			if (m_time - q.time >= q.interval)
			{
				q.time = m_time;
				WVector power = q.end - q.start > 0
					? q.startPower + (q.endPower - q.startPower) * ((m_time - q.start) / (q.end - q.start))
					: q.startPower;
				m_quake = WVector();
				m_quake.x += (rand() % 20001 - 10000) * power.x * 0.00005f;
				m_quake.y += (rand() % 20001 - 10000) * power.y * 0.00005f;
				m_quake.z += (rand() % 20001 - 10000) * power.z * 0.00005f;
				m_quakePos.x += power.x * q.pos.x;
				m_quakePos.y += power.y * q.pos.y;
				m_quakePos.z += power.z * q.pos.z;
				Add(powerSum, power);
			}
		}
	}
	if (!quaking)
		m_quake = WVector();
	else
	{
		if (!IsZero(powerSum.x)) m_quakePos.x /= powerSum.x;
		if (!IsZero(powerSum.y)) m_quakePos.y /= powerSum.y;
		if (!IsZero(powerSum.z)) m_quakePos.z /= powerSum.z;
	}

	m_flash = 0;
	for (size_t i = 0; i < m_param.flash.size(); ++i)
	{
		const FxSprayParam::Flash& f = m_param.flash[i];
		if (m_time >= f.start)
		{
			int value = -1;
			if (m_time <= f.peak)
				value = f.peak - f.start > 0 ? 255 * (int)(m_time - f.start) / (f.peak - f.start) : 0;
			else if (m_time <= f.end)
				value = f.end - f.peak > 0 ? 255 - 255 * (int)(m_time - f.peak) / (f.end - f.peak) : 0;
			if (value >= 0)
			{
				value += m_flash;
				m_flash = (unsigned char)(value > 255 ? 255 : value);
			}
		}
	}

	bool generated = Generate(m_time);

	WVector cmin, cmax;
	bool bRegen = false;
	GetCoreBox(cmin, cmax, m_param);

	for (std::list<FxParticle*>::iterator it = m_particle.begin(); it != m_particle.end();)
	{
		FxParticle* p = *it;
		WMatrix bgRot = GetBgRot(p);
		WVector bgPos = GetBgPos(p);
		WVector pos = p->pos * bgRot + bgPos;
		float age = p->lifeTime > 0.0f ? (p->lifeTime - p->life) / p->lifeTime * 100.0f : 100.0f;

		MakeTail(m_time, p, pos + p->bubble);

		if (!p->bStop)
		{
			WVector prev = pos;
			WVector wind = m_param.extWind * p->wind;
			pos = pos + (p->vel * bgRot + wind) * dt;

			WVector friction = Norm(p->vel) * (dt * (p->bGround ? m_param.groundFriction : m_param.friction));
			if ((p->vel.x > 0.0f && p->vel.x < friction.x) || (p->vel.x < 0.0f && p->vel.x > friction.x))
				p->vel.x = 0.0f;
			else
				p->vel.x -= friction.x;
			if ((p->vel.y > 0.0f && p->vel.y < friction.y) || (p->vel.y < 0.0f && p->vel.y > friction.y))
				p->vel.y = 0.0f;
			else
				p->vel.y -= friction.y;
			if ((p->vel.z > 0.0f && p->vel.z < friction.z) || (p->vel.z < 0.0f && p->vel.z > friction.z))
				p->vel.z = 0.0f;
			else
				p->vel.z -= friction.z;

			if (!p->bGround)
			{
				for (size_t g = 0; g < m_param.gravityPoint.size(); ++g)
				{
					const FxSprayParam::Gravity& gp = m_param.gravityPoint[g];
					WVector dir = GetBgPos() + gp.pos - pos;
					float dist = dir.Length();
					if (dist < gp.radius)
					{
						float sq = dist * dist;
						if (sq < 0.5f)
							sq = 0.5f;
						p->vel = p->vel + Norm(dir) * (dt * gp.power / sq);
					}
					else
						p->vel = p->vel + Norm(dir) * (dt * gp.outerPower);
				}

				float height = bgPos.y + m_param.groundHeight;
				if (GroundCollision(prev, pos, &height, m_param.groundRadius))
				{
					float y = pos.y - m_param.groundRadius;
					float gap = height - y > 0.0f ? height - y : y - height;
					if (prev.y < pos.y)
						pos.y = height - m_param.groundRadius;
					else
						pos.y = height + m_param.groundRadius;
					p->vel.y = m_param.groundBounce * p->vel.y * -0.01f;
					float speed = p->vel.y > 0.0f ? p->vel.y : -p->vel.y;
					if ((gap < 0.03f && speed < gap) || speed < 0.01f || (p->vel + m_param.gravity).Dot(p->vel) < 0.0f)
					{
						p->vel.y = 0.0f;
						p->bGround = true;
					}
					for (size_t s = 0; s < m_param.spawn.size(); ++s)
						if (m_param.spawn[s].type == 3)
							SpawnFrom(m_param.spawn[s], p);
				}
				else
					p->vel = p->vel + m_param.gravity * dt;

				if (!m_param.sprayRotation.empty())
					pos = GetBgPos() + (pos - GetBgPos()) * rot;
			}
		}

		for (size_t f = 0; f < m_param.fade.size(); ++f)
		{
			const FxSprayParam::Fade& fade = m_param.fade[f];
			if (age >= fade.start && age <= fade.end)
			{
				unsigned int color = 0;
				for (int i = 0; i < 4; ++i)
				{
					unsigned int v = fade.end - fade.start > 0.0f
						? (unsigned char)((fade.endColor[i] - fade.startColor[i]) * (age - fade.start) / (fade.end - fade.start) + fade.startColor[i])
						: fade.startColor[i];
					color |= v << (24 - i * 8);
				}
				p->color = color;
			}
		}

		if (m_param.sprite || m_param.petModel)
		{
			for (size_t r = 0; r < m_param.rotation.size(); ++r)
			{
				const FxSprayParam::Rot& ro = m_param.rotation[r];
				if (age >= ro.start && age <= ro.end)
				{
					float rate = ro.end - ro.start > 0.0f
						? (ro.endRate - ro.startRate) * (age - ro.start) / (ro.end - ro.start) + ro.startRate
						: ro.startRate;
					p->rot = p->rot + p->rotVel * (dt * rate * 0.01f);
				}
			}
			for (size_t s = 0; s < m_param.size.size(); ++s)
			{
				const FxSprayParam::Size& sz = m_param.size[s];
				if (age >= sz.start && age <= sz.end)
				{
					float rate = sz.end - sz.start > 0.0f
						? (sz.endSize - sz.startSize) * (age - sz.start) / (sz.end - sz.start) + sz.startSize
						: sz.startSize;
					p->size = rate * p->initSize * 0.01f;
				}
			}
			if (!p->bGround)
			{
				for (size_t f = 0; f < m_param.frame.size(); ++f)
				{
					const FxSprayParam::Frame& fr = m_param.frame[f];
					if (age >= fr.start && age <= fr.end && m_time - p->frameTime >= fr.interval)
					{
						p->frameTime = m_time;
						if (fr.startFrame == -1)
							p->frame = fr.endFrame > 0 ? rand() % fr.endFrame : 0;
						else if (++p->frame > fr.endFrame)
							p->frame = fr.startFrame;
					}
				}
			}
			// LifeTime 0: a volume whose particles live until they leave it.
			if (!m_param.lifeTime[0] && elapsed != 0.0f)
			{
				WMatrix rm = AttachRotation(m_attacher);
				WVector local = rm.Inverse().Rotate(pos - bgPos);
				bool inside;
				if (!m_param.bSphere)
				{
					inside = m_param.genMin.x - EPS <= local.x && local.x <= m_param.genMax.x + EPS &&
						m_param.genMin.y - EPS <= local.y && local.y <= m_param.genMax.y + EPS &&
						m_param.genMin.z - EPS <= local.z && local.z <= m_param.genMax.z + EPS;
				}
				else
					inside = (local - m_param.genMin).Length() < m_param.genMax.y + 0.0001f;
				if (inside)
					p->volOpacity = VolumeOpacity(local, cmin, cmax, m_param);
				else
				{
					InitParticle(m_param.bRegenAtSamePos, p, 100.0f, 0.0f, cmin, cmax, true);
					bRegen = false;
					++it;
					continue;
				}
			}
		}

		for (size_t s = 0; s < m_param.spawn.size(); ++s)
		{
			const FxSprayParam::Spawn& sp = m_param.spawn[s];
			if (sp.type == 1 && !p->spawnNum && age + 1.0f > sp.time)
				SpawnFrom(sp, p);
			if (sp.type == 0 && m_time - p->spawnTime > sp.time && sp.count > p->spawnNum)
			{
				p->spawnTime = m_time;
				SpawnFrom(sp, p);
			}
		}

		if (p->life > 0.0f && !bRegen)
		{
			p->life -= elapsed;
			if (p->life <= 0.0f)
			{
				for (size_t s = 0; s < m_param.spawn.size(); ++s)
					if (m_param.spawn[s].type == 2)
						SpawnFrom(m_param.spawn[s], p);
				if (m_param.bRegenAtSamePos)
				{
					// The original re-examines this particle on the next pass of its loop.
					bRegen = true;
					InitParticle(true, p, 100.0f, 0.0f, cmin, cmax, true);
				}
				else
				{
					it = m_particle.erase(it);
					delete p;
				}
				continue;
			}
		}

		if (!m_param.bubble.empty())
		{
			WVector amp;
			while (p->bubblePhase.size() < m_param.bubble.size())
			{
				float z = (rand() % 1001) * (PI * 2.0f * 0.001f);
				float y = (rand() % 1001) * (PI * 2.0f * 0.001f);
				float x = (rand() % 1001) * (PI * 2.0f * 0.001f);
				p->bubblePhase.push_back(WVector(x, y, z));
			}
			for (size_t b = 0; b < m_param.bubble.size(); ++b)
			{
				const FxSprayParam::Bubble& bu = m_param.bubble[b];
				const WVector& ph = p->bubblePhase[b];
				amp.x += (float)cos(bu.frequency.x * m_time * (PI * 2.0f * 0.001f) + ph.x) * bu.amplitude.x;
				amp.y += (float)cos(bu.frequency.y * m_time * (PI * 2.0f * 0.001f) + ph.y) * bu.amplitude.y;
				amp.z += (float)cos(bu.frequency.z * m_time * (PI * 2.0f * 0.001f) + ph.z) * bu.amplitude.z;
			}
			WVector forward = p->vel;
			WVector side = fabs(p->vel.x) > EPS || fabs(p->vel.z) > EPS ? WVector(0, 1, 0).Cross(p->vel) : WVector(0, 0, 1);
			WVector up = forward.Cross(side);
			p->bubble = p->bubble + (Norm(forward) * amp.x + Norm(up) * amp.y + Norm(side) * amp.z) * dt;
		}

		if (!m_param.sound.empty())
		{
			for (size_t s = 0; s < m_param.sound.size(); ++s)
			{
				const FxSprayParam::Sound& snd = m_param.sound[s];
				float volume = 1.0f;
				if (snd.range > 0.0f)
				{
					volume = (snd.range - (p->pos - m_box.m_camPos).Length()) / snd.range;
					if (volume < 0.0f)
						volume = 0.0f;
				}
				m_box.Host()->PlaySound(snd.name, volume);
			}
			m_param.sound.clear();
		}

		p->pos = (pos - GetBgPos(p)) * GetBgRot(p).Inverse();
		bRegen = false;
		++it;
	}

	m_time += elapsed;
	return !m_particle.empty() || generated;
}
