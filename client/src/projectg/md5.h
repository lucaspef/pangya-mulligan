#pragma once

typedef unsigned int uint32;

struct MD5Context
{
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

void MD5Init(struct MD5Context* context);
void MD5Update(struct MD5Context* context, unsigned char const* buf,
	unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context* context);
void MD5Transform(uint32 buf[4], uint32 const in[16]);

void GetMD5String(const char* src, char* dst);
void GetMD5String(const unsigned char* src, unsigned len, char* dst); // JP 983: 008304f0

typedef struct MD5Context MD5_CTX;
