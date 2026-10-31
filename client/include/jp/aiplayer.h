#pragma once
#include <vector>
#include <string>
#include "actor.h"
#include "icontentsdatacontainer.h"

// JP 983: documento dos jogadores de IA (IContentsDataContainer do tipo 0xa3; código em 00909…–00911…, frente A — classe
// ainda sem nome). Só o que a CAIPlayer chama; nomes meus.
struct sAIDocInfo983;
class CAIDocJp : public IContentsDataContainer
{
public:
	int GetCount983(); // 009091b0
	bool IsBusy983(); // 0090a520
	int GetInfo983(int index, int hole, sAIDocInfo983* pOut); // 0090d410
	bool Fn90fe00_983(int a, int b); // 0090fe00
	void Fn90d900_983(int kind, int param); // 0090d900
	void Fn911180_983(); // 00911180
	int GetNpcId983(int index); // 00909180: número do adversário pronto (0 = sem)
};

// registro devolvido pelo documento (0x28; 0046ec20 só lê o +8)
struct sAIDocInfo983
{
	sAIDocInfo983()
		: m_00(-1), m_04(false), m_08(0), m_pang(0), m_bonusPang(0), m_20(0)
	{
	}
	int m_00;
	bool m_04;
	int m_08; // +0x08: resultado do buraco em relação ao par (vai para sRivalData::holeScore)
	int m_0c;
	__int64 m_pang; // +0x10 (vai para sRivalData::holePang)
	__int64 m_bonusPang; // +0x18 (somado em sRivalData::totalBonusPang)
	int m_20;
	int m_24;
};

// tempos de uma tacada (0x0c; moldes de vetor próprios, 00472740 / 0046f270 / 0046fe90 — não é WVector). Nomes meus.
struct sAITime983
{
	float x; // duração de uma tacada (frames da vez ÷ tacadas)
	float y; // instante do começo
	float z; // y + sorteio dentro do resto da tacada
};

// um jogador de IA da partida (0x30; cópia em 00474370 / 00473e00, destrutor em 00474590)
struct sAIPlayerInfo983
{
	sAIPlayerInfo983()
		: m_step(0), m_bDone(false), m_phase(0)
	{
	}
	~sAIPlayerInfo983() { Clear983(); }
	void Clear983() // 00472cc0
	{
		if (m_path.size())
			m_path.clear();
		if (m_times.size())
			m_times.clear();
	}
	void MakeTimes983(int strokes, float frame); // 00473500

	int m_index; // +0x00
	int m_state; // +0x04: 1 = andando (ao criar e na mensagem 0x348), 0 = entre duas tacadas, 2 = chegou ao buraco
	unsigned short m_step; // +0x08: tacada corrente (índice em m_path / m_times)
	bool m_bDone; // +0x0a (0046ec90)
	std::vector<WVector> m_path; // +0x0c: onde a bola para em cada tacada
	int m_phase; // +0x1c: 0 espera, 1 antes da tacada, 2 bola no ar (CAIPlayer::OnProcess)
	std::vector<sAITime983> m_times; // +0x20: um por tacada (sAIPlayerInfo983::MakeTimes983)
};

// JP 983: ator "AIPlayer" — jogadores de IA da partida (vtable 00cfe584, sizeof 0x5c, criador 00475220; par no KR 839
// 00478e80…). A unidade vai de ~0046d8e0 a 004756f9 (o corte da F3.7.1, 00473f14, pegava só o fim). Nomes dos campos e dos
// métodos sem símbolo são meus.
class CAIPlayer : public IActor
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 004750c0

	CAIPlayer(); // 00475080
	virtual ~CAIPlayer(); // 00475120 (com `delete`: 004752c0)

	virtual void OnPreLoadInit(); // 0046ebe0
	virtual void OnLoad(); // 00471980
	virtual void OnInit(); // 0046f7e0
	virtual void OnProcess(float delta); // 004719a0
	virtual void OnDestroy(); // 0046ec10
	virtual void HandleMsg(const MsgObject& msg); // 00475560

private:
	void Release983(); // 0046da20
	bool CanBuild983(); // 0046da40
	int GetInfo983(int index, int hole, sAIDocInfo983* pOut); // 0046da60
	bool Fn46da70_983(int a, int b); // 0046da70
	void Reset983(); // 0046da90
	void IncDone983(); // 0046daa0
	int GetInfo08_983(int index, int hole); // 0046ec20
	bool IsValid983(unsigned int index); // 0046ec60
	void SetDone983(int index, bool bDone); // 0046ec90
	bool IsAllDone983(); // 0046ecd0
	int Rand983(unsigned int n); // 0046ef50
	int ShotKind983(int score); // 0046fa00
	int HumanCount983(); // 0046fa60
	void GetNpc983(int index, struct sAwardNpc983* pOut); // 0046fa90
	unsigned long GetCharType983(int index); // 0046fad0
	int HumanIndex983(int index); // 004701e0
	int FlyFrame983(const WVector* pPos); // 00470420
	void Arrive983(int index); // 004713b0
	void NextStep983(int index); // 004714b0
	void GetName983(int index, std::string* pName); // 00471ae0
	bool IsOnGreen983(const WVector* pPos); // 0046fbd0
	bool CalcPath983(int hole, int kind, int strokes, std::vector<WVector>* pPath); // 00474bb0
	// cálculo do caminho (sessão B38). Vivos: o que o CalcPath983 chama.
	float Dist983(WVector a, WVector b); // 0046ed40: distância no plano xz
	void Dir983(const WVector* pFrom, const WVector* pTo, WVector* pOut); // 0046ed90: direção no plano xz
	bool IsFair983(const WVector* pPos); // 0046f840
	void Turn983(const WVector* pDir, WVector* pOut); // 0046f8b0: gira a direção de um ângulo sorteado
	void Around983(const WVector* pCenter, const WVector* pFrom, float dist, WVector* pOut); // 0046f950
	const WVector* Pin983(); // 0046fba0
	bool LastNode983(class CNodeContainer* pNode, WVector* pOut); // 0046ecf0: último ponto da linha
	void Spread983(const WVector* pPin, const WVector* pNode, float dist, WVector* pOut); // 00470150
	void Jitter983(const WVector* pNode, WVector* pOut); // 00471260
	bool JitterNodes983(class CNodeContainer* pSrc, class CNodeContainer* pDst); // 004736a0
	bool Approach983(int count, class CNodeContainer* pSrc, class CNodeContainer* pDst); // 00473780
	bool CopyNodes983(class CNodeContainer* pSrc, class CNodeContainer* pDst, int count); // 004739c0
	bool Extend983(int extra, int hole, class CNodeContainer* pSrc, class CNodeContainer* pDst); // 00473a40
	bool MakePath983(int hole, int strokes, class CNodeContainer* pSrc, class CNodeContainer* pDst); // 004740f0
	bool PickLine983(int hole, int kind, int strokes, std::vector<class CNodeContainer>* pNodes, class CNodeContainer* pOut); // 004747c0
	// Sem chamador no 983 (seletor de tacada por tipo: código morto que o linker manteve).
	float Reach983(const WVector* pFrom, const WVector* pTo, const WVector* pTarget); // 0046ede0
	bool GreenFail983(int strokes, float dist); // 0046fb10
	bool HoleInFail983(); // 0046fb50
	bool CupPass983(float dist); // 0046fb70
	bool IsOut983(const WVector* pPos); // 0046fc50
	int RandRange983(int lo, int hi); // 0046fcb0
	bool IsBehind983(const WVector* pTarget, const WVector* pFrom); // 00470440
	float BallSize983(); // 004704f0
	int ShotResult983(const WVector* pPos, unsigned short strokes, float dist); // 004705a0
	void ShotLong983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut); // 00470620
	void ShotShort983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut); // 00470800
	void ShotNearPin983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut); // 004709e0
	void ShotGreenEdge983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut); // 00471630
	void ShotOut983(const WVector* pTarget, const WVector* pFrom, int n, WVector* pOut); // 00470bf0
	void GreenEdge983(const WVector* pDir, WVector* pOut); // 00470da0
	bool Shot983(WVector from, WVector to, int n, int kind, WVector* pOut); // 00471b60
	int ToIndex983(int player); // 004701f0
	void Start983(); // 00470210: começo da vez — resultado do buraco de cada IA vai para o m_rivalList
	void Fn470380_983(int index); // 00470380: refaz os totais (placar e pang) do rival da IA `index`
	void Fn471170_983(); // 00471170: lê grandprix_ai_player.lua
	void Finish983(int index); // 004715f0
	void Fn471e50_983(); // 00471e50: põe um sRivalData por IA no m_rivalList
	void Build983(int hole); // 004752f0

	int m_count; // +0x18: jogadores de IA do documento
	int m_done; // +0x1c: quantos já terminaram a vez
	std::vector<sAIPlayerInfo983> m_players; // +0x20
	unsigned char m_hole; // +0x30: posição do buraco corrente na ordem da partida (0…)
	unsigned char m_curHole; // +0x31: número do buraco corrente (CGolfDoc::m_currentHole)
	float m_time; // +0x34
	bool m_bTurn; // +0x38
	int m_greenFrame; // +0x3c: on_green_ball_fly_speed_frame
	int m_outFrame; // +0x40: outside_green_ball_fly_speed_frame
	int m_frame; // +0x44: o último enviado ao "Spot"
	float m_holeInDist; // +0x48: HoleIn_Valid_Distance
	int m_greenFailRate; // +0x4c: on_green_fail_rate
	int m_holeInFailRate; // +0x50: in_HoleIn_Valid_dist_fail_rate
	int m_cupPassRate; // +0x54: HoleCupPassRate
	CAIDocJp* m_pAIDoc; // +0x58

	friend struct sAIPlayerLayout983;
};
