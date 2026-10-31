#pragma once

class WBone;
class cFile;
struct w_pet_texture_info;

struct w_pet_vertex
{
	struct w_blended
	{
		unsigned char blendWeight;
		WBone* bone;
	};

	float x;
	float y;
	float z;
	float weight;
	unsigned char blendWeight;
	WBone* bone983; // 983: +0x14 osso do esqueleto lido (era char* bonename; nome = bone983->m_name)
	w_blended* list;
	int listNum;
};

// 983: versoes do .pet (chunk "VERS"; WPuppet::m_petVersion983). Comparadas so pelos 2 bytes de baixo (00b6a3d0).
#define PET_VERSION_DEFAULT983 0xfffe0000u
#define PET_VERSION_MODELNORMALS983 0xfffe0101u	// 00e429bc: normais ja no espaco do modelo (LoadPET_Mesh 00b7c79b)
#define PET_VERSION_FACELAYER983 0xfffe0102u	// 00e429c0: 1 byte por face (camada de material; LoadPET_Mesh 00b7c6e7)
#define PET_VERSION_UVLAYERS983 0xfffe0102u	// 00e429c0: n de camadas de UV por ponto
#define PET_VERSION_FOVKEYS983 0xfffe0103u	// 00e429c4: chaves de FOV na animacao do osso (WBone::LoadAnimationKey)
int __fastcall ComparePetVersion983(unsigned long a, unsigned long b);	// 00b6a3d0 (wpuppet.cpp)

// 983: 0x54 (era 0x18 no 645): ate 8 camadas de UV por ponto do triangulo
struct w_pet_tri_point
{
	enum { MAX_UV983 = 8 };
	int pos;
	float nx;
	float ny;
	float nz;
	unsigned char uvNum983;		// +0x10 (1 nos .pet anteriores a PET_VERSION_UVLAYERS983)
	float uv983[MAX_UV983][2];	// +0x14
	void Read983(cFile* file, unsigned long version);	// 00bcfe70
};

// 983: material de cada face (0xc; era w_pet_texture_info* por face no 645). Nomes meus.
struct w_pet_face983
{
	w_pet_texture_info* tex;	// +0
	int layer;					// +4: byte do .pet (0xff = -1); so com PET_VERSION_FACELAYER983
	void* layerMtl;				// +8: WPuppet +0x1fc [layer] com VS e PS (vira w_mesh::xpShaderMtl983)
};

struct w_skin_info
{
	unsigned char ucBoneID;
	int iSVtxIdx;
	int nVtxs;
	int iSTriIdx;
	int nTris;
};

struct w_pet_texture
{
	char filename[32];
	char flag;
	unsigned char group;
	unsigned long diffuse;
	int handle;
};

struct w_pet_texture_info
{
	char filename[64];
	unsigned char alpha;
	unsigned long diffuse;
	unsigned long drawFlags;
	unsigned long xulDrawFlag2;
	int texHandle;
	int mapType;
	bool bBind;
	float pu;
	float pv;
	float scalex;
	float scaley;
	unsigned char group;
};

struct w_pet_face_animation
{
	unsigned char group;
	char animName[32];
	char fName[32];
};
