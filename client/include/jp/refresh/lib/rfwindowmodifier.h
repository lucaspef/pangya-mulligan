#pragma once
#include <string>
#include "refresh/lib/rftypes.h"

// JP 983: RFVisualEffect.h e RFWindowModifier.h da Refresh (cópias fora de linha 00404490–00404830, emitidas pelo RFWnd.cpp).

// efeito visual da janela (vtable 00d630c4: 3 virtuais puras e o destrutor no slot 3; nomes das puras meus). O RFWnd::SetVisualEffect
// aceita também um número < 0x10 no lugar do ponteiro: efeito pronto da fábrica CreateVisualEffect983 (0042cfa0, tipos 0–2).
class RF_API RFVisualEffect
{
public:
	RFVisualEffect() { } // 004044a0
	virtual RFVisualEffect* Clone983() = 0; // slot 0 (SetVisualEffect guarda a cópia)
	virtual void Begin983(class RFWnd* wnd) = 0; // slot 1 (ShowWindow/Close: começa a abrir ou a fechar)
	virtual void Update983(class RFWnd* wnd, float delta) = 0; // slot 2 (OnUpdate)
	virtual ~RFVisualEffect() { } // 00404490 (slot 3)
};

// 0042cfa0 (RFVisualEffect.cpp; cria o efeito pronto 0, 1 ou 2; o 983 passa o tipo na pilha; nome meu)
RFVisualEffect* __stdcall CreateVisualEffect983(int type);

// modificador de janela por nome (0x3c; vtable 00d63080)
class RF_API RFWindowModifier
{
public:
	RFWindowModifier() { _visible = false; _alpha = 0.0f; } // 00404560
	virtual ~RFWindowModifier() { } // 00404590 (slot 0)
	virtual const std::wstring& GetName() const { return _name; } // 004045d0
	virtual void SetName(const std::wstring& name) { _name.assign(name, 0, std::wstring::npos); } // 004045e0
	virtual float GetAlpha() const { return _alpha; } // 00404600
	virtual void SetAlpha(float alpha) { _alpha = alpha; } // 00404610
	virtual bool IsVisible() const { return _visible; } // 00404620
	virtual void SetVisible(bool visible) { _visible = visible; } // 00404630
	virtual const RFVector2& GetPos() const { return _pos; } // 00404640
	virtual void SetPos(const RFVector2& pos) { _pos = pos; } // 00404650
	virtual const RFSize& GetSize() const { return _size; } // 00404670
	virtual void SetSize(const RFSize& size) { _size = size; } // 00404680

	std::wstring _name; // +0x04
	float _alpha; // +0x20
	bool _visible; // +0x24
	RFVector2 _pos; // +0x28
	RFSize _size; // +0x30
	int _38; // +0x38 (o construtor não grava; a cópia copia)
};
