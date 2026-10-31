#pragma once

// HACK: golfrule.cpp PCH variant only (REBANG_GOLFRULE_PCH). Inline bodies in the order the original
// PCH has them (c2 emits the PCH inlines past the end of the TU in PCH order).
__forceinline void WFlags::Enable(unsigned long flag)
{
	this->m_flag |= flag;
}

inline bool CChatMsg::IsOpened()
{
	return m_iState == 3;
}
inline bool CChatMsg::IsClosed()
{
	return m_iState == 0;
}
inline bool CChatMsg::IsActive() const
{
	return m_bActive;
}
inline WFont* CChatMsg::GetMaskedFont() const
{
	return m_pMaskedFont;
}
inline void CChatMsg::Toggle(bool toggle)
{
	m_bToggle = toggle;
}

inline void FrWnd::SetAlpha(float alpha)
{
	m_wndAlpha = alpha;
}
inline void FrWnd::HidePrivacy(bool hide)
{
	m_nFlags.Turn(FWF_PRIVACY, hide);
}
inline bool FrWnd::HasKeyFocus()
{
	unsigned char b = (unsigned char)((unsigned long)m_nFlags >> 4);
	return (b & 1) != 0 ? true : false;
}
inline FrScrollBar* FrWnd::GetScrollBar()
{
	return m_pScrBar;
}

inline FrButton::eButMode FrButton::GetStatus()
{
	return m_status;
}

inline int FrScrollBar::GetRowCapacity()
{
	return m_rowCapacity;
}

inline const char* FrEdit::GetEditText_Front()
{
	return m_editText[ETT_FRONT].c_str();
}
inline const char* FrEdit::GetEditText_Comp()
{
	return m_editText[ETT_COMP].c_str();
}
inline const char* FrEdit::GetEditText_End()
{
	return m_editText[ETT_END].c_str();
}
inline void FrEdit::SetFontColor(unsigned long color)
{
	m_fontColor = color;
}
inline float FrEdit::GetWidthLimit() const
{
	return m_widthLimit;
}

inline void Fresh::Toggle(bool s)
{
	show = s;
}
