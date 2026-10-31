// JP 983: Refresh, src\RFWnd.cpp (F3.7.13 item 3; LTCG — prova pela dll de prova, ver out/F3.7/s149/RELATORIO.md). A unidade
// emite também as cópias fora de linha dos cabeçalhos que inclui (classes exportadas: 00401000–00404830 e 00408470–00417140).
// 1º lote (B149): construtor/destrutor, Initialize/Create/Destroy, filhos, prioridade e UpdateLayout (00404830–004051f0).
// 2º lote (B150): o resto do RFWnd.cpp — centrar, caminho, animação, desenho, mostrar, teste de ponto, alfa, tratadores On*, Close e
// textos do usuário (004059c0–00408470).
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include "vc10_layout.h"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/rffactorymanager.h"
#include "refresh/rfwndstruct.h"

C_ASSERT_VC10(sizeof(RFEventHandler) == 0x30);
C_ASSERT_VC10(sizeof(RFEvent) == 0x2c);
C_ASSERT_VC10(sizeof(RFWindowModifier) == 0x3c);
C_ASSERT_VC10(offsetof(RFWnd, _modifiers) == 0x50);
C_ASSERT_VC10(offsetof(RFWnd, _children) == 0x8c);
C_ASSERT_VC10(offsetof(RFWnd, _userStrings) == 0xb0);
C_ASSERT_VC10(offsetof(RFWnd, _name) == 0xd8);
C_ASSERT_VC10(offsetof(RFWnd, _dragable) == 0x13c);
C_ASSERT_VC10(offsetof(RFWnd, _rc) == 0x158);
C_ASSERT_VC10(sizeof(RFWnd) == 0x178);
C_ASSERT_VC10(offsetof(RFWnd, _visible) == 0x61);
C_ASSERT_VC10(offsetof(RFWnd, _hitTestVisible) == 0x65);
C_ASSERT_VC10(offsetof(RFWnd, _visualState) == 0x78);
C_ASSERT_VC10(offsetof(RFWnd, _type) == 0xd4);

// D857: acessos da visão do jogo (rfscenemanager.h) com o layout real da lib
int RFWnd::GetType983()
{
	return _type;
}

bool RFWnd::IsDragable983()
{
	return _dragable;
}

const std::wstring& RFWnd::GetName983()
{
	return _name;
}

bool RFWnd::IsVisible983()
{
	return _visible;
}

void RFWnd::SetVisibleState983(bool show)
{
	_visible = show;
	_visualState = show ? VisualStateTypes::Visible983 : VisualStateTypes::Hidden983;
}

void RFWnd::SetHitTestVisible983(bool visible)
{
	_hitTestVisible = visible;
}

// desenhista e interface da RF (globais do RFManager.cpp; 00f1de20 / 00f1de14; nomes meus)
extern RFRenderer* g_pRFRenderer983;
extern RFInterface* g_pRFInterface983;
extern RFWnd* g_pRFRoot983; // 00f1de18: a janela-raiz (RFManager +0x4c; os filhos dela são as sobrepostas; UpdateAnchorValue não desconta a borda dela)
extern RFWnd* g_pRFDesktop983; // 00f1de1c: a área de trabalho (RFManager::RegisterDesktop/DeleteDesktop); o Draw/PreDraw desenha sempre
extern const std::wstring g_rfEmptyString983; // 00e46f04 (não exportado; fica entre os estáticos do RFDirector.cpp): GetUserString sem a chave

const unsigned int RFWnd::AnimMask_Visible = 1;
const std::wstring RFWnd::TypeName = L"WindowBase";
const std::wstring RFWnd::EventClose = L"Close";
const std::wstring RFWnd::EventFinalize = L"Finalize";
const std::wstring RFWnd::EventLButtonUp = L"LButtonUp";
const std::wstring RFWnd::EventLButtonDown = L"LButtonDown";
const std::wstring RFWnd::EventLButtonDblClick = L"LButtonDblClick";
const std::wstring RFWnd::EventRButtonUp = L"RButtonUp";
const std::wstring RFWnd::EventRButtonDown = L"RButtonDown";
const std::wstring RFWnd::EventMouseMove = L"MouseMove";
const std::wstring RFWnd::EventMouseWheel = L"MouseWheel";
const std::wstring RFWnd::EventMouseEnter = L"MouseEnter";
const std::wstring RFWnd::EventMouseLeave = L"MouseLeave";
const std::wstring RFWnd::EventContextMenu = L"ContextMenu";
const std::wstring RFWnd::EventKeyUp = L"KeyUp";
const std::wstring RFWnd::EventKeyDown = L"KeyDown";
const std::wstring RFWnd::EventChar = L"EventChar";
const std::wstring RFWnd::EventActivated = L"Activated";
const std::wstring RFWnd::EventDeactivated = L"Deactivated";
const std::wstring RFWnd::EventSetFocus = L"SetFocus";
const std::wstring RFWnd::EventKillFocus = L"KillFocus";
const std::wstring RFWnd::EventUpdate = L"Update";
const std::wstring RFWnd::EventDraw = L"Draw";
const std::wstring RFWnd::EventPlayUIAnimation = L"PlayUIAnimation";
const std::wstring RFWnd::EventVisibleChanged = L"VisibleChanged";
const std::wstring RFWnd::AnimOpening = L"_Opening_";
const std::wstring RFWnd::AnimClosing = L"_Closing_";

// ordem dos filhos (prioridade maior primeiro; empate: a não modal antes). Functor vazio: o 983 passa o byte dele ao _Stable_sort.
struct RFWndPriorityLess983
{
	bool operator()(RFWnd* a, RFWnd* b) const // 00402120
	{
		if (a->GetPriority() == b->GetPriority())
			return a->IsModal() < b->IsModal();
		return a->GetPriority() > b->GetPriority();
	}
};

// 00404830
const std::wstring& RFWnd::GetTypeName()
{
	return TypeName;
}

// 00404840
RFWnd::RFWnd()
	: _base(0), _super(0), _active(false), _overlapped(true), _hitTestVisible(true), _hitTestThrough(false),
	  _dragPos(0.0f, 0.0f), _modal(false), _initialPos(0.0f, 0.0f), _localPos(0.0f, 0.0f), _subControl(false),
	  _animationMask(0xffffffff)
{
	_dragable = false;
	_preDraw = false;
}

// 00404a40
RFWnd::~RFWnd()
{
}

// 00404bc0
void RFWnd::PreInitialize()
{
	_alpha = 1.0f;
	_animation = 0;
	_priorityInvalid = false;
	_dragging = false;
	_71 = false;
	_inputLock = false;
	_visible = true;
	_hiding = false;
	_enabled = true;
	_inheritsAlpha = true;
	_closeState = RFWNDCLOSESTATE_NONE983;
	_modal = false;
	_visualState = VisualStateTypes::Visible983;
	_visualEffect = 0;
	_priority = 0;
	UpdateAnchorValue();
	_clipChildren = true;
}

// 00404c20
void RFWnd::Initialize()
{
	PreInitialize();
	LoadResource();
	OnInitialUpdate();
	RegisterTo(L"", EventKeyDown, Subscriber(&RFWnd::KeyDownEventHandler, this));
}

// 00404d30
void RFWnd::Create(WndBaseStruct* base, RFWnd* parent)
{
	_type = (WndType)base->GetType();
	_name.assign(base->_name, 0, std::wstring::npos);
	// (no 983 a posição do WndBaseStruct é um RFVector2: cópia com mov)
	_localPos = *reinterpret_cast<const RFVector2*>(&base->_x);
	_initialPos = *reinterpret_cast<const RFVector2*>(&base->_x);
	_localSize = RFSize(base->_rect.Width(), base->_rect.Height());
	_initialSize = _localSize;
	_showSound.assign(base->_showSound, 0, std::wstring::npos);
	_hideSound.assign(base->_hideSound, 0, std::wstring::npos);
	_dragable = base->_dragable.Get();
	_hitTestVisible = base->_isHitTestVisible.Get();
	_hitTestThrough = base->_isHitTestThrough.Get();
	_startPosition = (StartPositionTypes::Type)base->_startPosition.Get();
	_escKeyAction = EscKeyActionTypes::None;
	_anchor = base->_anchor.Get();
	_clipChildren = base->_clipChildren.Get();
	_rc._clip = base->_clipChildren.Get();
	_base = base;
	if (parent)
		parent->RegisterChild(this);
}

// 00404e50
void RFWnd::Destroy()
{
	RFManager::GetInstance().NotifyWindowDestroyed(this);
	WindowEventArgs_<RFWnd> e(this);
	SendEvent(EventFinalize, e);
	OnDestroy();
	DeleteAllChildren();
	if (_animation)
	{
		delete _animation;
		_animation = 0;
	}
	if (_base)
	{
		delete _base;
		_base = 0;
	}
	if (_visualEffect)
	{
		delete _visualEffect;
		_visualEffect = 0;
	}
}

// 00404f40
void RFWnd::DeleteChild(RFWnd* child)
{
	std::vector<RFWnd*>::iterator itor = std::find(_children.begin(), _children.end(), child);
	if (itor != _children.end())
	{
		RFWnd* wnd = *itor;
		_children.erase(itor);
		wnd->Destroy();
		delete wnd;
	}
}

// 00404fc0
void RFWnd::DeleteAllChildren()
{
	while (!_children.empty())
	{
		RFWnd* wnd = _children.back();
		_children.pop_back();
		wnd->Destroy();
		delete wnd;
	}
}

// 00405020
void RFWnd::EjectChild_(RFWnd* child)
{
	std::vector<RFWnd*>::iterator itor = std::find(_children.begin(), _children.end(), child);
	if (itor != _children.end())
		_children.erase(itor);
}

// 00405080
void RFWnd::ValidatePriority()
{
	if (_priorityInvalid)
	{
		std::stable_sort(_children.begin(), _children.end(), RFWndPriorityLess983());
		_priorityInvalid = false;
	}
}

// 00405130
void RFWnd::InvalidatePriority()
{
	_priorityInvalid = true;
}

// 00405140
void RFWnd::UpdateLayout()
{
	OnLayoutChanged(0x10000000);
	std::for_each(_children.begin(), _children.end(), std::mem_fun(&RFWnd::UpdateLayout));
}

// 004051d0
RFRenderer* RFWnd::GetRenderer()
{
	return g_pRFRenderer983;
}

// 004051e0
RFInterface* RFWnd::GetInterface()
{
	return g_pRFInterface983;
}

// 004051f0: reposiciona/redimensiona os filhos ancorados (Left 1, Top 2, Right 4, Bottom 8)
void RFWnd::ApplyAnchor()
{
	for (unsigned int i = 0; i < _children.size(); i++)
	{
		RFWnd* child = _children[i];
		unsigned int anchor = child->_anchor;
		if (anchor)
		{
			RFVector2 pos = child->_localPos;
			float width = child->_localSize.width;
			float height = child->_localSize.height;
			if (anchor & 1)
				pos.x = child->_anchorValue.left;
			if (anchor & 4)
			{
				if (anchor & 1)
				{
					RFSize parent(GetSize().width, GetSize().height);
					width = parent.width - child->_anchorValue.right - pos.x;
				}
				else
				{
					RFSize parent(GetSize().width, GetSize().height);
					pos.x = parent.width - child->_anchorValue.right - child->_localSize.width;
				}
			}
			if (anchor & 2)
				pos.y = child->_anchorValue.top;
			if (anchor & 8)
			{
				if (anchor & 2)
				{
					RFSize parent(GetSize().width, GetSize().height);
					height = parent.height - child->_anchorValue.bottom - pos.y;
				}
				else
				{
					RFSize parent(GetSize().width, GetSize().height);
					pos.y = parent.height - child->_anchorValue.bottom - child->_localSize.height;
				}
			}
			child->SetPos(pos.x, pos.y);
			child->SetSize(RFSize(width, height) - RFSize());
		}
	}
}

// 004053f0: distâncias da janela às bordas do pai (a âncora)
void RFWnd::UpdateAnchorValue()
{
	RFWnd* super = _super;
	if (super)
	{
		RFRect parentRect = super->GetScreenRect();
		RFRect rect = GetScreenRect();
		if (super == g_pRFRoot983)
		{
			parentRect.right = rect.right;
			parentRect.bottom = rect.bottom;
		}
		_anchorValue.left = rect.left - parentRect.left;
		_anchorValue.right = parentRect.right - rect.right;
		_anchorValue.top = rect.top - parentRect.top;
		_anchorValue.bottom = parentRect.bottom - rect.bottom;
	}
}

// 004059c0: centraliza no pai (sem pai: na janela-raiz do RFManager)
void RFWnd::CenterWindow(RFWnd* parent)
{
	RFSize size;
	if (!parent)
		parent = _super;
	if (!parent)
		size = RFManager::GetInstance()._root->GetSize();
	else
		size = RFSize(parent->GetRect().Width(), parent->GetRect().Height());
	SetPos((size.width - GetRect().Width()) * 0.5, (size.height - GetRect().Height()) * 0.5);
}

// 00405b20
void RFWnd::CenterWindow_Horizontal(RFWnd* parent)
{
	float width;
	if (!parent)
		parent = _super;
	if (!parent)
		width = RFManager::GetInstance()._root->GetSize().width;
	else
		width = parent->GetRect().Width();
	float x = (width - GetRect().Width()) * 0.5;
	SetPos(x, GetPos().y);
}

// 00405c20
void RFWnd::CenterWindow_Vertical(RFWnd* parent)
{
	float height;
	if (!parent)
		parent = _super;
	if (!parent)
		height = RFManager::GetInstance()._root->GetSize().height;
	else
		height = parent->GetRect().Height();
	float y = (height - GetRect().Height()) * 0.5;
	SetPos(GetPos().x, y);
}

// 00405d20: janela pelo caminho ("a\b\c"; "\" no começo = a partir da área de trabalho; "." fica, ".." sobe)
RFWnd* RFWnd::GetWindow_(const std::wstring& path)
{
	RFWnd* wnd = this;
	size_t start = 0;
	std::wstring token;
	if (path.empty())
		return 0;
	if (path.at(0) == L'\\')
	{
		start = 1;
		wnd = RFManager::GetInstance().GetDesktop();
		if (!wnd)
			return 0;
	}
	for (;;)
	{
		size_t pos = path.find(L'\\', start);
		if (pos != std::wstring::npos)
		{
			token = path.substr(start, pos - start);
			start = pos + 1;
			if (token == L".")
				continue;
			if (token == L"..")
				wnd = wnd->_super;
			else
				wnd = wnd->GetChild(token);
			if (!wnd)
				return 0;
		}
		if (start == path.size())
			break;
		if (pos == std::wstring::npos)
		{
			token = path.substr(start, path.size() - start);
			wnd = wnd->GetChild(token);
			break;
		}
	}
	return wnd;
}

// 004062b0: diretor de animação da janela; com as cenas de abrir e fechar, o efeito visual pronto 2
void RFWnd::LoadUIAnimation(const std::wstring& name)
{
	if (!name.empty())
	{
		_animation = new RFDirector();
		_animation->LoadAnimation(name);
		_animation->SubscribeEvent(RFDirector::EventAnimFinished, Subscriber(&RFWnd::OnAnimFinished, this));
		if (_animation->GetAnimation(AnimOpening) && _animation->GetAnimation(AnimClosing))
		{
			if (_visualEffect)
			{
				delete _visualEffect;
				_visualEffect = 0;
			}
			_visualEffect = CreateVisualEffect983(2);
		}
	}
}

// 004064c0: posição local em pixels inteiros
void RFWnd::SetPos(float x, float y)
{
	x = (float)(int)x;
	y = (float)(int)y;
	RFVector2 pos(x, y);
	if (_localPos != pos)
	{
		_anchorValue.left = x;
		_localPos = pos;
		_anchorValue.top = y;
		OnLayoutChanged(1);
	}
}

// 00406550: posição local mais o deslocamento dos modificadores
RFVector2 RFWnd::GetPos() const
{
	RFVector2 pos = _localPos;
	for (std::vector<RFWindowModifier*>::const_iterator itor = _modifiers.begin(); itor != _modifiers.end(); ++itor)
	{
		RFVector2 offset = (*itor)->GetPos();
		pos.x += offset.x;
		pos.y += offset.y;
	}
	return pos;
}

// 004065c0
const RFVector2& RFWnd::GetLocalPos() const
{
	return _localPos;
}

// 004065d0
void RFWnd::SetScreenPos(float x, float y)
{
	if (_super)
	{
		RFVector2 offset(0.0f, 0.0f);
		for (RFWnd* wnd = _super; wnd; wnd = wnd->_super)
			offset = wnd->GetPos() + offset;
		RFVector2 pos = RFVector2(x, y) - offset;
		SetPos(pos.x, pos.y);
	}
	else
		SetPos(x, y);
}

// 00406670
RFVector2 RFWnd::GetScreenPos() const
{
	RFVector2 pos(0.0f, 0.0f);
	for (const RFWnd* wnd = this; wnd; wnd = wnd->_super)
		pos = wnd->GetPos() + pos;
	return pos;
}

// 004066c0: como o GetScreenPos, sem os modificadores
RFVector2 RFWnd::GetScreenPos_Impl() const
{
	RFVector2 pos(0.0f, 0.0f);
	for (const RFWnd* wnd = this; wnd; wnd = wnd->_super)
		pos = wnd->GetLocalPos() + pos;
	return pos;
}

// 00406700
void RFWnd::SetSize(const RFSize& size)
{
	if (_localSize != size)
	{
		_localSize = size;
		OnLayoutChanged(2);
	}
}

// 00406750: tamanho local mais o dos modificadores
RFSize RFWnd::GetSize() const
{
	RFSize size = _localSize;
	for (std::vector<RFWindowModifier*>::const_iterator itor = _modifiers.begin(); itor != _modifiers.end(); ++itor)
	{
		RFSize offset = (*itor)->GetSize();
		size.width += offset.width;
		size.height += offset.height;
	}
	return size;
}

// 004067b0: só o tamanho do retângulo
void RFWnd::SetRect(const RFRect& rect)
{
	SetSize(RFSize(rect.Width(), rect.Height()));
}

// 00406810
RFRect RFWnd::GetRect() const
{
	float width = GetSize().width;
	float height = GetSize().height;
	return RFRect(0.0f, 0.0f, width, height);
}

// 00406860
void RFWnd::SetScreenRect(const RFRect& rect)
{
	_localSize = RFSize(rect.Width(), rect.Height());
	SetScreenPos(rect.left, rect.top);
	OnLayoutChanged(3);
}

// 004068c0
RFRect RFWnd::GetScreenRect() const
{
	RFVector2 pos = GetScreenPos();
	float width = GetSize().width;
	float height = GetSize().height;
	return RFRect(pos.x, pos.y, pos.x + width, pos.y + height);
}

// 00405470: retângulo cliente na tela
RFRect RFWnd::ClientToScreen() const
{
	return GetRect().MoveRect(GetScreenPos().x, GetScreenPos().y);
}

// 00405540 (A818: y - y da janela; o 2º fsub [esp+8] vem depois do pop esi = y local, não o x)
RFVector2 RFWnd::ScreenToClient(const RFVector2& pt) const
{
	RFVector2 pos(0.0f, 0.0f);
	for (const RFWnd* wnd = this; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	return RFVector2(pt.x - pos.x, pt.y - pos.y);
}

// 004055b0 (A818: idem, y + y)
RFVector2 RFWnd::ClientToScreen(const RFVector2& pt) const
{
	RFVector2 pos(0.0f, 0.0f);
	for (const RFWnd* wnd = this; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	return RFVector2(pt.x + pos.x, pt.y + pos.y);
}

// 00405620
void RFWnd::PlaySfx(const std::wstring& name)
{
	if (!name.empty())
	{
		std::wstring sfx(name.c_str());
		RFManager::GetInstance()._interface->PlaySound983(sfx);
	}
}

// 00405700
void RFWnd::PrintText(const FontInfo& font, int x, int y, const std::wstring& text)
{
	float depth = _rc._depth;
	RFManager::GetInstance()._interface->PrintText983(font, x, y, text, depth);
}

// 00405760: Esc fecha a janela quando a ação do Esc é Close
void RFWnd::KeyDownEventHandler(EventArgs& e)
{
	if (static_cast<KeyEventArgs&>(e).key == VK_ESCAPE && _escKeyAction == EscKeyActionTypes::Close)
	{
		Close((RFWNDCLOSESTATE)2);
		e.handled = true;
	}
}

// 00405790 (top: o laço do 983 devolve sempre NULL)
const RFWnd* RFWnd::GetSuper(bool top) const
{
	if (top)
	{
		const RFWnd* wnd = this;
		while (wnd)
			wnd = wnd->GetSuper(false);
		return wnd;
	}
	return _super;
}

// 004057c0: o primeiro pai que não é subcontrole
RFWnd* RFWnd::GetControlSuper()
{
	RFWnd* wnd = this;
	while (wnd->_subControl == true)
	{
		RFWnd* super = wnd->_super;
		if (!super)
			return wnd;
		wnd = super;
		if (!wnd->_subControl)
			return wnd;
	}
	return 0;
}

// 004057f0
RFWnd* RFWnd::GetOverlapped()
{
	for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
		if (wnd->_overlapped)
			return wnd;
	return 0;
}

// 00405810: desce pelos filhos ativos
RFWnd* RFWnd::GetActiveChild()
{
	RFWnd* wnd = this;
	while (wnd->IsActive())
	{
		std::vector<RFWnd*>::iterator itor;
		for (itor = wnd->_children.begin(); itor != wnd->_children.end(); ++itor)
			if ((*itor)->IsActive())
				break;
		if (itor == wnd->_children.end())
			return wnd;
		wnd = *itor;
	}
	return 0;
}

// 00405880
bool RFWnd::IsActive() const
{
	if (!_active)
		return false;
	if (_super && !_super->IsActive())
		return false;
	return true;
}

// 004058a0: filho pelo nome (primeiro os diretos, depois em profundidade)
RFWnd* RFWnd::GetChild(const std::wstring& name)
{
	for (unsigned int i = 0; i < _children.size(); i++)
	{
		RFWnd* child = _children[i];
		if (child->_name == name)
			return child;
	}
	for (unsigned int i = 0; i < _children.size(); i++)
	{
		RFWnd* found = _children[i]->GetChild(name);
		if (found)
			return found;
	}
	return 0;
}

// 00405960
void RFWnd::RegisterChild(RFWnd* child)
{
	if (child)
	{
		if (std::find(_children.begin(), _children.end(), child) == _children.end())
		{
			if (g_pRFRoot983 != this)
				child->_overlapped = false;
			child->_super = this;
			_children.push_back(child);
			InvalidatePriority();
		}
	}
}

// 00405fc0
void RFWnd::EnableWindow(bool enable)
{
	if (_enabled != enable)
	{
		_enabled = enable;
		OnEnabled(enable);
		if (!enable && _active == true)
			_active = enable;
	}
}

// 00405ff0 (self = só esta janela)
bool RFWnd::IsWindowEnabled(bool self) const
{
	if (self == true)
		return _enabled;
	if (!_enabled)
		return false;
	if (_super && !_super->IsWindowEnabled(false))
		return false;
	return true;
}

// 00406030
void RFWnd::Activate()
{
	if (_overlapped && !_active)
	{
		_active = true;
		RFManager& mgr = RFManager::GetInstance();
		RFWnd* prev = mgr._activeWnd;
		if (this != prev)
		{
			if (prev)
				prev->Deactivate();
			mgr._activeWnd = this;
		}
		OnActivated();
	}
}

// 00406090
void RFWnd::Deactivate()
{
	if (_overlapped && _active)
	{
		_active = false;
		RFManager& mgr = RFManager::GetInstance();
		if (this == mgr._activeWnd)
			mgr._activeWnd = 0;
		OnDeactivated();
	}
}

// 004060e0
void RFWnd::SetFocus()
{
	OnSetFocus();
}

// 004060f0
void RFWnd::KillFocus()
{
	OnKillFocus();
}

// 00406100
void RFWnd::SetCapture()
{
	RFManager::GetInstance().SetCapturedWindow(this);
}

// 00406130
void RFWnd::ReleaseCapture()
{
	RFManager::GetInstance().SetCapturedWindow(0);
}

// 00406160
void RFWnd::AddModifier(RFWindowModifier* modifier)
{
	if (std::find(_modifiers.begin(), _modifiers.end(), modifier) == _modifiers.end())
		_modifiers.push_back(modifier);
}

// 004061f0
void RFWnd::RemoveModifier(RFWindowModifier* modifier)
{
	std::vector<RFWindowModifier*>::iterator itor = std::find(_modifiers.begin(), _modifiers.end(), modifier);
	if (itor != _modifiers.end())
		_modifiers.erase(itor);
}

// 00406240
RFWindowModifier* RFWnd::GetModifier(const std::wstring& name) const
{
	for (std::vector<RFWindowModifier*>::const_iterator itor = _modifiers.begin(); itor != _modifiers.end(); ++itor)
	{
		RFWindowModifier* modifier = *itor;
		if (modifier->GetName() == name)
			return modifier;
	}
	return 0;
}

// 00406430: assina `event` da janela `name` (vazio ou o próprio nome = esta; senão pelo caminho do GetWindow_)
void RFWnd::RegisterTo(const std::wstring& name, const std::wstring& event, const Subscriber& subscriber)
{
	RFWnd* wnd = this;
	if (!name.empty() && name != _name)
		wnd = GetWindow_(name);
	if (wnd)
		wnd->SubscribeEvent(event, subscriber);
}

// 00406960
void RFWnd::Draw(const RFRenderContext& rc)
{
	if (this != g_pRFRoot983 && this != g_pRFDesktop983 && _preDraw)
		return;
	_rc.Inherit983(rc._clip, rc._clipRect, rc._depth);
	if (!_rc._clip)
		_rc._clipRect = ClientToScreen();
	if (IsWindowVisible(false) || _visualState != VisualStateTypes::Hidden983)
	{
		WindowEventArgs_<RFWnd> e(this);
		SendEvent(EventDraw, e);
		if (!e.handled)
			OnDraw();
		ValidatePriority();
		for (unsigned int i = 0; i < _children.size(); i++)
			_children[i]->Draw(_rc);
	}
}

// 00406af0: desenho a partir desta janela (contexto novo, recortado nela)
void RFWnd::Draw2()
{
	if (IsWindowVisible(false) || _visualState != VisualStateTypes::Hidden983)
	{
		OnDraw();
		ValidatePriority();
		RFRenderContext rc;
		rc._clipRect = ClientToScreen();
		for (unsigned int i = 0; i < _children.size(); i++)
			_children[i]->Draw(rc);
	}
}

// 00406c00: como o Draw, só para as janelas marcadas com SetPreDraw
void RFWnd::PreDraw(const RFRenderContext& rc)
{
	if (this != g_pRFRoot983 && this != g_pRFDesktop983 && !_preDraw)
		return;
	_rc.Inherit983(rc._clip, rc._clipRect, rc._depth);
	if (!_rc._clip)
		_rc._clipRect = ClientToScreen();
	if (IsWindowVisible(false) || _visualState != VisualStateTypes::Hidden983)
	{
		WindowEventArgs_<RFWnd> e(this);
		SendEvent(EventDraw, e);
		if (!e.handled)
			OnDraw();
		ValidatePriority();
		for (unsigned int i = 0; i < _children.size(); i++)
			_children[i]->PreDraw(_rc);
	}
}

// 00406d90 (self = só esta janela)
bool RFWnd::IsWindowVisible(bool self) const
{
	if (self)
		return !_hiding && _visible;
	if (_hiding == true || !_visible)
		return false;
	if (_super && !_super->IsWindowVisible(false))
		return false;
	return true;
}

// 00406de0
void RFWnd::ShowWindow(bool show)
{
	if (!show && _visualState == VisualStateTypes::Hiding983)
		return;
	if (_visible == show && _visualState != VisualStateTypes::Hiding983)
		return;
	if (show)
	{
		PlaySfx(_showSound);
		if (_visualEffect)
			_visualState = VisualStateTypes::Showing983;
	}
	else
	{
		PlaySfx(_hideSound);
		if (_visualEffect)
			_visualState = VisualStateTypes::Hiding983;
	}
	if (_visualEffect)
		_visualEffect->Begin983(this);
	else if (!show)
		_visualState = VisualStateTypes::Hidden983;
	if (show)
	{
		_visible = true;
		_hiding = false;
		Activate();
	}
	else
	{
		if (_overlapped)
		{
			RFWnd* focus = RFManager::GetInstance().GetFocusWnd();
			if (focus && this == focus->GetOverlapped())
			{
				RFWnd* next = RFManager::GetInstance().FindActiveFocusableWnd(this);
				if (next)
				{
					RFManager::GetInstance().ActivateWindow(next);
					RFManager::GetInstance().FocusWindow(next);
				}
			}
		}
		if (_visualState != VisualStateTypes::Hidden983)
			_hiding = true;
		else
		{
			_visible = false;
			_hiding = false;
		}
		Deactivate();
	}
	OnShow(show);
	if (!show)
	{
		_dragging = false;
		_modal = false;
		return;
	}
	switch (_startPosition)
	{
	case StartPositionTypes::CenterParent:
		CenterWindow(0);
		break;
	case StartPositionTypes::CenterScreen:
	{
		RFSize size = RFManager::GetInstance()._root->GetSize();
		SetPos((size.width - GetRect().Width()) * 0.5, (size.height - GetRect().Height()) * 0.5);
		break;
	}
	}
	for (unsigned int i = 0; i < GetChildCount(); i++)
		if (GetChild(i)->IsFocusable())
			break;
}

// 00407070
void RFWnd::ShowModal()
{
	_modal = true;
	ShowWindow(true);
	RFManager::GetInstance().ActivateWindow(this);
	RFManager::GetInstance().FocusWindow(this);
}

// 00407110: passa o filho para o fim da lista (desenhado por último)
void RFWnd::BringChildWndToTop(RFWnd* child)
{
	std::vector<RFWnd*>::iterator itor = std::find(_children.begin(), _children.end(), child);
	if (itor != _children.end())
	{
		_children.erase(itor);
		_children.push_back(child);
		InvalidatePriority();
	}
}

// 004071f0
void RFWnd::SetPreDraw(bool preDraw)
{
	_preDraw = preDraw;
	for (unsigned int i = 0; i < _children.size(); i++)
		_children[i]->SetPreDraw(preDraw);
}

// 00407240: número < 0x10 = efeito pronto da fábrica; senão guarda uma cópia; NULL tira o efeito
void RFWnd::SetVisualEffect(RFVisualEffect* effect)
{
	if (_visualEffect)
	{
		delete _visualEffect;
		_visualEffect = 0;
	}
	if ((unsigned int)effect < 0x10)
		_visualEffect = CreateVisualEffect983((int)effect);
	else
		_visualEffect = effect->Clone983();
	if (!effect)
		_visualState = VisualStateTypes::Visible983;
}

// 00407290 (a2: aceita janela desabilitada)
bool RFWnd::HitTest(const RFVector2& pt, bool a2)
{
	if (!_visible || !_hitTestVisible)
		return false;
	if (!a2 && !_enabled)
		return false;
	RFRect rect = ClientToScreen();
	RFRect hit;
	if (_rc._clip)
		hit.IntersectRect(rect, _rc._clipRect);
	else
		hit = rect;
	return hit.PtInRect(pt) != 0;
}

// 00407340: filho sob o ponto, de cima para baixo (recursive: desce até a folha)
RFWnd* RFWnd::HitTestChild(const RFVector2& pt, bool recursive, bool a3)
{
	if (!_visible)
		return 0;
	if (_clipChildren && !HitTest(pt, a3))
		return 0;
	IteratorBase<std::vector<RFWnd*>::reverse_iterator> it = GetReverseWindowList();
	for (; it._cur != it._end; ++it._cur)
	{
		RFWnd* child = *it._cur;
		if (child->_clipChildren && !child->HitTest(pt, a3))
			continue;
		if (!recursive)
			return child;
		RFWnd* found = child->HitTestChild(pt, recursive, a3);
		if (found)
			return found;
	}
	if (!_clipChildren && !HitTest(pt, a3))
		return 0;
	return _hitTestThrough ? 0 : this;
}

// 00407420
RFWnd* RFWnd::ChildWindowFromPoint(float x, float y, RFCWPFlags flags)
{
	RFRect rect = ClientToScreen();
	if (!rect.PtInRect(x, y))
		return 0;
	if (!_visible && (flags & RFCWP_SKIPINVISIBLE983))
		return 0;
	if (!_enabled && (flags & RFCWP_SKIPDISABLED983))
		return 0;
	for (unsigned int i = 0; i < GetChildCount(); i++)
	{
		RFWnd* found = GetChild(i)->ChildWindowFromPoint(x, y, flags);
		if (found)
			return found;
	}
	return this;
}

// 00407520
bool RFWnd::IsMouseOver()
{
	return HitTest(RFManager::GetInstance().GetCursorPos(), false) != 0;
}

// 00407560
RFManager* RFWnd::GetMgr()
{
	return RFManager::GetInstancePtr();
}

// 00407570
void RFWnd::SetAlpha(float alpha)
{
	_alpha = (std::min)((std::max)(0.0f, alpha), 1.0f); // (o windows.h define min/max)
}

// 004075c0: alfa próprio vezes o dos modificadores (e o do pai, se herda)
float RFWnd::GetAlpha(bool self) const
{
	float alpha = _alpha;
	for (std::vector<RFWindowModifier*>::const_iterator itor = _modifiers.begin(); itor != _modifiers.end(); ++itor)
		alpha = (*itor)->GetAlpha() * alpha;
	if (self != true && _inheritsAlpha && _super)
		return _super->GetAlpha(false) * alpha;
	return alpha;
}

// 00407630
void RFWnd::OnLayoutChanged(unsigned int flags)
{
	if (flags & 2)
		ApplyAnchor();
}

// 00407650
void RFWnd::OnShow(bool show)
{
	VisibleChangedArgs e(this, _visible);
	SendEvent(EventVisibleChanged, e);
}

// 004076d0: arrastar começa aqui (posição do clique relativa à janela)
void RFWnd::OnLButtonDown(MouseEventArgs& e)
{
	if (_dragable)
	{
		SetCapture();
		_dragPos = ScreenToClient(e.pos);
	}
	_dragging = true;
	SendEvent(EventLButtonDown, e);
}

// 00407760 (o EventLButtonUp só sai se o botão desceu nesta janela)
void RFWnd::OnLButtonUp(MouseEventArgs& e)
{
	if (_dragable)
		ReleaseCapture();
	if (_dragging)
		SendEvent(EventLButtonUp, e);
	_dragging = false;
}

// 004077d0
void RFWnd::OnLButtonDblClick(MouseEventArgs& e)
{
	SendEvent(EventLButtonDblClick, e);
}

// 00407810
void RFWnd::OnRButtonDown(MouseEventArgs& e)
{
	SendEvent(EventRButtonDown, e);
}

// 00407850: soltar o botão direito sobre a janela = menu de contexto
void RFWnd::OnRButtonUp(MouseEventArgs& e)
{
	SendEvent(EventRButtonUp, e);
	if (HitTest(e.pos, false))
		SendEvent(EventContextMenu, e);
}

// 004078c0
void RFWnd::OnMouseMove(MouseEventArgs& e)
{
	if (_dragable && _dragging)
	{
		RFVector2 pos = e.pos - _dragPos;
		SetScreenPos(pos.x, pos.y);
	}
	SendEvent(EventMouseMove, e);
}

// 00407940
void RFWnd::OnMouseWheel(MouseEventArgs& e)
{
	SendEvent(EventMouseWheel, e);
}

// 00407980
void RFWnd::OnMouseEnter()
{
	WindowEventArgs_<RFWnd> e(this);
	SendEvent(EventMouseEnter, e);
}

// 004079f0
void RFWnd::OnMouseLeave()
{
	WindowEventArgs_<RFWnd> e(this);
	SendEvent(EventMouseLeave, e);
}

// 00407a60
void RFWnd::OnKeyDown(KeyEventArgs& e)
{
	SendEvent(EventKeyDown, e);
}

// 00407aa0
void RFWnd::OnKeyUp(KeyEventArgs& e)
{
	SendEvent(EventKeyUp, e);
}

// 00407ae0
void RFWnd::OnChar(CharEventArgs& e)
{
	SendEvent(EventChar, e);
}

// 00407b20
void RFWnd::OnActivated()
{
	EventArgs e;
	SendEvent(EventActivated, e);
}

// 00407b90
void RFWnd::OnDeactivated()
{
	EventArgs e;
	SendEvent(EventDeactivated, e);
}

// 00407c00: o foco ativa a janela sobreposta que contém esta
void RFWnd::OnSetFocus()
{
	GetOverlapped()->Activate();
	RFManager::GetInstance().NotifyWindowGotFocus(this);
	WindowEventArgs_<RFWnd> e(this);
	SendEvent(EventSetFocus, e);
}

// 00407d30
void RFWnd::OnKillFocus()
{
	RFManager::GetInstance().NotifyWindowLostFocus(this);
	WindowEventArgs_<RFWnd> e(this);
	SendEvent(EventKillFocus, e);
}

// 00407de0: atualiza os filhos; os fechados que já sumiram são destruídos no fim
#line 1499 "src\\RFWnd.cpp"
void RFWnd::OnUpdate(UpdateEventArgs& e)
{
	for (unsigned int i = 0; i < _children.size(); i++)
	{
		RFWnd* child = _children[i];
		if (child->_closeState != RFWNDCLOSESTATE_NONE983 && child->_visualState == VisualStateTypes::Hidden983)
			_updateList.push_back(child);
		else
			child->OnUpdate(e);
	}
	if (_visualEffect)
		_visualEffect->Update983(this, e.delta);
	if (_animation)
		_animation->FrameMove(this, e.delta);
	if (_hiding && _visualState == VisualStateTypes::Hidden983)
		_visible = false;
	SendEvent(EventUpdate, e);
	for (unsigned int i = 0; i < _updateList.size(); i++)
	{
		RFWnd* wnd = _updateList[i];
		std::vector<RFWnd*>::iterator itor = std::find(_children.begin(), _children.end(), wnd);
		assert(itor != _children.end());
		_children.erase(itor);
		wnd->Destroy();
		delete wnd;
	}
	_updateList.clear();
}

// 00408060
void RFWnd::OnDraw()
{
}

// 00408070
void RFWnd::OnAnimFinished(EventArgs& e)
{
}

// 00408080: fecha (com efeito visual: o OnUpdate do pai destrói quando o efeito termina)
void RFWnd::Close(RFWNDCLOSESTATE state)
{
	if (_closeState == RFWNDCLOSESTATE_NONE983)
	{
		_closeState = state;
		if (_visible)
		{
			if (_visualState != VisualStateTypes::Hiding983 && _visualEffect)
			{
				_visualState = VisualStateTypes::Hiding983;
				_visualEffect->Begin983(this);
			}
		}
		else if (_visualEffect)
			_visualState = VisualStateTypes::Hidden983;
		PlaySfx(_hideSound);
		WindowCloseEventArgs e(this, state);
		SendEvent(EventClose, e);
	}
}

// 00408150
void RFWnd::PlayAnimation(const std::wstring& name)
{
	if (_animation && !name.empty())
	{
		_animation->Play(this, name);
		AnimationEventArgs e;
		e.name.assign(name, 0, std::wstring::npos);
		SendEvent(EventPlayUIAnimation, e);
	}
}

// 004082e0
void RFWnd::StopAnimation()
{
	if (_animation)
		_animation->Stop();
}

// 00408300
bool RFWnd::IsAnimationPlaying() const
{
	if (_animation)
		return _animation->IsPlaying();
	return false;
}

// 00408320
void RFWnd::SetUserString(const std::wstring& key, const std::wstring& value)
{
	_userStrings[key] = value;
}

// 00408350 (chave ausente: texto vazio)
const std::wstring& RFWnd::GetUserString(const std::wstring& key) const
{
	std::map<std::wstring, std::wstring>::const_iterator itor = _userStrings.find(key);
	if (itor == _userStrings.end())
		return g_rfEmptyString983;
	return itor->second;
}

// 004083d0
bool RFWnd::IsUserStringDefined(const std::wstring& key) const
{
	return _userStrings.find(key) != _userStrings.end();
}

// 00408450
void RFWnd::SetPriority(int priority)
{
	_priority = priority;
	if (_super)
		_super->InvalidatePriority();
}
