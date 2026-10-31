#pragma once

#include <string.h>
#include <string>
#include <vector>
#include <list>
#include "netsyncactor.h"
#include "util.h"
#include "scenemanager.h"
#include "polysoup.h"

class GroundItem;
class CGimmickContainer;

class GroundItemMan : public NetSyncActor, public CRenderFuncPtr
{
	friend class GroundItem;

public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	GroundItemMan();
	virtual ~GroundItemMan();

	virtual void OnLoad();
	virtual void OnReload();
	virtual void OnInit();
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

	struct PlayerStats
	{
		PlayerStats() { Reset(); }

		void Reset() { memset(this, 0, sizeof(PlayerStats)); }

		int numAcquired[3];
		int numAcquiredLive[3];
	};

	virtual void NSC_OnStep(bool bForce, bool bReplay, float step, float time,
		int ballState);
	virtual void NSC_OnFirstNewShotFrame(float time, int ballState);
	virtual void NSC_OnFirstReplayFrame(int ballState);
	virtual void OnHoleOut();

	virtual void Display();
	virtual void DebugDisplay();

	// 983 0052d0dd le +0xa34 (VC10); no cl 14.44 m_pPlayerStats fica em 0xa2c (std::list/std::string menores)
	// (offset a partir do IActor*: sem cast, que no VC10 poe checagem de nulo no chamador)
	static size_t PlayerStatsOfs983() { C_ASSERT_VC10(offsetof(GroundItemMan, m_pPlayerStats) == 0xa34); return offsetof(GroundItemMan, m_pPlayerStats); }

protected:
	// 983: vt+0x44 (0085d6c0), +0x48 (0085e450), +0x4c (0085d780); no lugar de DisposeGroundItem/FindUnusedDisposeInfo do 645
	virtual int PlaceAllGroundItems();
	virtual void PlaceGroundItem(GimmickDispositionInformation* pInfo,
		const gaV3* pPos, FieldItem::eDisposeTextureType texType);
	virtual GimmickDispositionInformation* FindUnusedDisposeInfo(int texType);
	virtual GroundItem* CreateGroundItem(unsigned long id,
		GIMMICK_ITEM_TYPE type, FieldItem::eDisposeTextureType texType,
		const Ut::Spatial& spatial);
	void EnableGroundItemAll(bool bEnable);

private:
	static unsigned int ms_initCnt;

	std::list<GroundItem*> m_groundItemList;
	CGimmickContainer* m_pGimmickContainer;
	std::string m_sceneElemName;
	unsigned char m_enabled983[4]; // 983 +0xa30: item de campo ligado por jogador/time (msgs 0x2f5/0x2f6); o construtor nao os toca
	PlayerStats* m_pPlayerStats; // 983: 0xa34 (0085d650); sizeof 0xa38
};
