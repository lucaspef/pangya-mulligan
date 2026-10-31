#pragma once

// HACK: in the original the tchar.h inlines (_tcsncpy, _tcscmp, _tcsncmp) are
// merged about 700 bytes of PCH IL later into this unit's tail functions
// (GetLastError .. QueryHTTPResponse). Padding the shared PCH there moves
// clientsetting, shareddoc and browser, so this unit has its own PCH wrapper
// with the pad in front (27 increments; the window is 25..30).
// clang-format off
#define ILFILL9 v += 1; v += 1; v += 1; v += 1; v += 1; v += 1; v += 1; v += 1; v += 1;
namespace ILFILLTCHAR
{
	struct Pad
	{
		int v;
		void g() { ILFILL9 ILFILL9 ILFILL9 }
	};
}
#undef ILFILL9
// clang-format on

#include "stdafx.h"
