#pragma once

// HACK: PCH variant for realmyroommainui.cpp only. This unit is the only one with ~1 MB of IL, so it is the only
// one that reaches the PCH offsets of the Fresh inlines (frwnd.h, frbutton.h, frstatic.h, fredit.h, frlistbox.h).
// In the original they are compiled after ~150 of the unit's class templates and interleaved (by IL offset) with
// its CreateForm<> instantiations; in the shared PCH their offsets are lower and frlistbox.h comes before
// frstatic.h/fredit.h. stdafx.h includes stdafx_realmyroommainui_fresh.h (through RB_PCH_INC_FRBUTTON) in place
// of frbutton.h: it has the original header order with filler in between, so the IL offsets and the name count
// of the shared PCH are untouched.
#define REBANG_PCH_REALMYROOMMAINUI
#define RB_PCH_INC_FRBUTTON "stdafx_realmyroommainui_fresh.h"
#include "stdafx.h"

// HACK: shareddoc.inl:602 in the original (an inline of the PCH); the shared shareddoc.inl cannot take it
// (IL offsets of the other units), so it lives here, at the IL offset that gives the original emission order.
inline bool __fastcall IsMannerPlayer()
{
	unsigned long games =
		WSingleton<CSharedDoc>::Instance()->m_myInfo.stat.dwGameCount;
	if (games < 1)
	{
		games = 1;
	}
	if ((float)WSingleton<CSharedDoc>::Instance()
				->m_myInfo.stat.dwNoMannerGameCount /
			(float)games * 100.0f <
		3.0f)
	{
		return true;
	}
	return false;
}
