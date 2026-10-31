#pragma once

template <class T>
inline T Max(T a, T b)
{
	return (a < b) ? b : a;
}

template <class T>
inline T Min(T a, T b)
{
	return (a > b) ? b : a;
}
