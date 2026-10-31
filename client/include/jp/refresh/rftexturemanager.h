#pragma once
#include <string>
#include <map>
#include "engine/singleton.h"

class Bitmap;
class WOverlay;

// JP 983: cache de texturas da RF (WSingleton<RFTextureManager>, instância 00e78ca0, vtable 00d13d00; unidade
// rftexturemanager.cpp 00601ae0–00602490). Criado por 00a65710 (projectg.cpp da A). Nome por arquivo; o valor guarda o
// Bitmap (LoadBitmap983, sem chamador no 983) ou o WOverlay (Find983 → LoadOverlay983) e o tipo da imagem.
class RFTextureManager : public WSingleton<RFTextureManager>
{
public:
	// nó do map 0x54: chave +0xc, valor +0x28
	struct sTexture983
	{
		sTexture983() : bitmap(0), overlay(0) { }

		int type; // +0x0: 2 = .png, 1 = .tga, 0 = outro (GetImageType983)
		std::string name; // +0x4
		Bitmap* bitmap; // +0x20
		WOverlay* overlay; // +0x24 (devolvido com WResourceManager::Release)
	};

	RFTextureManager(); // 00601dc0
	virtual ~RFTextureManager(); // 00601ae0 (??_G 00601e80)

	Bitmap* LoadBitmap983(const std::string& name); // 00601eb0
	WOverlay* LoadOverlay983(const std::string& name); // 006020d0
	WOverlay* Find983(const std::string& name); // 006022f0

	std::map<std::string, sTexture983> m_textures; // +0x28 (cabeça +0x2c, tamanho +0x30)
};
