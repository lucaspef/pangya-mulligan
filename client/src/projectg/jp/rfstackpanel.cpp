// JP 983: RFStackPanel.cpp da Refresh (B209, F3.7.13): o painel que empilha os filhos na vertical ou na horizontal
// (0044e710–0044ecd0). Sem fonte no 645/839: escrito pelo asm do 983 (out/F3.7/s209). Os filhos são percorridos do último para
// o primeiro; cada um ocupa (tamanho local + tamanhos dos modificadores). _hAlign983 alinha o filho no eixo cruzado,
// _vAlign983 alinha o bloco no eixo da pilha (0 = início, 1 = centro, 2 = fim). ToString/GetTypeName em linha no rfcontrols.h.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

// 0044e710
RFStackPanel::RFStackPanel()
{
}

// 0044e730
RFStackPanel::~RFStackPanel()
{
}

// 0044e740
void RFStackPanel::Initialize()
{
	RFWnd::Initialize();
	switch (_orientation)
	{
	case OrientationTypes::Vertical:
		StackUpVertically();
		break;
	case OrientationTypes::Horizontal:
		StackUpHorizontally();
		break;
	}
}

// 0044e780
void RFStackPanel::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	StackPanelBase* panel = static_cast<StackPanelBase*>(base);
	_hAlign983 = panel->_hAlign.Get();
	_vAlign983 = panel->_vAlign.Get();
	_orientation = (OrientationTypes::Type)panel->_orientation.Get();
}

// 0044e7c0
OrientationTypes::Type RFStackPanel::GetOrientation() const
{
	return _orientation;
}

// 0044e7d0
void RFStackPanel::SetOrientation(OrientationTypes::Type orientation)
{
	_orientation = orientation;
}

// 0044e7e0
void RFStackPanel::OnLayoutChanged(unsigned int flags)
{
	if (flags & 2)
	{
		switch (_orientation)
		{
		case OrientationTypes::Vertical:
			StackUpVertically();
			break;
		case OrientationTypes::Horizontal:
			StackUpHorizontally();
			break;
		}
	}
	for (std::vector<RFWnd*>::iterator it = _children.begin(); it != _children.end(); ++it)
		(*it)->ApplyAnchor();
}

// 0044e840
void RFStackPanel::StackUp()
{
	switch (_orientation)
	{
	case OrientationTypes::Vertical:
		StackUpVertically();
		break;
	case OrientationTypes::Horizontal:
		StackUpHorizontally();
		break;
	}
	for (std::vector<RFWnd*>::iterator it = _children.begin(); it != _children.end(); ++it)
		(*it)->ApplyAnchor();
}

// 0044e890
void RFStackPanel::StackUp_Impl()
{
	switch (_orientation)
	{
	case OrientationTypes::Vertical:
		StackUpVertically();
		break;
	case OrientationTypes::Horizontal:
		StackUpHorizontally();
		break;
	}
}

// 0044e8c0
void RFStackPanel::StackUpVertically()
{
	float total = 0.0f;
	for (std::vector<RFWnd*>::iterator it = _children.begin(); it != _children.end(); ++it)
		total += (*it)->GetSize().height;
	RFSize mine = GetSize();
	float y = 0.0f;
	float x = 0.0f;
	switch (_vAlign983)
	{
	case 0:
		break;
	case 1:
		y = (float)((mine.height - total) * 0.5);
		break;
	case 2:
		y = mine.height - total;
		break;
	}
	for (std::vector<RFWnd*>::iterator it = _children.end(); it != _children.begin();)
	{
		--it;
		RFWnd* child = *it;
		switch (_hAlign983)
		{
		case 0:
			x = 0.0f;
			break;
		case 1:
			x = (float)((mine.width - child->GetSize().width) * 0.5);
			break;
		case 2:
			x = mine.width - child->GetSize().width;
			break;
		}
		float px = (float)(int)x;
		float py = (float)(int)y;
		if (px != child->_localPos.x || py != child->_localPos.y)
		{
			// (o 983 grava o y no slot +0x48 da âncora, o da direita; igual ao binário)
			child->_anchorValue.left = px;
			child->_localPos.x = px;
			child->_anchorValue.right = py;
			child->_localPos.y = py;
			child->OnLayoutChanged(1);
		}
		float used = child->_localSize.height;
		for (std::vector<RFWindowModifier*>::iterator m = child->_modifiers.begin(); m != child->_modifiers.end(); ++m)
			used += (*m)->GetSize().height;
		y += used;
	}
}

// 0044eac0
void RFStackPanel::StackUpHorizontally()
{
	float total = 0.0f;
	for (std::vector<RFWnd*>::iterator it = _children.begin(); it != _children.end(); ++it)
		total += (*it)->GetSize().width;
	RFSize mine = GetSize();
	float x = 0.0f;
	float y = 0.0f;
	switch (_hAlign983)
	{
	case 0:
		break;
	case 1:
		x = (float)((mine.width - total) * 0.5);
		break;
	case 2:
		x = mine.width - total;
		break;
	}
	for (std::vector<RFWnd*>::iterator it = _children.end(); it != _children.begin();)
	{
		--it;
		RFWnd* child = *it;
		switch (_vAlign983)
		{
		case 0:
			y = 0.0f;
			break;
		case 1:
			y = (float)((mine.height - child->GetSize().height) * 0.5);
			break;
		case 2:
			y = mine.height - child->GetSize().height;
			break;
		}
		float px = (float)(int)x;
		float py = (float)(int)y;
		if (px != child->_localPos.x || py != child->_localPos.y)
		{
			child->_anchorValue.left = px;
			child->_localPos.x = px;
			child->_anchorValue.right = py;
			child->_localPos.y = py;
			child->OnLayoutChanged(1);
		}
		float used = child->_localSize.width;
		for (std::vector<RFWindowModifier*>::iterator m = child->_modifiers.begin(); m != child->_modifiers.end(); ++m)
			used += (*m)->GetSize().width;
		x += used;
	}
}
