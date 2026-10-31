#include "minatl.h"
#include "projectg.h"
#include "wfont.h"
#include "titlefontex.h"

WTitleFontEx::WTitleFontEx()
	: m_pFont(NULL), m_color(0xffffffff)
{
}

WTitleFontEx::~WTitleFontEx()
{
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

float WTitleFontEx::Print(float x, float y, const char* text) const
{
	return m_pFont->Print(g_view, x, y, text, 0, m_color, NULL);
}

void WTitleFontEx::Print(float x, float y, const char* text,
	eTITLEFONTALINE align, float space, float scale) const
{
	if (!text)
		return;

	const char* p = text;
	const char* charSet = m_pFont->GetFontInfo()->pCharSet;
	float oldScale = m_pFont->GetScale();
	m_pFont->SetScale(scale);

	float width = 0.0f;

	switch (align)
	{
	case TFA_CENTER:
		while (*p)
		{
			int c = *p++;
			int index = strchr(charSet, c) - charSet;
			width += m_pFont->GetCharWidth(g_view, index) + space * scale;
		}
		width = (width - space) * 0.5f;
		break;

	case TFA_RIGHT:
		while (*p)
		{
			int c = *p++;
			int index = strchr(charSet, c) - charSet;
			width += m_pFont->GetCharWidth(g_view, index) + space * scale;
		}
		width = width - space;
		break;
	}

	x -= width;

	p = text;
	while (*p)
	{
		int c = *p++;
		int index = strchr(charSet, c) - charSet;
		float w = m_pFont->PutChar(g_view, x, y, index, 0, m_color);
		x += (w + space);
	}

	m_pFont->SetScale(oldScale);
}

float WTitleFontEx::GetFontH() const
{
	return (float)m_pFont->GetFontInfo()->fonth;
}
float WTitleFontEx::GetFontW() const
{
	return (float)m_pFont->GetFontInfo()->fontw;
}

int WTitleFontEx::GetCharIndex(char c)
{
	tagWTITLEFONT* info = m_pFont->GetFontInfo();
	int len = strlen(info->pCharSet);

	for (int i = 0; i < len; i++)
	{
		if (info->pCharSet[i] == c)
			return i;
	}

	return 0;
}

float WTitleFontEx::GetStringWidth(const char* text)
{
	if (!text)
		return 0.0f;

	float width = 0.0f;
	int len = strlen(text);

	for (int i = 0; i < len; i++)
		width += m_pFont->GetCharWidth(g_view, GetCharIndex(text[i]));

	return width;
}

CWindFont::CWindFont()
{
	static char* fileName[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,x";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

COpenTournamentFont::COpenTournamentFont()
{
	static char* fileName[] = { "pot_numbers.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 45;
	info.fonth = 43;
	info.numPages = 1;
	info.texw = 225;
	info.texh = 86;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

CBlueWindFontSmall::CBlueWindFontSmall()
{
	static char* fileName[] = { "blue_wind_s.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 18;
	info.fonth = 21;
	info.numPages = 1;
	info.texw = 108;
	info.texh = 42;
	info.pCharSet = "0123456789~";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

CBlueWindFontBig::CBlueWindFontBig()
{
	static char* fileName[] = { "blue_wind_b.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 29;
	info.fonth = 33;
	info.numPages = 1;
	info.texw = 174;
	info.texh = 66;
	info.pCharSet = "0123456789+-";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

CBlueHoleSkinFont::CBlueHoleSkinFont()
{
	static char* fileName[] = { "[font_hole_skins_blue.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 32;
	info.pCharSet = "1234567890+,";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

CBarFont::CBarFont()
{
	static char* fileName[] = { "[font_bar.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 32;
	info.pCharSet = "1234567890ym%-.";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 983: 17 fontes novas (RTTI do 983; texto gerado do 983 pela frente B, tools/f37/fonts983.py, pedido B58)
// 006f4ac0 (vtable 00d1c0e0; nomes de arquivo em 00e168a0)
CSingleNumCourseFont::CSingleNumCourseFont()
{
	static char* fileName[] = { "[single_num_course.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 10;
	info.fonth = 13;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 16;
	info.pCharSet = "0123456789x";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f4b80 (vtable 00d1c10c; nomes de arquivo em 00e168c8)
CRepeatHoleFont::CRepeatHoleFont()
{
	static char* fileName[] = { "[single_font_num.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 14;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 154;
	info.texh = 16;
	info.pCharSet = "1234567890!";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f4c40 (vtable 00d1c12c; nomes de arquivo em 00e168ec)
CClubSetWorkShopCardNumFont::CClubSetWorkShopCardNumFont()
{
	static char* fileName[] = { "clubmade_rankup_num.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 20;
	info.fonth = 22;
	info.numPages = 1;
	info.texw = 220;
	info.texh = 22;
	info.pCharSet = "0123456789x";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f4d00 (vtable 00d1c15c; nomes de arquivo em 00e1691c)
CSmallNumFont::CSmallNumFont()
{
	static char* fileName[] = { "[font_condition.png" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 32;
	info.pCharSet = "1234567890%/";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f4dc0 (vtable 00d1c178; nomes de arquivo em 00e1693c)
CAchieveTotalPointFont::CAchieveTotalPointFont()
{
	static char* fileName[] = { "[challenges_total_score.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 27;
	info.fonth = 33;
	info.numPages = 1;
	info.texw = 135;
	info.texh = 66;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f4e80 (vtable 00d1c19c; nomes de arquivo em 00e16968)
CAchieveUnitEarnPoint::CAchieveUnitEarnPoint()
{
	static char* fileName[] = { "[font_challenges_point.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 24;
	info.fonth = 24;
	info.numPages = 1;
	info.texw = 240;
	info.texh = 24;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f4f40 (vtable 00d1c1cc; nomes de arquivo em 00e16990)
CAchievementDate::CAchievementDate()
{
	static char* fileName[] = { "[font_challenges_date.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 8;
	info.fonth = 10;
	info.numPages = 1;
	info.texw = 88;
	info.texh = 10;
	info.pCharSet = "1234567890.";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f5000 (vtable 00d1c1f0; nomes de arquivo em 00e169b4)
CAttendanceDate::CAttendanceDate()
{
	static char* fileName[] = { "[font_check_date.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 40;
	info.fonth = 56;
	info.numPages = 1;
	info.texw = 200;
	info.texh = 112;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f50c0 (vtable 00d1c210; nomes de arquivo em 00e169d8)
CClubSetWorkShopEventFont::CClubSetWorkShopEventFont()
{
	static char* fileName[] = { "event_club_personal_count.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 14;
	info.fonth = 18;
	info.numPages = 1;
	info.texw = 140;
	info.texh = 18;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f5180 (vtable 00d1c244; nomes de arquivo em 00e16a04)
CGrandPrixGameRoomTimerFont::CGrandPrixGameRoomTimerFont()
{
	static char* fileName[] = { "gp_timecount_num.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 22;
	info.fonth = 28;
	info.numPages = 1;
	info.texw = 220;
	info.texh = 28;
	info.pCharSet = "0123456789";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f5240 (vtable 00d1c264; nomes de arquivo em 00e16a34)
CGrandPrixGameRoomTimerBigFont::CGrandPrixGameRoomTimerBigFont()
{
	static char* fileName[] = { "gp_timecount_num_big.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 28;
	info.fonth = 36;
	info.numPages = 1;
	info.texw = 280;
	info.texh = 36;
	info.pCharSet = "0123456789";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f5300 (vtable 00d1c2a0; nomes de arquivo em 00e16a68)
CGrandPrixGameRoomTicketFont::CGrandPrixGameRoomTicketFont()
{
	static char* fileName[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:, ";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f53c0 (vtable 00d1c2a8; nomes de arquivo em 00e16a98)
CGrandPrixDiffLevelFont::CGrandPrixDiffLevelFont()
{
	static char* fileName[] = { "gp_font_degree.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 10;
	info.fonth = 12;
	info.numPages = 1;
	info.texw = 100;
	info.texh = 12;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f5480 (vtable 00d1c2c4; nomes de arquivo em 00e16ac4)
CGrandPrixAwardRankFont::CGrandPrixAwardRankFont()
{
	static char* fileName[] = { "[font_gp_rankingnum.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 40;
	info.fonth = 44;
	info.numPages = 1;
	info.texw = 240;
	info.texh = 88;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f5540 (vtable 00d1c2e4; nomes de arquivo em 00e16af0)
CChristmasBakingFont::CChristmasBakingFont()
{
	static char* fileName[] = { "[font_condition_x3.png" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 48;
	info.fonth = 48;
	info.numPages = 1;
	info.texw = 384;
	info.texh = 96;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f5600 (vtable 00d1c304; nomes de arquivo em 00e16b18)
CTPItemCountFont::CTPItemCountFont()
{
	static char* fileName[] = { "number_tp.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 8;
	info.fonth = 10;
	info.numPages = 1;
	info.texw = 80;
	info.texh = 10;
	info.pCharSet = "1234567890";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

// 006f56c0 (vtable 00d1c32c; nomes de arquivo em 00e16b3c)
CMPItemCountFont::CMPItemCountFont()
{
	static char* fileName[] = { "number_mi.tga" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));

	info.filename = fileName;
	info.fontw = 8;
	info.fonth = 10;
	info.numPages = 1;
	info.texw = 104;
	info.texh = 10;
	info.pCharSet = "1234567890~/?";

	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

