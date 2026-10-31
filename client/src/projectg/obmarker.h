#pragma once

#include <vector>
#include "wmath.h"

class W3dSpr;

class CMarker
{
public:
	CMarker();
	virtual ~CMarker();

	void Reset();
	void AddList(const std::vector<WVector>& points);
	void Display();
	void GetReady();

private:
	std::vector<WVector> m_points;
	W3dSpr* m_sprite;
};
