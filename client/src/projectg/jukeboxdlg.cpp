#include "minatl.h"
#include "jukeboxdlg.h"
#include "fredit.h"
#include "frbutton.h"
#include "frlistbox.h"
#include "frscrollbar.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "woverlay.h"
#include "soundmanager.h"
#include "inputmanager.h"
#include "clientsetting.h"
#include "jp/lua983.h"
#include "jp/bgmplaylist.h"

extern CSoundManager* g_audio;
extern Fresh* g_pFresh;
extern const WRect g_rectUnitW983; // 00e4296c (retangulo de origem padrao)

// leitura por indice do lua_table_helper do 983: o texto sai por `call` (004d5ba0), definicao em dummyplayeractor.cpp
struct LuaIdx983 : lua_system::lua_table_helper
{
	void Push983(int index);
	template <typename T>
	T GetByIndex(int index);
};
template <>
const char* LuaIdx983::GetByIndex<const char*>(int index); // 004d5ba0
#define LUAIDX983(tbl) (*(LuaIdx983*)&(tbl))

// a barra de rolagem da lista e lida direto em +0x90 (o 983 nao chama GetScrollBar)
// D875: pelo membro (cl 14.44 m_pScrBar = 0x84: std::list/std::string menores no FrWnd)
struct FrListBoxScroll983 : public FrListBox
{
	using FrWnd::m_pScrBar;
};
C_ASSERT_VC10(offsetof(FrListBoxScroll983, m_pScrBar) == 0x90);
static inline FrScrollBar* ListScrollBar(FrListBox* list)
{
	return ((FrListBoxScroll983*)list)->m_pScrBar;
}

IMPLEMENT_OBJECT(FrJukeBoxDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrJukeBoxDlg, FrForm)

// D642: o 983 (msg map 00e1a168) reage no LBUTTONDOWN em todos os botoes e na lista (o 645 era LBUTTONUP)
ON_FRESH_VI("jukeboxbtn", FRCMD_INIT, FrJukeBoxDlg::OnBtnInit)
ON_FRESH_VV("jukeboxbtn", FRCMD_LBUTTONDOWN, FrJukeBoxDlg::OnBtnUp)
ON_FRESH_VI("jukeboxplay", FRCMD_INIT, FrJukeBoxDlg::OnPlayInit)
ON_FRESH_VV("jukeboxplay", FRCMD_LBUTTONDOWN, FrJukeBoxDlg::OnPlayBtnUp)
ON_FRESH_VV("jukeboxclose", FRCMD_LBUTTONDOWN, FrJukeBoxDlg::OnCloseBtnUp)
ON_FRESH_VI("jukeboxprev", FRCMD_INIT, FrJukeBoxDlg::OnPrevInit)
ON_FRESH_VV("jukeboxprev", FRCMD_LBUTTONDOWN, FrJukeBoxDlg::OnPrevBtnUp)
ON_FRESH_VI("jukeboxnext", FRCMD_INIT, FrJukeBoxDlg::OnNextInit)
ON_FRESH_VV("jukeboxnext", FRCMD_LBUTTONDOWN, FrJukeBoxDlg::OnNextBtnUp)
ON_FRESH_VI("jukeboxlist", FRCMD_INIT, FrJukeBoxDlg::OnListInit)
ON_FRESH_VI("jukeboxlist", FRCMD_OWNERDRAW, FrJukeBoxDlg::OnListOwnerDraw)
ON_FRESH_VV("jukeboxlist", FRCMD_LBUTTONDOWN, FrJukeBoxDlg::OnListBtnUp)
ON_FRESH_VI("jukeboxframe_1", FRCMD_INIT, FrJukeBoxDlg::OnFrameInit)
ON_FRESH_VI("jukeboxframe_1", FRCMD_OWNERDRAW, FrJukeBoxDlg::OnFrameOwnerDraw)

END_FRESH_MSGMAP()

bool FrJukeBoxDlg::m_preserve_isPlaying = true;

int FrJukeBoxDlg::m_maxCount;

int FrJukeBoxDlg::m_curBGM;

std::vector<sBGMInfo983> FrJukeBoxDlg::m_bgmList;

// 007ba760
FrJukeBoxDlg::FrJukeBoxDlg()
{
	m_fTime = 0.0f;
	m_pList = NULL;
	m_pPlay = NULL;
	CBGMPlayList983::Instance()->m_pJukeBox = this;
}

// 007ba400
FrJukeBoxDlg::~FrJukeBoxDlg()
{
	CBGMPlayList983::Instance()->m_pJukeBox = NULL;
	if (g_resrcmng && m_pLight)
	{
		g_resrcmng->Release(m_pLight);
		m_pLight = NULL;
	}
}

// 007bbab0
void FrJukeBoxDlg::LoadBGMList()
{
	if (m_bgmList.size() == 0)
	{
		lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
		if (script)
			script->RunFile("BGMlist.lua");
		lua_system::lua_table_helper count = script->GetTable("max_count_bgm_list");
		m_maxCount = LUA983_GET(count, int, "max_count");
		lua_system::lua_table_helper bgm = script->GetTable("bgm_list");
		if (0 < m_maxCount * 2)
		{
			int i = 2;
			do
			{
				sBGMInfo983 info;
				// B547: o texto e do estado lua, que DeleteScript fecha logo abaixo -> copia (ponteiro solto nunca casava com a BGM tocando)
				const char* f = LUAIDX983(bgm).GetByIndex<const char*>(i - 1);
				const char* t = LUAIDX983(bgm).GetByIndex<const char*>(i);
				info.file = _strdup(f ? f : "");
				info.title = _strdup(t ? t : "");
				m_bgmList.push_back(info);
			} while (i < m_maxCount * 2 && (i += 2));
		}
		lua_system::lua_script_manger::GetInstance()->DeleteScript(script);
	}
}

// 007bbce0
void __fastcall FrJukeBoxDlg::PlayWithMap(unsigned char map)
{
	LoadBGMList();

	std::vector<int> list;

	switch (map)
	{
	case 4:
	case 5:
		list.push_back(0);
		list.push_back(1);
		break;
	case 7:
		list.push_back(2);
		list.push_back(3);
		break;
	case 8:
		list.push_back(4);
		list.push_back(5);
		break;
	case 9:
		list.push_back(6);
		list.push_back(7);
		break;
	case 10:
		list.push_back(8);
		list.push_back(9);
		break;
	case 13:
		list.push_back(10);
		list.push_back(11);
		break;
	case 14:
		list.push_back(17);
		list.push_back(18);
		break;
	case 15:
		list.push_back(19);
		list.push_back(20);
		break;
	case 16:
		list.push_back(22);
		list.push_back(23);
		break;
	case 18:
		list.push_back(26);
		list.push_back(27);
		break;
	case 19:
		list.push_back(24);
		list.push_back(25);
		break;
	case 20:
		list.push_back(30);
		list.push_back(31);
		break;
	case 21:
		list.push_back(32);
		list.push_back(33);
		break;
	default:
		list.push_back(12);
		list.push_back(13);
		list.push_back(14);
		list.push_back(15);
		list.push_back(34);
		break;
	}

	if (list.size())
	{
		g_audio->StopBGM(true);
		m_curBGM = list[rand() % list.size()];
		g_audio->PlayBGM(m_bgmList[m_curBGM].file, false, true);
	}
}

// 007ba810
void FrJukeBoxDlg::SelectBGMByName(const char* name)
{
	for (int i = 0; i < m_maxCount; i++)
	{
		if (!_stricmp(name, m_bgmList[i].file))
		{
			m_curBGM = i;
			break;
		}
	}
	if (m_maxCount <= m_curBGM)
		m_curBGM = 0;
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
// 007bc1e0
bool FrJukeBoxDlg::OnInit()
{
	LoadBGMList();
	m_curBGM = rand() % m_maxCount;
	m_pLight = g_resrcmng->GetOverlay("jukebox_light.tga", 0);
	LoadBitmaps();
	InitLightRect();
	for (int i = 0; i <= m_maxCount; i++)
	{
		m_pList->AddItem((void*)i);
		if (i != m_maxCount && g_audio->IsPlaying(m_bgmList[i].file))
			m_curBGM = i;
	}
	m_bAnim = !g_audio->IsBGMStopped983(); // 983 007bc1e0: byte +0x75 do CSoundManager
	if (m_curBGM > 9)
		ListScrollBar(m_pList)->SetCurTopRow(m_curBGM - 9);

	bool bPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);
	for (int k = 0; k < m_maxCount; k++) jp983_rastro("B547 lista %d %s | %s", k, m_bgmList[k].file, m_bgmList[k].title);
	jp983_rastro("B543 Jukebox OnInit cur=%d file=%s playing=%d bgmName=%s", m_curBGM, m_bgmList[m_curBGM].file, (int)bPlaying, g_audio->GetBGMName());
	const char* name = bPlaying ? "btn_pause" : "btn_play";
	m_pPlay->SetButtonImg(MakeStr("%s_n", name), FrButton::NORMAL);
	m_pPlay->SetButtonImg(MakeStr("%s_o", name), FrButton::OVER);
	m_pPlay->SetButtonImg(MakeStr("%s_o", name), FrButton::PRESSED);

	EnableDrag(false);
	return FrForm::OnInit();
}

// 007ba4d0
void FrJukeBoxDlg::LoadBitmaps()
{
	m_pBar[0] = g_pFresh->GetBitmap("jukebox_btm_list_Push_1");
	m_pBar[1] = g_pFresh->GetBitmap("jukebox_btm_list_Push_2");
	m_pBar[2] = g_pFresh->GetBitmap("jukebox_btm_list_Hover_1");
	m_pBar[3] = g_pFresh->GetBitmap("jukebox_btm_list_Hover_2");
}

// 007bad60
void FrJukeBoxDlg::InitLightRect()
{
	const Bitmap* light = g_pFresh->GetBitmap("jukebox_light");
	m_lightRect.x = (float)(m_rect.x + 117.5);
	m_lightRect.y = (float)(m_rect.y - 144.5);
	m_lightRect.w = (float)light->Width();
	m_lightRect.h = (float)light->Height();
}

// 007ba4c0
void FrJukeBoxDlg::OnBtnUp()
{
	Close(true);
}

// 007ba4b0
void FrJukeBoxDlg::OnCloseBtnUp()
{
	Close(true);
}

// 007ba880
void FrJukeBoxDlg::OnBtnInit(int param)
{
	m_pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (g_pFresh->IsCurrentLayout("TOPPAGE"))
	{
		m_pBtn->SetPushDelay(0.0f);
		m_pBtn->UseDblClick(false);
	}
	m_pBtn->SetVisible(false);
}

// 007ba930
void FrJukeBoxDlg::OnPlayInit(int param)
{
	m_pPlay = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pPlay->SetPushDelay(0.0f);
	m_pPlay->UseDblClick(false);
}

// 007ba9c0
void FrJukeBoxDlg::OnNextInit(int param)
{
	m_pNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pNext->SetPushDelay(0.0f);
	m_pNext->UseDblClick(false);
}

// 007baa50
void FrJukeBoxDlg::OnPrevInit(int param)
{
	m_pPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pPrev->SetPushDelay(0.0f);
	m_pPrev->UseDblClick(false);
}

// 007bacf0
void FrJukeBoxDlg::OnListInit(int param)
{
	m_pList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

// 007bafd0
void FrJukeBoxDlg::OnFrameInit(int param)
{
	m_pFrame = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 007ba530
void FrJukeBoxDlg::OnFrameOwnerDraw(int)
{
	if (m_bAnim)
		m_pLight->Render(g_view, g_rectUnitW983, m_lightRect, 0, 0xffffffff, m_fTime * 2.0f, 0);
}

// 007bb1b0
void FrJukeBoxDlg::OnPlayBtnUp()
{
	bool bPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);

	if (bPlaying)
	{
		g_audio->PauseBGM(false);
		m_bAnim = false;
		CBGMPlayList983::Instance()->Clear983();
	}
	else
	{
		CBGMPlayList983::Instance()->SetBGM983(m_bgmList[m_curBGM].file);
		m_bAnim = true;
	}

	OnChangePlayPauseButton();

	AfxGetTask()->GetActor("AvatarChat")
		<< MsgObject(NULL, M983(0x69), !bPlaying, 0, 0, 0, 0);
}

// 007baae0
void FrJukeBoxDlg::OnPrevBtnUp()
{
	CBGMPlayList983::Instance()->m_bJukeBoxMode = false;
	bool bPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);

	m_curBGM--;
	if (m_curBGM < 0)
		m_curBGM = 1;

	if (bPlaying)
	{
		g_audio->StopBGM(true);
		g_audio->PlayBGM(m_bgmList[m_curBGM].file, false, true);
	}

	int top = (int)ListScrollBar(m_pList)->GetCurTopRow();
	if (m_curBGM - top == -1 || m_curBGM - top > 9)
		ListScrollBar(m_pList)->SetCurTopRow(m_curBGM);
}

// 007bab90
void FrJukeBoxDlg::OnNextBtnUp()
{
	CBGMPlayList983::Instance()->m_bJukeBoxMode = false;
	bool bPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);

	m_curBGM++;
	if ((unsigned int)m_curBGM > 1)
	{
		m_curBGM = 0;
		ListScrollBar(m_pList)->ScrollToFirst();
	}

	if (bPlaying)
	{
		g_audio->StopBGM(true);
		g_audio->PlayBGM(m_bgmList[m_curBGM].file, false, true);
	}

	if (m_curBGM != 0)
	{
		int top = m_curBGM - 9;
		if (top < 0)
			ListScrollBar(m_pList)->SetCurTopRow(0);
		else
			ListScrollBar(m_pList)->SetCurTopRow(top);
	}
}

// 007bac60
void FrJukeBoxDlg::OnChangePlayPauseButton()
{
	bool bPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);

	if (m_pPlay)
	{
		const char* name;
		if (bPlaying)
			name = "btn_pause";
		else
			name = "btn_play";

		m_pPlay->SetButtonImg(MakeStr("%s_n", name), FrButton::NORMAL);
		m_pPlay->SetButtonImg(MakeStr("%s_o", name), FrButton::OVER);
		m_pPlay->SetButtonImg(MakeStr("%s_o", name), FrButton::PRESSED);
	}
}

// 007badd0
void FrJukeBoxDlg::DrawPlayingBar(FrListItem* item)
{
	g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pBar[0],
		WRect((float)(item->pos.x - 15.0), (float)(item->pos.y + 1.0),
			(float)m_pBar[0]->Width(), (float)m_pBar[0]->Height()),
		0xffffffff, 0);
	g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pBar[1],
		WRect((float)(item->pos.x + 241.0), (float)(item->pos.y + 1.0),
			(float)m_pBar[1]->Width(), (float)m_pBar[1]->Height()),
		0xffffffff, 0);
}

// 007baed0
void FrJukeBoxDlg::DrawHoverBar(FrListItem* item)
{
	g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pBar[2],
		WRect((float)(item->pos.x - 15.0), (float)(item->pos.y + 1.0),
			(float)m_pBar[2]->Width(), (float)m_pBar[2]->Height()),
		0xffffffff, 0);
	g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pBar[3],
		WRect((float)(item->pos.x + 241.0), (float)(item->pos.y + 1.0),
			(float)m_pBar[3]->Width(), (float)m_pBar[3]->Height()),
		0xffffffff, 0);
}

// 007bb290
void FrJukeBoxDlg::OnListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (item)
	{
		FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
		unsigned long oldColor = gdi->GetTextColor();
		gdi->SetTextStyle(1);

		bool bPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);
		if (m_curBGM == item->idx && bPlaying)
			DrawPlayingBar(item);
		else if (m_pList->GetItemUnderCursor() == item)
			DrawHoverBar(item);

		if (item->idx == m_maxCount)
		{
			if (CBGMPlayList983::Instance()->m_bJukeBoxMode)
				g_pFresh->GetManager()->GetGDI()->SetTextColor(0xff0000, 0xffffffff);
			g_pFresh->GetManager()->PrintText(
				WPoint((float)(item->pos.x + 25.0), (float)(item->pos.y + 8.0)), 0,
				"(Random)", 300.0f, 0xff00ffff);
		}
		else
		{
			g_pFresh->GetManager()->PrintText(
				WPoint((float)(item->pos.x + 25.0), (float)(item->pos.y + 8.0)), 0,
				m_bgmList[item->idx].title, 300.0f, 0xff00ffff);
		}

		g_pFresh->GetManager()->GetGDI()->SetTextColor(oldColor, 0xffffffff);
		g_pFresh->GetManager()->GetGDI()->SetTextStyle(0);
	}
}

// 007bb6a0
void FrJukeBoxDlg::OnListBtnUp()
{
	FrListItem* item = m_pList->GetItemUnderCursor();
	if (item)
	{
		if (item->idx == m_maxCount)
		{
			if (!CBGMPlayList983::Instance()->m_bJukeBoxMode)
			{
				ShufflePlay();
				CBGMPlayList983::Instance()->m_bJukeBoxMode = true;
				m_bAnim = true;
				OnChangePlayPauseButton();
				return;
			}
			CBGMPlayList983::Instance()->m_bJukeBoxMode = false;
			CBGMPlayList983::Instance()->SetBGM983(m_bgmList[m_curBGM].file);
			return;
		}

		m_curBGM = item->idx;
		CBGMPlayList983::Instance()->m_bJukeBoxMode = false;
		bool bPlaying = g_audio->IsPlaying(m_bgmList[m_curBGM].file);
		if (bPlaying)
		{
			g_audio->PauseBGM(false);
			m_bAnim = false;
			CBGMPlayList983::Instance()->Clear983();
		}
		else
		{
			CBGMPlayList983::Instance()->SetBGM983(m_bgmList[m_curBGM].file);
			m_bAnim = true;
		}

		OnChangePlayPauseButton();

		AfxGetTask()->GetActor("AvatarChat")
			<< MsgObject(NULL, M983(0x69), !bPlaying, 0, 0, 0, 0);
	}
}

// 007bb400
void FrJukeBoxDlg::ShufflePlay()
{
	int* order = new int[m_maxCount];
	int i;
	for (i = 0; i < m_maxCount; i++)
		order[i] = i;
	for (i = 0; i < m_maxCount; i++)
	{
		int r = rand() % m_maxCount;
		m_curBGM = r;
		int tmp = order[i];
		order[i] = order[r];
		order[r] = tmp;
	}
	CBGMPlayList983::Instance()->Clear983();
	for (i = 0; i < m_maxCount; i++)
		CBGMPlayList983::Instance()->Add983(m_bgmList[order[i]].file);
	CBGMPlayList983::Instance()->Play983();
	if (order)
		delete[] order;
}

// 007bb4f0
void FrJukeBoxDlg::OnProc(const float delta)
{
	if (m_bAnim)
		m_fTime += delta;

	if (g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", false))
	{
		OnPlayBtnUp();
		g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}

	if (g_input->GetDown("\xbb\xf3", false))
	{
		OnPrevBtnUp();
	}
	else if (g_input->GetDown("\xc7\xcf", false))
	{
		OnNextBtnUp();
	}

	if (g_input->Get("\xc1\xc2", false))
	{
		COption::Instance()->aSetBgmVolume(Between(0.0f,
			(float)(COption::Instance()->aGetBgmVolume() - delta * 0.5), 1.0f));
		g_audio->SetBGMVolume(COption::Instance()->aGetBgmVolume());
	}
	else if (g_input->Get("\xbf\xec", false))
	{
		COption::Instance()->aSetBgmVolume(Between(0.0f,
			(float)(delta * 0.5 + COption::Instance()->aGetBgmVolume()), 1.0f));
		g_audio->SetBGMVolume(COption::Instance()->aGetBgmVolume());
	}
}
