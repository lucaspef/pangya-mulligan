#include "minatl.h"
#include "birthdayeventdlg.h"
#include "frbutton.h"
#include "frarea.h"
#include "frviewer.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "wview.h"
#include "simpleoverlay.h"
#include "gamedatadb.h"
#include "gdbitmapbox.h"
#include "gdtextbox.h"
#include "taskmanager.h"
#include "actor.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "../shared/localize.h"

extern Fresh* g_pFresh;

namespace BIRTHDAYEVENT
{
	int board5_5[10][25] = {
		{ 13, 2,  10, 25, 1,  12, 8,  22, 5,  16, 19, 6,  15, 14, 24, 17, 7,  4,  21,
         18,																			 9,  20, 23, 11, 3  },
		{ 7,  16, 18, 10, 21, 14, 1,  20, 2,  9,  19, 3,  6,  13, 25, 8,  5,  17, 23,
         12,																			 15, 11, 24, 22, 4  },
		{ 12, 3,  16, 9,  1,  4,  5,  2,  21, 10, 23, 6,  18, 14, 22, 20, 24, 15, 13,
         17,																			 7,  8,  19, 11, 25 },
		{ 6,  2,  21, 3,  17, 22, 20, 19, 8,  10, 14, 5,  13, 25, 1,  7,  23, 11, 18,
         16,																			 24, 9,  12, 15, 4  },
		{ 20, 17, 12, 9,  10, 25, 1,  19, 14, 15, 16, 3,  4,  7,  11, 18, 13, 21, 23,
         22,																			 6,  8,  2,  5,  24 },
		{ 23, 7,  3,  21, 11, 2,  4,  22, 24, 5,  19, 17, 10, 8,  18, 14, 15, 20, 6,
         16,																			 12, 13, 25, 1,  9  },
		{ 23, 21, 3,  4,  16, 2,  15, 24, 11, 10, 13, 18, 17, 6,  22, 5,  9,  8,  7,  1,
         25,																				 14, 12, 20, 19 },
		{ 1,  16, 15, 4,  24, 5,  2,  12, 20, 22, 9,  13, 8,  23, 21, 17, 11, 3,  19,
         18,																			 7,  14, 6,  25, 10 },
		{ 5,  20, 10, 4,  7,  8,  23, 17, 24, 22, 3,  13, 12, 25, 14, 16, 9,  19, 6,
         11,																			 18, 21, 15, 1,  2  },
		{ 25, 15, 17, 7,  18, 19, 13, 20, 22, 11, 1,  14, 21, 5,  16, 9,  4,  2,  8,
         24,																			 10, 6,  12, 3,  23 }
	};
	int board4_4[10][16] = {
		{ 14, 7,  5,  10, 2,  12, 9,  11, 13, 6,  8,  15, 16, 3,  4,  1  },
		{ 3,  14, 12, 1,  15, 6,  13, 11, 4,  7,  8,  2,  5,  10, 16, 9  },
		{ 16, 8,  10, 5,  2,  14, 4,  1,  3,  12, 13, 7,  15, 9,  6,  11 },
		{ 16, 1,  4,  15, 2,  14, 13, 7,  11, 12, 5,  10, 8,  6,  9,  3  },
		{ 13, 11, 6,  5,  16, 2,  4,  8,  12, 1,  7,  15, 14, 9,  3,  10 },
		{ 13, 2,  10, 3,  1,  12, 8,  4,  5,  16, 9,  6,  15, 14, 11, 7  },
		{ 6,  13, 7,  10, 8,  9,  11, 16, 15, 2,  12, 5,  14, 3,  1,  4  },
		{ 3,  9,  1,  15, 14, 11, 4,  6,  13, 16, 10, 12, 2,  7,  8,  5  },
		{ 13, 9,  2,  5,  15, 4,  11, 12, 1,  7,  16, 14, 3,  10, 8,  6  },
		{ 14, 1,  16, 3,  10, 9,  7,  6,  2,  15, 11, 5,  12, 8,  4,  13 }
	};
	int board3_3[10][9] = {
		{ 9, 2, 7, 3, 1, 6, 8, 4, 5 },
        { 5, 8, 3, 1, 2, 9, 7, 6, 4 },
		{ 9, 5, 7, 6, 2, 4, 8, 3, 1 },
        { 9, 7, 6, 3, 4, 1, 5, 8, 2 },
		{ 7, 4, 5, 9, 1, 6, 3, 2, 8 },
        { 3, 2, 7, 6, 1, 8, 4, 9, 5 },
		{ 1, 3, 2, 4, 7, 8, 6, 9, 5 },
        { 3, 4, 1, 5, 7, 2, 9, 6, 8 },
		{ 9, 3, 5, 7, 6, 1, 8, 2, 4 },
        { 7, 8, 9, 2, 3, 5, 6, 1, 4 }
	};
	int ns_valueTable[25] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
		4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576,
		2097152, 4194304, 8388608, 16777216 };
} // namespace BIRTHDAYEVENT


IMPLEMENT_OBJECT(FrBirthdayEventDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBirthdayEventDlg, FrForm)
ON_FRESH_VV("Help", FRCMD_LBUTTONDOWN,
	FrBirthdayEventDlg::OnLButtonDownHelpButton)
ON_FRESH_VI("Gift", FRCMD_INIT, FrBirthdayEventDlg::OnInitGiftButton)
ON_FRESH_VV("Gift", FRCMD_LBUTTONDOWN,
	FrBirthdayEventDlg::OnLButtonDownGiftButton)
ON_FRESH_VI("board", FRCMD_OWNERDRAW, FrBirthdayEventDlg::OnOwnerDrawBoardArea)
ON_FRESH_VI("boardNumber", FRCMD_INIT,
	FrBirthdayEventDlg::OnInitBoardNumberArea)
ON_FRESH_VI("BirthdayEvent_bg1", FRCMD_INIT,
	FrBirthdayEventDlg::OnInitBoardBG1Area)
END_FRESH_MSGMAP()

FrBirthdayEventDlg::FrBirthdayEventDlg()
{
	m_boardIndex = Doc()->m_myInfo.info.dwUID % 10;
	m_clickOffset = 15.0f;
}

FrBirthdayEventDlg::~FrBirthdayEventDlg()
{
	if (m_pGiftNumberUI)
	{
		delete m_pGiftNumberUI;
		m_pGiftNumberUI = NULL;
	}
	if (m_pBoardUI)
	{
		delete m_pBoardUI;
		m_pBoardUI = NULL;
	}
}

bool FrBirthdayEventDlg::OnInit()
{
	std::string name = m_eventName + "BirthdayEvent";
	m_pTextBox = (CGDTextBox*)CGameDataDB::Instance()->GetGameData(
		GAMEDATA_TEXTBOX, name.c_str());
	m_pBitmapBox = (CGDBitmapBox*)CGameDataDB::Instance()->GetGameData(
		GAMEDATA_BITMAPBOX, name.c_str());
	InitalizeCoordinates();
	InitalizeImages();
	return FrForm::OnInit();
}

void FrBirthdayEventDlg::InitalizeCoordinates()
{
	m_pBitmapBox->GetRect("BoardBG", m_board[0].boardRect);
	m_pBitmapBox->GetRect("GiftNumber", m_giftNumberRect);
	m_pBitmapBox->GetRect("Date", m_dateRect);
	m_pBitmapBox->GetRect("Click", m_clickRect);
}

void FrBirthdayEventDlg::InitalizeImages()
{
	m_pBoardUI = new CSimpleUI(m_pBitmapBox->GetFileName("BoardBG"));
	m_pGiftNumberUI = new CSimpleUI(m_pBitmapBox->GetFileName("GiftNumber"));
	int n = 1;
	BoardInfo* b = m_board;
	for (int row = 0; n < 17; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			WRect rect((m_board[0].boardRect.w + 1.0f) * (float)col,
				(m_board[0].boardRect.h + 1.0f) * (float)row,
				m_board[0].boardRect.w, m_board[0].boardRect.h);
			BoardInfo& board = *b;
			board.boardRect.x = (m_board[0].boardRect.w + 1.0f) * (float)col +
				m_board[0].boardRect.x;
			board.boardRect.y = (m_board[0].boardRect.h + 1.0f) * (float)row +
				m_board[0].boardRect.y;
			board.boardRect.w = m_board[0].boardRect.w;
			board.boardRect.h = m_board[0].boardRect.h;
			m_pBoardUI->Add(MakeStr("BoardBG_%d", n), rect, true);
			board.pNumberBmp =
				g_pFresh->RegisterBitmap(MakeStr("event_kooh2013_num2_%d", n));
			board.numberRect.x =
				(m_board[0].boardRect.w + 1.0f) * (float)col + 91.0f;
			board.numberRect.w = 37.0f;
			board.numberRect.h = 30.0f;
			board.numberRect.y =
				(m_board[0].boardRect.h + 1.0f) * (float)row + 161.0f;
			n++;
			b++;
		}
	}
	m_pDateBmp = g_pFresh->RegisterBitmap(m_pBitmapBox->GetFileName("Date"));
	m_pClickBmp = g_pFresh->RegisterBitmap(m_pBitmapBox->GetFileName("Click"));
	WRect rect(0.0f, 0.0f, m_giftNumberRect.w, m_giftNumberRect.h);
	for (int i = 0; i < 10; i++)
	{
		rect.x = (float)i * m_giftNumberRect.w;
		m_pGiftNumberUI->Add(MakeStr("GiftNumber_%d", i), rect, true);
	}
	m_pBoardBG1Area->SetBgImg(m_pBitmapBox->GetFileName("BoardBG1"));
}


bool FrBirthdayEventDlg::OnKoohBirthdayEventHelpDlg(int result, FrForm* form)
{
	if (!IsLocalContent((localContentType_t)0xa2 /*983: era 0x8e; medido 00b12450*/))
		return false;
	m_pHelpDlg = NULL;
	m_pBoardNumberArea->SetVisible(true);
	return true;
}

void FrBirthdayEventDlg::SetMissionData(unsigned long completeMask,
	unsigned long remainGiftCount)
{
	for (int i = 2; i < 18; i += 4)
	{
		if (BIRTHDAYEVENT::ns_valueTable
				[BIRTHDAYEVENT::board4_4[m_boardIndex][i - 2] - 1] &
			completeMask)
			m_mission[i - 2].bComplete = 1;
		if (BIRTHDAYEVENT::ns_valueTable
				[BIRTHDAYEVENT::board4_4[m_boardIndex][i - 1] - 1] &
			completeMask)
			m_mission[i - 1].bComplete = 1;
		if (BIRTHDAYEVENT::ns_valueTable
				[BIRTHDAYEVENT::board4_4[m_boardIndex][i] - 1] &
			completeMask)
			m_mission[i].bComplete = 1;
		if (BIRTHDAYEVENT::ns_valueTable
				[BIRTHDAYEVENT::board4_4[m_boardIndex][i + 1] - 1] &
			completeMask)
			m_mission[i + 1].bComplete = 1;
	}
	m_remainGiftCount = remainGiftCount;
	if (m_pGiftBtn)
	{
		bool bEnable = remainGiftCount > 0;
		m_pGiftBtn->SetVisible(bEnable);
		m_pGiftBtn->Enable(bEnable);
	}
	m_giftCount = div(remainGiftCount, 10);
}

void FrBirthdayEventDlg::SetRemainGiftCount(unsigned long count)
{
	m_remainGiftCount = count;
	if (m_pGiftBtn)
	{
		bool bEnable = count > 0;
		m_pGiftBtn->SetVisible(bEnable);
		m_pGiftBtn->Enable(bEnable);
	}
	m_giftCount = div(count, 10);
}

void FrBirthdayEventDlg::SetMissionInfo(std::vector<unsigned long>& missions,
	std::vector<unsigned long>& values)
{
	for (unsigned int i = 0; i < missions.size(); i++)
		m_mission[missions[i] - 1].value = values[i];
}

void FrBirthdayEventDlg::OnMouseMove(const WPoint& pos)
{
	CheckToolTip(pos);
}

void FrBirthdayEventDlg::OnProc(const float dt)
{
	static bool s_bClickReturn = true;
	static float s_clickSpeed;
	if (m_remainGiftCount > 0)
	{
		float offset = m_clickOffset;
		if (s_bClickReturn)
		{
			if (offset <= 15.0f)
			{
				m_clickOffset = m_clickOffset - s_clickSpeed * 27.0f * dt;
				if (s_clickSpeed < 6.0f)
					s_clickSpeed = s_clickSpeed + 1.0f;
			}
			if (m_clickOffset <= 0.0)
			{
				m_clickOffset = 0.0f;
				s_bClickReturn = false;
				s_clickSpeed = 6.0f;
				return;
			}
		}
		else
		{
			if (0.0f <= offset)
			{
				m_clickOffset = s_clickSpeed * 27.0f * dt + m_clickOffset;
				if (1.0f < s_clickSpeed)
					s_clickSpeed = s_clickSpeed - 1.0f;
			}
			if (15.0f <= m_clickOffset)
			{
				m_clickOffset = 15.0f;
				s_bClickReturn = true;
				s_clickSpeed = 1.0f;
			}
		}
	}
}

void FrBirthdayEventDlg::CheckToolTip(const WPoint& pos)
{
	WRect rect;
	rect.w = m_board[0].boardRect.w;
	rect.h = m_board[0].boardRect.h;
	for (int row = 0; row < 4; row++)
	{
		for (int col = 0; col < 4; col++)
		{
			rect.x = (float)col * m_board[0].boardRect.w +
				m_board[0].boardRect.x + m_rect.x;
			rect.y = (float)row * m_board[0].boardRect.h +
				m_board[0].boardRect.y + m_rect.y;
			if (rect.x <= pos.x && pos.x <= rect.w + rect.x &&
				rect.y <= pos.y && pos.y <= rect.h + rect.y)
			{
				m_pBoardNumberArea->SetRect(rect);
				MakeToolTipText(
					BIRTHDAYEVENT::board4_4[m_boardIndex][row * 4 + col]);
			}
		}
	}
}

void FrBirthdayEventDlg::MakeToolTipText(int mission)
{
	std::string key = MakeStr("MissionText%d", mission);
	std::string text = m_pTextBox->GetText(key);
	int kind = 0;
	if (text.find("%d", 0) == std::string::npos)
	{
		if (text.find("%s", 0) != std::string::npos)
			kind = 2;
	}
	else
		kind = 1;
	char buf[512];
	switch (kind)
	{
	case 1:
		m_pBoardNumberArea->SetToolTipText(
			std::string(MakeStr(text.c_str(), m_mission[mission - 1].value)));
		break;
	case 2:
		m_pBoardNumberArea->SetToolTipText(std::string(
			MakeStr(text.c_str(), MakeMapInfoText(mission - 1, buf))));
		break;
	default:
		m_pBoardNumberArea->SetToolTipText(text);
	}
}


char* FrBirthdayEventDlg::MakeMapInfoText(int mission, char* buf)
{
	memset(buf, 0, 512);
	int len = 1;
	for (int i = 0; i < 20; i++)
	{
		CMapTypeAdapter::eMapFlag flag =
			CMapTypeAdapter::GetMapFlag((CMapTypeAdapter::eMapIndex)i);
		if (m_mission[mission].value & flag)
		{
			IFF_STRUCT::sDesc* desc = ItemManager()->FindDesc(i | 0x28000000);
			if (desc)
			{
				wsprintf(buf + len, "%s,\n", desc->Desc,
					(int)strlen(desc->Desc));
				len = len + strlen(desc->Desc) + 2;
			}
		}
	}
	if (len == 0)
	{
		wsprintf(buf, "%s",
			K2L_Compatibility("\xbf\xcf\xc1\xd6\xc7\xd1 \xc4\xda\xbd\xba \xbe\xf8\xc0\xbd"), 16);
		return buf;
	}
	*buf = '\n';
	wsprintf(buf + (len - 1), "\n%s", K2L_Compatibility("\xbf\xcf\xc1\xd6"), 4);
	return buf;
}

void FrBirthdayEventDlg::SetEventName(std::string& name)
{
	m_eventName = name;
}

void FrBirthdayEventDlg::OnLButtonDownHelpButton()
{
	m_pHelpDlg = CreateForm<FrBirthdayEventHelpDlg>(g_pFresh->GetManager(),
		this, "KoohBirthdayEventHelp", NULL);
	if (m_pHelpDlg)
	{
		m_pHelpDlg->Open(
			(FRESH_PFN_RESULT)&FrBirthdayEventDlg::OnKoohBirthdayEventHelpDlg,
			0x11);
		m_pHelpDlg->OpenHelpDlg(m_pBitmapBox->GetFileName("Explanation"));
	}
	m_pBoardNumberArea->SetVisible(false);
}

void FrBirthdayEventDlg::OnInitGiftButton(int param)
{
	m_pGiftBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBirthdayEventDlg::OnLButtonDownGiftButton()
{
	if (m_giftCount.rem + m_giftCount.quot * 10 <= 0)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xbc\xb1\xb9\xb0 \xc1\xf6\xb1\xde \xc8\xbd\xbc\xf6\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
			0, 0, 0, 0));
		return;
	}
	WSendPacket packet(0x15b);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
}


void FrBirthdayEventDlg::OnOwnerDrawBoardArea(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	for (int i = 0; i < 16;)
	{
		for (int col = 0; col < 4; col++)
		{
			if (m_mission[i].bComplete)
			{
				m_pBoardUI->Render(MakeStr("BoardBG_%d", i + 1),
					m_board[i].boardRect.x + m_rect.x,
					m_board[i].boardRect.y + m_rect.y, 1.0f, 0);
			}
			else
			{
				WRect rect(m_board[i].numberRect.x + m_rect.x,
					m_board[i].numberRect.y + m_rect.y, m_board[i].numberRect.w,
					m_board[i].numberRect.h);
				gdi->DrawTexture(
					m_board[BIRTHDAYEVENT::board4_4[m_boardIndex][i] - 1]
						.pNumberBmp,
					rect, 0xffffffff, 0);
			}
			i++;
		}
	}
	WRect click(0.0f, 0.0f, m_clickRect.w, m_clickRect.h);
	if (m_remainGiftCount != 0)
	{
		if (m_giftCount.quot != 0)
			m_pGiftNumberUI->Render(MakeStr("GiftNumber_%d", m_giftCount.quot),
				m_giftNumberRect.x, m_giftNumberRect.y, 1.0f, 0x22);
		click.x = m_clickRect.x + m_clickOffset + m_rect.x;
		click.y = m_clickRect.y + m_rect.y;
		gdi->DrawTexture(m_pClickBmp, click, 0xffffffff, 0);
	}
	m_pGiftNumberUI->Render(MakeStr("GiftNumber_%d", m_giftCount.rem),
		m_giftNumberRect.w + m_giftNumberRect.x, m_giftNumberRect.y, 1.0f,
		0x22);
}

void FrBirthdayEventDlg::OnInitBoardNumberArea(int param)
{
	m_pBoardNumberArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrBirthdayEventDlg::OnInitBoardBG1Area(int param)
{
	m_pBoardBG1Area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

IMPLEMENT_OBJECT(FrBirthdayEventHelpDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBirthdayEventHelpDlg, FrForm)
ON_FRESH_VI("HelpView", FRCMD_INIT, FrBirthdayEventHelpDlg::OnInitHelpView)
END_FRESH_MSGMAP()

FrBirthdayEventHelpDlg::FrBirthdayEventHelpDlg()
{
}

FrBirthdayEventHelpDlg::~FrBirthdayEventHelpDlg()
{
}

void FrBirthdayEventHelpDlg::OnInitHelpView(int param)
{
	m_pHelpView = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void FrBirthdayEventHelpDlg::OpenHelpDlg(const char* filename)
{
	if (m_pHelpView)
		m_pHelpView->Open(filename);
}

IMPLEMENT_OBJECT(FrBirthdayEventMissionDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBirthdayEventMissionDlg, FrForm)
ON_FRESH_VI("MissionNumber", FRCMD_INIT,
	FrBirthdayEventMissionDlg::OnInitMissionNumberArea)
END_FRESH_MSGMAP()

FrBirthdayEventMissionDlg::FrBirthdayEventMissionDlg()
{
	m_completedMission = 0;
	m_fadeOutTime = 0.0f;
	m_bFadeOut = false;
	for (int i = 0; i < 16; i++)
		m_pMissionNumberBmp[i] =
			g_pFresh->RegisterBitmap(MakeStr("event_kooh2013_num2_%d", i + 1));
}

FrBirthdayEventMissionDlg::~FrBirthdayEventMissionDlg()
{
}

bool FrBirthdayEventMissionDlg::OnInit()
{
	return FrForm::OnInit();
}

void FrBirthdayEventMissionDlg::OnProc(const float dt)
{
	if (m_bFadeOut)
	{
		m_fadeOutTime = dt + m_fadeOutTime;
		if (m_fadeOutTime > 3.0f)
		{
			SetFadeout(true);
			m_fadeOutTime = 0.0f;
			m_bFadeOut = false;
		}
	}
}

void FrBirthdayEventMissionDlg::SetCompletedMissionInfo(unsigned long mission)
{
	m_completedMission = mission;
	m_pMissionNumberArea->SetBgImg(m_pMissionNumberBmp[mission - 1]);
}

void FrBirthdayEventMissionDlg::CalculatePos(unsigned long index,
	unsigned long count)
{
	WRect rect(0, 0, 0, 0);
	GetClientRect(rect);
	rect.x = (float)index * rect.w +
		(g_view->GetWidth() * 0.5f - (float)count * rect.w * 0.5f);
	SetClientRect(rect);
}

void FrBirthdayEventMissionDlg::OnInitMissionNumberArea(int param)
{
	m_pMissionNumberArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}
