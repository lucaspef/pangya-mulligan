#pragma once
#include <string>
#include "rtti.h"
#include "frwnd.h"

class Bitmap;
class FrGuiItem;
class FrWndManager;

// 983: efeito de entrada do FrArea (parâmetro "entrance_effect" = "from:top openingtime:1 duration:2 openingsound:x").
// A área entra deslizando de um lado, espera `duration` e sai pelo mesmo lado. Classe sem RTTI; nomes meus.
// sizeof 0x30 (push em 00bec30a).
class FrArea;
class FrAreaEntrance983
{
public:
	enum { FROM_TOP, FROM_BOTTOM, FROM_LEFT, FROM_RIGHT };
	enum { ST_WAIT_FADE, ST_SOUND, ST_OPENING, ST_HOLD, ST_CLOSING, ST_DONE };

	FrAreaEntrance983(FrArea* pArea); // 00beb2e0
	void Parse(const char* text); // 00beb400
	void SetFrom(const char* from); // 00beaa90
	void OnProc(float elapsed); // 00beae60
	void Opening(float elapsed); // 00beab90
	void Closing(float elapsed); // 00beacc0
	static float __stdcall Step(float target, float cur, float remain, float elapsed, bool* done); // 00bea560

	FrArea* m_pArea; // 0x00
	int m_from; // 0x04
	float m_remain; // 0x08 ("openingtime")
	float m_duration; // 0x0c ("duration"; < 0 = não fecha)
	std::string m_sound; // 0x10 ("openingsound")
	int m_state; // 0x2c
};

class FrArea : public FrWnd
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	FrArea();
	virtual ~FrArea();

	void Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent);
	bool SetBgImg(const Bitmap* pBitmap);
	bool SetBgImg(const char* name);
#ifdef REBANG_FR_LATE_PCH_INLINES
	const Bitmap* GetBgImg();
#else
	const Bitmap* GetBgImg() { return m_pBitmap; }
#endif
	void SetMouseEvent(bool active);
	void EnableClipping(bool enable) { m_isClipping = enable; }
	void SetClipArea(const WRect& rect) { m_rcClipArea = rect; }
	void GetClipArea(WRect& rect) { rect = m_rcClipArea; }

protected:
	virtual bool ClippingArea(WRect& src, WRect& dst);
	virtual void OnDraw();
	virtual void OnProc(const float deltaTime); // 983: 00beaf80
	virtual void OnSetCursor(bool bInClient, const WPoint& mousePos);
	virtual bool OnLButtonUp(const WPoint& mousePos);
	virtual bool OnLButtonDown(const WPoint& mousePos);

	FrGuiItem* m_pItem;
	const Bitmap* m_pBitmap;
	bool m_center;
	bool m_stretch;
	bool m_mouseEvent;
	unsigned long m_color;
	std::string m_imgName;
	bool m_isClipping;
	WRect m_rcClipArea;
	friend class FrAreaEntrance983;
	FrAreaEntrance983* m_pEntrance983; // 983: 0x110 (parâmetro "entrance_effect"); sizeof 0x114
};
