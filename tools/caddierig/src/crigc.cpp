// SPDX-License-Identifier: MIT
// crigc.cpp - command line batch compiler and regression tool.
//
//   crigc build   project.pmp out.pet [/v 1.3] [/k pet|mpet|bpet|apet]
//   crigc info    file.pet
//   crigc compare mine.pet theirs.pet [tolerance]
//   crigc check   project.pmp
//   crigc verify  directory            byte-exact round trip of every puppet under it
//   crigc verifypmp directory          byte-exact round trip of every PMP under it
//   crigc posetest  directory          open and pose every puppet under it
//   crigc normalcheck directory        compare normals with triangle winding, rest and posed
//   crigc fxcheck   directory          run every effect and resolve every script under it
//   crigc anicheck  motion.ani shipped.apet motionname
//   crigc metacheck project.pmp shipped.apet|bpet
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include "audio.h"
#include "compile.h"
#include "texture.h"
#include "wardrobe.h"
#include "document.h"
#include "fxsim.h"
#include "puppetscript.h"
#include "petpose.h"

static void PrintUtf8(const std::string& s)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), 0, 0);
	std::wstring w(n, L'\0');
	if (n)
		MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &w[0], n);
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode, written;
	if (GetConsoleMode(out, &mode))
		WriteConsoleW(out, w.data(), (DWORD)w.size(), &written, 0);
	else
		WriteFile(out, s.data(), (DWORD)s.size(), &written, 0);
}

static void Print(const char* fmt, ...)
{
	char buf[2048];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, fmt, va);
	va_end(va);
	PrintUtf8(buf);
}

static void PrintLog(const BuildLog& log)
{
	for (size_t i = 0; i < log.lines.size(); ++i)
		PrintUtf8(log.lines[i] + "\n");
}

static std::string ToUtf8(const wchar_t* w)
{
	int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
	std::string s(n > 0 ? n - 1 : 0, '\0');
	if (n > 1)
		WideCharToMultiByte(CP_UTF8, 0, w, -1, &s[0], n, 0, 0);
	return s;
}

static void WalkDir(const std::wstring& dir, std::vector<std::string>* out, const wchar_t** exts, int numExts)
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
		{
			WalkDir(path, out, exts, numExts);
			continue;
		}
		const wchar_t* dot = wcsrchr(fd.cFileName, L'.');
		if (!dot)
			continue;
		for (int i = 0; i < numExts; ++i)
			if (!_wcsicmp(dot, exts[i]))
				out->push_back(ToUtf8(path.c_str()));
	} while (FindNextFileW(h, &fd));
	FindClose(h);
}

static bool ReadWhole(const char* path, ByteBuf* data)
{
	FILE* f = OpenUtf8(path, L"rb");
	if (!f)
		return false;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	data->resize(size);
	if (size)
		fread(&(*data)[0], 1, size, f);
	fclose(f);
	return true;
}

static int CmdVerify(const std::wstring& dir)
{
	static const wchar_t* exts[] = { L".pet", L".mpet", L".bpet", L".apet" };
	std::vector<std::string> files;
	WalkDir(dir, &files, exts, 4);
	int ok = 0, parseFail = 0, byteFail = 0, trailing = 0, outOfRange = 0, damaged = 0, smtlFilled = 0, triExtraNonZero = 0, multiUv = 0;
	unsigned long long bytes = 0;
	int counts[4] = { 0, 0, 0, 0 };
	for (size_t i = 0; i < files.size(); ++i)
	{
		ByteBuf data, out;
		ReadWhole(files[i].c_str(), &data);
		bytes += data.size();
		PetFile pet;
		std::string error;
		PetKind kind = PetKindFromPath(files[i].c_str());
		counts[kind]++;
		if (!pet.LoadBuffer(data.empty() ? 0 : &data[0], data.size(), kind, &error))
		{
			if (parseFail++ < 20)
				Print("PARSE %s: %s\n", files[i].c_str(), error.c_str());
			continue;
		}
		// A clean parse consumes every known chunk exactly and stays in range.
		for (size_t c = 0; c < pet.chunks.size(); ++c)
		{
			const PetChunk& pc = pet.chunks[c];
			if (pc.id[0] && !pc.raw.empty() && strcmp(pc.id, "EXTR") && strcmp(pc.id, "SMTL") && strcmp(pc.id, "VERS"))
			{
				if (trailing++ < 15)
					Print("TAIL  %s: %s has %d unparsed bytes (v%d.%d)\n", files[i].c_str(), pc.id, (int)pc.raw.size(), pet.verMajor, pet.verMinor);
			}
			if (!pc.id[0])
				++damaged;
		}
		for (size_t c = 0; c < pet.chunks.size(); ++c)
			if (!strcmp(pet.chunks[c].id, "SMTL") && pet.chunks[c].raw.size() > 4)
				++smtlFilled;
		for (size_t t = 0; t < pet.triExtra.size(); ++t)
			if (pet.triExtra[t])
			{
				++triExtraNonZero;
				break;
			}
		for (size_t c = 0; c < pet.cornerUvCount.size(); ++c)
			if (pet.cornerUvCount[c] != 1)
			{
				++multiUv;
				break;
			}
		{
			bool oob = false;
			size_t maxIndex = 0;
			for (size_t p = 0; p < pet.points.size(); ++p)
				if (pet.points[p].pos > maxIndex)
					maxIndex = pet.points[p].pos;
			oob = !pet.points.empty() && maxIndex >= pet.vertices.size();
			for (size_t v = 0; v < pet.vertices.size() && !oob && !pet.bones.empty(); ++v)
				for (size_t b = 0; b < pet.vertices[v].blends.size(); ++b)
					if (pet.vertices[v].blends[b].bone >= (int)pet.bones.size())
						oob = true;
			for (size_t a = 0; a < pet.anims.size() && !oob; ++a)
				if (pet.anims[a].bone >= (int)pet.bones.size())
					oob = true;
			for (size_t t = 0; t < pet.triTextures.size() && !oob; ++t)
				if (pet.triTextures[t] >= pet.textures.size() && !pet.textures.empty())
					oob = true;
			if (oob && outOfRange++ < 15)
				Print("RANGE %s (v%d.%d)\n", files[i].c_str(), pet.verMajor, pet.verMinor);
		}
		pet.SaveBuffer(&out);
		if (out != data)
		{
			if (byteFail++ < 20)
			{
				size_t at = 0;
				while (at < out.size() && at < data.size() && out[at] == data[at])
					++at;
				Print("DIFF  %s: %d vs %d bytes, first at 0x%x\n", files[i].c_str(), (int)out.size(), (int)data.size(), (int)at);
			}
			continue;
		}
		++ok;
	}
	Print("%d files (%d pet, %d mpet, %d bpet, %d apet), %.1f MB: %d identical, %d parse failures, %d byte differences\n",
		(int)files.size(), counts[PET_FULL], counts[PET_MESH], counts[PET_BONE], counts[PET_ANIM], bytes / 1048576.0, ok, parseFail, byteFail);
	Print("strictness: %d chunks with unparsed bytes, %d files with out-of-range indices, %d files with undecodable tails\n", trailing, outOfRange, damaged);
	Print("oddities: %d files with SMTL entries, %d with a nonzero second triangle byte, %d with a UV count other than 1\n", smtlFilled, triExtraNonZero, multiUv);
	return parseFail || byteFail ? 1 : 0;
}

static int CmdVerifyPmp(const std::wstring& dir)
{
	static const wchar_t* exts[] = { L".pmp" };
	std::vector<std::string> files;
	WalkDir(dir, &files, exts, 1);
	int bad = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		ByteBuf data;
		ReadWhole(files[i].c_str(), &data);
		PmpFile pmp;
		std::string error;
		if (!pmp.Load(files[i].c_str(), &error))
		{
			Print("PARSE %s: %s\n", files[i].c_str(), error.c_str());
			++bad;
			continue;
		}
		std::string s = pmp.Serialize();
		if (pmp.cp949)
			s = Utf8ToCp949(s);
		bool same = s.size() == data.size() && (s.empty() || !memcmp(s.data(), &data[0], s.size()));
		// The same project must also survive a trip through CP949, the format the tool wrote.
		PmpFile again;
		bool recoded = again.Parse(Cp949ToUtf8(Utf8ToCp949(pmp.Serialize())), &error) && again.Serialize() == pmp.Serialize();
		int blocks[5] = { 0, 0, 0, 0, 0 };
		for (size_t k = 0; k < pmp.blocks.size(); ++k)
			blocks[pmp.blocks[k].kind]++;
		Print("%s %s: %s %s  tex %d box %d motion %d script %d other %d\n", same ? "same" : "DIFF", recoded ? "cp949-ok" : "CP949-LOSSY",
			pmp.cp949 ? "cp949" : "utf-8", PathBaseName(files[i]).c_str(), blocks[0], blocks[1], blocks[2], blocks[3], blocks[4]);
		if (!same || !recoded)
			++bad;
	}
	return bad ? 1 : 0;
}

static int CmdInfo(const char* path)
{
	PetFile pet;
	std::string error;
	if (!pet.Load(path, &error))
	{
		Print("%s\n", error.c_str());
		return 1;
	}
	Print("%s v%d.%d\n", PetKindExt(pet.kind), pet.verMajor, pet.verMinor);
	for (size_t i = 0; i < pet.chunks.size(); ++i)
		Print("  %s\n", pet.chunks[i].id);
	Print("bones %d, anim tracks %d, vertices %d, triangles %d, textures %d, face anims %d, frames %d, motions %d, boxes %d, skins %d\n",
		(int)pet.bones.size(), (int)pet.anims.size(), (int)pet.vertices.size(), (int)pet.points.size() / 3, (int)pet.textures.size(),
		(int)pet.faces.size(), (int)pet.frames.size(), (int)pet.motions.size(), (int)pet.colls.size(), (int)pet.skins.size());
	return 0;
}

// Compiles one .Ani onto a shipped skeleton and samples it against the shipped
// .apet over the motion's frames.
static int CmdAniCheck(const char* aniPath, const char* apetPath, const char* motionName)
{
	PetFile apet;
	std::string error;
	if (!apet.Load(apetPath, &error))
	{
		Print("%s\n", error.c_str());
		return 1;
	}
	const PetMotion* motion = 0;
	std::string want = Utf8ToCp949(motionName);
	for (size_t i = 0; i < apet.motions.size(); ++i)
		if (apet.motions[i].name == want)
			motion = &apet.motions[i];
	if (!motion)
	{
		Print("motion not found\n");
		return 1;
	}
	PetFile mine;
	mine.kind = PET_ANIM;
	mine.verMajor = 1;
	mine.verMinor = 3;
	mine.bones = apet.bones;
	BuildLog log;
	int frames = (int)(motion->endFrame - motion->startFrame);
	if (!MergeMotion(aniPath, (int)motion->startFrame, 0, frames, &mine, &log))
	{
		PrintLog(log);
		return 1;
	}
	FinishAnimation(&mine, 0.0002f);
	PrintLog(log);
	float maxPos = 0.0f, maxRot = 0.0f;
	std::string maxPosBone, maxRotBone;
	int samples = 0, keysMine = 0, keysTheirs = 0;
	for (size_t i = 0; i < mine.anims.size(); ++i)
	{
		const PetAnim& a = mine.anims[i];
		const PetAnim* b = 0;
		for (size_t k = 0; k < apet.anims.size(); ++k)
			if (apet.anims[k].bone == a.bone)
				b = &apet.anims[k];
		if (!b)
			continue;
		keysMine += (int)(a.pos.size() + a.rot.size());
		for (size_t k = 0; k < b->pos.size(); ++k)
			if (b->pos[k].time >= motion->startFrame / 30.0f - 0.001f && b->pos[k].time <= motion->endFrame / 30.0f + 0.001f)
				++keysTheirs;
		for (size_t k = 0; k < b->rot.size(); ++k)
			if (b->rot[k].time >= motion->startFrame / 30.0f - 0.001f && b->rot[k].time <= motion->endFrame / 30.0f + 0.001f)
				++keysTheirs;
		for (int f = 0; f <= frames; ++f)
		{
			float t = (motion->startFrame + f) / 30.0f;
			WVector p1 = EvalKeyPos(a.pos, t, WVector());
			WVector p2 = EvalKeyPos(b->pos, t, p1);
			float dp = (p1 - p2).Length();
			if (dp > maxPos)
			{
				maxPos = dp;
				maxPosBone = mine.bones[a.bone].name;
			}
			WQuat q1 = EvalKeyRot(a.rot, t, WQuat());
			WQuat q2 = EvalKeyRot(b->rot, t, q1);
			float dot = (float)fabs(q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
			float dr = 2.0f * (float)acos(dot > 1.0f ? 1.0f : dot);
			if (dr > maxRot)
			{
				maxRot = dr;
				maxRotBone = mine.bones[a.bone].name;
			}
			++samples;
		}
	}
	Print("%d bone-frames sampled over %d frames; worst position error %.5f (%s), worst rotation error %.3f deg (%s); keys %d mine / %d shipped\n",
		samples, frames, maxPos, Cp949ToUtf8(maxPosBone).c_str(), maxRot * 57.29578f, Cp949ToUtf8(maxRotBone).c_str(), keysMine, keysTheirs);
	return 0;
}

// Compares PMP blocks with the chunks that shipped from them.
static int CmdMetaCheck(const char* pmpPath, const char* petPath)
{
	PmpFile pmp;
	PetFile ship, mine;
	std::string error;
	if (!pmp.Load(pmpPath, &error) || !ship.Load(petPath, &error))
	{
		Print("%s\n", error.c_str());
		return 1;
	}
	ApplyBoxes(pmp, &mine);
	ApplyMotions(pmp, &mine);
	ApplyScripts(pmp, &mine);
	BuildLog log;
	if (ship.kind == PET_BONE)
		mine.motions.clear(), mine.frames.clear();
	else
		mine.colls.clear();

	int exact = 0, frameMoved = 0, unmatched = 0, fieldDiff = 0;
	for (size_t i = 0; i < mine.motions.size(); ++i)
	{
		const PetMotion& m = mine.motions[i];
		const PetMotion* s = 0;
		for (size_t k = 0; k < ship.motions.size(); ++k)
			if (ship.motions[k].name == m.name)
				s = &ship.motions[k];
		if (!s)
			++unmatched;
		else if (m.next != s->next || m.option != s->option || m.applyBone != s->applyBone || fabs(m.optionValue - s->optionValue) > 0.001f)
			++fieldDiff;
		else if (m.startFrame != s->startFrame || m.endFrame != s->endFrame)
			++frameMoved;
		else
			++exact;
	}
	log.Info("MOTI: %d in PMP, %d shipped; %d exact, %d same fields but moved on the timeline, %d other field diffs, %d not shipped",
		(int)mine.motions.size(), (int)ship.motions.size(), exact, frameMoved, fieldDiff, unmatched);
	int found = 0;
	for (size_t i = 0; i < mine.frames.size(); ++i)
		for (size_t k = 0; k < ship.frames.size(); ++k)
			if (ship.frames[k].script == mine.frames[i].script)
			{
				++found;
				break;
			}
	log.Info("FRAM: %d in PMP, %d shipped; %d PMP scripts appear verbatim in the shipped file", (int)mine.frames.size(), (int)ship.frames.size(), found);
	int boxExact = 0, boxFound = 0;
	for (size_t i = 0; i < mine.colls.size(); ++i)
		for (size_t k = 0; k < ship.colls.size(); ++k)
			if (ship.colls[k].name == mine.colls[i].name)
			{
				++boxFound;
				bool same = ship.colls[k].bone == mine.colls[i].bone;
				for (int c = 0; c < 6; ++c)
					same = same && fabs(ship.colls[k].box[c] - mine.colls[i].box[c]) < 0.00001f;
				if (same)
					++boxExact;
				break;
			}
	log.Info("COLL: %d in PMP, %d shipped; %d matched by name, %d numerically exact", (int)mine.colls.size(), (int)ship.colls.size(), boxFound, boxExact);
	PrintLog(log);
	return 0;
}

// Opens every puppet the way the application does and poses it at a few times.
static int CmdPoseTest(const std::wstring& dir)
{
	static const wchar_t* exts[] = { L".pet", L".mpet", L".bpet", L".apet" };
	std::vector<std::string> files;
	WalkDir(dir, &files, exts, 4);
	int bad = 0;
	long long nan = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		if (getenv("PMGC_TRACE"))
			Print("%s\n", files[i].c_str());
		Document doc;
		BuildLog log;
		if (!doc.OpenPuppet(files[i], &log))
		{
			++bad;
			continue;
		}
		float last = doc.LastFrame() / 30.0f;
		for (int s = 0; s < 4; ++s)
			doc.Pose(last * s / 3.0f);
		for (size_t p = 0; p < doc.parts.size(); ++p)
			for (size_t v = 0; v < doc.parts[p]->skinnedPos.size(); ++v)
				if (doc.parts[p]->skinnedPos[v].x != doc.parts[p]->skinnedPos[v].x)
					++nan;
		if (i % 1000 == 0)
			Print("%d...\n", (int)i);
	}
	Print("%d puppets opened and posed, %d failed to open, %lld NaN vertices\n", (int)files.size(), bad, nan);
	return bad ? 1 : 0;
}

// Do the file's normals agree with the outward side of their triangles, posed or not?
// The average normal of a face also shows which way a character looks.
static int CmdNormalCheck(const std::wstring& dir)
{
	static const wchar_t* exts[] = { L".pet", L".mpet", L".bpet" };
	std::vector<std::string> files;
	WalkDir(dir, &files, exts, 3);
	long long agree[2] = { 0, 0 }, against[2] = { 0, 0 };
	WVector facing;
	int faces = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		Document doc;
		BuildLog log;
		if (!doc.OpenPuppet(files[i], &log))
			continue;
		for (int pass = 0; pass < 2; ++pass)
		{
			doc.Pose(pass ? doc.LastFrame() / 60.0f : 0.0f);
			for (size_t p = 0; p < doc.parts.size(); ++p)
			{
				const MeshPart& part = *doc.parts[p];
				if (part.skinnedNormal.size() != part.cornerVertex.size())
					continue;
				bool face = !pass && part.name.find("_fc_def") != std::string::npos;
				for (size_t c = 0; c + 2 < part.cornerVertex.size(); c += 3)
				{
					const WVector& a = part.skinnedPos[part.cornerVertex[c]];
					WVector geo = (part.skinnedPos[part.cornerVertex[c + 1]] - a).Cross(part.skinnedPos[part.cornerVertex[c + 2]] - a);
					WVector n = part.skinnedNormal[c] + part.skinnedNormal[c + 1] + part.skinnedNormal[c + 2];
					float d = n.Dot(geo);
					if (d > 0.0f)
						++agree[pass];
					else if (d < 0.0f)
						++against[pass];
					if (face)
						facing = facing + geo;
				}
				faces += face;
			}
		}
	}
	Print("rest pose: %lld triangles agree with their normals, %lld against\n", agree[0], against[0]);
	Print("mid motion: %lld agree, %lld against\n", agree[1], against[1]);
	facing.Normalize();
	Print("%d character faces look along (%.2f, %.2f, %.2f)\n", faces, facing.x, facing.y, facing.z);
	return 0;
}

// Runs every shipped effect headless and checks every shipped script resolves.
class CheckHost : public FxHost
{
public:
	Document* doc;
	int sounds;
	std::vector<std::string> missing;
	virtual std::string ResolveAsset(const std::string& name) { return doc->ResolveAsset(name); }
	virtual void PlaySound(const std::string& name, float)
	{
		++sounds;
		if (doc->ResolveAsset(name + ".wav").empty())
			missing.push_back(name);
	}
	virtual FxPetModel* LoadPetModel(const std::string& name) { return (FxPetModel*)doc->LoadFxModel(name); }
};

static int CmdFxCheck(const std::wstring& dir)
{
	Document doc;
	doc.SetDataRoot(ToUtf8(dir.c_str()));
	CheckHost host;
	host.doc = &doc;
	host.sounds = 0;

	static const wchar_t* fxExts[] = { L".spr", L".seq" };
	std::vector<std::string> files;
	WalkDir(dir, &files, fxExts, 2);
	std::vector<WMatrix> world;
	int ok = 0, failed = 0, noParticles = 0;
	long long nan = 0, peak = 0;
	FxBox box(&host);
	for (size_t i = 0; i < files.size(); ++i)
	{
		FxAttacher a;
		a.hasPos[0] = true;
		if (!box.Attach(a, Utf8ToCp949(PathBaseName(files[i])), false))
		{
			++failed;
			continue;
		}
		int most = 0;
		// Three seconds at 30 fps, past the generation phase of nearly all of them.
		for (int f = 0; f < 90; ++f)
		{
			box.Process(1000.0f / 30.0f, world);
			int n = box.ParticleCount();
			if (n > most)
				most = n;
			for (std::list<FxSpray*>::iterator s = box.m_spray.begin(); s != box.m_spray.end(); ++s)
				for (std::list<FxParticle*>::iterator p = (*s)->m_particle.begin(); p != (*s)->m_particle.end(); ++p)
					if ((*p)->pos.x != (*p)->pos.x || (*p)->size != (*p)->size)
						++nan;
		}
		if (most > peak)
			peak = most;
		if (!most)
			++noParticles;
		box.ClearAll(false);
		++ok;
	}
	Print("effects: %d files run for 3s, %d failed to start, %d emitted nothing, peak %lld particles, %lld NaN particle-frames, %d sounds played (%d missing)\n",
		ok, failed, noParticles, peak, nan, host.sounds, (int)host.missing.size());
	Print("problems reported by the effect system: %d\n", (int)box.m_problems.size());
	for (size_t i = 0; i < box.m_problems.size() && i < 25; ++i)
		Print("  %s\n", Cp949ToUtf8(box.m_problems[i]).c_str());

	static const wchar_t* petExts[] = { L".pet", L".mpet", L".bpet", L".apet" };
	std::vector<std::string> pets;
	WalkDir(dir, &pets, petExts, 4);
	long long scripts = 0, commands = 0, unknown = 0, sndOk = 0, sndMiss = 0, fxOk = 0, fxMiss = 0, boxFx = 0, boxMiss = 0;
	std::map<std::string, int> unknownTokens, missSnd, missFx;
	for (size_t i = 0; i < pets.size(); ++i)
	{
		PetFile pet;
		std::string error;
		if (!pet.Load(pets[i].c_str(), &error))
			continue;
		for (size_t f = 0; f < pet.frames.size(); ++f)
		{
			++scripts;
			std::vector<ScriptCommand> cmds;
			ParsePuppetScript(pet.frames[f].script, &cmds);
			for (size_t c = 0; c < cmds.size(); ++c)
			{
				++commands;
				if (!IsKnownToken(cmds[c].token))
				{
					++unknown;
					unknownTokens[cmds[c].token]++;
				}
				if (IsSoundCommand(cmds[c]))
				{
					if (doc.ResolveAsset(cmds[c].args[0] + ".wav").empty())
					{
						++sndMiss;
						missSnd[cmds[c].args[0]]++;
					}
					else
						++sndOk;
				}
				ScriptFx fx;
				if (ParseFxCommand(cmds[c], &fx) && fx.file[0] != '*')
				{
					if (doc.ResolveAsset(fx.file).empty())
					{
						++fxMiss;
						missFx[fx.file]++;
					}
					else
						++fxOk;
				}
			}
		}
		for (size_t b = 0; b < pet.colls.size(); ++b)
		{
			std::vector<BoxFx> list;
			ParseBoxScript(pet.colls[b].script0.c_str() + std::string(" ") + pet.colls[b].script1.c_str(), &list);
			for (size_t k = 0; k < list.size(); ++k)
			{
				if (doc.ResolveAsset(list[k].file).empty())
				{
					++boxMiss;
					missFx[list[k].file]++;
				}
				else
					++boxFx;
			}
		}
	}
	Print("scripts: %lld messages, %lld commands, %lld with unknown tokens\n", scripts, commands, unknown);
	Print("  *snd: %lld found, %lld missing; *fx: %lld found, %lld missing; box fx: %lld found, %lld missing\n", sndOk, sndMiss, fxOk, fxMiss, boxFx, boxMiss);
	for (std::map<std::string, int>::iterator u = unknownTokens.begin(); u != unknownTokens.end(); ++u)
		Print("  unknown token %s x%d\n", Cp949ToUtf8(u->first).c_str(), u->second);
	int shown = 0;
	for (std::map<std::string, int>::iterator u = missSnd.begin(); u != missSnd.end() && shown < 12; ++u, ++shown)
		Print("  missing sound %s x%d\n", Cp949ToUtf8(u->first).c_str(), u->second);
	shown = 0;
	for (std::map<std::string, int>::iterator u = missFx.begin(); u != missFx.end() && shown < 12; ++u, ++shown)
		Print("  missing effect %s x%d\n", Cp949ToUtf8(u->first).c_str(), u->second);
	return failed ? 1 : 0;
}

// Checks every .wav decodes, and plays one of each format at a volume too low to hear.
static int CmdAudioCheck(const std::wstring& dir)
{
	static const wchar_t* exts[] = { L".wav" };
	std::vector<std::string> files;
	WalkDir(dir, &files, exts, 1);
	std::map<int, int> formats;
	std::map<int, std::string> sample;
	int bad = 0;
	for (size_t i = 0; i < files.size(); ++i)
	{
		ByteBuf d;
		ReadWhole(files[i].c_str(), &d);
		int tag = -1;
		for (size_t p = 12; p + 10 <= d.size();)
		{
			unsigned int len;
			memcpy(&len, &d[p + 4], 4);
			if (!memcmp(&d[p], "fmt ", 4))
			{
				tag = d[p + 8] | (d[p + 9] << 8);
				break;
			}
			p += 8 + len + (len & 1);
		}
		formats[tag]++;
		sample[tag] = files[i];
		if (!AudioCanPlay(files[i]))
		{
			if (bad++ < 10)
				Print("CANNOT PLAY %s (format %d)\n", files[i].c_str(), tag);
		}
	}
	Print("%d wav files, %d the system cannot play\n", (int)files.size(), bad);
	for (std::map<int, int>::iterator f = formats.begin(); f != formats.end(); ++f)
	{
		bool played = AudioPlay(sample[f->first], 0.0002f);
		DWORD start = GetTickCount();
		while (AudioVoices() > 0 && GetTickCount() - start < 15000)
		{
			Sleep(50);
			AudioUpdate();
		}
		Print("  format %d: %d files; test voice %s\n", f->first, f->second, !played ? "FAILED" : (AudioVoices() ? "still running after 15s" : "ran to completion"));
		AudioStopAll();
	}
	return bad ? 1 : 0;
}

static std::string ResolveForCheck(const std::string& name, void* doc)
{
	return ((Document*)doc)->ResolveAsset(name);
}

// Resolves every distinct *ptex in the data and reads its replacement rectangle.
static int CmdPtexCheck(const std::wstring& dir)
{
	TextureStartup();
	Document doc;
	doc.SetDataRoot(ToUtf8(dir.c_str()));
	static const wchar_t* exts[] = { L".pet", L".apet" };
	std::vector<std::string> pets;
	WalkDir(dir, &pets, exts, 2);
	std::map<std::string, int> done, whyCount;
	std::map<std::string, std::string> whyExample;
	int ok = 0, uses = 0, noTarget = 0;
	long long area = 0;
	for (size_t i = 0; i < pets.size(); ++i)
	{
		PetFile pet;
		std::string error;
		if (!pet.Load(pets[i].c_str(), &error) || pet.frames.empty())
			continue;
		PetFile face;
		bool avatar = pet.kind == PET_ANIM;
		if (avatar)
		{
			// Same choice as the application: the character's default face part.
			std::string stem = PathBaseName(pets[i]);
			size_t us = stem.find('_');
			std::string path = PathJoin(PathDir(pets[i]), stem.substr(0, us + 1) + "fc_def.mpet");
			if (!face.Load(path.c_str(), &error))
				face.Clear();
		}
		for (size_t f = 0; f < pet.frames.size(); ++f)
		{
			std::vector<ScriptCommand> cmds;
			ParsePuppetScript(pet.frames[f].script, &cmds);
			for (size_t c = 0; c < cmds.size(); ++c)
			{
				if (_stricmp(cmds[c].token.c_str(), "*ptex(") || cmds[c].args.empty() || cmds[c].args[0].empty())
					continue;
				size_t a = cmds[c].args[0][0] == '@' ? 1 : 0;
				if (a + 1 >= cmds[c].args.size())
					continue;
				++uses;
				std::string dst;
				if (avatar)
					dst = face.textures.empty() ? std::string() : face.textures[0].Name();
				else
				{
					for (size_t n = 0; n < pet.faces.size(); ++n)
						if (!_stricmp(pet.faces[n].Name().c_str(), cmds[c].args[a].c_str()))
							dst = pet.faces[n].File();
				}
				if (dst.empty())
				{
					++noTarget;
					continue;
				}
				std::string key = dst + "|" + cmds[c].args[a + 1];
				if (done.count(key))
					continue;
				done[key] = 1;
				TexImage base;
				std::string why;
				TexPart part;
				if (!LoadTexImage(doc.ResolveAsset(dst), &base))
					why = "target texture missing";
				else if (LoadTexPart(dst, cmds[c].args[a + 1], base.w, base.h, ResolveForCheck, &doc, &part, &why))
				{
					++ok;
					area += part.width * part.height;
					continue;
				}
				whyCount[why]++;
				whyExample[why] = Cp949ToUtf8(key);
			}
		}
	}
	Print("*ptex: %d uses, %d with no target; %d distinct texture+image pairs, %d usable, average patch %d pixels\n",
		uses, noTarget, (int)done.size(), ok, ok ? (int)(area / ok) : 0);
	for (std::map<std::string, int>::iterator w = whyCount.begin(); w != whyCount.end(); ++w)
		Print("  %s: %d, e.g. %s\n", w->first.c_str(), w->second, whyExample[w->first].c_str());
	TextureShutdown();
	return 0;
}

// Wears every part of every character in turn and poses the result.
static int CmdWardrobeCheck(const std::wstring& dir)
{
	static const wchar_t* exts[] = { L".bpet" };
	std::vector<std::string> bpets;
	WalkDir(dir, &bpets, exts, 1);
	int chars = 0, parts = 0, extras = 0, failed = 0, notReplaced = 0, nan = 0;
	std::map<std::string, int> hideKnown, hideUnknown;
	std::vector<WMatrix> unused;
	for (size_t b = 0; b < bpets.size(); ++b)
	{
		std::string stem = PathBaseName(bpets[b]);
		size_t us = stem.find('_');
		if (us == std::string::npos || stem.find("_def") == std::string::npos)
			continue;
		Document doc;
		BuildLog log;
		if (!doc.OpenPuppet(bpets[b], &log))
			continue;
		Wardrobe w;
		w.Scan(doc.baseDir, stem.substr(0, us));
		if (!w.Active())
			continue;
		++chars;
		w.Sync(&doc, 0);
		for (size_t i = 0; i < w.items.size(); ++i)
		{
			const WardrobeItem& it = w.items[i];
			if (it.extra)
				++extras;
			else
				++parts;
			for (size_t h = 0; h < it.hides.size(); ++h)
			{
				if (w.SlotIndex(it.hides[h]) >= 0)
					hideKnown[it.hides[h]]++;
				else
					hideUnknown[it.hides[h]]++;
			}
			size_t before = doc.parts.size();
			w.ResetToDefaults();
			w.Wear(it.file);
			w.Sync(&doc, 0);
			bool worn = false;
			for (size_t p = 0; p < doc.parts.size(); ++p)
				if (!_stricmp(doc.parts[p]->name.c_str(), it.file.c_str()))
					worn = true;
			if (!worn)
			{
				if (failed++ < 10)
					Print("NOT WORN %s\n", it.file.c_str());
				continue;
			}
			if (!it.extra && doc.parts.size() > before)
				++notReplaced;
			doc.Pose(1.0f);
			for (size_t p = 0; p < doc.parts.size(); ++p)
				for (size_t v = 0; v < doc.parts[p]->skinnedPos.size(); ++v)
					if (doc.parts[p]->skinnedPos[v].x != doc.parts[p]->skinnedPos[v].x)
						++nan;
		}
	}
	Print("%d characters; %d slot parts, %d extras; %d failed to wear; %d slot parts did not replace; %d NaN vertices\n", chars, parts, extras, failed, notReplaced, nan);
	Print("hides naming a doll slot:");
	for (std::map<std::string, int>::iterator i = hideKnown.begin(); i != hideKnown.end(); ++i)
		Print(" %s x%d", i->first.c_str(), i->second);
	Print("\nhides naming nothing known:");
	for (std::map<std::string, int>::iterator i = hideUnknown.begin(); i != hideUnknown.end(); ++i)
		Print(" %s x%d", i->first.c_str(), i->second);
	Print("\n");
	return failed ? 1 : 0;
}

int wmain(int argc, wchar_t** argv)
{
	if (argc < 3)
	{
		Print("CaddieRig batch compiler\n"
			"  crigc build project.pmp out.pet [/v 1.3] [/k pet|mpet|bpet|apet]\n"
			"  crigc info file.pet\n"
			"  crigc compare mine.pet theirs.pet [tolerance]\n"
			"  crigc check project.pmp\n"
			"  crigc verify directory\n"
			"  crigc verifypmp directory\n");
		return 2;
	}
	std::wstring cmd = argv[1];
	std::string a1 = ToUtf8(argv[2]);
	if (cmd == L"verify")
		return CmdVerify(argv[2]);
	if (cmd == L"verifypmp")
		return CmdVerifyPmp(argv[2]);
	if (cmd == L"info")
		return CmdInfo(a1.c_str());
	if (cmd == L"posetest")
		return CmdPoseTest(argv[2]);
	if (cmd == L"normalcheck")
		return CmdNormalCheck(argv[2]);
	if (cmd == L"fxcheck")
		return CmdFxCheck(argv[2]);
	if (cmd == L"wardrobecheck")
		return CmdWardrobeCheck(argv[2]);
	if (cmd == L"ptexcheck")
		return CmdPtexCheck(argv[2]);
	if (cmd == L"audiocheck")
		return CmdAudioCheck(argv[2]);
	if (cmd == L"anicheck" && argc >= 5)
		return CmdAniCheck(a1.c_str(), ToUtf8(argv[3]).c_str(), ToUtf8(argv[4]).c_str());
	if (cmd == L"metacheck" && argc >= 4)
		return CmdMetaCheck(a1.c_str(), ToUtf8(argv[3]).c_str());
	if (cmd == L"check" || cmd == L"build")
	{
		PmpFile pmp;
		std::string error;
		if (!pmp.Load(a1.c_str(), &error))
		{
			Print("%s\n", error.c_str());
			return 1;
		}
		BuildOptions opt;
		for (int i = 4; i + 1 < argc; ++i)
		{
			if (!wcscmp(argv[i], L"/v"))
			{
				opt.verMajor = _wtoi(argv[i + 1]);
				const wchar_t* dot = wcschr(argv[i + 1], L'.');
				opt.verMinor = dot ? _wtoi(dot + 1) : 0;
			}
			else if (!wcscmp(argv[i], L"/k"))
			{
				std::string name = std::string("x.") + ToUtf8(argv[i + 1]);
				opt.kind = PetKindFromPath(name.c_str());
			}
		}
		if (argc > 3 && cmd == L"build" && !wcsstr(argv[3], L"/k"))
			opt.kind = PetKindFromPath(ToUtf8(argv[3]).c_str());
		PetFile pet;
		BuildLog log;
		std::string base = PathDir(a1);
		bool ok = CompilePuppet(pmp, base.c_str(), opt, &pet, &log);
		if (ok)
			ValidatePuppet(pmp, pet, base.c_str(), &log);
		if (ok && cmd == L"build" && argc > 3)
		{
			std::string out = ToUtf8(argv[3]);
			if (!pet.Save(out.c_str(), &error))
			{
				Print("%s\n", error.c_str());
				ok = false;
			}
			else
				log.Info("wrote %s", out.c_str());
		}
		PrintLog(log);
		return ok ? 0 : 1;
	}
	if (cmd == L"compare" && argc >= 4)
	{
		PetFile a, b;
		std::string error;
		if (!a.Load(a1.c_str(), &error) || !b.Load(ToUtf8(argv[3]).c_str(), &error))
		{
			Print("%s\n", error.c_str());
			return 1;
		}
		BuildLog log;
		ComparePuppets(a, b, argc > 4 ? (float)_wtof(argv[4]) : 0.0002f, &log);
		PrintLog(log);
		return log.warnings ? 1 : 0;
	}
	Print("unknown command\n");
	return 2;
}
