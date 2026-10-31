#pragma once

// HACK: PCH variant for quadtree.cpp only. In the original every PCH inline that quadtree uses (Waabb::IsInclude,
// operator new(size_t, void*), ...) is compiled AFTER the unit's own functions and templates: the IL stream of the
// original quadtree.cpp ended before the PCH offset of wmath.h. Ours is several times larger (naked asm bodies plus
// their dead C copies), so the same inlines would be merged into the middle of the unit. This filler, placed before
// every other PCH inline, moves their offsets past the end of our quadtree.cpp without touching the shared PCH.
// clang-format off
#define RB_QT_ILFILL4 v += 1; v += 1; v += 1; v += 1;
#define RB_QT_ILFILL16 RB_QT_ILFILL4 RB_QT_ILFILL4 RB_QT_ILFILL4 RB_QT_ILFILL4
#define RB_QT_ILFILL64 RB_QT_ILFILL16 RB_QT_ILFILL16 RB_QT_ILFILL16 RB_QT_ILFILL16
#define RB_QT_ILFILL256 RB_QT_ILFILL64 RB_QT_ILFILL64 RB_QT_ILFILL64 RB_QT_ILFILL64
#define RB_QT_ILFILLFN(n) void f##n() { RB_QT_ILFILL256 }
#define RB_QT_ILFILLFN10(n) \
	RB_QT_ILFILLFN(n##0) RB_QT_ILFILLFN(n##1) RB_QT_ILFILLFN(n##2) RB_QT_ILFILLFN(n##3) RB_QT_ILFILLFN(n##4) \
	RB_QT_ILFILLFN(n##5) RB_QT_ILFILLFN(n##6) RB_QT_ILFILLFN(n##7) RB_QT_ILFILLFN(n##8) RB_QT_ILFILLFN(n##9)
#define RB_QT_ILFILLFN100(n) \
	RB_QT_ILFILLFN10(n##0) RB_QT_ILFILLFN10(n##1) RB_QT_ILFILLFN10(n##2) RB_QT_ILFILLFN10(n##3) RB_QT_ILFILLFN10(n##4) \
	RB_QT_ILFILLFN10(n##5) RB_QT_ILFILLFN10(n##6) RB_QT_ILFILLFN10(n##7) RB_QT_ILFILLFN10(n##8) RB_QT_ILFILLFN10(n##9)
struct RbQuadTreeIlFill
{
	int v;
	RB_QT_ILFILLFN100(00)
	RB_QT_ILFILLFN100(01)
	RB_QT_ILFILLFN100(02)
};
// clang-format on

#define REBANG_PCH_QUADTREE
#include "stdafx.h"
