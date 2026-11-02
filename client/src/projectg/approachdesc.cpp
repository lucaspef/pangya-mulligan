#include "minatl.h"
#include <stdio.h>
#include <string.h>
#include "approachdesc.h"
#include "commonutil.h"

// clang-format off
#define StrCopy(dst, src) \
	do \
	{ \
		const char* s_ = (src); \
		char* d_ = (dst); \
		char c_; \
		do \
		{ \
			c_ = *s_; \
			*d_ = c_; \
			s_++; \
			d_++; \
		} while (c_); \
	} while (0)
// clang-format on

// clang-format off
#define MISSION(n) \
	class Mission_##n : public DescBlock \
	{ \
	public: \
		Mission_##n(unsigned long p1, unsigned long p2, const char* p3) \
		{ \
			BuildDesc(p1, p2, p3); \
		} \
		virtual ~Mission_##n() \
		{ \
		} \
		virtual void BuildDesc(unsigned long p1, unsigned long p2, \
			const char* p3); \
	}; \
	void Mission_##n::BuildDesc(unsigned long p1, unsigned long p2, \
		const char* p3)
// clang-format on

MISSION(1)
{
	char sz[8] = "";
	if (p1 == 0)
		StrCopy(sz, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
	else
		StrCopy(sz, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("1\xc0\xa7 \xc0\xaf\xc0\xfa\xc0\xc7 \xba\xf1\xb0\xc5\xb8\xae\xb0\xa1 %s\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz));
}

MISSION(2)
{
	char sz[8] = "";
	if (p2 == 0)
		StrCopy(sz, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
	else
		StrCopy(sz, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("1~3\xc0\xa7\xc0\xc7 \xba\xf1\xb0\xc5\xb8\xae\xc0\xc7 \xc7\xd5\xb0\xe8\xb0\xa1 %s\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz));
}

MISSION(3)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("OUT(\xc4\xa8\xc0\xce)\xc0\xbb \xc7\xdf\xc0\xbb \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(4)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xc0\xfc\xbf\xf8\xc0\xcc 20\xc3\xca\xb0\xa1 \xc1\xf6\xb3\xaa\xb1\xe2 \xc0\xfc\xbf\xa1 \xc5\xb8\xb1\xb8\xb8\xa6 \xbf\xcf\xb7\xe1\xc7\xdf\xc0\xbb \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(5)
{
	char sz[8] = "";
	if (p1 == 0)
		StrCopy(sz, K2L_Compatibility("\xbf\xa9\xc0\xda"));
	else
		StrCopy(sz, K2L_Compatibility("\xb3\xb2\xc0\xda"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("1~3\xc0\xa7\xb0\xa1 \xc0\xfc\xba\xce %s(\xc4\xb3\xb8\xaf\xc5\xcd)\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz));
}

MISSION(6)
{
	char sz[16] = "";
	switch (p1)
	{
	case 1:
		StrCopy(sz, K2L_Compatibility("\xb1\xee\xb8\xc1\xc1\xa6\xb8\xae"));
		break;
	case 2:
		StrCopy(sz, K2L_Compatibility("\xc7\xcf\xbe\xe7\xc1\xa6\xb8\xae"));
		break;
	case 3:
		StrCopy(sz, K2L_Compatibility("\xba\xf4\xb8\xae"));
		break;
	case 4:
		StrCopy(sz, K2L_Compatibility("\xb6\xf3\xb9\xcc"));
		break;
	}
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("%s\xbf\xcd \xc7\xd4\xb2\xb2 \xc7\xc3\xb7\xb9\xc0\xcc\xc7\xd1 \xc0\xaf\xc0\xfa\xb0\xa1 1\xb5\xee\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz));
}

MISSION(7)
{
	sprintf(m_szDesc,
		K2L_Compatibility("%d\xc0\xa7\xb8\xa6 \xc7\xd1 \xc0\xaf\xc0\xfa\xb4\xc2 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
		p1);
}

MISSION(8)
{
	char sz[8] = "";
	if (p1 == 0)
		StrCopy(sz, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
	else
		StrCopy(sz, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("\xb8\xf0\xb5\xe7 \xc0\xaf\xc0\xfa\xc0\xc7 \xb3\xb2\xc0\xba \xb0\xc5\xb8\xae\xb0\xa1 %s\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz));
}

MISSION(9)
{
	char sz[8] = "";
	if (p1 == 0)
		StrCopy(sz, K2L_Compatibility("\xbf\xa9\xc0\xda"));
	else
		StrCopy(sz, K2L_Compatibility("\xb3\xb2\xc0\xda"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("1\xc0\xa7\xb0\xa1 %s(\xc4\xb3\xb8\xaf\xc5\xcd)\xc0\xcf \xb0\xe6\xbf\xec \xb8\xf0\xb5\xe7 %s(\xc4\xb3\xb8\xaf\xc5\xcd)\xb4\xc2 \xb9\xcc\xbc\xc7 \xbc\xba\xb0\xf8."),
			sz, sz));
}

// 983 004cf610: nome do personagem por indice (0..14; 13 e >14 nao escrevem)
static void __stdcall CharName983(char* dst, unsigned long idx)
{
	if (dst == 0)
		return;
	switch (idx)
	{
	case 0:
		StrCopy(dst, K2L_Compatibility("\xb4\xa9\xb8\xae"));
		break;
	case 1:
		StrCopy(dst, K2L_Compatibility("\xc7\xcf\xb3\xaa"));
		break;
	case 2:
		StrCopy(dst, K2L_Compatibility("\xbe\xc6\xc0\xfa"));
		break;
	case 3:
		StrCopy(dst, K2L_Compatibility("\xbc\xbc\xbd\xc7"));
		break;
	case 4:
		StrCopy(dst, K2L_Compatibility("\xb8\xc6\xbd\xba"));
		break;
	case 5:
		StrCopy(dst, K2L_Compatibility("\xc4\xed"));
		break;
	case 6:
		StrCopy(dst, K2L_Compatibility("\xbe\xc6\xb8\xb0"));
		break;
	case 7:
		StrCopy(dst, K2L_Compatibility("\xc4\xab\xc1\xee"));
		break;
	case 8:
		StrCopy(dst, K2L_Compatibility("\xb7\xe7\xbd\xc3\xbe\xc6"));
		break;
	case 9:
		StrCopy(dst, K2L_Compatibility("\xb3\xda"));
		break;
	case 10:
		StrCopy(dst, K2L_Compatibility("\xbd\xba\xc7\xc7\xc4\xab"));
		break;
	case 11:
		StrCopy(dst, K2L_Compatibility("\xb4\xa9\xb8\xae"));
		break;
	case 12:
		StrCopy(dst, K2L_Compatibility("\xc7\xcf\xb3\xaa"));
		break;
	case 14:
		StrCopy(dst, K2L_Compatibility("\xbc\xbc\xbd\xc7"));
		break;
	}
}

// 983 00854620: so o nome do personagem p2 (sem o texto de "10y")
MISSION(10)
{
	char sz[16] = "";
	CharName983(sz, p2);
	// 983 00854620
	StrCopy(m_szDesc,
		MakeStr(K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae\xb0\xa1 10y \xb9\xcc\xb8\xb8\xc0\xc7 \xb8\xf0\xb5\xe7 %s \xc0\xaf\xc0\xfa\xb4\xc2 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz));
}

MISSION(11)
{
	char sz[16] = "";
	if (p1 == 0)
	{
		StrCopy(sz, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
		StrCopy(sz + 8, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
	}
	else
	{
		StrCopy(sz, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
		StrCopy(sz + 8, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
	}
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae\xb0\xa1 %s\xc0\xce \xc0\xaf\xc0\xfa\xb0\xa1 %s\xc0\xce \xc0\xaf\xc0\xfa\xba\xb8\xb4\xd9 \xb8\xb9\xc0\xb8\xb8\xe9 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz, sz + 8));
}

MISSION(12)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xb0\xfa\xb9\xdd\xbc\xf6 \xc0\xcc\xbb\xf3\xc0\xcc OUT(\xc4\xa8\xc0\xce)\xc0\xbb \xc7\xdf\xc0\xbb \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(13)
{
	char sz[8] = "";
	if (p1 == 0)
		StrCopy(sz, K2L_Compatibility("\xbf\xa9\xc0\xda"));
	else
		StrCopy(sz, K2L_Compatibility("\xb3\xb2\xc0\xda"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae 1y\xc0\xcc\xb3\xbb\xc0\xc7 \xc0\xaf\xc0\xfa\xb0\xa1 %s(\xc4\xb3\xb8\xaf\xc5\xcd)\xc0\xce \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8"),
			sz));
}

MISSION(14)
{
	char sz[8] = "";
	switch (p1)
	{
	case 1:
		StrCopy(sz, K2L_Compatibility("1y"));
		break;
	case 2:
		StrCopy(sz, K2L_Compatibility("2y"));
		break;
	case 3:
		StrCopy(sz, K2L_Compatibility("3y"));
		break;
	case 4:
		StrCopy(sz, K2L_Compatibility("4y"));
		break;
	case 5:
		StrCopy(sz, K2L_Compatibility("5y"));
		break;
	}
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae\xb0\xa1 %s\xb9\xcc\xb8\xb8\xc0\xce \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8"),
			sz));
}

MISSION(15)
{
	char sz[8] = "";
	if (p2 == 0)
		StrCopy(sz, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
	else
		StrCopy(sz, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("%d\xc0\xa7 \xc0\xaf\xc0\xfa\xc0\xc7 \xb3\xb2\xc0\xba\xb0\xc5\xb8\xae\xb0\xa1 %s\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			p1, sz));
}

MISSION(16)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("4\xb8\xed \xc0\xcc\xbb\xf3\xc0\xc7 \xc0\xaf\xc0\xfa\xb0\xa1 \xc4\xa8\xc0\xce(OUT)\xc0\xbb \xc7\xd1 \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8"));
}

MISSION(17)
{
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("\xb8\xf0\xb5\xe7\xc0\xaf\xc0\xfa\xc0\xc7 \xb3\xb2\xc0\xba\xb0\xc5\xb8\xae \xc7\xd5\xb0\xe8\xb0\xa1 %dy \xb9\xcc\xb8\xb8\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			p2));
}

MISSION(18)
{
	sprintf(m_szDesc,
		K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae\xb0\xa1 %d.%d \xc0\xce \xc0\xaf\xc0\xfa\xb0\xa1 \xc0\xd6\xc0\xbb \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
		p1, p2);
}

MISSION(19)
{
	char sz[16] = "";
	if (p1 == 0)
		StrCopy(sz, K2L_Compatibility("\xb3\xb2\xc0\xda"));
	else
		StrCopy(sz, K2L_Compatibility("\xbf\xa9\xc0\xda"));
	if (p2 == 0)
		StrCopy(sz + 8, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
	else
		StrCopy(sz + 8, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("%s(\xc4\xb3\xb8\xaf\xc5\xcd)\xc0\xaf\xc0\xfa\xc0\xc7 \xb3\xb2\xc0\xba\xb0\xc5\xb8\xae \xc7\xd5\xb0\xe8\xb0\xa1 %s\xc0\xce \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz, sz + 8));
}

MISSION(20)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae\xb0\xa1 \xb0\xa1\xc0\xe5 \xb8\xd5 \xc0\xaf\xc0\xfa\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(21)
{
	sprintf(m_szDesc,
		MakeStr(
			K2L_Compatibility("\xb8\xf0\xb5\xe7\xc0\xaf\xc0\xfa\xc0\xc7 \xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3\xc0\xcc %d\xc3\xca \xb9\xcc\xb8\xb8\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			p1));
}

MISSION(22)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("1\xc0\xa7\xc7\xd1 \xc0\xaf\xc0\xfa\xc0\xc7 \xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3\xc0\xcc 10\xc3\xca \xb9\xcc\xb8\xb8\xc0\xcf\xb6\xa7 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(23)
{
	char sz[16] = "";
	switch (p1)
	{
	case 0:
		StrCopy(sz, K2L_Compatibility("\xba\xc0\xb4\xd9\xb8\xae"));
		break;
	case 1:
		StrCopy(sz, K2L_Compatibility("\xc7\xc7\xc7\xc9"));
		break;
	case 2:
		StrCopy(sz, K2L_Compatibility("\xb6\xec\xb6\xa5\xbb\xd1"));
		break;
	case 3:
		StrCopy(sz, K2L_Compatibility("\xb5\xb9\xc7\xc7\xb4\xcf"));
		break;
	case 4:
		StrCopy(sz, K2L_Compatibility("\xb7\xce\xb7\xce"));
		break;
	case 5:
		StrCopy(sz, K2L_Compatibility("\xc5\xa5\xb8\xb6"));
		break;
	case 6:
		StrCopy(sz, K2L_Compatibility("\xc6\xbc\xc5\xb0"));
		break;
	case 7:
		StrCopy(sz, K2L_Compatibility("\xc4\xab\xb5\xf0\xbf\xa1"));
		break;
	}
	sprintf(m_szDesc,
		K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae 20y\xb9\xcc\xb8\xb8, %s(\xc4\xb3\xb5\xf0)\xbb\xe7\xbf\xeb \xc0\xaf\xc0\xfa\xb4\xc2 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
		sz);
}

MISSION(24)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xb0\xa1\xc0\xe5 \xba\xfc\xb8\xa5\xbd\xc3\xb0\xa3\xbe\xc8\xbf\xa1 \xc5\xb8\xb1\xb8\xc7\xd1 \xc0\xaf\xc0\xfa\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(25)
{
	char sz[8] = "";
	if (p1 == 0)
		StrCopy(sz, K2L_Compatibility("\xc2\xa6\xbc\xf6"));
	else
		StrCopy(sz, K2L_Compatibility("\xc8\xa6\xbc\xf6"));
	StrCopy(m_szDesc,
		MakeStr(
			K2L_Compatibility("\xb3\xb2\xc0\xba\xb0\xc5\xb8\xae\xb0\xa1 %s\xc0\xce \xc0\xaf\xc0\xfa\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
			sz));
}

MISSION(26)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xb0\xa1\xc0\xe5 \xb4\xca\xb0\xd4 \xc5\xb8\xb1\xb8\xc7\xd1 \xc0\xaf\xc0\xfa\xc0\xcf \xb0\xe6\xbf\xec \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(27)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xc4\xb3\xb8\xaf\xc5\xcd \xbc\xba\xba\xb0\xb7\xce \xb3\xb2\xc0\xba\xb0\xc5\xb8\xae \xc6\xf2\xb1\xd5\xc0\xbb \xb3\xbb\xbe\xee \xc0\xfb\xc0\xba\xc2\xca\xc0\xcc \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(28)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xc4\xb3\xb8\xaf\xc5\xcd \xbc\xba\xba\xb0\xb7\xce \xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3 \xc6\xf2\xb1\xd5\xc0\xbb \xb3\xbb\xbe\xee \xb8\xb9\xc0\xba\xc2\xca\xc0\xcc \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

MISSION(29)
{
	if (p3 != NULL)
		StrCopy(m_szDesc,
			MakeStr(
				K2L_Compatibility("[%s]\xb4\xd4\xc0\xcc \xc4\xa8\xc0\xce\xc0\xbb \xc7\xcf\xb8\xe9 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."),
				p3));
}

MISSION(30)
{
	char sz[32] = "";
	switch (p1)
	{
	case 2:
		StrCopy(sz, K2L_Compatibility("\xc5\xe4\xb8\xb6\xc8\xa3\xc5\xa9"));
		break;
	case 4:
		StrCopy(sz, K2L_Compatibility("\xbd\xba\xc6\xc4\xc0\xcc\xc5\xa9"));
		break;
	case 8:
		StrCopy(sz, K2L_Compatibility("\xc4\xda\xba\xea\xb6\xf3"));
		break;
	}
	sprintf(m_szDesc,
		K2L_Compatibility("%s\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbf\xa9 \xc4\xa8\xc0\xce\xc0\xbb \xc7\xd1 \xc0\xaf\xc0\xfa\xb4\xc2 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8"),
		sz);
}

MISSION(31)
{
	StrCopy(m_szDesc,
		K2L_Compatibility("\xb0\xa1\xc0\xe5 \xbb\xa1\xb8\xae \xc4\xa8\xc0\xce\xc0\xbb \xc7\xd1 \xc0\xaf\xc0\xfa\xb4\xc2 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8."));
}

CApproachDesc::CApproachDesc(int type, unsigned long p1, unsigned long p2,
	const char* p3)
{
	m_pBlock = NULL;
	switch (type)
	{
	case 1:
		m_pBlock = new Mission_1(p1, p2, p3);
		break;
	case 2:
		m_pBlock = new Mission_2(p1, p2, p3);
		break;
	case 3:
		m_pBlock = new Mission_3(p1, p2, p3);
		break;
	case 4:
		m_pBlock = new Mission_4(p1, p2, p3);
		break;
	case 5:
		m_pBlock = new Mission_5(p1, p2, p3);
		break;
	case 6:
		m_pBlock = new Mission_6(p1, p2, p3);
		break;
	case 7:
		m_pBlock = new Mission_7(p1, p2, p3);
		break;
	case 8:
		m_pBlock = new Mission_8(p1, p2, p3);
		break;
	case 9:
		m_pBlock = new Mission_9(p1, p2, p3);
		break;
	case 10:
		m_pBlock = new Mission_10(p1, p2, p3);
		break;
	case 11:
		m_pBlock = new Mission_11(p1, p2, p3);
		break;
	case 12:
		m_pBlock = new Mission_12(p1, p2, p3);
		break;
	case 13:
		m_pBlock = new Mission_13(p1, p2, p3);
		break;
	case 14:
		m_pBlock = new Mission_14(p1, p2, p3);
		break;
	case 15:
		m_pBlock = new Mission_15(p1, p2, p3);
		break;
	case 16:
		m_pBlock = new Mission_16(p1, p2, p3);
		break;
	case 17:
		m_pBlock = new Mission_17(p1, p2, p3);
		break;
	case 18:
		m_pBlock = new Mission_18(p1, p2, p3);
		break;
	case 19:
		m_pBlock = new Mission_19(p1, p2, p3);
		break;
	case 20:
		m_pBlock = new Mission_20(p1, p2, p3);
		break;
	case 21:
		m_pBlock = new Mission_21(p1, p2, p3);
		break;
	case 22:
		m_pBlock = new Mission_22(p1, p2, p3);
		break;
	case 23:
		m_pBlock = new Mission_23(p1, p2, p3);
		break;
	case 24:
		m_pBlock = new Mission_24(p1, p2, p3);
		break;
	case 25:
		m_pBlock = new Mission_25(p1, p2, p3);
		break;
	case 26:
		m_pBlock = new Mission_26(p1, p2, p3);
		break;
	case 27:
		m_pBlock = new Mission_27(p1, p2, p3);
		break;
	case 28:
		m_pBlock = new Mission_28(p1, p2, p3);
		break;
	case 29:
		m_pBlock = new Mission_29(p1, p2, p3);
		break;
	case 30:
		m_pBlock = new Mission_30(p1, p2, p3);
		break;
	case 31:
		m_pBlock = new Mission_31(p1, p2, p3);
		break;
	}
}
