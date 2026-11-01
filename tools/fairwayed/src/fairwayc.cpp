// SPDX-License-Identifier: MIT
// fairwayc.cpp - batch verification for the FairwayED formats.
//
//   fairwayc roundtrip <dir>        loads and re-saves every .wep .gbin .sgbin .aibin .sbin and
//                                *_property.xml below dir, and compares the bytes
//   fairwayc projects <wep> <data>  compiles leaked projects, compares with what shipped
//   fairwayc imports <data>         reconstructs a project from each .gbin, compiles, compares
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include "areascript.h"
#include "course.h"
#include "fxsim.h"
#include "gbin.h"
#include "wcommon.h"
#include "props.h"
#include "texture.h"
#include "wep.h"

struct Tally { int total, same, differ, failed; std::string firstBad; };

static void Walk(const std::wstring& dir, std::vector<std::wstring>* out)
{
	WIN32_FIND_DATAW fd;
	HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &fd);
	if (h == INVALID_HANDLE_VALUE)
		return;
	do
	{
		if (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L".."))
			continue;
		std::wstring path = dir + L"\\" + fd.cFileName;
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			Walk(path, out);
		else
			out->push_back(path);
	} while (FindNextFileW(h, &fd));
	FindClose(h);
}

static void Note(Tally* t, const std::string& path, const char* what, const std::string& detail)
{
	std::string line = path + ": " + what + (detail.empty() ? "" : " (" + detail + ")");
	if (t->firstBad.empty())
		t->firstBad = line;
}

static int RoundTrip(const std::wstring& root)
{
	std::vector<std::wstring> files;
	Walk(root, &files);
	std::map<std::string, Tally> tally;
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::string path = Narrow(files[i]);
		size_t dot = path.rfind('.');
		if (dot == std::string::npos)
			continue;
		std::string ext = path.substr(dot + 1);
		for (size_t k = 0; k < ext.size(); ++k)
			ext[k] = (char)tolower(ext[k]);
		bool isProp = path.size() > 13 && !_stricmp(path.c_str() + path.size() - 13, "_property.xml");
		if (ext != "wep" && ext != "gbin" && ext != "sgbin" && ext != "aibin" && ext != "sbin" && !isProp)
			continue;
		std::string key = isProp ? "property" : ext;
		Tally& t = tally[key];
		++t.total;
		std::vector<u8> src, out;
		if (!ReadWholeFile(path, &src))
		{
			++t.failed;
			Note(&t, path, "cannot read", "");
			continue;
		}
		std::string error;
		bool ok;
		if (ext == "wep")
		{
			WepFile w;
			ok = w.Parse(std::string(src.begin(), src.end()), &error);
			if (ok)
			{
				std::string s = w.Serialize();
				out.assign(s.begin(), s.end());
			}
		}
		else if (ext == "sbin")
		{
			SbinFile s;
			ok = s.Parse(src, &error);
			if (ok)
				s.Serialize(&out);
		}
		else if (isProp)
		{
			PropFile p;
			ok = p.Parse(std::string(src.begin(), src.end()), &error);
			if (ok)
			{
				std::string s = p.Serialize();
				out.assign(s.begin(), s.end());
			}
		}
		else
		{
			GbinFile g;
			ok = g.Parse(src, ext == "aibin" ? GBIN_AIBIN : ext == "sgbin" ? GBIN_SGBIN : GBIN_GBIN, &error);
			if (ok)
				g.Serialize(&out);
		}
		if (!ok)
		{
			++t.failed;
			Note(&t, path, "parse failed", error);
		}
		else if (out == src)
		{
			++t.same;
		}
		else
		{
			++t.differ;
			size_t at = 0;
			while (at < out.size() && at < src.size() && out[at] == src[at])
				++at;
			char buf[96];
			sprintf_s(buf, "first difference at %u, sizes %u/%u", (unsigned)at, (unsigned)out.size(), (unsigned)src.size());
			Note(&t, path, "differs", buf);
		}
	}
	int bad = 0;
	for (std::map<std::string, Tally>::iterator it = tally.begin(); it != tally.end(); ++it)
	{
		printf("%-9s %6d files  %6d identical  %4d differ  %4d failed\n", it->first.c_str(), it->second.total,
			it->second.same, it->second.differ, it->second.failed);
		if (!it->second.firstBad.empty())
			printf("          first: %s\n", it->second.firstBad.c_str());
		bad += it->second.differ + it->second.failed;
	}
	return bad ? 1 : 0;
}

// -- field-by-field comparison of what the editor compiles with what shipped ------

struct Cmp
{
	int total, same;
	std::string first;
	Cmp() : total(0), same(0) { }
	void Add(bool ok, const char* what, const std::string& where)
	{
		++total;
		if (ok)
			++same;
		else if (first.empty())
			first = where + ": " + what;
	}
};

static bool Near(const float* a, const float* b, int n, float eps)
{
	for (int i = 0; i < n; ++i)
		if (fabs(a[i] - b[i]) > eps)
			return false;
	return true;
}

static std::map<std::string, Cmp> g_cmp;

static void Report()
{
	for (std::map<std::string, Cmp>::iterator it = g_cmp.begin(); it != g_cmp.end(); ++it)
	{
		printf("%-16s %7d / %7d  (%.1f%%)\n", it->first.c_str(), it->second.same, it->second.total,
			it->second.total ? it->second.same * 100.0 / it->second.total : 100.0);
		if (!it->second.first.empty())
			printf("                 first: %s\n", it->second.first.c_str());
	}
}

// Fields a project actually decides. When the shipped hole was rebuilt after the project
// leaked, counts differ and only what lines up by name is compared.
static void CompareHole(const GbinFile& got, const GbinFile& want, const std::string& who)
{
	g_cmp["camera count"].Add(got.cameras.size() == want.cameras.size(), "", who);
	for (size_t i = 0; i < got.cameras.size() && i < want.cameras.size(); ++i)
	{
		const GbinCamera& a = got.cameras[i];
		const GbinCamera& b = want.cameras[i];
		g_cmp["camera"].Add(!strcmp(a.name, b.name) && Near(a.eye, b.eye, 6, 0.011f) && fabs(a.fov - b.fov) < 0.011f, FixedStr(b.name, 32).c_str(), who);
	}
	g_cmp["point count"].Add(got.points.size() == want.points.size(), "", who);
	for (size_t i = 0; i < got.points.size() && i < want.points.size(); ++i)
	{
		const GbinPoint& a = got.points[i];
		const GbinPoint& b = want.points[i];
		char pbuf[200];
		sprintf_s(pbuf, "type %d/%d name %s/%s pos %.2f %.2f %.2f / %.2f %.2f %.2f opt %s/%s", a.type, b.type, a.name, b.name,
			a.pos[0], a.pos[1], a.pos[2], b.pos[0], b.pos[1], b.pos[2], a.option, b.option);
		g_cmp["point"].Add(a.type == b.type && !strcmp(a.name, b.name) && Near(a.pos, b.pos, 3, 0.011f) && !_stricmp(a.option, b.option), pbuf, who);
	}
	g_cmp["area count"].Add(got.areas.size() == want.areas.size(), "", who);
	for (size_t i = 0; i < got.areas.size() && i < want.areas.size(); ++i)
		g_cmp["area"].Add(got.areas[i].type == want.areas[i].type && !strcmp(got.areas[i].name, want.areas[i].name) && Near(got.areas[i].box, want.areas[i].box, 6, 0.011f), want.areas[i].name, who);
	g_cmp["node count"].Add(got.nodes.size() == want.nodes.size(), "", who);
	for (size_t i = 0; i < got.nodes.size() && i < want.nodes.size(); ++i)
	{
		bool ok = got.nodes[i].points.size() == want.nodes[i].points.size() &&
			FixedStr(got.nodes[i].name.data(), 16) == FixedStr(want.nodes[i].name.data(), 16);
		for (size_t k = 0; ok && k < got.nodes[i].points.size(); ++k)
			ok = Near(&got.nodes[i].points[k].x, &want.nodes[i].points[k].x, 3, 0.0011f);
		g_cmp["node"].Add(ok, want.nodes[i].name.c_str(), who);
	}
	bool sameTex = got.textures.size() == want.textures.size();
	std::map<std::string, bool> wantTex;
	for (size_t i = 0; i < want.textures.size(); ++i)
		wantTex[LowerCp949(FixedStr(want.textures[i].data(), 32))] = true;
	int inWant = 0;
	for (size_t i = 0; i < got.textures.size(); ++i)
		if (wantTex.find(LowerCp949(FixedStr(got.textures[i].data(), 32))) != wantTex.end())
			++inWant;
	bool orderOk = sameTex;
	for (size_t i = 0; orderOk && i < got.textures.size(); ++i)
		orderOk = !_stricmp(FixedStr(got.textures[i].data(), 32).c_str(), FixedStr(want.textures[i].data(), 32).c_str());
	g_cmp["texture set"].Add(sameTex && inWant == (int)got.textures.size(), "", who);
	g_cmp["texture order"].Add(orderOk, "", who);
	g_cmp["model count"].Add(got.models.size() == want.models.size(), "", who);
	if (got.models.size() == want.models.size())
	{
		for (size_t i = 0; i < got.models.size(); ++i)
		{
			const GbinModel& a = got.models[i];
			const GbinModel& b = want.models[i];
			if (_stricmp(a.name, b.name))
			{
				g_cmp["model name"].Add(false, b.name, who);
				continue;
			}
			g_cmp["model name"].Add(true, "", who);
			g_cmp["model flag"].Add(a.flag == b.flag, b.name, who);
			g_cmp["model faces"].Add(a.faceNum == b.faceNum, b.name, who);
			g_cmp["model detail"].Add(a.detail == b.detail, b.name, who);
			g_cmp["model script"].Add(!strcmp(a.option, b.option), (std::string(a.option) + " vs " + b.option).c_str(), who);
			char mbuf[300];
			sprintf_s(mbuf, "%s got %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.2f %.2f %.2f want %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.2f %.2f %.2f", b.name,
				a.mat[0], a.mat[1], a.mat[2], a.mat[3], a.mat[4], a.mat[5], a.mat[6], a.mat[7], a.mat[8], a.mat[9], a.mat[10], a.mat[11],
				b.mat[0], b.mat[1], b.mat[2], b.mat[3], b.mat[4], b.mat[5], b.mat[6], b.mat[7], b.mat[8], b.mat[9], b.mat[10], b.mat[11]);
			g_cmp["model matrix"].Add(Near(a.mat, b.mat, 12, 0.015f), mbuf, who);
			g_cmp["model localAabb"].Add(Near(a.localAabb, b.localAabb, 6, 0.05f), b.name, who);
			char buf[220];
			sprintf_s(buf, "%s got %.2f %.2f %.2f / %.2f %.2f %.2f want %.2f %.2f %.2f / %.2f %.2f %.2f", b.name,
				a.sceneAabb[0], a.sceneAabb[1], a.sceneAabb[2], a.sceneAabb[3], a.sceneAabb[4], a.sceneAabb[5],
				b.sceneAabb[0], b.sceneAabb[1], b.sceneAabb[2], b.sceneAabb[3], b.sceneAabb[4], b.sceneAabb[5]);
			g_cmp["model sceneAabb"].Add(Near(a.sceneAabb, b.sceneAabb, 6, 0.05f), buf, who);
		}
	}
	g_cmp["base faces"].Add(got.base.faceNum == want.base.faceNum, "", who);
	g_cmp["base localAabb"].Add(Near(got.base.localAabb, want.base.localAabb, 6, 0.05f), "", who);
	if (want.version == 0x71)
	{
		g_cmp["par"].Add(got.hole.par == want.hole.par, "", who);
		g_cmp["hole slots"].Add(Near(&got.hole.tee[0][0], &want.hole.tee[0][0], 16, 0.011f), "", who);
		g_cmp["base colours"].Add(got.baseColors == want.baseColors, "", who);
	}
}

// Compiles leaked projects and compares them with the .gbin of the same name.
static int CheckProjects(const std::wstring& wepRoot, const std::wstring& dataRoot)
{
	std::vector<std::wstring> files;
	Walk(wepRoot, &files);
	int holes = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::string path = Narrow(files[i]);
		if (LowerCp949(path.substr(path.size() - 4)) != ".wep")
			continue;
		Course c;
		c.SetDataRoot(Narrow(dataRoot));
		CourseLog log;
		if (!c.Open(path, &log) || !c.hasSource || c.source.version != 0x71)
			continue;
		GbinFile out;
		if (!c.Compile(&out, &log))
			continue;
		CompareHole(out, c.source, PathBaseName(path));
		++holes;
	}
	printf("%d projects compiled and compared\n", holes);
	Report();
	return 0;
}

// Reconstructs a project from every shipped .gbin, compiles it, and compares.
static int CheckImports(const std::wstring& dataRoot)
{
	std::vector<std::wstring> files;
	Walk(dataRoot, &files);
	int holes = 0, meshOk = 0, meshBad = 0, smooth = 0, rough = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::string path = Narrow(files[i]);
		if (LowerCp949(path.substr(path.size() - 5)) != ".gbin")
			continue;
		Course c;
		c.SetDataRoot(Narrow(dataRoot));
		CourseLog log;
		if (!c.Open(path, &log))
		{
			printf("failed: %s\n", path.c_str());
			continue;
		}
		if (c.hasSbin && !c.sbin.bodies.empty() && !c.sbin.isNew)
		{
			const SbinBody& b = c.sbin.bodies[0];
			bool ok = b.meshVtx.size() == c.baseMeshes.size();
			for (size_t m = 0; ok && m < b.meshVtx.size(); ++m)
				ok = b.meshVtx[m] == (int)c.baseMeshes[m].vertexCorner.size() && b.meshIdx[m] == (int)c.baseMeshes[m].index.size();
			if (ok)
				++meshOk;
			else
			{
				++meshBad;
				if (meshBad <= 3)
					printf("mesh mismatch: %s (%d/%d meshes)\n", path.c_str(), (int)b.meshVtx.size(), (int)c.baseMeshes.size());
			}
		}
		// One mesh vertex has one colour, so a right mesh order shows as agreement.
		PuppetAsset* base = c.BasePuppet();
		if (base && c.cornerColor.size() == base->pos.size())
		{
			std::map<int, u32> colorAt;
			bool agree = true;
			for (size_t m = 0; agree && m < c.baseMeshes.size(); ++m)
			{
				colorAt.clear();
				for (size_t f = 0; agree && f < c.baseMeshes[m].faces.size(); ++f)
				{
					for (int k = 0; k < 3; ++k)
					{
						int vert = c.baseMeshes[m].index[f * 3 + k];
						u32 col = c.cornerColor[c.baseMeshes[m].faces[f] * 3 + k];
						std::map<int, u32>::iterator it = colorAt.find(vert);
						if (it == colorAt.end())
							colorAt[vert] = col;
						else if (it->second != col)
							agree = false;
					}
				}
			}
			agree ? ++smooth : ++rough;
		}
		if (c.source.version != 0x71)
			continue;
		GbinFile out;
		if (!c.Compile(&out, &log))
			continue;
		CompareHole(out, c.source, PathBaseName(path));
		++holes;
	}
	printf("%d holes reconstructed, compiled and compared\n", holes);
	printf("mesh split agrees with .sbin in %d holes, not in %d\n", meshOk, meshBad);
	printf("colour order consistent in %d holes, not in %d\n", smooth, rough);
	Report();
	return 0;
}

// Runs every shipped WFxArea through fxsim headless, and every area script through the
// decoder, and says what could not be used.
static int CheckEffects(const std::wstring& dataRoot)
{
	std::vector<std::wstring> files;
	Walk(dataRoot, &files);
	int areas = 0, withParticles = 0, holes = 0;
	std::map<std::string, int> problems, scriptProblems, kinds;
	Course c;
	c.SetDataRoot(Narrow(dataRoot));
	CourseFxHost host(&c);
	FxBox box(&host);
	std::map<std::string, bool> emits;
	std::vector<WMatrix> world(1);
	for (size_t i = 0; i < files.size(); ++i)
	{
		std::string path = Narrow(files[i]);
		if (LowerCp949(path.substr(path.size() - 5)) != ".gbin")
			continue;
		GbinFile g;
		std::string error;
		if (!g.Load(path, &error))
			continue;
		++holes;
		c.wep.Clear();
		c.ImportGbin(g);
		for (size_t e = 0; e < c.wep.elements.size(); ++e)
		{
			const WepElement& el = c.wep.elements[e];
			if (!el.IsArea())
				continue;
			AreaScript s = ParseAreaScript(el);
			char key[32];
			sprintf_s(key, "kind %d", (int)s.kind);
			++kinds[key];
			for (size_t p = 0; p < s.problems.size(); ++p)
				++scriptProblems[s.problems[p]];
			if (s.kind != AREA_FX || s.fxFile.empty())
				continue;
			++areas;
			std::string lower = LowerCp949(s.fxFile);
			if (emits.find(lower) == emits.end())
			{
				box.ClearAll(false);
				FxAttacher a;
				a.count = 1;
				a.bone[0] = 0;
				bool ok = box.Attach(a, s.fxFile, false);
				int peak = 0;
				for (int step = 0; ok && step < 400; ++step)
				{
					box.m_camPos = WVector();
					box.Process(FxBox::FRAME_TIME, world);
					peak = max(peak, box.ParticleCount());
				}
				emits[lower] = peak > 0;
				if (!ok)
					++problems["could not attach " + lower];
				else if (!peak)
					++problems["no particles in 400 frames: " + lower];
			}
			if (emits[lower])
				++withParticles;
		}
	}
	printf("%d holes, %d effect areas, %d distinct files, %d areas with particles\n", holes, areas, (int)emits.size(), withParticles);
	for (std::map<std::string, int>::iterator k = kinds.begin(); k != kinds.end(); ++k)
		printf("  %s: %d\n", k->first.c_str(), k->second);
	for (std::map<std::string, int>::iterator k = scriptProblems.begin(); k != scriptProblems.end(); ++k)
		printf("  script: %s (%d)\n", k->first.c_str(), k->second);
	for (std::map<std::string, int>::iterator k = problems.begin(); k != problems.end(); ++k)
		printf("  %s\n", k->first.c_str());
	for (size_t p = 0; p < box.m_problems.size(); ++p)
		printf("  fxsim: %s\n", Cp949ToUtf8(box.m_problems[p]).c_str());
	return 0;
}

int wmain(int argc, wchar_t** argv)
{
	SetConsoleOutputCP(CP_UTF8);
	TextureStartup();
	if (argc >= 3 && !wcscmp(argv[1], L"roundtrip"))
		return RoundTrip(argv[2]);
	if (argc >= 4 && !wcscmp(argv[1], L"projects"))
		return CheckProjects(argv[2], argv[3]);
	if (argc >= 3 && !wcscmp(argv[1], L"imports"))
		return CheckImports(argv[2]);
	if (argc >= 3 && !wcscmp(argv[1], L"effects"))
		return CheckEffects(argv[2]);
	printf("usage: fairwayc roundtrip <dir>\n"
		"       fairwayc projects <wep dir> <data dir>\n"
		"       fairwayc imports <data dir>\n"
		"       fairwayc effects <data dir>\n");
	return 2;
}
