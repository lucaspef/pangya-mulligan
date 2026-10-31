#pragma once

namespace Cm
{
	enum eErrorCode
	{
		ERROR_NONE = 0
	};
}

inline bool IS_SUCCESSFUL(Cm::eErrorCode code)
{
	return code == Cm::ERROR_NONE ? true : false;
}
