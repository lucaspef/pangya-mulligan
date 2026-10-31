#include "minatl.h"
#include "../shared/globalgamedefine.h"
#include "uccbase.h"
#include "uccclothes.h"
#include "uccdrawdlg.h"
#include "jp/selfdesignhotkeydescimgdlg.h" // 983 007e4f30 (botao hotkey_desc_btn)
#include "realmyroommain.h"
#include "securitykeymanager.h"
#include "frgraphicinterface.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "frarea.h"
#include "notifyform.h"
#include "uccsetinfodlg.h"
#include "uccpickcolordlg.h"
#include "taskmanager.h"
#include "netresourcemanager.h"
#include "inputmanager.h"
#include "commonutil.h"
#include "uccmanager.h"
#include "projectg.h"
#include "mathconsts.h"
#include "clientsetting.h"
#include "mousecursor.h"


extern Fresh* g_pFresh;
extern WInputDev* g_mouse; // 983 00f02450
extern HWND g_hwnd; // 00f0245c

void GetResizedImage(const Bitmap* src, const Bitmap* mask, Bitmap* dst,
	int width, int height);
std::string GetMaskFilename(const char* filename);
void Convert32to24(Bitmap* src, Bitmap* dst, const Bitmap* mask);
const char* GetUccTempFoler();
bool ZipUccClothes(Bitmap* texture, Bitmap* mask, Bitmap* icon,
	const char* filename);
void G_CreateDirectory(const char* path);
std::string GenerateTextureFullPath(int side, unsigned long typeId,
	const char* uccIndex);
std::string GenerateIconFilename(unsigned long typeId, const char* uccIndex);

// HACK: IL filler, see z_ilfill.h. The PCH's WRect constructors must be compiled before WRect::operator-.
// clang-format off
namespace TUPAD_UCCDRAWDLG
{
	struct Pad
	{
		int v;
		void g()
		{
			ILFILL256 ILFILL256 ILFILL256 ILFILL256
		}
	};
}
// clang-format on

inline WRect WRect::operator-(WPoint& point) const
{
	return WRect(x - point.x, y - point.y, w, h);
}

IMPLEMENT_OBJECT(FrUccDrawDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrUccDrawDlg, FrForm)

ON_FRESH_VI("hotkey_desc_btn", FRCMD_INIT, FrUccDrawDlg::OnHotkeyDescBtnInit)
ON_FRESH_VV("hotkey_desc_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnHotkeyDescBtnUp983)
ON_FRESH_VI("ucccanvas", FRCMD_INIT, FrUccDrawDlg::OnUccCanvasInit)
ON_FRESH_VI("undo_btn", FRCMD_INIT, FrUccDrawDlg::OnUndoBtnInit)
ON_FRESH_VV("undo_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnUndoBtnUp)
ON_FRESH_VI("redo_btn", FRCMD_INIT, FrUccDrawDlg::OnRedoBtnInit)
ON_FRESH_VV("redo_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnRedoBtnUp)
ON_FRESH_VI("flip_btn", FRCMD_INIT, FrUccDrawDlg::OnFlipBtnInit)
ON_FRESH_VI("flip_btn", FRCMD_OWNERDRAW, FrUccDrawDlg::OnFlipBtnOwnerDraw)
ON_FRESH_VV("flip_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnFlipBtnUp)
ON_FRESH_VI("reset_btn", FRCMD_INIT, FrUccDrawDlg::OnResetBtnInit)
ON_FRESH_VV("reset_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnResetBtnUp)
ON_FRESH_VI("side1_area", FRCMD_INIT, FrUccDrawDlg::OnSide1AreaInit)
ON_FRESH_VI("side2_area", FRCMD_INIT, FrUccDrawDlg::OnSide2AreaInit)
ON_FRESH_VI("temp_save_btn", FRCMD_INIT, FrUccDrawDlg::OnTempSaveBtnInit)
ON_FRESH_VV("temp_save_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnTempSaveBtnUp)
ON_FRESH_VI("grid_btn", FRCMD_INIT, FrUccDrawDlg::OnGridBtnInit)
ON_FRESH_VV("grid_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnGridBtnUp)
ON_FRESH_VI("drag_btn", FRCMD_INIT, FrUccDrawDlg::OnDragBtnInit)
ON_FRESH_VV("drag_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnDragBtnUp)
ON_FRESH_VI("zoomin_btn", FRCMD_INIT, FrUccDrawDlg::OnZoomInBtnInit)
ON_FRESH_VV("zoomin_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnZoomInBtnUp)
ON_FRESH_VI("zoomout_btn", FRCMD_INIT, FrUccDrawDlg::OnZoomOutBtnInit)
ON_FRESH_VV("zoomout_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnZoomOutBtnUp)
ON_FRESH_VI("draw_tab_btn", FRCMD_INIT, FrUccDrawDlg::OnTabDrawBtnInit)
ON_FRESH_VV("draw_tab_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnTabDrawBtnUp)
ON_FRESH_VI("pattern_tab_btn", FRCMD_INIT, FrUccDrawDlg::OnTabPatternBtnInit)
ON_FRESH_VV("pattern_tab_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnTabPatternBtnUp)
ON_FRESH_VI("emoticon_tab_btn", FRCMD_INIT, FrUccDrawDlg::OnTabEmoticonBtnInit)
ON_FRESH_VV("emoticon_tab_btn", FRCMD_LBUTTONUP,
	FrUccDrawDlg::OnTabEmoticonBtnUp)
ON_FRESH_VI("text_tab_btn", FRCMD_INIT, FrUccDrawDlg::OnTabTextBtnInit)
ON_FRESH_VV("text_tab_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnTabTextBtnUp)
ON_FRESH_VI("finish_btn", FRCMD_INIT, FrUccDrawDlg::OnFinishBtnInit)
ON_FRESH_VV("finish_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnFinishBtnUp)
ON_FRESH_VI("close_btn", FRCMD_INIT, FrUccDrawDlg::OnCloseBtnInit)
ON_FRESH_VV("close_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnCloseBtnUp)
ON_FRESH_VI("paint_btn", FRCMD_INIT, FrUccDrawDlg::OnPaintBtnInit)
ON_FRESH_VV("paint_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnPaintBtnUp)
ON_FRESH_VI("pick_color_btn", FRCMD_INIT, FrUccDrawDlg::OnPickColorBtnInit)
ON_FRESH_VV("pick_color_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnPickColorBtnUp)
ON_FRESH_VI("cur_color_btn", FRCMD_INIT, FrUccDrawDlg::OnCurrentColorBtnInit)
ON_FRESH_VV("cur_color_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnCurrentColorBtnUp)
ON_FRESH_VI("cur_color_area", FRCMD_INIT, FrUccDrawDlg::OnCurrentColorAreaInit)
ON_FRESH_VI("color_list", FRCMD_INIT, FrUccDrawDlg::OnColorListInit)
ON_FRESH_VI("color_list", FRCMD_OWNERDRAW, FrUccDrawDlg::OnColorListOwnerDraw)
ON_FRESH_VV("color_list", FRCMD_LBUTTONDOWN, FrUccDrawDlg::OnColorListBtnDown)
ON_FRESH_VI("eraser_list", FRCMD_INIT, FrUccDrawDlg::OnEraserListInit)
ON_FRESH_VI("eraser_list", FRCMD_OWNERDRAW, FrUccDrawDlg::OnEraserListOwnerDraw)
ON_FRESH_VV("eraser_list", FRCMD_LBUTTONDOWN, FrUccDrawDlg::OnEraserListBtnDown)
ON_FRESH_VI("toolinfo_eraser_area", FRCMD_INIT,
	FrUccDrawDlg::OnToolInfoEraserAreaInit)
ON_FRESH_VI("brush_list", FRCMD_INIT, FrUccDrawDlg::OnBrushListInit)
ON_FRESH_VI("brush_list", FRCMD_OWNERDRAW, FrUccDrawDlg::OnBrushListOwnerDraw)
ON_FRESH_VV("brush_list", FRCMD_LBUTTONDOWN, FrUccDrawDlg::OnBrushListBtnDown)
ON_FRESH_VI("pen_list", FRCMD_INIT, FrUccDrawDlg::OnPenListInit)
ON_FRESH_VI("pen_list", FRCMD_OWNERDRAW, FrUccDrawDlg::OnPenListOwnerDraw)
ON_FRESH_VV("pen_list", FRCMD_LBUTTONDOWN, FrUccDrawDlg::OnPenListBtnDown)
ON_FRESH_VI("toolinfo_pen_area", FRCMD_INIT,
	FrUccDrawDlg::OnToolInfoPenAreaInit)
ON_FRESH_VI("toolinfo_brush_area", FRCMD_INIT,
	FrUccDrawDlg::OnToolInfoBrushAreaInit)
ON_FRESH_VI("color_list_area", FRCMD_INIT, FrUccDrawDlg::OnColorListsAreaInit)
ON_FRESH_VI("arrow_down_btn", FRCMD_INIT, FrUccDrawDlg::OnArrowDownBtnInit)
ON_FRESH_VV("arrow_down_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnArrowDownBtnUp)
ON_FRESH_VI("arrow_right_btn", FRCMD_INIT, FrUccDrawDlg::OnArrowRightBtnInit)
ON_FRESH_VV("arrow_right_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnArrowRightBtnUp)
ON_FRESH_VI("arrow_left_btn", FRCMD_INIT, FrUccDrawDlg::OnArrowLeftBtnInit)
ON_FRESH_VV("arrow_left_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnArrowLeftBtnUp)
ON_FRESH_VI("delete_btn", FRCMD_INIT, FrUccDrawDlg::OnDeleteBtnInit)
ON_FRESH_VV("delete_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnDeleteBtnUp)
ON_FRESH_VI("modify_btn", FRCMD_INIT, FrUccDrawDlg::OnModifyBtnInit)
ON_FRESH_VV("modify_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnModifyBtnUp)
ON_FRESH_VI("make_btn", FRCMD_INIT, FrUccDrawDlg::OnMakeBtnInit)
ON_FRESH_VV("make_btn", FRCMD_LBUTTONUP, FrUccDrawDlg::OnMakeBtnUp)
ON_FRESH_VI("rate_list", FRCMD_INIT, FrUccDrawDlg::OnRateListInit)
ON_FRESH_VI("rate_list", FRCMD_OWNERDRAW, FrUccDrawDlg::OnRateListOwnerDraw)
ON_FRESH_VV("rate_list", FRCMD_LBUTTONDOWN, FrUccDrawDlg::OnRateListBtnDown)
ON_FRESH_VI("pattern_list", FRCMD_INIT, FrUccDrawDlg::OnPatternListInit)
ON_FRESH_VI("pattern_list", FRCMD_OWNERDRAW,
	FrUccDrawDlg::OnPatternListOwnerDraw)
ON_FRESH_VV("pattern_list", FRCMD_LBUTTONDOWN,
	FrUccDrawDlg::OnPatternListBtnDown)
ON_FRESH_VI("base_color_list", FRCMD_INIT, FrUccDrawDlg::OnBaseColorListInit)
ON_FRESH_VI("extended_base_pattern_list", FRCMD_INIT,
	FrUccDrawDlg::OnExtendedBasePatternListInit)
ON_FRESH_VI("etc", FRCMD_INIT, FrUccDrawDlg::OnEtcInit)
ON_FRESH_VI("etc", FRCMD_OWNERDRAW, FrUccDrawDlg::OnEtcOwnerDraw)
ON_FRESH_VI("tooltip", FRCMD_INIT, FrUccDrawDlg::OnToolTipInit)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW, FrUccDrawDlg::OnToolTipOnwerDraw)

END_FRESH_MSGMAP()

FrUccDrawDlg::FrUccDrawDlg()
{
	Init();
	// 983 007e5486: em janela, o UCC prende o cursor e usa o cursor do Windows (dtor desfaz)
	m_prevCursorBind983 = BindMouseCursor983();
	if (BindMouseCursor983() != 1 && COption::Instance()->vIsWindowed()) // 00492410 (vWindowMode +0xcc)
	{
		BindMouseCursor983() = 1;
		if (g_mouse)
			g_mouse->InitDevice(g_hwnd, true);
		g_pMouseCursorHelper983->SetVisible(true);
	}
}

FrUccDrawDlg::~FrUccDrawDlg()
{
	Clear();
	m_pMyRoom983 = 0; // 983 dtor 007e5570
	if (m_prevCursorBind983 == 0 && COption::Instance()->vIsWindowed()) // 983 007e55c3
	{
		BindMouseCursor983() = 0;
		if (g_mouse)
			g_mouse->InitDevice(g_hwnd, false);
		g_pMouseCursorHelper983->SetVisible(false);
	}
}

// 983 007e2150: FrForm::OnDraw e, aberto pelo My Room (+0x8b0 = form do My Room,
// gravado em 00620e00), desenha o personagem por cima (0060d9b0).
void MyRoomPetDraw983(char* form); // jp/myroompreview983.cpp
void FrUccDrawDlg::OnDraw()
{
	FrForm::OnDraw();
	if (m_pMyRoom983 != 0)
		MyRoomPetDraw983((char*)m_pMyRoom983);
}

void FrUccDrawDlg::Init()
{
	m_unused38c983 = 0;
	m_pSetInfoDlg = NULL;
	m_pPickColorDlg = NULL;
	m_pAskResetDlg = NULL;
	m_pAskCompleteDlg = NULL;
	m_pAskCancelDlg = NULL;
	m_pWaitingForm = NULL;
	m_bWaiting = false;
	m_waitingTime = 0;
	m_pClothes[0] = NULL;
	m_pClothes[1] = NULL;
	m_pItemInfo = NULL;
	m_pCanvas = NULL;
	m_pUndoBtn = NULL;
	m_pRedoBtn = NULL;
	m_pResetBtn = NULL;
	m_pDragBtn = NULL;
	m_pZoomInBtn = NULL;
	m_pZoomOutBtn = NULL;
	m_pTabDrawBtn = NULL;
	m_pTabPatternBtn = NULL;
	m_pTabEmoticonBtn = NULL;
	m_pTabTextBtn = NULL;
	m_curEraser = 0;
	m_curTool = BRUSH_BRUSH;
	m_curSide = 0;
	m_drawMode = (eDrawMode)0;
	m_bSaved = true;
	m_bCloseAfterSave = false;
	m_name[0] = '\0';
	m_curPattern = -1;
	m_rate = 2;
	m_bArrowDown = true;
	m_bGrid = false;
	m_bInit = true;
	m_lastBrushIdx983 = 0; // 983 Init 007e2690
	m_lastPenIdx983 = -1;
	m_lastEraserIdx983 = -1;
	m_pDescImgDlg = NULL; // 983 0x3a4; lixo aqui ia para Close() no botao de atalhos
	m_pMyRoom983 = 0;
	m_canvasRect = WRect(0, 0, 0, 0);

	m_pToolBox = g_pFresh->RegisterBitmap("ucc_tool_box");
	m_pSelectBox = g_pFresh->RegisterBitmap("ucc_select_box");
	m_pBgText[0][0] = g_pFresh->RegisterBitmap("ucc_bg_text_front1");
	m_pBgText[0][1] = g_pFresh->RegisterBitmap("ucc_bg_text_back1");
	m_pBgText[1][0] = g_pFresh->RegisterBitmap("ucc_bg_text_front2");
	m_pBgText[1][1] = g_pFresh->RegisterBitmap("ucc_bg_text_back2");
	m_pToolChangeIcon = g_pFresh->RegisterBitmap("ucc_tool_change_icon");
	m_pSelectSBox2 = g_pFresh->RegisterBitmap("ucc_select_sbox2");
	m_pSelectBoxYellow = g_pFresh->RegisterBitmap("ucc_select_box_yellow");
	m_pSelectBoxPink = g_pFresh->RegisterBitmap("ucc_select_box_pink");
	m_pSelectColor = g_pFresh->RegisterBitmap("ucc_select_color");

	unsigned char color[9][3] = {
		{ 0xff, 0x1c, 0x1c },
		{ 0xff, 0xb1, 0x08 },
		{ 0xff, 0xe7, 0x19 },
		{ 0x7a, 0xb3, 0x37 },
		{ 0x47, 0xc0, 0xbd },
		{ 0x0e, 0x39, 0x87 },
		{ 0x04, 0x25, 0x63 },
		{ 0x92, 0x1c, 0xbe },
		{ 0x00, 0x00, 0x00 },
	};

	int i;
	for (i = 0; i < 18; ++i)
	{
		m_color[i].Create(1, 1, 32);
		if (i < 9)
		{
			m_color[i].SetPixel(0, 0, color[i][0], color[i][1], color[i][2],
				255);
		}
		else
		{
			m_color[i].SetPixel(0, 0, 255, 255, 255, 0);
		}
	}

	m_toolTip.Add((FrWnd**)&m_pUndoBtn, (eBarButton)0x3a);
	m_toolTip.Add((FrWnd**)&m_pRedoBtn, (eBarButton)0x39);
	m_toolTip.Add((FrWnd**)&m_pResetBtn, (eBarButton)0x3b);
	m_toolTip.Add((FrWnd**)&m_pDragBtn, (eBarButton)0x36);
	m_toolTip.Add((FrWnd**)&m_pZoomInBtn, (eBarButton)0x3c);
	m_toolTip.Add((FrWnd**)&m_pZoomOutBtn, (eBarButton)0x3d);
	m_toolTip.Add((FrWnd**)&m_pTempSaveBtn, (eBarButton)0x44);
	m_toolTip.Add((FrWnd**)&m_pPaintBtn, (eBarButton)0x38);
	m_toolTip.Add((FrWnd**)&m_pPickColorBtn, (eBarButton)0x3e);
	m_toolTip.Add((FrWnd**)&m_pFlipBtn, (eBarButton)0x45);
	m_toolTip.Add((FrWnd**)&m_pGridBtn, (eBarButton)0x37);
	m_toolTip.Add((FrWnd**)&m_pHotkeyDescBtn, (eBarButton)0x46);

	m_tab.Add(UCCTAB_DRAW, &m_pPickColorBtn, &m_pCurrentColorBtn, &m_pPaintBtn,
		&m_pCurrentColorArea, &m_pColorListsArea, NULL);
	m_tab.Add(UCCTAB_DRAW, &m_pColorList, &m_pEraserList, &m_pBrushList,
		&m_pPenList, NULL);
	m_tab.Add(UCCTAB_PATTERN, &m_pPickColorBtn, &m_pCurrentColorBtn,
		&m_pPaintBtn, &m_pCurrentColorArea, NULL);
	m_tab.Add(UCCTAB_PATTERN, &m_pColorList, &m_pEraserList, NULL);
	m_tab.Add(UCCTAB_PATTERN, &m_pArrowDownBtn, &m_pArrowRightBtn,
		&m_pArrowLeftBtn, NULL);
	m_tab.Add(UCCTAB_PATTERN, &m_pDeleteBtn, &m_pModifyBtn, &m_pMakeBtn, NULL);
	m_tab.Add(UCCTAB_PATTERN, &m_pRateList, &m_pPatternList, &m_pBaseColorList,
		NULL);

	int x, y;
	m_patternEmpty.Create(18, 18, 32);
	for (y = 0; y < 18; ++y)
	{
		for (x = 0; x < 18; ++x)
			m_patternEmpty.SetPixel(x, y, 0xf4, 0xf4, 0xf4, 255);
	}

	m_patternDefault.Create(88, 88, 32);
	for (y = 0; y < 88; ++y)
	{
		for (x = 0; x < 88; ++x)
			m_patternDefault.SetPixel(x, y, 255, 255, 255, 255);
	}

	for (i = 0; i < 24; ++i)
	{
		Bitmap* pBrush = UccBrushList(BRUSH_PATTERN)->GetBrush(i);
		if (pBrush)
		{
			GetResizedImage(pBrush, NULL, &m_pattern[i], 18, 18);
		}
		else
		{
			m_pattern[i] = m_patternEmpty;
			UccBrushList(BRUSH_PATTERN)->AddBrush(&m_patternDefault);
		}
	}

	m_cursor.SetRect((FrWnd**)&m_pCanvas);
	m_cursor.Add((COverlayCursor::eCursorType)1, "ucc_cursor_drag.tga", 12, 14);
	m_cursor.Add((COverlayCursor::eCursorType)2, "ucc_cursor_eraser.tga", 5, 6);
	m_cursor.Add((COverlayCursor::eCursorType)3, "ucc_cursor_eyedropper.tga", 6,
		22);
	m_cursor.Add((COverlayCursor::eCursorType)4, "ucc_cursor_paint.tga", 3, 18);
	m_cursor.Add((COverlayCursor::eCursorType)5, "ucc_cursor_select_color.tga",
		10, 10);
	m_cursor.Add((COverlayCursor::eCursorType)6, "ucc_cursor_zoom_in.tga", 11,
		10);
	m_cursor.Add((COverlayCursor::eCursorType)7, "ucc_cursor_zoom_out.tga", 11,
		10);
}

void FrUccDrawDlg::Clear()
{
	UccCurrentBrush()->SetBrush(BRUSH_BRUSH, 0, 0, 0, 0);
	SetCurrentColor(0, 0, 0);

	if (m_pBrushList->GetSelected())
		m_pBrushList->GetSelected()->selected = false;
	if (m_pPenList->GetSelected())
		m_pPenList->GetSelected()->selected = false;
	if (m_pPatternList->GetSelected())
		m_pPatternList->GetSelected()->selected = false;

	m_pBrushList->SelectItem(m_pBrushList->GetItem(0), true);

	if (m_pClothes[0])
	{
		delete m_pClothes[0];
		m_pClothes[0] = NULL;
	}
	if (m_pClothes[1])
	{
		delete m_pClothes[1];
		m_pClothes[1] = NULL;
	}

	m_bGrid = false;
}

bool FrUccDrawDlg::Open(FRESH_PFN_RESULT pFnResult, const WPoint& pos,
	unsigned long flag)
{
	bool ret = FrForm::Open(pFnResult, pos, flag);

	WPoint origin(m_rect.x, m_rect.y);
	m_canvasRect = m_pCanvas->GetRect() - origin;

	MoveCanvasToCenter();
	return ret;
}

void FrUccDrawDlg::OnCancel()
{
	if (IsSaved())
	{
		m_pAskCancelDlg = NULL;
		Clear();
		FrForm::OnCancel();
	}
	else if (!m_pAskCancelDlg)
	{
		m_pAskCancelDlg = CreateForm<FrNotifyForm>(g_pFresh->GetManager(), this,
			"notify_yesnocancel");
		m_pAskCancelDlg->SetMessage(
			K2L_Compatibility("\xb1\xd7\xb8\xae\xb4\xf8 \xc0\xc7\xbb\xf3\xc0\xbb \xc0\xd3\xbd\xc3\xc0\xfa\xc0\xe5\xc7\xcf\xb0\xed \xb3\xa1\xb3\xbb\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			false);
		m_pAskCancelDlg->Open(
			(FRESH_PFN_RESULT)&FrUccDrawDlg::OnAskCancelDlgResult, 1);
	}
}

bool FrUccDrawDlg::Close(eFormRet result, bool bFade)
{
	CloseWaitingForm();
	return FrForm::Close(result, bFade);
}

void FrUccDrawDlg::SetName(const char* name)
{
	strcpy(m_name, name);
}

void FrUccDrawDlg::Upload()
{
	G_CreateDirectory(MakeStr("%s\\", GetUccTempFoler()));

	if (IsSavingTemporarily())
	{
		std::string filename =
			GenerateTextureFullPath(0, m_pItemInfo->tid, m_pItemInfo->UccIndex);
		if (!ZipUccClothes(m_pClothes[0]->GetTexture32BPP(true),
				m_pClothes[1]->GetTexture32BPP(true), NULL, filename.c_str()))
		{
			Notify(MakeStr(
				K2L_Compatibility("\xbe\xf7\xb7\xce\xb5\xe5\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.(\xc4\xda\xb5\xe5:%d)"),
				4));
		}
		else
		{
			NetResManager()->UploadUccTempClothes(m_pItemInfo->tid,
				m_pItemInfo->UccIndex);
		}
	}
	else
	{
		std::string filename =
			GenerateTextureFullPath(0, m_pItemInfo->tid, m_pItemInfo->UccIndex);
		Bitmap* icon = new Bitmap(64, 84, 32);
		CreateItemThumbnail(m_pClothes[0]->GetTexture(),
			m_pClothes[0]->GetMask(), icon);
		if (!ZipUccClothes(m_pClothes[0]->GetTexture(),
				m_pClothes[1]->GetTexture(), icon, filename.c_str()))
		{
			Notify(MakeStr(
				K2L_Compatibility("\xbe\xf7\xb7\xce\xb5\xe5\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.(\xc4\xda\xb5\xe5:%d)"),
				3));
		}
		else
		{
			std::string iconName =
				GenerateIconFilename(m_pItemInfo->tid, m_pItemInfo->UccIndex);
			UccManager()->m_icon.insert(
				std::map<std::string, Bitmap*>::value_type(iconName, icon));
			NetResManager()->UploadUccClothes(m_pItemInfo->tid,
				m_pItemInfo->UccIndex, m_name);
		}
	}
}

void FrUccDrawDlg::Notify(const char* text)
{
	// 983 007e5840: ao ator principal da tarefa (nao ha ator "RealMyRoom" no form vivo; o aviso se perdia)
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)K2L_Compatibility(text), 0, 0, 0, 0));
}

void FrUccDrawDlg::SetCurrentColor(unsigned char r, unsigned char g,
	unsigned char b)
{
	m_currentColor.SetPixel(0, 0, r, g, b, 255);
	RefreshTexCache(m_currentColor);
	UccCurrentBrush()->SetBrush(r, g, b);
}

void FrUccDrawDlg::MoveCanvasToCenter()
{
	WPoint pos(m_rect.x, m_rect.y);
	WPoint org(m_canvasRect.x, m_canvasRect.y);
	pos += org;

	Bitmap* bitmap = m_pClothes[m_curSide]->GetTexture32BPP(false);
	if (bitmap)
	{
		int dx = (256 - bitmap->Width()) / 2;
		int dy = (256 - bitmap->Height()) / 2;
		pos.x += dx;
		pos.y += dy;
	}
	m_pCanvas->MoveWindow(pos);
}

void FrUccDrawDlg::OnProc(const float delta)
{
	if (!m_pClothes[m_curSide])
		return;

	const WPoint& mousePos = g_pFresh->GetManager()->GetMousePos();

	if (m_pClothes[m_curSide]->GetMode() == MODE_DRAW &&
		(m_curTool == BRUSH_BRUSH || m_curTool == BRUSH_PEN))
	{
		m_cursor.SetCursor((COverlayCursor::eCursorType)0);
	}

	if (m_pClothes[m_curSide]->IsMouseOver(mousePos))
	{
		if (g_input->GetButton(LEFT_BUTTON) > 0)
		{
			m_pClothes[m_curSide]->MouseEventHandler(LEFT_BUTTON,
				g_input->GetButton(LEFT_BUTTON), mousePos);
		}
	}
	else if (m_pClothes[m_curSide]->m_bDragging &&
		g_input->GetButton(LEFT_BUTTON) == 2)
	{
		WPoint pos;
		m_pClothes[m_curSide]->MouseEventHandler(LEFT_BUTTON, 2, pos);
	}

	// 983 007e6c50: sem Ctrl, um atalho por quadro (PAINT..PICKCOLOR; BRUSH/PEN/ERASER trocam de ferramenta e,
	// se ela ja estava ativa, passam ao proximo pincel); com Ctrl so UNDO/REDO. FLIP e SAVE nao existem no 983.
	if (!g_input->Get("LCONTROL", false) && !g_input->Get("RCONTROL", false))
	{
		if (g_input->GetDown("PAINT", false))
			OnPaintBtnUp();
		else if (g_input->GetDown("DRAG", false))
			OnDragBtnUp();
		else if (g_input->GetDown("GRID", false))
			OnGridBtnUp();
		else if (g_input->GetDown("BRUSH", false))
		{
			if (m_curTool == BRUSH_BRUSH)
				m_lastBrushIdx983 = (m_lastBrushIdx983 >= 5) ? 0 : m_lastBrushIdx983 + 1;
			UccCurrentBrush()->SetBrush(BRUSH_BRUSH, m_lastBrushIdx983);
			m_pBrushList->GetItem(m_lastBrushIdx983)->selected = true;
			OnTabDrawBtnUp();
			m_curTool = BRUSH_BRUSH;
		}
		else if (g_input->GetDown("PEN", false))
		{
			if (m_curTool == BRUSH_PEN || m_lastPenIdx983 == -1)
				m_lastPenIdx983 = (m_lastPenIdx983 >= 5) ? 0 : m_lastPenIdx983 + 1;
			UccCurrentBrush()->SetBrush(BRUSH_PEN, m_lastPenIdx983);
			m_pPenList->GetItem(m_lastPenIdx983)->selected = true;
			OnTabDrawBtnUp();
			m_curTool = BRUSH_PEN;
		}
		else if (g_input->GetDown("ERASER", false))
		{
			if (m_curTool == BRUSH_ERASER || m_lastEraserIdx983 == -1)
				m_lastEraserIdx983 = (m_lastEraserIdx983 >= 3) ? 0 : m_lastEraserIdx983 + 1;
			UccCurrentBrush()->SetBrush(BRUSH_ERASER, m_lastEraserIdx983);
			m_pEraserList->GetItem(m_lastEraserIdx983)->selected = true;
			OnTabDrawBtnUp();
			m_cursor.SetCursor((COverlayCursor::eCursorType)2);
			m_curTool = BRUSH_ERASER;
		}
		else if (g_input->GetDown("ZOOMIN", false))
			OnZoomInBtnUp();
		else if (g_input->GetDown("ZOOMOUT", false))
			OnZoomOutBtnUp();
		else if (g_input->GetDown("PICKCOLOR", false))
			OnPickColorBtnUp();
	}
	else if (g_input->GetDown("UNDO", false))
		OnUndoBtnUp();
	else if (g_input->GetDown("REDO", false))
		OnRedoBtnUp();

	m_pClothes[m_curSide]->Process();

	if (m_pUndoBtn && m_pRedoBtn && m_pClothes[m_curSide])
	{
		m_pUndoBtn->Enable(m_pClothes[m_curSide]->IsUndoPossible());
		m_pRedoBtn->Enable(m_pClothes[m_curSide]->IsRedoPossible());
	}

	if (m_bWaiting)
	{
		m_waitingTime += delta;

		if (!m_pWaitingForm)
		{
			m_pWaitingForm =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "wait");
			if (IsSavingTemporarily())
			{
				m_pWaitingForm->SetMessage(
					K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce \xc0\xc7\xbb\xf3\xc0\xbb \xc0\xd3\xbd\xc3 \xc0\xfa\xc0\xe5\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
					false);
			}
			else
			{
				m_pWaitingForm->SetMessage(
					K2L_Compatibility("\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce \xc0\xc7\xbb\xf3\xc0\xbb \xbe\xf7\xb7\xce\xb5\xe5 \xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
					false);
			}
			m_pWaitingForm->Open(NULL, 0);

			FrButton* pCancel = DYNAMIC_CAST(FrButton,
				m_pWaitingForm->FindChildByName("cancel"));
			if (pCancel)
				pCancel->Enable(false);
		}

		if (m_waitingTime > 20.0f)
		{
			eResourceType type =
				RESOURCE_UCC_CLOTHES; // 983 007e7086: sem asteca, sempre o tipo 0
			if (NetResManager()->IsFailedToUpload(m_pItemInfo->tid,
					m_pItemInfo->UccIndex, type))
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), // 983 007e70aa
					(int)MakeStr(
						K2L_Compatibility("\xbe\xf7\xb7\xce\xb5\xe5\xb0\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.(\xc4\xda\xb5\xe5:%d)"),
						2),
					0, 0, 0, 0));
				m_bWaiting = false;
			}
		}
	}
	else
	{
		CloseWaitingForm();
	}
}

void FrUccDrawDlg::SetUccDrawInfo(eDrawMode mode, sItemInfo* pItemInfo,
	bool bReset)
{
	m_pItemInfo = pItemInfo;
	m_drawMode = mode;

	unsigned long typeId = pItemInfo->tid;
	char uccIndex[9] = { 0 };
	strncpy(uccIndex, pItemInfo->UccIndex, 9);

	Bitmap temp[2];
	bool bTemp = false;

	if (typeId == 0 || strlen(uccIndex) == 0 || !IsUccClothes(typeId))
	{
		Notify(MakeStr(
			K2L_Compatibility("\xbc\xad\xb9\xf6\xb7\xce\xba\xce\xc5\xcd \xc7\xca\xbf\xe4\xc7\xd1 \xc1\xa4\xba\xb8\xb8\xa6 \xbe\xf2\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9(\xc4\xda\xb5\xe5:%d)"),
			1));
		m_bInit = false;
		OnCancel();
		return;
	}

	if (!bReset)
	{
		bTemp = UccManager()->GetUccTempClothes(m_pItemInfo->tid,
			m_pItemInfo->UccIndex, temp[0], temp[1]);
	}

	switch (mode)
	{
	case DRAW_CLOTHES:
		m_pClothes[0] = new CUccClothes;
		m_pClothes[1] = new CUccClothes;
		break;
	case DRAW_AZTEC:
		break;
	default:
		Notify(MakeStr(
			K2L_Compatibility("\xbf\xb9\xbb\xf3\xc4\xa1 \xb8\xf8\xc7\xd1 \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9(\xc4\xda\xb5\xe5:%d)"),
			1));
		m_bInit = false;
		OnCancel();
		return;
	}

	m_curSide = 0;

	if (mode == DRAW_CLOTHES)
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
		if (!part)
		{
			Notify(
				K2L_Compatibility("\xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xb4\xc2 \xc0\xc7\xbb\xf3\xc0\xd4\xb4\xcf\xb4\xd9."));
			m_bInit = false;
			OnCancel();
			return;
		}

		for (int i = 0; i < 2; ++i)
		{
			if (m_pClothes[i]->IsInitialized())
				continue;

			Bitmap* texture = g_resrcmng->LoadBitmap(part->Tex[i], 0, false);
			Bitmap* org = g_resrcmng->LoadBitmap(part->Tex[i], 0, false);
			Bitmap mask;

			if (org)
			{
				mask = *org;
			}
			else
			{
				mask.Create(texture->Width(), texture->Height(), 24);
				for (unsigned int y = 0; y < mask.Height(); ++y)
				{
					for (unsigned int x = 0; x < mask.Width(); ++x)
						mask.SetPixel(x, y, 255, 255, 255, 255);
				}
			}

			Bitmap* maskFile = g_resrcmng->LoadBitmap(
				GetMaskFilename(part->Tex[i]).c_str(), 0, false);
			if (maskFile)
			{
				for (unsigned int y = 0; y < mask.Height(); ++y)
				{
					for (unsigned int x = 0; x < mask.Width(); ++x)
					{
						unsigned char r, g, b;
						maskFile->GetPixel(x, y, r, g, b);
						if (r < 5 && g < 5 && b < 5)
							mask.SetPixel(x, y, 0, 0, 0, 255);
					}
				}
			}

			m_pClothes[i]->InitializeItemDraw(i, m_pCanvas, texture, &mask,
				m_pItemInfo->tid, m_pItemInfo->UccIndex);

			ClearTexCache(texture);
			if (texture)
				delete texture;
			ClearTexCache(org);
			if (org)
				delete org;
			ClearTexCache(maskFile);
			if (maskFile)
				delete maskFile;

			if (m_pClothes[i]->GetTexture())
			{
				g_resrcmng->UploadTexture(m_pClothes[i]->GetTextureName(),
					m_pClothes[i]->GetTexture(), 0, NULL);
			}

			m_pClothes[i]->SetParent(this);

			if ((m_pItemInfo->status & 2) && bTemp)
			{
				Bitmap* texture24 = m_pClothes[i]->GetTexture();
				Bitmap* texture32 = m_pClothes[i]->GetTexture32BPP(true);
				*texture32 = temp[i];
				Convert32to24(texture32, texture24, &mask);
			}
		}
	}

	if (!m_pClothes[0] || !m_pClothes[0]->GetTexture() ||
		!m_pClothes[0]->GetTexture32BPP(false) ||
		(m_pClothes[1] &&
			(!m_pClothes[1]->GetTexture() ||
				!m_pClothes[1]->GetTexture32BPP(false))))
	{
		Notify(
			K2L_Compatibility("\xc5\xd8\xbd\xba\xc3\xc4\xb8\xa6 \xba\xd2\xb7\xaf\xbf\xc0\xb4\xc2 \xb5\xa5\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."));
		m_bInit = false;
		OnCancel();
		return;
	}

	unsigned int zoomX =
		256 / m_pClothes[m_curSide]->GetTexture32BPP(false)->Width();
	unsigned int zoomY =
		256 / m_pClothes[m_curSide]->GetTexture32BPP(false)->Height();
	int maxZoom = zoomX < zoomY ? zoomX : zoomY;

	while (m_pClothes[m_curSide]->GetZoomRate() < maxZoom)
	{
		bool bZoomed = true;
		if (m_pClothes[0])
			bZoomed = m_pClothes[0]->ZoomIn(-1, -1);
		if (m_pClothes[1])
			bZoomed = bZoomed && m_pClothes[1]->ZoomIn(-1, -1);
		if (!bZoomed)
			break;
	}

	m_pZoomInBtn->Enable(m_pClothes[m_curSide]->GetZoomRate() < 8);
	m_pZoomOutBtn->Enable(m_pClothes[m_curSide]->GetZoomRate() > 1);

	if (m_canvasRect.w != 0)
		MoveCanvasToCenter();

	for (int i = 1; i >= 0; --i)
	{
		if (m_pClothes[i])
		{
			m_pClothes[i]->MustRefresh(2);
			m_pClothes[i]->Process();
		}
	}
}

void FrUccDrawDlg::OnUccCanvasInit(int param)
{
	m_pCanvas = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnUndoBtnInit(int param)
{
	m_pUndoBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnUndoBtnUp()
{
	if (m_pClothes[m_curSide])
		m_pClothes[m_curSide]->Undo();
}

void FrUccDrawDlg::OnRedoBtnInit(int param)
{
	m_pRedoBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnRedoBtnUp()
{
	if (m_pClothes[m_curSide])
		m_pClothes[m_curSide]->Redo();
}

void FrUccDrawDlg::OnFlipBtnInit(int param)
{
	m_pFlipBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnFlipBtnOwnerDraw(int param)
{
	WRect rect = m_pFlipBtn->GetRect();

	FrGraphicInterface* pGDI = GetGDI();
	if (!pGDI)
		return;

	if (m_pToolChangeIcon)
	{
		pGDI->DrawTexture(m_pToolChangeIcon,
			WRect(rect.x + 6.0f, rect.y + 6.0f,
				(float)m_pToolChangeIcon->Width(),
				(float)m_pToolChangeIcon->Height()),
			0xffffffff, 0);
	}
}

void FrUccDrawDlg::OnFlipBtnUp()
{
	eMode mode = m_pClothes[m_curSide]->GetMode();

	m_pClothes[m_curSide]->SetMode(MODE_DRAW);
	m_curSide = !m_curSide;

	m_pClothes[m_curSide]->MustRefresh(1);

	if (m_pSide1Area)
		m_pSide1Area->SetBgImg(m_pBgText[0][m_curSide]);
	if (m_pSide2Area)
		m_pSide2Area->SetBgImg(m_pBgText[1][1 - m_curSide]);

	m_pZoomInBtn->Enable(m_pClothes[m_curSide]->GetZoomRate() < 8);
	m_pZoomOutBtn->Enable(m_pClothes[m_curSide]->GetZoomRate() > 1);

	MoveCanvasToCenter();

	m_pClothes[m_curSide]->SetMode(mode);
}

void FrUccDrawDlg::OnResetBtnInit(int param)
{
	m_pResetBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnResetBtnUp()
{
	if (!m_pAskResetDlg)
	{
		m_pAskResetDlg =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno");
		m_pAskResetDlg->SetMessage(
			K2L_Compatibility("\xc1\xa4\xb8\xbb \xc3\xb3\xc0\xbd\xba\xce\xc5\xcd \xb4\xd9\xbd\xc3 \xb1\xd7\xb8\xae\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee? \n\n(\xc7\xf6\xc0\xe7 \xc0\xcc \xb1\xd7\xb8\xae\xb1\xe2\xbf\xeb \xc0\xc7\xbb\xf3\xbf\xa1 \xb1\xd7\xb7\xc1\xc1\xae \xc0\xd6\xb4\xc2 \n   \xb8\xf0\xb5\xe7 \xb1\xd7\xb8\xb2\xc0\xcc \xc1\xf6\xbf\xf6\xc1\xfd\xb4\xcf\xb4\xd9)"),
			false);
		m_pAskResetDlg->Open(
			(FRESH_PFN_RESULT)&FrUccDrawDlg::OnAskResetDlgResult, 1);
	}
}

void FrUccDrawDlg::OnGridBtnInit(int param)
{
	m_pGridBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnGridBtnUp()
{
	m_bGrid = !m_bGrid;
}

void FrUccDrawDlg::OnDragBtnInit(int param)
{
	m_pDragBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnDragBtnUp()
{
	CUccClothes* pClothes = m_pClothes[m_curSide];
	if (pClothes->GetMode() == MODE_DRAG)
	{
		pClothes->SetMode(MODE_DRAW);
		m_cursor.SetCursor((COverlayCursor::eCursorType)0);
	}
	else
	{
		pClothes->SetMode(MODE_DRAG);
		m_cursor.SetCursor((COverlayCursor::eCursorType)1);
	}
	SetToolbarStatus(m_pDragBtn);
}

void FrUccDrawDlg::OnZoomInBtnInit(int param)
{
	m_pZoomInBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnZoomInBtnUp()
{
	SetToolbarStatus(m_pZoomInBtn);
	m_cursor.SetCursor((COverlayCursor::eCursorType)6);
	m_pClothes[m_curSide]->SetMode(MODE_ZOOMIN);
}

void FrUccDrawDlg::OnZoomOutBtnInit(int param)
{
	m_pZoomOutBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnZoomOutBtnUp()
{
	SetToolbarStatus(m_pZoomOutBtn);
	m_cursor.SetCursor((COverlayCursor::eCursorType)7);
	m_pClothes[m_curSide]->SetMode(MODE_ZOOMOUT);
}

void FrUccDrawDlg::AfterZoom()
{
	m_pZoomInBtn->Enable(m_pClothes[m_curSide]->GetZoomRate() < 8);
	m_pZoomOutBtn->Enable(m_pClothes[m_curSide]->GetZoomRate() > 1);

	MoveCanvasToCenter();

	if (m_pClothes[m_curSide]->GetMode() == MODE_DRAW)
	{
		m_cursor.SetCursor((COverlayCursor::eCursorType)0);
		SetToolbarStatus(m_pTabDrawBtn);
	}
}

void FrUccDrawDlg::SetSaved(bool bSaved)
{
	// HACK: emission order (the original compiles SetSaved one wave later, right after AfterZoom)
	if (0)
		AfterZoom();

	m_bSaved = bSaved;

	if (bSaved)
	{
		m_fadingText.Print(
			"\xbc\xbf\xc7\xc1\xb5\xf0\xc0\xda\xc0\xce \xc0\xc7\xbb\xf3\xc0\xcc \xc0\xd3\xbd\xc3 \xc0\xfa\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.",
			3.0f, this, 105.0f, 20.0f, FADE_INOUT, 0xff000000);
	}

	if (m_bCloseAfterSave)
		Close(FrOK, true);
}

void FrUccDrawDlg::OnSide1AreaInit(int param)
{
	m_pSide1Area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnSide2AreaInit(int param)
{
	m_pSide2Area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnTempSaveBtnInit(int param)
{
	m_pTempSaveBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnTempSaveBtnUp()
{
	SecurityKeyManager()->ReqCreateKey(SK_UCC, m_pItemInfo->guid);
	m_bWaiting = true;
}

void FrUccDrawDlg::OnTabDrawBtnInit(int param)
{
	m_pTabDrawBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTabDrawBtn->SetStatus(FrButton::PRESSED);
}

void FrUccDrawDlg::OnTabDrawBtnUp()
{
	ChangeTab(UCCTAB_DRAW);
	m_pClothes[m_curSide]->SetMode(MODE_DRAW);
	m_cursor.SetCursor((COverlayCursor::eCursorType)0);
	SetToolbarStatus(m_pTabDrawBtn);
}

void FrUccDrawDlg::OnTabPatternBtnInit(int param)
{
	m_pTabPatternBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnTabPatternBtnUp()
{
	ChangeTab(UCCTAB_PATTERN);
	m_pClothes[m_curSide]->SetMode(MODE_DRAW);

	m_cursor.SetCursor((COverlayCursor::eCursorType)0);
	SetToolbarStatus(m_pTabPatternBtn);
}

void FrUccDrawDlg::OnTabEmoticonBtnInit(int param)
{
	m_pTabEmoticonBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTabEmoticonBtn->Enable(false);
}

void FrUccDrawDlg::OnTabEmoticonBtnUp()
{
}

void FrUccDrawDlg::OnTabTextBtnInit(int param)
{
	m_pTabTextBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTabTextBtn->Enable(false);
}

void FrUccDrawDlg::OnTabTextBtnUp()
{
}

void FrUccDrawDlg::OnFinishBtnInit(int param)
{
	m_pFinishBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::Finish()
{
	if (!m_pItemInfo)
		return;

	if (!m_pSetInfoDlg)
	{
		WRect rect = m_rect;
		m_pSetInfoDlg = CreateForm<FrUccSetInfoDlg>(g_pFresh->GetManager(),
			this, "ucc_setinfo");
		m_pSetInfoDlg->SetParent(this);
		m_pSetInfoDlg->Open(
			(FRESH_PFN_RESULT)&FrUccDrawDlg::OnUccSetInfoDlgResult,
			WPoint(rect.x + 30.0f, rect.y + 30.0f), 0);
	}
}

void FrUccDrawDlg::OnFinishBtnUp()
{
	Finish();
}

bool FrUccDrawDlg::OnAskCompleteDlgResult(int result, FrForm* pForm)
{
	m_pAskCompleteDlg = NULL;
	if (result == FrOK)
		Finish();
	return true;
}

bool FrUccDrawDlg::OnAskCancelDlgResult(int result, FrForm* pForm)
{
	m_pAskCancelDlg = NULL;

	if (result == FrYES)
	{
		m_bCloseAfterSave = true;
		SecurityKeyManager()->ReqCreateKey(SK_UCC, m_pItemInfo->guid);
		m_bWaiting = true;
	}
	else if (result == FrNO)
	{
		Clear();
		FrForm::OnCancel();
	}

	return true;
}

bool FrUccDrawDlg::OnAskResetDlgResult(int result, FrForm* pForm)
{
	if (result == FrOK)
		Reset();
	m_pAskResetDlg = NULL;
	return true;
}

bool FrUccDrawDlg::OnUccSetInfoDlgResult(int result, FrForm* pForm)
{
	m_pSetInfoDlg = NULL;
	if (result == 1)
	{
		SecurityKeyManager()->ReqCreateKey(SK_UCC, m_pItemInfo->guid);
		m_bWaiting = true;
	}
	return true;
}

void FrUccDrawDlg::OnCloseBtnInit(int param)
{
	m_pCloseBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnCloseBtnUp()
{
	OnCancel();
}

void FrUccDrawDlg::OnPaintBtnInit(int param)
{
	m_pPaintBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnPaintBtnUp()
{
	m_pClothes[m_curSide]->SetMode(MODE_PAINT);
	m_cursor.SetCursor((COverlayCursor::eCursorType)4);
	SetToolbarStatus(m_pTabDrawBtn);
}

void FrUccDrawDlg::OnPickColorBtnInit(int param)
{
	m_pPickColorBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnPickColorBtnUp()
{
	m_pClothes[m_curSide]->SetMode(MODE_SPOID);
	m_cursor.SetCursor((COverlayCursor::eCursorType)3);
	SetToolbarStatus(m_pTabDrawBtn);
}

void FrUccDrawDlg::OnCurrentColorBtnInit(int param)
{
	m_pCurrentColorBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnCurrentColorBtnUp()
{
	if (!m_pPickColorDlg)
	{
		WRect rect = m_rect;
		m_pPickColorDlg = CreateForm<FrUccPickColorDlg>(g_pFresh->GetManager(),
			this, "ucc_pickcolor");
		m_pPickColorDlg->SetParent(this);
		m_pPickColorDlg->Open(
			(FRESH_PFN_RESULT)&FrUccDrawDlg::OnUccPickColorDlgResult,
			WPoint(rect.x + 30.0f, rect.y + 30.0f), 0);
	}
}

// 983 007e58b0: abre a imagem de atalhos (form "selfdesigndescimg"); ja aberta = fecha
void FrUccDrawDlg::OnHotkeyDescBtnInit(int param)
{
	m_pHotkeyDescBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnHotkeyDescBtnUp983()
{
	if (m_pDescImgDlg)
	{
		m_pDescImgDlg->Close(true);
		return;
	}
	m_pDescImgDlg = CreateForm<SelfDesignHotkeyDescImgDlg>(g_pFresh->GetManager(), this, "selfdesigndescimg");
	if (m_pDescImgDlg)
	{
		WRect rect = m_rect;
		m_pDescImgDlg->Open((FRESH_PFN_RESULT)&FrUccDrawDlg::OnDescImgDlgResult983, WPoint(rect.x + 5.0f, rect.y + 5.0f), 1);
		m_pDescImgDlg->SetFadeout(false);
	}
}

bool FrUccDrawDlg::OnDescImgDlgResult983(int result, FrForm* pForm)
{
	m_pDescImgDlg = NULL;
	return true;
}

bool FrUccDrawDlg::OnUccPickColorDlgResult(int result, FrForm* pForm)
{
	m_pPickColorDlg = NULL;
	return true;
}

void FrUccDrawDlg::OnCurrentColorAreaInit(int param)
{
	m_pCurrentColorArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);

	m_currentColor.Create(1, 1, 32);
	m_currentColor.SetPixel(0, 0, 0, 0, 0, 255);
	m_pCurrentColorArea->SetBgImg(&m_currentColor);
}

void FrUccDrawDlg::OnColorListInit(int param)
{
	m_pColorList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	BuildColorList();
}

void FrUccDrawDlg::OnColorListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pGDI = GetGDI();
	if (!pGDI)
		return;

	WRect colorRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f, 9.0f, 9.0f);
	pGDI->DrawTexture((Bitmap*)pItem->pData, colorRect, 0xffffffff, 0);

	if (pItem->selected)
	{
		WRect selectRect(pItem->pos.x - 3.0f, pItem->pos.y - 3.0f,
			(float)m_pSelectColor->Width(), (float)m_pSelectColor->Height());
		pGDI->DrawTexture(m_pSelectColor, selectRect, 0xffffffff, 0);
	}
}

void FrUccDrawDlg::OnColorListBtnDown()
{
	FrListItem* pItem = m_pColorList->GetItemUnderCursor();
	if (!pItem)
		return;

	Bitmap* pColor = (Bitmap*)pItem->pData;
	if (!pColor)
		return;

	unsigned char r, g, b, a;
	pColor->GetPixel(0, 0, r, g, b, a);

	if (a == 255)
		SetCurrentColor(r, g, b);

	if (m_curTool == BRUSH_ERASER)
	{
		UccCurrentBrush()->SetBrush(BRUSH_ERASER,
			m_pEraserList->GetSelected()->idx);
	}
}

void FrUccDrawDlg::OnEraserListInit(int param)
{
	m_pEraserList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	BuildEraserList();
}

void FrUccDrawDlg::OnEraserListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pGDI = GetGDI();
	if (!pGDI)
		return;

	if (!m_pEraserList->GetSelected() && pItem->idx == m_curEraser)
		m_pEraserList->SelectItem(pItem, true);

	// 983 007e5bc0: PATTERN nao desmarca; moldura no ultimo indice escolhido (lido antes de desmarcar)
	if (pItem->selected)
	{
		if (m_curTool == BRUSH_BRUSH || m_curTool == BRUSH_PEN)
			pItem->selected = false;
		if (m_lastEraserIdx983 != pItem->idx)
			return;
		const Bitmap* pSelect = m_pSelectBoxYellow;
		pGDI->DrawTexture(pSelect,
			WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f,
				(float)pSelect->Width(), (float)pSelect->Height()),
			0xffffffff, 0);
	}
}

void FrUccDrawDlg::OnEraserListBtnDown()
{
	FrListItem* pItem = m_pEraserList->GetItemUnderCursor();
	if (!pItem)
		return;
	if (!pItem->pData)
		return;

	FrListItem* pSelected = m_pBrushList->GetSelected();
	if (pSelected)
		pSelected->selected = false;

	pSelected = m_pPenList->GetSelected();
	if (pSelected)
		pSelected->selected = false;

	pSelected = m_pPatternList->GetSelected();
	if (pSelected)
	{
		pSelected->selected = false;
		m_curPattern = -1;
	}

	UccCurrentBrush()->SetBrush(BRUSH_ERASER, pItem->idx);

	// 983 007e6951: cursor e ferramenta depois de OnTabDrawBtnUp, que passa pelo ChangeTab e zera os dois
	OnTabDrawBtnUp();
	m_cursor.SetCursor((COverlayCursor::eCursorType)2);
	m_curTool = BRUSH_ERASER;
	m_lastEraserIdx983 = pItem->idx; // 983 007e696d
}

void FrUccDrawDlg::OnToolInfoEraserAreaInit(int param)
{
	m_pToolInfoEraserArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnBaseColorListInit(int param)
{
	m_pBaseColorList = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnExtendedBasePatternListInit(int param)
{
	m_pExtendedBasePatternList = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnBrushListInit(int param)
{
	m_pBrushList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	BuildBrushList();
}

void FrUccDrawDlg::OnBrushListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pGDI = GetGDI();
	if (!pGDI)
		return;

	if (!m_pBrushList->GetSelected() && !m_pPenList->GetSelected() &&
		pItem->idx == UccCurrentBrush()->index &&
		UccCurrentBrush()->type == BRUSH_BRUSH)
	{
		m_pBrushList->SelectItem(pItem, true);
	}

	// 983 007e3f10: a moldura sai no item do ultimo indice escolhido (lido antes de desmarcar)
	if (pItem->selected)
	{
		if (m_curTool == BRUSH_ERASER || m_curTool == BRUSH_PEN)
			pItem->selected = false;
		if (m_lastBrushIdx983 != pItem->idx)
			return;
		const Bitmap* pSelect = m_pSelectBoxYellow;
		pGDI->DrawTexture(pSelect,
			WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f,
				(float)pSelect->Width(), (float)pSelect->Height()),
			0xffffffff, 0);
	}
}

void FrUccDrawDlg::OnBrushListBtnDown()
{
	FrListItem* pItem = m_pBrushList->GetItemUnderCursor();
	if (!pItem)
		return;
	if (!pItem->pData)
		return;

	FrListItem* pSelected = m_pPenList->GetSelected();
	if (pSelected)
		pSelected->selected = false;

	UccCurrentBrush()->SetBrush(BRUSH_BRUSH, pItem->idx);

	OnTabDrawBtnUp();
	m_curTool = BRUSH_BRUSH; // 983 007e69c6
	m_lastBrushIdx983 = pItem->idx; // 983 007e69d3
}

void FrUccDrawDlg::OnPenListInit(int param)
{
	m_pPenList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	BuildPenList();
}

void FrUccDrawDlg::OnPenListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pGDI = GetGDI();
	if (!pGDI)
		return;

	if (!m_pBrushList->GetSelected() && !m_pPenList->GetSelected() &&
		pItem->idx == UccCurrentBrush()->index &&
		UccCurrentBrush()->type == BRUSH_PEN)
	{
		m_pPenList->SelectItem(pItem, true);
	}

	// 983 007e3fa0: a moldura sai no item do ultimo indice escolhido (lido antes de desmarcar)
	if (pItem->selected)
	{
		if (m_curTool == BRUSH_ERASER || m_curTool == BRUSH_BRUSH)
			pItem->selected = false;
		if (m_lastPenIdx983 != pItem->idx)
			return;
		const Bitmap* pSelect = m_pSelectBoxYellow;
		pGDI->DrawTexture(pSelect,
			WRect(pItem->pos.x + 1.0f, pItem->pos.y + 1.0f,
				(float)pSelect->Width(), (float)pSelect->Height()),
			0xffffffff, 0);
	}
}

void FrUccDrawDlg::OnPenListBtnDown()
{
	FrListItem* pItem = m_pPenList->GetItemUnderCursor();
	if (!pItem)
		return;
	if (!pItem->pData)
		return;

	FrListItem* pSelected = m_pBrushList->GetSelected();
	if (pSelected)
		pSelected->selected = false;

	UccCurrentBrush()->SetBrush(BRUSH_PEN, pItem->idx);

	OnTabDrawBtnUp();
	m_curTool = BRUSH_PEN;
	m_lastPenIdx983 = pItem->idx; // 983 007e6a33
}

void FrUccDrawDlg::OnToolInfoPenAreaInit(int param)
{
	m_pToolInfoPenArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnToolInfoBrushAreaInit(int param)
{
	m_pToolInfoBrushArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnColorListsAreaInit(int param)
{
	m_pColorListsArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnArrowDownBtnInit(int param)
{
	m_pArrowDownBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnArrowDownBtnUp()
{
	m_bArrowDown = !m_bArrowDown;

	if (m_bArrowDown)
	{
		m_pArrowLeftBtn->Enable(m_curPattern > 0);
		m_pArrowRightBtn->Enable(m_curPattern + 6 < 24);
		m_pExtendedBasePatternList->SetVisible(false);

		WRect listRect = m_pPatternList->GetRect();
		listRect.h = 27.0f;
		m_pPatternList->SetRect(listRect);

		WRect arrowRect = m_pArrowDownBtn->GetRect();
		arrowRect.y -= 77.0f;
		m_pArrowDownBtn->SetRect(arrowRect);

		m_pArrowDownBtn->SetButtonImg("ucc_arrow_down_n", FrButton::NORMAL);
		m_pArrowDownBtn->SetButtonImg("ucc_arrow_down_o", FrButton::OVER);
	}
	else
	{
		m_pArrowLeftBtn->Enable(false);
		m_pArrowRightBtn->Enable(false);
		m_pExtendedBasePatternList->SetVisible(true);

		WRect listRect = m_pPatternList->GetRect();
		listRect.h = 101.0f;
		m_pPatternList->SetRect(listRect);

		WRect arrowRect = m_pArrowDownBtn->GetRect();
		arrowRect.y += 77.0f;
		m_pArrowDownBtn->SetRect(arrowRect);

		m_pArrowDownBtn->SetButtonImg("ucc_arrow_up_n", FrButton::NORMAL);
		m_pArrowDownBtn->SetButtonImg("ucc_arrow_up_o", FrButton::OVER);
	}

	BuildPatternList(m_curPattern, m_bArrowDown);
}

void FrUccDrawDlg::OnArrowRightBtnInit(int param)
{
	m_pArrowRightBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUccDrawDlg::OnArrowRightBtnUp()
{
	if (m_pPatternList->m_itemList.size() != 0)
	{
		FrListItem* pFirst = *m_pPatternList->m_itemList.begin();
		int first = (int)pFirst->pData;

		if (first + 6 < 24)
		{
			m_pArrowLeftBtn->Enable(first + 1 > 0);
			m_pArrowRightBtn->Enable(first + 7 < 24);

			m_pPatternList->ClearItem();
			for (int i = 0; i < 6; ++i)
				m_pPatternList->AddItem((void*)(first + 1 + i));
		}
	}
}

void FrUccDrawDlg::OnArrowLeftBtnInit(int param)
{
	m_pArrowLeftBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pArrowLeftBtn->Enable(false);
}

void FrUccDrawDlg::OnArrowLeftBtnUp()
{
	if (m_pPatternList->m_itemList.size() != 0)
	{
		FrListItem* pFirst = *m_pPatternList->m_itemList.begin();
		int first = (int)pFirst->pData;

		if (first != 0)
		{
			m_pArrowLeftBtn->Enable(first - 1 > 0);
			m_pArrowRightBtn->Enable(first + 5 < 24);

			m_pPatternList->ClearItem();
			for (int i = 0; i < 6; ++i)
				m_pPatternList->AddItem((void*)(first - 1 + i));
		}
	}
}

void FrUccDrawDlg::OnDeleteBtnInit(int param)
{
	m_pDeleteBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pDeleteBtn->Enable(false);
}

void FrUccDrawDlg::OnDeleteBtnUp()
{
}

void FrUccDrawDlg::OnModifyBtnInit(int param)
{
	m_pModifyBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pModifyBtn->Enable(false);
}

void FrUccDrawDlg::OnModifyBtnUp()
{
}

void FrUccDrawDlg::OnMakeBtnInit(int param)
{
	m_pMakeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pMakeBtn->Enable(false);
}

void FrUccDrawDlg::OnMakeBtnUp()
{
}

void FrUccDrawDlg::OnRateListInit(int param)
{
	m_pRateList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pRateList->AddItem((void*)-1);
	m_pRateList->AddItem((void*)-1);
	m_pRateList->AddItem((void*)-1);
	m_pRateList->AddItem((void*)-1);
}

void FrUccDrawDlg::OnRateListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pGDI = GetGDI();
	if (!pGDI)
		return;

	int rate = (pItem->idx + 1) * 2;
	char filename[40];
	sprintf(filename, "ucc_tool_scale_%d.tga", rate);
	Bitmap* pScale = g_resrcmng->LoadBitmap(filename, 0, false);

	if (!pScale)
		return;

	pGDI->DrawTexture(pScale,
		WRect(pItem->pos.x, pItem->pos.y, (float)pScale->Width(),
			(float)pScale->Height()),
		0xffffffff, 0);
	ClearTexCache(pScale); // 983 007e4587: LoadBitmap aloca a cada quadro
	delete pScale;

	if (rate == m_rate)
	{
		const Bitmap* pSelect = m_pSelectSBox2;
		pGDI->DrawTexture(pSelect,
			WRect(pItem->pos.x, pItem->pos.y, (float)pSelect->Width(),
				(float)pSelect->Height()),
			0xffffffff, 0);
	}
}

void FrUccDrawDlg::OnRateListBtnDown()
{
	FrListItem* pItem = m_pRateList->GetItemUnderCursor();
	if (pItem)
	{
		m_rate = (pItem->idx + 1) * 2;
		BuildCurrentPattern();
	}
}

void FrUccDrawDlg::OnPatternListInit(int param)
{
	m_pPatternList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	BuildPatternList(0, true);
}

void FrUccDrawDlg::OnPatternListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pGDI = GetGDI();
	if (!pGDI)
		return;

	bool bSelected = (int)pItem->pData == m_curPattern;

	if (bSelected)
	{
		if (m_curTool == BRUSH_ERASER)
			pItem->selected = false;
	}

	pGDI->DrawTexture(m_pToolBox,
		WRect(pItem->pos.x, pItem->pos.y, (float)m_pToolBox->Width(),
			(float)m_pToolBox->Height()),
		0xffffffff, 0);

	const Bitmap* pPattern = &m_pattern[(int)pItem->pData];
	if (pPattern)
	{
		if (bSelected)
		{
			pGDI->DrawTexture(&m_patternEmpty,
				WRect(pItem->pos.x, pItem->pos.y, (float)m_patternEmpty.Width(),
					(float)m_patternEmpty.Height()),
				0xffffffff, 0);
		}

		pGDI->DrawTexture(pPattern,
			WRect(pItem->pos.x + 3.0f, pItem->pos.y + 3.0f,
				(float)pPattern->Width(), (float)pPattern->Height()),
			0xffffffff, 0);
	}

	if (bSelected)
	{
		const Bitmap* pSelect = m_pSelectBox;
		pGDI->DrawTexture(pSelect,
			WRect(pItem->pos.x - 2.0f, pItem->pos.y - 2.0f,
				(float)pSelect->Width(), (float)pSelect->Height()),
			0xffffffff, 0);
	}
}

void FrUccDrawDlg::OnPatternListBtnDown()
{
	FrListItem* pItem = m_pPatternList->GetItemUnderCursor();
	if (pItem)
	{
		m_curPattern = (int)pItem->pData;

		m_curTool = BRUSH_PATTERN;
		BuildCurrentPattern();

		if (!m_bArrowDown)
			OnArrowDownBtnUp();
	}
}

void FrUccDrawDlg::OnEtcInit(int param)
{
	m_pEtc = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	ChangeTab(UCCTAB_DRAW);
}

void FrUccDrawDlg::OnEtcOwnerDraw(int param)
{
	WRect rect;
	rect.x = m_pCanvas->GetRect().x;
	rect.y = m_pCanvas->GetRect().y;

	if (!m_pClothes[m_curSide])
		return;

	rect.w = m_pClothes[m_curSide]->GetTexture32BPP(false)->Width();
	rect.h = m_pClothes[m_curSide]->GetTexture32BPP(false)->Height();

	// 983 (007e47e0): largura +6, altura so +3
	rect = WRect(rect.x - 3.0f, rect.y - 3.0f, rect.w + 6.0f, rect.h + 3.0f);

	FrGraphicInterface* pGDI = GetGDI();
	pGDI->LineBox(rect, 0x33000000);

	if (m_bGrid)
	{
		float step = m_pClothes[m_curSide]->GetZoomRate() * 5.0f;

		for (float x = rect.x; x <= rect.x + rect.w; x += step)
		{
			g_view->DrawLine2D(WPoint(x, rect.y), WPoint(x, rect.y + rect.h),
				0x0f000000, 0x0f000000, 0);
		}
		for (float y = rect.y; y <= rect.y + rect.h; y += step)
		{
			g_view->DrawLine2D(WPoint(rect.x, y), WPoint(rect.x + rect.w, y),
				0x0f000000, 0x0f000000, 0);
		}
	}

	WPoint pos(m_rect.x + 191.0f, m_rect.y + 298.0f);
	pGDI->SetTextStyle(1);
	float width = pGDI->PrintText11(pos, 0,
		m_curSide ? K2L_Compatibility("\xb5\xde\xb8\xe9") : K2L_Compatibility("\xbe\xd5\xb8\xe9"), NULL);
	pGDI->SetTextStyle(0);

	char zoom[16];
	sprintf(zoom, ": %d%%", m_pClothes[m_curSide]->GetZoomRate() * 100);
	pGDI->PrintText11(WPoint(pos.x + width, pos.y), 0, zoom, NULL);

	// 983 (007e47e0): titulos em 35,283 e 31,104; aviso em vermelho embaixo
	WPoint label(m_rect.x + 35.0f, m_rect.y + 283.0f);
	pGDI->PrintText11(label, 0,
		K2L_Compatibility("\xb5\xda\xc1\xfd\xb1\xe2"), NULL);
	label.x -= 4.0f;
	label.y -= 179.0f;
	pGDI->PrintText11(label, 0,
		K2L_Compatibility("\xc0\xd3\xbd\xc3\xc0\xfa\xc0\xe5"), NULL);

	m_cursor.Render();
	m_fadingText.Process();

	pGDI->SetTextColor(0xffff0000, 0xffffffff);
	pGDI->Print(WPoint(m_rect.x + 17.0f, m_rect.y + 423.0f), 0,
		K2L_Compatibility("\xc1\xa6" "3\xc0\xda\xc0\xc7 \xb1\xc7\xb8\xae\xb8\xa6 \xc4\xa7\xc7\xd8/\xb9\xcc\xc7\xb3\xbe\xe7\xbd\xc4\xc0\xbb"));
	pGDI->Print(WPoint(m_rect.x + 17.0f, m_rect.y + 438.0f), 0,
		K2L_Compatibility("\xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xb0\xa1 \xc0\xd6\xb4\xc2 \xb5\xf0\xc0\xda\xc0\xce\xc0\xba \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"));
}

void FrUccDrawDlg::OnToolTipInit(int param)
{
	m_pToolTip = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUccDrawDlg::OnToolTipOnwerDraw(int param)
{
	m_toolTip.Process();
}

void FrUccDrawDlg::BuildBrushList()
{
	if (m_pBrushList)
	{
		m_pBrushList->ClearItem();
		for (int i = 0; i < UccBrushList(BRUSH_BRUSH)->GetBrushNum(); ++i)
			m_pBrushList->AddItem(UccBrushList(BRUSH_BRUSH)->GetBrush(i));
	}
}

void FrUccDrawDlg::BuildPenList()
{
	if (m_pPenList)
	{
		m_pPenList->ClearItem();
		for (int i = 0; i < UccBrushList(BRUSH_PEN)->GetBrushNum(); ++i)
			m_pPenList->AddItem(UccBrushList(BRUSH_PEN)->GetBrush(i));
	}
}

void FrUccDrawDlg::BuildEraserList()
{
	if (m_pEraserList)
	{
		m_pEraserList->ClearItem();
		for (int i = 0; i < UccBrushList(BRUSH_ERASER)->GetBrushNum(); ++i)
			m_pEraserList->AddItem(UccBrushList(BRUSH_ERASER)->GetBrush(i));
	}
}

void FrUccDrawDlg::BuildPatternList(int index, bool bSelect)
{
	if (!m_pPatternList)
		return;

	int start = (index + 1) / 6 * 6;
	int end;

	if (bSelect)
	{
		int count = UccBrushList(BRUSH_PATTERN)->GetBrushNum();
		count = count < 6 ? count : 6;

		end = start + count;

		if (end > 24)
		{
			start = start - end + 24;
			end = 24;
		}
	}
	else
	{
		start = 0;
		end = 24;
	}

	m_pPatternList->ClearItem();
	for (int i = start; i < end; ++i)
		m_pPatternList->AddItem((void*)i);
}

void FrUccDrawDlg::BuildColorList()
{
	if (m_pColorList)
	{
		m_pColorList->ClearItem();
		for (int i = 0; i < 18; ++i)
			m_pColorList->AddItem(&m_color[i]);
	}
}

void FrUccDrawDlg::BuildCurrentPattern()
{
	if (0)
		((FrUccDrawDlg*)0)->FrUccDrawDlg::BuildPatternList(*(int*)0, *(bool*)0);
	Bitmap* pPattern = UccBrushList(BRUSH_PATTERN)->GetBrush(m_curPattern);
	Bitmap pattern;

	if (!pPattern)
		return;
}

void FrUccDrawDlg::ChangeTab(eUccTab tab)
{
	m_tab.Open(tab);

	m_curTab = tab;

	if (!m_bArrowDown)
		OnArrowDownBtnUp();

	switch (tab)
	{
	case UCCTAB_DRAW:
		m_pColorList->Enable(true);
		m_pPickColorBtn->Enable(true);
		m_pCurrentColorBtn->Enable(true);

		if (m_pPenList->GetSelected())
			m_curTool = BRUSH_PEN;
		else
			m_curTool = BRUSH_BRUSH;
		break;
	case UCCTAB_PATTERN:
		m_pColorList->Enable(false);
		m_pPickColorBtn->Enable(false);
		m_pCurrentColorBtn->Enable(false);
		m_curTool = BRUSH_PATTERN;
		break;
	}
}

void FrUccDrawDlg::Reset()
{
	eDrawMode mode = m_drawMode;
	sItemInfo* pItemInfo = m_pItemInfo;

	Clear();
	SetUccDrawInfo(mode, pItemInfo, true);
	ChangeTab(UCCTAB_DRAW);

	for (int i = 1; i >= 0; --i)
	{
		if (m_pClothes[i])
		{
			m_pClothes[i]->MustRefresh(2);
			m_pClothes[i]->Process();
		}
	}
}

void FrUccDrawDlg::CloseWaitingForm()
{
	if (m_pWaitingForm)
	{
		m_pWaitingForm->Close(FrOK, true);
		m_pWaitingForm = NULL;
		m_waitingTime = 0;
		m_bWaiting = false;
	}
}

void FrUccDrawDlg::SetToolbarStatus(FrButton* pButton)
{
	m_pGridBtn->SetStatus(FrButton::NORMAL);
	m_pDragBtn->SetStatus(FrButton::NORMAL);
	m_pZoomInBtn->SetStatus(FrButton::NORMAL);
	m_pZoomOutBtn->SetStatus(FrButton::NORMAL);
	if (m_pTabDrawBtn)
		m_pTabDrawBtn->SetStatus(FrButton::NORMAL);
	if (m_pTabPatternBtn)
		m_pTabPatternBtn->SetStatus(FrButton::NORMAL);
	if (m_pTabEmoticonBtn)
		m_pTabEmoticonBtn->SetStatus(FrButton::NORMAL);
	if (m_pTabTextBtn)
		m_pTabTextBtn->SetStatus(FrButton::NORMAL);
	if (pButton)
		pButton->SetStatus(FrButton::PRESSED);
}

bool FrUccDrawDlg::IsSavingTemporarily()
{
	return strlen(m_name) == 0;
}
