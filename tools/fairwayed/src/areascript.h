// SPDX-License-Identifier: MIT
// areascript.h - what the scripts in area and node names mean to the client.
//
// An area's name is its script. WPolySoup::Load, CNPC::SetNPC, CWind::AddWindArea,
// CPool::SetArea and CGolfRule read them with WSliceStr, so this does the same: find the key
// anywhere in the text, then read what follows. A misspelt key is silently ignored there
// too, which is what ScriptProblem reports.
#ifndef AREASCRIPT_H
#define AREASCRIPT_H

#include <string>
#include <vector>
#include "wep.h"

enum AreaKind
{
	AREA_EVENT,			// type 0, nothing the client reads
	AREA_EXTRA,			// type 0, *extra: where a chip-in bonus or short game starts
	AREA_SHORTGAME,		// type 0, *extra_shortgame
	AREA_CAMERA,		// type 0, *cam: the ball inside switches to an installed camera
	AREA_NPC,			// type 1
	AREA_LAKE,			// type 2
	AREA_WIND,			// type 4
	AREA_SOUND,			// type 5
	AREA_FX,			// type 6
	AREA_GREEN,			// type 7: in the client, in no shipped hole
	AREA_UNKNOWN		// 3 and 9: in the data, read by nothing in this client
};

struct ScriptRow { std::string label, value; };

struct AreaScript
{
	AreaKind kind;
	std::vector<ScriptRow> rows;		// UTF-8
	std::vector<std::string> problems;	// UTF-8
	std::string summary;				// a short line for the viewport, UTF-8

	std::string fxFile;					// CP949
	WVector fxRotation;					// radians
	bool fxSpecial;						// silviacannon: CWind plays it, not the area
	std::string cameraName;				// CP949
	std::string petName;				// CP949
	float windIntensity;
	WVector windDirection;
	float soundDistance;
};

AreaScript ParseAreaScript(const WepElement& area);

// A node's name is its script too. Empty kind means the name asks for nothing special.
struct NodeScript
{
	std::string kind;					// UTF-8
	std::string petName;				// CP949, what walks it
	std::vector<ScriptRow> rows;
};

NodeScript ParseNodeScript(const std::string& cp949Name);

int AreaTypeOfClass(const std::string& cls);
std::string AreaClassOfType(int type);
WepElement MakeAreaElement(const std::string& cls);	// with a working script for its class

#endif // AREASCRIPT_H
