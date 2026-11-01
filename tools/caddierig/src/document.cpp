// SPDX-License-Identifier: MIT
// document.cpp - project state and skeletal posing.
#include "document.h"
#include <windows.h>
#include <algorithm>
#include "petpose.h"
#include <stdlib.h>
#include <string.h>

// WMatrix -> quaternion, the inverse of SetRotation.
static WQuat RotationOf(const WMatrix& m)
{
	WQuat q;
	float tr = m.xx + m.yy + m.zz;
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

Document::Document()
{
	m_anim = 0;
	m_skeleton = 0;
	m_rootIndexed = false;
	Reset();
}

Document::~Document()
{
	ForgetFxModels();
	Reset();
}

void Document::Reset()
{
	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (parts[i]->owned)
			delete parts[i]->pet;
		delete parts[i];
	}
	parts.clear();
	delete m_anim;
	m_anim = 0;
	m_skeleton = 0;
	pmp.Clear();
	pet.Clear();
	projectPath.clear();
	sourcePath.clear();
	baseDir.clear();
	hasGeometrySource = false;
	dirty = false;
	texturesDirty = true;
	boneWorld.clear();
	boneHidden.clear();
	modelHidden = false;
	modelAlpha = 1.0f;
	m_trackOfBone.clear();
}

void Document::ForgetFxModels()
{
	for (std::map<std::string, MeshPart*>::iterator i = m_fxModels.begin(); i != m_fxModels.end(); ++i)
	{
		if (i->second)
		{
			delete i->second->pet;
			delete i->second;
		}
	}
	m_fxModels.clear();
}

void Document::SetDataRoot(const std::string& root)
{
	dataRoot = root;
	m_rootIndexed = false;
	m_rootIndex.clear();
	ForgetFxModels();
	texturesDirty = true;
}

// With no folder configured, a file inside a game dump finds its own "data" folder.
void Document::IndexDataRoot() const
{
	std::string root = dataRoot;
	if (root.empty())
	{
		std::string dir = baseDir;
		while (!dir.empty())
		{
			if (!_stricmp(PathBaseName(dir).c_str(), "data"))
			{
				root = dir;
				break;
			}
			std::string up = PathDir(dir);
			if (up == dir)
				break;
			dir = up;
		}
	}
	if (m_rootIndexed && root == m_indexedRoot)
		return;
	m_rootIndexed = true;
	m_indexedRoot = root;
	m_rootIndex.clear();
	static const wchar_t* const EXTS[] = { L".jpg", L".dds", L".tga", L".png", L".bmp", L".wav", L".spr", L".seq", L".pet", L".mpet", L".def" };
	IndexAssets(root, EXTS, sizeof(EXTS) / sizeof(EXTS[0]), &m_rootIndex);
}

std::string Document::ResolveAsset(const std::string& cp949Name) const
{
	std::string base = cp949Name;
	size_t slash = base.find_last_of("/\\");
	if (slash != std::string::npos)
		base.erase(0, slash + 1);
	std::string p = PathJoin(baseDir, Cp949ToUtf8(base));
	if (PathExists(p))
		return p;
	IndexDataRoot();
	std::map<std::string, std::string>::const_iterator it = m_rootIndex.find(LowerCp949(base));
	return it != m_rootIndex.end() ? it->second : std::string();
}

std::string Document::ResolveTexture(const std::string& name) const
{
	return ResolveAsset(Utf8ToCp949(name));
}

MeshPart* Document::LoadFxModel(const std::string& cp949Name)
{
	std::map<std::string, MeshPart*>::iterator it = m_fxModels.find(cp949Name);
	if (it != m_fxModels.end())
		return it->second;
	MeshPart* part = 0;
	std::string path = ResolveAsset(cp949Name);
	PetFile* pet = new PetFile;
	std::string error;
	if (!path.empty() && pet->Load(path.c_str(), &error) && !pet->vertices.empty())
	{
		part = new MeshPart;
		part->name = Cp949ToUtf8(cp949Name);
		part->pet = pet;
		part->owned = true;
		part->visible = true;
		part->isFx = true;
		PreparePart(part);
	}
	else
		delete pet;
	m_fxModels[cp949Name] = part;
	return part;
}

// -- opening ------------------------------------------------------------------

bool Document::OpenProject(const std::string& path, BuildLog* log)
{
	Reset();
	std::string error;
	if (!pmp.Load(path.c_str(), &error))
	{
		log->Warn("%s: %s", PathBaseName(path).c_str(), error.c_str());
		return false;
	}
	projectPath = path;
	sourcePath = path;
	baseDir = PathDir(path);
	log->Info("project %s (PMPF %s, %s)", PathBaseName(path).c_str(), pmp.version.c_str(), pmp.cp949 ? "CP949" : "UTF-8");
	return Rebuild(log);
}

bool Document::Rebuild(BuildLog* log)
{
	if (!pmp.aseName.empty() && !ResolveSource(baseDir, pmp.aseName).empty())
	{
		BuildOptions opt;
		opt.kind = PET_FULL;
		hasGeometrySource = CompilePuppet(pmp, baseDir.c_str(), opt, &pet, log);
	}
	else
	{
		// Without the Max export only the project side can be edited; PMP blocks still compile.
		hasGeometrySource = false;
		log->Warn("ASE %s is not next to the project: geometry unavailable, metadata only", pmp.aseName.c_str());
		ApplyTextures(pmp, 0, &pet);
		ApplyBoxes(pmp, &pet);
		ApplyMotions(pmp, &pet);
		ApplyScripts(pmp, &pet);
		LayoutChunks(&pet);
	}
	BuildParts();
	return true;
}

bool Document::OpenPuppet(const std::string& path, BuildLog* log)
{
	Reset();
	std::string error;
	if (!pet.Load(path.c_str(), &error))
	{
		log->Warn("%s: %s", PathBaseName(path).c_str(), error.c_str());
		return false;
	}
	sourcePath = path;
	baseDir = PathDir(path);
	PetToProject(pet, path.c_str(), &pmp);
	log->Info("puppet %s v%d.%d: %d bones, %d vertices, %d triangles, %d motions",
		PathBaseName(path).c_str(), pet.verMajor, pet.verMinor, (int)pet.bones.size(), (int)pet.vertices.size(),
		(int)pet.points.size() / 3, (int)pet.motions.size());

	std::string stem = PathStem(path);
	if (pet.kind == PET_BONE)
	{
		// A character: <x>_def.bpet + <x>_def.apet + the default clothing parts.
		std::string apet = PathJoin(baseDir, stem + ".apet");
		if (PathExists(apet))
		{
			m_anim = new PetFile;
			if (m_anim->Load(apet.c_str(), &error))
				log->Info("  animation %s: %d tracks, %d motions, %d scripts", PathBaseName(apet).c_str(),
					(int)m_anim->anims.size(), (int)m_anim->motions.size(), (int)m_anim->frames.size());
			else
			{
				delete m_anim;
				m_anim = 0;
			}
			if (m_anim)
			{
				// Its motions and scripts are part of what the author edits.
				PmpFile side;
				PetToProject(*m_anim, apet.c_str(), &side);
				pmp.blocks.insert(pmp.blocks.end(), side.blocks.begin(), side.blocks.end());
			}
		}
	}
	else if (pet.kind == PET_ANIM)
	{
		// Bones without rest matrices: pose them straight from their keys.
		log->Info("  animation-only puppet; open the matching .bpet to see it on a body");
	}
	BuildParts();

	if (pet.kind == PET_BONE)
	{
		size_t us = stem.find('_');
		if (us != std::string::npos)
		{
			std::string prefix = stem.substr(0, us + 1);
			std::string pattern = PathJoin(baseDir, prefix + "*_def.mpet");
			WIN32_FIND_DATAW fd;
			HANDLE h = FindFirstFileW(Wide(pattern).c_str(), &fd);
			if (h != INVALID_HANDLE_VALUE)
			{
				do
				{
					AttachPart(PathJoin(baseDir, Narrow(fd.cFileName)), log);
				} while (FindNextFileW(h, &fd));
				FindClose(h);
			}
		}
	}
	return true;
}

static std::string Utf8Base(const std::string& cp949Path)
{
	return PathBaseName(Cp949ToUtf8(cp949Path));
}

static void CollectBitmaps(const std::vector<AseMaterial>& list, std::vector<std::string>* out)
{
	for (size_t i = 0; i < list.size(); ++i)
	{
		if (!list[i].bitmap.empty())
		{
			std::string b = Utf8Base(list[i].bitmap);
			if (std::find(out->begin(), out->end(), b) == out->end())
				out->push_back(b);
		}
		CollectBitmaps(list[i].subs, out);
	}
}

bool Document::NewFromAse(const std::string& asePath, BuildLog* log)
{
	Reset();
	AseFile ase;
	std::string error;
	if (!ase.Load(asePath.c_str(), &error))
	{
		log->Warn("%s: %s", PathBaseName(asePath).c_str(), error.c_str());
		return false;
	}
	baseDir = PathDir(asePath);
	sourcePath = asePath;
	pmp.aseName = PathBaseName(asePath);
	pmp.version = "2.21";

	// The defaults every archived project starts from: one texture block per
	// bitmap, a box around each mesh node, and a "center" box.
	std::vector<std::string> bitmaps;
	CollectBitmaps(ase.materials, &bitmaps);
	for (size_t i = 0; i < bitmaps.size(); ++i)
	{
		std::string name = bitmaps[i];
		size_t dot = name.rfind('.');
		if (dot != std::string::npos)
			name.erase(dot);
		pmp.blocks.push_back(PmpFile::NewTexture(name, bitmaps[i], 0));
	}
	PetFile geo;
	BuildGeometry(ase, &geo, log);
	int firstMesh = -1;
	for (size_t i = 0; i < geo.bones.size(); ++i)
	{
		float box[6];
		if (!FitBoneBox(geo, (int)i, box))
			continue;
		if (firstMesh < 0)
			firstMesh = (int)i;
		std::string name = Cp949ToUtf8(geo.bones[i].name);
		pmp.blocks.push_back(PmpFile::NewBox(name, name, box));
	}
	float box[6];
	if (firstMesh >= 0 && FitBoneBox(geo, firstMesh, box))
		pmp.blocks.push_back(PmpFile::NewBox("center", Cp949ToUtf8(geo.bones[firstMesh].name), box));
	dirty = true;
	log->Info("new project from %s", PathBaseName(asePath).c_str());
	return Rebuild(log);
}

// -- parts ---------------------------------------------------------------------

void Document::BuildAnimMap()
{
	const PetFile& src = AnimSource();
	size_t n = m_skeleton ? m_skeleton->size() : 0;
	m_trackOfBone.assign(n, (const PetAnim*)0);
	for (size_t i = 0; i < src.anims.size(); ++i)
	{
		const PetAnim& a = src.anims[i];
		if (a.bone < 0 || a.bone >= (int)src.bones.size())
			continue;
		int mapped = -1;
		for (size_t k = 0; k < n; ++k)
		{
			if (!_stricmp((*m_skeleton)[k].name.c_str(), src.bones[a.bone].name.c_str()))
			{
				mapped = (int)k;
				break;
			}
		}
		if (mapped >= 0)
			m_trackOfBone[mapped] = &a;
	}
}

// WBone::FindFakeBodyBone: a leaf outside the biped that no vertex is attached to.
// Exported normals of biped-bound vertices are stored relative to it.
static int FindFakeBodyBone(const PetFile& p, int bone, const std::vector<bool>& hasVerts,
	const std::vector<std::vector<int> >& children)
{
	const std::string& name = p.bones[bone].name;
	if (!strstr(name.c_str(), "Bip") && !strstr(name.c_str(), "Jaw") && !hasVerts[bone] && children[bone].empty())
		return bone;
	for (size_t i = 0; i < children[bone].size(); ++i)
	{
		int found = FindFakeBodyBone(p, children[bone][i], hasVerts, children);
		if (found >= 0)
			return found;
	}
	return -1;
}

void Document::PreparePart(MeshPart* part)
{
	const PetFile& p = *part->pet;
	size_t nb = p.bones.size();
	part->restWorld.resize(nb);
	part->boneMap.resize(nb);
	for (size_t i = 0; i < nb; ++i)
	{
		const PetBone& b = p.bones[i];
		part->restWorld[i] = b.parent >= 0 && b.parent < (int)i ? b.mat * part->restWorld[b.parent] : b.mat;
		part->boneMap[i] = -1;
		for (size_t k = 0; m_skeleton && k < m_skeleton->size(); ++k)
		{
			if (!_stricmp((*m_skeleton)[k].name.c_str(), b.name.c_str()))
			{
				part->boneMap[i] = (int)k;
				break;
			}
		}
	}
	part->skin.resize(nb);

	part->restPos.resize(p.vertices.size());
	for (size_t i = 0; i < p.vertices.size(); ++i)
	{
		const PetVertex& v = p.vertices[i];
		int b = v.blends.empty() ? -1 : v.blends[0].bone;
		part->restPos[i] = b >= 0 && b < (int)nb ? v.pos * part->restWorld[b] : v.pos;
	}

	// .mpet addresses vertices per skin; the normals of the other kinds are bone relative.
	size_t corners = p.points.size();
	part->cornerVertex.resize(corners);
	part->restNormal.resize(corners);
	int fake = -1;
	if (p.kind != PET_MESH && nb)
	{
		std::vector<bool> hasVerts(nb, false);
		for (size_t i = 0; i < p.vertices.size(); ++i)
			if (!p.vertices[i].blends.empty() && p.vertices[i].blends[0].bone < (int)nb)
				hasVerts[p.vertices[i].blends[0].bone] = true;
		std::vector<std::vector<int> > children(nb);
		for (size_t i = 0; i < nb; ++i)
			if (p.bones[i].parent >= 0 && p.bones[i].parent < (int)nb)
				children[p.bones[i].parent].push_back((int)i);
		for (size_t i = 0; i < nb && fake < 0; ++i)
			if (p.bones[i].parent < 0)
				fake = FindFakeBodyBone(p, (int)i, hasVerts, children);
	}
	std::vector<int> baseOfTri(corners / 3, 0);
	if (p.kind == PET_MESH)
	{
		for (size_t s = 0; s < p.skins.size(); ++s)
		{
			const PetSkin& skin = p.skins[s];
			for (int t = skin.startTri < 0 ? 0 : skin.startTri; t < skin.startTri + skin.numTri && t < (int)baseOfTri.size(); ++t)
				baseOfTri[t] = skin.startVtx;
		}
	}
	for (size_t i = 0; i < corners; ++i)
	{
		const PetTriPoint& t = p.points[i];
		int vi = (int)t.pos + baseOfTri[i / 3];
		part->cornerVertex[i] = vi < (int)p.vertices.size() ? vi : 0;
		WVector n = t.normal;
		if (n.x == 0.0f && n.y == 0.0f && n.z == 0.0f)
		{
			size_t a = i / 3 * 3;
			if (a + 2 < corners)
			{
				WVector va = part->restPos[p.points[a].pos + baseOfTri[i / 3] < p.vertices.size() ? p.points[a].pos + baseOfTri[i / 3] : 0];
				WVector vb = part->restPos[p.points[a + 1].pos + baseOfTri[i / 3] < p.vertices.size() ? p.points[a + 1].pos + baseOfTri[i / 3] : 0];
				WVector vc = part->restPos[p.points[a + 2].pos + baseOfTri[i / 3] < p.vertices.size() ? p.points[a + 2].pos + baseOfTri[i / 3] : 0];
				n = (va - vb).Cross(vc - vb);
			}
		}
		else if (p.kind != PET_MESH && !p.vertices.empty())
		{
			int b = p.vertices[part->cornerVertex[i]].blends.empty() ? -1 : p.vertices[part->cornerVertex[i]].blends[0].bone;
			if (b >= 0 && b < (int)nb)
			{
				const char* bn = p.bones[b].name.c_str();
				bool biped = strstr(bn, "Bip") || strstr(bn, "Bone") || strstr(bn, "Jaw");
				n = part->restWorld[fake >= 0 && biped ? fake : b].Rotate(n);
			}
		}
		n.Normalize();
		part->restNormal[i] = n;
	}
	// Which bone *hidebone has to hide for each triangle: the skin's in a .mpet, and
	// otherwise the first vertex's, as WBone::SetMesh groups them.
	part->triBone.assign(corners / 3, -1);
	part->triOwnBone.assign(corners / 3, -1);
	part->ownHidden.assign(nb, false);
	for (size_t t = 0; t < corners / 3; ++t)
	{
		int vi = part->cornerVertex[t * 3];
		int b = p.vertices.empty() || p.vertices[vi].blends.empty() ? -1 : p.vertices[vi].blends[0].bone;
		if (b >= 0 && b < (int)part->boneMap.size())
		{
			part->triBone[t] = part->boneMap[b];
			part->triOwnBone[t] = b;
		}
	}
	for (size_t s = 0; s < p.skins.size(); ++s)
	{
		const PetSkin& skin = p.skins[s];
		if (skin.bone >= part->boneMap.size())
			continue;
		for (int t = skin.startTri < 0 ? 0 : skin.startTri; t < skin.startTri + skin.numTri && t < (int)part->triBone.size(); ++t)
		{
			part->triBone[t] = part->boneMap[skin.bone];
			part->triOwnBone[t] = skin.bone;
		}
	}
	part->skinnedPos = part->restPos;
	part->skinnedNormal = part->restNormal;
}

void Document::AddPart(PetFile* source, const std::string& name, bool owned)
{
	MeshPart* part = new MeshPart;
	part->name = name;
	part->pet = source;
	part->owned = owned;
	part->visible = true;
	part->isFx = false;
	PreparePart(part);
	parts.push_back(part);
}

void Document::BuildParts()
{
	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (parts[i]->owned)
			delete parts[i]->pet;
		delete parts[i];
	}
	parts.clear();

	m_skeleton = &pet.bones;
	size_t n = m_skeleton->size();
	m_restRot.resize(n);
	m_restPos.resize(n);
	for (size_t i = 0; i < n; ++i)
	{
		m_restRot[i] = RotationOf((*m_skeleton)[i].mat);
		m_restPos[i] = (*m_skeleton)[i].mat.Pivot();
	}
	boneWorld.resize(n);
	boneHidden.assign(n, false);
	BuildAnimMap();
	if (!pet.vertices.empty())
		AddPart(&pet, PathBaseName(sourcePath), false);
	texturesDirty = true;
}

bool Document::AttachPart(const std::string& mpetPath, BuildLog* log)
{
	PetFile* mesh = new PetFile;
	std::string error;
	if (!mesh->Load(mpetPath.c_str(), &error))
	{
		log->Warn("%s: %s", PathBaseName(mpetPath).c_str(), error.c_str());
		delete mesh;
		return false;
	}
	AddPart(mesh, PathBaseName(mpetPath), true);
	log->Info("  part %s: %d vertices, %d triangles", PathBaseName(mpetPath).c_str(), (int)mesh->vertices.size(), (int)mesh->points.size() / 3);
	return true;
}

void Document::RemovePart(size_t index)
{
	MeshPart* part = parts[index];
	parts.erase(parts.begin() + index);
	if (part->owned)
		delete part->pet;
	delete part;
}

// -- posing --------------------------------------------------------------------

void Document::Pose(float time)
{
	size_t n = m_skeleton ? m_skeleton->size() : 0;
	for (size_t i = 0; i < n; ++i)
	{
		const PetBone& b = (*m_skeleton)[i];
		WMatrix local;
		const PetAnim* track = m_trackOfBone[i];
		if (track)
		{
			local.SetRotation(EvalKeyRot(track->rot, time, m_restRot[i]));
			local.SetPivot(EvalKeyPos(track->pos, time, m_restPos[i]));
		}
		else if (pet.kind == PET_ANIM)
		{
			local.Reset();
		}
		else
		{
			local = b.mat;
		}
		boneWorld[i] = b.parent >= 0 && b.parent < (int)i ? local * boneWorld[b.parent] : local;
	}

	for (size_t pi = 0; pi < parts.size(); ++pi)
	{
		MeshPart& part = *parts[pi];
		if (!part.visible)
			continue;
		const PetFile& p = *part.pet;
		if (p.bones.empty())
			continue;		// nothing to skin against: stays at its rest positions
		for (size_t b = 0; b < p.bones.size(); ++b)
		{
			int mapped = part.boneMap[b];
			if (mapped >= 0 && mapped < (int)boneWorld.size())
				part.skin[b] = part.restWorld[b].Inverse() * boneWorld[mapped];
			else
				part.skin[b].Reset();
		}
		part.skinnedPos.resize(p.vertices.size());
		for (size_t i = 0; i < p.vertices.size(); ++i)
		{
			const PetVertex& v = p.vertices[i];
			const WVector& r = part.restPos[i];
			if (v.blends.size() == 1)
			{
				part.skinnedPos[i] = r * part.skin[v.blends[0].bone < (int)part.skin.size() ? v.blends[0].bone : 0];
				continue;
			}
			WVector sum;
			for (size_t k = 0; k < v.blends.size(); ++k)
			{
				int b = v.blends[k].bone;
				if (b < (int)part.skin.size())
					sum = sum + (r * part.skin[b]) * (v.blends[k].weight / 255.0f);
			}
			part.skinnedPos[i] = sum;
		}
		part.skinnedNormal.resize(p.points.size());
		for (size_t i = 0; i < p.points.size(); ++i)
		{
			const PetVertex& v = p.vertices[part.cornerVertex[i]];
			int b = v.blends.empty() ? 0 : v.blends[0].bone;
			part.skinnedNormal[i] = b < (int)part.skin.size() ? part.skin[b].Rotate(part.restNormal[i]) : part.restNormal[i];
		}
	}
}

void Document::WorldBounds(WVector* lo, WVector* hi) const
{
	bool any = false;
	WVector a, b;
	for (size_t pi = 0; pi < parts.size(); ++pi)
	{
		const MeshPart& part = *parts[pi];
		for (size_t i = 0; i < part.restPos.size(); ++i)
		{
			const WVector& v = part.restPos[i];
			if (!any)
			{
				a = b = v;
				any = true;
				continue;
			}
			if (v.x < a.x) a.x = v.x;
			if (v.y < a.y) a.y = v.y;
			if (v.z < a.z) a.z = v.z;
			if (v.x > b.x) b.x = v.x;
			if (v.y > b.y) b.y = v.y;
			if (v.z > b.z) b.z = v.z;
		}
	}
	if (!any)
	{
		for (size_t i = 0; i < boneWorld.size(); ++i)
		{
			WVector v = boneWorld[i].Pivot();
			if (!any)
			{
				a = b = v;
				any = true;
				continue;
			}
			if (v.x < a.x) a.x = v.x;
			if (v.y < a.y) a.y = v.y;
			if (v.z < a.z) a.z = v.z;
			if (v.x > b.x) b.x = v.x;
			if (v.y > b.y) b.y = v.y;
			if (v.z > b.z) b.z = v.z;
		}
	}
	if (!any)
	{
		a = WVector(-1.0f, -1.0f, -1.0f);
		b = WVector(1.0f, 1.0f, 1.0f);
	}
	*lo = a;
	*hi = b;
}

float Document::LastFrame() const
{
	const PetFile& s = AnimSource();
	float last = 0.0f;
	for (size_t i = 0; i < s.motions.size(); ++i)
		if ((float)s.motions[i].endFrame > last)
			last = (float)s.motions[i].endFrame;
	for (size_t i = 0; i < s.anims.size(); ++i)
	{
		if (!s.anims[i].pos.empty() && s.anims[i].pos.back().time * 30.0f > last)
			last = s.anims[i].pos.back().time * 30.0f;
		if (!s.anims[i].rot.empty() && s.anims[i].rot.back().time * 30.0f > last)
			last = s.anims[i].rot.back().time * 30.0f;
	}
	return last;
}

// -- edits ---------------------------------------------------------------------

void Document::ApplyBlockKind(PmpBlockKind kind)
{
	dirty = true;
	switch (kind)
	{
	case PMP_TEXTURE:
		{
			// Keep triangle slots; only names, flags and groups change.
			std::vector<PetTexture> old = pet.textures;
			ApplyTextures(pmp, 0, &pet);
			for (size_t i = 0; i < pet.textures.size() && i < old.size(); ++i)
				pet.textures[i].diffuse = old[i].diffuse;
			pet.AddChunk("TEXT");
			pet.AddChunk("FANM");
			texturesDirty = true;
		}
		break;
	case PMP_BOX:
		ApplyBoxes(pmp, &pet);
		pet.AddChunk("COLL");
		break;
	case PMP_MOTION:
		if (m_anim)
			ApplyMotions(pmp, m_anim);
		else
		{
			ApplyMotions(pmp, &pet);
			pet.AddChunk("MOTI");
		}
		break;
	case PMP_SCRIPT:
		if (m_anim)
			ApplyScripts(pmp, m_anim);
		else
		{
			ApplyScripts(pmp, &pet);
			pet.AddChunk("FRAM");
		}
		break;
	default:
		break;
	}
}

PmpBlock* Document::BlockAt(PmpBlockKind kind, int index)
{
	int n = 0;
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
		if (pmp.blocks[i].kind == kind && n++ == index)
			return &pmp.blocks[i];
	return 0;
}

const PmpBlock* Document::BlockAt(PmpBlockKind kind, int index) const
{
	return const_cast<Document*>(this)->BlockAt(kind, index);
}

int Document::BlockCount(PmpBlockKind kind) const
{
	int n = 0;
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
		if (pmp.blocks[i].kind == kind)
			++n;
	return n;
}

void Document::RemoveBlock(PmpBlockKind kind, int index)
{
	int n = 0;
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		if (pmp.blocks[i].kind == kind && n++ == index)
		{
			pmp.blocks.erase(pmp.blocks.begin() + i);
			break;
		}
	}
	ApplyBlockKind(kind);
}

void Document::AddBoxForBone(int bone)
{
	if (bone < 0 || bone >= (int)pet.bones.size())
		return;
	float box[6];
	if (!FitBoneBox(pet, bone, box))
	{
		box[0] = box[1] = box[2] = -0.05f;
		box[3] = box[4] = box[5] = 0.05f;
	}
	std::string name = Cp949ToUtf8(pet.bones[bone].name);
	pmp.blocks.push_back(PmpFile::NewBox(name, name, box));
	ApplyBlockKind(PMP_BOX);
}

void Document::AddCenterBox()
{
	if (pet.bones.empty())
		return;
	int bone = 0;
	float box[6];
	bool any = false;
	for (size_t i = 0; i < pet.bones.size() && !any; ++i)
	{
		any = FitBoneBox(pet, (int)i, box);
		if (any)
			bone = (int)i;
	}
	if (!any)
	{
		box[0] = box[1] = box[2] = -0.5f;
		box[3] = box[4] = box[5] = 0.5f;
	}
	pmp.blocks.push_back(PmpFile::NewBox("center", Cp949ToUtf8(pet.bones[bone].name), box));
	ApplyBlockKind(PMP_BOX);
}

void Document::AddScript(int frame, const std::string& message)
{
	// Blocks stay sorted by frame: CheckFrameData walks them in ascending order.
	PmpBlock b = PmpFile::NewScript(frame, message, std::string(), frame);
	size_t at = pmp.blocks.size();
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
	{
		if (pmp.blocks[i].kind == PMP_SCRIPT && atoi(pmp.blocks[i].arg.c_str()) > frame)
		{
			at = i;
			break;
		}
	}
	pmp.blocks.insert(pmp.blocks.begin() + at, b);
	ApplyBlockKind(PMP_SCRIPT);
}

void Document::AddTexture(const std::string& path)
{
	std::string file = PathBaseName(path);
	std::string name = file;
	size_t dot = name.rfind('.');
	if (dot != std::string::npos)
		name.erase(dot);
	size_t at = 0;
	for (size_t i = 0; i < pmp.blocks.size(); ++i)
		if (pmp.blocks[i].kind == PMP_TEXTURE)
			at = i + 1;
	pmp.blocks.insert(pmp.blocks.begin() + at, PmpFile::NewTexture(name, file, 0));
	ApplyBlockKind(PMP_TEXTURE);
}

bool Document::ImportMotion(const std::string& aniPath, const std::string& name, BuildLog* log)
{
	if (pet.bones.empty())
	{
		log->Warn("import a skeleton first: motions bind to bones by name");
		return false;
	}
	AseFile ani;
	std::string error;
	if (!ani.Load(aniPath.c_str(), &error))
	{
		log->Warn("%s: %s", PathBaseName(aniPath).c_str(), error.c_str());
		return false;
	}
	PetFile& target = m_anim ? *m_anim : pet;
	// Clips are laid end to end with one idle frame between them, as in the shipped .apet files.
	int start = 0;
	for (size_t i = 0; i < target.motions.size(); ++i)
		if ((int)target.motions[i].endFrame + 1 > start)
			start = (int)target.motions[i].endFrame + 1;
	int srcEnd = ani.lastFrame - ani.firstFrame;
	if (!MergeMotion(aniPath.c_str(), start, 0, srcEnd, &target, log))
		return false;
	FinishAnimation(&target, 0.0002f);

	std::string rootBone = pet.bones.empty() ? std::string() : Cp949ToUtf8(pet.bones[0].name);
	pmp.blocks.push_back(PmpFile::NewMotion(name, start, start + srcEnd, name, rootBone, PathBaseName(aniPath), 0, srcEnd));
	ApplyBlockKind(PMP_MOTION);
	target.AddChunk("ANIM");
	target.AddChunk("MOTI");
	target.AddChunk("FRAM");
	BuildAnimMap();
	log->Info("motion %s placed at frames %d-%d", name.c_str(), start, start + srcEnd);
	return true;
}

bool Document::SaveProject(const std::string& path, std::string* error)
{
	if (!pmp.Save(path.c_str(), error))
		return false;
	projectPath = path;
	baseDir = PathDir(path);
	dirty = false;
	return true;
}

bool Document::ExportPuppet(const std::string& path, int verMajor, int verMinor, BuildLog* log)
{
	PetKind kind = PetKindFromPath(path.c_str());
	std::string error;
	PetFile out;
	if (kind == PET_ANIM && m_anim)
	{
		out = *m_anim;
	}
	else if (kind != pet.kind && hasGeometrySource)
	{
		BuildOptions opt;
		opt.kind = kind;
		opt.verMajor = verMajor ? verMajor : 1;
		opt.verMinor = verMajor ? verMinor : 3;
		if (!CompilePuppet(pmp, baseDir.c_str(), opt, &out, log))
			return false;
	}
	else if (kind != pet.kind)
	{
		if (kind == PET_MESH || pet.kind == PET_MESH || pet.kind == PET_ANIM)
		{
			log->Warn("converting %s to %s needs the ASE: the skin table cannot be derived from a finished puppet",
				PetKindExt(pet.kind), PetKindExt(kind));
			return false;
		}
		out = pet;
		out.kind = kind;
		LayoutChunks(&out);
	}
	else
	{
		out = pet;
	}
	if (verMajor && (out.verMajor != verMajor || out.verMinor != verMinor))
	{
		out.verMajor = verMajor;
		out.verMinor = verMinor;
		if (out.VerIs(1, 0))
			out.RemoveChunk("VERS");
		else if (!out.HasChunk("VERS"))
		{
			out.AddChunk("VERS");
			std::rotate(out.chunks.begin(), out.chunks.end() - 1, out.chunks.end());
		}
		for (size_t i = 0; i < out.bones.size(); ++i)
			out.bones[i].tail = -1.0f;
	}
	if (!out.Save(path.c_str(), &error))
	{
		log->Warn("%s: %s", PathBaseName(path).c_str(), error.c_str());
		return false;
	}
	log->Info("exported %s (v%d.%d)", path.c_str(), out.verMajor, out.verMinor);
	return true;
}
