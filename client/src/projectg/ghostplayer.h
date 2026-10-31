#pragma once
#include <list>
#include "actor.h"

class CGhostDocument;
class CGhostHoleData;
class CSequenceStateMachine;
class CStateUnit;
class FrForm;
class FrGhostAward;
class WOverlay;
struct GHOST_ACT_DATA;
struct GHOST_SHOT_DATA;

class CGhostPlayer : public IActor, public FrCmdTarget
{
public:
	class cMovieStyleDisplay
	{
	public:
		cMovieStyleDisplay();
		~cMovieStyleDisplay();

		void ResetHeight();
		void Process(float delta);
		void Render();

	private:
		float m_height;
		WOverlay* m_pOverlay;
	};

	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; }

	CGhostPlayer();
	virtual ~CGhostPlayer();

	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnReload();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void OnDestroy();

protected:
	virtual void HandleMsg(const MsgObject& msg);

private:
	CStateUnit* CreateStateUnit(int type, const GHOST_ACT_DATA* pAct);
	bool DoShot(const GHOST_SHOT_DATA* pShot);
	void SetWindAndWeather(unsigned char holeNum);
	void UseItem(unsigned long typeId, unsigned long itemId);
	void SendAwardResult();
	bool ExceptionAlarmWaitCallBack(int result, FrForm* pForm);

	CGhostDocument* m_pGhostDoc;
	const CGhostHoleData* m_pHoleData;
	std::list<GHOST_SHOT_DATA*> m_shotList;
	CSequenceStateMachine* m_pStateMachine;
	FrGhostAward* m_pAward;
	cMovieStyleDisplay* m_pMovieStyle;
	int m_totalShotCount;
	unsigned int m_shotIndex;
	int m_errorCode;
	int m_bShotPlaying;
	int m_bAwardSent;
	unsigned long m_serverProperty;
};
