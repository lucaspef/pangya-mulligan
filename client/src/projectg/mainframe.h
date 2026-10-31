#pragma once

class CMainFrame
{
public:
	CMainFrame();
	virtual ~CMainFrame();

	int Main(int flags);
	void SetFPS(float fps);
	float GetFPS() const { return m_fps / m_fpsDivisor; }
	void SetGameSpeed(float speed) { m_gameSpeed = speed; }
	float GetGameSpeed() const { return m_gameSpeed; }

protected:
	virtual void Process(float dt) = 0;
	virtual void Draw() = 0;
	virtual void Paint() { }
	virtual unsigned long GetSystemTime() const = 0;
	virtual void Update(int time) { }

	// JP 983 (F5 C683): leem m_gameSpeed direto por g_pMainFrame983 (983 00e47e20, +0xc), como o 983; o getter muda a ordem do FPU
	friend class CAIPlayer;
	friend class CTutorGuide;

	void ResetTimer();
	void Quit();
	void Restart();

	int m_result;
	float m_fps;
	float m_gameSpeed;
	float m_unusedScale983; // 983 +0x10: so o ctor 00a615fe grava 1.0f; sem leitor no 983 (mainframe/CProjectG/g_pMainFrame983)
	float m_processedTime;
	unsigned long m_startTime;
	int m_frameCount;
	int m_drawCount;
	int m_fpsDivisor;
	float m_drawTime;

private:
	void FixFrameRate(bool bEnable);
	unsigned long GetTime() const { return GetSystemTime() - m_startTime; }
};
