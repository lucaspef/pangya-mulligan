// SPDX-License-Identifier: MIT
// areafx.cpp - plays WFxArea effects in the 3D pane.
//
// WPolySoup::Load attaches each type 6 area's file with the attacher's matrix at the box
// centre, rotated by the script. fxsim simulates it and fxdraw draws it. An area becomes one "bone" of the world matrices fxsim reads, so
// dragging or turning it moves its effect without restarting it.
#include "scene.h"
#include <GL/gl.h>
#include "areascript.h"
#include "fxdraw.h"

static CourseFxHost g_host(&g_ed.course);
static FxBox g_fx(&g_host);
static std::vector<WMatrix> g_world;
static std::vector<std::string> g_playing;
static std::string g_playingCourse;
static size_t g_problemsShown;

void AreaFxReset()
{
	g_fx.Clear();
	g_fx.m_problems.clear();
	g_problemsShown = 0;
	g_playing.clear();
	g_world.clear();
	g_playingCourse.clear();
}

void AreaFxRestart()
{
	g_fx.ClearAll(false);
	g_playing.clear();
	g_playingCourse.clear();
}

int AreaFxParticles()
{
	return g_fx.ParticleCount();
}

// Which areas play, in order. A change restarts everything; moving one doesn't.
static void CollectAreas(std::vector<std::string>* files, std::vector<WMatrix>* world)
{
	const std::vector<WepElement>& els = g_ed.course.wep.elements;
	for (size_t i = 0; i < els.size() && files->size() < 256; ++i)	// m_bgFx[256]
	{
		if (!els[i].IsArea() || AreaTypeOfClass(els[i].Class()) != 6)
			continue;
		AreaScript s = ParseAreaScript(els[i]);
		if (s.fxFile.empty() || s.fxSpecial)
			continue;
		WVector lo, hi;
		els[i].GetBox(&lo, &hi);
		WMatrix m = FxRotEuler(s.fxRotation);
		m.SetPivot((lo + hi) * 0.5f);
		files->push_back(s.fxFile);
		world->push_back(m);
	}
}

void AreaFxTick(float elapsedMs, const WVector& eye, const WVector& forward)
{
	if (!g_ed.showFx)
		return;
	std::vector<std::string> files;
	std::vector<WMatrix> world;
	CollectAreas(&files, &world);
	g_world.swap(world);
	if (files != g_playing || g_playingCourse != g_ed.course.path)
	{
		g_fx.ClearAll(false);
		g_playing = files;
		g_playingCourse = g_ed.course.path;
		for (size_t i = 0; i < files.size(); ++i)
		{
			FxAttacher a;
			a.count = 1;
			a.bone[0] = (int)i;
			g_fx.Attach(a, files[i], false);
		}
	}
	g_fx.m_camPos = eye;
	g_fx.m_camForward = forward;
	if (!g_fx.Finished())
		g_fx.Process(elapsedMs, g_world);
	for (; g_problemsShown < g_fx.m_problems.size(); ++g_problemsShown)
		EdLog("! Effect could not use: " + Cp949ToUtf8(g_fx.m_problems[g_problemsShown]));
}

void AreaFxDraw(const SceneView& v)
{
	if (!g_ed.showFx)
		return;
	const WVector& right = v.right;
	const WVector& up = v.up;

	// *pet particles are ordinary lit geometry, in their rest pose.
	for (std::list<FxSpray*>::iterator si = g_fx.m_spray.begin(); si != g_fx.m_spray.end(); ++si)
	{
		FxSpray& spray = **si;
		PuppetAsset* model = (PuppetAsset*)spray.m_param.petModel;
		if (!model)
			continue;
		for (std::list<FxParticle*>::iterator pi = spray.m_particle.begin(); pi != spray.m_particle.end(); ++pi)
		{
			FxParticle* p = *pi;
			WMatrix m = FxRotEuler(p->rot) * spray.m_rot;
			WVector at = (p->pos + p->bubble) * spray.GetBgRot(p) + spray.GetBgPos(p);
			float s = p->size;
			GLfloat gl[16] = { m.xx * s, m.yx * s, m.zx * s, 0, m.xy * s, m.yy * s, m.zy * s, 0, m.xz * s, m.yz * s, m.zz * s, 0, at.x, at.y, at.z, 1 };
			glPushMatrix();
			glMultMatrixf(gl);
			SceneCallPuppet(model);
			glPopMatrix();
		}
	}

	FxDrawParticles(g_fx, right, up);
	glColor4f(1, 1, 1, 1);
}
