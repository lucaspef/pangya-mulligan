#pragma once
#include "wflag.h"
#include "rtti.h"
#include "frcmdtarget.h"
#include "objectfactory.h"
#include "../../../include/engine/wtypes.h"

struct sFRESH_MSGMAP;

class FrWndManager;
class FrWnd;
class CChatMsg;
struct FrInputState;
class FrScrollBar;
class FrGraphicInterface;
class FrEmoticon;
class FrToolTip;

enum eFrFlags
{
	FRWF_NONE = 0x0, // 645 FWF_NONE: o SDK 10 (shlobj FOLDERFLAGS) tem o mesmo nome (D734 P7); sem uso
	FWF_TOOLTIPS = 0x1,
	FWF_VIEWFOCUS = 0x8,
	FWF_KEYFOCUS = 0x10,
	FWF_DESTROY = 0x20,
	FWF_FADEOUT = 0x40,
	FWF_INITED = 0x80,
	FWF_FADING = 0x100,
	FWF_TOPFOCUS = 0x200,
	FWF_FADING_EX = 0x400,
	FWF_FADEOUT_EX = 0x800,
	FWF_PRIVACY = 0x1000,
	FWF_FADEELEMENT = 0x2000,
	FWF_UNK4000_983 = 0x4000 // 983: testado em FrWnd::OnDisplay (00be8400) quando o 4º argumento é true; nome provisório
};

enum eFrStyle
{
	FWS_NONE = 0x0,
	FWS_VISIBLE = 0x1,
	FWS_DISABLED = 0x2,
	FWS_HASTITLE = 0x4,
	FWS_CHILD = 0x8,
	FWS_TOPMOST = 0x10,
	FWS_MOVEFRAME = 0x20,
	FWS_NOMOUSEEVENT = 0x40,
	FWS_KEYEVENT = 0x80,
	FWS_FIXED = 0x100,
	FWS_NODBLCLICK = 0x200,
	FWS_HOVER = 0x400,
	FWS_NOWHEELEVENT = 0x800
};

// 983: base nova do FrWnd (RTTI .?AVFrModalRenderer@@, vtable 00d5e5e4, 1 slot = destrutor 00be7e90; 0x14 bytes).
// Nomes dos campos são provisórios.
class FrModalRenderer
{
public:
	FrModalRenderer() { ResetModal(); }
	virtual ~FrModalRenderer() { ResetModal(); }
	void ResetModal()
	{
		m_modalPos = WPoint(0, 0);
		m_modalEmoticon983 = 0;
		m_modalOn = false;
	}

	WPoint m_modalPos;
	int m_modalEmoticon983; // 983 +0x14: FrEmoticon do gerenciador (fim de _Init 00be6cab); 00be7cf0 so testa != 0
	bool m_modalOn;
};

class FrWnd : public IObject, public FrCmdTarget, public FrModalRenderer
{
	// Not 100% sure if true.
	friend class Fresh;
	friend class FrWndManager;
	friend IObject* FrWndMakeInstance();

public:
	struct sToolTipData
	{
		int bFixWnd;
		WPoint posFixWnd;
		unsigned long style;
		std::string text; // 983: o texto da dica mora aqui (new de 0x2c em 00be9056)
	};

	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }
	virtual ~FrWnd();
	void DestroyChild();
	virtual bool Close(bool bFade);
	virtual bool Create(const char* lpszWindowText, const char* lpszWindowName,
		FrWndManager* pManager, unsigned long dwStyle, const WRect& rect,
		FrWnd* pParentWnd);
	virtual void PreCreateWindow(FrWndManager* pManager, unsigned long dwStyle,
		const WRect& rect, FrWnd* pParentWnd);
	bool SendCmdToOwnerTarget(FrCmdTarget* pCmdTarget, eFrCmd cmd, int var1,
		sFRESH_HANDLER* pHandler);
	bool SendCmdToOwnerTarget(eFrCmd cmd, int var1, sFRESH_HANDLER* pHandler);
	void SetOwner(FrCmdTarget* pOwner);
	void AddChild(FrWnd* pChild);
	FrWndManager* WndManager() const { return m_pWndManager; }
	FrGraphicInterface* GDI() const;
	FrEmoticon* Emo() const;
	FrWnd* GetParent() const;
	FrWnd* FindChild(const char* lpszWindowText);
	FrWnd* FindChild(const FrWnd* pWnd);
	FrWnd* FindChildByName(const char* lpszWindowName);
	FrWnd* FindChildByStyle(unsigned long style);
	FrWnd* FindChildForm(const char* lpszWindowName);
	bool IsChild(const FrWnd* pWnd) const;
	void CloseChild(FrWnd* pChildWnd, bool bFade, bool force);
	void CloseChildForm(bool bFade, bool force);
	void RemoveWindow(FrWnd* pWnd);
	void EnumerateChildWindow(bool (*callback)(FrWnd*, void*), void* parm);
	void GetClientRect(WRect& rect) const;
	void SetClientRect(const WRect& rect);
	void ClientToScreen(WPoint& point) const;
	void ClientToScreen(WRect& rect) const;
	void ScreenToClient(WPoint& point) const;
	void ScreenToClient(WRect& rect) const;
	virtual void MoveWindow(const WPoint& point);
	void SetRect(const WRect& rect);
	const WRect& GetRect() { return m_rect; }
	void SetWindowTextA(const char* lpszText);
	void GetWindowTextA(std::string& outText) const;
	void GetWindowTextA(char* lpszTextBuf, unsigned int nBuffMax) const;
	unsigned int GetWindowTextLengthA() const;
	void SetWindowName(const char* lpszName);
	void GetWindowName(std::string& outName) const;
	void GetWindowName(char* lpszNameBuf, unsigned int nBuffMax) const;
	unsigned int GetWindowNameLength() const;
	void EnableToolTip(bool enable);
	bool IsEnableToolTip() const;
	void SetToolTipText(const std::string& text);
	const std::string& GetWindowName983() const; // 983: 00be76b0 (fora de linha; COption 00494bb0)
	__forceinline int GetGuiType983() const { return m_guiType983; } // D871: o 983 le +0xc8 inline (ClubSetWorkShopForm 004f2e60); cru andava no cl novo
	const std::string* GetToolTipText() const; // 983: 00be76c0 devolve ponteiro (NULL sem dica)
	void SetFixTooltipWnd(const WPoint& pos);
	void SetUnFixToolTipWnd();
	int IsFixedToolTip() const;
	const WPoint& GetToolTipWndPos() const;
	void SetToolTipFrameStyle(unsigned long style);
	unsigned long GetToolTipFrameStyle() const;
	// 983: 4 argumentos (ret 0x10 em 00be8400); o 4º faz pular a janela com FWF_UNK4000_983
	void OnDisplay(bool checkTopmost, bool drawChild, bool drawOneself, bool checkFlag4000);
	void DisplayFlag4000_983(); // 983: 00be84d0 (filhos com FWF_UNK4000_983, depois da RF; nome meu)
	void OnProcess(const float deltaTime, FrInputState& istate,
		bool checkTopmost, bool bUnableMode);

	WFlags m_nFlags;

	const WFlags& GetStyle() const { return m_dwStyle; }
	float GetAlpha() const;
	void SetAlpha(float alpha);
	float GetAlpha2() const;
	void SetAlpha2(float alpha);
	void SetAlpha2ToChild(float a);
	virtual void Enable(bool enable) { m_dwStyle.Turn(FWS_DISABLED, !enable); }
	bool IsEnabled() const { return !m_dwStyle.GetFlag(FWS_DISABLED); }
	virtual void SetVisible(bool visible);
	bool IsVisible() const { return m_dwStyle.GetFlag(FWS_VISIBLE); }
	void SetKeyEvent(bool enable) { m_dwStyle.Turn(FWS_KEYEVENT, enable); }
	void SetTopmost(bool topmost);
#ifdef REBANG_TASKMAIN_PCH // HACK: taskmain_pch.h (in the class in the original; emission order)
	void SetFixed(bool fixed) { m_dwStyle.Turn(FWS_FIXED, fixed); }
#else
	void SetFixed(bool fixed);
#endif
	bool IsFixed() { return m_dwStyle.GetFlag(FWS_FIXED); }
	void UseDblClick(bool use) { m_dwStyle.Turn(FWS_NODBLCLICK, !use); }
	void EnableHover(bool enable, float time);
	void SetWheelEvent(bool enable);
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	void HidePrivacy(bool hide);
#else
	void HidePrivacy(bool hide) { m_nFlags.Turn(FWF_PRIVACY, hide); }
#endif
	void SetFadeout(bool bOut);
	void SetElementFadeOut(bool bOut);
	void SetViewFocus(bool takeFromOthers);
	void ResetViewFocus(FrWnd* pWndStop);
	FrWnd* FindViewFocused(bool enabled_visible);
	bool IsViewFocused() const;
	bool IsTopFocus() const;
	bool SetKeyFocus(bool resetPrevImeData);
	bool HasKeyFocus();
	bool ResetKeyFocus();
	void FindNextTopFocus(bool bForce);
	bool SetCapture();
	FrWnd* GetCapture();
	bool ReleaseCapture();
	void SetCursor(int hCursor);
	int GetCursor() const;
	void MoveCursor(const char* name);
	void SetPushSound(const char* name_const);
	bool PlayPushSound();
	FrScrollBar* GetScrollBar();
	void SetWheelFocus();

protected:
	FrWnd(FrWnd&);
	FrWnd();
	virtual void OnDraw();
	virtual void OnProc(const float deltaTime) { }
#ifdef REBANG_TASKMAIN_PCH // HACK: taskmain_pch.h (taskmain.obj has its own copy of this body)
	virtual void OnResize() { }
#else
	virtual void OnResize();
#endif
	virtual void OnMouseMove(const WPoint& mousePos) { }
	virtual bool OnLButtonUp(const WPoint& mousePos) { return false; }
	virtual bool OnLButtonDown(const WPoint& mousePos) { return false; }
#ifdef REBANG_TASKMAIN_PCH // HACK: taskmain_pch.h (taskmain.obj has its own copy of this body)
	virtual bool OnRButtonUp(const WPoint& point) { return false; }
#else
	virtual bool OnRButtonUp(const WPoint& point);
#endif
#ifdef REBANG_TASKMAIN_PCH // HACK: taskmain_pch.h (taskmain.obj has its own copy of this body)
	virtual bool OnRButtonDown(const WPoint& point) { return false; }
#else
	virtual bool OnRButtonDown(const WPoint& point);
#endif
#ifdef REBANG_TASKMAIN_PCH // HACK: taskmain_pch.h (taskmain.obj has its own copy of this body)
	virtual void OnDblClick(const WPoint& point) { }
#else
	virtual void OnDblClick(const WPoint& point);
#endif
	virtual void OnWheel(FrInputState& input) { }
	virtual const char* OnSelectText(const FrInputState* input) { return NULL; }
#ifdef REBANG_TASKMAIN_PCH // HACK: taskmain_pch.h (taskmain.obj has its own copy of this body)
	virtual void OnKeyFocus(CChatMsg* message) { }
#else
	virtual void OnKeyFocus(CChatMsg* message);
#endif
	virtual void OnSetCursor(bool bInClient, const WPoint& mousePos) { }
	virtual void EnableKeyFocus(FrInputState& input) { m_nFlags.Enable(0x10); }
	virtual void SetIconRect(const WRect& rect) { }

private:
	virtual void DoFadeDisplay(); // 983: virtual (slot 23 da vtable 00d5e604) e vazia (00be77b0)
	void DoFadeProcess(const float deltaTime, bool bExtend);
	void CheckHover(float deltaTime, bool bInClient, bool& hoverChecked);

protected:
	FrWnd* m_pParentWnd;
	FrCmdTarget* m_pOwner;
	std::list<FrWnd*> m_childList;
	WRect m_rect;
	WFlags m_dwStyle;
	std::string m_wndText;
	std::string m_wndName;
	float m_wndAlpha;
	float m_wndAlpha2;
	unsigned int m_nRefID;
	FrWndManager* m_pWndManager;
	FrScrollBar* m_pScrBar;
	float m_fadeTime;
	WRect m_iconRect;
	const char* m_szPushSound;
	float m_dblClickTimeout;
	bool m_dblClicked;
	WPoint m_dblClickPos;
	bool m_hoverOn;
	float m_hoverTime;
	float m_accHoverTime;

private:
	int m_guiType983; // 983: tipo do item (FrGuiItem::m_type), gravado por FrWndManager::DoCreate (00be3710); nome meu. Antes: 983: 4 bytes zerados no construtor no lugar da std::string m_wndToolTip (nome provisório)
	static FrToolTip* m_pToolTip;
	sToolTipData* m_pToolTipData;
	static FrToolTip* ToolTip();
	static bool IsToolTipInstantiated() { return m_pToolTip != NULL; }
	static void CreateToolTip(FrWndManager* pManager);
	static void DestroyToolTip();
};
