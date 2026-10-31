#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>

#include "../shared/classdefine.h"
#include "wboneset.h" // 983: CPetFrame::m_boneSet983 e um WBoneSet

class CPartTidList;
class IDummyPlayer; // 983: 6o argumento do CPetFrame::Build (include/jp/dummyplayer.h, frente B)
struct sUccItem;
class WPuppet;
class WBone;
namespace IFF983 { struct sAddonPart; }

struct sPart
{
	unsigned long tid;
	IFF_STRUCT::sPart* pPart;
	unsigned long uid;
	sPart()
		: tid(0), pPart(NULL), uid(0)
	{
	}
};

class CPartList
{
public:
	CPartList();
	~CPartList();

	bool Build(const CPartTidList* pTids);
	unsigned long SetPart(CPartTidList* pTids, unsigned long tid,
		unsigned long uid);
	bool SetHairClr(CPartTidList* pTids, unsigned char color);
	bool SetShirtsClr(CPartTidList* pTids, unsigned char color);
	bool IsIdentical(CPartTidList* pTids);
	bool FindPart(unsigned long tid) const;

	unsigned char GetNumTotalParts() const { return m_partNum; }

	const sPart* GetPart(unsigned char index) const
	{
#ifdef RB_GETPART_VIA_TOTAL
		if (index >= GetNumTotalParts())
#else
		if (index >= m_partNum)
#endif
			return NULL;

		return &m_part[index];
	}

protected:
	bool SetPart(unsigned long tid, unsigned long uid);
	void EmptyList(unsigned long mask);
	bool CheckSubParts();
	bool IsEmpty(unsigned long mask, unsigned char except);

public:
	unsigned long m_tidChar;
	unsigned char m_defPartNum;
	unsigned char m_partNum;
	unsigned char m_hairColor;
	unsigned char m_shirtsColor;
	sPart m_part[24];
};

// 983: visao de um registro do AddonPart.iff (IFF983::sAddonPart, 0x120 bytes, no CItemManager::m_AddonPartMap983 pelo grupo do
// personagem, 00a193b0). So tid/Data/Tex/OrgTex foram medidos (008b1bc0, 008b1840, 008b1220); nomes meus.
struct sSubPetDef983
{
	unsigned long active983; // +0x00: 1o dword dos registros IFF; nao lido aqui
	unsigned long tid;     // +0x04 (008b1840: comparado com a chave; 008b1220: tipo 0x4000000)
	char Data[40];         // +0x08 ("%s.mpet")
	char Tex[3][40];       // +0x30
	char OrgTex[3][40];    // +0xa8
};

// 983: sub-peca carregada (valor do mapa de +0x334: chave +0xc, pDef +0x10, pPet +0x14, tex +0x18 no no de 0x28)
struct sSubPet983
{
	sSubPet983() : pDef(NULL), pPet(NULL) { tex[0] = 0; tex[1] = 0; tex[2] = 0; }
	sSubPetDef983* pDef; // aponta para o IFF983::sAddonPart do mapa do CItemManager
	WPuppet* pPet;
	int tex[3];
};

// 983: membro de +0x334 do CPetFrame (0x24 bytes): sub-pecas do personagem (pets extras presos aos ossos). Nomes meus.
// construtor 008b1750, destrutor 008b1620. F5 C683: era sPetFrameUnk983_334 (nome pelo papel: sub-pecas addon do IFF).
struct sAddonSubPets983
{
	typedef std::multimap<unsigned long, sSubPet983> SubPetMap; // 008b1840 insere com repeticao (insert_equal)
	typedef std::pair<SubPetMap::iterator, SubPetMap::iterator> SubPetRange;

	sAddonSubPets983();
	~sAddonSubPets983();
	// 983 008b1840: acerta o mapa pelas registros de m_pAddonParts983 e devolve a faixa a recarregar
	// (bAll: todas; senao so as do slot/tid). Vazia se m_pAddonParts983 == NULL ou (!bAll && !m_bAddonParts983).
	SubPetRange Collect983(bool bAll, unsigned long key, unsigned long tid);
	// 983 008b1220: refaz m_bones com os ossos do boneco que tem o nome de um osso de malha de cada sub-peca tipo 0x4000000
	void CollectBones983(WPuppet* pBonePet);
	// 983 008b0d50
	void ReleaseAll983();
	// 983 008af2d0: alguma sub-peca carregada tem o boneco com esse nome (fim do HasMPet 008af620)
	bool HasSubPet983(const char* name);

	SubPetMap m_subPets;              // +0x00
	std::list<WBone*> m_bones;        // +0x0c (list do VS2010 = 0x10)
	std::multimap<unsigned int, IFF983::sAddonPart>* m_pAddonParts983; // +0x1c (SetPartPets com mask -1: FindAddonPartGroup983(m_tidChar))
	bool m_bAddonParts983;             // +0x20
};

// 983: as texturas de troca de uma peca (o "int tex[3]; int org[3];" do SetPartPets do 645 virou objeto). Nomes meus.
// tex = a textura nova, org = a que esta no modelo (ChangeTextureByHandle(tex, org)); o Release so solta as org.
struct sPartTex983
{
	sPartTex983();                 // 983 008ad8e0
	void Release();                // 983 008acb80 (nao e destrutor: o SetPart983 do 983 nao tem EH)
	void Clear();                  // 983 008acb20
	void Apply(WPuppet* pPuppet);  // 983 008acb40
	// 983 008b05a0 (stdcall): texturas da peca; roupa UCC (S4_UCC) com o desenho do jogador (_F/_B) se baixado
	static void __stdcall Load(IFF_STRUCT::sPart* pPart, unsigned long uid, sPartTex983* pOut);

	int tex[3];
	int org[3];
};

class CPetFrame
{
public:
	// 983: observador da hierarquia de ossos (vtable 00cfeb64, 1 virtual puro; a AvatarBase da frente B e um): o EndBuild983
	// avisa cada um registrado em +0x58 (pedido B28)
	class HierarchyChangeObserver
	{
	public:
		virtual void OnHierarchyChange(CPetFrame* pFrame) = 0;
	};

	CPetFrame();
	~CPetFrame();

	void AddObserver983(HierarchyChangeObserver* pObserver); // 983 008b14b0: acrescenta se ainda nao esta (push_back 008b1120)

	// 983 008b23f0: 6o argumento = o IDummyPlayer dono (lobby/podio; NULL nos outros): sem ele, ou com m_kind != 1 e m_e0,
	// a textura do rosto passa pelo CGDSkinItem ("TanLines")
	bool Build(const CPartTidList* pTids, unsigned long* pAuxParts, bool bColor,
		bool bAniPet, int face, IDummyPlayer* pOwner = NULL);
	bool Refresh(CPartTidList* pTids);
	bool ResetFace(CPartTidList* pTids);
	// 983 008b28b0: 4o argumento novo (o cabelo usa o tid da peca no SetHairClr983); os chamadores do 983 passam 1
	// (golfplayer 00746133, 0061e263) ou uma variavel (00566774). Fim pelo EndBuild983.
	unsigned long SetPart(CPartTidList* pTids, unsigned long tid,
		unsigned long uid, bool bHairTid = true);
	// 983 008b0960 (par 839 CPetFrame::SetPart; pedido B28): troca so as texturas da roupa UCC (sUccClothes: id +0, typeId +0x10)
	void SetPart983(const sUccItem* pUcc);
	bool SetAuxPart(unsigned char index, unsigned long tid);
	bool FindAuxPart(unsigned long tid);
	bool SetHairClr(CPartTidList* pTids, unsigned char color);
	bool SetShirtsClr(CPartTidList* pTids, unsigned char color);
	// 983: 008af6b0 (o SetHairClr do 983: com bSkin o rosto do "_hr" vem do CGDSkinItem::Apply com cur = hairTid; grava a cor nas pecas)
	bool SetHairClr983(CPartTidList* pTids, unsigned char color, unsigned long hairTid, bool bSkin);
	bool SetMakeup();
	char* GetMakeupTexture();
	void AddAniPet(char* name);
	WPuppet* GetBonePet();
	std::vector<WPuppet*>& GetAniPetList();
	// 983: tamanho do m_data do conjunto de ossos (+0x48 no 983/VC10; a AvatarBase 0047f130 lê por aqui, A823)
	int GetBoneCount983() const { return (int)m_boneSet983.m_data.size(); }
	WPuppet* GetCurAniPet();
	w_motion_data* FindMotionData(const char* name, bool bSelect,
		WPuppet** ppPet);
	int GetMotionCountIncludeName(const char* name, const char* include);
	void CalcBVolume();
	bool HasMPet(char* name);
	void SetInvisible(bool bInvisible);
	void ChangeTexture(char* from, char* to);
	bool ChangeTexturePart(char* anim, char* tex, Bitmap** ppBitmap,
		tagRECT* pRect);
	void ChangeTexturePart(char* anim, Bitmap* pBitmap, const tagRECT& rect);

	int GetFaceTex() { return m_faceTex; }
	unsigned long GetTidChar() { return m_tidChar; }

	void ApplyBoneSet983(WBoneSet* pBoneSet); // 983 008acc90 (m_pBonePet->ApplyBones983)
	void Animate983(WBoneSet* pBoneSet, const char* name, float time); // 983 008acdf0 (GetBones + ApplyBones do 645 juntos)
	void Transform(WView* pView, float delta, int flag);
	void UpdateBound(bool bForce);
	void UpdateLightSource(LightSet* pLight, bool bUpdate, WScene* pScene);
	void SetAlpha(unsigned char alpha);
	void SetAlpha(char* name, unsigned char alpha);
	void Render(WView* pView, bool bShadow, float alpha, bool b1, bool b2,
		bool b3);

protected:
	void ReleasePets(int mask);
	void SetPartPets(unsigned long mask); // 983 008b1ce0
	// 983 008af370: o pet de um slot (troca as texturas se e o mesmo .mpet, senao carrega e troca as malhas); devolve
	// se a camisa ficou com a textura de hShirts (o m_bShirtsClr do 645). szPet == NULL tira o pet.
	bool LoadPartPet983(const char* szPet, WPuppet** ppPet, int* partTex, int* tex, int* org, int hShirts);
	// 983 008b1bc0: recarrega as sub-pecas de m_addonSubPets983 (bAll: todas e refaz a lista de ossos)
	void RefreshSubPets983(bool bAll, unsigned long key, unsigned long tid);
	void EndSetPart();
	void ApplyHideMask(bool bApply);
	void EndBuild983(); // 983 008b2370: fim do Build (malhas, mascara, conjunto de ossos do tamanho do esqueleto, observadores)
	w_face_animation* FindFaceAnim(char* name);

	unsigned long m_tidChar;
	WPuppet* m_pBonePet;
	int m_hairTex;
	int m_shirtsTex;
	int m_faceTex;
	std::vector<WPuppet*> m_aniPetList;
	WPuppet* m_pCurAniPet;
	// 983: sizeof 0x358 (645: 0x2f4). +0x40 aqui: m_defPartNum vai para 0x68, m_pPartPet 0x74, m_partTex 0xd4, m_bShirtsClr 0x1f4,
	// m_partList 0x1f8, m_auxPart 0x320 (008acd56); +0x24 no fim.
	WBoneSet m_boneSet983; // 983: 0x28 (construtor 00b4f520 = WBoneSet::WBoneSet); m_data com 1 chave por osso (EndBuild983)
	std::vector<HierarchyChangeObserver*> m_hierObservers983; // 983: 0x58, observadores (o destrutor 008b1adb o esvazia)
	int m_defPartNum;
	int m_partNum;
	int m_makeupIndex;
	WPuppet* m_pPartPet[24];
	int m_partTex[24][3];
	bool m_bShirtsClr;
	bool m_bMakeup;
	CPartList m_partList;
	unsigned long m_auxPart[5];
	sAddonSubPets983 m_addonSubPets983; // 983: 0x334..0x358

public:
	CPartList& GetPartList() { return m_partList; }
	void SetAddonParts983(bool b) { m_addonSubPets983.m_bAddonParts983 = b; } // 983 007434c0: +0x354
};

namespace FIXHAIR
{
	struct sFixItem
	{
		int charIndex;
		unsigned long tid;
		std::string texture;
	};

	extern sFixItem exTable[3];
}
