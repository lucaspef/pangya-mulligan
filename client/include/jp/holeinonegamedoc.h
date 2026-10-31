// JP 983: `_gateway::_golfgame::CHoleInOneGameDoc` (documento do modo hole-in-one; unidade 0057062e–~00579640, vtable 00d0aba4,
// sizeof 0x90 pelo `new` de 00579370, tipo de conteúdo 0xaa). Layout pelo construtor 00579140 e pelos métodos (sessão B25).
// Os nomes `Fn…_983` são os provisórios da B23 (o CHoleInOneGameHandler os usa); o que cada um faz está no comentário.
#pragma once
#include "gatewaydocjp.h"

#include <map>
#include <vector>

// 8 bytes: posição possível do buraco. O 983 copia os dois floats pela FPU (cópia e atribuição escritas à mão): tem a cara do
// WVector2D do engine, que no fonte da frente A não tem construtor de cópia — por isso a classe própria (pedido B → A da B25).
struct sHioSpot983
{
	sHioSpot983() { }
	sHioSpot983(float x, float z) : m_x(x), m_z(z) { }
	sHioSpot983(const sHioSpot983& v) : m_x(v.m_x), m_z(v.m_z) { }
	sHioSpot983& operator=(const sHioSpot983& v) { m_x = v.m_x; m_z = v.m_z; return *this; }
	sHioSpot983 operator-(const sHioSpot983& v) const { return sHioSpot983(m_x - v.m_x, m_z - v.m_z); }
	float SquareMagnitude() const { return m_x * m_x + m_z * m_z; }
	float m_x;
	float m_z;
};
struct sHioPair983 { int m_a; int m_b; }; // 8 bytes (dois Decode4 da mensagem 0x18)
struct sHioRecord983 // 0x10 (param2 da mensagem 0x0d)
{
	sHioRecord983() { }
	sHioRecord983(unsigned long uid, int a, int b, int c) : m_uid(uid), m_04(a), m_08(b), m_0c(c) { } // cópia fora de linha em 00570640
	unsigned long m_uid;
	int m_04;
	int m_08;
	int m_0c;
};
struct sHioSpotDist983 // 0xc: índice da posição + vetor até o buraco (ordenação de 00578990)
{
	unsigned int m_index;
	float m_dx;
	float m_dz;
};

// Registro do mapa de CItemManager+0x2e8 (CSharedDoc+0x20c): flag9c bit 2 = entra na lista do modo, bit 1 = sai da
// lista em 00578580, == 2 no predicado 00570700. B1574: era visao sItemRecord983_14; agora o tipo real.
namespace IFF983 { struct sCupInCondition; }
class CItemTableJpHio
{
public:
	typedef std::map<unsigned int, IFF983::sCupInCondition> RecordMap;
	IFF983::sCupInCondition* Find983(unsigned long typeId); // 00a191c0 (tabela em CSharedDoc+0x20c)
	RecordMap& GetMap983(); // 009da0a0: `this + 0x2e8`
};

namespace _gateway
{
namespace _golfgame
{
class CHoleInOneGameDoc : public CGatewayControlDocJp
{
public:
	typedef std::map<unsigned long, sHioRecord983> RecordMap;
	typedef std::vector<IFF983::sCupInCondition*> ItemList;
	typedef std::vector<sHioPair983> PairList;
	typedef std::map<unsigned long, PairList*> PlayerPairMap;

	CHoleInOneGameDoc(int type); // 00579140
	virtual ~CHoleInOneGameDoc(); // 00579270 (00579510 com o delete)
	virtual bool Initialize(); // 00570660
	virtual bool Release() { return true; } // 005706b0 (slot 2)
	virtual bool Reset(); // 005793e0 (slot 3; o Initialize chama pela vtable)
	virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; } // 00579260

	static CGatewayControlDocJp* __fastcall Create(); // 00579370: `new CHoleInOneGameDoc(0xaa)`

	int GetMine0c983(); // 005706c0: m_pMine->m_0c (chamado de 0053b5c5 e 0053db66, fora da unidade)
	void Fn5706d0_983(); // ++m_28, m_24 = 0 (mensagem 0x0e com param2 != 0)
	void Fn5706e0_983(); // ++m_24, ++m_28 (mensagem 0x0e com param2 == 0)
	void Fn570750_983(); // zera m_3c/m_40/m_44 (mensagem 0x15)
	void SetItemTotal983(int total); // 00570760: m_58 = total (mensagem 0x10)
	void Fn570770_983(); // m_58 = 0 (mensagem 0x0e)
	bool SetRecord983(const sHioRecord983* p); // 00575ee0: procura p->m_uid em m_records e copia os 16 bytes (mensagem 0x0d)
	void Fn576690_983(); // m_items += todo registro da tabela com o bit 2 de flag9c (slot 5 do handler)
	bool Fn576820_983(unsigned long typeId); // m_items += registro de typeId, se existe (mensagem 0x0f)
	void Fn577220_983(); // um registro zerado por jogador da sala; m_pMine = o do próprio jogador (slot 5 do handler)
	void SetPlayerPairs983(unsigned long uid, PairList* pPairs); // 00577310: m_playerPairs[uid] se ainda não existe (mensagem 0x18)
	PairList* GetPlayerPairs983(unsigned long uid); // 005755d0: m_playerPairs[uid] ou NULL (placar do resultdlg)
	void Fn577ed0_983(); // ordena m_items por flag9c (mensagem 0x0f)
	void Fn578580_983(); // tira de m_items os registros com o bit 1 de flag9c (mensagem 0x0f)
	void SortSpots983(); // 00578990: reordena m_spots pela distância ao buraco (sem chamador por `call` no exe)
	void ResizePlayers983(int n); // 00579590: esvazia e redimensiona m_players; limpa m_playerPairs (mensagem 0x18)
	// Cópias fora de linha em 005795e0–00579630 (faixa do handler, nenhuma chamada vista no exe):
	int Get28_983() { return m_28; } // 005795e0
	void Set44_983(float v) { m_44 = v; } // 005795f0
	void Set3c_983(float v) { m_3c = v; } // 00579600
	void Set40_983(float v) { m_40 = v; } // 00579610
	float Get3c_983() { return m_3c; } // 00579620
	float Get40_983() { return m_40; } // 00579630

	CGatewayHandler* m_pHandler; // +0x0c: ator "CHoleInOneGameHandler" criado em Initialize
	RecordMap m_records; // +0x10: uid → registro (nó de 0x24)
	sHioRecord983* m_pMine; // +0x20: registro do próprio jogador
	int m_24; // +0x24
	int m_28; // +0x28 (o byte baixo vai para a ficha da sala +0x80 na mensagem 0x0d)
	ItemList m_items; // +0x2c
	float m_3c; // +0x3c (mensagem 0x07, param2; gravado pela 0x08)
	float m_40; // +0x40 (mensagem 0x07, param3; gravado pela 0x08)
	float m_44; // +0x44 (gravado pela mensagem 0x05, lido pela 0x06)
	std::vector<sHioSpot983> m_spots; // +0x48: posições possíveis do buraco (x, z); também vai por endereço a CGolfDoc 00661f30
	int m_58; // +0x58
	std::vector<PairList> m_players; // +0x5c: um vetor de pares por jogador (mensagem 0x18)
	PlayerPairMap m_playerPairs; // +0x6c: uid → vetor de pares (nó de 0x18)
	int m_7c; // +0x7c (1 em Initialize; mensagem 0x09)
	int m_80; // +0x80 (1 em Initialize)
	int m_84; // +0x84 (0 em Initialize)
	unsigned int m_spot; // +0x88: posição atual na lista m_spots
	bool m_8c; // +0x8c (o transformador Expert do handler lê este byte pelo contêiner 0xaa: inclina as peças)
};
}
}
