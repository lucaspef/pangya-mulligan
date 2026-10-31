#pragma once

// HACK! IL filler for the golfrule PCH variant (see golfrule_pch.h). Included between the ATL core
// headers and atlmisc.h so every PCH inline from atlmisc.h on moves later by the same amount.
#include "z_ilfill.h"

// clang-format off
#define GRFILL1K ILFILL256 ILFILL256 ILFILL256 ILFILL256
#define GRFILL4K GRFILL1K GRFILL1K GRFILL1K GRFILL1K
#define GRFILLFN(n) void f##n() { GRFILL4K }

namespace GRFILL
{
	struct Pad
	{
		int v;
		GRFILLFN(0)
		GRFILLFN(1)
		GRFILLFN(2)
		GRFILLFN(3)
		GRFILLFN(4)
		GRFILLFN(5)
		GRFILLFN(6)
		GRFILLFN(7)
		void g() { GRFILL1K ILFILL256 ILFILL16 ILFILL4 ILFILL4 v += 1; v += 1; v += 1; }
	};
}
// clang-format on
