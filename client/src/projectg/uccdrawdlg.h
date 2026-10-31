#pragma once

#include "frform.h"
#include "uccbrush.h"
#include "tooltipmanager.h"
#include "tabmanager.h"
#include "overlaycursor.h"
#include "fadingtext.h"

class FrArea;
class FrButton;
class FrListBox;
struct sItemInfo;
class CUccClothes;
class FrNotifyForm;
class FrUccSetInfoDlg;
class FrUccPickColorDlg;

enum eDrawMode
{
	DRAW_CLOTHES,
	DRAW_AZTEC
};

enum eUccTab
{
	UCCTAB_DRAW,
	UCCTAB_PATTERN
};

class FrUccDrawDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrUccDrawDlg)

	FrUccDrawDlg();
	virtual ~FrUccDrawDlg();

	void Init();
	void Clear();
	void SetMyRoom983(void* p) { m_pMyRoom983 = (int)p; } // 983 00620e00 grava +0x8b0
	virtual bool Close(eFormRet result, bool bFade);
	virtual bool Open(FRESH_PFN_RESULT pFnResult, const WPoint& pos,
		unsigned long flag);

	void SetName(const char* name);
	bool IsInitialized() { return m_bInit; }
	sItemInfo* GetItemInfo983() { return m_pItemInfo; } // 983 00605e70 le +0x110
	void Upload();
	void Notify(const char* text);
	void SetCurrentColor(unsigned char r, unsigned char g, unsigned char b);
	void SetSaved(bool bSaved);
	bool IsSaved() { return m_bSaved; }
	void AfterZoom();
	void MoveCanvasToCenter();
	void BuildColorList();
	void CloseWaitingForm();
	void SetToolbarStatus(FrButton* pButton);
	void BuildBrushList();
	void BuildPenList();
	void BuildEraserList();
	void BuildPatternList(int index, bool bSelect);
	void BuildCurrentPattern();
	void ChangeTab(eUccTab tab);
	void SetUccDrawInfo(eDrawMode mode, sItemInfo* pItemInfo, bool bReset);
	void Finish();

protected:
	// HACK: literal anchor of OnProc (see uccdrawdlg.cpp)
	friend unsigned char Min<unsigned char>(unsigned char a, unsigned char b);
	virtual void OnProc(const float delta);
	virtual void OnDraw();
	virtual void OnCancel();

	bool IsSavingTemporarily();
	void Reset();

	void OnUccCanvasInit(int param);
	void OnUndoBtnInit(int param);
	void OnUndoBtnUp();
	void OnRedoBtnInit(int param);
	void OnRedoBtnUp();
	void OnFlipBtnInit(int param);
	void OnFlipBtnOwnerDraw(int param);
	void OnFlipBtnUp();
	void OnResetBtnInit(int param);
	void OnResetBtnUp();
	void OnSide1AreaInit(int param);
	void OnSide2AreaInit(int param);
	void OnTempSaveBtnInit(int param);
	void OnTempSaveBtnUp();
	void OnGridBtnInit(int param);
	void OnGridBtnUp();
	void OnDragBtnInit(int param);
	void OnDragBtnUp();
	void OnZoomInBtnInit(int param);
	void OnZoomInBtnUp();
	void OnZoomOutBtnInit(int param);
	void OnZoomOutBtnUp();
	void OnTabDrawBtnInit(int param);
	void OnTabDrawBtnUp();
	void OnTabPatternBtnInit(int param);
	void OnTabPatternBtnUp();
	void OnTabEmoticonBtnInit(int param);
	void OnTabEmoticonBtnUp();
	void OnTabTextBtnInit(int param);
	void OnTabTextBtnUp();
	void OnFinishBtnInit(int param);
	void OnFinishBtnUp();
	void OnCloseBtnInit(int param);
	void OnCloseBtnUp();
	void OnPaintBtnInit(int param);
	void OnPaintBtnUp();
	void OnPickColorBtnInit(int param);
	void OnPickColorBtnUp();
	void OnCurrentColorBtnInit(int param);
	void OnCurrentColorBtnUp();
	void OnHotkeyDescBtnInit(int param);
	void OnHotkeyDescBtnUp983(); // 007e58b0
	bool OnDescImgDlgResult983(int result, FrForm* pForm); // 007e2270
	void OnCurrentColorAreaInit(int param);
	void OnColorListInit(int param);
	void OnColorListOwnerDraw(int param);
	void OnColorListBtnDown();
	void OnEraserListInit(int param);
	void OnEraserListOwnerDraw(int param);
	void OnEraserListBtnDown();
	void OnToolInfoEraserAreaInit(int param);
	void OnBrushListInit(int param);
	void OnBrushListOwnerDraw(int param);
	void OnBrushListBtnDown();
	void OnPenListInit(int param);
	void OnPenListOwnerDraw(int param);
	void OnPenListBtnDown();
	void OnToolInfoPenAreaInit(int param);
	void OnToolInfoBrushAreaInit(int param);
	void OnColorListsAreaInit(int param);
	void OnArrowDownBtnInit(int param);
	void OnArrowDownBtnUp();
	void OnArrowRightBtnInit(int param);
	void OnArrowRightBtnUp();
	void OnArrowLeftBtnInit(int param);
	void OnArrowLeftBtnUp();
	void OnDeleteBtnInit(int param);
	void OnDeleteBtnUp();
	void OnModifyBtnInit(int param);
	void OnModifyBtnUp();
	void OnMakeBtnInit(int param);
	void OnMakeBtnUp();
	void OnRateListInit(int param);
	void OnRateListOwnerDraw(int param);
	void OnRateListBtnDown();
	void OnPatternListInit(int param);
	void OnPatternListOwnerDraw(int param);
	void OnPatternListBtnDown();
	void OnBaseColorListInit(int param);
	void OnExtendedBasePatternListInit(int param);
	void OnEtcInit(int param);
	void OnEtcOwnerDraw(int param);
	void OnToolTipInit(int param);
	void OnToolTipOnwerDraw(int param);

	bool OnUccSetInfoDlgResult(int result, FrForm* pForm);
	bool OnUccPickColorDlgResult(int result, FrForm* pForm);
	bool OnAskCompleteDlgResult(int result, FrForm* pForm);
	bool OnAskCancelDlgResult(int result, FrForm* pForm);
	bool OnAskResetDlgResult(int result, FrForm* pForm);

public:
	sItemInfo* m_pItemInfo;
	CUccClothes* m_pClothes[2];
	bool m_bWaiting;
	FrForm* m_pWaitingForm;
	float m_waitingTime;
	Bitmap m_color[18];

protected:
	CToolTipManager m_toolTip;
	CTabManager m_tab;
	bool m_bSaved;
	bool m_bCloseAfterSave;
	int m_prevCursorBind983; // 983: o construtor guarda aqui [g+0x1e8] (007e5492); o destrutor compara com 0
	FrArea* m_pCanvas;
	FrButton* m_pUndoBtn;
	FrButton* m_pRedoBtn;
	FrButton* m_pFlipBtn;
	FrButton* m_pResetBtn;
	FrArea* m_pSide1Area;
	FrArea* m_pSide2Area;
	FrButton* m_pTempSaveBtn;
	FrButton* m_pHotkeyDescBtn; // 983: +0x2ec, hotkey_desc_btn (INIT 007e3670)
	FrButton* m_pGridBtn;
	FrButton* m_pDragBtn;
	FrButton* m_pZoomInBtn;
	FrButton* m_pZoomOutBtn;
	FrButton* m_pTabDrawBtn;
	FrButton* m_pTabPatternBtn;
	FrButton* m_pTabEmoticonBtn;
	FrButton* m_pTabTextBtn;
	FrButton* m_pFinishBtn;
	FrButton* m_pCloseBtn;
	FrButton* m_pPickColorBtn;
	FrButton* m_pCurrentColorBtn;
	FrButton* m_pPaintBtn;
	FrArea* m_pCurrentColorArea;
	FrListBox* m_pColorList;
	FrListBox* m_pEraserList;
	FrArea* m_pToolInfoEraserArea;
	Bitmap m_currentColor;
	FrListBox* m_pBrushList;
	FrListBox* m_pPenList;
	FrArea* m_pToolInfoPenArea;
	FrArea* m_pToolInfoBrushArea;
	FrArea* m_pColorListsArea;
	FrButton* m_pArrowDownBtn;
	FrButton* m_pArrowRightBtn;
	FrButton* m_pArrowLeftBtn;
	FrButton* m_pDeleteBtn;
	FrButton* m_pModifyBtn;
	FrButton* m_pMakeBtn;
	FrListBox* m_pRateList;
	FrListBox* m_pPatternList;
	FrArea* m_pBaseColorList;
	FrArea* m_pExtendedBasePatternList;
	FrArea* m_pEtc;
	FrArea* m_pToolTip;
	int m_unused38c983; // 983 0x38c: so zerado no Init (007e26ee), nenhum leitor no exe; nome meu
	FrUccSetInfoDlg* m_pSetInfoDlg;
	FrUccPickColorDlg* m_pPickColorDlg;
	FrNotifyForm* m_pAskCancelDlg;
	FrForm* m_pAskResetDlg;
	FrForm* m_pAskCompleteDlg;
	FrForm* m_pDescImgDlg; // 983: 0x3a4, form "selfdesigndescimg" (007e58b0); nome meu
	const Bitmap* m_pToolBox;
	const Bitmap* m_pSelectBox;
	const Bitmap* m_pSelectSBox2;
	const Bitmap* m_pBgText[2][2];
	const Bitmap* m_pToolChangeIcon;
	const Bitmap* m_pSelectBoxYellow;
	const Bitmap* m_pSelectBoxPink;
	const Bitmap* m_pSelectColor;
	int m_unused3d4983; // 983 0x3d4: sem acesso no exe (Init 007e2690 pula de 0x3d0 a 0x3d8); nome meu
	int m_curEraser;
	int m_curTool;
	int m_curSide;
	eDrawMode m_drawMode;
	char m_name[64];
	WRect m_canvasRect;
	Bitmap m_pattern[24];
	eUccTab m_curTab;
	Bitmap m_patternDefault;
	Bitmap m_patternEmpty;
	int m_curPattern;
	int m_rate;
	bool m_bArrowDown;
	bool m_bGrid;
	Bitmap m_unusedBitmap650983; // 983 0x650: so construido/destruido (ctor 007e544e, dtor 007e564b); nome meu
	bool m_bInit;
	COverlayCursor m_cursor;
	CFadingText m_fadingText;
	// 983: ultimo indice escolhido por ferramenta (BtnDown 007e6980/007e69e0/007e68e0; atalhos BRUSH/PEN/ERASER
	// no OnProc 007e6cf1; OwnerDraw 007e3f10/007e3fa0 desenha a moldura so no item desse indice). Init: 0, -1, -1.
	int m_lastBrushIdx983;
	int m_lastPenIdx983;
	int m_lastEraserIdx983;
	int m_pMyRoom983;

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbUploadFunclets(); // HACK: EH funclets of the naked Upload (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSetuccdrawinfoFunclets(); // HACK: EH funclets of the naked SetUccDrawInfo (nakedify)
#endif
};
