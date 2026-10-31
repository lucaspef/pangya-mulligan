#pragma once

// HACK: PCH variant for resultdlg.cpp only. c2 merges the PCH inlines into the TU by IL offset, and
// resultdlg emits FrForm::DisableFlag (frform.h) before FrGaugeBar::SetBorder (frgaugebar.h), so this
// PCH includes frgaugebar.h after frform.h. Other units keep their PCH (emission order).
#define REBANG_RESULTDLG_PCH
#include "stdafx.h"
