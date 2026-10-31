#pragma once
#include "refresh/rfrenderer.h"

class WOverlay;

// JP 983: dados da RefreshTexture (+0x4, 0x3c bytes). O construtor é embutido no 00603160; o 983 tem também a cópia fora de
// linha 00603130 (sem chamador), que o nosso cl não emite.
struct sRefreshTextureInfo983
{
	sRefreshTextureInfo983() // 00603130
	{
		m_src[0] = 0.0f;
		m_src[1] = 0.0f;
		m_src[2] = 0.0f;
		m_owner = 0;
		m_src[3] = 0.0f;
		m_pOverlay = NULL;
		m_scale[0] = 1.0f;
		m_scale[1] = 1.0f;
		m_scale[2] = 1.0f;
		m_30 = 0;
		m_alpha = 0xff;
	}

	int m_owner; // +0x00 (o +4 do RefreshRenderer)
	WOverlay* m_pOverlay; // +0x04
	float m_src[4]; // +0x08: 0, 0, largura, altura do overlay
	float m_scale[3]; // +0x18: 1, 1, 1
	char m_24[0xc]; // +0x24
	int m_30; // +0x30: 0
	int m_alpha; // +0x34: 0xff
	char m_38[4]; // +0x38
};

// JP 983: textura da RF feita de um WOverlay do jogo (RTTI RefreshTexture, vtable 00d13d44, 0x40 bytes — `push 0x40` do
// RefreshRenderer::CreateTexture 005f5c50). Corpo no começo da unidade rfscenemanager.cpp (006030c0–00603220).
class RefreshTexture : public RFTexture
{
public:
	RefreshTexture(WOverlay* pOverlay, int owner); // 00603160
	virtual ~RefreshTexture(); // 006031e0 (destrutor de apagar 006031f0)
	virtual unsigned long GetWidth(); // 006030c0
	virtual unsigned long GetHeight(); // 006030f0

	sRefreshTextureInfo983 m_info; // +0x04
};
C_ASSERT(sizeof(RefreshTexture) == 0x40);
