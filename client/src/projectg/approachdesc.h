#pragma once
#include <string.h>

class DescBlock
{
public:
	DescBlock() { memset(m_szDesc, 0, sizeof(m_szDesc)); }
	virtual ~DescBlock() { }

#ifdef REBANG_GOLFTASK_CPP // HACK: other includers keep their IL (emission order)
	char* GetDesc() { return m_szDesc; }
#endif

protected:
	virtual void BuildDesc(unsigned long p1, unsigned long p2, const char* p3)
	{
	}

	char m_szDesc[128];
};

class CApproachDesc
{
public:
	CApproachDesc(int type, unsigned long p1, unsigned long p2, const char* p3);

#ifdef REBANG_GOLFTASK_CPP // HACK: other includers keep their IL (emission order)
	~CApproachDesc()
	{
		if (m_pBlock)
			delete m_pBlock;
		m_pBlock = NULL;
	}

	char* GetDesc() { return m_pBlock ? m_pBlock->GetDesc() : NULL; }
#endif

protected:
	DescBlock* m_pBlock;
};
