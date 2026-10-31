#pragma once

#include <string>
#include <vector>
#include "gamath.h"
#include "polysoup.h"

class IObject;

namespace Ut
{
	void MakeObjectName(std::string& name, const char* prefix,
		const IObject& obj, int index);
	void BuildAppropriatePosList(
		const std::vector<WPolySoup::sTriangle*>* triangles, float interval,
		std::vector<WVector>& posList);
	char* GenDateTimeStrA(char* buf, const char* format);
	gaQ GetRotationQuaternion(const gaV3& a, const gaV3& b,
		const gaV3& fallbackAxis);

	struct Spatial
	{
		Spatial() { Reset(); }

		void Reset()
		{
			scl = 1.0f;
			rot = gaQ::ID;
			pos = gaV3::ZERO;
		}

		void ToMatrix(WMatrix& mat) const
		{
			WMatrix rotMat;
			((const WQuat&)rot).ConvertToRotationMatrix(rotMat);

			mat.Reset();
			mat *= scl;
			mat = mat * rotMat;

			mat.pivot = (const WVector&)pos;
		}

		float scl;
		gaQ rot;
		gaV3 pos;
	};
}
