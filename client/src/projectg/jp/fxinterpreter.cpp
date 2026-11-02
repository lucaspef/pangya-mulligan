// JP 983: unidade 0087f2b0–008907d0 — FxInterpreter (efeitos .seq/.spr ligados por roteiro: *fx(, *playif(, *fxstop( nas
// caixas de colisão, nos quadros do boneco e no taco). Cabeçalho próprio (include/jp/fxinterpreter983.h): o
// src/projectg/fxinterpreter.h da A tem o layout provisório do 645 (pedido B62). Os moldes std (map/list/vector/tr1::
// shared_ptr) são a maior parte da faixa (≈ 500 das 620 funções) e saem dos tipos dos membros. Sessão B62: 1º lote.
#define REBANG_FXSEQUENCE_CPP
#define REBANG_FX_CPP
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "wpuppet.h"
#include "wbone.h"
#include "fx.h"
#include "wind.h"
#include "fxinterpreter983.h"
#include "partsinfo.h"
#include "club.h"
#include "shareddoc.h"
#include <algorithm>

extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B1392 temporario

extern WResourceManager* g_resrcmng;
extern WMatrix g_camera;
extern WView* g_view;
const char* GetStrConst(const char* str, char* out);
const char* GetCharsTillSpace(const char* src, char* dst);

// Campos do 983 que os cabeçalhos da A (CFxSequence/CFxSpray do 645) não têm: acesso pelo deslocamento medido.
template <class T> static inline T& FxField983(void* p, int offset)
{
	return *(T*)((char*)p + offset);
}

// Attacher::operator!= do 983 (00878d60, fx.cpp da A; o fx.h da A não declara: pedido B63)
struct AttacherJp : public Attacher
{
	bool operator!=(const Attacher& other) const;
};

bool AttacherJp::operator!=(const Attacher& other) const // 00878d60: x, y e depois mat[4] e pos[4] (iguais = false)
{
	if (x != other.x || y != other.y)
		return true;
	for (int i = 0; i < 4; i++)
	{
		if (mat[i] != other.mat[i] || pos[i] != other.pos[i])
			return true;
	}
	return false;
}

// vento do 983 (wind.cpp da A; pedido B63): 006cc5f0 devolve por valor um objeto do vento e 006c8080 sorteia a mudança
// (0x30 bytes) que o .spr "wind_change*" copia para +0x1160 (00880c50). Nomes meus.
struct sWindChangeJp983
{
	WVector v[4];
};
struct CWindChangerJp983 : public CWind
{
	sWindChangeJp983 Roll983(); // 006c8080
};
CWindChangerJp983& WindChanger983() { return *(CWindChangerJp983*)&Wind(); } // 006cc5f0 (983: Wind -> FUN_006cc4b0, o singleton do vento)

// 004d1740: matriz 3x3 de rotacao de `angle` em torno do eixo 0 (x), 1 (y) ou 2 (z); o pivo sai zerado
static void RotAxis983(float* m, float angle, char axis)
{
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	if (axis == 0)
	{
		m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f;
		m[3] = 0.0f; m[4] = c; m[5] = -s;
		m[6] = 0.0f; m[7] = s; m[8] = c;
	}
	else if (axis == 1)
	{
		m[0] = c; m[1] = 0.0f; m[2] = s;
		m[3] = 0.0f; m[4] = 1.0f; m[5] = 0.0f;
		m[6] = -s; m[7] = 0.0f; m[8] = c;
	}
	else
	{
		m[0] = c; m[1] = s; m[2] = 0.0f;
		m[3] = -s; m[4] = c; m[5] = 0.0f;
		m[6] = 0.0f; m[7] = 0.0f; m[8] = 1.0f;
	}
	m[9] = m[10] = m[11] = 0.0f;
}

// 004d1600: out = a * b (linhas de 3 floats + pivo de 3 floats; o pivo de b soma ao final)
static void MulMat983(float* out, const float* a, const float* b)
{
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 3; c++)
			out[r * 3 + c] = a[r * 3 + 2] * b[6 + c] + a[r * 3 + 1] * b[3 + c] + a[r * 3] * b[c] + (r == 3 ? b[9 + c] : 0.0f);
}

// 006c8080: sorteia 10..19 graus para mais ou para menos em cima da direcao do vento e devolve a rotacao
// (eixos y e z; com vento vertical |y| >= 1e-5 passa por mais um giro de +-90 graus no eixo x)
sWindChangeJp983 CWindChangerJp983::Roll983()
{
	float deg = (float)((rand() % 10 + 10) * 0.01745329238474369);
	float angle;
	if ((rand() & 0x80000001) == 0)
		angle = GetDirectionSum983() + deg;
	else
		angle = GetDirectionSum983() - deg;
	float y = FxField983<float>(this, 0xc);
	if (y < 0.0f)
		y = -y;
	sWindChangeJp983 result;
	float* out = (float*)&result;
	float ry[12], rz[12];
	if (1.0000000e-5f <= y)
	{
		float half = FxField983<float>(this, 0xc) > 0.0f ? 1.5707963705062866f : -1.5707963705062866f;
		float rx[12], t[12];
		RotAxis983(ry, angle, 1);
		RotAxis983(rx, half, 0);
		RotAxis983(rz, angle, 2);
		MulMat983(t, rz, rx);
		MulMat983(out, t, ry);
	}
	else
	{
		RotAxis983(ry, angle, 1);
		RotAxis983(rz, angle, 2);
		MulMat983(out, rz, ry);
	}
	return result;
}

struct CFxSprayJp983 : public CFxSpray
{
	void SetWindChange983(const sWindChangeJp983* change); // 00880c50
};

int FxInterpreter::ms_count;

// 008846e0
FxInterpreter::sFxInfo::sFxInfo(int id, int bKeep, int type, int group, void* pFx, const Attacher& attacher,
	const char* name, bool bMark, unsigned long tid, const char* cond)
{
	m_bKeep = bKeep;
	m_id = id;
	m_pFx = pFx;
	m_type = type;
	m_group = group;
	m_attacher = attacher;
	m_bMark = bMark;
	m_tid = tid;
	m_bShown = false;
	m_bAlive = true;
	if (name)
		m_name = name;
	if (cond)
		m_cond = cond;
}

// 008847e0
FxInterpreter::sFxInfo::sFxInfo(int bKeep, int type, int group, const Attacher& attacher, const char* name,
	unsigned long tid, const char* cond)
{
	m_id = 0;
	m_pFx = NULL;
	m_type = type;
	m_bKeep = bKeep;
	m_group = group;
	m_attacher = attacher;
	m_bMark = false;
	m_tid = tid;
	m_bShown = false;
	m_bAlive = true;
	if (name)
		m_name = name;
	if (cond)
		m_cond = cond;
}

// 00883010
FxInterpreter::sFxInfo::~sFxInfo()
{
}

// 00880cd0
void FxInterpreter::sFxInfo::Close(CFxBox* pFxBox)
{
	if (pFxBox)
	{
		if (m_type == eET_SPRAY)
			pFxBox->DeleteSpray((CFxSpray*)m_pFx);
		else if (m_type == eET_SEQUENCE)
			pFxBox->DeleteSequence((CFxSequence*)m_pFx, true);
	}
	else if (CFx::Instance())
	{
		if (m_type == eET_SPRAY)
			CFx::Instance()->CloseSpray((CFxSpray*)m_pFx);
		else if (m_type == eET_SEQUENCE)
			CFx::Instance()->CloseSequence((CFxSequence*)m_pFx, true);
	}
}

// 00880c50
void CFxSprayJp983::SetWindChange983(const sWindChangeJp983* change)
{
	FxField983<sWindChangeJp983>(this, VC10_FXSPRAY_BGROT) = *change;
}

// 0087f2d0
void FxInterpreter::SetFxBox(CFxBox* pFxBox)
{
	if (pFxBox)
		m_pFxBox983 = pFxBox;
	if (m_pFxBox983)
	{
		m_pFxBox983->Init(g_resrcmng);
		m_pFxBox983->SetCamera(&g_camera);
		m_pFxBox983->m_frameTime = 1000.0f / 30.0f;
	}
}

// 0087f320
void FxInterpreter::Process(float elapsed)
{
	if (m_pFxBox983)
	{
		elapsed *= 1000.0;
		m_pFxBox983->ProcessAttacher(elapsed);
		m_pFxBox983->Process(elapsed);
	}
}

// 0087f370
void FxInterpreter::Display()
{
	if (m_pFxBox983)
		m_pFxBox983->Display(g_view);
}

// 00880d40
void FxInterpreter::RegisterKeeper(CFxBox* pFxBox)
{
	sFxKeeper keeper = { this, &CFxKeeper::EmptyFx };

	if (pFxBox)
		pFxBox->AddFxKeeper(keeper);
	else
		CFx::Instance()->AddFxKeeper(keeper);
}

// 008822c0: acha a 1ª ficha no texto, primeiro na forma com parêntese
bool FxInterpreter::FindToken(const char** str, int* format, int* index) const
{
	*index = 3;
	for (int f = 1; f > -1; f--)
	{
		for (int i = 0; i < 3; i++)
		{
			const char* p = strstr(*str, m_tokens[f][i].c_str());
			if (p)
			{
				*format = f;
				*index = i;
				*str = p;
				return true;
			}
		}
	}
	return false;
}

// 00884a00
bool FxInterpreter::IsSprayGone(void* spray)
{
	CFxBox* pFxBox = m_pFxBox983;
	if (pFxBox)
		return pFxBox->m_spray.find((CFxSpray*)spray) == pFxBox->m_spray.end();
	return !CFx::Instance()->FindSpray((CFxSpray*)spray);
}

// 00884a60
bool FxInterpreter::IsSequenceGone(void* sequence)
{
	CFxBox* pFxBox = m_pFxBox983;
	if (pFxBox)
		return pFxBox->m_sequence.find((CFxSequence*)sequence) == pFxBox->m_sequence.end();
	return !CFx::Instance()->FindSequence((CFxSequence*)sequence);
}

// B1418 temporario (item 15 do dono: brinco anexado no parse do buraco 1 some depois): nome do fx do brinco ou NULL
static const char* B1418_EarName(std::map<void*, FxInterpreter::FxInfoPtr>& infos, void* pFx)
{
	std::map<void*, FxInterpreter::FxInfoPtr>::iterator it = infos.find(pFx);
	if (it == infos.end() || !it->second || !strstr(it->second->m_name.c_str(), "G_earing"))
		return NULL;
	return it->second->m_name.c_str();
}

// 00884be0
void FxInterpreter::SetFxActive(int type, void* pFx, bool bActive)
{
	if (!pFx)
		return;
	if (const char* ear = B1418_EarName(m_infos, pFx)) // B1418 temporario
		jp983_rastro("B1418 ativo %s fx=%p ativo=%d modo=%d", ear, pFx, (int)bActive, m_mode);
	if (type == eET_SPRAY)
	{
		if (!IsSprayGone(pFx))
			FxField983<bool>(pFx, VC10_FXSPRAY_ACTIVE) = bActive;
	}
	else if (type == eET_SEQUENCE)
	{
		if (!IsSequenceGone(pFx))
			((CFxSequence*)pFx)->SetActive(bActive, true);
	}
}

// 00883bb0
void* FxInterpreter::CreateFx(const char* name, const Attacher* attacher, int type, bool bMark, bool bActive)
{
	int len = strlen(name);
	if (len < 5)
		return NULL;

	if (type == eET_SPRAY)
	{
		CFxSpray* spray;
		if (m_pFxBox983)
		{
			spray = m_pFxBox983->LoadSpray(name, NULL);
			if (!spray)
				return NULL;
			FxField983<bool>(spray, 4) = bMark;
			FxField983<const Attacher*>(spray, 8) = attacher;
			Attacher& att = FxField983<Attacher>(spray, 0xc);
			att.x = attacher->x;
			att.y = attacher->y;
			att.mat[0] = attacher->mat[0];
			att.mat[1] = attacher->mat[1];
			att.mat[2] = attacher->mat[2];
			att.mat[3] = attacher->mat[3];
			att.pos[0] = attacher->pos[0];
			att.pos[1] = attacher->pos[1];
			att.pos[2] = attacher->pos[2];
			att.pos[3] = attacher->pos[3];
			m_pFxBox983->AddSpray(spray);
		}
		else
		{
			spray = (CFxSpray*)CFx::Instance()->Attach(*attacher, name, bMark, NULL);
			if (!spray)
				return NULL;
		}
		FxField983<bool>(spray, VC10_FXSPRAY_ACTIVE) = bActive;
		return spray;
	}

	CFxSequence* sequence;
	if (m_pFxBox983)
	{
		sequence = m_pFxBox983->LoadSequence(name, true, NULL);
		if (!sequence)
			return NULL;
		FxField983<bool>(sequence, 4) = bMark;
		FxField983<const Attacher*>(sequence, 0x14) = attacher;
		FxField983<float>(sequence, 0x18) = attacher->x;
		FxField983<float>(sequence, 0x1c) = attacher->y;
		m_pFxBox983->AddSequence(sequence);
	}
	else
	{
		sequence = (CFxSequence*)CFx::Instance()->Attach(*attacher, name, bMark, NULL);
		if (!sequence)
			return NULL;
	}
	if (FxField983<bool>(sequence, VC10_FXSEQ_ACTIVE) != bActive)
		FxField983<bool>(sequence, VC10_FXSEQ_ACTIVE) = bActive;
	return sequence;
}

// 00883d00: recria o efeito de uma info pendente
void* FxInterpreter::Reopen(sFxInfo* info)
{
	void* pFx = CreateFx(info->m_name.c_str(), &info->m_attacher, info->m_type, info->m_bMark, true);
	if (pFx)
	{
		info->m_bAlive = true;
		return pFx;
	}
	return NULL;
}

// 00885840
void FxInterpreter::CloseFx(void* pFx, int type, bool bErase)
{
	if (!pFx)
		return;
	if (const char* ear = B1418_EarName(m_infos, pFx)) // B1418 temporario
		jp983_rastro("B1418 fecha %s fx=%p erase=%d", ear, pFx, (int)bErase);

	CFxBox* pFxBox = m_pFxBox983;
	if (pFxBox)
	{
		if (type == eET_NONE)
		{
			if (pFxBox->m_spray.find((CFxSpray*)pFx) != pFxBox->m_spray.end())
				type = eET_SPRAY;
			if (pFxBox->m_sequence.find((CFxSequence*)pFx) != pFxBox->m_sequence.end())
				type = eET_SEQUENCE;
		}
		if (type == eET_SPRAY)
			pFxBox->DeleteSpray((CFxSpray*)pFx);
		else if (type == eET_SEQUENCE)
			pFxBox->DeleteSequence((CFxSequence*)pFx, true);
	}
	else
	{
		if (!CFx::Instance())
			return;
		if (type == eET_NONE)
		{
			if (CFx::Instance()->FindSpray((CFxSpray*)pFx))
				type = eET_SPRAY;
			if (CFx::Instance()->FindSequence((CFxSequence*)pFx))
				type = eET_SEQUENCE;
		}
		if (type == eET_SPRAY)
			CFx::Instance()->CloseSpray((CFxSpray*)pFx);
		else if (type == eET_SEQUENCE)
			CFx::Instance()->CloseSequence((CFxSequence*)pFx, true);
	}
	if (bErase)
		ResetInfo(pFx);
}

// 00888eb0
void FxInterpreter::CloseInfo(sFxInfo* info, bool bErase)
{
	if (!info)
		return;
	// B1392 temporario (item 15 do dono: brinco apaga no 1o buraco/HIO)
	if (strstr(info->m_name.c_str(), "earing"))
		jp983_rastro("B1392 fx fecha %s tid=%lu keep=%d erase=%d cond=%s", info->m_name.c_str(), info->m_tid,
			(int)info->m_bKeep, (int)bErase, info->m_cond.c_str());
	CloseFx(info->m_pFx, info->m_type, bErase);
	if (!info->m_bKeep)
		return;
	if (bErase)
		ErasePendingByTid(info->m_tid);
	else
		info->m_bAlive = false;
}

// 00884cc0
FxInterpreter::sFxInfo* FxInterpreter::FindInfo(void* pFx)
{
	std::map<void*, FxInfoPtr>::iterator it = m_infos.find(pFx);
	return it != m_infos.end() ? it->second.get() : NULL;
}

// 00884c80
void FxInterpreter::ResetInfo(void* pFx)
{
	if (!pFx)
		return;
	std::map<void*, FxInfoPtr>::iterator it = m_infos.find(pFx);
	if (it != m_infos.end())
		it->second.reset();
}

// 00884e10 (CFxKeeper: a caixa avisa que o efeito acabou)
void FxInterpreter::EmptyFx(void* fx)
{
	if (!fx)
		return;
	if (const char* ear = B1418_EarName(m_infos, fx)) // B1418 temporario
		jp983_rastro("B1418 acabou %s fx=%p", ear, fx);
	std::map<void*, FxInfoPtr>::iterator it = m_infos.find(fx);
	if (it != m_infos.end() && it->second.use_count() > 0)
		it->second.reset();
}

// 00883d40
FxInterpreter::sFxInfo* FxInterpreter::FindPendingById(int id)
{
	for (std::list<FxInfoPtr>::iterator it = m_pending.begin(); it != m_pending.end(); ++it)
	{
		if ((*it)->m_id == id)
			return it->get();
	}
	return NULL;
}

// 00883d70
FxInterpreter::sFxInfo* FxInterpreter::FindPendingByFx(void* pFx)
{
	for (std::list<FxInfoPtr>::iterator it = m_pending.begin(); it != m_pending.end(); ++it)
	{
		if ((*it)->m_pFx == pFx)
			return it->get();
	}
	return NULL;
}

// 00887ef0
void FxInterpreter::ErasePendingByTid(unsigned long tid)
{
	for (std::list<FxInfoPtr>::iterator it = m_pending.begin(); it != m_pending.end(); ++it)
	{
		if ((*it)->m_tid == tid)
		{
			it->reset();
			m_pending.remove(*it);
			return;
		}
	}
}

// 008890a0
void FxInterpreter::CloseByTid(unsigned long tid)
{
	if (!tid)
		return;
	for (std::list<FxInfoPtr>::iterator it = m_pending.begin(); it != m_pending.end(); ++it)
	{
		if ((*it)->m_tid == tid)
		{
			CloseInfo(it->get(), false);
			(*it)->m_pFx = NULL;
		}
	}
}

// 00887c10
void FxInterpreter::CloseGroup(int slot, unsigned long group)
{
	if ((unsigned int)slot < 4)
	{
		std::map<unsigned long, std::list<void*> >::iterator it = m_groups[slot].find(group);
		if (it != m_groups[slot].end())
		{
			std::for_each(it->second.begin(), it->second.end(), sCloseFx(this));
			it->second.clear();
		}
	}
}

// 0088b140
void FxInterpreter::DetachGroups(int slot)
{
	int first = 0;
	int last = 4;
	if (slot > -1 && slot < 4)
	{
		first = slot;
		last = slot + 1;
	}
	for (int i = first; i < last; i++)
	{
		for (std::map<unsigned long, std::list<void*> >::iterator it = m_groups[i].begin(); it != m_groups[i].end(); ++it)
			CloseGroup(i, it->first);
		m_groups[i].clear();
	}
}

// 00887cb0
void FxInterpreter::DetachOrdered(int slot)
{
	int first = 0;
	int last = 4;
	if (slot > -1 && slot < 4)
	{
		first = slot;
		last = slot + 1;
	}
	for (int i = first; i < last; i++)
	{
		for (std::list<unsigned long>::iterator it = m_groupOrder[i].begin(); it != m_groupOrder[i].end(); ++it)
		{
			std::map<unsigned long, std::list<void*> >::iterator found = m_groups[i].find(*it);
			if (found != m_groups[i].end())
				CloseGroup(i, found->first);
		}
		m_groupOrder[i].clear();
	}
}

// 0088b8d0
void FxInterpreter::DetachNamed(int slot)
{
	int first = 0;
	int last = 4;
	if (slot > -1 && slot < 4)
	{
		first = slot;
		last = slot + 1;
	}
	for (int i = first; i < last; i++)
	{
		std::map<std::string, std::list<unsigned long> >::iterator it;
		for (it = m_namedA[i].begin(); it != m_namedA[i].end(); ++it)
		{
			for (std::list<unsigned long>::iterator l = it->second.begin(); l != it->second.end(); ++l)
				CloseByTid(*l);
			it->second.clear();
		}
		for (it = m_namedB[i].begin(); it != m_namedB[i].end(); ++it)
		{
			for (std::list<unsigned long>::iterator l = it->second.begin(); l != it->second.end(); ++l)
				CloseByTid(*l);
			it->second.clear();
		}
	}
}

// 00888f00
void FxInterpreter::DetachSlot(int id, int slot)
{
	if ((unsigned int)slot > 4)
		return;
	for (std::map<unsigned long, std::list<void*> >::iterator it = m_groups[slot].begin(); it != m_groups[slot].end(); ++it)
	{
		std::vector<std::list<void*>::iterator> dead;
		for (std::list<void*>::iterator l = it->second.begin(); l != it->second.end(); ++l)
		{
			sFxInfo* info = FindInfo(*l);
			if (!info)
				info = FindPendingByFx(*l);
			if (info && info->m_id == id)
			{
				CloseInfo(info, info->m_bKeep == 0);
				dead.push_back(l);
			}
		}
		for (unsigned int i = 0; i < dead.size(); i++)
		{
			if (dead[i] != it->second.end())
				it->second.erase(dead[i]);
		}
	}
}

// 00889df0
void FxInterpreter::DetachFx(int id)
{
	for (int i = 0; i < 4; i++)
		DetachSlot(id, i);
}

// 0088c420
void FxInterpreter::DetachFxAll(int slot)
{
	DetachGroups(slot);
	DetachOrdered(slot);
	DetachNamed(slot);
	if (slot == -1)
		m_nextId = 1;
}

// 008890f0
void FxInterpreter::ClearInfos()
{
	for (std::map<void*, FxInfoPtr>::iterator it = m_infos.begin(); it != m_infos.end(); ++it)
		it->second.reset();
	m_infos.clear();
}

// 00887f80
void FxInterpreter::ClearPending()
{
	std::for_each(m_pending.begin(), m_pending.end(), sResetInfo());
	m_pending.clear();
}

// 0088c460 (A: SetUnk983_1cc)
void FxInterpreter::SetMode(int mode)
{
	if (m_mode != mode)
	{
		std::for_each(m_modeFx.begin(), m_modeFx.end(), sToggleFx(this));
		m_modeFx.clear();
		m_mode = mode;
		ApplyMode(0);
	}
}

// 0088c6b0
void FxInterpreter::InitTokens()
{
	m_tokens[eSF_FX].push_back(std::string("*fx"));
	m_tokens[eSF_FX].push_back(std::string("*playif"));
	m_tokens[eSF_FX].push_back(std::string("*fxstop"));
	m_tokens[eSF_PLAYIF].push_back(std::string("*fx("));
	m_tokens[eSF_PLAYIF].push_back(std::string("*playif("));
	m_tokens[eSF_PLAYIF].push_back(std::string("*fxstop("));
}

// 008848e0
bool FxInterpreter::sFxInfo::IsSame(const sFxInfo* other) const
{
	if (m_bKeep != other->m_bKeep || m_type != other->m_type || m_group != other->m_group)
		return false;
	if (m_name != other->m_name || m_cond != other->m_cond)
		return false;
	if ((const AttacherJp&)m_attacher != other->m_attacher)
		return false;
	return m_bMark == other->m_bMark;
}

// 00884ac0
bool FxInterpreter::IsSameInfo(sFxInfo info, sFxInfo* other)
{
	return info.IsSame(other);
}

// 00884b20: o modo atual não desliga este efeito
bool FxInterpreter::IsActiveInMode(FxInfoPtr& info)
{
	std::map<int, std::vector<sFxInfoBase> >::iterator it = m_modeRules.find(m_mode);
	if (it != m_modeRules.end())
	{
		for (std::vector<sFxInfoBase>::iterator rule = it->second.begin(), end = it->second.end(); rule != end; ++rule)
		{
			if (info->m_name == rule->m_name)
				return false;
		}
	}
	return true;
}

// 00884d50
FxInterpreter::sFxInfo* FxInterpreter::FindPendingSame(sFxInfo* info)
{
	for (std::list<FxInfoPtr>::iterator it = m_pending.begin(); it != m_pending.end(); ++it)
	{
		sFxInfo* pending = it->get();
		if (pending->IsSame(info))
			return pending;
	}
	return NULL;
}

// 00885570
void* FxInterpreter::FindSpray(std::string name, const char* cond, const Attacher* attacher, unsigned long group,
	int slot, unsigned long tid)
{
	if ((unsigned int)slot < 4 && !m_groups[slot].empty())
	{
		GroupMap::iterator it = m_groups[slot].find(group);
		if (it != m_groups[slot].end())
		{
			for (std::list<void*>::iterator l = it->second.begin(); l != it->second.end(); ++l)
			{
				void* pFx = *l;
				std::map<void*, FxInfoPtr>::iterator found = m_infos.find(pFx);
				if (found != m_infos.end() && found->second)
				{
					sFxInfo* info = found->second.get();
					if (IsSameInfo(sFxInfo(info->m_bKeep, eET_SPRAY, info->m_group, *attacher, name.c_str(), tid, cond),
						info))
						return pFx;
				}
			}
		}
	}
	return NULL;
}

// 008856c0
void* FxInterpreter::FindSequence(std::string name, const char* cond, const Attacher* attacher, unsigned long group,
	int slot, unsigned long tid)
{
	if ((unsigned int)slot < 4 && !m_groups[slot].empty())
	{
		GroupMap::iterator it = m_groups[slot].find(group);
		if (it != m_groups[slot].end())
		{
			for (std::list<void*>::iterator l = it->second.begin(); l != it->second.end(); ++l)
			{
				void* pFx = *l;
				std::map<void*, FxInfoPtr>::iterator found = m_infos.find(pFx);
				if (found != m_infos.end() && found->second)
				{
					sFxInfo* info = found->second.get();
					if (IsSameInfo(sFxInfo(info->m_bKeep, eET_SEQUENCE, info->m_group, *attacher, name.c_str(), tid,
						cond), info))
						return pFx;
				}
			}
		}
	}
	return NULL;
}

// 00887db0: põe na lista de pendentes (ou devolve a igual que já está lá)
FxInterpreter::sFxInfo* FxInterpreter::AddPending(sFxInfo* info, bool* pbNew)
{
	*pbNew = false;
	if (!info)
		return NULL;
	sFxInfo* found = FindPendingSame(info);
	if (!found)
	{
		info->m_bShown = false;
		info->m_bAlive = false;
		m_pending.push_back(FxInfoPtr(info));
		*pbNew = true;
		return info;
	}
	delete info;
	return found;
}

// 0088b1f0
void FxInterpreter::AddInfo(void* pFx, sFxInfo* info)
{
	if (pFx && info)
	{
		ResetInfo(pFx);
		FxInfoPtr p(info);
		m_infos[pFx] = p;
	}
}

// 0088ca30
bool FxInterpreter::AddToGroup(int id, void* pFx, unsigned long group, int slot, bool bMark)
{
	if (bMark || (unsigned int)slot >= 4)
		return false;
	GroupMap::iterator it = m_groups[slot].find(group);
	if (it != m_groups[slot].end())
	{
		it->second.push_back(pFx);
		return true;
	}
	std::list<void*> l;
	l.push_back(pFx);
	m_groups[slot].insert(GroupMap::value_type(group, l));
	return true;
}

// 0088cb80
bool FxInterpreter::AddFx(int id, int type, void* pFx, const Attacher* attacher, const char* name, unsigned long group,
	int slot, bool bMark, const char* cond)
{
	if (!bMark && AddToGroup(id, pFx, group, slot, false))
	{
		AddInfo(pFx, new sFxInfo(m_nextId, 0, type, group, pFx, *attacher, name, false, 0, cond));
		if (group)
			m_groupOrder[slot].push_back(group);
		return true;
	}
	return false;
}

// 0088cc70
void FxInterpreter::AddNamedA(unsigned long tid, const char* key, int slot)
{
	NamedMap::iterator it = m_namedA[slot].find(std::string(key));
	if (it == m_namedA[slot].end())
	{
		std::list<unsigned long> l;
		l.push_back(tid);
		m_namedA[slot].insert(NamedMap::value_type(key, l));
	}
	else
		it->second.push_back(tid);
}

// 0088ced0
void FxInterpreter::AddNamedB(unsigned long tid, const char* cond, int slot)
{
	NamedMap::iterator it = m_namedB[slot].find(std::string(cond));
	if (it == m_namedB[slot].end())
	{
		std::list<unsigned long> l;
		l.push_back(tid);
		m_namedB[slot].insert(NamedMap::value_type(cond, l));
	}
	else
		it->second.push_back(tid);
}

// 0088d230 (o 983 junta as duas chamadas ao construtor da info num rabo só: DIFERENTE por isso)
int FxInterpreter::Attach(const char* name, const Attacher* attacher, int type, bool bMark, unsigned long group,
	int slot, bool bSameId)
{
	if (!name || !attacher)
		return 0;
	void* pFx;
	if (type == eET_SPRAY)
	{
		pFx = CreateFx(name, attacher, eET_SPRAY, bMark, true);
		if (!pFx)
			return 0;
		if (AddToGroup(m_nextId, pFx, group, slot, bMark))
			AddInfo(pFx, new sFxInfo(m_nextId, 0, eET_SPRAY, group, pFx, *attacher, name, bMark, 0, NULL));
	}
	else
	{
		if (type != eET_SEQUENCE)
			return 0;
		pFx = CreateFx(name, attacher, eET_SEQUENCE, bMark, true);
		if (!pFx)
			return 0;
		if (AddToGroup(m_nextId, pFx, group, slot, bMark))
			AddInfo(pFx, new sFxInfo(m_nextId, 0, eET_SEQUENCE, group, pFx, *attacher, name, bMark, 0, NULL));
	}
	if (bSameId)
		return m_nextId;
	return m_nextId++;
}

// 0088d3c0
int FxInterpreter::Attach(const char* name, const char* cond, const Attacher* attacher, int type, bool bMark,
	unsigned long group, int slot, bool bSameId)
{
	if (name && attacher)
	{
		if (type == eET_SPRAY)
		{
			void* pFx = CreateFx(name, attacher, eET_SPRAY, bMark, true);
			if (pFx)
			{
				AddFx(m_nextId, eET_SPRAY, pFx, attacher, name, group, slot, bMark, cond);
				if (bSameId)
					return m_nextId;
				return m_nextId++;
			}
		}
		else if (type == eET_SEQUENCE)
		{
			void* pFx = CreateFx(name, attacher, eET_SEQUENCE, bMark, true);
			if (pFx)
			{
				AddFx(m_nextId, eET_SEQUENCE, pFx, attacher, name, group, slot, bMark, cond);
				if (bSameId)
					return m_nextId;
				return m_nextId++;
			}
		}
	}
	return 0;
}

// 0088d4a0
unsigned long FxInterpreter::AttachPending(const char* key, const char* name, const char* cond, const Attacher* attacher,
	int type, bool bMark, unsigned long group, int slot, bool bSameId, bool bSameTid)
{
	if (!key || !name || !attacher || (unsigned int)slot >= 4)
		return 0;
	bool bNew = false;
	sFxInfo* info = AddPending(new sFxInfo(m_nextId, 1, type, group, NULL, *attacher, name, bMark, m_158, cond), &bNew);
	unsigned long tid = 0;
	if (info)
	{
		AddNamedA(info->m_tid, key, slot);
		if (cond && strlen(cond) != 0)
			AddNamedB(info->m_tid, cond, slot);
		if (!bNew)
			return info->m_tid;
		if (!bSameId)
			m_nextId++;
		tid = m_158;
		if (!bSameTid)
			m_158++;
	}
	return tid;
}

// 0088d610
unsigned long FxInterpreter::AttachPending(const char* cond, const char* name, const Attacher* attacher, int type,
	bool bMark, unsigned long group, int slot, bool bSameId, bool bSameTid)
{
	if (!cond || !name || !attacher || (unsigned int)slot >= 4)
		return 0;
	bool bNew = false;
	sFxInfo* info = AddPending(new sFxInfo(m_nextId, 2, type, group, NULL, *attacher, name, bMark, m_158, cond), &bNew); // 983 0088d610: 2 (a outra sobrecarga passa 1); entra na comparacao de duplicados
	if (!info)
		return 0;
	AddNamedB(info->m_tid, cond, slot);
	if (!bNew)
		return info->m_tid;
	if (!bSameId)
		m_nextId++;
	unsigned long tid = m_158;
	if (!bSameTid)
		m_158++;
	return tid;
}

// 0088d8f0: recria um efeito pendente
bool FxInterpreter::Revive(FxInfoPtr& info, int slot)
{
	if (strstr(info->m_name.c_str(), "earing")) // B1392 temporario
		jp983_rastro("B1392 fx recria %s tid=%lu alive=%d slot=%d", info->m_name.c_str(), info->m_tid,
			(int)info->m_bAlive, slot);
	if (info->m_bAlive)
		return false;
	bool bActive = IsActiveInMode(info);
	if (info->m_type == eET_SPRAY || info->m_type == eET_SEQUENCE)
	{
		void* pFx = CreateFx(info->m_name.c_str(), &info->m_attacher, info->m_type, info->m_bMark, bActive);
		if (pFx)
		{
			info->m_pFx = pFx;
			info->m_bAlive = true;
			info->m_bActive = bActive;
			AddToGroup(info->m_id, pFx, info->m_group, slot, info->m_bMark);
		}
	}
	return info->m_pFx != NULL;
}

// 00888d40
int FxInterpreter::GetFxType(std::string name)
{
	int type = eET_NONE;
	int len = name.size();
	if (len == 0 || len >= 64)
		return eET_NONE;
	std::string::size_type dot = name.rfind('.');
	std::string ext;
	ext.assign(name.begin() + dot + 1, name.begin() + len);
	if (ext.size() == 0 || ext.size() < 3)
		return eET_NONE;
	if (ext == "seq")
		type = eET_SEQUENCE;
	else if (ext == "spr")
		type = eET_SPRAY;
	return type;
}

// 00889d60: carrega e solta (aquece o cache do arquivo)
void FxInterpreter::Preload(const char* name)
{
	if (!CFx::Instance())
		return;
	switch (GetFxType(name))
	{
	case eET_SEQUENCE:
		{
			CFxSequence* sequence = CFx::Instance()->PvsFxBox983()->LoadSequence(name, false, NULL);
			if (sequence)
				delete sequence;
		}
		break;
	case eET_SPRAY:
		{
			CFxSpray* spray = CFx::Instance()->PvsFxBox983()->LoadSpray(name, NULL);
			if (spray)
				delete spray;
		}
		break;
	}
}

// 0088ba90: liga/desliga os efeitos que o modo atual lista
bool FxInterpreter::ApplyMode(int reserved)
{
	std::map<int, std::vector<sFxInfoBase> >::iterator it = m_modeRules.find(m_mode);
	if (it != m_modeRules.end())
	{
		std::vector<sFxInfoBase>& rules = it->second;
		char buf[64];
		for (std::map<void*, FxInfoPtr>::iterator i = m_infos.begin(); i != m_infos.end(); ++i)
		{
			FxInfoPtr& info = i->second;
			if (info)
			{
				std::vector<sFxInfoBase>::iterator rule = std::find(rules.begin(), rules.end(),
					sFxInfoBase(info->m_name.c_str(), true));
				if (rule != rules.end())
				{
					info->m_bActive = rule->m_bActive;
					strcpy(buf, info->m_name.c_str());
					SetFxActive(GetFxType(buf), info->m_pFx, info->m_bActive);
					m_modeFx.push_back(info);
				}
			}
		}
		std::for_each(m_pending.begin(), m_pending.end(), [&](FxInterpreter::FxInfoPtr& info) // 00889e60 (operator()), 0088a5e0
		{
			std::vector<FxInterpreter::sFxInfoBase>::iterator rule = std::find(rules.begin(), rules.end(),
				FxInterpreter::sFxInfoBase(info->m_name.c_str(), true));
			if (rule != rules.end())
			{
				info->m_bActive = rule->m_bActive;
				strcpy(buf, info->m_name.c_str());
				SetFxActive(GetFxType(buf), info->m_pFx, info->m_bActive);
				m_modeFx.push_back(info);
			}
		});
	}
	return true;
}

// 0088aa60: abre o .seq/.spr numa posição (fecha o anterior do mesmo arquivo); "wind_change*.spr" recebe a mudança de vento
bool FxInterpreter::PlayFx983(const char* file, const WVector* pos)
{
	void* old = NULL;
	int type = GetFxType(file);
	std::map<std::string, void*>::iterator it = m_playFx.find(file);
	if (it != m_playFx.end())
		old = it->second;
	switch (type)
	{
	case eET_SEQUENCE:
		{
			if (old && CFx::Instance()->FindSequence((CFxSequence*)old))
			{
				CFx::Instance()->CloseSequence((CFxSequence*)old, false);
				m_playFx[std::string(file)] = NULL;
			}
			CFxSequence* sequence = CFx::Instance()->OpenSequence(file, true);
			if (!sequence)
				return false;
			sequence->m_pos = *pos; // A844: era FxField983 VC10_FXSEQ_POS
			m_playFx[file] = sequence;
		}
		return true;
	case eET_SPRAY:
		{
			if (old && CFx::Instance()->FindSpray((CFxSpray*)old))
			{
				CFx::Instance()->CloseSpray((CFxSpray*)old);
				m_playFx[std::string(file)] = NULL;
			}
			CFxSpray* spray = CFx::Instance()->OpenSpray(file);
			if (!spray)
				return false;
			FxField983<WVector>(spray, VC10_FXSPRAY_POS) = *pos;
			m_playFx[file] = spray;
			if (!strcmp(file, "wind_change1.spr") || !strcmp(file, "wind_change4.spr") ||
				!strcmp(file, "wind_change9.spr"))
				{
					sWindChangeJp983 change = WindChanger983().Roll983();
					((CFxSprayJp983*)spray)->SetWindChange983(&change);
				}
		}
		return true;
	}
	return false;
}

// 0088aef0
void FxInterpreter::StopFx983(const char* file)
{
	void* old = NULL;
	std::map<std::string, void*>::iterator it = m_playFx.find(file);
	if (it != m_playFx.end())
		old = it->second;
	int type = GetFxType(file);
	if (!old)
		return;
	switch (type)
	{
	case eET_SEQUENCE:
		if (CFx::Instance()->FindSequence((CFxSequence*)old))
			CFx::Instance()->CloseSequence((CFxSequence*)old, false);
		m_playFx[file] = NULL;
		break;
	case eET_SPRAY:
		if (CFx::Instance()->FindSpray((CFxSpray*)old))
			CFx::Instance()->CloseSpray((CFxSpray*)old);
		m_playFx[file] = NULL;
		break;
	}
}

// 00882350
bool FxInterpreter::ParseKey(const char** str, char* key, int format, int index)
{
	char* buf = new char[64];
	const char* src = *str;
	const char* next;
	switch (format)
	{
	case eSF_FX:
		next = GetCharsTillSpace(src + strlen(m_tokens[eSF_FX][index].c_str()), buf);
		break;
	case eSF_PLAYIF:
		next = GetStrConst(src + strlen(m_tokens[eSF_PLAYIF][index].c_str()), buf);
		break;
	default:
		return false;
	}
	int len = strlen(buf);
	if (len != 0 && len < 64)
	{
		if (buf[0] == '#')
		{
			*str = next;
			unsigned int i;
			for (i = 0; i < strlen(buf) - 1; i++)
				key[i] = buf[i + 1];
			key[i] = 0;
			delete[] buf;
			return true;
		}
		delete[] buf;
	}
	return false;
}

// 0088c8b0
bool FxInterpreter::ParseName(const char** str, char* name, std::vector<std::string>* conds, int format, int index,
	bool bAfterKey)
{
	switch (format)
	{
	case eSF_FX:
		if (bAfterKey)
			*str = GetStrConst(*str, name);
		else
			*str = GetCharsTillSpace(*str + strlen(m_tokens[eSF_FX][index].c_str()), name);
		break;
	case eSF_PLAYIF:
		if (bAfterKey)
			*str = GetStrConst(*str, name);
		else
			*str = GetStrConst(*str + strlen(m_tokens[eSF_PLAYIF][index].c_str()), name);
		break;
	default:
		return false;
	}
	char cond[64];
	cond[0] = 0;
	while ((*str)[2] == '@')
	{
		*str = GetStrConst(*str, cond);
		unsigned int len = strlen(cond);
		if (len != 0 && len < 64)
		{
			conds->push_back(std::string(cond));
			cond[0] = 0;
		}
	}
	return true;
}

// 0088e210 (o laço é o std::for_each 0088d9b0 com o lambda 0088d750)
bool FxInterpreter::AttachGroups(int kind, const char* name, const char* key, std::vector<std::string>& conds,
	std::vector<unsigned long>& groups, const Attacher* attacher, int type, int slot)
{
	int result = 0;
	std::for_each(groups.begin(), groups.end(), [&](const unsigned long& group)
	{
		if (kind == 0)
			result = Attach(name, attacher, type, false, group, slot, true);
		else if (kind == 1)
		{
			if (conds.size() == 0)
				result = AttachPending(key, name, NULL, attacher, type, false, group, slot, true, false);
			else
			{
				for (std::vector<std::string>::iterator it = conds.begin(); it != conds.end(); ++it)
				{
					result = AttachPending(key, name, it->c_str(), attacher, type, false, group, slot, true,
						it + 1 != conds.end());
					if (!result)
						break;
				}
			}
		}
		else if (kind == 2)
		{
			for (std::vector<std::string>::iterator it = conds.begin(); it != conds.end(); ++it)
			{
				result = AttachPending(it->c_str(), name, attacher, type, false, group, slot, true,
					it + 1 != conds.end());
				if (!result)
					break;
			}
		}
	});
	return result != 0;
}

// 0088e860
int FxInterpreter::ParseBoundingBoxScript_fx(WPuppet* pet)
{
	bool bAttached = false;
	for (w_bound_box* box = pet->m_bbList.Start(); box; box = pet->m_bbList.Next())
	{
		if (!box->info->option || !box->bone || box->bone->m_flag.GetFlag(WBone::INVISIBLE))
			continue;

		const char* str = box->info->option;
		for (int format = 2, index = 3; FindToken(&str, &format, &index); format = 2, index = 3)
		{
			char key[64];
			char name[64];
			int kind = 0;
			if (ParseKey(&str, key, format, index))
				kind = 1;
			std::vector<std::string> conds;
			if (!ParseName(&str, name, &conds, format, index, kind == 1))
				continue;
			int type = GetFxType(name);
			if (type == eET_NONE)
				continue;
			if (conds.size() != 0 && kind != 1)
				kind = 2;
			Attacher attacher;
			attacher.mat[0] = &box->bone->m_matrix;
			attacher.pos[0] = &box->info->spherePivot;
			std::vector<unsigned long> groups;
			groups.push_back(0);
			bAttached = AttachGroups(kind, name, key, conds, groups, &attacher, type, 0);
		}
	}
	if (bAttached)
		return m_nextId++;
	return 0;
}

// 0088ecd0
int FxInterpreter::ParsePetFrameScript_fx(CPetFrame* pFrame, const float* pos, bool bParts, int slot)
{
	if (!pFrame)
		return 0;
	Attacher unused;
	bool bAttached = false;
	WPuppet* pet = pFrame->GetBonePet();
	for (w_bound_box* box = pet->m_bbList.Start(); box; box = pet->m_bbList.Next())
	{
		if (!box->info->option || !box->bone || box->bone->m_flag.GetFlag(WBone::INVISIBLE))
			continue;

		const char* str = box->info->option;
		for (int format = 2, index = 3; FindToken(&str, &format, &index); format = 2, index = 3)
		{
			std::vector<unsigned long> groups;
			char key[64];
			char name[64];
			char part[64] = "";
			int kind = 0;
			if (ParseKey(&str, key, format, index))
				kind = 1;
			std::vector<std::string> conds;
			if (!ParseName(&str, name, &conds, format, index, kind == 1))
				continue;
			int type = GetFxType(name);
			if (type == eET_NONE)
				continue;
			if (conds.size() != 0 && kind != 1)
				kind = 2;
			if (bParts)
			{
				str = GetStrConst(str, part);
				int len = strlen(part);
				if ((len == 0 || len >= 64) && kind != 1)
					continue;
				bool bAny = false;
				bool bFound = false;
				while (part[0] == '&')
				{
					bAny = true;
					unsigned long tid = atoi(part + 1);
					if (pFrame->GetPartList().FindPart(tid))
					{
						bFound = true;
						groups.push_back(tid);
					}
					if (pFrame->FindAuxPart(tid))
					{
						groups.push_back(tid);
						bFound = true;
						break;
					}
					if (bFound)
						break;
					str = GetStrConst(str, part);
					len = strlen(part);
					if (len == 0 || len >= 64)
						break;
				}
				if (bAny && !bFound)
					continue;
			}
			Attacher attacher;
			attacher.x = pos[0];
			attacher.y = pos[1];
			attacher.mat[0] = &box->bone->m_matrix;
			attacher.pos[0] = &box->info->spherePivot;
			if (groups.size() == 0)
				groups.push_back(0);
			bAttached = AttachGroups(kind, name, key, conds, groups, &attacher, type, slot);
		}
	}
	if (bAttached)
		return m_nextId++;
	return 0;
}


// 0088f2b0
int FxInterpreter::ParseAvatarScript_fx(CPetFrame* pFrame, sClubSet* pClubSet, int slot, bool bShowAll, bool bKeyOnly)
{
	if (!pFrame)
		return 0;
	Attacher attacher;
	bool bAttached = false;
	WPuppet* pet = pFrame->GetBonePet();
	for (w_bound_box* box = pet->m_bbList.Start(); box; box = pet->m_bbList.Next())
	{
		if (!box->info->option || !box->bone || box->bone->m_flag.GetFlag(WBone::INVISIBLE))
			continue;

		const char* str = box->info->option;
		for (int format = 2, index = 3; FindToken(&str, &format, &index); format = 2, index = 3)
		{
			std::vector<unsigned long> groups;
			char key[64];
			char name[64];
			char part[64];
			int kind = 0;
			if (ParseKey(&str, key, format, index))
				kind = 1;
			std::vector<std::string> conds;
			if (!ParseName(&str, name, &conds, format, index, kind == 1))
				continue;
			if (kind != 1 && bKeyOnly)
				continue;
			int type = GetFxType(name);
			if (type == eET_NONE)
				continue;
			if (conds.size() != 0 && kind != 1)
				kind = 2;
			str = GetStrConst(str, part);
			int len = strlen(part);
			if ((len == 0 || len >= 64) && kind != 1)
				continue;
			bool bAny = false;
			bool bFound = false;
			unsigned long tid = 0;
			while (part[0] == '&')
			{
				bAny = true;
				tid = atoi(part + 1);
				if (pFrame->GetPartList().FindPart(tid))
				{
					bFound = true;
					groups.push_back(tid);
				}
				for (int i = 0; i < 5; i++)
				{
					if (Doc()->m_userInfo[slot].charInfo.tidAuxParts[i] == tid)
					{
						bFound = true;
						groups.push_back(tid);
					}
				}
				if (bFound)
					break;
				if (pClubSet && pClubSet->pInfo &&
					(pClubSet->pInfo->ClubIds[GolfClub().GetType()] == tid || pClubSet->pInfo->c.TypeId == tid))
				{
					bFound = true;
					groups.push_back(tid);
					break;
				}
				str = GetStrConst(str, part);
				len = strlen(part);
				if (len == 0 || len >= 64)
					break;
			}
			if (bFound && strstr(name, "G_earing")) // B1418 temporario (parse ja provado: so marca o momento)
				jp983_rastro("B1418 anexa %s tid=%lu slot=%d", name, tid, slot);
			if (!bShowAll && bAny && bFound && HasGroupOrder(slot, tid))
				continue;
			if (bAny && !bFound)
				continue;
			attacher.mat[0] = &box->bone->m_matrix;
			attacher.pos[0] = &box->info->spherePivot;
			if (groups.size() == 0)
				groups.push_back(0);
			bAttached = AttachGroups(kind, name, key, conds, groups, &attacher, type, slot);
		}
	}
	if (bAttached)
		return m_nextId++;
	return 0;
}

// 0088f8f0
int FxInterpreter::ParseClubScript_fx(sClubSet* pClubSet, int slot)
{
	if (!pClubSet)
		return 0;
	Attacher attacher;
	bool bAttached = false;
	WPuppet* pet = pClubSet->pHead[GolfClub().GetType()];
	if (!pet)
		return 0;
	for (w_bound_box* box = pet->m_bbList.Start(); box; box = pet->m_bbList.Next())
	{
		if (!box->info->option || !box->bone || box->bone->m_flag.GetFlag(WBone::INVISIBLE))
			continue;

		const char* str = box->info->option;
		for (int format = 2, index = 3; FindToken(&str, &format, &index); format = 2, index = 3)
		{
			char key[64];
			char name[64];
			int kind = 0;
			if (ParseKey(&str, key, format, index))
				kind = 1;
			std::vector<std::string> conds;
			if (!ParseName(&str, name, &conds, format, index, kind == 1))
				continue;
			int type = GetFxType(name);
			if (type == eET_NONE)
				continue;
			if (conds.size() != 0 && kind != 1)
				kind = 2;
			int len = strlen(name);
			if ((len == 0 || len >= 64) && kind == 0)
				continue;
			attacher.mat[0] = &box->bone->m_matrix;
			attacher.pos[0] = &box->info->spherePivot;
			std::vector<unsigned long> groups;
			groups.push_back(0);
			bAttached = AttachGroups(kind, name, key, conds, groups, &attacher, type, slot);
		}
	}
	if (bAttached)
		return m_nextId++;
	return 0;
}

// 00883b70
bool FxInterpreter::HasGroupOrder(unsigned int slot, unsigned long group)
{
	if (group != 0 && slot < 5)
	{
		for (std::list<unsigned long>::iterator it = m_groupOrder[slot].begin(); it != m_groupOrder[slot].end(); ++it)
		{
			if (*it == group)
				return true;
		}
	}
	return false;
}

// 0088dc00: liga (recria) ou fecha os pendentes de um tid
bool FxInterpreter::ShowPendingByTid(unsigned long tid, unsigned int slot, bool bShow)
{
	if (tid == 0 || slot >= 4)
		return false;
	for (std::list<FxInfoPtr>::iterator it = m_pending.begin(); it != m_pending.end(); ++it)
	{
		if ((*it)->m_tid != tid)
			continue;
		if (bShow)
		{
			if ((*it)->m_bKeep != 1 || (*it)->m_bShown)
				Revive(*it, slot);
		}
		else
		{
			CloseInfo(it->get(), false);
			(*it)->m_pFx = NULL;
		}
	}
	return true;
}

// 0088dc90: idem pela chave; só recria se a condição do efeito for a atual da vaga
void FxInterpreter::ShowPendingByCond(unsigned long tid, bool bShow, unsigned int slot)
{
	if (tid == 0 || slot >= 4)
		return;
	for (std::list<FxInfoPtr>::iterator it = m_pending.begin(); it != m_pending.end(); ++it)
	{
		if ((*it)->m_tid != tid)
			continue;
		if (bShow)
		{
			if ((*it)->m_cond.size() == 0 || m_curCond[slot].compare((*it)->m_cond) == 0)
				Revive(*it, slot);
		}
		else
		{
			CloseInfo(it->get(), false);
			(*it)->m_pFx = NULL;
		}
		(*it)->m_bShown = bShow;
	}
}

// 0088fe40 (std::for_each 0088e300 com o lambda 0088e160)
void FxInterpreter::ShowNamed(const char* key, bool bShow, int slot)
{
	if (!key)
		return;
	int i = 0;
	int last = 4;
	if (slot >= 0 && slot < 4)
	{
		last = slot + 1;
		i = slot;
	}
	for (; i < last; ++i)
	{
		NamedMap::iterator it = m_namedA[i].find(key);
		if (it != m_namedA[i].end())
			std::for_each(it->second.begin(), it->second.end(), [this, &bShow, &i](unsigned long tid)
			{
				ShowPendingByCond(tid, bShow, i);
			});
	}
}

// 0088fff0
bool FxInterpreter::ShowAllNamed(bool bShow, int slot)
{
	int i = 0;
	int last = 4;
	if (slot >= 0 && slot < 4)
	{
		last = slot + 1;
		i = slot;
	}
	for (; i < last; ++i)
	{
		for (NamedMap::iterator it = m_namedA[i].begin(); it != m_namedA[i].end(); ++it)
		{
			for (std::list<unsigned long>::iterator tid = it->second.begin(); tid != it->second.end(); ++tid)
				ShowPendingByCond(*tid, bShow, i);
		}
	}
	return true;
}

// 008900e0: fecha os pendentes só da condição antiga e recria os da nova (std::for_each 0088e3a0 com o lambda 0088e1b0)
void FxInterpreter::SetCond(const char* cond, unsigned int slot)
{
	if (slot >= 4)
		return;
	std::string newCond;
	std::string oldCond;
	if (!cond || *cond == '@')
		newCond = cond;
	else
		newCond = std::string("@") + std::string(cond);
	if (m_curCond[slot].compare(newCond) == 0)
		return;
	oldCond = m_curCond[slot].c_str();
	m_curCond[slot] = newCond.c_str();
	jp983_rastro("B1392 cond %s -> %s slot=%u", oldCond.c_str(), newCond.c_str(), slot); // temporario
	std::list<unsigned long> newTids;
	std::list<unsigned long> oldTids;
	NamedMap::iterator it = m_namedB[slot].find(newCond.c_str());
	if (it != m_namedB[slot].end())
		newTids = it->second;
	it = m_namedB[slot].find(oldCond.c_str());
	if (it != m_namedB[slot].end())
		oldTids = it->second;
	std::for_each(oldTids.begin(), oldTids.end(), [&newTids, this, &slot](unsigned long& tid)
	{
		if (std::find(newTids.begin(), newTids.end(), tid) == newTids.end())
			ShowPendingByTid(tid, slot, false);
	});
	for (std::list<unsigned long>::iterator tid = newTids.begin(); tid != newTids.end(); ++tid)
		ShowPendingByTid(*tid, slot, true);
}

// 008905d0: o modo `mode` liga (bActive) ou desliga o efeito `name`
void FxInterpreter::AddModeRule(const char* name, int mode, bool bActive)
{
	std::map<int, std::vector<sFxInfoBase> >::iterator it = m_modeRules.find(mode);
	if (it != m_modeRules.end())
	{
		std::vector<sFxInfoBase>& rules = it->second;
		std::vector<sFxInfoBase>::iterator rule = std::find(rules.begin(), rules.end(), sFxInfoBase(name, true));
		if (rule != rules.end())
			rule->m_bActive = bActive;
		else
			rules.push_back(sFxInfoBase(name, bActive));
	}
	else
	{
		std::vector<sFxInfoBase> rules;
		rules.push_back(sFxInfoBase(name, bActive));
		m_modeRules.insert(std::make_pair(mode, rules));
	}
}

// 0088e520
FxInterpreter::FxInterpreter(CFxBox* pFxBox, int firstId)
{
	m_nextId = firstId + 1;
	m_pFxBox983 = pFxBox;
	m_158 = 1;
	InitTokens();
	RegisterKeeper(pFxBox);
	if (pFxBox)
		SetFxBox(pFxBox);
	ms_count++;
}

// 0088dee0
FxInterpreter::~FxInterpreter()
{
	ms_count--;
	DetachFxAll(-1);
	ClearInfos();
	ClearPending();
	for (int i = 0; i < 4; i++)
	{
		m_namedA[i].clear();
		m_namedB[i].clear();
	}
	if (m_pFxBox983)
		m_pFxBox983->ClearAll(false);
	else
		CFx::Instance()->ClearAll(ms_count != 0);
	m_playFx.clear();
}

C_ASSERT_VC10(sizeof(FxInterpreter::sFxInfoBase) == 0x24);
C_ASSERT_VC10(sizeof(FxInterpreter::sFxInfo) == 0x88);
C_ASSERT_VC10(sizeof(FxInterpreter) == 0x1e0);
