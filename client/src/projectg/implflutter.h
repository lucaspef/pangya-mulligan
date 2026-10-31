#pragma once

#include <string>
#include "weather.h"
#include "petal.h"

// CWeatherType wrapper around ntFlutter; T supplies the particle file name
template <class T>
class ntImplFlutter : public CWeatherType
{
public:
	ntImplFlutter(int num, const char* sprName, int sprIndex);
	virtual ~ntImplFlutter();

	virtual void Process(float dt);
	virtual void Display();
	virtual void DisplayFullScreenOverlay();
	virtual void Reset(int num, float rate);

private:
	T m_holder;
	ntFlutter m_flutter;
};

template <class T>
ntImplFlutter<T>::ntImplFlutter(int num, const char* sprName, int sprIndex)
	: m_flutter(num, m_holder.name, sprIndex)
{
}

template <class T>
ntImplFlutter<T>::~ntImplFlutter()
{
}

template <class T>
void ntImplFlutter<T>::Process(float dt)
{
	m_flutter.Process(dt, m_gravity);
}

template <class T>
void ntImplFlutter<T>::Display()
{
	m_flutter.Display();
}

template <class T>
void ntImplFlutter<T>::DisplayFullScreenOverlay()
{
	m_flutter.DisplayFullScreenOverlay();
}

template <class T>
void ntImplFlutter<T>::Reset(int num, float rate)
{
	m_flutter.Reset(num, rate);
}

struct sPetalNameHolder
{
	sPetalNameHolder()
		: name("pinkflower%02d.pet")
	{
	}

	std::string name;
};

struct sSilicaParticleNameHolder
{
	sSilicaParticleNameHolder()
		: name("Hell_powder%02d.pet")
	{
	}

	std::string name;
};
