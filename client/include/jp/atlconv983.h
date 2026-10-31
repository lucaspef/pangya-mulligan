#pragma once
#include <stdlib.h>

// JP 983: conversores de texto da ATL 10 (atlconv.h do VS2010), que o 983 usa; o build tem só a ATL 7.1 (build/atl71, sem
// AtlConvAllocMemory nem a nova tentativa no ERROR_INSUFFICIENT_BUFFER). Cópia com nomes próprios para não colidir com a 7.1.
// Corpos fora de linha no 983 (unidade jp/textconv.cpp, 005e49c0–005e51f0): AtlConvAllocMemory<char> 005e4a20,
// <wchar_t> 005e4ad0; CW2AEX983<128>: ctor 005e4c50, Init 005e4b80, dtor 005e4ab0; CA2WEX983<128>: ctor 005e4d30, Init 005e4c80,
// dtor 005e4b60. AtlThrow / AtlThrowLastWin32 são os da ATL 7.1 (005e49c0 / 005e49e0, iguais nas duas versões).
// ATL do toolset novo: AtlThrow e macro (atldef.h: ATL::AtlThrowImpl), ATL::AtlThrow nao compila; no VC10 = ATL 7.1, igual.
#if _MSC_VER > 1600
#define ATL983_THROW(hr) AtlThrow(hr)
#else
#define ATL983_THROW(hr) ATL::AtlThrow(hr)
#endif

namespace atl983
{
template <class _CharType>
inline void AtlConvAllocMemory(_CharType** ppBuff, int nLength, _CharType* pszFixedBuffer, int nFixedBufferLength)
{
	if (ppBuff == NULL)
		ATL983_THROW(E_INVALIDARG);
	if (nLength < 0)
		ATL983_THROW(E_INVALIDARG);
	if (pszFixedBuffer == NULL)
		ATL983_THROW(E_INVALIDARG);

	// buffer já alocado: realoca
	if (*ppBuff != pszFixedBuffer)
	{
		if (nLength > nFixedBufferLength)
		{
			_CharType* ppReallocBuf = static_cast<_CharType*>(_recalloc(*ppBuff, nLength, sizeof(_CharType)));
			if (ppReallocBuf == NULL)
				ATL983_THROW(E_OUTOFMEMORY);
			*ppBuff = ppReallocBuf;
		}
		else
		{
			free(*ppBuff);
			*ppBuff = pszFixedBuffer;
		}
	}
	else
	{
		if (nLength > nFixedBufferLength)
			*ppBuff = static_cast<_CharType*>(calloc(nLength, sizeof(_CharType)));
		else
			*ppBuff = pszFixedBuffer;
	}

	if (*ppBuff == NULL)
		ATL983_THROW(E_OUTOFMEMORY);
}

template <class _CharType>
inline void AtlConvFreeMemory(_CharType* pBuff, _CharType* pszFixedBuffer, int nFixedBufferLength)
{
	(nFixedBufferLength);
	if (pBuff != pszFixedBuffer)
		free(pBuff);
}

template <int t_nBufferLength = 128>
class CW2AEX983
{
public:
	CW2AEX983(LPCWSTR psz, UINT nCodePage) throw(...) : m_psz(m_szBuffer)
	{
		Init(psz, nCodePage);
	}
	~CW2AEX983() throw()
	{
		AtlConvFreeMemory(m_psz, m_szBuffer, t_nBufferLength);
	}

	operator LPSTR() const throw()
	{
		return m_psz;
	}

private:
	void Init(LPCWSTR psz, UINT nConvertCodePage) throw(...)
	{
		if (psz == NULL)
		{
			m_psz = NULL;
			return;
		}
		int nLengthW = lstrlenW(psz) + 1;
		int nLengthA = nLengthW * 4;

		AtlConvAllocMemory(&m_psz, nLengthA, m_szBuffer, t_nBufferLength);

		BOOL bFailed = (0 == ::WideCharToMultiByte(nConvertCodePage, 0, psz, nLengthW, m_psz, nLengthA, NULL, NULL));
		if (bFailed)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				nLengthA = ::WideCharToMultiByte(nConvertCodePage, 0, psz, nLengthW, NULL, 0, NULL, NULL);
				AtlConvAllocMemory(&m_psz, nLengthA, m_szBuffer, t_nBufferLength);
				bFailed = (0 == ::WideCharToMultiByte(nConvertCodePage, 0, psz, nLengthW, m_psz, nLengthA, NULL, NULL));
			}
		}
		if (bFailed)
			ATL::AtlThrowLastWin32();
	}

public:
	LPSTR m_psz;
	char m_szBuffer[t_nBufferLength];

private:
	CW2AEX983(const CW2AEX983&) throw();
	CW2AEX983& operator=(const CW2AEX983&) throw();
};

template <int t_nBufferLength = 128>
class CA2WEX983
{
public:
	CA2WEX983(LPCSTR psz, UINT nCodePage) throw(...) : m_psz(m_szBuffer)
	{
		Init(psz, nCodePage);
	}
	~CA2WEX983() throw()
	{
		AtlConvFreeMemory(m_psz, m_szBuffer, t_nBufferLength);
	}

	operator LPWSTR() const throw()
	{
		return m_psz;
	}

private:
	void Init(LPCSTR psz, UINT nCodePage) throw(...)
	{
		if (psz == NULL)
		{
			m_psz = NULL;
			return;
		}
		int nLengthA = lstrlenA(psz) + 1;
		int nLengthW = nLengthA;

		AtlConvAllocMemory(&m_psz, nLengthW, m_szBuffer, t_nBufferLength);

		BOOL bFailed = (0 == ::MultiByteToWideChar(nCodePage, 0, psz, nLengthA, m_psz, nLengthW));
		if (bFailed)
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				nLengthW = ::MultiByteToWideChar(nCodePage, 0, psz, nLengthA, NULL, 0);
				AtlConvAllocMemory(&m_psz, nLengthW, m_szBuffer, t_nBufferLength);
				bFailed = (0 == ::MultiByteToWideChar(nCodePage, 0, psz, nLengthA, m_psz, nLengthW));
			}
		}
		if (bFailed)
			ATL::AtlThrowLastWin32();
	}

public:
	LPWSTR m_psz;
	wchar_t m_szBuffer[t_nBufferLength];

private:
	CA2WEX983(const CA2WEX983&) throw();
	CA2WEX983& operator=(const CA2WEX983&) throw();
};
}
