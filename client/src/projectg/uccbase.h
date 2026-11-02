#pragma once

#include <list>
#include <vector>
#include "ucclibrary.h"
class FrArea;
class FrUccDrawDlg;

struct sDrawHistory
{
	sDrawHistory()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}
	int x;
	int y;
	int width;
	int height;
	Bitmap undo;
	Bitmap redo;
};
class CDrawHistory
{
public:
	CDrawHistory()
		: m_current(-1)
	{
	}

	~CDrawHistory() { Clear(); }

	void Clear();
	void Add(Bitmap* before, Bitmap* after, IPoint min, IPoint max);
	void Undo(Bitmap* bitmap);
	void Redo(Bitmap* bitmap);
	bool IsUndoPossible();
	bool IsRedoPossible();

private:
	void Draw(Bitmap* bitmap, sDrawHistory* history, bool bRedo);
	void CopyBitmap(Bitmap* src, Bitmap* dst, int sx, int sy, int w, int h,
		int dx, int dy);

	int m_current;
	std::vector<sDrawHistory*> m_history;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbAddFunclets(); // HACK: EH funclets of the naked Add (nakedify)
#endif
};

enum eMode
{
	MODE_DRAW,
	MODE_DRAG,
	MODE_SPOID,
	MODE_PAINT,
	MODE_ZOOMIN,
	MODE_ZOOMOUT
};

class CUccBase
{
	friend class FrUccDrawDlg;

public:
	CUccBase();
	virtual ~CUccBase();

	virtual void Initialize(FrArea* area, Bitmap* texture, Bitmap* mask);

	virtual bool IsInitialized() { return m_pArea && m_pTexture && m_pMask; }

	void SetParent(FrUccDrawDlg* parent) { m_pParent = parent; }

	virtual void InitVariables();
	virtual void DestroyVariables();

	virtual void Process();
	virtual void MouseEventHandler(eButton button, int event,
		const WPoint& pos);
	virtual bool IsMouseOver(const WPoint& pos);

	virtual void Draw(int x, int y, Bitmap* brush, bool bFirst);
	virtual void DrawLine(IPoint from, IPoint to, Bitmap* brush);

	void Paint(int x, int y, bool bUseBrushColor, unsigned char r,
		unsigned char g, unsigned char b);

	void MustRefresh(int flag) { m_refresh |= flag; }

	void MakeZoomedArea(int x, int y);
	bool ZoomIn(int x, int y);
	bool ZoomOut(int x, int y);
	bool IsZoomed() { return m_zoomRate > 1; }
	int GetZoomRate() { return m_zoomRate; }

	void SetMode(eMode mode) { m_mode = mode; }
	eMode GetMode() { return m_mode; }

	void Undo();
	void Redo();
	bool IsUndoPossible();
	bool IsRedoPossible();

	void SetTexture(Bitmap* texture);
	void SetMask(Bitmap* mask);
	void SetDefaultBrush(Bitmap* brush);

	Bitmap* GetTexture32BPP(bool bOriginal);
	Bitmap* GetTexture() { return m_pTexture; }

	Bitmap* GetMask() { return m_pMask; }

	void SetArea(FrArea* area) { m_pArea = area; }

protected:
	virtual void Refresh2D();
	virtual void Refresh3D();
	virtual void RefreshCache();

	virtual void DrawEventHandler(eButton button, int event, const WPoint& pos);
	virtual void MoveEventHandler(eButton button, int event, const WPoint& pos);
	virtual void SpoidEventHandler(eButton button, int event,
		const WPoint& pos);
	virtual void PaintEventHandler(eButton button, int event,
		const WPoint& pos);
	virtual void ZoomEventHandler(eButton button, int event, const WPoint& pos);

	void _DrawZoomedArea();

	bool m_bDragging;
	Bitmap* m_pTexture;
	Bitmap* m_pMask;
	Bitmap* m_pTexture32;
	FrArea* m_pArea;
	Bitmap* m_pDefaultBrush;
	IPoint m_lastPos;
	CDrawHistory m_history;
	Bitmap* m_pBackup;
	CDrawHistory m_history32;
	Bitmap* m_pBackup32;
	IPoint m_min;
	IPoint m_max;
	int m_refresh;
	WRect m_zoomArea;
	int m_zoomRate;
	Bitmap* m_pZoomed;
	eMode m_mode;
	FrUccDrawDlg* m_pParent;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbDrawzoomedareaFunclets(); // HACK: EH funclets of the naked _DrawZoomedArea (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbDraweventhandlerFunclets(); // HACK: EH funclets of the naked DrawEventHandler (nakedify)
#endif
};

class CUccBaseItemDraw : public CUccBase
{
public:
	CUccBaseItemDraw();
	virtual ~CUccBaseItemDraw();

	void InitializeItemDraw(int side, FrArea* area, Bitmap* texture,
		Bitmap* mask, unsigned long typeID, const char* uccIndex);
	virtual bool IsInitialized();
	virtual void Process();

	char* GetTextureName() { return m_textureName; }

private:
	virtual void Refresh2D();
	virtual void Refresh3D();
	void GenerateTextureName(int side, unsigned long typeID,
		const char* uccIndex);

	unsigned long m_typeID;
	char m_textureName[64];
	char m_uccIndex[9];
};
