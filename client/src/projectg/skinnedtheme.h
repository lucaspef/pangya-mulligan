#pragma once

#include <string>

class FrGroupBox;

struct THEME_BUTTONS
{
	char normal[64];
	char press[64];
	char disable[64];
	char over[64];
};

struct THEME_OVERBAR
{
	char bar[3][64];
};

struct THEME_OVERBAR_FRAME
{
	char frame[3][64];
};

struct _ISkinnedTheme
{
	_ISkinnedTheme() { }
	virtual const char* GetSkinedThemeClassName() = 0;
	virtual const char* GetWindOutterCircleImgName() = 0;
	virtual const char* GetWindInnerCircleImgName() = 0;
	virtual const char* GetRoomListBackGroundImgName() = 0;
	virtual void GetOverBarImgName(THEME_OVERBAR& overbar) = 0;
	virtual void GetOverBarFrameImgName(THEME_OVERBAR_FRAME& frame) = 0;
	virtual void GetMakeRoomButtonImgName(THEME_BUTTONS& buttons) = 0;
	virtual void GetQuickStartButtonImgName(THEME_BUTTONS& buttons) = 0;
};

namespace S5
{
	namespace THEME
	{
		void GetServerSelectButton(unsigned long flag, THEME_BUTTONS& buttons);
		void GetChannelSelectButton(unsigned long flag, THEME_BUTTONS& buttons);
		void ChangeGroupBoxChildAreaBgImg(FrGroupBox* pGroupBox,
			const char* areaName, const char* imgName);
		void ChangeOverbarTheme(_ISkinnedTheme* pTheme, FrGroupBox* pGroupBox);
		void SetSkinnedTheme(const char* name);
		void SetSkinnedTheme(unsigned long flag);
		_ISkinnedTheme* GetCurrentSkinnedTheme();
	}
}
