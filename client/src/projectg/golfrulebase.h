#pragma once
#include "jp/docofs983.h" // B1460: crus do CSharedDoc do 983 ancorados no membro (cl 14.44 anda)

class CGolfRule;
struct sPlayerData;

class CGolfRuleBase
{
public:
	CGolfRuleBase();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void AddPlayer(unsigned char index, const char* name,
		unsigned long uid, unsigned long oid, unsigned long caddie);
	virtual void AddPlayerOffline(unsigned char index, const char* name,
		unsigned long oid, unsigned long caddie);
	virtual int GetNumTimeout();
	virtual bool CheckGiveUp();
	// JP 983: a tabela virtual da base tem 17 posicoes (00d1978c); [7]-[15] sao novas e o destrutor passou a [16].
	// Nomes de [8], [9], [12], [13], [15] sao os da frente B (include/jp/golfruleholeinone.h); os V*_983 nao foram lidos.
	virtual void V7_983();                      // [7] 006a7a40: calcula CSharedDoc::m_pangRate (no 645, dentro do CGolfRule)
	virtual void EnableShotReview();            // [8] 006a6050
	virtual void OnHoleIn();                    // [9] 006a69d0: estatisticas da bola no buraco
	virtual void V10_983(float time);           // [10] 006a5eb0 (vazia na base; o OnProcess 0069bb50 chama no estado 10)
	virtual bool V11_983(bool* pbCaddie);       // [11] 006a7dc0: aviso de chance (albatross/eagle/birdie) ao ator "Letter"
	virtual void ReadyNextShot();               // [12] 006a5f40
	virtual void RequestShotReview(float time); // [13] 006a7fb0 (chamada pelo CGolfRule::DoPlayerPreview)
	virtual void V14_983();                     // [14] 006a8440: comeco da premiacao (case 0x20000 do CGolfRule::ChangeGameMode do 645)
	virtual void ShowScore(int arg);            // [15] 006a8430
	virtual ~CGolfRuleBase();                   // [16] 006a5f10

	void CheckCalipersCount();
	void SendShotReview983(); // 006a71d0 (chamada logo depois do pacote 0x34): tacadas a mais no par 4/5 quando o bit 1 de m_roomInfo+0xc9 esta ligado
	void DoScoreBoard983();   // 006a80a0
	void ShowScore983();      // 006a8130 (ShowScore do HoleInOne/GuildMatch/NewApproach no 983)
	void ShowHoleBg983(unsigned char gameType); // 006a5f90
	void NextHoleOffline983(); // 006a6130
	void NextHole983();        // 006a6f90
	bool IsHoleInOne983();     // 006a6f20 (nao usa o this)
	__forceinline void SetMyTicket983(sPlayerData* pPlayer); // fim do AddPlayer 006a7470/006a9160/006ab150 (inline no 983)

protected:
	CGolfRule* m_pGolfRule;
	unsigned long m_playerColor[4];
};

class CGolfRuleStroke : public CGolfRuleBase
{
public:
	CGolfRuleStroke(CGolfRule* pGolfRule);
	virtual ~CGolfRuleStroke();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
};

class CGolfRuleTeam : public CGolfRuleBase
{
public:
	CGolfRuleTeam(CGolfRule* pGolfRule);
	virtual ~CGolfRuleTeam();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void AddPlayer(unsigned char index, const char* name,
		unsigned long uid, unsigned long oid, unsigned long caddie);
	virtual int GetNumTimeout();
	virtual bool CheckGiveUp();
	virtual bool V11_983(bool* pbCaddie); // [11] 006aacb0: matchshot/winningshot no green (o "mode == 1" do 645)
	virtual void ReadyNextShot(); // [12] 006aab70 (vtable 00d199dc + 0x30)
};

class CGolfRuleGuildMatch : public CGolfRuleBase
{
public:
	CGolfRuleGuildMatch(CGolfRule* pGolfRule);
	virtual ~CGolfRuleGuildMatch();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void ReadyNextShot();               // [12] 006a89e0 (vtable 00d1982c + 0x30)
	virtual void RequestShotReview(float time); // [13] 006a8ad0 (vtable 00d1982c)
	virtual void ShowScore(int arg);            // [15] 006a89a0 (vtable 00d1982c + 0x3c) = ShowScore983
};

class CGolfRuleSkins : public CGolfRuleBase
{
public:
	CGolfRuleSkins(CGolfRule* pGolfRule);
	virtual ~CGolfRuleSkins();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void V7_983();        // [7] 006a9da0 (vtable 00d1990c + 0x1c): pang e base de cada buraco
	virtual bool V11_983(bool* pbCaddie); // [11] 006aa070: matchshot3/winningshot3 (green) ou Approach 30/60
	virtual void ReadyNextShot(); // [12] 006aa290 (vtable 00d1990c + 0x30)
};

class CGolfRuleApproach : public CGolfRuleBase
{
public:
	CGolfRuleApproach(CGolfRule* pGolfRule);
	virtual ~CGolfRuleApproach();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();

private:
	WVector GetRandomStartPos(unsigned long oid);
};

class CGolfRuleNewApproach : public CGolfRuleBase
{
public:
	CGolfRuleNewApproach(CGolfRule* pGolfRule);
	virtual ~CGolfRuleNewApproach();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void V10_983(float time);     // [10] 006a9790 (vtable 00d198bc + 0x28): fim do tempo da Approach
	virtual bool V11_983(bool* pbCaddie); // [11] 006a96b0: devolve false (sem aviso de chance nem Letter do turno)
	virtual void ReadyNextShot(); // [12] 006a9710 (vtable 00d198bc + 0x30)
	virtual void ShowScore(int arg); // [15] 006a96d0 (vtable 00d198bc + 0x3c) = ShowScore983

private:
	WVector GetRandomStartPos(unsigned long oid);
};

class CGolfRuleMatch : public CGolfRuleBase
{
public:
	CGolfRuleMatch(CGolfRule* pGolfRule);
	virtual ~CGolfRuleMatch();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void AddPlayer(unsigned char index, const char* name,
		unsigned long uid, unsigned long oid, unsigned long caddie);
};

class CGolfRuleTimeattack : public CGolfRuleBase
{
public:
	CGolfRuleTimeattack(CGolfRule* pGolfRule);
	virtual ~CGolfRuleTimeattack();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
};

class CGolfRuleQuickrace : public CGolfRuleBase
{
public:
	CGolfRuleQuickrace(CGolfRule* pGolfRule);
	virtual ~CGolfRuleQuickrace();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
};

class CGolfRuleTutorial : public CGolfRuleBase
{
public:
	CGolfRuleTutorial(CGolfRule* pGolfRule);
	virtual ~CGolfRuleTutorial();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void AddPlayer(unsigned char index, const char* name,
		unsigned long uid, unsigned long oid, unsigned long caddie); // [3] 006aba80

protected:
	void SetEquipCharCaddieInfoForTutorial();
};

class CGolfRule30Battle : public CGolfRuleBase
{
public:
	CGolfRule30Battle(CGolfRule* pGolfRule);
	virtual ~CGolfRule30Battle();

	virtual void SetPlayer();
	virtual void SetGrade();
	virtual void SetReady();
	virtual void RequestShotReview(float time); // [13] 006a5230 (vtable 00d19704)
	virtual void V7_983();                      // [7] 006a50b0
	virtual void ReadyNextShot();               // [12] 006a5120
	virtual void V14_983();                     // [14] 006a58c0
	virtual void ShowScore(int arg);            // [15] 006a5340
	WVector GetShortGameBallPos983(unsigned long guid); // 006a5500
	void StartGrandPrixAwards983();                     // 006a55b0
	void ProcessCamera983(float time);                  // 006a5000
};

// JP 983: fim do AddPlayer da base (006a7470), do Match (006a9160) e do Team (006ab150). Se o jogador e o proprio
// (uid == CSharedDoc +0x8cb), procura a ficha 0x1a00019f na lista +0x375c do doc compartilhado e grava a quantidade no
// CGolfRule +0x244 (0066e650). O 0x32d/0x32e do CGolfRule le e gasta esse contador; o nosso nunca o preenchia.
__forceinline void CGolfRuleBase::SetMyTicket983(sPlayerData* pPlayer)
{
	if (*(unsigned long*)((char*)pPlayer + 0x4b0) != *(unsigned long*)((char*)CSharedDoc::Instance() + DOCOFS983(0x8cb)))
		return;
	int* head = *(int**)((char*)CSharedDoc::Instance() + DOCOFS983(0x375c));
	int* n = (int*)*head;
	if (n == head)
		return;
	while (n != head && n[3] != 0x1a00019f)
		n = (int*)*n;
	if (n != head)
		*(long*)((char*)m_pGolfRule + GOLFRULE_OFS_AUTOSHOT983) = (short)n[5]; // B1463: 0x244 cru andava no cl 14.44 (0x238)
}
