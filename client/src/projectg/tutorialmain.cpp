#include "minatl.h"
#include "vc10_layout.h"
#define REBANG_TUTORIALMAIN_CPP
#include "tutorialmain.h"
#include "exhibition.h"
#include "fresh.h"
#include "frwnd.h"
#include "chatmsg.h"
#include "rankingdlg.h"
#include "questinfodlg.h"
#include "../shared/sharedtables.h"
#include "user_info.h"
#include "shareddoc.h"
#include "lobbymain.h"
#include "frlistbox.h"
#include "frscrollbar.h"
#include "frgraphicinterface.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "taskmanager.h"
#include "packet.h"
#include "contentsdoc.h"
#include "packets/tutorialcontainer.h"
#include <cmath>
#include <mmsystem.h>
#include "jp/taskwidget.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_tutorialComplete) == 0x10d50 && offsetof(CSharedDoc, m_tutorialStartMode) == 0x10d5c); // A737
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameTypeName) == 0x10595 && offsetof(CSharedDoc, m_gameTypeInfo) + 0x10 * sizeof(sGameTypeInfo) == 0x148); // A737
C_ASSERT_VC10(offsetof(CSharedDoc, m_gameMode) == 0x1048c && offsetof(CSharedDoc, m_bGameOver) == 0x10488 && offsetof(CSharedDoc, m_holeOrder) == 0x1051e); // A738
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594 && offsetof(CSharedDoc, m_golfGame.holes) == 0x10699); // A738
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.weather) == 0x1069a && offsetof(CSharedDoc, m_golfGame.shotTimeLimit) == 0x1069c); // A738
C_ASSERT_VC10(offsetof(CSharedDoc, m_slotList) == 0x109fc); // A738


extern Fresh* g_pFresh;

struct CTutorialMain::sTutorialMap
{
	const char* name;
	unsigned char type;
	unsigned char number;
	unsigned char reserved;
	unsigned char row;
	unsigned char id;
	int progress;
	int state;
	unsigned char enable;
};

struct CTutorialMain::sCategoryInfo
{
	unsigned char id;
	unsigned char count;
	short pad;
	int y;
};

struct CTutorialMain::sArrowInfo
{
	unsigned char begin;
	unsigned char end;
	unsigned char flag;
};

struct sTutoChild
{
	int index;
	int cond;
	int base;
	int count;
};

struct sTutoCondition
{
	unsigned int mask;
	unsigned int reward;
	unsigned int count;
};

static const sTutoCondition m_TutoCondition[20] = {	// 00d27188 (983; recompensas do 645 trocadas)
	{ 0x1,     0x1a000002, 10   },
	{ 0x2,     0x1800000b, 3    },
	{ 0x4,     0x18000025, 3    },
	{ 0x8,     0x18000005, 3    },
	{ 0x10,    0x18000004, 3    },
	{ 0x20,    0x1800000a, 3    },
	{ 0x40,    0x18000000, 3    },
	{ 0x80,    0x1a000010, 1000 },
	{ 0x100,   0x1a000002, 10   },
	{ 0x200,   0x18000028, 1    },
	{ 0x400,   0x18000006, 1    },
	{ 0x800,   0x1800000a, 3    },
	{ 0x1000,  0x18000000, 4    },
	{ 0x2000,  0x18000001, 4    },
	{ 0x8000,  0x18000005, 3    },
	{ 0x10000, 0x18000005, 3    },
	{ 0x20000, 0x18000005, 3    },
	{ 0x40000, 0x18000005, 3    },
	{ 0x80000, 0x18000005, 3    },
	{ 0x80000, 0x18000005, 3    },
};

static const sTutoChild m_CheckTable[8] = {
	{ 0, 0, 1, 1 },
	{ 1, 0, 2, 2 },
	{ 2, 0, 4, 2 },
	{ 3, 0, 5, 2 },
	{ 4, 7, 4, 3 },
	{ 5, 7, 4, 3 },
	{ 6, 7, 4, 3 },
	{ 7, 0, 1, 1 },
};

struct sTutoLink
{
	int id;
	int child;
	int count;
};

static const sTutoLink m_ChildTable[14] = {
	{ 0,   1,   1 },
	{ 1,   2,   2 },
	{ 2,   4,   2 },
	{ 3,   5,   2 },
	{ 4,   7,   1 },
	{ 5,   7,   1 },
	{ 6,   7,   1 },
	{ 7,   7,   0 },
	{ 8,   9,   2 },
	{ 9,   0xb, 1 },
	{ 0xa, 0xb, 1 },
	{ 0xb, 0xc, 2 },
	{ 0xc, 0xc, 0 },
	{ 0xd, 0xd, 0 },
};

struct sTutorialBitmap
{
	int index;
	const char* name;
};

static const sTutorialBitmap s_TutorialBitmaps[64] = {
	{ 0,  "tu_btn_01_n"       },
	{ 1,  "tu_btn_02_n"       },
	{ 2,  "tu_btn_03_n"       },
	{ 3,  "tu_btn_04_n"       },
	{ 4,  "tu_btn_05_n"       },
	{ 5,  "tu_btn_06_n"       },
	{ 6,  "tu_btn_07_n"       },
	{ 7,  "tu_btn_08_n"       },
	{ 8,  "tu_btn_09_n"       },
	{ 9,  "tu_btn_10_n"       },
	{ 10, "tu_btn_11_n"       },
	{ 11, "tu_btn_12_n"       },
	{ 12, "tu_btn_13_n"       },
	{ 13, "tu_btn_14_n"       },
	{ 14, "tu_btn_01_o"       },
	{ 15, "tu_btn_02_o"       },
	{ 16, "tu_btn_03_o"       },
	{ 17, "tu_btn_04_o"       },
	{ 18, "tu_btn_05_o"       },
	{ 19, "tu_btn_06_o"       },
	{ 20, "tu_btn_07_o"       },
	{ 21, "tu_btn_08_o"       },
	{ 22, "tu_btn_09_o"       },
	{ 23, "tu_btn_10_o"       },
	{ 24, "tu_btn_11_o"       },
	{ 25, "tu_btn_12_o"       },
	{ 26, "tu_btn_13_o"       },
	{ 27, "tu_btn_14_o"       },
	{ 28, "tu_btn_01_d"       },
	{ 29, "tu_btn_02_d"       },
	{ 30, "tu_btn_03_d"       },
	{ 31, "tu_btn_04_d"       },
	{ 32, "tu_btn_05_d"       },
	{ 33, "tu_btn_06_d"       },
	{ 34, "tu_btn_07_d"       },
	{ 35, "tu_btn_08_d"       },
	{ 36, "tu_btn_09_d"       },
	{ 37, "tu_btn_10_d"       },
	{ 38, "tu_btn_11_d"       },
	{ 39, "tu_btn_12_d"       },
	{ 40, "tu_btn_13_d"       },
	{ 41, "tu_btn_14_d"       },
	{ 42, "tu_btn_lock"       },
	{ 43, "tu_empty_first"    },
	{ 44, "tu_empty_first_n"  },
	{ 45, "tu_off_frame01"    },
	{ 46, "tu_off_frame02"    },
	{ 47, "tu_off_frame03"    },
	{ 48, "message_lock_n"    },
	{ 49, "tu_icon_clear"     },
	{ 50, "tu_arrow_n"        },
	{ 51, "tu_arrow_o"        },
	{ 52, "tu_icon_lock_01"   },
	{ 53, "tu_icon_lock_02"   },
	{ 54, "tu_obtain_01"      },
	{ 55, "tu_obtain_02"      },
	{ 56, "tu_obtain_02_nuri" },
	{ 57, "tu_obtain_02_hana" },
	{ 58, "tu_class_01"       },
	{ 59, "tu_class_02"       },
	{ 60, "tu_class_03"       },
	{ 61, "tu_tip_01"         },
	{ 62, "tu_tip_02"         },
	{ 63, "tu_tip_03"         },
};

static char s_szTaskClass[12] = "CTutorial";
static char s_szTaskName[12] = "Tutorial";
CTutorialMain::sTutorialResult CTutorialMain::m_CompleteItems[3] = {
	{ 0, 3 },
    { 1, 3 },
    { 2, 3 }
};

CTutorialMain::sCategoryInfo CTutorialMain::m_sCategoryItem[3] = {
	{ 0, 8, 0, 200 },
	{ 1, 6, 0, 370 },
	{ 2, 6, 0, 0   },
};
CTutorialMain::sArrowInfo CTutorialMain::m_ArrowItems[10] = {
	{ 0, 0, 1 },
	{ 1, 1, 1 },
	{ 2, 2, 1 },
	{ 3, 3, 1 },
	{ 4, 6, 1 },
	{ 7, 7, 1 },
	{ 0, 1, 1 },
	{ 2, 2, 1 },
	{ 3, 3, 1 },
	{ 4, 5, 1 },
};

// 983: as tres sao inline (o OnInit e o CheckNodeStates 007d3c30 embutem os switches); as copias fora de linha de
// 007d1fe0/007d2000 sobram de chamadas que o /Ob1 nao embutiu
inline unsigned char GetAccumulateLimit(unsigned char grade)
{
	unsigned char limit = 0;
	switch (grade)
	{
	case 0:
		limit = 8;
		break;
	case 1:
		limit = 14;
		break;
	case 2:
		limit = 0;
		break;
	}
	return limit;
}

inline unsigned char GetGradeBegin_Grade(unsigned char grade)
{
	unsigned char begin = 0;
	switch (grade)
	{
	case 0:
		begin = 0;
		break;
	case 1:
		begin = 8;
		break;
	case 2:
		begin = 0;
		break;
	}
	return begin;
}

__forceinline unsigned int GetQuestFlags(int index)
{
	return (unsigned int)CSharedDoc::Instance()->m_tutorialComplete[index];
}

__forceinline bool IsCompleteBeforeGrade(unsigned char grade)
{
	switch (grade)
	{
	case 0:
		return true;
	case 1:
		if (GetQuestFlags(0) != 0xff)
			return false;
		return true;
	case 2:
		if (GetQuestFlags(0) != 0xff || GetQuestFlags(1) != 0x3f00)
			return false;
		return true;
	}
	return false;
}

// 007d2030..007d2060: setters do documento 0xa4, definidos nesta unidade no 983 (sem chamador: o SetQuestInformation
// grava os campos direto)
void CTutorialContainer983::SetCategory983(int category)
{
	m_category = category;
}

void CTutorialContainer983::SetMode983(int mode)
{
	m_mode = mode;
}

void CTutorialContainer983::SetIndex983(int index)
{
	m_index = index;
}

void CTutorialContainer983::SetSendResult983(bool bSend)
{
	m_bSendResult = bSend;
}

// HACK: every field is a comma expression, which is not a constant expression: an aggregate with non-constant
// initializers keeps its dynamic initializer (the original stores all nine fields of the 14 records in .text$yc,
// into zero storage that sits after m_RTTI). With plain constants, or with a constructor, VC7.1 folds the table
// into .data.
#define TUTORIAL_FIELD(value) (0, value)
// 983 00cd7b90: name = K2L_Compatibility(texto) em cada uma das 14 entradas (e7b448, passo 0x18).
#define TUTORIAL_MAP(name, type, number, row, id) \
	{ K2L_Compatibility(name), TUTORIAL_FIELD(type), TUTORIAL_FIELD(number), \
		TUTORIAL_FIELD(0), TUTORIAL_FIELD(row), TUTORIAL_FIELD(id), \
		TUTORIAL_FIELD(0), TUTORIAL_FIELD(0), TUTORIAL_FIELD(1) }

CTutorialMain::sTutorialMap CTutorialMain::m_Tutorials[14] = {
	TUTORIAL_MAP("\xc5\xac\xb7\xb4\xbc\xb1\xc5\xc3", 0, 1, 0, 0),
	TUTORIAL_MAP("\xc5\xb8\xb1\xb8\xb9\xe6\xb9\xfd", 0, 2, 1, 1),
	TUTORIAL_MAP("\xc6\xce\xbe\xdf\xb0\xd4\xc0\xcc\xc1\xf6", 0, 3, 2, 2),
	TUTORIAL_MAP("\xc6\xdb\xc6\xc3\xc0\xc7 \xb1\xe2\xba\xbb", 0, 4, 2, 3),
	TUTORIAL_MAP("\xbe\xc6\xc0\xcc\xc5\xdb\xbb\xe7\xbf\xeb", 0, 5, 3, 4),
	TUTORIAL_MAP("\xc1\xf6\xb8\xe9\xbc\xd3\xbc\xba", 0, 6, 3, 5),
	TUTORIAL_MAP("\xbd\xba\xc4\xda\xbe\xee\xb0\xe8\xbb\xea", 0, 7, 3, 6),
	TUTORIAL_MAP("\xc8\xa6\xc0\xce\xc7\xcf\xb1\xe2", 0, 8, 4, 7),

	TUTORIAL_MAP("\xb0\xf8\xbf\xc5\xb1\xe2\xb1\xe2", 1, 9, 0, 8),
	TUTORIAL_MAP("\xb0\xf8\xc0\xc7 \xb1\xe2\xbf\xef\xb1\xe2", 1, 10, 1, 9),
	TUTORIAL_MAP("\xb9\xd9\xb6\xf7 \xc0\xd0\xb1\xe2", 1, 11, 1, 10),
	TUTORIAL_MAP("\xc5\xb8\xc1\xa1\xc0\xcc\xb5\xbf", 1, 12, 2, 11),
	TUTORIAL_MAP("\xc6\xc4\xbf\xf6 \xbd\xba\xc7\xc9", 1, 13, 3, 12),
	TUTORIAL_MAP("\xc6\xc4\xbf\xf6 \xc4\xbf\xba\xea", 1, 14, 3, 13),
};


// 983: so a fabrica de objetos (00cd7e80, sem o AddActorClass do IMPLEMENT_ACTOR do 645); o form "tutorialmain" e
// registrado a parte (00cd7ea0: fabrica de forms 00c0efc0/00c0ecb0 com 007d5160 -> CreateForm<CTutorialMain> 007d5090)
IMPLEMENT_OBJECT(CTutorialMain, FrTaskForm)

static FrForm* CreateTutorialMain983(FrWndManager* pManager, const char* name)
{
	return CreateForm<CTutorialMain>(pManager, NULL, name);
}
static FrFormRegistrar983 s_tutorialMainRegistrar("tutorialmain", CreateTutorialMain983);

// 983 (mapa 00e1b070; entradas preenchidas em 00cd7ec0): so a janela do tutorial, sem as barras do CTaskMain do 645
BEGIN_FRESH_MSGMAP(CTutorialMain, FrForm)
ON_FRESH_VI("pet_pipin", FRCMD_INIT, CTutorialMain::OnTutorialAreaPipin)
ON_FRESH_VI("pet_pipin", FRCMD_OWNERDRAW, CTutorialMain::OnTutorialAreaPipinOwnerDraw)
ON_FRESH_VI("tutorial_list", FRCMD_INIT, CTutorialMain::OnTutorialListInit)
ON_FRESH_VI("tutorial_list", FRCMD_OWNERDRAW, CTutorialMain::OnTutorialListOwnerDraw)
ON_FRESH_VV("tutorial_list", FRCMD_LBUTTONDOWN, CTutorialMain::OnTutorialListLBtnDown)
ON_FRESH_VV("tutorial_list", FRCMD_LBUTTONUP, CTutorialMain::OnTutorialListLBtnUp)
ON_FRESH_VV("close_wnd", FRCMD_LBUTTONUP, CTutorialMain::OnCloseWndUp983)
END_FRESH_MSGMAP()

// 007d2080
void CTutorialMain::LoadTutorialMainImages()
{
	for (int i = 0; i < 64; i++)
		m_pBitmaps[i] = g_pFresh->RegisterBitmap(s_TutorialBitmaps[i].name);
}

// 007d20c0
void CTutorialMain::EnableAllBtn()
{
	for (int i = 0; i < 14; i++)
		m_Tutorials[i].enable = 1;
}

// 007d20e0
void CTutorialMain::HidePet()
{
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
}

// 007d2110
void CTutorialMain::ProcessAnimateIcon()
{
	static float s_iconAngle;
	m_iconOffset = cos(s_iconAngle * g_DEGTORAD) * 5.0f;
	s_iconAngle += 1.0f;
	if (s_iconAngle > 360.0f)
		s_iconAngle = 0.0f;
}

// 007d2180
void CTutorialMain::DrawCourseArrow()
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = m_ArrowItems[i].begin; j <= m_ArrowItems[i].end; j++)
		{
			unsigned char base = 0;
			switch (m_Tutorials[j].type)
			{
			case 0:
				base = 0;
				break;
			case 1:
				base = 8;
				break;
			case 2:
				base = 0;
				break;
			}
			int state = m_Tutorials[j].state;
			if (state == 1 || state == 0)
				m_ArrowItems[base + i].flag = 0;
			else if (state == 2)
				m_ArrowItems[base + i].flag = 1;
		}
	}
}

// 007d2210
int CTutorialMain::IsInArea(int area, WPoint pt)
{
	const WPoint* rects;
	unsigned char count = 0;
	switch (area)
	{
	case 0:
		rects = m_areaRects;
		count = 8;
		break;
	case 1:
		rects = m_areaRects2;
		count = 6;
		break;
	}
	for (unsigned char i = 0; i < count; i++)
	{
		if (pt.x > rects[i].x && pt.x < rects[i].x + 85.0f && pt.y > rects[i].y &&
			pt.y < rects[i].y + 48.0f)
			return i;
	}
	return -1;
}

// 007d22b0: botao "close_wnd" (983)
void CTutorialMain::OnCloseWndUp983()
{
	Close(true);
}

// 007d22c0 (983): so o documento 0xa4 (+4 categoria, +8 modo, +0xc indice; o byte 6 do 645 nao vai)
void CTutorialMain::SetQuestInformation(sTutorialMap* map)
{
	m_pTutorialDoc983->m_category = ((unsigned char*)map)[4];
	m_pTutorialDoc983->m_mode = ((unsigned char*)map)[5];
	m_pTutorialDoc983->m_index = ((unsigned char*)map)[8];
}

// 007d2300 (983): +4 = a, +8 = b + 1, +0xc = b no documento 0xa4
void CTutorialMain::SetQuestInformation(int a, int b)
{
	m_pTutorialDoc983->m_category = a;
	m_pTutorialDoc983->m_mode = b + 1;
	m_pTutorialDoc983->m_index = b;
}

// 007d2460 (slot 6, FrWnd::Enable): a janela do tutorial some junto quando desabilitada
void CTutorialMain::Enable(bool enable)
{
	FrWnd::Enable(enable);
	FrWnd::SetVisible(enable);
}

// 007d2490 (+0x98): a CTaskWidget abriu a janela - documento 0xa4 do tutorial (pedido C58.1) e as imagens (645 OnLoad)
bool CTutorialMain::OnOpenWidget983()
{
	m_pTutorialDoc983 = NULL;
	IContentsDataContainer* pDoc = CContentsDoc::Instance()->GetContainer((localContentType_t)0xa4);
	if (pDoc != NULL)
		m_pTutorialDoc983 = (CTutorialContainer983*)pDoc;
	LoadTutorialMainImages();
	return true;
}

// 007d24d0
void CTutorialMain::OnTutorialAreaPipin(int param)
{
	m_pAreaPipin = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 007d2540
void CTutorialMain::OnTutorialListInit(int param)
{
	m_pTutorialList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pTutorialList)
	{
		m_pTutorialList->UseRightButton(true);
		m_pTutorialList->UseDummy(true);
		m_pTutorialList->SetPushSound(NULL);
		m_pTutorialList->ClearItem();
		for (int i = 0; i < 3; i++)
			m_pTutorialList->AddItem(&m_CompleteItems[i]);
		m_select.Clear();
	}
}

// 007d2610
int CTutorialMain::GetSelectedBtn(int area, WPoint pt)
{
	return IsInArea(area, pt);
}

// 007d2630 (983): solta a licao marcada (progress) da categoria do item selecionado
void CTutorialMain::OnTutorialListLBtnUp()
{
	if (!m_pTutorialList)
		return;
	FrListItem* selected = m_pTutorialList->GetItemUnderCursor();
	if (!selected)
		return;
	unsigned char category = (unsigned char)selected->idx;
	unsigned int end = GetAccumulateLimit(category);
	for (unsigned int i = GetGradeBegin_Grade(category); (int)i < (int)end; i++)
	{
		if (m_Tutorials[i].progress == 1)
		{
			m_Tutorials[i].progress = 0;
			return;
		}
	}
}

// 007d2980
void CTutorialMain::SetCompleteStates()
{
	CSharedDoc* doc = CSharedDoc::Instance();
	for (int category = 0; category < (int)m_categoryCount; category++)
	{
		int end = GetAccumulateLimit((unsigned char)category);
		for (int i = GetGradeBegin_Grade((unsigned char)category); i < end; i++)
		{
			m_Tutorials[i].state = 0;
			unsigned int flags;
			switch (category)
			{
			case 0:
				flags = doc->m_tutorialComplete[0];
				break;
			case 1:
				if (doc->m_tutorialComplete[0] != 0xff)
				{
					m_Tutorials[i].state = 2;
					continue;
				}
				flags = doc->m_tutorialComplete[1];
				break;
			case 2:
				if (doc->m_tutorialComplete[0] != 0xff || doc->m_tutorialComplete[1] != 0x3f00)
				{
					m_Tutorials[i].state = 2;
					continue;
				}
				flags = doc->m_tutorialComplete[2];
				break;
			default:
				continue;
			}
			if ((m_TutoCondition[i].mask & flags) != 0)
				m_Tutorials[i].state = 1;
		}
	}
}

// 007d2a90 (983): a flag da loja do bongdari mudou do CTaskMain (+0x122) para o CSharedDoc (+0x11410)
void CTutorialMain::OnTutorialAreaPipinOwnerDraw(int param)
{
	if (!CSharedDoc::Instance()->m_bBongdariShopOpen &&
		CRankingInfo::Instance()->GetDlg() == NULL &&
		(!CUserInfo::Instance() ||
			CUserInfo::Instance()->GetDlg() == NULL) &&
		m_pExhibition)
		m_pExhibition->Display(0.0f, 0.0f);
}

// 007d2ae0 (983): tudo relativo a posicao do item (pos), sem as alturas fixas do 645 pela linha do topo da barra de
// rolagem; a 2a serie perdeu a imagem 57 (presente ja pego) e o desenho final da imagem 50 saiu
void CTutorialMain::OnTutorialListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	float x = item->pos.x;
	float y = item->pos.y;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	switch (item->idx)
	{
	case 0:
	{
		gdi->DrawTexture(m_pBitmaps[45], WRect(item->pos.x, y, 30.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[46], WRect(item->pos.x + 30.0, y, 526.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[47], WRect(item->pos.x + 556.0, y, 30.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[58], WRect(item->pos.x + 12.0, y + 6.0, 153.0f, 51.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[61], WRect(item->pos.x + 380.0, y + 21.0, 194.0f, 19.0f), 0xffffffff, 0);
		int sel = IsInArea(item->idx, WPoint(g_input->GetMousePoint().x - x, g_input->GetMousePoint().y - y));
		for (int i = 0; i < 8; i++)
		{
			if (m_Tutorials[i].progress == 0)
			{
				const Bitmap* bmp = m_Tutorials[i].state == 2 ? m_pBitmaps[42] : m_pBitmaps[i];
				if (sel == -1 || sel != i || m_Tutorials[i].state == 2)
					gdi->DrawTexture(bmp, WRect(m_areaRects[i].x + x, y + m_areaRects[i].y, 85.0f, 48.0f),
						0xffffffff, 0);
				else
					gdi->DrawTexture(m_pBitmaps[14 + sel],
						WRect(m_areaRects[sel].x + x, m_areaRects[sel].y + y, 86.0f, 48.0f), 0xffffffff, 0);
			}
			else if (m_curQuest != 0xff)
			{
				gdi->DrawTexture(m_pBitmaps[28 + i], WRect(m_areaRects[i].x + x, y + m_areaRects[i].y, 85.0f, 48.0f),
					0xffffffff, 0);
			}
			if (m_Tutorials[i].state == 1)
				gdi->DrawTexture(m_pBitmaps[49],
					WRect(m_areaRects[i].x + x + 60.0, y + m_areaRects[i].y + 22.0, 19.0f, 19.0f), 0xffffffff, 0);
		}
		if (m_CompleteItems[0].image == 3)
		{
			gdi->DrawTexture(m_pBitmaps[52], WRect(item->pos.x + 496.0, y + 60.0, 79.0f, 97.0f), 0xffffffff, 0);
			gdi->DrawTexture(m_pBitmaps[53], WRect(item->pos.x + 496.0, y + 58.0 + m_iconOffset, 79.0f, 97.0f),
				0xffffffff, 0);
		}
		else
		{
			gdi->DrawTexture(m_pBitmaps[54], WRect(item->pos.x + 496.0, y + 73.0, 85.0f, 85.0f), 0xffffffff, 0);
		}
		float ax = 121.0f;
		float off = 0.0f;
		for (int a = 0; a < 6; a++)
		{
			if (m_ArrowItems[a].flag == 1)
				gdi->DrawTexture(m_pBitmaps[50], WRect(ax + x, off + y + 107.0, 10.0f, 10.0f), 0xffffffff, 0);
			else
				gdi->DrawTexture(m_pBitmaps[51], WRect(ax + x, off + y + 107.0, 10.0f, 10.0f), 0xffffffff, 0);
			off = -22.0f;
			if (a == 1)
			{
				ax += 89.0;
			}
			else if (a == 2)
			{
				off = 22.0f;
			}
			else
			{
				off = 0.0f;
				ax += 89.0;
			}
		}
		break;
	}
	case 1:
	{
		gdi->DrawTexture(m_pBitmaps[45], WRect(item->pos.x, y, 30.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[46], WRect(item->pos.x + 30.0, y, 526.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[47], WRect(item->pos.x + 556.0, y, 30.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[59], WRect(item->pos.x + 12.0, y + 6.0, 153.0f, 51.0f), 0xffffffff, 0);
		if (!IsLocalContent(S3_TUTORIAL_SECOND_GRADE))
		{
			gdi->DrawTexture(m_pBitmaps[63], WRect(item->pos.x + 380.0, y + 21.0, 194.0f, 19.0f), 0xffffffff, 0);
			gdi->DrawTexture(m_pBitmaps[48], WRect(x + 234.0, y + 67.0, 123.0f, 80.0f), 0xffffffff, 0);
			return;
		}
		gdi->DrawTexture(m_pBitmaps[62], WRect(item->pos.x + 380.0, y + 21.0, 194.0f, 19.0f), 0xffffffff, 0);
		const Bitmap* first = (int)CSharedDoc::Instance()->m_tutorialComplete[0] == 0xff ? m_pBitmaps[44] : m_pBitmaps[43];
		gdi->DrawTexture(first, WRect(x + 40.0, y + 91.0, 85.0f, 48.0f), 0xffffffff, 0);
		int sel = IsInArea(item->idx, WPoint(g_input->GetMousePoint().x - x, g_input->GetMousePoint().y - y));
		for (int i = 0; i < 6; i++)
		{
			if (m_Tutorials[8 + i].progress == 0)
			{
				const Bitmap* bmp = m_Tutorials[8 + i].state == 2 ? m_pBitmaps[42] : m_pBitmaps[8 + i];
				if (sel == -1 || sel != i || m_Tutorials[8 + i].state == 2)
					gdi->DrawTexture(bmp, WRect(m_areaRects2[i].x + x, y + m_areaRects2[i].y, 85.0f, 48.0f),
						0xffffffff, 0);
				else
					gdi->DrawTexture(m_pBitmaps[22 + i],
						WRect(m_areaRects2[i].x + x, y + m_areaRects2[i].y, 85.0f, 48.0f), 0xffffffff, 0);
			}
			else if (m_curQuest != 0xff)
			{
				gdi->DrawTexture(m_pBitmaps[36 + i],
					WRect(m_areaRects2[i].x + x, y + m_areaRects2[i].y, 85.0f, 48.0f), 0xffffffff, 0);
			}
			if (m_Tutorials[8 + i].state == 1)
				gdi->DrawTexture(m_pBitmaps[49],
					WRect(m_areaRects2[i].x + x + 60.0, y + m_areaRects2[i].y + 22.0, 19.0f, 19.0f), 0xffffffff, 0);
		}
		if (m_CompleteItems[1].image == 3)
		{
			gdi->DrawTexture(m_pBitmaps[52], WRect(item->pos.x + 496.0, y + 60.0, 79.0f, 97.0f), 0xffffffff, 0);
			gdi->DrawTexture(m_pBitmaps[53], WRect(item->pos.x + 496.0, y + 58.0 + m_iconOffset, 79.0f, 97.0f),
				0xffffffff, 0);
		}
		else
		{
			gdi->DrawTexture(m_pBitmaps[55], WRect(item->pos.x + 496.0, y + 73.0, 85.0f, 85.0f), 0xffffffff, 0);
		}
		float ax = 211.0f;
		float ay = y + 0.0 + 108.0;
		for (int a = 6; a < 10; a++)
		{
			if (m_ArrowItems[a].flag == 1)
				gdi->DrawTexture(m_pBitmaps[50], WRect(ax + x, ay, 10.0f, 10.0f), 0xffffffff, 0);
			else
				gdi->DrawTexture(m_pBitmaps[51], WRect(ax + x, ay, 10.0f, 10.0f), 0xffffffff, 0);
			ax += 89.0;
		}
		break;
	}
	case 2:
	{
		gdi->DrawTexture(m_pBitmaps[45], WRect(item->pos.x, y, 30.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[46], WRect(item->pos.x + 30.0, y, 526.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[47], WRect(item->pos.x + 556.0, y, 30.0f, 196.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[60], WRect(item->pos.x + 12.0, y + 6.0, 153.0f, 51.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[63], WRect(item->pos.x + 380.0, y + 21.0, 194.0f, 19.0f), 0xffffffff, 0);
		gdi->DrawTexture(m_pBitmaps[48], WRect(x + 234.0, y + 67.0, 123.0f, 80.0f), 0xffffffff, 0);
		break;
	}
	}
}

// 007d39f0 (983): presente da serie; ao fim esvazia o presente pendente do documento (+0x14)
void CTutorialMain::OpenGradeGift(unsigned char grade)
{
	FrCmdTarget* target = this ? static_cast<FrCmdTarget*>(this) : NULL;
	m_pQuestGift = CreateForm<FrQuestGift>(g_pFresh->GetManager(), target,
		"quest_gift", NULL);
	m_pQuestGift->SetData(grade);
	m_pQuestGift->Open(NULL, 2);
	m_pTutorialDoc983->m_pendingQuest = 0xff;
}

// 007d3a70 (983): o presente pendente da serie vem do documento 0xa4 (645: m_giftGrade, gravado pelo M983(0x153))
bool CTutorialMain::OnGiftDlgResult(int result, FrForm* form)
{
	if (result == 1)
	{
		unsigned char grade = m_pTutorialDoc983->m_pendingQuest;
		if (grade != 0xff)
			OpenGradeGift(grade);
	}
	return true;
}

// 007d3aa0
bool CTutorialMain::IsCompleteQuest(int category, int index)
{
	bool complete = false;
	if (m_Tutorials[index].type == category &&
		(m_TutoCondition[index].mask & GetQuestFlags(category)) != 0)
		complete = true;
	return complete;
}

// 007d3ae0
bool CTutorialMain::IsCompleteCategory(int category)
{
	int begin = 0;
	int end = 0;
	switch (category)
	{
	case 0:
		begin = 0;
		end = 8;
		break;
	case 1:
		begin = 8;
		end = 14;
		break;
	case 2:
		begin = 14;
		end = 20;
		break;
	}
	bool complete = true;
	if (begin < end)
	{
		const sTutoCondition* cond = &m_TutoCondition[begin];
		while ((cond->mask & GetQuestFlags(category)) != 0)
		{
			begin++;
			cond++;
			if (end <= begin)
				return true;
		}
		complete = false;
	}
	return complete;
}

// 007d3c30
void CTutorialMain::CheckNodeStates()
{
	for (int grade = 0; grade < (int)m_categoryCount; grade++)
	{
		if (!IsCompleteBeforeGrade((unsigned char)grade))
			return;
		int end = GetAccumulateLimit((unsigned char)grade);
		int i;
		for (i = GetGradeBegin_Grade((unsigned char)grade); i < end; i++)
			m_QuestInfo[i].count = 0;
		for (i = GetGradeBegin_Grade((unsigned char)grade); i < end; i++)
		{
			for (unsigned int j = 0; j < m_QuestInfo[i].children.size(); j++)
			{
				int same = 0;
				int child = m_QuestInfo[i].children[j];
				if (m_Tutorials[i].state == 1)
				{
					if (m_QuestInfo[child].count < m_QuestInfo[child].total)
						m_QuestInfo[child].count++;
				}
				else if (m_Tutorials[child].state == 0)
				{
					m_Tutorials[child].state = 2;
				}
				if (m_QuestInfo[child].total == m_QuestInfo[child].count &&
					m_Tutorials[child].state != 1)
					m_Tutorials[child].state = 0;
				if (m_QuestInfo[child].total > 1 &&
					m_Tutorials[child].state != 1)
				{
					for (int k = GetGradeBegin_Grade((unsigned char)grade); k < end; k++)
					{
						if (m_Tutorials[child].row == m_Tutorials[k].row)
							same++;
					}
					if (same > 2 && m_QuestInfo[child].count != 0)
						m_Tutorials[child].state = 0;
				}
			}
		}
		for (int c = 0; c < 3; c++)
			m_CompleteItems[c].image =
				IsCompleteCategory(m_CompleteItems[c].id) ? 4 : 3;
	}
}

// 007d3eb0 (983): licao do documento 0xa4; o resultado pode abrir o presente da serie (OnGiftDlgResult)
void CTutorialMain::OpenQuestGift()
{
	FrCmdTarget* target = this ? static_cast<FrCmdTarget*>(this) : NULL;
	m_pQuestGift = CreateForm<FrQuestGift>(g_pFresh->GetManager(), target,
		"quest_gift", NULL);
	m_pQuestGift->SetQuestExplane(m_pTutorialDoc983->m_index);
	m_pQuestGift->Open((FRESH_PFN_RESULT)&CTutorialMain::OnGiftDlgResult, 2);
}

// 007d4030 (+0x9c): a widget fechou (645 OnDestroy)
bool CTutorialMain::OnCloseWidget983()
{
	m_pipinTime.Reset();
	m_tutorialList.clear();
	FrTaskForm::OnCloseWidget983();
	return true;
}

// 007d40a0 (+0xa0): o que sobrou do HandleMsg do 645 (+4 mensagem, +8/+0xc parametros)
bool CTutorialMain::OnMsg983(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x14e):
		*(const char**)msg.param1 = s_szTaskClass;
		*(const char**)msg.param2 = s_szTaskName;
		break;
	case M983(0x151):
	{
		bool gift = *(bool*)&msg.param2;
		SetCompleteStates();
		CheckNodeStates();
		EnableAllBtn();
		DrawCourseArrow();
		if (gift)
			OpenQuestGift();
		break;
	}
	default:
		return false;
	}
	return true;
}

// 007d4110 (983): modo do documento 0xa4; deslocamentos do CSharedDoc do 983 (0x10594 = 0x10, nome em +0x148, bit em +0x5b)
void CTutorialMain::InitGolfTutorial()
{
	CSharedDoc::Instance()->m_tutorialStartMode = m_pTutorialDoc983->m_mode;
	CSharedDoc::Instance()->m_gameMode = 0;
	SetCurMap(0xfd);
	CSharedDoc::Instance()->m_golfGame.gameType = 0x10;
	strcpy(CSharedDoc::Instance()->m_golfGame.gameTypeName,
		CSharedDoc::Instance()->m_gameTypeInfo[0x10].name);
	for (int i = 0; i < 0x12; i++)
		CSharedDoc::Instance()->m_holeOrder[i] = 0;
	CSharedDoc::Instance()->m_golfGame.shotTimeLimit = 0;
	CSharedDoc::Instance()->m_golfGame.holes = 1;
	CSharedDoc::Instance()->m_holeOrder[0] = ((int)CSharedDoc::Instance()->m_tutorialStartMode >= 0xe) + 1;
	CSharedDoc::Instance()->m_golfGame.weather = 0;
	// cabeca da std::list m_slotList (1o dword no VC10 e no cl novo); bit 4 do no +0x5b
	int** head = *(int***)((char*)CSharedDoc::Instance() + offsetof(CSharedDoc, m_slotList));
	for (int** node = (int**)*head;
		node != *(int***)((char*)CSharedDoc::Instance() + offsetof(CSharedDoc, m_slotList));
		node = (int**)*node)
		*((unsigned char*)node + 0x5b) &= ~4;
	CSharedDoc::Instance()->m_bGameOver = 0;
}

// 007d4200
CTutorialMain::sTutorialInfo* CTutorialMain::FindTutorialList(int& id)
{
	std::list<sTutorialInfo>::iterator it = m_tutorialList.begin();
	for (; it != m_tutorialList.end(); ++it)
	{
		sTutorialInfo* info = &*it;
		if (info->id == id)
			return info;
	}
	return NULL;
}

// 007d4240
void CTutorialMain::CheckEnable(int index)
{
	const sTutoChild* child = &m_CheckTable[index];
	if (child->cond == 0)
	{
		for (int i = 0; i < child->count; i++)
		{
			int id = child->base + i;
			sTutorialInfo* info = FindTutorialList(id);
			if (!info)
				return;
			info->enabled = 1; // 983 007d428c: +4 libera a missao; +5 (done) so no MissionClear
		}
	}
	else
	{
		int done = 0;
		for (int i = 0; i < child->count; i++)
		{
			int id = child->base + i;
			sTutorialInfo* info = FindTutorialList(id);
			if (!info)
				return;
			if (info->done == 1)
				done++;
		}
		if (done == child->count)
		{
			int cond = child->cond;
			sTutorialInfo* info = FindTutorialList(cond);
			if (info)
				info->enabled = 1; // 983 007d42f3
		}
	}
}

CTutorialMain::CQuestInfo::CQuestInfo()
{
}
CTutorialMain::CQuestInfo::~CQuestInfo()
{
}

// 007d4380
CTutorialMain::~CTutorialMain()
{
	HidePet();
}

// 007d44b0 (983): o boneco fica no canto da janela (m_rect), nao em coordenadas fixas da tela
void CTutorialMain::ShowPet()
{
	HidePet();
	CExhibition* pet = new CExhibitionByHardwareTnL(m_rect.x + 75.0,
		m_rect.y + 100.0 + 170.0 + 15.0, 0.35f, false);
	if (pet)
	{
		pet->SetArea(150.0f, 340.0f);
		pet->SetDistanceRange(9.0f, 13.0f);
		pet->SetDistance(11.0f, true, false);
		bool ok = pet->SetModel("pipin", (eExhibitType)1);
		pet->SetControlFlag(0);
		pet->SetDirection(-0.45f);
		m_pExhibition = pet;
		m_pipinTime.m_name = "\xb1\xde\xc1\xa4\xb0\xc5";
		m_pipinTime.Reset();
		m_pipinTime.SetTime();
		if (!ok)
			delete pet;
	}
}

// 007d4630
void CTutorialMain::DecideNextMotion(unsigned char motion)
{
	switch (motion)
	{
	case 0:
		m_pipinTime.m_name = "\xb1\xe2\xba\xbb";
		break;
	case 1:
		m_pipinTime.m_name = "\xb1\xe2\xb4\xd9\xb8\xae\xb1\xe2";
		break;
	case 2:
		m_pipinTime.m_name = "\xc2\xf9\xbd\xba";
		break;
	case 3:
		m_pipinTime.m_name = "\xc0\xe7\xc3\xcb\xc7\xcf\xb1\xe2";
		break;
	case 4:
		m_pipinTime.m_name = "\xb1\xde\xc1\xa4\xb0\xc5";
		break;
	case 5:
		m_pipinTime.m_name = "\xc8\xa6\xc0\xce";
		break;
	case 6:
		m_pipinTime.m_name = "\xc8\xa6\xc0\xce\xbf\xf8";
		break;
	}
}

// 007d4700
void CTutorialMain::MissionClear(int& id, unsigned long& flags)
{
	sTutorialInfo* info = FindTutorialList(id);
	if (!info)
		return;
	if ((flags & m_TutoCondition[id].mask) != 0)
	{
		info->done = 1;
		CheckEnable(id);
	}
	if (id == 7 && (flags & 0x80) != 0)
		*(unsigned char*)&m_select = 1;
}

// 007d4750 (983): so a licao de golfe (os ramos de loja/my room do 645 sairam); ao fim fecha a janela
void CTutorialMain::StartSelectedTutorial()
{
	if (!m_pTutorialDoc983->IsValid983())
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xbf\xac\xbd\xc0\xb8\xf0\xb5\xe5 \xbc\xb3\xc1\xa4\xc0\xcc \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0));
		return;
	}
	if (m_pExhibition)
		HidePet();
	InitGolfTutorial();
	CMouseCursor::Instance()->SetMode(1);
	CMouseCursor::Instance()->SetActive(false);
	CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);
	FrTaskForm::Close(false);
}

// 007d4820
CTutorialMain::CTutorialMain()
{
	C_ASSERT_VC10(sizeof(CTutorialMain) == 0x460);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_pTutorialList) == 0x120);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_pExhibition) == 0x15c);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_pBitmaps) == 0x170);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_areaRects) == 0x270);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_select) == 0x2f4);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_QuestInfo) == 0x2f8);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_tutorialList) == 0x410);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_curQuest) == 0x41c);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_pipinTime) == 0x420);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_categoryCount) == 0x454);
	C_ASSERT_VC10(offsetof(CTutorialMain, m_pTutorialDoc983) == 0x458);
	m_pTutorialList = NULL;
	m_pInfoDlg = NULL;
	m_pQuestGift = NULL;
	m_pAreaPipin = NULL;
	m_pExhibition = NULL;
	m_cameraDir = 1.0f;
	m_bCameraReturn = false;
	m_lastTime = g_CurrentTime;
	m_areaRects[0] = WPoint(40.0f, 90.0f);
	m_areaRects[1] = WPoint(129.0f, 90.0f);
	m_areaRects[2] = WPoint(218.0f, 58.0f);
	m_areaRects[3] = WPoint(218.0f, 119.0f);
	m_areaRects[4] = WPoint(307.0f, 46.0f);
	m_areaRects[5] = WPoint(307.0f, 90.0f);
	m_areaRects[6] = WPoint(307.0f, 134.0f);
	m_areaRects[7] = WPoint(396.0f, 90.0f);
	m_areaRects2[0] = WPoint(129.0f, 91.0f);
	m_areaRects2[1] = WPoint(218.0f, 64.0f);
	m_areaRects2[2] = WPoint(218.0f, 117.0f);
	m_areaRects2[3] = WPoint(307.0f, 91.0f);
	m_areaRects2[4] = WPoint(396.0f, 64.0f);
	m_areaRects2[5] = WPoint(396.0f, 117.0f);
	m_arrowX[0] = 121.0f;
	m_arrowX[1] = 215.0f;
	m_arrowX[2] = 303.0f;
	m_arrowX[3] = 395.0f;
	m_arrowX[4] = 486.0f;
	m_curQuest = 0xff;
	m_reserved524 = 0;
	m_iconOffset = 0.0f;
	m_reserved52c = 0.0f;
	m_categoryCount = IsLocalContent(S3_TUTORIAL_SECOND_GRADE) + 1;
}

// 007d4ba0 (slot 32): arvore das licoes (645 OnInit) + o que o caso 0 do HandleMsg do 645 fazia depois do OpenLayout
bool CTutorialMain::OnInit()
{
	m_tutorialList.clear();
	for (int i = 0; i < 14; i++)
		m_Tutorials[i].state = 0;
	for (int grade = 0; grade < 3; grade++)
	{
		unsigned int end = GetAccumulateLimit((unsigned char)grade);
		for (unsigned int n = GetGradeBegin_Grade((unsigned char)grade);
			(int)n < (int)end; n++)
		{
			int parents = 0;
			int child = m_ChildTable[n].child;
			for (int c = 0; c < m_ChildTable[n].count; c++)
				m_QuestInfo[n].children.push_back(child++);
			for (unsigned int k = GetGradeBegin_Grade((unsigned char)grade);
				(int)k < (int)end; k++)
			{
				if (n != k)
				{
					if ((int)(m_Tutorials[n].row - m_Tutorials[k].row) < -1)
						break;
					if (m_Tutorials[n].row - 1 == m_Tutorials[k].row)
					{
						for (unsigned int j = 0;
							j < m_QuestInfo[k].children.size(); j++)
						{
							if (m_Tutorials[n].id == m_QuestInfo[k].children[j])
								parents++;
						}
					}
				}
			}
			m_QuestInfo[n].total = (unsigned char)parents;
			m_QuestInfo[n].count = 0;
		}
	}
	ShowPet();
	SetCompleteStates();
	CheckNodeStates();
	DrawCourseArrow();
	return FrForm::OnInit();
}

// 007d4e80 (slot 9; 645 OnProcess sem a camera do lobby e sem a base)
void CTutorialMain::OnProc(float dt)
{
	static float s_motionElapsed;
	static int s_idleElapsed;
	static int s_idleCheck;
	if (m_pExhibition)
	{
		m_pExhibition->Process(dt, false);
		w_motion_data* motion = m_pExhibition->GetMotionData();
		s_motionElapsed += dt;
		if (s_motionElapsed > motion->e - motion->s)
		{
			if (m_pipinTime.m_name.size() > 2)
			{
				if (strcmp(m_pipinTime.m_name.c_str(), motion->name) != 0)
				{
					m_pExhibition->SetMotion(m_pipinTime.m_name.c_str(), false,
						0.0f);
					s_motionElapsed = 0.0f;
					m_pipinTime.m_name = "";
				}
			}
		}
		s_idleElapsed = timeGetTime() - m_pipinTime.m_current;
		if (s_idleElapsed > 16000)
		{
			DecideNextMotion(3);
			m_pipinTime.m_current = timeGetTime();
			s_idleElapsed = 0;
		}
		else
		{
			s_idleCheck = timeGetTime() - m_pipinTime.m_start;
			if (s_idleCheck > 5000)
			{
				DecideNextMotion((unsigned char)(rand() % 3));
				m_pipinTime.m_start = timeGetTime();
			}
		}
	}
	ProcessAnimateIcon();
}

// 007d4ff0
void CTutorialMain::MissionComplete(unsigned long flags)
{
	int id = 0;
	do
	{
		int cur = id;
		if ((m_TutoCondition[id].mask & flags) != 0)
			MissionClear(id, flags);
		if (cur == 7 && (char)flags < 0)
			*(unsigned char*)&m_select = 1;
		id = cur + 1;
	} while (id < 8);
}

// 007d5040
bool CTutorialMain::OnTutorialInfoDlgResult(int result, FrForm* form)
{
	if (result != 0)
	{
		m_pInfoDlg = NULL;
		if (result == 1)
			StartSelectedTutorial();
		if (m_curQuest != 0xff)
		{
			m_Tutorials[m_curQuest].progress = 0;
			m_curQuest = 0xff;
		}
	}
	return true;
}

// 007d5200 (983): explicacao da licao do documento 0xa4
void CTutorialMain::OpenTutorialInfoDlg()
{
	if (!m_pInfoDlg)
	{
		m_pInfoDlg = CreateForm<FrQuestInfoDlg>(g_pFresh->GetManager(), this,
			"quest_info", NULL);
		m_pInfoDlg->SetQuestExplane(m_pTutorialDoc983->m_index);
		m_pInfoDlg->Open(
			(FRESH_PFN_RESULT)&CTutorialMain::OnTutorialInfoDlgResult, 3);
	}
}

// 007d5280
bool CTutorialMain::OnQuestionFormResult(int result, FrForm* form)
{
	if (result != 0)
	{
		m_pNotifyForm = NULL;
		if (result == 1)
			OpenTutorialInfoDlg();
		if (m_curQuest != 0xff)
		{
			m_Tutorials[m_curQuest].progress = 0;
			m_curQuest = 0xff;
		}
	}
	return true;
}

// 007d52d0 (983): avisos pelo K2L, abertos com a posicao padrao (sem o calculo de x do 645); o "pode mandar o 0xae"
// (645 m_reserved3cf) e o +0x15 do documento 0xa4
void CTutorialMain::OpenQuest(int category, int index)
{
	if (!m_Tutorials[index].enable)
		return;
	if (m_Tutorials[index].state == 2)
	{
		FrCmdTarget* target = this ? static_cast<FrCmdTarget*>(this) : NULL;
		m_pNotifyForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), target, "notify", NULL);
		m_pNotifyForm->SetMessage(K2L_Compatibility(
			"\xc0\xcc\xc0\xfc \xb5\xee\xb1\xde\xc0\xbb \xb8\xb6\xc4\xa3 \xc8\xc4 \xc7\xcf\xbd\xc7 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		m_pNotifyForm->Open(NULL, 3);
		m_pNotifyForm->EnableDrag(false);
		m_Tutorials[index].progress = 0;
		m_curQuest = 0xff;
		return;
	}
	SetQuestInformation(category, index);
	if (IsCompleteQuest(category, index))
	{
		m_pTutorialDoc983->m_bSendResult = false;
		m_pNotifyForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		m_pNotifyForm->SetMessage(K2L_Compatibility(
			"\xc0\xcc\xb9\xcc \xbf\xcf\xb7\xe1\xb5\xc8 \xc4\xf9\xbd\xba\xc6\xae\xc0\xcc\xb9\xc7\xb7\xce, \xc5\xac\xb8\xae\xbe\xee \xb1\xe2\xb3\xe4 \xbc\xb1\xb9\xb0\xc0\xbb \xb9\xde\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\xb1\xd7\xb7\xa1\xb5\xb5 \xb4\xd9\xbd\xc3 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			false);
		m_pNotifyForm->EnableDrag(false);
		m_pNotifyForm->Open(
			(FRESH_PFN_RESULT)&CTutorialMain::OnQuestionFormResult, 3);
		m_pNotifyForm->MoveCursor("ok");
		return;
	}
	m_pTutorialDoc983->m_bSendResult = true;
	OpenTutorialInfoDlg();
}

// 007d5460 (983): ponto relativo ao item (pos) em vez das alturas fixas do 645
void CTutorialMain::OnTutorialListLBtnDown()
{
	if (!m_pTutorialList)
		return;
	FrListItem* item = m_pTutorialList->GetItemUnderCursor();
	if (!item)
		return;
	if (IsLocalContent(S3_TUTORIAL_SECOND_GRADE) || item->idx < 1)
	{
		int sel = IsInArea(item->idx, WPoint(g_input->GetMousePoint().x - item->pos.x,
			g_input->GetMousePoint().y - item->pos.y));
		if (sel != -1)
		{
			if (item->idx == 1)
				sel += 8;
			if (m_Tutorials[sel].state != 2)
			{
				m_Tutorials[sel].progress = 1;
				m_curQuest = (unsigned char)sel;
			}
			OpenQuest(m_Tutorials[sel].type, sel);
		}
	}
}
