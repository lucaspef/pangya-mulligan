#pragma once

// HACK: PCH variant for scoreboard.cpp only. c2 merges the PCH inlines into the TU by IL offset, and
// scoreboard's COMDAT order needs two things the shared PCH does not have: the Fresh inlines in the
// order frframe.h, frstatic.h, frform.h, frdesktop.h, frlistbox.h, frarea.h (REBANG_FR_LATE_PCH_INLINES)
// and <new> after wmath.h (placement new is then compiled after the unit's template rounds, so
// std::_Construct<_Node*> waits for the next wave). Other units keep their PCH.
#define REBANG_SCOREBOARD_PCH
#include "stdafx.h"
