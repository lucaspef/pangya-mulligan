#include "wmath.h"

#include "wview.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wmesh.h"
#include "wpetfile.h"
#include "wboneset.h"
#include "wblockmodel.h"
#include "cfile.h"
#include <list>
#include <algorithm>
#include "wxtnlbuffer.h"
#include <string.h>
#include <float.h>

WTVertex* WBone::m_vtxList = 0;
WVector* WBone::m_vecList = 0;
int WBone::m_vtx_len = 0;
int WBone::m_vtx_count = 0;

int WriteID(int, FILE*);
int WisZero(const float& v, float e);
bool VECEQU(const WVector&, const WVector&);
bool QUATEQU(const WQuat&, const WQuat&);
struct w_3d_object;
struct w_vertex;
struct w_rigid;
struct w_keyframe;
struct w_faces;

struct w_rigid
{
	char objname[32];
	w_3d_object* obj;
	WBone* bone;
	float blendWeight;
};

struct w_vertex
{
	WVector v;
	float weight;
	char objname[32];
	w_rigid* rigidVtx;
	int rigidNum;
	w_3d_object* obj;
};

struct w_keyframe
{
	float t;
	WQuat* q;
	WVector* v;
	float* scale;
	w_keyframe* next;
	w_keyframe* prev;
};

struct w_3d_object
{
	w_3d_object* parent;
	w_3d_object* child;
	w_3d_object* next;
	w_vertex* gvertex;
	w_faces* gfaces;
	WMatrix matrix;
	char name[40];
	char parentName[40];
	int nvertex;
	int nfaces;
	int first;
	int type;
	WBone* bone;
	w_3d_object* list;
	w_keyframe* keyframe;
};

struct w_faces
{
	struct w_faces_info
	{
		w_3d_object* object;
		w_vertex* gvertex;
		WBone* bone;
		WVector norm;
		int vindex;
		float tu;
		float tv;
	};

	w_faces_info p[3];
	unsigned int diffuse;
	int texhandle;
	int subNum;
	int mapType;
	int mapFlags;
	int xiMapFlags2;
};

// w_common_bone_data: no wbone.h (983: 0xc8, usada tambem pelo WPuppet)

WBone::WBone()
{
	m_parent = 0;
	m_child = 0;
	m_next = 0;
	m_rendMode = 0;
	m_scale = 1.0f;
	m_lightLine = 0.0f;
	m_keyPos = 0;
	m_keyPosNum = 0;
	m_keyRot = 0;
	m_keyRotNum = 0;
	m_basicScale983 = WVector(1.0f, 1.0f, 1.0f); // 983
	m_petBoneFloat983 = -1.0f; // 983
	m_keyFlag = 0;
	m_keyScale = 0; // 983
	m_keyScaleNum = 0;
	m_key983_ec = 0;
	m_key983_f0 = 0;
	m_bound_sphere = WSphere(WVector::ZERO, 0.0f);
	m_mesh = 0;
	m_selfillumColor = 0xffffff;
	m_alpha = 0xff;
	m_basicQuat.Reset();
	m_basicMat.Reset();
	m_localMat.Reset();
	m_rotateMat.Reset();
	m_moveVec.Reset();
	m_matrix.Reset();
	m_light.type = 2;
	m_light.nearOne = WVector::UNIT_NEG_Y;
	m_bound_aabb = Waabb(WVector::ZERO, WVector(0.0001f, 0.0001f, 0.0001f));
	m_vtxColor = 0xffffffff;
	CountVtxBuff(1);
}

WBone::~WBone()
{
	if (m_mesh != 0)
	{
		ClearMesh(m_mesh);
		if (m_mesh != 0)
		{
			delete m_mesh;
			m_mesh = 0;
		}
	}
	bool shared = ((((ulong)m_flag) >> 6) & 1) != 0;
	if (!shared)
	{
		if (m_keyPos != 0)
			delete[] m_keyPos;
		if (m_keyRot != 0)
			delete[] m_keyRot;
	}
	if (m_next != 0)
	{
		delete m_next;
		m_next = 0;
	}
	if (m_child != 0)
	{
		delete m_child;
		m_child = 0;
	}
	--m_vtx_count;
	if (m_vtx_count == 0)
	{
		if (m_vtxList != 0)
		{
			delete[] m_vtxList;
			m_vtxList = 0;
			m_vtx_len = 0;
		}
	}
	m_bonescale = 1.0f;
}

// 983: o w_mesh tem construtor (tudo zero; f_d4_983 = -FLT_MAX)
w_mesh::w_mesh()
{
	vtxNum = 0;
	rigidNum = 0;
	blendedRigidNum = 0;
	blendedTotalNum = 0;
	vecList = 0;
	normList = 0;
	blendedVecList = 0;
	blendWeightList = 0;
	blendedNormalList = 0;
	originalVtxColorList = 0;
	maxBoneNum = 0;
	weightList = 0;
	indexNum = 0;
	indexList = 0;
	originalTexHandle = 0;
	originalTu = 0.0f;
	originalTv = 0.0f;
	originalScaleu = 0.0f;
	originalScalev = 0.0f;
	diffuse = 0;
	uv = 0;
	bEnv = false;
	bSpec = false;
	group = 0;
	flags = 0;
	next = 0;
	alpha = 0.0f;
	boneList = 0;
	blendedBoneList = 0;
	vtxColorList = 0;
	vtxColorPtrList = 0;
	myNormalList = 0;
	myBlendedNormalList = 0;
	xpShaderMtl983 = 0;
	texHandle = 0;
	tu = 0.0f;
	tv = 0.0f;
	scaleu = 0.0f;
	scalev = 0.0f;
	clipFlag = 0;
	drawFlag = 0;
	palette983 = 0;
	xpBatchGrp = 0;
	xpBatch = 0;
	xiBaseVtxIdx = 0;
	xiBaseIdxIdx = 0;
	xiDrawFlag2 = 0;
	aabbBone = 0;
	f_d4_983 = -FLT_MAX;
}

// 983 00ba7d60: construtor + operator=; listas com new[] (sem g_mem); UVs copiadas inteiras se houver grupo
w_mesh* WBone::CopyMesh(w_mesh* source)
{
	w_mesh* mesh = new w_mesh;
	*mesh = *source;
	mesh->next = 0;
	mesh->drawFlag = mesh->drawFlag & 0xfffff800 | source->texHandle;
	mesh->alpha = 1.0f;
	mesh->diffuse = source->diffuse;
	mesh->vtxColorList = new ulong[mesh->vtxNum];
	int i;
	for (i = 0; i < source->vtxNum; ++i)
		mesh->vtxColorList[i] = source->diffuse;
	if (mesh->rigidNum > 0)
	{
		mesh->boneList = new WBone*[mesh->rigidNum];
		mesh->vtxColorPtrList = new ulong*[mesh->rigidNum];
		memcpy(mesh->boneList, source->boneList, mesh->rigidNum * 4);
	}
	if (mesh->blendedRigidNum > 0)
	{
		mesh->blendedBoneList = new WBone*[mesh->blendedTotalNum];
		memcpy(mesh->blendedBoneList, source->blendedBoneList,
			mesh->blendedTotalNum * 4);
	}
	if (source->uv != 0 && mesh->group != 0)
		mesh->uv = new w_uvset983(*source->uv);
	if (((((ulong)m_flag) >> 16) & 1) != 0)
	{
		mesh->myNormalList = new WVector[mesh->vtxNum];
		memcpy(mesh->myNormalList, mesh->normList, mesh->vtxNum * 12);
		if (mesh->blendedTotalNum > 0)
		{
			mesh->myBlendedNormalList = new WVector[mesh->blendedTotalNum];
			memcpy(mesh->myBlendedNormalList, mesh->blendedNormalList,
				mesh->blendedTotalNum * 12);
		}
	}
	return mesh;
}

void WBone::CopyBone(w_common_bone_data* data)
{
	m_id = data->xid;
	SetName(data->name);
	m_bound_sphere = data->boundsphere;
	m_bound_aabb = data->aabb;
	m_keyPos = data->keyPos;
	m_keyPosNum = data->keyPosNum;
	m_keyRot = data->keyRot;
	m_keyRotNum = data->keyRotNum;
	m_keyScale = data->keyScale; // 983
	m_keyScaleNum = data->keyScaleNum;
	m_key983_ec = data->keyFov983;
	m_key983_f0 = data->keyFov983Num;
	m_keyFlag = data->keyFlag;
	SetBasicMatrix(data->basicMat);
	m_petBoneFloat983 = data->petBoneFloat983; // 983
	if (data->mesh != 0)
	{
		m_mesh = CopyMesh(data->mesh);
		if (m_mesh->rigidNum != 0 || m_mesh->blendedRigidNum != 0)
			m_flag.Enable(4);
		w_mesh* last = m_mesh;
		for (w_mesh* source = data->mesh->next; source != 0;
			source = source->next)
		{
			last->next = CopyMesh(source);
			if (last->next->rigidNum != 0 || last->next->blendedRigidNum != 0)
				m_flag.Enable(4);
			last = last->next;
		}
	}
	m_flag.Enable(0x60);
}

// 983 00b99e20 (nome meu): o inverso do CopyBone
void WBone::GetCommonData983(w_common_bone_data* data)
{
	data->xid = m_id;
	strcpy(data->name, m_name);
	data->boundsphere = m_bound_sphere;
	data->basicMat = m_basicMat;
	data->petBoneFloat983 = m_petBoneFloat983;
	data->aabb = m_bound_aabb;
	data->keyPos = m_keyPos;
	data->keyRot = m_keyRot;
	data->keyScale = m_keyScale;
	data->keyFov983 = m_key983_ec;
	data->keyPosNum = m_keyPosNum;
	data->keyRotNum = m_keyRotNum;
	data->keyScaleNum = m_keyScaleNum;
	data->keyFov983Num = m_key983_f0;
	data->keyFlag = m_keyFlag;
	data->mesh = m_mesh;
}

// 983 00ba4050 (nome meu)
bool WBone::AddOwner983(WPuppet* pet)
{
	if (std::find(m_list983_1d0.begin(), m_list983_1d0.end(), pet) != m_list983_1d0.end())
		return false;
	m_list983_1d0.push_back(pet);
	return true;
}

// 983 00ba3ff0 (nome meu)
bool WBone::RemoveOwner983(WPuppet* pet)
{
	std::list<WPuppet*>::iterator it = std::find(m_list983_1d0.begin(), m_list983_1d0.end(), pet);
	if (it == m_list983_1d0.end())
		return false;
	m_list983_1d0.erase(it);
	return true;
}

// 983 00ba4b40 (nome meu)
void WBone::AddOwnerUp983(WPuppet* pet)
{
	for (WBone* bone = m_parent; bone != 0 && bone->IsExtraBone983(); bone = bone->m_parent)
	{
		if (!bone->AddOwner983(pet))
			break;
	}
	AddOwner983(pet);
}

// 983 00ba9500: UVs num objeto so (apagado sem compartilhar, ou com grupo); listas com delete[] (sem g_mem)
void WBone::ClearMesh(w_mesh* mesh)
{
	if (mesh->next != 0)
	{
		ClearMesh(mesh->next);
		if (mesh->next != 0)
		{
			delete mesh->next;
			mesh->next = 0;
		}
	}
	bool shared = ((((ulong)m_flag) >> 5) & 1) != 0;
	if (!shared)
	{
		if (mesh->indexList != 0)
		{
			delete[] mesh->indexList;
			mesh->indexList = 0;
		}
		if (mesh->normList != 0)
		{
			delete[] mesh->normList;
			mesh->normList = 0;
		}
		if (mesh->vecList != 0)
		{
			delete[] mesh->vecList;
			mesh->vecList = 0;
		}
		if (mesh->weightList != 0)
		{
			delete[] mesh->weightList;
			mesh->weightList = 0;
		}
		if (mesh->originalVtxColorList != 0)
		{
			delete[] mesh->originalVtxColorList;
			mesh->originalVtxColorList = 0;
		}
		goto clear_uv;
	}
	if (mesh->group != 0)
	{
clear_uv:
		if (mesh->uv != 0)
		{
			delete mesh->uv;
			mesh->uv = 0;
		}
	}
	if (mesh->vtxColorList != 0)
	{
		delete[] mesh->vtxColorList;
		mesh->vtxColorList = 0;
	}
	if (mesh->myBlendedNormalList != 0)
	{
		delete[] mesh->myBlendedNormalList;
		mesh->myBlendedNormalList = 0;
	}
	if (mesh->myNormalList != 0)
	{
		delete[] mesh->myNormalList;
		mesh->myNormalList = 0;
	}
	if (mesh->blendedTotalNum > 0)
	{
		bool blendShared = ((((ulong)m_flag) >> 5) & 1) != 0;
		if (!blendShared)
		{
			if (mesh->blendWeightList != 0)
			{
				delete[] mesh->blendWeightList;
				mesh->blendWeightList = 0;
			}
			if (mesh->blendedVecList != 0)
			{
				delete[] mesh->blendedVecList;
				mesh->blendedVecList = 0;
			}
			if (mesh->blendedNormalList != 0)
			{
				delete[] mesh->blendedNormalList;
				mesh->blendedNormalList = 0;
			}
		}
		if (mesh->blendedBoneList != 0)
		{
			delete[] mesh->blendedBoneList;
			mesh->blendedBoneList = 0;
		}
	}
	if (mesh->rigidNum > 0)
	{
		if (mesh->boneList != 0)
		{
			delete[] mesh->boneList;
			mesh->boneList = 0;
		}
		if (mesh->vtxColorPtrList != 0)
		{
			delete[] mesh->vtxColorPtrList;
			mesh->vtxColorPtrList = 0;
		}
	}
}

// 983 00ba8100
WBone* WBone::MakeClone(WBone* parent, WBone* prev)
{
	bool bClone = (((ulong)m_flag >> 16) & 1) == 0 && !IsExtraBone983();
	if (bClone)
	{
		w_common_bone_data data;
		GetCommonData983(&data);
		WBone* clone = new WBone;
		clone->CopyBone(&data);
		if (parent != 0)
			clone->SetParent(parent);
		else if (prev != 0)
			prev->SetNext(clone, true);
		if (m_child != 0)
			m_child->MakeClone(clone, 0);
		if (m_next != 0)
			m_next->MakeClone(parent, clone);
		return clone;
	}
	if (m_next != 0)
		return m_next->MakeClone(parent, prev);
	return 0;
}

void WBone::MergeMesh(WBone* source, WBone* root)
{
	WMatrix matrix;
	matrix = source->m_matrix * ~m_matrix;
	if (m_mesh != 0)
	{
		w_mesh* last;
		for (last = m_mesh; last->next; last = last->next)
		{
		}
		last->next = source->m_mesh;
	}
	else
	{
		m_mesh = source->m_mesh;
	}
	for (w_mesh* mesh = source->m_mesh; mesh != 0; mesh = mesh->next)
	{
		int i;
		for (i = 0; i < mesh->vtxNum; ++i)
		{
			WVector& vec = mesh->vecList[i];
			vec = WVector(matrix.xx * vec.x + matrix.xz * vec.z +
					matrix.xy * vec.y + matrix.xm,
				matrix.yx * vec.x + matrix.yz * vec.z + matrix.yy * vec.y +
					matrix.ym,
				matrix.zx * vec.x + matrix.zz * vec.z + matrix.zy * vec.y +
					matrix.zm);
		}
		for (i = 0; i < mesh->rigidNum; ++i)
		{
			WBone*& bone = mesh->boneList[i];
			bone = root->FindBone(bone->m_name, 0);
		}
		for (i = 0; i < mesh->blendedTotalNum; ++i)
		{
			WBone*& bone = mesh->blendedBoneList[i];
			bone = root->FindBone(bone->m_name, 0);
		}
	}
	source->m_mesh = 0;
}

bool WBone::CheckRigidVtx(WBone* bone)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		int i;
		for (i = 0; i < mesh->rigidNum; ++i)
		{
			if (mesh->boneList[i] == bone)
				return true;
		}
		for (i = 0; i < mesh->blendedTotalNum; ++i)
		{
			if (mesh->blendedBoneList[i] == bone)
				return true;
		}
	}
	if (m_next != 0 && m_next->CheckRigidVtx(bone))
		return true;
	if (m_child != 0 && m_child->CheckRigidVtx(bone))
		return true;
	return false;
}

// 983 00ba5310
void WBone::ResetMeshBone(WPuppet* pet, WPuppet* owner, bool child)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		int i;
		for (i = 0; i < mesh->rigidNum; ++i)
		{
			if (mesh->boneList[i] != 0)
			{
				WBone* bone = pet->FindBone(mesh->boneList[i]->m_name);
				if (bone == 0)
					bone = pet->CreateMissingBone983(mesh->boneList[i], owner);
				if (bone->IsExtraBone983())
					bone->AddOwnerUp983(owner);
				mesh->boneList[i] = bone;
			}
		}
		for (i = 0; i < mesh->blendedTotalNum; ++i)
		{
			if (mesh->blendedBoneList[i] != 0)
			{
				WBone* bone = pet->FindBone(mesh->blendedBoneList[i]->m_name);
				if (bone == 0)
					bone = pet->CreateMissingBone983(mesh->blendedBoneList[i], owner);
				if (bone->IsExtraBone983())
					bone->AddOwnerUp983(owner);
				mesh->blendedBoneList[i] = bone;
			}
		}
		if (mesh->aabbBone != 0)
			mesh->aabbBone = pet->FindBone(mesh->aabbBone->m_name);
	}
	if (child)
	{
		if (m_next != 0)
			m_next->ResetMeshBone(pet, owner, true);
		if (m_child != 0)
			m_child->ResetMeshBone(pet, owner, true);
	}
}

void WBone::SetName(char* name)
{
	strcpy(m_name, name);
	m_hashCode = GetHashCode(m_name);
	// 983: m_motion_bone_name nao existe mais
}


void WBone::ReleaseChild(WBone* child)
{
	WBone* current = m_child;
	if (current == child)
	{
		m_child = current->m_next;
		return;
	}
	while (current != 0)
	{
		if (current->m_next == child)
		{
			current->m_next = child->m_next;
			return;
		}
		current = current->m_next;
	}
}

void WBone::SetNext(WBone* next, bool append)
{
	if (next && append)
	{
		WBone* tail;
		for (tail = next; tail->m_next; tail = tail->m_next)
		{
		}
		tail->m_next = m_next;
	}
	m_next = next;
}

void WBone::SetBasicMatrix(const WMatrix& matrix)
{
	// 983 (00b94260): quaternion da matriz com as linhas normalizadas; escala basica = comprimento das 3 linhas;
	// m_scale = comprimento da linha za da matriz local
	WMatrix norm = matrix;
	norm.Normalize();
	m_basicMat = matrix;
	m_localMat = matrix;
	m_matrix = matrix;
	float lx = matrix.xa.x * matrix.xa.x + matrix.xa.y * matrix.xa.y + matrix.xa.z * matrix.xa.z;
	float ly = matrix.ya.x * matrix.ya.x + matrix.ya.y * matrix.ya.y + matrix.ya.z * matrix.ya.z;
	float lz = matrix.za.x * matrix.za.x + matrix.za.y * matrix.za.y + matrix.za.z * matrix.za.z;
	m_basicScale983.x = (float)sqrt(lx);
	m_basicScale983.y = (float)sqrt(ly);
	m_basicScale983.z = (float)sqrt(lz);
	m_basicQuat = norm;
	m_basicQuat.Normalize();
	float len = m_localMat.za.x * m_localMat.za.x + m_localMat.za.y * m_localMat.za.y +
		m_localMat.za.z * m_localMat.za.z;
	m_scale = (float)sqrt(len);
}

inline char* WBone::GetNameSubBip(void)
{
	char* found = strstr(m_name, "Bip");
	if (found == 0)
		found = strstr(m_name, "bip");
	if (found != 0)
	{
		while (found[3] != 0)
		{
			if (found[3] != ' ' && found[2] == ' ')
				break;
			++found;
		}
		return found + 3;
	}
	return m_name;
}

void WBone::CopyBasicMatrix(WBone* root, bool child)
{
	WBone* target = root->FindBone(m_name, 0);
	if (target == 0 && strstr(m_name, "Bip") != 0)
		target = root->FindBone(GetNameSubBip(), 0);
	if (target != 0)
	{
		WMatrix matrix;
		matrix = GetKeyRot(0.0f);
		matrix.pivot = GetKeyPos(0.0f);
		target->SetBasicMatrix(matrix);
	}
	if (child)
	{
		if (m_next != 0)
			m_next->CopyBasicMatrix(root, child);
		if (m_child != 0)
			m_child->CopyBasicMatrix(root, child);
	}
}

void WBone::SetMatrix(const WMatrix& matrix, WView* view)
{
	if ((((ulong)m_flag) & 0x600) != 0)
	{
		bool rotated = ((((ulong)m_flag) >> 9) & 1) != 0;
		if (rotated)
			m_matrix = m_rotateMat * matrix;
		bool moved = ((((ulong)m_flag) >> 10) & 1) != 0;
		if (moved)
		{
			m_matrix.pivot -= m_moveVec * m_localMat;
		}
	}
	else
	{
		m_matrix = matrix;
	}
	for (WBone* bone = m_child; bone != 0; bone = bone->m_next)
		bone->SetMatrix(bone->m_localMat * m_matrix, view);
}

WVector WBone::GetKeyPos(float time)
{
	if (m_keyPos == 0)
		return m_basicMat.pivot;
	int low = 0;
	int high = m_keyPosNum - 1;
	int mid;
	do
	{
		mid = (low + high) >> 1;
		if (m_keyPos[mid].time >= time)
			high = mid;
		else
			low = mid + 1;
	} while (low < high);
	if (m_keyPos[mid].time != time)
	{
		if (mid != 0 || m_keyPos[0].time < time)
		{
			int index = mid;
			if (m_keyPos[mid].time > time)
				index = mid - 1;
			float rate = (time - m_keyPos[index].time) /
				(m_keyPos[high].time - m_keyPos[index].time);
			return (m_keyPos[high].pos - m_keyPos[index].pos) * rate +
				m_keyPos[index].pos;
		}
	}
	return m_keyPos[mid].pos;
}

// 983 (00b94730): igual a GetKeyPos, sobre as chaves de escala
WVector WBone::GetKeyScale(float time)
{
	if (m_keyScale == 0)
		return m_basicScale983;
	int low = 0;
	int high = m_keyScaleNum - 1;
	int mid;
	do
	{
		mid = (low + high) >> 1;
		if (m_keyScale[mid].time >= time)
			high = mid;
		else
			low = mid + 1;
	} while (low < high);
	if (m_keyScale[mid].time != time)
	{
		if (mid != 0 || m_keyScale[0].time < time)
		{
			int index = mid;
			if (m_keyScale[mid].time > time)
				index = mid - 1;
			float rate = (time - m_keyScale[index].time) /
				(m_keyScale[high].time - m_keyScale[index].time);
			return (m_keyScale[high].pos - m_keyScale[index].pos) * rate +
				m_keyScale[index].pos;
		}
	}
	return m_keyScale[mid].pos;
}

float WBone::GetKeyFOV983(float time)
{
	if (m_key983_ec == 0)
		return -1.0f;
	int low = 0;
	int high = m_key983_f0 - 1;
	int mid;
	do
	{
		mid = (low + high) >> 1;
		if (m_key983_ec[mid].time >= time)
			high = mid;
		else
			low = mid + 1;
	} while (low < high);
	if (m_key983_ec[mid].time != time)
	{
		if (mid != 0 || m_key983_ec[0].time < time)
		{
			int index = mid;
			if (m_key983_ec[mid].time > time)
				index = mid - 1;
			float rate = (time - m_key983_ec[index].time) /
				(m_key983_ec[high].time - m_key983_ec[index].time);
			return (m_key983_ec[high].fov - m_key983_ec[index].fov) * rate +
				m_key983_ec[index].fov;
		}
	}
	return m_key983_ec[mid].fov;
}

WQuat WBone::GetKeyRot(float time)
{
	if (0)
		Abs(time);
	if (m_keyRot == 0)
		return GetBasicRot();
	int low = 0;
	int high = m_keyRotNum - 1;
	int mid;
	do
	{
		mid = (low + high) >> 1;
		if (m_keyRot[mid].time >= time)
			high = mid;
		else
			low = mid + 1;
	} while (low < high);
	if (m_keyRot[mid].time != time && (mid != 0 || m_keyRot[0].time < time))
	{
		int index = mid;
		if (m_keyRot[mid].time > time)
			index = mid - 1;
		float rate = (time - m_keyRot[index].time) /
			(m_keyRot[high].time - m_keyRot[index].time);
		return WQuaternionSlerp(m_keyRot[index].rot, m_keyRot[high].rot, rate);
	}
	return m_keyRot[mid].rot;
}

// 983 (00ba2270): chave local com escala; sem conjunto, aplica direto no osso; filtro por nome opcional.
void WBone::InsertBoneSetList(WBoneSet* set, float time, int flag,
	WPuppet* filter)
{
	if (filter != 0 && !filter->FindBone(m_name))
		return;
	WBoneKey key;
	if ((flag & 4) == 0 || strstr(m_name, "tail") == 0)
	{
		if ((flag & 2) != 0)
		{
			key.name = m_name;
			key.quat = GetKeyRot(time);
			key.pivot = GetKeyPos(time);
			key.scale = GetKeyScale(time);
			key.flags = 7;
			if (set != 0)
				set->AddBoneKey(m_name, &key.quat, &key.pivot, &key.scale, 7);
			else
				ApplyBoneKey(&key);
		}
		else if (m_keyFlag != 0)
		{
			key.name = m_name;
			key.flags = (unsigned char)m_keyFlag;
			if (m_keyRot != 0)
				key.quat = GetKeyRot(time);
			if (m_keyPos != 0)
				key.pivot = GetKeyPos(time);
			if (m_keyScale != 0)
				key.scale = GetKeyScale(time);
			if (set != 0)
				set->AddBoneKey(m_name, &key.quat, &key.pivot, &key.scale,
					key.flags);
			else
				ApplyBoneKey(&key);
		}
	}
	if ((flag & 1) != 0)
	{
		for (WBone* bone = m_child; bone != 0; bone = bone->m_next)
			bone->InsertBoneSetList(set, time, flag, filter);
	}
}

WVector WBone::GetDeltaVec(float time)
{
	const WVector* src;
	w_keyframe_pos* keys = m_keyPos;
	if (keys != 0 && time > 0.0f)
	{
		WVector pos = GetKeyPos(time) - keys[0].pos;
		src = &pos;
	}
	else
	{
		src = &WVector::ZERO;
	}
	return *src;
}


// 983 (00b94cf0): o que a chave nao traz vem dos valores basicos do osso
void WBone::ApplyBoneKey(WBoneKey* key)
{
	const WVector& pos = (key->flags & 2) ? key->pivot : m_basicMat.pivot;
	const WQuat& quat = (key->flags & 1) ? key->quat : m_basicQuat;
	if (key->flags & 4)
		SetLocalMatrix(key->scale, quat, pos);
	else
		SetLocalMatrix(quat, pos);
}

void WBone::ApplyBoneSetList(WBoneSet* set)
{
	WBone* bone = this;
	do
	{
		WBoneKey* key = set->m_keyList.Find(bone->m_name);
		if (key != 0 && key->flags != 0)
			bone->ApplyBoneKey(key);
		if (bone->m_child != 0)
			bone->m_child->ApplyBoneSetList(set);
		bone = bone->m_next;
	} while (bone != 0);
}

void WBone::SetKeyframe(w_keyframe* frame, float end, float start)
{
	m_keyPosNum = 0;
	m_keyRotNum = 0;
	for (w_keyframe* key = frame; key != 0; key = key->next)
	{
		if (key->q != 0)
			++m_keyRotNum;
		if (key->v != 0)
			++m_keyPosNum;
		if (key->next == 0)
		{
			if (m_keyRotNum != 0 && (key->q == 0 || key->t < end))
				++m_keyRotNum;
			if (m_keyPosNum != 0 && (key->v == 0 || key->t < end))
				++m_keyPosNum;
		}
	}
	m_keyFlag = 0;
	if (m_keyRotNum != 0)
	{
		m_keyRot = new w_keyframe_rot[m_keyRotNum];
		m_keyFlag |= 1;
	}
	if (m_keyPosNum != 0)
	{
		m_keyPos = new w_keyframe_pos[m_keyPosNum];
		m_keyFlag |= 2;
	}
	int rot = 0;
	int pos = 0;
	for (w_keyframe* key = frame; key != 0; key = key->next)
	{
		if (key->q != 0)
		{
			m_keyRot[rot].time = key->t - start;
			m_keyRot[rot].rot = *key->q;
			++rot;
		}
		if (key->v != 0)
		{
			m_keyPos[pos].time = key->t - start;
			m_keyPos[pos].pos = *key->v;
			++pos;
		}
	}
	if (rot != 0 && rot != m_keyRotNum)
	{
		m_keyRot[rot].time = end - start;
		m_keyRot[rot].rot = m_keyRot[rot - 1].rot;
	}
	if (pos != 0 && pos != m_keyPosNum)
	{
		m_keyPos[pos].time = end - start;
		m_keyPos[pos].pos = m_keyPos[pos - 1].pos;
	}
}

bool WBone::CheckSuitableForKeyframe(float start, float end, float (*range)[2],
	int count, bool last)
{
	for (int i = 0; i < count; ++i)
	{
		if (last == 0)
		{
			if (start <= range[i][0] && range[i][0] <= end)
				return true;
		}
		else if (last == 1)
		{
			if (start <= range[i][1] && range[i][1] <= end)
				return true;
		}
	}
	if (count == 0)
		return true;
	return false;
}

bool __fastcall VECEQU(const WVector& left, const WVector& right)
{
	float diff;
	diff = left.x - right.x;
	if (Abs(diff) <= 0.0001f)
	{
		diff = left.y - right.y;
		if (Abs(diff) <= 0.0001f)
		{
			diff = left.z - right.z;
			if (Abs(diff) <= 0.0001f)
				return true;
		}
	}
	return false;
}

bool __fastcall QUATEQU(const WQuat& left, const WQuat& right)
{
	float diff;
	diff = left.x - right.x;
	if ((diff > 0.0f ? diff : -diff) <= 0.0001f)
	{
		diff = left.y - right.y;
		if ((diff > 0.0f ? diff : -diff) <= 0.0001f)
		{
			diff = left.z - right.z;
			if ((diff > 0.0f ? diff : -diff) <= 0.0001f)
			{
				if (Abs(left.w - right.w) <= 0.0001f)
					return true;
			}
		}
	}
	return false;
}

int WBone::OptimizeRotKey(float (*range)[2], int count, w_keyframe_rot** out)
{
	int* map = new int[m_keyRotNum];
	int num = 0;
	int i;
	int prev;
	WQuat rot;
	for (i = 0; i < m_keyRotNum; ++i)
	{
		if (i > 0 && i < m_keyRotNum - 1)
		{
			float t = (m_keyRot[i].time - m_keyRot[prev].time) /
				(m_keyRot[i + 1].time - m_keyRot[prev].time);
			rot = WQuaternionSlerp(m_keyRot[prev].rot, m_keyRot[i + 1].rot, t);
			if (QUATEQU(rot, m_keyRot[i].rot))
			{
				map[i] = -1;
				continue;
			}
		}
		map[i] = num;
		prev = i;
		++num;
	}
	if (num < m_keyRotNum && out != 0)
	{
		w_keyframe_rot* keys = new w_keyframe_rot[num];
		for (i = 0; i < m_keyRotNum; ++i)
		{
			if (map[i] >= 0)
				memcpy(&keys[map[i]], &m_keyRot[i], sizeof(w_keyframe_rot));
		}
		*out = keys;
	}
	delete[] map;
	return num;
}

int WBone::OptimizePosKey(float (*range)[2], int count, w_keyframe_pos** out)
{
	int* map = new int[m_keyPosNum];
	int i = 0;
	int num = 0;

	int prev;
	WVector pos;
	for (; i < m_keyPosNum; ++i)
	{
		if (i > 0 && i < m_keyPosNum - 1)
		{
			float t = (m_keyPos[i].time - m_keyPos[prev].time) /
				(m_keyPos[i + 1].time - m_keyPos[prev].time);
			pos = (m_keyPos[i + 1].pos - m_keyPos[prev].pos) * t +
				m_keyPos[prev].pos;
			if (VECEQU(pos, m_keyPos[i].pos))
			{
				map[i] = -1;
				continue;
			}
		}
		map[i] = num;
		prev = i;
		++num;
	}
	if (num < m_keyPosNum && out != 0)
	{
		w_keyframe_pos* keys = new w_keyframe_pos[num];
		for (i = 0; i < m_keyPosNum; ++i)
		{
			if (map[i] >= 0)
				memcpy(&keys[map[i]], &m_keyPos[i], sizeof(w_keyframe_pos));
		}
		*out = keys;
	}
	delete[] map;
	return num;
}

void WBone::OptimizeBoneSet(float (*range)[2], int count, int child)
{
	w_keyframe_rot* rot = m_keyRot;
	w_keyframe_pos* pos = m_keyPos;
	if (m_keyRot != 0)
	{
		m_keyRotNum = OptimizeRotKey(range, count, &m_keyRot);
		if (rot != m_keyRot)
			delete[] rot;
	}
	if (m_keyPos != 0)
	{
		m_keyPosNum = OptimizePosKey(range, count, &m_keyPos);
		if (pos != m_keyPos)
			delete[] pos;
	}
	if (m_next != 0 && child != 0)
		m_next->OptimizeBoneSet(range, count, 1);
	if (m_child != 0 && child != 0)
		m_child->OptimizeBoneSet(range, count, 1);
}

// 983 00bab400: como no 645, mas cada malha leva uma paleta de ossos (no maximo WBonePalette983::ms_maxBones):
// face que nao cabe fica para a proxima malha do mesmo material; material = textura + grupo + camada da face.
void WBone::SetMesh(w_pet_vertex* vertexList, int vtxNum,
	w_pet_tri_point* pointList, w_pet_face983* faces, WBone** boneList,
	int faceNum, WPuppet* pet)
{
	int owned = 0;
	int pick = 0;
	int face;
	for (face = 0; face < faceNum; ++face)
	{
		int j;
		for (j = 0; j < 3; ++j)
		{
			if (j != 0)
			{
				WBone* held = boneList[face * 3 + pick];
				WBone* other = boneList[face * 3 + j];
				if (held == other)
					continue;
				if (other->FindBone(held->m_name, 0) == 0)
					continue;
			}
			pick = j;
		}
		if (boneList[face * 3 + pick] == this)
			++owned;
	}

	if (m_mesh != 0)
	{
		ClearMesh(m_mesh);
		if (m_mesh != 0)
		{
			delete m_mesh;
			m_mesh = 0;
		}
	}

	if (owned != 0)
	{
		int* faceList = new int[owned];
		int* groupList = new int[owned];
		int* write = faceList;
		for (face = 0; face < faceNum; ++face)
		{
			int j;
			for (j = 0; j < 3; ++j)
			{
				if (j != 0)
				{
					WBone* held = boneList[face * 3 + pick];
					WBone* other = boneList[face * 3 + j];
					if (held == other)
						continue;
					if (other->FindBone(held->m_name, 0) == 0)
						continue;
				}
				pick = j;
			}
			if (boneList[face * 3 + pick] == this)
			{
				*write = face;
				++write;
			}
		}
		int done = 0;
		while (done < owned)
		{
			WBonePalette983 palette;
			WTriBones983 tri;
			int scan;
			for (scan = 0; scan < owned; ++scan)
			{
				if (faceList[scan] >= 0)
					break;
			}
			w_pet_face983* first = &faces[faceList[scan]];
			int used = 0;
			for (int i = 0; i < owned; ++i)
			{
				int index = faceList[i];
				if (index < 0)
					continue;
				w_pet_face983* cur = &faces[index];
				if (cur->tex->texHandle != first->tex->texHandle ||
					cur->tex->group != first->tex->group ||
					cur->layer != first->layer)
					continue;
				w_pet_tri_point* point = &pointList[index * 3];
				tri.Set(&vertexList[point[0].pos], &vertexList[point[1].pos],
					&vertexList[point[2].pos]);
				if (palette.AddTriangle(tri.bones))
				{
					++done;
					groupList[used] = faceList[i];
					faceList[i] = -1;
					++used;
				}
			}
			if (used != 0)
			{
				w_mesh* mesh = GetIndexedmesh(vertexList, pointList, boneList,
					faces, used, groupList);
				mesh->palette983 = pet->GetPalette983(palette);
				mesh->group = first->tex->group;
				mesh->next = m_mesh;
				m_mesh = mesh;
				if (mesh->rigidNum != 0 || mesh->blendedRigidNum != 0)
					m_flag.Enable(4);
			}
		}
		delete[] groupList;
		delete[] faceList;
	}

	if (m_next != 0)
		m_next->SetMesh(vertexList, vtxNum, pointList, faces, boneList,
			faceNum, pet);
	if (m_child != 0)
		m_child->SetMesh(vertexList, vtxNum, pointList, faces, boneList,
			faceNum, pet);
}

// 983 00bab7f0: as faces de um osso (pet de malha), divididas pela paleta de ossos como no SetMesh
void WBone::SetBoneMesh(w_pet_vertex* vertexList, int vtxNum,
	w_pet_tri_point* pointList, w_pet_face983* faceList, WBone** boneList,
	int faceNum, WPuppet* pet)
{
	if (m_mesh != 0)
	{
		ClearMesh(m_mesh);
		if (m_mesh != 0)
		{
			delete m_mesh;
			m_mesh = 0;
		}
	}
	int* mark = new int[faceNum];
	int* faces = new int[faceNum];
	memset(mark, 0, faceNum * 4);
	int done = 0;
	if (faceNum > 0)
	{
		do
		{
			WBonePalette983 palette;
			WTriBones983 tri;
			int i = 0;
			do
			{
				if (mark[i] >= 0)
					break;
				++i;
			} while (i < faceNum);
			w_pet_face983* first = &faceList[i];
			int count = 0;
			for (; i < faceNum; ++i)
			{
				w_pet_face983* cur = &faceList[i];
				if (cur->tex->texHandle == first->tex->texHandle &&
					cur->tex->group == first->tex->group &&
					cur->layer == first->layer && mark[i] == 0)
				{
					w_pet_tri_point* point = &pointList[i * 3];
					tri.Set(&vertexList[point[0].pos], &vertexList[point[1].pos],
						&vertexList[point[2].pos]);
					if (palette.AddTriangle(tri.bones))
					{
						faces[count] = i;
						mark[i] = -1;
						++count;
					}
				}
			}
			if (count != 0)
			{
				done += count;
				m_flag.Enable(0x10000);
				w_mesh* mesh = GetIndexedmesh(vertexList, pointList, boneList,
					faceList, count, faces);
				mesh->palette983 = pet->GetPalette983(palette);
				mesh->group = first->tex->group;
				mesh->next = m_mesh;
				m_mesh = mesh;
				if (mesh->rigidNum != 0 || mesh->blendedRigidNum != 0)
					m_flag.Enable(4);
			}
		} while (done < faceNum);
	}
	delete[] faces;
	delete[] mark;
}

struct w_temp_vertex_list
{
	w_temp_vertex_list() { }
	short index;
	WVector pos;
	WVector normal;
	float weight;
	WBone* bone;
	unsigned char uvNum; // 983: camadas de UV do ponto
	float uv[w_pet_tri_point::MAX_UV983][2];
	float tu; // GetIndexedmesh(w_faces**) 00ba8240
	float tv;
	w_vertex* gvertex;
	w_pet_vertex* vertex;
	int kind;
};

// 983: coordenada de UV da chave do mapa de vertices; < 00b93ca0 e > 00b93ce0 (u, depois v). Nome meu.
struct w_vertex_uv983
{
	float u, v;
	bool operator<(const w_vertex_uv983& o) const;
	bool operator>(const w_vertex_uv983& o) const;
};

// 983: chave do mapa de vertices do GetIndexedmesh 00ba9e10 (0x54): posicao, peso e UVs exatos. Nome meu.
struct w_vertex_key983
{
	WVector pos;			// +0x00
	float weight;			// +0x0c
	unsigned char uvNum;	// +0x10
	w_vertex_uv983 uv[w_pet_tri_point::MAX_UV983];	// +0x14
	bool operator<(const w_vertex_key983& o) const;	// 00b94d40
};

// 983: normal e osso do vertice (0x10); == 00b94e50 (normal a 0.001, mesmo osso). Nome meu.
struct w_vertex_attr983
{
	WVector normal;
	WBone* bone;
	bool operator==(const w_vertex_attr983& o) const;
};

struct w_vertex_slot983
{
	w_vertex_slot983(const w_vertex_attr983& a, int i) : attr(a), index(i) { }
	w_vertex_attr983 attr;
	int index;
};

// 983: vertices repetidos da malha (contador + mapa chave -> normais/ossos com o indice). Nome meu.
class w_vertex_index983
{
public:
	w_vertex_index983() : m_count(0) { }
	int Add(const w_vertex_key983& key, const w_vertex_attr983& attr, bool& added);	// 00ba9680

	int m_count;
	std::map<w_vertex_key983, std::vector<w_vertex_slot983> > m_map;
};

bool w_vertex_uv983::operator<(const w_vertex_uv983& o) const
{
	if (u < o.u)
		return true;
	if (u > o.u)
		return false;
	if (v < o.v)
		return true;
	return false;
}

bool w_vertex_uv983::operator>(const w_vertex_uv983& o) const
{
	if (u > o.u)
		return true;
	if (u < o.u)
		return false;
	if (v > o.v)
		return true;
	return false;
}

bool w_vertex_key983::operator<(const w_vertex_key983& o) const
{
	if (pos.x < o.pos.x)
		return true;
	if (pos.x > o.pos.x)
		return false;
	if (pos.y < o.pos.y)
		return true;
	if (pos.y > o.pos.y)
		return false;
	if (pos.z < o.pos.z)
		return true;
	if (pos.z > o.pos.z)
		return false;
	if (weight < o.weight)
		return true;
	if (weight > o.weight)
		return false;
	if (uvNum < o.uvNum)
		return true;
	if (uvNum > o.uvNum)
		return false;
	for (unsigned char i = 0; i < uvNum; ++i)
	{
		const w_vertex_uv983& a = uv[i];
		const w_vertex_uv983& b = o.uv[i];
		if (a < b)
			return true;
		if (a > b)
			return false;
	}
	return false;
}

bool w_vertex_attr983::operator==(const w_vertex_attr983& o) const
{
	if (fabs(normal.x - o.normal.x) > 0.001f)
		return false;
	if (fabs(normal.y - o.normal.y) > 0.001f)
		return false;
	if (fabs(normal.z - o.normal.z) > 0.001f)
		return false;
	return bone == o.bone;
}

int w_vertex_index983::Add(const w_vertex_key983& key,
	const w_vertex_attr983& attr, bool& added)
{
	std::map<w_vertex_key983, std::vector<w_vertex_slot983> >::iterator it =
		m_map.find(key);
	if (it == m_map.end())
	{
		m_map[key].push_back(w_vertex_slot983(attr, m_count));
		++m_count;
		added = true;
		return m_count - 1;
	}
	std::vector<w_vertex_slot983>& slots = it->second;
	std::vector<w_vertex_slot983>::iterator p;
	for (p = slots.begin(); p != slots.end(); ++p)
	{
		if (p->attr == attr)
			break;
	}
	if (p == slots.end())
	{
		slots.push_back(w_vertex_slot983(attr, m_count));
		++m_count;
		added = true;
		return m_count - 1;
	}
	added = false;
	return p->index;
}

// 983: vertice provisorio do GetIndexedmesh 00ba9e10 (0x74; new[] sem construtor): chave (+0), normal/osso (+0x54)
struct w_temp_vertex983
{
	WVector pos;			// +0x00
	float weight;			// +0x0c
	unsigned char uvNum;	// +0x10
	float uv[w_pet_tri_point::MAX_UV983][2];	// +0x14
	WVector normal;			// +0x54
	WBone* bone;			// +0x60
	short index;			// +0x64
	int pad983_68;			// +0x68 sem uso no 983 (00ba9e10 so grava 0..0x64, 0x6c, 0x70)
	w_pet_vertex* vertex;	// +0x6c
	int kind;				// +0x70
};

// 983 00ba9e10: vertices repetidos pelo mapa (posicao/peso/UVs exatos, normal a 0.001 e mesmo osso; o 645
// comparava a posicao a 1e-5 contra todos os anteriores)
w_mesh* WBone::GetIndexedmesh(w_pet_vertex* vtxList, w_pet_tri_point* triList,
	WBone** triBoneTable, w_pet_face983* faces, int faceNum,
	int* faceIndex)
{
	short* table;
	int blendTotal = 0;
	int blendNum = 0;
	int maxBoneNum = 0;
	w_vertex_index983 vertexIndex;
	unsigned char uvNum = 0;
	WMatrix invMat;
	invMat = ~m_matrix;

	int l = faceNum * 3;
	w_temp_vertex983* list = new w_temp_vertex983[l];
	int rigidNum = 0;
	int i, j, k;
	for (i = 0; i < l; ++i)
	{
		w_pet_tri_point* point = &triList[faceIndex[i / 3] * 3 + i % 3];
		w_vertex_key983 key;
		key.pos = WVector(vtxList[point->pos].x, vtxList[point->pos].y,
			vtxList[point->pos].z);
		key.weight = vtxList[point->pos].weight;
		key.uvNum = point->uvNum983;
		memcpy(key.uv, point->uv983, key.uvNum * 8);
		w_vertex_attr983 attr;
		attr.normal = WVector(point->nx, point->ny, point->nz);
		bool added = false;
		attr.bone = triBoneTable[point - triList];
		short index = (short)vertexIndex.Add(key, attr, added);
		list[i].index = index;
		if (added)
		{
			w_temp_vertex983* t = &list[index];
			t->pos = key.pos;
			t->weight = key.weight;
			t->uvNum = key.uvNum;
			memcpy(t->uv, key.uv, key.uvNum * 8);
			t->normal = attr.normal;
			t->bone = attr.bone;
			t->vertex = &vtxList[point->pos];
			t->kind = vtxList[point->pos].list != 0 ? 2 : (attr.bone != this);
			if (t->kind != 0)
				++rigidNum;
			if (t->kind > 1)
			{
				++blendNum;
				blendTotal += t->vertex->listNum;
			}
			if (uvNum == 0)
				uvNum = t->uvNum;
		}
	}

	w_mesh* mesh = new w_mesh; // 983: construtor zera (sem memset)
	mesh->originalTexHandle = mesh->texHandle = faces[faceIndex[0]].tex->texHandle;
	mesh->originalTu = mesh->tu = faces[faceIndex[0]].tex->pu;
	mesh->originalTv = mesh->tv = faces[faceIndex[0]].tex->pv;
	mesh->originalScaleu = mesh->scaleu = faces[faceIndex[0]].tex->scalex;
	mesh->originalScalev = mesh->scalev = faces[faceIndex[0]].tex->scaley;
	mesh->diffuse = faces[faceIndex[0]].tex->diffuse;
	mesh->group = 0;
	mesh->next = 0;
	mesh->alpha = (float)(faces[faceIndex[0]].tex->alpha / 255.0f);
	mesh->flags = faces[faceIndex[0]].tex->drawFlags | (mesh->alpha < 1.0f ? 0x400000 : 0); // 983: 0x400000 (645: 0x20400000)
	mesh->xiDrawFlag2 = faces[faceIndex[0]].tex->xulDrawFlag2;
	mesh->bEnv = faces[faceIndex[0]].tex->mapType == 1 || faces[faceIndex[0]].tex->mapType == 2 ? true : false;
	mesh->bSpec = faces[faceIndex[0]].tex->mapType == 3;
	// 983: material da camada da face, com os mesmos bits de desenho da malha
	mesh->xpShaderMtl983 = (WxShaderMtl*)faces[faceIndex[0]].layerMtl;
	if (mesh->xpShaderMtl983 != 0)
		mesh->xpShaderMtl983->SetRenderFlags(mesh->flags, mesh->xiDrawFlag2);
	mesh->indexNum = l;
	mesh->indexList = new unsigned short[l];

	mesh->rigidNum = rigidNum;
	mesh->vtxColorPtrList =
		rigidNum > 0 ? new ulong*[rigidNum] : 0;
	mesh->boneList =
		mesh->rigidNum > 0 ? new WBone*[mesh->rigidNum] : 0;
	if (mesh->boneList != 0)
		memset(mesh->boneList, 0, mesh->rigidNum * 4);
	mesh->blendedRigidNum = blendNum;
	mesh->blendedTotalNum = blendTotal;
	mesh->blendWeightList =
		blendTotal > 0 ? new float[blendTotal] : 0;
	mesh->blendedBoneList =
		blendTotal > 0 ? new WBone*[blendTotal] : 0;
	mesh->blendedVecList = blendTotal > 0 ? new WVector[blendTotal] : 0;
	mesh->blendedNormalList = blendTotal > 0 ? new WVector[blendTotal] : 0;

	mesh->vtxNum = vertexIndex.m_count;
	mesh->vecList = new WVector[mesh->vtxNum];
	mesh->normList = new WVector[mesh->vtxNum];
	mesh->vtxColorList = new ulong[mesh->vtxNum];
	mesh->originalVtxColorList = new ulong[mesh->vtxNum];
	mesh->myBlendedNormalList = 0;
	mesh->myNormalList = 0;
	if (((((ulong)m_flag) >> 16) & 1) != 0)
		mesh->weightList = new float[mesh->vtxNum];
	else
		mesh->weightList = 0;
	// 983: camadas de UV = as do 1o ponto (2 com mapa de ambiente/brilho: a 2a e copia da 1a)
	if (uvNum != 0)
	{
		if (faces[faceIndex[0]].tex->mapType < 2)
		{
			w_uvset983* uv = new w_uvset983;
			uv->resize(uvNum);
			for (unsigned char layer = uvNum; layer != 0; --layer)
				(*uv)[uvNum - layer].resize(mesh->vtxNum);
			mesh->uv = uv;
		}
		else
		{
			w_uvset983* uv = new w_uvset983;
			uv->resize(2);
			for (w_uvset983::iterator it = uv->begin(); it != uv->end(); ++it)
				it->resize(mesh->vtxNum);
			mesh->uv = uv;
		}
	}
	for (i = 0; i < mesh->vtxNum; ++i)
	{
		mesh->vtxColorList[i] = mesh->diffuse;
		mesh->originalVtxColorList[i] = mesh->vtxColorList[i];
	}
	AllocVTX(mesh->vtxNum);

	table = new short[mesh->vtxNum];
	int n;
	for (n = 0, i = 0, j = 0, k = blendNum, l = rigidNum; i < mesh->indexNum;
		++i)
	{
		if (list[i].index == n)
		{
			switch (list[n].kind)
			{
			case 0:
				table[n] = (short)l++;
				break;
			case 1:
				table[n] = (short)k++;
				break;
			case 2:
				table[n] = (short)j++;
				break;
			}
			++n;
		}
	}

	for (i = 0, k = 0, n = 0; i < mesh->indexNum; ++i)
	{
		mesh->indexList[i] = table[list[i].index];
		if (list[i].index != n)
			continue;
		if (table[n] < blendNum)
		{
			for (j = 0; j < list[n].vertex->listNum; ++j)
			{
				mesh->blendedBoneList[k] = list[n].vertex->list[j].bone;
				mesh->blendWeightList[k] =
					(float)list[n].vertex->list[j].blendWeight / 255.0f;
				mesh->blendedVecList[k] =
					list[n].pos * ~mesh->blendedBoneList[k]->GetMatrix();
				mesh->blendedNormalList[k] = RotVec(list[n].normal,
					~mesh->blendedBoneList[k]->GetMatrix());
				mesh->blendedNormalList[k].Normalize();
				++k;
			}
			if (maxBoneNum < list[n].vertex->listNum)
				maxBoneNum = list[n].vertex->listNum;
		}
		if (table[n] < rigidNum)
		{
			mesh->boneList[table[n]] = list[n].bone;
			mesh->vecList[table[n]] = list[n].pos * ~list[n].bone->m_matrix;
			mesh->vtxColorPtrList[table[n]] = &mesh->vtxColorList[table[n]];
			mesh->normList[table[n]] =
				RotVec(list[n].normal, ~list[n].bone->m_matrix);
		}
		else
		{
			mesh->vecList[table[n]] = list[n].pos * invMat;
			mesh->normList[table[n]] = RotVec(list[n].normal, invMat);
		}
		if (mesh->weightList != 0)
			mesh->weightList[table[n]] = list[n].weight;
		mesh->normList[table[n]].Normalize();
		if (uvNum != 0)
		{
			w_uv983* uv = &(*mesh->uv)[0][table[n]];
			int* face = &faceIndex[i / 3];
			uv->u = list[n].uv[0][0] * faces[*face].tex->scalex + faces[*face].tex->pu;
			uv->v = list[n].uv[0][1] * faces[*face].tex->scaley + faces[*face].tex->pv;
			if (faces[faceIndex[0]].tex->mapType > 1)
			{
				// 983: 2a camada = copia da 1a (ambiente/brilho)
				(*mesh->uv)[1][table[n]].u = uv->u;
				(*mesh->uv)[1][table[n]].v = uv->v;
			}
			else
			{
				// 983: as outras camadas vao cruas (sem escala/deslocamento)
				for (unsigned char layer = 1; layer < uvNum; ++layer)
				{
					(*mesh->uv)[layer][table[n]].u = list[n].uv[layer][0];
					(*mesh->uv)[layer][table[n]].v = list[n].uv[layer][1];
				}
			}
		}
		++n;
	}

	if (maxBoneNum == 0 && mesh->rigidNum > 0)
		maxBoneNum = 1;
	mesh->maxBoneNum = maxBoneNum;
	CalcMeshAABB(*this, *mesh);
	delete[] table;
	delete[] list;
	return mesh;
}

void WBone::SetMesh(w_faces* faces, int count)
{
	m_mesh = 0;
	if (count != 0)
	{
		w_faces** list = new w_faces*[count];
		int i;
		for (i = 0; i < count; ++i)
			list[i] = &faces[i];
		qsort(list, count, 4, SortByTextureHandle);
		int first = 0;
		int last = 0;
		while (last < count)
		{
			while (
				last < count && list[first]->texhandle == list[last]->texhandle)
				++last;
			w_mesh* mesh = GetIndexedmesh(&list[first], last - first);
			mesh->next = m_mesh;
			m_mesh = mesh;
			first = last;
		}
		delete[] list;
	}
}

int __cdecl WBone::SortByTextureHandle(const void* left, const void* right)
{
	w_faces* a = *(w_faces**)left;
	w_faces* b = *(w_faces**)right;
	if (a->texhandle == b->texhandle)
		return a->p[0].bone - b->p[0].bone;
	return a->texhandle - b->texhandle;
}

w_mesh* WBone::GetIndexedmesh(w_faces** faces, int count)
{
	int rigidNum = 0;
	int blendedRigidNum = 0;
	int blendedTotalNum = 0;
	w_temp_vertex_list* list = new w_temp_vertex_list[count * 3];
	int cornerNum = count * 3;

	int i, j, k, l, n;
	WVector vec;
	WMatrix invMat;
	for (i = 0; i < cornerNum; ++i)
	{
		w_faces::w_faces_info* p = &faces[i / 3]->p[i % 3];
		list[i].index = -1;
		list[i].pos = p->object->gvertex[p->vindex].v;
		list[i].normal = p->norm;
		list[i].bone = p->object->gvertex[p->vindex].obj != 0
			? p->object->gvertex[p->vindex].obj->bone
			: p->object->bone;
		list[i].tu = p->tu;
		list[i].tv = p->tv;
		list[i].kind = p->object->gvertex[p->vindex].rigidNum != 0
			? 2
			: (list[i].bone != this);
		list[i].gvertex = &p->object->gvertex[p->vindex];
	}

	for (i = 0, n = 0; i < cornerNum; ++i)
	{
		for (j = 0; j <= i; ++j)
		{
			w_faces::w_faces_info* p = &faces[j / 3]->p[j % 3];
			if (list[i].tu == p->tu && list[i].tv == p->tv &&
				WisEqual(list[i].pos, p->object->gvertex[p->vindex].v, 1e-5f) &&
				list[i].bone ==
					(p->object->gvertex[p->vindex].obj != 0
							? p->object->gvertex[p->vindex].obj->bone
							: p->object->bone) &&
				(list[i].normal - p->norm).Magnitude() <= 0.01f)
			{
				if (i == j)
				{
					list[i].index = (short)n++;
					if (list[i].kind != 0)
						++rigidNum;
					if (list[i].kind == 2)
					{
						blendedTotalNum += list[i].gvertex->rigidNum;
						++blendedRigidNum;
					}
				}
				else
				{
					list[i].index = list[j].index;
				}
			}
		}
	}

	w_mesh* mesh = new w_mesh; // 983: construtor zera (sem memset)
	mesh->originalTexHandle = mesh->texHandle = faces[0]->texhandle;
	mesh->originalTu = mesh->tu = 0.0f;
	mesh->originalTv = mesh->tv = 0.0f;
	mesh->originalScaleu = mesh->scaleu = 1.0f;
	mesh->originalScalev = mesh->scalev = 1.0f;
	mesh->diffuse = faces[0]->diffuse;
	mesh->group = 0;
	mesh->next = 0;
	mesh->alpha = (float)((unsigned char*)&faces[0]->diffuse)[3] / 255.0f;
	mesh->flags = (mesh->alpha < 1.0f ? 0x400000 : 0) | faces[0]->mapFlags; // 983 00ba8240: 0x400000 (645: 0x20400000 = sem ZWRITE)
	mesh->xiDrawFlag2 = faces[0]->xiMapFlags2;
	mesh->bEnv =
		faces[0]->mapType == 1 || faces[0]->mapType == 2 ? true : false;
	mesh->bSpec = faces[0]->mapType == 3;
	mesh->indexNum = cornerNum;
	mesh->indexList = new unsigned short[cornerNum];

	mesh->rigidNum = rigidNum;
	mesh->vtxColorPtrList =
		rigidNum > 0 ? new ulong*[rigidNum] : 0;
	mesh->boneList =
		mesh->rigidNum > 0 ? new WBone*[mesh->rigidNum] : 0;
	if (mesh->rigidNum > 0)
		memset(mesh->boneList, 0, mesh->rigidNum * 4);
	mesh->blendedRigidNum = blendedRigidNum;
	mesh->blendWeightList =
		blendedTotalNum > 0 ? new float[blendedTotalNum] : 0;
	mesh->blendedBoneList =
		blendedTotalNum > 0 ? new WBone*[blendedTotalNum] : 0;
	mesh->blendedVecList =
		blendedTotalNum > 0 ? new WVector[blendedTotalNum] : 0;
	mesh->blendedNormalList =
		blendedTotalNum > 0 ? new WVector[blendedTotalNum] : 0;
	mesh->blendedTotalNum = blendedTotalNum;
	mesh->vtxNum = n;
	mesh->vecList = new WVector[mesh->vtxNum];
	mesh->normList = new WVector[mesh->vtxNum];
	mesh->vtxColorList = new ulong[mesh->vtxNum];
	mesh->originalVtxColorList = new ulong[mesh->vtxNum];
	mesh->myBlendedNormalList = 0;
	mesh->myNormalList = 0;
	mesh->weightList = 0;
	// 983: camadas de UV (2 com mapa de ambiente/brilho; PROVISORIO: o 983 le o no de camadas da face)
	mesh->NewUV983(faces[0]->mapType > 1 ? 2 : 1, mesh->vtxNum);
	float (*uvData)[2] = mesh->UVData983();
	float (*uvBackup)[2] = mesh->UVBackup983();
	for (i = 0; i < mesh->vtxNum; ++i)
	{
		mesh->originalVtxColorList[i] = mesh->vtxColorList[i] = mesh->diffuse;
	}
	AllocVTX(mesh->vtxNum);

	short* slotList = new short[mesh->vtxNum];
	for (n = 0, i = 0, j = 0, l = blendedRigidNum, k = rigidNum;
		i < mesh->indexNum; ++i)
	{
		if (list[i].index == n)
		{
			switch (list[i].kind)
			{
			case 0:
				slotList[n] = (short)k++;
				break;
			case 1:
				slotList[n] = (short)l++;
				break;
			case 2:
				slotList[n] = (short)j++;
				break;
			}
			++n;
		}
	}

	for (i = 0, n = 0, k = 0; i < mesh->indexNum; ++i)
	{
		mesh->indexList[i] = slotList[list[i].index];
		if (list[i].index != n)
			continue;
		mesh->vecList[slotList[n]] = list[i].pos;
		if (list[i].gvertex->obj == 0)
		{
			mesh->normList[slotList[n]] = list[i].normal;
		}
		else
		{
			mesh->normList[slotList[n]] = RotVec(list[i].normal,
				m_matrix * ~list[i].gvertex->obj->matrix);
		}
		mesh->normList[slotList[n]].Normalize();
		if (slotList[n] < mesh->blendedRigidNum)
		{
			for (l = 0; l < list[i].gvertex->rigidNum; ++l, ++k)
			{
				WMatrix mInv;
				mInv = ~list[i].gvertex->rigidVtx[l].obj->matrix;
				mesh->blendedBoneList[k] = list[i].gvertex->rigidVtx[l].bone;
				mesh->blendWeightList[k] =
					list[i].gvertex->rigidVtx[l].blendWeight;
				w_3d_object* object = list[i].gvertex->obj;
				mesh->blendedVecList[k] = list[i].pos * object->matrix * mInv;
				const WMatrix& boneMatrix = m_matrix;
				mesh->blendedNormalList[k] =
					RotVec(list[i].normal, boneMatrix * mInv);
				mesh->blendedNormalList[k].Normalize();
			}
		}
		if (slotList[n] < rigidNum)
		{
			mesh->boneList[slotList[n]] = list[i].bone;
			mesh->vtxColorPtrList[slotList[n]] =
				&mesh->vtxColorList[slotList[n]];
		}
		uvData[slotList[n]][0] = list[i].tu;
		uvData[slotList[n]][1] = list[i].tv;
		if (uvBackup != 0)
		{
			uvBackup[slotList[n]][0] = list[i].tu;
			uvBackup[slotList[n]][1] = list[i].tv;
		}
		++n;
	}
	CalcMeshAABB(*this, *mesh);
	delete[] slotList;
	delete[] list;
	return mesh;
}

int WBone::GetMeshForFaceAnimation(unsigned char group, w_mesh** list,
	int count)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if (mesh->group == group)
		{
			if (list != 0)
				list[count] = mesh;
			++count;
		}
	}
	if (m_next != 0)
		count = m_next->GetMeshForFaceAnimation(group, list, count);
	if (m_child != 0)
		count = m_child->GetMeshForFaceAnimation(group, list, count);
	return count;
}

int WBone::GetMeshForFaceAnimation(int handle, w_mesh** list, int count)
{
	static int faceGroup = 1;
	w_mesh* mesh = m_mesh;
	if (mesh != 0)
	{
		int group = faceGroup;
		do
		{
			if (mesh->originalTexHandle == handle || handle == 0)
			{
				if (list != 0)
				{
					list[count] = mesh;
					if (mesh->group == 0 && handle != 0)
					{
						if (count == 0)
							++group;
						mesh->group = group;
					}
				}
				++count;
			}
			mesh = mesh->next;
		} while (mesh != 0);
		faceGroup = group;
	}
	if (m_next != 0)
		count = m_next->GetMeshForFaceAnimation(handle, list, count);
	if (m_child != 0)
		count = m_child->GetMeshForFaceAnimation(handle, list, count);
	return count;
}

void WBone::CombineVtxColor(bool child)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		int i;
		for (i = 0; i < mesh->blendedRigidNum; ++i)
			mesh->vtxColorPtrList[i] = &mesh->vtxColorList[i];
		for (; i < mesh->rigidNum; ++i)
		{
			ulong* color = (ulong*)mesh->boneList[i]->FindVColor(
				mesh->vecList[i], mesh->normList[i]);
			if (color == 0)
				color = &mesh->vtxColorList[i];
			mesh->vtxColorPtrList[i] = color;
		}
	}
	if (child)
	{
		if (m_next != 0)
			m_next->CombineVtxColor(true);
		if (m_child != 0)
			m_child->CombineVtxColor(true);
	}
}

ulong* WBone::FindVColor(const WVector& pos, const WVector& normal)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		for (int i = mesh->rigidNum; i < mesh->vtxNum; ++i)
		{
			if (WisEqual(mesh->vecList[i], pos, 1.0e-5f) &&
				WisEqual(mesh->normList[i], normal, 1.0e-5f))
				return (ulong*)&mesh->vtxColorList[i];
		}
	}
	return 0;
}

void WBone::Transform(const WMatrix& mat, WView* view, float matscale, int flag)
{
	WSphere tr_bound_sphere;
	float scale;
	if (!(flag & NOUPDATEMAT))
	{
		if (m_flag.GetFlag(ROTATEBONE | MOVEBONE | SCALEBONE))
		{
			if (m_flag.GetFlag(ROTATEBONE))
				m_matrix = m_rotateMat * m_localMat;
			else
				m_matrix = m_localMat;
			if (m_flag.GetFlag(SCALEBONE))
				m_matrix.AxisScale(
					WVector(m_bonescale, m_bonescale, m_bonescale));
			if (m_flag.GetFlag(MOVEBONE))
				m_matrix.pivot -= m_moveVec;
			m_matrix = m_matrix * mat;
		}
		else
		{
			m_matrix = m_localMat * mat;
		}
	}
	if (view)
	{
		scale = matscale * m_scale;
		tr_bound_sphere.pos = Transform(m_bound_sphere.pos);
		scale *= m_bound_sphere.radius;
		tr_bound_sphere = WSphere(tr_bound_sphere.pos, scale);
		bool inside = view->InFrustum(tr_bound_sphere) == true ||
			WisZero(tr_bound_sphere.radius, g_EPSILON);
		m_flag.Turn(VISIBLE,
			m_flag.GetFlag(HASRIGIDVTX) || (flag & NOCLIP) || inside);
		if (m_flag.GetFlag(HASRIGIDVTX))
			m_flag.Turn(CLIPPING, !(flag & NOCLIP));
		else
			m_flag.Turn(CLIPPING,
				!(flag & NOCLIP) &&
					(!IsVisible() || !view->InFrustumSafe(tr_bound_sphere)));
	}
	if (m_next)
		m_next->Transform(mat, view, matscale, flag);
	if (m_child)
		m_child->Transform(m_matrix, view, matscale * m_scale, flag);
}

void WBone::DisableFog(void)
{
	// 983 00b95190: recursivo (filho e irmaos) e limpa o bit 0x10000000 (o mesmo que o WView
	// usa como DisableFog: flag & 0xefffffff); o 645 limpava 0x08000000 e so a lista da bone
	WBone* bone = this;
	do
	{
		for (w_mesh* mesh = bone->m_mesh; mesh != 0; mesh = mesh->next)
			mesh->drawFlag &= 0xefffffffUL;
		if (bone->m_child != 0)
			bone->m_child->DisableFog();
		bone = bone->m_next;
	} while (bone != 0);
}

void WBone::EnableAlphaBlend(void)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if ((mesh->drawFlag & 0x80000000UL) != 0)
			mesh->drawFlag = mesh->drawFlag & 0x7fbfffffUL | 0x00400000UL;
	}
}

void WBone::SetRenderMode(int mode)
{
	m_rendMode = mode;
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		switch (mode)
		{
		case 0:
			mesh->drawFlag =
				(((mesh->flags & 0x1800000) != 0x800000) ? 0x10000000 : 0) |
				(mesh->texHandle & 0x3ffff) | mesh->flags;
			break;
		case 1:
			mesh->drawFlag =
				(mesh->texHandle & 0x3ffff) | mesh->flags | 0x40000000;
			break;
		case 5:
			mesh->drawFlag =
				(((mesh->flags & 0x1800000) != 0x800000) ? 0x10000000 : 0) |
				(mesh->texHandle & 0x3ffff) | mesh->flags | 0x40000;
			break;
		case 2:
			mesh->drawFlag = mesh->flags;
			break;
		case 3:
			mesh->drawFlag = 0x8000000; // 983 00b95260: bits do 983 (645: 0x4000000; modos 0/5 0x8000000)
			break;
		case 4:
			mesh->drawFlag = (mesh->texHandle & 0x3ffff) | 0x20300000;
			break;
		}
	}
	if (m_next != 0)
		m_next->SetRenderMode(mode);
	if (m_child != 0)
		m_child->SetRenderMode(mode);
}

void WBone::SetAlpha(unsigned char alpha, int recurse)
{
	if (m_alpha != alpha)
	{
		m_alpha = alpha;
		if (alpha == 0)
			m_flag.Enable(1);
		if ((((ulong)m_flag) & 1) != 0 && alpha > 0)
			m_flag.Disable(1);
		for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
		{
			ulong color = (ulong)(int)((float)alpha * mesh->alpha) << 24;
			if (color == 0xff000000)
			{
				int flag = mesh->drawFlag;
				if ((flag & 0x1800000) == 0 && (mesh->flags & 0x400000) == 0)
					flag &= 0xffbfffff;
				else
					flag |= 0x400000;
				mesh->drawFlag = flag;
			}
			else
			{
				// 983 00b95390: só 0x400000 (o 645 também desligava a escrita de Z com 0x20000000)
				mesh->drawFlag |= 0x400000;
			}
			for (int i = 0; i < mesh->vtxNum; ++i)
			{
				ulong& vtxColor = mesh->vtxColorList[i];
				vtxColor = (vtxColor & 0xffffff) | color;
			}
		}
	}
	if (recurse != 0)
	{
		if (m_next != 0)
			m_next->SetAlpha(alpha, 1);
		if (m_child != 0)
			m_child->SetAlpha(alpha, 1);
	}
}

void WBone::RestoreVtxColor(void)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
		for (int index = 0; index < mesh->vtxNum; ++index)
			mesh->vtxColorList[index] = mesh->originalVtxColorList[index];
}

void WBone::SetLightLine(float value)
{
	WBone* bone = this;
	do
	{
		bone->m_lightLine = value;
		if (bone->m_next != 0)
			bone->m_next->SetLightLine(value);
		bone = bone->m_child;
	} while (bone != 0);
}

void WBone::SetLight(LightSet* light)
{
	memcpy(&m_light, light, sizeof(m_light));
	if (m_next != 0)
		m_next->SetLight(light);
	if (m_child != 0)
		m_child->SetLight(light);
}

void WBone::CalcLight(WScene* scene, int flag)
{
	bool selfIllum = ((((ulong)m_flag) >> 1) & 1) != 0;
	if (!selfIllum && scene != 0)
	{
		WVector pos = m_bound_sphere.pos * m_matrix;
		CalcLight(scene->GetLightSet(pos), false, flag, 0);
	}
	else
	{
		int mode = flag | 0x4000000;
		if ((mode & 0x4000000) != 0 || selfIllum)
			CalcLight_SelfIllum();
		else
			CalcLight_Diffuse_Equals_Ambient(0, mode);
	}
	if (m_next != 0)
		m_next->CalcLight(scene, flag);
	if (m_child != 0)
		m_child->CalcLight(scene, flag);
}

void WBone::CalcLight(LightSet* light, bool child, int flag, WScene* scene)
{
	bool selfIllum = ((((ulong)m_flag) >> 1) & 1) != 0;
	if ((flag & 0x4000000) == 0 && !selfIllum)
	{
		if (light != 0)
		{
			if (light->type == 1)
			{
				if (flag == 0x1000000)
					CalcLight_Point(light, flag, scene);
				else
					CalcLight_Directional_Per_Bone(light, flag, scene);
			}
			else if (light->type == 2)
			{
				CalcLight_Directional(light, flag, scene);
			}
		}
		else
		{
			CalcLight_Diffuse_Equals_Ambient(0, flag);
		}
	}
	else
	{
		CalcLight_SelfIllum();
	}
	if (child)
	{
		if (m_next != 0)
			m_next->CalcLight(light, child, flag, scene);
		if (m_child != 0)
			m_child->CalcLight(light, child, flag, scene);
	}
}

void WBone::CalcLight_SelfIllum(void)
{
	w_mesh* mesh = m_mesh;
	if (mesh != 0)
	{
		do
		{
			ulong alpha = (ulong)(int)((float)m_alpha * mesh->alpha) << 24;
			ulong color;
			if (mesh->texHandle == 0)
				color = Modulate(m_selfillumColor, mesh->diffuse);
			else
				color = m_selfillumColor;
			color |= alpha;
			for (int i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] = color;
			mesh = mesh->next;
		} while (mesh != 0);
	}
	if (m_vtxColor != 0xffffffff)
	{
		for (w_mesh* target = m_mesh; target != 0; target = target->next)
		{
			if ((target->drawFlag & 0x1000000) == 0)
			{
				for (int i = 0; i < target->vtxNum; ++i)
					target->vtxColorList[i] &= m_vtxColor;
			}
		}
	}
}

void WBone::CalcLight_Point(LightSet* lig, int lightmode, WScene* scene)
{
	w_mesh* mesh = m_mesh;
	if (mesh == 0)
		return;

	ulong diffuse, ambient, calcAlpha, c;
	WVector vLocLightPos, vLightDir;
	float t;
	int i, k;
	vLocLightPos = lig->nearOne * ~m_matrix;

	for (; mesh != 0; mesh = mesh->next)
	{
		if (mesh->texHandle == 0)
		{
			diffuse = Modulate(lig->diffuse, mesh->diffuse);
			ambient = Modulate(lig->ambient, mesh->diffuse);
		}
		else
		{
			diffuse = lig->diffuse;
			ambient = lig->ambient;
		}
		calcAlpha = (ulong)(int)(m_alpha * mesh->alpha) << 24;

		if ((mesh->xiDrawFlag2 & 0x4) != 0)
		{
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] = lig->ambient2 | calcAlpha;
			continue;
		}
		if ((mesh->xiDrawFlag2 & 0x10) != 0)
		{
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] = calcAlpha | 0xffffff;
			continue;
		}
		if ((lightmode & 0x2000000) != 0)
			continue;

		k = 0;
		for (i = 0; i < mesh->blendedRigidNum; ++i)
		{
			WVector v, vN;
			vN.x = vN.y = vN.z = 0.0f;
			v.x = v.y = v.z = 0.0f;
			float fTotal = 0.0f;
			for (; fTotal < 0.999f; ++k)
			{
				v += mesh->blendedBoneList[k]->Transform(
						 mesh->blendedVecList[k]) *
					mesh->blendWeightList[k];
				vN += RotVec(mesh->blendedNormalList[k],
						  mesh->blendedBoneList[k]->GetMatrix()) *
					mesh->blendWeightList[k];
				fTotal += mesh->blendWeightList[k];
			}
			vN.Normalize();
			vLightDir = (lig->nearOne - v).Normalize();
			t = (float)(vLightDir * vN) * 255.0f;
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}

		for (; i < mesh->rigidNum; ++i)
		{
			WVector v, vN;
			v = mesh->boneList[i]->Transform(mesh->vecList[i]);
			vN = RotVec(mesh->normList[i], mesh->boneList[i]->GetMatrix());
			vN.Normalize();
			vLightDir = (lig->nearOne - v).Normalize();
			t = vLightDir * vN * 255.0f;
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}

		for (; i < mesh->vtxNum; ++i)
		{
			vLightDir = (vLocLightPos - mesh->vecList[i]).Normalize();
			t = vLightDir * mesh->normList[i] * 255.0f;
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}
	}

	if (m_vtxColor != 0xffffffff)
	{
		for (mesh = m_mesh; mesh != 0; mesh = mesh->next)
		{
			if ((mesh->drawFlag & 0x1000000) != 0)
				continue;
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] &= m_vtxColor;
		}
	}
}

void WBone::CalcLight_Directional_Per_Bone(LightSet* lig, int lightmode,
	WScene* scene)
{
	if (m_mesh == 0)
		return;

	float t;
	ulong diffuse, ambient, calcAlpha;
	WVector vWorLightDir;
	ulong c;
	int i, k;
	w_mesh* mesh;
	WVector vLocLightDir;
	if (m_flag.GetFlag(4))
	{
		vWorLightDir = lig->nearOne - m_matrix.pivot;
		vWorLightDir = vWorLightDir.Normalize() * 255.0f;
		vLocLightDir = RotVec(vWorLightDir, ~m_matrix);
		vLocLightDir = vLocLightDir.Normalize() * 255.0f;
	}
	else
	{
		vLocLightDir = RotVec(lig->nearOne - m_matrix.pivot, ~m_matrix);
		vLocLightDir.Normalize();
	}

	for (mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if (mesh->texHandle == 0)
		{
			diffuse = Modulate(lig->diffuse, mesh->diffuse);
			ambient = Modulate(lig->ambient, mesh->diffuse);
		}
		else
		{
			diffuse = lig->diffuse;
			ambient = lig->ambient;
		}
		calcAlpha = (ulong)(int)(m_alpha * mesh->alpha) << 24;

		if ((mesh->xiDrawFlag2 & 0x4) != 0)
		{
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] = lig->ambient2 | calcAlpha;
			continue;
		}
		if ((mesh->xiDrawFlag2 & 0x10) != 0)
		{
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] = calcAlpha | 0xffffff;
			continue;
		}
		if ((lightmode & 0x2000000) != 0)
			continue;

		for (k = 0, i = 0; i < mesh->blendedRigidNum; ++i)
		{
			WVector vN;
			float fTotal;
			vN.x = vN.y = vN.z = 0.0f;
			for (fTotal = 0.0f; fTotal < 0.999f; ++k)
			{
				vN += RotVec(mesh->blendedNormalList[k],
						  mesh->blendedBoneList[k]->m_matrix) *
					mesh->blendWeightList[k];
				fTotal += mesh->blendWeightList[k];
			}
			vN.Normalize();
			t = (float)(vN * vWorLightDir);
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}

		for (; i < mesh->rigidNum; ++i)
		{
			WVector vN;
			vN = RotVec(mesh->normList[i], mesh->boneList[i]->GetMatrix());
			vN.Normalize();
			t = (float)(vN * vWorLightDir);
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}

		for (; i < mesh->vtxNum; ++i)
		{
			t = (float)(vLocLightDir * mesh->normList[i]);
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}
	}

	if (m_vtxColor != 0xffffffff)
	{
		for (mesh = m_mesh; mesh != 0; mesh = mesh->next)
		{
			if ((mesh->drawFlag & 0x1000000) != 0)
				continue;
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] &= m_vtxColor;
		}
	}
}

void WBone::CalcLight_Directional(LightSet* lig, int lightmode, WScene* scene)
{
	if (m_mesh == 0)
		return;

	ulong diffuse;
	float t;
	WVector vWorLightDir;
	ulong ambient;
	WVector vLocLightDir;
	ulong calcAlpha;
	w_mesh* mesh;
	int i, k;
	ulong c;
	if (m_flag.GetFlag(4))
		vWorLightDir = lig->nearOne * -255.0f;

	vLocLightDir = RotVec(lig->nearOne * -1.0f, ~m_matrix);
	vLocLightDir = vLocLightDir.Normalize() * 255.0f;

	for (mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		WVector* normList;
		WVector vN;
		if (mesh->texHandle == 0)
		{
			diffuse = Modulate(lig->diffuse, mesh->diffuse);
			ambient = Modulate(lig->ambient, mesh->diffuse);
		}
		else
		{
			diffuse = lig->diffuse;
			ambient = lig->ambient;
		}
		calcAlpha = (ulong)(int)(m_alpha * mesh->alpha) << 24;

		if ((mesh->xiDrawFlag2 & 0x4) != 0)
		{
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] = lig->ambient2 | calcAlpha;
			continue;
		}
		if ((mesh->xiDrawFlag2 & 0x10) != 0)
		{
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] = calcAlpha | 0xffffff;
			continue;
		}
		if ((lightmode & 0x2000000) != 0)
			continue;

		normList = mesh->myBlendedNormalList ? mesh->myBlendedNormalList
											 : mesh->blendedNormalList;
		if (normList != 0)
		{
			float fTotal;
			for (k = 0, i = 0; i < mesh->blendedRigidNum; ++i)
			{
				vN.Reset();
				for (fTotal = 0.0f;
					fTotal < 0.999999f && k < mesh->blendedTotalNum; ++k)
				{
					WVector tmp = RotVec(normList[k],
						mesh->blendedBoneList[k]->GetMatrix());
					float w = mesh->blendWeightList[k];
					float tz = tmp.z * w;
					float ty = tmp.y * w;
					float tx = tmp.x * w;
					vN += WVector(tx, ty, tz);
					fTotal += mesh->blendWeightList[k];
				}
				vN.Normalize();
				t = (float)(vN.x * vWorLightDir.x + vN.z * vWorLightDir.z +
					vN.y * vWorLightDir.y);
				if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
					t = t * -1.0f;
				c = t <= 0.0f ? ambient
							  : AddDiffuse(ambient, diffuse, (unsigned char)t);
				if ((lightmode & 0x8000000) != 0)
				{
					mesh->vtxColorList[i] =
						AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) |
						calcAlpha;
				}
				else
				{
					mesh->vtxColorList[i] = c | calcAlpha;
				}
			}
		}
		else
		{
			i = mesh->blendedRigidNum;
		}

		normList = mesh->myNormalList ? mesh->myNormalList : mesh->normList;
		if (normList == 0)
			continue;

		for (; i < mesh->rigidNum; ++i)
		{
			if (&mesh->vtxColorList[i] != mesh->vtxColorPtrList[i])
				continue;
			vN = RotVec(normList[i], mesh->boneList[i]->GetMatrix());
			vN.Normalize();
			t = (float)(vN.x * vWorLightDir.x + vN.z * vWorLightDir.z +
				vN.y * vWorLightDir.y);
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}

		for (; i < mesh->vtxNum; ++i)
		{
			t = vLocLightDir * normList[i];
			if ((mesh->xiDrawFlag2 & 0x200) != 0 && t < 0.0f)
				t = t * -1.0f;
			c = t <= 0.0f ? ambient
						  : AddDiffuse(ambient, diffuse, (unsigned char)t);
			if ((lightmode & 0x8000000) != 0)
			{
				mesh->vtxColorList[i] =
					AddDiffuse(mesh->vtxColorList[i] & 0xffffff, c) | calcAlpha;
			}
			else
			{
				mesh->vtxColorList[i] = c | calcAlpha;
			}
		}
	}

	if (m_vtxColor != 0xffffffff)
	{
		for (mesh = m_mesh; mesh != 0; mesh = mesh->next)
		{
			if ((mesh->drawFlag & 0x1000000) != 0)
				continue;
			for (i = 0; i < mesh->vtxNum; ++i)
				mesh->vtxColorList[i] &= m_vtxColor;
		}
	}
}

void WBone::CalcLight_Diffuse_Equals_Ambient(LightSet* light, int flag)
{
	w_mesh* mesh = m_mesh;
	if (mesh != 0)
	{
		int add = flag & 0x8000000;
		do
		{
			ulong color;
			if (light != 0)
			{
				if (mesh->texHandle == 0)
					color = Modulate(light->diffuse, mesh->diffuse);
				else
					color = light->diffuse;
			}
			else
			{
				color = 0;
			}
			ulong value =
				((ulong)(int)((float)m_alpha * mesh->alpha) << 24) | color;
			int i;
			if (add != 0)
			{
				for (i = 0; i < mesh->vtxNum; ++i)
					mesh->vtxColorList[i] =
						AddDiffuse(mesh->vtxColorList[i], value);
			}
			else
			{
				for (i = 0; i < mesh->vtxNum; ++i)
					mesh->vtxColorList[i] = value;
			}
			mesh = mesh->next;
		} while (mesh != 0);
	}
	if (m_vtxColor != 0xffffffff)
	{
		for (w_mesh* target = m_mesh; target != 0; target = target->next)
		{
			if ((target->drawFlag & 0x1000000) == 0)
			{
				for (int i = 0; i < target->vtxNum; ++i)
					target->vtxColorList[i] &= m_vtxColor;
			}
		}
	}
}

void WBone::ResetLight(void)
{
	WBone* bone = this;
	for (;;)
	{
		for (w_mesh* mesh = bone->m_mesh; mesh != 0; mesh = mesh->next)
			for (int index = 0; index < mesh->vtxNum; ++index)
				mesh->vtxColorList[index] = mesh->diffuse;
		if (bone->m_next != 0)
			bone->m_next->ResetLight();
		WBone* child = bone->m_child;
		if (child == 0)
			break;
		bone = child;
	}
}

void WBone::SetVtxColor(ulong color)
{
	m_vtxColor = color;
	RestoreVtxColor();
	if (m_next != 0)
		m_next->SetVtxColor(color);
	if (m_child != 0)
		m_child->SetVtxColor(color);
}

void WBone::CalcENVCoord(WView* view, w_mesh* m)
{
	int i;
	WMatrix trans;
	WVector norm, vb, va;
	if (m->UVBackup983())
	{
		for (i = 0; i < m->vtxNum; ++i)
		{
			va = m->vecList[i] * m_matrix - view->camera.pivot;
			vb = RotVec(m->normList[i], m_matrix);
			norm =
				va - vb * (2 * (va * vb) / (vb.Magnitude() * vb.Magnitude()));
			norm *= 0.5f / norm.Magnitude();
			m->UVData983()[i][0] = 0.5f - norm.x;
			m->UVData983()[i][1] = 0.5f - norm.y;
		}
	}
	else
	{
		trans = m_matrix * view->invcamera;
		trans *= 0.5f / WVectorLen(trans.xa);
		for (i = 0; i < m->vtxNum; ++i)
		{
			norm = RotVec(m->normList[i], trans);
			m->UVData983()[i][0] = norm.x + 0.5f;
			m->UVData983()[i][1] = 0.5f - norm.y;
		}
	}
}

void WBone::CalcHilightCoord(WView* view, const LightSet& light, w_mesh* mesh)
{
	int i = 0;
	ulong type;
	if (!(type = light.type))
		return;
	WMatrix mInv;
	mInv = ~m_matrix;
	if (type == 1)
		return;
	if (type != 2)
		return;
	{
		WVector vLocalL, vXA, vYA, vZA, vN, v, vLocalE;
		WVector* normList;
		normList = mesh->myBlendedNormalList ? mesh->myBlendedNormalList
											 : mesh->blendedNormalList;
		vLocalE = view->GetCamera().pivot;
		vLocalL = light.nearOne;
		vLocalL.Normalize();
		const WVector* direction = &light.nearOne;
		const WVector* camera = &view->GetCamera().pivot;
		int k = 0;
		for (i = 0; i < mesh->blendedRigidNum; ++i)
		{
			float fTotal;
			vN.Reset();
			v.Reset();
			for (fTotal = 0.0f; fTotal < 0.999999f; ++k)
			{
				vN +=
					RotVec(normList[k], mesh->blendedBoneList[k]->GetMatrix()) *
					mesh->blendWeightList[k];
				v += (mesh->blendedVecList[k] *
						 mesh->blendedBoneList[k]->GetMatrix()) *
					mesh->blendWeightList[k];
				fTotal += mesh->blendWeightList[k];
			}
			vN.Normalize();
			if ((float)(vLocalL * vN) >= 0.0f)
			{
				mesh->UVBackup983()[i][0] = mesh->UVBackup983()[i][1] = 0.75f;
			}
			else
			{
				vZA = (vLocalL + (v - vLocalE).Normalize()).Normalize();
				if ((float)(vZA * vN) >= 0.0f)
				{
					mesh->UVBackup983()[i][0] = mesh->UVBackup983()[i][1] = 0.75f;
				}
				else
				{
					if (WisEqual(vZA.y, -1.0f, g_EPSILON) ||
						WisEqual(vZA.y, 1.0f, g_EPSILON))
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_X, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}
					else
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_Y, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}
					mesh->UVBackup983()[i][0] = Wabs(vXA * vN);
					mesh->UVBackup983()[i][1] = Wabs(vYA * vN);
				}
			}
		}
		(void)(vLocalL * vN);
		for (; i < mesh->rigidNum; ++i)
		{
			vLocalL =
				RotVec(*direction, ~mesh->boneList[i]->GetMatrix()).Normalize();
			if (vLocalL * mesh->normList[i] >= 0.0f)
			{
				mesh->UVBackup983()[i][0] = mesh->UVBackup983()[i][1] = 0.75f;
			}
			else
			{
				vLocalE = *camera * ~mesh->boneList[i]->GetMatrix();
				vZA = (vLocalL + (mesh->vecList[i] - vLocalE).Normalize())
						  .Normalize();
				if (vZA * mesh->normList[i] >= 0.0f)
				{
					mesh->UVBackup983()[i][0] = mesh->UVBackup983()[i][1] = 0.75f;
				}
				else
				{
					if (WisEqual(vZA.y, -1.0f, g_EPSILON) ||
						WisEqual(vZA.y, 1.0f, g_EPSILON))
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_X, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}
					else
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_Y, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}
					mesh->UVBackup983()[i][0] = Wabs(vXA * mesh->normList[i]);
					mesh->UVBackup983()[i][1] = Wabs(vYA * mesh->normList[i]);
				}
			}
		}
		vLocalE = *camera * mInv;
		vLocalL = RotVec(*direction, mInv).Normalize();
		for (; i < mesh->vtxNum; ++i)
		{
			if ((float)(vLocalL * mesh->normList[i]) >= 0.0f)
			{
				mesh->UVBackup983()[i][0] = mesh->UVBackup983()[i][1] = 0.75f;
			}
			else
			{
				vZA = (vLocalL + (mesh->vecList[i] - vLocalE).Normalize())
						  .Normalize();
				if (vZA * mesh->normList[i] >= 0.0f)
				{
					mesh->UVBackup983()[i][0] = mesh->UVBackup983()[i][1] = 0.75f;
				}
				else
				{
					if (WisEqual(vZA.y, -1.0f, g_EPSILON) ||
						WisEqual(vZA.y, 1.0f, g_EPSILON))
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_X, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}
					else
					{
						vXA =
							WCrossProduct(WVector::UNIT_POS_Y, vZA).Normalize();
						vYA = WCrossProduct(vZA, vXA).Normalize();
					}
					mesh->UVBackup983()[i][0] = Wabs(vXA * mesh->normList[i]);
					mesh->UVBackup983()[i][1] = Wabs(vYA * mesh->normList[i]);
				}
			}
		}
	}
}

void RenderAABB(WView* view, const Waabb& aabb, const WMatrix& mat, float rlen)
{
	static int box_v[8][3] = {
		{ 0, 0, 0 },
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 1, 1, 0 },
        { 0, 0, 1 },
		{ 1, 0, 1 },
        { 0, 1, 1 },
        { 1, 1, 1 }
	};
	static int box_p[6][4] = {
		{ 0, 2, 3, 1 },
        { 4, 5, 7, 6 },
        { 0, 1, 5, 4 },
        { 3, 2, 6, 7 },
		{ 1, 3, 7, 5 },
        { 2, 0, 4, 6 }
	};
	WVector vt[8];
	for (int i = 0; i < 8; ++i)
		vt[i] = WVector(box_v[i][0] ? (aabb.max.x + rlen) : (aabb.min.x - rlen),
					box_v[i][1] ? (aabb.max.y + rlen) : (aabb.min.y - rlen),
					box_v[i][2] ? (aabb.max.z + rlen) : aabb.min.z - rlen) *
			mat;
	for (int f = 0; f < 6; ++f)
		for (int k = 0; k < 4; ++k)
			view->DrawLine(vt[box_p[f][(k - 1) & 3]], 0xffff0000,
				vt[box_p[f][k]], 0xffff0000, 0);
}

static void DebugTest(WView* view, const WVector& pos, ulong diffuse,
	float rlen)
{
	view->DrawLine(pos - WVector(rlen, 0.0f, 0.0f), diffuse,
		pos + WVector(rlen, 0.0f, 0.0f), diffuse, 0);
	view->DrawLine(pos - WVector(0.0f, rlen, 0.0f), diffuse,
		pos + WVector(0.0f, rlen, 0.0f), diffuse, 0);
	view->DrawLine(pos - WVector(0.0f, 0.0f, rlen), diffuse,
		pos + WVector(0.0f, 0.0f, rlen), diffuse, 0);
}

void WBone::Render(WView* view, const LightSet& light)
{
	WVector vWorLightDir, vLocLightDir, vLocEyePos;
	int i, k, n;
	float t, w;
	w_mesh* f;
	WVector vec, v;
	WTVertex* pv;
	int d0 = 0;

	if (IsVisible() && !m_flag.GetFlag(INVISIBLE) && !IsHidden())
	{
		vWorLightDir = light.nearOne * -1.0f;
		vLocLightDir = RotVec(vWorLightDir, ~GetMatrix());
		vLocEyePos = view->GetCamera().pivot * ~GetMatrix();

		for (f = m_mesh, n = 0; f != 0; f = f->next, ++n)
		{
			if (f->alpha == 0.0f ||
				(view->IsShadowView() && (f->drawFlag & 0x300000) == 0x100000))
				continue;
			if (view->ProcessEffect())
			{
				if (f->bEnv)
					CalcENVCoord(view, f);
				if (f->bSpec)
					CalcHilightCoord(view, light, f);
			}

			i = 0;
			if (f->UVBackup983() != 0)
			{
				k = 0;
				if (f->blendedVecList != 0)
				{
					for (; i < f->blendedRigidNum; ++i)
					{
						t = 0.0f;
						w = f->blendWeightList[k];
						vec = f->blendedBoneList[k]->Transform(
								  f->blendedVecList[k]) *
							w;
						t += w;
						for (++k; t < 0.999999f && k < f->blendedTotalNum; ++k)
						{
							WBone*& bone = f->blendedBoneList[k];
							w = f->blendWeightList[k];
							vec += bone->Transform(f->blendedVecList[k]) * w;
							t += w;
						}
						pv = GetVtxBuff(i);
						pv->tu = f->UVData983()[i][0];
						pv->tv = f->UVData983()[i][1];
						pv->lu = f->UVBackup983()[i][0];
						pv->lv = f->UVBackup983()[i][1];
						pv->diffuse = *f->vtxColorPtrList[i];
						pv->SetPosition(vec);
					}
				}
				for (; i < f->rigidNum; ++i)
				{
					pv = GetVtxBuff(i);
					pv->tu = f->UVData983()[i][0];
					pv->tv = f->UVData983()[i][1];
					pv->lu = f->UVBackup983()[i][0];
					pv->lv = f->UVBackup983()[i][1];
					pv->diffuse = *f->vtxColorPtrList[i];
					pv->SetPosition(f->boneList[i]->Transform(f->vecList[i]));
				}
				if (f->vtxNum - i > 0)
				{
					for (; i < f->vtxNum; ++i)
					{
						pv = GetVtxBuff(i);
						pv->diffuse = f->vtxColorList[i];
						pv->tu = f->UVData983()[i][0];
						pv->tv = f->UVData983()[i][1];
						pv->lu = f->UVBackup983()[i][0];
						pv->lv = f->UVBackup983()[i][1];
					}
					WTVertex* vertices;
					if (m_flag.GetFlag(PHYSICSMODEL))
					{
						for (k = f->rigidNum; k < f->vtxNum; ++k)
							DebugTest(view, m_physic->list[n][k], 0xffff0000,
								1.5999999f);
						vertices = GetVtxBuff();
						for (i = f->rigidNum; i < f->vtxNum; ++i)
						{
							vertices[i].SetPosition(m_physic->list[n][i]);
						}
					}
					else
					{
						vertices = GetVtxBuff();
						for (i = f->rigidNum; i < f->vtxNum; ++i)
						{
							vertices[i].SetPosition(f->vecList[i] * m_matrix);
						}
					}
				}
			}
			else
			{
				k = 0;
				if (f->blendedVecList != 0)
				{
					for (; i < f->blendedRigidNum; ++i)
					{
						t = 0.0f;
						w = f->blendWeightList[k];
						vec = f->blendedBoneList[k]->Transform(
								  f->blendedVecList[k]) *
							w;
						t += w;
						for (++k; t < 0.999999f && k < f->blendedTotalNum; ++k)
						{
							WBone*& bone = f->blendedBoneList[k];
							w = f->blendWeightList[k];
							vec += bone->Transform(f->blendedVecList[k]) * w;
							t += w;
						}
						pv = GetVtxBuff(i);
						pv->tu = f->UVData983()[i][0];
						pv->tv = f->UVData983()[i][1];
						pv->diffuse = *f->vtxColorPtrList[i];
						pv->SetPosition(vec);
					}
				}
				for (; i < f->rigidNum; ++i)
				{
					pv = GetVtxBuff(i);
					pv->tu = f->UVData983()[i][0];
					pv->tv = f->UVData983()[i][1];
					pv->diffuse = *f->vtxColorPtrList[i];
					pv->SetPosition(f->boneList[i]->Transform(f->vecList[i]));
				}
				if (f->vtxNum - i > 0)
				{
					for (; i < f->vtxNum; ++i)
					{
						pv = GetVtxBuff(i);
						pv->diffuse = f->vtxColorList[i];
						pv->tu = f->UVData983()[i][0];
						pv->tv = f->UVData983()[i][1];
					}
					WTVertex* vertices;
					if (m_flag.GetFlag(PHYSICSMODEL))
					{
						for (k = f->rigidNum; k < f->vtxNum; ++k)
							DebugTest(view, m_physic->list[n][k], 0xffff0000,
								1.5999999f);
						vertices = GetVtxBuff();
						for (k = f->rigidNum; k < f->vtxNum; ++k)
						{
							vertices[k].SetPosition(m_physic->list[n][k]);
						}
					}
					else
					{
						vertices = GetVtxBuff();
						for (i = f->rigidNum; i < f->vtxNum; ++i)
						{
							vertices[i].SetPosition(f->vecList[i] * m_matrix);
						}
					}
				}
			}

			if (!view->IsShadowView() && (f->xiDrawFlag2 & 0x200) != 0) // 983: dupla face 0x200
			{
				if ((f->xiDrawFlag2 & 0x14) == 0)
				{
					WVector pos, normal;
					WVector* normList;
					float lightDot, eyeDot;
					normList = f->myBlendedNormalList ? f->myBlendedNormalList
													  : f->blendedNormalList;
					for (i = k = 0; i < f->blendedRigidNum; ++i)
					{
						normal.Reset();
						pos.Reset();
						for (t = 0.0f; t < 0.999999f; ++k)
						{
							normal += RotVec(normList[k],
										  f->blendedBoneList[k]->GetMatrix()) *
								f->blendWeightList[k];
							pos += f->blendedBoneList[k]->Transform(
									   f->blendedVecList[k]) *
								f->blendWeightList[k];
							t = (float)(t + f->blendWeightList[k]);
						}
						eyeDot = (view->GetCamera().pivot - pos) * normal;
						lightDot = (float)(vWorLightDir * normal);
						if (lightDot * eyeDot < 0.0f)
						{
							WTVertex* vtx = GetVtxBuff(i);
							vtx->diffuse =
								(vtx->diffuse & 0xff000000) | light.ambient;
						}
					}
					normList = f->myNormalList ? f->myNormalList : f->normList;
					for (; i < f->rigidNum; ++i)
					{
						v = view->GetCamera().pivot *
							~f->boneList[i]->GetMatrix();
						eyeDot = (v - f->vecList[i]) * normList[i];
						lightDot = (float)(vWorLightDir *
							RotVec(normList[i], f->boneList[i]->GetMatrix()));
						if (lightDot * eyeDot < 0.0f)
						{
							WTVertex* vtx = GetVtxBuff(i);
							vtx->diffuse =
								(vtx->diffuse & 0xff000000) | light.ambient;
						}
					}
					for (; i < f->vtxNum; ++i)
					{
						eyeDot = (vLocEyePos - f->vecList[i]) * normList[i];
						lightDot = vLocLightDir * normList[i];
						if (lightDot * eyeDot < 0.0f)
						{
							WTVertex* vtx = GetVtxBuff(i);
							vtx->diffuse =
								(vtx->diffuse & 0xff000000) | light.ambient;
						}
					}
				}
			}

			// 983 00b9edc9: camadas de UV (mesh+0x50) e material de shader (mesh+0x80); so o CExhibitionBySoftwareTnL passa por aqui
			view->DrawIndexedTriangles(GetVtxBuff(), f->vtxNum, f->indexList,
				f->indexNum, f->drawFlag, f->xiDrawFlag2, f->uv ? (int)f->uv->size() : 0,
				f->xpShaderMtl983);
		}
	}

	if (m_next != 0)
		m_next->Render(view, light);
	if (m_child != 0)
		m_child->Render(view, light);
}


void WBone::RenderNormals(WView* view, float axisLen)
{
	WVector v, n;
	float w, t;
	ulong c;
	int i, k;
	WVector* normList;
	for (w_mesh* m = m_mesh; m; m = m->next)
	{
		i = 0;
		normList = m->myBlendedNormalList ? m->myBlendedNormalList
										  : m->blendedNormalList;
		if (m->blendedVecList && normList)
		{
			k = 0;
			for (; i < m->blendedRigidNum; i++)
			{
				c = WisEqual(normList[k], m->blendedNormalList[k], g_EPSILON)
					? 0xffff0000
					: 0xff00ff00;
				t = 0;
				w = m->blendWeightList[k];
				v = m->blendedBoneList[k]->Transform(m->blendedVecList[k]) * w;
				n = RotVec(normList[k], m->blendedBoneList[k]->GetMatrix()) * w;
				t += w;
				for (k++; t < 0.999999f && k < m->blendedTotalNum; k++)
				{
					w = m->blendWeightList[k];
					v +=
						m->blendedBoneList[k]->Transform(m->blendedVecList[k]) *
						w;
					n += RotVec(normList[k],
							 m->blendedBoneList[k]->GetMatrix()) *
						w;
					t += w;
				}
				n.Normalize();
				view->DrawLine(v, c, v + n * axisLen, c, 0x300000);
			}
		}
		normList = m->myNormalList ? m->myNormalList : m->normList;
		if (normList)
		{
			for (; i < m->rigidNum; i++)
			{
				c = WisEqual(normList[i], m->normList[i], g_EPSILON)
					? 0xffff0000
					: 0xff00ff00;
				v = m->boneList[i]->Transform(m->vecList[i]);
				n = RotVec(normList[i], m->boneList[i]->GetMatrix());
				n.Normalize();
				view->DrawLine(v, c, v + n * axisLen, c, 0x300000);
			}
			for (; i < m->vtxNum; i++)
			{
				c = WisEqual(normList[i], m->normList[i], g_EPSILON)
					? 0xffff0000
					: 0xff00ff00;
				v = m->vecList[i] * m_matrix;
				n = RotVec(normList[i], GetMatrix());
				n.Normalize();
				view->DrawLine(v, c, v + n * axisLen, c, 0x300000);
			}
		}
	}
	if (m_child)
		m_child->RenderNormals(view, axisLen);
	if (m_next)
		m_next->RenderNormals(view, axisLen);
}

void WBone::ApplyScale(float scale)
{
	m_localMat.pivot *= scale;
	m_basicMat.pivot *= scale;
	m_matrix.pivot *= scale;
	if (m_keyPos != 0)
	{
		for (int i = 0; i < m_keyPosNum; ++i)
		{
			WVector& pos = m_keyPos[i].pos;
			pos *= scale;
		}
	}
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		int i;
		for (i = 0; i < mesh->blendedTotalNum; ++i)
		{
			WVector& vec = mesh->blendedVecList[i];
			vec *= scale;
		}
		for (i = 0; i < mesh->vtxNum; ++i)
		{
			WVector& vec = mesh->vecList[i];
			vec *= scale;
		}
	}
	if (m_next != 0)
		m_next->ApplyScale(scale);
	if (m_child != 0)
		m_child->ApplyScale(scale);
}

void WBone::SetBoundBox(Waabb aabb, bool merge)
{
	Waabb bound;
	if (merge)
	{
		bound.min = WVector(Min(m_bound_aabb.min.x, aabb.min.x),
			Min(m_bound_aabb.min.y, aabb.min.y),
			Min(m_bound_aabb.min.z, aabb.min.z));
		bound.max = WVector(Max(m_bound_aabb.max.x, aabb.max.x),
			Max(m_bound_aabb.max.y, aabb.max.y),
			Max(m_bound_aabb.max.z, aabb.max.z));
	}
	else
		bound = aabb;
	m_bound_aabb = bound;
	m_bound_sphere = WSphere((bound.max + bound.min) * 0.5f,
		(float)ceil(WVectorLen(bound.max - bound.min) * 0.5f));
}

void WBone::FixNormal(const WMatrix& matrix)
{
	WMatrix world;
	world = m_localMat * matrix;
	WMatrix inverse;
	inverse = ~world;
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		for (int i = 0; i < mesh->vtxNum; ++i)
		{
			WVector& normal = mesh->normList[i];
			normal = WVector(inverse.xx * normal.x + inverse.xz * normal.z +
					inverse.xy * normal.y,
				inverse.yx * normal.x + inverse.yz * normal.z +
					inverse.yy * normal.y,
				inverse.zx * normal.x + inverse.zz * normal.z +
					inverse.zy * normal.y);
		}
	}
	if (m_next != 0)
		m_next->FixNormal(matrix);
	if (m_child != 0)
		m_child->FixNormal(world);
}

WBone* WBone::FindBone(const char* name, ulong hash)
{
	if (hash == 0)
		hash = GetHashCode(name);
	if (m_hashCode == hash && strcmp(name, m_name) == 0)
		return this;
	if (m_next != 0)
	{
		WBone* found = m_next->FindBone(name, hash);
		if (found != 0)
			return found;
	}
	if (m_child != 0)
	{
		WBone* found = m_child->FindBone(name, hash);
		if (found != 0)
			return found;
	}
	return 0;
}

WBone* WBone::FindBone(int id)
{
	if (m_id == id)
		return this;
	if (m_next != 0)
	{
		WBone* found = m_next->FindBone(id);
		if (found != 0)
			return found;
	}
	if (m_child != 0)
	{
		WBone* found = m_child->FindBone(id);
		if (found != 0)
			return found;
	}
	return 0;
}

WBone* WBone::FindBoneByTail(const char* tail)
{
	if (strstr(m_name, tail) != 0)
		return this;
	if (m_next != 0)
	{
		WBone* found = m_next->FindBoneByTail(tail);
		if (found != 0)
			return found;
	}
	if (m_child != 0)
	{
		WBone* found = m_child->FindBoneByTail(tail);
		if (found != 0)
			return found;
	}
	return 0;
}

WBone* WBone::FindFakeBodyBone(w_pet_vertex* list, int count)
{
	if (strstr(m_name, "Bip") == 0 && strstr(m_name, "Jaw") == 0)
	{
		int i;
		for (i = 0; i < count; ++i)
		{
			if (strcmp(list[i].bone983->m_name, m_name) == 0)
				break;
		}
		if (i == count && m_child == 0)
			return this;
	}
	if (m_next != 0)
	{
		WBone* found = m_next->FindFakeBodyBone(list, count);
		if (found != 0)
			return found;
	}
	if (m_child != 0)
	{
		WBone* found = m_child->FindFakeBodyBone(list, count);
		if (found != 0)
			return found;
	}
	return 0;
}

int WBone::GetConvexArea(WBlockModel* model)
{
	int i;
	w_mesh* mesh;
	int total = 0;
	for (mesh = m_mesh; mesh != 0; mesh = mesh->next)
		total += mesh->indexNum;
	WVector* list = new WVector[total];
	int count = 0;
	for (mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		for (i = 0; i < mesh->indexNum; ++i)
		{
			list[count++] = mesh->vecList[mesh->indexList[i]] * GetMatrix();
		}
	}
	model->AddTriangleList(list, total);
	delete list;
	return 1;
}

void WBone::ApplySpecularMap(int handle)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if (mesh->bSpec == false)
		{
			mesh->bSpec = true;
			// 983: camada 1 das UVs (era o uvBackup)
			if (mesh->uv == 0)
				mesh->uv = new w_uvset983(1, w_uvlayer983(mesh->vtxNum));
			mesh->uv->resize(2, w_uvlayer983(mesh->vtxNum));
			mesh->texHandle |= (handle & 0x7f) << 11;
			mesh->drawFlag = (mesh->drawFlag & 0xfffff800) | mesh->texHandle;
		}
	}
	if (m_next != 0)
		m_next->ApplySpecularMap(handle);
	if (m_child != 0)
		m_child->ApplySpecularMap(handle);
}

int __fastcall WriteID(int id, FILE* file)
{
	if (id >= 0xfe)
	{
		if (file != 0)
		{
			fputc(0xfe, file);
			fwrite(&id, 2, 1, file);
		}
		return 3;
	}
	if (file != 0)
		fputc(id, file);
	return 1;
}

int WBone::SaveBoneName(FILE* file, int type, char* name)
{
	int written = 0;
	bool skip = name != 0 && m_mesh != 0 && stricmp(name, m_name) != 0;
	if (file != 0)
	{
		if (!skip)
		{
			if (!((type == 0x82 || type == 0x66) && m_mesh != 0))
			{
				fwrite(m_name, 1, strlen(m_name) + 1, file);
				WriteID(m_parent != 0 ? m_parent->m_id : -1, file);
			}
			if (type == 0xff || (type == 0x82 && m_mesh == 0) ||
				type == 0x11b || type == 6)
			{
				WMatrix matrix;
				matrix = m_basicMat;
				if (type == 6)
				{
					for (WBone* bone = m_parent; bone != 0;
						bone = bone->m_parent)
						matrix = matrix * bone->m_basicMat;
					matrix.xm *= 3.125f;
					matrix.ym *= 3.125f;
					matrix.zm *= 3.125f;
				}
				fwrite(&matrix.xx, 4, 1, file);
				fwrite(&matrix.yx, 4, 1, file);
				fwrite(&matrix.zx, 4, 1, file);
				fwrite(&matrix.xy, 4, 1, file);
				fwrite(&matrix.yy, 4, 1, file);
				fwrite(&matrix.zy, 4, 1, file);
				fwrite(&matrix.xz, 4, 1, file);
				fwrite(&matrix.yz, 4, 1, file);
				fwrite(&matrix.zz, 4, 1, file);
				fwrite(&matrix.xm, 4, 1, file);
				fwrite(&matrix.ym, 4, 1, file);
				fwrite(&matrix.zm, 4, 1, file);
			}
		}
	}
	else
	{
		if (!skip)
		{
			if (!((type == 0x82 || type == 0x66) && m_mesh != 0))
			{
				written = strlen(m_name) +
					WriteID(m_parent != 0 ? m_parent->GetID() : -1, 0) + 1;
			}
			if (type == 0xff || (type == 0x82 && m_mesh == 0) ||
				type == 0x11b || type == 6)
				written += 48;
		}
	}
	if (m_child != 0)
		written += m_child->SaveBoneName(file, type, name);
	if (m_next != 0)
		written += m_next->SaveBoneName(file, type, name);
	return written;
}

int WBone::SaveAnimationKey(FILE* file, int id, float (*range)[2], int count)
{
	int written = 0;
	w_keyframe_pos* posKeys = 0;
	w_keyframe_rot* rotKeys = 0;
	int posNum, rotNum, i;
	float zero;
	if (m_keyPosNum != 0 || m_keyRotNum != 0)
	{
		if (m_keyPosNum > 0)
		{
			posNum = OptimizePosKey(range, count, &posKeys);
			if (m_keyPosNum == posNum)
				posKeys = m_keyPos;
		}
		else
		{
			posNum = 0;
		}
		if (m_keyRotNum > 0)
		{
			rotNum = OptimizeRotKey(range, count, &rotKeys);
			if (m_keyRotNum == rotNum)
				rotKeys = m_keyRot;
		}
		else
		{
			rotNum = 0;
		}
		if (file != 0)
		{
			WriteID(m_id, file);
			fwrite(&posNum, 4, 1, file);
			for (i = 0; i < posNum; ++i)
			{
				fwrite(&posKeys[i].time, 4, 1, file);
				fwrite(&posKeys[i].pos.x, 4, 1, file);
				fwrite(&posKeys[i].pos.y, 4, 1, file);
				fwrite(&posKeys[i].pos.z, 4, 1, file);
			}
			fwrite(&rotNum, 4, 1, file);
			w_keyframe_rot* rot = rotKeys;
			for (i = 0; i < rotNum; ++i)
			{
				fwrite(&rot[i].time, 4, 1, file);
				fwrite(&rot[i].rot.x, 4, 1, file);
				fwrite(&rot[i].rot.y, 4, 1, file);
				fwrite(&rot[i].rot.z, 4, 1, file);
				fwrite(&rot[i].rot.w, 4, 1, file);
			}
			zero = 0.0f;
			fwrite(&zero, 4, 1, file);
		}
		written = WriteID(m_id, 0) + posNum * 16 + rotNum * 20 + 12;
		if (m_keyPosNum > 0 && m_keyPos != posKeys)
			delete[] posKeys;
		if (m_keyRotNum > 0 && m_keyRot != rotKeys)
			delete[] rotKeys;
	}
	if (m_child != 0)
		written += m_child->SaveAnimationKey(file, id, range, count);
	if (m_next != 0)
		written += m_next->SaveAnimationKey(file, id, range, count);
	return written;
}

int WBone::SetID(int id, bool flag, char* name)
{
	if ((flag || m_mesh == 0) &&
		(name == 0 || m_mesh == 0 || stricmp(name, m_name) == 0))
	{
		m_id = id;
		++id;
	}
	if (m_child != 0)
		id = m_child->SetID(id, flag, name);
	if (m_next != 0)
		id = m_next->SetID(id, flag, name);
	return id;
}

void WBone::CountBones(int& count)
{
	WBone* bone = this;
	do
	{
		++count;
		WBone* child = bone->m_child;
		if (child != 0)
			child->CountBones(count);
		bone = bone->m_next;
	} while (bone != 0);
}

float WBone::LoadAnimationKey(cFile* fp, unsigned long petVersion983)
{
	float t = 0.0f;
	int i;
	WQuat rot;
	fp->Read(&m_keyPosNum, 4);
	if (m_keyPosNum != 0)
	{
		m_keyPos = new w_keyframe_pos[m_keyPosNum];
		m_keyFlag |= 2;
		for (i = 0; i < m_keyPosNum; ++i)
		{
			fp->Read(&m_keyPos[i].time, 4);
			fp->Read(&m_keyPos[i].pos.x, 4);
			fp->Read(&m_keyPos[i].pos.y, 4);
			fp->Read(&m_keyPos[i].pos.z, 4);
			if (t < m_keyPos[i].time)
				t = m_keyPos[i].time;
		}
	}
	fp->Read(&m_keyRotNum, 4);
	if (m_keyRotNum != 0)
	{
		m_keyFlag |= 1;
		m_keyRot = new w_keyframe_rot[m_keyRotNum];
		for (i = 0; i < m_keyRotNum; ++i)
		{
			fp->Read(&m_keyRot[i].time, 4);
			fp->Read(&rot.x, 4);
			fp->Read(&rot.y, 4);
			fp->Read(&rot.z, 4);
			fp->Read(&rot.w, 4);
			rot.Normalize();
			m_keyRot[i].rot = rot;
			if (t < m_keyRot[i].time)
				t = m_keyRot[i].time;
		}
	}
	// 983 (00b97010): chaves de escala (o 645 lia aqui um contador e o descartava) e, a partir do .pet 0xfffe0103, chaves de FOV
	fp->Read(&m_keyScaleNum, 4);
	if (m_keyScaleNum != 0)
	{
		m_keyScale = new w_keyframe_pos[m_keyScaleNum];
		m_keyFlag |= 4;
		for (i = 0; i < m_keyScaleNum; ++i)
		{
			fp->Read(&m_keyScale[i].time, 4);
			fp->Read(&m_keyScale[i].pos.x, 4);
			fp->Read(&m_keyScale[i].pos.y, 4);
			fp->Read(&m_keyScale[i].pos.z, 4);
			if (t < m_keyScale[i].time)
				t = m_keyScale[i].time;
		}
	}
	if (ComparePetVersion983(petVersion983, PET_VERSION_FOVKEYS983) >= 0)
	{
		fp->Read(&m_key983_f0, 4);
		if (m_key983_f0 != 0)
		{
			m_key983_ec = new w_keyframe_fov983[m_key983_f0];
			for (i = 0; i < m_key983_f0; ++i)
			{
				fp->Read(&m_key983_ec[i].time, 4);
				fp->Read(&m_key983_ec[i].fov, 4);
				if (t < m_key983_ec[i].time)
					t = m_key983_ec[i].time;
			}
		}
	}
	return t;
}

void WBone::GetName(char** buffer, int& count)
{
	if (m_parent != 0)
		memcpy(*buffer, m_parent->m_name, strlen(m_parent->m_name));
	*buffer += 0x1e;
	memcpy(*buffer, m_name, strlen(m_name));
	*buffer += 0x1e;
	++count;
	if (m_next != 0)
		m_next->GetName(buffer, count);
	if (m_child != 0)
		m_child->GetName(buffer, count);
}

void WBone::GetBoneInfo(int* totals)
{
	WBone* bone = this;
	for (;;)
	{
		for (w_mesh* mesh = bone->m_mesh; mesh != 0; mesh = mesh->next)
		{
			totals[1] += mesh->vtxNum;
			totals[2] += mesh->indexNum / 3;
		}
		++totals[0];
		if (bone->m_next != 0)
			bone->m_next->GetBoneInfo(totals);
		WBone* child = bone->m_child;
		if (child == 0)
			break;
		bone = child;
	}
}

int WBone::GetOverlapBox(float* box, WBone* bone)
{
	for (w_mesh* mesh = bone->m_mesh; mesh != 0; mesh = mesh->next)
	{
		for (int i = 0; i < mesh->indexNum; i += 3)
		{
			for (int k = 0; k < 3; ++k)
			{
				int index = mesh->indexList[i + k];
				WVector point;
				if (bone != this)
				{
					if (mesh->indexList[i] >= mesh->rigidNum)
						continue;
					if (mesh->boneList[index] != this)
						continue;
				}
				if (mesh->indexList[i] < mesh->rigidNum)
					point = mesh->vecList[index] *
						mesh->boneList[index]->GetMatrix() * ~GetMatrix();
				else
					point = mesh->vecList[index];
				box[0] = box[0] < point.x ? box[0] : point.x;
				box[1] = box[1] > point.x ? box[1] : point.x;
				box[2] = box[2] < point.y ? box[2] : point.y;
				box[3] = box[3] > point.y ? box[3] : point.y;
				box[4] = box[4] < point.z ? box[4] : point.z;
				box[5] = box[5] > point.z ? box[5] : point.z;
			}
		}
	}
	if (bone->m_next != 0)
		GetOverlapBox(box, bone->m_next);
	if (bone->m_child != 0)
		GetOverlapBox(box, bone->m_child);
	return 1;
}

void WBone::GetTotalOverlapBox(WVector* min, WVector* max, WMatrix* matrix,
	bool visibleOnly)
{
	WVector v;
	int i;
	w_mesh* mesh;

	if (matrix)
	{
		for (mesh = m_mesh; mesh; mesh = mesh->next)
		{
			if (visibleOnly && !IsMeshVisible(this, mesh))
				continue;

			for (i = 0; i < mesh->vtxNum; ++i)
			{
				if (i < mesh->rigidNum)
					v = mesh->boneList[i]->Transform(mesh->vecList[i]);
				else
					v = Transform(mesh->vecList[i]);

				v = v * *matrix;

				min->x = min->x < v.x ? min->x : v.x;
				max->x = max->x > v.x ? max->x : v.x;
				min->y = min->y < v.y ? min->y : v.y;
				max->y = max->y > v.y ? max->y : v.y;
				min->z = min->z < v.z ? min->z : v.z;
				max->z = max->z > v.z ? max->z : v.z;
			}
		}
	}
	else
	{
		for (mesh = m_mesh; mesh; mesh = mesh->next)
		{
			if (visibleOnly && !IsMeshVisible(this, mesh))
				continue;

			for (i = 0; i < mesh->vtxNum; ++i)
			{
				if (i < mesh->rigidNum)
					v = mesh->boneList[i]->Transform(mesh->vecList[i]);
				else
					v = Transform(mesh->vecList[i]);

				min->x = min->x < v.x ? min->x : v.x;
				max->x = max->x > v.x ? max->x : v.x;
				min->y = min->y < v.y ? min->y : v.y;
				max->y = max->y > v.y ? max->y : v.y;
				min->z = min->z < v.z ? min->z : v.z;
				max->z = max->z > v.z ? max->z : v.z;
			}
		}
	}

	if (m_next)
		m_next->GetTotalOverlapBox(min, max, matrix, visibleOnly);

	if (m_child)
		m_child->GetTotalOverlapBox(min, max, matrix, visibleOnly);
}


void WBone::Rotate(float angle, int axis, bool reset)
{
	if (reset)
		m_rotateMat.Reset();
	m_rotateMat.Rotate(angle, (char)axis);
	m_flag.Turn(ROTATEBONE, !reset || !WisZero(angle, g_EPSILON));
}

void WBone::Move(WVector& vec, bool child)
{
	m_moveVec = vec;
	m_flag.Enable(0x400);
	if (child)
	{
		if (m_next != 0)
			m_next->Move(vec, child);
		if (m_child != 0)
			m_child->Move(vec, child);
	}
}

void WBone::SetSelfIllum(bool enabled, bool recursive)
{
	WBone* bone = this;
	do
	{
		if (enabled == true)
			bone->m_flag.Enable(2);
		else
			bone->m_flag.Disable(2);
		if (!recursive)
			return;
		WBone* next = bone->m_next;
		if (next != 0)
			next->SetSelfIllum(enabled, recursive);
		bone = bone->m_child;
	} while (bone != 0);
}

void WBone::SetSelfIllumColor(ulong color, bool recursive)
{
	WBone* bone = this;
	bone->m_selfillumColor = color & 0xffffff;
	if (recursive)
	{
		for (;;)
		{
			WBone* next = bone->m_next;
			if (next != 0)
				next->SetSelfIllumColor(color, recursive);
			bone = bone->m_child;
			if (bone == 0)
				break;
			bone->m_selfillumColor = color & 0xffffff;
		}
	}
}

void WBone::SetInvisible(bool recursive, bool invisible)
{
	WBone* bone = this;
	do
	{
		if (invisible)
			bone->m_flag.Enable(0x40000);
		else
			bone->m_flag.Disable(0x40000);
		if (!recursive)
			return;
		WBone* next = bone->m_next;
		if (next != 0)
			next->SetInvisible(recursive, invisible);
		bone = bone->m_child;
	} while (bone != 0);
}

void WBone::HideBone(bool recursive)
{
	WBone* bone = this;
	do
	{
		bone->m_flag.Enable(1);
		if (!recursive)
			return;
		WBone* next = bone->m_next;
		if (next != 0)
			next->HideBone(recursive);
		bone = bone->m_child;
	} while (bone != 0);
}

void WBone::ShowBone(bool recursive)
{
	WBone* bone = this;
	do
	{
		bone->m_flag.Disable(1);
		if (!recursive)
			return;
		WBone* next = bone->m_next;
		if (next != 0)
			next->ShowBone(recursive);
		bone = bone->m_child;
	} while (bone != 0);
}

void WBone::AllocVTX(int count)
{
	if (m_vtx_len < count)
	{
		m_vtx_len = count;
		if (m_vtxList != 0)
			delete[] m_vtxList;
		m_vtxList = new WTVertex[m_vtx_len];
	}
}

void WBone::CountVtxBuff(int count)
{
	m_vtx_count += count;
	if (m_vtx_count == 0)
	{
		if (m_vtxList != 0)
		{
			delete[] m_vtxList;
			m_vtxList = 0;
			m_vtx_len = 0;
		}
	}
}

void WBone::SetRigidVtxFlag(void)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if (mesh->rigidNum != 0 || mesh->blendedRigidNum != 0)
		{
			m_flag.Enable(4);
			break;
		}
	}
	if (m_next != 0)
		m_next->SetRigidVtxFlag();
	if (m_child != 0)
		m_child->SetRigidVtxFlag();
}

bool WBone::CompareBasicMatrix(WBone* other)
{
	if (((((ulong)m_flag) >> 16) & 1) == 0)
	{
		WBone* found = other->FindBone(m_name, 0);
		if (found == 0)
			return false;
		if (m_basicMat != found->m_basicMat)
			return false;
	}
	if (m_child != 0 && !m_child->CompareBasicMatrix(other))
		return false;
	if (m_next != 0 && !m_next->CompareBasicMatrix(other))
		return false;
	return true;
}

void WBone::xSetAlpha(unsigned char alpha, int recurse)
{
	m_alpha = alpha;
	if (alpha == 0)
		m_flag.Enable(1);
	if ((((ulong)m_flag) & 1) != 0 && alpha > 0)
		m_flag.Disable(1);
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		ulong value = (ulong)(int)((float)alpha * mesh->alpha);
		if ((value & 0xff) == 0xff)
		{
			int flag = mesh->drawFlag;
			if ((flag & 0x1800000) == 0 && (mesh->flags & 0x400000) == 0)
				flag &= 0xffbfffff;
			else
				flag |= 0x400000;
			mesh->drawFlag = flag;
		}
		else
		{
			// 983 00b97ae0: só 0x400000, como o SetAlpha
			mesh->drawFlag |= 0x400000;
		}
	}
	if (recurse != 0)
	{
		if (m_next != 0)
			m_next->xSetAlpha(alpha, 1);
		if (m_child != 0)
			m_child->xSetAlpha(alpha, 1);
	}
}

void WBone::xRender(WView* view, WxBatchState* state, bool setMatrix,
	int handle, int newHandle)
{
	bool visible = IsVisible();
	bool mirror = m_flag.GetFlag(INVISIBLE);
	bool hidden = IsHidden();
	if (visible && !mirror && !hidden)
	{
		for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
		{
			if (mesh->xpBatch == 0)
				continue;
			state->xpShaderMtl = mesh->xpShaderMtl983; // 983 00b97bc0: *state = mesh+0x80 (A328)
			state->xiBaseVtxIdx = mesh->xpBatch->xiBaseVtxIdx;
			state->xiBaseIdxIdx = mesh->xpBatch->xiBaseIdxIdx;
			state->xnVtxs = mesh->vtxNum;
			state->xnIdxs = mesh->indexNum;
			if (setMatrix)
			{
				state->xmW = m_matrix;
				state->xnmTransfs = 0;
			}
			int draw = mesh->drawFlag;
			if ((draw & 0x7ff) == handle && handle != newHandle)
				state->xiFlag0 = draw ^ ((draw ^ newHandle) & 0x7ff);
			else
				state->xiFlag0 = draw;
			int flag2 = mesh->xiDrawFlag2;
			state->xiFlag1 = flag2;
			if ((flag2 & 4) != 0)
				state->xdwDiffuse =
					((ulong)m_alpha << 24) | view->xGetLight()->ambient2;
			else
				state->xdwDiffuse = ((ulong)m_alpha << 24) | 0xffffff;
			view->xDrawIndexedTriangles(*state);
		}
	}
	if (m_next != 0)
		m_next->xRender(view, state, setMatrix, handle, newHandle);
	if (m_child != 0)
		m_child->xRender(view, state, setMatrix, handle, newHandle);
}

void WBone::ReplaceKeyframe(WBone* bone, float time)
{
	WMatrix other, mine;
	mine = m_localMat;
	other = bone->m_localMat;

	m_keyFlag = 0;
	if (m_keyRot != 0)
	{
		delete[] m_keyRot;
		m_keyRot = 0;
	}
	m_keyRotNum = bone->m_keyRotNum;
	if (m_keyRotNum != 0)
	{
		m_keyRot = new w_keyframe_rot[m_keyRotNum];
		memcpy(m_keyRot, bone->m_keyRot, m_keyRotNum * 20);
		m_keyFlag |= 1;
		m_flag.Disable(0x40);
	}
	else
	{
		WQuat qSrc, qDst;
		qSrc = other;
		qDst = mine;
		if (WisZero(Wabs(qSrc.x - qDst.x), 1e-5f) == 0 ||
			WisZero(Wabs(qSrc.y - qDst.y), 1e-5f) == 0 ||
			WisZero(Wabs(qSrc.z - qDst.z), 1e-5f) == 0 ||
			WisZero(Wabs(qSrc.w - qDst.w), 1e-5f) == 0)
		{
			m_keyRotNum = 2;
			m_keyRot = new w_keyframe_rot[2];
			m_keyRot[0].time = 0;
			m_keyRot[0].rot = qSrc;
			m_keyRot[1].time = time;
			m_keyRot[1].rot = qSrc;
			m_keyFlag |= 1;
			m_flag.Disable(0x40);
		}
	}

	if (m_keyPos != 0)
	{
		delete[] m_keyPos;
		m_keyPos = 0;
	}
	m_keyPosNum = bone->m_keyPosNum;
	if (m_keyPosNum != 0)
	{
		m_keyPos = new w_keyframe_pos[m_keyPosNum];
		memcpy(m_keyPos, bone->m_keyPos, m_keyPosNum * 16);
	}
	else
	{
		WVector vSrc = other.pivot;
		WVector vDst = mine.pivot;
		if (WisEqual(vSrc, vDst, 1e-5f) != 0)
			return;
		m_keyPosNum = 2;
		m_keyPos = new w_keyframe_pos[2];
		m_keyPos[0].time = 0;
		m_keyPos[0].pos = vSrc;
		m_keyPos[1].time = time;
		m_keyPos[1].pos = vSrc;
	}
	m_keyFlag |= 2;
	m_flag.Disable(0x40);
}

WBone* WBone::AttachMeshBone(WBone* bone, int handle, int oldHandle)
{
	m_next = new WBone;
	m_next->SetName(bone->m_name);
	m_next->m_id = 0;
	m_next->SetBasicMatrix(bone->m_basicMat);

	w_mesh* mesh = bone->m_mesh;
	w_mesh** tail = &m_next->m_mesh;
	while (mesh != 0)
	{
		if ((((ulong)m_next->m_flag) & 0x10020) == 0)
			m_next->m_flag.Enable(0x10020);
		*tail = m_next->CopyMesh(mesh);
		if (*tail != 0)
		{
			if (handle != oldHandle && ((*tail)->drawFlag & 0x7ff) == oldHandle)
				(*tail)->drawFlag = ((*tail)->drawFlag & ~0x7ff) | handle;
			bool rigid = ((((ulong)m_next->m_flag) >> 2) & 1) != 0;
			if (rigid == false)
			{
				if ((*tail)->rigidNum != 0 || (*tail)->blendedRigidNum != 0)
					m_next->m_flag.Enable(4);
			}
		}
		mesh = mesh->next;
		if (*tail != 0)
			tail = &(*tail)->next;
	}
	return m_next;
}

void WBone::ChangeTextureByHandle(int handle, int oldHandle, bool next,
	bool child)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if (mesh->originalTexHandle == oldHandle)
			mesh->drawFlag = (mesh->drawFlag & 0xfffff800) | handle;
		// 983 (00b97db0 -> 00bb4a70): tambem troca a textura do material de shader da camada
		if (mesh->xpShaderMtl983 != 0)
		{
			WxShaderMtl::tTextureByName& textures = mesh->xpShaderMtl983->m_textures;
			for (WxShaderMtl::tTextureByName::iterator it = textures.begin();
				it != textures.end(); ++it)
			{
				if (it->second.m_hOrgTex == oldHandle)
					it->second.m_value = handle;
			}
		}
	}
	if (next && m_next != 0)
		m_next->ChangeTextureByHandle(handle, oldHandle, next, child);
	if (child && m_child != 0)
		m_child->ChangeTextureByHandle(handle, oldHandle, next, child);
}

bool WBone::ChangeTexture(const char* name, int oldHandle, int handle)
{
	if (name == 0 || oldHandle == 0 || handle == 0)
		return false;
	{
		if (m_hashCode == GetHashCode(name) && strcmp(name, m_name) == 0)
		{
			for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
			{
				if (mesh->originalTexHandle == oldHandle)
					mesh->drawFlag = (mesh->drawFlag & 0xfffff800) | handle;
			}
			return true;
		}
	}
	if (m_next != 0 && m_next->ChangeTexture(name, oldHandle, handle))
		return true;
	if (m_child != 0 && m_child->ChangeTexture(name, oldHandle, handle))
		return true;
	return false;
}

bool WBone::ChangeTexture(const char* name, int handle)
{
	if (name == 0 || handle == 0)
		return false;
	{
		if (m_hashCode == GetHashCode(name) && strcmp(name, m_name) == 0)
		{
			for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
			{
				if (mesh->originalTexHandle != 0)
					mesh->drawFlag = (mesh->drawFlag & 0xfffff800) | handle;
			}
			return true;
		}
	}
	if (m_next != 0 && m_next->ChangeTexture(name, handle))
		return true;
	if (m_child != 0 && m_child->ChangeTexture(name, handle))
		return true;
	return false;
}

void WBone::ResetMyNormalList(void)
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if (mesh->myNormalList != 0)
			memcpy(mesh->myNormalList, mesh->normList, mesh->vtxNum * 12);
		if (mesh->myBlendedNormalList != 0)
			memcpy(mesh->myBlendedNormalList, mesh->blendedNormalList,
				mesh->blendedTotalNum * 12);
	}
}

// 983 00bad230
void WBone::GatherNormalMergeInfo(w_normalmerge& merge) const
{
	for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
	{
		if (mesh->weightList == 0)
			continue;
		for (int i = 0; i < mesh->vtxNum; ++i)
		{
			if (mesh->weightList[i] != 1.0f)
				merge.AddVertex(this, mesh, i);
		}
	}
}


void WBone::SetMirrored(bool mirrored)
{
	WBone* bone = this;
	for (;;)
	{
		if (mirrored)
			bone->m_flag.Enable(0x20000);
		else
			bone->m_flag.Disable(0x20000);
		for (w_mesh* mesh = bone->m_mesh; mesh != 0; mesh = mesh->next)
		{
			mesh->xiDrawFlag2 &= 0xfffff9ffUL; // 983 00b98040: bits 0x200/0x400 (eram 0x400/0x800)
			int draw = mesh->xiDrawFlag2;
			if (mirrored)
				draw |= 0x400;
			mesh->xiDrawFlag2 = draw;
		}
		if (bone->m_next != 0)
			bone->m_next->SetMirrored(mirrored);
		WBone* child = bone->m_child;
		if (child == 0)
			break;
		bone = child;
	}
}

void WBone::SetScale(float scale)
{
	m_bonescale = scale;
	m_flag.Enable(0x80000);
}

void __fastcall WBone::CalcMeshAABB(WBone& bone, w_mesh& mesh)
{
	int i;
	if (mesh.vtxNum <= 0)
		return;
	if (mesh.rigidNum == 0)
	{
		mesh.aabbBone = &bone;
	}
	else
	{
		std::map<WBone*, float> counts;
		for (i = 0; i < mesh.rigidNum; ++i)
		{
			std::map<WBone*, float>::iterator found;
			found = counts.find(mesh.boneList[i]);
			if (found == counts.end())
				counts.insert(
					std::pair<WBone* const, float>(mesh.boneList[i], 1.0f));
			else
				(*found).second = (*found).second + 1.0f;
		}
		if (i < mesh.vtxNum)
		{
			counts.insert(std::pair<WBone* const, float>(&bone, 1.0f));
			for (; i < mesh.vtxNum; ++i)
				counts[&bone] += 1.0f;
		}
		if (counts.size() > 0)
		{
			std::map<WBone*, float>::const_iterator best, scan;
			best = counts.begin();
			scan = best;
			for (scan++; scan != counts.end(); scan++)
			{
				if ((*scan).second > (*best).second)
					best = scan;
			}
			mesh.aabbBone = (*best).first;
		}
	}

	WMatrix inverse = ~mesh.aabbBone->m_matrix;
	WBone* prevBone = 0;
	WMatrix m;
	WVector v;
	mesh.localAabb.Clear();

	i = 0;
	for (; i < mesh.rigidNum; ++i)
	{
		if (mesh.boneList[i] != mesh.aabbBone)
		{
			if (mesh.boneList[i] != prevBone)
			{
				prevBone = mesh.boneList[i];
				m = mesh.boneList[i]->GetMatrix() * inverse;
			}
			v = mesh.vecList[i] * m;
		}
		else
			v = mesh.vecList[i];
		mesh.localAabb.AddPoint(v);
	}
	if (&bone != mesh.aabbBone)
		m = bone.m_matrix * inverse;
	for (; i < mesh.vtxNum; ++i)
	{
		if (&bone != mesh.aabbBone)
			v = mesh.vecList[i] * m;
		else
			v = mesh.vecList[i];
		mesh.localAabb.AddPoint(v);
	}
}

bool __fastcall WBone::IsMeshVisible(WBone* bone, w_mesh* mesh)
{
	return bone->IsVisible() && !bone->m_flag.GetFlag(INVISIBLE) &&
		!bone->IsHidden() && mesh->alpha > 0.0f;
}

bool WBone::CheckMeshBoneAllVertexRigid(void) const
{
	if (((((ulong)m_flag) >> 16) & 1) != 0)
	{
		for (w_mesh* mesh = m_mesh; mesh != 0; mesh = mesh->next)
		{
			if (mesh->rigidNum < mesh->vtxNum)
				return false;
		}
	}
	if (m_next != 0 && !m_next->CheckMeshBoneAllVertexRigid())
		return false;
	if (m_child != 0 && !m_child->CheckMeshBoneAllVertexRigid())
		return false;
	return true;
}

WBone::w_normalmerge::w_normalmerge(WPuppet& puppet)
{
	WBone* bone;
	for (bone = puppet.FindMeshBone(0); bone != 0;
		bone = puppet.FindMeshBone(bone))
		bone->ResetMyNormalList();
	for (bone = puppet.FindMeshBone(0); bone != 0;
		bone = puppet.FindMeshBone(bone))
		bone->GatherNormalMergeInfo(*this);
	MergeNormal();
}

// 983 00bacfe0: a malha entra no mapa do ponto junto com o osso dono dela
void WBone::w_normalmerge::AddVertex(const WBone* bone, w_mesh* mesh, int index)
{
	std::vector<P>& plist = m_wlist[mesh->weightList[index]];
	P* p = Find(plist, mesh->vecList[index]);
	if (p == 0)
	{
		plist.push_back(P(mesh->vecList[index]));
		p = &*plist.rbegin();
	}
	typedef std::pair<const WBone*, std::vector<N> > BoneNormals;
	std::map<w_mesh*, BoneNormals>::iterator m = p->m_meshList.find(mesh);
	if (m == p->m_meshList.end())
	{
		std::vector<N> empty;
		m = p->m_meshList.insert(
			std::pair<w_mesh*, BoneNormals>(mesh, BoneNormals(bone, empty))).first;
	}
	std::vector<N>& nlist = (*m).second.second;
	N* n = Find(nlist, mesh->normList[index]);
	if (n == 0)
	{
		nlist.push_back(N(mesh->normList[index]));
		n = &*nlist.rbegin();
	}
	n->m_idxList.push_back(index);
}

// 983 00ba5460: junta as normais de malhas de ossos diferentes (645: de bonecos diferentes, pelo mpetName)
void WBone::w_normalmerge::MergeNormal(void)
{
	std::map<float, std::vector<P> >::const_iterator w;
	for (w = m_wlist.begin(); w != m_wlist.end(); w++)
	{
		std::vector<P>::const_iterator p;
		for (p = (*w).second.begin(); p != (*w).second.end(); p++)
		{
			std::map<w_mesh*, std::pair<const WBone*, std::vector<N> > >::const_iterator m;
			for (m = (*p).m_meshList.begin(); m != (*p).m_meshList.end(); m++)
			{
				w_mesh* mesh = (*m).first;
				if (mesh->myNormalList == 0)
					continue;
				std::vector<N>::const_iterator n;
				for (n = (*m).second.second.begin(); n != (*m).second.second.end(); n++)
				{
					int index = *(*n).m_idxList.begin();
					bool merged = false;
					std::map<w_mesh*, std::pair<const WBone*, std::vector<N> > >::const_iterator other =
						m;
					for (other++; other != (*p).m_meshList.end(); other++)
					{
						w_mesh* mesh2 = (*other).first;
						if (mesh2->myNormalList == 0)
							continue;
						if ((*m).second.first == (*other).second.first)
							continue;
						std::vector<N>::const_iterator n2;
						for (n2 = (*other).second.second.begin();
							n2 != (*other).second.second.end(); n2++)
						{
							merged = true;
							int index2 = *(*n2).m_idxList.begin();
							mesh->myNormalList[index] +=
								mesh2->normList[index2];
							mesh2->myNormalList[index2] += (*n).m_n;
							SetMyNormalsEqualToFirstNormal(mesh2,
								(*n2).m_idxList);
						}
					}
					if (merged)
						SetMyNormalsEqualToFirstNormal(mesh, (*n).m_idxList);
				}
			}
		}
	}
	NormalizeMergedNormals();
	ApplyToBlendedNormalList();
}

void WBone::w_normalmerge::NormalizeMergedNormals(void)
{
	std::map<float, std::vector<P> >::const_iterator w;
	std::vector<P>::const_iterator p;
	std::map<w_mesh*, std::pair<const WBone*, std::vector<N> > >::const_iterator m;
	std::vector<N>::const_iterator n;

	for (w = m_wlist.begin(); w != m_wlist.end(); w++)
	{
		for (p = (*w).second.begin(); p != (*w).second.end(); p++)
		{
			for (m = (*p).m_meshList.begin(); m != (*p).m_meshList.end(); m++)
			{
				w_mesh* mesh = (*m).first;
				if ((*m).first->myNormalList)
				{
					for (n = (*m).second.second.begin(); n != (*m).second.second.end(); n++)
					{
						for (std::vector<int>::const_iterator i =
								 (*n).m_idxList.begin();
							i != (*n).m_idxList.end(); i++)
						{
							if (WisEqual(mesh->myNormalList[*i],
									mesh->normList[*i], 1e-05f) == 0)
								mesh->myNormalList[*i].Normalize();
						}
					}
				}
			}
		}
	}
}

// 983 00ba4b90: as matrizes originais saem da paleta de ossos da malha (645: lista do boneco pelo mpetName)
void WBone::w_normalmerge::ApplyToBlendedNormalList(void)
{
	w_mesh* mesh;
	int slot, i;
	float weight;
	std::list<w_mesh*> meshList;

	std::map<float, std::vector<P> >::const_iterator w;
	for (w = m_wlist.begin(); w != m_wlist.end(); w++)
	{
		std::vector<P>::const_iterator p;
		for (p = (*w).second.begin(); p != (*w).second.end(); p++)
		{
			std::map<w_mesh*, std::pair<const WBone*, std::vector<N> > >::const_iterator m;
			for (m = (*p).m_meshList.begin(); m != (*p).m_meshList.end(); m++)
			{
				mesh = (*m).first;
				if (std::find(meshList.begin(), meshList.end(), mesh) ==
					meshList.end())
					meshList.push_back(mesh);
			}
		}
	}

	std::list<w_mesh*>::const_iterator it;
	for (it = meshList.begin(); it != meshList.end(); it++)
	{
		mesh = *it;
		slot = 0;
		for (i = 0; i < mesh->blendedRigidNum; ++i)
		{
			if (mesh->weightList[i] == 1.0f ||
				WisEqual(mesh->myNormalList[i], mesh->normList[i], 1e-05f) != 0)
			{
				for (weight = 0.0f; weight < 0.999f; ++slot)
				{
					weight += mesh->blendWeightList[slot];
				}
			}
			else
			{
				for (weight = 0.0f; weight < 0.999f; ++slot)
				{
					WBonePalette983* palette = mesh->palette983;
					mesh->myBlendedNormalList[slot] =
						RotVec(mesh->myNormalList[i],
							*palette->GetMatrix(mesh->boneList[i]) *
								~*palette->GetMatrix(mesh->blendedBoneList[slot]));
					weight += mesh->blendWeightList[slot];
				}
			}
		}
	}
}

WBone::w_normalmerge::P* __fastcall WBone::w_normalmerge::Find(
	std::vector<P>& plist, const WVector& p)
{
	std::vector<P>::iterator it;
	it = plist.begin();
	std::vector<P>::iterator last = plist.end();
	while (it != last)
	{
		if (WisEqual((*it).m_p, p, 0.001f))
			return &*it;
		it++;
	}
	return 0;
}

WBone::w_normalmerge::N* __fastcall WBone::w_normalmerge::Find(
	std::vector<N>& nlist, const WVector& n)
{
	std::vector<N>::iterator it;
	it = nlist.begin();
	std::vector<N>::iterator last = nlist.end();
	while (it != last)
	{
		if (WisEqual((*it).m_n, n, 0.001f))
			return &*it;
		it++;
	}
	return 0;
}

void __fastcall WBone::w_normalmerge::SetMyNormalsEqualToFirstNormal(
	w_mesh* mesh, const std::vector<int>& idxList)
{
	if (idxList.size() <= 1)
		return;
	std::vector<int>::const_iterator it = idxList.begin();
	int first = *it;
	for (; it != idxList.end(); it++)
		mesh->myNormalList[*it] = mesh->myNormalList[first];
}

// 983: 00b96ff0
int WBone::GetMeshNum983()
{
	int num = 0;
	for (w_mesh* mesh = m_mesh; mesh; mesh = mesh->next)
		num++;
	return num;
}

// 983: 00b98210
void WBone::ForEachBone983(WPuppet* pet, WBone* bone, BoneCallback983 func, void* param)
{
	do
	{
		func(pet, bone, param);
		if (bone->m_child)
			ForEachBone983(pet, bone->m_child, func, param);
		bone = bone->m_next;
	} while (bone);
}

// 983: 00b98250
void WBone::ForEachBonePost983(WPuppet* pet, WBone* bone, BoneCallback983 func, void* param)
{
	if (bone->m_child)
		ForEachBonePost983(pet, bone->m_child, func, param);
	if (bone->m_next)
		ForEachBonePost983(pet, bone->m_next, func, param);
	func(pet, bone, param);
}

// 983: 00ba6410
void WBone::GetWorldVertices983(WBone* bone, w_mesh* mesh, std::vector<WVector>& out)
{
	out.resize(mesh->vtxNum);

	int i = 0;
	int k = 0;
	for (i = 0; i < mesh->blendedRigidNum; i++)
	{
		float w = mesh->blendWeightList[k];
		out[i] = mesh->blendedBoneList[k]->Transform(mesh->blendedVecList[k]) * w;
		k++;
		float fTotal = 0.0f + w;
		while (fTotal < 0.999999)
		{
			if (k >= mesh->blendedTotalNum)
				break;
			w = mesh->blendWeightList[k];
			out[i] += mesh->blendedBoneList[k]->Transform(mesh->blendedVecList[k]) * w;
			k++;
			fTotal += w;
		}
	}

	for (; i < mesh->rigidNum; i++)
		out[i] = mesh->boneList[i]->Transform(mesh->vecList[i]);

	for (; i < mesh->vtxNum; i++)
		out[i] = bone->Transform(mesh->vecList[i]);
}

// ---- 983: paleta de ossos da malha (w_mesh::palette983) ----

WTriBones983::WTriBones983()
{
	Clear();
}

void WTriBones983::Clear()
{
	memset(this, 0, sizeof(WTriBones983));
}

void WTriBones983::Set(const w_pet_vertex* a, const w_pet_vertex* b, const w_pet_vertex* c)
{
	const w_pet_vertex* tri[3] = { a, b, c };
	memset(this, 0, sizeof(WTriBones983));
	for (int i = 0; i < 3; ++i)
	{
		const w_pet_vertex* v = tri[i];
		if (v->list == 0)
			bones[i][0] = v->bone983;
		else
		{
			for (int j = 0; j < v->listNum; ++j)
				bones[i][j] = v->list[j].bone;
		}
	}
}

unsigned int WBonePalette983::ms_maxBones = 256;

WBonePalette983::WBonePalette983()
{
}

void WBonePalette983::SetMaxBones(unsigned int n)
{
	ms_maxBones = n;
}

unsigned int WBonePalette983::IndexOf(WBone* bone) const
{
	for (unsigned int i = 0; i < bones.size(); ++i)
	{
		if (bones[i] == bone)
			return i;
	}
	return bones.size();
}

const WMatrix* WBonePalette983::GetMatrix(WBone* bone) const
{
	unsigned int i = IndexOf(bone);
	if (i < bones.size())
		return orig[i];
	return &WMatrix::IDENTITY;
}

void WBonePalette983::Remove(WBone* bone)
{
	unsigned int n = bones.size();
	unsigned int i;
	for (i = 0; i < n; ++i)
	{
		if (bones[i] == bone)
			break;
	}
	if (i < n)
	{
		bones[i] = 0;
		transf[i] = &WMatrix::IDENTITY;
		orig[i] = &WMatrix::IDENTITY;
	}
}

unsigned int WBonePalette983::CountUsed() const
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < bones.size(); ++i)
	{
		if (bones[i])
			++count;
	}
	return count;
}

unsigned int WBonePalette983::CountMissing(const std::vector<WBone*>& other) const
{
	unsigned int count = 0;
	for (std::vector<WBone*>::const_iterator it = other.begin(); it != other.end(); ++it)
	{
		if (std::find(bones.begin(), bones.end(), *it) == bones.end())
			++count;
	}
	return count;
}

void WBonePalette983::GetMissing(const std::vector<WBone*>& other, std::vector<WBone*>& out) const
{
	std::vector<WBone*> missing;
	missing.reserve(other.size());
	for (std::vector<WBone*>::const_iterator it = other.begin(); it != other.end(); ++it)
	{
		if (std::find(bones.begin(), bones.end(), *it) == bones.end())
			missing.push_back(*it);
	}
	out = missing;
}

void WBonePalette983::AddBones(const std::vector<WBone*>& add)
{
	unsigned int slot = 0;
	for (unsigned int i = 0; i < add.size(); ++i)
	{
		for (; slot < bones.size(); ++slot)
		{
			if (bones[slot] == 0)
			{
				bones[slot] = add[i];
				break;
			}
		}
		if (slot >= bones.size())
			bones.push_back(add[i]);
	}
}

bool WBonePalette983::AddTriangle(WBone* const tri[3][4])
{
	unsigned int used = CountUsed();
	if (used >= ms_maxBones)
		return false;
	std::vector<WBone*> list;
	list.reserve(12);
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
		{
			WBone* bone = tri[i][j];
			if (bone == 0)
				break;
			if (std::find(list.begin(), list.end(), bone) == list.end())
				list.push_back(bone);
		}
	}
	if (CountMissing(list) + used > ms_maxBones)
		return false;
	GetMissing(list, list);
	AddBones(list);
	return true;
}

bool WBonePalette983::Merge(const WBonePalette983& other)
{
	unsigned int count = CountUsed();
	if (count >= ms_maxBones)
		return false;
	count += CountMissing(other.bones);
	if (count > ms_maxBones)
		return false;
	std::vector<WBone*> missing;
	GetMissing(other.bones, missing);
	AddBones(missing);
	return true;
}

void WBonePalette983::CopyFrom(const WBonePalette983& src, WPuppet* pet)
{
	bones.resize(src.bones.size());
	for (unsigned int i = 0; i < bones.size(); ++i)
		bones[i] = pet->FindBone(src.bones[i]->m_name);
}

// 983 00ba5740: transf = matriz do osso pelo mapa id -> &osso->m_matrix do boneco (+0x1e0); orig = matriz original do
// mesmo id guardada pelo nome do .pet da fonte. O 983 nao confere se os ids existem nos mapas.
void WBonePalette983::Bind(WPuppet* pet, WPuppet* source)
{
	WPuppet* from = source;
	if (source == 0)
		from = pet;
	else if (pet != source)
	{
		for (unsigned int i = 0; i < bones.size(); ++i)
		{
			if (bones[i])
				bones[i] = pet->FindBone(bones[i]->m_name);
		}
	}
	const WPuppet::w_original_matrix_list983::tMatrixById& list =
		WPuppet::ms_xOrginalMatrixList.Get(from->m_petName);
	transf.resize(bones.size());
	orig.resize(bones.size());
	for (unsigned int i = 0; i < bones.size(); ++i)
	{
		WBone* bone = bones[i];
		if (bone == 0)
		{
			transf[i] = &WMatrix::IDENTITY;
			orig[i] = &WMatrix::IDENTITY;
		}
		else
		{
			int id = bone->m_id;
			transf[i] = pet->m_boneMatrices983.find(id)->second;
			orig[i] = &list.find(id)->second;
		}
	}
}
