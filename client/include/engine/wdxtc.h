#pragma once
#include <ddraw.h>

struct sClr32;
struct sDXTC_ClrBlk;
struct sDXTC_ExpAlphaBlk;
struct sDXTC_LinearAlphaBlk3Bit;
class WDXTC
{
public:
	WDXTC();
	~WDXTC();
	bool Load(const unsigned char*, int);
	_DDSURFACEDESC2* GetDdsd2();
	unsigned char* GetDecompData();

private:
	bool DecompDXTC();
	void Decomp_DXT1();
	void Decomp_DXT3();
	void Decomp_DXT5();
	void DecompBlock_DXT1(unsigned char*, const sDXTC_ClrBlk*);
	void DecompBlock_DXT3(unsigned char*, const sDXTC_ClrBlk*);
	void DecompBlock_DXT5(unsigned char*, const sDXTC_ClrBlk*);
	void GetColorFromBlock(sClr32*, const sDXTC_ClrBlk*);
	void DecodeColor(sClr32*, const sDXTC_ClrBlk*, sClr32*);
	void DecodeExpAlphaBlock(sClr32*, const sDXTC_ExpAlphaBlk*);
	void DecodeLinearAlphaBlock3Bit(sClr32*, const sDXTC_LinearAlphaBlk3Bit*);
	void SaveAsBmp(char*) const;
	_DDSURFACEDESC2 m_DDSD2;
	unsigned char* m_pCompData;
	unsigned char* m_pDecompData;
};
