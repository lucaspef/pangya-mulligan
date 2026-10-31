#pragma once
#include <stdlib.h>
template <typename T>
class WCrypticValue
{
public:
	WCrypticValue()
	{
		m_key32 = ((rand() % 0xff | 0x12) << 24) |
			((rand() % 0xfe | 0x34) << 16) | ((rand() % 0xfe | 0xf0) << 8) |
			(rand() % 0xff);
		m_key16 =
			((unsigned char)(rand() % 0xfe | 0xf0) << 8) | (rand() % 0xff);
		m_key8 = rand() % 0xff;
	}
	bool IsSafe()
	{
		WCrypticValue* address;
		address = this;
		for (unsigned int i = 0; i < sizeof(T); ++i)
			if (m_data[sizeof(T) + i] !=
				(m_data[i] ^ ((unsigned char*)&address)[i]))
				return false;
		return true;
	}
	WCrypticValue& operator=(T value)
	{
		Set(value);
		return *this;
	}
	WCrypticValue& operator=(const WCrypticValue& value)
	{
		Set(value.Get());
		return *this;
	}
	bool operator==(T value) { return Get() == value; }
	bool operator>(T value) { return Get() > value; }
	bool operator<(T value) { return Get() < value; }
	bool operator>=(T value) { return Get() >= value; }
	bool operator<=(T value) { return Get() <= value; }
	void operator+=(T value)
	{
		T result = Get() + value;
		Set(result);
	}
	void operator-=(T value)
	{
		T result = Get() - value;
		Set(result);
	}
	void operator*=(T value)
	{
		T result = Get() * value;
		Set(result);
	}
	void operator/=(T value)
	{
		T result = Get() / value;
		Set(result);
	}
	T operator+(T value) { return Get() + value; }
	T operator-(T value) { return Get() - value; }
	T operator*(T value) { return Get() * value; }
	T operator/(T value) { return Get() / value; }
	operator T() { return Get(); }

private:
	void MakeCheckSum()
	{
		WCrypticValue* address;
		address = this;
		unsigned char* checkSum = m_data + sizeof(T);
		for (unsigned int i = 0; i < sizeof(T); ++i)
			checkSum[i] = m_data[i] ^ ((unsigned char*)&address)[i];
	}
	T Get() const
	{
		switch (sizeof(T))
		{
		case 1:
		{
			unsigned char value = ~(m_key8 ^ m_data[0]);
			return *(T*)&value;
		}
		case 2:
		{
			unsigned short value = ~(m_key16 ^ *(unsigned short*)m_data);
			return *(T*)&value;
		}
		case 4:
		{
			unsigned long value = ~(m_key32 ^ *(unsigned long*)m_data);
			return *(T*)&value;
		}
		}
		return T();
	}
	void Set(T value)
	{
		switch (sizeof(T))
		{
		case 1:
			*(unsigned char*)m_data = ~*(unsigned char*)&value ^ m_key8;
			break;
		case 2:
			*(unsigned short*)m_data = ~*(unsigned short*)&value ^ m_key16;
			break;
		case 4:
			*(unsigned long*)m_data = ~*(unsigned long*)&value ^ m_key32;
			break;
		}
		MakeCheckSum();
	}
	unsigned char m_data[8];
	unsigned long m_key32;
	unsigned short m_key16;
	unsigned char m_key8;
};
