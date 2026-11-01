// SPDX-License-Identifier: MIT
// petfile.cpp - chunk reader and writer for Puppet files.
//
// The read order inside each chunk follows WPuppet::LoadPET_* in the client.
#include "petfile.h"
#include "wcommon.h"
#include <stdio.h>
#include <string.h>

// -- byte stream -------------------------------------------------------------

namespace
{
	struct Reader
	{
		const u8* p;
		const u8* end;
		bool bad;

		Reader(const u8* data, size_t size) : p(data), end(data + size), bad(false) { }

		size_t Left() const { return (size_t)(end - p); }
		void Read(void* out, size_t n)
		{
			if (Left() < n)
			{
				bad = true;
				memset(out, 0, n);
				p = end;
				return;
			}
			memcpy(out, p, n);
			p += n;
		}
		u8 U8() { u8 v; Read(&v, 1); return v; }
		u32 U32() { u32 v; Read(&v, 4); return v; }
		int S32() { int v; Read(&v, 4); return v; }
		float F32() { float v; Read(&v, 4); return v; }
		int ID()
		{
			int id = U8();
			if (id == 0xff)
				return -1;
			if (id == 0xfe)
			{
				u8 b[2];
				Read(b, 2);
				return b[0] | (b[1] << 8);
			}
			return id;
		}
		std::string CStr()
		{
			const u8* s = p;
			while (p < end && *p)
				++p;
			std::string out((const char*)s, p - s);
			if (p < end)
				++p;
			else
				bad = true;
			return out;
		}
		std::string SStr()
		{
			u32 len = U32();
			if (len > Left())
			{
				bad = true;
				return std::string();
			}
			std::string out((const char*)p, len);
			p += len;
			return out;
		}
		WVector Vec()
		{
			WVector v;
			v.x = F32();
			v.y = F32();
			v.z = F32();
			return v;
		}
	};

	struct Writer
	{
		ByteBuf& b;
		explicit Writer(ByteBuf& out) : b(out) { }

		void Write(const void* data, size_t n)
		{
			if (n)
				b.insert(b.end(), (const u8*)data, (const u8*)data + n);
		}
		void U8(u8 v) { b.push_back(v); }
		void U32(u32 v) { Write(&v, 4); }
		void S32(int v) { Write(&v, 4); }
		void F32(float v) { Write(&v, 4); }
		void ID(int id)
		{
			if (id < 0)
				U8(0xff);
			else if (id < 0xfe)
				U8((u8)id);
			else
			{
				U8(0xfe);
				U8((u8)id);
				U8((u8)(id >> 8));
			}
		}
		void CStr(const std::string& s) { Write(s.c_str(), s.size() + 1); }
		void SStr(const std::string& s)
		{
			U32((u32)s.size());
			Write(s.data(), s.size());
		}
		void Vec(const WVector& v)
		{
			F32(v.x);
			F32(v.y);
			F32(v.z);
		}
	};

	std::string FixedStr(const char* s, size_t n)
	{
		size_t len = 0;
		while (len < n && s[len])
			++len;
		return std::string(s, len);
	}

	void SetFixedStr(char* out, size_t n, const std::string& s)
	{
		memset(out, 0, n);
		size_t len = s.size() < n - 1 ? s.size() : n - 1;
		memcpy(out, s.data(), len);
	}
}

std::string PetTexture::Name() const { return FixedStr(filename, sizeof(filename)); }
void PetTexture::SetName(const std::string& name) { SetFixedStr(filename, sizeof(filename), name); }
std::string PetFaceAnim::Name() const { return FixedStr(name, sizeof(name)); }
std::string PetFaceAnim::File() const { return FixedStr(file, sizeof(file)); }
void PetFaceAnim::Set(const std::string& n, const std::string& f)
{
	SetFixedStr(name, sizeof(name), n);
	SetFixedStr(file, sizeof(file), f);
}

PetKind PetKindFromPath(const char* path)
{
	const char* dot = strrchr(path, '.');
	if (dot)
	{
		if (!_stricmp(dot, ".mpet"))
			return PET_MESH;
		if (!_stricmp(dot, ".bpet"))
			return PET_BONE;
		if (!_stricmp(dot, ".apet"))
			return PET_ANIM;
	}
	return PET_FULL;
}

const char* PetKindExt(PetKind kind)
{
	switch (kind)
	{
	case PET_MESH: return ".mpet";
	case PET_BONE: return ".bpet";
	case PET_ANIM: return ".apet";
	default: return ".pet";
	}
}

// -- PetFile -----------------------------------------------------------------

PetFile::PetFile()
{
	Clear();
}

void PetFile::Clear()
{
	kind = PET_FULL;
	verMajor = 1;
	verMinor = 0;
	chunks.clear();
	textures.clear();
	bones.clear();
	anims.clear();
	skins.clear();
	vertices.clear();
	points.clear();
	cornerUvCount.clear();
	extraUv.clear();
	triTextures.clear();
	triExtra.clear();
	faces.clear();
	frames.clear();
	motions.clear();
	colls.clear();
}

bool PetFile::HasChunk(const char* id) const
{
	for (size_t i = 0; i < chunks.size(); ++i)
		if (!strcmp(chunks[i].id, id))
			return true;
	return false;
}

void PetFile::AddChunk(const char* id)
{
	if (HasChunk(id))
		return;
	PetChunk c;
	memset(c.id, 0, sizeof(c.id));
	memcpy(c.id, id, 4);
	if (!strcmp(id, "VERS"))
	{
		c.raw.push_back(0xfe);
		c.raw.push_back(0xff);
	}
	else if (!strcmp(id, "SMTL"))
	{
		c.raw.assign(4, 0);		// count 0, all shipped files seen so far with no entries
	}
	chunks.push_back(c);
}

void PetFile::RemoveChunk(const char* id)
{
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		if (!strcmp(chunks[i].id, id))
		{
			chunks.erase(chunks.begin() + i);
			return;
		}
	}
}

int PetFile::FindBone(const std::string& name) const
{
	for (size_t i = 0; i < bones.size(); ++i)
		if (!_stricmp(bones[i].name.c_str(), name.c_str()))
			return (int)i;
	return -1;
}

static bool ReadBody(PetFile& pet, const char* id, Reader& r)
{
	if (!strcmp(id, "VERS"))
	{
		pet.verMinor = r.U8();
		pet.verMajor = r.U8();
		// The bytes FE FF follow in every file; they stay in the chunk's raw tail.
	}
	else if (!strcmp(id, "TEXT"))
	{
		u32 n = r.U32();
		if (n * 44ull > r.Left())
			return false;
		pet.textures.resize(n);
		for (u32 i = 0; i < n; ++i)
		{
			PetTexture& t = pet.textures[i];
			r.Read(t.filename, 32);
			t.flag = (char)r.U8();
			t.group = r.U8();
			r.Read(t.pad, 2);
			t.diffuse = r.U32();
			t.handle = r.S32();
		}
	}
	else if (!strcmp(id, "BONE"))
	{
		int n = r.U8();
		// Some empty puppets stop right after the zero, without the wide count.
		if (n == 0 && r.Left() >= 2)
		{
			n = r.U8();
			n |= r.U8() << 8;
		}
		pet.bones.resize(n);
		for (int i = 0; i < n && !r.bad; ++i)
		{
			PetBone& b = pet.bones[i];
			b.name = r.CStr();
			b.parent = r.ID();
			b.tail = -1.0f;
			if (pet.kind != PET_ANIM)
			{
				WMatrix& m = b.mat;
				float* f = &m.xx;
				for (int k = 0; k < 12; ++k)
					f[k] = r.F32();
				if (pet.VerAtLeast(1, 3))
					b.tail = r.F32();
			}
		}
	}
	else if (!strcmp(id, "ANIM"))
	{
		for (;;)
		{
			int bone = r.ID();
			if (bone < 0 || r.bad)
				break;
			pet.anims.push_back(PetAnim());
			PetAnim& a = pet.anims.back();
			a.bone = bone;
			u32 n = r.U32();
			if (n * 16ull > r.Left())
				return false;
			a.pos.resize(n);
			for (u32 i = 0; i < n; ++i)
			{
				a.pos[i].time = r.F32();
				a.pos[i].v = r.Vec();
			}
			n = r.U32();
			if (n * 20ull > r.Left())
				return false;
			a.rot.resize(n);
			for (u32 i = 0; i < n; ++i)
			{
				PetKeyRot& k = a.rot[i];
				k.time = r.F32();
				k.q.x = r.F32();
				k.q.y = r.F32();
				k.q.z = r.F32();
				k.q.w = r.F32();
			}
			// Every version has this count. v1.0 files always store 0 here, and the older
			// client this source comes from reads it without using it.
			n = r.U32();
			if (n * 16ull > r.Left())
				return false;
			a.scale.resize(n);
			for (u32 i = 0; i < n; ++i)
			{
				a.scale[i].time = r.F32();
				a.scale[i].v = r.Vec();
			}
			if (pet.VerAtLeast(1, 3))
			{
				n = r.U32();
				if (n * 8ull > r.Left())
					return false;
				a.flags.resize(n);
				for (u32 i = 0; i < n; ++i)
				{
					a.flags[i].time = r.F32();
					a.flags[i].value = r.F32();
				}
			}
		}
	}
	else if (!strcmp(id, "MESH"))
	{
		if (pet.kind == PET_MESH)
		{
			int n = r.U8();
			pet.skins.resize(n);
			for (int i = 0; i < n; ++i)
			{
				// Laid out as w_skin_info: the bone byte comes first, padded to 4.
				PetSkin& s = pet.skins[i];
				s.bone = r.U8();
				r.Read(s.pad, 3);
				s.startVtx = r.S32();
				s.numVtx = r.S32();
				s.startTri = r.S32();
				s.numTri = r.S32();
			}
		}
		u32 n = r.U32();
		if (n > r.Left())
			return false;
		pet.vertices.resize(n);
		for (u32 i = 0; i < n && !r.bad; ++i)
		{
			PetVertex& v = pet.vertices[i];
			v.pos = r.Vec();
			v.weight = pet.kind == PET_MESH ? r.F32() : 1.0f;
			v.pad[0] = v.pad[1] = 0;
			int total = 0;
			do
			{
				PetBlend b;
				b.weight = r.U8();
				b.bone = r.ID();
				v.blends.push_back(b);
				total += b.weight;
			} while (total < 255 && !r.bad);
			if (v.blends.size() == 1)
				r.Read(v.pad, 2);
		}
		n = r.U32();
		if (n * 72ull > r.Left())
			return false;
		pet.points.resize(n * 3);
		bool uvCounts = pet.VerAtLeast(1, 2);
		if (uvCounts)
			pet.cornerUvCount.resize(n * 3);
		for (u32 i = 0; i < n * 3; ++i)
		{
			PetTriPoint& t = pet.points[i];
			t.pos = r.U32();
			t.normal = r.Vec();
			int sets = 1;
			if (uvCounts)
			{
				sets = r.U8();
				pet.cornerUvCount[i] = (u8)sets;
			}
			t.u = t.v = 0.0f;
			if (sets > 0)
			{
				t.u = r.F32();
				t.v = r.F32();
			}
			for (int k = 1; k < sets; ++k)
			{
				pet.extraUv.push_back(r.F32());
				pet.extraUv.push_back(r.F32());
			}
		}
		pet.triTextures.resize(n);
		if (n)
			r.Read(&pet.triTextures[0], n);
		if (uvCounts)
		{
			pet.triExtra.resize(n);
			if (n)
				r.Read(&pet.triExtra[0], n);
		}
	}
	else if (!strcmp(id, "FANM"))
	{
		u32 n = r.U32();
		if (n * 65ull > r.Left())
			return false;
		pet.faces.resize(n);
		for (u32 i = 0; i < n; ++i)
		{
			pet.faces[i].group = r.U8();
			r.Read(pet.faces[i].name, 32);
			r.Read(pet.faces[i].file, 32);
		}
	}
	else if (!strcmp(id, "FRAM"))
	{
		u32 n = r.U32();
		if (n > r.Left())
			return false;
		pet.frames.resize(n);
		for (u32 i = 0; i < n; ++i)
		{
			PetFrame& f = pet.frames[i];
			f.frame = r.U32();
			f.script = r.SStr();
			f.unused1 = r.SStr();
			f.unused2 = r.SStr();
			// LoadPET_Frame returns early on a lone empty entry, before "check"
			f.hasCheck = r.Left() >= 4 && !(n <= 1 && f.script.empty());
			f.check = f.hasCheck ? r.U32() : 0;
		}
	}
	else if (!strcmp(id, "MOTI"))
	{
		u32 n = r.U32();
		if (n > r.Left())
			return false;
		pet.motions.resize(n);
		for (u32 i = 0; i < n; ++i)
		{
			PetMotion& m = pet.motions[i];
			m.name = r.SStr();
			m.startFrame = r.U32();
			m.endFrame = r.U32();
			m.next = r.SStr();
			m.option = r.SStr();
			m.optionValue = r.F32();
			m.applyBone = r.SStr();
		}
	}
	else if (!strcmp(id, "COLL"))
	{
		u32 n = r.U32();
		if (n > r.Left())
			return false;
		pet.colls.resize(n);
		for (u32 i = 0; i < n; ++i)
		{
			PetColl& c = pet.colls[i];
			c.shape = r.S32();
			c.visible = r.S32();
			c.name = r.SStr();
			c.bone = r.SStr();
			c.script0 = r.SStr();
			c.script1 = r.SStr();
			for (int k = 0; k < 6; ++k)
				c.box[k] = r.F32();
		}
	}
	else
	{
		return false;	// kept raw by the caller
	}
	return !r.bad;
}

static void WriteBody(const PetFile& pet, const char* id, Writer& w)
{
	if (!strcmp(id, "VERS"))
	{
		w.U8((u8)pet.verMinor);
		w.U8((u8)pet.verMajor);
	}
	else if (!strcmp(id, "TEXT"))
	{
		w.U32((u32)pet.textures.size());
		for (size_t i = 0; i < pet.textures.size(); ++i)
		{
			const PetTexture& t = pet.textures[i];
			w.Write(t.filename, 32);
			w.U8((u8)t.flag);
			w.U8(t.group);
			w.Write(t.pad, 2);
			w.U32(t.diffuse);
			w.S32(t.handle);
		}
	}
	else if (!strcmp(id, "BONE"))
	{
		int n = (int)pet.bones.size();
		if (n < 256)
			w.U8((u8)n);
		else
		{
			w.U8(0);
			w.U8((u8)n);
			w.U8((u8)(n >> 8));
		}
		for (int i = 0; i < n; ++i)
		{
			const PetBone& b = pet.bones[i];
			w.CStr(b.name);
			w.ID(b.parent);
			if (pet.kind != PET_ANIM)
			{
				w.Write(&b.mat.xx, 48);
				if (pet.VerAtLeast(1, 3))
					w.F32(b.tail);
			}
		}
	}
	else if (!strcmp(id, "ANIM"))
	{
		for (size_t i = 0; i < pet.anims.size(); ++i)
		{
			const PetAnim& a = pet.anims[i];
			w.ID(a.bone);
			w.U32((u32)a.pos.size());
			for (size_t k = 0; k < a.pos.size(); ++k)
			{
				w.F32(a.pos[k].time);
				w.Vec(a.pos[k].v);
			}
			w.U32((u32)a.rot.size());
			for (size_t k = 0; k < a.rot.size(); ++k)
			{
				const PetKeyRot& q = a.rot[k];
				w.F32(q.time);
				w.F32(q.q.x);
				w.F32(q.q.y);
				w.F32(q.q.z);
				w.F32(q.q.w);
			}
			w.U32((u32)a.scale.size());
			for (size_t k = 0; k < a.scale.size(); ++k)
			{
				w.F32(a.scale[k].time);
				w.Vec(a.scale[k].v);
			}
			if (pet.VerAtLeast(1, 3))
			{
				w.U32((u32)a.flags.size());
				for (size_t k = 0; k < a.flags.size(); ++k)
				{
					w.F32(a.flags[k].time);
					w.F32(a.flags[k].value);
				}
			}
		}
		w.U8(0xff);
	}
	else if (!strcmp(id, "MESH"))
	{
		if (pet.kind == PET_MESH)
		{
			w.U8((u8)pet.skins.size());
			for (size_t i = 0; i < pet.skins.size(); ++i)
			{
				const PetSkin& s = pet.skins[i];
				w.U8(s.bone);
				w.Write(s.pad, 3);
				w.S32(s.startVtx);
				w.S32(s.numVtx);
				w.S32(s.startTri);
				w.S32(s.numTri);
			}
		}
		w.U32((u32)pet.vertices.size());
		for (size_t i = 0; i < pet.vertices.size(); ++i)
		{
			const PetVertex& v = pet.vertices[i];
			w.Vec(v.pos);
			if (pet.kind == PET_MESH)
				w.F32(v.weight);
			for (size_t k = 0; k < v.blends.size(); ++k)
			{
				w.U8(v.blends[k].weight);
				w.ID(v.blends[k].bone);
			}
			if (v.blends.size() == 1)
				w.Write(v.pad, 2);
		}
		w.U32((u32)(pet.points.size() / 3));
		size_t extra = 0;
		for (size_t i = 0; i < pet.points.size(); ++i)
		{
			const PetTriPoint& t = pet.points[i];
			w.U32(t.pos);
			w.Vec(t.normal);
			int sets = 1;
			if (pet.VerAtLeast(1, 2))
			{
				sets = i < pet.cornerUvCount.size() ? pet.cornerUvCount[i] : 1;
				w.U8((u8)sets);
			}
			if (sets > 0)
			{
				w.F32(t.u);
				w.F32(t.v);
			}
			for (int k = 1; k < sets; ++k)
			{
				w.F32(pet.extraUv[extra++]);
				w.F32(pet.extraUv[extra++]);
			}
		}
		if (!pet.triTextures.empty())
			w.Write(&pet.triTextures[0], pet.triTextures.size());
		if (pet.VerAtLeast(1, 2))
		{
			// A puppet upgraded to this version has none yet; zero is what plain meshes store.
			for (size_t i = 0; i < pet.triTextures.size(); ++i)
				w.U8(i < pet.triExtra.size() ? pet.triExtra[i] : 0);
		}
	}
	else if (!strcmp(id, "FANM"))
	{
		w.U32((u32)pet.faces.size());
		for (size_t i = 0; i < pet.faces.size(); ++i)
		{
			w.U8(pet.faces[i].group);
			w.Write(pet.faces[i].name, 32);
			w.Write(pet.faces[i].file, 32);
		}
	}
	else if (!strcmp(id, "FRAM"))
	{
		w.U32((u32)pet.frames.size());
		for (size_t i = 0; i < pet.frames.size(); ++i)
		{
			const PetFrame& f = pet.frames[i];
			w.U32(f.frame);
			w.SStr(f.script);
			w.SStr(f.unused1);
			w.SStr(f.unused2);
			if (f.hasCheck)
				w.U32(f.check);
		}
	}
	else if (!strcmp(id, "MOTI"))
	{
		w.U32((u32)pet.motions.size());
		for (size_t i = 0; i < pet.motions.size(); ++i)
		{
			const PetMotion& m = pet.motions[i];
			w.SStr(m.name);
			w.U32(m.startFrame);
			w.U32(m.endFrame);
			w.SStr(m.next);
			w.SStr(m.option);
			w.F32(m.optionValue);
			w.SStr(m.applyBone);
		}
	}
	else if (!strcmp(id, "COLL"))
	{
		w.U32((u32)pet.colls.size());
		for (size_t i = 0; i < pet.colls.size(); ++i)
		{
			const PetColl& c = pet.colls[i];
			w.S32(c.shape);
			w.S32(c.visible);
			w.SStr(c.name);
			w.SStr(c.bone);
			w.SStr(c.script0);
			w.SStr(c.script1);
			for (int k = 0; k < 6; ++k)
				w.F32(c.box[k]);
		}
	}
}

static bool IsKnownChunk(const char* id)
{
	static const char* known[] = { "VERS", "TEXT", "BONE", "ANIM", "MESH", "FANM", "FRAM", "MOTI", "COLL" };
	for (size_t i = 0; i < sizeof(known) / sizeof(known[0]); ++i)
		if (!strcmp(id, known[i]))
			return true;
	return false;
}

static bool IsPlausibleId(const char* id)
{
	for (int i = 0; i < 4; ++i)
		if (id[i] < 'A' || id[i] > 'Z')
			return false;
	return true;
}

bool PetFile::LoadBuffer(const u8* data, size_t size, PetKind petKind, std::string* error)
{
	Clear();
	kind = petKind;
	size_t pos = 0;
	char msg[128];
	while (pos < size)
	{
		PetChunk c;
		memset(c.id, 0, sizeof(c.id));
		u32 len = 0;
		if (size - pos >= 8)
		{
			memcpy(c.id, data + pos, 4);
			memcpy(&len, data + pos + 4, 4);
		}
		// A few shipped .mpet files have a MESH size that lands inside the next chunk.
		// SeekChunkFromPET walks off the same way, so the rest is dead data: keep it as is.
		if (size - pos < 8 || !IsPlausibleId(c.id) || len > size - pos - 8)
		{
			memset(c.id, 0, sizeof(c.id));
			c.raw.assign(data + pos, data + size);
			chunks.push_back(c);
			break;
		}
		pos += 8;
		Reader r(data + pos, len);
		if (IsKnownChunk(c.id))
		{
			if (!ReadBody(*this, c.id, r))
			{
				if (error)
				{
					sprintf_s(msg, "chunk %s is malformed", c.id);
					*error = msg;
				}
				return false;
			}
			// bytes past the parsed structure, kept for the round trip
			if (r.Left())
				c.raw.assign(r.p, r.end);
		}
		else if (len)
		{
			c.raw.assign(data + pos, data + pos + len);
		}
		chunks.push_back(c);
		pos += len;
	}
	return true;
}

bool PetFile::Load(const char* path, std::string* error)
{
	FILE* f = OpenUtf8(path, L"rb");
	if (!f)
	{
		if (error)
			*error = "cannot open file";
		return false;
	}
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	ByteBuf data(size);
	if (size)
		fread(&data[0], 1, size, f);
	fclose(f);
	return LoadBuffer(size ? &data[0] : 0, size, PetKindFromPath(path), error);
}

void PetFile::SaveBuffer(ByteBuf* out) const
{
	out->clear();
	Writer w(*out);
	for (size_t i = 0; i < chunks.size(); ++i)
	{
		const PetChunk& c = chunks[i];
		if (!c.id[0])
		{
			w.Write(&c.raw[0], c.raw.size());
			continue;
		}
		w.Write(c.id, 4);
		size_t sizePos = out->size();
		w.U32(0);
		if (IsKnownChunk(c.id))
			WriteBody(*this, c.id, w);
		if (!c.raw.empty())
			w.Write(&c.raw[0], c.raw.size());
		u32 len = (u32)(out->size() - sizePos - 4);
		memcpy(&(*out)[sizePos], &len, 4);
	}
}

bool PetFile::Save(const char* path, std::string* error) const
{
	ByteBuf data;
	SaveBuffer(&data);
	FILE* f = OpenUtf8(path, L"wb");
	if (!f)
	{
		if (error)
			*error = "cannot create file";
		return false;
	}
	if (!data.empty())
		fwrite(&data[0], 1, data.size(), f);
	fclose(f);
	return true;
}
