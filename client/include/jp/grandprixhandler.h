#pragma once
// JP 983: ator "CGrandPrixHandler" (RTTI .?AVCGrandPrixHandler@_grandprix@_gateway@@; vtables 00d09f14 / 00d09f0c / 00d09f00, 0x30 bytes
// pelo `new` de 0055a910, construtor 0055a7c0, destrutor 0055a860). Unidade 00559cc0–0055aa10 (sessão B65; o corte da F3.7.1 começava
// em 0055a390: as mensagens ficavam no fim do corte do documento). Recebe as mensagens do Grand Prix (param1 = função, tabela montada
// em 0055a3e0: 13 funções, ids 0x00–0x0d sem o 0x0a) e repassa ao CGrandPrixDocument (documento 0xaf) ou abre as janelas
// "grandprixopen" (FrGrandPrixOpenDlg) e "gplimitdlg" (FrGrandPrixLimitDlg). Base secundária FrCmdTarget em +0x20 (só o destrutor;
// o mapa de mensagens é o da base). Nomes dos métodos meus; a coluna "id" é a chave do FunctionMapper.
#include "controllerhandler.h"
#include "frcmdtarget.h"
#include "grandprixdocument.h"

class FrGrandPrixLimitDlg;

namespace _gateway
{
namespace _grandprix
{
class FrGrandPrixOpenDlg;

class CGrandPrixHandler : public CGatewayHandlerJp, public FrCmdTarget
{
public:
	static const WRTTI m_RTTI; // 00e751b8
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 0055a830

	CGrandPrixHandler(); // 0055a7c0
	virtual ~CGrandPrixHandler() { } // 0055a860 (0055a9a0 com delete; ajustes 0055a840 / 0055a850)
	virtual bool Initialize(); // 0055a3e0
	virtual bool Execute(const MsgObject& msg); // 0055a390

	CGrandPrixDocument* Doc983() { return (CGrandPrixDocument*)m_pDocument; } // embutida; 00559ce0 é uma cópia fora de linha sem chamador

	void OnReadList(const MsgObject& msg); // id 0x00, 00559fc0: doc ReadList983(pacote) + RefreshTicket983
	void OnUpdatePair(const MsgObject& msg); // id 0x01, 00559ff0: doc UpdatePair983(pacote) + RefreshTicket983
	void OnRead02(const MsgObject& msg); // id 0x02, 0055a020: doc 005564a0(pacote)
	void On03(); // id 0x03, 00559d60: doc 00555140
	void OnGetItemCount(const MsgObject& msg); // id 0x04, 0055a120: *(int*)param3 = quantidade do item param2 (sem o item, não grava)
	void OnRead05(const MsgObject& msg); // id 0x05, 0055a250: doc 005582b0(pacote)
	void OnMsg06(const MsgObject& msg); // id 0x06, 00559cd0: vazia
	void OnOpen(const MsgObject& msg); // id 0x07, 0055a040: abre "grandprixopen" com o sGPEnter983 de param2
	void OnLimit(const MsgObject& msg); // id 0x08, 0055a0b0: abre "gplimitdlg" com a linha (sGPRoomItem983) de param2
	void OnSetTypeId(const MsgObject& msg); // id 0x09, 0055a160: doc SetTypeId983(param2); mensagem 0x23 ao ator principal; On03
	void OnClearTimer(const MsgObject& msg); // 0055a1f0: doc ClearTimer983(param2) (sem id na tabela nem chamador)
	void On0b(const MsgObject& msg); // id 0x0b, 0055a210: doc 005566b0(param2)
	void On0c(const MsgObject& msg); // id 0x0c, 0055a230: doc 005566e0(param2)
	void On0d(); // id 0x0d, 00559d70: doc 00552750

private:
	FrGrandPrixLimitDlg* m_pLimitDlg; // +0x24
	FrGrandPrixOpenDlg* m_pOpenDlg; // +0x28 (não iniciado pelo construtor)
	bool m_2c; // +0x2c (uso não visto)
};
}
}
