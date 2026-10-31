// JP 983: unidade dummyplayeractor (004d5410–004dabd2). IDummyPlayer INTEIRA (sessões B32 e B33: construtor 004d78f0,
// destrutor 004d7a30, as 10 virtuais próprias, SetPosition, LookAt983, RefreshFx983, ScriptProcess, SetMotionData983, Play983,
// ChangeTexture983 e os carregadores estáticos Load983 004d7c60 / Build983 004d7fb0; B34: IsName983 e ReadDefault983) e a
// CDummyPlayerActor INTEIRA (pódio do Grand Prix; sessões B33 e B34, no fim do arquivo). Fonte reconstruída do 983 (classes sem par no 645; par no KR 839).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "dummyplayer.h"
#include "golfdocofs983.h" // B1459
#include "lobbyjp.h"
#include "shareddoc.h"
#include "docofs983.h" // D864: crus do CSharedDoc ancorados no membro (VS 2022)
#include "commonutil.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wresrcmng.h"
#include "fxinterpreter.h"
#include "petframe.h"
#include "soundmanager.h"
#include "texturepart.h"
#include "parttidlist.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "fx.h"
#include "ucclibrary.h"
#include "uccmanager.h"
#include <ctype.h>
#include <float.h>
#include <algorithm>

extern const WVector g_vUp983_e42538; // 00e42538 = (0, 1, 0)
extern const WVector g_vZero983_f041c4; // 00f041c4
const char* GetStrConst(const char* src, char* dst);

// WPuppet (engine, frente A): o acerto da matriz 004d18f0 (cópia fora de linha da unidade dummypet)
class WPuppetPlayerJp : public WPuppet
{
public:
	void SetMatrix983(const WMatrix& m); // 004d18f0
};

// CPetFrame (frente A, incompleto): o WPuppet de +4 e 008acdf0 (repassa a 00b86cf0 com o boneco de +0x24)
class CPetFrameJp : public CPetFrame
{
public:
	void Animate983(const char* a, const char* b, float time); // 008acdf0
	bool Build983(const CPartTidList* tids, unsigned long* uids, bool a, bool b, int option, IDummyPlayer* pOwner); // 008b23f0 (6 argumentos no 983)
	void SetPart983(const void* part); // 008b0960
	void CalcBVolume983(); // 008afc50 (CPetFrame::CalcBVolume)
	char* TexName983(); // 008acc30
	CPartList* PartList983() { return &m_partList; } // 983: +0x1f8 (645: +0x1b8); D872: membro da visao (cl 14.44 +0x1ec)
	WPuppetPlayerJp* Puppet04_983() { return *(WPuppetPlayerJp**)((char*)this + 4); }
};

// FxInterpreter do 983 (frente A, incompleto): as duas formas de ligar os efeitos a um CPetFrame e 0088fe40
class FxInterpreterPlayerJp : public FxInterpreter
{
public:
	// as funcoes do 983 ja estao em jp/fxinterpreter.cpp (B): estes nomes so repassam com os argumentos do chamador
	int AttachFrameA983(CPetFrame* pFrame, int a, int b, int c, int d) // 0088f2b0
	{
		return ParseAvatarScript_fx(pFrame, (sClubSet*)a, b, c != 0, d != 0);
	}
	int AttachFrameB983(CPetFrame* pFrame, float* pRange, int a, int b) // 0088ecd0
	{
		return ParsePetFrameScript_fx(pFrame, pRange, a != 0, b);
	}
	void Hide983(const char* name, int a, int b) { ShowNamed(name, a != 0, b); } // 0088fe40
	void Close983(int a, unsigned long tid) { CloseGroup(a, tid); } // 00887c10
	char* FindSpr983(std::string name, const char* cond, Attacher* attacher, unsigned long tid, int a, int b) // 00885570
	{
		return (char*)FindSpray(name, cond, attacher, tid, a, b);
	}
	char* FindSeq983(std::string name, const char* cond, Attacher* attacher, unsigned long tid, int a, int b) // 008856c0
	{
		return (char*)FindSequence(name, cond, attacher, tid, a, b);
	}
	void Attach983(const char* name, const char* cond, Attacher* attacher, int type, int a, unsigned long tid, int b,
		int c) // 0088d3c0 (type: 1 = .seq, 2 = .spr)
	{
		Attach(name, cond, attacher, type, a != 0, tid, b, c != 0);
	}
};

// CPetFrame do 983 tem 0x358 (`push 0x358` em 004d7cb5); o da frente A ainda tem o tamanho do 645: `new (PETFRAME983) CPetFrame`
// aloca o tamanho do 983 sem embrulhar o construtor (o embrulho numa derivada custava dois `mov` a mais)
struct sPetFrameSize983
{
};
inline void* operator new(size_t, const sPetFrameSize983&)
{
	return ::operator new(0x358);
}
inline void operator delete(void* p, const sPetFrameSize983&)
{
	::operator delete(p);
}
static const sPetFrameSize983 PETFRAME983 = sPetFrameSize983();

// CGolfDoc (frente A): montadores da lista de peças de um boneco (golfdoc.cpp do 983; corpos não lidos)
class CGolfDocPlayerJp
{
public:
	void MakePartsFull983(CPartTidList* out, unsigned long charTid, int a, int b, unsigned long* tids, unsigned long* uids); // 00661fb0
	void MakeParts983(CPartTidList* out, unsigned long charTid, int a, int b, unsigned long* tids, unsigned long* uids); // 006620b0
};

// CItemManager (frente A): registro do personagem (rosto em +0x138)
class CItemManagerPlayerJp
{
public:
	char* FindChar983(unsigned long tid); // 00a17120
};

// 00965250 = GetPVS() do 983 (a static local CPVSPool, 2 CQuadTree de 0x2234; 0094af60 escolhe o m_pool[ms_InstanceIdx]).
// 0094f660 e uma CQuadTree: devolve o ponto do chao sob `pos` (GetMeshList 0094d110 + ColTest 0094f3f0 com bAll = 1).
// Os tres floats que o 983 passa depois do WVector sao os do proprio pos (por valor); o `0, 0, 0` antigo era erro de leitura.
class CShadowWorld983 : public CQuadTree
{
public:
	WVector Convert983(WVector pos, bool bKeepPos, int* pIndex, bool bWater)
	{
		if (m_pHolder == NULL)
			return pos;
		GetMeshList(pos);
		int index = -1;
		WVector r = ColTest(pos, bKeepPos, &index, true);
		float y = r.y;
		if (bKeepPos && index == -1)
			y = m_pHolder->aabb.min.y;
		if (pIndex != NULL)
			*pIndex = index;
		if (bWater)
		{
			int n = (int)m_water.size();
			for (int i = 0; i < n; i++)
			{
				const sWater& w = m_water[i];
				if (w.aabb.min.x < r.x && r.x < w.aabb.max.x && w.aabb.min.z < r.z && r.z < w.aabb.max.z &&
					y < w.aabb.max.y)
				{
					y = w.aabb.max.y;
					if (pIndex != NULL)
						*pIndex = -2;
					break;
				}
			}
		}
		if (y < m_pHolder->aabb.min.y)
		{
			y = m_pHolder->aabb.min.y;
			if (pIndex != NULL && *pIndex > -2)
				*pIndex = -1;
		}
		r.y = y;
		return r;
	}
	// 00958bb0 (983: flag da camera em CSharedDoc+0x11024 -> +0x160, depois raio da luz contra os modelos).
	// O CQuadTree::GetShadowIntensity da A ainda usa +0x5040/+0x150 do 645 (null -> AV no lobby, b181-1/b181-2):
	// sem sombra por modelo (1.0) ate a A corrigir os offsets (pedido B181.1); entao trocar por GetShadowIntensity(pos).
	float Bright983(const WVector& pos)
	{
		(void)pos;
		return 1.0f;
	}
};
inline CShadowWorld983* GetShadowWorld983() { return static_cast<CShadowWorld983*>(&GetPVS()); } // 00965250

// IPetScriptObject (frente A): leitor do *ptex( que serve o jogador (o da IDummyPet é 004d34b0)
class IPetScriptObjectJp : public IPetScriptObject
{
public:
	bool ParsePTex983(w_motion_data* motion, std::string* pTexture, const char*& p, int kind, bool bFlag); // 005ace20
	void ScriptShowBone983(CPetFrame* pFrame, int cmd, const char*& p) { ScriptShowBone(pFrame, cmd, p); }
	const char* ScriptPlayIf983(CPetFrame* pFrame, const char*& p) { return ScriptPlayIf(pFrame, p); }
	void ScriptShowVis983(CPetFrame* pFrame, bool bShow, const char*& p) { ScriptShowVis(pFrame, bShow, p); }
};

struct sDeleteTexPart983
{
	void operator()(sTexturePart* p) const
	{
		if (p)
			delete p;
	}
};

IDummyPlayer::IDummyPlayer(int kind)
	: m_pFrame((CPetFrame*)NULL),
	  m_kind(kind),
	  m_pos(g_vZero983_f041c4),
	  m_pMotion(NULL),
	  m_pTexPart(NULL),
	  m_ce(true),
	  m_bInit(false),
	  m_pFx(NULL),
	  m_fxId(0)
{
}

IDummyPlayer::~IDummyPlayer()
{
	std::for_each(m_texList.begin(), m_texList.end(), sDeleteTexPart983());
	if (m_pFx)
	{
		delete m_pFx;
		m_pFx = NULL;
	}
	if (m_pFrame.get())
	{
		m_pFrame.reset();
		m_pFrame = std::tr1::shared_ptr<CPetFrame>((CPetFrame*)NULL);
	}
	m_bInit = false;
}

void IDummyPlayer::OnScriptSnd(const char*& p)
{
	static char sndName[0x40];
	p = GetStrConst(p, sndName);
	if (strlen(sndName) != 0 && strlen(sndName) < 64)
	{
		WVector pos = m_matrix.pivot - g_camera.pivot;
		g_audio->PlaySfx(sndName, &pos, 5, NULL, NULL, 0.5f, 200.0f);
	}
}

void IDummyPlayer::OnScriptHideClub(const char*& p)
{
	CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", false);
}

void IDummyPlayer::SetPosition(const WVector& pos)
{
	m_matrix.Reset();
	m_matrix.ya = g_vUp983_e42538;
	m_matrix.xa = WCrossProduct(m_matrix.ya, m_matrix.za);
	m_matrix.pivot = GetShadowWorld983()->Convert983(pos, false, NULL, false);
	m_pos = pos;
}

void IDummyPlayer::LookAt983(WVector pos)
{
	WVector dir;
	dir.x = pos.x - m_matrix.pivot.x;
	dir.y = 0;
	dir.z = pos.z - m_matrix.pivot.z;
	WVector front = m_matrix.za * -1.0f;
	float angle = CalcDeltaAngle(dir, front);
	float mag = angle;
	if (mag <= 0)
		mag = -mag;
	if (mag > 0.0174532924f)
	{
		WMatrix rot;
		RotMat(angle, 1, rot);
		m_matrix = rot * m_matrix;
	}
}

void IDummyPlayer::ParsePuppetScript(char* script, bool bSkip) // 004d4940 (repassa ao 005ac510)
{
	ParseScript(script, bSkip);
}

void IDummyPlayer::UpdateTexture983()
{
	if (!m_bShow || m_pFrame.get() == NULL)
		return;
	sTexturePart* pPart = m_pTexPart;
	if (pPart == NULL || pPart == m_pTexApplied)
		return;
	m_pTexApplied = pPart;
	if (pPart->pBitmap == NULL)
		m_pFrame->GetBonePet()->ChangeTexturePart(m_pFrame->m_skinTexture, pPart->texture, &pPart->pBitmap, &pPart->rect);
	else
		m_pFrame->GetBonePet()->ChangeTexturePart(m_pFrame->m_skinTexture, pPart->pBitmap, pPart->rect);
	m_pFrame->SetMakeup();
}

void IDummyPlayer::OnScriptShowBone(const char*& p, int cmd)
{
	((IPetScriptObjectJp*)this)->ScriptShowBone983(m_pFrame.get(), cmd, p);
}

void IDummyPlayer::OnScriptShowVis(const char*& p, int cmd)
{
	((IPetScriptObjectJp*)this)->ScriptShowVis983(m_pFrame.get(), cmd == 0xd, p);
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // A680 temporario

void IDummyPlayer::RefreshFx983(bool bAll, bool bFlag, int a, int b)
{
	if (bAll)
		m_pFx->DetachFxAll(-1);
	else
		m_pFx->DetachFx(m_fxId);
	if (bFlag)
	{
		m_fxId = ((FxInterpreterPlayerJp*)m_pFx)->AttachFrameA983(m_pFrame.get(), 0, 0, a, 0);
	}
	else
	{
		float range[2];
		range[0] = FLT_MAX;
		range[1] = range[0];
		m_fxId = ((FxInterpreterPlayerJp*)m_pFx)->AttachFrameB983(m_pFrame.get(), range, 1, 0);
	}
	((FxInterpreterPlayerJp*)m_pFx)->Hide983("\xb4\xeb\xc3\xb5\xbb\xe7", 1, 0);
	((FxInterpreterPlayerJp*)m_pFx)->Hide983("\xbf\xe4\xc1\xa4\xb3\xaf\xb0\xb3", 1, 0);
	// A680 (dono9 d, brinco "as vezes" no lobby): fx novo, caminho e se o frame tem o brinco/anel de teste (White Feather / Crimson Ring)
	CPartList& pl = m_pFrame->m_partList;
	jp983_rastro("A680 fx id=%d all=%d flag=%d np=%d brinco=%d anel=%d", m_fxId, (int)bAll, (int)bFlag, (int)((unsigned char*)&pl)[5],
		(int)(pl.FindPart(0x081a4801) || pl.FindPart(0x081a4800)), (int)m_pFrame->FindAuxPart(0x70010080));
}

void IDummyPlayer::ScriptProcess(w_motion_data* motion, float from, float to, bool bAll, bool bSkip)
{
	if (motion == NULL)
		return;
	if ((!(from <= motion->e) || from == to) && !bAll)
		return;

	int start, end;
	from *= 30.0f;
	if (bAll)
	{
		start = Ceil(from, 0.03f);
		end = start;
	}
	else
	{
		float limit = motion->e;
		if (limit > to)
			limit = to;
		to = limit * 30.0f;
		start = Ceil(from, 0.03f);
		end = Wabs((float)floor(to) - to) < g_EPSILON ? (int)to - 1 : (int)floor(to);

		if (start == (int)floor(motion->s * 30.0f + 0.5f))
			start++;

		if (start > end)
			return;
	}

	if (m_pFrame->GetCurAniPet()->CheckFrameData(start, end, (int&)from, (int&)to))
	{
		for (int i = (int&)from; i <= (int&)to; i++)
			ParsePuppetScript(m_pFrame->GetCurAniPet()->GetFrameData(i), bSkip);
	}
}

bool IDummyPlayer::SetMotionData983(const char* name)
{
	m_pMotion = m_pFrame->FindMotionData(name, true, NULL);
	if (m_pMotion == NULL)
	{
		m_bPlaying = false;
		return false;
	}
	m_motionTime = Min(m_pMotion->s, m_pMotion->e);
	ScriptProcess(m_pMotion, m_pMotion->s, m_pMotion->s, true, false);
	return true;
}

void IDummyPlayer::Play983(const char* name)
{
	m_60 = 3.14159274f;
	m_bShow = true;
	m_bPlaying = false;
	m_motionTime = 0;
	SetMotionData983(name);
	RefreshFx983(true, m_ce, 0, 1);
}

bool IDummyPlayer::SetMotion(const char* name, bool bFlag)
{
	if (name == NULL)
		return false;
	m_60 = 3.14159274f;
	m_light.type = 2;
	m_light.ambient = ModifyColor(g_lightset.ambient, 1.0f);
	m_light.nearOne = g_lightset.nearOne;
	m_motionTime = 0;
	m_matrix.Reset();
	m_pTexPart = NULL;
	m_pTexApplied = NULL;
	m_bShow = false;
	m_bPlaying = false;
	m_ce = bFlag;
	if (m_pFx == NULL)
		m_pFx = new FxInterpreter(NULL, 0);
	SetMotionData983(name);
	RefreshFx983(false, bFlag, 0, 1);
	m_bInit = true;
	return true;
}

void IDummyPlayer::Process(float dt)
{
	if (!m_bPlaying || m_pMotion == NULL || !m_bShow)
		return;
	m_motionTime += dt;
	ScriptProcess(m_pMotion, m_motionTime - dt, m_motionTime, false, false);
	if (m_motionTime > m_pMotion->e)
	{
		if (m_pMotion->s != m_pMotion->e)
			SetMotionData983(m_pMotion->nextmotion);
		else
			m_bPlaying = false;
	}
	else
	{
		((CPetFrameJp*)m_pFrame.get())->Puppet04_983()->SetMatrix983(m_matrix);
		((CPetFrameJp*)m_pFrame.get())->Animate983(NULL, NULL, m_motionTime);
		m_pFrame->GetBonePet()->UpdateBound(true, true);
	}
	if (g_view->InFrustum(m_pFrame->GetBonePet()->GetBoundSphere()))
	{
		WBone* pHead = m_pFrame->GetBonePet()->FindBoneByTail("Head");
		m_light.diffuse = ModifyColor(g_lightset.diffuse, GetShadowWorld983()->Bright983(*(WVector*)((char*)pHead + 0x164)));
		m_pFrame->UpdateLightSource(&m_light, false, NULL);
	}
}

void IDummyPlayer::OnScriptPlayIf(const char*& p)
{
	const char* name = ((IPetScriptObjectJp*)this)->ScriptPlayIf983(m_pFrame.get(), p);
	if (name)
		SetMotionData983(name);
}

void IDummyPlayer::ChangeTexture983(const char* texture)
{
	std::list<sTexturePart*>::iterator it = std::find_if(m_texList.begin(), m_texList.end(), sFindTexPart(texture));
	if (it != m_texList.end())
	{
		if (*it != m_pTexPart)
			m_pTexPart = *it;
	}
	else
	{
		sTexturePart* pTexPart = new sTexturePart;
		strcpy(pTexPart->texture, texture);
		m_texList.push_back(pTexPart);
		m_pTexPart = pTexPart;
	}
}

void IDummyPlayer::OnScriptPTex(const char*& p)
{
	std::string texture;
	if (((IPetScriptObjectJp*)this)->ParsePTex983(m_pMotion, &texture, p, m_kind, m_e0))
		ChangeTexture983(texture.c_str());
}

// 004d8580: *fx( do roteiro do movimento — forma do CGolfPlayer::OnScriptFx do 645, com o FxInterpreter do boneco no lugar do
// CFx global: acha o efeito (.spr 00885570 / o resto 008856c0) e só liga ou desliga (cmd == 7); se não existe, cria (0088d3c0).
void IDummyPlayer::OnScriptFx(const char*& p, bool bSkip, int cmd)
{
	if (bSkip)
		return;
	std::vector<unsigned long> ids;
	char cond[64];
	char fxName[64];
	char boneName[64];
	char name[64];
	cond[0] = 0;
	p = GetStrConst(p, cond);
	if (strlen(cond) - 1 > 62)
		return;
	if (cond[0] == '@')
	{
		if (m_pMotion == NULL || strcmp(cond + 1, m_pMotion->name) != 0)
			return;
		p = GetStrConst(p, fxName);
		if ((int)strlen(fxName) < 1 || (int)strlen(fxName) >= 64)
			return;
	}
	else
	{
		strcpy(fxName, cond);
		cond[0] = 0;
	}
	p = GetStrConst(p, boneName);
	int boneLen = (int)strlen(boneName);
	if (boneLen == 0 || boneLen >= 64)
		return;
	bool bCond = false;
	if (boneName[0] == '&')
	{
		unsigned long tid;
		do
		{
			tid = atoi(boneName + 1);
			if (!bCond && ((CPetFrameJp*)m_pFrame.get())->PartList983()->FindPart(tid))
			{
				bCond = true;
				ids.push_back(tid);
			}
			p = GetStrConst(p, boneName);
			boneLen = (int)strlen(boneName);
		} while (boneLen != 0 && boneLen < 64 && boneName[0] == '&');
		if (!bCond)
			return;
		((FxInterpreterPlayerJp*)m_pFx)->Close983(0, tid);
		ids.push_back(tid);
	}
	strcpy(name, boneName);
	if (strlen(name) == 0 || strlen(name) >= 64)
		return;
	Attacher attacher;
	unsigned int j;
	for (j = 0; j < 4; j++)
	{
		if (j != 0)
		{
			p = GetStrConst(p, name);
			if (strlen(name) == 0 || strlen(name) >= 64)
				break;
		}
		WBone* pBone = m_pFrame->GetBonePet()->FindBone(name);
		if (pBone)
		{
			attacher.mat[j] = &pBone->m_matrix;
			continue;
		}
		w_bound_box* pBox = m_pFrame->GetBonePet()->m_bbList.Find(name);
		if (pBox)
		{
			if (pBox->bone)
				attacher.mat[j] = &pBox->bone->m_matrix;
			attacher.pos[j] = &pBox->info->spherePivot;
		}
	}
	if ((int)j <= 0)
		return;
	if (strcmp("i_tak.seq", fxName) == 0)
		return;
	char* ext = strrchr(fxName, '.');
	if (ext == NULL || strlen(ext) < 4)
		return;
	if (stricmp(ext + 1, "spr") == 0)
	{
		if (ids.size() > 0)
		{
			for (std::vector<unsigned long>::iterator it = ids.begin(); it != ids.end(); ++it)
			{
				char* pSpr = ((FxInterpreterPlayerJp*)m_pFx)->FindSpr983(fxName, cond, &attacher, *it, 0, 0);
				if (pSpr)
					*(bool*)(pSpr + VC10_FXSPRAY_ACTIVE) = cmd == 7; // D872: CFxSpray::m_bActive
				else
					((FxInterpreterPlayerJp*)m_pFx)->Attach983(fxName, cond, &attacher, 2, 1, *it, 0, 0);
			}
		}
		else
		{
			char* pSpr = ((FxInterpreterPlayerJp*)m_pFx)->FindSpr983(fxName, cond, &attacher, 0, 0, 0);
			if (pSpr)
			{
				*(bool*)(pSpr + VC10_FXSPRAY_ACTIVE) = cmd == 7; // D872: CFxSpray::m_bActive
				return;
			}
			((FxInterpreterPlayerJp*)m_pFx)->Attach983(fxName, cond, &attacher, 2, 1, 0, 0, 0);
		}
	}
	else
	{
		if (ids.size() > 0)
		{
			for (std::vector<unsigned long>::iterator it = ids.begin(); it != ids.end(); ++it)
			{
				char* pSeq = ((FxInterpreterPlayerJp*)m_pFx)->FindSeq983(fxName, cond, &attacher, *it, 0, 0);
				if (pSeq)
				{
					if (*(bool*)(pSeq + VC10_FXSEQ_ACTIVE) != (cmd == 7)) // A753: CFxSequence::m_bActive
						*(bool*)(pSeq + VC10_FXSEQ_ACTIVE) = cmd == 7;
				}
				else
					((FxInterpreterPlayerJp*)m_pFx)->Attach983(fxName, cond, &attacher, 1, 1, *it, 0, 0);
			}
		}
		else
		{
			char* pSeq = ((FxInterpreterPlayerJp*)m_pFx)->FindSeq983(fxName, cond, &attacher, 0, 0, 0);
			if (pSeq)
			{
				if (*(bool*)(pSeq + VC10_FXSEQ_ACTIVE) != (cmd == 7)) // A753: CFxSequence::m_bActive
					*(bool*)(pSeq + VC10_FXSEQ_ACTIVE) = cmd == 7;
				return;
			}
			((FxInterpreterPlayerJp*)m_pFx)->Attach983(fxName, cond, &attacher, 1, 1, 0, 0, 0);
		}
	}
}

// 004d7c60 (estática, /Gr: boneco em ecx, tids em edx): cria o CPetFrame se faltar, monta a lista de peças pelo CGolfDoc
// (bFull: 00661fb0, senão 006620b0), constrói o boneco, põe na cena, aplica as roupas UCC e a textura de rosto "_1".
bool IDummyPlayer::Load983(IDummyPlayer* pPlayer, unsigned long* tids, unsigned long* buildUids, unsigned long* uids, int a, int b,
	bool bNoScene, int bFull)
{
	if (pPlayer->m_pFrame.get() == NULL)
		pPlayer->m_pFrame = std::tr1::shared_ptr<CPetFrame>(new (PETFRAME983) CPetFrame);
	CPartTidList parts;
	if (bFull)
	{
		((CGolfDocPlayerJp*)GOLFDOC())->MakePartsFull983(&parts, pPlayer->m_charTid | 0x4000000, a, b, tids, uids);
		pPlayer->m_e0 = true;
	}
	else
	{
		((CGolfDocPlayerJp*)GOLFDOC())->MakeParts983(&parts, pPlayer->m_charTid | 0x4000000, a, b, tids, uids);
		pPlayer->m_e0 = false;
	}
	((CPetFrameJp*)pPlayer->m_pFrame.get())->Build983(&parts, buildUids, true, true, -1, pPlayer);
	((CPetFrameJp*)pPlayer->m_pFrame.get())->CalcBVolume983();
	if (!bNoScene)
	{
		CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, pPlayer->m_name.c_str(), 0x18, 0);
		CSceneManagerJp::Instance983()->SetPuppet(pPlayer->m_name.c_str(), pPlayer->m_pFrame->GetBonePet());
	}
	if (uids)
	{
		for (int i = 0; i < 24; i++)
		{
			if (uids[i] != 0 && uids[i] != (unsigned long)-1 && IsUccClothes(tids[i]))
			{
				sUccClothes* pClothes = UccManager()->FindClothes(uids[i]);
				if (pClothes)
					((CPetFrameJp*)pPlayer->m_pFrame.get())->SetPart983(pClothes);
			}
		}
	}
	if (parts.m_tid[1] && (parts.m_tid[1] & 0x1800) == 0x1000)
	{
		char* pChar = ((CItemManagerPlayerJp*)&Doc()->m_itemManager)->FindChar983(pPlayer->m_charTid | 0x4000000);
		if (pChar)
		{
			char* faceTex = pChar + 0x138;
			char szFace[256];
			strcpy(szFace, faceTex);
			sprintf(strrchr(szFace, '.'), "_%d%s", 1, strrchr(faceTex, '.'));
			int hTex = g_resrcmng->LoadTexture(faceTex, 0, 0, szFace);
			w_texlist* pTex = g_resrcmng->FindTexture(faceTex);
			WPuppet* pPet = pPlayer->m_pFrame->GetBonePet();
			if (pTex && pPet)
			{
				pPet->ChangeTextureByHandle(hTex, pTex->texhandle, 0);
				if (pPlayer->m_pFrame.get())
					pPet->ChangeTexturePart(hTex, ((CPetFrameJp*)pPlayer->m_pFrame.get())->TexName983(), NULL, NULL);
			}
		}
	}
	return true;
}

// 004d7fb0 (estática; lista de peças pronta em edx): cria o CPetFrame se faltar, constrói e põe na cena
bool IDummyPlayer::Build983(IDummyPlayer* pPlayer, const CPartTidList* tids, unsigned long* uids, bool bNoScene)
{
	if (pPlayer->m_pFrame.get() == NULL)
		pPlayer->m_pFrame = std::tr1::shared_ptr<CPetFrame>(new (PETFRAME983) CPetFrame);
	((CPetFrameJp*)pPlayer->m_pFrame.get())->Build983(tids, uids, true, true, -1, pPlayer);
	((CPetFrameJp*)pPlayer->m_pFrame.get())->CalcBVolume983();
	if (!bNoScene)
	{
		CSceneManagerJp::Instance983()->AddElement983(4, 0, 0, pPlayer->m_name.c_str(), 0x18, 0);
		CSceneManagerJp::Instance983()->SetPuppet(pPlayer->m_name.c_str(), pPlayer->m_pFrame->GetBonePet());
	}
	return true;
}

// layout medido no 983: `new` de 0xe4 em 00567a61; construtor 004d78f0
struct sDummyPlayerLayout983
{
	C_ASSERT_VC10(sizeof(IDummyPlayer) == 0xe4);
	C_ASSERT(offsetof(IDummyPlayer, m_name) == 0x14);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_matrix) == 0x30);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_light) == 0x64);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_pos) == 0x80);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_pMotion) == 0xb0);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_texList) == 0xb8);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_pTexPart) == 0xc4);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_pFx) == 0xd0);
	C_ASSERT_VC10(offsetof(IDummyPlayer, m_e0) == 0xe0);
};

// ---------------------------------------------------------------------------------------------------------------------
// CDummyPlayerActor, 1ª PARTE (sessão B33): construtor 004d9760, destrutor 004d9890, criador 004d9a70, registro 004d9ae0,
// OnAnotherHand 004d54c0, OnDestroy 004d5410, OnProcess 004d6960, OnDisplay 004d6a70, OnLoad 004d9fa0, LoadCrowdProc983
// 004daaa0 e OnPreLoadInit 004dab10. A 2ª PARTE (sessão B34, mais abaixo) fecha a classe.
#include "dummyplayeractor.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"

// 00e74d34 = CHeadIcon (headicon.cpp da frente A; singleton WSingleton<CHeadIcon>): SetPlayerPos 006e4990, SetLevel 006e4aa0.
// O CHeadIcon do 983 tem campos que o headicon.h (layout do 645) não tem; visão de empréstimo só para eles (pedido B75 à A):
// +0x2c = nº de jogadores do SetLevel/GetPlayerNum 006e4a80 (-1 = o do CGolfDoc; 4 se o jogador local está no pódio, senão 3)
// e +0xf4/+0xf8 (zerados pelo construtor do ator; -30 no pódio).
#include "headicon.h"
// B1536: a visao crua CHeadIconJp (+0x2c/+0xf4/+0xf8 do VC10) saiu; no cl 14.44 o WSingleton encolhe 4 e +0xf8 = m_iconMap
// (head zerado/-30 -> AV em _Find_lower_bound no icone de item); usar os inline do headicon.h.

// estado do jogo: dword +0x158 do CGolfDoc do 983 (campo da frente A ainda sem nome)
#define GOLFSTATE983() (*(unsigned int*)GDOCPTR983(GOLFDOC(), 0x158))

IMPLEMENT_ACTOR(CDummyPlayerActor, IActor)

CDummyPlayerActor::CDummyPlayerActor()
	: m_pScript(NULL), m_1d8(-1), m_readyKey(-1)
{
	AddSkipList(0xe0);
	for (int i = 0; i < 4; ++i)
		m_pFrames[i] = NULL;
	if (CHeadIcon::Instance())
	{
		CHeadIcon::Instance()->SetPodiumOffset983(0, 0);
	}
}

CDummyPlayerActor::~CDummyPlayerActor()
{
	for (int i = 0; i < 4; ++i)
		m_pFrames[i] = NULL;
}

void CDummyPlayerActor::OnAnotherHand(int key, void* param)
{
	m_loads[key].m_fn();
	m_loads[key].m_bStop = true;
}

void CDummyPlayerActor::OnDestroy()
{
	for (int i = 0; i < 2; ++i)
	{
		m_loads[i].m_bStop = true;
		if (m_hand.IsWorking(i))
			m_hand.WaitUntilWorkDone((unsigned long)-1);
	}
	g_resrcmng->UnlockAllByThread(m_hand.GetThreadID());
	if (m_pScript)
	{
		lua_system::lua_script_manger::GetInstance()->ReleaseScript(m_pScript);
		m_pScript = NULL;
	}
}

// 004d6960: entrega o resultado do trabalho esperado (mensagem 0x33e a si mesmo), leva a posição da cabeça de cada lugar ao
// painel dos nomes e anda os bonecos
void CDummyPlayerActor::OnProcess(float delta)
{
	for (int i = 0; i < 2; ++i)
	{
		if (m_readyKey == i && !(m_loads[i].m_mask & GOLFSTATE983()) && m_loads[i].m_bReady == true)
		{
			m_readyKey = -1;
			m_loads[i].m_bReady = false;
			HandleMsg(MsgObject(NULL, 0x33e, 0, 0, 0, 0, 0));
			break;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (CHeadIcon::Instance() && m_pFrames[i])
		{
			WVector pos = m_pFrames[i]->GetBonePet()->FindBone("Bip01 Head")->m_matrix.pivot;
			CHeadIcon::Instance()->SetPlayerPos(i, pos);
		}
	}
	for (std::vector<std::tr1::shared_ptr<IDummyPlayer> >::iterator it = m_players.begin(); it != m_players.end(); ++it)
		(*it)->Process(delta);
}

void CDummyPlayerActor::OnDisplay()
{
	for (std::vector<std::tr1::shared_ptr<IDummyPlayer> >::iterator it = m_players.begin(); it != m_players.end(); ++it)
		(*it)->UpdateTexture983();
}

// 004d9fa0: só nos modos 0x14 e 0x15 (CSharedDoc+0x109b8): lê a torcida do roteiro e carrega cada boneco
void CDummyPlayerActor::OnLoad()
{
	switch (*(unsigned char*)DOCPTR983(0x109b8))
	{
	case 0x14:
	case 0x15:
		{
			ReadCrowd983();
			std::vector<sDummyActorCrowd983>::iterator end = m_crowd.end();
			for (std::vector<sDummyActorCrowd983>::iterator it = m_crowd.begin(); it != end; ++it)
				LoadCrowd983(MakeStr("AWARD_CROWD_%d", it->id), it->value);
		}
		break;
	}
}

// 004daaa0: trabalho 1 (outra linha): a torcida, um boneco por vez, até o OnDestroy mandar parar
bool CDummyPlayerActor::LoadCrowdProc983()
{
	std::vector<sDummyActorCrowd983>::iterator end = m_crowd.end();
	for (std::vector<sDummyActorCrowd983>::iterator it = m_crowd.begin(); it != end; ++it)
	{
		LoadCrowd983(MakeStr("AWARD_CROWD_%d", it->id), it->value);
		Sleep(15);
		if (m_loads[1].m_bStop == true)
			return false;
	}
	return true;
}

void CDummyPlayerActor::OnPreLoadInit()
{
	m_pScript = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (m_pScript)
		m_pScript->RunFile("grandprix_ceremony.lua");
	m_loads[0].m_mask = 0x1000;
	_util::WMemFun983<CDummyPlayerActor, bool> fun;
	m_loads[0].m_fn = *_util::InitMemFun983(&fun, this, &CDummyPlayerActor::LoadPlayersProc983);
	m_loads[0].m_bStop = false;
	m_loads[0].m_bReady = false;
	m_loads[1].m_mask = 0;
	m_loads[1].m_fn = *_util::InitMemFun983(&fun, this, &CDummyPlayerActor::LoadCrowdProc983);
	m_loads[1].m_bStop = false;
	m_loads[1].m_bReady = false;
}

// ---------------------------------------------------------------------------------------------------------------------
// CDummyPlayerActor, 2ª PARTE (sessão B34): HandleMsg 004d9ff0, RequestReady983 004d4910, ReadCrowd983 004d93a0,
// LoadPlayer983 004d9ba0, LoadCrowd983 004d9e20, LoadPlayersProc983 004da9c0; e, da IDummyPlayer, IsName983 004d7230 e
// ReadDefault983 004d80f0.
#include "polysoup.h"
#include "wview.h"
#include "contentsdoc.h"
#include "icontentsdoc.h"
#include "../../shared/LuaSystem/luatablehelper.h"

// leitura por índice do lua_table_helper do 983 (004d5ba0 texto, 004d5250 tabela filha; a da frente A só lê por nome)
struct LuaIdx983 : lua_system::lua_table_helper
{
	void Push983(int index);
	template <typename T>
	T GetByIndex(int index);
	lua_system::lua_table_helper GetTableByIndex(int index); // 004d5250
};
inline void LuaIdx983::Push983(int index)
{
	if (table.m_obj)
	{
		lua_pushnumber(state, (double)index);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) != LUA_TNIL)
		{
			lua_remove(state, -1);
			lua_rawgeti(state, table.m_obj->m_index, index);
			return;
		}
		lua_remove(state, -1);
	}
	lua_pushnil(state);
}
template <typename T>
T LuaIdx983::GetByIndex(int index)
{
	Push983(index);
	return lua_tinker::pop<T>(state);
}
lua_system::lua_table_helper LuaIdx983::GetTableByIndex(int index)
{
	Push983(index);
	return lua_system::lua_table_helper(lua_tinker::pop<lua_tinker::table>(state));
}
#define LUAIDX983(tbl) (*(LuaIdx983*)&(tbl))

// registro de 00a17cb0 (CItemManager do 983: achado pelo número; adversário pronto do Grand Prix — nome em +8, personagem em
// +0x30 e 7 peças em +0x38). Classe e nomes NÃO identificados.
struct sAwardNpc983
{
	char m_00[8];
	char name[0x28]; // +0x08
	unsigned long charIndex; // +0x30
	int m_34;
	unsigned long parts[7]; // +0x38
};
class CItemManagerAwardJp
{
public:
	sAwardNpc983* FindNpc983(unsigned long id); // 00a17cb0
};
#define ITEMMGRAWARDJP() ((CItemManagerAwardJp*)&Doc()->m_itemManager)

// A824: documento 0xaf = o real (_grandprix::CGrandPrixDocument). O espelho antigo (char[0x40] + vector) era o layout do VC10:
// no cl 14.44 os 3 vector e o map antes de m_awards encolhem e o vector real fica antes de +0x40 (premiados lidos de lixo no exe VS).
#include "grandprixdocument.h"
typedef sGPAward983 sGrandPrixAward983;
typedef _gateway::_grandprix::CGrandPrixDocument CGrandPrixDocumentAwardJp;

// sorteio de 00860980/008613d0 (classe NÃO identificada; 0x1c com um vector em +0xc)
class CAwardPick983
{
public:
	CAwardPick983(int a, int b, int c, int d); // 00860980
	~CAwardPick983() { }
	bool Get983(std::vector<int>* out); // 008613d0
	int m_lo, m_hi, m_tries, m_count; // +0 .. +0xc
	int m_zero[3]; // +0x10 (zerados pelo ctor; no 983 um vector de 12 bytes sem uso)
};

CAwardPick983::CAwardPick983(int a, int b, int c, int d)
	: m_lo(a), m_hi(b), m_tries(c), m_count(d)
{
	m_zero[0] = 0;
	m_zero[1] = 0;
	m_zero[2] = 0;
}

// 008613d0: lista de m_count valores -1; a cada rodada troca os -1 por rand()%(hi-lo)+lo, ordena e tira repetidos;
// completa com -1 ate m_count; se em m_tries rodadas nao fechar, falha
bool CAwardPick983::Get983(std::vector<int>* out)
{
	std::list<int> l(m_count, -1);
	int i = 0;
	for (; i < m_tries; )
	{
		for (std::list<int>::iterator it = l.begin(); it != l.end(); ++it)
			if (*it == -1)
				*it = rand() % (m_hi - m_lo) + m_lo;
		l.sort();
		l.unique();
		if ((unsigned int)m_count <= l.size())
			break;
		l.insert(l.end(), m_count - l.size(), -1);
		++i;
	}
	if (m_tries <= i)
		return false;
	out->assign(l.begin(), l.end());
	return true;
}

class WViewAwardJp : public WView
{
public:
	void SetFov983(float fov); // 00b4fa90
};
class CCameraWorkAwardJp
{
public:
	float Play983(int a, const char* name); // 006d6be0 (CCameraWork::LoadCameraMotion983: devolve float, o chamador desempilha)
};
extern WView* g_pMainView983; // 00f02458
extern CCameraWorkAwardJp* g_pCameraWork983_e74d30; // 00e74d30
extern WMatrix g_camera; // 00f024a0 (983: g_camera, era stub zerado)
#define AWARDSOUP983() (*(WPolySoup**)((char*)GOLFDOC() + 8))
// 983: o nome do emblema fica 1 byte antes do `szEmblemName` do fonte da frente A (sGuild tem 20, não 21): pedido B34
#define EMBLEM983(info) ((char*)&(info) + 0x40)

struct sDummyKindIs983
{
	sDummyKindIs983(int k) : kind(k) { }
	bool operator()(const std::tr1::shared_ptr<IDummyPlayer>& p) const { return p->m_kind == kind; }
	int kind;
};
struct sDummyNameIs983
{
	sDummyNameIs983(const char* n) : name(n) { }
	bool operator()(const std::tr1::shared_ptr<IDummyPlayer>& p) const { return p->IsName983(name); }
	const char* name;
};

bool IDummyPlayer::IsName983(const char* name)
{
	return m_name == name;
}

// 004d80f0: lê a lista "essentialPosition" do personagem (o 983 monta o vector e não usa), zera as saídas, põe o conjunto
// padrão do personagem e, se ele tiver alguma peça no mapa de peças, monta a lista completa (00661fb0). false = sem peças.
bool IDummyPlayer::ReadDefault983(IDummyPlayer* pPlayer, lua_system::lua_script* pScript, unsigned long* tids, unsigned long* uids)
{
	unsigned long index = pPlayer->m_charTid;
	bool bOwn = false;
	if (pScript == NULL)
	{
		pScript = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
		if (pScript)
			pScript->RunFile("grandprix_ceremony.lua");
		bOwn = true;
	}
	std::vector<int> essential;
	{
		lua_system::lua_table_helper tbl = pScript->GetTable("essentialPosition");
		lua_system::lua_table_helper sub0 = LUAIDX983(tbl).GetTableByIndex(index + 1);
		int count = lua_tinker::call<int, lua_tinker::table>(sub0.state, "get_array_size", sub0.table);
		for (int i = 1; i <= count; ++i)
		{
			lua_system::lua_table_helper sub = LUAIDX983(tbl).GetTableByIndex(index + 1);
			essential.push_back(LUAIDX983(sub).GetByIndex<int>(i));
		}
	}
	memset(tids, 0, 0x60);
	uids[0] = 0;
	uids[1] = 0;
	uids[2] = 0;
	uids[3] = 0;
	uids[4] = 0;
	Doc()->m_itemManager.GetDefCombo(pPlayer->m_charTid | 0x4000000, tids);
	PartIterator first = Doc()->m_itemManager.m_PartMap.lower_bound((index | 0x200) << 18);
	PartIterator last = Doc()->m_itemManager.m_PartMap.upper_bound((index << 18) | 0x802e0ff);
	if (first == last)
		return false;
	CPartTidList parts;
	((CGolfDocPlayerJp*)GOLFDOC())->MakePartsFull983(&parts, pPlayer->m_charTid | 0x4000000, 0, 0, tids, NULL);
	if (bOwn && pScript)
		lua_system::lua_script_manger::GetInstance()->ReleaseScript(pScript);
	return true;
}

void CDummyPlayerActor::RequestReady983(unsigned int key)
{
	if (key > 1)
		return;
	m_readyKey = key;
	for (unsigned int i = 0; i < 2; ++i)
	{
		if (i == key)
		{
			m_loads[i].m_bReady = true;
			return;
		}
	}
}

// 004d93a0: os 4 personagens da torcida — sorteados (o 13 vira 2) ou, se o sorteio falhar, 0 a 3
void CDummyPlayerActor::ReadCrowd983()
{
	m_crowd.clear();
	std::vector<int> picked;
	CAwardPick983 pick(0, 15, 100, 4);
	if (pick.Get983(&picked) == true && picked.size() >= 4)
	{
		for (unsigned int i = 0; i < 4;)
		{
			if (picked[i] == 13)
				picked[i] = 2;
			sDummyActorCrowd983 crowd;
			crowd.value = picked[i];
			++i;
			crowd.id = i;
			m_crowd.push_back(crowd);
		}
	}
	else
	{
		for (int i = 0; i < 4;)
		{
			sDummyActorCrowd983 crowd;
			crowd.value = i;
			++i;
			crowd.id = i;
			m_crowd.push_back(crowd);
		}
	}
}

// 004d9ba0: cria o boneco de um lugar do pódio. uid -1 sem jogador = personagem sorteado com as peças padrão; adversário
// pronto = personagem e 7 peças da tabela 00a17cb0; jogador da sala = as peças que vieram no registro. O movimento sai de
// awardmotionSetting[personagem + 1][lugar]; fora dos lugares 1 a 4 o boneco é apagado.
void CDummyPlayerActor::LoadPlayer983(unsigned long uid, unsigned char place, sDummyActorArgs983* args, int charIndex, int bUser)
{
	IDummyPlayer* pPlayer = new IDummyPlayer(0);
	if (uid == (unsigned long)-1)
	{
		if (bUser == 0)
		{
			pPlayer->m_charTid = rand() % 15;
			IDummyPlayer::ReadDefault983(pPlayer, m_pScript, args->tids, args->buildUids);
			goto named;
		}
	}
	else if (bUser == 0)
	{
		sAwardNpc983* pNpc = ITEMMGRAWARDJP()->FindNpc983(uid);
		if (pNpc)
		{
			Doc()->m_itemManager.GetDefCombo(pNpc->charIndex | 0x4000000, args->tids);
			pPlayer->m_charTid = pNpc->charIndex;
			for (int i = 0; i < 7; ++i)
			{
				unsigned long part = pNpc->parts[i];
				unsigned long pos = (part >> 13) & 0x1f;
				if (pos <= 0x18 && part)
					args->tids[pos] = part;
			}
		}
		else
		{
			IDummyPlayer::ReadDefault983(pPlayer, m_pScript, args->tids, args->buildUids);
		}
		goto named;
	}
	pPlayer->m_charTid = charIndex;
named:
	pPlayer->m_name = MakeStr("AWARD_PLAYER_%d_%d", place, pPlayer->m_charTid);
	IDummyPlayer::Load983(pPlayer, args->tids, args->buildUids, args->uids, args->a, args->b, false, bUser);
	lua_system::lua_table_helper tbl = m_pScript->GetTable("awardmotionSetting");
	if ((unsigned char)(place - 1) <= 3)
	{
		const char* motion = LUAIDX983(LUAIDX983(tbl).GetTableByIndex(pPlayer->m_charTid + 1)).GetByIndex<const char*>(place);
		if (pPlayer->SetMotion(motion, true))
			m_players.push_back(std::tr1::shared_ptr<IDummyPlayer>(pPlayer));
	}
	else
	{
		delete pPlayer;
	}
}

// 004d9e20: um boneco da torcida (tipo 1), com as peças padrão do personagem e o movimento "그랑프리박수" (aplauso)
void CDummyPlayerActor::LoadCrowd983(const char* name, int value)
{
	IDummyPlayer* pPlayer = new IDummyPlayer(1);
	pPlayer->m_charTid = value;
	pPlayer->m_name = name;
	unsigned long tids[24] = { 0 };
	unsigned long uids[5] = { 0, 0, 0, 0, 0 };
	IDummyPlayer::ReadDefault983(pPlayer, m_pScript, tids, uids);
	IDummyPlayer::Load983(pPlayer, tids, uids, NULL, 0, 0, true, 0);
	if (pPlayer->SetMotion("\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae\xb9\xda\xbc\xf6", true))
		m_players.push_back(std::tr1::shared_ptr<IDummyPlayer>(pPlayer));
}

// 004da9c0: trabalho 0 (outra linha): um boneco por registro de premiado do documento do Grand Prix
bool CDummyPlayerActor::LoadPlayersProc983()
{
	CGrandPrixDocumentAwardJp* pDoc;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);
	if (pDoc == NULL)
		return false;
	for (std::vector<sGrandPrixAward983>::iterator it = pDoc->m_awards.begin(); it != pDoc->m_awards.end(); ++it)
	{
		int charIndex = 15;
		for (int i = 0; i < 24; ++i)
		{
			if (it->bUser == 0)
				break;
			if (it->tids[i])
			{
				charIndex = (it->tids[i] >> 18) & 0xff;
				break;
			}
		}
		sDummyActorArgs983 args;
		args.tids = it->tids;
		args.buildUids = it->buildUids;
		args.uids = it->uids;
		args.a = it->a;
		args.b = it->b;
		LoadPlayer983(it->uid, (unsigned char)it->place, &args, charIndex, it->bUser);
		Sleep(15);
		if (m_loads[0].m_bStop == true)
			return false;
	}
	return true;
}

// 004d9ff0. 0x33e = carregar os premiados (em rede: trabalho 0; fora: três sorteados); 0x33f = começar a cerimônia (movimento
// de cada boneco, câmera "시상식카메라" e trilha "그랑프리0N"); 0x341 = pôr os premiados nos pontos da cena e copiar nome,
// clube e nível de cada um para o m_userInfo do lugar; 0x342 = "os premiados estão prontos?"; 0x343 = carregar a torcida
// (trabalho 1); 0x344 = pôr um boneco da torcida na cena; 0x345 = pedir a entrega de um trabalho.
void CDummyPlayerActor::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case 0x341:
		{
			CGrandPrixDocumentAwardJp* pDoc;
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf, pDoc);
			if (pDoc == NULL)
				break;
			lua_system::lua_table_helper tbl = m_pScript->GetTable("awardEnvironmentSetting");
			std::vector<std::tr1::shared_ptr<IDummyPlayer> >::iterator end = m_players.end();
			int myPlace = 4;
			for (int place = 1; place <= 4; ++place)
			{
				const char* name = MakeStr("AWARD_PLAYER_%d", place);
				for (std::vector<std::tr1::shared_ptr<IDummyPlayer> >::iterator it = m_players.begin(); it != end; ++it)
				{
					IDummyPlayer* pPlayer = it->get();
					if (pPlayer->m_name.find(name) == std::string::npos)
						continue;
					WPolySoup::sPoint* pPoint = AWARDSOUP983()->FindPoint(LUAIDX983(tbl).GetByIndex<const char*>(place));
					if (pPoint)
					{
						pPlayer->SetPosition(pPoint->pos);
						pPlayer->m_bShow = true;
						pPlayer->m_bPlaying = true;
					}
					pPlayer->m_d8[0] = place;
					if ((unsigned int)(place - 1) >= pDoc->m_awards.size())
						break;
					unsigned long uid = pDoc->m_awards[place - 1].uid;
					pPlayer->m_d8[1] = uid;
					sUserInfo& user = Doc()->m_userInfo[place - 1];
					if (pDoc->m_awards[place - 1].bUser)
					{
						for (unsigned int i = 0; i < Doc()->m_rivalList.size(); ++i)
						{
							sRivalData* pRival = &Doc()->m_rivalList[i];
							if (pRival == NULL || pRival->uid != uid)
								continue;
							if (uid == Doc()->m_myInfo.info.dwUID)
								myPlace = place;
							strcpy(user.info.sNick, pRival->nickname);
							strcpy(user.info.sGuild, pRival->guildName);
							strcpy(EMBLEM983(user.info), pRival->guildMark);
							user.info.dwIdentity = pRival->capability;
							user.info.dwGuid = i;
							user.info.dwGuildId = pRival->guildUID;
							user.info.angelicWings = pRival->flag;
							user.stat.Level = (unsigned char)pRival->level;
							m_pFrames[place - 1] = pPlayer->m_pFrame.get();
							break;
						}
					}
					else
					{
						sAwardNpc983* pNpc = ITEMMGRAWARDJP()->FindNpc983(uid);
						if (pNpc)
						{
							for (unsigned int i = 0; i < Doc()->m_rivalList.size(); ++i)
							{
								sRivalData* pRival = &Doc()->m_rivalList[i];
								if (pRival == NULL || _stricmp(pRival->nickname, pNpc->name) != 0)
									continue;
								strcpy(user.info.sNick, pRival->nickname);
								strcpy(user.info.sGuild, pRival->guildName);
								strcpy(EMBLEM983(user.info), pRival->guildMark);
								user.info.dwIdentity = pRival->capability;
								user.info.dwGuid = i;
								user.info.dwGuildId = pRival->guildUID;
								user.info.angelicWings = pRival->flag;
								user.stat.Level = (unsigned char)pRival->level;
								pRival->rank = (unsigned char)place;
								m_pFrames[place - 1] = pPlayer->m_pFrame.get();
								break;
							}
						}
					}
					break;
				}
			}
			if (CHeadIcon::Instance())
			{
				CHeadIcon::Instance()->SetPlayerNum983((myPlace == 4) + 3);
				CHeadIcon::Instance()->SetLevel();
				CHeadIcon::Instance()->SetPodiumOffset983(0, -30.0f);
			}
		}
		break;
	case 0x342:
		if (m_1d8 == 0)
			*(bool*)msg.param1 = m_loads[0].m_bStop != false;
		else
			*(bool*)msg.param1 = false;
		break;
	case 0x33e:
		if (OnlinePlay() == 1)
		{
			if (!m_hand.IsWorking(0))
			{
				m_1d8 = 0;
				m_hand.Work(this, 0, NULL, true, -1);
			}
		}
		else
		{
			for (std::vector<std::tr1::shared_ptr<IDummyPlayer> >::iterator it = m_players.begin(); it != m_players.end();)
			{
				if ((*it)->m_kind == 0)
				{
					CSceneManagerJp::ms_pInstance->DeleteElement((*it)->m_name.c_str());
					it = m_players.erase(it);
				}
				else
				{
					++it;
				}
			}
			unsigned long tids[24];
			memset(tids, 0, sizeof(tids));
			unsigned long buildUids[5] = { 0, 0, 0, 0, 0 };
			unsigned long uids[5] = { 0, 0, 0, 0, 0 };
			sDummyActorArgs983 args;
			memset(&args, 0, sizeof(args));
			args.tids = tids;
			args.buildUids = buildUids;
			args.uids = uids;
			LoadPlayer983((unsigned long)-1, 1, &args, rand() % 15, 0);
			LoadPlayer983((unsigned long)-1, 2, &args, rand() % 15, 0);
			LoadPlayer983((unsigned long)-1, 3, &args, rand() % 15, 0);
		}
		break;
	case 0x33f:
		{
			lua_system::lua_table_helper tbl = m_pScript->GetTable("awardmotionSetting");
			for (std::vector<std::tr1::shared_ptr<IDummyPlayer> >::iterator it = m_players.begin(); it != m_players.end(); ++it)
			{
				IDummyPlayer* pPlayer = it->get();
				pPlayer->Play983(LUAIDX983(tbl).GetByIndex<const char*>(pPlayer->m_charTid + 1));
			}
			float fov;
			WPolySoup::sCamera* pCamera = AWARDSOUP983()->FindCamera("\xbd\xc3\xbb\xf3\xbd\xc4\xc4\xab\xb8\xde\xb6\xf3");
			if (pCamera)
			{
				g_camera = pCamera->mat;
				fov = (float)(pCamera->fov * 0.017453292384743690);
			}
			else
			{
				g_camera.xa = WVector(1.0f, 0.0f, 0.0f);
				g_camera.ya = WVector(0.0f, 1.0f, 0.0f);
				g_camera.za = WVector(0.0f, 0.0f, 1.0f);
				g_camera.pivot = WVector(0.0f, 0.0f, 0.0f);
				fov = 0.95993108f;
			}
			((WViewAwardJp*)g_pMainView983)->SetFov983(fov);
			g_pCameraWork983_e74d30->Play983(1, MakeStr("\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae" "0%d", rand() % 5 + 1));
		}
		break;
	case 0x343:
		if (std::count_if(m_players.begin(), m_players.end(), sDummyKindIs983(1)) == 0)
		{
			ReadCrowd983();
			m_loads[1].m_bStop = false;
			m_1d8 = 1;
			m_hand.Work(this, 1, NULL, true, -1);
		}
		break;
	case 0x344:
		{
			Waabb* pBox = (Waabb*)msg.param2;
			std::vector<std::tr1::shared_ptr<IDummyPlayer> >::iterator it =
				std::find_if(m_players.begin(), m_players.end(), sDummyNameIs983(MakeStr("AWARD_CROWD_%d", msg.param1)));
			if (it != m_players.end())
			{
				IDummyPlayer* pPlayer = it->get();
				pPlayer->m_bShow = true;
				pPlayer->m_bPlaying = true;
				pPlayer->SetPosition(pBox->GetCenter());
				pPlayer->LookAt983(WVector(0.0f, 0.0f, 0.0f));
				CSceneManagerJp::ms_pInstance->AddElement983(4, 0, 0, pPlayer->m_name.c_str(), 0x18, 0);
				CSceneManagerJp::ms_pInstance->SetPuppet(pPlayer->m_name.c_str(), pPlayer->m_pFrame->GetBonePet());
			}
		}
		break;
	case 0x345:
		RequestReady983(msg.param1);
		break;
	}
}

// layout medido no 983: `new` de 0x3e0 em 004d9a70; construtor 004d9760
struct sDummyPlayerActorLayout983
{
	C_ASSERT_VC10(sizeof(sDummyActorLoad983) == 0x28);
	C_ASSERT(offsetof(sDummyActorLoad983, m_fn) == 8);
	C_ASSERT_VC10(offsetof(sDummyActorLoad983, m_bStop) == 0x20);
	C_ASSERT_VC10(sizeof(sDummyActorSlot983) == 0x68);
	C_ASSERT_VC10(sizeof(CDummyPlayerActor) == 0x3e0);
	C_ASSERT(offsetof(CDummyPlayerActor, m_pScript) == 0x1c);
	C_ASSERT(offsetof(CDummyPlayerActor, m_players) == 0x20);
	C_ASSERT_VC10(offsetof(CDummyPlayerActor, m_crowd) == 0x40);
	C_ASSERT_VC10(offsetof(CDummyPlayerActor, m_hand) == 0x50);
	C_ASSERT_VC10(offsetof(CDummyPlayerActor, m_readyKey) == 0x1dc);
	C_ASSERT_VC10(offsetof(CDummyPlayerActor, m_loads) == 0x1e0);
	C_ASSERT_VC10(offsetof(CDummyPlayerActor, m_slots) == 0x230);
	C_ASSERT_VC10(offsetof(CDummyPlayerActor, m_pFrames) == 0x3d0);
	// B34: o que o HandleMsg e os carregadores leem de fora da unidade (medido no desmonte de 004d9ff0/004d9ba0/004d80f0)
	C_ASSERT(sizeof(sDummyActorArgs983) == 0x10);
	C_ASSERT(sizeof(sGrandPrixAward983) == 0xe4);
	C_ASSERT(offsetof(sGrandPrixAward983, buildUids) == 0x6c);
	C_ASSERT(offsetof(sGrandPrixAward983, a) == 0xe0);
	C_ASSERT(offsetof(sAwardNpc983, parts) == 0x38);
	C_ASSERT(sizeof(CAwardPick983) == 0x1c);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_itemManager) == 0x20c);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_itemManager) + offsetof(CItemManager, m_PartMap) == 0x23c); // objeto do mapa (o 0x240 anterior era o `_Myhead`; aviso da A, sessão 77)
	C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) == 0x38b0 && sizeof(sUserInfo) == 0x31c6);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_rivalList) == 0x1042c && sizeof(sRivalData) == 0x1b8);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo) + offsetof(sMyInfo, info) + offsetof(sPangYaUserInfo, dwUID) == 0x8cb);
	C_ASSERT(offsetof(sUserInfo, info) + offsetof(sPangYaUserInfo, sNick) == 0x18);
	C_ASSERT(offsetof(sUserInfo, info) + offsetof(sPangYaUserInfo, sGuild) == 0x2e);
	C_ASSERT(offsetof(sUserInfo, info) + offsetof(sPangYaUserInfo, dwIdentity) == 0x72);
	C_ASSERT(offsetof(sUserInfo, info) + offsetof(sPangYaUserInfo, dwGuid) == 0x7a);
	C_ASSERT(offsetof(sUserInfo, info) + offsetof(sPangYaUserInfo, dwGuildId) == 0x8a);
	C_ASSERT(offsetof(sUserInfo, stat) + offsetof(sPangYaUserStatistics, Level) == 0x17d);
	C_ASSERT(offsetof(sRivalData, guildMark) == 0x17c && offsetof(sRivalData, capability) == 0x198);
	C_ASSERT(offsetof(sRivalData, guildUID) == 0x174 && offsetof(sRivalData, flag) == 0x170 && offsetof(sRivalData, rank) == 0x152);
	C_ASSERT(offsetof(WPolySoup::sPoint, pos) == 0x24 && offsetof(WPolySoup::sCamera, fov) == 0x50);
};

// 008acc30: nome da textura da parte atual (+0x70 = indice, +0x1fd = n de partes, entradas de 0xc em +0x204; nome em +0xf4 da parte).
// Sem parte valida o original devolve [4]+0xf4 (o puppet em +4)
char* CPetFrameJp::TexName983()
{
	// D872: +0x70/+0x1fd/+0x204 = m_makeupIndex, m_partList+5 (m_partNum), m_partList+0xc (m_part[idx].pPart): pelos membros (cl 14.44 -0xc)
	unsigned char idx = (unsigned char)m_makeupIndex;
	const char* pl = (const char*)&m_partList;
	if (*((unsigned char*)pl + 5) <= idx)
		return (char*)*(char**)((char*)this + 4) + 0xf4;
	return (char*)*(char**)(pl + idx * 0xc + 0xc) + 0xf4;
}

// 008acdf0: CPetFrame::Animate983 (partsinfo.cpp, GetBones + ApplyBones juntos); `a` e o WBoneSet* (NULL = o do proprio quadro)
void CPetFrameJp::Animate983(const char* a, const char* b, float time)
{
	((CPetFrame*)this)->Animate983((WBoneSet*)a, b, time);
}

// 004d18f0: WPuppet::SetMatrix (m_mat = mat; o 983 deixa uma copia fora de linha, a inline do 645 e a mesma coisa)
void WPuppetPlayerJp::SetMatrix983(const WMatrix& m)
{
	SetMatrix(m);
}

