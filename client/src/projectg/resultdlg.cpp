#include "minatl.h"
#include "resultdlg.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "exhibition.h"
#include "netresourcemanager.h"
#include "projectg.h"
#include "wfont.h"
#include "scoredlg.h"
#include "user_info.h"
#include "userinfoform.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "treasuredlg.h"
#include "treasure.h"
#include "gmtoolkit.h"
#include "intrusion.h"
#include "../shared/sharedtables.h"
#include "mathconsts.h"
#include "taskmanager.h"
#include "s5/utilities.h"
#include "jp/shareddocjp.h"
#include "icontentsdoc.h"
#include "jp/holeinonegamedoc.h"
#include "jp/levelnametable.h"
#include <sstream>


#define RB_STR_BLUETEAM_RANK "30in_team_blueteam_rank"
#define RB_STR_REDTEAM_RANK "30in_team_redteam_rank"
#define RB_STR_30IN_RANK "30in_rank"

extern Fresh* g_pFresh;
extern "C" __declspec(dllimport) int __cdecl mkdir(const char*);
extern int __fastcall float2int(float value);

inline unsigned char MyIndex()
{
	bool bGM = (Doc()->m_myInfo.info.dwIdentity >> 1) & 1;
	if (bGM)
	{
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 0xb: // 983 0053a260 (MyIndex) inclui 0xb
			return Doc()->GetIndex(Doc()->m_myInfo.info.dwGalleryGuid);
		default:
			return GOLFDOC()->m_currentPlayer;
		}
	}
	return Doc()->GetIndex(Doc()->m_myInfo.info.dwGuid);
}

__forceinline FrUserInfoForm* USERINFODLG()
{
	if (CUserInfo::Instance())
		return CUserInfo::Instance()->GetDlg();
	return NULL;
}

IMPLEMENT_OBJECT(FrResultDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrResultDlg, FrForm)

ON_FRESH_VI("stage", FRCMD_INIT, FrResultDlg::OnStageInit)
ON_FRESH_VV("stage", FRCMD_OWNERDRAW, FrResultDlg::OnStageOwnerDraw)
ON_FRESH_VI("trophy", FRCMD_INIT, FrResultDlg::OnTrophyInit)
ON_FRESH_VI("id", FRCMD_INIT, FrResultDlg::OnIdInit)
ON_FRESH_VI("id", FRCMD_OWNERDRAW, FrResultDlg::OnIdOwnerDraw)
ON_FRESH_VI("match", FRCMD_INIT, FrResultDlg::OnMatchInit)
ON_FRESH_VI("smatch", FRCMD_INIT, FrResultDlg::OnMatchStaticInit)
ON_FRESH_VI("score", FRCMD_INIT, FrResultDlg::OnScoreInit)
ON_FRESH_VI("sscore", FRCMD_INIT, FrResultDlg::OnScoreStaticInit)
ON_FRESH_VI("pang", FRCMD_INIT, FrResultDlg::OnPangInit)
ON_FRESH_VI("bonus", FRCMD_INIT, FrResultDlg::OnBonusInit)
ON_FRESH_VI("sbonus", FRCMD_INIT, FrResultDlg::OnBonusStaticInit)
ON_FRESH_VI("getitem", FRCMD_INIT, FrResultDlg::OnGetItemInit)
ON_FRESH_VV("getitem", FRCMD_OWNERDRAW, FrResultDlg::OnGetItemOwnerDraw)
ON_FRESH_VI("level", FRCMD_INIT, FrResultDlg::OnLevelInit)
ON_FRESH_VI("exp", FRCMD_INIT, FrResultDlg::OnExpInit)
ON_FRESH_VI("exp_gb", FRCMD_INIT, FrResultDlg::OnExpGaugeBarInit)
ON_FRESH_VI("levelup", FRCMD_INIT, FrResultDlg::OnLevelUpInit)

END_FRESH_MSGMAP()

FrResultDlg::FrResultDlg()
{
	m_expTime = 0;
	m_pStage = NULL;
	m_pLevel = NULL;
	m_pExp = NULL;
	m_pExpGaugeBar = NULL;
	m_pExhibition = NULL;
	m_pLevelUp = NULL;
	m_pPartTidList = NULL;
	memset(&m_winner, 0, sizeof(m_winner));
	FindWinner();

	static char* s_resultFontFiles[] = { "[bigfont.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_resultFontFiles;
	info.fontw = 32;
	info.fonth = 32;
	info.numPages = 1;
	info.texw = 256;
	info.texh = 128;
	info.pCharSet = "1234567890ym%-./x ";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);

	if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
		m_pHalloweenOverlay =
			g_resrcmng->GetOverlay(MakeStr("%s.tga", "z_Halloween_Male"), 0);
}

FrResultDlg::~FrResultDlg()
{
	if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
	{
		if (g_resrcmng && m_pHalloweenOverlay)
		{
			g_resrcmng->Release(m_pHalloweenOverlay);
			m_pHalloweenOverlay = NULL;
		}
	}
	if (m_pPartTidList)
	{
		delete m_pPartTidList;
		m_pPartTidList = NULL;
	}
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = NULL;
	}
}

void FrResultDlg::ResetVars()
{
	m_pStage = NULL;
	m_pLevel = NULL;
	m_pExp = NULL;
	m_pExpGaugeBar = NULL;
	m_pExhibition = NULL;
	m_pLevelUp = NULL;
	m_pPartTidList = NULL;
	memset(&m_winner, 0, sizeof(m_winner));
}

void FrResultDlg::FindWinner()
{
	memset(Doc()->m_teamResult, 0, sizeof(Doc()->m_teamResult));
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
	{
		if (it->rank == 1)
		{
			m_winner = *it;
			if (Doc()->m_roomInfo.gameType == 4)
				return;
			if (Doc()->m_roomInfo.gameType == 10)
				return;
		}
		Doc()->m_teamResult[it->team].score += it->rank;
		Doc()->m_teamResult[it->team].pang += it->totalPang;
	}
}

bool FrResultDlg::OnInit()
{
	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		m_startLevel = m_level = Doc()->m_userInfo[0].stat.Level;
		m_exp = Doc()->m_userInfo[0].stat.dwExp;
	}
	else
	{
		m_startLevel = m_level = Doc()->m_myInfo.stat.Level;
		m_exp = Doc()->m_myInfo.stat.dwExp;
	}
	m_addExp = Doc()->m_holeStatistics[0].dwExp;
	int nextExp = Doc()->m_levelTable[m_level].exp;
	m_bExpDone = m_addExp == 0;
	m_exp *= 10;
	m_addExp *= 10;
	nextExp *= 10;
	if (m_pExp)
	{
		char buf[32];
		sprintf(buf, "%d / %d", m_exp / 10, nextExp / 10);
		m_pExp->SetCaption(buf);
	}
	if (m_pLevel)
		m_pLevel->SetBgImg(MakeStr("level_%03d", m_level + 1));
	if (m_pExpGaugeBar)
	{
		m_pExpGaugeBar->SetBorder(1);
		m_pExpGaugeBar->SetRange(0, nextExp, 0);
		m_pExpGaugeBar->SetPos(m_exp);
	}
	// 983 (00ae9320): formato 00d4d6fc (1o lugar = %s), DAT_00d65136 = "" so preenche o %49s
	SetDesc(MakeStr(K2L_Compatibility("%49s1\265\356\300\272 %s\264\324\300\314 \302\367\301\366\307\317\274\314\275\300\264\317\264\331."), "", m_winner.nickname));
	// 983 (00ae9320): no Grand Prix (tipos 0x14/0x15) a estatística não vai ao servidor; Log30sScore fica fora do pacote
	if (!Doc()->m_bSentHoleStat)
	{
		Doc()->m_bSentHoleStat = true;
		if (!(bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		{
			switch (Doc()->m_roomInfo.realGameType)
			{
			case GAME_TYPE_GRANDPRIX983:
			case GAME_TYPE_GRANDPRIX983_2:
				break;
			default:
			{
				WSendPacket packet(6);
				packet.EncodeBuffer(&Doc()->m_holeStatistics[0],
					sizeof(sPangYaUserStatistics));
				packet.Send(TO_GAME);
			}
			}
			Doc()->Log30sScore();
		}
	}
	return true;
}

void FrResultDlg::OnProc(const float delta)
{
	m_expTime += delta;
	if (m_pExhibition)
	{
		const WRect& rect = m_pStage->GetRect();
		if (m_stageRect.x != rect.x || m_stageRect.y != rect.y)
		{
			m_pExhibition->SetCenter(rect.x + rect.w * 0.5f,
				rect.y + rect.h * 0.5f);
			m_stageRect = rect;
		}
		m_pExhibition->Process(delta, true);
	}
	if (!m_pExpGaugeBar || m_bExpDone)
	{
		if (m_bExpDone && m_pLevelUp && m_startLevel < m_level)
			m_pLevelUp->SetVisible(true);
		return;
	}
	if (m_expTime < 1.0f)
		return;
	float over = m_expTime - 1.0f;
	if (over < 0.01f)
		return;
	int step = float2int(over * 100.0f);
	int level = m_level;
	m_expTime -= step / 100.0f;
	int nextExp = Doc()->m_levelTable[level].exp * 10;
	if (step > abs(m_addExp))
	{
		m_bExpDone = true;
		m_exp += ((m_addExp > 0) * 2 - 1) * abs(m_addExp);
		m_addExp = 0;
	}
	else
	{
		int add = ((m_addExp > 0) * 2 - 1) * step;
		m_exp += add;
		m_addExp -= add;
	}
	if (m_exp < 0 && !m_bExpDone)
	{
		m_bExpDone = true;
		m_exp = 0;
	}
	if (m_exp >= nextExp)
	{
		m_exp -= nextExp;
		m_level = level + 1;
		nextExp = Doc()->m_levelTable[m_level].exp * 10;
		m_pExpGaugeBar->SetRange(0, nextExp, 0);
		m_pExpGaugeBar->SetPos(m_exp);
		if (m_pLevel)
		{
			char buf[32];
			sprintf(buf, "level_%03d", m_level + 1);
			m_pLevel->SetBgImg(buf);
		}
	}
	else
	{
		m_pExpGaugeBar->SetPos(m_exp);
	}
	if (m_pExp)
	{
		char buf[32];
		sprintf(buf, "%d / %d", m_exp / 10, nextExp / 10);
		m_pExp->SetCaption(buf);
	}
}

void FrResultDlg::OnStageInit(int param)
{
	m_pStage = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pStage)
	{
		char name[64] = "";
		sprintf(name, "30in_back_%s",
			(Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data
									   : NULL));
		m_pStage->SetBgImg(name);
	}
	ShowPet();
}

void FrResultDlg::OnStageOwnerDraw()
{
	char name[64] = "";
	sprintf(name, "30in_back_%s",
		(Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL));
	m_pStage->SetBgImg(name);
	if (m_pExhibition)
		m_pExhibition->Display(0.0f, 0.0f);
	const WRect& rect = m_pStage->GetRect();
	WPoint pos(rect.x + 333.0f, rect.y + 34.0f);
	char* text;
	if (Doc()->m_roomInfo.gameType == 10 &&
		(Doc()->m_rivalList[MyIndex()].approachTime == -1 ||
			Doc()->m_rivalList[MyIndex()].approachResultDistance == -1))
		text = MakeStr("-");
	else
		text = MakeStr("%d", Doc()->m_rivalList[MyIndex()].rank);
	if (m_pTitleFont)
		m_pTitleFont->Print(g_view, pos.x, pos.y, text, 0, 0xffffffff, 0);
}

void FrResultDlg::OnTrophyInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (!pArea)
		return;
	if (Doc()->m_eventType)
	{
		IFF_STRUCT::sMatch* pMatch =
			ItemManager()->FindMatch(Doc()->m_roomType);
		if (pMatch && Doc()->m_eventType >= 1 && Doc()->m_eventType <= 3)
			pArea->SetBgImg(pMatch->Icon[Doc()->m_eventType - 1]);
		else
			pArea->SetVisible(false);
		char* text = MakeStr("%d", Doc()->m_rivalList[MyIndex()].rank);
		WPoint pos;
		pos.x = m_pTitleFont->GetTextWidthOld983(g_view, text) + GetRect().x + 340.0f;
		if (Doc()->m_golfGame.gameType == 5)
			pos.y = GetRect().y + 100.0f;
		else
			pos.y = GetRect().y + 40.0f;
		pArea->MoveWindow(pos);
	}
	else
		pArea->SetVisible(false);
}

void FrResultDlg::OnMatchStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_roomInfo.gameType == 10)
		pStatic->SetCaption(K2L_Compatibility("\xb3\xb2\xc0\xba \xb0\xc5\xb8\xae :")); // 983 00ae8230 (B1278: K2L, nao vazio)
}

void FrResultDlg::OnMatchInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	const char* text;
	if (Doc()->m_roomInfo.gameType == 10)
	{
		unsigned long distance;
		if (Doc()->m_rivalList[MyIndex()].approachTime != -1 &&
			(distance = Doc()->m_rivalList[MyIndex()].approachResultDistance) !=
				-1)
			text = MakeStr("%d.%d y", distance / 10, distance % 10);
		else
			text = "Out";
	}
	else
	{
		IFF_STRUCT::sMatch* pMatch =
			ItemManager()->FindMatch(Doc()->m_roomType);
		if (!pMatch)
			return;
		char name[80] = "";
		if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
			strcpy(name, K2L_Compatibility("\xbc\xc5\xc7\xc3\xc4\xda\xbd\xba")); // 983 00aecce0 (FUN_00b4ec60 = K2L_Compatibility)
		else
			strcpy(name, pMatch->Name);
		text = name;
	}
	pStatic->SetCaption(text);
}

void FrResultDlg::OnIdInit(int param)
{
	m_pId = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrResultDlg::OnIdOwnerDraw(int)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		float offset = 0.0f;
		sRivalData& rival = Doc()->m_rivalList[MyIndex()];
		if (rival.guildUID)
		{
			const Bitmap* emblem =
				NetResourceManager::Instance()->GetEmblem983(
					rival.guildUID, rival.guildEmblemVer983);
			if (emblem)
			{
				device->DrawTexture(emblem,
					WRect(m_pId->GetRect().x + offset, m_pId->GetRect().y + offset - 6.0,
						(float)emblem->Width(), (float)emblem->Height()),
					0xffffffff, 0);
				offset = offset + 27.0;
			}
		}
		if (!CProjectG::Instance()->HidePrivacy())
		{
			device->SetTextColor(0xff000000, 0xffffffff);
			device->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pId->GetRect().x + offset, m_pId->GetRect().y + 0.0), 0,
				rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrResultDlg::OnScoreStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_roomInfo.gameType == 10)
		pStatic->SetCaption(K2L_Compatibility("\xb3\xb2\xc0\xba \xbd\xc3\xb0\xa3 :")); // 983 00ae82b0 (B1278: K2L, nao vazio)
}

void FrResultDlg::OnScoreInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_roomInfo.gameType == 10)
	{
		int time = Doc()->m_rivalList[MyIndex()].approachTime;
		if (time == -1)
			pStatic->SetCaption("-");
		else
			pStatic->SetCaption(MakeStr(K2L_Compatibility("%d.%03d \xc3\xca"), time / 1000, time % 1000)); // 983 00aecf36
	}
	else
	{
		pStatic->SetCaption(
			MakeStr(Doc()->m_rivalList[MyIndex()].totalScore > 0 ? "+%d" : "%d",
				Doc()->m_rivalList[MyIndex()].totalScore));
	}
}

void FrResultDlg::OnPangInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_roomInfo.gameType == 10)
	{
		pStatic->SetCaption("0");
		for (int i = 6; i < 12; ++i)
		{
			if (Doc()->m_awardItem[i].uid == MyGuid(true))
			{
				pStatic->SetCaption(MakeStr("%d", Doc()->m_awardItem[i].pang));
				return;
			}
		}
	}
	else
		pStatic->SetCaption(
			MakeStr("%I64d", Doc()->m_rivalList[MyIndex()].totalPang));
}

void FrResultDlg::OnBonusStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_roomInfo.gameType == 10)
		pStatic->SetVisible(false);
}

void FrResultDlg::OnBonusInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_roomInfo.gameType == 10)
		pStatic->SetVisible(false);
	// 983 (00636040): o resultado e multiplicado pela taxa de bonus do jogador (item auxiliar / vetor +0x114b8)
	int bonus = (int)((float)(Doc()->m_rivalList[MyIndex()].totalPang +
				Doc()->m_rivalList[MyIndex()].totalBonusPang) *
			Doc()->m_pangRate * (float)(unsigned int)Doc()->m_expRate /
			100.0 * ((CSharedDocJp*)Doc())->GetBonusRate983(MyIndex(), 3) -
		(float)Doc()->m_rivalList[MyIndex()].totalPang);
	pStatic->SetCaption(MakeStr("%I64d", (__int64)bonus));
}

void FrResultDlg::OnGetItemInit(int param)
{
	m_pGetItem = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrResultDlg::OnGetItemOwnerDraw()
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	float offset = 0.0f;
	for (int i = 0; i < 12; i++)
	{
		if (Doc()->m_awardItem[i].uid == MyGuid(true) &&
			Doc()->m_awardItem[i].pang)
		{
			IFF_ITEM_COMMON* pItem =
				ItemManager()->FindCommonItem(Doc()->m_awardItem[i].pang);
			const Bitmap* bitmap = NULL;
			switch ((unsigned long)Doc()->m_awardItem[i].pang >> 26)
			{
			case 2:
				bitmap =
					g_pFresh->GetManager()->GetBitmap("FASHION", pItem->Icon);
				break;
			case 5:
				bitmap =
					g_pFresh->GetManager()->GetBitmap("ITEMS", pItem->Icon);
				break;
			case 6:
				bitmap =
					g_pFresh->GetManager()->GetBitmap("ITEMS", pItem->Icon);
				break;
			default:
				continue;
			}
			if (bitmap)
			{
				device->DrawTexture(bitmap,
					WRect(offset + m_pGetItem->GetRect().x,
						m_pGetItem->GetRect().y, (float)bitmap->Width(),
						(float)bitmap->Height()),
					0xffffffff, 0);
				offset += 50.0f;
			}
		}
	}
	if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/) && Doc()->m_bWearHalloweenItem)
	{
		if (m_pHalloweenOverlay)
		{
			m_pHalloweenOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(offset + m_pId->GetRect().x + 150.0f,
					m_pId->GetRect().y + 5.0f,
					(float)m_pHalloweenOverlay->GetWidth(),
					(float)m_pHalloweenOverlay->GetHeight()),
				0, 0xffffffff, 0, 0);
		}
		device->SetTextStyle(1);
		device->SetTextColor(0xff000000, 0xffffffff);
		device->SetScale(2.0f);
		g_pFresh->GetManager()->PrintText(
			WPoint(m_pId->GetRect().x + 141.0f, m_pId->GetRect().y + 41.0f), 0,
			"+100", -1.0f, 0xffffffff);
		device->SetTextColor(0xffffffff, 0xffffffff);
		g_pFresh->GetManager()->PrintText(
			WPoint(m_pId->GetRect().x + 140.0f, m_pId->GetRect().y + 40.0f), 0,
			"+100", -1.0f, 0xffffffff);
		device->SetScale(1.0f);
	}
	for (std::vector<unsigned long>::iterator it =
			 Doc()->m_acquireItemList[0].begin();
		it != Doc()->m_acquireItemList[0].end(); ++it)
	{
		const Bitmap* bitmap = NULL;
		if (*it >> 26 == 6)
		{
			IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(*it);
			if (!pItem)
				continue;
			bitmap = g_pFresh->GetManager()->GetBitmap("ITEMS", pItem->c.Icon);
		}
		else
			continue; // 983: sem o ramo QUESTDROP (so tipo 6)
		if (bitmap)
		{
			device->DrawTexture(bitmap,
				WRect(offset + m_pGetItem->GetRect().x - 11.0f,
					m_pGetItem->GetRect().y - 5.0f, (float)bitmap->Width(),
					(float)bitmap->Height()),
				0xffffffff, 0);
			offset += 50.0f;
		}
	}
}

void FrResultDlg::OnLevelInit(int param)
{
	m_pLevel = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrResultDlg::OnExpInit(int param)
{
	m_pExp = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrResultDlg::OnExpGaugeBarInit(int param)
{
	m_pExpGaugeBar = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
}

void FrResultDlg::OnLevelUpInit(int param)
{
	m_pLevelUp = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pLevelUp)
		m_pLevelUp->SetVisible(false);
}

void FrResultDlg::ShowPet()
{
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
	if (m_pPartTidList)
	{
		delete m_pPartTidList;
		m_pPartTidList = NULL;
	}
	if (!m_pStage)
		return;
	WRect rect = m_pStage->GetRect();
	m_pExhibition = new CExhibitionByHardwareTnL(rect.w * 0.5f + rect.x,
		rect.h * 0.5f + rect.y, 0.33f, false);
	m_pExhibition->SetDistance(20.0f, true, false);
	m_pExhibition->SetArea(rect.w, rect.h);
	unsigned char index = MyIndex();
	unsigned char motion;
	if (index < Doc()->m_rivalList.size())
	{
		if (Doc()->m_golfGame.gameType == 5)
		{
			if ((unsigned int)Doc()->m_teamResult[0].score <
					(unsigned int)Doc()->m_teamResult[1].score ||
				((unsigned int)Doc()->m_teamResult[0].score ==
						(unsigned int)Doc()->m_teamResult[1].score &&
					Doc()->m_teamResult[0].pang > Doc()->m_teamResult[1].pang))
				motion = Doc()->m_rivalList[MyIndex()].team != 0 ? 3 : 1;
			else if ((unsigned int)Doc()->m_teamResult[0].score >
					(unsigned int)Doc()->m_teamResult[1].score ||
				((unsigned int)Doc()->m_teamResult[0].score ==
						(unsigned int)Doc()->m_teamResult[1].score &&
					Doc()->m_teamResult[0].pang < Doc()->m_teamResult[1].pang))
				motion = Doc()->m_rivalList[MyIndex()].team != 1 ? 3 : 1;
			else
				motion = 2;
		}
		else if (Doc()->m_golfGame.gameType == 6)
		{
			if (Doc()->m_eventFlag == 0)
				motion = Doc()->m_rivalList[MyIndex()].team != 0 ? 3 : 1;
			else if (Doc()->m_eventFlag == 1)
				motion = Doc()->m_rivalList[MyIndex()].team != 1 ? 3 : 1;
			else
				motion = 3;
		}
		else
		{
			unsigned char rank = Doc()->m_rivalList[index].rank;
			if (rank == 1)
				motion = 1;
			else
				motion = rank > Doc()->m_rivalList.size() * 6 / 10 ? 3 : 2;
		}
	}
	else
		motion = 3;
	if ((Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
	{
		m_pPartTidList = new CPartTidList;
		if (!m_pPartTidList->SetTids(&Doc()->m_userInfo[0].charInfo, 0xff) ||
			!m_pPartTidList->IsComboValid())
			m_pPartTidList->SetDefaultTids();
		m_pExhibition->SetModel(*m_pPartTidList,
			// 983 00aed260: formato DAT_00d4da50 = "%d" + sufixo cp949 (motion de colocacao)
			Doc()->m_userInfo[0].charInfo.tidAuxParts, MakeStr("%d\265\356\270\360\274\307", motion),
			3.0f, 0);
	}
	else
	{
		m_pExhibition->SetModel(Doc()->GetMyPartTidList(),
			Doc()->GetMyAuxPartTidList(), MakeStr("%d\265\356\270\360\274\307", motion), 3.0f, 0);
	}
}

IMPLEMENT_OBJECT(FrResultTeamDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrResultTeamDlg, FrForm)

ON_FRESH_VI("stage", FRCMD_INIT, FrResultTeamDlg::OnStageInit)
ON_FRESH_VV("stage", FRCMD_OWNERDRAW, FrResultTeamDlg::OnStageOwnerDraw)
ON_FRESH_VI("trophy", FRCMD_INIT, FrResultTeamDlg::OnTrophyInit)
ON_FRESH_VI("match", FRCMD_INIT, FrResultTeamDlg::OnMatchInit)
ON_FRESH_VI("id", FRCMD_INIT, FrResultTeamDlg::OnIdInit)
ON_FRESH_VI("id", FRCMD_OWNERDRAW, FrResultTeamDlg::OnIdOwnerDraw)
ON_FRESH_VI("score", FRCMD_INIT, FrResultTeamDlg::OnScoreInit)
ON_FRESH_VI("pang", FRCMD_INIT, FrResultTeamDlg::OnPangInit)
ON_FRESH_VI("bonus", FRCMD_INIT, FrResultTeamDlg::OnBonusInit)
ON_FRESH_VI("getitem", FRCMD_INIT, FrResultTeamDlg::OnGetItemInit)
ON_FRESH_VV("getitem", FRCMD_OWNERDRAW, FrResultTeamDlg::OnGetItemOwnerDraw)
ON_FRESH_VI("level", FRCMD_INIT, FrResultTeamDlg::OnLevelInit)
ON_FRESH_VI("exp", FRCMD_INIT, FrResultTeamDlg::OnExpInit)
ON_FRESH_VI("exp_gb", FRCMD_INIT, FrResultTeamDlg::OnExpGaugeBarInit)
ON_FRESH_VI("levelup", FRCMD_INIT, FrResultTeamDlg::OnLevelUpInit)
ON_FRESH_VI("red", FRCMD_INIT, FrResultTeamDlg::OnRedInit)
ON_FRESH_VV("red", FRCMD_OWNERDRAW, FrResultTeamDlg::OnRedOwnerDraw)
ON_FRESH_VI("blue", FRCMD_INIT, FrResultTeamDlg::OnBlueInit)
ON_FRESH_VV("blue", FRCMD_OWNERDRAW, FrResultTeamDlg::OnBlueOwnerDraw)
ON_FRESH_VI("red_score", FRCMD_INIT, FrResultTeamDlg::OnRedScoreInit)
ON_FRESH_VI("blue_score", FRCMD_INIT, FrResultTeamDlg::OnBlueScoreInit)
ON_FRESH_VI("red_pang", FRCMD_INIT, FrResultTeamDlg::OnRedPangInit)
ON_FRESH_VI("blue_pang", FRCMD_INIT, FrResultTeamDlg::OnBluePangInit)
ON_FRESH_VI("result", FRCMD_INIT, FrResultTeamDlg::OnResultInit)
ON_FRESH_VI("rank_static", FRCMD_INIT, FrResultTeamDlg::OnRankStaticInit)
ON_FRESH_VI("result_static", FRCMD_INIT, FrResultTeamDlg::OnResultStaticInit)
ON_FRESH_VI("win_lose", FRCMD_INIT, FrResultTeamDlg::OnWinLoseInit)
ON_FRESH_VI("matchtype", FRCMD_INIT, FrResultTeamDlg::OnMatchTypeInit)
ON_FRESH_VI("bonus_static", FRCMD_INIT, FrResultTeamDlg::OnBonusStaticInit)

END_FRESH_MSGMAP()

void FrResultTeamDlg::OnStageInit(int param)
{
	FrResultDlg::OnStageInit(param);
	m_pGuildTrophy = NULL;
	if (Doc()->m_golfGame.gameType == 6 &&
		Doc()->m_eventFlag ==
			Doc()
				->m_rivalList[Doc()->GetIndex(Doc()->m_teamPlayerGuid[0])]
				.team &&
		Doc()->m_eventFlag != 2)
	{
		m_pGuildTrophy = g_pFresh->GetManager()->GetBitmap("TROPHY", "guild");
	}
}

bool FrResultTeamDlg::OnInit()
{
	FrResultDlg::OnInit();
	if (Doc()->m_golfGame.gameType == 6)
	{
		memset(&m_guildInfo, 0, sizeof(m_guildInfo));
		// 983 (00af0290): formatos 00d4dbe0 (empate) / 00d4dbc0 (guilda %s venceu); DAT_00d65136 = "" so preenche o %49s
		if (Doc()->m_eventFlag == 2)
			SetDesc(MakeStr(K2L_Compatibility("%49s \264\331\275\303 \307\321\271\370 \300\332\277\365\300\273 \260\241\270\256\274\274\277\344^^"), ""));
		else
		{
			sRivalData* r0 =
				&Doc()->m_rivalList[Doc()->GetIndex(Doc()->m_teamPlayerGuid[0])];
			sRivalData* r1 =
				&Doc()->m_rivalList[Doc()->GetIndex(Doc()->m_teamPlayerGuid[1])];
			sRivalData* w = NULL;
			if (Doc()->m_eventFlag == r0->team)
				w = r0;
			else if (Doc()->m_eventFlag == r1->team)
				w = r1;
			if (w)
				SetDesc(MakeStr(K2L_Compatibility("%49s %s\261\346\265\345\260\241 \275\302\270\256\307\317\277\264\275\300\264\317\264\331."), "", w->guildName));
		}
		memcpy(&m_guildInfo, &Doc()->m_roomInfo.GuildInfo, sizeof(m_guildInfo));
	}
	return true;
}

void FrResultTeamDlg::OnStageOwnerDraw()
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	WPoint from = GetRect().TopLeft();
	WPoint to(GetRect().w + from.x - 10.0f, from.y + 91.0f);
	from.y = to.y;
	g_view->DrawLine2D(from, to, 0xff000000, 0xff000000, 0);
	if (Doc()->m_golfGame.gameType == 6)
	{
		if (m_pExhibition)
			m_pExhibition->Display(0.0f, 0.0f);
		const WRect& rect = m_pStage->GetRect();
		if (m_pGuildTrophy)
		{
			device->DrawTexture(m_pGuildTrophy,
				WRect(rect.x + 343.0f, rect.y + 10.0f,
					(float)m_pGuildTrophy->Width(),
					(float)m_pGuildTrophy->Height()),
				0xffffffff, 0);
		}
		const Bitmap* emblem;
		if (Doc()->m_eventFlag == 0)
		{
			if (GUILDMATCHUP(0)->team != 0)
				return;
			emblem = m_pRedEmblem;
		}
		else if (Doc()->m_eventFlag == 1)
		{
			if (GUILDMATCHUP(0)->team != 1)
				return;
			emblem = m_pBlueEmblem;
		}
		else
			return;
		if (emblem)
		{
			device->DrawTexture(emblem,
				WRect(rect.x + 364.0f, rect.y + 22.0f, (float)emblem->Width(),
					(float)emblem->Height()),
				0xffffffff, 0);
		}
	}
	else
		FrResultDlg::OnStageOwnerDraw();
}

void FrResultTeamDlg::OnTrophyInit(int param)
{
	FrResultDlg::OnTrophyInit(param);
}

void FrResultTeamDlg::OnMatchTypeInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	// 983 00ae83a0: guilda = texto proprio (gameType 6)
	if (pStatic)
		pStatic->SetCaption(Doc()->m_golfGame.gameType == 6
				? K2L_Compatibility("\xb1\xe6\xb5\xe5 \xb1\xe2\xbf\xa9\xb5\xb5 :")
				: K2L_Compatibility("\xb4\xeb   \xc8\xb8 :"));
}

void FrResultTeamDlg::OnMatchInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (Doc()->m_golfGame.gameType == 6)
	{
		if (pStatic)
			pStatic->SetCaption(MakeStr("%d \301\241", Doc()->m_guildPoint));
	}
	else
	{
		IFF_STRUCT::sMatch* pMatch =
			ItemManager()->FindMatch(Doc()->m_roomType);
		if (pStatic && pMatch)
			// 983 00ae8430: sufixo pelo K2L_Compatibility (B1278)
			pStatic->SetCaption(MakeStr("%s %s", pMatch->Name, K2L_Compatibility("\xc6\xc0\xc0\xfc")));
	}
}

void FrResultTeamDlg::OnIdInit(int param)
{
	m_pTeamId = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrResultTeamDlg::OnIdOwnerDraw(int)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		float offset = 0.0f;
		sRivalData& rival = Doc()->m_rivalList[MyIndex()];
		if (rival.guildUID)
		{
			const Bitmap* emblem =
				NetResourceManager::Instance()->GetEmblem983(
					rival.guildUID, rival.guildEmblemVer983);
			if (emblem)
			{
				device->DrawTexture(emblem,
					WRect(m_pTeamId->GetRect().x + offset, m_pTeamId->GetRect().y + offset - 6.0,
						(float)emblem->Width(), (float)emblem->Height()),
					0xffffffff, 0);
				offset = offset + 27.0;
			}
		}
		if (!CProjectG::Instance()->HidePrivacy())
		{
			device->SetTextColor(0xff000000, 0xffffffff);
			device->SetTextStyle(0);
			g_pFresh->GetManager()->PrintText(
				WPoint(m_pTeamId->GetRect().x + offset, m_pTeamId->GetRect().y + 0.0),
				0, rival.nickname, -1.0f, 0xffffffff);
		}
	}
}

void FrResultTeamDlg::OnScoreInit(int param)
{
	FrResultDlg::OnScoreInit(param);
}

void FrResultTeamDlg::OnPangInit(int param)
{
	FrResultDlg::OnPangInit(param);
}

void FrResultTeamDlg::OnBonusStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	// 983 00ae8510: guilda = texto proprio (gameType 6)
	if (pStatic)
		pStatic->SetCaption(Doc()->m_golfGame.gameType == 6
				? K2L_Compatibility("\xb1\xe6\xb5\xe5 \xc6\xce :")
				: K2L_Compatibility("\xba\xb8\xb3\xca\xbd\xba :"));
}

void FrResultTeamDlg::OnBonusInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_golfGame.gameType == 6)
	{
		pStatic->SetCaption(MakeStr("%d \306\316", Doc()->m_guildBonusPang));
		return;
	}
	// 983 (00636040): o resultado e multiplicado pela taxa de bonus do jogador (item auxiliar / vetor +0x114b8)
	int bonus = (int)((float)(Doc()->m_rivalList[MyIndex()].totalPang +
				Doc()->m_rivalList[MyIndex()].totalBonusPang) *
			Doc()->m_pangRate * (float)(unsigned int)Doc()->m_expRate /
			100.0 * ((CSharedDocJp*)Doc())->GetBonusRate983(MyIndex(), 3) -
		(float)Doc()->m_rivalList[MyIndex()].totalPang);
	if (Doc()->m_eventFlag == 2)
		pStatic->SetCaption(MakeStr("%I64d", (__int64)bonus));
	else if (Doc()->m_eventFlag == Doc()->m_rivalList[MyIndex()].team)
		pStatic->SetCaption(MakeStr("%I64d+100", (__int64)bonus));
	else
		pStatic->SetCaption(MakeStr("%I64d-50", (__int64)bonus));
}

void FrResultTeamDlg::OnGetItemInit(int param)
{
	FrResultDlg::OnGetItemInit(param);
}

void FrResultTeamDlg::OnGetItemOwnerDraw()
{
	FrResultDlg::OnGetItemOwnerDraw();
}

void FrResultTeamDlg::OnLevelInit(int param)
{
	FrResultDlg::OnLevelInit(param);
}

void FrResultTeamDlg::OnExpInit(int param)
{
	FrResultDlg::OnExpInit(param);
}

void FrResultTeamDlg::OnExpGaugeBarInit(int param)
{
	FrResultDlg::OnExpGaugeBarInit(param);
}

void FrResultTeamDlg::OnLevelUpInit(int param)
{
	FrResultDlg::OnLevelUpInit(param);
}

void FrResultTeamDlg::OnRankStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_golfGame.gameType == 6)
		pStatic->SetVisible(false);
}

void FrResultTeamDlg::OnResultStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_golfGame.gameType != 6)
		pStatic->SetVisible(false);
}

void FrResultTeamDlg::OnRedInit(int param)
{
	m_pRed = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pRed)
	{
		if (Doc()->m_golfGame.gameType == 6)
			m_pRedEmblem = NetResourceManager::Instance()->GetEmblem983(
				Doc()->m_roomInfo.GuildInfo.nGuildID[0], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[0]);
		else if (Doc()->m_eventFlag != 1) // 983
			m_pRed->SetBgImg("30in_team_redteam");
		else
			m_pRed->SetBgImg("30in_team_redteam_dark");
	}
}

void FrResultTeamDlg::OnRedOwnerDraw()
{
	if (Doc()->m_golfGame.gameType == 6)
	{
		WPoint pos = m_pRed->GetRect().TopLeft();
		if (m_pRedEmblem)
		{
			g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pRedEmblem,
				WRect(pos.x + 10.0, pos.y + 1.0, (float)m_pRedEmblem->Width(),
					(float)m_pRedEmblem->Height()),
				0xffffffff, 0);
		}
		g_pFresh->GetManager()->PrintText(WPoint(pos.x, pos.y + 30.0), 0,
			m_guildInfo.szName[0], -1.0f, 0xffffffff);
	}
}

void FrResultTeamDlg::OnBlueInit(int param)
{
	m_pBlue = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBlue)
	{
		if (Doc()->m_golfGame.gameType == 6)
			m_pBlueEmblem = NetResourceManager::Instance()->GetEmblem983(
				Doc()->m_roomInfo.GuildInfo.nGuildID[1], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[1]);
		else if (Doc()->m_eventFlag != 0) // 983
			m_pBlue->SetBgImg("30in_team_blueteam");
		else
			m_pBlue->SetBgImg("30in_team_blueteam_dark");
	}
}

void FrResultTeamDlg::OnBlueOwnerDraw()
{
	if (Doc()->m_golfGame.gameType == 6)
	{
		WPoint pos = m_pBlue->GetRect().TopLeft();
		if (m_pBlueEmblem)
		{
			g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pBlueEmblem,
				WRect(pos.x + 10.0, pos.y + 1.0,
					(float)m_pBlueEmblem->Width(),
					(float)m_pBlueEmblem->Height()),
				0xffffffff, 0);
		}
		g_pFresh->GetManager()->PrintText(WPoint(pos.x, pos.y + 30.0), 0,
			m_guildInfo.szName[1], -1.0f, 0xffffffff);
	}
}

void FrResultTeamDlg::OnRedScoreInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_golfGame.gameType == 5)
		pStatic->SetCaption(MakeStr("%d", Doc()->m_teamResult[0].score));
	else
		pStatic->SetCaption(MakeStr("%d", Doc()->m_guildScore[0]));
}

void FrResultTeamDlg::OnBlueScoreInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	if (Doc()->m_golfGame.gameType == 5)
		pStatic->SetCaption(MakeStr("%d", Doc()->m_teamResult[1].score));
	else
		pStatic->SetCaption(MakeStr("%d", Doc()->m_guildScore[1]));
}

void FrResultTeamDlg::OnRedPangInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	pStatic->SetCaption(MakeStr("%I64dp", Doc()->m_teamResult[0].pang));
}

void FrResultTeamDlg::OnBluePangInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
		return;
	pStatic->SetCaption(MakeStr("%I64dp", Doc()->m_teamResult[1].pang));
}

void FrResultTeamDlg::OnResultInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (!pArea)
		return;
	if (Doc()->m_golfGame.gameType == 6)
		return;
	switch (Doc()->m_eventFlag)
	{
	case 0:
		if (Doc()->m_rivalList[MyIndex()].team == 0)
			pArea->SetBgImg("30in_team_redteam_win");
		else
			pArea->SetBgImg("30in_team_blueteam_lose");
		break;
	case 1:
		if (Doc()->m_rivalList[MyIndex()].team == 0)
			pArea->SetBgImg("30in_team_redteam_lose");
		else
			pArea->SetBgImg("30in_team_blueteam_win");
		break;
	case 2:
		if (Doc()->m_rivalList[MyIndex()].team == 0)
			pArea->SetBgImg("30in_team_redteam_draw");
		else
			pArea->SetBgImg("30in_team_blueteam_draw");
		break;
	}
}

void FrResultTeamDlg::OnWinLoseInit(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (!pArea)
		return;
	if (Doc()->m_golfGame.gameType != 6)
		return;
	char name[32];
	switch (Doc()->m_eventFlag)
	{
	case 0:
		if (Doc()->m_rivalList[MyIndex()].team == 0)
			strcpy(name, "win2");
		else
			strcpy(name, "lose2");
		break;
	case 1:
		if (Doc()->m_rivalList[MyIndex()].team == 0)
			strcpy(name, "lose2");
		else
			strcpy(name, "win2");
		break;
	case 2:
		strcpy(name, "draw2");
		break;
	}
	pArea->SetBgImg(g_pFresh->GetManager()->GetBitmap("SCORECARD", name));
}

bool UniteRankCompare(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;
	if (pA->rank < pB->rank)
		return true;
	else if (pA->rank > pB->rank)
		return false;
	if (pA->quitOrder > pB->quitOrder)
		return true;
	else if (pA->quitOrder < pB->quitOrder)
		return false;
	if (pA->totalScore < pB->totalScore)
		return true;
	else if (pA->totalScore > pB->totalScore)
		return false;
	if (pA->totalPang > pB->totalPang)
		return true;
	else if (pA->totalPang < pB->totalPang)
		return false;
	return pA->order < pB->order;
}

bool ReportRankCompare(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;
	if (pA->rank < pB->rank)
		return true;
	else if (pA->rank > pB->rank)
		return false;
	if (pA->quitOrder > pB->quitOrder)
		return true;
	else if (pA->quitOrder < pB->quitOrder)
		return false;
	if (pA->totalScore < pB->totalScore)
		return true;
	else if (pA->totalScore > pB->totalScore)
		return false;
	if (pA->totalPang > pB->totalPang)
		return true;
	else if (pA->totalPang < pB->totalPang)
		return false;
	return pA->order < pB->order;
}

bool UniteGuildIndexCompare(const void* a, const void* b)
{
	const sRivalData* pA = (const sRivalData*)a;
	const sRivalData* pB = (const sRivalData*)b;
	if (pA->quitOrder > pB->quitOrder)
		return true;
	else if (pA->quitOrder < pB->quitOrder)
		return false;
	if (pA->guildPoint > pB->guildPoint)
		return true;
	else if (pA->guildPoint < pB->guildPoint)
		return false;
	if (pA->totalPang > pB->totalPang)
		return true;
	else if (pA->totalPang < pB->totalPang)
		return false;
	return pA->order < pB->order;
}

// 983 00ae6ce0: so pela posicao (rank); novo no 983, usado no Grand Prix por guilda
bool UniteRankOnlyCompare(const void* a, const void* b)
{
	return ((const sRivalData*)a)->rank < ((const sRivalData*)b)->rank;
}

IMPLEMENT_OBJECT(FrUniteResultDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrUniteResultDlg, FrForm)

ON_FRESH_VI("cancel", FRCMD_INIT, FrUniteResultDlg::OnCancelButtonInit)
ON_FRESH_VI("awardinfo", FRCMD_INIT, FrUniteResultDlg::OnAwardInfoInit)
ON_FRESH_VV("awardinfo", FRCMD_LBUTTONUP, FrUniteResultDlg::OnAwardInfoClick)
ON_FRESH_VI("myinfoarea", FRCMD_INIT, FrUniteResultDlg::OnMyInfoAreaInit)
ON_FRESH_VI("myinfoarea", FRCMD_OWNERDRAW, FrUniteResultDlg::OnMyInfoAreaDraw)
ON_FRESH_VI("exp_gb", FRCMD_INIT, FrUniteResultDlg::OnExpGaugeBarInit)
ON_FRESH_VI("exp_area", FRCMD_INIT, FrUniteResultDlg::OnExpAreaInit)
ON_FRESH_VI("exp_area", FRCMD_OWNERDRAW, FrUniteResultDlg::OnExpAreaOwnerDraw)
ON_FRESH_VI("resultlist", FRCMD_INIT, FrUniteResultDlg::OnRankListBoxInit)
ON_FRESH_VI("resultlist", FRCMD_OWNERDRAW,
	FrUniteResultDlg::OnRankListOwnerDraw)
ON_FRESH_VV("resultlist", FRCMD_LBUTTONUP, FrUniteResultDlg::OnRankListLBtnUp)
ON_FRESH_VV("resultlist", FRCMD_RBUTTONUP, FrUniteResultDlg::OnRankListRBtnUp)
ON_FRESH_VI("resultwinner", FRCMD_INIT, FrUniteResultDlg::OnDescAreaInit)
ON_FRESH_VI("resultwinner", FRCMD_OWNERDRAW,
	FrUniteResultDlg::OnDescAreaOwnerDraw)
ON_FRESH_VI("awardarea", FRCMD_INIT, FrUniteResultDlg::OnAwardAreaInit)
ON_FRESH_VI("awardarea", FRCMD_OWNERDRAW,
	FrUniteResultDlg::OnAwardAreaOwnerDraw)
ON_FRESH_VI("itemarea", FRCMD_INIT, FrUniteResultDlg::OnItemAreaInit)
ON_FRESH_VI("itemarea", FRCMD_OWNERDRAW, FrUniteResultDlg::OnItemAreaOwnerDraw)
ON_FRESH_VI("teamarea", FRCMD_INIT, FrUniteResultDlg::OnTeamResultInit)
ON_FRESH_VI("teamarea", FRCMD_OWNERDRAW,
	FrUniteResultDlg::OnTeamResultOwnerDraw)
ON_FRESH_VI("tooltip", FRCMD_OWNERDRAW, FrUniteResultDlg::OnTooltipOwnerDraw)
ON_FRESH_VI("nextlevel", FRCMD_INIT, FrUniteResultDlg::OnNextLevelTextInit)

END_FRESH_MSGMAP()

void FrUniteResultDlg::OnCancelButtonInit(int param)
{
	if (!m_ctrl.m_pCancelBtn)
		m_ctrl.m_pCancelBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUniteResultDlg::OnAwardInfoInit(int param)
{
	if (Doc()->m_golfGame.gameType == 0xb) // 983 00ae8c48
		return;
	if (!m_ctrl.m_pAwardInfoBtn)
		m_ctrl.m_pAwardInfoBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrUniteResultDlg::OnAwardInfoClick()
{
	if (Doc()->m_golfGame.gameType != 0xb && !m_ctrl.m_pAwardInfoDlg)
	{
		m_ctrl.m_pAwardInfoDlg = CreateForm<FrAwardInfoDlg>(
			g_pFresh->GetManager(), this, "awardinfomation");
		m_ctrl.m_pAwardInfoDlg->Open(
			(FRESH_PFN_RESULT)&FrUniteResultDlg::OnAwardInfoDlgResult, 0);
	}
}

void FrUniteResultDlg::OnMyInfoAreaInit(int param)
{
	if (!m_ctrl.m_pMyInfoArea)
		m_ctrl.m_pMyInfoArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUniteResultDlg::OnMyInfoAreaDraw(int)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	sRivalData rival;
	if (IsLocalContent((localContentType_t)20) && m_bTikiReport)
		rival = Doc()->m_rivalList[m_tikiReportIndex];
	else
	{
		if (Doc()->m_rivalList.size() <= MyIndex())
			return;
		rival = Doc()->m_rivalList[MyIndex()];
	}
	WPoint pos(m_ctrl.m_pMyInfoArea->GetRect().x,
		m_ctrl.m_pMyInfoArea->GetRect().y);
	float y = 0.0f;
	int space = device->SetSpace(0);
	unsigned char team = 2;
	if (m_resultDlgType)
		team = rival.team;
	if (m_ctrl.m_pRankBack[team])
	{
		WPoint textPos;
		textPos.x = pos.x + 15.0f;
		textPos.y =
			11.0f - (float)(m_ctrl.m_pRankBack[team]->Height() / 2) + pos.y;
		device->DrawTexture(m_ctrl.m_pRankBack[team],
			WRect(textPos.x, textPos.y,
				(float)m_ctrl.m_pRankBack[team]->Width(),
				(float)m_ctrl.m_pRankBack[team]->Height()),
			0xffffffff, 0);
		device->SetTextColor(0xffffffff, 0xffffffff);
		device->SetTextStyle(1);
		float x = (float)(m_ctrl.m_pRankBack[team]->Width() / 2) + textPos.x;
		textPos.y += 2.0f;
		WPoint rankPos(x, textPos.y);
		unsigned char rank;
		if (m_resultDlgType == 2)
			rank = (unsigned char)m_myRank;
		else
			rank = rival.rank;
		if (rank < 0xff)
			device->Print(rankPos, 1, "%d", rank);
		else
			device->Print(rankPos, 1, "-");
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xffffffff);
		y = textPos.y;
	}
	m_ctrl.m_pLevelIcon =
		g_pFresh->GetBitmap(MakeStr("level_%03d", rival.level + 1));
	if (m_ctrl.m_pLevelIcon)
	{
		device->DrawTexture(m_ctrl.m_pLevelIcon,
			WRect(pos.x + 47.0f,
				11.0f - (float)(m_ctrl.m_pLevelIcon->Height() / 2) + pos.y,
				(float)m_ctrl.m_pLevelIcon->Width(),
				(float)m_ctrl.m_pLevelIcon->Height()),
			0xffffffff, 0);
	}
	if (rival.guildUID)
	{
		m_ctrl.m_pEmblem =
			NetResourceManager::Instance()->GetEmblem983(rival.guildUID, rival.guildEmblemVer983);
		if (m_ctrl.m_pEmblem)
		{
			device->DrawTexture(m_ctrl.m_pEmblem,
				WRect(pos.x + 110.0f,
					11.0f - (float)(m_ctrl.m_pEmblem->Height() / 2) + pos.y,
					(float)m_ctrl.m_pEmblem->Width(),
					(float)m_ctrl.m_pEmblem->Height()),
				0xffffffff, 0);
		}
	}
	if (m_bTikiReport)
	{
		if (rival.uid == Doc()->m_myInfo.info.dwUID)
		{
			device->SetTextColor(0xff000000, 0xffffffff);
			device->SetTextStyle(1);
		}
		else
		{
			if (rival.state == 3)
				device->SetTextColor(0xff808080, 0xffffffff);
			else
				device->SetTextColor(0xff000000, 0xffffffff);
			device->SetTextStyle(0);
		}
	}
	else
	{
		if (rival.oid == Doc()->m_myInfo.info.dwGuid)
		{
			device->SetTextColor(0xff000000, 0xffffffff);
			device->SetTextStyle(1);
		}
		else
		{
			if (rival.state == 3)
				device->SetTextColor(0xff808080, 0xffffffff);
			else
				device->SetTextColor(0xff000000, 0xffffffff);
			device->SetTextStyle(0);
		}
	}
	WPoint namePos(pos.x + 141.0f, y);
	g_pFresh->GetManager()->PrintText(namePos, 0, rival.nickname, -1.0f,
		0xffffffff);
	WPoint scorePos(pos.x + 270.0f, y);
	if (m_resultDlgType == 2)
		device->Print(scorePos, 2, "%d", rival.guildPoint);
	else
		device->Print(scorePos, 2, "%d", rival.totalScore);
	unsigned char gameType = Doc()->m_golfGame.gameType;
	if (gameType > 3 && gameType < 6)
	{
		WPoint pangPos(pos.x + 340.0f, y);
		device->Print(pangPos, 2, "%I64d", rival.totalPang);
		WPoint bonusPos(pos.x + 347.0f, y);
		device->SetTextColor(0xff006493, 0xffffffff);
		if (rival.totalBonusPang > 0)
		{
			device->SetTextStyle(1);
			device->Print(bonusPos, 0, "(+%I64d)", rival.totalBonusPang);
		}
		else
			device->Print(bonusPos, 0, "(0)");
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xffffffff);
		bool bDrawn = false;
		// 983 0063bba0: item de assistencia 0x1be00016 com quantidade 1, fora dos tipos 11..14
		bool hasAssist = false;
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0xb) > 3)
		{
			sItemInfo* pAssist = Doc()->FindMyItemByTypeId983(0x1be00016);
			if (pAssist)
				hasAssist = pAssist->Common[0] == 1;
		}
		if (hasAssist && !m_bTikiReport && m_ctrl.m_pAssistTitle)
		{
			device->DrawTexture(m_ctrl.m_pAssistTitle,
				WRect(pos.x + 318.0f, pos.y + 14.0f,
					(float)m_ctrl.m_pAssistTitle->Width(),
					(float)m_ctrl.m_pAssistTitle->Height()),
				0xffffffff, 0);
		}
		// 983 00aee5e4: so com m_pPremium10 (+0x1bc) != 0
		if (CanShowPremium10983(&rival) && m_ctrl.m_pPremium10)
		{
			device->DrawTexture(m_ctrl.m_pPremium10,
				WRect(pos.x + 408.0f,
					11.0f - (float)(m_ctrl.m_pPremium10->Height() >> 1) +
						pos.y,
					(float)m_ctrl.m_pPremium10->Width(),
					(float)m_ctrl.m_pPremium10->Height()),
				0xffffffff, 0);
			bDrawn = true;
		}
		IFF_ITEM_COMMON* pItem =
			ItemManager()->FindCommonItem(rival.mascotTypeId);
		if (pItem)
		{
			WRect rect(pos.x + 453.0f,
				11.0f - (float)(m_ctrl.m_pMascot01->Height() >> 1) + pos.y,
				(float)m_ctrl.m_pMascot01->Width(),
				(float)m_ctrl.m_pMascot01->Height());
			const Bitmap* mascot = NULL;
			switch (pItem->TypeId)
			{
			case 0x40000001:
				mascot = m_ctrl.m_pMascot01;
				break;
			case 0x40000002:
				mascot = m_ctrl.m_pMascot02;
				break;
			case 0x40000009:
				mascot = m_ctrl.m_pMascot09;
				break;
			case 0x4000000a:
				mascot = m_ctrl.m_pMascot10;
				break;
			case 0x4000000b:
				mascot = m_ctrl.m_pMascot11;
				break;
			case 0x4000000c:
				mascot = m_ctrl.m_pMascot12;
				break;
			case 0x40000027:
				mascot = m_ctrl.m_pMascot39;
				break;
			case 0x40000028:
				mascot = m_ctrl.m_pMascot40;
				break;
			}
			if (mascot)
			{
				device->DrawTexture(mascot, rect, 0xffffffff, 0);
				bDrawn = true;
			}
		}
		if (rival.pangMastery == 1)
		{
			device->DrawTexture(m_ctrl.m_pBonusMa,
				WRect(pos.x + 470.0f,
					11.0f - (float)(m_ctrl.m_pBonusMa->Height() >> 1) + pos.y,
					(float)m_ctrl.m_pBonusMa->Width(),
					(float)m_ctrl.m_pBonusMa->Height()),
				0xffffffff, 0);
			bDrawn = true;
		}
		if (rival.pangNitro == 1)
		{
			WRect rect(0.0f, 0.0f, (float)m_ctrl.m_pBonusMa2->Width(),
				(float)m_ctrl.m_pBonusMa2->Height());
			if (rival.pangMastery == 1)
			{
				rect.x = (float)(m_ctrl.m_pBonusMa2->Width() >> 1) + pos.x +
					470.0f;
				rect.y = 11.0f - (float)(m_ctrl.m_pBonusMa2->Height() >> 1) +
					pos.y;
			}
			else
			{
				rect.x = pos.x + 470.0f;
				rect.y = 11.0f - (float)(m_ctrl.m_pBonusMa2->Height() >> 1) +
					pos.y;
			}
			device->DrawTexture(m_ctrl.m_pBonusMa2, rect, 0xffffffff, 0);
		}
		else if (bDrawn != true && m_ctrl.m_pIconZero)
		{
			device->DrawTexture(m_ctrl.m_pIconZero,
				WRect(pos.x + 440.0f,
					11.0f - (float)(m_ctrl.m_pIconZero->Height() >> 1) +
						pos.y,
					(float)m_ctrl.m_pIconZero->Width(),
					(float)m_ctrl.m_pIconZero->Height()),
				0xffffffff, 0);
		}
	}
	else if (gameType == 11)
	{
		// 983: tela do hole-in-one: acertos, tacadas, %, pang e bonus
		int hits = (int)rival.hioScore983;
		device->Print(WPoint(pos.x + 321.0f, y), 2, "%d", hits);
		device->Print(WPoint(pos.x + 371.0f, y), 2, "%d",
			(int)(signed char)rival.holeStroke[0]);
		float percent = 0.0f;
		if (hits != 0)
			percent = ((float)hits / (float)rival.holeStroke[0]) * 100.0f;
		device->Print(WPoint(pos.x + 431.0f, y), 2, "%.0f %%", (double)percent);
		device->Print(WPoint(pos.x + 474.0f, y), 2, "%d",
			(int)(rival.hioScore983 >> 32));
		device->Print(WPoint(pos.x + 526.0f, y), 2, "%I64d", rival.holePang[0]);
		WPoint bonusPos(pos.x + 531.0f, y);
		device->SetTextColor(0xff006493, 0xffffffff);
		if (rival.totalBonusPang > 0)
		{
			device->SetTextStyle(1);
			device->Print(bonusPos, 0, "(+%I64d)", rival.totalBonusPang);
		}
		else
			device->Print(bonusPos, 0, "(0)");
		device->SetTextColor(0xff000000, 0xffffffff);
		device->SetTextStyle(0);
	}
	device->SetSpace(space);
}

void FrUniteResultDlg::OnExpGaugeBarInit(int param)
{
	if (!m_ctrl.m_pExpGaugeBar)
		m_ctrl.m_pExpGaugeBar = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
}

void FrUniteResultDlg::OnExpAreaInit(int param)
{
	if (!m_ctrl.m_pExpArea)
		m_ctrl.m_pExpArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUniteResultDlg::OnExpAreaOwnerDraw(int)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		int space = gdi->SetSpace(0);
		WRect rect = m_ctrl.m_pExpGaugeBar->GetRect();
		WPoint pos(rect.x + rect.w * 0.5f, rect.y + rect.h * 0.5f - 5.0f);
		if (!m_bLevelUp)
		{
			gdi->SetTextColor(0xffffffff, 0xffffffff);
			gdi->SetTextStyle(1);
			gdi->Print(pos, 1, "%d / %d", m_exp / 10, m_maxExp / 10);
			gdi->SetTextColor(0xff000000, 0xffffffff);
			gdi->SetTextStyle(0);
		}
		else
		{
			const Bitmap* bitmap = g_pFresh->GetBitmap("font_levelup");
			if (bitmap)
			{
				gdi->DrawTexture(bitmap,
					WRect(pos.x - bitmap->Width() / 2,
						pos.y - (rect.h * 0.5f - 5.0f), bitmap->Width(),
						bitmap->Height()),
					0xffffffff, 0);
			}
		}
		gdi->SetSpace(space);
	}
}

void FrUniteResultDlg::OnRankListBoxInit(int param)
{
	if (!m_ctrl.m_pRankList)
		m_ctrl.m_pRankList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_ctrl.m_pRankList->UseRightButton(true);
}

void FrUniteResultDlg::OnRankListOwnerDraw(int param)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	sRivalData* pInfo = (sRivalData*)pItem->pData;
	if (!pInfo)
		return;
	if (Doc()->m_golfGame.gameType == 11 && pInfo->state != 2)
		return;
	float y = 0.0f;
	int space = device->SetSpace(0);
	if (pItem->selected == true &&
		!(m_bTikiReport && IsLocalContent((localContentType_t)20)))
	{
		device->Box(WRect(pItem->pos.x + 4.0f, pItem->pos.y,
						(float)m_ctrl.m_pRankList->GetItemWidth(),
						(float)m_ctrl.m_pRankList->GetItemHeight()),
			0x999b9dff, 0, 0.0f);
	}
	// 983: realGameType 0x14/0x15 (modo de eventos): linhas com bGrandPrixAI983 != 0 nao contam
	bool eventMode = (unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2;
	unsigned char team = 2;
	if (m_resultDlgType)
		team = pInfo->team;
	if (team < 3 && m_ctrl.m_pRankBack[team])
	{
		float tx = pItem->pos.x + 13.0f;
		float ty = 11.0f - (float)m_ctrl.m_pRankBack[team]->Height() * 0.5f +
			pItem->pos.y;
		device->DrawTexture(m_ctrl.m_pRankBack[team],
			WRect(tx, ty, (float)m_ctrl.m_pRankBack[team]->Width(),
				(float)m_ctrl.m_pRankBack[team]->Height()),
			0xffffffff, 0);
		device->SetTextColor(0xffffffff, 0xffffffff);
		device->SetTextStyle(1);
		float x = (float)m_ctrl.m_pRankBack[team]->Width() * 0.5f + tx;
		ty += 2.0f;
		int rank;
		if (m_resultDlgType == 2)
		{
			if (eventMode)
			{
				if (pInfo->oid == Doc()->m_myInfo.info.dwGuid &&
					pInfo->bGrandPrixAI983 == 0)
					m_myRank = pItem->no + 1;
			}
			else if (pInfo->oid == Doc()->m_myInfo.info.dwGuid)
				m_myRank = pItem->no + 1;
			rank = pItem->no + 1;
		}
		else
			rank = pInfo->rank;
		if (rank < 0xff)
			device->Print(WPoint(x, ty), 1, "%d", rank);
		else
			device->Print(WPoint(x, ty), 1, "-");
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xffffffff);
		y = ty;
	}
	IFF_STRUCT::sSkin* pSkin;
	if (pInfo->titleTypeId &&
		(pSkin = ItemManager()->FindSkin(pInfo->titleTypeId)) != NULL)
		m_ctrl.m_pLevelIcon =
			g_pFresh->GetManager()->GetBitmap("TITLES", pSkin->c.Icon);
	else
		m_ctrl.m_pLevelIcon =
			g_pFresh->GetBitmap(MakeStr("level_%03d", pInfo->level + 1));
	if (eventMode && pInfo->bGrandPrixAI983 != 0)
		m_ctrl.m_pLevelIcon = NULL;
	if (m_ctrl.m_pLevelIcon)
	{
		device->DrawTexture(m_ctrl.m_pLevelIcon,
			WRect(pItem->pos.x + 47.0f,
				11.0f - (float)(m_ctrl.m_pLevelIcon->Height() / 2) +
					pItem->pos.y,
				(float)m_ctrl.m_pLevelIcon->Width(),
				(float)m_ctrl.m_pLevelIcon->Height()),
			0xffffffff, 0);
	}
	if (pInfo->guildUID)
	{
		m_ctrl.m_pEmblem =
			NetResourceManager::Instance()->GetEmblem983(pInfo->guildUID, pInfo->guildEmblemVer983);
		if (m_ctrl.m_pEmblem)
		{
			device->DrawTexture(m_ctrl.m_pEmblem,
				WRect(pItem->pos.x + 110.0f,
					11.0f - (float)(m_ctrl.m_pEmblem->Height() / 2) +
						pItem->pos.y,
					(float)m_ctrl.m_pEmblem->Width(),
					(float)m_ctrl.m_pEmblem->Height()),
				0xffffffff, 0);
		}
	}
	bool isMe;
	if (m_bTikiReport)
		isMe = pInfo->uid == Doc()->m_myInfo.info.dwUID;
	else if (eventMode)
		isMe = pInfo->oid == Doc()->m_myInfo.info.dwGuid &&
			pInfo->bGrandPrixAI983 == 0;
	else
		isMe = pInfo->oid == Doc()->m_myInfo.info.dwGuid;
	if (isMe)
	{
		device->SetTextColor(0xff000000, 0xffffffff);
		device->SetTextStyle(1);
	}
	else
	{
		if (pInfo->state == 3)
			device->SetTextColor(0xff808080, 0xffffffff);
		else
			device->SetTextColor(0xff000000, 0xffffffff);
		device->SetTextStyle(0);
	}
	WPoint namePos(pItem->pos.x + 141.0f, y);
	g_pFresh->GetManager()->PrintText(namePos, 0, pInfo->nickname, -1.0f,
		0xffffffff);
	WPoint scorePos(pItem->pos.x + 270.0f, y);
	if (m_resultDlgType == 2)
		device->Print(scorePos, 2, "%d", pInfo->guildPoint);
	else
		device->Print(scorePos, 2, "%d", pInfo->totalScore);
	unsigned char gameType = Doc()->m_golfGame.gameType;
	if (gameType > 3 && gameType < 6)
	{
		WPoint pangPos(pItem->pos.x + 340.0f, y);
		device->Print(pangPos, 2, "%I64d", pInfo->totalPang);
		WPoint bonusPos(pItem->pos.x + 347.0f, y);
		device->SetTextColor(0xff006493, 0xffffffff);
		if (pInfo->totalBonusPang > 0)
		{
			device->SetTextStyle(1);
			device->Print(bonusPos, 0, "(+%I64d)", pInfo->totalBonusPang);
		}
		else
			device->Print(bonusPos, 0, "(0)");
		device->SetTextColor(0xff000000, 0xffffffff);
		device->SetTextStyle(0);
		bool bDrawn = false;
		// 983 00aeabac: so com o bitmap carregado (this+0x1bc != 0)
		if (CanShowPremium10983(pInfo) && m_ctrl.m_pPremium10)
		{
			device->DrawTexture(m_ctrl.m_pPremium10,
				WRect(pItem->pos.x + 408.0f,
					11.0f - (float)(m_ctrl.m_pPremium10->Height() / 2) +
						pItem->pos.y,
					(float)m_ctrl.m_pPremium10->Width(),
					(float)m_ctrl.m_pPremium10->Height()),
				0xffffffff, 0);
			bDrawn = true;
		}
		IFF_ITEM_COMMON* pMascot =
			ItemManager()->FindCommonItem(pInfo->mascotTypeId);
		if (pMascot)
		{
			WRect rect(pItem->pos.x + 453.0f,
				11.0f - (float)(m_ctrl.m_pMascot01->Height() / 2) +
					pItem->pos.y,
				(float)m_ctrl.m_pMascot01->Width(),
				(float)m_ctrl.m_pMascot01->Height());
			const Bitmap* mascot = NULL;
			switch (pMascot->TypeId)
			{
			case 0x40000001:
				mascot = m_ctrl.m_pMascot01;
				break;
			case 0x40000002:
				mascot = m_ctrl.m_pMascot02;
				break;
			case 0x40000009:
				mascot = m_ctrl.m_pMascot09;
				break;
			case 0x4000000a:
				mascot = m_ctrl.m_pMascot10;
				break;
			case 0x4000000b:
				mascot = m_ctrl.m_pMascot11;
				break;
			case 0x4000000c:
				mascot = m_ctrl.m_pMascot12;
				break;
			case 0x40000027:
				mascot = m_ctrl.m_pMascot39;
				break;
			case 0x40000028:
				mascot = m_ctrl.m_pMascot40;
				break;
			}
			if (mascot)
			{
				device->DrawTexture(mascot, rect, 0xffffffff, 0);
				bDrawn = true;
			}
		}
		if (pInfo->pangMastery == 1)
		{
			device->DrawTexture(m_ctrl.m_pBonusMa,
				WRect(pItem->pos.x + 470.0f,
					11.0f - (float)(m_ctrl.m_pBonusMa->Height() / 2) +
						pItem->pos.y,
					(float)m_ctrl.m_pBonusMa->Width(),
					(float)m_ctrl.m_pBonusMa->Height()),
				0xffffffff, 0);
			bDrawn = true;
		}
		if (pInfo->pangNitro == 1)
		{
			WRect rect(0.0f, 0.0f, (float)m_ctrl.m_pBonusMa2->Width(),
				(float)m_ctrl.m_pBonusMa2->Height());
			if (pInfo->pangMastery == 1)
			{
				rect.x = (float)(m_ctrl.m_pBonusMa2->Width() / 2) + 470.0f +
					pItem->pos.x;
				rect.y = 11.0f - (float)(m_ctrl.m_pBonusMa2->Height() / 2) +
					pItem->pos.y;
			}
			else
			{
				rect.x = pItem->pos.x + 470.0f;
				rect.y = 11.0f - (float)(m_ctrl.m_pBonusMa2->Height() / 2) +
					pItem->pos.y;
			}
			device->DrawTexture(m_ctrl.m_pBonusMa2, rect, 0xffffffff, 0);
			bDrawn = true;
		}
		else if (bDrawn != true && m_ctrl.m_pIconZero)
		{
			device->DrawTexture(m_ctrl.m_pIconZero,
				WRect(pItem->pos.x + 440.0f,
					11.0f - (float)(m_ctrl.m_pIconZero->Height() / 2) +
						pItem->pos.y,
					(float)m_ctrl.m_pIconZero->Width(),
					(float)m_ctrl.m_pIconZero->Height()),
				0xffffffff, 0);
		}
		int medal = -1;
		if (m_resultDlgType != 3 &&
			!(IsLocalContent((localContentType_t)44 /*983: era 43; interpolado*/) &&
				Doc()->m_roomInfo.nUserLimit >= 100))
		{
			if (pInfo->hole == Doc()->m_holeOrder[0])
				medal = -1;
			else
			{
				medal = GetMedalIndex(pInfo);
				if (medal >= 0 && medal <= 2 && pInfo->flag > 3 &&
					!m_bTikiReport)
					medal = 3;
			}
		}
		float offset = 0.0f;
		if (medal >= 0 && medal <= 2)
		{
			device->DrawTexture(m_ctrl.m_pMedal[medal],
				WRect(pItem->pos.x + 515.0f,
					11.0f - (float)(m_ctrl.m_pMedal[medal]->Height() / 2) +
						pItem->pos.y,
					(float)m_ctrl.m_pMedal[medal]->Width(),
					(float)m_ctrl.m_pMedal[medal]->Height()),
				0xffffffff, 0);
			offset = (float)m_ctrl.m_pMedal[medal]->Width();
		}
		// 983: premios coloridos so com mais de 14 jogadores e mais de 17 buracos
		int set = 0;
		if (Doc()->m_rivalList.size() > 14 && Doc()->m_golfGame.holes > 17)
			set = 1;
		const unsigned long* prizeCount = &Doc()->m_guildRoundScore[0];
		for (int i = 0; i < 6; i++)
		{
			bool mine = Doc()->m_awardItem[i].uid == pInfo->oid;
			if (eventMode)
				mine = mine && pInfo->bGrandPrixAI983 == 0;
			if (!mine)
				continue;
			const Bitmap* prize = (&m_ctrl.m_pPrizeGray[0])[i + set * 6];
			WRect rect(pItem->pos.x + 515.0f + offset,
				11.0f - (float)(prize->Height() / 2) + pItem->pos.y,
				(float)prize->Width(), (float)prize->Height());
			device->DrawTexture(prize, rect, 0xffffffff, 0);
			if (m_resultDlgType != 2 && prizeCount[i] != 0)
				m_ctrl.m_pAngelFont->Print(g_view, rect.x + 10.0f,
					rect.y + 10.0f, MakeStr("%d", prizeCount[i]), 0,
					0xffffffff, NULL);
			if (!m_bRankTooltip)
				m_bRankTooltip = SetTooltip(rect, i);
			offset += (float)prize->Width();
		}
		if ((int)offset == 0 && m_ctrl.m_pIconZero)
		{
			device->DrawTexture(m_ctrl.m_pIconZero,
				WRect(pItem->pos.x + 515.0f,
					11.0f - (float)(m_ctrl.m_pIconZero->Height() / 2) +
						pItem->pos.y,
					(float)m_ctrl.m_pIconZero->Width(),
					(float)m_ctrl.m_pIconZero->Height()),
				0xffffffff, 0);
		}
	}
	else if (gameType == 11)
	{
		// 983: tela do hole-in-one (tipo 11): acertos, tacadas, %, pang, trofeu e premios
		int hits = (int)pInfo->hioScore983;
		device->Print(WPoint(pItem->pos.x + 321.0f, y), 2, "%d", hits);
		device->Print(WPoint(pItem->pos.x + 371.0f, y), 2, "%d",
			(int)(signed char)pInfo->holeStroke[0]);
		float percent = 0.0f;
		if (hits != 0)
			percent = ((float)hits / (float)pInfo->holeStroke[0]) * 100.0f;
		device->Print(WPoint(pItem->pos.x + 431.0f, y), 2, "%.0f %%",
			(double)percent);
		device->Print(WPoint(pItem->pos.x + 474.0f, y), 2, "%d",
			(int)(pInfo->hioScore983 >> 32));
		device->Print(WPoint(pItem->pos.x + 526.0f, y), 2, "%I64d",
			pInfo->holePang[0]);
		WPoint bonusPos(pItem->pos.x + 531.0f, y);
		device->SetTextColor(0xff006493, 0xffffffff);
		if (pInfo->totalBonusPang > 0)
		{
			device->SetTextStyle(1);
			device->Print(bonusPos, 0, "(+%I64d)", pInfo->totalBonusPang);
		}
		else
			device->Print(bonusPos, 0, "(0)");
		device->SetTextColor(0xff000000, 0xffffffff);
		device->SetTextStyle(0);
		float offset = 0.0f;
		int idx = -1;
		if (pInfo->hioTrophyTid983 == 0x2d0a6200)
			idx = 0;
		else if (pInfo->hioTrophyTid983 == 0x2d0a6300)
			idx = 1;
		else if (pInfo->hioTrophyTid983 == 0x2d0a6400)
			idx = 2;
		if (pInfo->hioTrophyTid983 != 0 && idx >= 0)
		{
			const Bitmap* trophy = m_ctrl.m_pMedal[idx];
			device->DrawTexture(trophy,
				WRect(pItem->pos.x + 581.0f,
					11.0f - (float)(trophy->Height() >> 1) + pItem->pos.y,
					(float)trophy->Width(), (float)trophy->Height()),
				0xffffffff, 0);
			offset = (float)trophy->Width() + 0.0f;
		}
		if (pInfo->hioPrizePang983 > 0)
		{
			const char* name = "item1_16";
			if (Doc()->m_hioJackpotMode983 == 1)
				name = "item_JackpotPang";
			const Bitmap* bitmap = g_pFresh->GetBitmap(name);
			device->DrawTexture(bitmap,
				WRect(pItem->pos.x + 591.0f + offset, pItem->pos.y + 0.0f,
					28.0f, 28.0f),
				0xffffffff, 0);
			offset += 30.0f;
		}
		_gateway::_golfgame::CHoleInOneGameDoc::PairList* pPairs = NULL;
		if (m_pHoleInOneData)
			pPairs = ((_gateway::_golfgame::CHoleInOneGameDoc*)m_pHoleInOneData)
						 ->GetPlayerPairs983(pInfo->oid);
		if (pPairs)
		{
			for (size_t i = 0; i < pPairs->size(); i++)
			{
				IFF_ITEM_COMMON* pCommon =
					ItemManager()->FindCommonItem((*pPairs)[i].m_a);
				const Bitmap* bitmap = g_pFresh->GetBitmap(pCommon->Icon);
				device->DrawTexture(bitmap,
					WRect(pItem->pos.x + 591.0f + offset, pItem->pos.y + 1.0f,
						30.0f, 30.0f),
					0xffffffff, 0);
				offset += 24.0f;
			}
		}
	}
	device->SetSpace(space);
}

void FrUniteResultDlg::OnRankListLBtnUp()
{
	if (0)
		((FrUniteResultDlg*)0)->FrUniteResultDlg::OnDescAreaOwnerDraw(*(int*)0);
	if (Doc()->m_golfGame.gameType == 11) // 983: gameType 0xb nao seleciona
		return;
	if (m_bTikiReport)
		return;
	FrListItem* pItem = m_ctrl.m_pRankList->GetItemUnderCursor();
	if (!pItem)
		return;
	sRivalData* pInfo = (sRivalData*)pItem->pData;
	if (!pInfo)
		return;
	{
		unsigned char realGT = Doc()->m_roomInfo.realGameType; // 983: 0x14/0x15 com +0x1b4 != 0 nao seleciona
		if ((unsigned char)(realGT - 0x14) <= 1 && pInfo->bGrandPrixAI983)
			return;
	}
	m_selectedUID = pInfo->oid;
	m_ctrl.m_pRankList->SelectItem(pItem, true);
	if (!m_ctrl.m_pScoreDlg)
	{
		if (Doc()->m_golfGame.gameType == 6)
		{
			m_ctrl.m_pScoreDlg = CreateForm<FrScoreDlg>(g_pFresh->GetManager(),
				this, "score_guild");
			m_ctrl.m_pScoreDlg->Open(
				(FRESH_PFN_RESULT)&FrUniteResultDlg::OnScoreDlgResult, 0);
			m_ctrl.m_pScoreDlg->EnableDrag(false);
			m_ctrl.m_pScoreDlg->SetGuildPlayer(m_selectedUID);
			m_ctrl.m_pScoreDlg->SetVisibleCloseBtn(false);
		}
		else
		{
			m_ctrl.m_pScoreDlg =
				CreateForm<FrScoreDlg>(g_pFresh->GetManager(), this, "score");
			m_ctrl.m_pScoreDlg->Open(
				(FRESH_PFN_RESULT)&FrUniteResultDlg::OnScoreDlgResult, 0);
			m_ctrl.m_pScoreDlg->EnableDrag(false);
			if (m_selectedUID == 0xffffffff)
				m_selectedUID = Doc()->m_myInfo.info.dwGuid;
			m_ctrl.m_pScoreDlg->SetPlayer(m_selectedUID);
			m_ctrl.m_pScoreDlg->SetVisibleCloseBtn(true);
		}
	}
	if (m_ctrl.m_pScoreDlg)
		m_ctrl.m_pScoreDlg->SetPlayer(m_selectedUID);
}

void FrUniteResultDlg::OnRankListRBtnUp()
{
	if (m_bTikiReport)
		return;
	FrListItem* pItem = m_ctrl.m_pRankList->GetItemUnderCursor();
	if (!pItem)
		return;
	sRivalData* pInfo = (sRivalData*)pItem->pData;
	if (!pInfo)
		return;
	// 983 (00aeb8d0): no modo 0x14/0x15 a linha de AI (bGrandPrixAI983 != 0) abre
	// a ficha "aiinfo" (CUserInfo+0x30) e nao troca o jogador do placar.
	if ((unsigned)(Doc()->m_roomInfo.realGameType - 0x14) <= 1 &&
		pInfo->bGrandPrixAI983)
	{
		CUserInfo::Instance()->SetInfo(pInfo->uid, pInfo->oid, true, true,
			false, false, pInfo->nickname, pInfo->bGrandPrixAI983,
			pInfo->grandPrixAIId983);
		if (CUserInfo::Instance())
		{
			FrForm* dlg = CUserInfo::Instance()->GetAiInfoDlg();
			if (dlg)
				dlg->DisableFlag(1);
		}
		return;
	}
	m_selectedUID = pInfo->oid;
	if (m_ctrl.m_pScoreDlg)
		m_ctrl.m_pScoreDlg->SetPlayer(m_selectedUID);
	if (CUserInfo::Instance() && pInfo->state != 3)
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(pInfo->oid);
		if (it != Doc()->m_briefUserInfoMap.end())
			it->second.roomIndex = Doc()->m_roomInfo.roomGuid;
		CUserInfo::Instance()->SetInfo(pInfo->uid, pInfo->oid, true, true,
			false, false, pInfo->nickname);
		FrUserInfoForm* dlg = USERINFODLG();
		if (dlg)
			dlg->DisableFlag(1);
	}
}

void FrUniteResultDlg::OnDescAreaInit(int param)
{
	if (!m_ctrl.m_pDescArea)
		m_ctrl.m_pDescArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUniteResultDlg::OnDescAreaOwnerDraw(int)
{
	// 983 00ae8d10 (out/B1442): formato por m_resultDlgType (+0x250) / m_eventFlag (Doc +0x10499),
	// arg m_winner.nickname (+0x278), em (rect.x + 15, rect.y - 1)
	const WRect& rect = m_ctrl.m_pDescArea->GetRect();
	float x = rect.x;
	float y = rect.y;
	x += 15.0;
	y -= 1.0;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->SetTextStyle(0);
	const char* fmt;
	if (!m_resultDlgType)
		fmt = "1\265\356\300\272 %s\264\324\300\314 \302\367\301\366\307\317\274\314\275\300\264\317\264\331.";
	else if (!Doc()->m_eventFlag)
		fmt = "RED \306\300\300\314 \275\302\270\256\307\317\277\264\275\300\264\317\264\331.";
	else
		fmt = "BLUE \306\300\300\314 \275\302\270\256\307\317\277\264\275\300\264\317\264\331.";
	const char* text = MakeStr(K2L_Compatibility(fmt), m_winner.nickname);
	if (text)
		g_pFresh->GetManager()->PrintText(WPoint(x, y), 0, text, -1.0f, 0xffffffff);
}

void FrUniteResultDlg::OnAwardAreaInit(int param)
{
	if (!m_ctrl.m_pAwardArea)
		m_ctrl.m_pAwardArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUniteResultDlg::OnAwardAreaOwnerDraw(int)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	// 983: 455 na tela do tipo 11 (que nao desenha premio), 382 nas demais
	float baseY = 382.0f;
	unsigned char gameType = Doc()->m_golfGame.gameType;
	if (gameType == 11)
		return;
	bool bDrawn = false;
	float x = GetRect().x;
	float y = GetRect().y;
	if (gameType > 3 && gameType < 6)
	{
		sRivalData rival;
		if (m_bTikiReport && IsLocalContent((localContentType_t)20))
			rival = Doc()->m_rivalList[m_tikiReportIndex];
		else
			rival = Doc()->m_rivalList[MyIndex()];
		IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(Doc()->m_roomType);
		float drawX = x + 505.0f;
		if (pMatch && Doc()->m_eventType >= 1 && Doc()->m_eventType <= 3)
		{
			const Bitmap* bitmap =
				g_pFresh->GetBitmap(pMatch->Icon[Doc()->m_eventType - 1]);
			if (bitmap)
			{
				device->DrawTexture(bitmap,
					WRect(505.0f - (float)(bitmap->Width() >> 1) + x,
						baseY - (float)(bitmap->Height() >> 1) + y,
						(float)bitmap->Width(), (float)bitmap->Height()),
					0xffffffff, 0);
				drawX += (float)(bitmap->Width() >> 1);
				bDrawn = true;
			}
		}
		// 983: premios coloridos so com mais de 14 jogadores e mais de 17 buracos
		int set = 0;
		if (Doc()->m_rivalList.size() > 14 && Doc()->m_golfGame.holes > 17)
			set = 1;
		// 983: Doc+0x106e4: 6 contadores por premio (no nosso doc: guildRoundScore..teamPang)
		const unsigned long* prizeCount = &Doc()->m_guildRoundScore[0];
		for (int i = 0; i < 6; i++)
		{
			unsigned long uid = m_bTikiReport ? rival.uid : MyGuid(true);
			if (Doc()->m_awardItem[i].uid != uid)
				continue;
			const Bitmap* prize = (&m_ctrl.m_pPrizeGray[0])[i + set * 6];
			WRect rect(drawX, baseY - (float)(prize->Height() >> 1) + y,
				(float)prize->Width(), (float)prize->Height());
			device->DrawTexture(prize, rect, 0xffffffff, 0);
			if (m_resultDlgType != 2 && prizeCount[i] != 0)
				m_ctrl.m_pAngelFont->Print(g_view, rect.x + 10.0f, rect.y + 10.0f,
					MakeStr("%d", prizeCount[i]), 0, 0xffffffff, NULL);
			drawX += (float)prize->Width();
			if (!m_bAwardTooltip)
				m_bAwardTooltip = SetTooltip(rect, i);
			bDrawn = true;
		}
		if (bDrawn)
			return;
	}
	const Bitmap* bitmap = g_pFresh->GetBitmap("icon_zero_vs");
	if (bitmap)
	{
		device->DrawTexture(bitmap,
			WRect(x + 522.0f, baseY - (float)(bitmap->Height() >> 1) + y,
				(float)bitmap->Width(), (float)bitmap->Height()),
			0xffffffff, 0);
	}
}

void FrUniteResultDlg::OnItemAreaInit(int param)
{
	if (!m_ctrl.m_pItemArea)
		m_ctrl.m_pItemArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUniteResultDlg::OnItemAreaOwnerDraw(int)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	WPoint pos(GetRect().x, GetRect().y);
	const Bitmap* bitmap = NULL;
	float offset = 0.0f;
	float couponOffset = 0.0f;
	// 983: 455 na tela do tipo 11, 382 nas demais
	float baseY = 382.0f;
	if (Doc()->m_golfGame.gameType == 11)
		baseY = 455.0f;
	for (int i = 0; i < 12; i++)
	{
		if (Doc()->m_awardItem[i].uid == MyGuid(true) &&
			Doc()->m_awardItem[i].pang)
		{
			IFF_ITEM_COMMON* pItem =
				ItemManager()->FindCommonItem(Doc()->m_awardItem[i].pang);
			if (pItem)
			{
				switch ((unsigned long)Doc()->m_awardItem[i].pang >> 26)
				{
				case 2:
					bitmap = g_pFresh->GetManager()->GetBitmap("FASHION",
						pItem->Icon);
					break;
				case 5:
					bitmap =
						g_pFresh->GetManager()->GetBitmap("ITEMS", pItem->Icon);
					break;
				case 6:
					bitmap =
						g_pFresh->GetManager()->GetBitmap("ITEMS", pItem->Icon);
					break;
				default:
					bitmap = NULL;
					continue;
				}
			}
			if (bitmap)
			{
				if (ItemManager()->GetCouponKind(Doc()->m_awardItem[i].pang) ==
					1)
					couponOffset = -10.0f;
				device->DrawTexture(bitmap,
					WRect(372.0f - ((float)bitmap->Width() + offset) + pos.x,
						baseY - (float)((bitmap->Height() - 23) / 2) + pos.y +
							couponOffset,
						(float)bitmap->Width(), (float)bitmap->Height()),
					0xffffffff, 0);
				offset += (float)bitmap->Width();
			}
		}
	}
	unsigned char gameType = Doc()->m_golfGame.gameType;
	if (gameType > 3)
	{
		if (gameType < 6)
		{
			const WRect& area = m_ctrl.m_pItemArea->GetRect();
			for (std::map<unsigned long, int>::iterator it = m_prizeMap.begin();
				it != m_prizeMap.end(); ++it)
			{
				unsigned long id = (*it).first;
				int count = (*it).second;
				IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(id);
				if (pItem)
				{
					bitmap = g_pFresh->GetBitmap(pItem->Icon);
					if (bitmap)
					{
						float w = (float)bitmap->Width();
						WRect rect((area.x + area.w) - w * 0.75f - offset,
							area.y, w, (float)bitmap->Height());
						if (ItemManager()->GetCouponKind(id) != 0)
							rect.x -= 14.0f;
						device->DrawTexture(bitmap, rect, 0xffffffff, 0);
						if (count > 1 && id != 0x1a000010)
						{
							char text[8] = "";
							itoa(count, text, 10);
							m_ctrl.m_pWindFont->Print(g_view, rect.x + 30.0f,
								area.y + 35.0f, text, 0, 0xffffffff, NULL);
						}
						offset += (float)bitmap->Width() * 0.75f;
					}
				}
			}
		}
		else if (gameType == 11)
		{
			sRivalData me = Doc()->m_rivalList[MyIndex()];
			int total = 0;
			if (me.hioTrophyTid983 != 0)
				total = 1;
			if (me.hioPrizePang983 > 0)
				total++;
			_gateway::_golfgame::CHoleInOneGameDoc::PairList* pPairs = NULL;
			if (m_pHoleInOneData)
			{
				pPairs = ((_gateway::_golfgame::CHoleInOneGameDoc*)m_pHoleInOneData)
							 ->GetPlayerPairs983(Doc()->m_myInfo.info.dwGuid);
				if (pPairs && pPairs->size())
					total += (int)pPairs->size();
			}
			float x0 = ((pos.x + GetRect().w) * 0.5f + pos.x) -
				(float)total * 70.0f * 0.5f;
			float y0 = pos.y + baseY;
			offset = 0.0f;
			if (me.hioTrophyTid983 != 0)
			{
				int idx;
				if (me.hioTrophyTid983 == 0x2d0a6200)
					idx = 0;
				else if (me.hioTrophyTid983 == 0x2d0a6300)
					idx = 1;
				else if (me.hioTrophyTid983 == 0x2d0a6400)
					idx = 2;
				else
					idx = -1;
				if (idx >= 0)
				{
					const Bitmap* trophy = m_pHoleInOneTrophy[idx];
					device->DrawTexture(trophy,
						WRect(x0 + 0.0f, y0 - 25.0f,
							(float)trophy->Width(), (float)trophy->Height()),
						0xffffffff, 0);
					offset = 70.0f;
				}
			}
			if (me.hioPrizePang983 > 0)
			{
				const char* name = "item1_16";
				if (Doc()->m_hioJackpotMode983 == 1)
					name = "item_JackpotPang";
				bitmap = g_pFresh->GetBitmap(name);
				WRect rect(x0 + offset, y0 - 23.0f, (float)bitmap->Width(),
					(float)bitmap->Height());
				device->DrawTexture(bitmap, rect, 0xffffffff, 0);
				char text[12] = "";
				itoa((int)me.hioPrizePang983, text, 10);
				m_ctrl.m_pWindFont->Print(g_view, rect.x + 30.0f,
					rect.y + 35.0f, text, 0, 0xffffffff, NULL);
				offset += 80.0f;
			}
			if (pPairs)
			{
				for (size_t i = 0; i < pPairs->size(); i++)
				{
					IFF_ITEM_COMMON* pItem =
						ItemManager()->FindCommonItem((*pPairs)[i].m_a);
					if (!pItem)
						continue;
					bitmap = g_pFresh->GetBitmap(pItem->Icon);
					float scale = 0.8f;
					if ((float)bitmap->Width() < 70.0f)
						scale = 1.0f;
					WRect rect(x0 + offset, y0 - 25.0f,
						(float)bitmap->Width() * scale,
						(float)bitmap->Height() * scale);
					device->DrawTexture(bitmap, rect, 0xffffffff, 0);
					if ((*pPairs)[i].m_b > 1)
					{
						char text[12] = "";
						itoa((*pPairs)[i].m_b, text, 10);
						m_ctrl.m_pWindFont->Print(g_view, rect.x + 30.0f,
							y0 + 10.0f, text, 0, 0xffffffff, NULL);
					}
					offset += 70.0f;
				}
			}
			// 983: com a lista de pares vazia segue para o teste do icone zero
			if (pPairs && pPairs->size())
				return;
		}
	}
	if (offset < 0.01f)
	{
		bitmap = g_pFresh->GetBitmap("icon_zero_vs");
		device->DrawTexture(bitmap,
			WRect(pos.x + 260.0f,
				baseY - (float)(bitmap->Height() / 2) + pos.y,
				(float)bitmap->Width(), (float)bitmap->Height()),
			0xffffffff, 0);
	}
}

C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.realGameType) == 0x109b8 && offsetof(CSharedDoc, m_holeOrder) == 0x1051e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.bAdminMaster) == 0x1095e);

void FrUniteResultDlg::OnTeamResultInit(int param)
{
	if (Doc()->m_golfGame.gameType == 0xb ||
		(unsigned)(Doc()->m_roomInfo.realGameType - 0x14) <= 1) // B1450: ex-cru Doc()+0x109b8
		return;
	if (!m_ctrl.m_pTeamResult)
		m_ctrl.m_pTeamResult = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrUniteResultDlg::OnTeamResultOwnerDraw(int)
{
	if (Doc()->m_golfGame.gameType == 11) // 983: gameType 0xb nao desenha o placar de times
		return;
	if (!m_resultDlgType)
		return;
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	WRect rect = GetRect();
	WRect redRect(rect.x + 13.0f,
		468.0f - (float)(m_ctrl.m_pRankBack[0]->Height() / 2) + rect.y,
		(float)m_ctrl.m_pRankBack[0]->Width(),
		(float)m_ctrl.m_pRankBack[0]->Height());
	device->DrawTexture(m_ctrl.m_pRankBack[0], redRect, 0xffffffff, 0);
	WRect blueRect(redRect.x,
		492.0f - (float)(m_ctrl.m_pRankBack[1]->Height() / 2) + rect.y,
		(float)m_ctrl.m_pRankBack[1]->Width(),
		(float)m_ctrl.m_pRankBack[1]->Height());
	device->DrawTexture(m_ctrl.m_pRankBack[1], blueRect, 0xffffffff, 0);
	if (m_resultDlgType == 1)
	{
		const Bitmap* bitmap = g_pFresh->GetBitmap("icon_redteam");
		if (bitmap)
		{
			device->DrawTexture(bitmap,
				WRect(117.0f - (float)(bitmap->Width() / 2) + rect.x,
					468.0f - (float)(bitmap->Height() / 2) + rect.y,
					(float)bitmap->Width(), (float)bitmap->Height()),
				0xffffffff, 0);
		}
		bitmap = g_pFresh->GetBitmap("icon_blueteam");
		if (bitmap)
		{
			device->DrawTexture(bitmap,
				WRect(117.0f - (float)(bitmap->Width() / 2) + rect.x,
					492.0f - (float)(bitmap->Height() / 2) + rect.y,
					(float)bitmap->Width(), (float)bitmap->Height()),
				0xffffffff, 0);
		}
	}
	else
	{
		const Bitmap* bitmap = NetResourceManager::Instance()->GetEmblem983(
			Doc()->m_roomInfo.GuildInfo.nGuildID[0], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[0]);
		if (bitmap)
		{
			device->DrawTexture(bitmap,
				WRect(117.0f - (float)(bitmap->Width() / 2) + rect.x,
					471.0f - (float)(bitmap->Height() / 2) + rect.y,
					(float)bitmap->Width(), (float)bitmap->Height()),
				0xffffffff, 0);
		}
		bitmap = NetResourceManager::Instance()->GetEmblem983(
			Doc()->m_roomInfo.GuildInfo.nGuildID[1], Doc()->m_roomInfo.GuildInfo.nEmblemVer983[1]);
		if (bitmap)
		{
			device->DrawTexture(bitmap,
				WRect(117.0f - (float)(bitmap->Width() / 2) + rect.x,
					492.0f - (float)(bitmap->Height() / 2) + rect.y,
					(float)bitmap->Width(), (float)bitmap->Height()),
				0xffffffff, 0);
		}
		const WRect& area = m_ctrl.m_pTeamResult->GetRect();
		g_pFresh->GetManager()->PrintText(
			WPoint(area.x + 120.0f, area.y + 17.0f), 0,
			Doc()->m_roomInfo.GuildInfo.szName[0], -1.0f, 0xffffffff);
		g_pFresh->GetManager()->PrintText(
			WPoint(area.x + 120.0f, area.y + 39.0f), 0,
			Doc()->m_roomInfo.GuildInfo.szName[1], -1.0f, 0xffffffff);
	}
	rect = GetRect();
	float x = rect.x + 277.0f;
	float y = rect.y + 463.0f;
	device->Print(WPoint(x, y), 1, "%d", Doc()->m_teamResult[0].score);
	float y2 = rect.y + 487.0f;
	device->Print(WPoint(x, y2), 1, "%d", Doc()->m_teamResult[1].score);
	x = rect.x + 357.0f;
	device->Print(WPoint(x, y), 1, "%I64d", Doc()->m_teamResult[0].pang);
	device->Print(WPoint(x, y2), 1, "%I64d", Doc()->m_teamResult[1].pang);

	const Bitmap* bitmap;
	if (Doc()->m_eventFlag == 0)
		bitmap = g_pFresh->GetBitmap("30in_team_redteam_win");
	else if (Doc()->m_eventFlag == 1)
		bitmap = g_pFresh->GetBitmap("30in_team_blueteam_win");
	else if (IsLocalContent((localContentType_t)20) && m_bTikiReport)
	{
		if (Doc()->m_rivalList[m_tikiReportIndex].team == 0)
			bitmap = g_pFresh->GetBitmap("30in_team_redteam_draw");
		else
			bitmap = g_pFresh->GetBitmap("30in_team_blueteam_draw");
	}
	else
	{
		if (Doc()->m_rivalList[MyIndex()].team == 0)
			bitmap = g_pFresh->GetBitmap("30in_team_redteam_draw");
		else
			bitmap = g_pFresh->GetBitmap("30in_team_blueteam_draw");
	}
	if (bitmap)
	{
		m_teamAniTime = m_teamAniTime < 3.14f ? m_teamAniTime : 3.14f;
		float scale = sinf(m_teamAniTime) * 0.2f + 1.0f;
		float cy = 482.0f - (float)bitmap->Height() * scale * 0.5f + rect.y;
		float cx = 522.0f - (float)bitmap->Width() * scale * 0.5f + rect.x;
		rect.x = cx;
		rect.y = cy;
		rect.w = (float)bitmap->Width() * scale;
		rect.h = (float)bitmap->Height() * scale;
		device->DrawTexture(bitmap, rect, 0xffffffff, 0);
	}
}

void FrUniteResultDlg::OnTooltipOwnerDraw(int)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;
	if (!m_ctrl.m_pCurTooltip)
		return;
	float x = g_input->GetMousePoint().x;
	float y = g_input->GetMousePoint().y;
	float width = (float)m_ctrl.m_pCurTooltip->Width();
	float height = (float)m_ctrl.m_pCurTooltip->Height();
	WRect rect(x + 12.0f, y + 23.0f, width, height);
	if (rect.x >= g_view->GetWidth() - width)
		rect.x = g_view->GetWidth() - width;
	if (rect.y >= g_view->GetHeight() - height)
		rect.y = g_view->GetHeight() - height;
	gdi->DrawTexture(m_ctrl.m_pCurTooltip, rect, 0xffffffff, 0);
}

void FrUniteResultDlg::OnNextLevelTextInit(int param)
{
	if (Doc()->m_golfGame.gameType == 0xb)
		return;
	if (!m_ctrl.m_pNextLevelText)
		m_ctrl.m_pNextLevelText = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

__forceinline FrUniteResultDlg::sUniteResultControls::sUniteResultControls()
{
	m_pCancelBtn = NULL;
	m_pAwardInfoBtn = NULL;
	m_pRankList = NULL;
	m_pMyInfoArea = NULL;
	m_pAwardInfoDlg = NULL;
	m_pScoreDlg = NULL;
	m_pTreasureScore = NULL;
	m_pTreasureGift = NULL;
	m_pExpGaugeBar = NULL;
	m_pExpArea = NULL;
	m_pDescArea = NULL;
	m_pAwardArea = NULL;
	m_pItemArea = NULL;
	m_pTeamResult = NULL;
	m_pNextLevelText = NULL;
	m_pRankBack[0] = NULL;
	m_pRankBack[1] = NULL;
	m_pRankBack[2] = NULL;
	m_pLevelIcon = NULL;
	m_pEmblem = NULL;
	m_pPcBonus = NULL;
	m_pPremium10 = NULL;
	m_pAssistTitle = NULL;
	m_pMascot02 = NULL;
	m_pMascot01 = NULL;
	m_pMascot09 = NULL;
	m_pMascot10 = NULL;
	m_pMascot11 = NULL;
	m_pMascot12 = NULL;
	m_pMascot39 = NULL;
	m_pMascot40 = NULL;
	m_pBonusMa = NULL;
	m_pBonusMa2 = NULL;
	m_pMedal[0] = NULL;
	m_pMedal[1] = NULL;
	m_pMedal[2] = NULL;
	m_pMedal[3] = NULL;
	for (int i = 0; i < 6; ++i)
		m_pPrizeGray[i] = NULL;
	m_pPrize[0] = NULL;
	m_pPrize[1] = NULL;
	m_pPrize[2] = NULL;
	m_pPrize[3] = NULL;
	m_pPrize[4] = NULL;
	m_pPrize[5] = NULL;
	m_pIconZero = NULL;
	m_pCurTooltip = NULL;
	m_pTooltip[0] = NULL;
	m_pTooltip[1] = NULL;
	m_pTooltip[2] = NULL;
	m_pTooltip[3] = NULL;
	m_pTooltip[4] = NULL;
	m_pTooltip[5] = NULL;
}

FrUniteResultDlg::FrUniteResultDlg()
{
	m_expTime = 0;
	m_resultDlgType = 0;
	m_bLevelUp = false;
	m_level = 0;
	m_exp = 0;
	m_maxExp = 0;
	m_addExp = 0;
	m_selectedUID = 0xffffffff;
	m_teamAniTime = 0;
	m_elapsedTime = 0;
	m_myRank = 0;
	m_bRankTooltip = false;
	m_bAwardTooltip = false;

	m_ctrl.m_pAngelFont = g_resrcmng->GetTitleFont();
	char* angelFontFiles[] = { "[font_angel.jpg" };
	m_ctrl.m_angelFontInfo.filename = angelFontFiles;
	m_ctrl.m_angelFontInfo.flag = 0;
	m_ctrl.m_angelFontInfo.fonth = 11;
	m_ctrl.m_angelFontInfo.fontw = 6;
	m_ctrl.m_angelFontInfo.numPages = 1;
	m_ctrl.m_angelFontInfo.texw = 128;
	m_ctrl.m_angelFontInfo.texh = 32;
	m_ctrl.m_angelFontInfo.pCharSet = "1234567890";
	m_bTikiReport = false;
	GetLocalTime(&m_tikiReportTime);
	m_tikiReportIndex = 0;
	m_ctrl.m_pAngelFont->Create(&m_ctrl.m_angelFontInfo);

	static char* s_uniteFontFiles[] = { "[font_wind.jpg" };
	memset(&m_ctrl.m_windFontInfo, 0, sizeof(m_ctrl.m_windFontInfo));
	m_ctrl.m_windFontInfo.filename = s_uniteFontFiles;
	m_ctrl.m_windFontInfo.fontw = 16;
	m_ctrl.m_windFontInfo.fonth = 16;
	m_ctrl.m_windFontInfo.numPages = 1;
	m_ctrl.m_windFontInfo.texw = 128;
	m_ctrl.m_windFontInfo.texh = 64;
	m_ctrl.m_windFontInfo.pCharSet = "1234567890ym%-.?/:,";
	m_ctrl.m_pWindFont = g_resrcmng->GetTitleFont();
	m_ctrl.m_pWindFont->Create(&m_ctrl.m_windFontInfo);
}

FrUniteResultDlg::~FrUniteResultDlg()
{
	if (g_resrcmng && m_ctrl.m_pWindFont)
	{
		g_resrcmng->Release(m_ctrl.m_pWindFont);
		m_ctrl.m_pWindFont = NULL;
	}
	if (g_resrcmng && m_ctrl.m_pAngelFont)
	{
		g_resrcmng->Release(m_ctrl.m_pAngelFont);
		m_ctrl.m_pAngelFont = NULL;
	}
	m_bTikiReport = false;
	m_tikiReportIndex = 0;
	if (USERINFODLG())
		CUserInfo::Instance()->Close();
	if (m_ctrl.m_pTreasureScore)
		m_ctrl.m_pTreasureScore->Close(false);
	if (m_ctrl.m_pTreasureGift)
		m_ctrl.m_pTreasureGift->Close(false);
	if (Doc())
	{
		if (Doc()->m_golfGame.gameType == 11 && Doc()->m_roomInfo.realGameType != 12 &&
			Doc()->m_roomInfo.realGameType != 14 && Doc()->m_roomInfo.realGameType != 19)
		{
			if (DOCJP()->GetState983() == 4)
			{
				CTaskManager::Instance()->PostMsg(NULL, "lobby", 0, (int)"OPPAGE", 0, 0, 0);
				CTaskManager::Instance()->PostMsg(NULL, "lobby", 0x8b, 4, 0, 0, 0);
			}
			S5::UTIL::SendExitRoom983(0);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
		}
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1 && DOCJP()->GetGPChannel983() == 0)
		{
			S5::UTIL::SendExitGPRoom983(0);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
		}
	}
}

bool FrUniteResultDlg::Close(bool bOK)
{
	return FrForm::Close(bOK);
}

bool FrUniteResultDlg::CheckChildDlgVisible()
{
	if (m_ctrl.m_pAwardInfoDlg)
	{
		if (m_ctrl.m_pAwardInfoDlg->IsVisible())
		{
			m_ctrl.m_pAwardInfoDlg->Close(true);
			return true;
		}
	}
	if (m_ctrl.m_pScoreDlg)
	{
		if (m_ctrl.m_pScoreDlg->IsVisible())
		{
			m_ctrl.m_pScoreDlg->Close(true);
			return true;
		}
	}
	if (m_ctrl.m_pTreasureScore || m_ctrl.m_pTreasureGift)
		return true;
	if (USERINFODLG())
	{
		CUserInfo::Instance()->Close();
		return true;
	}
	return false;
}

inline BOOL IsAdministrator(unsigned long identity = 0xffffffff)
{
	if (identity == 0xffffffff)
		identity = Doc()->m_myInfo.info.dwIdentity;
	BOOL bAdmin = (identity & 4) == 4;
	return bAdmin;
}

// HACK: the original inlines CGMToolkit::GetPrizeSwitch() (in-class in gmtoolkit.h); gmtoolkit.cpp
// defines it out of line and projectg (SaveScreenShot) is sensitive to gmtoolkit.h, so the
// member is read here through a using-declaration.
struct sGMToolkitPrize : public CGMToolkit
{
	using CGMToolkit::m_bPrizeSwitch;
};
#define GM_PRIZE_SWITCH() \
	(((sGMToolkitPrize*)CGMToolkit::Instance())->m_bPrizeSwitch)

// Open-coded string copy of the original (not strcpy: the copy loop is indexed).
#define COPY_TEXT(dstText, srcText) \
	{ \
		char* dst = dstText; \
		const char* src = srcText; \
		int i = 0; \
		char c; \
		do \
		{ \
			c = src[i]; \
			dst[i] = c; \
			i++; \
		} while (c); \
	}

static CLevelNameTable983 s_levelNames983; // 00e78cf0 (Get 008329d0)

// 983 00643fe0 (metodo do CSharedDoc: nivel+1 -> titulo, premio do nivel)
__declspec(noinline) std::string BuildLevelUpDesc983()
{
	unsigned char level = Doc()->m_myInfo.stat.Level;
	IFF983::sLevelUpPrizeItem* prize = ItemManager()->FindLevelUpPrizeItem983(level);
	std::ostringstream os;
	os << K2L_Compatibility(s_levelNames983.Get(level + 1).name)
	   << K2L_Compatibility("\xb7\xce ") << (prize ? prize->szNextPrize : "");
	return os.str();
}

bool FrUniteResultDlg::OnInit()
{
	LoadBitmapA();
	SetResultDlgType();
	SetDataToRankListBox();
	FindWinner();
	SetPrizeResult();
	if (IContentsDataContainer* pContainer =
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa))
		m_pHoleInOneData = pContainer;
	if (IsLocalContent((localContentType_t)10) && IsAdministrator() &&
		CGMToolkit::Instance() && GM_PRIZE_SWITCH())
		CGMToolkit::Instance()->SendPrize(m_ctrl.m_pRankList);
	unsigned char gt = Doc()->m_golfGame.gameType;
	if (gt == 4 || gt == 5 || gt == 11)
	{
	m_ctrl.m_pCancelBtn->Enable(false);
	DisableFlag(1);
	m_level = Doc()->m_myInfo.stat.Level;
	m_exp = Doc()->m_myInfo.stat.dwExp * 10;
	m_maxExp = Doc()->m_levelTable[m_level].exp * 10;
	m_addExp = Doc()->m_holeStatistics[0].dwExp * 10;
	if (gt != 11 && (unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
		m_exp -= m_addExp; // 983: Grand Prix por guilda desconta o exp do buraco
	m_bDone = m_addExp == 0;
	if (m_ctrl.m_pExpGaugeBar)
	{
		m_ctrl.m_pExpGaugeBar->SetBorder(1);
		m_ctrl.m_pExpGaugeBar->SetRange(0, m_maxExp, 0);
		m_ctrl.m_pExpGaugeBar->SetPos(m_exp);
	}
	if (gt != 11) // 983 (00aec210): o bloco do proximo nivel so nos tipos 4 e 5
	{
	const WRect& listRect = m_ctrl.m_pRankList->GetRect();
	float y = listRect.y;
	float h = listRect.h;
	WPoint pos;
	pos.x = m_ctrl.m_pNextLevelText->GetRect().x;
	pos.y = h + y + 100.0f;
	m_ctrl.m_pNextLevelText->MoveWindow(pos);
	m_ctrl.m_pNextLevelText->SetVisible(
		IsLocalContent((localContentType_t)0x2a /*983: era 0x29; medido*/));
	unsigned long totalExp =
		Doc()->m_holeStatistics[0].dwExp + Doc()->m_myInfo.stat.dwExp;
	if (totalExp > Doc()->m_levelTable[m_level].exp)
	{
		m_ctrl.m_pNextLevelText->SetVisible(false);
	}
	else
	{
		// 983 00643fe0: "<titulo do nivel+1>\xb7\xce <nome do premio de nivel>" (substitui a tabela s_levelUpText do 645)
		std::string text = BuildLevelUpDesc983();
		m_ctrl.m_pNextLevelText->AddLine(text.c_str(), 0, false);
	}
	}
	}
	// 983: o tipo 11 nao envia estatistica nem poe legenda
	if (gt == 11)
		return true;
	if (m_bTikiReport && IsLocalContent((localContentType_t)0x14))
	{
		Doc()->m_bSentHoleStat = true;
		if ((unsigned char)(Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
			return true;
		Doc()->Log30sScore();
	}
	else if (!Doc()->m_bSentHoleStat)
	{
		Doc()->m_bSentHoleStat = true;
		if ((unsigned char)(Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
			return true;
		WSendPacket packet((enumClientPacket)6);
		packet.EncodeBuffer(&Doc()->m_holeStatistics[0],
			sizeof(sPangYaUserStatistics));
		packet.Send(TO_GAME);
		Doc()->Log30sScore();
	}
	if (Doc()->m_roomInfo.realGameType == 12 || Doc()->m_roomInfo.realGameType == 14 ||
		Doc()->m_roomInfo.realGameType == 19)
		return true;
	const char* matchName;
	char report[256];
	if (m_bTikiReport && IsLocalContent((localContentType_t)0x14))
	{
		sprintf(report, "%d-%d-%d %d:%d",
			m_tikiReportTime.wYear, m_tikiReportTime.wMonth,
			m_tikiReportTime.wDay, m_tikiReportTime.wHour,
			m_tikiReportTime.wMinute);
		matchName = report;
	}
	else
	{
		switch (Doc()->m_roomInfo.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case GAME_TYPE_HOLEINONE983: // 983: 4, 5, 6, 9, 10, 11 e 18 (IsMassGame embutido)
		case GAME_TYPE_USEMAX:
			if (Doc()->m_roomInfo.gameType != 10)
			{
				IFF_STRUCT::sMatch* pMatch =
					ItemManager()->FindMatch(Doc()->m_roomInfo.tidMatch);
				if (pMatch)
				{
					const char* name = Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX
						? K2L_Compatibility("\274\305\307\303\304\332\275\272")
						: pMatch->Name;
					matchName = name;
				}
				else
				{
					unsigned char gameType = Doc()->m_roomInfo.gameType;
					if (gameType >= GAME_TYPE_MAX)
						matchName = NULL;
					else
						matchName = Doc()->m_gameTypeInfo[gameType].name;
				}
				break;
			}
		default:
		{
			unsigned char gameType = Doc()->m_roomInfo.gameType;
			if (gameType >= GAME_TYPE_MAX)
				matchName = NULL;
			else
				matchName = Doc()->m_gameTypeInfo[gameType].name;
			break;
		}
		}
	}
	char type[32];
	memset(type, 0, sizeof(type));
	switch (m_resultDlgType)
	{
	case 0:
		COPY_TEXT(type, K2L_Compatibility("\260\263\300\316\300\374"));
		break;
	case 1:
		COPY_TEXT(type, K2L_Compatibility("\306\300\300\374"));
		break;
	case 2:
		COPY_TEXT(type, K2L_Compatibility("\261\346\265\345\300\374"));
		break;
	}
	if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) < 2)
		SetCaption(MakeStr(K2L_Compatibility("")));
	else
		SetCaption(MakeStr(K2L_Compatibility("            (%s %s)"), matchName, type));
	if (0)
	{
		std::string caption;
	}
	return true;
}

void FrUniteResultDlg::OnProc(const float delta)
{
	float time = m_expTime + delta;
	m_bRankTooltip = false;
	m_bAwardTooltip = false;
	m_expTime = time;
	m_elapsedTime += delta;
	if (time < 1.0f)
		return;
	if (time - 1.0f < 0.01f)
		return;
	int step = (int)((time - 1.0f) * 100.0f);
	m_expTime = time - step * 0.01f;
	int nextExp = Doc()->m_levelTable[m_level].exp * 10;
	int addExp = m_addExp;
	if (step > (addExp > 0 ? addExp : -addExp))
	{
		m_bDone = true;
		step = addExp > 0 ? addExp : -addExp;
		m_exp += ((addExp > 0) * 2 - 1) * step;
		m_addExp = 0;
	}
	else
	{
		int add = ((addExp > 0) * 2 - 1) * step;
		m_exp += add;
		m_addExp = addExp - add;
	}
	if (m_exp < 0 && !m_bDone)
	{
		m_bDone = true;
		m_exp = 0;
	}
	if (m_exp >= nextExp)
	{
		m_exp -= nextExp;
		m_level++;
		m_bLevelUp = true;
		if (m_ctrl.m_pExpGaugeBar)
		{
			m_ctrl.m_pExpGaugeBar->SetRange(0,
				Doc()->m_levelTable[m_level].exp * 10, 0);
			m_ctrl.m_pExpGaugeBar->SetPos(m_exp);
		}
	}
	else if (m_ctrl.m_pExpGaugeBar)
	{
		m_ctrl.m_pExpGaugeBar->SetPos(m_exp);
	}
	m_teamAniTime += delta * 2.0f;
	if (m_elapsedTime >= 17.0f)
		m_ctrl.m_pRankList->UseRightButton(false);
	if (m_bDone == true)
	{
		if (!CTHunter::Instance()->IsReceiveGiftPacket() ||
			m_bTikiReport == true)
		{
			m_ctrl.m_pCancelBtn->Enable(true);
			EnableFlag(1);
		}
		else if (CTHunter::Instance()->IsReceiveGiftPacket() == true &&
			!Doc()->m_bTreasureResult)
		{
			if (!m_ctrl.m_pTreasureScore)
			{
				m_ctrl.m_pTreasureScore = CreateForm<FrTreasureScore>(
					g_pFresh->GetManager(), this, "trscore");
				m_ctrl.m_pTreasureScore->Open(
					(FRESH_PFN_RESULT)&FrUniteResultDlg::
						OnTreasureScoreDlgResult,
					0);
				m_ctrl.m_pTreasureScore->EnableDrag(false);
			}
			if (!m_ctrl.m_pTreasureGift)
			{
				m_ctrl.m_pTreasureGift = CreateForm<FrTreasureGift>(
					g_pFresh->GetManager(), this, "trgift");
				m_ctrl.m_pTreasureGift->MoveWindow(
					m_ctrl.m_pTreasureGift->GetCalcPoint());
				m_ctrl.m_pTreasureGift->Open(
					(FRESH_PFN_RESULT)&FrUniteResultDlg::
						OnTreasureGiftDlgResult,
					1);
				m_ctrl.m_pTreasureGift->EnableDrag(false);
			}
			if (m_ctrl.m_pTreasureScore && m_ctrl.m_pTreasureGift)
				Doc()->m_bTreasureResult = true;
		}
	}
}

void FrUniteResultDlg::LoadBitmapA()
{
	m_ctrl.m_pRankBack[0] = g_pFresh->GetBitmap(RB_STR_REDTEAM_RANK);
	m_ctrl.m_pRankBack[1] = g_pFresh->GetBitmap(RB_STR_BLUETEAM_RANK);
	m_ctrl.m_pRankBack[2] = g_pFresh->GetBitmap(RB_STR_30IN_RANK);
	m_ctrl.m_pLevelIcon = NULL;
	m_ctrl.m_pEmblem = NULL;
	m_ctrl.m_pPcBonus = NULL;
	m_ctrl.m_pAssistTitle = g_pFresh->GetBitmap("assist_title");
	m_ctrl.m_pPremium10 = g_pFresh->GetBitmap("premium+10");
	m_ctrl.m_pMascot02 = g_pFresh->GetBitmap("bonus_mascot_02");
	m_ctrl.m_pMascot01 = g_pFresh->GetBitmap("bonus_mascot_01");
	m_ctrl.m_pMascot09 = g_pFresh->GetBitmap("bonus_mascot_09");
	m_ctrl.m_pMascot10 = g_pFresh->GetBitmap("bonus_mascot_10");
	m_ctrl.m_pMascot11 = g_pFresh->GetBitmap("bonus_mascot_11");
	m_ctrl.m_pMascot12 = g_pFresh->GetBitmap("bonus_mascot_12");
	m_ctrl.m_pMascot39 = g_pFresh->GetBitmap("bonus_mascot_39");
	m_ctrl.m_pMascot40 = g_pFresh->GetBitmap("bonus_mascot_40");
	m_ctrl.m_pBonusMa = g_pFresh->GetBitmap("bonus_ma");
	m_ctrl.m_pBonusMa2 = g_pFresh->GetBitmap("bonus_ma2");
	m_ctrl.m_pMedal[0] = g_pFresh->GetBitmap("rank_gold");
	m_ctrl.m_pMedal[1] = g_pFresh->GetBitmap("rank_silver");
	m_ctrl.m_pMedal[2] = g_pFresh->GetBitmap("rank_bronze");
	m_ctrl.m_pMedal[3] = g_pFresh->GetBitmap("rank_noaward");
	m_pHoleInOneTrophy[0] = g_pFresh->GetBitmap("holeinone_trophy_gold");
	m_pHoleInOneTrophy[1] = g_pFresh->GetBitmap("holeinone_trophy_silver");
	m_pHoleInOneTrophy[2] = g_pFresh->GetBitmap("holeinone_trophy_bronze");
	m_ctrl.m_pPrizeGray[0] = g_pFresh->GetBitmap("prize_goodluck_gray");
	m_ctrl.m_pPrizeGray[1] = g_pFresh->GetBitmap("prize_speeder_gray");
	m_ctrl.m_pPrizeGray[2] = g_pFresh->GetBitmap("prize_runner_gray");
	m_ctrl.m_pPrizeGray[3] = g_pFresh->GetBitmap("prize_chipin_gray");
	m_ctrl.m_pPrizeGray[4] = g_pFresh->GetBitmap("prize_long_putting_gray");
	m_ctrl.m_pPrizeGray[5] = g_pFresh->GetBitmap("prize_recovery_gray");
	m_ctrl.m_pPrize[0] = g_pFresh->GetBitmap("prize_goodluck");
	m_ctrl.m_pPrize[1] = g_pFresh->GetBitmap("prize_speeder");
	m_ctrl.m_pPrize[2] = g_pFresh->GetBitmap("prize_runner");
	m_ctrl.m_pPrize[3] = g_pFresh->GetBitmap("prize_chipin");
	m_ctrl.m_pPrize[4] = g_pFresh->GetBitmap("prize_long_putting");
	m_ctrl.m_pPrize[5] = g_pFresh->GetBitmap("prize_recovery");
	m_ctrl.m_pIconZero = g_pFresh->GetBitmap("icon_zero");
	m_ctrl.m_pTooltip[0] = g_pFresh->GetBitmap("tooltip_goodluck");
	m_ctrl.m_pTooltip[1] = g_pFresh->GetBitmap("tooltip_speeder");
	m_ctrl.m_pTooltip[2] = g_pFresh->GetBitmap("tooltip_runner");
	m_ctrl.m_pTooltip[3] = g_pFresh->GetBitmap("tooltip_chipin");
	m_ctrl.m_pTooltip[4] = g_pFresh->GetBitmap("tooltip_longputter");
	m_ctrl.m_pTooltip[5] = g_pFresh->GetBitmap("tooltip_recovery");
	m_ctrl.m_pTooltip[6] = g_pFresh->GetBitmap("tooltip_winning_contents");
}

// 983 00ae70e0: mesmo jogador? (com o flag +0x1b4 do outro, por apelido; senao por oid)
bool FrUniteResultDlg::CanShowPremium10983(const sRivalData* pRival)
{
	if (!m_ctrl.m_pPremium10)
		return false;
	if (pRival->bGrandPrixAI983 != 0)
		return false;
	return ((pRival->capability >> 14) & 1) != 0;
}

bool FrUniteResultDlg::IsSameRival(const sRivalData* a, const sRivalData* b)
{
	C_ASSERT_VC10(offsetof(sRivalData, bGrandPrixAI983) == 0x1b4);
	if (b->bGrandPrixAI983)
		return strncmp(a->nickname, b->nickname, 22) == 0;
	return a->oid == b->oid;
}

// 983 006335c0 (CSharedDoc, sem par no nosso shareddoc.cpp): indice de medalha pela contagem e posicao ajustada
static __forceinline int MedalByCountRank(int count, int rank)
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

int FrUniteResultDlg::GetMedalIndex(const sRivalData* pRival)
{
	bool bGuild = (unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1;
	std::vector<sRivalData>::iterator it;
	if (Doc()->m_golfGame.gameType == 11)
	{
		if (Doc()->m_rivalList.size() == 0)
			return 3;
		for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
		{
			if (bGuild ? IsSameRival(&*it, pRival) : it->oid == pRival->oid)
				return it->rank - 1;
		}
		return 3;
	}
	unsigned char members = 0;
	if (IsLocalContent((localContentType_t)47 /*983: era 46; interpolado*/))
		members = CIntrusion::Instance()->GetCalcExpMemberSize();
	if (members == 0)
	{
		if (Doc()->m_rivalList.size() < 10)
			return 3;
	}
	else if (members < 10)
		return 3;
	unsigned char idx = 0xff;
	unsigned char i = 0;
	for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it, ++i)
	{
		if (bGuild ? IsSameRival(&*it, pRival) : it->oid == pRival->oid)
		{
			idx = i;
			break;
		}
	}
	if (idx != 0xff && *((unsigned char*)&Doc()->m_rivalList[idx] + 0x170) <= 3)
	{
	}
	else if (!m_bTikiReport)
		return 3;
	if (IsLocalContent((localContentType_t)20) && m_bTikiReport)
	{
		for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
		{
			if (bGuild ? IsSameRival(&*it, pRival) : it->oid == pRival->oid)
				return *((unsigned char*)&*it + 0x80) - 1;
		}
	}
	const sRivalData* base = &Doc()->m_rivalList[0];
	unsigned char rank = base[idx].rank;
	unsigned char count = 0;
	unsigned char ahead = 0;
	if (members != 0)
	{
		count = members;
		for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
		{
			if (it->state != 3 && it->rank < rank && *((unsigned char*)&*it + 0x170) > 3)
				++ahead;
		}
	}
	else
	{
		for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
		{
			if (it->state == 3)
			{
				bool skip = false;
				unsigned char hole = *((unsigned char*)&*it + 0x151);
				for (unsigned char a = 0; a < 0x12; ++a)
				{
					if (Doc()->m_holeOrder[a] == hole) // B1450: ex-cru Doc()+0x1051e+a
					{
						skip = a + 1 < 4;
						break;
					}
				}
				if (skip)
					continue;
			}
			++count;
			if (it->state != 3 && it->rank < rank && *((unsigned char*)&*it + 0x170) > 3)
				++ahead;
		}
	}
	return MedalByCountRank(count, (unsigned char)(rank - ahead));
}

void FrUniteResultDlg::SetResultDlgType()
{
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 11:
		m_resultDlgType = 0;
		break;
	case 5:
		m_resultDlgType = 1;
		break;
	case 6:
		m_resultDlgType = 2;
		break;
	}
}

void FrUniteResultDlg::SetTikiReportState(_SYSTEMTIME& time)
{
	if (!IsLocalContent((localContentType_t)0x14))
		return;
	m_bTikiReport = true;
	m_tikiReportTime = time;
	m_tikiReportIndex = 0;
	int index = 0;
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it, ++index)
	{
		if (it->uid == Doc()->m_myInfo.info.dwUID)
		{
			m_tikiReportIndex = index;
			break;
		}
	}
}

// 983 00636d00 (CSharedDoc, sem par no nosso shareddoc.cpp): rank pelo score e, no empate, pelo pang
static __declspec(noinline) void CalcRivalRank()
{
	if (Doc()->m_golfGame.gameType == 11)
		return;
	std::vector<sRivalData>::iterator it;
	for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
	{
		if (it->state == 3)
		{
			it->rank = 0xff;
			continue;
		}
		it->rank = 1;
		for (std::vector<sRivalData>::iterator other = Doc()->m_rivalList.begin();
			other != Doc()->m_rivalList.end(); ++other)
		{
			if (it == other || other->state == 3)
				continue;
			if (it->totalScore > other->totalScore)
				it->rank++;
			else if (it->totalScore == other->totalScore)
			{
				if (it->totalPang < other->totalPang)
					it->rank++;
			}
		}
	}
}

void FrUniteResultDlg::SetDataToRankListBox()
{
	if (Doc()->m_golfGame.gameType != 11)
	{
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
			Doc()->CalcRivalRank983();
		else
			CalcRivalRank();
	}
	std::vector<sRivalData>::iterator it;
	for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
	{
		if (Doc()->m_golfGame.gameType != 11 || it->state == 2)
			m_ctrl.m_pRankList->AddItem(&*it);
	}
	if (m_resultDlgType == 2)
		m_ctrl.m_pRankList->SortItem(UniteGuildIndexCompare);
	else
	{
		// 983: testa CTask+0x1fc (009b7770) antes do IsCurrentActor
		if (AfxGetTask()->GetMainActor() &&
			AfxGetTask()->IsCurrentActor983("realmyroom"))
			m_ctrl.m_pRankList->SortItem(ReportRankCompare);
		else
			m_ctrl.m_pRankList->SortItem(UniteRankCompare);
		if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
			m_ctrl.m_pRankList->SortItem(UniteRankOnlyCompare);
	}
	if (Doc()->m_roomInfo.gameType != 11 && Doc()->m_roomInfo.bAdminMaster && // B1450: ex-cru Doc()+0x1095e
		Doc()->m_roomInfo.nUserLimit >= 100)
		SaveGMEvent();
}

bool FrUniteResultDlg::SetTooltip(WRect rect, int index)
{
	if (index >= 0 && index <= 6)
	{
		if (rect.IsInRect(
				WPoint(g_input->GetMousePoint().x, g_input->GetMousePoint().y)))
		{
			m_ctrl.m_pCurTooltip = m_ctrl.m_pTooltip[index];
			CMouseCursor::Instance()->SetCursor(1);
			return true;
		}
		if (!m_bRankTooltip && !m_bAwardTooltip)
			m_ctrl.m_pCurTooltip = NULL;
	}
	return false;
}

void FrUniteResultDlg::FindWinner()
{
	if (Doc()->m_roomInfo.realGameType == 12 || Doc()->m_roomInfo.realGameType == 14)
	{
		m_winner = Doc()->m_rivalList[m_tikiReportIndex];
		return;
	}
	memset(Doc()->m_teamResult, 0, sizeof(Doc()->m_teamResult));
	for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
		it != Doc()->m_rivalList.end(); ++it)
	{
		if (it->rank == 1)
		{
			if (Doc()->m_golfGame.gameType != 11 || it->state == 2)
				m_winner = *it;
			if (Doc()->m_roomInfo.gameType == 11)
				return;
			if (Doc()->m_roomInfo.gameType == 4)
				return;
			if (Doc()->m_roomInfo.gameType == 10)
				return;
		}
		switch (m_resultDlgType)
		{
		case 2:
			Doc()->m_teamResult[it->team].score += it->guildPoint;
			Doc()->m_teamResult[it->team].pang += it->totalPang;
			break;
		case 1:
			Doc()->m_teamResult[it->team].score += it->rank;
			Doc()->m_teamResult[it->team].pang += it->totalPang;
			break;
		}
	}
}

bool FrUniteResultDlg::OnAwardInfoDlgResult(int result, FrForm* form)
{
	m_ctrl.m_pAwardInfoDlg = NULL;
	return true;
}

bool FrUniteResultDlg::OnScoreDlgResult(int result, FrForm* form)
{
	m_ctrl.m_pScoreDlg = NULL;
	return true;
}

bool FrUniteResultDlg::OnTreasureScoreDlgResult(int result, FrForm* form)
{
	m_ctrl.m_pTreasureScore = NULL;
	return true;
}

bool FrUniteResultDlg::OnTreasureGiftDlgResult(int result, FrForm* form)
{
	g_input->ExclusiveGetDownUseDone("ESCAPE");
	m_ctrl.m_pTreasureGift = NULL;
	if (result)
	{
		if (m_ctrl.m_pTreasureScore)
			m_ctrl.m_pTreasureScore->Close(true);
		if (m_ctrl.m_pCancelBtn)
			m_ctrl.m_pCancelBtn->Enable(true);
		EnableFlag(1);
	}
	return true;
}

void FrUniteResultDlg::SetPrizeResult()
{
	m_prizeMap.clear();
	std::vector<unsigned long>& items = Doc()->m_acquireItemList[0];
	for (std::vector<unsigned long>::iterator it = items.begin();
		it != items.end(); ++it)
	{
		unsigned long typeId = *it;
		std::map<unsigned long, int>::iterator found = m_prizeMap.find(typeId);
		if (found == m_prizeMap.end())
			m_prizeMap.insert(std::pair<const unsigned long, int>(typeId, 1));
		else
			(*found).second++;
	}
}

bool FrUniteResultDlg::SaveGMEvent()
{
	mkdir("log");
	mkdir("log/event");
	SYSTEMTIME time;
	GetLocalTime(&time);
	FILE* fp =
		fopen(MakeStr("log/event/Result_%04d_%02d_%02d_%02d_%02d.txt",
				  time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute),
			"wt");
	if (!fp)
		return false;

	char buf[512] = "";
	sprintf(buf, "Room Info : %s > %s > %s(%d)\n", Doc()->m_curGameServer.name,
		Doc()->m_curChannel.Name, Doc()->m_roomInfo.title,
		Doc()->m_roomInfo.roomGuid);
	fwrite(buf, strlen(buf), 1, fp);
	sprintf(buf, "Rank : uid\tnick\t\thole\tscore\tgamepang\tbonuspang\n");
	fwrite(buf, strlen(buf), 1, fp);
	sprintf(buf,
		"------------------------------------------------------------\n");
	fwrite(buf, strlen(buf), 1, fp);

	char hole[32] = "";
	int rank = 1;
	FrListBox::ITEM_LIST::iterator it = m_ctrl.m_pRankList->m_itemList.begin();
	for (; it != m_ctrl.m_pRankList->m_itemList.end(); ++it)
	{
		sRivalData* pRival = (sRivalData*)(*it)->pData;
		if (pRival->state == 2 || GetHoleIndex(pRival->hole) == 19)
			sprintf(hole, "End");
		else if (pRival->state == 3)
			sprintf(hole, "Out");
		else
			sprintf(hole, "%d", GetHoleIndex(pRival->hole));
		sprintf(buf, "%03d : %d\t%s\t%s\t%d\t%I64d\t%I64d\n", rank++,
			pRival->uid, pRival->nickname, hole, pRival->totalScore,
			pRival->totalPang, pRival->totalBonusPang);
		fwrite(buf, strlen(buf), 1, fp);
	}
	sprintf(buf,
		"------------------------------------------------------------\n");
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
	return true;
}

IMPLEMENT_OBJECT(FrAwardInfoDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrAwardInfoDlg, FrForm)
END_FRESH_MSGMAP()

FrAwardInfoDlg::FrAwardInfoDlg()
{
	m_elapsedTime = 0;
}

FrAwardInfoDlg::~FrAwardInfoDlg()
{
}

bool FrAwardInfoDlg::OnInit()
{
	SetCaption(K2L_Compatibility("\xbc\xf6\xbb\xf3\xb3\xbb\xbf\xeb")); // 983 00ae71d0
	return true;
}

void FrAwardInfoDlg::OnProc(const float delta)
{
	m_elapsedTime += delta;
}
