#include "minatl.h"
#include <iterator> // VC10: std::back_inserter
#include "ghostdocument.h"
#include "shareddoc.h"
#include "actor.h"
#include "wind.h"
#include "club.h"
#include "golfball.h"
#include "cfile.h"
#include "xunzip.h"
#include "xzip.h"
#include "type_convert.h"
#include "mathconsts.h"
#include "../shared/sharedtables.h"
#include "s5/utilities.h"
#include <io.h>
#include <algorithm>
#include "jp/docofs983.h" // D874: 0x7b8 (m_myInfo) ancorado no membro (VS 2022)

// HACK: stands in for an unknown erased inline use of __rtti_obj
static __declspec(thread) int __rtti_tls_pad;



namespace
{
	template <class From, class To>
	void __fastcall py_data_cast(From& from, To& to)
	{
		to = *reinterpret_cast<const To*>(&from);
	}

	template void __fastcall py_data_cast<float, int>(float&, int&);
	template void __fastcall py_data_cast<const int, float>(const int&, float&);
}

const char* GHOST_EXCEPTION_LOG_FILENAME = "ghost_exception_log.txt";

unsigned char __fastcall GetHoleType()
{
	if (0)
		((CGhostHoleData*)0)
			->CGhostHoleData::RecordAct(*(int*)0, *(int*)0, *(const int**)0,
				*(int*)0);
	return CSharedDoc::Instance()->m_holeType;
}

unsigned char __fastcall MyIndex();

struct ghost_card_converter
{
	GHOST_CARD_DATA operator()(const sSCardAvilityPeriodInfo& info) const
	{
		GHOST_CARD_DATA data;
		memset(&data, 0, sizeof(data));
		type_convert(&info, &data);
		return data;
	}
};

namespace
{
	const char* GetCardTypeString(int type)
	{
		if (type == 0)
		{
			return "CARD_CHARACTER";
		}
		if (type == 1)
		{
			return "CARD_CADDIE";
		}
		if (type == 2)
		{
			return "CARD_SPECIAL";
		}
		if (type == 3)
		{
			return "CARD_PACK";
		}
		if (type == 4)
		{
			return "CARD_BOX";
		}
		return "NOT DEFINED";
	}

	const char* GetCardAbilityString(int ability)
	{
		if (ability == -1)
		{
			return "SPECIAL_ABILITY_NONE"; // 983 00813da0
		}
		if (ability == 0)
		{
			return "SPECIAL_EXP_ADD";
		}
		if (ability == 1)
		{
			return "SPECIAL_PANG_RATE_UP_TIME";
		}
		if (ability == 2)
		{
			return "SPECIAL_EXP_REAE_UP_TIME";
		}
		if (ability == 3)
		{
			return "SPECIAL_PANG_ADD";
		}
		if (ability == 4)
		{
			return "SPECIAL_POWER_UP_TIME";
		}
		if (ability == 5)
		{
			return "SPECIAL_CONTROL_UP_TIME";
		}
		if (ability == 6)
		{
			return "SPECIAL_ACCCURACY_UP_TIME";
		}
		if (ability == 7)
		{
			return "SPECIAL_SPIN_UP_TIME";
		}
		if (ability == 8)
		{
			return "SPECIAL_CURVE_UP_TIME";
		}
		if (ability == 9)
		{
			return "SPECIAL_START_COMBOGAUGE_UP_TIME";
		}
		if (ability == 10)
		{
			return "SPECIAL_SLOT_UP_TIME";
		}
		if (ability == 11)
		{
			return "SPECIAL_PANGYAZONE_UP";
		}
		if (ability == 12)
		{
			return "SPECIAL_CLEARBONUS_UP_SEPIAWIND";
		}
		if (ability == 13)
		{
			return "SPECIAL_CLEARBONUS_UP_WINDHILL";
		}
		if (ability == 14)
		{
			return "SPECIAL_CLEARBONUS_UP_PINKWIND";
		}
		if (ability == 15)
		{
			return "SPECIAL_CLEARBONUS_UP_BLUEMOON";
		}
		if (ability == 16)
		{
			return "SPECIAL_RANDOMPANG_ADD";
		}
		if (ability == 17)
		{
			return "SPECIAL_TREASUREPOINT_UP";
		}
		if (ability == 18)
		{
			return "SPECIAL_RAINFALL_PROBABILITY_UP";
		}
		if (ability == 19)
		{
			return "SPECIAL_CLEARBONUS_UP_BLUE_LAGOON";
		}
		if (ability == 20)
		{
			return "SPECIAL_CLEARBONUS_UP_BLUE_WATER";
		}
		if (ability == 21)
		{
			return "SPECIAL_CLEARBONUS_UP_SHINING_SAND";
		}
		if (ability == 22)
		{
			return "SPECIAL_CLEARBONUS_UP_INFERNO";
		}
		if (ability == 23)
		{
			return "SPECIAL_CLEARBONUS_UP_SILVIA_CANNON";
		}
		if (ability == 24)
		{
			return "SPECIAL_CLEARBONUS_UP_EASTERN_VALLEY";
		}
		if (ability == 25)
		{
			return "SPECIAL_CLEARBONUS_UP_GREEN_SAND";
		}
		if (ability == 26)
		{
			return "SPECIAL_RANGUAGE_UP";
		}
		if (ability == 27)
		{
			return "SPECIAL_PANGYA_COMBOGUAGE_UP";
		}
		if (ability == 0x22)
		{
			return "SPECIAL_ABILITY_MAX";
		}
		return "NOT DEFINED";
	}

	const char* GetActionString(GHOST::eGHOSTACTMSG msg)
	{
		if (msg == GHOST::CHANGE_WIND)
		{
			return "GHOST::CHANGE_WIND";
		}
		if (msg == GHOST::CHANGE_WEATHER)
		{
			return "GHOST::CHANGE_WEATHER";
		}
		if (msg == GHOST::CHANGE_DIRECTION)
		{
			return "GHOST::CHANGE_DIRECTION";
		}
		if (msg == GHOST::CHANGE_CLUB)
		{
			return "GHOST::CHANGE_CLUB";
		}
		if (msg == GHOST::USE_ITEM)
		{
			return "GHOST::USE_ITEM";
		}
		if (msg == GHOST::BALL_DROP)
		{
			return "GHOST::BALL_DROP";
		}
		if (msg == GHOST::TIME_OUT)
		{
			return "GHOST::TIME_OUT";
		}
		if (msg == GHOST::GAME_TIME_OVER)
		{
			return "GHOST::GAME_TIME_OVER";
		}
		if (msg == GHOST::SAVE_SHOT_DATA)
		{
			return "GHOST::SAVE_SHOT_DATA";
		}
		return "NOT DEFINED";
	}

	struct print_ghost_card_data
	{
		print_ghost_card_data(FILE* fp)
			: m_fp(fp)
		{
		}

		void operator()(const GHOST_CARD_DATA& card) const
		{
			if (card.m_attachedId != 0)
			{
				fprintf(m_fp, "\tATTACHED : %d(%s), PARTS_ITEMID : %d\n",
					card.m_attachedId,
					ItemManager()->GetItemName(card.m_attachedId),
					card.m_partsItemId);
			}
			fprintf(m_fp, "\tCARD TYPEID : %d(%s)\n", card.m_typeId,
				ItemManager()->GetItemName(card.m_typeId));
			fprintf(m_fp, "\tCARD ITEMID : %d\n", card.m_itemId);
			fprintf(m_fp, "\tCARD TYPE : %d(%s)\n", card.m_cardType,
				GetCardTypeString(card.m_cardType));
			fprintf(m_fp, "\tCARD ABILITY : %d(%s)\n", card.m_ability,
				GetCardAbilityString(card.m_ability));
			fprintf(m_fp, "\tABILITY VALUE : %d\n", card.m_abilityValue);
			fprintf(m_fp, "\tCARD SLOTNUM : %d\n", card.m_slotNum);
			fprintf(m_fp, "\n");
		}

		FILE* m_fp;
	};
}

CGhostHoleData::CGhostHoleData()
	: m_holeNum(0)
{
	m_pCurrentShot = new GHOST_SHOT_DATA;
	memset(m_pCurrentShot, 0, sizeof(GHOST_SHOT_DATA));
}

CGhostHoleData::~CGhostHoleData()
{
	std::for_each(m_shotDataList.begin(), m_shotDataList.end(),
		Delete_Object());
	m_shotDataList.clear();
	if (m_pCurrentShot)
	{
		delete m_pCurrentShot;
		m_pCurrentShot = NULL;
	}
}

__declspec(noinline) void CGhostHoleData::RecordAct(int index, int type, const int* args,
	int count)
{
	GHOST_ACT_DATA* act = &m_pCurrentShot->m_acts[index];
	act->m_used = 1;
	act->m_type = (unsigned char)type;
	int* dst = act->m_args;
	dst[0] = -1;
	dst[1] = -1;
	dst[2] = -1;
	dst[3] = -1;
	dst[4] = -1;
	if (count != 0 && args != NULL)
	{
		if (count < 1)
		{
			count = 1;
		}
		else if (count > 5)
		{
			count = 5;
		}
		memcpy(dst, args, count * sizeof(int));
	}
}

void CGhostHoleData::RecordDummyShot(unsigned int frame)
{
	m_pCurrentShot->m_valid = 1;
	m_pCurrentShot->m_frame = frame;
	m_pCurrentShot->m_holeIndex = CSharedDoc::Instance()->m_pGolfDoc->m_currentHole;
	GHOST_SHOT_DATA* copy = new GHOST_SHOT_DATA;
	*copy = *m_pCurrentShot;
	m_shotDataList.push_back(copy);
	memset(m_pCurrentShot, 0, sizeof(GHOST_SHOT_DATA));
}

unsigned int CGhostHoleData::RollBackShotData(unsigned int count0)
{
	unsigned int count = count0;
	memset(m_pCurrentShot, 0, sizeof(GHOST_SHOT_DATA));
	std::list<GHOST_SHOT_DATA*>::reverse_iterator it = m_shotDataList.rbegin();
	while (it != m_shotDataList.rend() && count != 0)
	{
		--count;
		delete *it;
		it = std::list<GHOST_SHOT_DATA*>::reverse_iterator(
			m_shotDataList.erase((++it).base()));
	}
	return count0 - count;
}

const GHOST_SHOT_DATA* CGhostHoleData::RecordShotData(
	const sShotProtocol* protocol, unsigned int frame)
{
	m_pCurrentShot->m_valid = 0;
	m_pCurrentShot->m_frame = frame;
	m_pCurrentShot->m_holeIndex = CSharedDoc::Instance()->m_pGolfDoc->m_currentHole;
	type_convert(protocol, &m_pCurrentShot->m_protocol);
	if (GetCurMap() == 7)
	{
		unsigned long start = Wind().GetSpWindStartFrame();
		RecordAct(3, 2, (const int*)&start, 1);
	}
	m_pCurrentShot->m_windDirection = Wind().GetGlobalDirection();
	m_pCurrentShot->m_windIntensity = Wind().GetPower983();
	m_pCurrentShot->m_windVec[0] = Wind().GetGlobalWind().x;
	m_pCurrentShot->m_windVec[1] = Wind().GetGlobalWind().y;
	m_pCurrentShot->m_windVec[2] = Wind().GetGlobalWind().z;
	m_pCurrentShot->m_ballPos[0] = GolfBall().m_pos.x;
	m_pCurrentShot->m_ballPos[1] = GolfBall().m_pos.y;
	m_pCurrentShot->m_ballPos[2] = GolfBall().m_pos.z;
	m_pCurrentShot->m_windIntensityInt = (int)Wind().GetPower983();
	m_pCurrentShot->m_cameraAngle = CSharedDoc::Instance()->m_pGolfDoc->m_cameraData.angle;
	GHOST_SHOT_DATA* copy = new GHOST_SHOT_DATA;
	*copy = *m_pCurrentShot;
	m_shotDataList.push_back(copy);
	memset(m_pCurrentShot, 0, sizeof(GHOST_SHOT_DATA));
	return copy;
}

void CGhostHoleData::AddShotData(const GHOST_SHOT_DATA* shot)
{
	GHOST_SHOT_DATA* copy = new GHOST_SHOT_DATA;
	*copy = *shot;
	m_shotDataList.push_back(copy);
}

CGhostDocument::CGhostDocument()
{
	m_shotCount = 0;
	m_invalidate = 0;
	m_pUserData = new GHOST_USER_DATA;
	memset(m_pUserData, 0, sizeof(GHOST_USER_DATA));
	m_pHeader = new GHOST_GAME_BRIEF;
	memset(m_pHeader, 0, sizeof(GHOST_GAME_BRIEF));
	m_pHoleData = new CGhostHoleData[18];
	for (int i = 0; i < 18; ++i)
	{
		m_pHoleData[i].SetHoleNum(i + 1);
	}
}

CGhostDocument::~CGhostDocument()
{
	if (!m_exceptionLog.empty())
	{
		WriteExceptionLogFile();
	}
	if (m_pUserData)
	{
		delete m_pUserData;
		m_pUserData = NULL;
	}
	if (m_pHeader)
	{
		delete m_pHeader;
		m_pHeader = NULL;
	}
	if (m_pHoleData)
	{
		delete[] m_pHoleData;
		m_pHoleData = NULL;
	}
}

void CGhostDocument::RecordGameData()
{
	m_pHeader->m_map = GetCurMap();
	m_pHeader->m_gameType = CSharedDoc::Instance()->m_golfGame.gameType;
	m_pHeader->m_holeType = GetHoleType();
	m_pHeader->m_serverProperty = S5::UTIL::GetServerProperty();
	for (int i = 0; i < 18; ++i)
	{
		m_pHeader->m_holeScore[i] =
			CSharedDoc::Instance()->m_holeRandom[i];
		m_pHeader->m_holePar[i] = CSharedDoc::Instance()->m_holeOrder[i];
	}
}

void CGhostDocument::RecordUserData()
{
	const sUserInfo* pInfo = &CSharedDoc::Instance()->m_userInfo[MyIndex()];
	memset(m_pUserData, 0, sizeof(GHOST_USER_DATA));
	type_convert userConv(pInfo, m_pUserData);
	const std::list<sSCardAvilityPeriodInfo>& src =
		CSharedDoc::Instance()->m_cardAbilityList[MyIndex()];
	std::list<sSCardAvilityPeriodInfo> cards;
	std::copy(src.begin(), src.end(), std::back_inserter(cards));
	std::list<sSCardAvilityPeriodInfo>::iterator it = cards.begin();
	while (it != cards.end())
	{
		if ((*it).cardType == 0 || (*it).cardType == 1)
		{
			int i = 0;
			do
			{
				if ((*it).partsTid == pInfo->charInfo.tidParts[i] &&
					(*it).partsUid == pInfo->charInfo.ItemIdList[i])
				{
					goto keep;
				}
				++i;
			} while (i < 24);
			it = cards.erase(it);
			continue;
		}
keep:
		++it;
	}
	if (!cards.empty())
	{
		m_cards.reserve(cards.size());
		std::transform(cards.begin(), cards.end(), std::back_inserter(m_cards),
			ghost_card_converter());
	}
}

void CGhostDocument::RecordHoleData(const MsgObject& msg)
{
	CGhostHoleData* pHole =
		&m_pHoleData[CSharedDoc::Instance()->m_pGolfDoc->m_currentHole - 1];
	switch (msg.message)
	{
	case 0:
		pHole->RecordAct(0, 0, &msg.param1, 4);
		break;
	case M983(0x1):
		pHole->RecordAct(1, 0, &msg.param1, 1);
		break;
	case M983(0x2):
		pHole->RecordAct(2, 1, &msg.param1, 1);
		break;
	case M983(0x5):
		pHole->RecordAct(5, 2, &msg.param1, 2);
		if (msg.param1 == 0x1800000e)
		{
			pHole->RollBackShotData(1);
			--m_shotCount;
		}
		break;
	case M983(0x6):
		pHole->RecordAct(6, 0, &msg.param1, 1);
		break;
	case M983(0x7):
		{ const int* s7 = (const int*)msg.param1; int v7[3] = { s7[0], s7[1], s7[2] }; pHole->RecordAct(7, 1, v7, 3); }
		++m_shotCount;
		pHole->RecordDummyShot(m_shotCount);
		break;
	case M983(0x8):
		pHole->RecordAct(8, 0, NULL, 0);
		++m_shotCount;
		pHole->RecordDummyShot(m_shotCount);
		break;
	case M983(0xc):
		++m_shotCount;
		pHole->RecordShotData((const sShotProtocol*)msg.param1, m_shotCount);
		break;
	}
}

void CGhostDocument::RecordException(const char* text)
{
	if (text != NULL)
	{
		m_exceptionLog.push_back(std::string(text));
	}
}

bool CGhostDocument::WriteTextFile(const char* path) const
{
	if (strlen(m_pUserData->m_nick) == 0)
	{
		return false;
	}
	FILE* fp;
	if (path == NULL)
	{
		fp = fopen(MakeTextFileName().c_str(), "wt");
	}
	else
	{
		fp = fopen(path, "wt");
	}
	if (fp == NULL)
	{
		return false;
	}
	SYSTEMTIME st;
	GetLocalTime(&st);
	fprintf(fp,
		"=================================================================================\n");
	fprintf(fp, "SAVE DATE      : %04d-%02d-%02d %02d:%02d:%02d\n", st.wYear,
		st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	fprintf(fp, "DATA VERSION   : %d\n", 0x3e9);
	fprintf(fp, "HEADER SIZE    : %d\n", 0x1c);
	fprintf(fp,
		"DATA SIZE      : GHOST_GAME_DATA(%d), GHOST_USER_DATA(%d), GHOST_SHOT_DATA(%d)\n",
		100, 0x25c, 0x144);
	fprintf(fp,
		"---------------------------------------------------------------------------------\n");
	fprintf(fp, "USER NICK      : %s\n", m_pUserData->m_nick);
	fprintf(fp, "USER ID        : %s\n", m_pUserData->m_id);
	fprintf(fp, "GUILD          : %s\n",
		strlen(m_pUserData->m_guild) == 0 ? "-" : m_pUserData->m_guild);
	fprintf(fp, "EMBLEM         : %s\n",
		strlen(m_pUserData->m_emblem) == 0 ? "-" : m_pUserData->m_emblem);
	fprintf(fp, "IDENTITY       : %d\n", m_pUserData->m_identity);
	const char* text = "FEMALE";
	if (m_pUserData->m_gender == 0)
	{
		text = "MALE";
	}
	fprintf(fp, "GENDER         : %s\n", text);
	fprintf(fp, "ANGEL WING     : %d\n", m_pUserData->m_angelWing);
	fprintf(fp, "ANGEL EFFECT   : %d\n", m_pUserData->m_angelEffect);
	fprintf(fp, "DEVIL WING     : %d\n", m_pUserData->m_devilWing);
	int level = m_pUserData->m_level;
	int levelIndex = level < 0 ? 0 : (level > 0x46 ? 0x46 : level);
	fprintf(fp, "LEVEL          : %d(%s)\n", level,
		g_LevelTable[levelIndex].name);
	fprintf(fp,
		"---------------------------------------------------------------------------------\n");
	fprintf(fp, "CHAR TYPEID    : %d(%s)\n", m_pUserData->m_charTypeId,
		ItemManager()->GetItemName(m_pUserData->m_charTypeId));
	fprintf(fp, "HAIR COLOR     : %d\n", m_pUserData->m_hairColor);
	fprintf(fp, "SHIRTS COLOR   : %d\n", m_pUserData->m_shirtsColor);
	for (int i = 0; i < 24; ++i)
	{
		if (m_pUserData->m_parts[i] != 0)
		{
			fprintf(fp, "PARTS(%02d)      : %d(%s)\n", i,
				m_pUserData->m_parts[i],
				ItemManager()->GetItemName(m_pUserData->m_parts[i]));
		}
	}
	for (int i = 0; i < 5; ++i)
	{
		if (m_pUserData->m_auxParts[i] != 0)
		{
			fprintf(fp, "AUXPARTS(%02d)   : %d(%s)\n", i,
				m_pUserData->m_auxParts[i],
				ItemManager()->GetItemName(m_pUserData->m_auxParts[i]));
		}
	}
	fprintf(fp, "CHAR PCL       : ");
	for (int i = 0; i < 5; ++i)
	{
		fprintf(fp, "%d ", m_pUserData->m_charPcl[i]);
	}
	fprintf(fp, "\n");
	fprintf(fp, "CLUB TYPEID    : %d(%s)\n", m_pUserData->m_clubTypeId,
		ItemManager()->GetItemName(m_pUserData->m_clubTypeId));
	fprintf(fp, "CLUB PCL       : ");
	for (int i = 0; i < 5; ++i)
	{
		fprintf(fp, "%d ", m_pUserData->m_clubPcl[i]);
	}
	fprintf(fp, "\n");
	fprintf(fp, "BALL TYPEID    : %d(%s)\n", m_pUserData->m_ballTypeId,
		ItemManager()->GetItemName(m_pUserData->m_ballTypeId));
	text = m_pUserData->m_caddieTypeId != 0
		? ItemManager()->GetItemName(m_pUserData->m_caddieTypeId)
		: "NULL";
	fprintf(fp, "CADDIE TYPEID  : %d(%s)\n", m_pUserData->m_caddieTypeId, text);
	text = m_pUserData->m_caddieParts != 0
		? ItemManager()->GetItemName(m_pUserData->m_caddieParts)
		: "NULL";
	fprintf(fp, "CADDIE PARTS   : %d(%s)\n", m_pUserData->m_caddieParts, text);
	fprintf(fp, "CADDIE LEVEL   : %d\n", m_pUserData->m_caddieLevel);
	text = m_pUserData->m_mascotTypeId != 0
		? ItemManager()->GetItemName(m_pUserData->m_mascotTypeId)
		: "NULL";
	fprintf(fp, "MASCOT TYPEID  : %d(%s)\n", m_pUserData->m_mascotTypeId, text);
	fprintf(fp, "MASCOT MSG     : %s\n",
		strlen(m_pUserData->m_mascotMsg) == 0 ? "-" : m_pUserData->m_mascotMsg);
	for (int i = 0; i < 10; ++i)
	{
		if (m_pUserData->m_itemSlot[i] != 0)
		{
			fprintf(fp, "ITEMSLOT(%02d)   : %d(%s)\n", i,
				m_pUserData->m_itemSlot[i],
				ItemManager()->GetItemName(m_pUserData->m_itemSlot[i]));
		}
	}
	fprintf(fp,
		"---------------------------------------------------------------------------------\n");
	fprintf(fp, "CARD SIZE      : %d\n", m_cards.size());
	std::for_each(m_cards.begin(), m_cards.end(), print_ghost_card_data(fp));
	fprintf(fp,
		"---------------------------------------------------------------------------------\n");
	unsigned char map = m_pHeader->m_map;
	unsigned int course = (map > 0x7f && map != 0xfd) ? map - 0x80 : map;
	fprintf(fp, "COURSE         : %s\n",
		ItemManager()->GetItemName(course | 0x28000000));
	switch (m_pHeader->m_gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 14:
		text = "30S_BATTLE";
		break;
	default:
		text = "STROKE";
	}
	fprintf(fp, "GAME TYPE      : %d(%s)\n", m_pHeader->m_gameType, text);
	fprintf(fp, "HOLE TYPE      : %d\n", m_pHeader->m_holeType);
	fprintf(fp, "HOLE INDEX     : ");
	for (int i = 0; i < 18; ++i)
	{
		fprintf(fp, "%d ", m_pHeader->m_holePar[i]);
	}
	fprintf(fp, "\n");
	fprintf(fp,
		"---------------------------------------------------------------------------------\n");
	for (int i = 0; i < 18; ++i)
	{
		const std::list<GHOST_SHOT_DATA*>& shots =
			m_pHoleData[i].GetShotDataList();
		for (std::list<GHOST_SHOT_DATA*>::const_iterator it = shots.begin();
			it != shots.end(); ++it)
		{
			WriteTextFile(fp, *it);
		}
	}
	fclose(fp);
	return true;
}

bool CGhostDocument::WriteTextFile(FILE* fp, const CGhostHoleData* hole) const
{
	const std::list<GHOST_SHOT_DATA*>& shots = hole->GetShotDataList();
	for (std::list<GHOST_SHOT_DATA*>::const_iterator it = shots.begin();
		it != shots.end(); ++it)
	{
		WriteTextFile(fp, *it);
	}
	return true;
}

bool CGhostDocument::WriteTextFile(FILE* fp, const GHOST_SHOT_DATA* shot) const
{
	fprintf(fp, "HOLE INDEX     : %d\n", shot->m_holeIndex);
	fprintf(fp, "SHOT INDEX     : %d\n", shot->m_frame);
	fprintf(fp, "DUMMY SHOT     : %s\n", shot->m_valid ? "TRUE" : "FALSE");
	fprintf(fp, "MAX CLICK      : %.02f\n", shot->m_protocol.m_maxClick);
	fprintf(fp, "MIN CLICK      : %.02f\n", shot->m_protocol.m_minClick);
	fprintf(fp, "CURVE, SPIN    : %.02f, %.02f\n", shot->m_protocol.m_curve,
		shot->m_protocol.m_spin);
	fprintf(fp, "ACCURACY       : %d\n", shot->m_protocol.m_accuracy);
	fprintf(fp, "SPECIAL        : %d\n", shot->m_protocol.m_special);
	fprintf(fp, "FRAME          : %d\n", shot->m_protocol.m_frame);
	fprintf(fp, "SHOT ANGLE     : %.02f\n", shot->m_protocol.m_shotAngle);
	fprintf(fp, "SHOT TIME      : %d\n", shot->m_protocol.m_shotTime);
	fprintf(fp, "CENTER         : %.02f\n", shot->m_protocol.m_center);
	fprintf(fp, "CLUB INDEX     : %d(%s)\n", shot->m_protocol.m_club,
		GolfClub().GetClubString((eClub)shot->m_protocol.m_club));
	fprintf(fp, "RANDOM POWER   : %.02f\n", shot->m_protocol.m_randomPower);
	fprintf(fp, "RANDOM ANGLE   : %.02f\n", shot->m_protocol.m_randomAngle);
	fprintf(fp, "CAMERA ANGLE   : %.02f\n", shot->m_cameraAngle);
	fprintf(fp, "\n");
	int i = 0;
	const GHOST_ACT_DATA* act = shot->m_acts;
	for (; i < 9; ++i)
	{
		if (act->m_used != 0)
		{
			fprintf(fp, "\tACTION : [%s] ",
				GetActionString((GHOST::eGHOSTACTMSG)i));
			for (int j = 0; j < 5; ++j)
			{
				if (act->m_args[j] != -1)
				{
					if (act->m_type == 1)
					{
						float value;
						py_data_cast(act->m_args[j], value);
						fprintf(fp, "%.02f ", value);
					}
					else
					{
						fprintf(fp, "%d ", act->m_args[j]);
					}
				}
			}
			fprintf(fp, "\n");
		}
		++act;
	}
	fprintf(fp, "\n");
	fprintf(fp, "\t\tBALL POSITION : %.02f, %.02f, %.02f\n", shot->m_ballPos[0],
		shot->m_ballPos[1], shot->m_ballPos[2]);
	fprintf(fp, "\t\tWIND DIR : %.02f\n", shot->m_windDirection);
	fprintf(fp, "\t\tWIND PWR : %.02f\n", shot->m_windIntensity);
	fprintf(fp, "\t\tGLOBAL WIND : %.02f, %.02f, %.02f\n", shot->m_windVec[0],
		shot->m_windVec[1], shot->m_windVec[2]);
	fprintf(fp,
		"---------------------------------------------------------------------------------\n");
	return true;
}

std::string CGhostDocument::MakeTextFileName() const
{
	if (0)
		((CGhostDocument*)0)
			->CGhostDocument::WriteTextFile((FILE*)0, (const CGhostHoleData*)0);
	char name[260] = { 0 };
	sprintf(name, "ghost_save_data(%s).txt",
		(char*)CSharedDoc::Instance() + DOCOFS983(0x7b8));
	return std::string(name);
}

const CGhostHoleData* CGhostDocument::GetHoleData(unsigned char holeNum) const
{
	const CGhostHoleData* pHole = NULL;
	if (m_pHoleData)
	{
		pHole = &m_pHoleData[holeNum] - 1;
	}
	return pHole;
}

memory_buffer::memory_buffer(unsigned int size)
{
	m_pBuffer = NULL;
	m_length = 0;
	m_readPos = 0;
	reserve(size);
}

memory_buffer::memory_buffer(char* data, unsigned int size)
{
	m_pBuffer = NULL;
	m_length = 0;
	m_readPos = 0;
	m_capacity = size;
	m_pBuffer = calloc(size, 1);
	encode(data, size);
}

memory_buffer::~memory_buffer()
{
	if (m_pBuffer)
	{
		free(m_pBuffer);
	}
}

unsigned int memory_buffer::reserve(unsigned int size)
{
	m_capacity = size;
	if (m_pBuffer == NULL)
	{
		m_pBuffer = calloc(size, 1);
	}
	else
	{
		m_pBuffer = realloc(m_pBuffer, size);
	}
	return m_capacity;
}

__forceinline void memory_buffer::encode(const void* data, unsigned int size)
{
	if (m_length + size > m_capacity)
	{
		m_capacity = 1;
		while (m_capacity < m_length + size)
		{
			m_capacity <<= 1;
		}
		unsigned int cap = m_capacity;
		void* pBuf = m_pBuffer;
		if (pBuf == NULL)
		{
			m_pBuffer = calloc(cap, 1);
		}
		else
		{
			m_pBuffer = realloc(pBuf, cap);
		}
	}
	memcpy((char*)m_pBuffer + m_length, data, size);
	m_length += size;
}

__forceinline void memory_buffer::decode(void* data, unsigned int size)
{
	memcpy(data, (char*)m_pBuffer + m_readPos, size);
	m_readPos += size;
}

const void* memory_buffer::get_buffer() const
{
	return m_pBuffer;
}

unsigned int memory_buffer::length() const
{
	return m_length;
}

void CGhostDocument::Export(const std::string& path)
{
	GHOST_DATA_HEADER header;
	header.m_version = 0x3e9;
	header.m_size4 = 0x25c;
	header.m_size8 = 100;
	header.m_shotUnitSize = 0x144;
	header.m_shotCount = 0;
	header.m_cardUnitSize = 0x20;
	header.m_cardCount = m_cards.size();
	for (int hole = 0; hole < 18; ++hole)
	{
		header.m_shotCount += m_pHoleData[hole].m_shotDataList.size();
	}
	unsigned int bufferSize =
		header.GetDataBodySize() + sizeof(GHOST_DATA_HEADER);
	calloc(bufferSize, 1);
	memory_buffer buffer(bufferSize);
	unsigned int value = 0x3e9;
	buffer.encode(&value, 4);
	value = sizeof(header);
	buffer.encode(&value, 4);
	buffer.encode(&header, value);
	buffer.encode(m_pUserData, 0x25c);
	buffer.encode(m_pHeader, 100);
	for (int hole = 0; hole < 18; ++hole)
	{
		const CGhostHoleData* pHole = &m_pHoleData[hole];
		int shots = pHole->m_shotDataList.size();
		buffer.encode(&shots, 4);
		for (std::list<GHOST_SHOT_DATA*>::const_iterator it =
				 pHole->m_shotDataList.begin();
			it != pHole->m_shotDataList.end(); ++it)
		{
			buffer.encode(*it, 0x144);
		}
	}
	value = m_cards.size();
	buffer.encode(&value, 4);
	if (value != 0)
	{
		buffer.encode(&m_cards[0], m_cards.size() << 5);
	}
	HZIP hz = CreateZipZ((void*)path.c_str(), 0, 2, 0);
	CreateDirectoryA(".\\temporary_save\\", NULL);
	ZipAdd(hz, "ghost_document_data", buffer.m_pBuffer, buffer.m_length, 3);
	std::string logPath(".\\temporary_save\\");
	logPath += GHOST_EXCEPTION_LOG_FILENAME;
	if (_access(logPath.c_str(), 0) != -1)
	{
		ZipAdd(hz, GHOST_EXCEPTION_LOG_FILENAME, (void*)logPath.c_str(), 0, 2);
	}
	if (IsZipHandleU(hz))
	{
		CloseZipU(hz);
	}
	else
	{
		CloseZipZ(hz);
	}
	DeleteFileA(logPath.c_str());
	RemoveDirectoryA(".\\temporary_save\\");
}

bool CGhostDocument::Import(const std::string& path)
{
	cFile* pFile = GetCFile(path.c_str(), -1, 0);
	if (pFile == NULL)
	{
		return false;
	}
	unsigned int size = pFile->m_nLen;
	unsigned char* pData = new unsigned char[size];
	pFile->Read(pData, size);
	CloseCFile(pFile);
	bool result = Import(pData, size);
	delete[] pData;
	return result;
}

bool CGhostDocument::Import(unsigned char* data, unsigned int size)
{
	if (data == NULL || size == 0)
	{
		return false;
	}
	GHOST_DATA_HEADER header;
	int index = 0;
	int hole;
	GHOST_SHOT_DATA* copy;
	unsigned int count;
	ZIPENTRY ze;
	HZIP hz = OpenZipU((void*)data, size, ZIP_MEMORY);
	if (FindZipItemA(hz, "ghost_document_data", true, &index, &ze) != ZR_OK)
	{
		if (IsZipHandleU(hz))
		{
			CloseZipU(hz);
			return false;
		}
		CloseZipZ(hz);
		return false;
	}
	char* unzipped = new char[ze.unc_size];
	UnzipItem(hz, index, unzipped, ze.unc_size, ZIP_MEMORY);
	if (IsZipHandleU(hz))
	{
		CloseZipU(hz);
	}
	else
	{
		CloseZipZ(hz);
	}
	memory_buffer buffer(unzipped, ze.unc_size);
	delete[] unzipped;
	int magic = *(int*)((char*)buffer.m_pBuffer + buffer.m_readPos);
	buffer.m_readPos += 4;
	if (magic != 0x3e9)
	{
		return false;
	}
	unsigned int headerSize =
		*(unsigned int*)((char*)buffer.m_pBuffer + buffer.m_readPos);
	buffer.m_readPos += 4;
	memset(&header, 0, sizeof(header));
	memcpy(&header, (char*)buffer.m_pBuffer + buffer.m_readPos, headerSize);
	buffer.m_readPos += headerSize;
	buffer.decode(m_pUserData, header.m_size4);
	buffer.decode(m_pHeader, header.m_size8);
	for (hole = 0; hole < 18; ++hole)
	{
		count = *(unsigned int*)((char*)buffer.m_pBuffer + buffer.m_readPos);
		buffer.m_readPos += 4;
		while (count != 0)
		{
			GHOST_SHOT_DATA shot;
			memset(&shot, 0, sizeof(shot));
			memcpy(&shot, (char*)buffer.m_pBuffer + buffer.m_readPos,
				header.m_shotUnitSize);
			buffer.m_readPos += header.m_shotUnitSize;
			copy = new GHOST_SHOT_DATA;
			*copy = shot;
			m_pHoleData[hole].m_shotDataList.push_back(copy);
			--count;
		}
	}
	count = *(unsigned int*)((char*)buffer.m_pBuffer + buffer.m_readPos);
	buffer.m_readPos += 4;
	if (count != 0)
	{
		m_cards.reserve(count);
		for (unsigned int i = 0; i < count; ++i)
		{
			GHOST_CARD_DATA card;
			memset(&card, 0, sizeof(card));
			memcpy(&card, (char*)buffer.m_pBuffer + buffer.m_readPos,
				header.m_cardUnitSize);
			buffer.m_readPos += header.m_cardUnitSize;
			m_cards.push_back(card);
		}
	}
	return true;
}

bool CGhostDocument::WriteExceptionLogFile() const
{
	return true;
}
