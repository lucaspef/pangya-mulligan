#pragma once

class CPartTidList;
class WPuppet;
class WBoneSet;
class WScene;
class WView;
struct LightSet;
struct w_motion_data;

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class CPartList
{
public:
	bool FindPart(unsigned long tid) const;

	unsigned char m_unused0[0x13c];
};

// D872: deslocamentos do CPetFrame real (partsinfo.h; conferidos por C_ASSERT em partsinfo.cpp). Antes de m_partList ha 2
// std::vector e o WBoneSet: no cl 14.44 (VS 2022) eles medem menos que no VC10 e tudo dali em diante anda -0xc (out/D872/probe.py).
#if defined(_MSC_VER) && _MSC_VER > 1600
#define REBANG_PETFRAME_MAKEUP 0x64
#define REBANG_PETFRAME_PARTLIST 0x1ec
#define REBANG_PETFRAME_SIZE 0x340
#else
#define REBANG_PETFRAME_MAKEUP 0x70
#define REBANG_PETFRAME_PARTLIST 0x1f8
#define REBANG_PETFRAME_SIZE 0x358
#endif

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class CPetFrame
{
public:
	CPetFrame();
	~CPetFrame();

	// 983 008b23f0: 6 argumentos (ret 0x18); o 6o e o IDummyPlayer dono (NULL aqui; ver partsinfo.h)
	bool Build(const CPartTidList* tids, unsigned long* uids, bool bAni,
		bool bTnL, int option, class IDummyPlayer* pOwner = NULL);
	bool Refresh(CPartTidList* tids);
	WPuppet* GetBonePet();
	WPuppet* GetCurAniPet();
	void ApplyBoneSet983(WBoneSet* set); // 983 008acc90
	void Animate983(WBoneSet* set, const char* name, float time); // 983 008acdf0 (GetBones + ApplyBones do 645 juntos)
	void UpdateLightSource(LightSet* light, bool addLight, WScene* scene);
	void SetAlpha(unsigned char alpha);
	void Render(WView* view, bool bTransform, float scale, bool bFlag0,
		bool bFlag1, bool bFlag2);
	bool FindAuxPart(unsigned long tid);
	bool SetMakeup();
	bool HasMPet(char* name);
	void SetInvisible(bool bInvisible);
	w_motion_data* FindMotionData(const char* name, bool bFlag,
		WPuppet** ppPuppet);

	// visao parcial do CPetFrame de partsinfo.h, com os deslocamentos do 983 (m_partList +0x1f8; 645: +0x1b8)
	unsigned long m_tid;
	WPuppet* m_pBonePet; // +0x04
	unsigned char m_unused8[0x8];
	int m_skinTexture; // +0x10
	unsigned char m_unused14[REBANG_PETFRAME_MAKEUP - 0x14];
	int m_makeupIndex; // +0x70 (008acc30 le o byte baixo)
	unsigned char m_unused74[REBANG_PETFRAME_PARTLIST - REBANG_PETFRAME_MAKEUP - 4];
	CPartList m_partList; // +0x1f8 (com m_auxPart[5] no fim)
	// 983: o resto ate sizeof 0x358 (o `new CPetFrame` do 983 faz `push 0x358`, 00876dc9; o partsinfo.h tem m_addonSubPets983 aqui).
	// Sem isto a visao media 0x334 e quem faz `new CPetFrame` por ela alocava 0x24 bytes a menos do que o construtor escreve.
	unsigned char m_unused334[REBANG_PETFRAME_SIZE - REBANG_PETFRAME_PARTLIST - sizeof(CPartList)]; // +0x334
};
