#pragma once
// JP 983: estruturas do leitor de layout da Refresh (RTTI .?AUWndBaseStruct@@ e as `*Base`, vtables 00d659e4…00d65ac4;
// .?AVFrameImageSet@@ 00d65af4; Testing::RF*Exception 00d65ad0…). Cada estrutura guarda o que o XML de um controle diz:
// propriedades tipadas (rfproperty.h) registradas pelo nome num mapa sem caixa (+0x16c) e campos lidos à parte. Slot 1 da
// vtable = tipo do controle (0 janela, 1 WndEx, 2 Label, 3 Button, 4 LabelButton, 5 AnimButton, 6 Check, 7 Picture,
// 8 Progress, 9 Scrollbar, 10 Slider, 0xb Edit, 0xc List, 0xd ListSC, 0xe StackPanel, 0xf TextArea, 0x10 TextAreaSC,
// 0x11 Combo, 0x12 Custom). Corpos em jp/rfwndstruct.cpp. Sessão B146 (F3.7.13 item 2). Nomes dos campos sem propriedade e
// das funções meus (a RTTI só dá as classes); campos que nenhum construtor toca ficam com o deslocamento no nome.
#include <exception>
#include <map>
#include <string>
#include "refresh/rfproperty.h"
#include "refresh/rfimageset.h"

namespace Testing
{
	// 00439e90 (a mensagem larga vira texto pelo código de página da Refresh, 0042ab40)
	class RFException : public std::exception
	{
	public:
		explicit RFException(const std::wstring& message);
	};

	// 00439f60
	class RFInvalidArgException : public std::exception
	{
	public:
		explicit RFInvalidArgException(const std::wstring& message);
	};

	// 0043a030: + código em +0xc
	class RFFileException : public std::exception
	{
	public:
		RFFileException(const std::wstring& message, int code);
	protected:
		int _code; // +0x0c
	};
}

// 0043a110 / 0043a1d0 / 0043a2c0
void RFThrowException983(const std::wstring& message);
void RFThrowInvalidArgException983(const std::wstring& message);
void RFThrowFileException983(const std::wstring& message, int code);

// 0042ab40: wstring → texto no código de página da Refresh (outra unidade)
std::string RFWideToMulti983(const std::wstring& text);

// comparação do mapa de propriedades (00439db0): primeiro o tamanho, depois _stricmp
struct RFPropertyNameLess983
{
	bool operator()(const std::string& left, const std::string& right) const;
};

// imagem em 9 partes (0x128; ctor 0043a370, dtor 0043a440): 9 × { arquivo, modo }. Nomes pelo <asset type> do leitor antigo
// (RFXmlLoader::ReadWndEx 00442070: BG +0x08, EDGE_LT/RT/LB/RB +0x28…+0x88, FRAME_L/R/T/B +0xa8…+0x108; B148). O leitor de
// <Texture Type="Frame"> (0043eb30) preenche na ordem dos <Image>: BG, LT, LB, RT, RB, T, B, L, R
class FrameImageSet : public ImageSet
{
public:
	struct Part
	{
		Part() : _tileMode(0) {}
		std::wstring _file; // +0x00
		int _tileMode; // +0x1c (0 "Stretch", 1 "Tile")
	};

	FrameImageSet();
	virtual ~FrameImageSet();
	virtual int GetType() const { return 2; } // 0043a5c0

	Part _background; // +0x08 "BG"
	Part _edgeLT; // +0x28
	Part _edgeRT; // +0x48
	Part _edgeLB; // +0x68
	Part _edgeRB; // +0x88
	Part _frameL; // +0xa8
	Part _frameR; // +0xc8
	Part _frameT; // +0xe8
	Part _frameB; // +0x108
};

// retângulo do XML (<… l t r b>; 4 floats zerados no construtor, fldz/fst no 983). Ordem do 983: esquerda, direita, cima,
// baixo (RFReadRect983 grava l +0, r +4, t +8, b +0xc; Width() = +4 − +0 no assert "base->_itemrect.Width() > 0", B148)
struct RFRect983
{
	RFRect983() : left(0.0f), right(0.0f), top(0.0f), bottom(0.0f) {}
	float Width() const { return right - left; }
	float Height() const { return bottom - top; }
	float left, right, top, bottom;
};

// registra uma propriedade embutida pelo nome (no 983 o AddProperty sai em linha, com o teste de nulo)
#define RF_REGISTER_PROPERTY983(property, name) RegisterProperty(&(property), name)

// janela (0x17c; ctor 004394c0, dtor 00439a80)
struct WndBaseStruct
{
	typedef std::map<std::string, RFProperty*, RFPropertyNameLess983> PropertyMap;

	WndBaseStruct();
	virtual ~WndBaseStruct();
	virtual int GetType() const { return 0; } // 00439cf0

	void AddProperty(RFProperty* property); // 00439d00
	__forceinline void RegisterProperty(RFProperty* property, const char* name)
	{
		property->SetName(name);
		if (property)
			_properties[property->GetName()] = property;
	}
	RFProperty* GetProperty(const std::string& name); // 00439d80

	std::wstring _name; // +0x04
	float _x; // +0x20 (<pos x y>; zerados no construtor)
	float _y; // +0x24
	RFRect983 _rect; // +0x28 (<rect l t r b>)
	std::wstring _showSound; // +0x38 (<snd type="SHOW">)
	std::wstring _hideSound; // +0x54 (<snd type="HIDE">)
	RFBooleanProperty _dragable; // +0x70 "Dragable"
	RFBooleanProperty _isHitTestVisible; // +0x94
	RFBooleanProperty _isHitTestThrough; // +0xb8
	RFStartPositionProperty _startPosition; // +0xdc
	RFEscKeyActionProperty _escKeyAction; // +0x100
	RFAnchorProperty _anchor; // +0x124
	RFBooleanProperty _clipChildren; // +0x148
	PropertyMap _properties; // +0x16c (0x10)
};

// janela com fundo (0x180; dtor 0043a9b0)
struct WndExBase : public WndBaseStruct
{
	WndExBase() { _background = 0; }
	virtual ~WndExBase();
	virtual int GetType() const { return 1; } // 0043aa40

	RFReferenceCount* _background; // +0x17c
};

// 0x328 (ctor 0043b140, dtor 0043b930)
struct LabelBase : public WndBaseStruct
{
	LabelBase();
	virtual ~LabelBase();
	virtual int GetType() const { return 2; } // 0043b890

	std::wstring _text; // +0x17c
	RFRectProperty _padding; // +0x198
	RFStringProperty _fontName; // +0x1c8
	RFIntegerProperty _fontSize; // +0x204
	RFColorProperty _color; // +0x228
	RFColorProperty _shadowColor; // +0x24c
	RFColorProperty _outlineColor; // +0x270
	int _style; // +0x294 (<style val>)
	RFHAlignProperty _hAlign; // +0x298
	RFVAlignProperty _vAlign; // +0x2bc
	RFTextTrimmingProperty _textTrimming; // +0x2e0
	RFBooleanProperty _fitToContent; // +0x304
};

// 0x204 (ctor 0043bc30, dtor 0043bd90)
struct ButtonBase : public WndBaseStruct
{
	ButtonBase();
	virtual ~ButtonBase();
	virtual int GetType() const { return 3; } // 0043bf80

	RFReferenceCount* _normalImage; // +0x17c (<Asset Name="Normal">; versão antiga <asset type="NORMAL">)
	RFReferenceCount* _pushImage; // +0x180 ("Push"; "DOWN")
	RFReferenceCount* _hoverImage; // +0x184 ("Hover"; "OVER")
	RFReferenceCount* _disableImage; // +0x188 ("Disable"; "DISABLE")
	std::wstring _pushSound; // +0x18c (<snd type="DOWN">)
	std::wstring _hoverSound; // +0x1a8 (<snd type="OVER">)
	std::wstring _clickSound; // +0x1c4 (<snd type="RELEASE">)
	RFBooleanProperty _isDefaultButton; // +0x1e0
};

// 0x29c (ctor 0043bfc0, dtor 0043c320)
struct LabelButtonBase : public ButtonBase
{
	LabelButtonBase();
	virtual ~LabelButtonBase();
	virtual int GetType() const { return 4; } // 0043c2e0

	LabelBase* _label; // +0x204 (<control type="LABEL">; o construtor não o zera)
	bool _hasNormalColor; // +0x208 (true no construtor; false se o XML não tem <NormalColor val>)
	RFColorProperty _normalColor; // +0x20c
	RFColorProperty _hoverColor; // +0x230
	RFColorProperty _pushColor; // +0x254
	RFColorProperty _disableColor; // +0x278
};

// 0x20c (ctor 0043c420, dtor 0043c440)
struct AnimButtonBase : public ButtonBase
{
	AnimButtonBase();
	virtual ~AnimButtonBase();
	virtual int GetType() const { return 5; } // 0043c510

	RFReferenceCount* _enterImage; // +0x204 (<asset type="ENTER">)
	RFReferenceCount* _leaveImage; // +0x208 (<asset type="LEAVE">)
	bool _group; // +0x20c (<Asset Name="Group"> / <asset type="GROUP">: a imagem fica em _normalImage; sizeof 0x210)
};

// 0x1c0 (ctor 0043a600, dtor 0043a720)
struct CheckBase : public WndBaseStruct
{
	CheckBase();
	virtual ~CheckBase();
	virtual int GetType() const { return 6; } // 0043a850

	RFReferenceCount* _uncheckedImage; // +0x17c
	RFReferenceCount* _checkedImage; // +0x180
	RFStringProperty _pushSound; // +0x184
};

// 0x184 (dtor 0043d9b0; o construtor, embutido nos leitores, só troca a vtable: _image não é zerado)
struct PictureBase : public WndBaseStruct
{
	virtual ~PictureBase();
	virtual int GetType() const { return 7; } // 0043da40

	RFReferenceCount* _image; // +0x17c
	bool _static; // +0x180 ("Static" true, "Animation" false; true antes do leitor)
};

// 0x180 (dtor 0043daf0; construtor embutido sem zerar _image, como o da PictureBase)
struct ProgressBase : public WndBaseStruct
{
	virtual ~ProgressBase();
	virtual int GetType() const { return 8; } // 0043db80

	RFReferenceCount* _image; // +0x17c
};

// 0x19c (ctor 00440290; ??_G 00440360)
struct ScrollbarBase : public WndBaseStruct
{
	ScrollbarBase();
	virtual int GetType() const { return 9; } // 0043c550

	ButtonBase* _buttonLT; // +0x17c (<control type="BUTTON_LT">)
	ButtonBase* _buttonRB; // +0x180 ("BUTTON_RB")
	ButtonBase* _thumb; // +0x184 ("THUMB")
	RFRect983 _scrollRect; // +0x188 (<scrollrect>)
	bool _horizontal; // +0x198 (atributo "horizontal")
};

// 0x19c (ctor 004402c0; ??_G 00440390)
struct SliderBase : public WndBaseStruct
{
	SliderBase();
	virtual int GetType() const { return 10; } // 0043c560

	ButtonBase* _buttonLT; // +0x17c (<control type="BUTTON_LT">)
	ButtonBase* _buttonRB; // +0x180 ("BUTTON_RB")
	ButtonBase* _thumb; // +0x184 ("THUMB")
	RFRect983 _slideRect; // +0x188 (<sliderect>)
	bool _horizontal; // +0x198 (atributo "horizontal")
};

// 0x270 (ctor 0043aaf0, dtor 0043aef0)
struct EditBase : public WndExBase
{
	EditBase();
	virtual ~EditBase();
	virtual int GetType() const { return 0xb; } // 0043b100

	RFStringProperty _fontName; // +0x180
	RFIntegerProperty _fontSize; // +0x1bc
	RFColorProperty _color; // +0x1e0
	RFColorProperty _shadowColor; // +0x204
	RFColorProperty _outlineColor; // +0x228
	int _style; // +0x24c (<style val>)
	int _halign; // +0x250 (<halign val>)
	int _valign; // +0x254 (<valign val>)
	RFRect983 _padding; // +0x258 (<padding>)
	wchar_t _passwordChar; // +0x268 (1º caractere de <passwordchar val>)
	RFReferenceCount* _caret; // +0x26c (o construtor não o zera)
};

// 0x1d4 (ctor 0043d270, dtor 0043d3e0)
struct ListBase : public WndExBase
{
	ListBase();
	virtual ~ListBase();
	virtual int GetType() const { return 0xc; } // 0043d3a0

	RFRect983 _padding; // +0x180 (<padding>)
	int _column; // +0x190 (<column val>)
	RFRect983 _itemrect; // +0x194 (<itemrect>; nome do assert do 983)
	int _selection; // +0x1a4 (<selection val>)
	bool _deselectable; // +0x1a8 (<deselectable val>)
	int _rclickaction; // +0x1ac (<rclickaction val>)
	RFOrientationProperty _orientation; // +0x1b0
};

// 0x1dc (ctor 0043d4a0; destrutor = o da ListBase)
struct ListSCBase : public ListBase
{
	ListSCBase();
	virtual int GetType() const { return 0xd; } // 0043d4c0

	ScrollbarBase* _hScrollbar; // +0x1d4 (<control type="HSCROLLBAR">; zerados no construtor; sizeof 0x1dc)
	ScrollbarBase* _vScrollbar; // +0x1d8 ("VSCROLLBAR")
};

// 0x1e8 (ctor 0043c570, dtor 0043c810)
struct StackPanelBase : public WndBaseStruct
{
	StackPanelBase();
	virtual ~StackPanelBase();
	virtual int GetType() const { return 0xe; } // 0043c7d0

	RFHAlignProperty _hAlign; // +0x17c
	RFVAlignProperty _vAlign; // +0x1a0
	RFOrientationProperty _orientation; // +0x1c4
};

// 0x2ec (ctor 0043c8d0, dtor 0043cec0)
struct TextAreaBase : public WndExBase
{
	TextAreaBase();
	virtual ~TextAreaBase();
	virtual int GetType() const { return 0xf; } // 0043d1a0

	RFStringProperty _fontName; // +0x180
	RFIntegerProperty _fontSize; // +0x1bc
	RFColorProperty _color; // +0x1e0
	RFColorProperty _shadowColor; // +0x204
	RFColorProperty _outlineColor; // +0x228
	RFRect983 _padding; // +0x24c (<padding>)
	int _style; // +0x25c (<style val>)
	int _halign; // +0x260 (<halign val>)
	RFBooleanProperty _editable; // +0x264
	RFStringProperty _text; // +0x288
	RFBooleanProperty _fitToContent; // +0x2c4
	RFReferenceCount* _caret; // +0x2e8 (o construtor não o zera)
};

// 0x308 (ctor 0043d1e0; destrutor = o da TextAreaBase)
struct TextAreaSCBase : public TextAreaBase
{
	TextAreaSCBase();
	virtual int GetType() const { return 0x10; } // 0043d220

	ScrollbarBase* _hScrollbar; // +0x2ec (<control type="HSCROLLBAR">; zerado no construtor)
	ScrollbarBase* _vScrollbar; // +0x2f0 ("VSCROLLBAR"; zerado no construtor)
	bool _showAlwaysH; // +0x2f4 (<showalways hscroll vscroll>)
	bool _showAlwaysV; // +0x2f5
	bool _autoVisibleH; // +0x2f6 (<autovisible hscroll vscroll>)
	bool _autoVisibleV; // +0x2f7
	RFRect983 _autoVisibleRect; // +0x2f8 (<autovisiblerect>; sizeof 0x308)
};

// 0x230 (ctor 0043d510, dtor 0043d880)
struct ComboBase : public WndBaseStruct
{
	ComboBase();
	virtual ~ComboBase();
	virtual int GetType() const { return 0x11; } // 0043d840

	ButtonBase* _button; // +0x17c (<control type="BUTTON">)
	TextAreaSCBase* _textArea; // +0x180 ("TEXTAREA")
	ListSCBase* _listArea; // +0x184 ("LISTAREA")
	RFColorProperty _normalColor; // +0x188
	RFColorProperty _selectColor; // +0x1ac
	RFStringProperty _fontName; // +0x1d0
	RFIntegerProperty _fontSize; // +0x20c
};

// 0x198 (ctor 00440330, dtor 004403c0)
struct CustomBase : public WndBaseStruct
{
	CustomBase();
	virtual ~CustomBase();
	virtual int GetType() const { return 0x12; } // 00439e80

	std::wstring _className; // +0x17c
};
