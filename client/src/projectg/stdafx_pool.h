#pragma once

// HACK: PCH variant for pool.cpp only. The original PCH held the inline bodies pool emits (WView::InFrustum(const
// Waabb&), WMatrix::operator~, WMatrix(const WPlane&), WPlane::Normalize, WView::SetReflective...). Adding them to
// the shared PCH moves the IL offsets and the name count every other unit depends on, so they are compiled only here.
#define REBANG_PCH_POOL
#include "stdafx.h"
