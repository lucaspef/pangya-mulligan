#pragma once

// HACK: PCH variant for realmyroomdata.cpp only. The original PCH held Wobb::operator*=(const WVector&) right before
// Wobb::operator=(const Wobb&) (wmath.inl) and CSceneManager::SetAlpha. Adding them to the shared PCH moves the IL
// offsets and the name count every other unit depends on, so they are compiled only here.
#define REBANG_PCH_REALMYROOMDATA
#include "stdafx.h"
