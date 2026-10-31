#pragma once

// HACK: PCH variant for golftask.cpp only. c2 merges the PCH inlines into the TU by IL offset, and
// golftask's COMDAT order needs float2int (commonutil.h), FrDesktop::SetBgColor, CTask::IsInited,
// CGolfDoc::IsReported and sFindRival (shareddoc.h) as PCH inlines at the positions the original PCH
// has them (after the unit's main pass and template rounds). Other units keep their PCH.
#define REBANG_GOLFTASK_PCH
#include "stdafx.h"
