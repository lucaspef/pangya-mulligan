#pragma once

// HACK: PCH variant for taskmain.cpp only. c2 merges the PCH inlines into the TU by IL offset, and
// taskmain.obj shows the include order of the original PCH (inlines after its main pass): bitmap.h,
// wview.h, wfont.h, wflag.h, objectfactory.h, frwnd.h, frbutton.h, frstatic.h, frform.h, frtabbutton.h,
// frwndmanager.h, frgraphicinterface.h, fresh.h, packet.h, soundmanager.h, inputmanager.h,
// globalgamedefine.h, taskmanager.h, shareddoc.h. This variant includes them in that order and has the
// inline bodies the original headers had; other units keep their PCH (emission order, canaries).
#define REBANG_TASKMAIN_PCH
#include "stdafx.h"
