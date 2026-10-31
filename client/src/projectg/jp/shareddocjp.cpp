// JP 983: corpos dos métodos pequenos do CSharedDoc que a B usa pela visão CSharedDocJp (include/jp/shareddocjp.h). No 983
// ficam na unidade do CSharedDoc (0062be40–0062bf90, frente A); aqui só para o link não acusar (pedidos C28/C29). Quando os
// campos tiverem nome no shareddoc.h, estes corpos vão para o shareddoc.cpp. Deslocamentos medidos no 983. Sessão B77.
#include "minatl.h"
#include "projectg.h"
#include "shareddocjp.h"
#include "vc10_layout.h"

// A732: crus 0x1xxxx -> membros do CSharedDoc (o cl novo desloca o bloco; VC10 = os do 983)
#define SDOC983 ((CSharedDoc*)this)
C_ASSERT_VC10(offsetof(CSharedDoc, m_playerCutinItem983) == 0x11448 && offsetof(CSharedDoc, m_auxPartValues983) == 0x114b8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_lobbyState983) == 0x11638);

// 0062be40
void CSharedDocJp::ClearSetEffect983()
{
	SDOC983->m_playerCutinItem983[0] = 0;
	SDOC983->m_playerCutinItem983[1] = 0;
	SDOC983->m_playerCutinItem983[2] = 0;
	SDOC983->m_playerCutinItem983[3] = 0;
}

// 0062be60
void CSharedDocJp::SetSetEffect983(unsigned char player, unsigned long typeId)
{
	SDOC983->m_playerCutinItem983[player] = typeId;
}

// 0062bea0
void CSharedDocJp::SetAuxpartValues983(void* pValues, int player)
{
	SDOC983->m_auxPartValues983[player] = pValues;
}

// 0062bec0
void CSharedDocJp::ClearAuxpartValues983()
{
	SDOC983->m_auxPartValues983[0] = 0;
	SDOC983->m_auxPartValues983[1] = 0;
	SDOC983->m_auxPartValues983[2] = 0;
	SDOC983->m_auxPartValues983[3] = 0;
}

// 0062bf90
int CSharedDocJp::GetState983()
{
	return SDOC983->m_lobbyState983;
}
