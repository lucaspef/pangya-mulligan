#pragma once

#if !defined(_MSC_VER) || _MSC_VER == 1600
#define WINVER 0x0400
#define _WIN32_WINNT 0x0400
#else // ATL do toolset novo exige _WIN32_WINNT >= 0x0501 (HMONITOR em atlwin.h; projeto do VS, D736)
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#endif
#define _ATL_STATIC_REGISTRY

#define WLIST_ALLOC(size) malloc(size)
#define WLIST_FREE(ptr) free(ptr)

#include <winsock2.h>
#include <windows.h>
#include "../../include/engine/wtypes.h"
#ifdef REBANG_CLIENTSETTING_PCH
#define REBANG_PCH_LATE_ATL_WIN_MODULE
#endif
#include <atlbase.h>
#include <atlapp.h>
extern WTL::CAppModule _Module;
#include <atlwin.h>
#ifdef REBANG_GOLFRULE_PCH
#include "z_golfrule_ilfill.h"
#endif
#include <atlmisc.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>

#ifndef REBANG_SCOREBOARD_PCH // HACK: scoreboard_pch.h (<new> comes later in its PCH)
#include <math.h>
#include <stdexcept>
#endif
#define REBANG_PROJECTG_PCH // HACK: see RotMat in wmath.inl
#ifndef REBANG_SCOREBOARD_PCH
#include <math.h>
#include <xutility>
#endif
#ifdef REBANG_CLIENTSETTING_PCH
#define REBANG_PCH_MATH_HELPERS
#endif
#ifdef REBANG_GOLFRULE_PCH
#include "z_golfrule_ilfill2.h"
#endif
#include "wmath.h"
#include "gamath.h"
#include "wutil.h"
#include "wminmax.h"
#include "wlist.h"
#include "wmemblock.h"
#include "wlock.h"
#include "wmempak.h"
#include "rectmng.h"
#include "wscene.h"
#include "wvideo.h"
#include "wmesh.h"
#include "wresrcmng.h"
#include "wdevmng.h"
#include "wxtnlbuffer.h"
#include "xzip.h"
#include "cfile.h"
#include "wreg.h"
#include "bitmap.h"
#ifdef REBANG_TASKMAIN_PCH // HACK: see taskmain_pch.h
#include "wview.h"
#include "wfont.h"
#endif
#include "wavi.h"
#include "wflag.h"
#include "wangrealapp.h"
#include "westpak.h"
#include "wblockmodel.h"
#include "wboneset.h"
// HACK: wmath.h should include it; here for the symbol ordering
#include "wmathutil.inl"
#include "wpetfile.h"
#ifndef REBANG_TASKMAIN_PCH // taskmain_pch.h: after packet.h
#include "soundmanager.h"
#endif
// 983: RotMat(float, char) fica em wmath.inl (include/engine, layout do 983)
#include "z_ilfill.h"
ILFILL0
#ifdef REBANG_CLIENTSETTING_PCH
// 983: Wabs/WisZero/WisEqual ficam em wmath.h (include/engine, layout do 983)
#include "background.h"
// 983: WView::GetFOV é função de verdade (00b4fa30, wview.cpp); sem REBANG_PCH_GETFOV
#include "wview.h"
#include "wlocalize.h"
#endif
#ifdef REBANG_GOLFTASK_PCH // golftask.cpp: commonutil.h:70 in the original, before rtti.h in its PCH (emission order)
inline int float2int(float value)
{
	return (int)value;
}
#endif
#include "objectfactory.h"
#ifndef REBANG_CLIENTSETTING_PCH
#include "background.h"
#include "wpuppet.h"
#ifdef REBANG_PCH_QUADTREE // HACK: only in the quadtree PCH, other units keep their IL (emission order)
#include "wmathutil.h"
#endif
#include "wview.h"
#endif
#include "w3danispr.h"
#include "woverlay.h"
#include "singleton.h"

#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>

#include "basicdebug.h"
#include "commonutil.h"
#ifndef REBANG_GOLFRULE_PCH // golfrule.cpp includes it itself (original emission order)
#include "slicestr.h"
#endif

#include "tinyxml.h"

#include "../shared/exceptionreport.h"
#include "lock.hpp"
#if !defined(REBANG_CLIENTSETTING_PCH) && !defined(REBANG_TASKMAIN_PCH)
#include "packet.h"
#endif
#include "titles_client.h"
#if !defined(REBANG_CLIENTSETTING_PCH) && !defined(REBANG_TASKMAIN_PCH)
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
#include "chatmsg.h"
// HACK: inline in the original chatmsg.h (taskmanager.obj has its own copy, between the inlines of
// objectfactory.h and errorcode.h)
inline void CChatMsg::SetChatFacialUser(std::string& user)
{
	m_chatFacialUser = user;
}
#include "../shared/errorcode.h"
#endif
#include "clientsocket.h"
#endif
// HACK: console needs map<string,tweaker*> instantiated before map<string,resBitmap_t> (frelement.h) and
// the implicit tweaker_database constructor before ~map<string,resBitmap_t>, and the local static guards
// are numbered in parse order (CIconToolTip::GetInstance $S11, tweaker_database::instance $S13), so both
// headers are parsed here; CIconToolTip::GetCurTipIndex keeps its place in the IL order below.
#include "tooltip.h"
#include "tweaker.h"
#ifdef REBANG_SCOREBOARD_PCH
// HACK: scoreboard's PCH inlines come out as frframe.h, frstatic.h, frform.h, frdesktop.h, frlistbox.h,
// frarea.h after its main pass; the inlines that order shows are defined after frframe.h below
#define REBANG_FR_LATE_PCH_INLINES
#endif
#ifdef REBANG_TASKMAIN_PCH // HACK: the Fresh headers in the order taskmain.obj shows (see taskmain_pch.h)
#include "frbutton.h"
#include "frstatic.h"
#define REBANG_FRFORM_IN_PCH
#include "frform.h"
#include "frtabbutton.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "packet.h"
#include "clientsocket.h"
#include "soundmanager.h"
#endif
#include RB_PCH_INC_FRBUTTON
#include "frwndinl.h"
#ifndef REBANG_RESULTDLG_PCH
#include "frgaugebar.h"
#endif
#include "frlistbox.h"
#include "frstatic.h"
#include "frgraphicinterface.h"
#include "fredit.h"
#include "frarea.h"
#include "fresh.h"
#include "wfont.h"
#include "ucclibrary.h"
#include "hackingmanager.h"
#ifdef REBANG_CLIENTSETTING_PCH
// HACK: FrForm is in the PCH here (tutorial: FrForm::EnableFlag is a PCH inline between
// FrWnd::SetKeyEvent and CInputManager::BlockMouse); frframe.h keeps its later place.
#define REBANG_FRFORM_IN_PCH
#include "frform.h"
#endif
#include "frcombobox.h"
#include "frwndmanager.h"
// HACK: should be inline with class definition, but symbol ordering is tricky
inline FrDesktop* Fresh::GetDesktop() const
{
	return manager->GetDesktop();
}
#ifdef REBANG_CLIENTSETTING_PCH
#include "packet.h"
#include "clientsocket.h"
#else
// HACK: FrForm is in the PCH here (tutorial: FrForm::EnableFlag is a PCH inline between
// FrWnd::SetKeyEvent and CInputManager::BlockMouse); frframe.h keeps its later place.
#define REBANG_FRFORM_IN_PCH
#include "frform.h"
#endif
#ifdef REBANG_GOLFTASK_PCH // golftask.cpp: FrDesktop::SetBgColor is a PCH inline after FrForm::SetCloseResult
#include "frdesktop.h"
#endif
#ifdef REBANG_RESULTDLG_PCH // HACK: after FrForm in the resultdlg PCH (DisableFlag before SetBorder)
#include "frgaugebar.h"
#endif
#include "inputmanager.h"
#include "ccrc32.h"
// HACK (só VC10: no STL novo size() é noexcept e a instanciação explícita não fecha, D734 P8)
#if _MSC_VER == 1600
template unsigned int std::vector<CRC_MT::sCRC_CHKTASK*>::size() const;
#endif
// HACK: CSceneManager::SetFog (used by particle) is a PCH inline between
// FrWndManager::GetGDI and CTask::GetDocument in the original, but scenemanager.h
// comes later in this PCH, so its body lives on a stand-in declared here.
struct CSceneManagerFog
{
#if _MSC_VER > 1600 // A755: VS 2022 - CSceneManager anda no cl 14.44; corpo pelos membros, depois de scenemanager.h
	inline void SetFog(bool bFog);
#else
	void SetFog(bool bFog) { *((bool*)this + 0xb8) = bFog; }
#endif
	bool
	SetFog(); // CSceneManager::ToogleNoOverlapCamera, defined after taskmanager.h
};
#ifdef REBANG_CLIENTSETTING_PCH
// HACK: establish the original singleton class-instantiation order without storage.
class WNetworkSystem;
typedef char
	RbClientsettingNetworkSingleton[sizeof(WSingleton<WNetworkSystem>)];
#endif
#ifdef REBANG_TASKMAIN_PCH // HACK: see taskmain_pch.h
#include "../shared/globalgamedefine.h"
#endif
#include "taskmanager.h"
// HACK: CSceneManager::ToogleNoOverlapCamera (used by realmyroommain) is a PCH inline between
// CAnotherHand::Leave and CIconToolTip::GetCurTipIndex in the original, but scenemanager.h
// comes later in this PCH, so its body lives on the stand-in, as an overload of SetFog (a new
// identifier in the PCH would disturb the name-count canaries: mascotinfodlg, projectg, polysoup).
#if _MSC_VER <= 1600 // A756: no cl novo o corpo vem pelos membros, depois de scenemanager.h
inline bool CSceneManagerFog::SetFog()
{
	return *((bool*)this + 0x129) = !*((bool*)this + 0x129);
}
#endif
inline int CIconToolTip::GetCurTipIndex()
{
	return m_curTipIndex;
}
#include "z_ilfill.h"
// commonutil.h:76 in the original; defined here, next to the IL filler
inline int Ceil(float value, float threshold)
{
	if (value > 0.0f)
	{
		int i = (int)value;
		return (value - (float)i < threshold) ? i : i + 1;
	}
	else
	{
		int i = (int)value;
		return ((float)i - value < threshold) ? i : i - 1;
	}
}
// HACK: inline in the class definition in the original frlistbox.h; defined here, next to the
// IL filler, so that the IL offsets of the other PCH headers do not move
inline FrListItem::FrListItem()
	: pData(NULL), underCursor(false), selected(false)
{
	noRBtnIdxList.clear();
}
inline FrListItem::~FrListItem()
{
	noRBtnIdxList.clear();
}
inline void FrListItem::AddNoRBtnIdx(int idx)
{
	noRBtnIdxList.push_back(idx);
}
ILFILL1
#include "frframe.h"
#ifdef REBANG_FR_LATE_PCH_INLINES
inline const char* FrStatic::GetCaption()
{
	return m_caption.c_str();
}
#endif
inline void FrForm::DescHidePrivacy()
{
	m_pBaseFrm->HidePrivacy(true);
}
inline void FrForm::SetCaptionOffset(bool offset)
{
	m_pBaseFrm->SetCaptionOffset(offset);
}
#ifdef REBANG_FR_LATE_PCH_INLINES
#include "frdesktop.h"
inline void FrListBox::UseRightButton(int mode)
{
	m_useRightButton = mode;
}
inline FrListItem* FrListBox::GetItemUnderCursor()
{
	return m_pItemUnderCursor;
}
inline int FrListBox::GetItemWidth()
{
	return m_itemSize.w;
}
inline int FrListBox::GetItemHeight()
{
	return m_itemSize.h;
}
inline void FrListBox::SetItemSize(int w, int h)
{
	m_itemSize.w = w;
	m_itemSize.h = h;
}
inline const Bitmap* FrArea::GetBgImg()
{
	return m_pBitmap;
}
#endif
#include "fremoticon.h"
// HACK: avatar_avatar emits it right after FrEmoticon::SetAnim (same pattern as Fresh::GetDesktop).
inline bool Fresh::IsVisible() const
{
	return show;
}
#include "frviewer.h"
// HACK: tutorialmain needs FrScrollBar::GetCurTopRow_Int before FrListBox::UseDummy in the PCH IL
// (one line: the line records are part of the PCH IL size; ILFILL1 grew by its cost).
// clang-format off
inline void FrListBox::UseDummy(bool use) { m_useDummy = use; }
// clang-format on
#ifdef REBANG_GOLFRULE_PCH
#include "golfrule_pch_inl.h"
#endif
#include "tweaker.h"
#include "networksystem.h"
#include "networkmonitor.h"
#include "projectg.h"
#include "gdtextbox.h"
#include "gdbitmapbox.h"
#ifndef REBANG_GOLFRULE_PCH // golfrule: after shareddoc.h (std::list<> classes instantiated first)
#include "voiceitem.h"
#include "gdvoiceitem.h"
#endif
#include "../shared/globalgamedefine.h"
#ifdef REBANG_GOLFTASK_PCH // golftask.cpp: taskmanager.h comes after globalgamedefine.h in the original PCH (emission order)
inline bool CTask::IsInited()
{
	return (m_taskFlag & 2) ? true : false;
}
#endif
#include "../shared/globalnetworkdefine.h"
#include "actor.h"
#include "taskmain.h"
#include "nodecontainer.h"
// HACK: IActor::OnProcess is defined here, after nodecontainer.h, so its PCH IL offset follows
// CNodeContainer::GetList (golfbg emits it after GetList in the original)
inline void IActor::OnProcess(float delta)
{
}
#include "scenemanager.h"
#if _MSC_VER > 1600 // A755: 983 00776760 = [this+0xb8] (m_bFog, logo apos m_pWeather/m_pCloud); no cl 14.44 a base anda -4
inline void CSceneManagerFog::SetFog(bool bFog)
{
	*((bool*)&((CSceneManager*)this)->m_pWeather + 8) = bFog;
}
#endif
#if _MSC_VER > 1600 // A756: 983 007ac230 (OnLoad do My Room, apos "[shadow.jpg") alterna [SceneManager+0x11c] (objeto de 0x124);
// o 0x129 do VC10 cai fora do objeto. 0x11c = m_bTransparent (visao completa e reduzida) (cl 14.44 0x100)
inline bool CSceneManagerFog::SetFog()
{
#ifdef REBANG_PCH_SCENEMANAGER
	bool& b = ((CSceneManager*)this)->m_bTransparent;
	return b = !b;
#else
	return ((CSceneManager*)this)->ToggleTransparent();
#endif
}
#endif
#ifndef REBANG_STDAFX_STATICS
#include "courseorder.h"
#endif
// HACK: workaround for unsolved symbol ordering problems
template void std::vector<WVector>::reserve(unsigned int);
// HACK: never defined; PCH name-count canary (polysoup WPolySoup::MakeStatistics), see shareddoc.h RbNamePad
void RbPchNamePad0();
void RbPchNamePad1();
void RbPchNamePad2();
#include "shareddoc.h"
#ifdef REBANG_GOLFRULE_PCH
#include "voiceitem.h"
#include "gdvoiceitem.h"
#endif
#include "uccmanager.h"
#include "frtext.h"
#include "mainframe.h"
#include "wtemplate.h"
// HACK: the original PCH instantiated std::string::iterator::operator* in the template round
// after the basic_string(const_iterator, const_iterator) body (frtext.h StringExtractFindFirst),
// between const_iterator::operator* and const_iterator::operator== (ucclibrary COMDAT order).
// reverse_iterator::operator* is a class member (round 1) that references iterator::operator*.
inline void RbDerefStringIterator()
{
	std::string::reverse_iterator* p = 0;
	**p;
}
// HACK: emission order of privatetradedlg (string::iterator::operator+ and
// operator+= are instantiated by the PCH in the original, after
// const_iterator::operator-; the header that did it is not known. ILFILL3
// pays for this body and RbInstPad below for the two instantiations)
inline std::string::iterator RbStringIteratorAt(std::string& str, int pos)
{
	return str.begin() + pos;
}
// HACK: 10 more PCH template instantiations (a recursive chain, one call of in-place IL):
// projectg (SaveScreenShot), polysoup (MakeStatistics) and frienddlg react to the number of
// PCH-instantiated functions; with ILFILL3 8 increments smaller itemmanager stays in order.
// camerawork added two PCH inline bodies (wmath.inl), so two fewer instantiations here (13 -> 11).
// ghostgamedlg added Ceil and the FrListItem inlines (and their list<int> instantiations): 11 -> 6.
// privatetradedlg added RbStringIteratorAt (iterator::operator+ and operator+=): 6 -> 4.
template <int N>
struct RbInstPad
{
	static void Pad() { RbInstPad<(N > 0 ? N - 1 : 0)>::Pad(); }
};
inline void RbInstPadAll()
{
	RbInstPad<4>::Pad();
}
#if defined(REBANG_CLIENTSETTING_PCH) && _MSC_VER == 1600 // ATL novo já traz os corpos (D736)

// HACK: these ATL bodies belong after the TU template rounds in the original PCH merge.
// Their unchanged C bodies also create the implicit _ATL_WIN_MODULE70 ctor/dtor here.
namespace ATL
{
	inline void CAtlWinModule::Term()
	{
		// HACK: retain the original dependency on the member cleanup wave; no emitted code.
		if (0)
			m_rgWindowClassAtoms.CSimpleArray<ATOM>::~CSimpleArray();
		AtlWinModuleTerm(this, _AtlBaseModule.GetModuleInstance());
	}

	inline CAtlWinModule::CAtlWinModule()
	{
		cbSize = sizeof(_ATL_WIN_MODULE);
		HRESULT hr = AtlWinModuleInit(this);
		if (FAILED(hr))
			CAtlBaseModule::m_bInitFailed = true;
	}

	inline CAtlWinModule::~CAtlWinModule()
	{
		// HACK: defer cleanup until the original constructor wave, without emitted code.
		if (0)
			new (this) CAtlWinModule();
		Term();
	}
}
#endif
#ifdef REBANG_GOLFRULE_PCH
// HACK: the original PCH instantiated Min<float> (golfrule.obj emits it among the PCH templates,
// after the std::vector<WVector> members)
inline void RbGolfRuleMinFloat()
{
	Min<float>(0.0f, 1.0f);
}
#endif
