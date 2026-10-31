#pragma once

template <bool Value>
struct xtAlways
{
	static bool Get() { return Value; }
};
