#pragma once

#include <list>

struct sSCardAvilityPeriodInfo;
class FrSPCardBuffDlg;

// JP 983: layout medido no construtor 0080a900, no Init 00809300 e nos leitores 00809640-0080a6b0 (sizeof 0x1384,
// `push 0x1384` em 00a67065). Em relacao ao 645: registro do taco por jogador (+0xa32), cartoes presos ao taco
// (+0xc20), 3 efeitos novos do cartao especial (+0xb8c, +0xba0, +0xbb4), tabela do caddie com 17 efeitos (era 13) e
// tabela nova dos cartoes de NPC (+0x10ac, 12 efeitos). Os nomes com 983 sao meus.
#pragma pack(push, 1)
class CCardManager : public WSingleton<CCardManager>
{
public:
	CCardManager();
	virtual ~CCardManager();

	void Init();
	void CalcCardPeriodAndStatus();
	void CalcCardPeriodAndStatus983(); // 0080a810: so o jogador 0, pela lista de +0x381c do CSharedDoc

	void SetCharacterCardAvility(const sSCardAvilityPeriodInfo& info, int slot);
	int SetSpecialCardAvility(const sSCardAvilityPeriodInfo& info, int slot);
	void SetCaddieCardAvility(const sSCardAvilityPeriodInfo& info, int slot);
	void SetNpcCardAvility983(const sSCardAvilityPeriodInfo& info, int slot); // 0080af90

	void AddCharacterCard983(IFF_STRUCT::sCard* pCard, int slot); // 008093d0
	void AddClubCard983(IFF_STRUCT::sCard* pCard, int slot); // 008094a0
	void AddCaddieCard983(IFF_STRUCT::sCard* pCard, int slot); // 00809570
	void AddNpcCard983(IFF_STRUCT::sCard* pCard, int slot); // 008095e0

	void SetPlayerIndex(unsigned char index);
	void SetOffLinePlayerIndex(unsigned char index);

	void SetClubInfo983(int slot, const sClubInfo& info); // 008099c0
	sClubInfo& GetClubInfo983(int slot); // 008099f0

	void SetWindColor(unsigned char color) { m_windColor = color; }
	unsigned char GetWindColor() { return m_windColor; }

	float GetTotalRange(bool bPowerRange); // 983: devolve float (00809ea0)
	int GetTotalComboGauge983(); // 00809f10
	float GetTotalPangyaZone();
	int GetPowerRangeDown();

	int GetCardPeriodStatus(int index);
	int GetCardStatusSlot(int index);
	int GetClubCardStatusSlot983(int index); // 0080a000
	int GetNpcCardStatus983(int index); // 0080a620
	float GetCardPeriodComboGauge();
	int GetCardPeriodSlot();
	float GetCardPeriodPangRate();
	float GetCardPeriodPangRate2_983(); // JP 983: 0080a3f0 (cartao de NPC, efeito 1)
	float GetPeriodRangeUp983(); // 00809710
	int GetPeriodComboGaugeUp983(); // 00809720
	unsigned char GetPeriodFlag20_983(); // 0080a560

	int GetCardSuccesssProbUp();
	float GetCardRangeUp();
	__int64 GetCardBounceBonusUp();
	float GetCardPowerRangeUp();
	int GetCardComboGaugeUp();
	float GetCardPangyaZoneUp();
	unsigned char GetWindPowerDown(float wind); // 983: recebe a forca do vento (0080a160)

	float GetSpecialCardPangyaZoneUp();
	float GetSpecialClearBonusProb(unsigned char course);
	float GetCaddieCardPangYaZoneUpWind1();
	float GetCaddieCardPangYaZoneUpWind7_983(); // 00809760 (efeito 15 do caddie)
	float GetCaddieCard16_983(); // 0080a3c0 (efeito 16 do caddie)

	int GetNpcCardValue983(int index); // 00809790
	float GetNpcCardPangyaZone983(int index); // 008097c0
	float GetNpcCard4Rate983(); // 008097f0 (efeito 4, x 0.01)
	float GetNpcCard3Rate983(); // 0080a420 (efeito 3, x 0.01)
	float GetCurrentRange983(); // 0080a470
	float GetNpcCardPangyaZoneByRange983(); // 0080a6b0

	void UpdateCardBuffDlg();
	void InitCardBuffDlgPosition();
	void ResetSpecialBuffTick();

	void SetCardBuffDlg(FrSPCardBuffDlg* pDlg) { m_pCardBuffDlg = pDlg; }
	void SetSPCardAlarm(bool bAlarm) { m_bSPCardAlarm = bAlarm; }
	bool ShowSPCardAlarm() { return m_bSPCardAlarm; }

protected:
	virtual void OnProc(float delta);

public:
	std::list<unsigned long>& GetActiveCardList() { return m_activeCardList; }

private:
	struct sCaddieCardAvility
	{
		int effect;
		int value;
		int type;
	};

public:
	unsigned char m_playerSlot[5]; // +0x28
	sCharacterInfo m_charInfo[5]; // +0x2d
	sClubInfo m_clubInfo[5]; // +0xa32

private:
	unsigned char m_pad983_abe[2];
	int m_periodStatus[5][5]; // +0xac0
	int m_periodSlot[5]; // +0xb24
	float m_periodComboGauge[5]; // +0xb38
	float m_periodPangRate[5]; // +0xb4c
	float m_periodPangyaZone[5]; // +0xb60
	float m_specialClearBonusProb[5]; // +0xb74
	unsigned char m_specialClearBonusCourse; // +0xb88
	unsigned char m_pad983_b89[3];
	float m_periodRangeUp[5]; // +0xb8c (efeito 0x1b do cartao especial)
	int m_periodComboGaugeUp[5]; // +0xba0 (efeito 0x1c)
	unsigned char m_periodFlag20[5]; // +0xbb4 (efeito 0x20)
	unsigned char m_pad983_bb9[3];
	int m_cardStatus[5][5]; // +0xbbc
	int m_clubCardStatus[5][5]; // +0xc20
	int m_powerRangeDown[5]; // +0xc84
	unsigned char m_windColor; // +0xc98
	unsigned char m_pad983_c99[3];
	sCaddieCardAvility m_caddieCard[5][17]; // +0xc9c
	unsigned long m_reserved1098; // +0x1098 (983 sem acesso: entre m_caddieCard e m_playerIndex)
	unsigned char m_playerIndex; // +0x109c
	unsigned char m_pad983_109d[3];
	std::list<unsigned long> m_activeCardList; // +0x10a0 (0xc bytes no VS2010)
	sCaddieCardAvility m_npcCard[5][12]; // +0x10ac
	FrSPCardBuffDlg* m_pCardBuffDlg; // +0x137c
	bool m_bSPCardAlarm; // +0x1380
	unsigned char m_pad983_1381[3];

	friend struct CCardManagerLayout983;
};
#pragma pack(pop)
