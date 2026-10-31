#pragma once
#include <string>
#include "rtti.h"
#include "frwnd.h"
#include "../../../include/engine/wtypes.h"

class FrGuiItem;
class FrWndManager;

class FrStatic : public FrWnd
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	FrStatic();
	virtual ~FrStatic();
	void Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent);
	void SetCaption(const char* caption); // 983: fora de linha (00c02500), corta em 100 bytes + "..."
	void SetCaption11(const char* caption)
	{
		m_caption = caption;
		m_smallFont = true;
	}
#ifdef REBANG_FR_LATE_PCH_INLINES
	const char* GetCaption();
#else
	const char* GetCaption() { return m_caption.c_str(); }
#endif
	void SetClippingArea(WRect* rect);
	void SetTextStyle(unsigned long style) { m_style = style; } // 983: sem corpo próprio na unidade
	void SetTextColor(unsigned long color, unsigned long outlineColor);
	virtual void Enable(bool enable);
	virtual void MoveWindow(const WPoint& point); // 983: 00c01ec0
	// 983: texto rolante (parâmetro "scrolling" do item). Nomes meus.
	void ScrollBack983(float elapsed, float speed); // 00c01d50
	void ScrollText983(float elapsed, float speed); // 00c01e40
	static bool __stdcall IsLongCaption983(const std::string& caption); // 00c01f30

protected:
	virtual void OnDraw();
	virtual void OnProc(const float deltaTime); // 983: 00c02160

	std::string m_caption;
	unsigned long m_color;
	unsigned long m_outlineColor;
	unsigned long m_style;
	unsigned long m_align;
	WRect* m_clip; // 983: 0xfc
	bool m_smallFont; // 983: 0x100
	bool m_scrolling983; // 983: 0x101
	WRect m_scrollRect983; // 983: 0x104 (posição/largura do texto desenhado)
}; // sizeof 0x114 (983: push em 00c0244a)
