#pragma once
// JP 983: salas do lobby. `CLobbyRoom` (vtable 00d0c55c, 9 entradas) é a interface da lista de salas; as duas derivadas são
// a `CNormalLobbyRoom` (vtable 00d0c5cc) e a `CGrandPrixLobbyRoom` (vtable 00d0c584), em include/jp/normallobbyroom.h (unidade
// normallobbyroom, 00597055–0059e260). Os nomes dos métodos virtuais são meus, pelo que a FrMultiplayRoomDlg repassa a cada
// um; o número de argumentos vem dos `ret N` da base (00598150–005981c0).
//
// `CLobbyRoomSelector983` (nome meu; sem RTTI) é o objeto guardado em FrRoomListDlg+0x140: o tipo de sala atual e o vetor
// com uma `CLobbyRoom*` por tipo (0 = salas comuns, 1 = Grand Prix, pela ordem de `CSharedDoc+0x11630`). Métodos em
// 005a01a0–005a0310. O construtor 005a0780 (sessão B50) recebe o documento 0xa5 e cria as duas salas; quem chama o construtor
// não foi levantado.

#include <vector>
#include "shareddocjp.h"

class FrListBox;
class FrRoomListDlg;
struct sRoomListDoc983;
namespace lua_system { class lua_script; }

void LoadLobbyRoomScript983(const char* name); // 005980d0
lua_system::lua_script* GetLobbyRoomScript983(); // 00598130
void SetRoomListDoc983(sRoomListDoc983* pDoc); // 00598120 (normallobbyroom.cpp)
void LoadRoomListLayout983(); // 0059dbe0: tabelas de roomlist_layout_ui.lua (normallobbyroom.cpp)
void LoadRoomListLayoutThunk983(); // 0059e250: `jmp` para a de cima

class CLobbyRoom
{
public:
	CLobbyRoom(); // 00598140
	virtual ~CLobbyRoom(); // 00598870 (corpo 005981d0)
	virtual void Init(FrRoomListDlg* pDlg); // 00598150
	virtual void AddRoom983(FrListBox* pList, int room); // 00598160: `room` = sRoomInfo* que chegou do servidor
	virtual void DelRoom983(FrListBox* pList, int room); // 00598170: `room` = sRoomInfo* que saiu
	virtual void OnRoomListOwnerDraw(int param, FrRoomListDlg* pDlg); // 00598180
	virtual void OnRoomListLDown(FrRoomListDlg* pDlg); // 00598190
	virtual void OnRoomListRUp(FrRoomListDlg* pDlg); // 005981a0
	virtual void OnRoomListDClick(FrRoomListDlg* pDlg); // 005981b0
	virtual void LoadScript983(); // 005981c0 (só a do Grand Prix faz algo: 00599cd0)
};

class CLobbyRoomSelector983
{
public:
	CLobbyRoomSelector983(sRoomListDoc983* pDoc); // 005a0780
	// 005a01a0 (embutida nos repasses): o tipo 2 quer dizer "ainda não lido" — vale o de CSharedDoc+0x11630 (0062bf50)
	int GetType983()
	{
		if (m_type == 2)
			m_type = ((CSharedDocJp*)Doc())->GetRoomListKind983();
		return m_type;
	}
	void Init983(FrRoomListDlg* pDlg); // 005a01c0
	void Fn5a01f0_983(FrListBox* pList, int a); // 005a01f0
	void Fn5a0220_983(FrListBox* pList, int a); // 005a0220
	void OnRoomListOwnerDraw983(int param, FrRoomListDlg* pDlg); // 005a0250
	void OnRoomListLDown983(FrRoomListDlg* pDlg); // 005a0280
	void OnRoomListRUp983(FrRoomListDlg* pDlg); // 005a02b0
	void OnRoomListDClick983(FrRoomListDlg* pDlg); // 005a02e0
	void Fn5a0310_983(int a, FrRoomListDlg* pDlg); // 005a0310
	void LoadScript983(); // 005a06b0: LoadScript983 de cada sala (CLobbyMain::OnLoad 00ab5a70)
	void ReloadScript983(const char* name); // 005a06e0: relê o script (005980d0) e chama o de cima (msg 0x340 do 00abc140)

	int m_type; // +0: 0 ou 1; 2 = ainda não lido do documento
	std::vector<CLobbyRoom*> m_rooms; // +4 (0x10 no VS2010)
	std::vector<void*> m_14; // +0x14: segundo vetor, zerado pelo construtor; 005a0900 acrescenta (elemento não levantado)
};
