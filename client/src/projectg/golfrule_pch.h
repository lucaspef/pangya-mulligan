#pragma once

// HACK: PCH variant for golfrule.cpp only. Most of golfrule.cpp is byte-exact naked asm, whose IL is
// much larger than the IL of the original C, so the PCH inlines (which c2 merges into the TU by IL
// offset) would be emitted far too early. This variant pads the PCH IL (z_golfrule_ilfill.h) so the
// PCH inlines land where the original has them, and other units keep their PCH.
#define REBANG_GOLFRULE_PCH
#include "stdafx.h"
