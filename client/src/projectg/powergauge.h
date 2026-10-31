#pragma once

#include <vector>

class CImpactZoneObject;

// JP 983 (unidade 0085b760-0085ce70; construtor 0085ca20, `operator new` 0x58 em 00a65779): a area do PangYa e a soma das
// zonas de impacto da frente B (CImpactZoneObject::m_zone, +0x64) com a do caddie, a dos cartoes, a do taco/colar (+0x44)
// e a do mascote (+0x38); a tabela de 100 rand() (+0x28) e a "sorte" sincronizada pelo servidor (0x16a do OnPacketCommon).
// Sem a lista de itens da sorte, a orelha de elfo, as asas e as pecas auxiliares do 645 (viraram zonas de impacto).
class CPowerGauge : public WSingleton<CPowerGauge>
{
public:
	CPowerGauge(); // 0085ca20
	virtual ~CPowerGauge(); // 0085c660

	void SetDebugPangYaArea(float area); // 645 (console); o 983 nao tem o campo
	void SetDebugAirNightLuckyPangYaArea(float area); // 0085b7a0 (+0x38: area do mascote, CMascot 00762f60)
	float GetDebugAirNightLuckyPangYaArea(); // 0085b830
	float GetPangYaArea(); // 0085cad0
	void SetValue40_983(float value); // 983: 0085b840 (nome provisorio; o item de jogo grava 0.0 / 1.0)
	float GetValue40_983(); // 983: 0085b850

	void ResetRandTable983(); // 0085c7e0: 100 x rand() (OnPacketCommon 0x16a depois do srand do servidor)
	void SetRandTable983(const std::vector<int>& table); // 0085cdd0
	int GetNextRand983(); // 0085bc50: proximo da tabela (circular)
	float GetMascotArea983(unsigned long typeId); // 0085b7b0
	void UpdateClubArea983(); // 0085c710: esvazia as zonas e recalcula +0x44
	void AddImpactZone983(int playerIndex, CImpactZoneObject* pObject); // 0085caf0
	float GetClubArea983(); // 0085b760

private:
	bool Init(); // 0085ca10
	float GetImpactZoneArea983(); // 0085c190
	float GetAddLuckyPangYaArea(); // 0085c1c0: caddie da sorte
	float GetAddCardArea(); // 0085bc30
	float GetPangYaAreaBase983(); // 0085c470
	float GetPangYaAreaItem983(); // 0085c4d0: com o +0x40, minimo 3
	float GetClubSetArea983(int playerIndex); // 0085beb0: taco 0x10000014 pelo nivel
	float GetLuckyNecklaceArea(int playerIndex); // 0085bfc0

	std::vector<int> m_randTable983; // +0x28
	float m_debugAirNightLuckyPangYaArea; // +0x38
	int m_randIndex983; // +0x3c
	float m_value40_983; // +0x40 (0085b840/0085b850)
	float m_clubArea983; // +0x44
	std::vector<CImpactZoneObject*> m_impactZones983; // +0x48
};
