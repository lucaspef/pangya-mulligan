// SPDX-License-Identifier: MIT
// editor.h - what the viewports, panels and main window share.
#ifndef EDITOR_H
#define EDITOR_H

#include <windows.h>
#include <string>
#include <vector>
#include "course.h"

enum Tool
{
	TOOL_SELECT,		// and move
	TOOL_ROTATE,
	TOOL_SCALE,
	TOOL_ADD_PUPPET,
	TOOL_ADD_POINT,
	TOOL_ADD_CAMERA,
	TOOL_ADD_AREA,
	TOOL_NODE,
	TOOL_COUNT
};

enum PaneShade { SHADE_WIRE, SHADE_FLAT, SHADE_TEXTURED, SHADE_SURFACE };

struct Editor
{
	Course course;
	std::vector<ElementRef> sel;
	Tool tool;
	float grid;
	bool snap;
	bool snapGround;		// moved things stay on the base model
	bool showPuppets, showPoints, showCameras, showAreas, showNodes, showLabels;
	bool showShadow, showFog, showSky;
	bool showFx;			// area effects in the 3D pane
	bool showScripts;		// what area and node scripts do
	int activeNode;			// node list the Node tool adds to, -1 for none
	std::string addPuppet;	// CP949 file name the Puppet tool places
	std::string addPoint;	// CP949 name for the next WPoint; empty asks for "WPoint"
	std::string addPointClass;
	std::string addAreaClass;
	int surfaceHighlight;	// class index, -1 for none
	bool korean;
	HWND main;
	HINSTANCE inst;

	Editor();
	bool IsSelected(const ElementRef& r) const;
};

extern Editor g_ed;

inline const wchar_t* T(const wchar_t* ko, const wchar_t* en) { return g_ed.korean ? ko : en; }

// main.cpp
void EdLog(const std::string& utf8);
void EdFlush(const CourseLog& log);
void EdSelectionChanged();		// redraws, refreshes the property grid and tree selection
void EdSceneChanged();			// elements added, removed or renamed
void EdValuesChanged();			// a moved or edited value; no structure change
void EdStatus(int part, const std::wstring& text);
void EdCommand(int id);
void EdRunConsole(const std::string& line);
void EdSetTool(Tool t);
void EdDeleteSelection();
void EdDuplicateSelection();
int EdAddElement(const WepElement& e, const std::string& label);
bool EdPrompt(const wchar_t* title, const wchar_t* label, std::wstring* value);

// viewport.cpp
void ViewsRegister(HINSTANCE inst);
void ViewsCreate(HWND parent);
void ViewsLayout(int x, int y, int w, int h);
void ViewsRedraw();
void ViewsFrameAll();
void ViewsFrameSelection();
void ViewsSetActiveShade(PaneShade shade);
PaneShade ViewsActiveShade();
void ViewsMaximizeActive();
void ViewsLookThrough(int cameraElement);	// -1 returns to free flight
void ViewsSetCamera(const WVector& eye, float yawDegrees, float pitchDegrees);
void ViewsFlyThrough(bool on);
bool ViewsFlying();
void ViewsTick();
HWND ViewsPreviewCreate(HWND parent);
void ViewsPreviewSet(const std::string& cp949Name);
void ViewsCaptureInto(HDC dc, int originX, int originY);	// window origin on screen
void ViewsStepEffects(float ms);

// panels.cpp
void PanelsCreate(HWND parent);
void PanelsLayout(int x, int y, int w, int h);
void PanelsRefreshScene();
void PanelsRefreshProps();
void PanelsSyncTreeSelection();
void PanelsRefreshBrowser();
void PanelsRefreshSurfaces();
void PanelsRefreshHole();
void PanelsLanguage();
void PanelsSwitchTab(int tab);
bool PanelsNotify(NMHDR* hdr);
bool PanelsCommand(int id, int code);

std::wstring ElementLabel(const WepElement& e, int index);

#endif // EDITOR_H
