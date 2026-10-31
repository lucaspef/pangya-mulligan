#pragma once

class WSliceStr
{
public:
	WSliceStr(const char* str) { m_str = str; }

	bool Get(const char* key, const char* format, ...);

private:
	const char* m_str;
};
