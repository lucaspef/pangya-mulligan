#pragma once

enum eTITLEFONTALINE
{
	TFA_LEFT = 0,
	TFA_CENTER = 1,
	TFA_RIGHT = 2
};

class WTitleFontEx
{
public:
	virtual ~WTitleFontEx();

	float Print(float x, float y, const char* text) const;
	void Print(float x, float y, const char* text, eTITLEFONTALINE align,
		float space, float scale) const;
	float GetFontH() const;
	float GetFontW() const;
	float GetStringWidth(const char* text);

	void SetColor(unsigned long color) { m_color = color; }

protected:
	WTitleFontEx();

	int GetCharIndex(char c);

	WTitleFont* m_pFont;
	unsigned long m_color;
};

class CWindFont : public WTitleFontEx
{
public:
	CWindFont();
};

class COpenTournamentFont : public WTitleFontEx
{
public:
	COpenTournamentFont();
};

class CBlueWindFontSmall : public WTitleFontEx
{
public:
	CBlueWindFontSmall();
};

class CBlueWindFontBig : public WTitleFontEx
{
public:
	CBlueWindFontBig();
};

class CBlueHoleSkinFont : public WTitleFontEx
{
public:
	CBlueHoleSkinFont();
};

class CBarFont : public WTitleFontEx
{
public:
	CBarFont();
};

// 983: fontes novas do titlefontex do 983 (pedido B58; construtores 006f4ac0-006f56c0)
class CSingleNumCourseFont : public WTitleFontEx
{
public:
	CSingleNumCourseFont(); // 006f4ac0
};

class CRepeatHoleFont : public WTitleFontEx
{
public:
	CRepeatHoleFont(); // 006f4b80
};

class CClubSetWorkShopCardNumFont : public WTitleFontEx
{
public:
	CClubSetWorkShopCardNumFont(); // 006f4c40
};

class CSmallNumFont : public WTitleFontEx
{
public:
	CSmallNumFont(); // 006f4d00
};

class CAchieveTotalPointFont : public WTitleFontEx
{
public:
	CAchieveTotalPointFont(); // 006f4dc0
};

class CAchieveUnitEarnPoint : public WTitleFontEx
{
public:
	CAchieveUnitEarnPoint(); // 006f4e80
};

class CAchievementDate : public WTitleFontEx
{
public:
	CAchievementDate(); // 006f4f40
};

class CAttendanceDate : public WTitleFontEx
{
public:
	CAttendanceDate(); // 006f5000
};

class CClubSetWorkShopEventFont : public WTitleFontEx
{
public:
	CClubSetWorkShopEventFont(); // 006f50c0
};

class CGrandPrixGameRoomTimerFont : public WTitleFontEx
{
public:
	CGrandPrixGameRoomTimerFont(); // 006f5180
};

class CGrandPrixGameRoomTimerBigFont : public WTitleFontEx
{
public:
	CGrandPrixGameRoomTimerBigFont(); // 006f5240
};

class CGrandPrixGameRoomTicketFont : public WTitleFontEx
{
public:
	CGrandPrixGameRoomTicketFont(); // 006f5300
};

class CGrandPrixDiffLevelFont : public WTitleFontEx
{
public:
	CGrandPrixDiffLevelFont(); // 006f53c0
};

class CGrandPrixAwardRankFont : public WTitleFontEx
{
public:
	CGrandPrixAwardRankFont(); // 006f5480
};

class CChristmasBakingFont : public WTitleFontEx
{
public:
	CChristmasBakingFont(); // 006f5540
};

class CTPItemCountFont : public WTitleFontEx
{
public:
	CTPItemCountFont(); // 006f5600
};

class CMPItemCountFont : public WTitleFontEx
{
public:
	CMPItemCountFont(); // 006f56c0
};
