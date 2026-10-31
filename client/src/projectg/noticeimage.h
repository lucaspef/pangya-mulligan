#pragma once

#include <list>
#include <string>
#include <vector>

struct sNoticeImage
{
	struct sBn
	{
		sBn() { }

		int typeId;
		WRect rect;
	};

	sNoticeImage() { }

	std::string name; // 983: std::string (0x28 por item; no 645 era char[32], 0x2c)
	std::list<sBn> bnList;
};

// 983: 00896e10
void LoadNoticeImageFile(const char* filename,
	std::vector<sNoticeImage>& images, int maxNum, const char* lang);
