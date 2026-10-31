// JP 983: pincéis da Refresh (B154, F3.7.13 item (8)): a fábrica RFBrush::Create (0042a8b0, logo depois dos controles) e os pincéis
// de imagem (0045a7f0–0045b980: RFSingleBrush, RFPaintBrush::Draw, RFFullFrameBrush, RFPatchBrush). É o fundo das janelas RFWndEx
// (rfwndex.cpp). Pseudo-C do Ghidra em out/F3.7/s154/ghidra_brush.c.
#include <string>
#include "refresh/lib/rfbrush.h"
#include "refresh/rfimageset.h"
#include "refresh/rfwndstruct.h"

extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// 0042a8b0: pelo tipo do conjunto de imagem (sem conjunto ou tipo 0 = sem pincel)
RFBrush* RFBrush::Create(ImageSet* imageSet)
{
	if (imageSet)
	{
		switch (imageSet->GetType())
		{
		case 0:
			return 0;
		case 1:
			return new RFSingleBrush(static_cast<SingleImageSet*>(imageSet));
		case 2:
			return new RFFullFrameBrush(static_cast<FrameImageSet*>(imageSet));
		case 3:
			return new RFPatchBrush(static_cast<NinePatchImageSet*>(imageSet));
		case 4:
			return new RFPaintBrush;
		}
	}
	return new RFNullBrush;
}

// 0045a7f0
RFSingleBrush::RFSingleBrush(const SingleImageSet* imageSet)
	: _texture(0)
{
	_texture = g_pRFRenderer983->CreateTexture(imageSet->_path);
	_tileMode = (TileModeTypes::Type)imageSet->_tileMode;
}

// 0045a8e0
RFSingleBrush::~RFSingleBrush()
{
	if (_texture)
		g_pRFRenderer983->DeleteTexture(_texture);
}

// 0045a950
void RFSingleBrush::Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth)
{
	g_pRFRenderer983->Draw(_texture, pos, size, alpha, depth, _tileMode);
}

// 0045a990
RFSize RFSingleBrush::GetImageSize() const
{
	if (_texture)
		return RFSize((float)_texture->GetWidth(), (float)_texture->GetHeight());
	return RFSize(0.0f, 0.0f);
}

// 0045aa00: retângulo preto com o alfa e, 1 pixel para dentro, o de cor 0x225982
void RFPaintBrush::Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth)
{
	unsigned int color = (int)(alpha * 255.0) << 24;
	g_pRFRenderer983->FillRectangle(color, pos.x, pos.y, size.width, size.height, depth);
	g_pRFRenderer983->FillRectangle(color | 0x225982, (float)(pos.x + 1.0), (float)(pos.y + 1.0), (float)(size.width - 2.0),
		(float)(size.height - 2.0), depth);
}

// 0045aaf0
RFFullFrameBrush::RFFullFrameBrush(const FrameImageSet* imageSet)
	: _background(0), _edgeLT(0), _edgeLB(0), _edgeRT(0), _edgeRB(0), _frameT(0), _frameB(0), _frameL(0), _frameR(0)
{
	_edgeLT = g_pRFRenderer983->CreateTexture(imageSet->_edgeLT._file);
	_frameT = g_pRFRenderer983->CreateTexture(imageSet->_frameT._file);
	_edgeRT = g_pRFRenderer983->CreateTexture(imageSet->_edgeRT._file);
	_frameL = g_pRFRenderer983->CreateTexture(imageSet->_frameL._file);
	_background = g_pRFRenderer983->CreateTexture(imageSet->_background._file);
	_frameR = g_pRFRenderer983->CreateTexture(imageSet->_frameR._file);
	_edgeLB = g_pRFRenderer983->CreateTexture(imageSet->_edgeLB._file);
	_frameB = g_pRFRenderer983->CreateTexture(imageSet->_frameB._file);
	_edgeRB = g_pRFRenderer983->CreateTexture(imageSet->_edgeRB._file);
	_tileT = (TileModeTypes::Type)imageSet->_frameT._tileMode;
	_tileB = (TileModeTypes::Type)imageSet->_frameB._tileMode;
	_tileL = (TileModeTypes::Type)imageSet->_frameL._tileMode;
	_tileR = (TileModeTypes::Type)imageSet->_frameR._tileMode;
	_tileBG = (TileModeTypes::Type)imageSet->_background._tileMode;
}

// 0045ac70
RFFullFrameBrush::~RFFullFrameBrush()
{
	RFTexture** textures = &_background;
	for (int i = 0; i < 9; i++)
	{
		if (textures[i])
		{
			g_pRFRenderer983->DeleteTexture(textures[i]);
			textures[i] = 0;
		}
	}
}

// 0045acf0: cantos no tamanho da textura, bordas e fundo esticados (ou repetidos) no resto. No 983 a borda esquerda usa o modo da
// direita (+0x34; o +0x30 só é gravado), mantido.
void RFFullFrameBrush::Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth)
{
	TileModeTypes::Type stretch;
	RFSize part(0.0f, 0.0f);
	RFVector2 at = pos;
	if (_edgeLT)
	{
		part.width = (float)_edgeLT->GetWidth();
		part.height = (float)_edgeLT->GetHeight();
		stretch = TileModeTypes::Stretch;
		g_pRFRenderer983->Draw(_edgeLT, at, part, alpha, depth, stretch);
	}
	if (_frameT)
	{
		at.x = part.width + at.x;
		part.width = (size.width - part.width) - (float)_edgeRT->GetWidth();
		part.height = (float)_frameT->GetHeight();
		g_pRFRenderer983->Draw(_frameT, at, part, alpha, depth, _tileT);
	}
	if (_edgeRT)
	{
		at.x = at.x + part.width;
		part.width = (float)_edgeRT->GetWidth();
		part.height = (float)_edgeRT->GetHeight();
		stretch = TileModeTypes::Stretch;
		g_pRFRenderer983->Draw(_edgeRT, at, part, alpha, depth, stretch);
	}
	if (_frameL)
	{
		at.x = pos.x;
		at.y = at.y + part.height;
		part.width = (float)_frameL->GetWidth();
		part.height = (size.height - (float)_edgeLT->GetHeight()) - (float)_edgeLB->GetHeight();
		g_pRFRenderer983->Draw(_frameL, at, part, alpha, depth, _tileR);
	}
	if (_background)
	{
		at.x = part.width + at.x;
		part.width = (size.width - part.width) - (float)_frameR->GetWidth();
		g_pRFRenderer983->Draw(_background, at, part, alpha, depth, _tileBG);
	}
	if (_frameR)
	{
		at.x = at.x + part.width;
		part.width = (float)_frameR->GetWidth();
		g_pRFRenderer983->Draw(_frameR, at, part, alpha, depth, _tileR);
	}
	if (_edgeLB)
	{
		at.x = pos.x;
		at.y = at.y + part.height;
		part.width = (float)_edgeLB->GetWidth();
		part.height = (float)_edgeLB->GetHeight();
		stretch = TileModeTypes::Stretch;
		g_pRFRenderer983->Draw(_edgeLB, at, part, alpha, depth, stretch);
	}
	if (_frameB)
	{
		at.x = part.width + at.x;
		part.width = (size.width - part.width) - (float)_edgeRB->GetWidth();
		g_pRFRenderer983->Draw(_frameB, at, part, alpha, depth, _tileB);
	}
	if (_edgeRB)
	{
		at.x = at.x + part.width;
		part.width = (float)_edgeRB->GetWidth();
		stretch = TileModeTypes::Stretch;
		g_pRFRenderer983->Draw(_edgeRB, at, part, alpha, depth, stretch);
	}
}

// 0045b200
RFPatchBrush::RFPatchBrush(const NinePatchImageSet* imageSet)
	: _texture(0)
{
	_texture = g_pRFRenderer983->CreateTexture(imageSet->_path);
	// "ControlPoint" na ordem do XML (os nomes do leitor não batem com o uso: 1º e 2º são linhas horizontais)
	SetControlPoints(imageSet->_left, imageSet->_top, imageSet->_right, imageSet->_bottom);
}

// 0045b390
RFPatchBrush::~RFPatchBrush()
{
	if (_texture)
		g_pRFRenderer983->DeleteTexture(_texture);
}

// 0045b830
void RFPatchBrush::SetControlPoints(float top, float bottom, float left, float right)
{
	if (_texture)
	{
		float w = (float)_texture->GetWidth();
		float h = (float)_texture->GetHeight();
		_texelU = 1.0f / w;
		_texelV = 1.0f / h;
		_outerUV[0].x = _texelU;
		_outerUV[0].y = _texelV;
		float u1 = (w - 1.0f) / w;
		_outerUV[1].x = u1;
		_outerUV[1].y = _texelV;
		_outerUV[2].x = _texelU;
		float v1 = (h - 1.0f) / h;
		_outerUV[2].y = v1;
		_outerUV[3].y = v1;
		_outerUV[3].x = u1;
		_innerUV[0].x = left / w;
		_innerUV[0].y = top / h;
		_innerUV[1].x = right / w;
		_innerUV[1].y = top / h;
		_innerUV[2].x = left / w;
		_innerUV[2].y = bottom / h;
		_innerUV[3].y = bottom / h;
		_innerUV[3].x = right / w;
		_cornerSize[0].x = left - 1.0f;
		_cornerSize[0].y = top - 1.0f;
		_cornerSize[1].x = left - 1.0f;
		_cornerSize[1].y = (h - bottom) - 1.0f;
		_cornerSize[2].x = (w - right) - 1.0f;
		_cornerSize[3].x = _cornerSize[2].x;
		_cornerSize[2].y = top - 1.0f;
		_cornerSize[3].y = _cornerSize[1].y;
	}
}

// 0045b400: 3 × 3 pedaços (cantos no tamanho dos pontos de controle; o meio com 1 texel de folga)
void RFPatchBrush::Draw(const RFVector2& pos, const RFSize& size, float alpha, float depth)
{
	TileModeTypes::Type stretch = TileModeTypes::Stretch;
	RFVector2 at = pos;
	RFSize part(_cornerSize[0].x, _cornerSize[0].y);
	g_pRFRenderer983->Draw(_texture, _outerUV[0].x, _outerUV[0].y, _innerUV[0].x, _innerUV[0].y, at, part, alpha, depth, stretch);
	at.x = part.width + at.x;
	part.width = (size.width - part.width) - _cornerSize[2].x;
	part.height = _cornerSize[0].y;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _innerUV[0].x, _outerUV[0].y, _innerUV[1].x, _innerUV[1].y, at, part, alpha, depth, stretch);
	at.x = at.x + part.width;
	part.width = _cornerSize[2].x;
	part.height = _cornerSize[2].y;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _innerUV[1].x, _outerUV[1].y, _outerUV[1].x, _innerUV[1].y, at, part, alpha, depth, stretch);

	at.x = pos.x;
	at.y = at.y + part.height;
	part.width = _cornerSize[0].x;
	part.height = (size.height - _cornerSize[0].y) - _cornerSize[1].y;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _outerUV[0].x, _innerUV[0].y, _innerUV[2].x, _innerUV[2].y, at, part, alpha, depth, stretch);
	at.x = part.width + at.x;
	part.width = (size.width - part.width) - _cornerSize[2].x;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _texelU + _innerUV[0].x, _innerUV[0].y + _texelV, _innerUV[3].x - _texelU, _innerUV[3].y - _texelV,
		at, part, alpha, depth, stretch);
	at.x = at.x + part.width;
	part.width = _cornerSize[2].x;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _innerUV[1].x, _innerUV[1].y, _outerUV[1].x, _innerUV[3].y, at, part, alpha, depth, stretch);

	at.x = pos.x;
	at.y = at.y + part.height;
	part.width = _cornerSize[1].x;
	part.height = _cornerSize[1].y;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _outerUV[2].x, _innerUV[2].y, _innerUV[2].x, _outerUV[2].y, at, part, alpha, depth, stretch);
	at.x = part.width + at.x;
	part.width = (size.width - part.width) - _cornerSize[3].x;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _innerUV[2].x, _innerUV[2].y, _innerUV[3].x, _outerUV[3].y, at, part, alpha, depth, stretch);
	at.x = at.x + part.width;
	part.width = _cornerSize[3].x;
	stretch = TileModeTypes::Stretch;
	g_pRFRenderer983->Draw(_texture, _innerUV[3].x, _innerUV[3].y, _outerUV[3].x, _outerUV[3].y, at, part, alpha, depth, stretch);
}
