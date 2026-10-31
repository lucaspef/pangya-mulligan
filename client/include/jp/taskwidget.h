#pragma once
#include "vc10_layout.h"
// JP 983: CTaskWidget (unidade 0064f1a0–0064f900; vtables 00d1694c / 00d16944 / 00d16938; RTTI 00e78d88; sizeof 0x1f8 pelo
// `new` de 0064f810). "Tarefa pequena" que roda por cima da tarefa corrente: um `FrTaskForm` criado pelo nome na fábrica de
// forms do Fresh, aberto por uma segunda linha de execução (`CAnotherHand`, chave 1). Quem cria é o `CTaskManager` (009bf6e0:
// `ObjectFactory().CreateObject(nome)`, guarda em `CTask+0x1fc`, `SetLoadingMsg983`, `Start983`). A única derivada do 983 é a
// `CMyRoomWidget` (include/jp/myroomwidget.h). Layout pelo construtor 0064f620 e pelo destrutor 0064f710; nomes dos métodos
// e dos campos são meus (sessão B53).

#include <string>
#include <map>
#include "objectfactory.h"
#include "anotherhand.h"
#include "frcmdtarget.h"
#include "lock.hpp"
#include "taskmanager.h"

class FrForm;
class FrWndManager;
class FrTaskForm;
class WReceivedPacket;

// 00c0efc0 / 00c0dff0 (faixa do Fresh, sem nome no mapa): fábrica de forms pelo nome — instância estática local com um
// `std::map` (nó de 0x30). Empréstimo; nomes meus (pedido B → A da B53).
class FrFormFactory983
{
public:
	typedef FrForm* (*Creator)(FrWndManager* pManager, const char* name); // CreateForm<T>(pManager, NULL, name)
	static FrFormFactory983& Instance983(); // 00c0efc0
	void Register983(const char* name, Creator creator); // 00c0ecb0
	FrForm* Create983(const char* name, FrWndManager* pManager); // 00c0dff0

private:
	std::map<std::string, Creator> m_creators;
};

// registro estatico do form no nome da fabrica (983: 00cd2300 "realmyroom", 00cd7ea0 "tutorialmain", 00cea580 "shopmain");
// cada unidade registra o seu CreateForm<T> (o 983 usa o thunk 00a92d70: ecx = nome, edx = gerenciador)
struct FrFormRegistrar983
{
	FrFormRegistrar983(const char* name, FrFormFactory983::Creator creator) { FrFormFactory983::Instance983().Register983(name, creator); }
};

class CMouseCursorJp983
{
public:
	void SetCursor983(int cursor, int flag); // 00891020: o `SetCursor` do 645 com um argumento a mais
	int GetCursor983(); // 00891000
	// embutidas (cópias fora de linha no rfscenemanager.cpp)
	void SetLocked983(bool locked) { m_bLocked983 = locked; } // 00603270
	bool IsLocked983() { return m_bLocked983; } // 00603280

	char m_000[0x138];
	bool m_bLocked983; // +0x138: o RFSceneManager não troca o cursor (nome meu)
};
extern CMouseCursorJp983* g_pMouseCursor983; // 00e73e7c (WSingleton<CMouseCursor>)

class CTaskWidget : public IObject, public CHandOwner, public FrCmdTarget
{
	DECLARE_OBJECT(CTaskWidget)

public:
	CTaskWidget(); // 0064f620
	virtual ~CTaskWidget(); // 0064f710 (com `delete`: 0064f8a0; repasse de +8: 0064f6f0)

	virtual void Open(); // 0064f3b0 [2]: cria o form `m_formName`; erro = mensagem 0x23 com o texto 0x1389 / 0x138a
	virtual void Activate(); // 0064f230 [3]: mostra o form, solta o mouse e a tecla ESCAPE
	virtual void Close(); // 0064f1a0 [4]
	virtual void Process(float delta); // 0064f290 [5]: enquanto carrega, chama o [7] e dorme 30 ms
	virtual int OnPacket(WReceivedPacket& packet); // 0064f1e0 [6]: 0 = não tratou
	virtual void OnProcessWhileLoading(float delta); // 0064f1c0 [7], vazia
	virtual void OnDisplayWhileLoading(); // 0064f1d0 [8], vazia

	FrTaskForm* GetForm() const { return m_pForm; } // cópia fora de linha em 0064f1f0
	void Begin983(); // 0064f310: prende o mouse e a tecla ESCAPE, cursor 7, manda `m_loadingMsg` ao ator principal
	void Start983(); // 0064f600: Begin983 + Open + Activate (chamada pelo CTaskManager 009bf6e0)
	void SetLoadingMsg983(const char* msg); // 0064f8d0

protected:
	virtual void OnAnotherHand(int key, void* pParam); // 0064f5d0: chave 1 = Start983

	unsigned long m_flag; // +0x0c: 1 = form criado, 2 = ativa
	bool m_bLoading; // +0x10: só o construtor e a Activate gravam (falso); a Process testa
	CAnotherHand m_anotherHand; // +0x14
	int m_19c; // +0x19c: zerado pela Begin983
	FrTaskForm* m_pForm; // +0x1a0
	std::string m_formName; // +0x1a4: nome do form na fábrica (a derivada grava antes de chamar a Open)
	std::string m_loadingMsg; // +0x1c0
	_client::CCriticalSection m_cs; // +0x1dc
};

C_ASSERT_VC10(sizeof(CTaskWidget) == 0x1f8);
