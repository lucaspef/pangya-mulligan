// SPDX-License-Identifier: MIT
// wep.cpp - reads and writes .wep projects.
#include "wep.h"
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wcommon.h"

// -- fields -------------------------------------------------------------------

const WepField* WepFields::Find(const std::string& key) const
{
	for (size_t i = 0; i < fields.size(); ++i)
		if (fields[i].key == key)
			return &fields[i];
	return 0;
}

std::string WepFields::Get(const std::string& key) const
{
	const WepField* f = Find(key);
	return f ? f->value : std::string();
}

int WepFields::GetInt(const std::string& key, int def) const
{
	const WepField* f = Find(key);
	return f ? atoi(f->value.c_str()) : def;
}

float WepFields::GetFloat(const std::string& key, float def) const
{
	const WepField* f = Find(key);
	return f ? (float)atof(f->value.c_str()) : def;
}

WVector WepFields::GetVec(const std::string& key) const
{
	WVector v;
	const WepField* f = Find(key);
	if (f)
		sscanf(f->value.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
	return v;
}

// Keys WangED writes, in its order. Anything else goes at the end.
static const char* const KEY_ORDER[] = {
	"GUID", "Type", "Class", "Name", "WadFile", "BspFile", "Filename", "Script",
	"Pos", "MinMax", "Dest", "Fov", "Bank", "Rot", "Scale", "Data",
	"Ambient", "Diffuse", "Ambient2", "Shadow", "Par",
	"FlagShow", "NoLOD", "ForceOpaque", "DiffLayer", "NoFog", "NoAlphaSort", "NoShadow"
};

static int KeyRank(const std::string& key)
{
	for (size_t i = 0; i < sizeof(KEY_ORDER) / sizeof(KEY_ORDER[0]); ++i)
		if (key == KEY_ORDER[i])
			return (int)i;
	return 1000;
}

static bool KeyIsQuoted(const std::string& key)
{
	return key == "Type" || key == "Class" || key == "Name" || key == "WadFile" || key == "BspFile" ||
		key == "Filename" || key == "Script" || key == "Data" || key == "DiffLayer";
}

void WepFields::Set(const std::string& key, const std::string& value)
{
	for (size_t i = 0; i < fields.size(); ++i)
	{
		if (fields[i].key == key)
		{
			fields[i].value = value;
			return;
		}
	}
	WepField f;
	f.key = key;
	f.sep = key == "Type" || key == "Name" || key == "GUID" ? "  " : " ";
	f.value = value;
	f.quoted = KeyIsQuoted(key);
	size_t at = fields.size();
	for (size_t i = 0; i < fields.size(); ++i)
	{
		if (KeyRank(fields[i].key) > KeyRank(key))
		{
			at = i;
			break;
		}
	}
	fields.insert(fields.begin() + at, f);
}

void WepFields::SetVec(const std::string& key, const WVector& v, int decimals)
{
	char buf[128];
	sprintf_s(buf, "%.*f %.*f %.*f", decimals, v.x, decimals, v.y, decimals, v.z);
	Set(key, buf);
}

void WepFields::SetFloat(const std::string& key, float f, int decimals)
{
	char buf[64];
	sprintf_s(buf, "%.*f", decimals, f);
	Set(key, buf);
}

void WepFields::SetInt(const std::string& key, int i)
{
	char buf[32];
	sprintf_s(buf, "%d", i);
	Set(key, buf);
}

void WepFields::SetColor(const std::string& key, int r, int g, int b)
{
	char buf[64];
	sprintf_s(buf, "%d %d %d", r, g, b);
	Set(key, buf);
}

void WepFields::Remove(const std::string& key)
{
	for (size_t i = 0; i < fields.size(); ++i)
	{
		if (fields[i].key == key)
		{
			fields.erase(fields.begin() + i);
			return;
		}
	}
}

void WepElement::GetBox(WVector* lo, WVector* hi) const
{
	*lo = WVector();
	*hi = WVector();
	sscanf(Get("MinMax").c_str(), "%f %f %f %f %f %f", &lo->x, &lo->y, &lo->z, &hi->x, &hi->y, &hi->z);
}

void WepElement::SetBox(const WVector& lo, const WVector& hi)
{
	char buf[128];
	sprintf_s(buf, "%.2f %.2f %.2f\t%.2f %.2f %.2f", lo.x, lo.y, lo.z, hi.x, hi.y, hi.z);
	Set("MinMax", buf);
}

void WepNodePoint::Set(const WVector& v)
{
	pos = v;
	char buf[96];
	sprintf_s(buf, "%.3f %.3f %.3f", v.x, v.y, v.z);
	text = buf;
}

// -- parser -------------------------------------------------------------------

namespace
{
	class LineReader
	{
	public:
		LineReader(const std::string& t) : text(t), at(0) { }
		bool End() const { return at >= text.size(); }
		// A line with its terminator stripped; the terminator is always CR LF in the data.
		std::string Next()
		{
			size_t e = text.find('\n', at);
			std::string line;
			if (e == std::string::npos)
			{
				line = text.substr(at);
				at = text.size();
			}
			else
			{
				line = text.substr(at, e - at);
				at = e + 1;
			}
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			return line;
		}
		std::string Blanks()
		{
			std::string gap;
			while (!End())
			{
				size_t save = at;
				std::string line = Next();
				if (!line.empty())
				{
					at = save;
					break;
				}
				gap += "\r\n";
			}
			return gap;
		}
		std::string Rest() const { return text.substr(at); }
		const std::string& text;
		size_t at;
	};
}

static bool ParseField(const std::string& rawLine, WepField* f)
{
	size_t i = 0;
	while (i < rawLine.size() && (rawLine[i] == '\t' || rawLine[i] == ' '))
		++i;
	if (i >= rawLine.size() || rawLine[i] != '*')
		return false;
	size_t k = i + 1;
	while (k < rawLine.size() && rawLine[k] != ' ' && rawLine[k] != '\t')
		++k;
	f->key = rawLine.substr(i + 1, k - i - 1);
	size_t v = k;
	while (v < rawLine.size() && (rawLine[v] == ' ' || rawLine[v] == '\t'))
		++v;
	f->sep = rawLine.substr(k, v - k);
	std::string value = rawLine.substr(v);
	f->quoted = value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"';
	f->value = f->quoted ? value.substr(1, value.size() - 2) : value;
	return true;
}

static void AppendField(std::string* out, const WepField& f)
{
	*out += "\t*";
	*out += f.key;
	*out += f.sep;
	if (f.quoted)
		*out += '"';
	*out += f.value;
	if (f.quoted)
		*out += '"';
	*out += "\r\n";
}

// "*Key value" at the top level; false when the line is something else.
static bool TopInt(const std::string& line, const char* key, int* out)
{
	size_t n = strlen(key);
	if (line.compare(0, n, key) != 0 || line.size() <= n || line[n] != ' ')
		return false;
	*out = atoi(line.c_str() + n + 1);
	return true;
}

void WepFile::Clear()
{
	header.clear();
	rootDir.clear();
	puppets.clear();
	visGroups = 0;
	nodes.clear();
	elements.clear();
	trailer.clear();
	lfOnly = false;
	utf8 = false;
	gapAfterRoot = gapAfterVis = gapAfterNodeCount = gapAfterElementCount = "\r\n";
	gapAfterPuppets = "\r\n\r\n";
}

bool WepFile::Parse(const std::string& source, std::string* error)
{
	Clear();
	// Everything below works in CR LF and CP949; Serialize turns both back. The archived
	// projects have been through UTF-8, so a strict decode decides which one this is.
	std::string text;
	lfOnly = source.find("\r\n") == std::string::npos;
	if (lfOnly)
	{
		text.reserve(source.size() + source.size() / 16);
		for (size_t i = 0; i < source.size(); ++i)
		{
			if (source[i] == '\n')
				text += '\r';
			text += source[i];
		}
	}
	else
	{
		text = source;
	}
	bool ascii = true;
	for (size_t i = 0; ascii && i < text.size(); ++i)
		ascii = (unsigned char)text[i] < 0x80;
	utf8 = !ascii && MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.c_str(), (int)text.size(), 0, 0) > 0;
	if (utf8)
		text = Utf8ToCp949(text);
	LineReader r(text);
	if (text.compare(0, 4, "#WEP") != 0)
	{
		*error = "not a WEP project: no #WEP line";
		return false;
	}
	std::string line;
	for (;;)
	{
		if (r.End())
		{
			*error = "no *RootDir";
			return false;
		}
		size_t save = r.at;
		line = r.Next();
		if (line.compare(0, 8, "*RootDir") == 0)
			break;
		header += text.substr(save, r.at - save);
	}
	WepField f;
	ParseField(line, &f);
	rootDir = f.value;
	gapAfterRoot = r.Blanks();

	int count = 0;
	if (!TopInt(r.Next(), "*NumPuppet", &count))
	{
		*error = "no *NumPuppet";
		return false;
	}
	for (int i = 0; i < count; ++i)
	{
		line = r.Next();
		size_t a = line.find('"'), b = line.rfind('"');
		if (a == std::string::npos || b <= a)
		{
			*error = "bad puppet line: " + line;
			return false;
		}
		puppets.push_back(line.substr(a + 1, b - a - 1));
	}
	gapAfterPuppets = r.Blanks();
	if (!TopInt(r.Next(), "*NumVisGroup", &visGroups) || visGroups != 0)
	{
		*error = "*NumVisGroup is not 0; no shipped project has any and the layout is unknown";
		return false;
	}
	gapAfterVis = r.Blanks();
	if (!TopInt(r.Next(), "*NumNodeList", &count))
	{
		*error = "no *NumNodeList";
		return false;
	}
	gapAfterNodeCount = r.Blanks();
	for (int i = 0; i < count; ++i)
	{
		line = r.Next();	// *Node n
		r.Next();			// {
		WepNode node;
		std::string guid;
		for (;;)
		{
			line = r.Next();
			if (line == "}" || r.End())
				break;
			if (!ParseField(line, &f))
				continue;
			if (f.key == "GUID" && (node.Has("Show")))
			{
				guid = line;
			}
			else if (f.key == "Npos")
			{
				WepNodePoint p;
				p.guid = guid;
				guid.clear();
				p.text = f.value;
				sscanf(f.value.c_str(), "%f %f %f", &p.pos.x, &p.pos.y, &p.pos.z);
				node.points.push_back(p);
			}
			else
			{
				node.fields.push_back(f);
			}
		}
		r.Blanks();
		nodes.push_back(node);
	}
	if (!TopInt(r.Next(), "*NumElement", &count))
	{
		*error = "no *NumElement";
		return false;
	}
	gapAfterElementCount = r.Blanks();
	for (int i = 0; i < count; ++i)
	{
		r.Next();
		r.Next();
		WepElement e;
		for (;;)
		{
			line = r.Next();
			if (line == "}" || r.End())
				break;
			if (ParseField(line, &f))
				e.fields.push_back(f);
		}
		r.Blanks();
		elements.push_back(e);
	}
	trailer = r.Rest();
	return true;
}

std::string WepFile::Serialize() const
{
	std::string out;
	out.reserve(64 * 1024);
	char buf[64];
	out += header;
	out += "*RootDir \"" + rootDir + "\"\r\n";
	out += gapAfterRoot;
	sprintf_s(buf, "*NumPuppet %d\r\n", (int)puppets.size());
	out += buf;
	for (size_t i = 0; i < puppets.size(); ++i)
	{
		sprintf_s(buf, "%4d : \"", (int)i);
		out += buf;
		out += puppets[i];
		out += "\"\r\n";
	}
	out += gapAfterPuppets;
	sprintf_s(buf, "*NumVisGroup %d\r\n", visGroups);
	out += buf;
	out += gapAfterVis;
	sprintf_s(buf, "*NumNodeList %d\r\n", (int)nodes.size());
	out += buf;
	out += gapAfterNodeCount;
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		sprintf_s(buf, "*Node %d\r\n{\r\n", (int)i);
		out += buf;
		for (size_t k = 0; k < nodes[i].fields.size(); ++k)
		{
			WepField f = nodes[i].fields[k];
			if (f.key == "Num")
			{
				sprintf_s(buf, "%d", (int)nodes[i].points.size());
				f.value = buf;
			}
			AppendField(&out, f);
		}
		for (size_t k = 0; k < nodes[i].points.size(); ++k)
		{
			if (!nodes[i].points[k].guid.empty())
				out += nodes[i].points[k].guid + "\r\n";
			out += "\t*Npos\t" + nodes[i].points[k].text + "\r\n";
		}
		out += "}\r\n\r\n";
	}
	sprintf_s(buf, "*NumElement %d\r\n", (int)elements.size());
	out += buf;
	out += gapAfterElementCount;
	for (size_t i = 0; i < elements.size(); ++i)
	{
		sprintf_s(buf, "*Element %d\r\n{\r\n", (int)i);
		out += buf;
		for (size_t k = 0; k < elements[i].fields.size(); ++k)
			AppendField(&out, elements[i].fields[k]);
		out += "}\r\n\r\n";
	}
	out += trailer;
	if (utf8)
		out = Cp949ToUtf8(out);
	if (lfOnly)
	{
		std::string lf;
		lf.reserve(out.size());
		for (size_t i = 0; i < out.size(); ++i)
			if (out[i] != '\r')
				lf += out[i];
		return lf;
	}
	return out;
}

bool WepFile::Load(const std::string& path, std::string* error)
{
	std::vector<unsigned char> data;
	if (!ReadWholeFile(path, &data))
	{
		*error = "cannot open " + path;
		return false;
	}
	return Parse(std::string(data.begin(), data.end()), error);
}

bool WepFile::Save(const std::string& path, std::string* error) const
{
	std::string text = Serialize();
	return WriteWholeFile(path, std::vector<unsigned char>(text.begin(), text.end()), error);
}

// -- helpers ------------------------------------------------------------------

const WepElement* WepFile::Game() const
{
	for (size_t i = 0; i < elements.size(); ++i)
		if (elements[i].IsGame())
			return &elements[i];
	return 0;
}

WepElement* WepFile::Game()
{
	for (size_t i = 0; i < elements.size(); ++i)
		if (elements[i].IsGame())
			return &elements[i];
	return 0;
}

const WepElement* WepFile::Ground() const
{
	for (size_t i = 0; i < elements.size(); ++i)
		if (elements[i].IsPuppet() && elements[i].IsGround())
			return &elements[i];
	return 0;
}

int WepFile::FindElement(const std::string& cls, const std::string& name) const
{
	for (size_t i = 0; i < elements.size(); ++i)
		if ((cls.empty() || elements[i].Class() == cls) && elements[i].Get("Name") == name)
			return (int)i;
	return -1;
}

// WGround stays last, where WangED keeps it, so positions in the file keep their meaning.
int WepFile::InsertElement(const WepElement& e)
{
	size_t at = elements.size();
	if (at && elements[at - 1].IsGround())
		--at;
	elements.insert(elements.begin() + at, e);
	return (int)at;
}

// Prefers the folder this puppet's neighbours came from, so the path keeps the project's form.
std::string WepFile::PuppetFile(const std::string& name) const
{
	for (size_t i = 0; i < puppets.size(); ++i)
		if (!_stricmp(PathBaseName(puppets[i]).c_str(), name.c_str()))
			return puppets[i];
	return rootDir + name;
}

void WepFile::SyncPuppetList()
{
	std::vector<std::string> kept;
	std::vector<bool> used(puppets.size(), false);
	std::vector<std::string> added;
	for (size_t i = 0; i < elements.size(); ++i)
	{
		if (!elements[i].IsPuppet())
			continue;
		std::string file = elements[i].Get("Filename");
		bool found = false;
		for (size_t k = 0; k < puppets.size() && !found; ++k)
		{
			if (puppets[k] == file)
			{
				used[k] = true;
				found = true;
			}
		}
		for (size_t k = 0; k < added.size() && !found; ++k)
			found = added[k] == file;
		if (!found)
			added.push_back(file);
	}
	for (size_t k = 0; k < puppets.size(); ++k)
		if (used[k])
			kept.push_back(puppets[k]);
	kept.insert(kept.end(), added.begin(), added.end());
	puppets.swap(kept);
}

void WepFile::NewEmpty()
{
	Clear();
	header = "#WEP V1.20 for Pangya\r\n#This is auto generated project file which is made by WangED. Don't edit this file.\r\n\r\n\r\n";
	rootDir = "D:/ProjectG/data/";
	trailer = "#End of WangED project file.\r\n";
	WepElement game = WepMakeElement("GAME", 0);
	elements.push_back(game);
	WepElement cam = WepMakeElement("CAMERA", "");
	cam.Set("Name", "DefaultCamera");
	cam.SetVec("Pos", WVector(0.0f, 120.0f, -300.0f));
	cam.SetVec("Dest", WVector(0.0f, 0.0f, 0.0f));
	elements.push_back(cam);
}

// Node blocks separate key and value with tabs, unlike elements.
WepNode WepMakeNode(const std::string& name)
{
	static const struct { const char* key; const char* sep; bool quoted; } LAYOUT[] = {
		{ "Name", "\t", true }, { "Type", "\t", false }, { "Num", "\t\t", false }, { "Show", "\t", false }
	};
	WepNode n;
	for (int i = 0; i < 4; ++i)
	{
		WepField f;
		f.key = LAYOUT[i].key;
		f.sep = LAYOUT[i].sep;
		f.quoted = LAYOUT[i].quoted;
		f.value = i == 0 ? name : i == 3 ? "1" : "0";
		n.fields.push_back(f);
	}
	return n;
}

WepElement WepMakeElement(const char* type, const char* cls)
{
	WepElement e;
	e.Set("Type", type);
	std::string t = type;
	if (t == "GAME")
	{
		e.Set("WadFile", "");
		e.Set("BspFile", "");
		e.Set("Ambient", "128 128 128");
		e.Set("Diffuse", "255 255 255");
		e.Set("Ambient2", "255 255 255");
		e.Set("Shadow", "128 128 128");
		e.Set("Par", "4");
		return e;
	}
	e.Set("Class", cls);
	if (t == "CAMERA")
	{
		e.Set("Name", "WCamera");
		e.Set("Pos", "0.00 0.00 0.00");
		e.Set("Dest", "0.00 0.00 100.00");
		e.Set("Fov", "60.00");
		e.Set("Bank", "0.00");
	}
	else if (t == "POINT")
	{
		e.Set("Name", "");
		e.Set("Pos", "0.00 0.00 0.00");
		if (strcmp(cls, "WPoint") && strcmp(cls, "WGlobalLight"))
			e.Set("Data", "");
	}
	else if (t == "AREA")
	{
		e.Set("Name", "");
		e.Set("MinMax", "-10.00 -10.00 -10.00\t10.00 10.00 10.00");
	}
	else if (t == "PUPPET")
	{
		e.Set("Filename", "");
		e.Set("Pos", "0.00 0.00 0.00");
		e.Set("Rot", "0.0 0.0 0.0");
		e.Set("Scale", "1.00");
		e.Set("FlagShow", "1");
		e.Set("NoLOD", "0");
		e.Set("ForceOpaque", "0");
		e.Set("DiffLayer", "normal");
	}
	return e;
}

// With row vectors: scale * Rx(-rx) * Ry(-ry) * Rz(rz), in degrees, where each R is the
// engine's WMatrix::Rotate for a positive angle. y is turned the opposite way to a hand
// rule, which is why 12 degrees in a .wep comes out as (cos, 0, sin) on the first row.
WMatrix WepPuppetMatrix(const WVector& pos, const WVector& rot, float scale)
{
	const float D = 3.14159265358979f / 180.0f;
	float cx = (float)cos(-rot.x * D), sx = (float)sin(-rot.x * D);
	float cy = (float)cos(-rot.y * D), sy = (float)sin(-rot.y * D);
	float cz = (float)cos(rot.z * D), sz = (float)sin(rot.z * D);
	WMatrix x, y, z;
	x.yx = cx; x.zx = sx; x.xy = 0;
	// Members follow the BONE chunk: (xx yx zx) is the first row.
	x.xx = 1; x.yx = 0; x.zx = 0;
	x.xy = 0; x.yy = cx; x.zy = sx;
	x.xz = 0; x.yz = -sx; x.zz = cx;
	y.xx = cy; y.yx = 0; y.zx = -sy;
	y.xy = 0; y.yy = 1; y.zy = 0;
	y.xz = sy; y.yz = 0; y.zz = cy;
	z.xx = cz; z.yx = sz; z.zx = 0;
	z.xy = -sz; z.yy = cz; z.zy = 0;
	z.xz = 0; z.yz = 0; z.zz = 1;
	WMatrix m = x * y * z;
	m.xx *= scale; m.yx *= scale; m.zx *= scale;
	m.xy *= scale; m.yy *= scale; m.zy *= scale;
	m.xz *= scale; m.yz *= scale; m.zz *= scale;
	m.SetPivot(pos);
	return m;
}

// The inverse, for projects imported from a .gbin. Rot comes back with the scale removed.
WVector WepRotFromMatrix(const WMatrix& mat, float* scale)
{
	const float R = 180.0f / 3.14159265358979f;
	float s = (float)sqrt(mat.xx * mat.xx + mat.yx * mat.yx + mat.zx * mat.zx);
	*scale = s;
	if (s < 0.000001f)
		return WVector();
	float inv = 1.0f / s;
	float r00 = mat.xx * inv, r01 = mat.yx * inv, r02 = mat.zx * inv;
	float r12 = mat.zy * inv, r22 = mat.zz * inv;
	float r10 = mat.xy * inv, r11 = mat.yy * inv;
	// Rx(a) Ry(b) Rz(c) in the convention above: r02 = -sin b.
	float sb = -r02;
	if (sb > 1.0f) sb = 1.0f;
	if (sb < -1.0f) sb = -1.0f;
	float b = (float)asin(sb), a, c;
	if (fabs(sb) < 0.9999f)
	{
		a = (float)atan2(r12, r22);
		c = (float)atan2(r01, r00);
	}
	else
	{
		a = (float)atan2(-mat.yz * inv, r11);
		c = 0.0f;
		(void)r10;
	}
	return WVector(-a * R, -b * R, c * R);
}
