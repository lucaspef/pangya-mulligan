// SPDX-License-Identifier: MIT
// scene.h - fixed-function drawing of a course. OpenGL 1.1 display lists throughout.
#ifndef SCENE_H
#define SCENE_H

#include "editor.h"

struct SceneView
{
	WVector eye, forward, right, up;
	bool ortho;
	float halfHeight;	// ortho: world units from the centre to the top edge
	float fov;			// perspective, vertical degrees
	int width, height;
	PaneShade shade;
};

void SceneDraw(const SceneView& v);
void SceneDrawPreview(PuppetAsset* a, float yaw, int width, int height);
void SceneInvalidateBase();			// baked colours or surface classes changed
void SceneDrawOverlayText(const SceneView& v, const wchar_t* text, int x, int y);
void SceneCallPuppet(PuppetAsset* a);	// draws it at the current matrix

// areafx.cpp
void AreaFxReset();						// before the assets it holds go away
void AreaFxRestart();
int AreaFxParticles();
void AreaFxTick(float elapsedMs, const WVector& eye, const WVector& forward);
void AreaFxDraw(const SceneView& v);

// Screen position of a world point. False when behind the camera.
bool SceneProject(const SceneView& v, const WVector& p, float* x, float* y);
// A ray from the eye through a pixel, for picking.
void SceneRay(const SceneView& v, int x, int y, WVector* origin, WVector* dir);

#endif // SCENE_H
