// SPDX-License-Identifier: MIT
// course.h - the open hole: its project, everything compiled from or next to it, the
// rules that turn one into a .gbin, and undo.
#ifndef COURSE_H
#define COURSE_H

#include <map>
#include <string>
#include <vector>
#include "fxsim.h"
#include "gbin.h"
#include "props.h"
#include "wep.h"

// One Puppet model as a course uses it: bones applied, not animated.
struct PuppetAsset
{
	std::string name;			// lower case base name, CP949
	std::string path;			// UTF-8, empty when not found
	bool loaded;
	PetFile pet;
	std::vector<WVector> pos;	// per triangle corner, model space
	std::vector<WVector> normal;
	WVector lo, hi;
	std::vector<std::string> textureNames;	// used by a triangle, in texture table order
	void* render;				// owned by scene.cpp

	PuppetAsset() : loaded(false), render(0) { }
};

// A w_mesh of the base model as WBone::SetMesh builds it. The .gbin colours and the .sbin
// fragments both index these rather than the file's triangle order.
struct BaseMesh
{
	int textureSlot;
	std::vector<int> faces;		// triangles of the .pet, in order
	std::vector<int> index;		// corner -> mesh vertex
	std::vector<int> vertexCorner;	// mesh vertex -> first corner of the .pet
};

struct CourseLog
{
	std::vector<std::string> lines;	// UTF-8; a leading "! " marks a warning
	int warnings;
	CourseLog() : warnings(0) { }
	void Info(const char* fmt, ...);
	void Warn(const char* fmt, ...);
};

struct ElementRef
{
	enum Kind { NONE, ELEMENT, NODE, NODE_POINT } kind;
	int index;		// element or node
	int point;		// node point
	ElementRef() : kind(NONE), index(-1), point(-1) { }
	ElementRef(Kind k, int i, int p = -1) : kind(k), index(i), point(p) { }
	bool operator==(const ElementRef& o) const { return kind == o.kind && index == o.index && point == o.point; }
};

struct Snapshot
{
	std::string label;
	std::vector<std::string> puppets;
	std::vector<WepNode> nodes;
	std::vector<WepElement> elements;
};

struct TerrainHit
{
	bool hit;
	float t;
	WVector pos, normal;
	int element;	// -1 for the base model
};

struct PropReport;

class Course
{
public:
	Course();
	~Course();
	void Clear();
	void New();
	// .wep, or a compiled .gbin to reconstruct a project from.
	bool Open(const std::string& utf8Path, CourseLog* log);
	bool SaveWep(const std::string& utf8Path, std::string* error);

	void SetDataRoot(const std::string& utf8Root);
	std::string Resolve(const std::string& cp949Name) const;		// by base name below the data root
	PuppetAsset* Puppet(const std::string& cp949PathOrName);
	std::vector<std::string> AllPuppetNames() const;	// every .pet below the data root (CP949, lower)

	// -- compile -------------------------------------------------------------
	void ImportGbin(const GbinFile& g);
	bool Compile(GbinFile* out, CourseLog* log);
	bool ExportHole(const std::string& gbinPath, CourseLog* log);
	void Validate(CourseLog* log);
	std::vector<std::string> UsedTextures();	// what the .gbin's texture table would hold
	void ComputeBaseMeshes();
	// Colours and shadow maps belong to the base model they were baked for.
	bool BaseColorsValid() const;
	void BakeBaseColors(CourseLog* log);
	void ClearShadowMaps(CourseLog* log);

	// -- scene ---------------------------------------------------------------
	PuppetAsset* BasePuppet();
	WMatrix ElementMatrix(const WepElement& e) const;
	void ElementBounds(const WepElement& e, WVector* lo, WVector* hi);
	void SceneBounds(WVector* lo, WVector* hi);
	TerrainHit CastRay(const WVector& from, const WVector& dir, bool baseOnly, float maxDist = 1e9f);
	bool DropToGround(WVector* pos);
	// The same, remembered per x, z: for points drawn every frame. Follows the base model.
	bool StandOnGround(WVector* pos);
	// tee/pin slots: 시작점, 시작점2, 끝점1_1..3, 끝점2_1..3.
	static std::string HoleSlotName(int slot);
	static int HoleSlotCount() { return 8; }
	int FindHoleSlot(int slot) const;
	WVector SunDirection() const;	// the way the light travels
	void GameColor(const char* key, float rgb[3]) const;

	// -- undo ----------------------------------------------------------------
	void Checkpoint(const std::string& label);	// call before changing anything
	bool Undo();
	bool Redo();

	WepFile wep;
	std::string path;			// UTF-8, may be empty
	std::string dataRoot;
	std::string baseName;		// sand_06
	bool dirty;

	bool hasSource;
	GbinFile source;			// the .gbin found next to the project
	std::string sourcePath;
	bool hasSbin;
	SbinFile sbin;
	bool sbinCleared;			// write an empty .sbin at the next export
	PropFile prop;
	bool hasFog;
	float fogColor[3], fogNear, fogFar;
	std::vector<BaseMesh> baseMeshes;
	std::vector<u32> cornerColor;	// per corner of the .pet; empty means none
	int revision;				// bumps with anything that changes what gets drawn

private:
	void FindCompanions(CourseLog* log);
	void MapBaseColors();
	void IndexRoot() const;
	void RestoreSnapshot(const Snapshot& s);
	void TakeSnapshot(Snapshot* s, const std::string& label) const;
	void FreeAssets();

	std::map<std::string, PuppetAsset*> m_assets;
	mutable std::map<std::string, std::string> m_index;
	mutable bool m_indexed;
	std::vector<Snapshot> m_undo, m_redo;
	std::string m_groundKey;
	std::map<std::pair<u32, u32>, std::pair<bool, float> > m_groundY;
};

// What fxsim needs from a course, for the editor and the verifier alike.
class CourseFxHost : public FxHost
{
public:
	explicit CourseFxHost(Course* c) : course(c) { }
	virtual std::string ResolveAsset(const std::string& name) { return course->Resolve(name); }
	virtual void PlaySound(const std::string&, float) { }	// silent
	// Whoever draws the spray casts this back to PuppetAsset.
	virtual FxPetModel* LoadPetModel(const std::string& name)
	{
		PuppetAsset* a = course->Puppet(name);
		return a->loaded ? (FxPetModel*)a : 0;
	}

private:
	Course* course;
};

void GrowBox(WVector* lo, WVector* hi, const WVector& q);

#endif // COURSE_H
