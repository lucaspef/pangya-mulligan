#pragma once

#include "wlist.h"
#include "singleton.h"

enum eButton
{
	LEFT_BUTTON,
	RIGHT_BUTTON,
	MIDDLE_BUTTON
};

class CInputManager : public WSingleton<CInputManager>
{
public:
	enum eInputMode
	{
		INPUT_GAME,
		INPUT_CHAT,
	};

	struct KeyPair;

	CInputManager()
		: m_keyList(32, 32), m_sensitivity(1.0f), m_bSwapButton(false)
	{
		Reset();
	}
	virtual ~CInputManager();

	void Reset();
	void Update();

	void Register(const char* name, int key, unsigned long flag, int type);
	void EraseAllList();

	int Get(const char* name, bool bCheckMode);
	int Get(int key);
	int GetList();
	int GetDown(const char* name, bool bCheckMode);
	int GetUp(const char* name);
	void ExclusiveGetDownUseDone(const char* name);
	unsigned long GetEventTime(const char* name);

	void IncludeKey(const char* name);
	void ExcludeKey(const char* name);
	void BlockKey(const char* name);
	void FreeKey(const char* name);
	void BlockKey();
	void FreeKey();
	void SetFakeInput(const char* name, bool bDown);

	int GetButton(eButton button);
	bool GetDoubleClick(eButton button);
	unsigned long GetEventTime(eButton button);
	void SetFakeInput(eButton button);

	WVector GetButtonDownPos(eButton button)
	{
		return m_button[button].downPos;
	}

	WVector GetMouseDelta();
	void SetMousePoint(float x, float y);
	void SetMousePointToLoginBox();
	void SwapMouseButton(bool bSwap);

	void SetInputMode(eInputMode mode) { m_inputMode = mode; }

	WVector GetMousePoint() const { return m_mousePoint; }
	void FixPointer(bool bFix) { m_bFixPointer = bFix; }

	void SetSensitivity(float sensitivity) { m_sensitivity = sensitivity; }
	float GetSensitivity() const { return m_sensitivity; }

	void BlockMouse();
	void FreeMouse();
	void BlockMouse(eButton button) { m_button[button].bBlock = true; }
	void FreeMouse(eButton button) { m_button[button].bBlock = false; }

	void SetActive(bool bActive) { m_bActive = bActive; }

private:
	struct sButton
	{
		int state;
		WVector downPos;
		WVector upPos;
		unsigned long interval;
		unsigned long lastTime;
		bool bFake;
		bool bBlock;
	};

	WList<KeyPair*> m_keyList;
	WVector m_mouseDelta;
	WVector m_mousePoint;
	bool m_bFixPointer;
	sButton m_button[3];
	unsigned long m_mouseFlag;
	eInputMode m_inputMode;
	float m_sensitivity;
	bool m_bActive;
	bool m_bSwapButton;
};

extern CInputManager* g_input;

class WInputDev;
extern WInputDev* g_ime;
extern WInputDev* g_mouse;
