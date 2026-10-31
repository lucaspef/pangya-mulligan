#pragma once

#include "frform.h"
class FrPointEventNoticeDlg;
class FrPointEventDescDlg;

class CCommodityItem
{
public:
	enum eItemType
	{
	};

	CCommodityItem();
	CCommodityItem(const CCommodityItem& rhs);
	CCommodityItem(unsigned long typeID, unsigned long value,
		eItemType itemType);
	virtual ~CCommodityItem() { }

	void SetTypeID(unsigned long typeID);
	void SetValue(unsigned long value);
	void SetItemType(eItemType itemType);
	unsigned long GetTypeID() const;
	unsigned long GetValue() const;
	eItemType GetItemType() const;

	const CCommodityItem& operator=(const CCommodityItem& rhs);

protected:
	eItemType m_itemType;
	unsigned long m_typeID;
	unsigned long m_value;
};

class CCommodity
{
public:
	static const int MAX_ITEM_COUNT = 4;

	CCommodity();
	CCommodity(const CCommodity& rhs);
	CCommodity(const CCommodityItem& item, unsigned short cost);
	virtual ~CCommodity() { }

	void SetCommodityCost(unsigned short cost);
	unsigned short GetCommodityItemCount() const;
	unsigned short GetCommodityCost() const;
	bool InsertCommodityItem(const CCommodityItem& item);
	const CCommodityItem& GetCommodityItem(unsigned short index) const;

	const CCommodity& operator=(const CCommodity& rhs);

protected:
	CCommodityItem m_item[MAX_ITEM_COUNT];
	unsigned short m_itemCount;
	unsigned short m_cost;
};

class CCommodityTable
{
public:
	static const int MAX_COMMODITY_COUNT = 10;

	CCommodityTable();
	virtual ~CCommodityTable() { }

	const CCommodity& GetCommodity(unsigned short index) const;

protected:
	bool InitializeTable();

	CCommodity m_commodity[MAX_COMMODITY_COUNT];
};

class CCommodityStatusManager
{
public:
	static const int MAX_COMMODITY_COUNT = 10;

	enum eUpdateType
	{
		UPDATE_NONE,
		UPDATE_MAX,
		UPDATE_MIN,
		UPDATE_REPLACE,
	};

	struct sCommodityStatus
	{
		unsigned long status;
		unsigned long myCount;
		unsigned long totalCount;
	};

	CCommodityStatusManager();
	virtual ~CCommodityStatusManager();

	unsigned long GetStatus(unsigned short index) const;
	unsigned long GetMyCount(unsigned short index) const;
	unsigned long GetTotalCount(unsigned short index) const;
	eUpdateType GetUpdateType(unsigned short index) const;
	void SetStatus(unsigned short index, unsigned long status);
	void SetMyCount(unsigned short index, unsigned long count);
	void SetTotalCount(unsigned short index, unsigned long count);
	void SetData(unsigned short index, unsigned long status,
		unsigned long myCount, unsigned long totalCount);
	void UpdateTotalCount(unsigned short index, unsigned long count);
	void UpdateStatus(unsigned short index, unsigned long status);
	void UpdateMyCount(unsigned short index, unsigned long count);

protected:
	void InitializeStatus();

	sCommodityStatus* m_pStatus;
	eUpdateType m_updateType[MAX_COMMODITY_COUNT];
};

class CCommodityButton
{
public:
	enum eCommodityButtonType
	{
		BUTTON_GIFT1,
		BUTTON_GIFT2,
		BUTTON_CONTRIBUTION,
		BUTTON_NONE,
	};

	CCommodityButton();
	virtual ~CCommodityButton();

	eCommodityButtonType GetButtonType() const;
	void ChangeButtonType(eCommodityButtonType type);
	void ButtonSwitch(bool bEnable);
	bool IsVisible() const;
	bool BindButton(FrButton* pButton);
	void SetCover(const char* name);
	void DrawCover();

protected:
	eCommodityButtonType m_buttonType;
	FrButton* m_pButton;
	const Bitmap* m_pCover;
	WPoint m_coverPos;
};

class CCommoditySlot : public CCommodityButton
{
public:
	static const int MAX_COMMODITY_SLOT_COUNT = 10;

	enum eSlotStatus
	{
		SLOT_ENABLE,
		SLOT_SOLDOUT,
		SLOT_RECEIVED,
		SLOT_DISABLE,
	};

	struct sCommoditySlotData
	{
		unsigned short slotIndex;
		unsigned long countFlag;
		unsigned long markFlag;
		const char* soldOutMark;
		const char* giftMark;
		_WPOINT myCountPos;
		_WPOINT totalCountPos;
		_WPOINT markPos;
		FrButton* pButton;
		const char* cover;
		CCommodityButton::eCommodityButtonType buttonType;
	};

	CCommoditySlot();
	virtual ~CCommoditySlot();

	void InitSlot(sCommoditySlotData& data);
	void SetMyCount(unsigned long count);
	void SetTotalCount(unsigned long count);
	void SetSlotStatus(eSlotStatus status);
	unsigned short GetSlotIndex() const;
	eSlotStatus GetSlotStatus() const;
	WRect GetButtonRect() const;
	void AddSlotLink(const CCommoditySlot* pSlot);
	void Process();
	void DrawSlot(WPoint pos);

protected:
	unsigned short m_slotIndex;
	eSlotStatus m_slotStatus;
	unsigned long m_countFlag;
	unsigned long m_markFlag;
	const Bitmap* m_pSoldOutMark;
	const Bitmap* m_pGiftMark;
	unsigned long m_myCount;
	unsigned long m_totalCount;
	WPoint m_myCountPos;
	WPoint m_totalCountPos;
	WPoint m_markPos;
	unsigned short m_linkCount;
	const CCommoditySlot* m_pLink[MAX_COMMODITY_SLOT_COUNT];
};

class FrPointEventNoticeDlg : public FrForm
{
	DECLARE_OBJECT(FrPointEventNoticeDlg)

	FrPointEventNoticeDlg();
	virtual ~FrPointEventNoticeDlg();

	virtual bool OnInit();

	void SetCommodity(int index, unsigned long type);

protected:
	void OnInitBtnYes(int param);
	void OnInitBtnNo(int param);
	void OnLBDownBtnYes();
	void OnLBDownBtnNo();
	void OnInitDrawArea(int param);
	void OnOwnerDrawDrawArea(int param);

	FrButton* m_pBtnYes;
	FrButton* m_pBtnNo;
	FrArea* m_pDrawArea;
	int m_commodityIndex;
	const Bitmap* m_pItemImage;
	unsigned long m_type;

	DECLARE_FRESH_MSGMAP()
};

class FrPointEventDescDlg : public FrForm
{
	DECLARE_OBJECT(FrPointEventDescDlg)

	FrPointEventDescDlg();
	virtual ~FrPointEventDescDlg();

	void SetDescImage(const char* filename);

protected:
	void OnInitDescViewer(int param);

	FrViewer* m_pDescViewer;
	char m_szDescImage[64];

	DECLARE_FRESH_MSGMAP()
};

class FrPointEventDlg : public FrForm
{
	DECLARE_OBJECT(FrPointEventDlg)

	static const int MAX_COMMODITY_SLOT_COUNT = 10;

	FrPointEventDlg();
	virtual ~FrPointEventDlg();

	virtual bool OnInit();
	virtual void OnProc(const float delta);

	bool OnPointEventDescDlgResult(int result, FrForm* pForm);
	bool OnPointEventNoticeDlgResult(int result, FrForm* pForm);
	void Confirm(unsigned long index);
	void ResultMessage(unsigned long result);
	void UnSelectCommodity();
	void SendUpdateDataPacket();
	void LocalDataUpdate(unsigned short index);

protected:
	void OnInitDescButton(int param);
	void OnLBDownDescButton();
	void OnInitBtnCommodity01(int param);
	void OnLBDownBtnCommodity01();
	void OnInitBtnCommodity02(int param);
	void OnLBDownBtnCommodity02();
	void OnInitBtnCommodity03(int param);
	void OnLBDownBtnCommodity03();
	void OnInitBtnCommodity04(int param);
	void OnLBDownBtnCommodity04();
	void OnInitBtnCommodity05(int param);
	void OnLBDownBtnCommodity05();
	void OnInitBtnCommodity06(int param);
	void OnLBDownBtnCommodity06();
	void OnInitBtnCommodity07(int param);
	void OnLBDownBtnCommodity07();
	void OnInitBtnCommodity08(int param);
	void OnLBDownBtnCommodity08();
	void OnInitBtnCommodity09(int param);
	void OnLBDownBtnCommodity09();
	void OnOwnerDrawDataArea(int param);
	void OnOwnerDrawContribution(int param);
	void OnInitBtnDescContribution(int param);
	void OnLBDownBtnDescContribution();
	void OnInitBtnContribution(int param);
	void OnLBDownBtnContribution();

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	static const int m_iMaxContribution = 200000;
	static const int m_iPangDouble2 = 120000;
	static const int m_iExpDouble = 70000;
#endif
	static const int m_iPangDouble1 = 30000;
#ifndef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	static const int m_iExpDouble = 70000;
	static const int m_iPangDouble2 = 120000;
	static const int m_iMaxContribution = 200000;
#endif

	float m_elapsed;
	FrButton* m_pDescButton;
	FrPointEventDescDlg* m_pDescDlg;
	FrPointEventNoticeDlg* m_pNoticeDlg;
	unsigned long m_myPoint;
	unsigned short m_selectCommodity;
	FrButton* m_pBtnCommodity[MAX_COMMODITY_SLOT_COUNT];
	CCommoditySlot m_commoditySlot[MAX_COMMODITY_SLOT_COUNT];
	float m_contribution;
	FrButton* m_pBtnDescContribution;
	const Bitmap* m_pContributionGage;
	const Bitmap* m_pContributionGageBg;
	const Bitmap* m_pContributionFrame[2];
	const Bitmap* m_pIconSuccess;

public:
	unsigned long GetMyPoint() const { return m_myPoint; }
	void SetMyPoint(unsigned long point) { m_myPoint = point; }

	DECLARE_FRESH_MSGMAP()
};
