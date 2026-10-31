#pragma once

#include <string>
#include "weather.h"

class CPetBody;

class ntFlutter
{
public:
	ntFlutter(int num, const std::string& petName, int type);
	~ntFlutter();

	virtual void Process(float dt, float gravity);
	virtual void Display();
	virtual void DisplayFullScreenOverlay();
	virtual void Reset(int num, float rate);

	struct sFrag
	{
		WVector pos;
		WVector center;
		unsigned long pad18; // +0x18: 006f9600 Process/Reset nao tocam
		WVector offset;
		float yaw;
		float swing;
		bool bVisible;
	};

private:
	void ReleasePets();

	static const int MAX_FLUTTER_PET;

	float m_rate;
	Waabb m_area;
	sFrag* m_frag;
	CPetBody* m_pet[10];
	int m_num;
	const std::string& m_petName;
};
