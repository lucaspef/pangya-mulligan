#pragma once
#include "frelement.h"

class Bitmap;
class FrElement;
class FrCmdTarget;
class FrWndManager;
class FrDesktop;

class Fresh
{
public:
	Fresh();
	virtual ~Fresh();
	bool Init(const char*, bool, const char*);
	void ShutDown();
	void OnProcess(float);
	void OnDisplay(bool);
	void CloseLayout();
	const FreshBitmapMap& ListBres(std::string&);
	void HidePrivacy(bool);
	bool IsCurrentLayout(const char*) const;
	const Bitmap* RegisterBitmap(const char*);
	const Bitmap* GetBitmap(const char*);
	bool OpenLayout(const char*, FrCmdTarget*, bool);
	bool IsDesktopFocused() const;
	FrDesktop* GetDesktop() const;
#ifdef REBANG_GOLFRULE_PCH
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	void Toggle(bool s);
#else
	void Toggle(bool s) { show = s; }
#endif
#endif
#ifdef REBANG_CLIENTSETTING_PCH
	// HACK: this accessor is emitted by taskmain, not clientsetting.
	__declspec(dllimport) __forceinline FrWndManager* GetManager() const
	{
		return manager;
	}
#else
	FrWndManager* GetManager() const { return manager; }
#endif
	bool IsVisible() const;

public:
	FrElementDoc document;
	FrWndManager* manager;
	bool show;
	std::string filename;
};
