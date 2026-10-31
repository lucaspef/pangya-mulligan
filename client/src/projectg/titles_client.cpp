#include "minatl.h"
#include "vc10_layout.h"
#include "titles_client.h"
#include "../shared/sharedtitledoc.inl"

void RegisterTitles(TitleManager* titlemanager)
{
	titlemanager->Register(TITLE_PANGYASHOT, new Title21);
	titlemanager->Register(TITLE_STRAIGHTROAD, new Title22);
	titlemanager->Register(TITLE_GREENMASTER, new Title23);
	titlemanager->Register(TITLE_COURSEMASTER, new Title24);
	titlemanager->Register(TITLE_GOLD, new Title25);
	titlemanager->Register(TITLE_SILVER, new Title26);
	titlemanager->Register(TITLE_BRONZE, new Title27);
	titlemanager->Register(TITLE_MANNERANGEL, new Title28);
	titlemanager->Register((eTitle)50, new Title50);
	titlemanager->Register((eTitle)51, new Title51);
	titlemanager->Register((eTitle)52, new Title52);
	titlemanager->Register((eTitle)53, new Title53);
	titlemanager->Register((eTitle)381, new Title381);
	titlemanager->Register((eTitle)380, new Title380);
	titlemanager->Register((eTitle)69, new Title69);
	titlemanager->Register((eTitle)70, new Title70);
	titlemanager->Register((eTitle)71, new Title71);
	titlemanager->Register((eTitle)72, new Title72);
	titlemanager->Register((eTitle)73, new Title73);
	titlemanager->Register((eTitle)74, new Title74);
	titlemanager->Register((eTitle)123, new Title123);
	titlemanager->Register((eTitle)124, new Title124);
	titlemanager->Register((eTitle)125, new Title125);
	titlemanager->Register((eTitle)126, new Title126);
	titlemanager->Register((eTitle)357, new Title357);
	titlemanager->Register((eTitle)358, new Title358);
	titlemanager->Register((eTitle)359, new Title359);
	titlemanager->Register((eTitle)360, new Title360);
	titlemanager->Register((eTitle)361, new Title361);
}

IMPLEMENT_TITLE(21)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckPangyaPercent(Doc()->m_myInfo.stat.dwPangya,
			Doc()->m_myInfo.stat.dwDrive, 70))
	{
		return false;
	}
	return true;
}

IMPLEMENT_TITLE(22)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckFairawayPercent(Doc()->m_myInfo.stat.dwFairway,
			Doc()->m_myInfo.stat.dwHole, Doc()->m_myInfo.stat.dwMatchHole, 70))
	{
		return false;
	}
	return true;
}

IMPLEMENT_TITLE(23)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckPuttinPercent(Doc()->m_myInfo.stat.dwPuttIn,
			Doc()->m_myInfo.stat.dwPutt, 80))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(24)
{
	if (!CheckCourseMaster(Doc()->m_myInfo))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(25)
{
	if (!CheckGoldMedal(Doc()->m_myInfo.trophy, 10))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(26)
{
	if (!CheckSilverMedal(Doc()->m_myInfo.trophy, 10))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(27)
{
	if (!CheckBronzeMedal(Doc()->m_myInfo.trophy, 10))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(28)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckNoMannerPercent(Doc()->m_myInfo.stat.dwNoMannerGameCount,
			Doc()->m_myInfo.stat.dwGameCount, 3))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(50)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckPangyaPercent(Doc()->m_myInfo.stat.dwPangya + Doc()->m_oldSeasonStatistics.dwPangya,
			Doc()->m_myInfo.stat.dwDrive + Doc()->m_oldSeasonStatistics.dwDrive, 77))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(51)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckFairawayPercent(Doc()->m_myInfo.stat.dwFairway + Doc()->m_oldSeasonStatistics.dwFairway,
			Doc()->m_myInfo.stat.dwHole + Doc()->m_oldSeasonStatistics.dwHole,
			Doc()->m_myInfo.stat.dwMatchHole + Doc()->m_oldSeasonStatistics.dwMatchHole, 72))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(52)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckPuttinPercent(Doc()->m_myInfo.stat.dwPuttIn + Doc()->m_oldSeasonStatistics.dwPuttIn,
			Doc()->m_myInfo.stat.dwPutt + Doc()->m_oldSeasonStatistics.dwPutt, 90))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(53)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckNoMannerPercent(Doc()->m_myInfo.stat.dwNoMannerGameCount,
			Doc()->m_myInfo.stat.dwGameCount, 2))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(69)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -420))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(70)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -390))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(71)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -350))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(72)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -300))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(73)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -200))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(74)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -80))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(380)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -470))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(381)
{
	if (!CheckBestScoreSum(Doc()->m_myInfo, -540))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(123)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckNoMannerPercent(Doc()->m_myInfo.stat.dwNoMannerGameCount,
			Doc()->m_myInfo.stat.dwGameCount, 1))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(124)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckPuttinPercent(Doc()->m_myInfo.stat.dwPuttIn + Doc()->m_oldSeasonStatistics.dwPuttIn,
			Doc()->m_myInfo.stat.dwPutt + Doc()->m_oldSeasonStatistics.dwPutt, 95))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(125)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckFairawayPercent(Doc()->m_myInfo.stat.dwFairway + Doc()->m_oldSeasonStatistics.dwFairway,
			Doc()->m_myInfo.stat.dwHole + Doc()->m_oldSeasonStatistics.dwHole,
			Doc()->m_myInfo.stat.dwMatchHole + Doc()->m_oldSeasonStatistics.dwMatchHole, 90))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(126)
{
	if (!CheckLevel(Doc()->m_myInfo.stat.Level, BEGINNER_E))
	{
		return false;
	}

	if (!CheckPangyaPercent(Doc()->m_myInfo.stat.dwPangya + Doc()->m_oldSeasonStatistics.dwPangya,
			Doc()->m_myInfo.stat.dwDrive + Doc()->m_oldSeasonStatistics.dwDrive, 85))
	{
		return false;
	}

	return true;
}

IMPLEMENT_TITLE(357)
{
	if (!CheckPang(Doc()->m_myInfo.stat.i64Pang, 1000000))
	{
		return false;
	}
	return true;
}

IMPLEMENT_TITLE(358)
{
	if (!CheckPang(Doc()->m_myInfo.stat.i64Pang, 3000000))
	{
		return false;
	}
	return true;
}

IMPLEMENT_TITLE(359)
{
	if (!CheckPang(Doc()->m_myInfo.stat.i64Pang, 5000000))
	{
		return false;
	}
	return true;
}

IMPLEMENT_TITLE(360)
{
	if (!CheckHoleinCount(Doc()->m_myInfo.stat.wHoleInOne, 300))
	{
		return false;
	}
	return true;
}

IMPLEMENT_TITLE(361)
{
	if (!CheckHoleinCount(Doc()->m_myInfo.stat.wHoleInOne, 500))
	{
		return false;
	}
	return true;
}
C_ASSERT_VC10(offsetof(CSharedDoc, m_oldSeasonStatistics) == 0x10779);
C_ASSERT(offsetof(sMyInfo, mapStat) + 0x1d == 0x317);
