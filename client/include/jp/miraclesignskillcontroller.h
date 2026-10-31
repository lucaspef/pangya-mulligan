#pragma once
#include "controller.h"
#include "miraclesignskillobject.h"

// JP 983: unidade vizinha (jp/automiraclesigncontroller.cpp, ainda não escrita): o jogador `index` é o meu? Nome provisório.
bool IsControlledPlayerMine(unsigned char index); // 0047dd60

// JP 983: unidade jp/gamestringmanager.cpp (F3.7.3, ainda não escrita): texto da tabela de strings do jogo. Nome provisório.
const char* GetGameString(unsigned int id); // 00540a10

// JP 983: 0056ad10 (cópia fora de linha nesta unidade; o controlador usa embutida). Nome provisório: tipos de jogo 0x0b..0x0e.
inline bool IsMiracleSignOffGame(int gameType)
{
	if (gameType < 0xb || gameType > 0xe)
		return false;
	return true;
}

// JP 983: RTTI .?AVCMiracleSignSkillController@@, vtable 00d0a818, 0x10 bytes (sem campo próprio).
class CMiracleSignSkillController : public IController
{
	DECLARE_OBJECT(CMiracleSignSkillController)
public:
	CMiracleSignSkillController(); // 0056acc0 (fora de linha: o MakeInstance chama)
	CMiracleSignSkillObject* GetControlled(); // 0056ad00
	bool Apply(unsigned char index); // 0056aea0
	virtual bool On07(int arg); // 0056afe0
};
