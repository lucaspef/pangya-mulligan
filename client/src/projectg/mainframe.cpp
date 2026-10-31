#include "minatl.h"
#include "mainframe.h"

extern WMatrix g_camera;
extern bool g_bQuit;


static float s_lastGameMonCheck;

CMainFrame::CMainFrame()
{
	// layout do 983 (ctor 00a615e0): +0xc m_gameSpeed, +0x10, +0x14 m_processedTime, +0x28 m_drawTime
	C_ASSERT(offsetof(CMainFrame, m_gameSpeed) == 0xc && offsetof(CMainFrame, m_unusedScale983) == 0x10);
	C_ASSERT(offsetof(CMainFrame, m_processedTime) == 0x14 && offsetof(CMainFrame, m_drawTime) == 0x28);
	m_result = 0;
	m_processedTime = 0;
	m_drawTime = 0;

	m_gameSpeed = 1.0f;
	m_unusedScale983 = 1.0f;

	SetFPS(30.0f);

	InitMath();

	g_camera.Reset();
}

CMainFrame::~CMainFrame()
{
	UninitMath();
}

void CMainFrame::ResetTimer()
{
	m_startTime = GetSystemTime();
	m_processedTime = 0;

	m_drawTime = 0;
	m_frameCount = 0;
	m_drawCount = 0;
}

void CMainFrame::FixFrameRate(bool bEnable)
{
	if (bEnable)
	{
		int rate = m_drawCount * 100 / m_frameCount;

		if (GetFPS() >= 15.0f && 100 - rate >= 55)
		{
			m_fpsDivisor++;
		}

		if (m_fpsDivisor - 1 >= 1 && rate == 100 &&
			m_frameCount / m_fps * 0.5f <= m_drawTime)
		{
			m_fpsDivisor -= (m_drawTime >= m_frameCount / m_fps * 0.75f &&
								m_fpsDivisor - 2 >= 1)
				? 2
				: 1;
		}
	}

	m_drawTime = 0;
	m_frameCount = 0;
	m_drawCount = 0;
}

int CMainFrame::Main(int flags)
{
	float time = (float)GetTime();

	if (!(flags & 4))
	{
		if (m_processedTime + 1500.0f < time)
			m_processedTime = time - 100.0f;
	}

	while (time > m_processedTime)
	{
		float dt = time - m_processedTime;

		if (dt > 100.0f)
			dt = 100.0f;

		m_processedTime += dt;

		Update(!(flags & 0x10) ? (int)m_processedTime : 0);

		float sec = dt * 0.001f;

		Process(sec);
	}

	if (!m_result && !(flags & 1))
	{
		Draw();
		Paint();
	}

	if (time - s_lastGameMonCheck > 10000.0f)
	{
		g_bQuit = false;
		s_lastGameMonCheck = time;
	}

	return m_result;
}

void CMainFrame::SetFPS(float fps)
{
	m_fps = fps;
	m_fpsDivisor = 1;
	m_frameCount = m_drawCount = 0;
}

void CMainFrame::Quit()
{
	m_result = 1;
}

void CMainFrame::Restart()
{
	m_result = 2;
}
