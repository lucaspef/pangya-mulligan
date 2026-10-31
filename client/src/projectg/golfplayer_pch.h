#pragma once

// HACK: PCH variant for golfplayer.cpp only. The unit's COMDAT order needs CGolfDoc::GetTeamGameWinner,
// sUccRefreshItem's constructor, WBone::IsInvisible and CClub::TypeUpdated out of the shared PCH (they are
// defined by golfplayer.cpp at the positions of the original), and it reads CClub's flags as a dword.
// Other units keep their PCH.
#define REBANG_GOLFPLAYER_CPP
#include "stdafx.h"
