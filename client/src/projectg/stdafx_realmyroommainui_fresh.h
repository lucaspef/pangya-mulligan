#pragma once

// HACK: see stdafx_realmyroommainui.h. Filler sizes were measured against the original emission order.
// clang-format off
#define RB_RMMUI_FILL_0 \
	namespace RbRealMyRoomMainUiFill \
	{ \
		struct Pad0 \
		{ \
			int v; \
			ILFILLFN(090) ILFILLFN(091) ILFILLFN(092) ILFILLFN(093) \
			void g() { ILFILL64 ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 v += 1; v += 1; } \
		}; \
	}
#define RB_RMMUI_FILL_A \
	namespace RbRealMyRoomMainUiFill \
	{ \
		struct PadA \
		{ \
			int v; \
			void g() { ILFILL16 ILFILL16 ILFILL16 } \
		}; \
	}
#define RB_RMMUI_FILL_B
#define RB_RMMUI_FILL_C
#define RB_RMMUI_FILL_D \
	namespace RbRealMyRoomMainUiFill \
	{ \
		struct PadD \
		{ \
			int v; \
			void g() { ILFILL64 } \
		}; \
	}
#define RB_RMMUI_FILL_E \
	namespace RbRealMyRoomMainUiFill \
	{ \
		struct PadE \
		{ \
			int v; \
			void g() { ILFILL64 ILFILL64 ILFILL16 ILFILL4 ILFILL4 } \
		}; \
	}
#define RB_RMMUI_FILL_F void RbRealMyRoomMainUiFill() { int v = 0; ILFILL16 ILFILL16 }
// clang-format on

// HACK: the original PCH reached classdefine.h (IFF_STRUCT::sItem::IsRandomBox) before the Fresh headers.
// clang-format off
#define RB_RMMUI_FILL_G \
	namespace RbRealMyRoomMainUiFill \
	{ \
		struct PadG \
		{ \
			int v; \
			ILFILLFN10(G0) ILFILLFN(G90) ILFILLFN(G91) ILFILLFN(G92) \
			ILFILLFN(G93) ILFILLFN(G94) ILFILLFN(G95) ILFILLFN(G96) \
			ILFILLFN(G97) ILFILLFN(G98) \
			void g() { ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 ILFILL4 v += 1; v += 1; } \
		}; \
	}
// clang-format on
RB_RMMUI_FILL_G
#include "../shared/classdefine.h"
RB_RMMUI_FILL_0
#include "frwnd.h"
RB_RMMUI_FILL_A
#include "frbutton.h"
RB_RMMUI_FILL_B
#include "frstatic.h"
RB_RMMUI_FILL_C
#include "frgraphicinterface.h"
#include "fredit.h"
RB_RMMUI_FILL_D
// HACK: the original frlistbox.h had more inline code between GetPreviousSelected and ResetTopRow; the
// filler member is spliced in after the UnselectAllItem declaration (frlistbox.h itself stays untouched).
#define UnselectAllItem() \
	UnselectAllItem(); \
	RB_RMMUI_FILL_F void RbRealMyRoomMainUiFillEnd()
#include "frlistbox.h"
#undef UnselectAllItem
RB_RMMUI_FILL_E
#include "frtabbutton.h"
#include "frwndinl.h"
#include "frgaugebar.h"
