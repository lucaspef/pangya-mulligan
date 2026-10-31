#pragma once
#include "vc10_layout.h" // VC10_OFS_BO
#include "jp/docofs983.h" // A757: CSharedDocLobbyJp +0x8cb/+0xa26/+0xa2e/+0xa8e ancorados no m_myInfo (cl 14.44: 0x7a0 -> 0x570)
// JP 983: empréstimos comuns dos atores Lobby* (LobbyCaddie, LobbyItems, LobbyMascot, LobbyPlayer). Tudo aqui é de território
// da frente A ou ainda sem dono; nomes meus, endereços do exe 983. PROVISÓRIO: pedido em out/PEDIDOS.md (sessão B28).

// functor "objeto + método" que os atores guardam num std::tr1::function (RTTI `_util::WMemFun<LobbyCaddie,bool>`): 0x10, com
// o ponteiro de método em +8 (o +4 não é gravado: alinhamento natural de 8, sem alinhar a pilha)
namespace _util
{
	template <class T, class R>
	struct WMemFun983
	{
		typedef R (T::*Func)();
		T* m_pObj;
		union
		{
			Func m_pFun; // +0x08
			__int64 m_align;
		};
		R operator()() const { return (m_pObj->*m_pFun)(); }
	};
	// 00560160: recebe o destino em ecx e o objeto em edx (não é construtor)
	template <class T, class R>
	WMemFun983<T, R>* __fastcall InitMemFun983(WMemFun983<T, R>* pOut, T* pObj, R (T::*pFun)())
	{
		pOut->m_pObj = pObj;
		pOut->m_pFun = pFun;
		return pOut;
	}
}

// botões do mouse (objeto em +0x34 do singleton 00e73e4c): 1 apertou, 2 soltou, 4 clique, 0x100 segurando
class CMouseButtons983
{
public:
	unsigned int Get983(); // 005f3b90
};

// RFWnd / RFManager (frente A, sem cabeçalho no 983): só o que o ponteiro do mouse usa
class CRFWndJp
{
public:
	WSize GetSize983(); // 00406750 (RFWnd::GetSize)
};
class CRFManagerJp
{
public:
	bool IsModalExist983(); // 00418990 (RFManager::IsModalExist)
	CRFWndJp* GetRoot983(); // D856: RFManager::_root pela lib (rfmanager.cpp; +0x4c só no VC10)
};

// singleton 00e73e4c: ponteiro do mouse da interface
class CUiPointer983
{
public:
	static CUiPointer983* Instance983() { return ms_pInstance; }
	CMouseButtons983* GetButtons983() { return m_pButtons; }
	WVector* GetPos983(); // 006038b0
	bool IsOverWnd983(float x, float y); // 00604320
	void SetCursor983(int cursor, int flag); // 00603710

	// 00565570: tamanho da tela da interface (800 × 600 sem o gerente)
	WSize GetScreenSize983()
	{
		if (m_pRF)
			return m_pRF->GetRoot983()->GetSize983();
		return WSize(800.0f, 600.0f);
	}

	static CUiPointer983* ms_pInstance; // 00e73e4c
	// D870: RFSceneManager vem de BaseObject (std::string 0x1c no VC10, 0x18 no cl novo): tudo 4 antes no VS 2022
	char m_00[VC10_OFS_BO(0x28)];
	CRFManagerJp* m_pRF; // +0x28 = RFSceneManager::m_pManager
	char m_2c[8];
	CMouseButtons983* m_pButtons; // +0x34
	char m_38[8];
	bool m_show40; // +0x40: ligado/desligado junto do HIDE da sala de avatar (ProcessUI 007b3347); nome meu
};

// singleton 00f02340 = g_pFresh (Fresh*). D865: o +0x40 do VC10 e Fresh::manager (FrElementDoc antes tem STL: anda no VS 2022)
class Fresh;
struct sF02340_983;
extern sF02340_983* g_pF02340_983;
#define FRESHMGR983() (((Fresh*)g_pF02340_983)->manager)

extern int g_lobbyModal983; // 00e78cac: 0 = lobby (CLobbyMain), 1 = conversa de avatar (definido no rflobbydesktop.cpp)

// singleton 00e78ca8 = o RFLobbyDesktop (include/jp/refresh/rflobbydesktop.h; visão): estado do lobby (+0x1fc: 2…4 e 5…7 são dois grupos de telas; 3 e 6 = a variante em que os bonecos atendem o mouse)
// D870: o RFLobbyDesktop (RFWndEx/RFFormDesktopBase com STL) anda no cl novo: m_state 0x1ec, m_bCursorBusy 0x22c, m_bChanged 0x240
// (medido out/D870/probe.py); C_ASSERT contra o tipo real em lobbymain.cpp
#if defined(_MSC_VER) && _MSC_VER > 1600
#define REBANG_LOBBYSTATE_000 0x1ec
#define REBANG_LOBBYSTATE_200 0x3c
#else
#define REBANG_LOBBYSTATE_000 0x1fc
#define REBANG_LOBBYSTATE_200 0x44
#endif
class CLobbyState983
{
public:
	static CLobbyState983* Instance983() { return ms_pInstance; }
	bool IsGroupB983() const; // 0055fe60: 5, 6 ou 7
	bool IsGroupA983() const; // 0055fe80: 2, 3 ou 4
	bool IsPickable983() const; // 0055fea0: 3 ou 6
	int IsState10_983() const { return m_state == 10; } // embutida (cópia fora de linha em 004ff190)
	int IsState11_983() const { return m_state == 11; } // idem (004ff1a0)

	static CLobbyState983* ms_pInstance; // 00e78ca8
	char m_000[REBANG_LOBBYSTATE_000];
	int m_state; // +0x1fc
	char m_200[REBANG_LOBBYSTATE_200 - 4];
	bool m_bLocked; // +0x240: os botões não respondem (B1454; RFLobbyDesktop::m_bLocked)
	char m_241[3];
	bool m_bCursorBusy; // +0x244
	bool m_bOverPet; // +0x245
	char m_246[0x12];
	bool m_bChanged; // +0x258: o boneco do jogador mudou de peça (LobbyPlayer)
	int IsMain983() const; // 00565af0 (unidade lobbyplayer): 2 ou 5
	void Refresh983(); // 005fbe40: refaz o lobby depois de compra/presente/my room (16 chamadores; 005f8380, 005fa980,
	                   // 005fa900, 005fae60(1)); nome meu, corpo não escrito (unidade do lobby, 005f…)
};

// 00565a90 (unidade lobbyplayer): o ponteiro do mouse está em cima do boneco que fica em `pos`?
bool __fastcall IsMouseOverPet983(const WVector& mouse, const WVector& pos);
// 004db4b0: interpola de `from` a `to` em `duration` segundos
float __cdecl Fade983(float from, float to, float time, float duration);

extern bool g_bFocus; // 983 00e33794 (foco da janela: WM_SETFOCUS/WM_KILLFOCUS)
extern const char* g_lobbyAppearFx983; // 00e0c02c

// CSceneManager (frente A): 009b2900 e o ponteiro do singleton 00e74230
class CSceneManagerJp
{
public:
	static CSceneManagerJp* Instance983() { return ms_pInstance; }
	void AddElement983(int type, int a, int b, const char* name, int layer, int c); // 009b2900
	void SetPuppet(const char* name, WPuppet* puppet); // 009b0eb0
	void DeleteElement(const char* name); // 009b4130
	static CSceneManagerJp* ms_pInstance; // 00e74230
};

// CSharedDoc (frente A): caddie do jogador
struct sCaddie983
{
	int m_00;
	unsigned long typeId; // +0x04
	unsigned long skinId; // +0x08
};
class CSharedDocLobbyJp
{
public:
	unsigned long FindCharIdByTid983(unsigned long tid); // 00643ed0 (-1 = não tem)
	unsigned long GetClubSetId983() const { return *(unsigned long*)((char*)this + DOCOFS983(0xa2e)); }
	unsigned long GetMascotId983() const { return *(unsigned long*)((char*)this + DOCOFS983(0xa8e)); } // mascote em uso
	unsigned long GetUid983() const { return *(unsigned long*)((char*)this + DOCOFS983(0x8cb)); }
	unsigned long GetCaddieId983() const { return *(unsigned long*)((char*)this + DOCOFS983(0xa26)); }
	unsigned long* GetCaddieIdPtr983() { return (unsigned long*)((char*)this + DOCOFS983(0xa26)); }
};
#define DOCLOBBYJP() ((CSharedDocLobbyJp*)Doc())

// CItemManager do 983 (CSharedDoc+0x20c)
struct sItem983
{
	int m_00;
	unsigned long typeId; // +0x04
};
class CItemManagerLobbyJp
{
public:
	char* Find1e0_983(unsigned long typeId); // 00a171e0 (registro com o nome em +0x49 e o taco em +0xc0)
	sItem983* Find2a0_983(unsigned long typeId); // 00a172a0
	sItem983* FindCadItem983(unsigned long typeId); // 00a172f0 (par 839: CItemManager::FindCadItem)
};
#define ITEMMGRLOBBYJP() ((CItemManagerLobbyJp*)&Doc()->m_itemManager) // 983 +0x20c (C_ASSERT_VC10 lobbyplayer.cpp); cl 14.44 0x1e4
