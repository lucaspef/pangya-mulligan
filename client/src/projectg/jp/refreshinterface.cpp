// JP 983: unidade 005f4ea0–005f5340 (RefreshInterface, a RFInterface do jogo). O mapa da F3.7.1 a punha em "lojas_pontos"
// (string "Premium service" da vizinha); é ligação da biblioteca Refresh. 005f4e60 (seção crítica) é da unidade anterior.
// Inicializador 00cd21c0 = o s_rfEmptyText983 do rfinterface.h.
#include "refreshinterface.h"
#include "refresh/rftextmanager.h"
#include "engine/cfile.h"
#include "engine/wresrcmng.h"
#include "soundmanager.h"

// 005e4d60 (unidade atl.cpp 005e49c0–005e51f0, sem corpo): wstring → string na página 932
std::string WideToMultiByte983(const std::wstring& text);

// 005f4ea0
void RefreshInterface::FClose(void* file)
{
	CloseCFile((cFile*)file);
}

// 005f4ec0
long RefreshInterface::FTell(void* file)
{
	return ((cFile*)file)->Tell();
}

// 005f4ee0
int RefreshInterface::FSeek(void* file, long offset, int origin)
{
	((cFile*)file)->Seek(offset, origin);
	return 0;
}

// 005f4f00
unsigned int RefreshInterface::FRead(void* file, void* buffer, unsigned int size)
{
	((cFile*)file)->Read(buffer, size);
	return 0;
}

// 005f4f50
RefreshInterface::RefreshInterface()
{
}

// 005f4f60
void RefreshInterface::Slot3_983(int a, int b)
{
}

// 005f4f70 (fora de linha: o destrutor de apagar 005f5060 o chama)
RefreshInterface::~RefreshInterface()
{
}

// A822: o estilo e o FontInfo da RF (rfcontrols.h) por referencia: size logo apos o wstring, cores depois (vale nos 2 cl)
C_ASSERT(offsetof(RFTextStyle983, size) == sizeof(std::wstring) && offsetof(RFTextStyle983, colors) == sizeof(std::wstring) + 4);

// 005f4f80
void RefreshInterface::DrawText983(const RFTextStyle983& style, int x, int y, const std::wstring& text, int unused)
{
	if (text.size() == 0)
		return;
	RFTextManager* pText = RFTextManager::Instance();
	if (!pText)
		return;
	if (style.size <= 11)
		pText->DrawText11_983((float)(x - 1.0), (float)y, text, style.colors[0], style.colors[1], style.colors[2], style.colors[3]);
	else
		pText->DrawText983((float)(x - 1.0), (float)y, text, style.colors[0], style.colors[1], style.colors[2], style.colors[3]);
}

// 005f5090
RFSize RefreshInterface::GetTextExtent983(const RFTextStyle983& style, const std::wstring& text)
{
	if (RFTextManager::Instance())
		return RFTextManager::Instance()->GetTextExtent983(style, text);
	return RFSize(0.0f, 0.0f);
}

// 005f50e0
void* RefreshInterface::FOpen(const std::wstring& path)
{
	std::string name = WideToMultiByte983(path);
	return g_resrcmng->GetCFile(name.c_str(), 0xffff);
}

// 005f5170
std::wstring RefreshInterface::GetBasePath(int unused)
{
	return s_rfEmptyText983;
}

// 005f51b0 (som da RF: o nome vem com ".wav", o g_audio quer sem)
void RefreshInterface::PlaySound983(const std::wstring& name)
{
	if (g_audio && name.size() != 0)
	{
		std::string sound = WideToMultiByte983(name.c_str());
		size_t pos = sound.find(".wav");
		if (pos != std::string::npos)
			sound.assign(sound.begin(), sound.begin() + pos);
		g_audio->PlaySfx(sound.c_str());
	}
}
