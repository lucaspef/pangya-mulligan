#pragma once
#include "refresh/rfinterface.h"

// JP 983: implementação da RFInterface pelo jogo (RTTI RefreshInterface, vtable 00d13004, 4 bytes — `push 4` do 00604160,
// que guarda o objeto em +0x2c do gerente de cena). Som pelo g_audio, texto pelo RFTextManager, arquivos pelo
// WResourceManager (cFile). Unidade 005f4ea0–005f5340 (refreshinterface.cpp).
class RefreshInterface : public RFInterface
{
public:
	RefreshInterface(); // 005f4f50
	virtual ~RefreshInterface(); // 005f4f70 (destrutor de apagar 005f5060)

	virtual void PlaySound983(const std::wstring& name); // 005f51b0
	virtual void DrawText983(const RFTextStyle983& style, int x, int y, const std::wstring& text, int unused); // 005f4f80
	virtual RFSize GetTextExtent983(const RFTextStyle983& style, const std::wstring& text); // 005f5090
	virtual void Slot3_983(int a, int b); // 005f4f60
	virtual std::wstring GetBasePath(int unused); // 005f5170
	virtual void* FOpen(const std::wstring& path); // 005f50e0
	virtual void FClose(void* file); // 005f4ea0
	virtual long FTell(void* file); // 005f4ec0
	virtual int FSeek(void* file, long offset, int origin); // 005f4ee0
	virtual unsigned int FRead(void* file, void* buffer, unsigned int size); // 005f4f00
};
