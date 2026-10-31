#pragma once

#include "lzo1x.h"

#define IN_LEN (128 * 1024ul)
#define OUT_LEN (IN_LEN + IN_LEN / 16 + 64 + 3)

class CCompressBuffer
{
public:
	CCompressBuffer();
	~CCompressBuffer();

	void Init();
	int Compress(unsigned char* pSrc, int srcSize);
	int Decompress(unsigned char* pSrc, int srcSize);
	unsigned char* GetNewBuffer();
	int GetNewSize();

	enum
	{
		MAX_BUF_LEN = 4096
	};
	enum
	{
		eHeaderLen = 4
	};
	enum
	{
		eCompressed,
		eNotCompressed
	};

private:
	int Max(int a, int b) { return a > b ? a : b; }

	unsigned char* m_Buffer;
	bool m_bBufferUsed;
	int m_bInitialized;
	int m_iSize;
};
