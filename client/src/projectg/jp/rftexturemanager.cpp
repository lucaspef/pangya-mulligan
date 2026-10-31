// JP 983: unidade 00601ae0–00602490 — RFTextureManager, o cache de texturas da RF (map nome → bitmap/overlay). O começo é
// conferido pela cobertura: GetImageType983 (00600a70) e os moldes do map (006008a0…, 00600cd0–00601a80) estão antes do corte
// da F3.7.1, no fim da faixa da rflobbydesktop.
#include "refresh/rftexturemanager.h"
#include "engine/wresrcmng.h"
#include "engine/bitmap.h"
#include "engine/woverlay.h"

extern WResourceManager* g_resrcmng; // 00f02460

// 00600a70: tipo da imagem pela extensão (a posição achada fica em pos)
static int __stdcall GetImageType983(const std::string& name, size_t* pos)
{
	int type = 0;
	std::string file = name;
	if ((*pos = file.find(".png")) != std::string::npos)
		type = 2;
	else if ((*pos = file.find(".tga")) != std::string::npos)
		type = 1;
	return type;
}

// 00601dc0
RFTextureManager::RFTextureManager()
{
}

// 00601ae0
RFTextureManager::~RFTextureManager()
{
	for (std::map<std::string, sTexture983>::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
	{
		if (it->second.bitmap)
		{
			delete it->second.bitmap;
			it->second.bitmap = 0;
		}
		if (g_resrcmng && it->second.overlay)
		{
			g_resrcmng->Release(it->second.overlay);
			it->second.overlay = 0;
		}
	}
	m_textures.clear();
}

// 00601eb0
Bitmap* RFTextureManager::LoadBitmap983(const std::string& name)
{
	if (!g_resrcmng)
		return 0;
	size_t pos = std::string::npos;
	int type = GetImageType983(std::string(name.c_str()), &pos);
	std::string file(name.c_str());
	Bitmap* pBitmap = g_resrcmng->LoadBitmap(name.c_str(), 0, false);
	sTexture983 tex;
	tex.name = name;
	tex.type = type;
	tex.bitmap = pBitmap;
	m_textures[std::string(name.c_str())] = tex;
	return pBitmap;
}

// 006020d0
WOverlay* RFTextureManager::LoadOverlay983(const std::string& name)
{
	if (!g_resrcmng)
		return 0;
	size_t pos = std::string::npos;
	int type = GetImageType983(std::string(name.c_str()), &pos);
	std::string file(name.c_str());
	WOverlay* pOverlay = g_resrcmng->GetOverlay(file.c_str(), 0x80000000);
	sTexture983 tex;
	tex.name = name;
	tex.type = type;
	tex.overlay = pOverlay;
	m_textures[std::string(name.c_str())] = tex;
	return pOverlay;
}

// 006022f0
WOverlay* RFTextureManager::Find983(const std::string& name)
{
	std::map<std::string, sTexture983>::iterator it = m_textures.find(std::string(name.c_str()));
	if (it != m_textures.end())
		return it->second.overlay;
	return LoadOverlay983(std::string(name.c_str()));
}
