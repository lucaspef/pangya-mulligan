#pragma once

class CInitDelegate
{
public:
	typedef void (*FUNC)();

	static void AddInitFunction(FUNC func);
	static void AddReleaseFunction(FUNC func);
	static void ExecuteInitFunctions();
	static void ExecuteReleaseFunctions();

private:
	static FUNC m_InitFunctions[256];
	static FUNC m_ReleaseFunctions[256];
	static int m_numInitFunctions;
	static int m_numReleaseFunctions;
};
