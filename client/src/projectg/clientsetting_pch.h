#pragma once

// HACK: clientsetting's original PCH has different inline definition positions.
// Use the shared headers with its own include/IL layout so other units keep their PCH.
// HACK: the original first object requests CRT libraries before later ATL/PCH directives.
#pragma comment(lib, "MSVCRT")
#pragma comment(lib, "OLDNAMES")

#define REBANG_CLIENTSETTING_PCH
#include "stdafx.h"
