#pragma once

#include "actor.h"

// Background actor of a golf course: loads the course geometry, sky, wave and
// river data for the current hole and handles the weather/sky messages.
class CGolfBg : public IActor
{
public:
	virtual ~CGolfBg() { }

	DECLARE_OBJECT(CGolfBg)

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

protected:
	CGolfBg();

	void DoReLoad();
	void DoInitAfterLoad();
	void SetStartPoint(const WVector& pos);
	void SetHolePoint(const WVector& pos);
	void SetGridCenter(WVector& pos);
	void SetSkyColor(unsigned long color);
	void SetLowestPoint();
	const char* GetSkyBoxName();
	void SetRenderWeather(unsigned char weather);
	unsigned char GetRenderWeather() { return m_renderWeather; }

	unsigned char m_renderWeather;
};
