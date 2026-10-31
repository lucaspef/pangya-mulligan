#pragma once
#include <list>
#include <string>
#include "../../../include/engine/wtypes.h"
#include "frelement.h"
#include "chatmsg.h"

class Bitmap;
class CChatMsg;
class FrCmdTarget;
class FrCursor;
class FrDesktop;
class FrEdit;
class FrEmoticon;
class FrGraphicInterface;
class FrScrollBar;
class FrWnd;
class WSize;

struct FrInputState
{
	unsigned long mouse;
	bool hoverChecked;
	WPoint mousePos;
	WPoint oldMousePos;
	float wheelDelta;
	FrWnd* keyFocused;
	FrScrollBar* wheelFocused;
	CChatMsg* im;
};

class FrWndManager
{
public:
	enum eFadeState
	{
		OPENING = 0x0,
		OPENED = 0x1,
	};

	FrWndManager(FrElementDoc* pDoc);
	virtual ~FrWndManager();

	bool Init(const char* wallPaper, bool exclusiveKey);
	void Display(bool drawDesktop);
	void Process(const float deltaTime);
	FrElementDoc* GetDocument() const;
	const Bitmap* GetBitmap(const char* resource, const char* id) const;
	void CreateToolTip();
	void DestroyToolTip();
	bool CreateLayout(const char* layout, FrCmdTarget* owner, bool firstTime);
	void CloseLayout();
	bool IsValidWindow(FrWnd* pWnd);
	bool AddTopmostWindow(FrWnd* pWnd);
	bool DeleteTopmostWindow(FrWnd* pWnd);
	FrWnd* GetModalForm();
	FrWnd* DoCreate(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent,
		FrCmdTarget* pOwner);
	__forceinline FrDesktop* GetDesktop() { return this->m_pDesktop; }
	void DisplayFlag4000_983(); // 983: 00be2840 (m_pDesktop->DisplayFlag4000_983)
	unsigned int& RefIndex() { return m_nRefIndex; }
	FrGraphicInterface* GetGDI() { return m_pDevice; }
	// D865: campos 0xa9/0xaa/0xac do 983 que as unidades jp/ liam por offset cru do VC10 (g_pFresh+0x40 -> +0xa9..)
	__forceinline bool& RFMouseFocus983() { return m_bNoInput983; }
	__forceinline bool& RFKeyFocus983() { return m_bRFKeyFocus983; }
	__forceinline int GetFormCount983() const { return m_formCount983; }
	__forceinline FrGraphicInterface* GetGDI983() { return m_pDevice; } // GetGDI sai fora de linha no VC10
	const WPoint& GetMousePos() const { return m_istate.mousePos; }
	FrEmoticon* GetEmoticon() { return m_pEmoticon; }
	FrWnd* HandOverViewFocus();
	void DisplayImeCandidate983(); // 983: função nova (00be39e0, 1.319 B): desenha a lista de candidatos do IME; chamada no fim do Display. CORPO PENDENTE
	int CountForms983(); // 983: 983: função nova (00be4730, 205 B), chamada pelo FrWnd::OnProcess depois de destruir um filho; nome provisório
	FrWnd* FindWndAt983(float x, float y); // 983: 00be48c0 (janela debaixo do ponto; pedido B137.1; corpo na B por enquanto)
	bool SetKeyFocus(FrWnd* pWnd, bool resetPrevImeData);
	FrWnd* GetKeyFocused() { return m_istate.keyFocused; }
	CChatMsg* GetIme983() { return m_istate.im; } // 983 (FrEdit::OnDraw 00bfa554: [mgr+0x8c]+0x34); nome meu
	bool CanGetKeyFocus(FrWnd* pWnd);
	bool ResetKeyFocus(FrWnd* pWnd);
	bool MoveKeyFocusToNext(bool resetPrevImeData);
	FrScrollBar* GetWheelFocus() { return m_istate.wheelFocused; }
	void SetWheelFocus(FrScrollBar* pScrBar);
	void SetCursor(int hCursor) { m_cursorIndex = hCursor; }
	int GetCursor() { return m_cursorIndex; }
	void MoveCursor(FrWnd* pParent, const char* name);
	__forceinline const char* GetLayoutID() { return this->m_layoutID.c_str(); }
	bool SetCapture(FrWnd* pWnd);
	FrWnd* GetCapture() { return m_pCaptured; }
	bool ReleaseCapture(FrWnd* pWnd);
	WPoint GetCreatePosition(const WSize& rectSize);
	void ConfineRect(WRect& dr);
	void RemoveFade();
	void CloseWindow(FrWnd* pWnd, bool bFade);
	void CloseForm(bool bFade);
	bool HasEscKeyWindow();
	bool HasValidWindow();
	void SetExclusiveKey(bool set);
	bool IsKeyExclusive() { return m_exclusiveKey; }
	void ResetKey()
	{
		if (m_istate.im)
			m_istate.im->Reset();
	}
	void ResetTopFocus();
	float GetTextWidth(const char* text);
	float PrintText(const WPoint& pos, unsigned long align, const char* text,
		float limit, unsigned long emoDiffuse);
	float PrintText11(const WPoint& pos, unsigned long align, const char* text,
		float limit, unsigned long emoDiffuse);
	FrInputState* GetInputState();
	void IME_ShowCandWindow(FrEdit* pFocusedEdit, WPoint& caretPos);
	std::list<FrWnd*>& GetTopWndList() { return m_topmostList; }
	__forceinline void HidePrivacy(bool hide) { this->m_hidePrivacy = hide; }
	__forceinline bool HidePrivacy() { return this->m_hidePrivacy; }

protected:
	void CheckSystemStatus();
	void ProcessHotKey();
	void ProcessKey(FrWnd* pOldKeyFocused);

	FrElementDoc* m_pDoc;
	FrDesktop* m_pDesktop;
	FrCursor* m_pCursor;
	FrInputState m_istate;
	bool m_exclusiveKey;
	std::list<FrWnd*> m_topmostList;
	std::string m_layoutID;
	std::string m_prevLayoutID983; // 983: último layout fechado (CloseLayout 00be50f0); nome meu. Antes: 983: 2ª string (construtor 00be4e06..0c; nome provisório)
	FrWnd* m_pCaptured;
	FrGraphicInterface* m_pDevice;
	unsigned int m_nRefIndex;
	int m_cursorIndex;
	FrEmoticon* m_pEmoticon;
	float m_fadeoutAlpha;
	eFadeState m_state;
	FrEdit* m_pFocusedEdit;
	WPoint m_caretPos;
	bool m_hidePrivacy;
	bool m_bNoInput983; // 983: 0xa9, CheckSystemStatus (00be3600) sai sem ler a entrada quando ligado; nome meu. Antes: 983: 2 bools e 1 dword novos, zerados no construtor (nomes provisórios)
	bool m_bRFKeyFocus983;
	int m_formCount983; // 983: 0xac, contado por CountForms983 (00be4730); nome meu
};

// D865: campos do 983 que as unidades jp/ liam por offset cru do VC10 (g_pFresh->manager +0x84/+0xa9/+0xaa/+0xac).
// No VC10 fica o cru (o build nao embute os acessores: sairia call); no cl novo o membro (o STL antes anda o layout).
#if defined(_MSC_VER) && _MSC_VER == 1600
#define FRMGR_GDI983(m) (*(FrGraphicInterface**)((char*)(m) + 0x84))
#define FRMGR_RFMOUSE983(m) (*(bool*)((char*)(m) + 0xa9))
#define FRMGR_RFKEY983(m) (*(bool*)((char*)(m) + 0xaa))
#define FRMGR_FORMCOUNT983(m) (*(int*)((char*)(m) + 0xac))
#else
#define FRMGR_GDI983(m) ((m)->GetGDI983())
#define FRMGR_RFMOUSE983(m) ((m)->RFMouseFocus983())
#define FRMGR_RFKEY983(m) ((m)->RFKeyFocus983())
#define FRMGR_FORMCOUNT983(m) ((m)->GetFormCount983())
#endif
