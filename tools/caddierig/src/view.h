// SPDX-License-Identifier: MIT
// view.h - the OpenGL 1.1 viewport window.
#ifndef VIEW_H
#define VIEW_H

#include <windows.h>
#include "document.h"
#include "fxsim.h"

enum ViewShade { SHADE_TEXTURED, SHADE_FLAT, SHADE_WIRE };

struct ViewOptions
{
	ViewShade shade;
	bool skeleton;
	bool boxes;
	bool normals;
	bool grid;
	bool lit;
	int selectedBone;
	int selectedBox;
	int selectedFace;	// face animation being previewed, -1 for none
	bool effects;
};

void ViewRegisterClass(HINSTANCE inst);
HWND ViewCreate(HWND parent, HINSTANCE inst, Document* doc, ViewOptions* opts);
void ViewFrameAll();					// aims the camera at the model
void ViewSetPreset(int preset);			// 0 persp, 1 front, 2 side, 3 top
void ViewRefreshTextures();
void ViewSetFx(FxBox* box);
// *ptex: paint a part image into a live texture. Target is a FANM name, or on a
// character always its face. False with a reason when it cannot be done.
bool ViewApplyTexPart(const std::string& target, const std::string& image, std::string* why);
void ViewResetTexParts();
void ViewForgetPart(MeshPart* part);		// before the document destroys it
void ViewSetPartVariant(MeshPart* part, const std::vector<std::string>& paths);

#endif // VIEW_H
