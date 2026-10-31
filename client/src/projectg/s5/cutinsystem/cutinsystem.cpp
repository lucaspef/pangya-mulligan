#include "minatl.h"
#include "cutinsystem.h"
#include "projectg.h"
#include "clientsetting.h"

int WisZero(const float& v, float e);

namespace
{
	bool IsScalingCutInStyle(eCUTINANISTLYE style)
	{
		return style == CUTIN_ZOOM_IN || style == CUTIN_ZOOM_OUT;
	}

	bool IsAlphaCutInStyle(eCUTINANISTLYE style)
	{
		return style == CUTIN_FADE_IN || style == CUTIN_FADE_OUT;
	}

	bool IsRotateCutInStyle(eCUTINANISTLYE style)
	{
		return style == CUTIN_ROTATE;
	}
}

const float CUTIN_DELAY = 0.1f;
const float CUTIN_MAX_ALPHA = 255.0f;
const float CUTIN_MIN_ALPHA = 0.0f;

static float s_fMaxRotate = g_PI * 8.0f;

#define RELEASE_CUTIN_OVERLAY(p) \
	if (g_resrcmng) \
	{ \
		if (p) \
		{ \
			g_resrcmng->Release(p); \
			p = NULL; \
		} \
	}

namespace S5
{
	IMPLEMENT_ACTOR(CCutin, IActor)

	CCutin::CCutin()
		: m_state(0),
		  m_pCharOverlay(NULL),
		  m_pBgOverlay(NULL),
		  m_pPatternOverlay(NULL),
		  m_pTextOverlay(NULL),
		  m_alpha(CUTIN_MAX_ALPHA),
		  m_rotate(0.0f),
		  m_scale(1.0f),
		  m_delay(CUTIN_DELAY),
		  m_gameState(0),
		  m_condition(0)
	{
		m_dispPriority = DISP_PRIORITY_4;
		ClearAllData(true);
	}

	// 983 00b33d30: vozes do cut-in por tid (CGDVoiceItem "VoiceCutin")
	void CCutin::OnInit()
	{
		CGDVoiceItem* pData = (CGDVoiceItem*)CGameDataDB::Instance()->GetGameData(
			GAMEDATA_VOICEITEM, "VoiceCutin");
		if (pData)
		{
			std::list<sGD_VOICEITEM>& voices = pData->GetVoiceList();
			for (std::list<sGD_VOICEITEM>::iterator it = voices.begin(); it != voices.end(); ++it)
				m_voiceMap.insert(std::make_pair(it->typeID, *it));
		}
	}

	// 983 00b33400: primeira voz do tid cujo estado casa com a condicao do cut-in
	void CCutin::PlayVoice(unsigned long tid, unsigned long condition)
	{
		typedef std::multimap<unsigned long, sGD_VOICEITEM>::const_iterator It;
		std::pair<It, It> range = m_voiceMap.equal_range(tid);
		for (It it = range.first; it != range.second; ++it)
		{
			if (it->second.state & condition)
			{
				if (g_audio && !it->second.name.empty())
					g_audio->PlaySfx(it->second.name.c_str());
				return;
			}
		}
	}

	void CCutin::OnDestroy()
	{
		RELEASE_CUTIN_OVERLAY(m_pCharOverlay);
		RELEASE_CUTIN_OVERLAY(m_pBgOverlay);
		RELEASE_CUTIN_OVERLAY(m_pPatternOverlay);
		RELEASE_CUTIN_OVERLAY(m_pTextOverlay);
	}

	void CCutin::OnProcess(float delta)
	{
		switch (m_state)
		{
		case 1:
			if (CheckGameState())
				m_state = 2;
			break;

		case 2:
			if (PrepareCutIn())
			{
				m_state = 3;
				SendMsg(this, "Screen", M983(0x190), 0, 0, 0, 0);
			}
			else
				ClearAllData(false);
			break;

		case 3:
			if (ProcessWorkIn(delta))
				m_state = 5;
			break;

		case 5:
			if (ProcessWorkInFinish(delta))
			{
				OutDraw_Setting();
				m_state = 6;
			}
			break;

		case 6:
			if (ProcessWorkOut(delta))
				m_state = 7;
			break;

		case 7:
			RELEASE_CUTIN_OVERLAY(m_pCharOverlay);
			RELEASE_CUTIN_OVERLAY(m_pBgOverlay);
			RELEASE_CUTIN_OVERLAY(m_pPatternOverlay);
			RELEASE_CUTIN_OVERLAY(m_pTextOverlay);

			PostMsg(this, "GolfRule", M983(0x24d), 0, (int)"CUTIN_DRAW_SUCCES", 0, 0);
			m_state = 8;
			SendMsg(this, "Screen", M983(0x190), 1, 0, 0, 0);
			break;

		case 8:
			ClearAllData(false);
			break;
		}
	}

	void CCutin::OnDisplay()
	{
		if (m_state <= 2 || m_pCharOverlay == NULL)
			return;

		if (m_state >= 6)
		{
			DisplayOverlay(m_pBgOverlay, m_info.OutType, m_rcBg);
			DisplayOverlay(m_pPatternOverlay, m_info.OutType, m_rcPattern);
			DisplayOverlay(m_pCharOverlay, m_info.OutType, m_rcChar);
			DisplayOverlay(m_pTextOverlay, m_info.OutType, m_rcText);
		}
		else
		{
			DisplayOverlay(m_pBgOverlay, m_info.BgType, m_rcBg);
			DisplayOverlay(m_pPatternOverlay, m_info.PatternType, m_rcPattern);
			DisplayOverlay(m_pCharOverlay, m_info.CharType, m_rcChar);
			DisplayOverlay(m_pTextOverlay, m_info.TextType, m_rcText);
		}
	}

	void CCutin::HandleMsg(const MsgObject& msg)
	{
		if (msg.time > g_CurrentTime)
		{
			StoreDelayedMsg(msg);
			return;
		}

		switch (msg.message)
		{
		// 983 00b336f0: cut-in em curso so recebe a condicao nova e a voz (sem fila "next" nem "mode_modeDriveMotion" do 645)
		case M983(0x24c):
			if (GetCondition() == 0)
			{
				m_info = *(CUTIN_ANI_STYPEINFO*)msg.param2;

				RELEASE_CUTIN_OVERLAY(m_pCharOverlay);
				RELEASE_CUTIN_OVERLAY(m_pBgOverlay);
				RELEASE_CUTIN_OVERLAY(m_pPatternOverlay);
				RELEASE_CUTIN_OVERLAY(m_pTextOverlay);

				m_pCharOverlay = g_resrcmng->GetOverlay(m_info.szCharTex, 0);
				if (m_pCharOverlay)
				{
					m_pBgOverlay = g_resrcmng->GetOverlay(m_info.szBgTex, 0);
					m_pPatternOverlay =
						g_resrcmng->GetOverlay(m_info.szPattern, 0);
					m_pTextOverlay = g_resrcmng->GetOverlay(m_info.szText, 0);
				}

				SetCondition(msg.param3);
			}
			m_state = 1;
			PlayVoice(msg.param4, msg.param3);
			break;

		// 983 (00b336f0): estado do jogo e 0x30c (o 645 usava M983(0x78)); 0x30b novo, zera o estado (troca de vez, 0067c92a)
		case 0x30c:
			SetGameState(msg.param1);
			if (msg.param1 == 0)
				ClearAllData(false);
			break;

		case 0x30b:
			m_gameState = 0;
			break;
		}
	}

	bool CCutin::PrepareCutIn()
	{
		if (m_state > 1)
		{
			if (m_pCharOverlay == NULL)
				return false;

			SetOverlayInfo(m_pCharOverlay, m_info.CharType, m_rcChar);
			SetOverlayInfo(m_pBgOverlay, m_info.BgType, m_rcBg);
			SetOverlayInfo(m_pPatternOverlay, m_info.PatternType, m_rcPattern);
			SetOverlayInfo(m_pTextOverlay, m_info.TextType, m_rcText);
			return true;
		}
		return false;
	}

	bool CCutin::ProcessWorkIn(float delta)
	{
		bool bBg = ProcessOverlay(delta, m_pBgOverlay, m_info.BgType, m_rcBg);
		bool bPattern = ProcessOverlay(delta, m_pPatternOverlay,
			m_info.PatternType, m_rcPattern);
		bool bChar =
			ProcessOverlay(delta, m_pCharOverlay, m_info.CharType, m_rcChar);
		bool bText =
			ProcessOverlay(delta, m_pTextOverlay, m_info.TextType, m_rcText);

		if (bBg == true && bPattern == true && bChar == true && bText == true)
			return true;

		return false;
	}

	bool CCutin::ProcessWorkInFinish(float delta)
	{
		SAFE_SUBVALUE(m_delay, 0.0f, delta);
		if (Wabs(m_delay) < g_EPSILON)
		{
			m_delay = CUTIN_DELAY;
			if (m_info.OutType == CUTIN_ROTATE)
				m_rotate = s_fMaxRotate;
			return true;
		}
		return false;
	}

	bool CCutin::ProcessWorkOut(float delta)
	{
		bool bBg =
			ProcessOverlay_Out(delta, m_pBgOverlay, m_info.BgType, m_rcBg);
		bool bPattern = ProcessOverlay_Out(delta, m_pPatternOverlay,
			m_info.PatternType, m_rcPattern);
		bool bChar = ProcessOverlay_Out(delta, m_pCharOverlay, m_info.CharType,
			m_rcChar);
		bool bText = ProcessOverlay_Out(delta, m_pTextOverlay, m_info.TextType,
			m_rcText);

		if (bBg == true && bPattern == true && bChar == true && bText == true)
			return true;

		return false;
	}

	void CCutin::ClearAllData(bool bAll)
	{
		if (!bAll)
		{
			m_state = 0;
			m_alpha = CUTIN_MAX_ALPHA;
			m_rotate = 0.0f;
			m_scale = 1.0f;
			m_delay = CUTIN_DELAY;
			m_condition = 0;
		}

		m_info.Reset();

		memset(&m_rcChar, 0, sizeof(WRect));
		memset(&m_rcBg, 0, sizeof(WRect));
		memset(&m_rcPattern, 0, sizeof(WRect));
		memset(&m_rcText, 0, sizeof(WRect));

		RELEASE_CUTIN_OVERLAY(m_pCharOverlay);
		RELEASE_CUTIN_OVERLAY(m_pBgOverlay);
		RELEASE_CUTIN_OVERLAY(m_pPatternOverlay);
		RELEASE_CUTIN_OVERLAY(m_pTextOverlay);
	}

	void CCutin::SetOverlayInfo(WOverlay* pOverlay, eCUTINANISTLYE style,
		WRect& rect)
	{
		if (pOverlay == NULL)
			return;

		rect.w = (float)pOverlay->GetWidth();
		rect.h = (float)pOverlay->GetHeight();

		switch (m_info.PosType)
		{
		case CUTIN_DP_NONE:
			rect.x = 0.0f;
			rect.y = 0.0f;
			break;

		case CUTIN_DP_CENTER:
			rect.x = (g_view->GetWidth() - rect.w) * 0.5f;
			rect.y = (g_view->GetHeight() - rect.h) * 0.5f;
			break;

		case CUTIN_DP_RIGHT:
			break;

		case CUTIN_DP_LEFT:
			break;

		case CUTIN_DP_TOP:
			rect.x = 0.0f;
			rect.y = 0.0f;
			break;

		case CUTIN_DP_BOTTOM:
			rect.x = g_view->GetWidth() - rect.w;
			rect.y = g_view->GetHeight() - rect.h;
			break;
		}

		ScreenRate(rect, pOverlay);

		switch (style)
		{
		case CUTIN_TOLEFT:
			rect.x = g_view->GetWidth();
			break;

		case CUTIN_TORIGHT:
			rect.x = -rect.w;
			break;

		case CUTIN_ZOOM_IN:
			m_scale = 1.0f;
			break;

		case CUTIN_ZOOM_OUT:
			m_scale = 0.0f;
			break;

		case CUTIN_FADE_IN:
			m_alpha = 0.0f;
			break;

		case CUTIN_FADE_OUT:
			m_alpha = CUTIN_MAX_ALPHA;
			break;
		}
	}

	bool CCutin::ProcessOverlay(float delta, WOverlay* pOverlay,
		eCUTINANISTLYE style, WRect& rect)
	{
		if (pOverlay == NULL)
			return true;

		switch (style)
		{
		case CUTIN_NONE:
			SAFE_SUBVALUE(m_delay, 0.0f, delta);
			return WisZero(m_delay, g_EPSILON) ? true : false;

		case CUTIN_TORIGHT:
		{
			// The original keeps a dead width comparison here.
			float fTarget = (g_view->GetWidth() > 1024.0f) ? 0.0f : 0.0f;

			float fSpeed;
			if (rect.x < -100.0f)
				fSpeed = 3.0f;
			else
				fSpeed = 0.08f;

			if (SAFE_ADDVALUE(rect.x, fTarget, fSpeed * rect.w * delta) < 0.0f)
				return false;
		}
		break;

		case CUTIN_TOLEFT:
		{
			float fSpeed;
			if (rect.x < 100.0f)
				fSpeed = 0.08f;
			else
				fSpeed = 3.0f;

			if (SAFE_SUBVALUE(rect.x, 0.0f, fSpeed * rect.w * delta) > 0.0f)
				return false;
		}
		break;

		case CUTIN_ZOOM_IN:
			if (SAFE_SUBVALUE(m_scale, 0.0f, delta) > 0.0f)
				return false;
			break;

		case CUTIN_ZOOM_OUT:
			if (SAFE_ADDVALUE(m_scale, 1.0f, delta) < 1.0f)
				return false;
			break;

		case CUTIN_ROTATE:
			if (SAFE_ADDVALUE(m_rotate, s_fMaxRotate, s_fMaxRotate * delta) <
				s_fMaxRotate)
				return false;
			break;

		case CUTIN_FADE_IN:
			if (SAFE_ADDVALUE(m_alpha, CUTIN_MAX_ALPHA,
					delta * CUTIN_MAX_ALPHA) < CUTIN_MAX_ALPHA)
				return false;
			break;

		case CUTIN_FADE_OUT:
			if (SAFE_SUBVALUE(m_alpha, CUTIN_MIN_ALPHA,
					delta * CUTIN_MAX_ALPHA) > 0.0f)
				return false;
			break;
		}
		return true;
	}

	bool CCutin::ProcessOverlay_Out(float delta, WOverlay* pOverlay,
		eCUTINANISTLYE style, WRect& rect)
	{
		if (pOverlay == NULL)
			return true;

		if (m_info.OutType == CUTIN_REVERSE || m_info.OutType == CUTIN_UNTIL)
		{
			if (m_info.OutType == CUTIN_UNTIL)
			{
				if (style == CUTIN_TORIGHT)
					style = CUTIN_TOLEFT;
				else if (style == CUTIN_TOLEFT)
					style = CUTIN_TORIGHT;
			}
		}
		else
			style = m_info.OutType;

		switch (style)
		{
		case CUTIN_NONE:
			SAFE_SUBVALUE(m_delay, 0.0f, delta);
			return WisZero(m_delay, g_EPSILON) ? true : false;

		case CUTIN_TORIGHT:
		{
			float fMin = -rect.w;
			float fSpeed;
			if (rect.x > fMin * 0.9f)
				fSpeed = 0.08f;
			else
				fSpeed = 3.0f;

			float fResult =
				SAFE_SUBVALUE(rect.x, fMin, fSpeed * delta * rect.w);
			if (fResult > fMin)
				return false;
		}
		break;

		case CUTIN_TOLEFT:
		{
			float fMax = g_view->GetWidth();
			if (SAFE_ADDVALUE(rect.x, fMax, delta * rect.w) <
				g_view->GetWidth())
				return false;
		}
		break;

		case CUTIN_ZOOM_IN:
			if (SAFE_SUBVALUE(m_scale, 0.0f, delta) > 0.0f)
				return false;
			break;

		case CUTIN_ZOOM_OUT:
			if (SAFE_ADDVALUE(m_scale, 1.0f, delta) < 1.0f)
				return false;
			break;

		case CUTIN_ROTATE:
			if (SAFE_SUBVALUE(m_rotate, 0.0f, s_fMaxRotate * delta) > 0.0f)
				return false;
			break;

		case CUTIN_FADE_IN:
			if (SAFE_ADDVALUE(m_alpha, CUTIN_MAX_ALPHA,
					delta * CUTIN_MAX_ALPHA) < CUTIN_MAX_ALPHA)
				return false;
			break;

		case CUTIN_FADE_OUT:
			if (SAFE_SUBVALUE(m_alpha, CUTIN_MIN_ALPHA,
					delta * CUTIN_MAX_ALPHA) > 0.0f)
				return false;
			break;
		}
		return true;
	}

	void CCutin::DisplayOverlay(WOverlay* pOverlay, eCUTINANISTLYE style,
		WRect& rect)
	{
		float fAlpha = CUTIN_MAX_ALPHA;
		float fRotate = 0.0f;
		WRect rcDraw = rect;

		if (pOverlay == NULL)
			return;

		if (IsAlphaCutInStyle(style))
			fAlpha = m_alpha;

		if (IsRotateCutInStyle(style))
			fRotate = m_rotate;
		else if (IsScalingCutInStyle(style))
			RectScaling(rcDraw, m_scale);

		pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f), rcDraw, 0,
			((unsigned char)fAlpha << 24) | 0xffffff, fRotate, 0);
	}

	void CCutin::OutDraw_Setting()
	{
		switch (m_info.OutType)
		{
		case CUTIN_ZOOM_IN:
			m_scale = 1.0f;
			break;

		case CUTIN_ZOOM_OUT:
			m_scale = 0.0f;
			break;

		case CUTIN_FADE_IN:
			m_alpha = 0.0f;
			break;

		case CUTIN_FADE_OUT:
			m_alpha = CUTIN_MAX_ALPHA;
			break;
		}
	}

	// 983 00b321c0: no modo 0 espera tambem o estado 8
	bool CCutin::CheckGameState()
	{
		return IsGameState(4) && (Doc()->m_gameMode != 0 || IsGameState(8)) && IsGameState(0x10);
	}

	bool CCutin::IsGameState(unsigned long state)
	{
		if (m_gameState & state)
			return true;
		return false;
	}

	bool CCutin::ISCondition(unsigned long condition)
	{
		if (m_condition & condition)
			return true;
		return false;
	}

	void CCutin::RectScaling(WRect& rect, float scale)
	{
		float fCenterX = rect.x + rect.w * 0.5f;
		float fCenterY = rect.y + rect.h * 0.5f;

		rect.w *= scale;
		rect.h *= scale;
		rect.x = fCenterX - rect.w * 0.5f;
		rect.y = fCenterY - rect.h * 0.5f;
	}

	void CCutin::ScreenRate(WRect& rect, WOverlay* pOverlay)
	{
		if (pOverlay == NULL)
			return;

		// HACK: the center Y and the X rate share a WVector2D so that the stack
		// slots (center X in the rect argument slot) match the original.
		float fCenterX = rect.x + rect.w * 0.5f;
		WVector2D v;
		v.y = rect.y + rect.h * 0.5f;
		v.x = g_view->GetWidth() / 1024.0f;
		float fHeight = g_view->GetHeight();

		// 983 00b31aa0: largura sempre pela taxa X; em wide a altura tambem (mantem o aspecto), sem olhar a camada
		if (COption::Instance()->vIsWideMode())
		{
			rect.w *= v.x;
			rect.h *= v.x;
		}
		else
		{
			rect.w *= v.x;
			rect.h *= fHeight / 768.0f;
		}

		rect.x = fCenterX - rect.w * 0.5f;
		rect.y = v.y - rect.h * 0.5f;

		if (pOverlay->GetWidth() > pOverlay->GetHeight())
			rect.x = 0.0f;
		else
			rect.y = 0.0f;
	}

	std::string CCutin::TwAnimateTest(const tweaker_param& param)
	{
		IFF_STRUCT::S5::sCutinInformation* pInfo =
			ItemManager()->FindCutinInformation(param[0].to_int());

		if (pInfo)
		{
			RELEASE_CUTIN_OVERLAY(m_pCharOverlay);
			m_pCharOverlay = g_resrcmng->GetOverlay(pInfo->Char_Tex, 0);
			RELEASE_CUTIN_OVERLAY(m_pBgOverlay);
			m_pBgOverlay = g_resrcmng->GetOverlay(pInfo->Bg_Tex, 0);
			RELEASE_CUTIN_OVERLAY(m_pPatternOverlay);
			m_pPatternOverlay = g_resrcmng->GetOverlay(pInfo->Pattern_Tex, 0);
			RELEASE_CUTIN_OVERLAY(m_pTextOverlay);
			m_pTextOverlay = g_resrcmng->GetOverlay(pInfo->Text_Tex, 0);

			m_info.PosType = (eCUTINANI_DRAWPOS)param[1].to_int();
			m_info.CharType = (eCUTINANISTLYE)param[2].to_int();
			m_info.BgType = (eCUTINANISTLYE)param[3].to_int();
			m_info.PatternType = (eCUTINANISTLYE)param[4].to_int();
			m_info.TextType = (eCUTINANISTLYE)param[5].to_int();
			m_info.OutType = (eCUTINANISTLYE)param[6].to_int();

			m_state = 2;
			return "setup ok.";
		}

		m_state = 0;
		return "img loding failed.";
	}
}
