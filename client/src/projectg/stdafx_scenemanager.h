#pragma once

// HACK: PCH variant for scenemanager.cpp only. The original PCH held the full CSceneManager declaration and the
// inline bodies scenemanager.obj emits (SceneElement, CustomElement, WBone::GetAlpha/GetLight). Adding them to the
// shared PCH moves the IL offsets and the name count every other unit depends on, so they are compiled only here.
#define REBANG_PCH_SCENEMANAGER
#include "stdafx.h"
