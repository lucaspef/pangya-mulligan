#pragma once

#include "frform.h"
#include "uccbase.h"
#include "uccbrush.h"

class CUccEditBrush : public CUccBase
{
public:
	CUccEditBrush();
	virtual ~CUccEditBrush();

	virtual bool IsInitialized() { return m_pTexture != NULL; }
	virtual void Process();
	virtual void DrawLine(IPoint from, IPoint to, Bitmap* brush);

	Bitmap* GetTexture32BPP();
	Bitmap* ProduceOriginalBrush();

protected:
	virtual void Refresh2D() { }
	virtual void Refresh3D() { }

	Bitmap m_originalBrush;
};

class FrUccEditBrushDlg : public FrForm
{
	DECLARE_OBJECT(FrUccEditBrushDlg)
	FrUccEditBrushDlg();
	virtual ~FrUccEditBrushDlg();

	void InitNew();
	void InitModify();
	void SetIndex(eBrushType type, int index);

protected:
	void OnCanvasInit(int param);
	void OnOKBtnUp();
	void OnCancelBtnUp();

	virtual void OnProc(const float dt);

	int m_index;
	eBrushType m_type;
	FrArea* m_pCanvas;
	Bitmap* m_pCanvasImage;
	Bitmap m_defaultBrush[2];
	CUccEditBrush m_editBrush;

	DECLARE_FRESH_MSGMAP()
};
