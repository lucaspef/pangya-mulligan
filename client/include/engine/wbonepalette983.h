#pragma once
#include "wmath.h"
#include <vector>

class WBone;
class WPuppet;

// 983: paleta de ossos de uma malha (w_mesh::palette983, +0xa0; 0x30 B; nomes meus). No 983 o indice de osso gravado no vertex
// buffer e a posicao do osso nesta paleta (IndexOf), nao WBone::m_id, e as matrizes saem dela (GetMatrix). As paletas moram
// numa lista do WPuppet (+0x1f0, WPuppet::GetPalette983 junta as que cabem). No maximo ms_maxBones ossos (00e42b00, 256).
// Posicao vaga = osso 0 (Remove), com as duas matrizes apontando para a identidade.
struct WBonePalette983
{
	std::vector<WBone*> bones;			// +0x00
	std::vector<const WMatrix*> transf;	// +0x10: matriz do osso no boneco (mapa id -> matriz do WPuppet +0x1e0)
	std::vector<const WMatrix*> orig;	// +0x20: matriz original do osso (por nome do boneco e id)

	WBonePalette983();							// 00ba40d0 (fora de linha no 983)
	unsigned int IndexOf(WBone* bone) const;		// 00b9fa60 (bones.size() se nao achar)
	const WMatrix* GetMatrix(WBone* bone) const;	// 00b9faa0 (orig; identidade se nao achar)
	void Remove(WBone* bone);						// 00b9fad0
	unsigned int CountUsed() const;					// 00b9fb30
	unsigned int CountMissing(const std::vector<WBone*>& other) const;	// 00ba25e0
	void GetMissing(const std::vector<WBone*>& other, std::vector<WBone*>& out) const;	// 00ba4150
	void AddBones(const std::vector<WBone*>& add);	// 00ba42f0 (preenche as vagas primeiro)
	bool AddTriangle(WBone* const tri[3][4]);		// 00ba4f20 (ossos dos 3 vertices; false se nao couber)
	bool Merge(const WBonePalette983& other);		// 00ba5140 (false se nao couber)
	void CopyFrom(const WBonePalette983& src, WPuppet* pet);	// 00ba40f0 (ossos de mesmo nome no boneco)
	void Bind(WPuppet* pet, WPuppet* source);		// 00ba5740 (enche transf/orig)

	static void SetMaxBones(unsigned int n);		// 00b98200
	static unsigned int ms_maxBones;				// 00e42b00
};

struct w_pet_vertex;

// 983: ossos dos 3 vertices de um triangulo (0x30 = 3 x 4; nomes meus): vertice rigido = so o osso em [i][0], com mistura =
// os ossos da lista. Entrada do WBonePalette983::AddTriangle (SetMesh 00bab400 / SetBoneMesh 00bab7f0; polysoup tambem).
struct WTriBones983
{
	WBone* bones[3][4];

	WTriBones983();					// 00b9fa50 (fora de linha: chama Clear)
	void Clear();					// 00b98170 (memset 0x30)
	void Set(const w_pet_vertex* a, const w_pet_vertex* b, const w_pet_vertex* c);	// 00b98180
};
