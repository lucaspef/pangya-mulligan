#pragma once
#include "wmath.h"
#include "wlist.h"
#include <vector>

// 983: unidade refeita (00b4ed00...00b4f625). A escala da chave e por eixo (WVector em +0x20, flags em +0x2c) e as chaves
// ficam num std::vector do proprio conjunto; sairam m_preAlloc/m_curAlloc, CopyFrom e BlendBoneSet. Nomes das funcoes
// novas (CopyTo, BlendTo, Blend estatica, AddBoneKey(WBoneKey*)) sao nossos.
struct WBoneKey
{
	WBoneKey() : name(0) { } // 983: so o nome e zerado (00ba2296; _Uninit_def_fill_n 0047eba0)
	char* name;
	WQuat quat; // 0x04
	WVector pivot; // 0x14
	WVector scale; // 0x20
	unsigned char flags; // 0x2c: 1 = quat, 2 = pivot, 4 = scale
};

// 983: Find desta lista nunca e expandida em linha (ApplyBoneSetList 00ba1193, GetBones 00b86f5e chamam 00b4ef60)
template <>
WBoneKey* WList<WBoneKey*>::Find(const char* keycode) const;

class WBoneSet
{
public:
	WBoneSet(); // 00b4f520
	WBoneSet(int count); // 00b4f580
	void Clear() { m_keyList.Reset(); }
	WBoneKey* FindBoneKey(char* name); // 00b4f190
	void AddBoneKey(char* name, WQuat* quat, WVector* pivot, WVector* scale,
		int flags); // 00b4f0e0
	void AddBoneKey(WBoneKey* key); // 00b4f1b0
	void CopyTo(WBoneSet* dest); // 00b4f1e0
	void BlendTo(WBoneSet* dest, float ratio); // 00b4f250
	static void __fastcall Blend(WBoneSet* dest, WBoneSet* first,
		WBoneSet* second, float ratio); // 00b4f4e0

	// PROVISORIO (s59): API do 645, so para avatar_avatar.cpp e realmyroom_avatar.cpp (corpo do 645) compilarem;
	// no 983 essas unidades chamam BlendTo direto (007a71ec, 007a78ad) e ainda nao foram relidas.
	void CopyFrom(WBoneSet* source)
	{
		Clear();
		source->CopyTo(this);
	}
	void Blend(WBoneSet* first, WBoneSet* second, float ratio)
	{
		Blend(this, first, second, ratio);
	}

	WList<WBoneKey*> m_keyList;
	std::vector<WBoneKey> m_data; // 0x20
	// 983: sizeof 0x30 (0047f0a9/0047f0be: dois conjuntos a 0x30 um do outro); o m_index do 645 saiu (dava 0x34; pedido B28)
};

#include "wboneset.inl"
