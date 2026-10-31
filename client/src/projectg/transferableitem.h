#pragma once

#include <list>
#include "singleton.h"
#include "../shared/globalgamedefine.h"
#include "../shared/globalnetworkdefine.h"

struct sMailBoxListForPage
{
	std::list<sMailInfoBrief> m_lMailList;
	int iTotalPage;
	int iCurPage;
	int iTotalCount;
};

class CTransferableItem : public WSingleton<CTransferableItem>
{
public:
	CTransferableItem();
	virtual ~CTransferableItem();

	void OnLoad();
	void OnDestory();
	void Process(float time);
	void Render();
	void Reset();

	std::list<sTradeItem>* GetList_TransferableItem();
	void ClearTransferableItemList();
	void SetTransferableItemList();
	bool GetTransferableItem(sItemInfo& info, sTradeItem& item, bool bCount);
	void SetTransferableEtcItemList();
	void SetListEquipParts();
	bool IsEquipParts(unsigned long tid);
	bool IsUccEquipped(sItemInfo* pItem);
	int GetNumEquipItem(unsigned long tid);

	void Set_RegistrationListSize(int size);
	bool SetRegistrationItem(unsigned long guid);
	bool SetRegistrationItem(unsigned long guid, int num);
	bool IsTransferableItem(unsigned long guid, sTradeItem& item, int num);
	bool IsRegisterationItem(unsigned long guid, sTradeItem& item);
	bool IsRegisterationItem(unsigned long guid);
	std::list<sTradeItem>* GetList_RegistrationItem();
	void ResetRegistrationItemList();
	int Get_RegistrationListSize();
	bool DelRegistrationItem(unsigned long guid);
	bool DelRegistrationItem_itor_return(
		unsigned long guid, std::list<sTradeItem>::iterator* next);

	void SetMailInfoPage(sMailBoxListForPage& page);
	void SetMailInfo(sMailInfo& info);
	sMailBoxListForPage* GetMailBoxPageList();
	sMailInfo* GetMailInfo();

	int GetNumTradingItem(unsigned long guid);
	bool IsEquipParts(unsigned long uid, unsigned long tid);
	bool IsCheckCardParts(unsigned long uid, unsigned long tid);

private:
	std::list<sTradeItem> m_listTransferableItem;
	std::list<unsigned long> m_listEquipParts;
	std::list<sTradeItem> m_listRegistrationItem;
	unsigned int m_registrationListSize;
	sMailBoxListForPage m_mailBoxPageList;
	sMailInfo m_mailInfo;
};
