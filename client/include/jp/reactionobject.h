#pragma once
#include <vector>
#include "engine/wmath.h"
#include "actor.h"
#include "scenemanager.h"

class WPuppet;

// JP 983: objetos que reagem à bola (os cristais da Abbot Mine). RTTI .?AVCReactionObject@@ (vtable 00d12e94),
// .?AVCAbbotCrystal@@ (00d12eac); gerenciados pela CReactionObjectArea / CReactionObjectMng (mesma faixa 005f01d0–005f35b2).
// Corpo em src/projectg/jp/reactionobject.cpp. Nomes de método e de campo meus (sessões B117/B118).

// 0x40 bytes
class CReactionObject
{
public:
	CReactionObject()
		: m_pPuppet(NULL),
		  m_matrix(WMatrix::IDENTITY),
		  m_bHit(false)
	{
	}
	CReactionObject(const char* name); // 005f0a60 (sem chamador)
	CReactionObject(WPuppet* pPuppet); // 005f0a90 (sem chamador)
	virtual ~CReactionObject(); // 005f0230; deleting 005f0870

	virtual void Reset() { } // 005f0200
	virtual void OnHit() { } // 005f0210
	virtual void SetPuppet(WPuppet* pPuppet); // 005f0260

	void Display(); // 005f0ab0
	void SetPosition(const WVector& pos, bool unused983); // 005f0b50 (os dois chamadores passam 0)
	bool CheckHit983(); // 005f10b0: a esfera do boneco encosta na bola
	WPuppet* GetPuppet983() const { return m_pPuppet; }

protected:
	WPuppet* m_pPuppet; // +0x04
	WMatrix m_matrix; // +0x08 (posição em +0x2c)
	bool m_bHit; // +0x38
	float m_hitDelay; // +0x3c (0,8 ao trocar o boneco; uso na área)
};

// 0x40 bytes: gira ao acaso no Reset e explode ("crystal_bomb01.spr" + som "funobj01_hit_%02d") no OnHit
class CAbbotCrystal : public CReactionObject
{
public:
	CAbbotCrystal(WPuppet* pPuppet); // 005f0290
	virtual ~CAbbotCrystal() { } // deleting 005f08b0

	virtual void Reset(); // 005f0b70
	virtual void OnHit(); // 005f0c20
};

// 0x34 bytes; vtable 00d12f00 [DisplayShadow, Display, DebugDisplay, dtor, Process983]. Uma área "fill" do mapa: uma grade de
// objetos (cristais) espalhados pela caixa ou pelo polígono, com deslocamento ao acaso; a bola que passa estoura os que toca.
// Registrada no CSceneManager como "오브젝트 채움 영역" (área de preenchimento de objetos).
class CReactionObjectArea : public CRenderFuncPtr
{
public:
	CReactionObjectArea() // 005f1910 (fora de linha)
		: m_jitterX(15),
		  m_jitterZ(12)
	{
		m_box.Clear();
		m_objects.clear();
	}
	virtual ~CReactionObjectArea(); // 005f19d0; deleting 005f1db0

	virtual void Display(); // 005f1350
protected:
	virtual void DebugDisplay(); // 005f1390
public:
	virtual void Process983(float delta); // 005f1790

	bool IsInPolygon983(const std::vector<WVector>& poly, const WVector& pos); // 005f08e0 (this não usado)
	void CheckHit983(); // 005f13e0
	WPuppet* LoadPuppet983(int type); // 005f1aa0 (this não usado)
	CReactionObject* CreateObject983(int type, WPuppet* pPuppet); // 005f1b90 (this não usado)
	void MakeGrid983(WPuppet* pPuppet, float density, std::vector<WVector>* pOut); // 005f1de0
	void MakeGridInPolygon983(const std::vector<WVector>& poly, WPuppet* pPuppet, float density,
		std::vector<WVector>* pOut); // 005f23b0
	void InitBox983(const Waabb& box, int type, float density, bool unused983); // 005f2470
	void InitPolygon983(const std::vector<WVector>& poly, int type, float density, bool unused983); // 005f2590

protected:
	Waabb m_box; // +0x04
	std::vector<CReactionObject*> m_objects; // +0x1c
	int m_jitterX; // +0x2c (15 deslocamentos ao acaso em x)
	int m_jitterZ; // +0x30 (12 em z)
};

// 0x28 bytes; vtable 00d12f5c, RTTI 00e78c90 (inicializadores 00cd2170/00cd2190, fábrica 005f3140). O ator que monta as áreas
// a partir do mapa: áreas do tipo 8 com "*t"/"*w" no nome (caixa) e nós do tipo 0xe com "fill" no nome (polígono).
class CReactionObjectMng : public IActor
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 005f3000

	CReactionObjectMng() { }
	virtual ~CReactionObjectMng(); // 005f3010; deleting 005f30a0

	virtual void OnInit(); // 005f34a0
	virtual void OnProcess(float delta); // 005f2ee0
	virtual void OnDestroy() { Clear983(); } // 005f32c0

protected:
	virtual void HandleMsg(const MsgObject& msg); // 005f27e0

public:
	void Clear983(); // 005f31f0
	void AddBoxArea983(const Waabb& box, const char* name); // 005f32d0
	void AddPolygonArea983(const std::vector<WVector>& poly, const char* name); // 005f33b0

protected:
	std::vector<CReactionObjectArea*> m_areas; // +0x18
};
