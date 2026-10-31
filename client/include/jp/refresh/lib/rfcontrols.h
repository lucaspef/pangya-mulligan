#pragma once
#include "vc10_layout.h"
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"

// JP 983: controles da Refresh como a biblioteca os declara (F3.7.13 item (3) 3º lote, sessão B151). Classes exportadas pelo
// 983: as cópias fora de linha dos métodos embutidos e os construtores de cópia / operator= implícitos estão no RFWnd.cpp
// (00402a80–00402e30 e 004141b0–00417140). Assinaturas pela exportação (out/F3.7/s149/exports983_nomes.tsv); os corpos que
// não são embutidos ficam nas unidades dos controles (item (5)/(7)). Layout medido nos construtores de cópia (build/B/s151/lay.py)
// e conferido pelo sizeof dos destrutores de apagar (push <tamanho>). Gerado por build/B/s151/gen.py (editar lá).
// As janelas do jogo ainda usam as visões de refresh/rfbase.h e refresh/rfscenemanager.h (unificar no fim do item (5)).

class ImageSet;
class RFListItemBase;
class RFListItemWnd;
class RFValidator;
class RFBrush; // pincel criado de um ImageSet (refresh/lib/rfbrush.h; RFBrush::Create 0042a8b0)
class RFCaret983; // refresh/lib/rftext.h
class RFText983; // refresh/lib/rftext.h (0x54; ctor 00456260)

struct HorzAlignTypes { enum Type { Left = 0, Center = 1, Right = 2 }; }; // valores pelo RFHAlignProperty
struct VertAlignTypes { enum Type { Top = 0, Center = 1, Bottom = 2 }; }; // RFVAlignProperty
struct TextTrimmingTypes { enum Type { None = 0, CharacterEllipsis = 1 }; }; // RFTextTrimmingProperty
struct OrientationTypes { enum Type { Vertical = 0, Horizontal = 1 }; }; // RFOrientationProperty
struct ListSelectionTypes { enum Type { Single983 = 0, Multiple983 = 1 }; }; // nomes meus
enum RFRCLICKACTION { RFRCLICKACTION_SELECT983 = 0, RFRCLICKACTION_NONE983 = 1 }; // 1: o botão direito não seleciona (RFList::ButtonDown_Impl)
enum RFSCROLLBAR_TYPE { RFSCROLLBAR_HORZ983 = 1, RFSCROLLBAR_VERT983 = 2, RFSCROLLBAR_BOTH983 = 3 }; // RFListSC::LockScrollbar

// fonte de um texto (0x30, não exportada; GetFontInfo dos controles; campos pelos Set*Color/SetStyle do RFLabel/RFEdit/RFTextArea)
struct FontInfo
{
	FontInfo() : size(0), style(0), color(0), outlineColor(0), shadowColor(0) { } // (embutido no RFLabel::RFLabel 0041f0c0)
	std::wstring name; // +0x00
	int size; // +0x1c
	unsigned int style; // +0x20
	unsigned int color; // +0x24
	unsigned int outlineColor; // +0x28
	unsigned int shadowColor; // +0x2c
};

class RFInterpolation;

// rolagem suave da barra (0x1c; RFScrollBar +0x1a4; não exportada, nome meu; o construtor de cópia do 983 a copia com rep movsd).
// Construtor 00428c00, Reset 00428cd0, Step 00428cf0 (rfscrollbar.cpp). Curva: Bézier (0.3,0.7)-(0.5,1.0) em 0x400 passos.
struct RFSmoothScroll983
{
	RFSmoothScroll983(); // 00428c00
	~RFSmoothScroll983(); // (no ~RFScrollBar 00428ec0)
	// 00428cd0 (embutida no OnLButtonDown e no MoveToScroll)
	void Reset()
	{
		elapsed = 0.0f;
		active = false;
		t = 0.0f;
		duration = 600;
		from = 0;
		to = 0;
	}
	int Step(float dt); // 00428cf0: avança dt segundos e devolve a posição

	bool active; // +0x0
	unsigned int duration; // +0x4 (ms)
	int from; // +0x8
	int to; // +0xc
	float elapsed; // +0x10 (s)
	float t; // +0x14 (0..1)
	RFInterpolation* curve; // +0x18
};

// área de um hiperlink do texto (0x2c: o vector do RFTextArea anda de 0x2c em 0x2c; palpite: texto + retângulo)
struct RFHyperLink983
{
	std::wstring text;
	RFRect rect;
};

// janela com fundo (0x180; vtable 00d6403c)
class RF_API RFWndEx : public RFWnd
{
public:
	RFWndEx(); // 0041d980
	virtual ~RFWndEx(); // 0041d9a0
	virtual void Create(WndBaseStruct*,RFWnd*); // 0041d9b0
protected:
	virtual void PreInitialize(); // 0041d9e0
	virtual void LoadResource(); // 0041da40
public:
	virtual void OnDestroy(); // 0041da60
	virtual void OnDraw(); // 0041da90
	static std::wstring const TypeName; // 00e46cd4
	virtual const wchar_t* ToString() const { return L"RFWndEx"; } // 004141b0
	virtual const std::wstring& GetTypeName() { return TypeName; } // 004141c0

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFBrush* _brush; // +0x178 (feito do _imageSet no LoadResource: RFBrush::Create 0042a8b0)
	ImageSet* _imageSet; // +0x17c (fundo da janela; a visão do jogo chama de m_layer983)
	// D857: para a visão do jogo (rfscenemanager.h, +0x17c do VC10): fora de linha, com o layout real (no toolset novo fica antes)
	int GetImageSet983();
};

// botão (0x1f4; vtable 00d62f1c)
class RF_API RFButton : public RFWnd
{
public:
	enum RFBUTTON_STATE { RFBS_NORMAL983 = 0, RFBS_DOWN983 = 1, RFBS_OVER983 = 2, RFBS_DISABLE983 = 3 };
	RFButton(); // 00420e10
	virtual ~RFButton(); // 00420e70
protected:
	virtual void OnStateChanged() { } // 00402ab0 (slot 34)
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 00420f30
protected:
	virtual void PreInitialize(); // 00420fd0
	virtual void LoadResource(); // 004211c0
	virtual void OnDestroy(); // 00421210
public:
	void ButtonReset(); // 00421240
protected:
	virtual void OnDraw(); // 00421260
	virtual void OnEnabled(bool); // 00421420
	virtual void OnShow(bool); // 00421440
	virtual void OnLButtonDown(MouseEventArgs&); // 004214d0
	virtual void OnLButtonUp(MouseEventArgs&); // 00421540
	virtual void OnMouseEnter(); // 00421690
	virtual void OnMouseLeave(); // 00421710
	void MouseEnter_Impl(); // 004217c0
	void MouseLeave_Impl(); // 00421820
	void DefaultButtonEventHandler(EventArgs&); // 00421870
public:
	void ChangeNormalButtonImage(std::wstring); // 004218f0
	void ChangeOverButtonImage(std::wstring); // 00421990
	void ChangeDownButtonImage(std::wstring); // 00421a30
	void ChangeDisableButtonImage(std::wstring); // 00421ad0
	static std::wstring const EventClicked; // 00e46d28
	static std::wstring const TypeName; // 00e46d44
	virtual const wchar_t* ToString() const { return L"RFButton"; } // 00402a80
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00402a90
	RFBUTTON_STATE GetState() const { return _state; } // 00402aa0
#if _MSC_VER > 1600
	void SetState983(int state); // D976: para a visão do jogo (rfbase.h)
	int GetState983();
#endif
	// 00402da0 (desabilitado = EnableWindow(false), embutido)
	void SetState(RFBUTTON_STATE state)
	{
		if (state == RFBS_DISABLE983)
			EnableWindow(false);
		else
		{
			_state = state;
			_pressed = (state == RFBS_DOWN983);
			OnStateChanged();
		}
	}
	const std::wstring& GetPushSound() const { return _pushSound; } // 00402e00
	const std::wstring& GetOverSound() const { return _overSound; } // 00402e10
	const std::wstring& GetReleaseSound() const { return _releaseSound; } // 00402e20

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFBUTTON_STATE _state; // +0x178
	bool _pressed; // +0x17c
	bool _17d; // (true entre MouseEnter_Impl e MouseLeave_Impl)
	bool _17e; // ButtonBase::_isDefaultButton (Enter dispara o EventClicked: DefaultButtonEventHandler)
	std::wstring _pushSound; // +0x180 (DOWN)
	std::wstring _overSound; // +0x19c (OVER)
	std::wstring _releaseSound; // +0x1b8 (RELEASE)
	// pincéis na ordem de RFBUTTON_STATE (o OnDraw indexa por _state; B156 corrigiu down/over pelos Change*ButtonImage)
	RFBrush* _normalBrush; // +0x1d4 (ChangeNormalButtonImage; campos soltos: um array reinicia a rotação de registradores da cópia)
	RFBrush* _downBrush; // +0x1d8
	RFBrush* _overBrush; // +0x1dc
	RFBrush* _disableBrush; // +0x1e0
	ImageSet* _normalImage; // +0x1e4 (ButtonBase::_normalImage)
	ImageSet* _downImage; // +0x1e8 (_pushImage)
	ImageSet* _overImage; // +0x1ec (_hoverImage)
	ImageSet* _disableImage; // +0x1f0
};

// rótulo (0x1e4; vtable 00d63a3c)
class RF_API RFLabel : public RFWnd
{
public:
	RFLabel(); // 0041f0c0
	virtual ~RFLabel(); // 0041f140
	virtual void Create(WndBaseStruct*,RFWnd*); // 0041f1c0
	void FitToContent(); // 0041f340
	void AutoFit(bool); // 0041f490
protected:
	virtual void PreInitialize(); // 0041f4b0
	void putComma_(wchar_t const*,wchar_t*); // 0041f510
public:
	void SetLabel(int,bool); // 0041f580
	void SetLabel(const std::wstring&); // 0041f830
	void SetPadding(const RFRect&); // 0041f860
	const RFRect& GetPadding() const; // 0041f890
protected:
	virtual void OnDraw(); // 0041f8a0
public:
	static std::wstring const TypeName; // 00e46d0c
	virtual const wchar_t* ToString() const { return L"RFLabel"; } // 004142e0
	virtual const std::wstring& GetTypeName() { return TypeName; } // 004142f0
	const std::wstring& GetLabel() const { return _label; } // 00414660
	void SetColor(unsigned int color) { _fontInfo.color = color; } // 00414670
#if _MSC_VER > 1600
	void SetColor983(unsigned int color); // D877: para a visão do jogo (rfbase.h)
#endif
	unsigned int GetColor() const { return _fontInfo.color; } // 00414680
	void SetShadowColor(unsigned int color) { _fontInfo.shadowColor = color; } // 00414690
	unsigned int GetShadowColor() const { return _fontInfo.shadowColor; } // 004146a0
	void SetOutlineColor(unsigned int color) { _fontInfo.outlineColor = color; } // 004146b0
	unsigned int GetOutlineColor() const { return _fontInfo.outlineColor; } // 004146c0
	void SetStyle(unsigned int style) { _fontInfo.style = style; } // 004146d0
	unsigned int GetStyle() const { return _fontInfo.style; } // 004146e0
	void SetVAlign(VertAlignTypes::Type align) { _vAlign = align; } // 004146f0
	void SetHAlign(HorzAlignTypes::Type align) { _hAlign = align; } // 00414700
	VertAlignTypes::Type GetVAlign() const { return _vAlign; } // 00414710
	HorzAlignTypes::Type GetHAlign() const { return _hAlign; } // 00414720
	const FontInfo& GetFontInfo() const { return _fontInfo; } // 00414730
	void SetTextTrimmingType(TextTrimmingTypes::Type type) { _textTrimming = type; } // 00414740
	TextTrimmingTypes::Type GetTextTrimmingType() const { return _textTrimming; } // 00414750

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	std::wstring _label; // +0x178
	RFRect _padding; // +0x194
	FontInfo _fontInfo; // +0x1a4
	HorzAlignTypes::Type _hAlign; // +0x1d4
	VertAlignTypes::Type _vAlign; // +0x1d8
	TextTrimmingTypes::Type _textTrimming; // +0x1dc
	bool _autoFit; // +0x1e0 (AutoFit; nome meu)
};

// botão com rótulo (0x210; vtable 00d63acc)
class RF_API RFLabelButton : public RFButton
{
public:
	RFLabelButton(); // 00425f50
	virtual ~RFLabelButton(); // 00425fb0
	virtual void Create(WndBaseStruct*,RFWnd*); // 00425fc0
protected:
	virtual void PreInitialize(); // 004260e0
public:
	void SetLabel(const std::wstring&); // 00426120
	const std::wstring& GetLabel() const; // 00426160
protected:
	virtual void OnUpdate(UpdateEventArgs&); // 00426170
public:
	static std::wstring const TypeName; // 00e46e40
	virtual const wchar_t* ToString() const { return L"RFLabelButton"; } // 00414760
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00414770

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFLabel* _labelWnd983; // +0x1f4 (nome meu)
	int _1f8;
	bool _1fc;
	unsigned int _normalColor983; // +0x200 (cor do texto por estado; palpite)
	unsigned int _overColor983; // +0x204
	unsigned int _downColor983; // +0x208
	unsigned int _disableColor983; // +0x20c
};

// botão animado (0x208; vtable 00d637dc)
class RF_API RFAnimButton : public RFButton
{
public:
	RFAnimButton(); // 004344f0
	virtual ~RFAnimButton(); // 00434560
	virtual void LoadAnimation(const std::wstring&); // 00434770
	virtual void UnloadAnimation(); // 00434840
	virtual R_ERRORTYPE SetAnimation(RFDirector*); // 00434870
private:
	virtual void LoadAnimation_(const std::wstring&); // 00434f30
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 00434570
protected:
	virtual void PreInitialize(); // 004345d0
	virtual void OnDestroy(); // 00434650
	virtual void LoadResource(); // 004346a0
public:
	void SetAnimPos(float,float); // 004348e0
protected:
	const std::wstring& GetAnimName(int) const; // 00434910
	virtual void OnStateChanged(); // 004349a0
	virtual void OnLButtonDown(MouseEventArgs&); // 004349f0
	virtual void OnLButtonUp(MouseEventArgs&); // 00434aa0
	virtual void OnMouseEnter(); // 00434b60
	virtual void OnMouseLeave(); // 00434c60
	virtual void OnUpdate(UpdateEventArgs&); // 00434d10
	virtual void OnDraw(); // 00434db0
private:
	virtual void OnEnabled(bool); // 00434f50
public:
	static std::wstring const TypeName; // 00e46fe4
	virtual const wchar_t* ToString() const { return L"RFAnimButton"; } // 00414910
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00414920
	RFDirector* GetDirector() { return _director; } // 00414930

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFDirector* _director; // +0x1f4 (criado no PreInitialize; cenas = arquivos das imagens da <AnimButton>)
	int _animState983; // +0x1f8 (cena de passagem tocando: 4 ENTER, 5 LEAVE; 0 = a do _state; B160 pelo Create/OnUpdate)
	ImageSet* _enterImage; // +0x1fc (AnimButtonBase::_enterImage; SingleImageSet com o caminho da cena)
	ImageSet* _leaveImage; // +0x200 (AnimButtonBase::_leaveImage)
	bool _group; // +0x204 (AnimButtonBase::_group: uma cena só, a do _normalImage)
};

// caixa de marcar (0x1a8; vtable 00d6387c)
class RF_API RFCheck : public RFWnd
{
public:
	RFCheck(); // 00425a80
	virtual ~RFCheck(); // 00425ab0
	virtual void Create(WndBaseStruct*,RFWnd*); // 00425b00
protected:
	virtual void PreInitialize(); // 00425b50
	virtual void LoadResource(); // 00425bc0
	virtual void OnDestroy(); // 00425bf0
public:
	bool GetCheck() const; // 00425c20
	void SetCheck(bool); // 00425c30
protected:
	virtual void OnLButtonDown(MouseEventArgs&); // 00425c40
	virtual void OnLButtonUp(MouseEventArgs&); // 00425c90
	virtual void OnDraw(); // 00425d80
public:
	static std::wstring const EventChanged; // 00e46e08
	static std::wstring const TypeName; // 00e46e24
	virtual const wchar_t* ToString() const { return L"RFCheck"; } // 00414aa0
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00414ab0

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	bool _check; // +0x178
	std::wstring _sound983; // +0x17c (nome meu)
	RFBrush* _brushes[2]; // +0x198 (palpite: desmarcado/marcado)
	ImageSet* _imageSets[2]; // +0x1a0
};

// argumentos do RFCheck::EventChanged (não exportada; vtable 00d64538; base WindowEventArgs_<RFCheck> 00d64530)
class CheckChangedEventArgs : public WindowEventArgs_<RFCheck>
{
public:
	CheckChangedEventArgs(RFCheck* wnd, bool check) : WindowEventArgs_<RFCheck>(wnd), check(check) { }

	bool check; // +0xc (estado novo)
};

// lista (0x1dc; vtable 00d63b5c)
// item de lista (RFList.h; não exportadas; 23 slots, vtable 00d643bc; nomes dos virtuais meus salvo GetTypeName/HitTest/Select)
class RFListItemBase
{
public:
	RFListItemBase() : _index(-1), _drawIndex983(-1), _selected(false), _over983(false), _disabled983(false) { }
	virtual ~RFListItemBase() { } // 00421d90
	virtual const std::wstring& GetTypeName() const = 0; // slot 1
	virtual bool HitTest(const RFVector2& pt) = 0; // slot 2
	virtual void SetIndex(int index) { _index = index; } // 00421cf0
	virtual int GetIndex() const { return _index; } // 00421d00
	virtual void SetDrawIndex983(int index) { _drawIndex983 = index; } // 00421d10 (ordem no desenho: só os visíveis no recorte)
	virtual int GetDrawIndex983() const { return _drawIndex983; } // 00421d20
	virtual void Show983(bool show) = 0; // slot 7
	virtual bool IsVisible983() const = 0; // slot 8
	virtual void Select(bool select) { _selected = select; } // 00421d30
	virtual bool IsSelected() const { return _selected; } // 00421d40
	virtual void SetOver983(bool over) { _over983 = over; } // 00421d50
	virtual bool IsOver983() const { return _over983; } // 00421d60
	virtual void Disable983(bool disable) { _disabled983 = disable; } // 00421d70
	virtual bool IsDisabled983() const { return _disabled983; } // 00421d80
	virtual void SetPos(const RFVector2& pos) = 0; // slot 15
	virtual const RFVector2& GetPos() = 0; // slot 16
	virtual void SetSize(const RFSize& size) = 0; // slot 17
	virtual const RFSize& GetSize() = 0; // slot 18
	virtual void SetAlpha(float alpha) = 0; // slot 19
	virtual float GetAlpha() const = 0; // slot 20
	virtual void SetData(void* data) = 0; // slot 21
	virtual void* GetData() const = 0; // slot 22

	int _index; // +0x04
	int _drawIndex983; // +0x08
	bool _selected; // +0x0c
	bool _over983; // +0x0d
	bool _disabled983; // +0x0e
};

// item de dados (0x2c; vtable 00d6447c; RFList::InsertItem): só guarda o ponteiro do usuário e o retângulo
class RFListItem : public RFListItemBase
{
public:
	RFListItem() : _visible983(false), _pos(0.0f, 0.0f), _size(0.0f, 0.0f), _data(0) { }
	virtual const std::wstring& GetTypeName() const { return TypeName; } // 00422410
	virtual bool HitTest(const RFVector2& pt); // 00422420
	virtual void Show983(bool show) { _visible983 = show; } // 004224a0
	virtual bool IsVisible983() const { return _visible983; } // 004224b0
	virtual void SetPos(const RFVector2& pos) { _pos = pos; } // 004224c0
	virtual const RFVector2& GetPos() { return _pos; } // 004224e0
	virtual void SetSize(const RFSize& size) { _size = size; } // 004224f0
	virtual const RFSize& GetSize() { return _size; } // 00422510
	virtual void SetAlpha(float alpha) { _alpha = alpha; } // 00422520
	virtual float GetAlpha() const { return _alpha; } // 00422530
	virtual void SetData(void* data) { _data = data; } // 00422540
	virtual void* GetData() const { return _data; } // 00422550
	static std::wstring const TypeName; // 00e46dd0 ("UserItemType")

	bool _visible983; // +0x10
	RFVector2 _pos; // +0x14
	RFSize _size; // +0x1c
	float _alpha; // +0x24 (o construtor não inicializa)
	void* _data; // +0x28
};

// item janela (0x24; vtable 00d6441c; RFList::InsertWindowItem_Impl): o dado é a própria janela; estados pelas animações
// _Normal_/_Selected_/_Over_/_Disabled_ da janela
class RFListItemWnd : public RFListItemBase
{
public:
	RFListItemWnd() : _wnd(0), _pos(0.0f, 0.0f), _size(0.0f, 0.0f) { }
	virtual const std::wstring& GetTypeName() const { return TypeName; } // 00421e00
	virtual bool HitTest(const RFVector2& pt); // 00421e10
	virtual void Show983(bool show); // 00421e40
	virtual bool IsVisible983() const; // 00421e50
	virtual void Select(bool select); // 00421e80
	virtual void SetOver983(bool over); // 00421f60
	virtual void Disable983(bool disable); // 00422070
	virtual void SetPos(const RFVector2& pos); // 004221a0
	virtual const RFVector2& GetPos(); // 004221d0
	virtual void SetSize(const RFSize& size); // 00422230
	virtual const RFSize& GetSize(); // 00422290
	virtual void SetAlpha(float alpha); // 004222c0
	virtual float GetAlpha() const; // 00422320
	virtual void SetData(void* data) { _wnd = static_cast<RFWnd*>(data); } // 00422380
	virtual void* GetData() const { return _wnd; } // 00422390
	static std::wstring const TypeName; // 00e46dec ("WindowItemType")

	RFWnd* _wnd; // +0x10
	RFVector2 _pos; // +0x14 (tela)
	RFSize _size; // +0x1c
};

class RFList;

// EventSelChanged (0x10; vtable 00d644e4): a lista em +0x8, o item em +0xc
class ListSelChangedEventArgs : public WindowEventArgs_<RFList>
{
public:
	ListSelChangedEventArgs(RFList* list, RFListItemBase* item_) : WindowEventArgs_<RFList>(list), item(item_) { }

	RFListItemBase* item; // +0x0c
};

// EventDrawItem (0x18; vtable 00d644ec): item visível no recorte e a sua posição na tela
class ListDrawItemEventArgs : public WindowEventArgs_<RFList>
{
public:
	ListDrawItemEventArgs(RFList* list) : WindowEventArgs_<RFList>(list), pos(0.0f, 0.0f) { }

	RFListItemBase* item; // +0x0c
	RFVector2 pos; // +0x10
};

class RF_API RFList : public RFWndEx
{
public:
	RFList(); // 00422660
	virtual ~RFList(); // 004226e0
	virtual void ItemClipEnable(bool enable) { _itemClip = enable; } // 00414df0 (slot 34)
protected:
	virtual void OnItemAdded(); // 00422b10
	virtual void OnItemRemoved(); // 00422b20
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 00422750
protected:
	virtual void PreInitialize(); // 00422830
	virtual void OnDestroy(); // 00422890
	virtual void OnLayoutChanged(unsigned int); // 00422950
public:
	void EnableMultiSelect(bool); // 00422970 (_selectionMode = Multiple983/Single983)
	void RelaseAllSelectedItem(); // 00422990
	void RelaseAllOverItem(); // 00422ac0
protected:
	virtual void OnLButtonDown(MouseEventArgs&); // 00422b30
	virtual void OnRButtonDown(MouseEventArgs&); // 00422b70
	virtual void OnMouseEnter(); // 00422bc0
	virtual void OnMouseLeave(); // 00422c30
	virtual void OnMouseMove(MouseEventArgs&); // 00422cc0
	void ReindexItems(); // 00422d40
	void DeleteFromSelList(RFListItemBase*); // 00422da0
	void ButtonDown_Impl(MouseEventArgs&); // 00422e00
	void MultipleSelectionProc(RFListItemBase*,MouseEventArgs&); // 00422f50
	void SingleSelectionProc(RFListItemBase*,MouseEventArgs&); // 00422f90
	void UpdateSelectList(); // 00423000
	virtual void OnUpdate(UpdateEventArgs&); // 00423180
	void UpdateItemPosition(); // 004231a0
	void VericalAlignment(const RFVector2&); // 00423240
	void HorizontalAlignment(const RFVector2&); // 004234a0
	virtual void OnDraw(); // 00423700
	void OnItemMouseLeave(EventArgs&); // 00423a60
public:
	float GetAllItemsWidth() const; // 00423a70
	RFRect GetItemRect(int,int,bool); // 00423bc0
	float GetAllItemsHeight() const; // 00423c80
	OrientationTypes::Type GetOrientation() const; // 00423dd0
	void SetOrientation(OrientationTypes::Type); // 00423de0
	void InsertItem(void*); // 00423df0
	RFWnd* InsertWindowItem(const std::wstring&); // 00423f70
	RFWnd* InsertWindowItem(RFWnd*); // 00423fa0
	RFWnd* InsertWindowItem(const std::wstring&,unsigned int); // 00424040
	RFWnd* InsertWindowItem(RFWnd*,unsigned int); // 00424090
protected:
	RFListItemWnd* InsertWindowItem_Impl(RFWnd*); // 00424100
public:
	void SetItemData(int,void*); // 00424340
	void* GetItemData(int); // 004243a0
	RFListItemBase* GetListItem(int); // 004243f0
	void SelectItem(int,bool); // 00424440
	void SelectItem(void*,bool); // 00424540
	void UnselectItem(int,bool); // 004246e0
	void UnselectItem(void*,bool); // 004247d0
	void EnableItem(void*,bool); // 00424920
	RFListItemBase* GetItemUnderCursor(); // 00424970
protected:
	RFListItemBase* GetItemOnPosition_Impl(const RFVector2&); // 004249d0
public:
	void DeleteItem(int); // 00424a60
	void DeleteItem(void*); // 00424b70
	void DeleteAllItems(); // 00424ca0
protected:
	bool IsValidIndex(int) const; // 00424db0
public:
	static std::wstring const EventSelChanged; // 00e46d60
	static std::wstring const EventBeginDrawItem; // 00e46d7c
	static std::wstring const EventEndDrawItem; // 00e46d98
	static std::wstring const EventDrawItem; // 00e46db4
	virtual const wchar_t* ToString() const { return L"RFList"; } // 00414cf0
	const RFRect& GetPadding() const { return _padding; } // 00414d00
	void SetPadding(const RFRect& padding) { _padding = padding; } // 00414d10
	float GetItemWidth() const { return _itemSize.width; } // 00414d40
	float GetItemHeight() const { return _itemSize.height; } // 00414d50
	ListSelectionTypes::Type GetSelectionMode() const { return _selectionMode; } // 00414d60
	void SetSelectionMode(ListSelectionTypes::Type mode) { _selectionMode = mode; } // 00414d70
	bool IsDeselectable() const { return _deselectable; } // 00414d80
	void SetDeselectable(bool b) { _deselectable = b; } // 00414d90
	bool IsReselectable() const { return _reselectable; } // 00414da0
	void SetReselectable(bool b) { _reselectable = b; } // 00414db0
	RFRCLICKACTION GetRClickAction() const { return _rClickAction; } // 00414dc0
	void SetRClickAction(RFRCLICKACTION action) { _rClickAction = action; } // 00414dd0
	int GetFirstDrawItemIndex() const { return _firstDrawItem; } // 00414de0
	int GetSelItemCount() const { return _selItems.size(); } // 00415110
	// 00415120
	RFListItemBase* GetSelItem(int index)
	{
		if (index < 0 || index >= GetSelItemCount())
			return 0;
		return _selItems[index];
	}
	int GetColumnCount() const { return _columnCount; } // 00415150
	int GetItemCount() const { return _items.size(); } // 00415160
	int GetItemCount983(); // C731: visão do jogo (rfbase.h m_items983 +0x180 do VC10), fora de linha

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	std::vector<RFListItemBase*> _items; // +0x180
	std::vector<RFListItemBase*> _selItems; // +0x190
	RFVector2 _scrollPos983; // +0x1a0 (deslocamento dos itens; o RFListSC põe −posição das barras)
	int _firstDrawItem; // +0x1a8 (1º item que cruza a área cliente; −1 = nenhum)
	OrientationTypes::Type _orientation; // +0x1ac (GetOrientation 00423dd0)
	bool _itemClip; // +0x1b0
	RFRect _padding; // +0x1b4
	int _columnCount; // +0x1c4
	RFSize _itemSize; // +0x1c8 (copiado com mov: membro float solto sairia com fld/fstp)
	ListSelectionTypes::Type _selectionMode; // +0x1d0
	bool _deselectable; // +0x1d4
	bool _reselectable; // +0x1d5
	RFRCLICKACTION _rClickAction; // +0x1d8
};

// o "polegar" da barra (0x208; não exportada; vtable 00d65dbc; ctor 0045a490, embute o do RFButton): arrastável, preso ao
// retângulo de rolagem da barra (_scrollRect, relativo à barra)
class RFThumbButton : public RFButton
{
public:
	RFThumbButton(); // 0045a490
	virtual ~RFThumbButton() { } // (??_G 0045a510)
	virtual const wchar_t* ToString() const; // 0045a480
	virtual void Create(WndBaseStruct*,RFWnd*); // 0045a550
protected:
	virtual void PreInitialize(); // 0045a560
	virtual void OnMouseMove(MouseEventArgs&); // 0045a5b0
public:
	bool _horizontal; // +0x1f4
	RFRect _scrollRect; // +0x1f8
};

class RFScrollBar;
// argumentos do RFScrollBar::EventPosChanged (não exportada; vtable 00d64730; base WindowEventArgs_<RFScrollBar> 00d64728)
class ScrollBarPosChangedEventArgs : public WindowEventArgs_<RFScrollBar>
{
public:
	ScrollBarPosChangedEventArgs(RFScrollBar* wnd, int pos) : WindowEventArgs_<RFScrollBar>(wnd), pos(pos) { } // 00428db0
	int pos; // +0xc
};

// barra de rolagem (0x1d4; vtable 00d63dbc)
class RF_API RFScrollBar : public RFWnd
{
public:
	RFScrollBar(); // 00428e30
	virtual ~RFScrollBar(); // 00428ec0
protected:
	virtual void OnLTButClicked(EventArgs&); // 00429b80
	virtual void OnRBButClicked(EventArgs&); // 00429bd0
	virtual void OnThumbMouseMove(EventArgs&); // 00429a90
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 00428f30
protected:
	virtual void PreInitialize(); // 00429110
	virtual void OnLayoutChanged(unsigned int); // 00429390
	virtual void OnEnabled(bool); // 00429430
	virtual void OnLButtonUp(MouseEventArgs&); // 00429440
	virtual void OnRButtonUp(MouseEventArgs&); // 00429460
	virtual void OnLButtonDown(MouseEventArgs&); // 00429480
	virtual void OnUpdate(UpdateEventArgs&); // 00429620
public:
	void SetScrollSize(int); // 00429660
	void SetScrollRange(int,int); // 00429670
	void SetScrollPos(int); // 00429770
	int GetScrollPos() const; // 00429830 (export QBE)
	void Next(); // 00429840
	void Prev(); // 004298a0
	void ScrollToTop(); // 00429900
	void ScrollToBottom(); // 00429910
	bool IsBottom(); // 00429920
	void MoveToScroll(int); // 00429930
protected:
	virtual void OnMouseWheel(MouseEventArgs&); // 00429a30
	void PositionToLevel(const RFVector2&); // 00429c20
	void LevelToThumbPosition(); // 00429da0
	void SmoothScrolling(float); // 0042a090
public:
	static std::wstring const EventPosChanged; // 00e46ecc
	static std::wstring const TypeName; // 00e46ee8
	virtual const wchar_t* ToString() const { return L"RFScrollBar"; } // 00415170
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00415180
	const RFPoint& GetScrollRange() const { return _range; } // 00415190
	void EnableSmoothScroll(bool enable) { _smoothScroll = enable; } // 004151a0
	void EnableSmoothScroll983(bool enable); // D857: visão do jogo (rfbase.h m_bWheel983 +0x19c do VC10), fora de linha

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFButton* _ltButton983; // +0x178 (nome meu; <control type="BUTTON_LT">)
	RFButton* _rbButton983; // +0x17c ("BUTTON_RB")
	RFThumbButton* _thumb983; // +0x180 ("THUMB")
	RFSize _thumbSize983; // +0x184 (tamanho do polegar no Create: altura mínima no SetScrollRange)
	RFPoint _range; // +0x18c (x = mínimo, y = máximo)
	int _pos983; // +0x194
	int _size983; // +0x198 (passo dos botões)
	bool _smoothScroll; // +0x19c
	int _accel983; // +0x1a0 (cliques seguidos durante a rolagem suave, até 5)
	RFSmoothScroll983 _smooth983; // +0x1a4
	RFRect _scrollRect983; // +0x1c0 (ScrollbarBase::_scrollRect; relativo à barra)
	bool _horizontal983; // +0x1d0
};

// lista com barras (0x1e8; vtable 00d63bf4)
class RF_API RFListSC : public RFList
{
public:
	RFListSC(); // 00450080
	virtual ~RFListSC(); // 004500a0
protected:
	virtual void OnHScrollPosChanged(EventArgs&); // 00450da0
	virtual void OnVScrollPosChanged(EventArgs&); // 00450de0
	virtual void OnScrollDraw(EventArgs&); // 00450e20
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 004500b0
protected:
	virtual void PreInitialize(); // 00450270
public:
	virtual bool HitTest(const RFVector2&,bool); // 004505a0
	void LockScrollbar(RFSCROLLBAR_TYPE,bool); // 00450640
	bool IsScrollBarAvailable(RFSCROLLBAR_TYPE) const; // 004506d0
	void ScrollToTop(); // 004507b0
	void ScrollToBottom(); // 004507f0
	void ScrollToItem(int); // 00450830
protected:
	void UpdateScrollbar(); // 00450930
	virtual void OnItemAdded(); // 00450b20
	virtual void OnItemRemoved(); // 00450b30
	virtual void OnUpdate(UpdateEventArgs&); // 00450b40
public:
	void SetScrollSize(unsigned int); // 00450b70
protected:
	float GetScrollSize(); // 00450b80
	virtual void OnMouseWheel(MouseEventArgs&); // 00450c30
public:
	static unsigned int const PAGE; // 00d64cc8
	static std::wstring const TypeName; // 00e470c4
	virtual const wchar_t* ToString() const { return L"RFListSC"; } // 00415520
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00415530
	RFScrollBar* GetHScrollBar() { return _hScrollBar; } // 00415540
	RFScrollBar* GetVScrollBar() { return _vScrollBar; } // 00415550

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFScrollBar* _hScrollBar; // +0x1dc
	RFScrollBar* _vScrollBar; // +0x1e0
	unsigned int _scrollSize983; // +0x1e4 (SetScrollSize)
};

// EventHyperLinkClicked (vtable 00d646e4) / EventHyperLinkEnter (00d646ec): janela em +8, 2o vptr em +0xc, alvo do link em +0x18
// (layout pelo OnMouseMove 00427170 / HitTestHyperLink 00426f60; nomes meus)
class HyperLinkTextEventArgs983 : public WindowEventArgs_<RFWnd>
{
public:
	HyperLinkTextEventArgs983(RFWnd* wnd, const std::wstring& link) : WindowEventArgs_<RFWnd>(wnd), _vptr2(0), _10(0), _14(0)
	{
		link983.assign(link, 0, std::wstring::npos);
	}

	int _vptr2; // +0xc
	int _10, _14;
	std::wstring link983; // +0x18
};

// EventRichTextDrawImage (vtable 00d646f4; SendDrawImageEvent 00428040): janela em +8, 2o vptr em +0xc, y da imagem em +0x10,
// x em +0x18 (palpite: o asm grava y em +0x10 e x em +0x18) e o nome em +0x20
class RichTextDrawImageEventArgs983 : public WindowEventArgs_<RFWnd>
{
public:
	RichTextDrawImageEventArgs983(RFWnd* wnd, const RFVector2& pos, const std::wstring& name)
		: WindowEventArgs_<RFWnd>(wnd), _vptr2(0), y983(pos.y), _14(0), x983(pos.x), _1c(0)
	{
		name983.assign(name, 0, std::wstring::npos);
	}

	int _vptr2; // +0xc
	float y983; // +0x10
	int _14;
	float x983; // +0x18
	int _1c;
	std::wstring name983; // +0x20
};

// área de texto (0x274; vtable 00d63eec)
class RF_API RFTextArea : public RFWndEx
{
public:
	RFTextArea(); // 00426580
	virtual ~RFTextArea(); // 00426690
	virtual void SetText(const std::wstring&); // 00427bc0
	virtual void AddText(const std::wstring&); // 00427bf0
	virtual void AddPlainText(const std::wstring&); // 00427c50
	virtual const std::wstring& GetText(); // 00427c90
	virtual void ClearText(); // 00427d10
	virtual void Create(WndBaseStruct*,RFWnd*); // 004267b0
protected:
	virtual void PreInitialize(); // 00426b20
	virtual void OnDestroy(); // 00426b90
	virtual void LoadResource(); // 00426bf0
	virtual void OnLayoutChanged(unsigned int); // 00426c90
	virtual void OnEnabled(bool); // 00426d90
	virtual void OnLButtonDown(MouseEventArgs&); // 00426e00
	virtual void OnRButtonDown(MouseEventArgs&); // 00426ee0
	virtual void OnRButtonUp(MouseEventArgs&); // 00426f30
	bool HitTestHyperLink(MouseEventArgs&,bool); // 00426f60
	virtual void OnMouseMove(MouseEventArgs&); // 00427170
	virtual void OnMouseLeave(); // 004274e0
	virtual void OnKeyDown(KeyEventArgs&); // 004275f0
	virtual void OnChar(CharEventArgs&); // 00427810
	virtual void OnSetFocus(); // 004278d0
	virtual void OnKillFocus(); // 00427950
	virtual void OnDraw(); // 004279c0
	virtual void OnUpdate(UpdateEventArgs&); // 00427b30
	virtual void OnMouseWheel(MouseEventArgs&); // 00427b80
public:
	void SetHAlign(HorzAlignTypes::Type); // 00427c30
	void SetTextLimit(unsigned int); // 00427d30
	unsigned int GetTextLimit() const; // 00427d50
	unsigned int GetLineLimit() const; // 00427d60
	void FitToContent(bool); // 00427d70
	void AutoFit(bool); // 00427f90
	void Invaildate(); // 00427fb0
	void SendDrawImageEvent(const RFVector2&,const std::wstring&); // 00428040
protected:
	void ApplyGlobalPadding(RFRect&) const; // 00428130
public:
	int GetTextHeight(); // 00428220
	int GetNumOfLine() const; // 00428300
protected:
	void DrawCaret(float); // 00428310
	void ResetCaretBlink(); // 004283a0
public:
	static std::wstring const EventHyperLinkClicked; // 00e46e5c
	static std::wstring const EventHyperLinkEnter; // 00e46e78
	static std::wstring const EventHyperLinkLeave; // 00e46e94
	static std::wstring const EventRichTextDrawImage; // 00e46eb0
	virtual const wchar_t* ToString() const { return L"RFTextArea"; } // 00415690
	void SetPadding(const RFRect& padding) { _padding = padding; } // 004156a0
	const RFRect& GetPadding() const { return _padding; } // 004156d0
	void SetHyperLinkFontInfo(const FontInfo& font) { _hyperLinkFont = font; } // 004156e0
	void SetColor(unsigned int color) { _fontInfo.color = color; } // 00415c50
	unsigned int GetColor() const { return _fontInfo.color; } // 00415c60
	void SetShadowColor(unsigned int color) { _fontInfo.shadowColor = color; } // 00415c70
	unsigned int GetShadowColor() const { return _fontInfo.shadowColor; } // 00415c80
	void SetOutlineColor(unsigned int color) { _fontInfo.outlineColor = color; } // 00415c90
	unsigned int GetOutlineColor() const { return _fontInfo.outlineColor; } // 00415ca0
	void SetStyle(unsigned int style) { _fontInfo.style = style; } // 00415cb0
	unsigned int GetStyle() const { return _fontInfo.style; } // 00415cc0
	const FontInfo& GetFontInfo() const { return _fontInfo; } // 00415cd0
	// D857: para a visão do jogo (rfbase.h, +0x240/+0x248/+0x254 do VC10): fora de linha, com o layout real
	int GetFontSize983();
	void SetFontColor983(unsigned int color);
	float GetPaddingWidth983();
	float GetPaddingHeight983();

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFText983* _text983; // +0x180 (o RFText; nome meu; era _paragraph983)
	std::wstring _text; // +0x184
	bool _1a0;
	unsigned int _textLimit983; // +0x1a4
	unsigned int _lineLimit983; // +0x1a8
	int _1ac;
	RFCaret983* _caret983; // +0x1b0 (RFCaret criado no LoadResource; nome meu)
	FontInfo _hyperLinkFont; // +0x1b4
	int _1e4; // clique pendente (OnLButtonDown com composicao do IME ativa: o OnChar posiciona o cursor)
	float _1e8; // posicao do clique pendente (x)
	float _1ec; // (y)
	std::vector<RFHyperLink983> _hyperLinks983; // +0x1f0 (elemento 0x2c)
	bool _200;
	std::wstring _204;
	int _220;
	FontInfo _fontInfo; // +0x224
	RFRect _padding; // +0x254
	HorzAlignTypes::Type _hAlign983; // +0x264 (SetHAlign; palpite)
	int _268;
	bool _26c;
	bool _26d;
	int _270;
};

// área de texto com barras (0x294; vtable 00d63f8c)
class RF_API RFTextAreaSC : public RFTextArea
{
public:
	RFTextAreaSC(); // 0044d6a0
	virtual ~RFTextAreaSC(); // 0044d6e0
protected:
	virtual void OnHScrollPosChanged(EventArgs&); // 0044e460
	virtual void OnVScrollPosChanged(EventArgs&); // 0044e470
	virtual void OnScrollDraw(EventArgs&); // 0044e4b0
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 0044d6f0
protected:
	virtual void PreInitialize(); // 0044d910
public:
	virtual bool HitTest(const RFVector2&,bool); // 0044dc10
protected:
	virtual void OnLayoutChanged(unsigned int); // 0044dc90
	virtual void OnChar(CharEventArgs&); // 0044dcc0
	virtual void OnKeyDown(KeyEventArgs&); // 0044dce0
	virtual void OnUpdate(UpdateEventArgs&); // 0044dde0
	virtual void OnMouseWheel(MouseEventArgs&); // 0044de30
public:
	virtual void SetText(const std::wstring&); // 0044df00
	virtual void AddText(const std::wstring&); // 0044df30
	virtual void AddPlainText(const std::wstring&); // 0044df70
	virtual void ClearText(); // 0044dfb0
	void LockScrollbar(RFSCROLLBAR_TYPE,bool); // 0044dfe0
	bool IsScrollBarAvailable(RFSCROLLBAR_TYPE); // 0044e070
	void ScrollToTop(); // 0044e130
	void ScrollToBottom(); // 0044e150
protected:
	void StickToBottomProcess(); // 0044e170
	void UpdateScrollbar(); // 0044e220
public:
	static std::wstring const TypeName; // 00e47054
	virtual const wchar_t* ToString() const { return L"RFTextAreaSC"; } // 00415ce0
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00415cf0
	void StickToBottom(bool stick) { _stickToBottom = stick; } // 00415d00
	RFScrollBar* GetHScrollBar() { return _hScrollBar; } // 00415f00
	RFScrollBar* GetVScrollBar() { return _vScrollBar; } // 00415f10

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFScrollBar* _hScrollBar; // +0x274
	RFScrollBar* _vScrollBar; // +0x278
	bool _showAlwaysH; // +0x27c (<showalways hscroll vscroll>, copiados do TextAreaSCBase)
	bool _showAlwaysV; // +0x27d
	bool _autoVisibleH; // +0x27e (<autovisible ...>)
	bool _autoVisibleV; // +0x27f
	RFRect _autoVisibleRect; // +0x280 (soma/subtrai do retangulo quando a barra aparece/some)
	bool _stickToBottom; // +0x290
};

// caixa de edição (0x228; vtable 00d639ac)
class RF_API RFEdit : public RFWndEx
{
public:
	RFEdit(); // 00435000
	virtual ~RFEdit(); // 004350c0
	virtual void Create(WndBaseStruct*,RFWnd*); // 00435150
protected:
	virtual void PreInitialize(); // 00435230
	virtual void OnDestroy(); // 004352d0
	virtual void LoadResource(); // 00435310
public:
	void SetText(wchar_t const*); // 00435390
	wchar_t const* GetText() const; // 004353e0
	void ClearText(); // 00435400
protected:
	virtual void OnEnabled(bool); // 004354a0
	virtual void OnSetFocus(); // 00435510
	virtual void OnKillFocus(); // 004355d0
	virtual void OnLButtonDown(MouseEventArgs&); // 00435690
	virtual void OnKeyDown(KeyEventArgs&); // 004358a0
	virtual void OnChar(CharEventArgs&); // 00435ab0
	virtual void OnDraw(); // 00435cd0
	virtual void OnUpdate(UpdateEventArgs&); // 004361c0
	std::wstring InsertStringAtCaretPos(const std::wstring&); // 00436210
	void DeleteCharAtCaretPos(); // 004363b0
	void DeleteSelectionChar(); // 00436400
	void DeleteChar(int,int); // 00436460
	void CleanupPasswordText(); // 004364e0
	int GetTextHAlignPos(const RFRect&,const std::wstring&); // 00436530
	int GetTextVAlignPos(const RFRect&,const std::wstring&); // 004367c0
	int GetCaretPos(const std::wstring&,bool); // 00436930
public:
	void EnableCopyAndPaste(bool); // 00436a50
protected:
	void DrawCaret(const RFVector2&,float); // 00436a70
	void ResetCaretBlink(); // 00436ad0
	void ResetSelection(); // 00436af0
	void DrawSelection(const RFVector2&,float); // 00436b20
	void SetCaretLocate(int); // 00436d40
	void AddCaretLocate(int); // 00436d80
	bool KeyDownCopyAndPaste(unsigned int); // 00436dc0
	void KeyDownSelectStart(); // 00436ff0
	void KeyDownSelectEnd(); // 00437030
	void KeyDownHome(); // 00437060
	void KeyDownEnd(); // 00437080
	void KeyDownDeleteChar(bool); // 004370a0
	void KeyDownArrow(int); // 00437140
	void KeyDownReturn(KeyEventArgs&); // 00437180
public:
	static std::wstring const EventEnterKeyDown; // 00e47000
	static std::wstring const TypeName; // 00e4701c
	virtual const wchar_t* ToString() const { return L"RFEdit"; } // 00415f20
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00415f30
	bool IsEnableCopyAndPaste() const { return _copyAndPaste; } // 00415f40
	void SetLimitText(int limit) { _limitText = limit; } // 00415f50
	int GetLimitText() const { return _limitText; } // 00415f60
	virtual bool IsFocusable() const { return true; } // 00415f70
protected:
	int GetCaretLocate() const { return _caretLocate; } // 00415f80
public:
	void SetColor(unsigned int color) { _fontInfo.color = color; } // 00416400
	unsigned int GetColor() const { return _fontInfo.color; } // 00416410
	void SetShadowColor(unsigned int color) { _fontInfo.shadowColor = color; } // 00416420
	unsigned int GetShadowColor() const { return _fontInfo.shadowColor; } // 00416430
	void SetOutlineColor(unsigned int color) { _fontInfo.outlineColor = color; } // 00416440
	unsigned int GetOutlineColor() const { return _fontInfo.outlineColor; } // 00416450
	void SetSelectionColor(unsigned int color, unsigned int bgColor) { _selColor = color; _selBgColor = bgColor; } // 00416460
	void SetStyle(unsigned int style) { _fontInfo.style = style; } // 00416480
	unsigned int GetStyle() const { return _fontInfo.style; } // 00416490
	void SetPadding(const RFRect& padding) { _padding = padding; } // 004164a0
	const RFRect& GetPadding() const { return _padding; } // 004164d0
	void SetPasswordChar(wchar_t ch) { _passwordChar = ch; } // 004164e0
	wchar_t GetPasswordChar() const { return _passwordChar; } // 00416500
	bool IsFocused() const { return _focused; } // 00416510
	void SetValidator(RFValidator* validator) { _validator = validator; } // 00416520
	RFValidator* GetValidator() const { return _validator; } // 00416530

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	std::wstring _text; // +0x180
	bool _focused; // +0x19c
	bool _copyAndPaste; // +0x19d
	int _limitText; // +0x1a0
	int _scrollX; // +0x1a4 (x do texto na janela: GetTextHAlignPos)
	int _scrollY; // +0x1a8 (y do texto: GetTextVAlignPos)
	int _caretLocate; // +0x1ac
	bool _selecting; // +0x1b0 (ha trecho de composicao do IME selecionado)
	char _1b1[3];
	int _selStart; // +0x1b4
	int _selEnd; // +0x1b8
	RFCaret983* _caret983; // +0x1bc (criado no LoadResource se ha imagem; apagado no OnDestroy)
	int _shiftBegin; // +0x1c0 (selecao com Shift: inicio)
	int _shiftEnd; // +0x1c4 (fim)
	bool _1c8;
	RFValidator* _validator; // +0x1cc
	unsigned int _selColor; // +0x1d0
	unsigned int _selBgColor; // +0x1d4
	FontInfo _fontInfo; // +0x1d8
	HorzAlignTypes::Type _hAlign983; // +0x208 (palpite)
	VertAlignTypes::Type _vAlign983; // +0x20c (palpite)
	RFRect _padding; // +0x210
	wchar_t _passwordChar; // +0x220
	ImageSet* _caretImageSet; // +0x224 (imagem do cursor: <caret> do EditBase)
};

// caixa de combinação (0x1b8; vtable 00d6390c)
class RF_API RFComboBox : public RFWnd
{
public:
	RFComboBox(); // 00454950
	virtual ~RFComboBox(); // 00454980
protected:
	virtual void OnCBButClicked(EventArgs&); // 004556c0
	virtual void OnListSelChanged(EventArgs&); // 00455710
	virtual void OnListBeginDrawItem(EventArgs&); // 00455880
	virtual void OnListDrawItem(EventArgs&); // 00455890
	virtual void OnListEndDrawItem(EventArgs&); // 00455b60
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 004549d0
	virtual void Destroy(); // 00454ca0
protected:
	virtual void PreInitialize(); // 00454cb0
	virtual void OnShow(bool); // 00455080
	void ShowListArea(bool); // 00455140
public:
	virtual bool HitTest(const RFVector2&,bool); // 004551a0
	void AddText(const std::wstring&); // 00455220
	void SetCurSel(int); // 004552b0
	int GetCurSel() const; // 00455360
	const std::wstring& GetText(int) const; // 00455370
	const std::wstring& GetCurText() const; // 004553f0
	int GetItemCount() const; // 00455460
	void DeleteAllItems(); // 00455480
	bool IsValidItemidx(int) const; // 00455570
protected:
	virtual void OnMouseWheel(MouseEventArgs&); // 004555a0
	virtual void OnKillFocus(); // 00455680
public:
	static std::wstring const EventSelChanged; // 00e470e0
	static std::wstring const TypeName; // 00e470fc
	virtual const wchar_t* ToString() const { return L"RFComboBox"; } // 00416540
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00416550
	void EnableMWheelSelect(bool enable) { _mWheelSelect = enable; } // 00416560
	RFListSC* GetListArea() const { return _listArea; } // 00416570

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFButton* _button; // +0x178 (<control type="BUTTON">: abre/fecha a lista; _priority = -1)
	RFTextAreaSC* _textArea; // +0x17c (texto da selecao atual)
	RFListSC* _listArea; // +0x180
	bool _listShown; // +0x184 (lista aberta)
	int _curSel983; // +0x188
	bool _mWheelSelect; // +0x18c
	unsigned int _normalColor983; // +0x190 (cor do texto do item)
	unsigned int _selectColor983; // +0x194 (item sob o mouse ou selecionado)
	std::wstring _fontName983; // +0x198
	int _fontSize983; // +0x1b4
};

class RFSlider;
// argumentos do RFSlider::EventPosChanged (nao exportada; vtable 00d65ba0)
class SliderPosChangedEventArgs : public WindowEventArgs_<RFSlider>
{
public:
	SliderPosChangedEventArgs(RFSlider* wnd, int pos) : WindowEventArgs_<RFSlider>(wnd), pos(pos) { }
	int pos; // +0xc
};

// controle deslizante (0x1ac; vtable 00d63e54)
class RF_API RFSlider : public RFWnd
{
public:
	RFSlider(); // 0044ed60
	virtual ~RFSlider(); // 0044eda0
protected:
	virtual void OnLTButClicked(EventArgs&); // 0044f9d0
	virtual void OnRBButClicked(EventArgs&); // 0044fa80
	virtual void OnThumbMouseMove(EventArgs&); // 0044f8e0
public:
	virtual void Create(WndBaseStruct*,RFWnd*); // 0044edb0
protected:
	virtual void PreInitialize(); // 0044ef60
	virtual void OnEnabled(bool); // 0044f1e0
	virtual void OnLButtonDown(MouseEventArgs&); // 0044f270
	virtual void OnLButtonUp(MouseEventArgs&); // 0044f420
public:
	void SetSlideSize(int); // 0044f470
	void SetSlideRange(int,int); // 0044f480
	void SetSlidePos(int); // 0044f530
	int GetSlidePos() const; // 0044f5f0
	void Next(); // 0044f600
	void Prev(); // 0044f660
	void SlideToTop(); // 0044f6c0
	void SlideToBottom(); // 0044f6d0
protected:
	virtual void OnMouseMove(MouseEventArgs&); // 0044f6e0
	virtual void OnMouseWheel(MouseEventArgs&); // 0044f870
	void PositionToLevel(const RFVector2&); // 0044fb30
	void LevelToThumbPosition(); // 0044fcb0
public:
	static std::wstring const EventPosChanged; // 00e4708c
	static std::wstring const TypeName; // 00e470a8
	virtual const wchar_t* ToString() const { return L"RFSlider"; } // 00416810
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00416820

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFButton* _ltButton983; // +0x178 (<control type="BUTTON_LT">)
	RFButton* _rbButton983; // +0x17c ("BUTTON_RB")
	RFThumbButton* _thumb983; // +0x180 ("THUMB")
	RFPoint _range983; // +0x184 (x = minimo, y = maximo; Create: 0..5)
	int _pos983; // +0x18c
	int _size983; // +0x190 (passo dos botoes)
	bool _dragging983; // +0x194 (OnLButtonDown ate o OnLButtonUp)
	RFRect _slideRect983; // +0x198 (SliderBase::_slideRect; relativo ao controle)
	bool _horizontal983; // +0x1a8
};

// painel empilhado (0x184; vtable 00d640fc)
class RF_API RFStackPanel : public RFWnd
{
public:
	RFStackPanel(); // 0044e710
	virtual ~RFStackPanel(); // 0044e730
	virtual void Initialize(); // 0044e740
	virtual void Create(WndBaseStruct*,RFWnd*); // 0044e780
	OrientationTypes::Type GetOrientation() const; // 0044e7c0
	void SetOrientation(OrientationTypes::Type); // 0044e7d0
protected:
	virtual void OnLayoutChanged(unsigned int); // 0044e7e0
public:
	void StackUp(); // 0044e840
protected:
	void StackUp_Impl(); // 0044e890
	void StackUpVertically(); // 0044e8c0
	void StackUpHorizontally(); // 0044eac0
public:
	static std::wstring const TypeName; // 00e47070
	virtual const wchar_t* ToString() const { return L"RFStackPanel"; } // 00416a50
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00416a60

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	int _hAlign983; // +0x178 (HorzAlignTypes: posição de cada filho na coluna; do StackPanelBase::_hAlign)
	int _vAlign983; // +0x17c (VertAlignTypes: posição do bloco inteiro; do StackPanelBase::_vAlign)
	OrientationTypes::Type _orientation; // +0x180
};

// figura (0x188; vtable 00d63c9c)
class RF_API RFPicture : public RFWnd
{
public:
	RFPicture(); // 0041e400
	virtual ~RFPicture(); // 0041e420
	virtual void Create(WndBaseStruct*,RFWnd*); // 0041e430
protected:
	virtual void PreInitialize(); // 0041e470
public:
	virtual void Destroy(); // 0041e4d0
protected:
	virtual void LoadResource(); // 0041e520
	virtual void OnDestroy(); // 0041e630
	virtual void OnLayoutChanged(unsigned int); // 0041e660
public:
	void LoadAnimation(const std::wstring&); // 0041e6c0
	void UnloadAnimation(); // 0041e7d0
	R_ERRORTYPE SetAnimation(RFDirector*); // 0041e800
private:
	void LoadAnimation_(const std::wstring&,const std::wstring&); // 0041e900
public:
	void SetAnimPos(float,float); // 0041e9d0
	void Play(const std::wstring&); // 0041ea00
	void Stop(); // 0041ea80
	bool IsPlaying() const; // 0041eaa0
protected:
	virtual void OnUpdate(UpdateEventArgs&); // 0041eac0
	virtual void OnDraw(); // 0041eb20
public:
	void SetPicture(const std::wstring&); // 0041ed00
	void SetPatchPicture(const std::wstring&,float,float,float,float); // 0041ede0
	RFSize GetTextureSize() const; // 0041eef0
	void FitToContent(); // 0041ef30
	static std::wstring const TypeName; // 00e46cf0
	virtual const wchar_t* ToString() const { return L"RFPicture"; } // 00416ba0
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00416bb0
	RFDirector* GetDirector() { return _director; } // 00416bc0

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	RFDirector* _director; // +0x178
	bool _static983; // +0x17c (PictureBase::_static; false = animação "ANIMPICTURE" pelo _director)
	RFBrush* _brush983; // +0x180 (RFBrush::Create do _imageSet983)
	ImageSet* _imageSet983; // +0x184 (da PictureBase, ou criado pelo SetPicture/SetPatchPicture)
};

// barra de progresso (0x194; vtable 00d63d2c)
class RF_API RFProgress : public RFWnd
{
public:
	RFProgress(); // 0044d2b0
	virtual ~RFProgress(); // 0044d2d0
	virtual void Create(WndBaseStruct*,RFWnd*); // 0044d2e0
protected:
	virtual void PreInitialize(); // 0044d310
	virtual void LoadResource(); // 0044d3a0
	virtual void OnDestroy(); // 0044d3c0
	virtual void OnDraw(); // 0044d3f0
public:
	void SetRange(int,int); // 0044d620
	void SetProgress(int); // 0044d660
	int GetProgress() const; // 0044d690
	static std::wstring const TypeName; // 00e47038
	virtual const wchar_t* ToString() const { return L"RFProgress"; } // 00416d10
	virtual const std::wstring& GetTypeName() { return TypeName; } // 00416d20
	void SetHorizontal(bool horizontal) { _horizontal = horizontal; } // 00416d30
	bool IsHorizontal() const { return _horizontal; } // 00416d40

public: // (campos: pelos construtores de cópia do 983; nomes meus salvo os dos acessores)
	bool _horizontal; // +0x178
	int _min983; // +0x17c
	int _max983; // +0x180
	int _progress983; // +0x184
	bool _188;
	RFBrush* _brush983; // +0x18c (palpite)
	ImageSet* _imageSet983; // +0x190 (palpite)
};

// tamanhos do 983 (push <tamanho> dos destrutores de apagar 00414270…00416e70; RFScrollBar pelo construtor de cópia)
C_ASSERT_VC10(sizeof(FontInfo) == 0x30);
C_ASSERT_VC10(sizeof(RFWndEx) == 0x180);
C_ASSERT_VC10(sizeof(RFButton) == 0x1f4);
C_ASSERT_VC10(sizeof(RFLabel) == 0x1e4);
C_ASSERT_VC10(sizeof(RFLabelButton) == 0x210);
C_ASSERT_VC10(sizeof(RFAnimButton) == 0x208);
C_ASSERT_VC10(sizeof(RFCheck) == 0x1a8);
C_ASSERT_VC10(sizeof(RFList) == 0x1dc);
C_ASSERT_VC10(sizeof(RFScrollBar) == 0x1d4);
C_ASSERT_VC10(sizeof(RFListSC) == 0x1e8);
C_ASSERT_VC10(sizeof(RFTextArea) == 0x274);
C_ASSERT_VC10(sizeof(RFTextAreaSC) == 0x294);
C_ASSERT_VC10(sizeof(RFEdit) == 0x228);
C_ASSERT_VC10(sizeof(RFComboBox) == 0x1b8);
C_ASSERT_VC10(sizeof(RFSlider) == 0x1ac);
C_ASSERT_VC10(sizeof(RFStackPanel) == 0x184);
C_ASSERT_VC10(sizeof(RFPicture) == 0x188);
C_ASSERT_VC10(sizeof(RFProgress) == 0x194);
