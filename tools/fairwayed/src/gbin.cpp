// SPDX-License-Identifier: MIT
// gbin.cpp - reads and writes .gbin, .sgbin, .aibin and .sbin.
#include "gbin.h"
#include <stdio.h>
#include <string.h>

// -- helpers ------------------------------------------------------------------

void FixedCopy(char* dst, size_t size, const std::string& s)
{
	memset(dst, 0, size);
	size_t n = s.size() < size - 1 ? s.size() : size - 1;
	memcpy(dst, s.data(), n);
}

std::string FixedStr(const char* s, size_t size)
{
	size_t n = 0;
	while (n < size && s[n])
		++n;
	return std::string(s, n);
}

namespace
{
	struct Bad { const char* what; };

	class In
	{
	public:
		In(const std::vector<u8>& d) : data(d), at(0) { }
		void Raw(void* dst, size_t n)
		{
			if (n > data.size() - at)
				throw Bad{ "unexpected end of file" };
			if (n)
				memcpy(dst, &data[at], n);
			at += n;
		}
		int Int() { int v; Raw(&v, 4); return v; }
		u32 U32() { u32 v; Raw(&v, 4); return v; }
		u8 Byte() { u8 v; Raw(&v, 1); return v; }
		int Count(size_t minEach)
		{
			int n = Int();
			if (n < 0 || (minEach && (size_t)n > (data.size() - at) / minEach))
				throw Bad{ "implausible count" };
			return n;
		}
		const std::vector<u8>& data;
		size_t at;
	};

	class Out
	{
	public:
		Out(std::vector<u8>* d) : data(d) { }
		void Raw(const void* src, size_t n)
		{
			if (n)
				data->insert(data->end(), (const u8*)src, (const u8*)src + n);
		}
		void Int(int v) { Raw(&v, 4); }
		void U32(u32 v) { Raw(&v, 4); }
		void Byte(u8 v) { Raw(&v, 1); }
		std::vector<u8>* data;
	};
}

// -- .gbin --------------------------------------------------------------------

void GbinFile::Clear()
{
	memcpy(id, "WEPX", 4);
	version = 0x71;
	kind = GBIN_GBIN;
	modelNum[0] = modelNum[1] = modelNum[2] = 0;
	cameras.clear();
	points.clear();
	areas.clear();
	textures.clear();
	nodes.clear();
	hasBase = false;
	memset(&base, 0, sizeof(base));
	baseColors.clear();
	boneColors.clear();
	models.clear();
	groups.clear();
	par = 0;
	memset(&hole, 0, sizeof(hole));
	gap.clear();
}

static void ReadModel(In& in, GbinModel* m, bool v72)
{
	m->flag = in.Byte();
	in.Raw(m->pad, 3);
	m->faceNum = in.Int();
	in.Raw(m->name, 32);
	in.Raw(m->localAabb, 24);
	in.Raw(m->sceneAabb, 24);
	in.Raw(m->mat, 48);
	m->detail = in.Int();
	in.Raw(m->option, 32);
	if (v72)
	{
		in.Raw(m->localMat, 48);
		in.Raw(&m->link, 8);
	}
	else
	{
		memset(m->localMat, 0, sizeof(m->localMat));
		m->link.id = 0;
		m->link.parent = -1;
	}
}

static void WriteModel(Out& out, const GbinModel& m, bool v72)
{
	out.Byte(m.flag);
	out.Raw(m.pad, 3);
	out.Int(m.faceNum);
	out.Raw(m.name, 32);
	out.Raw(m.localAabb, 24);
	out.Raw(m.sceneAabb, 24);
	out.Raw(m.mat, 48);
	out.Int(m.detail);
	out.Raw(m.option, 32);
	if (v72)
	{
		out.Raw(m.localMat, 48);
		out.Raw(&m.link, 8);
	}
}

bool GbinFile::Parse(const std::vector<u8>& data, GbinKind k, std::string* error)
{
	Clear();
	kind = k;
	try
	{
		In in(data);
		in.Raw(id, 4);
		version = in.U32();
		if (version != 0x71 && version != 0x72)
			throw Bad{ "unsupported version" };
		bool v72 = version >= 0x72;
		modelNum[0] = in.Int();
		modelNum[1] = in.Int();
		modelNum[2] = in.Int();
		int cameraNum = in.Count(64), pointNum = in.Count(45), areaNum = in.Count(89);
		int textureNum = in.Count(32), nodeNum = in.Count(24);
		int groupNum = v72 ? in.Count(0) : 0;

		cameras.resize(cameraNum);
		for (int i = 0; i < cameraNum; ++i)
		{
			GbinCamera& c = cameras[i];
			in.Raw(c.name, 32);
			in.Raw(c.eye, 12);
			in.Raw(c.at, 12);
			in.Raw(&c.fov, 4);
			c.reserved = in.Int();
			memset(c.localEye, 0, 24);
			c.link.id = 0;
			c.link.parent = -1;
			if (v72)
			{
				in.Raw(c.localEye, 12);
				in.Raw(c.localAt, 12);
				in.Raw(&c.link, 8);
			}
		}
		points.resize(pointNum);
		for (int i = 0; i < pointNum; ++i)
		{
			GbinPoint& p = points[i];
			p.type = in.Byte();
			in.Raw(p.name, 32);
			in.Raw(p.pos, 12);
			memset(p.option, 0, 64);
			if (p.type)
				in.Raw(p.option, 64);
			memset(p.localPos, 0, 12);
			p.link.id = 0;
			p.link.parent = -1;
			if (v72)
			{
				in.Raw(p.localPos, 12);
				in.Raw(&p.link, 8);
			}
		}
		areas.resize(areaNum);
		for (int i = 0; i < areaNum; ++i)
		{
			GbinArea& a = areas[i];
			a.type = in.Byte();
			in.Raw(a.name, 64);
			in.Raw(a.box, 24);
			memset(a.localBox, 0, 24);
			a.link.id = 0;
			a.link.parent = -1;
			if (v72)
			{
				in.Raw(a.localBox, 24);
				in.Raw(&a.link, 8);
			}
		}
		textures.resize(textureNum);
		for (int i = 0; i < textureNum; ++i)
		{
			char buf[32];
			in.Raw(buf, 32);
			textures[i].assign(buf, 32);
		}
		size_t nameLen = kind == GBIN_AIBIN ? 32 : 16;
		nodes.resize(nodeNum);
		for (int i = 0; i < nodeNum; ++i)
		{
			GbinNode& n = nodes[i];
			char buf[32];
			in.Raw(buf, nameLen);
			n.name.assign(buf, nameLen);
			int num = in.Count(0);
			n.reserved = in.Int();
			if ((size_t)num > (data.size() - in.at) / 12)
				throw Bad{ "implausible node size" };
			n.points.resize(num);
			if (num)
				in.Raw(&n.points[0], num * 12);
		}
		if (kind == GBIN_GBIN)
		{
			hasBase = true;
			ReadModel(in, &base, v72);
			if (v72)
			{
				int bones = in.Count(8);
				boneColors.resize(bones);
				for (int i = 0; i < bones; ++i)
				{
					int len = in.Count(1);
					boneColors[i].name.resize(len);
					if (len)
						in.Raw(&boneColors[i].name[0], len);
					int meshes = in.Count(4);
					boneColors[i].meshes.resize(meshes);
					for (int m = 0; m < meshes; ++m)
					{
						int num = in.Count(4);
						boneColors[i].meshes[m].colors.resize(num);
						if (num)
							in.Raw(&boneColors[i].meshes[m].colors[0], num * 4);
					}
				}
			}
			else
			{
				if (base.faceNum < 0 || (size_t)base.faceNum * 12 > data.size() - in.at)
					throw Bad{ "base model colours run past the end" };
				baseColors.resize(base.faceNum * 3);
				if (base.faceNum)
					in.Raw(&baseColors[0], base.faceNum * 12);
			}
		}
		int total = modelNum[0] + modelNum[1] + modelNum[2];
		if (total < 0)
			throw Bad{ "implausible model count" };
		models.resize(total);
		for (int i = 0; i < total; ++i)
			ReadModel(in, &models[i], v72);
		if (v72)
		{
			groups.resize(groupNum);
			for (int i = 0; i < groupNum; ++i)
			{
				GbinGroup& g = groups[i];
				in.Raw(&g.link, 8);
				in.Raw(g.name, 32);
				in.Raw(g.unknown1, 32);
				in.Raw(g.mat, 48);
				in.Raw(g.localMat, 48);
				g.unknown2 = in.Int();
			}
			if (kind == GBIN_GBIN)
				par = in.Byte();
			if (in.at != data.size())
				throw Bad{ "bytes left at the end of the file" };
		}
		else
		{
			if (kind == GBIN_GBIN)
			{
				if (data.size() < in.at + 0x41)
					throw Bad{ "no room for the hole info" };
				gap.assign(data.begin() + in.at, data.end() - 0x41);
				in.at = data.size() - 0x41;
				hole.par = in.Byte();
				in.Raw(hole.tee, 16);
				in.Raw(hole.pin, 48);
				par = hole.par;
			}
			else if (in.at != data.size())
			{
				gap.assign(data.begin() + in.at, data.end());
			}
		}
	}
	catch (const Bad& b)
	{
		*error = b.what;
		return false;
	}
	return true;
}

void GbinFile::Serialize(std::vector<u8>* result) const
{
	result->clear();
	Out out(result);
	bool v72 = version >= 0x72;
	out.Raw(id, 4);
	out.U32(version);
	out.Raw(modelNum, 12);
	out.Int((int)cameras.size());
	out.Int((int)points.size());
	out.Int((int)areas.size());
	out.Int((int)textures.size());
	out.Int((int)nodes.size());
	if (v72)
		out.Int((int)groups.size());
	for (size_t i = 0; i < cameras.size(); ++i)
	{
		const GbinCamera& c = cameras[i];
		out.Raw(c.name, 32);
		out.Raw(c.eye, 12);
		out.Raw(c.at, 12);
		out.Raw(&c.fov, 4);
		out.Int(c.reserved);
		if (v72)
		{
			out.Raw(c.localEye, 12);
			out.Raw(c.localAt, 12);
			out.Raw(&c.link, 8);
		}
	}
	for (size_t i = 0; i < points.size(); ++i)
	{
		const GbinPoint& p = points[i];
		out.Byte(p.type);
		out.Raw(p.name, 32);
		out.Raw(p.pos, 12);
		if (p.type)
			out.Raw(p.option, 64);
		if (v72)
		{
			out.Raw(p.localPos, 12);
			out.Raw(&p.link, 8);
		}
	}
	for (size_t i = 0; i < areas.size(); ++i)
	{
		const GbinArea& a = areas[i];
		out.Byte(a.type);
		out.Raw(a.name, 64);
		out.Raw(a.box, 24);
		if (v72)
		{
			out.Raw(a.localBox, 24);
			out.Raw(&a.link, 8);
		}
	}
	for (size_t i = 0; i < textures.size(); ++i)
		out.Raw(textures[i].data(), 32);
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const GbinNode& n = nodes[i];
		out.Raw(n.name.data(), n.name.size());
		out.Int((int)n.points.size());
		out.Int(n.reserved);
		if (!n.points.empty())
			out.Raw(&n.points[0], n.points.size() * 12);
	}
	if (hasBase)
	{
		WriteModel(out, base, v72);
		if (v72)
		{
			out.Int((int)boneColors.size());
			for (size_t i = 0; i < boneColors.size(); ++i)
			{
				out.Int((int)boneColors[i].name.size());
				out.Raw(boneColors[i].name.data(), boneColors[i].name.size());
				out.Int((int)boneColors[i].meshes.size());
				for (size_t m = 0; m < boneColors[i].meshes.size(); ++m)
				{
					const std::vector<u32>& c = boneColors[i].meshes[m].colors;
					out.Int((int)c.size());
					if (!c.empty())
						out.Raw(&c[0], c.size() * 4);
				}
			}
		}
		else if (!baseColors.empty())
		{
			out.Raw(&baseColors[0], baseColors.size() * 4);
		}
	}
	for (size_t i = 0; i < models.size(); ++i)
		WriteModel(out, models[i], v72);
	if (v72)
	{
		for (size_t i = 0; i < groups.size(); ++i)
		{
			const GbinGroup& g = groups[i];
			out.Raw(&g.link, 8);
			out.Raw(g.name, 32);
			out.Raw(g.unknown1, 32);
			out.Raw(g.mat, 48);
			out.Raw(g.localMat, 48);
			out.Int(g.unknown2);
		}
		if (kind == GBIN_GBIN)
			out.Byte(par);
	}
	else
	{
		if (!gap.empty())
			out.Raw(&gap[0], gap.size());
		if (kind == GBIN_GBIN)
		{
			out.Byte(hole.par);
			out.Raw(hole.tee, 16);
			out.Raw(hole.pin, 48);
		}
	}
}

static GbinKind KindOfPath(const std::string& path)
{
	size_t dot = path.rfind('.');
	std::string ext = dot == std::string::npos ? std::string() : path.substr(dot + 1);
	if (!_stricmp(ext.c_str(), "aibin"))
		return GBIN_AIBIN;
	if (!_stricmp(ext.c_str(), "sgbin"))
		return GBIN_SGBIN;
	return GBIN_GBIN;
}

bool GbinFile::Load(const std::string& path, std::string* error)
{
	std::vector<u8> data;
	if (!ReadWholeFile(path, &data))
	{
		*error = "cannot open " + path;
		return false;
	}
	return Parse(data, KindOfPath(path), error);
}

bool GbinFile::Save(const std::string& path, std::string* error) const
{
	std::vector<u8> data;
	Serialize(&data);
	return WriteWholeFile(path, data, error);
}

// -- .sbin --------------------------------------------------------------------

static const u32 SBIN_NEW_MAGIC = 0xFFFE0100;

static void ReadBody(In& in, SbinBody* b, bool isNew)
{
	if (isNew)
	{
		int len = in.Count(1);
		b->name.resize(len);
		if (len)
			in.Raw(&b->name[0], len);
	}
	int meshNum = in.Count(8);
	b->meshVtx.resize(meshNum);
	b->meshIdx.resize(meshNum);
	for (int i = 0; i < meshNum; ++i)
	{
		b->meshVtx[i] = in.Int();
		b->meshIdx[i] = in.Int();
	}
	int vtxNum = in.Count(0), uvNum = in.Count(0);
	if ((size_t)vtxNum > in.data.size() / 12 || (size_t)uvNum > in.data.size() / 8)
		throw Bad{ "implausible vertex count" };
	b->vtx.resize(vtxNum * 3);
	b->uvs.resize(uvNum * 2);
	if (vtxNum)
		in.Raw(&b->vtx[0], vtxNum * 12);
	if (uvNum)
		in.Raw(&b->uvs[0], uvNum * 8);
	for (int s = 0; s < 3; ++s)
	{
		SbinSection& sec = b->sections[s];
		sec.size = in.Int();
		if (sec.size <= 0)
			continue;
		size_t start = in.at;
		int num = in.Count(12);
		sec.packs.resize(num);
		for (int p = 0; p < num; ++p)
		{
			SbinPack& pack = sec.packs[p];
			pack.width = in.Int();
			pack.height = in.Int();
			if (pack.width < 0 || pack.height < 0 || pack.width > 65536 || pack.height > 65536)
				throw Bad{ "implausible shadow map size" };
			pack.dxt1.resize(pack.width * pack.height / 2);
			if (!pack.dxt1.empty())
				in.Raw(&pack.dxt1[0], pack.dxt1.size());
			int frags = in.Count(sizeof(SbinInFrag));
			pack.frags.resize(frags);
			if (frags)
				in.Raw(&pack.frags[0], frags * sizeof(SbinInFrag));
		}
		if (in.at - start != (size_t)sec.size)
			throw Bad{ "section size disagrees with its contents" };
	}
	in.Raw(b->outNum, 12);
	for (int s = 0; s < 3; ++s)
	{
		if (b->outNum[s] > 0)
		{
			b->outCounts[s].resize(meshNum);
			if (meshNum)
				in.Raw(&b->outCounts[s][0], meshNum * 4);
		}
	}
	for (int s = 0; s < 3; ++s)
	{
		if (b->outNum[s] > 0)
		{
			if ((size_t)b->outNum[s] > (in.data.size() - in.at) / sizeof(SbinOutFrag))
				throw Bad{ "implausible fragment count" };
			b->outFrags[s].resize(b->outNum[s]);
			in.Raw(&b->outFrags[s][0], b->outNum[s] * sizeof(SbinOutFrag));
		}
	}
}

bool SbinFile::Parse(const std::vector<u8>& data, std::string* error)
{
	Clear();
	try
	{
		In in(data);
		u32 magic = data.size() >= 4 ? *(const u32*)&data[0] : 0;
		isNew = magic == SBIN_NEW_MAGIC;
		int count = 1;
		if (isNew)
		{
			in.U32();
			count = in.Count(8);
		}
		bodies.resize(count);
		for (int i = 0; i < count; ++i)
			ReadBody(in, &bodies[i], isNew);
		if (in.at != data.size())
			throw Bad{ "bytes left at the end of the file" };
	}
	catch (const Bad& b)
	{
		*error = b.what;
		return false;
	}
	return true;
}

void SbinFile::Serialize(std::vector<u8>* result) const
{
	result->clear();
	Out out(result);
	if (isNew)
	{
		out.U32(SBIN_NEW_MAGIC);
		out.Int((int)bodies.size());
	}
	for (size_t i = 0; i < bodies.size(); ++i)
	{
		const SbinBody& b = bodies[i];
		if (isNew)
		{
			out.Int((int)b.name.size());
			out.Raw(b.name.data(), b.name.size());
		}
		out.Int((int)b.meshVtx.size());
		for (size_t m = 0; m < b.meshVtx.size(); ++m)
		{
			out.Int(b.meshVtx[m]);
			out.Int(b.meshIdx[m]);
		}
		out.Int((int)b.vtx.size() / 3);
		out.Int((int)b.uvs.size() / 2);
		if (!b.vtx.empty())
			out.Raw(&b.vtx[0], b.vtx.size() * 4);
		if (!b.uvs.empty())
			out.Raw(&b.uvs[0], b.uvs.size() * 4);
		for (int s = 0; s < 3; ++s)
		{
			const SbinSection& sec = b.sections[s];
			out.Int(sec.size);
			if (sec.size <= 0)
				continue;
			out.Int((int)sec.packs.size());
			for (size_t p = 0; p < sec.packs.size(); ++p)
			{
				const SbinPack& pack = sec.packs[p];
				out.Int(pack.width);
				out.Int(pack.height);
				if (!pack.dxt1.empty())
					out.Raw(&pack.dxt1[0], pack.dxt1.size());
				out.Int((int)pack.frags.size());
				if (!pack.frags.empty())
					out.Raw(&pack.frags[0], pack.frags.size() * sizeof(SbinInFrag));
			}
		}
		out.Raw(b.outNum, 12);
		for (int s = 0; s < 3; ++s)
			if (b.outNum[s] > 0 && !b.outCounts[s].empty())
				out.Raw(&b.outCounts[s][0], b.outCounts[s].size() * 4);
		for (int s = 0; s < 3; ++s)
			if (b.outNum[s] > 0)
				out.Raw(&b.outFrags[s][0], b.outFrags[s].size() * sizeof(SbinOutFrag));
	}
}

bool SbinFile::Load(const std::string& path, std::string* error)
{
	std::vector<u8> data;
	if (!ReadWholeFile(path, &data))
	{
		*error = "cannot open " + path;
		return false;
	}
	return Parse(data, error);
}

bool SbinFile::Save(const std::string& path, std::string* error) const
{
	std::vector<u8> data;
	Serialize(&data);
	return WriteWholeFile(path, data, error);
}
