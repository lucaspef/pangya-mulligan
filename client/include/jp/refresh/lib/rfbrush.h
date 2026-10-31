#pragma once
#include "refresh/lib/rftypes.h"
#include "refresh/lib/rfrenderer.h"

// JP 983: pincéis da Refresh (F3.7.13 item (8), B154): o fundo de uma janela feito de um ImageSet (rfimageset.h / rfwndstruct.h).
// RTTI .?AVRFBrush@@ (vtable 00d64794) e as derivadas; slot 0 = destrutor de apagar, 1 = Draw, 2 = tamanho natural. A fábrica
// 0042a8b0 escolhe pelo tipo do ImageSet (slot 1): 1 Single, 2 Frame, 3 NinePatch, 4 Paint, 0 = nenhum, outro = Null.
// Os pincéis de imagem (0045a7f0–0045b980) pedem as texturas ao desenhista global (00f1de20). Nomes dos métodos meus.
class ImageSet;
class SingleImageSet;
class FrameImageSet;
class NinePatchImageSet;

class RFBrush
{
public:
	virtual ~RFBrush() { } // 0042a710 / 0042a790 (só a vtable)
	virtual void Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth) = 0;
	virtual RFSize GetImageSize() const = 0;

	static RFBrush* Create(ImageSet* imageSet); // 0042a8b0
};

// não desenha (vtable 00d647a4; tipo desconhecido)
class RFNullBrush : public RFBrush
{
public:
	virtual void Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth) { } // 0042a720
	virtual RFSize GetImageSize() const { return RFSize(0.0f, 0.0f); } // 0042a730
};

// moldura de 1 pixel colorida (vtable 00d647b4; tipo 4: Draw 0045aa00)
class RFPaintBrush : public RFBrush
{
public:
	virtual void Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth); // 0045aa00
	virtual RFSize GetImageSize() const { return RFSize(0.0f, 0.0f); } // 0042a7a0
};

// uma textura (0xc; vtable 00d65e4c; construtor 0045a7f0, destrutor 0045a8e0)
class RFSingleBrush : public RFBrush
{
public:
	RFSingleBrush(const SingleImageSet* imageSet); // 0045a7f0
	virtual ~RFSingleBrush(); // 0045a8e0
	virtual void Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth); // 0045a950
	virtual RFSize GetImageSize() const; // 0045a990

	RFTexture* _texture; // +0x04
	TileModeTypes::Type _tileMode; // +0x08
};

// moldura em 9 partes (0x3c; vtable 00d65e5c; construtor 0045aaf0, destrutor 0045ac70)
class RFFullFrameBrush : public RFBrush
{
public:
	RFFullFrameBrush(const FrameImageSet* imageSet); // 0045aaf0
	virtual ~RFFullFrameBrush(); // 0045ac70
	virtual void Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth); // 0045acf0
	virtual RFSize GetImageSize() const { return RFSize(0.0f, 0.0f); } // 0045aad0

	RFTexture* _background; // +0x04
	RFTexture* _edgeLT; // +0x08
	RFTexture* _edgeLB; // +0x0c
	RFTexture* _edgeRT; // +0x10
	RFTexture* _edgeRB; // +0x14
	RFTexture* _frameT; // +0x18
	RFTexture* _frameB; // +0x1c
	RFTexture* _frameL; // +0x20
	RFTexture* _frameR; // +0x24
	TileModeTypes::Type _tileT; // +0x28
	TileModeTypes::Type _tileB; // +0x2c
	TileModeTypes::Type _tileL; // +0x30
	TileModeTypes::Type _tileR; // +0x34
	TileModeTypes::Type _tileBG; // +0x38
};

// imagem esticada pelos pontos de controle do NinePatchImageSet (0x70; vtable 00d65e6c; construtor 0045b200, destrutor 0045b390):
// 9 pedaços da mesma textura pelo Draw de coordenadas de textura (slot 3 do desenhista)
class RFPatchBrush : public RFBrush
{
public:
	RFPatchBrush(const NinePatchImageSet* imageSet); // 0045b200
	virtual ~RFPatchBrush(); // 0045b390
	virtual void Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth); // 0045b400
	virtual RFSize GetImageSize() const { return RFSize(0.0f, 0.0f); } // 0045b1c0

	// coordenadas e medidas pelo tamanho da textura (0045b830; as 4 linhas de corte em pixels da imagem)
	void SetControlPoints(float top, float bottom, float left, float right);

	RFTexture* _texture; // +0x04
	float _texelU; // +0x08 1/largura
	float _texelV; // +0x0c 1/altura
	RFVector2 _outerUV[4]; // +0x10 cantos de fora (EC, DC, EB, DB), 1 texel para dentro
	RFVector2 _innerUV[4]; // +0x30 cantos de dentro (pontos de controle)
	RFVector2 _cornerSize[4]; // +0x50 tamanho em pixels dos cantos: EC, EB, DC, DB
};
