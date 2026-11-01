// SPDX-License-Identifier: MIT
// document.h - the open project: PMP blocks, the compiled puppet and everything
// the viewport needs to pose it.
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <map>
#include <string>
#include <vector>
#include "compile.h"

// One renderable mesh. A .pet has one, a character is a .bpet with many .mpet parts.
struct MeshPart
{
	std::string name;
	PetFile* pet;
	bool owned;
	bool visible;
	std::vector<WMatrix> restWorld;		// its own skeleton, used to place the vertices
	std::vector<int> boneMap;			// its bones -> document skeleton, by name
	std::vector<WVector> restPos;		// vertex rest positions in model space
	std::vector<WVector> restNormal;	// per triangle corner
	std::vector<int> cornerVertex;		// absolute vertex index per corner
	std::vector<unsigned int> glTextures;	// per TEXT entry
	std::vector<int> triBone;			// document skeleton bone owning each triangle, or -1
	std::vector<int> triOwnBone;		// the same in this part's own bones
	std::vector<bool> ownHidden;		// *hidebone on mesh bones the skeleton doesn't have
	std::vector<std::string> textureOverride;	// colour variant paths, per TEXT entry
	bool isFx;							// a *pet particle model, not part of the character
	std::vector<WVector> skinnedPos;
	std::vector<WVector> skinnedNormal;
	std::vector<WMatrix> skin;			// rest -> pose, per bone of this part
};

class Document
{
public:
	Document();
	~Document();
	void Reset();

	// All paths UTF-8.
	bool OpenProject(const std::string& path, BuildLog* log);
	bool OpenPuppet(const std::string& path, BuildLog* log);
	bool NewFromAse(const std::string& asePath, BuildLog* log);
	bool Rebuild(BuildLog* log);
	bool SaveProject(const std::string& path, std::string* error);
	// verMajor 0 keeps the version of the source.
	bool ExportPuppet(const std::string& path, int verMajor, int verMinor, BuildLog* log);
	bool ImportMotion(const std::string& aniPath, const std::string& name, BuildLog* log);
	bool AttachPart(const std::string& mpetPath, BuildLog* log);
	void RemovePart(size_t index);
	void AddBoxForBone(int bone);
	void AddCenterBox();
	void AddScript(int frame, const std::string& message);
	void AddTexture(const std::string& path);

	// Re-derives the chunk a block of this kind compiles into.
	void ApplyBlockKind(PmpBlockKind kind);
	PmpBlock* BlockAt(PmpBlockKind kind, int index);
	const PmpBlock* BlockAt(PmpBlockKind kind, int index) const;
	int BlockCount(PmpBlockKind kind) const;
	void RemoveBlock(PmpBlockKind kind, int index);

	// Pose at a timeline position (seconds); fills boneWorld and skins every visible part.
	void Pose(float time);
	void WorldBounds(WVector* lo, WVector* hi) const;
	const std::vector<PetBone>& Skeleton() const { return *m_skeleton; }
	bool HasSkeleton() const { return m_skeleton && !m_skeleton->empty(); }
	float LastFrame() const;
	const PetFile& ScriptSource() const { return m_anim ? *m_anim : pet; }
	const PetFile& AnimSource() const { return m_anim ? *m_anim : pet; }
	bool IsAvatar() const { return m_anim != 0; }

	std::string ResolveTexture(const std::string& name) const;	// UTF-8 name
	std::string ResolveAsset(const std::string& cp949Name) const;
	MeshPart* LoadFxModel(const std::string& cp949Name);		// for *pet sprays
	void ForgetFxModels();
	void SetDataRoot(const std::string& root);

	PmpFile pmp;
	PetFile pet;
	std::string projectPath;	// .pmp, may be empty
	std::string sourcePath;		// whatever was opened
	std::string baseDir;
	std::string dataRoot;
	bool hasGeometrySource;		// geometry can be recompiled from an ASE
	bool dirty;
	bool texturesDirty;

	std::vector<MeshPart*> parts;
	std::vector<WMatrix> boneWorld;
	std::vector<bool> boneHidden;
	bool modelHidden;	// *hide
	float modelAlpha;	// *opa

private:
	void BuildParts();
	void AddPart(PetFile* source, const std::string& name, bool owned);
	void PreparePart(MeshPart* part);
	void BuildAnimMap();
	void IndexDataRoot() const;

	PetFile* m_anim;			// the .apet next to a .bpet
	const std::vector<PetBone>* m_skeleton;
	std::vector<WQuat> m_restRot;
	std::vector<WVector> m_restPos;
	std::vector<const PetAnim*> m_trackOfBone;

	mutable std::map<std::string, std::string> m_rootIndex;
	mutable bool m_rootIndexed;
	mutable std::string m_indexedRoot;
	std::map<std::string, MeshPart*> m_fxModels;
};

#endif // DOCUMENT_H
