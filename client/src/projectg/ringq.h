#pragma once

// Fixed-size ring queue: m_head/m_tail index into m_buf[m_size]
template <class T>
class TRingQ
{
public:
	TRingQ()
	{
		m_buf = 0;
		m_size = 0;
		m_tail = 0;
		m_head = 0;
	}
	~TRingQ()
	{
		if (m_buf)
			delete[] m_buf;
	}

	int Init(int size)
	{
		if (size < 2)
			return 1;
		if (m_buf)
			delete[] m_buf;
		m_buf = new T[size];
		m_size = size;
		m_tail = 0;
		m_head = 0;
		return 0;
	}
	T* GetHead(unsigned int n) const { return &m_buf[(m_head + n) % m_size]; }
	T* GetTail(unsigned int n) const
	{
		int i = m_tail - n;
		if (i < 0)
			i = m_size - n + m_tail;
		return &m_buf[i];
	}
	int GetLen() const
	{
		if (m_tail < m_head)
			return m_size - m_head + m_tail;
		return m_tail - m_head;
	}
	int IsUndersink(unsigned int n) const { return (int)n > GetLen(); }
	int IsOverflow(unsigned int n) const
	{
		if (m_head > m_tail)
		{
			if ((int)n < m_head - m_tail)
				return 0;
		}
		else if (m_size > (int)(m_tail - m_head + n))
			return 0;
		return 1;
	}
	int IsEmpty() const { return IsUndersink(1); }
	int IsFull() const { return IsOverflow(1); }
	T* StepHead(unsigned int n)
	{
		if (IsUndersink(n))
			return 0;
		m_head = (m_head + n) % m_size;
		return &m_buf[m_head];
	}
	T* StepTail(unsigned int n)
	{
		if (IsOverflow(n))
			return 0;
		m_tail = (m_tail + n) % m_size;
		return &m_buf[m_tail];
	}

	T* m_buf;   // 0x00
	int m_head; // 0x04
	int m_tail; // 0x08
	int m_size; // 0x0c
};
