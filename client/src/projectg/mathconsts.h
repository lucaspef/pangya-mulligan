#pragma once

namespace
{
	inline bool IsValidRange(float f)
	{
		return f < g_HUGE && f > g_EPSILON;
	}
}
