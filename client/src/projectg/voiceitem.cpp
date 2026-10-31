#include "minatl.h"
#include "voiceitem.h"
#include "gamedatadb.h"
#include "gdvoiceitem.h"
#include "bonusticket.h"
#include "wresrcmng.h"
#include "club.h"
#include "golfball.h"
#include "soundmanager.h"

extern WResourceManager* g_resrcmng;
extern const char* g_plSound[];

inline void sVOICECLUBINSERT::operator()(const sGD_VOICEITEM& item) const
{
	CVoiceItem::Instance()->SetSound(item.typeID, item.name, item.state,
		(unsigned char)item.numOfSounds);
}

inline void sVOICECLUBVOICEINSERT::operator()(const sGD_VOICEITEM& item) const
{
	CVoiceItem::Instance()->SetClubVoice(item.typeID, item.name, item.state,
		item.numOfSounds, item.charIndex);
}

CVoiceItem::CVoiceItem()
{
}

CVoiceItem::~CVoiceItem()
{
}

bool CVoiceItem::PlaySound(unsigned long tid, eWaveState state, int charType)
{
	switch (tid >> 26)
	{
	case 4:
		return ClubPlaySound(tid, state, charType);
	case 5:
		return AztecPlaySound(tid, state);
	}

	return false;
}

void CVoiceItem::Initialize()
{
	CGDVoiceItem* pData = (CGDVoiceItem*)CGameDataDB::Instance()->GetGameData(
		GAMEDATA_VOICEITEM, "VoiceClub");
	if (pData)
	{
		std::for_each(pData->GetVoiceList().begin(),
			pData->GetVoiceList().end(), sVOICECLUBVOICEINSERT());
	}

	pData = (CGDVoiceItem*)CGameDataDB::Instance()->GetGameData(
		GAMEDATA_VOICEITEM, "SoundEffectClub");
	if (pData)
	{
		std::for_each(pData->GetVoiceList().begin(),
			pData->GetVoiceList().end(), sVOICECLUBINSERT());
	}
}

void CVoiceItem::Destroy()
{
	std::multimap<unsigned long, cVoice*>::iterator it = m_voiceMap.begin();
	while (it != m_voiceMap.end())
	{
		if (it->second)
		{
			delete it->second;
			it->second = NULL;
			m_voiceMap.erase(it++);
		}
		else
		{
			++it;
		}
	}

	m_voiceMap.clear();

	it = m_effectMap.begin();
	while (it != m_effectMap.end())
	{
		if (it->second)
		{
			delete it->second;
			it->second = NULL;
			m_effectMap.erase(it++);
		}
		else
		{
			++it;
		}
	}

	m_effectMap.clear();
}

void CVoiceItem::SetSound(unsigned long tid, std::string name,
	unsigned long state, unsigned char num)
{
	cVoice* pVoice = new cVoice;
	pVoice->state = state;
	pVoice->name = name;
	pVoice->num = num;

	m_effectMap.insert(
		std::multimap<unsigned long, cVoice*>::value_type(tid, pVoice));
}

// 983 00859830: som padrao de cada estado quando o clubset nao tem o seu
static void DefaultClubSfx983(unsigned long state, std::string& name)
{
	switch (state)
	{
	case 0x2: name = "swing_tee_s"; break;
	case 0x4: name = "swing_tee_w"; break;
	case 0x8: name = "swing_drive_s"; break;
	case 0x10: name = "swing_drive_w"; break;
	case 0x20: name = "swing_normal_s"; break;
	case 0x40: name = "swing_normal_w"; break;
	case 0x80:
	case 0x100: name = "swing_putting"; break;
	case 0x200: name = "swing_vain"; break;
	case 0x400: name = "swing_miss_wood"; break;
	case 0x800:
	case 0x1000: name = "swing_miss_iron"; break;
	case 0x2000: name = "swing_powershot_effect"; break;
	case 0x4000: name = "swing_cobra"; break;
	case 0x8000: name = "swing_tomahawk"; break;
	case 0x10000: name = "swing_spike"; break;
	}
}

void CVoiceItem::PlayClubSfx983(unsigned long tid, unsigned long state)
{
	std::string name("");
	cVoice* pVoice = NULL;

	std::multimap<unsigned long, cVoice*>::iterator it = m_effectMap.lower_bound(tid);
	std::multimap<unsigned long, cVoice*>::iterator end = m_effectMap.upper_bound(tid);
	for (; it != end; ++it)
	{
		if (it->second->state & state)
		{
			pVoice = it->second;
			name = pVoice->name;
			break;
		}
	}

	if (name.empty())
	{
		DefaultClubSfx983(state, name);
	}
	else
	{
		char buf[0x80];
		memset(buf, 0, sizeof(buf));
		if (pVoice->num > 1)
		{
			int r = rand();
			if (strstr(name.c_str(), "%d"))
				sprintf(buf, name.c_str(), r % pVoice->num);
			name = buf;
		}
	}

	g_audio->PlaySfx(name.c_str(), NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

void CVoiceItem::SetClubVoice(unsigned long tid, std::string name,
	unsigned long state, int num, int kind)
{
	cVoice* pVoice = new cVoice;
	pVoice->state = state;
	pVoice->name = name;
	pVoice->num = num;
	pVoice->kind = kind;

	m_voiceMap.insert(
		std::multimap<unsigned long, cVoice*>::value_type(tid, pVoice));
}

bool CVoiceItem::ClubPlaySound(unsigned long tid, eWaveState state,
	int charType)
{
	std::string soundName("");
	cVoice* pVoice = NULL;

	std::multimap<unsigned long, cVoice*>::iterator it = m_voiceMap.lower_bound(tid);
	if (it != m_voiceMap.end())
	{
		while (it != m_voiceMap.upper_bound(tid))
		{
			cVoice* p = it->second;
			if (p->state & state)
			{
				pVoice = p;
				soundName = pVoice->name;
				break;
			}
			++it;
		}
	}

	if (soundName.empty())
		return false;

	if (charType == -1)
	{
		charType = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid &
			0x3ffffff;
	}

	if (pVoice->kind == -2)
		return PlayVoiceNoChar(soundName, charType, pVoice);
	if (pVoice->kind == -1)
		return PlayVoiceChar(soundName, charType, pVoice);
	if (pVoice->kind >= 0 && pVoice->kind < 15)
		return PlayVoiceKind(soundName, charType, pVoice);

	return PlayVoiceByType(soundName, charType, tid, pVoice, state);
}

bool CVoiceItem::PlayVoiceNoChar(std::string name, int charType,
	cVoice* pVoice)
{
	char buf[128];
	memset(buf, 0, sizeof(buf));

	if (pVoice->num == -1)
		sprintf(buf, name.c_str());
	else
		sprintf(buf, name.c_str(), rand() % pVoice->num);

	if (!IsExistVoiceFile(buf))
		return false;

	g_audio->PlaySfx(buf);
	return true;
}

bool CVoiceItem::PlayVoiceChar(std::string name, int charType, cVoice* pVoice)
{
	char buf[128];
	memset(buf, 0, sizeof(buf));

	if (pVoice->num == -1)
		sprintf(buf, name.c_str(), charType);
	else
		sprintf(buf, name.c_str(), charType, rand() % pVoice->num);

	if (!IsExistVoiceFile(buf))
		return false;

	g_audio->PlaySfx(buf);
	return true;
}

bool CVoiceItem::PlayVoiceKind(std::string name, int charType, cVoice* pVoice)
{
	char buf[128];
	memset(buf, 0, sizeof(buf));

	if (pVoice->kind == charType)
	{
		if (pVoice->num == -1)
		{
			sprintf(buf, name.c_str());
		}
		else
		{
			int count = 0;
			std::string fmt(name);
			std::string token("%d");
			size_t pos = fmt.find("%d", 0, 2);
			while (pos != std::string::npos)
			{
				fmt.erase(pos, token.size());
				++count;
				pos = fmt.find("%d", 0, 2);
			}

			if (count == 1)
				sprintf(buf, name.c_str(), rand() % pVoice->num);
			else if (count == 2)
				sprintf(buf, name.c_str(), pVoice->kind, rand() % pVoice->num);
			else
				sprintf(buf, name.c_str());
		}

		if (!IsExistVoiceFile(buf))
			return false;

		g_audio->PlaySfx(buf);
		return true;
	}

	return false;
}

bool CVoiceItem::PlayVoiceByType(std::string name, int charType,
	unsigned long tid, cVoice* pVoice, eWaveState state)
{
	char buf[128];
	memset(buf, 0, sizeof(buf));

	unsigned long index = tid & 0x3ffffff;
	switch (index)
	{
	case 0x1a:
	{
		if (charType != 5)
			return false;

		int r;
		if (IsMassGame())
			r = rand();
		else
			r = CBonusTicket::Instance().GetRandomTicket("VOICE_CLUB");
		int idx = r % 2;
		sprintf(buf, name.c_str(), 5, idx);
		break;
	}

	case 0x17:
		if (state == WAVE_SHOT)
		{
			int r;
			if (IsMassGame())
				r = rand();
			else
				r = CBonusTicket::Instance().GetRandomTicket("VOICE_CLUB");
			int idx = r % 2;
			sprintf(buf, name.c_str(), charType, idx);
		}
		else
		{
			sprintf(buf, name.c_str(), charType);
		}
		break;

	case 0x1f:
	{
		int idx = rand() % 2;
		sprintf(buf, name.c_str(), charType, idx);
		break;
	}

	case 0x2d:
		if (charType >= 15)
			return false;

		if (state != 0x40 && state != 0x200)
			sprintf(buf, name.c_str(), charType, rand() % 2);
		else
			sprintf(buf, name.c_str(), charType, 0);
		break;

	case 0x44:
	case 0x45:
	{
		std::string fmt(name);
		if (charType == 8)
		{
			std::string to(index == 0x44 ? "negima_club_a_" : "negima_club_b_");
			std::string from("negima_club_");
			str_Replace(fmt, from, to);
		}

		int idx = 0;
		if (pVoice->num > 1)
			idx = rand() % pVoice->num;
		sprintf(buf, fmt.c_str(), charType, idx);
		break;
	}

	case 0x4e:
	case 0x4f:
	{
		std::string fmt(name);
		if (charType == 1 || charType == 12 || charType == 9)
		{
			std::string to(
				index == 0x4e ? "monogatari_club_a_" : "monogatari_club_b_");
			std::string from("monogatari_club_");
			str_Replace(fmt, from, to);
		}

		int idx = 0;
		if (pVoice->num > 1)
			idx = rand() % pVoice->num;
		sprintf(buf, fmt.c_str(), charType, idx);
		break;
	}
	}

	if (!IsExistVoiceFile(buf))
		return false;

	g_audio->PlaySfx(buf);
	return true;
}

bool CVoiceItem::AztecPlaySound(unsigned long tid, eWaveState state)
{
	std::string soundName("");

	std::multimap<unsigned long, cVoice*>::iterator it = m_voiceMap.find(tid);
	if (it != m_voiceMap.end())
	{
		while (it != m_voiceMap.upper_bound(tid))
		{
			cVoice* p = it->second;
			if (p->state & state)
			{
				soundName = p->name;
				break;
			}
			++it;
		}
	}

	if (soundName.empty())
		return false;

	g_audio->PlaySfx(soundName.c_str());
	return true;
}

bool CVoiceItem::IsExistVoiceFile(char* name)
{
	const char* file = MakeStr("%s.wav", name);
	if (_stricmp(file, g_resrcmng->FindMatchFile(file)))
		return true;
	return false;
}

void CSoundItemManager::PlayCurrentSoundItem()
{
	CClub& club = GolfClub();
	unsigned char powerShot = (unsigned char)club.m_powerShot;
	unsigned char clubType = club.m_pClub->type;
	unsigned long ballTid;
	unsigned long clubTid;
	int clubState;
	bool tailB;

	if (clubType > 1)
	{
		if (clubType != 2)
			return;

		ballTid = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall;
		if (CItemSoundTable::Instance().PlaySound(ballTid, 2, -1))
			return;

		clubTid = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid;
		if (ballTid == 0x1400000b)
		{
			tailB = true;
		}
		else
		{
			if (GolfBall().m_initSpecial & 0x10)
				return;
			tailB = false;
		}
	}
	else
	{
		ballTid = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall;
		if (GolfBall().m_initSpecial & 0x70)
		{
			int state = 2;
			if (ballTid == 0x140000d6 || ballTid == 0x140000c8 ||
				ballTid == 0x140000ce || ballTid == 0x140000cf)
			{
				if (GOLFDOC()->m_bar.grade == 4)
					state = 0x8000;
				else
					state = 0x10000;
			}

			if (CItemSoundTable::Instance().PlaySound(ballTid, state, -1))
				return;
			if (ballTid == 0x1400000b)
				return;

			clubTid = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid;
			if (GolfBall().m_initSpecial & 0x10 ||
				GolfBall().m_initSpecial & 0x20 ||
				GolfBall().m_initSpecial & 0x40)
			{
				tailB = true;
			}
			else
			{
				clubState = 2;
				goto playClubVoice;
			}
		}
		else
		{
			if (CItemSoundTable::Instance().PlaySound(ballTid, 2, -1))
				return;

			clubTid = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid;
			if (ballTid == 0x1400000b)
			{
				tailB = true;
			}
			else
			{
				if (GolfBall().m_initSpecial & 0x10)
					return;
				if (GolfBall().m_initSpecial & 0x20)
					return;
				if (GolfBall().m_initSpecial & 0x40)
					return;
				tailB = false;
			}
		}
	}

	clubState = 2; // 983 0085a65c/0085a6fb: powerShot 1 e 3 = 4, 2 = 8
	if (powerShot == 2)
		clubState = 8;
	else if (powerShot == 1 || powerShot == 3)
		clubState = 4;

playClubVoice:
	if (CVoiceItem::Instance()->PlaySound(clubTid, (CVoiceItem::eWaveState)clubState, -1))
		return;

	if (tailB)
	{
		if (WearWig(GOLFDOC()->m_currentPlayer))
			g_audio->PlaySfx(MakeStr("t_Ex8-%d", rand() % 4 + 1));
		if (WearHeadgear983(0, GOLFDOC()->m_currentPlayer))
			g_audio->PlaySfx("HG_Pangya_1");
	}
	else
	{
		if (WearHeadgear983(0, GOLFDOC()->m_currentPlayer))
			g_audio->PlaySfx("HG_Pangya_1");
		else
			g_audio->PlaySfx(g_plSound[6]);
	}
}

bool WearHeadgear983(int unused, unsigned char player)
{
	static unsigned long s_hatTid[4] = {
		0x08104011,
		0x810a00c,
		0x08100008,
		0x08112005,
	};

	CPartTidList* pList = &GOLFDOC()->GetPartTidList(player);
	if (!pList)
		return false;

	bool found = false;
	for (unsigned int i = 0; i < 4; ++i)
	{
		if (pList->IsEquipped(s_hatTid[i], 0))
		{
			found = true;
			break;
		}
	}

	return found;
}

bool WearWig(unsigned char player)
{
	static unsigned long s_wigTid[7] = {
		0x08000809,
		0x08040811,
		0x08080808,
		0x080c0003,
		0x08100005,
		0x08140003,
		0x08180005,
	};

	CPartTidList* pList = &GOLFDOC()->GetPartTidList(player);
	if (!pList)
		return false;

	unsigned long charIndex =
		Doc()->m_userInfo[player].charInfo.tid & 0x3ffffff;
	if (charIndex >= 7)
		return false;

	return pList->IsEquipped(s_wigTid[charIndex], 0);
}

// ---- CItemSoundTable (983 00b3a330..00b3b1e0) ----

CItemSoundTable& CItemSoundTable::Instance()
{
	static CItemSoundTable s_table;
	return s_table;
}

CItemSoundTable::CItemSoundTable()
{
	LoadGameData();

	AddFixed(0x14000023, 1, 0, 30);
	AddFixed(0x14000024, 1, 0, 30);
	AddFixed(0x14000026, 1, 0, 30);
	AddFixed(0x14000027, 1, 0, 30);
	AddFixed(0x14000028, 1, 0, 30);
	AddFixed(0x14000029, 1, 0, 30);
	AddFixed(0x1400002a, 1, 0, 30);
	AddFixed(0x1400002b, 1, 0, 30);
	AddFixed(0x1400002d, 1, 0, 30);
	AddFixed(0x14000005, 1, 0, 30);
	AddFixed(0x14000016, 1, 0, 30);
	AddFixed(0x14000041, 0, 0, 30);
	AddFixed(0x14000080, 1, 0, 30);
	AddFixed(0x14000087, 0, 0, 0);
	AddFixed(0x1400002e, 0, 0, 0);
	AddFixed(0x1400002f, 1, 0, 30);
	AddFixed(0x14000030, 1, 0, 30);
	AddFixed(0x14000035, 1, 0, 30);
	AddFixed(0x1400003c, 1, 0, 30);
	AddFixed(0x1400003d, 1, 0, 30);
	AddFixed(0x14000092, 1, 0, 30);
	AddFixed(0x14000093, 1, 0, 30);
	AddFixed(0x14000094, 1, 0, 30);
	AddFixed(0x14000095, 1, 0, 30);
	AddFixed(0x140000d6, 1, 0, 30);
	AddFixed(0x14000040, 0, 0, 30);
	AddFixed(0x14000055, 1, 0, 30);
}

void CItemSoundTable::LoadGameData()
{
	CGDVoiceItem* pData = (CGDVoiceItem*)CGameDataDB::Instance()->GetGameData(
		GAMEDATA_VOICEITEM, "VoiceAztec");
	if (!pData)
		return;

	std::list<sGD_VOICEITEM>& list = pData->GetVoiceList();
	for (std::list<sGD_VOICEITEM>::iterator it = list.begin(); it != list.end();
		++it)
		Add(it->typeID, it->name, it->state, it->numOfSounds);
}

int CItemSoundTable::GetFixed983(unsigned long typeID, int kind)
{
	std::map<unsigned long, sFixed*>::iterator it = m_fixed.find(typeID);
	if (it == m_fixed.end())
		return kind == 1 ? 30 : 0;
	switch (kind)
	{
	case 1:
		return it->second->a;
	case 2:
		return it->second->b;
	case 3:
		return it->second->c;
	}
	return 0;
}

void CItemSoundTable::AddFixed(unsigned long tid, int a, int b, int c)
{
	if (m_fixed.find(tid) != m_fixed.end())
		return;

	sFixed* p = new sFixed;
	p->a = a;
	p->b = b;
	p->c = c;
	m_fixed[tid] = p;
}

void CItemSoundTable::Add(unsigned long tid, const std::string& name,
	unsigned long state, int count)
{
	sEntry* pEntry;
	std::map<unsigned long, sEntry*>::iterator it = m_entries.find(tid);
	if (it == m_entries.end())
		pEntry = new sEntry;
	else
		pEntry = it->second;

	if (count == 1)
	{
		sSound s;
		s.state = state;
		s.name = name;
		pEntry->sounds.push_back(s);
	}
	else
	{
		// 983: sprintf(buf, name, i + 1) (o nome e o formato)
		for (int i = 0; i < count; ++i)
		{
			char buf[0x80];
			memset(buf, 0, sizeof(buf));
			sprintf(buf, name.c_str(), i + 1);
			sSound s;
			s.state = state;
			s.name = buf;
			pEntry->sounds.push_back(s);
		}
	}

	if (it == m_entries.end())
		m_entries[tid] = pEntry;
}

bool CItemSoundTable::PlayEntry(sEntry* pEntry, unsigned long state,
	int charType)
{
	std::vector<const sSound*> match;
	for (size_t i = 0; i < pEntry->sounds.size(); ++i)
	{
		if (pEntry->sounds[i].state & state)
			match.push_back(&pEntry->sounds[i]);
	}

	if (match.empty())
		return false;

	unsigned long idx = (unsigned long)charType;
	if (charType == -1)
		idx = (unsigned long)rand();
	idx %= (unsigned long)match.size();

	g_audio->PlaySfx(match[idx]->name.c_str(), NULL, 0, NULL, NULL, 0.5f,
		200.0f);
	return true;
}

bool CItemSoundTable::PlaySound(unsigned long typeID, int state, int charType)
{
	std::map<unsigned long, sEntry*>::iterator it = m_entries.find(typeID);
	if (it == m_entries.end())
		return false;

	return PlayEntry(it->second, (unsigned long)state, charType);
}

// 983 00b3ac70 -> 00b3a810: nome de um dos sons do tid cujo estado casa (aleatorio); vazio se nao ha
std::string CItemSoundTable::GetSoundName(unsigned long typeID, int state)
{
	std::map<unsigned long, sEntry*>::iterator it = m_entries.find(typeID);
	if (it == m_entries.end())
		return std::string();

	std::vector<const sSound*> match;
	for (size_t i = 0; i < it->second->sounds.size(); ++i)
	{
		if (it->second->sounds[i].state & (unsigned long)state)
			match.push_back(&it->second->sounds[i]);
	}
	if (match.empty())
		return std::string();
	return match[(unsigned long)rand() % match.size()]->name;
}
