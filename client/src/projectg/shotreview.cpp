#include "minatl.h"
#include "shotreview.h"
#include "projectg.h"
#include "vc10_layout.h"
#include "golfdoc.h"
#include "golfball.h"
#include "inputmanager.h"
#include "shotpreview.h" // JP983_DEV_PREVIEW (Ctrl+R da conta GM)
#include "titlefontex.h"
#include "quadtree.h"
#include "commonutil.h"
#include "wresrcmng.h"
#include "woverlay.h"
#include "wview.h"
#include "shareddoc.h"
#include "soundmanager.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "../shared/globalgamedefine.h"
#include "mathconsts.h"
#include <algorithm>
#include <functional>
#include <math.h>

int __fastcall WisZero(const float& v, float e);
extern WMatrix g_camera; // 00f024a0 (983: g_camera, era stub zerado)

SpotHistory::SpotHistory()
{
	m_numberBase = 0;
	m_pFont = new CBarFont;
	m_pFont->SetColor(0xffffffff);

	m_pIndicator = g_resrcmng->GetOverlay("[fall_indicator1.png", 0);
	m_pWaterIndicator = g_resrcmng->GetOverlay("[fall_indicator2.png", 0);
}

SpotHistory::~SpotHistory()
{
	if (g_resrcmng && m_pWaterIndicator)
	{
		g_resrcmng->Release(m_pWaterIndicator);
		m_pWaterIndicator = NULL;
	}
	if (g_resrcmng && m_pIndicator)
	{
		g_resrcmng->Release(m_pIndicator);
		m_pIndicator = NULL;
	}
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = NULL;
	}
}

void SpotHistory::Clear()
{
	m_spots.clear();
}

void SpotHistory::Display()
{
	if (!(GOLFDOC()->m_gameMode & 0x200))
		return;

	if (m_spots.size() == 0)
		return;

	std::vector<float> alphas;
	alphas.reserve(m_spots.size());

	float alpha = 180.0f;
	for (unsigned int i = 0; i < m_spots.size(); ++i)
	{
		alphas.push_back(alpha);
		alpha -= 20.0f;
		if (0.0f > alpha)
			alpha = 0.0f;
		else
			alpha -= 20.0f;
	}
	std::reverse(alphas.begin(), alphas.end());

	int count = 0;
	for (std::list<std::pair<WVector, WVector> >::iterator it =
			 m_spots.begin();
		it != m_spots.end(); ++it)
	{
		unsigned long color = ((unsigned char)alphas[count] << 24) | 0xc8c8c8;

		// 983: a linha sai da posicao da tacada com a altura do pino (hole.pin.y) ate o ponto no chao
		WVector from = (*it).first;
		from.y = GOLFDOC()->GetHoleData().pin.y;
		int index;
		WVector to = GetPVS().GetGroundPoint((*it).second, true, &index, true);
		to.y = from.y; // 983 (008490b0, 008492f4..): tambem o ponto de chegada vai para a altura do pino (linha e indicador)
		g_view->DrawLine(from, color, to, color, 0x20300000, 2.0f);

		WTVertex vtx;
		g_view->Projection2(&vtx, to);

		WOverlay* pIndicator = index == -1 ? m_pWaterIndicator : m_pIndicator;
		int width = pIndicator->GetWidth();
		int height = pIndicator->GetHeight();
		{
			_WRECT dst = { vtx.x - (float)(width >> 1),
				vtx.y - (float)(height >> 1), (float)width, (float)height };
			_WRECT src = { 0.0f, 0.0f, 1.0f, 1.0f };
			pIndicator->Render(g_view, src, dst, 0, color, 0, 0);
		}

		++count;
		m_pFont->Print(vtx.x, vtx.y - m_pFont->GetFontH() * 0.5f,
			MakeStr("%d", m_numberBase + count), TFA_CENTER, 0.0f, 1.0f);
	}
}

void SpotHistory::AddItem(unsigned int param1, unsigned int param2)
{
	const WVector& from = *(const WVector*)param1;
	const WVector& to = *(const WVector*)param2;
	m_spots.push_back(std::pair<WVector, WVector>(from, to));
}

void SpotHistory::PopBack()
{
	if (!m_spots.empty())
		m_spots.pop_back();
}

void SpotHistory::PopFront()
{
	if (!m_spots.empty())
		m_spots.pop_front();
	++m_numberBase;
}

const std::pair<WVector, WVector>* SpotHistory::At(unsigned int index) const
{
	if (index >= m_spots.size())
		return NULL;
	std::list<std::pair<WVector, WVector> >::const_iterator it =
		m_spots.begin();
	std::advance(it, index);
	return &*it;
}

BallTraceHistory::BallTraceHistory()
{
	m_pIndicator = g_resrcmng->GetOverlay("[fall_indicator1.png", 0);
}

BallTraceHistory::~BallTraceHistory()
{
	if (g_resrcmng && m_pIndicator)
	{
		g_resrcmng->Release(m_pIndicator);
		m_pIndicator = NULL;
	}
}

void BallTraceHistory::Clear()
{
	m_traces.clear();
}

void BallTraceHistory::Display()
{
	if (!(GOLFDOC()->m_gameMode & 0x200))
		return;

	if (m_traces.empty())
		return;

	for (std::list<std::vector<WVector> >::iterator it = m_traces.begin();
		it != m_traces.end(); ++it)
	{
		std::vector<WVector>& trace = *it;
		if (trace.size() != 0)
		{
			float alpha = 100.0f;
			float step = 140.0f / trace.size();
			for (unsigned int i = 0; i < trace.size() - 1; ++i)
			{
				unsigned long color = ((unsigned char)alpha << 24) | 0xc81414;
				g_view->DrawLine(trace[i], color, trace[i + 1], color,
					0x20300000, 2.0f);
				alpha += step;
			}

			if (m_pIndicator)
			{
				WVector last = trace.back();
				WTVertex vtx;
				g_view->Projection2(&vtx, last);

				_WRECT src = { 0.0f, 0.0f, 1.0f, 1.0f };
				_WRECT dst = { vtx.x - 2.25f, vtx.y - 2.25f, 4.5f, 4.5f };
				m_pIndicator->Render(g_view, src, dst, 0, 0xffc81414, 0, 0);
			}
		}
	}
	g_view->Flush(0, 1);
}

void BallTraceHistory::AddItem(unsigned int param1, unsigned int param2)
{
	m_traces.push_back(*(const std::vector<WVector>*)param1);
}

void BallTraceHistory::PopBack()
{
	if (!m_traces.empty())
		m_traces.pop_back();
}

void BallTraceHistory::PopFront()
{
	if (!m_traces.empty())
		m_traces.pop_front();
}

FirstBoundHistory::FirstBoundHistory()
{
	m_pIndicator = g_resrcmng->GetOverlay("{indicator.bmp", 0);
	m_pFont = new CBarFont;
	m_pFont->SetColor(0xffffffff);
}

FirstBoundHistory::~FirstBoundHistory()
{
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = NULL;
	}
	if (g_resrcmng && m_pIndicator)
	{
		g_resrcmng->Release(m_pIndicator);
		m_pIndicator = NULL;
	}
}

void FirstBoundHistory::Clear()
{
	m_points.clear();
}

void FirstBoundHistory::Display()
{
	if (!(GOLFDOC()->m_gameMode & 0x200))
		return;

	for (std::list<std::pair<WVector, WVector> >::iterator it =
			 m_points.begin();
		it != m_points.end(); ++it)
		DrawIndicator(*it, 0xc8ff0000);
}

void FirstBoundHistory::AddItem(unsigned int param1, unsigned int param2)
{
	if (param1 == 0)
		m_points.push_back(*(const std::pair<WVector, WVector>*)param2);
}

void FirstBoundHistory::PopBack()
{
	if (!m_points.empty())
		m_points.pop_back();
}

void FirstBoundHistory::PopFront()
{
	if (!m_points.empty())
		m_points.pop_front();
}

void FirstBoundHistory::DrawIndicator(
	const std::pair<WVector, WVector>& bounce, unsigned long color)
{
	int index;
	WVector ground = GetPVS().GetGroundPoint(bounce.second, true, &index, true);

	WTVertex vtx;
	g_view->Projection2(&vtx, ground);

	ground.x = vtx.x;
	ground.y = vtx.y;
	{
		_WRECT dst = { ground.x - 10.0f, ground.y - 20.0f, 20.0f, 20.0f };
		_WRECT src = { 0.0f, 0.0f, 1.0f, 1.0f };
		m_pIndicator->Render(g_view, src, dst, 0, color, 0, 0);
	}

	WVector to = bounce.second;
	to.y = 0.0f;
	WVector from = bounce.first;
	from.y = 0.0f;
	float dist = (to - from).Magnitude();
	m_pFont->Print(ground.x, ground.y - 40.0f - 3.0f,
		MakeStr("%.1fy", dist * 0.3125f), TFA_CENTER, 0.0f, 1.0f);
}

IMPLEMENT_ACTOR(ShotReview, IActor)

__forceinline void ShotReview::ForEachHistory(void (IReviewHistory::*fn)())
{
	std::for_each(m_histories.begin(), m_histories.end(), std::mem_fun(fn));
}

// 983: o ShotReview e um IActor "solto" (sem CRenderFuncPtr/RegisterElement): o Display vem da lista de atores.
ShotReview::ShotReview()
{
	m_bVisible = true;
	m_bRecordBounce = true;
	m_bEnabled = false;
	m_maxHistory = 10;
	m_count = 0;
	m_bHistoryMode = false;
	m_bReviewCamera = false;
	m_reviewIndex = 0;
	m_pFrame = NULL;
	AddSkipList(0xa3000);

	m_dispPriority = DISP_PRIORITY_3;

	m_histories.push_back(new SpotHistory);
	m_histories.push_back(new BallTraceHistory);
	m_histories.push_back(new FirstBoundHistory);

	m_pFrame = g_resrcmng->GetOverlay("airnote_frame.tga", 0);
}

ShotReview::~ShotReview()
{
	std::for_each(m_histories.begin(), m_histories.end(), Delete_Object());

	if (g_resrcmng && m_pFrame)
	{
		g_resrcmng->Release(m_pFrame);
		m_pFrame = NULL;
	}
}

// 983 (00849d20): pega os dois botoes da GolfRule (MAIN.shot_review_button_pre/next), 0x2c6 = FindChildByName
void ShotReview::OnLoad()
{
	m_buttons.clear();

	FrWnd* found[2] = { NULL, NULL };
	SendMsg(this, "GolfRule", 0x2c6, (int)"shot_review_button_pre",
		(int)&found[0], 0, 0);
	SendMsg(this, "GolfRule", 0x2c6, (int)"shot_review_button_next",
		(int)&found[1], 0, 0);

	for (int i = 0; i < 2; ++i)
	{
		FrButton* button = found[i] ? DYNAMIC_CAST(FrButton, found[i]) : NULL;
		if (button != NULL)
		{
			*(unsigned long*)((char*)button + VC10_FRWND_STYLE) |= FWS_NODBLCLICK; // m_dwStyle (protegido; B1471: 0x40 no cl 14.44, +0x44 = m_wndText)
			button->SetPushDelay(0.0f);
			button->Enable(i == 0);
			m_buttons.push_back(button);
		}
	}

	RefreshButtons();
}

void ShotReview::OnInit()
{
	m_count = 0;
	ForEachHistory(&IReviewHistory::Clear);
}

// 0084afc0: fora da faixa 11..14 do realGameType: Ctrl+REPLAY liga/desliga, modo de historico e teclas da revisao
void ShotReview::OnProcess(float delta)
{
	if (!m_bEnabled)
		return;
	switch (Doc()->m_roomInfo.realGameType)
	{
	case 0xb: case 0xc: case 0xd: case 0xe:
		break;
	default:
#ifdef JP983_DEV_PREVIEW
		if (DevPreviewCtrlR983()) // conta GM: Ctrl+R e da previsao de tacada (shotpreview.cpp)
			;
		else
#endif
		if (g_input->Get("LCONTROL", true) && g_input->GetDown("REPLAY", true))
			m_bVisible = !m_bVisible;
		UpdateHistoryMode();
		OnReviewKeys();
		break;
	}
}

void ShotReview::OnDisplay()
{
	if (m_bVisible && m_bEnabled && GOLFDOC()->m_gameMode == 0x200)
	{
		if (GolfBall().m_groundType != 2 || m_bReviewCamera)
			ForEachHistory(&IReviewHistory::Display);

		// 008482e0: o 983 le m_buttons[0] sem testar a lista vazia e copia o retangulo do botao antes de deslocar
		if (m_pFrame && m_buttons[0] && (m_buttons[0]->GetStyle().GetFlag(FWS_VISIBLE)))
		{
			const WRect& rc = m_buttons[0]->GetRect();
			_WRECT dst = { rc.x, rc.y, rc.w, rc.h };
			dst.x -= 8.0f;
			dst.y -= 33.0f;
			dst.w = (float)m_pFrame->GetWidth();
			dst.h = (float)m_pFrame->GetHeight();
			_WRECT src = { 0.0f, 0.0f, 1.0f, 1.0f };
			m_pFrame->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
		}
	}
}

IReviewHistory* ShotReview::FindHistory(const char* name) const
{
	std::vector<IReviewHistory*>::const_iterator it = std::find_if(
		m_histories.begin(), m_histories.end(), ReviewHistoryFinder(name));
	return it != m_histories.end() ? *it : NULL;
}

__forceinline SpotHistory* ShotReview::FindSpotHistory() const
{
	return static_cast<SpotHistory*>(FindHistory("SpotHistory"));
}

// 00846760: quantas tacadas da lista de revisao valem (min(max, contador), >= 0)
int ShotReview::ReviewCount() const
{
	int count = m_count;
	if (count > m_maxHistory)
		count = m_maxHistory;
	if (count < 0)
		count = 0;
	return count;
}

// 00847310: mostra/esconde os botoes
void ShotReview::SetButtonsVisible(bool visible)
{
	for (unsigned int i = 0; i < m_buttons.size(); ++i)
		m_buttons[i]->SetVisible(visible);
}

// 00847360: habilita e posiciona os dois botoes no canto direito da tela
void ShotReview::RefreshButtons()
{
	int cx = (int)(g_view->GetWidth() - 67.0);
	for (unsigned int i = 0; i < m_buttons.size(); ++i)
	{
		if (m_buttons[i] == NULL)
			continue;
		m_buttons[i]->SetVisible(m_bHistoryMode);
		const WRect& rc = m_buttons[i]->GetRect();
		float w = rc.w;
		float h = rc.h;
		int offset = i != 0 ? 0x1c : -0x1c;
		WRect rect((float)(((float)cx - 14.0) + (float)offset - 7.0), 65.0f, w, h);
		m_buttons[i]->SetRect(rect);
	}
}

// 00847c30: liga/desliga o modo "historico" (botoes) conforme o modo de jogo (0x200 = revisao)
void ShotReview::UpdateHistoryMode()
{
	unsigned char type = Doc()->m_roomInfo.realGameType;
	if (type < 0xb || type > 0xe)
	{
		if (GOLFDOC()->m_gameMode & 0x200)
		{
			if (!m_bHistoryMode && m_count > 0)
			{
				SetButtonsVisible(true);
				m_bHistoryMode = true;
			}
		}
		else if (m_bHistoryMode == true)
		{
			SetButtonsVisible(false);
			m_bHistoryMode = false;
			m_buttons[0]->Enable(true);
			m_buttons[1]->Enable(false);
		}
	}
}

// 00847ce0: devolve a camera salva e avisa a GolfRule (0x2fe)
void ShotReview::ReleaseReviewCamera()
{
	if (m_bReviewCamera)
	{
		memcpy(&GOLFDOC()->GetCameraData(), m_savedCamera, sizeof(m_savedCamera));
		memcpy(&g_camera, m_savedMatrix, sizeof(m_savedMatrix));
		m_bReviewCamera = false;
		SendMsg(this, "GolfRule", 0x2fe, 0, 0, 0, 0);
	}
}

// 00849f80: aponta a camera para a tacada 'index' (caixa from..to) e guarda a camera anterior
void ShotReview::ApplyReviewCamera(int index)
{
	if (index > ReviewCount() - 1)
		return;

	SpotHistory* history = FindSpotHistory();
	const std::pair<WVector, WVector>* spot =
		history ? history->At(index) : NULL;
	if (spot == NULL)
		return;

	if (!m_bReviewCamera)
	{
		memcpy(m_savedCamera, &GOLFDOC()->GetCameraData(), sizeof(m_savedCamera));
		memcpy(m_savedMatrix, &g_camera, sizeof(m_savedMatrix));
		m_bReviewCamera = true;
	}

	const WVector& from = spot->first;
	const WVector& to = spot->second;

	WVector up(0.0f, -1.0f, 0.0f);
	WVector dir(to.x - from.x, to.y - from.y, to.z - from.z);
	WVector rawDir = dir;
	dir.Normalize();

	WVector right(up.z * dir.y - up.y * dir.z, up.x * dir.z - dir.x * up.z,
		dir.x * up.y - up.x * dir.y);

	float cx = (to.x + from.x) * 0.5f;
	float cy = (to.y + from.y) * 0.5f;
	float cz = (to.z + from.z) * 0.5f;
	float length = (float)sqrt((double)(rawDir.z * rawDir.z +
		rawDir.x * rawDir.x + rawDir.y * rawDir.y));

	WMatrix& cam = g_camera;
	cam.xa = right;
	cam.ya = dir;
	cam.za = up;
	cam.pivot = WVector(cx, (float)(length * 1.5) + cy, cz);

	sCameraData& data = GOLFDOC()->GetCameraData();
	data.target = from;
	data.angle = CalcDeltaAngle(rawDir, WVector(0.0f, 0.0f, 1.0f));
	WVector n = rawDir;
	n.Normalize();
	data.mat.za = n;
}

// 0084a520: anterior/proxima tacada
void ShotReview::StepReview(bool next)
{
	if (m_count == 0)
		return;

	if ((!next ? m_buttons[0] : m_buttons[1])->IsEnabled())
	{
		int count = ReviewCount();
		if (next)
			++m_reviewIndex;
		else
			--m_reviewIndex;

		if (m_reviewIndex > count - 1)
		{
			m_reviewIndex = count - 1;
			if (m_bReviewCamera == true)
			{
				ReleaseReviewCamera();
				m_buttons[0]->Enable(true);
				m_buttons[1]->Enable(false);
				return;
			}
		}
		if (m_reviewIndex < 0)
			m_reviewIndex = 0;
		if (m_reviewIndex == 0)
		{
			m_buttons[0]->Enable(false);
			m_buttons[1]->Enable(true);
		}
		if (m_reviewIndex < ReviewCount() - 1)
			m_buttons[1]->Enable(true);
		if (m_reviewIndex > 0)
			m_buttons[0]->Enable(true);

		ApplyReviewCamera(m_reviewIndex);
	}
}

// 0084a2d0: entra na revisao pela ultima tacada
void ShotReview::SelectLastReview()
{
	m_reviewIndex = ReviewCount() - 1;
	if (m_reviewIndex < 1)
	{
		m_reviewIndex = 0;
		m_buttons[0]->Enable(false);
	}
	m_buttons[1]->Enable(true);
	ApplyReviewCamera(m_reviewIndex);
}

// 0084ae40: teclas REVIEW_PREV/REVIEW_NEXT
static bool IsReviewGameMode()
{
	int mode = (int)GOLFDOC()->m_gameMode;
	return mode == 8 || mode == 0x200;
}

void ShotReview::OnReviewKeys()
{
	if (!IsReviewGameMode())
		return;

	bool pressed = false;
	if (m_bReviewCamera)
	{
		if (g_input->GetDown("REVIEW_PREV", true))
		{
			StepReview(false);
			pressed = true;
		}
		if (g_input->GetDown("REVIEW_NEXT", true))
		{
			StepReview(true);
			pressed = true;
		}
	}
	else
	{
		if (!g_input->GetDown("REVIEW_PREV", true))
			return;
		if (GOLFDOC()->m_gameMode == 8)
			SendMsg(this, "GolfRule", 0x2ff, 0, 0, 0, 0);
		SelectLastReview();
		pressed = true;
	}

	if (pressed)
		g_audio->PlaySfx("ui_icon_click");
}

void ShotReview::HandleMsg(const MsgObject& msg)
{
	if (msg.message != 0x2fd)
		return;

	std::string command((const char*)msg.param1);

	if (command == "RESET_REVIEW")
	{
		m_count = 0;
		ForEachHistory(&IReviewHistory::Clear);
	}
	else if (command == "SET_MAX_HIS")
	{
		m_maxHistory = msg.param2;
	}
	else if (command == "ENABLE")
	{
		m_bEnabled = msg.param2 != 0;
	}
	else if (command == "GET_VISIBLE_STATE")
	{
		*(bool*)msg.param2 = m_bVisible;
	}
	else if (command == "POP_BACK_ITEM")
	{
		ForEachHistory(&IReviewHistory::PopBack);
		--m_count;
	}
	else if (command == "VISIBLE")
	{
		m_bVisible = msg.param2 != 0;
	}
	else if (command == "RESET_UI")
	{
		RefreshButtons();
	}
	else if (command == "SET_REVIEW_CAMERA")
	{
		bool next = msg.param2 != 0;
		if (!m_bReviewCamera)
		{
			if (!next)
			{
				if (GOLFDOC()->m_gameMode == 8)
					SendMsg(this, "GolfRule", 0x2ff, 0, 0, 0, 0);
				SelectLastReview();
			}
		}
		else
		{
			StepReview(next);
		}
	}
	else if (command == "RELEASE_REVIEW_CAMERA")
	{
		ReleaseReviewCamera();
	}
	else if (command == "IS_REVIEW_CAMERA_MODE")
	{
		*(bool*)msg.param2 = m_bReviewCamera;
	}
	else if (command == "IS_MOUSE_OVER_REVIEW_BUTTON")
	{
		WVector mouse = g_input->GetMousePoint();
		bool over = false;
		for (unsigned int i = 0; i < m_buttons.size(); ++i)
		{
			FrButton* button = m_buttons[i];
			if (button->GetStyle().GetFlag(FWS_VISIBLE))
			{
				WRect rect = button->GetRect();
				WPoint point(mouse.x, mouse.y);
				if (rect.IsInRect(point))
					over = true;
			}
		}
		*(bool*)msg.param2 = over;
	}
	else if (command == "SAVE_SPOT_INDICATOR")
	{
		IReviewHistory* history = FindHistory("SpotHistory");
		if (history)
		{
			WVector ballPos = GolfBall().m_pos;
			WVector spot;
			SendMsg(this, "Spot", M983(0xf4), (int)&spot, 0, 0, 0);
			history->AddItem((unsigned int)&ballPos, (unsigned int)&spot);
		}
		++m_count;
		if (m_maxHistory < m_count)
			ForEachHistory(&IReviewHistory::PopFront);
	}
	else if (command == "RECORD_REAL_BOUNCE_BALL")
	{
		if (m_bRecordBounce)
		{
			m_bRecordBounce = false;

			WVector realPos = *(const WVector*)msg.param2;
			std::pair<WVector, WVector> bounce;
			SpotHistory* spots = FindSpotHistory();
			if (spots)
			{
				const std::pair<WVector, WVector>* last =
					spots->At(ReviewCount() - 1);
				if (last == NULL)
					return;
				bounce.first = last->first;
			}
			IReviewHistory* history = FindHistory("FirstBoundHistory");
			if (history)
			{
				bounce.second = realPos;
				history->AddItem(0, (unsigned int)&bounce);
			}
		}
	}
	else if (command == "SAVE_BALL_TRACE")
	{
		m_bRecordBounce = true;

		const CGolfBall* pBall = (const CGolfBall*)msg.param2;
		std::vector<const CGolfBall::sExpected*> frames;
		for (int i = 0; i <= pBall->m_renderNum; ++i)
			frames.push_back(pBall->PreData(i));

		// Thin the trace down: drop every other frame without an event until
		// only the limit is left.
		unsigned int limit = 50;
		while (frames.size() > limit)
		{
			unsigned int count = 0;
			std::vector<const CGolfBall::sExpected*>::iterator it =
				frames.begin();
			while (it != frames.end())
			{
				++count;
				if (count % 2 == 0)
				{
					if ((*it)->event > 0)
					{
						++limit;
					}
					else
					{
						it = frames.erase(it);
						continue;
					}
				}
				++it;
			}
		}

		if (!frames.empty())
		{
			std::vector<WVector> trace;
			for (std::vector<const CGolfBall::sExpected*>::iterator it =
					 frames.begin();
				it != frames.end(); ++it)
			{
				int index;
				WVector point =
					GetPVS().GetGroundPoint((*it)->pos, true, &index, true);
				GetPVS().GetGroundType(point, true, NULL);
				trace.push_back(point);
			}

			IReviewHistory* history = FindHistory("BallTraceHistory");
			if (history)
				history->AddItem((unsigned int)&trace, 0);
		}
	}
}
