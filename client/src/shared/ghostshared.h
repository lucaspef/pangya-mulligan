#pragma once

#include <stdlib.h>
#include <vector>

class CGhostGameBonusUtil
{
public:
	struct GAMEBONUS
	{
		int point;
		unsigned char clickpang;
		int maxPang;

		GAMEBONUS(int _point, unsigned char _cbonus, int _maxpang)
			: point(_point), clickpang(_cbonus), maxPang(_maxpang)
		{
		}
	};

	CGhostGameBonusUtil()
		: m_vecBonus()
	{
		m_vecBonus.reserve(10);

		m_vecBonus.push_back(GAMEBONUS(-2000, 10, 500));
		m_vecBonus.push_back(GAMEBONUS(0, 20, 1000));
		m_vecBonus.push_back(GAMEBONUS(1000, 30, 1500));
		m_vecBonus.push_back(GAMEBONUS(2000, 40, 2000));
		m_vecBonus.push_back(GAMEBONUS(3000, 50, 2500));
		m_vecBonus.push_back(GAMEBONUS(4000, 60, 3000));
		m_vecBonus.push_back(GAMEBONUS(5000, 70, 3500));
		m_vecBonus.push_back(GAMEBONUS(6000, 80, 4000));
		m_vecBonus.push_back(GAMEBONUS(7000, 100, 5000));
		m_vecBonus.push_back(GAMEBONUS(0x7fffffff, 200, 10000));
	}

	~CGhostGameBonusUtil() { }

	bool IsUserWin(int user_score, int user_pang, int ghost_score,
		int ghost_pang)
	{
		int user = abs(user_score);
		int ghost = abs(ghost_score);

		return (user > ghost)
			? true
			: ((user == ghost && user_pang > ghost_pang) ? true : false);
	}

	int GetOneClickBonus(unsigned char hole, bool bUserWin, int UserScore,
		int course_difficulty, unsigned char ghostLv, unsigned char userLv)
	{
		int index = GetPangBonusIndex(GetBonusPoint(bUserWin, UserScore,
			course_difficulty, ghostLv, userLv));
		int bonus = m_vecBonus[index].clickpang * hole / 18;
		return bonus > 1 ? 1 : bonus;
	}

private:
	int GetBonusPoint(bool bWin, int score, int course_difficulty,
		unsigned char ghostLv, unsigned char userLv)
	{
		if (course_difficulty >= 5)
			course_difficulty = 5;

		return (score + course_difficulty * 20 + (bWin ? 10 : -5) * 6) * 50 +
			abs(ghostLv - userLv) * 20;
	}

	int GetPangBonusIndex(int iBonusPoint)
	{
		int index = 0;
		std::vector<GAMEBONUS>::iterator it = m_vecBonus.begin();
		for (; it != m_vecBonus.end(); ++it, ++index)
			if (iBonusPoint <= (*it).point)
				break;

		return index;
	}

	std::vector<GAMEBONUS> m_vecBonus;
};
