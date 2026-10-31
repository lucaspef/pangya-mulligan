// JP 983: Refresh, src\RFManager.cpp (F3.7.13; LTCG — prova pela dll de prova, ver out/F3.7/s145/RELATORIO.md). 1º lote (B145): os
// leitores de texto das propriedades do layout (RFProperty::SetValue de cada derivada, 00464f70–00465240) e os conversores que
// eles usam (00419db0–0041a3b0, 0042aaa0, 00464f40). Asserts com o __FILE__/linha do 983 (#line antes de cada função com assert).
// Nomes das funções livres meus (…983).
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include "vc10_layout.h"
#include <windows.h>
#include <malloc.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "refresh/rfproperty.h"

#define RF_ARGB983(a, r, g, b) ((unsigned long)((((a) & 0xff) << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff)))

// 0042aaa0: texto UTF-8 → wstring (buffer na pilha)
std::wstring RFUtf8ToWide983(const char* text)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)_alloca(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, text, -1, buf, len);
	return std::wstring(buf);
}

// 00464f40
std::wstring RFUtf8ToWide983(const std::string& text)
{
	return RFUtf8ToWide983(text.c_str());
}

#line 1175 "src\\RFManager.cpp"
static bool RFParseBoolean983(const std::string& string)
{
	if (!_stricmp(string.c_str(), "True"))
		return true;
	if (_stricmp(string.c_str(), "False"))
		assert(!"Invalid Boolean string.");
	return false;
}

#line 1187 "src\\RFManager.cpp"
// 00419db0: "#RRGGBB" / "#RRGGBBAA" ou número
static unsigned long RFParseColor983(const std::string& string)
{
	if (string.empty())
		return 0;

	if (string[0] == '#')
	{
		assert(string.size() == 7 || string.size() == 9);

		int rgba[4] = { 255, 255, 255, 255 };
		const char* p = string.c_str() + 1;
		const std::string::size_type count = (string.size() - 1) / 2;
		for (std::string::size_type i = 0; i < count; ++i)
			sscanf_s(p + i * 2, "%02x", &rgba[i]);

		return RF_ARGB983(rgba[3], rgba[0], rgba[1], rgba[2]);
	}

	return atoi(string.c_str());
}

#line 1241 "src\\RFManager.cpp"
// 00419f30: "Left, Top, …"
static int RFParseAnchor983(const std::string& string)
{
	const char delimiter[] = ", ";
	int anchor = 0;
	std::string token;
	std::string::size_type pos = 0;

	for (;;)
	{
		std::string::size_type next = string.find(delimiter, pos);
		std::string::size_type end = (next == std::string::npos) ? string.size() : next;
		token = string.substr(pos, end - pos);
		pos = end + (next != std::string::npos ? 2 : 0);

		if (!_stricmp(token.c_str(), "Left"))
			anchor |= 1;
		else if (!_stricmp(token.c_str(), "Top"))
			anchor |= 2;
		else if (!_stricmp(token.c_str(), "Right"))
			anchor |= 4;
		else if (!_stricmp(token.c_str(), "Bottom"))
			anchor |= 8;
		else if (!_stricmp(token.c_str(), "None"))
			anchor = 0;
		else
			assert(!"Invalid Anchor string.");

		if (pos == string.size())
			break;
	}
	return anchor;
}

#line 1282 "src\\RFManager.cpp"
// 0041a170
static int RFParseHAlign983(const std::string& string)
{
	if (isdigit(string.at(0)))
		return atoi(string.c_str());

	if (!_stricmp(string.c_str(), "Left"))
		return 0;
	if (!_stricmp(string.c_str(), "Center"))
		return 1;
	if (!_stricmp(string.c_str(), "Right"))
		return 2;

	assert(!"Invalid HorzAlignTypes string.");
	return 0;
}

#line 1305 "src\\RFManager.cpp"
// 0041a240
static int RFParseVAlign983(const std::string& string)
{
	if (isdigit(string.at(0)))
		return atoi(string.c_str());

	if (!_stricmp(string.c_str(), "Top"))
		return 0;
	if (!_stricmp(string.c_str(), "Center"))
		return 1;
	if (!_stricmp(string.c_str(), "Bottom"))
		return 2;

	assert(!"Invalid VertAlignTypes string.");
	return 0;
}

#line 1327 "src\\RFManager.cpp"
// 0041a310
static int RFParseTextTrimming983(const std::string& string)
{
	if (!_stricmp(string.c_str(), "None"))
		return 0;
	if (!_stricmp(string.c_str(), "CharacterEllipsis"))
		return 1;

	assert(!"Invalid TextTrimmingTypes string.");
	return 0;
}

// 00464ff0
void RFBooleanProperty::SetValue(const std::string& value)
{
	_value = RFParseBoolean983(value);
}

// 00465070
void RFIntegerProperty::SetValue(const std::string& value)
{
	_value = atoi(value.c_str());
}

// 00464f70
void RFStringProperty::SetValue(const std::string& value)
{
	_value = RFUtf8ToWide983(value);
}

// 004650a0
void RFStartPositionProperty::SetValue(const std::string& value)
{
	if (!_stricmp(value.c_str(), "CenterParent"))
		_value = 1;
	else
		_value = _stricmp(value.c_str(), "CenterScreen") ? 0 : 2;
}

// 00465110
void RFEscKeyActionProperty::SetValue(const std::string& value)
{
	_value = _stricmp(value.c_str(), "Close") ? 0 : 1;
}

// 00465140
void RFAnchorProperty::SetValue(const std::string& value)
{
	_value = RFParseAnchor983(value);
}

// 00465160
void RFColorProperty::SetValue(const std::string& value)
{
	_value = RFParseColor983(value);
}

// 00465180
void RFHAlignProperty::SetValue(const std::string& value)
{
	_value = RFParseHAlign983(value);
}

// 004651a0
void RFVAlignProperty::SetValue(const std::string& value)
{
	_value = RFParseVAlign983(value);
}

// 004651c0
void RFTextTrimmingProperty::SetValue(const std::string& value)
{
	_value = RFParseTextTrimming983(value);
}

// 004651e0
void RFOrientationProperty::SetValue(const std::string& value)
{
	if (!_stricmp(value.c_str(), "Vertical"))
		_value = 0;
	else
		_value = _stricmp(value.c_str(), "Horizontal") ? 0 : 1;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// F3.7.13 item (4) 1º lote (B151): a classe RFManager (00417450–00419c40) — singleton, janelas, foco/ativação, modais, teste de
// acerto e os avisos das janelas. Faltam (próximos lotes): construtor/destrutor (dependem das 14 fábricas padrão 00433b90–004343b0 do
// item (5)), CreateWnd/CreateChild, Post*Event, SetRenderer(nome), UpdateInputMethod e os Inject* restantes.
#undef CreateDesktop // windows.h (CreateDesktopW); aqui é o RFManager::CreateDesktop 00417a20
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfime.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/rffactorymanager.h"
#include "refresh/rfwndstruct.h"

// vista do RFXmlLoader (refresh/rfxmlloader.h não convive com os cabeçalhos da lib numa unidade: RFSize/RFRect/RFInterface
// das visões velhas): só os membros que o CreateWnd/CreateChild chamam, com os mesmos nomes decorados, e o tamanho real (0xa8,
// objeto na pilha do CreateWnd). O GetAnimationPath fora de linha (00416ee0) é a cópia desta unidade, como no 983.
class RFXmlLoader
{
public:
	explicit RFXmlLoader(RFInterface* rfInterface); // 0043dc30
	virtual ~RFXmlLoader(); // 0043de70
	int Load(const std::wstring& filename); // 0043e380
	void Clear(); // 0043df70
	WndBaseStruct* GetRoot() const; // 0043e5c0
	WndBaseStruct* GetChild(WndBaseStruct* parent, int index); // 0043e5d0
	int GetChildCount(WndBaseStruct* parent); // 0043e620 (−1 quando a estrutura não tem filhas registradas)
	std::wstring GetAnimationPath() const { return _animationPath; } // 00416ee0

private: // membros com os nomes e deslocamentos da visão velha (refresh/rfxmlloader.h); tipos da lib onde convivem
	unsigned int _version; // +0x04 (atributo "version" da raiz)
	bool _loaded; // +0x08
	unsigned long _document[0x12]; // +0x0c (rf::TiXmlDocument, 0x48 bytes)
	RFInterface* _interface; // +0x54
	std::vector<WndBaseStruct*> _structs; // +0x58 (janelas de topo; a 1ª é a raiz)
	std::map<WndBaseStruct*, std::vector<WndBaseStruct*> > _children; // +0x68 (filhas de cada estrutura)
	std::wstring _animationPath; // +0x78 (propriedade "UIAnimation")
	WndBaseStruct* _style; // +0x94 (última estrutura lida de um arquivo de estilo)
	std::map<std::string, void*> _imageSets; // +0x98 (ImageSet*; sizeof 0xa8)
};
C_ASSERT_VC10(sizeof(RFXmlLoader) == 0xa8);


// globais do RFManager.cpp (nomes meus)
RFManager* RFSingleton<RFManager>::_singleton; // 00f1de10
RFInterface* g_pRFInterface983; // 00f1de14
RFWnd* g_pRFRoot983; // 00f1de18
RFWnd* g_pRFDesktop983; // 00f1de1c
RFRenderer* g_pRFRenderer983; // 00f1de20

// 00e46c64–00e46cb8 (inicializadores em 00cc7600–00cc76f0)
const std::wstring RFManager::EventHyperLink = L"HyperLink";
const std::wstring RFManager::EventGlobalHyperLinkEnter = L"GlobalHyperLinkEnter";
const std::wstring RFManager::EventGlobalHyperLinkLeave = L"GlobalHyperLinkLeave";
const std::wstring RFManager::EventInputMethodStatusChanged = L"IMStatusChanged";

// dll do desenhista do SetRenderer(nome, …) (bloco de 4 bytes do 983; nome meu)
struct RFRendererModule983
{
	RFRendererModule983(const std::wstring& name) : module(LoadLibraryW(name.c_str())) { }
	~RFRendererModule983()
	{
		if (module)
		{
			FreeLibrary(module);
			module = 0;
		}
	}

	HMODULE module;
};

// RFInputMethodProvider/RFIme/RFCustomIme: refresh/lib/rfime.h (rfime.cpp, B163).

// fábricas padrão das janelas (RTTI RF<tipo>Factory; cada uma é um estático de função, 00433b90–004343b0)
#define RF_WINDOW_FACTORY983(T) \
	class T##Factory : public RFWindowFactory \
	{ \
	public: \
		T##Factory() : RFWindowFactory(T::TypeName) { } \
		virtual RFWnd* Create() { return new T; } \
		virtual void Destroy(RFWnd* wnd) { delete wnd; } \
	}; \
	static RFWindowFactory* Get##T##Factory983() \
	{ \
		static T##Factory factory; \
		return &factory; \
	}
RF_WINDOW_FACTORY983(RFAnimButton)
RF_WINDOW_FACTORY983(RFButton) // 00433c30
RF_WINDOW_FACTORY983(RFCheck)
RF_WINDOW_FACTORY983(RFComboBox) // (B265)
RF_WINDOW_FACTORY983(RFEdit) // (B265; o RFEdit e da B214)
RF_WINDOW_FACTORY983(RFLabelButton)
RF_WINDOW_FACTORY983(RFLabel) // 00433eb0
RF_WINDOW_FACTORY983(RFListSC)
RF_WINDOW_FACTORY983(RFPicture) // 00434090
RF_WINDOW_FACTORY983(RFProgress) // (B211)
RF_WINDOW_FACTORY983(RFScrollBar) // 004341d0
RF_WINDOW_FACTORY983(RFSlider) // (B213)
RF_WINDOW_FACTORY983(RFStackPanel) // (B209)
RF_WINDOW_FACTORY983(RFTextAreaSC) // (B265)
RF_WINDOW_FACTORY983(RFWndEx) // 00434450

// 00417490
RFManager::RFManager()
	: _rendererModule983(0), _renderer(0), _interface(0), _inputMethod(0), _inputsFrozen(false), _root(0), _desktop(0),
	  _activeWnd(0), _focusWnd(0), _capturedWnd(0), _mouseOverWnd(0), _visualEffect(0), _globalPaddingEnabled(false),
	  _usingCustomInputMethod(false)
{
	_cursorPos = RFVector2(0.0f, 0.0f);
	_factoryManager = new RFWindowFactoryManager;
	// o 983 registra as 15 (AnimButton, Button, Check, ComboBox, Edit, WndEx, Label, LabelButton, ListSC, Picture, Progress,
	// ScrollBar, Slider, StackPanel, TextAreaSC). Só as das classes que já têm construtor aqui (B155 WndEx; B156 Button, Label,
	// Picture; B160 AnimButton, Check, LabelButton; B161 ScrollBar; B162 ListSC; B209 StackPanel; B211 Progress; B213 Slider; B214 Edit; B265 TextAreaSC e ComboBox: as 15 do 983); sem fábrica o CreateChild pula o controle (com fábrica e construtor stub, o objeto ficaria sem vtable).
	_factoryManager->AddFactory(GetRFAnimButtonFactory983());
	_factoryManager->AddFactory(GetRFButtonFactory983());
	_factoryManager->AddFactory(GetRFCheckFactory983());
	_factoryManager->AddFactory(GetRFComboBoxFactory983());
	_factoryManager->AddFactory(GetRFEditFactory983());
	_factoryManager->AddFactory(GetRFWndExFactory983());
	_factoryManager->AddFactory(GetRFLabelFactory983());
	_factoryManager->AddFactory(GetRFLabelButtonFactory983());
	_factoryManager->AddFactory(GetRFListSCFactory983());
	_factoryManager->AddFactory(GetRFPictureFactory983());
	_factoryManager->AddFactory(GetRFScrollBarFactory983());
	_factoryManager->AddFactory(GetRFProgressFactory983());
	_factoryManager->AddFactory(GetRFSliderFactory983());
	_factoryManager->AddFactory(GetRFStackPanelFactory983());
	_factoryManager->AddFactory(GetRFTextAreaSCFactory983());
	SetVisualEffect(CreateVisualEffect983(0));

	_root = new RFWnd;
	_root->Initialize();
	_root->_active = true;
	_root->SetName(L"__refresh_system_layer__");
	const RFSize size(1024.0f, 768.0f);
	if (_root->_localSize.width != size.width || _root->_localSize.height != size.height)
	{
		_root->_localSize = size;
		_root->OnLayoutChanged(2);
	}
	g_pRFRoot983 = _root;
}

// 004177f0
RFManager::~RFManager()
{
	delete _factoryManager;
	if (_visualEffect)
	{
		delete _visualEffect;
		_visualEffect = 0;
	}
	_root->DeleteAllChildren();
	if (_root)
	{
		delete _root;
		_root = 0;
	}
	if (_rendererModule983)
	{
		RFRendererModule983* module = (RFRendererModule983*)_rendererModule983;
		typedef void (*DeleteRenderer)();
		DeleteRenderer deleteRenderer = (DeleteRenderer)GetProcAddress(module->module, std::string("deleteRenderer").c_str());
		deleteRenderer();
		_renderer = 0;
		g_pRFRenderer983 = 0;
		delete module;
	}
	if (_inputMethod)
	{
		if (RFInputMethodService* ime = _inputMethod->_ime)
		{
			_inputMethod->_ime = 0;
			delete ime;
		}
		if (_inputMethod)
		{
			delete _inputMethod;
			_inputMethod = 0;
		}
	}
}

// 00417450
#line 56 "src\\RFManager.cpp"
RFManager& __fastcall RFManager::GetInstance()
{
	assert(_singleton);
	return *_singleton;
}

// 00417480
RFManager* __fastcall RFManager::GetInstancePtr()
{
	return _singleton;
}

// 00417e70
void RFManager::FreezeInputs(bool freeze)
{
	_inputsFrozen = freeze;
}

// 00417e80
bool RFManager::IsInputsFroze() const
{
	return _inputsFrozen;
}

// 004182f0
void RFManager::ActivateWindow(RFWnd* wnd)
{
	if (wnd && wnd != _root && wnd != _activeWnd && wnd->_overlapped)
	{
		if (_activeWnd)
			_activeWnd->Deactivate();
		_activeWnd = wnd;
		wnd->Activate();
		_activeWnd->_super->BringChildWndToTop(_activeWnd);
	}
}

// 00418340: o foco vai para o controle de cima quando a janela é parte de um controle
void RFManager::FocusWindow(RFWnd* wnd)
{
	if (!wnd)
	{
		if (_focusWnd)
		{
			_focusWnd->OnKillFocus();
			_focusWnd = 0;
		}
		return;
	}
	RFWnd* old = _focusWnd;
	if (wnd->_subControl)
		_focusWnd = wnd->GetControlSuper();
	else
		_focusWnd = wnd;
	if (old != _focusWnd)
	{
		if (old)
			old->OnKillFocus();
		if (_focusWnd)
			_focusWnd->OnSetFocus();
	}
}

// 004183b0
R_ERRORTYPE RFManager::DeleteDesktop()
{
	R_ERRORTYPE ret = DeleteWnd(_desktop);
	g_pRFDesktop983 = 0;
	return ret;
}

// 00418410
#line 371 "src\\RFManager.cpp"
R_ERRORTYPE RFManager::DeleteWnd(RFWnd* wnd)
{
	if (!wnd)
		return R_FAIL983;
	NotifyWindowDestroyed(wnd);
	RFWnd* parent = wnd->_super;
	assert(parent);
	parent->DeleteChild(wnd);
	return R_OK983;
}

// 00418460
void RFManager::DeleteAllWnd()
{
	_root->DeleteAllChildren();
}

// 00418470: a área de trabalho fica atrás das outras (prioridade 1)
void RFManager::RegisterDesktop(RFWnd* wnd)
{
	_desktop = wnd;
	g_pRFDesktop983 = wnd;
	wnd->_priority = 1;
	if (wnd->_super)
		wnd->_super->_priorityInvalid = true;
	_root->RegisterChild(wnd);
}

// 004184b0
RFWnd* RFManager::GetDesktop()
{
	return _desktop;
}

// D856: acessos das visões do jogo (rfscenemanager.h/lobbyjp.h) com o layout real da lib
RFManager* RFManager::New983()
{
	return new RFManager;
}

RFWnd* RFManager::GetRoot983()
{
	return _root;
}

RFWnd* RFManager::GetFocusWnd983()
{
	return _focusWnd;
}

// visão do lobbyjp.h (CUiPointer983::GetScreenSize983 00565570) sobre o mesmo objeto; só o acessor
class CRFWndJp;
class CRFManagerJp
{
public:
	CRFWndJp* GetRoot983();
};

CRFWndJp* CRFManagerJp::GetRoot983()
{
	return reinterpret_cast<CRFWndJp*>(reinterpret_cast<RFManager*>(this)->_root);
}

// 004184c0
RFWnd* RFManager::GetWnd(const std::wstring& name)
{
	return _root->GetChild(name);
}

// 004184d0
RFWnd* RFManager::GetInputTargetWindow() const
{
	if (_activeWnd)
		return _activeWnd->GetActiveChild();
	return 0;
}

// 004184e0
void RFManager::SetRenderer(RFRenderer* renderer)
{
	_renderer = renderer;
	g_pRFRenderer983 = renderer;
}

// 00418630
RFRenderer* RFManager::GetRenderer()
{
	return _renderer;
}

// 00418640
void RFManager::SetInterface(RFInterface* iface)
{
	_interface = iface;
	g_pRFInterface983 = iface;
}

// 00418660
RFInterface* RFManager::GetInterface()
{
	return _interface;
}

// 004187c0
RFInputMethodProvider* RFManager::GetInputMethod()
{
	return _inputMethod;
}

// 004187d0
void RFManager::BringWndToTop(RFWnd* wnd)
{
	_root->BringChildWndToTop(wnd);
}

// 004187e0: só os controles que abrem por cima (tipos 3–6 e 9–13) sobem entre os irmãos
void RFManager::BringControlToTop(RFWnd* wnd)
{
	if (wnd->_subControl)
		wnd = wnd->GetControlSuper();
	if (wnd->_super)
	{
		switch (wnd->_type)
		{
		case 3: case 4: case 5: case 6:
		case 9: case 10: case 11: case 12: case 13:
			wnd->_super->BringChildWndToTop(wnd);
			break;
		}
	}
}

// 00418860: aceita o número de um efeito pronto (< 0x10) no lugar do ponteiro, como o RFWnd::SetVisualEffect
void RFManager::SetVisualEffect(RFVisualEffect* effect)
{
	if (_visualEffect)
	{
		delete _visualEffect;
		_visualEffect = 0;
	}
	if ((unsigned int)effect < 0x10)
		_visualEffect = CreateVisualEffect983((int)effect);
	else
		_visualEffect = effect;
}

// 004188a0
const wchar_t* RFManager::GetVersion() const
{
	return L"librefresh-1.5.5 Release (Thu Jun 18 21:25:25 2015) - Japan";
}

// 004188b0
RFWnd* RFManager::HitTest(const RFVector2& pt, bool a2)
{
	return _root->HitTestChild(pt, false, a2);
}

// 004188d0: com modal aberta, só ela (e os filhos) recebe o mouse
RFWnd* RFManager::HitTest2(const RFVector2& pt, bool a2, bool a3)
{
	RFWnd* modal = GetLastActiveModal();
	if (modal)
		return modal->HitTestChild(pt, a2, a3);
	RFWnd* wnd = _root->HitTestChild(pt, a2, a3);
	return wnd != _root ? wnd : 0;
}

// 00418950: a modal de cima (filhos da raiz de trás para a frente)
RFWnd* RFManager::GetLastActiveModal() const
{
	for (std::vector<RFWnd*>::reverse_iterator itor = _root->_children.rbegin(); itor != _root->_children.rend(); ++itor)
		if ((*itor)->_modal)
			return *itor;
	return 0;
}

// 00418990
bool RFManager::IsModalExist() const
{
	for (std::vector<RFWnd*>::reverse_iterator itor = _root->_children.rbegin(); itor != _root->_children.rend(); ++itor)
		if ((*itor)->_modal)
			return true;
	return false;
}

// 004189d0: a janela de cima que pode ficar com o foco (menos `except`)
RFWnd* RFManager::FindActiveFocusableWnd(RFWnd* except)
{
	for (std::vector<RFWnd*>::reverse_iterator itor = _root->_children.rbegin(); itor != _root->_children.rend(); ++itor)
	{
		RFWnd* wnd = *itor;
		if (except == wnd)
			continue;
		if (wnd->_priority <= -1 && !wnd->_modal)
			continue;
		if (_desktop && wnd == _desktop)
			return wnd;
		if (!wnd->_hitTestVisible || wnd->_hiding == true || !wnd->_visible)
			continue;
		if (!wnd->_super || wnd->_super->IsWindowVisible(false))
			return wnd;
	}
	return 0;
}

// 00418a60: OnMouseLeave/OnMouseEnter (com captura, só para a janela capturada)
void RFManager::UpdateWindowContainingMouse(const RFVector2& pt)
{
	RFWnd* wnd = HitTest2(pt, true, false);
	if (!wnd)
		return;
	if (!_mouseOverWnd)
		_mouseOverWnd = _root;
	if (wnd == _mouseOverWnd)
		return;
	if (!_capturedWnd || _mouseOverWnd == _capturedWnd)
		_mouseOverWnd->OnMouseLeave();
	if (!_capturedWnd || wnd == _capturedWnd)
	{
		BringControlToTop(wnd);
		wnd->OnMouseEnter();
	}
	_mouseOverWnd = wnd;
}

// 00418ad0
bool RFManager::InjectLButtonDown(float x, float y)
{
	if (IsInputsFroze())
		return false;
	RFWnd* wnd = _capturedWnd;
	if (!wnd)
	{
		wnd = HitTest2(RFVector2(x, y), true, false);
		if (!wnd)
			return false;
	}
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	ActivateWindow(overlapped);
	FocusWindow(wnd);
	MouseEventArgs e(wnd, RFVector2(x, y), 0, 0.0f);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnLButtonDown(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00419010: fora da raiz não conta
bool RFManager::InjectMouseMove(float x, float y)
{
	RFSize size = _root->GetSize();
	if (x < 0.0f || size.width < x || y < 0.0f || size.height < y)
		return false;
	RFVector2 pt(x, y);
	UpdateWindowContainingMouse(pt);
	_cursorPos = pt;
	RFWnd* wnd = _capturedWnd;
	if (!wnd)
	{
		wnd = HitTest2(RFVector2(x, y), true, false);
		if (!wnd)
			return false;
	}
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	MouseEventArgs e(wnd, pt, 3, 0.0f);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnMouseMove(e);
		if (e.handled)
			break;
	}
	return true;
}

// 004198d0
void RFManager::NotifyWindowGotFocus(RFWnd* wnd)
{
	if (_focusWnd && wnd != _focusWnd)
		_focusWnd->OnKillFocus();
	_focusWnd = wnd;
}

// 00419900
void RFManager::NotifyWindowLostFocus(RFWnd* wnd)
{
	if (wnd == _focusWnd)
		_focusWnd = 0;
}

// 00419920
void RFManager::NotifyWindowActivated(RFWnd* wnd)
{
	if (wnd != _activeWnd)
	{
		if (_activeWnd)
			_activeWnd->Deactivate();
		_activeWnd = wnd;
	}
}

// 00419950
void RFManager::NotifyWindowDeactivated(RFWnd* wnd)
{
	if (wnd == _activeWnd)
		_activeWnd = 0;
}

// 00419970: esquece a janela; o foco vai para a de cima que puder (ou para a área de trabalho)
void RFManager::NotifyWindowDestroyed(RFWnd* wnd)
{
	if (wnd == _activeWnd)
		_activeWnd = 0;
	if (wnd == _focusWnd)
	{
		if (wnd->GetOverlapped()->_hitTestVisible)
		{
			RFWnd* next = FindActiveFocusableWnd(0);
			FocusWindow(next ? next : _desktop);
		}
		else
			FocusWindow(_desktop);
	}
	if (wnd == _capturedWnd)
		_capturedWnd = 0;
	if (wnd == _mouseOverWnd)
		_mouseOverWnd = 0;
	if (wnd == _desktop)
		_desktop = 0;
}

// 00419c30
bool RFManager::IsUsingCustomInputMethod()
{
	return _usingCustomInputMethod;
}

// ---------------------------------------------------------------------------------------------------------------------------------
// 2º lote (B155): criação pelo XML, eventos de link/IME, desenhista por dll, os Inject* restantes e o nome do controle por tipo.

// 00417a20: a área de trabalho fica atrás das outras (prioridade 1); só uma
RFWnd* RFManager::CreateDesktop(const std::wstring& name, RFWnd* wnd)
{
	if (_desktop)
		return 0;
	wnd = CreateWnd(name, wnd, 0);
	if (!wnd)
		return 0;
	_desktop = wnd;
	g_pRFDesktop983 = wnd;
	wnd->_priority = 1;
	if (wnd->_super)
		wnd->_super->_priorityInvalid = true;
	_root->RegisterChild(wnd);
	return wnd;
}

// B268: cópias fora de linha / auxiliares do RFWnd.cpp (00402e30-004033f0), conferidas no asm
// 00402e30 / 00402ea0: corpo idêntico ao CreateDesktop 00417a20
RFWnd* RFManager::CreateDesktopA983(const std::wstring& name, RFWnd* wnd)
{
	return CreateDesktop(name, wnd);
}

RFWnd* RFManager::CreateDesktopW983(const std::wstring& name, RFWnd* wnd)
{
	return CreateDesktop(name, wnd);
}

// 00402f40
void RFManager::SetTopMost983(RFWnd* wnd)
{
	wnd->_priority = -1;
	if (wnd->_super)
		wnd->_super->_priorityInvalid = true;
}

// 00402ff0
RFSize& RFManager::GetResolution983(RFSize& out)
{
	out = _root->GetSize();
	return out;
}

// 00403010
void RFManager::SetResolution983(const RFSize& size)
{
	RFWnd* root = _root;
	if (root->_localSize.width != size.width || root->_localSize.height != size.height)
	{
		root->_localSize = size;
		root->OnLayoutChanged(2);
	}
}

// 00403060
void RFManager::UpdateLayout983()
{
	_root->OnLayoutChanged(0x10000000);
	for (std::vector<RFWnd*>::iterator it = _root->_children.begin(); it != _root->_children.end(); ++it)
		(*it)->UpdateLayout();
}

// 004033f0
RFWnd* RFManager::FindWnd983(float x, float y)
{
	return HitTest2(RFVector2(x, y), true, true);
}

// 004030b0 (o 983 copia o mapa de eventos +4, os campos +0x34..+0x89 e a lista +0x8c; não passa pelo construtor do RFSingleton)
RFManager::RFManager(const RFManager& o)
	: RFEventHandler(o), RFManagerBase983(), RFSingleton<RFManager>(o), _rendererModule983(o._rendererModule983),
	  _renderer(o._renderer), _interface(o._interface), _inputMethod(o._inputMethod), _factoryManager(o._factoryManager),
	  _inputsFrozen(o._inputsFrozen), _root(o._root), _desktop(o._desktop), _activeWnd(o._activeWnd), _focusWnd(o._focusWnd),
	  _capturedWnd(o._capturedWnd), _mouseOverWnd(o._mouseOverWnd), _visualEffect(o._visualEffect), _68(o._68), _6c(o._6c),
	  _cursorPos(o._cursorPos), _globalPadding(o._globalPadding), _globalPaddingEnabled(o._globalPaddingEnabled),
	  _usingCustomInputMethod(o._usingCustomInputMethod), _list8c(o._list8c)
{
}

// 00403230
RFManager& RFManager::operator=(const RFManager& o)
{
	if (this != &o)
	{
		RFEventHandler::operator=(o);
		_rendererModule983 = o._rendererModule983;
		_renderer = o._renderer;
		_interface = o._interface;
		_inputMethod = o._inputMethod;
		_factoryManager = o._factoryManager;
		_inputsFrozen = o._inputsFrozen;
		_root = o._root;
		_desktop = o._desktop;
		_activeWnd = o._activeWnd;
		_focusWnd = o._focusWnd;
		_capturedWnd = o._capturedWnd;
		_mouseOverWnd = o._mouseOverWnd;
		_visualEffect = o._visualEffect;
		_68 = o._68;
		_6c = o._6c;
		_cursorPos = o._cursorPos;
		_globalPadding = o._globalPadding;
		_globalPaddingEnabled = o._globalPaddingEnabled;
		_usingCustomInputMethod = o._usingCustomInputMethod;
		_list8c = o._list8c;
	}
	return *this;
}

// 00417a90: lê o XML (<caminho base> + name), cria a janela (wnd já alocada pelo jogo ou pela fábrica do tipo da raiz), os filhos,
// o efeito visual e a animação; a janela nasce escondida. Arquivo que não abre devolve NULL; outro erro sobe.
// O 983 pega a fábrica pelo RFSingleton<RFWindowFactoryManager> (00f1de4c, o mesmo objeto do _factoryManager).
RFWnd* RFManager::CreateWnd(const std::wstring& name, RFWnd* wnd, RFWnd* parent)
{
	RFXmlLoader loader(_interface);
	std::wstring animation;
	try
	{
		loader.Load(_interface->GetBasePath(RFBASEPATH_DEFAULT983) + name);
		animation = loader.GetAnimationPath();
		WndBaseStruct* base = loader.GetRoot();
		if (!wnd)
		{
			RFWindowFactory* factory = _factoryManager->GetFactory(GetCompatibleControlName((WndType)base->GetType()));
			if (factory)
			{
				wnd = factory->Create();
				wnd->Create(base, 0);
			}
		}
		else
			wnd->Create(base, 0);
		(parent ? parent : _root)->RegisterChild(wnd);
		CreateChild(loader, wnd, base);
		wnd->Initialize();
		wnd->SetVisualEffect(_visualEffect);
		wnd->LoadUIAnimation(animation);
		if (wnd->_visible)
		{
			wnd->_visible = false;
			wnd->_visualState = VisualStateTypes::Hidden983;
			wnd->OnShow(false);
		}
		(parent ? parent : g_pRFRoot983)->ApplyAnchor();
		loader.Clear();
		return wnd;
	}
	catch (...)
	{
		if (!g_pRFInterface983->FOpen(std::wstring(name.c_str())))
			return 0;
		throw;
	}
}

// 00417e90: os filhos de cada estrutura, pela fábrica do tipo (tipo sem fábrica fica de fora)
void RFManager::CreateChild(RFXmlLoader& loader, RFWnd* parent, WndBaseStruct* base)
{
	int count = loader.GetChildCount(base);
	for (int i = 0; i < count; i++)
	{
		WndBaseStruct* child = loader.GetChild(base, i);
		RFWindowFactory* factory = _factoryManager->GetFactory(GetCompatibleControlName((WndType)child->GetType()));
		if (factory)
		{
			RFWnd* wnd = factory->Create();
			wnd->Create(child, parent);
			CreateChild(loader, wnd, child);
			wnd->Initialize();
		}
	}
}

// 00417fd0
void RFManager::PostHyperLinkEvent(RFWnd* wnd, const std::wstring& link, const std::wstring& param)
{
	HyperLinkEventArgs e(wnd, link, param);
	EventMap::iterator itor = _events.find(EventHyperLink);
	if (itor != _events.end())
		itor->second(e);
}

// 004180e0
void RFManager::PostGlobalHyperLinkEnterEvent(RFWnd* wnd, const std::wstring& link, const std::wstring& param)
{
	HyperLinkEventArgs e(wnd, link, param);
	EventMap::iterator itor = _events.find(EventGlobalHyperLinkEnter);
	if (itor != _events.end())
		itor->second(e);
}

// 004181f0
void RFManager::PostGlobalHyperLinkLeaveEvent(RFWnd* wnd)
{
	WindowEventArgs_<RFWnd> e(wnd);
	EventMap::iterator itor = _events.find(EventGlobalHyperLinkLeave);
	if (itor != _events.end())
		itor->second(e);
}

// 00418270
void RFManager::PostIMStatusEvent(const LocaleIndicatorTypes::Type& locale, const unsigned int& status)
{
	IMStatusEventArgs e(locale, status);
	EventMap::iterator itor = _events.find(EventInputMethodStatusChanged);
	if (itor != _events.end())
		itor->second(e);
}

// 00418500: desenhista de uma dll ("createRenderer"(param, interface)). Dll que não carrega: o 983 apaga o bloco e deixa o ponteiro
// solto no _rendererModule983 (o destrutor o usaria); aqui zera.
void RFManager::SetRenderer(const std::wstring& module, void* param)
{
	if (_rendererModule983)
		delete (RFRendererModule983*)_rendererModule983;
	RFRendererModule983* renderer = new RFRendererModule983(module);
	_rendererModule983 = renderer;
	if (!renderer->module)
	{
		delete renderer;
		_rendererModule983 = 0;
		return;
	}
	typedef RFRenderer* (__fastcall *CreateRenderer)(void* param, RFInterface* iface);
	CreateRenderer createRenderer = (CreateRenderer)GetProcAddress(renderer->module, std::string("createRenderer").c_str());
	g_pRFRenderer983 = createRenderer(param, _interface);
	_renderer = g_pRFRenderer983;
}

// 004187a0
void RFManager::RestoreInputMethod()
{
	if (_inputMethod && _inputMethod->_ime)
		_inputMethod->_ime->Restore983();
}

// 00418be0: sem ativar/focar
bool RFManager::InjectLButtonUp(float x, float y)
{
	if (IsInputsFroze())
		return false;
	RFWnd* wnd = _capturedWnd;
	if (!wnd)
	{
		wnd = HitTest2(RFVector2(x, y), true, false);
		if (!wnd)
			return false;
	}
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	MouseEventArgs e(wnd, RFVector2(x, y), 0, 0.0f);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnLButtonUp(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00418ce0
bool RFManager::InjectLButtonDblClick(float x, float y)
{
	if (IsInputsFroze())
		return false;
	RFWnd* wnd = _capturedWnd;
	if (!wnd)
	{
		wnd = HitTest2(RFVector2(x, y), true, false);
		if (!wnd)
			return false;
	}
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	ActivateWindow(overlapped);
	FocusWindow(wnd);
	MouseEventArgs e(wnd, RFVector2(x, y), 0, 0.0f);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnLButtonDblClick(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00418df0
bool RFManager::InjectRButtonDown(float x, float y)
{
	if (IsInputsFroze())
		return false;
	RFWnd* wnd = _capturedWnd;
	if (!wnd)
	{
		wnd = HitTest2(RFVector2(x, y), true, false);
		if (!wnd)
			return false;
	}
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	ActivateWindow(overlapped);
	FocusWindow(wnd);
	MouseEventArgs e(wnd, RFVector2(x, y), 1, 0.0f);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnRButtonDown(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00418f00: também ativa/foca (como o RButtonDown)
bool RFManager::InjectRButtonUp(float x, float y)
{
	if (IsInputsFroze())
		return false;
	RFWnd* wnd = _capturedWnd;
	if (!wnd)
	{
		wnd = HitTest2(RFVector2(x, y), true, false);
		if (!wnd)
			return false;
	}
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	ActivateWindow(overlapped);
	FocusWindow(wnd);
	MouseEventArgs e(wnd, RFVector2(x, y), 1, 0.0f);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnRButtonUp(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00419180
bool RFManager::InjectMouseWheel(int delta, float x, float y)
{
	if (IsInputsFroze())
		return false;
	RFWnd* wnd = _capturedWnd;
	if (!wnd)
	{
		wnd = HitTest2(RFVector2(x, y), true, false);
		if (!wnd)
			return false;
	}
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	MouseEventArgs e(wnd, RFVector2(x, y), 3, (float)delta);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnMouseWheel(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00419280: para a janela com o foco e os pais
bool RFManager::InjectKeyDown(unsigned int key, unsigned int flags)
{
	if (IsInputsFroze() || !_focusWnd)
		return false;
	RFWnd* wnd = _focusWnd;
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	KeyEventArgs e(wnd, key, flags);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnKeyDown(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00419340
bool RFManager::InjectKeyUp(unsigned int key, unsigned int flags)
{
	if (IsInputsFroze() || !_focusWnd)
		return false;
	RFWnd* wnd = _focusWnd;
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	KeyEventArgs e(wnd, key, flags);
	for (RFWnd* target = wnd; target; target = target->_super)
	{
		target->OnKeyUp(e);
		if (e.handled)
			break;
	}
	return true;
}

// 00419400: tira um de cada caractere de controle (0x00–0x1f e 0x7f) e entrega o texto só à janela com o foco (sem subir)
bool RFManager::InjectChar(wchar_t* text, bool composing, const RFPoint& pos)
{
	if (IsInputsFroze())
		return false;
	std::wstring filtered(text);
	for (wchar_t c = 0; c < 0x20; c++)
	{
		std::wstring::size_type at = filtered.find(c);
		if (at != std::wstring::npos)
			filtered.erase(at, 1);
	}
	std::wstring::size_type at = filtered.find(L'\x7f');
	if (at != std::wstring::npos)
		filtered.erase(at, 1);

	RFWnd* wnd = _focusWnd;
	if (!wnd)
		return false;
	RFWnd* overlapped = wnd->GetOverlapped();
	if (overlapped && (overlapped->_visualState == VisualStateTypes::Hiding983 || overlapped->_inputLock == true))
		return false;
	CharEventArgs e(wnd, filtered.c_str(), composing, pos);
	wnd->OnChar(e);
	return true;
}

// 004196d0: contexto com o recorte da raiz na tela
bool RFManager::InjectPreDraw()
{
	RFRenderContext rc;
	rc._clipRect = _root->ClientToScreen();
	_root->PreDraw(rc);
	return true;
}

// 00419780
bool RFManager::InjectDraw()
{
	RFRenderContext rc;
	rc._clipRect = _root->ClientToScreen();
	_root->Draw(rc);
	return true;
}

// 00419830: foco numa janela que sumiu volta para a área de trabalho; depois o OnUpdate da árvore
bool RFManager::InjectUpdate(float delta)
{
	if (_focusWnd)
	{
		RFWnd* focus = _focusWnd;
		if (focus->_hiding == true || !focus->_visible || (focus->_super && !focus->_super->IsWindowVisible(false)))
			FocusWindow(_desktop);
	}
	UpdateEventArgs e;
	e.delta = delta;
	_root->OnUpdate(e);
	return true;
}

// 004199f0
const std::wstring& RFManager::GetCompatibleControlName(WndType type)
{
	switch (type)
	{
	case 0: case 1: return RFWndEx::TypeName;
	case 2: return RFLabel::TypeName;
	case 3: return RFButton::TypeName;
	case 4: return RFLabelButton::TypeName;
	case 5: return RFAnimButton::TypeName;
	case 6: return RFCheck::TypeName;
	case 7: return RFPicture::TypeName;
	case 8: return RFProgress::TypeName;
	case 9: return RFScrollBar::TypeName;
	case 10: return RFSlider::TypeName;
	case 11: return RFEdit::TypeName;
	case 12: case 13: return RFListSC::TypeName;
	case 14: return RFStackPanel::TypeName;
	case 15: case 16: return RFTextAreaSC::TypeName;
	case 17: return RFComboBox::TypeName;
	}
#line 1127 "src\\RFManager.cpp"
	assert(!"Unknown window type.");
	static const std::wstring sUnknownName(L"<unknown>");
	return sUnknownName;
}

// 00418670: o IME da janela do jogo (custom: o jogo desenha a composição; RFCustomIme), desligado e em modo alfanumérico
void RFManager::InitializeInputMethod(HWND hWnd, bool custom)
{
	_usingCustomInputMethod = custom;
	_inputMethod = new RFInputMethodProvider();
	if (custom)
		_inputMethod->_ime = new RFCustomIme(hWnd);
	else
		_inputMethod->_ime = new RFIme(hWnd);
	if (_inputMethod->_ime)
		_inputMethod->_ime->SetNativeMode983(false);
	if (_inputMethod->_ime)
		_inputMethod->_ime->SetEnable983(false);
}

// 00419c40: o texto que o IME terminou, um resultado por vez
void RFManager::UpdateInputMethod()
{
	if (!_inputMethod || !_inputMethod->_ime)
		return;
	int count = _inputMethod->_ime->GetResultCount983();
	if (!count)
		return;
	wchar_t buffer[0x100] = { 0 };
	bool composing = false;
	RFPoint pos(0, 0);
	for (int i = count; i > 0; i--)
	{
		int index = _inputMethod->_ime ? _inputMethod->_ime->GetResultString983(0, 0, &composing) : -1;
		if (_inputMethod->_ime)
			_inputMethod->_ime->GetResultString983(buffer, index, &composing);
		InjectChar(buffer, composing, pos);
	}
}

C_ASSERT_VC10(offsetof(RFManager, _rendererModule983) == 0x34);
C_ASSERT_VC10(offsetof(RFManager, _cursorPos) == 0x70);
C_ASSERT_VC10(offsetof(RFManager, _usingCustomInputMethod) == 0x89);
C_ASSERT_VC10(offsetof(RFManager, _list8c) == 0x8c);
