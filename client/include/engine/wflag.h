#pragma once

class WFlags
{
public:
	WFlags()
		: m_flag(0)
	{
	}
#ifdef REBANG_GOLFRULE_PCH
	// HACK: golfrule.cpp: inline without emitting the COMDAT owned by a later unit.
	__declspec(dllimport) __forceinline WFlags(const unsigned long& flag)
#else
	__forceinline WFlags(const unsigned long& flag)
#endif
		: m_flag(flag)
	{
	}
	void Set(unsigned long value) { this->m_flag = value; }
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	__forceinline void Enable(unsigned long flag);
#else
	__forceinline void Enable(unsigned long flag) { this->m_flag |= flag; }
#endif
	__forceinline void Disable(unsigned long flag) { this->m_flag &= ~flag; }
	__forceinline void Turn(unsigned long flag, bool on)
	{
		if (on == true)
			this->m_flag |= flag;
		else
			this->m_flag &= ~flag;
	}
	void Reset() { this->m_flag = 0; }
	bool GetFlag(unsigned long flag) const
	{
		return (this->m_flag & flag) ? true : false;
	}
#ifdef REBANG_GOLFRULE_PCH
	__declspec(dllimport) __forceinline operator unsigned long() const
	{
		return m_flag;
	}
#else
	__forceinline operator unsigned long() const { return m_flag; }
#endif

private:
	unsigned long m_flag;
};
