#pragma once
#include <string>
#include "rftypes.h"

// JP 983: biblioteca Refresh (RF, 00401000–0046d8d0, LTCG; fonte original em
// E:\program_code\server_client_launcher\trunk\code\client\Refresh). Só o que as unidades de ligação do ProjectG
// (refreshinterface.cpp, …) usam. Nomes de método da RF tirados do 839 (Ghidra); os sem nome são meus (sufixo 983).

// estilo de texto que a RF passa ao desenho/medida (só os campos lidos pela ligação; nome meu)
struct RFTextStyle983
{
	std::wstring name; // +0x00 (A822: era char[0x1c] = wstring do VC10; no cl 14.44 o wstring tem 0x18 e size/cores andavam 4:
	                   //  texto do label_channel_name escuro e fonte pequena no exe VS 2022). Espelho do FontInfo (rfcontrols.h)
	int size; // +0x1c: tamanho da fonte (< 12 = fonte pequena, "…11" do FrGraphicInterface)
	unsigned long colors[4]; // +0x20: style (flags), color, outlineColor, shadowColor do FontInfo
};

// texto vazio (inicializador por unidade: 00cd21c0 → 00e13088 na refreshinterface, 00cd21e0 → 00e130c4 na refreshrenderer)
static const std::wstring s_rfEmptyText983(L"");

// interface que o jogo implementa para a RF (RTTI RFInterface, vtable 00d12fc4; slots 0–3 puros)
class RFInterface
{
public:
	virtual void PlaySound983(const std::wstring& name) = 0; // slot 0
	virtual void DrawText983(const RFTextStyle983& style, int x, int y, const std::wstring& text, int unused) = 0; // slot 1
	virtual RFSize GetTextExtent983(const RFTextStyle983& style, const std::wstring& text) = 0; // slot 2
	virtual void Slot3_983(int a, int b) = 0; // slot 3 (a RefreshInterface não faz nada)
	virtual std::wstring GetBasePath(int unused); // 0042ac40 (slot 4)
	virtual void* FOpen(const std::wstring& path); // 0042ac80 (_wfopen_s "rb")
	virtual void FClose(void* file); // 0042acb0
	virtual long FTell(void* file); // 0042acd0
	virtual int FSeek(void* file, long offset, int origin); // 0042acf0
	virtual unsigned int FRead(void* file, void* buffer, unsigned int size); // 0042ad10
	virtual std::wstring TranslateText(const std::wstring& text); // 0042ad40
	virtual void OnKeyboardAcquired(int arg); // 00403660
	virtual void OnInputMethodStateChanged(int arg); // 00403670
	virtual void DebugPrint(const wchar_t* text); // 0042ad80
	virtual ~RFInterface() { } // 00403680 (na RF); cópia fora de linha em cada unidade de ligação (005f4f70)
};
