#pragma once

#include "actor.h"

class CSpin : public IActor
{
public:
	virtual ~CSpin() { }

	DECLARE_OBJECT(CSpin)

	virtual void OnPreLoadInit();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void HandleMsg(const MsgObject& msg);

protected:
	CSpin();

	void Reset();
	void GetNextDirection();
	void ProcessLoopHitPoint(float delta, float& x, float& y, float& z);
	bool IsPointInAutoPoint();
	void ProcessAutoPoint(float delta);
	bool IsInclude(float x, float y);

	float m_hitX;
	float m_hitY;
	float m_rangeX;
	float m_rangeY;
	float m_centerX;
	float m_centerY;
	float m_angle;
	unsigned long m_pad983_34; // +0x34: sem leitura/escrita em CSpin no 983 (0079a230..0079bb00)
	unsigned long m_color;
	float m_autoX;
	float m_autoY;
	bool m_bActive;
	bool m_bAutoPoint;
	bool m_bClicked;
	bool m_bLoop;
	int m_direction;
	int m_nextDirection;
	int m_loopType;
	bool m_bUnused;
	bool m_bLock;
};
