#include "minatl.h"
#include "vc10_layout.h"
#include "letter_effect.h"
#include "projectg.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "contentsdoc.h"
#include "capturedbg.h"
#include "headicon.h"
#include "inputmanager.h"
#include "commonutil.h"
#include "../shared/localize.h"
#include "wview.h"

extern WMatrix g_camera; // 00f024a0
extern const _WRECT g_rectUnit983_e4296c; // 00e4296c ({0, 0, 1, 1})
extern const WVector g_vZero983_f041c4; // 00f041c4

// CFxBox::Finished (0096d7f0) só é visível na fxbox.cpp: visão com o nome no jp.tsv
class CFxBoxJp983 : public CFxBox
{
public:
	bool Finished();
};

// HACK: unused; references g_EPSILON so it is emitted in .rdata as in the
// original
static bool IsZero(float f)
{
	return f < g_EPSILON;
}

IMPLEMENT_ACTOR(CLetter, IActor)

CLetter::CLetter()
	: m_pCurrent(NULL), m_texts(32, 16), m_pangyaTexts983(32, 16)
{
	m_pFont = NULL;
	m_pFastForward = NULL;
	m_pReplay = NULL;
	m_pSrankDeco983 = NULL;
	m_pSrankDeco3983 = NULL;
	m_srankFade983 = 0;
	m_srank983 = 0;
	m_bSrankFaded983 = false;
	m_srankAlpha983 = 0.0f;
	m_dispPriority = DISP_PRIORITY_4;
	AddSkipList(0x2000);
	// 983 00760f10: câmera própria do logo S-rank e a CPvsFxBox +0xac
	m_srankCamera983 = WMatrix::IDENTITY;
	m_fxBox983.Init(g_resrcmng);
	m_fxBox983.SetCamera(&g_camera);
	m_fxBox983.m_frameTime = 33.333332f;
}

void CLetter::OnPreLoadInit()
{
	m_fadeTime = 0.5f;
	m_bShow = true;
	m_replayTime = 0.0f;
}

void CLetter::OnInit()
{
	// 983 0075ec60: +0x538, +0x60, +0x6c, +0x530, +0x52c, +0x534
	m_srankAlpha983 = 0;
	m_state = 0;
	m_align = 2;
	m_srank983 = 0;
	m_srankFade983 = 0;
	m_bSrankFaded983 = false;
}

void CLetter::OnDestroy()
{
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}

	for (sText* pText = m_texts.Start(); pText; pText = m_texts.Next())
		delete pText;

	m_texts.Reset();

	for (sText* pText = m_pangyaTexts983.Start(); pText; pText = m_pangyaTexts983.Next())
		delete pText;

	m_pangyaTexts983.Reset();

	// 983 0075f3d0: texturas do S-rank e a CPvsFxBox
	if (g_resrcmng)
	{
		if (m_pSrankDeco983)
		{
			g_resrcmng->Release(m_pSrankDeco983);
			m_pSrankDeco983 = NULL;
		}
		if (g_resrcmng && m_pSrankDeco3983)
		{
			g_resrcmng->Release(m_pSrankDeco3983);
			m_pSrankDeco3983 = NULL;
		}
	}
	m_fxBox983.ClearAll(false);
}

// HACK: defined before OnLoad (the original line info puts it at the end of
// the file) so that it is emitted right after OnDestroy, as in the original
void CLetter::AddOverlay(const char* name, const char* file, WRect* rect,
	bool bWaitInput, WList<sText*>* pList)
{
	if (pList == NULL)
		pList = &m_texts; // 983 0075f550: a lista vem no 5o parametro (+0x1c textos, +0x3c PangYa!)

	if (pList->Find(name)) // 983 0075f550: nome repetido nao entra de novo
		return;

	sText* pText = new sText;

	pText->pOverlay = (WOverlay*)g_resrcmng->GetOverlay(file, 0);
	if (pText->pOverlay == NULL)
	{
		delete pText;
		return;
	}

	strcpy(pText->name, name);
	pText->life = 0;

	float texWidth = (float)pText->pOverlay->GetWidth();
	float texHeight = (float)pText->pOverlay->GetHeight();

	// 983 0075f550: fdiv pelo tamanho da textura (nao multiplica pelo inverso)
	if (rect)
	{
		pText->src.x = rect->x / texWidth;
		pText->src.y = rect->y / texHeight;
		pText->src.w = rect->w / texWidth;
		pText->src.h = rect->h / texHeight;
	}
	else
	{
		pText->src = WRect(0.0f, 0.0f, 1.0f, 1.0f);
	}

	pText->dst.w = texWidth * pText->src.w;
	pText->dst.h = texHeight * pText->src.h;
	pText->dst.x =
		(float)(int)(g_view->GetWidth() * 0.5f - pText->dst.w * 0.5f);
	pText->dst.y =
		(float)(int)(g_view->GetHeight() * 0.25f - pText->dst.h * 0.5f);
	pText->bWaitInput = bWaitInput;
	pText->number.bShow = false;

	pList->AddItem(pText, pText->name, false);
}

// JP 983 (0075fe70): "\xc6\xce\xbe\xdf%02d" = "[text_pangya%02d.jpg" do ClubSet.iff do taco (o que o GetIndexPangyaLogoImage manda no 0x1fe)
void CLetter::AddPangyaOverlay983(unsigned long clubTid)
{
	char name[16], file[32];
	unsigned int kind = CSharedDoc::Instance()->GetClubSetKind983(clubTid);
	sprintf(name, "\xc6\xce\xbe\xdf%02d", kind);
	sprintf(file, "[text_pangya%02d.jpg", kind);
	AddOverlay(name, file, NULL, false, &m_pangyaTexts983);
	ScalePangya983(m_pangyaTexts983.Find(name)); // 0075f9c0
}

// 983 (0075f9c0 e 0x271b do HandleMsg): PangYa! cresce com a tela, clamp(altura * 0.00125, 1, 2)
float CLetter::GetPangyaScale983()
{
	float scale = g_view->GetHeight() * 0.00125f;
	if (scale > 2.0f)
		scale = 2.0f;
	else if (scale < 1.0f)
		scale = 1.0f;
	return scale;
}

void CLetter::ScalePangya983(sText* pText)
{
	if (pText == NULL)
		return;

	float scale = GetPangyaScale983();
	pText->dst.w = (float)(int)(pText->pOverlay->GetWidth() * pText->src.w * scale);
	pText->dst.h = (float)(int)(pText->pOverlay->GetHeight() * pText->src.h * scale);
	pText->dst.x =
		(float)(int)(g_view->GetWidth() * 0.5f - pText->dst.w * 0.5f);
	pText->dst.y =
		(float)(int)(g_view->GetHeight() * 0.25f - pText->dst.h * 0.5f);
}

void CLetter::OnLoad()
{
	// A175: 983 (0075ff40) carrega "[text_pangya00.jpg" e o texto do taco de cada jogador (0075fe70, A449).
	// Lista propria em +0x3c + escala 0075f9c0 (A450); texturas S-rank +0xa4/+0xa8 (A451).
	AddOverlay("\xc6\xce\xbe\xdf"
			   "00",
		"[text_pangya00.jpg", NULL, false, &m_pangyaTexts983);
	ScalePangya983(m_pangyaTexts983.Find("\xc6\xce\xbe\xdf"
										 "00"));
	m_pSrankDeco983 = (WOverlay*)g_resrcmng->GetOverlay("[text_srankdeco1.jpg", 0);
	m_pSrankDeco3983 = (WOverlay*)g_resrcmng->GetOverlay("[S_Rank_deco_3.jpg", 0);
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
		AddPangyaOverlay983(CSharedDoc::Instance()->m_userInfo[i].clubInfo.tid);
	AddOverlay("OB", "[ob.jpg", NULL, false);

	// 983 0075ff40: so Match (gameType 1) tem win/lose/draw; letreiros de placar e "chance" so fora do Match
	if (CSharedDoc::Instance()->m_golfGame.gameType == 1)
	{
		AddOverlay("win", "[hole_end.jpg", &WRect(0.0f, 384.0f, 256.0f, 64.0f),
			false);
		AddOverlay("lose", "[hole_end.jpg",
			&WRect(256.0f, 384.0f, 256.0f, 64.0f), false);
		AddOverlay("draw", "[hole_end.jpg", &WRect(0.0f, 448.0f, 256.0f, 64.0f),
			false);
	}
	else
	{
		AddOverlay("\xbe\xcb\xb9\xd9\xc6\xae\xb7\xce\xbd\xba", "[hole_end.jpg",
			&WRect(0.0f, 0.0f, 512.0f, 64.0f), false);
		AddOverlay("\xc0\xcc\xb1\xdb", "[hole_end.jpg",
			&WRect(0.0f, 64.0f, 256.0f, 64.0f), false);
		AddOverlay("\xb9\xf6\xb5\xf0", "[hole_end.jpg",
			&WRect(256.0f, 64.0f, 256.0f, 64.0f), false);
		AddOverlay("\xc6\xc4", "[hole_end.jpg", &WRect(0.0f, 128.0f, 256.0f, 64.0f),
			false);
		AddOverlay("\xba\xb8\xb1\xe2", "[hole_end.jpg",
			&WRect(256.0f, 128.0f, 256.0f, 64.0f), false);
		AddOverlay("\xb4\xf5\xba\xed\xba\xb8\xb1\xe2", "[hole_end.jpg",
			&WRect(0.0f, 192.0f, 512.0f, 64.0f), false);
		AddOverlay("\xc6\xae\xb8\xae\xc7\xc3\xba\xb8\xb1\xe2", "[hole_end.jpg",
			&WRect(0.0f, 256.0f, 512.0f, 64.0f), false);
		AddOverlay("+4", "[hole_end.jpg", &WRect(0.0f, 320.0f, 64.0f, 64.0f),
			false);
		AddOverlay("+5", "[hole_end.jpg", &WRect(64.0f, 320.0f, 64.0f, 64.0f),
			false);
		AddOverlay("+6", "[hole_end.jpg", &WRect(128.0f, 320.0f, 64.0f, 64.0f),
			false);
		AddOverlay("+7", "[hole_end.jpg", &WRect(192.0f, 320.0f, 64.0f, 64.0f),
			false);
		AddOverlay("+8", "[hole_end.jpg", &WRect(256.0f, 320.0f, 64.0f, 64.0f),
			false);
		AddOverlay("+9", "[hole_end.jpg", &WRect(320.0f, 320.0f, 64.0f, 64.0f),
			false);
		AddOverlay("+10", "[hole_end.jpg", &WRect(384.0f, 320.0f, 128.0f, 64.0f),
			false);
		AddOverlay("\xb9\xf6\xb5\xf0\xc2\xf9\xbd\xba", "[birdie_shot.jpg", NULL,
			false);
		AddOverlay("\xc0\xcc\xb1\xdb\xc2\xf9\xbd\xba", "[eagle_shot.jpg", NULL,
			false);
		AddOverlay("\xbe\xcb\xb9\xd9\xc6\xae\xb7\xce\xbd\xba\xc2\xf9\xbd\xba",
			"[albatross_shot.jpg", NULL, false);
	}

	AddOverlay("\xbf\xf6\xc5\xcd\xc7\xd8\xc0\xfa\xb5\xe5", "[shot_end.jpg",
		&WRect(0.0f, 0.0f, 512.0f, 64.0f), false);
	AddOverlay("\xc6\xf7\xb1\xe2", "[shot_end.jpg",
		&WRect(0.0f, 64.0f, 256.0f, 64.0f), false);
	AddOverlay("\xc8\xa6\xc0\xce\xbf\xf8", "[text_holeinone.jpg", NULL, false);
	AddOverlay("\xc5\xb8\xc0\xd3\xbe\xc6\xbf\xf4", "[timeout.jpg", NULL, false);
	AddOverlay("winningshot", "[for_win1.jpg", NULL, false);
	AddOverlay("matchshot", "[for_win2.jpg", NULL, false);
	AddOverlay("winningshot2", "[for_win1_2.jpg", NULL, false);
	AddOverlay("matchshot2", "[for_win2_2.jpg", NULL, false);
	AddOverlay("winningshot3", "[for_win1_3.jpg", NULL, false);
	AddOverlay("matchshot3", "[for_win2_2.jpg", NULL, false);
	AddOverlay("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
			   "30",
		"[approach30.jpg", NULL, true);
	AddOverlay("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
			   "60",
		"[approach60.jpg", NULL, true);
	AddOverlay("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
			   "30_\xbf\xac\xc0\xe5",
		"[approach30_extra.jpg", NULL, true);
	SetPos("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
		   "30_\xbf\xac\xc0\xe5",
		g_view->GetWidth() * 0.5f, g_view->GetHeight() * 0.25f + 64.0f);
	AddOverlay("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
			   "60_\xbf\xac\xc0\xe5",
		"[approach60_extra.jpg", NULL, true);
	SetPos("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
		   "60_\xbf\xac\xc0\xe5",
		g_view->GetWidth() * 0.5f, g_view->GetHeight() * 0.25f + 64.0f);
	AddOverlay("OneShot", "[oneshot.jpg", NULL, true);
	SetPos("OneShot", g_view->GetWidth() * 0.5f,
		g_view->GetHeight() * 0.25f + 42.0f);
	AddOverlay("\xc2\xf7\xb7\xca\xc7\xa5\xbd\xc3", "[play.jpg", NULL, true);
	AddOverlay("NoPlay", "[noplay.jpg", NULL, false);
	AddOverlay("Retry", "[noplay2.jpg", NULL, false);
	AddOverlay("Wait", "[wait.jpg", NULL, false);
	SetPos("Wait", g_view->GetWidth() * 0.5f, g_view->GetHeight() * 0.18f);
	AddOverlay("AI", "[ai.jpg", NULL, false);
	SetPos("AI", g_view->GetWidth() * 0.5f, g_view->GetHeight() * 0.18f);
	AddOverlay("Replay", "[replay-icon.jpg", NULL, false);
	SetPos("Replay", 45.0f, 60.0f);
	AddOverlay("FastForward", "[forward-icon.jpg", NULL, false);
	SetPos("FastForward", 45.0f, 60.0f);
	AddOverlay("Demo", "[demo.png", NULL, false);
	SetPos("Demo", g_view->GetWidth() * 0.5f, g_view->GetHeight() * 0.18f);

	m_pFont = g_resrcmng->GetTitleFont();

	char* filename = "[bigfont.jpg";
	m_fontInfo.filename = &filename;
	m_fontInfo.flag = 0;
	m_fontInfo.fonth = 32;
	m_fontInfo.fontw = 32;
	m_fontInfo.numPages = 1;
	m_fontInfo.texw = 256;
	m_fontInfo.texh = 128;
	m_fontInfo.pCharSet = "1234567890ym%-./x ";
	m_pFont->Create(&m_fontInfo);
}

void CLetter::Print(float x, float y, const char* text, float space, int type,
	float scale, unsigned long color)
{
	const char* p = text;
	if (p == NULL || strlen(p) == 0)
		return;

	const char* charset = m_fontInfo.pCharSet;
	m_pFont->SetScale(scale);

	float width = 0.0f;

	switch (m_align)
	{
	case 1:
		while (*p)
		{
			int c = *p;
			p++;
			int idx = strchr(charset, c) - charset;
			width += m_pFont->GetCharWidth(g_view, idx) + space * scale;
		}
		p = text;
		width = (width - space) * 0.5f;
		break;

	case 2:
		while (*p)
		{
			int c = *p;
			p++;
			int idx = strchr(charset, c) - charset;
			width += m_pFont->GetCharWidth(g_view, idx) + space * scale;
		}
		p = text;
		width = width - space;
		break;
	}

	x -= width;

	while (*p)
	{
		int c = *p;
		p++;
		int idx = strchr(charset, c) - charset;
		float w = m_pFont->PutChar(g_view, x, y, idx, type, color);
		w += space;
		x += w;
	}
}

void CLetter::SetPos(const char* name, float x, float y)
{
	sText* pText = m_texts.Find(name);
	if (pText)
	{
		pText->dst.x = (float)(int)(x - pText->dst.w * 0.5f);
		pText->dst.y = (float)(int)(y - pText->dst.h * 0.5f);
	}
}

void CLetter::OnProcess(float delta)
{
	m_replayTime += delta;

	if (m_pCurrent == NULL)
		return;

	if (CCapturedBg::Instance()->IsCaptured())
		return;

	if (GOLFDOC()->m_gameMode & 0x8210)
		m_state = 0;

	switch (m_state)
	{
	case 1:
		m_alpha += delta / m_fadeTime * 255.0f;
		if (m_alpha > 255.0f)
			m_state = 2;
		break;

	case 2:
		if (m_pCurrent->life > 0.0f)
		{
			m_pCurrent->life -= delta;
			if (m_pCurrent->life < 0.0f)
			{
				m_srankFade983 = 3; // 983 0075f720
				m_state = 3;
			}
			else if (m_pCurrent->life < 1.0f && m_srank983 == 1 && !m_bSrankFaded983)
			{
				m_srankFade983 = 1; // 983: o enfeite S-rank entra no último segundo
				m_bSrankFaded983 = true;
			}
		}

		if (m_pCurrent->bWaitInput && g_input->GetList())
		{
			m_srankFade983 = 3;
			m_state = 3;
		}
		break;

	case 3:
		m_alpha -= delta / m_fadeTime * 255.0f;
		if (m_alpha < 0.0f)
		{
			m_alpha = 0;
			m_state = 0;
		}
		break;
	}

	float alpha = m_alpha;
	if (alpha < 0.0f)
		alpha = 0.0f;
	else if (alpha > 255.0f)
		alpha = 255.0f;
	m_alpha = alpha;

	// 983 0075f720: fade do [S_Rank_deco_3 e a sequência pangya_logo.seq
	if (m_srankFade983 == 1)
	{
		m_srankAlpha983 += delta / m_fadeTime * 255.0f;
		if (m_srankAlpha983 > 255.0f)
			m_srankAlpha983 = 255.0f;
	}
	else if (m_srankFade983 == 3)
	{
		m_srankAlpha983 -= delta / m_fadeTime * 255.0f;
		if (m_srankAlpha983 < 0.0f)
		{
			m_srankAlpha983 = 0;
			m_srankFade983 = 0;
			m_bSrankFaded983 = false;
		}
	}

	if (m_pSrankDeco983 && m_srank983 == 1)
	{
		if (((CFxBoxJp983*)&m_fxBox983)->Finished())
		{
			m_fxBox983.ClearAll(false);
			m_srank983 = 0;
			m_srankFade983 = 0;
			return;
		}

		float ms = delta * 1000.0f;
		m_fxBox983.ProcessAttacher(ms);
		m_fxBox983.Process(ms);
	}
}

void CLetter::DisplayReplay()
{
	if (!m_bShow)
		return;

	static bool s_bBlink = false;
	static bool s_bVisible = false;

	if (m_replayTime > 1.0f)
	{
		s_bBlink ^= true;
		m_replayTime = 0.5f;
		s_bVisible = true;
	}

	if (s_bBlink)
	{
		if (s_bVisible && m_pReplay)
			m_pReplay->pOverlay->Render(g_view, m_pReplay->src, m_pReplay->dst,
				0, 0xffffffff, 0.0f, 0);
	}
	else if (s_bVisible)
	{
		s_bVisible = false;
	}
}

void CLetter::OnDisplay()
{
	if (CCapturedBg::Instance()->IsCaptured())
		return;

	if (CHeadIcon::Instance())
		CHeadIcon::Instance()->DisplayOverPlayer();

	DisplayReplay();

	if (m_pFastForward)
	{
		if (m_pReplay)
			m_pFastForward->dst.x = 88.0f;
		else
			m_pFastForward->dst.x = 13.0f;

		m_pFastForward->pOverlay->Render(g_view, m_pFastForward->src,
			m_pFastForward->dst, 0, 0xffffffff, 0.0f, 0);
	}

	if (!m_bShow || m_state == 0 || m_pCurrent == NULL)
		return;

	if (GOLFDOC()->m_gameMode & 0x20400)
		return;

	if (m_srank983 == 1) // 983 0075fab0: enfeites S-rank e o logo 3D antes do texto
	{
		if (m_pSrankDeco983)
			m_pSrankDeco983->Render(g_view, g_rectUnit983_e4296c, m_srankRect983, 0,
				((int)m_alpha << 24) + 0xffffff, 0.0f, 0);
		if (m_pSrankDeco3983)
			m_pSrankDeco3983->Render(g_view, g_rectUnit983_e4296c, m_srankRect983, 0,
				((int)m_srankAlpha983 << 24) + 0xffffff, 0.0f, 0);

		if (((CFxBoxJp983*)&m_fxBox983)->Finished())
			return;

		bool bFog = g_view->SetFogEnable(false);
		g_view->SetCamera(m_srankCamera983);
		float oldNear = g_view->clip_near;
		WView::PROJECTION_MODE mode = g_view->GetProjectionMode();
		float oldFar = g_view->clip_far;
		float oldFov = g_view->GetFOV();
		g_view->SetProjectionMode((WView::PROJECTION_MODE)0);
		g_view->SetFOV(1.5707964f);
		g_view->SetClip(0.5f, 50.0f, false);
		g_view->UpdateCamera();
		m_fxBox983.Display(g_view);
		g_view->Flush(1, 1);
		g_view->SetProjectionMode(mode);
		g_view->SetClip(oldNear, oldFar, false);
		g_view->SetFOV(oldFov);
		g_view->ResetClippingArea();
		g_view->ResetScreenCenter();
		g_view->UpdateCamera();
		g_view->SetFogEnable(bFog);
	}

	m_pCurrent->pOverlay->Render(g_view, m_pCurrent->src, m_pCurrent->dst, 0,
		((int)m_alpha << 24) + 0xffffff, 0.0f, 0);

	if (m_pCurrent->number.bShow)
	{
		// 983 0075fd77: centralizado (m_align = 1 so aqui), x = w/2 - 40, y = h/4 + 4, sem espacamento
		int oldAlign = m_align;
		m_align = 1;
		float height = g_view->GetHeight();
		float width = g_view->GetWidth();
		Print(width * 0.5f - 40.0f, height * 0.25f + 4.0f,
			MakeStr("%d", m_pCurrent->number.value), 0.0f, 0,
			m_pCurrent->number.scale, m_pCurrent->number.color);
		m_align = oldAlign;
	}
}

// 983 00760840 (0x1fe com param3 == 3): logo pangya_logo.seq na câmera própria e os enfeites S-rank
bool CLetter::StartSrank983()
{
	CFxSequence* pSeq = m_fxBox983.LoadSequence("pangya_logo.seq", true, NULL);
	if (pSeq == NULL)
		return false;

	float height = g_view->SCREEN_YS;
	float scale = (float)(height * (double)0.00125f);
	m_srankCamera983.pivot = m_srankCamera983.za * -19.0f;
	float dist = (float)(height / 400.0 * 6.0);
	m_srankCamera983.pivot -= m_srankCamera983.za * dist;
	if (scale > 2.0f)
		scale = 2.0f;
	else if (scale < 1.0f)
		scale = 1.0f;

	WVector2D center = g_view->GetScreenCenter();
	pSeq->m_attachX = center.x;
	pSeq->m_attachY = center.y;
	pSeq->m_pos = g_vZero983_f041c4;
	m_fxBox983.AddSequence(pSeq);
	double offY = scale * 20.0;
	m_fxBox983.SetScreenCenter(center.x, (float)(center.y * 0.5 + offY));

	float texW = (float)m_pSrankDeco983->GetWidth();
	float texH = (float)m_pSrankDeco983->GetHeight();
	m_srankRect983.x = (float)(int)(float)(g_view->SCREEN_XS * 0.5 - texW * scale * 0.5);
	m_srankRect983.y = (float)(int)(float)(g_view->SCREEN_YS * 0.25 - texH * scale * 0.5 + offY);
	m_srankRect983.w = texW * scale;
	m_srankRect983.h = texH * scale;
	m_srank983 = 1;
	return true;
}

void CLetter::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}

	switch (msg.message)
	{
	case M983(0x192):
		m_pReplay = m_texts.Find("Replay");
		m_replayTime = 0.5f;
		break;

	case M983(0x193):
		m_pFastForward = m_texts.Find("FastForward");
		break;

	case M983(0x18c):
		if (m_pCurrent && msg.param1)
		{
			if (strstr(m_pCurrent->name, (const char*)msg.param1))
				m_pCurrent = NULL;
		}
		else
		{
			m_pCurrent = NULL;
			m_pReplay = NULL;
			m_pFastForward = NULL;
		}
		break;

	case M983(0x18f):
		m_state = 3;
		break;

	case M983(0x18d):
		m_pCurrent = m_texts.Find((const char*)msg.param1);
		if (m_pCurrent)
			m_srank983 = 0;
		else
		{
			// 983 00760840: se nao achou nos textos, procura nos PangYa! (+0x3c)
			m_pCurrent = m_pangyaTexts983.Find((const char*)msg.param1);
			if (m_pCurrent == NULL)
				break;
			if (msg.param3 == 3 && !StartSrank983())
				break;
		}

		if (msg.param2)
			m_pCurrent->life = *(float*)msg.param2;
		else
			m_pCurrent->life = 0;

		if (msg.param3 == 1) // 983 00760840: so o 1 entra sem fade (o PangYa! do 983 manda 2 ou 3)
		{
			m_alpha = 255.0f;
			m_state = 2;
		}
		else
		{
			m_alpha = 0;
			m_state = 1;
		}
		break;

	case M983(0x18e):
		m_state = 1;
		break;

	case M983(0x191):
		m_pCurrent = m_texts.Find((const char*)msg.param1);
		if (m_pCurrent == NULL)
			break;

		m_pCurrent->number = *(sNumber*)msg.param2;
		m_pCurrent->number.bShow = true;
		break;

	case M983(0x190):
		m_bShow = msg.param1 == 1;
		break;

	case 0x205: // 983 (00760c57): numero novo, sem par no 645; param1 = tid do taco
		AddPangyaOverlay983(msg.param1);
		break;

	case 10011:
	{
		for (sText* pText = m_texts.Start(); pText; pText = m_texts.Next())
		{
			pText->dst.x =
				(float)(int)(g_view->GetWidth() * 0.5f - pText->dst.w * 0.5f);
			pText->dst.y =
				(float)(int)(g_view->GetHeight() * 0.25f - pText->dst.h * 0.5f);
		}

		for (sText* pText = m_pangyaTexts983.Start(); pText; pText = m_pangyaTexts983.Next())
			ScalePangya983(pText); // 983 00760840 (0x271b): PangYa! refaz a escala com a tela nova

		SetPos("Wait", g_view->GetWidth() * 0.5f, g_view->GetHeight() * 0.18f);
		SetPos("AI", g_view->GetWidth() * 0.5f, g_view->GetHeight() * 0.18f);
		SetPos("Replay", 45.0f, 60.0f);
		SetPos("FastForward", 45.0f, 60.0f);
	}
	break;
	}
}
