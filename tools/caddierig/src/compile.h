// SPDX-License-Identifier: MIT
// compile.h - the PMP + ASE + ANI -> PET compiler, plus the reverse direction
// (project blocks recovered from a finished PET).
#ifndef COMPILE_H
#define COMPILE_H

#include <string>
#include <vector>
#include "asefile.h"
#include "petfile.h"
#include "pmpfile.h"

struct BuildLog
{
	std::vector<std::string> lines;		// UTF-8; "! " prefix for warnings
	int warnings;

	BuildLog() : warnings(0) { }
	void Info(const char* fmt, ...);
	void Warn(const char* fmt, ...);
};

struct BuildOptions
{
	PetKind kind;
	int verMajor, verMinor;		// 1.0 writes no VERS chunk
	float posTolerance;			// key reduction, in world units

	BuildOptions() : kind(PET_FULL), verMajor(1), verMinor(0), posTolerance(0.0002f) { }
};

// Max -> game space: swap Y and Z, scale by 0.32 (g_CM_TO_WU in the client).
const float CRIG_WORLD_SCALE = 0.32f;
WVector MaxToGame(const WVector& v);
WVector MaxDirToGame(const WVector& v);
WMatrix MaxTmToGame(const WVector tm[4]);

// Full compile. baseDir (UTF-8) is where the ASE and .Ani files are looked for.
bool CompilePuppet(const PmpFile& pmp, const char* baseDir, const BuildOptions& opt, PetFile* pet, BuildLog* log);

// Geometry only, from a parsed ASE. Fills BONE and MESH.
void BuildGeometry(const AseFile& ase, PetFile* pet, BuildLog* log);

// Merges one .Ani into pet->anims. Keys are shifted so source frame srcStart lands on
// timeline frame start. Call FinishAnimation once after the last motion.
bool MergeMotion(const char* aniPath, int start, int srcStart, int srcEnd, PetFile* pet, BuildLog* log);
void FinishAnimation(PetFile* pet, float posTolerance);

// Project metadata -> chunks. Each may be called on its own after an edit.
void ApplyTextures(const PmpFile& pmp, const AseFile* ase, PetFile* pet);
void ApplyBoxes(const PmpFile& pmp, PetFile* pet);
void ApplyMotions(const PmpFile& pmp, PetFile* pet);
void ApplyScripts(const PmpFile& pmp, PetFile* pet);

// PET -> project blocks, so finished files can be opened and re-authored.
void PetToProject(const PetFile& pet, const char* fileName, PmpFile* pmp);

// Layout of chunks for the chosen kind and version.
void LayoutChunks(PetFile* pet);

// Auto-fit: bounds of the vertices rigidly attached to a bone, in its own space.
// Returns false when the bone owns no vertices.
bool FitBoneBox(const PetFile& pet, int bone, float box[6]);

// Author-facing checks against the rules the client's loader relies on.
// foundTextures: UTF-8 names the caller located itself, e.g. in its search folder.
void ValidatePuppet(const PmpFile& pmp, const PetFile& pet, const char* baseDir, BuildLog* log,
	const std::vector<std::string>* foundTextures = 0);

// Tolerant comparison used by "crigc compare".
void ComparePuppets(const PetFile& a, const PetFile& b, float tol, BuildLog* log);

// Finds a file by name in the directory and common ASE sibling spots.
std::string ResolveSource(const std::string& baseDir, const std::string& name);

#endif // COMPILE_H
