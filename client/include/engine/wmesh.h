#pragma once
#include "wmath.h"
#include <vector>

class WBone;
struct WxBatch;
struct WxBatchGrp;
struct WBonePalette983;
struct WxShaderMtl;

// 983: coordenada de textura de um vertice (8 B; o resize 009394e0 copia como 2 dwords)
struct w_uv983
{
	float u, v;
};

// 983: as UVs da malha viraram camadas (uvBackup/uvData do 645 sairam). new em GetIndexedmesh 00ba8240
// (2 camadas se mapType > 1; senao o no de camadas da face), copia em CopyMesh 00ba7d60 (00ba7cb0),
// apagadas em ClearMesh 00ba9500 (00941b90 = destrutor que apaga, 00940ae0 = _Tidy). Camada 0 = uvData
// do 645, camada 1 = uvBackup do 645 (env/specular/lightmap).
typedef std::vector<w_uv983> w_uvlayer983;
typedef std::vector<w_uvlayer983> w_uvset983;

// layout do 983 (sizeof 0xd8; construtor 00ba0ff0, operator= 00b93ef0). Diferencas para o 645 (0x110):
// UVs num so ponteiro em +0x50; campo novo em +0x80 (material de shader; o xRenderSubset 00bd3520 o poe no estado);
// paleta de ossos em +0xa0 (no lugar do mpetName: o WxTnLBuffer do 983 pega os ossos/matrizes por ela);
// float novo em +0xd4 (-FLT_MAX no construtor). Bits de xiDrawFlag2 (+0xb4) do 983: 0x200 = dupla face,
// 0x800 = com ossos (eram 0x400/0x1000 no 645).
struct w_mesh
{
	w_mesh();	// 00ba0ff0

	int vtxNum, rigidNum, blendedRigidNum, blendedTotalNum;	// +0x00
	WVector* vecList;			// +0x10
	WVector* normList;			// +0x14
	WVector* blendedVecList;	// +0x18
	float* blendWeightList;		// +0x1c
	WVector* blendedNormalList;	// +0x20
	ulong* originalVtxColorList;	// +0x24
	int maxBoneNum;				// +0x28
	float* weightList;			// +0x2c
	int indexNum;				// +0x30
	ushort* indexList;			// +0x34
	int originalTexHandle;		// +0x38
	float originalTu, originalTv, originalScaleu, originalScalev;	// +0x3c
	ulong diffuse;				// +0x4c
	w_uvset983* uv;				// +0x50
	bool bEnv, bSpec;			// +0x54
	int group, flags;			// +0x58
	w_mesh* next;				// +0x60
	float alpha;				// +0x64
	WBone** boneList;			// +0x68
	WBone** blendedBoneList;	// +0x6c
	ulong* vtxColorList;		// +0x70
	ulong** vtxColorPtrList;	// +0x74
	WVector* myNormalList;		// +0x78
	WVector* myBlendedNormalList;	// +0x7c
	WxShaderMtl* xpShaderMtl983;	// +0x80 (nome meu): material de shader da camada; vai para WxBatchState::xpShaderMtl
	int texHandle;				// +0x84
	float tu, tv, scaleu, scalev;	// +0x88
	int clipFlag, drawFlag;		// +0x98
	WBonePalette983* palette983;	// +0xa0 (nome meu)
	WxBatchGrp* xpBatchGrp;		// +0xa4
	WxBatch* xpBatch;			// +0xa8
	int xiBaseVtxIdx, xiBaseIdxIdx, xiDrawFlag2;	// +0xac
	WBone* aabbBone;			// +0xb8
	Waabb localAabb;			// +0xbc
	float f_d4_983;				// +0xd4 (nome meu); fim do w_mesh do 983 (0xd8)

	// camada n das UVs como float[2] (0 se nao houver): 0 = uvData do 645, 1 = uvBackup do 645
	float (*UVLayer983(unsigned int n) const)[2]
	{
		if (uv == 0 || n >= uv->size() || (*uv)[n].empty())
			return 0;
		return reinterpret_cast<float (*)[2]>(&(*uv)[n][0]);
	}
	float (*UVData983() const)[2] { return UVLayer983(0); }
	float (*UVBackup983() const)[2] { return UVLayer983(1); }
	// troca as UVs por `layers` camadas de `num` vertices (zeradas); layers 0 apaga
	void NewUV983(unsigned int layers, int num)
	{
		delete uv;
		uv = layers != 0 ? new w_uvset983(layers, w_uvlayer983(num)) : 0;
	}
};
