#pragma once

// HACK: PCH variant for taskmanager.cpp only. taskmanager.obj needs inline bodies, declarations and
// template instantiations in the precompiled headers that the original headers had (emission order
// of its COMDATs); other units keep their PCH (IL offsets and name-count canaries).
#define REBANG_TASKMANAGER_PCH
#include "stdafx.h"
