// SPDX-License-Identifier: MIT
// gbin.h - compiled hole data (.gbin, .sgbin, .aibin) and shadow data (.sbin).
//
// Layouts follow WPolySoup::Load, LoadMapCheckData and LoadShadowmapInfo for 0x71, and
// docs/gbin.hexpat in rebang for 0x72. Fixed strings are kept with whatever the exporter
// left past their terminator, so an unmodified file saves back byte for byte.
#ifndef GBIN_H
#define GBIN_H

#include <string>
#include <vector>
#include "petfile.h"
#include "wcommon.h"

enum GbinKind { GBIN_GBIN, GBIN_SGBIN, GBIN_AIBIN };

struct GbinLink { int id, parent; };

struct GbinCamera
{
	char name[32];
	float eye[3], at[3], fov;
	int reserved;
	float localEye[3], localAt[3];
	GbinLink link;
};

struct GbinPoint
{
	u8 type;				// 0 marker, 1 light, 2 sun, 3 moon
	char name[32];
	float pos[3];
	char option[64];		// not stored for a marker
	float localPos[3];
	GbinLink link;
};

struct GbinArea
{
	u8 type;				// 0 event, 1 npc, 2 lake, 5 sound, 6 fx
	char name[64];
	float box[6];
	float localBox[6];
	GbinLink link;
};

struct GbinNode
{
	std::string name;		// raw, 16 bytes or 32 in .aibin
	int reserved;
	std::vector<WVector> points;
};

// sBgModel.
struct GbinModel
{
	u8 flag;
	u8 pad[3];
	int faceNum;
	char name[32];
	float localAabb[6];
	float sceneAabb[6];
	float mat[12];
	int detail;
	char option[32];
	float localMat[12];
	GbinLink link;
};

struct GbinMeshColors { std::vector<u32> colors; };
struct GbinBoneColors { std::string name; std::vector<GbinMeshColors> meshes; };

struct GbinGroup
{
	GbinLink link;
	char name[32];
	u8 unknown1[32];
	float mat[12];
	float localMat[12];
	int unknown2;
};

struct GbinHoleInfo
{
	u8 par;
	float tee[2][2];
	float pin[6][2];
};

class GbinFile
{
public:
	GbinFile() { Clear(); }
	void Clear();
	bool Load(const std::string& utf8Path, std::string* error);
	bool Parse(const std::vector<u8>& data, GbinKind kind, std::string* error);
	void Serialize(std::vector<u8>* out) const;
	bool Save(const std::string& utf8Path, std::string* error) const;

	char id[4];
	u32 version;
	GbinKind kind;
	int modelNum[3];
	std::vector<GbinCamera> cameras;
	std::vector<GbinPoint> points;
	std::vector<GbinArea> areas;
	std::vector<std::string> textures;	// raw 32 bytes each
	std::vector<GbinNode> nodes;
	bool hasBase;
	GbinModel base;
	std::vector<u32> baseColors;		// 0x71: one per triangle corner, in mesh order
	std::vector<GbinBoneColors> boneColors;	// 0x72
	std::vector<GbinModel> models;
	std::vector<GbinGroup> groups;
	u8 par;								// 0x72
	GbinHoleInfo hole;					// 0x71
	std::vector<u8> gap;				// unread bytes before the hole info, if any
};

// -- .sbin --------------------------------------------------------------------

struct SbinInFrag
{
	u8 mesh, pad;
	unsigned short tri;
	unsigned short vtx[3];
	unsigned short uv[3];
};

struct SbinOutFrag { unsigned short tri; unsigned short vtx[3]; };

struct SbinPack
{
	int width, height;
	std::vector<u8> dxt1;
	std::vector<SbinInFrag> frags;
};

struct SbinSection
{
	int size;			// stored; 0 means empty
	std::vector<SbinPack> packs;
};

struct SbinBody
{
	std::string name;	// newer form only
	std::vector<int> meshVtx, meshIdx;
	std::vector<float> vtx;		// 3 per vertex
	std::vector<float> uvs;		// 2 per uv
	SbinSection sections[3];
	int outNum[3];
	std::vector<int> outCounts[3];
	std::vector<SbinOutFrag> outFrags[3];
};

class SbinFile
{
public:
	SbinFile() : isNew(false) { }
	void Clear() { bodies.clear(); isNew = false; }
	bool Load(const std::string& utf8Path, std::string* error);
	bool Parse(const std::vector<u8>& data, std::string* error);
	void Serialize(std::vector<u8>* out) const;
	bool Save(const std::string& utf8Path, std::string* error) const;

	bool isNew;
	std::vector<SbinBody> bodies;
};

void FixedCopy(char* dst, size_t size, const std::string& s);	// zero-fills, truncates
std::string FixedStr(const char* s, size_t size);				// up to the terminator

#endif // GBIN_H
