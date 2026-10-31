#pragma once
#include <list>
#include "rtti.h"
#include "frwnd.h"
#include "../../../include/engine/wtypes.h"

class Bitmap;
class FrGuiItem;
class FrWndManager;

struct TButtonItem
{
	const Bitmap* bitmap[3];
	unsigned long index;
	WRect rect;
	bool underCursor;
	bool selected;
};

typedef std::list<TButtonItem*> TBUTTON_LIST;

class FrTabButton : public FrWnd
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	FrTabButton();
	virtual ~FrTabButton();
	void Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent);
	void ClearTab();
	void AddTab(const char* imgNames[], unsigned long index); // 983: sem o tamanho (00c07e10)
	void AddTab(const char* imgName, unsigned long index, WSize* size);
	void Select(unsigned long index);
	void UnSelect(unsigned long index);
#ifdef REBANG_DEFER_SETSEPWIDTH // HACK: makeroomdlg emission order (SetSepWidth was a PCH inline there)
	void SetSepWidth(float width)
	{
		if (0)
			GetRect();
		m_sepWidth = width;
	}
#else
	void SetSepWidth(float width) { m_sepWidth = width; }
#endif
	TButtonItem* GetItemUnderCursor() { return m_underCursor; }
	TButtonItem* GetSelected() { return m_selected; }

protected:
	void Select(TButtonItem* i);
	void UnSelect(TButtonItem* i);
	void InitButtonItem(TButtonItem* tab, unsigned long index, WSize* size);
	virtual void PreCreateWindow(FrWndManager* manager, unsigned long style,
		const WRect& rect, FrWnd* parent);
	virtual void OnDraw();
	virtual void OnProc(const float deltaTime);
	virtual void OnMouseMove(const WPoint& point);
	virtual bool OnLButtonDown(const WPoint& point);
	virtual bool OnLButtonUp(const WPoint& point);
	virtual void OnSetCursor(bool bInClient, const WPoint& point);

	float m_sepWidth;
	TBUTTON_LIST m_tBtnList;
	TButtonItem* m_underCursor;
	TButtonItem* m_selected;
	TButtonItem* m_pressed;
	const Bitmap* m_sepImg;
	const Bitmap* m_pBtnBgImg[3];

	enum eBtnType
	{
		BT_NONE,
		BT_DOWN,
		BT_UP
	};

	eBtnType m_btnType;
};
