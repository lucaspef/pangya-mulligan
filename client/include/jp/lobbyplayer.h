#pragma once
#include <map>
#include <functional>
#include <memory>
#include "actor.h"
#include "anotherhand.h"
#include "dummyplayer.h"
#include "lobbyjp.h"

class CPartTidList;

// JP 983: `_client::CCriticalSection` com contagem de giro (vtable 00d0a5ec, 0x1c; construtor 00565e70, destrutor 00565ea0,
// Enter 00565ec0, Leave 00565ed0). A da frente A (src/projectg/lock.hpp) não tem o construtor com a contagem: empréstimo.
// (o windows.h deste build não declara a função: _WIN32_WINNT abaixo de 0x0403)
extern "C" __declspec(dllimport) BOOL __stdcall InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION pcs, DWORD spin);
class CCriticalSection983
{
public:
	CCriticalSection983(unsigned long spin) { InitializeCriticalSectionAndSpinCount(&m_cs, spin); }
	virtual ~CCriticalSection983() { DeleteCriticalSection(&m_cs); }
	void lock() { EnterCriticalSection(&m_cs); }
	void unlock() { LeaveCriticalSection(&m_cs); }
	CRITICAL_SECTION m_cs; // +0x04
};

// trava de escopo sobre a seção crítica (o 983 guarda o ponteiro dela na pilha, com estado de EH)
struct CCsGuard983
{
	CCsGuard983(CCriticalSection983& cs) : m_pCs(&cs) { m_pCs->lock(); }
	~CCsGuard983() { m_pCs->unlock(); }
	CCriticalSection983* m_pCs;
};

// JP 983: um trabalho de carga da LobbyPlayer (0x28; construtor 00567530, destrutor 00567540): o functor da outra linha e o
// pedido (posto pelo Request983 00566000, consumido pelo LoadWork983 00568730).
struct sLobbyPlayerLoad983
{
	std::tr1::function<bool()> m_loader; // +0x00
	bool m_bLoaded; // +0x18: a outra linha acabou (ou não há pedido)
	bool m_bReady; // +0x19: há pedido à espera da outra linha
	unsigned long m_tid; // +0x1c: tipo do personagem pedido
	unsigned long m_id; // +0x20: personagem do jogador pedido (-1 = nenhum)
	int m_24;
};

// retângulo do boneco na tela (00565730; padrão em 00f14668)
struct sLobbyRect983
{
	sLobbyRect983() { }
	sLobbyRect983(const sLobbyRect983& r)
	{
		x = r.x;
		y = r.y;
		w = r.w;
		h = r.h;
	}
	float x;
	float y;
	float w;
	float h;
};

// JP 983: jogador (avatar) posto no lobby (vtables 00d0a644 / 00d0a63c, sizeof 0x438, criador 00569310; par no KR 839
// 00567f90…). Dois trabalhos de carga (dois CAnotherHand): o 0 carrega um personagem do jogador (ou o tipo dado), o 1 o
// personagem padrão de um tipo (vitrine). Um boneco em carga (+0x1c) e um em cena (+0x24). Nomes dos campos e dos métodos
// sem símbolo são meus. Sessões B31 (layout) e B35 (o resto).
class LobbyPlayer : public IActor, public CHandOwner
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00569040

	LobbyPlayer(); // 005690c0
	virtual ~LobbyPlayer(); // 00568ee0 (com `delete`: 005692e0)

	virtual void OnPreLoadInit(); // 00568c10
	virtual void OnLoad(); // 00565b40 (vazia)
	virtual void OnProcess(float delta); // 00568600
	virtual void OnDisplay(); // 00566330
	virtual void OnDestroy(); // 00568dc0
	virtual void HandleMsg(const MsgObject& msg); // 005693c0

private:
	virtual void OnAnotherHand(int key, void* param); // 00565f10

	void Request983(int work, unsigned long tid, unsigned long id); // 00566000
	void SetPoint983(const char* name); // 00566350
	bool Place983(); // 00566390
	void SetVisible983(bool bVisible, bool bFade); // 00566560 (ninguém chama no 983)
	bool Fade983(float dt); // 005665c0
	int ChangeParts983(CPartTidList* pList, unsigned long hairTid, unsigned long a, bool bHair); // 005666d0
	bool SetAuxPart983(unsigned char slot, unsigned long tid); // 005668a0
	bool SetHair983(CPartTidList* pList, unsigned char color, unsigned long hairTid); // 005669f0
	bool SetUcc983(int a, unsigned long uccId); // 00566b30
	void Rotate983(float angle); // 00566c40
	bool Input983(float dt); // 00566cd0
	bool MotionEnd983(); // 00567050
	bool RandomMotion983(); // 00567140
	bool IsLoading983(); // 00567280
	bool Load983(unsigned long id, unsigned long tid); // 00567a20
	bool LoadDefault983(unsigned long tid); // 00567e10
	void Unload983(); // 005680b0
	void UnloadDefault983(); // 00568230
	bool LoadByWork983(int work, unsigned long id, unsigned long tid); // 00568480
	void Promote983(); // 005684b0
	void LoadWork983(int work); // 00568730
	bool LoadProc0_983(); // 00568860
	bool LoadProc1_983(); // 005688e0
	sLobbyRect983 GetScreenRect983(); // 00565b50

	std::tr1::shared_ptr<IDummyPlayer> m_pLoad; // +0x1c: boneco em carga
	std::tr1::shared_ptr<IDummyPlayer> m_pShow; // +0x24: boneco em cena
	std::map<unsigned long, std::tr1::shared_ptr<IDummyPlayer> > m_cache1; // +0x2c (tipo do valor suposto: nó de 0x1c)
	std::map<unsigned long, std::tr1::shared_ptr<IDummyPlayer> > m_cache2; // +0x3c (idem)
	unsigned long m_loadId; // +0x4c: personagem do boneco em carga
	unsigned long m_loadTid; // +0x50
	unsigned long m_id; // +0x54: personagem do boneco em cena
	unsigned long m_tid; // +0x58
	WVector m_pos; // +0x5c
	bool m_bDone[2]; // +0x68: o trabalho i acabou de carregar
	bool m_bPlace[2]; // +0x6a: pôr na cena o boneco do trabalho i
	int m_6c; // ninguém grava
	sLobbyPlayerLoad983 m_loads[2]; // +0x70
	CAnotherHand m_hands[2]; // +0xc0
	float m_fadeTime; // +0x3d0
	bool m_bVisible; // +0x3d4
	bool m_bFading; // +0x3d5
	int m_work; // +0x3d8: último trabalho rodado
	WMatrix m_matrix; // +0x3dc (começa na identidade 00f041d0; o Load983 copia a do boneco)
	bool m_40c; // começa em true; ninguém lê
	bool m_bAppear; // +0x40d: aparecer com efeito e som
	int m_motionParam; // +0x410: param2 da mensagem 0x199
	bool m_bRandomMotion; // +0x414: param3 da mensagem 0x199
	bool m_bLocked; // +0x415: 0x2ae / 0x2af
	bool m_bDrag; // +0x416
	CCriticalSection983 m_csPet; // +0x418
	int m_434; // fim: o 983 arredonda a 8 (0x438)

	friend struct sLobbyPlayerLayout983;
};
