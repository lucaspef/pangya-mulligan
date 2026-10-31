#pragma once

#include "actor.h"

class CSimpleUI;

class CItemWindow : public IActor
{
public:
	virtual ~CItemWindow() { }

	DECLARE_OBJECT(CItemWindow)

	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

	enum eState
	{
		STATE_OPENED,
		STATE_OPENING,
		STATE_CLOSING,
		STATE_CLOSED
	};

	void ResetUiPosition();
	static bool IsClosed();

protected:
	CItemWindow();

	void ResetVars();
	unsigned long* GetTidItemSlot(unsigned char player);
	void CheckInput();
	void ExecuteNow(bool bSound);
	void OnClosed(float delta);
	void OnRight(float delta);
	void OnLeft(float delta);
	void OnOpened(float delta);
	bool ProcessItem(unsigned char player, unsigned long tid, bool bConsume,
		unsigned long random);
	bool IsUsable(unsigned long tid);
	bool UseItem(int slot);
	void NetUseItem(unsigned long a, unsigned long b, unsigned long c);
	void DrawBackground983();
	void Draw();

	static eState ms_state;

	std::map<unsigned long, WOverlay*> m_iconMap;
	WOverlay* m_pOverlay;
	WRect m_dst;
	float m_curX;
	float m_targetX;
	float m_velocity;
	float m_mouseX;
	float m_mouseY;
	unsigned char m_frame;
	CSimpleUI* m_pUI;
	int m_hover;
	int m_selected;
	int m_curSlot;
	unsigned long m_itemTid;
	float m_unused74;
	float m_unused78;
	float m_effTime;
	float m_effX;
	float m_effY;
	float m_effVX;
	float m_effVY;
	bool m_bAnim;
	bool m_bFail;
	bool m_bActive;
	bool m_bOpenAllowed;
};
