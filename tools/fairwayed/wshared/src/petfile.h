// SPDX-License-Identifier: MIT
// petfile.h - in-memory form of a Puppet (.pet/.mpet/.bpet/.apet) file.
//
// Every byte the format stores is kept, including uninitialised padding that
// the original exporter leaked into the fixed-size strings, so an unmodified
// file saves back byte for byte. Strings stay in the game's CP949.
#ifndef PETFILE_H
#define PETFILE_H

#include <string>
#include <vector>
#include "wmath.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef std::vector<u8> ByteBuf;

enum PetKind
{
	PET_FULL,	// .pet
	PET_MESH,	// .mpet (skin table, per-vertex weight)
	PET_BONE,	// .bpet
	PET_ANIM	// .apet (bones have no rest matrix)
};

// WPuppet::LoadPET_Texture: drawing flags live in the first three characters of a name.
inline bool NameHasFlag(const std::string& name, const char* flags) { return name.find_first_of(flags) < 3; }

struct PetTexture
{
	char filename[32];	// may carry garbage after the terminator
	char flag;			// 0 = may be atlas packed
	u8 group;
	u8 pad[2];
	u32 diffuse;		// ARGB
	int handle;

	std::string Name() const;
	void SetName(const std::string& name);
};

struct PetBone
{
	std::string name;
	int parent;
	WMatrix mat;
	float tail;			// v1.3+: extra float after the matrix, always -1.0 so far
};

struct PetKeyPos { float time; WVector v; };
struct PetKeyRot { float time; WQuat q; };
struct PetKeyFlag { float time; float value; };

struct PetAnim
{
	int bone;
	std::vector<PetKeyPos> pos;
	std::vector<PetKeyRot> rot;
	std::vector<PetKeyPos> scale;
	std::vector<PetKeyFlag> flags;
};

struct PetBlend { u8 weight; int bone; };

struct PetVertex
{
	WVector pos;		// local to blends[0].bone
	float weight;		// .mpet only
	std::vector<PetBlend> blends;
	u8 pad[2];			// present when there is a single blend
};

struct PetTriPoint
{
	u32 pos;
	WVector normal;
	float u, v;
};

struct PetSkin
{
	u8 bone;
	u8 pad[3];
	int startVtx, numVtx, startTri, numTri;
};

struct PetFaceAnim
{
	u8 group;
	char name[32];
	char file[32];

	std::string Name() const;
	std::string File() const;
	void Set(const std::string& name, const std::string& file);
};

struct PetFrame
{
	u32 frame;
	std::string script, unused1, unused2;
	u32 check;
	bool hasCheck;
};

struct PetMotion
{
	std::string name;
	u32 startFrame, endFrame;
	std::string next, option;
	float optionValue;
	std::string applyBone;
};

struct PetColl
{
	int shape;			// 0 sphere, 1 box
	int visible;
	std::string name, bone, script0, script1;
	float box[6];		// min xyz, max xyz
};

// One entry per chunk in file order; decides what Save() writes and where.
struct PetChunk
{
	char id[5];			// empty: undecodable bytes at the end of a damaged file
	ByteBuf raw;		// unknown chunk body, or known-chunk trailing bytes
};

class PetFile
{
public:
	PetFile();
	void Clear();

	bool Load(const char* path, std::string* error);
	bool LoadBuffer(const u8* data, size_t size, PetKind kind, std::string* error);
	void SaveBuffer(ByteBuf* out) const;
	bool Save(const char* path, std::string* error) const;

	bool HasChunk(const char* id) const;
	void AddChunk(const char* id);		// no-op when present
	void RemoveChunk(const char* id);
	int FindBone(const std::string& name) const;

	bool VerAtLeast(int major, int minor) const
	{
		return verMajor > major || (verMajor == major && verMinor >= minor);
	}
	bool VerIs(int major, int minor) const { return verMajor == major && verMinor == minor; }

	PetKind kind;
	int verMajor, verMinor;
	std::vector<PetChunk> chunks;
	std::vector<PetTexture> textures;
	std::vector<PetBone> bones;
	std::vector<PetAnim> anims;
	std::vector<PetSkin> skins;
	std::vector<PetVertex> vertices;
	std::vector<PetTriPoint> points;	// 3 per triangle
	// v1.2+: each corner stores a UV count before its UVs (nearly always 1).
	std::vector<u8> cornerUvCount;		// empty before v1.2
	std::vector<float> extraUv;			// the UVs past the first, in corner order
	std::vector<u8> triTextures;
	std::vector<u8> triExtra;			// v1.2+: a second byte per triangle, meaning unknown
	std::vector<PetFaceAnim> faces;
	std::vector<PetFrame> frames;
	std::vector<PetMotion> motions;
	std::vector<PetColl> colls;
};

PetKind PetKindFromPath(const char* path);
const char* PetKindExt(PetKind kind);

#endif // PETFILE_H
