#pragma once

#include "frwnd.h"

#ifndef REBANG_TASKMAIN_PCH // taskmain_pch.h: in the class (frwnd.h)
inline void FrWnd::SetFixed(bool fixed)
{
	m_dwStyle.Turn(FWS_FIXED, fixed);
}
#endif

inline float FrWnd::GetAlpha() const
{
	return m_wndAlpha;
}

#ifndef REBANG_GOLFRULE_PCH // golfrule.cpp: bodies in golfrule_pch_inl.h (emission order)
inline void FrWnd::SetAlpha(float alpha)
{
	m_wndAlpha = alpha;
}

inline FrScrollBar* FrWnd::GetScrollBar()
{
	return m_pScrBar;
}
#endif
