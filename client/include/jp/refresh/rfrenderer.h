#pragma once
#include <string>
#include "rftypes.h"

// JP 983: textura da RF (RTTI RFTexture; GetWidth/GetHeight da RF, no 839 00405fc0/00405fd0)
class RFTexture
{
public:
	virtual ~RFTexture() { }
	virtual unsigned long GetWidth(); // slot 1 (00403430: xor eax, eax; a RefreshTexture converte o float com fistp qword)
	virtual unsigned long GetHeight(); // slot 2
};

// JP 983: desenhista da RF (RTTI RFRenderer, vtable 00d1304c; ctor 004035a0, dtor 00403590). Os três Draw são sobrecargas:
// o MSVC as põe na vtable em ordem inversa à da declaração (slot 2 = ret 0xc, 3 = ret 0x28, 4 = ret 0x18).
class RFRenderer
{
public:
	virtual RFTexture* CreateTexture(const std::wstring& name); // 00403510 (slot 0: devolve NULL)
	virtual void DeleteTexture(RFTexture* texture); // 00403520
	virtual void Draw(RFTexture* texture, const RFVector2& pos, const RFSize& size, float alpha, int a5, int a6); // 00403530 (slot 4)
	virtual void Draw(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10); // 00403540 (slot 3)
	virtual void Draw(int a1, int a2, int a3); // 00403550 (slot 2)
	virtual void PushClip983(const RFRect& rect) = 0; // slot 5
	virtual void PopClip983() = 0; // slot 6
	virtual void FillRectangle(unsigned long color, float x, float y, float w, float h, int a6); // 00403560 (slot 7)
	virtual void OnPreResetDevice(); // 00403570
	virtual void OnPostResetDevice(); // 00403580
	virtual ~RFRenderer() { } // 00403590 (na RF); cópia embutida no destrutor da RefreshRenderer
};
