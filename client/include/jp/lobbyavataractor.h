#pragma once
#include <list>
#include <string>
#include <vector>
#include "actor.h"
#include "anotherhand.h"
#include "lobbyjp.h"
#include "lobbyplayer.h" // CCriticalSection983

class CPartTidList;
class WOverlay;
struct w_motion_data;
namespace lua_system
{
	class lua_script;
}

// CExhibition do 983 (frente A: src/projectg/exhibition.h ainda é o do 645, sem virtuais). Só o que a LobbyAvatarActor usa;
// sizeof 0x64c pelo `new` de 005920f0 e 0058f9b0. EMPRÉSTIMO (pedido B39 em out/PEDIDOS.md); nomes dos métodos pelo 645.
class CExhibition983
{
public:
	CExhibition983(float x, float y, float anchor, unsigned long uid, bool bAutoRecover); // 00877e30
	virtual ~CExhibition983();
	virtual void Display(float x, float y); // [1]
	virtual bool SetModel(CPartTidList& parts, unsigned long* auxParts, const char* motion, float delay, int option); // [2]

	bool SetModel(unsigned long typeId); // 00877ff0
	void Process(float dt, bool bInput);
	bool SetMotion(const char* name, bool bContinue, float time);
	void SetDistance(float distance, bool bZoom, bool bFixed); // 00874a20
	void SetArea(float w, float h); // 00873900
	void SetCenter(float x, float y); // 00873e30
	const WVector2D& GetCenter(); // 008738d0
	void SetDirection(float direction); // 00873830
	const char* GetMotionName(); // 00873810
	w_motion_data* GetMotionData(); // 008739f0
	bool InValidArea(); // 00873d50
	void AttachAngelWingFx(); // 00873aa0 (nome do 645)
	void SetChatFacial(int facial, unsigned long time); // 00875c30 (nome do 645)
	void SetDistanceRange(float fMin, float fMax) // embutida (0050bac4)
	{
		m_fDistanceMin = fMin;
		m_fDistanceMax = fMax;
	}

	char m_004[0xa4];
	CPetFrame* m_pPetFrame; // +0xa8
	char m_0ac[0x54];
	float m_fDistanceMin; // +0x100 (645: SetDistanceRange)
	float m_fDistanceMax; // +0x104
	char m_108[0x454];
	unsigned long m_tid; // +0x55c
	char m_560[4];
	unsigned long m_flags; // +0x564
	char m_568[0x18];
	unsigned long m_partTids[24]; // +0x580 (CPartTidList)
	char m_5e0[0x60];
	bool m_b640; // a LobbyAvatarActor zera ao pôr em cena
	bool m_bFocused; // +0x641: Fresh::IsDesktopFocused
	bool m_bMotionToggle; // +0x642: mensagem 0x199 com o mesmo movimento
	bool m_bRandomMotion; // +0x643: troca o movimento de espera sozinho
	char m_644[8];
};
// vtable 00d05e9c (4 virtuais; o construtor é o da base e a vtable é posta pelo chamador)
class CExhibitionByHardwareTnL983 : public CExhibition983
{
public:
	CExhibitionByHardwareTnL983(float x, float y, float anchor, unsigned long uid, bool bAutoRecover)
		: CExhibition983(x, y, anchor, uid, bAutoRecover)
	{
	}
	virtual ~CExhibitionByHardwareTnL983();
	virtual void Display(float x, float y);
	virtual bool SetModel(CPartTidList& parts, unsigned long* auxParts, const char* motion, float delay, int option);
};

// JP 983: pedido de um boneco do lobby (0x248; construtor 0058f910, destrutor 0058ea00). Vem do documento 0xa0 (bonecos de
// outros jogadores; bOwn = false) ou é criado aqui com um personagem do próprio jogador.
#pragma pack(push, 1)
struct sLobbyAvatarInfo983
{
	sLobbyAvatarInfo983(); // 0058f910
	unsigned long uid; // +0x000
	sCharacterInfo info; // +0x004 (0x201)
	char m_205[3];
	std::string name; // +0x208
	std::string title; // +0x224
	int titleType; // +0x240: 0 nenhum, 1 o texto de `title`, 2 `title` traduzido
	bool bOwn; // +0x244: criado por esta unidade (apagar depois de carregar)
	char m_245[3];
};
#pragma pack(pop)

// boneco carregado (0x3c; destrutor 0058ea90): elemento da lista de +0x34 e do vetor de +0x40
struct sLobbyAvatar983
{
	CExhibition983* pExh;
	std::string name; // +0x04
	std::string title; // +0x20
};

// par de floats de cópia trivial (o 983 copia por `mov`, não por `fld`/`fstp` como o WVector2D)
struct sLobbyPos983
{
	float x;
	float y;
};
inline sLobbyPos983 MakePos983(float x, float y)
{
	sLobbyPos983 v;
	v.x = x;
	v.y = y;
	return v;
}

// posição de um boneco na faixa do lobby (0x1c): anda em x até o alvo
struct sLobbyAvatarMove983
{
	sLobbyAvatarMove983(CExhibition983* p); // 0058d2b0
	sLobbyAvatarMove983(CExhibition983* p, sLobbyPos983 v, float speed_); // 0058d300
	void SetPos(sLobbyPos983 v); // 0058c480
	void SetTarget(sLobbyPos983 v); // 0058c4b0
	void Step(float dt); // 0058c4d0
	void Reset(); // 0058c570

	CExhibition983* pExh;
	sLobbyPos983 pos; // +0x04
	sLobbyPos983 target; // +0x0c
	int state; // +0x14: 0 parado, 1 indo para a esquerda, 2 para a direita
	float speed; // +0x18
};

// passeio do mascote (0x28): sorteia um ponto, vira e anda; parado, espera 3 s
struct sLobbyMascotWalk983
{
	sLobbyMascotWalk983(CExhibition983* p, WVector2D pos, float speed); // 0058de30
	~sLobbyMascotWalk983(); // 0058d370
	void SetTarget(const WVector2D* p); // 0058d390
	void Pick(); // 0058d450
	void Process(float dt); // 0058deb0

	CExhibition983* pExh;
	int dir; // +0x04: 0 esquerda, 1 direita, 2 parado
	int prevDir; // +0x08
	float tx; // +0x0c
	float ty;
	sLobbyAvatarMove983* pMove; // +0x14
	float targetAngle; // +0x18
	float angle; // +0x1c
	float speed; // +0x20
	float wait; // +0x24
};

// faixa com o título do boneco (0x38): três imagens (ponta, meio, ponta) e o texto
struct sLobbyAvatarTitle983
{
	sLobbyAvatarTitle983(const char* s, const WVector2D* pos); // 0058fe80
	~sLobbyAvatarTitle983(); // 0058eb20
	void SetText(const char* s); // 0058f560
	void Display(); // 0058dfc0

	std::string text;
	WOverlay* img[3]; // +0x1c
	int width; // +0x28: largura do meio (potência de 2 que cobre o texto)
	float x; // +0x2c
	float y;
	bool bShow; // +0x34
};

// botão "esconder a interface" (0x24)
struct sLobbyHideButton983
{
	sLobbyHideButton983(const WVector2D* pos, const char* a, const char* b, const char* c); // 0058d4d0
	~sLobbyHideButton983(); // 0058d570
	void Process(); // 0058d5c0
	void Display(); // 0058c5d0

	int state; // 1 = o ponteiro está em cima
	short m_04;
	_WRECT rect; // +0x08
	WOverlay* img[3]; // +0x18
};

// JP 983: bonecos 2D do lobby (vtables 00d0b524 / 00d0b51c, sizeof 0x278, criador 00592570; par no KR 839). Fundo do lobby,
// até `max_avatar_num` personagens (os do documento 0xa0 e os do próprio jogador) carregados numa linha à parte
// (CAnotherHand), o mascote que passeia e o botão de esconder a interface. Nomes dos campos e dos métodos sem símbolo são
// meus. Sessão B39.
class LobbyAvatarActor : public IActor, public CHandOwner
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00590ef0

	LobbyAvatarActor(); // 00591820
	virtual ~LobbyAvatarActor(); // 00590da0 (com `delete`: 00591a00)

	virtual void OnPreLoadInit(); // 0058d120
	virtual void OnLoad(); // 0058c3f0 (vazia)
	virtual void OnInit(); // 0058c400 (vazia)
	virtual void OnProcess(float delta); // 005928e0
	virtual void OnDisplay(); // 0058e610
	virtual void OnDestroy(); // 00592610 (salto para 00592030)
	virtual void HandleMsg(const MsgObject& msg); // 00592620

private:
	virtual void OnAnotherHand(int key, void* param); // 00592f50

	void SetBg983(const char* name); // 0058c410
	bool IsBusy983(); // 0058d340
	void Layout983(); // 0058d150
	const char* TitleText983(int type, std::string* s); // 0058ddf0
	void UpdateFocus983(); // 0058eae0
	void Kick983(); // 0058f4b0
	void LoadMascot983(); // 0058f9b0
	void Request983(sLobbyAvatarInfo983* p); // 00590f00
	void Init983(); // 005910d0
	void AddRandomChars983(int n); // 00591390
	void AddDummies983(); // 00591600 (sem chamador no 983)
	void Collect983(); // 00591a30
	void Unload983(); // 00591ce0
	void Destroy983(); // 00592030
	void LoadAvatar983(sLobbyAvatarInfo983* p); // 005920f0
	void AddMyChars983(); // 00592370
	void Start983(); // 00592440

	WOverlay* m_pBg; // +0x1c
	WOverlay* m_pBgNext; // +0x20: fundo novo, entrando
	float m_bgFade; // +0x24
	WOverlay* m_pShadow; // +0x28
	WOverlay* m_pMark; // +0x2c
	lua_system::lua_script* m_pScript; // +0x30 (lobby_bg.lua)
	std::list<sLobbyAvatar983> m_loaded; // +0x34: carregados pela outra linha
	std::vector<sLobbyAvatar983> m_avatars; // +0x40
	std::vector<sLobbyAvatarMove983> m_moves; // +0x50
	std::list<sLobbyAvatarInfo983*> m_requests; // +0x60
	std::vector<sLobbyAvatarTitle983*> m_titles; // +0x6c
	int m_7c; // só zerado
	CAnotherHand m_hand; // +0x80
	CExhibition983* m_pSelected; // +0x208
	float m_areaW; // +0x20c
	float m_areaH;
	float m_petSizeRate; // +0x214
	int m_petPosFixY; // +0x218
	int m_maxAvatarNum; // +0x21c
	int m_requestCount; // +0x220
	float m_uccWait; // +0x224
	std::list<std::string> m_uccPending; // +0x228: roupas UCC pedidas ao servidor
	CExhibition983* m_pCaddie; // +0x234 (ninguém cria no 983)
	CExhibition983* m_pMascot; // +0x238
	WVector2D m_mascotPos; // +0x23c
	WVector2D m_caddiePos; // +0x244
	sLobbyMascotWalk983* m_pMascotWalk; // +0x24c
	int m_motionParam; // +0x250: param2 da mensagem 0x199
	bool m_bShow; // +0x254
	sLobbyHideButton983* m_pHideBtn; // +0x258
	CCriticalSection983 m_cs; // +0x25c

	friend struct sLobbyAvatarLayout983;
};
