#pragma once

inline int cToken::GetPos() const
{
	return m_Pos;
}

inline int cToken::GetLen() const
{
	return m_Len;
}

// HACK: out-of-line noticeboard usage
__declspec(noinline) inline bool cToken::GetToken(char** ppOut,
	const char* pcSep, int nSep)
{
	if (!GetResultBuf())
	{
		if (ppOut)
		{
			*ppOut = 0;
		}
		return false;
	}

	if (ppOut)
	{
		*ppOut = m_pcResBuf;
	}
	return GetToken(m_pcResBuf, pcSep, nSep);
}

inline bool cToken::GetTokenWithoutFail(char* const pOut, const char* pcSep,
	int nSep)
{
	bool bRet = false;
	*pOut = 0;
	while (m_Pos < m_Len)
	{
		bRet = GetToken(pOut, pcSep, nSep);
		if (*pOut)
			break;
	}
	return bRet;
}

inline bool cToken::GetTokenWithoutFail(char** ppOut, const char* pcSep,
	int nSep)
{
	if (!GetResultBuf())
	{
		if (ppOut)
			*ppOut = 0;
		return false;
	}
	if (ppOut)
		*ppOut = m_pcResBuf;
	return GetTokenWithoutFail(m_pcResBuf, pcSep, nSep);
}

inline bool cToken::GetTokenFullSep(char** ppOut, const char* pcSep, int nSep)
{
	if (!GetResultBuf())
	{
		if (ppOut)
		{
			*ppOut = 0;
		}
		return false;
	}

	if (ppOut)
	{
		*ppOut = m_pcResBuf;
	}
	return GetTokenFullSep(m_pcResBuf, pcSep, nSep);
}

inline const char* cTokenV::GetBuf() const
{
	return m_pcBuf;
}

inline const char* cTokenS::GetBuf() const
{
	return m_pcBuf;
}

inline int PosB(const char* str, int len, const char* sub, int sublen)
{
	for (int i = 0; i < len - sublen + 1; ++i)
	{
		for (int j = 0;; ++j)
		{
			if (j >= sublen)
				return i;
			if (str[i + j] != sub[j])
				break;
		}
		if (Custom_IsLeadByte(str[i]))
			++i;
	}
	return -1;
}
