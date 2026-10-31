#pragma once
#include <string>
#include "refresh/lib/rftypes.h"

// JP 983: RFInterface.h da Refresh como a biblioteca o vê (exportada; vtable 00d12fc4, slots 0–3 puros; construtor/destrutor/cópia
// fora de linha 00403660–004036f0, emitidos pelo RFWnd.cpp). Nomes dos slots 0–3 meus (o jogo os implementa na RefreshInterface;
// ver a visão refresh/rfinterface.h). Usado pelo RFWnd::PlaySfx/PrintText.
struct FontInfo;

enum RFBASEPATHTYPE { RFBASEPATH_DEFAULT983 = 0 };
struct InputMethodStateTypes
{
	enum Type { CompositionStart983 = 0, CompositionEnd983 = 1, CandidateOpen983 = 2, CandidateClose983 = 3 }; // RFIme (B163)
};

class RF_API RFInterface
{
public:
	RFInterface() { } // 00403690
	virtual void PlaySound983(const std::wstring& name) = 0; // slot 0
	virtual void PrintText983(const FontInfo& font, int x, int y, const std::wstring& text, float depth) = 0; // slot 1 (depth = RFRenderContext::_depth)
	virtual RFSize GetTextExtent983(const FontInfo& font, const std::wstring& text) = 0; // slot 2
	virtual void Slot3_983(int a, int b) = 0; // slot 3
	virtual std::wstring GetBasePath(RFBASEPATHTYPE type); // 0042ac40
	virtual void* FOpen(const std::wstring& path); // 0042ac80
	virtual void FClose(void* file); // 0042acb0
	virtual long FTell(void* file); // 0042acd0
	virtual int FSeek(void* file, long offset, int origin); // 0042acf0
	virtual unsigned int FRead(void* file, void* buffer, unsigned int size); // 0042ad10
	virtual std::wstring TranslateText(const std::wstring& text); // 0042ad40
	virtual void OnKeyboardAcquired(bool acquired) { } // 00403660
	virtual void OnInputMethodStateChanged(InputMethodStateTypes::Type state) { } // 00403670
	virtual void DebugPrint(const wchar_t* text); // 0042ad80
	virtual ~RFInterface() { } // 00403680
};
