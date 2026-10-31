#pragma once

#include <list>
#include <string>
#include "singleton.h"

class CTimerEvent
{
public:
	CTimerEvent();
	virtual ~CTimerEvent();

	virtual void OnTimer() = 0;
	void ClearTimer(const char* name);
	bool AddTimer(float time, const char* name);

protected:	// B72: a FrGrandPrixAwardDlg::OnTimer (00549210) compara e limpa o nome
	std::string m_timerName;
};

class CTimerManager : public WSingleton<CTimerManager>
{
public:
	CTimerManager();
	virtual ~CTimerManager();

	bool AddTimer(float time, const char* name, CTimerEvent* event);
	void AllClear();
	void ClearTimer(const char* name);
	void UpdateTimer(float dt);

	// D876: o GP (jp/grandprixdocument.cpp, 983 0099cae0/0099cc60) le a lista e as vagas pelos membros (cl 14.44 move os dois)
	friend class CTimerManagerGpJp;

private:
	struct sTimer
	{
		float time;
		float elapsed;
		CTimerEvent* event;
		int id; // 983: indice da vaga (AddTimer983 0099cb4a grava i; RemoveTimer983 0099cc76 procura por ele)
		std::string name;
		bool active;

		void Clear()
		{
			time = 0.0f;
			elapsed = 0.0f;
			event = NULL;
			name.clear();
			active = false;
		}
	};

	std::list<sTimer*> m_timerList;
	sTimer m_timers[10];
};
