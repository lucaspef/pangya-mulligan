#pragma once

#include "frform.h"
#include "ucclibrary.h"
#include "ucccolorpicker.h"

class FrArea;
class FrListBox;

class FrUccDrawDlg;

class FrUccPickColorDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrUccPickColorDlg)

	FrUccPickColorDlg();

	void Init();
	void SetParent(FrUccDrawDlg* pParent);
	void BuildColorList();
	void SetCurrentColor(unsigned char r, unsigned char g, unsigned char b);
	void MoveCanvasCursor(unsigned char x, unsigned char y);
	void MoveCanvasCursor(unsigned char r, unsigned char g, unsigned char b);
	void MoveCanvasCursor(const WPoint& pos);
	void MoveBrightnessCursor(unsigned char l);
	void MoveBrightnessCursor(unsigned char r, unsigned char g,
		unsigned char b);
	void MoveBrightnessCursor(const WPoint& pos);

protected:
	virtual void OnProc(const float delta);

	void OnColorListInit(int param);
	void OnColorListOwnerDraw(int param);
	void OnColorListBtnDown();
	void OnCancelBtnInit(int param);
	void OnCancelBtnUp();
	void OnOkBtnInit(int param);
	void OnOkBtnUp();
	void OnUseColorBtnInit(int param);
	void OnUseColorBtnUp();
	void OnCanvasInit(int param);
	void OnCanvasCursorInit(int param);
	void OnBrightnessCanvasInit(int param);
	void OnBrightnessCursor1Init(int param);
	void OnBrightnessCursor2Init(int param);
	void OnNewColorInit(int param);
	void OnPrevColorInit(int param);

private:
	void FindColor(unsigned char r, unsigned char g, unsigned char b);

protected:
	FrUccDrawDlg* m_pParent;
	Bitmap m_prevColor;
	Bitmap m_newColor;
	sRGB m_color;
	CUccColorPicker m_colorPicker;
	CUccColorBrightness m_colorBrightness;
	const Bitmap* m_pSelectColor;
	FrButton* m_pCancelBtn;
	FrButton* m_pOkBtn;
	FrButton* m_pUseColorBtn;
	FrListBox* m_pColorList;
	FrArea* m_pCanvas;
	FrArea* m_pCanvasCursor;
	FrArea* m_pBrightnessCanvas;
	FrArea* m_pBrightnessCursor1;
	FrArea* m_pBrightnessCursor2;
	FrArea* m_pNewColor;
	FrArea* m_pPrevColor;

private:
	DECLARE_FRESH_MSGMAP()
};
