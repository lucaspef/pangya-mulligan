// SPDX-License-Identifier: MIT
// areascript.cpp - see areascript.h.
#include "areascript.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fxsim.h"
#include "wcommon.h"

// -- WSliceStr ----------------------------------------------------------------

static bool SliceFloat(const std::string& s, const char* key, float* out)
{
	size_t p = s.find(key);
	if (p == std::string::npos)
		return false;
	return sscanf(s.c_str() + p + strlen(key), "%f", out) == 1;
}

static bool SliceInt(const std::string& s, const char* key, int* out)
{
	size_t p = s.find(key);
	if (p == std::string::npos)
		return false;
	return sscanf(s.c_str() + p + strlen(key), "%d", out) == 1;
}

static bool SliceWord(const std::string& s, const char* key, std::string* out)
{
	size_t p = s.find(key);
	if (p == std::string::npos)
		return false;
	char word[128] = "";
	if (sscanf(s.c_str() + p + strlen(key), "%127s", word) != 1)
		return false;
	*out = word;
	return true;
}

static void Row(std::vector<ScriptRow>* rows, const char* label, const std::string& value)
{
	ScriptRow r;
	r.label = label;
	r.value = value;
	rows->push_back(r);
}

static std::string Num(float f)
{
	char buf[32];
	sprintf_s(buf, "%g", f);
	return buf;
}

// -- classes ------------------------------------------------------------------

int AreaTypeOfClass(const std::string& cls)
{
	if (cls == "WEventArea") return 0;
	if (cls == "WNPC") return 1;
	if (cls == "WLake") return 2;
	if (cls == "WWindArea") return 4;
	if (cls == "WSoundArea") return 5;
	if (cls == "WFxArea") return 6;
	if (cls == "WGreenArea") return 7;
	if (cls.compare(0, 5, "WArea") == 0) return atoi(cls.c_str() + 5);
	return 0;
}

std::string AreaClassOfType(int type)
{
	switch (type)
	{
	case 0: return "WEventArea";
	case 1: return "WNPC";
	case 2: return "WLake";
	case 4: return "WWindArea";
	case 5: return "WSoundArea";
	case 6: return "WFxArea";
	case 7: return "WGreenArea";
	}
	char buf[32];
	sprintf_s(buf, "WArea%d", type);	// 3 and 9 are in the data, with no known name
	return buf;
}

WepElement MakeAreaElement(const std::string& cls)
{
	WepElement e = WepMakeElement("AREA", cls.c_str());
	if (cls == "WFxArea")
		e.Set("Name", "*fx cl_default.spr 0.000 0.000 0.000");
	else if (cls == "WWindArea")
		e.Set("Name", "3.0 0 0 normal");
	else if (cls == "WNPC")
		e.Set("Name", "*type 3 *pet NPC_Mole.pet *num 1");
	else if (cls == "WLake")
		e.Set("Name", "*type 0");
	else if (cls == "WSoundArea")
		e.Set("Name", "sound.wav 60 -1");
	return e;
}

// -- NPC tables ---------------------------------------------------------------

// The class each *type builds in CNPC::SetNPC. 7, 10 and anything else build nothing.
static const char* CreatureName(int type)
{
	switch (type)
	{
	case 0: return "CGull";
	case 1: return "CButterfly";
	case 2: return "CDuck";
	case 3: return "CMole";
	case 4: return "CDolphin";
	case 5: return "CCrab";
	case 6: return "CSparrow";
	case 8: return "CFighter";
	case 9: return "CDisposable";
	case 13: return "ntPenguin_Sailor";
	case 14: return "ntPenguin_Duck";
	case 16: return "ntNpc_BlackSparrow";
	case 18: return "ntNpc_PolaPenguin1";
	case 19: return "ntNpc_PolaPenguin2";
	case 21: return "CNPC_BIRD";
	case 22: return "CNPC_PARROT";
	case 31: return "ntNpc_WizCityArea";
	}
	return 0;
}

// -- areas --------------------------------------------------------------------

static void ParseEvent(const std::string& name, AreaScript* s)
{
	if (name.find("*cam") != std::string::npos)
	{
		s->kind = AREA_CAMERA;
		SliceWord(name, "*cam", &s->cameraName);
		Row(&s->rows, "Use", "ball inside switches to an installed camera");
		Row(&s->rows, "Camera", Cp949ToUtf8(s->cameraName));
		s->summary = "camera: " + Cp949ToUtf8(s->cameraName);
		return;
	}
	if (name.find("*extra_shortgame") != std::string::npos)
	{
		s->kind = AREA_SHORTGAME;
		Row(&s->rows, "Use", "short game start box; the tee is picked at random inside");
		s->summary = "short game start";
		return;
	}
	if (name.find("*extra") != std::string::npos)
	{
		s->kind = AREA_EXTRA;
		Row(&s->rows, "Use", "approach start box; the tee is picked at random inside (LoadMapCheckData)");
		s->summary = "approach start";
		return;
	}
	if (name.find("extra") != std::string::npos)
		s->problems.push_back("names \"extra\" without the exact \"*extra\" the client looks for");
	s->summary = Cp949ToUtf8(name);
}

static void ParseNpc(const std::string& name, AreaScript* s)
{
	s->kind = AREA_NPC;
	int type = -1;
	bool hasType = SliceInt(name, "*type", &type);
	SliceWord(name, "*pet", &s->petName);
	int num = 0;
	bool hasNum = SliceInt(name, "*num", &num);
	const char* creature = CreatureName(type);
	char typeText[32], countText[32];
	sprintf_s(typeText, "%d", type);
	sprintf_s(countText, "%d", hasNum ? num : 1);
	Row(&s->rows, "Creature", std::string(creature ? creature : "none") + " (*type " + typeText + ")");
	Row(&s->rows, "Pet", Cp949ToUtf8(s->petName));
	Row(&s->rows, "Count", countText);
	float f;
	if (SliceFloat(name, "*vel", &f))
		Row(&s->rows, "Speed", Num(f));
	if (SliceFloat(name, "*s", &f) && name.find("*s ") != std::string::npos)
		Row(&s->rows, "Scale", Num(f));
	if (!hasType)
		s->problems.push_back("has no *type, which CNPC::SetNPC reads without a default");
	else if (!creature)
		s->problems.push_back("*type " + std::string(typeText) + " builds no creature in the decompiled client");
	if (type == 16 && s->petName.find("BTsparrow") == std::string::npos)
		s->problems.push_back("*type 16 ignores *pet and always uses NPC_BTsparrow.pet");
	if (type == 6 && !hasNum)
		s->problems.push_back("*type 6 without *num builds no creature");
	s->summary = Cp949ToUtf8(s->petName) + " x" + countText;
}

static void ParseLake(const std::string& name, AreaScript* s)
{
	s->kind = AREA_LAKE;
	int type = 0;
	bool hasType = SliceInt(name, "*type", &type);
	bool noRipple = name.find("*noripple") != std::string::npos;
	if (noRipple)
		type = 1;	// CPool::SetArea overrides *type
	char buf[32];
	sprintf_s(buf, "%d", type);
	Row(&s->rows, "Water type", buf);
	Row(&s->rows, "Ripples", noRipple ? "off" : "on");
	Row(&s->rows, "Use", "water surface at the top of the box; the ball splashes in it");
	if (!hasType && name.find("type") != std::string::npos)
		s->problems.push_back("spells *type so the client doesn't see it; it plays as type 0");
	s->summary = noRipple ? "water, no ripples" : std::string("water type ") + buf;
}

static void ParseWind(const std::string& name, AreaScript* s)
{
	s->kind = AREA_WIND;
	const char* text = name.c_str();
	char op = '+';
	const char* opName = "adds to the hole's wind";
	switch (*text)
	{
	case '*': op = '*'; opName = "multiplies the hole's wind"; ++text; break;
	case '+': ++text; break;
	case '-': op = '-'; opName = "subtracts from the hole's wind"; ++text; break;
	case '=': op = '='; opName = "replaces the hole's wind"; ++text; break;
	}
	float pitch = 0.0f, yaw = 0.0f;
	char difficulty[8] = "hard";	// the client's default
	sscanf(text, "%f %f %f %7s", &s->windIntensity, &pitch, &yaw, difficulty);
	// AddWindArea: (RotMat(yaw, x) * RotMat(pitch, y)).za, so "pitch" is the heading.
	const float D = 3.14159265f / 180.0f;
	s->windDirection = FxRotEuler(WVector(yaw * D, pitch * D, 0.0f)).Rotate(WVector(0, 0, 1));
	Row(&s->rows, "Effect", opName);
	Row(&s->rows, "Strength", Num(s->windIntensity));
	Row(&s->rows, "Heading", Num(pitch));
	Row(&s->rows, "Tilt", Num(yaw));
	const char* active = !_stricmp(difficulty, "normal") ? "DiffFlag 0, 1, 2" : !_stricmp(difficulty, "hard") ? "DiffFlag 2" : !_stricmp(difficulty, "easy") ? "DiffFlag 1" : "DiffFlag 1, 2";
	Row(&s->rows, "Active on", std::string(difficulty) + ": " + active);
	char buf[64];
	sprintf_s(buf, "wind %c%g", op, s->windIntensity);
	s->summary = buf;
}

static void ParseSound(const std::string& name, AreaScript* s)
{
	s->kind = AREA_SOUND;
	char file[64] = "";
	float interval = -1.0f;
	s->soundDistance = 60.0f;
	sscanf(name.c_str(), "%63s %f %f", file, &s->soundDistance, &interval);
	Row(&s->rows, "Sound", Cp949ToUtf8(file));
	Row(&s->rows, "Reach", Num(s->soundDistance));
	Row(&s->rows, "Repeat", interval < 0.0f ? "loop" : Num(interval));
	s->summary = Cp949ToUtf8(file);
}

static void ParseFx(const std::string& name, AreaScript* s)
{
	s->kind = AREA_FX;
	size_t p = name.find("*fx");
	char file[64] = "";
	float x = 0, y = 0, z = 0;
	if (p != std::string::npos)
		sscanf(name.c_str() + p + 3, "%63s %f %f %f", file, &x, &y, &z);
	s->fxFile = file;
	s->fxRotation = WVector(x, y, z);
	if (!_stricmp(file, "silviacannon.seq"))
	{
		s->fxSpecial = true;
		Row(&s->rows, "Use", "CWind plays this when its special wind fires; its Y rotation is the wind's direction");
	}
	Row(&s->rows, "Effect", Cp949ToUtf8(file));
	Row(&s->rows, "Rotation", Num(x) + " " + Num(y) + " " + Num(z));
	Row(&s->rows, "Origin", "centre of the box");
	if (p == std::string::npos || !file[0])
		s->problems.push_back("has no \"*fx file\" to play");
	else
	{
		const char* dot = strrchr(file, '.');
		if (!dot || (_stricmp(dot, ".spr") && _stricmp(dot, ".seq")))
			s->problems.push_back("names a file that isn't .spr or .seq");
	}
	s->summary = Cp949ToUtf8(file);
}

AreaScript ParseAreaScript(const WepElement& area)
{
	AreaScript s;
	s.kind = AREA_EVENT;
	s.windIntensity = 0.0f;
	s.soundDistance = 0.0f;
	s.fxSpecial = false;
	std::string name = area.Get("Name");
	switch (AreaTypeOfClass(area.Class()))
	{
	case 0: ParseEvent(name, &s); break;
	case 1: ParseNpc(name, &s); break;
	case 2: ParseLake(name, &s); break;
	case 4: ParseWind(name, &s); break;
	case 5: ParseSound(name, &s); break;
	case 6: ParseFx(name, &s); break;
	case 7:
		s.kind = AREA_GREEN;
		Row(&s.rows, "Use", "the green, for CQuadTree::ResetHoleData");
		s.summary = "green";
		break;
	default:
		s.kind = AREA_UNKNOWN;
		Row(&s.rows, "Use", "nothing in the decompiled client reads this type");
		s.summary = Cp949ToUtf8(name);
		break;
	}
	return s;
}

// -- nodes --------------------------------------------------------------------

// CNPC::SetNPC(node), in its order: earlier keywords win over later ones.
NodeScript ParseNodeScript(const std::string& name)
{
	NodeScript s;
	float f;
	bool npc = name.find("npc") != std::string::npos;
	if (name.find("outline") != std::string::npos)
		s.kind = "Course boundary (IsOutOfBound)";
	else if (name.find("ib") != std::string::npos)
		s.kind = "Inner boundary";
	else if (name.find("ob") != std::string::npos)
		s.kind = "Outer boundary";
	else if (name.find("auto") != std::string::npos)
		s.kind = "Fly-through path";
	else if (name.find("beach") != std::string::npos)
		s.kind = "Beach edge";
	else if (name.find("riverex") != std::string::npos)
		s.kind = "River, extended";
	else if (name.find("waterfall") != std::string::npos)
		s.kind = "Waterfall";
	else if (name.find("river") != std::string::npos)
		s.kind = "River";
	else if (name.find("lava") != std::string::npos)
		s.kind = "Lava";
	else if (name.find("sea") != std::string::npos)
		s.kind = "Sea";
	else if (name.find("wave") != std::string::npos)
		s.kind = "Wave";
	else if (name.find("camera") != std::string::npos)
		s.kind = "Camera path";
	if (npc)
	{
		const char* creature = "CDisposable";
		if (name.find("santa") != std::string::npos)
		{
			creature = "CSanta, 3 reindeer and Santa";
			s.petName = "NPC_Santa.pet";
			if (SliceFloat(name, "*r", &f))
				Row(&s.rows, "Radius", Num(f));
		}
		else if (name.find("flight") != std::string::npos)
		{
			creature = "CFighter";
			s.petName = "NPC_Fighter.pet";
		}
		else if (name.find("ufo") != std::string::npos)
		{
			creature = "CUfo";
			s.petName = "sand_house12.pet";
		}
		else if (name.find("penguinwalk") != std::string::npos)
		{
			creature = "ntPenguin";
			s.petName = "NPC_penguin02.pet";
		}
		else if (name.find("drg") != std::string::npos)
		{
			creature = "ntNpc_Dragon";
			s.petName = "NPC_dragon.pet";
		}
		else if (name.find("ship") != std::string::npos)
		{
			creature = "ntNpc_PolaShip";
			s.petName = "NPC_ship.pet";
		}
		else if (name.find("vessel") != std::string::npos)
		{
			creature = "CNPC_TRSHIP";
			s.petName = name.find("vessel4") != std::string::npos ? "gs_ship_04.pet" : "gs_ship_02.pet";
		}
		else if (name.find("walex") != std::string::npos)
		{
			creature = "CNPC_TORTOISE";
			s.petName = name.find("walex03") != std::string::npos ? "npc_alex03.pet" : "npc_alex01.pet";
		}
		else if (name.find("herald") != std::string::npos)
		{
			creature = "CNPC_TORTOISE";
			s.petName = name.find("herald01") != std::string::npos ? "NPC_Herald01.pet" : "NPC_Herald03.pet";
		}
		else if (name.find("npc_t0") != std::string::npos)
		{
			creature = "ntNpc_WizCityNode";
			s.petName = "npc_insam_ani_N.pet";
		}
		else if (name.find("npc_t1") != std::string::npos)
		{
			creature = "ntNpc_WizCityNode";
			s.petName = "npc_cat_ani02_N.pet";
		}
		else
		{
			size_t p = name.find("npc_");
			if (p != std::string::npos)
			{
				std::string base = name.substr(p + 4);
				s.petName = base.substr(0, base.find(' ')) + ".pet";
			}
		}
		s.kind = std::string("Creature path: ") + creature;
		if (!s.petName.empty())
			Row(&s.rows, "Pet", Cp949ToUtf8(s.petName));
		if (SliceFloat(name, "*vel", &f))
			Row(&s.rows, "Speed", Num(f));
		else if (SliceFloat(name, "*v", &f))
			Row(&s.rows, "Speed", Num(f));
		if (SliceFloat(name, "*d", &f))
			Row(&s.rows, "Delay", Num(f));
		if (SliceFloat(name, "*t", &f))
			Row(&s.rows, "Time", Num(f));
	}
	return s;
}
