#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>
#include "../shared/localize.h"
#include "icontentsdoc.h"

class CGiftBoxEvent : public IContentsDataContainer
{
public:
	void SetOpenedBoxTID(unsigned long tid);
	unsigned long GetOpenedBoxTID();

	virtual bool Initialize();
	virtual bool Release();

protected:
	unsigned long m_openedBoxTID;
};

class CWorldTourEvent : public IContentsDataContainer
{
public:
	void SetMapFlag(unsigned long flag);
	void SetGotGift(bool bGot);
	void SetCanGift(bool bCan);
	unsigned long GetMapFlag() const;
	bool GetGotGift() const;
	bool GetCanGift() const;

	virtual bool Initialize();
	virtual bool Release();

protected:
	unsigned long m_mapFlag;
	bool m_bGotGift;
	bool m_bCanGift;
};

class CHalloweenEvent : public IContentsDataContainer
{
public:
	void SetMapFlag(unsigned long flag);
	void SetGiftCnt(unsigned long cnt);
	void SetShowGiftDlg(bool bShow);
	void SetEventState(bool bState);
	unsigned long GetMapFlag() const;
	unsigned long GetGiftCnt() const;
	bool GetShowGiftDlg() const;
	bool GetEventState() const;

	virtual bool Initialize();
	virtual bool Release();

protected:
	unsigned long m_mapFlag;
	unsigned long m_giftCnt;
	bool m_bShowGiftDlg;
	bool m_bEventState;
};

class CEscapeBeginnerEvent : public IContentsDataContainer
{
public:
	void SetFlag(bool bFlag);
	bool GetFlag() const;

	virtual bool Initialize();
	virtual bool Release();

protected:
	bool m_bFlag;
};

class CPassingTitleChk : public IContentsDataContainer
{
public:
	void SetFlag(bool bFlag);
	bool GetFlag() const;

	virtual bool Initialize();
	virtual bool Release();

protected:
	bool m_bFlag;
};

class CChristmasEvent : public IContentsDataContainer
{
public:
	void SetFlag(bool bFlag);
	bool GetFlag() const;

	virtual bool Initialize();
	virtual bool Release();

protected:
	bool m_bFlag;
};

class CTicketExchange : public IContentsDataContainer
{
public:
	void SetFlag(bool bFlag);
	bool GetFlag() const;

	virtual bool Initialize();
	virtual bool Release();

protected:
	bool m_bFlag;
};

class CMissionEvent : public IContentsDataContainer
{
public:
	virtual bool Initialize();
	virtual bool Release();

	void SetEventFlag(unsigned int flag);
	unsigned int GetEventFlag() const;
	void DecodeMissionInfo(WReceivedPacket& packet);

	bool IsCompleteMission(int index);
	bool IsCompleteAllDayMission();
	bool IsCompleteAllTermMission();
	bool IsCompleteCourse(int course);

	void SetDayGiftFlag(bool bFlag) { m_bDayGiftFlag = bFlag; }
	bool GetDayGiftFlag() const { return m_bDayGiftFlag; }
	void SetTermGiftFlag(bool bFlag) { m_bTermGiftFlag = bFlag; }
	bool GetTermGiftFlag() const { return m_bTermGiftFlag; }

	int GetCondition(int index) { return m_missionInfo[index].condition; }
	const int GetMapCount() const { return 15; }

private:
	int GetMissionField(int index) const;
	void ClearMissionInfo() { m_missionInfo.clear(); }

	unsigned int m_eventFlag;
	bool m_bDayGiftFlag;
	bool m_bTermGiftFlag;
	std::vector<GlobalEnum::sMissionInfo> m_missionInfo;
};

class CPeriodContents : public IContentsDataContainer
{
public:
	struct sPeriodUnit
	{
		_SYSTEMTIME startTime;
		_SYSTEMTIME endTime;
		bool bNoEndTime;
		bool bUsable;
	};

	virtual bool Initialize();
	virtual bool Release();

	void AddUsableContents(int type, const _SYSTEMTIME* pStart,
		const _SYSTEMTIME* pEnd, bool bNoEndTime);
	bool IsUsableContents(int type);
	void Set_PreSettingContentsDate();
	void CheckAllContents();
	bool CheckContent(int type);

private:
	void IsUsable(sPeriodUnit& unit);

protected:
	std::map<int, sPeriodUnit> m_contentsMap;
};

#if !defined(REBANG_ICONTENTSDOC) && \
	!defined(CONTENTSDOC_DEFER_GIMMICK_CONTAINER)
#include "guildinfo.h"
#endif

class CChristmasSockEvent : public IContentsDataContainer
{
public:
	void SetFlag(bool bFlag);
	bool GetFlag() const;

	virtual bool Initialize();
	virtual bool Release();

protected:
	bool m_bFlag;
};

class CVectorSlideTestContainer : public IContentsDataContainer
{
public:
	virtual bool Initialize();
	virtual bool Release();

	void SetNewTypeVectorSlide(bool bNewType)
	{
		m_bNewTypeVectorSlide = bNewType;
	}
	void SetLimitSpeed(int speed) { m_limitSpeed = speed; }
	void SetBoundHeight(int height) { m_boundHeight = height; }
	void SetLimitVectorSlideCount(int count)
	{
		m_limitVectorSlideCount = count;
	}

	bool IsNewTypeVectorSlide() { return m_bNewTypeVectorSlide; }
	int GetLimitSpeed() { return m_limitSpeed; }
	int GetBoundHeight() { return m_boundHeight; }
	int GetLimitVectorSlideCount() { return m_limitVectorSlideCount; }

protected:
	bool m_bNewTypeVectorSlide;
	int m_limitSpeed;
	int m_boundHeight;
	int m_limitVectorSlideCount;
};

// rebang: CGimmickContainer lives in gimmickcontainer.h; contentsdoc.cpp/icontentsdoc.cpp include
// it after guildinfo.h (vector<AcquiredItemInfo> template order of the original)
#if !defined(REBANG_ICONTENTSDOC) && \
	!defined(CONTENTSDOC_DEFER_GIMMICK_CONTAINER)
#include "gimmickcontainer.h"
#endif
