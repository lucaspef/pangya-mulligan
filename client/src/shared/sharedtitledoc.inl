float RecalculatePercent(float Value);

inline bool CheckLevel(unsigned long level, eLevel limitlevel)
{
	if (level >= limitlevel)
		return true;
	return false;
}

inline bool CheckPangyaPercent(unsigned long pangya, unsigned long drive,
	float limitpercent)
{
	float percent = (float)pangya / (float)drive * 100;
	if (RecalculatePercent(percent) >= limitpercent)
		return true;
	return false;
}

inline bool CheckFairawayPercent(unsigned long dwFair, unsigned long hole,
	unsigned long matchhole, float limitpercent)
{
	float percent = (float)dwFair / ((float)hole - (float)matchhole) * 100;
	if (RecalculatePercent(percent) >= limitpercent)
		return true;
	return false;
}

inline bool CheckPuttinPercent(unsigned long puttin, unsigned long putt,
	float limitpercent)
{
	float percent = (float)puttin / (float)putt * 100;
	if (RecalculatePercent(percent) >= limitpercent)
		return true;
	return false;
}

inline bool CheckNoMannerPercent(unsigned long nomannergamecount,
	unsigned long gamecount, float limitpercent)
{
	float percent = (float)nomannergamecount / (float)gamecount * 100;
	if (RecalculatePercent(percent) < limitpercent)
		return true;
	return false;
}

inline bool CheckGoldMedal(sTrophyStatistics& TrohpyStatics, unsigned long limitmedal)
{
	unsigned long medal = 0;
	for (int i = 0; i < 13; i++)
	{
		medal += TrohpyStatics.Trophy[i][0];
	}

	if (medal >= limitmedal)
	{
		return true;
	}
	return false;
}

inline bool CheckSilverMedal(sTrophyStatistics& TrohpyStatics,
	unsigned long limitmedal)
{
	unsigned long medal = 0;
	for (int i = 0; i < 13; i++)
	{
		medal += TrohpyStatics.Trophy[i][1];
	}

	if (medal >= limitmedal)
	{
		return true;
	}
	return false;
}

inline bool CheckBronzeMedal(sTrophyStatistics& trohpystatics,
	unsigned long limitmedal)
{
	unsigned long medal = 0;
	for (int i = 0; i < 13; i++)
	{
		medal += trohpystatics.Trophy[i][2];
	}

	if (medal >= limitmedal)
	{
		return true;
	}
	return false;
}

bool CheckCourseMaster(sMyInfo& myinfo)
{
	// 983: 12 vetores de 22 mapas seguidos a partir de mapStat (sMyInfo::mapStat/classicMapStat/mapStat983)
	const sMapStatistics* stat = myinfo.mapStat;
	bool bCompleteMap[22] = { false };

	for (int i = 0; i < 22; i++)
	{
		if (i == 12 || i == 17)
		{
			bCompleteMap[i] = true;
			continue;
		}

		for (int j = 0; j < 12; j++)
		{
			if (stat[j * 22 + i].cBestScore != 127)
			{
				bCompleteMap[i] = true;
				break;
			}
		}
	}

	for (int i = 0; i < 22; i++)
	{
		if (bCompleteMap[i] == false)
			return false;
	}

	return true;
}

bool CheckBestScoreSum(sMyInfo& myinfo, int limitscore)
{
	// 983: menor melhor-score entre os vetores 2 e 6 (mapStat983[0] e [4]) de cada mapa
	const sMapStatistics* stat = myinfo.mapStat;
	int sum = 0;
	char a, b, score;
	for (int i = 0; i < 22; i += 2)
	{
		a = stat[2 * 22 + i].cBestScore;
		b = stat[6 * 22 + i].cBestScore;
		score = (std::min)(a, b);
		if (score != 127)
			sum += score;

		b = stat[6 * 22 + i + 1].cBestScore;
		a = stat[2 * 22 + i + 1].cBestScore;
		score = (std::min)(a, b);
		if (score != 127)
			sum += score;
	}

	if (sum <= limitscore)
		return true;
	return false;
}

float RecalculatePercent(float Value)
{
	float percent = floor((Value + 0.05f) * 10.0f) / 10.0f; // 983 00835c3c fdiv
	return percent;
}

inline bool CheckPang(__int64 pang, __int64 limitpang)
{
	if (pang >= limitpang)
	{
		return true;
	}
	return false;
}

inline bool CheckHoleinCount(unsigned long holein, unsigned long limitholein)
{
	if (holein >= limitholein)
	{
		return true;
	}
	return false;
}
