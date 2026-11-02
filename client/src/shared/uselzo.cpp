#include "minatl.h"
#include "uselzo.h"

CCompressBuffer::CCompressBuffer()
{
	Init();
}

CCompressBuffer::~CCompressBuffer()
{
	if (m_Buffer)
	{
		delete[] m_Buffer;
		m_Buffer = NULL;
	}
}

void CCompressBuffer::Init()
{
	m_bInitialized = LZO1X_OK;
	m_bBufferUsed = false;
	m_iSize = 0;
	m_Buffer = NULL;
}

int CCompressBuffer::Compress(unsigned char* pSrc, int srcSize)
{
	if (m_bBufferUsed)
	{
		m_iSize = 0;
		return 1;
	}

	if (srcSize == 0)
	{
		return 3;
	}

	if (pSrc == NULL)
	{
		return 2;
	}

	m_bBufferUsed = true;

	if (m_Buffer)
	{
		delete[] m_Buffer;
		m_Buffer = NULL;
	}

	m_Buffer = new unsigned char[Max(MAX_BUF_LEN, srcSize * 2)];
	memset(m_Buffer, 0, Max(MAX_BUF_LEN, srcSize * 2));

	int size = srcSize;
	for (int i = 0; i < eHeaderLen - 1; i++)
	{
		m_Buffer[eHeaderLen - 1 - i] = size % 255;
		size = (size - m_Buffer[eHeaderLen - 1 - i]) / 255;
	}

	if (m_bInitialized != LZO1X_OK)
	{
		m_Buffer[0] = eNotCompressed;
		for (int i = 0; i < srcSize; i++)
		{
			m_Buffer[i + eHeaderLen] = pSrc[i];
		}

		m_iSize = srcSize + eHeaderLen;
		return 4;
	}

	m_Buffer[0] = eCompressed;

	size_t uDestLen = 0;
	int res = Lzo1xCompress(pSrc, srcSize, m_Buffer + eHeaderLen,
		Max(MAX_BUF_LEN, srcSize * 2) - eHeaderLen, &uDestLen);
	if (res != LZO1X_OK)
	{
		char log[128];
		sprintf(log, "::COMPRESS -> The result should be zero! (res : %d)",
			res);
	}

	m_iSize = uDestLen + eHeaderLen;
	return 0;
}

int CCompressBuffer::Decompress(unsigned char* pSrc, int srcSize)
{
	if (m_bBufferUsed)
	{
		m_iSize = 0;
		return 1;
	}

	if (pSrc == NULL)
	{
		return 2;
	}

	if (srcSize < eHeaderLen)	// E489: curto lia cabecalho fora; tamanho negativo ia ao lzo
	{
		return 3;
	}

	int WrittenOrgSize = 0;
	int mul = 1;
	for (int i = 0; i < eHeaderLen - 1; i++)
	{
		WrittenOrgSize += pSrc[eHeaderLen - 1 - i] * mul;
		mul *= 255;
	}

	m_bBufferUsed = true;

	if (m_Buffer)
	{
		delete[] m_Buffer;
		m_Buffer = NULL;
	}

	const int capacity = Max(WrittenOrgSize + 10, MAX_BUF_LEN);
	m_Buffer = new unsigned char[capacity];
	memset(m_Buffer, 0, capacity);

	if (pSrc[0] == eNotCompressed)
	{
		// E489: confere antes de copiar; cabecalho menor que o corpo estourava o heap
		if (WrittenOrgSize != srcSize - eHeaderLen)
		{
			m_iSize = 0;
			return 5;
		}

		memcpy(m_Buffer, pSrc + eHeaderLen, WrittenOrgSize);
		m_iSize = WrittenOrgSize;
		return 0;
	}
	else if (pSrc[0] == eCompressed)
	{
		if (m_bInitialized != LZO1X_OK)
		{
			m_iSize = 0;
			return 7;
		}

		size_t uDestLen = 0;
		int res = Lzo1xDecompress(pSrc + eHeaderLen, srcSize - eHeaderLen,
			m_Buffer, capacity, &uDestLen);	// E489: limita a saida ao que o pacote pode escrever
		if (res != LZO1X_OK)
		{
			char log[128];
			sprintf(log,
				"::DECOMPRESS -> The result should be zero! (res : %d)", res);
			return 8;
		}

		if (WrittenOrgSize == uDestLen)
		{
			m_iSize = uDestLen;
		}
		else
		{
			m_iSize = 0;
			return 6;
		}

		return 0;
	}
	else
	{
		return 9;
	}
}

unsigned char* CCompressBuffer::GetNewBuffer()
{
	if (m_iSize == 0)
	{
		return NULL;
	}
	else
	{
		return m_Buffer;
	}
}

int CCompressBuffer::GetNewSize()
{
	return m_iSize;
}
