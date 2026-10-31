#pragma once

// HACK! Second IL filler of the golfrule PCH variant (see golfrule_pch.h): included right before wmath.h so
// the PCH inlines from wmath.h on stay past the end of golfrule.cpp's IL (after its end-of-TU templates),
// as in the original. Any size large enough works.
#include "z_golfrule_ilfill.h"

// clang-format off
namespace GRFILL
{
	struct Pad2
	{
		int v;
		GRFILLFN(0) GRFILLFN(1) GRFILLFN(2) GRFILLFN(3)
	};
}
// clang-format on
