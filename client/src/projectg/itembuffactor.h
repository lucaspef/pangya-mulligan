#pragma once

#include <list>
#include "actor.h"
#include "frcmdtarget.h"
#include "../../include/engine/wtypes.h"

struct sSCardAvilityPeriodInfo;
struct sItemInfo;
class FrSPCardBuffDlg;
class WReceivedPacket;
struct FrListItem;
class FrForm;
class Bitmap;

class CItemBuff : public IActor, public FrCmdTarget
{
protected:
	CItemBuff();

public:
	DECLARE_OBJECT(CItemBuff)

	virtual void OnLoad();
	virtual void HandleMsg(const MsgObject& msg);

	static void ResetBuffList();
	static void ProcessPacket(WReceivedPacket& packet);
	static void AddBuffItem(sSCardAvilityPeriodInfo& info);
	static void RemoveBuffItem(unsigned long typeId);
	static void UpdateBuffDlg(FrSPCardBuffDlg* dlg);

protected:
	void HandleBuffItemSubMsg(const MsgObject& msg);
	void DrawMiniButton(FrListItem* pItem, const WPoint& point);
	void ButtonDown(FrListItem* pItem, const WPoint& point);
	void UseBuffItem(sItemInfo* pItem);
	void IsCompatibleBuff(unsigned long typeId,
		const std::list<sSCardAvilityPeriodInfo>& buffList, bool& bCompatible,
		unsigned long& conflictId);
	bool OnUseBuffItemDlgResult(int result, FrForm* pForm);

	const Bitmap* m_pNormalBmp;
	const Bitmap* m_pOverBmp;
	WRect m_rect;
	unsigned long m_typeId;

	static std::list<sSCardAvilityPeriodInfo> ms_buffList;
};
