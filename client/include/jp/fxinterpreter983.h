#pragma once

// JP 983: FxInterpreter da unidade 0087f2b0–008907d0 (RTTI .?AVFxInterpreter@@, vtable 00d34928; sFxInfoBase 00d34908,
// sFxInfo 00d34910; tr1::_Ref_count<sFxInfo> 00d348f4). Layout medido no construtor 0088e520 e no destrutor 0088dee0
// (sizeof 0x1e0, C_ASSERT no .cpp). Substitui o src/projectg/fxinterpreter.h da frente A (cabeçalho do 645 com o layout
// provisório; pedido B62 em out/PEDIDOS.md). Nomes de membro e de método são meus, salvo os que a A já usa.
// Sessão B62 (F3.7.7, 1º lote).

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "fxbox.h"
#include "fx.h" // Attacher (membro por valor do sFxInfo)

class WPuppet;
class CFxBox;
class CPetFrame;
struct sClubSet;

class FxInterpreter : public CFxKeeper
{
public:
	enum eScripFormat
	{
		eSF_FX,
		eSF_PLAYIF,
	};

	enum eEffectType
	{
		eET_NONE,
		eET_SEQUENCE,
		eET_SPRAY,
	};

	// 0x24 (vtable 00d34908): nome do efeito e se está ligado
	struct sFxInfoBase
	{
		sFxInfoBase() : m_bActive(true) { }
		sFxInfoBase(const char* name, bool bActive) // 00884620 (embutido no ApplyMode; fora de linha no 008905d0)
		{
			if (name)
				m_name = name;
			m_bActive = bActive;
		}
		virtual ~sFxInfoBase() { }

		bool operator==(const sFxInfoBase& other) const { return m_name == other.m_name; }

		std::string m_name; // +0x04
		bool m_bActive; // +0x20
	};

	// 0x88 (vtable 00d34910): um efeito criado pelo interpretador
	struct sFxInfo : public sFxInfoBase
	{
		sFxInfo(int id, int bKeep, int type, int group, void* pFx, const Attacher& attacher, const char* name, bool bMark,
			unsigned long tid, const char* cond); // 008846e0
		sFxInfo(int bKeep, int type, int group, const Attacher& attacher, const char* name, unsigned long tid,
			const char* cond); // 008847e0 (efeito ainda não criado: id 0, pFx NULL)
		virtual ~sFxInfo(); // 00883010 (fora de linha: o 983 chama)

		void Close(CFxBox* pFxBox); // 00880cd0
		bool IsSame(const sFxInfo* other) const; // 008848e0

		int m_id; // +0x24: id devolvido pelo Parse*/Attach*
		int m_group; // +0x28
		int m_bKeep; // +0x2c: fica na lista de pendentes (m_pending) quando o efeito acaba
		int m_type; // +0x30: eEffectType
		void* m_pFx; // +0x34: CFxSequence* / CFxSpray*
		std::string m_cond; // +0x38
		Attacher m_attacher; // +0x54
		bool m_bMark; // +0x7c: CFx::Attach(…, bMark)
		unsigned long m_tid; // +0x80
		bool m_bShown; // +0x84: ligado pela última ShowPendingByCond (0088dc90)
		bool m_bAlive; // +0x85
	};

	typedef std::tr1::shared_ptr<sFxInfo> FxInfoPtr;
	typedef std::map<unsigned long, std::list<void*> > GroupMap;
	typedef std::map<std::string, std::list<unsigned long> > NamedMap;

	// functores dos std::for_each
	struct sCloseFx
	{
		sCloseFx(FxInterpreter* pOwner) : m_pOwner(pOwner) { }
		void operator()(void* pFx) { m_pOwner->CloseFx(pFx, eET_NONE, true); } // 008868e0 (for_each)
		FxInterpreter* m_pOwner;
	};
	struct sResetInfo
	{
		void operator()(FxInfoPtr& info) const { info.reset(); } // 008851f0 (for_each)
	};
	struct sToggleFx
	{
		sToggleFx(FxInterpreter* pOwner) : m_pOwner(pOwner) { }
		void operator()(FxInfoPtr& info) // 00885d10 (for_each): troca o estado de cada efeito do modo antigo
		{
			info->m_bActive = !info->m_bActive;
			m_pOwner->SetFxActive(info->m_type, info->m_pFx, info->m_bActive);
		}
		FxInterpreter* m_pOwner;
	};

public:
	FxInterpreter(CFxBox* pFxBox, int firstId); // 0088e520
	virtual ~FxInterpreter(); // 0088dee0
	virtual void EmptyFx(void* fx); // 00884e10

	void SetFxBox(CFxBox* pFxBox); // 0087f2d0
	void Process(float elapsed); // 0087f320
	void Display(); // 0087f370

	void SetMode(int mode); // 0088c460 (A: SetUnk983_1cc)
	int ParseBoundingBoxScript_fx(WPuppet* pet); // 0088e860: *fx( nas caixas de colisão; devolve o id (0 = nada)
	bool PlayFx983(const char* file, const WVector* pos); // 0088aa60 (.seq/.spr solto numa posição; nome da A)
	void StopFx983(const char* file); // 0088aef0
	static int GetFxType(std::string name); // 00888d40: pela extensão (.seq/.spr)
	static void Preload(const char* name); // 00889d60
	void DetachFx(int id); // 00889df0
	void DetachFxAll(int slot = -1); // 0088c420
	void CloseGroup(int slot, unsigned long group); // 00887c10

	// efeito já ligado com o mesmo nome/condição/acoplador no grupo (pedidos B33: FindSpr983/FindSeq983)
	void* FindSpray(std::string name, const char* cond, const Attacher* attacher, unsigned long group, int slot,
		unsigned long tid); // 00885570
	void* FindSequence(std::string name, const char* cond, const Attacher* attacher, unsigned long group, int slot,
		unsigned long tid); // 008856c0
	// cria e liga (pedido B33: Attach983); devolve o id (bSameId: não avança o contador)
	int Attach(const char* name, const char* cond, const Attacher* attacher, int type, bool bMark, unsigned long group,
		int slot, bool bSameId); // 0088d3c0
	int Attach(const char* name, const Attacher* attacher, int type, bool bMark, unsigned long group, int slot,
		bool bSameId); // 0088d230
	// efeito pendente (criado só quando a condição/nome aparecer); devolve o tid
	unsigned long AttachPending(const char* key, const char* name, const char* cond, const Attacher* attacher, int type,
		bool bMark, unsigned long group, int slot, bool bSameId, bool bSameTid); // 0088d4a0
	unsigned long AttachPending(const char* cond, const char* name, const Attacher* attacher, int type, bool bMark,
		unsigned long group, int slot, bool bSameId, bool bSameTid); // 0088d610

	// *fx( nas caixas do boneco de um CPetFrame (devolvem o id, 0 = nada); "&tid" depois do nome = só se a peça estiver vestida
	int ParsePetFrameScript_fx(CPetFrame* pFrame, const float* pos, bool bParts, int slot); // 0088ecd0
	int ParseAvatarScript_fx(CPetFrame* pFrame, sClubSet* pClubSet, int slot, bool bShowAll,
		bool bKeyOnly); // 0088f2b0 (peças, peças extras do jogador da vaga e o taco)
	int ParseClubScript_fx(sClubSet* pClubSet, int slot); // 0088f8f0: cabeça do taco atual
	// liga/desliga os pendentes de uma chave "#…" (slot < 0 ou ≥ 4 = todas as vagas)
	void ShowNamed(const char* key, bool bShow, int slot); // 0088fe40
	bool ShowAllNamed(bool bShow, int slot); // 0088fff0
	void SetCond(const char* cond, unsigned int slot); // 008900e0: troca a condição "@…" da vaga
	void AddModeRule(const char* name, int mode, bool bActive); // 008905d0
	void DetachNamed(int slot); // 0088b8d0 (A: DetachC983); público: ntAvatar 007a4dae chama direto

protected:
	void RegisterKeeper(CFxBox* pFxBox); // 00880d40
	void InitTokens(); // 0088c6b0
	bool FindToken(const char** str, int* format, int* index) const; // 008822c0

	bool IsSprayGone(void* spray); // 00884a00
	bool IsSequenceGone(void* sequence); // 00884a60
	void SetFxActive(int type, void* pFx, bool bActive); // 00884be0
	void* CreateFx(const char* name, const Attacher* attacher, int type, bool bMark, bool bActive); // 00883bb0
	void* Reopen(sFxInfo* info); // 00883d00
	void CloseFx(void* pFx, int type, bool bErase); // 00885840
	void CloseInfo(sFxInfo* info, bool bErase); // 00888eb0

	sFxInfo* FindInfo(void* pFx); // 00884cc0
	void ResetInfo(void* pFx); // 00884c80
	sFxInfo* FindPendingById(int id); // 00883d40
	sFxInfo* FindPendingByFx(void* pFx); // 00883d70
	void ErasePendingByTid(unsigned long tid); // 00887ef0
	void CloseByTid(unsigned long tid); // 008890a0

	void DetachGroups(int slot); // 0088b140 (A: DetachA983)
	void DetachOrdered(int slot); // 00887cb0 (A: DetachB983)
	void DetachSlot(int id, int slot); // 00888f00
	void ClearInfos(); // 008890f0
	void ClearPending(); // 00887f80
	bool ApplyMode(int reserved); // 0088ba90 (argumento sem uso)

	bool ParseKey(const char** str, char* key, int format, int index); // 00882350: "#chave" logo depois da ficha
	bool ParseName(const char** str, char* name, std::vector<std::string>* conds, int format, int index,
		bool bAfterKey); // 0088c8b0: nome do efeito e condições "@…"
	// liga o efeito em cada grupo (kind 0 = já, 1 = pendente pela chave, 2 = pendente pela condição)
	bool AttachGroups(int kind, const char* name, const char* key, std::vector<std::string>& conds,
		std::vector<unsigned long>& groups, const Attacher* attacher, int type, int slot); // 0088e210
	bool IsSameInfo(sFxInfo info, sFxInfo* other); // 00884ac0
	bool IsActiveInMode(FxInfoPtr& info); // 00884b20
	sFxInfo* FindPendingSame(sFxInfo* info); // 00884d50
	sFxInfo* AddPending(sFxInfo* info, bool* pbNew); // 00887db0
	void AddInfo(void* pFx, sFxInfo* info); // 0088b1f0
	bool AddToGroup(int id, void* pFx, unsigned long group, int slot, bool bMark); // 0088ca30
	bool AddFx(int id, int type, void* pFx, const Attacher* attacher, const char* name, unsigned long group, int slot,
		bool bMark, const char* cond); // 0088cb80
	void AddNamedA(unsigned long tid, const char* key, int slot); // 0088cc70
	void AddNamedB(unsigned long tid, const char* cond, int slot); // 0088ced0
	bool Revive(FxInfoPtr& info, int slot); // 0088d8f0
	bool ShowPendingByTid(unsigned long tid, unsigned int slot, bool bShow); // 0088dc00
	void ShowPendingByCond(unsigned long tid, bool bShow, unsigned int slot); // 0088dc90
	bool HasGroupOrder(unsigned int slot, unsigned long group); // 00883b70

	static int ms_count; // 00e7d634

	std::vector<std::string> m_tokens[2]; // +0x08: [eSF_FX] "*fx" "*fxstop" …, [eSF_PLAYIF] "*playif" …
	GroupMap m_groups[4]; // +0x28: grupo → efeitos (chave do m_infos)
	std::list<unsigned long> m_groupOrder[4]; // +0x68
	std::map<void*, FxInfoPtr> m_infos; // +0x98: efeito → info
	NamedMap m_namedA[4]; // +0xa8
	NamedMap m_namedB[4]; // +0xe8
	std::list<FxInfoPtr> m_pending; // +0x128
	std::map<int, std::vector<sFxInfoBase> > m_modeRules; // +0x134: modo → efeitos que o modo liga/desliga
	std::list<FxInfoPtr> m_modeFx; // +0x144
	CFxBox* m_pFxBox983; // +0x150: caixa própria (NULL = CFx global)
	int m_nextId; // +0x154
	int m_158; // +0x158
	std::string m_curCond[4]; // +0x15c: condição atual ("@…") de cada vaga (SetCond 008900e0)
	int m_mode; // +0x1cc
	std::map<std::string, void*> m_playFx; // +0x1d0: PlayFx983, último efeito por arquivo
};
