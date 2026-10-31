#pragma once

// HACK: PCH variant for transferableitem.cpp only. The original PCH instantiated
// list<sTradeItem>::erase(iterator) (transferableitem emission order), but any reference to it in the
// shared PCH breaks the template order of shareddoc.cpp.
#define REBANG_PCH_TRADE_ERASE
#include "stdafx.h"
