#include "minatl.h"
#include "ghostplayer.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "objectfactory.h"
#include "projectg.h"
#include "packet.h"
#include "ghostmanager.h"
// HACK: ghostdocument.h calls the game data GHOST_GAME_BRIEF, which collides with the real
// GHOST_GAME_BRIEF of ghosttypes.h (pulled in by ghostgamedlg.h)
#define GHOST_GAME_BRIEF GHOST_GAME_DATA
#include "ghostdocument.h"
#undef GHOST_GAME_BRIEF
#include "sequencestatemachine.h"
#include "ghostgamedlg.h"
#include "soundmanager.h"
#include "taskmanager.h"
#include "club.h"
#include "commonutil.h"
#include "golfball.h"
#include "ghosthandler.h"
#include "fresh.h"
extern Fresh* g_pFresh;
#include "s5/utilities.h"
#include "type_convert.h"
#include "wind.h"
#include "mathconsts.h"
#include <algorithm>
#include <iterator>
#include <vector>


namespace
{

	bool IsGhostTurn()
	{
		return GOLFDOC()->m_currentPlayer != 0;
	}

	// A state of the ghost replay: runs OnStateRun once per frame until it
	// reports done, and stays current until m_timer runs out.
	class CGhostStateUnit : public CStateUnit
	{
	public:
		virtual const char* GetClassNameA() const { return "CGhostStateUnit"; }
		CGhostStateUnit()
			: m_elapsed(0.0f), m_timer(2.0f), m_timeLimit(2.0f), m_done(false)
		{
		}
		virtual ~CGhostStateUnit() { }

		void SetTimer(float fTime) { m_timer = m_timeLimit = fTime; }

		virtual void OnRender() { }

		virtual bool OnStateRun() = 0;

	private:
		virtual void OnProcess(float delta)
		{
			m_delta = delta;
			if (!OnFinished() && !m_done)
			{
				m_elapsed += delta;
				m_done = OnStateRun();

				if (m_done)
				{
					OnPrintLog(MakeStr(
						"Runned time: %.1f sec, Finish remain : %.1f sec",
						m_elapsed,
						m_timeLimit > 0.0f ? m_timeLimit - m_elapsed : 0.0f));
				}
			}

			m_timer -= delta;
		}

		virtual bool OnFinished() { return m_timer <= 0.0f; }

	protected:
		float m_elapsed;   // 0xc
		float m_timer;     // 0x10
		float m_timeLimit; // 0x14
		bool m_done;       // 0x18
		float m_delta;     // 0x1c
	};

	class CWindChangeState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const { return "CWindChangeState"; }
		CWindChangeState(unsigned char power, unsigned char direction,
			unsigned char type, unsigned char special)
			: m_power(power),
			  m_direction(direction),
			  m_type(type),
			  m_special(special)
		{
		}

		virtual bool OnStateRun()
		{
			SetTimer(1.0f);
			Wind().Init(m_power, m_direction, m_type, m_special);

			return true;
		}

	private:
		unsigned char m_power;     // 0x20
		unsigned char m_direction; // 0x21
		unsigned char m_type;      // 0x22
		unsigned char m_special;   // 0x23
	};

	class CWeatherChangeState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const
		{
			return "CWeatherChangeState";
		}
		CWeatherChangeState(unsigned char weather)
			: m_weather(weather)
		{
		}

		virtual bool OnStateRun()
		{
			AfxGetTask()->GetActor("GolfBg")
				<< MsgObject(0, M983(0x142), m_weather, 1, 0, 0, 0);

			return true;
		}

	private:
		unsigned char m_weather; // 0x20
	};

	class CDirectionChangeState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const
		{
			return "CDirectionChangeState";
		}
		CDirectionChangeState(float direction)
			: m_direction(direction)
		{
		}

		virtual bool OnStateRun()
		{
			SetTimer(3.0f);
			// The camera reads the direction through the address in param1.
			AfxGetTask()->GetActor("Camera")
				<< MsgObject(0, M983(0xf2), (int)&m_direction, 0, 0, 0, 0);

			return true;
		}

	private:
		float m_direction; // 0x20
	};

	class CSpecialWindState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const
		{
			return "CSpecialWindState";
		}
		CSpecialWindState() { }

		virtual bool OnStateRun() { return true; }
	};

	class CClubChangeState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const { return "CClubChangeState"; }
		CClubChangeState(unsigned char club)
			: m_club(club)
		{
		}

		virtual bool OnStateRun()
		{
			if (GolfClub().GetIndex() != m_club)
			{
				GolfClub().SetClub(m_club);
				g_audio->PlaySfx("\xc5\xac\xb7\xb4\xb1\xb3\xc3\xbc", 0, 0, 0, 0,
					0.5f, 200.0f);
			}

			return true;
		}

	private:
		unsigned char m_club; // 0x20
	};

	class CUseItemState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const { return "CUseItemState"; }
		CUseItemState(unsigned long itemTypeId, unsigned long itemId)
			: m_itemTypeId(itemTypeId), m_itemId(itemId)
		{
		}

		virtual bool OnStateRun()
		{
			AfxGetTask()->GetActor("Item")
				<< MsgObject(0, M983(0x19f), -2, m_itemTypeId, m_itemId, 0, 0);

			return true;
		}

	private:
		unsigned long m_itemTypeId; // 0x20
		unsigned long m_itemId;     // 0x24
	};

	class CPoweShotMotionState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const
		{
			return "CPoweShotMotionState";
		}
		CPoweShotMotionState(unsigned char powerShot)
			: m_powerShot(powerShot)
		{
		}

		virtual bool OnStateRun()
		{
			if (m_powerShot)
			{
				AfxGetTask()->GetActor("Player")
					<< MsgObject(0, M983(0x11f), m_powerShot, 0, 0, 0, 0);
			}

			return true;
		}

	private:
		unsigned char m_powerShot; // 0x20
	};

	class CBallDropState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const { return "CBallDropState"; }
		CBallDropState(float x, float y, float z)
		{
			m_x = x;
			m_y = y;
			m_z = z;
		}

		virtual bool OnStateRun()
		{
			SetTimer(2.5f);
			// The rule reads the position through the address in param1.
			AfxGetTask()->GetActor("GolfRule")
				<< MsgObject(0, M983(0x8d), (int)&m_x, 0, 0, 0, 0);

			return true;
		}

	private:
		float m_x; // 0x20
		float m_y; // 0x24
		float m_z; // 0x28
	};

	class CTimeOutState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const { return "CTimeOutState"; }
		CTimeOutState() { }

		virtual bool OnStateRun()
		{
			SetTimer(3.0f);
			AfxGetTask()->GetActor("GolfRule")
				<< MsgObject(0, M983(0x7c), -2, 0, 0, 0, 0);

			return true;
		}
	};

	class CReadyToShotState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const
		{
			return "CReadyToShotState";
		}
		CReadyToShotState() { }

		virtual bool OnStateRun()
		{
			SetTimer(0.0f);
			return true;
		}
	};

	// Moves the ball to the recorded start of the shot when it lies elsewhere.
	class CWarpEventState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const { return "CWarpEventState"; }
		CWarpEventState(const GHOST_SHOT_DATA* pShot)
			: m_warp(false)
		{
			if (!WisEqual(GolfBall().m_pos.x, pShot->m_ballPos[0], g_EPSILON) ||
				!WisEqual(GolfBall().m_pos.y, pShot->m_ballPos[1], g_EPSILON) ||
				!WisEqual(GolfBall().m_pos.z, pShot->m_ballPos[2], g_EPSILON))
			{
				m_warp = true;
				m_pos[0] = pShot->m_ballPos[0];
				m_pos[1] = pShot->m_ballPos[1];
				m_pos[2] = pShot->m_ballPos[2];
			}
		}

		virtual bool OnStateRun()
		{
			SetTimer(0.0f);
			if (m_warp)
			{
				// The rule reads the position through the address in param1.
				AfxGetTask()->GetActor("GolfRule")
					<< MsgObject(0, M983(0x8e), (int)m_pos, 0, 0, 0, 0);
			}

			return true;
		}

	private:
		bool m_warp;    // 0x20
		float m_pos[3]; // 0x24
	};

	// The masks closing in at the start of the ghost's turn.
	class CGhostTurnBeginAnimationState : public CGhostStateUnit
	{
	public:
		virtual const char* GetClassNameA() const
		{
			return "CGhostTurnBeginAnimationState";
		}

		CGhostTurnBeginAnimationState();
		virtual ~CGhostTurnBeginAnimationState();

		virtual bool OnStateRun();
		virtual void OnRender();

	protected:
		void SelectMaskType();
		bool RunAnimate(float delta);

	private:
		int m_maskType;                  // 0x20 (chosen by SelectMaskType)
		WOverlay* m_pOverlay;            // 0x24
		std::vector<WRect> m_maskRects;  // 0x28
		std::vector<float> m_maskSpeeds; // 0x38
		float m_z;                       // 0x48
		float m_alpha;                   // 0x4c
		float m_animTime;                // 0x50
	};

	inline CGhostTurnBeginAnimationState::CGhostTurnBeginAnimationState()
		: m_maskType(0),
		  m_pOverlay(0),
		  m_z(0.0f),
		  m_alpha(255.0f),
		  m_animTime(0.0f)
	{
		SetTimer(30.0f);
		m_pOverlay = g_resrcmng->GetOverlay("black_mask.jpg", 0);
		SelectMaskType();
	}

	inline CGhostTurnBeginAnimationState::~CGhostTurnBeginAnimationState()
	{
		if (g_resrcmng && m_pOverlay)
		{
			g_resrcmng->Release(m_pOverlay);
			m_pOverlay = 0;
		}
	}

	inline bool CGhostTurnBeginAnimationState::OnStateRun()
	{
		float delta = m_delta;
		if (RunAnimate(delta))
		{
			SetTimer(0.0f);
			return true;
		}

		return false;
	}

	inline void CGhostTurnBeginAnimationState::OnRender()
	{
		std::vector<WRect>::iterator it = m_maskRects.begin();
		while (it != m_maskRects.end())
		{
			m_pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f), *it, 0,
				(unsigned char)m_alpha << 24, m_z, 0);
			++it;
		}
	}

	inline void CGhostTurnBeginAnimationState::SelectMaskType()
	{
		float width = g_view->GetWidth();
		float height = g_view->GetHeight();

		m_z = 0.0f;
		m_alpha = 255.0f;
		m_animTime = 0.0f;

		m_maskType = rand() % 7;
		if (m_maskType >= 7)
		{
			m_maskType = 0;
		}

		switch (m_maskType)
		{
		case 0:
			// Vertical bars with their own speed.
			m_maskSpeeds.reserve(20);
			for (int i = 0; i < 20; ++i)
			{
				m_maskSpeeds.push_back((rand() % 30 + 10) * 0.1f);
			}
		case 4:
		case 5:
			m_maskRects.reserve(20);
			for (int i = 0; i < 20; ++i)
			{
				float barWidth = width * 0.05f;
				m_maskRects.push_back(
					WRect(i * barWidth, 0.0f, barWidth, height));
			}
			break;
		case 1:
			// Horizontal bars with their own speed.
			m_maskSpeeds.reserve(20);
			for (int i = 0; i < 20; ++i)
			{
				m_maskSpeeds.push_back((rand() % 30 + 10) * 0.1f);
			}
		case 3:
		case 6:
			m_maskRects.reserve(20);
			for (int i = 0; i < 20; ++i)
			{
				float barHeight = height * 0.05f;
				m_maskRects.push_back(
					WRect(0.0f, i * barHeight, width, barHeight));
			}
			break;
		case 2:
			m_maskRects.push_back(
				WRect(0.0f, (width - height) * -0.5f, width, width));
			break;
		}
	}

	// The closing speed of the masks (nameless constant of the original).
	const float MASK_SPEED = 0.8333333f;

	// HACK: the original unit owns storage for g_PI right after the mask speed (no code refers to it).
	inline void RbGhostPlayerConsts()
	{
		(void)&g_PI;
	}

	inline bool CGhostTurnBeginAnimationState::RunAnimate(float delta)
	{
		float width = g_view->GetWidth();
		float height = g_view->GetHeight();
		bool finished = false;
		int i = 0;

		switch (m_maskType)
		{
		case 5:
		{
			float barWidth = width * 0.05f;
			float move = barWidth * delta * MASK_SPEED;
			for (std::vector<WRect>::iterator it = m_maskRects.begin();
				it != m_maskRects.end(); ++it)
			{
				if ((*it).w > 0.0f)
				{
					if (((*it).w -= move) <= 0.0f)
						(*it).w = 0.0f;
				}
				else
					finished = true;
			}
			break;
		}
		case 4:
		{
			float barWidth = width * 0.05f;
			float move = barWidth * delta * MASK_SPEED;
			for (std::vector<WRect>::iterator it = m_maskRects.begin();
				it != m_maskRects.end(); ++it)
			{
				if ((*it).x < width)
				{
					if (((*it).x += move) >= width)
						(*it).x = width;
				}
				if ((*it).w > 0.0f)
				{
					if (((*it).w -= move) <= 0.0f)
						(*it).w = 0.0f;
				}
				else
					finished = true;
			}
			break;
		}
		case 1:
		{
			if (m_alpha > 0.0f)
			{
				if ((m_alpha -= delta * 127.5f) <= 0.0f)
					m_alpha = 0.0f;
			}

			float move = width * delta * 0.5f;
			int closed = 0;
			for (std::vector<WRect>::iterator it = m_maskRects.begin();
				it != m_maskRects.end(); ++it)
			{
				if (i % 2)
				{
					float speed = m_maskSpeeds[i];
					if ((*it).x < width)
					{
						if (((*it).x += move * speed) >= width)
							(*it).x = width;
					}
				}

				float speed = m_maskSpeeds[i];
				if ((*it).w > 0.0f)
				{
					if (((*it).w -= move * speed) <= 0.0f)
						(*it).w = 0.0f;
				}
				else
					++closed;

				++i;
			}

			if (closed == m_maskRects.size())
			{
				finished = true;
			}
			break;
		}
		case 0:
		{
			if (m_alpha > 0.0f)
			{
				if ((m_alpha -= delta * 127.5f) <= 0.0f)
					m_alpha = 0.0f;
			}

			float move = height * delta * 0.5f;
			int closed = 0;
			i = 0;
			for (std::vector<WRect>::iterator it = m_maskRects.begin();
				it != m_maskRects.end(); ++it)
			{
				if (i % 2)
				{
					float speed = m_maskSpeeds[i];
					if ((*it).y < height)
					{
						if (((*it).y += move * speed) >= height)
							(*it).y = height;
					}
				}

				float speed = m_maskSpeeds[i];
				if ((*it).h > 0.0f)
				{
					if (((*it).h -= move * speed) <= 0.0f)
						(*it).h = 0.0f;
				}
				else
					++closed;

				++i;
			}

			if (closed == m_maskRects.size())
			{
				finished = true;
			}
			break;
		}
		case 6:
		{
			float barHeight = height * 0.05f;
			float move = barHeight * delta * MASK_SPEED;
			for (std::vector<WRect>::iterator it = m_maskRects.begin();
				it != m_maskRects.end(); ++it)
			{
				if ((*it).h > 0.0f)
				{
					if (((*it).h -= move) <= 0.0f)
						(*it).h = 0.0f;
				}
				else
					finished = true;
			}
			break;
		}
		case 3:
		{
			float barHeight = height * 0.05f;
			float move = barHeight * delta * MASK_SPEED;
			for (std::vector<WRect>::iterator it = m_maskRects.begin();
				it != m_maskRects.end(); ++it)
			{
				if ((*it).y < height)
				{
					if (((*it).y += move) >= height)
						(*it).y = height;
				}
				if ((*it).h > 0.0f)
				{
					if (((*it).h -= move) <= 0.0f)
						(*it).h = 0.0f;
				}
				else
					finished = true;
			}
			break;
		}
		case 2:
		{
			if (m_alpha > 0.0f)
			{
				m_alpha -= delta * m_alpha * MASK_SPEED;
				if (m_alpha <= 0.0f)
				{
					m_alpha = 0.0f;
				}
			}

			m_z += delta * 12.566371f;

			std::vector<WRect>::iterator it = m_maskRects.begin();
			float move = width * delta * MASK_SPEED;
			if (it != m_maskRects.end())
			{
				if ((*it).w > 0.0f)
				{
					(*it).x += move * 0.5f;
					(*it).y += move * 0.5f;
					(*it).w -= move;
					(*it).h -= move;
					if ((*it).w <= 0.0f)
					{
						finished = true;
					}
				}
			}
			break;
		}
		}

		return finished;
	}

} // namespace

inline CGhostPlayer::cMovieStyleDisplay::cMovieStyleDisplay()
{
	m_height = 0.0f;
	m_pOverlay = g_resrcmng->GetOverlay("black_mask.jpg", 0);
}

inline CGhostPlayer::cMovieStyleDisplay::~cMovieStyleDisplay()
{
	if (g_resrcmng && m_pOverlay)
	{
		g_resrcmng->Release(m_pOverlay);
		m_pOverlay = 0;
	}
}

inline void CGhostPlayer::cMovieStyleDisplay::ResetHeight()
{
	m_height = 0.0f;
}

inline void CGhostPlayer::cMovieStyleDisplay::Process(float delta)
{
	float maxHeight = g_view->GetHeight() * (1.0f / 7.0f);
	if (m_height < maxHeight)
	{
		m_height += delta * maxHeight;
	}

	if (m_height > maxHeight)
	{
		m_height = maxHeight;
	}
}

inline void CGhostPlayer::cMovieStyleDisplay::Render()
{
	float maxHeight = g_view->GetHeight() * (1.0f / 7.0f);
	float alpha = m_height / maxHeight * 255.0f;

	m_pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
		WRect(0.0f, 0.0f, g_view->GetWidth(), m_height), 0,
		((unsigned char)alpha << 24) | 0xFFFFFF, 0.0f, 0);

	m_pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
		WRect(0.0f, g_view->GetHeight() - m_height, g_view->GetWidth(),
			m_height),
		0, ((unsigned char)alpha << 24) | 0xFFFFFF, 0.0f, 0);
}

IMPLEMENT_ACTOR(CGhostPlayer, IActor)

CGhostPlayer::CGhostPlayer()
	: m_pGhostDoc(0),
	  m_pHoleData(0),
	  m_pStateMachine(0),
	  m_pAward(0),
	  m_pMovieStyle(0),
	  m_totalShotCount(0),
	  m_shotIndex(0),
	  m_errorCode(0),
	  m_bShotPlaying(0),
	  m_bAwardSent(0)
{
	m_dispPriority = DISP_PRIORITY_5;
}

CGhostPlayer::~CGhostPlayer()
{
}

CStateUnit* CGhostPlayer::CreateStateUnit(int type,
	const GHOST_ACT_DATA* pAction)
{
	switch (type)
	{
	case 0:
	{
		unsigned char power = (unsigned char)pAction->m_args[0];
		unsigned char direction = (unsigned char)pAction->m_args[1];
		unsigned char windType = (unsigned char)pAction->m_args[2];
		unsigned char special = (unsigned char)pAction->m_args[3];

		return new CWindChangeState(power, direction, windType, special);
	}
	case 1:
	{
		unsigned char weather = (unsigned char)pAction->m_args[0];
		return new CWeatherChangeState(weather);
	}
	case 2:
	{
		float direction = *(const float*)&pAction->m_args[0];
		return new CDirectionChangeState(direction);
	}
	case 3:
		return new CSpecialWindState;
	case 4:
	{
		unsigned char club = (unsigned char)pAction->m_args[0];
		return new CClubChangeState(club);
	}
	case 5:
	{
		unsigned long itemTypeId = pAction->m_args[0];
		unsigned long itemId = pAction->m_args[1];
		return new CUseItemState(itemTypeId, itemId);
	}
	case 6:
	{
		unsigned char powerShot = (unsigned char)pAction->m_args[0];
		return new CPoweShotMotionState(powerShot);
	}
	case 7:
	{
		float x = *(const float*)&pAction->m_args[0];
		float y = *(const float*)&pAction->m_args[1];
		float z = *(const float*)&pAction->m_args[2];
		return new CBallDropState(x, y, z);
	}
	case 8:
		return new CTimeOutState;
	}

	return 0;
}

void CGhostPlayer::OnLoad()
{
	m_pGhostDoc = CGhostManager::Instance()->GetDocument();

	// The replay runs under the server property the ghost was recorded with.
	m_serverProperty = S5::UTIL::GetServerProperty();
	const GHOST_GAME_DATA* pGameData = m_pGhostDoc->m_pHeader;
	if (pGameData->m_serverProperty & 0x80)
	{
		S5::UTIL::SetServerProperty(m_serverProperty | 0x80);
	}
	else
	{
		S5::UTIL::SetServerProperty(m_serverProperty & ~0x80);
	}

	m_pStateMachine = new CSequenceStateMachine(this, "GhostSequenceMachine");
	m_pMovieStyle = new cMovieStyleDisplay;
}

void CGhostPlayer::OnReload()
{
}

void CGhostPlayer::OnInit()
{
	m_pStateMachine->Clear();
	m_shotList.clear();
	m_shotIndex = 0;

	m_pHoleData = m_pGhostDoc->GetHoleData(GOLFDOC()->m_currentHole);

	if (m_pHoleData)
	{
		std::list<GHOST_SHOT_DATA*> const& shotDataList =
			m_pHoleData->GetShotDataList();
		std::copy(shotDataList.begin(), shotDataList.end(),
			std::back_inserter(m_shotList));

		int shotNum = m_totalShotCount;
		std::list<GHOST_SHOT_DATA*>::iterator it = m_shotList.begin();
		while (it != m_shotList.end())
		{
			GHOST_SHOT_DATA* pShot = *it;

			m_pStateMachine->BackInsert(new CWarpEventState(pShot));
			m_pStateMachine->BackInsert(new CGhostTurnBeginAnimationState);

			for (int i = 0; i < 9; ++i)
			{
				if (pShot->m_acts[i].m_used)
				{
					m_pStateMachine->BackInsert(
						CreateStateUnit(i, &pShot->m_acts[i]));
				}
			}

			if (pShot->m_frame != ++shotNum)
			{
				m_errorCode = 1;
				break;
			}

			m_pStateMachine->BackInsert(new CReadyToShotState);
			++it;
		}

		if (!m_shotList.empty())
		{
			SetWindAndWeather(GOLFDOC()->m_currentHole);
			return;
		}
	}

	m_errorCode = 3;
}

void CGhostPlayer::OnProcess(float delta)
{
	if (!m_pAward && m_errorCode)
	{
		char text[64] = { 0 };
		sprintf(text, "GHOST FILE BREAK! (CODE:%d)(HOLE:%d)(SHOT:%d)",
			m_errorCode, GOLFDOC()->m_currentHole, m_totalShotCount);
		const GHOST_FILE_INFO* pBrief =
			CGhostManager::Instance()->GetLatestFileInfo();
		CGhostHandler::Instance()->SendErrorReport(text,
			MakeStr("%d_%d_%d_%d_%d_%d.GST", pBrief->m_date, pBrief->m_time,
				pBrief->m_course, pBrief->m_score, pBrief->m_uid,
				pBrief->m_index));

		HandleMsg(MsgObject(0, M983(0xe), 0, 0, 0, 0, 0));
		return;
	}

	if (m_pAward)
	{
		if (m_pAward->IsFinishCalcBonus() && !m_bAwardSent)
		{
			m_bAwardSent = 1;
			SendAwardResult();
		}
		return;
	}

	if (!IsGhostTurn())
	{
		if (m_bShotPlaying)
		{
			m_bShotPlaying = 0;
		}
		return;
	}

	if (m_bShotPlaying == 1)
	{
		m_pMovieStyle->Process(delta);
	}

	if (!(GOLFDOC()->m_gameMode & 0x308))
	{
		return;
	}

	if (!m_pStateMachine->Process(delta))
	{
		m_errorCode = 4;
		return;
	}

	if (m_pStateMachine->GetCurrentState() &&
		strcmp("CReadyToShotState",
			m_pStateMachine->GetCurrentState()->GetClassNameA()) == 0)
	{
		if (m_shotIndex < m_shotList.size())
		{
			std::list<GHOST_SHOT_DATA*>::const_iterator it = m_shotList.begin();
			std::advance(it, m_shotIndex);
			if (it != m_shotList.end())
			{
				GHOST_SHOT_DATA* pShot = *it;
				if (pShot->m_valid == 0)
				{
					if (DoShot(pShot))
					{
						m_bShotPlaying = 1;
						m_pMovieStyle->ResetHeight();
					}
					else
					{
						m_errorCode = 2;
						return;
					}
				}
			}

			++m_shotIndex;
			++m_totalShotCount;
		}
	}
}

bool CGhostPlayer::DoShot(const GHOST_SHOT_DATA* pShot)
{
	if (pShot->m_holeIndex != GOLFDOC()->m_currentHole)
	{
		return false;
	}

	memset((sShotProtocol*)GOLFDOC()->m_shotSync983, 0, sizeof(sShotProtocol));
	type_convert(&pShot->m_protocol, (sShotProtocol*)GOLFDOC()->m_shotSync983);
	AfxGetTask()->GetActor("GolfRule") << MsgObject(0, M983(0x7a), 0, 0, 0, 0, 0);

	return true;
}

void CGhostPlayer::OnDisplay()
{
	if (!Doc()->m_bGameOver && IsGhostTurn())
	{
		m_pStateMachine->Render();
	}
}

void CGhostPlayer::OnDestroy()
{
	if (m_pStateMachine)
	{
		delete m_pStateMachine;
		m_pStateMachine = 0;
	}
	if (m_pMovieStyle)
	{
		delete m_pMovieStyle;
		m_pMovieStyle = 0;
	}

	S5::UTIL::SetServerProperty(m_serverProperty);
}

void CGhostPlayer::HandleMsg(const MsgObject& msg)
{
	if (Doc()->m_gameMode != 5)
		return;

	switch (msg.message)
	{
	case M983(0x12):
	{
		WSendPacket packet((enumClientPacket)0xc2);
		packet.Encode1(3);
		packet.Encode4(msg.param1);
		packet.Send(TO_GAME);

		if (msg.param1 == 0x1A000011)
		{
			CProjectG::Instance()->SetGameSpeed(3.0f);
			AfxGetTask()->GetActor("Letter")
				<< MsgObject(0, M983(0x193), 0, 0, 0, 0, 0);
		}
		break;
	}

	case M983(0xd):
		*(int*)msg.param1 = m_bShotPlaying;
		break;

	case M983(0xe):
		if (m_pAward == 0)
		{
			m_pAward = CreateForm<FrGhostAward>(g_pFresh->GetManager(), 0,
				"GHOST_AWARD", 0);
			m_pAward->Init((PLAYER(0)->order) == 1);

			if (msg.param1 != 0)
			{
				Doc()->m_bGameOver = true;
				CTaskManager::Instance()->PostMsg(this, "GolfRule", M983(0x78),
					0x20000, 0, 0, 0);
				m_pAward->Open(0, 0);
			}
			else
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", 0);
				pForm->SetTopmost(true);
				pForm->FindNextTopFocus(false);
				pForm->SetTimeLimit(5.0f);
				// "A corrupted ghost was found; the game is stopped and the result screen follows."
				pForm->SetMessage(
					K2L_Compatibility("\xBC\xD5\xBB\xF3\xB5\xC8\x20\xB5\xA5\xC0\xCC\xC5\xCD\xB0\xA1\x20\xB9\xDF\xB0\xDF\xB5\xC7\xBE\xEE\x20\xB0\xD4\xC0\xD3\xC0\xBB\x20\xC1\xDF\xB4\xDC\xC7\xCF\xB0\xED\x20\xC0\xE1\xBD\xC3\x20\xC8\xC4\x20\xBD\xC3\xBB\xF3\xBD\xC4\xC0\xBB\x20\xC1\xF8\xC7\xE0\xC7\xD5\xB4\xCF\xB4\xD9\x2E\x0A"),
					false);
				pForm->Open(
					(FRESH_PFN_RESULT)&CGhostPlayer::ExceptionAlarmWaitCallBack,
					0);
			}
		}
		break;

	case M983(0x10):
		m_bShotPlaying = 0;
		break;

	case M983(0x11):
		if (m_bShotPlaying == 1)
			m_pMovieStyle->Render();
		break;

	case M983(0xf):
	{
		// 983: lista de typeids (00a20330: so o compasso 0x1a000040, se o CItemManager o conhece);
		// manda o 0xc2/3 se o jogador tem quantidade > 0 (word +0xc do item, 0063b570)
		std::vector<unsigned long> ids;
		if (Doc()->m_itemManager.FindItem(0x1a000040))
			ids.push_back(0x1a000040);

		std::vector<unsigned long>::iterator it;
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			sItemInfo* pItem = Doc()->FindMyItemByTypeId983(*it);
			if (pItem && pItem->Common[0] > 0)
			{
				WSendPacket packet((enumClientPacket)0xc2);
				packet.Encode1(3);
				packet.Encode4(0x1A000040);
				packet.Send(TO_GAME);
				break;
			}
		}
		break;
	}
	}
}

void CGhostPlayer::SetWindAndWeather(unsigned char holeNum)
{
	const CGhostHoleData* pHoleData = m_pGhostDoc->GetHoleData(holeNum);

	if (pHoleData)
	{
		const GHOST_SHOT_DATA* pShot = *pHoleData->GetShotDataList().begin();
		const GHOST_ACT_DATA& windAction = pShot->m_acts[0];
		if (windAction.m_used)
		{
			CWindChangeState wind((unsigned char)windAction.m_args[0],
				(unsigned char)windAction.m_args[1],
				(unsigned char)windAction.m_args[2],
				(unsigned char)windAction.m_args[3]);
			wind.OnStateRun();
		}

		const GHOST_ACT_DATA& weatherAction = pShot->m_acts[1];
		if (weatherAction.m_used)
		{
			CWeatherChangeState weather((unsigned char)weatherAction.m_args[0]);
			weather.OnStateRun();
		}
	}
}

void CGhostPlayer::SendAwardResult()
{
	// HACK: dead reference; the original compiles this function one wave later, after UseItem.
	if (0)
		UseItem(0, 0);

	if (m_pAward)
	{
		GHOST_USER_DATA* pUserData = m_pGhostDoc->m_pUserData;
		WSendPacket packet((enumClientPacket)0xc2);
		packet.Encode1(6);
		packet.Encode4(pUserData->m_uid);
		packet.Encode4(CGhostManager::Instance()->GetLatestFileInfo()->m_index);
		packet.Encode1(GetCurMap());
		packet.Encode4(m_pAward->GetBasePang());
		packet.Encode4(m_pAward->GetAutoBonus());
		packet.Encode4(m_pAward->GetClickBonus());
		packet.Encode4(m_pAward->GetClickCount());
		packet.Encode4(GOLFDOC()->GetPlayer(0)->score);
		packet.Encode4(GOLFDOC()->GetPlayer(1)->score);
		sPlayerData* p0 = GOLFDOC()->GetPlayer(0);
		packet.Encode4(p0->totalPang);
		sPlayerData* p1 = GOLFDOC()->GetPlayer(1);
		packet.Encode4(p1->totalPang);
		packet.Encode1(Doc()->m_userInfo[1].stat.Level);
		packet.Encode1(Doc()->m_userInfo[0].stat.Level);
		packet.Encode1(GetHoleIndex(GOLFDOC()->m_currentHole));
		packet.Send(TO_GAME);

		if (m_errorCode)
			m_pGhostDoc->SetInvalidate();
	}
}

void CGhostPlayer::UseItem(unsigned long itemTypeId, unsigned long itemId)
{
	if (itemTypeId == 0x1A000040)
	{
		return;
	}

	AfxGetTask()->GetActor("Item")
		<< MsgObject(0, M983(0x19f), -1, itemTypeId, itemId, 0, 0);
}

bool CGhostPlayer::ExceptionAlarmWaitCallBack(int result, FrForm* pForm)
{
	Doc()->m_bGameOver = true;
	CTaskManager::Instance()->PostMsg(this, "GolfRule", M983(0x78), 0x20000, 0, 0, 0);

	if (m_pAward)
	{
		m_pAward->Open(0, 0);
	}
	else
	{
		m_pAward = CreateForm<FrGhostAward>(g_pFresh->GetManager(), 0,
			"GHOST_AWARD", 0);
		m_pAward->Init((PLAYER(0)->order) == 1);
		m_pAward->Open(0, 0);
	}

	return true;
}
