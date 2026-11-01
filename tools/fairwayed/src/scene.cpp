// SPDX-License-Identifier: MIT
// scene.cpp - draws what course.h holds.
//
// Blending follows CaddieRig's viewport: the client depth-sorts, here solid texels write
// depth and soft texels blend over them. Base models draw their baked colours unlit
// (WPolySoup::Load sets m_lightmode 0x1000000); objects get the GAME light on GL_LIGHT0.
#include "scene.h"
#include "areascript.h"
#include "fxsim.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <map>
#include "texture.h"

// What the filename prefix asks for, from CaddieRig's view.cpp.
enum BlendMode { BLEND_NONE, BLEND_ALPHA, BLEND_TEST };

static BlendMode BlendOfTexture(const std::string& name)
{
	if (NameHasFlag(name, "]{"))
		return BLEND_TEST;
	return NameHasFlag(name, "+[~}") ? BLEND_ALPHA : BLEND_NONE;
}

enum DrawPass { PASS_OPAQUE, PASS_BLEND_SOLID, PASS_BLEND_SOFT, PASS_COUNT };

struct PuppetRender
{
	GLuint lists[PASS_COUNT];
	GLuint wire;
	GLuint surface;
	GLuint rim;			// base model triangles with a see-through corner
	bool hasRim;
	int surfaceHighlight;
	std::vector<GLuint> textures;
	bool hasBlend;
	int builtFor;		// base colour generation
};

static std::map<std::string, GLuint> g_textures;	// by resolved path
static int g_baseGeneration = 1;
static std::vector<GLuint> g_shadowTextures;
static GLuint g_shadowList;
static int g_shadowFor;
static std::map<wchar_t, GLuint> g_glyphs;
static GLuint g_skyFar, g_skyNear, g_skyUp, g_skyDn;
static int g_skyFor;

void SceneInvalidateBase()
{
	++g_baseGeneration;
}

static GLuint TextureFor(const std::string& cp949Name)
{
	std::string path = g_ed.course.Resolve(cp949Name);
	if (path.empty())
		return 0;
	std::map<std::string, GLuint>::iterator it = g_textures.find(path);
	if (it != g_textures.end())
		return it->second;
	GLuint t = LoadGLTexture(path);
	g_textures[path] = t;
	return t;
}

static void HashColor(int index, float* r, float* g, float* b)
{
	static const unsigned char PALETTE[][3] = {
		{ 96, 200, 96 }, { 240, 220, 100 }, { 60, 140, 60 }, { 230, 200, 140 }, { 90, 170, 240 }, { 200, 90, 80 },
		{ 170, 110, 220 }, { 110, 220, 210 }, { 240, 150, 60 }, { 150, 150, 150 }, { 240, 120, 180 }, { 130, 120, 70 },
		{ 70, 90, 180 }, { 200, 240, 120 }, { 120, 60, 60 }, { 250, 250, 210 }
	};
	const unsigned char* c = PALETTE[(index < 0 ? 0 : index) % 16];
	*r = c[0] / 255.0f;
	*g = c[1] / 255.0f;
	*b = c[2] / 255.0f;
}

// Shipped holes give the corners around their outer edge alpha 0, which fades the course
// into the sky's bottom plane behind it.
static bool IsRimTriangle(const std::vector<u32>& colors, size_t t)
{
	return (colors[t * 3] >> 24) != 255 || (colors[t * 3 + 1] >> 24) != 255 || (colors[t * 3 + 2] >> 24) != 255;
}

static void BuildRim(PuppetAsset* a, PuppetRender* r)
{
	const PetFile& p = a->pet;
	const std::vector<u32>& colors = g_ed.course.cornerColor;
	r->hasRim = false;
	glNewList(r->rim, GL_COMPILE);
	if (colors.size() == a->pos.size())
	{
		int current = -1;
		bool open = false;
		for (size_t t = 0; t < p.points.size() / 3; ++t)
		{
			if (!IsRimTriangle(colors, t))
				continue;
			int slot = t < p.triTextures.size() ? p.triTextures[t] : 0;
			if (slot != current)
			{
				if (open)
					glEnd();
				current = slot;
				GLuint tex = slot < (int)r->textures.size() ? r->textures[slot] : 0;
				if (tex)
				{
					glBindTexture(GL_TEXTURE_2D, tex);
					glEnable(GL_TEXTURE_2D);
				}
				else
				{
					glDisable(GL_TEXTURE_2D);
				}
				glBegin(GL_TRIANGLES);
				open = true;
			}
			r->hasRim = true;
			for (int c = 0; c < 3; ++c)
			{
				size_t i = t * 3 + c;
				u32 col = colors[i];
				glColor4ub((GLubyte)(col >> 16), (GLubyte)(col >> 8), (GLubyte)col, (GLubyte)(col >> 24));
				glTexCoord2f(p.points[i].u, p.points[i].v);
				glVertex3f(a->pos[i].x, a->pos[i].y, a->pos[i].z);
			}
		}
		if (open)
			glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	glEndList();
}

static void BuildPass(PuppetAsset* a, PuppetRender* r, DrawPass pass, bool baked)
{
	const PetFile& p = a->pet;
	size_t tris = p.points.size() / 3;
	glNewList(r->lists[pass], GL_COMPILE);
	int current = -2;
	bool open = false;
	bool wanted = false;
	const std::vector<u32>& colors = g_ed.course.cornerColor;
	bool useColors = baked && colors.size() == a->pos.size();
	for (size_t t = 0; t < tris; ++t)
	{
		int slot = t < p.triTextures.size() ? p.triTextures[t] : 0;
		if (slot != current)
		{
			if (open)
			{
				glEnd();
				open = false;
			}
			current = slot;
			bool valid = slot < (int)p.textures.size();
			BlendMode mode = valid ? BlendOfTexture(p.textures[slot].Name()) : BLEND_NONE;
			wanted = pass == PASS_OPAQUE ? mode != BLEND_ALPHA : mode == BLEND_ALPHA;
			if (!wanted)
				continue;
			GLuint tex = valid && slot < (int)r->textures.size() ? r->textures[slot] : 0;
			if (tex)
			{
				glBindTexture(GL_TEXTURE_2D, tex);
				glEnable(GL_TEXTURE_2D);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
			}
			if (pass == PASS_BLEND_SOFT)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDepthMask(GL_FALSE);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_LESS, 0.95f);
			}
			else if (pass == PASS_BLEND_SOLID)
			{
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GEQUAL, 0.95f);
			}
			else if (mode == BLEND_TEST)
			{
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);
			}
			else
			{
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
				glDisable(GL_ALPHA_TEST);
			}
			// WBone::CalcLight: flag2 0x10 ('+') is full bright, flag2 4 (']') ambient only.
			if (baked || (valid && NameHasFlag(p.textures[slot].Name(), "+]")))
				glDisable(GL_LIGHTING);
			else
				glEnable(GL_LIGHTING);
			u32 d = valid ? p.textures[slot].diffuse : 0xffffffff;
			if (useColors)
				glColor4f(1, 1, 1, 1);
			else if (tex)
				glColor4f(((d >> 16) & 255) / 255.0f, ((d >> 8) & 255) / 255.0f, (d & 255) / 255.0f, 1.0f);
			else
				glColor4f(0.72f, 0.72f, 0.74f, 1.0f);
			glBegin(GL_TRIANGLES);
			open = true;
		}
		if (!open || (useColors && IsRimTriangle(colors, t)))
			continue;
		for (int c = 0; c < 3; ++c)
		{
			size_t i = t * 3 + c;
			if (useColors)
			{
				u32 col = colors[i];
				glColor4ub((GLubyte)(col >> 16), (GLubyte)(col >> 8), (GLubyte)col, 255);
			}
			glTexCoord2f(p.points[i].u, p.points[i].v);
			glNormal3f(a->normal[i].x, a->normal[i].y, a->normal[i].z);
			glVertex3f(a->pos[i].x, a->pos[i].y, a->pos[i].z);
		}
	}
	if (open)
		glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
	glEndList();
}

static PuppetRender* Render(PuppetAsset* a, bool baked)
{
	PuppetRender* r = (PuppetRender*)a->render;
	if (r && (!baked || r->builtFor == g_baseGeneration))
		return r;
	if (!r)
	{
		r = new PuppetRender;
		a->render = r;
		GLuint base = glGenLists(PASS_COUNT + 3);
		for (int i = 0; i < PASS_COUNT; ++i)
			r->lists[i] = base + i;
		r->wire = base + PASS_COUNT;
		r->surface = base + PASS_COUNT + 1;
		r->rim = base + PASS_COUNT + 2;
		r->hasRim = false;
		r->surfaceHighlight = -100000;
		r->textures.resize(a->pet.textures.size(), 0);
		for (size_t t = 0; t < a->pet.textures.size(); ++t)
		{
			std::string name = a->pet.textures[t].Name();
			if (!name.empty() && name != "null")
				r->textures[t] = TextureFor(name);
		}
		glNewList(r->wire, GL_COMPILE);
		glBegin(GL_TRIANGLES);
		for (size_t i = 0; i < a->pos.size(); ++i)
		{
			glNormal3f(a->normal[i].x, a->normal[i].y, a->normal[i].z);
			glVertex3f(a->pos[i].x, a->pos[i].y, a->pos[i].z);
		}
		glEnd();
		glEndList();
	}
	r->hasBlend = false;
	for (size_t t = 0; t < a->pet.textures.size(); ++t)
		if (BlendOfTexture(a->pet.textures[t].Name()) == BLEND_ALPHA)
			r->hasBlend = true;
	for (int i = 0; i < PASS_COUNT; ++i)
		BuildPass(a, r, (DrawPass)i, baked);
	if (baked)
		BuildRim(a, r);
	r->builtFor = g_baseGeneration;
	return r;
}

static void BuildSurface(PuppetAsset* a, PuppetRender* r)
{
	if (r->surfaceHighlight == g_baseGeneration * 1000 + g_ed.surfaceHighlight)
		return;
	r->surfaceHighlight = g_baseGeneration * 1000 + g_ed.surfaceHighlight;
	const PetFile& p = a->pet;
	glNewList(r->surface, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	std::map<int, int> classOfSlot;
	for (size_t t = 0; t < p.points.size() / 3; ++t)
	{
		int slot = t < p.triTextures.size() ? p.triTextures[t] : 0;
		std::map<int, int>::iterator it = classOfSlot.find(slot);
		int cls;
		if (it != classOfSlot.end())
			cls = it->second;
		else
		{
			cls = slot < (int)p.textures.size() ? g_ed.course.prop.ClassOfTexture(p.textures[slot].Name()) : -1;
			classOfSlot[slot] = cls;
		}
		float cr, cg, cb;
		HashColor(cls, &cr, &cg, &cb);
		if (g_ed.surfaceHighlight >= 0 && cls != g_ed.surfaceHighlight)
		{
			cr = cr * 0.25f + 0.1f;
			cg = cg * 0.25f + 0.1f;
			cb = cb * 0.25f + 0.1f;
		}
		glColor3f(cr, cg, cb);
		for (int c = 0; c < 3; ++c)
		{
			const WVector& n = a->normal[t * 3 + c];
			const WVector& v = a->pos[t * 3 + c];
			glNormal3f(n.x, n.y, n.z);
			glVertex3f(v.x, v.y, v.z);
		}
	}
	glEnd();
	glEndList();
}

// -- shadow maps --------------------------------------------------------------

static GLuint UploadDxt1(const SbinPack& pack)
{
	std::vector<unsigned char> rgba(pack.width * pack.height * 4);
	const unsigned char* src = pack.dxt1.empty() ? 0 : &pack.dxt1[0];
	for (int by = 0; by < pack.height / 4; ++by)
	{
		for (int bx = 0; bx < pack.width / 4; ++bx, src += 8)
		{
			unsigned char pal[4][4];
			DecodeDxtColorBlock(src, pal, true);
			for (int y = 0; y < 4; ++y)
			{
				for (int x = 0; x < 4; ++x)
				{
					int idx = (src[4 + y] >> (x * 2)) & 3;
					unsigned char* d = &rgba[((by * 4 + y) * pack.width + bx * 4 + x) * 4];
					d[0] = pal[idx][0];
					d[1] = pal[idx][1];
					d[2] = pal[idx][2];
					d[3] = 255;	// the pack multiplies over the course, so alpha has no say
				}
			}
		}
	}
	GLuint t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pack.width, pack.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgba[0]);
	return t;
}

// smAddSmapPack: below 0x8000 a code indexes the .sbin's own vertices, above it the mesh's.
// Each pack multiplies over the base model (draw flag 0x21100000).
static void BuildShadowList()
{
	Course& c = g_ed.course;
	if (!g_shadowList)
		g_shadowList = glGenLists(1);
	glNewList(g_shadowList, GL_COMPILE);
	PuppetAsset* base = c.BasePuppet();
	if (base && c.hasSbin && !c.sbin.bodies.empty() && !c.sbin.isNew)
	{
		const SbinBody& b = c.sbin.bodies[0];
		const SbinSection& sec = b.sections[0];
		for (size_t p = 0; p < sec.packs.size(); ++p)
		{
			const SbinPack& pack = sec.packs[p];
			if (pack.width <= 0 || pack.height <= 0)
				continue;
			GLuint t = UploadDxt1(pack);
			g_shadowTextures.push_back(t);
			glBindTexture(GL_TEXTURE_2D, t);
			glBegin(GL_TRIANGLES);
			for (size_t f = 0; f < pack.frags.size(); ++f)
			{
				const SbinInFrag& fr = pack.frags[f];
				if (fr.mesh >= c.baseMeshes.size())
					continue;
				const BaseMesh& mesh = c.baseMeshes[fr.mesh];
				for (int k = 0; k < 3; ++k)
				{
					int code = fr.vtx[k];
					WVector v;
					if (code < 0x8000)
					{
						if (code * 3 + 2 >= (int)b.vtx.size())
							continue;
						v = WVector(b.vtx[code * 3], b.vtx[code * 3 + 1], b.vtx[code * 3 + 2]);
					}
					else
					{
						code -= 0x8000;
						if (code >= (int)mesh.vertexCorner.size())
							continue;
						v = base->pos[mesh.vertexCorner[code]];
					}
					if (fr.uv[k] * 2 + 1 < (int)b.uvs.size())
						glTexCoord2f(b.uvs[fr.uv[k] * 2], b.uvs[fr.uv[k] * 2 + 1]);
					glVertex3f(v.x, v.y, v.z);
				}
			}
			glEnd();
		}
	}
	glEndList();
	g_shadowFor = g_ed.course.revision;
}

// -- sky ----------------------------------------------------------------------

// Geometry from CSky::LoadSky, drawn in CSky::Render's order around the camera: the up and
// bottom planes, then the far and near cylinders. Clouds, stars and the moon aren't drawn.
static void EnsureSky()
{
	Course& c = g_ed.course;
	if (g_skyFor == c.revision)
		return;
	g_skyFor = c.revision;
	std::string stem = c.baseName;
	size_t us = stem.rfind('_');
	if (us != std::string::npos)
		stem.erase(us);
	stem = Utf8ToCp949(stem);
	g_skyFar = TextureFor(stem + "_far.jpg");
	if (!g_skyFar)
		g_skyFar = TextureFor("[" + stem + "_far.jpg");
	g_skyNear = TextureFor(stem + "_near.jpg");
	if (!g_skyNear)
		g_skyNear = TextureFor("[" + stem + "_near.jpg");
	g_skyUp = TextureFor(stem + "_up.jpg");
	g_skyDn = TextureFor(stem + "_dn.jpg");
}

static void SkyPlane(GLuint tex, float height)
{
	if (!tex)
		return;
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(1, 0); glVertex3f(-500, height, 500);
	glTexCoord2f(0, 0); glVertex3f(500, height, 500);
	glTexCoord2f(0, 1); glVertex3f(500, height, -500);
	glTexCoord2f(1, 1); glVertex3f(-500, height, -500);
	glEnd();
}

static void SkyCylinder(GLuint tex, float radius)
{
	if (!tex)
		return;
	const float PI = 3.14159265f;
	const float heights[3] = { radius * PI / 4.0f, radius * PI / 5.0f, radius * -PI / 5.0f };
	const float vs[3] = { 0.0f, 0.1f, 1.0f };
	glBindTexture(GL_TEXTURE_2D, tex);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int band = 0; band < 2; ++band)
	{
		glBegin(GL_QUAD_STRIP);
		for (int i = 0; i <= 32; ++i)
		{
			float a = i / 32.0f * 2.0f * PI, x = (float)cos(a) * radius, z = (float)sin(a) * radius;
			float u = 1.0f - i / 32.0f;
			// LoadSky's top row keeps the sky colour's own alpha, which fades into the up plane.
			glColor4f(1, 1, 1, band == 0 ? 0.0f : 1.0f);
			glTexCoord2f(u, vs[band]);
			glVertex3f(x, heights[band], z);
			glColor4f(1, 1, 1, 1);
			glTexCoord2f(u, vs[band + 1]);
			glVertex3f(x, heights[band + 1], z);
		}
		glEnd();
	}
	glDisable(GL_BLEND);
}

static void DrawSky(const SceneView& v)
{
	EnsureSky();
	glPushMatrix();
	glTranslatef(v.eye.x, v.eye.y, v.eye.z);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDepthMask(GL_FALSE);
	glColor3f(1, 1, 1);
	const float PLANE = 250.0f * 3.14159265f / 4.0f;
	SkyPlane(g_skyUp, PLANE);
	SkyPlane(g_skyDn, -PLANE);
	SkyCylinder(g_skyFar, 500.0f);
	SkyCylinder(g_skyNear, 400.0f);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

// -- helpers ------------------------------------------------------------------

bool SceneProject(const SceneView& v, const WVector& p, float* x, float* y)
{
	WVector d = p - v.eye;
	float cx = d.Dot(v.right), cy = d.Dot(v.up), cz = d.Dot(v.forward);
	if (v.ortho)
	{
		float s = v.height * 0.5f / v.halfHeight;
		*x = v.width * 0.5f + cx * s;
		*y = v.height * 0.5f - cy * s;
		return true;
	}
	if (cz < 0.1f)
		return false;
	float f = v.height * 0.5f / (float)tan(v.fov * 3.14159265f / 360.0f);
	*x = v.width * 0.5f + cx / cz * f;
	*y = v.height * 0.5f - cy / cz * f;
	return true;
}

void SceneRay(const SceneView& v, int x, int y, WVector* origin, WVector* dir)
{
	float nx = x - v.width * 0.5f, ny = v.height * 0.5f - y;
	if (v.ortho)
	{
		float s = v.halfHeight / (v.height * 0.5f);
		*origin = v.eye + v.right * (nx * s) + v.up * (ny * s) - v.forward * 20000.0f;
		*dir = v.forward;
		return;
	}
	float f = v.height * 0.5f / (float)tan(v.fov * 3.14159265f / 360.0f);
	WVector d = v.forward * f + v.right * nx + v.up * ny;
	d.Normalize();
	*origin = v.eye;
	*dir = d;
}

static void SetupCamera(const SceneView& v)
{
	glViewport(0, 0, v.width, v.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = (float)v.width / (float)(v.height > 0 ? v.height : 1);
	if (v.ortho)
		glOrtho(-v.halfHeight * aspect, v.halfHeight * aspect, -v.halfHeight, v.halfHeight, -60000.0, 60000.0);
	else
		gluPerspective(v.fov, aspect, 1.0, 12000.0);
	glMatrixMode(GL_MODELVIEW);
	float m[16] = {
		v.right.x, v.up.x, -v.forward.x, 0,
		v.right.y, v.up.y, -v.forward.y, 0,
		v.right.z, v.up.z, -v.forward.z, 0,
		-v.right.Dot(v.eye), -v.up.Dot(v.eye), v.forward.Dot(v.eye), 1
	};
	glLoadMatrixf(m);
}

// WMatrix is 12 floats, three rows and a pivot; OpenGL wants them as four-long columns.
static void MultMatrix(const WMatrix& m)
{
	float g[16] = {
		m.xx, m.yx, m.zx, 0,
		m.xy, m.yy, m.zy, 0,
		m.xz, m.yz, m.zz, 0,
		m.xm, m.ym, m.zm, 1
	};
	glMultMatrixf(g);
}

static void Vtx(const WVector& p)
{
	glVertex3f(p.x, p.y, p.z);
}

static void DrawText3D(const WVector& p, const wchar_t* text)
{
	glRasterPos3f(p.x, p.y, p.z);
	HDC dc = wglGetCurrentDC();
	static HFONT font;
	if (!font)
		font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, NONANTIALIASED_QUALITY, 0, L"Gulim");
	for (const wchar_t* c = text; *c; ++c)
	{
		std::map<wchar_t, GLuint>::iterator it = g_glyphs.find(*c);
		GLuint list;
		if (it != g_glyphs.end())
			list = it->second;
		else
		{
			list = glGenLists(1);
			HGDIOBJ old = SelectObject(dc, font);
			wglUseFontBitmapsW(dc, *c, 1, list);
			SelectObject(dc, old);
			g_glyphs[*c] = list;
		}
		glCallList(list);
	}
}

void SceneDrawOverlayText(const SceneView& v, const wchar_t* text, int x, int y)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, v.width, v.height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	WVector p((float)x, (float)y, 0.0f);
	DrawText3D(p, text);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

static void DrawGrid(const SceneView& v)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	float aspect = (float)v.width / (float)(v.height > 0 ? v.height : 1);
	float halfW = v.halfHeight * aspect;
	float step = g_ed.grid;
	while (step * v.height / (2.0f * v.halfHeight) < 12.0f)
		step *= 2.0f;
	float major = step * 8.0f;
	WVector c = v.eye;
	float cx = c.Dot(v.right), cy = c.Dot(v.up);
	glBegin(GL_LINES);
	for (int axis = 0; axis < 2; ++axis)
	{
		const WVector& dir = axis == 0 ? v.right : v.up;
		const WVector& other = axis == 0 ? v.up : v.right;
		float center = axis == 0 ? cx : cy;
		float half = axis == 0 ? halfW : v.halfHeight;
		float otherCenter = axis == 0 ? cy : cx;
		float otherHalf = axis == 0 ? v.halfHeight : halfW;
		int first = (int)floor((center - half) / step), last = (int)ceil((center + half) / step);
		for (int i = first; i <= last; ++i)
		{
			float at = i * step;
			if (i == 0)
				glColor3f(0.42f, 0.42f, 0.62f);
			else if (fmod(fabs(at) + 0.001f, major) < 0.01f)
				glColor3f(0.28f, 0.28f, 0.30f);
			else
				glColor3f(0.17f, 0.17f, 0.18f);
			Vtx(dir * at + other * (otherCenter - otherHalf));
			Vtx(dir * at + other * (otherCenter + otherHalf));
		}
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
}

static void BoxLines(const WVector& lo, const WVector& hi)
{
	static const int e[12][2] = { {0,1},{1,3},{3,2},{2,0},{4,5},{5,7},{7,6},{6,4},{0,4},{1,5},{2,6},{3,7} };
	glBegin(GL_LINES);
	for (int i = 0; i < 12; ++i)
	{
		for (int k = 0; k < 2; ++k)
		{
			int w = e[i][k];
			glVertex3f(w & 1 ? hi.x : lo.x, w & 2 ? hi.y : lo.y, w & 4 ? hi.z : lo.z);
		}
	}
	glEnd();
}

// A marker that stays the same size on screen.
static float PixelScale(const SceneView& v, const WVector& p)
{
	if (v.ortho)
		return 2.0f * v.halfHeight / v.height;
	float z = (p - v.eye).Dot(v.forward);
	if (z < 1.0f)
		z = 1.0f;
	return 2.0f * z * (float)tan(v.fov * 3.14159265f / 360.0f) / v.height;
}

static void Diamond(const SceneView& v, const WVector& p, float px)
{
	float s = PixelScale(v, p) * px;
	WVector r = v.right * s, u = v.up * s;
	glBegin(GL_LINE_LOOP);
	Vtx(p + u);
	Vtx(p + r);
	Vtx(p - u);
	Vtx(p - r);
	glEnd();
}

static void Label(const SceneView& v, const WVector& p, const std::wstring& text)
{
	if (!g_ed.showLabels || text.empty())
		return;
	float s = PixelScale(v, p);
	DrawText3D(p + v.right * (9.0f * s) + v.up * (5.0f * s), text.c_str());
}

// -- elements -----------------------------------------------------------------

static void DrawPointElement(const SceneView& v, const WepElement& e, bool selected)
{
	WVector p = e.GetVec("Pos");
	std::string cls = e.Class(), name = e.Get("Name");
	float r = 0.95f, g = 0.95f, b = 0.3f;
	bool tee = false, pin = false;
	for (int s = 0; s < Course::HoleSlotCount(); ++s)
	{
		if (name == Course::HoleSlotName(s))
		{
			if (s < 2) tee = true;
			else pin = true;
		}
	}
	if (cls == "WSun") { r = 1.0f; g = 0.75f; b = 0.1f; }
	else if (cls == "WMoon") { r = 0.7f; g = 0.8f; b = 1.0f; }
	else if (cls == "WGlobalLight" || cls == "WSecondaryGlobalLight") { r = 1.0f; g = 1.0f; b = 0.7f; }
	else if (tee) { r = 0.3f; g = 0.7f; b = 1.0f; }
	else if (pin) { r = 1.0f; g = 0.35f; b = 0.35f; }
	if (selected) { r = 1.0f; g = 1.0f; b = 1.0f; }
	glColor3f(r, g, b);
	Course& c = g_ed.course;
	if ((tee || pin) && p.y == 0.0f)
		c.StandOnGround(&p);	// shipped tee and pin points all stand at y = 0
	Diamond(v, p, selected ? 8.0f : 6.0f);
	if (pin)
	{
		float s = PixelScale(v, p);
		WVector up(0, 1, 0);
		float h = max(5.0f, s * 26.0f);
		glBegin(GL_LINES);
		Vtx(p);
		Vtx(p + up * h);
		glEnd();
		glBegin(GL_TRIANGLES);
		Vtx(p + up * h);
		Vtx(p + up * (h * 0.7f));
		Vtx(p + up * (h * 0.85f) + v.right * (h * 0.35f));
		glEnd();
	}
	if (cls == "WGlobalLight")
	{
		glBegin(GL_LINES);
		Vtx(p);
		Vtx(p + c.SunDirection() * (PixelScale(v, p) * 30.0f));
		glEnd();
	}
	std::wstring text = WideCp(name);
	if (text.empty())
		text = Wide(cls);
	Label(v, p, text);
}

static void DrawCameraElement(const SceneView& v, const WepElement& e, bool selected)
{
	WVector p = e.GetVec("Pos"), d = e.GetVec("Dest");
	if (selected)
		glColor3f(1, 1, 1);
	else
		glColor3f(0.35f, 0.9f, 0.45f);
	WVector f = d - p;
	f.Normalize();
	WVector r = WVector(0, 1, 0).Cross(f);
	r.Normalize();
	WVector u = f.Cross(r);
	float s = PixelScale(v, p) * 16.0f;
	float t = (float)tan(e.GetFloat("Fov", 60.0f) * 3.14159265f / 360.0f);
	WVector c = p + f * (s * 1.6f);
	WVector cr = r * (s * 1.6f * t * 1.33f), cu = u * (s * 1.6f * t);
	glBegin(GL_LINES);
	Vtx(p); Vtx(c + cr + cu);
	Vtx(p); Vtx(c - cr + cu);
	Vtx(p); Vtx(c + cr - cu);
	Vtx(p); Vtx(c - cr - cu);
	Vtx(c + cr + cu); Vtx(c - cr + cu);
	Vtx(c - cr + cu); Vtx(c - cr - cu);
	Vtx(c - cr - cu); Vtx(c + cr - cu);
	Vtx(c + cr - cu); Vtx(c + cr + cu);
	glEnd();
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0F0F);
	glBegin(GL_LINES);
	Vtx(p);
	Vtx(d);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	Diamond(v, d, 3.0f);
	Label(v, p, WideCp(e.Get("Name")));
}

static void AreaColor(const std::string& cls, float* r, float* g, float* b)
{
	*r = 0.9f; *g = 0.6f; *b = 0.2f;
	if (cls == "WNPC") { *r = 0.9f; *g = 0.4f; *b = 0.9f; }
	else if (cls == "WLake") { *r = 0.25f; *g = 0.6f; *b = 1.0f; }
	else if (cls == "WWindArea") { *r = 0.75f; *g = 1.0f; *b = 1.0f; }
	else if (cls == "WSoundArea") { *r = 0.3f; *g = 0.9f; *b = 0.8f; }
	else if (cls == "WFxArea") { *r = 1.0f; *g = 0.4f; *b = 0.3f; }
}

static void NodeColor(const std::string& name, float* r, float* g, float* b)
{
	*r = 0.8f; *g = 0.8f; *b = 0.8f;
	if (name.find("outline") != std::string::npos) { *r = 1.0f; *g = 0.2f; *b = 0.2f; }
	else if (name.find("auto") != std::string::npos) { *r = 0.2f; *g = 1.0f; *b = 0.4f; }
	else if (name.find("ib") != std::string::npos) { *r = 0.9f; *g = 0.2f; *b = 0.7f; }
	else if (name.find("ob") != std::string::npos) { *r = 1.0f; *g = 0.6f; *b = 0.1f; }
	else if (name.find("npc") != std::string::npos) { *r = 0.8f; *g = 0.4f; *b = 1.0f; }
	else if (name.find("river") != std::string::npos || name.find("sea") != std::string::npos ||
		name.find("wave") != std::string::npos || name.find("beach") != std::string::npos || name.find("waterfall") != std::string::npos)
	{ *r = 0.2f; *g = 0.7f; *b = 1.0f; }
	else if (name.find("lava") != std::string::npos) { *r = 1.0f; *g = 0.3f; *b = 0.0f; }
	else if (name.find("camera") != std::string::npos) { *r = 0.3f; *g = 0.9f; *b = 0.5f; }
}

static void DrawNodes(const SceneView& v)
{
	const std::vector<WepNode>& nodes = g_ed.course.wep.nodes;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const WepNode& n = nodes[i];
		if (n.points.empty())
			continue;
		bool active = g_ed.activeNode == (int)i;
		bool selected = g_ed.IsSelected(ElementRef(ElementRef::NODE, (int)i));
		std::string name = n.Get("Name");
		float r, g, b;
		NodeColor(name, &r, &g, &b);
		if (!n.GetInt("Show", 1))
		{
			r *= 0.5f; g *= 0.5f; b *= 0.5f;
		}
		if (selected || active)
			glLineWidth(2.0f);
		glColor3f(r, g, b);
		bool closed = name.find("outline") != std::string::npos || name.find("ib") != std::string::npos || name.find("ob") != std::string::npos;
		glBegin(closed ? GL_LINE_LOOP : GL_LINE_STRIP);
		for (size_t k = 0; k < n.points.size(); ++k)
			Vtx(n.points[k].pos);
		glEnd();
		glLineWidth(1.0f);
		if (active || selected || g_ed.tool == TOOL_NODE)
		{
			for (size_t k = 0; k < n.points.size(); ++k)
			{
				bool sp = g_ed.IsSelected(ElementRef(ElementRef::NODE_POINT, (int)i, (int)k));
				glColor3f(sp ? 1.0f : r, sp ? 1.0f : g, sp ? 1.0f : b);
				Diamond(v, n.points[k].pos, sp ? 6.0f : 4.0f);
			}
		}
		glColor3f(r, g, b);
		Label(v, n.points[0].pos, WideCp(name));
	}
}

// -- what scripts do ----------------------------------------------------------

void SceneCallPuppet(PuppetAsset* a)
{
	PuppetRender* r = Render(a, false);
	glCallList(r->lists[PASS_OPAQUE]);
	if (r->hasBlend)
	{
		glCallList(r->lists[PASS_BLEND_SOLID]);
		glCallList(r->lists[PASS_BLEND_SOFT]);
	}
}

// Water CPool would build: a sheet at the top of the box, drawn in the scene's depth.
static void DrawScriptSurfaces()
{
	const std::vector<WepElement>& els = g_ed.course.wep.elements;
	if (!g_ed.showAreas)
		return;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	for (size_t i = 0; i < els.size(); ++i)
	{
		if (!els[i].IsArea())
			continue;
		WVector lo, hi;
		els[i].GetBox(&lo, &hi);
		if (els[i].Class() == "WLake")
		{
			glColor4f(0.2f, 0.5f, 0.95f, 0.3f);
			glBegin(GL_QUADS);
			glVertex3f(lo.x, hi.y, lo.z);
			glVertex3f(hi.x, hi.y, lo.z);
			glVertex3f(hi.x, hi.y, hi.z);
			glVertex3f(lo.x, hi.y, hi.z);
			glEnd();
		}
	}
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

static void Arrow(const SceneView& v, const WVector& from, const WVector& dir, float length)
{
	WVector to = from + dir * length;
	glBegin(GL_LINES);
	Vtx(from);
	Vtx(to);
	WVector side = dir.Cross(v.forward);
	side.Normalize();
	Vtx(to);
	Vtx(to - dir * (length * 0.3f) + side * (length * 0.15f));
	Vtx(to);
	Vtx(to - dir * (length * 0.3f) - side * (length * 0.15f));
	glEnd();
}

static void DrawNpcGhost(const std::string& pet, const WVector& at)
{
	PuppetAsset* a = g_ed.course.Puppet(pet);
	if (!a->loaded)
		return;
	glPushMatrix();
	glTranslatef(at.x, at.y, at.z);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glCallList(Render(a, false)->wire);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPopMatrix();
}

static void DrawScriptMarks(const SceneView& v)
{
	Course& c = g_ed.course;
	const std::vector<WepElement>& els = c.wep.elements;
	if (g_ed.showAreas)
	{
		for (size_t i = 0; i < els.size(); ++i)
		{
			if (!els[i].IsArea())
				continue;
			AreaScript s = ParseAreaScript(els[i]);
			WVector lo, hi;
			els[i].GetBox(&lo, &hi);
			WVector mid = (lo + hi) * 0.5f;
			switch (s.kind)
			{
			case AREA_WIND:
			{
				// A row of arrows through the box, sized with the strength.
				glColor3f(0.75f, 1.0f, 1.0f);
				float length = PixelScale(v, mid) * (14.0f + min(s.windIntensity, 40.0f) * 0.5f);
				for (int ax = -1; ax <= 1; ++ax)
					for (int az = -1; az <= 1; ++az)
						Arrow(v, mid + WVector(ax * (hi.x - lo.x) * 0.3f, 0.0f, az * (hi.z - lo.z) * 0.3f), s.windDirection, length);
				break;
			}
			case AREA_CAMERA:
			{
				int cam = c.wep.FindElement("", s.cameraName);
				if (cam >= 0 && els[cam].IsCamera())
				{
					glColor3f(0.35f, 0.9f, 0.45f);
					glEnable(GL_LINE_STIPPLE);
					glLineStipple(2, 0xAAAA);
					glBegin(GL_LINES);
					Vtx(mid);
					Vtx(els[cam].GetVec("Pos"));
					glEnd();
					glDisable(GL_LINE_STIPPLE);
				}
				else
				{
					glColor3f(1.0f, 0.2f, 0.2f);
					Diamond(v, mid, 8.0f);
				}
				break;
			}
			case AREA_EXTRA:
			case AREA_SHORTGAME:
				// The floor of the box, where the tee lands.
				glColor3f(0.3f, 0.7f, 1.0f);
				glBegin(GL_LINES);
				glVertex3f(lo.x, lo.y, lo.z);
				glVertex3f(hi.x, lo.y, hi.z);
				glVertex3f(hi.x, lo.y, lo.z);
				glVertex3f(lo.x, lo.y, hi.z);
				glEnd();
				break;
			case AREA_NPC:
				glColor3f(0.9f, 0.4f, 0.9f);
				if (!s.petName.empty())
					DrawNpcGhost(s.petName, WVector(mid.x, lo.y, mid.z));
				break;
			case AREA_SOUND:
			{
				glColor3f(0.3f, 0.9f, 0.8f);
				glBegin(GL_LINE_LOOP);
				for (int k = 0; k < 40; ++k)
				{
					float a = k * 6.2831853f / 40.0f;
					glVertex3f(mid.x + (float)cos(a) * s.soundDistance, mid.y, mid.z + (float)sin(a) * s.soundDistance);
				}
				glEnd();
				break;
			}
			case AREA_FX:
			{
				// The way the script turns the effect: its local Z.
				glColor3f(1.0f, 0.4f, 0.3f);
				WVector dir = FxRotEuler(s.fxRotation).Rotate(WVector(0, 0, 1));
				Arrow(v, mid, dir, PixelScale(v, mid) * 28.0f);
				break;
			}
			default:
				break;
			}
			if (!s.problems.empty())
			{
				glColor3f(1.0f, 0.15f, 0.15f);
				Diamond(v, lo, 7.0f);
			}
		}
	}
	if (g_ed.showNodes)
	{
		for (size_t i = 0; i < c.wep.nodes.size(); ++i)
		{
			const WepNode& n = c.wep.nodes[i];
			if (n.points.empty())
				continue;
			NodeScript s = ParseNodeScript(n.Get("Name"));
			if (s.petName.empty())
				continue;
			glColor3f(0.8f, 0.4f, 1.0f);
			DrawNpcGhost(s.petName, n.points[0].pos);
			if (n.points.size() >= 2)
			{
				WVector dir = n.points[1].pos - n.points[0].pos;
				dir.Normalize();
				Arrow(v, n.points[0].pos, dir, PixelScale(v, n.points[0].pos) * 30.0f);
			}
		}
	}
}

// -- the scene ----------------------------------------------------------------

static void SetupLight()
{
	Course& c = g_ed.course;
	float amb[3], dif[3];
	c.GameColor("Ambient", amb);
	c.GameColor("Diffuse", dif);
	float a4[4] = { amb[0], amb[1], amb[2], 1 }, d4[4] = { dif[0], dif[1], dif[2], 1 };
	WVector to = c.SunDirection() * -1.0f;
	float pos[4] = { to.x, to.y, to.z, 0 };
	float black[4] = { 0, 0, 0, 1 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
	glLightfv(GL_LIGHT0, GL_AMBIENT, a4);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, d4);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_NORMALIZE);
}

void SceneDraw(const SceneView& v)
{
	Course& c = g_ed.course;
	float clear[3] = { 0.0f, 0.0f, 0.0f };
	if (!v.ortho)
	{
		clear[0] = c.fogColor[0];
		clear[1] = c.fogColor[1];
		clear[2] = c.fogColor[2];
		if (!c.hasFog)
		{
			clear[0] = 0.45f; clear[1] = 0.58f; clear[2] = 0.75f;
		}
	}
	glClearColor(clear[0], clear[1], clear[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	SetupCamera(v);
	if (v.ortho)
		DrawGrid(v);
	if (!v.ortho && g_ed.showSky && v.shade == SHADE_TEXTURED)
		DrawSky(v);
	SetupLight();

	bool fog = !v.ortho && g_ed.showFog && c.hasFog && v.shade == SHADE_TEXTURED;
	if (fog)
	{
		float col[4] = { c.fogColor[0], c.fogColor[1], c.fogColor[2], 1 };
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, col);
		glFogf(GL_FOG_START, c.fogNear);
		glFogf(GL_FOG_END, c.fogFar);
	}

	const std::vector<WepElement>& els = c.wep.elements;
	bool wire = v.shade == SHADE_WIRE;
	bool textured = v.shade == SHADE_TEXTURED;
	bool surface = v.shade == SHADE_SURFACE;
	if (wire)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.0f);
	if (g_ed.showPuppets)
	{
		for (int pass = 0; pass < (textured ? (int)PASS_COUNT : 1); ++pass)
		{
			for (size_t i = 0; i < els.size(); ++i)
			{
				const WepElement& e = els[i];
				if (!e.IsPuppet())
					continue;
				PuppetAsset* a = c.Puppet(e.Get("Filename"));
				if (!a->loaded)
					continue;
				bool ground = e.IsGround();
				PuppetRender* r = Render(a, ground);
				if (pass != PASS_OPAQUE && !r->hasBlend)
					continue;
				WMatrix m = c.ElementMatrix(e);
				glPushMatrix();
				MultMatrix(m);
				if (textured)
				{
					glCallList(r->lists[pass]);
				}
				else if (surface)
				{
					BuildSurface(a, r);
					glDisable(GL_LIGHTING);
					glCallList(r->surface);
				}
				else if (wire)
				{
					glDisable(GL_LIGHTING);
					if (ground)
						glColor3f(0.28f, 0.45f, 0.30f);
					else if (e.Class() == "WRotObj")
						glColor3f(0.8f, 0.5f, 0.8f);
					else if (e.Class() == "WVtxObj")
						glColor3f(0.5f, 0.75f, 0.85f);
					else
						glColor3f(0.55f, 0.55f, 0.75f);
					glCallList(r->wire);
				}
				else
				{
					glEnable(GL_LIGHTING);
					if (ground)
						glColor3f(0.55f, 0.72f, 0.5f);
					else
						glColor3f(0.72f, 0.72f, 0.76f);
					glCallList(r->wire);
				}
				glPopMatrix();
			}
			if (pass == PASS_OPAQUE && textured && c.wep.Ground())
			{
				PuppetAsset* a = c.BasePuppet();
				if (a && Render(a, true)->hasRim)
				{
					WMatrix m = c.ElementMatrix(*c.wep.Ground());
					glPushMatrix();
					MultMatrix(m);
					glDisable(GL_LIGHTING);
					glDisable(GL_ALPHA_TEST);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glDepthMask(GL_FALSE);
					glCallList(((PuppetRender*)a->render)->rim);
					glDepthMask(GL_TRUE);
					glDisable(GL_BLEND);
					glPopMatrix();
				}
			}
			// The base model's shadow maps sit between opaque and blended geometry.
			if (pass == PASS_OPAQUE && textured && g_ed.showShadow && c.hasSbin)
			{
				if (g_shadowFor != c.revision || !g_shadowList)
				{
					// Texture objects go when the project does; rebuilding per revision is
					// cheap next to how rarely the sbin itself changes.
					if (!g_shadowTextures.empty())
					{
						glDeleteTextures((GLsizei)g_shadowTextures.size(), &g_shadowTextures[0]);
						g_shadowTextures.clear();
					}
					BuildShadowList();
				}
				const WepElement* g = c.wep.Ground();
				if (g)
				{
					WMatrix m = c.ElementMatrix(*g);
					glPushMatrix();
					MultMatrix(m);
					glDisable(GL_LIGHTING);
					glEnable(GL_TEXTURE_2D);
					glEnable(GL_BLEND);
					glBlendFunc(GL_ZERO, GL_SRC_COLOR);
					glDepthMask(GL_FALSE);
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(-1.0f, -1.0f);
					glColor3f(1, 1, 1);
					glCallList(g_shadowList);
					glDisable(GL_POLYGON_OFFSET_FILL);
					glDepthMask(GL_TRUE);
					glDisable(GL_BLEND);
					glDisable(GL_TEXTURE_2D);
					glPopMatrix();
				}
			}
		}
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);

	if (!v.ortho)
		AreaFxDraw(v);
	glDisable(GL_LIGHTING);
	if (g_ed.showScripts)
		DrawScriptSurfaces();

	// Editor marks, always on top of the depth they share with the scene.
	glDepthFunc(GL_ALWAYS);
	if (g_ed.showScripts)
		DrawScriptMarks(v);
	for (size_t i = 0; i < els.size(); ++i)
	{
		const WepElement& e = els[i];
		ElementRef ref(ElementRef::ELEMENT, (int)i);
		bool selected = g_ed.IsSelected(ref) || g_ed.IsSelected(ElementRef(ElementRef::ELEMENT, (int)i, 1));
		if (e.IsPoint() && g_ed.showPoints)
		{
			DrawPointElement(v, e, selected);
		}
		else if (e.IsCamera() && g_ed.showCameras)
		{
			DrawCameraElement(v, e, selected);
		}
		else if (e.IsArea() && g_ed.showAreas)
		{
			WVector lo, hi;
			e.GetBox(&lo, &hi);
			float r, g, b;
			AreaColor(e.Class(), &r, &g, &b);
			if (selected)
				glColor3f(1, 1, 1);
			else
				glColor3f(r, g, b);
			BoxLines(lo, hi);
			if (g_ed.showLabels)
				Label(v, lo, g_ed.showScripts ? Wide(ParseAreaScript(e).summary) : WideCp(e.Get("Name")));
		}
		else if (e.IsPuppet() && selected)
		{
			PuppetAsset* a = c.Puppet(e.Get("Filename"));
			WMatrix m = c.ElementMatrix(e);
			glColor3f(1.0f, 0.95f, 0.3f);
			if (a->loaded)
			{
				glPushMatrix();
				MultMatrix(m);
				if (!e.IsGround())
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDepthFunc(GL_LEQUAL);
					Render(a, false);
					glCallList(((PuppetRender*)a->render)->wire);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDepthFunc(GL_ALWAYS);
				}
				BoxLines(a->lo, a->hi);
				glPopMatrix();
			}
		}
		else if (e.IsPuppet() && !c.Puppet(e.Get("Filename"))->loaded && g_ed.showPuppets)
		{
			// Unresolved puppet: a box and its name so it can still be placed.
			WVector p = e.GetVec("Pos");
			glColor3f(1.0f, 0.3f, 0.5f);
			BoxLines(p - WVector(2, 2, 2), p + WVector(2, 2, 2));
			Label(v, p, WideCp(PathBaseName(e.Get("Filename"))));
		}
	}
	if (g_ed.showNodes)
		DrawNodes(v);
	glDepthFunc(GL_LEQUAL);
}

void SceneDrawPreview(PuppetAsset* a, float yaw, int width, int height)
{
	glViewport(0, 0, width, height);
	glClearColor(0.16f, 0.17f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!a || !a->loaded)
		return;
	WVector center = (a->lo + a->hi) * 0.5f;
	float r = (a->hi - a->lo).Length() * 0.5f;
	if (r < 0.01f)
		r = 1.0f;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, (double)width / (height ? height : 1), r * 0.05, r * 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -r * 2.8f);
	glRotatef(20, 1, 0, 0);
	glRotatef(yaw, 0, 1, 0);
	glScalef(1, 1, -1);
	glTranslatef(-center.x, -center.y, -center.z);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glEnable(GL_LIGHT0);
	float amb[4] = { 0.5f, 0.5f, 0.5f, 1 }, dif[4] = { 0.8f, 0.8f, 0.8f, 1 }, pos[4] = { 0.3f, 0.8f, 0.5f, 0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_NORMALIZE);
	PuppetRender* rr = Render(a, false);
	for (int pass = 0; pass < PASS_COUNT; ++pass)
		if (pass == PASS_OPAQUE || rr->hasBlend)
			glCallList(rr->lists[pass]);
	glDisable(GL_LIGHTING);
}
