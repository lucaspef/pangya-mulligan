#pragma once

struct rc4_key
{
	unsigned char state[256];
	unsigned char x;
	unsigned char y;
};

class CRC4
{
public:
	CRC4();
	CRC4(unsigned char* key, int len);
	virtual ~CRC4();

	void SetKey(unsigned char* key);
	void Conversion(unsigned char* buffer, int len);

private:
	void PrepareKey(unsigned char* key, int len);

	rc4_key m_key;
	unsigned char* m_pKeyData;
};
