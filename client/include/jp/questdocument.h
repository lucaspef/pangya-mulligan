#pragma once
#include "vc10_layout.h"
// JP 983: _gateway::_questoneday::CQuestDocument (documento 0xac do CContentsDoc; vtable 00d12c98 pelo construtor 005ede60, sizeof 0xb0
// pelo `new` de 005ee040). Mesma unidade do CQuestHandler (os inicializadores estáticos 00cd2100 do documento e 00cd2110…00cd2160 do
// handler estão em sequência): a unidade vai de ~005e71a0 a 005f0200, compilada com LTCG, quase toda instâncias da STL.
// Escrito em jp/questhandler.cpp (handler na B99, documento na B100). Layout pelo construtor e pelo destrutor (005edaf0). Nomes meus.
#include "gatewaydocjp.h"
#include "achievement.h"
#include <vector>
#include <map>

namespace _quest
{
class CQuestItem;
struct sQuestCount983;
}
namespace IFF983
{
struct sQuestItem;
struct sQuestStuff;
struct sAchievement;
}

// campos do Achievement.iff (0x600) que o documento lê (o IFF983_REC da A só tem bFinal/TypeId). O construtor 004e2820 (unidade da
// FrAchievement, achievementdlg.cpp) zera +0xc0, os 10 nomes e os 10 typeids, e põe −1 na categoria. Nomes meus (movida do questhandler.cpp na B102).
struct sAchievementView983
{
	sAchievementView983(); // 004e2820

	unsigned char bFinal; // +0x000
	unsigned long TypeId; // +0x004
	char name983[0x40]; // +0x008: nome da conquista (cabeçalho comum do IFF; texto JP nos 201 registros do pak)
	unsigned char level983; // +0x048: nível do cabeçalho comum; 0 nos 201 registros (B1558)
	char bitmap[0x77]; // +0x049: figura da janela "ach_main" (Fresh::GetBitmap)
	unsigned long lastStuffId983; // +0x0c0: última peça (stuffIds[n-1], QuestStuff.iff) = conquista em etapas (194/201 no Achievement.iff); 0 = sem ordem
	int category; // +0x0c4: 0..7 (aba da janela "ach_main")
	char names[10][0x81]; // +0x0c8
	unsigned long stuffIds[10]; // +0x5d4: peças (QuestStuff.iff)
	int unused5fc983; // +0x5fc: 0 nos 201 registros; o 983 só zera no ctor 004e283b
};
C_ASSERT(sizeof(sAchievementView983) == 0x600);
C_ASSERT(offsetof(sAchievementView983, level983) == 0x48 && offsetof(sAchievementView983, bitmap) == 0x49);
inline const sAchievementView983* AchievementView983(const IFF983::sAchievement* p) { return (const sAchievementView983*)p; }

namespace _gateway
{
namespace _questoneday
{
class CQuestHandler;

// linha das listas da janela (0xc): missão do QuestItem.iff e a missão do jogador (NULL = já resolvida)
struct sQuestSlot983
{
	IFF983::sQuestItem* pIff; // +0x0: nome em +8, peça (QuestStuff.iff) em +0xcc, prêmio {typeid +0xf4, quantidade +0xfc}
	_quest::sQuestCount983* pCount; // +0x4: contagem do servidor (CSharedDoc+0x3840) da missão; NULL = sem medidor
	_quest::CQuestItem* pItem; // +0x8
};

// uma conquista do Achievement.iff com as peças dela (valor 0x1c do mapa m_achievementInfos, nó 0x30)
struct sAchievementInfo983
{
	unsigned long typeId; // +0x00: Achievement.iff +4
	int points; // +0x04: pontos que as peças dão (prêmio 0x6c000001 das 3 posições do QuestStuff.iff)
	IFF983::sAchievement* pIff; // +0x08
	std::vector<IFF983::sQuestStuff*> stuffs; // +0x0c: peças (Achievement.iff +0x5d4, 10 typeids; 0 = vazio)
};

class CQuestDocument : public CGatewayControlDocJp
{
public:
	typedef std::map<unsigned long, _achievement::CAchievement> AchievementMap; // nó 0xd0 (chave + enchimento + 0xb0)
	typedef std::map<unsigned long, sAchievementInfo983> AchievementInfoMap; // nó 0x30
	enum { MAX_CATEGORY = 8 }; // Achievement.iff +0xc4

	CQuestDocument(int type); // 005ede60
	virtual ~CQuestDocument(); // 005edaf0 (apagador 005ee010)
	virtual bool Initialize(); // 005edc40
	virtual bool Release(); // 005e7250
	virtual bool Reset(); // 005e7260: zera as duas datas
	virtual CGatewayHandler* GetGatewayHandler(); // 005edc30

	static CGatewayControlDocJp* __fastcall Create(); // 005ee040: `new CQuestDocument(0xac)`

	bool HasCurrent983() const; // 005e7fd0: !m_current.empty()
	sQuestSlot983* FindCurrentByUid983(unsigned long uid); // 005e9620: linha de m_current cuja missão tem esse uid (NULL)
	void SetDate983(const SYSTEMTIME& st, int index); // 005e7290: 0 = m_todayDate, 1 = m_procDate (índice >= 2 ignorado)
	void SetFuture983(const std::vector<unsigned long>& uids); // 005ec050: m_future refeita pelas missões do jogador (estado START) com esses typeids
	void SetCurrent983(const std::vector<_quest::CQuestItem*>& items); // 005ecd80: m_current refeita pelas missões do jogador
	void AddAchievement983(const _achievement::CAchievement& a); // 005eb9f0: entra em m_newAchievements se o typeid ainda não está lá
	void FlipAchievements983(); // 005ecde0: m_achievements = m_newAchievements; m_newAchievements esvaziado
	int GetAchievementPoint983(); // 005ed3d0: soma dos pontos de m_achievements; a FrBongdariShopDlg recebe (+0x584)
	int GetCategoryPoint983(int category); // 005ed500: soma dos pontos das conquistas de uma categoria (FrAchievement)
	int GetPoint983(const _achievement::CAchievement* p); // 005ed6c0: QuestStuff.iff +0xf4 das peças 0..último contador
	std::vector<IFF983::sQuestStuff*>* FindStuffs983(unsigned long typeId); // 005ea470: peças da conquista (NULL)

protected:
	void SortAchievementIffs983(); // 005ec0d0: stable_sort de m_achievementIffs pela categoria
	void BuildAchievementInfos983(); // 005ed780: m_achievementInfos e m_categoryPoints, categoria por categoria
	void MakeStuffs983(std::vector<IFF983::sQuestStuff*>& stuffs, IFF983::sAchievement* pIff); // 005eac20: acrescenta as peças
	int CountStuffPoints983(const std::vector<IFF983::sQuestStuff*>& stuffs); // 005e9670 (accumulate 005e7b60)

public:

	CQuestHandler* m_pHandler; // +0x0c: ator "CQuestHandler" (o Initialize 005edc40 cria pela fábrica)
	std::vector<sQuestSlot983> m_future; // +0x10: "futurework" (as de amanhã)
	std::vector<sQuestSlot983> m_current; // +0x20: "currentwork" (as de hoje)
	SYSTEMTIME m_todayDate; // +0x30: data da lista de hoje ("datetoday")
	SYSTEMTIME m_procDate; // +0x40: data do andamento ("dateproc")
	std::vector<IFF983::sAchievement*> m_achievementIffs; // +0x50: o Achievement.iff inteiro (00a20180), ordenado pela categoria
	AchievementInfoMap m_achievementInfos; // +0x60: por typeid (destruído por 005ed020)
	AchievementMap m_achievements; // +0x70: as que a janela "ach_main" mostra
	AchievementMap m_newAchievements; // +0x80: recebidas na mensagem 8 do handler
	int m_categoryPoints[MAX_CATEGORY]; // +0x90: pontos possíveis por categoria (zerados no Initialize)
};
}
}
C_ASSERT(sizeof(_gateway::_questoneday::sQuestSlot983) == 0xc);
C_ASSERT_VC10(sizeof(_gateway::_questoneday::sAchievementInfo983) == 0x1c);
C_ASSERT_VC10(sizeof(_gateway::_questoneday::CQuestDocument) == 0xb0);
