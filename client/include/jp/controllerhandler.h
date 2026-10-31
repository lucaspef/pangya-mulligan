#pragma once
#include "controllerdoc.h"
#include "objectfactory.h"
#include "actor.h"
#include "wtemplate.h"

// JP 983: `_gateway::CGatewayHandler` (vtables 00d026fc / 00d026f4, construtor 004bd460, 0x20 bytes) deriva de IObject e de
// FunctionMapper, nesta ordem, e tem sete virtuais depois de Initialize/Execute; o do fonte da frente A (gatewayinterface.h) é
// `FunctionMapper, FrCmdTarget`. PROVISÓRIO: classe de empréstimo até o pedido em out/PEDIDOS.md (sessão B14) ser atendido.
// Nomes dos sete virtuais meus (o que cada um faz só foi visto no CControllerHandler).
// JP 983: a MsgObject tem leitores de parâmetro por tipo, fora de linha (004bb1d0…004bb300 chamam 004bad80…004baee0 com um
// seletor vazio de 1 byte na pilha); a do fonte da frente A só tem os campos. PROVISÓRIO: classe de empréstimo sem campo (pedido
// em out/PEDIDOS.md, sessão B14). Nomes meus.
template<class T> struct MsgArg983 { };
class MsgObjectJp : public MsgObject
{
public:
	template<class T> T Param2() const;
	template<class T> T Param2(MsgArg983<T>) const;
	template<class T> T Param3() const;
	template<class T> T Param3(MsgArg983<T>) const;
	template<class T> T Param4() const;
	template<class T> T Param4(MsgArg983<T>) const;
};
template<class T> T MsgObjectJp::Param2() const { return Param2(MsgArg983<T>()); }
template<class T> T MsgObjectJp::Param2(MsgArg983<T>) const { return (T)param2; }
template<class T> T MsgObjectJp::Param3() const { return Param3(MsgArg983<T>()); }
template<class T> T MsgObjectJp::Param3(MsgArg983<T>) const { return (T)param3; }
template<class T> T MsgObjectJp::Param4() const { return Param4(MsgArg983<T>()); }
template<class T> T MsgObjectJp::Param4(MsgArg983<T>) const { return (T)param4; }
#define MSGJP(msg) ((const MsgObjectJp&)(msg))

namespace _gateway
{
class CGatewayHandlerJp : public IObject, public FunctionMapper
{
public:
	CGatewayHandlerJp() : m_pDocument(NULL) { }
	virtual ~CGatewayHandlerJp() { }
	virtual bool Initialize() = 0; // slot 2
	virtual bool Execute(const MsgObject& msg) = 0; // slot 3
	virtual void On04() { } // 004bd520
	virtual void On05() { } // 004bd530
	virtual void On06() { } // 004bd540
	virtual void OnProcess(float) { } // slot 7 (004bd550)
	virtual void OnDisplay() { } // slot 8 (004bd560)
	// slot 9 = 004bb3d0 (E135): sem nenhuma funcao no FunctionMapper chama Initialize; senao true. Era `return true`, e o Gateway
	// (EnsureInitialized983) nunca montava a tabela dos manipuladores sem OnEnter proprio (Achieve/Quest do Pop-Up nao abriam).
	virtual bool OnEnter() { if (m_functionMap.size() == 0) return Initialize(); return true; }
	virtual void OnLeave() { } // slot 10

protected:
	IContentsDataContainer* m_pDocument; // +0x1c
};

namespace _controller
{
// JP 983: ator "CControllerHandler" (RTTI .?AVCControllerHandler@_controller@_gateway@@, vtables 00d02734 / 00d0272c, 0x20 bytes pelo
// `new` de 004bd7b0, construtor 004bd6e0). Recebe as mensagens do jogo (param1 = função, tabela montada em 004bcae0) e repassa
// aos objetos controlados do CControllerDoc. Nomes dos métodos meus; a coluna "id" é a chave do FunctionMapper.
class CControllerHandler : public CGatewayHandlerJp
{
public:
	static const WRTTI m_RTTI; // 00e74cc4
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 004bd700

	CControllerHandler() { } // 004bd6e0
	virtual ~CControllerHandler() { } // 004bd720 (004bd840 com delete)
	virtual bool Initialize(); // 004bcae0
	virtual bool Execute(const MsgObject& msg); // 004bc930
	virtual void OnProcess(float delta); // 004bc570: Fire06(delta) em todos os objetos (só com o doc em modo 0x10)
	virtual void OnDisplay(); // 004bc660: Fire12 em todos
	virtual bool OnEnter(); // 004bb3e0: RefreshEnable do doc; monta a tabela se ainda está vazia
	virtual void OnLeave(); // 004bb130: FireOn03 + Clear do doc

	CControllerDoc* Doc983() const { return (CControllerDoc*)m_pDocument; }
	void FireOn07(int player, unsigned long ability, int arg); // 004bb150: slot 7 do objeto (habilidade 1..0x23)
	void FireOn08(int player, unsigned long ability, int arg); // 004bb180: marca m_28 e chama o slot 8
	bool InitializeIfEmpty(); // 004bb3d0 (ninguém chama)

	void OnSetObjects(const MsgObject& msg); // id 0x00, 004bb400: param2 = jogador (< 4)
	void OnSetSubObjects(const MsgObject& msg); // id 0x01, 004bc5a0: param2 = jogador (byte), param3 = sCharacterInfo*
	void OnFire04(); // id 0x02, 004bc680
	void OnFire05(); // id 0x03, 004bc6b0
	void OnTurn(const MsgObject& msg); // id 0x04, 004bc330: param2 = jogador; On06(jogador) em todos e Fire07 nos do jogador
	void OnFire08(); // id 0x05, 004bc6d0
	void OnFire10(const MsgObject& msg); // id 0x06, 004bc6f0: param2 = argumento
	void OnSnapshot(const MsgObject& msg); // id 0x07, 004bc9a0: param2 = jogador (byte); Snapshot dos objetos dele
	void OnFire09(); // id 0x08, 004bc720
	void OnAddAbility(const MsgObject& msg); // id 0x09, 004bb430: param2 = sAbilityMsg*
	void OnSave(const MsgObject& msg); // id 0x0a, 004bbc70: param2 = FILE*; quantidade + Save de cada objeto do jogador local
	void OnLoad(const MsgObject& msg); // id 0x0b, 004bb5f0: param2 = FILE*; quantidade + (habilidade, Load) de cada objeto
	void OnObject10(const MsgObject& msg); // id 0x0c, 004bbce0: slot 10 dos objetos do jogador 0 com param2
	void OnLoaded(); // id 0x0d, 004bb6b0: SetActive983(true) + FireOn09 e Purge do jogador local
	void OnFireAll(); // id 0x0f, 004bc400: Fire04, Fire05, Fire06(0) e Fire07(jogador) em todos
	void OnGetObject(const MsgObject& msg); // id 0x10, 004bb470: *(CControlledObject**)param4 = objeto (param2, param3)
	void OnObject08(const MsgObject& msg); // id 0x11, 004bb4b0: slot 8 do objeto (param2, param3) com param4
	void OnObject07(const MsgObject& msg); // id 0x12, 004bb510: slot 7 do objeto (param2, param3) com param4
	void OnPacket13(const MsgObject& msg); // id 0x13, 004bb560: sAbilityPacket → slot 7 (eu) ou 8 (outro) do objeto
	void OnPacket14(const MsgObject& msg); // id 0x14, 004bb700: uid, valor, byte → habilidade 0x0b com sAuxpartPacket*
	void OnPacket15(const MsgObject& msg); // id 0x15, 004bb7c0: (4 bytes), uid, byte, float → habilidade 0x0c com sWindPacket*
	void OnPacket16(const MsgObject& msg); // id 0x16, 004bb860: valor, uid → slot 8 da habilidade 0x10, se o jogador tem
	void OnFire13(); // id 0x17, 004bc740
	void OnPacket18(const MsgObject& msg); // id 0x18, 004bb8c0: (4 bytes), uid → aviso 0x214 ao "GuideLine" se tem a 0x13
	void OnPacket19(const MsgObject& msg); // id 0x19, 004bb980: o mesmo para a habilidade 0x20
};

// corpo do pacote da mensagem 0x13 (0x14 bytes lidos de uma vez; 004bb560 monta os valores iniciais 0, 0, 0, 1, -1)
struct sAbilityPacket
{
	sAbilityPacket() : m_ability(0), m_04(0), m_arg(0), m_0c(1), m_uid(0xffffffff) { }
	unsigned long m_ability; // +0x00
	int m_04;
	int m_arg; // +0x08
	int m_0c;
	unsigned long m_uid; // +0x10
};

// argumento do slot 7/8 da habilidade 0x0b na mensagem 0x14 (004bb700)
struct sAuxpartPacket
{
	int m_player; // +0x00
	unsigned long m_value; // +0x04
	unsigned long m_uid; // +0x08
	int m_flag; // +0x0c
};

// argumento do slot 7/8 da habilidade 0x0c na mensagem 0x15 (004bb7c0)
struct sWindPacket
{
	float m_value; // +0x00
	unsigned char m_flag; // +0x04
};

// argumento da mensagem 0x09 (004bb430 lê os cinco campos e repassa a CControllerDoc::AddAbility)
struct sAbilityMsg
{
	unsigned char m_player; // +0x00
	unsigned long m_ability; // +0x04
	int m_08;
	unsigned long m_typeId; // +0x0c
	float m_value; // +0x10
};
}
}
