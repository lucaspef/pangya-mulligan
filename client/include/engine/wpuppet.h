#pragma once
#include "wmath.h"
#include "wresource.h"
#include "wscene.h"
#include "wlist.h"
#include <vector>
#include <string>
#include <map>
#include <list>
#include "wbonepalette983.h"
#include "wlock.h"

typedef std::vector<const WMatrix*> WMatrixPtrList;
struct WxShaderMtl;

class WBone;
class WBoneSet;
class WBlockModel;
class Bitmap;
class cFile;
class WView;
class WScene;
struct w_mesh;
struct w_pet_texture_info;
struct tagRECT;

// 983: tambem em wbone.h (mesmo tipo). Nome meu.
class WPuppet;
typedef void (__fastcall *BoneCallback983)(WPuppet* pet, WBone* bone, void* param);

struct w_bound_box_info
{
	Waabb aabb;
	WVector spherePivot;
	float sphereLength;
	char* option;
	char name[1];
};

struct w_motion_data
{
	float s;
	float e;
	float end;
	int option1;
	float option2;
	char* applybone;
	char* name;
	char* nextmotion;
	char ptr[1];
};

struct w_face_animation
{
	w_mesh** mesh;
	int meshNum;
	int group;
	char name[32];
	char filename[32];
	float tu;
	float tv;
	float du;
	float dv;
	float scalex;
	float scaley;
	int texHandle;
};

struct w_bound_box
{
	WBone* bone;
	w_bound_box_info* info;
};

struct w_motion_addition
{
	WList<w_motion_data*>* mdList;
	WBone* rootbone;
	float s;
	float e;
	float length;
	float h_gap;
	char name[1];
};

struct w_share_pet_data
{
	struct w_frame
	{
		int num;
		int check;
		char* data;
	};

	// 983 (0x3c; MakeClone 00b92640): o boneco de origem no lugar do petName/rootbone do 645 (o Share 00b92340 tira os
	// dois dele e grava m_pOrigin983) e, no fim, o vetor de materiais de shader
	WPuppet* pet;
	float length;
	int faceNum;
	int numFramedata;
	w_frame* frame;
	Waabb bound;
	WList<w_bound_box*>* bbList;
	WList<w_motion_data*>* mdList;
	WList<w_face_animation*>* fanimList;
	std::vector<WxShaderMtl*>* shaderMtls983; // 0x38
};

class WPuppet : public WResource
{
public:
	// 983: modificador do boneco (RTTI .?AVw_modifier@WPuppet@@; derivada PetModifier::CSuperDeformedPet, vtable
	// 00d52cdc). Guardado no mapa +0x210 pelo nome do tipo; o slot 1 e chamado depois de cada pose (ApplyModifiers983).
	class w_modifier
	{
	public:
		virtual ~w_modifier() {}
		virtual void Apply(WPuppet* pet) = 0; // nome meu
	};

	struct w_tex_piece
	{
		float pu;
		float pv;
		float scalex;
		float scaley;
		int texHandle;
		int count;
		char name[1];
	};

	WPuppet();
	virtual ~WPuppet();
	void Share(w_share_pet_data* shareData);
	virtual WPuppet* MakeClone(bool needBuffer);
	virtual int LoadPET(char* name, bool flag, int loadFlags);
	virtual void UpdateLightSource(LightSet* light, bool addLight,
		WScene* scene);
	virtual void Render(WView* view, bool transform, float scale, bool flag0,
		bool flag1, bool software);
	virtual void xUpdateTnLBuffers(bool) { }
	virtual void EndMeshBoneWork();

	void Transform(WView* view, float scale, int flags);
	int GetRenderMode() const;
	WBone* GetRootBone() { return m_rootbone; }
	int GetFaceNum() { return m_faceNum; }
	int GetLightMode() { return m_lightmode; }
	// 983: paletas de ossos das malhas (nomes meus)
	WBonePalette983* GetPalette983(const WBonePalette983& palette);	// 00b90940
	void BindPalettes983(WPuppet* source);							// 00b7b460
	static void __fastcall InternPalettes983(WPuppet* pet, WBone* bone, void* param);	// 00b910a0 (BoneCallback983)

	void LockBoneSetPos(unsigned char);
	unsigned char GetBoneSetPos() const;

	void SetRenderMode(int mode);
	void SetLightLine(float value);
	void SetLightMode(unsigned long);
	void UpdateLightSource(WScene* scene);
	void RenderHierarchy(WView* view, bool transform, float value, float scale);
	void RenderNormals(WView* view, bool transform, float value, float scale);
	WVector InvTransform(const WVector&);
	void ApplyBones983(WBoneSet* set); // 983 00b7b180: m_rootbone->ApplyBoneSetList(set) e os modificadores
	void ApplyModifiers983(); // 983 00b79730
	w_modifier* FindModifier983(const std::string& name); // 983 00b7f380 (chave = typeid(*mod).name())
	void AddModifier983(w_modifier* modifier); // 983 00b8e4f0 (sem chamador direto no 983)
	void AddCollisionObject(const char* name, WBlockModel* model);
	void SetAlpha(unsigned char alpha);
	void SetAlpha(char* name, unsigned char alpha);
	unsigned char GetAlpha(char* name);
	void SetMirrored(bool mirrored);
	WBone* FindBone(int id);
	WBone* FindBone(const char* name);
	WBone* FindBoneByTail(const char* tail);
	void GetBoneName(char** names, int& count);
	WMatrix GetRootMatrix();
	WVector GetPivot(const char* name);
	void SetVtxColor(unsigned long color);
	void SetSelfIllum(char* name, bool enabled);
	void SetSelfIllumColor(char* name, unsigned long color);
	void ToggleSelfIllum(char* name);
	void ShowBone(char* name);
	void HideBone(char* name);
	void ScaleBone(char* name, float scale);
	void RotateBone(char* name, float angle, int axis);
	void MoveBone(char* name, WVector& vector);
	void SetPhysicsModel(const char* name, int model);
	void UpdatePhysics(float delta, const WVector& accel);
	void DisableFog();
	void EnableAlphaBlend();
	void OptimizeBone();
	bool HaveCenterBoundBox();
	bool CompareBasicMatrix(WPuppet* other) const;
	void ChangeTexture(char* name, int handle);
	bool ChangeTexturePart(int handle, char* name, Bitmap** bitmap,
		tagRECT* rect);
	void ChangeTexturePart(int handle, Bitmap* bitmap, const tagRECT& rect);
	bool ChangeTexturePart(char* name, char* part, Bitmap** bitmap,
		tagRECT* rect);
	void ChangeTexturePart(char* name, Bitmap* bitmap, const tagRECT& rect);
	void InstantTexChange(int, int);
	void ChangeTextureByHandle(int from, int to, WPuppet* other);
	bool ChangeMesh(char* name);
	WBone* FindMeshBone(WBone* previous);
	void ReplaceMeshBones(WPuppet* oldPuppet, WPuppet* newPuppet, int handle,
		int oldHandle);
	void ApplyFaceAnimation(char* name, float time);
	w_face_animation* FindFaceAnim(char* name);
	char* FindTexAniList(char* name);
	w_motion_data* FindMotionData(const char* name);
	float GetMotionLength(const char* name);
	char* GetNextMotionName(const char* name);
	int GetMotionCountIncludeName(const char* include, const char* exclude);
	w_motion_data* GetFirstMotionData() { return m_mdList->Start(); }
	w_motion_data* GetNextMotionData();
	w_motion_addition* GetFirstAddedMotionList();
	w_motion_addition* GetNextAddedMotionList();
	// 983 00b86cf0 (nome nosso): GetBones e aplica no esqueleto (ret 0x18; recursivo nas misturas; filter == this vira
	// NULL; com set NULL aplica direto nos ossos e chama os modificadores). No 983 substitui o GetBones+ApplyBones.
	void Animate983(WBoneSet* set, const char* name, float time, int mode, char* bone, WPuppet* filter);
	WVector GetDeltaVec(float time);
	WVector GetDeltaVec(char* name, float time);
	WVector GetDeltaVec(char* name, float from, float to);
	bool CheckFrameData(int first, int last, int& from, int& to);
#ifdef REBANG_PCH_QUADTREE // HACK: only in the quadtree PCH, other units keep their IL (emission order)
	float GetAnimationLength() { return m_aniLen; }
	unsigned int GetFrameCount() { return m_framenum; }
#else
	float GetAnimationLength();
	unsigned int GetFrameCount();
#endif
	char* GetFrameData(int index) { return m_frame[index].data; }
	float GetFrameDataTime(int index)
	{
		return m_frame[index].num * (1.0f / 30.0f);
	}
	bool GetFrameCheck(int);
	WMatrix& GetMatrix() { return m_mat; }
	void SetMatrix(const WMatrix& mat) { m_mat = mat; } // 983 004d18f0 (copia fora de linha; CExhibition::Animate, IDummyPet)
	const char* GetPuppetName() const { return m_petName; }
	void CalcBound(Waabb* box, bool flag);
	void CalcBound(bool reset, WPuppet* puppet, bool mode, char* name,
		Waabb* box);
	const Waabb& GetBoundBox() { return m_bound; }
	__forceinline const WSphere& GetBoundSphere() const
	{
		return m_boundSphere;
	}
	void UpdateBound(bool transform, bool flag);
	// 983: 00b6a410 -- percorre todos os ossos a partir da raiz (WBone::ForEachBone983). Nome meu.
	static void __fastcall ForEachBone983(WPuppet* pet, BoneCallback983 func, void* param);
	void UpdateBSphere();
	Waabb GetTransformedBoundBox() const;
	bool GetBoundBoxPlane(char* name, WPlane* plane);
	bool GetAlignBoundBoxPlane(char* name, WPlane* plane, Waabb* box);
	bool DotContact(char* name, const WVector& dot);
	w_bound_box* FindBoundBox(const char* name) { return m_bbList.Find(name); }
	void RenderBoundBox(const char* name, WView* view, unsigned long color,
		float size);
	// 983 00b91d10: mapa id -> &osso->m_matrix (+0x1e0) e matrizes originais pelo nome do .pet (o 645 fazia uma lista)
	void xBuildTransfMatPtrList();

protected:
	virtual void DetachMeshBone(WPuppet* source);
	// 983 00b6a200: tira o boneco (param = WPuppet**) da lista de donos dos ossos 0x100000 (nome meu)
	static void __fastcall RemoveOwner983(WPuppet* pet, WBone* bone, void* param);
	// 983 00b8cc60: apaga os filhos 0x100000 sem dono (depois do Detach/Attach; nome meu)
	static void __fastcall ReleaseUnusedExtraBones983(WPuppet* pet, WBone* bone, void* param);
	// 983 00b6a430: como o ForEachBone983, mas filhos e irmaos antes do osso (WBone::ForEachBonePost983; nome meu)
	static void __fastcall ForEachBonePost983(WPuppet* pet, BoneCallback983 func, void* param);
	// 983 00b7d5b0: esquece um osso 0x100000 (matriz +0x1e0, paletas, caixas) antes de apaga-lo (nome meu)
	void ForgetExtraBone983(WBone* bone);
	void AttachMeshBone(WPuppet* other, int handle, int oldHandle);
	void FindMeshBone(WBone** found, WBone** previous, const char* name);
	void MergeNormal();
	void SetDefaultFaceAnimation();
	void SetBoundBox(char* name, Waabb* aabb, char* boneName);

private:
	void FreeBone(WBone*);
	void LoadPET(cFile* file, bool flag, char* directory, int type);
	w_pet_texture_info* LoadPET_Texture(cFile* file, bool flag, char* directory,
		int* count);
	void ReadVersion983(cFile* file); // 983 00b6a0f0 (nome meu)
	void LoadPET_Extra983(cFile* file); // 983 00b6a090: chunk "EXTR" (ossos marcados com 0x100000; nome meu)
	// 983 00b81b30: id -> &osso->m_matrix dos ossos sem 0x10000 (o primeiro de cada id fica; nome meu)
	void CollectBoneMatrices983(WBone* bone, std::map<int, const WMatrix*>& list);
	// 983: materiais de shader do .pet (chunk "SMTL", so com VS e PS; nomes meus)
	void LoadPET_ShaderMtl983(cFile* file); // 00b90200
	void CloneShaderMtls983(WBone* bone, int hTex, int hOrgTex); // 00b91470
	// 983 00b91750: copia dos materiais do boneco de origem (Share) e troca nas malhas (RemapShaderMtls983 00b6ed80)
	void CopyShaderMtls983(const std::vector<WxShaderMtl*>& source);
	static void __fastcall RemapShaderMtls983(WPuppet* pet, WBone* bone, void* param);

public:
	// 983 00b81ca0: osso que este boneco nao tem (0x100000, chunk "EXTR"): copia-o do m_pOrigin983 com os pais que
	// faltarem e o pendura no primeiro pai que existe; owner nao e' usado. Devolve a copia do osso pedido.
	WBone* CreateMissingBone983(WBone* bone, WPuppet* owner);

private:
	// 983 00b81ba0: matrizes (+0x1e0) e caixas do m_pOrigin983 dos ossos criados
	void RegisterCreatedBones983(std::list<WBone*>& bones);
	// 983 00b74010: copia uma caixa (info compartilhada) com o osso deste boneco
	void AddBoundBox983(w_bound_box* box);
	void LoadPET_Mesh(cFile* file, WBone* bone, w_pet_texture_info* texture,
		bool flag);
	WBone* LoadPET_Bone(cFile* file, int version);
	float LoadPET_Animation(cFile* file, WBone* bone);
	void LoadPET_FaceAnim(cFile* file, w_pet_texture_info* texture, WBone* bone,
		char* name);
	void LoadPET_Frame(cFile* file);
	WList<w_motion_data*>* LoadPET_Motion(cFile* file, float scale);
	void LoadPET_Collision(cFile* file);
	static int SeekChunkFromPET(cFile* file, char* tag);
	void GetDirectory(char* path, char* directory);
	void GetName(char* path, char* name);
	void FixNormal();
	bool CheckStaticBone(WBone* bone);
	bool ReduceBone(WBone* bone);
	WBone* MergeBone(WBone* target, WBone* source);
	void MergeRootMotion(WList<w_motion_data*>* list, WBone* bone);
	void MakeTemporaryCenterBox();
	void SetTexPiece(char* name, int handle, float pu, float pv, float scalex,
		float scaley);
	w_tex_piece* GetTexPiece(char* name);
	void ClearTexPiece();
	static WList<w_tex_piece*>* m_tex_piece;
	static unsigned char ms_lockedBoneSetPos;
	static unsigned char ms_boneSetPos;
	static int box_v[8][3];
	static int box_p[6][4];

public:
	char m_petName[0x40]; // 983: 0x2c
	WBone* m_rootbone; // 983: 0x6c
	int m_lightmode; // 983: 0x70
	WList<WBone*> m_phybone; // 983: 0x74
	WMatrix m_mat; // 983: 0x94
	WList<w_motion_addition*> m_addedMotionList; // 983: 0xc4
	int m_rendMode; // 983: 0xe4
	WPuppet* m_pOrigin983; // 983: 0xe8 (this no construtor; clone = != this); nome meu
	bool m_bHaveCenter; // 983: lugar não medido (0xec livre)
	w_share_pet_data::w_frame* m_frame; // 983: 0xf0
	int m_framenum;
	int m_faceNum; // 983: 0xf8
	Waabb m_bound;
	WSphere m_boundSphere;
	w_bound_box m_BBox;
	WList<int>* m_storeTexList;
	WList<WPuppet::w_tex_piece*> m_use_tex_piece;
	WList<w_bound_box*> m_bbList;
	WList<w_motion_data*>* m_mdList;
	WList<w_face_animation*> m_fanimList;
	// 983: 0x194 (0x44), ossos achados pelo nome com trava (FindBone 00b78680 -> Find 00b76b00; FreeBone -> Remove 00b796f0)
	struct w_bone_cache983
	{
		w_bone_cache983() : m_pet(0), m_list(4, 16) {} // 00b733d0
		WBone* Find(const char* name); // 00b76b00
		void Remove(WBone* bone); // 00b796f0

		WPuppet* m_pet; // +0x0 (o construtor do WPuppet grava this no fim)
		WList<WBone*> m_list; // +0x4
		WLock m_lock; // +0x24
	};
	w_bone_cache983 m_boneList; // 983: 0x194
	float m_aniLen; // 983: 0x1d8
	int m_iPetType; // 983: 0x1dc (LoadPET 00b92760 grava o tipo; GetPalette983 compara com 0x20104)
	std::map<int, const WMatrix*> m_boneMatrices983; // 983: 0x1e0, id do osso -> &osso->m_matrix (00b91d10, 00b81ba0)
	std::list<WBonePalette983> m_palettes983; // 983: 0x1f0 (nó 0x38); paletas das malhas (w_mesh::palette983)
	// 983: 0x1fc, materiais de shader do .pet (LoadPET_ShaderMtl983; a face aponta para [camada]); o dono apaga. Era o
	// m_xTransfMatPtrList do 645, que o 983 nao tem (o 983 usa o mapa +0x1e0 e o ms_xOrginalMatrixList com trava).
	std::vector<WxShaderMtl*> m_shaderMtls983;
	unsigned long m_petVersion983; // 983: 0x20c, versao do .pet (chunk "VERS"; PET_VERSION_DEFAULT983 sem ele)
	// 983: 0x210, objetos guardados pelo nome do tipo (typeid(*obj).name(): 00b8e4f0); o destrutor apaga
	std::map<std::string, w_modifier*> m_modifiers983;

public:
	// 983: matrizes originais dos ossos pelo nome do .pet, com trava (objeto global 00f14a18; o 645 tinha um mapa de
	// listas sem trava). Nomes dos metodos meus.
	struct w_original_matrix_list983
	{
		typedef std::map<int, WMatrix> tMatrixById;

		bool Has(const std::string& name); // 00b81ec0
		void Add(const std::string& name, const std::map<int, const WMatrix*>& list); // 00b91bc0
		const tMatrixById& Get(const std::string& name); // 00b7f2a0

	private:
		bool xHas(const std::string& name); // 00b7f310 (sem a trava)

		std::map<std::string, tMatrixById> m_list; // +0x0 (0x10)
		WLock m_lock; // +0x10
	};
	static w_original_matrix_list983 ms_xOrginalMatrixList;
};
