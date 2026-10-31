#include "minatl.h"
#include "bingoeventdlg.h"
#include "frbutton.h"
#include "frarea.h"
#include "frviewer.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "frwndinl.h"
#include "wfont.h"
#include "netresourcemanager.h"
#include "vc10_layout.h"
extern WView* g_view; // 00f02458


extern Fresh* g_pFresh;

// HACK: lobbytask (earlier in the link) owns this literal and keeps it as a local static of its naked
// OnPacket body, so the genuine ??_C@ COMDAT must not be emitted here (it would add a second copy)
static const char rb_bingo_noTryCount[] =
	"\xc3\xdf\xc3\xb7\xb1\xc7\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.";

namespace NS_BINGO
{
	int ns_board[10][25] = {
		{
         13,  2,
         10,     25,
         1,      12,
         8,      22,
         5,      16,
         19,     6,
         15,     14,
         24,     17,
         7,     4,
         21, 18,
         9,  20,
         23, 11,
         3, },
		{
         7,  16,
         18,      10,
         21,     14,
         1,     20,
         2,      9,
         19,      3,
         6,     13,
         25,      8,
         5,    17,
         23,  12,
         15, 11,
         24, 22,
         4, },
		{
         12, 3,
         16,  9,
         1,     4,
         5,     2,
         21,      10,
         23,      6,
         18,     14,
         22,     20,
         24,    15,
         13, 17,
         7, 8,
         19, 11,
         25, },
		{
         6,  2,
         21, 3,
         17, 22,
         20,      19,
         8,     10,
         14,     5,
         13,     25,
         1,      7,
         23,    11,
         18, 16,
         24,  9,
         12, 15,
         4, },
		{
         20, 17,
         12,  9,
         10, 25,
         1,  19,
         14,     15,
         16,     3,
         4,     7,
         11,      18,
         13,    21,
         23, 22,
         6,  8,
         2, 5,
         24, },
		{
         23, 7,
         3, 21,
         11,   2,
         4,   22,
         24, 5,
         19,     17,
         10,      8,
         18,     14,
         15,     20,
         6, 16,
         12, 13,
         25, 1,
         9, },
		{
         23, 21,
         3, 4,
         16,  2,
         15, 24,
         11,  10,
         13,  18,
         17,     6,
         22,      5,
         9,    8,
         7, 1,
         25, 14,
         12, 20,
         19, },
		{
         1,  16,
         15,  4,
         24,   5,
         2,   12,
         20, 22,
         9, 13,
         8, 23,
         21,      17,
         11,    3,
         19, 18,
         7,  14,
         6, 25,
         10, },
		{
         5, 20,
         10,  4,
         7, 8,
         23,  17,
         24, 22,
         3,  13,
         12,  25,
         14, 16,
         9,     19,
         6, 11,
         18,  21,
         15, 1,
         2, },
		{
         25, 15,
         17,  7,
         18, 19,
         13, 20,
         22,  11,
         1,  14,
         21, 5,
         16,  9,
         4, 2,
         8, 24,
         10, 6,
         12, 3,
         23, },
	};

	int ns_pointTable[25] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100,
		0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000,
		0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000, 0x1000000 };

	int ns_resultTable[12] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80,
		0x100, 0x200, 0x400, 0x800 };
}

CBingoPoint::CBingoPoint(int number)
	: m_pointNumber(number), m_bEnable(false), m_property(0)
{
	// HACK: emission order
	if (0)
	{
		EnableProperty(0);
		DisableProperty(0);
		IsProperty(0);
	}
}

CBingoPoint::~CBingoPoint()
{
}

void CBingoPoint::DrawPoint(FrGraphicInterface* gdi,
	const Bitmap** const bitmap, const WRect& rect, float x, float y)
{
	const Bitmap* image;
	if (m_property & 1)
		image = bitmap[4];
	else if (m_property & 2)
		image = bitmap[3];
	else
		image = bitmap[2];
	gdi->DrawTexture(image, rect, WRect(x, y, rect.w, rect.h), 0xffffffff, 0);
}

CBingoPoint& CBingoPoint::operator=(int number)
{
	m_pointNumber = number;
	return *this;
}

CBingoPoint& CBingoPoint::operator=(bool b)
{
	m_bEnable = b;
	return *this;
}

CBingoLine::CBingoLine(int size)
{
	// HACK: the original emits vector<CBingoPoint>::empty()
	if (0)
		m_points.empty();
}

CBingoLine::~CBingoLine()
{
}

void CBingoLine::ClearOverProperty()
{
	int count = m_points.size();
	for (int i = 0; i < count; i++)
		m_points[i].DisableProperty(1);
}

CBingoPoint* CBingoLine::FindBingoPoint(int number)
{
	int count = m_points.size();
	for (int i = 0; i < count; i++)
	{
		if (m_points[i].GetPointNumber() == number)
			return &m_points[i];
	}
	return NULL;
}

CBingoPoint& CBingoLine::GetBingoPoint(int index)
{
	return m_points[index];
}

const CBingoPoint& CBingoLine::GetBingoPoint(int index) const
{
	return m_points[index];
}

int CBingoLine::IsHavePointNumber(int number)
{
	int count = m_points.size();
	if (count == 0)
		return -1;
	for (int i = 0; i < count; i++)
	{
		if (m_points[i].GetPointNumber() == number)
			return i;
	}
	return -1;
}

bool CBingoLine::IsCompleteLine()
{
	int count = m_points.size();
	for (int i = 0; i < count; i++)
	{
		if (m_points[i].IsProperty(2) == false)
			return false;
	}
	return true;
}

int CBingoLine::operator[](int index)
{
	return m_points[index].GetPointNumber();
}

const int CBingoLine::operator[](int index) const
{
	return m_points[index].GetPointNumber();
}

CBingoLine& CBingoLine::operator=(const CBingoLine& line)
{
	m_points.clear();
	m_points.assign(line.m_points.begin(), line.m_points.end());
	m_reserved = line.m_reserved;
	return *this;
}

bool CBingoLine::operator+=(CBingoPoint& point)
{
	m_points.push_back(point);
	return true;
}

CBingoBoard::CBingoBoard()
{
	Initialize();
}

CBingoBoard::~CBingoBoard()
{
}

CBingoLine& CBingoBoard::operator[](int index)
{
	return m_lines[index];
}

bool CBingoBoard::Initialize()
{
	if (!m_lines.empty())
		m_lines.clear();
	for (int i = 0; i < 5; i++)
	{
		CBingoLine line(0);
		for (int j = 0; j < 5; j++)
			line += CBingoPoint(NS_BINGO::ns_board[0][i * 5 + j]);
		m_lines.push_back(line);
	}
	m_completeLine.reserve(m_lines.size() * 2 + 2);
	m_completeLine.assign(12, 0);
	return true;
}

bool CBingoBoard::AimPoint(int number)
{
	int count = m_lines.size();
	int index = -1;
	int i;
	for (i = 0; i < count; i++)
	{
		index = m_lines[i].IsHavePointNumber(number);
		if (index != -1)
			break;
	}
	CBingoPoint& point = m_lines[i].GetBingoPoint(index);
	if (point.IsProperty(2))
		return false;
	point.EnableProperty(2);
	ReScanBoard();
	return true;
}

void CBingoBoard::ClearOverProperty()
{
	int count = m_lines.size();
	for (int i = 0; i < count; i++)
		m_lines[i].ClearOverProperty();
}

CBingoPoint* CBingoBoard::FindBingoPoint(int number)
{
	int count = m_lines.size();
	for (int i = 0; i < count; i++)
	{
		CBingoPoint* point = m_lines[i].FindBingoPoint(number);
		if (point != NULL)
			return point;
	}
	return NULL;
}

void CBingoBoard::ReScanBoard()
{
	int index = 0;
	int count = m_lines.size();
	int i;
	int j;
	bool complete;

	for (i = 0; i < count; i++)
	{
		if (m_lines[i].IsCompleteLine())
			m_completeLine[index] = 1;
		index++;
	}

	complete = false;
	for (i = 0; i < count; i++)
	{
		for (j = 0; j < count; j++)
		{
			if (!m_lines[j].GetBingoPoint(j).IsProperty(2))
				break;
			complete = true;
		}
		if (complete == true)
			m_completeLine[index] = 1;
		index++;
		complete = false;
	}

	complete = true;
	j = 0;
	for (i = 0; i < count; i++)
	{
		if (!m_lines[i].GetBingoPoint(j--).IsProperty(2))
		{
			complete = false;
			break;
		}
	}
	if (complete)
		m_completeLine[index++] = 1;

	complete = true;
	j = count - 1;
	for (i = 0; i < count; i++)
	{
		if (!m_lines[i].GetBingoPoint(j--).IsProperty(2))
		{
			complete = false;
			break;
		}
	}
	if (complete)
		m_completeLine[index] = 1;
}

IMPLEMENT_OBJECT(FrBingoEventDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBingoEventDlg, FrForm)

ON_FRESH_VI("start", FRCMD_INIT, FrBingoEventDlg::OnInitStartButton)
ON_FRESH_VV("start", FRCMD_LBUTTONDOWN,
	FrBingoEventDlg::OnLButtonDownStartButton)
ON_FRESH_VI("detail", FRCMD_INIT, FrBingoEventDlg::OnInitEventDetail)
ON_FRESH_VV("detail", FRCMD_LBUTTONDOWN,
	FrBingoEventDlg::OnLButtonDownEventDetail)
ON_FRESH_VI("boardNumber", FRCMD_INIT, FrBingoEventDlg::OnInitBoardNumberArea)
ON_FRESH_VI("boardNumber", FRCMD_OWNERDRAW,
	FrBingoEventDlg::OnOwnerDrawBoardNumberArea)

END_FRESH_MSGMAP()

FrBingoEventDlg::FrBingoEventDlg()
{
	m_pStartButton = NULL;
	m_pEventDetail = NULL;
	m_pDetailDlg = NULL;
	m_pGiftDlg = NULL;
	m_tryCount = 0;
	m_boardIndex = 0;
	memset(m_pBitmap, 0, sizeof(m_pBitmap));
	m_pBoardNumberArea = NULL;
	m_selectedNumber = 0;
	m_completeLine = 0;
	m_aimedPoint = 0;
	m_newLine = 0;
	m_bWaitResult = false;
	m_bReserved1 = false;
	m_bReserved2 = false;
	memset(m_tryCountText, 0, sizeof(m_tryCountText));
}

FrBingoEventDlg::~FrBingoEventDlg()
{
}

bool FrBingoEventDlg::OnInit()
{
	m_boardIndex = Doc()->m_myInfo.info.dwUID % 10;
	LoadBingoImage();
	m_boardPos = WPoint(40.0f, 205.0f);
	m_randomNumberPos = WPoint(380.0f, 180.0f);
	m_tryCountPos = WPoint(380.0f, 403.0f);
	InitCharMatchMap();
	InitNumberMatchMap();
	return FrForm::OnInit();
}

void CalcOffset(int index, int columns, float& x, float& y, float width,
	float height, float unused)
{
	int row = index / columns;
	int column = index % columns;
	x = column * width;
	y = row * height;
}

bool FrBingoEventDlg::OnDetailDlgResult(int result, FrForm* form)
{
	m_pDetailDlg = NULL;
	return true;
}

bool FrBingoEventDlg::OnGiftDlgResult(int result, FrForm* form)
{
	m_pGiftDlg = NULL;
	return true;
}

void FrBingoEventDlg::SetBingoData(GlobalEnum::sBingoInfo* info)
{
	m_tryCount = info->tryCount;
	m_completeLine = info->completeLine;
	m_aimedPoint = info->aimedPoint;
	memset(m_tryCountText, 0, sizeof(m_tryCountText));
	itoa(m_tryCount > 99 ? 99 : m_tryCount, m_tryCountText, 10);
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			CBingoPoint& point = m_board[i].GetBingoPoint(j);
			point.SetPointNumber(NS_BINGO::ns_board[m_boardIndex][i * 5 + j]);
			if (m_aimedPoint &
				NS_BINGO::ns_pointTable[point.GetPointNumber() - 1])
				point.EnableProperty(2);
		}
	}
}

void FrBingoEventDlg::SetGiftData(std::vector<unsigned long>* gifts,
	unsigned int completeLine)
{
	m_completeLine = completeLine;
	m_pGiftDlg = CreateForm<FrBingoGiftDlg>(g_pFresh->GetManager(), this,
		"bingo_gift", NULL);
	if (m_pGiftDlg)
	{
		m_pGiftDlg->Open((FRESH_PFN_RESULT)&FrBingoEventDlg::OnGiftDlgResult,
			1);
		m_pGiftDlg->SetGiftData(gifts);
	}
}

void FrBingoEventDlg::SetSelectedData(int number, int count,
	unsigned int newLine)
{
	m_selectedNumber = number;
	CBingoPoint* point = m_board.FindBingoPoint(number);
	if (point)
		point->EnableProperty(1);
	m_newLine = newLine;
	m_bWaitResult = false;
}

void FrBingoEventDlg::InitCharMatchMap()
{
	char ch = '0';
	for (int i = 0; i < 10; i++)
	{
		int column = (unsigned int)i % 10;
		int row = (unsigned int)i / 10;
		m_charMatchMap.insert(
			std::make_pair(ch++, sPoint(column * 20.0f, row * 32.0f)));
	}
}

void FrBingoEventDlg::InitNumberMatchMap()
{
	int i;
	for (i = 0; i < 25; i++)
	{
		int column = (unsigned int)i % 6;
		int row = (unsigned int)i / 6;
		m_boardNumberMatchMap.insert(
			std::make_pair(i + 1, sPoint(column * 39.0f, row * 32.0f)));
	}
	for (i = 0; i < 25; i++)
	{
		int column = (unsigned int)i % 5;
		int row = (unsigned int)i / 5;
		m_randomNumberMatchMap.insert(
			std::make_pair(i + 1, sPoint(column * 48.0f, row * 37.0f)));
	}
}

void FrBingoEventDlg::LoadBingoImage()
{
	const char* names[10] = { "bingo_conut_num_all", "bingo_num_all",
		"bingo_num_01", "bingo_num_02", "bingo_num_03", "bingo_line_1",
		"bingo_line_2", "bingo_line_3", "bingo_line_4", "bingo_ment" };
	for (int i = 0; i < 10; i++)
		m_pBitmap[i] = g_pFresh->RegisterBitmap(names[i]);
}

void FrBingoEventDlg::OnProc(const float dt)
{
}

void FrBingoEventDlg::DrawBoardNumber(FrGraphicInterface* gdi,
	const WRect& rect)
{
	WRect src(0.0f, 0.0f, 39.0f, 32.0f);
	float columnX = 0.0f;
	float rowY = 0.0f;
	float rowX = 0.0f;
	float slantX = 0.0f;
	float slantRowY = 0.0f;
	for (int i = 0; i < 5; i++)
	{
		float slantY = 0.0f;
		for (int j = 0; j < 5; j++)
		{
			sPoint& pt = m_boardNumberMatchMap[m_board[i][j]];
			src.x = pt.x;
			src.y = pt.y;
			float x = slantX + rect.x + rowX + columnX + m_boardPos.x;
			float y = rect.y + m_boardPos.y + slantY + slantRowY + rowY;
			m_board[i].GetBingoPoint(j).DrawPoint(gdi, m_pBitmap, src, x, y);
			columnX = (src.w + 9.0f) * (j + 1);
			slantX += 4.0f;
			slantY -= 8.0f;
		}
		rowX = 9.0f * (i + 1);
		rowY = (src.h + 18.0f) * (i + 1);
		slantRowY += 2.0f;
		columnX = 0.0f;
		slantX = 0.0f;
	}
}

void FrBingoEventDlg::DrawRandomNumber(FrGraphicInterface* gdi,
	const WRect& rect)
{
	WRect src(0.0f, 0.0f, 48.0f, 37.0f);
	sPoint& pt = m_randomNumberMatchMap[m_selectedNumber];
	src.x = pt.x;
	src.y = pt.y;
	gdi->DrawTexture(m_pBitmap[1], src,
		WRect(rect.x + m_randomNumberPos.x, rect.y + m_randomNumberPos.y, src.w,
			src.h),
		0xffffffff, 0);
}

void FrBingoEventDlg::DrawTryCount(FrGraphicInterface* gdi, const WRect& rect)
{
	WRect src(0.0f, 0.0f, 20.0f, 32.0f);
	float offset = 0.0f;
	int length = strlen(m_tryCountText);
	float x = rect.x + m_tryCountPos.x;
	float y = rect.y + m_tryCountPos.y;
	switch (length)
	{
	case 1:
		x += 12.0f;
		break;
	case 2:
		x += 3.0f;
		break;
	}
	for (int i = 0; i < length; i++)
	{
		sPoint& pt = m_charMatchMap[m_tryCountText[i]];
		src.x = pt.x;
		src.y = pt.y;
		gdi->DrawTexture(m_pBitmap[0], src, WRect(x + offset, y, src.w, src.h),
			0xffffffff, 0);
		offset += 20.0f;
	}
}

void FrBingoEventDlg::DrawCompleteBar(FrGraphicInterface* gdi,
	const WRect& rect)
{
	const Bitmap* bitmap;
	float width;
	float height;
	float x;
	float y;
	for (int i = 0; i < 12; i++)
	{
		if (i < 5)
		{
			bitmap = m_pBitmap[5];
			width = 221.0f;
			height = 42.0f;
			x = i * 7.5f + 15.0f;
			y = i * 53.0f - 20.0f;
		}
		else if (i < 10 && i > 4)
		{
			bitmap = m_pBitmap[6];
			width = 44.0f;
			height = 219.0f;
			x = (i - 5) * 54.0f + 10.0f;
			y = 15.0f - (i - 5) * 8.0f;
		}
		else if (i == 10)
		{
			bitmap = m_pBitmap[7];
			width = 246.0f;
			x = 20.0f;
			height = 181.0f;
			y = 20.0f;
		}
		else
		{
			bitmap = m_pBitmap[8];
			width = 181.0f;
			x = 50.0f;
			height = 246.0f;
			y = -15.0f;
		}
		if (m_completeLine & NS_BINGO::ns_resultTable[i])
		{
			gdi->DrawTexture(bitmap,
				WRect(m_boardPos.x + rect.x + x, m_boardPos.y + rect.y + y,
					width, height),
				0xffffffff, 0);
		}
	}
}

void FrBingoEventDlg::DrawBingoLogo(FrGraphicInterface* gdi, const WRect& rect)
{
	float x;
	float y;
	unsigned int width = m_pBitmap[9]->Width();
	unsigned int height = m_pBitmap[9]->Height();
	for (int i = 0; i < 12; i++)
	{
		if (i < 5)
		{
			y = i * 53.0f - 20.0f;
			x = i * 7.5f + 15.0f + width / 2;
		}
		else if (i < 10 && i > 4)
		{
			x = (i - 5) * 54.0f + 10.0f;
			y = 15.0f - (i - 5) * 8.0f + height / 2;
		}
		else if (i == 10)
		{
			x = width / 2 + 20.0f;
			y = height / 2 + 20.0f;
		}
		else
		{
			x = width / 2 + 50.0f;
			y = height / 2 - 15.0f;
		}
		if (m_newLine & NS_BINGO::ns_resultTable[i])
		{
			gdi->DrawTexture(m_pBitmap[9],
				WRect(m_boardPos.x + rect.x + x, m_boardPos.y + rect.y + y,
					67.0f, 32.0f),
				0xffffffff, 0);
		}
	}
}

void FrBingoEventDlg::OnInitStartButton(int param)
{
	m_pStartButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBingoEventDlg::OnLButtonDownStartButton()
{
	if (m_bWaitResult == true)
		return;
	if (m_tryCount < 1)
	{
		AfxGetTask()->GetActor("Lobby")
			<< MsgObject(NULL, M983(0x23), (int)rb_bingo_noTryCount, 0, 0, 0, 0);
		return;
	}
	m_board.ClearOverProperty();
	m_newLine = 0;
	int total = 0;
	for (int i = 0; i < 12; i++)
		total += NS_BINGO::ns_resultTable[i];
	if (m_completeLine == total)
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
			(int)"\xc0\xcc\xb9\xcc \xb8\xf0\xb5\xe7 \xbc\xb1\xb9\xb0\xc0\xbb \xb4\xd9 \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.",
			0, 0, 0, 0);
		return;
	}
	WSendPacket packet((enumClientPacket)0xe3);
	packet.Encode4(1);
	packet.Send(TO_GAME);
	m_bWaitResult = true;
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
}

void FrBingoEventDlg::OnInitEventDetail(int param)
{
	m_pEventDetail = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBingoEventDlg::OnLButtonDownEventDetail()
{
	m_pDetailDlg = CreateForm<FrBingoDetailDlg>(g_pFresh->GetManager(), this,
		"bingo_detail", NULL);
	if (m_pDetailDlg)
	{
		m_pDetailDlg->Open(
			(FRESH_PFN_RESULT)&FrBingoEventDlg::OnDetailDlgResult, 1);
	}
}

void FrBingoEventDlg::OnInitBoardNumberArea(int param)
{
	m_pBoardNumberArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrBingoEventDlg::OnOwnerDrawBoardNumberArea(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		const WRect& rect = GetRect();
		DrawCompleteBar(gdi, rect);
		DrawBoardNumber(gdi, rect);
		DrawRandomNumber(gdi, rect);
		DrawTryCount(gdi, rect);
		DrawBingoLogo(gdi, rect);
	}
}

IMPLEMENT_OBJECT(FrBingoDetailDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBingoDetailDlg, FrForm)

ON_FRESH_VI("detailview", FRCMD_INIT, FrBingoDetailDlg::OnInitExplaneViewer)

END_FRESH_MSGMAP()

FrBingoDetailDlg::FrBingoDetailDlg()
{
}

FrBingoDetailDlg::~FrBingoDetailDlg()
{
}

void FrBingoDetailDlg::OnInitExplaneViewer(int param)
{
	m_pExplaneViewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pExplaneViewer)
		m_pExplaneViewer->Open("bingo_explanation.jpg");
}

IMPLEMENT_OBJECT(FrBingoGiftDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBingoGiftDlg, FrForm)

ON_FRESH_VI("giftList", FRCMD_INIT, FrBingoGiftDlg::OnInitGiftArea)
ON_FRESH_VI("giftList", FRCMD_OWNERDRAW, FrBingoGiftDlg::OnOwnerDrawGiftArea)

END_FRESH_MSGMAP()

FrBingoGiftDlg::FrBingoGiftDlg()
	: m_pGiftArea(NULL)
{
	m_offsetX[0] = 84.0f;
	m_offsetX[1] = 52.0f;
	m_offsetX[2] = 21.0f;
	m_offsetX[3] = -5.0f;
	memset(m_pGiftImage, 0, sizeof(m_pGiftImage));

	static char* s_countFont[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_countFont;
	info.numPages = 1;
	info.fontw = 16;
	info.fonth = 16;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pCountFont = g_resrcmng->GetTitleFont();
	m_pCountFont->Create(&info);
	C_ASSERT_VC10(sizeof(FrBingoGiftDlg) == 0x160 && offsetof(FrBingoGiftDlg, m_pCountFont) == 0x154); // 983: push 0x160 em 00aff25a
}

FrBingoGiftDlg::~FrBingoGiftDlg()
{
	if (g_resrcmng && m_pCountFont)
	{
		g_resrcmng->Release(m_pCountFont);
		m_pCountFont = NULL;
	}
}

void FrBingoGiftDlg::SetGiftData(std::vector<unsigned long>* gifts)
{
	m_giftList.assign(gifts->begin(), gifts->end());
	int count = m_giftList.size();
	for (int i = 0; i < count; i++)
	{
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(m_giftList[i]);
		if (common)
			m_pGiftImage[i] = g_pFresh->GetBitmap(common->Icon);
	}
}

void FrBingoGiftDlg::SetGiftCount(std::vector<unsigned long>* counts)
{
	m_giftCount.assign(counts->begin(), counts->end());
}

void FrBingoGiftDlg::OnInitGiftArea(int param)
{
	m_pGiftArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrBingoGiftDlg::OnOwnerDrawGiftArea(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		float offset = 0.0f;
		const WRect& rect = m_pGiftArea->GetRect();
		int count = m_giftList.size();
		for (int i = 0; i < count; i++)
		{
			const Bitmap* bitmap = m_pGiftImage[i];
			gdi->DrawTexture(bitmap,
				WRect(offset + rect.x + m_offsetX[count - 1], rect.y,
					(float)bitmap->Width(), (float)bitmap->Height()),
				0xffffffff, 0);
			if (m_giftCount[i])
			{
				char text[9] = { 0 };
				sprintf(text, "%d", m_giftCount[i]);
				m_pCountFont->Print(g_view,
					rect.x + m_offsetX[count - 1] + offset + 26.0f,
					rect.y + 53.0f, text, 0, 0xffffffff, NULL);
			}
			offset += 63.0f;
		}
	}
}
