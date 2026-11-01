// SPDX-License-Identifier: MIT
// course.cpp - see course.h.
#include "course.h"
#include <windows.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "areascript.h"

// -- small things -------------------------------------------------------------

void CourseLog::Info(const char* fmt, ...)
{
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, fmt, va);
	va_end(va);
	lines.push_back(buf);
}

void CourseLog::Warn(const char* fmt, ...)
{
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, fmt, va);
	va_end(va);
	lines.push_back(std::string("! ") + buf);
	++warnings;
}

// -- construction -------------------------------------------------------------

Course::Course()
{
	m_indexed = false;
	revision = 1;
	Clear();
}

Course::~Course()
{
	FreeAssets();
}

void Course::FreeAssets()
{
	for (std::map<std::string, PuppetAsset*>::iterator i = m_assets.begin(); i != m_assets.end(); ++i)
		delete i->second;
	m_assets.clear();
}

void Course::Clear()
{
	wep.Clear();
	path.clear();
	baseName.clear();
	dirty = false;
	hasSource = false;
	source.Clear();
	sourcePath.clear();
	hasSbin = false;
	sbin.Clear();
	m_groundKey.clear();
	m_groundY.clear();
	sbinCleared = false;
	prop.Clear();
	hasFog = false;
	fogColor[0] = fogColor[1] = fogColor[2] = 0.75f;
	fogNear = 0.0f;
	fogFar = 1500.0f;
	baseMeshes.clear();
	cornerColor.clear();
	m_undo.clear();
	m_redo.clear();
	++revision;
}

void Course::New()
{
	Clear();
	wep.NewEmpty();
}

void Course::SetDataRoot(const std::string& root)
{
	dataRoot = root;
	m_indexed = false;
	m_index.clear();
	FreeAssets();
	m_groundKey.clear();
	m_groundY.clear();
	++revision;
}

// -- asset index --------------------------------------------------------------

void Course::IndexRoot() const
{
	if (m_indexed)
		return;
	m_indexed = true;
	m_index.clear();
	static const wchar_t* const EXTS[] = { L".jpg", L".dds", L".tga", L".png", L".bmp", L".pet", L".gbin", L".sbin", L".xml", L".txt", L".spr", L".seq" };
	IndexAssets(dataRoot, EXTS, sizeof(EXTS) / sizeof(EXTS[0]), &m_index);
}

std::string Course::Resolve(const std::string& cp949Name) const
{
	IndexRoot();
	std::map<std::string, std::string>::const_iterator it = m_index.find(LowerCp949(PathBaseName(cp949Name)));
	return it != m_index.end() ? it->second : std::string();
}

std::vector<std::string> Course::AllPuppetNames() const
{
	IndexRoot();
	std::vector<std::string> out;
	for (std::map<std::string, std::string>::const_iterator i = m_index.begin(); i != m_index.end(); ++i)
		if (i->first.size() > 4 && !i->first.compare(i->first.size() - 4, 4, ".pet"))
			out.push_back(i->first);
	return out;
}

// The bones are applied the way CaddieRig's Document::PreparePart does it: each vertex
// through its first bone, normals rotated by the same bone, a zero normal from the face.
static void PrepareAsset(PuppetAsset* a)
{
	const PetFile& p = a->pet;
	size_t nb = p.bones.size();
	std::vector<WMatrix> world(nb);
	for (size_t i = 0; i < nb; ++i)
		world[i] = p.bones[i].parent >= 0 && p.bones[i].parent < (int)i ? p.bones[i].mat * world[p.bones[i].parent] : p.bones[i].mat;
	std::vector<WVector> rest(p.vertices.size());
	bool any = false;
	WVector lo, hi;
	for (size_t i = 0; i < p.vertices.size(); ++i)
	{
		const PetVertex& v = p.vertices[i];
		int b = v.blends.empty() ? -1 : v.blends[0].bone;
		rest[i] = b >= 0 && b < (int)nb ? v.pos * world[b] : v.pos;
	}
	size_t corners = p.points.size();
	a->pos.resize(corners);
	a->normal.resize(corners);
	for (size_t i = 0; i < corners; ++i)
	{
		const PetTriPoint& t = p.points[i];
		size_t vi = t.pos < rest.size() ? t.pos : 0;
		a->pos[i] = rest.empty() ? WVector() : rest[vi];
		int b = rest.empty() || p.vertices[vi].blends.empty() ? -1 : p.vertices[vi].blends[0].bone;
		WVector n = b >= 0 && b < (int)nb ? world[b].Rotate(t.normal) : t.normal;
		n.Normalize();
		a->normal[i] = n;
		const WVector& q = a->pos[i];
		if (!any)
		{
			lo = hi = q;
			any = true;
		}
		lo.x = min(lo.x, q.x); lo.y = min(lo.y, q.y); lo.z = min(lo.z, q.z);
		hi.x = max(hi.x, q.x); hi.y = max(hi.y, q.y); hi.z = max(hi.z, q.z);
	}
	for (size_t i = 0; i + 2 < corners; i += 3)
	{
		if (fabs(p.points[i].normal.x) + fabs(p.points[i].normal.y) + fabs(p.points[i].normal.z) < 1e-5f)
		{
			WVector n = (a->pos[i] - a->pos[i + 1]).Cross(a->pos[i + 2] - a->pos[i + 1]);
			n.Normalize();
			a->normal[i] = a->normal[i + 1] = a->normal[i + 2] = n;
		}
	}
	a->lo = lo;
	a->hi = hi;
	std::vector<bool> used(p.textures.size(), false);
	for (size_t t = 0; t < p.triTextures.size(); ++t)
		if (p.triTextures[t] < used.size())
			used[p.triTextures[t]] = true;
	for (size_t t = 0; t < p.textures.size(); ++t)
		if (used[t])
			a->textureNames.push_back(p.textures[t].Name());
}

PuppetAsset* Course::Puppet(const std::string& nameOrPath)
{
	std::string key = LowerCp949(PathBaseName(nameOrPath));
	std::map<std::string, PuppetAsset*>::iterator it = m_assets.find(key);
	if (it != m_assets.end())
		return it->second;
	PuppetAsset* a = new PuppetAsset;
	a->name = key;
	a->path = Resolve(key);
	if (a->path.empty() && !path.empty())
	{
		std::string beside = PathDir(path) + "\\" + Cp949ToUtf8(PathBaseName(nameOrPath));
		if (PathExists(beside))
			a->path = beside;
	}
	std::string error;
	if (!a->path.empty() && a->pet.Load(a->path.c_str(), &error))
	{
		a->loaded = true;
		PrepareAsset(a);
	}
	m_assets[key] = a;
	return a;
}

// -- open / save --------------------------------------------------------------

std::string Course::HoleSlotName(int slot)
{
	static const char* const NAMES[] = {
		"\xBD\xC3\xC0\xDB\xC1\xA1", "\xBD\xC3\xC0\xDB\xC1\xA1\x32",	// 시작점, 시작점2
		"\xB3\xA1\xC1\xA1\x31_1", "\xB3\xA1\xC1\xA1\x31_2", "\xB3\xA1\xC1\xA1\x31_3",	// 끝점1_n
		"\xB3\xA1\xC1\xA1\x32_1", "\xB3\xA1\xC1\xA1\x32_2", "\xB3\xA1\xC1\xA1\x32_3"	// 끝점2_n
	};
	return slot >= 0 && slot < 8 ? NAMES[slot] : "";
}

int Course::FindHoleSlot(int slot) const
{
	return wep.FindElement("WPoint", HoleSlotName(slot));
}

void Course::FindCompanions(CourseLog* log)
{
	// A saved-as project keeps its hole's identity through the base model's name.
	const WepElement* ground = wep.Ground();
	if (ground)
	{
		std::string stem = Cp949ToUtf8(PathStem(ground->Get("Filename")));
		if (Resolve(Utf8ToCp949(baseName + ".gbin")).empty() && !Resolve(Utf8ToCp949(stem + ".gbin")).empty())
			baseName = stem;
	}
	if (baseName.empty())
		return;
	std::string error;
	if (!hasSource)
	{
		std::string beside = PathDir(path) + "\\" + baseName + ".gbin";
		std::string p = Resolve(Utf8ToCp949(baseName + ".gbin"));
		if (PathExists(beside))
			p = beside;
		if (!p.empty() && source.Load(p, &error))
		{
			hasSource = true;
			sourcePath = p;
			log->Info("Compiled hole: %s (version 0x%X)", p.c_str(), source.version);
		}
	}
	std::string sp = hasSource ? sourcePath.substr(0, sourcePath.rfind('.')) + ".sbin" : Resolve(Utf8ToCp949(baseName + ".sbin"));
	if (!sp.empty())
	{
		FILE* f = OpenUtf8(sp.c_str(), L"rb");
		if (f)
		{
			fclose(f);
			if (sbin.Load(sp, &error))
			{
				hasSbin = true;
				log->Info("Shadow data: %s", sp.c_str());
			}
			else
			{
				log->Warn("%s: %s", sp.c_str(), error.c_str());
			}
		}
	}
	// wind_01 -> wind_property.xml, wind_fog.txt.
	std::string stem = baseName;
	size_t us = stem.rfind('_');
	if (us != std::string::npos)
		stem.erase(us);
	std::string xml = Resolve(Utf8ToCp949(stem + "_property.xml"));
	if (!xml.empty())
	{
		if (prop.Load(xml, &error))
			log->Info("Surface table: %s", xml.c_str());
		else
			log->Warn("%s: %s", xml.c_str(), error.c_str());
	}
	std::string fog = Resolve(Utf8ToCp949(stem + "_fog.txt"));
	if (!fog.empty())
	{
		FILE* f = OpenUtf8(fog.c_str(), L"rb");
		if (f)
		{
			float r, g, b, n, fr;
			if (fscanf(f, "%f %f %f %f %f", &r, &g, &b, &n, &fr) == 5)
			{
				hasFog = true;
				fogColor[0] = r / 255.0f;
				fogColor[1] = g / 255.0f;
				fogColor[2] = b / 255.0f;
				fogNear = n;
				fogFar = fr;
			}
			fclose(f);
		}
	}
}

bool Course::Open(const std::string& p, CourseLog* log)
{
	std::string ext = LowerCp949(p.substr(p.rfind('.') == std::string::npos ? p.size() : p.rfind('.')));
	std::string root = dataRoot;
	Clear();
	std::string error;
	path = p;
	baseName = PathStem(p);
	if (ext == ".gbin")
	{
		if (!source.Load(p, &error))
		{
			log->Warn("%s: %s", p.c_str(), error.c_str());
			Clear();
			return false;
		}
		hasSource = true;
		sourcePath = p;
		ImportGbin(source);
		path = p.substr(0, p.rfind('.')) + ".wep";
		log->Info("Reconstructed a project from %s (version 0x%X)", p.c_str(), source.version);
		dirty = true;
	}
	else if (!wep.Load(p, &error))
	{
		log->Warn("%s: %s", p.c_str(), error.c_str());
		Clear();
		return false;
	}
	FindCompanions(log);
	ComputeBaseMeshes();
	MapBaseColors();
	log->Info("%d elements, %d node lists, %d puppets", (int)wep.elements.size(), (int)wep.nodes.size(), (int)wep.puppets.size());
	++revision;
	return true;
}

bool Course::SaveWep(const std::string& p, std::string* error)
{
	// An untouched project goes back exactly as it came, stale list entries included.
	if (dirty)
		wep.SyncPuppetList();
	if (!wep.Save(p, error))
		return false;
	path = p;
	baseName = PathStem(p);
	dirty = false;
	return true;
}

// -- import -------------------------------------------------------------------

static void AddPointOption(WepElement* e, const char* option)
{
	e->Set("Data", FixedStr(option, 64));
}

void Course::ImportGbin(const GbinFile& g)
{
	wep.NewEmpty();
	wep.elements.clear();
	WepElement game = WepMakeElement("GAME", 0);
	game.SetInt("Par", g.par);
	for (size_t i = 0; i < g.points.size(); ++i)
	{
		if (g.points[i].type != 1)
			continue;
		unsigned d = 0xffffff, a = 0x808080, a2 = 0xffffff;
		sscanf(g.points[i].option, "%x %x %x", &d, &a, &a2);
		game.SetColor("Diffuse", (d >> 16) & 255, (d >> 8) & 255, d & 255);
		game.SetColor("Ambient", (a >> 16) & 255, (a >> 8) & 255, a & 255);
		game.SetColor("Ambient2", (a2 >> 16) & 255, (a2 >> 8) & 255, a2 & 255);
	}
	wep.elements.push_back(game);

	for (size_t i = 0; i < g.cameras.size(); ++i)
	{
		const GbinCamera& c = g.cameras[i];
		std::string name = FixedStr(c.name, 32);
		WepElement e = WepMakeElement("CAMERA", i == 0 ? "" : "WCamera");
		e.Set("Name", name);
		e.SetVec("Pos", WVector(c.eye[0], c.eye[1], c.eye[2]));
		e.SetVec("Dest", WVector(c.at[0], c.at[1], c.at[2]));
		e.SetFloat("Fov", c.fov);
		wep.elements.push_back(e);
	}
	for (size_t i = 0; i < g.points.size(); ++i)
	{
		const GbinPoint& p = g.points[i];
		static const char* const CLASSES[] = { "WPoint", "WGlobalLight", "WSun", "WMoon" };
		WepElement e = WepMakeElement("POINT", p.type < 4 ? CLASSES[p.type] : "WPoint");
		e.Set("Name", FixedStr(p.name, 32));
		e.SetVec("Pos", WVector(p.pos[0], p.pos[1], p.pos[2]));
		if (p.type >= 2)
		{
			AddPointOption(&e, p.option);
		}
		wep.elements.push_back(e);
	}
	for (size_t i = 0; i < g.areas.size(); ++i)
	{
		const GbinArea& a = g.areas[i];
		WepElement e = WepMakeElement("AREA", AreaClassOfType(a.type).c_str());
		e.Set("Name", FixedStr(a.name, 64));
		e.SetBox(WVector(a.box[0], a.box[1], a.box[2]), WVector(a.box[3], a.box[4], a.box[5]));
		wep.elements.push_back(e);
	}
	for (size_t i = 0; i < g.nodes.size(); ++i)
	{
		WepNode n = WepMakeNode(FixedStr(g.nodes[i].name.data(), g.nodes[i].name.size()));
		for (size_t k = 0; k < g.nodes[i].points.size(); ++k)
		{
			WepNodePoint np;
			np.Set(g.nodes[i].points[k]);
			n.points.push_back(np);
		}
		wep.nodes.push_back(n);
	}
	std::string dir = wep.rootDir;
	for (size_t i = 0; i < g.models.size(); ++i)
	{
		const GbinModel& m = g.models[i];
		WepElement e = WepMakeElement("PUPPET", (m.flag & 7) == 2 ? "WRotObj" : (m.flag & 7) == 4 ? "WVtxObj" : "WObject");
		e.Set("Filename", dir + FixedStr(m.name, 32));
		std::string script = FixedStr(m.option, 32);
		if (!script.empty())
			e.Set("Script", script);
		WMatrix w;
		memcpy(&w, m.mat, sizeof(m.mat));
		float s = 1.0f;
		WVector rot = WepRotFromMatrix(w, &s);
		e.SetVec("Pos", w.Pivot());
		e.SetVec("Rot", rot, 1);
		e.SetFloat("Scale", s);
		e.SetInt("NoLOD", (m.flag & 0x10) ? 1 : 0);
		e.SetInt("ForceOpaque", (m.flag & 8) ? 1 : 0);
		e.Set("DiffLayer", m.detail == 1 ? "diff1" : m.detail == 2 ? "diff2" : "normal");
		wep.elements.push_back(e);
	}
	if (g.hasBase)
	{
		WepElement e = WepMakeElement("PUPPET", "WGround");
		e.Set("Filename", dir + FixedStr(g.base.name, 32));
		e.SetVec("Pos", WVector(g.base.mat[9], g.base.mat[10], g.base.mat[11]));
		wep.elements.push_back(e);
	}
	wep.SyncPuppetList();
}

// -- base model ---------------------------------------------------------------

PuppetAsset* Course::BasePuppet()
{
	const WepElement* g = wep.Ground();
	if (!g)
		return 0;
	PuppetAsset* a = Puppet(g->Get("Filename"));
	return a->loaded ? a : 0;
}

// WBone::SetMesh and GetIndexedmesh: faces split by (texture, group) in first-use order,
// each new mesh pushed on the front of the chain. Vertices merge on position, UV and normal.
void Course::ComputeBaseMeshes()
{
	baseMeshes.clear();
	PuppetAsset* a = BasePuppet();
	if (!a)
		return;
	const PetFile& p = a->pet;
	size_t faces = p.points.size() / 3;
	std::vector<BaseMesh> built;
	std::vector<std::pair<std::string, int> > keys;
	for (size_t f = 0; f < faces; ++f)
	{
		int slot = f < p.triTextures.size() ? p.triTextures[f] : 0;
		std::string name = slot < (int)p.textures.size() ? LowerCp949(p.textures[slot].Name()) : std::string();
		int group = slot < (int)p.textures.size() ? p.textures[slot].group : 0;
		size_t m = 0;
		for (; m < keys.size(); ++m)
			if (keys[m].second == group && keys[m].first == name)
				break;
		if (m == keys.size())
		{
			keys.push_back(std::make_pair(name, group));
			BaseMesh fresh;
			fresh.textureSlot = slot;
			built.push_back(fresh);
		}
		built[m].faces.push_back((int)f);
	}
	for (size_t m = 0; m < built.size(); ++m)
	{
		BaseMesh& mesh = built[m];
		mesh.index.reserve(mesh.faces.size() * 3);
		// Hashing on the exact position first keeps huge meshes quick; the 1e-5 tolerance of
		// the original only matters between values the exporter never produces.
		std::map<std::pair<u32, std::pair<u32, u32> >, std::vector<int> > bucket;
		for (size_t f = 0; f < mesh.faces.size(); ++f)
		{
			for (int c = 0; c < 3; ++c)
			{
				int corner = mesh.faces[f] * 3 + c;
				const WVector& q = p.vertices.empty() ? a->pos[corner] : p.vertices[p.points[corner].pos].pos;
				const PetTriPoint& pt = p.points[corner];
				u32 x, y, z;
				memcpy(&x, &q.x, 4);
				memcpy(&y, &q.y, 4);
				memcpy(&z, &q.z, 4);
				std::vector<int>& list = bucket[std::make_pair(x, std::make_pair(y, z))];
				int found = -1;
				for (size_t i = 0; i < list.size(); ++i)
				{
					const PetTriPoint& o = p.points[mesh.vertexCorner[list[i]]];
					if (o.u == pt.u && o.v == pt.v &&
						fabs(a->normal[mesh.vertexCorner[list[i]]].x - a->normal[corner].x) <= 0.001f &&
						fabs(a->normal[mesh.vertexCorner[list[i]]].y - a->normal[corner].y) <= 0.001f &&
						fabs(a->normal[mesh.vertexCorner[list[i]]].z - a->normal[corner].z) <= 0.001f)
					{
						found = list[i];
						break;
					}
				}
				if (found < 0)
				{
					found = (int)mesh.vertexCorner.size();
					mesh.vertexCorner.push_back(corner);
					list.push_back(found);
				}
				mesh.index.push_back(found);
			}
		}
	}
	baseMeshes.assign(built.rbegin(), built.rend());
}

void Course::MapBaseColors()
{
	cornerColor.clear();
	PuppetAsset* a = BasePuppet();
	if (!a || !hasSource)
		return;
	size_t corners = a->pet.points.size();
	if (source.version < 0x72)
	{
		if (source.baseColors.size() != corners)
			return;
		cornerColor.resize(corners, 0xffffffff);
		size_t at = 0;
		for (size_t m = 0; m < baseMeshes.size(); ++m)
			for (size_t f = 0; f < baseMeshes[m].faces.size(); ++f)
				for (int c = 0; c < 3; ++c)
					cornerColor[baseMeshes[m].faces[f] * 3 + c] = source.baseColors[at++];
	}
	else if (source.boneColors.size() == 1 && source.boneColors[0].meshes.size() == baseMeshes.size())
	{
		cornerColor.resize(corners, 0xffffffff);
		for (size_t m = 0; m < baseMeshes.size(); ++m)
		{
			const BaseMesh& mesh = baseMeshes[m];
			const std::vector<u32>& colors = source.boneColors[0].meshes[m].colors;
			if (colors.size() != mesh.vertexCorner.size())
				continue;
			for (size_t f = 0; f < mesh.faces.size(); ++f)
				for (int c = 0; c < 3; ++c)
					cornerColor[mesh.faces[f] * 3 + c] = colors[mesh.index[f * 3 + c]];
		}
	}
}

bool Course::BaseColorsValid() const
{
	return !cornerColor.empty();
}

// The first-order lighting the shipped colours look like: ambient plus sun by surface angle,
// dropped to the shadow colour when something stands between the corner and the sun.
void Course::BakeBaseColors(CourseLog* log)
{
	PuppetAsset* a = BasePuppet();
	if (!a)
	{
		log->Warn("No base model to light.");
		return;
	}
	if (baseMeshes.empty())
		ComputeBaseMeshes();
	const WepElement* g = wep.Ground();
	WMatrix mat = ElementMatrix(*g);
	float amb[3], dif[3], shd[3];
	GameColor("Ambient", amb);
	GameColor("Diffuse", dif);
	GameColor("Shadow", shd);
	WVector travel = SunDirection();
	WVector toSun = travel * -1.0f;
	size_t corners = a->pos.size();
	std::vector<u32> oldColors = cornerColor;
	cornerColor.resize(corners);
	int shadowed = 0;
	std::map<std::pair<u32, u32>, bool> cache;
	for (size_t i = 0; i < corners; ++i)
	{
		WVector w = a->pos[i] * mat;
		WVector n = mat.Rotate(a->normal[i]);
		n.Normalize();
		float d = n.Dot(toSun);
		if (d < 0.0f)
			d = 0.0f;
		bool lit = true;
		if (d > 0.0f)
		{
			u32 kx, kz;
			memcpy(&kx, &w.x, 4);
			memcpy(&kz, &w.z, 4);
			std::pair<u32, u32> key(kx, kz);
			std::map<std::pair<u32, u32>, bool>::iterator it = cache.find(key);
			if (it != cache.end())
				lit = it->second;
			else
			{
				TerrainHit h = CastRay(w + toSun * 0.2f, toSun, false, 2500.0f);
				lit = !h.hit;
				cache[key] = lit;
			}
		}
		if (!lit)
			++shadowed;
		float rgb[3];
		for (int k = 0; k < 3; ++k)
		{
			float c = lit ? amb[k] + dif[k] * d : shd[k];
			rgb[k] = c > 1.0f ? 1.0f : c;
		}
		// Shipped colours are opaque except a few percent of alpha 0 corners, which the shadow
		// maps read; those stay as they were when there is anything to keep.
		u32 alpha = i < oldColors.size() ? oldColors[i] & 0xff000000u : 0xff000000u;
		cornerColor[i] = alpha | ((u32)(rgb[0] * 255.0f) << 16) | ((u32)(rgb[1] * 255.0f) << 8) | (u32)(rgb[2] * 255.0f);
	}
	log->Info("Lit %d corners of the base model, %d of them in shadow.", (int)corners, shadowed);
	++revision;
	dirty = true;
}

void Course::ClearShadowMaps(CourseLog* log)
{
	if (!hasSbin)
	{
		log->Info("There are no shadow maps to clear.");
		return;
	}
	for (size_t b = 0; b < sbin.bodies.size(); ++b)
	{
		SbinBody& body = sbin.bodies[b];
		for (int s = 0; s < 3; ++s)
		{
			body.sections[s].size = 0;
			body.sections[s].packs.clear();
			body.outNum[s] = 0;
			body.outCounts[s].clear();
			body.outFrags[s].clear();
		}
	}
	sbinCleared = true;
	dirty = true;
	++revision;
	log->Info("Shadow maps cleared; the next export writes an .sbin with no fragments.");
}

// -- scene --------------------------------------------------------------------

WMatrix Course::ElementMatrix(const WepElement& e) const
{
	if (e.IsPuppet())
		return WepPuppetMatrix(e.GetVec("Pos"), e.GetVec("Rot"), e.GetFloat("Scale", 1.0f));
	WMatrix m;
	m.SetPivot(e.GetVec("Pos"));
	return m;
}

void GrowBox(WVector* lo, WVector* hi, const WVector& q)
{
	lo->x = min(lo->x, q.x); lo->y = min(lo->y, q.y); lo->z = min(lo->z, q.z);
	hi->x = max(hi->x, q.x); hi->y = max(hi->y, q.y); hi->z = max(hi->z, q.z);
}

void Course::ElementBounds(const WepElement& e, WVector* lo, WVector* hi)
{
	if (e.IsArea())
	{
		e.GetBox(lo, hi);
		return;
	}
	WVector pos = e.GetVec("Pos");
	if (e.IsPuppet())
	{
		PuppetAsset* a = Puppet(e.Get("Filename"));
		if (a->loaded && !a->pos.empty())
		{
			WMatrix m = ElementMatrix(e);
			*lo = *hi = a->pos[0] * m;
			for (size_t i = 1; i < a->pos.size(); ++i)
				GrowBox(lo, hi, a->pos[i] * m);
			return;
		}
	}
	*lo = pos - WVector(3, 3, 3);
	*hi = pos + WVector(3, 3, 3);
}

void Course::SceneBounds(WVector* lo, WVector* hi)
{
	bool any = false;
	for (size_t i = 0; i < wep.elements.size(); ++i)
	{
		const WepElement& e = wep.elements[i];
		if (e.IsGame() || (e.IsPoint() && e.Class() != "WPoint"))
			continue;
		WVector a, b;
		ElementBounds(e, &a, &b);
		if (!any)
		{
			*lo = a;
			*hi = b;
			any = true;
		}
		GrowBox(lo, hi, a);
		GrowBox(lo, hi, b);
	}
	if (!any)
	{
		*lo = WVector(-100, -10, -100);
		*hi = WVector(100, 10, 100);
	}
}

// Moller-Trumbore, against the base model and optionally everything else.
static bool RayTri(const WVector& o, const WVector& d, const WVector& a, const WVector& b, const WVector& c, float* t)
{
	WVector e1 = b - a, e2 = c - a;
	WVector p = d.Cross(e2);
	float det = e1.Dot(p);
	if (fabs(det) < 1e-7f)
		return false;
	float inv = 1.0f / det;
	WVector s = o - a;
	float u = s.Dot(p) * inv;
	if (u < 0.0f || u > 1.0f)
		return false;
	WVector q = s.Cross(e1);
	float v = d.Dot(q) * inv;
	if (v < 0.0f || u + v > 1.0f)
		return false;
	float hit = e2.Dot(q) * inv;
	if (hit <= 0.0f)
		return false;
	*t = hit;
	return true;
}

static bool RayBox(const WVector& o, const WVector& d, const WVector& lo, const WVector& hi, float maxDist)
{
	float t0 = 0.0f, t1 = maxDist;
	const float* po = &o.x;
	const float* pd = &d.x;
	const float* pl = &lo.x;
	const float* ph = &hi.x;
	for (int k = 0; k < 3; ++k)
	{
		if (fabs(pd[k]) < 1e-7f)
		{
			if (po[k] < pl[k] || po[k] > ph[k])
				return false;
			continue;
		}
		float a = (pl[k] - po[k]) / pd[k], b = (ph[k] - po[k]) / pd[k];
		if (a > b)
			std::swap(a, b);
		if (a > t0) t0 = a;
		if (b < t1) t1 = b;
		if (t0 > t1)
			return false;
	}
	return true;
}

TerrainHit Course::CastRay(const WVector& from, const WVector& dir, bool baseOnly, float maxDist)
{
	TerrainHit best;
	best.hit = false;
	best.t = maxDist;
	best.element = -1;
	for (size_t i = 0; i < wep.elements.size(); ++i)
	{
		const WepElement& e = wep.elements[i];
		if (!e.IsPuppet() || (baseOnly && !e.IsGround()))
			continue;
		if (!e.GetInt("FlagShow", 1))
			continue;
		PuppetAsset* a = Puppet(e.Get("Filename"));
		if (!a->loaded)
			continue;
		WMatrix m = ElementMatrix(e);
		WMatrix inv = m.Inverse();
		WVector o = from * inv;
		WVector d = inv.Rotate(dir);
		if (!RayBox(o, d, a->lo, a->hi, 1e9f))
			continue;
		for (size_t c = 0; c + 2 < a->pos.size(); c += 3)
		{
			float t;
			if (!RayTri(o, d, a->pos[c], a->pos[c + 1], a->pos[c + 2], &t) || t >= best.t)
				continue;
			// The world and local parameters agree because d is not normalised after inverting.
			best.hit = true;
			best.t = t;
			best.element = e.IsGround() ? -1 : (int)i;
			WVector n = (a->pos[c + 1] - a->pos[c]).Cross(a->pos[c + 2] - a->pos[c]);
			n = m.Rotate(n);
			n.Normalize();
			if (n.Dot(dir) > 0.0f)
				n = n * -1.0f;
			best.normal = n;
		}
	}
	if (best.hit)
		best.pos = from + dir * best.t;
	return best;
}

bool Course::DropToGround(WVector* pos)
{
	TerrainHit h = CastRay(WVector(pos->x, pos->y + 4000.0f, pos->z), WVector(0, -1, 0), true);
	if (!h.hit)
		return false;
	*pos = h.pos;
	return true;
}

bool Course::StandOnGround(WVector* pos)
{
	const WepElement* g = wep.Ground();
	std::string key = g ? g->Get("Filename") + g->Get("Pos") + g->Get("Rot") + g->Get("Scale") : std::string();
	if (key != m_groundKey)
	{
		m_groundKey = key;
		m_groundY.clear();
	}
	u32 kx, kz;
	memcpy(&kx, &pos->x, 4);
	memcpy(&kz, &pos->z, 4);
	std::pair<u32, u32> at(kx, kz);
	std::map<std::pair<u32, u32>, std::pair<bool, float> >::iterator it = m_groundY.find(at);
	if (it == m_groundY.end())
	{
		WVector q = *pos;
		bool hit = DropToGround(&q);
		it = m_groundY.insert(std::make_pair(at, std::make_pair(hit, q.y))).first;
	}
	if (it->second.first)
		pos->y = it->second.second;
	return it->second.first;
}

// The client uses the first WGlobalLight: it normalises -position into the direction.
WVector Course::SunDirection() const
{
	for (size_t i = 0; i < wep.elements.size(); ++i)
	{
		if (wep.elements[i].Class() == "WGlobalLight")
		{
			WVector d = wep.elements[i].GetVec("Pos") * -1.0f;
			d.Normalize();
			return d;
		}
	}
	return WVector(0.0f, -0.70710678f, 0.70710678f);
}

void Course::GameColor(const char* key, float rgb[3]) const
{
	int r = 128, g = 128, b = 128;
	const WepElement* e = wep.Game();
	if (e)
		sscanf(e->Get(key).c_str(), "%d %d %d", &r, &g, &b);
	rgb[0] = r / 255.0f;
	rgb[1] = g / 255.0f;
	rgb[2] = b / 255.0f;
}

// -- compile ------------------------------------------------------------------

std::vector<std::string> Course::UsedTextures()
{
	std::vector<std::string> out;
	std::map<std::string, bool> seen;
	// The base model first, as in the shipped tables. The rest of their order has no rule
	// anyone has found, and the client only looks entries up by name.
	for (int pass = 0; pass < 2; ++pass)
	{
		for (size_t i = 0; i < wep.elements.size(); ++i)
		{
			const WepElement& e = wep.elements[i];
			if (!e.IsPuppet() || e.IsGround() != (pass == 0))
				continue;
			PuppetAsset* a = Puppet(e.Get("Filename"));
			if (!a->loaded)
				continue;
			for (size_t t = 0; t < a->textureNames.size(); ++t)
			{
				std::string key = LowerCp949(a->textureNames[t]);
				if (seen.find(key) == seen.end())
				{
					seen[key] = true;
					out.push_back(a->textureNames[t]);
				}
			}
		}
	}
	return out;
}

static void SetMatrix(float out[12], const WMatrix& m)
{
	memcpy(out, &m, sizeof(float) * 12);
}

static void BoxOf(PuppetAsset* a, const WMatrix& m, float out[6])
{
	WVector lo, hi;
	if (a->pos.empty())
	{
		lo = hi = m.Pivot();
	}
	else
	{
		lo = hi = a->pos[0] * m;
		for (size_t i = 1; i < a->pos.size(); ++i)
			GrowBox(&lo, &hi, a->pos[i] * m);
	}
	out[0] = lo.x; out[1] = lo.y; out[2] = lo.z;
	out[3] = hi.x; out[4] = hi.y; out[5] = hi.z;
}

// sceneAabb: the vertices in the bottom 10 units of the model, boxed up to that height.
// Worked out from the data: shipped boxes are exactly 10 tall or the whole model. This is
// the closest rule found, exact for 56% of shipped models and close on the rest; the client
// only passes the box to CSceneManager::RegisterElement.
static void SceneBoxOf(PuppetAsset* a, const WMatrix& m, float out[6])
{
	if (a->pos.empty())
	{
		BoxOf(a, m, out);
		return;
	}
	std::vector<WVector> w(a->pos.size());
	float bottom = 1e30f;
	for (size_t i = 0; i < a->pos.size(); ++i)
	{
		w[i] = a->pos[i] * m;
		bottom = min(bottom, w[i].y);
	}
	float top = bottom + 10.0f;
	bool any = false, taller = false;
	WVector lo, hi;
	for (size_t i = 0; i < w.size(); ++i)
	{
		if (w[i].y > top)
		{
			taller = true;
			continue;
		}
		if (!any)
		{
			lo = hi = w[i];
			any = true;
		}
		else
		{
			GrowBox(&lo, &hi, w[i]);
		}
	}
	if (taller)
		hi.y = top;
	out[0] = lo.x; out[1] = lo.y; out[2] = lo.z;
	out[3] = hi.x; out[4] = hi.y; out[5] = hi.z;
}

static std::string HexColor(const std::string& rgb)
{
	int r = 0, g = 0, b = 0;
	sscanf(rgb.c_str(), "%d %d %d", &r, &g, &b);
	char buf[16];
	sprintf_s(buf, "%02x%02x%02x", r, g, b);
	return buf;
}

bool Course::Compile(GbinFile* out, CourseLog* log)
{
	out->Clear();
	const WepElement* game = wep.Game();
	const WepElement* ground = wep.Ground();
	if (!game)
	{
		log->Warn("The project has no GAME element.");
		return false;
	}
	if (!ground)
	{
		log->Warn("The project has no WGround puppet, so there is no hole to compile.");
		return false;
	}
	out->version = 0x71;
	out->kind = GBIN_GBIN;
	out->hasBase = true;

	for (size_t i = 0; i < wep.elements.size(); ++i)
	{
		const WepElement& e = wep.elements[i];
		if (e.IsCamera())
		{
			GbinCamera c;
			memset(&c, 0, sizeof(c));
			FixedCopy(c.name, 32, e.Get("Name"));
			WVector p = e.GetVec("Pos"), d = e.GetVec("Dest");
			memcpy(c.eye, &p, 12);
			memcpy(c.at, &d, 12);
			c.fov = e.GetFloat("Fov", 60.0f);
			c.link.parent = -1;
			out->cameras.push_back(c);
		}
		else if (e.IsPoint())
		{
			std::string cls = e.Class();
			if (cls == "WSecondaryGlobalLight")
				continue;	// WangED only uses it to shade; no .gbin carries it
			GbinPoint p;
			memset(&p, 0, sizeof(p));
			p.type = cls == "WGlobalLight" ? 1 : cls == "WSun" ? 2 : cls == "WMoon" ? 3 : 0;
			FixedCopy(p.name, 32, e.Get("Name"));
			WVector pos = e.GetVec("Pos");
			memcpy(p.pos, &pos, 12);
			if (p.type == 1)
			{
				std::string option = HexColor(game->Get("Diffuse")) + " " + HexColor(game->Get("Ambient")) + " " + HexColor(game->Get("Ambient2"));
				FixedCopy(p.option, 64, option);
			}
			else if (p.type)
			{
				FixedCopy(p.option, 64, e.Get("Data"));
			}
			p.link.parent = -1;
			out->points.push_back(p);
		}
		else if (e.IsArea())
		{
			GbinArea a;
			memset(&a, 0, sizeof(a));
			a.type = (u8)AreaTypeOfClass(e.Class());
			FixedCopy(a.name, 64, e.Get("Name"));
			WVector lo, hi;
			e.GetBox(&lo, &hi);
			memcpy(a.box, &lo, 12);
			memcpy(a.box + 3, &hi, 12);
			a.link.parent = -1;
			out->areas.push_back(a);
		}
	}

	std::vector<std::string> tex = UsedTextures();
	for (size_t i = 0; i < tex.size(); ++i)
	{
		char buf[32];
		FixedCopy(buf, 32, tex[i]);
		out->textures.push_back(std::string(buf, 32));
	}
	for (size_t i = 0; i < wep.nodes.size(); ++i)
	{
		GbinNode n;
		// All 16 bytes can be name; shipped ones like "npc_walex03 *vel 3" just get cut off.
		n.name = wep.nodes[i].Get("Name").substr(0, 16);
		n.name.resize(16, '\0');
		n.reserved = 0;
		for (size_t k = 0; k < wep.nodes[i].points.size(); ++k)
			n.points.push_back(wep.nodes[i].points[k].pos);
		out->nodes.push_back(n);
	}

	PuppetAsset* base = Puppet(ground->Get("Filename"));
	memset(&out->base, 0, sizeof(out->base));
	FixedCopy(out->base.name, 32, PathBaseName(ground->Get("Filename")));
	WMatrix baseMat = ElementMatrix(*ground);
	SetMatrix(out->base.mat, baseMat);
	if (base->loaded)
	{
		out->base.faceNum = (int)base->pet.points.size() / 3;
		BoxOf(base, baseMat, out->base.localAabb);
	}
	else
	{
		log->Warn("Base model %s was not found; its bounds and colours are empty.", Cp949ToUtf8(ground->Get("Filename")).c_str());
	}
	out->base.link.parent = -1;

	// Colours go back in mesh order, the order WPolySoup::Load reads them.
	if (base->loaded)
	{
		size_t corners = base->pet.points.size();
		out->baseColors.assign(corners, 0xffffffff);
		if (cornerColor.size() == corners)
		{
			size_t at = 0;
			for (size_t m = 0; m < baseMeshes.size(); ++m)
				for (size_t f = 0; f < baseMeshes[m].faces.size(); ++f)
					for (int c = 0; c < 3; ++c)
						out->baseColors[at++] = cornerColor[baseMeshes[m].faces[f] * 3 + c];
		}
		else
		{
			log->Warn("No vertex colours for the base model; it compiles white. Use Build > Light Base Model.");
		}
	}

	// In project order: WPolySoup::Load reads them all and filters each by its detail.
	for (size_t i = 0; i < wep.elements.size(); ++i)
	{
		const WepElement& e = wep.elements[i];
		if (!e.IsPuppet() || e.IsGround())
			continue;
		if (!e.GetInt("FlagShow", 1))
			continue;
		GbinModel m;
		memset(&m, 0, sizeof(m));
		std::string cls = e.Class();
		m.flag = (u8)((cls == "WRotObj" ? 2 : cls == "WVtxObj" ? 4 : 0) |
			(e.GetInt("ForceOpaque") ? 8 : 0) | (e.GetInt("NoLOD") ? 0x10 : 0));
		std::string file = PathBaseName(e.Get("Filename"));
		FixedCopy(m.name, 32, file);
		WMatrix mat = ElementMatrix(e);
		SetMatrix(m.mat, mat);
		FixedCopy(m.option, 32, e.Get("Script"));
		std::string layer = e.Get("DiffLayer");
		m.detail = layer == "diff1" ? 1 : layer == "diff2" ? 2 : 0;
		m.link.parent = -1;
		PuppetAsset* a = Puppet(file);
		if (a->loaded)
		{
			m.faceNum = (int)a->pet.points.size() / 3;
			BoxOf(a, mat, m.localAabb);
			SceneBoxOf(a, mat, m.sceneAabb);
		}
		else
		{
			log->Warn("Puppet %s was not found; it compiles with empty bounds.", Cp949ToUtf8(file).c_str());
			WVector p = mat.Pivot();
			m.localAabb[0] = m.localAabb[3] = m.sceneAabb[0] = m.sceneAabb[3] = p.x;
			m.localAabb[1] = m.localAabb[4] = m.sceneAabb[1] = m.sceneAabb[4] = p.y;
			m.localAabb[2] = m.localAabb[5] = m.sceneAabb[2] = m.sceneAabb[5] = p.z;
		}
		++out->modelNum[m.detail];
		out->models.push_back(m);
	}

	// LoadMapCheckData: the last 0x41 bytes. Slots fall back on the first of their kind.
	out->par = (u8)game->GetInt("Par", 4);
	out->hole.par = out->par;
	float slot[8][2];
	bool have[8];
	for (int s = 0; s < HoleSlotCount(); ++s)
	{
		int e = FindHoleSlot(s);
		have[s] = e >= 0;
		if (e >= 0)
		{
			WVector p = wep.elements[e].GetVec("Pos");
			slot[s][0] = p.x;
			slot[s][1] = p.z;
		}
		else
		{
			slot[s][0] = slot[s][1] = 0.0f;
		}
	}
	for (int s = 1; s < HoleSlotCount(); ++s)
	{
		int first = s < 2 ? 0 : 2;
		if (!have[s] && have[first])
			memcpy(slot[s], slot[first], 8);
	}
	memcpy(out->hole.tee, slot, 16);
	memcpy(out->hole.pin, slot + 2, 48);
	return true;
}

bool Course::ExportHole(const std::string& gbinPath, CourseLog* log)
{
	GbinFile g;
	if (!Compile(&g, log))
		return false;
	if (hasSource && source.version >= 0x72)
		log->Warn("The source hole is version 0x%X. FairwayED writes 0x71, without its object hierarchy.", source.version);
	std::string error;
	if (!g.Save(gbinPath, &error))
	{
		log->Warn("%s", error.c_str());
		return false;
	}
	log->Info("Wrote %s: %d models, %d textures, %d nodes, %d areas", gbinPath.c_str(), (int)g.models.size(), (int)g.textures.size(), (int)g.nodes.size(), (int)g.areas.size());
	if (hasSbin)
	{
		std::string sp = gbinPath.substr(0, gbinPath.rfind('.')) + ".sbin";
		if (!sbin.Save(sp, &error))
		{
			log->Warn("%s", error.c_str());
			return false;
		}
		log->Info("Wrote %s%s", sp.c_str(), sbinCleared ? " (no shadow fragments)" : "");
		if (!sbinCleared)
			log->Info("Note: this .sbin is the one baked for the original placement. FairwayED draws and carries shadow maps but cannot bake them.");
	}
	else
	{
		log->Warn("No .sbin goes with this hole. The client treats a missing one as no shadow maps.");
	}
	return true;
}

// The limits the loader relies on, and what a hole cannot play without.
void Course::Validate(CourseLog* log)
{
	int warningsBefore = log->warnings;
	const WepElement* game = wep.Game();
	const WepElement* ground = wep.Ground();
	if (!game)
		log->Warn("No GAME element.");
	if (!ground)
		log->Warn("No WGround puppet: WPolySoup::Load needs a base model.");
	else if (!Puppet(ground->Get("Filename"))->loaded)
		log->Warn("Base model not found: %s", Cp949ToUtf8(ground->Get("Filename")).c_str());
	if (game)
	{
		int par = game->GetInt("Par");
		if (par < 3 || par > 5)
			log->Warn("Par %d: every shipped hole is 3, 4 or 5.", par);
	}
	if (FindHoleSlot(0) < 0)
		log->Warn("No tee point (\xEC\x8B\x9C\xEC\x9E\x91\xEC\xA0\x90).");
	for (int s = 2; s < 5; ++s)
		if (FindHoleSlot(s) < 0)
			log->Warn("Pin slot %s is missing; the client can pick any of the three.", Cp949ToUtf8(HoleSlotName(s)).c_str());
	if (wep.FindElement("WPoint", "\xB1\xD7\xB8\xAE\xB5\xE5") < 0)
		log->Info("Note: no grid point (\xEA\xB7\xB8\xEB\xA6\xAC\xEB\x93\x9C).");

	int lights = 0, suns = 0, cameras = 0, puppets = 0, missing = 0;
	bool hasDefaultCam = false;
	std::map<std::string, bool> reported;
	for (size_t i = 0; i < wep.elements.size(); ++i)
	{
		const WepElement& e = wep.elements[i];
		std::string cls = e.Class();
		if (cls == "WGlobalLight") ++lights;
		if (cls == "WSun") ++suns;
		if (e.IsCamera())
		{
			++cameras;
			if (e.Get("Name") == "DefaultCamera")
				hasDefaultCam = true;
			if (e.Get("Name").size() > 31)
				log->Warn("Camera name over 31 bytes: %s", Cp949ToUtf8(e.Get("Name")).c_str());
		}
		if (e.IsPoint() && e.Get("Name").size() > 31)
			log->Warn("Point name over 31 bytes: %s", Cp949ToUtf8(e.Get("Name")).c_str());
		if (e.IsPoint() && e.Get("Data").size() > 63)
			log->Warn("Point data over 63 bytes: %s", Cp949ToUtf8(e.Get("Data")).c_str());
		if (e.IsArea())
		{
			if (e.Get("Name").size() > 63)
				log->Warn("Area name over 63 bytes: %s", Cp949ToUtf8(e.Get("Name")).c_str());
			WVector lo, hi;
			e.GetBox(&lo, &hi);
			if (lo.x > hi.x || lo.y > hi.y || lo.z > hi.z)
				log->Warn("Area %s has its corners inverted.", Cp949ToUtf8(e.Get("Name")).c_str());
			AreaScript script = ParseAreaScript(e);
			std::string label = Cp949ToUtf8(e.Get("Name"));
			for (size_t p = 0; p < script.problems.size(); ++p)
				log->Warn("%s \"%s\" %s.", cls.c_str(), label.c_str(), script.problems[p].c_str());
			if (script.kind == AREA_NPC && !script.petName.empty() && Resolve(script.petName).empty() && !reported[script.petName])
			{
				reported[script.petName] = true;
				log->Warn("NPC area names %s, which was not found.", Cp949ToUtf8(script.petName).c_str());
			}
			if (script.kind == AREA_FX && !script.fxFile.empty() && Resolve(script.fxFile).empty())
				log->Warn("Effect area names %s, which was not found.", Cp949ToUtf8(script.fxFile).c_str());
			if (script.kind == AREA_CAMERA)
			{
				int cam = wep.FindElement("", script.cameraName);
				if (cam < 0 || !wep.elements[cam].IsCamera())
					log->Warn("Camera area names camera \"%s\", which doesn't exist.", Cp949ToUtf8(script.cameraName).c_str());
			}
		}
		if (e.IsPuppet())
		{
			++puppets;
			std::string file = PathBaseName(e.Get("Filename"));
			if (file.size() > 31)
				log->Warn("Puppet name over 31 bytes: %s", Cp949ToUtf8(file).c_str());
			if (e.Get("Script").size() > 31)
				log->Warn("Puppet script over 31 bytes on %s", Cp949ToUtf8(file).c_str());
			if (!e.IsGround() && !Puppet(file)->loaded)
			{
				++missing;
				if (!reported[file])
				{
					reported[file] = true;
					log->Warn("Puppet not found: %s", Cp949ToUtf8(file).c_str());
				}
			}
			float s = e.GetFloat("Scale", 1.0f);
			if (s <= 0.0f)
				log->Warn("Puppet %s has scale %.2f.", Cp949ToUtf8(file).c_str(), s);
		}
	}
	if (lights != 1)
		log->Warn("%d WGlobalLight points; the client takes its colours from the last one.", lights);
	if (suns > 1)
		log->Info("Note: %d suns; CSky starts the lens flare at the first.", suns);
	if (!hasDefaultCam)
		log->Warn("No DefaultCamera.");

	bool outline = false, autoNode = false;
	for (size_t i = 0; i < wep.nodes.size(); ++i)
	{
		std::string name = wep.nodes[i].Get("Name");
		if (name.size() > 16)
			log->Info("Note: node name over 16 bytes gets cut off in the .gbin: %s", Cp949ToUtf8(name).c_str());
		if (name.find("outline") != std::string::npos)
			outline = true;
		if (name.find("auto") != std::string::npos)
			autoNode = true;
		if (wep.nodes[i].points.size() < 2)
			log->Warn("Node %s has fewer than two points.", Cp949ToUtf8(name).c_str());
	}
	if (!outline)
		log->Warn("No outline node: IsOutOfBound has no boundary to test.");
	if (!autoNode)
		log->Warn("No auto node: the automatic fly-through has no path.");

	// Tee and pins have to stand on the course, and inside the outline.
	for (int s = 0; s < HoleSlotCount(); ++s)
	{
		int ei = FindHoleSlot(s);
		if (ei < 0)
			continue;
		WVector pos = wep.elements[ei].GetVec("Pos");
		if (ground && BasePuppet() && !CastRay(WVector(pos.x, pos.y + 4000.0f, pos.z), WVector(0, -1, 0), true).hit)
			log->Warn("%s is not above the base model.", Cp949ToUtf8(HoleSlotName(s)).c_str());
	}

	if (prop.Loaded() && BasePuppet())
	{
		PuppetAsset* base = BasePuppet();
		for (size_t t = 0; t < base->textureNames.size(); ++t)
		{
			int cls = prop.ClassOfTexture(base->textureNames[t]);
			const XmlNode* c = prop.ClassAt(cls);
			if (!c || c->Attr("name") == "default")
				log->Info("Note: base texture %s has no surface class; it plays as default.", Cp949ToUtf8(base->textureNames[t]).c_str());
		}
		if (!prop.WepEntry(Utf8ToCp949(baseName + ".wep")))
			log->Info("Note: %s.wep has no <wep> entry in the surface table.", baseName.c_str());
	}
	else if (!prop.Loaded())
	{
		log->Info("Note: no surface table was found for this course.");
	}

	log->Info("Map check: %d warning(s) in %d cameras, %d puppets (%d unresolved), %d node lists.",
		log->warnings - warningsBefore, cameras, puppets, missing, (int)wep.nodes.size());
}

// -- undo ---------------------------------------------------------------------

void Course::TakeSnapshot(Snapshot* s, const std::string& label) const
{
	s->label = label;
	s->puppets = wep.puppets;
	s->nodes = wep.nodes;
	s->elements = wep.elements;
}

void Course::RestoreSnapshot(const Snapshot& s)
{
	wep.puppets = s.puppets;
	wep.nodes = s.nodes;
	wep.elements = s.elements;
	++revision;
	dirty = true;
}

void Course::Checkpoint(const std::string& label)
{
	m_undo.push_back(Snapshot());
	TakeSnapshot(&m_undo.back(), label);
	if (m_undo.size() > 64)
		m_undo.erase(m_undo.begin());
	m_redo.clear();
	dirty = true;
}

bool Course::Undo()
{
	if (m_undo.empty())
		return false;
	Snapshot now;
	TakeSnapshot(&now, m_undo.back().label);
	m_redo.push_back(now);
	RestoreSnapshot(m_undo.back());
	m_undo.pop_back();
	return true;
}

bool Course::Redo()
{
	if (m_redo.empty())
		return false;
	Snapshot now;
	TakeSnapshot(&now, m_redo.back().label);
	m_undo.push_back(now);
	RestoreSnapshot(m_redo.back());
	m_redo.pop_back();
	return true;
}
