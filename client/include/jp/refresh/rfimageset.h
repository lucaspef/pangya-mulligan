#pragma once
// JP 983: conjuntos de imagem da Refresh (RTTI .?AVImageSet@@ 00d641e0, .?AVSingleImageSet@@ 00d641ec,
// .?AVNinePatchImageSet@@ 00d641f8; a FrameImageSet fica no rfwndstruct.h). O RFXmlLoader os cria a partir do <Textures>
// do XML (rfxmlloader.cpp) e os guarda pelo nome com contagem de referências. Slot 1 = tipo (0 base, 1 Single, 2 Frame,
// 3 NinePatch). Destrutores e GetType no 983 em 0041e1c0…0041e360 (perto dos controles; aqui em linha). Campos meus. B147.
#include <string>
#include "refresh/rfreferencecount.h"

class ImageSet : public RFReferenceCount
{
public:
	virtual ~ImageSet() {} // ??_G 0041e1d0
	virtual int GetType() const { return 0; } // 0041e1c0
};

// uma imagem (0x28; construtor embutido no leitor 0043f010)
class SingleImageSet : public ImageSet
{
public:
	SingleImageSet() : _tileMode(0) {}
	virtual ~SingleImageSet() {} // ??_G 0041e270
	virtual int GetType() const { return 1; } // 0041e260

	std::wstring _path; // +0x08
	int _tileMode; // +0x24 (0 "Stretch", 1 "Tile")
};

// imagem esticada pelos pontos de controle (0x34; construtor embutido no leitor 0043f290)
class NinePatchImageSet : public ImageSet
{
public:
	NinePatchImageSet() {}
	virtual ~NinePatchImageSet() {} // ??_G 0041e360
	virtual int GetType() const { return 3; } // 0041e350

	std::wstring _path; // +0x08
	float _left; // +0x24 ("ControlPoint" = 4 números separados por vírgula; não zerados)
	float _top; // +0x28
	float _right; // +0x2c
	float _bottom; // +0x30
};
