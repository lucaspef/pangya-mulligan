#pragma once

struct sCrcFile
{
	char filename[32];
	unsigned long crc;
};

extern const unsigned long g_dwMapCheckNum;
extern sCrcFile g_CheckFile[];
