#include "minatl.h"
#include "awarddlg.h"
#include "projectg.h"
#include "netresourcemanager.h"
#include "intrusion.h"
#include "../shared/localize.h"

// HACK: reuse gmtoolkit's original literal storage after nakedification.
#define rb_gmt_literal__08MPLGCJPF "recovery"
#define rb_gmt_literal__08NJIKOPLD "longputt"
#define rb_gmt_literal__06HPAPPIOI "chipin"
#define rb_gmt_literal__06OMGFCGFC "runner"
#define rb_gmt_literal__07LKNAHCPB "speeder"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrAwardDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrAwardDlg, FrForm)

ON_FRESH_VI("sgold", FRCMD_INIT, FrAwardDlg::OnGoldStaticInit)
ON_FRESH_VI("ssilver", FRCMD_INIT, FrAwardDlg::OnSilverStaticInit)
ON_FRESH_VI("sbronze", FRCMD_INIT, FrAwardDlg::OnBronzeStaticInit)
ON_FRESH_VI("winner1", FRCMD_INIT, FrAwardDlg::OnWinner1Init)
ON_FRESH_VI("winner1", FRCMD_OWNERDRAW, FrAwardDlg::OnWinner1OwnerDraw)
ON_FRESH_VI("winner2", FRCMD_INIT, FrAwardDlg::OnWinner2Init)
ON_FRESH_VI("winner2", FRCMD_OWNERDRAW, FrAwardDlg::OnWinner2OwnerDraw)
ON_FRESH_VI("winner3", FRCMD_INIT, FrAwardDlg::OnWinner3Init)
ON_FRESH_VI("winner3", FRCMD_OWNERDRAW, FrAwardDlg::OnWinner3OwnerDraw)
ON_FRESH_VI("winner4", FRCMD_INIT, FrAwardDlg::OnWinner4Init)
ON_FRESH_VI("winner4", FRCMD_OWNERDRAW, FrAwardDlg::OnWinner4OwnerDraw)
ON_FRESH_VI("winner5", FRCMD_INIT, FrAwardDlg::OnWinner5Init)
ON_FRESH_VI("winner5", FRCMD_OWNERDRAW, FrAwardDlg::OnWinner5OwnerDraw)
ON_FRESH_VI("winner6", FRCMD_INIT, FrAwardDlg::OnWinner6Init)
ON_FRESH_VI("winner6", FRCMD_OWNERDRAW, FrAwardDlg::OnWinner6OwnerDraw)
ON_FRESH_VI("luck", FRCMD_INIT, FrAwardDlg::OnLuckInit)
ON_FRESH_VI("luck", FRCMD_OWNERDRAW, FrAwardDlg::OnLuckOwnerDraw)
ON_FRESH_VI(rb_gmt_literal__07LKNAHCPB, FRCMD_INIT, FrAwardDlg::OnSpeederInit)
ON_FRESH_VI(rb_gmt_literal__07LKNAHCPB, FRCMD_OWNERDRAW,
	FrAwardDlg::OnSpeederOwnerDraw)
ON_FRESH_VI(rb_gmt_literal__06OMGFCGFC, FRCMD_INIT, FrAwardDlg::OnRunnerInit)
ON_FRESH_VI(rb_gmt_literal__06OMGFCGFC, FRCMD_OWNERDRAW,
	FrAwardDlg::OnRunnerOwnerDraw)
ON_FRESH_VI(rb_gmt_literal__06HPAPPIOI, FRCMD_INIT, FrAwardDlg::OnChipInInit)
ON_FRESH_VI(rb_gmt_literal__06HPAPPIOI, FRCMD_OWNERDRAW,
	FrAwardDlg::OnChipInOwnerDraw)
ON_FRESH_VI(rb_gmt_literal__08NJIKOPLD, FRCMD_INIT, FrAwardDlg::OnLongPuttInit)
ON_FRESH_VI(rb_gmt_literal__08NJIKOPLD, FRCMD_OWNERDRAW,
	FrAwardDlg::OnLongPuttOwnerDraw)
ON_FRESH_VI(rb_gmt_literal__08MPLGCJPF, FRCMD_INIT, FrAwardDlg::OnRecoveryInit)
ON_FRESH_VI(rb_gmt_literal__08MPLGCJPF, FRCMD_OWNERDRAW,
	FrAwardDlg::OnRecoveryOwnerDraw)

END_FRESH_MSGMAP()

void FrAwardDlg::OnGoldStaticInit(int param)
{
	FrStatic* text = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (text)
	{
		int count = 0;
		for (int i = 0; i < 6; ++i)
		{
			if (Doc()->m_awardItem[6 + i].uid != 0xffffffff)
			{
				if (GetMedalIndex(Doc()->m_awardItem[6 + i].uid) == 0)
					++count;
			}
		}
		if (count == 0)
			text->SetVisible(false);
		if (Doc()->m_golfGame.gameType == 6)
			text->SetCaption(
				K2L_Compatibility("\261\346\265\345\261\342\277\251\265\265 1\300\247 :"));
		else if (Doc()->m_golfGame.gameType == 10)
			text->SetCaption(K2L_Compatibility("1\300\247 :"));
	}
}

void FrAwardDlg::OnSilverStaticInit(int param)
{
	FrStatic* text = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (text)
	{
		int count = 0;
		for (int i = 0; i < 6; ++i)
		{
			if (Doc()->m_awardItem[6 + i].uid != 0xffffffff)
			{
				int medal = GetMedalIndex(Doc()->m_awardItem[6 + i].uid);
				if (medal == 0)
					++count;
				else if (medal == 1)
					count += 10;
			}
		}
		if (count > 10)
		{
			WRect rect;
			text->GetClientRect(rect);
			rect.y += (count % 10) * 25.0f;
			text->SetClientRect(rect);
		}
		else
			text->SetVisible(false);
		if (Doc()->m_golfGame.gameType == 6)
			text->SetCaption(
				K2L_Compatibility("\261\346\265\345\261\342\277\251\265\265 2\300\247 :"));
		else if (Doc()->m_golfGame.gameType == 10)
			text->SetCaption(K2L_Compatibility("2\300\247 :"));
	}
}

void FrAwardDlg::OnBronzeStaticInit(int param)
{
	FrStatic* text = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (text)
	{
		int count = 0;
		for (int i = 0; i < 6; ++i)
		{
			if (Doc()->m_awardItem[6 + i].uid != 0xffffffff)
			{
				int medal = GetMedalIndex(Doc()->m_awardItem[6 + i].uid);
				if (medal == 2)
					++count;
				else if (medal <= 1)
					count += 10;
			}
		}
		if (count > 10)
		{
			WRect rect;
			text->GetClientRect(rect);
			rect.y += (count % 10) * 25.0f;
			text->SetClientRect(rect);
		}
		else
			text->SetVisible(false);
		if (Doc()->m_golfGame.gameType == 6)
			text->SetCaption(
				K2L_Compatibility("\261\346\265\345\261\342\277\251\265\265 3\300\247 :"));
		else if (Doc()->m_golfGame.gameType == 10)
			text->SetCaption(K2L_Compatibility("3\300\247 :"));
	}
}

// 983 006335c0 (CSharedDoc, sem par no nosso shareddoc.cpp): indice de medalha pela contagem e posicao ajustada
static int MedalByCountRank(int count, int rank)
{
	if (count <= 14)
		return rank == 1 ? 2 : 3;
	if (count <= 18)
	{
		if (rank == 1)
			return 1;
		return rank == 2 ? 2 : 3;
	}
	if (count <= 22)
	{
		if (rank == 1)
			return 0;
		if (rank == 2)
			return 1;
		return rank == 3 ? 2 : 3;
	}
	if (count <= 26)
	{
		if (rank == 1)
			return 0;
		if (rank == 2)
			return 1;
		return rank <= 4 ? 2 : 3;
	}
	if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
	{
		if (rank == 1)
			return 0;
		if (rank == 2)
			return 1;
		return rank == 3 ? 2 : 3;
	}
	if (rank == 1)
		return 0;
	if (rank <= 3)
		return 1;
	return rank <= 6 ? 2 : 3;
}

int FrAwardDlg::GetMedalIndex(unsigned long uid)
{
	if (Doc()->m_golfGame.gameType == 10)
	{
		unsigned char index = Doc()->GetIndex(uid);
		if (index == 0xff)
			return 3;

		for (int i = 0; i < 6; ++i)
			if (*(unsigned long*)((char*)&Doc()->m_awardItem[6] + i * 12) ==
				Doc()->m_rivalList[index].oid)
				return Doc()->m_rivalList[index].rank;
	}
	else if (Doc()->m_golfGame.gameType == 6)
	{
		if (Doc()->m_rivalList.size() < 6)
			return 3;
		unsigned char index = Doc()->GetIndex(uid);
		if (index == 0xff)
			return 3;

		if (Doc()->m_rivalList[index].flag > 3)
			return 3;

		for (int i = 0; i < 6; ++i)
		{
			if (*(unsigned long*)((char*)&Doc()->m_awardItem[6] + i * 12) ==
				Doc()->m_rivalList[index].oid)
			{
				if (i <= 0)
					return 0;
				if (i < 3)
					return 1;
				if (i < 6)
					return 2;
				return 3;
			}
		}
	}
	else
	{
		unsigned char members = 0;
		if (IsLocalContent((localContentType_t)47 /*983: era 46; interpolado*/))
		{
			if (Doc()->m_golfGame.gameType == 4)
				members = CIntrusion::Instance()->GetCalcExpMemberSize();
			else
				members = 0;
		}
		if (!members)
		{
			if (Doc()->m_rivalList.size() < 10)
				return 3;
		}
		else if (members < 10)
			return 3;
		unsigned char index = Doc()->GetIndex(uid);
		if (index == 0xff)
			return 3;

		if (Doc()->m_rivalList[index].flag > 3)
			return 3;
		unsigned char rank = Doc()->m_rivalList[index].rank;
		unsigned char count = 0;
		unsigned char excluded = 0;
		if (!members)
		{
			std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			std::vector<sRivalData>::iterator end = Doc()->m_rivalList.end();
			for (; it != end; ++it)
			{
				unsigned char state = (*it).state;
				if ((state == 3 && GetHoleIndex((*it).hole) >= 4) ||
					(*it).state != 3)
					++count;
				if (state != 3 && (*it).rank < rank && (*it).flag > 3)
					++excluded;
			}
		}
		else
		{
			std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			std::vector<sRivalData>::iterator end = Doc()->m_rivalList.end();
			count = members;
			for (; it != end; ++it)
				if ((*it).state != 3 && (*it).rank < rank && (*it).flag > 3)
					++excluded;
		}
		rank -= excluded;
		// 983 00ac6bc2: tabela unica 006335c0, sem ramo por 9/18 buracos
		return MedalByCountRank(count, rank);
	}
	return 3;
}

void FrAwardDlg::OnWinner1Init(int param)
{
	m_pWinner1 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pWinner1->HidePrivacy(true);
}

void FrAwardDlg::OnWinner1OwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[6].uid);
		if (index != 0xff)
		{
			unsigned long color = Doc()->m_awardItem[6].uid == MyGuid(false)
				? 0xffff0000
				: 0xff000000;
			sRivalData& rival = Doc()->m_rivalList[index];
			float offset = 0.0f;
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pWinner1->GetRect().x + offset,
							m_pWinner1->GetRect().y + offset - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset += 27.0;
				}
			}
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pWinner1->GetRect().x + offset,
					m_pWinner1->GetRect().y + 0.0),
				0, rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrAwardDlg::OnWinner2Init(int param)
{
	m_pWinner2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pWinner2->HidePrivacy(true);
}

void FrAwardDlg::OnWinner2OwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[7].uid);
		if (index != 0xff)
		{
			unsigned long color = Doc()->m_awardItem[7].uid == MyGuid(false)
				? 0xffff0000
				: 0xff000000;
			sRivalData& rival = Doc()->m_rivalList[index];
			float offset = 0.0f;
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pWinner2->GetRect().x + offset,
							m_pWinner2->GetRect().y + offset - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset += 27.0;
				}
			}
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pWinner2->GetRect().x + offset,
					m_pWinner2->GetRect().y + 0.0),
				0, rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrAwardDlg::OnWinner3Init(int param)
{
	m_pWinner3 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pWinner3->HidePrivacy(true);
}

void FrAwardDlg::OnWinner3OwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[8].uid);
		if (index != 0xff)
		{
			unsigned long color = Doc()->m_awardItem[8].uid == MyGuid(false)
				? 0xffff0000
				: 0xff000000;
			sRivalData& rival = Doc()->m_rivalList[index];
			float offset = 0.0f;
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pWinner3->GetRect().x + offset,
							m_pWinner3->GetRect().y + offset - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset += 27.0;
				}
			}
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pWinner3->GetRect().x + offset,
					m_pWinner3->GetRect().y + 0.0),
				0, rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrAwardDlg::OnWinner4Init(int param)
{
	m_pWinner4 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pWinner4->HidePrivacy(true);
}

void FrAwardDlg::OnWinner4OwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[9].uid);
		if (index != 0xff)
		{
			unsigned long color = Doc()->m_awardItem[9].uid == MyGuid(false)
				? 0xffff0000
				: 0xff000000;
			sRivalData& rival = Doc()->m_rivalList[index];
			float offset = 0.0f;
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pWinner4->GetRect().x + offset,
							m_pWinner4->GetRect().y + offset - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset += 27.0;
				}
			}
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pWinner4->GetRect().x + offset,
					m_pWinner4->GetRect().y + 0.0),
				0, rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrAwardDlg::OnWinner5Init(int param)
{
	m_pWinner5 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pWinner5->HidePrivacy(true);
}

void FrAwardDlg::OnWinner5OwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[10].uid);
		if (index != 0xff)
		{
			unsigned long color = Doc()->m_awardItem[10].uid == MyGuid(false)
				? 0xffff0000
				: 0xff000000;
			sRivalData& rival = Doc()->m_rivalList[index];
			float offset = 0.0f;
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pWinner5->GetRect().x + offset,
							m_pWinner5->GetRect().y + offset - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset += 27.0;
				}
			}
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pWinner5->GetRect().x + offset,
					m_pWinner5->GetRect().y + 0.0),
				0, rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrAwardDlg::OnWinner6Init(int param)
{
	m_pWinner6 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pWinner6->HidePrivacy(true);
}

void FrAwardDlg::OnWinner6OwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[11].uid);
		if (index != 0xff)
		{
			unsigned long color = Doc()->m_awardItem[11].uid == MyGuid(false)
				? 0xffff0000
				: 0xff000000;
			sRivalData& rival = Doc()->m_rivalList[index];
			float offset = 0.0f;
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pWinner6->GetRect().x + offset,
							m_pWinner6->GetRect().y + offset - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset += 27.0;
				}
			}
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pWinner6->GetRect().x + offset,
					m_pWinner6->GetRect().y + 0.0),
				0, rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrAwardDlg::OnLuckInit(int param)
{
	m_pLuck = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardDlg::OnLuckOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[0].uid);
		unsigned long color = Doc()->m_awardItem[0].uid == MyGuid(false)
			? 0xffff0000
			: 0xff000000;
		float offset = 0.0f;
		const char* nick;
		if (index == 0xff)
			nick = K2L_Compatibility("    -    ");
		else
		{
			sRivalData& rival = Doc()->m_rivalList[index];
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pLuck->GetRect().x, m_pLuck->GetRect().y - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset = 27.0f;
				}
			}
			nick = rival.nickname;
		}
		if (CProjectG::Instance()->HidePrivacy() == false)
		{
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			// 983 00ac7580..00ac82f0: com vencedor, nome + " (%d \310\270)" do contador do premio (Doc+0x106e4)
			if (index == 0xff)
				g_pFresh->GetManager()->PrintText(WPoint(m_pLuck->GetRect().x + offset, m_pLuck->GetRect().y), 0,
					nick, -1.0f, 0xffffffff);
			else
			{
				std::string text(nick);
				text += MakeStr(K2L_Compatibility(" (%d \310\270)"), (&Doc()->m_guildRoundScore[0])[0]);
				g_pFresh->GetManager()->PrintText(WPoint(m_pLuck->GetRect().x + offset, m_pLuck->GetRect().y), 0,
					text.c_str(), -1.0f, 0xffffffff);
			}
		}
	}
}

void FrAwardDlg::OnSpeederInit(int param)
{
	m_pSpeeder = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardDlg::OnSpeederOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[1].uid);
		unsigned long color = Doc()->m_awardItem[1].uid == MyGuid(false)
			? 0xffff0000
			: 0xff000000;
		float offset = 0.0f;
		const char* nick;
		if (index == 0xff)
			nick = K2L_Compatibility("    -    ");
		else
		{
			sRivalData& rival = Doc()->m_rivalList[index];
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pSpeeder->GetRect().x,
							m_pSpeeder->GetRect().y - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset = 27.0f;
				}
			}
			nick = rival.nickname;
		}
		if (CProjectG::Instance()->HidePrivacy() == false)
		{
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			if (index == 0xff)
				g_pFresh->GetManager()->PrintText(WPoint(m_pSpeeder->GetRect().x + offset, m_pSpeeder->GetRect().y), 0,
					nick, -1.0f, 0xffffffff);
			else
			{
				std::string text(nick);
				text += MakeStr(K2L_Compatibility(" (%d \310\270)"), (&Doc()->m_guildRoundScore[0])[1]);
				g_pFresh->GetManager()->PrintText(WPoint(m_pSpeeder->GetRect().x + offset, m_pSpeeder->GetRect().y), 0,
					text.c_str(), -1.0f, 0xffffffff);
			}
		}
	}
}

void FrAwardDlg::OnRunnerInit(int param)
{
	m_pRunner = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardDlg::OnRunnerOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[2].uid);
		unsigned long color = Doc()->m_awardItem[2].uid == MyGuid(false)
			? 0xffff0000
			: 0xff000000;
		float offset = 0.0f;
		const char* nick;
		if (index == 0xff)
			nick = K2L_Compatibility("    -    ");
		else
		{
			sRivalData& rival = Doc()->m_rivalList[index];
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pRunner->GetRect().x,
							m_pRunner->GetRect().y - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset = 27.0f;
				}
			}
			nick = rival.nickname;
		}
		if (CProjectG::Instance()->HidePrivacy() == false)
		{
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			if (index == 0xff)
				g_pFresh->GetManager()->PrintText(WPoint(m_pRunner->GetRect().x + offset, m_pRunner->GetRect().y), 0,
					nick, -1.0f, 0xffffffff);
			else
			{
				std::string text(nick);
				text += MakeStr(K2L_Compatibility(" (%d \310\270)"), (&Doc()->m_guildRoundScore[0])[2]);
				g_pFresh->GetManager()->PrintText(WPoint(m_pRunner->GetRect().x + offset, m_pRunner->GetRect().y), 0,
					text.c_str(), -1.0f, 0xffffffff);
			}
		}
	}
}

void FrAwardDlg::OnChipInInit(int param)
{
	m_pChipIn = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardDlg::OnChipInOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[3].uid);
		unsigned long color = Doc()->m_awardItem[3].uid == MyGuid(false)
			? 0xffff0000
			: 0xff000000;
		float offset = 0.0f;
		const char* nick;
		if (index == 0xff)
			nick = K2L_Compatibility("    -    ");
		else
		{
			sRivalData& rival = Doc()->m_rivalList[index];
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pChipIn->GetRect().x,
							m_pChipIn->GetRect().y - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset = 27.0f;
				}
			}
			nick = rival.nickname;
		}
		if (CProjectG::Instance()->HidePrivacy() == false)
		{
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			if (index == 0xff)
				g_pFresh->GetManager()->PrintText(WPoint(m_pChipIn->GetRect().x + offset, m_pChipIn->GetRect().y), 0,
					nick, -1.0f, 0xffffffff);
			else
			{
				std::string text(nick);
				text += MakeStr(K2L_Compatibility(" (%d \310\270)"), (&Doc()->m_guildRoundScore[0])[3]);
				g_pFresh->GetManager()->PrintText(WPoint(m_pChipIn->GetRect().x + offset, m_pChipIn->GetRect().y), 0,
					text.c_str(), -1.0f, 0xffffffff);
			}
		}
	}
}

void FrAwardDlg::OnLongPuttInit(int param)
{
	m_pLongPutt = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardDlg::OnLongPuttOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[4].uid);
		unsigned long color = Doc()->m_awardItem[4].uid == MyGuid(false)
			? 0xffff0000
			: 0xff000000;
		float offset = 0.0f;
		const char* nick;
		if (index == 0xff)
			nick = K2L_Compatibility("    -    ");
		else
		{
			sRivalData& rival = Doc()->m_rivalList[index];
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pLongPutt->GetRect().x,
							m_pLongPutt->GetRect().y - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset = 27.0f;
				}
			}
			nick = rival.nickname;
		}
		if (CProjectG::Instance()->HidePrivacy() == false)
		{
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			if (index == 0xff)
				g_pFresh->GetManager()->PrintText(WPoint(m_pLongPutt->GetRect().x + offset, m_pLongPutt->GetRect().y), 0,
					nick, -1.0f, 0xffffffff);
			else
			{
				std::string text(nick);
				text += MakeStr(K2L_Compatibility(" (%d \310\270)"), (&Doc()->m_guildRoundScore[0])[4]);
				g_pFresh->GetManager()->PrintText(WPoint(m_pLongPutt->GetRect().x + offset, m_pLongPutt->GetRect().y), 0,
					text.c_str(), -1.0f, 0xffffffff);
			}
		}
	}
}

void FrAwardDlg::OnRecoveryInit(int param)
{
	m_pRecovery = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAwardDlg::OnRecoveryOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		unsigned char index = Doc()->GetIndex(Doc()->m_awardItem[5].uid);
		unsigned long color = Doc()->m_awardItem[5].uid == MyGuid(false)
			? 0xffff0000
			: 0xff000000;
		float offset = 0.0f;
		const char* nick;
		if (index == 0xff)
			nick = K2L_Compatibility("    -    ");
		else
		{
			sRivalData& rival = Doc()->m_rivalList[index];
			if (rival.guildUID)
			{
				const Bitmap* emblem =
					NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
				if (emblem)
				{
					gdi->DrawTexture(emblem,
						WRect(m_pRecovery->GetRect().x,
							m_pRecovery->GetRect().y - 6.0f,
							(float)emblem->Width(), (float)emblem->Height()),
						0xffffffff, 0);
					offset = 27.0f;
				}
			}
			nick = rival.nickname;
		}
		if (CProjectG::Instance()->HidePrivacy() == false)
		{
			gdi->SetTextColor(color, 0xffffffff);
			gdi->SetTextStyle(0);
			if (index == 0xff)
				g_pFresh->GetManager()->PrintText(WPoint(m_pRecovery->GetRect().x + offset, m_pRecovery->GetRect().y), 0,
					nick, -1.0f, 0xffffffff);
			else
			{
				std::string text(nick);
				text += MakeStr(K2L_Compatibility(" (%d \310\270)"), (&Doc()->m_guildRoundScore[0])[5]);
				g_pFresh->GetManager()->PrintText(WPoint(m_pRecovery->GetRect().x + offset, m_pRecovery->GetRect().y), 0,
					text.c_str(), -1.0f, 0xffffffff);
			}
		}
	}
}
