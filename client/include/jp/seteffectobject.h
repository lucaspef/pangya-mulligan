#pragma once
#include <map>
#include <queue>
#include <vector>
#include "controlledobject.h"

// JP 983: registro do efeito de conjunto devolvido por 00a177f0 (tabela do CItemManager; nomes meus, só os campos lidos aqui).
struct sSetEffectData983
{
	char m_00[4];
	int m_ability[3]; // +0x04 (1…8: qual CSetAbility*; 0 = nenhum)
	int m_trigger[3]; // +0x10 (2, 4, 8, 0x10, 0x20: em que momento vale — CSetEffectState::ToTrigger)
	unsigned long m_typeId[5]; // +0x1c (peças do conjunto)
	unsigned char m_count[5]; // +0x30 (quantas de cada)
};

// JP 983: andamento de um conjunto de um jogador (valor do std::map do CSetEffectObject; sem RTTI, nome meu), 0x28 bytes.
class CSetEffectState
{
public:
	CSetEffectState(); // 00627820
	void ResetComplete(); // 00627850
	bool IsComplete(); // 00627860
	bool Check(); // 00627870: as cinco peças e as cinco contagens iguais às do registro
	void SetPart(int part); // 00627930
	int GetPart(); // 00627940
	void Add(unsigned long typeId, unsigned char count); // 00627950
	int ToTrigger(int kind); // 006279a0
	int GetAbility(int kind); // 006281d0

	sSetEffectData983* m_pData; // +0x00
	int m_part; // +0x04
	unsigned short m_player; // +0x08
	unsigned long m_have[5]; // +0x0c
	unsigned char m_count[5]; // +0x20
	bool m_bComplete; // +0x25
};

// JP 983: RTTI .?AVCSetAbility@@, vtable 00d15dfc, 4 virtuais, 0x4 bytes. Nomes dos virtuais meus.
class CSetAbility
{
public:
	CSetAbility() { } // 00627a30
	virtual ~CSetAbility(); // 006277b0 (006277f0 com delete)
	virtual bool Cancel(); // 006277c0
	virtual bool Apply(CSetEffectState* pState); // 006277d0
	virtual bool IsDeferred(); // 006277e0
};

// JP 983: RTTI .?AVCSetAbilityMotionChange@@, vtable 00d15e10
class CSetAbilityMotionChange : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState* pState); // 0062a3c0
};

// JP 983: RTTI .?AVCSetAbilityAddMotion@@, vtable 00d15e24
class CSetAbilityAddMotion : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState*) { return true; } // 00627a40
};

// JP 983: RTTI .?AVCSetAbilityHiddenCutIn@@, vtable 00d15e38
class CSetAbilityHiddenCutIn : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState* pState); // 00628d80
	virtual bool IsDeferred() { return true; } // 00628260
};

// JP 983: RTTI .?AVCSetAbilityAuxpartImpactZone@@, vtable 00d15e4c
class CSetAbilityAuxpartImpactZone : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState* pState); // 00628dd0
	virtual bool IsDeferred() { return true; } // 00628290
};

// JP 983: RTTI .?AVCSetAbilityPlusYard@@, vtable 00d15e60
class CSetAbilityPlusYard : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState* pState); // 00628e70
	virtual bool IsDeferred() { return true; } // 006282c0
};

// JP 983: RTTI .?AVCSetAbilitySlotPlus@@, vtable 00d15e74
class CSetAbilitySlotPlus : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState* pState); // 00628f20
	virtual bool IsDeferred() { return true; } // 006282f0
};

// JP 983: RTTI .?AVCSetAbilityWindMinus@@, vtable 00d15e88
class CSetAbilityWindMinus : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState* pState); // 00628fc0
	virtual bool IsDeferred() { return true; } // 00628320
};

// JP 983: RTTI .?AVCSetAbilityAnimalPangya@@, vtable 00d15e9c
class CSetAbilityAnimalPangya : public CSetAbility
{
public:
	virtual bool Apply(CSetEffectState* pState); // 00629060
	virtual bool IsDeferred() { return true; } // 00628350
};

// JP 983: RTTI .?AVCSetEffectObject@@, vtable 00d15ec0, 0x8c bytes (unidade 006277b0–0062b2e0, com a família CSetAbility*).
class CSetEffectObject : public CControlledObject
{
public:
	CSetEffectObject(eSpecialAbility ability); // 0062b0a0
	virtual ~CSetEffectObject(); // 0062aeb0 (0062b1f0 com delete)
	virtual void On01(const sControlledParam* pParam); // 0062afc0
	virtual void On05() { } // 00627a20
	virtual void On09(int); // 0062b010

	void Apply(int kind); // 0062a8e0 (nome meu): o controlador passa o seu m_0c
	void ApplyDeferred(); // 0062a9a0: o mesmo para o momento 0x10, só com os efeitos de IsDeferred()
	void Flush(); // 0062a6e0: aplica e esvazia a fila
	void Add(unsigned long typeId, unsigned long setId); // 0062aa80
	CSetAbility* CreateAbility(int ability); // 00628c20
	CSetAbility* GetAbility(int ability); // 00628d30

private:
	std::map<unsigned long, CSetEffectState> m_states; // +0x50 (chave = id do conjunto)
	std::queue<std::pair<unsigned long, int> > m_pending; // +0x60 (id do conjunto, efeito)
	std::vector<CSetAbility*> m_abilities; // +0x78 (9 posições; criados sob demanda)
	unsigned char m_playerIndex; // +0x88
};

// JP 983: o objeto global 00e768f8 (antes "CSetEffectHolderJp", de empréstimo) é a CMotionAdvisor: include/jp/motionadvisor.h (B110).

// JP 983: método do CItemManager que o fonte do 645 não tem (classe de empréstimo; pedido em out/PEDIDOS.md).
class CItemManagerJp
{
public:
	sSetEffectData983* FindSetEffect983(int id); // 00a177f0
	sControlledSource* FindAbility983(unsigned long typeId); // 00a177a0: registro de habilidade do item (0 se não tem)
};
