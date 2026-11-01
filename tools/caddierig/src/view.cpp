// SPDX-License-Identifier: MIT
// view.cpp - fixed-function viewport. Everything goes through client-side vertex
// arrays and glBegin; nothing newer than OpenGL 1.1 is required.
#include "view.h"
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <algorithm>
#include <map>
#include <vector>
#include "fxdraw.h"
#include "texture.h"

static HWND g_wnd;
static HDC g_dc;
static HGLRC g_rc;
static Document* g_doc;
static ViewOptions* g_opts;
static GLuint g_fontBase;
static FxBox* g_fx;

static float g_yaw = 0.6f, g_pitch = 0.3f, g_dist = 3.0f;
static WVector g_target;
static int g_preset;
static int g_lastX, g_lastY;

void ViewSetFx(FxBox* box)
{
	g_fx = box;
}

void ViewFrameAll()
{
	if (!g_doc)
		return;
	WVector lo, hi;
	g_doc->WorldBounds(&lo, &hi);
	g_target = (lo + hi) * 0.5f;
	float r = (hi - lo).Length() * 0.5f;
	if (r < 0.001f)
		r = 1.0f;
	g_dist = r * 2.6f;
	InvalidateRect(g_wnd, 0, FALSE);
}

void ViewSetPreset(int preset)
{
	g_preset = preset;
	switch (preset)
	{
	case 1: g_yaw = 0.0f; g_pitch = 0.0f; break;
	case 2: g_yaw = 1.5708f; g_pitch = 0.0f; break;
	case 3: g_yaw = 0.0f; g_pitch = 1.5f; break;
	default: break;
	}
	InvalidateRect(g_wnd, 0, FALSE);
}

// -- textures -----------------------------------------------------------------

// What the file name prefix asks for, as far as LoadPET_Texture shows it. The
// exact D3D states behind the draw flags are not in the loader; these are the
// closest fixed-function equivalents.
enum BlendMode { BLEND_NONE, BLEND_ALPHA, BLEND_TEST };

static BlendMode BlendOfTexture(const std::string& name)
{
	if (NameHasFlag(name, "]{"))
		return BLEND_TEST;
	return NameHasFlag(name, "+[~}") ? BLEND_ALPHA : BLEND_NONE;
}

// A texture *ptex has written to. The client patches video memory and never gets the
// original back except through "reset" images; keeping one here makes seeking possible.
struct LiveTexture
{
	MeshPart* part;
	int slot;
	TexImage base, current;
	bool dirty;
};
static std::vector<LiveTexture*> g_live;
static std::map<std::string, TexPart*> g_partCache;		// null: known not to work
static std::map<std::string, std::string> g_partErrors;

static void ClearLiveTextures()
{
	for (size_t i = 0; i < g_live.size(); ++i)
		delete g_live[i];
	g_live.clear();
	for (std::map<std::string, TexPart*>::iterator i = g_partCache.begin(); i != g_partCache.end(); ++i)
		delete i->second;
	g_partCache.clear();
	g_partErrors.clear();
}

static std::string ResolveForPart(const std::string& name, void* doc)
{
	return ((Document*)doc)->ResolveAsset(name);
}

static void ReloadPartTextures(MeshPart& part)
{
	for (size_t i = 0; i < part.glTextures.size(); ++i)
		if (part.glTextures[i])
			glDeleteTextures(1, &part.glTextures[i]);
	part.glTextures.assign(part.pet->textures.size(), 0);
	for (size_t i = 0; i < part.pet->textures.size(); ++i)
	{
		std::string name = Cp949ToUtf8(part.pet->textures[i].Name());
		if (name.empty() || name == "null")
			continue;
		std::string path = i < part.textureOverride.size() && !part.textureOverride[i].empty()
			? part.textureOverride[i] : g_doc->ResolveTexture(name);
		if (!path.empty())
			part.glTextures[i] = LoadGLTexture(path);
	}
}

static void EnsureTextures()
{
	if (!g_doc->texturesDirty)
		return;
	g_doc->texturesDirty = false;
	ClearLiveTextures();
	wglMakeCurrent(g_dc, g_rc);
	for (size_t p = 0; p < g_doc->parts.size(); ++p)
		ReloadPartTextures(*g_doc->parts[p]);
}

// Parts come and go with the wardrobe; only the new ones need loading.
static void LoadNewPartTextures()
{
	for (size_t p = 0; p < g_doc->parts.size(); ++p)
		if (g_doc->parts[p]->glTextures.size() != g_doc->parts[p]->pet->textures.size())
			ReloadPartTextures(*g_doc->parts[p]);
}

void ViewForgetPart(MeshPart* part)
{
	for (size_t i = 0; i < part->glTextures.size(); ++i)
		if (part->glTextures[i])
			glDeleteTextures(1, &part->glTextures[i]);
	part->glTextures.clear();
	for (size_t i = g_live.size(); i-- > 0;)
	{
		if (g_live[i]->part == part)
		{
			delete g_live[i];
			g_live.erase(g_live.begin() + i);
		}
	}
}

void ViewSetPartVariant(MeshPart* part, const std::vector<std::string>& paths)
{
	part->textureOverride = paths;
	ReloadPartTextures(*part);
	InvalidateRect(g_wnd, 0, FALSE);
}

// The texture a *ptex names. WPuppet::ChangeTexturePart looks in FANM; CGolfPlayer
// ignores the name and always uses the character's face (partsinfo.cpp: FaceTex).
static bool FindPartTarget(const std::string& target, MeshPart** outPart, int* outSlot)
{
	if (g_doc->IsAvatar())
	{
		// Accessories also carry "_fc_", and hair can share the file. The face is the
		// texture named for it, on whichever part wears it now.
		MeshPart* fallback = 0;
		for (size_t p = 0; p < g_doc->parts.size(); ++p)
		{
			MeshPart* part = g_doc->parts[p];
			for (size_t t = 0; t < part->pet->textures.size(); ++t)
			{
				std::string name = part->pet->textures[t].Name();
				std::transform(name.begin(), name.end(), name.begin(), ::tolower);
				if (name.find("face") != std::string::npos)
				{
					*outPart = part;
					*outSlot = (int)t;
					return true;
				}
			}
			if (!fallback && part->name.find("_fc_") != std::string::npos && !part->pet->textures.empty())
				fallback = part;
		}
		if (!fallback)
			return false;
		*outPart = fallback;
		*outSlot = 0;
		return true;
	}
	for (size_t p = 0; p < g_doc->parts.size(); ++p)
	{
		MeshPart* part = g_doc->parts[p];
		for (size_t f = 0; f < part->pet->faces.size(); ++f)
		{
			if (_stricmp(part->pet->faces[f].Name().c_str(), target.c_str()))
				continue;
			for (size_t t = 0; t < part->pet->textures.size(); ++t)
			{
				if (!_stricmp(part->pet->textures[t].Name().c_str(), part->pet->faces[f].File().c_str()))
				{
					*outPart = part;
					*outSlot = (int)t;
					return true;
				}
			}
		}
	}
	return false;
}

bool ViewApplyTexPart(const std::string& target, const std::string& image, std::string* why)
{
	EnsureTextures();
	LoadNewPartTextures();
	MeshPart* part;
	int slot;
	if (!FindPartTarget(target, &part, &slot))
	{
		*why = "no texture for target";
		return false;
	}
	if (slot >= (int)part->glTextures.size() || !part->glTextures[slot])
	{
		*why = "target texture not loaded";
		return false;
	}
	LiveTexture* live = 0;
	for (size_t i = 0; i < g_live.size(); ++i)
		if (g_live[i]->part == part && g_live[i]->slot == slot)
			live = g_live[i];
	std::string dstName = part->pet->textures[slot].Name();
	if (!live)
	{
		live = new LiveTexture;
		live->part = part;
		live->slot = slot;
		live->dirty = false;
		if (!LoadTexImage(g_doc->ResolveAsset(dstName), &live->base))
		{
			delete live;
			*why = "target texture not loaded";
			return false;
		}
		live->current = live->base;
		g_live.push_back(live);
	}
	std::string key = dstName + "|" + image;
	std::map<std::string, TexPart*>::iterator it = g_partCache.find(key);
	TexPart* tp;
	if (it != g_partCache.end())
		tp = it->second;
	else
	{
		tp = new TexPart;
		std::string error;
		if (!LoadTexPart(dstName, image, live->base.w, live->base.h, ResolveForPart, g_doc, tp, &error))
		{
			delete tp;
			tp = 0;
			g_partErrors[key] = error;
		}
		g_partCache[key] = tp;
	}
	if (!tp)
	{
		*why = g_partErrors[key];
		return false;
	}
	for (int r = 0; r < tp->height; ++r)
	{
		unsigned char* dst = &live->current.rgba[((tp->top + r) * live->current.w + tp->left) * 4];
		const unsigned char* src = &tp->rgba[r * tp->width * 4];
		if (tp->hasAlpha)
			memcpy(dst, src, tp->width * 4);
		else
			for (int c = 0; c < tp->width; ++c)
				memcpy(dst + c * 4, src + c * 4, 3);
	}
	live->dirty = true;
	InvalidateRect(g_wnd, 0, FALSE);
	return true;
}

void ViewResetTexParts()
{
	for (size_t i = 0; i < g_live.size(); ++i)
	{
		g_live[i]->current = g_live[i]->base;
		g_live[i]->dirty = true;
	}
}

static void UploadLiveTextures()
{
	for (size_t i = 0; i < g_live.size(); ++i)
	{
		if (!g_live[i]->dirty)
			continue;
		g_live[i]->dirty = false;
		UploadTexImage(g_live[i]->part->glTextures[g_live[i]->slot], g_live[i]->current);
	}
}

void ViewRefreshTextures()
{
	g_doc->texturesDirty = true;
	InvalidateRect(g_wnd, 0, FALSE);
}

// -- drawing ------------------------------------------------------------------

static void DrawBoxLines(const float b[6])
{
	glBegin(GL_LINES);
	static const int e[12][2] = { {0,1},{1,3},{3,2},{2,0},{4,5},{5,7},{7,6},{6,4},{0,4},{1,5},{2,6},{3,7} };
	for (int i = 0; i < 12; ++i)
	{
		for (int k = 0; k < 2; ++k)
		{
			int v = e[i][k];
			glVertex3f(v & 1 ? b[3] : b[0], v & 2 ? b[4] : b[1], v & 4 ? b[5] : b[2]);
		}
	}
	glEnd();
}

static void DrawSphereLines(const float b[6])
{
	float cx = (b[0] + b[3]) * 0.5f, cy = (b[1] + b[4]) * 0.5f, cz = (b[2] + b[5]) * 0.5f;
	float rx = (b[3] - b[0]) * 0.5f, ry = (b[4] - b[1]) * 0.5f, rz = (b[5] - b[2]) * 0.5f;
	for (int axis = 0; axis < 3; ++axis)
	{
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < 24; ++i)
		{
			float a = i * 6.28318f / 24.0f;
			float c = (float)cos(a), s = (float)sin(a);
			if (axis == 0)
				glVertex3f(cx + c * rx, cy + s * ry, cz);
			else if (axis == 1)
				glVertex3f(cx + c * rx, cy, cz + s * rz);
			else
				glVertex3f(cx, cy + c * ry, cz + s * rz);
		}
		glEnd();
	}
}

// The client depth-sorts transparent geometry itself (draw flag 0x400000). Per-texel
// passes get the same result here without sorting: solid texels of a blended texture
// write depth like any opaque surface, and only its soft texels are blended over them.
enum DrawPass { PASS_OPAQUE, PASS_BLEND_SOLID, PASS_BLEND_SOFT, PASS_COUNT };

// CPetBody::SetLight's fallback: the direction the light travels, in game space.
static const WVector LIGHT_TRAVEL(0.0f, -0.70710678f, 0.70710678f);
static WVector g_eye;		// camera, in game space

#define GL_TEXTURE0_ARB 0x84C0
#define GL_TEXTURE1_ARB 0x84C1
typedef void (APIENTRY* ActiveTextureFn)(GLenum);
typedef void (APIENTRY* MultiTexCoord2fFn)(GLenum, GLfloat, GLfloat);
static ActiveTextureFn g_activeTexture;
static MultiTexCoord2fFn g_multiTexCoord2f;

// WPuppet::LoadPET: the first texture with '!' in its first three characters is the
// puppet's specular map, whatever the texture is called.
static int SpecularSlot(const PetFile& p)
{
	for (size_t i = 0; i < p.textures.size(); ++i)
		if (NameHasFlag(p.textures[i].Name(), "!"))
			return (int)i;
	return -1;
}

// WBone::CalcHilightCoord. The maps are brightest at (0, 0) and black at 0.75.
static void HilightCoord(const WVector& n, const WVector& pos, float* u, float* v)
{
	*u = *v = 0.75f;
	if (LIGHT_TRAVEL.Dot(n) >= 0.0f)
		return;
	WVector toPos = pos - g_eye;
	toPos.Normalize();
	WVector z = LIGHT_TRAVEL + toPos;
	z.Normalize();
	if (z.Dot(n) >= 0.0f)
		return;
	WVector x = fabs(z.y) > 0.999999f ? WVector(1.0f, 0.0f, 0.0f).Cross(z) : WVector(0.0f, 1.0f, 0.0f).Cross(z);
	x.Normalize();
	WVector y = z.Cross(x);
	y.Normalize();
	*u = (float)fabs(x.Dot(n));
	*v = (float)fabs(y.Dot(n));
}

static void DrawPart(const MeshPart& part, DrawPass pass)
{
	const PetFile& p = *part.pet;
	size_t tris = p.points.size() / 3;
	int current = -2;
	bool open = false;
	bool hilight = false;
	int specSlot = -1;
	GLuint specTex = 0;
	if (g_multiTexCoord2f && g_opts->lit && g_opts->shade == SHADE_TEXTURED)
	{
		specSlot = SpecularSlot(p);
		if (specSlot >= 0 && specSlot < (int)part.glTextures.size())
			specTex = part.glTextures[specSlot];
	}
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
			bool wanted;
			if (pass == PASS_OPAQUE)
				wanted = mode == BLEND_NONE || mode == BLEND_TEST;
			else if (pass == PASS_BLEND_SOLID)
				wanted = mode == BLEND_ALPHA;
			else
				wanted = mode == BLEND_ALPHA;
			if (!wanted)
				continue;

			GLuint tex = valid && slot < (int)part.glTextures.size() ? part.glTextures[slot] : 0;
			if (valid && g_opts->selectedFace >= 0 && g_opts->selectedFace < (int)p.faces.size())
			{
				const PetFaceAnim& fa = p.faces[g_opts->selectedFace];
				if (fa.group == p.textures[slot].group)
				{
					for (size_t k = 0; k < p.textures.size(); ++k)
						if (!_stricmp(p.textures[k].Name().c_str(), fa.File().c_str()) && k < part.glTextures.size())
							tex = part.glTextures[k];
				}
			}
			if (g_opts->shade == SHADE_TEXTURED && tex)
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
			}
			else
			{
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
			}
			if (mode == BLEND_TEST)
			{
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);
			}
			else if (pass == PASS_BLEND_SOLID)
			{
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GEQUAL, 0.95f);
			}
			else if (pass == PASS_BLEND_SOFT)
			{
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_LESS, 0.95f);
			}
			else
			{
				glDisable(GL_ALPHA_TEST);
			}
			// WBone::CalcLight: flag2 0x10 ('+') is full bright, flag2 4 (']') ambient only.
			if (g_opts->lit && g_opts->shade != SHADE_WIRE)
			{
				if (valid && NameHasFlag(p.textures[slot].Name(), "+]"))
					glDisable(GL_LIGHTING);
				else
					glEnable(GL_LIGHTING);
			}
			float alpha = part.isFx ? 1.0f : g_doc->modelAlpha;
			if (alpha < 1.0f)
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			u32 d = valid ? p.textures[slot].diffuse : 0xffffffff;
			if (g_opts->shade == SHADE_FLAT || !tex)
				glColor4f(0.72f, 0.72f, 0.74f, alpha);
			else
				glColor4f(((d >> 16) & 255) / 255.0f, ((d >> 8) & 255) / 255.0f, (d & 255) / 255.0f, alpha);
			// WDirect3D8::SetVtxMode: draw flag 0x40000 adds stage 1 to the lit texture.
			// WBone::ApplySpecularMap leaves out a mesh drawn with the map itself.
			hilight = specTex && slot != specSlot;
			if (specTex)
			{
				g_activeTexture(GL_TEXTURE1_ARB);
				if (hilight)
				{
					glBindTexture(GL_TEXTURE_2D, specTex);
					glEnable(GL_TEXTURE_2D);
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
				}
				else
					glDisable(GL_TEXTURE_2D);
				g_activeTexture(GL_TEXTURE0_ARB);
			}
			glBegin(GL_TRIANGLES);
			open = true;
		}
		if (!open)
			continue;
		if (!part.isFx && t < part.triBone.size() && part.triBone[t] >= 0 &&
			part.triBone[t] < (int)g_doc->boneHidden.size() && g_doc->boneHidden[part.triBone[t]])
			continue;
		if (!part.isFx && t < part.triOwnBone.size() && part.triOwnBone[t] >= 0 &&
			part.triOwnBone[t] < (int)part.ownHidden.size() && part.ownHidden[part.triOwnBone[t]])
			continue;
		for (int c = 0; c < 3; ++c)
		{
			size_t i = t * 3 + c;
			const PetTriPoint& pt = p.points[i];
			const WVector& n = part.skinnedNormal[i];
			const WVector& v = part.skinnedPos[part.cornerVertex[i]];
			glTexCoord2f(pt.u, pt.v);
			if (hilight)
			{
				float hu, hv;
				HilightCoord(n, v, &hu, &hv);
				g_multiTexCoord2f(GL_TEXTURE1_ARB, hu, hv);
			}
			glNormal3f(n.x, n.y, n.z);
			glVertex3f(v.x, v.y, v.z);
		}
	}
	if (open)
		glEnd();
	if (specTex)
	{
		g_activeTexture(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		g_activeTexture(GL_TEXTURE0_ARB);
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
}

// -- effects ----------------------------------------------------------------------

static void EnsureFxModel(MeshPart* part)
{
	if (part->glTextures.size() == part->pet->textures.size())
		return;
	ReloadPartTextures(*part);
}

static void DrawFx()
{
	if (!g_fx || !g_opts->effects)
		return;
	GLfloat mv[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv);
	WVector right(mv[0], mv[4], mv[8]), up(mv[1], mv[5], mv[9]);

	// *pet particles are ordinary lit geometry, in their rest pose.
	for (std::list<FxSpray*>::iterator si = g_fx->m_spray.begin(); si != g_fx->m_spray.end(); ++si)
	{
		FxSpray& spray = **si;
		MeshPart* model = (MeshPart*)spray.m_param.petModel;
		if (!model)
			continue;
		EnsureFxModel(model);
		for (std::list<FxParticle*>::iterator pi = spray.m_particle.begin(); pi != spray.m_particle.end(); ++pi)
		{
			FxParticle* p = *pi;
			WMatrix m = FxRotEuler(p->rot) * spray.m_rot;
			WVector at = (p->pos + p->bubble) * spray.GetBgRot(p) + spray.GetBgPos(p);
			float s = p->size;
			GLfloat gl[16] = { m.xx * s, m.yx * s, m.zx * s, 0, m.xy * s, m.yy * s, m.zy * s, 0, m.xz * s, m.yz * s, m.zz * s, 0, at.x, at.y, at.z, 1 };
			glPushMatrix();
			glMultMatrixf(gl);
			glEnable(GL_CULL_FACE);
			glEnable(GL_NORMALIZE);
			for (int pass = 0; pass < PASS_COUNT; ++pass)
				DrawPart(*model, (DrawPass)pass);
			glDisable(GL_CULL_FACE);
			glPopMatrix();
		}
	}

	FxDrawParticles(*g_fx, right, up);
}

static void DrawScene()
{
	Document& doc = *g_doc;
	RECT rc;
	GetClientRect(g_wnd, &rc);
	int w = rc.right > 1 ? rc.right : 1, h = rc.bottom > 1 ? rc.bottom : 1;
	glViewport(0, 0, w, h);
	glClearColor(0.42f, 0.45f, 0.50f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float zn = g_dist * 0.01f, zf = g_dist * 100.0f;
	if (g_preset == 0)
		gluPerspective(40.0, (double)w / h, zn, zf);
	else
	{
		double ext = g_dist * 0.4;
		glOrtho(-ext * w / h, ext * w / h, -ext, ext, -zf, zf);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -g_dist);
	glRotatef(g_pitch * 57.29578f, 1.0f, 0.0f, 0.0f);
	glRotatef(g_yaw * 57.29578f, 0.0f, 1.0f, 0.0f);
	// The game is left-handed, so Z is mirrored for display. Shipped triangles measure
	// counter-clockwise from outside in file coordinates; the mirror makes them clockwise.
	glScalef(1.0f, 1.0f, -1.0f);
	// The camera's back axis, read in game space because Z is already mirrored.
	GLfloat rot[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, rot);
	WVector back(rot[2], rot[6], rot[10]);
	g_eye = g_target + back * g_dist;
	WVector quake;
	int flash = 0;
	if (g_fx && g_opts->effects)
	{
		g_fx->m_camForward = back * -1.0f;
		g_fx->m_camPos = g_eye;
		g_fx->GetOutput(g_fx->m_camPos, &quake, &flash);
	}
	glTranslatef(-g_target.x - quake.x, -g_target.y - quake.y, -g_target.z - quake.z);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	if (g_opts->grid)
	{
		WVector lo, hi;
		doc.WorldBounds(&lo, &hi);
		float span = (hi - lo).Length();
		float step = 1.0f;
		while (step * 20.0f < span * 1.5f)
			step *= 2.0f;
		while (step * 5.0f > span && step > 0.01f)
			step *= 0.5f;
		float y = 0.0f;
		glBegin(GL_LINES);
		for (int i = -10; i <= 10; ++i)
		{
			if (i == 0)
				glColor3f(0.25f, 0.27f, 0.30f);
			else
				glColor3f(0.36f, 0.39f, 0.44f);
			glVertex3f(i * step, y, -10.0f * step);
			glVertex3f(i * step, y, 10.0f * step);
			glVertex3f(-10.0f * step, y, i * step);
			glVertex3f(10.0f * step, y, i * step);
		}
		glEnd();
	}

	if (g_opts->shade == SHADE_WIRE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	if (g_opts->lit && g_opts->shade != SHADE_WIRE)
	{
		// CPolySoup's defaults, from CPetBody::SetLight's direction away from a course: it
		// shines along (0, -1, 1), and characters look along -Z. A course's own sun is in its map.
		GLfloat ambient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat dir[4] = { -LIGHT_TRAVEL.x, -LIGHT_TRAVEL.y, -LIGHT_TRAVEL.z, 0.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_POSITION, dir);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_NORMALIZE);
	}
	// A faded model shows only its nearest surface, which the client gets by sorting.
	glDepthFunc(GL_LEQUAL);
	if (doc.modelAlpha < 1.0f && !doc.modelHidden)
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		for (int pass = PASS_OPAQUE; pass <= PASS_BLEND_SOLID; ++pass)
			for (size_t p = 0; p < doc.parts.size(); ++p)
				if (doc.parts[p]->visible)
					DrawPart(*doc.parts[p], (DrawPass)pass);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	for (int pass = 0; pass < PASS_COUNT; ++pass)
		for (size_t p = 0; p < doc.parts.size(); ++p)
			if (doc.parts[p]->visible && !doc.modelHidden)
				DrawPart(*doc.parts[p], (DrawPass)pass);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawFx();
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	if (g_opts->normals)
	{
		WVector lo, hi;
		doc.WorldBounds(&lo, &hi);
		float len = (hi - lo).Length() * 0.02f;
		glColor3f(0.3f, 0.9f, 1.0f);
		glBegin(GL_LINES);
		for (size_t p = 0; p < doc.parts.size(); ++p)
		{
			const MeshPart& part = *doc.parts[p];
			if (!part.visible)
				continue;
			for (size_t i = 0; i < part.cornerVertex.size(); ++i)
			{
				const WVector& v = part.skinnedPos[part.cornerVertex[i]];
				WVector e = v + part.skinnedNormal[i] * len;
				glVertex3f(v.x, v.y, v.z);
				glVertex3f(e.x, e.y, e.z);
			}
		}
		glEnd();
	}

	if (g_opts->boxes)
	{
		glDisable(GL_DEPTH_TEST);
		for (size_t i = 0; i < doc.pet.colls.size(); ++i)
		{
			const PetColl& c = doc.pet.colls[i];
			int bone = -1;
			for (size_t k = 0; k < doc.Skeleton().size(); ++k)
				if (!_stricmp(doc.Skeleton()[k].name.c_str(), c.bone.c_str()))
				{
					bone = (int)k;
					break;
				}
			if (bone < 0 || bone >= (int)doc.boneWorld.size())
				continue;
			const WMatrix& m = doc.boneWorld[bone];
			GLfloat gl[16] = { m.xx, m.yx, m.zx, 0, m.xy, m.yy, m.zy, 0, m.xz, m.yz, m.zz, 0, m.xm, m.ym, m.zm, 1 };
			glPushMatrix();
			glMultMatrixf(gl);
			bool selected = (int)i == g_opts->selectedBox;
			if (selected)
			{
				glColor3f(1.0f, 1.0f, 0.2f);
				glLineWidth(2.0f);
			}
			else if (c.visible)
				glColor3f(0.25f, 0.95f, 0.45f);
			else
				glColor3f(0.2f, 0.55f, 0.35f);
			if (c.shape == 0)
				DrawSphereLines(c.box);
			else
				DrawBoxLines(c.box);
			glLineWidth(1.0f);
			glPopMatrix();
		}
		glEnable(GL_DEPTH_TEST);
	}

	if (g_opts->skeleton && doc.HasSkeleton())
	{
		glDisable(GL_DEPTH_TEST);
		const std::vector<PetBone>& bones = doc.Skeleton();
		glBegin(GL_LINES);
		for (size_t i = 0; i < bones.size(); ++i)
		{
			int parent = bones[i].parent;
			if (parent < 0 || parent >= (int)doc.boneWorld.size())
				continue;
			if ((int)i == g_opts->selectedBone)
				glColor3f(1.0f, 1.0f, 0.2f);
			else
				glColor3f(1.0f, 0.55f, 0.15f);
			WVector a = doc.boneWorld[parent].Pivot(), b = doc.boneWorld[i].Pivot();
			glVertex3f(a.x, a.y, a.z);
			glVertex3f(b.x, b.y, b.z);
		}
		glEnd();
		glPointSize(3.0f);
		glBegin(GL_POINTS);
		for (size_t i = 0; i < bones.size(); ++i)
		{
			if ((int)i == g_opts->selectedBone)
				glColor3f(1.0f, 1.0f, 0.2f);
			else
				glColor3f(1.0f, 0.8f, 0.4f);
			WVector a = doc.boneWorld[i].Pivot();
			glVertex3f(a.x, a.y, a.z);
		}
		glEnd();
		glPointSize(1.0f);

		if (g_opts->selectedBone >= 0 && g_opts->selectedBone < (int)doc.boneWorld.size())
		{
			const WMatrix& m = doc.boneWorld[g_opts->selectedBone];
			WVector lo, hi;
			doc.WorldBounds(&lo, &hi);
			float len = (hi - lo).Length() * 0.06f;
			WVector o = m.Pivot();
			WVector x = o + m.Rotate(WVector(len, 0, 0));
			WVector y = o + m.Rotate(WVector(0, len, 0));
			WVector z = o + m.Rotate(WVector(0, 0, len));
			glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3f(o.x, o.y, o.z);
			glVertex3f(x.x, x.y, x.z);
			glColor3f(0, 1, 0);
			glVertex3f(o.x, o.y, o.z);
			glVertex3f(y.x, y.y, y.z);
			glColor3f(0, 0.3f, 1);
			glVertex3f(o.x, o.y, o.z);
			glVertex3f(z.x, z.y, z.z);
			glEnd();
		}
		glEnable(GL_DEPTH_TEST);
	}

	// Corner label
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	if (flash > 0)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(255, 255, 255, (GLubyte)flash);
		glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(w, 0);
		glVertex2i(w, h);
		glVertex2i(0, h);
		glEnd();
		glDisable(GL_BLEND);
	}
	static const char* presets[4] = { "Perspective", "Front", "Side", "Top" };
	static const char* shades[3] = { "Textured", "Flat", "Wireframe" };
	char label[128];
	sprintf_s(label, "%s | %s", presets[g_preset], shades[g_opts->shade]);
	glColor3f(0.92f, 0.92f, 0.86f);
	glRasterPos2i(8, h - 16);
	glListBase(g_fontBase);
	glCallLists((GLsizei)strlen(label), GL_UNSIGNED_BYTE, label);
}

static LRESULT CALLBACK ViewProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			g_dc = GetDC(hwnd);
			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 24;
			int pf = ChoosePixelFormat(g_dc, &pfd);
			SetPixelFormat(g_dc, pf, &pfd);
			g_rc = wglCreateContext(g_dc);
			wglMakeCurrent(g_dc, g_rc);
			// Every card of the time has this; without it a puppet simply has no highlights.
			g_activeTexture = (ActiveTextureFn)wglGetProcAddress("glActiveTextureARB");
			g_multiTexCoord2f = (MultiTexCoord2fFn)wglGetProcAddress("glMultiTexCoord2fARB");
			if (!g_activeTexture)
				g_multiTexCoord2f = 0;
			HFONT font = CreateFontW(13, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, L"Tahoma");
			SelectObject(g_dc, font);
			g_fontBase = glGenLists(256);
			wglUseFontBitmapsA(g_dc, 0, 256, g_fontBase);
			return 0;
		}
	case WM_ERASEBKGND:
		return 1;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			wglMakeCurrent(g_dc, g_rc);
			if (g_doc)
			{
				EnsureTextures();
				LoadNewPartTextures();
				UploadLiveTextures();
				DrawScene();
			}
			SwapBuffers(g_dc);
			EndPaint(hwnd, &ps);
			return 0;
		}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		SetCapture(hwnd);
		SetFocus(hwnd);
		g_lastX = GET_X_LPARAM(lp);
		g_lastY = GET_Y_LPARAM(lp);
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (!(wp & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)))
			ReleaseCapture();
		return 0;
	case WM_MOUSEMOVE:
		{
			int x = GET_X_LPARAM(lp), y = GET_Y_LPARAM(lp);
			int dx = x - g_lastX, dy = y - g_lastY;
			g_lastX = x;
			g_lastY = y;
			if (wp & MK_LBUTTON)
			{
				g_preset = 0;
				g_yaw += dx * 0.01f;
				g_pitch += dy * 0.01f;
				if (g_pitch > 1.55f) g_pitch = 1.55f;
				if (g_pitch < -1.55f) g_pitch = -1.55f;
				InvalidateRect(hwnd, 0, FALSE);
			}
			else if (wp & (MK_RBUTTON | MK_MBUTTON))
			{
				float k = g_dist * 0.0016f;
				float cy = (float)cos(g_yaw), sy = (float)sin(g_yaw);
				// Camera right and up in game space (Z is mirrored for display).
				g_target.x -= dx * k * cy;
				g_target.z += dx * k * sy;
				g_target.y += dy * k;
				InvalidateRect(hwnd, 0, FALSE);
			}
			return 0;
		}
	case WM_MOUSEWHEEL:
		g_dist *= GET_WHEEL_DELTA_WPARAM(wp) > 0 ? 0.87f : 1.15f;
		InvalidateRect(hwnd, 0, FALSE);
		return 0;
	case WM_DESTROY:
		wglMakeCurrent(0, 0);
		wglDeleteContext(g_rc);
		ReleaseDC(hwnd, g_dc);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

void ViewRegisterClass(HINSTANCE inst)
{
	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = ViewProc;
	wc.hInstance = inst;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = L"CaddieRig_View";
	RegisterClassW(&wc);
}

HWND ViewCreate(HWND parent, HINSTANCE inst, Document* doc, ViewOptions* opts)
{
	g_doc = doc;
	g_opts = opts;
	g_wnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"CaddieRig_View", L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 100, 100, parent, 0, inst, 0);
	return g_wnd;
}
