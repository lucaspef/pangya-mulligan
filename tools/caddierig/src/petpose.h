// SPDX-License-Identifier: MIT
// petpose.h - key evaluation, same search-and-interpolate as WBone::GetKeyPos/GetKeyRot.
#ifndef PETPOSE_H
#define PETPOSE_H

#include "petfile.h"

inline WVector EvalKeyPos(const std::vector<PetKeyPos>& keys, float time, const WVector& rest)
{
	if (keys.empty())
		return rest;
	if (time <= keys[0].time)
		return keys[0].v;
	if (time >= keys.back().time)
		return keys.back().v;
	size_t low = 0, high = keys.size() - 1;
	while (high - low > 1)
	{
		size_t mid = (low + high) / 2;
		if (keys[mid].time > time)
			high = mid;
		else
			low = mid;
	}
	float span = keys[high].time - keys[low].time;
	float t = span > 0.0f ? (time - keys[low].time) / span : 0.0f;
	return keys[low].v + (keys[high].v - keys[low].v) * t;
}

inline WQuat EvalKeyRot(const std::vector<PetKeyRot>& keys, float time, const WQuat& rest)
{
	if (keys.empty())
		return rest;
	if (time <= keys[0].time)
		return keys[0].q;
	if (time >= keys.back().time)
		return keys.back().q;
	size_t low = 0, high = keys.size() - 1;
	while (high - low > 1)
	{
		size_t mid = (low + high) / 2;
		if (keys[mid].time > time)
			high = mid;
		else
			low = mid;
	}
	float span = keys[high].time - keys[low].time;
	float t = span > 0.0f ? (time - keys[low].time) / span : 0.0f;
	return WSlerp(keys[low].q, keys[high].q, t);
}

#endif // PETPOSE_H
