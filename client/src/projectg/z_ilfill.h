#pragma once

// HACK! This file contains filler to balance IL offsets between the PCH and TU.
// Ideally this file will disappear as we zero in on the actual contents of the
// PCH and TUs, but it will take a while.

// clang-format off
#define ILFILL4 v += 1; v += 1; v += 1; v += 1;
#define ILFILL16 ILFILL4 ILFILL4 ILFILL4 ILFILL4
#define ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL16
#define ILFILL256 ILFILL64 ILFILL64 ILFILL64 ILFILL64

#define ILFILLFN(n) void f##n() { ILFILL256 }
#define ILFILLFN10(n) \
	ILFILLFN(n##0) ILFILLFN(n##1) ILFILLFN(n##2) ILFILLFN(n##3) \
	ILFILLFN(n##4) ILFILLFN(n##5) ILFILLFN(n##6) ILFILLFN(n##7) \
	ILFILLFN(n##8) ILFILLFN(n##9)

#ifdef REBANG_CLIENTSETTING_PCH
#define ILFILL1 \
	namespace ILFILL1 \
	{ \
		struct Pad \
		{ \
			int v; \
			ILFILLFN(10) ILFILLFN(11) ILFILLFN(12) ILFILLFN(13) \
			ILFILLFN(14) \
			void g() { ILFILL16 ILFILL16 ILFILL4 ILFILL4 v += 1; v += 1; v += 1; } \
		}; \
	}
#else
#define ILFILL1 \
	namespace ILFILL1 \
	{ \
		struct Pad \
		{ \
			int v; \
			ILFILLFN(10) ILFILLFN(11) ILFILLFN(12) ILFILLFN(13) \
			void g() { ILFILL64 ILFILL64 ILFILL64 ILFILL4 ILFILL4 v += 1; v += 1; v += 1; } \
		}; \
	}
#endif
// npc: 8 fewer increments pay for WPuppet::GetFrameDataTime (wpuppet.h).
// ghostgamedlg: 46 fewer increments (one ILFILLFN traded for 261) pay for Ceil and the FrListItem
// inlines defined right before ILFILL1 in stdafx.h.

// avatar_avatar: 135 fewer increments pay for RadianNormalize2PI/RadianNormalizePI/RadianDelta (wmath.h),
// operator!=(WVector) (wmath.inl) and RayIntersectAABB (wmathutil.inl); ILFILL2 lost 6 for
// Fresh::IsVisible (stdafx.h, after fremoticon.h).

// HACK: 10 fewer increments pay for WVector::Negate (wmath.inl) and keep itemmanager in order.
// camerawork: 96 fewer increments (one ILFILLFN traded for 261) pay for the PCH inlines it needs in
// source order: WDotProduct(WVector) and RotMat(const WVector&, float) in wmath.inl, GetFrameData and
// GetMatrix in wpuppet.h.
#define ILFILL0 \
	namespace ILFILL0 \
	{ \
		struct Pad \
		{ \
			int v; \
			ILFILLFN(0) ILFILLFN(1) ILFILLFN(2) ILFILLFN(3) \
			ILFILLFN(4) \
			\
			void g() { ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 v += 1; v += 1; v += 1; } \
		}; \
	}

#define ILFILL2 \
	namespace ILFILL2 \
	{ \
		struct Pad \
		{ \
			int v; \
			ILFILLFN10(1) ILFILLFN10(2) ILFILLFN(0) ILFILLFN(1) \
			void g() { ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 ILFILL4 } \
		}; \
	}

#define ILFILL2A \
	namespace ILFILL2A \
	{ \
		struct Pad \
		{ \
			int v; \
			ILFILLFN(0) \
			void g() { ILFILL256 ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 ILFILL4 ILFILL4 v += 1; v += 1; v += 1; } \
		}; \
	}

#define ILFILL3 \
	namespace ILFILL3 \
	{ \
		struct Pad \
		{ \
			int v; \
			void g() { ILFILL256 ILFILL256 ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 ILFILL4 ILFILL4 v += 1; v += 1; } \
		}; \
	}

#define ILFILLB1 \
	template <int N> struct TuFill { int v; void g() { ILFILL256 ILFILL16 ILFILL4 v += 1; v += 1; v += 1; } }; \
	template <int N> struct TuMid { void g() { TuFill<N>().g(); } }; \
	template <int N> struct TuPad \
	{ \
		int v; \
		void g() { ILFILL256 ILFILL16 ILFILL16 ILFILL16 v += 1; v += 1; TuMid<N>().g(); } \
	}; \
	namespace TUPAD1 \
	{ \
		struct Pad \
		{ \
			int v; \
			ILFILLFN(0) ILFILLFN(1) ILFILLFN(2) ILFILLFN(3) \
			void g() { ILFILL16 ILFILL16 v += 1; v += 1; v += 1; ILFILL64 ILFILL64 ILFILL64 ILFILL16 ILFILL4 ILFILL4 ILFILL4 v += 1; v += 1; v += 1; } \
		}; \
	}

#define ILFILLB2 \
	namespace TUPAD2 \
	{ \
		struct Pad \
		{ \
			void g() { TuPad<2>().g(); } \
		}; \
	}
#define ILFILLW1 \
	namespace TUPADW1 \
	{ \
		struct Pad \
		{ \
			int v; \
			void g() { ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 v += 1; ILFILL64 ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 ILFILL4 v += 1; v += 1; } \
		}; \
	}

#define ILFILLTU_SHAREDDOC \
	namespace ILFILLTU_SHAREDDOC \
	{ \
		struct Pad \
		{ \
			int v; \
			ILFILLFN(0) \
			void g() { ILFILL16 ILFILL16 } \
		}; \
	}

// HACK! The number of inline functions in a PCH impacts scheduling ties in c2. This is knbown to impact:
// - projectg SaveScreenShot
// - WPolySoup::MakeStatistics
// These can be removed as we fill in unknown inlines in our headers...
inline void ILCOUNT0(){}
inline void ILCOUNT1(){}
inline void ILCOUNT2(){}
inline void ILCOUNT3(){}
inline void ILCOUNT4(){}
inline void ILCOUNT5(){}
inline void ILCOUNT6(){}
// HACK! Declarations cost no IL but add names to the PCH; the merge of upstream 1fbe523 into our
// tree needed 32..40 more; globalnetworkdefine.h in the PCH (transferableitem) brought 33 of them.
// uccmanager.h in the PCH (on top of transferableitem) needs 3 declarations plus 12 empty inlines
// (MakeStatistics, SaveScreenShot, OnInit); their IL is paid for by ILFILL0 (window 36..51
// increments for itemmanager, 43 used).
void ILDECL4();
void ILDECL5();
void ILDECL6();
inline void ILDECL7(){}
inline void ILDECL8(){}
inline void ILDECL9(){}
inline void ILDECL10(){}
inline void ILDECL11(){}
inline void ILDECL12(){}
inline void ILDECL13(){}
inline void ILDECL14(){}
inline void ILDECL15(){}
inline void ILDECL16(){}
inline void ILDECL17(){}
inline void ILDECL18(){}
// clang-format on

// HACK: stdafx.h includes frbutton.h through this macro; only the realmyroommainui PCH variant
// (stdafx_realmyroommainui.h) points it to its own wrapper header.
#ifndef RB_PCH_INC_FRBUTTON
#define RB_PCH_INC_FRBUTTON "frbutton.h"
#endif
