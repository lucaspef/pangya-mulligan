#pragma once

class FrCmdTarget;
class FrGuiItem;
class FrWnd;
class FrWndManager;

class FrMacroItem
{
public:
	FrMacroItem();
	void SetOwner(FrCmdTarget* pOwner);
	void Init(FrGuiItem& item, FrWndManager* pManager, FrWnd* pParent);

protected:
	FrCmdTarget* m_pOwner;
};
