#pragma once
#include <functional>
#include "gatewaydoc.h"
#include "icontentsdoc.h"

// JP 983: o que mudou nas bases dos "gateway docs" e o fonte do 645 (frente A) ainda não tem. PROVISÓRIO: pedido em out/PEDIDOS.md
// (sessão B13); quando entrar em gatewaydoc.h / icontentsdoc.h / actor.h, trocar os nomes `…Jp` pelos de verdade e apagar este arquivo.
namespace _gateway
{
// CGatewayControlDoc do 983 (vtable 00d023a8, construtor 004b5f90 com um argumento): ganhou dois inteiros depois do vptr.
// O construtor só grava +0x08 (o tipo de conteúdo, 0xad no CControllerDoc); +0x04 fica como veio do `new`.
class CGatewayControlDocJp : public IContentsDataContainer
{
public:
	CGatewayControlDocJp(int type) : m_type(type) { }
	virtual ~CGatewayControlDocJp() { }
	virtual CGatewayHandler* GetGatewayHandler() { return NULL; }
	// 004b5270: falso só com m_04 == 1 fora do layout "TOPPAGE" (chamado pelo CControllerHandler em 004bc5aa; nome meu)
	bool IsUsable983();

protected:
	int m_04; // +0x04 (1 e 0x10 são os valores testados; quem grava não foi levantado)
	int m_type; // +0x08
};
}

// CContentsDoc do 983 (lista de criadores 00e7e1b0): agora no icontentsdoc.h da A (`CContentsDoc::CreateCreatorList983` 00912b70,
// `RegisterCreator983` 00914760); a visão `CContentsDocJp` saiu na B145 (pedido C81.1).

// CActorFactory do 983 (00a48210 cria pelo nome, 00a49000 guarda): agora `ActorRegistry983().Get/Add` (A156.1); a visão
// `CActorFactoryJp` saiu na B150 (B149.1 atendido pela A no actor.cpp da pg-a157).
