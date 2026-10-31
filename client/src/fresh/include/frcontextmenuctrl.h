#pragma once
#include <list>
#include "frwnd.h"
#include "../../../include/engine/wtypes.h"

class Bitmap;
class FrElementFrame;
class FrGuiItem;
class FrWndManager;

class FrContextMenuCtrl : public FrWnd
{
public:
	FrContextMenuCtrl();
	virtual ~FrContextMenuCtrl();
	virtual void SetVisible(bool visible);
	void Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent);
	void UnableMenuItem(const char* text);
	void ToggleMenuItem(const char* text, const char* newText);
	const char* GetSelectMenuText();
	void SetSelectMenuText(const char* text);
	void SetMenuList(const char* list);

	enum eMenuType
	{
		SEPARATOR,
		MENU
	};

	struct sMenuItem
	{
		int nType;
		char szText[32];
		bool underCursor;
		bool selected;
		bool Enable;
		float top;
		float bottom;
		const Bitmap* bmp;
	};

	typedef std::list<sMenuItem> MENU_LIST;

protected:
	MENU_LIST m_MenuList;
	MENU_LIST::iterator m_underCursor;
	MENU_LIST::iterator m_selected;
	unsigned long m_color;
	unsigned long m_outlineColor;
	unsigned long m_style;
	unsigned long m_selColor;
	unsigned long m_selOutlineColor;
	unsigned long m_unableColor;
	unsigned long m_selStyle;
	bool m_center;
	WRect* m_clip;
	float m_hlAlpha;
	char m_hlAlphaD;
	bool m_bIsIcon;

	virtual void OnDraw();
	virtual void OnProc(const float deltaTime);
	virtual void OnMouseMove(const WPoint& point);
	virtual bool OnLButtonDown(const WPoint& point);
	virtual bool OnLButtonUp(const WPoint& point);
	void Select(MENU_LIST::iterator it);
	void Unselect(MENU_LIST::iterator it);
	void UpdateRectInfo(int type, const Bitmap** bitmaps);
	void DrawFrame(int type, float x, float y, const Bitmap** bitmaps);
	void DrawIcon(WPoint& point, const Bitmap* bitmap, float alpha, bool b);

	FrElementFrame* m_pFrame;
	WSize m_a;
	WSize m_b;
	WSize m_c;
	WSize m_min;
	float m_width;
	float m_height;
	const Bitmap* m_pBaseBmp[9];
	const Bitmap* m_pSubBmp[9];
};
