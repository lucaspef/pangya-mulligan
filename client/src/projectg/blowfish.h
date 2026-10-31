#pragma once

#define MAXKEYBYTES 56
#define NPASS 16

class CBlowFish
{
private:
	DWORD* PArray;
	DWORD (*SBoxes)[256];
	void Blowfish_encipher(DWORD* xl, DWORD* xr);
	void Blowfish_decipher(DWORD* xl, DWORD* xr);

public:
	CBlowFish();
	~CBlowFish();
	void Initialize(BYTE key[], int keybytes);
	DWORD GetOutputLength(DWORD lInputLong);
	DWORD Encode(BYTE* pInput, BYTE* pOutput, DWORD lSize);
	void Decode(BYTE* pInput, BYTE* pOutput, DWORD lSize);
};

union aword
{
	DWORD dword;
	BYTE byte[4];
	struct
	{
		unsigned int byte3 : 8;
		unsigned int byte2 : 8;
		unsigned int byte1 : 8;
		unsigned int byte0 : 8;
	} w;
};
