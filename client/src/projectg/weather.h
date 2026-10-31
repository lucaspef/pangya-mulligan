#pragma once

class CWeatherType
{
public:
	CWeatherType()
		: m_gravity(1.0f)
	{
	}
	virtual ~CWeatherType() { }

	virtual void Process(float dt) = 0;
	virtual void Display() = 0;
	virtual void DisplayFullScreenOverlay() { }
	virtual void UpdateWind() { }
	virtual void Reset(int num, float rate) { }

	void SetGravity(float gravity) { m_gravity = gravity; }

protected:
	float m_gravity;
};

class CCloud : public CWeatherType
{
public:
	CCloud() { }
	virtual ~CCloud() { }
	virtual void Display();
	virtual void Process(float dt) { }
	virtual void DisplayFullScreenOverlay();
};
