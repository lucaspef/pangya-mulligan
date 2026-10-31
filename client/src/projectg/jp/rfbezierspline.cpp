// JP 983: RFBezierSpline.cpp da Refresh (B158, F3.7.13 item (6); 0042ee70–0042f540). Curva de "easeValue" de cada valor animado:
// Bézier cúbica de (0,0) a (1,1) amostrada em `resolution` pontos; GetValue(t) = y do primeiro ponto com x >= t (busca binária).
// 0042f3d0–0042f540 são do std::vector<RFVector2>.
#include <vector>
#include <algorithm>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfdirector.h"

// 0042ee70
RFBezierSpline::RFBezierSpline(int resolution, const RFVector2& control1, const RFVector2& control2)
	: _p1(control1), _p2(control2), _resolution(resolution)
{
	_p3 = RFVector2(1.0f, 1.0f);
	GenerateBezierPoints(_p0, _p1, _p2, _p3);
}

// 0042ef20
RFBezierSpline::~RFBezierSpline()
{
}

// 0042ef80
float RFBezierSpline::GetValue(float t) const
{
	int count = (int)_points.size();
	int low = 0;
	int high = count - 1;
	while (low <= high)
	{
		int middle = (high + low) / 2;
		int one = 1;
		float previous = _points[(std::max)(middle, one) - 1].x;
		float current = _points[middle].x;
		if (previous <= t && t <= current)
			return _points[middle].y;
		if (current <= t)
			low = middle + 1;
		else
			high = middle - 1;
	}
	return _points[count - 1].y;
}

// 0042f050
void RFBezierSpline::GenerateBezierPoints(const RFVector2& p0, const RFVector2& p1, const RFVector2& p2, const RFVector2& p3)
{
	_points.clear();
	for (int i = 0; i < _resolution; ++i)
		_points.push_back(GetBezierPoint((float)i / (float)_resolution, p0, p1, p2, p3));
}

// 0042f1e0
RFVector2 RFBezierSpline::GetBezierPoint(float t, const RFVector2& p0, const RFVector2& p1, const RFVector2& p2, const RFVector2& p3) const
{
	float u = 1.0f - t;
	float b0 = u * u * u;
	float b1 = (float)(3.0 * t * u * u);
	float b2 = (float)(t * 3.0 * t * u);
	float b3 = t * t * t;
	return RFVector2(b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x, b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y);
}
