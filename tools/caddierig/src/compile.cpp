// SPDX-License-Identifier: MIT
// compile.cpp - turns a PMP project and its Max exports into a Puppet.
//
// Everything here was recovered by comparing the archived ASE/PMP/ANI sources
// against the PET files that shipped from them; see README.md for the evidence.
#include "compile.h"
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

static const float FPS = 30.0f;

// -- log ---------------------------------------------------------------------

void BuildLog::Info(const char* fmt, ...)
{
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, fmt, va);
	va_end(va);
	lines.push_back(buf);
}

void BuildLog::Warn(const char* fmt, ...)
{
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buf, fmt, va);
	va_end(va);
	lines.push_back(std::string("! ") + buf);
	++warnings;
}

std::string ResolveSource(const std::string& baseDir, const std::string& name)
{
	std::string base = PathBaseName(name);
	std::string p = PathJoin(baseDir, base);
	if (PathExists(p))
		return p;
	p = PathJoin(PathJoin(baseDir, "ase"), base);
	if (PathExists(p))
		return p;
	p = PathJoin(PathJoin(baseDir, "ani"), base);
	if (PathExists(p))
		return p;
	return std::string();
}

// -- coordinate conversion ---------------------------------------------------

WVector MaxDirToGame(const WVector& v)
{
	return WVector(v.x, v.z, v.y);
}

WVector MaxToGame(const WVector& v)
{
	return WVector(v.x * CRIG_WORLD_SCALE, v.z * CRIG_WORLD_SCALE, v.y * CRIG_WORLD_SCALE);
}

WMatrix MaxTmToGame(const WVector tm[4])
{
	// Swapping the Y and Z axes swaps both the rows and the columns.
	WVector r0 = MaxDirToGame(tm[0]);
	WVector r1 = MaxDirToGame(tm[2]);
	WVector r2 = MaxDirToGame(tm[1]);
	WVector p = MaxToGame(tm[3]);
	WMatrix m;
	m.xx = r0.x; m.yx = r0.y; m.zx = r0.z;
	m.xy = r1.x; m.yy = r1.y; m.zy = r1.z;
	m.xz = r2.x; m.yz = r2.y; m.zz = r2.z;
	m.xm = p.x; m.ym = p.y; m.zm = p.z;
	return m;
}

// Max stores left-handed axis/angle samples; this is how they appear in ANIM.
static WQuat MaxAxisAngleToQuat(const WVector& axis, float angle)
{
	float s = (float)sin(angle * 0.5f);
	return WQuat(-axis.x * s, -axis.z * s, -axis.y * s, (float)cos(angle * 0.5f));
}

static WQuat MatrixToQuat(const WMatrix& m)
{
	// Inverse of WMatrix::SetRotation.
	float tr = m.xx + m.yy + m.zz;
	WQuat q;
	if (tr > 0.0f)
	{
		float s = (float)sqrt(tr + 1.0f) * 2.0f;
		q.w = 0.25f * s;
		q.x = (m.yz - m.zy) / s;
		q.y = (m.zx - m.xz) / s;
		q.z = (m.xy - m.yx) / s;
	}
	else if (m.xx > m.yy && m.xx > m.zz)
	{
		float s = (float)sqrt(1.0f + m.xx - m.yy - m.zz) * 2.0f;
		q.w = (m.yz - m.zy) / s;
		q.x = 0.25f * s;
		q.y = (m.yx + m.xy) / s;
		q.z = (m.zx + m.xz) / s;
	}
	else if (m.yy > m.zz)
	{
		float s = (float)sqrt(1.0f + m.yy - m.xx - m.zz) * 2.0f;
		q.w = (m.zx - m.xz) / s;
		q.x = (m.yx + m.xy) / s;
		q.y = 0.25f * s;
		q.z = (m.zy + m.yz) / s;
	}
	else
	{
		float s = (float)sqrt(1.0f + m.zz - m.xx - m.yy) * 2.0f;
		q.w = (m.xy - m.yx) / s;
		q.x = (m.zx + m.xz) / s;
		q.y = (m.zy + m.yz) / s;
		q.z = 0.25f * s;
	}
	q.Normalize();
	return q;
}

// -- geometry ----------------------------------------------------------------

static const AseMaterial* MaterialFor(const AseFile& ase, const AseNode& node, int mtlId)
{
	if (node.materialRef < 0 || node.materialRef >= (int)ase.materials.size())
		return 0;
	const AseMaterial& m = ase.materials[node.materialRef];
	if (m.subs.empty())
		return &m;
	int wanted = m.numSubs > 0 ? mtlId % m.numSubs : mtlId;
	for (size_t i = 0; i < m.subs.size(); ++i)
		if (m.subs[i].index == wanted)
			return &m.subs[i];
	return &m.subs[0];
}

void BuildGeometry(const AseFile& ase, PetFile* pet, BuildLog* log)
{
	pet->bones.clear();
	pet->vertices.clear();
	pet->points.clear();
	pet->triTextures.clear();
	pet->skins.clear();

	std::vector<WMatrix> world(ase.nodes.size());
	pet->bones.resize(ase.nodes.size());
	for (size_t i = 0; i < ase.nodes.size(); ++i)
	{
		const AseNode& n = ase.nodes[i];
		world[i] = MaxTmToGame(n.tm);
		PetBone& b = pet->bones[i];
		b.name = n.name;
		b.parent = n.parent.empty() ? -1 : ase.FindNode(n.parent);
		b.tail = -1.0f;
		if (!n.parent.empty() && b.parent < 0 && log)
			log->Warn("bone %s: parent %s is not in the scene", Cp949ToUtf8(n.name).c_str(), Cp949ToUtf8(n.parent).c_str());
		if (b.parent >= (int)i)
		{
			if (log)
				log->Warn("bone %s comes before its parent; the client resolves parents in file order", Cp949ToUtf8(n.name).c_str());
		}
		b.mat = b.parent >= 0 ? world[i] * world[b.parent].Inverse() : world[i];
	}

	int base = 0;
	for (size_t i = 0; i < ase.nodes.size(); ++i)
	{
		const AseNode& n = ase.nodes[i];
		if (n.verts.empty() || n.faces.empty())
			continue;
		WMatrix toBone = world[i].Inverse();
		PetSkin skin;
		skin.startVtx = base;
		skin.numVtx = (int)n.verts.size();
		skin.startTri = (int)pet->triTextures.size();
		skin.numTri = (int)n.faces.size();
		skin.bone = (u8)i;
		skin.pad[0] = skin.pad[1] = skin.pad[2] = 0;
		pet->skins.push_back(skin);

		for (size_t k = 0; k < n.verts.size(); ++k)
		{
			PetVertex v;
			v.pos = MaxToGame(n.verts[k]) * toBone;
			v.weight = 1.0f;
			v.pad[0] = v.pad[1] = 0;
			PetBlend blend;
			blend.weight = 255;
			blend.bone = (int)i;
			v.blends.push_back(blend);
			pet->vertices.push_back(v);
		}
		for (size_t f = 0; f < n.faces.size(); ++f)
		{
			const AseFace& face = n.faces[f];
			for (int c = 0; c < 3; ++c)
			{
				PetTriPoint p;
				// Swapping Y and Z flips handedness, so corners go out as A C B.
				static const int order[3] = { 0, 2, 1 };
				int corner = order[c];
				// .mpet addresses vertices per skin, the other kinds use the whole list
				p.pos = pet->kind == PET_MESH ? face.v[corner] : face.v[corner] + base;
				size_t ni = f * 3 + corner;
				if (ni < n.vertNormals.size())
				{
					p.normal = toBone.Rotate(MaxDirToGame(n.vertNormals[ni]));
					p.normal.Normalize();
				}
				p.u = 0.0f;
				p.v = 0.0f;
				size_t ti = f * 3 + corner;
				if (ti < n.tfaces.size() && n.tfaces[ti] < (int)n.tverts.size())
				{
					const WVector& tv = n.tverts[n.tfaces[ti]];
					p.u = tv.x;
					p.v = 1.0f - tv.y;
				}
				pet->points.push_back(p);
			}
			pet->triTextures.push_back(0);
		}
		base += (int)n.verts.size();
	}
}

// Textures come from the PMP order; each triangle finds its slot through its
// Max material's bitmap name.
static std::string LowerBase(const std::string& path)
{
	std::string b = PathBaseName(path);
	std::transform(b.begin(), b.end(), b.begin(), ::tolower);
	return b;
}

static std::string StripExt(const std::string& name)
{
	size_t p = name.rfind('.');
	return p == std::string::npos ? name : name.substr(0, p);
}

static u32 DiffuseOf(const AseMaterial& m)
{
	// Truncated, not rounded: Max's 0.5882 ships as 0x95.
	int a = (int)((1.0f - m.transparency) * 255.0f);
	int r = (int)(m.diffuse[0] * 255.0f);
	int g = (int)(m.diffuse[1] * 255.0f);
	int b = (int)(m.diffuse[2] * 255.0f);
	return (u32)((a << 24) | (r << 16) | (g << 8) | b);
}

static const AseMaterial* FindMaterialByBitmap(const std::vector<AseMaterial>& list, const std::string& base)
{
	for (size_t i = 0; i < list.size(); ++i)
	{
		if (!list[i].bitmap.empty() && LowerBase(list[i].bitmap) == base)
			return &list[i];
		const AseMaterial* sub = FindMaterialByBitmap(list[i].subs, base);
		if (sub)
			return sub;
	}
	return 0;
}

void ApplyTextures(const PmpFile& pmp, const AseFile* ase, PetFile* pet)
{
	pet->textures.clear();
	pet->faces.clear();
	std::vector<std::string> blockNames;
	int nextGroup = 2;
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		const PmpBlock& b = pmp.blocks[i];
		if (b.kind != PMP_TEXTURE)
			continue;
		std::string file = Utf8ToCp949(LowerBase(b.Get("텍스쳐파일")));
		std::string name = StripExt(Utf8ToCp949(PathBaseName(b.Get("텍스쳐이름"))));
		const std::string& parent = b.Get("부모텍스쳐");
		bool specular = NameHasFlag(file, "!");

		PetTexture t;
		memset(&t, 0, sizeof(t));
		t.SetName(file);
		t.flag = b.GetInt("텍스쳐연결") ? 0 : 1;
		t.diffuse = 0xffffffff;
		if (ase)
		{
			const AseMaterial* m = FindMaterialByBitmap(ase->materials, LowerBase(b.Get("텍스쳐파일")));
			if (m)
				t.diffuse = DiffuseOf(*m);
		}
		if (specular)
		{
			t.group = 0;
			t.handle = 1;
		}
		else
		{
			if (parent != "@" && !parent.empty())
			{
				// A child swaps in for its parent's meshes: same group, one FANM entry.
				for (size_t k = 0; k < blockNames.size(); ++k)
					if (blockNames[k] == parent)
						t.group = pet->textures[k].group;
			}
			if (!t.group)
			{
				t.group = (u8)nextGroup;
				nextGroup += 2;
			}
			t.handle = 128 + (int)pet->textures.size();
			PetFaceAnim fa;
			memset(&fa, 0, sizeof(fa));
			fa.group = t.group;
			fa.Set(name, file);
			pet->faces.push_back(fa);
		}
		pet->textures.push_back(t);
		blockNames.push_back(b.Get("텍스쳐이름"));
	}
}

static void AssignTriangleTextures(const AseFile& ase, PetFile* pet, BuildLog* log)
{
	size_t tri = 0;
	for (size_t i = 0; i < ase.nodes.size(); ++i)
	{
		const AseNode& n = ase.nodes[i];
		if (n.verts.empty() || n.faces.empty())
			continue;
		for (size_t f = 0; f < n.faces.size(); ++f, ++tri)
		{
			const AseMaterial* m = MaterialFor(ase, n, n.faces[f].mtl);
			int slot = 0;
			if (m && !m->bitmap.empty())
			{
				std::string want = LowerBase(m->bitmap);
				bool found = false;
				for (size_t t = 0; t < pet->textures.size(); ++t)
				{
					if (_stricmp(pet->textures[t].Name().c_str(), want.c_str()) == 0)
					{
						slot = (int)t;
						found = true;
						break;
					}
				}
				if (!found && log)
				{
					log->Warn("material bitmap %s has no texture block", want.c_str());
					// report each missing bitmap once
					for (size_t t = f; t < n.faces.size(); ++t)
						pet->triTextures[tri + (t - f)] = 0;
					tri += n.faces.size() - f - 1;
					break;
				}
			}
			pet->triTextures[tri] = (u8)slot;
		}
	}
}

void ApplyBoxes(const PmpFile& pmp, PetFile* pet)
{
	pet->colls.clear();
	static const char* keys[6] = { "최소값x", "최소값y", "최소값z", "최대값x", "최대값y", "최대값z" };
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		const PmpBlock& b = pmp.blocks[i];
		if (b.kind != PMP_BOX)
			continue;
		PetColl c;
		c.shape = b.GetInt("모양(o,ㅁ)");
		c.visible = b.GetInt("보이기");
		c.name = Utf8ToCp949(b.Get("박스이름"));
		c.bone = Utf8ToCp949(b.Get("부모본이름"));
		// "@" means no script. Shipped files store it as one NUL byte in the first slot
		// and as an empty string in the second.
		const std::string& s0 = b.Get("스크립트0");
		const std::string& s1 = b.Get("스크립트1");
		c.script0 = s0 == "@" ? std::string(1, '\0') : Utf8ToCp949(s0);
		c.script1 = s1 == "@" ? std::string() : Utf8ToCp949(s1);
		for (int k = 0; k < 6; ++k)
			c.box[k] = b.GetFloat(keys[k]);
		pet->colls.push_back(c);
	}
}

void ApplyMotions(const PmpFile& pmp, PetFile* pet)
{
	pet->motions.clear();
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		const PmpBlock& b = pmp.blocks[i];
		if (b.kind != PMP_MOTION)
			continue;
		PetMotion m;
		m.name = Utf8ToCp949(b.Get("동작이름"));
		m.startFrame = (u32)b.GetInt("시작프레임");
		m.endFrame = (u32)b.GetInt("종료프레임");
		m.next = Utf8ToCp949(b.Get("다음동작"));
		m.option = Utf8ToCp949(b.Get("연결방법"));
		m.optionValue = b.GetFloat("연결시간");
		m.applyBone = Utf8ToCp949(b.Get("최상위본"));
		pet->motions.push_back(m);
	}
}

void ApplyScripts(const PmpFile& pmp, PetFile* pet)
{
	pet->frames.clear();
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		const PmpBlock& b = pmp.blocks[i];
		if (b.kind != PMP_SCRIPT)
			continue;
		int frame = atoi(b.arg.c_str());
		if (frame < 0)
			continue;		// "-001": written for a clip no motion places on the timeline
		PetFrame f;
		f.frame = (u32)frame;
		f.script = Utf8ToCp949(b.Get("메시지"));
		f.check = (u32)b.GetInt("플래그");
		f.hasCheck = true;
		pet->frames.push_back(f);
	}
	if (pet->frames.size() == 1 && pet->frames[0].script.empty())
		pet->frames[0].hasCheck = false;
}

// -- animation ---------------------------------------------------------------

static PetAnim& AnimForBone(PetFile* pet, int bone)
{
	for (size_t i = 0; i < pet->anims.size(); ++i)
		if (pet->anims[i].bone == bone)
			return pet->anims[i];
	pet->anims.push_back(PetAnim());
	PetAnim& a = pet->anims.back();
	a.bone = bone;
	return a;
}

bool MergeMotion(const char* aniPath, int start, int srcStart, int srcEnd, PetFile* pet, BuildLog* log)
{
	AseFile ani;
	std::string error;
	if (!ani.Load(aniPath, &error))
	{
		if (log)
			log->Warn("%s: %s", PathBaseName(aniPath).c_str(), error.c_str());
		return false;
	}
	int tpf = ani.ticksPerFrame > 0 ? ani.ticksPerFrame : 160;
	float t0 = start / FPS;
	float tEnd = (start + srcEnd - srcStart) / FPS;
	std::vector<WMatrix> world(ani.nodes.size());
	for (size_t i = 0; i < ani.nodes.size(); ++i)
		world[i] = MaxTmToGame(ani.nodes[i].tm);

	int matched = 0;
	for (size_t i = 0; i < ani.nodes.size(); ++i)
	{
		const AseNode& n = ani.nodes[i];
		int bone = pet->FindBone(n.name);
		if (bone < 0)
			continue;
		++matched;
		int parent = n.parent.empty() ? -1 : ani.FindNode(n.parent);
		WMatrix local = parent >= 0 ? world[i] * world[parent].Inverse() : world[i];
		WVector restPos = local.Pivot();
		WQuat restRot = MatrixToQuat(local);

		PetAnim& a = AnimForBone(pet, bone);
		// Samples are clipped to the motion's source range. Bones without a track hold
		// their rest pose so interpolation cannot leak in from the neighbouring clip.
		bool anyPos = false;
		for (size_t k = 0; k < n.posKeys.size(); ++k)
		{
			int frame = n.posKeys[k].tick / tpf;
			if (frame < srcStart || frame > srcEnd)
				continue;
			PetKeyPos key;
			key.time = (start + frame - srcStart) / FPS;
			key.v = MaxToGame(n.posKeys[k].v);
			a.pos.push_back(key);
			anyPos = true;
		}
		if (!anyPos)
		{
			PetKeyPos key;
			key.time = t0;
			key.v = restPos;
			a.pos.push_back(key);
			key.time = tEnd;
			a.pos.push_back(key);
		}

		// The first sample is absolute, the others are deltas against the previous one.
		WQuat q = restRot;
		bool anyRot = false;
		for (size_t k = 0; k < n.rotKeys.size(); ++k)
		{
			const AseKeyRot& rk = n.rotKeys[k];
			WQuat d = MaxAxisAngleToQuat(rk.axis, rk.angle);
			q = k == 0 ? d : q * d;
			int frame = rk.tick / tpf;
			if (frame < srcStart || frame > srcEnd)
				continue;
			PetKeyRot key;
			key.time = (start + frame - srcStart) / FPS;
			key.q = q;
			key.q.Normalize();
			a.rot.push_back(key);
			anyRot = true;
		}
		if (!anyRot)
		{
			PetKeyRot key;
			key.time = t0;
			key.q = restRot;
			a.rot.push_back(key);
			key.time = tEnd;
			a.rot.push_back(key);
		}
	}
	if (log)
		log->Info("  %s: %d frames, %d bones matched", PathBaseName(aniPath).c_str(), srcEnd - srcStart, matched);
	return true;
}

// Greedy removal of keys the client would reproduce by interpolating anyway.
void FinishAnimation(PetFile* pet, float posTolerance)
{
	float last = 0.0f;
	for (size_t i = 0; i < pet->anims.size(); ++i)
	{
		PetAnim& a = pet->anims[i];
		if (a.pos.size() > 2)
		{
			std::vector<PetKeyPos> out;
			out.push_back(a.pos[0]);
			for (size_t k = 1; k + 1 < a.pos.size(); ++k)
			{
				const PetKeyPos& p = out.back();
				const PetKeyPos& n = a.pos[k + 1];
				float span = n.time - p.time;
				float t = span > 0.0f ? (a.pos[k].time - p.time) / span : 0.0f;
				WVector lerp = p.v + (n.v - p.v) * t;
				if ((lerp - a.pos[k].v).Length() > posTolerance)
					out.push_back(a.pos[k]);
			}
			out.push_back(a.pos.back());
			a.pos.swap(out);
		}
		if (a.rot.size() > 2)
		{
			std::vector<PetKeyRot> out;
			out.push_back(a.rot[0]);
			for (size_t k = 1; k + 1 < a.rot.size(); ++k)
			{
				const PetKeyRot& p = out.back();
				const PetKeyRot& n = a.rot[k + 1];
				float span = n.time - p.time;
				float t = span > 0.0f ? (a.rot[k].time - p.time) / span : 0.0f;
				WQuat s = WSlerp(p.q, n.q, t);
				float dot = (float)fabs(s.x * a.rot[k].q.x + s.y * a.rot[k].q.y + s.z * a.rot[k].q.z + s.w * a.rot[k].q.w);
				if (dot < 0.9999999f)
					out.push_back(a.rot[k]);
			}
			out.push_back(a.rot.back());
			a.rot.swap(out);
		}
		if (!a.pos.empty() && a.pos.back().time > last)
			last = a.pos.back().time;
		if (!a.rot.empty() && a.rot.back().time > last)
			last = a.rot.back().time;
	}
	// Bones must appear in id order for the reader's FindBone loop to stay cheap.
	for (size_t i = 1; i < pet->anims.size(); ++i)
	{
		for (size_t k = i; k > 0 && pet->anims[k - 1].bone > pet->anims[k].bone; --k)
			std::swap(pet->anims[k - 1], pet->anims[k]);
	}
	// Scale (since v1.1) and flag (since v1.3) tracks: nothing authors them, write the
	// constants that shipped files carry when a bone is not scaled or hidden.
	for (size_t i = 0; i < pet->anims.size(); ++i)
	{
		PetAnim& a = pet->anims[i];
		a.scale.clear();
		a.flags.clear();
		if (pet->VerAtLeast(1, 1))
		{
			PetKeyPos s;
			s.time = 0.0f;
			s.v = WVector(1.0f, 1.0f, 1.0f);
			a.scale.push_back(s);
			// Duas chaves, como no jogo: com uma so o GetKeyScale do cliente divide por zero depois de t=0 e o osso some
			s.time = last;
			a.scale.push_back(s);
		}
		if (pet->VerAtLeast(1, 3))
		{
			PetKeyFlag f;
			f.time = 0.0f;
			f.value = -1.0f;
			a.flags.push_back(f);
			f.time = last;
			a.flags.push_back(f);
		}
	}
}

// -- layout ------------------------------------------------------------------

void LayoutChunks(PetFile* pet)
{
	static const char* full[] = { "TEXT", "BONE", "ANIM", "MESH", "FANM", "FRAM", "MOTI", "COLL" };
	static const char* mesh[] = { "TEXT", "BONE", "MESH", "FANM" };
	static const char* bone[] = { "BONE", "COLL" };
	static const char* anim[] = { "BONE", "ANIM", "FRAM", "MOTI" };
	const char** order = full;
	size_t count = 8;
	if (pet->kind == PET_MESH)
	{
		order = mesh;
		count = 4;
	}
	else if (pet->kind == PET_BONE)
	{
		order = bone;
		count = 2;
	}
	else if (pet->kind == PET_ANIM)
	{
		order = anim;
		count = 4;
	}
	pet->chunks.clear();
	if (!pet->VerIs(1, 0))
		pet->AddChunk("VERS");
	for (size_t i = 0; i < count; ++i)
	{
		// SMTL sits right after TEXT from v1.2 on
		pet->AddChunk(order[i]);
		if (!strcmp(order[i], "TEXT") && pet->VerAtLeast(1, 2))
			pet->AddChunk("SMTL");
	}
}

// -- full compile ------------------------------------------------------------

bool CompilePuppet(const PmpFile& pmp, const char* baseDir, const BuildOptions& opt, PetFile* pet, BuildLog* log)
{
	pet->Clear();
	pet->kind = opt.kind;
	pet->verMajor = opt.verMajor;
	pet->verMinor = opt.verMinor;

	std::string asePath = ResolveSource(baseDir, pmp.aseName);
	AseFile ase;
	std::string error;
	if (asePath.empty() || !ase.Load(asePath.c_str(), &error))
	{
		if (log)
			log->Warn("cannot load ASE %s %s", pmp.aseName.c_str(), error.c_str());
		return false;
	}
	if (log)
		log->Info("ASE %s: %d nodes, %d materials", pmp.aseName.c_str(), (int)ase.nodes.size(), (int)ase.materials.size());

	BuildGeometry(ase, pet, log);
	ApplyTextures(pmp, &ase, pet);
	AssignTriangleTextures(ase, pet, log);
	ApplyBoxes(pmp, pet);
	ApplyMotions(pmp, pet);
	ApplyScripts(pmp, pet);

	int missing = 0;
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		const PmpBlock& b = pmp.blocks[i];
		if (b.kind != PMP_MOTION)
			continue;
		const std::string& src = b.Get("원본파일");
		if (src.empty())
			continue;
		std::string path = ResolveSource(baseDir, src);
		if (path.empty())
		{
			if (!missing && log)
				log->Warn("motion source %s not found (further misses counted, not listed)", src.c_str());
			++missing;
			continue;
		}
		MergeMotion(path.c_str(), b.GetInt("시작프레임"), b.GetInt("원본시작프레임"), b.GetInt("원본종료프레임"), pet, log);
	}
	if (missing && log)
		log->Warn("%d motions have no .Ani on disk; they are in MOTI but have no keys", missing);
	FinishAnimation(pet, opt.posTolerance);
	LayoutChunks(pet);

	if (log)
		log->Info("built: %d bones, %d vertices, %d triangles, %d textures, %d boxes, %d motions, %d scripts",
			(int)pet->bones.size(), (int)pet->vertices.size(), (int)pet->points.size() / 3,
			(int)pet->textures.size(), (int)pet->colls.size(), (int)pet->motions.size(), (int)pet->frames.size());
	return true;
}

// -- PET -> project ----------------------------------------------------------

void PetToProject(const PetFile& pet, const char* fileName, PmpFile* pmp)
{
	pmp->Clear();
	pmp->aseName = StripExt(PathBaseName(fileName)) + ".ASE";
	for (size_t i = 0; i < pet.textures.size(); ++i)
	{
		const PetTexture& t = pet.textures[i];
		std::string file = Cp949ToUtf8(t.Name());
		std::string name = StripExt(file);
		for (size_t k = 0; k < pet.faces.size(); ++k)
		{
			if (!_stricmp(pet.faces[k].File().c_str(), t.Name().c_str()))
				name = Cp949ToUtf8(pet.faces[k].Name());
		}
		if (NameHasFlag(file, "!"))
			name = "Specular Map";
		pmp->blocks.push_back(PmpFile::NewTexture(name, file, t.flag ? 0 : 1));
	}
	for (size_t i = 0; i < pet.colls.size(); ++i)
	{
		const PetColl& c = pet.colls[i];
		PmpBlock b = PmpFile::NewBox(Cp949ToUtf8(c.name), Cp949ToUtf8(c.bone), c.box);
		char num[16];
		sprintf_s(num, "%d", c.shape);
		b.Set("모양(o,ㅁ)", num);
		sprintf_s(num, "%d", c.visible);
		b.Set("보이기", num);
		if (c.script0.size() > 1)
			b.Set("스크립트0", Cp949ToUtf8(c.script0));
		if (!c.script1.empty())
			b.Set("스크립트1", Cp949ToUtf8(c.script1));
		pmp->blocks.push_back(b);
	}
	for (size_t i = 0; i < pet.frames.size(); ++i)
	{
		const PetFrame& f = pet.frames[i];
		PmpBlock b = PmpFile::NewScript((int)f.frame, Cp949ToUtf8(f.script), std::string(), (int)f.frame);
		char num[16];
		sprintf_s(num, "%u", f.check);
		b.Set("플래그", num);
		pmp->blocks.push_back(b);
	}
	for (size_t i = 0; i < pet.motions.size(); ++i)
	{
		const PetMotion& m = pet.motions[i];
		PmpBlock b = PmpFile::NewMotion(Cp949ToUtf8(m.name), (int)m.startFrame, (int)m.endFrame,
			Cp949ToUtf8(m.next), Cp949ToUtf8(m.applyBone), std::string(), (int)0, (int)(m.endFrame - m.startFrame));
		b.Set("연결방법", Cp949ToUtf8(m.option));
		char num[32];
		sprintf_s(num, "%.4f", m.optionValue);
		b.Set("연결시간", num);
		pmp->blocks.push_back(b);
	}
}

bool FitBoneBox(const PetFile& pet, int bone, float box[6])
{
	bool any = false;
	for (size_t i = 0; i < pet.vertices.size(); ++i)
	{
		const PetVertex& v = pet.vertices[i];
		if (v.blends.empty() || v.blends[0].bone != bone)
			continue;
		if (!any)
		{
			box[0] = box[3] = v.pos.x;
			box[1] = box[4] = v.pos.y;
			box[2] = box[5] = v.pos.z;
			any = true;
			continue;
		}
		if (v.pos.x < box[0]) box[0] = v.pos.x;
		if (v.pos.y < box[1]) box[1] = v.pos.y;
		if (v.pos.z < box[2]) box[2] = v.pos.z;
		if (v.pos.x > box[3]) box[3] = v.pos.x;
		if (v.pos.y > box[4]) box[4] = v.pos.y;
		if (v.pos.z > box[5]) box[5] = v.pos.z;
	}
	return any;
}

// -- validation --------------------------------------------------------------

void ValidatePuppet(const PmpFile& pmp, const PetFile& pet, const char* baseDir, BuildLog* log,
	const std::vector<std::string>* foundTextures)
{
	int before = log->warnings;
	bool hasCenter = false;

	for (size_t i = 0; i < pet.textures.size(); ++i)
	{
		std::string full = Cp949ToUtf8(pet.textures[i].Name());
		if (full.empty() || full == "null")
			continue;
		if (foundTextures && std::find(foundTextures->begin(), foundTextures->end(), full) != foundTextures->end())
			continue;
		if (ResolveSource(baseDir, full).empty())
			log->Warn("texture %s: file not found next to the project", full.c_str());
	}
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		const PmpBlock& b = pmp.blocks[i];
		if (b.kind == PMP_TEXTURE)
		{
			std::string file = LowerBase(b.Get("텍스쳐파일"));
			if (Utf8ToCp949(file).size() > 31)
				log->Warn("texture %s: over 31 bytes, TEXT stores 32 including the terminator", file.c_str());
		}
	}
	for (size_t i = 0; i < pet.colls.size(); ++i)
	{
		const PetColl& c = pet.colls[i];
		std::string name = Cp949ToUtf8(c.name);
		if (name == "center" || name == "bound")
			hasCenter = true;
		if (!pet.bones.empty() && pet.FindBone(c.bone) < 0)
			log->Warn("box %s: bone %s does not exist (FindBone returns null in SetBoundBox)", name.c_str(), Cp949ToUtf8(c.bone).c_str());
		if (c.box[0] > c.box[3] || c.box[1] > c.box[4] || c.box[2] > c.box[5])
			log->Warn("box %s: min is greater than max", name.c_str());
		if (name.size() > 511 || c.bone.size() > 511)
			log->Warn("box %s: name exceeds the loader's 512 byte buffer", name.c_str());
	}
	if (!hasCenter && !pet.colls.empty())
		log->Warn("no \"center\" or \"bound\" box; the client will synthesize one from the mesh bounds");

	for (size_t i = 0; i < pet.motions.size(); ++i)
	{
		const PetMotion& m = pet.motions[i];
		std::string name = Cp949ToUtf8(m.name);
		if (m.endFrame < m.startFrame)
			log->Warn("motion %s: end frame before start frame", name.c_str());
		if (!m.next.empty())
		{
			bool found = false;
			for (size_t k = 0; k < pet.motions.size(); ++k)
				if (pet.motions[k].name == m.next)
					found = true;
			if (!found)
				log->Warn("motion %s: next motion %s does not exist; GetBones dereferences it", name.c_str(), Cp949ToUtf8(m.next).c_str());
		}
		if (!m.applyBone.empty() && !pet.bones.empty() && pet.FindBone(m.applyBone) < 0)
			log->Warn("motion %s: apply bone %s does not exist", name.c_str(), Cp949ToUtf8(m.applyBone).c_str());
		if (!_stricmp(m.option.c_str(), "blending") && m.optionValue <= 0.0f)
			log->Warn("motion %s: blending with zero transition time", name.c_str());
	}
	if (pet.frames.size() > 10000)
		log->Warn("%d script frames; LoadPET_Frame silently ignores more than 10000", (int)pet.frames.size());
	for (size_t i = 0; i < pet.frames.size(); ++i)
	{
		if (pet.frames[i].script.size() >= 1024)
			log->Warn("script at frame %u: over the loader's 1024 byte buffer", pet.frames[i].frame);
		if (i > 0 && pet.frames[i].frame < pet.frames[i - 1].frame)
			log->Warn("script at frame %u: out of order; CheckFrameData assumes ascending frames", pet.frames[i].frame);
	}
	for (size_t i = 0; i < pet.bones.size(); ++i)
	{
		if (pet.bones[i].parent >= (int)i)
			log->Warn("bone %s: parent must precede it", Cp949ToUtf8(pet.bones[i].name).c_str());
		if (pet.bones[i].name.size() > 255)
			log->Warn("bone %d: name over the loader's 256 byte buffer", (int)i);
	}
	if (pet.textures.size() > 255)
		log->Warn("%d textures, but triangles address them with one byte", (int)pet.textures.size());
	for (size_t i = 0; i < pet.vertices.size(); ++i)
	{
		const PetVertex& v = pet.vertices[i];
		if (v.blends.size() > 16)
		{
			log->Warn("vertex %d: %d blends; the loader's array holds 16", (int)i, (int)v.blends.size());
			break;
		}
	}
	if (pet.kind != PET_ANIM && pet.bones.empty())
		log->Warn("no bones: LoadPET_Bone returns null and the client crashes on Transform");
	log->Info("validation finished: %d issue(s)", log->warnings - before);
}

// -- comparison --------------------------------------------------------------

static bool Near(float a, float b, float tol)
{
	return fabs(a - b) <= tol;
}

void ComparePuppets(const PetFile& a, const PetFile& b, float tol, BuildLog* log)
{
	if (a.bones.size() != b.bones.size())
		log->Warn("BONE: %d vs %d", (int)a.bones.size(), (int)b.bones.size());
	else
	{
		int bad = 0;
		for (size_t i = 0; i < a.bones.size(); ++i)
		{
			const float* x = &a.bones[i].mat.xx;
			const float* y = &b.bones[i].mat.xx;
			bool differ = a.bones[i].name != b.bones[i].name || a.bones[i].parent != b.bones[i].parent;
			for (int k = 0; k < 12 && !differ; ++k)
				differ = !Near(x[k], y[k], tol);
			if (differ && !bad++)
				log->Warn("BONE: first mismatch at %d (%s)", (int)i, Cp949ToUtf8(a.bones[i].name).c_str());
		}
		log->Info("BONE: %d bones, %d mismatched", (int)a.bones.size(), bad);
	}

	if (a.vertices.size() != b.vertices.size())
		log->Warn("vertices: %d vs %d", (int)a.vertices.size(), (int)b.vertices.size());
	else
	{
		int bad = 0;
		for (size_t i = 0; i < a.vertices.size(); ++i)
		{
			const PetVertex& x = a.vertices[i];
			const PetVertex& y = b.vertices[i];
			if ((x.pos - y.pos).Length() > tol || x.blends.size() != y.blends.size())
			{
				if (!bad++)
					log->Warn("vertex %d: (%f %f %f) vs (%f %f %f)", (int)i, x.pos.x, x.pos.y, x.pos.z, y.pos.x, y.pos.y, y.pos.z);
			}
		}
		log->Info("MESH vertices: %d, %d mismatched", (int)a.vertices.size(), bad);
	}

	if (a.points.size() != b.points.size())
		log->Warn("triangles: %d vs %d", (int)a.points.size() / 3, (int)b.points.size() / 3);
	else
	{
		int badPos = 0, badUv = 0, badNormal = 0, badTex = 0;
		for (size_t i = 0; i < a.points.size(); ++i)
		{
			const PetTriPoint& x = a.points[i];
			const PetTriPoint& y = b.points[i];
			if (x.pos != y.pos && !badPos++)
				log->Warn("corner %d: index %u vs %u", (int)i, x.pos, y.pos);
			if ((!Near(x.u, y.u, tol) || !Near(x.v, y.v, tol)) && !badUv++)
				log->Warn("corner %d: uv (%f %f) vs (%f %f)", (int)i, x.u, x.v, y.u, y.v);
			if ((x.normal - y.normal).Length() > tol * 4.0f && !badNormal++)
				log->Warn("corner %d: normal (%f %f %f) vs (%f %f %f)", (int)i, x.normal.x, x.normal.y, x.normal.z, y.normal.x, y.normal.y, y.normal.z);
		}
		for (size_t i = 0; i < a.triTextures.size() && i < b.triTextures.size(); ++i)
			if (a.triTextures[i] != b.triTextures[i])
				++badTex;
		log->Info("MESH corners: %d, index %d / uv %d / normal %d mismatched; triangle textures %d mismatched",
			(int)a.points.size(), badPos, badUv, badNormal, badTex);
	}

	if (a.textures.size() != b.textures.size())
		log->Warn("TEXT: %d vs %d", (int)a.textures.size(), (int)b.textures.size());
	else
	{
		for (size_t i = 0; i < a.textures.size(); ++i)
		{
			const PetTexture& x = a.textures[i];
			const PetTexture& y = b.textures[i];
			if (_stricmp(x.Name().c_str(), y.Name().c_str()) || x.flag != y.flag || x.diffuse != y.diffuse || x.group != y.group || x.handle != y.handle)
				log->Warn("TEXT %d: %s flag %d group %d diffuse %08x handle %d vs %s flag %d group %d diffuse %08x handle %d", (int)i,
					Cp949ToUtf8(x.Name()).c_str(), x.flag, x.group, x.diffuse, x.handle,
					Cp949ToUtf8(y.Name()).c_str(), y.flag, y.group, y.diffuse, y.handle);
		}
		log->Info("TEXT: %d textures compared", (int)a.textures.size());
	}

	if (a.faces.size() != b.faces.size())
		log->Warn("FANM: %d vs %d", (int)a.faces.size(), (int)b.faces.size());
	else
	{
		for (size_t i = 0; i < a.faces.size(); ++i)
		{
			if (a.faces[i].Name() != b.faces[i].Name() || a.faces[i].File() != b.faces[i].File() || a.faces[i].group != b.faces[i].group)
				log->Warn("FANM %d: %s/%s/%d vs %s/%s/%d", (int)i,
					Cp949ToUtf8(a.faces[i].Name()).c_str(), Cp949ToUtf8(a.faces[i].File()).c_str(), a.faces[i].group,
					Cp949ToUtf8(b.faces[i].Name()).c_str(), Cp949ToUtf8(b.faces[i].File()).c_str(), b.faces[i].group);
		}
		log->Info("FANM: %d entries compared", (int)a.faces.size());
	}

	if (a.colls.size() != b.colls.size())
		log->Warn("COLL: %d vs %d", (int)a.colls.size(), (int)b.colls.size());
	else
	{
		int bad = 0;
		for (size_t i = 0; i < a.colls.size(); ++i)
		{
			const PetColl& x = a.colls[i];
			const PetColl& y = b.colls[i];
			bool differ = x.shape != y.shape || x.visible != y.visible || x.name != y.name || x.bone != y.bone || x.script0 != y.script0 || x.script1 != y.script1;
			for (int k = 0; k < 6 && !differ; ++k)
				differ = !Near(x.box[k], y.box[k], 0.00001f);
			if (differ && !bad++)
				log->Warn("COLL %d: %d %d [%s] [%s] %d/%d (%f..%f) vs %d %d [%s] [%s] %d/%d (%f..%f)", (int)i,
					x.shape, x.visible, Cp949ToUtf8(x.name).c_str(), Cp949ToUtf8(x.bone).c_str(), (int)x.script0.size(), (int)x.script1.size(), x.box[0], x.box[5],
					y.shape, y.visible, Cp949ToUtf8(y.name).c_str(), Cp949ToUtf8(y.bone).c_str(), (int)y.script0.size(), (int)y.script1.size(), y.box[0], y.box[5]);
		}
		log->Info("COLL: %d boxes, %d mismatched", (int)a.colls.size(), bad);
	}

	if (a.motions.size() != b.motions.size())
		log->Warn("MOTI: %d vs %d", (int)a.motions.size(), (int)b.motions.size());
	{
		int matched = 0, bad = 0;
		for (size_t i = 0; i < a.motions.size(); ++i)
		{
			for (size_t k = 0; k < b.motions.size(); ++k)
			{
				const PetMotion& x = a.motions[i];
				const PetMotion& y = b.motions[k];
				if (x.name != y.name)
					continue;
				++matched;
				if (x.startFrame != y.startFrame || x.endFrame != y.endFrame || x.next != y.next || x.option != y.option || x.applyBone != y.applyBone)
				{
					if (!bad++)
						log->Warn("MOTI %s: %u-%u next %s vs %u-%u next %s", Cp949ToUtf8(x.name).c_str(), x.startFrame, x.endFrame,
							Cp949ToUtf8(x.next).c_str(), y.startFrame, y.endFrame, Cp949ToUtf8(y.next).c_str());
				}
				break;
			}
		}
		log->Info("MOTI: %d of %d names matched, %d differ", matched, (int)a.motions.size(), bad);
	}

	if (a.frames.size() != b.frames.size())
		log->Warn("FRAM: %d vs %d", (int)a.frames.size(), (int)b.frames.size());
	else
	{
		int bad = 0;
		for (size_t i = 0; i < a.frames.size(); ++i)
			if (a.frames[i].frame != b.frames[i].frame || a.frames[i].script != b.frames[i].script || a.frames[i].check != b.frames[i].check)
				if (!bad++)
					log->Warn("FRAM %d: %u [%s] chk %u vs %u [%s] chk %u", (int)i, a.frames[i].frame, Cp949ToUtf8(a.frames[i].script).c_str(), a.frames[i].check,
						b.frames[i].frame, Cp949ToUtf8(b.frames[i].script).c_str(), b.frames[i].check);
		log->Info("FRAM: %d entries, %d mismatched", (int)a.frames.size(), bad);
	}
}
