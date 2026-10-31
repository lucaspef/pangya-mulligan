#pragma once
#include "wflag.h"
#include <map>
#include <list>
#include <vector>
#include <stdio.h>
#include "wmath.h"
#include "wscene.h"
#include "wvideo.h"

class WScene;
class WPuppet;
class WBoneSet;
struct WBoneKey;
class WBlockModel;
class cFile;

struct w_mesh;
struct w_faces;

// 983: funcao chamada para cada osso por WPuppet::ForEachBone983 (ecx = boneco, edx = osso, pilha = contexto). Nome meu.
class WBone;
typedef void (__fastcall *BoneCallback983)(WPuppet* pet, WBone* bone, void* param);

#include "wbonepalette983.h"

struct w_keyframe;
struct w_pet_vertex;
struct w_pet_tri_point;
struct w_pet_texture_info;
struct w_pet_face983;

struct w_keyframe_pos
{
	float time;
	WVector pos;
};

// 983: chave de FOV da camera (8 bytes; lida pelo GetKeyFOV983 00b94860; nome nosso)
struct w_keyframe_fov983
{
	float time;
	float fov;
};

struct w_keyframe_rot
{
	float time;
	WQuat rot;
};

// 983: 0xc8 (GetCommonData983 00b99e20 / CopyBone 00ba7f70); o 645 tinha 0xa4 e era local da wbone.cpp. Novos: o float
// +0x84 (WBone::m_petBoneFloat983), as chaves de escala e de FOV. O 983 a monta na pilha tambem no WPuppet (00b81ca0).
struct w_common_bone_data
{
	int xid;
	char name[64];
	WSphere boundsphere; // 0x44
	WMatrix basicMat; // 0x54
	float petBoneFloat983; // 0x84
	Waabb aabb; // 0x88
	w_keyframe_pos* keyPos; // 0xa0
	w_keyframe_rot* keyRot;
	w_keyframe_pos* keyScale;
	w_keyframe_fov983* keyFov983; // 0xac
	int keyPosNum; // 0xb0
	int keyRotNum;
	int keyScaleNum;
	int keyFov983Num; // 0xbc
	int keyFlag; // 0xc0
	w_mesh* mesh; // 0xc4
};

class WBone
{
public:
	enum RenderFlag
	{
		NOCLIP = 0x1,
		NOUPDATEMAT = 0x2,
	};

	enum w_bone_flag
	{
		HIDEBONE = 0x1,
		SELFILLUM = 0x2,
		HASRIGIDVTX = 0x4,
		VISIBLE = 0x8,
		CLIPPING = 0x10,
		CLONED_MESH = 0x20,
		CLONED_ANI = 0x40,
		ROTATEBONE = 0x200,
		MOVEBONE = 0x400,
		TRANSFORM = 0x1000,
		PHYSICSMODEL = 0x2000,
		COLORPERVERTEX = 0x4000,
		CHECK_SHADOW = 0x8000,
		MESHBONE = 0x10000,
		MIRRORED = 0x20000,
		INVISIBLE = 0x40000,
		SCALEBONE = 0x80000,
	};

	struct w_bone_physic_link
	{
		struct w_bone_physic_link_element
		{
			int offset;
			float length;
		};

		int vtxnum;
		WVector* pos;
		WVector* vel;
		WVector** list;
		w_bone_physic_link_element* link;
		int* linkoffset;
	};
	class w_normalmerge
	{
	public:
		w_normalmerge(WPuppet& puppet);

	private:
		struct N
		{
			N(const WVector& n)
				: m_n(n)
			{
			}
			WVector m_n;
			std::vector<int, std::allocator<int> > m_idxList;
		};
		struct P
		{
			P(const WVector& p)
				: m_p(p)
			{
			}
			WVector m_p;
			// 983: no no' do mapa, depois da malha, vem o osso dono dela (+0x10) e so' entao a lista (+0x14); a juncao
			// compara os ossos (no 645 era o nome do boneco, w_mesh::mpetName, que o 983 nao tem)
			std::map<w_mesh*, std::pair<const WBone*, std::vector<WBone::w_normalmerge::N> > > m_meshList;
		};

	public:
		void AddVertex(const WBone* bone, w_mesh* mesh, int index);	// 983 00bacfe0 (o osso e' novo)

	private:
		void MergeNormal();
		void NormalizeMergedNormals();
		void ApplyToBlendedNormalList();
		static P* Find(std::vector<P>& list, const WVector& v);
		static N* Find(std::vector<N>& list, const WVector& v);
		static void SetMyNormalsEqualToFirstNormal(w_mesh*,
			const std::vector<int>&);
		std::map<float, std::vector<WBone::w_normalmerge::P> > m_wlist;
	};

	// 983 (00b93d40): a matriz local leva a escala por eixo
	void SetLocalMatrix(const WVector& scale, const WQuat& quat,
		const WVector& pos)
	{
		WMatrix rot;
		rot = quat;
		m_localMat.Reset();
		m_localMat.xa.x = scale.x;
		m_localMat.ya.y = scale.y;
		m_localMat.za.z = scale.z;
		m_localMat = m_localMat * rot;
		m_localMat.pivot = pos;
	}

	void SetLocalMatrix(const WQuat& quat, const WVector& pos)
	{
		SetLocalMatrix(m_basicScale983, quat, pos);
	}

	bool IsVisible() { return m_flag.GetFlag(VISIBLE); }

	bool IsHidden() { return m_flag.GetFlag(HIDEBONE); }

#ifdef REBANG_GOLFPLAYER_CPP // golfplayer.cpp defines it out of line (emission order)
	bool IsInvisible();
#else
	bool IsInvisible() { return m_flag.GetFlag(INVISIBLE); }
#endif

	WQuat& GetBasicRot() { return m_basicQuat; }

private:
	WTVertex* GetVtxBuff() { return m_vtxList; }

	WTVertex* GetVtxBuff(int index) { return m_vtxList + index; }

public:
	void EnableAlphaBlend(void);
	void RestoreVtxColor(void);
	void ResetLight(void);
	void GetBoneInfo(int* totals);
	void SetInvisible(bool recursive, bool invisible);
	void ResetMyNormalList(void);

protected:
	void CalcLight_Directional(LightSet* light, int flag, WScene* scene);

	void CalcLight_Directional_Per_Bone(LightSet* light, int flag,
		WScene* scene);

	void CalcLight_Point(LightSet* light, int flag, WScene* scene);

	void CalcLight_Diffuse_Equals_Ambient(LightSet* light, int flag);

	void CalcLight_SelfIllum(void);

public:
	void ApplyScale(float scale);
	WQuat GetKeyRot(float time);
	void SetMatrix(const WMatrix& matrix, WView* view);
	void CalcENVCoord(WView* view, w_mesh* mesh);
	void CalcHilightCoord(WView* view, const LightSet& light, w_mesh* mesh);
	int GetOverlapBox(float* box, WBone* bone);
	int SaveBoneName(FILE* file, int type, char* name);
	int SaveAnimationKey(FILE* file, int id, float (*range)[2], int count);

private:
	int OptimizePosKey(float (*range)[2], int count, w_keyframe_pos** out);

	int OptimizeRotKey(float (*range)[2], int count, w_keyframe_rot** out);

public:
	void GatherNormalMergeInfo(w_normalmerge& merge) const;
	static void __fastcall CalcMeshAABB(WBone& bone, w_mesh& mesh);
	int GetMeshNum983(); // 983: 00b96ff0
	// 983: 00b98210 -- chama func neste osso, nos filhos e nos irmaos seguintes. Nome meu.
	static void __fastcall ForEachBone983(WPuppet* pet, WBone* bone, BoneCallback983 func, void* param);
	// 983 00b98250: filho e irmaos primeiro, depois o osso (o callback pode apagar o osso; nome meu)
	static void __fastcall ForEachBonePost983(WPuppet* pet, WBone* bone, BoneCallback983 func, void* param);
	// 983: 00ba6410 -- vertices da malha no espaco do mundo (misturados, rigidos e do proprio osso). Nome meu.
	static void __fastcall GetWorldVertices983(WBone* bone, w_mesh* mesh, std::vector<WVector>& out);
	// 983 00ba9e10: material por face (w_pet_face983) e camadas de UV dos pontos
	w_mesh* GetIndexedmesh(w_pet_vertex* vertexList, w_pet_tri_point* pointList,
		WBone** boneList, w_pet_face983* faceList, int faceNum,
		int* faceIndexList);
	w_mesh* CopyMesh(w_mesh* source);
	void CopyBone(w_common_bone_data* data); // 983 00ba7f70
	void GetCommonData983(w_common_bone_data* data); // 983 00b99e20 (nome meu)

	void SetRigidVtxFlag(void);
	void AllocVTX(int count);
	int SetID(int id, bool flag, char* name);
	bool ChangeTexture(const char* name, int handle);
	bool ChangeTexture(const char* name, int oldHandle, int handle);
	void OptimizeBoneSet(float (*range)[2], int count, int child);
	void CopyBasicMatrix(WBone* root, bool child);
	void xRender(WView* view, WxBatchState* state, bool setMatrix, int handle,
		int newHandle);

private:
	static int __cdecl SortByTextureHandle(const void* left, const void* right);

	char* GetNameSubBip(void);

	bool CheckSuitableForKeyframe(float start, float end, float (*range)[2],
		int count, bool last);

public:
	void SetMesh(w_faces* faces, int count);
	void SetKeyframe(w_keyframe* frame, float end, float start);
	void ReplaceKeyframe(WBone* bone, float time);
	w_mesh* GetIndexedmesh(w_faces** faces, int count);
	unsigned long* FindVColor(const WVector& pos, const WVector& normal);

private:
	void CountVtxBuff(int count);

	void ClearMesh(w_mesh* mesh);

public:
	WBone();
	~WBone();
	WBone* FindBone(const char* name, unsigned long hashCode);
	WBone* FindBone(int id);
	WBone* FindBoneByTail(const char* tail);
	static bool IsMeshVisible(WBone* bone, w_mesh* mesh);

#ifdef REBANG_PCH_SCENEMANAGER // HACK: only in the scenemanager PCH, other units keep their IL and name count
	unsigned char GetAlpha() { return m_alpha; }
	const LightSet& GetLight() { return m_light; }
#endif
	void SetAlpha(unsigned char alpha, int recursive);
	void xSetAlpha(unsigned char alpha, int recursive);
	void SetLight(LightSet* light);
	void CalcLight(LightSet* light, bool recursive, int lightMode,
		WScene* scene);
	void CalcLight(WScene* scene, int lightMode);

	void UpdatePhysicsModel(float delta, const WVector& accel);
	void ResetPhysicsModel();
	void SetPhysicsModel(int type);
	void SetRenderMode(int mode);

	void Transform(const WMatrix& mat, WView* view, float matscale, int flag);
	void Transform(WVector& result, const WVector* vec);

	void Render(WView* view, const LightSet& light);
	void RenderNormals(WView* view, float axisLen);

	// 983 00ba5310: ossos das malhas pelo nome no boneco pet; os que ele nao tem (marcados 0x100000, chunk "EXTR") sao
	// criados por WPuppet::CreateMissingBone983; os marcados registram owner (AddOwnerUp983). O 645 recebia a raiz.
	void ResetMeshBone(WPuppet* pet, WPuppet* owner, bool bRecur);
	// 983 00ba4050: poe o boneco na lista +0x1d0 se ainda nao esta (true = pos); 00ba4b40: o mesmo nos pais marcados
	// com 0x100000 (para no primeiro que ja tinha) e neste osso. Nomes meus.
	bool AddOwner983(WPuppet* pet);
	bool RemoveOwner983(WPuppet* pet); // 983 00ba3ff0 (nome meu)
	// 983: bit 0x100000 = osso do chunk "EXTR" (lido como bit: shr 0x14 / test 1). Nome meu.
	bool IsExtraBone983() const { return ((unsigned long)m_flag >> 20) & 1; }
	void AddOwnerUp983(WPuppet* pet);
	void ApplyBoneSetList(WBoneSet* set);
	int GetConvexArea(WBlockModel* model);
	void SetLightLine(float value);
	void SetMirrored(bool mirrored);
	void GetName(char** names, int& count);
	char* GetBoneName() { return m_name; }
	void SetVtxColor(unsigned long color);
	float LoadAnimationKey(cFile* file, unsigned long petVersion983);	// 983 00b97010: + escala e FOV
	// 983 00bab7f0 / 00bab400: malhas divididas pela paleta de ossos (WBonePalette983::AddTriangle);
	// pet = boneco dono das paletas (WPuppet::GetPalette983)
	void SetBoneMesh(w_pet_vertex* vertex, int count, w_pet_tri_point* point,
		w_pet_face983* face, WBone** bone, int flag, WPuppet* pet);
	void SetMesh(w_pet_vertex* vertex, int count, w_pet_tri_point* point,
		w_pet_face983* face, WBone** bone, int flag, WPuppet* pet);
	WBone* FindFakeBodyBone(w_pet_vertex* vertex, int count);
	void FixNormal(const WMatrix& matrix);
	void DisableFog();
	bool CompareBasicMatrix(WBone* bone);
	void GetTotalOverlapBox(WVector* min, WVector* max, WMatrix* matrix,
		bool flag);
	void SetBasicMatrix(const WMatrix& matrix);
	void SetNext(WBone* bone, bool recursive);
	void SetParent(WBone* parent)
	{
		if (parent != 0)
		{
			m_next = parent->m_child;
			m_parent = parent;
			parent->m_child = this;
		}
		else
			m_parent = 0;
	}
	void SetName(char* name);
	void ChangeTextureByHandle(int from, int to, bool recursive, bool flag);
	WVector GetDeltaVec(float time);
	bool CheckRigidVtx(WBone* bone);
	void SetBoundBox(Waabb box, bool flag);
	void MergeMesh(WBone* source, WBone* root);
	void ReleaseChild(WBone* child);
	int GetMeshForFaceAnimation(unsigned char group, w_mesh** mesh, int count);
	int GetMeshForFaceAnimation(int group, w_mesh** mesh, int count);
	void CombineVtxColor(bool recursive);
	// 983 00ba8100: nao clona os ossos 0x10000 nem os 0x100000 (pula para o irmao); prev = irmao anterior do clone
	WBone* MakeClone(WBone* parent, WBone* prev);
	void Rotate(float angle, int axis, bool flag);
	void Move(WVector& vector, bool recursive);
	void SetScale(float scale);
	void InsertBoneSetList(WBoneSet* set, float time, int flag,
		WPuppet* filter); // 983: 4o argumento (00ba2270)
	void ApplyBoneKey(WBoneKey* key); // 983: 00b94cf0 (nome nosso)
	WVector GetKeyScale(float time); // 983: 00b94730 (nome nosso)
	float GetKeyFOV983(float time); // 983: 00b94860 (nome nosso): FOV da camera no tempo; -1 sem chaves
	void SetSelfIllum(bool enabled, bool recursive);
	void SetSelfIllumColor(unsigned long color, bool recursive);
	void ShowBone(bool recursive);
	void HideBone(bool recursive);
	WVector GetKeyPos(float time);
	void ApplySpecularMap(int handle);
	void CountBones(int& count);
	WBone* AttachMeshBone(WBone* bone, int handle, int oldHandle);
	bool CheckMeshBoneAllVertexRigid() const;

	void ReleaseLink()
	{
		m_child = 0;
		m_next = 0;
	}

	w_mesh* GetMesh() { return m_mesh; }
	WVector Transform(const WVector& vector) { return vector * m_matrix; }
	WSphere GetBoundSphere() { return m_bound_sphere; }
	bool HasAnimation()
	{
		// 983 00b745c7: escala e FOV tambem contam
		if (!m_keyPos && !m_keyRot && !m_keyScale && !m_key983_ec)
			return false;
		return true;
	}
	void SetIDNumber(int value) { m_id = value; }
	int GetID() { return m_id; }
	char* GetBoneMotionName() { return m_name; } // 983: sem m_motion_bone_name
	WVector& GetPivot() { return m_basicMat.pivot; }
	WMatrix& GetBasicMatrix() { return m_basicMat; }
#ifdef WBONE_GETLOCALMATRIX
	// HACK: declared unconditionally it moves realmyroommainui's inline COMDATs
	WMatrix& GetLocalMatrix() { return m_localMat; }
#endif
	WMatrix& GetMatrix() { return m_matrix; }
	void ToggleSelfIllum()
	{
		unsigned char off = (unsigned char)~((unsigned long)m_flag >> 1);
		if (off & 1)
			m_flag.Enable(SELFILLUM);
		else
			m_flag.Disable(SELFILLUM);
	}

	WBone* GetChild() const { return m_child; }
	WBone* GetNext() const { return m_next; }
	WBone* GetParent() const { return m_parent; }

protected:
	void ReleasePhysicsModel();
	int FindLink(int body, w_mesh* mesh,
		w_bone_physic_link::w_bone_physic_link_element* out, int offset);

public:
	// 983: ordem nova (construtor 00ba47d0, SetID 00b96f50). O nome abre a classe e a arvore sobe para o topo; m_motion_bone_name
	// saiu (ApplyBoneSetList 00ba1180 passa o proprio osso/nome ao FindBoneKey). Novos: escala basica, um float, 2 pares de chaves
	// (tipos e nomes provisorios) e uma lista no fim.
	char m_name[64]; // 0x00
	int m_id; // 0x40
	WBone* m_child; // 0x44
	WBone* m_next; // 0x48
	w_mesh* m_mesh; // 0x4c
	WBone* m_parent; // 0x50
	WBone::w_bone_physic_link* m_physic; // 0x54
	unsigned int m_hashCode; // 0x58
	WQuat m_basicQuat; // 0x5c
	WVector m_basicScale983; // 0x6c, construtor: (1, 1, 1)
	WMatrix m_basicMat; // 0x78
	float m_petBoneFloat983; // 0xa8, construtor: -1.0f; float lido do .pet apos a matriz do osso (>= 0xfffe0103)
	WSphere m_bound_sphere; // 0xac
	Waabb m_bound_aabb; // 0xbc
	w_keyframe_pos* m_keyPos; // 0xd4
	int m_keyPosNum;
	w_keyframe_rot* m_keyRot; // 0xdc
	int m_keyRotNum;
	w_keyframe_pos* m_keyScale; // 0xe4: chaves de escala (GetKeyScale 00b94730; nome nosso)
	int m_keyScaleNum;
	w_keyframe_fov983* m_key983_ec; // 0xec: chaves de FOV (GetKeyFOV983)
	int m_key983_f0;
	int m_keyFlag; // 0xf4
	float m_lightLine; // 0xf8
	int m_rendMode; // 0xfc
	unsigned int m_selfillumColor; // 0x100
	unsigned __int8 m_alpha;
	float m_scale;
	WFlags m_flag;
	WMatrix m_localMat;
	WMatrix m_matrix;
	WMatrix m_rotateMat;
	WVector m_moveVec;
	LightSet m_light;
	unsigned int m_vtxColor; // 0x1c8
	float m_bonescale; // 0x1cc
	std::list<WPuppet*> m_list983_1d0; // 983: 0x1d0, bonecos que usam este osso (AddOwner983 00ba4050); sizeof 0x1dc

private:
	static WTVertex* m_vtxList;
	static WVector* m_vecList;
	static int m_vtx_len;
	static int m_vtx_count;
};
