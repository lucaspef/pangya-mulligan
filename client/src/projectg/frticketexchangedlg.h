#pragma once

#include "frform.h"
#include <map>

class FrTicketExchangeDescDlg;
// HACK
#define FR_DYNAMIC_CAST(T, p) \
	((__rtti_obj = (void*)(p)) \
			? (T*)((IObject*)__rtti_obj)->DynamicCast(&T::m_RTTI) \
			: (T*)NULL)

class TicketInfo : public WSingleton<TicketInfo>
{
public:
	struct sTicketItem
	{
		unsigned long count;
		unsigned long remain;
	};

	TicketInfo()
	{
		m_ticketCount = 0;
		m_state = 0xff;
	}
	virtual ~TicketInfo() { m_itemMap.clear(); }

	unsigned long m_ticketCount;
	unsigned char m_state;
	std::map<unsigned long, sTicketItem> m_itemMap;

	sTicketItem FindCount(unsigned long tid)
	{
		sTicketItem item;
		memset(&item, 0, sizeof(item));

		std::map<unsigned long, sTicketItem>::iterator it = m_itemMap.find(tid);
		if (it != m_itemMap.end())
		{
			item = it->second;
		}
		return item;
	}
};

class TicketItemObject
{
public:
	TicketItemObject()
	{
		m_tid = 0;
		m_quantity = 0;
		m_pButton = NULL;
		m_pViewer = NULL;
		m_pFrameViewer = NULL;

		m_pEdit[0] = NULL;
		m_pEdit[1] = NULL;
	}
	~TicketItemObject() { }

private:
	unsigned long m_tid;
	unsigned long m_quantity;
	FrButton* m_pButton;
	FrEdit* m_pEdit[2];
	FrViewer* m_pViewer;
	FrViewer* m_pFrameViewer;

public:
	void set_TID(unsigned long tid) { m_tid = tid; }
	unsigned long get_TID() { return m_tid; }

	void set_Quantity(unsigned long quantity) { m_quantity = quantity; }
	unsigned long get_Quantity() { return m_quantity; }

	FrButton* get_Button(int obj)
	{
		if (m_pButton == NULL)
			m_pButton = FR_DYNAMIC_CAST(FrButton, obj);

		return m_pButton;
	}

	FrEdit* get_Edit(unsigned int index, int obj)
	{
		if (index > 2)
			return NULL;

		if (m_pEdit[index] == NULL)
			m_pEdit[index] = FR_DYNAMIC_CAST(FrEdit, obj);

		return m_pEdit[index];
	}

	FrViewer* get_Viewer(int obj, bool bFrame)
	{
		FrViewer* pViewer = m_pViewer;
		if (bFrame)
			pViewer = m_pFrameViewer;

		if (pViewer == NULL)
			pViewer = FR_DYNAMIC_CAST(FrViewer, obj);

		if (bFrame)
			m_pFrameViewer = pViewer;
		else
			m_pViewer = pViewer;

		return pViewer;
	}
};

class FrTicketExchangeDescDlg : public FrForm
{
	DECLARE_OBJECT(FrTicketExchangeDescDlg)

	FrTicketExchangeDescDlg();
	virtual ~FrTicketExchangeDescDlg();

	void SetDescImage(const char* image);

protected:
	void OnInitDescViewer(int);

	FrViewer* m_pDescViewer;
	char m_descImage[64];

	DECLARE_FRESH_MSGMAP()
};

class FrTicketExchangeDlg : public FrForm
{
	DECLARE_OBJECT(FrTicketExchangeDlg)

	FrTicketExchangeDlg();
	virtual ~FrTicketExchangeDlg();

	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void Reponse(unsigned long type, unsigned char result);
	bool OnTicketExchangeDescResult(int result, FrForm* form);

protected:
	void onInit_Icon_0(int);
	void onInit_Icon_1(int);
	void onInit_Icon_2(int);
	void onInit_Icon_3(int);
	void onInit_Icon_4(int);
	void onInit_Icon_5(int);
	void onInit_Icon_6(int);
	void onInit_Icon_7(int);

	void onInit_ItemE0(int);
	void onLBtnUp_ItemE0();
	void onInit_Edit0E0(int);
	void onInit_Edit1E0(int);
	void onInit_ItemE1(int);
	void onLBtnUp_ItemE1();
	void onInit_Edit0E1(int);
	void onInit_Edit1E1(int);

	void onInit_ItemG0(int);
	void onLBtnUp_ItemG0();
	void onInit_Edit0G0(int);
	void onInit_ItemG1(int);
	void onLBtnUp_ItemG1();
	void onInit_Edit0G1(int);
	void onInit_ItemG2(int);
	void onLBtnUp_ItemG2();
	void onInit_Edit0G2(int);
	void onInit_ItemG3(int);
	void onLBtnUp_ItemG3();
	void onInit_Edit0G3(int);

	void onInit_ItemI0(int);
	void onLBtnUp_ItemI0();
	void onInit_Item_I1(int);
	void onLBtnUp_ItemI1();

	void onInit_View0(int);
	void onInit_View1(int);
	void onInit_View2(int);
	void onInit_View3(int);

	void onInit_Number_0(int);
	void onInit_Number_1(int);
	void onInit_Number_2(int);
	void onInit_Number_3(int);

	void onLBtnUp_Help();

private:
	TicketItemObject* FindItem(unsigned long id);
	void Auto_Initialize(unsigned int kind, unsigned int index, int obj,
		unsigned long id);
	void ChangeItemState(TicketItemObject* item, int state);
	void SetCountImage(unsigned int count);
	bool Request(unsigned int type, unsigned long tid, unsigned int quantity);
	void ApplyInformation();

	std::map<unsigned long, TicketItemObject*> m_itemMap;
	FrTicketExchangeDescDlg* m_pDescDlg;
	FrViewer* m_pNumber[4];

	static sFRESH_ENTRY _MsgEntries[];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;
#ifndef REBANG_SEMANTIC_ONLY
public:
#endif
};
