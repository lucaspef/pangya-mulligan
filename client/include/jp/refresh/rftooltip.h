#pragma once
#include <string>
#include "rfbase.h"

class FrGraphicInterface;

// JP 983: dicas da RF (unidade rfcommontooltip.cpp 005ce010–005cfe30). RTTI .?AVRFTooltip@@ (vtable 00d108a4),
// .?AVRFCommonTooltip@@ (00d1095c), .?AVRFCommonTooltipByImage@@ (00d109fc); base RFFormBase<RFWndEx>; 0x1bc bytes (as duas
// derivadas não têm campos). Janelas de "tooltip_common_wnd.xml" / "tooltip_common_wnd2.xml", criadas pelo RFTooltipHelper.
// A dica segue o mouse (m_bFollowMouse) ou fica embaixo da janela-alvo; some quando o mouse sai do alvo. Nomes meus, menos as
// classes e os da RF.
class RFTooltip : public RFFormBase<RFWndEx>
{
public:
	RFTooltip(); // 005ce0c0
	virtual ~RFTooltip() { } // ??_G 005ce410

	// 005cedb0: assina o EventUpdate (Update983) e pega o FrGraphicInterface da tela para medir o texto
	virtual void OnInitialUpdate();
	// 005ce160 (slot 34, +0x88): alvo (NULL = segue o mouse), estilo (0 = simples, 1 = escuro), tamanho da fonte
	virtual void SetTooltip983(RFWnd* target, int style, int fontSize);
	virtual void Show983(bool show); // 005ce010 (slot 35, +0x8c)
	virtual void UpdateLayout983() = 0; // slot 36 (+0x90)
	virtual void Update983(EventArgs& e); // 005ce8c0 (slot 37, +0x94)

	float GetLineHeight983(int fontSize); // 005ce020
	float GetTextWidth983(const std::wstring& text, int fontSize); // 005ceec0
	RFSize MeasureText983(const std::wstring& text, int fontSize); // 005cf390

protected:
	bool m_bShow; // +0x184
	FrGraphicInterface* m_pGraphic; // +0x188
	RFVector2 m_pos; // +0x18c (posição na tela)
	RFRect m_targetRect; // +0x194
	bool m_bFollowMouse; // +0x1a4
	RFWnd* m_pTarget; // +0x1a8
	int m_style; // +0x1ac
	RFWnd* m_pFrame; // +0x1b0
	RFTextArea* m_pTextArea; // +0x1b4
	RFPicture* m_pPicture; // +0x1b8 (só a RFCommonTooltipByImage)
};

// texto; criada por Create983 (00e781f0 guarda o pai)
class RFCommonTooltip : public RFTooltip
{
public:
	RFCommonTooltip() { } // 005ce350 (cópia fora de linha)
	virtual ~RFCommonTooltip() { } // ??_G 005ce380

	static RFCommonTooltip* Create983(RFWnd* parent); // 005ceb30
	void SetText983(const std::wstring& text); // 005ce060
	virtual void SetTooltip983(RFWnd* target, int style, int fontSize); // 005cf700
	virtual void UpdateLayout983(); // 005cf9d0

	static RFWnd* s_pParent983; // 00e781f0

private:
	static void ShowChild983(RFWnd* wnd, const wchar_t* name, bool show); // 005cf1f0
};

// imagem + texto; 00e781f8 guarda o pai
class RFCommonTooltipByImage : public RFTooltip
{
public:
	RFCommonTooltipByImage() { } // 005ce3b0 (cópia fora de linha)
	virtual ~RFCommonTooltipByImage() { } // ??_G 005ce3e0

	static RFCommonTooltipByImage* Create983(RFWnd* parent); // 005cec70
	void SetImageText983(const std::wstring& image, const std::wstring& text); // 005cf080
	virtual void SetTooltip983(RFWnd* target, int style, int fontSize); // 005cfab0
	virtual void UpdateLayout983(); // 005cfcb0

	static RFWnd* s_pParent983; // 00e781f8

private:
	static void ShowChild983(RFWnd* wnd, const wchar_t* name, bool show); // 005cf2c0
};
