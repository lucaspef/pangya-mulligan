// SPDX-License-Identifier: MIT
// asefile.h - 3ds Max ASCII Scene Export reader.
//
// .ASE holds the rig and meshes; .Ani files are the same format and hold one
// motion each (TM_ANIMATION tracks, no meshes).
#ifndef ASEFILE_H
#define ASEFILE_H

#include <string>
#include <vector>
#include "wmath.h"

struct AseMaterial
{
	std::string name;
	std::string bitmap;
	float diffuse[3];
	float transparency;
	int index;				// SUBMATERIAL number; the exporter skips empty slots
	int numSubs;			// NUMSUBMTLS, counts the skipped slots too
	std::vector<AseMaterial> subs;
};

struct AseFace
{
	int v[3];
	int smooth;
	int mtl;
};

struct AseKeyPos { int tick; WVector v; };
struct AseKeyRot { int tick; WVector axis; float angle; };

struct AseNode
{
	std::string name;
	std::string parent;
	bool helper;
	WVector tm[4];		// TM_ROW0..3, Max coordinates
	int materialRef;

	std::vector<WVector> verts;
	std::vector<AseFace> faces;
	std::vector<WVector> tverts;
	std::vector<int> tfaces;		// 3 per face
	std::vector<WVector> faceNormals;
	std::vector<WVector> vertNormals;	// 3 per face, in A B C order

	std::vector<AseKeyPos> posKeys;
	std::vector<AseKeyRot> rotKeys;
};

class AseFile
{
public:
	AseFile();
	bool Load(const char* path, std::string* error);
	int FindNode(const std::string& name) const;

	int firstFrame, lastFrame, frameSpeed, ticksPerFrame;
	std::vector<AseMaterial> materials;
	std::vector<AseNode> nodes;
};

#endif // ASEFILE_H
