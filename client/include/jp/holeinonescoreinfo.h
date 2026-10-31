#pragma once
#include <map>
#include <string>
#include <vector>

#include "fontinfo.h"

// JP 983: "%N" grande que aparece no modo hole-in-one e desliza do meio da tela para o canto (vtable 00d095ac, sizeof 0x74;
// criado em 0075200f pelo OnLoad da tela de jogo, guardado em +0x254). No 983 mora no fontinfo.cpp, entre CShotCountInfo e
// CGuildInfo. Nomes dos campos e dos métodos sem símbolo são meus.
class CHoleInOneScoreInfo : public CFontInfo
{
public:
	CHoleInOneScoreInfo(); // 0053ebc0
	virtual ~CHoleInOneScoreInfo(); // 0053ecc0

	virtual void Process(float dt); // 0052bbd0
	virtual void Display(); // 0052bc70

	void Reset983(); // 00529080

private:
	void Animate983(); // 0052ba20

	float m_startX; // +0x2c
	float m_startY; // +0x30
	float m_endX; // +0x34
	float m_endY; // +0x38
	float m_x; // +0x3c
	float m_y; // +0x40
	float m_t; // +0x44 0…1 desliza, 1…2 espera
	float m_alpha; // +0x48 0…255
	float m_scale; // +0x4c
	int m_state; // +0x50 0 parado, 1 aparece, 2 desliza, 4 espera, 3 some
	int m_54[2]; // +0x54 (o construtor não grava; pFont/pInfo de um 2º sFont sem uso)
	std::map<float, sScaledFont> m_fonts2; // +0x5c (nó 0x38 no 983, como o sFont::scaled)
	float m_dt; // +0x6c
	int m_percent; // +0x70
};

// JP 983: vozes da torcida na sala de espera do campo 0x40 (o do hole-in-one): troca a música por um "oneshot_hio1_N" sorteado
// a cada 5…9 s e devolve a música ao sair (sizeof 0x54, sem vtable; criada com `new` pelo FrGameRoomDlg — chamadores 005087aa, 0050b549,
// 004ff1d6 e 00503350). Nome da classe e dos campos meus.
class CRoomCheer983
{
public:
	CRoomCheer983(unsigned char course); // 0053f0d0
	~CRoomCheer983(); // 0053f010

	void Process(const float& dt); // 0053ef40
	void SetRestoreBgm983(bool b) { m_restoreBgm = b; } // 983 004ff2e0: msg 0x314 da sala

private:
	void Add983(const std::string& name) { m_names.push_back(name); } // embutida (um temporário por chamada no 983)

	std::vector<std::string> m_names; // +0x00 (0x10)
	std::string m_10; // +0x10 (só construído e destruído aqui)
	std::string m_prevBgm; // +0x2c
	float m_timer; // +0x48
	float m_interval; // +0x4c
	unsigned char m_course; // +0x50
	bool m_enabled; // +0x51
	bool m_restoreBgm; // +0x52
};
