#pragma once

#include <map>
#include "singleton.h"

class WOverlay;

class CHeadIcon : public WSingleton<CHeadIcon>
{
	struct sPlayerHead
	{
		unsigned short icon;
		float time;
		WVector screen;
		WVector unused14;
		float dist;
		WOverlay* pLevel;

		sPlayerHead() { }
	};

public:
	CHeadIcon();
	virtual ~CHeadIcon();

	void SetLevel();
	int GetPlayerNum983(); // 006e4a80
	void SetPlayerPos(int index, WVector& pos);
	bool SetIcon(unsigned long uid, unsigned short icon);
	bool SendIcon(unsigned short icon);
	void Process(float dt);
	void DisplayOverPlayer();
	void DisplayBesideBoard(float left, float right);

	bool GetVisible() const { return m_bShow; }
	void Show() { m_bShow = true; }
	void Hide() { m_bShow = false; }
	void Toggle() { m_bShow = !m_bShow; }
	// 983 (s100): campos novos para o pódio do Grand Prix (B75); inline como o 983 (os leitores leem +0x2c/+0xf4/+0xf8 direto)
	void SetPlayerNum983(int num) { m_playerNum983 = num; }
	void SetPodiumOffset983(float x, float y) { m_podiumOffX983 = x; m_podiumOffY983 = y; }
	float GetPodiumOffX983() const { return m_podiumOffX983; }
	float GetPodiumOffY983() const { return m_podiumOffY983; }

private:
	void DisplayOverPlayer(unsigned char index, bool bName);

	// 983 (construtor 006e5400, destrutor 006e51d0; pedido B75, s100): m_bShow em +0x28 (depois do BaseObject do WSingleton),
	// int novo em +0x2c, m_head em +0x30, dois floats novos em +0xf4/+0xf8; sizeof 0x100 -> 0x10c (new 0x10c em 00a6978a). Nomes dos novos: da B (B75).
	bool m_bShow;
	int m_playerNum983; // +0x2c: nº de jogadores do desenho; -1 (construtor) = o do CGolfDoc (GetPlayerNum983 006e4a80); o pódio do GP põe 3/4
	sPlayerHead m_head[4]; // +0x30
	WOverlay* m_pEmoticon; // +0xd0
	WRect m_src; // +0xd4
	WRect m_dst; // +0xe4
	float m_podiumOffX983; // +0xf4: 0 no construtor; o pódio do Grand Prix zera / põe -30
	float m_podiumOffY983; // +0xf8
	std::map<unsigned long, WOverlay*> m_iconMap; // +0xfc
};
