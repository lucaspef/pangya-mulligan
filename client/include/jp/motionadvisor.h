#pragma once
#include <map>
#include <string>
#include <vector>
#include "engine/singleton.h"

// JP 983: troca do nome da animação do personagem por efeito de conjunto (classe nova; RTTI CMotionAdvisor, vtable 00d0ce88,
// base WSingleton<CMotionAdvisor> 00d0ce80, instância 00e768f8, 0x48 bytes — `push 0x48` do criador 00a65895). Para cada
// jogador (chave = FindKeyByPlayer983) guarda, por nome de animação, as partes que pedem a troca e o nome que a substitui.
// Quem escreve: CSetAbilityMotionChange::Apply (AddPart + SetMotionName "기본자세" → "연패기본자세"); quem lê: 00876b90 e
// 007a77d0 (GetMotionName); quem limpa: CSetEffectController::On02/On03 (Reset). Nomes de método e de campo meus.
class CMotionAdvisor : public WSingleton<CMotionAdvisor>
{
public:
	struct sMotion // 0x48 (valor do mapa interno; nó de 0x74)
	{
		std::vector<int> parts; // +0x00: partes do conjunto que pedem a troca
		std::string names[2]; // +0x10: [0] nome original, [1] nome que o substitui (vazio = sem troca)
	};
	typedef std::map<std::string, sMotion> NameMap;
	typedef std::map<unsigned int, NameMap> MotionMap;

	CMotionAdvisor(); // 005a6e90
	virtual ~CMotionAdvisor(); // 005a6d80 (slot 0 = 005a6fa0)

	bool Reset(); // 005a6530
	const char* GetMotionName(unsigned int key, int part, const char* name); // 005a6540 (name se não há troca; NULL sem o nome)
	void AddPart(unsigned int key, unsigned long part); // 005a6670 (part entra como int no vector: o 983 faz a cópia)
	void SetMotionName(unsigned int key, const char* name, const char* newName); // 005a6ba0

protected:
	void Clear(); // 005a6370

	MotionMap m_motions; // +0x28
	std::vector<std::string> m_names; // +0x38 (nomes de animação conhecidos; o construtor põe "기본자세")
};
