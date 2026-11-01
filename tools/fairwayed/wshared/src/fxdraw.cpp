// SPDX-License-Identifier: MIT
// fxdraw.cpp - see fxdraw.h. Blend modes and sprite cells follow the client.
#include "fxdraw.h"
#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include "texture.h"

static void SetColor(unsigned int argb)
{
	glColor4ub((GLubyte)(argb >> 16), (GLubyte)(argb >> 8), (GLubyte)argb, (GLubyte)(argb >> 24));
}

// wd3d8.cpp, the blend states behind the 0x1800000 bits of a draw flag.
static void SetFxBlend(unsigned int mode)
{
	switch (mode)
	{
	case 0x800000: glBlendFunc(GL_SRC_ALPHA, GL_ONE); break;
	case 0x1000000: glBlendFunc(GL_ZERO, GL_SRC_COLOR); break;
	case 0x1800000: glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR); break;
	default: glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
	}
}

static bool BindSprite(FxSprite* s)
{
	if (!s->tried)
	{
		s->tried = true;
		if (!s->path.empty())
			s->texture = LoadGLTexture(s->path, &s->texW, &s->texH);
	}
	if (!s->texture)
		return false;
	glBindTexture(GL_TEXTURE_2D, s->texture);
	return true;
}

static bool SpriteCell(const FxSprite* s, int frame, float* u0, float* v0, float* u1, float* v1)
{
	if (s->cellW <= 0 || s->cellH <= 0 || s->texW <= 0 || s->texH <= 0)
		return false;
	int nx = s->texW / s->cellW, ny = s->texH / s->cellH;
	if (nx <= 0 || ny <= 0 || frame < 0 || frame >= nx * ny)
		return false;
	float ux = (float)s->cellW / s->texW, uy = (float)s->cellH / s->texH;
	*u0 = (frame % nx) * ux;
	*v0 = (frame / nx) * uy;
	*u1 = *u0 + ux;
	*v1 = *v0 + uy;
	return true;
}

static void Vtx(const WVector& v)
{
	glVertex3f(v.x, v.y, v.z);
}

static void DrawBillboard(const FxSprite* s, int frame, const WVector& pos, float w, float h, float angle, const WVector& right, const WVector& up)
{
	float u0, v0, u1, v1;
	if (!SpriteCell(s, frame, &u0, &v0, &u1, &v1))
		return;
	float c = (float)cos(angle), sn = (float)sin(angle);
	float hw = w * 0.5f, hh = h * 0.5f;
	static const float sx[4] = { -1, 1, 1, -1 }, sy[4] = { 1, 1, -1, -1 };
	const float us[4] = { u0, u1, u1, u0 }, vs[4] = { v0, v0, v1, v1 };
	glBegin(GL_QUADS);
	for (int i = 0; i < 4; ++i)
	{
		float x = sx[i] * hw, y = sy[i] * hh;
		float rx = x * c - y * sn, ry = x * sn + y * c;
		glTexCoord2f(us[i], vs[i]);
		Vtx(pos + right * rx + up * ry);
	}
	glEnd();
}

static void DrawOriented(const FxSprite* s, int frame, const WVector& pos, float w, float h, const WMatrix& rot)
{
	float u0, v0, u1, v1;
	if (!SpriteCell(s, frame, &u0, &v0, &u1, &v1))
		return;
	float hw = w * 0.5f, hh = h * 0.5f;
	glBegin(GL_QUADS);
	glTexCoord2f(u0, v0); Vtx(pos + rot.Rotate(WVector(-hw, hh, 0)));
	glTexCoord2f(u1, v0); Vtx(pos + rot.Rotate(WVector(hw, hh, 0)));
	glTexCoord2f(u1, v1); Vtx(pos + rot.Rotate(WVector(hw, -hh, 0)));
	glTexCoord2f(u0, v1); Vtx(pos + rot.Rotate(WVector(-hw, -hh, 0)));
	glEnd();
}

static unsigned int TailColor(const FxSpray& spray, const FxParticle* p, int n, int len, bool clampAlpha)
{
	const unsigned char* t = spray.m_param.tailColor;
	unsigned int out = 0;
	for (int i = 0; i < 4; ++i)
	{
		int shift = 24 - i * 8;
		int own = (p->color >> shift) & 255;
		int v = t[i] * (len - n) / len + own * n / len;
		if (i == 0 && clampAlpha && v > own)
			v = own;
		out |= (unsigned int)(v & 255) << shift;
	}
	return out;
}

static void DrawTails(const FxSpray& spray, const FxParticle* p, const WVector& pos, const WVector& right, const WVector& up)
{
	const FxSprayParam& m = spray.m_param;
	if (!p->tail.empty())
	{
		int len = (int)p->tail.size();
		if (m.sprite && m.sprite->texture)
		{
			SetFxBlend(m.blend[1]);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, m.sprite->texture);
			float h = p->size, w = p->ratio * p->size;
			for (int n = len - 1; n >= 0; --n)
			{
				unsigned int c = spray.ParticleColor(p->tail[n].pos, p, TailColor(spray, p, n, len, false));
				if (!(c & 0xff000000))
					continue;
				SetColor(c);
				DrawBillboard(m.sprite, p->tail[n].frame, p->tail[n].pos, w, h, p->rot.x, right, up);
			}
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINE_STRIP);
			SetColor(spray.ParticleColor(pos, p, p->color));
			Vtx(pos);
			for (int n = len - 1; n >= 0; --n)
			{
				SetColor(spray.ParticleColor(p->tail[n].pos, p, TailColor(spray, p, n, len, false)));
				Vtx(p->tail[n].pos);
			}
			glEnd();
		}
	}
	if (!p->bandana.empty() && m.tailTexture && BindSprite(m.tailTexture))
	{
		int len = (int)p->bandana.size();
		float total = 0.0f;
		for (int i = 1; i < len; ++i)
			total += p->bandana[i].length;
		const FxBand& last = p->bandana.back();
		float length = (pos - (last.pos[0] + last.pos[1]) * 0.5f).Length();
		float sum = length + total;
		WVector half = (last.pos[0] - last.pos[1]) * 0.5f;

		SetFxBlend(m.blend[1]);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUAD_STRIP);
		float u = 0.0f;
		SetColor(spray.ParticleColor(pos, p, p->color));
		glTexCoord2f(u, 0.0f); Vtx(pos + half);
		glTexCoord2f(u, 1.0f); Vtx(pos - half);
		float run = length;
		for (int n = len - 1; n >= 0; --n)
		{
			const FxBand& b = p->bandana[n];
			u = !m.bMonolith || sum == 0.0f ? (float)((len - n) & 1) : run / sum;
			SetColor(spray.ParticleColor(b.pos[0], p, TailColor(spray, p, n, len, true)));
			glTexCoord2f(u, 0.0f); Vtx(b.pos[0]);
			glTexCoord2f(u, 1.0f); Vtx(b.pos[1]);
			run += b.length;
		}
		glEnd();
	}
}

void FxDrawParticles(FxBox& box, const WVector& right, const WVector& up)
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDisable(GL_ALPHA_TEST);
	for (std::list<FxSpray*>::iterator si = box.m_spray.begin(); si != box.m_spray.end(); ++si)
	{
		FxSpray& spray = **si;
		const FxSprayParam& m = spray.m_param;
		if (m.petModel)
			continue;
		bool textured = m.sprite && BindSprite(m.sprite);
		for (std::list<FxParticle*>::iterator pi = spray.m_particle.begin(); pi != spray.m_particle.end(); ++pi)
		{
			FxParticle* p = *pi;
			WVector pos = (p->pos + p->bubble) * spray.GetBgRot(p) + spray.GetBgPos(p);
			unsigned int c = spray.ParticleColor(pos, p, p->color);
			if (c & 0xff000000)
			{
				SetFxBlend(m.blend[0]);
				SetColor(c);
				if (textured)
				{
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, m.sprite->texture);
					float h = p->size, w = p->ratio * p->size;
					WVector at = pos;
					if (m.camOffset != 0.0f)
					{
						// CFxSpray::DrawSprite pulls the sprite toward the camera.
						WVector dir = pos - box.m_camPos;
						dir.Normalize();
						at = pos - dir * m.camOffset;
					}
					if (m.bBillboard)
						DrawBillboard(m.sprite, p->frame, at, w, h, p->rot.x, right, up);
					else
						DrawOriented(m.sprite, p->frame, at, w, h, FxRotEuler(p->rot) * spray.GetBgRot(p));
				}
				else if (!m.sprite)
				{
					glDisable(GL_TEXTURE_2D);
					WVector dir = p->vel;
					dir.Normalize();
					glBegin(GL_LINES);
					Vtx(pos);
					Vtx(pos + dir * 0.03f);
					glEnd();
				}
			}
			DrawTails(spray, p, pos, right, up);
		}
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}
